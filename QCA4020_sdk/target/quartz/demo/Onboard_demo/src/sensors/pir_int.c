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

#include "qapi/qurt_thread.h"
#include "stdint.h"
#include <qcli.h>
#include <qcli_api.h>
#include <qurt_timer.h>
#include <qapi_status.h>
#include <qapi_i2c_master.h>
#include "qapi_tlmm.h"
#include "qapi_gpioint.h"
#include "sensors_demo.h"
#include <log_util.h>
#include "onboard.h"

#define PIR_PIN              27

qurt_signal_t onboard_sigevent;
qapi_GPIO_ID_t  gpio_id;

/**
 * Int_Callback function is to handle the PIR interrupts
 */
void pir_int_callback(qapi_GPIOINT_Callback_Data_t data)
{
    //LOG_INFO("PIR Callback\n");
    //printf("hai prathyusha");
    qurt_signal_set(&onboard_sigevent, PIR_THREAD_SIGNAL_INTR);        //Change According to your requirement
    return;
}

/**
 * func() handles the interrupt genearted by
 * PIR sensors
 */
void pir_register_intr()
{
    int32_t gpio_pin = PIR_PIN;
    // uint32_t sig;
    // Necessary Data Type declarations
    qapi_GPIO_ID_t  gpio_id;
    qapi_Instance_Handle_t pH1;
    qapi_Status_t status = QAPI_OK;
    qapi_TLMM_Config_t tlmm_config;

    tlmm_config.pin = gpio_pin;
    tlmm_config.func = 0;   // Using the functionality tied to pin mux value
    tlmm_config.dir =  QAPI_GPIO_INPUT_E;
    tlmm_config.pull = QAPI_GPIO_PULL_DOWN_E;
    tlmm_config.drive = QAPI_GPIO_2MA_E; // drive is for output pins

    status = qapi_TLMM_Get_Gpio_ID(&tlmm_config, &gpio_id);

    if (status == QAPI_OK)
    {
        status = qapi_TLMM_Config_Gpio(gpio_id, &tlmm_config);
        if (status != QAPI_OK)
        {
            LOG_ERROR("Failed to config GPIO\n");
            //handle here Release_Gpio_Id
        }
    }

    if (qapi_GPIOINT_Register_Interrupt(&pH1, gpio_pin, (qapi_GPIOINT_CB_t )pir_int_callback,
                0, QAPI_GPIOINT_TRIGGER_EDGE_DUAL_E, QAPI_GPIOINT_PRIO_MEDIUM_E, false) != QAPI_OK)
    {
        LOG_ERROR("Interrupt Registeration failed\n");
        //Handle here release GPIo id
    }
//    LOG_INFO("PIR registration sucsess full ---------------------------------\n");
}

void pir_deregister(qapi_Instance_Handle_t pH1, qapi_TLMM_Config_t tlmm_config, int32_t gpio_pin )
{
    qapi_Status_t status = QAPI_OK;
    status = qapi_GPIOINT_Deregister_Interrupt(&pH1, gpio_pin);
    if (status != QAPI_OK)
        LOG_ERROR("Deregistering the Interrupt failed\n");

    if (qapi_TLMM_Release_Gpio_ID(&tlmm_config, gpio_id ) != QAPI_OK)
        LOG_ERROR("GPIO pin %d release Failed\n", gpio_pin);

    return;
}
