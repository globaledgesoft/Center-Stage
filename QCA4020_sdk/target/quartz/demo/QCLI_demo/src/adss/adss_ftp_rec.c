/*
 * Copyright (c) 2015-2018 Qualcomm Technologies, Inc.
 * 2015-2016 Qualcomm Atheros, Inc.
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
#include "qurt_signal.h"
#include "qurt_mutex.h"
#include "qurt_thread.h"
#include "qapi/qapi_types.h"
#include "qapi/qapi_status.h"
#include "qapi/qapi_socket.h"
#include "qapi/qapi_netservices.h"
#include "qapi_i2s.h"
#include <qcli_api.h>

#include "malloc.h"
#include "adss_demo.h"
#include "adss_ftp.h"

#include "netutils.h"

/********************************************************************************************************/

extern QCLI_Group_Handle_t qcli_adss_group;              /* Handle for our QCLI Command Group. */

#define htonl(l) (((((l) >> 24) & 0x000000ff)) | \
                 ((((l) >>  8) & 0x0000ff00)) | \
                 (((l) & 0x0000ff00) <<  8) | \
                 (((l) & 0x000000ff) << 24))
#define ntohl(l) htonl(l)
#define htons(s)    ((((s) >> 8) & 0xff) | (((s) << 8) & 0xff00))
#define ntohs(s) htons(s)

#define DEBUG_ADSS_FTP_PRINTF
#if defined(DEBUG_ADSS_FTP_PRINTF)
#define ADSS_FTP_DEBUG_PRINTF(args...) QCLI_Printf(qcli_adss_group,args)
#else
#define ADSS_FTP_DEBUG_PRINTF(args...)
#endif


extern ADSS_FTP_SESSION_t *adss_ftp_session;


/***************************************************************************************************/
ADSS_RET_STATUS adss_Tcp_Socket_Connect_Server(char *srv_ip_addr, uint16_t port)
{
	struct sockaddr_in svr_addr;

	qapi_Status_t   status;

	if( inet_pton(AF_INET, srv_ip_addr, &(adss_ftp_session->remote_ip_addr)) != 0 )
    {
        return ADSS_ERR_FTP_URL_FORMAT;
    }

    if((adss_ftp_session->data_connect_sock = qapi_socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        adss_ftp_session->data_connect_sock = 0;
        ADSS_FTP_DEBUG_PRINTF("ERROR: Unable to create tcp socket\r\n");
        status = ADSS_ERR_FTP_CREATE_SOCKET;
        return status;
    }

    ADSS_FTP_DEBUG_PRINTF("Server IP:%08X\r\n", adss_ftp_session->remote_ip_addr);
    memset(&svr_addr, 0, sizeof(svr_addr));
	
    svr_addr.sin_addr.s_addr = adss_ftp_session->remote_ip_addr;
    svr_addr.sin_port = htons(port);
    svr_addr.sin_family  = AF_INET;

    status = qapi_connect(adss_ftp_session->data_connect_sock, (struct sockaddr *)&svr_addr, sizeof(svr_addr));
	return status;		
}

/*************************************************************************************************************/
/*************************************************************************************************************/ 
/*
 *
 */
ADSS_RET_STATUS adss_Tcp_Socket_Send_Data(uint8_t *buffer, uint32_t buf_len, uint32_t *ret_size)
{
    ADSS_RET_STATUS rtn = ADSS_SUCCESS;
    int32_t sent_bytes;
	uint32_t rcv_total_len = 0;
	uint32_t need_send_len = buf_len;

    *ret_size = sent_bytes = 0;

    if( adss_ftp_session->data_connect_sock == 0 )
    {
        return ADSS_ERR_FTP_DATA_SOCK_CLOSED;
    }
	
    /* -----------------------------------------------------*/
    /* process FTP data                                   */
    /* -----------------------------------------------------*/
    while (1)
    {
        /*-----------------------------------------------------------*/
        /* Receiving data from FTP server.*/
        {
            /* upload it*/
			sent_bytes = qapi_send(adss_ftp_session->data_connect_sock, (char *)buffer, need_send_len, 0);

            if( sent_bytes >  0 )
            {
                //do receive data from peer
				need_send_len -= sent_bytes;
				buffer += sent_bytes;
				rcv_total_len += sent_bytes;
				
				if (need_send_len == 0)
				{
					rtn = ADSS_SUCCESS;
					break;					
				}
            }
        } 
    }

    *ret_size = rcv_total_len;
//    ADSS_FTP_DEBUG_PRINTF("Ftp_Send_Data: rtn=%d, sent=%d\r\n", rtn, *ret_size);
    return rtn;
}
 
/* I2S send test */
extern qapi_I2S_Channel_Config_t i2s_port3_config;
extern volatile  qapi_I2S_Handle  hdI2S;
extern volatile  uint32_t   send_count;

void adss_mbox_ftp_rec_dma_callback(void *hd, uint32_t status, void *param)
{
    send_count++;

	put_ftp_data_buf(param);

	qurt_signal_set(&adss_ftp_session->adss_dma_cb_signal, ADSS_DMA_CALLBACK_SIG_MASK);
}

qapi_Status_t adss_Receive_Speaker_Init(uint32_t buf_len, uint32_t pkt_count)
{
  qapi_I2S_Instance_e inst;
  qapi_Status_t  status;

  inst = QAPI_I2S_INSTANCE_004_E;

  i2s_port3_config.buf_Size = buf_len;
  i2s_port3_config.num_Tx_Desc = pkt_count;
  i2s_port3_config.num_Rx_Desc = 0;
  i2s_port3_config.freq = QAPI_I2S_FREQ_16_KHZ_E;	  // wifi throughput limitation

  status = qapi_I2S_Init(inst, &i2s_port3_config, (void **)&hdI2S);
  if ( QAPI_OK != status ) 
      return status;

  qapi_I2S_Open(hdI2S);

  status = qapi_I2S_Intr_Register (hdI2S, adss_mbox_ftp_rec_dma_callback, 0);
  if ( QAPI_OK != status ) 
      return status; 
  
  return  QAPI_OK;
}

void tcp_socket_data_send_task(void *param)
{
	uint32_t  	ret_size, buf_len;
	ADSS_RET_STATUS rtn;
	uint8_t     *pbuf;

	buf_len = *(uint32_t *)param;
/*
 *  set up upload data connection
 */
 
	do {
		pbuf = get_ftp_data_buf();
	    if (pbuf == NULL)
			break;
		
		rtn = adss_Tcp_Socket_Send_Data(pbuf, buf_len, &ret_size);
		
		if (rtn != ADSS_SUCCESS)
			break;
		put_ftp_empty_buf(pbuf);
	} while (1);
	
	adss_ftp_session->thread_id = 0;
	qurt_thread_stop();
}

ADSS_RET_STATUS adss_Tcp_socket_rec_wifi(char* srv_ip_addr, uint16_t port, uint32_t buf_len, uint32_t pkt_count)
{
	uint32_t  	i;
    uint8_t   	**pbuf_v, *pbuf;
	uint32_t    sent_len;
    qapi_Status_t  status;
	ADSS_RET_STATUS rtn;
	qurt_time_t  duration = 1000;
	
    ADSS_FTP_DEBUG_PRINTF("Server IP:%s port:%d\r\n", srv_ip_addr,  port);
    ADSS_FTP_DEBUG_PRINTF("buf size:%d pkt count:%d\r\n", buf_len,  pkt_count);

	rtn = adss_playOnWifi_Init();
    if( rtn != ADSS_SUCCESS )
    {
		return  rtn;
    }
	
	rtn = init_buf_link(pkt_count);
    if( rtn != ADSS_SUCCESS )
    {
		return  rtn;
    }

	rtn = adss_Tcp_Socket_Connect_Server(srv_ip_addr, port);

    if( rtn != ADSS_SUCCESS )
    {
	    ADSS_FTP_DEBUG_PRINTF("TCP socket connection Fails\r\n");
		return  rtn;
    }

	qurt_thread_attr_init (&adss_ftp_session->attr);
	qurt_thread_attr_set_name (&adss_ftp_session->attr, FTP_THREAD_NAME);
	qurt_thread_attr_set_priority (&adss_ftp_session->attr, FTP_THREAD_PRIORITY);
	qurt_thread_attr_set_stack_size (&adss_ftp_session->attr, FTP_THREAD_STACK_SIZE);
	qurt_thread_create(&adss_ftp_session->thread_id, &adss_ftp_session->attr, tcp_socket_data_send_task,  &buf_len);
	
	pbuf_v = (uint8_t **)malloc(sizeof(uint8_t *) * pkt_count);
	if (pbuf_v == NULL)
		return ADSS_NO_MEMORY;
	
	status = adss_Receive_Speaker_Init(buf_len, pkt_count);
    if (status != QAPI_OK)
	{
		ADSS_FTP_DEBUG_PRINTF("I2S Init Fail\r\n");
		return ADSS_FAILURE;
	}
	qurt_signal_init(&adss_ftp_session->adss_dma_cb_signal);
	
	for (i=0; i < pkt_count; i++)
	{
		status = qapi_I2S_Get_Buffer(&pbuf);
		pbuf_v[i] = pbuf;
	}
	
	qapi_I2S_Send_Receive(NULL, NULL, 0, hdI2S, pbuf_v, pkt_count);

	do
	{
		qurt_signal_wait(&adss_ftp_session->adss_dma_cb_signal, ADSS_DMA_CALLBACK_SIG_MASK, QURT_SIGNAL_ATTR_CLEAR_MASK);		
		pbuf = peek_get_ftp_empty_buf();
		if (pbuf != NULL)
		{
			status = qapi_I2S_Receive_Data(hdI2S, pbuf, buf_len, &sent_len);	  			
		}
	} while (audio_echo_loop_flag);
	 

	qurt_signal_set(&adss_ftp_session->buf_data_signal, ADSS_USR_TASK_DONE_SIG_MASK);
	while (adss_ftp_session->thread_id != 0)
	{
		qurt_thread_sleep (duration);		
	}

	qurt_signal_delete(&adss_ftp_session->adss_dma_cb_signal);

	adss_Ftp_Fin();
	
	qapi_I2S_Deinit (hdI2S);
	
	qurt_thread_stop();

	return 0;
}
