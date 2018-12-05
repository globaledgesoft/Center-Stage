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

/** @file qapi_dhcpv4c.h
 *
 * @addtogroup qapi_networking_dhcpv4c
 * @{
 *
 * @details The Dynamic Host Configuration Protocol IPv4 (DHCPv4) client service
 * provides a collection of API functions that allow the application to manage
 * automatic IPv4 configuration for a given network interface.
 * This configuration includes the interface IPv4 address, subnet mask,
 * default gateway, and DNS configuration.
 * In order to initiate a DHCPv4 client transaction, the application must
 * use the IP configuration API.
 *
 * @}
 */

#ifndef _QAPI_DHCPV4C_H_
#define _QAPI_DHCPV4C_H_

#include "stdint.h"

/** @addtogroup qapi_networking_dhcpv4c
@{ */

/**
 * @brief DHCPv4 client success callback,
 */
typedef int32_t (* qapi_Net_DHCPv4c_Success_CB_t)(
    uint32_t ipv4_Addr,
    /**< IPv4 address in network order. */

    uint32_t subnet_Mask,
    /**< Subnet mask in network order. */

    uint32_t gateway
    /**< Default gateway's IPv4 address in network order. */
    );

/**
 * @brief Releases a DHCPv4 lease.
 *
 * @param[in] interface_Name    Must be wlan0 or wlan1.
 *
 * @return On success, 0 is returned. On error, -1 is returned.
 */
int32_t qapi_Net_DHCPv4c_Release(const char * interface_Name);

/**
 * @brief Registers a callback for a DHCPv4 client.
 *
 * @details When the client successfully obtains an address, the callback will be invoked to return its
 * IPv4 address, subnet mask, and default gateway.
 *
 * @param[in] interface_Name    Must be wlan0 or wlan1.
 * @param[in] CB    Callback function.
 *
 * @return
 * On success, 0 is returned; on error, -1 is returned.
 */
int32_t qapi_Net_DHCPv4c_Register_Success_Callback(const char * interface_Name, qapi_Net_DHCPv4c_Success_CB_t CB);

/** @} */

#endif /* _QAPI_DHCPV4C_H_ */
