/*
 * Copyright (c) 2015-2018 Qualcomm Technologies, Inc.
 * 2015-2016 Qualcomm Atheros, Inc.
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

#include <stdio.h>
#include <qurt_signal.h>
#include <qapi/qurt_thread.h>
#include <qapi/qapi_status.h>
#include <qapi/qapi_pwm.h>
#include <qapi/qapi_timer.h>
#include <qapi/qapi_tlmm.h>
#include <stdint.h>
#include <qcli.h>
#include <qcli_api.h>
#include <qapi_pwm.h>
#include <qurt_timer.h>

#include "pwm_demo.h"

#define PWM_wait(msec)    do { \
                              qurt_time_t qtime;\
                              qtime = qurt_timer_convert_time_to_ticks(msec, QURT_TIME_MSEC);\
                              qurt_thread_sleep(qtime);\
                          } while (0)

#define PWM_wait_us_no_switch(x)  busywait(x)

qapi_PWM_Config_t pwm_config = {
   7500,
   2000,
   2000,
   0,
   QAPI_PWM_SOURCE_CLK_NORMAL_MODE_E,
};

qapi_PWM_Handle_t  handleArray[2] = {NULL, NULL};

int32_t pwm_driver_test( uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List )
{
	qapi_Status_t status;
	qapi_PWM_Handle_t   handle;
	uint32_t   channel = QAPI_PWM_CHANNEL_7_E;
	uint32_t   wait_time_ms = 1000;
	
	if (Parameter_Count >= 1)
	{
		pwm_config.freq = Parameter_List->Integer_Value;
		Parameter_List++;
	}

	if (Parameter_Count >= 2)
	{
		pwm_config.duty = Parameter_List->Integer_Value;
		Parameter_List++;
	}

	if (Parameter_Count >= 3)
	{
		pwm_config.phase = Parameter_List->Integer_Value;
		Parameter_List++;
	}

	if (Parameter_Count >= 4)
	{
		channel = Parameter_List->Integer_Value;
		Parameter_List++;
	}

	if (Parameter_Count >= 5)
	{
		wait_time_ms = Parameter_List->Integer_Value * 1000;
		Parameter_List++;
	}
	
	status = qapi_PWM_Channel_Open(channel, &handle);
	if (status != QAPI_OK)
           return -1;

	status = qapi_PWM_Channel_Set(handle, &pwm_config);
	if (status != QAPI_OK)
           return -2;

	handleArray[0] = handle;
	
	status = qapi_PWM_Enable(handleArray, 1, 1);
	if (status != QAPI_OK)
           return -3;

	PWM_wait(wait_time_ms);
		
	qapi_PWM_Channel_Close(handle);
        
        return 0;
}
