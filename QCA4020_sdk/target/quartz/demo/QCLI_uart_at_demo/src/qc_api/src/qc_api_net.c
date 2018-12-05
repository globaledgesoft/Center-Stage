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

/*-------------------------------------------------------------------------
 * Include Files
 *-----------------------------------------------------------------------*/
#include "qc_at_net.h"
#include "qosa_util.h"

/*-------------------------------------------------------------------------
 * Static and Global varialble Declarations
 *-----------------------------------------------------------------------*/
extern QCLI_Context_t QCLI_Context;
extern const struct in6_addr in6addr_any;
int32_t sockid = -1;
static char msg_invalid_id[] = "Invalid server id\n";

/* extern QCLI_Command_Status_t multi_sockv4_create(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
extern QCLI_Command_Status_t multi_sock_create(QCLI_Parameter_t *Parameter_List);
extern QCLI_Command_Status_t multi_sock_close(void);
extern QCLI_Command_Status_t ssl_command_handler(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List); */

static int32_t ipconfig_dhcps_success_cb(uint8_t *macaddr, uint32_t ipaddr)
{
    if (macaddr != NULL)
    {
        char ip_str[20];

        LOG_AT_EVT("EVT_NET: DHCPv4s, Client IP=%s  Client MAC=%02x:%02x:%02x:%02x:%02x:%02x\n", inet_ntop(AF_INET, &ipaddr, ip_str, sizeof(ip_str)), macaddr[0], macaddr[1], macaddr[2], macaddr[3], macaddr[4], macaddr[5]);
    }

    return QCLI_STATUS_SUCCESS_E;
}

static int32_t ipconfig_dhcpc_success_cb(uint32_t addr, uint32_t mask, uint32_t gw)
{
    char ip_str[20];
    char mask_str[20];
    char gw_str[20];

    LOG_AT_EVT("EVT_NET: DHCPv4c - IP=%s  Subnet Mask=%s  Gateway=%s\n", inet_ntop(AF_INET, &addr, ip_str, sizeof(ip_str)), inet_ntop(AF_INET, &mask, mask_str, sizeof(mask_str)), inet_ntop(AF_INET, &gw, gw_str, sizeof(gw_str)));

    return QCLI_STATUS_SUCCESS_E;
}

static void DHCPv6c_New_Lease_Complete_CB(void * cb_ctxt, const char * interface_name, const ip6_addr * ip6addr)
{
    char ip_str[48];

    LOG_AT_EVT("EVT_NET: DHCPv6c_New_Lease_Complete_CB - interface=%s, ipv6=%s\n", interface_name, inet_ntop(AF_INET6, ip6addr, ip_str, sizeof(ip_str)));
}

static void DHCPv6c_Release_Lease_Complete_CB(void * cb_ctxt, const char * interface_name, const ip6_addr * ip6addr)
{
    char ip_str[48];

    LOG_AT_EVT("EVT_NET: DHCPv6c_Release_Lease_Complete_CB - interface=%s, ipv6=%s\n", interface_name, inet_ntop(AF_INET6, ip6addr, ip_str, sizeof(ip_str)));
}

static void DHCPv6c_New_Prefix_Lease_Complete_CB(void * cb_ctxt, const char * interface_name, const ip6_addr * prefix_base, const uint8_t prefix_length)
{
    char ip_str[48];

    LOG_AT_EVT("EVT_NET: DHCPv6c_New_Prefix_Lease_Complete_CB - interface=%s, prefix=%s/%d\n", interface_name, inet_ntop(AF_INET6, prefix_base, ip_str, sizeof(ip_str)), prefix_length);
}

static void DHCPv6c_Release_Prefix_Lease_Complete_CB(void * cb_ctxt, const char * interface_name, const ip6_addr * prefix_base, const uint8_t prefix_length)
{
    char ip_str[48];

    LOG_AT_EVT("EVT_NET: DHCPv6c_Release_Prefix_Lease_Complete_CB - interface=%s, prefix=%s/%d\n", interface_name, inet_ntop(AF_INET6, prefix_base, ip_str, sizeof(ip_str)), prefix_length);
}

static int32_t display_interface_info(char *interface_name, qbool_t if_is_up)
{
    ip6_addr v6Global, v6GlobalExtd, v6LinkLocal, v6DefGw;
    uint32_t LinkPrefix, GlobalPrefix, DefGwPrefix, GlobalExtdPrefix;
    qapi_Net_DNS_Server_List_t svr_list;
    char msg_ipv4_target_error[] = "Operation failed";

    LOG_AT("%s: %s\n", interface_name, if_is_up ? "UP" : "DOWN");

    if (strncmp(interface_name, "wlan", 4) == 0 || strcmp(interface_name, "wlbr0") == 0)
    {
        /* MAC */
        uint32_t addr, mask, gw;
        int i, err = -1;
        const uint8_t * physical_address;
        uint32_t physical_address_length;

        if ((err = qc_drv_net_Interface_Get_Physical_Address(qc_api_get_qc_drv_context(), interface_name, &physical_address, &physical_address_length)) == 0)
        {
            LOG_AT(" Phy Address: %02X", physical_address[0]);
            for ( i = 1; i < physical_address_length; i++ )
            {
                LOG_AT(":%02X", physical_address[i]);
            }

            LOG_AT("\n");
        }

        /* IPv4 */
        if ((err = qc_drv_net_IPv4_Config(qc_api_get_qc_drv_context(), interface_name, QAPI_NET_IPV4CFG_QUERY_E, &addr, &mask, &gw)) == 0)
        {
            char ip_str[20], mask_str[20], gw_str[20];

            LOG_AT(" IPv4: %s  Subnet Mask: %s  Default Gateway: %s\n", inet_ntop(AF_INET, &addr, ip_str, sizeof(ip_str)), inet_ntop(AF_INET, &mask, mask_str, sizeof(mask_str)), inet_ntop(AF_INET, &gw, gw_str, sizeof(gw_str)));
        }

        if (err == QAPI_NET_ERR_SOCKET_CMD_TIME_OUT)
        {
            LOG_ERR("%s\n", msg_ipv4_target_error);
        }
    }

    /* IPv6 */
    memset(&v6LinkLocal,0, sizeof(ip6_addr));
    memset(&v6Global,0, sizeof(ip6_addr));
    memset(&v6DefGw,0, sizeof(ip6_addr));
    memset(&v6GlobalExtd,0, sizeof(ip6_addr));
    LinkPrefix = GlobalPrefix = DefGwPrefix = GlobalExtdPrefix = 0;
    if (qc_drv_net_IPv6_Get_Address(qc_api_get_qc_drv_context(), interface_name, (uint8_t *)&v6LinkLocal, (uint8_t *)&v6Global, (uint8_t *)&v6DefGw, (uint8_t *)&v6GlobalExtd, &LinkPrefix, &GlobalPrefix, &DefGwPrefix, &GlobalExtdPrefix) == 0)
    {
        char ip_str[48];

        if (v6LinkLocal.s_addr[0])
        {
            if (inet_ntop(AF_INET6, &v6LinkLocal, ip_str, sizeof(ip_str)) != NULL)
            {
                if (LinkPrefix)
                    LOG_AT(" IPv6 Link-local Address ..... : %s/%d\n", ip_str, LinkPrefix);
                else
                    LOG_AT(" IPv6 Link-local Address ..... : %s\n", ip_str);
            }

            if (inet_ntop(AF_INET6, &v6Global, ip_str, sizeof(ip_str)) != NULL)
            {
                if (GlobalPrefix)
                    LOG_AT(" IPv6 Global Address ......... : %s/%d\n", ip_str, GlobalPrefix);
                else
                    LOG_AT(" IPv6 Global Address ......... : %s\n", ip_str);
            }

            if (inet_ntop(AF_INET6, &v6DefGw, ip_str, sizeof(ip_str)) != NULL)
            {
                if (DefGwPrefix)
                    LOG_AT(" IPv6 Default Gateway  ....... : %s/%d\n", ip_str, DefGwPrefix);
                else
                    LOG_AT(" IPv6 Default Gateway  ....... : %s\n", ip_str);

            }

            if (inet_ntop(AF_INET6, &v6GlobalExtd, ip_str, sizeof(ip_str)) != NULL)
            {
                if (GlobalExtdPrefix)
                    LOG_AT(" IPv6 Global Address 2 ....... : %s/%d\n", ip_str, GlobalExtdPrefix);
                else
                    LOG_AT(" IPv6 Global Address 2 ....... : %s\n", ip_str);
            }
        }
    }

    /* DNS */
    memset(&svr_list, 0, sizeof(qapi_Net_DNS_Server_List_t));
    if (qc_drv_net_DNSc_get_srvr_list(qc_api_get_qc_drv_context(), &svr_list) == 0)
    {
        int i;
        char ip_str[48];

        for (i = 0; i < QAPI_NET_DNS_SERVER_MAX; ++i)
        {
            if (svr_list.svr[i].type != AF_UNSPEC)
            {
                LOG_AT(" DNS Server: %s\n",
                        inet_ntop(svr_list.svr[i].type, &svr_list.svr[i].a, ip_str, sizeof(ip_str)));
            }
        }
    }

    return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t qc_api_net_Help(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    /* Take the mutex before modifying any global variables. */
    if (TAKE_LOCK(QCLI_Context.CLI_Mutex))
    {
        Display_Help(QCLI_Context.Executing_Group, Parameter_Count, Parameter_List);
        RELEASE_LOCK(QCLI_Context.CLI_Mutex);
        return QCLI_STATUS_SUCCESS_E;
    }

    return QCLI_STATUS_ERROR_E;
}

QCLI_Command_Status_t qc_api_net_Ping(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    char ip_str[16];
    uint32_t i, count, size;
    struct ip46addr addr;
    int e;
    char *host;

    host = Parameter_List[0].String_Value;
    count = (Parameter_Count > 1) ? Parameter_List[1].Integer_Value : 1;
    size  = (Parameter_Count > 2) ? Parameter_List[2].Integer_Value : 64;

    if (inet_pton(AF_INET, host, &addr.a) != 0)   /* not in xxx.xxx.xxx.xxx format */
    {
        if (strlen(host) > MAX_FQDN_LEN)
        {
            LOG_ERR("host name cannot be more then %u Bytes\n", MAX_FQDN_LEN);
            return QCLI_STATUS_ERROR_E;
        }

        addr.type = AF_INET;
        if (qc_drv_net_DNSc_Reshost(qc_api_get_qc_drv_context(), host, &addr) != 0)
        {
            LOG_ERR("Cannot resolve %s\n", host);
            return QCLI_STATUS_ERROR_E;
        }
        else
        {
            LOG_AT("Ping %s (%s):\n",
                    host,
                    inet_ntop(AF_INET, &addr.a, ip_str, sizeof(ip_str)));
        }
    }

    for (i = 0; i < count; i++)
    {
        uint32_t t1, t2, ms;

        t1 = app_get_time(NULL);
        e = qc_drv_net_Ping(qc_api_get_qc_drv_context(), addr.a.addr4, size);
        if (e == 0)
        {
            t2 = app_get_time(NULL);
            ms = t2 - t1;
            LOG_AT("%d bytes from %s: seq=%d time=%u ms\n",
                    size, inet_ntop(AF_INET, &addr.a, ip_str, sizeof(ip_str)), i+1, ms);
            if (i != count - 1)
            {
                app_msec_delay(980);
            }
        }
        else if (e == QAPI_NET_ERR_INVALID_IPADDR)
        {
            LOG_ERR("Invalid IP address\n");
            return QCLI_STATUS_ERROR_E;
        }
        else
        {
            LOG_AT("Request timed out\n");
        }
    }

    return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t qc_api_net_Ifconfig(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
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
            num = qc_drv_net_Get_All_Ifnames(qc_api_get_qc_drv_context(), NULL);
            if (num == 0)
            {
                /* If no interface, is interface down? */
                LOG_ERR("Not able to get interface details.\r\n");
                LOG_ERR("ERROR\r\n");
                return QCLI_STATUS_ERROR_E;
            }

            p = pp = malloc(num * sizeof(qapi_Net_Ifnameindex_t));
            if (p)
            {
                qc_drv_net_Get_All_Ifnames(qc_api_get_qc_drv_context(), p);
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
            if (qc_drv_net_Interface_Exist(qc_api_get_qc_drv_context(), interface_name, &if_is_up))
            {
                display_interface_info(interface_name, if_is_up);
            }
            else
            {
                LOG_ERR("%s does not exist\n", interface_name);
                return QCLI_STATUS_ERROR_E;
            }
            break;

            /* Set IPv4 address for WLAN interfaces */
        case 3:
        case 4:
            interface_name = Parameter_List[0].String_Value;
            if (strncmp(interface_name, "wlan", 4) != 0 && strcmp(interface_name, "wlbr0") != 0)
            {
                LOG_ERR("Not a WLAN interface\n");
                return QCLI_STATUS_ERROR_E;
            }

            /* interface's IP address */
            if (inet_pton(AF_INET, Parameter_List[1].String_Value, &addr) != 0)
            {
                LOG_ERR("Invalid IP address\n");
                return QCLI_STATUS_ERROR_E;
            }

            /* subnet mask */
            if (inet_pton(AF_INET, Parameter_List[2].String_Value, &mask) != 0)
            {
                LOG_ERR("Invalid subnet mask\n");
                return QCLI_STATUS_ERROR_E;
            }

            /* default gateway */
            if (Parameter_Count == 4)
            {
                if (inet_pton(AF_INET, Parameter_List[3].String_Value, &gw) != 0)
                {
                    LOG_ERR("Invalid gateway address\n");
                    return QCLI_STATUS_ERROR_E;
                }

                gw_addr = &gw;
            }

            e = qc_drv_net_IPv4_Config(qc_api_get_qc_drv_context(), interface_name, QAPI_NET_IPV4CFG_STATIC_IP_E, &addr, &mask, gw_addr);
            if (e != 0)
            {
                if (e == QAPI_NET_ERR_INVALID_IPADDR)
                {
                    LOG_ERR("Invalid IP address\n");
                }
                else
                {
                    LOG_ERR("Static IPv4 configure failed\n");
                }
                return QCLI_STATUS_ERROR_E;
            }
            break;

        default:
            return QCLI_STATUS_USAGE_E;
    }

    return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t qc_api_net_DhcpV4Client(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    char *interface_name;
    char *cmd;
    qbool_t if_is_up = FALSE;

    if (Parameter_Count < 1 || Parameter_List == NULL)
    {
        return QCLI_STATUS_USAGE_E;
    }

    interface_name = Parameter_List[0].String_Value;
    if (qc_drv_net_Interface_Exist(qc_api_get_qc_drv_context(),interface_name, &if_is_up) == FALSE || if_is_up == FALSE)
    {
        LOG_ERR("ERROR: %s does not exist or is DOWN.\n", interface_name);
        return QCLI_STATUS_ERROR_E;
    }

    cmd = Parameter_List[1].String_Value;
    if (Parameter_Count == 1 || strncmp(cmd, "new", 3) == 0)
    {
        if (qc_drv_net_DHCPv4c_Register_Success_Callback(qc_api_get_qc_drv_context(), interface_name, ipconfig_dhcpc_success_cb) != 0 || qc_drv_net_IPv4_Config(qc_api_get_qc_drv_context(), interface_name, QAPI_NET_IPV4CFG_DHCP_IP_E, NULL, NULL, NULL) != 0)
        {
            LOG_ERR("ERROR: DHCPv4 new failed\n");
            return QCLI_STATUS_ERROR_E;
        }
    }
    else if (strncmp(cmd, "release", 3) == 0)
    {
        if (qc_drv_net_DHCPv4c_Release(qc_api_get_qc_drv_context(),
                    interface_name) != 0)
        {
            LOG_ERR("ERROR: DHCPv4 release failed\n");
            return QCLI_STATUS_ERROR_E;
        }
    }

    return QCLI_STATUS_SUCCESS_E;
}

/*****************************************************************************
 *           [0]   [1] [2]           [3]           [4]
 * Dhcpv4s wlan0  pool 192.168.1.10  192.168.1.50  3600
 *****************************************************************************/
QCLI_Command_Status_t qc_api_net_DhcpV4server(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
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
                !Parameter_List[4].Integer_Is_Valid && (strcmp(Parameter_List[4].String_Value, DHCP_INFINITE_LEASE_TIME) != 0)) {
            LOG_ERR("Invalid lease time specified, setting to default value\n");
        }

        if (Parameter_Count == 5 && Parameter_List[4].Integer_Is_Valid)
        {
            leasetime = Parameter_List[4].Integer_Value;
        }

        e = inet_pton(AF_INET, start_ip_addr_string, &startaddr);
        if (e != 0)
        {
            LOG_ERR("Invalid start address\n");
            return QCLI_STATUS_ERROR_E;
        }

        startaddr = ntohl(startaddr);
        e = inet_pton(AF_INET, end_ip_addr_string, &endaddr);
        if (e != 0)
        {
            LOG_ERR("Invalid end address\n");
            return QCLI_STATUS_ERROR_E;
        }

        endaddr = ntohl(endaddr);
        if (startaddr >= endaddr)
        {
            LOG_ERR("Start address must be less than end address\n");
            return QCLI_STATUS_ERROR_E;
        }

        /* Get interface's IP address */
        e = qc_drv_net_IPv4_Config(qc_api_get_qc_drv_context(), interface_name, QAPI_NET_IPV4CFG_QUERY_E, &addr, &mask, NULL);
        if (e != 0)
        {
            LOG_ERR("Getting interface address failed\n");
            return QCLI_STATUS_ERROR_E;
        }

        addr = ntohl(addr);
        mask = ntohl(mask);
        if ((addr & mask) != (startaddr & mask) ||
                (addr & mask) != (endaddr & mask))
        {
            LOG_ERR("Pool IP and interface IP should be in the same subnet\n");
            return QCLI_STATUS_ERROR_E;
        }

        if ((addr >= startaddr) && (addr <= endaddr))
        {
            LOG_ERR("Please configure pool beyond interface IP address\n");
            return QCLI_STATUS_ERROR_E;
        }

        if (qc_drv_net_DHCPv4s_Register_Success_Callback(qc_api_get_qc_drv_context(), interface_name, ipconfig_dhcps_success_cb) != 0 || qc_drv_net_DHCPv4s_Set_Pool(qc_api_get_qc_drv_context(), interface_name, htonl(startaddr), htonl(endaddr), leasetime) != 0)
        {
            LOG_ERR("Config DHCP pool failed\n");
            return QCLI_STATUS_ERROR_E;
        }
    }
    else if (strncmp(cmd, "stop", 4) == 0)
    {
        LOG_ERR("DHCP Server stop not supported: %s\n", cmd);
        return QCLI_STATUS_ERROR_E;
    }
    else
    {
        LOG_ERR("Invalid command: %s\n", cmd);
        return QCLI_STATUS_ERROR_E;
    }

    return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t qc_api_net_AutoIp(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    char *interface_name;

    if (Parameter_Count < 1 || Parameter_List == NULL)
    {
        return QCLI_STATUS_USAGE_E;
    }

    interface_name = Parameter_List[0].String_Value;
    if (qc_drv_net_IPv4_Config(qc_api_get_qc_drv_context(), interface_name, QAPI_NET_IPV4CFG_AUTO_IP_E, NULL, NULL, NULL) != 0)
    {
        LOG_ERR("Auto IPv4 failed\n");
        return QCLI_STATUS_ERROR_E;
    }

    return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t qc_api_net_PingV6(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
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
                    LOG_ERR("Size should be <= %d\n", CFG_PACKET_SIZE_MAX_TX);
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
            LOG_ERR("host name cannot be more then %u Bytes\n", MAX_FQDN_LEN);
            return QCLI_STATUS_ERROR_E;
        }

        addr.type = AF_INET6;
        if (qc_drv_net_DNSc_Reshost(qc_api_get_qc_drv_context(), host, &addr) != 0)
        {
            LOG_ERR("Invalid IP address\n");
            return QCLI_STATUS_ERROR_E;
        }
        else
        {
            LOG_AT("Ping %s (%s):\n",
                    host,
                    inet_ntop(AF_INET6, &addr.a, ip_str, sizeof(ip_str)));
        }
    }

    ip6addr = (uint8_t *)&addr.a;
    if (QAPI_IS_IPV6_MULTICAST(ip6addr) || QAPI_IS_IPV6_LINK_LOCAL(ip6addr))
    {
        is_ifname_specification_needed = 1;
    }

    if (is_ifname_specification_needed && ifname == NULL)
    {
        LOG_ERR("Error: The specified IPv6 address is either multicast or link-local, please specify source interface name using the -I option\n");
        return QCLI_STATUS_ERROR_E;
    }

    for (i = 0; i < count; i++)
    {
        uint32_t t1, t2, ms;

        t1 = app_get_time(NULL);
        e = qc_drv_net_Ping6(qc_api_get_qc_drv_context(), ip6addr, size, ifname);
        if (e == 0)
        {
            t2 = app_get_time(NULL);
            ms = t2 - t1;
            LOG_AT("%d bytes from %s: seq=%d time=%u ms\n",
                    size, inet_ntop(AF_INET6, &addr.a, ip_str, sizeof(ip_str)), i+1, ms);
        }
        else if (e == 1)
        {
            LOG_AT("Request timed out\n");
        }
        else
        {
            LOG_ERR("PING failed\n");
            return QCLI_STATUS_ERROR_E;
        }

#define PING6_DELAY_DURATION_IN_MS 1000

        if ( i < count - 1 ) {
            app_msec_delay(PING6_DELAY_DURATION_IN_MS);
        }
    }

    return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t qc_api_net_DhcpV6Client(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
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
        if (qc_drv_net_DHCPv6c_Register_New_IPv6_Lease_Complete_Callback(qc_api_get_qc_drv_context(), ifname, DHCPv6c_New_Lease_Complete_CB, NULL) != 0 || qc_drv_net_DHCPv6c_Register_Release_IPv6_Lease_Complete_Callback(qc_api_get_qc_drv_context(), ifname, DHCPv6c_Release_Lease_Complete_CB, NULL) != 0 || qc_drv_net_DHCPv6c_Enable(qc_api_get_qc_drv_context(), ifname) != 0)
        {
            goto fail;
        }
    }
    else if (strncmp(cmd, "disable", 3) == 0)
    {
        if (qc_drv_net_DHCPv6c_Disable(qc_api_get_qc_drv_context(), ifname) != 0)
        {
            goto fail;
        }
    }
    else if (strncmp(cmd, "new", 3) == 0)
    {
        char * ifname_to_obtain_ia_for = Parameter_List[2].String_Value;
        int are_same_interfaces = (strcmp(ifname, ifname_to_obtain_ia_for) == 0);

        if (!are_same_interfaces)
        {
            int status_code =
                qc_drv_net_DHCPv6c_Register_New_Prefix_Lease_Complete_Callback(qc_api_get_qc_drv_context(), ifname_to_obtain_ia_for, DHCPv6c_New_Prefix_Lease_Complete_CB, NULL);
            if (status_code) {
                goto fail;
            }
            status_code =
                qc_drv_net_DHCPv6c_Register_Release_Prefix_Lease_Complete_Callback(qc_api_get_qc_drv_context(), ifname_to_obtain_ia_for, DHCPv6c_Release_Prefix_Lease_Complete_CB, NULL);
            if (status_code) {
                goto fail;
            }
        }

        if (qc_drv_net_DHCPv6c_New_Lease(qc_api_get_qc_drv_context(), ifname, ifname_to_obtain_ia_for) != 0)
        {
            goto fail;
        }
    }
    else if (strncmp(cmd, "release", 3) == 0)
    {
        char * ifname_to_obtain_ia_for = Parameter_List[2].String_Value;
        if (qc_drv_net_DHCPv6c_Release_Lease(qc_api_get_qc_drv_context(), ifname, ifname_to_obtain_ia_for) != 0)
        {
            goto fail;
        }
    }
    else if (strncmp(cmd, "confirm", 3) == 0)
    {
        char * ifname_to_obtain_ia_for = Parameter_List[2].String_Value;
        if (qc_drv_net_DHCPv6c_Confirm_Lease(qc_api_get_qc_drv_context(), ifname, ifname_to_obtain_ia_for) != 0)
        {
            goto fail;
        }
    }
    else
    {
        LOG_ERR("Invalid command: %s\n", cmd);
        return QCLI_STATUS_ERROR_E;
    }

    return QCLI_STATUS_SUCCESS_E;

fail:
    LOG_ERR("%s failed\n", cmd);
    return QCLI_STATUS_ERROR_E;
}

QCLI_Command_Status_t qc_api_net_SntpClient(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
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

        started = qc_drv_net_SNTPc_start(qc_api_get_qc_drv_context());
        LOG_AT("SNTP client is %s.\n", started ? "started" : "stopped");
        if (qc_drv_net_SNTPc_get_srvr_list(qc_api_get_qc_drv_context(), &svr_list) == 0)
        {
            for (i = 0; i < QAPI_NET_SNTP_SERVER_MAX; ++i)
            {
                LOG_INFO("%d: %s     %s\n", svr_list.svr[i].name[0] != '\0' ? svr_list.svr[i].name : "****", svr_list.svr[i].addr[0] != '\0' ? svr_list.svr[i].addr : "****");
                if (svr_list.svr[i].status == QAPI_NET_SNTP_SERVER_STATUS_KOD)
                {
                    LOG_DEBUG("KOD");
                }
                else if (svr_list.svr[i].status == QAPI_NET_SNTP_SERVER_STATUS_NORESP)
                {
                    LOG_DEBUG("NORESP");
                }
                LOG_AT("\n");
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

        if (!qc_drv_net_SNTPc_start(qc_api_get_qc_drv_context()))
        {
            LOG_ERR("%s\n", msg_sntp_not_started);
            return QCLI_STATUS_ERROR_E;
        }

        svr = Parameter_List[1].String_Value;
        if (strlen(svr) > 64)
        {
            LOG_ERR("%s\n", msg_name_too_long);
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
            LOG_ERR("%s\n", msg_invalid_id);
            return QCLI_STATUS_ERROR_E;
        }

        e = qc_drv_net_SNTPc_add_srvr(qc_api_get_qc_drv_context(), svr, id);
        if (e == QAPI_NET_ERR_INVALID_IPADDR)
        {
            LOG_ERR("Invalid server address\n");
        }
        else if (e == QAPI_NET_ERR_CANNOT_GET_SCOPEID)
        {
            LOG_ERR("Failed to get scope id\n");
        }
    }

    /* Sntpc  del  1 */
    else if (strncmp(cmd, "delsvr", 3) == 0)
    {
        if (Parameter_Count < 2)
        {
            goto parm_error;
        }

        if (!qc_drv_net_SNTPc_start(qc_api_get_qc_drv_context()))
        {
            LOG_ERR("%s\n", msg_sntp_not_started);
            return QCLI_STATUS_ERROR_E;
        }

        if (Parameter_List[1].Integer_Is_Valid)
        {
            id = Parameter_List[1].Integer_Value;
        }
        else
        {
            LOG_ERR("%s\n", msg_invalid_id);
            return QCLI_STATUS_ERROR_E;
        }

        e = qc_drv_net_SNTPc_del_srvr(qc_api_get_qc_drv_context(), id);
    }

    /* Sntpc  start */
    else if (strncmp(cmd, "start", 3) == 0)
    {
        e = qc_drv_net_SNTPc_cmd(qc_api_get_qc_drv_context(), QAPI_NET_SNTP_START_E);
    }

    /* Sntpc  stop */
    else if (strncmp(cmd, "stop", 3) == 0)
    {
        e = qc_drv_net_SNTPc_cmd(qc_api_get_qc_drv_context(), QAPI_NET_SNTP_STOP_E);
    }

    /* Sntpc  disable */
    else if (strncmp(cmd, "disable", 3) == 0)
    {
        e = qc_drv_net_SNTPc_cmd(qc_api_get_qc_drv_context(), QAPI_NET_SNTP_DISABLE_E);
    }

    /* Sntpc  utc */
    else if (strncmp(cmd, "utc", 3) == 0)
    {
        char *mon_str[12] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
        char *day_str[7] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
        qapi_Net_SNTP_Tm_t tm;

        e = qc_drv_net_SNTPc_get_broke_down_time(qc_api_get_qc_drv_context(), &tm);
        if (!e)
        {
            /*  Dec 1, 2015 Mon 18:09:41 UTC */
            LOG_INFO("%s %u, %02u %s %02u:%02u:%02u UTC\n",
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
        LOG_ERR("Invalid subcommand: %s\n", cmd);
        goto parm_error;
    }

    if (e)
    {
        LOG_ERR("%s failed\n", cmd);
        return QCLI_STATUS_ERROR_E;
    }
    else
    {
        return QCLI_STATUS_SUCCESS_E;
    }

parm_error:
    return QCLI_STATUS_USAGE_E;
}

QCLI_Command_Status_t qc_api_net_DnsClient(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
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

        started = qc_drv_net_DNSc_start(qc_api_get_qc_drv_context());
        LOG_AT("DNS client is %s.\n", started ? "started" : "stopped");
        e = qc_drv_net_DNSc_get_srvr_list(qc_api_get_qc_drv_context(), &svr_list);
        if (e == 0)
        {
            for (i = 0; i < QAPI_NET_DNS_SERVER_MAX; ++i)
            {
                if (svr_list.svr[i].type == AF_UNSPEC)
                {
                    LOG_INFO("%d: ****\n", i);
                }
                else
                {
                    LOG_INFO("%d: %s\n", i, inet_ntop(svr_list.svr[i].type, &svr_list.svr[i].a, ip_str, sizeof(ip_str)));
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
            LOG_ERR("address cannot be more then 48 bytes\n");
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
            LOG_ERR("%s\n", msg_invalid_id);
            return QCLI_STATUS_ERROR_E;
        }

        e = qc_drv_net_DNSc_add_srvr(qc_api_get_qc_drv_context(), svr, id);
        if (e == QAPI_NET_ERR_INVALID_IPADDR)
        {
            LOG_ERR("Invalid server address\n");
        }
        else if (e == QAPI_NET_ERR_CANNOT_GET_SCOPEID)
        {
            LOG_ERR("Failed to get scope id\n");
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
            LOG_ERR("%s\n", msg_invalid_id);
            return QCLI_STATUS_ERROR_E;
        }

        e = qc_drv_net_DNSc_del_srvr(qc_api_get_qc_drv_context(), id);
    }

    /* Dnsc start */
    else if (strncmp(cmd, "start", 3) == 0)
    {
        e = qc_drv_net_DNSc_cmd(qc_api_get_qc_drv_context(), QAPI_NET_DNS_START_E);
    }

    /* Dnsc stop */
    else if (strncmp(cmd, "stop", 3) == 0)
    {
        e = qc_drv_net_DNSc_cmd(qc_api_get_qc_drv_context(), QAPI_NET_DNS_STOP_E);
    }

    /* Dnsc disable */
    else if (strncmp(cmd, "disable", 3) == 0)
    {
        e = qc_drv_net_DNSc_cmd(qc_api_get_qc_drv_context(), QAPI_NET_DNS_DISABLE_E);
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

        if  (!qc_drv_net_DNSc_start(qc_api_get_qc_drv_context()))
        {
            LOG_ERR("DNS client is not started yet.\n");
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
            LOG_INFO("Resolve to \"v4\" or \"v6\"?\n");
            goto parm_error;
        }

        e = qc_drv_net_DNSc_Reshost(qc_api_get_qc_drv_context(), svr, &ipaddr);
        if (e)
        {
            LOG_ERR("Unable to resolve %s\n", svr);
        }
        else
        {
            LOG_INFO("%s --> %s\n", svr, inet_ntop(ipaddr.type, &ipaddr.a, ip_str, sizeof(ip_str)));
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

        if  (!qc_drv_net_DNSc_start(qc_api_get_qc_drv_context()))
        {
            LOG_ERR("DNS client is not started yet.\n");
            return QCLI_STATUS_ERROR_E;
        }

        svr = Parameter_List[1].String_Value;
        he = gethostbyname(svr);
        if (he == NULL)
        {
            LOG_ERR("Unable to resolve %s\n", svr);
        }
        else
        {
            LOG_INFO("%s -->", he->h_name);
            while (*(he->h_addr_list) != NULL)
            {
                LOG_AT(" %s\n", inet_ntop(AF_INET, *(he->h_addr_list), ip_str, sizeof(ip_str)));
                ++(he->h_addr_list);
                LOG_AT("                   ");
            }
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

        if  (!qc_drv_net_DNSc_start(qc_api_get_qc_drv_context()))
        {
            LOG_ERR("DNS client is not started yet.\n");
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
            LOG_INFO("Resolve to \"v4\" or \"v6\"?\n");
            goto parm_error;
        }

        svr = Parameter_List[1].String_Value;
        he = gethostbyname2(svr, af);
        if (he == NULL)
        {
            LOG_ERR("Unable to resolve %s to IP%s address\n",
                    svr, (af == AF_INET) ? "v4" : "v6");
        }
        else
        {
            LOG_INFO("%s -->", he->h_name);
            while (*(he->h_addr_list) != NULL)
            {
                LOG_AT(" %s", inet_ntop(af, *(he->h_addr_list), ip_str, sizeof(ip_str)));
                ++(he->h_addr_list);
            }
            LOG_AT("\n");
            e = 0;
        }
    }

    else
    {
        LOG_ERR("Invalid subcommand: %s\n", cmd);
        goto parm_error;
    }

    if (e)
    {
        LOG_ERR("%s failed\n", cmd);
        return QCLI_STATUS_ERROR_E;
    }
    else
    {
        return QCLI_STATUS_SUCCESS_E;
    }

parm_error:
    return QCLI_STATUS_USAGE_E;
}

QCLI_Command_Status_t qc_api_net_Bridge(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
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
        e = qc_drv_net_Bridge_Enable(qc_api_get_qc_drv_context(), (strncmp(cmd, "enable", 3) == 0)? 1: 0);
    }

    /* Set MAC entry age in bridge database*/
    else if (strncmp(cmd, "aging", 3) == 0)
    {
        if (Parameter_Count < 2)
        {
            goto parm_error;
        }

        e = qc_drv_net_Bridge_Set_Aging_Timeout(qc_api_get_qc_drv_context(), Parameter_List[1].Integer_Value);
    }

    /* Dump bridge mac database entried*/
    else if (strncmp(cmd, "showmacs", 3) == 0)
    {
        qapi_Net_Bridge_DB_Entry_t *ptr;
        uint32_t count = 0;
        if ((e = qc_drv_net_Bridge_Show_MACs(qc_api_get_qc_drv_context(), &ptr, &count)) != -1)
        {
            if (count == 0) {
                LOG_ERR("No Entries\n");
            }
            else {
                void *buf = (void *)ptr;
                LOG_INFO("DevId\tBridged MAC address\tTTL\n");
                while(count){
                    LOG_INFO("%02d\t\t%02X:%02X:%02X:%02X:%02X:%02X\t%d\n", ptr->dev_ID, ptr->addr[0], ptr->addr[1],ptr->addr[2],ptr->addr[3],ptr->addr[4],ptr->addr[5], ptr->age);
                    count--;
                    ptr++;
                }
                free(buf);
            }
        }
        else {
            LOG_ERR("Invalid command, use help\n");
        }
    }

    if (e)
    {
        switch (e)
        {
            case QAPI_ERR_NOT_SUPPORTED:
                LOG_ERR("Operation not supported\n", cmd);
                break;

            case QAPI_ERR_NO_MEMORY:
                LOG_ERR("Out of memory\n", cmd);
                break;

            default:
                LOG_ERR("Command failed. Run ifconfig and verify wlan0 and wlan1 are up\n", cmd);

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

QCLI_Command_Status_t qc_api_net_SockClose(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if (Parameter_Count < 1 || !Parameter_List || (!Parameter_List[0].Integer_Is_Valid))
        return QCLI_STATUS_USAGE_E;

    net_sock_close(Parameter_List[0].Integer_Value);

    return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t qc_api_net_TcpConnect(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    uint16_t portnum;
    char *ipaddr = NULL;
    struct sockaddr_in addr = {0};
    int sockid;

    if ((Parameter_Count != 2) || (!Parameter_List) || (!Parameter_List[1].Integer_Is_Valid))
    {
        return QCLI_STATUS_USAGE_E;
    }

    // Port number
    portnum = Parameter_List[1].Integer_Value;
    // IP addr
    ipaddr = Parameter_List[0].String_Value;
    if (inet_pton(AF_INET, ipaddr, &addr.sin_addr.s_addr) != 0)
    {
        LOG_ERR("Invalid IP address\n");
        return -1;
    }

    sockid = net_sock_open(0, AF_INET, SOCK_STREAM, portnum, (uint8_t *)ipaddr);
    if (sockid < 0)
    {
        LOG_ERR("Failed to open socket %d\n", sockid);
        return QCLI_STATUS_ERROR_E;
    }

    LOG_AT("Session id %d\n", sockid);
    return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t qc_api_net_TcpV6Connect(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
#if 1
    uint16_t portnum;
    char *ipv6addr = NULL;
    struct sockaddr_in6 addr = {0};
    int sockid;

    if ((Parameter_Count != 2) || (!Parameter_List) || (!Parameter_List[1].Integer_Is_Valid))
    {
        return QCLI_STATUS_USAGE_E;
    }

    // Port number
    portnum = Parameter_List[1].Integer_Value;
    // IPV6 addr
    ipv6addr = Parameter_List[0].String_Value;

    if (inet_pton(AF_INET6, ipv6addr, &addr.sin_addr.s_addr) != 0)
    {
        LOG_ERR("Invalid IPV6 address\n");
        return QCLI_STATUS_ERROR_E;
    }

    sockid = net_sock_open(0, AF_INET6, SOCK_STREAM, portnum, (uint8_t *)ipv6addr);
    if (sockid < 0)
    {
        LOG_ERR("Failed to open socket %d\n", sockid);
        return QCLI_STATUS_ERROR_E;
    }

    LOG_AT("Session id %d\n", sockid);
    return QCLI_STATUS_SUCCESS_E;
#endif
#if 0
    uint16_t portnum;
    char *ipaddr = NULL;
    char ipstr[46];
    struct sockaddr_in6 ipv6;
    int bytes;
    char *send_buf = NULL;
    uint32_t peer_pkt_byte = 512;
    uint32_t timeout = 0;
    uint32_t ticks = 0;

    if ((Parameter_Count != 4) || (!Parameter_List[0].Integer_Is_Valid) || (!Parameter_List) || (Parameter_List[1].Integer_Is_Valid) || (!Parameter_List[2].Integer_Is_Valid) || (!Parameter_List[3].Integer_Is_Valid))
    {
        return QCLI_STATUS_USAGE_E;
    }

    portnum = Parameter_List[0].Integer_Value;
    ipaddr = Parameter_List[1].String_Value;
    peer_pkt_byte = Parameter_List[2].Integer_Value;
    timeout = Parameter_List[3].Integer_Value;

    send_buf = malloc(peer_pkt_byte);
    if (send_buf == NULL) {
        LOG_ERR("Not enough memory available\n");
        return QCLI_STATUS_ERROR_E;
    }

    memset(&ipv6, 0 , sizeof(struct sockaddr_in6));
    ipv6.sin_family = AF_INET6;
    ipv6.sin_port = htons(portnum);
    if (inet_pton(AF_INET6, ipaddr, &ipv6.sin_addr) != 0) {
        LOG_ERR("Invalid IPv6 address\n");
        return -1;
    }

    sockid = qc_drv_net_socket(qc_api_get_qc_drv_context(), AF_INET6, SOCK_STREAM, 0);
    if (sockid < 0) {
        LOG_ERR("Failed to open socket %d\n", sockid);
        return QCLI_STATUS_ERROR_E;
    }

    void *addr;
    addr = &(ipv6.sin_addr);
    inet_ntop(AF_INET6, addr, ipstr, sizeof(ipstr));
    LOG_INFO("IP address: %s\n", ipstr);

    if (qc_drv_net_connect(qc_api_get_qc_drv_context(), sockid, (struct sockaddr *)&ipv6, sizeof(ipv6)) < 0)
    {
        LOG_ERR("Failed to connect socket %d\n", sockid);
        return QCLI_STATUS_ERROR_E;
    }

    ticks = (uint32_t)qurt_timer_convert_ticks_to_time(qurt_timer_get_ticks(), QURT_TIME_MSEC);

    while ((uint32_t)qurt_timer_convert_ticks_to_time(qurt_timer_get_ticks(), QURT_TIME_MSEC) - ticks < (timeout * 1000))
    {
        memset(send_buf, 'A', peer_pkt_byte - 1);
        send_buf[peer_pkt_byte - 1] = 0;
        bytes = qc_drv_net_send(qc_api_get_qc_drv_context(), sockid, send_buf, peer_pkt_byte, 0);
        if (bytes <= 0) {
            LOG_INFO("qapi_send returns :%d\n", bytes);
        } else
            LOG_INFO("sent bytes%d\n", bytes);

        memset(send_buf, 0, peer_pkt_byte);
        bytes = qc_drv_net_recv(qc_api_get_qc_drv_context(), sockid, send_buf, peer_pkt_byte, 0);
        if (bytes == 0) {
            LOG_INFO("Server terminated!\r\n");
            break;
        }
        else if (bytes < 0)
        {
            LOG_INFO("qapi_send returns :%d\n", bytes);
        }
        LOG_INFO("Received bytes are: %d\ndata: %s\n", bytes, send_buf);
    }
    free(send_buf);
    qc_drv_net_socketclose(qc_api_get_qc_drv_context(), sockid);

    return QCLI_STATUS_SUCCESS_E;
#endif
}

QCLI_Command_Status_t qc_api_net_UdpV6Connect(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    uint16_t portnum;
    char *ipv6addr = NULL;
    struct sockaddr_in6 addr = {0};
    int sockid;

    if ((Parameter_Count != 2) || (!Parameter_List) || (!Parameter_List[1].Integer_Is_Valid))
    {
        return QCLI_STATUS_USAGE_E;
    }

    // Port number
    portnum = Parameter_List[1].Integer_Value;
    // IP addr
    ipv6addr = Parameter_List[0].String_Value;
    if (inet_pton(AF_INET6, ipv6addr, &addr.sin_addr.s_addr) != 0)
    {
        LOG_ERR("Invalid IP address\n");
        return -1;
    }

    sockid = net_sock_open(0, AF_INET6, SOCK_DGRAM, portnum, (uint8_t *)ipv6addr);
    if (sockid < 0)
    {
        LOG_ERR("Failed to open socket %d\n", sockid);
        return QCLI_STATUS_ERROR_E;
    }

    LOG_AT("Session id %d\n", sockid);
    return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t qc_api_net_UdpConnect(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    uint16_t portnum;
    char *ipaddr = NULL;
    struct sockaddr_in addr = {0};
    int sockid;

    if ((Parameter_Count != 2) || (!Parameter_List) || (!Parameter_List[1].Integer_Is_Valid))
    {
        return QCLI_STATUS_USAGE_E;
    }

    // Port number
    portnum = Parameter_List[1].Integer_Value;
    // IP addr
    ipaddr = Parameter_List[0].String_Value;
    if (inet_pton(AF_INET, ipaddr, &addr.sin_addr.s_addr) != 0)
    {
        LOG_ERR("Invalid IP address\n");
        return -1;
    }

    sockid = net_sock_open(0, AF_INET, SOCK_DGRAM, portnum, (uint8_t *)ipaddr);
    if (sockid < 0)
    {
        LOG_ERR("Failed to open socket %d\n", sockid);
        return QCLI_STATUS_ERROR_E;
    }

    LOG_AT("Session id %d\n", sockid);
    return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t qc_api_net_TcpV6Server(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    uint16_t portnum;
    char *ipv6addr = NULL;
    struct sockaddr_in6 addr = {0};
    int sockid;

    if ((Parameter_Count != 2) || (!Parameter_List) || (!Parameter_List[1].Integer_Is_Valid))
    {
        return QCLI_STATUS_USAGE_E;
    }

    // Port number
    portnum = Parameter_List[1].Integer_Value;

    // IP addr
    ipv6addr = Parameter_List[0].String_Value;

    if (inet_pton(AF_INET6, ipv6addr, &addr.sin_addr.s_addr) != 0)
    {
        LOG_ERR("Invalid IP address\n");
        return -1;
    }

    sockid = net_sock_open(1, AF_INET6, SOCK_STREAM, portnum, (uint8_t *)ipv6addr);
    if (sockid < 0)
    {
        LOG_ERR("Failed to open socket %d\n", sockid);
        return QCLI_STATUS_ERROR_E;
    }

    LOG_AT("Session id %d\n", sockid);
    return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t qc_api_net_TcpServer(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    uint16_t portnum;
    char *ipaddr = NULL;
    struct sockaddr_in addr = {0};
    int sockid;

    if ((Parameter_Count != 2) || (!Parameter_List) || (!Parameter_List[1].Integer_Is_Valid))
    {
        return QCLI_STATUS_USAGE_E;
    }

    // Port number
    portnum = Parameter_List[1].Integer_Value;

    // IP addr
    ipaddr = Parameter_List[0].String_Value;

    if (inet_pton(AF_INET, ipaddr, &addr.sin_addr.s_addr) != 0)
    {
        LOG_ERR("Invalid IP address\n");
        return -1;
    }

    sockid = net_sock_open(1, AF_INET, SOCK_STREAM, portnum, (uint8_t *)ipaddr);
    if (sockid < 0)
    {
        LOG_ERR("Failed to open socket %d\n", sockid);
        return QCLI_STATUS_ERROR_E;
    }

    LOG_AT("Session id %d\n", sockid);
    return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t qc_api_net_UdpServer(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    uint16_t portnum;
    char *ipaddr = NULL;
    struct sockaddr_in addr = {0};
    int sockid;

    if ((Parameter_Count != 2) || (!Parameter_List) || (!Parameter_List[1].Integer_Is_Valid))
    {
        return QCLI_STATUS_USAGE_E;
    }

    // Port number
    portnum = Parameter_List[1].Integer_Value;
    // IP addr
    ipaddr = Parameter_List[0].String_Value;
    if (inet_pton(AF_INET, ipaddr, &addr.sin_addr.s_addr) != 0)
    {
        LOG_ERR("Invalid IP address\n");
        return -1;
    }

    sockid = net_sock_open(1, AF_INET, SOCK_DGRAM, portnum, (uint8_t *)ipaddr);
    if (sockid < 0)
    {
        LOG_ERR("Failed to open socket %d\n", sockid);
        return QCLI_STATUS_ERROR_E;
    }

    LOG_AT("Session id %d\n", sockid);
    return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t qc_api_net_UdpV6Server(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    uint16_t portnum;
    char *ipv6addr = NULL;
    struct sockaddr_in6 addr = {0};
    int sockid;

    if ((Parameter_Count != 2) || (!Parameter_List) || (!Parameter_List[1].Integer_Is_Valid))
    {
        return QCLI_STATUS_USAGE_E;
    }

    // Port number
    portnum = Parameter_List[1].Integer_Value;
    // IP addr
    ipv6addr = Parameter_List[0].String_Value;

    if (inet_pton(AF_INET6, ipv6addr, &addr.sin_addr.s_addr) != 0)
    {
        LOG_ERR("Invalid IP address\n");
        return -1;
    }

    sockid = net_sock_open(1, AF_INET6, SOCK_DGRAM, portnum, (uint8_t *)ipv6addr);
    if (sockid < 0)
    {
        LOG_ERR("Failed to open socket %d\n", sockid);
        return QCLI_STATUS_ERROR_E;
    }

    LOG_ERR("Session id %d\n", sockid);
    return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t qc_api_net_SockInfo(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if ((Parameter_Count < 1) || (!Parameter_List) || (!Parameter_List[0].Integer_Is_Valid))
    {
        return QCLI_STATUS_USAGE_E;
    }

    if (net_sock_info(Parameter_List[0].Integer_Value))
        return QCLI_STATUS_ERROR_E;

    return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t qc_api_net_TxData(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    uint16_t portnum = 0;
    uint32_t sid;
    uint32_t len;
    char *ipaddr = NULL;
    struct sockaddr_in addr = {0};
    struct sockaddr_in6 addrv6 = {0};
    int32_t ret;

    if ((Parameter_Count < 2) || (Parameter_Count == 3) || (!Parameter_List) || (!Parameter_List[0].Integer_Is_Valid) || (!Parameter_List[1].Integer_Is_Valid))
    {
        return QCLI_STATUS_USAGE_E;
    }

    // Session ID number
    sid = Parameter_List[0].Integer_Value;
    // Data Length
    len = Parameter_List[1].Integer_Value;

    if (Parameter_Count > 3)
    {
    // IP addr
        ipaddr = Parameter_List[2].String_Value;
        if (inet_pton(AF_INET, ipaddr, &addr.sin_addr.s_addr) != 0)
        {
            if (inet_pton(AF_INET6, ipaddr, &addrv6.sin_addr.s_addr) != 0)
            {
                LOG_ERR("Invalid IP address\n");
                return QCLI_STATUS_USAGE_E;
            }
        }

        // Port number
        portnum = Parameter_List[3].Integer_Value;
    }

    ret = net_sock_set_active_session(sid, portnum, (uint8_t *)ipaddr);
    if (ret < 0)
    {
        LOG_ERR("Failed to set active session\n", ret);
        return QCLI_STATUS_ERROR_E;
    }

    // Enable Data Mode
    QCLI_Set_DataMode(1, len);

    return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t qc_api_net_RxData(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int ret;

    if ((Parameter_Count != 1) || (!Parameter_List[0].Integer_Is_Valid))
    {
        return QCLI_STATUS_USAGE_E;
    }

    if ((ret = net_sock_read(Parameter_List[0].Integer_Value)))
    {
        LOG_ERR("Read Failed ret = %d\n", ret);
        return QCLI_STATUS_ERROR_E;
    }

    return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t qc_api_net_SendData(char *tx_data, uint32_t len)
{
    int32_t ret;

    if ((ret = net_sock_send_data(tx_data, len)))
    {
        LOG_ERR("Send Failed with ret=%d\n", ret);
        return ret;
    }

    return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t qc_api_net_HttpClient(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if (0 == httpc_command_handler(Parameter_Count, Parameter_List)) {
        return QCLI_STATUS_SUCCESS_E;
    }

    return QCLI_STATUS_ERROR_E;
}

static void httpsvr_help(void)
{
    LOG_INFO("httpsvr init [v4|v6|v46] [http|https|http_https] [-c <cert_file>] [-p <httpport>] [-s <httpsport>] [-i <ifname>] [-x <index_page>] [-r <root_path>]\n");
    LOG_INFO("httpsvr [start|stop]\n");
    LOG_INFO("httpsvr [addctype|delctype] <content-type1> [<content-type2> ..]\n");
    LOG_INFO("httpsvr setbufsize <TX buffer size> <RX buffer size>\n");
    sslconfig_help("httpsvr sslconfig");
    LOG_INFO("Examples:\n");
    LOG_INFO(" httpsvr init v6 http\n");
    LOG_INFO(" httpsvr init v4 http_https -c cert.bin\n");
    LOG_INFO(" httpsvr init v46 https -c cert.bin -p 8080 -s 1443 -i wlan1 -x index.html -r /mywebfolder\n");
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
 * httpsvr status
 * httpsvr stop
 * httpsvr shutdown
 *****************************************************************************/
QCLI_Command_Status_t qc_api_net_HttpServer(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    char *cmd;
    int i;
    qapi_Status_t e;

    if (Parameter_Count == 0 || Parameter_List == NULL)
    {
        httpsvr_help();
        return QCLI_STATUS_SUCCESS_E;
    }

    cmd = Parameter_List[0].String_Value;

    if (strncmp(cmd, "status", 6) == 0)
    {
        qapi_Net_HTTPs_Status_t status;
        char *a = NULL, *m = NULL;

        if (qc_drv_net_HTTPs_Get_Status(qc_api_get_qc_drv_context(), &status) != QAPI_OK)
        {
            LOG_INFO("HTTP server has not been created!\n\n");
            return QCLI_STATUS_ERROR_E;
        }

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

            LOG_AT("HTTP server is %s (%s/%s)\n", qc_api_get_HTTPs_Is_Started(qc_api_get_qc_drv_context()) ? "running" : "stopped", a, m);

            if (status.mode == QAPI_NET_HTTPSVR_HTTP_E)
            {
                LOG_INFO("HTTP port: %d\n", status.http_Port);
            }
            else if (status.mode == QAPI_NET_HTTPSVR_HTTPS_E)
            {
                LOG_INFO("HTTPS port: %d\n", status.https_Port);
            }
            else if (status.mode == QAPI_NET_HTTPSVR_HTTP_AND_HTTPS_E)
            {
                LOG_INFO("HTTP port: %d  HTTPS port: %d\n", status.http_Port, status.https_Port);
            }

            LOG_INFO("Landing page: %s  Root path: %s\n", status.root_Index_Page, status.root_Path);

            LOG_INFO("TX bufsize: %u  RX bufsize: %u\n", status.txbufsize, status.rxbufsize);

            LOG_AT("\n");
            return QCLI_STATUS_SUCCESS_E;
    }
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
            LOG_ERR("\"%s\" is not supported.\n", Parameter_List[1].String_Value);
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
            LOG_ERR("\"%s\" is not supported.\n", Parameter_List[2].String_Value);
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
                            LOG_AT("Unknown option: %s\n", Parameter_List[i].String_Value);
                            return QCLI_STATUS_ERROR_E;
                    }
                }
                else
                {
                    LOG_AT("Unknown option: %s\n", Parameter_List[i].String_Value);
                    return QCLI_STATUS_ERROR_E;
                }

                if (i == Parameter_Count)
                {
                    LOG_AT("What is value of %s?\n", Parameter_List[i-1].String_Value);
                    return QCLI_STATUS_ERROR_E;
                }
            } /* for */
        }

        /* do it !! */
        if (qc_drv_net_HTTPs_Init(qc_api_get_qc_drv_context(), &cfg) != QAPI_OK)
        {
            LOG_ERR("Failed to Init HTTP server\n");
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
            LOG_AT("What are SSL parameters?\n");
            httpsvr_help();
            return QCLI_STATUS_ERROR_E;
        }

        if (httpsvr_sslcfg == NULL)
        {
            httpsvr_sslcfg = malloc(sizeof(qapi_Net_SSL_Config_t));
            if (httpsvr_sslcfg == NULL)
            {
                LOG_ERR("Allocation failure\n");
                return QCLI_STATUS_ERROR_E;
            }
            memset(httpsvr_sslcfg, 0, sizeof(qapi_Net_SSL_Config_t));
        }

        /* Parse SSL config parameters from command line */
        if (ssl_parse_config_parameters(Parameter_Count-1, &Parameter_List[1], httpsvr_sslcfg, 0, true) == QCLI_STATUS_ERROR_E)
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
            e = qc_drv_net_HTTPs_Set_SSL_Config(qc_api_get_qc_drv_context(), httpsvr_sslcfg);
            if (e != QAPI_OK)
            {
                LOG_ERR("SSL config failed\n");
            }
        }

        if (qc_drv_net_HTTPs_Start(qc_api_get_qc_drv_context()) != QAPI_OK)
        {
            LOG_INFO("Have you done 'httpsvr init' yet?\n");
            return QCLI_STATUS_ERROR_E;
        }
    }

    /* httpsvr  stop */
    else if (strncmp(cmd, "stop", 3) == 0)
    {
        qc_drv_net_HTTPs_Stop(qc_api_get_qc_drv_context());
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
        qc_drv_net_HTTPs_Shutdown(qc_api_get_qc_drv_context());
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
            e = qc_drv_net_HTTPs_Register_Content_Type(qc_api_get_qc_drv_context(), Parameter_List[i].String_Value, strlen(Parameter_List[i].String_Value));
            if (e != QAPI_OK)
            {
                LOG_ERR("Failed to register \"%s\"\n", Parameter_List[i].String_Value);
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
            qc_drv_net_HTTPs_Unregister_Content_Type(qc_api_get_qc_drv_context(), Parameter_List[i].String_Value, strlen(Parameter_List[i].String_Value));
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

        if (Parameter_List[1].Integer_Is_Valid && Parameter_List[2].Integer_Is_Valid)
        {
            txbufsize = Parameter_List[1].Integer_Value;
            rxbufsize = Parameter_List[2].Integer_Value;
            e = qc_drv_net_HTTPs_Set_Buffer_Size(qc_api_get_qc_drv_context(), txbufsize, rxbufsize);
            if (e != QAPI_OK)
            {
                LOG_ERR("Failed to set buffer size\n");
                return QCLI_STATUS_ERROR_E;
            }
        }
        else
        {
            LOG_ERR("Invalid size\n");
            return QCLI_STATUS_ERROR_E;
        }
    }

    else
    {
        LOG_ERR("\"%s\" is not supported.\n", cmd);
        return QCLI_STATUS_ERROR_E;
    }

    return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t qc_api_net_MsockCreate(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    /* Handling upto 6 parameters according to Specification */
    if (Parameter_Count != 6 || !Parameter_List || !Parameter_List[2].Integer_Is_Valid || !Parameter_List[3].Integer_Is_Valid || !Parameter_List[4].Integer_Is_Valid || !Parameter_List[5].Integer_Is_Valid){
        return QCLI_STATUS_USAGE_E;
    }

    if (0 == multi_sock_create(Parameter_List))
    {
        return QCLI_STATUS_SUCCESS_E;
    }

    return QCLI_STATUS_ERROR_E;
}

QCLI_Command_Status_t qc_api_net_MsockClose(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if (0 == multi_sock_close())
    {
        return QCLI_STATUS_SUCCESS_E;
    }

    return QCLI_STATUS_ERROR_E;
}

QCLI_Command_Status_t qc_api_net_cert_handler(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if(Parameter_Count < 1)
    {
        return QCLI_STATUS_USAGE_E;
    }

    if (0 == strcmp(Parameter_List[0].String_Value, "store")) {
        return store_cert(Parameter_Count - 1, &Parameter_List[1]);
    }
    else if (0 == strcmp(Parameter_List[0].String_Value, "delete")) {
        return delete_cert(Parameter_Count - 1, &Parameter_List[1]);
    }
    else if (0 == strcmp(Parameter_List[0].String_Value, "list")) {
        return list_cert(Parameter_Count - 1, &Parameter_List[1]);
    }
    else if (0 == strcmp(Parameter_List[0].String_Value, "get")) {
        return get_cert(Parameter_Count - 1, &Parameter_List[1]);
    }
    else if (0 == strcmp(Parameter_List[0].String_Value, "hash")) {
        return hash_cert(Parameter_Count - 1, &Parameter_List[1]);
    }
    else if (0 == strcmp(Parameter_List[0].String_Value, "gencsr")) {
        return generate_csr(Parameter_Count - 1, &Parameter_List[1]);
    }

    return QCLI_STATUS_USAGE_E;
}


QCLI_Command_Status_t qc_api_net_ssl_handler(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if(Parameter_Count < 1)
    {
        return QCLI_STATUS_USAGE_E;
    }

    if (0 == strncmp(Parameter_List[0].String_Value, "start", 3)) {
        return ssl_start(Parameter_Count - 1, &Parameter_List[1]);
    }
    else if (0 == strncmp(Parameter_List[0].String_Value, "stop", 3)) {
        return ssl_stop(Parameter_Count - 1, &Parameter_List[1]);
    }
    else if (0 == strncmp(Parameter_List[0].String_Value, "config", 3)) {
        return ssl_config(Parameter_Count - 1, &Parameter_List[1]);
    }
    else if (0 == strncmp(Parameter_List[0].String_Value, "cert", 3)) {
        return ssl_add_cert(Parameter_Count - 1, &Parameter_List[1]);
    }
    else if (0 == strncmp(Parameter_List[0].String_Value, "psk", 3)) {
        return ssl_add_psk_table(Parameter_Count - 1, &Parameter_List[1]);
    }
    else if (0 == strncmp(Parameter_List[0].String_Value, "ecjpake", 7)) {
        return ssl_set_ecjpake_params(Parameter_Count - 1, &Parameter_List[1]);
    }
    else if (0 == strncmp(Parameter_List[0].String_Value, "max_clients", 10)) {
        return ssl_set_dtls_server_max_clients(Parameter_Count - 1, &Parameter_List[1]);
    }
    else if (0 == strncmp(Parameter_List[0].String_Value, "idle_timer", 10)) {
        return ssl_set_dtls_server_idle_timer(Parameter_Count - 1, &Parameter_List[1]);
    }
	else
    {
        return QCLI_STATUS_USAGE_E;
    }
}

QCLI_Command_Status_t qc_api_net_benchtx(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if (0 == bench_common_tx4(Parameter_Count, Parameter_List))
    {
        return QCLI_STATUS_SUCCESS_E;
    }

    return QCLI_STATUS_ERROR_E;
}

QCLI_Command_Status_t qc_api_net_benchrx(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if (0 == bench_common_rx4(Parameter_Count, Parameter_List))
    {
        return QCLI_STATUS_SUCCESS_E;
    }

    return QCLI_STATUS_ERROR_E;
}

