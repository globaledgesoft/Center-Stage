/* Copyright (c) 2011-2016.  Qualcomm Atheros, Inc.
 * All rights reserved.
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

#ifndef _FW_UPGRADE_FTP_H_
#define _FW_UPGRADE_FTP_H_

#include "qapi_firmware_upgrade.h"

/*
 * Firmware Upgrade FTP Status codes
 */
typedef enum {
        QCOM_FW_UPGRADE_ERR_FTP_SESSION_ALREADY_START_E = 2000,
        QCOM_FW_UPGRADE_ERR_FTP_SESSION_NOT_START_E,
        QCOM_FW_UPGRADE_ERR_FTP_GET_LOCAL_ADDRESS_E, 
        QCOM_FW_UPGRADE_ERR_FTP_CREATE_SOCKET_E,
        QCOM_FW_UPGRADE_ERR_FTP_CONTROL_SOCKET_CLOSED_E,
        QCOM_FW_UPGRADE_ERR_FTP_DATA_SOCK_CLOSED_E,        
        QCOM_FW_UPGRADE_ERR_FTP_URL_FORMAT_E,
        QCOM_FW_UPGRADE_ERR_FTP_NO_MEMORY_E,
        QCOM_FW_UPGRADE_ERR_FTP_CONNECT_FAIL_E,
        QCOM_FW_UPGRADE_ERR_FTP_BIND_FAIL_E,
        QCOM_FW_UPGRADE_ERR_FTP_PEER_CLOSED_E,
        QCOM_FW_UPGRADE_ERR_FTP_SEND_COMMAND_E,
        QCOM_FW_UPGRADE_ERR_FTP_LISTEN_DATA_PORT_E,
        QCOM_FW_UPGRADE_ERR_FTP_DATA_CONNECTION_TIMEOUT_E,
        QCOM_FW_UPGRADE_ERR_FTP_ACCEPT_DATA_CONNECT_E,
        QCOM_FW_UPGRADE_ERR_FTP_FILE_NOT_COMPLETE_E,
        QCOM_FW_UPGRADE_ERR_FTP_SYST_E,
        QCOM_FW_UPGRADE_ERR_FTP_LOGIN_INCORRECT_E,
        QCOM_FW_UPGRADE_ERR_FTP_SET_TYPE_E,
        QCOM_FW_UPGRADE_ERR_FTP_SET_PORT_E,
        QCOM_FW_UPGRADE_ERR_FTP_FILE_NOT_FOUND_E,
        QCOM_FW_UPGRADE_ERR_FTP_DIR_NOT_EXIST_E,
        QCOM_FW_UPGRADE_ERR_FTP_RESTART_NOT_SUPPORT_E,
} QCOM_FW_UPGRADE_FTP_STATUS_CODE_t;

/*****************************************************************************************************************/
qapi_Fw_Upgrade_Status_Code_t plugin_Ftp_Init(const char* interface_name, const char *url, void *init_param);
qapi_Fw_Upgrade_Status_Code_t plugin_Ftp_Fin(void);
qapi_Fw_Upgrade_Status_Code_t plugin_Ftp_Recv_Data(uint8_t *buffer, uint32_t buf_len, uint32_t *ret_size);
qapi_Fw_Upgrade_Status_Code_t plugin_Ftp_Abort(void);
qapi_Fw_Upgrade_Status_Code_t plugin_Ftp_Resume(const char* interface_name, const char *url, uint32_t offset);

#endif /*_FW_UPGRADE_FTP_H_ */
