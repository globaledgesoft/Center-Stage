/*
* Copyright (c) 2017 Qualcomm Technologies, Inc.
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


#ifndef _SHADOW_SAMPLE_H_

#define _SHADOW_SAMPLE_H_

#include "aws_iot_shadow_json.h"
#include "qcli_api.h"
#include "qapi_ssl.h"
#include "qapi_socket.h"
#include "qapi_ns_utils.h"
#include "qapi_netbuf.h"
#include "netutils.h"       /* time_struct_t */

#undef A_OK
#define A_OK                    QAPI_OK

#undef A_ERROR
#define A_ERROR                 -1

#define AWS_TASK_PRIORITY    20
#define AWS_TASK_STACK_SIZE  4096
#define MAX_JSON_OBJ_COUNT   10

#define AWS_MAX_DOMAIN_SIZE 128
#define AWS_MAX_THING_SIZE  128
#define AWS_MAX_FILE_NAME   128
#define AWS_MAX_CERT_NAME   128
#define AWS_MAX_STRING_SIZE 128

/* max key size 63 + 1 for null termination */
#define MAX_JSON_KEY_SIZE    64

/* Customer structure is used to take care of const char* usage for key variable */
struct jsonStruct_q {
    char *pKey; ///< JSON key
    void *pData; ///< pointer to the data (JSON value)
    JsonPrimitiveType type; ///< type of JSON
    jsonStructCallback_t cb; ///< callback to be executed on receiving the Key value pair
};

struct aws_params {
    char                      *host_name;
    char                      *thing_name;
    uint32_t                   num_objects;
    char                      *file_name;
    char                      *schema_string;
    struct qapi_fs_stat_type  *sbuf;
    char                      *client_cert;
    uint32_t                   update_timer;
};

void 
shadow_sample();

QCLI_Command_Status_t 
shadow_sample_init(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);

QCLI_Command_Status_t 
shadow_sample_destroy(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);

void 
shadow_sample_cleanup();

QCLI_Command_Status_t 
shadow_sample_set_params(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);

QCLI_Command_Status_t 
shadow_sample_run(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);

void 
shadow_sample_randomize_data();

#endif /* _SHADOW_SAMPLE_H_ */

