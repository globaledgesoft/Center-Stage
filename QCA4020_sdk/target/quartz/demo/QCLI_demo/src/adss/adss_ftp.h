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
   
#ifndef __ADSS_FTP__H__
#define __ADSS_FTP__H__

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

typedef struct data_buf_link_s {
	struct data_buf_link_s  *next;
	uint8_t   *buf_ptr;
} Data_Buf_Link_t;

#define ADSS_FTP_CMD_BUF_MAX                 256

#define ADSS_EMPTY_BUF_AVAIL_SIG_MASK          0x01
#define ADSS_DATA_BUF_AVAIL_SIG_MASK           0x02
#define ADSS_USR_TASK_DONE_SIG_MASK            0x04

#define ADSS_EMPTY_BUF_SIG_MASK     (ADSS_EMPTY_BUF_AVAIL_SIG_MASK | ADSS_USR_TASK_DONE_SIG_MASK)
#define ADSS_DATA_BUF_SIG_MASK      (ADSS_DATA_BUF_AVAIL_SIG_MASK | ADSS_USR_TASK_DONE_SIG_MASK)

#define ADSS_DMA_CALLBACK_SIG_MASK             0x08
#define ADSS_WAV_FILE_DL_DONE_SIG_MASK         0x10

#define ADSS_DMA_WAV_DL_SIG_MASK               (ADSS_DMA_CALLBACK_SIG_MASK | ADSS_WAV_FILE_DL_DONE_SIG_MASK)


#define htonl(l) (((((l) >> 24) & 0x000000ff)) | \
                 ((((l) >>  8) & 0x0000ff00)) | \
                 (((l) & 0x0000ff00) <<  8) | \
                 (((l) & 0x000000ff) << 24))
#define ntohl(l) htonl(l)
#define htons(s)    ((((s) >> 8) & 0xff) | (((s) << 8) & 0xff00))
#define ntohs(s) htons(s)

#define OTA_FTP_DATA_PORT                        1068
#define OTA_FTP_CMD_BUF_LEN                      1024
#define OTA_FTP_RECEIVE_TIMEOUT                 (30*1000)

#define ADSS_INFO_PRINTF(args...) QCLI_Printf(qcli_adss_group,args)

#define DEBUG_ADSS_FTP_PRINTF
#if defined(DEBUG_ADSS_FTP_PRINTF)
#define ADSS_FTP_DEBUG_PRINTF(args...) QCLI_Printf(qcli_adss_group,args)
#else
#define ADSS_FTP_DEBUG_PRINTF(args...)
#endif

typedef struct adss_ftp_session_s {
    int32_t data_sock;                /* data socket.*/
    int32_t data_connect_sock;
    int32_t control_sock;             /* control socket.*/
    uint32_t local_ip_addr;
    uint8_t local_v6addr[16];
    uint32_t remote_ip_addr;
    uint8_t  remote_v6addr[16];
    uint16_t cmd_port;
    uint16_t data_port;
    char user[32];
    char password[12];
    char file[32];
    uint8_t v6_enable_flag;
    int32_t scope_id;
	
	qurt_thread_t thread_id;
	qurt_thread_attr_t attr;
	qurt_signal_t  buf_signal;
	
	Data_Buf_Link_t  *m_pEmptyLink;
	qurt_mutex_t  buf_empty_mutex;
	qurt_signal_t  buf_empty_signal;
	
	Data_Buf_Link_t  *m_pDataLink;
	Data_Buf_Link_t  *m_pDataLink_Trail;
	qurt_mutex_t  buf_data_mutex;
	qurt_signal_t  buf_data_signal;

	Data_Buf_Link_t  *m_pFreeLink;
	qurt_mutex_t  buf_free_link_mutex;

	qurt_signal_t  adss_dma_cb_signal;
	
	uint8_t       wav_fmt[78];
	
	char cmd_buf[ADSS_FTP_CMD_BUF_MAX];
	char resp_buf[ADSS_FTP_CMD_BUF_MAX];
} ADSS_FTP_SESSION_t;


extern QCLI_Group_Handle_t qcli_adss_group;              /* Handle for our QCLI Command Group. */
extern uint32_t generate_rand32(void);

extern qapi_I2S_Channel_Config_t i2s_port0_config;
extern volatile  qapi_I2S_Handle  hdI2S;
extern volatile  uint32_t   send_count;

ADSS_RET_STATUS adss_Ftp_Connect_Server(const char* interface_name, char *url, uint32_t offset);
ADSS_RET_STATUS adss_Ftp_Close_Data_Sock(void);
ADSS_RET_STATUS adss_Ftp_Close_Control_Sock(void);
ADSS_RET_STATUS adss_Ftp_Close_Data_Connect_Sock(void);

ADSS_RET_STATUS adss_Ftp_Send_Cmd_Resp(char *cmd, char *param, int *resp_code);
void release_buf_link(Data_Buf_Link_t *p);
uint8_t *get_ftp_empty_buf();
uint8_t *get_ftp_data_buf();
uint8_t *peek_get_ftp_empty_buf();
ADSS_RET_STATUS adss_Ftp_Recv_Data(uint8_t *buffer, uint32_t buf_len, uint32_t *ret_size);
uint8_t *put_ftp_data_buf(uint8_t *pbuf);
void put_ftp_empty_buf(uint8_t *pbuf);
ADSS_RET_STATUS adss_playOnWifi_Init();
ADSS_RET_STATUS  init_buf_link(int size);
ADSS_RET_STATUS adss_Ftp_Fin(void);

void tcp_socket_data_send_task(void *param);

extern ADSS_FTP_SESSION_t *adss_ftp_session;
extern ADSS_RET_STATUS adss_Tcp_Socket_Connect_Server(char *srv_ip_addr, uint16_t port);
extern ADSS_RET_STATUS adss_Ftp_Recv_Cmd(uint32_t to, int *resp_code);

#endif
