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

 /******************************************************************************
 * sdio.c
 *
 * SDCC Master side transport layer implementation
 *
 ******************************************************************************/
/*=============================================================================

                              EDIT HISTORY 
  $Header: 

when         who     what, where, why
----------   ---     ---------------------------------------------------------- 
2017-07-11   mmtd      Initial creation
======================================================================*/
#include <stdio.h>
#include "stdlib.h"
#include "qurt_signal.h"
#include "qapi/qurt_thread.h"
#include "stdint.h"
#include "qapi/qapi_status.h"
#include "qapi/qapi_tlmm.h"
#include "qapi/qapi_slp.h"
#include "qapi_master_sdcc.h"
#include <qcli.h>
#include <qcli_api.h>
#include <qurt_timer.h>
#include "osal_types.h"
#include "htc_defs.h"
#include "transport.h"
#include "sdio.h"
#include "osal_types.h"
#include "htc_defs.h"
#include "htc.h"
#include "dl_list.h"
#include "htc_internal.h"
#include "hif.h"
#include "hif_internal.h"

/* Prototypes */
static htc_status_t sdio_hal_read_data (void *tdev, io_mode_t iomode, unsigned int addr, void *src, int count);
static htc_status_t sdio_hal_write_data (void *tdev, io_mode_t iomode, unsigned int addr, void *src, int count);
static htc_status_t sdio_hal_register_interrupt (void *tdev, hif_isr_handler_t pfunc);
static htc_status_t sdio_hal_deregister_interrupt (void *tdev);
static htc_status_t sdio_hal_enable_func (void *tdev);
static htc_status_t sdio_hal_disable_func (void *tdev);

/* #define SDIO_DUMP_READ */
/* #define SDIO_DUMP_WRITE */

/* Following are GPIO and Frequency configuration for SDIO */

#define SDIO_ADMA_DESCR_ITEMS         (3)   /* adma descriptor items */
#define SDIO_BUS_WIDTH                (4)   /* sdio bus width 1 bit or 4 bits */
#define SDIO_FREQ                     (400000)    /* bus frequency */
#define SDIO_BLOCK_SIZE               HIF_ENDPOINT_BLOCK_SIZE  /* block size */

#define SDIO_CLK_CONFIG_PIN           (18)
#define SDIO_CMD_CONFIG_PIN           (19)
#define SDIO_DATA_0_CONFIG_PIN        (20)
#define SDIO_DATA_1_CONFIG_PIN        (21)
#define SDIO_DATA_2_CONFIG_PIN        (22)
#define SDIO_DATA_3_CONFIG_PIN        (23)

#define SDIO_CLK_CONFIG_FUNC          (3)
#define SDIO_CMD_CONFIG_FUNC          (3)
#define SDIO_DATA_0_CONFIG_FUNC       (3)
#define SDIO_DATA_1_CONFIG_FUNC       (3)
#define SDIO_DATA_2_CONFIG_FUNC       (3)
#define SDIO_DATA_3_CONFIG_FUNC       (3)

extern QCLI_Group_Handle_t qcli_sdio_group;              /* Handle for our QCLI Command Group. */


static trans_dev_ops_t sdio_ops =  {
   .read_data = sdio_hal_read_data,
   .write_data = sdio_hal_write_data,
   .register_interrupt = sdio_hal_register_interrupt,
   .deregister_interrupt = sdio_hal_deregister_interrupt,
   .enable_trans = sdio_hal_enable_func,
   .disable_trans = sdio_hal_disable_func,
};

void sdio_dump_bytes (uint8 *buf, uint32 count) 
{

	uint32 i=0;

	for ( i=0; i <count; i++) {
		 QCLI_Printf(qcli_sdio_group, " %x ", buf[i]);
	}
	QCLI_Printf(qcli_sdio_group, "\n");

}

static qapi_Status_t cmd52_func0_read_byte(void* sdio_Handle, uint32 reg_Addr, uint8 *data_Out)
{
    qapi_Status_t  status;
    qapi_SDCC_Op_Flags_e flags;
    uint32 dev_Fn;
    
    flags = QAPI_SDCC_DIR_READ_E;
    dev_Fn = 0;
    
    status = qapi_SDCCM_Send_Receive_Byte(sdio_Handle, flags, dev_Fn, reg_Addr, 0, data_Out);

    
    return status;
}

static qapi_Status_t cmd52_func0_write_byte(void* sdio_Handle, uint32 reg_Addr, uint8 *data_Out)
{
    qapi_Status_t  status;
    qapi_SDCC_Op_Flags_e flags;
    uint32 dev_Fn;
    
    flags = QAPI_SDCC_DIR_WRITE_E;
    dev_Fn = 0;
    
    status = qapi_SDCCM_Send_Receive_Byte(sdio_Handle, flags, dev_Fn, reg_Addr, *data_Out, data_Out);
    
    return status;
}

static htc_status_t sdio_hal_read_data (void *tdev, io_mode_t iomode, unsigned int addr, void *src, int count) {
    uint32 flags, dev_Fn = 1; 
	qapi_Status_t status;
	sdio_device_t *dev = NULL;
    trans_device_t *trans_dev = tdev;
    

    flags = QAPI_SDCC_DIR_READ_E;

    if(tdev == NULL)
    {
        return HTC_EINVAL;
    }
    dev = (sdio_device_t*) trans_dev->dev;

	if (iomode == MEMORY_MODE) {
		flags |= (QAPI_SDCC_OP_INC_ADDR_E);
	}

	if ((count % SDIO_BLOCK_SIZE) == 0)
		flags |= QAPI_SDCC_BLOCK_MODE_E;


    status = qapi_SDCCM_Receive_Data_Ext (dev->handle,
                                       flags,
                                       dev_Fn,
                                       addr,
                                       count,
                                       src);
	
#ifdef SDIO_DUMP_READ
   QCLI_Printf(qcli_sdio_group, "Status %d of %s\n", status, __func__);

   QCLI_Printf(qcli_sdio_group, "Bytes read at addr  %x of len %d\n", addr, count);
   sdio_dump_bytes(src, count);
#endif   
   
	/* Re enable card interrupt after every IO transaction,
	because in SDCC driver, card interrupt is getting disabled */
	status = qapi_SDCCM_EnableDisableIrq (dev->handle, TRUE);

	return qapi_to_htcstatus(status);
}

static htc_status_t sdio_hal_write_data (void *tdev, io_mode_t iomode, unsigned int addr, void *src, int count) {
    uint32 flags, dev_Fn = 1; 
	qapi_Status_t status;
	sdio_device_t *dev = NULL;
    trans_device_t *trans_dev = tdev;
    
    flags = QAPI_SDCC_DIR_WRITE_E;

    if(tdev == NULL)
    {
        return HTC_EINVAL;
    }
    dev = (sdio_device_t*) trans_dev->dev;

	if (iomode == MEMORY_MODE) {
		flags |= (QAPI_SDCC_OP_INC_ADDR_E);
	}

	if ((count % SDIO_BLOCK_SIZE) == 0)
		flags |= QAPI_SDCC_BLOCK_MODE_E;
	
    status = qapi_SDCCM_Send_Data_Ext (dev->handle,
                                       flags,
                                       dev_Fn,
                                       addr,
                                       count,
                                       src);
	
	QCLI_Printf(qcli_sdio_group, "Status %d of %s\n", status, __func__);
	
#ifdef SDIO_DUMP_WRITE

	QCLI_Printf(qcli_sdio_group, "Bytes written at addr  %x of len %d\n", addr, count);
	sdio_dump_bytes(src, count);
#endif

	/* Re enable card interrupt after every IO transaction, 
	because in SDCC driver, card interrupt is getting disabled */
	status = qapi_SDCCM_EnableDisableIrq (dev->handle, TRUE);

	return qapi_to_htcstatus(status);
}

static htc_status_t sdio_hal_register_interrupt (void *tdev, hif_isr_handler_t pfunc) {
	qapi_Status_t status;
    uint32  reg_Addr;
    uint8   data_Out;
	sdio_device_t *dev = NULL;
    trans_device_t *trans_dev = tdev;

    if(tdev == NULL)
    {
        return HTC_EINVAL;
    }
    dev = (sdio_device_t*) trans_dev->dev;


    reg_Addr = SDIO_CCCR_IENx;
    data_Out = 0x0;
    status = cmd52_func0_read_byte(dev->handle, reg_Addr, &data_Out);
    data_Out  |= 0x03;
    status = cmd52_func0_write_byte(dev->handle, reg_Addr, &data_Out);
    if (status != QAPI_OK)
    {
		QCLI_Printf(qcli_sdio_group, "Interrupt enable Failed!\n");
    }
	
	QCLI_Printf(qcli_sdio_group, "Interrupt enabled handle %x!\n", dev->handle);
	
	status = qapi_SDCCM_Intr_Register (dev->handle, pfunc, tdev);
	status = qapi_SDCCM_EnableDisableIrq (dev->handle,TRUE);
	return qapi_to_htcstatus(status);
}

static htc_status_t sdio_hal_deregister_interrupt (void *tdev) {
	qapi_Status_t status;
    uint32  reg_Addr;
    uint8   data_Out;
	sdio_device_t *dev = NULL;
    trans_device_t *trans_dev = tdev;

    if(tdev == NULL)
    {
        return HTC_EINVAL;
    }
    dev = (sdio_device_t*) trans_dev->dev;

    reg_Addr = SDIO_CCCR_IENx;
    data_Out = 0x0;
    status = cmd52_func0_read_byte(dev->handle, reg_Addr, &data_Out);
    data_Out  &= ~(0x03);
    status = cmd52_func0_write_byte(dev->handle, reg_Addr, &data_Out);
    if (status != QAPI_OK)
    {
		QCLI_Printf(qcli_sdio_group, "Interrupt disable Failed handle %x!\n", dev->handle);
    } 	

	status = qapi_SDCCM_EnableDisableIrq (dev->handle, FALSE);

	
	status = qapi_SDCCM_Intr_Deregister (dev->handle);
	return qapi_to_htcstatus(status);

}


static htc_status_t sdio_hal_enable_func (void *tdev) {
    qapi_Status_t  status;
    uint32  reg_Addr;
    uint8   data_Out;
    uint16  u16_data;
	sdio_device_t *dev = NULL;
    trans_device_t *trans_dev = tdev;

    if(tdev == NULL)
    {
        return HTC_EINVAL;
    }
    dev = (sdio_device_t*) trans_dev->dev;
    
    /* Async int delay */
    reg_Addr = CCCR_SDIO_ASYNC_INT_DELAY_ADDRESS;
    data_Out = 0x0;
    status = cmd52_func0_read_byte(dev->handle, reg_Addr, &data_Out);

    data_Out  = (data_Out & ~CCCR_SDIO_ASYNC_INT_DELAY_MASK) |
         ((dev->asyncintdelay << CCCR_SDIO_ASYNC_INT_DELAY_LSB) & CCCR_SDIO_ASYNC_INT_DELAY_MASK);
    status = cmd52_func0_write_byte(dev->handle, reg_Addr, &data_Out);
    if (status != QAPI_OK)
    {
		QCLI_Printf(qcli_sdio_group, "Setting Async int delay Failed!\n");
    } 	


    QCLI_Printf(qcli_sdio_group, "Enable Func 1 \n");
    reg_Addr = SDIO_CCCR_IOEx;
    data_Out = 0x3;
    status = cmd52_func0_write_byte(dev->handle, reg_Addr, &data_Out);
    if (status != QAPI_OK)
    {
        QCLI_Printf(qcli_sdio_group, "Enable Func 1  fails  result:%d\n", status);
    }   
    

    reg_Addr = SDIO_FBR_BLOCK_SIZE;
    data_Out = 0x00;
    u16_data = data_Out;
    status = cmd52_func0_write_byte(dev->handle, reg_Addr, &data_Out);
    data_Out = 0x01;
    u16_data |= (data_Out << 8);    
    status = cmd52_func0_write_byte(dev->handle, reg_Addr+1, &data_Out);

    if (status != QAPI_OK)
    {
		QCLI_Printf(qcli_sdio_group, "Setting Master SDIO FBR Block Size[0x%x]=0x%02x Failed!\n", reg_Addr, u16_data);
    } 	


	/* Enable the card interrupt*/ 
	status = qapi_SDCCM_EnableDisableIrq (dev->handle, TRUE);
	
	return qapi_to_htcstatus(status);

}

static htc_status_t sdio_hal_disable_func (void *tdev) {
    qapi_Status_t  status;
    uint32  reg_Addr;
    uint8   data_Out;
	sdio_device_t *dev = NULL;
    trans_device_t *trans_dev = tdev;

    if(tdev == NULL)
    {
        return HTC_EINVAL;
    }
    dev = (sdio_device_t*) trans_dev->dev;

    QCLI_Printf(qcli_sdio_group, "Disable Func 1 \n");
	
    reg_Addr = SDIO_CCCR_IOEx;
    data_Out = 0x0;
    status = cmd52_func0_write_byte(dev->handle, reg_Addr, &data_Out);
	if (status != QAPI_OK)
    {
		QCLI_Printf(qcli_sdio_group, "Disable Func 1 Failed!\n");
    }
	
	/* Disable the card interrupt*/ 
	status = qapi_SDCCM_EnableDisableIrq (dev->handle, FALSE);
    
    
	
	return qapi_to_htcstatus(status);
}

htc_status_t sdio_hal_init (trans_device_t *tdev)
{
    qapi_Status_t  status;
    uint32  reg_Addr;
    uint8   data_Out;
	sdio_device_t *dev = NULL;
    

    QCLI_Printf(qcli_sdio_group, "Initialize Master SDIO\n");

	dev = (sdio_device_t*) malloc(sizeof(sdio_device_t));

	if (dev == NULL) {
		return HTC_NO_MEMORY;
	}
	
	memset (dev, 0, sizeof(sdio_device_t));
    dev->id = tdev->id;
    tdev->dev = (void *)dev;

    /* Populate device configuration reading from DevCFG */
    dev->sdio_Config.adma_Descr_Items = SDIO_ADMA_DESCR_ITEMS;    /* adma descriptor items */
    dev->sdio_Config.SDCC_Bus_Width = SDIO_BUS_WIDTH;      /* sdio bus width 1 bit or 4 bits */
    dev->sdio_Config.freq = SDIO_FREQ;     /* bus frequency */
    dev->sdio_Config.block_Size = SDIO_BLOCK_SIZE;    /* block size */
      
    dev->asyncintdelay = 2;

    status = qapi_SDCCM_Init(&dev->sdio_Config, &dev->handle);
    if (status != QAPI_OK)
    {
        QCLI_Printf(qcli_sdio_group, "Master SDIO Init fails  result:%d\n", status);
        goto error;
    }   
    
    status = qapi_SDCCM_Open(dev->handle);
    if (status != QAPI_OK)
    {
        qapi_SDCCM_Deinit (dev->handle);
        goto error;

    }

    reg_Addr = 0;
    status = cmd52_func0_read_byte(dev->handle, reg_Addr, &data_Out);
    if (status != QAPI_OK)
    {
        qapi_SDCCM_Close(dev->handle);
        qapi_SDCCM_Deinit(dev->handle);
        goto error;
    }   
    QCLI_Printf(qcli_sdio_group, "CCCR version=0x%02x\n", (data_Out & 0x0F));       
    QCLI_Printf(qcli_sdio_group, "SDIO version=0x%02x\n", ((data_Out >> 4) & 0x0F));        

/*    SDIO_CCCR_DRIVE_STRENGTH */
    reg_Addr = SDIO_CCCR_DRIVE_STRENGTH;
    data_Out = 0x0;
    status = cmd52_func0_read_byte(dev->handle, reg_Addr, &data_Out);
    if (status != QAPI_OK)
    {
        qapi_SDCCM_Close(dev->handle);
        qapi_SDCCM_Deinit(dev->handle);
        goto error;
    } 

    data_Out = (data_Out &
            (~(SDIO_DRIVE_DTSx_MASK << SDIO_DRIVE_DTSx_SHIFT))) |
            SDIO_DTSx_SET_TYPE_D;
    status = cmd52_func0_write_byte(dev->handle, reg_Addr, &data_Out);
    if (status != QAPI_OK)
    {
        QCLI_Printf(qcli_sdio_group, "Master SDIO CMD52 write fails  result:%d\n", status);      
        qapi_SDCCM_Close(dev->handle);
        qapi_SDCCM_Deinit(dev->handle);
        goto error;

    } 

    reg_Addr = CCCR_SDIO_DRIVER_STRENGTH_ENABLE_ADDR;
    data_Out = 0x0;
    status = cmd52_func0_read_byte(dev->handle, reg_Addr, &data_Out);
    if (status != QAPI_OK)
    {
        QCLI_Printf(qcli_sdio_group, "Master SDIO CMD52 read fails  result:%d\n", status);      
        qapi_SDCCM_Close(dev->handle);
        qapi_SDCCM_Deinit(dev->handle);
        goto error;

    } 	
    data_Out = (data_Out & (~CCCR_SDIO_DRIVER_STRENGTH_ENABLE_MASK)) |
                             CCCR_SDIO_DRIVER_STRENGTH_ENABLE_A |
                             CCCR_SDIO_DRIVER_STRENGTH_ENABLE_C |
                             CCCR_SDIO_DRIVER_STRENGTH_ENABLE_D;
    status = cmd52_func0_write_byte(dev->handle, reg_Addr, &data_Out);
    if (status != QAPI_OK)
    {
        QCLI_Printf(qcli_sdio_group, "Setting drive strenghts failed  result:%d\n", status);      
        qapi_SDCCM_Close(dev->handle);
        qapi_SDCCM_Deinit(dev->handle);
        goto error;
    } 

 
    /* MMC bus width, set it to sdio_Config.SDCC_Bus_Width */
    /* SDIO_CCCR_IF */
    reg_Addr = SDIO_CCCR_IF;
    data_Out = (SDIO_BUS_CD_DISABLE|SDIO_BUS_WIDTH_4BIT);
    status = cmd52_func0_write_byte(dev->handle, reg_Addr, &data_Out);
    if (status != QAPI_OK)
    {
        QCLI_Printf(qcli_sdio_group, "Setting bus width failed  result:%d\n", status);      
        qapi_SDCCM_Close(dev->handle);
        qapi_SDCCM_Deinit(dev->handle);
        goto error;
    } 

	tdev->ops = &sdio_ops;

	/* Note, qurt SDCC driver does not have hotplug detection functionality.
	hence it is assumed that the device is always present */
	
	if((tdev->hif_callbacks.hif_trans_device_inserted != NULL) && (status == QAPI_OK))
		tdev->hif_callbacks.hif_trans_device_inserted(tdev, FALSE);

	return qapi_to_htcstatus(status);

error:
    free(dev);
    return HTC_ERROR;
}


htc_status_t sdio_hal_deinit (trans_device_t *tdev)
{
    qapi_Status_t  status;
    uint32  reg_Addr;
    uint8   data_Out;
	sdio_device_t *dev = NULL;

    if(tdev == NULL)
    {
        return HTC_EINVAL;
    }
    dev = (sdio_device_t*) tdev->dev;

    /*  This shall also be called from hot plug device removal handler
	   	Since Quartz SDCC driver does not have hotplug functionality, the device disconnect event
    	is sent during de-initialization */
	if (tdev->hif_callbacks.hif_trans_device_removed != NULL)
		tdev->hif_callbacks.hif_trans_device_removed(tdev, FALSE);


	
	/* Reset SDIO Card */
    reg_Addr = SDIO_CCCR_ABORT;
    data_Out = (SDIO_IO_RESET);
    status = cmd52_func0_write_byte(dev->handle, reg_Addr, &data_Out);
    if (status != QAPI_OK)
    {
        QCLI_Printf(qcli_sdio_group, "SDIO Card reset failed with status :%d\n", status);      
    }

    status = qapi_SDCCM_Close (dev->handle);

    qapi_SDCCM_Deinit (dev->handle);

    free(dev);
    tdev->dev = NULL;
	tdev->ops = NULL;

	return qapi_to_htcstatus(status);
}
