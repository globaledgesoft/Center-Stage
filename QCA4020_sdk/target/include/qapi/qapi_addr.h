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

/** @file qapi_addr.h
 */

#ifndef _QAPI_ADDR_H_
#define _QAPI_ADDR_H_

#include <stdint.h>

/** @addtogroup qapi_networking_socket
@{ */

/**
 * @brief IPv4 Internet address.
 */
#ifndef __IN_ADDR__
#define __IN_ADDR__
struct in_addr
{
   uint32_t s_addr;
   /**< IPv4 address in network order. */
};

typedef unsigned long ip_addr;
#endif

/**
 * @brief IPv6 Internet address.
 */
#ifndef __IN6_ADDR__
#define __IN6_ADDR__
typedef struct in6_addr
{
   uint8_t  s_addr[16];
   /**< 128-bit IPv6 address. */
} ip6_addr;
#endif

/**
 * @brief IPv4/IPv6 Internet address union.
 */
#ifndef __IP46_ADDR__
#define __IP46_ADDR__
struct ip46addr
{
    uint16_t type;
    /**< AF_INET or AF_INET6. */
    union
    {
        unsigned long   addr4;
        /**< IPv4 address. */
        ip6_addr        addr6;
        /**< IPv6 address. */
    } a;
    /**< Address union. */
};
#endif

/** @} */

#endif /* _QAPI_ADDR_H_ */
