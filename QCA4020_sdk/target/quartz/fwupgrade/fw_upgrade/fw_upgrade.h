/*
* Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
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

#ifndef _FW_UPGRADE_H
#define _FW_UPGRADE_H
#include <qapi/qapi_crypto.h>

/**********************************************************************************************************/
/* Firmware Upgrade definition                                                                            */      
/**********************************************************************************************************/
#define OM_SMEM_FW_UPGRADE_ID_SESSION_CXT       0x18
#define OM_SMEM_FW_UPGRADE_ID_IMG_HDR           0x19

#define FW_UPGRADE_BUF_SIZE                 2048
#define FW_UPGRADE_HASH_LEN                 QAPI_CRYPTO_SHA256_DIGEST_BYTES              //32B
#define FW_UPGRADE_INTERFACE_NAME_LEN       32
#define FW_UPGRADE_URL_LEN                  256
#define FW_UPGRADE_FILENAME_LEN             128
#define FW_UPGRADE_URL_TOTAL_LEN            (FW_UPGRADE_URL_LEN + FW_UPGRADE_FILENAME_LEN)
#define FW_UPGRADE_MAX_IMAGES_NUM           30
#define FW_UPGRADE_FORAMT_PARTIAL_UPGRADE   1

#define QAPI_FU_FWD_RANK_TRIAL		0xFFFFFFFF
#define QAPI_FU_FWD_RANK_GOLDEN		0x00000000
#define QAPI_FU_FWD_STATUS_VALID	0x01
#define QAPI_FU_FWD_STATUS_INVALID	0x00
#define QAPI_FU_FWD_STATUS_UNUSED	0xFF
#define QAPI_FU_FWD_TRIAL_UNUSED    0xFF
#define QAPI_FU_FWD_IMAGE_UNUSED	0xFFFFFFFF

/*************************************************************************************************************/
/* type definition                                                                                           */
/*************************************************************************************************************/

/*
 * fw_Upgrade_session_status
 */
typedef enum {
    FW_UPGRADE_SESSION_NOT_START_E = 0,
    FW_UPGRADE_SESSION_RUNNING_E,
	FW_UPGRADE_SESSION_SUSPEND_E,
    FW_UPGRADE_SESSION_CANCEL_E,
    FW_UPGRADE_SESSION_ERROR_E,    
} fw_Upgrade_Session_Status_t;

/*
 * Firmware Upgrade ImageSet Header Structure 
 */
typedef struct {
    uint32_t magic;
    uint32_t version;
    uint32_t format;
    uint32_t length;
    uint8_t  num_images;
} __attribute__ ((packed)) fw_Upgrade_ImageSet_Hdr_Part1_t;

/* 
 * Firmware Upgrade Sub Image Header Structure 
 */
typedef struct {
    uint32_t magic;
    uint32_t image_id;
    uint32_t version;
    uint8_t  image_file[FW_UPGRADE_FILENAME_LEN];
    uint32_t disk_size;
    uint32_t image_length;
    uint32_t hash_type;
    uint8_t  hash[FW_UPGRADE_HASH_LEN];
} __attribute__ ((packed)) fw_Upgrade_Image_Hdr_t;

/*
 * Data context for firmware upgrade session
 */
typedef struct {
    uint32_t error_code;         /*qapi_Fw_Upgrade_Status_Code */
    uint8_t  is_first;          

    uint32_t buf_len;           /* total available buffer length */
    uint32_t buf_offset;        /* processed buffer length */
    
    uint32_t image_index;       /* image index number */
    uint32_t image_wrt_count;   /* image flashed length */
    uint32_t image_wrt_length;  /* image total length */
    uint32_t total_images;      /* total number of images */
    uint32_t file_read_count;   /* received length from remote file */
    
    qapi_Fw_Upgrade_State_t      fw_upgrade_state;   /* fw upgrade session state */
    fw_Upgrade_Session_Status_t  fw_upgrade_session_status;   /* fw upgrade session status */
    uint8_t download_flag[FW_UPGRADE_MAX_IMAGES_NUM];   /* mark for download image or duplicate image from current */   

    char url[FW_UPGRADE_URL_LEN];         /* stored URL */
    char cfg_file[FW_UPGRADE_URL_LEN];    /* stored config filw name */
    char interface_name[FW_UPGRADE_INTERFACE_NAME_LEN];       /* store interface name */
    void *init_param;                     /* store init_param */

    uint32_t flags;
    uint32_t format;                /* 1: partial fw upgrade, 2: all-in-one fw upgrade */
    uint32_t trial_flash_start;		/* available flash start address */
    uint32_t trial_flash_size;		/* trial partition size in flash */
    uint8_t  trial_FWD_idx;			/* trial FWD number */
    qapi_Part_Hdl_t partition_hdl;
    
    qapi_Fw_Upgrade_Plugin_t plugin;
    qapi_Fw_Upgrade_CB_t     fw_upgrade_cb;
    qapi_Crypto_Op_Hdl_t     digest_ctx;        /* crypto ctx */
    uint8_t  *config_buf;    /* buffer to store config file before parse */
    
    uint32_t  data_ready_len;
    uint8_t  *data_ready_ptr;
} fw_Upgrade_Context_t;

/*************************************************************************************************************/
/*************************************************************************************************************/
/*
 * get ota session state
 */
qapi_Fw_Upgrade_State_t fw_Upgrade_Get_State(void);

/*
 * get ota session status
 */
fw_Upgrade_Session_Status_t fw_Upgrade_Get_Session_Status(void);

/*
 * set ota session status
 */
qapi_Fw_Upgrade_Status_Code_t fw_Upgrade_Set_Session_Status(fw_Upgrade_Session_Status_t status);

/*
 * start OTA upgrade session 
 */
qapi_Fw_Upgrade_Status_Code_t fw_Upgrade(char *interface_name, qapi_Fw_Upgrade_Plugin_t *plugin, char *url, char *cfg_file, uint32_t flags, qapi_Fw_Upgrade_CB_t cb, void *init_param);

/*
 * cancel OTA session
 */
qapi_Fw_Upgrade_Status_Code_t fw_Upgrade_Session_Cancel(void);

/*
 * process result of OTA session
 */
qapi_Fw_Upgrade_Status_Code_t fw_Upgrade_Session_Done(uint32_t result);

/*
 * suspend OTA session
 */
qapi_Fw_Upgrade_Status_Code_t fw_Upgrade_Session_Suspend(void);

/*
 * resume OTA session
 */
qapi_Fw_Upgrade_Status_Code_t fw_Upgrade_Session_Resume(void);

/*
 * get Firmware Upgrade Scheme Parameter from DEVCFG
 */
qapi_Fw_Upgrade_Status_Code_t fw_Upgrade_Get_Scheme_Param(uint32_t id, uint32_t *param);

/* internal API at qapi_firmware_upgrade */
qapi_Status_t qapi_Fw_Upgrade_get_Trial_Active_FWD_Index(uint8_t *trial, uint8_t *current, uint32_t *rank);

/*
 * start firmware upgrade session which triggered by host
 */
qapi_Fw_Upgrade_Status_Code_t fw_Upgrade_Host_Init(uint32_t flags);

/*
 * stop firmware upgrade session which triggered by host
 */
qapi_Fw_Upgrade_Status_Code_t fw_Upgrade_Host_Deinit(void);

/*
 * pass data to firmware upgrade session which triggered by host
 */
qapi_Fw_Upgrade_Status_Code_t fw_Upgrade_Host_Write(char *buffer, int32_t len);

/*
 * Get Firmware Upgrade Error Code
 */
qapi_Fw_Upgrade_Status_Code_t fw_Upgrade_Get_Error_Code(void);

#endif /* _FW_UPGRADE_H */
