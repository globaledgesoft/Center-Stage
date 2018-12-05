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

#ifndef _AWS_UTIL_H_
#define _AWS_UTIL_H_

/*-------------------------------------------------------------------------
 * Include Files
 *-----------------------------------------------------------------------*/
#define MAX_LENGTH_OF_UPDATE_JSON_BUFFER   600
#define MAX_LENGTH_OF_RX_JSON_BUFFER       1024

//AWS Server Details
#define HOST_ID    "ABCD_xxx.xxxxxx.amazonaws.com"
#define THING_NAME "AB_xx"
#define AWS_CERT_LOC "cert"
#define AWS_CALIST_LOC "calist"


#define SNTPC_SVR_ADDR  "132.163.97.4"
//Macro related to Certificate
#define SIZE_CERT 3650
#define hex_to_dec_nibble(hex_nibble) ( (hex_nibble >= 'a') ? (hex_nibble-'a'+10) : ((hex_nibble >= 'A') ? (hex_nibble-'A'+10) : (hex_nibble-'0')) )

extern QCLI_Group_Handle_t qcli_onboard;

#undef IOT_INFO
#define IOT_INFO(...)           QCLI_Printf(qcli_onboard, __VA_ARGS__)
#undef IOT_ERROR
#define IOT_ERROR(...)          QCLI_Printf(qcli_onboard, __VA_ARGS__)
#undef IOT_WARN
#define IOT_WARN(...)           QCLI_Printf(qcli_onboard, __VA_ARGS__)
#undef IOT_VERBOSE
#define IOT_VERBOSE(...)        QCLI_Printf(qcli_onboard, __VA_ARGS__)

/*----------------------------------------------------
 * AWS Events
 *---------------------------------------------------*/
#define SIGNAL_AWS_START_EVENT                   (1<<0)
#define SIGNAL_AWS_STOP_EVENT                    (1<<1)

boolean is_aws_running(void);
int32_t Start_aws(void);
int32_t Stop_aws(void);
int32_t Initialize_aws(void);

typedef struct aws_json_data{
    char device_name[128];
    char sensor_name[128];
    char sensor_val[128];
}json_rx;

#endif
