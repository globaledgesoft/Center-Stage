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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "qcli.h"
#include "qcli_api.h"

#include "qurt_thread.h"
#include "qurt_types.h"
#include "qurt_timer.h"
#include "qurt_mutex.h"
#include "qurt_error.h"

#include "qapi_socket.h"
#include "qapi_timer.h"
#include "net_utils.h"
#include "qapi_ns_utils.h"
#include "qc_api_net.h"
#include "qc_drv_net.h"
#include "qc_api_main.h"
#include "qosa_util.h"

/*-------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 *-----------------------------------------------------------------------*/
#define MAX_NO_OF_SESSIONS                   16
#define SEND_THREAD_PRIORITY                 10
#define RECV_THREAD_PRIORITY                 10
#define THRD_STACK_SIZE                      2048
#define SIZE_BUF                             1500
#define IPV4_MODE                            4
#define IPV6_MODE                            6
#define TCP_PROTO                            0
#define UDP_PROTO                            1
#define RECV_BYTES                           1500

// Signals to synchronize
#define STOP_SIGNAL                          (0x1 << 0)
#define SEND_STOPPED_SIGNAL                  (0x1 << 1)
#define RECV_STOPPED_SIGNAL                  (0x1 << 2)

/*-------------------------------------------------------------------------
 * Type Declarations
 *-----------------------------------------------------------------------*/
typedef struct sock_info {
    uint32_t valid;
    uint32_t server;
    uint16_t ip_mode;
    uint16_t local_port;
    uint16_t peer_port;
    int32_t  sock_id;
    uint32_t proto;
    int32_t  session_id;
    uint32_t sent_bytes;
    uint32_t recv_bytes;
    uint8_t  local_ip[48];
    uint8_t  peer_ip[48];
} sock_info_t;

/*-------------------------------------------------------------------------
 * Static & global Variable Declarations
 *-----------------------------------------------------------------------*/
extern QCLI_Group_Handle_t qcli_net_handle;
static sock_info_t session[MAX_NO_OF_SESSIONS];
static fd_set rset, sockset;
static uint32_t num_session;
static uint32_t signal_set;
static qurt_mutex_t slock;
static sock_info_t *active_session;

static char recv_buf[RECV_BYTES];
static qurt_signal_t stop_signal;

/*-------------------------------------------------------------------------
 * Function Declarations
 *-----------------------------------------------------------------------*/
#define PRINT_SESSION_DETAILS
#ifdef PRINT_SESSION_DETAILS
static void print_session_details(sock_info_t *psession);
#endif


/*-------------------------------------------------------------------------
 *Function_name : session_set_info : copies the session details
 *-------------------------------------------------------------------------*/
static void session_set_info(sock_info_t *psession, uint8_t server, uint32_t ip_mode, uint32_t protocol, uint8_t *ip_addr, uint16_t port)
{
    psession->ip_mode = ip_mode;
    psession->proto = protocol;
    psession->server = !!server;
    if (server)
    {
        strcpy((char *)psession->local_ip, (char *)ip_addr);
        psession->local_port = port;
    }
    else
    {
        strcpy((char *)psession->peer_ip, (char *)ip_addr);
        psession->peer_port = port;
    }
    psession->sent_bytes = 0;
    psession->recv_bytes = 0;
}

/*-------------------------------------------------------------------------
 * Function_name :sock_close - Closes the socket for corresponding session
 *-------------------------------------------------------------------------*/
static int32_t sock_close(sock_info_t *psession)
{
    int index;

    if (psession->sock_id != -1)
    {
        qc_drv_net_fd_clr(qc_api_get_qc_drv_context(), psession->sock_id, &rset);
        if(0 != qc_drv_net_socketclose(qc_api_get_qc_drv_context(), psession->sock_id))
        {
            LOG_ERR("sock _closing is failed\n");
            return -1;
        }
    }
    for (index = 0; index < MAX_NO_OF_SESSIONS; index++)
        print_session_details(&session[index]);

    return 0;
}

/*-------------------------------------------------------------------------
 * Function_name : sock_open
 * Return value: Socket descriptor for succsess , -1 for failure
 * Opens the Socket for corresponding session
 *-------------------------------------------------------------------------*/
static int sock_open(sock_info_t *psession)
{
    int32_t socket = -1;

    socket = qc_drv_net_socket(qc_api_get_qc_drv_context(), psession->ip_mode, psession->proto, 0);
    if (socket < 0)
    {
        LOG_ERR("Socket creation error\n");
        return -1;
    }

    if (psession->server)
    {
        LOG_INFO("SetSock:%d\n", qc_drv_net_setsockopt(qc_api_get_qc_drv_context(), socket, SOL_SOCKET, SO_NBIO, NULL, 0));
    }

    LOG_DEBUG("SocketID:%d\n", socket);
    return socket;
}

#if 0
/*----------------------------------------------------------------------------
 *Function_name:process_multi_sock_close
 * To Close the sessions
 *---------------------------------------------------------------------------*/
static void sock_close_all(void)
{
    int32_t index;

    for (index = 0; index < MAX_NO_OF_SESSIONS; index++)
    {
        if (session[index].valid)
        {
            sock_close(&session[index]);
        }
    }
    memset(session, 0, sizeof(session));
}
#endif
/*-------------------------------------------------------------------------
 * Function_name :fill_server_details_connect
 * Return Value : 0 for succsess -1 for failue
 * Fill the serverdetails in server address and connect to the server if it is TCP protocol
 *-------------------------------------------------------------------------*/
static int32_t session_create(sock_info_t *psession)
{
    int ret;
    struct sockaddr *addr;
    struct sockaddr_in serv_addr;
    struct sockaddr_in6 serv6_addr;
    uint8_t *ipaddr;
    uint16_t port;
    int32_t len_addr = 0;

    if (psession->server)
    {
        ipaddr = psession->local_ip;
        port = psession->local_port;
    }
    else
    {
        ipaddr = psession->peer_ip;
        port = psession->peer_port;
    }

    if (psession->ip_mode == AF_INET)
    {
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port);
		if (inet_pton(AF_INET, (const char*)ipaddr, &serv_addr.sin_addr.s_addr) != 0)
        {
            LOG_ERR("Invalid IPv4 address\n");
            return -1;
        }
        addr = (struct sockaddr *)&serv_addr;
        len_addr = (int32_t) sizeof(serv_addr);
    }
    else if (psession->ip_mode == AF_INET6)
    {
        serv6_addr.sin_family = AF_INET6;
        serv6_addr.sin_port = htons(port);
        if (inet_pton(AF_INET6, (const char*)ipaddr, &serv6_addr.sin_addr.s_addr) != 0)
        {
            LOG_ERR("Invalid IPv6 address\n");
            return -1;
        }
        //memcpy(&serv6_addr.sin_addr.s_addr, ipaddr, sizeof(serv6_addr.sin_addr.s_addr));
        addr = (struct sockaddr *)&serv6_addr;
        len_addr = (int32_t)sizeof(serv6_addr);

    }
    else
    {
        return -1;
    }

    if (!psession->server && psession->proto == SOCK_STREAM)
    {
        LOG_INFO("Connecting to port:%d ip:%s\n", port, ipaddr);
        if (qc_drv_net_connect(qc_api_get_qc_drv_context(), psession->sock_id, (struct sockaddr *)addr, len_addr))
        {
            LOG_ERR("connection error on the socket: %d, session: %d\n", psession->sock_id, num_session);
            return -1;
        }
    }
    else if(psession->server)
    {
        LOG_INFO("Binding to port:%d ip:%s\n", port, ipaddr);
        ret = qc_drv_net_bind(qc_api_get_qc_drv_context(), psession->sock_id, (struct sockaddr *) addr, len_addr);
        if (ret < 0)
        {
            LOG_ERR("Socket binding error %d\n", ret);
            return -1;
        }
        /* set to non-blocking mode */
        LOG_DEBUG("SetSock:%d\n", qc_drv_net_setsockopt(qc_api_get_qc_drv_context(), psession->sock_id, SOL_SOCKET, SO_NBIO, NULL, 0));

        if (psession->proto == SOCK_STREAM)
        {
            LOG_INFO("Listening on port:%d ip:%s\n", port, ipaddr);
            ret = qc_drv_net_listen(qc_api_get_qc_drv_context(), psession->sock_id, 3);
            if (ret < 0)
            {
                LOG_ERR("Socket listen error %d\n", ret);
                return -1;
            }
        }
    }
    else
    {
        LOG_DEBUG("UDP Socket no connection required\n");
    }

    qc_drv_net_fd_set(qc_api_get_qc_drv_context(), psession->sock_id, &rset);
#ifdef PRINT_SESSION_DETAILS
	print_session_details(psession);
#endif
    num_session++;

    return 0;
}

/*-------------------------------------------------------------------------
 *  Function_name : session_delete
 *  Return Value : None
 *-------------------------------------------------------------------------*/
static void session_delete(uint32_t index)
{
    qurt_mutex_lock(&slock);
    if (index < MAX_NO_OF_SESSIONS && session[index].valid)
    {
        sock_close(&session[index]);
        session[index].valid = 0;
        num_session--;
    }
    qurt_mutex_unlock(&slock);
}

/*-------------------------------------------------------------------------
 *  Function_name : session_allocate
 *  Return Value : <session index> for succsess -1 for failure
 *-------------------------------------------------------------------------*/
static int32_t session_allocate(uint8_t server, uint32_t ip_mode, uint32_t proto, uint16_t port, uint8_t *ip)
{
    uint32_t index = 0;
    //int32_t sock_id;
    //int32_t status = 0;

    qurt_mutex_lock(&slock);
    for (index = 0; index < MAX_NO_OF_SESSIONS; index++)
    {
        if (!session[index].valid)
        {
            break;
        }
    }

    // No free slot/session available
    if (index >= MAX_NO_OF_SESSIONS)
    {
        qurt_mutex_unlock(&slock);
        return -1;
    }

    memset(&session[index], 0, sizeof(sock_info_t));
    session_set_info(&session[index], server, ip_mode, proto, ip, port);
    session[index].session_id = index;
    session[index].sock_id = -1;
    qurt_mutex_unlock(&slock);

    return index;
}

#ifdef PRINT_SESSION_DETAILS
/*-------------------------------------------------------------------------
 * Function_name :  print_session_details - Prints the session details
 *-------------------------------------------------------------------------*/
static void print_session_details(sock_info_t *psession)
{
    if (psession->valid)
    LOG_INFO("%d) %s sid:%d\tproto:%d\tP:%d\tIP:%s\t\n",
            psession->session_id,
            psession->server ? "Server" : "Client",
            psession->sock_id,
            psession->proto,
            psession->server ? psession->local_port:psession->peer_port,
            psession->server ? psession->local_ip:psession->peer_ip);
}
#endif
/*-------------------------------------------------------------------------
 * Function_name : Recv_Thread_Multisock
 * Thread for handling socket descriptors when message received from the server
 *-------------------------------------------------------------------------*/
static void Recv_Thread_Multisock(void *param)
{
    uint32_t index, index1;
    int32_t ret;
    int32_t len_addr;
    int32_t sock_handle, newSocket;
    struct sockaddr *addr = NULL;
    struct sockaddr_in serv_addr;
    struct sockaddr_in6 serv6_addr;
    sock_info_t *cur = NULL;
    uint8_t ipaddr[48];

    while(1)
    {
        qc_drv_net_fd_zero(qc_api_get_qc_drv_context(), &sockset);
        qurt_mutex_lock(&slock);
        sockset = rset;
        qurt_mutex_unlock(&slock);

        //QCLI_Printf("FDC:%d FD SET %d %d %d %d\n", rset.fd_count, rset.fd_array[0], rset.fd_array[1], rset.fd_array[2], rset.fd_array[3]);
        //QCLI_Printf("FDC:%d\n", rset.fd_count);
        ret = qc_drv_net_select(qc_api_get_qc_drv_context(), &sockset, NULL, NULL, 4000);
        if (signal_set == STOP_SIGNAL)
        {
            LOG_AT_EVT("EVT_NET: THREAD_STOP Exiting Recv thread\n");
            QCLI_Display_Prompt();
            break;
        }
        if (ret == 0)
        {
            continue;
        }
        else if (sockset.fd_count == 0)
        {
	    /* Consol will not come if it is commented */                       
            LOG_AT("\n");
	    //qurt_thread_sleep(10);
            continue;
        }
        else
        {
            //QCLI_Printf("FDC:%d FD SET %d %d %d %d\n", sockset.fd_count, sockset.fd_array[0], sockset.fd_array[1], sockset.fd_array[2], sockset.fd_array[3]);
            LOG_DEBUG("FDC:%d\n", sockset.fd_count);
            qurt_mutex_lock(&slock);
            for (index = 0; index < MAX_NO_OF_SESSIONS; index++)
            {
                if (session[index].valid)
                {
                    if (qc_drv_net_fd_isset(qc_api_get_qc_drv_context(), session[index].sock_id, &sockset))
                    {
                        cur = &session[index];
                        sock_handle = cur->sock_id;

                        //if (cur->proto == SOCK_DGRAM)
                        {
                            if (cur->ip_mode == AF_INET)
                            {
                                addr = (struct sockaddr *)&serv_addr;
                                len_addr = sizeof(serv_addr);
                            }
                            else if (cur->ip_mode == AF_INET6)
                            {
                                addr = (struct sockaddr *)&serv6_addr;
                                len_addr = sizeof(serv6_addr);
                            }
                        }
                        //LOG_AT_EVT("EVT_NET: Event on sessionid:%d sockid:%d\n", index, sock_handle);

                        if (cur->server && (cur->proto==SOCK_STREAM))
                        {
                            LOG_DEBUG("Server sock Accepting connection\n");
                            newSocket = qc_drv_net_accept(qc_api_get_qc_drv_context(), sock_handle, (struct sockaddr *)addr, &len_addr);
                            if (newSocket <= 0)
                            {
                                LOG_ERR("Socket accept error %d\n", ret);
                                qurt_mutex_unlock(&slock);
                                continue;
                            }
                            if (cur->ip_mode == AF_INET)
                                inet_ntop(cur->ip_mode, &addr->u.sin.sin_addr, (char *)ipaddr, sizeof(ipaddr));
                            if (cur->ip_mode == AF_INET6)
                                inet_ntop(cur->ip_mode, &addr->u.sin6.sin_addr, (char *)ipaddr, sizeof(ipaddr));

                            index1 = session_allocate(0, cur->ip_mode, cur->proto, ntohs(addr->sa_port), ipaddr);
                            if (index < 0)
                            {
                                LOG_WARN("Max session reached!\n");
                                qc_drv_net_socketclose(qc_api_get_qc_drv_context(), newSocket);
                                qurt_mutex_unlock(&slock);
                                continue;
                            }
                            // Activate the session by settin sockid and valid flag
                            session[index1].sock_id = newSocket;
                            session[index1].valid = 1;

                            // Add to select list
                            LOG_DEBUG("SetSock:%d\n", qc_drv_net_setsockopt(qc_api_get_qc_drv_context(), newSocket, SOL_SOCKET, SO_NBIO, NULL, 0));
                            qc_drv_net_fd_set(qc_api_get_qc_drv_context(), session[index1].sock_id, &rset);
#ifdef PRINT_SESSION_DETAILS
                            print_session_details(&session[index1]);
#endif
                            LOG_AT_EVT("EVT_NET: ACCEPT %d sessionid:%d sockid:%d\n", index1, index, sock_handle);
                            QCLI_Display_Prompt();
                            num_session++;
                        }
                        else
                        {
                            memset(recv_buf, 0, sizeof(recv_buf));
                            LOG_AT_EVT("EVT_NET: RECEIVE sessionid:%d sockid:%d\n", index, sock_handle);
                            qc_drv_net_fd_clr(qc_api_get_qc_drv_context(), sock_handle, &rset);
                            QCLI_Display_Prompt();
                        }
                    }
                }
            }
            qurt_mutex_unlock(&slock);
        }
    }

    LOG_AT_EVT("EVT_NET: THREAD_STOP Exiting Recv thread due to stop\n");
    qurt_signal_set(&stop_signal, RECV_STOPPED_SIGNAL);
    qurt_thread_stop();

    return;
}

/*---------------------------------------------------------------------------
  - Create  thread
  --------------------------------------------------------------------------*/
static int32_t create_thread(const char *thr_name, int32_t thr_prio, uint32_t thr_stack_size,
        void (*thread_func)(void *param))
{
    uint32_t len = 0;
    qurt_thread_attr_t attr;

    qurt_thread_t thid = 0;
    qurt_thread_attr_init(&attr);
    qurt_thread_attr_set_name(&attr, thr_name);
    qurt_thread_attr_set_priority(&attr, thr_prio);
    qurt_thread_attr_set_stack_size(&attr, thr_stack_size);
    if( 0 != qurt_thread_create(&thid, &attr, thread_func, &len))
    {
        LOG_ERR("Thread creation is failed\n");
        return -1;
    }

    return 0;
}

#if 0
/*----------------------------------------------------------------------------
 *Function_name: print_multi_sock_log_report
 *Prints session send and recv reports
 *--------------------------------------------------------------------------*/
static void print_multi_sock_log_report(void)
{
    int32_t index;

    QCLI_Printf("== Session Info ==\n");
    for (index = 0; index < num_session; index++)
    {
        QCLI_Printf("session Id: %d SndBytes: %u RcvBytes: %u\n",
                     session[index].session_id, session[index].sent_bytes,
                     session[index].recv_bytes);
    }
    num_session = 0;

    return;
}
#endif

int net_sock_send_data(char *tx_data, uint32_t data_len)
{
    int errno = 0;
    int32_t bytes_sent = 0;
    int32_t len_addr = 0;
    struct sockaddr *addr = NULL;
    struct sockaddr_in serv_addr;
    struct sockaddr_in6 serv6_addr;

    qurt_mutex_lock(&slock);

    if (active_session->proto == SOCK_STREAM)
    {
        bytes_sent = qc_drv_net_send(qc_api_get_qc_drv_context(), active_session->sock_id, tx_data, data_len, 0);
    }
    else if (active_session->proto == SOCK_DGRAM)
    {
        if (active_session->ip_mode == AF_INET)
        {
            serv_addr.sin_family = AF_INET;
            serv_addr.sin_port = htons(active_session->peer_port);
            if (inet_pton(AF_INET, (const char*)active_session->peer_ip, &serv_addr.sin_addr.s_addr) != 0)
            {
                LOG_ERR("Invalid IPv4 address\n");
                qurt_mutex_unlock(&slock);
                return -1;
            }
            addr = (struct sockaddr *)&serv_addr;
            len_addr = (int32_t) sizeof(serv_addr);
        }
        else if (active_session->ip_mode == AF_INET6)
        {
            serv6_addr.sin_family = AF_INET6;
            serv6_addr.sin_port = htons(active_session->peer_port);

            if (inet_pton(AF_INET6, (const char*)active_session->peer_ip, &serv6_addr.sin_addr.s_addr) != 0)
            {
                LOG_ERR("Invalid IPv6 address\n");
                qurt_mutex_unlock(&slock);
                return -1;
            }
            addr = (struct sockaddr *)&serv6_addr;
            len_addr = (int32_t)sizeof(serv6_addr);
        }
        else
        {
            LOG_ERR("Invalid Address Family\n");
        }

        bytes_sent = qc_drv_net_sendto(qc_api_get_qc_drv_context(), active_session->sock_id, tx_data, data_len, 0, addr, len_addr);
    }
    else
    {
        LOG_ERR("Invalid Protocol\n");
    }

    if ( bytes_sent != data_len )
    {
        errno = qc_drv_net_errno(qc_api_get_qc_drv_context(), active_session->sock_id);
        if ( errno != ENOBUFS )
        {
            LOG_ERR("Failed on a call to qapi_send, bytes_sent=%d, errno=%d\n", bytes_sent, errno);
        }
    }

    qurt_mutex_unlock(&slock);

    return errno;
}

int net_sock_set_active_session(int32_t sid, uint16_t portnum, uint8_t *ipaddr)
{
    if (sid < 0 || sid > MAX_NO_OF_SESSIONS)
        return -1;

    qurt_mutex_lock(&slock);
    active_session = &session[sid];
    if (!active_session->valid)
    {
        qurt_mutex_unlock(&slock);
        return -1;
    }

    if (portnum)
        active_session->peer_port = portnum;

    if (ipaddr)
        strcpy((char *)active_session->peer_ip, (char *)ipaddr);

    qurt_mutex_unlock(&slock);

    return 0;
}

void net_sock_close(uint32_t id)
{
    session_delete(id);
}

int net_sock_open(uint8_t server, uint32_t family, uint32_t proto, uint16_t portnum, uint8_t *ipaddr)
{
    int32_t status;
    uint32_t index;
    int32_t sock_id;

    qurt_mutex_lock(&slock);
    index = session_allocate(server, family, proto, portnum, ipaddr);
    if (index < 0)
    {
        LOG_ERR("Failed to allocate session\n");
        qurt_mutex_unlock(&slock);
        return -1;
    }

    sock_id = sock_open(&session[index]);
    if (sock_id < -1)
    {
        qurt_mutex_unlock(&slock);
        return -1;
    }
    session[index].sock_id = sock_id;

    status = session_create(&session[index]);
    if (status == -1)
    {
        sock_close(&session[index]);
        qurt_mutex_unlock(&slock);
        return -1;
    }

    // Activate the session
    session[index].valid = 1;
    qurt_mutex_unlock(&slock);

    return index;
}

int net_sock_read(int32_t sid)
{
    sock_info_t *pses;
    int32_t ret;
    int32_t len_addr;
    struct sockaddr *addr = NULL;
    struct sockaddr_in serv_addr;
    struct sockaddr_in6 serv6_addr;
    uint8_t ipaddr[48];

    if (sid < 0 || sid > MAX_NO_OF_SESSIONS)
        return -1;

    qurt_mutex_lock(&slock);
    pses = &session[sid];

    if (pses->ip_mode == AF_INET)
    {
        addr = (struct sockaddr *)&serv_addr;
        len_addr = sizeof(serv_addr);
    }
    else if (pses->ip_mode == AF_INET6)
    {
        addr = (struct sockaddr *)&serv6_addr;
        len_addr = sizeof(serv6_addr);
    }
    memset(recv_buf, 0, sizeof(recv_buf));

    ret = qc_drv_net_recvfrom(qc_api_get_qc_drv_context(), pses->sock_id, recv_buf, sizeof(recv_buf), 0, addr, &len_addr);
    if (ret > 0)
    {
        pses->recv_bytes += ret;
        if (pses->ip_mode == AF_INET)
            inet_ntop(pses->ip_mode, &addr->u.sin.sin_addr, (char *)ipaddr, sizeof(ipaddr));
        if (pses->ip_mode == AF_INET6)
            inet_ntop(pses->ip_mode, &addr->u.sin6.sin_addr, (char *)ipaddr, sizeof(ipaddr));

        LOG_AT("Received data L:%d PP:%d PIP:%s D:%s\r\n", ret, ntohs(addr->sa_port), ipaddr, recv_buf);
    }
    else if (ret == 0)
    {
        LOG_AT("No data\n");
    }
    else if (ret < 0)
    {
        LOG_ERR("Error on sessioid:%d. Closing session\n", sid);
        session_delete(sid);
        qurt_mutex_unlock(&slock);
        return -1;
    }

    if (!qc_drv_net_fd_isset(qc_api_get_qc_drv_context(), session[sid].sock_id, &rset))
        qc_drv_net_fd_set(qc_api_get_qc_drv_context(), session[sid].sock_id, &rset);
    qurt_mutex_unlock(&slock);

    return 0;
}

int net_sock_info(int32_t sid)
{
    if (sid < 0 || sid > MAX_NO_OF_SESSIONS)
        return -1;

    qurt_mutex_lock(&slock);
    print_session_details(&session[sid]);
    qurt_mutex_unlock(&slock);

    return 0;
}

int net_sock_initialize(void)
{
    qurt_mutex_init(&slock);
    qurt_signal_init(&stop_signal);

    if( create_thread("recv_thread", RECV_THREAD_PRIORITY, THRD_STACK_SIZE, Recv_Thread_Multisock))
    {
        LOG_ERR("Failed to create Recv Thread\n");
        return -1;
    }

    return 0;
}
