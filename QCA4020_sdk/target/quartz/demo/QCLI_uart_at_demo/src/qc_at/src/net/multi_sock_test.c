/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
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

/*-------------------------------------------------------------------------
 * Include Files
 *-----------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "qcli.h"
#include "qcli_api.h"

#include "qurt_thread.h"
#include "qurt_types.h"
#include "qurt_timer.h"
#include "qurt_mutex.h"
#include "qurt_error.h"

#include "qapi_socket.h"
#include "qapi_timer.h"
#include "net_utils.h"
#include "qapi_ns_utils.h"
#include "qc_api_main.h"
#include "qc_drv_net.h"
#include "qosa_util.h"

/*-------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 *-----------------------------------------------------------------------*/
#define MAX_NO_OF_SESSIONS                   12
#define DEFAULT_TIME_DURATION                1000
#define ENABLE_SEESION_LOGGING               1
#define SEND_THREAD_PRIORITY                 10
#define RECV_THREAD_PRIORITY                 10
#define THRD_STACK_SIZE                      2048
#define SIZE_BUF                             1500
#define IPV4_MODE                            4
#define IPV6_MODE                            6
#define TCP_PROTO                            0
#define UDP_PROTO                            1
#define DEFAULT_SEND_BYTES                   512
#define RECV_BYTES                           1500
#define CHAR_SEND                            'a'

// Signals to synchronize
#define STOP_SIGNAL                          (0x1 << 0)
#define SEND_STOPPED_SIGNAL                  (0x1 << 1)
#define RECV_STOPPED_SIGNAL                  (0x1 << 2)

/*-------------------------------------------------------------------------
 * Type Declarations
 *-----------------------------------------------------------------------*/
typedef struct sock_test_struct {
    char     server_ip[32];
    uint16_t ip_mode;
    uint16_t port_no;
    int32_t  sock_id;
    uint32_t proto;
    int32_t  session_id;
    uint32_t sent_bytes;
    uint32_t recv_bytes;
} sock_session;

/*-------------------------------------------------------------------------
 * Static & global Variable Declarations
 *-----------------------------------------------------------------------*/
static sock_session *session[MAX_NO_OF_SESSIONS];
static fd_set rset, sockset;
static uint32_t num_session;
static uint32_t signal_set;
static uint32_t time_ms;
static uint32_t num_send_bytes;
static qurt_mutex_t multiSockLock;

static char *recv_buf;
static char *send_buf;
static qurt_signal_t stop_signal;

/*-------------------------------------------------------------------------
 * Function Declarations
 *-----------------------------------------------------------------------*/
#ifdef DEBUG_PRINT
static void print_session_details(sock_session *session);
#endif
static void process_multi_sock_close(void);
int multi_sock_create(uint16_t ip_mode, uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);


/*-------------------------------------------------------------------------
 *Function_name : copy_sock_params : copies the session details
 *-------------------------------------------------------------------------*/
static void copy_sock_params(sock_session *session, uint32_t ip_mode, int32_t id, uint32_t protocol, char *ip_addr, uint16_t port, int32_t s_id)
{
    session->ip_mode = ip_mode;
    session->sock_id = id;
    session->proto = protocol;
    session->port_no = port;
    strcpy(session->server_ip,ip_addr);
    session->session_id = s_id;
    session->sent_bytes = 0;
    session->recv_bytes = 0;
}

/*-------------------------------------------------------------------------
 * Function_name :sock_close - Closes the socket for corresponding session
 *-------------------------------------------------------------------------*/
static int32_t sock_close(sock_session *session)
{
    if (session->sock_id != -1)
    {
        qc_drv_net_fd_clr(qc_api_get_qc_drv_context(), session->sock_id, &rset);
        if(0 != qc_drv_net_socketclose(qc_api_get_qc_drv_context(), session->sock_id))
        {
            LOG_ERR("sock _closing is failed\n");
            return -1;
        }
    }
    return 0;
}

/*-------------------------------------------------------------------------
 * Function_name : sock_open
 * Return value: Socket descriptor for succsess , -1 for failure
 * Opens the Socket for corresponding session
 *-------------------------------------------------------------------------*/
static int sock_open(sock_session *session)
{
    int32_t socket = -1;
    int32_t family;

    if (session->ip_mode == IPV6_MODE)
    {
        family = AF_INET6;
    }
    else if (session->ip_mode == IPV4_MODE)
    {
        family = AF_INET;
    }
    else
    {
        LOG_ERR("Invalide IP mode :%d\n", session->ip_mode);
        return -1;
    }

    if (session->proto == TCP_PROTO)
    {
        socket = qc_drv_net_socket(qc_api_get_qc_drv_context(), family, SOCK_STREAM, 0);
    }
    else if (session->proto == UDP_PROTO)
    {
        socket = qc_drv_net_socket(qc_api_get_qc_drv_context(), family, SOCK_DGRAM, 0);
    }
    else
    {
        LOG_ERR("Network protocol is not valid\n");
    }

    if (socket < 0)
    {
        LOG_ERR("Socket creation error\n");
        return -1;
    }

    session->sock_id = socket;
    return 0;
}

/*----------------------------------------------------------------------------
 *Function_name:process_multi_sock_close
 * To Close the sessions
 *---------------------------------------------------------------------------*/
static void process_multi_sock_close(void)
{
    int32_t index;

    for (index = 0; index < MAX_NO_OF_SESSIONS; index++)
    {
        if (session[index] != NULL)
        {
            sock_close(session[index]);
            free(session[index]);
            session[index] = NULL;
        }
    }

    memset(session, 0, sizeof(session));
}
/*-------------------------------------------------------------------------
 * Function_name :fill_server_details_connect
 * Return Value : 0 for succsess -1 for failue
 * Fill the serverdetails in server address and connect to the server if it is TCP protocol
 *-------------------------------------------------------------------------*/
static int32_t create_session(sock_session *session)
{
    struct sockaddr *addr;
    struct sockaddr_in serv_addr;
    struct sockaddr_in6 serv6_addr;
    int32_t len_addr = 0;

    if (session->ip_mode == IPV4_MODE)
    {
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(session->port_no);
		if (inet_pton(AF_INET, session->server_ip, &serv_addr.sin_addr.s_addr) != 0)
        {
            LOG_ERR("Invalid IP address\n");
            return -1;
        }
        addr = (struct sockaddr *)&serv_addr;
        len_addr = (int32_t) sizeof(serv_addr);
    }
    else if (session->ip_mode == IPV6_MODE)
    {
        serv6_addr.sin_family = AF_INET6;
        serv6_addr.sin_port = htons(session->port_no);
        memcpy(&serv6_addr.sin_addr.s_addr, session->server_ip, sizeof(serv6_addr.sin_addr.s_addr));
        addr = (struct sockaddr *)&serv6_addr;
        len_addr = (int32_t)sizeof(serv6_addr);

    }
    else
    {
        return -1;
    }

    if (session->proto == TCP_PROTO)
    {
        if (qc_drv_net_connect(qc_api_get_qc_drv_context(), session->sock_id, (struct sockaddr *)addr, len_addr))
        {
            LOG_ERR("connection error on the socket: %d, session: %d\n", session->sock_id, num_session);
            return -1;
        }
    }
    qc_drv_net_fd_set(qc_api_get_qc_drv_context(), session->sock_id, &rset);
    session->session_id = num_session;
	#ifdef DEBUG_PRINT
	print_session_details(session);
	#endif
    num_session++;

    return 0;
}

/*-------------------------------------------------------------------------
 *  Function_name : sock_test_proto
 *  Return Value : 0 for succsess -1 for failure
 *  checks the protocol and creates the session ids
 *-------------------------------------------------------------------------*/
static int32_t sock_test_proto(uint32_t ip_mode, uint32_t protocol, char *ip, uint16_t port, uint32_t num_sock)
{
    uint32_t index = 0;
    int32_t sock_id;
    int32_t status = 0;

    for (index = 0; index < num_sock; index++)
    {
        session[index] = malloc(sizeof(sock_session));
        if (NULL == session[index])
        {
            LOG_ERR("Memory exhausted\n");
            return -1;
        }

        memset(session[index], 0, sizeof(sock_session));
        copy_sock_params(session[index], ip_mode, -1, protocol, ip, port, -1);
        sock_id = sock_open(session[index]);
        if (sock_id == -1)
        {
            return -1;
        }

        status = create_session(session[index]);
        if (status == -1)
        {
            return -1;
        }
    }

    LOG_AT("Total no of sockets created:%d\n", num_sock);
    return 0;
}

#ifdef DEBUG_PRINT
/*-------------------------------------------------------------------------
 * Function_name :  print_session_details - Prints the session details
 *-------------------------------------------------------------------------*/
static void print_session_details(sock_session *session)
{
    LOG_DEBUG("sock_handle : %d\n",  session->sock_id);
    LOG_DEBUG("protocol : %d\n",  session->proto);
    LOG_DEBUG("Port : %d\n",  session->port_no);
    LOG_DEBUG("Server ip: %s\n",  session->server_ip);
    LOG_DEBUG("Session id: %d\n",  session->session_id);
}
#endif
/*-------------------------------------------------------------------------
 * Function_name : Recv_Thread_Multisock
 * Thread for handling socket descriptors when message received from the server
 *-------------------------------------------------------------------------*/
static void Recv_Thread_Multisock(void *param)
{
    uint32_t index;
    int32_t ret;
    int32_t len_addr;
    int32_t sock_handle;
    struct sockaddr *addr = NULL;
    struct sockaddr_in serv_addr;
    struct sockaddr_in6 serv6_addr;
    sock_session *cur = NULL;

    recv_buf = malloc(RECV_BYTES);
    if (recv_buf == NULL)
    {
        LOG_DEBUG("sent_byes memory allocation failed\n");
        qurt_signal_set(&stop_signal, RECV_STOPPED_SIGNAL);
        qurt_thread_stop();
        return;
    }

    while (num_session)
    {
        qc_drv_net_fd_zero(qc_api_get_qc_drv_context(), &sockset);
        sockset = rset;

        ret = qc_drv_net_select(qc_api_get_qc_drv_context(), &sockset, NULL, NULL, 100);
        if (signal_set == STOP_SIGNAL)
        {
            LOG_AT("Exiting Recv thread\n");
            break;
        }
        if (ret == 0)
        {
            continue;
        }
        else
        {
            qurt_mutex_lock(&multiSockLock);
            for (index = 0; index < num_session; index++)
            {
                if (session[index] != NULL)
                {
                    if (qc_drv_net_fd_isset(qc_api_get_qc_drv_context(), session[index]->sock_id, &sockset))
                    {
                        cur = session[index];
                        sock_handle = cur->sock_id;

                        if (cur->proto == UDP_PROTO)
                        {
                            if (cur->ip_mode == IPV4_MODE)
                            {
                                addr = (struct sockaddr *)&serv_addr;
                                len_addr = sizeof(serv_addr);
                            }
                            else if (cur->ip_mode == IPV6_MODE)
                            {
                                addr = (struct sockaddr *)&serv6_addr;
                                len_addr = sizeof(serv6_addr);
                            }
                        }
                        ret = qc_drv_net_recvfrom(qc_api_get_qc_drv_context(), sock_handle, recv_buf, RECV_BYTES, 0, addr, &len_addr);
                        if (ret > 0)
                        {
                            cur->recv_bytes += ret;
                        }
                        else if (ret == -1)
                        {
                            LOG_ERR("Exiting Recv thread due to failure\n");
                            qurt_signal_set(&stop_signal, RECV_STOPPED_SIGNAL);
                            qurt_thread_stop();
                            return;
                        }
                    }
                }
            }
            qurt_mutex_unlock(&multiSockLock);
        }
    }

    LOG_AT("Exiting Recv thread due to stop\n");
    qurt_signal_set(&stop_signal, RECV_STOPPED_SIGNAL);
    qurt_thread_stop();

    return;
}

/*-------------------------------------------------------------------------
 * Functoin: Send_Thread_Multisock
 * Thread for sending messages to the server
 *-------------------------------------------------------------------------*/
static void Send_Thread_Multisock(void *param)
{
    uint32_t index;
    int32_t sock_handle;
    struct sockaddr *addr = NULL;
    struct sockaddr_in serv_addr;
    struct sockaddr_in6 serv6_addr;
    int32_t ret = 0;
    int32_t len = 0;
    uint32_t stop_sig_mask = 0; /* Possible signal masks for this event */
    char ch = CHAR_SEND;
    qurt_time_t timeout = 0;
    sock_session *cur = NULL;

    stop_sig_mask = STOP_SIGNAL;
    signal_set = 0;

    send_buf = malloc(num_send_bytes);
    if (send_buf == NULL)
    {
        LOG_ERR("sent_bytes memory allocation failed\n");
        qurt_signal_set(&stop_signal, SEND_STOPPED_SIGNAL);
        qurt_thread_stop();
        return;
    }

    memset(send_buf, ch, num_send_bytes);
    timeout = qurt_timer_convert_time_to_ticks(time_ms, QURT_TIME_MSEC); // By default 1 sec

    while (num_session)
    {
        ret = qurt_signal_wait_timed(&stop_signal, stop_sig_mask, (QURT_SIGNAL_ATTR_WAIT_ANY |
                    QURT_SIGNAL_ATTR_CLEAR_MASK), &signal_set, timeout);
        if (ret == QURT_EINVALID || signal_set == STOP_SIGNAL)
        {
            break;
        }

        qurt_mutex_lock(&multiSockLock);
        for (index = 0; index < num_session; index++)
        {
            cur = session[index];
            sock_handle = cur->sock_id;

            if (cur->proto == UDP_PROTO)
            {
                if (cur->ip_mode == IPV4_MODE)
                {
                    serv_addr.sin_family = AF_INET;
                    serv_addr.sin_port = htons(cur->port_no);
					if (inet_pton(AF_INET, cur->server_ip, &serv_addr.sin_addr.s_addr) != 0)
                    {
                        LOG_ERR("Invalid IP address\n");
                        return ;
                    }
                    addr = (struct sockaddr *)&serv_addr;
                    len = sizeof(serv_addr);
                }
                else if (cur->ip_mode == IPV6_MODE)
                {
                    serv6_addr.sin_family = AF_INET6;
                    serv6_addr.sin_port = htons(cur->port_no);
                    memcpy(&serv6_addr.sin_addr.s_addr, cur->server_ip, sizeof(serv6_addr.sin_addr.s_addr));
                    addr = (struct sockaddr *)&serv6_addr;
                    len = sizeof(serv6_addr);
                }
            }
            ret = qc_drv_net_sendto(qc_api_get_qc_drv_context(), sock_handle, send_buf, num_send_bytes, 0, addr, len);
            if (ret < 0)
            {
                LOG_ERR("sending failed on the session: %d\n", cur->session_id);
                qurt_signal_set(&stop_signal, SEND_STOPPED_SIGNAL);
                qurt_thread_stop();
                return;
            }
            else
            {
                cur->sent_bytes += ret;
            }
        }
        qurt_mutex_unlock(&multiSockLock);
    }

    LOG_AT("Stopped sending Thread\n");
    qurt_signal_set(&stop_signal, SEND_STOPPED_SIGNAL);
    qurt_thread_stop();

    return;
}

/*---------------------------------------------------------------------------
  - Create  thread
  --------------------------------------------------------------------------*/
static int32_t create_thread(const char *thr_name, int32_t thr_prio, uint32_t thr_stack_size,
        void (*thread_func)(void *param))
{
    uint32_t len = 0;
    qurt_thread_attr_t attr;

    qurt_thread_t thid = 0;
    qurt_thread_attr_init(&attr);
    qurt_thread_attr_set_name(&attr, thr_name);
    qurt_thread_attr_set_priority(&attr, thr_prio);
    qurt_thread_attr_set_stack_size(&attr, thr_stack_size);
    if( 0 != qurt_thread_create(&thid, &attr, thread_func, &len))
    {
        LOG_ERR("Thread creation is failed\n");
        return -1;
    }

    return 0;
}



/*----------------------------------------------------------------------------
 *Function_name: print_multi_sock_log_report
 *Prints session send and recv reports
 *--------------------------------------------------------------------------*/
static void print_multi_sock_log_report(void)
{
    int32_t index;

    LOG_INFO("== Session Info ==\n");
    for (index = 0; index < num_session; index++)
    {
        LOG_INFO("session Id: %d SndBytes: %u RcvBytes: %u\n",
                     session[index]->session_id, session[index]->sent_bytes,
                     session[index]->recv_bytes);
    }
    num_session = 0;

    return;
}

/*-------------------------------------------------------------------------
 * Function_name : multi_sock_create
 * Takes the input from command line process the multisocket creation
 *-------------------------------------------------------------------------*/
int multi_sock_create(uint16_t ip_mode, uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    char *protocol;
    uint16_t port;
    uint32_t num_sock;
    int32_t status;
    char *server_ip = NULL;

    num_sock = Parameter_List[1].Integer_Value;
    if (num_sock > MAX_NO_OF_SESSIONS)
    {
        LOG_AT("Max sock supported is %d\n", MAX_NO_OF_SESSIONS);
        return -1;
    }

    qurt_mutex_init(&multiSockLock);
    qurt_signal_init(&stop_signal);

    // Iniailize defaults
    time_ms = DEFAULT_TIME_DURATION;
    num_send_bytes = DEFAULT_SEND_BYTES;

    protocol = Parameter_List[0].String_Value;
    server_ip = Parameter_List[2].String_Value;
    port = Parameter_List[3].Integer_Value;
    if(Parameter_Count == 5)
    {
        time_ms = Parameter_List[4].Integer_Value;
    }
    if(Parameter_Count == 6)
    {
        num_send_bytes = Parameter_List[5].Integer_Value;
    }
    qc_drv_net_fd_zero(qc_api_get_qc_drv_context(), &rset);

    if (!(strcasecmp(protocol, "tcp")))
    {
        status = sock_test_proto(ip_mode, TCP_PROTO, server_ip, port, num_sock);
        if (status == -1)
        {
            LOG_ERR("Multi Sock Test failed\n");
            goto error;
        }
    }
    else if (!(strcasecmp(protocol, "udp")))
    {
        status = sock_test_proto(ip_mode, UDP_PROTO, server_ip, port, num_sock);
        if (status == -1)
        {
            goto error;
        }
    }
    else
    {
        LOG_ERR("Invalid protocol\n");
        goto error;
    }

    if( create_thread("send_thread", SEND_THREAD_PRIORITY, THRD_STACK_SIZE, Send_Thread_Multisock))
    {
        LOG_ERR("Failed to create Onboard Thread\n");
        goto error;
    }

    if( create_thread("recv_thread", RECV_THREAD_PRIORITY, THRD_STACK_SIZE, Recv_Thread_Multisock))
    {
        LOG_ERR("Failed to create Onboard Thread\n");
        goto error;
    }

    return 0;
error:
    process_multi_sock_close();
    qurt_signal_delete(&stop_signal);
    qurt_mutex_destroy(&multiSockLock);

    return -1;

}

/*-------------------------------------------------------------------------
 *Function_name :   multi_sock_close
 *Return value :   0
 *Does not the input from commandline to close sockets
 *-------------------------------------------------------------------------*/
QCLI_Command_Status_t multi_sock_close(void)
{
    uint32_t sig_mask = SEND_STOPPED_SIGNAL|RECV_STOPPED_SIGNAL;

    if (!num_session)
    {
        LOG_AT("No session to stop\n");
        return QCLI_STATUS_SUCCESS_E;
    }

    qurt_signal_set(&stop_signal, STOP_SIGNAL);
    qurt_signal_wait(&stop_signal, sig_mask, (QURT_SIGNAL_ATTR_WAIT_ALL |
                QURT_SIGNAL_ATTR_CLEAR_MASK));

    print_multi_sock_log_report();

    qurt_mutex_lock(&multiSockLock);
    process_multi_sock_close();
    qurt_mutex_unlock(&multiSockLock);

    qurt_mutex_destroy(&multiSockLock);
    qurt_signal_delete(&stop_signal);
    free(send_buf);
    free(recv_buf);

    return QCLI_STATUS_SUCCESS_E;
}

/*-------------------------------------------------------------------------
 * Function_name : multi_sockv4_create
 * Takes the input from command line process the multisocket creation
 *-------------------------------------------------------------------------*/
QCLI_Command_Status_t multi_sockv4_create(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if ((Parameter_Count < 4) || Parameter_List == NULL)
    {
        return QCLI_STATUS_USAGE_E;
    }
    if (num_session)
    {
        LOG_AT("Multi Sock test is in progress. Stop multisocket test and retry\n");
        return QCLI_STATUS_ERROR_E;
    }
    multi_sock_create(IPV4_MODE, Parameter_Count, Parameter_List);

    return  0;
}
/*-------------------------------------------------------------------------
 * Function_name : multi_sockv6_create
 * Takes the input from command line process the multisocket creation
 *-------------------------------------------------------------------------*/
QCLI_Command_Status_t multi_sockv6_create(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if ((Parameter_Count < 4) || Parameter_List == NULL)
    {
        return QCLI_STATUS_USAGE_E;
    }
    if (num_session)
    {
        LOG_AT("Multi Sock test is in progress. Stop multisocket test and retry\n");
        return QCLI_STATUS_ERROR_E;
    }
    multi_sock_create(IPV6_MODE, Parameter_Count, Parameter_List);
    return 0;

}

