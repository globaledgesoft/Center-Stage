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

#ifndef  __THREAD_UTIL_H__
#define  __THREAD_UTIL_H__
#include "qapi_types.h"
#include "qcli_api.h"
#include "qapi_socket.h"
#include "qurt_thread.h"
#include "led_utils.h"
#include "qurt_mutex.h"
#include "aws_util.h"
#include <qapi_netservices.h>

#define BORDER_ROUTER
#define JOINER_ROUTER           	  0
#define JOINER              	          1
#define JOINER_TIMEOUT	     	      65000
#define JOINER_EXTADDR	                  0ULL
#define TRUE                     	  1
#define FALSE                    	  0
#define THRD_UDP_PORT 		       9000
#define JOINER_POLL_PERIOD	 	  1
#define THREAD_DEV_ID_LIST_SIZE       	  4 
#define THREAD_PAYLOAD_LENGTH 	         600 
#define RECEIVE_THREAD_PRIORITY    	 10
#define RECEIVE_THREAD_STACK_SIZE      2048
#define MAX_THREAD_KEY_SIZE 		 16
#define BOARD_NAME_SIZE  		 20
#define INET6_ADDRSTRLEN  		 48
#define PIR_PAYLOAD_LENGTH              100
#define BOARD_NAME_BUF_SIZE              32

void Initialize_Thread_Demo(void);

typedef struct Thread_Device_ID_s
{
    qbool_t	        	InUse;
    uint8_t             BoardName[BOARD_NAME_SIZE];
    uint8_t             IpAddr[INET6_ADDRSTRLEN];
    uint8_t             Buffer[THREAD_PAYLOAD_LENGTH];
    uint8_t             BreachBuf[THREAD_PAYLOAD_LENGTH];
    uint32_t            Cmd_Id;
}Thread_Device_ID_t;

typedef struct Thread_Context_s
{
    Thread_Device_ID_t        DevIDList[THREAD_DEV_ID_LIST_SIZE + 1];
}Thread_Context_t;

uint32_t Thread_Initialize(uint32_t Device_Config);
uint32_t Thread_UseDefaultInfo(void);
uint32_t Thread_Interface_Start(void);
uint32_t Thread_MeshCoP_CommissionerStart(void);
uint32_t Thread_MeshCoP_CommissionerAddJoiner(const char *PassPhrase, uint64_t ExtAddr, uint32_t TimeOut);
uint32_t Thread_AddBorderRouter(char *Ipv6PrefixAddr, uint8_t PrefixLen, uint32_t Preference, uint32_t Stability, uint32_t BRConfigFlags);
uint32_t RegisterServerData(void);
uint32_t Thread_StartBorderAgent(char * Wlan_Interface);
uint32_t Thread_MeshCoP_JoinerStart(const char * PassPhrase);
uint32_t Thread_SetIPStackInteg(uint32_t IP_Stack_Enable);
uint32_t Thread_Get_IPv6_Addr(const char *Interface, char *Global_IP_Str);
uint32_t Thread_AddUnicastAddress(const char *IPv6_Addr,uint8_t Prefix_Len, uint32_t Preferred);
uint32_t Thread_Get_Interfaces(void);
uint32_t Thread_SetPollPeriod(uint32_t Poll_Period);


uint32_t Thread_AddBorderRouter(char *Ipv6PrefixAddr, uint8_t PrefixLen, uint32_t Preference, uint32_t Stability, uint32_t BRConfigFlags);
uint32_t RegisterServerData(void);
uint32_t Thread_StartBorderAgent(char * Wlan_Interface);
void Thread_Udp_Server(void);
void Thread_Udp_Joiner(void);
void Thread_Udp_Border_Router(void);
void Getting_Data_From_Joiner(char*, char*);
void Thread_ShowDeviceList();
int32_t Start_Thread_Router(const char *PassPhrase);
int32_t Start_Thread_Boarder_Router(const char *PassPhrase);
int32_t Start_Thread_Joiner(const char *PassPhrase);
void get_board_name(char* Thread_Payload, char* board_name);
void route6_show(void);
uint32_t route6_add(const char * ifname, ip6_addr * dest, uint32_t prefixlen, ip6_addr * nexthop);
uint32_t route6_del(const char * ifname, ip6_addr * dest, uint32_t prefixlen);
uint32_t Thread_Get_IPV6_Addr(const char *Interface, char *Global_IP_Str);
uint32_t Thread_AddUnicastAddress(const char *IPv6_Addr,uint8_t Prefix_Len, uint32_t Preferred);
uint32_t Thread_Get_Interfaces(void);
int32_t Send_Remote_device_update_to_aws(char *);
int32_t Send_data_to_router(char *Thread_Payload);
uint32_t Thread_PIR_Data_Send(void);
int32_t Get_Joiner_Confirm_Status();
void Thread_read_sensors();
int32_t get_localdevice_name(char *buf, int32_t buf_size);
void Thread_Shutdown(void);
void Socket_close(void);
boolean is_recv_thread_stopped(void);
void check_route(void);
#endif
