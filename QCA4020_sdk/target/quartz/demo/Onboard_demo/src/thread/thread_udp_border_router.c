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

struct sockaddr_in6 client6_addr;
struct sockaddr_in6 server6_addr;

int32_t thread_sockid;
static int32_t sock_close = 0;

void Socket_close()
{
    qapi_socketclose(thread_sockid);
    sock_close = 1;

}
void Border_Router_Receive_Data()
{
    char thread_buf[512];
    int32_t bytes;
    int32_t len;
    char IpAddr[48];

    len = (int32_t) sizeof(server6_addr);
    memset(IpAddr, 0, sizeof(IpAddr));
    while (1)
    {
        memset(thread_buf, 0, sizeof(thread_buf));
        LOG_INFO(" Receiving Data from Joiner\n");
        bytes = qapi_recvfrom(thread_sockid, thread_buf, sizeof(thread_buf)-1, 0, (struct sockaddr *)&client6_addr, &len);
        if (bytes > 0)
        {
            thread_buf[bytes] = '\0';
            LOG_INFO("Received bytes are: %d\t%s\n", bytes, thread_buf);
            memset(IpAddr, 0, sizeof(IpAddr));
            inet_ntop(AF_INET6, &client6_addr.sin_addr.s_addr, IpAddr, sizeof(IpAddr));
            Getting_Data_From_Joiner(thread_buf, IpAddr);
            Thread_ShowDeviceList();
            qurt_thread_sleep(200);
        }
        if (sock_close)
        {
            LOG_INFO("Recv thread stopped\n"); 
            sock_close = 0;
            qurt_thread_stop();
        }
    }
}

void Thread_Udp_Border_Router(void)
{
    int32_t ret;
    int32_t len;
    char Global_IP_Addr[48];
    const char IPv6_Interface[] = "ot0";
    char str[INET6_ADDRSTRLEN];

    qurt_thread_attr_t Thread_Attribte;
    qurt_thread_t      Thread_Handle;
    int                Result;
    const struct in6_addr in6addr_any;
    /* Start the main demo thread. */
    qurt_thread_attr_init(&Thread_Attribte);
    qurt_thread_attr_set_name(&Thread_Attribte, "Receiving Thread");
    qurt_thread_attr_set_priority(&Thread_Attribte, RECEIVE_THREAD_PRIORITY);
    qurt_thread_attr_set_stack_size(&Thread_Attribte, RECEIVE_THREAD_STACK_SIZE);


    server6_addr.sin_family = AF_INET6;
    server6_addr.sin_port = htons(THRD_UDP_PORT);
    server6_addr.sin_scope_id = 0;

    ret = Thread_Get_IPv6_Addr(IPv6_Interface, Global_IP_Addr);
    if (ret != SUCCESS)
    {
        LOG_ERROR("Failed to get IPV6 address\n");
    }
    LOG_INFO("ot0 interface Global IP Address :%s\n",Global_IP_Addr);
    inet_pton(AF_INET6, Global_IP_Addr, &(server6_addr.sin_addr.s_addr));
    // now get it back and print it
    memset(str, 0, sizeof(str));
    inet_ntop(AF_INET6, &(server6_addr.sin_addr.s_addr), str, INET6_ADDRSTRLEN);

    LOG_INFO("Border ROuter IP: %s\n", str); // prints IPV6 address

    client6_addr.sin_family = AF_INET6;
    client6_addr.sin_port = htons(THRD_UDP_PORT);
    client6_addr.sin_scope_id = 0;
    client6_addr.sin_addr = in6addr_any;

    thread_sockid = qapi_socket(AF_INET6, SOCK_DGRAM, 0);
    if (thread_sockid < 0)
    {
        LOG_ERROR("Failed to open socket %d\n", thread_sockid);
        return;
    }

    len = sizeof(server6_addr);
    ret = qapi_bind(thread_sockid, (struct sockaddr *)&server6_addr, len);
    if (ret < 0)
    {
        LOG_ERROR("Socket binding error %d\n", ret);
        return;
    }

    LOG_INFO("Server started.........\n");

    Result = qurt_thread_create(&Thread_Handle, &Thread_Attribte, Border_Router_Receive_Data, NULL);
    if(Result != QURT_EOK)
    {
        LOG_ERROR("Failed to start QCLI thread.");
    }

    return;
}
