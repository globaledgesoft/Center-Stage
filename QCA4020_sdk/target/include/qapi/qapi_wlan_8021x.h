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

#ifndef _QAPI_WLAN_8021X_H_
#define _QAPI_WLAN_8021X_H_

/**
@file qapi_wlan_8021x.h
This section provides APIs, macros definitions, enumerations and data structures
for applications to perform WLAN 8021x control operations.
*/

#include "qapi/qapi_status.h"

/** @addtogroup qapi_wlan_8021x
@{ */

/**
@ingroup qapi_wlan_8021x
Identifies the enable/disable options for WLAN 8021x .
*/
typedef enum
{
    QAPI_WLAN_8021X_DISABLE_E  = 0, /**< Disable WLAN 8021X. */
    QAPI_WLAN_8021X_ENABLE_E   = 1  /**< Enable the WLAN 8021X. */
} qapi_WLAN_8021x_Enable_e;

/**
@ingroup qapi_wlan_8021x
Enables/disables the WLAN 8021X.
This is a blocking call and returns on allocated/freed resource for WLAN 8021x.

Use QAPI_WLAN_ENABLE_E as the parameter for enabling and QAPI_WLAN_DISABLE_E for disabling WLAN.

@datatypes
#qapi_WLAN_8021x_Enable_e

@param[in] enable  QAPI_WLAN_8021X_DISABLE_E or QAPI_WLAN_8021X_ENABLE_E.

@return
QAPI_OK -- Enabling or disabling WLAN succeeded. \n
Nonzero value -- Enabling or disabling failed.

@dependencies
To enable, use qapi_WLAN_Enable before qapi_wlan_8021x_enable. \n
To Disable, use qapi_WLAN_Enable after qapi_wlan_8021x_enable.
*/
qapi_Status_t qapi_wlan_8021x_enable (qapi_WLAN_8021x_Enable_e enable);

#endif
