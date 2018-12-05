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


#ifndef _IPERF_H_

#define _IPERF_H_

#include "qcli_api.h"
#include "qapi_ssl.h"
#include "qapi_socket.h"
#include "qapi_ns_utils.h"
#include "qapi_netbuf.h"
#include "netutils.h"       /* time_struct_t */
#include "bench.h"



#undef A_OK
#define A_OK                    QAPI_OK

#undef A_ERROR
#define A_ERROR                 -1


#define IPERF_SERVER 0
#define IPERF_CLIENT 1
#define IPERF_DEFAULT_PORT 5001
#define IPERF3_DEFAULT_PORT 5201
#define IPERF_DEFAULT_RUNTIME 10
#define IPERF_MAX_PACKET_SIZE_TCP 1452 /* Max performance without splitting packets */
#define IPERF_MAX_PACKET_SIZE_UDP 1462 /* Max UDP */
#define IPERF_MAX_PACKET_SIZE_TCPV6 1424 /* Max performance without splitting packets */
#define IPERF_MAX_PACKET_SIZE_UDPV6 1452 /* Max UDP */

/* Note for this macro:
 * Some implementations represent 1KB as 1024 bytes,
 * while others represent 1KB as 1000 bytes.
 * The latter may produce slightly higher stats,
 * although the raw performance is still the same.
 *
 * Modify this macro based on your needs.
 */
#define BYTES_PER_KILO_BYTE 1000

#define  IPERF_DEFAULT_UDP_RATE  (BYTES_PER_KILO_BYTE * BYTES_PER_KILO_BYTE); // Default UDP Rate, 1 Mbit/sec

/* iperf3 control states */
#define TEST_START        1
#define TEST_RUNNING      2
#define TEST_END          4
#define PARAM_EXCHANGE    9
#define CREATE_STREAMS   10
#define SERVER_TERMINATE 11
#define CLIENT_TERMINATE 12
#define EXCHANGE_RESULTS 13
#define DISPLAY_RESULTS  14
#define IPERF_START      15
#define IPERF_DONE       16
#define ACCESS_DENIED    -1
#define SERVER_ERROR     -2

#define COOKIE_SIZE     37
#define COOKIE_CLIENT_NAME     "QCA402x client"

QCLI_Command_Status_t iperf(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
void iperf_result_print(STATS *pCxtPara, uint32_t prev, uint32_t cur);

#endif /* _IPERF_H_ */
