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

/*****************************************************************************************************************************/
/*                                                                                                                           */
/*       Firmware Upgrade                                                                                                           */
/*                                                                                                                           */
/*****************************************************************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stringl.h>
#include <qapi/qapi_crypto.h>
#include <DALSysTypes.h>
#include <DALSys.h>
#include <DALDeviceId.h>
#include "qapi/qapi_types.h"
#include "qapi/qapi_status.h"
#include "qapi/qapi_pmu.h"
#include "qapi/qapi_firmware_upgrade.h"
#include "qapi/qapi_firmware_image_id.h"
#include "qapi/qapi_fs.h"
#include "qapi/qapi_fatal_err.h"
#include "qapi/qapi_om_smem.h"
#include "qurt_error.h"
#include "qurt_mutex.h"
#include "qurt_signal.h"
#include "qurt_thread.h"
#include "qurt_types.h"
#include "malloc.h"
#include "fw_upgrade.h"
#include "fw_upgrade_dal_id.h"


/*************************************************************************************************************/
/* Note: these difination and funcitons are internal now. They will be replaced by QAPI later                */
/*************************************************************************************************************/
#define FS_MOUNT_FLAG_CREATE_FS   1
extern int fs_mount(const char *device_name, const char *mount_dir,
	                const char *fs_name, uint32 flags, void *data);
extern int fs_umount(const char *mount_dir);

/*************************************************************************************************************/
/*************************************************************************************************************/
/**
   This definition determines the size of the stack (in bytes) that is used
   for command threads.
*/
#define THREAD_STACK_SIZE                                               (3072)

/**
*/
#define FWUP_THREAD_PRIORITY                                            18

#define TAKE_LOCK(__lock__)             ((qurt_mutex_lock_timed(&(__lock__), QURT_TIME_WAIT_FOREVER)) == QURT_EOK)
#define RELEASE_LOCK(__lock__)          do { qurt_mutex_unlock(&(__lock__)); } while(0)
	
#if defined(DEBUG_FW_UPGRADE_PRINTF)
#define FW_UPGRADE_D_PRINTF(args...) printf(args)
#else
#define FW_UPGRADE_D_PRINTF(args...)
#endif

#ifndef MIN
#define MIN( a, b ) ((a)<(b)) ? (a) : (b)
#endif

#define   FWUP_RX_DATA_DONE_SIG_MASK    			0x01
#define   FWUP_RX_DATA_ERROR_SIG_MASK              0x02
#define   FWUP_RX_DATA_FINISH_SIG_MASK             0x04
#define   FWUP_DATA_RX_ALL_SIG_MASK				(FWUP_RX_DATA_DONE_SIG_MASK | FWUP_RX_DATA_FINISH_SIG_MASK | FWUP_RX_DATA_ERROR_SIG_MASK)

#define   FWUP_BUFFER_EMPTY_SIG_MASK               0x01

#define   FWUP_THREAD_CLOSE_SIG_MASK               0x01



/*************************************************************************************************************/
/* Firmware Upgrade Globals                                                                                               */
/*************************************************************************************************************/
fw_Upgrade_Context_t   *fw_upgrade_sess_cxt  = NULL;
fw_Upgrade_Image_Hdr_t *fw_upgrade_image_hdr = NULL;     /* fw upgrade image header */
uint8 Fw_Upgrade_Mutex_Init = 0;
qurt_mutex_t    Fw_Upgrade_Mutex;
qurt_signal_t   data_ready_signal;
qurt_signal_t   data_drain_signal;
qurt_signal_t   thread_close_signal;

/*************************************************************************************************************/
/*************************************************************************************************************/
static fw_Upgrade_Context_t *fw_Upgrade_Get_Context(void);
static void fw_Upgrade_Set_Error_Code(uint32_t err_code);
static void fw_Upgrade_Update_Callback(uint32_t state, uint32_t status);
static void fw_Upgrade_Set_State(qapi_Fw_Upgrade_State_t state);
static qapi_Fw_Upgrade_Status_Code_t fw_Upgrade_Process_Config_File(uint8_t *buf);
static qapi_Fw_Upgrade_Status_Code_t fw_Upgrade_Verify_Image_Hash(fw_Upgrade_Image_Hdr_t *image_hdr);
static qapi_Fw_Upgrade_Status_Code_t fw_Upgrade_Process_Receive_Image(uint8_t *buffer);
static qapi_Fw_Upgrade_Status_Code_t fw_Upgrade_Process_Duplicate_FS(uint32_t flags);
static qapi_Fw_Upgrade_Status_Code_t fw_Upgrade_Process_Duplicate_Images(void);
static qapi_Fw_Upgrade_Status_Code_t fw_Upgrade_Session_Init(char *interface_name, qapi_Fw_Upgrade_Plugin_t *plugin, char *url, char *cfg_file, uint32_t flags, qapi_Fw_Upgrade_CB_t fw_upgrade_callback, void *init_param);
static qapi_Fw_Upgrade_Status_Code_t fw_Upgrade_Session_Fin(void);
static qapi_Fw_Upgrade_Status_Code_t fw_Upgrade_Session_Prepare_Suspend(void);
static qapi_Fw_Upgrade_Status_Code_t fw_Upgrade_Session_Process(void);
static qapi_Fw_Upgrade_Status_Code_t fw_Upgrade_Session_Finalize(qapi_Fw_Upgrade_Status_Code_t rtn);
static qapi_Fw_Upgrade_Status_Code_t fw_Upgrade_Plugin_Init(void);
static qapi_Fw_Upgrade_Status_Code_t fw_Upgrade_Plugin_Fin(void);
static qapi_Fw_Upgrade_Status_Code_t fw_Upgrade_Plugin_Recv_Data(uint8_t *buffer, uint32_t buf_len, uint32_t *ret_size);
static qapi_Fw_Upgrade_Status_Code_t fw_Upgrade_Plugin_Abort(void);
static qapi_Fw_Upgrade_Status_Code_t fw_Upgrade_Plugin_Resume(void);

/*************************************************************************************************************/
/*************************************************************************************************************/

/*
 * get fw upgrade session context
 */
static fw_Upgrade_Context_t *fw_Upgrade_Get_Context(void)
{
  return fw_upgrade_sess_cxt;
}

/*
 * fw upgrade session init
 */
static qapi_Fw_Upgrade_Status_Code_t fw_Upgrade_Session_Init(char *interface_name, qapi_Fw_Upgrade_Plugin_t *plugin, char *url, char *cfg_file, uint32_t flags, qapi_Fw_Upgrade_CB_t fw_upgrade_callback, void *init_param)
{
    qapi_Fw_Upgrade_Status_Code_t rtn = QAPI_FW_UPGRADE_OK_E;
    uint32_t url_len = 0, filename_len = 0;
    uint32_t i;
    
    if( qapi_Fw_Upgrade_init() != QAPI_OK ) {
        rtn = QAPI_FW_UPGRADE_ERR_FLASH_INIT_TIMEOUT_E;
        goto session_init_end;
    }

    if( Fw_Upgrade_Mutex_Init == 0 ) {
        qurt_mutex_init(&Fw_Upgrade_Mutex);
        qurt_signal_create(&data_ready_signal);
        qurt_signal_create(&data_drain_signal);
        qurt_signal_create(&thread_close_signal);        
        Fw_Upgrade_Mutex_Init = 1;
    }

   /* Initialize all variables */
    if( QAPI_OK != qapi_OMSM_Alloc(QAPI_OMSM_DEFAULT_AON_POOL, OM_SMEM_FW_UPGRADE_ID_SESSION_CXT, sizeof(fw_Upgrade_Context_t), (void**)&fw_upgrade_sess_cxt) ) {
        rtn = QAPI_FW_UPGRADE_ERR_INSUFFICIENT_MEMORY_E;
        goto session_init_end;
    }
    if( QAPI_OK != qapi_OMSM_Commit(QAPI_OMSM_DEFAULT_AON_POOL, OM_SMEM_FW_UPGRADE_ID_SESSION_CXT) ) {
        rtn = QAPI_FW_UPGRADE_ERR_INSUFFICIENT_MEMORY_E;
        goto session_init_end;        
    }
    
    /* Clear all data */
    memset(fw_upgrade_sess_cxt, '\0', sizeof(fw_Upgrade_Context_t));

    /* init the default setting */
    fw_Upgrade_Set_State(QAPI_FW_UPGRADE_STATE_NOT_START_E);
    fw_Upgrade_Set_Session_Status(FW_UPGRADE_SESSION_RUNNING_E);
    fw_Upgrade_Set_Error_Code(QAPI_FW_UPGRADE_OK_E);
    fw_upgrade_sess_cxt->partition_hdl = NULL;
    fw_upgrade_sess_cxt->flags = flags;
    fw_upgrade_sess_cxt->fw_upgrade_cb = (qapi_Fw_Upgrade_CB_t) fw_upgrade_callback;
    fw_upgrade_sess_cxt->init_param = init_param;
    memcpy((void *)&(fw_upgrade_sess_cxt->plugin), (void *)plugin, sizeof(qapi_Fw_Upgrade_Plugin_t) );
    if(    (fw_upgrade_sess_cxt->plugin.fw_Upgrade_Plugin_Init == NULL)
        || (fw_upgrade_sess_cxt->plugin.fw_Upgrade_Plugin_Recv_Data == NULL)
        || (fw_upgrade_sess_cxt->plugin.fw_Upgrade_Plugin_Abort == NULL)
        || (fw_upgrade_sess_cxt->plugin.fw_Upgrade_Plugin_Resume == NULL)
		|| (fw_upgrade_sess_cxt->plugin.fw_Upgrade_Plugin_Fin == NULL)   ) {
        rtn = QAPI_FW_UPGRADE_ERR_PLUGIN_ENTRY_EMPTY_E;
        goto session_init_end;
    }

    /* save interface name */
    if(strlen(interface_name) >=  FW_UPGRADE_INTERFACE_NAME_LEN) {
        rtn = QAPI_FW_UPGRADE_ERR_INTERFACE_NAME_TOO_LONG_E;
        goto session_init_end;
    }
    memset(fw_upgrade_sess_cxt->interface_name,0,FW_UPGRADE_INTERFACE_NAME_LEN);
    strncpy(fw_upgrade_sess_cxt->interface_name, interface_name,FW_UPGRADE_INTERFACE_NAME_LEN);

    url_len = strlen(url);
    filename_len = strlen(cfg_file);

    /* set url buffer to 0 */
    memset(fw_upgrade_sess_cxt->url, 0, FW_UPGRADE_URL_LEN);
    /* Add '/' to URL if it is not already added and save the URL. */
    if( url_len > 0) {
        if(url[url_len-1] != '/') {
            if(url_len + 1 >=  FW_UPGRADE_URL_LEN) {
                rtn = QAPI_FW_UPGRADE_ERR_URL_TOO_LONG_E;
                goto session_init_end;
            }
            memcpy(fw_upgrade_sess_cxt->url, url, url_len);
            fw_upgrade_sess_cxt->url[url_len] = '/';
        }
        else {
            if(url_len >=  FW_UPGRADE_URL_LEN) {
                rtn = QAPI_FW_UPGRADE_ERR_URL_TOO_LONG_E;
                goto session_init_end;
            }
            memcpy(fw_upgrade_sess_cxt->url, url, url_len);
        }
    }

    /* Save the config file name */
    if (filename_len >= FW_UPGRADE_FILENAME_LEN) {
        rtn = QAPI_FW_UPGRADE_ERR_URL_TOO_LONG_E;
        goto session_init_end;
    }
    memset(fw_upgrade_sess_cxt->cfg_file, 0, FW_UPGRADE_FILENAME_LEN);
    memcpy(fw_upgrade_sess_cxt->cfg_file, cfg_file, strlen(cfg_file));

    fw_upgrade_sess_cxt->is_first = 1;
    fw_upgrade_sess_cxt->format = 1;    /* 1: partial upgrade, 2: all-in-one */
    fw_upgrade_sess_cxt->buf_len = 0;
    fw_upgrade_sess_cxt->buf_offset = 0;
    fw_upgrade_sess_cxt->file_read_count = 0;

    fw_upgrade_sess_cxt->image_index = 0;
    fw_upgrade_sess_cxt->image_wrt_count = 0;
    fw_upgrade_sess_cxt->image_wrt_length = 0;
    fw_upgrade_sess_cxt->total_images = 0;
    fw_upgrade_sess_cxt->config_buf = NULL;
    
    for(i =0; i < FW_UPGRADE_MAX_IMAGES_NUM; i++)
        fw_upgrade_sess_cxt->download_flag[i] = 1;
    
    /* allocate crypto resource */
    fw_upgrade_sess_cxt->digest_ctx = 0;
    if (qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_SHA256_E, QAPI_CRYPTO_MODE_DIGEST_E, 0, &(fw_upgrade_sess_cxt->digest_ctx)) != QAPI_OK) {
        rtn = QAPI_FW_UPGRADE_ERR_INSUFFICIENT_MEMORY_E;
        goto session_init_end;
    }

    fw_Upgrade_Set_Session_Status(FW_UPGRADE_SESSION_RUNNING_E);
    return QAPI_FW_UPGRADE_OK_E;

session_init_end:
    fw_Upgrade_Set_Error_Code(rtn);
    fw_Upgrade_Session_Fin();
    return rtn;
}

/*
 * fw upgrade session fin
 */
static qapi_Fw_Upgrade_Status_Code_t fw_Upgrade_Session_Fin(void)
{
    qapi_OMSM_alloc_status_t alloc_status;
    uint16 buff_size;

    if (fw_upgrade_sess_cxt != NULL && fw_upgrade_sess_cxt->config_buf != NULL ) {
        free(fw_upgrade_sess_cxt->config_buf);
        fw_upgrade_sess_cxt->config_buf = NULL;
    }
    
    //check AON_FW_UPGRADE memory 
    qapi_OMSM_Check_Status(QAPI_OMSM_DEFAULT_AON_POOL, OM_SMEM_FW_UPGRADE_ID_IMG_HDR, &alloc_status);    
    if(alloc_status == QAPI_OMSM_BUF_COMMITTED_E || alloc_status == QAPI_OMSM_BUF_RETRIEVING_E) {
        if( alloc_status != QAPI_OMSM_BUF_RETRIEVING_E ) {
            qapi_OMSM_Retrieve(QAPI_OMSM_DEFAULT_AON_POOL, OM_SMEM_FW_UPGRADE_ID_IMG_HDR, &buff_size, (void**)&fw_upgrade_image_hdr);
        }
    
        qapi_OMSM_Free(QAPI_OMSM_DEFAULT_AON_POOL, OM_SMEM_FW_UPGRADE_ID_IMG_HDR);
        fw_upgrade_image_hdr = NULL;
    }
	
    qapi_OMSM_Check_Status(QAPI_OMSM_DEFAULT_AON_POOL, OM_SMEM_FW_UPGRADE_ID_SESSION_CXT, &alloc_status);    
    if(alloc_status == QAPI_OMSM_BUF_COMMITTED_E || alloc_status == QAPI_OMSM_BUF_RETRIEVING_E) {
        if (fw_upgrade_sess_cxt) {
            if( fw_upgrade_sess_cxt->partition_hdl != NULL ) {
                qapi_Fw_Upgrade_Close_Partition(fw_upgrade_sess_cxt->partition_hdl);
                fw_upgrade_sess_cxt->partition_hdl = NULL;
            }

            //free crypto
            if( fw_upgrade_sess_cxt->digest_ctx != 0 ) {
                qapi_Crypto_Op_Free(fw_upgrade_sess_cxt->digest_ctx);
                fw_upgrade_sess_cxt->digest_ctx = 0;
            }   
        }
        if( alloc_status != QAPI_OMSM_BUF_RETRIEVING_E ) {
            qapi_OMSM_Retrieve(QAPI_OMSM_DEFAULT_AON_POOL, OM_SMEM_FW_UPGRADE_ID_SESSION_CXT, &buff_size, (void**)&fw_upgrade_sess_cxt);
        }
    
        qapi_OMSM_Free(QAPI_OMSM_DEFAULT_AON_POOL, OM_SMEM_FW_UPGRADE_ID_SESSION_CXT);
        fw_upgrade_sess_cxt = NULL;
    }

    if( Fw_Upgrade_Mutex_Init != 0 ) {
        qurt_signal_set(&data_drain_signal, FWUP_BUFFER_EMPTY_SIG_MASK);
        qurt_mutex_destroy(&Fw_Upgrade_Mutex);
        qurt_signal_delete(&data_ready_signal);
        qurt_signal_delete(&data_drain_signal);
        qurt_signal_delete(&thread_close_signal);
        Fw_Upgrade_Mutex_Init = 0;   
    }
    
    return QAPI_FW_UPGRADE_OK_E;
}

/*
 * fw upgrade session suspend
 */
static qapi_Fw_Upgrade_Status_Code_t fw_Upgrade_Session_Prepare_Suspend(void)
{
    if (fw_upgrade_sess_cxt) {
        if( fw_upgrade_sess_cxt->partition_hdl != NULL ) {
            qapi_Fw_Upgrade_Close_Partition(fw_upgrade_sess_cxt->partition_hdl);
            fw_upgrade_sess_cxt->partition_hdl = NULL;
        }

        //free crypto
        if( fw_upgrade_sess_cxt->digest_ctx != 0 ) {
            qapi_Crypto_Op_Free(fw_upgrade_sess_cxt->digest_ctx);
            fw_upgrade_sess_cxt->digest_ctx = 0;
        }
    }
    return QAPI_FW_UPGRADE_OK_E;
}

/*
 * process fw upgrade session
 */
static qapi_Fw_Upgrade_Status_Code_t fw_Upgrade_Session_Process(void)
{
    qapi_Fw_Upgrade_Status_Code_t rtn = QAPI_FW_UPGRADE_OK_E;
    fw_Upgrade_Context_t *fw_upgrade_cxt;
    uint8_t *buffer = NULL;
    uint8_t  run = 1;
    uint32_t received, param;

    //check battery level if need
	if( fw_Upgrade_Get_Scheme_Param(FW_UPGRADE_SCHEME_PROP_BATTERY_CHECK_ID, &param ) == QAPI_FW_UPGRADE_OK_E && param != 0 ) {
		//get battery good level ref
		if( fw_Upgrade_Get_Scheme_Param(FW_UPGRADE_SCHEME_PROP_BATTERY_REF_LEVEL_ID, &param) == QAPI_FW_UPGRADE_OK_E ) {
			qapi_PM_Vbatt_Status_Type_t  Vbatt_level_Status = QAPI_PM_VBATT_BELOW_TREHOLD_E;
 
            // check the battery level
			if(qapi_PM_Vbatt_Level_Good( param,  &Vbatt_level_Status) == QAPI_OK) {
				if(Vbatt_level_Status == QAPI_PM_VBATT_ABOVE_TREHOLD_E) {
					//Battery level is above desired_Vbatt_Threshold
				}
				else if (Vbatt_level_Status == QAPI_PM_VBATT_BELOW_TREHOLD_E)
				{
					//Battery level is below desired_Vbatt_Threshold
					return QAPI_FW_UPGRADE_ERR_BATTERY_LEVEL_TOO_LOW_E;
				}
			}
		}
	}		

    /* get fw upgrade context */
    fw_upgrade_cxt = fw_Upgrade_Get_Context();
    if( fw_upgrade_cxt == NULL ) {
        return QAPI_FW_UPGRADE_ERR_SESSION_NOT_START_E;
    }
    
    /*Allocate buffer*/
    if((buffer = malloc(FW_UPGRADE_BUF_SIZE)) == NULL) {
        FW_UPGRADE_D_PRINTF("Out of memory error\r\n");
        return QAPI_FW_UPGRADE_ERR_INSUFFICIENT_MEMORY_E;
    }

    while( (run == 1) && (fw_Upgrade_Get_Session_Status() == FW_UPGRADE_SESSION_RUNNING_E) )
    {
        switch(fw_Upgrade_Get_State())
        {
            case QAPI_FW_UPGRADE_STATE_NOT_START_E:
                fw_Upgrade_Update_Callback(fw_Upgrade_Get_State(), fw_Upgrade_Get_Error_Code());
                fw_Upgrade_Set_State(QAPI_FW_UPGRADE_STATE_GET_TRIAL_INFO_E);
                break;

            case QAPI_FW_UPGRADE_STATE_GET_TRIAL_INFO_E:
                fw_Upgrade_Update_Callback(fw_Upgrade_Get_State(), fw_Upgrade_Get_Error_Code());
                //locate available partition for trial FWD
                if( qapi_Fw_Upgrade_Select_Trial_FWD(&(fw_upgrade_cxt->trial_FWD_idx), &(fw_upgrade_cxt->trial_flash_start), &(fw_upgrade_cxt->trial_flash_size)) != QAPI_OK ) {
                    rtn =  QAPI_FW_UPGRADE_ERR_FLASH_NOT_SUPPORT_FW_UPGRADE_E;
                    run = 0;
                    break;
                }

                if( qapi_Fw_Upgrade_Get_Active_FWD(NULL, NULL) == fw_upgrade_cxt->trial_FWD_idx ) {
                    //if current running FWD is trial FWD, just reject the request
                    rtn =  QAPI_FW_UPGRADE_ERR_TRIAL_IS_RUNNING_E;
                    run = 0;
                    break;
                }

                fw_Upgrade_Set_State(QAPI_FW_UPGRADE_STATE_ERASE_FWD_E);
                break;

            case QAPI_FW_UPGRADE_STATE_ERASE_FWD_E:
                fw_Upgrade_Update_Callback(fw_Upgrade_Get_State(), fw_Upgrade_Get_Error_Code());
                //erase trial FWD
                if( qapi_Fw_Upgrade_Erase_FWD(fw_upgrade_cxt->trial_FWD_idx) != QAPI_OK ) {
                    rtn = QAPI_FW_UPGRADE_ERR_FLASH_ERASE_FAIL_E;
                    run = 0;
                    break;
                }
                
                fw_Upgrade_Set_State(QAPI_FW_UPGRADE_STATE_ERASE_SECOND_FS_E);
                break;

            case  QAPI_FW_UPGRADE_STATE_ERASE_SECOND_FS_E:
            {
                uint32_t disk_size;
                
                fw_Upgrade_Update_Callback(fw_Upgrade_Get_State(), fw_Upgrade_Get_Error_Code());
                                
                if( qapi_Fw_Upgrade_Find_Partition(qapi_Fw_Upgrade_Get_Active_FWD(NULL, NULL), FS2_IMG_ID, &fw_upgrade_cxt->partition_hdl) != QAPI_OK ) {
                    rtn = QAPI_FW_UPGRADE_ERR_FLASH_IMAGE_NOT_FOUND_E;
                    run = 0;
                    break;
                }
 
                qapi_Fw_Upgrade_Get_Partition_Size(fw_upgrade_cxt->partition_hdl, &disk_size); 
                
                // erase flash where to store the second FS 
                if( qapi_Fw_Upgrade_Erase_Partition(fw_upgrade_cxt->partition_hdl, 0, disk_size) != QAPI_OK ) {
                    rtn = QAPI_FW_UPGRADE_ERR_FLASH_ERASE_PARTITION_E;
                    run = 0;
                    break;
                }                
                qapi_Fw_Upgrade_Close_Partition(fw_upgrade_cxt->partition_hdl);
                fw_upgrade_cxt->partition_hdl = NULL;

                fw_Upgrade_Set_State(QAPI_FW_UPGRADE_STATE_PREPARE_FS_E);
                break;
            }

            case  QAPI_FW_UPGRADE_STATE_PREPARE_FS_E:
            {
                uint32_t disk_size = 0, disk_start = 0, version = 0;
                
                fw_Upgrade_Update_Callback(fw_Upgrade_Get_State(), fw_Upgrade_Get_Error_Code());
                                
                if( qapi_Fw_Upgrade_Find_Partition(qapi_Fw_Upgrade_Get_Active_FWD(NULL, NULL), FS1_IMG_ID, &fw_upgrade_cxt->partition_hdl) != QAPI_OK ) {
                    rtn = QAPI_FW_UPGRADE_ERR_FLASH_IMAGE_NOT_FOUND_E;
                    run = 0;
                    break;
                }
 
                qapi_Fw_Upgrade_Get_Partition_Size(fw_upgrade_cxt->partition_hdl, &disk_size); 
                qapi_Fw_Upgrade_Get_Partition_Start(fw_upgrade_cxt->partition_hdl, &disk_start);
                qapi_Fw_Upgrade_Get_Image_Version(fw_upgrade_cxt->partition_hdl, &version);
                qapi_Fw_Upgrade_Close_Partition(fw_upgrade_cxt->partition_hdl);
                fw_upgrade_cxt->partition_hdl = NULL;
                
                // create image for trial image's FS2
                if( qapi_Fw_Upgrade_Create_Partition(fw_upgrade_cxt->trial_FWD_idx, FS2_IMG_ID, version, disk_start, disk_size, &fw_upgrade_cxt->partition_hdl) != QAPI_OK ) {
                    rtn = QAPI_FW_UPGRADE_ERR_FLASH_CREATE_PARTITION_E;
                    run = 0;
                    break;
                }
                qapi_Fw_Upgrade_Close_Partition(fw_upgrade_cxt->partition_hdl);
                fw_upgrade_cxt->partition_hdl = NULL;

                fw_Upgrade_Set_State(QAPI_FW_UPGRADE_STATE_PREPARE_CONNECT_E);
                break;
            }
            
            case QAPI_FW_UPGRADE_STATE_PREPARE_CONNECT_E:
                /* check if received config file already */
                if(fw_upgrade_cxt->is_first == 0) {
                    fw_Upgrade_Image_Hdr_t *img_hdr;
                    uint32_t disk_size, disk_start;
                    
                    for(; fw_upgrade_cxt->image_index < fw_upgrade_cxt->total_images; fw_upgrade_cxt->image_index++ )
                    {
                        if(fw_upgrade_cxt->download_flag[fw_upgrade_cxt->image_index] != 0)
                            break;
                    }

                    //this is special case for FS1 IMG and remote file size is 0
                    img_hdr = fw_upgrade_image_hdr;
                    img_hdr += fw_upgrade_cxt->image_index;

                    if((img_hdr->image_id == FS1_IMG_ID) && (img_hdr->image_length == 0) ) {
                        if( qapi_Fw_Upgrade_Find_Partition(qapi_Fw_Upgrade_Get_Active_FWD(NULL, NULL), FS2_IMG_ID, &fw_upgrade_cxt->partition_hdl) != QAPI_OK ) {
                            rtn = QAPI_FW_UPGRADE_ERR_FLASH_IMAGE_NOT_FOUND_E;
                            run = 0;
                            break;
                        }
 
                        qapi_Fw_Upgrade_Get_Partition_Size(fw_upgrade_cxt->partition_hdl, &disk_size); 
                        qapi_Fw_Upgrade_Get_Partition_Start(fw_upgrade_cxt->partition_hdl, &disk_start);
                        qapi_Fw_Upgrade_Close_Partition(fw_upgrade_cxt->partition_hdl);
                        fw_upgrade_cxt->partition_hdl = NULL;
                
                        //File system disk size is pre-set when first time download
                        //Firmware Upgrade can't change size other than original size  
                        if( img_hdr->disk_size > disk_size ) {
                            rtn = QAPI_FW_UPGRADE_ERR_INCORRECT_IMAGE_LENGTH_E;
                            run = 0;
                            break;
                        }
                
                        // create image for trial image's FS2
                        if( qapi_Fw_Upgrade_Create_Partition(fw_upgrade_cxt->trial_FWD_idx, img_hdr->image_id, img_hdr->version, disk_start, disk_size, &fw_upgrade_cxt->partition_hdl) != QAPI_OK ) {
                            rtn = QAPI_FW_UPGRADE_ERR_FLASH_CREATE_PARTITION_E;
                            run = 0;
                            break;
                        }
                        qapi_Fw_Upgrade_Close_Partition(fw_upgrade_cxt->partition_hdl);
                        fw_upgrade_cxt->partition_hdl = NULL;
                        fw_upgrade_cxt->image_index++;
                        fw_Upgrade_Set_State(QAPI_FW_UPGRADE_STATE_PREPARE_CONNECT_E); 
                        break;
                    }

                    if( fw_upgrade_cxt->image_index >= fw_upgrade_cxt->total_images) {
                        fw_Upgrade_Set_State(QAPI_FW_UPGRADE_STATE_DUPLICATE_IMAGES_E);
                    } else {
                        fw_Upgrade_Set_State(QAPI_FW_UPGRADE_STATE_CONNECT_SERVER_E);
                    }
                } else {   
                    fw_Upgrade_Set_State(QAPI_FW_UPGRADE_STATE_CONNECT_SERVER_E);    
                }
                break;
                
            case QAPI_FW_UPGRADE_STATE_CONNECT_SERVER_E:
                fw_Upgrade_Update_Callback(fw_Upgrade_Get_State(), fw_Upgrade_Get_Error_Code());
                //init FW Upgrade Plugin
                if( (rtn = fw_Upgrade_Plugin_Init()) != QAPI_FW_UPGRADE_OK_E ) {
                    run = 0;
                    break;
                }
                fw_Upgrade_Set_State(QAPI_FW_UPGRADE_STATE_RECEIVE_DATA_E);
                break;

            case QAPI_FW_UPGRADE_STATE_RESUME_SERVICE_E:
            {
                uint32_t offset, len, nbytes, total;
                fw_Upgrade_Image_Hdr_t *img_hdr;

                fw_Upgrade_Update_Callback(fw_Upgrade_Get_State(), fw_Upgrade_Get_Error_Code());

                /* get fw upgrade session context */
                fw_upgrade_cxt = fw_Upgrade_Get_Context();
                if( fw_upgrade_cxt == NULL ) {
                    rtn = QAPI_FW_UPGRADE_ERR_SESSION_NOT_START_E;
                    run = 0;
                    break;
                }

                img_hdr = fw_upgrade_image_hdr;
                img_hdr += fw_upgrade_cxt->image_index;

                //open partition handle
                if( qapi_Fw_Upgrade_Find_Partition(fw_upgrade_cxt->trial_FWD_idx, img_hdr->image_id, &fw_upgrade_cxt->partition_hdl) != QAPI_OK ) {
                    rtn = QAPI_FW_UPGRADE_ERR_FLASH_IMAGE_NOT_FOUND_E;
                    run = 0;
                    break;
                }

                //re-calculate hash
                //allocate crypto resource
                //if( fw_upgrade_cxt->digest_ctx != 0 )
                //	qapi_Crypto_Op_Free(fw_upgrade_sess_cxt->digest_ctx);

                if (qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_SHA256_E, QAPI_CRYPTO_MODE_DIGEST_E, 0, &(fw_upgrade_sess_cxt->digest_ctx)) != QAPI_OK) {
                    rtn = QAPI_FW_UPGRADE_ERR_INSUFFICIENT_MEMORY_E;
                    run = 0;
                    break;
                }
                //init crypto
                if (qapi_Crypto_Op_Reset(fw_upgrade_cxt->digest_ctx) != QAPI_OK) {
                    rtn = QAPI_FW_UPGRADE_ERR_CRYPTO_FAIL_E;
                    run = 0;
                    break;
                }

                offset = 0;
                len = 0;
                total = 0;

                //calculate fw upgrade image HASH
                while( total < fw_upgrade_cxt->image_wrt_count )
                {
                    if( (fw_upgrade_cxt->image_wrt_count- total) > FW_UPGRADE_BUF_SIZE) {
                        len = FW_UPGRADE_BUF_SIZE;
                    } else {
                        len = fw_upgrade_cxt->image_wrt_count- total;
                    }
                    if( qapi_Fw_Upgrade_Read_Partition(fw_upgrade_cxt->partition_hdl, offset, (char *)buffer, len, &nbytes) != QAPI_OK) {
                        rtn = QAPI_FW_UPGRADE_ERR_FLASH_READ_FAIL_E;
                        run = 0;
                        break;
                    }

                    qapi_Crypto_Op_Digest_Update(fw_upgrade_cxt->digest_ctx, (uint8_t *)buffer, nbytes);
                    offset += nbytes;
                    total += nbytes;
                }

                //erase image if need


               	fw_Upgrade_Set_State(QAPI_FW_UPGRADE_STATE_RESUME_SERVER_E);
               	break;
            }
            case QAPI_FW_UPGRADE_STATE_RESUME_SERVER_E:
                fw_Upgrade_Update_Callback(fw_Upgrade_Get_State(), fw_Upgrade_Get_Error_Code());
                //call FW Upgrade Plugin resume
                if( (rtn = fw_Upgrade_Plugin_Resume()) != QAPI_FW_UPGRADE_OK_E ) {
                	run = 0;
                	break;
                }
                fw_Upgrade_Set_State(QAPI_FW_UPGRADE_STATE_RECEIVE_DATA_E);
                break;

            case QAPI_FW_UPGRADE_STATE_RECEIVE_DATA_E:
                fw_Upgrade_Update_Callback(fw_Upgrade_Get_State(), fw_Upgrade_Get_Error_Code());
                /* Receiving data from FTP server.*/
                rtn = fw_Upgrade_Plugin_Recv_Data((uint8_t *)buffer, FW_UPGRADE_BUF_SIZE, &received);
                if( (rtn == QAPI_FW_UPGRADE_OK_E) && (received > 0) ) {
                    /* handle data */
                    fw_upgrade_cxt->buf_len = received;
                    fw_upgrade_cxt->buf_offset = 0;
                    
                    if(fw_upgrade_cxt->is_first == 1) {
                        fw_Upgrade_Set_State(QAPI_FW_UPGRADE_STATE_PROCESS_CONFIG_FILE_E);
                    } else {
                        fw_Upgrade_Set_State(QAPI_FW_UPGRADE_STATE_PROCESS_IMAGE_E);
                    }
                } else if( (rtn == QAPI_FW_UPGRADE_OK_E) && (received == 0) ) {
                    //no more data
                    run = 0;
                } else if( rtn != QAPI_FW_UPGRADE_OK_E ) {
                    // can't get data
                    run = 0;
                }
                break;

            case QAPI_FW_UPGRADE_STATE_PROCESS_CONFIG_FILE_E:
                fw_Upgrade_Update_Callback(fw_Upgrade_Get_State(), fw_Upgrade_Get_Error_Code());
                /* parse fw upgrade image Header */
                if( (rtn = fw_Upgrade_Process_Config_File(buffer)) != QAPI_FW_UPGRADE_OK_E ) {
                    fw_Upgrade_Plugin_Abort();
                    run = 0;
                }
                break;
                
            case  QAPI_FW_UPGRADE_STATE_PROCESS_IMAGE_E:
                fw_Upgrade_Update_Callback(fw_Upgrade_Get_State(), fw_Upgrade_Get_Error_Code());
                if( (rtn = fw_Upgrade_Process_Receive_Image(buffer)) != QAPI_FW_UPGRADE_OK_E ) {
                    fw_Upgrade_Plugin_Abort();
                    run = 0;
                } else if(  fw_Upgrade_Get_State() == QAPI_FW_UPGRADE_STATE_PROCESS_IMAGE_E){
                    //continue receiving data
                    fw_Upgrade_Set_State(QAPI_FW_UPGRADE_STATE_RECEIVE_DATA_E);
                }
                break;

            case QAPI_FW_UPGRADE_STATE_DISCONNECT_SERVER_E:
                fw_Upgrade_Update_Callback(fw_Upgrade_Get_State(), fw_Upgrade_Get_Error_Code());
                fw_Upgrade_Plugin_Fin();
                fw_Upgrade_Set_State(QAPI_FW_UPGRADE_STATE_PREPARE_CONNECT_E);
                break;
                
            case QAPI_FW_UPGRADE_STATE_DUPLICATE_IMAGES_E:
                fw_Upgrade_Update_Callback(fw_Upgrade_Get_State(), fw_Upgrade_Get_Error_Code());
                 //duplicate images from current to trial if need
                if( (rtn = fw_Upgrade_Process_Duplicate_Images()) != QAPI_FW_UPGRADE_OK_E ) {
                    run = 0;
                    break;
                }
                fw_Upgrade_Set_State(QAPI_FW_UPGRADE_STATE_DUPLICATE_FS_E);
                break;
                
            case QAPI_FW_UPGRADE_STATE_DUPLICATE_FS_E:
                fw_Upgrade_Update_Callback(fw_Upgrade_Get_State(), fw_Upgrade_Get_Error_Code());
                //check duplicate FS flag
                if(fw_upgrade_cxt->flags & QAPI_FW_UPGRADE_FLAG_DUPLICATE_ACTIVE_FS ) {
                    //copy files from FS1 to FS2
                    if( (rtn = fw_Upgrade_Process_Duplicate_FS(fw_upgrade_cxt->flags)) != QAPI_FW_UPGRADE_OK_E ) {
                        run = 0;
                        break;
                    }
                }
            
                fw_Upgrade_Set_State(QAPI_FW_UPGRADE_STATE_FINISH_E);
                break;
            case QAPI_FW_UPGRADE_STATE_FINISH_E:
                fw_Upgrade_Update_Callback(fw_Upgrade_Get_State(), fw_Upgrade_Get_Error_Code());
                run = 0;
                break;
            default:
                break;
        }  //switch(...

    }  //while(...

    /* free bufer */
    if(buffer) {
    	free(buffer);
    }
    
    //set error code
    if( fw_Upgrade_Get_Session_Status() == FW_UPGRADE_SESSION_CANCEL_E ) {
        fw_Upgrade_Set_Error_Code(QAPI_FW_UPGRADE_ERR_SESSION_CANCELLED_E);
    } else {
        fw_Upgrade_Set_Error_Code(rtn);
    }
    return (rtn);
}

/*
 * finalize fw upgrade session
 */
static qapi_Fw_Upgrade_Status_Code_t fw_Upgrade_Session_Finalize(qapi_Fw_Upgrade_Status_Code_t rtn)
{
    fw_Upgrade_Context_t *fw_upgrade_cxt;

    /* get fw upgrade session context */
    fw_upgrade_cxt = fw_Upgrade_Get_Context();
    if( fw_upgrade_cxt == NULL ) {
        return QAPI_FW_UPGRADE_ERR_SESSION_NOT_START_E;
    }

    //call plugin fin
    fw_Upgrade_Plugin_Fin();

    //check if this session is cancelled or set to suspend
    if( fw_Upgrade_Get_Session_Status() == FW_UPGRADE_SESSION_CANCEL_E ) {
    	rtn = QAPI_FW_UPGRADE_ERR_SESSION_CANCELLED_E;
    } else if( (rtn == QAPI_FW_UPGRADE_OK_E) && (fw_Upgrade_Get_State() != QAPI_FW_UPGRADE_STATE_FINISH_E) ) {
        rtn = QAPI_FW_UPGRADE_ERR_INCOMPLETE_E;
    }

    //set final error code
    fw_Upgrade_Set_Error_Code(rtn);
    //update state and err code
    fw_Upgrade_Update_Callback(fw_Upgrade_Get_State(), fw_Upgrade_Get_Error_Code());

    /* everything is good */
    if( (rtn == QAPI_FW_UPGRADE_OK_E) && (fw_Upgrade_Get_State() == QAPI_FW_UPGRADE_STATE_FINISH_E) ) {
        qapi_Fw_Upgrade_Set_FWD_Status(fw_upgrade_cxt->trial_FWD_idx, QAPI_FU_FWD_STATUS_VALID);

        //check AUTO_REBOOT flag
        if(fw_upgrade_cxt->flags & QAPI_FW_UPGRADE_FLAG_AUTO_REBOOT ) {
            fw_Upgrade_Session_Fin();
            //reboot system here
            qapi_Fw_Upgrade_Reboot_System();
        }
    }

    if( fw_Upgrade_Get_Session_Status() == FW_UPGRADE_SESSION_SUSPEND_E ) {
    	fw_Upgrade_Session_Prepare_Suspend();
    	rtn = QAPI_FW_UPGRADE_ERR_SESSION_SUSPEND_E;
	} else {
    	fw_Upgrade_Session_Fin();
    }
    return rtn;
}

/*
 * set fw upgrade session error code
 */
static void fw_Upgrade_Set_Error_Code(uint32_t err_code)
{
    fw_Upgrade_Context_t *fw_upgrade_cxt = fw_Upgrade_Get_Context();
    if( fw_upgrade_cxt != NULL ) {
      fw_upgrade_cxt->error_code = err_code;
    }
}

/*
 * call fw upgrade callback
 */
static void fw_Upgrade_Update_Callback(uint32_t state, uint32_t status)
{
    fw_Upgrade_Context_t *fw_upgrade_cxt = fw_Upgrade_Get_Context();
    if( (fw_upgrade_cxt != NULL) &&(fw_upgrade_cxt->fw_upgrade_cb != NULL) ) {
        fw_upgrade_cxt->fw_upgrade_cb(state, status);
    }
}

/*
 * set fw upgrade session state
 */
static void fw_Upgrade_Set_State(qapi_Fw_Upgrade_State_t state)
{
    fw_Upgrade_Context_t *fw_upgrade_cxt;
    if( TAKE_LOCK(Fw_Upgrade_Mutex) ) {
        /* get fw upgrade session context */
        fw_upgrade_cxt = fw_Upgrade_Get_Context();
    
        if( fw_upgrade_cxt == NULL) {
            QAPI_FATAL_ERR(0,0,0);
        }

        if( fw_upgrade_cxt != NULL ) {
            fw_upgrade_cxt->fw_upgrade_state = state;
        }
        RELEASE_LOCK(Fw_Upgrade_Mutex);    
    }
}

/*
 * process fw upgrade conifg file
 * 
 * Partial Upgrade Flow:
 *     receive whole config file
 *     check if fields are valid at config file header
 *     calc hash of config file and compare the result with hash field at config file
 *     save image entries to AON
 *     check if fields are valid at each image entry
 *     calc hash at current image and determine if the image need to be downloaded.
 *     image will only be downloaded if the hash of the new image is different from the hash of the current image.
 *
 * All-in-one Upgrade Flow:
 *     receive imageset header
 *     check if fields are valid at config file header
 *     calc hash of config file and compare the result with hash field at header
 *     save image entries to AON
 *     check if fields are valid at each image entry
 *
 * Firmware Upgrade Image HEADER format: 
      uint32 sig          
      uint32 ver
      uint32 format      
      uint32 image_len    
      uint8  num_images   
      IMG_ENTRY
          ....
      IMG_ENTRY
      uint8 HASH[32]
*/
static qapi_Fw_Upgrade_Status_Code_t fw_Upgrade_Process_Config_File(uint8_t *buf)
{
    qapi_Fw_Upgrade_Status_Code_t rtn = QAPI_FW_UPGRADE_OK_E;
    uint32_t i, len, offset, block_size, disk_size, nbytes;
    uint8_t  hash[FW_UPGRADE_HASH_LEN], *hash_buf=NULL;
    uint8_t  active_fwd;
    fw_Upgrade_ImageSet_Hdr_Part1_t *imgset_hdr;
    fw_Upgrade_Context_t     *fw_upgrade_cxt;
    fw_Upgrade_Image_Hdr_t   *img_hdr;
    qapi_Part_Hdl_t hdl;

    fw_upgrade_cxt = fw_Upgrade_Get_Context();
    if( fw_upgrade_cxt == NULL ) {
      return QAPI_FW_UPGRADE_ERR_SESSION_NOT_START_E;     
    }

    //received first buffer for config file 
    if( fw_upgrade_cxt->config_buf == NULL ) {
        if(fw_upgrade_cxt->buf_len < sizeof(fw_Upgrade_ImageSet_Hdr_Part1_t) ) {
            rtn = QAPI_FW_UPGRADE_ERR_INCORRECT_IMAGE_HDR_E;
            goto parse_img_hdr_end;  
        }

        //get firmware upgrade image header part1
        imgset_hdr =  (fw_Upgrade_ImageSet_Hdr_Part1_t *) buf;
        
        //check total images
        if( !( imgset_hdr->num_images > 0 && imgset_hdr->num_images <= FW_UPGRADE_MAX_IMAGES_NUM) ) {
            FW_UPGRADE_D_PRINTF("num of firmware upgrade images are not correct\r\n");
            rtn = QAPI_FW_UPGRADE_ERR_INCORRECT_IMAGE_HDR_E;
            goto parse_img_hdr_end;  
        }
        //check header
        if( (imgset_hdr->magic == 0) || (imgset_hdr->length == 0) || (imgset_hdr->format == 0) ) {
            FW_UPGRADE_D_PRINTF("firmware upgrade image signature is not correct\r\n");
            rtn = QAPI_FW_UPGRADE_ERR_INCORRECT_IMAGE_HDR_E;
            goto parse_img_hdr_end;  
        }
                
        //save fw_upgrade format -- partial upgrade or all in one
        fw_upgrade_cxt->format = imgset_hdr->format;
        
        len = imgset_hdr->length;
        if( len != (sizeof(fw_Upgrade_ImageSet_Hdr_Part1_t) + imgset_hdr->num_images * sizeof(fw_Upgrade_Image_Hdr_t) + FW_UPGRADE_HASH_LEN) ) {
            FW_UPGRADE_D_PRINTF("hdr length is not not correct\r\n");
            rtn = QAPI_FW_UPGRADE_ERR_INCORRECT_IMAGE_HDR_E;
            goto parse_img_hdr_end;                 
        }    
        
        fw_upgrade_cxt->config_buf = (uint8_t *) malloc(len);
        if(fw_upgrade_cxt->config_buf == NULL ) {
            rtn = QAPI_FW_UPGRADE_ERR_INSUFFICIENT_MEMORY_E;
            goto parse_img_hdr_end;             
        }

        fw_upgrade_cxt->buf_offset = MIN(fw_upgrade_cxt->buf_len, len);
        memcpy(fw_upgrade_cxt->config_buf, buf,  fw_upgrade_cxt->buf_offset);
        
        //don't receive whole config file at this packet yet
        if( fw_upgrade_cxt->buf_len < len ) {
            fw_upgrade_cxt->image_wrt_count = fw_upgrade_cxt->buf_len;
            //continue receiving data
            fw_Upgrade_Set_State(QAPI_FW_UPGRADE_STATE_RECEIVE_DATA_E);
            goto parse_img_hdr_end;             
        }
    } else {
        imgset_hdr =  (fw_Upgrade_ImageSet_Hdr_Part1_t *) fw_upgrade_cxt->config_buf;
        len = MIN(fw_upgrade_cxt->buf_len, imgset_hdr->length - fw_upgrade_cxt->image_wrt_count);
        memcpy(fw_upgrade_cxt->config_buf+fw_upgrade_cxt->image_wrt_count, buf, len);
        fw_upgrade_cxt->image_wrt_count += len;
        fw_upgrade_cxt->buf_offset = len;
        if( fw_upgrade_cxt->image_wrt_count < imgset_hdr->length ) {
            //continue receiving data
            fw_Upgrade_Set_State(QAPI_FW_UPGRADE_STATE_RECEIVE_DATA_E);
            goto parse_img_hdr_end;             
        }
    }
    
    //get firmware upgrade image header part1
    imgset_hdr =  (fw_Upgrade_ImageSet_Hdr_Part1_t *) fw_upgrade_cxt->config_buf;
    
    /* calc hash offset */
    offset = sizeof(fw_Upgrade_ImageSet_Hdr_Part1_t) + imgset_hdr->num_images * sizeof(fw_Upgrade_Image_Hdr_t); 

    //init crypto 
    if (qapi_Crypto_Op_Reset(fw_upgrade_cxt->digest_ctx) != QAPI_OK) {
        rtn = QAPI_FW_UPGRADE_ERR_CRYPTO_FAIL_E;
        goto parse_img_hdr_end;     
    }
    //calculate firmware upgrade image header HASH
    qapi_Crypto_Op_Digest_Update(fw_upgrade_cxt->digest_ctx, (uint8_t *)imgset_hdr, offset);
    len = FW_UPGRADE_HASH_LEN;
    qapi_Crypto_Op_Digest_Final(fw_upgrade_cxt->digest_ctx, NULL, 0, (uint8_t *)hash, &len );

    //compare firmware upgrade image Header HASH
    if( memcmp(fw_upgrade_cxt->config_buf+offset, hash, FW_UPGRADE_HASH_LEN) != 0 ) {
        FW_UPGRADE_D_PRINTF("HASH is incorrect\r\n");
        rtn = QAPI_FW_UPGRADE_ERR_INCORRECT_IMAGE_CHECKSUM_E;
        goto parse_img_hdr_end;  
    }

    //write magic number
    if( qapi_Fw_Upgrade_Set_FWD_Magic(fw_upgrade_cxt->trial_FWD_idx, imgset_hdr->magic) != QAPI_OK ) {
        rtn = QAPI_FW_UPGRADE_ERR_FLASH_WRITE_FAIL_E;
        goto parse_img_hdr_end;  
    }
    
    //write version
    if( qapi_Fw_Upgrade_Set_FWD_Version(fw_upgrade_cxt->trial_FWD_idx, imgset_hdr->version) != QAPI_OK ) {
        rtn = QAPI_FW_UPGRADE_ERR_FLASH_WRITE_FAIL_E;
        goto parse_img_hdr_end;  
    }
      
    //write num of images
    if( qapi_Fw_Upgrade_Set_FWD_Total_Images(fw_upgrade_cxt->trial_FWD_idx, imgset_hdr->num_images+1) != QAPI_OK ) {
        rtn = QAPI_FW_UPGRADE_ERR_FLASH_WRITE_FAIL_E;
        goto parse_img_hdr_end;  
    }

    //save total images
    fw_upgrade_cxt->total_images = imgset_hdr->num_images;
    
    /*Allocate buffer*/
    len = imgset_hdr->num_images * sizeof(fw_Upgrade_Image_Hdr_t);
	
	//allocate AON_MEM for image_hdr
	{
		qapi_OMSM_alloc_status_t alloc_status;
		uint16 buff_size;

		//check AON_FW_UPGRADE memory 
		qapi_OMSM_Check_Status(QAPI_OMSM_DEFAULT_AON_POOL, OM_SMEM_FW_UPGRADE_ID_IMG_HDR, &alloc_status);    
		if(alloc_status == QAPI_OMSM_BUF_COMMITTED_E || alloc_status == QAPI_OMSM_BUF_RETRIEVING_E) {
			if( alloc_status != QAPI_OMSM_BUF_RETRIEVING_E ) {
				qapi_OMSM_Retrieve(QAPI_OMSM_DEFAULT_AON_POOL, OM_SMEM_FW_UPGRADE_ID_IMG_HDR, &buff_size, (void**)&fw_upgrade_image_hdr);
			}
    
			qapi_OMSM_Free(QAPI_OMSM_DEFAULT_AON_POOL, OM_SMEM_FW_UPGRADE_ID_IMG_HDR);
			fw_upgrade_image_hdr = NULL;
		}
    
		/* allocate aon_mem */
		if( QAPI_OK != qapi_OMSM_Alloc(QAPI_OMSM_DEFAULT_AON_POOL, OM_SMEM_FW_UPGRADE_ID_IMG_HDR, len, (void**)&fw_upgrade_image_hdr) ) {
			rtn = QAPI_FW_UPGRADE_ERR_INSUFFICIENT_MEMORY_E;
			goto parse_img_hdr_end;
		}
		if( QAPI_OK != qapi_OMSM_Commit(QAPI_OMSM_DEFAULT_AON_POOL, OM_SMEM_FW_UPGRADE_ID_IMG_HDR) ) {
			rtn = QAPI_FW_UPGRADE_ERR_INSUFFICIENT_MEMORY_E;
			goto parse_img_hdr_end;
		}  	
	}
    
    /*save firmware upgrade image entries */
    memcpy((uint8_t *)(fw_upgrade_image_hdr), fw_upgrade_cxt->config_buf+sizeof(fw_Upgrade_ImageSet_Hdr_Part1_t), len);

    /* check image entries */
    for(i = 0, len = 0, disk_size = 0, img_hdr = fw_upgrade_image_hdr; i < imgset_hdr->num_images; i++ )
    {
        //check image length
        if(     (img_hdr->image_id == 0) 
            ||  (img_hdr->magic == 0)
            ||  (img_hdr->hash_type == 0)
            ||  (img_hdr->disk_size == 0)
            ||  (img_hdr->disk_size <  img_hdr->image_length) ) {
            FW_UPGRADE_D_PRINTF("firmware upgrade image length setting is not correct\r\n");
            rtn = QAPI_FW_UPGRADE_ERR_INCORRECT_IMAGE_HDR_E;
            goto parse_img_hdr_end;  
        } else {
            /* adjust disk_size to align with flash_block_size */
            qapi_Fw_Upgrade_Get_Flash_Block_Size(&block_size);
            if(img_hdr->disk_size % block_size != 0) img_hdr->disk_size += block_size;
            img_hdr->disk_size = img_hdr->disk_size / block_size * block_size;
            
            /* when calc the disk_size, exclude the first and second File system 
               due to the File ssytem are pre-reserved already
            */
            if( (img_hdr->image_id != FS1_IMG_ID) && (img_hdr->image_id != FS2_IMG_ID) ) {
                disk_size += img_hdr->disk_size;
                len += img_hdr->image_length;
            } 
        }

        // set pointer to next image header
        img_hdr++;
    }
    
    //check if flash has enough space to store images with disk_size 
    if( disk_size > fw_upgrade_cxt->trial_flash_size ) {
        rtn = QAPI_FW_UPGRADE_ERR_FLASH_NOT_ENOUGH_SPACE_E;
        goto parse_img_hdr_end;
    }    
    
    if( len > disk_size ) {
        rtn = QAPI_FW_UPGRADE_ERR_INCORRECT_IMAGE_HDR_E;
        goto parse_img_hdr_end;        
    }    

    /* adjust file read count for all-in-one fw upgrade */
    fw_upgrade_cxt->file_read_count = imgset_hdr->length;

    /* free config buffer */
    if(fw_upgrade_cxt->config_buf != NULL ) {
        free(fw_upgrade_cxt->config_buf);
        fw_upgrade_cxt->config_buf = NULL;
    }
    
    /* all-in-one fw upgrade case */
    if(  fw_upgrade_cxt->format != FW_UPGRADE_FORAMT_PARTIAL_UPGRADE ) { 
        //imageset header is fully received and processed, move to next stage 
        fw_upgrade_cxt->is_first = 0;
        if( fw_upgrade_cxt->buf_offset >= fw_upgrade_cxt->buf_len)
            fw_Upgrade_Set_State(QAPI_FW_UPGRADE_STATE_RECEIVE_DATA_E); 
        else
            fw_Upgrade_Set_State(QAPI_FW_UPGRADE_STATE_PROCESS_IMAGE_E);
        goto parse_img_hdr_end;
    }

    /* this is for partial fw upgrade case */
    qapi_Fw_Upgrade_Get_Flash_Block_Size(&block_size);    
    active_fwd = qapi_Fw_Upgrade_Get_Active_FWD(NULL, NULL);    
    hash_buf = (uint8_t *) malloc(block_size);
    if( hash_buf == NULL ) {
        rtn = QAPI_FW_UPGRADE_ERR_INSUFFICIENT_MEMORY_E;
        goto parse_img_hdr_end;
    }    

    /* check images if need download */
    for(i = 0, img_hdr = fw_upgrade_image_hdr; i < fw_upgrade_cxt->total_images; i++, img_hdr++  )
    {
        fw_upgrade_cxt->download_flag[i] = 1;  /* default is to download */
        if( (img_hdr->image_id != FS1_IMG_ID) && (img_hdr->image_id != FS2_IMG_ID) ) {
            if( qapi_Fw_Upgrade_Find_Partition(active_fwd, img_hdr->image_id, &hdl) != QAPI_OK )
                continue;

            /* calc hash */
            if (qapi_Crypto_Op_Reset(fw_upgrade_cxt->digest_ctx) != QAPI_OK ) {
                qapi_Fw_Upgrade_Close_Partition(hdl);
                
                rtn = QAPI_FW_UPGRADE_ERR_CRYPTO_FAIL_E;
                goto parse_img_hdr_end;     
            }
            
            qapi_Fw_Upgrade_Get_Partition_Size(hdl, &disk_size);
            
            for( offset = 0; offset < disk_size; offset += block_size )
            {
                qapi_Fw_Upgrade_Read_Partition(hdl, offset, (char *)hash_buf, block_size, &nbytes);
                qapi_Crypto_Op_Digest_Update(fw_upgrade_cxt->digest_ctx, hash_buf, block_size);                
            }
            qapi_Fw_Upgrade_Close_Partition(hdl);
            len = FW_UPGRADE_HASH_LEN;
            qapi_Crypto_Op_Digest_Final(fw_upgrade_cxt->digest_ctx, NULL, 0, (uint8_t *)hash, &len );
            
            //compare firmware upgrade image Header HASH
            if( memcmp(img_hdr->hash, hash, FW_UPGRADE_HASH_LEN) == 0 ) {
                fw_upgrade_cxt->download_flag[i] = 0;  
            }
        }
    }
    
    if(hash_buf != NULL)
        free(hash_buf);
    
    //config file is fully received, move to next stage 
    fw_upgrade_cxt->is_first = 0;
    fw_Upgrade_Set_State(QAPI_FW_UPGRADE_STATE_DISCONNECT_SERVER_E);
    return rtn;
    
parse_img_hdr_end:
    if( (rtn != QAPI_FW_UPGRADE_OK_E) && (fw_upgrade_cxt->config_buf != NULL ) ) {
        free(fw_upgrade_cxt->config_buf);
        fw_upgrade_cxt->config_buf = NULL;
    }
    if(hash_buf != NULL)
        free(hash_buf);
    return rtn;
}

/*
 * verify image hash
 */
static qapi_Fw_Upgrade_Status_Code_t fw_Upgrade_Verify_Image_Hash(fw_Upgrade_Image_Hdr_t *image_hdr)
{
    qapi_Fw_Upgrade_Status_Code_t rtn = QAPI_FW_UPGRADE_OK_E;
    fw_Upgrade_Context_t *fw_upgrade_cxt;
    uint8_t *hash_org, hash_result[FW_UPGRADE_HASH_LEN];
    uint8_t *hash_buf = NULL;
    uint32_t len;
        
    fw_upgrade_cxt = fw_Upgrade_Get_Context();
    if( fw_upgrade_cxt == NULL ) {
        return QAPI_FW_UPGRADE_ERR_SESSION_NOT_START_E;
    }

    /* get org hash offset at image header */
    hash_org = (uint8_t *) image_hdr + sizeof(fw_Upgrade_Image_Hdr_t) - FW_UPGRADE_HASH_LEN;

    if( fw_upgrade_cxt->format == FW_UPGRADE_FORAMT_PARTIAL_UPGRADE ) {
        len = image_hdr->disk_size - image_hdr->image_length;
        hash_buf = (uint8_t *) malloc(len);
        if( hash_buf == NULL ) {
            return QAPI_FW_UPGRADE_ERR_INSUFFICIENT_MEMORY_E;    
        }    
        memset(hash_buf, 0xff, len);
        qapi_Crypto_Op_Digest_Update(fw_upgrade_cxt->digest_ctx, hash_buf, len);
    }
    
    /* get result */ 
    len = FW_UPGRADE_HASH_LEN;
    qapi_Crypto_Op_Digest_Final(fw_upgrade_cxt->digest_ctx, NULL, 0, hash_result, &len );
    
    if( hash_buf != NULL)
        free(hash_buf);
    
    /* compare fw upgrade image HASH */
    if( memcmp(hash_org, hash_result, FW_UPGRADE_HASH_LEN) != 0 ) {
        FW_UPGRADE_D_PRINTF("HASH is incorrect\r\n");
        rtn = QAPI_FW_UPGRADE_ERR_INCORRECT_IMAGE_CHECKSUM_E;
    }

    return rtn;
}

/*
 * process firmware upgrade image
 */
static qapi_Fw_Upgrade_Status_Code_t fw_Upgrade_Process_Receive_Image(uint8_t *buffer)
{
    qapi_Fw_Upgrade_Status_Code_t rtn = QAPI_FW_UPGRADE_OK_E;
    fw_Upgrade_Context_t *fw_upgrade_cxt;
    fw_Upgrade_Image_Hdr_t *img_hdr;
    uint32_t buf_len = 0, write_len, block_size;

    fw_upgrade_cxt = fw_Upgrade_Get_Context();
    if( fw_upgrade_cxt == NULL )
      return QAPI_FW_UPGRADE_ERR_SESSION_NOT_START_E;

    /* get flash block size */
    qapi_Fw_Upgrade_Get_Flash_Block_Size(&block_size);  
    
    while(1)
    {
        /* for all-in-one fw upgrade case */
        if( fw_upgrade_cxt->format != FW_UPGRADE_FORAMT_PARTIAL_UPGRADE) {  
            //all buffers have been processed
            if(fw_upgrade_cxt->buf_offset >= fw_upgrade_cxt->buf_len ) {
                break;
            }

            //get available buf length
            buf_len = fw_upgrade_cxt->buf_len - fw_upgrade_cxt->buf_offset;            
        }
        
        img_hdr = fw_upgrade_image_hdr;
        img_hdr += fw_upgrade_cxt->image_index;
        
        if(fw_upgrade_cxt->image_wrt_length == 0 )  {//image entry not init
            fw_upgrade_cxt->image_wrt_count = 0;
            fw_upgrade_cxt->image_wrt_length = img_hdr->image_length;
          
            //create one image entry
            if(img_hdr->image_id == FS1_IMG_ID) {
                uint32_t disk_size, disk_start;
                if( qapi_Fw_Upgrade_Find_Partition(qapi_Fw_Upgrade_Get_Active_FWD(NULL, NULL), FS2_IMG_ID, &fw_upgrade_cxt->partition_hdl) != QAPI_OK ) {
                    rtn = QAPI_FW_UPGRADE_ERR_FLASH_IMAGE_NOT_FOUND_E;
                    break;
                }
 
                qapi_Fw_Upgrade_Get_Partition_Size(fw_upgrade_cxt->partition_hdl, &disk_size); 
                qapi_Fw_Upgrade_Get_Partition_Start(fw_upgrade_cxt->partition_hdl, &disk_start);
                qapi_Fw_Upgrade_Close_Partition(fw_upgrade_cxt->partition_hdl);
                fw_upgrade_cxt->partition_hdl = NULL;
                
                //File system disk size is pre-set when first time download
                //Firmware Upgrade can't change size other than original size  
                if( img_hdr->disk_size > disk_size ) {
                    rtn = QAPI_FW_UPGRADE_ERR_INCORRECT_IMAGE_LENGTH_E;
                    break;
                }
                
                // create image for trial image's FS2
                if( qapi_Fw_Upgrade_Create_Partition(fw_upgrade_cxt->trial_FWD_idx, img_hdr->image_id, img_hdr->version, disk_start, disk_size, &fw_upgrade_cxt->partition_hdl) != QAPI_OK ) {
                    rtn = QAPI_FW_UPGRADE_ERR_FLASH_CREATE_PARTITION_E;
                    break;
                }                
            } else {    
                if( qapi_Fw_Upgrade_Create_Partition(fw_upgrade_cxt->trial_FWD_idx, img_hdr->image_id, img_hdr->version, fw_upgrade_cxt->trial_flash_start, img_hdr->disk_size, &fw_upgrade_cxt->partition_hdl) != QAPI_OK ) {
                    rtn = QAPI_FW_UPGRADE_ERR_FLASH_CREATE_PARTITION_E;
                    break;
                }

                // erase first block
                if( qapi_Fw_Upgrade_Erase_Partition(fw_upgrade_cxt->partition_hdl, 0, block_size) != QAPI_OK ) {
                    rtn = QAPI_FW_UPGRADE_ERR_FLASH_ERASE_PARTITION_E;
                    break;
                }                
            }
            
            //process the case of image length is 0x0 when using all-in-one fw upgrade
            if((fw_upgrade_cxt->format != FW_UPGRADE_FORAMT_PARTIAL_UPGRADE) && (img_hdr->image_length == 0 ) ) {
                //free partition handle
                qapi_Fw_Upgrade_Close_Partition(fw_upgrade_cxt->partition_hdl);
                fw_upgrade_cxt->partition_hdl = NULL;
                
                //FS1_IMG has its own start address and size
                if( img_hdr->image_id != FS1_IMG_ID) 
                {
                    //adjust flash start address for next entry
                    fw_upgrade_cxt->trial_flash_start += img_hdr->disk_size;  
                }
                
                //still have data at buffer and move to next image entry
                fw_upgrade_cxt->image_index++;
                fw_upgrade_cxt->image_wrt_length = 0;
            
                //check if we have received all images
                if( fw_upgrade_cxt->image_index >= fw_upgrade_cxt->total_images ) {
                    fw_Upgrade_Set_State(QAPI_FW_UPGRADE_STATE_DUPLICATE_FS_E);
                    break;
                }
                
                continue;
            }
            //reset crypto engine
            if (qapi_Crypto_Op_Reset(fw_upgrade_cxt->digest_ctx) != QAPI_OK) {
                rtn = QAPI_FW_UPGRADE_ERR_CRYPTO_FAIL_E;
                break;
            }
        }
      
        //set write_flash_len
        if( fw_upgrade_cxt->format == FW_UPGRADE_FORAMT_PARTIAL_UPGRADE ) {
            write_len = fw_upgrade_cxt->buf_len;
            fw_upgrade_cxt->buf_offset = 0;
            if( write_len  > (fw_upgrade_cxt->image_wrt_length - fw_upgrade_cxt->image_wrt_count) ) {
                rtn = QAPI_FW_UPGRADE_ERR_INCORRECT_IMAGE_LENGTH_E;
                break;
            }
        } else {
            write_len = MIN(buf_len, (fw_upgrade_cxt->image_wrt_length - fw_upgrade_cxt->image_wrt_count));
        }

        //update firmware upgrade image HASH
        qapi_Crypto_Op_Digest_Update(fw_upgrade_cxt->digest_ctx, (uint8_t *)&buffer[fw_upgrade_cxt->buf_offset], write_len);

        //check flash block if need erase first
        {
            uint32_t first_block, last_block;
        
            if(  (fw_upgrade_cxt->image_wrt_count / block_size) != ((fw_upgrade_cxt->image_wrt_count + write_len - 1) / block_size ) ) {
                first_block = fw_upgrade_cxt->image_wrt_count / block_size+1;
                
                last_block = (fw_upgrade_cxt->image_wrt_count + write_len) / block_size;
                if(((fw_upgrade_cxt->image_wrt_count + write_len) % block_size) != 0 ) last_block++;
                // erase blocks
                if( qapi_Fw_Upgrade_Erase_Partition(fw_upgrade_cxt->partition_hdl, first_block*block_size, (last_block-first_block)*block_size) != QAPI_OK ) {
                    rtn = QAPI_FW_UPGRADE_ERR_FLASH_ERASE_PARTITION_E;
                    break;
                }
            }
        }
        
        //write flash
        if( qapi_Fw_Upgrade_Write_Partition(fw_upgrade_cxt->partition_hdl, fw_upgrade_cxt->image_wrt_count, (char *)&buffer[fw_upgrade_cxt->buf_offset], write_len) != QAPI_OK ) {
            rtn = QAPI_FW_UPGRADE_ERR_FLASH_WRITE_PARTITION_E;
            break;
        }
        
        //update record
        fw_upgrade_cxt->buf_offset += write_len;
        fw_upgrade_cxt->file_read_count += write_len;
        fw_upgrade_cxt->image_wrt_count += write_len;
    
        //flash one image, move to next one 
        if( fw_upgrade_cxt->image_wrt_count >= fw_upgrade_cxt->image_wrt_length ) {
            //verify image HASH
            if( (rtn = fw_Upgrade_Verify_Image_Hash(img_hdr)) != QAPI_FW_UPGRADE_OK_E ) {
                break;              
            }
            
            //free partition handle
            qapi_Fw_Upgrade_Close_Partition(fw_upgrade_cxt->partition_hdl);
            fw_upgrade_cxt->partition_hdl = NULL;
          
            //FS1_IMG has its own start address and size
            if( img_hdr->image_id != FS1_IMG_ID) {
                //adjust flash start address for next entry
                fw_upgrade_cxt->trial_flash_start += img_hdr->disk_size;  
            }
            
            //still have data at buffer and move to next image entry
            fw_upgrade_cxt->image_index++;
            fw_upgrade_cxt->image_wrt_length = 0;
            
            if( fw_upgrade_cxt->format == FW_UPGRADE_FORAMT_PARTIAL_UPGRADE ) {
                //move to next state
                fw_Upgrade_Set_State(QAPI_FW_UPGRADE_STATE_DISCONNECT_SERVER_E);
            } else {
                //check if we have received all images
                if( fw_upgrade_cxt->image_index >= fw_upgrade_cxt->total_images ) {
                    fw_Upgrade_Set_State(QAPI_FW_UPGRADE_STATE_DUPLICATE_FS_E);
                    break;
                }                
            }
        }
        
        //check if need erase block for next round
        if(  ((fw_upgrade_cxt->image_wrt_count % block_size) == 0) && (fw_upgrade_cxt->image_wrt_length >0) ) {
            // erase block
            if( qapi_Fw_Upgrade_Erase_Partition(fw_upgrade_cxt->partition_hdl, fw_upgrade_cxt->image_wrt_count, block_size) != QAPI_OK ) {
                rtn = QAPI_FW_UPGRADE_ERR_FLASH_ERASE_PARTITION_E;
                break;
            }
        }

        if( fw_upgrade_cxt->format == FW_UPGRADE_FORAMT_PARTIAL_UPGRADE ) {
            /* it is done for this round */
            break;
        }
    }
    
    return rtn;
}

/*
 * process duplicate file system
 */
static qapi_Fw_Upgrade_Status_Code_t fw_Upgrade_Process_Duplicate_FS(uint32_t flags)
{
#define BUF_SIZE    1024
#define NAME_LEN    256
    static const char *fs_root_bdev_name = "spinor";
    static const char *fs1_root_name = "/spinor/";    
    static const char *fs2_root_name = "/spinor2/";
    int result = 1;
    int fd_write = -1, fd_read = -1;
    uint32_t num_read, num_write;
    uint8_t fs2_name[NAME_LEN], *name_ptr1, *name_ptr2;
    uint8_t *buf = (uint8_t *) malloc(BUF_SIZE);
    qapi_Fw_Upgrade_Status_Code_t rtn = QAPI_FW_UPGRADE_OK_E;
    qapi_fs_iter_handle_t iter_handle1 = NULL, iter_handle2;
    struct qapi_fs_iter_entry file_info;
	qapi_Status_t status;
	
    if( buf  == NULL ) { 
        rtn = QAPI_FW_UPGRADE_ERR_INSUFFICIENT_MEMORY_E;
        goto dup_fs_end;
    }
    
    //get FS1 file list handle 
    status = qapi_Fs_Iter_Open(fs1_root_name, &iter_handle1);
    if ( QAPI_OK != status ) {
        rtn = QAPI_FW_UPGRADE_ERR_FILE_OPEN_ERROR_E;
        goto dup_fs_end;
    }

    /*--------------------------------------------------------*/
    //prepare FS2
    status = qapi_Fs_Iter_Open(fs2_root_name, &iter_handle2);
    if ( QAPI_OK != status ) {
        result = fs_mount(fs_root_bdev_name, fs2_root_name, "ffs", FS_MOUNT_FLAG_CREATE_FS, (void*)FS2_IMG_ID);
        if( result != 0 ) {
            rtn = QAPI_FW_UPGRADE_ERR_MOUNT_FILE_SYSTEM_ERROR_E;
            goto dup_fs_end;
        }

        status = qapi_Fs_Iter_Open(fs2_root_name, &iter_handle2);
        if ( QAPI_OK != status ) {        
            rtn = QAPI_FW_UPGRADE_ERR_FILE_OPEN_ERROR_E;
            goto dup_fs_end;
        }
    }
    qapi_Fs_Iter_Close(iter_handle2);

    //copy files from FS1 to FS2
    do {
        uint32_t len1, len2;
        
        status = qapi_Fs_Iter_Next(iter_handle1, &file_info);
        if ( QAPI_OK == status ) {
            //open file at FS1
            status = qapi_Fs_Open(file_info.file_path, QAPI_FS_O_RDONLY, &fd_read);
            if ( QAPI_OK != status ) {
                rtn = QAPI_FW_UPGRADE_ERR_FILE_OPEN_ERROR_E;
                goto dup_fs_end;
            }
    
            //prepare FS2 file name
            len2 = strnlen(fs2_root_name, NAME_LEN);
            name_ptr2 = (uint8_t *) strcpy((char *)fs2_name, fs2_root_name) + len2;
            name_ptr1 = (uint8_t *) strstr(file_info.file_path, fs1_root_name);
            if( name_ptr1 == NULL ) {
                break;
            }    
            
            name_ptr1 += strlen(fs1_root_name);
            len1 = strlen((char *) name_ptr1);
            
            //validate the buffer length
            if( NAME_LEN - len2 < len1 ) {
                rtn = QAPI_FW_UPGRADE_ERR_FILE_NAME_TOO_LONG_E;
                break;
            }
            
            name_ptr2 = (uint8_t *) strcpy((char *) name_ptr2, (char *)name_ptr1) + len1;
            *name_ptr2 = '\0';
            
            //check KEEP_TRIAL_FILE Flag
            if( flags & QAPI_FW_UPGRADE_FLAG_DUPLICATE_KEEP_TRIAL_FS ) {
                //open file at FS2 as READONLY
                status = qapi_Fs_Open((char *) fs2_name, QAPI_FS_O_RDONLY, &fd_write);
                if ( QAPI_OK == status ) {
                    //file exists, skip copying the file 
                    goto next_file;
                }
            }
            
            //open file at FS2
            status = qapi_Fs_Open((char *) fs2_name, QAPI_FS_O_CREAT | QAPI_FS_O_RDWR | QAPI_FS_O_TRUNC, &fd_write);
            if ( QAPI_OK != status ) {
                rtn = QAPI_FW_UPGRADE_ERR_FILE_OPEN_ERROR_E;
                goto dup_fs_end;
            }
            
            //copy file
            while( (qapi_Fs_Read(fd_read, buf, BUF_SIZE, &num_read) == QAPI_OK) && (num_read > 0))
            {
                uint32_t offset, count;

                offset = 0;
                count = num_read;
                
                while(count > 0)
                {
                    if( qapi_Fs_Write(fd_write, buf+offset, count, &num_write) != QAPI_OK ) {
                        rtn = QAPI_FW_UPGRADE_ERR_FILE_WRITE_ERROR_E;
                        break;
                    }
                    
                    offset += num_write;
                    count -= num_write;
                }
            }
next_file:            
            //close the files
            if ( fd_read >= 0 ) {
                qapi_Fs_Close(fd_read);
                fd_read = -1;
            }

            if ( fd_write >= 0 ) {
                qapi_Fs_Close(fd_write);
                fd_write = -1;
            }            
        }
    } while ( QAPI_OK == status );


dup_fs_end:
    if( buf ) {
        free(buf);
    }
    
    if( fd_read >= 0 ) {
        qapi_Fs_Close(fd_read);
    }

    if( fd_write >= 0 ) {
        qapi_Fs_Close(fd_write);
    }
    
    if( iter_handle1 != NULL) {
        qapi_Fs_Iter_Close(iter_handle1);
    }
    
    if( result == 0) {
        fs_umount((char *) fs2_name);
    }
    return rtn;
}

/*
 * process duplicate images from current to trial if need
 */
static qapi_Fw_Upgrade_Status_Code_t fw_Upgrade_Process_Duplicate_Images(void)
{
    qapi_Fw_Upgrade_Status_Code_t rtn = QAPI_FW_UPGRADE_OK_E;
    uint32_t i, offset, size, nbytes;
    uint8_t  *buf=NULL;
    uint8_t  active_fwd;
    fw_Upgrade_Context_t     *fw_upgrade_cxt;
    fw_Upgrade_Image_Hdr_t   *img_hdr;
    qapi_Part_Hdl_t hdl = NULL;

    fw_upgrade_cxt = fw_Upgrade_Get_Context();
    if( fw_upgrade_cxt == NULL ) {
      return QAPI_FW_UPGRADE_ERR_SESSION_NOT_START_E;     
    }
    
    qapi_Fw_Upgrade_Get_Flash_Block_Size(&size);    
    active_fwd = qapi_Fw_Upgrade_Get_Active_FWD(NULL, NULL);    
    buf = (uint8_t *) malloc(size);
    if( buf == NULL ) {
        rtn = QAPI_FW_UPGRADE_ERR_INSUFFICIENT_MEMORY_E;
        goto dup_img_end;
    }
    
    //check images if need download
    for(i = 0, img_hdr = fw_upgrade_image_hdr; i < fw_upgrade_cxt->total_images; i++, img_hdr++  )
    {
        if( (fw_upgrade_cxt->download_flag[i] == 0) && (img_hdr->image_id != FS1_IMG_ID) && (img_hdr->image_id != FS2_IMG_ID) ) {
            if( qapi_Fw_Upgrade_Find_Partition(active_fwd, img_hdr->image_id, &hdl) != QAPI_OK ) {
                rtn = QAPI_FW_UPGRADE_ERR_INCORRECT_IMAGE_HDR_E;
                break;
            }                

            if( qapi_Fw_Upgrade_Create_Partition(fw_upgrade_cxt->trial_FWD_idx, img_hdr->image_id, img_hdr->version, fw_upgrade_cxt->trial_flash_start, img_hdr->disk_size, &fw_upgrade_cxt->partition_hdl) != QAPI_OK ) {
                rtn = QAPI_FW_UPGRADE_ERR_FLASH_CREATE_PARTITION_E;
                break;
            }


            for( offset = 0; offset < img_hdr->disk_size; offset += size )
            {
                qapi_Fw_Upgrade_Read_Partition(hdl, offset, (char *)buf, size, &nbytes);
                // erase one block
                if( qapi_Fw_Upgrade_Erase_Partition(fw_upgrade_cxt->partition_hdl, offset, size) != QAPI_OK ) {
                    rtn = QAPI_FW_UPGRADE_ERR_FLASH_ERASE_PARTITION_E;
                    break;
                } 
                //write flash
                if( qapi_Fw_Upgrade_Write_Partition(fw_upgrade_cxt->partition_hdl, offset, (char *)buf, size) != QAPI_OK ) {
                    rtn = QAPI_FW_UPGRADE_ERR_FLASH_WRITE_PARTITION_E;
                    break;
                }                
            }
            qapi_Fw_Upgrade_Close_Partition(hdl);
            hdl = NULL;
            qapi_Fw_Upgrade_Close_Partition(fw_upgrade_cxt->partition_hdl);
            fw_upgrade_cxt->partition_hdl = NULL;
            
            //adjust flash start address for next entry
            fw_upgrade_cxt->trial_flash_start += img_hdr->disk_size;  
        }
    }
       
dup_img_end:
    if(fw_upgrade_cxt->partition_hdl ) {
        qapi_Fw_Upgrade_Close_Partition(fw_upgrade_cxt->partition_hdl);
        fw_upgrade_cxt->partition_hdl = NULL;
    }
    if( hdl != NULL) {
        qapi_Fw_Upgrade_Close_Partition(hdl);
    }
    if(buf != NULL) {
        free(buf);
    }
    return rtn;
}

/*
 * call fw upgrade plugin's init callback
 */
static qapi_Fw_Upgrade_Status_Code_t fw_Upgrade_Plugin_Init(void)
{
    fw_Upgrade_Context_t *fw_upgrade_cxt;
    fw_Upgrade_Image_Hdr_t *img_hdr;
	qapi_Fw_Upgrade_Status_Code_t ret = QAPI_FW_UPGRADE_OK_E;
    char *combined_url = NULL;
    uint32_t url_len = 0;

    fw_upgrade_cxt = fw_Upgrade_Get_Context();
    if( fw_upgrade_cxt == NULL )
      return QAPI_FW_UPGRADE_ERR_SESSION_NOT_START_E;

    fw_upgrade_cxt->file_read_count = 0;

    combined_url = (char *)malloc(FW_UPGRADE_URL_TOTAL_LEN);
    if (!combined_url) {
        FW_UPGRADE_D_PRINTF("Out of memory error\r\n");
        return QAPI_FW_UPGRADE_ERR_INSUFFICIENT_MEMORY_E;
    }
    memset(combined_url, 0, FW_UPGRADE_URL_TOTAL_LEN);
    url_len = strlen(fw_upgrade_cxt->url);
    memcpy(combined_url, fw_upgrade_cxt->url, url_len);
    
    if( fw_upgrade_cxt->is_first != 0) {
        memcpy(&combined_url[url_len], fw_upgrade_cxt->cfg_file, strlen(fw_upgrade_cxt->cfg_file));
    } else {
        img_hdr = fw_upgrade_image_hdr;
        img_hdr += fw_upgrade_cxt->image_index;
        memcpy(&combined_url[url_len], img_hdr->image_file, strlen((char *)img_hdr->image_file));
    }

    ret = fw_upgrade_cxt->plugin.fw_Upgrade_Plugin_Init(fw_upgrade_cxt->interface_name, (const char *)combined_url, fw_upgrade_cxt->init_param);
    free(combined_url);
    return ret;
}

/*
 * call fw upgrade plugin's Recev_Data callback
 */
static qapi_Fw_Upgrade_Status_Code_t fw_Upgrade_Plugin_Recv_Data(uint8_t *buffer, uint32_t buf_len, uint32_t *ret_size)
{
    fw_Upgrade_Context_t *fw_upgrade_cxt;
    
    fw_upgrade_cxt = fw_Upgrade_Get_Context();
    if( fw_upgrade_cxt == NULL )
      return QAPI_FW_UPGRADE_ERR_SESSION_NOT_START_E;
 
    return fw_upgrade_cxt->plugin.fw_Upgrade_Plugin_Recv_Data(buffer, buf_len, ret_size);
}

/*
 * call fw upgrade plugin's Fin callback
 */
static qapi_Fw_Upgrade_Status_Code_t fw_Upgrade_Plugin_Fin(void)
{
    fw_Upgrade_Context_t *fw_upgrade_cxt;
    
    fw_upgrade_cxt = fw_Upgrade_Get_Context();
    if( fw_upgrade_cxt == NULL )
      return QAPI_FW_UPGRADE_ERR_SESSION_NOT_START_E;
 
    return fw_upgrade_cxt->plugin.fw_Upgrade_Plugin_Fin();
}

/*
 * call fw upgrade plugin's abort callback
 */
static qapi_Fw_Upgrade_Status_Code_t fw_Upgrade_Plugin_Abort(void)
{
    fw_Upgrade_Context_t *fw_upgrade_cxt;
    
    fw_upgrade_cxt = fw_Upgrade_Get_Context();
    if( fw_upgrade_cxt == NULL )
      return QAPI_FW_UPGRADE_ERR_SESSION_NOT_START_E;
 
    return fw_upgrade_cxt->plugin.fw_Upgrade_Plugin_Abort(); 
}

/*
 * call fw upgrade plugin's resume callback
 */
static qapi_Fw_Upgrade_Status_Code_t fw_Upgrade_Plugin_Resume(void)
{
    fw_Upgrade_Context_t *fw_upgrade_cxt;
    fw_Upgrade_Image_Hdr_t *img_hdr;    
	qapi_Fw_Upgrade_Status_Code_t ret = QAPI_FW_UPGRADE_OK_E;
    char *combined_url = NULL;
    uint32_t url_len = 0;

    fw_upgrade_cxt = fw_Upgrade_Get_Context();
    if( fw_upgrade_cxt == NULL )
      return QAPI_FW_UPGRADE_ERR_SESSION_NOT_START_E;

    combined_url = (char *)malloc(FW_UPGRADE_URL_TOTAL_LEN);
    if (!combined_url) {
        FW_UPGRADE_D_PRINTF("Out of memory error\r\n");
        return QAPI_FW_UPGRADE_ERR_INSUFFICIENT_MEMORY_E;
    }
    memset(combined_url, 0, FW_UPGRADE_URL_TOTAL_LEN);
    url_len = strlen(fw_upgrade_cxt->url);
    memcpy(combined_url, fw_upgrade_cxt->url, url_len);
    
    if( fw_upgrade_cxt->format != FW_UPGRADE_FORAMT_PARTIAL_UPGRADE) { /* all-in-one fw upgrade */
        memcpy(&combined_url[url_len], fw_upgrade_cxt->cfg_file, strlen(fw_upgrade_cxt->cfg_file));
    } else {
        if( fw_upgrade_cxt->is_first != 0) {
            memcpy(&combined_url[url_len], fw_upgrade_cxt->cfg_file, strlen(fw_upgrade_cxt->cfg_file));
        } else {
            img_hdr = fw_upgrade_image_hdr;
            img_hdr += fw_upgrade_cxt->image_index;
            memcpy(&combined_url[url_len], img_hdr->image_file, strlen((char *)img_hdr->image_file));
        }        
    }
    
    ret = fw_upgrade_cxt->plugin.fw_Upgrade_Plugin_Resume(fw_upgrade_cxt->interface_name, (const char *)combined_url, fw_upgrade_cxt->file_read_count);
    free(combined_url);
    return ret;
}

/*
 * get Firmware Upgrade Scheme Parameter from DEVCFG
 */
qapi_Fw_Upgrade_Status_Code_t fw_Upgrade_Get_Scheme_Param(uint32_t id, uint32_t *param)
{
   DALSYSPropertyHandle hProps;
   DALSYSPropertyVar var;

   if( DAL_SUCCESS != DALSYS_GetDALPropertyHandle (DALDEVICEID_FW_UPGRADE_SCHEME, hProps)) {
      return  QAPI_FW_UPGRADE_ERROR_E;
   }
   if (DAL_SUCCESS != DALSYS_GetPropertyValue(hProps, NULL, id , &var)) {
     return QAPI_FW_UPGRADE_ERROR_E;
   }
   *param = var.Val.dwVal;
 
   return QAPI_FW_UPGRADE_OK_E;
}

/*
 * get fw upgrade session active status
 */
fw_Upgrade_Session_Status_t fw_Upgrade_Get_Session_Status(void)
{
    fw_Upgrade_Session_Status_t rtn = FW_UPGRADE_SESSION_NOT_START_E;
    fw_Upgrade_Context_t *fw_upgrade_cxt;

    if( Fw_Upgrade_Mutex_Init == 0 )
        return rtn;
    
    if( TAKE_LOCK(Fw_Upgrade_Mutex) ) {
        /* get fw upgrade session context */
        fw_upgrade_cxt = fw_Upgrade_Get_Context();
        if( fw_upgrade_cxt == NULL ) {
            rtn = FW_UPGRADE_SESSION_NOT_START_E;
        } else {    
            rtn = fw_upgrade_cxt->fw_upgrade_session_status;
        }
        RELEASE_LOCK(Fw_Upgrade_Mutex);
    }
    
    return rtn;
}

/*
 * set fw upgrade session active status
 */
qapi_Fw_Upgrade_Status_Code_t fw_Upgrade_Set_Session_Status(fw_Upgrade_Session_Status_t status)
{
    qapi_Fw_Upgrade_Status_Code_t rtn = QAPI_FW_UPGRADE_ERR_SESSION_NOT_START_E;
    fw_Upgrade_Context_t *fw_upgrade_cxt;

    if( Fw_Upgrade_Mutex_Init == 0 )
        return rtn;
    
    if( TAKE_LOCK(Fw_Upgrade_Mutex) ) {    
        /* get fw upgrade session context */
        fw_upgrade_cxt = fw_Upgrade_Get_Context();
        if( fw_upgrade_cxt == NULL ) {
            rtn = QAPI_FW_UPGRADE_ERR_SESSION_NOT_START_E;
        } else {    
            fw_upgrade_cxt->fw_upgrade_session_status = status;
            rtn = QAPI_FW_UPGRADE_OK_E;
        }
        RELEASE_LOCK(Fw_Upgrade_Mutex);
    }
	return rtn;
}

/*
 * get fw upgrade session state
 */
qapi_Fw_Upgrade_State_t fw_Upgrade_Get_State(void)
{   
    qapi_Fw_Upgrade_State_t rtn = QAPI_FW_UPGRADE_STATE_NOT_START_E;
    fw_Upgrade_Context_t *fw_upgrade_cxt;

    if( Fw_Upgrade_Mutex_Init == 0 )
        return rtn;
    
    if( TAKE_LOCK(Fw_Upgrade_Mutex) ) {
        /* get fw upgrade session context */
        fw_upgrade_cxt = fw_Upgrade_Get_Context();
        if( fw_upgrade_cxt == NULL ) {
            rtn = QAPI_FW_UPGRADE_STATE_NOT_START_E;
        } else {    
            rtn = fw_upgrade_cxt->fw_upgrade_state;
        }
        RELEASE_LOCK(Fw_Upgrade_Mutex);
    }
	return rtn;
}

/*
 * get fw upgrade session error code
 */
qapi_Fw_Upgrade_Status_Code_t fw_Upgrade_Get_Error_Code(void)
{
    fw_Upgrade_Context_t *fw_upgrade_cxt = fw_Upgrade_Get_Context();
    if( fw_upgrade_cxt != NULL ) {
      return (qapi_Fw_Upgrade_Status_Code_t) fw_upgrade_cxt->error_code;
    }  
    return QAPI_FW_UPGRADE_ERR_SESSION_NOT_START_E;
}

/*
 * start firmware upgrade session 
 */
qapi_Fw_Upgrade_Status_Code_t fw_Upgrade(char *interface_name, qapi_Fw_Upgrade_Plugin_t *plugin, char *url, char *cfg_file, uint32_t flags, qapi_Fw_Upgrade_CB_t cb, void *init_param)
{
    qapi_Fw_Upgrade_Status_Code_t rtn = QAPI_FW_UPGRADE_OK_E;
    qapi_OMSM_alloc_status_t alloc_status;

    //check AON_FW_UPGRADE memory 
    qapi_OMSM_Check_Status(QAPI_OMSM_DEFAULT_AON_POOL, OM_SMEM_FW_UPGRADE_ID_SESSION_CXT, &alloc_status);

    if(alloc_status == QAPI_OMSM_BUF_COMMITTED_E || alloc_status == QAPI_OMSM_BUF_RETRIEVING_E) {
        return QAPI_FW_UPGRADE_ERR_SESSION_IN_PROGRESS_E;
    }

    /* session init */
    if( (rtn = fw_Upgrade_Session_Init(interface_name, plugin, url, cfg_file, flags, cb, init_param)) != QAPI_FW_UPGRADE_OK_E ) {
        /* fail to session init, just return here with error code */
        return rtn;
    }

    /* fw upgrade session process */
    rtn = fw_Upgrade_Session_Process();
    return fw_Upgrade_Session_Finalize(rtn);
}

/*
 * resume firmware upgrade session
 */
qapi_Fw_Upgrade_Status_Code_t fw_Upgrade_Session_Resume(void)
{
    qapi_Fw_Upgrade_Status_Code_t rtn = QAPI_FW_UPGRADE_OK_E;
    uint32_t state;
    uint16 buff_size;
    qapi_OMSM_alloc_status_t alloc_status;
    
    //check AON_FW_UPGRADE memory 
    qapi_OMSM_Check_Status(QAPI_OMSM_DEFAULT_AON_POOL, OM_SMEM_FW_UPGRADE_ID_SESSION_CXT, &alloc_status);    
    if(alloc_status == QAPI_OMSM_BUF_COMMITTED_E || alloc_status == QAPI_OMSM_BUF_RETRIEVING_E) {
        if( alloc_status != QAPI_OMSM_BUF_COMMITTED_E ) {
			if( QAPI_OK != qapi_OMSM_Commit(QAPI_OMSM_DEFAULT_AON_POOL, OM_SMEM_FW_UPGRADE_ID_SESSION_CXT) ) {
				return QAPI_FW_UPGRADE_ERR_SESSION_NOT_START_E;
			}
		}  
		if( QAPI_OK != qapi_OMSM_Retrieve(QAPI_OMSM_DEFAULT_AON_POOL, OM_SMEM_FW_UPGRADE_ID_SESSION_CXT, &buff_size, (void**)&fw_upgrade_sess_cxt) ) {    
			return QAPI_FW_UPGRADE_ERR_SESSION_NOT_START_E;
		}
    } else {
        return QAPI_FW_UPGRADE_ERR_SESSION_NOT_START_E;
    }
    
    qapi_OMSM_Check_Status(QAPI_OMSM_DEFAULT_AON_POOL, OM_SMEM_FW_UPGRADE_ID_IMG_HDR, &alloc_status);    
    if(alloc_status == QAPI_OMSM_BUF_COMMITTED_E || alloc_status == QAPI_OMSM_BUF_RETRIEVING_E) {
        if( alloc_status != QAPI_OMSM_BUF_COMMITTED_E ) {
			if( QAPI_OK != qapi_OMSM_Commit(QAPI_OMSM_DEFAULT_AON_POOL, OM_SMEM_FW_UPGRADE_ID_IMG_HDR) ) {
				return QAPI_FW_UPGRADE_ERR_SESSION_NOT_START_E;
			}
		}  
		
        if( QAPI_OK != qapi_OMSM_Retrieve(QAPI_OMSM_DEFAULT_AON_POOL, OM_SMEM_FW_UPGRADE_ID_IMG_HDR, &buff_size, (void**)&fw_upgrade_image_hdr) )
            return QAPI_FW_UPGRADE_ERR_SESSION_NOT_START_E;
    }

    /* get fw upgrade session context */
    if( fw_Upgrade_Get_Context() == NULL ) {
      return QAPI_FW_UPGRADE_ERR_SESSION_NOT_START_E;
    }

    if( Fw_Upgrade_Mutex_Init == 0 ) {
        qurt_mutex_init(&Fw_Upgrade_Mutex);
        qurt_signal_init(&data_ready_signal);
        qurt_signal_init(&data_drain_signal);
        qurt_signal_init(&thread_close_signal);
        Fw_Upgrade_Mutex_Init = 1;
    }
	
    if (fw_Upgrade_Get_Session_Status() != FW_UPGRADE_SESSION_SUSPEND_E) {
        return QAPI_FW_UPGRADE_ERR_SESSION_IN_PROGRESS_E;
    }

    /* reset session status */
    fw_Upgrade_Set_Session_Status(FW_UPGRADE_SESSION_RUNNING_E);

    /* init FW_Upgrade */
    if( qapi_Fw_Upgrade_init() != QAPI_OK ) {
        fw_Upgrade_Set_Session_Status(FW_UPGRADE_SESSION_NOT_START_E);
        rtn = QAPI_FW_UPGRADE_ERR_FLASH_INIT_TIMEOUT_E;
        goto session_resume_end;
    }

    //restore State
    state = (uint32_t)fw_Upgrade_Get_State();
    if( state >= QAPI_FW_UPGRADE_STATE_RESUME_SERVICE_E ) {
    	fw_Upgrade_Set_State(QAPI_FW_UPGRADE_STATE_RESUME_SERVICE_E);
    }

    //reset error code
    fw_Upgrade_Set_Error_Code(QAPI_FW_UPGRADE_OK_E);
    rtn = fw_Upgrade_Session_Process();

session_resume_end:
	return fw_Upgrade_Session_Finalize(rtn);
}

/*
 * cancel firmware upgrade session
 */
qapi_Fw_Upgrade_Status_Code_t fw_Upgrade_Session_Cancel(void)
{
    if( fw_Upgrade_Get_Context() == NULL )
        return QAPI_FW_UPGRADE_ERR_SESSION_NOT_START_E;
	if( fw_Upgrade_Get_Session_Status() == FW_UPGRADE_SESSION_RUNNING_E  )
		return fw_Upgrade_Set_Session_Status(FW_UPGRADE_SESSION_CANCEL_E);
	return fw_Upgrade_Session_Fin();
}

/*
 * suspend firmware upgrade session
 */
qapi_Fw_Upgrade_Status_Code_t fw_Upgrade_Session_Suspend(void)
{
	if( (fw_Upgrade_Get_Context() == NULL) || (fw_Upgrade_Get_Session_Status() != FW_UPGRADE_SESSION_RUNNING_E)  )
		return QAPI_FW_UPGRADE_ERR_SESSION_NOT_START_E;
	return fw_Upgrade_Set_Session_Status(FW_UPGRADE_SESSION_SUSPEND_E);
}

/*
 * done firmware upgrade session
 */
qapi_Fw_Upgrade_Status_Code_t fw_Upgrade_Session_Done(uint32_t result)
{
    qapi_Fw_Upgrade_Status_Code_t rtn = QAPI_FW_UPGRADE_ERROR_E;
    qapi_Status_t q_rtn;
    uint8_t trial, current;
    uint32_t rank;
    uint32_t id;
    qapi_Part_Hdl_t hdl = NULL;
        
    q_rtn = qapi_Fw_Upgrade_get_Trial_Active_FWD_Index(&trial, &current, &rank);
    if( q_rtn == QAPI_OK ) {
        /* process trial image based on result */
        if( result ) {
            /*
             *  check if trial has Pre-ROT image
             *  set the id to patch image id if it has
             */
            id = PRE_ROT_IMG_ID;
            if( qapi_Fw_Upgrade_Find_Partition(trial, id, &hdl) == QAPI_OK ) {   
                id = M4_IMG_ID;
                q_rtn = qapi_Fw_Upgrade_Set_Image_ID(hdl, id);
                qapi_Fw_Upgrade_Close_Partition(hdl);    
                if( q_rtn != QAPI_OK ) {
                    goto session_done_end;
                }
            }

            /* accept trial image here */
            if( qapi_Fw_Upgrade_Accept_Trial_FWD() == QAPI_OK) 
                rtn = QAPI_FW_UPGRADE_OK_E;
        } else {
            /*reject trial image */
            if( qapi_Fw_Upgrade_Reject_Trial_FWD() == QAPI_OK) 
                rtn = QAPI_FW_UPGRADE_OK_E;
        }
    }
session_done_end:
    return rtn;
}

/******************************************************************************************************/
/*              Host Firmware Upgrade APIs                                                            */
/******************************************************************************************************/
/*
 * HOST plugin Init
 * 
 * 
 */
qapi_Fw_Upgrade_Status_Code_t plugin_Host_Init(const char* interface_name, const char *url, void *init_param)
{
    return QAPI_FW_UPGRADE_OK_E;
}	

/*
 * OTA Host plugin receive data
 *    buffer:    received data buffer
 *   buf_len:    received data buffer size in bytes
 *  ret_size:    data size in buffer after receiving done
 */
qapi_Fw_Upgrade_Status_Code_t plugin_Host_Recv_Data(uint8_t *buffer, uint32_t buf_len, uint32_t *ret_size)
{
	uint32_t  signals;
	qapi_Fw_Upgrade_Status_Code_t rtn = QAPI_FW_UPGRADE_OK_E;
    fw_Upgrade_Context_t *fw_upgrade_cxt = fw_Upgrade_Get_Context();

   	*ret_size = 0;

    if( fw_upgrade_cxt == NULL ) {
      return QAPI_FW_UPGRADE_ERR_SESSION_NOT_START_E;
    }
    
	do {
		signals = qurt_signal_wait(&data_ready_signal, FWUP_DATA_RX_ALL_SIG_MASK, QURT_SIGNAL_ATTR_CLEAR_MASK);
		
		if (signals & FWUP_RX_DATA_DONE_SIG_MASK) {
			if(fw_upgrade_cxt->data_ready_len > buf_len) {
				memcpy(buffer, fw_upgrade_cxt->data_ready_ptr, buf_len);
				
				fw_upgrade_cxt->data_ready_len -= buf_len;
				fw_upgrade_cxt->data_ready_ptr += buf_len;
				
				*ret_size = buf_len;
				qurt_signal_set(&data_ready_signal, FWUP_RX_DATA_DONE_SIG_MASK);
				break;
			}
			else
			{
				memcpy(buffer, fw_upgrade_cxt->data_ready_ptr, fw_upgrade_cxt->data_ready_len);
				
				*ret_size = fw_upgrade_cxt->data_ready_len;
				fw_upgrade_cxt->data_ready_len = 0;
				qurt_signal_set(&data_drain_signal, FWUP_BUFFER_EMPTY_SIG_MASK);
                break;
			}
		}
		if (signals & FWUP_RX_DATA_FINISH_SIG_MASK)
		{
			
		}
		if (signals & FWUP_RX_DATA_ERROR_SIG_MASK)
		{
            rtn = QAPI_FW_UPGRADE_ERR_SESSION_CANCELLED_E;
            break;
		}
	} while (0);

	return rtn;
}

/*
 *  host fin plugin
 */
qapi_Fw_Upgrade_Status_Code_t plugin_Host_Fin(void)
{
    return QAPI_FW_UPGRADE_OK_E;
}

/*
 *  host abort plugin 
 */
qapi_Fw_Upgrade_Status_Code_t plugin_Host_Abort(void)
{
	return QAPI_FW_UPGRADE_OK_E;
}

/*
 *  host resume plugin
 */
qapi_Fw_Upgrade_Status_Code_t plugin_Host_Resume(const char* interface_name, const char *url, uint32_t offset)
{
	return QAPI_FW_UPGRADE_OK_E;
}


/**
   @brief This function is the thread for host firmware upgrade session.

   @param Thread_Parameter is the parameter specified when the thread was
          started. 
*/
static void fw_Upgrade_Host_Thread(void *Thread_Parameter)
{
    qapi_Fw_Upgrade_Status_Code_t rtn;
    
    /* fw upgrade session process */
    rtn = fw_Upgrade_Session_Process();
    if( rtn != QAPI_FW_UPGRADE_OK_E )
    {
        fw_Upgrade_Set_Session_Status(FW_UPGRADE_SESSION_ERROR_E);
        qurt_signal_set(&data_drain_signal, FWUP_BUFFER_EMPTY_SIG_MASK);
    }
    qurt_signal_wait(&thread_close_signal, FWUP_THREAD_CLOSE_SIG_MASK, QURT_SIGNAL_ATTR_CLEAR_MASK);
    fw_Upgrade_Session_Finalize(rtn);
    
    /* Terminate the thread. */
    qurt_thread_stop();
    return;
}

/**
 * Starts a firmware upgrade session from host.
 *
 * @param[in] flags         Flags with bits defined for a firmware upgrade. See the qapi_Fw_Upgrade flag for a definition.
 *
 * @return
 *  On success, QAPI_FW_UPGRADE_OK_E is returned. \n 
 *  On error, error code defined by enum #qapi_Fw_Upgrade_Status is returned.
 */
qapi_Fw_Upgrade_Status_Code_t fw_Upgrade_Host_Init(uint32_t flags)
{
    qurt_thread_attr_t    Thread_Attribte;
    qurt_thread_t         Thread_Handle;
    int                   Thread_Result;
    qapi_OMSM_alloc_status_t alloc_status;
    qapi_Fw_Upgrade_Status_Code_t rtn = QAPI_FW_UPGRADE_OK_E;
    qapi_Fw_Upgrade_Plugin_t plugin = { plugin_Host_Init,
                                plugin_Host_Recv_Data,
                                plugin_Host_Abort,
								plugin_Host_Resume,
                                plugin_Host_Fin};

    //check AON_FW_UPGRADE memory 
    qapi_OMSM_Check_Status(QAPI_OMSM_DEFAULT_AON_POOL, OM_SMEM_FW_UPGRADE_ID_SESSION_CXT, &alloc_status);

    if(alloc_status == QAPI_OMSM_BUF_COMMITTED_E || alloc_status == QAPI_OMSM_BUF_RETRIEVING_E) {
        return QAPI_FW_UPGRADE_ERR_SESSION_IN_PROGRESS_E;
    }

    /* session init */
    if( (rtn = fw_Upgrade_Session_Init(NULL, &plugin, NULL, NULL, flags, NULL, NULL)) != QAPI_FW_UPGRADE_OK_E ) {
        /* fail to session init, just return here with error code */
        return rtn;
    }
    
    if( fw_Upgrade_Get_Context() != NULL )
    {
        /* Create a thread for the command. */
        qurt_thread_attr_init(&Thread_Attribte);
        qurt_thread_attr_set_name(&Thread_Attribte, "FWUP_Thread");
        qurt_thread_attr_set_priority(&Thread_Attribte, FWUP_THREAD_PRIORITY);
        qurt_thread_attr_set_stack_size(&Thread_Attribte, THREAD_STACK_SIZE);
        Thread_Result = qurt_thread_create(&Thread_Handle, &Thread_Attribte, fw_Upgrade_Host_Thread, (void *)NULL);

        if(Thread_Result != QURT_EOK)
        {
            rtn = QAPI_FW_UPGRADE_ERR_CREATE_THREAD_ERROR_E;
        }
    } else {
        rtn = QAPI_FW_UPGRADE_ERR_SESSION_NOT_START_E;
    }
    return rtn;
}

/**
 * Stop a firmware upgrade session which triggered by host.
 *
 * @return
 *  On success, QAPI_FW_UPGRADE_OK_E is returned. \n 
 *  On error, error code defined by enum #qapi_Fw_Upgrade_Status is returned.
 */
qapi_Fw_Upgrade_Status_Code_t fw_Upgrade_Host_Deinit(void)
{
    if( fw_Upgrade_Get_Context() == NULL )
        return QAPI_FW_UPGRADE_ERR_SESSION_NOT_START_E;
	if(    (fw_Upgrade_Get_Session_Status() == FW_UPGRADE_SESSION_RUNNING_E )  
        && (fw_Upgrade_Get_State() < QAPI_FW_UPGRADE_STATE_DUPLICATE_FS_E )   )
    {
		fw_Upgrade_Set_Session_Status(FW_UPGRADE_SESSION_CANCEL_E);
    }
    qurt_signal_set(&thread_close_signal, FWUP_THREAD_CLOSE_SIG_MASK);
    
	return QAPI_FW_UPGRADE_OK_E;
}

/**
 * Pass buffer with len to firmware upgrade session which triggered by host.
 *
 * @return
 *  On success, QAPI_FW_UPGRADE_OK_E is returned. \n 
 *  On error, error code defined by enum #qapi_Fw_Upgrade_Status is returned.
 */
qapi_Fw_Upgrade_Status_Code_t fw_Upgrade_Host_Write(char *buffer, int32_t len)
{
    fw_Upgrade_Context_t *fw_upgrade_cxt = fw_Upgrade_Get_Context();
      
    if( fw_upgrade_cxt == NULL )
        return QAPI_FW_UPGRADE_ERR_SESSION_NOT_START_E;

    if( fw_Upgrade_Get_Session_Status() == FW_UPGRADE_SESSION_ERROR_E  ) 
        return fw_Upgrade_Get_Error_Code();
    
	if( fw_Upgrade_Get_Session_Status() != FW_UPGRADE_SESSION_RUNNING_E  ) 
        return QAPI_FW_UPGRADE_ERR_SESSION_NOT_START_E;        
    
    if (len && buffer)
    {			
        fw_upgrade_cxt->data_ready_len = len;
		fw_upgrade_cxt->data_ready_ptr = (uint8_t *)buffer;
			
		qurt_signal_set(&data_ready_signal, FWUP_RX_DATA_DONE_SIG_MASK);
		qurt_signal_wait(&data_drain_signal, FWUP_BUFFER_EMPTY_SIG_MASK, QURT_SIGNAL_ATTR_CLEAR_MASK);
    }
    return QAPI_FW_UPGRADE_OK_E;
}

