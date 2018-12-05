/*
* Copyright (c) 2017 - 2018 Qualcomm Technologies, Inc.
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
#include "qapi_rtc.h"
#include "qapi_heap_status.h"
#include "qapi_reset.h"
#include "qapi_fatal_err.h"
#include "platform_demo.h"
#include "qapi/qapi_device_info.h"
#include "qapi_crypto.h"
#include "qurt_thread.h"
#include "qapi_tlmm.h"

/*
 * This file contains the command handlers for file management operations
 * on non-volatile memory like list, delete, read, write
 *
 */

QCLI_Group_Handle_t qcli_platform_handle; /* Handle for Fs Command Group. */


#define PLATFORM_DEMO_PRINTF(...) QCLI_Printf(qcli_platform_handle, __VA_ARGS__)
#define MAX_FRAG_ARR 100		/*Maximum number of malloc buffers*/
#define LARGE_BLOCK_SIZE 20000   
#define MAX_FRAG_SZ 2000		/* Default upper limit of Buffer Size*/

QCLI_Command_Status_t platform_demo_time(uint32_t parameters_count, QCLI_Parameter_t * parameters);
QCLI_Command_Status_t platform_demo_device_info(uint32_t parameters_count, QCLI_Parameter_t * parameters);
QCLI_Command_Status_t platform_demo_free(uint32_t parameters_count, QCLI_Parameter_t * parameters);
QCLI_Command_Status_t platform_demo_reset(uint32_t parameters_count, QCLI_Parameter_t * parameters);
QCLI_Command_Status_t platform_demo_watchdog_reset(uint32_t parameters_count, QCLI_Parameter_t * parameters);
QCLI_Command_Status_t platform_demo_malloc_test(uint32_t parameters_count, QCLI_Parameter_t * parameters);
QCLI_Command_Status_t platform_demo_reset_reason_test(uint32_t parameters_count, QCLI_Parameter_t * parameters);
QCLI_Command_Status_t platform_demo_gpio(uint32_t parameters_count, QCLI_Parameter_t * parameters);

const QCLI_Command_t platform_cmd_list[] =
{
    {platform_demo_time, false, "time", "\n", "get/set current time in Julian format\n"},
    {platform_demo_device_info, false, "info", "\n", "get device specific data\n"},
    {platform_demo_free, false, "free", "\n", "display the heap size and an approximation of free amount of heap bytes\n"},
    {platform_demo_reset, false, "reset", "\n", "reset the platform\n"},
    {platform_demo_watchdog_reset, false, "wdrst", "\n", "trigger watchdog reset\n"},
    {platform_demo_malloc_test, false, "utmalloc", "\n", "Malloc Unit Test \n"},
    {platform_demo_reset_reason_test, false, "reset_reason", "\n", "Display reset reason \n"},
	{platform_demo_gpio, false, "gpio", "\n", "Display and configure SoC GPIOs\n"},
};

const QCLI_Command_Group_t platform_cmd_group =
{
    "Platform",              /* Group_String: will display cmd prompt as "Platform> " */
    sizeof(platform_cmd_list)/sizeof(platform_cmd_list[0]),   /* Command_Count */
    platform_cmd_list        /* Command_List */
};


/*****************************************************************************
 * This function is used to register the Fs Command Group with QCLI.
 *****************************************************************************/
void Initialize_Platform_Demo(void)
{
    /* Attempt to reqister the Command Groups with the qcli framework.*/
    qcli_platform_handle = QCLI_Register_Command_Group(NULL, &platform_cmd_group);
    if (qcli_platform_handle)
    {
        QCLI_Printf(qcli_platform_handle, "Platform Registered\n");
    }

    return;
}

QCLI_Command_Status_t platform_demo_get_time(uint32_t parameters_count, QCLI_Parameter_t * parameters)
{
    qapi_Time_t tm;
    int status;

    status = qapi_Core_RTC_Get(&tm);
    if ( 0 != status ) {
        PLATFORM_DEMO_PRINTF("Failed on a call to qapi_Time_Get(), status=%d\r\n", status);
        PLATFORM_DEMO_PRINTF("Please note that this is likely happened because the time was not set\r\n", status);
        return QCLI_STATUS_ERROR_E;
    }

    PLATFORM_DEMO_PRINTF("Julian Time: \r\n");
    PLATFORM_DEMO_PRINTF("year = %d\r\n", tm.year);
    PLATFORM_DEMO_PRINTF("month = %d\r\n", tm.month);
    PLATFORM_DEMO_PRINTF("day = %d\r\n", tm.day);
    PLATFORM_DEMO_PRINTF("hour = %d\r\n", tm.hour);
    PLATFORM_DEMO_PRINTF("minute = %d\r\n", tm.minute);
    PLATFORM_DEMO_PRINTF("second = %d\r\n", tm.second);
    PLATFORM_DEMO_PRINTF("day_Of_Week = %d\r\n", tm.day_Of_Week);

    return QCLI_STATUS_SUCCESS_E;
}

void print_usage_set_time()
{
    PLATFORM_DEMO_PRINTF("Usage: time set year month day hour minute second day_Of_Week\r\n");
    PLATFORM_DEMO_PRINTF("\t\t year: Year [1980 through 2100]\r\n");
    PLATFORM_DEMO_PRINTF("\t\t month: Month of year [1 through 12]\r\n");
    PLATFORM_DEMO_PRINTF("\t\t day: Day of month [1 through 31]\r\n");
    PLATFORM_DEMO_PRINTF("\t\t hour: Hour of day [0 through 23]\r\n");
    PLATFORM_DEMO_PRINTF("\t\t minute: Minute of hour [0 through 59]\r\n");
    PLATFORM_DEMO_PRINTF("\t\t second: Second of minute [0 through 59]\r\n");
    PLATFORM_DEMO_PRINTF("\t\t day_Of_Weak: Day of the week [0 through 6] (corresponding to Monday through Sunday)\r\n");
}


QCLI_Command_Status_t platform_demo_set_time(uint32_t parameters_count, QCLI_Parameter_t * parameters)
{
    qapi_Time_t tm;
    int status;

    if ( parameters_count != 7 ) {
        PLATFORM_DEMO_PRINTF("Invalid number of arguments\r\n");
        goto platform_demo_set_time_on_error;
    }

    // check year
    if ( 1 &&
        (parameters[0].Integer_Is_Valid) &&
        (parameters[0].Integer_Value >= 1980) &&
        (parameters[0].Integer_Value <= 2100)
        )
    {
        tm.year = parameters[0].Integer_Value;
    }
    else
    {
        PLATFORM_DEMO_PRINTF("Invalid year\r\n");
        goto platform_demo_set_time_on_error;
    }

    // check month
    if ( 1 &&
        (parameters[1].Integer_Is_Valid) &&
        (parameters[1].Integer_Value >= 1) &&
        (parameters[1].Integer_Value <= 12)
       )
    {
        tm.month = parameters[1].Integer_Value;
    }
    else
    {
        PLATFORM_DEMO_PRINTF("Invalid month\r\n");
        goto platform_demo_set_time_on_error;
    }

    // check day
    if ( 1 &&
        (parameters[2].Integer_Is_Valid) &&
        (parameters[2].Integer_Value >= 1) &&
        (parameters[2].Integer_Value <= 31)
       )
    {
        tm.day = parameters[2].Integer_Value;
    }
    else
    {
        PLATFORM_DEMO_PRINTF("Invalid day\r\n");
        goto platform_demo_set_time_on_error;
    }

    // check hour
    if ( 1 &&
        (parameters[3].Integer_Is_Valid) &&
        (parameters[3].Integer_Value >= 0) &&
        (parameters[3].Integer_Value <= 23)
       )
    {
        tm.hour = parameters[3].Integer_Value;
    }
    else
    {
        PLATFORM_DEMO_PRINTF("Invalid hour\r\n");
        goto platform_demo_set_time_on_error;
    }

    // check minute
    if ( 1 &&
        (parameters[4].Integer_Is_Valid) &&
        (parameters[4].Integer_Value >= 0) &&
        (parameters[4].Integer_Value <= 59)
       )
    {
        tm.minute = parameters[4].Integer_Value;
    }
    else
    {
        PLATFORM_DEMO_PRINTF("Invalid minute\r\n");
        goto platform_demo_set_time_on_error;
    }

    // check second
    if ( 1 &&
        (parameters[5].Integer_Is_Valid) &&
        (parameters[5].Integer_Value >= 0) &&
        (parameters[5].Integer_Value <= 59)
       )
    {
        tm.second = parameters[5].Integer_Value;
    }
    else
    {
        PLATFORM_DEMO_PRINTF("Invalid second\r\n");
        goto platform_demo_set_time_on_error;
    }

    // check day of the week
    if ( 1 &&
        (parameters[6].Integer_Is_Valid) &&
        (parameters[6].Integer_Value >= 0) &&
        (parameters[6].Integer_Value <= 6)
       )
    {
        tm.day_Of_Week = parameters[6].Integer_Value;
    }
    else
    {
        PLATFORM_DEMO_PRINTF("Invalid day_Of_Week\r\n");
        goto platform_demo_set_time_on_error;
    }

    status = qapi_Core_RTC_Set(&tm);
    if (0 != status ) {
        PLATFORM_DEMO_PRINTF("Failed on a call to qapi_Time_Set(), status=%d\r\n", status);
        goto platform_demo_set_time_on_error;
    }

    return QCLI_STATUS_SUCCESS_E;

platform_demo_set_time_on_error:
    print_usage_set_time();
    return QCLI_STATUS_ERROR_E;
}


QCLI_Command_Status_t platform_demo_time(uint32_t parameters_count, QCLI_Parameter_t * parameters)
{
    if ( parameters_count < 1 ) {
        PLATFORM_DEMO_PRINTF("Invalid number of arguments\r\n");
        goto platform_demo_time_on_error;
    }

    if ( 0 == strcmp(parameters[0].String_Value, "get") ) {
        return platform_demo_get_time(parameters_count-1, &parameters[1]);
    }
    else if ( 0 == strcmp(parameters[0].String_Value, "set") ) {
        return platform_demo_set_time(parameters_count-1, &parameters[1]);
    }

platform_demo_time_on_error:
    PLATFORM_DEMO_PRINTF("Usage: time get/set <params>\r\n");
    return QCLI_STATUS_ERROR_E;
}


void print_bytes(void * buffer, size_t buffer_size)
{
    int i;
    uint8_t * p_buffer = (uint8_t *) buffer;
    for ( i = 0; i < buffer_size; i++ ) {
        if ( (0 != i) && (0 == i % 16) ) {
            PLATFORM_DEMO_PRINTF("\r\n");
        }
        PLATFORM_DEMO_PRINTF("%02x ", p_buffer[i]);
    }
    PLATFORM_DEMO_PRINTF("\r\n");
}

qapi_Device_Specific_Data_t helper_convert_string_to_device_specific_data_type(const char * p_device_specific_data_type)
{
    qapi_Device_Specific_Data_t device_specific_data_type = QAPI_DEVICE_DATA_INVALID_E;

    if ( 0 == strcmp(p_device_specific_data_type, "make") )
    {
        device_specific_data_type = QAPI_DEVICE_DATA_MAKE_E;
    }
    else if ( 0 == strcmp(p_device_specific_data_type, "model") )
    {
        device_specific_data_type = QAPI_DEVICE_DATA_MODEL_E;
    }
    else if ( 0 == strcmp(p_device_specific_data_type, "version") )
    {
        device_specific_data_type = QAPI_DEVICE_DATA_VERSION_E;
    }
    else if ( 0 == strcmp(p_device_specific_data_type, "oem") )
    {
        device_specific_data_type = QAPI_DEVICE_DATA_OEM_ID_E;
    }
    else if ( 0 == strcmp(p_device_specific_data_type, "serial") )
    {
        device_specific_data_type = QAPI_DEVICE_DATA_SERIAL_NUMBER_E;
    }
    else if ( 0 == strcmp(p_device_specific_data_type, "security") )
    {
        device_specific_data_type = QAPI_DEVICE_DATA_SECURITY_STATE_E;
    }
    else if ( 0 == strcmp(p_device_specific_data_type, "soc") )
    {
        device_specific_data_type = QAPI_DEVICE_DATA_SOC_HW_VERSION_E;
    }

    return device_specific_data_type;
}

QCLI_Command_Status_t platform_demo_device_info(uint32_t parameters_count, QCLI_Parameter_t * parameters)
{
    const char * p_command = 0;
    const char * p_device_specific_data_type = 0;
    qapi_Device_Specific_Data_t device_specific_data_type;
    qapi_Status_t status;
    uint8_t * p_buffer = 0;
    size_t buffer_size = 0;

    if ( parameters_count < 2 ) {
        PLATFORM_DEMO_PRINTF("Invalid number of arguments\r\n");
        goto platform_demo_device_info_on_error;
    }

    p_command = parameters[0].String_Value;
    if ( 0 != strcmp(p_command, "get") ) {
        PLATFORM_DEMO_PRINTF("Invalid info command, the only valid command is get\r\n");
        goto platform_demo_device_info_on_error;
    }

    p_device_specific_data_type = parameters[1].String_Value;
    device_specific_data_type =
        helper_convert_string_to_device_specific_data_type(p_device_specific_data_type);


    status =
        qapi_Core_Device_Info_Get_Specific_Data(
            device_specific_data_type,
            (void **) &p_buffer,
            &buffer_size
            );
    if ( 0 == status )
    {
        PLATFORM_DEMO_PRINTF("Data (%d bytes): ", buffer_size);
        print_bytes(p_buffer, buffer_size);
        PLATFORM_DEMO_PRINTF("\r\n");
        if ( p_buffer ) {
            free(p_buffer);
        }
    }
    else
    {
        PLATFORM_DEMO_PRINTF("Failed to get device specific info\r\n");
        goto platform_demo_device_info_on_error;
    }

    return QCLI_STATUS_SUCCESS_E;

platform_demo_device_info_on_error:
    PLATFORM_DEMO_PRINTF("Usage: info get <data_type>\r\n");
    PLATFORM_DEMO_PRINTF("\t<data_type>: could be one of the following: \r\n");
    PLATFORM_DEMO_PRINTF("\t\t make - make of the device\r\n");
    PLATFORM_DEMO_PRINTF("\t\t model - model of the device\r\n");
    PLATFORM_DEMO_PRINTF("\t\t version - version of the device\r\n");
    PLATFORM_DEMO_PRINTF("\t\t oem - OEM ID of the device\r\n");
    PLATFORM_DEMO_PRINTF("\t\t serial - serial number of the device\r\n");
    PLATFORM_DEMO_PRINTF("\t\t security - security state of the device\r\n");
    PLATFORM_DEMO_PRINTF("\t\t soc - SoC HW version of the device\r\n");
    return QCLI_STATUS_ERROR_E;
}

QCLI_Command_Status_t platform_demo_free(uint32_t parameters_count, QCLI_Parameter_t * parameters)
{
    uint32_t total, free;

    if(qapi_Heap_Status(&total, &free) != QAPI_OK) {
        PLATFORM_DEMO_PRINTF("Error getting heap status\n");
        return QCLI_STATUS_ERROR_E;
    }

    PLATFORM_DEMO_PRINTF("           total       used       free\n");
    PLATFORM_DEMO_PRINTF("Heap:   %8d   %8d   %8d\n", total, total-free, free);

    return QCLI_STATUS_SUCCESS_E;
}


QCLI_Command_Status_t platform_demo_reset(uint32_t parameters_count, QCLI_Parameter_t * parameters)
{
	qapi_System_Reset();
	return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t platform_demo_watchdog_reset(uint32_t parameters_count, QCLI_Parameter_t * parameters)
{
    //trigger watchdog rereset
    QAPI_FATAL_ERR(0,0,0);
	
    return QCLI_STATUS_SUCCESS_E;
}


QCLI_Command_Status_t platform_demo_malloc_test(uint32_t parameters_count, QCLI_Parameter_t * parameters)
{
	int i, sz=0;
	uint32_t total,remain;
	int* test_arr[MAX_FRAG_ARR];
	int max_sz = MAX_FRAG_SZ;
	
	 if ( parameters_count > 1 ) {
        max_sz = parameters[0].Integer_Value;
    }
	
	PLATFORM_DEMO_PRINTF("Fragment upper limit %d\n",max_sz);
	
	/*Allocate a big chunk to get closer to Heap upper limit*/
	int* large_block = malloc(LARGE_BLOCK_SIZE);
	
	if(large_block == NULL){
		PLATFORM_DEMO_PRINTF("Large Block allocation failed\n");
	} else {
		PLATFORM_DEMO_PRINTF("Allocated %d\n",LARGE_BLOCK_SIZE);
	}
	
	for (i=0;i<MAX_FRAG_ARR;i++)
	{
		/*Try to allocate MAX_FRAG_ARR buffers of random sizes*/
		sz=0;
		qapi_Crypto_Random_Get(&sz,2);
		sz = sz%max_sz;
		PLATFORM_DEMO_PRINTF("Trying to allocate %d bytes\n",sz);
		test_arr[i] = malloc(sz);
		if(test_arr[i] == NULL){
			qapi_Heap_Status(&total, &remain);
			PLATFORM_DEMO_PRINTF("\nMalloc failed, free sz %d\n",remain);
		}
		else
			PLATFORM_DEMO_PRINTF("Allocated %d\n",sz);
		qurt_thread_sleep(100);
		
		if(i > 0 && i%7 == 0){
			if(test_arr[i-4]){
				PLATFORM_DEMO_PRINTF("\nFreeing index %d\n",i-4);
				free(test_arr[i-4]);
				test_arr[i-4] = NULL;
			}
		}
	}
	
	for (i=MAX_FRAG_ARR;i>0;i--)
	{
		if(test_arr[i-1]){
			free(test_arr[i-1]);
			test_arr[i-1] = NULL;
		}
	}	
	free(large_block);
	
    return QCLI_STATUS_SUCCESS_E;
}


QCLI_Command_Status_t platform_demo_reset_reason_test(uint32_t parameters_count, QCLI_Parameter_t * parameters)
{
    qapi_System_Reset_Result_t reason;

    if (qapi_System_Get_BootReason(&reason) == QAPI_OK)
    {
        switch (reason)
        {
        case BOOT_REASON_PON_COLD_BOOT:
            PLATFORM_DEMO_PRINTF("\nReset Reason is Hardware cold reset \n");
            break;
        case BOOT_REASON_WATCHDOG_BOOT:
            PLATFORM_DEMO_PRINTF("\nReset Reason is Watchdog reset \n");
            break;
        case BOOT_REASON_SW_COLD_BOOT:
            PLATFORM_DEMO_PRINTF("\nReset Reason is Software cold reset \n");
            break;
        default:
            PLATFORM_DEMO_PRINTF("\nReset Reason UNKNOWN!! \n");
       }
    }
    else
    {
        PLATFORM_DEMO_PRINTF("\nReset Reason FAILED!! \n");
    }

    return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t platform_demo_gpio(uint32_t Parameter_Count, QCLI_Parameter_t * Parameter_List)
{
    char *p_command = NULL;
    uint32_t total_gpios;
    uint32_t parameter_index;
    qapi_TLMM_Config_t gpio_cfg;
    qapi_GPIO_ID_t gpio_id;
    uint8_t gpio_id_set = 0;
    QCLI_Command_Status_t ret_val = QCLI_STATUS_SUCCESS_E;
    qapi_GPIO_Value_t out_val_p = QAPI_GPIO_INVALID_VALUE_E;
    static const char *strength_str[] =
    { "1.6", "2.7", "4.0" };
    static const char *pull_str[] =
    { "None", "Down", "Up" };
    static const char *dir_str[] =
    { "Input", "Output" };

    if (qapi_TLMM_Get_Total_Gpios(&total_gpios) != QAPI_OK)
        return QCLI_STATUS_ERROR_E;

    /* Convert to index */
    total_gpios--;

    if (Parameter_Count < 1)
    {
        PLATFORM_DEMO_PRINTF("Error: Pin number required\n");
        goto gpio_demo_usage;
    }

    if (!Parameter_List[0].Integer_Is_Valid)
    {
        p_command = Parameter_List[0].String_Value;

        if (strcmp(p_command, "show") == 0)
        {
            uint32_t i;

            PLATFORM_DEMO_PRINTF(
                    "\tID\tFunc\tStr\tDir\tPull\tInval\tOutval\n");
            PLATFORM_DEMO_PRINTF(
                    "\t-----------------------------------------------------\n");

            for (i = 0; i <= total_gpios; i++)
            {
                gpio_cfg.pin = i;

                // Get config
                if(qapi_TLMM_Get_Gpio_Config(&gpio_cfg) != QAPI_OK)
                {
                    PLATFORM_DEMO_PRINTF("\t%d\t**FAIL TO READ**\n", i);
                }
                else
                {
                    qapi_GPIO_Value_t out_val;
                    qapi_GPIO_Value_t in_val = (qapi_GPIO_Value_t)0xB; // For busy

                    // Get ID
                    if (qapi_TLMM_Get_Gpio_ID(&gpio_cfg, &gpio_id) == QAPI_OK)
                    {
                        in_val = qapi_TLMM_Read_Gpio(gpio_id, gpio_cfg.pin);

                        // Release
                        qapi_TLMM_Release_Gpio_ID(&gpio_cfg, gpio_id);
                    }

                    out_val = qapi_TLMM_Get_Gpio_Outval(gpio_cfg.pin);

                    // Print config
                    PLATFORM_DEMO_PRINTF("\t%d\t%d\t%smA\t%s\t%s\t%X\t%x\n", i,
                            gpio_cfg.func, strength_str[gpio_cfg.drive],
                            dir_str[gpio_cfg.dir], pull_str[gpio_cfg.pull],
                            in_val, out_val);
                }
            }

            return QCLI_STATUS_SUCCESS_E;
        }
        else
        {
            goto gpio_demo_usage;
        }
    }

    memset(&gpio_cfg, 0, sizeof(qapi_TLMM_Config_t));
    gpio_cfg.pin = Parameter_List[0].Integer_Value;

    if (gpio_cfg.pin > total_gpios)
    {
        PLATFORM_DEMO_PRINTF("Error: Valid GPIO range 0-%d\n", total_gpios);
        return QCLI_STATUS_ERROR_E;
    }

    if (qapi_TLMM_Get_Gpio_ID(&gpio_cfg, &gpio_id) == QAPI_OK)
    {
        gpio_id_set = 1;
    }

    // Get config
    if(qapi_TLMM_Get_Gpio_Config(&gpio_cfg) != QAPI_OK)
    {
        PLATFORM_DEMO_PRINTF("Error: Failed reading GPIO %d configuration\n", gpio_cfg.pin);
    }

    if (Parameter_Count == 1) // Just print the GPIO configuration
    {
        qapi_GPIO_Value_t out_val;
        qapi_GPIO_Value_t in_val = (qapi_GPIO_Value_t)0xB;
        out_val = qapi_TLMM_Get_Gpio_Outval(gpio_cfg.pin);

        if(gpio_id_set)
            in_val = qapi_TLMM_Read_Gpio(gpio_id, gpio_cfg.pin);

        // Print config
        PLATFORM_DEMO_PRINTF("Function: %d\nStrength: %smA\nDirection: %s\nPull: %s\nInval: %X\nOutval: %d\n",
                gpio_cfg.func, strength_str[gpio_cfg.drive],
                dir_str[gpio_cfg.dir], pull_str[gpio_cfg.pull],
                in_val, out_val);

        goto gpio_demo_end;
    }
    else if (!gpio_id_set)
    {
        PLATFORM_DEMO_PRINTF("Error: GPIO is busy and cannot be modified\n");
        return QCLI_STATUS_ERROR_E;
    }

    for (parameter_index = 1; parameter_index < Parameter_Count;
            parameter_index++)
    {
        if (0 == strcmp((char*) Parameter_List[parameter_index].String_Value, "func"))
        {
            parameter_index++;
            if (parameter_index == Parameter_Count)
            {
                PLATFORM_DEMO_PRINTF("ERROR: Missing value for %s\n",
                        Parameter_List[parameter_index - 1].String_Value);
                goto gpio_demo_usage;
            }

            if (!Parameter_List[parameter_index].Integer_Is_Valid)
            {
                goto gpio_demo_usage;
            }

            gpio_cfg.func =
                    (uint32_t) Parameter_List[parameter_index].Integer_Value;
            if (gpio_cfg.func > 15) // 0..15
            {
                goto gpio_demo_usage;
            }
        }
        else if (0 == strcmp((char*) Parameter_List[parameter_index].String_Value, "strength"))
        {
            parameter_index++;
            if (parameter_index == Parameter_Count)
            {
                PLATFORM_DEMO_PRINTF("ERROR: Missing value for %s\n",
                        Parameter_List[parameter_index - 1].String_Value);
                goto gpio_demo_usage;
            }

            if (!Parameter_List[parameter_index].Integer_Is_Valid)
            {
                goto gpio_demo_usage;
            }

            gpio_cfg.drive =
                    (uint32_t) Parameter_List[parameter_index].Integer_Value;
            if (gpio_cfg.drive > 2) // 0..2
            {
                goto gpio_demo_usage;
            }
        }
        else if (0 == strcmp((char*) Parameter_List[parameter_index].String_Value, "dir"))
        {
            parameter_index++;
            if (parameter_index == Parameter_Count)
            {
                PLATFORM_DEMO_PRINTF("ERROR: Missing value for %s\n",
                        Parameter_List[parameter_index - 1].String_Value);
                goto gpio_demo_usage;
            }

            if (!Parameter_List[parameter_index].Integer_Is_Valid)
            {
                goto gpio_demo_usage;
            }

            gpio_cfg.dir =
                    (uint32_t) Parameter_List[parameter_index].Integer_Value;
            if (gpio_cfg.dir > 1) // 0..1
            {
                goto gpio_demo_usage;
            }
        }
        else if (0 == strcmp((char*) Parameter_List[parameter_index].String_Value, "pull"))
        {
            parameter_index++;
            if (parameter_index == Parameter_Count)
            {
                PLATFORM_DEMO_PRINTF("ERROR: Missing value for %s\n",
                        Parameter_List[parameter_index - 1].String_Value);
                goto gpio_demo_usage;
            }

            if (!Parameter_List[parameter_index].Integer_Is_Valid)
            {
                goto gpio_demo_usage;
            }

            gpio_cfg.pull =
                    (uint32_t) Parameter_List[parameter_index].Integer_Value;
            if (gpio_cfg.pull > 2) // 0..2
            {
                goto gpio_demo_usage;
            }
        }
        else if (0 == strcmp((char*) Parameter_List[parameter_index].String_Value, "outval"))
        {
            parameter_index++;
            if (parameter_index == Parameter_Count)
            {
                PLATFORM_DEMO_PRINTF("ERROR: Missing value for %s\n",
                        Parameter_List[parameter_index - 1].String_Value);
                goto gpio_demo_usage;
            }

            if (!Parameter_List[parameter_index].Integer_Is_Valid)
            {
                goto gpio_demo_usage;
            }

            out_val_p =
                    (qapi_GPIO_Value_t) Parameter_List[parameter_index].Integer_Value;
            if (out_val_p > 1) // 0..1
            {
                goto gpio_demo_usage;
            }
        }
        else
        {
            PLATFORM_DEMO_PRINTF("ERROR: Unknown option (%s)\n",
                    (char* )Parameter_List[parameter_index].String_Value);
            goto gpio_demo_usage;
        }
    }

    if (qapi_TLMM_Config_Gpio(gpio_id, &gpio_cfg) != QAPI_OK)
    {
        PLATFORM_DEMO_PRINTF("Error: Cannot set the GPIO configuration\n");
        ret_val = QCLI_STATUS_ERROR_E;
    }

    if(out_val_p != QAPI_GPIO_INVALID_VALUE_E)
    {
        if(qapi_TLMM_Drive_Gpio(gpio_id, gpio_cfg.pin, out_val_p) != QAPI_OK)
        {
            PLATFORM_DEMO_PRINTF("Error: Cannot set the GPIO output value\n");
            ret_val = QCLI_STATUS_ERROR_E;
        }
    }

    gpio_demo_end:
    if (gpio_id_set)
    {
        qapi_TLMM_Release_Gpio_ID(&gpio_cfg, gpio_id);
    }

    return ret_val;

    gpio_demo_usage:
    PLATFORM_DEMO_PRINTF(
            "Usage:\n\tgpio <show | GPIO_ID: 0..%d> [func <0..15>] [strength <0..2>] [dir <0..1>] [pull <0..2>] [outval <0..1>]\n",
            total_gpios);
    PLATFORM_DEMO_PRINTF("\tstrength: 0:1.6mA, 1:2.7mA ,2:4.0mA\n");
    PLATFORM_DEMO_PRINTF("\tdir: 0:input, 1:output\n");
    PLATFORM_DEMO_PRINTF("\tpull: 0:no pull, 1:pull down, 2:pull up\n");

    if (gpio_id_set)
    {
        qapi_TLMM_Release_Gpio_ID(&gpio_cfg, gpio_id);
    }

    return QCLI_STATUS_USAGE_E;
}
