/*
 * Copyright (c) 2015-2018 Qualcomm Technologies, Inc.
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

/**
* @file qapi_wlan_bridge.h
*
* @addtogroup qapi_networking_wlanbrg
* @{
*
 * @details The WLAN bridging service provides a collection of API functions
 * that allow the application to set up and manage a 802.1D Bridge over two
 * Wi-Fi interfaces; one is an AP, and one a STA (or P2P equivalents).
 *
* @}
*/

#ifndef _QAPI_WLAN_BRIDGE_H_
#define _QAPI_WLAN_BRIDGE_H_

#include "stdint.h"
#include "qapi/qapi_status.h"

/** @addtogroup qapi_networking_wlanbrg
@{ */

/** MAC address length of the relayed addresses. */
#define QAPI_WLAN_BRIDGE_MAC_ADDR_LEN	6

/**
* @brief Structure to hold relay MAC database information.
*
* @details This structure is used when allocating a buffer to hold
* a bridge relay database.
*/
typedef struct {
   uint8_t addr[QAPI_WLAN_BRIDGE_MAC_ADDR_LEN];
   /**< Hardware MAC address. */

   uint16_t dev_ID;
   /**< Device ID. */

   uint32_t age;
   /**< Age timeout for MAC entry. */
} qapi_Net_Bridge_DB_Entry_t;

/**
* @brief Enables/disable a bridge in a target in the case of an IP offload.
*
* @param[in] enable  1 to enable a bridge; 0 to disable.
*
* @return
* 0 if the operation succeeded, -1 otherwise.
*/
qapi_Status_t qapi_Net_Bridge_Enable(uint32_t enable);

/**
* @brief Configures a MAC age timeout. 
*
* @param[in] timeout  Timeout value.
*
* @return
* 0 if the operation succeeded, -1 otherwise.
*/
qapi_Status_t qapi_Net_Bridge_Set_Aging_Timeout(uint32_t timeout);

/**
* @brief Shows bridge relay table.
*
* @details Call this API to fetch a bridge relay MAC database.
*
* @param[in] ptr    Pointer address to where the data is updated.
* @param[in] count  Number of entries in the relay table.
*
* @return
* 0 if the operation succeeded, -1 otherwise.
*/
qapi_Status_t qapi_Net_Bridge_Show_MACs(qapi_Net_Bridge_DB_Entry_t **ptr, uint32_t *count);

/** @} */

#endif /* _QAPI_WLAN_BRIDGE_H_ */
