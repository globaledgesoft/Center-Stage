/*
* Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
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
#include <stdlib.h>
#include <qcli.h>
#include <qcli_api.h>

QCLI_Command_Status_t gpio_interrupt(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t gpio_tlmm(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);

extern QCLI_Group_Handle_t qcli_peripherals_group;  /* Handle for our peripherals subgroup. */

QCLI_Group_Handle_t qcli_gpio_group;                /* Handle for our QCLI Command Group. */

const QCLI_Command_t gpio_cmd_list[] =
{
    // cmd_function      start_thread      cmd_string               usage_string                   description
    { gpio_interrupt,      false,      "gpio_interrupt_test",      "<gpio_pin>, <trigger_value = 0:TRIGGER_LEVEL_HIGH| 1:TRIGGER_LEVEL_LOW|                               2:TRIGGER_EDGE_RISING| 3: TRIGGER_EDGE_FALLING| 4:TRIGGER_EDGE_DUAL>",      "enable/disable interrupt on GPIO" },
    { gpio_tlmm,           false,      "gpio_tlmm",                "<gpio_pin>, <function = 0-15>, <direction = 0:input| 1:output>,                                       <pull = 0:No_Pull| 1:Pull_Down| 2:Pull_Up>, <value = 0:pin_low |1:pin_high>",       "to drive GPIO in io mode" },
};

const QCLI_Command_Group_t gpio_cmd_group =
{
    "GPIO",
    (sizeof(gpio_cmd_list) / sizeof(gpio_cmd_list[0])),
    gpio_cmd_list
};

/* This function is used to register the SPI Command Group with */
/* QCLI.                                                        */
void Initialize_GPIO_Demo(void)
{
    /* Attempt to reqister the Command Groups with the qcli framework.*/
    qcli_gpio_group = QCLI_Register_Command_Group(qcli_peripherals_group, &gpio_cmd_group);
    if (qcli_gpio_group)
    {
        QCLI_Printf(qcli_gpio_group, "GPIO INTR/TLMM Registered\n");
    }
}
