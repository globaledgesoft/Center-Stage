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

#ifndef _QAPI_NS_GEN_V4_H_
#define _QAPI_NS_GEN_V4_H_

#include <stdint.h>
#include "qapi/qapi_status.h"    /* qapi_Status_t */

/** @file qapi_ns_gen_v4.h
*
*/

/** @addtogroup qapi_networking_services
@{ */

/**
 * @brief Maximum size in bytes of hostname
 */
#define QAPI_NET_HOSTNAME_SIZE  (32)

/**
 * @brief Checks if the IPv4 address is multicast.
 *
 * @details This macro returns 1 if the passed IPv4 address is multicast.
 *          IPv4 multicast addresses are in the range 224.0.0.0 through 239.255.255.255.
 *
 * @param[in]      ipv4_Address    IPv4 address to check. Must be in network order.
 *
 * @return         1 if the IPv4 address is multicast, or 0 otherwise.
 *
 */
#define QAPI_IPV4_IS_MULTICAST(ipv4_Address)    (((uint32_t)(ipv4_Address) & 0xf0) == 0xe0)

/**
 * @brief Commands for IPv4 configuration.
 */
typedef enum
{
    QAPI_NET_IPV4CFG_QUERY_E,
    /**< Get IPv4 parameters of an interface, such as IP address, subnet mask, and default gateway. */

    QAPI_NET_IPV4CFG_STATIC_IP_E,
    /**< Assign IPv4 address, subnet mask, and default gateway. */

    QAPI_NET_IPV4CFG_DHCP_IP_E,
    /**< Run DHCPv4 client to obtain IPv4 parameters from the DHCPv4 server. */

    QAPI_NET_IPV4CFG_AUTO_IP_E,
    /**< Run auto-IP (automatic private IP addressing). */

    QAPI_NET_IPV4CFG_MAX_E
} qapi_Net_IPv4cfg_Command_t;

/**
 * @brief IPv4 routing object.
 */
typedef struct
{
    uint32_t RSVD;
    /**< Reserved. */

    uint32_t ipRouteDest;
    /**< Destination IPv4 address of this route. */

    uint32_t ipRouteMask;
    /**< Indicates the mask to be logical-ANDed with the
         destination address before being compared to the
         value in the ipRouteDest field. */

    uint32_t ipRouteNextHop;
    /**< IPv4 address of the next hop of this route. */

    uint32_t ipRouteIfIndex;
    /**< Index value that uniquely identifies the local
         interface through which the next hop of this
         route should be reached. */

    uint32_t ipRouteProto;
    /**< Routing mechanism via which this route was learned. */

    char     ifName[8];
    /**< Textual name of the interface. */

} qapi_Net_IPv4_Route_t;

/**
 * @brief Maximum IPv4 routing configurations.
 */
#define QAPI_NET_IPV4_MAX_ROUTES    (3)

/**
 * @brief IPv4 routing objects list.
 */
typedef struct
{
    uint32_t                route_Count;
    /**< Number of qapi_Net_IPv4_Route_t routes in the routing table. */

    qapi_Net_IPv4_Route_t   route[QAPI_NET_IPV4_MAX_ROUTES];
    /**< Array of qapi_Net_IPv4_Route_t routes. */

} qapi_Net_IPv4_Route_List_t;

/**
 * @brief Routing commands.
 */
typedef enum {
    QAPI_NET_ROUTE_ADD_E,
    /**< Add a route. */
    QAPI_NET_ROUTE_DEL_E,
    /**< Delete a route. */
    QAPI_NET_ROUTE_SHOW_E,
    /**< Show routes. */
    QAPI_NET_ROUTE_MAX_E
} qapi_Net_Route_Command_t;

/**
 * @brief Sets/gets IPv4 parameters, or triggers a DHCP client.
 *
 * @param[in] interface_Name  Interface name; must be wlan0 or wlan1.
 * @param[in] cmd   Command mode. Possible values are: \n
 *                  QAPI_NET_IPv4CFG_QUERY_E (0) \n
 *                  QAPI_NET_IPv4CFG_STATIC_IP_E (1) \n
 *                  QAPI_NET_IPv4CFG_DHCP_IP_E (2) \n
 *                  QAPI_NET_IPv4CFG_AUTO_IP_E (3)
 * @param[in] ipv4_Addr  Pointer to the IPv4 address in network order.
 * @param[in] subnet_Mask    Pointer to the IPv4 subnet mask in network order.
 * @param[in] gateway    Pointer to the default gateway's IPv4 address in network order.
 *
 * @return
 * On success, 0 is returned; on error, -1 is returned.
 */
qapi_Status_t qapi_Net_IPv4_Config(
                const char *interface_Name,
                qapi_Net_IPv4cfg_Command_t cmd,
                uint32_t *ipv4_Addr,
                uint32_t *subnet_Mask,
                uint32_t *gateway);

/**
 * @brief Sends an IPv4 ping.
 *
 * @param[in] ipv4_Addr  IPv4 destination address in network order.
 * @param[in] size  Size of the ping payload in bytes.
 *
 * @return On success, 0 is returned; on error, -1 is returned.
 */
qapi_Status_t qapi_Net_Ping(uint32_t ipv4_Addr, uint32_t size);

/**
 * @brief Adds, deletes, or queries an IPv4 route.
 *
 * @param[in] interface_Name  Interface name; must be wlan0 or wlan1.
 * @param[in] cmd   Command mode. Possible values are: \n
 *                  QAPI_NET_ROUTE_ADD_E (0) \n
 *                  QAPI_NET_ROUTE_DEL_E (1) \n
 *                  QAPI_NET_ROUTE_SHOW_E (2)
* @param[in] ipv4_Addr  Pointer to the IPv4 address in network order.
 * @param[in] subnet_Mask    Pointer to the IPv4 subnet mask in network order.
 * @param[in] gateway    Pointer to the default gateway's IPv4 address in network order.
 * @param[in] route_List    Buffer to contain a list of routes returned with the QAPI_NET_ROUTE_SHOW_E command.
 *
 * @return On success, 0 is returned; on error, -1 is returned.
 */
qapi_Status_t qapi_Net_IPv4_Route(
                const char *interface_Name,
                qapi_Net_Route_Command_t cmd,
                uint32_t *ipv4_Addr,
                uint32_t *subnet_Mask,
                uint32_t *gateway,
                qapi_Net_IPv4_Route_List_t *route_List);

/**
 * @brief Set hostname.
 *
 * @param[in] hostname  Null-terminated textual name of a host.
 *                      The string length of hostname should be <= QAPI_NET_HOSTNAME_SIZE
 *
 * @return On success, 0 is returned; on error, -1 is returned.
 */
qapi_Status_t qapi_Net_Set_Hostname(const char *hostname);

/** @} */ /* end_addtogroup qapi_networking_services */

#endif /* _QAPI_NS_GEN_V4_H_ */
