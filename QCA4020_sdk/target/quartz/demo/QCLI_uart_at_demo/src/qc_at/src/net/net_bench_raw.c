/*
 * Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
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

#include <string.h>
#include "net_bench.h"
#include "qapi_delay.h"

extern uint8_t benchtx_quit;
extern uint8_t benchrx_quit;

/************************************************************************
* NAME: qca_raw_tx
*
* DESCRIPTION: Start TX RAW socket throughput test.
************************************************************************/
int bench_raw_tx(THROUGHPUT_CXT *p_tCxt)
{
    struct sockaddr_in foreign_addr;
    struct sockaddr *to;
    uint32_t tolen;
    char ip_str [48];
    int32_t send_bytes, result;
    uint32_t packet_size, message_size;
    char *pb;
    uint32_t cur_packet_number, i, n_send_ok;
    int send_flag = 0;
    int family;
    int tos_opt;
    uint16_t proto;

    proto = p_tCxt->params.tx_params.port;
    if (proto > 255)
    {
        QCLI_Printf("ERROR: Protocol is greater than 255.\n");
        return -1;
    }

    family = AF_INET;
    inet_ntop(family, &p_tCxt->params.tx_params.ip_address, ip_str, sizeof(ip_str));

    memset(&foreign_addr, 0, sizeof(foreign_addr));
    foreign_addr.sin_addr.s_addr    = p_tCxt->params.tx_params.ip_address;
    foreign_addr.sin_family         = family;

    to = (struct sockaddr *)&foreign_addr;
    tolen = sizeof(foreign_addr);
    tos_opt = IP_TOS;

    /* Create socket */
    if ((p_tCxt->sock_peer = qapi_socket(family, SOCK_RAW, (int)proto)) == A_ERROR)
    {
        QCLI_Printf("ERROR: Socket creation failed\n");
        return -1;
    }

    packet_size = message_size = p_tCxt->params.tx_params.packet_size;

    if (p_tCxt->protocol == IP_RAW_HDR && p_tCxt->test_type == TX)
    {
        int on = 1;
        if (qapi_setsockopt(p_tCxt->sock_peer, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) != QAPI_OK)
        {
            QCLI_Printf("ERROR: set IP_HDRINCL failure\n");
            goto ERROR_2;
        }
        packet_size += sizeof(ipv4_header_t);  /* IPv4 hdr is 20 bytes */
    }
    else  /* IP_RAW_TX */
    if (p_tCxt->params.tx_params.ip_tos > 0)
    {
	    qapi_setsockopt(p_tCxt->sock_peer, IP_OPTIONS, tos_opt, &p_tCxt->params.tx_params.ip_tos, sizeof(uint8_t));
    }

    /* ------ Start test.----------- */
    QCLI_Printf("****************************************************************\n");
    QCLI_Printf("Raw Socket TX Test (IP_HDRINCL is %s)\n", (p_tCxt->protocol == IP_RAW_HDR && p_tCxt->test_type == TX) ? "on" : "off");
    QCLI_Printf("****************************************************************\n");
    QCLI_Printf("Remote IP addr: %s\n", ip_str);
    QCLI_Printf("Protocol: %d\n", proto);
    QCLI_Printf("Message size: %d\n", message_size);
    QCLI_Printf("Number of messages: %d\n", p_tCxt->params.tx_params.packet_number);
    //QCLI_Printf("Delay in microseconds: %d\n", p_tCxt->params.tx_params.interval_us);
    QCLI_Printf("Type benchquit to terminate test\n");
    QCLI_Printf("****************************************************************\n");

    /* Connect to the server.*/
    QCLI_Printf("Connecting\n");
    if (qapi_connect(p_tCxt->sock_peer, to, tolen) == A_ERROR)
    {
        QCLI_Printf("ERROR: Conection failed\n");
        goto ERROR_2;
    }

    /*Reset all counters*/
    cur_packet_number = 0;
    i = 0;
    n_send_ok = 0;
    bench_common_clear_stats(p_tCxt);

    QCLI_Printf("Sending\n");
    app_get_time(&p_tCxt->pktStats.first_time);

    uint32_t is_test_done = 0;
    while ( !is_test_done )
    {
        if (benchtx_quit)
        {
            app_get_time(&p_tCxt->pktStats.last_time);
            break;
        }

        /* allocate the buffer, if needed */
        if ( p_tCxt->buffer == NULL )
        {
            while ((p_tCxt->buffer = qapi_Net_Buf_Alloc(packet_size, QAPI_NETBUF_APP)) == NULL)
            {
                /*Wait till we get a buffer*/
                if (benchtx_quit)
                {
                    app_get_time(&p_tCxt->pktStats.last_time);
                    goto ERROR_2;
                }
                /*Allow small delay to allow other thread to run*/
                qapi_Task_Delay(1000);
            }

            /* Clear the buffer */
            memset(p_tCxt->buffer, 0, packet_size);
        }

        /* Update net buffer:
         *
         * [START]<4-byte Packet Index><4-byte Packet Size>000102..FF000102..FF0001..[END]
         * Byte counts: 8 + 4 + 4 + (message_size-22) + 6
         *
         */
        pb = (p_tCxt->protocol == IP_RAW_HDR && p_tCxt->test_type == TX) ?
             &p_tCxt->buffer[sizeof(ipv4_header_t)] : p_tCxt->buffer;

        if (message_size >= 22)
        {
            char *msg_start = "[START]";
            char *msg_end = "[END]";
            uint32_t val;

            /* Add "[START]": 8 bytes */
            qapi_Net_Buf_Update(pb, 0, msg_start, 8, QAPI_NETBUF_APP);

            /* Packet index: 4 bytes */
            val = htonl(cur_packet_number);
            qapi_Net_Buf_Update(pb, 8, &val, 4, QAPI_NETBUF_APP);

            /* Packet size: 4 bytes */
            val = htonl(message_size);
            qapi_Net_Buf_Update(pb, 12, &val, 4, QAPI_NETBUF_APP);

            /* Add pattern
             * The pattern is repeated '00 01 02 03 .. FE FF'
             */
            bench_common_add_pattern(pb + 16, message_size - 16 - 6);

            /* Add "[END]": 6 bytes */
            qapi_Net_Buf_Update(pb, message_size-6, msg_end, 6, QAPI_NETBUF_APP);
        }
        else if (message_size >= 4)
        {
            /* Small buffer, just send the packet index */
            uint32_t val;

            /* Packet index */
            val = htonl(cur_packet_number);
            qapi_Net_Buf_Update(pb, 0, &val, 4, QAPI_NETBUF_APP);
        }

        /* Add IPv4 header */
        if (p_tCxt->protocol == IP_RAW_HDR && p_tCxt->test_type == TX)
        {
            ipv4_header_t *iphdr = (ipv4_header_t *)p_tCxt->buffer;

            iphdr->ver_ihl = 0x45; /* ver: IPv4, IHL=20 bytes */
            iphdr->tos = p_tCxt->params.tx_params.ip_tos;
            iphdr->len = htons(packet_size);
            iphdr->id = 0;
            iphdr->flags_offset = 0;
            iphdr->ttl = 255;
            iphdr->protocol = (uint8_t)proto;
            iphdr->hdr_chksum = 0;
            iphdr->sourceip = p_tCxt->params.tx_params.source_ipv4_addr; /* already in net order */
            iphdr->destip   = foreign_addr.sin_addr.s_addr;  /* already in net order */
        }

        /* Starting sending */
        do
        {
            send_bytes = qapi_send(p_tCxt->sock_peer, p_tCxt->buffer, packet_size, send_flag);
            //send_bytes = qapi_sendto(p_tCxt->sock_peer, p_tCxt->buffer, packet_size, send_flag, to, tolen);

            if ( send_bytes != packet_size )
            {
                int errno = qapi_errno(p_tCxt->sock_peer);

                if ( (EPIPE == errno) ||
                     (ENOTSOCK == errno) ||
                     (EBADF == errno) ||
                     (EIEIO == errno))
                {
                    QCLI_Printf("Exxxx: send_bytes=%d, errno=%d\n", send_bytes, errno);
                    is_test_done = 1;
                    break;
                }
                else
                {
                    if ( ENOBUFS != errno )
                    {
                        QCLI_Printf("Failed to qapi_send, send_bytes=%d, errno=%d\n", send_bytes, errno);
                    }
                    else
                    {
                        //QCLI_Printf("ENOBUFS: send_bytes=%d\n", send_bytes);
                    }
                    /* severe push back, let the other processes run for 50 ms */
                    qapi_Task_Delay(50000);
                }
            }
            else
            {
                cur_packet_number ++;
            }

            app_get_time(&p_tCxt->pktStats.last_time);
            if (++i >= 500)
            {
                QCLI_Printf(".");
                i = 0;
            }


            QCLI_Printf("%d send_bytes = %d\n", cur_packet_number, send_bytes);


            if (send_bytes > 0)
            {
                p_tCxt->pktStats.bytes += send_bytes;
                ++n_send_ok;
            }

            /*Test mode can be "number of packets" or "fixed time duration"*/
            if (p_tCxt->params.tx_params.test_mode == PACKET_TEST)
            {
                if ((cur_packet_number >= p_tCxt->params.tx_params.packet_number))
                {
                    is_test_done = 1;
                    break;
                }
            }
            else if (p_tCxt->params.tx_params.test_mode == TIME_TEST)
            {
                if (bench_common_check_test_time(p_tCxt))
                {
                    is_test_done = 1;
                    break;
                }
            }

            /****Bandwidth control***********/
           /* if (p_tCxt->params.tx_params.interval_us)
                qapi_Task_Delay(p_tCxt->params.tx_params.interval_us);*/

        } while ( !((is_test_done) || (send_bytes == packet_size) || (NULL == p_tCxt->buffer)) );   /* send loop */

    } /* while ( !is_test_done ) */

    if ( p_tCxt->buffer )
    {
        qapi_Net_Buf_Free(p_tCxt->buffer, QAPI_NETBUF_APP);
    }

    QCLI_Printf("\nSent %u packets, %llu bytes to %s (%u)\n",
            cur_packet_number, p_tCxt->pktStats.bytes, ip_str, cur_packet_number - n_send_ok);

    /* Send endmark packet and wait for stats from server */
    result = bench_common_wait_for_response(p_tCxt, to, tolen, cur_packet_number);

    if (result != QAPI_OK)
    {
        QCLI_Printf("Raw Tx test failed, did not receive Ack from Peer\n");
    }

ERROR_2:
    bench_common_print_test_results(p_tCxt, &p_tCxt->pktStats);
    qapi_socketclose(p_tCxt->sock_peer);

    return 0;
}

/************************************************************************
* NAME: bench_raw_rx
*
* DESCRIPTION: Start throughput raw socket server.
************************************************************************/
int bench_raw_rx(THROUGHPUT_CXT *p_tCxt)
{
    uint16_t proto;
    int32_t  received, message_size;
    int32_t  conn_sock;

    struct sockaddr_in  local_addr;
    struct sockaddr *addr;
    int addrlen;

    struct sockaddr_in  foreign_addr;
    int32_t fromlen;
    struct sockaddr *from;

    char ip_str[48], *pb;
    int family;

    proto = p_tCxt->params.rx_params.port;
    if (proto > 255)
    {
        QCLI_Printf("ERROR: Protocol is greater than 255.\n");
        return -1;
    }

    if ((p_tCxt->buffer = pb = qapi_Net_Buf_Alloc(CFG_PACKET_SIZE_MAX_RX, QAPI_NETBUF_APP)) == NULL)
    {
        QCLI_Printf("Out of memory error\n");
        p_tCxt->buffer = NULL;
        return -1;
    }

    family = AF_INET;
    memset(&foreign_addr, 0, sizeof(foreign_addr));
    from = (struct sockaddr *)&foreign_addr;
    fromlen = sizeof(struct sockaddr_in);

    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = family;
    local_addr.sin_addr.s_addr = p_tCxt->params.rx_params.local_address;
    addr = (struct sockaddr *)&local_addr;
    addrlen = sizeof(struct sockaddr_in);

    /* Create socket */
    if ((p_tCxt->sock_local = qapi_socket(family, SOCK_RAW, (int)proto)) == A_ERROR)
    {
        QCLI_Printf("ERROR: Socket creation error.\n");
        goto ERROR_1;
    }

    /* Bind */
    if (qapi_bind( p_tCxt->sock_local, addr, addrlen) != QAPI_OK)
    {
        QCLI_Printf("ERROR: Socket bind error.\n");
        goto ERROR_2;
    }

    if (p_tCxt->params.rx_params.mcEnabled)
    {
        struct ip_mreq group;

        group.imr_multiaddr = p_tCxt->params.rx_params.mcIpaddr;
        group.imr_interface = p_tCxt->params.rx_params.local_address ?
                              p_tCxt->params.rx_params.local_address : p_tCxt->params.rx_params.mcRcvIf;

        if (qapi_setsockopt(p_tCxt->sock_local, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void *)&group, sizeof(group)) != QAPI_OK)
        {
            QCLI_Printf("ERROR: set IP_ADD_MEMBERSHIP failure\n");
            goto ERROR_2;
        }
    }

    if (p_tCxt->protocol == IP_RAW_HDR && p_tCxt->test_type == RX)
    {
        int on = 1;
        if (qapi_setsockopt(p_tCxt->sock_local, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) != QAPI_OK)
        {
            QCLI_Printf("ERROR: set IP_HDRINCL failure\n");
            goto ERROR_2;
        }
    }

    memset(ip_str,0,sizeof(ip_str));

    /* ------ Start test.----------- */
    QCLI_Printf("****************************************************\n");
    QCLI_Printf(" Raw Socket RX Test (IP_HDRINCL is %s)\n", (p_tCxt->protocol == IP_RAW_HDR && p_tCxt->test_type == RX) ? "on" : "off");
    QCLI_Printf("****************************************************\n");
    QCLI_Printf("Bind address %s\n", inet_ntop(family, &local_addr.sin_addr, ip_str, sizeof(ip_str)));
    QCLI_Printf("Protocol %d\n", proto);
    QCLI_Printf("Type benchquit to termintate test\n");
    QCLI_Printf("****************************************************\n");

    while (!benchrx_quit) /* Main loop */
    {
        int32_t i = 0;  /* for debugging use */
        int32_t is_first = 1;

        QCLI_Printf("Waiting\n");

        bench_common_clear_stats(p_tCxt);
        memset(ip_str,0,sizeof(ip_str));

        while (!benchrx_quit)   /* Receive loop */
        {
            do
            {
                fd_set rset;

                if (benchrx_quit)
                {
                    goto ERROR_3;
                }

                /* block for 500msec or until a packet is received */
                FD_ZERO(&rset);
                FD_SET(p_tCxt->sock_local, &rset);

                conn_sock = qapi_select(&rset, NULL, NULL, 500);
                if (conn_sock == A_ERROR)
                    goto ERROR_3;       // socket no longer valid

            } while (conn_sock == 0);

            /* Receive data */
            received = message_size = qapi_recvfrom( p_tCxt->sock_local, (char*)(&p_tCxt->buffer[0]), CFG_PACKET_SIZE_MAX_RX, 0, from, &fromlen);

            ++i;

            if (received >= 0)
            {
                if (p_tCxt->protocol == IP_RAW_HDR && p_tCxt->test_type == RX)
                {
                    message_size -= 20;
                    pb = p_tCxt->buffer + 20;
                }

                /* end of test */
                if (message_size == sizeof(EOT_PACKET) &&
                    ((EOT_PACKET *)pb)->code == (int)END_OF_TEST_CODE)
                {
                	 if (!is_first)
                	 {
						/* Get 'last_time' before send_ack().  send_ack() will
						 * delay for 1000 ms, which causes inaccurate throughput
						 * if we get 'last_time' after send_ack()
						 */
						app_get_time(&p_tCxt->pktStats.last_time);

						/* Send throughput results to Peer so that it can display correct results */
						send_ack(p_tCxt, from, fromlen);
						break;
                	 }
                }
                else
                {

                    time_struct_t ts;

                    app_get_time(&ts);
                    QCLI_Printf("%d sec=%u ms=%u rcvd=%u\n", i, ts.seconds, ts.milliseconds, received);


                    p_tCxt->pktStats.bytes += received;
                    ++p_tCxt->pktStats.pkts_recvd;
                    if (is_first)
                    {
                        QCLI_Printf("Receiving\n");

                        app_get_time(&p_tCxt->pktStats.first_time);


                        QCLI_Printf("first_time: sec=%u ms=%u\n",
                                p_tCxt->pktStats.first_time.seconds,
                                p_tCxt->pktStats.first_time.milliseconds);

                        is_first = 0;
                    }
                }
            }
            else
            {
                QCLI_Printf("ERROR: %d received= %d\n", i, received);
                break;
            }
        } /* receive_loop */

ERROR_3:
        QCLI_Printf("Received %llu bytes from %s\n", p_tCxt->pktStats.bytes,
                inet_ntop(family, &foreign_addr.sin_addr, ip_str, sizeof(ip_str)));

        bench_common_print_test_results(p_tCxt, &p_tCxt->pktStats);

        /* Clear any remote host association on the socket. We can reuse addr
         * here, to set a zero remote address, since that sockaddr is only
         * used to bind the socket to a local port.
         */
        memset(addr, 0, addrlen);
        qapi_connect(p_tCxt->sock_local, addr, addrlen);
    } /* main loop */

ERROR_2:
    qapi_socketclose(p_tCxt->sock_local);

ERROR_1:

    QCLI_Printf(BENCH_TEST_COMPLETED);

    if (p_tCxt->buffer)
        qapi_Net_Buf_Free(p_tCxt->buffer, QAPI_NETBUF_APP);

    return 0;
}

