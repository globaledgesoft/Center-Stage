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

typedef enum adss_ret_status_s {
	ADSS_SUCCESS = 0,
	ADSS_FAILURE,
	ADSS_NO_MEMORY,
	ADSS_ERR_CREATE_SOCKET,
	ADSS_ERR_FTP_SEND_COMMAND,
	ADSS_ERR_FTP_CONTROL_SOCKET_CLOSED,
	ADSS_ERR_FTP_SESSION_ALREADY_START,
	ADSS_ERR_FTP_SESSION_NOT_START,
	ADSS_ERR_FTP_URL_FORMAT,
	ADSS_ERR_FTP_GET_LOCAL_ADDRESS,
	ADSS_ERR_FTP_CREATE_SOCKET,
	ADSS_ERR_FTP_BIND_FAIL,
	ADSS_ERR_FTP_CONNECT_FAIL,
	ADSS_ERR_FTP_PEER_CLOSED,
	ADSS_ERR_FTP_DIR_NOT_EXIST,
	ADSS_ERR_FTP_LOGIN_INCORRECT,
	ADSS_ERR_FTP_SYST,
	ADSS_ERR_FTP_SET_TYPE,
	ADSS_ERR_FTP_SET_PORT,
	ADSS_ERR_FTP_RESTART_NOT_SUPPORT,
	ADSS_ERR_FTP_FILE_NOT_FOUND,
	ADSS_ERR_FTP_DATA_CONNECTION_TIMEOUT,
	ADSS_ERR_FTP_ACCEPT_DATA_CONNECT,
	ADSS_ERR_FTP_FILE_NOT_COMPLETE,
	ADSS_ERR_FTP_DATA_SOCK_CLOSED,
} ADSS_RET_STATUS;

#define ADSS_FTP_CMD_BUF_MAX                 256
#define		ADSS_USR_SEND_DONE_SIG_MASK      0x40

#define		MAX_BUFF_COUNT		2
#define     MAX_DATA_BUFF_SIZE		48
#define		I2S_DMA_BLOCK_SIZE		64
#define		I2S_FREQUENCY           QAPI_I2S_FREQ_44_1_KHZ_E
#define		PCM_FREQUENCY           QAPI_I2S_FREQ_32_KHZ_E

extern  QCLI_Group_Handle_t qcli_adss_group;              /* Handle for our QCLI Command Group. */
extern  volatile int	audio_echo_loop_flag;
extern  uint16_t audio_data[];
extern  uint32_t audio_data_size;

extern volatile int	audio_echo_loop_flag;

void init_pcm_data ();

void Initialize_ADSS_Demo(void);

