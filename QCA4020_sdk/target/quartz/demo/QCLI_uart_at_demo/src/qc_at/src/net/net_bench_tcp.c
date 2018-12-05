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
#include "qapi_netbuf.h"
#include "net_bench.h"
#include "net_iperf.h"
#include "qapi_netservices.h"
#include "qapi_socket.h"
#include "qapi_ns_gen_v4.h"
#include "qapi_ns_gen_v6.h"
#include "qurt_mutex.h"
#include "qapi_delay.h"

extern uint8_t benchtx_quit;
extern uint8_t benchrx_quit;
static QUEUE_T tcp_zcq;

#define BENCH_TCP_PKTS_PER_DOT	1000 /* Produce a progress dot each X packets */

#define BENCH_TCP_RX_MAX_SESSIONS 12 /* Max number of TCP RX sessions */
#define BENCH_TCP_MAX_SERVERS 5 /* Max number of TCP servers that can execute in parallel */

bench_tcp_session_t g_tcpSessions[BENCH_TCP_RX_MAX_SESSIONS]; /* Array of TCP Session objects */

qurt_mutex_t sessionLock; /* Lock to protect the global session object array */
qurt_mutex_t serverLock; /* Lock to protect the global TCP Server object array */
int sessionRefCount=0; /* Total number of active TCP/SSL RX sessions */
int tcpRefCount=0; /* Number of active TCP RX sessions */
int sslRefCount=0; /* Number of active SSL RX sessions */


bench_tcp_server_t g_tcpServers[BENCH_TCP_MAX_SERVERS]; /* Array of TCP Server objects */
int serverRefCount=0; /* Total number of active TCP/SSL Servers */

uint32_t bench_tcp_IsPortInUse(uint16_t port)
{
	uint32_t ret = 0;
	int i=0;
	bench_tcp_session_t *session;

	for (i=0; i<BENCH_TCP_RX_MAX_SESSIONS; i++) {
		session = &g_tcpSessions[i];
		if (session->busySlot && (session->port == port))
			ret = 1;
	}

	return ret;
}

/*****************************************************************************
 *****************************************************************************/
static int tcp_rx_upcall(void *so, void *pkt, int code)
{
    /*if (p_rxtCxt == NULL)
    {
        return -1;
    }

    p_rxtCxt->rxcode = code;*/

    if (pkt)
    {
        PACKET pp = (PACKET)pkt;
        QCLI_Printf("%s: pkt=0x%p so=%p blen=%lu tlen=%lu plen=%lu buff=%p prot=%p pk_next=%p\n", __func__,
                pkt, so, pp->nb_Blen, pp->nb_Tlen, pp->nb_Plen, pp->nb_Buff, pp->nb_Prot, pp->pk_Next);
        enqueue(&tcp_zcq, pkt);
    }

    return 0;
}

/**************************************************************
 * FUNCTION: bench_tcp_CreateSession(THROUGHPUT_CXT *)
 * @brief: Create TCP Session object.
 * @return: sessionId if success, -1 on failure.
 *************************************************************/
static int bench_tcp_CreateSession(THROUGHPUT_CXT *p_tCxt)
{
	bench_tcp_session_t *session;
	int sessionId=-1;
	int index=0;

	if (!p_tCxt) {
		QCLI_Printf("%s:Context is NULL", __func__);
		return -1;
	}

	qurt_mutex_lock(&sessionLock);
	for (index=0; index<BENCH_TCP_RX_MAX_SESSIONS; index++) {
		if (!g_tcpSessions[index].busySlot) {
			sessionId = index;
			break;
		}
	}

	if (index >= BENCH_TCP_RX_MAX_SESSIONS) {
		qurt_mutex_unlock(&sessionLock);
		return -2;
	}

	g_tcpSessions[sessionId].busySlot = 1;

	session = &g_tcpSessions[sessionId];

	session->ctxt = p_tCxt;

	if (p_tCxt->protocol == SSL) {
		sslRefCount++;
	}
	else
	{
		tcpRefCount++;
	}

	sessionRefCount++;
	qurt_mutex_unlock(&sessionLock);

	return sessionId;
}

/*****************************************************************************
 * FUNCTION: bench_tcp_CloseSession(bench_tcp_session_t *)
 * @brief: Close an existing TCP RX session and destroy the TCP session object.
 * @return: void
 *****************************************************************************/
static void bench_tcp_CloseSession(bench_tcp_session_t *session, fd_set *rd_set)
{
	qurt_mutex_lock(&sessionLock);
	session->ready = 0;
	session->busySlot = 0;

	if (session->ctxt->is_iperf) {
		iperf_result_print(&session->pktStats, 0, 0);
	}
	else {
		app_get_time(&session->pktStats.last_time);
    	bench_common_print_test_results(session->ctxt, &session->pktStats);
    }

    QCLI_Printf(BENCH_TEST_COMPLETED);
    qurt_mutex_unlock(&sessionLock);

	if (session->sslInst.connHandle != QAPI_NET_SSL_INVALID_HANDLE)
	{
		qapi_Net_SSL_Shutdown(session->sslInst.connHandle);
	}

	if (session->buffer ) {
			qapi_Net_Buf_Free(session->buffer, session->netbuf_id);
	}

	qurt_mutex_lock(&sessionLock);
	qapi_fd_clr(session->sock_peer, rd_set);
	qapi_socketclose(session->sock_peer);
	
	if (session->ctxt->protocol == SSL) {
		sslRefCount--;
	}
	else
	{
		tcpRefCount--;
	}
	sessionRefCount--;
	memset(session, 0, sizeof(bench_tcp_session_t));

	if (!sessionRefCount) {
		qapi_fd_zero(rd_set);
	}

	qurt_mutex_unlock(&sessionLock);
}

/********************************************************************************
 * FUNCTION: bench_tcp_CloseAllSessions()
 * @brief: Close all existing TCP RX sessions with the given ctxt
 *         and destroy the corresponding TCP session objects.
 * @return: void
 ********************************************************************************/
static void bench_tcp_CloseAllSessions(THROUGHPUT_CXT *p_tCxt, fd_set *rd_set)
{
	int i=0;
	bench_tcp_session_t *session;
	
	if(sessionRefCount >0)
	{
		for (i=0; i < BENCH_TCP_RX_MAX_SESSIONS; i++) {
			session = &g_tcpSessions[i];

			if (session->busySlot && (session->ctxt == p_tCxt)){
				QCLI_Printf("Closing SessionId:%d\n", i);
				bench_tcp_CloseSession(session, rd_set);
			}
		}
	}
	
	if (!sessionRefCount && !serverRefCount) {
		qurt_mutex_destroy(&sessionLock);
	}
}

/*******************************************************************
 * FUNCTION: bench_tcp_rx_quit(int)
 * @brief: Close an active TCP RX session.
 * @return: void
 ******************************************************************/
void bench_tcp_rx_quit(int serverId)
{
	bench_tcp_server_t *server=NULL;

	if (serverId < 0 || serverId >= BENCH_TCP_MAX_SERVERS) {
		QCLI_Printf("Invalid session id %d\n", serverId);
		return;
	}

	qurt_mutex_lock(&serverLock);
	server = &g_tcpServers[serverId];
	if (server->busySlot) {
		server->exit = 1;
	}
	qurt_mutex_unlock(&serverLock);
}

static int bench_tcp_getServerId()
{
	int serverId = -1;
	int i=0;

	qurt_mutex_lock(&serverLock);
	for (i=0; i<BENCH_TCP_MAX_SERVERS;i++) {
		if (!g_tcpServers[i].busySlot) {
			serverId = i;
			break;
		}
	}

	if (serverId >= BENCH_TCP_MAX_SERVERS) {
		qurt_mutex_unlock(&serverLock);
		return -1;
	}

	g_tcpServers[serverId].busySlot = 1;
	serverRefCount++;
	qurt_mutex_unlock(&serverLock);

	return serverId;
}

static void bench_tcp_stopServer(bench_tcp_server_t *server)
{
	qurt_mutex_lock(&serverLock);
	if (server->sockfd) {
		qapi_socketclose(server->sockfd);
	}
	memset(server, 0, sizeof(bench_tcp_server_t));
	serverRefCount--;
	qurt_mutex_unlock(&serverLock);

	if (!serverRefCount) {
		qurt_mutex_destroy(&serverLock);
	}
}

void bench_tcp_rx_dump_servers()
{
	int i=0;

	if (!serverRefCount) {
		QCLI_Printf("No TCP servers found\n");
		return;
	}

	qurt_mutex_lock(&serverLock);
	for(i=0; i<BENCH_TCP_MAX_SERVERS; i++) {
		if (g_tcpServers[i].busySlot) {
			QCLI_Printf("****** TCP SERVER ******\n");
			QCLI_Printf("ServerId: %d\n", i);
			QCLI_Printf("Port: %d\n", g_tcpServers[i].port);
			QCLI_Printf("***********************\n");
		}
	}
	qurt_mutex_unlock(&serverLock);

}
/************************************************************************
* NAME: bench_tcp_rx
*
* DESCRIPTION: Start throughput TCP server.
************************************************************************/
void bench_tcp_rx(THROUGHPUT_CXT *p_tCxt)
{
    int32_t received;
    int32_t conn_sock, printit = 1;
    struct sockaddr_in local_addr;
    struct sockaddr_in6 local_addr6;
    struct sockaddr *addr;
    uint32_t addrlen;
    struct sockaddr_in foreign_addr;
    struct sockaddr_in6 foreign_addr6;
    struct sockaddr *from;
    int32_t fromlen;
    void *sin_addr, *local_sin_addr;
    char ip_str[48];

    SSL_INST *ssl = bench_ssl_GetInstance(SSL_SERVER_INST);
    int32_t i = 0;
    int32_t netbuf_id = QAPI_NETBUF_APP;
    uint32_t retry=20;

    int sessionId=0;
    int family;
    bench_tcp_session_t *session=NULL;
    bench_tcp_session_t *sess=NULL;
    int sock_peer;
    int newSession=0,index=0;
    bench_tcp_server_t *tcpServer=NULL;
    int serverId=-1;
    fd_set rd_set;

    if (serverRefCount == BENCH_TCP_MAX_SERVERS) {
    	QCLI_Printf("%s: Max num of servers supported is %d\n", __func__, BENCH_TCP_MAX_SERVERS);
    	return;
    }

	if (!sessionRefCount) {
		qurt_mutex_init(&sessionLock);
		memset(g_tcpSessions, 0, sizeof(g_tcpSessions));
	}

	if (!serverRefCount) {
		qurt_mutex_init(&serverLock);
		memset(g_tcpServers, 0, sizeof(g_tcpServers));
	}

	serverId = bench_tcp_getServerId();
	if (serverId == -1) {
		QCLI_Printf("%s: Invalid ServerId %d\n", __func__, serverId);
		return;
	}

	tcpServer = &g_tcpServers[serverId];

	QCLI_Printf("TCP Server Id: %d\n", serverId);

	tcpServer->port = p_tCxt->params.rx_params.port;

	qapi_fd_zero(&rd_set);

    if (p_tCxt->params.rx_params.v6)
    {
        family = AF_INET6;

        memset(&local_addr6, 0, sizeof(local_addr6));
        local_addr6.sin_port = htons(tcpServer->port);
        local_addr6.sin_family = family;
        memcpy(&local_addr6.sin_addr, p_tCxt->params.rx_params.local_v6addr, sizeof(local_addr6.sin_addr));
        addr = (struct sockaddr *)&local_addr6;
        local_sin_addr = &local_addr6.sin_addr;
        addrlen = sizeof(struct sockaddr_in6);

        from = (struct sockaddr *)&foreign_addr6;
        fromlen = sizeof(struct sockaddr_in6);
        sin_addr = &foreign_addr6.sin_addr;
    }
    else
    {
        family = AF_INET;

        memset(&local_addr, 0, sizeof(local_addr));
        local_addr.sin_port = htons(tcpServer->port);
        local_addr.sin_family = family;
        local_addr.sin_addr.s_addr = p_tCxt->params.rx_params.local_address;
        addr = (struct sockaddr *)&local_addr;
        local_sin_addr = &local_addr.sin_addr;
        addrlen = sizeof(struct sockaddr_in);

        from = (struct sockaddr *)&foreign_addr;
        fromlen = sizeof(struct sockaddr_in);
        sin_addr = &foreign_addr.sin_addr;
    }
    
    /* Create listen socket */
	if ((tcpServer->sockfd = qapi_socket(family, SOCK_STREAM, 0)) == A_ERROR)
	{
		QCLI_Printf("ERROR: Socket creation error.\n");
		goto ERROR_1;
	}

	/* Bind socket */
	if (qapi_bind(tcpServer->sockfd, addr, addrlen) == A_ERROR)
	{
		QCLI_Printf("ERROR: Socket bind error.\n");
		goto tcp_rx_QUIT;
	}

	/* set to non-blocking mode */
	qapi_setsockopt(tcpServer->sockfd, SOL_SOCKET, SO_NBIO, NULL, 0);

    /* Configure queue sizes */
    bench_config_queue_size(tcpServer->sockfd);

	/* Listen */
	if (qapi_listen(tcpServer->sockfd, 0) == A_ERROR)
	{
		QCLI_Printf("ERROR: Socket listen error.\n");
		goto tcp_rx_QUIT;
	}

	if (printit)
	{
		memset(ip_str, 0, sizeof(ip_str));

		if (p_tCxt->is_iperf)
		{
			QCLI_Printf("------------------------------------------------------------\n");
			QCLI_Printf("Server listening on TCP port %d\n",   tcpServer->port);
			QCLI_Printf("------------------------------------------------------------\n");
		}
		else
		{
			/* ------ Start test.----------- */
			QCLI_Printf("*****************************************\n");
			QCLI_Printf("IOT %s RX Test\n", (p_tCxt->protocol == SSL && p_tCxt->test_type == RX) ? "SSL" : "TCP");
			QCLI_Printf("*****************************************\n");
			QCLI_Printf("Bind address %s\n", inet_ntop(family, local_sin_addr, ip_str, sizeof(ip_str)));
			QCLI_Printf("Local port %d\n", tcpServer->port);
			QCLI_Printf("Type benchquit to terminate test\n");
			QCLI_Printf("*****************************************\n");
		}

		printit = 0;
	}

	QCLI_Printf("Waiting\n");

	do
	{
		fd_set rset;
		qapi_fd_zero(&rset);

		if (benchrx_quit || tcpServer->exit) {
			goto tcp_rx_QUIT;
		}

		/* Accept incoming connection */
		if ((sock_peer = qapi_accept(tcpServer->sockfd, from, &fromlen)) == A_ERROR)
		{
			if (!sessionRefCount) {
				qapi_Task_Delay(1000);
				continue;
			}
			else if (p_tCxt->protocol == SSL && !sslRefCount) {
				qapi_Task_Delay(1000);
				continue;
			}
			else if (p_tCxt->protocol == TCP && !tcpRefCount) {
				qapi_Task_Delay(1000);
				continue;
			}
		}
		else {
			newSession = 1;
		}

		/* Create a new TCP session object */
		if (newSession) {
            int opt = 1;

			/* set to non-blocking mode */
			qapi_setsockopt(sock_peer, SOL_SOCKET, SO_NBIO, NULL, 0);

            /* enable TCP keepalive */
            qapi_setsockopt(sock_peer, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt));

            /* Configure queue sizes */
            bench_config_queue_size(sock_peer);

			if ((sessionId = bench_tcp_CreateSession(p_tCxt)) < 0) {
				QCLI_Printf("Failed to create TCP RX session object\n");
				qapi_socketclose(sock_peer);
				newSession=0;
				continue;
			}

			session = &g_tcpSessions[sessionId];
			session->ctxt = p_tCxt;
			session->port = tcpServer->port;

			/*Allocate buffer*/
			if ((session->buffer = qapi_Net_Buf_Alloc(CFG_PACKET_SIZE_MAX_RX, netbuf_id)) == NULL)
			{
				QCLI_Printf("Out of memory error\n");
				bench_tcp_CloseSession(session, &rd_set);
				newSession=0;
				continue;
			}

			session->sock_peer = sock_peer;

			if (p_tCxt->is_iperf) {
				p_tCxt->iperf_stream_id++;
				session->pktStats.iperf_stream_id = p_tCxt->iperf_stream_id;
				session->pktStats.iperf_time_sec = 0;
				session->pktStats.iperf_display_interval = p_tCxt->pktStats.iperf_display_interval;
			}

			/* Kick start SSL handshake if protocol is SSL */
			if (p_tCxt->protocol == SSL)
			{
				session->sslInst.ssl_state = bench_ssl_rx_setup(ssl, sock_peer, &session->sslInst.connHandle, 0);
				if (session->sslInst.ssl_state == QAPI_ERROR) {
					bench_tcp_CloseSession(session, &rd_set);
				}
				else {
					app_get_time(&session->sslInst.hs_start_time);

					/* Handshake completed successfully */
					if (session->sslInst.ssl_state == QAPI_SSL_OK_HS) {
						session->isFirst = 1;
						qapi_fd_set(session->sock_peer, &rd_set);

						session->ready = 1;
						QCLI_Printf("Accepted conn from %s:%d\n",
								inet_ntop(family, sin_addr, ip_str, sizeof(ip_str)),
								ntohs(from->sa_port));

						QCLI_Printf("Session %d ready\n", sessionId);
					}
				}
			}
			else
			{
				session->isFirst = 1;
				qapi_fd_set(session->sock_peer, &rd_set);

				session->ready = 1;
				QCLI_Printf("Accepted conn from %s:%d\n",
						inet_ntop(family, sin_addr, ip_str, sizeof(ip_str)),
						ntohs(from->sa_port));

				QCLI_Printf("Session %d ready\n", sessionId);
			}
			newSession = 0;
		}
		/* Check the status of SSL handshake if the protocol is SSL */
		else if (sessionRefCount && (p_tCxt->protocol == SSL)){
			bench_tcp_session_t *sslSession=NULL;
			int numSessionsToBeHandled = sessionRefCount;
			for (i=0; i<BENCH_TCP_RX_MAX_SESSIONS && numSessionsToBeHandled; i++){
				sslSession = &g_tcpSessions[i];
				if (sslSession->busySlot) {
					if (!sslSession->ready) {
						sslSession->sslInst.ssl_state = bench_ssl_Con_Get_Status(&sslSession->sslInst);
						if( sslSession->sslInst.ssl_state != QAPI_SSL_HS_IN_PROGRESS )
						{
							bench_ssl_Print_SSL_Handshake_Status(sslSession->sslInst.ssl_state);

							if (sslSession->sslInst.ssl_state == QAPI_SSL_OK_HS){
									sslSession->isFirst = 1;
									qapi_fd_set(sslSession->sock_peer, &rd_set);

									sslSession->ready = 1;
									QCLI_Printf("Accepted conn from %s:%d\n",
											inet_ntop(family, sin_addr, ip_str, sizeof(ip_str)),
											ntohs(from->sa_port));

									QCLI_Printf("Session %d ready\n", i);
									const char* alpn_protocol = qapi_Net_SSL_ALPN_Protocol_Get(sslSession->sslInst.connHandle);
									if (alpn_protocol) {
										QCLI_Printf("ALPN Negotiated Protocol: %s\n", alpn_protocol);
									}
							}
							else {
								/* Handle error status */
								bench_tcp_CloseSession(sslSession, &rd_set);
							}
						}
					}
					numSessionsToBeHandled--;
				}
			}
		}

		/* Socket select to check if data is available */
		rset = rd_set;
		conn_sock = qapi_select(&rset, NULL, NULL, 10);

		if (conn_sock == 0) {
			/* No activity. Continue with the next session */
			continue;
		}
		else {
			index = 0;
			while (conn_sock > 0) {
				if (index == BENCH_TCP_RX_MAX_SESSIONS)
					break;
				sess = &g_tcpSessions[index++];
				if ((sess->ctxt == p_tCxt) && sess->ready && qapi_fd_isset(sess->sock_peer, &rset)) {
				    /*Packet is available, receive it*/

					if (sess->ctxt->protocol == SSL && sess->ctxt->test_type == RX)
					{
						received = qapi_Net_SSL_Read(sess->sslInst.connHandle, sess->buffer, CFG_PACKET_SIZE_MAX_RX);
					}
					else
					{
						received = qapi_recv( sess->sock_peer, (char*)(&sess->buffer[0]), CFG_PACKET_SIZE_MAX_RX, 0);
					}

					/*Valid packet received*/
					if (received > 0)
					{
						sess->pktStats.bytes += received;


						if (sess->isFirst)
						{
							QCLI_Printf("Receiving for session %d\n", index-1);

							/*This is the first packet, set initial time used to calculate throughput*/
							app_get_time(&sess->pktStats.first_time);
							sess->isFirst = 0;

							if (p_tCxt->is_iperf)
							{
								sess->iperf_display_last = sess->pktStats.first_time.seconds ;
								sess->iperf_display_next = sess->iperf_display_last + sess->pktStats.iperf_display_interval;
							}
						}

						if (p_tCxt->print_buf)
							bench_print_buffer(sess->buffer, received, from, DUMP_DIRECTION_RX);

						if (p_tCxt->is_iperf) {
							time_struct_t iperf_curr_time;
							app_get_time(&iperf_curr_time);
							uint32_t cur_time = iperf_curr_time.seconds;

							if(cur_time >= sess->iperf_display_next && sess->pktStats.iperf_display_interval)
							{
								iperf_result_print(&session->pktStats, sess->iperf_display_last, cur_time);
								sess->iperf_display_last = cur_time;
								sess->iperf_display_next = cur_time + sess->pktStats.iperf_display_interval;
							}
						}

	                    if (p_tCxt->echo) {

	                        /* Echo the buffer back to the sender (best effort, no retransmission). */
                            qapi_send(sess->sock_peer, sess->buffer, received, MSG_DONTWAIT) ;
	                    }

						QCLI_Printf("%d received= %d total=%llu\n", i, received, sess->pktStats.bytes);
					}
					else /* received <= 0 */
					{
						QCLI_Printf("%d received= %d total=%llu\n", i, received, sess->pktStats.bytes);

						if (retry > 0)
						{
							qapi_Task_Delay(1000);
							--retry;
							continue;
						}
						else
						{
							QCLI_Printf("\nReceived %llu bytes for session %d\n",
										sess->pktStats.bytes,
										index-1);

							bench_tcp_CloseSession(sess, &rd_set);
						}
					}
					conn_sock--;
				}
			}
		}
	} while (1);

tcp_rx_QUIT:
	QCLI_Printf("Exiting TCP Server Id:%d\n", serverId);
	if (tcpServer) {
		bench_tcp_stopServer(tcpServer);
	}
ERROR_1:
	bench_tcp_CloseAllSessions(p_tCxt, &rd_set);

	return;
}

/*****************************************************************************
 *****************************************************************************/
void bench_tcp_rx_zc(THROUGHPUT_CXT *p_tCxt)
{
    uint16_t port;
    int32_t conn_sock, isFirst, i, printit = 1;
    struct sockaddr_in     local_addr;
    struct sockaddr_in6    local_addr6;
    struct sockaddr        *addr;
    int addrlen;
    struct sockaddr_in     foreign_addr;
    struct sockaddr_in6    foreign_addr6;
    struct sockaddr        *from;
    int32_t fromlen;
    void *sin_addr, *local_sin_addr;
    char ip_str[48];
    PACKET pkt;
    int family;
    int retry=20;
    SSL_INST *ssl = bench_ssl_GetInstance(SSL_SERVER_INST);

    port = p_tCxt->params.rx_params.port;

    if (p_tCxt->params.rx_params.v6)
    {
        family = AF_INET6;
        from = (struct sockaddr *)&foreign_addr6;
        fromlen = sizeof(struct sockaddr_in6);
        sin_addr = &foreign_addr6.sin_addr;

        memset(&local_addr6, 0, sizeof(local_addr6));
        local_addr6.sin_port   = htons(port);
        local_addr6.sin_family = family;
        memcpy(&local_addr6.sin_addr, p_tCxt->params.rx_params.local_v6addr, 16);
        addr = (struct sockaddr *)&local_addr6;
        local_sin_addr = &local_addr6.sin_addr;
        addrlen = sizeof(struct sockaddr_in6);
    }
    else /* IPv4 */
    {
        family = AF_INET;
        from = (struct sockaddr *)&foreign_addr;
        fromlen = sizeof(struct sockaddr_in);
        sin_addr = &foreign_addr.sin_addr;

        memset(&local_addr, 0, sizeof(local_addr));
        local_addr.sin_port     = htons(port);
        local_addr.sin_family   = family;
        local_addr.sin_addr.s_addr = p_tCxt->params.rx_params.local_address;
        addr = (struct sockaddr *)&local_addr;
        local_sin_addr = &local_addr.sin_addr;
        addrlen = sizeof(struct sockaddr_in);
    }

    while (1)
    {
        int opt = 1;

        /* Open socket */
        if ((p_tCxt->sock_local = qapi_socket( family, SOCK_STREAM, 0)) == A_ERROR)
        {
            QCLI_Printf("ERROR: Socket creation error.\n");
            goto ERROR_1;
        }

        /* Bind */
        if (qapi_bind( p_tCxt->sock_local, addr, addrlen) == A_ERROR)
        {
            QCLI_Printf("ERROR: Socket bind error.\n");
            goto ERROR_2;
        }

        /* set to non-blocking mode */
        qapi_setsockopt(p_tCxt->sock_local, SOL_SOCKET, SO_NBIO, NULL, 0);

        if (printit)
        {
            memset(ip_str, 0, sizeof(ip_str));

            /* ------ Start test.----------- */
            QCLI_Printf("*****************************************\n");
            QCLI_Printf("IOT TCP Zero-copy RX Test\n" );
            QCLI_Printf("*****************************************\n");
            QCLI_Printf("Bind address %s\n", inet_ntop(family, local_sin_addr, ip_str, sizeof(ip_str)));
            QCLI_Printf("Local port %d\n", port);
            QCLI_Printf("Type benchquit to terminate test\n");
            QCLI_Printf("*****************************************\n");

            printit = 0;
        }

        /* Listen. */
        if (qapi_listen( p_tCxt->sock_local, 0) == A_ERROR)
        {
            QCLI_Printf("ERROR: Socket listen error.\n");
            goto ERROR_2;
        }

        QCLI_Printf("\nWaiting\n");

        do
        {
            if (benchrx_quit)
            {
            	benchrx_quit=0;
                goto tcp_rx_QUIT;
            }

            /* Accept incoming connection */
            if ((p_tCxt->sock_peer = qapi_accept(p_tCxt->sock_local, from, &fromlen)) != A_ERROR)
            {
                break;
            }

            qapi_Task_Delay(1000);
        } while (1);

        /* We allow only one connected socket,
         * so close the listening socket to
         * prevent another client connection.
         */
        qapi_socketclose(p_tCxt->sock_local);
        p_tCxt->sock_local = 0;

        p_tCxt->pktStats.bytes = 0;
        memset(&p_tCxt->pktStats.first_time, 0, sizeof(time_struct_t));
        memset(&p_tCxt->pktStats.last_time, 0, sizeof(time_struct_t));
        memset(ip_str, 0, sizeof(ip_str));

        if (p_tCxt->protocol == SSL && p_tCxt->test_type == RX)
        {
            if (bench_ssl_rx_setup(ssl, p_tCxt->sock_peer, &ssl->ssl, 1) < 0) {
                qapi_socketclose(p_tCxt->sock_peer);
                goto tcp_rx_QUIT;
            }

        }

        /* Set rx callback */
        qapi_setsockopt(p_tCxt->sock_peer, 0, SO_CALLBACK, (void *)tcp_rx_upcall, 0);

        /* enable TCP keepalive */
        qapi_setsockopt(p_tCxt->sock_peer, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt));

        QCLI_Printf("Accepted conn from %s:%d\n",
                inet_ntop(family, sin_addr, ip_str, sizeof(ip_str)),
                ntohs(from->sa_port));

        i = 0;
        isFirst = 1;

        while (1) /* Receiving data */
        {
            do
            {
                fd_set rset;

                if (benchrx_quit)
                {
                    if (ssl->ssl != QAPI_NET_SSL_INVALID_HANDLE)
                    {
                        qapi_Net_SSL_Shutdown(ssl->ssl);
                        ssl->ssl = QAPI_NET_SSL_INVALID_HANDLE;
                    }
                    app_get_time(&p_tCxt->pktStats.last_time);
                    qapi_socketclose(p_tCxt->sock_peer);
                    benchrx_quit=0;
                    goto tcp_rx_QUIT;
                }

                qapi_fd_zero(&rset);
                qapi_fd_set(p_tCxt->sock_peer, &rset);
                conn_sock = qapi_select(&rset, NULL, NULL, 500);
            } while (conn_sock == 0);

            QCLI_Printf("conn_sock=%d\n", conn_sock);

            /*Packet is available, receive it*/
            while ((pkt = (PACKET)dequeue(&tcp_zcq)) != NULL)
            {
                ++i;

                p_tCxt->pktStats.bytes += pkt->nb_Tlen;

                if (isFirst)
                {
                    QCLI_Printf("Receiving\n");

                    /*This is the first packet, set initial time used to calculate throughput*/
                    app_get_time(&p_tCxt->pktStats.first_time);
                    isFirst = 0;
                }

                QCLI_Printf("%d tlen=%u total=%llu plen=%u pkt=0x%p pk_next=0x%p\n",
                        i, pkt->nb_Tlen, p_tCxt->pktStats.bytes, pkt->nb_Plen, pkt, pkt->pk_Next);

                /* For zerocopy, App has to free the received pkt */
                qapi_Net_Buf_Free(pkt, QAPI_NETBUF_SYS);
            } /* dequeue loop */

            if (p_tCxt->rxcode == ECONNRESET || p_tCxt->rxcode == ESHUTDOWN)
            {
                if (retry > 0)
                {
                    qapi_Task_Delay(1000);
                    --retry;
                    continue;
                }
                else
                {
                    /* Receive is done */
                    app_get_time(&p_tCxt->pktStats.last_time);

                    QCLI_Printf("\nReceived %llu bytes from %s:%d\n",
                            p_tCxt->pktStats.bytes, ip_str, ntohs(from->sa_port));

                    if (ssl->ssl != QAPI_NET_SSL_INVALID_HANDLE)
                    {
                        qapi_Net_SSL_Shutdown(ssl->ssl);
                        ssl->ssl = QAPI_NET_SSL_INVALID_HANDLE;
                    }
                    qapi_socketclose(p_tCxt->sock_peer);

                    bench_common_print_test_results(p_tCxt, &p_tCxt->pktStats);

                    break;
                }
            }
        } /*receive loop*/
    } /* main loop */

tcp_rx_QUIT:
    /* check if there are pkts on the queue and free them ! */
    if (tcp_zcq.q_len)
    {
        QCLI_Printf("There are still %u pkts on queue.\n", tcp_zcq.q_len);
        while ((pkt = (PACKET)dequeue(&tcp_zcq)) != NULL)
        {
            qapi_Net_Buf_Free(pkt, QAPI_NETBUF_SYS);
        }
    }
    bench_common_print_test_results(p_tCxt, &p_tCxt->pktStats);

ERROR_2:
    if (p_tCxt->sock_local > 0)
        qapi_socketclose(p_tCxt->sock_local);

ERROR_1:
    QCLI_Printf(BENCH_TEST_COMPLETED);

    return;
}


#define INCREMENTAL_PATTERN_SIZE 256
char g_incremental_pattern[INCREMENTAL_PATTERN_SIZE];

/************************************************************************
* NAME: bench_tcp_tx
*
* DESCRIPTION: Start TCP Transmit test.
************************************************************************/
void bench_tcp_tx(THROUGHPUT_CXT *p_tCxt)
{
    struct sockaddr_in foreign_addr;
    struct sockaddr_in6 foreign_addr6;
    struct sockaddr *to;
    uint32_t tolen;
    char ip_str[48];
    uint32_t packet_size = p_tCxt->params.tx_params.packet_size;
    uint32_t cur_packet_number;
    uint32_t buffer_offset;
    int32_t result;
    SSL_INST *ssl = bench_ssl_GetInstance(SSL_CLIENT_INST);
    uint32_t i = BENCH_TCP_PKTS_PER_DOT, j = 0;
    uint32_t netbuf_id;
    int send_flag;
    int family;
    int tos_opt;
    uint32_t zerocopy_send;

    uint32_t iperf_display_interval=0;
    uint32_t iperf_display_last=0;
    uint32_t iperf_display_next=0;
    int opt = 1;

    memset(ip_str, 0, sizeof(ip_str));

    for ( i = 0; i < INCREMENTAL_PATTERN_SIZE; i++ ) {
        g_incremental_pattern[i] = i;
    }

    if (p_tCxt->is_iperf)
    {
        p_tCxt->pktStats.iperf_time_sec = 0;
        p_tCxt->iperf_stream_id += 1;
    }

    if (p_tCxt->params.tx_params.v6)
    {
        family = AF_INET6;
        inet_ntop(family, &p_tCxt->params.tx_params.v6addr[0], ip_str, sizeof(ip_str));

        memset(&foreign_addr6, 0, sizeof(foreign_addr6));
        memcpy(&foreign_addr6.sin_addr, p_tCxt->params.tx_params.v6addr, sizeof(foreign_addr6.sin_addr));;
        foreign_addr6.sin_port     = htons(p_tCxt->params.tx_params.port);
        foreign_addr6.sin_family   = family;
        foreign_addr6.sin_scope_id = p_tCxt->params.tx_params.scope_id;

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

        to = (struct sockaddr *)&foreign_addr;
        tolen = sizeof(foreign_addr);
        tos_opt = IP_TOS;
    }

    zerocopy_send = p_tCxt->params.tx_params.zerocopy_send;

    if (!p_tCxt->is_iperf)
    {
        /* ------ Start test.----------- */
        QCLI_Printf("\n**********************************************************\n");
        QCLI_Printf("IOT %s TX Test\n", (p_tCxt->protocol == SSL && p_tCxt->test_type == TX) ? "SSL" : "TCP");
        QCLI_Printf("**********************************************************\n");
        QCLI_Printf("Remote IP addr: %s\n", ip_str);
        QCLI_Printf("Remote port: %d\n", p_tCxt->params.tx_params.port);
        QCLI_Printf("Message size: %d\n", p_tCxt->params.tx_params.packet_size);
        QCLI_Printf("Number of messages: %d\n", p_tCxt->params.tx_params.packet_number);
        QCLI_Printf("Delay in microseconds: %u\n", p_tCxt->params.tx_params.interval_us);
        QCLI_Printf("Zerocopy send: %s\n", zerocopy_send != 0 ? "Yes" : "No");
        QCLI_Printf("Type benchquit to cancel\n");
        QCLI_Printf("**********************************************************\n");
    }

    /*Reset all counters*/
    cur_packet_number = 0;
    buffer_offset = 0;

    /* Create socket */
    if ((p_tCxt->sock_peer = qapi_socket(family, SOCK_STREAM, 0)) == A_ERROR)
    {
        QCLI_Printf("ERROR: Unable to create socket\n");
        goto ERROR_1;
    }

    if (p_tCxt->params.tx_params.ip_tos > 0)
    {
	    qapi_setsockopt(p_tCxt->sock_peer, IP_OPTIONS, tos_opt, &p_tCxt->params.tx_params.ip_tos, sizeof(uint8_t));
    }

    if (p_tCxt->is_iperf) 
    {
        int32_t window_size = 0;      
        qapi_getsockopt(p_tCxt->sock_peer,SOL_SOCKET, SO_SNDBUF, &window_size, (int32_t *)sizeof(window_size));

        QCLI_Printf("------------------------------------------------------------\n");
        QCLI_Printf("Client connecting to %s, TCP port %d\n", ip_str, p_tCxt->params.tx_params.port);
        QCLI_Printf("TCP window size: %d bytes\n",window_size);
        QCLI_Printf("------------------------------------------------------------\n");
    }

    /* enable TCP keepalive */
    qapi_setsockopt(p_tCxt->sock_peer, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt));

    /* Configure queue sizes */
    bench_config_queue_size(p_tCxt->sock_peer);

    /* Connect to the server.*/
    QCLI_Printf("Connecting\n");
    if (qapi_connect( p_tCxt->sock_peer, to, tolen) == A_ERROR)
    {
        QCLI_Printf("Connection failed.\n");
        goto ERROR_2;
    }

    if (p_tCxt->protocol == SSL && p_tCxt->test_type == TX)
    {
        if (ssl->ssl == QAPI_NET_SSL_INVALID_HANDLE)
        {
            // Create SSL connection object
            ssl->ssl = qapi_Net_SSL_Con_New(ssl->sslCtx, QAPI_NET_SSL_TLS_E);
            if (ssl->ssl == QAPI_NET_SSL_INVALID_HANDLE)
            {
                QCLI_Printf("ERROR: Unable to create SSL context\n");
                goto ERROR_2;
            }

            // configure the SSL connection
            if (ssl->config_set)
            {
                result = qapi_Net_SSL_Configure(ssl->ssl, &ssl->config);
                if (result < QAPI_OK)
                {
                    QCLI_Printf("ERROR: SSL configure failed (%d)\n", result);
                    goto ERROR_2;
                }
            }

        }

        // Add socket handle to SSL connection
        result = qapi_Net_SSL_Fd_Set(ssl->ssl, p_tCxt->sock_peer);
        if (result < 0)
        {
            QCLI_Printf("ERROR: Unable to add socket handle to SSL (%d)\n", result);
            goto ERROR_2;
        }

        // SSL handshake with server
        app_get_time(&p_tCxt->pktStats.first_time);
        result = qapi_Net_SSL_Connect(ssl->ssl);
        app_get_time(&p_tCxt->pktStats.last_time);
        QCLI_Printf("TLS Handshake time: %d ms\n", app_get_time_difference(&p_tCxt->pktStats.first_time, &p_tCxt->pktStats.last_time));

        if (result < 0)
        {
            if (result == QAPI_SSL_OK_HS)
            {
                /** The peer's SSL certificate is trusted, CN matches the host name, time is valid */
                QCLI_Printf("The certificate is trusted\n");
            }
            else if (result == QAPI_ERR_SSL_CERT_CN)
            {
                /** The peer's SSL certificate is trusted, CN matches the host name, time is expired */
                QCLI_Printf("ERROR: The certificate is expired\n");
                goto ERROR_2;
            }
            else if (result == QAPI_ERR_SSL_CERT_TIME)
            {
                /** The peer's SSL certificate is trusted, CN does NOT match the host name, time is valid */
                QCLI_Printf("ERROR: The certificate is trusted, but the host name is not valid\n");
                goto ERROR_2;
            }
            else if (result == QAPI_ERR_SSL_CERT_NONE)
            {
                /** The peer's SSL certificate is trusted, CN does NOT match host name, time is expired */
                QCLI_Printf("ERROR: The certificate is expired and the host name is not valid\n");
                goto ERROR_2;
            }
            else
            {
                QCLI_Printf("ERROR: SSL connect failed (%d)\n", result);
                goto ERROR_2;
            }
        }
    }
    const char* alpn_protocol = qapi_Net_SSL_ALPN_Protocol_Get(ssl->ssl);
    if (alpn_protocol) {
        QCLI_Printf("ALPN Negotiated Protocol: %s\n", alpn_protocol);
    }

    QCLI_Printf("Sending\n");

    if (zerocopy_send)
    {
        netbuf_id = QAPI_NETBUF_SYS;
        if (p_tCxt->protocol == SSL && p_tCxt->test_type == TX) {
            netbuf_id |= QAPI_NETBUF_SSL;
        }
        send_flag = MSG_ZEROCOPYSEND;
    }
    else
    {
        netbuf_id = QAPI_NETBUF_APP;
        send_flag = 0;
    }

    app_get_time(&p_tCxt->pktStats.first_time);

    if (p_tCxt->is_iperf) {
        iperf_display_interval = p_tCxt->pktStats.iperf_display_interval;
        iperf_display_last = p_tCxt->pktStats.first_time.seconds ;
        iperf_display_next = iperf_display_last + iperf_display_interval;

    }
    

    while (1)
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
                                i = 16;
                                uint32_t end_pattern_index = packet_size - 6;
                                uint32_t bytes_to_copy = 0;
                                while ( i < end_pattern_index ) {
                                    #define MIN(a, b) ( (a<b) ? (a) : b )
                                    bytes_to_copy = MIN(sizeof(g_incremental_pattern), end_pattern_index - i);
                                    qapi_Net_Buf_Update(p_tCxt->buffer, i, g_incremental_pattern, bytes_to_copy, netbuf_id);
                                    i += bytes_to_copy;
                                }
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
        		idx = 8;
        	else
        		idx = 0;

                    if (!p_tCxt->is_iperf)
                    {
			    /* Packet index */
			    val = htonl(cur_packet_number);
			    qapi_Net_Buf_Update(p_tCxt->buffer, idx, &val, 4, netbuf_id);
                    }
                    else
                    {
			    val = htonl(cur_packet_number);
			    qapi_Net_Buf_Update(p_tCxt->buffer, 0, &val, 4, netbuf_id);                        
                    }
		}

        uint32_t bytes_to_send = packet_size - buffer_offset;
        int bytes_sent = 0;

        if (p_tCxt->protocol == SSL && p_tCxt->test_type == TX)
        {
            bytes_sent = qapi_Net_SSL_Write(ssl->ssl, &p_tCxt->buffer[buffer_offset], bytes_to_send);
        }
        else
        {
            bytes_sent = qapi_send( p_tCxt->sock_peer, &p_tCxt->buffer[buffer_offset], bytes_to_send, send_flag);
        }

        ++j;
        QCLI_Printf("%d bytes_sent = %d\n", j, bytes_sent);

        if (++i >= BENCH_TCP_PKTS_PER_DOT && !p_tCxt->print_buf)
        {
            QCLI_Printf(".");
            i = 0;
        }

        if ( bytes_sent != bytes_to_send )
        {
            int errno = qapi_errno(p_tCxt->sock_peer);

            if ( errno != ENOBUFS )
            {
            	QCLI_Printf("\nFailed on a call to qapi_send, bytes_sent=%d, errno=%d\n", bytes_sent, errno);
                // the socket has closed - no point of continuing the test
                QCLI_Printf("Socket got closed\n");
                if ( zerocopy_send && !p_tCxt->params.tx_params.v6 && (bytes_sent >= 0) && (bytes_sent < bytes_to_send) )
                {
                    // this is a special case of zero-copy partial transmission of IPv4 buffer.  In this case
                    // the buffer is actually freed inside qapi_send(), so we must not free it again.
                    p_tCxt->buffer = NULL;
                }

                app_get_time(&p_tCxt->pktStats.last_time);
                break;
            }
            else
            {
                // severe push back, let the other processes run (won't happen on blocking sockets)
                qapi_Task_Delay(1000);
            }
        }

        // Useful notes related to TCP zerocopy send:
        //
        // In TCP zerocopy send we can have the following qapi_send() return codes:
        // IPv4:
        //  return_code < 0: the buffer was not sent, qapi_send() did not free the buffer if qapi_errno() returns ENOBUFS.
        //  return_code == packet_size: the buffer was sent successfully, qapi_send() freed the buffer
        //  return_code < packet_size: the buffer sent partially, qapi_send() freed the buffer
        // IPv6:
        //  return_code < 0: the buffer was not sent, qapi_send() did not free the buffer
        //  return_code == packet_size: the buffer was sent successfully, qapi_send() freed the buffer
        //
        // Please note that for TCP zerocopy send there are no partial RE-transmit.  For TCP zerocopy send,
        // if the buffer is transmitted partially, the remaining of the buffer is freed (lost).

        if ( bytes_sent >= 0  )
        {
            p_tCxt->pktStats.bytes += bytes_sent;

            if ( bytes_sent == bytes_to_send )
            {
                cur_packet_number++;
                buffer_offset = 0;
            }
            else
            {
                buffer_offset += bytes_sent;
            }

            if ( zerocopy_send )
            {
                buffer_offset = 0;
                p_tCxt->buffer = NULL;
            }

            if (p_tCxt->print_buf)
            	bench_print_buffer(p_tCxt->buffer, bytes_sent, to, DUMP_DIRECTION_TX);
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

        // check the test completion condition based on number of packets sent
        if (p_tCxt->params.tx_params.test_mode == PACKET_TEST)
        {
            if ((cur_packet_number >= p_tCxt->params.tx_params.packet_number))
            {
                /* Test completed, print throughput results.*/
                app_get_time(&p_tCxt->pktStats.last_time);
                break;
            }
        }

        // check the test comletion condition based on time
        if ( p_tCxt->params.tx_params.test_mode == TIME_TEST )
        {
            app_get_time(&p_tCxt->pktStats.last_time);

            if (bench_common_check_test_time(p_tCxt))
            {
                /* Test completed, print test results.*/
                break;
            }
        }

        /****Bandwidth control- add delay if user has specified it************/
/*        if (p_tCxt->params.tx_params.interval_us)
            qapi_Task_Delay(p_tCxt->params.tx_params.interval_us);
*/
    } /* send loop */

    if ( p_tCxt->buffer )
    {
        qapi_Net_Buf_Free(p_tCxt->buffer, netbuf_id);
    }

    if (p_tCxt->is_iperf)
    {
        iperf_result_print(&p_tCxt->pktStats, 0, 0);
    }
    else
    {
        QCLI_Printf("\nSent %u/%u messages, %llu bytes to %s %d\n",
            cur_packet_number, j, p_tCxt->pktStats.bytes, ip_str, p_tCxt->params.tx_params.port);
    }


ERROR_2:

    if (!p_tCxt->is_iperf)
    {    
        bench_common_print_test_results(p_tCxt, &p_tCxt->pktStats);
    }

    if (ssl->ssl != QAPI_NET_SSL_INVALID_HANDLE)
    {
        qapi_Net_SSL_Shutdown(ssl->ssl);
        ssl->ssl = QAPI_NET_SSL_INVALID_HANDLE;
    }
    qapi_socketclose( p_tCxt->sock_peer);

ERROR_1:
    QCLI_Printf(BENCH_TEST_COMPLETED);

    return;
}
