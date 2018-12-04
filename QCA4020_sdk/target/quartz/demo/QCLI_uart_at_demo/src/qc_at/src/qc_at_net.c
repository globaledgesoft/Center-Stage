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

QCLI_Command_Status_t qc_at_net_Help(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int ret = QCLI_STATUS_SUCCESS_E;

    ret = qc_api_net_Help(Parameter_Count, Parameter_List);
    if (0 == ret)
        LOG_AT_OK();
    else
        LOG_AT_ERROR();

    return ret;
}

QCLI_Command_Status_t qc_at_net_Ping(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int ret = QCLI_STATUS_SUCCESS_E;

    if (Parameter_Count < 1 || Parameter_List == NULL)
    {
        LOG_AT_ERROR();
        return QCLI_STATUS_USAGE_E;
    }

   /* if (QCLI_STATUS_SUCCESS_E != Uart_Thread_Create(qc_api_net_Ping, Parameter_Count, Parameter_List))
    {
        LOG_AT_ERROR();
        return QCLI_STATUS_ERROR_E;
    } */

    ret = qc_api_net_Ping(Parameter_Count, Parameter_List);
    if (0 == ret)
        LOG_AT_OK();
    else
        LOG_AT_ERROR();

    return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t qc_at_net_Ifconfig(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int ret = QCLI_STATUS_SUCCESS_E;

    ret = qc_api_net_Ifconfig(Parameter_Count, Parameter_List);
    if (0 == ret)
        LOG_AT_OK();
    else
        LOG_AT_ERROR();

    return ret;
}

QCLI_Command_Status_t qc_at_net_DhcpV4Client(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int ret = QCLI_STATUS_SUCCESS_E;

    ret = qc_api_net_DhcpV4Client(Parameter_Count, Parameter_List);
    if (0 == ret)
        LOG_AT_OK();
    else
        LOG_AT_ERROR();

    return ret;
}

static QCLI_Command_Status_t qc_at_net_DhcpV4server(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int ret = QCLI_STATUS_SUCCESS_E;

    ret = qc_api_net_DhcpV4server(Parameter_Count, Parameter_List);
    if (0 == ret)
        LOG_AT_OK();
    else
        LOG_AT_ERROR();

    return ret;
}

QCLI_Command_Status_t qc_at_net_AutoIp(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int ret = QCLI_STATUS_SUCCESS_E;

    ret = qc_api_net_AutoIp(Parameter_Count, Parameter_List);
    if (0 == ret)
        LOG_AT_OK();
    else
        LOG_AT_ERROR();

    return ret;
}

QCLI_Command_Status_t qc_at_net_PingV6(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int ret = QCLI_STATUS_SUCCESS_E;

    ret = qc_api_net_PingV6(Parameter_Count, Parameter_List);
    if (0 == ret)
        LOG_AT_OK();
    else
        LOG_AT_ERROR();

    return ret;
}

QCLI_Command_Status_t qc_at_net_DhcpV6Client(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int ret = QCLI_STATUS_SUCCESS_E;

    ret = qc_api_net_DhcpV6Client(Parameter_Count, Parameter_List);
    if (0 == ret)
        LOG_AT_OK();
    else
        LOG_AT_ERROR();

    return ret;
}

QCLI_Command_Status_t qc_at_net_SntpClient(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int ret = QCLI_STATUS_SUCCESS_E;

    ret = qc_api_net_SntpClient(Parameter_Count, Parameter_List);
    if (0 == ret)
        LOG_AT_OK();
    else
        LOG_AT_ERROR();

    return ret;
}

QCLI_Command_Status_t qc_at_net_DnsClient(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int ret = QCLI_STATUS_SUCCESS_E;

    ret = qc_api_net_DnsClient(Parameter_Count, Parameter_List);
    if (0 == ret)
        LOG_AT_OK();
    else
        LOG_AT_ERROR();

    return ret;
}

QCLI_Command_Status_t qc_at_net_Bridge(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int ret = QCLI_STATUS_SUCCESS_E;

    ret = qc_api_net_Bridge(Parameter_Count, Parameter_List);
    if (0 == ret)
        LOG_AT_OK();
    else
        LOG_AT_ERROR();

    return ret;
}

QCLI_Command_Status_t qc_at_net_SockClose(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int ret = QCLI_STATUS_SUCCESS_E;

    ret = qc_api_net_SockClose(Parameter_Count, Parameter_List);
    if (0 == ret)
        LOG_AT_OK();
    else
        LOG_AT_ERROR();

    return ret;
}

QCLI_Command_Status_t qc_at_net_TcpV6Connect(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int ret = QCLI_STATUS_SUCCESS_E;

    ret = qc_api_net_TcpV6Connect(Parameter_Count, Parameter_List);
    if (0 == ret)
        LOG_AT_OK();
    else
        LOG_AT_ERROR();

    return ret;
}

QCLI_Command_Status_t qc_at_net_TcpConnect(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int ret = QCLI_STATUS_SUCCESS_E;

    ret = qc_api_net_TcpConnect(Parameter_Count, Parameter_List);
    if (0 == ret)
        LOG_AT_OK();
    else
        LOG_AT_ERROR();

    return ret;
}

QCLI_Command_Status_t qc_at_net_UdpV6Connect(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int ret = QCLI_STATUS_SUCCESS_E;

    ret = qc_api_net_UdpV6Connect(Parameter_Count, Parameter_List);
    if (0 == ret)
        LOG_AT_OK();
    else
        LOG_AT_ERROR();

    return ret;
}

QCLI_Command_Status_t qc_at_net_UdpConnect(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int ret = QCLI_STATUS_SUCCESS_E;

    ret = qc_api_net_UdpConnect(Parameter_Count, Parameter_List);
    if (0 == ret)
        LOG_AT_OK();
    else
        LOG_AT_ERROR();

    return ret;
}

QCLI_Command_Status_t qc_at_net_TcpV6Server(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int ret = QCLI_STATUS_SUCCESS_E;

    ret = qc_api_net_TcpV6Server(Parameter_Count, Parameter_List);
    if (0 == ret)
        LOG_AT_OK();
    else
        LOG_AT_ERROR();

    return ret;
}

QCLI_Command_Status_t qc_at_net_TcpServer(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int ret = QCLI_STATUS_SUCCESS_E;

    ret = qc_api_net_TcpServer(Parameter_Count, Parameter_List);
    if (0 == ret)
        LOG_AT_OK();
    else
        LOG_AT_ERROR();

    return ret;
}

QCLI_Command_Status_t qc_at_net_UdpServer(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int ret = QCLI_STATUS_SUCCESS_E;

    ret = qc_api_net_UdpServer(Parameter_Count, Parameter_List);
    if (0 == ret)
        LOG_AT_OK();
    else
        LOG_AT_ERROR();

    return ret;
}

QCLI_Command_Status_t qc_at_net_UdpV6Server(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int ret = QCLI_STATUS_SUCCESS_E;

    ret = qc_api_net_UdpV6Server(Parameter_Count, Parameter_List);
    if (0 == ret)
        LOG_AT_OK();
    else
        LOG_AT_ERROR();

    return ret;
}

QCLI_Command_Status_t qc_at_net_SockInfo(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int ret = QCLI_STATUS_SUCCESS_E;

    ret = qc_api_net_SockInfo(Parameter_Count, Parameter_List);
    if (0 == ret)
        LOG_AT_OK();
    else
        LOG_AT_ERROR();

    return ret;
}

QCLI_Command_Status_t qc_at_net_TxData(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int ret = QCLI_STATUS_SUCCESS_E;

    ret = qc_api_net_TxData(Parameter_Count, Parameter_List);
    if (0 == ret)
        LOG_AT_OK();
    else
        LOG_AT_ERROR();

    return ret;
}

QCLI_Command_Status_t qc_at_net_RxData(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int ret = QCLI_STATUS_SUCCESS_E;

    ret = qc_api_net_RxData(Parameter_Count, Parameter_List);
    if (0 == ret)
        LOG_AT_OK();
    else
        LOG_AT_ERROR();

    return ret;
}

QCLI_Command_Status_t qc_at_net_HttpClient(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int ret = QCLI_STATUS_SUCCESS_E;

    ret = qc_api_net_HttpClient(Parameter_Count, Parameter_List);
    if (0 == ret)
        LOG_AT_OK();
    else
        LOG_AT_ERROR();

    return ret;
}

QCLI_Command_Status_t qc_at_net_HttpServer(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int ret = QCLI_STATUS_SUCCESS_E;

    ret = qc_api_net_HttpServer(Parameter_Count, Parameter_List);
    if (0 == ret)
        LOG_AT_OK();
    else
        LOG_AT_ERROR();

    return ret;
}

QCLI_Command_Status_t qc_at_multi_sockv4_create(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int ret = QCLI_STATUS_SUCCESS_E;

    ret = qc_api_net_MsockCreate(Parameter_Count, Parameter_List);
    if (0 == ret)
        LOG_AT_OK();
    else
        LOG_AT_ERROR();

    return ret;
}

QCLI_Command_Status_t qc_at_net_MsockClose(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int ret = QCLI_STATUS_SUCCESS_E;

    ret = qc_api_net_MsockClose(Parameter_Count, Parameter_List);
    if (0 == ret)
        LOG_AT_OK();
    else
        LOG_AT_ERROR();

    return ret;
}

QCLI_Command_Status_t qc_at_cert_handler(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int ret = QCLI_STATUS_SUCCESS_E;

    ret = qc_api_net_cert_handler(Parameter_Count, Parameter_List);
    if (0 == ret)
        LOG_AT_OK();
    else
        LOG_AT_ERROR();

    return ret;
}


QCLI_Command_Status_t qc_at_ssl_handler(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int ret = QCLI_STATUS_SUCCESS_E;

    ret = qc_api_net_ssl_handler(Parameter_Count, Parameter_List);
    if (0 == ret)
        LOG_AT_OK();
    else
        LOG_AT_ERROR();

    return ret;
}

QCLI_Command_Status_t qc_at_benchtx(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int ret = QCLI_STATUS_SUCCESS_E;

    ret = qc_api_net_benchtx(Parameter_Count, Parameter_List);
    if (0 == ret)
        LOG_AT_OK();
    else
        LOG_AT_ERROR();

    return ret;
}

QCLI_Command_Status_t qc_at_benchrx(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int ret = QCLI_STATUS_SUCCESS_E;

    ret = qc_api_net_benchrx(Parameter_Count, Parameter_List);
    if (0 == ret)
        LOG_AT_OK();
    else
        LOG_AT_ERROR();

    return ret;
}

const QCLI_Command_t atn_cmd_list[] =
{
    /*  cmd function                  flag      cmd_string             usage_string                                Description      */
    { qc_at_net_Help,                 false,    "HELP",                "",                                         "Display the available network commands."
    },
    { qc_at_net_Ping,                 false,    "PING",                "<host>,[<count>,[size]]",          "Send ICMP ECHO_REQUEST to network hosts in IPv4 network."
    },
    { qc_at_net_Ifconfig,             false,    "IFCONFIG",            "[<interface>,<ipv4addr>,<subnetmask>,[default_gateway]]",    "Configure a network interface."
    },
    { qc_at_net_DhcpV4Client,         false,    "DHCPV4C",             "<interface>,[new|release]",                "DHCPv4 Client: Acquire an IPv4 address using Dynamic Host Configuration Protocol v4."
    },
    { qc_at_net_DhcpV4server,         false,    "DHCPV4S",             "<interface name>,<pool|stop>,<start_ip>,<end_ip>,[<lease_time_sec>|infinite]",             "DHCPv4 Server: Set up and configure Dynamic Host Configuration Protocol v4 server"
    },
    { qc_at_net_AutoIp,               false,    "AUTOIPV4",            "<interface_name>",                         "Auto IPv4: Generate an IPv4 Link-Local address."
    },
    { qc_at_net_PingV6,               false,    "PING6",               "<host>,[-c <count>],[-s <size>],[-I <interface>]",     "Reset Wlan stack."
    },
    { qc_at_net_DhcpV6Client,         false,    "DHCPV6C",             "<interface>,[enable|new|confirm|disable|release]",     "DHCPv6 Client: Acquire an IPv6 address using Dynamic Host Configuration Protocol v6."
    },
    { qc_at_net_SntpClient,           false,    "SNTPC",               "start|stop|disable\naddsvr,<server>,<id>\ndelsvr,<id>\nutc\n",    "SNTP Client: Acquire time from the network using Simple Network Time Protocol v4."
    },
    { qc_at_net_DnsClient,            false,    "DNSC",                "start|stop|disable\naddsvr,<server>,[<id>]\ndelsvr,<id>\ngethostbyname,<host>\n<resolve|gethostbyname2>,<host>,[v4|v6]",        "DNS Client: Resolves and caches Domain Name System domain names."
    },
    { qc_at_net_Bridge,               false,    "BRIDGE",              "enable|disable\naging,<val>\nshowmacs",     "WLAN Bridge: Configure IEEE 802.1D Layer 2 bridging over WLAN interfaces."
    },
    { qc_at_net_SockInfo,             false,    "SINFO",               "<sid>",                                    "Socket information."
    },    
    { qc_at_net_SockClose,            false,    "CLOSE",               "<sid>",                                    "Close a IPv4 socket."
    },
    { qc_at_net_TcpConnect,           false,    "CTCP",                "<ip_address>,<port>",                      "TCP client connects to the server."
    },
    { qc_at_net_TcpV6Connect,         false,    "CTCPV6",              "<ip_address>,<port>",                      "TCPV6 client connects to the server."
    },    
    { qc_at_net_UdpConnect,           false,    "CUDP",                "<ip_address>,<port>",                      "UDP client connects to the server."
    },
    { qc_at_net_UdpV6Connect,         false,    "CUDPV6",              "<ip_address>,<port>",                      "UDPV6 client connects to the server."
    },    
    { qc_at_net_TcpServer,            false,    "TCPSERVER",           "<ip_address>,<port>",                      "Start the TCP server."
    },
    { qc_at_net_TcpV6Server,          false,    "TCPV6SERVER",         "<ip_address>,<port>",                      "Start the TCPV6 server."
    },
    { qc_at_net_UdpServer,            false,    "UDPSERVER",           "<ip_address>,<port>",                      "Start the UDP server."
    },
    { qc_at_net_UdpV6Server,          false,    "UDPV6SERVER",         "<ip_address>,<port>",                      "Start the UDPV6 server."
    },
    { qc_at_net_TxData,               false,    "TXDATA",              "<session id>,<len>,[<ip_address>,<port>]", "Tx data for mentioned session"
    },
    { qc_at_net_RxData,               false,    "RXDATA",              "<session id>",                             "Receive data for mentioned session"
    },    
    { qc_at_net_HttpClient,           false,    "HTTPC",               "start\nATHTTPC=<stop>\nATHTTPC=<connect>,[<server>,<port>,<ssl-index>]\nATHTTPC=<disc>,[<client_num>]\nATHTTPC=<get>,[<client_num>,<url>]\nATHTTPC=<put>,[<client_num>,<url>]\nATHTTPC=<post>,[<client_num>,<url>]\nATHTTPC=<patch>,[<client_num>,<url>]\nATHTTPC=<setbody>,[<client_num>,<len>]\nATHTTPC=<addheader>,[<client_num>,<hdr_name>,<hdr_value>]\nATHTTPC=<clearheader>,[<client_num>]\nATHTTPC=<setparam>,[<client_num>]\nATHTTPC=<setparam>,[<client_num>]\nATHTTPC=<config>,<httpc_demo_max_body_len>,<httpc_demo_max_header_len>]",      "Configures the HTTP client at the run time."
    },
    { qc_at_net_HttpServer,           false,    "HTTPS",               "init,<v4|v6|v46>,<http|https|http_https>,<cert_file>,[<httpport>] [<httpsport>],[<ifname>],[<index_page>],[<root_path>]]\nATNHTTPS=<start>\nATNHTTPS=<stop>\nATNHTTPS=<setbufsize>,<TX_buffer_size>,<RX_buffer_size>\nATNHTTPS=<addctype>,[<content-type1>,[<content-type2> ..]\nATNHTTPS=<delctype>,[<content-type1>,[<content-type2> ..]\nATNHTTPS=<sslconfig>\n[<keyword_1> <value_1> <keyword_1> <value_1> …]",         "Configures the HTTP server at the run time."
    },
    { qc_at_multi_sockv4_create,      true,     "MSOCKCREATE",         "<protocol>,<no_of_socket>,<server_ip>,<portno>,<time>,<num_of_bytes>",        "Create multi socket."
    },
    { qc_at_net_MsockClose,           true,     "MSOCKCLOSE",          "[session_id]",                             "Close multi socket."
    },
    { qc_at_cert_handler,             false,    "CERT",                "<store|delete|list|get|hash|download|gencsr> <argument>...\n",    "\nTLS Certificate manager: Perform certificate management operations.\n"
    },
    { qc_at_ssl_handler,              false,    "SSL",                 "ssl,<start|stop|config|cert|psk|ecjpake|max_clients|idle_timer>,<argument>...",       "Secure Socket Layer: Configure Secure Socket Layer for TLS connections\n"
        "Type command name to get more info on usage. For example \"ssl start\"."
    },
    { qc_at_benchtx,                  false,    "BENCHTX",             "",                                   "Perform IPv4 transmit (TX) benchmarking test"        
    },
    { qc_at_benchrx,                  false,    "BENCHRX",             "",                                   "Perform IPv4 receive (RX) benchmarking test"
    },
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
    if (atn_group == NULL)
    {
        LOG_WARN("Failed to register Network commands.\r\n");
    }
    net_sock_initialize();
}
