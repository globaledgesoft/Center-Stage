/*
 * Copyright (c) 2016-2018 Qualcomm Technologies, Inc.
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
#include "net_bench.h"
#include "qapi_netservices.h"
#include "qcli_api.h"

#define HTTPC_PRINTF(...) QCLI_Printf(__VA_ARGS__)
#define HTTPC_DEMO_MAX_NUM (4)
#define HTTPC_DEMO_DEFAULT_BODY_LEN 120
#define HTTPC_DEMO_DEFAULT_MAX_BODY_LEN 2048
#define HTTPC_DEMO_MAX_BODY_LEN 10240
#define HTTPC_DEMO_DEFAULT_MAX_HEADER_LEN   350
#define MAX_PRINTF_LENGTH 256
uint16_t httpc_demo_max_body_len = 0;
uint16_t httpc_demo_max_header_len = 0;

struct http_client_demo_s
{
	qapi_Net_HTTPc_handle_t client;
	uint32_t num;
	uint32_t total_len;
	qapi_Net_SSL_Obj_Hdl_t sslCtx;
    qapi_Net_SSL_Config_t *sslCfg;
} http_client_demo[HTTPC_DEMO_MAX_NUM];

void http_client_cb_demo(void* arg, int32_t state, void* http_resp)
{
    (void) arg;
    qapi_Net_HTTPc_Response_t* temp = (qapi_Net_HTTPc_Response_t *)http_resp;
    struct http_client_demo_s* hc = (struct http_client_demo_s *)arg;
    uint32_t* ptotal_len = NULL;
    uint32_t tmp_len;

    if (arg)
    {
        ptotal_len = &hc->total_len;
    }
    else
    {
        HTTPC_PRINTF("HTTP Client Demo arg error %d\n", state);
        return;
    }


    if (state >= 0)
    {
        int32_t resp_code = temp->resp_Code;

        if (temp->length && temp->data)
        {
            uint8_t * data = NULL;
            if ((data = malloc(MAX_PRINTF_LENGTH+1)) == NULL)
            {
                HTTPC_PRINTF("HTTP Client Demo malloc error %d\n", state);
                return;
            }
			/*print buffer is only 256B*/
            if( temp->length > MAX_PRINTF_LENGTH)
            {
                tmp_len = temp->length;
                while(tmp_len > MAX_PRINTF_LENGTH)
                {
                    memcpy(data, temp->data, MAX_PRINTF_LENGTH);
                    data[MAX_PRINTF_LENGTH] = '\0';
                    HTTPC_PRINTF("%s", data);
                    temp->data += MAX_PRINTF_LENGTH;
                    tmp_len -= MAX_PRINTF_LENGTH;

                }
                if( tmp_len > 0 )
                {
                    memcpy(data, temp->data, tmp_len);
                    data[tmp_len] = '\0';
                    HTTPC_PRINTF("%s", data);

                }
            }
            else
            {
                memcpy(data, temp->data, temp->length);
                data[temp->length] = '\0';
                HTTPC_PRINTF("%s", data);
            }
            *ptotal_len += temp->length;
            free(data);
        }

        if (state == 0 && *ptotal_len)
        {
            HTTPC_PRINTF("=========> http client Received: total size %d, Resp_code %d\n", *ptotal_len, resp_code);
            *ptotal_len = 0; // Finished
        }
    }
    else
    {
        if(QAPI_NET_HTTPC_RX_ERROR_SERVER_CLOSED == state)
            HTTPC_PRINTF("HTTP Client server closed on client[%d].\n", hc->num);
        else
            HTTPC_PRINTF("HTTP Client Receive error: %d\nPlease input 'httpc disconnect %d'\n", state, hc->num);
        *ptotal_len = 0;
    }
}
char * httpc_malloc_body_demo(uint32_t len)
{
    char * body = NULL;
    int32_t i;

    body = malloc(len+1);
    if (body) {
        for (i=0; i<len; i++) {
            *(body + i) = 'A' + i % 26;
        }
        *(body + len) = '\0';
     } else {
         HTTPC_PRINTF("malloc failed\n");
     }

     return body;
}

QCLI_Command_Status_t httpc_command_handler(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int error = QAPI_OK;
    struct http_client_demo_s* arg = NULL;
    char* command = Parameter_List[0].String_Value;
    uint32_t num = 0;
    qapi_Net_HTTPc_Method_e req_cmd = 0;

    if(Parameter_Count < 1)
    {
usage:
        HTTPC_PRINTF("\nhttpc start\n");
        HTTPC_PRINTF("httpc stop\n");
        HTTPC_PRINTF("httpc connect <server> <port> <timeout>\n");
        HTTPC_PRINTF("httpc disconnect <client_num>\n");
        HTTPC_PRINTF("httpc get <client_num> <url>\n");
        HTTPC_PRINTF("httpc put <client_num> <url>\n");
        HTTPC_PRINTF("httpc post <client_num> <url>\n");
        HTTPC_PRINTF("httpc patch <client_num> <url>\n");
        HTTPC_PRINTF("httpc setbody <client_num> <len>\n");
        HTTPC_PRINTF("httpc addheaderfield <client_num> <hdr_name> <hdr_value>\n");
        HTTPC_PRINTF("httpc clearheader <client_num>\n");
        HTTPC_PRINTF("httpc setparam <client_num> <key> <value>\n");
	    HTTPC_PRINTF("httpc config <httpc_demo_max_body_len> <httpc_demo_max_header_len>\n");
        sslconfig_help("httpc sslconfig <client_num>");
        HTTPC_PRINTF("httpc cbaddhead <client_num> <enable|diable>\n");
        return QCLI_STATUS_ERROR_E;
    }
    if (strcmp(command, "start") == 0){
        error = qapi_Net_HTTPc_Start();
        if (error)
        {
            HTTPC_PRINTF("HTTP Client start failed %d\r\n",error);
            return QCLI_STATUS_ERROR_E;
        }
        return QCLI_STATUS_SUCCESS_E;
    }
    if (strcmp(command, "stop") == 0){
        uint32_t i;
        error = qapi_Net_HTTPc_Stop();
        for (i = 0; i < HTTPC_DEMO_MAX_NUM; i++)
        {
            arg = &http_client_demo[i];

            if (arg->sslCtx != QAPI_NET_SSL_INVALID_HANDLE)
                qapi_Net_SSL_Obj_Free(arg->sslCtx);

            if (arg->sslCfg)
            {
                free(arg->sslCfg);
            }
            memset(arg, 0, sizeof(struct http_client_demo_s));
        }
        if (error)
        {
            HTTPC_PRINTF("HTTP Client stop failed %d\r\n",error);
            return QCLI_STATUS_ERROR_E;
        }
        return QCLI_STATUS_SUCCESS_E;
    }
    if (strcmp(command, "config") == 0)
    {
        if(Parameter_Count < 3){
            goto usage;
        }
        if(Parameter_List[1].Integer_Value%4!=0)
            goto usage;

        httpc_demo_max_body_len = (Parameter_List[1].Integer_Value > HTTPC_DEMO_MAX_BODY_LEN)?HTTPC_DEMO_MAX_BODY_LEN:Parameter_List[1].Integer_Value;
        httpc_demo_max_header_len = (Parameter_List[2].Integer_Value > HTTPC_DEMO_DEFAULT_MAX_HEADER_LEN)?HTTPC_DEMO_DEFAULT_MAX_HEADER_LEN:Parameter_List[2].Integer_Value;
        HTTPC_PRINTF("Max body len:%d max header len:%d\r\n", httpc_demo_max_body_len, httpc_demo_max_header_len);
        return QCLI_STATUS_SUCCESS_E;
    }
    if (strcmp(command, "connect") == 0)
    {
        uint32_t port = 0;
        uint32_t server_offset = 0;
        uint32_t timeout = 0;
        uint32_t i;
        uint16_t httpc_max_body_length = 0;
        uint16_t httpc_max_header_length = 0;

        if(Parameter_Count < 4){
		goto usage;
        }

        if(strlen((char*)Parameter_List[1].String_Value) >= 64)
        {
            HTTPC_PRINTF("Maximum 64 bytes supported as Connect URL\n");
            return QCLI_STATUS_ERROR_E;
        }

        for (i = 0; i < HTTPC_DEMO_MAX_NUM; i++)
        {
            if (http_client_demo[i].client == NULL)
            {
                arg = &http_client_demo[i];
                arg->num = i + 1;
                arg->total_len = 0;
                break;
            }
        }

        if (!arg)
        {
            HTTPC_PRINTF("No More avalible HTTP CLIENT\n");
            return QCLI_STATUS_ERROR_E;
        }

        port = Parameter_List[2].Integer_Value;

        if (port == 0)
        {
            port = 80;
        }

        timeout = Parameter_List[3].Integer_Value;

        if(strncmp(Parameter_List[1].String_Value, "https://", 8) == 0)
        {
            server_offset = 8;
            arg->sslCtx = qapi_Net_SSL_Obj_New(QAPI_NET_SSL_CLIENT_E);
            if (arg->sslCtx == QAPI_NET_SSL_INVALID_HANDLE)
            {
                HTTPC_PRINTF("ERROR: Unable to create SSL context\n");
                if (arg->sslCfg)
                {
                    free(arg->sslCfg);
                }
                memset(arg, 0, sizeof(struct http_client_demo_s));
                return QCLI_STATUS_ERROR_E;
            }
        }
        else if(strncmp(Parameter_List[1].String_Value, "http://", 7) == 0)
        {
            server_offset = 7;
        }
        httpc_max_body_length = (httpc_demo_max_body_len)?httpc_demo_max_body_len:HTTPC_DEMO_DEFAULT_MAX_BODY_LEN;
        httpc_max_header_length = (httpc_demo_max_header_len)?httpc_demo_max_header_len:HTTPC_DEMO_DEFAULT_MAX_HEADER_LEN;

        arg->client = qapi_Net_HTTPc_New_sess(timeout, arg->sslCtx, http_client_cb_demo, (void *)arg, httpc_max_body_length, httpc_max_header_length);
        if (arg->client == NULL)
        {
            HTTPC_PRINTF("There is no available http client session\r\n");
            if (arg->sslCtx != QAPI_NET_SSL_INVALID_HANDLE)
                qapi_Net_SSL_Obj_Free(arg->sslCtx);

            if (arg->sslCfg)
            {
                free(arg->sslCfg);
            }

            memset(arg, 0, sizeof(struct http_client_demo_s));
            return QCLI_STATUS_ERROR_E;
        }

        //HTTPC_PRINTF("client=0x%p sslCtx=%x sslCfg=0x%p\n", arg->client, arg->sslCtx, arg->sslCfg);

        error = QAPI_OK;
        if ((arg->sslCtx != 0 && arg->sslCtx != QAPI_NET_SSL_INVALID_HANDLE) && /* SSL object is created */
            arg->sslCfg != NULL)                                                /* SSL parameters are parsed */
        {
            error = qapi_Net_HTTPc_Configure_SSL(arg->client, arg->sslCfg);
        }

        if (error == QAPI_OK)
        {
            error = qapi_Net_HTTPc_Connect(arg->client, (const char *)(Parameter_List[1].String_Value + server_offset), port);
        }

        if (error)
        {
            HTTPC_PRINTF("http client connect failed %d\n",error);
            qapi_Net_HTTPc_Free_sess(arg->client);
            if (arg->sslCtx != QAPI_NET_SSL_INVALID_HANDLE)
                qapi_Net_SSL_Obj_Free(arg->sslCtx);

            if (arg->sslCfg)
            {
                free(arg->sslCfg);
            }

            memset(arg, 0, sizeof(struct http_client_demo_s));
            return QCLI_STATUS_ERROR_E;
        }

        HTTPC_PRINTF("http client connect success <client num> = %d\n", arg->num);
        return QCLI_STATUS_SUCCESS_E;
    }

    /* For commands which require <client_num> */
    if (Parameter_Count < 2)
    {
        HTTPC_PRINTF("What is client_num?\n");
        return QCLI_STATUS_SUCCESS_E;
    }
    num = Parameter_List[1].Integer_Value;

    /*       [0]        [1]         [2]      [3]    [4]  [5] [6]   [7]
     * httpc sslconfig <client_num> protocol TLS1.2 time 1   alert 0
     * (client_num: 1..HTTPC_DEMO_MAX_NUM)
     */
    if (strncmp(command, "sslconfig", 3) == 0)
    {
        QCLI_Command_Status_t status;

        if (Parameter_Count < 3)
        {
            HTTPC_PRINTF("What are SSL parameters?\n");
            return QCLI_STATUS_SUCCESS_E;
        }

        if (num > 0 && num <= HTTPC_DEMO_MAX_NUM)
        {
            arg = &http_client_demo[num - 1];
            if (arg->sslCfg == NULL)
            {
                arg->sslCfg = malloc(sizeof(qapi_Net_SSL_Config_t));
                if (arg->sslCfg == NULL)
                {
                    HTTPC_PRINTF("Allocation failure\n");
                    return QCLI_STATUS_ERROR_E;
                }
                memset(arg->sslCfg, 0, sizeof(qapi_Net_SSL_Config_t));
            }

            /* Parse SSL config parameters from command line */
            status = ssl_parse_config_parameters(
                    Parameter_Count-2,
                    &Parameter_List[2],
                    arg->sslCfg,
                    arg->sslCtx,
                    false);
            if (status == QCLI_STATUS_USAGE_E)
            {
                goto usage;
            }

            /* HTTP always uses TLS. If configured for DTLS, silently ignore */
            if (arg->sslCfg->protocol <= QAPI_NET_SSL_DTLS_E ||
                    arg->sslCfg->protocol > QAPI_NET_SSL_PROTOCOL_TLS_1_2)
            {
                arg->sslCfg->protocol = 0;
            }
        }
        else
        {
            status = QCLI_STATUS_ERROR_E;
            HTTPC_PRINTF("<client num> error\n");
        }

        return status;
    }

    /* For other commands which need <client_num> argument */
    if (num > 0 && num <= HTTPC_DEMO_MAX_NUM)
    {
        arg = &http_client_demo[num - 1];

        if (arg->client == NULL || arg->num != num)
        {
            arg = NULL;
        }
    }
    HTTPC_PRINTF("<client num> %d\n",num);

    if (strcmp(command, "disconnect") == 0)
    {
        if (!arg)
        {
            HTTPC_PRINTF("<client num> error\n");
            return QCLI_STATUS_ERROR_E;
        }
        qapi_Net_HTTPc_Free_sess(arg->client);
        if (arg->sslCtx != QAPI_NET_SSL_INVALID_HANDLE)
            qapi_Net_SSL_Obj_Free(arg->sslCtx);

        if (arg->sslCfg)
        {
            free(arg->sslCfg);
        }

        memset(arg, 0, sizeof(struct http_client_demo_s));

        return QCLI_STATUS_SUCCESS_E;
    }

    if (strcmp(command, "get") == 0)
    {
        req_cmd = QAPI_NET_HTTP_CLIENT_GET_E;
    }
    else if (strcmp(command, "put") == 0)
    {
        req_cmd = QAPI_NET_HTTP_CLIENT_PUT_E;
    }
    else if (strcmp(command, "post") == 0)
    {
        req_cmd = QAPI_NET_HTTP_CLIENT_POST_E;
    }
    else if (strcmp(command, "patch") == 0)
    {
        req_cmd = QAPI_NET_HTTP_CLIENT_PATCH_E;
    }

    if (req_cmd)
    {
        HTTPC_PRINTF("req_cmd %d\n",req_cmd);
        if (Parameter_Count < 3) {
            HTTPC_PRINTF("Missing <url>\n");
            return QCLI_STATUS_ERROR_E;
        }

        if (!arg)
        {
            HTTPC_PRINTF("<client num> error\n");
            return QCLI_STATUS_ERROR_E;
        }


        error = qapi_Net_HTTPc_Request(arg->client, req_cmd, (const char *)Parameter_List[2].String_Value);

    }
    else if (strcmp(command, "setbody") == 0)
    {
        char *body = NULL;
        uint32_t len = HTTPC_DEMO_DEFAULT_BODY_LEN;

        if (!arg)
        {
            HTTPC_PRINTF("<client num> error\n");
            return QCLI_STATUS_ERROR_E;
        }

        if (Parameter_Count > 2) {
            len = Parameter_List[2].Integer_Value;
        }

        if (len > HTTPC_DEMO_MAX_BODY_LEN)
            len = HTTPC_DEMO_MAX_BODY_LEN;

		body = httpc_malloc_body_demo(len);
        if (!body)
		    return QCLI_STATUS_ERROR_E;

        HTTPC_PRINTF("len = %d : %d\n%s\n", len, strlen(body), body);

		error = qapi_Net_HTTPc_Set_Body(arg->client, (const char*)body, strlen(body));

        free(body);
    }
    else if (strcmp(command, "addheaderfield") == 0)
    {
        if (!arg)
        {
            HTTPC_PRINTF("<client num> error\n");
            return QCLI_STATUS_ERROR_E;
        }

        if (Parameter_Count < 4)
        {
            HTTPC_PRINTF("Missing parameters\n");
            return QCLI_STATUS_ERROR_E;
        }
        error = qapi_Net_HTTPc_Add_Header_Field(arg->client, Parameter_List[2].String_Value, Parameter_List[3].String_Value);
    }
    else if (strcmp(command, "clearheader") == 0)
    {
        if (!arg)
        {
            HTTPC_PRINTF("<client num> error\n");
            return QCLI_STATUS_ERROR_E;
        }

        error = qapi_Net_HTTPc_Clear_Header(arg->client);
    }
    else if(strcmp(command, "setparam") == 0)
    {
        if (!arg)
        {
            HTTPC_PRINTF("<client num> error\n");
            return QCLI_STATUS_ERROR_E;
        }

        if (Parameter_Count < 4)
        {
            HTTPC_PRINTF("Missing parameters\n");
            return QCLI_STATUS_ERROR_E;
        }
        error = qapi_Net_HTTPc_Set_Param(arg->client, Parameter_List[2].String_Value, Parameter_List[3].String_Value);
    }
    else if(strcmp(command, "cbaddhead") == 0)
    {
        if (!arg)
        {
            HTTPC_PRINTF("<client num> error\n");
            return QCLI_STATUS_ERROR_E;
        }


        if ((strncmp(Parameter_List[2].String_Value, "enable", 3) == 0) ||
            (strncmp(Parameter_List[2].String_Value, "disable", 3) == 0))
            error = qapi_Net_HTTPc_CB_Enable_Adding_Header(arg->client, (strncmp(Parameter_List[2].String_Value, "enable", 3) == 0)? 1: 0);
    }
    else
    {
        HTTPC_PRINTF("Unknown http client command.\n");
        return QCLI_STATUS_ERROR_E;
    }

    if (error)
    {
        HTTPC_PRINTF("http client %s failed on error: %d\n", command, error);
    }

    return QCLI_STATUS_SUCCESS_E;
}



