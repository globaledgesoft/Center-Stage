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

 
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "qapi_status.h"
#include "bench.h"
#include "iperf.h"
#include "qapi_delay.h"
#include "qapi_ns_gen_v6.h"

#ifdef CONFIG_NET_TXRX_DEMO

extern QCLI_Group_Handle_t qcli_net_handle; /* Handle for Net Command Group. */
extern uint8_t benchtx_quit;
extern uint8_t benchrx_quit;
uint16_t bench_udp_rx_port_in_use = 0;    /* Used to prevent two udp rx streams from using the same port */
QUEUE_T udp_zcq;

#define BENCH_UDP_PKTS_PER_DOT	1000 /* Produce a progress dot each X packets */

/************************************************************************
 ************************************************************************/
uint32_t bench_udp_IsPortInUse(uint16_t port)
{
	int ret = 0;
	if (bench_udp_rx_port_in_use == port) {
			QCLI_Printf(qcli_net_handle, "port %d is in use; use another port.\n", bench_udp_rx_port_in_use);
			ret = 1;
	}
	else {
			bench_udp_rx_port_in_use = port;
	}
	
	return ret;
}

void rxreorder_udp_payload_init
    (stat_udp_pattern_t *stat_udp)
{
    memset(stat_udp, 0, sizeof(stat_udp_pattern_t));
}

int rxreorder_udp_payload_valid
    (stat_udp_pattern_t *stat_udp)
{
    return !!(stat_udp->stat_valid);
}

void rxreorder_udp_payload_statistics
    (stat_udp_pattern_t *stat_udp, char* buffer, int32_t len)
{
    UDP_PATTERN_PACKET udp_pattern;

    if (len < sizeof(UDP_PATTERN_PACKET)) {
        return;
    }

    memcpy(&udp_pattern, buffer, sizeof(UDP_PATTERN_PACKET));
    if (udp_pattern.code != CODE_UDP) {
        return;
    }

    if (!rxreorder_udp_payload_valid(stat_udp)) {
        stat_udp->stat_valid = 1;
        stat_udp->seq_last = udp_pattern.seq;
    }
    stat_udp->pkts_seq_recvd++;
    if (IEEE80211_SN_LESS(udp_pattern.seq, stat_udp->seq_last)) {
        stat_udp->pkts_seq_less++;
    } else {
        stat_udp->seq_last = udp_pattern.seq;
    }
}

#define RATIO_BASE    (10000)
void rxreorder_udp_payload_report
    (stat_udp_pattern_t *stat_udp)
{
    if (!rxreorder_udp_payload_valid(stat_udp)) {
        return;
    }

    if (stat_udp->pkts_plan) {
        stat_udp->ratio_of_drop = ratio((stat_udp->pkts_plan-stat_udp->pkts_recvd), stat_udp->pkts_plan, RATIO_BASE);
    }
    stat_udp->ratio_of_seq_less = ratio(stat_udp->pkts_seq_less, stat_udp->pkts_seq_recvd, RATIO_BASE);
    QCLI_Printf(qcli_net_handle, "udp pkts: plan=%d recvd=%d drop_ratio=%d/%d\n",
        stat_udp->pkts_plan, stat_udp->pkts_recvd, stat_udp->ratio_of_drop, RATIO_BASE);
    QCLI_Printf(qcli_net_handle, "udp pkts of seq: recvd=%d less=%d less_ratio=%d/%d\n",
        stat_udp->pkts_seq_recvd, stat_udp->pkts_seq_less, stat_udp->ratio_of_seq_less, RATIO_BASE);
}

#ifdef CONFIG_NET_SSL_DEMO
/*****************************************************************************
 *****************************************************************************/
static int udp_rx_upcall(void *so, void *pkt, int code, void *src, int family)
{
    if (pkt)
    {
        UDP_ZC_RX_INFO *p;

        if ((p = qapi_Net_Buf_Alloc(sizeof(UDP_ZC_RX_INFO), QAPI_NETBUF_APP)) == NULL)
        {
            return -1;
        }

        p->pkt = (PACKET)pkt;

        if (family == AF_INET)
        {
            memcpy(&p->from, src, sizeof(struct sockaddr_in));

#ifdef ZCOPY_UDP_RX_DEBUG
            {
                PACKET pp = (PACKET)pkt;
                struct sockaddr_in *from = (struct sockaddr_in *)&p->from;
                char ip_str[16];

                QCLI_Printf(qcli_net_handle, "%s: pkt=0x%p from %s:%u tlen=%u plen=%u pk_next=0x%p so=0x%p\n", __func__, pkt,
                        inet_ntop(family, &from->sin_addr, ip_str, sizeof(ip_str)), ntohs(from->sin_port),
                        pp->nb_tlen, pp->nb_plen, pp->pk_next, so);
            }
#endif
        }
        else
        {
            memcpy(&p->from, src, sizeof(struct sockaddr_in6));

#ifdef ZCOPY_UDP_RX_DEBUG
            {
                PACKET pp = (PACKET)pkt;
                struct sockaddr_in6 *from = (struct sockaddr_in6 *)&p->from;
                char ip_str[48];

                QCLI_Printf(qcli_net_handle, "%s: pkt=0x%p from %s:%u tlen=%u plen=%u pk_next=0x%p s=0x%p\n", __func__, pkt,
                        inet_ntop(family, &from->sin_addr, ip_str, sizeof(ip_str)), ntohs(from->sin_port),
                        pp->nb_tlen, pp->nb_plen, pp->pk_next, so);
            }
#endif
        }

        enqueue(&udp_zcq, p);
    }

    return 0;
}
#endif

#if 0
int bench_udp_open(THROUGHPUT_CXT *p_tCxt)
{
	struct sockaddr_in  local_addr;
    struct sockaddr_in6 local_addr6;
	struct sockaddr_in  foreign_addr;
    struct sockaddr_in6 foreign_addr6;
	int family;
	bench_common_start_cmd_t start;
	struct sockaddr *addr;
	void *local_sin_addr, *sin_addr;
	int addrlen;
	char ip_str[48];

	memset(ip_str,0,sizeof(ip_str));
	
	if (p_tCxt->params.rx_params.v6)
    {
        family = AF_INET6;
        local_sin_addr = &local_addr6.sin_addr;
		sin_addr = &foreign_addr6.sin_addr;
		addr = (struct sockaddr *)&local_addr6;
        addrlen = sizeof(struct sockaddr_in6);
        memset(&local_addr6, 0, sizeof(local_addr6));
        local_addr6.sin_port = htons(p_tCxt->params.rx_params.port);
        local_addr6.sin_family = family;
        memcpy(&local_addr6.sin_addr, p_tCxt->params.rx_params.local_v6addr, sizeof(ip6_addr));
        inet_ntop(family, sin_addr, ip_str, sizeof(ip_str));
    }
    else
    {
        family = AF_INET;
        local_sin_addr = &local_addr.sin_addr;
		sin_addr = &foreign_addr.sin_addr;
        addrlen = sizeof(struct sockaddr_in);
		addr = (struct sockaddr *)&local_addr;
        memset(&local_addr, 0, sizeof(local_addr));
        local_addr.sin_port = htons(p_tCxt->params.rx_params.port);
        local_addr.sin_family = family;
        local_addr.sin_addr.s_addr = p_tCxt->params.rx_params.local_address;
		inet_ntop(family, sin_addr, ip_str, sizeof(ip_str));
    }

    /* Open socket */
    if ((p_tCxt->sock_local = qapi_socket(family, SOCK_DGRAM, 0))== A_ERROR)
    {
        QCLI_Printf(qcli_net_handle, "ERROR: Socket creation error.\n");
        return -1;
    }

    /* Bind */
    if (qapi_bind( p_tCxt->sock_local, addr, addrlen) != QAPI_OK)
    {
        QCLI_Printf(qcli_net_handle, "ERROR: Socket bind error.\n");
        qapi_socketclose(p_tCxt->sock_local);
		return -2;
    }

    if (p_tCxt->params.rx_params.mcEnabled)
    {
        if (p_tCxt->params.rx_params.v6)
        {
            struct ipv6_mreq group6;
            memcpy(&group6.ipv6mr_multiaddr, p_tCxt->params.rx_params.mcIpv6addr, sizeof(ip6_addr));
            group6.ipv6mr_interface = p_tCxt->params.rx_params.scope_id;
            if (qapi_setsockopt(p_tCxt->sock_local, IPPROTO_IP, IPV6_JOIN_GROUP, (void *)&group6, sizeof(group6)) != QAPI_OK)
            {
                QCLI_Printf(qcli_net_handle, "ERROR: Socket set option failure.\n");
                qapi_socketclose(p_tCxt->sock_local);
				return -3;
            }
        }
        else
        {
            struct ip_mreq group;
            group.imr_multiaddr = p_tCxt->params.rx_params.mcIpaddr;
            if(p_tCxt->params.rx_params.local_address)
            	group.imr_interface = p_tCxt->params.rx_params.local_address;
            else
            	group.imr_interface = p_tCxt->params.rx_params.mcRcvIf;
            if (qapi_setsockopt(p_tCxt->sock_local, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void *)&group, sizeof(group)) != QAPI_OK)
            {
                QCLI_Printf(qcli_net_handle, "ERROR: Socket set option failure.\n");
                qapi_socketclose(p_tCxt->sock_local);
				return -4;
            }
        }
    }
	
	start.addr = local_sin_addr;
	start.port = p_tCxt->params.rx_params.port;
	
	QCLI_Printf(qcli_net_handle, "****************************************************\n");
	QCLI_Printf(qcli_net_handle, "IOT %s RX Test\n", (p_rxtCxt->test_type == RX && p_rxtCxt->protocol == SSL) ? "SSL" : "UDP");
	QCLI_Printf(qcli_net_handle, "****************************************************\n");
	QCLI_Printf(qcli_net_handle, "Bind address: %s\n", ip_str);
	QCLI_Printf(qcli_net_handle, "Local port: %d\n", start->port);
	QCLI_Printf(qcli_net_handle, "Type benchquit to terminate test\n");
	QCLI_Printf(qcli_net_handle, "****************************************************\n");
	return 0;
}
#endif


/************************************************************************
* NAME: bench_udp_rx
*
* DESCRIPTION: Start throughput UDP server.
************************************************************************/
void bench_udp_rx(THROUGHPUT_CXT *p_tCxt)
{
    int32_t  received;
    int32_t  conn_sock;
    struct sockaddr *addr;
    int addrlen;
    int32_t fromlen;
    struct sockaddr *from;
	struct sockaddr_in  local_addr;
    struct sockaddr_in6 local_addr6;
	struct sockaddr_in  foreign_addr;
    struct sockaddr_in6 foreign_addr6;
    void *local_sin_addr;
    char ip_str[48];
    int family;
    uint16_t port;
    uint32_t iperf_display_interval = 0;
    uint32_t iperf_display_last = 0;
    uint32_t iperf_display_next = 0;
    uint32_t cur_packet_number = 0;
    uint64_t send_bytes = 0; /* for UDP echo */
    STATS echo_stats;
#ifdef CONFIG_NET_SSL_DEMO
    SSL_INST *ssl = bench_ssl_GetInstance(SSL_SERVER_INST);
    int status = 0;
#endif
	uint32_t  from_ip_address;
	
    if ((p_tCxt->buffer = qapi_Net_Buf_Alloc(CFG_PACKET_SIZE_MAX_RX, QAPI_NETBUF_APP)) == NULL)
    {
        QCLI_Printf(qcli_net_handle, "Out of memory error\n");
        goto ERROR_1;
    }

    port = p_tCxt->params.rx_params.port;

    if (p_tCxt->params.rx_params.v6)
    {
        family = AF_INET6;
        from = (struct sockaddr *)&foreign_addr6;
        addr = (struct sockaddr *)&local_addr6;
        local_sin_addr = &local_addr6.sin_addr;
        fromlen = addrlen = sizeof(struct sockaddr_in6);

        memset(&local_addr6, 0, sizeof(local_addr6));
        local_addr6.sin_port = htons(port);
        local_addr6.sin_family = family;
        memcpy(&local_addr6.sin_addr, p_tCxt->params.rx_params.local_v6addr, sizeof(ip6_addr));
    }
    else
    {
        family = AF_INET;
        from = (struct sockaddr *)&foreign_addr;
        addr = (struct sockaddr *)&local_addr;
        local_sin_addr = &local_addr.sin_addr;
        fromlen = addrlen = sizeof(struct sockaddr_in);

        memset(&local_addr, 0, sizeof(local_addr));
        local_addr.sin_port = htons(port);
        local_addr.sin_family = family;
        local_addr.sin_addr.s_addr = p_tCxt->params.rx_params.local_address;
    }

    /* Open socket */
    if ((p_tCxt->sock_local = qapi_socket(family, SOCK_DGRAM, 0))== A_ERROR)
    {
        QCLI_Printf(qcli_net_handle, "ERROR: Socket creation error.\n");
        goto ERROR_1;
    }

    /* Bind */
    if (qapi_bind( p_tCxt->sock_local, addr, addrlen) != QAPI_OK)
    {
        QCLI_Printf(qcli_net_handle, "ERROR: Socket bind error.\n");
        goto ERROR_2;
    }

    if (p_tCxt->params.rx_params.mcEnabled)
    {
        if (p_tCxt->params.rx_params.v6)
        {
            struct ipv6_mreq group6;
            memcpy(&group6.ipv6mr_multiaddr, p_tCxt->params.rx_params.mcIpv6addr, sizeof(ip6_addr));
            group6.ipv6mr_interface = p_tCxt->params.rx_params.scope_id;
            if (qapi_setsockopt(p_tCxt->sock_local, IPPROTO_IP, IPV6_JOIN_GROUP, (void *)&group6, sizeof(group6)) != QAPI_OK)
            {
                QCLI_Printf(qcli_net_handle, "ERROR: Socket set option failure.\n");
                goto ERROR_2;
            }
        }
        else
        {
            struct ip_mreq group;
            group.imr_multiaddr = p_tCxt->params.rx_params.mcIpaddr;
            if(p_tCxt->params.rx_params.local_address)
            	group.imr_interface = p_tCxt->params.rx_params.local_address;
            else
            	group.imr_interface = p_tCxt->params.rx_params.mcRcvIf;
            if (qapi_setsockopt(p_tCxt->sock_local, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void *)&group, sizeof(group)) != QAPI_OK)
            {
                QCLI_Printf(qcli_net_handle, "ERROR: Socket set option failure.\n");
                goto ERROR_2;
            }
        }
    }

    memset(ip_str,0,sizeof(ip_str));

    if (p_tCxt->is_iperf)
    {
        int32_t buffer_size = 0;
        qapi_getsockopt(p_tCxt->sock_local,SOL_SOCKET, SO_RCVBUF, &buffer_size, (int32_t *)sizeof(buffer_size));
        QCLI_Printf(qcli_net_handle, "------------------------------------------------------------\n");
        QCLI_Printf(qcli_net_handle, "Server listening on UDP port %d\n", port);
        QCLI_Printf(qcli_net_handle, "UDP buffer size:  %d\n",buffer_size);
        QCLI_Printf(qcli_net_handle, "------------------------------------------------------------\n");
    }
    else
    {
        /* ------ Start test.----------- */
        QCLI_Printf(qcli_net_handle, "****************************************************\n");
        QCLI_Printf(qcli_net_handle, "IOT %s RX Test\n", (p_tCxt->test_type == RX && p_tCxt->protocol == SSL) ? "SSL" : "UDP");
        QCLI_Printf(qcli_net_handle, "****************************************************\n");
        QCLI_Printf(qcli_net_handle, "Bind address: %s\n", inet_ntop(family, local_sin_addr, ip_str, sizeof(ip_str)));
        QCLI_Printf(qcli_net_handle, "Local port: %d\n", port);
        QCLI_Printf(qcli_net_handle, "Type benchquit to terminate test\n");
        QCLI_Printf(qcli_net_handle, "****************************************************\n");
    }

    /* The current implementation only supports a single DTLS client
     * per server.  This sets up the DTLS connection and associates
     * it with the local socket of the server.
     */
#ifdef CONFIG_NET_SSL_DEMO
    if (p_tCxt->test_type == RX && p_tCxt->protocol == SSL)
    {
		/* Setup SSL parameters and associate connection with socket */
		if (bench_ssl_rx_setup(ssl, p_tCxt->sock_local, &ssl->ssl, 1) < 0) {
			goto ERROR_2;
		}
    }
#endif
    if (p_tCxt->is_iperf) {
        iperf_display_interval = p_tCxt->pktStats.iperf_display_interval;
        iperf_display_last = p_tCxt->pktStats.first_time.seconds ;
        iperf_display_next = iperf_display_last + iperf_display_interval;
    }

    /* Configure queue sizes */
    bench_config_queue_size(p_tCxt->sock_local);

    memset(&echo_stats, 0, sizeof(STATS));

    while (!benchrx_quit) /* Main loop */
    {
        int32_t i = 0;
        int32_t is_first = 1;
        stat_udp_pattern_t stat_udp;

        QCLI_Printf(qcli_net_handle, "Waiting\n");

        bench_common_clear_stats(p_tCxt);
        memset(ip_str,0,sizeof(ip_str));
        rxreorder_udp_payload_init(&stat_udp);

        cur_packet_number = 0;
        send_bytes = 0; /* for UDP echo */

        while (!benchrx_quit)   /* Receive loop */
        {
            do
            {
                fd_set rset;

                if (benchrx_quit)
                {
					app_get_time(&p_tCxt->pktStats.last_time);
                    goto ERROR_3;
                }

                /* block for 500msec or until a packet is received */
                qapi_fd_zero(&rset);
                qapi_fd_set(p_tCxt->sock_local, &rset);

                conn_sock = qapi_select(&rset, NULL, NULL, 500);
                if (conn_sock == A_ERROR)
                    goto ERROR_3;       // socket no longer valid

            } while (conn_sock == 0);

            /* Receive data */
#ifdef CONFIG_NET_SSL_DEMO
            if (p_tCxt->test_type == RX && p_tCxt->protocol == SSL)
            {
				received = qapi_Net_SSL_Read_From(ssl->ssl, (char*)(&p_tCxt->buffer[0]), CFG_PACKET_SIZE_MAX_RX, from, &fromlen);
            }
            else
#endif
            {
                received = qapi_recvfrom( p_tCxt->sock_local,
                        (char*)(&p_tCxt->buffer[0]),
                        CFG_PACKET_SIZE_MAX_RX, 0,
                        from, &fromlen);
            }

            ++i;

            if (received >= 0)
            {

                if (p_tCxt->is_iperf)
                {
                    if (p_tCxt->buffer[0] == 0xFF)
                    {
                        int send_result;
                        uint32_t retry = 40;
                        app_get_time(&p_tCxt->pktStats.last_time);

                        while (retry)
                        {
                            send_result = qapi_sendto(p_tCxt->sock_local, p_tCxt->buffer, received, 0, from, fromlen);
                            if ( send_result != received )
                            {
                                QCLI_Printf(qcli_net_handle, "%d error while sending ack packet, e=%d\n", retry, send_result);
                                qapi_Task_Delay(1000);
                            }
                            else
                            {
                                break;
                            }
                            retry--;
                        }
                        break;
                    }
                }
                
                if (received > 8)
            	{
					from_ip_address = *((int32_t*)(&p_tCxt->buffer[0])+1);
					if ((htonl(from_ip_address) & 0xff) == 0xff)
					{
						g_cookie_mc = *((int32_t*)(&p_tCxt->buffer[0]));
					}
					else
	            	{
	            		g_cookie_uc = *((int32_t*)(&p_tCxt->buffer[0]));
					}
            	}

                if (received != sizeof(EOT_PACKET) ||
                    ((EOT_PACKET *)p_tCxt->buffer)->code != (int)END_OF_TEST_CODE)
                {
#ifdef UDP_RX_STATUS_DEBUG
                    time_struct_t ts;

                    app_get_time(&ts);
                    QCLI_Printf(qcli_net_handle, "%d sec=%u ms=%u rcvd=%u\n", i, ts.seconds, ts.milliseconds, received);
#endif
                    p_tCxt->pktStats.bytes += received;
                    ++p_tCxt->pktStats.pkts_recvd;
                    rxreorder_udp_payload_statistics(&stat_udp,
                        p_tCxt->buffer, received);
                    if (is_first)
                    {
#ifdef CONFIG_NET_SSL_DEMO
                        if (p_tCxt->test_type == RX && p_tCxt->protocol == SSL) {
                            const char* alpn_protocol = qapi_Net_SSL_ALPN_Get_Protocol_For_Peer(ssl->ssl, from, fromlen);
                            if (alpn_protocol) {
                                QCLI_Printf(qcli_net_handle, "Negotiated Application Protocol: %s\n", alpn_protocol);
                            }
						}
#endif
                        QCLI_Printf(qcli_net_handle, "Receiving\n");

                        app_get_time(&p_tCxt->pktStats.first_time);

#ifdef UDP_RX_TS_DEBUG
                        QCLI_Printf(qcli_net_handle, "first_time: sec=%u ms=%u\n",
                                p_tCxt->pktStats.first_time.seconds,
                                p_tCxt->pktStats.first_time.milliseconds);
#endif

                        is_first = 0;
                    }

                    if (p_tCxt->print_buf)
                    	bench_print_buffer(p_tCxt->buffer, received, from, DUMP_DIRECTION_RX);

                    if (p_tCxt->echo) {
                        int ret;

                        /* Echo the buffer back to the sender (best effort, no retransmission). */
                        ret = qapi_sendto(p_tCxt->sock_local, p_tCxt->buffer, received, MSG_DONTWAIT, from, fromlen);

                        if (ret > 0) {
                            send_bytes += ret;
                            cur_packet_number++;
                        }
                    }

                    if (p_tCxt->is_iperf && p_tCxt->pktStats.iperf_display_interval)
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
                }
                else if (!is_first) /* End of transfer. */
                {
                    app_get_time(&p_tCxt->pktStats.last_time);

                    if (rxreorder_udp_payload_valid(&stat_udp)) {
                        stat_udp.pkts_plan = ((EOT_PACKET *)p_tCxt->buffer)->packet_count;
                    }

#ifdef UDP_RX_TS_DEBUG
                    QCLI_Printf(qcli_net_handle, "last_time: sec=%u ms=%u rcvd=%u\n",
                            p_tCxt->pktStats.last_time.seconds,
                            p_tCxt->pktStats.last_time.milliseconds, received);
#endif

                    /* Send throughput results to Peer so that it can display correct results*/
                    send_ack(p_tCxt, from, fromlen);

#ifdef CONFIG_NET_SSL_DEMO
					/* Since we know at the application layer that the client disconnected,
					 * we an explicitly close the peer's DTLS connection even if the client
                     * didn't send us a close notify.  If the client already closed,
                     * a status of QAPI_ERR_SSL_CONN_NOT_FOUND will be return which we can
                     * ignore.
					 */
			        if (p_tCxt->protocol == SSL && ssl->ssl != QAPI_NET_SSL_INVALID_HANDLE)
			        {
						status = qapi_Net_SSL_Close_Client_Con(ssl->ssl, from, fromlen);
						if((status != QAPI_OK) && (status != QAPI_ERR_SSL_CONN_NOT_FOUND))
						{
							goto ERROR_2;
						}
			        }
#endif
                    break;
                }
            }
            else
            {
                QCLI_Printf(qcli_net_handle, "%d received= %d\n", i, received);
                break;
            }
        } /* receive_loop */

ERROR_3:
        QCLI_Printf(qcli_net_handle, "Received %llu bytes, Packets %u  from %s:%d\n",
                p_tCxt->pktStats.bytes, 
                p_tCxt->pktStats.pkts_recvd,
                ip_str,
                ntohs(from->sa_port));

        if (rxreorder_udp_payload_valid(&stat_udp)) {
            stat_udp.pkts_recvd = p_tCxt->pktStats.pkts_recvd;
            rxreorder_udp_payload_report(&stat_udp);
        }

        if (p_tCxt->is_iperf)
        {
            iperf_result_print(&p_tCxt->pktStats, 0,0);
            break;
        }
        else
        {
            app_get_time(&p_tCxt->pktStats.last_time);
            bench_common_print_test_results(p_tCxt, &p_tCxt->pktStats);

            if (p_tCxt->echo)
            {
                if (send_bytes > 0)
                {
                    echo_stats = p_tCxt->pktStats;
                    echo_stats.bytes = send_bytes;
                }

                QCLI_Printf(qcli_net_handle, "\nSent %u packets, %llu bytes\n",
                        cur_packet_number, send_bytes);

                p_tCxt->test_type = TX;
                bench_common_print_test_results(p_tCxt, &echo_stats);
                p_tCxt->test_type = RX;
            }
        }

    } /* main loop */

ERROR_2:
#ifdef CONFIG_NET_SSL_DEMO
    if (ssl->ssl != QAPI_NET_SSL_INVALID_HANDLE)
	{
    	qapi_Net_SSL_Shutdown(ssl->ssl);
        ssl->ssl = QAPI_NET_SSL_INVALID_HANDLE;
	}
#endif
	qapi_socketclose(p_tCxt->sock_local);

ERROR_1:

	bench_udp_rx_port_in_use = 0;

    if (!p_tCxt->is_iperf)
    {
        QCLI_Printf(qcli_net_handle, BENCH_TEST_COMPLETED);
    }

    if (p_tCxt->buffer)
        qapi_Net_Buf_Free(p_tCxt->buffer, QAPI_NETBUF_APP);

    return;
}

/*****************************************************************************
 *****************************************************************************/
void bench_udp_rx_zc(THROUGHPUT_CXT *p_tCxt)
{
    int32_t received;
    int32_t conn_sock;
    struct sockaddr *addr;
    int addrlen;
    int32_t fromlen;
    struct sockaddr *from;
    void *sin_addr, *local_sin_addr;
	struct sockaddr_in  local_addr;
    struct sockaddr_in6 local_addr6;
	struct sockaddr_in  foreign_addr;
    struct sockaddr_in6 foreign_addr6;
    char ip_str[48];
    PACKET pkt;
    UDP_ZC_RX_INFO *p;
    int family;
#ifdef CONFIG_NET_SSL_DEMO
    SSL_INST *ssl = bench_ssl_GetInstance(SSL_SERVER_INST);
#endif
    uint16_t port;

	memset(ip_str,0,sizeof(ip_str));

    port = p_tCxt->params.rx_params.port;

    if (p_tCxt->params.rx_params.v6)
    {
        family = AF_INET6;
        from = (struct sockaddr *)&foreign_addr6;
        sin_addr = &foreign_addr6.sin_addr;
        addr = (struct sockaddr *)&local_addr6;
        local_sin_addr = &local_addr6.sin_addr;
        fromlen = addrlen = sizeof(struct sockaddr_in6);

        memset(&local_addr6, 0, sizeof(local_addr6));
        local_addr6.sin_port   = htons(port);
        local_addr6.sin_family = family;
        memcpy(&local_addr6.sin_addr, p_tCxt->params.rx_params.local_v6addr, sizeof(ip6_addr));
    }
    else
    {
        family = AF_INET;
        from = (struct sockaddr *)&foreign_addr;
        sin_addr = &foreign_addr.sin_addr;
        addr = (struct sockaddr *)&local_addr;
        local_sin_addr = &local_addr.sin_addr;
        fromlen = addrlen = sizeof(struct sockaddr_in);

        memset(&local_addr, 0, sizeof(local_addr));
        local_addr.sin_port     = htons(port);
        local_addr.sin_family   = family;
        local_addr.sin_addr.s_addr = p_tCxt->params.rx_params.local_address;
    }

    /* Open socket */
    if ((p_tCxt->sock_local = qapi_socket(family, SOCK_DGRAM, 0)) == A_ERROR)
    {
        QCLI_Printf(qcli_net_handle, "ERROR: Socket creation error.\n");
        goto ERROR_1;
    }

    /* Bind */
    if (qapi_bind(p_tCxt->sock_local, addr, addrlen) != QAPI_OK)
    {
        QCLI_Printf(qcli_net_handle, "ERROR: Socket bind error.\n");
        goto ERROR_2;
    }

    if (p_tCxt->params.rx_params.mcEnabled)
    {
        if (p_tCxt->params.rx_params.v6)
        {
            struct ipv6_mreq group6;
            memcpy(&group6.ipv6mr_multiaddr, p_tCxt->params.rx_params.mcIpv6addr, sizeof(ip6_addr));
            group6.ipv6mr_interface = p_tCxt->params.rx_params.scope_id;
            if (qapi_setsockopt(p_tCxt->sock_local, IPPROTO_IP, IPV6_JOIN_GROUP, (void *)&group6, sizeof(group6)) != QAPI_OK)
            {
                QCLI_Printf(qcli_net_handle, "ERROR: Socket set option failure.\n");
                goto ERROR_2;
            }
        }
        else
        {
            struct ip_mreq group;
            group.imr_multiaddr = p_tCxt->params.rx_params.mcIpaddr;
            if(p_tCxt->params.rx_params.local_address)
            	group.imr_interface = p_tCxt->params.rx_params.local_address;
            else
            	group.imr_interface = p_tCxt->params.rx_params.mcRcvIf;
            if (qapi_setsockopt(p_tCxt->sock_local, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void *)&group, sizeof(group)) != QAPI_OK)
            {
                QCLI_Printf(qcli_net_handle, "ERROR: Socket set option failure.\n");
                goto ERROR_2;
            }
        }
    }

    memset(ip_str,0,sizeof(ip_str));

    /* ------ Start test.----------- */
    QCLI_Printf(qcli_net_handle, "****************************************************\n");
    QCLI_Printf(qcli_net_handle, "IOT %s Zero-Copy RX Test\n", (p_tCxt->test_type == RX && p_tCxt->protocol == SSL) ? "SSL" : "UDP");
    QCLI_Printf(qcli_net_handle, "****************************************************\n");
    QCLI_Printf(qcli_net_handle, "Bind address %s\n", inet_ntop(family, local_sin_addr, ip_str, sizeof(ip_str)));
    QCLI_Printf(qcli_net_handle, "Local port %d\n", port);
    QCLI_Printf(qcli_net_handle, "Type benchquit to terminate test\n");
    QCLI_Printf(qcli_net_handle, "****************************************************\n");

	if (p_tCxt->params.rx_params.v6) {
		from = (struct sockaddr *)&foreign_addr6;
		sin_addr = &foreign_addr6.sin_addr;
		addr = (struct sockaddr *)&local_addr6;
		memset(&local_addr6, 0, sizeof(local_addr6));
        local_addr6.sin_port = htons(p_tCxt->params.rx_params.port);
        local_addr6.sin_family = family;
        memcpy(&local_addr6.sin_addr, p_tCxt->params.rx_params.local_v6addr, sizeof(ip6_addr));
	}
	else {
		from = (struct sockaddr *)&foreign_addr;
		sin_addr = &foreign_addr.sin_addr;
		addr = (struct sockaddr *)&local_addr;
		memset(&local_addr, 0, sizeof(local_addr));
        local_addr.sin_port = htons(p_tCxt->params.rx_params.port);
        local_addr.sin_family = family;
        local_addr.sin_addr.s_addr = p_tCxt->params.rx_params.local_address;
	}

#ifdef CONFIG_NET_SSL_DEMO
    /* The callback will be set later for SSL. */
    if(!(p_tCxt->test_type == RX && p_tCxt->protocol == SSL))
    {
        /* Set rx callback */
        if (qapi_setsockopt(p_tCxt->sock_local, 0, SO_UDPCALLBACK, (void *)udp_rx_upcall, 0) != QAPI_OK)
        {
            QCLI_Printf(qcli_net_handle, "ERROR: Socket callback\n");
            goto ERROR_2;
        }
    }
#endif

    while (!benchrx_quit)   /* main loop */
    {
        int32_t i = 0;
        int32_t isfirst = 1;
        int32_t eot = 0;

        QCLI_Printf(qcli_net_handle, "Waiting\n");

        p_tCxt->pktStats.bytes = 0;
        p_tCxt->pktStats.pkts_recvd = 0;
        memset(ip_str, 0, sizeof(ip_str));

        while (!benchrx_quit)   /* Receive loop */
        {
            do
            {
                fd_set rset;

                if (benchrx_quit)
                {
					app_get_time(&p_tCxt->pktStats.last_time);
                    goto ERROR_3;
                }

                /* block for 500msec or until a packet is received */
                qapi_fd_zero(&rset);
                qapi_fd_set(p_tCxt->sock_local, &rset);
                conn_sock = qapi_select(&rset, NULL, NULL, 500);

            } while (conn_sock == 0);

#ifdef CONFIG_NET_SSL_DEMO
            if ((p_tCxt->test_type == RX) && (p_tCxt->protocol == SSL) && (ssl->ssl == QAPI_NET_SSL_INVALID_HANDLE))
            {
                received = qapi_recvfrom( p_tCxt->sock_local,
                                          NULL, 0,
                                          MSG_PEEK,
                                          from, &fromlen);
                if (received < 0)
                    goto ERROR_2;

                /* Bind this socket to the remote address, so DTLS packets
                 * will be sent there without specifying the address in
                 * every sendto call.
                 */
                qapi_connect(p_tCxt->sock_local, from, fromlen);

                if (bench_ssl_rx_setup(ssl, p_tCxt->sock_local, &ssl->ssl, 1) < 0)
                    goto ERROR_2;

                /* Set rx callback */
                if (qapi_setsockopt(p_tCxt->sock_local, 0, SO_UDPCALLBACK, (void *)udp_rx_upcall, 0) != QAPI_OK)
                {
                    QCLI_Printf(qcli_net_handle, "ERROR: Socket callback\n");
                    goto ERROR_2;
                }

                continue;
            }
#endif

#ifdef ZCOPY_UDP_RX_DEBUG
            QCLI_Printf(qcli_net_handle, "conn_sock=%d\n", conn_sock);
#endif
            /* Dequeue pkt */
            while ((p = (UDP_ZC_RX_INFO *)dequeue(&udp_zcq)) != NULL)
            {
                pkt = p->pkt;
                received = pkt->nb_Tlen;

                if (received != sizeof(EOT_PACKET) ||
                    ((EOT_PACKET *)pkt->nb_Prot)->code != (int)END_OF_TEST_CODE)
                {
                    ++i;
                    ++p_tCxt->pktStats.pkts_recvd;
                    p_tCxt->pktStats.bytes += received;

                    if (isfirst)
                    {
                        QCLI_Printf(qcli_net_handle, "Receiving\n");
                        memcpy(from, &p->from, fromlen);
                        app_get_time(&p_tCxt->pktStats.first_time);
                        isfirst = 0;
                    }

#ifdef ZCOPY_UDP_RX_DEBUG
                    QCLI_Printf(qcli_net_handle, "%d Receiving %u bytes from %s:%d. total=%llu pkt=0x%p\n",
                            i, received,
                            inet_ntop(family, sin_addr, ip_str, sizeof(ip_str)),
                            ntohs(from->sa_port),
                            p_tCxt->bytes, pkt);
#endif
                }
                else /* end of transfer. */
                {
                    app_get_time(&p_tCxt->pktStats.last_time);
                    eot = 1;
                }

                /* App has to free the pkt */
                qapi_Net_Buf_Free(pkt, QAPI_NETBUF_SYS);
                qapi_Net_Buf_Free(p, QAPI_NETBUF_APP);
            } /* dequeue loop */

            if (eot)
            {
                send_ack_zc(p_tCxt, from, fromlen);
                break;
            }
        } /* receive_loop */

ERROR_3:
        //app_get_time(&p_tCxt->last_time);
        /* check if there are pkts on the queue and free them ! */
        if (udp_zcq.q_len)
        {
            QCLI_Printf(qcli_net_handle, "There are still %u pkts on queue.\n", udp_zcq.q_len);
            while ((p = (UDP_ZC_RX_INFO *)dequeue(&udp_zcq)) != NULL)
            {
                pkt = p->pkt;
                qapi_Net_Buf_Free(pkt, QAPI_NETBUF_SYS);
                qapi_Net_Buf_Free(p, QAPI_NETBUF_APP);
            }
        }

        QCLI_Printf(qcli_net_handle, "Received %llu bytes from %s:%d\n",
                p_tCxt->pktStats.bytes,
                inet_ntop(family, sin_addr, ip_str, sizeof(ip_str)),
                ntohs(from->sa_port));

        bench_common_print_test_results (p_tCxt, &p_tCxt->pktStats);

#ifdef CONFIG_NET_SSL_DEMO
        if (ssl->ssl != QAPI_NET_SSL_INVALID_HANDLE)
        {
            qapi_Net_SSL_Shutdown(ssl->ssl);
            ssl->ssl = QAPI_NET_SSL_INVALID_HANDLE;
        }
#endif

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
	bench_udp_rx_port_in_use = 0;
    QCLI_Printf(qcli_net_handle, BENCH_TEST_COMPLETED);

    return;
}

/************************************************************************
* NAME: bench_common_wait_for_response
*
* DESCRIPTION: In UDP uplink test, the test is terminated by transmitting
* end-mark (single byte packet). We have implemented a feedback mechanism
* where the Peer will reply with receive stats allowing us to display correct
* test results.
* Parameters: pointer to throughput context
************************************************************************/
int bench_common_wait_for_response(THROUGHPUT_CXT *p_tCxt, struct sockaddr *to, uint32_t tolen, uint32_t cur_packet_number)
{
    uint32_t received;
    int error = A_ERROR;
    struct sockaddr_in local_addr;
    struct sockaddr_in6 local_addr6;
    struct sockaddr *addr;
    uint32_t addrlen;
    stat_packet_t *stat_packet, stats;
    EOT_PACKET eot_packet, *endmark;
    uint32_t retry_counter = 0;
#ifdef CONFIG_NET_SSL_DEMO
    SSL_INST *ssl = bench_ssl_GetInstance(SSL_CLIENT_INST);
#endif
    int family = (int)to->sa_family;

    stat_packet = &stats;

    if (p_tCxt->test_type == TX && p_tCxt->protocol == UDP)
    {
        if (family == AF_INET)
        {
            memset(&local_addr, 0, sizeof(local_addr));
            /* Receive peer's ACK at original dest port */
            local_addr.sin_port = ((struct sockaddr_in *)to)->sin_port;
            local_addr.sin_family = family;
            addr = (struct sockaddr *)&local_addr;
            addrlen = sizeof(struct sockaddr_in);
        }
        else
        {
            memset(&local_addr6, 0, sizeof(local_addr6));
            local_addr6.sin_port = ((struct sockaddr_in6 *)to)->sin_port;
            local_addr6.sin_family = family;
            addr = (struct sockaddr *)&local_addr6;
            addrlen = sizeof(struct sockaddr_in6);
        }

        /* Create socket */
        if ((p_tCxt->sock_local = qapi_socket(family, SOCK_DGRAM, 0)) == A_ERROR)
        {
            QCLI_Printf(qcli_net_handle, "%s: Socket creation error\n", __func__);
            goto ERROR_1;
        }

        /* Bind */
        if (qapi_bind(p_tCxt->sock_local, addr, addrlen) != QAPI_OK)
        {
            QCLI_Printf(qcli_net_handle, "%s: Socket bind error\n", __func__);
            goto ERROR_2;
        }
    }

    if (p_tCxt->test_type == TX && (p_tCxt->protocol == IP_RAW || p_tCxt->protocol == IP_RAW_HDR))
    {
        /* Same RAW socket can be used to recv ACK */
        p_tCxt->sock_local = p_tCxt->sock_peer;

        /* Turn off IP_HDRINCL */
        int off = 0;
        qapi_setsockopt(p_tCxt->sock_peer, IPPROTO_IP, IP_HDRINCL, &off, sizeof(off));
    }

    if (p_tCxt->is_iperf)
    {
        uint32_t iperf_term_pkt;
        uint32_t iperfPktCnt = 0;

        iperfPktCnt = cur_packet_number-1;
        iperfPktCnt = ~iperfPktCnt;

        /*
        time_struct_t iperf_curr_time;
        app_get_time(&iperf_curr_time);
        uint32_t cur_time = iperf_curr_time.seconds * 1000 + iperf_curr_time.milliseconds;   
        */
        
        iperf_term_pkt =  htonl(iperfPktCnt);
        
        /*
        *((uint32_t*)iperf_term_pkt + 2) = cur_time;
        */
        
        while (retry_counter < 10)
        {
            int sent_bytes = 0;

            sent_bytes = qapi_sendto( p_tCxt->sock_peer, (char*)&iperf_term_pkt, sizeof(iperf_term_pkt), 0, to, tolen) ;

            if (sent_bytes < 0) 
            {
                QCLI_Printf(qcli_net_handle, "UDP send terminate packet error %d , retry %d \r\n", sent_bytes, retry_counter);

                retry_counter ++;
                qapi_Task_Delay(100000);
            } 
            else 
            {
                error = QAPI_OK;
                break;
            }
            
        }
    }
    else
    {
    while (retry_counter < 10)
    {
        int conn_sock, sent_bytes;
        fd_set rset;

        endmark = &eot_packet;

        /* Send endmark packet */
        ((EOT_PACKET*)endmark)->code            = HOST_TO_LE_LONG(END_OF_TEST_CODE);
        ((EOT_PACKET*)endmark)->packet_count    = htonl(cur_packet_number);

#ifdef CONFIG_NET_SSL_DEMO
        if (p_tCxt->protocol == SSL && p_tCxt->test_type == TX)
        {
            sent_bytes = qapi_Net_SSL_Write(ssl->ssl, (char *)endmark, sizeof(EOT_PACKET));
        }
        else
#endif
        {
            sent_bytes = qapi_sendto( p_tCxt->sock_peer, (char *)endmark, sizeof(EOT_PACKET), 0, to, tolen) ;
        }

        if (sent_bytes != sizeof(EOT_PACKET))
        	break;

        //QCLI_Printf(qcli_net_handle, "%d sent EOT_PACKET %d bytes\n", retry_counter, sent_bytes);

        FD_ZERO(&rset);

#ifdef CONFIG_NET_SSL_DEMO
        if (p_tCxt->protocol == SSL && p_tCxt->test_type == TX)
        {
            /* SSL will expect the ACK to come back over the existing SSL
             * channel on the original socket.
             */
            FD_SET(p_tCxt->sock_peer, &rset);
        }
        else
#endif
        {
            FD_SET(p_tCxt->sock_local, &rset);
        }

        if ((conn_sock = qapi_select(&rset, NULL, NULL, 200)) > 0)
        {
            /* Receive data */
#ifdef CONFIG_NET_SSL_DEMO
            if (p_tCxt->protocol == SSL && p_tCxt->test_type == TX)
            {
                received = qapi_Net_SSL_Read(ssl->ssl, (char *)stat_packet, sizeof(stat_packet_t));
            }
            else
#endif
            {
                received = qapi_recvfrom( p_tCxt->sock_local, (char*)(stat_packet), sizeof(stat_packet_t), 0, NULL, NULL);
            }

            if (received == sizeof(stat_packet_t))
            {
                QCLI_Printf(qcli_net_handle, "%d received %u-byte statistics\n", retry_counter, received);
                error = QAPI_OK;

#ifdef USE_SERVER_STATS
                /*Response received from peer, extract test statistics*/
                stat_packet->msec = HOST_TO_LE_LONG(stat_packet->msec);
                stat_packet->kbytes = HOST_TO_LE_LONG(stat_packet->kbytes);
                stat_packet->bytes = HOST_TO_LE_LONG(stat_packet->bytes);
                stat_packet->numPackets = HOST_TO_LE_LONG(stat_packet->numPackets);

                p_tCxt->pktStats.first_time.seconds = p_tCxt->pktStats.last_time.seconds = 0;
                p_tCxt->pktStats.first_time.milliseconds = 0;
                p_tCxt->pktStats.last_time.milliseconds = stat_packet->msec;
                p_tCxt->bytes = stat_packet->kbytes*1024 + stat_packet->bytes;
#endif
                break;
            }
            else
            {
                QCLI_Printf(qcli_net_handle, "%d Did not receive response: %d\n", retry_counter, received);
                retry_counter++;
            }
        }
        else
        {
            //QCLI_Printf(qcli_net_handle, "%d conn_sock=%d ms=%u\n", retry_counter, conn_sock, app_get_time(NULL));
            retry_counter++;
        }
    } /* while(retry_counter) */
    }

ERROR_2:
    if (p_tCxt->protocol == UDP && p_tCxt->test_type == TX)
    {
        qapi_socketclose(p_tCxt->sock_local);
    }

ERROR_1:
    return error;
}

/************************************************************************
* NAME: bench_udp_tx
*
* DESCRIPTION: Start TX UDP throughput test.
************************************************************************/
void bench_udp_tx(THROUGHPUT_CXT *p_tCxt)
{
    struct sockaddr_in foreign_addr;
    struct sockaddr_in6 foreign_addr6;
    struct sockaddr *to;
    uint32_t tolen;
    char ip_str [48];
    int32_t send_bytes, result;
    uint32_t packet_size = p_tCxt->params.tx_params.packet_size;
    uint32_t cur_packet_number, i, n_send_ok;
    uint32_t netbuf_id;
    int send_flag;
    int family;
    int tos_opt;
    uint32_t zerocopy_send;
#ifdef CONFIG_NET_SSL_DEMO
    SSL_INST *ssl = bench_ssl_GetInstance(SSL_CLIENT_INST);
    uint32_t dtls_data_mtu = 0;
#endif
    struct sockaddr_in src_sin;

    /* iperf display related */
    uint32_t iperf_display_interval = 0;
    time_struct_t iperf_display_last;

    /* iperf bandwidth limitation */
    uint32_t iperf_udp_packets_per_second = 0;
    uint32_t iperf_udp_packets_counter = 0;
    time_struct_t iperf_udp_start_time;

    if (p_tCxt->params.tx_params.v6)
    {
        family = AF_INET6;
        inet_ntop(family, p_tCxt->params.tx_params.v6addr, ip_str, sizeof(ip_str));

        memset(&foreign_addr6, 0, sizeof(foreign_addr6));
        memcpy(&foreign_addr6.sin_addr, p_tCxt->params.tx_params.v6addr, sizeof(foreign_addr6.sin_addr));
        foreign_addr6.sin_port      = htons(p_tCxt->params.tx_params.port);
        foreign_addr6.sin_family    = family;
        foreign_addr6.sin_scope_id  = p_tCxt->params.tx_params.scope_id;

        to = (struct sockaddr *)&foreign_addr6;
        tolen = sizeof(foreign_addr6);
        tos_opt = IPV6_TCLASS;
    }
    else
    {
        family = AF_INET;
        inet_ntop(family, &p_tCxt->params.tx_params.ip_address, ip_str, sizeof(ip_str));

        memset(&foreign_addr, 0, sizeof(foreign_addr));
        foreign_addr.sin_addr.s_addr    = p_tCxt->params.tx_params.ip_address;
        foreign_addr.sin_port           = htons(p_tCxt->params.tx_params.port);
        foreign_addr.sin_family         = family;

        src_sin.sin_family              = family;
        src_sin.sin_addr.s_addr         = p_tCxt->params.tx_params.source_ipv4_addr;
        src_sin.sin_port                = htons(0);

        to = (struct sockaddr *)&foreign_addr;
        tolen = sizeof(foreign_addr);
        tos_opt = IP_TOS;
    }

    zerocopy_send = p_tCxt->params.tx_params.zerocopy_send;

    if (p_tCxt->is_iperf)
    {
        QCLI_Printf(qcli_net_handle, "------------------------------------------------------------\n");        
        QCLI_Printf(qcli_net_handle, "Client connecting to %s, UDP port %d\n",ip_str, p_tCxt->params.tx_params.port);
        QCLI_Printf(qcli_net_handle, "------------------------------------------------------------\n");

    }
    else
    {
        /* ------ Start test.----------- */
        QCLI_Printf(qcli_net_handle, "****************************************************************\n");
        QCLI_Printf(qcli_net_handle, "IOT %s TX Test\n", (p_tCxt->protocol == SSL && p_tCxt->test_type == TX) ? "SSL" : "UDP");
        QCLI_Printf(qcli_net_handle, "****************************************************************\n");
        QCLI_Printf(qcli_net_handle, "Remote IP addr: %s\n", ip_str);
        QCLI_Printf(qcli_net_handle, "Remote port: %d\n", p_tCxt->params.tx_params.port);
        QCLI_Printf(qcli_net_handle, "Message size: %d\n", p_tCxt->params.tx_params.packet_size);
        QCLI_Printf(qcli_net_handle, "Number of messages: %d\n", p_tCxt->params.tx_params.packet_number);
        QCLI_Printf(qcli_net_handle, "Delay in microseconds: %u\n", p_tCxt->params.tx_params.interval_us);
        QCLI_Printf(qcli_net_handle, "Zerocopy send: %s\n", zerocopy_send != 0 ? "Yes" : "No");
        QCLI_Printf(qcli_net_handle, "Type benchquit to terminate test\n");
        QCLI_Printf(qcli_net_handle, "****************************************************************\n");
    }

    /* Create UDP socket */
    if ((p_tCxt->sock_peer = qapi_socket(family, SOCK_DGRAM, 0)) == A_ERROR)
    {
        QCLI_Printf(qcli_net_handle, "Socket creation failed\n");
        goto ERROR_1;
    }

	if (p_tCxt->params.tx_params.source_ipv4_addr != 0) {
		if (qapi_bind(p_tCxt->sock_peer, (struct sockaddr*)&src_sin, sizeof(src_sin)) == A_ERROR) {
			QCLI_Printf(qcli_net_handle, "Socket bind failed\n");
			goto ERROR_2;
		}
	}

    if (p_tCxt->params.tx_params.ip_tos > 0)
    {
	    qapi_setsockopt(p_tCxt->sock_peer, IP_OPTIONS, tos_opt, &p_tCxt->params.tx_params.ip_tos, sizeof(uint8_t));
    }

    if (p_tCxt->params.tx_params.v6 && QAPI_IS_IPV6_MULTICAST(p_tCxt->params.tx_params.v6addr))
    {
    	uint32_t val;

        /* Configure value to be used in the Hop Limit field in IPv6 header of
         * outgoing multicast datagrams
         */
        val = 16;
        if (qapi_setsockopt(p_tCxt->sock_peer, IPPROTO_IP, IPV6_MULTICAST_HOPS, &val,
                          sizeof(int)) < 0)
        {
        	goto ERROR_2;
        }

        /* disable local loopback of outgoing multicast datagrams */
        val = IPV6_MC_LPBK_DIS;
        if (qapi_setsockopt(p_tCxt->sock_peer, IPPROTO_IP, IPV6_MULTICAST_LOOP, &val,
                          sizeof(unsigned int)) < 0)
        {
        	goto ERROR_2;
        }

        if (qapi_setsockopt(p_tCxt->sock_peer, IPPROTO_IP, IPV6_MULTICAST_IF, &foreign_addr6.sin_scope_id,
                          sizeof(unsigned int)) < 0)
        {
        	goto ERROR_2;
        }
    }

    /* Configure queue sizes */
    bench_config_queue_size(p_tCxt->sock_peer);

    /* Connect to the server.*/
    QCLI_Printf(qcli_net_handle, "Connecting\n");
    if (qapi_connect( p_tCxt->sock_peer, to, tolen) == A_ERROR)
    {
        QCLI_Printf(qcli_net_handle, "Connection failed\n");
        goto ERROR_2;
    }

#ifdef CONFIG_NET_SSL_DEMO
    if (p_tCxt->protocol == SSL && p_tCxt->test_type == TX)
    {
        if (ssl->ssl == QAPI_NET_SSL_INVALID_HANDLE)
        {
            // Create SSL connection object
            ssl->ssl = qapi_Net_SSL_Con_New(ssl->sslCtx, QAPI_NET_SSL_DTLS_E);
            if (ssl->ssl == QAPI_NET_SSL_INVALID_HANDLE)
            {
                QCLI_Printf(qcli_net_handle, "ERROR: Unable to create SSL context\n");
                goto ERROR_2;
            }

            // configure the SSL connection
            if (ssl->config_set)
            {
                result = qapi_Net_SSL_Configure(ssl->ssl, &ssl->config);
                if (result < QAPI_OK)
                {
                    QCLI_Printf(qcli_net_handle, "ERROR: SSL configure failed (%d)\n", result);
                    goto ERROR_2;
                }
            }
        }

        // Add socket handle to SSL connection
        result = qapi_Net_SSL_Fd_Set(ssl->ssl, p_tCxt->sock_peer);
        if (result < 0)
        {
            QCLI_Printf(qcli_net_handle, "ERROR: Unable to add socket handle to SSL (%d)\n", result);
            goto ERROR_2;
        }

        // SSL handshake with server
        app_get_time(&p_tCxt->pktStats.first_time);
        result = qapi_Net_SSL_Connect(ssl->ssl);
        app_get_time(&p_tCxt->pktStats.last_time);
        QCLI_Printf(qcli_net_handle, "DTLS Handshake time: %d ms\n", app_get_time_difference(&p_tCxt->pktStats.first_time, &p_tCxt->pktStats.last_time));

        if (result < 0)
        {
            if (result == QAPI_SSL_OK_HS)
            {
                /** The peer's SSL certificate is trusted, CN matches the host name, time is valid */
                QCLI_Printf(qcli_net_handle, "The certificate is trusted\n");
            }
            else if (result == QAPI_ERR_SSL_CERT_CN)
            {
                /** The peer's SSL certificate is trusted, CN matches the host name, time is expired */
                QCLI_Printf(qcli_net_handle, "ERROR: The certificate is expired\n");
                goto ERROR_2;
            }
            else if (result == QAPI_ERR_SSL_CERT_TIME)
            {
                /** The peer's SSL certificate is trusted, CN does NOT match the host name, time is valid */
                QCLI_Printf(qcli_net_handle, "ERROR: The certificate is trusted, but the host name is not valid\n");
                goto ERROR_2;
            }
            else if (result == QAPI_ERR_SSL_CERT_NONE)
            {
                /** The peer's SSL certificate is trusted, CN does NOT match host name, time is expired */
                QCLI_Printf(qcli_net_handle, "ERROR: The certificate is expired and the host name is not valid\n");
                goto ERROR_2;
            }
            else
            {
                QCLI_Printf(qcli_net_handle, "ERROR: SSL connect failed (%d)\n", result);
                goto ERROR_2;
            }   
        }

        dtls_data_mtu = qapi_Net_SSL_DTLS_Client_Get_Data_MTU(ssl->ssl);
        if(packet_size > dtls_data_mtu) {
        	packet_size = dtls_data_mtu;
            QCLI_Printf(qcli_net_handle, "Reducing packet size to %d to avoid IP fragmentation \n", dtls_data_mtu);
        }
    }

    const char* alpn_protocol = qapi_Net_SSL_ALPN_Protocol_Get(ssl->ssl);
    if (alpn_protocol) {
        QCLI_Printf(qcli_net_handle, "ALPN Negotiated Protocol: %s\n", alpn_protocol);
    }
#endif

    /* Sending.*/
    QCLI_Printf(qcli_net_handle, "Sending\n");

    if (zerocopy_send)
    {
        netbuf_id = QAPI_NETBUF_SYS;
#ifdef CONFIG_NET_SSL_DEMO
        if (p_tCxt->protocol == SSL && p_tCxt->test_type == TX) {
            netbuf_id |= QAPI_NETBUF_SSL;
        }
#endif
        send_flag = MSG_ZEROCOPYSEND;
    }
    else
    {
        netbuf_id = QAPI_NETBUF_APP;
        send_flag = 0;
    }

    /*Reset all counters*/
    cur_packet_number = 0;
    i = BENCH_UDP_PKTS_PER_DOT;
    n_send_ok = 0;

    app_get_time(&p_tCxt->pktStats.first_time);


    if (p_tCxt->is_iperf)
    {
        /* Convert bps to B/s, and then to packets/sec */
        iperf_udp_packets_per_second = 1 + ((p_tCxt->pktStats.iperf_udp_rate / 8) / p_tCxt->params.tx_params.packet_size);

        app_get_time(&iperf_udp_start_time);

        iperf_display_interval = p_tCxt->pktStats.iperf_display_interval * 1000; // convert to ms
        iperf_display_last = p_tCxt->pktStats.first_time;
    }

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
            while ((p_tCxt->buffer = qapi_Net_Buf_Alloc(packet_size, netbuf_id)) == NULL)
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

            if (netbuf_id == QAPI_NETBUF_APP)
            {
                /* Clear the buffer */
                memset(p_tCxt->buffer, 0, packet_size);
            }
            else
            {
                /* Todo: zero-copy clear */
            }

			/* Update net buffer:
			 *
			 * [START]<4-byte Packet Index><4-byte Packet Size>000102..FF000102..FF0001..[END]
			 * Byte counts: 8 + 4 + 4 + (packet_size-22) + 6
			 *
			 */
			if (packet_size >= 22)
			{
				char *pkt_start = "[START]";
				char *pkt_end = "[END]";
				uint32_t val;

                        if (!p_tCxt->is_iperf)
                        {
                            /* Add "[START]" */
                            qapi_Net_Buf_Update(p_tCxt->buffer, 0, pkt_start, 8, netbuf_id);

                            /* Packet index */
                            val = htonl(cur_packet_number);
                            qapi_Net_Buf_Update(p_tCxt->buffer, 8, &val, 4, netbuf_id);

                            /* Packet size */
                            val = htonl(packet_size);
                            qapi_Net_Buf_Update(p_tCxt->buffer, 12, &val, 4, netbuf_id);
                        }
                        else
                        {
                            val = htonl(cur_packet_number);
                            qapi_Net_Buf_Update(p_tCxt->buffer, 0, &val, 4, netbuf_id);  
                        }

                        /* Add pattern
                        * The pattern is repeated 00 01 02 03 .. FE FF
                        */
                        if (netbuf_id == QAPI_NETBUF_APP)
                        {
                            if (!p_tCxt->is_iperf)
                            {
                                bench_common_add_pattern(p_tCxt->buffer + 16, packet_size - 16 - 6);
                            }
                            else
                            {
                                bench_common_add_pattern(p_tCxt->buffer + 4, packet_size - 4);
                            }
                        }
                        else
                        {
                            if (!p_tCxt->is_iperf)
                            {
                                uint8_t data[] = {0xaa, 0xbb, 0xcc, 0xdd};
                                qapi_Net_Buf_Update(p_tCxt->buffer, 16, data, packet_size - 16 - 6, netbuf_id);
                            }
                            else
                            {
                                uint8_t data[] = {0xaa, 0xbb, 0xcc, 0xdd};
                                qapi_Net_Buf_Update(p_tCxt->buffer, 4, data, packet_size - 4, netbuf_id);                                
                            }
                        }

                        if (!p_tCxt->is_iperf)
                        {
                            /* Add "[END]" */
                            qapi_Net_Buf_Update(p_tCxt->buffer, packet_size-6, pkt_end, 6, netbuf_id);
                        }
			}
			else if (packet_size >= 4)
			{
				/* Small buffer, just send the packet index */
				uint32_t val;

				/* Packet index */
				val = htonl(cur_packet_number);
				qapi_Net_Buf_Update(p_tCxt->buffer, 0, &val, 4, netbuf_id);
			}
        }
        else
		{
			uint32_t val;
			uint32_t idx;

			if (packet_size >= 22)
			{
				idx = 8;
			}
			else
			{
				idx = 0;
			}

			/* Packet index */
			val = htonl(cur_packet_number);
			qapi_Net_Buf_Update(p_tCxt->buffer, idx, &val, 4, netbuf_id);
		}

        do
        {
            if (benchtx_quit)
            {
                app_get_time(&p_tCxt->pktStats.last_time);
                is_test_done = 1;
                break;
            }
#ifdef CONFIG_NET_SSL_DEMO
            if (p_tCxt->protocol == SSL && p_tCxt->test_type == TX)
            {
                send_bytes = qapi_Net_SSL_Write(ssl->ssl, p_tCxt->buffer, packet_size);
                if(send_bytes < 0) {
                    if(qapi_Net_SSL_Client_Closed_By_Peer(ssl->ssl)) {
                        QCLI_Printf(qcli_net_handle, "\nDTLS connection closed by peer.");
                        is_test_done = 1;
                        app_get_time(&p_tCxt->pktStats.last_time);
                        break;
                    }
                }
            }
            else
#endif
            {
                //send_bytes = qapi_sendto(p_tCxt->sock_peer, p_tCxt->buffer, packet_size, send_flag, to, tolen) ;
                send_bytes = qapi_send(p_tCxt->sock_peer, p_tCxt->buffer, packet_size, send_flag);
            }

            if ( send_bytes != packet_size )
            {
                int errno = qapi_errno(p_tCxt->sock_peer);
                if ( (EPIPE == errno) ||
                     (ENOTSOCK == errno) ||
                     (EBADF == errno) ||
                     (EIEIO == errno) ||
                     (ENETUNREACH == errno) )
                {
                    QCLI_Printf(qcli_net_handle, "\nError: send_bytes=%d, errno=%d\n", send_bytes, errno);
                    if ( zerocopy_send && !p_tCxt->params.tx_params.v6 && (send_bytes >= 0) && (send_bytes < packet_size) )
                    {
                        /* this is a special case of zero-copy partial transmission of IPv4 buffer.  In this case
                         * the buffer is actually freed inside qapi_send(), so we must not free it again.
                         */
                        p_tCxt->buffer = NULL;
                    }
                    is_test_done = 1;
                    app_get_time(&p_tCxt->pktStats.last_time);
                    break;
                }
                else
                {
                    if ( ENOBUFS != errno )
                    {
                        QCLI_Printf(qcli_net_handle, "\nFailed to qapi_send, send_bytes=%d, errno=%d\n", send_bytes, errno);
                    }

                    /* severe push back, let the other processes run (won't happen on blocking sockets) */
                    qapi_Task_Delay(1000);
                }
            }
            else
            {
                cur_packet_number ++;
            }

            if (++i >= BENCH_UDP_PKTS_PER_DOT && !p_tCxt->print_buf && !p_tCxt->is_iperf)
            {
                QCLI_Printf(qcli_net_handle, ".");
                i = 0;
            }

            /* Useful notes related to UDP zerocopy send:
             *
             * In UDP zerocopy send,  we can have the following qapi_send() return codes:
             * IPv4:
             *  return_code < 0: the buffer was not sent, qapi_send() freed the buffer
             *  return_code == packet_size: the buffer was sent successfully, qapi_send() freed the buffer
             *  return_code < packet_size: the buffer sent partially, qapi_send() freed the buffer
             * IPv6:
             *  return_code < 0: the buffer was not sent, qapi_send() freed the buffer
             *  return_code == packet_size: the buffer was sent successfully, qapi_send() freed the buffer
             */
            if (zerocopy_send)
            {
                p_tCxt->buffer = NULL;
            }

#ifdef UDP_TX_DEBUG
            QCLI_Printf(qcli_net_handle, "%d send_bytes = %d\n", cur_packet_number, send_bytes);
#endif

            if (send_bytes > 0)
            {
                p_tCxt->pktStats.bytes += send_bytes;
                ++n_send_ok;

                if (p_tCxt->is_iperf)
                {
                    /* Small buffer, just send the packet index */
                    uint32_t val;

                    /* Packet index */
                    val = htonl(cur_packet_number);
                    qapi_Net_Buf_Update(p_tCxt->buffer, 0, &val, 4, netbuf_id);
                }

                if (p_tCxt->print_buf)
                    bench_print_buffer(p_tCxt->buffer, send_bytes, to, DUMP_DIRECTION_TX);
            }

            if (p_tCxt->is_iperf)
            {
                time_struct_t iperf_curr_time;

                if(p_tCxt->pktStats.iperf_display_interval)
                {
                    app_get_time(&iperf_curr_time);

                    if(app_get_time_difference(&iperf_display_last, &iperf_curr_time) >= iperf_display_interval)
                    {
                        iperf_result_print(&p_tCxt->pktStats, iperf_display_last.seconds, iperf_curr_time.seconds);
                        iperf_display_last = iperf_curr_time;
                    }
                }

                /* iperf bandwidth */
                iperf_udp_packets_counter++;

                if(iperf_udp_packets_counter == iperf_udp_packets_per_second)
                {
                    uint32_t iperf_diff_time;

                    /* Get the current time and calculate the sleep needed till the end of the second */
                    app_get_time(&iperf_curr_time);

                    /* This is the diff in milliseconds */
                    iperf_diff_time = app_get_time_difference(&iperf_udp_start_time, &iperf_curr_time);

                    /* Check that the diff is less than a second. If it's more than a second,
                     * it means that we were asked to limit the bandwidth to a value we cannot
                     * reach, so we are behind. In this case, no sleep is required, just push as much as
                     * we can...
                     */
                    if(iperf_diff_time < 1000)
                    {
                        /* Task delay QAPI input is in uS */
                        qapi_Task_Delay((1000 - iperf_diff_time) * 1000);
                    }

                    /* Restart the timer and clear the counter */
                    app_get_time(&iperf_udp_start_time);
                    iperf_udp_packets_counter = 0;
                }
            }

            /*Test mode can be "number of packets" or "fixed time duration"*/
            if (p_tCxt->params.tx_params.test_mode == PACKET_TEST)
            {
                if ((cur_packet_number >= p_tCxt->params.tx_params.packet_number))
                {
                    app_get_time(&p_tCxt->pktStats.last_time);
                    is_test_done = 1;
                    break;
                }
            }
            else if (p_tCxt->params.tx_params.test_mode == TIME_TEST)
            {
                app_get_time(&p_tCxt->pktStats.last_time);
                if (bench_common_check_test_time(p_tCxt))
                {
                    is_test_done = 1;
                    break;
                }
            }

            /****non-iperf bandwidth control***********/
            if (p_tCxt->params.tx_params.interval_us)
                qapi_Task_Delay(p_tCxt->params.tx_params.interval_us);

        } while ( !((is_test_done) || (send_bytes == packet_size) || (NULL == p_tCxt->buffer)) );   /* send loop */

    } /* while ( !is_test_done ) */

    if ( p_tCxt->buffer )
    {
        qapi_Net_Buf_Free(p_tCxt->buffer, netbuf_id);
    }

    if (!p_tCxt->is_iperf)
    {
        QCLI_Printf(qcli_net_handle, "\nSent %u packets, %llu bytes to %s %d (%u)\n",
            cur_packet_number, p_tCxt->pktStats.bytes, ip_str, p_tCxt->params.tx_params.port, cur_packet_number - n_send_ok);
    }

    /* Send endmark packet and wait for stats from server */
    result = bench_common_wait_for_response(p_tCxt, to, tolen, cur_packet_number);

    if (!p_tCxt->is_iperf)
    {
        if (result != QAPI_OK)
        {
            QCLI_Printf(qcli_net_handle, "%s Transmit test failed, did not receive Ack from Peer\n", (p_tCxt->protocol == SSL && p_tCxt->test_type == TX) ? "SSL" : "UDP");
        }
    }

ERROR_2:
    if (p_tCxt->is_iperf)
    {
        iperf_result_print(&p_tCxt->pktStats, 0,0);
    }
    else
    {
        bench_common_print_test_results(p_tCxt, &p_tCxt->pktStats);
    }

#ifdef CONFIG_NET_SSL_DEMO
    if (ssl->ssl != QAPI_NET_SSL_INVALID_HANDLE) {
    	bench_ssl_Print_Last_Alert(ssl->ssl);
    	qapi_Net_SSL_Shutdown(ssl->ssl);
    	ssl->ssl = QAPI_NET_SSL_INVALID_HANDLE;
    }
#endif

    qapi_socketclose(p_tCxt->sock_peer);

ERROR_1:
    if (!p_tCxt->is_iperf)
    {
        QCLI_Printf(qcli_net_handle, BENCH_TEST_COMPLETED);
    }

    return;
}
#endif
