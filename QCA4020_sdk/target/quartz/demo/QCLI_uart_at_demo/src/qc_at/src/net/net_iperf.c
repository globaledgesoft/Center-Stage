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

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "qapi_status.h"
#include "net_iperf.h"
#include "net_bench.h"
#include "qapi_netservices.h"
#include "qapi_socket.h"
#include "qapi_ns_gen_v4.h"
#include "qapi_ns_gen_v6.h"

extern QCLI_Group_Handle_t qcli_net_handle; /* Handle for Net Command Group. */
extern uint8_t benchtx_quit;
extern uint8_t benchrx_quit;

#define IPERF_PRINTF(...) QCLI_Printf(__VA_ARGS__)

QCLI_Command_Status_t iperf(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    unsigned int protocol = TCP;
    unsigned int port = IPERF_DEFAULT_PORT;
    unsigned int seconds = IPERF_DEFAULT_RUNTIME;
    unsigned int pktSize = 0;
    int operation_mode = -1;  
    unsigned int udpRate = IPERF_DEFAULT_UDPRate;
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
       
        if ((protocol == UDP) && mcastEnabled)
        {
            IPERF_PRINTF("Multicast transmit not yet implemented\n");
        }
        else if (protocol == UDP)
        {
            
            bench_common_SetParams(&tCxt, tCxt.params.tx_params.v6, "udp", port, TX);
            bench_udp_tx(&tCxt);    
        }
        else if (protocol == TCP)
        {
            bench_common_SetParams(&tCxt, tCxt.params.tx_params.v6, "tcp", port, TX);
            bench_tcp_tx(&tCxt);
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
    pCxtPara->kbytes += pCxtPara->bytes / 1024;


    if (msInterval > 0) {
    	throughput_Kbps = (pCxtPara->bytes / msInterval) * 8;
    	bytes = pCxtPara->bytes;
    	sec_val1 = pCxtPara->iperf_time_sec;
    	sec_val2 = pCxtPara->iperf_time_sec + pCxtPara->iperf_display_interval;

    	if (bytes > 1024*1024) {
    		transfer_unit = "M";
    		rem_bytes = ((bytes % (1024*1024)) * 100) / (1024*1024);
    		bytes /= 1024*1024;
    	}
    	else if (bytes > 1024) {
    		transfer_unit = "K";
    		rem_bytes = ((bytes % (1024)) * 100) / 1024;
    		bytes /= 1024;
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

    	if (bytes > 1024*1024) {
    		transfer_unit = "G";
    		rem_bytes = ((bytes % (1024*1024)) * 100) / (1024*1024);
    		bytes /= 1024*1024;
    	}
    	else if (bytes > 1024) {
    		transfer_unit = "M";
    		rem_bytes = ((bytes % (1024)) * 100) / 1024;
    		bytes /= 1024;
    	} else if (bytes) {
    		transfer_unit = "K";
    	}
    }

	if (throughput_Kbps > 1000) {
		bandwidth_unit = "M";
	}
	else if (pCxtPara->bytes > 0) {
		bandwidth_unit = "K";
	}
       if (throughput_Kbps > 1000)  {
	IPERF_PRINTF("[%3d] %2d.0-%2d.0 sec %3d.%02d %sBytes %d.%02d %sbits/sec\n",
			pCxtPara->iperf_stream_id,
			sec_val1, sec_val2,
			bytes, rem_bytes, transfer_unit,
			throughput_Kbps/1000, (throughput_Kbps%1000)/10, bandwidth_unit);
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

