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

#include <qapi_socket.h>
#include <qapi/qapi_socket.h>
#include <qapi_netservices.h>
#include <qapi_ns_gen_v4.h>
#include <qapi_dhcpv4c.h>
#include "qapi_ns_utils.h"
#include "qurt_types.h"
#include "qurt_timer.h"
#include "qurt_thread.h"
#include "qcli_api.h"
#include <stdlib.h>

#include "netutils.h"
#include "log_util.h"

/*-------------------------------------------------------------------------
 *Typedef and Declarations
 *------------------------------------------------------------------------*/
int32_t stop_dns_client(void)
{
    if (qapi_Net_DNSc_Is_Started())
    {
        return -1;
    }
    qapi_Net_DNSc_Command(QAPI_NET_DNS_DISABLE_E);
    return 0;
}

int32_t start_sntpc(void)
{
    int32_t e = -1;
    if (!qapi_Net_SNTPc_Is_Started())
    {
        e = qapi_Net_SNTPc_Command(QAPI_NET_SNTP_START_E);
        LOG_VERBOSE("Sntpc start success: %d\n", e); 
    }
    if (e)
        LOG_ERROR("Sntpc start failed\n");
    return e;
}

int32_t stop_sntpc(void)
{
    int32_t e = 0;
    if (qapi_Net_SNTPc_Is_Started())
        e = qapi_Net_SNTPc_Command(QAPI_NET_SNTP_STOP_E);
    if (e)
        LOG_ERROR("Sntpc stop failed\n");
    return e;
}

int32_t is_sntp_started(void)
{
    int32_t e;
    e = qapi_Net_SNTPc_Is_Started();
    return e;
}

int32_t print_sntpc_server_list(void)
{
    int32_t i = 0;
    int32_t started;
    qapi_Net_SNTP_Server_List_t svr_list;

    started = qapi_Net_SNTPc_Is_Started();
    LOG_VERBOSE("SNTP client is %s.\n", started ? "started" : "stopped");
    if (qapi_Net_SNTPc_Get_Server_List(&svr_list) == 0)
    {
        for (i = 0; i < QAPI_NET_SNTP_SERVER_MAX; ++i)
        {
            LOG_VERBOSE("%d: ", i);
            LOG_VERBOSE("%s    ", svr_list.svr[i].name[0] != '\0' ? svr_list.svr[i].name : "****");
            LOG_VERBOSE("%s  ", svr_list.svr[i].addr[0] != '\0' ? svr_list.svr[i].addr : "****");
            if (svr_list.svr[i].status == QAPI_NET_SNTP_SERVER_STATUS_KOD)
            {
                LOG_VERBOSE("KOD");
            }
            else if (svr_list.svr[i].status == QAPI_NET_SNTP_SERVER_STATUS_NORESP)
            {
                LOG_VERBOSE("NORESP");
            }
            LOG_VERBOSE("\n"); 
        }
    }
    return 0;
}

int32_t add_sntp_svr(char *server_address)
{
    int32_t e = -1;
    int32_t id = QAPI_NET_SNTP_ANY_SERVER_ID;
    if (qapi_Net_SNTPc_Is_Started())
    {
        LOG_INFO("SNTP ADD SVR Called\n");
        e = qapi_Net_SNTPc_Add_Server(server_address, id);
        if (e == QAPI_NET_ERR_INVALID_IPADDR)
        {
            LOG_ERROR("Invalid sntp server address\n");
        }
        else if (e == QAPI_NET_ERR_CANNOT_GET_SCOPEID)
        {
            LOG_ERROR("Failed to get scope id\n");
        }
        LOG_INFO("Add server return value : %d\n", e);
        print_sntpc_server_list();

    }
    return e;
}

int32_t resolve_ip_address(char *server)
{
    struct ip46addr ipaddr;
    int e;
    char ip_str[48] = { 0 };
    char ip_buf[16] = {0};

    if (!is_Inet6Pton((char*)server, ip_buf))
    {
        ipaddr.type = AF_INET6;
    } else {
        ipaddr.type = AF_INET;
    }


    LOG_INFO("try to resolve ....\n");
    if  (!qapi_Net_DNSc_Is_Started())
    {
        qapi_Net_DNSc_Command(QAPI_NET_DNS_START_E);
    }


    if  (!qapi_Net_DNSc_Is_Started())
    {
        LOG_INFO("DNS client is not started yet.\n");
        return -1;
    }


    e = qapi_Net_DNSc_Reshost(server, &ipaddr);
    LOG_INFO("e Value: %d\n", e);
    if (e)
    {
        qapi_Net_DNSc_Command(QAPI_NET_DNS_DISABLE_E);
        LOG_INFO("Unable to resolve %s\n", server);
        return -1;
    }
    else
    {
        LOG_INFO("\n%s --> %s\n", server, ip_str);
    }

    return e;
}

int32_t dhcp_release(char *ifname)
{
    if(0 != qapi_Net_DHCPv4c_Release(ifname))
        return FAILURE;
    return SUCCESS;
}

int32_t dns_add_server(char *svr, uint32_t id)
{
    if( 0!= qapi_Net_DNSc_Add_Server(svr,id))
        return FAILURE;
    return SUCCESS;
}

int32_t add_dns_svr_list(void)
{
    char ip_str[48] = { 0 };
    uint32_t addr = 0, mask = 0, gw=0;
    int32_t err;

    if ((err = qapi_Net_IPv4_Config("wlan1", QAPI_NET_IPV4CFG_QUERY_E, &addr, &mask, &gw)) == 0)
    {
       inet_ntop(AF_INET, &gw, ip_str,sizeof(ip_str));
       LOG_VERBOSE("Gateway address :%s\n", ip_str);
    }

    if (qapi_Net_DNSc_Is_Started())
    {
        dns_add_server(ip_str,0);
        dns_add_server("8.8.8.8",1);
    }
    else
    {
        return -1;
    }
   return 0;
}

int32_t start_dns_client(void)
{
    if  (!qapi_Net_DNSc_Is_Started())
    {
        qapi_Net_DNSc_Command(QAPI_NET_DNS_START_E);
    }

    return 0;
}

int32_t iface_has_ipaddress(char *ifname)
{
    int32_t err;
    uint32_t addr = 0, mask = 0, gw=0;

    if ((err = qapi_Net_IPv4_Config(ifname, QAPI_NET_IPV4CFG_QUERY_E, &addr, &mask, &gw)) == 0)
    {
        LOG_VERBOSE("IPADDR:%x, MSK:%x, GW:%x\n", addr, mask, gw);
        if (addr && gw)
            return SUCCESS;
        else 
            return FAILURE;
    }
    
    return SUCCESS;
}

/*---------------------------------------------------------------------------
  - if_config: Configures the ip address
  --------------------------------------------------------------------------*/
int32_t ifv4_config(char *ifname, char *ip_str, char *mask_str, char *gw_str)
{
    uint32_t addr, mask, gw;
    int32_t ret_val;

    /* interface's IP address */
    if (inet_pton(AF_INET, ip_str, &addr) != 0)
    {
        return FAILURE;
    }

    /* subnet mask */
    if (inet_pton(AF_INET, mask_str, &mask) != 0)
    {
        return FAILURE;
    }

    if (inet_pton(AF_INET, gw_str, &gw) != 0)
    {
        return FAILURE;
    }

    ret_val = qapi_Net_IPv4_Config(ifname, QAPI_NET_IPV4CFG_STATIC_IP_E, &addr, &mask, &gw);
    if (ret_val != 0)
    {
        return FAILURE;
    }

    return SUCCESS;
}

/*-------------------------------------------------------------------------
  - Dhcp_Server: Run the DHCP Server
  -------------------------------------------------------------------------*/
int32_t dhcpv4_server(char *ifname, char *start_pool, char *endpool, uint32_t leasetime, net_dhcpv4s_CB_t success_cb)
{
    uint32_t startaddr, endaddr;
    uint32_t addr, mask;
    int32_t ret_val;

    ret_val = inet_pton(AF_INET, start_pool, &startaddr);
    if (ret_val != 0)
    {
        return FAILURE;
    }

    ret_val = inet_pton(AF_INET, endpool, &endaddr);
    if (ret_val != 0)
    {
        return FAILURE;
    }

    if (startaddr >= endaddr)
    {
        return FAILURE;
    }

    /* Get interface's IP address */
    ret_val = qapi_Net_IPv4_Config(ifname, QAPI_NET_IPV4CFG_QUERY_E, &addr, &mask, NULL);
    if (ret_val != 0)
    {
        return FAILURE;
    }

    if ((addr & mask) != (startaddr & mask) ||
            (addr & mask) != (endaddr & mask))
    {
        return FAILURE;
    }

    if ((addr >= startaddr) && (addr <= endaddr))
    {
        return FAILURE;
    }

    if (qapi_Net_DHCPv4s_Register_Success_Callback(ifname, (qapi_Net_DHCPv4s_Success_CB_t)success_cb) != 0 ||
            qapi_Net_DHCPv4s_Set_Pool(ifname, startaddr, endaddr, leasetime) != 0)
    {
        return FAILURE;
    }

    return SUCCESS;
}

/*-------------------------------------------------------------------------
  - Dhcpv4_client: Run the DHCP Client
  -------------------------------------------------------------------------*/
int32_t dhcpv4_client(char *ifname, net_dhcpv4c_CB_t success_cb)
{
    if (qapi_Net_DHCPv4c_Register_Success_Callback(ifname, (qapi_Net_DHCPv4c_Success_CB_t)success_cb) != 0 ||
            qapi_Net_IPv4_Config(ifname, QAPI_NET_IPV4CFG_DHCP_IP_E, NULL, NULL, NULL) != 0)
    {
        return FAILURE;
    }
    return SUCCESS;
}

/**************************************************************************/
uint32_t app_get_time(time_struct_t *time)
{
    uint32_t ticks, ms;

    ticks = (uint32_t)qurt_timer_get_ticks();
    ms = (uint32_t)qurt_timer_convert_ticks_to_time(ticks, QURT_TIME_MSEC);

    if (time)
    {
        time->seconds = ms / 1000;
        time->milliseconds = ms % 1000;
        time->ticks = ticks;
    }

    return ms;
}

void app_msec_delay(uint32_t ms)
{
    uint32_t ticks;

    ticks = qurt_timer_convert_time_to_ticks(ms, QURT_TIME_MSEC);
    if (ms != 0 && ticks == 0)
    {
        ticks = 1;
    }
    qurt_thread_sleep(ticks);

    return;
}
/** Delete the previous route entry */
uint32_t route6_del(char * ifname, ip6_addr * dest, uint32_t prefixlen)
{
    uint32_t Result;
    uint32_t Ret_Val = FAILURE;

    LOG_INFO("Deleting route entry\n");

    Result  = qapi_Net_IPv6_Route_Del(ifname, dest, prefixlen);

    if(Result == SUCCESS)
    {
        LOG_INFO("SUCCESS: Deleted route entry\n");
        Ret_Val = SUCCESS;
    }
    else
    {
        LOG_INFO("FAILED: Unable to delete route entry\n");
        Ret_Val = FAILURE;
    }
    return Ret_Val;
}

/** Add the rote entry with global prefix */
uint32_t route6_add(char * ifname, ip6_addr * dest, uint32_t prefixlen, ip6_addr * nexthop)
{
    uint32_t Result;
    uint32_t Ret_Val = FAILURE;

    LOG_INFO("Adding rOute entry\n");

    Result  = qapi_Net_IPv6_Route_Add(ifname, dest, prefixlen, nexthop);

    if(Result == SUCCESS)
    {
        LOG_INFO("SUCCESS: Added route entry\n");
        Ret_Val = SUCCESS;
    }
    else
    {
        LOG_INFO("FAILED: Unable to add route entry\n");
        Ret_Val = FAILURE;
    }
    return Ret_Val;
}

/** Rout6 show */
void route6_show(void)
{
        uint32_t count, i;
        qapi_Net_IPv6_Route_t *buf;
        char dest_str[40];
        char nexthop_str[40];
        qapi_Status_t rc = QAPI_ERROR;

        rc = qapi_Net_IPv6_Routing_Table_Get(NULL, &count);
        if (rc == QAPI_OK)
        {
            if (count == 0)
            {
                LOG_INFO("Routing table is empty.\n");
            }
            else
            {
                buf = malloc(count * sizeof(qapi_Net_IPv6_Route_t));
                if (buf == NULL)
                {
                    rc = QAPI_ERROR;
                    LOG_ERROR("FAILED: route6\n");
                }

                rc = qapi_Net_IPv6_Routing_Table_Get(buf, &count);
                if (rc == QAPI_OK)
                {
                    LOG_INFO("Destination           Nexthop              iface\n");
                    for (i = 0; i < count; ++i)
                    {
                        LOG_INFO("%s/%d      %s      %s\n",
                                inet_ntop(AF_INET6, buf->ipv6RouteDest, dest_str, sizeof(dest_str)),
                                buf->ipv6RoutePfxLength,
                                inet_ntop(AF_INET6, buf->ipv6RouteNextHop, nexthop_str, sizeof(nexthop_str)),
                                buf->ifName);
                    }
                }
                free(buf);
            }
        }
 }

int32_t check_route_table(char *addr)
{
    uint32_t count, i;
    int32_t ret = 0;
    qapi_Net_IPv6_Route_t *buf;
    char dest_str[40];
    qapi_Status_t rc = QAPI_ERROR;

    LOG_INFO("Destination addres: %s\n", addr);
    rc = qapi_Net_IPv6_Routing_Table_Get(NULL, &count);
    if (rc == QAPI_OK)
    {
        if (count == 0)
        {
            LOG_INFO("Routing table is empty.\n");
        }
        else
        {
            buf = malloc(count * sizeof(qapi_Net_IPv6_Route_t));
            if (buf == NULL)
            {
                rc = QAPI_ERROR;
                LOG_ERROR("FAILED: route6\n");
            }

            rc = qapi_Net_IPv6_Routing_Table_Get(buf, &count);
            if (rc == QAPI_OK)
            {
                LOG_INFO("Destination           Nexthop              iface\n");
                for (i = 0; i < count; ++i)
                {
                    LOG_INFO("%s", inet_ntop(AF_INET6, buf->ipv6RouteDest, dest_str, sizeof(dest_str)));
                    LOG_INFO("Destination address : %s\n", dest_str);
                    rc = strcasecmp(dest_str,addr);
                    LOG_INFO("Rc value: %d\n", rc);
                    if (rc == 0)
                    {
                        ret = 1;
                        break;
                    }
                }
            }
            free(buf);
        }
    }
    return ret;
}

