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
   
#ifndef __ADSS_PCM__H__
#define __ADSS_PCM__H__

#include "qurt_signal.h"

#define		FTP_THREAD_STACK_SIZE       2048
#define		FTP_THREAD_PRIORITY         9
#define     FTP_THREAD_NAME             "ftp_audio"

typedef struct wave_fmt_s {
	uint32_t  ChunkID;
	uint32_t  ChunkSize;
	uint32_t  Format;
	uint32_t  SubChunk1ID;
	uint32_t  SubChunk1Size;
	uint16_t  AudioFormat;
	uint16_t  NumChannels;
	uint32_t  SampleRate;
	uint32_t  ByteRate;
	uint16_t  BlockAlign;
	uint16_t  BitsPerSample;
	uint32_t  SubChunk2ID;
	uint32_t  SubChunk2Size;
} ADSS_WAVE_FMT_t;

/*typedef struct data_buf_link_s {
	struct data_buf_link_s  *next;
	uint8_t   *buf_ptr;
} Data_Buf_Link_t;
*/

#define MAX_SLOTS		16

extern  uint8_t  fixed_slots[];
extern uint32_t    max_slots;

#define ADSS_FTP_CMD_BUF_MAX                 256

#define ADSS_EMPTY_BUF_AVAIL_SIG_MASK          0x01
#define ADSS_DATA_BUF_AVAIL_SIG_MASK           0x02
#define ADSS_USR_TASK_DONE_SIG_MASK            0x04

#define ADSS_EMPTY_BUF_SIG_MASK     (ADSS_EMPTY_BUF_AVAIL_SIG_MASK | ADSS_USR_TASK_DONE_SIG_MASK)
#define ADSS_DATA_BUF_SIG_MASK      (ADSS_DATA_BUF_AVAIL_SIG_MASK | ADSS_USR_TASK_DONE_SIG_MASK)

#define ADSS_DMA_CALLBACK_SIG_MASK             0x08
#define ADSS_WAV_FILE_DL_DONE_SIG_MASK         0x10

#define ADSS_DMA_WAV_DL_SIG_MASK               (ADSS_DMA_CALLBACK_SIG_MASK | ADSS_WAV_FILE_DL_DONE_SIG_MASK)

#define		PCM_FREQUENCY           QAPI_I2S_FREQ_32_KHZ_E

#define DEBUG_ADSS_PCM_PRINTF
#if defined(DEBUG_ADSS_PCM_PRINTF)
#define ADSS_DEBUG_PRINTF(args...) QCLI_Printf(qcli_adss_group,args)
#else
#define ADSS_DEBUG_PRINTF(args...)
#endif

extern QCLI_Group_Handle_t qcli_adss_group;              /* Handle for our QCLI Command Group. */

extern qapi_PCM_Config_t pcm_config;
extern qapi_PCM_Config_t pcm_in_config;

extern qapi_I2S_Handle  hd_pcm_out;
extern qapi_I2S_Handle  hd_pcm_in;

extern volatile  uint32_t   loop_count;

extern qurt_signal_t pcm_rcv_done_signal;
extern qurt_signal_t pcm_rcv_done_signal;
extern uint32_t       pkt_count;
extern uint32_t       pkt_total;
extern volatile  uint32_t   send_count;
extern  qapi_PCM_Config_t pcm_config;
extern  qapi_PCM_Config_t pcm_in_config;

typedef struct adss_pcm_session_s {
    uint8_t  local_v6addr[16];
    uint8_t  remote_v6addr[16];
	
    char file[32];
	qurt_signal_t  adss_rx_done_signal;
	qurt_signal_t  adss_tx_done_signal;
	
	uint8_t       wav_fmt[78];
	
	char cmd_buf[ADSS_FTP_CMD_BUF_MAX];
	char resp_buf[ADSS_FTP_CMD_BUF_MAX];
} ADSS_PCM_SESSION_t;

ADSS_RET_STATUS adss_Tcp_Connect_Server(const char* interface_name, char *url, uint32_t offset);
ADSS_RET_STATUS adss_Tcp_Close_Data_Sock(void);
ADSS_RET_STATUS adss_Tcp_Close_Control_Sock(void);
ADSS_RET_STATUS adss_Tcp_Close_Data_Connect_Sock(void);
ADSS_RET_STATUS adss_PCM_Deinit();

void adss_pcm_send_receive(uint32_t pkt_count, uint32_t buf_size, uint32_t sample_freq);

#endif
