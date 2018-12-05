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

/** @file qapi_netservices.h
*/

#ifndef _QAPI_NETSERVICES_H_
#define _QAPI_NETSERVICES_H_


/*********************************************************************************
 * Status codes
**********************************************************************************/
#include "qapi/qapi_net_status.h"

/*********************************************************************************
 * Utilities
**********************************************************************************/
#include "qapi/qapi_addr.h"
#include "qapi/qapi_ns_utils.h"
#include "qapi/qapi_netbuf.h"

/*********************************************************************************
 * Socket
**********************************************************************************/
#include "qapi/qapi_socket.h"

/*********************************************************************************
 * General IPv4/IPv6 Network Services
**********************************************************************************/
#include "qapi/qapi_ns_gen_v4.h"
#include "qapi/qapi_ns_gen_v6.h"

/*********************************************************************************
 * DHCPv6 Client
**********************************************************************************/
#include "qapi/qapi_dhcpv6c.h"

/*********************************************************************************
 * DHCPv4 Client
**********************************************************************************/
#include "qapi/qapi_dhcpv4c.h"

/*********************************************************************************
 * DHCPv4 Server
**********************************************************************************/
#include "qapi/qapi_dhcpv4s.h"

/*********************************************************************************
 * Bridging
**********************************************************************************/

/*********************************************************************************
 * HTTP Client
**********************************************************************************/
#include "qapi/qapi_httpc.h"

/*********************************************************************************
 * HTTP Server
**********************************************************************************/
#include "qapi/qapi_httpsvr.h"

/*********************************************************************************
 * DNS Client
**********************************************************************************/
#include "qapi/qapi_dnsc.h"

/*********************************************************************************
 * DNS Server
**********************************************************************************/
#include "qapi/qapi_dnss.h"

/*********************************************************************************
 * SNTP Client
**********************************************************************************/
#include "qapi/qapi_sntpc.h"

/*********************************************************************************
 * DNS Service Discovery client
 *********************************************************************************/
#include "qapi/qapi_dnssd.h"

/*********************************************************************************
 * Multicast DNS Responder
 *********************************************************************************/
#include "qapi/qapi_mdnss.h"

#endif /* _QAPI_NETSERVICES_H_ */
