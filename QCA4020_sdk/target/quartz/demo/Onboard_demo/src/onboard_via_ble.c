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

#include "ble_util.h"
#include "onboard_ble.h"
#include "onboard.h"
#include "wifi_util.h"
#include "log_util.h"

#if defined BOARD_SUPPORTS_WIFI && (ONBOARDED_OPERATION_MODE & OPERATION_MODE_WIFI)
qurt_signal_t wlan_sigevent;


/**
 * @func : Initialize_wifi()
 * @Desc : Initialize wlan interface to onboard
 */
int32_t Initialize_wifi()
{
    if (SUCCESS != qurt_signal_init(&wlan_sigevent))
    {
        LOG_ERROR("Signal init event failed\n");
        return FAILURE;
    }

    if (SUCCESS != wlan_enable(WLAN_NUM_OF_DEVICES, &wlan_sigevent))
    {
        LOG_ERROR("Wlan enable failed\n");
        return FAILURE;
    }

    return SUCCESS;
}
#endif

/**
 * @func : Register_services()
 * @Desc : Register the wifi and zigbee services
 */
static int32_t Register_services()
{
#ifndef OFFLINE
#if defined BOARD_SUPPORTS_WIFI && (ONBOARDED_OPERATION_MODE & OPERATION_MODE_WIFI)
    if (Initialize_wifi() != SUCCESS)
    {
        LOG_ERROR("BLE: error Initializing WiFi interface\n");
        return FAILURE;
    }
    if (Register_wifi_service() != SUCCESS)
    {
        LOG_ERROR("BLE: error register_onboard_service()\n");
        return FAILURE;
    }
#endif
#endif

#if (ONBOARDED_OPERATION_MODE & OPERATION_MODE_ZIGBEE)
    if (Register_zigbee_service() != SUCCESS)
    {
        LOG_ERROR("BLE: error register_onboard_service()\n");
        return FAILURE;
    }
#endif

#if (ONBOARDED_OPERATION_MODE & OPERATION_MODE_THREAD)
	if (Register_thread_service() != SUCCESS)
    {
        LOG_ERROR("BLE: error register_onboard_service()\n");
        return FAILURE;
    }
#endif
#ifdef OFFLINE
    if (Register_offline_service() != SUCCESS)
    {
        LOG_ERROR("BLE: error register_onboard_service()\n");
        return FAILURE;
    }
#endif

    return SUCCESS;
}

int32_t Start_onboard_via_ble()
{
    if (AdvertiseLE(1) != SUCCESS)
    {
        LOG_ERROR("BLE: error AdvertiseLE()\n");
        return FAILURE;
    }

    return SUCCESS;
}


/**
 * @func : InitializeOnboarding
 * @Desc : This function is the Entry Point
 *         Initializes BLE Stack and also registers the Onboard services
 */
int32_t Initialize_onboard_via_ble()
{
    if (InitializeBluetooth() != SUCCESS)
    {
        LOG_ERROR("BLE: Error InitializeBluetooth()\n");
        return FAILURE;
    }

    if (Register_services() != SUCCESS)
    {
        LOG_ERROR("BLE: Error Registering Services()\n");
        return FAILURE;
    }

    return SUCCESS;
}
