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
#include "qapi_timer.h"

/**
 * @func  : permit_join
 * @param : Takes time in seconds, and it will allow the device to be in
 *          permit mode to that Duration, max it will tale 255sec
 * @Desc  : function will set the device to perimit mode.
 */
void permit_join(uint8_t PermitTime)
{
    qapi_Status_t      Result;

    LOG_INFO("Timer Set PermitTime %d sec\n", PermitTime);

    Result = Zigbee_PermitJoin(PermitTime);
    if (Result != SUCCESS)
    {
        LOG_ERROR("Failed to Permit\n");
    }

    return;
}

/**
 * @func  : timer_zb_cb
 * @param : Callback data
 * @Desc  : after every timer expiry callbacks will be called.
 */
void timer_zb_cb(uint32_t data)
{
    permit_join(PERMIT_TIME);
    return;
}

/**
 * @func : initialize_timer
 * @Desc : function will initialize the timer to certain time interval,
 *         after timer expiry callback function will be called
 */
void initialize_timer()
{
    qapi_TIMER_handle_t timer_handle;
    qapi_TIMER_define_attr_t timer_def_attr;
    qapi_TIMER_set_attr_t timer_set_attr;

    /*Timer intialization part */
    timer_def_attr.cb_type = QAPI_TIMER_FUNC1_CB_TYPE;
    timer_def_attr.sigs_func_ptr = timer_zb_cb;
    timer_def_attr.sigs_mask_data = 0x1;
    timer_def_attr.deferrable = false;
    timer_set_attr.reload =true;
    timer_set_attr.time = TIME_DURATION;
    timer_set_attr.unit = QAPI_TIMER_UNIT_SEC;

    /* Defines the timer or Memory allocation */
    qapi_Timer_Def(&timer_handle, &timer_def_attr);

    qapi_Timer_Set(timer_handle, &timer_set_attr);
    LOG_INFO("Timer is Set\n");
}

/**
 * @func : Start_ZB_Coordinator
 * @Param: Master_key is the security key to be verified at the time of joining the boards   
 * @Desc : Function will initialize the Zigbee stack and also,
 *         sets the device in Zigbee Co-ordinator Mode
 */

int32_t Start_ZB_Coordinator(char *Master_key)
{
    qapi_Status_t      Result;
    uint8_t ClEndPoint = LIGHT_CLUSTER_ENDPOINT;
    uint8_t CustomClEndPoint = CUSTOM_CLUSTER_ENDPOINT;
    uint8_t DimmerClEndPoint = DIMMER_CLUSTER_ENDPOINT;
    uint8_t DimmerClEndPointType = CLUSTER_ENDPOINT_TYPE_DIMMER_SWITCH; /* Cluster Endpoint Type - 1: Dimmable Light 2: :Dimmer Switch */
    uint8_t ClEndPointType = CLUSTER_ENDPOINT_TYPE_LIGHT_SWITCH; /* Cluster Endpoint Type - 1: Light 2: :Light Switch */
    uint8_t CustomClEndPointType = CUSTOM_CLUSTER_SERVER;

    /** Initializing zigbee stack */
    Result = Zigbee_Initialize();
    if (Result != SUCCESS)
    {
        LOG_ERROR("Failed to Initialize Zigbee Stack\n");
        return FAILURE;
    }

    /** Setting Extended Address. Passing BLE mac addr @Param*/
    Result = Zigbee_SetExtAddress(GetExtenAddr());
    if (Result != SUCCESS)
    {
        LOG_ERROR("Failed to Set Extended Address to Cordinator\n");
        return FAILURE;
    }

    /** Getting Extended Address */
    Zigbee_GetAddresses();

    /** Forming Network, @Param enable/disable router, security, Co-ordinator mode, channel mask and Link Key*/
    Result = Zigbee_Form(ZIGBEE_SECURITY, CO_ORDINATOR_DISTRIBUTION, ZIGBEE_CHANNEL_MASK,Master_key);
    if (Result != SUCCESS)
    {
        LOG_ERROR("Failed to Form Zigbee network\n");
        return FAILURE;
    }
    else
        LOG_INFO("Successfully created Zigbee network\n");

    /** Permitting Router and End Devices "PermitTime" seconds to join the network */
    permit_join(PERMIT_TIME);
    initialize_timer();

    /** Creating cluster endpoint type - Light Switch */
    Result = Zigbee_CL_CreateEndPoint(ClEndPoint,ClEndPointType);

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

    Result = Zigbee_CL_CreateEndPoint(CustomClEndPoint, CustomClEndPointType);

    if(Result != SUCCESS)
    {
        LOG_ERROR("Failed to create Custom Cluster Endpoint\n");
        return FAILURE;
    }

    return SUCCESS;
}
