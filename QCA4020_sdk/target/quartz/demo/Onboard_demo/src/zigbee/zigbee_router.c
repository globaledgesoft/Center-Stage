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

#include <qapi/com_dtypes.h>

#include "zigbee_util.h"
#include "onboard.h"
#include "led_utils.h"

/**
 * @func : Start_ZB_Router
 * @Param: Master_key is the security key to be verified at the time of joining the boards
 * @Desc : function will initialize the zigbee stack
 *         Device mode will be set to Router to join the formed network.(i.e Co-ordinator)
 */
int32_t Start_ZB_Router(char *Master_key)
{
    qapi_Status_t      Result;

    /*Initializing zigbee stack */
    Result = Zigbee_Initialize();
    if (Result != SUCCESS)
    {
        LOG_ERROR( "In Start_ZB_Router Code: Failed to Initialize Zigbee Stack\n");
        return FAILURE;
    }
    else
    {
		/** Zigbee LED Blink */
        R15_4_LED_CONFIG(1,50);
    }

    /** Setting Extended Address. Passing BLE mac addr @Param*/	
    Result = Zigbee_SetExtAddress(GetExtenAddr());
    if (Result != SUCCESS)
    {
        LOG_ERROR("Failed to Set Extended Address to Router\n");
        return FAILURE;
    }

    /** Getting Extended Address */
    Zigbee_GetAddresses();

    /** Joining Network, @Param enable/disable router, security, Rejoin/join mode, channel mask and Link Key*/
    Result = Zigbee_Join(ENABLE_ROUTER, ZIGBEE_SECURITY, ZIGBEE_REJOIN, ZIGBEE_CHANNEL_MASK, Master_key);
    if (Result != SUCCESS)
    {
        LOG_ERROR("Failed to Join the network\n");
        return FAILURE;
    }

    LOG_INFO("Router Successfully Joined Zigbee network\n");
    return SUCCESS;
}
