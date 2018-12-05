/*
* Copyright (c) 2016 Qualcomm Technologies, Inc.
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
#include <string.h>
#include "qapi_status.h"
#include "bench.h"
#include "qurt_types.h"
#include "qurt_timer.h"

#ifdef CONFIG_NET_TXRX_DEMO

//#define UDP_RX_DEBUG
//#define UDP_RX_TS_DEBUG
//#define UDP_RX_STATUS_DEBUG
//#define TCP_RX_DEBUG
//#define ZCOPY_UDP_RX_DEBUG
//#define ZCOPY_TCP_RX_DEBUG
//#define UDP_TX_DEBUG
//#define TCP_TX_DEBUG
//#define SEND_ACK_DEBUG
//#define TCP_RX_RETRY_AFTER_FIN
//#define RAW_RX_TS_DEBUG
//#define RAW_RX_STATUS_DEBUG
#define USE_SERVER_STATS

extern QCLI_Group_Handle_t qcli_net_handle; /* Handle for Net Command Group. */
extern QUEUE_T udp_zcq;
extern uint16_t bench_udp_rx_port_in_use;
uint8_t benchtx_quit;
uint8_t benchrx_quit;
uint8_t dump_enabled = 0;
uint8_t dump_flags = 0;
uint32_t pattern_type;
uint8_t pattern;
int32_t txqueue_size = -1;
int32_t rxqueue_size = -1;

/************************************************************************
 ************************************************************************/
void bench_common_clear_stats(THROUGHPUT_CXT *p_tCxt)
{
    p_tCxt->pktStats.bytes = 0;
    p_tCxt->pktStats.kbytes = 0;
    p_tCxt->pktStats.sent_bytes = 0;
    p_tCxt->pktStats.pkts_recvd = 0;
}

/************************************************************************
* NAME: check_test_time
*
* DESCRIPTION: If test mode is time, check if current time has exceeded
* test time limit
* Parameters: pointer to throughput context
************************************************************************/
uint32_t bench_common_check_test_time(THROUGHPUT_CXT *p_tCxt)
{
    uint32_t duration;          /* in ticks */
    uint64_t total_interval;    /* in msec */
    uint32_t last_tick = p_tCxt->pktStats.last_time.ticks;
    uint32_t first_tick = p_tCxt->pktStats.first_time.ticks;

    if (last_tick < first_tick)
    {
        /* Assume the systick wraps around once */
        duration = ~first_tick + 1 + last_tick;
    }
    else
    {
        duration = last_tick - first_tick;
    }

    total_interval = duration * qurt_timer_convert_ticks_to_time(1, QURT_TIME_MSEC);

    if (total_interval >= p_tCxt->params.tx_params.tx_time*1000)
        return 1;
    else
        return 0;
}

/*******************************************************************************************************
 *         [0]       [1]         [2]
 * pattern set <pattern_type> <pattern>
 *
 * <pattern_type> 0 use continuous/default pattern 00...FF
 *                1 use static pattern, pattern given by <pattern>
 *                2 use printable ASCII chars(0x20 to 0x7F).
 *
 * <pattern>      Pattern to use in data for TCP/UDP/RAW TX. This is relevant only for
 *                pattern_type=1
 *
 ******************************************************************************************************/
QCLI_Command_Status_t bench_common_set_pattern(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	if (Parameter_Count < 2 || Parameter_Count > 3) {
		return QCLI_STATUS_USAGE_E;
	}

	pattern_type = Parameter_List[1].Integer_Value;

	switch(pattern_type) {
		case 1:
			if (Parameter_Count == 2) {
				QCLI_Printf(qcli_net_handle, "Missing arguments, Provide the pattern. For eg: 0xAA\n");
			}
			else {
				pattern = Parameter_List[2].Integer_Value;
				QCLI_Printf(qcli_net_handle, "Pattern set to %x\n", pattern);
			}
			break;
		case 2:
			QCLI_Printf(qcli_net_handle, "Pattern type set to ASCII chars(0x20-0x7F)\n");
			break;
		default:
			QCLI_Printf(qcli_net_handle, "Pattern type is default(continuous) 00..FF\n");
			break;
	}

	return QCLI_STATUS_SUCCESS_E;
}

/************************************************************************
 * The pattern:
 * < ---          len                       -->
 * 00 01 02 03 .. FE FF 00 01 02 .. FF 00 01 ..
 * A
 * |
 * p
************************************************************************/
void bench_common_add_pattern(char *p, int len)
{
    int n, ml,count;

    if (pattern_type == 1) {
    	while (len--) {
    		*p++ = (char)pattern;
    	}
    }
    else if (pattern_type == 2) {
    	while (len) {
    		ml = min(96, len);
    		for (n = 0x20,count=0; n <=0x7F && count<ml; n++, p++, count++) {
    			*p = (char)n;
    		}
    		len -= ml;
    	}
    }
    else {
		while (len)
		{
			ml = min(256, len);
			for (n = 0; n < ml; ++n, ++p)
			{
				*p = (char)n;
			}
			len -= ml;
		}
    }
}

/************************************************************************
************************************************************************/
void bench_common_print_test_results(THROUGHPUT_CXT *p_tCxt, STATS *pktStats)
{
    /* Print throughput results.*/
    unsigned long long total_bytes = 0;
    uint64_t total_interval;    /* in msec */
    uint32_t duration;          /* in ticks */
    uint32_t ms_per_tick, sec_interval;
    uint32_t throughput;
    uint32_t last_tick = pktStats->last_time.ticks;
    uint32_t first_tick = pktStats->first_time.ticks;

    if (last_tick < first_tick)
    {
        /* Assume the systick wraps around once */
        duration = ~first_tick + 1 + last_tick;
#ifdef BENCH_TEST_RESULT_DEBUG
        QCLI_Printf(qcli_net_handle, "last: 0x%x first: 0x%x duration: 0x%x\n",
                        last_tick, first_tick, duration);
#endif
    }
    else
    {
        duration = last_tick - first_tick; 
    }

    ms_per_tick = qurt_timer_convert_ticks_to_time(1, QURT_TIME_MSEC);
    total_interval = duration * ms_per_tick; 
    sec_interval = duration / (1000/ms_per_tick);

    if (total_interval > 0)
    {
        /*No test was run, or test is terminated, print results of previous test*/
        if (pktStats->bytes == 0)
        {
            total_bytes     = pktStats->last_bytes;
            total_interval  = pktStats->last_interval;
            throughput      = pktStats->last_throughput;
        }
        else
        {
            total_bytes = pktStats->bytes;
            /* Take care of wrap around cases. If number of bytes exceeds
               0x3FFFFFFFFFFFFFLL, it exceeds 64 bits and wraps around
               resulting in wrong throughput number */
            if (total_bytes <= 0x3FFFFFFFFFFFFFLL)
            {
                /* (N/T) bytes/ms = (1000/128)*(N/T) Kbits/s */
                throughput = (total_bytes*125/duration) / (ms_per_tick*16); /* in Kb/sec */
            }
            else
            {
                unsigned long long bytes;
                unsigned long long kbytes;

                /* Convert bytes to kb and divide by seconds for this case */
                kbytes  = total_bytes / 1024;
                bytes   = total_bytes % 1024;
                throughput = ((kbytes*8) / sec_interval) + ((bytes*8/1024) / sec_interval); /* Kb/sec */
            }
            pktStats->last_interval   = total_interval;
            pktStats->last_bytes      = total_bytes;
            pktStats->last_throughput = throughput;
        }
    }
    else
    {
        total_bytes = pktStats->bytes + 1024*pktStats->kbytes;
        throughput = 0;
    }

    QCLI_Printf(qcli_net_handle, "\nResults for %s %s test:\n\n", (p_tCxt->protocol == TCP)? "TCP":((p_tCxt->protocol == UDP)?"UDP": "SSL"),
	    							(p_tCxt->test_type == RX)?"Receive":"Transmit");
    QCLI_Printf(qcli_net_handle, "\t%llu KBytes %llu bytes (%llu bytes) in %u seconds %u ms (%llu miliseconds)\n",
            total_bytes/1024, total_bytes%1024, total_bytes, sec_interval, (uint32_t)(total_interval%1000), total_interval);

    QCLI_Printf(qcli_net_handle, "\n\tThroughput: %u Kbits/sec\n", throughput);
}


/************************************************************************
 ************************************************************************/
static void tx_help(uint32_t v6)
{
    if (v6)
    {
        QCLI_Printf(qcli_net_handle, "benchtx6 <Rx IP> <port> {tcp|tcpzc|udp|udpzc|ssl} <msg size> <mode> <arg> <delay in microseconds between msgs> [<tclass>]\n");
        QCLI_Printf(qcli_net_handle, " <mode> can be 0 or 1.\n");
        QCLI_Printf(qcli_net_handle, " If <mode> is 0, <arg> is time to TX in seconds.\n");
        QCLI_Printf(qcli_net_handle, " If <mode> is 1, <arg> is number of msgs to TX.\n");
        QCLI_Printf(qcli_net_handle, "Examples:\n");
        QCLI_Printf(qcli_net_handle, " benchtx6 fe80::865d:d7ff:fe40:3498%%wlan1 2390 udp 1400 1 100 0 0xA0\n");
        QCLI_Printf(qcli_net_handle, " benchtx6 2001:db8:85a3:8d3:1319:8a2e:370:734 2390 tcpzc 512 0 30 5\n");
    }
    else
    {
        QCLI_Printf(qcli_net_handle, "benchtx <Rx IP> <port> {tcp|tcpzc|udp|udpzc|ssl} <msg size> <mode> <arg> <delay in microseconds between msgs> [<tos>] <source IP>\n");
        QCLI_Printf(qcli_net_handle, "benchtx <Rx IP> <protocol> raw <msg size> <mode> <arg> <delay in microseconds between msgs> [<tos>]\n");
        QCLI_Printf(qcli_net_handle, "benchtx <Rx IP> <protocol> rawh <msg size> <mode> <arg> <delay in microseconds between msgs> <tos> <source IP>\n");
        QCLI_Printf(qcli_net_handle, " <mode> can be 0 or 1.\n");
        QCLI_Printf(qcli_net_handle, " If <mode> is 0, <arg> is time to TX in seconds.\n");
        QCLI_Printf(qcli_net_handle, " If <mode> is 1, <arg> is number of msgs to TX.\n");
        QCLI_Printf(qcli_net_handle, "Examples:\n");
        QCLI_Printf(qcli_net_handle, " benchtx 192.168.1.20 2390 udp 1400 1 100 0 0xA0\n");
        QCLI_Printf(qcli_net_handle, " benchtx 255.255.255.255 5001 udp 1200 0 30 0 0xA0 192.168.1.145\n");
        QCLI_Printf(qcli_net_handle, " benchtx 192.168.1.20 26 raw 1400 0 60 0\n");
        QCLI_Printf(qcli_net_handle, " benchtx 192.168.1.20 26 rawh 1400 1 100 10 0xA0 192.168.1.100\n");
    }
}
uint32_t bench_common_IsTCP(THROUGHPUT_CXT *p_rxtCxt)
{
	return (p_rxtCxt->protocol == TCP);
}

uint32_t bench_common_IsUDP(THROUGHPUT_CXT *p_rxtCxt)
{
	return (p_rxtCxt->protocol == UDP);
}

uint32_t bench_common_IsSSL(THROUGHPUT_CXT *p_rxtCxt)
{
	return (p_rxtCxt->protocol == SSL);
}

void bench_common_SetProtocol(THROUGHPUT_CXT *p_rxtCxt, const char* protocol)
{
	if (strcasecmp("udp", protocol) == 0 || strcasecmp("udpzc", protocol) == 0
	        || strcasecmp("udpecho", protocol) == 0) {
		p_rxtCxt->protocol = UDP;
	}
	else if (strcasecmp("tcp", protocol) == 0 || strcasecmp("tcpzc", protocol) == 0
	        || strcasecmp("tcpecho", protocol) == 0) {
		p_rxtCxt->protocol = TCP;
	}
#ifdef CONFIG_NET_SSL_DEMO
	else if (strcasecmp("ssl", protocol) == 0 || strcasecmp("sslzc", protocol) == 0) {
		p_rxtCxt->protocol = SSL;
	}
#endif
	else if (strcasecmp("raw", protocol) == 0) {
		p_rxtCxt->protocol = IP_RAW;
	}
	else if (strcasecmp("rawh", protocol) == 0) {
		p_rxtCxt->protocol = IP_RAW_HDR;
	}
	else {
	    p_rxtCxt->protocol = ~0; /* Invalid protocol */
	}

	if (strcasecmp("udpzc", protocol) == 0 || strcasecmp("tcpzc", protocol) == 0 || strcasecmp("sslzc", protocol) == 0) {
		p_rxtCxt->zc = 1;
	}

    if (strcasecmp("udpecho", protocol) == 0 || strcasecmp("tcpecho", protocol) == 0) {
        p_rxtCxt->echo = 1;
    }
}

uint32_t bench_common_IsPortInUse(THROUGHPUT_CXT *p_rxtCxt, uint16_t port)
{
	if (bench_common_IsUDP(p_rxtCxt)) {
		if (bench_udp_IsPortInUse(port)) {
			return 1;
		}
	}
	else if (bench_common_IsTCP(p_rxtCxt)) {
		if (bench_tcp_IsPortInUse(port)) {
			return 1;
		}
	}

	return 0;
}

uint32_t bench_common_IsZeroCopy(THROUGHPUT_CXT *p_rxtCxt)
{
	return p_rxtCxt->zc;
}

uint32_t bench_common_SetParams(THROUGHPUT_CXT *p_rxtCxt, uint32_t v6, const char *protocol, uint16_t port, enum test_type type)
{
	bench_common_SetProtocol(p_rxtCxt, protocol);

	if (type == RX) {
		if (bench_common_IsPortInUse(p_rxtCxt, port)) {
			QCLI_Printf(qcli_net_handle, "port %d is in use; use another port.\n", port);
			return QCLI_STATUS_ERROR_E;
		}
	}

	switch(type) {
		case RX:
    	p_rxtCxt->params.rx_params.v6 = v6;
    	p_rxtCxt->params.rx_params.port = port;
		break;
		case TX:
		p_rxtCxt->params.tx_params.v6 = v6;
		p_rxtCxt->params.tx_params.port = port;
		break;
	}
	p_rxtCxt->test_type = type;
	return 0;
}

char* bench_common_GetInterfaceNameFromStr(char *ipstr)
{
	char * interface_name_with_percent_char = strchr(ipstr, '%');
	char * interface_name = NULL;

	if ( interface_name_with_percent_char ) {
		interface_name = interface_name_with_percent_char + 1;
	}
	else {
		QCLI_Printf(qcli_net_handle, "Multicast IPv6 address is used, must append %%interface_name to the address\n");
		QCLI_Printf(qcli_net_handle, "For example: ff02::1%%wlan1 if you intend to listen on a multicast address on interface wlan1\n");
	}
	
	return interface_name;
}

/************************************************************************
 *          [0]            [1]  [2]   [3] [4]  [5] [6] [7]   [8]
 * benchtx  192.168.1.100  2390 tcpzc 500  0   30   0
 * benchtx  192.168.1.100  2390 tcp   500  0   30   0  0xA0
 * benchtx  192.168.1.100  26   raw   500  1   10   0  0xA0
 * benchtx  192.168.1.100  26   rawh  500  1   10   0  0xA0  192.168.1.20
 *
 *          [0]                             [1]  [2] [3]  [4]  [5]    [6]
 * benchtx6 FE80::865D:D7FF:FE40:3498%wlan1 2390 udp 1400  1   10000  10
 ************************************************************************/
static QCLI_Command_Status_t bench_common_tx(THROUGHPUT_CXT *p_tCxt, uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    char *protocol;
    int e = 0;
	uint16_t port = 0;
#ifdef CONFIG_NET_SSL_DEMO
	qapi_Net_SSL_Role_t *ssl_role;
	SSL_INST *ssl_inst;
#endif

    /* TOS */
    if (Parameter_Count >= 8)
    {
        if (!Parameter_List[7].Integer_Is_Valid ||
            Parameter_List[7].Integer_Value > 0xFF || Parameter_List[7].Integer_Value < 0)
        {
            QCLI_Printf(qcli_net_handle, "Incorrect %s %d\n",
                p_tCxt->params.tx_params.v6 ? "TCLASS" : "TOS", Parameter_List[7].Integer_Value);
            return QCLI_STATUS_ERROR_E;
        }
        else
        {
            p_tCxt->params.tx_params.ip_tos = Parameter_List[7].Integer_Value;
        }
    }

    /* Packet size */
    if (!Parameter_List[3].Integer_Is_Valid ||
        (p_tCxt->params.tx_params.packet_size = Parameter_List[3].Integer_Value) == 0
       )
    {
        QCLI_Printf(qcli_net_handle, "ERROR: packet size is 0\n");
        return QCLI_STATUS_ERROR_E;
    }

    /* Test mode */
    if (!Parameter_List[4].Integer_Is_Valid ||
        ((p_tCxt->params.tx_params.test_mode = Parameter_List[4].Integer_Value) != TIME_TEST &&
         p_tCxt->params.tx_params.test_mode != PACKET_TEST))
    {
        QCLI_Printf(qcli_net_handle, "Invalid test mode, please enter 0 for duration test or 1 for packet test\n");
        return QCLI_STATUS_ERROR_E;
    }

    /* Number of packets OR time period */
    if (p_tCxt->params.tx_params.test_mode == TIME_TEST)
    {
        p_tCxt->params.tx_params.tx_time = Parameter_List[5].Integer_Value;
    }
    else if (p_tCxt->params.tx_params.test_mode == PACKET_TEST)
    {
        p_tCxt->params.tx_params.packet_number = Parameter_List[5].Integer_Value;
    }

    /* Inter packet interval for Bandwidth control */
    p_tCxt->params.tx_params.interval_us = Parameter_List[6].Integer_Value;

    /* Port */
    port = Parameter_List[1].Integer_Value;

    benchtx_quit = 0;

    protocol = Parameter_List[2].String_Value;

    bench_common_SetParams(p_tCxt, p_tCxt->params.tx_params.v6, protocol, port, TX);

	if (DUMP_IS_TX_ENABLED)
		p_tCxt->print_buf = 1;

    switch(p_tCxt->protocol) {
	    case TCP:
   		if (bench_common_IsZeroCopy(p_tCxt)) {
        		p_tCxt->params.tx_params.zerocopy_send = 1;
    		}
        	bench_tcp_tx(p_tCxt);
		break;
		case UDP:
   		if (bench_common_IsZeroCopy(p_tCxt)) {
			p_tCxt->params.tx_params.zerocopy_send = 1;
		}

   		if (Parameter_Count == 8 && p_tCxt->params.tx_params.ip_address == 0xFFFFFFFF) {
   			QCLI_Printf(qcli_net_handle, "If dest is broadcast, please specify the src ip\n");
			e = -1;
			goto end;
   		}

   		if (Parameter_Count == 9) {
			if (inet_pton(AF_INET, Parameter_List[8].String_Value, &p_tCxt->params.tx_params.source_ipv4_addr) != 0)
			{
				QCLI_Printf(qcli_net_handle, "Incorrect address %s\n", Parameter_List[8].String_Value);
				e = -1;
				goto end;
			}
   		}
        bench_udp_tx(p_tCxt);
		break;
		case IP_RAW:
		p_tCxt->test_type = TX;
        	e = bench_raw_tx(p_tCxt);
		break;
		case IP_RAW_HDR:
        if (Parameter_Count != 9)
        {
            tx_help(p_tCxt->params.tx_params.v6);
            e = -1;
            goto end;
        }
        else if (inet_pton(AF_INET, Parameter_List[8].String_Value, &p_tCxt->params.tx_params.source_ipv4_addr) != 0)
        {
            QCLI_Printf(qcli_net_handle, "Incorrect address %s\n", Parameter_List[8].String_Value);
            e = -1;
            goto end;
        }
        e = bench_raw_tx(p_tCxt);
	break;
#ifdef CONFIG_NET_SSL_DEMO
		case SSL:
		ssl_role = bench_ssl_GetSSLRole(SSL_CLIENT_INST);
        if (*ssl_role)
        {
            QCLI_Printf(qcli_net_handle, "ERROR: busy.\n");
            e = -1;
            goto end;
        }
		ssl_inst = bench_ssl_GetInstance(SSL_CLIENT_INST);
        if (ssl_inst->sslCtx == QAPI_NET_SSL_INVALID_HANDLE || ssl_inst->role != QAPI_NET_SSL_CLIENT_E)
        {
            QCLI_Printf(qcli_net_handle, "ERROR: SSL client not started (Use 'ssl start client' first).\n");
            e = -1;
            goto end;
        }
        bench_ssl_InitInstance(SSL_CLIENT_INST, QAPI_NET_SSL_CLIENT_E);
        if (bench_ssl_IsDTLS(SSL_CLIENT_INST))
            bench_udp_tx(p_tCxt);
        else
            bench_tcp_tx(p_tCxt);
		bench_ssl_ResetInstance(SSL_CLIENT_INST);
	break;
#endif
   default:
        QCLI_Printf(qcli_net_handle, "Invalid protocol: %s\n", protocol);
        e = -1;
		break;
	}

end:
    if (e != 0)
    {
        return QCLI_STATUS_ERROR_E;
    }
    return QCLI_STATUS_SUCCESS_E;
}

/************************************************************************
 *         [0]           [1]  [2]   [3] [4]  [5] [6]  [7]   [8]
 * benchtx 192.168.1.100 2390 tcpzc 500  0   30   0
 * benchtx 192.168.1.100 2390 tcp   500  0   30   0   0xA0
 * benchtx 192.168.1.100 19   raw   500  0   30   0
 * benchtx 192.168.1.100 19   raw   500  0   30   0   0xA0
 * benchtx 192.168.1.100 19   rawh  500  0   30   0   0xA0  192.168.1.20
 ************************************************************************/
QCLI_Command_Status_t bench_common_tx4(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    THROUGHPUT_CXT tCxt;
    uint32_t v6 = 0;

    if (Parameter_Count < 7 || Parameter_Count > 9 || Parameter_List == NULL)
    {
        tx_help(v6);
        return QCLI_STATUS_SUCCESS_E;
    }

    memset(&tCxt, 0, sizeof(THROUGHPUT_CXT));
    tCxt.params.tx_params.v6 = v6;

    /*Get IPv4 address of Peer (network order)*/
    if (inet_pton(AF_INET, Parameter_List[0].String_Value, &tCxt.params.tx_params.ip_address) != 0)
    {
        QCLI_Printf(qcli_net_handle, "Incorrect address %s\n", Parameter_List[0].String_Value);
        return QCLI_STATUS_ERROR_E;
    }

    return bench_common_tx(&tCxt, Parameter_Count, Parameter_List);
}

/***********************************************************************
 *          [0]                             [1]  [2] [3]  [4] [5]    [6] [7]
 * benchtx6 FE80::865D:D7FF:FE40:3498%wlan1 2390 udp 1400 1   10000  10  0xA0
 ***********************************************************************/
QCLI_Command_Status_t bench_common_tx6(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    THROUGHPUT_CXT tCxt;
    int32_t retval;
    uint32_t v6 = 1;
    char *receiver_ip;

    if (Parameter_Count < 7 || Parameter_Count > 8 || Parameter_List == NULL)
    {
        tx_help(v6);
        return QCLI_STATUS_SUCCESS_E;
    }

    memset(&tCxt, 0, sizeof(THROUGHPUT_CXT));
    tCxt.params.tx_params.v6 = v6;

    /*Get IP address of Peer*/
    receiver_ip = Parameter_List[0].String_Value;
    if ((retval = inet_pton(AF_INET6, receiver_ip, tCxt.params.tx_params.v6addr)) != 0)
    {
        QCLI_Printf(qcli_net_handle, "Incorrect IPv6 address %s\n", receiver_ip);
        return QCLI_STATUS_ERROR_E;
    }

    if (QAPI_IS_IPV6_LINK_LOCAL(tCxt.params.tx_params.v6addr) ||
    		QAPI_IS_IPV6_MULTICAST(tCxt.params.tx_params.v6addr))
    {
        /* if this is a link local address, then the interface must be specified after % */
        char * interface_name = bench_common_GetInterfaceNameFromStr(receiver_ip);
        if (!interface_name) {
            return QCLI_STATUS_ERROR_E;
        }

        retval = qapi_Net_IPv6_Get_Scope_ID(interface_name, &tCxt.params.tx_params.scope_id);
        if ( retval != 0 ) {
            QCLI_Printf(qcli_net_handle, "Failed to get scope id for the interface %s\n", interface_name);
            return QCLI_STATUS_ERROR_E;
        }
    }

    return bench_common_tx(&tCxt, Parameter_Count, Parameter_List);
}

/************************************************************************
 ************************************************************************/
static void rx_help(uint32_t v6)
{
    if (v6)
    {
        QCLI_Printf(qcli_net_handle, "benchrx6 {udp|udpzc|udpecho|tcp|tcpzc|tcpecho|ssl|sslzc} <port>\n");
        QCLI_Printf(qcli_net_handle, "benchrx6 {udp|udpzc} <port> [<multicast_ip%%if_name>]\n");
        QCLI_Printf(qcli_net_handle, "Examples:\n");
        QCLI_Printf(qcli_net_handle, " benchrx6 tcp 2390\n");
        QCLI_Printf(qcli_net_handle, " benchrx6 udp 2390 ff02::1:2%%wlan1\n");
    }
    else
    {
        QCLI_Printf(qcli_net_handle, "benchrx {tcp|tcpzc|tcpecho|ssl|sslzc} <port> [<local_ip>]\n");
        QCLI_Printf(qcli_net_handle, "benchrx {udp|udpzc|udpecho} <port> [<local_ip>|<multicast_ip>|<multicast_ip%%if_name>|<multicast_ip> <local_ip>]\n");
        QCLI_Printf(qcli_net_handle, "benchrx {raw|rawh} <protocol> [<local_ip>|<multicast_ip>|<multicast_ip%%if_name>|<multicast_ip> <local_ip>]\n");
        QCLI_Printf(qcli_net_handle, "Examples:\n");
        QCLI_Printf(qcli_net_handle, " benchrx tcp 2390\n");
        QCLI_Printf(qcli_net_handle, " benchrx tcpzc 5000 192.168.1.100\n");
        QCLI_Printf(qcli_net_handle, " benchrx udp 6000 224.1.1.100%%wlan1\n");
        QCLI_Printf(qcli_net_handle, " benchrx udp 7000 224.1.1.100 192.168.1.100\n");
        QCLI_Printf(qcli_net_handle, " benchrx rawh 26\n");
    }
}

int bench_common_SetMCParams(THROUGHPUT_CXT *p_rxtCxt, char *remoteIP, char *localIP, int v6)
 {
	if (v6) {
		/*          [0]          [1]    [2]
		 * benchrx6 {udp|udpzc}  <port> [<multicast_ip%ifname>]
		 */

		/* IPv6 address: verify mcast address to be IPv6 */
		if (inet_pton(AF_INET6, remoteIP, p_rxtCxt->params.rx_params.mcIpv6addr)
				!= 0||
				!QAPI_IS_IPV6_MULTICAST(p_rxtCxt->params.rx_params.mcIpv6addr)) {
			QCLI_Printf(qcli_net_handle,
					"Incorrect IPv6 multicast address %s\n", remoteIP);
			return -1;
		}

		/* Check if the interface name is appended (must be specified after %) */
		char *interface_name = bench_common_GetInterfaceNameFromStr(remoteIP);
		if (!interface_name) {
			return -1;
		}

		int retval = qapi_Net_IPv6_Get_Scope_ID(interface_name,
				&p_rxtCxt->params.rx_params.scope_id);
		if (retval != 0) {
			QCLI_Printf(qcli_net_handle,
					"Failed to get scope id for the interface %s\n",
					interface_name);
			return -3;
		}
		p_rxtCxt->params.rx_params.mcEnabled = 1;
	} else {
		/* For ipv4 we bind the socket to a local_ip if local_ip is provided
		 * else we bind to INADDR_ANY */
		/*          [0]         [1]      [2]             [3]
		 * benchrx  {udp|udpzc} <port>  [<multicast_ip>  <local_ip>]
		 */
		if (localIP) {
			if (inet_pton(AF_INET, remoteIP,
					&p_rxtCxt->params.rx_params.mcIpaddr) != 0||
			!QAPI_IPV4_IS_MULTICAST(p_rxtCxt->params.rx_params.mcIpaddr)) {
				QCLI_Printf(qcli_net_handle,
						"Incorrect IPv4 multicast address %s\n", remoteIP);
				return -1;
			}

			p_rxtCxt->params.rx_params.mcEnabled = 1;
			if (inet_pton(AF_INET, localIP,
					&p_rxtCxt->params.rx_params.local_address) != 0) {
				QCLI_Printf(qcli_net_handle, "Incorrect IPV4 address %s\n",
						localIP);
				return -2;
			}
		} else {
			/*          [0]         [1]      [2]
			 * benchrx  {udp|udpzc} <port>  [<multicast IP%if_name>]
			 */
			char *temp_address_str = remoteIP;
			char *address_str;
			uint32_t address;

			if (strchr(temp_address_str, '%')) {
				address_str = strtok(temp_address_str, "%");

				if (inet_pton(AF_INET, address_str, &address) != 0) {
					QCLI_Printf(qcli_net_handle, "Incorrect IPV4 address %s\n",
							address_str);
					return -2;
				}

				if (QAPI_IPV4_IS_MULTICAST(address)) {
					/* Get the interface specified after % */
					char * interface_name = strtok(NULL, "%");

					uint32_t addr;
					if (qapi_Net_IPv4_Config(interface_name,
							QAPI_NET_IPV4CFG_QUERY_E, &addr, NULL, NULL) != 0) {
						QCLI_Printf(qcli_net_handle,
								"Invalid interface name %s\n", interface_name);
						return -2;
					}

					p_rxtCxt->params.rx_params.mcIpaddr = address;
					p_rxtCxt->params.rx_params.mcRcvIf = addr;
					p_rxtCxt->params.rx_params.mcEnabled = 1;
				} else {
					QCLI_Printf(qcli_net_handle, "Invalid Multicast IP %s\n",
							address_str);
					return -4;
				}
			}
			/*          [0]                             [1]      [2]
			 * benchrx  {tcp|tcpzc|udp|udpzc|ssl|sslzc} <port>  [<local_ip>]
			 * benchrx  {udp|udpzc}                     <port>  [<multicast_ip>]
			 */
			else {
				address_str = temp_address_str;
				if (inet_pton(AF_INET, address_str, &address) != 0) {
					QCLI_Printf(qcli_net_handle, "Incorrect IPV4 address %s\n",
							localIP);
					return -2;
				}

				if (QAPI_IPV4_IS_MULTICAST(address)) {
					p_rxtCxt->params.rx_params.mcIpaddr = address;
					p_rxtCxt->params.rx_params.mcEnabled = 1;
				} else if (address != 0 && address != 0xFFFFFFFF) { /* not zero address or broadcast */
					p_rxtCxt->params.rx_params.local_address = address;
				} else {
					QCLI_Printf(qcli_net_handle, "Invalid IP: %s\n",
							address_str);
					return -5;
				}
			}
		}
	}

	return 0;
}

/************************************************************************
 *          [0]                             [1]    [2]                       [3]
 * benchrx  {tcp|tcpzc|udp|udpzc|ssl|sslzc} <port> [<local_ip>]
 * benchrx  {udp|udpzc}                     <port> [<multicast_ip>]
 * benchrx  {udp|udpzc}                     <port> [<multicast_ip%if_name>]
 * benchrx  {udp|udpzc}                     <port> [<multicast_ip>           <local_ip>]
 * benchrx  {tcp}                           {servers}
 *
 * benchrx6 {tcp|tcpzc|udp|udpzc|ssl|sslzc} <port>
 * benchrx6 {udp|udpzc}                     <port> [<multicast_ip%if_name>]
 * benchrx6 {tcp}                           {servers}
 ************************************************************************/
static QCLI_Command_Status_t bench_common_rx(uint32_t v6, uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	char *protocol;
	int e = 0;
	char *receiver_ip;
	char *local_ip = NULL;
	uint16_t port;
	THROUGHPUT_CXT *ctxt=NULL;
#ifdef CONFIG_NET_SSL_DEMO
	int proto;
#endif

	if (Parameter_Count < 2 || Parameter_Count > 4 || Parameter_List == NULL) {
		rx_help(v6);
		return QCLI_STATUS_SUCCESS_E;
	}

	if (Parameter_Count == 2 && (strcmp(Parameter_List[1].String_Value, "servers") == 0)) {
		bench_tcp_rx_dump_servers();
		return QCLI_STATUS_SUCCESS_E;
	}

	protocol = Parameter_List[0].String_Value;
	port = Parameter_List[1].Integer_Value;

	if (Parameter_Count >= 3) {
		receiver_ip = Parameter_List[2].String_Value;
	}

	if (Parameter_Count == 4) {
		local_ip = Parameter_List[3].String_Value;
	}

	/* Allocate Rx context */
	ctxt = (THROUGHPUT_CXT *)malloc(sizeof(THROUGHPUT_CXT));
	if (ctxt == NULL) {
		QCLI_Printf(qcli_net_handle, "No memory\n");
		return QCLI_STATUS_ERROR_E;
	}
	memset(ctxt, 0, sizeof(THROUGHPUT_CXT));

	if (bench_common_SetParams(ctxt, v6, protocol, port, RX)) {
		e = -1;
		goto end;
	}

	if (DUMP_IS_RX_ENABLED)
		ctxt->print_buf = 1;

	benchrx_quit = 0;

	if (Parameter_Count >= 3) {
		 if (bench_common_SetMCParams(ctxt,  receiver_ip, local_ip, v6) < 0) {
             bench_udp_rx_port_in_use = 0;
			 return QCLI_STATUS_ERROR_E;
		 }
	}

	switch(ctxt->protocol) {
	case TCP:
	if (bench_common_IsZeroCopy(ctxt)) {
		bench_tcp_rx_zc(ctxt);
	}
	else {
		bench_tcp_rx(ctxt);
	}
	break;
	case UDP:
	if (bench_common_IsZeroCopy(ctxt)) {
		bench_udp_rx_zc(ctxt);
	}
	else {
		bench_udp_rx(ctxt);
	}
	break;
#ifdef CONFIG_NET_SSL_DEMO
	case SSL:
	bench_ssl_InitInstance(SSL_SERVER_INST, QAPI_NET_SSL_SERVER_E);
	proto = bench_ssl_GetProtocol();
	if (proto < 0) {
		e = -3;
		goto end;
	}

	if (proto == TCP) {
		if (bench_common_IsZeroCopy(ctxt)) {
			bench_tcp_rx_zc(ctxt);
		}
		else {
			bench_tcp_rx(ctxt);
		}
	}
	else {
		if (bench_common_IsZeroCopy(ctxt)) {
			bench_udp_rx_zc(ctxt);
		}
		else {
			bench_udp_rx(ctxt);
		}
	}

	bench_ssl_ResetInstance(SSL_SERVER_INST);
	break;
#endif
	case IP_RAW:
	case IP_RAW_HDR:
	ctxt->test_type = RX;
	e = bench_raw_rx(ctxt);
	break;
	default:
	QCLI_Printf(qcli_net_handle, "Invalid protocol: %s\n", protocol);
	break;
	}

	// Add protocol type RAW and RAW_H.

	end:
	if (ctxt) {
		free(ctxt);
		ctxt = NULL;
	}

	if (e) {
		return QCLI_STATUS_ERROR_E;
	}

	return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t bench_common_rx4(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    return bench_common_rx(0, Parameter_Count, Parameter_List);
}

QCLI_Command_Status_t bench_common_rx6(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    return bench_common_rx(1, Parameter_Count, Parameter_List);
}

/************************************************************************
 *            [0]          [1]
 * benchquit [rx | tx] [session id]
 ************************************************************************/
QCLI_Command_Status_t benchquit(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	int serverId = 0;

    if (Parameter_Count == 0)
    {
        benchtx_quit = 1;
        benchrx_quit = 1;
        return QCLI_STATUS_SUCCESS_E;
    }

    if (Parameter_Count > 3)
    {
        return QCLI_STATUS_USAGE_E;
    }

    if (strcasecmp("rx", Parameter_List[0].String_Value) == 0)
    {
        if (Parameter_Count < 2)
        {
            return QCLI_STATUS_USAGE_E;
        }

        serverId = Parameter_List[1].Integer_Value;
    	bench_tcp_rx_quit(serverId);
    }
    else if (strcasecmp("tx", Parameter_List[0].String_Value) == 0)
    {
        benchtx_quit = 1;
    }
    else
    {
        return QCLI_STATUS_ERROR_E;
    }

    return QCLI_STATUS_SUCCESS_E;
}

/************************************************************************
 ************************************************************************/
int32_t g_cookie_uc = 1234;
int32_t g_cookie_mc = 4321;

#define UINT32MAX   (0xffffffff)
#ifndef min
#define  min(a,b)    (((a) <= (b)) ? (a) : (b))
#endif

unsigned short ratio(uint32_t numerator, uint32_t denominator, unsigned short base)
{
    unsigned short ret = 0;
    if (base) {
        if (denominator) {
            if (numerator) {
                unsigned short multiplier = min(UINT32MAX/numerator, base);
                ret = ((numerator*multiplier)/denominator)*(base/multiplier);
            } else {
                ret = 0;
            }
        } else {
            QCLI_Printf(qcli_net_handle, "warning, denominator=%d\n", denominator);
        }
    } else {
        QCLI_Printf(qcli_net_handle, "warning, base=%d not supported\n", base);
    }
    return ret;
}

/************************************************************************
* NAME: send_ack
*
* DESCRIPTION:
* In the UDP receive test, the test is terminated on receiving
* an 8-byte endMark UDP packet with the 1st word in the payload = 0xaabbccdd.
* We have implemented a feedback mechanism where we (the server) will ack
* the endMark with a packet containing RX stats allowing client to display
* correct test results. The Ack packet will contain time duration and number
* of bytes received.
* Implementation details-
* 1. Client sends endMark packet, then waits for 500 ms for a response.
* 2. We, on receiving endMark, send ACK (containing RX stats), and waits
*    1000 ms to check for more incoming packets.
* 3. If the client receives this ACK, it will stop sending endMark packets.
* 4. If we do not see the endMark packet for 1000 ms, we will assume SUCCESS
*    and exit gracefully.
* 5. Each side makes 40 attempts.
************************************************************************/
#define MAX_ACK_RETRY   40
void send_ack(THROUGHPUT_CXT *p_tCxt, struct sockaddr *faddr, int addrlen)
{
    int send_result;
    uint32_t received;
    uint32_t retry = MAX_ACK_RETRY;

    stat_packet_t *stat;
    char pktbuf[ sizeof(stat_packet_t) + sizeof(ipv4_header_t)];
    int pktlen;

    uint32_t last_time_ms   = (p_tCxt->pktStats.last_time.seconds * 1000 + p_tCxt->pktStats.last_time.milliseconds);
    uint32_t first_time_ms  = (p_tCxt->pktStats.first_time.seconds * 1000 + p_tCxt->pktStats.first_time.milliseconds);
    uint32_t total_interval = last_time_ms - first_time_ms;
#ifdef CONFIG_NET_SSL_DEMO
    SSL_INST *ssl = &ssl_inst[SSL_SERVER_INST];
#endif
    uint16_t port_save = 0;

    stat = (stat_packet_t *)pktbuf;
    pktlen = sizeof(stat_packet_t);

    if (!(p_tCxt->test_type == RX && (p_tCxt->protocol == IP_RAW ||
        p_tCxt->protocol == IP_RAW_HDR || p_tCxt->protocol == SSL)))
    {
        /* Change client's rx port to be the same as our rx port
         * because client will receive the ACK from us on this port.
         */
        port_save = faddr->sa_port;
        faddr->sa_port = htons(p_tCxt->params.rx_params.port);
    }
    else /* IP_RAW_RX || IP_RAW_RX_HDR */
    {
        if (p_tCxt->test_type == RX && p_tCxt->protocol == IP_RAW_HDR)
        {
            if (p_tCxt->params.rx_params.local_address != 0)
            {
                ipv4_header_t *iphdr = (ipv4_header_t *)pktbuf;

                /* Reset stat pointer and pktlen */
                stat = (stat_packet_t *)&pktbuf[sizeof(ipv4_header_t)];
                pktlen += sizeof(ipv4_header_t);

                /* Add IPv4 header */
                iphdr->ver_ihl = 0x45; /* ver: IPv4, IHL=20 bytes */
                iphdr->tos = 0;
                iphdr->len = htons(pktlen);
                iphdr->id = 0;
                iphdr->flags_offset = 0;
                iphdr->ttl = 255;
                iphdr->protocol = (uint8_t)p_tCxt->params.rx_params.port;
                iphdr->hdr_chksum = 0;
                iphdr->sourceip = p_tCxt->params.rx_params.local_address;  /* already in net order */
                iphdr->destip   = ((struct sockaddr_in *)faddr)->sin_addr.s_addr;  /* already in net order */
            }
            else
            {
                /* Turn off IP_HDRINCL so the stack can fill the source IP in the header */
                int off = 0;
                qapi_setsockopt(p_tCxt->sock_local, IPPROTO_IP, IP_HDRINCL, &off, sizeof(off));
            }
        }
    }

    stat->kbytes      = p_tCxt->pktStats.bytes/1024;
    stat->bytes       = p_tCxt->pktStats.bytes;
    stat->msec        = total_interval;
    stat->numPackets  = p_tCxt->pktStats.pkts_recvd;

    while (retry)
    {
#ifdef CONFIG_NET_SSL_DEMO
        /* send ACK to client */
        if (p_tCxt->protocol == SSL && p_tCxt->test_type == RX)
        {
            send_result = qapi_Net_SSL_Write_To(ssl->ssl, (char*)stat, pktlen, faddr, addrlen);
            if(send_result == QAPI_ERR_SSL_CONN_NOT_FOUND) {
            	QCLI_Printf(qcli_net_handle, "SSL connection closed by peer\n");
            	retry = 0;
            	break;
            }
        }
        else
#endif
        {
            send_result = qapi_sendto(p_tCxt->sock_local, pktbuf, pktlen, 0, faddr, addrlen);
        }

        if (send_result < 0)
        {
        	uint32_t counter = 0;

        	do
        	{
				received = qapi_recvfrom(p_tCxt->sock_local, (char*)(&p_tCxt->buffer[0]), CFG_PACKET_SIZE_MAX_RX, MSG_DONTWAIT, NULL, NULL);
				counter++;
        	}
			while (received > 0 || counter < 10);

        	if (received > 0)
        	{
        		/* Peer continues to send data even after EOT, need to abort */
        		retry = 0;
        		break;
        	}
        }

        if ( send_result != pktlen )
        {
            QCLI_Printf(qcli_net_handle, "Error while sending stat packet, e=%d\n", send_result);
            app_msec_delay(1);
        }
        else /* Sending ACK is successful.  Now waiting 1000 ms for more endmark packets from client. */
        {
            int32_t conn_sock;
            fd_set rset;
#ifdef SEND_ACK_DEBUG
            QCLI_Printf(qcli_net_handle, "%d sent ACK\n", retry);
#endif
            FD_ZERO(&rset);
            FD_SET(p_tCxt->sock_local, &rset);

            conn_sock = qapi_select(&rset, NULL, NULL, 1000);
            if (conn_sock > 0)
            {
#ifdef CONFIG_NET_SSL_DEMO
                if (p_tCxt->protocol == SSL && p_tCxt->test_type == RX)
                {
                    received = qapi_Net_SSL_Read(ssl->ssl, (char*)(&p_tCxt->buffer[0]), CFG_PACKET_SIZE_MAX_RX);
                }
                else
#endif
                {
                    received = qapi_recvfrom(p_tCxt->sock_local, (char*)(&p_tCxt->buffer[0]), CFG_PACKET_SIZE_MAX_RX, 0, NULL, NULL);
                }

                if (received > 0)
                {
                    QCLI_Printf(qcli_net_handle, "ACK success (%u)\n", MAX_ACK_RETRY - retry);
                    break;
                }
#ifdef SEND_ACK_DEBUG
                QCLI_Printf(qcli_net_handle, " received %d\n", received);
#endif
            }
            else
            {
            	/* ACK dropped or lost, peer does not retry */
            	break;
            }
        }
        retry--;
    } /* while */

    if (retry == 0)
    {
        QCLI_Printf(qcli_net_handle, "Did not receive ACK.\n");
    }

    if (!(p_tCxt->test_type == RX && (p_tCxt->protocol == IP_RAW ||
        p_tCxt->protocol != IP_RAW_HDR)))
    {
        faddr->sa_port = port_save;
    }

    if (p_tCxt->protocol == IP_RAW_HDR && p_tCxt->test_type == RX &&
        p_tCxt->params.rx_params.local_address == 0)
    {
        /* Turn on IP_HDRINCL */
        int on = 1;
        qapi_setsockopt(p_tCxt->sock_local, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on));
    }

    return;
}

/************************************************************************
 ************************************************************************/
void send_ack_zc(THROUGHPUT_CXT *p_tCxt, struct sockaddr *faddr, uint32_t addrlen)
{
    uint32_t retry = MAX_ACK_RETRY;
    PACKET pkt;
    stat_packet_t *stat;
    uint32_t last_time_ms   = (p_tCxt->pktStats.last_time.seconds * 1000 + p_tCxt->pktStats.last_time.milliseconds);
    uint32_t first_time_ms  = (p_tCxt->pktStats.first_time.seconds * 1000 + p_tCxt->pktStats.first_time.milliseconds);
    uint32_t total_interval = last_time_ms - first_time_ms;
    uint32_t kbytes = p_tCxt->pktStats.bytes/1024;
    int32_t result;
    int32_t conn_sock;
    fd_set rset;
    UDP_ZC_RX_INFO *p;
#ifdef CONFIG_NET_SSL_DEMO
    SSL_INST *ssl = bench_ssl_GetInstance(SSL_SERVER_INST);
#endif
    uint16_t port_save;

    /* Change client's rx port to be the same as our rx port
     * because client will receive the ACK from us on this port.
     */
    port_save = faddr->sa_port;
    faddr->sa_port = htons(p_tCxt->params.rx_params.port);

    /* send stats pkt to client */
    while (retry)
    {
        if ((pkt = qapi_Net_Buf_Alloc(sizeof(stat_packet_t), QAPI_NETBUF_SYS)) == NULL)
        {
            QCLI_Printf(qcli_net_handle, "alloc failure\n");
            break;
        }

        stat = (stat_packet_t *)pkt->nb_Prot;
        stat->bytes         = p_tCxt->pktStats.bytes;
        stat->kbytes        = kbytes;
        stat->msec          = total_interval;
        stat->numPackets    = p_tCxt->pktStats.pkts_recvd;
        pkt->nb_Plen = pkt->nb_Tlen = sizeof(stat_packet_t);

#ifdef CONFIG_NET_SSL_DEMO
        if (p_tCxt->protocol == SSL && p_tCxt->test_type == RX)
        {
            result = qapi_Net_SSL_Write_To(ssl->ssl, (char*)pkt, sizeof(stat_packet_t), faddr, addrlen);
        }
        else
#endif
        {
            result = qapi_sendto(p_tCxt->sock_local, (char*)pkt, sizeof(stat_packet_t),
                    0, faddr, addrlen);
        }

        if ( result == A_ERROR )
        {
            QCLI_Printf(qcli_net_handle, "error while sending stat packet\n");
            break;
        }
        else /* Sending ACK is successful.  Now waiting 1000 ms for more endmark packets from client. */
        {
#ifdef SEND_ACK_DEBUG
            QCLI_Printf(qcli_net_handle, "%d sent ACK\n", retry);
#endif
            qapi_fd_zero(&rset);
            qapi_fd_set(p_tCxt->sock_local, &rset);
            conn_sock = qapi_select(&rset, NULL, NULL, 1000);
            if (conn_sock > 0)
            {
                while ((p = (UDP_ZC_RX_INFO *)dequeue(&udp_zcq)) != NULL)
                {
#ifdef SEND_ACK_DEBUG
                    QCLI_Printf(qcli_net_handle, "received %d\n", ((PACKET)p->pkt)->nb_tlen);
#endif
                    qapi_Net_Buf_Free(p->pkt, QAPI_NETBUF_SYS);
                    qapi_Net_Buf_Free(p, QAPI_NETBUF_APP);
                }
            }
            else if (retry != 0)
            {
                QCLI_Printf(qcli_net_handle, "ACK success (%u)\n", MAX_ACK_RETRY - retry);
                break;
            }
        }
        retry--;
    } /*while(retry)*/

    if (retry == 0)
    {
        QCLI_Printf(qcli_net_handle, "Did not receive ACK.\n");
    }

    faddr->sa_port = port_save;

    return;
}

void bench_print_spaces_internal(QCLI_Group_Handle_t handle, uint32_t spaces)
{
	uint32_t i;

	for (i = 0; i < spaces; i++)
	{
		QCLI_Printf(handle, " ");
	}
}

#define bench_isprint(__c) (__c>=32 && __c<=126)

void bench_print_ascii_internal(QCLI_Group_Handle_t handle, const char *buf, uint32_t len)
{
	uint32_t i;

	for (i = 0; i < len; i++)
	{
		int c = (int)buf[i];
		if(bench_isprint(c))
		{
			QCLI_Printf(handle, "%c", c);
		}
		else
		{
			QCLI_Printf(handle, ".");
		}
	}
}

void bench_print_buffer(const char *buf, uint32_t len, struct sockaddr *sock_addr, uint8_t direction)
{
	#define PRINT_ELEMENTS_PER_LINE 16
	#define PRINT_CHARS_PER_ELEMENT 3
	#define PRINT_TOTAL_SPACES PRINT_CHARS_PER_ELEMENT*(PRINT_ELEMENTS_PER_LINE)+1+3

	uint32_t i, j = PRINT_ELEMENTS_PER_LINE;
	uint32_t spaces = PRINT_TOTAL_SPACES;
	uint32_t print_offset = 0;
	const QCLI_Command_Group_t nettmp_cmd_group =
	{
	    "",              /* Group_String: will display cmd prompt as "Net> " */
	    0,   /* Command_Count */
	    NULL        /* Command_List */
	};

	QCLI_Group_Handle_t handle2;
    handle2 = QCLI_Register_Command_Group(NULL, &nettmp_cmd_group);

    QCLI_Group_Handle_t handle = qcli_net_handle;

    if(!buf)
    {
    	QCLI_Printf(qcli_net_handle, "Buffer error\n");
    	return;
    }

    if(!handle)
    {
    	QCLI_Printf(qcli_net_handle, "Handle error\n");
    	return;
    }

    if (sock_addr && (sock_addr->sa_family == AF_INET || sock_addr->sa_family == AF_INET6)
    		&& sock_addr->sa_port > 0)
    {
    	char ip_str[48];
    	void *sin_addr;

    	if (sock_addr->sa_family == AF_INET)
    	{
    		sin_addr = &sock_addr->u.sin.sin_addr;
    	}
    	else
    	{
    		sin_addr = &sock_addr->u.sin6.sin_addr;
    	}

    	inet_ntop(sock_addr->sa_family, sin_addr, ip_str, sizeof(ip_str));
    	QCLI_Printf(handle, "%s %d bytes %s %s:%d\n", direction ? "<==" : "==>",
    			len, direction ? "from" : "to", ip_str, htons(sock_addr->sa_port));
    }

    QCLI_Printf(handle, "\n");

	for (i = 0; i < len; i++)
	{
		uint32_t val = (uint32_t)buf[i];
		QCLI_Printf(handle, "%02x ", val);
		j--;
		spaces -= PRINT_CHARS_PER_ELEMENT;

		if (j == PRINT_ELEMENTS_PER_LINE/2)
		{
			QCLI_Printf(handle, " ");
			spaces--;
		}
		else if (j == 0)
		{
			bench_print_spaces_internal(handle, spaces);
			bench_print_ascii_internal(handle, &buf[print_offset], PRINT_ELEMENTS_PER_LINE);
			spaces = PRINT_TOTAL_SPACES;
			j = PRINT_ELEMENTS_PER_LINE;
			print_offset += PRINT_ELEMENTS_PER_LINE;
			QCLI_Printf(handle, "\n");
		}
	}

	if (j != PRINT_ELEMENTS_PER_LINE)
	{
		bench_print_spaces_internal(handle, spaces);
		bench_print_ascii_internal(handle, buf + print_offset, PRINT_ELEMENTS_PER_LINE - j);
	}

	QCLI_Printf(handle, "\n");
	QCLI_Unregister_Command_Group(handle2);
}

/************************************************************************
            [0]          [1]
 * queuecfg [rx | tx] [size]
 ************************************************************************/
QCLI_Command_Status_t queuecfg(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if (Parameter_Count > 2)
    {
        return QCLI_STATUS_USAGE_E;
    }

    if (strcasecmp("rx", Parameter_List[0].String_Value) == 0)
    {
        if (Parameter_Count < 2)
        {
            return QCLI_STATUS_USAGE_E;
        }

        rxqueue_size = Parameter_List[1].Integer_Value;

        if(rxqueue_size <= 0)
        {
            rxqueue_size = -1;
            QCLI_Printf(qcli_net_handle, "RX queue size: DEFAULT\n");
        }
        else
        {
            QCLI_Printf(qcli_net_handle, "RX queue size: %d\n", rxqueue_size);
        }
    }
    else if (strcasecmp("tx", Parameter_List[0].String_Value) == 0)
    {
        if (Parameter_Count < 2)
        {
            return QCLI_STATUS_USAGE_E;
        }

        txqueue_size = Parameter_List[1].Integer_Value;

        if(txqueue_size <= 0)
        {
            txqueue_size = -1;
            QCLI_Printf(qcli_net_handle, "TX queue size: DEFAULT\n");
        }
        else
        {
            QCLI_Printf(qcli_net_handle, "TX queue size: %d\n", txqueue_size);
        }
    }
    else
    {
        return QCLI_STATUS_USAGE_E;
    }

    QCLI_Printf(qcli_net_handle, "Restart benchmarking threads to apply changes.\n");
    return QCLI_STATUS_SUCCESS_E;
}

void bench_config_queue_size(int32_t sock)
{
    if(rxqueue_size > 0)
        qapi_setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &rxqueue_size, sizeof(int32_t));

    if(txqueue_size > 0)
        qapi_setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &txqueue_size, sizeof(int32_t));
}

#endif
