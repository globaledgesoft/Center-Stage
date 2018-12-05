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

 
#include CUST_H
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <DALSysTypes.h>
#include <DALSys.h>
#include <DALDeviceId.h>
#include "qapi/qapi_types.h"
#include "qapi/qapi_status.h"
#include "qapi/qapi_firmware_upgrade.h"
#include "qapi/qapi_firmware_image_id.h"
#include "qapi/qapi_reset.h"
#include "qapi/qapi_fatal_err.h"
#include "flash.h"
#include "flash_private.h"
#include "flash_fwd.h"
#include "fs_api.h"
#include "platform_util.h"
#include "fw_upgrade.h"
#include "fw_upgrade_dal_id.h"

////////////////////////////////////////////////////////////////////////////////

#define BOOT_FWD_TYPE_REG		(0x10000038)
#define VALID_FWD_PRESET_REG	(0x10000044)
#define WATCHDOG_RESET_COUNTER  (0x10000048)
#define FORCE_BOOT_FWD_TYPE_REG (0x1000003C)

#define PBL_AON_FWD_BOOT_BLOCK (RAM_AON_PBL_RESERVED_START_ADDRESS + 0x34)

#define QAPI_ERROR_FU_NOT_INIT                  1000

#define QAPI_FW_UPGRADE_FLASH_START_ADDRESS             (0)
#define QAPI_FW_UPGRADE_MAX_FWD                         (3)
#define QAPI_FW_UPGRADE_MAX_HANDLES                     (5)
#define QAPI_FW_UPGRADE_FWD_SIGNATURE_OFFSET            (0)
#define QAPI_FW_UPGRADE_FWD_VERSION_OFFSET              (QAPI_FW_UPGRADE_FWD_SIGNATURE_OFFSET + 4)
#define QAPI_FW_UPGRADE_FWD_RANK_OFFSET                 (QAPI_FW_UPGRADE_FWD_VERSION_OFFSET + 4)
#define QAPI_FW_UPGRADE_FWD_STATUS_OFFSET               (QAPI_FW_UPGRADE_FWD_RANK_OFFSET + 4)
#define QAPI_FW_UPGRADE_FWD_NUM_OF_IMAGES_OFFSET        (QAPI_FW_UPGRADE_FWD_STATUS_OFFSET+1)

#define QAPI_FWD_BOOT_TYPE_GOLDEN	(0x38)
#define QAPI_FWD_BOOT_TYPE_CURRENT	(0x37)
#define QAPI_FWD_BOOT_TYPE_TRIAL	(0x36)

#define QAPI_FWD_WATCHDOG_RESET_COUNT_MAX    (0x03)
/* FWD_SIGNATURE (=0x46574454 "FWDT") */
#define QAPI_FW_UPGRADE_MAGIC_V1		0x54445746

#define PARTITION_CLIENT_HANDLE_START  &partition_handles[0]
#define PARTITION_CLIENT_HANDLE_END    &partition_handles[QAPI_FW_UPGRADE_MAX_HANDLES - 1]

//internal structure for firmware upgrade context
typedef struct FU_Partition_Client {
    uint8_t ref_count;
    uint8_t FWD_idx;
    uint8_t img_idx;
	uint32_t img_id;
	uint32_t img_version;
	uint32_t img_start;
	uint32_t img_size;
} FU_Partition_Client_t;

/*************************************************************************************************/
/* global variables 																			 */
/*************************************************************************************************/
					
boolean FU_initialized = FALSE;
flash_handle_t flash_handle;
uint32_t flash_block_size;
FU_Partition_Client_t partition_handles[QAPI_FW_UPGRADE_MAX_HANDLES];

/*************************************************************************************************/
static uint32_t qapi_Fw_Upgrade_Get_FWD_Support_Num(void);
static uint32_t qapi_Fw_Upgrade_Get_First_Programming_Image_Start_Address(void);
static qapi_Status_t qapi_Fw_Upgrade_validation(void);
static flash_handle_t qapi_Fw_Upgrade_get_flash_handle(void);
static qapi_Status_t qapi_Fw_Upgrade_partition_handle_validation(qapi_Part_Hdl_t handle);
static qapi_Status_t qapi_Fw_Upgrade_flash_parameters_validation(qapi_Part_Hdl_t  handle,
                    uint32_t byte_offset, uint32_t byte_count, void *buffer);
static FU_Partition_Client_t *qapi_Fw_Upgrade_Get_Partition_Client_Entry(void);
static uint32_t qapi_Fw_Upgrade_Get_Image_End_Addr(qapi_Part_Hdl_t hdl);
static qapi_Status_t qapi_Fw_Upgrade_get_ImageSet_End_Addr(uint8_t fwd_num, uint32_t *endAddr);
static qapi_Status_t qapi_Fw_Upgrade_get_Current_Index(uint8_t *current);
static uint32_t fw_upgrade_get_wdog_reset_counter(void);
qapi_Status_t qapi_Fw_Upgrade_get_Trial_Active_FWD_Index(uint8_t *trial, uint8_t *current, uint32_t *rank);
/*********************************************************************************************/
static uint32_t fw_upgrade_get_wdog_reset_counter(void)
{
    uint32_t rtn = *((uint32_t *)WATCHDOG_RESET_COUNTER);
    return rtn; 
}
/* PBL Bug work around for Trial to Golden image rollback */
void fw_upgrade_rollback_check(void)
{
    uint8_t current_id = 0;
    uint32_t fwd_boot_type;
	   
    if (fw_upgrade_get_wdog_reset_counter() == (QAPI_FWD_WATCHDOG_RESET_COUNT_MAX - 1)) {
	 
        qapi_Fw_Upgrade_Get_Active_FWD(&fwd_boot_type, NULL);
	           
        /* Current image is trial image */
        if( fwd_boot_type == QAPI_FW_UPGRADE_FWD_BOOT_TYPE_TRIAL ) {
	           
            /* Check if CURRENT FWD is INVALID */
            if (qapi_Fw_Upgrade_get_Current_Index(&current_id) != QAPI_OK) {
                /* If invalid, then rollback to Golden image  */
                (*(uint32_t *)FORCE_BOOT_FWD_TYPE_REG) = QAPI_FWD_BOOT_TYPE_GOLDEN;
                qapi_System_Reset();
            }
        }
    }
}
	 
qapi_Status_t qapi_Fw_Upgrade_init(void)
{
    qapi_Status_t rtn;
    struct flash_info dev_info;    
    int i;
    
    /* FU init is done already */
    if( (FU_initialized == TRUE) && (flash_handle != 0)  )
        return QAPI_OK;
    
    if( flash_open(FLASH_ALL, &flash_handle) != FLASH_DEVICE_DONE )
    {
        rtn = QAPI_ERROR;
        goto init_end;
    }

    if( flash_get_info(flash_handle, &dev_info) != FLASH_DEVICE_DONE )
    {
        rtn = QAPI_ERROR;
        goto init_end;
    }

    /* Get some information about the device */
    flash_block_size = dev_info.pages_per_block * dev_info.page_size_bytes;
    
    for(i = 0; i < QAPI_FW_UPGRADE_MAX_HANDLES; i++ )
        partition_handles[i].ref_count = 0;

    FU_initialized = TRUE;
    rtn = QAPI_OK;
init_end:    
    return rtn;
}

qapi_Status_t qapi_Fw_Upgrade_Get_Flash_Block_Size(uint32_t *size)
{
    if( qapi_Fw_Upgrade_validation() != QAPI_OK )
    {
        return QAPI_ERROR_FU_NOT_INIT;
    }
    
    /* Get flash size */
    *size = flash_block_size;    
    return QAPI_OK; 
}

qapi_Status_t qapi_Fw_Upgrade_Get_Flash_Size(uint32_t *size)
{
    qapi_Status_t rtn;
    struct flash_info dev_info; 
    
    if( qapi_Fw_Upgrade_validation() != QAPI_OK )
    {
        rtn = QAPI_ERROR_FU_NOT_INIT;
        goto flash_size_end;
    }
    
    if( flash_get_info(qapi_Fw_Upgrade_get_flash_handle(), &dev_info) != FLASH_DEVICE_DONE )
    {
        rtn = QAPI_ERROR;
        goto flash_size_end;
    }

    /* Get flash size */
    *size = dev_info.block_count * dev_info.pages_per_block * dev_info.page_size_bytes;    
    rtn = QAPI_OK; 
    
flash_size_end:
    return rtn;    
}

//Firmware Descriptor APIs
qapi_Status_t qapi_Fw_Upgrade_Erase_FWD(uint8_t FWD_idx)
{
    qapi_Status_t rtn;

    if( qapi_Fw_Upgrade_validation() != QAPI_OK )
    {
        rtn = QAPI_ERROR_FU_NOT_INIT;
        goto erase_FWD_end;
    }
    
    if( FWD_idx >= QAPI_FW_UPGRADE_MAX_FWD ) {
      rtn = QAPI_ERR_INVALID_PARAM;
      goto erase_FWD_end; 
    }
    
    if( flash_erase(qapi_Fw_Upgrade_get_flash_handle(), FWD_idx, 1) != FLASH_DEVICE_DONE )
    {
        rtn = QAPI_ERROR;  
    } else {
        rtn = QAPI_OK;      
    }
                 
erase_FWD_end:
    return rtn;
}

qapi_Status_t qapi_Fw_Upgrade_Set_FWD_Magic(uint8_t FWD_idx, uint32_t magic)
{
    qapi_Status_t rtn = QAPI_OK;
    uint32_t start_addr;

    if( qapi_Fw_Upgrade_validation() != QAPI_OK )
    {
        rtn = QAPI_ERROR_FU_NOT_INIT;
        goto set_magic_end;
    }
    
    if( FWD_idx >= QAPI_FW_UPGRADE_MAX_FWD ) {
      rtn = QAPI_ERR_INVALID_PARAM;
      goto set_magic_end; 
    }

    start_addr = flash_block_size*FWD_idx + QAPI_FW_UPGRADE_FWD_SIGNATURE_OFFSET;
    
    if( flash_write(qapi_Fw_Upgrade_get_flash_handle(), start_addr, 4, (void *) &magic) != FLASH_DEVICE_DONE ) 
    {
        rtn = QAPI_ERROR;
    }
    
set_magic_end:
    return rtn;
}

qapi_Status_t qapi_Fw_Upgrade_Get_FWD_Magic(uint8_t FWD_idx, uint32_t *magic)
{
    qapi_Status_t rtn = QAPI_OK;
    uint32_t start_addr;

    if( qapi_Fw_Upgrade_validation() != QAPI_OK )
    {
        rtn = QAPI_ERROR_FU_NOT_INIT;
        goto get_magic_end;
    }
    
    if( FWD_idx >= QAPI_FW_UPGRADE_MAX_FWD ) {
      rtn = QAPI_ERR_INVALID_PARAM;
      goto get_magic_end; 
    }

    start_addr = flash_block_size*FWD_idx + QAPI_FW_UPGRADE_FWD_SIGNATURE_OFFSET;
    
    if( flash_read(qapi_Fw_Upgrade_get_flash_handle(), start_addr, 4, (void *) magic) != FLASH_DEVICE_DONE ) 
    {
        rtn = QAPI_ERROR;
    }
    
get_magic_end:
    return rtn;
}

qapi_Status_t qapi_Fw_Upgrade_Set_FWD_Rank(uint8_t FWD_idx, uint32_t rank)
{
    qapi_Status_t rtn = QAPI_OK;
    uint32_t start_addr;

    if( qapi_Fw_Upgrade_validation() != QAPI_OK )
    {
        rtn = QAPI_ERROR_FU_NOT_INIT;
        goto set_rank_end;
    }
    
    if( FWD_idx >= QAPI_FW_UPGRADE_MAX_FWD ) {
      rtn = QAPI_ERR_INVALID_PARAM;
      goto set_rank_end; 
    }

    start_addr = flash_block_size* FWD_idx + QAPI_FW_UPGRADE_FWD_RANK_OFFSET;
    
    if( flash_write(qapi_Fw_Upgrade_get_flash_handle(), start_addr, 4, (void *) &rank) != FLASH_DEVICE_DONE ) 
    {
        rtn = QAPI_ERROR;
    }
    
set_rank_end:
    return rtn;
}

qapi_Status_t qapi_Fw_Upgrade_Get_FWD_Rank(uint8_t FWD_idx, uint32_t *rank)
{
    qapi_Status_t rtn = QAPI_OK;
    uint32_t start_addr;

    if( qapi_Fw_Upgrade_validation() != QAPI_OK )
    {
        rtn = QAPI_ERROR_FU_NOT_INIT;
        goto get_rank_end;
    }
    
    if( FWD_idx >= QAPI_FW_UPGRADE_MAX_FWD ) {
      rtn = QAPI_ERR_INVALID_PARAM;
      goto get_rank_end; 
    }

    start_addr = flash_block_size*FWD_idx + QAPI_FW_UPGRADE_FWD_RANK_OFFSET;
    
    if( flash_read(qapi_Fw_Upgrade_get_flash_handle(), start_addr, 4, (void *) rank) != FLASH_DEVICE_DONE ) 
    {
        rtn = QAPI_ERROR;
    }
    
get_rank_end:
    return rtn;
}

qapi_Status_t qapi_Fw_Upgrade_Set_FWD_Version(uint8_t FWD_idx, uint32_t version)
{
    qapi_Status_t rtn = QAPI_OK;
    uint32_t start_addr;

    if( qapi_Fw_Upgrade_validation() != QAPI_OK )
    {
        rtn = QAPI_ERROR_FU_NOT_INIT;
        goto set_version_end;
    }
    
    if( FWD_idx >= QAPI_FW_UPGRADE_MAX_FWD ) {
      rtn = QAPI_ERR_INVALID_PARAM;
      goto set_version_end; 
    }

    start_addr = flash_block_size*FWD_idx + QAPI_FW_UPGRADE_FWD_VERSION_OFFSET;
    
    if( flash_write(qapi_Fw_Upgrade_get_flash_handle(), start_addr, 4, (void *) &version) != FLASH_DEVICE_DONE ) 
    {
        rtn = QAPI_ERROR;
    }
    
set_version_end:
    return rtn;
}

qapi_Status_t qapi_Fw_Upgrade_Get_FWD_Version(uint8_t FWD_idx, uint32_t *version)
{
    qapi_Status_t rtn = QAPI_OK;
    uint32_t start_addr;

    if( qapi_Fw_Upgrade_validation() != QAPI_OK )
    {
        rtn = QAPI_ERROR_FU_NOT_INIT;
        goto get_version_end;
    }
    
    if( FWD_idx >= QAPI_FW_UPGRADE_MAX_FWD ) {
      rtn = QAPI_ERR_INVALID_PARAM;
      goto get_version_end; 
    }

    start_addr = flash_block_size*FWD_idx + QAPI_FW_UPGRADE_FWD_VERSION_OFFSET;
    
    if( flash_read(qapi_Fw_Upgrade_get_flash_handle(), start_addr, 4, (void *) version) != FLASH_DEVICE_DONE ) 
    {
        rtn = QAPI_ERROR;
    }
    
get_version_end:
    return rtn;
}

qapi_Status_t qapi_Fw_Upgrade_Set_FWD_Status(uint8_t FWD_idx, uint8_t status)
{
    qapi_Status_t rtn = QAPI_OK;
    uint32_t start_addr;

    if( qapi_Fw_Upgrade_validation() != QAPI_OK )
    {
        rtn = QAPI_ERROR_FU_NOT_INIT;
        goto set_status_end;
    }
    
    if( FWD_idx >= QAPI_FW_UPGRADE_MAX_FWD ) {
      rtn = QAPI_ERR_INVALID_PARAM;
      goto set_status_end; 
    }

    start_addr = flash_block_size*FWD_idx + QAPI_FW_UPGRADE_FWD_STATUS_OFFSET;
    
    if( flash_write(qapi_Fw_Upgrade_get_flash_handle(), start_addr, 1, (void *) &status) != FLASH_DEVICE_DONE ) 
    {
        rtn = QAPI_ERROR;
    }
    
set_status_end:
    return rtn;
}

qapi_Status_t qapi_Fw_Upgrade_Get_FWD_Status(uint8_t FWD_idx, uint8_t *status)
{
    qapi_Status_t rtn = QAPI_OK;
    uint32_t start_addr;

    if( qapi_Fw_Upgrade_validation() != QAPI_OK )
    {
        rtn = QAPI_ERROR_FU_NOT_INIT;
        goto get_status_end;
    }
    
    if( FWD_idx >= QAPI_FW_UPGRADE_MAX_FWD ) {
      rtn = QAPI_ERR_INVALID_PARAM;
      goto get_status_end; 
    }

    start_addr = flash_block_size*FWD_idx + QAPI_FW_UPGRADE_FWD_STATUS_OFFSET;
    
    if( flash_read(qapi_Fw_Upgrade_get_flash_handle(), start_addr, 1, (void *) status) != FLASH_DEVICE_DONE ) 
    {
        rtn = QAPI_ERROR;
    }
    
get_status_end:
    return rtn;
}


qapi_Status_t qapi_Fw_Upgrade_Set_FWD_Total_Images(uint8_t FWD_idx, uint8_t image_nums)
{
    qapi_Status_t rtn = QAPI_OK;
    uint32_t start_addr;

    if( qapi_Fw_Upgrade_validation() != QAPI_OK )
    {
        rtn = QAPI_ERROR_FU_NOT_INIT;
        goto set_total_images_end;
    }
    
    if( FWD_idx >= QAPI_FW_UPGRADE_MAX_FWD ) {
      rtn = QAPI_ERR_INVALID_PARAM;
      goto set_total_images_end; 
    }

    start_addr = flash_block_size*FWD_idx + QAPI_FW_UPGRADE_FWD_NUM_OF_IMAGES_OFFSET;
    
    if( flash_write(qapi_Fw_Upgrade_get_flash_handle(), start_addr, 1, (void *) &image_nums) != FLASH_DEVICE_DONE ) 
    {
        rtn = QAPI_ERROR;
    }
    
set_total_images_end:
    return rtn;
}  
  
qapi_Status_t qapi_Fw_Upgrade_Get_FWD_Total_Images(uint8_t FWD_idx, uint8_t *image_nums)
{
    qapi_Status_t rtn = QAPI_OK;
    uint32_t start_addr;

    if( qapi_Fw_Upgrade_validation() != QAPI_OK )
    {
        rtn = QAPI_ERROR_FU_NOT_INIT;
        goto get_total_images_end;
    }
    
    if( FWD_idx >= QAPI_FW_UPGRADE_MAX_FWD ) {
      rtn = QAPI_ERR_INVALID_PARAM;
      goto get_total_images_end; 
    }

    start_addr = flash_block_size*FWD_idx + QAPI_FW_UPGRADE_FWD_NUM_OF_IMAGES_OFFSET;
    
    if( flash_read(qapi_Fw_Upgrade_get_flash_handle(), start_addr, 1, (void *) image_nums) != FLASH_DEVICE_DONE ) 
    {
        rtn = QAPI_ERROR;
    }
    
get_total_images_end:
    return rtn;
}

uint8_t qapi_Fw_Upgrade_Get_Active_FWD(uint32_t *fwd_boot_type, uint32_t *valid_fwd)
{
    if( fwd_boot_type != NULL )
    {
        uint32 fwd_type = *((uint32_t *)BOOT_FWD_TYPE_REG);
        switch(fwd_type) {
        case QAPI_FWD_BOOT_TYPE_GOLDEN:
          *fwd_boot_type = QAPI_FW_UPGRADE_FWD_BOOT_TYPE_GOLDEN; 
          break;
        case QAPI_FWD_BOOT_TYPE_CURRENT:
          *fwd_boot_type = QAPI_FW_UPGRADE_FWD_BOOT_TYPE_CURRENT;
          break;
        case QAPI_FWD_BOOT_TYPE_TRIAL:
          *fwd_boot_type = QAPI_FW_UPGRADE_FWD_BOOT_TYPE_TRIAL;
          break;
        }   
    } 
  
    if( valid_fwd != NULL ) *valid_fwd = *((uint32_t *)VALID_FWD_PRESET_REG);	  
    return *((uint8 *)PBL_AON_FWD_BOOT_BLOCK);
}

qapi_Status_t qapi_Fw_Upgrade_Accept_Trial_FWD(void)
{
    qapi_Status_t rtn;
    uint8_t trial, current;
    uint32_t rank, fwd_boot_type;
    
    /* reset wdog reset counter */
    qapi_System_WDTCount_Reset();
        
    rtn = qapi_Fw_Upgrade_get_Trial_Active_FWD_Index(&trial, &current, &rank);
    if(  rtn == QAPI_OK )
    {
		qapi_Fw_Upgrade_Get_Active_FWD(&fwd_boot_type, NULL);
		//only allow active FWD is trial to accept
		if( fwd_boot_type == QAPI_FW_UPGRADE_FWD_BOOT_TYPE_TRIAL )
		{	
			//set trial rank to the biggest number 
			rtn = qapi_Fw_Upgrade_Set_FWD_Rank(trial, rank+1);
			//set current FWD status to invalid except current FWD is golden
			if( (rtn == QAPI_OK) && (current != 0) ) {  
				rtn = qapi_Fw_Upgrade_Set_FWD_Status(current, QAPI_FU_FWD_STATUS_INVALID);
            }
		} else {
			rtn = QAPI_ERROR;
		}
    } 
    return rtn;  
}

qapi_Status_t qapi_Fw_Upgrade_Reject_Trial_FWD(void)
{
    qapi_Status_t rtn;
    uint8_t trial, current;
    uint32_t rank;
    
    /* reset wdog reset counter */
    qapi_System_WDTCount_Reset();
    
    rtn = qapi_Fw_Upgrade_get_Trial_Active_FWD_Index(&trial, &current, &rank);
    if(  rtn == QAPI_OK )
    {
        //set trial FWD status to invalid
        rtn = qapi_Fw_Upgrade_Set_FWD_Status(trial, QAPI_FU_FWD_STATUS_INVALID);
        //set trial FWD rank to 1 
        if( rtn == QAPI_OK ) {
            rtn = qapi_Fw_Upgrade_Set_FWD_Rank(trial, 1);
        }
    } 
    return rtn;  
}

qapi_Status_t qapi_Fw_Upgrade_Select_Trial_FWD(uint8_t *fwd_index, uint32_t *start_address, uint32_t *size)
{
    qapi_Status_t rtn = QAPI_ERROR;
    uint32_t config, flash_size, end_addr, rank;
    uint32_t img_start =0;
    uint8_t trial, current;

	/* get FWD support num */
    config = qapi_Fw_Upgrade_Get_FWD_Support_Num();
    
    /*only support one partition, can't upgrade */
    if( config == 1) return rtn;
    
	/* get whole flash size */
    if( qapi_Fw_Upgrade_Get_Flash_Size(&flash_size) != QAPI_OK )
    {
        /* can't get flash size */
        return rtn;
    }

	/* get the start addess of app image except file system image */
	img_start = qapi_Fw_Upgrade_Get_First_Programming_Image_Start_Address();
	/* get current FWD index num and biggest rank num */
    qapi_Fw_Upgrade_get_Trial_Active_FWD_Index(&trial, &current, &rank);
    
    if( config == 2)  /* support 2 FWDs case */
    {
		/* set trial FWD index num */
        *fwd_index = current==0?1:0;
		/* we need set size and start address align with flash_block_size */
		flash_size = (flash_size - img_start)/ flash_block_size;
		*size = flash_size/config*flash_block_size;
		*start_address = img_start+(*fwd_index)*(*size);
		if( *fwd_index == 1) 
		{	
			if( qapi_Fw_Upgrade_get_ImageSet_End_Addr(0, &end_addr) != QAPI_OK )
			{	
				/* can't get fwd info */
				return rtn;
			}
		
			if( *start_address < end_addr )
			{
				*size = *size - (end_addr - *start_address);
				*start_address = end_addr;
			}
		}
    } else {   /* support 3 (golgen+2) FWDs case */
        *fwd_index = current==0?1:(current==1)?2:1;        
		if( qapi_Fw_Upgrade_get_ImageSet_End_Addr(0, &end_addr) != QAPI_OK )
		{
			/* can't get fwd info */
			return rtn;
		}
		
		/* we need set size and start address align with flash_block_size */
		flash_size = (flash_size - end_addr)/ flash_block_size;
		*size = flash_size/(config-1)*flash_block_size;
		*start_address = end_addr+(*fwd_index-1)*(*size);
    } 
    return QAPI_OK;   
}

qapi_Status_t qapi_Fw_Upgrade_Format_Partition(uint8_t FWD_idx, uint32_t format_size)
{
    qapi_Status_t rtn = QAPI_ERROR;    
    uint32_t config, flash_size, img_start;
    uint32_t partition_size, partition_size_in_block;
    uint32_t start_block, block_count;
   
    config = qapi_Fw_Upgrade_Get_FWD_Support_Num();
    
    if( qapi_Fw_Upgrade_Get_Flash_Size(&flash_size) != QAPI_OK )
    {
        /* can't get flash size */
        return rtn;
    }
    
    img_start = qapi_Fw_Upgrade_Get_First_Programming_Image_Start_Address();
    
    /* we need set size and start address align with flash_block_size */
    partition_size = (flash_size-img_start) / config;
    partition_size_in_block = partition_size / flash_block_size;
    start_block = (img_start + partition_size * FWD_idx) / flash_block_size;

    if( format_size == 0)
    {
        block_count = partition_size_in_block;
    } else {
        if( (format_size % flash_block_size) != 0)
            format_size += flash_block_size;

        block_count = format_size / flash_block_size;

        //check if the erase size is less than partition size
        if( block_count > partition_size_in_block)
            return rtn;
    }
    
    if( flash_erase(qapi_Fw_Upgrade_get_flash_handle(), start_block,  block_count) != FLASH_DEVICE_DONE )
    {
        rtn = QAPI_ERROR;  
    } else {
        rtn = QAPI_OK;      
    }
    return rtn;
}

/* 
 * this function is to get the flash start address for the first image after FS1 and FS2
 */ 
static uint32_t qapi_Fw_Upgrade_Get_First_Programming_Image_Start_Address(void)
{
    qapi_Part_Hdl_t  hdl;    
    uint32_t img1_size =0, img1_start =0, img_start = 0;
    uint8_t  current;

    current = qapi_Fw_Upgrade_Get_Active_FWD(NULL, NULL);
    
    /* get FS1 info */
    if( qapi_Fw_Upgrade_Find_Partition(current, FS1_IMG_ID, &hdl) == QAPI_OK )
    {
        qapi_Fw_Upgrade_Get_Partition_Size(hdl, &img1_size);
        qapi_Fw_Upgrade_Get_Partition_Start(hdl, &img1_start);
        qapi_Fw_Upgrade_Close_Partition(hdl);
        img_start = img1_start + img1_size;
    }
    
    /* get FS2 info */
    if( qapi_Fw_Upgrade_Find_Partition(current, FS2_IMG_ID, &hdl) == QAPI_OK )
    {
        qapi_Fw_Upgrade_Get_Partition_Size(hdl, &img1_size);
        qapi_Fw_Upgrade_Get_Partition_Start(hdl, &img1_start);
        qapi_Fw_Upgrade_Close_Partition(hdl);
        img1_start += img1_size;
    }
    
    //pick the bigger one
    if( img1_start > img_start) img_start = img1_start;

    //make sure the start address should be bigger than FWDs
    img1_start = qapi_Fw_Upgrade_Get_FWD_Support_Num() * flash_block_size;
    if( img1_start > img_start) img_start = img1_start;

    return img_start;
}

static uint32_t qapi_Fw_Upgrade_Get_FWD_Support_Num(void)
{
	uint32_t config;
	
    //get FWD support num 
	fw_Upgrade_Get_Scheme_Param(FW_UPGRADE_SCHEME_PROP_FWD_SUPPORT_NUM_ID, &config );

    //only support 2 FWDs and 1+2 FWDs
	if( config != 2 ) config = 3;
    return config;
}

static qapi_Status_t qapi_Fw_Upgrade_validation()
{
    if(FU_initialized == TRUE )
        return QAPI_OK;
    else
        return QAPI_ERROR;
}

static flash_handle_t qapi_Fw_Upgrade_get_flash_handle()
{
    return flash_handle; 
}

static qapi_Status_t qapi_Fw_Upgrade_partition_handle_validation(qapi_Part_Hdl_t handle)
{
    FU_Partition_Client_t *h = PARTITION_CLIENT_HANDLE_START;
  
    while (h <= PARTITION_CLIENT_HANDLE_END)
    {
        if (((FU_Partition_Client_t *)handle == h) && (h->ref_count > 0))
        {
        return QAPI_OK;
        }
        h++;
    }

    return QAPI_ERROR;
}

static qapi_Status_t qapi_Fw_Upgrade_flash_parameters_validation (qapi_Part_Hdl_t  handle, uint32_t byte_offset, uint32_t byte_count, void *buffer)
{
  uint32_t limit = 0;
  qapi_Status_t rtn = QAPI_OK;
  FU_Partition_Client_t *private = NULL;

  if( (handle == NULL) || (buffer == NULL) || (byte_count == 0) )
  {
    rtn  = QAPI_ERR_INVALID_PARAM;
    goto end1;
  }
  
  if( qapi_Fw_Upgrade_partition_handle_validation(handle) != QAPI_OK )
  {
    rtn = QAPI_ERR_INVALID_PARAM;
    goto end1;
  }

  private = (FU_Partition_Client_t *)handle;

  limit = private->img_start + private->img_size;

  /* Boundary Check */
  if ((private->img_start + byte_offset + byte_count) > limit)
  {
    rtn = QAPI_ERR_INVALID_PARAM;
    goto end1;
  }

end1:
  return rtn;
}

static FU_Partition_Client_t *qapi_Fw_Upgrade_Get_Partition_Client_Entry()
{
    uint32_t i;

    i = 0;
    while( i < QAPI_FW_UPGRADE_MAX_HANDLES )
    {
        if( partition_handles[i].ref_count == 0 )
          break;
        i++;
    }
    
    //no entry available
    if( i >= QAPI_FW_UPGRADE_MAX_HANDLES )
    {
       return NULL;
    } else {
       return (&partition_handles[i]);
    }
}

static uint32_t qapi_Fw_Upgrade_Get_Image_End_Addr(qapi_Part_Hdl_t hdl)
{
    uint32_t start, size;
    qapi_Fw_Upgrade_Get_Partition_Start(hdl, &start);  
    qapi_Fw_Upgrade_Get_Partition_Size(hdl, &size);
	return (start+size);
}

static qapi_Status_t qapi_Fw_Upgrade_get_ImageSet_End_Addr(uint8_t fwd_num, uint32_t *endAddr)
{
	qapi_Status_t 	rtn = QAPI_ERROR;
	uint32_t        magic, rtnAddr;
	qapi_Part_Hdl_t hdl = NULL, hdl_next;
   
	qapi_Fw_Upgrade_Get_FWD_Magic(fwd_num, &magic);	
    if( magic == QAPI_FW_UPGRADE_MAGIC_V1 ) 
    {
        if( qapi_Fw_Upgrade_First_Partition(fwd_num, &hdl) == QAPI_OK )
        {
            *endAddr = qapi_Fw_Upgrade_Get_Image_End_Addr(hdl);
            while (qapi_Fw_Upgrade_Next_Partition(hdl, &hdl_next) == QAPI_OK )
            {
                qapi_Fw_Upgrade_Close_Partition(hdl);
                hdl = hdl_next;
                rtnAddr = qapi_Fw_Upgrade_Get_Image_End_Addr(hdl);
				if( rtnAddr > *endAddr)
				{
					*endAddr = rtnAddr;
				}
            }
            qapi_Fw_Upgrade_Close_Partition(hdl);
			rtn = QAPI_OK;
        }
    }	
	
	return rtn;
}

qapi_Status_t qapi_Fw_Upgrade_get_Current_Index(uint8_t *current)
{
    qapi_Status_t rtn = QAPI_ERROR;
    uint32_t config, magic, rank_l, i;
    uint8_t status, image_nums;
    
    /* init return */
    *current = QAPI_FU_FWD_STATUS_UNUSED;

	/* init FW upgrade module */
	if( qapi_Fw_Upgrade_init() != QAPI_OK)
		goto get_cur_end;	
    
    /* we support 2 FWDs and 1 GLN+2FWDs mode */
    config = qapi_Fw_Upgrade_Get_FWD_Support_Num();
    
    /* verify FWD 1 and FWD 2 */
    for( i = 0; i < config; i++ )
    {
        if( qapi_Fw_Upgrade_Get_FWD_Magic(i, &magic) != QAPI_OK )
            goto get_cur_end;
        if( qapi_Fw_Upgrade_Get_FWD_Rank(i, &rank_l) != QAPI_OK )
            goto get_cur_end;
        if( qapi_Fw_Upgrade_Get_FWD_Status(i, &status) != QAPI_OK )
            goto get_cur_end;
        if( qapi_Fw_Upgrade_Get_FWD_Total_Images(i, &image_nums) != QAPI_OK )
            goto get_cur_end;

        if( (magic == QAPI_FW_UPGRADE_MAGIC_V1) && (status == QAPI_FU_FWD_STATUS_VALID) && (image_nums > 0) && (image_nums != 0xff) )  
        {
            if( (rank_l != QAPI_FU_FWD_RANK_TRIAL) && (rank_l  > 0) )
            {
                *current = i;
                break;
            } 
        }
    }
 
    // if *current is QAPI_FU_FWD_STATUS_UNUSED, it means we don't find current FWD
    if( *current != QAPI_FU_FWD_STATUS_UNUSED ) 
        rtn = QAPI_OK;
    
get_cur_end:    
    return rtn;    
}

qapi_Status_t qapi_Fw_Upgrade_get_Trial_Active_FWD_Index(uint8_t *trial, uint8_t *current, uint32_t *rank)
{
    qapi_Status_t rtn = QAPI_ERROR;
    uint32_t config, magic, rank_l, i, s1, s2;
    uint8_t status, image_nums;
    
    /* init return */
    *trial = QAPI_FU_FWD_TRIAL_UNUSED;
    *current = 0;
    *rank = 0;

	/* init FW upgrade module */
	if( qapi_Fw_Upgrade_init() != QAPI_OK)
		goto cmd_get_index_end;		
    
    /* we support 2 FWDs and 1 GLN+2FWDs mode */
    config = qapi_Fw_Upgrade_Get_FWD_Support_Num();
	
    if( config == 1) 
        goto cmd_get_index_end;

    //two FWDs
    if( config == 2) {
        s1 = 0;
        s2 = 2;
    } else {  //three FWDs
        qapi_Fw_Upgrade_Get_FWD_Rank(0, &rank_l);
        *rank = rank_l;
        
        s1 = 1;
        s2 = 3;
    }
    
    /* verify FWD 1 and FWD 2 */
    for( i = s1; i < s2; i++ )
    {
        if( qapi_Fw_Upgrade_Get_FWD_Magic(i, &magic) != QAPI_OK )
            goto cmd_get_index_end;
        if( qapi_Fw_Upgrade_Get_FWD_Rank(i, &rank_l) != QAPI_OK )
            goto cmd_get_index_end;
        if( qapi_Fw_Upgrade_Get_FWD_Status(i, &status) != QAPI_OK )
            goto cmd_get_index_end;
        if( qapi_Fw_Upgrade_Get_FWD_Total_Images(i, &image_nums) != QAPI_OK )
            goto cmd_get_index_end;

        if( (magic == QAPI_FW_UPGRADE_MAGIC_V1) && (status == QAPI_FU_FWD_STATUS_VALID) && (image_nums > 0) && (image_nums != 0xff) )  
        {
            if( rank_l == QAPI_FU_FWD_RANK_TRIAL)
            {
                *trial = i;
            } else {
                if( rank_l > *rank ) {
                    *rank = rank_l;
                    *current = i;
                }
            }
        }
    }
 
    // if *trial is QAPI_FU_FWD_TRIAL_UNUSED, it means we don't find trial FWD
    if( *trial != QAPI_FU_FWD_TRIAL_UNUSED ) 
        rtn = QAPI_OK;
    
cmd_get_index_end:
    return rtn;
}

/******************************************************************************************************************************/
qapi_Status_t qapi_Fw_Upgrade_Create_Partition(uint8_t FWD_idx, uint32_t id, uint32_t version, uint32_t start, uint32_t size, qapi_Part_Hdl_t *hdl)
{
    qapi_Status_t rtn = QAPI_OK;
    uint32_t start_addr, i;
    fwd fwd;
    image_entry img_entry;
    FU_Partition_Client_t *client_entry = NULL;
    
    if( qapi_Fw_Upgrade_validation() != QAPI_OK )
    {
        rtn = QAPI_ERROR_FU_NOT_INIT;
        goto create_partition_end;
    }
    
    if( FWD_idx >= QAPI_FW_UPGRADE_MAX_FWD )
    {
      rtn = QAPI_ERR_INVALID_PARAM;
      goto create_partition_end; 
    }

    //check start addr and size
    if ( (start % flash_block_size) != 0 || (size % flash_block_size) != 0 )
    {
      rtn = QAPI_ERR_INVALID_PARAM;
      goto create_partition_end; 
    }
    
    //get valid client entry
    if( (client_entry = qapi_Fw_Upgrade_Get_Partition_Client_Entry()) == NULL )
    {
       rtn = QAPI_ERROR;
       goto create_partition_end;        
    }
    
    start_addr = flash_block_size*FWD_idx;
    if( flash_read(qapi_Fw_Upgrade_get_flash_handle(), start_addr, sizeof(fwd), (void *) &fwd) != FLASH_DEVICE_DONE ) 
    {
        rtn = QAPI_ERROR;
        goto create_partition_end; 
    }

    //find valid free entry
    for(i = 0; i < MAX_FW_IMAGE_ENTRIES; i++)
    {
        if( fwd.image_entries[i].image_id == QAPI_FU_FWD_IMAGE_UNUSED )
            break;
    }
    
    // there is no free image entry
    if( i >= MAX_FW_IMAGE_ENTRIES ) 
    {
        rtn = QAPI_ERROR;
        goto create_partition_end; 
    }

    start_addr = start_addr + 32 + i*sizeof(image_entry);
    
    memset((void *) &img_entry, 0xff, sizeof(img_entry));
    img_entry.image_id = id;
    memcpy(&img_entry.img_version, &version, 4);
    img_entry.start_block = start / flash_block_size ;
    img_entry.total_blocks = size / flash_block_size;
    
    if( flash_write(qapi_Fw_Upgrade_get_flash_handle(), start_addr, sizeof(image_entry), (void *) &img_entry) != FLASH_DEVICE_DONE ) 
    {
        rtn = QAPI_ERROR;
        goto create_partition_end; 
    }    
    
    client_entry->FWD_idx = FWD_idx; 
    client_entry->img_idx = i; 
    client_entry->img_id = id;
    client_entry->img_version = version;
    client_entry->img_start = start;
    client_entry->img_size = size;
    client_entry->ref_count = 1; 
    
    *hdl = client_entry;
    
create_partition_end:
    return rtn;
}

qapi_Status_t qapi_Fw_Upgrade_Close_Partition(qapi_Part_Hdl_t hdl)
{
    FU_Partition_Client_t *h = PARTITION_CLIENT_HANDLE_START;
  
    while (h <= PARTITION_CLIENT_HANDLE_END)
    {
        if (((FU_Partition_Client_t *)hdl == h) && (h->ref_count > 0))
        {
            break;
        }
        h++;
    }
    
    if( h >= PARTITION_CLIENT_HANDLE_END )
        return QAPI_ERROR;

    h->ref_count = 0;
    return QAPI_OK;
}

qapi_Status_t qapi_Fw_Upgrade_First_Partition(uint8_t FWD_idx, qapi_Part_Hdl_t *hdl)
{
    qapi_Status_t rtn = QAPI_OK;
    uint32_t start_addr;
    fwd fwd;
    FU_Partition_Client_t *client_entry = NULL;
        
    if( qapi_Fw_Upgrade_validation() != QAPI_OK )
    {
        rtn = QAPI_ERROR_FU_NOT_INIT;
        goto first_partition_end;
    }
    
    if( FWD_idx >= QAPI_FW_UPGRADE_MAX_FWD )
    {
      rtn = QAPI_ERR_INVALID_PARAM;
      goto first_partition_end; 
    }

    if( (client_entry = qapi_Fw_Upgrade_Get_Partition_Client_Entry()) == NULL )
    {
       rtn = QAPI_ERROR;
       goto first_partition_end;        
    }
    
    //read FWD
    start_addr = flash_block_size*FWD_idx;
    if( flash_read(qapi_Fw_Upgrade_get_flash_handle(), start_addr, sizeof(fwd), (void *) &fwd) != FLASH_DEVICE_DONE ) 
    {
        rtn = QAPI_ERROR;
        goto first_partition_end; 
    }

    if( fwd.total_images == 0 )
    {
        rtn = QAPI_ERROR;
        goto first_partition_end; 
    }
    
    client_entry->FWD_idx = FWD_idx; 
    client_entry->img_idx = 0; 
    client_entry->img_id = fwd.image_entries[0].image_id;
    memcpy(&client_entry->img_version, &fwd.image_entries[0].img_version, 4);
    client_entry->img_start = fwd.image_entries[0].start_block * flash_block_size;
    client_entry->img_size = fwd.image_entries[0].total_blocks * flash_block_size;
    client_entry->ref_count = 1; 
    
    *hdl = client_entry;
    
first_partition_end:
    return rtn;
}

qapi_Status_t qapi_Fw_Upgrade_Next_Partition(qapi_Part_Hdl_t curr, qapi_Part_Hdl_t *hdl)
{
    qapi_Status_t rtn = QAPI_OK;
    uint32_t start_addr, idx;
    fwd fwd;
    FU_Partition_Client_t *h, *client_entry = NULL;
    
    if( qapi_Fw_Upgrade_validation() != QAPI_OK )
    {
        rtn = QAPI_ERROR_FU_NOT_INIT;
        goto next_partition_end;
    }
    
    if( qapi_Fw_Upgrade_partition_handle_validation(curr) != QAPI_OK )
    {
        rtn = QAPI_ERR_INVALID_PARAM;
        goto next_partition_end;
    }
    
    h = (FU_Partition_Client_t *)curr;
    
    if( (client_entry = qapi_Fw_Upgrade_Get_Partition_Client_Entry()) == NULL )
    {
       rtn = QAPI_ERROR;
       goto next_partition_end;        
    }
    
    start_addr = flash_block_size * h->FWD_idx;
    if( flash_read(qapi_Fw_Upgrade_get_flash_handle(), start_addr, sizeof(fwd), (void *) &fwd) != FLASH_DEVICE_DONE ) 
    {
        rtn = QAPI_ERROR;
        goto next_partition_end; 
    }

    //check if is valid id
    idx = h->img_idx+1;
    if( idx >= MAX_FW_IMAGE_ENTRIES )
    {
        rtn = QAPI_ERROR;
        goto next_partition_end; 
    }
    
    if( fwd.image_entries[idx].image_id  == QAPI_FU_FWD_IMAGE_UNUSED )
    {
        rtn = QAPI_ERROR;
        goto next_partition_end; 
    }
    
    client_entry->FWD_idx = h->FWD_idx; 
    client_entry->img_idx = idx; 
    client_entry->img_id = fwd.image_entries[idx].image_id;
    memcpy(&client_entry->img_version, &fwd.image_entries[idx].img_version, 4);
    client_entry->img_start = fwd.image_entries[idx].start_block * flash_block_size;
    client_entry->img_size = fwd.image_entries[idx].total_blocks * flash_block_size;
    client_entry->ref_count = 1;

    *hdl = client_entry;
    
next_partition_end:
    return rtn;
}


qapi_Status_t qapi_Fw_Upgrade_Find_Partition(uint8_t FWD_idx, uint32_t id, qapi_Part_Hdl_t *hdl)
{
    qapi_Status_t rtn = QAPI_OK;
    uint32_t start_addr, i;
    fwd fwd;
    FU_Partition_Client_t *client_entry = NULL;    

    if( qapi_Fw_Upgrade_validation() != QAPI_OK )
    {
        rtn = QAPI_ERROR_FU_NOT_INIT;
        goto find_partition_end;
    }
    
    if( FWD_idx >= QAPI_FW_UPGRADE_MAX_FWD )
    {
      rtn = QAPI_ERR_INVALID_PARAM;
      goto find_partition_end; 
    }

    if( (client_entry = qapi_Fw_Upgrade_Get_Partition_Client_Entry()) == NULL )
    {
       rtn = QAPI_ERROR;
       goto find_partition_end;        
    }
    
    start_addr = flash_block_size*FWD_idx;
    if( flash_read(qapi_Fw_Upgrade_get_flash_handle(), start_addr, sizeof(fwd), (void *) &fwd) != FLASH_DEVICE_DONE ) 
    {
        rtn = QAPI_ERROR;
        goto find_partition_end; 
    }

    //find valid entry
    for(i = 0; i < MAX_FW_IMAGE_ENTRIES; i++)
    {
        if( fwd.image_entries[i].image_id == id )
            break;
    }
    
    // there is no free image entry
    if( i >= MAX_FW_IMAGE_ENTRIES ) 
    {
        rtn = QAPI_ERROR;
        goto find_partition_end; 
    }

    client_entry->FWD_idx = FWD_idx; 
    client_entry->img_idx = i; 
    client_entry->img_id = id;
    memcpy(&client_entry->img_version, &fwd.image_entries[i].img_version, 4);
    client_entry->img_start = fwd.image_entries[i].start_block * flash_block_size;
    client_entry->img_size = fwd.image_entries[i].total_blocks * flash_block_size;
    client_entry->ref_count = 1;

    *hdl = client_entry;
    
find_partition_end:
    return rtn;
}

qapi_Status_t qapi_Fw_Upgrade_Get_Image_ID(qapi_Part_Hdl_t hdl, uint32_t *id)
{
    FU_Partition_Client_t *h;
    
    if( qapi_Fw_Upgrade_partition_handle_validation(hdl) != QAPI_OK )
        return QAPI_ERR_INVALID_PARAM;
    
    h = (FU_Partition_Client_t *)hdl;
    *id = h->img_id;
    return QAPI_OK;
}

qapi_Status_t qapi_Fw_Upgrade_Set_Image_ID(qapi_Part_Hdl_t *hdl, uint32_t id)
{
    qapi_Status_t rtn = QAPI_OK;
    uint32_t start_addr, new_id;
    uint8_t FWD_idx;
    FU_Partition_Client_t *client_entry = (FU_Partition_Client_t *)hdl;
    
    if( qapi_Fw_Upgrade_validation() != QAPI_OK )
    {
        rtn = QAPI_ERROR_FU_NOT_INIT;
        goto set_image_id_end;
    }
    
    qapi_Fw_Upgrade_Get_Partition_FWD(hdl, &FWD_idx); 
    if( FWD_idx >= QAPI_FW_UPGRADE_MAX_FWD )
    {
      rtn = QAPI_ERR_INVALID_PARAM;
      goto set_image_id_end; 
    }

    start_addr = flash_block_size*FWD_idx + 32 + client_entry->img_idx*sizeof(image_entry);
    new_id = id;
    
    if( flash_write(qapi_Fw_Upgrade_get_flash_handle(), start_addr, sizeof(uint32_t), (void *) &new_id) != FLASH_DEVICE_DONE ) 
    {
        rtn = QAPI_ERROR;
        goto set_image_id_end; 
    }
    
    client_entry->img_id = new_id;
    
set_image_id_end:
    return rtn;
}

qapi_Status_t qapi_Fw_Upgrade_Get_Image_Version(qapi_Part_Hdl_t hdl, uint32_t *version)
{
    FU_Partition_Client_t *h = NULL;
    
    if( qapi_Fw_Upgrade_partition_handle_validation(hdl) != QAPI_OK )
        return QAPI_ERR_INVALID_PARAM;
    
    h = (FU_Partition_Client_t *)hdl;
    *version = h->img_version;
    return QAPI_OK;
}

qapi_Status_t qapi_Fw_Upgrade_Set_Image_Version(qapi_Part_Hdl_t *hdl, uint32_t version)
{
    qapi_Status_t rtn = QAPI_OK;
    uint32_t start_addr, new_id;
    uint8_t FWD_idx;
    FU_Partition_Client_t *client_entry = (FU_Partition_Client_t *)hdl;
    
    if( qapi_Fw_Upgrade_validation() != QAPI_OK )
    {
        rtn = QAPI_ERROR_FU_NOT_INIT;
        goto set_image_id_end;
    }
    
    qapi_Fw_Upgrade_Get_Partition_FWD(hdl, &FWD_idx); 
    if( FWD_idx >= QAPI_FW_UPGRADE_MAX_FWD )
    {
      rtn = QAPI_ERR_INVALID_PARAM;
      goto set_image_id_end; 
    }

    start_addr = (flash_block_size * FWD_idx + 32) + (client_entry->img_idx * sizeof(image_entry) + 12);
    
    if( flash_write(qapi_Fw_Upgrade_get_flash_handle(), start_addr, sizeof(uint32_t), (void *) &version) != FLASH_DEVICE_DONE ) 
    {
        rtn = QAPI_ERROR;
        goto set_image_id_end; 
    }
    
    client_entry->img_id = new_id;
    
set_image_id_end:
    return rtn;
}

qapi_Status_t qapi_Fw_Upgrade_Get_Partition_Size(qapi_Part_Hdl_t hdl, uint32_t *size)
{
    FU_Partition_Client_t *h;
    
    if( qapi_Fw_Upgrade_partition_handle_validation(hdl) != QAPI_OK )
        return QAPI_ERR_INVALID_PARAM;
    
    h = (FU_Partition_Client_t *)hdl;
    *size = h->img_size;
    return QAPI_OK;
}

qapi_Status_t qapi_Fw_Upgrade_Get_Partition_Start(qapi_Part_Hdl_t hdl, uint32_t *start)
{
    FU_Partition_Client_t *h;
    
    if( qapi_Fw_Upgrade_partition_handle_validation(hdl) != QAPI_OK )
        return QAPI_ERR_INVALID_PARAM;
    
    h = (FU_Partition_Client_t *)hdl;
    *start = h->img_start;
    return QAPI_OK;
}

qapi_Status_t qapi_Fw_Upgrade_Get_Partition_FWD(qapi_Part_Hdl_t hdl, uint8_t *FWD_idx)
{
    FU_Partition_Client_t *h;
    
    if( qapi_Fw_Upgrade_partition_handle_validation(hdl) != QAPI_OK )
        return QAPI_ERR_INVALID_PARAM;
    
    h = (FU_Partition_Client_t *)hdl;
    *FWD_idx = h->FWD_idx;
    return QAPI_OK;
}

qapi_Status_t qapi_Fw_Upgrade_Erase_Partition(qapi_Part_Hdl_t hdl, uint32_t offset, uint32_t nbytes)
{
    qapi_Status_t rtn = QAPI_OK;
    FU_Partition_Client_t *h;
    uint32_t start_block, total_blocks;
    
    if( qapi_Fw_Upgrade_partition_handle_validation(hdl) != QAPI_OK )
    {
        rtn = QAPI_ERR_INVALID_PARAM;
        goto erase_partition_end;
    }
    
    if( (nbytes == 0) || (nbytes % flash_block_size != 0) || (offset % flash_block_size != 0 ) )
    {
        rtn = QAPI_ERR_INVALID_PARAM;
        goto erase_partition_end;
    }
 
    h = (FU_Partition_Client_t *)hdl;
    if( offset + nbytes > h->img_size )
    {
        rtn = QAPI_ERR_INVALID_PARAM;
        goto erase_partition_end;
    }
    
    start_block = (h->img_start+offset) / flash_block_size;
    total_blocks = nbytes / flash_block_size;
    
    if( flash_erase(qapi_Fw_Upgrade_get_flash_handle(), start_block, total_blocks) != FLASH_DEVICE_DONE )
    {
        rtn = QAPI_ERROR;  
    }

erase_partition_end: 
    return rtn;
}

qapi_Status_t qapi_Fw_Upgrade_Write_Partition(qapi_Part_Hdl_t hdl, uint32_t offset, char *buf, uint32_t nbytes)
{
    qapi_Status_t rtn = QAPI_OK;
    FU_Partition_Client_t *h;
    uint32_t start_addr;
    
    if( qapi_Fw_Upgrade_flash_parameters_validation(hdl, offset, nbytes, buf) != QAPI_OK )
        return QAPI_ERR_INVALID_PARAM;
    
    h = (FU_Partition_Client_t *)hdl;
    start_addr = h->img_start + offset;
    
    if( flash_write(qapi_Fw_Upgrade_get_flash_handle(), start_addr, nbytes, (void *) buf) != FLASH_DEVICE_DONE ) 
    {
        rtn = QAPI_ERROR;
    }
    
    return rtn;    
  
}

qapi_Status_t qapi_Fw_Upgrade_Read_Partition(qapi_Part_Hdl_t hdl, uint32_t offset, char *buf, uint32_t max_bytes, uint32_t *nbytes)
{
    qapi_Status_t rtn = QAPI_OK;
    FU_Partition_Client_t *h;
    uint32_t start_addr;
    
    *nbytes = 0;
    
    if( qapi_Fw_Upgrade_flash_parameters_validation(hdl, offset, max_bytes, buf) != QAPI_OK )
        return QAPI_ERR_INVALID_PARAM;
    
    h = (FU_Partition_Client_t *)hdl;
    start_addr = h->img_start + offset;

    if( flash_read(qapi_Fw_Upgrade_get_flash_handle(), start_addr, max_bytes, (void *) buf) != FLASH_DEVICE_DONE ) 
    {
        rtn = QAPI_ERROR;
    }
    
    *nbytes = max_bytes;
    
    return rtn;
}

/**
 * @brief Get the size of the OEM Reserved area in bytes
 *
 * @details This is defined as the area on flash between the end of the last 
 *          Firmware Descriptor and the start of the block owned by some partition. 
 *          A value of 0 indicates that there is no reserved space. 
 *          An OEM may also choose to reserve space at the end of flash 
 *          or wherever it would like (as long as the OEM’s FW Upgrader is aware of the reservation).
 *
 * @param[out] nbytes    the size of the OEM Reserved area in bytes.
 * 
 * @return On success, QAPI_OK is returned. On error, QAPI_ERROR is returned.
 */

//TODO
qapi_Status_t qapi_Fw_Upgrade_Get_Reserved_OEM_Space(uint32_t *nbytes)
{
    *nbytes = 0;
    return QAPI_OK;     
}

/**
 * @brief   Get the total number of bytes of unused flash space between two flash offsets
 *          and the size of the largest contiguous area of free space within that range
 *
 * @details  A byte is considered free if it is not in use. A byte is in use if it belongs 
 *           to a flash block that is in use. 
 *           A flash block is in use if it belongs to a valid partition which belongs to 
 *           one or more valid FWDs.
 *
 * @param[in] lower_bound    the start offset of range to get MAX free space
 *
 * @param[in] upper_bound    the end offset of range to get MAX free space
 *
 * @param[out] free_bytes    the total number of bytes of unused flash space between two flash offsets
 *
 * @param[out] config_free_bytes    the size of the largest contiguous area of free space within that range
 * 
 * @return On success, QAPI_OK is returned. On error, QAPI_ERROR is returned.
 */

//TODO
qapi_Status_t qapi_Fw_Upgrade_Get_Max_Free_Space(uint32_t lower_bound, uint32_t upper_bound, uint32_t *free_bytes, uint32_t *config_free_bytes)
{
    return QAPI_OK;     
}

/**
 * @brief Get the start offset of a range of desired_sz bytes of free space on flash within the specified range.
 *
 * @details 
 *
 * @param[in] lower_bound    the start offset of range to get free space
 *
 * @param[in] upper_bound    the end offset of range to get free space
 *
 * @param[in] desired_sz    the desired total number of bytes of free flash space between two flash offsets
 *
 * @param[out] free_offset    the start offset for the free space within that range
 * 
 * @return On success, QAPI_OK is returned. On error, QAPI_ERROR is returned.
 */

//TODO
qapi_Status_t qapi_Fw_Upgrade_Find_Flash_Free_Space(uint32_t lower_bound, uint32_t upper_bound, uint32_t desired_sz, uint32_t *free_offset)
{
    return QAPI_OK;     
}

/**
 * @brief Initiates a reboot
 *
 * @details
 *
 * @return On success, QAPI_OK is returned. On error, QAPI_ERROR is returned.
 */

qapi_Status_t qapi_Fw_Upgrade_Reboot_System(void)
{
    //restart system ...........
    platform_restart_device();
    
    return QAPI_OK;     
}

/**
 * @brief    Creates/Defines a partition which starts at the specified flash offset
*            and has the specified size. 
 *
 * @details  This is exactly like qapi_Fw_Upgrade_Create_Partition except it permits 
 *           overlaps with an existing partition. 
 *           The caller may choose to use the values for ID, start, and size 
 *           which match an existing partition. 
 *           This creates a second reference to that same physical partition. 
 *           The caller may alter any of these parameters in sensible ways, 
 *           though it would be unusual to alter size and extremely unusual 
 *           to alter start.
 *
 * @param[in] FWD_idx    firmware descriptor index number to operate
 *
 * @param[in] id         partition ID
 *
 * @param[in] start      start flash offset for the partition
 *
 * @param[in] size       partition size
 *
 * @param[out] hdl       partition handle for partition operation
 *
 * @return On success, QAPI_OK is returned. On error, QAPI_ERROR is returned.
 */

//TODO
qapi_Status_t qapi_Fw_Upgrade_Reference_Partition(uint8_t FWD_idx, uint32_t id, uint32_t start, uint32_t size, qapi_Part_Hdl_t *hdl)
{
    
    return QAPI_ERROR;
}

//TODO
qapi_Status_t qapi_Fw_Upgrade_Validate_Partition(qapi_Part_Hdl_t hdl)
{
	return QAPI_ERROR;	
}
//TODO
qapi_Status_t qapi_Fw_Upgrade_Invalidate_Partition(qapi_Part_Hdl_t hdl)
{
	return QAPI_ERROR;	
}

/**
 * @brief Scan backwards from the end of the specified partition until a non-blank block is found. 
 *
 * @details  Return the partition-relative byte offset of the start of that block. 
 *           This API is intended to support continuation after interrupted operation 
 *           so that the FW Upgrader can discover where it left off.
 *
 * @param[in] hdl       partition handle
 *
 * @param[out] offset    offset of the last written block.
 *
 * @return On success, QAPI_OK is returned. On error, QAPI_ERROR is returned.
 */

//TODO
qapi_Status_t qapi_Fw_Upgrade_Find_Last_Written_Partition_Block(qapi_Part_Hdl_t hdl, uint32_t *offset)
{
	return QAPI_ERROR;	
}

/**
 * @brief    Check active FWD to invalidate trial fwd if need
 *
 * @details  if system boots from current or golden FWD and trial FWD is present, 
 *           need to invalidate trial FWD
 *
 * @return On success, QAPI_OK is returned. On error, QAPI_ERROR is returned.
 */
qapi_Status_t qapi_Fw_Upgrade_Verify_FWD()
{
	/* check active FWD to invalidate trial fwd if need                */
    uint32_t fwd_boot_type, valid_fwd;
    uint8_t current;
    
	qapi_Fw_Upgrade_Get_Active_FWD(&fwd_boot_type, &valid_fwd);
		
    /* if boot from current or golden and trial fwd is present, need to invalidate trial */ 
    if( (fwd_boot_type != QAPI_FW_UPGRADE_FWD_BOOT_TYPE_TRIAL) && ((valid_fwd & (1<<QAPI_FW_UPGRADE_FWD_BIT_TRIAL)) != 0) ) 
	{
	    /* invalidate trial fwd */
		qapi_Fw_Upgrade_Reject_Trial_FWD();
	} else if( (fwd_boot_type == QAPI_FW_UPGRADE_FWD_BOOT_TYPE_GOLDEN) && ((valid_fwd & (1<<QAPI_FW_UPGRADE_FWD_BIT_CURRENT)) != 0) ) {
        /* if boot from golden and current fwd is present,  */ 
        if( qapi_Fw_Upgrade_get_Current_Index(&current) == QAPI_OK ) {
            /* reset wdog reset counter */
            qapi_System_WDTCount_Reset();
            /* delete current fwd */
            qapi_Fw_Upgrade_Erase_FWD(current);    
        }
    }

    return QAPI_OK;
}


/*
 * from here, these are high level APIs for firmware upgrade
 */
 
/**
 * Start the firmware upgrade session.
 *
 * The caller from the application domain specifies all the required parameters, 
 * including the plugin functions, source of the image, and flags. 
 * The session automatically ends in case of an error. 
 *
 * @param[in] interface_Name    Network interface name, e.g., wlan1.
 *
 * @param[in] plugin        Parameter of type qapi_Fw_Upgrade_Plugin_t containing \n
 *                          set of plugin call back functions.
 *                          For more details, refer to qapi_Fw_Upgrade_Plugin_t.
 *
 * @param[in] url           Source information for an firmware upgrade. \n
 *                          For FTP: \n
 *                          [user_name]:[password]@[IPV4 address]:[port]/[file name]  for IPV4 \n
 *                          [user_name]:[password]@|[IPV6 address]|:[port]/[file name]  for IPV6
 *
 * @param[in] cfg_File      Image file information for a firmware upgrade. \n
 *
 * @param[in] flags         Flags with bit define for an firmware upgrade. see "qapi_Fw_Upgrade flag" for definition
 *
 * @param[in] cb            Optional Callback function called by firmware upgrade engine to provide status information.
 *
 * @param[in] init_Param    Optional init parameter passed to firmware upgrade init function.
 *
 * @return
 *  On success, QAPI_FW_UPGRADE_OK_E is returned. \n 
 *  On error, error code defined by qapi_Fw_Upgrade_Status_Code_t is returned.
 */
qapi_Fw_Upgrade_Status_Code_t qapi_Fw_Upgrade(char *interface_Name, qapi_Fw_Upgrade_Plugin_t *plugin, char *url, char *cfg_File, uint32_t flags, qapi_Fw_Upgrade_CB_t cb, void *init_Param)
{
    return fw_Upgrade(interface_Name, plugin, url, cfg_File, flags, cb, init_Param);
}

/**
 * Cancels the firmware upgrade session.
 *
 * @return
 *  On success, QAPI_FW_UPGRADE_OK_E is returned. \n 
 *  On error, error code defined by qapi_Fw_Upgrade_Status_Code_t is returned.
 */
qapi_Fw_Upgrade_Status_Code_t qapi_Fw_Upgrade_Cancel(void)
{
    return fw_Upgrade_Session_Cancel();
}

/**
 * Activates or invalidates the trial image. Application calls this API after it has verified that
 * the image is valid/invalid. The criteria for image validity is defined by the application.
 *
 * @param[in] result        1: image is valid; set trial image to active. \n
 *                          0: image is invalid; invalidate trial image.  
 * @param[in] flags         bit0: 1: device reboots after activate or invalidate the trial image
 *                                0: device doesn't reboot after activate or invalidate the trial image
 * @note:                   if reboot_flag is set, device will reboot and there is no return.
 * @return
 * On success, QAPI_FW_UPGRADE_OK_E is returned. \n 
 * On error, QAPI_FW_UPGRADE_ERROR_E is returned.
 */
qapi_Fw_Upgrade_Status_Code_t qapi_Fw_Upgrade_Done(uint32_t result, uint32_t flags)
{
    qapi_Fw_Upgrade_Status_Code_t rtn = QAPI_FW_UPGRADE_ERROR_E;
    
    rtn = fw_Upgrade_Session_Done(result);
    
    /* check reboot flag here */
    if( (rtn == QAPI_FW_UPGRADE_OK_E) && ((flags & QAPI_FW_UPGRADE_FLAG_AUTO_REBOOT) != 0) )
    {
        /* reboot system here ..... */
        qapi_Fw_Upgrade_Reboot_System();
    }

    return rtn;    
}

/**
 * Suspend the firmware upgrade session.
 *
 * @return
 *  On success, QAPI_FW_UPGRADE_OK_E is returned. \n
 *  On error, error code defined by qapi_Fw_Upgrade_Status_Code_t is returned.
 */
qapi_Fw_Upgrade_Status_Code_t qapi_Fw_Upgrade_Suspend(void)
{
	return fw_Upgrade_Session_Suspend();
}

/**
 * Resume the firmware upgrade session.
 *
 * @return
 *  On success, QAPI_FW_UPGRADE_OK_E is returned. \n
 *  On error, error code defined by qapi_Fw_Upgrade_Status_Code_t is returned.
 */
qapi_Fw_Upgrade_Status_Code_t qapi_Fw_Upgrade_Resume(void)
{
    return fw_Upgrade_Session_Resume();
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
qapi_Fw_Upgrade_Status_Code_t qapi_Fw_Upgrade_Host_Init(uint32_t flags)
{
    return fw_Upgrade_Host_Init(flags);
}

/**
 * Stop a firmware upgrade session which triggered by host.
 *
 * @return
 *  On success, QAPI_FW_UPGRADE_OK_E is returned. \n 
 *  On error, error code defined by enum #qapi_Fw_Upgrade_Status is returned.
 */
qapi_Fw_Upgrade_Status_Code_t qapi_Fw_Upgrade_Host_Deinit(void)
{
    return fw_Upgrade_Host_Deinit();
}

/**
 * Pass buffer with len to firmware upgrade session which triggered by host.
 *
 * @return
 *  On success, QAPI_FW_UPGRADE_OK_E is returned. \n 
 *  On error, error code defined by enum #qapi_Fw_Upgrade_Status is returned.
 */
qapi_Fw_Upgrade_Status_Code_t qapi_Fw_Upgrade_Host_Write(char *buffer, int32_t len)
{
    return fw_Upgrade_Host_Write(buffer, len);
}

/**
 * Get firmware upgrade session state.
 *
 * @return
 *   state defined by #qapi_Fw_Upgrade_State is returned.
 */
qapi_Fw_Upgrade_State_t qapi_Fw_Upgrade_Get_State(void)
{
    return fw_Upgrade_Get_State();
}

/**
 * Get firmware upgrade session status.
 *
 * @return
 *  status code defined by #qapi_Fw_Upgrade_Status is returned.
 */
qapi_Fw_Upgrade_Status_Code_t qapi_Fw_Upgrade_Get_Status(void)
{
    return fw_Upgrade_Get_Error_Code();
}



