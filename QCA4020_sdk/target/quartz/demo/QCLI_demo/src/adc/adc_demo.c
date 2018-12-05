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
#include <string.h>
#include <stdlib.h>
#include <qcli.h>
#include <qcli_api.h>
#include <qapi_wlan.h>
#include "adc_demo.h"

extern QCLI_Group_Handle_t qcli_peripherals_group;              /* Handle for our peripherals subgroup. */
QCLI_Group_Handle_t qcli_adc_group;              /* Handle for our QCLI Command Group. */

QCLI_Command_Status_t adc_test(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);

const QCLI_Command_t adc_cmd_list[] =
{
   // cmd_function        start_thread          cmd_string               usage_string                   description
   { adc_test,    false,          "read",                     "",                    "read"   },
};

const QCLI_Command_Group_t adc_cmd_group =
{
    "ADC",
    (sizeof(adc_cmd_list) / sizeof(adc_cmd_list[0])),
    adc_cmd_list
};

   /* This function is used to register the wlan Command Group with    */
   /* QCLI.                                                             */
void Initialize_ADC_Demo(void)
{
   /* Attempt to reqister the Command Groups with the qcli framework.*/
   qcli_adc_group = QCLI_Register_Command_Group(qcli_peripherals_group, &adc_cmd_group);
   if(qcli_adc_group)
   {
      QCLI_Printf(qcli_adc_group, "ADC Registered \n");
   }
}

QCLI_Command_Status_t adc_test(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    uint32_t i;
    int32_t  adc_driver_test( void *pvParameters );
    qapi_ADC_Read_Result_t *result;
    qapi_ADC_Range_t  *range;
    int32_t  status;
    
    status = adc_driver_test(Parameter_List);
    if (status != 0)
    {
	QCLI_Printf(qcli_adc_group, "ADC fails\n");
        return QCLI_STATUS_ERROR_E;
    }
    for (i=0; i < MAX_CHANNELS; i++)
	{
		result = &chan_result[i].chan_result;
		range = &chan_result[i].range;
		
		QCLI_Printf(qcli_adc_group, "ADC[%i] = %duV,  %d.%03dmV,  %d.%03dV  Range=[%d.%03dV, %d.%03dV]\n", i, result->microvolts, 
										result->microvolts/1000, result->microvolts%1000,
										result->microvolts/1000000, (result->microvolts%1000000) / 1000,
										range->min_uv/1000000, (range->min_uv%1000000)/1000, 
										range->max_uv/1000000, (range->max_uv%1000000) / 1000);
		
	}
	
    return QCLI_STATUS_SUCCESS_E;
}
