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

#include "qapi_socket.h"
#include "netutils.h"
#include "thread_util.h"
#include "log_util.h"
#include "qurt_error.h"
#include "sensor_json.h"

#include <qapi_netservices.h>

struct sockaddr_in6 client6_addr;
struct sockaddr_in6 server6_addr;
struct sockaddr_in6 border_router_addr;
char Global_IP_Addr[48];
const char IPv6_Interface[] = "ot0";

int32_t thread_sockid = -1;
int32_t sent_bytes_fail;
int32_t Recv_thread_stop = 0;

uint8_t thread_buf[300];
void ip_show(void)
{
    int32_t ret;
    ret = Thread_Get_IPV6_Addr(IPv6_Interface, Global_IP_Addr);
    if (ret != SUCCESS)
    {
        LOG_ERROR("Failed to get IPV6 address\n");
        return;
    }

}

boolean is_recv_thread_stopped(void)
{
    return !!Recv_thread_stop;
}

/** Send sensor data to Border Router */
int32_t Send_data_to_router(char *Thread_Payload)
{
    int32_t bytes_sent;
    uint32_t len;

    if (thread_sockid > 0)
    {
        check_route();
        LOG_INFO("Sending ----------\n");
        len = (int32_t) sizeof(border_router_addr);
        bytes_sent = qapi_sendto(thread_sockid, (char *) Thread_Payload, (int32_t) ((strlen((char *) Thread_Payload))+1),
                0, (struct sockaddr *) &border_router_addr, len);
        LOG_INFO("\n\n /*********** Sent Bytes: %d  **************/\n\n",bytes_sent);

        if (bytes_sent < 0 )
        {
            LOG_INFO("--------------------Routing information is adding---------------\n");
            sent_bytes_fail++;
            route6_show();
            if (sent_bytes_fail > 2 )
            {
                qapi_socketclose(thread_sockid);
                thread_sockid  = -1;
            }
        }
        else 
        {
            sent_bytes_fail = 0;
        }
    }
    return SUCCESS;
}

/** Receive data from Border Router */
void Joiner_Receive_Data()
{

    int32_t bytes;
    int32_t len;
    uint8_t IpAddr[48];


    len = (int32_t) sizeof(server6_addr);
    memset(IpAddr, 0, sizeof(IpAddr));

    const struct in6_addr in6addr_any;
    client6_addr.sin_addr = in6addr_any;
    while (1)
    {

        memset(thread_buf, 0, sizeof(thread_buf));


        bytes = qapi_recvfrom(thread_sockid, (char *) thread_buf, sizeof(thread_buf)-1, 0, (struct sockaddr *)&client6_addr, &len);

        LOG_INFO("Received bytes are: %d\t %s\n", bytes, thread_buf);
        if( bytes > 0)
        {
            thread_buf[bytes] = '\0';
            LOG_INFO("Received bytes are: %d\t %s\n", bytes, thread_buf);
#if AWS_IOT
            parse_recived_data((char *)thread_buf);
#endif
            LOG_INFO("Parsed receive data done\n");
        }
        qurt_thread_sleep(5000);
        if (bytes < 0)
        {
            Recv_thread_stop =1;
            qurt_thread_stop();
        }
    }


}

/** Start UDP comminication on Thread network */
void Thread_Udp_Joiner()
{
    int32_t ret;
    char str[INET6_ADDRSTRLEN];

    qurt_thread_attr_t Thread_Attribte;
    qurt_thread_t      Thread_Handle;
    int                Result;


    Recv_thread_stop = 0;
    /* Start the main demo thread. */
    qurt_thread_attr_init(&Thread_Attribte);
    qurt_thread_attr_set_name(&Thread_Attribte, "Receiving Thread");
    qurt_thread_attr_set_priority(&Thread_Attribte, RECEIVE_THREAD_PRIORITY);
    qurt_thread_attr_set_stack_size(&Thread_Attribte, RECEIVE_THREAD_STACK_SIZE);


    server6_addr.sin_family = AF_INET6;
    server6_addr.sin_port = htons(THRD_UDP_PORT);
    server6_addr.sin_scope_id = 0;

    memset(Global_IP_Addr, 0, sizeof(Global_IP_Addr));

    ret = Thread_Get_IPV6_Addr(IPv6_Interface, Global_IP_Addr);
    if (ret != SUCCESS)
    {
        LOG_ERROR("Failed to get IPV6 address\n");
        return;
    }
    LOG_INFO("ot0 interface Global IP Address :%s\n",Global_IP_Addr);
    inet_pton(AF_INET6, Global_IP_Addr, &(server6_addr.sin_addr.s_addr));
    // now get it back and print it
    memset(str, 0, sizeof(str));
    inet_ntop(AF_INET6, &(server6_addr.sin_addr.s_addr), str, INET6_ADDRSTRLEN);

    LOG_INFO("Joiner IP: %s\n", str); // prints IPV6 address

    border_router_addr.sin_family = AF_INET6;
    border_router_addr.sin_port = htons(THRD_UDP_PORT);
    border_router_addr.sin_scope_id = 0;

    inet_pton(AF_INET6, "FD00:102:304::B836:2913:702D:9C9C", &(border_router_addr.sin_addr));

    // now get it back and print it
    inet_ntop(AF_INET6, &(border_router_addr.sin_addr), str, INET6_ADDRSTRLEN);

    LOG_INFO("%s\n", str); // prints IPV6 address


    thread_sockid = qapi_socket(AF_INET6, SOCK_DGRAM, 0);
    LOG_INFO("\n\nserver_sockid:%d\n\n",thread_sockid);
    if (thread_sockid < 0)
    {
        LOG_ERROR("Failed to open socket %d\n", thread_sockid);
        return;
    }

    Result = qurt_thread_create(&Thread_Handle, &Thread_Attribte, Joiner_Receive_Data, NULL);
    if(Result != QURT_EOK)
    {
        LOG_ERROR("Failed to start QCLI thread.");
    }
}
