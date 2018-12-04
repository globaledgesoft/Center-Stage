/*=============================================================================

                               PLATFORM_OEM_INIT_SOM

GENERAL DESCRIPTION
  This file contains the initial operations Quartz M4 for SOM mode.

EXTERNALIZED FUNCTIONS

INITIALIZATION AND SEQUENCING REQUIREMENTS
  None

  $Header: //components/rel/core.ioe/1.0/platform/apps_proc/src/platform_oem_som.c#4 $
  $Author: pwbldsvc $
  $DateTime: 2018/05/30 03:44:40 $
=============================================================================*/

/*
 * Copyright (c) 2015-2018 Qualcomm Technologies, Inc.
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

/*==================================================================================================
                                           INCLUDES
==================================================================================================*/
#include <stdint.h>
#include "platform_oem.h"


/*TEST ONLY - This function need to be defined by OEM Application */
extern void som_app_init(void);

/* TEST ONLY */
extern void som_app_entry_internal(void);

/*==================================================================================================
                      OEM Application for SOM - Can be Moved out to OEM specific file
==================================================================================================*/
void som_app_entry(void)
{
   /* TEST ONLY */
   som_app_entry_internal();
}

/*************************************************************************************************
**************************************************************************************************

                                 DO NOT EDIT BELOW                                       

**************************************************************************************************
*************************************************************************************************/

extern void i2cm_init(void);
extern void adc_init(void);
extern void pwm_init(void);
extern void diagnostic_sensormode_init(void);

extern void adc_deinit(void);
extern void pwm_deinit(void);
extern void diagnostic_sensormode_deinit(void);
extern void i2cm_deinit(void);


typedef void (*PLATFORM_FUNCTION_TYPE)(void);


PLATFORM_FUNCTION_TYPE init_sensormode_functions[] =
{
   #ifdef I2C_INIT_COLD
   /* I2C initialization*/
   i2cm_init,
   #endif

   #ifdef ADC_INIT_COLD
   /* ADC initialization */
   adc_init,
   #endif

   #ifdef PWM_INIT_COLD
   /* PWM initialization */
   pwm_init,
   #endif

   #ifdef DIAG_INIT_COLD
   /* Diagnostic services */
   diagnostic_sensormode_init,
   #endif
   
   som_app_init,
   
   0,
};

PLATFORM_FUNCTION_TYPE deinit_sensormode_functions[] =
{

   #ifdef DIAG_INIT_COLD
   /* de-initialize diag */
   diagnostic_sensormode_deinit,
   #endif

   #ifdef PWM_INIT_COLD
   /* de-initialize pwm */
   pwm_deinit,
   #endif

   #ifdef ADC_INIT_COLD
   /* de-initialize adc */
   adc_deinit,
   #endif

   #ifdef I2C_INIT_COLD
   /* de-initialize i2c master */
   i2cm_deinit,
   #endif

   0,
};
