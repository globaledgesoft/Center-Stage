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

#include "thread_util.h"
#include "log_util.h"
#include "qapi_types.h"

/* : Start_Thread_Joiner
 * @Param: Passphrase is the security key to be verified at the time of joining the boards
 * @Desc : Function will initialize the Thread stack and also,
 *         sets the device in Thread Joiner mode.
 */
int32_t Start_Thread_Router(const char *PassPhrase)
{
    uint32_t Result;
    uint32_t Device_Config = JOINER;
    LOG_INFO("%s\n",__func__);

    /** Initializing Joining Router */
    Result = Thread_Initialize(Device_Config);

    if (Result != SUCCESS)
    {
        LOG_ERROR("Failed to Initialize Thread Stack\n");
        return FAILURE;
    }

    /** Blinking R15_4 LED after Thread initialization */
    R15_4_LED_CONFIG(1,50);


    qurt_thread_sleep(500);

    /** Attempts to commission onto an existing Thread network */
    Result =  Thread_MeshCoP_JoinerStart(PassPhrase);
    if (Result != SUCCESS)
    {
        LOG_ERROR("Failed to join thread network\n");
        return FAILURE;
    }

    qurt_thread_sleep(1500);

    /** Start the Thread Interface, connecting or starting a network */
    Result = Thread_Interface_Start();

    if (Result != SUCCESS)
    {
        LOG_ERROR("Failed to Start Thread Interface\n");
        return FAILURE;
    }

    /** Constantly glowing R15_4 LED after Joiner connecting to Border ROuter **/
    R15_4_LED_CONFIG(1,100);

    return SUCCESS;
}
