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

#ifndef __NETUTILS_H__
#define __NETUTILS_H__

#include <stdint.h>

#undef htons
#undef ntohs
#undef htonl
#undef ntohl
#define htons(s)    ((((s) >> 8) & 0xff) | (((s) << 8) & 0xff00))
#define ntohs(s)    htons(s)
#define htonl(l)    (((((l) >> 24) & 0x000000ff)) | \
                    ((((l) >>  8) & 0x0000ff00)) | \
                    (((l) & 0x0000ff00) <<  8) | \
                    (((l) & 0x000000ff) << 24))
#define ntohl(l)    htonl(l)

#ifndef min
#define  min(a,b)    (((a) <= (b)) ? (a) : (b))
#endif

#define HEX_BYTES_PER_LINE      16

#define INTR_DISABLE()
#define INTR_ENABLE()

#define IPV4ADDR_STR_LEN                   15                           /* IPv4 String length */
#define IPV6ADDR_STR_LEN                   45                           /* IPv6 String length */

/* queue element: cast to right type */
struct q_elt
{
    struct q_elt *qe_next;  /* pointer to next elt */
};
typedef struct q_elt  *qp;  /* q pointer */

/* queue header */
typedef struct queue
{
    struct q_elt *q_head;   /* first element in queue */
    struct q_elt *q_tail;   /* last element in queue */
    int  q_len;             /* number of elements in queue */
} QUEUE_T;

typedef struct
{
    uint32_t    seconds;        /* number of seconds */
    uint32_t    milliseconds;   /* number of milliseconds */
    uint32_t    ticks;
} time_struct_t;

void enqueue(QUEUE_T *q, void *item);
void * dequeue(QUEUE_T *q);

/* Return milliseconds */
uint32_t app_get_time(time_struct_t *time);
void app_msec_delay(uint32_t ms);
void route6_show(void);

void app_hexdump(void *inbuf, unsigned inlen, int ascii, int addr);


/*-------------------------------------------------------------------------
  - Preprocessor Definitions and Constants
  ------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------
 *Typedef and Declarations
 *------------------------------------------------------------------------*/

typedef int32_t (* net_dhcpv4s_CB_t)(
    uint8_t  *mac_Addr,
    /**< Client MAC address. */

    uint32_t ipv4_Addr
    /**< Client IPv4 address in network order. */
    );

typedef int32_t (* net_dhcpv4c_CB_t)(
    uint32_t ipv4_Addr,
    /**< IPv4 address in network order. */

    uint32_t subnet_Mask,
    /**< Subnet mask in network order. */

    uint32_t gateway
    /**< Default gateway's IPv4 address in network order. */
    );


/*-------------------------------------------------------------------------
  - Dhcp_Server: Run the DHCP Server
  -------------------------------------------------------------------------*/
int32_t dhcpv4_server(char *ifname, char *start_pool, char *endpool, uint32_t leasetime, net_dhcpv4s_CB_t success_cb);

/*---------------------------------------------------------------------------
  - if_config: Configures the ip address
  --------------------------------------------------------------------------*/
int32_t ifv4_config(char *interface, char *ip_str, char *mask_str, char *gw_str);

/*-------------------------------------------------------------------------
  - Dhcpv4_client: Run the DHCP Client
  -------------------------------------------------------------------------*/
int32_t dhcpv4_client(char *ifname, net_dhcpv4c_CB_t success_cb);
/*---------------------------------------------------------------------------
 - To check device got ipaddress or not
 ---------------------------------------------------------------------------*/
int32_t iface_has_ipaddress(char *interface_name);
/*-----------------------------------------------------------------------------
 * To release the ip address
 * ---------------------------------------------------------------------------*/
int32_t dhcp_release(char *interfacename);
/*-----------------------------------------------------------------------------
 * To stop dns client 
 * ---------------------------------------------------------------------------*/
int32_t stop_dns_client(void);
/*-----------------------------------------------------------------------------
 * To resolve_ip_address from hostname 
 * ---------------------------------------------------------------------------*/
int32_t resolve_ip_address(char *server);
/*-----------------------------------------------------------------------------
 * To add dns server 
 * ---------------------------------------------------------------------------*/

int32_t dns_add_server(char *svr, uint32_t id);
/*-----------------------------------------------------------------------------
 * To start dns client 
 * ---------------------------------------------------------------------------*/
int32_t start_dns_client(void);
/*-----------------------------------------------------------------------------
 * To add dns servers 
 * ---------------------------------------------------------------------------*/
int32_t add_dns_svr_list(void);
/*-----------------------------------------------------------------------------
 * To check IPV4 or IPV6
 * ---------------------------------------------------------------------------*/
int is_Inet6Pton(char * src, void * dst);
/*-----------------------------------------------------------------------------
 * To add sntp server address
 * ---------------------------------------------------------------------------*/
int32_t add_sntp_svr(char *server_address);
/*-----------------------------------------------------------------------------
 * To print sntp server address list
 * ---------------------------------------------------------------------------*/
int32_t print_sntpc_server_list(void);

/*-----------------------------------------------------------------------------
 * To check whether sntp client is started or not 
 * ---------------------------------------------------------------------------*/
int32_t is_sntp_started(void);
/*-----------------------------------------------------------------------------
 * To stop sntp client 
 * ---------------------------------------------------------------------------*/
int32_t stop_sntpc(void);

/*-----------------------------------------------------------------------------
 * To start sntp client 
 * ---------------------------------------------------------------------------*/
int32_t start_sntpc(void);

int32_t check_route_table(char *);
#endif /* _netutils_h_ */
