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

#include "sensors_demo.h"
#include "adc_demo.h"
#include "pwm_demo.h"
#include "pwm_demo.h"
#include "peripherals_demo.h"
#include "master_sdcc_demo.h"
#include "htc_slave_demo.h"
#include "htc_demo.h"
#ifdef CONFIG_CDB_PLATFORM
#include "spi_demo.h"
#include "gpio_demo.h"
#endif
#include "keypad_demo.h"

QCLI_Group_Handle_t qcli_peripherals_group;


const QCLI_Command_Group_t peripherals_cmd_group =
{
    "Peripherals",
    0,
    NULL
};

   /* This function is used to register the portfolio subgroup with    */
   /* QCLI.                                                             */
void Initialize_Peripherals_Demo(void)
{
   /* Attempt to reqister the Command Groups with the qcli framework.*/
   qcli_peripherals_group = QCLI_Register_Command_Group(NULL, &peripherals_cmd_group);

   if(qcli_peripherals_group != NULL)
   {
	   Initialize_ADC_Demo();
	   Initialize_Sensors_Demo();
	   Initialize_PWM_Demo();
	   Initialize_SDCCM_Demo();
	   Initialize_HTCSlave_Demo();
	   Initialize_HTCHost_Demo();
#ifdef CONFIG_CDB_PLATFORM
   	   Initialize_SPI_Demo();
	   Initialize_GPIO_Demo();
#endif
	   Initialize_Keypad_Demo();
   }
   
   QCLI_Printf(qcli_peripherals_group, "Peripherals Registered \n");
}
