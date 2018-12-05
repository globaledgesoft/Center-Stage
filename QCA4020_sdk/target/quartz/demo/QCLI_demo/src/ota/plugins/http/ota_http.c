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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "qapi/qapi_types.h"
#include "qapi/qapi_status.h"
#include "qapi/qurt_signal.h"
#include "qapi/qapi_socket.h"
#include "qapi/qapi_netservices.h"
#include "qapi/qapi_firmware_upgrade.h"
#include "malloc.h"
#include "netutils.h"
#include "bench.h"
#include "ota_http.h"

extern QCLI_Group_Handle_t qcli_fw_upgrade_group;              /* Handle for our QCLI Command Group. */
#define FW_UPGRADE_PRINTF_HANDLE  qcli_fw_upgrade_group

#define HTTPC_DEFAULT_MAX_BODY_LEN 2048
#define HTTPC_DEFAULT_MAX_HEADER_LEN   350
#define MAX_PRINTF_LENGTH 256

#define   HTTPC_RX_DATA_DONE_SIG_MASK    			0x01
#define   HTTPC_RX_DATA_ERROR_SIG_MASK              0x02
#define   HTTPC_RX_DATA_FINISH_SIG_MASK             0x04
#define   HTTPC_DATA_RX_ALL_SIG_MASK				(HTTPC_RX_DATA_DONE_SIG_MASK | HTTPC_RX_DATA_FINISH_SIG_MASK | HTTPC_RX_DATA_ERROR_SIG_MASK)
qurt_signal_t   data_ready_signal;

#define   HTTPC_BUFFER_EMPTY_SIG_MASK               0x01
qurt_signal_t   data_drain_signal;

uint32_t  data_ready_len;
char  *data_ready_ptr;

struct http_client_t
{
	qapi_Net_HTTPc_handle_t client;
	uint32_t total_len;
	qapi_Net_SSL_Obj_Hdl_t sslCtx;
    qapi_Net_SSL_Config_t *sslCfg;
	char http_svr[64];
	uint32_t  timeout, port;
	char  dl_file[32];
} http_client;

/*
 *  HTTP client thread callback
 *        arg:  parameter that qapi_Net_HTTPc_New_sess pass, that is, &http_client
 *      state:  indicate HTPP client status 
 *  http_resp:  HTTP client response
 */
void http_client_cb(void* arg, int32_t state, void* http_resp)
{
    qapi_Net_HTTPc_Response_t* temp = (qapi_Net_HTTPc_Response_t *)http_resp;
    struct http_client_t* hc = (struct http_client_t *)arg;

    if (state >= 0)
    {

        if (temp->length && temp->data)
        {			
			data_ready_len = temp->length;
			data_ready_ptr = (char *)temp->data;
			
			qurt_signal_set(&data_ready_signal, HTTPC_RX_DATA_DONE_SIG_MASK);
			qurt_signal_wait(&data_drain_signal, HTTPC_BUFFER_EMPTY_SIG_MASK, QURT_SIGNAL_ATTR_CLEAR_MASK);
			hc->total_len += temp->length;
        }

        if (state == 0 && hc->total_len)
        {
            hc->total_len = 0; // Finished
			qurt_signal_set(&data_ready_signal, HTTPC_RX_DATA_FINISH_SIG_MASK);
        }
    }
    else
    {
		qurt_signal_set(&data_ready_signal, HTTPC_RX_DATA_ERROR_SIG_MASK);
        hc->total_len = 0;
    }
}

/*
 * OTA HTTP plugin Init
 * interface_name:    interface name, such as wlan1
 *            url:    parameters, format: <timeout>:<server>:<port>/<url>
 */
qapi_Fw_Upgrade_Status_Code_t plugin_Http_Init(const char* interface_name, const char *url, void *init_param)
{
    int error = QAPI_OK;
	char  *svr_ptr, *ptr, ed_buf[32];
    qapi_Net_HTTPc_Method_e req_cmd = 0;
    uint32_t port = 0;
    uint32_t server_offset = 0;
    uint32_t timeout = 0;

    /* start HTTP client thread */
    error = qapi_Net_HTTPc_Start();
    if (error)
    {
        return QCLI_STATUS_ERROR_E;
    }
	    
    /* connect HTTP server */
    {

		ptr = strchr(url, ':');
		
		if( ptr == NULL )
		{
			return FW_UPGRADE_ERR_HTTP_URL_FORMAT_E;
		}
		
		memcpy(ed_buf, url, ptr - url);
		ed_buf[ptr - url] = '\0';
		
        http_client.timeout = atoi(ed_buf);

		ptr = ptr + 1;
        if(strlen(ptr) >= 64)
        {
            return QAPI_FW_UPGRADE_ERROR_E;
        }

        if(strncmp(ptr, "https://", 8) == 0)
        {
            server_offset = 8;
            http_client.sslCtx = qapi_Net_SSL_Obj_New(QAPI_NET_SSL_CLIENT_E);
            if (http_client.sslCtx == QAPI_NET_SSL_INVALID_HANDLE)
            {
                if (http_client.sslCfg)
                {
                    free(http_client.sslCfg);
                }
                memset(&http_client, 0, sizeof(struct http_client_t));
                return QCLI_STATUS_ERROR_E;
            }
        }
        else if(strncmp(ptr, "http://", 7) == 0)
        {
            server_offset = 7;
        }

		svr_ptr = ptr + server_offset;
		
		ptr = strchr(svr_ptr, '/');
		if( ptr == NULL )
		{
			return FW_UPGRADE_ERR_HTTP_URL_FORMAT_E;
		}
		memcpy(http_client.http_svr, svr_ptr, ptr - svr_ptr);
		http_client.http_svr[ptr - svr_ptr]='\0';
		
		strcpy(http_client.dl_file, ptr+1);

        http_client.port = 80;
		svr_ptr = http_client.http_svr;
		ptr = strchr(svr_ptr, ':');
		if (ptr != NULL)
		{
			*ptr = '\0';
			ptr++;
			port = atoi(ptr);

        	if (port != 0)
				http_client.port = port;			
		}
		
	    QCLI_Printf(FW_UPGRADE_PRINTF_HANDLE, "svr_ptr: %s\r\n", http_client.http_svr);
		QCLI_Printf(FW_UPGRADE_PRINTF_HANDLE, "port=%d\r\n", http_client.port);
	    QCLI_Printf(FW_UPGRADE_PRINTF_HANDLE, "file uri: %s\r\n", http_client.dl_file);
		
		qurt_signal_init(&data_ready_signal);
		qurt_signal_init(&data_drain_signal);
		
        http_client.client = qapi_Net_HTTPc_New_sess(timeout, http_client.sslCtx, http_client_cb, 
		                            (void *)&http_client, HTTPC_DEFAULT_MAX_BODY_LEN, HTTPC_DEFAULT_MAX_HEADER_LEN);
		
        if (http_client.client == NULL)
        {
			QCLI_Printf(FW_UPGRADE_PRINTF_HANDLE, "There is no available http client session\r\n");
            if (http_client.sslCtx != QAPI_NET_SSL_INVALID_HANDLE)
                qapi_Net_SSL_Obj_Free(http_client.sslCtx);

            if (http_client.sslCfg)
            {
                free(http_client.sslCfg);
            }

            memset(&http_client, 0, sizeof(struct http_client_t));
            return QAPI_FW_UPGRADE_ERROR_E;
        }
		QCLI_Printf(FW_UPGRADE_PRINTF_HANDLE, "new session done\r\n");

        error = QAPI_OK;

        if (error == QAPI_OK)
        {
            error = qapi_Net_HTTPc_Connect(http_client.client, http_client.http_svr, http_client.port);
        }

        if (error)
        {
            qapi_Net_HTTPc_Free_sess(http_client.client);
            if (http_client.sslCtx != QAPI_NET_SSL_INVALID_HANDLE)
                qapi_Net_SSL_Obj_Free(http_client.sslCtx);

            if (http_client.sslCfg)
            {
                free(http_client.sslCfg);
            }

            memset(&http_client, 0, sizeof(struct http_client_t));
            return QAPI_FW_UPGRADE_ERROR_E;
        }

		QCLI_Printf(FW_UPGRADE_PRINTF_HANDLE, "connect done !!!\r\n");
    }
	
/*
 * trigger GET request
 */
    req_cmd = QAPI_NET_HTTP_CLIENT_GET_E;
    error = qapi_Net_HTTPc_Request(http_client.client, req_cmd, http_client.dl_file);
	if (error == QAPI_OK)
		QCLI_Printf(FW_UPGRADE_PRINTF_HANDLE, "request done\r\n");
		
	return QAPI_FW_UPGRADE_OK_E;
}

/*
 * OTA HTTP plugin receive data
 *    buffer:    received data buffer
 *   buf_len:    received data buffer size in bytes
 *  ret_size:    data size in buffer after receiving done
 */

qapi_Fw_Upgrade_Status_Code_t plugin_Http_Recv_Data(uint8_t *buffer, uint32_t buf_len, uint32_t *ret_size)
{
	uint32_t  signals;
	
	*ret_size = 0;
	do {
		signals = qurt_signal_wait(&data_ready_signal, HTTPC_DATA_RX_ALL_SIG_MASK, QURT_SIGNAL_ATTR_CLEAR_MASK);
		
		if (signals & HTTPC_RX_DATA_DONE_SIG_MASK)
		{
			if  (data_ready_len > buf_len)
			{
				memcpy(buffer, data_ready_ptr, buf_len);
				
				data_ready_len -= buf_len;
				data_ready_ptr += buf_len;
				
				*ret_size = buf_len;
				qurt_signal_set(&data_ready_signal, HTTPC_RX_DATA_DONE_SIG_MASK);
				break;
			}
			else
			{
				memcpy(buffer, data_ready_ptr, data_ready_len);
				
				*ret_size = data_ready_len;
				data_ready_len = 0;
				qurt_signal_set(&data_drain_signal, HTTPC_BUFFER_EMPTY_SIG_MASK);				
			}
		}
		if (signals & HTTPC_RX_DATA_FINISH_SIG_MASK)
		{
			
		}
		if (signals & HTTPC_RX_DATA_ERROR_SIG_MASK)
		{
			
		}
	} while (0);

	return QAPI_FW_UPGRADE_OK_E;
}

/*
 *  OTA HTTP done
 */
qapi_Fw_Upgrade_Status_Code_t plugin_Http_Fin(void)
{
    int error = qapi_Net_HTTPc_Stop();
	
	if (http_client.client != NULL)
	{
		qapi_Net_HTTPc_Free_sess(http_client.client);
		http_client.client = NULL;
	}

	if (error == QAPI_OK)
       return QAPI_FW_UPGRADE_OK_E;
   
    return QAPI_FW_UPGRADE_ERROR_E;
}

/*
 *  OTA HTTP process abort
 */
qapi_Fw_Upgrade_Status_Code_t plugin_Http_Abort(void)
{
    qapi_Net_HTTPc_Stop();

	if (http_client.client != NULL)
	{
		qapi_Net_HTTPc_Free_sess(http_client.client);
		http_client.client = NULL;
	}
	return QAPI_FW_UPGRADE_OK_E;
}

/*
 *  HTTP doesn't support resume
 */
qapi_Fw_Upgrade_Status_Code_t plugin_Http_Resume(const char* interface_name, const char *url, uint32_t offset)
{
	return QAPI_FW_UPGRADE_OK_E;
}

