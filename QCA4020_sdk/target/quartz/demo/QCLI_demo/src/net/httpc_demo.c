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
#include "bench.h"

#ifdef CONFIG_NET_HTTPC_DEMO

//#define HTTPC_DEMO_DEBUG
#ifdef HTTPC_DEMO_DEBUG
#pragma GCC push_options
#pragma GCC optimize ("O0")
#endif

extern QCLI_Group_Handle_t qcli_net_handle; /* Handle for Net Command Group. */

#define HTTPC_PRINTF(...)                   QCLI_Printf(qcli_net_handle, __VA_ARGS__)
#define HTTPC_DEMO_MAX_NUM                  (4)
#define HTTPC_DEMO_DEFAULT_MAX_BODY_LEN     2048
#define HTTPC_DEMO_MAX_BODY_LEN             10240
#define HTTPC_DEMO_DEFAULT_MAX_HEADER_LEN   350
#define MAX_PRINTF_LENGTH                   256

#define REQUEST_TIMEOUT_MS                  5000    /* 5 sec */
#define BODY_BUFFER_SIZE                    300
#define HEADER_BUFFER_SIZE                  200
#define RX_BUFFER_SIZE                      512
#define MAX_URL_LENGTH                      128
#define MAX_HOST_LENGTH                     64
#define HEXDUMP(inbuf, inlen, ascii, addr)  app_hexdump(inbuf, inlen, ascii, addr)
static uint8_t hd = 0;

uint16_t httpc_demo_max_body_len = 0;
uint16_t httpc_demo_max_header_len = 0;

struct http_client_demo_s
{
	qapi_Net_HTTPc_handle_t client;
	uint32_t num;
	uint32_t total_len;
#ifdef CONFIG_NET_SSL_DEMO
	qapi_Net_SSL_Obj_Hdl_t sslCtx;
    qapi_Net_SSL_Config_t *sslCfg;
#endif
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

    if (state >= QAPI_NET_HTTPC_RX_FINISHED)
    {
        int32_t resp_code = temp->resp_Code;

        if (temp->length && temp->data)
        {
            if (hd)
            {
                HTTPC_PRINTF("@@ Received %d bytes:\n", temp->length);
                HEXDUMP((char *)temp->data, temp->length, true, false);
            }
            else
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
                free(data);
            }
            *ptotal_len += temp->length;
        }

        if (state == QAPI_NET_HTTPC_RX_FINISHED)
        {
            HTTPC_PRINTF("=========> http client Received: total size %d, Resp_code %d\n", *ptotal_len, resp_code);
            *ptotal_len = 0; // Finished
        }
        else
        if (state == QAPI_NET_HTTPC_RX_TUNNEL_ESTABLISHED)
        {
            HTTPC_PRINTF("#### TUNNEL ESTABLISHED: received %d bytes ####\n", *ptotal_len);
            *ptotal_len = 0;
        }
        else
        if (state == QAPI_NET_HTTPC_RX_DATA_FROM_TUNNEL)
        {
            HTTPC_PRINTF("#### Received %d bytes from TUNNEL ####\n", *ptotal_len);
            *ptotal_len = 0;
        }
        else
        if (state == QAPI_NET_HTTPC_RX_TUNNEL_CLOSED)
        {
            HTTPC_PRINTF("!!!! TUNNEL CLOSED !!!!\n");
            *ptotal_len = 0;
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
        HTTPC_PRINTF("httpc {start | stop}\n");
        HTTPC_PRINTF("httpc new [-t <timeout_ms> -b <body_buffer_size> -h <header_buffer_size> -r <rx_buffer_size> -s -c <calist>]\n");
        HTTPC_PRINTF("httpc destroy <client_num>\n");
        HTTPC_PRINTF("httpc conn <client_num> <origin_server or proxy> [<port>]\n");
        HTTPC_PRINTF("httpc disconn <client_num>\n");
        HTTPC_PRINTF("httpc {get | head | post | put | delete | patch} <client_num> [<url>]\n");
        HTTPC_PRINTF("httpc tunnel <client_num> <origin_server> <port> [-s -c <calist>]\n");
        HTTPC_PRINTF("httpc sendraw <client_num> [<data>]\n");
        HTTPC_PRINTF("httpc setbody <client_num> [<len>]\n");
        HTTPC_PRINTF("httpc addheaderfield <client_num> <hdr_name> <hdr_value>\n");
        HTTPC_PRINTF("httpc clearheader <client_num>\n");
        HTTPC_PRINTF("httpc setparam <client_num> <key> <value>\n");
        HTTPC_PRINTF("httpc cbaddhead <client_num> {enable|disable}\n");
#ifdef CONFIG_NET_SSL_DEMO
	    sslconfig_help("httpc sslconfig <client_num>");
#endif
        HTTPC_PRINTF("The following commands are deprecated:\n");
        HTTPC_PRINTF(" httpc connect <origin_server or proxy> <port> <timeout in ms>\n");
        HTTPC_PRINTF(" httpc disconnect <client_num>\n");
	    HTTPC_PRINTF(" httpc config <httpc_demo_max_body_len> <httpc_demo_max_header_len>\n");
        HTTPC_PRINTF("\nExamples:\n");
        HTTPC_PRINTF(" httpc start\n");
        HTTPC_PRINTF(" httpc new         -or-\n");
        HTTPC_PRINTF(" httpc new -s -b 300 -h 200 -r 1024 -t 10000\n");
#ifdef CONFIG_NET_SSL_DEMO
        HTTPC_PRINTF(" httpc sslconfig 1 protocol TLS1.2 cipher QAPI_NET_TLS_RSA_WITH_AES_256_GCM_SHA384\n");
#endif
        HTTPC_PRINTF(" httpc conn 1 www.example.com       -or-\n");
        HTTPC_PRINTF(" httpc conn 1 192.168.2.30 8080\n");
        HTTPC_PRINTF(" httpc get 1\n");
        HTTPC_PRINTF(" httpc get 1 /cgi/my_cgi_script.pl\n");
        HTTPC_PRINTF(" httpc tunnel 1 apis.google.com 443\n");
        HTTPC_PRINTF(" httpc sendraw 1 \"Hello, World!\"\n");
        HTTPC_PRINTF(" httpc disconn 1\n");
        HTTPC_PRINTF(" httpc destroy 1\n");
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

#ifdef CONFIG_NET_SSL_DEMO
            if (arg->sslCtx != QAPI_NET_SSL_INVALID_HANDLE)
                qapi_Net_SSL_Obj_Free(arg->sslCtx);

            if (arg->sslCfg)
            {
                ssl_free_config_parameters(arg->sslCfg);
                free(arg->sslCfg);
            }
#endif
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
    if (strcmp(command, "hd") == 0)
    {
        hd ^= 1;
        return QCLI_STATUS_SUCCESS_E;
    }

    /*       [0] [1] [2]      [3]  [4]      [5]  [6]         [7]  [8]            [9]
     * httpc new -t <timeout> -b <body_len> -h  <header_len> -r  <rxbuffer_size> -s
     */
    if (strcmp(command, "new") == 0)
    {
        int i;
        int timeout_ms = REQUEST_TIMEOUT_MS;
        int body_size = BODY_BUFFER_SIZE;
        int header_size = HEADER_BUFFER_SIZE;
        int rxbuffer_size = RX_BUFFER_SIZE;
        qbool_t secure_session = false;
        char *calist = NULL;

        for (i = 1; i < Parameter_Count; i++)
        {
            if (Parameter_List[i].String_Value[0] == '-')
            {
                switch (Parameter_List[i].String_Value[1])
                {
                    case 's':   /* -s */
                        secure_session = true;
                        break;

                    case 'c':   /* -c ca_list */
                        i++;
                        calist = Parameter_List[i].String_Value;
                        break;

                    case 't':   /* -t 10000 */
                        i++;
                        if (!Parameter_List[i].Integer_Is_Valid)
                        {
                            HTTPC_PRINTF("Invalid timeout: %s\n", Parameter_List[i].String_Value);
                            return QCLI_STATUS_ERROR_E;
                        }
                        timeout_ms = Parameter_List[i].Integer_Value;
                        break;

                    case 'b':   /* -b 300 */
                        i++;
                        if (!Parameter_List[i].Integer_Is_Valid)
                        {
                            HTTPC_PRINTF("Invalid body size: %s\n", Parameter_List[i].String_Value);
                            return QCLI_STATUS_ERROR_E;
                        }
                        body_size = Parameter_List[i].Integer_Value;
                        break;

                    case 'h':   /* -h 200 */
                        i++;
                        if (!Parameter_List[i].Integer_Is_Valid)
                        {
                            HTTPC_PRINTF("Invalid header size: %s\n", Parameter_List[i].String_Value);
                            return QCLI_STATUS_ERROR_E;
                        }
                        header_size = Parameter_List[i].Integer_Value;
                        break;

                    case 'r':   /* -r 512 */
                        i++;
                        if (!Parameter_List[i].Integer_Is_Valid)
                        {
                            HTTPC_PRINTF("Invalid rxbuffer size: %s\n", Parameter_List[i].String_Value);
                            return QCLI_STATUS_ERROR_E;
                        }
                        rxbuffer_size = (Parameter_List[i].Integer_Value < RX_BUFFER_SIZE) ?
                                        RX_BUFFER_SIZE : Parameter_List[i].Integer_Value;
                        break;

                    default:
                        HTTPC_PRINTF("Unknown option: %s\n", Parameter_List[i].String_Value);
                        return QCLI_STATUS_ERROR_E;
                }
            }
            else
            {
                HTTPC_PRINTF("Unknown option: %s\n", Parameter_List[i].String_Value);
                return QCLI_STATUS_ERROR_E;
            }

            if (i == Parameter_Count)
            {
                QCLI_Printf(qcli_net_handle, "What is value of %s?\n", Parameter_List[i-1].String_Value);
                return QCLI_STATUS_ERROR_E;
            }
        } /* for */

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

        if (arg == NULL)
        {
            HTTPC_PRINTF("Cannot create more than %d clients\n", HTTPC_DEMO_MAX_NUM);
            return QCLI_STATUS_ERROR_E;
        }

#ifdef CONFIG_NET_SSL_DEMO
        if (secure_session)
        {
            qapi_Net_SSL_Obj_Hdl_t sslCtx;

            sslCtx = qapi_Net_SSL_Obj_New(QAPI_NET_SSL_CLIENT_E);
            if (sslCtx == QAPI_NET_SSL_INVALID_HANDLE)
            {
                HTTPC_PRINTF("Unable to create SSL context\n");
                return QCLI_STATUS_ERROR_E;
            }

            /* Read calist from filesystem and add it to the SSL context. */
            if (calist != NULL && calist[0] != '\0')
            {
                if (qapi_Net_SSL_Cert_Load(sslCtx, QAPI_NET_SSL_CA_LIST_E, calist) != QAPI_OK)
                {
                    HTTPC_PRINTF("Unable to load CAlist\n");
                    qapi_Net_SSL_Obj_Free(sslCtx);
                    return QCLI_STATUS_ERROR_E;
                }
            }
            arg->sslCtx = sslCtx;
        }
#endif

        arg->client = qapi_Net_HTTPc_New_sess2(timeout_ms,
                                               arg->sslCtx,
                                               http_client_cb_demo,
                                               (void *)arg,
                                               body_size,
                                               header_size,
                                               rxbuffer_size);

        if (arg->client == NULL)
        {
            HTTPC_PRINTF("Failed to create a client instance\n");

#ifdef CONFIG_NET_SSL_DEMO
            if (arg->sslCtx != QAPI_NET_SSL_INVALID_HANDLE)
            {
                qapi_Net_SSL_Obj_Free(arg->sslCtx);
            }

            if (arg->sslCfg)
            {
                ssl_free_config_parameters(arg->sslCfg);
                free(arg->sslCfg);
            }
#endif

            memset(arg, 0, sizeof(*arg));
            return QCLI_STATUS_ERROR_E;
        }

        HTTPC_PRINTF("HTTP client created. <client num> = %d\n", arg->num);
        HTTPC_PRINTF("%ssecure  rxbuf:%d  bodybuf:%d  headerbuf:%d  timeout:%dms",
                secure_session ? "" : "in", rxbuffer_size, body_size, header_size, timeout_ms);

        return QCLI_STATUS_SUCCESS_E;
    }

    /*       [0]       [1]              [2]      [3]
     * httpc connect <server or proxy>  <port>  <timeout_ms>
     */
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

        /* httpc connect https://www.example.com 443 36000 */ 
        if(strncmp(Parameter_List[1].String_Value, "https://", 8) == 0)
        {
#ifdef CONFIG_NET_SSL_DEMO
            server_offset = 8;
            arg->sslCtx = qapi_Net_SSL_Obj_New(QAPI_NET_SSL_CLIENT_E);
            if (arg->sslCtx == QAPI_NET_SSL_INVALID_HANDLE)
            {
                HTTPC_PRINTF("ERROR: Unable to create SSL context\n");
                if (arg->sslCfg)
                {
                    ssl_free_config_parameters(arg->sslCfg);
                    free(arg->sslCfg);
                }
                memset(arg, 0, sizeof(struct http_client_demo_s));
                return QCLI_STATUS_ERROR_E;
            }
#else
            HTTPC_PRINTF("ERROR: SSL not supported\n");
            return QCLI_STATUS_ERROR_E;
#endif
        }

        /* httpc connect http://www.example.com 80 36000 */ 
        else if(strncmp(Parameter_List[1].String_Value, "http://", 7) == 0)
        {
            server_offset = 7;
        }

        /* httpc connect www.example.com 80 36000 */ 
        else
        {
            server_offset = 0;
        }
        httpc_max_body_length = (httpc_demo_max_body_len)?httpc_demo_max_body_len:HTTPC_DEMO_DEFAULT_MAX_BODY_LEN;
        httpc_max_header_length = (httpc_demo_max_header_len)?httpc_demo_max_header_len:HTTPC_DEMO_DEFAULT_MAX_HEADER_LEN;

        arg->client = qapi_Net_HTTPc_New_sess(timeout,
#ifdef CONFIG_NET_SSL_DEMO
                arg->sslCtx,
#else
                0,
#endif
                http_client_cb_demo, (void *)arg, httpc_max_body_length, httpc_max_header_length);

        if (arg->client == NULL)
        {
            HTTPC_PRINTF("There is no available http client session\r\n");
#ifdef CONFIG_NET_SSL_DEMO
            if (arg->sslCtx != QAPI_NET_SSL_INVALID_HANDLE)
                qapi_Net_SSL_Obj_Free(arg->sslCtx);

            if (arg->sslCfg)
            {
                ssl_free_config_parameters(arg->sslCfg);
                free(arg->sslCfg);
            }
#endif
            memset(arg, 0, sizeof(struct http_client_demo_s));
            return QCLI_STATUS_ERROR_E;
        }

        //HTTPC_PRINTF("client=0x%p sslCtx=%x sslCfg=0x%p\n", arg->client, arg->sslCtx, arg->sslCfg);

        error = QAPI_OK;
#ifdef CONFIG_NET_SSL_DEMO
        if ((arg->sslCtx != 0 && arg->sslCtx != QAPI_NET_SSL_INVALID_HANDLE) && /* SSL object is created */
            arg->sslCfg != NULL)                                                /* SSL parameters are parsed */
        {
            error = qapi_Net_HTTPc_Configure_SSL(arg->client, arg->sslCfg);
        }
#endif
        if (error == QAPI_OK)
        {
            error = qapi_Net_HTTPc_Connect(arg->client, (const char *)(Parameter_List[1].String_Value + server_offset), port);
        }

        if (error)
        {
            HTTPC_PRINTF("http client connect failed %d\n",error);
            qapi_Net_HTTPc_Free_sess(arg->client);
#ifdef CONFIG_NET_SSL_DEMO
            if (arg->sslCtx != QAPI_NET_SSL_INVALID_HANDLE)
                qapi_Net_SSL_Obj_Free(arg->sslCtx);

            if (arg->sslCfg)
            {
                ssl_free_config_parameters(arg->sslCfg);
                free(arg->sslCfg);
            }
#endif
            memset(arg, 0, sizeof(struct http_client_demo_s));
            return QCLI_STATUS_ERROR_E;
        }

        HTTPC_PRINTF("http client connect success <client num> = %d\n", arg->num);
        return QCLI_STATUS_SUCCESS_E;
    }

    /*************************************************************************
     * For commands which require <client_num>
     * <client_num> should be in between 1 and HTTPC_DEMO_MAX_NUM (inclusive)
     *************************************************************************/
    if (Parameter_Count < 2)
    {
        HTTPC_PRINTF("What is client_num?\n");
        return QCLI_STATUS_SUCCESS_E;
    }

    num = Parameter_List[1].Integer_Value;
    if (num <= 0 || num > HTTPC_DEMO_MAX_NUM)
    {
        HTTPC_PRINTF("Client_num must be in between 1 and %d\n", HTTPC_DEMO_MAX_NUM);
        return QCLI_STATUS_ERROR_E;
    }

    arg = &http_client_demo[num - 1];
    if (arg->client == NULL || arg->num != num)
    {
        HTTPC_PRINTF("Client %d does not exist\n", num);
        return QCLI_STATUS_ERROR_E;
    }

    /*       [0]  [1]           [2]               [3]
     * httpc conn <client num> <server or proxy> [<port>]
     */
    if (strcmp(command, "conn") == 0)
    {
        char *host;
        uint16_t port = 0;
        
        if (Parameter_Count < 3)
        {
            goto usage;
        }
        
        host = Parameter_List[2].String_Value;
        if (strlen(host) > MAX_HOST_LENGTH)
        {
            HTTPC_PRINTF("Hostname too long. Cannot be over %d\n", MAX_HOST_LENGTH);
            return QCLI_STATUS_ERROR_E;
        }

        if (Parameter_Count >= 4)
        {
            if (!Parameter_List[3].Integer_Is_Valid)
            {
                HTTPC_PRINTF("Invalid port\n");
                return QCLI_STATUS_ERROR_E;
            }
            port = Parameter_List[3].Integer_Value;
        }

        if (port == 0)
        {
            port = 80;

#ifdef CONFIG_NET_SSL_DEMO
            if (arg->sslCtx != QAPI_NET_SSL_INVALID_HANDLE)
            {
                port = 443;
            }
#endif
        }

        error = QAPI_OK;

#ifdef CONFIG_NET_SSL_DEMO
        if (arg->sslCtx != QAPI_NET_SSL_INVALID_HANDLE && /* SSL object is created */
            arg->sslCfg != NULL)                          /* SSL parameters are parsed */
        {
            error = qapi_Net_HTTPc_Configure_SSL(arg->client, arg->sslCfg);
        }
#endif

        if (error == QAPI_OK)
        {
            error = qapi_Net_HTTPc_Connect(arg->client, host, port);
        }

        if (error)
        {
            HTTPC_PRINTF("%s failed %d\n", command, error);

            qapi_Net_HTTPc_Free_sess(arg->client);

#ifdef CONFIG_NET_SSL_DEMO
            if (arg->sslCtx != QAPI_NET_SSL_INVALID_HANDLE)
            {
                qapi_Net_SSL_Obj_Free(arg->sslCtx);
            }

            if (arg->sslCfg)
            {
                ssl_free_config_parameters(arg->sslCfg);
                free(arg->sslCfg);
            }
#endif

            memset(arg, 0, sizeof(*arg));
            return QCLI_STATUS_ERROR_E;
        }

        HTTPC_PRINTF("%s to %s:%d succeeded\n", command, host, port);
        return QCLI_STATUS_SUCCESS_E;
    }

#ifdef CONFIG_NET_SSL_DEMO
    /*       [0]        [1]         [2]      [3]    [4]  [5] [6]   [7]
     * httpc sslconfig <client_num> protocol TLS1.2 time 1   alert 0
     */
    if (strncmp(command, "sslconfig", 3) == 0)
    {
        QCLI_Command_Status_t status;

        if (Parameter_Count < 3)
        {
            HTTPC_PRINTF("What are SSL parameters?\n");
            return QCLI_STATUS_SUCCESS_E;
        }

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
        else
        {
            /* free previous ssl parameters */
            ssl_free_config_parameters(arg->sslCfg);
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

        return status;
    }
#endif

    if (strcmp(command, "disconnect") == 0 ||   /* deprecated */
        strcmp(command, "destroy") == 0)
    {
        qapi_Net_HTTPc_Free_sess(arg->client);

#ifdef CONFIG_NET_SSL_DEMO
        if (arg->sslCtx != QAPI_NET_SSL_INVALID_HANDLE)
            qapi_Net_SSL_Obj_Free(arg->sslCtx);

        if (arg->sslCfg)
        {
            ssl_free_config_parameters(arg->sslCfg);
            free(arg->sslCfg);
        }
#endif
        memset(arg, 0, sizeof(struct http_client_demo_s));

        return QCLI_STATUS_SUCCESS_E;
    }

    if (strncmp(command, "disconn", 4) == 0)
    {
        qapi_Net_HTTPc_Disconnect(arg->client);
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
    else if (strcmp(command, "head") == 0)
    {
        req_cmd = QAPI_NET_HTTP_CLIENT_HEAD_E;
    }
    else if (strcmp(command, "delete") == 0)
    {
        req_cmd = QAPI_NET_HTTP_CLIENT_DELETE_E;
    }
    else if (strcmp(command, "tunnel") == 0)
    {
        req_cmd = QAPI_NET_HTTP_CLIENT_CONNECT_E;
    }

    /*       [0]    [1]           [2]            [3]    [4] [5] [6]
     * httpc tunnel <client_num> <origin server> <port> [-s -c  <calist>]
     */

    if (req_cmd == QAPI_NET_HTTP_CLIENT_CONNECT_E)
    {
        ip6_addr addr6;
        char host_port_string[80];
        int i;
        qbool_t origin_server_is_https = false; /* origin server is NOT an HTTPS server */
        char *calist = NULL;

        if (Parameter_Count < 4)
        {
            HTTPC_PRINTF("Missing <origin server> and/or <port>\n");
            return QCLI_STATUS_ERROR_E;
        }

        for (i = 4; i < Parameter_Count; i++)
        {
            if (Parameter_List[i].String_Value[0] == '-')
            {
                switch (Parameter_List[i].String_Value[1])
                {
                    case 's':   /* -s */
                        origin_server_is_https = true;
                        break;

                    case 'c':   /* -c ca_list.bin */
                        i++;
                        calist = Parameter_List[i].String_Value;
                        break;

                    default:
                        HTTPC_PRINTF("Unknown option: %s\n", Parameter_List[i].String_Value);
                        return QCLI_STATUS_ERROR_E;
                }
            }
            else
            {
                HTTPC_PRINTF("Unknown option: %s\n", Parameter_List[i].String_Value);
                return QCLI_STATUS_ERROR_E;
            }

            if (i == Parameter_Count)
            {
                QCLI_Printf(qcli_net_handle, "What is value of %s?\n", Parameter_List[i-1].String_Value);
                return QCLI_STATUS_ERROR_E;
            }
        } /* for */

        if (inet_pton(AF_INET6, Parameter_List[2].String_Value, &addr6) == 0)   /* an IPv6 address */
        {
            snprintf(host_port_string, sizeof(host_port_string), "[%s]:%s",
                    Parameter_List[2].String_Value, Parameter_List[3].String_Value);
        }
        else
        {
            snprintf(host_port_string, sizeof(host_port_string), "%s:%s",
                    Parameter_List[2].String_Value, Parameter_List[3].String_Value);
        }

        if (Parameter_List[3].Integer_Value == 443)
        {
            origin_server_is_https = true;
        }

        if (origin_server_is_https)
        {
            error = qapi_Net_HTTPc_Tunnel_To_HTTPS(arg->client, (const char *)calist, (const char *)host_port_string);
        }
        else
        {
            error = qapi_Net_HTTPc_Request(arg->client, req_cmd, (const char *)host_port_string);
        }
    }
    /* httpc get <client_num> [<url>] */
    else if (req_cmd)
    {
        char *url = "/";

        if (Parameter_Count >= 3)
        {
            url = Parameter_List[2].String_Value;
            if (strlen(url) > MAX_URL_LENGTH)
            {
                HTTPC_PRINTF("URL too long. Cannot be over %d\n", MAX_URL_LENGTH);
                return QCLI_STATUS_ERROR_E;
            }
        }
        error = qapi_Net_HTTPc_Request(arg->client, req_cmd, (const char *)url);
    }
    /*       [0]      [1]         [2]
     * httpc setbody <client_num> [<len>]
     */
    else if (strcmp(command, "setbody") == 0)
    {
        char *body = NULL;
        uint32_t len = BODY_BUFFER_SIZE;

        if (Parameter_Count > 2) {
            len = Parameter_List[2].Integer_Value;
        }

        if (len > BODY_BUFFER_SIZE)
            len = BODY_BUFFER_SIZE;

		body = httpc_malloc_body_demo(len);
        if (!body)
		    return QCLI_STATUS_ERROR_E;

        //HTTPC_PRINTF("len = %d : %d\n%s\n", len, strlen(body), body);
        HTTPC_PRINTF("body len = %d\n", strlen(body));

		error = qapi_Net_HTTPc_Set_Body(arg->client, (const char*)body, strlen(body));

        free(body);
    }
    else if (strcmp(command, "addheaderfield") == 0)
    {
        if (Parameter_Count < 4)
        {
            HTTPC_PRINTF("Missing parameters\n");
            return QCLI_STATUS_ERROR_E;
        }
        error = qapi_Net_HTTPc_Add_Header_Field(arg->client, Parameter_List[2].String_Value, Parameter_List[3].String_Value);
    }
    else if (strcmp(command, "clearheader") == 0)
    {
        error = qapi_Net_HTTPc_Clear_Header(arg->client);
    }
    else if(strcmp(command, "setparam") == 0)
    {
        if (Parameter_Count < 4)
        {
            HTTPC_PRINTF("Missing parameters\n");
            return QCLI_STATUS_ERROR_E;
        }
        error = qapi_Net_HTTPc_Set_Param(arg->client, Parameter_List[2].String_Value, Parameter_List[3].String_Value);
    }
    else if(strcmp(command, "cbaddhead") == 0)
    {
        uint16_t enable = 0;

        if (Parameter_Count < 3)
        {
            HTTPC_PRINTF("Missing parameters\n");
            return QCLI_STATUS_ERROR_E;
        }

        if (strncmp(Parameter_List[2].String_Value, "enable", 3) == 0)
        {
            enable = 1;
        }
        error = qapi_Net_HTTPc_CB_Enable_Adding_Header(arg->client, enable);
    }
    else if(strncmp(command, "sendraw", 4) == 0)
    {
        char *buf;
        /*       [0]     [1]   [2]
         * httpc sendraw  1   ["<data string>"]
         */
        if (Parameter_Count < 3)
        {
            buf = "HEAD / HTTP/1.1\r\n"
                  "Host: 172.217.14.110:443\r\n"
                  "Accept: text/html, */*\r\n"
                  "User-Agent: Quartz IOE\r\n"
                  "Connection: keep-alive\r\n"
                  "Cache-control: no-cache\r\n"
                  "\r\n";
        }
        else
        {
            buf = Parameter_List[2].String_Value;
        }
        error = qapi_Net_HTTPc_Send_Data(arg->client, buf, strlen(buf));
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
#endif

#ifdef HTTPC_DEMO_DEBUG
#pragma GCC pop_options
#endif
