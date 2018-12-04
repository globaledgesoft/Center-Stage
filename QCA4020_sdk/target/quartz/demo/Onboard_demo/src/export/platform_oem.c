/*=============================================================================

                               PLATFORM_INIT 

GENERAL DESCRIPTION
  This file contains the initial operations Quartz M4.

EXTERNALIZED FUNCTIONS

INITIALIZATION AND SEQUENCING REQUIREMENTS
  None

  $Header: //components/rel/core.ioe/1.0/platform/apps_proc/src/platform_oem.c#10 $
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

/*************************************************************************************************
**************************************************************************************************

                                 DO NOT EDIT BELOW                                       

**************************************************************************************************
*************************************************************************************************/

extern void i2cm_init(void);
extern void adc_init(void);
extern void pwm_init(void);
extern void diagnostic_init(void);
extern void diagnostic_sensormode_init(void);
extern void qca_module_init(void);
extern void fs_init(void);
extern void fs_diag_init(void);
extern void rfs_server_init(void);

extern void adc_deinit(void);
extern void pwm_deinit(void);
extern void diagnostic_deinit(void);
extern void diagnostic_sensormode_deinit(void);
extern void i2cm_deinit(void);
extern void fs_deinit(void);
extern void fs_diag_deinit(void);

typedef void (*PLATFORM_FUNCTION_TYPE)(void);

PLATFORM_FUNCTION_TYPE init_coldboot_functions[] =
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
   diagnostic_init,
   #endif

   #ifdef FS_INIT_COLD
   /* Initializes the filesystem */
   fs_init,
   #endif
   
   #ifdef FS_DIAG_INIT_COLD
   /* Initializes the filesystem */
   fs_diag_init,
   #endif

   #ifdef RFS_INIT_COLD
   /* Initializes the remote filesystem */
   rfs_server_init,
   #endif

   #ifdef QCA_INIT_COLD
   /* Initializes QCA modules */
   qca_module_init,
   #endif

   #ifdef PLATFORM_TEST_INIT
   platform_test_1,
   #endif

   0,
};

PLATFORM_FUNCTION_TYPE deinit_coldboot_functions[] =
{
   /* TODO: qca_deinit */

   #ifdef FS_DIAG_INIT_COLD
   /* de-initialize file system */
   fs_diag_deinit,
   #endif

   #ifdef FS_INIT_COLD
   /* de-initialize file system */
   fs_deinit,
   #endif

   #ifdef DIAG_INIT_COLD
   /* de-initialize diag */
   diagnostic_deinit,
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
