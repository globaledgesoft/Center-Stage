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

/** @file qapi_dhcpv4s.h
 *
 * @addtogroup qapi_networking_dhcpv4s
 * @{
 *
 * @details The Dynamic Host Configuration Protocol IPv4 (DHCPv4) service
 * provides a collection of API functions that allow the application to manage
 * a local DHCPv4 Server configuration, which is used by clients attached to
 * the device running the server. The application configures the IPv4 address
 * pool and lease time, and the rest of the configuration is taken from the
 * interface itself.
 * In order to start the DHCPv4 server, the application must setup the
 * pool configuration and, optionally, register to the success callback.
 *
 * @}
 */

#ifndef _QAPI_DHCPV4S_H_
#define _QAPI_DHCPV4S_H_

#include "stdint.h"

/** @addtogroup qapi_networking_dhcpv4s
@{ */

/**
 * @brief DHCPv4 server success callback.
 */
typedef int32_t (* qapi_Net_DHCPv4s_Success_CB_t)(
    uint8_t  *mac_Addr,
    /**< Client MAC address. */

    uint32_t ipv4_Addr
    /**< Client IPv4 address in network order. */
    );

/**
 * @brief Configures the IPv4 address pool of the DHCPv4 server.
 *
 * @param[in] interface_Name    Must be wlan0 or wlan1.
 * @param[in] start_IP          Starting IPv4 address in network order.
 * @param[in] end_IP            Ending IPv4 address in network order.
 * @param[in] lease_Time        Lease time in seconds.
 *
 * @return
 * On success, 0 is returned; on error, -1 is returned.
 */
int32_t qapi_Net_DHCPv4s_Set_Pool(const char *interface_Name, uint32_t start_IP, uint32_t end_IP, uint32_t lease_Time);

/**
 * @brief Registers a callback for the DHCPv4 server.
 *
 * @details When the client successfully obtains an address, the callback will be invoked to return its
 * IPv4 address and hardware address.
 *
 * @param[in] interface_Name    Must be wlan0 or wlan1.
 * @param[in] CB    Callback function.
 *
 * @return
 * On success, 0 is returned; on error, -1 is returned.
 */
int32_t qapi_Net_DHCPv4s_Register_Success_Callback(const char *interface_Name, qapi_Net_DHCPv4s_Success_CB_t CB);

/** @} */

#endif /* _QAPI_DHCPV4S_H_ */
