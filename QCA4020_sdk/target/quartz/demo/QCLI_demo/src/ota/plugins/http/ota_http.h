/* Copyright (c) 2017 Qualcomm Technologies Incorporated.
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

#ifndef _OTA_HTTP_H_
#define _OTA_HTTP_H_

/*
 * Firmware Upgrade HTTP Status codes
 */
typedef enum {
        FW_UPGRADE_ERR_HTTP_SESSION_ALREADY_START_E = 3000,
        FW_UPGRADE_ERR_HTTP_SESSION_NOT_START_E,
        FW_UPGRADE_ERR_HTTP_GET_LOCAL_ADDRESS_E, 
        FW_UPGRADE_ERR_HTTP_URL_FORMAT_E,
        FW_UPGRADE_ERR_HTTP_NO_MEMORY_E,
        FW_UPGRADE_ERR_HTTP_CONNECT_FAIL_E,
        FW_UPGRADE_ERR_HTTP_SSL_CREATE_CTX_ERR_E,
        FW_UPGRADE_ERR_HTTP_SSL_CONNECT_FAIL_E,  
        FW_UPGRADE_ERR_HTTP_SSL_ADD_SOCKET_FAIL_E,
        FW_UPGRADE_ERR_HTTP_SEND_E,
        FW_UPGRADE_ERR_HTTP_FILE_NOT_COMPLETE_E,
        FW_UPGRADE_ERR_HTTP_FILE_NOT_FOUND_E,
        FW_UPGRADE_ERR_HTTP_FILE_NAME_TOO_LONG_E,
        FW_UPGRADE_ERR_HTTP_DIR_NOT_EXIST_E,  
        FW_UPGRADE_ERR_HTTP_IMAGE_DOWNLOAD_FAIL_E,        
        FW_UPGRADE_ERR_HTTP_SERVER_RSP_TIMEOUT_E,
        FW_UPGRADE_ERR_HTTP_HEADER_ERROR_E,
} FW_UPGRADE_HTTP_STATUS_CODE_t;

/*****************************************************************************************************************/
qapi_Fw_Upgrade_Status_Code_t plugin_Http_Init(const char* interface_name, const char *url, void *init_param);
qapi_Fw_Upgrade_Status_Code_t plugin_Http_Fin(void);
qapi_Fw_Upgrade_Status_Code_t plugin_Http_Recv_Data(uint8_t *buffer, uint32_t buf_len, uint32_t *ret_size);
qapi_Fw_Upgrade_Status_Code_t plugin_Http_Abort(void);
qapi_Fw_Upgrade_Status_Code_t plugin_Http_Resume(const char* interface_name, const char *url, uint32_t offset);

#endif /*_OTA_HTTP_H_ */
