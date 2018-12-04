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

/**
   @brief This function registers the ADSS demo commands with QCLI.
*/
   
#ifndef __ADSS_MEM__H__
#define __ADSS_MEM__H__

typedef struct adss_data_buf_link_s {
	struct adss_data_buf_link_s  *next;
	uint8_t   *buf_ptr;
} ADSS_DATA_BUF_LINK_t;

#define ADSS_FTP_CMD_BUF_MAX                 256

#define ADSS_EMPTY_BUF_AVAIL_SIG_MASK          0x01
#define ADSS_DATA_BUF_AVAIL_SIG_MASK           0x02
#define ADSS_USR_TASK_DONE_SIG_MASK            0x04

#define ADSS_EMPTY_BUF_SIG_MASK     (ADSS_EMPTY_BUF_AVAIL_SIG_MASK | ADSS_USR_TASK_DONE_SIG_MASK)
#define ADSS_DATA_BUF_SIG_MASK      (ADSS_DATA_BUF_AVAIL_SIG_MASK | ADSS_USR_TASK_DONE_SIG_MASK)

#define ADSS_DMA_CALLBACK_SIG_MASK             0x08
#define ADSS_WAV_FILE_DL_DONE_SIG_MASK         0x10

#define ADSS_DMA_WAV_DL_SIG_MASK               (ADSS_DMA_CALLBACK_SIG_MASK | ADSS_WAV_FILE_DL_DONE_SIG_MASK)


typedef struct adss_mem_session_s {

	qurt_signal_t    buf_signal;

	ADSS_DATA_BUF_LINK_t  *m_pEmptyLink;
	qurt_mutex_t     buf_empty_mutex;
	qurt_signal_t    buf_empty_signal;
	
	ADSS_DATA_BUF_LINK_t  *m_pDataLink;
	ADSS_DATA_BUF_LINK_t  *m_pDataLink_Trail;
	qurt_mutex_t     buf_data_mutex;
	qurt_signal_t    buf_data_signal;

#if  defined(STREAM_SPEED_CONTROL)	
	uint32_t		 buf_empty_count;
	uint32_t		 buf_data_count;
#endif

	ADSS_DATA_BUF_LINK_t  *m_pFreeLink;
	qurt_mutex_t     buf_free_link_mutex;

	ADSS_DATA_BUF_LINK_t  *m_pLinkBuffAddr;	
} ADSS_MEM_SESSION_t;

extern ADSS_MEM_SESSION_t  *m_pAdssMem;

#define  MEM_BUF_READY()  qurt_signal_set(&m_pAdssMem->buf_data_signal, ADSS_DATA_BUF_SIG_MASK);	
#define  MEM_BUF_USR_TASK_DONE()  qurt_signal_set(&m_pAdssMem->buf_data_signal, ADSS_USR_TASK_DONE_SIG_MASK);


void adss_release_buf_link(ADSS_DATA_BUF_LINK_t *p);
void adss_put_tcp_empty_buf(uint8_t *pbuf);

uint8_t *adss_get_tcp_empty_buf();
uint8_t *adss_get_tcp_data_buf();
uint8_t *adss_peek_get_tcp_empty_buf();
uint8_t *adss_put_tcp_data_buf(uint8_t *pbuf);

ADSS_DATA_BUF_LINK_t  *adss_get_buf_link();

ADSS_RET_STATUS  adss_init_buf_link(int size);
ADSS_RET_STATUS  adss_Deinit_buf_link();

uint32_t adss_get_empty_buf_count();

#endif
