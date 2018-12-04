/*
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 */
// Copyright (c) 2018 Qualcomm Technologies, Inc.
// All rights reserved.
// Redistribution and use in source and binary forms, with or without modification, are permitted (subject to the limitations in the disclaimer below) 
// provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, 
// this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// Neither the name of Qualcomm Technologies, Inc. nor the names of its contributors may be used to endorse or promote products derived 
// from this software without specific prior written permission.
// NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE. 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, 
// BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, 
// OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, 
// EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "qcli_api.h"
#include "qapi_status.h"
#include "qapi_timer.h"
#include "qapi_socket.h"


#include "netutils.h"
#include "qurt_error.h"
#include "qurt_thread.h"
#include "qurt_signal.h"

#include "cpu_profiler_demo.h"


#define MAX_COUNTERS_TO_SEND_AT_ONCE 128


#ifndef MIN
   #define  MIN( x, y ) ( ((x) < (y)) ? (x) : (y) )
#endif


/*
 * This file contains the command handlers for file management operations
 * on non-volatile memory like list, delete, read, write
 *
 */

QCLI_Group_Handle_t qcli_cpu_profiler_handle; /* Handle for CPU Profiler Command Group. */


#define CPU_PROFILER_DEMO_PRINTF(...) QCLI_Printf(qcli_cpu_profiler_handle, __VA_ARGS__)



QCLI_Command_Status_t cpu_profiler_cli_handler_enable(uint32_t parameters_count, QCLI_Parameter_t * parameters);
QCLI_Command_Status_t cpu_profiler_cli_handler_disable(uint32_t parameters_count, QCLI_Parameter_t * parameters);
QCLI_Command_Status_t cpu_profiler_cli_handler_start(uint32_t parameters_count, QCLI_Parameter_t * parameters);
QCLI_Command_Status_t cpu_profiler_cli_handler_stop(uint32_t parameters_count, QCLI_Parameter_t * parameters);
QCLI_Command_Status_t cpu_profiler_cli_handler_reset(uint32_t parameters_count, QCLI_Parameter_t * parameters);
QCLI_Command_Status_t cpu_profiler_cli_send_results(uint32_t parameters_count, QCLI_Parameter_t * parameters);
QCLI_Command_Status_t cpu_profiler_cli_print_results(uint32_t parameters_count, QCLI_Parameter_t * parameters);

const QCLI_Command_t cpu_profiler_cmd_list[] =
{
    {cpu_profiler_cli_handler_enable, false, "enable", "\n", "enable profiling\n"},
    {cpu_profiler_cli_handler_disable, false, "disable", "\n", "disable profiling\n"},
    {cpu_profiler_cli_handler_start, false, "start", "\n", "start profiling\n"},
    {cpu_profiler_cli_handler_stop, false, "stop", "\n", "stop profiling\n"},
    {cpu_profiler_cli_handler_reset, false, "reset", "\n", "reset profiling\n"},

    {cpu_profiler_cli_send_results, false, "send_results", "\n", "send profiling results\n"},
    {cpu_profiler_cli_print_results, false, "print_results", "\n", "print profiling\n"},
};

const QCLI_Command_Group_t cpu_profiler_cmd_group =
{
    "CpuProfiler",              /* Group_String: will display cmd prompt as "CpuProfiler> " */
    sizeof(cpu_profiler_cmd_list)/sizeof(cpu_profiler_cmd_list[0]),   /* Command_Count */
    cpu_profiler_cmd_list        /* Command_List */
};


// The value of the g_cpu_profiler_number_of_functions is patched in the build.bat
// during the post build step
uint32_t g_cpu_profiler_number_of_functions = 1;
uint32_t * g_cpu_profiler_memory_block = 0;
uint32_t g_cpu_profiler_memory_block_size = 0;


/*****************************************************************************
 * This function is used to register the Fs Command Group with QCLI.
 *****************************************************************************/
void Initialize_CpuProfiler_Demo(void)
{
    /* Attempt to reqister the Command Groups with the qcli framework.*/
    qcli_cpu_profiler_handle = QCLI_Register_Command_Group(NULL, &cpu_profiler_cmd_group);
    if (qcli_cpu_profiler_handle)
    {
        QCLI_Printf(qcli_cpu_profiler_handle, "CpuProfiler Registered\n");
    }

    g_cpu_profiler_memory_block_size = sizeof(uint32_t)*g_cpu_profiler_number_of_functions;
    g_cpu_profiler_memory_block = (uint32_t *) malloc(g_cpu_profiler_memory_block_size);

    return;
}



cpu_profiler_ctxt_t g_cpu_profiler_ctxt = {
    .sock = QAPI_ERROR
};



enum {
    MSG_TYPE_REQUEST_FUNCTION_METADATAS = 1,
    MSG_TYPE_REPLY_FUNCTION_METADATAS = 2,
    MSG_TYPE_INDICATE_FUNCTION_TIMES = 3,
};


typedef struct type_length_s {
    uint32_t type;
    uint32_t length;
} type_length_t;


#define MAX_RECEIVE_ATTEMPTS 10
#define MAX_SEND_ATTEMPTS 10


static int helper_receive_data(int32_t sock, void * buffer, uint32_t bytes_to_receive) {
    int32_t attempts = MAX_RECEIVE_ATTEMPTS;
    uint32_t total_bytes_received = 0;
    uint32_t receive_flags = 0;
    char * vp_buffer = (char *) buffer;
    while ( bytes_to_receive > 0 ) {
        int32_t bytes_received =
            qapi_recv(sock, &vp_buffer[total_bytes_received], bytes_to_receive, receive_flags);
        if ( bytes_received < 0 ) {
            return -1;
        }
        else if ( bytes_received == 0 ) {
            attempts--;
            if ( attempts == 0 ) {
                return -2;
            }
        }
        else {
            attempts = MAX_RECEIVE_ATTEMPTS;
        }
        if ( bytes_received > bytes_to_receive ) {
            return -3;
        }
        total_bytes_received += bytes_received;
        bytes_to_receive -= bytes_received;
    }
    return 0;
}


static int helper_send_data(int32_t sock, void * buffer, uint32_t bytes_to_send) {
    uint32_t total_bytes_sent = 0;
    uint32_t send_flags = 0;
    char * vp_buffer = (char *) buffer;
    while ( bytes_to_send > 0 ) {
        int32_t bytes_sent =
            qapi_send(sock, &vp_buffer[total_bytes_sent], bytes_to_send, send_flags);
        if ( bytes_sent != bytes_to_send ) {
            int errno = qapi_errno(sock);
            if ( 0 ||
                (EPIPE == errno) ||
                (EBADF == errno) )
            {
                // the socket has closed - no point of continuing the test
                CPU_PROFILER_DEMO_PRINTF("Socket got closed\r\n");
                break;
            }
            else {
                // severe push back, let the other processes run for 1 ms
                qosal_msec_delay(1);
            }
        }
        total_bytes_sent += bytes_sent;
        bytes_to_send -= bytes_sent;
    }
    return 0;
}


int cpu_profiler_cleanup()
{
    memset(g_cpu_profiler_ctxt.function_info_list, 0, MAX_FUNCTION_INFO_LIST_LENGTH*sizeof(g_cpu_profiler_ctxt.function_info_list[0]));
    g_cpu_profiler_ctxt.function_info_list_length = 0;
    if ( g_cpu_profiler_ctxt.function_samples ) {
        free(g_cpu_profiler_ctxt.function_samples);
        g_cpu_profiler_ctxt.function_samples = 0;
    }
#if 0
    if ( g_cpu_profiler_ctxt.function_addresses ) {
        free(g_cpu_profiler_ctxt.function_addresses);
        g_cpu_profiler_ctxt.function_addresses = 0;
    }
#endif
    if ( QAPI_ERROR != g_cpu_profiler_ctxt.sock ) {
        qapi_socketclose(g_cpu_profiler_ctxt.sock);
        g_cpu_profiler_ctxt.sock = QAPI_ERROR;
    }

    return 0;
}


#define WriteReg(address, value) \
    (*((volatile uint32_t *) address) = value)

#define ReadReg(address) \
    (*((volatile uint32_t *) address))

#define CPU_PROFILER_TIMER_INTERRUPT_INDEX 1
#define NVIC_SYSTEM_INTERRUPTS_COUNT 16
#define CPU_PROFILER_TIMER_INTERRUPT_INDEX_IN_NVIC (NVIC_SYSTEM_INTERRUPTS_COUNT + CPU_PROFILER_TIMER_INTERRUPT_INDEX)
#define CPU_PROFILER_TIMER_LOAD      0x44001020
#define CPU_PROFILER_TIMER_CONTROL   0x44001028
#define CPU_PROFILER_TIMER_INTCLR    0x4400102c
#define CPU_PROFILER_TIMER_BGLOAD    0x44001038



extern uint32_t vectTable[255];
extern void cpu_profiler_interrupt_irq_handler(void);

int cpu_profiler_is_enabled() {
    return ( QAPI_ERROR != g_cpu_profiler_ctxt.sock );
}


int cpu_profiler_start(uint32_t time_between_samples_in_us, uint32_t time_to_run_profiler_in_seconds)
{
    // the APSS_TM runs at 32MHz => each us = 32 ticks
    uint32_t sampling_period = 32 * time_between_samples_in_us;

    g_cpu_profiler_ctxt.count_of_pcs_captured = 0;
    g_cpu_profiler_ctxt.count_of_pcs_to_capture = 1000 * 1000 * time_to_run_profiler_in_seconds / time_between_samples_in_us;

    IRQ_Disable();

    // We want to use the TIMER1 interrupt to sample PC values.
    // First save the address of the default interrupt handler for TIMER1 interrupt in "original_isr" variable
    // Second, patch the NVIC vector table directly.
    g_cpu_profiler_ctxt.original_isr = vectTable[CPU_PROFILER_TIMER_INTERRUPT_INDEX_IN_NVIC];
    vectTable[CPU_PROFILER_TIMER_INTERRUPT_INDEX_IN_NVIC] = (uint32_t) cpu_profiler_interrupt_irq_handler;

    WriteReg(CPU_PROFILER_TIMER_BGLOAD, sampling_period);
    WriteReg(CPU_PROFILER_TIMER_LOAD, sampling_period);
    WriteReg(CPU_PROFILER_TIMER_CONTROL, 0xe2);

    InterruptController_SetEnable(CPU_PROFILER_TIMER_INTERRUPT_INDEX);

    IRQ_Enable();
    return 0;
}


int cpu_profiler_stop()
{
    IRQ_Disable();

    vectTable[CPU_PROFILER_TIMER_INTERRUPT_INDEX_IN_NVIC] = g_cpu_profiler_ctxt.original_isr;

    WriteReg(CPU_PROFILER_TIMER_BGLOAD, 0xffffffff);
    WriteReg(CPU_PROFILER_TIMER_LOAD, 0xffffffff);
    WriteReg(CPU_PROFILER_TIMER_CONTROL, 0);

    InterruptController_ClearEnable(CPU_PROFILER_TIMER_INTERRUPT_INDEX);
    InterruptController_ClearPending(CPU_PROFILER_TIMER_INTERRUPT_INDEX);

    IRQ_Enable();


    int i;
    for ( i = 0; i < g_cpu_profiler_ctxt.function_info_list_length; i++ ) {
        function_info_t * p_function_info = &g_cpu_profiler_ctxt.function_info_list[i];
        p_function_info->count += g_cpu_profiler_ctxt.function_samples[p_function_info->index];
        g_cpu_profiler_ctxt.function_samples[p_function_info->index] = 0;
    }


    return 0;
}


int cpu_profiler_reset()
{
    if ( g_cpu_profiler_ctxt.function_samples ) {
        memset(
            g_cpu_profiler_ctxt.function_samples,
            0,
            g_cpu_profiler_ctxt.functions_count * sizeof(g_cpu_profiler_ctxt.function_samples[0])
            );
    }


    memset(g_cpu_profiler_ctxt.function_info_list, 0, MAX_FUNCTION_INFO_LIST_LENGTH*sizeof(g_cpu_profiler_ctxt.function_info_list[0]));
    g_cpu_profiler_ctxt.function_info_list_length = 0;

    g_cpu_profiler_ctxt.count_of_pcs_captured = 0;

    return 0;
}


int cpu_profiler_helper_connect_to_server_and_retrive_list_of_functions(uint32_t server_ip, uint32_t server_port)
{
    int status;
    struct sockaddr_in remote_sockaddr;
    type_length_t type_length_header;

    g_cpu_profiler_ctxt.sock = qapi_socket( AF_INET, SOCK_STREAM, 0);
    if( QAPI_ERROR == g_cpu_profiler_ctxt.sock ) {
        g_cpu_profiler_ctxt.sock = 0;
        CPU_PROFILER_DEMO_PRINTF("Failed on a call to qapi_socket(), status=%d\r\n", g_cpu_profiler_ctxt.sock);
        goto cpu_profiler_helper_connect_to_server_and_retrive_list_of_functions_on_error;
    }


    // connect to the server to report results to
    memset(&remote_sockaddr, 0, sizeof(remote_sockaddr));
    remote_sockaddr.sin_addr.s_addr = server_ip;
    remote_sockaddr.sin_port = htons(server_port);
    remote_sockaddr.sin_family = AF_INET;

    qosal_msec_delay(20);

    status = qapi_connect( g_cpu_profiler_ctxt.sock, (struct sockaddr*)(&remote_sockaddr), sizeof(remote_sockaddr));
    if( -1 == status ) {
        CPU_PROFILER_DEMO_PRINTF("Failed on a call to qapi_connect(), status=%d\r\n", status);
        goto cpu_profiler_helper_connect_to_server_and_retrive_list_of_functions_on_error;
    }

    qosal_msec_delay(20);

    // send message to request functions metadatas

    type_length_header.type = htonl(MSG_TYPE_REQUEST_FUNCTION_METADATAS);
    type_length_header.length = htonl(0);
    status = helper_send_data(g_cpu_profiler_ctxt.sock, &type_length_header, sizeof(type_length_header));
    if ( 0 != status ) {
        CPU_PROFILER_DEMO_PRINTF("Failed on a call to helper_send_data() requesting function list, status=%d\r\n", status);
        goto cpu_profiler_helper_connect_to_server_and_retrive_list_of_functions_on_error;
    }

    // receive the header of the MSG_TYPE_REPLY_FUNCTION_METADATAS message
    status = helper_receive_data(g_cpu_profiler_ctxt.sock, &type_length_header, sizeof(type_length_header));
    if ( 0 != status ) {
        CPU_PROFILER_DEMO_PRINTF("Failed on a call to helper_receive_data() retrieving function list, status=%d\r\n", status);
        goto cpu_profiler_helper_connect_to_server_and_retrive_list_of_functions_on_error;
    }
    type_length_header.type = ntohl(type_length_header.type);
    type_length_header.length = ntohl(type_length_header.length);
    if ( type_length_header.type != MSG_TYPE_REPLY_FUNCTION_METADATAS ) {
        CPU_PROFILER_DEMO_PRINTF("Received wrong message type, expecting=%d, but received=%d\r\n", MSG_TYPE_REPLY_FUNCTION_METADATAS, type_length_header.type);
        goto cpu_profiler_helper_connect_to_server_and_retrive_list_of_functions_on_error;
    }
    if ( (type_length_header.length == 0) || type_length_header.length & 0x3 ) {
        CPU_PROFILER_DEMO_PRINTF("Wrong message length, length=%d\r\n", type_length_header.length);
        goto cpu_profiler_helper_connect_to_server_and_retrive_list_of_functions_on_error;
    }

    // allocate the profiling data structures
    g_cpu_profiler_ctxt.functions_count = type_length_header.length / sizeof(uint32_t);

    if ( type_length_header.length > g_cpu_profiler_memory_block_size )
    {
        CPU_PROFILER_DEMO_PRINTF(
            "Space necessary to fit the function_addresses in memory (%d) is larger than pre-allocated g_cpu_profiler_memory_block (%d)\r\n",
            type_length_header.length,
            g_cpu_profiler_memory_block_size
            );
        CPU_PROFILER_DEMO_PRINTF("Please increase the size of g_cpu_profiler_memory_block and re-compile\r\n");
        goto cpu_profiler_helper_connect_to_server_and_retrive_list_of_functions_on_error;
    }
    g_cpu_profiler_ctxt.function_addresses = g_cpu_profiler_memory_block;
    memset(g_cpu_profiler_ctxt.function_addresses, 0, type_length_header.length);

    uint32_t function_samples_size = g_cpu_profiler_ctxt.functions_count*sizeof(g_cpu_profiler_ctxt.function_samples[0]);
    g_cpu_profiler_ctxt.function_samples = (uint16_t *) malloc(function_samples_size);
    if ( !g_cpu_profiler_ctxt.function_samples ) {
        CPU_PROFILER_DEMO_PRINTF("Failed to allocate function_samples\r\n");
        goto cpu_profiler_helper_connect_to_server_and_retrive_list_of_functions_on_error;
    }
    memset(g_cpu_profiler_ctxt.function_samples, 0, function_samples_size);

    // receive the payload of the MSG_TYPE_REPLY_FUNCTION_METADATAS message
    // please note that due to performance considerations, the PC doesn't byte swap the values of function_addresses
    status = helper_receive_data(g_cpu_profiler_ctxt.sock, g_cpu_profiler_ctxt.function_addresses, type_length_header.length);
    if (  0 != status ) {
        CPU_PROFILER_DEMO_PRINTF("Failed on a call to helper_receive_data() receiving function addresses, status=%d\r\n", status);
        goto cpu_profiler_helper_connect_to_server_and_retrive_list_of_functions_on_error;
    }

    return 0;

cpu_profiler_helper_connect_to_server_and_retrive_list_of_functions_on_error:
    cpu_profiler_cleanup();
    return -1;
}




QCLI_Command_Status_t cpu_profiler_cli_handler_enable(uint32_t parameters_count, QCLI_Parameter_t * parameters)
{
    if ( cpu_profiler_is_enabled() )
    {
        CPU_PROFILER_DEMO_PRINTF("CpuProfiler is already enabled, disable it before re-enabling\r\n");
        return QCLI_STATUS_ERROR_E;
    }

    int status;

    uint32_t server_ip;
    uint32_t server_port;

    // Extract log server IP address and port from the CLI parameters
    if ( 2 != parameters_count ) {
        CPU_PROFILER_DEMO_PRINTF("Invalid number of arguments, must be exactly 2\r\n");
        goto cpu_profiler_cli_handler_enable_on_error;
    }

    if ( 0 != inet_pton(AF_INET, parameters[0].String_Value, &server_ip) ) {
        CPU_PROFILER_DEMO_PRINTF("Invalid server_ip address (%s)\r\n", parameters[0].String_Value);
        goto cpu_profiler_cli_handler_enable_on_error;
    }

    if ( !parameters[1].Integer_Is_Valid ) {
        CPU_PROFILER_DEMO_PRINTF("Invalid server_port (%s)\r\n", parameters[1].String_Value);
        goto cpu_profiler_cli_handler_enable_on_error;
    }
    server_port = parameters[1].Integer_Value;


    status = cpu_profiler_helper_connect_to_server_and_retrive_list_of_functions(server_ip, server_port);
    if ( 0 != status ) {
        CPU_PROFILER_DEMO_PRINTF("Failed on a call to cpu_profiler_helper_connect_to_server_and_retrive_list_of_functions(), status=%d\r\n", status);
        goto cpu_profiler_cli_handler_enable_on_error;
    }

    memset(g_cpu_profiler_ctxt.function_info_list, 0, MAX_FUNCTION_INFO_LIST_LENGTH*sizeof(g_cpu_profiler_ctxt.function_info_list[0]));
    g_cpu_profiler_ctxt.function_info_list_length = 0;

    return QCLI_STATUS_SUCCESS_E;

cpu_profiler_cli_handler_enable_on_error:
    cpu_profiler_cleanup();
    CPU_PROFILER_DEMO_PRINTF("Usage: enable server_ip server_port\r\n");
    return QCLI_STATUS_ERROR_E;
}


QCLI_Command_Status_t cpu_profiler_cli_handler_disable(uint32_t parameters_count, QCLI_Parameter_t * parameters)
{
    int status;

    status = cpu_profiler_stop();
    if ( 0 != status ) {
        CPU_PROFILER_DEMO_PRINTF("Failed on a call to cpu_profiler_stop(), status=%d\r\n", status);
    }

    status = cpu_profiler_cleanup();
    if ( 0 != status ) {
        CPU_PROFILER_DEMO_PRINTF("Failed on a call to cpu_profiler_cleanup(), status=%d\r\n", status);
    }

    return QCLI_STATUS_SUCCESS_E;
}


uint32_t calculate_timer_load_value(uint32_t sampling_frequency)
{
     return (32000000 / sampling_frequency);
}


QCLI_Command_Status_t cpu_profiler_cli_handler_start(uint32_t parameters_count, QCLI_Parameter_t * parameters)
{
    int status;

    if ( !cpu_profiler_is_enabled() ) {
        CPU_PROFILER_DEMO_PRINTF("Must enable the cpu_profiler first\r\n");
        return QCLI_STATUS_ERROR_E;
    }

    cpu_profiler_stop();

    // list of parameters:
    // 1st: us_between_samples
    // 2nd: time in seconds to run the profiler

    uint32_t time_between_samples_in_us = 50;
    uint32_t time_to_run_profiler_in_seconds = 5;

    if ( parameters_count > 0 )
    {
        if ( parameters[0].Integer_Is_Valid ) {
            time_between_samples_in_us = parameters[0].Integer_Value;
        }
        else {
            goto cpu_profiler_cli_handler_start_on_error;
        }
    }

    if ( parameters_count > 1 )
    {
        if ( parameters[1].Integer_Is_Valid ) {
            time_to_run_profiler_in_seconds = parameters[1].Integer_Value;
        }
        else {
            goto cpu_profiler_cli_handler_start_on_error;
        }
    }

    status = cpu_profiler_start(time_between_samples_in_us, time_to_run_profiler_in_seconds);
    if ( 0 != status ) {
        CPU_PROFILER_DEMO_PRINTF("Failed on a call to cpu_profiler_start(), status=%d\r\n", status);
    }

    return QCLI_STATUS_SUCCESS_E;

cpu_profiler_cli_handler_start_on_error:
    CPU_PROFILER_DEMO_PRINTF("Usage: start <time_between_samples_in_us> <time_to_run_profiler_in_seconds>\r\n");
    CPU_PROFILER_DEMO_PRINTF("\t<time_between_samples_in_us>: time in us between each PC (default=50us),\r\n");
    CPU_PROFILER_DEMO_PRINTF("\t<profiling_duration>: time in seconds to run the profiler (defaults=5s),\r\n");
    return QCLI_STATUS_ERROR_E;
}


QCLI_Command_Status_t cpu_profiler_cli_handler_stop(uint32_t parameters_count, QCLI_Parameter_t * parameters)
{
    if ( !cpu_profiler_is_enabled() ) {
        CPU_PROFILER_DEMO_PRINTF("Must enable the cpu_profiler first\r\n");
        return QCLI_STATUS_ERROR_E;
    }

    cpu_profiler_stop();

    return QCLI_STATUS_SUCCESS_E;
}


QCLI_Command_Status_t cpu_profiler_cli_handler_reset(uint32_t parameters_count, QCLI_Parameter_t * parameters)
{
    if ( !cpu_profiler_is_enabled() ) {
        CPU_PROFILER_DEMO_PRINTF("Must enable the cpu_profiler first\r\n");
        return QCLI_STATUS_ERROR_E;
    }

    cpu_profiler_reset();

    return QCLI_STATUS_SUCCESS_E;
}


QCLI_Command_Status_t cpu_profiler_cli_send_results(uint32_t parameters_count, QCLI_Parameter_t * parameters)
{
    if ( !cpu_profiler_is_enabled() ) {
        CPU_PROFILER_DEMO_PRINTF("Must enable the cpu_profiler first\r\n");
        return QCLI_STATUS_ERROR_E;
    }

    int status;

    // sends results to the server
    type_length_t type_length_header;
    type_length_header.type = htonl(MSG_TYPE_INDICATE_FUNCTION_TIMES);
    const uint32_t payload_length = g_cpu_profiler_ctxt.functions_count*sizeof(uint32_t);
    type_length_header.length = htonl(payload_length);

    status = helper_send_data(g_cpu_profiler_ctxt.sock, &type_length_header, sizeof(type_length_header));
    if ( 0 != status ) {
        CPU_PROFILER_DEMO_PRINTF("Failed on a call to helper_send_data() sending TLV header, status=%d\r\n", status);
        return QCLI_STATUS_ERROR_E;
    }

    uint32_t max_buffer_elements_count = MAX_COUNTERS_TO_SEND_AT_ONCE;
    uint32_t buffer_size = max_buffer_elements_count * sizeof(uint32_t);
    uint32_t * p_buffer = (uint32_t *) malloc(buffer_size);
    if ( !p_buffer ) {
        CPU_PROFILER_DEMO_PRINTF("Failed to allocate p_buffer\r\n");
        goto cpu_profiler_cli_send_results_error;
    }
    int i = 0;
    while ( i < g_cpu_profiler_ctxt.functions_count ) {
        uint32_t elements_to_send = MIN(max_buffer_elements_count, (g_cpu_profiler_ctxt.functions_count-i));
        uint32_t j;
        for ( j = 0; j < elements_to_send; j++ ) {
            uint32_t sample_index = i + j;
            p_buffer[j] = g_cpu_profiler_ctxt.function_samples[sample_index];
            int k;
            for ( k = 0; k < g_cpu_profiler_ctxt.function_info_list_length; k++ ) {
                function_info_t * p_function_info = &g_cpu_profiler_ctxt.function_info_list[k];
                if ( p_function_info->index == sample_index ) {
                    p_buffer[j] += p_function_info->count;
                }
            }
        }
        status = helper_send_data(g_cpu_profiler_ctxt.sock, p_buffer, elements_to_send*sizeof(uint32_t));
        if ( 0 != status ) {
            CPU_PROFILER_DEMO_PRINTF("Failed on a call to helper_send_data() sending profiling results, status=%d\r\n", status);
            goto cpu_profiler_cli_send_results_error;
        }
        i += elements_to_send;
    }

    return QCLI_STATUS_SUCCESS_E;

cpu_profiler_cli_send_results_error:
    if ( p_buffer ) {
        free(p_buffer);
        p_buffer = 0;
    }
    return QCLI_STATUS_ERROR_E;
}


static inline uint32_t helper_is_index_recorded_as_cpu_hogger(
    uint32_t * cpu_hogger_indices_array,
    uint32_t number_of_recorded_cpu_hoggers,
    uint32_t index
    )
{
    int ii;
    for ( ii = 0; ii < number_of_recorded_cpu_hoggers; ii++ ) {
        if ( cpu_hogger_indices_array[ii] == index ) {
            return 1;
        }
    }
    return 0;
}

static int helper_print_top_most_cpu_hoggers(const int cpu_hoggers_to_print)
{
    uint32_t cpu_hogger_indices_size = cpu_hoggers_to_print*sizeof(uint32_t);
    uint32_t * cpu_hogger_indices_array = (uint32_t *) malloc(cpu_hogger_indices_size);
    if ( !cpu_hogger_indices_array ) {
        CPU_PROFILER_DEMO_PRINTF("Failed to allocate cpu_hogger_indices_array\r\n");
        return -1;
    }
    memset(cpu_hogger_indices_array, 0, cpu_hogger_indices_size);

    uint32_t number_of_recorded_cpu_hoggers;

    for ( number_of_recorded_cpu_hoggers = 0; number_of_recorded_cpu_hoggers < MIN(g_cpu_profiler_ctxt.function_info_list_length, cpu_hoggers_to_print); number_of_recorded_cpu_hoggers++ ) {
        int i;
        uint32_t cpu_hogger_index = 0;
        uint32_t cpu_hogger_value = 0;
        for ( i  = 0; i < g_cpu_profiler_ctxt.function_info_list_length; i++ ) {
            function_info_t * p_function_info = &g_cpu_profiler_ctxt.function_info_list[i];
            if (!helper_is_index_recorded_as_cpu_hogger(cpu_hogger_indices_array, number_of_recorded_cpu_hoggers, p_function_info->index))
            {
                cpu_hogger_index = p_function_info->index;
                cpu_hogger_value = p_function_info->count;
                break;
            }
        }
        i++;
        for ( ; i < g_cpu_profiler_ctxt.function_info_list_length; i++ ) {
            function_info_t * p_function_info = &g_cpu_profiler_ctxt.function_info_list[i];
            if ( 1 &&
                (cpu_hogger_value <= p_function_info->count ) &&
                (!helper_is_index_recorded_as_cpu_hogger(cpu_hogger_indices_array, number_of_recorded_cpu_hoggers, p_function_info->index))
               )
            {
                cpu_hogger_index = p_function_info->index;
                cpu_hogger_value = p_function_info->count;
            }
        }
        cpu_hogger_indices_array[number_of_recorded_cpu_hoggers] = cpu_hogger_index;
    }

    for ( ; number_of_recorded_cpu_hoggers < cpu_hoggers_to_print; number_of_recorded_cpu_hoggers++ ) {
        uint32_t cpu_hogger_index = 0;
        uint32_t cpu_hogger_value = 0;
        uint32_t i;
        for ( i = 0; i < g_cpu_profiler_ctxt.functions_count; i++ ) {
            if (!helper_is_index_recorded_as_cpu_hogger(cpu_hogger_indices_array, number_of_recorded_cpu_hoggers, i)) {
                cpu_hogger_index = i;
                cpu_hogger_value = g_cpu_profiler_ctxt.function_samples[i];
                break;
            }
        }
        i++;
        for ( ; i < g_cpu_profiler_ctxt.functions_count; i++ ) {
            if ( 1
                && (cpu_hogger_value <= g_cpu_profiler_ctxt.function_samples[i])
                && (!helper_is_index_recorded_as_cpu_hogger(cpu_hogger_indices_array, number_of_recorded_cpu_hoggers, i))
                )
            {
                cpu_hogger_index = i;
                cpu_hogger_value = g_cpu_profiler_ctxt.function_samples[i];
            }
        }
        cpu_hogger_indices_array[number_of_recorded_cpu_hoggers] = cpu_hogger_index;
    }


    // count total number of samples taken during profiling
    uint32_t i;
    uint32_t total_samples = 0;
    for ( i = 0; i < g_cpu_profiler_ctxt.functions_count; i++ ) {
        total_samples += g_cpu_profiler_ctxt.function_samples[i];
    }
    for ( i = 0; i < g_cpu_profiler_ctxt.function_info_list_length; i++ ) {
        function_info_t * p_function_info = &g_cpu_profiler_ctxt.function_info_list[i];
        total_samples += p_function_info->count;
    }


    // print the CPU hoggers
    CPU_PROFILER_DEMO_PRINTF("The %d top most CPU hoggers are:\r\n", cpu_hoggers_to_print);
    CPU_PROFILER_DEMO_PRINTF("Function Address, CPU Utilization %% [Samples]:  thread_id [samples] ...\r\n");
    for ( i = 0; i < cpu_hoggers_to_print; i++ ) {
        int cpu_hogger_index = cpu_hogger_indices_array[i];
        uint32_t function_address = g_cpu_profiler_ctxt.function_addresses[cpu_hogger_index];
        uint32_t function_samples = g_cpu_profiler_ctxt.function_samples[cpu_hogger_index];
        int j;
        function_info_t * p_function_info = 0;
        for ( j = 0; j < g_cpu_profiler_ctxt.function_info_list_length; j++ ) {
            if ( g_cpu_profiler_ctxt.function_info_list[j].index == cpu_hogger_index ) {
                p_function_info = &g_cpu_profiler_ctxt.function_info_list[j];
                function_samples += p_function_info->count;
                break;
            }
        }

        uint32_t percent_cpu_utilization = (total_samples) ? (100 * function_samples / total_samples) : 0;
        CPU_PROFILER_DEMO_PRINTF(
            "%08x, %d [%d]",
            function_address,
            percent_cpu_utilization,
            function_samples
            );

#if ENABLE_MOST_USED_FUNCTIONS_THREAD_ID_LOGGING
        if ( p_function_info ) {
            CPU_PROFILER_DEMO_PRINTF(":\t");
            for ( j = 0; j < p_function_info->thread_info_list_length; j++ ) {
                thread_info_t * p_thread_info = &p_function_info->thread_info_list[j];
                CPU_PROFILER_DEMO_PRINTF("0x%08x [%d]\t", p_thread_info->thread_id, p_thread_info->count);
            }
        }
#endif

        CPU_PROFILER_DEMO_PRINTF("\r\n");

    }

    if ( cpu_hogger_indices_array ) {
        free(cpu_hogger_indices_array);
    }

    return 0;
}


QCLI_Command_Status_t cpu_profiler_cli_print_results(uint32_t parameters_count, QCLI_Parameter_t * parameters)
{
    if ( !cpu_profiler_is_enabled() ) {
        CPU_PROFILER_DEMO_PRINTF("Must enable the cpu_profiler first\r\n");
        return QCLI_STATUS_ERROR_E;
    }

    int status;

    uint32_t number_of_top_cpu_hoggers_to_print = 0;

    if ( 1 &&
         (parameters_count == 1) &&
         parameters[0].Integer_Is_Valid
         )
    {
        number_of_top_cpu_hoggers_to_print = parameters[0].Integer_Value;
    }
    else
    {
        CPU_PROFILER_DEMO_PRINTF("Invalid argument\r\n");
        goto cpu_profiler_cli_print_results_on_error;
    }

    if ( number_of_top_cpu_hoggers_to_print > g_cpu_profiler_ctxt.functions_count ) {
        CPU_PROFILER_DEMO_PRINTF(
            "number_of_top_cpu_hoggers_to_print (%d) must not be larger than total number of functions (%d)\r\n",
            number_of_top_cpu_hoggers_to_print,
            g_cpu_profiler_ctxt.functions_count
            );
        goto cpu_profiler_cli_print_results_on_error;
    }

    status = helper_print_top_most_cpu_hoggers(number_of_top_cpu_hoggers_to_print);
    if ( 0 != status ) {
        goto cpu_profiler_cli_print_results_on_error;
    }

    return QCLI_STATUS_SUCCESS_E;

cpu_profiler_cli_print_results_on_error:
    CPU_PROFILER_DEMO_PRINTF("Usage: print_results number_of_top_cpu_hoggers_to_print\r\n");
    return QCLI_STATUS_ERROR_E;
}
