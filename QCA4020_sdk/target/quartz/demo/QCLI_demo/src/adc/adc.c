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
#include "stdint.h"
#include <qcli.h>
#include <qcli_api.h>
#include "qapi/qurt_signal.h"
#include "qapi/qurt_thread.h"
#include "qapi/qapi_status.h"
#include "qapi/qapi_adc.h"
#include "adc_demo.h"


adc_result_t chan_result[MAX_CHANNELS];

qapi_ADC_Config_t adc_config = {
   0xFF,  /**< Mask of which channels to enable. */
   10,     /**< Continious mode measurement period in microseconds. */
   QAPI_ADC_POWER_MODE_NORMAL_E,  /**< Power mode. */	
};

int32_t adc_driver_test( void *pvParameters )
{
	qapi_Status_t status;
	uint32_t     i;
	qapi_ADC_Handle_t handle;
	uint32_t    attributes, num_channels;
	qbool_t     keep_enabled = 1;
	
	attributes = QAPI_ADC_ATTRIBUTE_NONE;	
	status = qapi_ADC_Open(&handle, attributes);
	qapi_ADC_Set_Config(handle, &adc_config);

	status = qapi_ADC_Get_Num_Channels(handle, &num_channels);
	if (QAPI_OK != status) {		
		return -1;
	}
	
	for (i=0; i < num_channels && i < MAX_CHANNELS; i++)
	{
		status = qapi_ADC_Get_Range( handle, i, &chan_result[i].range );
		if (QAPI_OK != status) {		
			return -2;
		}
		
		status = qapi_ADC_Read_Channel(handle, i, &chan_result[i].chan_result);
		if (QAPI_OK != status) {		
			return -3;
		}		
	}

	qapi_ADC_Close(handle, keep_enabled);
        
        return 0;
}

void AdcSingleShotConv() 
{ 
  qapi_Status_t adcReturnVal;
  qapi_ADC_Handle_t handle = NULL;
  uint8 Attributes = QAPI_ADC_ATTRIBUTE_NONE;
  uint32_t  uNum_of_chan, uChan;
  qapi_ADC_Range_t  range;
			
  adcReturnVal = qapi_ADC_Open(&handle, Attributes);
  if (QAPI_OK != adcReturnVal) {		
    return;
  }
	
  adcReturnVal = qapi_ADC_Get_Num_Channels(handle, &uNum_of_chan); 
  if ( (QAPI_OK != adcReturnVal) || (0 == uNum_of_chan) ) {		
    return ;
  }
		
  for(uChan = 0; uChan < uNum_of_chan; uChan++) {
    adcReturnVal =  qapi_ADC_Get_Range( handle, uChan, &range );
    if (QAPI_OK != adcReturnVal) {		
      return ;
    }
		
    adcReturnVal =  qapi_ADC_Read_Channel( handle, uChan, &chan_result[uChan].chan_result );
    if (QAPI_OK != adcReturnVal) {		
      return ;
    }
		
#if 0  
    eAdcPowerVal = ADCGetActivePowerState();
    if ( QAPI_ADC_ATTRIBUTE_BUFFERING == Attributes) {			
      if ( ADC_TEST_PS_RET != eAdcPowerVal ) {
        return ;
      }
    }
    else {
      if ( ADC_TEST_PS_OFF != eAdcPowerVal) {
        return ;
      }
    }			
#endif

  }
		
  adcReturnVal = qapi_ADC_Close(handle, (qbool_t)false);
  if (QAPI_OK != adcReturnVal) {		
    return;
  }	
		
  return;
}
