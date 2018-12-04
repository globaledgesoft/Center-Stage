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
#include "sensor_json.h"
#include <qapi_netservices.h>
#include "netutils.h"

/* : Start_Thread_Joiner
 * @Param: Passphrase is the security key to be verified at the time of joining the boards
 * @Desc : Function will initialize the Thread stack and also,
 *         sets the device in Thread Joiner mode.
 */
static const char IPv6_Interface[] = "ot0";
static ip6_addr dest_present, dest_new,nexthop;
static uint32_t     prefixlen = 64;
char Ipv6PrefixAddrLocal[24] = "FE80::";
int32_t Start_Thread_Joiner(const char *PassPhrase)
{
    uint32_t Result;
    uint32_t Device_Config = JOINER;
    char Ipv6PrefixAddrGlobal[24] = "FD00:102:304::";
    char Ipv6NextHope[48] = "5401:10:C101::2903:0:2903:0";
    uint32_t Poll_Period = JOINER_POLL_PERIOD;

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

    /** Setting Poll Period */
    Result = Thread_SetPollPeriod(Poll_Period);
    if (Result != SUCCESS)
    {
        LOG_ERROR("Failed to Set Poll Period\n");
        return FAILURE;
    }

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

    /** Constantly glowing R15_4 LED after Joiner connecting to Border Router **/
    R15_4_LED_CONFIG(1,100);
    qurt_thread_sleep(500);

    if (inet_pton(AF_INET6, Ipv6PrefixAddrLocal, &dest_present) != 0)
    {
        LOG_INFO("ERROR: Incorrect IPv6 dest\n");
        return FAILURE;
    }
    if (inet_pton(AF_INET6, Ipv6PrefixAddrGlobal, &dest_new) != 0)
    {
        LOG_INFO("ERROR: Incorrect IPv6 dest\n");
        return FAILURE;
    }
    if (inet_pton(AF_INET6, Ipv6NextHope, &nexthop) != 0)
    {
        LOG_INFO("ERROR: Incorrect IPv6 nexthop\n");
        return FAILURE;
    }

    qurt_thread_sleep(500);

    /** Delete the previous route entry */
    route6_del(IPv6_Interface, &dest_present, prefixlen);
    qurt_thread_sleep(50);

    /** Add the rote entry with global prefix */
    route6_add(IPv6_Interface, &dest_new, prefixlen, &nexthop);
    qurt_thread_sleep(50);
    /** shows the routing table */
    route6_show();
    qurt_thread_sleep(50);
    route6_show();

    /** Registering the PIR interrupt */
    pir_register_intr();
    /** calling UDP client */
    LOG_INFO(" calling UDP Client\n");
    Thread_Udp_Joiner();

    return SUCCESS;
}

void check_route(void)
{
    if (check_route_table(Ipv6PrefixAddrLocal))
    {
        route6_del(IPv6_Interface, &dest_present, prefixlen);
        qurt_thread_sleep(20);
        route6_show();
        qurt_thread_sleep(20);
        route6_add(IPv6_Interface, &dest_new, prefixlen, &nexthop);
        route6_show();

    }
}
