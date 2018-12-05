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
#include <string.h>

#include <qcli_api.h>
#include "qurt_thread.h"
#include "qapi/qapi_status.h"
#include "qapi_tlmm.h"
#include "qapi_gpioint.h"
#include "qurt_signal.h"

/* Handle for our QCLI Command Group. */
extern  QCLI_Group_Handle_t qcli_gpio_group;

#define GPIO_SIGNAL_INTR                                (1<<0)

static qurt_signal_t gpio_int_signal;

/**
 * Int_Callback function is to handle the pir interrupts
 */
void gpio_int_callback(qapi_GPIOINT_Callback_Data_t data)
{
    qurt_signal_set(&gpio_int_signal, GPIO_SIGNAL_INTR);
    return;
}

/**
 * func: gpio_Interrupt_Handle()
 * set the interrupt for desired pin with given Interrupt trigger
 */
QCLI_Command_Status_t gpio_interrupt_handle(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    /**
     * Necessary Data Type declarations
     */
    int32_t gpio_pin, trigger;
    qapi_GPIO_ID_t  gpio_id;
    qapi_Instance_Handle_t pH1;
    qapi_Status_t status = QCLI_STATUS_SUCCESS_E;
    qapi_TLMM_Config_t tlmm_config;

    gpio_pin = Parameter_List[0].Integer_Value;
    trigger = Parameter_List[1].Integer_Value;

    tlmm_config.pin = gpio_pin;
    tlmm_config.func = 0;   // Using the functionality tied to pin mux value
    tlmm_config.dir =  QAPI_GPIO_INPUT_E;
    tlmm_config.pull =  QAPI_GPIO_NO_PULL_E;
    tlmm_config.drive = QAPI_GPIO_2MA_E;

    status = qapi_TLMM_Get_Gpio_ID(&tlmm_config, &gpio_id);
    if (status == QCLI_STATUS_SUCCESS_E)
    {
        status = qapi_TLMM_Config_Gpio(gpio_id, &tlmm_config);
        if (status != QCLI_STATUS_SUCCESS_E)
        {
            QCLI_Printf(qcli_gpio_group, "Failed to config GPIO\n");
            goto release_gpio;
        }
    }

    if (0 != qurt_signal_init(&gpio_int_signal))
    {
        QCLI_Printf(qcli_gpio_group, "Not able to initialize signal\n");
        goto release_gpio;
    }

    if (qapi_GPIOINT_Register_Interrupt(&pH1, gpio_pin, (qapi_GPIOINT_CB_t)gpio_int_callback,
                0,  trigger, QAPI_GPIOINT_PRIO_MEDIUM_E, false) != QCLI_STATUS_SUCCESS_E)
    {
        QCLI_Printf(qcli_gpio_group, "Error in Registering Interrupt!\n");
        goto release_gpio;
    }

    qurt_signal_wait(&gpio_int_signal, GPIO_SIGNAL_INTR, QURT_SIGNAL_ATTR_CLEAR_MASK);
    QCLI_Printf(qcli_gpio_group, "Received Interrupt Signal\n");

    // Deregister the GPIO Interrupt
    status = qapi_GPIOINT_Deregister_Interrupt(&pH1, gpio_pin);
    if (status != QCLI_STATUS_SUCCESS_E)
    {
        QCLI_Printf(qcli_gpio_group, "Error in Deregistering Interrupt\n");
    }

release_gpio:
    if (qapi_TLMM_Release_Gpio_ID(&tlmm_config, gpio_id) != QCLI_STATUS_SUCCESS_E)
        QCLI_Printf(qcli_gpio_group, "GPIO pin %d release Failed\n", gpio_pin);

    return QCLI_STATUS_SUCCESS_E;                                                                   
}

QCLI_Command_Status_t gpio_interrupt(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if (Parameter_Count < 2)
    {
        QCLI_Printf(qcli_gpio_group, "USAGE: gpio_pin INT_trigger\n");
        QCLI_Printf(qcli_gpio_group, "\r gpio_pin -> gpio pin number\n\r INT_trigger Values\n\r 0 - TRIGGER_LEVEL_HIGH \
                \n\r 1 - TRIGGER_LEVEL_LOW\n\r 2 - TRIGGER_EDGE_RISING \
                \n\r 3 - TRIGGER_EDGE_FALLING\n\r 4 - TRIGGER_EDGE_DUAL");
        return QCLI_STATUS_SUCCESS_E;
    }

    gpio_interrupt_handle(Parameter_Count, Parameter_List);
    return QCLI_STATUS_SUCCESS_E;
}

int32_t gpio_io_handle(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    qapi_Status_t status = QCLI_STATUS_SUCCESS_E;
    int32_t gpio_num, fun, dir, pull, val;
    qapi_TLMM_Config_t tlmm_config;
    qapi_GPIO_ID_t  gpio_id;

    gpio_num = Parameter_List[0].Integer_Value;
    fun = Parameter_List[1].Integer_Value;
    dir = Parameter_List[2].Integer_Value;
    pull = Parameter_List[3].Integer_Value;
    if (dir == 1 && Parameter_Count < 5)
    {
        QCLI_Printf(qcli_gpio_group, "Provide output value to drive\n");
        return QCLI_STATUS_ERROR_E;
    }
    else
        val = Parameter_List[4].Integer_Value;

    QCLI_Printf(qcli_gpio_group, "Pin %d, fun %d, dir %d pull %d, val %d \n", gpio_num, fun, dir, pull, val);

    tlmm_config.pin = gpio_num;
    tlmm_config.func = fun;
    tlmm_config.dir =  dir;
    tlmm_config.pull = pull;
    tlmm_config.drive = QAPI_GPIO_2MA_E;

    status = qapi_TLMM_Get_Gpio_ID(&tlmm_config, &gpio_id);
    if (status != QCLI_STATUS_SUCCESS_E)
    {
        QCLI_Printf(qcli_gpio_group," TLMM Get failed:%d\n", status);
        return QCLI_STATUS_ERROR_E;
    }

    status = qapi_TLMM_Config_Gpio(gpio_id, &tlmm_config);
    if (status != QCLI_STATUS_SUCCESS_E)
    {
        QCLI_Printf(qcli_gpio_group, "Failed to config GPIO\n");
        return QCLI_STATUS_ERROR_E;
    }
    qurt_thread_sleep(20);

    if (dir == QAPI_GPIO_INPUT_E)
        QCLI_Printf(qcli_gpio_group, "Value Read %d\n", qapi_TLMM_Read_Gpio(gpio_id, gpio_num));
    else
    {
        if (qapi_TLMM_Drive_Gpio(gpio_id, gpio_num, val) != QCLI_STATUS_SUCCESS_E)
        {
            QCLI_Printf(qcli_gpio_group, "Not able to drive the pin %d\n", gpio_num);
        }
        else
            QCLI_Printf(qcli_gpio_group, "Check for %d on the pin %d\n", val, gpio_num);
    }
    qurt_thread_sleep(20);

    if (qapi_TLMM_Release_Gpio_ID(&tlmm_config, gpio_id) == QCLI_STATUS_SUCCESS_E)
        QCLI_Printf(qcli_gpio_group, "GPIO pin %d released successfully\n", gpio_num);
    else
        QCLI_Printf(qcli_gpio_group, "GPIO pin %d release failed\n", gpio_num);

    return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t gpio_tlmm(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if (Parameter_Count < 4)
    {
        QCLI_Printf(qcli_gpio_group, "USAGE: gpio_pin fun dir pull val\n");
        QCLI_Printf(qcli_gpio_group, "\r gpio_pin -> pin number \n\r fun -> function value (0-15) \
                                      \n\r dir -> direction(input/ output) 0-input 1-output");
        QCLI_Printf(qcli_gpio_group, "\n\r pull -> gpio pull up/down/no-pull 0-No pull 1-Pull Down 2-Pull Up \
                                      \n\r val(value to drive if direction is output)");
        return QCLI_STATUS_SUCCESS_E;
    }

    gpio_io_handle(Parameter_Count, Parameter_List);

    return QCLI_STATUS_SUCCESS_E;
}
