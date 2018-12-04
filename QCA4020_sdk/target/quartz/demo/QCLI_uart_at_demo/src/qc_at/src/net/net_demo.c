/*
 * Copyright (c) 2016-2018 Qualcomm Technologies, Inc.
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

 
#include <stdio.h>
#include <stdlib.h>
#include "qcli.h"
#include "qcli_util.h"
#include "qurt_thread.h"
#include "qurt_timer.h"
#include "qapi_socket.h"
#include "qapi_netservices.h"
#include "net_bench.h"
#include "qapi_wlan_bridge.h"
#include "net_demo.h"
#include "net_iperf.h"
#include "qapi_netprofile.h"
#include "qapi_omtm.h"
#include "qapi_httpsvr.h"
#include "qapi_netbuf.h"
#include "qapi_ns_utils.h"
#include "qapi_ns_gen_v4.h"
#include "qapi_crypto.h"

#define MAX_FQDN_LEN   255
#define DHCP_INFINITE_LEASE_TIME "infinite"

QCLI_Group_Handle_t atn_group;
static qapi_Net_SSL_Config_t * httpsvr_sslcfg;
static char msg_invalid_id[] = "Invalid server id\n";
extern QCLI_Context_t QCLI_Context;
int32_t sockid = -1;
/*****************************************************************************
 * addr mask and gw are in network order.
 *****************************************************************************/
static int32_t ipconfig_dhcpc_success_cb(uint32_t addr, uint32_t mask, uint32_t gw)
{
    char ip_str[20];
    char mask_str[20];
    char gw_str[20];

    QCLI_Printf("DHCPv4c: IP=%s  Subnet Mask=%s  Gateway=%s\n",
            inet_ntop(AF_INET, &addr, ip_str, sizeof(ip_str)),
            inet_ntop(AF_INET, &mask, mask_str, sizeof(mask_str)),
            inet_ntop(AF_INET, &gw, gw_str, sizeof(gw_str)));

    return 0;
}

static void DHCPv6c_New_Lease_Complete_CB(
    void * cb_ctxt,
    const char * interface_name,
    const ip6_addr * ip6addr
    )
{
    char ip_str[48];
    QCLI_Printf("\nDHCPv6c_New_Lease_Complete_CB: interface=%s, ipv6=%s\n",
                interface_name,
                inet_ntop(AF_INET6, ip6addr, ip_str, sizeof(ip_str)));
}

static void DHCPv6c_Release_Lease_Complete_CB(
    void * cb_ctxt,
    const char * interface_name,
    const ip6_addr * ip6addr
    )
{
    char ip_str[48];
    QCLI_Printf("\nDHCPv6c_Release_Lease_Complete_CB: interface=%s, ipv6=%s\n",
                interface_name,
                inet_ntop(AF_INET6, ip6addr, ip_str, sizeof(ip_str)));
}

static void DHCPv6c_New_Prefix_Lease_Complete_CB(
    void * cb_ctxt,
    const char * interface_name,
    const ip6_addr * prefix_base,
    const uint8_t prefix_length
    )
{
    char ip_str[48];
    QCLI_Printf("\nDHCPv6c_New_Prefix_Lease_Complete_CB: interface=%s, prefix=%s/%d\n",
                interface_name,
                inet_ntop(AF_INET6, prefix_base, ip_str, sizeof(ip_str)),
                prefix_length);
}

/*****************************************************************************
 *****************************************************************************/
static void DHCPv6c_Release_Prefix_Lease_Complete_CB(
    void * cb_ctxt,
    const char * interface_name,
    const ip6_addr * prefix_base,
    const uint8_t prefix_length
    )
{
    char ip_str[48];
    QCLI_Printf("\nDHCPv6c_Release_Prefix_Lease_Complete_CB: interface=%s, prefix=%s/%d\n",
                interface_name,
                inet_ntop(AF_INET6, prefix_base, ip_str, sizeof(ip_str)),
                prefix_length);
}

static int32_t display_interface_info(char *interface_name, qbool_t if_is_up)
{
    ip6_addr v6Global, v6GlobalExtd, v6LinkLocal, v6DefGw;
    uint32_t LinkPrefix, GlobalPrefix, DefGwPrefix, GlobalExtdPrefix;
    qapi_Net_DNS_Server_List_t svr_list;
    char msg_ipv4_target_error[] = "Operation failed";

    QCLI_Printf("%s: %s\n", interface_name,
                if_is_up ? "UP" : "DOWN");

    if (strncmp(interface_name, "wlan", 4) == 0 ||
        strcmp(interface_name, "wlbr0") == 0)
    {
        /* MAC */
        uint32_t addr, mask, gw;
        int i, err = -1;
        const uint8_t * physical_address;
        uint32_t physical_address_length;

        if ((err = qapi_Net_Interface_Get_Physical_Address(
                        interface_name,
                        &physical_address,
                        &physical_address_length
                        )) == 0)
        {
            QCLI_Printf(" Phy Address: %02X", physical_address[0]);
            for ( i = 1; i < physical_address_length; i++ )
            {
                QCLI_Printf(":%02X", physical_address[i]);
            }
            QCLI_Printf("\n");
        }

        /* IPv4 */
        if ((err = qapi_Net_IPv4_Config(interface_name, QAPI_NET_IPV4CFG_QUERY_E, &addr, &mask, &gw)) == 0)
        {
            char ip_str[20], mask_str[20], gw_str[20];

            QCLI_Printf(" IPv4: %s  Subnet Mask: %s  Default Gateway: %s\n",
                        inet_ntop(AF_INET, &addr, ip_str, sizeof(ip_str)),
                        inet_ntop(AF_INET, &mask, mask_str, sizeof(mask_str)),
                        inet_ntop(AF_INET, &gw, gw_str, sizeof(gw_str)));
        }

        if (err == QAPI_NET_ERR_SOCKET_CMD_TIME_OUT)
        {
            QCLI_Printf("%s\n", msg_ipv4_target_error);
        }
    }

    /* IPv6 */
    memset(&v6LinkLocal,0, sizeof(ip6_addr));
    memset(&v6Global,0, sizeof(ip6_addr));
    memset(&v6DefGw,0, sizeof(ip6_addr));
    memset(&v6GlobalExtd,0, sizeof(ip6_addr));
    LinkPrefix = GlobalPrefix = DefGwPrefix = GlobalExtdPrefix = 0;
    if (qapi_Net_IPv6_Get_Address(interface_name,
                (uint8_t *)&v6LinkLocal,
                (uint8_t *)&v6Global,
                (uint8_t *)&v6DefGw,
                (uint8_t *)&v6GlobalExtd,
                &LinkPrefix,
                &GlobalPrefix,
                &DefGwPrefix,
                &GlobalExtdPrefix) == 0)
    {
        char ip_str[48];

        if (v6LinkLocal.s_addr[0])
        {
            if (inet_ntop(AF_INET6, &v6LinkLocal, ip_str, sizeof(ip_str)) != NULL)
            {
                if (LinkPrefix)
                    QCLI_Printf(" IPv6 Link-local Address ..... : %s/%d\n", ip_str, LinkPrefix);
                else
                    QCLI_Printf(" IPv6 Link-local Address ..... : %s\n", ip_str);
            }

            if (inet_ntop(AF_INET6, &v6Global, ip_str, sizeof(ip_str)) != NULL)
            {
                if (GlobalPrefix)
                    QCLI_Printf(" IPv6 Global Address ......... : %s/%d\n", ip_str, GlobalPrefix);
                else
                    QCLI_Printf(" IPv6 Global Address ......... : %s\n", ip_str);
            }

            if (inet_ntop(AF_INET6, &v6DefGw, ip_str, sizeof(ip_str)) != NULL)
            {
                if (DefGwPrefix)
                    QCLI_Printf(" IPv6 Default Gateway  ....... : %s/%d\n", ip_str, DefGwPrefix);
                else
                    QCLI_Printf(" IPv6 Default Gateway  ....... : %s\n", ip_str);

            }

            if (inet_ntop(AF_INET6, &v6GlobalExtd, ip_str, sizeof(ip_str)) != NULL)
            {
                if (GlobalExtdPrefix)
                    QCLI_Printf(" IPv6 Global Address 2 ....... : %s/%d\n", ip_str, GlobalExtdPrefix);
                else
                    QCLI_Printf(" IPv6 Global Address 2 ....... : %s\n", ip_str);
            }
        }
    }

    /* DNS */
    memset(&svr_list, 0, sizeof(qapi_Net_DNS_Server_List_t));
    if (qapi_Net_DNSc_Get_Server_List(&svr_list) == 0)
    {
        int i;
        char ip_str[48];

        for (i = 0; i < QAPI_NET_DNS_SERVER_MAX; ++i)
        {
            if (svr_list.svr[i].type != AF_UNSPEC)
            {
                QCLI_Printf(" DNS Server: %s\n",
                            inet_ntop(svr_list.svr[i].type, &svr_list.svr[i].a, ip_str, sizeof(ip_str)));
            }
        }
    }

    return 0;
}

QCLI_Command_Status_t net_Help(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    QCLI_Command_Status_t Ret_Val;
    uint32_t              Result;
    int32_t               Index;

    if(TAKE_LOCK(QCLI_Context.CLI_Mutex))
    {
        Result = Display_Help(QCLI_Context.Executing_Group, Parameter_Count, Parameter_List);

        /* if there was an error parsing the command list, print out an error
           message here (this is in addition to the usage message that will be
           printed out). */
        if(Result > 0)
        {
            QCLI_Printf("Command \"%s", Parameter_List[0].String_Value);

            for(Index = 1; Index < Result; Index ++)
            {
                QCLI_Printf(" %s", Parameter_List[Index].String_Value);
            }

            QCLI_Printf("\" not found.\n");

            Ret_Val = QCLI_STATUS_USAGE_E;
        }
        else
        {
            Ret_Val = QCLI_STATUS_SUCCESS_E;
        }

        RELEASE_LOCK(QCLI_Context.CLI_Mutex);
    }
    else
    {
        Ret_Val = QCLI_STATUS_ERROR_E;
    }

    return(Ret_Val);
}

QCLI_Command_Status_t net_Ping(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    char ip_str[16];
    uint32_t i, count = 1, size = 64;
    struct ip46addr addr;
    char * host;
    int e;

    if (Parameter_Count < 1 || Parameter_List == NULL)
    {
        return QCLI_STATUS_USAGE_E;
    }

    for (i = 1; i < Parameter_Count ; i++)
    {
        if (strcmp(Parameter_List[i].String_Value, "-c") == 0)
        {
            ++i;
            if (Parameter_List[i].Integer_Is_Valid)
            {
                count = Parameter_List[i].Integer_Value;
            }
        }
        else if (strcmp(Parameter_List[i].String_Value, "-s") == 0)
        {
            ++i;
            if (Parameter_List[i].Integer_Is_Valid)
            {
                size = Parameter_List[i].Integer_Value;
                if (size > CFG_PACKET_SIZE_MAX_TX)
                {
                    QCLI_Printf("Size should be <= %d\n", CFG_PACKET_SIZE_MAX_TX);
                    return QCLI_STATUS_ERROR_E;
                }
            }
        }
    } /* for loop */

    host = (char *)Parameter_List[0].String_Value;
    if (inet_pton(AF_INET, host, &addr.a) != 0)   /* not in xxx.xxx.xxx.xxx format */
    {
        if (strlen(host) > MAX_FQDN_LEN)
        {
            QCLI_Printf("host name cannot be more then %u Bytes\n", MAX_FQDN_LEN);
            return QCLI_STATUS_ERROR_E;
        }

        addr.type = AF_INET;
        if (qapi_Net_DNSc_Reshost(host, &addr) != 0)
        {
            QCLI_Printf("Cannot resolve %s\n", host);
            return QCLI_STATUS_ERROR_E;
        }
        else
        {
            QCLI_Printf("Ping %s (%s):\n",
                    host,
                    inet_ntop(AF_INET, &addr.a, ip_str, sizeof(ip_str)));
        }
    }

    for (i = 0; i < count; i++)
    {
        uint32_t t1, t2, ms;

        t1 = app_get_time(NULL);

        e = qapi_Net_Ping(addr.a.addr4, size);
        if (e == 0)
        {
            t2 = app_get_time(NULL);
            ms = t2 - t1;
            QCLI_Printf("%d bytes from %s: seq=%d time=%u ms\n",
                    size, inet_ntop(AF_INET, &addr.a, ip_str, sizeof(ip_str)), i+1, ms);
            if (i != count - 1)
            {
                app_msec_delay(980);
            }
        }
        else if (e == QAPI_NET_ERR_INVALID_IPADDR)
        {
            QCLI_Printf("Invalid IP address\n");
            return QCLI_STATUS_ERROR_E;
        }
        else
        {
            QCLI_Printf("Request timed out\n");
        }
    }

    return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t net_Ifconfig(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int i, num, e;
    qapi_Net_Ifnameindex_t *p, *pp;
    char * interface_name;
    uint32_t addr, mask, gw, *gw_addr = NULL;
    qbool_t if_is_up;

    switch (Parameter_Count)
    {
        /* display all interfaces */
        case 0:
            num = qapi_Net_Get_All_Ifnames(NULL);
            if (num == 0)
            {
                /* If no interface, is interface down? */
                QCLI_Printf("Not able to get interface details.\r\n");
                QCLI_Printf("ERROR\r\n");
                return QCLI_STATUS_ERROR_E;
            }

            p = pp = malloc(num * sizeof(qapi_Net_Ifnameindex_t));
            if (p)
            {
                qapi_Net_Get_All_Ifnames(p);
                for (i = 0; i < num; ++i)
                {
                    display_interface_info(pp->interface_Name, pp->if_Is_Up);
                    ++pp;
                }

                free(p);
            }
            else
            {
                return QCLI_STATUS_USAGE_E;
            }

            break;

        /* display this interface */
        case 1:
            interface_name = Parameter_List[0].String_Value;
            if (qapi_Net_Interface_Exist(interface_name, &if_is_up))
            {
                display_interface_info(interface_name, if_is_up);
            }
            else
            {
                QCLI_Printf("%s does not exist\n", interface_name);
                return QCLI_STATUS_ERROR_E;
            }
            break;

        /* Set IPv4 address for WLAN interfaces */
        case 3:
        case 4:
            interface_name = Parameter_List[0].String_Value;
            if (strncmp(interface_name, "wlan", 4) != 0 &&
                strcmp(interface_name, "wlbr0") != 0)
            {
                QCLI_Printf("Not a WLAN interface\n");
                return QCLI_STATUS_ERROR_E;
            }

            /* interface's IP address */
            if (inet_pton(AF_INET, Parameter_List[1].String_Value, &addr) != 0)
            {
                QCLI_Printf("Invalid IP address\n");
                return QCLI_STATUS_ERROR_E;
            }

            /* subnet mask */
            if (inet_pton(AF_INET, Parameter_List[2].String_Value, &mask) != 0)
            {
                QCLI_Printf("Invalid subnet mask\n");
                return QCLI_STATUS_ERROR_E;
            }

            /* default gateway */
            if (Parameter_Count == 4)
            {
                if (inet_pton(AF_INET, Parameter_List[3].String_Value, &gw) != 0)
                {
                    QCLI_Printf("Invalid gateway address\n");
                    return QCLI_STATUS_ERROR_E;
                }

                gw_addr = &gw;
            }

            e = qapi_Net_IPv4_Config(interface_name, QAPI_NET_IPV4CFG_STATIC_IP_E, &addr, &mask, gw_addr);
            if (e != 0)
            {
                if (e == QAPI_NET_ERR_INVALID_IPADDR)
                {
                    QCLI_Printf("Invalid IP address\n");
                }
                else
                {
                    QCLI_Printf("Static IPv4 configure failed\n");
                }
                return QCLI_STATUS_ERROR_E;
            }
            break;

        default:
            return QCLI_STATUS_USAGE_E;
    }

    return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t net_DhcpV4Client(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    char *interface_name;
    char *cmd;
    qbool_t if_is_up = FALSE;

    if (Parameter_Count < 1 || Parameter_List == NULL)
    {
        return QCLI_STATUS_USAGE_E;
    }

    interface_name = Parameter_List[0].String_Value;
    if (qapi_Net_Interface_Exist(interface_name, &if_is_up) == FALSE ||
        if_is_up == FALSE)
    {
        QCLI_Printf("ERROR: %s does not exist or is DOWN.\n", interface_name);
        return QCLI_STATUS_ERROR_E;
    }

    cmd = Parameter_List[1].String_Value;

    if (Parameter_Count == 1 || strncmp(cmd, "new", 3) == 0)
    {
        if (qapi_Net_DHCPv4c_Register_Success_Callback(interface_name, ipconfig_dhcpc_success_cb) != 0 ||
            qapi_Net_IPv4_Config(interface_name, QAPI_NET_IPV4CFG_DHCP_IP_E, NULL, NULL, NULL) != 0)
        {
            QCLI_Printf("ERROR: DHCPv4 new failed\n");
            return QCLI_STATUS_ERROR_E;
        }
    }
    else if (strncmp(cmd, "release", 3) == 0)
    {
        if (qapi_Net_DHCPv4c_Release(interface_name) != 0)
        {
            QCLI_Printf("ERROR: DHCPv4 release failed\n");
            return QCLI_STATUS_ERROR_E;
        }
    }

    return QCLI_STATUS_SUCCESS_E;
}

static int32_t ipconfig_dhcps_success_cb(uint8_t *macaddr, uint32_t ipaddr)
{
    if (macaddr != NULL)
    {
        char ip_str[20];

        QCLI_Printf("DHCPv4s: Client IP=%s  Client MAC=%02x:%02x:%02x:%02x:%02x:%02x\n",
                inet_ntop(AF_INET, &ipaddr, ip_str, sizeof(ip_str)),
                macaddr[0], macaddr[1], macaddr[2], macaddr[3], macaddr[4], macaddr[5]);
    }

    return 0;
}

/*****************************************************************************
 *           [0]   [1] [2]           [3]           [4]
 * Dhcpv4s wlan0  pool 192.168.1.10  192.168.1.50  3600
 *****************************************************************************/
static QCLI_Command_Status_t net_DhcpV4server(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    char *interface_name, *cmd;
    char *start_ip_addr_string;
    char *end_ip_addr_string;
    uint32_t startaddr, endaddr;
    uint32_t addr, mask;
    int leasetime = 0xFFFFFFFF;   /* very very long */
    int e;

    if (Parameter_Count < 4 || Parameter_Count > 5 ||
        Parameter_List == NULL)
    {
        return QCLI_STATUS_USAGE_E;
    }

    interface_name  = Parameter_List[0].String_Value;
    cmd             = Parameter_List[1].String_Value;
    if (strncmp(cmd, "poo", 3) == 0)
    {
        start_ip_addr_string    = Parameter_List[2].String_Value;
        end_ip_addr_string      = Parameter_List[3].String_Value;
        if (Parameter_Count == 5 &&
        	!Parameter_List[4].Integer_Is_Valid &&
        	(strcmp(Parameter_List[4].String_Value, DHCP_INFINITE_LEASE_TIME) != 0)) {
            QCLI_Printf("Invalid lease time specified, setting to default value\n");
        }
        if (Parameter_Count == 5 && Parameter_List[4].Integer_Is_Valid)
        {
            leasetime = Parameter_List[4].Integer_Value;
        }

        e = inet_pton(AF_INET, start_ip_addr_string, &startaddr);
        if (e != 0)
        {
            QCLI_Printf("Invalid start address\n");
            return QCLI_STATUS_ERROR_E;
        }
        startaddr = ntohl(startaddr);

        e = inet_pton(AF_INET, end_ip_addr_string, &endaddr);
        if (e != 0)
        {
            QCLI_Printf("Invalid end address\n");
            return QCLI_STATUS_ERROR_E;
        }
        endaddr = ntohl(endaddr);

        if (startaddr >= endaddr)
        {
            QCLI_Printf("Start address must be less than end address\n");
            return QCLI_STATUS_ERROR_E;
        }

        /* Get interface's IP address */
        e = qapi_Net_IPv4_Config(interface_name, QAPI_NET_IPV4CFG_QUERY_E, &addr, &mask, NULL);
        if (e != 0)
        {
            QCLI_Printf("Getting interface address failed\n");
            return QCLI_STATUS_ERROR_E;
        }
        addr = ntohl(addr);
        mask = ntohl(mask);

        if ((addr & mask) != (startaddr & mask) ||
            (addr & mask) != (endaddr & mask))
        {
            QCLI_Printf("Pool IP and interface IP should be in the same subnet\n");
            return QCLI_STATUS_ERROR_E;
        }

        if ((addr >= startaddr) && (addr <= endaddr))
        {
            QCLI_Printf("Please configure pool beyond interface IP address\n");
            return QCLI_STATUS_ERROR_E;
        }

        if (qapi_Net_DHCPv4s_Register_Success_Callback(interface_name, ipconfig_dhcps_success_cb) != 0 ||
            qapi_Net_DHCPv4s_Set_Pool(interface_name, htonl(startaddr), htonl(endaddr), leasetime) != 0)
        {
            QCLI_Printf("Config DHCP pool failed\n");
            return QCLI_STATUS_ERROR_E;
        }
    }
    else if(strncmp(cmd, "stop", 4) == 0)
    {
        QCLI_Printf("DHCP Server stop not supported: %s\n", cmd);
        return QCLI_STATUS_ERROR_E;
    }
    else
    {
        QCLI_Printf("Invalid command: %s\n", cmd);
        return QCLI_STATUS_ERROR_E;
    }

    return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t net_AutoIp(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    char *interface_name;

    if (Parameter_Count < 1 || Parameter_List == NULL)
    {
        return QCLI_STATUS_USAGE_E;
    }

    interface_name = Parameter_List[0].String_Value;

    if (qapi_Net_IPv4_Config(interface_name, QAPI_NET_IPV4CFG_AUTO_IP_E, NULL, NULL, NULL) != 0)
    {
        QCLI_Printf("Auto IPv4 failed\n");
        return QCLI_STATUS_ERROR_E;
    }

    return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t net_PingV6(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    uint8_t *ip6addr;
    struct ip46addr addr;
    char ip_str[48];
    uint32_t i, count = 1, size = 64;
    char * ifname = NULL;
    char * host;
    uint32_t is_ifname_specification_needed = 0;
    int32_t e;

    if (Parameter_Count < 1 || Parameter_List == NULL)
    {
        return QCLI_STATUS_USAGE_E;
    }

    for (i = 1; i < Parameter_Count ; i++)
    {
        if (strcmp(Parameter_List[i].String_Value, "-c") == 0)
        {
            ++i;
            if (Parameter_List[i].Integer_Is_Valid)
            {
                count = Parameter_List[i].Integer_Value;
            }
        }
        else if (strcmp(Parameter_List[i].String_Value, "-s") == 0)
        {
            ++i;
            if (Parameter_List[i].Integer_Is_Valid)
            {
                size = Parameter_List[i].Integer_Value;
                if (size > CFG_PACKET_SIZE_MAX_TX)
                {
                    QCLI_Printf("Size should be <= %d\n", CFG_PACKET_SIZE_MAX_TX);
                    return QCLI_STATUS_ERROR_E;
                }
            }
        }
        else if (strcmp(Parameter_List[i].String_Value, "-I") == 0)
        {
            ++i;
            ifname = Parameter_List[i].String_Value;
        }
    } /* for loop */

    host = (char *)Parameter_List[0].String_Value;
    if (inet_pton(AF_INET6, host, &addr.a) != 0)    /* not IPv6 address */
    {
        if (strlen(host) > MAX_FQDN_LEN)
        {
            QCLI_Printf("host name cannot be more then %u Bytes\n", MAX_FQDN_LEN);
            return QCLI_STATUS_ERROR_E;
        }

        addr.type = AF_INET6;
        if (qapi_Net_DNSc_Reshost(host, &addr) != 0)
        {
            QCLI_Printf("Invalid IP address\n");
            return QCLI_STATUS_ERROR_E;
        }
        else
        {
            QCLI_Printf("Ping %s (%s):\n",
                    host,
                    inet_ntop(AF_INET6, &addr.a, ip_str, sizeof(ip_str)));
        }
    }

    ip6addr = (uint8_t *)&addr.a;
    if (QAPI_IS_IPV6_MULTICAST(ip6addr) ||
        QAPI_IS_IPV6_LINK_LOCAL(ip6addr))
    {
        is_ifname_specification_needed = 1;
    }

    if (is_ifname_specification_needed && ifname == NULL)
    {
        QCLI_Printf("Error: The specified IPv6 address is either multicast or link-local, please specify source interface name using the -I option\n");
        return QCLI_STATUS_ERROR_E;
    }

    for (i = 0; i < count; i++)
    {
        uint32_t t1, t2, ms;

        t1 = app_get_time(NULL);

        e = qapi_Net_Ping6(ip6addr, size, ifname);
        if (e == 0)
        {
            t2 = app_get_time(NULL);
            ms = t2 - t1;
            QCLI_Printf("%d bytes from %s: seq=%d time=%u ms\n",
                    size, inet_ntop(AF_INET6, &addr.a, ip_str, sizeof(ip_str)), i+1, ms);
        }
        else if (e == 1)
        {
            QCLI_Printf("Request timed out\n");
        }
        else
        {
            QCLI_Printf("PING failed\n");
            return QCLI_STATUS_ERROR_E;
        }

        #define PING6_DELAY_DURATION_IN_MS 1000

        if ( i < count - 1 ) {
             app_msec_delay(PING6_DELAY_DURATION_IN_MS);
        }
    }

    return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t net_DhcpV6Client(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    char * ifname;
    char * cmd;

    if (Parameter_Count < 2 || Parameter_List == NULL)
    {
        return QCLI_STATUS_USAGE_E;
    }

    ifname  = Parameter_List[0].String_Value;
    cmd     = Parameter_List[1].String_Value;

    if (strncmp(cmd, "enable", 3) == 0)
    {
        if (qapi_Net_DHCPv6c_Register_New_IPv6_Lease_Complete_Callback(
                    ifname, DHCPv6c_New_Lease_Complete_CB, NULL) != 0 ||
            qapi_Net_DHCPv6c_Register_Release_IPv6_Lease_Complete_Callback(
                    ifname, DHCPv6c_Release_Lease_Complete_CB, NULL) != 0 ||
            qapi_Net_DHCPv6c_Enable(ifname) != 0)
        {
            goto fail;
        }
    }
    else if (strncmp(cmd, "disable", 3) == 0)
    {
        if (qapi_Net_DHCPv6c_Disable(ifname) != 0)
        {
            goto fail;
        }
    }
    else if (strncmp(cmd, "new", 3) == 0)
    {
        char * ifname_to_obtain_ia_for = Parameter_List[2].String_Value;
        int are_same_interfaces = (strcmp(ifname, ifname_to_obtain_ia_for) == 0);
        if ( !are_same_interfaces )
        {
            int status_code =
                qapi_Net_DHCPv6c_Register_New_Prefix_Lease_Complete_Callback(
                    ifname_to_obtain_ia_for,
                    DHCPv6c_New_Prefix_Lease_Complete_CB,
                    NULL
                    );
            if ( status_code ) {
                goto fail;
            }
            status_code =
                qapi_Net_DHCPv6c_Register_Release_Prefix_Lease_Complete_Callback(
                    ifname_to_obtain_ia_for,
                    DHCPv6c_Release_Prefix_Lease_Complete_CB,
                    NULL
                    );
            if ( status_code ) {
                goto fail;
            }
        }
        if (qapi_Net_DHCPv6c_New_Lease(ifname, ifname_to_obtain_ia_for) != 0)
        {
            goto fail;
        }
    }
    else if (strncmp(cmd, "release", 3) == 0)
    {
        char * ifname_to_obtain_ia_for = Parameter_List[2].String_Value;
        if (qapi_Net_DHCPv6c_Release_Lease(ifname, ifname_to_obtain_ia_for) != 0)
        {
            goto fail;
        }
    }
    else if (strncmp(cmd, "confirm", 3) == 0)
    {
        char * ifname_to_obtain_ia_for = Parameter_List[2].String_Value;
        if (qapi_Net_DHCPv6c_Confirm_Lease(ifname, ifname_to_obtain_ia_for) != 0)
        {
            goto fail;
        }
    }
    else
    {
        QCLI_Printf("Invalid command: %s\n", cmd);
        return QCLI_STATUS_ERROR_E;
    }

    return QCLI_STATUS_SUCCESS_E;

fail:
    QCLI_Printf("%s failed\n", cmd);
    return QCLI_STATUS_ERROR_E;
}

QCLI_Command_Status_t net_SntpClient(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int32_t e = -1;
    int32_t id;
    char msg_sntp_not_started[] = "SNTP client is not started yet.";
    char msg_name_too_long[] = "Domain name or address cannot be more then 64 bytes\n";
    char *cmd, *svr;

    /* Sntpc */
    if (Parameter_Count == 0)
    {
        int32_t i = 0;
        int32_t started;
        qapi_Net_SNTP_Server_List_t svr_list;

        started = qapi_Net_SNTPc_Is_Started();
        QCLI_Printf("SNTP client is %s.\n", started ? "started" : "stopped");
        if (qapi_Net_SNTPc_Get_Server_List(&svr_list) == 0)
        {
            for (i = 0; i < QAPI_NET_SNTP_SERVER_MAX; ++i)
            {
                QCLI_Printf("%d: ", i);
                QCLI_Printf("%s    ", svr_list.svr[i].name[0] != '\0' ? svr_list.svr[i].name : "****");
                QCLI_Printf("%s  ", svr_list.svr[i].addr[0] != '\0' ? svr_list.svr[i].addr : "****");
                if (svr_list.svr[i].status == QAPI_NET_SNTP_SERVER_STATUS_KOD)
                {
                    QCLI_Printf("KOD");
                }
                else if (svr_list.svr[i].status == QAPI_NET_SNTP_SERVER_STATUS_NORESP)
                {
                    QCLI_Printf("NORESP");
                }
                QCLI_Printf("\n");
            }
        }

        /* If not started, we want to display cmd syntax */
        if (!started)
        {
            return QCLI_STATUS_USAGE_E;
        }

        return QCLI_STATUS_SUCCESS_E;
    }

    cmd = Parameter_List[0].String_Value;

    /*       [0] [1]                                    [2]
     * Sntpc add 10.234.20.15
     * Sntpc add 2002:c023:9c17:303:edd3:6b74:5915:dbe7  1
     * Sntpc add na.qualcomm.com                         0
     */
    if (strncmp(cmd, "addsvr", 3) == 0)
    {
        if (Parameter_Count < 2)
        {
            goto parm_error;
        }

        if (!qapi_Net_SNTPc_Is_Started())
        {
            QCLI_Printf("%s\n", msg_sntp_not_started);
            return QCLI_STATUS_ERROR_E;
        }

        svr = Parameter_List[1].String_Value;

        if (strlen(svr) > 64)
        {
            QCLI_Printf("%s\n", msg_name_too_long);
            return QCLI_STATUS_ERROR_E;
        }

        if (Parameter_Count == 2)
        {
            id = QAPI_NET_SNTP_ANY_SERVER_ID;
        }
        else if (Parameter_List[2].Integer_Is_Valid)
        {
            id = Parameter_List[2].Integer_Value;
        }
        else
        {
            QCLI_Printf("%s\n", msg_invalid_id);
            return QCLI_STATUS_ERROR_E;
        }

        e = qapi_Net_SNTPc_Add_Server(svr, id);
        if (e == QAPI_NET_ERR_INVALID_IPADDR)
        {
            QCLI_Printf("Invalid server address\n");
        }
        else if (e == QAPI_NET_ERR_CANNOT_GET_SCOPEID)
        {
            QCLI_Printf("Failed to get scope id\n");
        }
    }

    /* Sntpc  del  1 */
    else if (strncmp(cmd, "delsvr", 3) == 0)
    {
        if (Parameter_Count < 2)
        {
            goto parm_error;
        }

        if (!qapi_Net_SNTPc_Is_Started())
        {
            QCLI_Printf("%s\n", msg_sntp_not_started);
            return QCLI_STATUS_ERROR_E;
        }

        if (Parameter_List[1].Integer_Is_Valid)
        {
            id = Parameter_List[1].Integer_Value;
        }
        else
        {
            QCLI_Printf("%s\n", msg_invalid_id);
            return QCLI_STATUS_ERROR_E;
        }

        e = qapi_Net_SNTPc_Del_Server(id);
    }

    /* Sntpc  start */
    else if (strncmp(cmd, "start", 3) == 0)
    {
        e = qapi_Net_SNTPc_Command(QAPI_NET_SNTP_START_E);
    }

    /* Sntpc  stop */
    else if (strncmp(cmd, "stop", 3) == 0)
    {
        e = qapi_Net_SNTPc_Command(QAPI_NET_SNTP_STOP_E);
    }

    /* Sntpc  disable */
    else if (strncmp(cmd, "disable", 3) == 0)
    {
        e = qapi_Net_SNTPc_Command(QAPI_NET_SNTP_DISABLE_E);
    }

    /* Sntpc  utc */
    else if (strncmp(cmd, "utc", 3) == 0)
    {
        char *mon_str[12] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
        char *day_str[7] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
        qapi_Net_SNTP_Tm_t tm;

        e = qapi_Net_SNTPc_Get_Brokendown_Time(&tm);
        if (!e)
        {
            /*  Dec 1, 2015 Mon 18:09:41 UTC */
            QCLI_Printf("%s %u, %02u %s %02u:%02u:%02u UTC\n",
                    mon_str[tm.tm_mon],
                    tm.tm_mday,
                    tm.tm_year + 1900,
                    day_str[tm.tm_wday],
                    tm.tm_hour,
                    tm.tm_min,
                    tm.tm_sec);
        }
    }
    else
    {
        QCLI_Printf("Invalid subcommand: %s\n", cmd);
        goto parm_error;
    }

    if (e)
    {
        QCLI_Printf("%s failed\n", cmd);
        return QCLI_STATUS_ERROR_E;
    }
    else
    {
        return QCLI_STATUS_SUCCESS_E;
    }

parm_error:
    return QCLI_STATUS_USAGE_E;
}

QCLI_Command_Status_t net_DnsClient(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int32_t e = -1;
    uint32_t id;
    char *cmd, *svr;

    /* Dnsc */
    if (Parameter_Count == 0)
    {
        int32_t i;
        int32_t started;
        qapi_Net_DNS_Server_List_t svr_list;
        char ip_str[48];

        started = qapi_Net_DNSc_Is_Started();
        QCLI_Printf("DNS client is %s.\n", started ? "started" : "stopped");
        e = qapi_Net_DNSc_Get_Server_List(&svr_list);
        if (e == 0)
        {
            for (i = 0; i < QAPI_NET_DNS_SERVER_MAX; ++i)
            {
                if (svr_list.svr[i].type == AF_UNSPEC)
                {
                    QCLI_Printf("%d: ****\n", i);
                }
                else
                {
                    QCLI_Printf("%d: %s\n", i,
                            inet_ntop(svr_list.svr[i].type, &svr_list.svr[i].a, ip_str, sizeof(ip_str)));
                }
            }
        }

        /* If not started, we want to display cmd syntax */
        if (!started)
        {
            return QCLI_STATUS_USAGE_E;
        }

        return QCLI_STATUS_SUCCESS_E;
    }

    cmd = Parameter_List[0].String_Value;

    /*      [0]   [1]           [2]
     * Dnsc add   192.168.1.30
     * Dnsc add   192.168.1.30   1
     */
    if (strncmp(cmd, "addsvr", 3) == 0)
    {
        if (Parameter_Count < 2)
        {
            goto parm_error;
        }

        svr = Parameter_List[1].String_Value;
        if (strlen(svr) > 48)
        {
            QCLI_Printf("address cannot be more then 48 bytes\n");
            return QCLI_STATUS_ERROR_E;
        }

        if (Parameter_Count == 2)
        {
            id = QAPI_NET_DNS_ANY_SERVER_ID;
        }
        else if (Parameter_List[2].Integer_Is_Valid)
        {
            id = Parameter_List[2].Integer_Value;
        }
        else
        {
            QCLI_Printf("%s\n", msg_invalid_id);
            return QCLI_STATUS_ERROR_E;
        }

        e = qapi_Net_DNSc_Add_Server(svr, id);
        if (e == QAPI_NET_ERR_INVALID_IPADDR)
        {
            QCLI_Printf("Invalid server address\n");
        }
        else if (e == QAPI_NET_ERR_CANNOT_GET_SCOPEID)
        {
            QCLI_Printf("Failed to get scope id\n");
        }
    }

    /*
     * Dnsc del  1
     */
    else if (strncmp(cmd, "delsvr", 3) == 0)
    {
        if (Parameter_Count < 2)
        {
            goto parm_error;
        }

        if (Parameter_List[1].Integer_Is_Valid)
        {
            id = Parameter_List[1].Integer_Value;
        }
        else
        {
            QCLI_Printf("%s\n", msg_invalid_id);
            return QCLI_STATUS_ERROR_E;
        }

        e = qapi_Net_DNSc_Del_Server(id);
    }

    /* Dnsc start */
    else if (strncmp(cmd, "start", 3) == 0)
    {
        e = qapi_Net_DNSc_Command(QAPI_NET_DNS_START_E);
    }

    /* Dnsc stop */
    else if (strncmp(cmd, "stop", 3) == 0)
    {
        e = qapi_Net_DNSc_Command(QAPI_NET_DNS_STOP_E);
    }

    /* Dnsc disable */
    else if (strncmp(cmd, "disable", 3) == 0)
    {
        e = qapi_Net_DNSc_Command(QAPI_NET_DNS_DISABLE_E);
    }

    /*      [0]     [1]          [2]
     * Dnsc resolve <hostname>
     * Dnsc resolve <hostname>   v4
     * Dnsc resolve <hostname>   v6
     */
    else if (strncmp(cmd, "resolve", 3) == 0)
    {
        struct ip46addr ipaddr;
        char ip_str[48];

        if (Parameter_Count < 2)
        {
            goto parm_error;
        }

        if  (!qapi_Net_DNSc_Is_Started())
        {
            QCLI_Printf("DNS client is not started yet.\n");
            return QCLI_STATUS_ERROR_E;
        }

        svr = Parameter_List[1].String_Value;
        memset(&ipaddr, 0, sizeof(ipaddr));
        if (Parameter_Count == 2 || strcmp(Parameter_List[2].String_Value, "v4") == 0)
        {
            ipaddr.type = AF_INET;
        }
        else if (strcmp(Parameter_List[2].String_Value, "v6") == 0)
        {
            ipaddr.type = AF_INET6;
        }
        else
        {
            QCLI_Printf("Resolve to \"v4\" or \"v6\"?\n");
            goto parm_error;
        }

        e = qapi_Net_DNSc_Reshost(svr, &ipaddr);
        if (e)
        {
            QCLI_Printf("Unable to resolve %s\n", svr);
        }
        else
        {
            QCLI_Printf("\n%s --> %s\n", svr, inet_ntop(ipaddr.type, &ipaddr.a, ip_str, sizeof(ip_str)));
        }
    }

    /*      [0]            [1]
     * Dnsc gethostbyname  192.168.1.30
     * Dnsc gethostbyname  www.qualcomm.com
     */
    else if (strcmp(cmd, "gethostbyname") == 0)
    {
        char ip_str[20];
        struct qapi_hostent_s * he;

        if (Parameter_Count < 2)
        {
            goto parm_error;
        }

        if  (!qapi_Net_DNSc_Is_Started())
        {
            QCLI_Printf("DNS client is not started yet.\n");
            return QCLI_STATUS_ERROR_E;
        }

        svr = Parameter_List[1].String_Value;
        he = gethostbyname(svr);
        if (he == NULL)
        {
            QCLI_Printf("Unable to resolve %s\n", svr);
        }
        else
        {
            QCLI_Printf("\n%s -->", he->h_name);
            while (*(he->h_addr_list) != NULL)
            {
                QCLI_Printf(" %s", inet_ntop(AF_INET, *(he->h_addr_list), ip_str, sizeof(ip_str)));
                ++(he->h_addr_list);
            }
            QCLI_Printf("\n");
            e = 0;
        }
    }

    /*      [0]            [1]               [2]
     * Dnsc gethostbyname2  192.168.1.30
     * Dnsc gethostbyname2  www.qualcomm.com [v4]
     */
    else if (strcmp(cmd, "gethostbyname2") == 0)
    {
        char ip_str[48];
        struct qapi_hostent_s * he;
        int af;

        if (Parameter_Count < 2)
        {
            goto parm_error;
        }

        if  (!qapi_Net_DNSc_Is_Started())
        {
            QCLI_Printf("DNS client is not started yet.\n");
            return QCLI_STATUS_ERROR_E;
        }

        if (Parameter_Count == 2 || strcmp(Parameter_List[2].String_Value, "v4") == 0)
        {
            af = AF_INET;
        }
        else if (strcmp(Parameter_List[2].String_Value, "v6") == 0)
        {
            af = AF_INET6;
        }
        else
        {
            QCLI_Printf("Resolve to \"v4\" or \"v6\"?\n");
            goto parm_error;
        }

        svr = Parameter_List[1].String_Value;
        he = gethostbyname2(svr, af);
        if (he == NULL)
        {
            QCLI_Printf("Unable to resolve %s to IP%s address\n",
                    svr, (af == AF_INET) ? "v4" : "v6");
        }
        else
        {
            QCLI_Printf("\n%s -->", he->h_name);
            while (*(he->h_addr_list) != NULL)
            {
                QCLI_Printf(" %s", inet_ntop(af, *(he->h_addr_list), ip_str, sizeof(ip_str)));
                ++(he->h_addr_list);
            }
            QCLI_Printf("\n");
            e = 0;
        }
    }

    else
    {
        QCLI_Printf("Invalid subcommand: %s\n", cmd);
        goto parm_error;
    }

    if (e)
    {
        QCLI_Printf("%s failed\n", cmd);
        return QCLI_STATUS_ERROR_E;
    }
    else
    {
        return QCLI_STATUS_SUCCESS_E;
    }

parm_error:
    return QCLI_STATUS_USAGE_E;
}

QCLI_Command_Status_t net_Bridge(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int32_t e = -1;
    char *cmd;

    /* invalid options */
    if (Parameter_Count < 1 || Parameter_List == NULL)
    {
        goto parm_error;;
    }

    cmd = Parameter_List[0].String_Value;

    if ((strncmp(cmd, "enable", 3) == 0) || (strncmp(cmd, "disable", 3) == 0))
    {
        e = qapi_Net_Bridge_Enable((strncmp(cmd, "enable", 3) == 0)? 1: 0);
    }

    /* Set MAC entry age in bridge database*/
    else if (strncmp(cmd, "aging", 3) == 0)
    {
        if (Parameter_Count < 2)
        {
            goto parm_error;
        }

        e = qapi_Net_Bridge_Set_Aging_Timeout(Parameter_List[1].Integer_Value);
    }

    /* Dump bridge mac database entried*/
    else if (strncmp(cmd, "showmacs", 3) == 0)
    {
        qapi_Net_Bridge_DB_Entry_t *ptr;
        uint32_t count = 0;
        if((e = qapi_Net_Bridge_Show_MACs(&ptr, &count)) != -1)
        {
		   if(count == 0) {
			  QCLI_Printf("\nNo Entries\n");
		   }
		   else {
	           void *buf = (void *)ptr;
	           QCLI_Printf("\nDevId\tBridged MAC address\tTTL\n");
	           while(count){
	              QCLI_Printf("%02d\t\t%02X:%02X:%02X:%02X:%02X:%02X\t%d\n", ptr->dev_ID,
	                       ptr->addr[0], ptr->addr[1],ptr->addr[2],ptr->addr[3],ptr->addr[4],ptr->addr[5], ptr->age);
	              count--;
	              ptr++;
	           }
	           free(buf);
		   }
       }else{
           QCLI_Printf("Invalid command, use help\n");
       }
    }
    if (e)
    {
    	switch(e)
    	{
    	case QAPI_ERR_NOT_SUPPORTED:
    		QCLI_Printf("Operation not supported\n", cmd);
    		break;

    	case QAPI_ERR_NO_MEMORY:
    		QCLI_Printf("Out of memory\n", cmd);
    		break;

    	default:
    		QCLI_Printf("Command failed. Run ifconfig and verify wlan0 and wlan1 are up\n", cmd);

    	}

        return QCLI_STATUS_ERROR_E;
    }
    else
    {
        return QCLI_STATUS_SUCCESS_E;
    }

    parm_error:
    return QCLI_STATUS_USAGE_E;
}

QCLI_Command_Status_t net_SockClose(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    qapi_socketclose(sockid);
    return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t net_TcpConnect(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    uint16_t portnum;
    char *ipaddr = NULL;
    struct sockaddr_in addr = {0};
    int bytes;
    char *send_buf = NULL;
    uint32_t peer_pkt_byte = 512;
    uint32_t timeout = 0;
    uint32_t ticks = 0;

    if ((Parameter_Count != 4) || (!Parameter_List[0].Integer_Is_Valid) || (!Parameter_List) ||
        (Parameter_List[1].Integer_Is_Valid) || (!Parameter_List[2].Integer_Is_Valid) ||
        (!Parameter_List[3].Integer_Is_Valid))
    {
        qurt_thread_stop();
        return QCLI_STATUS_USAGE_E;
    }

    portnum = Parameter_List[0].Integer_Value;
    if ((!Parameter_List[1].Integer_Is_Valid))
    {
        ipaddr = Parameter_List[1].String_Value;
    }
    peer_pkt_byte = Parameter_List[2].Integer_Value;
    timeout = Parameter_List[3].Integer_Value;

    send_buf = malloc(peer_pkt_byte);
    memset(send_buf, 'A', peer_pkt_byte);

    sockid = qapi_socket(AF_INET, SOCK_STREAM, 0);
    if (sockid < 0)
    {
        QCLI_Printf("Failed to open socket %d\n", sockid);
	return QCLI_STATUS_ERROR_E;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(portnum);
    addr.sin_addr.s_addr = inet_addr(ipaddr);

    if (qapi_connect(sockid, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        QCLI_Printf("Failed to connect socket %d\n", sockid);
	return QCLI_STATUS_ERROR_E;
    }

    ticks = (uint32_t)qurt_timer_convert_ticks_to_time(qurt_timer_get_ticks(), QURT_TIME_MSEC);

    while ((uint32_t)qurt_timer_convert_ticks_to_time(qurt_timer_get_ticks(), QURT_TIME_MSEC) - ticks < (timeout * 1000))
    {
        memset(send_buf, 'A', peer_pkt_byte);
        bytes = qapi_send(sockid, send_buf, peer_pkt_byte, 0);
        QCLI_Printf("sent bytes%d\n", bytes);

        memset(send_buf, 0, peer_pkt_byte);
        bytes = qapi_recv(sockid, send_buf, peer_pkt_byte, 0);
        QCLI_Printf("Received bytes are: %d\ndata: %s\n", bytes, send_buf);
    }

    free(send_buf);
    qapi_socketclose(sockid);

    return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t net_UdpConnect(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    uint16_t portnum;
    char *ipaddr = NULL;
    struct sockaddr_in addr = {0};
    int32_t len = sizeof(addr);
    int bytes;
    char *send_buf = NULL;
    uint32_t peer_pkt_byte = 512;
    uint32_t timeout = 0;
    uint32_t ticks = 0;

    if ((Parameter_Count != 4) || (!Parameter_List[0].Integer_Is_Valid) || (!Parameter_List) ||
        (Parameter_List[1].Integer_Is_Valid) || (!Parameter_List[2].Integer_Is_Valid) ||
        (!Parameter_List[3].Integer_Is_Valid))
    {
        qurt_thread_stop();
        return QCLI_STATUS_USAGE_E;
    }

    portnum = Parameter_List[0].Integer_Value;
    if ((!Parameter_List[1].Integer_Is_Valid))
    {
        ipaddr = Parameter_List[1].String_Value;
    }
    peer_pkt_byte = Parameter_List[2].Integer_Value;
    timeout = Parameter_List[3].Integer_Value;

    send_buf = malloc(peer_pkt_byte);
    memset(send_buf, 'A', peer_pkt_byte);

    sockid = qapi_socket(AF_INET, SOCK_DGRAM, 0);
    if (sockid < 0)
    {
        QCLI_Printf("Failed to open socket %d\n", sockid);
	return QCLI_STATUS_ERROR_E;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(portnum);
    addr.sin_addr.s_addr = inet_addr(ipaddr);

    ticks = (uint32_t)qurt_timer_convert_ticks_to_time(qurt_timer_get_ticks(), QURT_TIME_MSEC);

    while ((uint32_t)qurt_timer_convert_ticks_to_time(qurt_timer_get_ticks(), QURT_TIME_MSEC) - ticks < (timeout * 1000))
    {
        memset(send_buf, 'A', peer_pkt_byte);
        bytes = qapi_sendto(sockid, send_buf, peer_pkt_byte, 0, (struct sockaddr *) &addr, len);
        QCLI_Printf("sent bytes%d\n", bytes);

        memset(send_buf, 0, peer_pkt_byte);
        bytes = qapi_recvfrom(sockid, send_buf, peer_pkt_byte, 0, (struct sockaddr *)&addr, &len);
        QCLI_Printf("Received bytes are: %d\ndata: %s\n", bytes, send_buf);

    }
    free(send_buf);
    qapi_socketclose(sockid);

    return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t net_TcpServer(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int ret;
    int bytes;
    uint16_t portnum;
    uint32_t timeout = 0;
    uint16_t echo = 0;
	int32_t len = 0;
    struct sockaddr_in addr = {0};
    struct sockaddr_in addr1 = {0};
    char recv_buf[1500] = {0};
    uint32_t ticks = 0;

    if ((Parameter_Count < 2) || (!Parameter_List[0].Integer_Is_Valid) || (!Parameter_List) ||
        (!Parameter_List[1].Integer_Is_Valid))
    {
        qurt_thread_stop();
        return QCLI_STATUS_USAGE_E;
    }

    portnum = Parameter_List[0].Integer_Value;

    if ((Parameter_Count == 2))
    {
        if(Parameter_List[1].Integer_Is_Valid)
            timeout = Parameter_List[1].Integer_Value;
        else
            echo = 1;
    }

    timeout = Parameter_List[1].Integer_Value;

    if ((Parameter_Count == 3))
    {
        echo = 1;
    }

    sockid = qapi_socket(AF_INET, SOCK_STREAM, 0);
    if (sockid < 0)
    {
        QCLI_Printf("Failed to open socket %d\n", sockid);
	return QCLI_STATUS_ERROR_E;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(portnum);
    addr.sin_addr.s_addr = inet_addr("0.0.0.0");

    ret = qapi_bind(sockid, (struct sockaddr *) &addr, sizeof(addr));
    if (ret < 0)
    {
        QCLI_Printf("Socket binding error %d\n", ret);
	return QCLI_STATUS_ERROR_E;
    }

    ret = qapi_listen(sockid, 3);
    if (ret < 0)
    {
        QCLI_Printf("Socket binding error %d\n", ret);
	return QCLI_STATUS_ERROR_E;
    }

	len = sizeof(addr1);
    QCLI_Printf("Going to accept\n", ret);
    ret = qapi_accept(sockid, (struct sockaddr *)&addr1, &len);
    if (ret <= 0)
    {
        QCLI_Printf("Socket accept error %d\n", ret);
        return QCLI_STATUS_ERROR_E;
    }
    QCLI_Printf("TCP Server started.........\n");

    ticks = (uint32_t)qurt_timer_convert_ticks_to_time(qurt_timer_get_ticks(), QURT_TIME_MSEC);

    while ((uint32_t)qurt_timer_convert_ticks_to_time(qurt_timer_get_ticks(), QURT_TIME_MSEC) - ticks < (timeout * 1000))
    {
        memset(recv_buf, 0, sizeof(recv_buf));
        bytes = qapi_recv(sockid, recv_buf, sizeof(recv_buf), 0);
        QCLI_Printf("Received bytes are: %d\n data:%s\n", bytes, recv_buf);

        if (echo)
        {
            QCLI_Printf("sendig data is %s\n", recv_buf);
            bytes = qapi_send(sockid, recv_buf, sizeof(recv_buf), 0);
            QCLI_Printf("sent bytes%d\n", bytes);

        }
    }
    return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t net_UdpServer(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int ret;
    int bytes;
    uint32_t timeout = 0;
    uint16_t echo = 0;
    uint16_t portnum;
    struct sockaddr_in addr;
    struct sockaddr_in addr1;
    char recv_buf[1500] = {0};
    int32_t len = 0;
    uint32_t ticks = 0;

    if ((Parameter_Count < 2) || (!Parameter_List[0].Integer_Is_Valid) || (!Parameter_List) ||
        (!Parameter_List[1].Integer_Is_Valid))
    {
        qurt_thread_stop();
        return QCLI_STATUS_USAGE_E;
    }

    portnum = Parameter_List[0].Integer_Value;

    timeout = Parameter_List[1].Integer_Value;

    if ((Parameter_Count == 3))
    {
        echo = 1;
    }

    sockid = qapi_socket(AF_INET, SOCK_DGRAM, 0);
    if (sockid < 0)
    {
        QCLI_Printf("Failed to open socket %d\n", sockid);
	return QCLI_STATUS_ERROR_E;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(portnum);
    addr.sin_addr.s_addr = inet_addr("0.0.0.0");

    ret = qapi_bind(sockid, (struct sockaddr *) &addr, sizeof(addr));
    if (ret < 0)
    {
        QCLI_Printf("Socket binding error %d\n", ret);
	return QCLI_STATUS_ERROR_E;
    }

    len = sizeof(addr1);
    QCLI_Printf("UDP Server started.........\n");

    ticks = (uint32_t)qurt_timer_convert_ticks_to_time(qurt_timer_get_ticks(), QURT_TIME_MSEC);

    while ((uint32_t)qurt_timer_convert_ticks_to_time(qurt_timer_get_ticks(), QURT_TIME_MSEC) - ticks < (timeout * 1000))
    {
        memset(recv_buf, 0, sizeof(recv_buf));
        bytes = qapi_recvfrom(sockid, recv_buf, sizeof(recv_buf), 0, (struct sockaddr *)&addr1, &len);
        QCLI_Printf("Received bytes are: %d\ndata: %s\n", bytes, recv_buf);

        if (echo)
        {
            QCLI_Printf("sendig data is %s\n", recv_buf);
            bytes = qapi_sendto(sockid, recv_buf, sizeof(recv_buf), 0, (struct sockaddr *) &addr1, len);
            QCLI_Printf("sent bytes%d\n", bytes);

        }
    }
    QCLI_Printf("Timeout UDP Server stoped.........\n");
    return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t net_HttpClient(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	if (0 == httpc_command_handler(Parameter_Count, Parameter_List)) {
		return QCLI_STATUS_SUCCESS_E;
	}
	return QCLI_STATUS_ERROR_E;
}

static void httpsvr_help(void)
{
    QCLI_Printf("httpsvr init [v4|v6|v46] [http|https|http_https] [-c <cert_file>] [-p <httpport>] [-s <httpsport>] [-i <ifname>] [-x <index_page>] [-r <root_path>]\n");
    QCLI_Printf("httpsvr [start|stop]\n");
    QCLI_Printf("httpsvr [addctype|delctype] <content-type1> [<content-type2> ..]\n");
    QCLI_Printf("httpsvr setbufsize <TX buffer size> <RX buffer size>\n");
    sslconfig_help("httpsvr sslconfig");

    QCLI_Printf("Examples:\n");
    QCLI_Printf(" httpsvr init v6 http\n");
    QCLI_Printf(" httpsvr init v4 http_https -c cert.bin\n");
    QCLI_Printf(" httpsvr init v46 https -c cert.bin -p 8080 -s 1443 -i wlan1 -x index.html -r /mywebfolder\n");
}

/*****************************************************************************
 *         [0]  [1] [2]   [3] [4]         [5] [6] [7] [8] [9] [10]   [11] [12]       [13] [14]
 * httpsvr init v4  https -c  ca.cert.bin -p  80  -s  443  -i  wlan1 -x   index.html -r   /web/
 * httpsvr init v4  http
 * httpsvr init v6  http -r /mywebfolder
 * httpsvr init v46 http -p 8080
 * httpsvr init v46 https -c cert
 * httpsvr setbufsize <txbufsize> <rxbufsize>
 * httpsvr addctype "text/html"
 * httpsvr delctype "text/html"
 * httpsvr start
 * httpsvr stop
 * httpsvr shutdown
 *****************************************************************************/
QCLI_Command_Status_t net_HttpServer(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    char *cmd;
    int i;
    qapi_Status_t e;

    if (Parameter_Count == 0 || Parameter_List == NULL)
    {
        qapi_Net_HTTPs_Status_t status;
        char *a = NULL, *m = NULL;

        if (qapi_Net_HTTPs_Get_Status(&status) != QAPI_OK)
        {
            QCLI_Printf("HTTP server has not been created. Please do \"httpsvr init\".\n\n");
        }
        else
        {
            if (status.family == AF_INET)
            {
                a = "IPv4";
            }
            else if (status.family == AF_INET6)
            {
                a = "IPv6";
            }
            else if (status.family == AF_INET_DUAL46)
            {
                a = "IPv4 IPv6";
            }

            if (status.mode == QAPI_NET_HTTPSVR_HTTP_E)
            {
                m = "HTTP";
            }
            else if (status.mode == QAPI_NET_HTTPSVR_HTTPS_E)
            {
                m = "HTTPS";
            }
            else if (status.mode == QAPI_NET_HTTPSVR_HTTP_AND_HTTPS_E)
            {
                m = "HTTP HTTPS";
            }

            QCLI_Printf("HTTP server is %s (%s/%s)\n",
                    qapi_Net_HTTPs_Is_Started() ? "running" : "stopped", a, m);

            if (status.mode == QAPI_NET_HTTPSVR_HTTP_E)
            {
                QCLI_Printf("HTTP port: %d\n", status.http_Port);
            }
            else if (status.mode == QAPI_NET_HTTPSVR_HTTPS_E)
            {
                QCLI_Printf("HTTPS port: %d\n", status.https_Port);
            }
            else if (status.mode == QAPI_NET_HTTPSVR_HTTP_AND_HTTPS_E)
            {
                QCLI_Printf("HTTP port: %d  HTTPS port: %d\n",
                            status.http_Port, status.https_Port);
            }

            QCLI_Printf("Landing page: %s  Root path: %s\n",
                    status.root_Index_Page, status.root_Path);

            QCLI_Printf("TX bufsize: %u  RX bufsize: %u\n",
                    status.txbufsize, status.rxbufsize);

            QCLI_Printf("\n");
        }

        httpsvr_help();
        return QCLI_STATUS_SUCCESS_E;
    }

    cmd = Parameter_List[0].String_Value;

    /*         [0]  [1] [2]   [3] [4]         [5] [6] [7] [8]  [9] [10]  [11] [12]       [13] [14]
     * httpsvr init v4  https -c  ca.cert.bin -p  80  -s  443  -i  wlan1 -x   index.html -r   /web/
     * httpsvr init v4  http
     * httpsvr init v4  https -c ca.cert.bin
     * httpsvr init v6  http  -p 8080
     * httpsvr init v46 http
     */
    if (strncmp(cmd, "init", 3) == 0)
    {
        qapi_Net_HTTPs_Config_t cfg;

        if (Parameter_Count < 3)
        {
            httpsvr_help();
            return QCLI_STATUS_SUCCESS_E;
        }

        memset(&cfg, 0, sizeof(cfg));

        /* IPv4 or IPv6 */
        if (strcmp(Parameter_List[1].String_Value, "v4") == 0)
        {
            cfg.family = AF_INET;
        }
        else if (strcmp(Parameter_List[1].String_Value, "v6") == 0)
        {
            cfg.family = AF_INET6;
        }
        else if (strcmp(Parameter_List[1].String_Value, "v46") == 0)
        {
            cfg.family = AF_INET_DUAL46;
        }
        else
        {
            QCLI_Printf("\"%s\" is not supported.\n", Parameter_List[1].String_Value);
            return QCLI_STATUS_ERROR_E;
        }

        /* HTTP or HTTPS */
        if (strcmp(Parameter_List[2].String_Value, "http") == 0)
        {
            cfg.mode = QAPI_NET_HTTPSVR_HTTP_E;
        }
        else if (strcmp(Parameter_List[2].String_Value, "https") == 0)
        {
            cfg.mode = QAPI_NET_HTTPSVR_HTTPS_E;
        }
        else if (strncmp(Parameter_List[2].String_Value, "http_https", 2) == 0)
        {
            cfg.mode = QAPI_NET_HTTPSVR_HTTP_AND_HTTPS_E;
        }
        else
        {
            QCLI_Printf("\"%s\" is not supported.\n", Parameter_List[2].String_Value);
            return QCLI_STATUS_ERROR_E;
        }

        if (Parameter_Count >= 5)
        {
            for (i = 3; i < Parameter_Count; ++i)
            {
                if (Parameter_List[i].String_Value[0] == '-')
                {
                    switch (Parameter_List[i].String_Value[1])
                    {
                    /* Cert_file */
                    case 'c':
                        i++;
                        strncpy(cfg.cert_File, Parameter_List[i].String_Value, sizeof(cfg.cert_File) - 1);
                        break;

                    /* http_port */
                    case 'p':
                        i++;
                        cfg.http_Port = Parameter_List[i].Integer_Value;
                        break;

                    /* https_port */
                    case 's':
                        i++;
                        cfg.https_Port = Parameter_List[i].Integer_Value;
                        break;

                    /* ifname */
                    case 'i':
                        i++;
                        strncpy(cfg.interface_Name, Parameter_List[i].String_Value, sizeof(cfg.interface_Name) - 1);
                        break;

                    /* root_index_page */
                    case 'x':
                        i++;
                        strncpy(cfg.root_Index_Page, Parameter_List[i].String_Value, sizeof(cfg.root_Index_Page) - 1);
                        break;

                    /* root_path */
                    case 'r':
                        i++;
                        strncpy(cfg.root_Path, Parameter_List[i].String_Value, sizeof(cfg.root_Path) - 1);
                        break;

                    default:
                        QCLI_Printf("Unknown option: %s\n", Parameter_List[i].String_Value);
                        return QCLI_STATUS_ERROR_E;
                    }
                }
                else
                {
                    QCLI_Printf("Unknown option: %s\n", Parameter_List[i].String_Value);
                    return QCLI_STATUS_ERROR_E;
                }

                if (i == Parameter_Count)
                {
                    QCLI_Printf("What is value of %s?\n", Parameter_List[i-1].String_Value);
                    return QCLI_STATUS_ERROR_E;
                }
            } /* for */
        }

        /* do it !! */
        if (qapi_Net_HTTPs_Init(&cfg) != QAPI_OK)
        {
            QCLI_Printf("Failed to Init HTTP server\n");
            return QCLI_STATUS_ERROR_E;
        }
    } /* cmd == "init" */

    /*         [0]       [1]      [2]    [3]    [4]       [5]    [6]
     * httpsvr sslconfig protocol TLS1.2 domain yahoo.com alert  1
     */
    else if (strncmp(cmd, "sslconfig", 3) == 0 || strcmp(cmd, "sslcfg") == 0)
    {
        if (Parameter_Count < 2)
        {
            QCLI_Printf("What are SSL parameters?\n");
            httpsvr_help();
            return QCLI_STATUS_ERROR_E;
        }

        if (httpsvr_sslcfg == NULL)
        {
            httpsvr_sslcfg = malloc(sizeof(qapi_Net_SSL_Config_t));
            if (httpsvr_sslcfg == NULL)
            {
                QCLI_Printf("Allocation failure\n");
                return QCLI_STATUS_ERROR_E;
            }
            memset(httpsvr_sslcfg, 0, sizeof(qapi_Net_SSL_Config_t));
        }

        /* Parse SSL config parameters from command line */
        if (ssl_parse_config_parameters(
                Parameter_Count-1,
                &Parameter_List[1],
                httpsvr_sslcfg,
                0,
                true) == QCLI_STATUS_ERROR_E)
        {
            ssl_free_config_parameters(httpsvr_sslcfg);
            free(httpsvr_sslcfg);
            httpsvr_sslcfg = NULL;
            return QCLI_STATUS_ERROR_E;
        }
    }

    /* httpsvr  start */
    else if (strncmp(cmd, "start", 3) == 0)
    {
        if (httpsvr_sslcfg)
        {
            e = qapi_Net_HTTPs_Set_SSL_Config(httpsvr_sslcfg);
            if (e != QAPI_OK)
            {
                QCLI_Printf("SSL config failed\n");
            }
        }

        if (qapi_Net_HTTPs_Start() != QAPI_OK)
        {
            QCLI_Printf("Have you done 'httpsvr init' yet?\n");
            return QCLI_STATUS_ERROR_E;
        }
    }

    /* httpsvr  stop */
    else if (strncmp(cmd, "stop", 3) == 0)
    {
        qapi_Net_HTTPs_Stop();
    }

    /* httpsvr  shutdown */
    else if (strncmp(cmd, "shutdown", 3) == 0)
    {
        if (httpsvr_sslcfg)
        {
            ssl_free_config_parameters(httpsvr_sslcfg);
            free(httpsvr_sslcfg);
            httpsvr_sslcfg = NULL;
        }
        qapi_Net_HTTPs_Shutdown();
    }

    /*         [0]      [1]         [2]
     * httpsvr addctype "text/html" "application/json"
     */
    else if (strncmp(cmd, "addctype", 3) == 0)
    {
        if (Parameter_Count < 2)
        {
            httpsvr_help();
            return QCLI_STATUS_SUCCESS_E;
        }

        for (i = 1; i < Parameter_Count; ++i)
        {
            e = qapi_Net_HTTPs_Register_Content_Type(Parameter_List[i].String_Value, strlen(Parameter_List[i].String_Value));
            if (e != QAPI_OK)
            {
                QCLI_Printf("Failed to register \"%s\"\n", Parameter_List[i].String_Value);
            }
        }
    }

    /* httpsvr  delctype */
    else if (strncmp(cmd, "delctype", 3) == 0)
    {
        if (Parameter_Count < 2)
        {
            httpsvr_help();
            return QCLI_STATUS_SUCCESS_E;
        }

        for (i = 1; i < Parameter_Count; ++i)
        {
            qapi_Net_HTTPs_Unregister_Content_Type(Parameter_List[i].String_Value, strlen(Parameter_List[i].String_Value));
        }
    }

    /* httpsvr setbufsize <txbufsize> <rxbufsize> */
    else if (strncmp(cmd, "setbufsize", 3) == 0)
    {
        uint16_t txbufsize, rxbufsize;

        if (Parameter_Count < 3)
        {
            httpsvr_help();
            return QCLI_STATUS_ERROR_E;
        }

        if (Parameter_List[1].Integer_Is_Valid &&
            Parameter_List[2].Integer_Is_Valid)
        {
            txbufsize = Parameter_List[1].Integer_Value;
            rxbufsize = Parameter_List[2].Integer_Value;
            e = qapi_Net_HTTPs_Set_Buffer_Size(txbufsize, rxbufsize);
            if (e != QAPI_OK)
            {
                QCLI_Printf("Failed to set buffer size\n");
                return QCLI_STATUS_ERROR_E;
            }
        }
        else
        {
            QCLI_Printf("Invalid size\n");
            return QCLI_STATUS_ERROR_E;
        }
    }

    else
    {
        QCLI_Printf("\"%s\" is not supported.\n", cmd);
        return QCLI_STATUS_ERROR_E;
    }

    return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t net_MsockCreate(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	if( Parameter_Count != 4 || !Parameter_List || !Parameter_List[2].Integer_Is_Valid || !Parameter_List[3].Integer_Is_Valid ){
        return QCLI_STATUS_USAGE_E;
    }
	if (0 == multi_sock_create(Parameter_List))
	{
	    return QCLI_STATUS_SUCCESS_E;
	}

    return QCLI_STATUS_ERROR_E;
}

QCLI_Command_Status_t net_MsockClose(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	if( Parameter_Count > 1 || !Parameter_List || !Parameter_List[0].Integer_Is_Valid ){
        return QCLI_STATUS_USAGE_E;
    }
	if (0 == multi_sock_close(Parameter_Count, Parameter_List))
	{
	    return QCLI_STATUS_SUCCESS_E;
	}

    return QCLI_STATUS_ERROR_E;
}

const QCLI_Command_t atn_cmd_list[] =
{
    /*	cmd function           flag     cmd_string 	             usage_string	                               Description      */
	{net_Help,                 false,   "HELP",                  "",                                         "Display the available network commands."
    },
    {net_Ping,                 true,    "PING",                  "<host>,[-c <count>],[-s <size>]",          "Send ICMP ECHO_REQUEST to network hosts in IPv4 network."
    },
    {net_Ifconfig,             false,   "IFCONFIG",              "<interface>,[(<ipv4addr>,<subnetmask>,<default_gateway>)]",		"Configure a network interface."
    },
    {net_DhcpV4Client,         false,   "DHCPV4C",               "<interface>,[new|release]",        		 "DHCPv4 Client: Acquire an IPv4 address using Dynamic Host Configuration Protocol v4."
    },
    {net_DhcpV4server,         false,   "DHCPV4S",               "<interface name> <pool|stop> (<start_ip> <end_ip>) [<lease_time_sec>|infinite]",        		 "DHCPv4 Server: Set up and configure Dynamic Host Configuration Protocol v4 server"
    },
    {net_AutoIp,               false,   "AUTOIPV4",              "<interface_name>",                         "Auto IPv4: Generate an IPv4 Link-Local address."
    },
    {net_PingV6,               false,    "PING6",               	 "<host>,[-c <count>],[-s <size>],[-I <interface>]", 		"Reset Wlan stack."
    },
    {net_DhcpV6Client,         false,   "DHCPV6C",               "<interface>,[enable|new|confirm|disable|release]",     	"DHCPv6 Client: Acquire an IPv6 address using Dynamic Host Configuration Protocol v6."
    },
    {net_SntpClient,           false,   "SNTPC",              	 "[start|stop|disable]\n[addsvr <server> <id>]\n[delsvr <id>]\n[utc]\n",		"SNTP Client: Acquire time from the network using Simple Network Time Protocol v4."
    },
    {net_DnsClient,   		   false,   "DNSC",           		 "[start|stop|disable]\naddsvr <server> [<id>]\ndelsvr <id>\ngethostbyname <host>\n[resolve|gethostbyname2] <host> [v4|v6]\n",																     "DNS Client: Resolves and caches Domain Name System domain names."
    },
    {net_Bridge,               false,   "BRIDGE",			 	 "[enable|disable]\n[aging <val>]\n[showmacs]",     "WLAN Bridge: Configure IEEE 802.1D Layer 2 bridging over WLAN interfaces."
    },
    {net_SockClose,            false,   "CLOSE",                                "",
    },
    {net_TcpConnect,           false,   "CTCP",			 		 "<port>,<ip_addr>,<per_pkt_byte>,<timeout>", 					 "TCP client connects to the server."
    },
    {net_UdpConnect,           false,   "CUDP",		 			 "<port>,<ip_addr>,<per_pkt_byte>,<timeout>", 					 "UDP client connects to the server."
    },
    {net_TcpServer,            true,   "TCPSERVER",		     "<port>,<timeout>,<echo>" 		 		 "Start the TCP server."
    },
    {net_UdpServer,            true,   "UDPSERVER",		     "<port>,<timeout>,<echo>", 		 		 "Start the UDP server."
    },
    {net_HttpClient,           false,   "HTTPC",		 	     "<start>\nATHTTPC=<stop>\nATHTTPC=<connect>,[<server>,<port>,<ssl-index>]\nATHTTPC=<disc>,[<client_num>]\nATHTTPC=<get>,[<client_num>,<url>]\nATHTTPC=<put>,[<client_num>,<url>]\nATHTTPC=<post>,[<client_num>,<url>]\nATHTTPC=<patch>,[<client_num>,<url>]\nATHTTPC=<setbody>,[<client_num>,<len>]\nATHTTPC=<addheader>,[<client_num>,<hdr_name>,<hdr_value>]\nATHTTPC=<clearheader>,[<client_num>]\nATHTTPC=<setparam>,[<client_num>]\nATHTTPC=<setparam>,[<client_num>]\nATHTTPC=<config>,<httpc_demo_max_body_len>,<httpc_demo_max_header_len>]", 		 	 					  "Configures the HTTP client at the run time."
    },
    {net_HttpServer,           false,   "HTTPS",		 	 	 "[init <v4|v6|v46>,<http|https|http_https>,<cert_file>],[<httpport>] [<httpsport>],[<ifname>],[<index_page>],[<root_path>]]\nATNHTTPS=<start>\nATNHTTPS=<stop>\nATNHTTPS=<setbufsize>,<TX_buffer_size>,<RX_buffer_size>\nATNHTTPS=<addctype>,[<content-type1>,[<content-type2> ..]\nATNHTTPS=<delctype>,[<content-type1>,[<content-type2> ..]\nATNHTTPS=<sslconfig>\n[<keyword_1> <value_1> <keyword_1> <value_1> …]", 		 	 					 							 "Configures the HTTP server at the run time."
    },
	{net_MsockCreate,          true,   "MSOCKCREATE",		     "<protocol>,<server_ip>,<portno>,<num_of_sockets>",		"Create multi socket."
    },
	{net_MsockClose,           true,   "MSOCKCLOSE",		     "[session_id]", 		 		 			 "Close multi socket."
    }
};

const QCLI_Command_Group_t atn_cmd_group =
{
    "ATN",
    (sizeof(atn_cmd_list)/sizeof(atn_cmd_list[0])),
    atn_cmd_list
};

void Initialize_ATN_Demo(void)
{
    /* Attempt to reqister the Command Groups with the qcli framework.*/
    atn_group = QCLI_Register_Command_Group(NULL, &atn_cmd_group);
    if(atn_group) {
        QCLI_Printf("ATN Registered \n");
    }
}
