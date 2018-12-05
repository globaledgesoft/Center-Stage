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


#ifndef _IPERF_H_

#define _IPERF_H_

#include "qcli_api.h"
#include "qapi_ssl.h"
#include "qapi_socket.h"
#include "qapi_ns_utils.h"
#include "qapi_netbuf.h"
#include "net_utils.h"       /* time_struct_t */
#include "net_bench.h"



#undef A_OK
#define A_OK                    QAPI_OK

#undef A_ERROR
#define A_ERROR                 -1

#define  SO_SNDBUF      0x1001      /* send buffer size */
#define  SO_RCVBUF      0x1002      /* receive buffer size */


#define IPERF_SERVER 0
#define IPERF_CLIENT 1
#define IPERF_DEFAULT_PORT 5001
#define IPERF_DEFAULT_RUNTIME 10
#define IPERF_MAX_PACKET_SIZE_TCP 1452 /* Max performance without splitting packets */
#define IPERF_MAX_PACKET_SIZE_UDP 1462 /* Max UDP */
#define IPERF_MAX_PACKET_SIZE_TCPV6 1424 /* Max performance without splitting packets */
#define IPERF_MAX_PACKET_SIZE_UDPV6 1452 /* Max UDP */
#define  IPERF_DEFAULT_UDPRate  (1024 * 1024); // Default UDP Rate, 1 Mbit/sec

#define IPERF_kKilo_to_Unit  1024;
#define IPERF_kMega_to_Unit  (1024 * 1024);
#define IPERF_kGiga_to_Unit  (1024 * 1024 * 1024);

#define IPERF_kkilo_to_Unit  1000;
#define IPERF_kmega_to_Unit (1000 * 1000);
#define IPERF_kgiga_to_Unit (1000 * 1000 * 1000);

QCLI_Command_Status_t iperf(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
void iperf_result_print(STATS *pCxtPara, uint32_t prev, uint32_t cur);

#endif /* _IPERF_H_ */
