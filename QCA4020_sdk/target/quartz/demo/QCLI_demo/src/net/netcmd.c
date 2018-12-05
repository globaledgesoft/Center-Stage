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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "qurt_types.h"
#include "qcli.h"
#include "qcli_api.h"
#include "qapi_socket.h"
#include "qapi_netservices.h"
#include "bench.h"
#include "qapi_wlan_bridge.h"
#include "net_demo.h"
#include "iperf.h"

#include "qapi_netprofile.h"
#include "qapi_omtm.h"
#include "qapi_httpsvr.h"
#include "qapi_netbuf.h"
#include "qapi_ns_utils.h"
#include "qapi_ns_gen_v4.h"
#include "qapi_crypto.h"
#include "qapi_ua.h"
#include "httpsvr/cgi/htmldata.h"
#include "../export/platform_oem.h"

/* TEMP */
#define QCA4020 1

#ifdef CONFIG_NET_DNSSD_DEMO
/* mDNS interface name */
static qapi_Net_DNSSD_Ctxt_t *qapi_Net_DNSSD_Ctxt;
#endif

#ifdef CONFIG_NET_PING_DEMO
static QCLI_Command_Status_t ping(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t ping6(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
#endif
#ifdef CONFIG_NET_DHCPV4C_DEMO
static QCLI_Command_Status_t dhcpv4c(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
#endif
#ifdef CONFIG_NET_AUTOIP_DEMO
static QCLI_Command_Status_t autoip(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
#endif
static QCLI_Command_Status_t ifconfig(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
#ifdef CONFIG_NET_DHCPV6C_DEMO
static QCLI_Command_Status_t dhcpv6c(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
#endif
#ifdef CONFIG_NET_SNTPC_DEMO
static QCLI_Command_Status_t sntpc(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
#endif
#ifdef CONFIG_NET_DNSC_DEMO
static QCLI_Command_Status_t dnsc(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
#endif
#ifdef CONFIG_NET_WLAN_BRIDGE_DEMO
static QCLI_Command_Status_t bridge(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
#endif
static QCLI_Command_Status_t cfg_netbuf(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
#ifdef CONFIG_NET_DHCPV4C_DEMO
static QCLI_Command_Status_t dhcpv4s(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
#endif
static QCLI_Command_Status_t nsstrrcl(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
#ifdef CONFIG_NET_DNSS_DEMO
static QCLI_Command_Status_t dnss(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
#endif
#ifdef CONFIG_NET_HTTPS_DEMO
static QCLI_Command_Status_t httpsvr(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
#endif
#ifdef CONFIG_NET_MDNSS_DEMO
static QCLI_Command_Status_t mdnss(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
#endif
#ifdef CONFIG_NET_DNSSD_DEMO
static QCLI_Command_Status_t dnssd(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
#endif
static QCLI_Command_Status_t freeq(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
#ifdef CONFIG_NET_TXRX_DEMO
static QCLI_Command_Status_t dump_command_handler(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
#endif
#ifdef CONFIG_NET_ROUTE_DEMO
static QCLI_Command_Status_t set_ipv6_router_prefix(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t route6(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t route(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
#endif
#ifdef CONFIG_NET_RAW_SOCKET_DEMO
extern QCLI_Command_Status_t eth(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
#endif
#ifdef CONFIG_NET_TCP_KEEPALIVE_DEMO
static QCLI_Command_Status_t tcpka(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
#endif
#ifdef CONFIG_NET_MQTTC_DEMO
QCLI_Command_Status_t mqttc(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
#endif
#ifdef CONFIG_NET_USER_ACCOUNT_DEMO
static QCLI_Command_Status_t user(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
#endif
#ifdef CONFIG_NET_WEBSOCKETC_DEMO
QCLI_Command_Status_t websocketc_command_handler(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
#endif

#define DHCP_INFINITE_LEASE_TIME "infinite"

const QCLI_Command_t net_cmd_list[] =
{
#ifdef CONFIG_NET_PING_DEMO
    /* Command_Function, Start_Thread, Command_String, Usage_String, Description */
    {ping,      true,   "ping",     "\n\nping <host> [-c <count>] [-s <size>]\n",
                                    "\nSend ICMP ECHO_REQUEST to network hosts in IPv4 network"},
#endif
#ifdef CONFIG_NET_DHCPV4C_DEMO
    {dhcpv4c,   false,  "dhcpv4c",  "\n\ndhcpv4c <interface> [new|release]\n",
                                    "\nDHCPv4 Client: Acquire an IPv4 address using Dynamic Host Configuration Protocol v4"},
#endif
#ifdef CONFIG_NET_AUTOIP_DEMO
    {autoip,    false,  "autoipv4", "\n\nautoipv4 <interface>\n",
                                    "\nAuto IPv4: Generate an IPv4 Link-Local address"},
#endif
    {ifconfig,  false,  "ifconfig", "\n\nifconfig <interface> [(<ipv4addr> <subnetmask> <default_gateway>)]\n",
                                    "\nConfigure a network interface"},
#ifdef CONFIG_NET_PING_DEMO
    {ping6,     true,   "ping6",    "\n\nping6 <host> [-c <count>] [-s <size>] [-I <interface>]\n",
                                    "\nSend ICMP ECHO_REQUEST to network hosts in IPv6 network"},
#endif
#ifdef CONFIG_NET_DHCPV6C_DEMO
    {dhcpv6c,   false,  "dhcpv6c",  "\n\ndhcpv6c <interface> [enable|new|confirm|disable|release]\n",
                                    "\nDHCPv6 Client: Acquire an IPv6 address using Dynamic Host Configuration Protocol v6"},
#endif
#ifdef CONFIG_NET_ROUTE_DEMO
    {set_ipv6_router_prefix,
                false,  "prefix",   "\n\nprefix <interface> [(<ipv6addr> <prefixlen> <prefix_lifetime> <valid_lifetime>)]\n",
                                    "\nSet the IPv6 prefix for IPv6 routing"},
#endif
#ifdef CONFIG_NET_SNTPC_DEMO
    {sntpc,     false,  "sntpc",    "\n\nsntpc [start|stop|disable]\n"
                                        "sntpc [addsvr <server> <id>]\n"
                                        "sntpc [delsvr <id>]\n"
                                        "sntpc [utc]\n",
                                        "\nSNTP Client: Acquire time from the network using Simple Network Time Protocol v4"},
#endif
#ifdef CONFIG_NET_DNSC_DEMO
    {dnsc,      false,  "dnsc",     "\n\ndnsc [start|stop|disable]\n"
                                        "dnsc addsvr <server> [<id>]\n"
                                        "dnsc delsvr <id>\n"
                                        "dnsc gethostbyname <host>\n"
                                        "dnsc [resolve|gethostbyname2] <host> [v4|v6]\n",
                                        "\nDNS Client: Resolves and caches Domain Name System domain names"},
#endif
#ifdef CONFIG_NET_WLAN_BRIDGE_DEMO
    {bridge,    false,  "bridge",   "\n\nbridge [enable|disable]\n"
                                        "bridge [aging <val>]\n"
                                        "bridge [showmacs]\n",
                                        "\nWLAN Bridge: Configure IEEE 802.1D Layer 2 bridging over WLAN interfaces"},
#endif
    {cfg_netbuf, false, "profile",  "\n\nprofile set <profile_id> (Where profile_id is 1=performance, 2=best effort, 3=memory optimized)\n"
                                        "profile custom [(<pool_size> (<buffer_size> <buffer_amount>...))]\n"
                                        "profile show\n",
                                        "\nConfigure network buffer pools"},
#ifdef CONFIG_NET_SSL_DEMO
    {cert_command_handler,
                false,  "cert",     "\n\ncert [store|delete|list|get|hash|download|gencsr|validate|expiration] <argument>...\n",
                                    "\nX.509 Certificate manager: Perform certificate management operations.\n"
                                    "Type command name to get more info on usage. For example \"cert store\".\n"},
    {ssl_command_handler,
                false,  "ssl",      "\n\nssl [start|stop|config|cert|psk|ecjpake|max_clients|idle_timer] <argument>...\n",
                                    "\nSecure Socket Layer: Configure Secure Socket Layer for TLS connections\n"
                                    "Type command name to get more info on usage. For example \"ssl start\"."},
#endif
#ifdef CONFIG_NET_TXRX_DEMO
    {bench_common_rx4,
                true,   "benchrx",  "\n\nType \"benchrx\" to get more info on usage\n",
                                    "\nPerform IPv4 receive (RX) benchmarking test"},
    {bench_common_tx4,
                true,   "benchtx",  "\n\nType \"benchtx\" to get more info on usage\n",
                                    "\nPerform IPv4 transmit (TX) benchmarking test"},
    {benchquit, false,  "benchquit", "\n\nbenchquit [rx|tx] <sessionid_for_tcprx>\n",
                                    "\nTerminate some or all ongoing benchmarking tests"},
#ifdef QCA4020
    {bench_uapsd_test,
                true,   "uapsdtest", "\n\n uapsdtest (<Remote IP> <port> <the number of packets> <time interval> <access category>)\n",
                                     "\nPerform transmit traffic test to send packets with specified WLAN Access Category"},
#endif
#endif
#ifdef CONFIG_NET_DHCPV4C_DEMO
    {dhcpv4s,   false,  "dhcpv4s",  "\n\ndhcpv4s <interface name> pool (<start_ip> <end_ip>) [<lease_time_sec>|infinite]\n",
                                    "\nDHCPv4 Server: Set up and configure Dynamic Host Configuration Protocol v4 server"},
#endif
    {nsstrrcl,  false,  "nsstrrcl", "\n\nnsstrrcl exit\n",
                                    "\nNetwork stack store-recall test (internal)"},
#ifdef CONFIG_NET_DNSS_DEMO
    {dnss,      false,  "dnss",     "\n\ndnss [start|stop|disable]\n"
                                        "dnss [addhost (<hostname> <host IP address>)] [<time-to-live>]\n"
                                        "dnss [delhost <hostname>]\n",
                                        "\nDNS Server: Set up and configure Dynamic Name System server"},
#endif
#ifdef CONFIG_NET_TXRX_DEMO
    {bench_common_rx6,
                true,   "benchrx6", "\n\nType \"benchrx6\" to get more info on usage\n",
                                    "\nPerform IPv6 receive (RX) benchmarking test"},
    {bench_common_tx6,
                true,   "benchtx6", "\n\nType \"benchtx6\" to get more info on usage\n",
                                    "\nPerform IPv6 transmit (TX) benchmarking test"},
#endif
#ifdef CONFIG_NET_HTTPS_DEMO
    {httpsvr,   false,  "httpsvr",  "\n\nType \"httpsvr\" to get more info on usage\n",
                                      "\nHTTP Server: Set up and configure Hypertext Transfer Protocol server"},
#endif
#ifdef CONFIG_NET_MDNSS_DEMO
	{mdnss,     false,  "mdnss",    "\n\nmdnss start [ipv4|ipv6|ipboth] <interface> <blocking>\n"
	                                    "mdnss stop <interface>\n"
	                                    "mdnss sethostname <hostname>\n"
	                                    "mdnss addsvc [(<instance_name> <port> <txt>)]\n"
                                        "mdnss rmsvc <instance_name>\n"
			                            "mdnss updatetxt [(<service_name> <txt>)]\n",
			                            "\nmDNS Server: Set up and configure Multicast Domain Name System server"},
#endif
#ifdef CONFIG_NET_HTTPC_DEMO
    {httpc_command_handler,
                false,  "httpc",    "\n\nhtppc [start|stop]\n"
                                        "httpc [connect|disconnect|get|post|put|patch] <...>\n",
                                        "\nHTTP Client: Perform Hypertext Transport protocol client operations.\n"
                                        "Type command name to get more info on usage. For example \"httpc get\".\n"},
#endif
#ifdef CONFIG_NET_WEBSOCKETC_DEMO
    {websocketc_command_handler,
				false,  "websocketc",   "websocketc [connect|disconnect] <...>\n",
										"\nWebsocket Client: Perform Websocket client operations.\n"
										"Type command name to get more info on usage. For example \"websocketc connect\".\n"},
#endif
#ifdef CONFIG_NET_DNSSD_DEMO
    {dnssd,     false,  "dnssd",    "\n\ndnssd start [ipv4|ipv6|ipboth] <interface>\n"
                                        "dnssd stop\n"
                                        "dnssd init <timeout in milliseconds>\n"
                                        "dnssd discover <instance name>\n"
                                        "dnssd gettargetinfo <instance name>\n",
                                        "\nDNS-SD: Setup and configure Domain Name System Service Discovery"},
#endif
#ifdef CONFIG_NET_MQTTC_DEMO
    {mqttc,     false,  "mqttc",    "\n\nType \"mqttc\" to get more info on usage\n",
                                        "\nMQTT Client: Set up and configure MQ Telemetry Transport client"},
#endif
    {freeq,     false,  "free",     "\n\n", "Show the status of network buffers and sockets in the system"},
#ifdef CONFIG_NET_TXRX_DEMO
    {iperf,     true,   "iperf",    "\n\nType \"iperf\" to get more info on usage\n",
                                        "\niperf: Perform network throughput tests"},
	{dump_command_handler,
	            true,   "dump",     "\n\ndump [enable|disable] <tx> <rx>\n",
	                                "\nConfigure TX/RX data dump on benchtx/rx tests"},
	{bench_common_set_pattern,
	            false,  "pattern",  "\n\npattern set <pattern_type> <pattern>\n"
	                                   "Where pattern_type is:\n0: default (continuous pattern)\n"
									   "1: static pattern, specify a byte like 0xAA for <pattern>\n"
									   "2: ASCII characters\n",
									   "\nConfigure the payload data pattern to be used benchmarking transmission tests"},
#endif
#ifdef CONFIG_NET_ROUTE_DEMO
    {route6,    false,  "route6",   "\n\nroute6 add [(<dest> <next_hop> <interface>)]\n"
                                        "route6 del [(<dest> <interface>)]\n"
                                        "route6 show\n",
                                        "\nShow and manipulate the IPv6 routing table"},
	{route, 	false, 	"route", 	"\n\nroute show <interface>\n"
								        "route add [(<ifname> <ipv4addr> <mask> <gateway>)]\n"
							            "route del <ifname> <ipv4addr> <mask> <gateway>\n", 
									    "\nShow and manipulate the IPv4 routing table"},
#endif
#ifdef CONFIG_NET_RAW_SOCKET_DEMO
    {eth,       true,   "eth",      "\n\nType \"eth\" to get more info on usage\n\n", "Tx/Rx Ethernet II frames"},
#endif
#ifdef CONFIG_NET_TCP_KEEPALIVE_DEMO
    {tcpka,     false,  "tcpka",    "\n\ntcpka [-i idle_time (in seconds)] [-r resp_timeout (in seconds)]\n", "Get/Set TCP Keepalive parameters"},
#endif
#ifdef CONFIG_NET_TXRX_DEMO
    {queuecfg,
                true,   "queuecfg", "\n\nqueuecfg [tx|rx] <size_in_bytes>\n",
                                    "\nConfigure socket transmission or reception queue size, in bytes"},
#endif
#ifdef CONFIG_NET_USER_ACCOUNT_DEMO
    {user,     false,  "user",    "\n\nType \"user\" to get more info on usage\n", "User account management"},
#endif
};

const QCLI_Command_Group_t net_cmd_group =
{
    "Net",              /* Group_String: will display cmd prompt as "Net> " */
    sizeof(net_cmd_list)/sizeof(net_cmd_list[0]),   /* Command_Count */
    net_cmd_list        /* Command_List */
};

QCLI_Group_Handle_t qcli_net_handle;     /* Handle for Net Command Group. */

#ifdef CONFIG_NET_SNTPC_DEMO
static char msg_invalid_id[] = "Invalid server id\n";
#endif
#ifdef CONFIG_NET_HTTPS_DEMO
#ifdef CONFIG_NET_SSL_DEMO
static qapi_Net_SSL_Config_t * httpsvr_sslcfg;
#endif
#endif
#define MAX_FQDN_LEN   255

/*****************************************************************************
 * This function is used to register the Net Command Group with QCLI.
 *****************************************************************************/
void Initialize_Net_Demo(void)
{
    /* Attempt to reqister the Command Groups with the qcli framework.*/
    qcli_net_handle = QCLI_Register_Command_Group(NULL, &net_cmd_group);
    if (qcli_net_handle)
    {
        QCLI_Printf(qcli_net_handle, "Net Registered\n");
    }

#ifdef FS_INIT_COLD
    /* Initialize user account service */
    if (qapi_Net_User_Init() != QAPI_OK)
    {
        QCLI_Printf(qcli_net_handle, "ERROR: Failed to init User Account service\n");
    }
#endif
    return;
}

#ifdef CONFIG_NET_USER_ACCOUNT_DEMO
/*****************************************************************************
 *      [0]     [1]        [2]        [3] [4]
 * user adduser <username> <password> [-s <service>]
 * user deluser <username> <password>
 * user passwd  <username> <password>
 * user addservice <username> <password> <service>
 * user delservice <username> <password> <service>
 * user list
 *****************************************************************************/
static void user_help(void)
{
    QCLI_Printf(qcli_net_handle, "user add <username> <password> [-s <service>]\n");
    QCLI_Printf(qcli_net_handle, "user del <username> <password>\n");
    QCLI_Printf(qcli_net_handle, "user passwd <username> <password> <new password>\n");
    QCLI_Printf(qcli_net_handle, "user addsvc <username> <password> <service>\n");
    QCLI_Printf(qcli_net_handle, "user delsvc <username> <password> <service>\n");
    QCLI_Printf(qcli_net_handle, "  where 'service' is a bitmask. 0x1 = HTTP service\n");
    QCLI_Printf(qcli_net_handle, "user list\n");
    QCLI_Printf(qcli_net_handle, "Examples:\n");
    QCLI_Printf(qcli_net_handle, " user add admin admin -s 0x1\n");
    QCLI_Printf(qcli_net_handle, " user del admin admin\n");
    QCLI_Printf(qcli_net_handle, " user passwd admin admin my_new_password\n");
    QCLI_Printf(qcli_net_handle, " user addsvc admin admin 0x1\n");
}

static char * get_user_err_str(qapi_Status_t sts)
{
    char *str = "";

    switch (sts)
    {
        case QAPI_NET_STATUS_USER_INTERNAL_ERROR:
            str = "Internal error";
            break;

        case QAPI_NET_STATUS_USER_INVALID_PARAMETER:
            str = "Invalid username or password";
            break;

        case QAPI_NET_STATUS_USER_TABLE_FULL:
            str = "User account table is full";
            break;

        case QAPI_NET_STATUS_USER_NOT_FOUND:
            str = "User is not found";
            break;

        case QAPI_NET_STATUS_USER_ALREADY_EXIST:
            str = "User already exists";
            break;

        case QAPI_NET_STATUS_USER_OPEN_ERROR:
            str = "Failed to open user account table";
            break;

        case QAPI_NET_STATUS_USER_WRITE_ERROR:
            str = "Failed to write user account table";
            break;

        case QAPI_NET_STATUS_USER_READ_ERROR:
            str = "Failed to read user account table";
            break;

        case QAPI_NET_STATUS_USER_INCORRECT_PASSWORD:
            str = "Incorrect password";
            break;
    }

    return str;
}

static QCLI_Command_Status_t user(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    char *cmd;
    char *username, *password;
    int i;
    qapi_Status_t sts;

    if (Parameter_Count < 1)
    {
        user_help();
        return QCLI_STATUS_SUCCESS_E;
    }

    cmd = Parameter_List[0].String_Value;

    if (strncmp(cmd, "list", 1) == 0)
    {
        uint32_t size;
        char *buf;
        qapi_Net_User_Info_t *up;
        int num;

        num = qapi_Net_User_List(NULL);
        if (num >= 0)
        {
            QCLI_Printf(qcli_net_handle, "%d users:\n", num);

            if (num > 0)
            {
                size = num * sizeof(qapi_Net_User_Info_t);
                buf = malloc(size);
                if (buf == NULL)
                {
                    QCLI_Printf(qcli_net_handle, "ERROR: no memory\n");
                    return QCLI_STATUS_ERROR_E;
                }
                memset(buf, 0, size);

                up = (qapi_Net_User_Info_t *)buf;
                if ((num = qapi_Net_User_List(up)) < QAPI_OK)
                {
                    free(buf);
                    QCLI_Printf(qcli_net_handle, "ERROR: %s\n", get_user_err_str((qapi_Status_t)num));
                    return QCLI_STATUS_ERROR_E;
                }

                for (i = 0; i < num; ++i)
                {
                    QCLI_Printf(qcli_net_handle, "[%d]: %s  0x%02x\n", i+1, up->username, up->service);
                    ++up;
                }
                free(buf);
            }
        }
        else
        {
            QCLI_Printf(qcli_net_handle, "ERROR: %s\n", get_user_err_str((qapi_Status_t)num));
            return QCLI_STATUS_ERROR_E;
        }

        return QCLI_STATUS_SUCCESS_E;
    }

    if (Parameter_Count < 3)
    {
        user_help();
        return QCLI_STATUS_ERROR_E;
    }

    username = Parameter_List[1].String_Value;
    password = Parameter_List[2].String_Value;

    /*      [0]  [1]        [2]       [3]  [4]
     * user add <username> <password> [-s  <service>]
     */
    if (strcmp(cmd, "add") == 0)
    {
        uint32_t service = SERVICE_ALL;

        if (Parameter_Count >= 4)   /* there are options */
        {
            for (i = 3; i < Parameter_Count; ++i)
            {
                if (Parameter_List[i].String_Value[0] == '-')
                {
                    switch (Parameter_List[i].String_Value[1])
                    {
                        /* service */
                        case 's':
                            i++;
                            if (Parameter_List[i].Integer_Is_Valid)
                            {
                                service = Parameter_List[i].Integer_Value;
                            }
                            break;

                        default:
                            QCLI_Printf(qcli_net_handle, "Unknown option: %s\n", Parameter_List[i].String_Value);
                            return QCLI_STATUS_ERROR_E;
                    }
                }
                else
                {
                    QCLI_Printf(qcli_net_handle, "Unknown option: %s\n", Parameter_List[i].String_Value);
                    return QCLI_STATUS_ERROR_E;
                }

                if (i == Parameter_Count)
                {
                    QCLI_Printf(qcli_net_handle, "What is value of %s?\n", Parameter_List[i-1].String_Value);
                    return QCLI_STATUS_ERROR_E;
                }
            } /* for */
        }

        if ((sts = qapi_Net_User_Add((const char *)username, (const char *)password, service)) != QAPI_OK)
        {
            QCLI_Printf(qcli_net_handle, "ERROR: %s\n", get_user_err_str(sts));
            return QCLI_STATUS_ERROR_E;
        }
    }

    /*      [0]   [1]        [2]        
     * user del  <username> <password>
     */
    else if (strcmp(cmd, "del") == 0)
    {
        if ((sts = qapi_Net_User_Del((const char *)username, (const char *)password)) != QAPI_OK)
        {
            QCLI_Printf(qcli_net_handle, "ERROR: %s\n", get_user_err_str(sts));
            return QCLI_STATUS_ERROR_E;
        }
    }

    /*      [0]     [1]        [2]        [3] 
     * user passwd <username> <password> <new password>
     */
    else if (strcmp(cmd, "passwd") == 0)
    {
        char *new_password;

        if (Parameter_Count < 4)
        {
            user_help();
            return QCLI_STATUS_ERROR_E;
        }

        new_password = Parameter_List[3].String_Value;

        if ((sts = qapi_Net_User_Change_Password((const char *)username, (const char *)password, (const char *)new_password)) != QAPI_OK)
        {
            QCLI_Printf(qcli_net_handle, "ERROR: %s\n", get_user_err_str(sts));
            return QCLI_STATUS_ERROR_E;
        }
    }

    /*      [0]         [1]        [2]        [3]
     * user addservice <username> <password> <service>
     */
    else if (strcmp(cmd, "addsvc") == 0)
    {
        uint32_t service;

        if (Parameter_Count < 4)
        {
            user_help();
            return QCLI_STATUS_ERROR_E;
        }

        if (!Parameter_List[3].Integer_Is_Valid)
        {
            QCLI_Printf(qcli_net_handle, "Invalid service value\n");
            return QCLI_STATUS_ERROR_E;
        }

        service = Parameter_List[3].Integer_Value;
        if ((sts = qapi_Net_User_Add_Service((const char *)username, (const char *)password, service)) != QAPI_OK)
        {
            QCLI_Printf(qcli_net_handle, "ERROR: %s\n", get_user_err_str(sts));
            return QCLI_STATUS_ERROR_E;
        }
    }

    /*      [0]    [1]        [2]         [3]
     * user delsvc <username> <password> <service>
     */
    else if (strcmp(cmd, "delsvc") == 0)
    {
        uint32_t service;

        if (Parameter_Count < 4)
        {
            user_help();
            return QCLI_STATUS_ERROR_E;
        }

        if (!Parameter_List[3].Integer_Is_Valid)
        {
            QCLI_Printf(qcli_net_handle, "Invalid service value\n");
            return QCLI_STATUS_ERROR_E;
        }

        service = Parameter_List[3].Integer_Value;
        if ((sts = qapi_Net_User_Del_Service((const char *)username, (const char *)password, service)) != QAPI_OK)
        {
            QCLI_Printf(qcli_net_handle, "ERROR: %s\n", get_user_err_str(sts));
            return QCLI_STATUS_ERROR_E;
        }
    }

    else
    {
        QCLI_Printf(qcli_net_handle, "\"%s\" is not supported.\n", cmd);
        return QCLI_STATUS_ERROR_E;
    }

    return QCLI_STATUS_SUCCESS_E;
}
#endif

#ifdef CONFIG_NET_TCP_KEEPALIVE_DEMO
/*****************************************************************************
 *       [0] [1] [2] [3]
 * tcpka
 * tcpka -i  300 -r  75
 *****************************************************************************/
static QCLI_Command_Status_t tcpka(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int32_t idle_sec = 7200;    /* 2 hours */
    int32_t timeout_sec = 75;
    int i;

    if (qapi_Net_TCP_Keepalive_Get(&idle_sec, &timeout_sec) != QAPI_OK)
    {
        QCLI_Printf(qcli_net_handle, "ERROR: failed to get TCP keepalive parameters\n");
        return QCLI_STATUS_ERROR_E;
    }

    if (Parameter_Count < 1)
    {
        QCLI_Printf(qcli_net_handle, "TCP keepalive Idle time: %ds\n", idle_sec);
        QCLI_Printf(qcli_net_handle, "TCP keepalive Probe Response timeout: %ds\n", timeout_sec);
        return QCLI_STATUS_SUCCESS_E;
    }

    for (i = 0; i < Parameter_Count ; i++)
    {
        if (strcmp(Parameter_List[i].String_Value, "-i") == 0)
        {
            ++i;
            if (Parameter_List[i].Integer_Is_Valid)
            {
                idle_sec = Parameter_List[i].Integer_Value;
            }
        }
        else if (strcmp(Parameter_List[i].String_Value, "-r") == 0)
        {
            ++i;
            if (Parameter_List[i].Integer_Is_Valid)
            {
                timeout_sec = Parameter_List[i].Integer_Value;
            }
        }
    }

    if (qapi_Net_TCP_Keepalive_Set(idle_sec, timeout_sec) != QAPI_OK) 
    {
        QCLI_Printf(qcli_net_handle, "ERROR: failed to set TCP Keepalive parameters\n");
        return QCLI_STATUS_ERROR_E;
    }

    return QCLI_STATUS_SUCCESS_E;
}
#endif

#ifdef CONFIG_NET_HTTPS_DEMO
/*****************************************************************************
 *****************************************************************************/
static void httpsvr_help(void)
{
    QCLI_Printf(qcli_net_handle, "httpsvr init [v4|v6|v46] [http|https|http_https] [-c <cert_file>] [-p <httpport>] [-s <httpsport>] [-i <ifname>] [-x <index_page>] [-r <root_path>]\n");
    QCLI_Printf(qcli_net_handle, "httpsvr [start|stop]\n");
    QCLI_Printf(qcli_net_handle, "httpsvr [addctype|delctype] <content-type1> [<content-type2> ..]\n");
    QCLI_Printf(qcli_net_handle, "httpsvr setbufsize <TX buffer size> <RX buffer size>\n");
#ifdef CONFIG_NET_SSL_DEMO
    sslconfig_help("httpsvr sslconfig");
#endif

    QCLI_Printf(qcli_net_handle, "Examples:\n");
    QCLI_Printf(qcli_net_handle, " httpsvr init v6 http\n");
    QCLI_Printf(qcli_net_handle, " httpsvr init v4 http_https -c cert.bin\n");
    QCLI_Printf(qcli_net_handle, " httpsvr init v46 https -c cert.bin -p 8080 -s 1443 -i wlan1 -x index.html -r /mywebfolder\n");
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
static QCLI_Command_Status_t httpsvr(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
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
            QCLI_Printf(qcli_net_handle, "HTTP server has not been created. Please do \"httpsvr init\".\n\n");
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

            QCLI_Printf(qcli_net_handle, "HTTP server is %s (%s/%s)\n",
                    qapi_Net_HTTPs_Is_Started() ? "running" : "stopped", a, m);

            if (status.mode == QAPI_NET_HTTPSVR_HTTP_E)
            {
                QCLI_Printf(qcli_net_handle, "HTTP port: %d\n", status.http_Port);
            }
            else if (status.mode == QAPI_NET_HTTPSVR_HTTPS_E)
            {
                QCLI_Printf(qcli_net_handle, "HTTPS port: %d\n", status.https_Port);
            }
            else if (status.mode == QAPI_NET_HTTPSVR_HTTP_AND_HTTPS_E)
            {
                QCLI_Printf(qcli_net_handle, "HTTP port: %d  HTTPS port: %d\n",
                            status.http_Port, status.https_Port);
            }

            QCLI_Printf(qcli_net_handle, "Landing page: %s  Root path: %s\n",
                    status.root_Index_Page, status.root_Path);

            QCLI_Printf(qcli_net_handle, "TX bufsize: %u  RX bufsize: %u\n",
                    status.txbufsize, status.rxbufsize);

            QCLI_Printf(qcli_net_handle, "\n");
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
            QCLI_Printf(qcli_net_handle, "\"%s\" is not supported.\n", Parameter_List[1].String_Value);
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
            QCLI_Printf(qcli_net_handle, "\"%s\" is not supported.\n", Parameter_List[2].String_Value);
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
                        QCLI_Printf(qcli_net_handle, "Unknown option: %s\n", Parameter_List[i].String_Value);
                        return QCLI_STATUS_ERROR_E;
                    }
                }
                else
                {
                    QCLI_Printf(qcli_net_handle, "Unknown option: %s\n", Parameter_List[i].String_Value);
                    return QCLI_STATUS_ERROR_E;
                }

                if (i == Parameter_Count)
                {
                    QCLI_Printf(qcli_net_handle, "What is value of %s?\n", Parameter_List[i-1].String_Value);
                    return QCLI_STATUS_ERROR_E;
                }
            } /* for */
        }

        cfg.webfiles_Setup = webfiles_setup;

        /* do it !! */
        if (qapi_Net_HTTPs_Init(&cfg) != QAPI_OK)
        {
            QCLI_Printf(qcli_net_handle, "Failed to Init HTTP server\n");
            return QCLI_STATUS_ERROR_E;
        }
    } /* cmd == "init" */
#ifdef CONFIG_NET_SSL_DEMO
    /*         [0]       [1]      [2]    [3]    [4]       [5]    [6] 
     * httpsvr sslconfig protocol TLS1.2 domain yahoo.com alert  1
     */
    else if (strncmp(cmd, "sslconfig", 3) == 0 || strcmp(cmd, "sslcfg") == 0)
    {
        if (Parameter_Count < 2)
        {
            QCLI_Printf(qcli_net_handle, "What are SSL parameters?\n");
            httpsvr_help();
            return QCLI_STATUS_ERROR_E;
        }

        if (httpsvr_sslcfg == NULL)
        {
            httpsvr_sslcfg = malloc(sizeof(qapi_Net_SSL_Config_t));
            if (httpsvr_sslcfg == NULL)
            {
                QCLI_Printf(qcli_net_handle, "Allocation failure\n");
                return QCLI_STATUS_ERROR_E;
            }
            memset(httpsvr_sslcfg, 0, sizeof(qapi_Net_SSL_Config_t));
        }
        else
        {
            /* free previous ssl parameters */
            ssl_free_config_parameters(httpsvr_sslcfg);
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
#endif
    /* httpsvr  start */
    else if (strncmp(cmd, "start", 3) == 0)
    {
#ifdef CONFIG_NET_SSL_DEMO
        if (httpsvr_sslcfg)
        {
            e = qapi_Net_HTTPs_Set_SSL_Config(httpsvr_sslcfg);
            if (e != QAPI_OK)
            {
                QCLI_Printf(qcli_net_handle, "SSL config failed\n");
            }
        }
#endif
        if (qapi_Net_HTTPs_Start() != QAPI_OK)
        {
            QCLI_Printf(qcli_net_handle, "Have you done 'httpsvr init' yet?\n");
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
#ifdef CONFIG_NET_SSL_DEMO
        if (httpsvr_sslcfg)
        {
            ssl_free_config_parameters(httpsvr_sslcfg);
            free(httpsvr_sslcfg);
            httpsvr_sslcfg = NULL;
        }
#endif
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
                QCLI_Printf(qcli_net_handle, "Failed to register \"%s\"\n", Parameter_List[i].String_Value);
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
                QCLI_Printf(qcli_net_handle, "Failed to set buffer size\n");
                return QCLI_STATUS_ERROR_E;
            }
        }
        else
        {
            QCLI_Printf(qcli_net_handle, "Invalid size\n");
            return QCLI_STATUS_ERROR_E;
        }
    }

    else
    {
        QCLI_Printf(qcli_net_handle, "\"%s\" is not supported.\n", cmd);
        return QCLI_STATUS_ERROR_E;
    }

    return QCLI_STATUS_SUCCESS_E;
}
#endif

#ifdef CONFIG_NET_PING_DEMO
/*****************************************************************************
 *      [0]         [1]  [2]   [3] [4]
 * Ping 192.168.1.1 [-c  100] [-s  1500]
 *****************************************************************************/
static QCLI_Command_Status_t ping(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
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
                    QCLI_Printf(qcli_net_handle, "Size should be <= %d\n", CFG_PACKET_SIZE_MAX_TX);
                    return QCLI_STATUS_ERROR_E;
                }
            }
        }
    } /* for loop */

    host = (char *)Parameter_List[0].String_Value;
    if (inet_pton(AF_INET, host, &addr.a) != 0)   /* not in xxx.xxx.xxx.xxx format */
    {
#ifdef CONFIG_NET_DNSC_DEMO
        if (strlen(host) > MAX_FQDN_LEN)
        {
            QCLI_Printf(qcli_net_handle, "host name cannot be more than %u Bytes\n", MAX_FQDN_LEN);
            return QCLI_STATUS_ERROR_E;
        }

        addr.type = AF_INET;

        if (qapi_Net_DNSc_Reshost(host, &addr) != 0)
        {
            QCLI_Printf(qcli_net_handle, "Cannot resolve %s\n", host);
            return QCLI_STATUS_ERROR_E;
        }
        else
        {
            QCLI_Printf(qcli_net_handle, "Ping %s (%s):\n",
                    host,
                    inet_ntop(AF_INET, &addr.a, ip_str, sizeof(ip_str)));
        }
#else
        QCLI_Printf(qcli_net_handle, "DNS client demo disabled, use IP address\n");
        return QCLI_STATUS_ERROR_E;
#endif
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
            QCLI_Printf(qcli_net_handle, "%d bytes from %s: seq=%d time=%u ms\n",
                    size, inet_ntop(AF_INET, &addr.a, ip_str, sizeof(ip_str)), i+1, ms);
            if (i != count - 1)
            {
                app_msec_delay(980);
            }
        }
        else if (e == QAPI_NET_ERR_INVALID_IPADDR)
        {
            QCLI_Printf(qcli_net_handle, "Invalid IP address\n");
            return QCLI_STATUS_ERROR_E;
        }
        else
        {
            QCLI_Printf(qcli_net_handle, "Request timed out\n");
        }
    }

    return QCLI_STATUS_SUCCESS_E;
}
#endif

#ifdef CONFIG_NET_DHCPV4C_DEMO
/*****************************************************************************
 * addr mask and gw are in network order.
 *****************************************************************************/
static int32_t ipconfig_dhcpc_success_cb(uint32_t addr, uint32_t mask, uint32_t gw)
{
    char ip_str[20];
    char mask_str[20];
    char gw_str[20];

    QCLI_Printf(qcli_net_handle, "DHCPv4c: IP=%s  Subnet Mask=%s  Gateway=%s\n",
            inet_ntop(AF_INET, &addr, ip_str, sizeof(ip_str)),
            inet_ntop(AF_INET, &mask, mask_str, sizeof(mask_str)),
            inet_ntop(AF_INET, &gw, gw_str, sizeof(gw_str)));

    return 0;
}

/*****************************************************************************
 *         [0]   [1]
 * Dhcpv4c wlan0 new|release
 *****************************************************************************/
static QCLI_Command_Status_t dhcpv4c(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
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
        QCLI_Printf(qcli_net_handle, "ERROR: %s does not exist or is DOWN.\n", interface_name);
        return QCLI_STATUS_ERROR_E;
    }

    cmd = Parameter_List[1].String_Value;

    if (Parameter_Count == 1 || strncmp(cmd, "new", 3) == 0)
    {
        if (qapi_Net_DHCPv4c_Register_Success_Callback(interface_name, ipconfig_dhcpc_success_cb) != 0 ||
            qapi_Net_IPv4_Config(interface_name, QAPI_NET_IPV4CFG_DHCP_IP_E, NULL, NULL, NULL) != 0)
        {
            QCLI_Printf(qcli_net_handle, "ERROR: DHCPv4 new failed\n");
            return QCLI_STATUS_ERROR_E;
        }
    }
    else if (strncmp(cmd, "release", 3) == 0)
    {
        if (qapi_Net_DHCPv4c_Release(interface_name) != 0)
        {
            QCLI_Printf(qcli_net_handle, "ERROR: DHCPv4 release failed\n");
            return QCLI_STATUS_ERROR_E;
        }
    }

    return QCLI_STATUS_SUCCESS_E;
}
#endif

#ifdef CONFIG_NET_AUTOIP_DEMO
/*****************************************************************************
 *          [0]
 * autoipv4 wlan0
 *****************************************************************************/
static QCLI_Command_Status_t autoip(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    char *interface_name;

    if (Parameter_Count < 1 || Parameter_List == NULL)
    {
        return QCLI_STATUS_USAGE_E;
    }

    interface_name = Parameter_List[0].String_Value;

    if (qapi_Net_IPv4_Config(interface_name, QAPI_NET_IPV4CFG_AUTO_IP_E, NULL, NULL, NULL) != 0)
    {
        QCLI_Printf(qcli_net_handle, "Auto IPv4 failed\n");
        return QCLI_STATUS_ERROR_E;
    }

    return QCLI_STATUS_SUCCESS_E;
}
#endif

/*****************************************************************************
 *****************************************************************************/
static int32_t display_interface_info(char *interface_name, qbool_t if_is_up)
{
    ip6_addr v6Global, v6GlobalExtd, v6LinkLocal, v6DefGw;
    uint32_t LinkPrefix, GlobalPrefix, DefGwPrefix, GlobalExtdPrefix;
    qapi_Net_DNS_Server_List_t svr_list;
    char msg_ipv4_target_error[] = "Operation failed";

    QCLI_Printf(qcli_net_handle, "%s: %s\n", interface_name,
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
            QCLI_Printf(qcli_net_handle, " Phy Address: %02X", physical_address[0]);
            for ( i = 1; i < physical_address_length; i++ )
            {
                QCLI_Printf(qcli_net_handle, ":%02X", physical_address[i]);
            }
            QCLI_Printf(qcli_net_handle, "\n");
        }

        /* IPv4 */
        if ((err = qapi_Net_IPv4_Config(interface_name, QAPI_NET_IPV4CFG_QUERY_E, &addr, &mask, &gw)) == 0)
        {
            char ip_str[20], mask_str[20], gw_str[20];

            QCLI_Printf(qcli_net_handle, " IPv4: %s  Subnet Mask: %s  Default Gateway: %s\n",
                        inet_ntop(AF_INET, &addr, ip_str, sizeof(ip_str)),
                        inet_ntop(AF_INET, &mask, mask_str, sizeof(mask_str)),
                        inet_ntop(AF_INET, &gw, gw_str, sizeof(gw_str)));
        }

        if (err == QAPI_NET_ERR_SOCKET_CMD_TIME_OUT)
        {
            QCLI_Printf(qcli_net_handle, "%s\n", msg_ipv4_target_error);
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
                    QCLI_Printf(qcli_net_handle, " IPv6 Link-local Address ..... : %s/%d\n", ip_str, LinkPrefix);
                else
                    QCLI_Printf(qcli_net_handle, " IPv6 Link-local Address ..... : %s\n", ip_str);
            }

            if (inet_ntop(AF_INET6, &v6Global, ip_str, sizeof(ip_str)) != NULL)
            {
                if (GlobalPrefix)
                    QCLI_Printf(qcli_net_handle, " IPv6 Global Address ......... : %s/%d\n", ip_str, GlobalPrefix);
                else
                    QCLI_Printf(qcli_net_handle, " IPv6 Global Address ......... : %s\n", ip_str);
            }

            if (inet_ntop(AF_INET6, &v6DefGw, ip_str, sizeof(ip_str)) != NULL)
            {
                if (DefGwPrefix)
                    QCLI_Printf(qcli_net_handle, " IPv6 Default Gateway  ....... : %s/%d\n", ip_str, DefGwPrefix);
                else
                    QCLI_Printf(qcli_net_handle, " IPv6 Default Gateway  ....... : %s\n", ip_str);

            }

            if (inet_ntop(AF_INET6, &v6GlobalExtd, ip_str, sizeof(ip_str)) != NULL)
            {
                if (GlobalExtdPrefix)
                    QCLI_Printf(qcli_net_handle, " IPv6 Global Address 2 ....... : %s/%d\n", ip_str, GlobalExtdPrefix);
                else
                    QCLI_Printf(qcli_net_handle, " IPv6 Global Address 2 ....... : %s\n", ip_str);
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
                QCLI_Printf(qcli_net_handle, " DNS Server: %s\n",
                            inet_ntop(svr_list.svr[i].type, &svr_list.svr[i].a, ip_str, sizeof(ip_str)));
            }
        }
    }

    return 0;
}

/*****************************************************************************
 *          [0]   [1]           [2]         [3]
 * ifconfig
 * ifconfig lo0
 * ifconfig wlan0 192.168.1.100 255.255.255 192.168.1.1     //ifname must be wlan0 or wlan1 or wlbr0
 *****************************************************************************/
static QCLI_Command_Status_t ifconfig(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
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
                /* If no interface, we want to display cmd syntax */
                return QCLI_STATUS_USAGE_E;
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
                QCLI_Printf(qcli_net_handle, "%s does not exist\n", interface_name);
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
                QCLI_Printf(qcli_net_handle, "Not a WLAN interface\n");
                return QCLI_STATUS_ERROR_E;
            }

            /* interface's IP address */
            if (inet_pton(AF_INET, Parameter_List[1].String_Value, &addr) != 0)
            {
                QCLI_Printf(qcli_net_handle, "Invalid IP address\n");
                return QCLI_STATUS_ERROR_E;
            }

            /* subnet mask */
            if (inet_pton(AF_INET, Parameter_List[2].String_Value, &mask) != 0)
            {
                QCLI_Printf(qcli_net_handle, "Invalid subnet mask\n");
                return QCLI_STATUS_ERROR_E;
            }

            /* default gateway */
            if (Parameter_Count == 4)
            {
                if (inet_pton(AF_INET, Parameter_List[3].String_Value, &gw) != 0)
                {
                    QCLI_Printf(qcli_net_handle, "Invalid gateway address\n");
                    return QCLI_STATUS_ERROR_E;
                }

                gw_addr = &gw;
            }

            e = qapi_Net_IPv4_Config(interface_name, QAPI_NET_IPV4CFG_STATIC_IP_E, &addr, &mask, gw_addr);
            if (e != 0)
            {
                if (e == QAPI_NET_ERR_INVALID_IPADDR)
                {
                    QCLI_Printf(qcli_net_handle, "Invalid IP address\n");
                }
                else
                {
                    QCLI_Printf(qcli_net_handle, "Static IPv4 configure failed\n");
                }
                return QCLI_STATUS_ERROR_E;
            }
            break;

        default:
            return QCLI_STATUS_USAGE_E;
    }

    return QCLI_STATUS_SUCCESS_E;
}

#ifdef CONFIG_NET_DHCPV4C_DEMO
/*****************************************************************************
 *
 *****************************************************************************/
static int32_t ipconfig_dhcps_success_cb(uint8_t *macaddr, uint32_t ipaddr)
{
    if (macaddr != NULL)
    {
        char ip_str[20];

        QCLI_Printf(qcli_net_handle, "DHCPv4s: Client IP=%s  Client MAC=%02x:%02x:%02x:%02x:%02x:%02x\n",
                inet_ntop(AF_INET, &ipaddr, ip_str, sizeof(ip_str)),
                macaddr[0], macaddr[1], macaddr[2], macaddr[3], macaddr[4], macaddr[5]);
    }

    return 0;
}

/*****************************************************************************
 *           [0]   [1] [2]           [3]           [4]
 * Dhcpv4s wlan0  pool 192.168.1.10  192.168.1.50  3600
 *****************************************************************************/
static QCLI_Command_Status_t dhcpv4s(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
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
            QCLI_Printf(qcli_net_handle, "Invalid lease time specified, setting to default value\n");
        }
        if (Parameter_Count == 5 && Parameter_List[4].Integer_Is_Valid)
        {
            leasetime = Parameter_List[4].Integer_Value;
        }

        e = inet_pton(AF_INET, start_ip_addr_string, &startaddr);
        if (e != 0)
        {
            QCLI_Printf(qcli_net_handle, "Invalid start address\n");
            return QCLI_STATUS_ERROR_E;
        }
        startaddr = ntohl(startaddr);

        e = inet_pton(AF_INET, end_ip_addr_string, &endaddr);
        if (e != 0)
        {
            QCLI_Printf(qcli_net_handle, "Invalid end address\n");
            return QCLI_STATUS_ERROR_E;
        }
        endaddr = ntohl(endaddr);

        if (startaddr >= endaddr)
        {
            QCLI_Printf(qcli_net_handle, "Start address must be less than end address\n");
            return QCLI_STATUS_ERROR_E;
        }

        /* Get interface's IP address */
        e = qapi_Net_IPv4_Config(interface_name, QAPI_NET_IPV4CFG_QUERY_E, &addr, &mask, NULL);
        if (e != 0)
        {
            QCLI_Printf(qcli_net_handle, "Getting interface address failed\n");
            return QCLI_STATUS_ERROR_E;
        }
        addr = ntohl(addr);
        mask = ntohl(mask);

        if ((addr & mask) != (startaddr & mask) ||
            (addr & mask) != (endaddr & mask))
        {
            QCLI_Printf(qcli_net_handle, "Pool IP and interface IP should be in the same subnet\n");
            return QCLI_STATUS_ERROR_E;
        }

        if ((addr >= startaddr) && (addr <= endaddr))
        {
            QCLI_Printf(qcli_net_handle, "Please configure pool beyond interface IP address\n");
            return QCLI_STATUS_ERROR_E;
        }

        if (qapi_Net_DHCPv4s_Register_Success_Callback(interface_name, ipconfig_dhcps_success_cb) != 0 ||
            qapi_Net_DHCPv4s_Set_Pool(interface_name, htonl(startaddr), htonl(endaddr), leasetime) != 0)
        {
            QCLI_Printf(qcli_net_handle, "Config DHCP pool failed\n");
            return QCLI_STATUS_ERROR_E;
        }
    }
    else
    {
        QCLI_Printf(qcli_net_handle, "Invalid command: %s\n", cmd);
        return QCLI_STATUS_ERROR_E;
    }

    return QCLI_STATUS_SUCCESS_E;
}
#endif

#ifdef CONFIG_NET_DHCPV6C_DEMO
/*****************************************************************************
 *****************************************************************************/
static void DHCPv6c_New_Lease_Complete_CB(
    void * cb_ctxt,
    const char * interface_name,
    const ip6_addr * ip6addr
    )
{
    char ip_str[48];
    QCLI_Printf(qcli_net_handle, "\nDHCPv6c_New_Lease_Complete_CB: interface=%s, ipv6=%s\n",
                interface_name,
                inet_ntop(AF_INET6, ip6addr, ip_str, sizeof(ip_str)));
}

/*****************************************************************************
 *****************************************************************************/
static void DHCPv6c_Release_Lease_Complete_CB(
    void * cb_ctxt,
    const char * interface_name,
    const ip6_addr * ip6addr
    )
{
    char ip_str[48];
    QCLI_Printf(qcli_net_handle, "\nDHCPv6c_Release_Lease_Complete_CB: interface=%s, ipv6=%s\n",
                interface_name,
                inet_ntop(AF_INET6, ip6addr, ip_str, sizeof(ip_str)));
}

/*****************************************************************************
 *****************************************************************************/
static void DHCPv6c_New_Prefix_Lease_Complete_CB(
    void * cb_ctxt,
    const char * interface_name,
    const ip6_addr * prefix_base,
    const uint8_t prefix_length
    )
{
    char ip_str[48];
    QCLI_Printf(qcli_net_handle, "\nDHCPv6c_New_Prefix_Lease_Complete_CB: interface=%s, prefix=%s/%d\n",
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
    QCLI_Printf(qcli_net_handle, "\nDHCPv6c_Release_Prefix_Lease_Complete_CB: interface=%s, prefix=%s/%d\n",
                interface_name,
                inet_ntop(AF_INET6, prefix_base, ip_str, sizeof(ip_str)),
                prefix_length);
}
#endif

#ifdef CONFIG_NET_PING_DEMO
/*****************************************************************************
 *          [0]                    [1] [2]    [3] [4]     [5] [6]
 * Ping6 FE80::865D:D7FF:FE40:19C8 [-c  100] [-s  1500]  [-I  wlan0]
 *****************************************************************************/
static QCLI_Command_Status_t ping6(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
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
                    QCLI_Printf(qcli_net_handle, "Size should be <= %d\n", CFG_PACKET_SIZE_MAX_TX);
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
#ifdef CONFIG_NET_DNSC_DEMO
        if (strlen(host) > MAX_FQDN_LEN)
        {
            QCLI_Printf(qcli_net_handle, "host name cannot be more then %u Bytes\n", MAX_FQDN_LEN);
            return QCLI_STATUS_ERROR_E;
        }

        addr.type = AF_INET6;
        if (qapi_Net_DNSc_Reshost(host, &addr) != 0)
        {
            QCLI_Printf(qcli_net_handle, "Invalid IP address\n");
            return QCLI_STATUS_ERROR_E;
        }
        else
        {
            QCLI_Printf(qcli_net_handle, "Ping %s (%s):\n",
                    host,
                    inet_ntop(AF_INET6, &addr.a, ip_str, sizeof(ip_str)));
        }
#else
        QCLI_Printf(qcli_net_handle, "DNS client demo disabled, use IP address\n");
        return QCLI_STATUS_ERROR_E;
#endif
    }

    ip6addr = (uint8_t *)&addr.a;
    if (QAPI_IS_IPV6_MULTICAST(ip6addr) ||
        QAPI_IS_IPV6_LINK_LOCAL(ip6addr))
    {
        is_ifname_specification_needed = 1;
    }

    if (is_ifname_specification_needed && ifname == NULL)
    {
        QCLI_Printf(qcli_net_handle, "Error: The specified IPv6 address is either multicast or link-local, please specify source interface name using the -I option\n");
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
            QCLI_Printf(qcli_net_handle, "%d bytes from %s: seq=%d time=%u ms\n",
                    size, inet_ntop(AF_INET6, &addr.a, ip_str, sizeof(ip_str)), i+1, ms);
        }
        else if (e == 1)
        {
            QCLI_Printf(qcli_net_handle, "Request timed out\n");
        }
        else
        {
            QCLI_Printf(qcli_net_handle, "Network is unreachable\n");
            return QCLI_STATUS_ERROR_E;
        }

        #define PING6_DELAY_DURATION_IN_MS 1000

        if ( i < count - 1 ) {
             app_msec_delay(PING6_DELAY_DURATION_IN_MS);
        }
    }

    return QCLI_STATUS_SUCCESS_E;
}
#endif

#ifdef CONFIG_NET_DHCPV6C_DEMO
/*****************************************************************************
 *         [0]   [1]
 * Dhcpv6c wlan0 enable|disable|new|confirm|release
 *****************************************************************************/
static QCLI_Command_Status_t dhcpv6c(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
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
        char * ifname_to_obtain_ia_for = ifname;
        if ( Parameter_Count > 2 ) {
            ifname_to_obtain_ia_for = Parameter_List[2].String_Value;
        }
        int are_same_interfaces = (strcmp(ifname, ifname_to_obtain_ia_for) == 0);
        if ( !are_same_interfaces )
        {
            int status_code =
                qapi_Net_DHCPv6c_Register_New_Prefix_Lease_Complete_Callback(
                    ifname_to_obtain_ia_for,
                    DHCPv6c_New_Prefix_Lease_Complete_CB,
                    NULL
                    );
            if ( 0 != status_code ) {
                goto fail;
            }
            status_code =
                qapi_Net_DHCPv6c_Register_Release_Prefix_Lease_Complete_Callback(
                    ifname_to_obtain_ia_for,
                    DHCPv6c_Release_Prefix_Lease_Complete_CB,
                    NULL
                    );
            if ( 0 != status_code ) {
                goto fail;
            }
        }
        else {
            int status_code =
                qapi_Net_DHCPv6c_Register_New_IPv6_Lease_Complete_Callback(
                    ifname_to_obtain_ia_for,
                    DHCPv6c_New_Lease_Complete_CB,
                    NULL
                    );
            if ( 0 != status_code ) {
                goto fail;
            }
            status_code =
                qapi_Net_DHCPv6c_Register_Release_IPv6_Lease_Complete_Callback(
                    ifname_to_obtain_ia_for,
                    DHCPv6c_Release_Lease_Complete_CB,
                    NULL
                    );
            if ( 0 != status_code ) {
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
        char * ifname_to_obtain_ia_for = ifname;
        if ( Parameter_Count > 2 ) {
            ifname_to_obtain_ia_for = Parameter_List[2].String_Value;
        }
        if (qapi_Net_DHCPv6c_Release_Lease(ifname, ifname_to_obtain_ia_for) != 0)
        {
            goto fail;
        }
    }
    else if (strncmp(cmd, "confirm", 3) == 0)
    {
        char * ifname_to_obtain_ia_for = ifname;
        if ( Parameter_Count > 2 ) {
            ifname_to_obtain_ia_for = Parameter_List[2].String_Value;
        }
        if (qapi_Net_DHCPv6c_Confirm_Lease(ifname, ifname_to_obtain_ia_for) != 0)
        {
            goto fail;
        }
    }
    else
    {
        QCLI_Printf(qcli_net_handle, "Invalid command: %s\n", cmd);
        return QCLI_STATUS_ERROR_E;
    }

    return QCLI_STATUS_SUCCESS_E;

fail:
    QCLI_Printf(qcli_net_handle, "%s failed\n", cmd);
    return QCLI_STATUS_ERROR_E;
}

#endif

#ifdef CONFIG_NET_SNTPC_DEMO
/*****************************************************************************
 *         [0]        [1]                                      [2]
 * Sntpc
 * Sntpc  start
 * Sntpc  stop
 * Sntpc  disable
 * Sntpc  add         10.234.20.15
 * Sntpc  add         na.qualcomm.com                           0
 * Sntpc  add         2002:c023:9c17:303:edd3:6b74:5915:dbe7    1
 * Sntpc  add         FE80::865D:D7FF:FE40:3498%wlan1           1
 * Sntpc  del         1
 * Sntpc  utc
 *****************************************************************************/
static QCLI_Command_Status_t sntpc(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
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
        QCLI_Printf(qcli_net_handle, "SNTP client is %s.\n", started ? "started" : "stopped");
        if (qapi_Net_SNTPc_Get_Server_List(&svr_list) == 0)
        {
            for (i = 0; i < QAPI_NET_SNTP_SERVER_MAX; ++i)
            {
                QCLI_Printf(qcli_net_handle, "%d: ", i);
                QCLI_Printf(qcli_net_handle, "%s    ", svr_list.svr[i].name[0] != '\0' ? svr_list.svr[i].name : "****");
                QCLI_Printf(qcli_net_handle, "%s  ", svr_list.svr[i].addr[0] != '\0' ? svr_list.svr[i].addr : "****");
                if (svr_list.svr[i].status == QAPI_NET_SNTP_SERVER_STATUS_KOD)
                {
                    QCLI_Printf(qcli_net_handle, "KOD");
                }
                else if (svr_list.svr[i].status == QAPI_NET_SNTP_SERVER_STATUS_NORESP)
                {
                    QCLI_Printf(qcli_net_handle, "NORESP");
                }
                QCLI_Printf(qcli_net_handle, "\n");
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
            QCLI_Printf(qcli_net_handle, "%s\n", msg_sntp_not_started);
            return QCLI_STATUS_ERROR_E;
        }

        svr = Parameter_List[1].String_Value;

        if (strlen(svr) > 64)
        {
            QCLI_Printf(qcli_net_handle, "%s\n", msg_name_too_long);
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
            QCLI_Printf(qcli_net_handle, "%s\n", msg_invalid_id);
            return QCLI_STATUS_ERROR_E;
        }

        e = qapi_Net_SNTPc_Add_Server(svr, id);
        if (e == QAPI_NET_ERR_INVALID_IPADDR)
        {
            QCLI_Printf(qcli_net_handle, "Invalid server address\n");
        }
        else if (e == QAPI_NET_ERR_CANNOT_GET_SCOPEID)
        {
            QCLI_Printf(qcli_net_handle, "Failed to get scope id\n");
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
            QCLI_Printf(qcli_net_handle, "%s\n", msg_sntp_not_started);
            return QCLI_STATUS_ERROR_E;
        }

        if (Parameter_List[1].Integer_Is_Valid)
        {
            id = Parameter_List[1].Integer_Value;
        }
        else
        {
            QCLI_Printf(qcli_net_handle, "%s\n", msg_invalid_id);
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
            QCLI_Printf(qcli_net_handle, "%s %u, %02u %s %02u:%02u:%02u UTC\n",
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
        QCLI_Printf(qcli_net_handle, "Invalid subcommand: %s\n", cmd);
        goto parm_error;
    }

    if (e)
    {
        QCLI_Printf(qcli_net_handle, "%s failed\n", cmd);
        return QCLI_STATUS_ERROR_E;
    }
    else
    {
        return QCLI_STATUS_SUCCESS_E;
    }

parm_error:
    return QCLI_STATUS_USAGE_E;
}
#endif

#ifdef CONFIG_NET_DNSC_DEMO
/*****************************************************************************
 *       [0]     [1]                            [2]
 * Dnsc
 * Dnsc start
 * Dnsc stop
 * Dnsc disable
 * Dnsc resolve  sclcelinux1.qualcomm.com
 * Dnsc resolve  sclcelinux1.qualcomm.com       v4
 * Dnsc resolve  sclcelinux1.qualcomm.com       v6
 * Dnsc add      192.168.1.30
 * Dnsc add      192.168.1.300   1
 * Dnsc add      FE80::865D:D7FF:FE40:3498%wlan1
 * Dnsc del      1
 *
 *       [0]            [1]                      [2]
 * Dnsc gethostbyname   sclcelinux1.qualcomm.com
 * Dnsc gethostbyname2  sclcelinux1.qualcomm.com [v4]
 * Dnsc gethostbyname2  sclcelinux1.qualcomm.com [v6]
 *****************************************************************************/
static QCLI_Command_Status_t dnsc(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
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
        QCLI_Printf(qcli_net_handle, "DNS client is %s.\n", started ? "started" : "stopped");
        e = qapi_Net_DNSc_Get_Server_List(&svr_list);
        if (e == 0)
        {
            for (i = 0; i < QAPI_NET_DNS_SERVER_MAX; ++i)
            {
                if (svr_list.svr[i].type == AF_UNSPEC)
                {
                    QCLI_Printf(qcli_net_handle, "%d: ****\n", i);
                }
                else
                {
                    QCLI_Printf(qcli_net_handle, "%d: %s\n", i,
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
            QCLI_Printf(qcli_net_handle, "address cannot be more then 48 bytes\n");
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
            QCLI_Printf(qcli_net_handle, "%s\n", msg_invalid_id);
            return QCLI_STATUS_ERROR_E;
        }

        e = qapi_Net_DNSc_Add_Server(svr, id);
        if (e == QAPI_NET_ERR_INVALID_IPADDR)
        {
            QCLI_Printf(qcli_net_handle, "Invalid server address\n");
        }
        else if (e == QAPI_NET_ERR_CANNOT_GET_SCOPEID)
        {
            QCLI_Printf(qcli_net_handle, "Failed to get scope id\n");
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
            QCLI_Printf(qcli_net_handle, "%s\n", msg_invalid_id);
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
            QCLI_Printf(qcli_net_handle, "DNS client is not started yet.\n");
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
            QCLI_Printf(qcli_net_handle, "Resolve to \"v4\" or \"v6\"?\n");
            goto parm_error;
        }

        e = qapi_Net_DNSc_Reshost(svr, &ipaddr);
        if (e)
        {
            QCLI_Printf(qcli_net_handle, "Unable to resolve %s\n", svr);
        }
        else
        {
            QCLI_Printf(qcli_net_handle, "\n%s --> %s\n", svr, inet_ntop(ipaddr.type, &ipaddr.a, ip_str, sizeof(ip_str)));
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
            QCLI_Printf(qcli_net_handle, "DNS client is not started yet.\n");
            return QCLI_STATUS_ERROR_E;
        }

        svr = Parameter_List[1].String_Value;
        he = gethostbyname(svr);
        if (he == NULL)
        {
            QCLI_Printf(qcli_net_handle, "Unable to resolve %s\n", svr);
        }
        else
        {
            QCLI_Printf(qcli_net_handle, "\n%s -->", he->h_name);
            while (*(he->h_addr_list) != NULL)
            {
                QCLI_Printf(qcli_net_handle, " %s", inet_ntop(AF_INET, *(he->h_addr_list), ip_str, sizeof(ip_str)));
                ++(he->h_addr_list);
            }
            QCLI_Printf(qcli_net_handle, "\n");
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
            QCLI_Printf(qcli_net_handle, "DNS client is not started yet.\n");
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
            QCLI_Printf(qcli_net_handle, "Resolve to \"v4\" or \"v6\"?\n");
            goto parm_error;
        }

        svr = Parameter_List[1].String_Value;
        he = gethostbyname2(svr, af);
        if (he == NULL)
        {
            QCLI_Printf(qcli_net_handle, "Unable to resolve %s to IP%s address\n",
                    svr, (af == AF_INET) ? "v4" : "v6");
        }
        else
        {
            QCLI_Printf(qcli_net_handle, "\n%s -->", he->h_name);
            while (*(he->h_addr_list) != NULL)
            {
                QCLI_Printf(qcli_net_handle, " %s", inet_ntop(af, *(he->h_addr_list), ip_str, sizeof(ip_str)));
                ++(he->h_addr_list);
            }
            QCLI_Printf(qcli_net_handle, "\n");
            e = 0;
        }
    }

    else
    {
        QCLI_Printf(qcli_net_handle, "Invalid subcommand: %s\n", cmd);
        goto parm_error;
    }

    if (e)
    {
        QCLI_Printf(qcli_net_handle, "%s failed\n", cmd);
        return QCLI_STATUS_ERROR_E;
    }
    else
    {
        return QCLI_STATUS_SUCCESS_E;
    }

parm_error:
    return QCLI_STATUS_USAGE_E;
}
#endif

#ifdef CONFIG_NET_WLAN_BRIDGE_DEMO
/*****************************************************************************
 *           [0]                   [1]
 * bridge
 * bridge enable
 * bridge disable
 * bridge mac_entry_age <value>
 * bridge showmacs
 *****************************************************************************/
static QCLI_Command_Status_t bridge(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
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
			  QCLI_Printf(qcli_net_handle, "\nNo Entries\n");
		   }
		   else {
	           void *buf = (void *)ptr;
	           QCLI_Printf(qcli_net_handle, "\nDevId\tBridged MAC address\tTTL\n");
	           while(count){
	              QCLI_Printf(qcli_net_handle,"%02d\t\t%02X:%02X:%02X:%02X:%02X:%02X\t%d\n", ptr->dev_ID,
	                       ptr->addr[0], ptr->addr[1],ptr->addr[2],ptr->addr[3],ptr->addr[4],ptr->addr[5], ptr->age);
	              count--;
	              ptr++;
	           }
	           free(buf);
		   }
       }else{
           QCLI_Printf(qcli_net_handle, "Invalid command, use help\n");
       }
    }
    if (e)
    {
    	switch(e)
    	{
    	case QAPI_ERR_NOT_SUPPORTED:
    		QCLI_Printf(qcli_net_handle, "Operation not supported\n", cmd);
    		break;

    	case QAPI_ERR_NO_MEMORY:
    		QCLI_Printf(qcli_net_handle, "Out of memory\n", cmd);
    		break;

    	default:
    		QCLI_Printf(qcli_net_handle, "Command failed. Run ifconfig and verify wlan0 and wlan1 are up\n", cmd);

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
#endif

#ifdef CONFIG_NET_ROUTE_DEMO
/*****************************************************************************
 *                     [0]             [1]         [2]        [3]               [4]
 * prefix <interfaceName> <v6addr> <prefixlen> <prefix_lifetime> <valid_lifetime>
 ****************************************************************************/
static QCLI_Command_Status_t set_ipv6_router_prefix(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    uint8_t v6addr[16];
    int retval;
    int prefixlen = 0;
    int prefix_lifetime = 0;
    int valid_lifetime = 0;
    char *interface_name;

    if (Parameter_Count < 5)
    {
        return QCLI_STATUS_USAGE_E;
    }

    retval = inet_pton(AF_INET6, Parameter_List[1].String_Value, v6addr);
    if (retval != 0)
    {
        QCLI_Printf(qcli_net_handle, "Invalid IPv6 prefix\n");
        return QCLI_STATUS_ERROR_E;
    }

    interface_name  = Parameter_List[0].String_Value;
    prefixlen       = Parameter_List[2].Integer_Value;
    prefix_lifetime = Parameter_List[3].Integer_Value;
    valid_lifetime  = Parameter_List[4].Integer_Value;
    retval = qapi_Net_IPv6_Config_Router_Prefix(interface_name, v6addr, prefixlen, prefix_lifetime, valid_lifetime);
    if (retval != 0)
    {
        QCLI_Printf(qcli_net_handle, "Setting router prefix failed\n");
        return QCLI_STATUS_ERROR_E;
    }

    return QCLI_STATUS_SUCCESS_E;
}

/*****************************************************************************
 *        [0]  [1]    [2]               [3]
 * route6 add <dest> <next_hop>         <interfaceName> 
 * route6 del <dest> <interfaceName>
 * route6 show
 ****************************************************************************/
static void route6_help(void)
{
    QCLI_Printf(qcli_net_handle, "route6 add <dest> <next_hop> <interface>\n");
    QCLI_Printf(qcli_net_handle, "route6 del <dest> <interface>\n");
    QCLI_Printf(qcli_net_handle, "route6 show\n");
    QCLI_Printf(qcli_net_handle, "Examples:\n");
    QCLI_Printf(qcli_net_handle, " route6 add fda8:6c3:ce53:a890::/64 fda8:6c3:ce53:a890::3e wlan1\n");
    QCLI_Printf(qcli_net_handle, " route6 del fda8:6c3:ce53:a890::/64 wlan1\n");
}

static QCLI_Command_Status_t route6(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    char *cmd, *cp, *ifname;
    ip6_addr dest, nexthop;
    uint32_t prefixlen = 0;
	qapi_Status_t rc = QAPI_ERROR;

    if (Parameter_Count < 1)
    {
        route6_help();
        return QCLI_STATUS_SUCCESS_E;
    }

    cmd = Parameter_List[0].String_Value;

    /*        [0]  [1]                     [2]                   [3]
     * route6 add fda8:6c3:ce53:a890::/64 fda8:6c3:ce53:a890::3e wlan1
     */
    if (strcmp(cmd, "add") == 0)
    {
        if (Parameter_Count < 4)
        {
            route6_help();
            goto end;
        }

        cp = strchr(Parameter_List[1].String_Value, '/');
        if (cp == NULL)
        {
            prefixlen = 128;
        }
        else
        {
            *cp++ = '\0';
            prefixlen = strtol(cp, 0, 0);
        }

        if (inet_pton(AF_INET6, Parameter_List[1].String_Value, &dest) != 0)
        {
            QCLI_Printf(qcli_net_handle, "ERROR: Incorrect IPv6 dest\n");
            return QCLI_STATUS_ERROR_E;
        }

        if (inet_pton(AF_INET6, Parameter_List[2].String_Value, &nexthop) != 0)
        {
            QCLI_Printf(qcli_net_handle, "ERROR: Incorrect IPv6 nexthop\n");
            return QCLI_STATUS_ERROR_E;
        }

        ifname = Parameter_List[3].String_Value;

        rc = qapi_Net_IPv6_Route_Add(ifname, &dest, prefixlen, &nexthop);
    }
    else if (strcmp(cmd, "del") == 0)
    {
        if (Parameter_Count < 3)
        {
            route6_help();
            goto end;
        }

        cp = strchr(Parameter_List[1].String_Value, '/');
        if (cp == NULL)
        {
            prefixlen = 128;
        }
        else
        {
            *cp++ = '\0';
            prefixlen = strtol(cp, 0, 0);
        }

        if (inet_pton(AF_INET6, Parameter_List[1].String_Value, &dest) != 0)
        {
            QCLI_Printf(qcli_net_handle, "ERROR: Incorrect IPv6 dest\n");
            return QCLI_STATUS_ERROR_E;
        }

        ifname = Parameter_List[2].String_Value;

        rc = qapi_Net_IPv6_Route_Del(ifname, &dest, prefixlen);
    }
    else if (strncmp(cmd, "show", 3) == 0)
    {
        uint32_t count, i;
        qapi_Net_IPv6_Route_t *buf;
        char dest_str[40];
        char nexthop_str[40];

        rc = qapi_Net_IPv6_Routing_Table_Get(NULL, &count);
        if (rc == QAPI_OK)
        {
            if (count == 0)
            {
                QCLI_Printf(qcli_net_handle, "Routing table is empty.\n");
            }
            else
            {
                buf = malloc(count * sizeof(qapi_Net_IPv6_Route_t));
                if (buf == NULL)
                {
                    rc = QAPI_ERROR;
                    goto end;
                }

                rc = qapi_Net_IPv6_Routing_Table_Get(buf, &count);
                if (rc == QAPI_OK)
                {
                    QCLI_Printf(qcli_net_handle, "Destination           Nexthop              iface\n");
                    for (i = 0; i < count; ++i)
                    {
                        QCLI_Printf(qcli_net_handle, "%s/%d      %s      %s\n",
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
    else
    {
        QCLI_Printf(qcli_net_handle, "ERROR: Invalid command: %s\n", cmd);
    }

end:
    if (rc == QAPI_OK)
    {
        return QCLI_STATUS_SUCCESS_E;
    }
    else
    {
        return QCLI_STATUS_ERROR_E;
    }
}
#endif

/*****************************************************************************
 *                     [0]                           [1]              [2]             [3]             [4]....
 * cfg_netbuf   <custom> <pool_size> <buf_size> <num_buf> <buf_size> <num_buf>
 *                   <set> <profileID>
 *                   <get>
 ****************************************************************************/
static QCLI_Command_Status_t cfg_netbuf(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    qapi_Net_Profile_Custom_Pool_t *net_buf_pool;
    qapi_Net_Profile_Type_t profile;
    uint8_t pool_size;
    QCLI_Command_Status_t status = QCLI_STATUS_SUCCESS_E;
    char *profileString[QAPI_NET_PROFILE_MAX] = { "PERFORMANCE",
                                                  "BEST EFFORT",
                                                  "MEMORY OPTIMIZIED",
                                                  "CUSTOM     " };
    uint32_t i = 0;
    char *cmd;
    int ret = 0;

    /* invalid options */
    if (Parameter_Count < 1 || Parameter_List == NULL)
    {
        goto parm_error;
    }

    cmd = Parameter_List[0].String_Value;

   if(strncmp(cmd, "custom", 3) == 0){
        /* validate the input */
        if( Parameter_Count < 2){
            goto parm_error;
        }

        /* read the no pool and validate size and count for each pool */
        pool_size = Parameter_List[1].Integer_Value;
        if( Parameter_Count < (2 + (pool_size * 2))){
            goto parm_error;
        }

        net_buf_pool = malloc((sizeof(qapi_Net_Profile_Custom_Pool_t) * pool_size));
        if( net_buf_pool == NULL){
            goto parm_error;
        }

        for(i= 2; i < Parameter_Count; i+=2){
           net_buf_pool->buf_Size = Parameter_List[i].Integer_Value;
           net_buf_pool->buf_Num = Parameter_List[i+1].Integer_Value;
           net_buf_pool++;
        }
        net_buf_pool -= (pool_size);
        ret= qapi_Net_Profile_Set_Custom(net_buf_pool, pool_size);
	if (ret != 0) {
		QCLI_Printf(qcli_net_handle, "Set custom profile failed %d\n", ret);
            status = QCLI_STATUS_ERROR_E;
        }
        free(net_buf_pool);
    }
    else if(strncmp(cmd, "set", 2) == 0){
        /* validate the input */
        if( Parameter_Count < 2){
            goto parm_error;
        }

        profile = Parameter_List[1].Integer_Value;
        ret = qapi_Net_Profile_Set_Active(profile);
        if (ret != 0) {
        	if (ret == QAPI_ERR_BUSY) {
        		QCLI_Printf(qcli_net_handle, "ERROR: Cannot switch profiles with open sockets\nPlease stop all active services and traffic tests and try again\n");
        	}
			QCLI_Printf(qcli_net_handle, "Set Profile failed ret:%d\n", ret);
			return QCLI_STATUS_ERROR_E;
		}
        QCLI_Printf(qcli_net_handle, "Active Profile : %s\n", profileString[profile - 1]);
    }
    /* wmiconfig --netbuf get_active_profile  */
    else if(strncmp(cmd, "show", 2) == 0){
        if(0 != qapi_Net_Profile_Get_Active(&net_buf_pool, &pool_size, &profile))
            return QCLI_STATUS_ERROR_E;

        if(pool_size){
            QCLI_Printf(qcli_net_handle, " \n Profile : %s\n", profileString[profile - 1]);
            QCLI_Printf(qcli_net_handle, " BufSize     Count\n");
          do{
             QCLI_Printf(qcli_net_handle, "%8d  %8d\n", net_buf_pool->buf_Size, net_buf_pool->buf_Num);
             net_buf_pool++;
          }while(--pool_size);
       }
    }
    else
        return QCLI_STATUS_USAGE_E;

    return status;

    parm_error:
    return QCLI_STATUS_USAGE_E;
}

#define OM_FOM 0
#define OM_SOM 1
#define OM_MOM 2
/*****************************************************************************
 *           [0]
 * nsstrrcl  exit
 ****************************************************************************/
static QCLI_Command_Status_t nsstrrcl(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    char *cmd;
    int e = -1;

    /* invalid options */
    if (Parameter_Count < 1 || Parameter_List == NULL)
    {
        return QCLI_STATUS_USAGE_E;
    }

    cmd = Parameter_List[0].String_Value;

    if (strncmp(cmd, "exit", 3) == 0)
    {
        e = qapi_OMTM_Switch_Operating_Mode(OM_SOM, QAPI_OMTM_SWITCH_NOW_E);
        QCLI_Printf(qcli_net_handle, "exit(): %d\n", e);
    }
    else
    {
        return QCLI_STATUS_USAGE_E;
    }

    return QCLI_STATUS_SUCCESS_E;
}

#ifdef CONFIG_NET_DNSS_DEMO
/*****************************************************************************
 *       [0]     [1]            [2]                        [3]
 * dnss
 * dnss start
 * dnss stop
 * dnss disable
 * dnss addhost  sclcelinux     192.168.1.50
 * dnss addhost  www.yahoo.com  2001:4998:c:a06::2:4008    3600
 * dnss addhost  www.yahoo.com  206.190.36.45
 * dnss delhost  www.yahoo.com
 *****************************************************************************/
static QCLI_Command_Status_t dnss(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int32_t e = -1;
    char *cmd, *hostname, *addrstr;

    /* dnss */
    if (Parameter_Count == 0)
    {
        int32_t i, n;
        int32_t started;
        qapi_Net_DNS_Host_t *list;
        char ip_str[16];
        char ip6_str[40];
        ip6_addr ip6unspecified = { { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} };

        started = qapi_Net_DNSs_Is_Started();
        QCLI_Printf(qcli_net_handle, "DNS server is %s.\n", started ? "started" : "stopped");
        e = qapi_Net_DNSs_Get_Host_List(&n, NULL);
        if (e == 0)
        {
            QCLI_Printf(qcli_net_handle, "Host table: %d entries\n", n);
        }
        if (n > 0 &&
            (list = (qapi_Net_DNS_Host_t *)malloc(n * sizeof(qapi_Net_DNS_Host_t))) != NULL)
        {
            qapi_Net_DNSs_Get_Host_List(&n, list);
            for (i = 0; i < n; ++i)
            {
                QCLI_Printf(qcli_net_handle, "  %s    %s    %s    %u\n",
                        list[i].host_Name,
                        list[i].addr4 == 0 ? "****" : inet_ntop(AF_INET, &list[i].addr4, ip_str, sizeof(ip_str)),
                        memcmp(&list[i].addr6, &ip6unspecified, sizeof(ip6_addr)) == 0 ? "****" :
                                inet_ntop(AF_INET6, &list[i].addr6, ip6_str, sizeof(ip6_str)),
                        list[i].ttl);
            }
            free(list);
        }

        /* If not started, we want to display cmd syntax */
        if (!started)
        {
            return QCLI_STATUS_USAGE_E;
        }

        return QCLI_STATUS_SUCCESS_E;
    }

    cmd = Parameter_List[0].String_Value;

    /*      [0]      [1]            [2]                        [3]
     * dnss addhost  sclcelinux     192.168.1.50
     * dnss addhost  www.yahoo.com  2001:4998:c:a06::2:4008    3600
     */
    if (strncmp(cmd, "addhost", 3) == 0)
    {
        struct ip46addr hostaddr;
        uint32_t ttl;

        if (Parameter_Count < 3 || Parameter_Count > 4)
        {
            goto parm_error;
        }

        hostname = Parameter_List[1].String_Value;
        if (strlen(hostname) >= QAPI_NET_DNS_MAX_HOSTNAME_LEN)
        {
            QCLI_Printf(qcli_net_handle, "hostname cannot be more than %d bytes\n", QAPI_NET_DNS_MAX_HOSTNAME_LEN);
            return QCLI_STATUS_ERROR_E;
        }

        addrstr = Parameter_List[2].String_Value;
        if (inet_pton(AF_INET, addrstr, &hostaddr.a.addr4) == 0)
        {
            hostaddr.type = AF_INET;
        }
        else if (inet_pton(AF_INET6, addrstr, &hostaddr.a.addr6) == 0)
        {
            hostaddr.type = AF_INET6;
        }
        else
        {
            hostaddr.type = AF_UNSPEC;
        }

        if (Parameter_Count == 3)
        {
            ttl = 0;
        }
        else if (Parameter_List[3].Integer_Is_Valid)
        {
            ttl = Parameter_List[3].Integer_Value;
        }
        else
        {
            QCLI_Printf(qcli_net_handle, "Invalid Time-to-live\n");
            return QCLI_STATUS_ERROR_E;
        }

        e = qapi_Net_DNSs_Add_Host(hostname, &hostaddr, ttl);

        if (e == QAPI_NET_ERR_INVALID_IPADDR)
        {
            QCLI_Printf(qcli_net_handle, "Invalid host address\n");
        }
    }

    /*
     *      [0]      [1]
     * dnss delhost  www.yahoo.com
     */
    else if (strncmp(cmd, "delhost", 3) == 0)
    {
        if (Parameter_Count != 2)
        {
            goto parm_error;
        }

        hostname = Parameter_List[1].String_Value;
        e = qapi_Net_DNSs_Del_Host(hostname);
    }

    /* dnss start */
    else if (strncmp(cmd, "start", 3) == 0)
    {
        e = qapi_Net_DNSs_Command(QAPI_NET_DNS_SERVER_START_E);
    }

    /* dnss stop */
    else if (strncmp(cmd, "stop", 3) == 0)
    {
        e = qapi_Net_DNSs_Command(QAPI_NET_DNS_SERVER_STOP_E);
    }

    /* dnss disable */
    else if (strncmp(cmd, "disable", 3) == 0)
    {
        e = qapi_Net_DNSs_Command(QAPI_NET_DNS_SERVER_DISABLE_E);
    }

    else
    {
        QCLI_Printf(qcli_net_handle, "Invalid subcommand: %s\n", cmd);
        goto parm_error;
    }

    if (e)
    {
        QCLI_Printf(qcli_net_handle, "%s failed\n", cmd);
        return QCLI_STATUS_ERROR_E;
    }
    else
    {
        return QCLI_STATUS_SUCCESS_E;
    }

parm_error:
    return QCLI_STATUS_USAGE_E;
}
#endif

#ifdef CONFIG_NET_MDNSS_DEMO
int mdnss_cmd_cb(void *arg)
{
	qapi_Net_mDNS_Svc_Reg_Status_t *reg_status = NULL;
	qapi_Status_t status = QAPI_OK;

	if (arg) {
		reg_status = (qapi_Net_mDNS_Svc_Reg_Status_t *)arg;
		if (reg_status->status < 0) {
			status = __QAPI_ERROR(QAPI_MOD_NETWORKING, -reg_status->status);
		}

		if (status != QAPI_OK) {
			if (status == QAPI_NET_STATUS_MDNSD_HOSTNAME_CONFLICT) {
				QCLI_Printf(qcli_net_handle, "Hostname conflict\n");
			}
			else if (status == QAPI_NET_STATUS_MDNSD_SVC_CONFLICT) {
				QCLI_Printf(qcli_net_handle, "Service name conflict\n");
			}
			QCLI_Printf(qcli_net_handle, "Registration status %d\n", reg_status->status);
			QCLI_Printf(qcli_net_handle, "Svc Name:%s\n", reg_status->svc_Name);
		}
		else {
			QCLI_Printf(qcli_net_handle, "Registration successful for %s\n", reg_status->svc_Name);
		}
	}

	return QAPI_OK;
}

/* Default mdnss mode is blocking. But user can override the setting on cmdline */
uint8_t g_mdnss_blocking = 1;

/*****************************************
 * mdnss start <ipv4 | ipv6 | ipboth> <interface> <blocking>
 * mdnss sethostname <hostname>
 * mdnss stop
 * mdnss addsvc <svc_name> <host_port> <txt>
 * mdnss rmsvc <svc_name>
 *
 ******************************************/
static QCLI_Command_Status_t mdnss(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	QCLI_Command_Status_t status = QCLI_STATUS_SUCCESS_E;
	int ret = 0;
	char *input, *cmd;

	if (Parameter_Count == 0 || Parameter_List == NULL) {
		status = QCLI_STATUS_USAGE_E;
		return QCLI_STATUS_USAGE_E;
	}

	cmd = Parameter_List[0].String_Value;
	input = Parameter_List[1].String_Value;

	//_Name._Myprinter._tcp.local or something like that.
	if (strcmp(cmd, "addsvc") == 0) {
		qapi_Net_mDNS_Svc_Info_t svc={0};
		int txtCount=0, i=0, j=0;
		input = Parameter_List[1].String_Value;
		svc.instance = strtok(input,".");
		svc.type = strtok(NULL, "");
		svc.port = Parameter_List[2].Integer_Value;
		if (Parameter_Count >= 3) {
			txtCount = Parameter_Count - 3;

			if(txtCount >= QAPI_NET_MDNS_MAX_TXT_RECORDS) {
			    ret = QAPI_ERR_INVALID_PARAM;
	            status = QCLI_STATUS_ERROR_E;
	            goto end;
			}

			for (i = 0,j = 3; i < txtCount; i++, j++) {
				svc.txt[i] = Parameter_List[j].String_Value;
			}
		}

		ret = qapi_Net_mDNS_Command(QAPI_NET_MDNS_ADDSVC_E, &svc, g_mdnss_blocking, mdnss_cmd_cb);
		if (ret != 0) {
			if(ret == QAPI_NET_STATUS_MDNSD_SVC_REGISTRATION_FAILURE) {
				QCLI_Printf(qcli_net_handle, "Service name conflict\n");
			}
			status = QCLI_STATUS_ERROR_E;
			goto end;
		}
		else {
			if (g_mdnss_blocking) {
				QCLI_Printf(qcli_net_handle, "Service added successfully\n");
			}
		}
	}
	else if (strcmp(cmd, "rmsvc") == 0) {
		if (!input) {
			status = QCLI_STATUS_ERROR_E;
			goto end;
		}

		ret = qapi_Net_mDNS_Command(QAPI_NET_MDNS_RMSVC_E, input, g_mdnss_blocking, NULL);
		if (ret < 0) {
			status = QCLI_STATUS_ERROR_E;
			goto end;
		}
		else {
			QCLI_Printf(qcli_net_handle, "Service removed successfully\n");
		}
	}
	else if (strcmp(cmd, "start") == 0) {
		qapi_Net_mDNS_Start_t start;
		input = Parameter_List[1].String_Value;

		if (Parameter_Count < 3) {
			status = QCLI_STATUS_USAGE_E;
			QCLI_Printf(qcli_net_handle, "Please provide ipVersion and ifname\n");
			goto end;
		}

		if (strcmp(input, "ipv4") == 0) {
			start.ip_Version = AF_INET;
		}
		else if (strcmp(input, "ipv6") == 0) {
			start.ip_Version = AF_INET6;
		}
		else if (strcmp(input, "ipboth") == 0) {
			start.ip_Version = AF_INET_DUAL46;
		}
		else {
			QCLI_Printf(qcli_net_handle, "Please specify ipversion ipv4 | ipv6 | ipboth\n");
			status = QCLI_STATUS_ERROR_E;
			goto end;
		}

		start.if_Name = Parameter_List[2].String_Value;

		if (Parameter_Count == 4) {
			g_mdnss_blocking = Parameter_List[3].Integer_Value;
		}

		ret = qapi_Net_mDNS_Command(QAPI_NET_MDNS_START_E, &start, g_mdnss_blocking, mdnss_cmd_cb);

		if(ret == QAPI_OK) {
			QCLI_Printf(qcli_net_handle, "mdnss service started successfully\n");
		}
		else {
			QCLI_Printf(qcli_net_handle, "mdnss service start failed\n");
			status = QCLI_STATUS_ERROR_E;
			goto end;
		}
	}
	else if (strcmp(cmd, "stop") == 0) {
		ret = qapi_Net_mDNS_Command(QAPI_NET_MDNS_STOP_E, NULL, g_mdnss_blocking, NULL);
		if(ret != 0) {
			status = QCLI_STATUS_ERROR_E;
			goto end;
		}
		else {
			QCLI_Printf(qcli_net_handle, "mdnss service stopped successfully\n");
		}
	}
	else if (strcmp(cmd, "sethostname") == 0) {
		char *hostname = Parameter_List[1].String_Value;
		ret = qapi_Net_mDNS_Command(QAPI_NET_MDNS_SET_HOSTNAME_E, hostname, g_mdnss_blocking, NULL);
		if(ret != 0) {
			status = QCLI_STATUS_ERROR_E;
			goto end;
		}
		else {
			if (g_mdnss_blocking) {
				QCLI_Printf(qcli_net_handle,"Set hostname successful\n");
			}
		}
	}
	else if (strcmp(cmd, "updatetxt") == 0) {
		qapi_Net_mDNS_TXT_t txt_update;

		txt_update.svc_Name = Parameter_List[1].String_Value;
		txt_update.txt = Parameter_List[2].String_Value;

		ret = qapi_Net_mDNS_Command(QAPI_NET_MDNS_UPDATE_TXT_RECORD_E, &txt_update, g_mdnss_blocking, NULL);
		if (ret != 0) {
			status = QCLI_STATUS_ERROR_E;
			goto end;
		}
		else {
			QCLI_Printf(qcli_net_handle,"Text update successful\n");
		}
	}
	else
	    return QCLI_STATUS_USAGE_E;
end:
	if (status) {
		if (ret < 0) {
			switch(ret) {
				case QAPI_NET_ERR_SOCKET_FAILURE:
					QCLI_Printf(qcli_net_handle, "Socket failure\n");
					break;
				case QAPI_ERR_INVALID_PARAM:
					QCLI_Printf(qcli_net_handle, "Invalid input specified. Check cmd syntax\n");
					break;
				case QAPI_ERR_NO_ENTRY:
					QCLI_Printf(qcli_net_handle, "MDNS instance doesn't exist\n");
					break;
				case QAPI_ERR_NO_MEMORY:
					QCLI_Printf(qcli_net_handle, "Memory allocation failed\n");
					break;
				case QAPI_NET_ERR_OPERATION_FAILED:
					QCLI_Printf(qcli_net_handle, "Socket send failed\n");
					break;
				case QAPI_ERR_BUSY:
					QCLI_Printf(qcli_net_handle, "Operation Busy\n");
					break;
				case QAPI_ERROR:
					QCLI_Printf(qcli_net_handle, "Internal Error\n");
					break;
				case QAPI_ERR_EXISTS:
					QCLI_Printf(qcli_net_handle, "Entry already exists\n");
					break;
				case QAPI_ERR_NO_RESOURCE:
					QCLI_Printf(qcli_net_handle, "Resource allocation failed\n");
					break;
				case QAPI_NET_STATUS_MDNSD_HOSTNAME_CONFLICT:
					QCLI_Printf(qcli_net_handle,"Hostname conflict\n");
					break;
				case QAPI_NET_STATUS_MDNSD_STATE_INVALID:
					QCLI_Printf(qcli_net_handle, "Invalid state. Set hostname before service registration\n");
					break;
				default:
					QCLI_Printf(qcli_net_handle, "Unexpected Error\n");
					break;
			}
		}
		QCLI_Printf(qcli_net_handle, "%s failed status %d\n", cmd, ret);
	}

	return status;
}
#endif

#ifdef CONFIG_NET_DNSSD_DEMO
void dnssd_cmd_complete(qapi_Net_DNSSD_Discover_t *buf)
{
	int minEntryCount=0;
	int minDataCount=0;
	int i=0,j=0,k=0;
	char ip_str[40];
	qapi_Net_DNSSD_Discover_Entry_t *entry;

	minEntryCount = (buf->ctxt->max_Entries < buf->entry_Count) ? buf->ctxt->max_Entries : buf->entry_Count;
	if (!minEntryCount) {
		QCLI_Printf(qcli_net_handle, "%s: No data available\n", __func__);
		goto end;
	}


	for(i=0; i<minEntryCount; i++) {
		entry = buf->entries[i];
		minDataCount = (buf->ctxt->max_Entries < entry->data_Count) ? buf->ctxt->max_Entries : entry->data_Count;
		for(j=0; j<minDataCount; j++) {
			qapi_Net_DNSSD_Data_t *data = entry->data[j];
			switch(data->type) {
				case QAPI_NET_DNSSD_IPV4_ADDR:
					inet_ntop(AF_INET, &data->data.ipv4_Addr, ip_str, sizeof(ip_str));
					QCLI_Printf(qcli_net_handle, "Received type: %d IP: %s\n", data->type, ip_str);
					break;
				case QAPI_NET_DNSSD_PTR:
					QCLI_Printf(qcli_net_handle, "Received PTR record: name %s\n", data->data.svr_Name);
					break;
				case QAPI_NET_DNSSD_TYPE_TXT_INFO:
					for (k=0; k<5; k++) {
						if (data->data.txt[k]) {
							QCLI_Printf(qcli_net_handle, "Received type: %d txt %s\n", data->type, data->data.txt[k]);
						}
					}
					break;
				case QAPI_NET_DNSSD_IPV6_ADDR:
					if (inet_ntop(AF_INET6, data->data.ipv6_Addr, ip_str, sizeof(ip_str)) != NULL) {
						QCLI_Printf(qcli_net_handle, "Received type: %d ipv6: %s\n", data->type, ip_str);
					}
					break;
				case QAPI_NET_DNSSD_TYPE_TARGET_INFO:
					QCLI_Printf(qcli_net_handle, "Received type: %d name %s \n priority: %d\n weight: %d\n port:%d\n target:%s\n",
							data->type, data->name, data->data.target->priority, data->data.target->weight, data->data.target->port, data->data.target->server_Name);
					break;
				default:
					QCLI_Printf(qcli_net_handle, "%s: data type %d not supported\n", __func__, data->type);
					break;
			}
		}
	}

end:
	return;
}

/********************************************************************************************
 * dnssd start <ipv4 | ipv6 | ipboth> <ifname>
 * dnssd stop
 * dnssd init timout
 * dnssd discover <instance_name>
 * dnssd gettargetinfo <instance_name>
 * *****************************************************************************************/
static QCLI_Command_Status_t dnssd(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	char *cmd, *input;
	QCLI_Command_Status_t status = QCLI_STATUS_SUCCESS_E;
	int ret=0;

	if (Parameter_Count == 0) {
		QCLI_Printf(qcli_net_handle, "dnssd start <ipv4|ipv6|ipboth> ifname\n"
						"dnssd stop\n"
						"dnssd init <timeout in milliseconds>\n"
						"dnssd discover <instance name>\n"
						"dnssd gettargetinfo <instance name>\n");
		status = QCLI_STATUS_USAGE_E;
		return status;
	}

	cmd = Parameter_List[0].String_Value;

	if (strcmp("start", cmd) == 0) {
		qapi_Net_DNSSD_Start_t start;
		input = Parameter_List[1].String_Value;

		if (strcmp(input, "ipv4") == 0) {
			start.ip_Version = AF_INET;
		}
		else if (strcmp(input, "ipv6") == 0) {
			start.ip_Version = AF_INET6;
		}
		else if (strcmp(input, "ipboth") == 0) {
			start.ip_Version = AF_INET_DUAL46;
		}
		else {
			QCLI_Printf(qcli_net_handle, "Please specify ipversion ipv4 | ipv6 | ipboth\n");
			status = QCLI_STATUS_ERROR_E;
			goto end;
		}

		start.if_Name = Parameter_List[2].String_Value;
		ret = qapi_Net_DNSSD_Start(&start);
		if (ret != 0) {
			status = QCLI_STATUS_ERROR_E;
			goto end;
		}
		else {
			QCLI_Printf(qcli_net_handle, "DNSSD service started successfully\n");
		}
	}
	else if (strcmp("init", cmd) == 0) {
		int timeout = QAPI_NET_DNSSD_MAX_TIMEOUT;
		qapi_Net_DNSSD_Init_t init;
		if (Parameter_Count == 2) {
			timeout = Parameter_List[1].Integer_Value;
		}

		init.ctxt = &qapi_Net_DNSSD_Ctxt;
		init.timeout = timeout;
		init.max_Entries = QAPI_NET_DNSSD_MAX_BUF_ENTRIES;
		init.cb = dnssd_cmd_complete;

		ret = qapi_Net_DNSSD_Init(&init);
		if (ret != 0) {
			QCLI_Printf(qcli_net_handle, "Allocation failed\n");
			qapi_Net_DNSSD_Ctxt = NULL;
			status = QCLI_STATUS_ERROR_E;
			goto end;
		}
		else {
			QCLI_Printf(qcli_net_handle, "Init successful\n");
		}
	}
	else if (strcmp("stop", cmd) == 0) {
		ret = qapi_Net_DNSSD_Stop(qapi_Net_DNSSD_Ctxt);
		if (ret != 0) {
			status = QCLI_STATUS_ERROR_E;
			goto end;
		}
		else {
			QCLI_Printf(qcli_net_handle, "Service discovery stopped\n");
			qapi_Net_DNSSD_Ctxt = NULL;
		}
	}
	else if (strcmp("discover", cmd) == 0) {
		input = Parameter_List[1].String_Value;
		ret = qapi_Net_DNSSD_Discover(input);
		if (ret < 0) {
			status = QCLI_STATUS_ERROR_E;
			QCLI_Printf(qcli_net_handle, "Discovery failed\n");
			goto end;
		}
		else {
			QCLI_Printf(qcli_net_handle, "Discovery in progress\n");
		}
	}
	else if (strcmp("gettargetinfo", cmd) == 0) {
		input = Parameter_List[1].String_Value;
		ret = qapi_Net_DNSSD_Get_Target_Info(input);
		if (ret != 0) {
			status = QCLI_STATUS_ERROR_E;
			QCLI_Printf(qcli_net_handle, "Get TargetInfo failed\n");
			goto end;
		}
		else {
			QCLI_Printf(qcli_net_handle, "Request in progress\n");
		}
	}
    else
        return QCLI_STATUS_USAGE_E;
end:
	if (status) {
		switch(ret) {
			case QAPI_NET_ERR_SOCKET_FAILURE:
				QCLI_Printf(qcli_net_handle, "Socket failure\n");
				break;
			case QAPI_ERR_INVALID_PARAM:
				QCLI_Printf(qcli_net_handle, "Invalid input. Check input parameters\n");
				break;
			case QAPI_ERR_NO_ENTRY:
				QCLI_Printf(qcli_net_handle, "DNSSD instance doesn't exist\n");
				break;
			case QAPI_ERR_NO_MEMORY:
				QCLI_Printf(qcli_net_handle, "Memory allocation failed\n");
				break;
			case QAPI_NET_ERR_OPERATION_FAILED:
				QCLI_Printf(qcli_net_handle, "Socket send failed\n");
				break;
			case QAPI_ERR_BUSY:
				QCLI_Printf(qcli_net_handle, "Operation Busy\n");
				break;
			case QAPI_ERROR:
				QCLI_Printf(qcli_net_handle, "Internal Error\n");
				break;
			default:
				QCLI_Printf(qcli_net_handle, "Unexpected Error\n");
				break;
		}
		QCLI_Printf(qcli_net_handle, "%s failed status %d\n", cmd, ret);
	}

	return status;
}
#endif

static QCLI_Command_Status_t freeq(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	int ret=0;
	QCLI_Command_Status_t status = QCLI_STATUS_SUCCESS_E;
	int idx=0;
	qapi_Net_Buf_Queue_Status_t qStatus;
	uint8_t size = QAPI_NETBUF_MAX_QUEUES;
	qapi_Net_Socket_Status_t socket_status;

	ret = qapi_Net_Buf_Free_Queue_Status(&qStatus);

	if (ret < 0) {
		status = QCLI_STATUS_ERROR_E;
		QCLI_Printf(qcli_net_handle, "ERROR: received %d\n", ret);
		return status;
	}

	QCLI_Printf(qcli_net_handle, "Total free buffers: %d\n", qStatus.total_Free_Buffers);

	for (idx=0; idx<size; idx++) {
		QCLI_Printf(qcli_net_handle, "%s: %d/%d\n", qStatus.queue[idx].name, qStatus.queue[idx].free_Buffers, qStatus.queue[idx].total_Buffers);
	}

	if (qapi_Net_Get_Socket_Status(&socket_status) == QAPI_OK) {
		uint32_t free_sockets;

		free_sockets = socket_status.total_sockets - (socket_status.open_af_inet + socket_status.open_af_inet6);
		QCLI_Printf(qcli_net_handle, "\nTotal free sockets: %d/%d\n", free_sockets, socket_status.total_sockets);
		QCLI_Printf(qcli_net_handle, "Open IPv4/AF_INET sockets: %d\nOpen IPv6/AF_INET6 sockets: %d\n",
				socket_status.open_af_inet, socket_status.open_af_inet6);
	}

	return status;
}

#ifdef CONFIG_NET_TXRX_DEMO
static QCLI_Command_Status_t dump_command_handler(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	char *cmd;
	QCLI_Command_Status_t status = QCLI_STATUS_SUCCESS_E;
	uint32_t i = 1;

	if (Parameter_Count == 0) {
		status = QCLI_STATUS_USAGE_E;
		return status;
	}

	cmd = Parameter_List[0].String_Value;

	if (strcmp("enable", cmd) == 0) {
		dump_enabled = 1;
	}
	else if (strcmp("disable", cmd) == 0) {
		dump_enabled = 0;
		dump_flags = 0;
		return status;
	}
	else {
		status = QCLI_STATUS_USAGE_E;
		return status;
	}

	Parameter_Count--;

	if (Parameter_Count) {
		dump_flags = 0;
	} else if (dump_enabled) {
		dump_flags = (DUMP_ENABLE_TX | DUMP_ENABLE_RX);
	}

	while (Parameter_Count) {
		cmd = Parameter_List[i].String_Value;

		if (strcmp("tx", cmd) == 0) {
			dump_flags |= DUMP_ENABLE_TX;
		}
		else if (strcmp("rx", cmd) == 0) {
			dump_flags |= DUMP_ENABLE_RX;
		}
		else {
			dump_enabled = 0;
			dump_flags = 0;
			status = QCLI_STATUS_USAGE_E;
			return status;
		}

		i++;
		Parameter_Count--;
	}

	return status;
}
#endif

#ifdef CONFIG_NET_ROUTE_DEMO
static QCLI_Command_Status_t route(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	char *ifName;
	char *cmd;
	qapi_Net_IPv4_Route_List_t *routeList = NULL;
	qapi_Status_t ret = 0;
	int i = 0;
	uint32_t ipv4_addr = 0;
	uint32_t mask = 0;
	uint32_t gateway = 0;
	char ip_str[20];
	char mask_str[20];
	char gw_str[20];

	if (Parameter_Count == 0 || Parameter_Count < 2) {
		return QCLI_STATUS_USAGE_E;
	}

	cmd = Parameter_List[0].String_Value;
	ifName = Parameter_List[1].String_Value;

	routeList = (qapi_Net_IPv4_Route_List_t *)malloc(sizeof(qapi_Net_IPv4_Route_List_t));
	if (!routeList) {
		QCLI_Printf(qcli_net_handle, "route: malloc failed\n");
		return QCLI_STATUS_ERROR_E;
	}

	if (strcmp(cmd, "show") == 0) {
		ret = qapi_Net_IPv4_Route(ifName, QAPI_NET_ROUTE_SHOW_E, NULL, NULL, NULL, routeList);
		if (!ret) {
			QCLI_Printf(qcli_net_handle, "IPv4 Routing Table\n\n");
			QCLI_Printf(qcli_net_handle, "\tDestination\t\tGateway\t\tSubnet Mask\t\tIface\n");
			QCLI_Printf(qcli_net_handle,"-------------------------------------------------------------------------\n");
			for (i=0; i < routeList->route_Count; i++) {
	            QCLI_Printf(qcli_net_handle, "%15s\t %15s\t %15s\t %5s\n",
	                        inet_ntop(AF_INET, &routeList->route[i].ipRouteDest, ip_str, sizeof(ip_str)),
	                        inet_ntop(AF_INET, &routeList->route[i].ipRouteNextHop, gw_str, sizeof(gw_str)),
	                        inet_ntop(AF_INET, &routeList->route[i].ipRouteMask, mask_str, sizeof(mask_str)),
							routeList->route[i].ifName);
			}
		}
		else {
			QCLI_Printf(qcli_net_handle, "route show failed %d\n", ret);
		}
	}
	else if (strcmp(cmd, "add") == 0) {
		if (inet_pton(AF_INET, Parameter_List[2].String_Value, &ipv4_addr) != 0) {
			QCLI_Printf(qcli_net_handle, "Bad IPv4 address\n");
		}

		if (inet_pton(AF_INET, Parameter_List[3].String_Value, &mask) != 0) {
			QCLI_Printf(qcli_net_handle, "Bad mask\n");
		}

		if (inet_pton(AF_INET, Parameter_List[4].String_Value, &gateway) != 0) {
			QCLI_Printf(qcli_net_handle, "Bad gateway\n");
		}

		ret = qapi_Net_IPv4_Route(ifName, QAPI_NET_ROUTE_ADD_E, &ipv4_addr, &mask, &gateway, NULL);
		if (ret) {
			QCLI_Printf(qcli_net_handle, "route add failed %d\n", ret);
		}
		else {
			QCLI_Printf(qcli_net_handle, "Route added\n");
		}
	}
	else if (strcmp(cmd, "del") == 0) {
		if (inet_pton(AF_INET, Parameter_List[2].String_Value, &ipv4_addr) != 0) {
			QCLI_Printf(qcli_net_handle, "Bad IPv4 address\n");
		}

		if (inet_pton(AF_INET, Parameter_List[3].String_Value, &mask) != 0) {
			QCLI_Printf(qcli_net_handle, "Bad mask\n");
		}

		if (inet_pton(AF_INET, Parameter_List[4].String_Value, &gateway) != 0) {
			QCLI_Printf(qcli_net_handle, "Bad gateway\n");
		}

		ret = qapi_Net_IPv4_Route(ifName, QAPI_NET_ROUTE_DEL_E, &ipv4_addr, &mask, &gateway, NULL);
		if (ret) {
			QCLI_Printf(qcli_net_handle, "route deletion failed %d\n", ret);
		}
		else {
			QCLI_Printf(qcli_net_handle, "Route deleted\n");
		}
	}

	if (routeList) {
		free(routeList);
	}

	if (ret != 0) {
		return QCLI_STATUS_ERROR_E;
	}
	else {
		return QCLI_STATUS_SUCCESS_E;
	}
}
#endif
