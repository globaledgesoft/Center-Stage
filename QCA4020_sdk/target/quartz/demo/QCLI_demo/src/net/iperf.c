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

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "qapi_status.h"
#include "iperf.h"
#include "bench.h"

#ifdef CONFIG_NET_TXRX_DEMO
extern QCLI_Group_Handle_t qcli_net_handle; /* Handle for Net Command Group. */
extern uint8_t benchtx_quit;
extern uint8_t benchrx_quit;

#define IPERF_PRINTF(...) QCLI_Printf(qcli_net_handle, __VA_ARGS__)

static void iperf3_make_cookie(char *str, int len);
static void iperf3_tx_reverse(THROUGHPUT_CXT *p_tCxt);


/***************************************************************************************
 * 
 * iperf function
 *
 **************************************************************************************/
QCLI_Command_Status_t iperf(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    unsigned int protocol = TCP;
    unsigned int port = IPERF_DEFAULT_PORT;
    unsigned int port_tag = 0;
    unsigned int seconds = IPERF_DEFAULT_RUNTIME;
    unsigned int pktSize = 0;
    int operation_mode = -1;
    int reverse_mode = 0;
    unsigned int udpRate = IPERF_DEFAULT_UDP_RATE;
    unsigned short mcastEnabled = 0;

    unsigned int ipAddress = 0;
    unsigned int numOfPkts = 0;
    unsigned int interval = 0;
    unsigned int index = 0;

    THROUGHPUT_CXT tCxt;
    THROUGHPUT_CXT rCxt;
    uint32_t v6 = 0;
    char *receiver_ip;

    memset(&tCxt, 0, sizeof(THROUGHPUT_CXT));
    memset(&rCxt, 0, sizeof(THROUGHPUT_CXT));

    index = 0;

    if(Parameter_Count < 1)
    {
        IPERF_PRINTF("\nUsage: iperf [-s|-c host] [options]\n");
        IPERF_PRINTF("  -p  = The server port for the server to listen on and the client to connect to. This should be the same in both client and server. Default is 5001.\n");
        IPERF_PRINTF("  -i  = Sets the interval time in seconds between periodic bandwidth, jitter, and loss reports\n");
        IPERF_PRINTF("  -u  = Use UDP rather than TCP\n");
        IPERF_PRINTF("  -l = The length of buffers to read or write\n");
        IPERF_PRINTF("  -t = The time in seconds to transmit for\n");
        IPERF_PRINTF("  -n = The number of buffers to transmit\n");
        IPERF_PRINTF("  -b = Set target bandwidth to n bits/sec\n");
        IPERF_PRINTF("  -r = Client Reserve Mode\n");     
        IPERF_PRINTF("  -V = IPV6\n");

        return QCLI_STATUS_ERROR_E;
    }


    while (index < Parameter_Count)
    {
        
         if (0 == strcmp(Parameter_List[index].String_Value, "-u"))
         {
                index++;
                protocol = UDP;
         }
         else if (0 == strcmp(Parameter_List[index].String_Value, "-s"))
         {
                index++;
                operation_mode = IPERF_SERVER;
         }
         else if (0 == strcmp(Parameter_List[index].String_Value, "-c"))
         {
                index++;
                operation_mode = IPERF_CLIENT;

                if(inet_pton(AF_INET, Parameter_List[index].String_Value, &ipAddress) != 0) 
                {

                    /* check if it's IPV6 */
                    receiver_ip = Parameter_List[index].String_Value;
                    if (inet_pton(AF_INET6, receiver_ip, tCxt.params.tx_params.v6addr) != 0)
                    {
                        IPERF_PRINTF("Incorrect IP address %s\n", receiver_ip);
                        return QCLI_STATUS_ERROR_E;
                    }
                    else
                    {
                        /* is valid IPV6*/
                        if (QAPI_IS_IPV6_LINK_LOCAL(tCxt.params.tx_params.v6addr) ||
                            QAPI_IS_IPV6_MULTICAST(tCxt.params.tx_params.v6addr))
                        {
                            /* if this is a link local address, then the interface must be specified after % */

                            char * interface_name = (char*) bench_common_GetInterfaceNameFromStr(receiver_ip);
                            if (!interface_name) 
                            {
                                IPERF_PRINTF("this is a link local address, then the interface must be specified after %\n", receiver_ip);
                                return QCLI_STATUS_ERROR_E;
                            }

                            if (qapi_Net_IPv6_Get_Scope_ID(interface_name, &tCxt.params.tx_params.scope_id) != 0)
                            {
                                IPERF_PRINTF("Failed to get scope id for the interface %s\n", interface_name);
                                return QCLI_STATUS_ERROR_E;                           
                            }
                        }
                    }
                }
                else
                {
                    /* is valid IPV4 */
                    if ((ipAddress & 0xf0000000) == 0xE0000000) //224.xxx.xxx.xxx - 239.xxx.xxx.xxx
                    {
                        mcastEnabled =1;
                    }
                }
                index++;

         }
         else if (0 == strcmp(Parameter_List[index].String_Value, "-p"))
         {
                index++;
                port = Parameter_List[index].Integer_Value;
                port_tag = 1;
                index++;
                
                if(port > 64*1024)
                {
                    IPERF_PRINTF("error: invalid port\n");
                    return QCLI_STATUS_ERROR_E;
                }
         }
         else if (0 == strcmp(Parameter_List[index].String_Value, "-i"))
         {
                index++;
                interval = Parameter_List[index].Integer_Value;
                index++;
         }
         else if (0 == strcmp(Parameter_List[index].String_Value, "-l"))
         {
                index++;
                pktSize = Parameter_List[index].Integer_Value;
                index++;
         }
         else if (0 == strcmp(Parameter_List[index].String_Value, "-t"))
         {
                index++;
                seconds = Parameter_List[index].Integer_Value;
                index++;            

         }
         else if (0 == strcmp(Parameter_List[index].String_Value, "-n"))
         {
                index++;
                numOfPkts = Parameter_List[index].Integer_Value;
                index++;            

         }
         else if (0 == strcmp(Parameter_List[index].String_Value, "-b"))
         {
                index++;
                udpRate = Parameter_List[index].Integer_Value;
                index++;            
         }
         else if (0 == strcmp(Parameter_List[index].String_Value, "-V"))
         {
                index++;
                v6 = 1;
         }
         else if (0 == strcmp(Parameter_List[index].String_Value, "-r"))
         {
                index++;
                reverse_mode = 1;
         }          
         else if (0 == strcmp(Parameter_List[index].String_Value, "-h"))
         {
                index++;
                IPERF_PRINTF("Usage: iperf [-s|-c host] [options]\n");
                IPERF_PRINTF("       iperf [-h]\n");        
         }
         else
         {
                /*silent ignore*/
                index++;
         }
    }

    tCxt.is_iperf = 1;
    tCxt.pktStats.iperf_display_interval = interval;
    tCxt.pktStats.iperf_udp_rate = udpRate;

    rCxt.is_iperf = 1;
    rCxt.pktStats.iperf_display_interval = interval;
    rCxt.pktStats.iperf_udp_rate = udpRate;

    if (operation_mode == IPERF_CLIENT) 
    {
        benchtx_quit = 0;
        tCxt.params.tx_params.v6 = v6;
        tCxt.params.tx_params.ip_address = ipAddress;
        if (pktSize > 0)
        {
            if (v6)
            {
                if (protocol == TCP)
                {
                    pktSize = min(pktSize, IPERF_MAX_PACKET_SIZE_TCPV6);
                }
                else
                {
                    pktSize = min(pktSize, IPERF_MAX_PACKET_SIZE_UDPV6);
                }
            }
            else
            {
                if (protocol == TCP)
                {
                    pktSize = min(pktSize, IPERF_MAX_PACKET_SIZE_TCP);
                }
                else
                {
                    pktSize = min(pktSize, IPERF_MAX_PACKET_SIZE_UDP);
                }
            }
        }
        else
        {
            if (v6)
            {
                if (protocol == TCP)
                {
                    pktSize = IPERF_MAX_PACKET_SIZE_TCPV6;
                }
                else
                {
                    pktSize = IPERF_MAX_PACKET_SIZE_UDPV6;
                }
            }
            else
            {
                if (protocol == TCP)
                {
                    pktSize = IPERF_MAX_PACKET_SIZE_TCP;
                }
                else
                {
                    pktSize = IPERF_MAX_PACKET_SIZE_UDP;
                }
            }
        }
        tCxt.params.tx_params.packet_size = pktSize;
        tCxt.params.tx_params.test_mode = TIME_TEST;
        tCxt.params.tx_params.tx_time = seconds;
        if (numOfPkts > 0)
        {
            tCxt.params.tx_params.test_mode = PACKET_TEST;
            tCxt.params.tx_params.packet_number = numOfPkts;
        }

        /* set default iperf3 port if reverse_mode */
        if( (reverse_mode != 0) && (port_tag == 0) )
        {
            port = IPERF3_DEFAULT_PORT;
        }
       
        if ((protocol == UDP) && mcastEnabled)
        {
            IPERF_PRINTF("Multicast transmit not yet implemented\n");
        }
        else if (protocol == UDP)
        {
            bench_common_SetParams(&tCxt, tCxt.params.tx_params.v6, "udp", port, TX);
            if( reverse_mode == 0 )
            {            
                bench_udp_tx(&tCxt);
            }
            else   /* iperf3 udp reverse mode */
            {
                iperf3_tx_reverse(&tCxt);                
            }
        }
        else if (protocol == TCP)
        {
            bench_common_SetParams(&tCxt, tCxt.params.tx_params.v6, "tcp", port, TX);
            if( reverse_mode == 0 )
            {
                bench_tcp_tx(&tCxt);
            }
            else  /* iperf3 tcp reverse mode */
            {
                iperf3_tx_reverse(&tCxt);                
            }
        }
    } 
    else if (operation_mode == IPERF_SERVER) 
    {
        benchrx_quit = 0;
        rCxt.params.rx_params.v6 = v6;
        if ((protocol == UDP) && mcastEnabled)
        {
            IPERF_PRINTF("Multicast receive not yet implemented\n");
        }
        else if (protocol == UDP)
        {
            bench_common_SetParams(&rCxt, rCxt.params.rx_params.v6, "udp", port, RX);
            bench_udp_rx(&rCxt);     
        }
        else if (protocol == TCP)
        {
            bench_common_SetParams(&rCxt, rCxt.params.rx_params.v6, "tcp", port, RX);
            bench_tcp_rx(&rCxt);
        }
    } 
    else 
    {
    	IPERF_PRINTF("Usage: iperf [-s|-c host] [options]\n");
    	IPERF_PRINTF("Try `iperf -h` for more information.\n");
    	return QCLI_STATUS_ERROR_E;
    }
    
    return QCLI_STATUS_SUCCESS_E;
}


void
iperf_result_print(STATS * pCxtPara, uint32_t prev, uint32_t cur)
{
    uint32_t throughput_Kbps = 0;
    uint32_t msInterval;
    uint32_t bytes, rem_bytes = 0;
	char *transfer_unit = " ";
	char *bandwidth_unit = " ";
	uint32_t sec_val1, sec_val2;

    msInterval = (cur - prev) * 1000;
    pCxtPara->kbytes += pCxtPara->bytes / BYTES_PER_KILO_BYTE;


    if (msInterval > 0) {
    	throughput_Kbps = (pCxtPara->bytes / msInterval) * 8;
    	bytes = pCxtPara->bytes;
    	sec_val1 = pCxtPara->iperf_time_sec;
    	sec_val2 = pCxtPara->iperf_time_sec + pCxtPara->iperf_display_interval;

    	if (bytes > BYTES_PER_KILO_BYTE*BYTES_PER_KILO_BYTE) {
    		transfer_unit = "M";
    		rem_bytes = ((bytes % (BYTES_PER_KILO_BYTE*BYTES_PER_KILO_BYTE)) * 100) / (BYTES_PER_KILO_BYTE*BYTES_PER_KILO_BYTE);
    		bytes /= BYTES_PER_KILO_BYTE*BYTES_PER_KILO_BYTE;
    	}
    	else if (bytes > BYTES_PER_KILO_BYTE) {
    		transfer_unit = "K";
    		rem_bytes = ((bytes % (BYTES_PER_KILO_BYTE)) * 100) / BYTES_PER_KILO_BYTE;
    		bytes /= BYTES_PER_KILO_BYTE;
    	}

    } else {
    	msInterval = (pCxtPara->last_time.seconds - pCxtPara->first_time.seconds) * 1000;

    	if (msInterval == 0) {
    		return; /* error */
    	}

    	pCxtPara->iperf_time_sec -= pCxtPara->iperf_display_interval;

    	/* Final stats */
    	throughput_Kbps = (pCxtPara->kbytes /
    			(msInterval/1000)) * 8;

    	sec_val1 = 0;
    	sec_val2 = msInterval/1000;
    	bytes = pCxtPara->kbytes; /* Note: working with KB */

    	if (bytes > BYTES_PER_KILO_BYTE*BYTES_PER_KILO_BYTE) {
    		transfer_unit = "G";
    		rem_bytes = ((bytes % (BYTES_PER_KILO_BYTE*BYTES_PER_KILO_BYTE)) * 100) / (BYTES_PER_KILO_BYTE*BYTES_PER_KILO_BYTE);
    		bytes /= BYTES_PER_KILO_BYTE*BYTES_PER_KILO_BYTE;
    	}
    	else if (bytes > BYTES_PER_KILO_BYTE) {
    		transfer_unit = "M";
    		rem_bytes = ((bytes % (BYTES_PER_KILO_BYTE)) * 100) / BYTES_PER_KILO_BYTE;
    		bytes /= BYTES_PER_KILO_BYTE;
    	} else if (bytes) {
    		transfer_unit = "K";
    	}
    }

	if (throughput_Kbps > BYTES_PER_KILO_BYTE) {
		bandwidth_unit = "M";
	}
	else if (pCxtPara->bytes > 0) {
		bandwidth_unit = "K";
	}
       if (throughput_Kbps > BYTES_PER_KILO_BYTE)  {
	IPERF_PRINTF("[%3d] %2d.0-%2d.0 sec %3d.%02d %sBytes %d.%02d %sbits/sec\n",
			pCxtPara->iperf_stream_id,
			sec_val1, sec_val2,
			bytes, rem_bytes, transfer_unit,
			throughput_Kbps/BYTES_PER_KILO_BYTE, (throughput_Kbps%BYTES_PER_KILO_BYTE)/10, bandwidth_unit);
       } else if (pCxtPara->bytes > 0) {
	IPERF_PRINTF("[%3d] %2d.0-%2d.0 sec %3d.%02d %sBytes %d.%02d %sbits/sec\n",
			pCxtPara->iperf_stream_id,
			sec_val1, sec_val2,
			bytes, rem_bytes, transfer_unit,
			throughput_Kbps, 0, bandwidth_unit);       
       }

	/* Clear for next time */
	pCxtPara->bytes = 0;
	pCxtPara->iperf_time_sec += pCxtPara->iperf_display_interval;
}

static void iperf3_make_cookie(char *str, int len)
{
    memset(str, 0, len);
    strncpy((char *)str, COOKIE_CLIENT_NAME, len);
    return;
}

/************************************************************************
* NAME: iperf3_tx_reverse
*
* DESCRIPTION: Start TCP/UDP Transmit reverse test.
************************************************************************/
void iperf3_tx_reverse(THROUGHPUT_CXT *p_tCxt)
{
    struct sockaddr_in foreign_addr;
    struct sockaddr_in6 foreign_addr6;
    struct sockaddr *to;
    uint32_t tolen;
    char ip_str[48];
    char state = PARAM_EXCHANGE;
    char sub_state = 0;
    uint32_t cur_packet_number = 0;
    int32_t netbuf_id = QAPI_NETBUF_APP;
    int32_t received;
    int32_t exchange_result_len = 0;
    int family;
    fd_set rd_set;
    int32_t conn_sock;
    uint32_t iperf_display_interval=0;
    uint32_t iperf_display_last=0;
    uint32_t iperf_display_next=0;
    int opt = 1;
    int test_running = 1;

    p_tCxt->sock_control = A_ERROR;
    p_tCxt->sock_peer = A_ERROR;    
    p_tCxt->pktStats.iperf_time_sec = 0;

    memset(ip_str, 0, sizeof(ip_str));    
    
    /*Allocate buffer*/
    if ((p_tCxt->buffer = qapi_Net_Buf_Alloc(CFG_PACKET_SIZE_MAX_RX, netbuf_id)) == NULL)
    {
        QCLI_Printf(qcli_net_handle, "Out of memory error\n");
        goto ERROR_2;
    }
    
    if (p_tCxt->params.tx_params.v6) /* IPV6 */
    {
        family = AF_INET6;
        inet_ntop(family, &p_tCxt->params.tx_params.v6addr[0], ip_str, sizeof(ip_str));

        memset(&foreign_addr6, 0, sizeof(foreign_addr6));
        memcpy(&foreign_addr6.sin_addr, p_tCxt->params.tx_params.v6addr, sizeof(foreign_addr6.sin_addr));
        foreign_addr6.sin_port     = htons(p_tCxt->params.tx_params.port);
        foreign_addr6.sin_family   = family;
        foreign_addr6.sin_scope_id = p_tCxt->params.tx_params.scope_id;

        to = (struct sockaddr *)&foreign_addr6;
        tolen = sizeof(foreign_addr6);
    } else {  /* IPv4 */
        family = AF_INET;
        inet_ntop(family, &p_tCxt->params.tx_params.ip_address, ip_str, sizeof(ip_str));

        memset(&foreign_addr, 0, sizeof(foreign_addr));
        foreign_addr.sin_addr.s_addr    = p_tCxt->params.tx_params.ip_address;
        foreign_addr.sin_port           = htons(p_tCxt->params.tx_params.port);
        foreign_addr.sin_family         = family;

        to = (struct sockaddr *)&foreign_addr;
        tolen = sizeof(foreign_addr);
    }

    /* create control socket */
    if ((p_tCxt->sock_control = qapi_socket(family, SOCK_STREAM, 0)) == A_ERROR)
    {
        QCLI_Printf(qcli_net_handle, "ERROR: Unable to create control socket\n");
        goto ERROR_2;
    }

    /* enable TCP keepalive */
    qapi_setsockopt(p_tCxt->sock_control, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt));

    /* set to non-blocking mode */
	qapi_setsockopt(p_tCxt->sock_control, SOL_SOCKET, SO_NBIO, NULL, 0);
    
    /* Connect to the server.*/
    if (qapi_connect( p_tCxt->sock_control, to, tolen) == A_ERROR)
    {
        QCLI_Printf(qcli_net_handle, "Control Socket Connection Failed.\n");
        goto ERROR_2;
    }
    
    /* send cookie */
    iperf3_make_cookie(p_tCxt->buffer, COOKIE_SIZE);
    if( qapi_send( p_tCxt->sock_control, (char *)p_tCxt->buffer, COOKIE_SIZE, 0) <=0 )
    {
        QCLI_Printf(qcli_net_handle, "ERROR: Failed to send cookie\n");
        goto ERROR_2;
    }
    
    qapi_fd_zero(&rd_set);
    qapi_fd_set(p_tCxt->sock_control, &rd_set);
    
    while(1)
    {
		fd_set rset;
		qapi_fd_zero(&rset);
        rset = rd_set;
        
        if (benchtx_quit)
        {
            app_get_time(&p_tCxt->pktStats.last_time);
            break;
        }

		/* Socket select to check if data is available */
		conn_sock = qapi_select(&rset, NULL, NULL, 10);

		if (conn_sock == 0) 
        {
			/* No activity. Continue with the next round */
			continue;
		} else {
            /* control socket */
            if ( qapi_fd_isset(p_tCxt->sock_control, &rset)) 
            {
                if( state == EXCHANGE_RESULTS)
                {
                    int32_t l;
                    /* when state is EXCHANGE_RESULTS */
                    /* receive remote results length, then receive results */
                    if( sub_state == 0 ) {
                        /* receive server result length */
                        if( qapi_recv( p_tCxt->sock_control, (char*) &l, sizeof(l), 0) < 0 )
                        {
                            QCLI_Printf(qcli_net_handle, "Control Socket Receive Results Length Error\n");
                            goto ERROR_2;
                        }
                        exchange_result_len  = ntohl(l);
                        sub_state = 1;
                    } else {
                        /* receive server result */
                        l = exchange_result_len < CFG_PACKET_SIZE_MAX_RX?exchange_result_len:CFG_PACKET_SIZE_MAX_RX;
                        received = qapi_recv( p_tCxt->sock_control, (char*) p_tCxt->buffer, l, 0);
                        if( received < 0 )
                        {
                            QCLI_Printf(qcli_net_handle, "Control Socket Receive Results Error\n");
                            goto ERROR_2;
                        } else {
                            exchange_result_len = exchange_result_len - received;
                            if(  exchange_result_len == 0)
                                state = DISPLAY_RESULTS;
                        }
                    }
                } else {
                    /* receive state */
                    received = qapi_recv( p_tCxt->sock_control, (char*)(&state), sizeof(unsigned char), 0);
                    if (received == 0) 
                    {
                        QCLI_Printf(qcli_net_handle, "Control Socket Error to receive\n");
                        goto ERROR_2; 
                    }

                    switch ((int)state) {
                    case PARAM_EXCHANGE:
                        /* send parameters */
                        {
                            int32_t len, l;
                            #define BUF_LEN 512
                            /* generate parameters */
                            snprintf(p_tCxt->buffer, 
                                     BUF_LEN, 
                                     "{\"%s\":true,\"omit\":0,\"%s\":%d,\"parallel\":1,\"reverse\":true,\"len\":%d,\"client_version\":\"3.1.2\" }", 
                                     p_tCxt->protocol == TCP?"tcp":"udp", 
                                     p_tCxt->params.tx_params.test_mode == TIME_TEST?"time":"blockcount", 
                                     p_tCxt->params.tx_params.test_mode == TIME_TEST?p_tCxt->params.tx_params.tx_time:p_tCxt->params.tx_params.packet_number, 
                                     p_tCxt->params.tx_params.packet_size);
                            
                            /* send parameters length */
                            len = strlen(p_tCxt->buffer);
                            l = htonl(len);
                            qapi_send( p_tCxt->sock_control, (char *) &l, sizeof(l), 0);
                            app_msec_delay(1);
                            /* send parameters */
                            qapi_send( p_tCxt->sock_control, (char *) p_tCxt->buffer, len, 0);
                            app_msec_delay(1);
                        }
                        break;
                    case CREATE_STREAMS:
                        p_tCxt->iperf_stream_id = 1;
                        p_tCxt->pktStats.iperf_stream_id = p_tCxt->iperf_stream_id;

                        if( p_tCxt->protocol == TCP )     
                        {
                            int32_t window_size = 0;      
                            /* Create TCP data socket */
                            if ((p_tCxt->sock_peer = qapi_socket(family, SOCK_STREAM, 0)) == A_ERROR)
                            {
                                QCLI_Printf(qcli_net_handle, "ERROR: Unable to create socket\n");
                                goto ERROR_2;
                            }

                            /* Configure queue sizes */
                            bench_config_queue_size(p_tCxt->sock_peer);
                        
                            qapi_getsockopt(p_tCxt->sock_peer,SOL_SOCKET, SO_SNDBUF, &window_size, (int32_t *)sizeof(window_size));

                            QCLI_Printf(qcli_net_handle, "------------------------------------------------------------\n");
                            QCLI_Printf(qcli_net_handle, "Client connecting to %s, TCP port %d\n", ip_str, p_tCxt->params.tx_params.port);
                            QCLI_Printf(qcli_net_handle, "TCP window size: %d bytes\n",window_size);
                            QCLI_Printf(qcli_net_handle, "------------------------------------------------------------\n");
                            
                            /* enable TCP keepalive */
                            qapi_setsockopt(p_tCxt->sock_peer, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt));

                            /* set to non-blocking mode */
                            qapi_setsockopt(p_tCxt->sock_peer, SOL_SOCKET, SO_NBIO, NULL, 0);                            
                        
                            /* display connecting */
                            QCLI_Printf(qcli_net_handle, "Connecting\n");
                            
                        } else {  /*UDP */
                            int32_t buffer_size = 0;      
                            /* Create UDP data socket */
                            if ((p_tCxt->sock_peer = qapi_socket(family, SOCK_DGRAM, 0)) == A_ERROR)
                            {
                                QCLI_Printf(qcli_net_handle, "ERROR: Unable to create socket\n");
                                goto ERROR_2;
                            }                            

                            /* Configure queue sizes */
                            bench_config_queue_size(p_tCxt->sock_peer);
                        
                            qapi_getsockopt(p_tCxt->sock_peer,SOL_SOCKET, SO_RCVBUF, &buffer_size, (int32_t *)sizeof(buffer_size));

                            QCLI_Printf(qcli_net_handle, "------------------------------------------------------------\n");
                            QCLI_Printf(qcli_net_handle, "Client connecting to %s, UDP port %d\n", ip_str, p_tCxt->params.tx_params.port);
                            QCLI_Printf(qcli_net_handle, "UDP buffer size: %d bytes\n",buffer_size);
                            QCLI_Printf(qcli_net_handle, "------------------------------------------------------------\n");
                            
                            /* set to non-blocking mode */
                            qapi_setsockopt(p_tCxt->sock_peer, SOL_SOCKET, SO_NBIO, NULL, 0); 
                        }                            

                        /* Connect to the server */
                        if (qapi_connect( p_tCxt->sock_peer, to, tolen) == A_ERROR)
                        {
                            QCLI_Printf(qcli_net_handle, "Connection failed.\n");
                            goto ERROR_2;
                        }

                        qapi_fd_set(p_tCxt->sock_peer, &rd_set);
                        
                        if( p_tCxt->protocol == TCP ) 
                        {    
                            /* send cookie */
                            iperf3_make_cookie(p_tCxt->buffer, COOKIE_SIZE);
                            if( qapi_send( p_tCxt->sock_peer, (char *)p_tCxt->buffer, COOKIE_SIZE, 0) <=0 )
                            {
                                QCLI_Printf(qcli_net_handle, "ERROR: Failed to send cookie\n");
                                goto ERROR_2;
                            }
                        } else { /* UDP */
                            /* send anything with 4 bytes length */
                            if( qapi_send( p_tCxt->sock_peer, (char *)p_tCxt->buffer, 4, 0) <=0 )
                            {
                                QCLI_Printf(qcli_net_handle, "ERROR: Failed to send UDP\n");
                                goto ERROR_2;
                            }
                        }

                        /*Reset all counters*/
                        cur_packet_number = 0;
                        p_tCxt->pktStats.bytes = 0;
                        p_tCxt->pktStats.iperf_time_sec = 0;
                        
                        app_get_time(&p_tCxt->pktStats.first_time);
  
                        iperf_display_interval = p_tCxt->pktStats.iperf_display_interval;
                        iperf_display_last = p_tCxt->pktStats.first_time.seconds;
                        iperf_display_next = iperf_display_last + iperf_display_interval;

                        QCLI_Printf(qcli_net_handle, "Receiving\n");
                        break;
                    case TEST_START:
                        break;
                    case TEST_RUNNING:
                        break;
                    case EXCHANGE_RESULTS:
                        {
                        int32_t len, l;
                        #define BUF_LEN    512
                        /* generate result */
                        snprintf(p_tCxt->buffer, BUF_LEN, "{\"cpu_util_total\":0.011,\"cpu_util_user\":0.001,\"cpu_util_system\":0.01,\"sender_has_retransmits\":0,\"streams\":[{\"id\":1,\"bytes\":%lld,\"retransmits\":-1,\"jitter\":0,\"errors\":0,\"packets\":0,\"start_time\":0,\"end_time\":%ld}]}", p_tCxt->pktStats.bytes, p_tCxt->pktStats.last_time.seconds - p_tCxt->pktStats.first_time.seconds);
                        len = strlen(p_tCxt->buffer);
                        l = htonl(len);
                        /* send result length */
                        qapi_send( p_tCxt->sock_control, (char *) &l, sizeof(l), 0);
                        /* send result */
                        qapi_send( p_tCxt->sock_control, (char *) p_tCxt->buffer, len, 0);
                        }
                        break;
                    case DISPLAY_RESULTS:
                        /* send IPERF DONE */
                        {
                        unsigned char ch = IPERF_DONE; 
                        qapi_send( p_tCxt->sock_control, (char *) &ch, sizeof(ch), 0);
                        /* finish here */
                        benchtx_quit = 1;
                        }
                        break;
                    case IPERF_DONE:
                        break;
                    case SERVER_TERMINATE:
                        /*
                        * Temporarily be in DISPLAY_RESULTS phase so we can get
                        * ending summary statistics.
                        */
                        /*quit */
                        QCLI_Printf(qcli_net_handle, "server terminate\n");
                        benchtx_quit = 1;
                        break;
                    case ACCESS_DENIED:
                        QCLI_Printf(qcli_net_handle, "Access denied\n");
                        goto ERROR_2; 
                        break;
                    case SERVER_ERROR:
                        {
                        int32_t err, i_errno, errno;
                        app_msec_delay(50);                     
                        if( qapi_recv( p_tCxt->sock_control, (char*) &err, sizeof(err), 0) < 0 )
                        {
                            QCLI_Printf(qcli_net_handle, "Control Socket Receive Error\n");
                            goto ERROR_2;
                        }
                        i_errno = ntohl(err);
                        app_msec_delay(50); 
                        if( qapi_recv( p_tCxt->sock_control, (char*) &err, sizeof(err), 0) < 0 )
                        {
                            QCLI_Printf(qcli_net_handle, "Control Socket Receive Error\n");
                            goto ERROR_2;
                        }
                        errno = ntohl(err);
                        QCLI_Printf(qcli_net_handle, "Server Error: i_errno: %d, errno:%d\n", i_errno, errno);
                        goto ERROR_2;
                        }
                    default:
                        QCLI_Printf(qcli_net_handle, "Unknown control code:%d\n", state);
                        goto ERROR_2; 
                    }  /* switch */    
                }
            }  /* end of control socket process */
            /* receive data from peer */
            if( (p_tCxt->sock_peer != A_ERROR) && (qapi_fd_isset(p_tCxt->sock_peer, &rset)) ) 
            {
                if( p_tCxt->protocol == TCP ) {  
                    received = qapi_recv( p_tCxt->sock_peer, (char*)(&p_tCxt->buffer[0]), CFG_PACKET_SIZE_MAX_RX, 0);
                } else {
                    received = qapi_recvfrom( p_tCxt->sock_peer, (char*)(&p_tCxt->buffer[0]), CFG_PACKET_SIZE_MAX_RX, 0, to, (int32_t *)&tolen);
                    if( received == 0 )
                        continue;
                }

			    /*Valid packet received*/
                if (received > 0)
				{
                    cur_packet_number++;
                    p_tCxt->pktStats.bytes += received;
				} else {/* received <= 0 */
                    QCLI_Printf(qcli_net_handle, "\nReceived %llu bytes \n", p_tCxt->pktStats.bytes);
                    /* remote may close the socket */
                    benchtx_quit = 1;
                    continue;
				}

                if(p_tCxt->pktStats.iperf_display_interval > 0)
                {
                    time_struct_t iperf_curr_time;
                    app_get_time(&iperf_curr_time);
                    uint32_t cur_time = iperf_curr_time.seconds;

                    if(cur_time >= iperf_display_next) 
                    {
                        iperf_result_print(&p_tCxt->pktStats, iperf_display_last, cur_time);
                        iperf_display_last = cur_time;
                        iperf_display_next = cur_time + iperf_display_interval;
                    }
                }

                // check the test completion condition based on number of packets sent
                if ( (p_tCxt->params.tx_params.test_mode == PACKET_TEST) && (test_running != 0) )
                {
                    if ((cur_packet_number >= p_tCxt->params.tx_params.packet_number))
                    {
                        char ch = TEST_END;
                        /* Test completed */
                        app_get_time(&p_tCxt->pktStats.last_time);
                        /* send TEST_END */
                        if( qapi_send( p_tCxt->sock_control, (char *) &ch, sizeof(ch), 0) <=0 )
                        {
                            QCLI_Printf(qcli_net_handle, "Error to send Test END\n");
                            goto ERROR_2;
                        }
                        test_running = 0;                            
                    }
                }

                // check the test completion condition based on time
                if ( (p_tCxt->params.tx_params.test_mode == TIME_TEST) && (test_running != 0) )
                {
                    app_get_time(&p_tCxt->pktStats.last_time);
                    if (bench_common_check_test_time(p_tCxt))
                    {
                        char ch = TEST_END;
                        /* Test completed */
                        /* send TEST_END */
                        if( qapi_send( p_tCxt->sock_control, (char *) &ch, sizeof(ch), 0) <=0 )
                        {
                            QCLI_Printf(qcli_net_handle, "Error to send Test END\n");
                            goto ERROR_2;
                        }
                        test_running = 0;
                    }
                }
            }  /* end receive data */
        }
    } /* while(1) */

    iperf_result_print(&p_tCxt->pktStats, 0, 0);

ERROR_2:
    if( p_tCxt->sock_peer != A_ERROR )
        qapi_socketclose( p_tCxt->sock_peer);

    if( p_tCxt->sock_control != A_ERROR )
        qapi_socketclose( p_tCxt->sock_control);

    if( p_tCxt->buffer != NULL )
        qapi_Net_Buf_Free(p_tCxt->buffer, netbuf_id);

    return;
}


#endif
