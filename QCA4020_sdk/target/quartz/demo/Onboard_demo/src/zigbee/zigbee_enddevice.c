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
#include "zcl_util.h"
#include "onboard.h"
#include "led_utils.h"
#include "sensor_json.h"
/**
 * @func : Start_ZB_EndDev
 * @Param: Master_key is the security key to be verified at the time of joining
 * @Desc : Initialize the Zigbee stack.
 *         It will set the device in End Device Mode to connect to the Formed network(i.e Co-ordinator)
 */
int32_t Start_ZB_EndDev(char *Master_key)
{
    qapi_Status_t      Result;
    uint8_t ClEndPoint = LIGHT_CLUSTER_ENDPOINT;
    uint8_t CustomClEndPoint = CUSTOM_CLUSTER_ENDPOINT;
    uint8_t DimmerClEndPoint = DIMMER_CLUSTER_ENDPOINT;
    uint8_t DimmerClEndPointType = CLUSTER_ENDPOINT_TYPE_DIMMABLE_LIGHT; /* Cluster Endpoint Type - 1: Dimmable Light 2: :Dimmer Switch */
    uint8_t ClEndPointType = CLUSTER_ENDPOINT_TYPE_LIGHT; /* Cluster Endpoint Type - 1: Light 2: :Light Switch */
    uint8_t CustomClEndPointType = CUSTOM_CLUSTER_CLIENT;

    /** Initializing zigbee stack */
    Result = Zigbee_Initialize();
    if (Result != SUCCESS)
    {
        LOG_ERROR("Failed to Initialize Zigbee Stack\n");
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
        LOG_ERROR("Failed to Set Extended Address to EndDevice\n");
        return FAILURE;
    }

    /** Getting Extended Address */
    Zigbee_GetAddresses();

    /** Joining Network, @Param enable/disable end device, security, Rejoin/join mode, channel mask and Link Key*/
    Result = Zigbee_Join(ENABLE_ENDDEVICE, ZIGBEE_SECURITY, ZIGBEE_REJOIN, ZIGBEE_CHANNEL_MASK, Master_key);
    if (Result != SUCCESS)
    {
        LOG_ERROR("Failed to Join the network\n");
        return FAILURE;
    }

    qurt_thread_sleep(DELAY);

    /** Creating cluster endpoint type - Light */
    Result = Zigbee_CL_CreateEndPoint(ClEndPoint, ClEndPointType);

    if(Result != SUCCESS)
    {
        LOG_ERROR("Failed to create Cluster Endpoint\n");
        return FAILURE;
    }

    /** Creating cluster endpoint type - Dimmer Switch */
    Result = Zigbee_CL_CreateEndPoint(DimmerClEndPoint, DimmerClEndPointType);
    if(Result != SUCCESS)
    {
        LOG_ERROR("Failed to create Cluster Endpoint\n");
        return FAILURE;
    }

    /** Creating custom cluster endpoint type - client */
    Result = Zigbee_CL_CreateEndPoint(CustomClEndPoint, CustomClEndPointType);

    if(Result != SUCCESS)
    {
        LOG_ERROR("Failed to create Custom Cluster Endpoint\n");
        return FAILURE;
    }
    pir_register_intr();

    LOG_INFO(  "End Device Successfully Joined network\n");
    return SUCCESS;
}
