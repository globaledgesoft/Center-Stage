/*
 * Copyright (c) 2011-2018 Qualcomm Technologies, Inc.
 * 2011-2016 Qualcomm Atheros, Inc.
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

#include "net_bench.h"
#include <string.h>

#define	TOS_VO	    0xD0
#define	TOS_VI	    0xA0
#define	TOS_BK	    0x20
#define	TOS_BE	    0x00

extern QCLI_Group_Handle_t qcli_net_handle; /* Handle for Net Command Group. */
QCLI_Command_Status_t bench_uapsd_test(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	int32_t            retval = -1;
	uint32_t           remote_ip_address;
	uint16_t 		   remote_port;
	uint32_t 		   num_pkts;
	uint16_t 		   time_interval;
	uint8_t 		   ac;
	struct sockaddr_in foreign_addr;
	struct sockaddr *to;
	uint32_t tolen =0;
	int32_t sock_peer =0;
	uint32_t packet_size =200;
	char dscp =0;
	char* sendbuffer;
	char *receiver_ip;
	uint32_t netbuf_id;
    int send_flag;
	uint32_t allocbuf_trynums =0;
	uint32_t cur_packet_number =0;
	uint32_t cookie =0;
    uint8_t data[4] = {0xbb, 0xbb, 0xbb, 0xbb};
    if(Parameter_Count < 5)
    {
        /*Incorrect number of params, exit*/
        return QCLI_STATUS_USAGE_E;
    }
	else
	{
		receiver_ip = Parameter_List[0].String_Value;
		if ((retval = inet_pton(AF_INET, receiver_ip, &remote_ip_address)) != 0)
		{
			QCLI_Printf("Incorrect IPv4 address %s\n", receiver_ip);
			return QCLI_STATUS_USAGE_E;
		}
		remote_port = Parameter_List[1].Integer_Value;

		num_pkts = Parameter_List[2].Integer_Value;
		time_interval = Parameter_List[3].Integer_Value;
		ac = Parameter_List[4].Integer_Value;
		if( ac>3 )
		{
			QCLI_Printf("access category should be 0~3 0-BE 1-BK 2-VI 3-VO\n");
			return QCLI_STATUS_USAGE_E;
		}
	}
    memset(&foreign_addr, 0, sizeof(foreign_addr));
    foreign_addr.sin_addr.s_addr    = remote_ip_address;
    foreign_addr.sin_port           = htons(remote_port);
    foreign_addr.sin_family         = AF_INET;
    to = (struct sockaddr *)&foreign_addr;
	tolen = sizeof(foreign_addr);
	switch( ac)
	{
		case 0:
			dscp = 0x00;
			break;
		case 1:
			dscp = TOS_BK;
			break;
		case 2:
			dscp = TOS_VI;
			break;
		case 3:
			dscp = TOS_VO;
			break;
		default:
			dscp = TOS_BE;
			break;
	}

		    /* Create UDP socket */
    if ((sock_peer = qapi_socket(AF_INET, SOCK_DGRAM, 0)) == A_ERROR) {
		return QCLI_STATUS_ERROR_E;
    }
	qapi_setsockopt(sock_peer, IP_OPTIONS, IP_TOS, (void*)&dscp, sizeof(dscp));
	if (qapi_connect( sock_peer, to, tolen) == A_ERROR)
    {
        QCLI_Printf(qcli_net_handle,"Conection failed\n");
        goto ERROR1;
    }

    netbuf_id = QAPI_NETBUF_APP;
    send_flag = 0;

	while(1)
    {

        while ((sendbuffer = qapi_Net_Buf_Alloc(packet_size, netbuf_id)) == NULL)
        {
            /*Wait till we get a buffer*/
			allocbuf_trynums++;
			if(allocbuf_trynums >1000)
				goto ERROR1;
            /*Allow small delay to allow other thread to run*/
            app_msec_delay(1);
        }
		if((htonl(remote_ip_address) & 0xff) == 0xff)
		{
			cookie = g_cookie_mc;
		}
		else
		{
			cookie = g_cookie_uc;
		}
		qapi_Net_Buf_Update(sendbuffer, 0, &cookie, 4, netbuf_id);
	    qapi_Net_Buf_Update(sendbuffer, 4, &remote_ip_address, 4, netbuf_id);
        qapi_Net_Buf_Update(sendbuffer, 8, &g_cookie_uc, 4, netbuf_id);
        qapi_Net_Buf_Update(sendbuffer, 12, &g_cookie_mc, 4, netbuf_id);
        qapi_Net_Buf_Update(sendbuffer, 16, &sendbuffer, 4, netbuf_id);
        qapi_Net_Buf_Update(sendbuffer, 20, data, packet_size-20, netbuf_id);
		if((htonl(remote_ip_address) & 0xff) == 0xff)
		{
			g_cookie_mc++;
		}
		else
		{
			g_cookie_uc++;
		}
		qapi_sendto(sock_peer, sendbuffer, packet_size, send_flag, to, tolen) ;
		cur_packet_number++;

        qapi_Net_Buf_Free(sendbuffer, netbuf_id);

		if(time_interval>0)
			app_msec_delay(time_interval);

        if((cur_packet_number >= num_pkts))
        {
            break;
        }

	}

	QCLI_Printf("uapsdtest send the number of data pkts=%d\n",cur_packet_number);
	qapi_socketclose(sock_peer);
	return QCLI_STATUS_SUCCESS_E;
ERROR1:
		qapi_socketclose(sock_peer);
		return QCLI_STATUS_ERROR_E;
}

