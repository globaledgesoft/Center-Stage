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
 * master_sdcc.c
 *
 * SDCC Master side host interface layer implementation
 *
 ******************************************************************************/
/*=============================================================================

                              EDIT HISTORY 
  $Header: 

when         who     what, where, why
----------   ---     ---------------------------------------------------------- 
2017-09-10   mmtd      Added support for block transfer over mbox endpoints
2017-09-10   mb        Initial creation
======================================================================*/
 
#include <stdio.h>
#include "qurt_signal.h"
#include "qapi/qurt_thread.h"
#include "stdint.h"
#include "qapi/qapi_status.h"
#include "qapi/qapi_tlmm.h"
#include "qapi_master_sdcc.h"
#include <qcli.h>
#include <qcli_api.h>
#include <qurt_timer.h>
#include "master_sdcc.h"

extern QCLI_Group_Handle_t qcli_sdio_group;              /* Handle for our QCLI Command Group. */

uint8_t tx_buf[HIF_MBOX_BLOCK_SIZE];
uint8_t rx_buf[HIF_MBOX_BLOCK_SIZE];
qapi_SDCC_Handle sdio_Handle;
uint8_t data[1024];
uint8_t cis[4096];

qapi_SDCC_Config_t  sdio_Config = {
    /**< adma descriptor items */
    3,
    /**< sdio bus width 1 bit or 4 bits */  
    4,
    /**< bus frequency */
    400000,
    /**< block size */
    HIF_MBOX_BLOCK_SIZE,
};

void isr_Cb(void *param)
{
    
}

void hif_dump_buffer (uint8 *pBuf, uint32 size)
{
    uint32 i=0;
    for ( i = 0; i < size; i ++)
        QCLI_Printf(qcli_sdio_group, "%x ", pBuf[i]); 
}

void hif_init_txbuffer(void)
{
    uint32 i=0;
    for ( i = 0; i < HIF_MBOX_BLOCK_SIZE; i ++)
        tx_buf[i]=i;
    return ;
}

void hif_clear_rxbuffer(void)
{
    uint32 i=0;
    for ( i = 0; i < HIF_MBOX_BLOCK_SIZE; i ++)
        rx_buf[i]=0;
    return ;
}

qapi_Status_t hif_tx_block_mbox(void)
{
    qapi_Status_t  status;
    /* Send bytes in tx_buf in blocking mode */ 

    QCLI_Printf(qcli_sdio_group, "Sending block:\n");
    hif_dump_buffer (tx_buf, HIF_MBOX_BLOCK_SIZE);

#ifdef USES_SDCC_DMA
    status = cmd53_write_bytes_dma(sdio_Handle, 1, HIF_MBOX0_ADDRESS, tx_buf, HIF_MBOX_BLOCK_SIZE, HIF_MBOX_BLOCK_SIZE,  1 );
#else
    status = cmd53_write_bytes(sdio_Handle, 1, HIF_MBOX0_ADDRESS, HIF_MBOX_BLOCK_SIZE, tx_buf, 1 );
#endif
    return status;
}

qapi_Status_t hif_rx_block_mbox(void)
{
    qapi_Status_t  status;
    
    /* Read bytes  in rx_buf in blocking mode */  
    status = cmd53_read_bytes(sdio_Handle, 1, HIF_MBOX0_ADDRESS, HIF_MBOX_BLOCK_SIZE, rx_buf, 1 );
    QCLI_Printf(qcli_sdio_group, "Received block:\n");
    hif_dump_buffer (rx_buf, HIF_MBOX_BLOCK_SIZE);
    return status;
}

int sdio_test( uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List )
{
    qapi_Status_t  status = QAPI_OK;
    uint32     select_52;
    uint32   Fn;
    uint32    reg_Addr;
    uint8    reg_data;
    uint32 reg_word;

    if ( Parameter_Count < 1 )
    {
        return -1;
    }
    reg_word = 0;
    select_52 = Parameter_List->Integer_Value;
    Parameter_List++;
    Fn = Parameter_List->Integer_Value;
    Parameter_List++;
    reg_Addr = Parameter_List->Integer_Value;
    Parameter_List++;
    reg_data = Parameter_List->Integer_Value;
    reg_word |= reg_data;
    reg_word <<= 8;
    Parameter_List++;
    reg_data = Parameter_List->Integer_Value;
    reg_word |= reg_data;
    reg_word <<= 8;
    Parameter_List++;
    reg_data = Parameter_List->Integer_Value;
    reg_word |= reg_data;
    reg_word <<= 8;
    Parameter_List++;
    reg_data = Parameter_List->Integer_Value;
    reg_word |= reg_data;
    reg_word <<= 8;
    Parameter_List++;
    
    switch (select_52){
    case    0:
        app_sdio_init();
        break;
    case    1:
        status = cmd52_read_sdio_registers(sdio_Handle);
        break;
    case    2:
        status = cmd53_read_function_0_bytes(sdio_Handle);      
        break;
    case    3:  
        status = cmd53_read_function_0_blocks(sdio_Handle);
        break;
    case    4:
        status = cmd53_read_function_0_scatter_gather(sdio_Handle);     
        break;
    case    5:
        status = cmd53_write_sdio_reg(sdio_Handle, Fn, reg_Addr, &reg_data,0 );
        break;
    case    6:
        status = cmd53_read_sdio_reg(sdio_Handle, Fn, reg_Addr, &reg_data,0 );
        break;
    case    7:
        status = cmd53_write_sdio_reg_word(sdio_Handle, Fn, reg_Addr, (uint8 *) &reg_word,1 );
        break;
    case    8:
        status =  hif_tx_block_mbox();
        break;        
    case    9:
        {
        hif_clear_rxbuffer();
        status =  hif_rx_block_mbox();
        }
        break;        
    }
    QCLI_Printf(qcli_sdio_group, "SDIO cmd Done status = %x\n", status);
    return 0;
}

qapi_TLMM_Config_t Qz4020_wifiPwr = {
  33,                    //  uint32_t              pin;    /**< Physical pin number. */ 
  0,                     // uint32_t              func;   /**< Pin function select. */
  QAPI_GPIO_OUTPUT_E,    // qapi_GPIO_Direction_t dir;    /**< Direction (input or output). */
  QAPI_GPIO_PULL_UP_E,   // qapi_GPIO_Pull_t      pull;   /**< Pull value. */
  QAPI_GPIO_2MA_E,       // qapi_GPIO_Drive_t     drive;  /**< Drive strength. */
};
qapi_GPIO_ID_t     Qz4020_pwd_gpio_id;

void app_master_sdio_msec_delay(uint32_t ms)
{
    uint32_t ticks;

    ticks = qurt_timer_convert_time_to_ticks(ms, QURT_TIME_MSEC);
    if (ms != 0 && ticks == 0)
    {
        ticks = 1;
    }
    qurt_thread_sleep(ticks);

    return;
}

void Qz4020_PowerUpDown(uint32_t powerUp)
{   
    qapi_Status_t ret = qapi_TLMM_Get_Gpio_ID( &Qz4020_wifiPwr, &Qz4020_pwd_gpio_id);
    if (ret != QAPI_OK)
    {
        return;
    }
    ret = qapi_TLMM_Config_Gpio(Qz4020_pwd_gpio_id, &Qz4020_wifiPwr);
    if (ret != QAPI_OK)
    {
        return;
    }

    if (powerUp){
        qapi_TLMM_Drive_Gpio(Qz4020_pwd_gpio_id, Qz4020_wifiPwr.pin, QAPI_GPIO_LOW_VALUE_E);
        app_master_sdio_msec_delay(CUSTOM_HW_POWER_UP_PULSE_WIDTH);
        qapi_TLMM_Drive_Gpio(Qz4020_pwd_gpio_id, Qz4020_wifiPwr.pin, QAPI_GPIO_HIGH_VALUE_E);
    }
    else
    {
       qapi_TLMM_Drive_Gpio(Qz4020_pwd_gpio_id, Qz4020_wifiPwr.pin, QAPI_GPIO_LOW_VALUE_E);
    }   
}

qapi_Status_t app_sdio_init()
{
    qapi_Status_t  status;
    uint32  reg_Addr;
    uint16  u16_data;
    uint8   data_Out;
    

    QCLI_Printf(qcli_sdio_group, "Initialize Master SDIO\n");
    Qz4020_PowerUpDown(TRUE);
    
    status = qapi_SDCCM_Init(&sdio_Config, &sdio_Handle);
    if (status != QAPI_OK)
    {
        QCLI_Printf(qcli_sdio_group, "Master SDIO Init fails  result:%d\n", status);
        return status;
    }   
    
    status = qapi_SDCCM_Open(sdio_Handle);
    if (status != QAPI_OK)
    {
        QCLI_Printf(qcli_sdio_group, "Master SDIO Open fails  result:%d\n", status);        
        return status;
    }   

    reg_Addr = 0;
    status = cmd52_read_one_byte(sdio_Handle, reg_Addr, &data_Out);
    if (status != QAPI_OK)
    {
        QCLI_Printf(qcli_sdio_group, "Master SDIO CMD52 read fails  result:%d\n", status);      
        return status;
    }   
    QCLI_Printf(qcli_sdio_group, "CCCR version=0x%02x\n", (data_Out & 0x0F));       
    QCLI_Printf(qcli_sdio_group, "SDIO version=0x%02x\n", ((data_Out >> 4) & 0x0F));        

/*    SDIO_CCCR_DRIVE_STRENGTH */
    reg_Addr = SDIO_CCCR_DRIVE_STRENGTH;
    data_Out = 0x0;
    status = cmd52_read_one_byte(sdio_Handle, reg_Addr, &data_Out);

    data_Out = (data_Out &
            (~(SDIO_DRIVE_DTSx_MASK << SDIO_DRIVE_DTSx_SHIFT))) |
            SDIO_DTSx_SET_TYPE_D;
    status = cmd52_write_one_byte(sdio_Handle, reg_Addr, &data_Out);

    reg_Addr = CCCR_SDIO_DRIVER_STRENGTH_ENABLE_ADDR;
    data_Out = 0x0;
    status = cmd52_read_one_byte(sdio_Handle, reg_Addr, &data_Out);
    data_Out = (data_Out & (~CCCR_SDIO_DRIVER_STRENGTH_ENABLE_MASK)) |
                             CCCR_SDIO_DRIVER_STRENGTH_ENABLE_A |
                             CCCR_SDIO_DRIVER_STRENGTH_ENABLE_C |
                             CCCR_SDIO_DRIVER_STRENGTH_ENABLE_D;
    status = cmd52_write_one_byte(sdio_Handle, reg_Addr, &data_Out);

 
    /* MMC bus width, set it to sdio_Config.SDCC_Bus_Width */
    /* SDIO_CCCR_IF */
    reg_Addr = SDIO_CCCR_IF;
    data_Out = (SDIO_BUS_CD_DISABLE|SDIO_BUS_WIDTH_4BIT);
    status = cmd52_write_one_byte(sdio_Handle, reg_Addr, &data_Out);


    /* Async int delay */
    reg_Addr = CCCR_SDIO_ASYNC_INT_DELAY_ADDRESS;
    data_Out = 0x0;
    status = cmd52_read_one_byte(sdio_Handle, reg_Addr, &data_Out);
    
    data_Out  = (data_Out & ~CCCR_SDIO_ASYNC_INT_DELAY_MASK) |
         ((asyncintdelay << CCCR_SDIO_ASYNC_INT_DELAY_LSB) & CCCR_SDIO_ASYNC_INT_DELAY_MASK);
    status = cmd52_write_one_byte(sdio_Handle, reg_Addr, &data_Out);

    QCLI_Printf(qcli_sdio_group, "Enable Func 1 \n");
    reg_Addr = SDIO_CCCR_IOEx;
    data_Out = 0x3;
    status = cmd52_write_one_byte(sdio_Handle, reg_Addr, &data_Out);

    reg_Addr = SDIO_FBR_BLOCK_SIZE;
    data_Out = 0x00;
    u16_data = data_Out;
    status = cmd52_write_one_byte(sdio_Handle, reg_Addr, &data_Out);
    data_Out = 0x01;
    u16_data |= (data_Out << 8);    
    status = cmd52_write_one_byte(sdio_Handle, reg_Addr+1, &data_Out);
    QCLI_Printf(qcli_sdio_group, "Setting Master SDIO FBR Block Size[0x%x]=0x%02x\n", reg_Addr, u16_data);

    hif_init_txbuffer();
    hif_clear_rxbuffer();
    return status;
}

qapi_Status_t app_sdio_deinit()
{
    qapi_Status_t  status;

    status = qapi_SDCCM_Close ( sdio_Handle );
    qapi_TLMM_Release_Gpio_ID (&Qz4020_wifiPwr, Qz4020_pwd_gpio_id);
   
    return status;
}

qapi_Status_t cmd52_read_one_byte(qapi_SDCC_Handle sdio_Handle, uint32 reg_Addr, uint8 *data_Out)
{
    qapi_Status_t  status;
    qapi_SDCC_Op_Flags_e flags;
    uint32 dev_Fn;
    
    flags = QAPI_SDCC_DIR_READ_E;
    dev_Fn = 0;
    
    status = qapi_SDCCM_Send_Receive_Byte(sdio_Handle, flags, dev_Fn, reg_Addr, 0, data_Out);
    return status;
}

qapi_Status_t cmd52_write_one_byte(qapi_SDCC_Handle sdio_Handle, uint32 reg_Addr, uint8 *data_Out)
{
    qapi_Status_t  status;
    qapi_SDCC_Op_Flags_e flags;
    uint32 dev_Fn;
    
    flags = QAPI_SDCC_DIR_WRITE_E;
    dev_Fn = 0;
    
    status = qapi_SDCCM_Send_Receive_Byte(sdio_Handle, flags, dev_Fn, reg_Addr, *data_Out, data_Out);
    return status;
}

qapi_Status_t cmd53_read_bytes(qapi_SDCC_Handle sdio_Handle, 
                                uint32 dev_Fn,
                                uint32 reg_Addr,
                                uint32 data_Len_In_Bytes,
                                uint8 *data_Buf,
                                uint32_t block_flag)
{
    qapi_Status_t  status;
    uint32 flags; 
   
    flags = QAPI_SDCC_DIR_READ_E | QAPI_SDCC_OP_INC_ADDR_E;
    if (block_flag)
       flags |= QAPI_SDCC_BLOCK_MODE_E;

    status = qapi_SDCCM_Receive_Data_Ext (sdio_Handle,
                                       flags,
                                       dev_Fn,
                                       reg_Addr,
                                       data_Len_In_Bytes,
                                       data_Buf);
    return status;
}


qapi_Status_t cmd53_write_bytes(qapi_SDCC_Handle sdio_Handle, 
                                uint32 dev_Fn,
                                uint32 reg_Addr,
                                uint32 data_Len_In_Bytes,
                                uint8 *data_Out,
                                uint32_t block_flag)
{
    qapi_Status_t  status;
    uint32 flags; 
    
    flags = QAPI_SDCC_DIR_WRITE_E | QAPI_SDCC_OP_INC_ADDR_E;
    if (block_flag)
       flags |= QAPI_SDCC_BLOCK_MODE_E;

    status = qapi_SDCCM_Send_Data_Ext (sdio_Handle,
                                       flags,
                                       dev_Fn,
                                       reg_Addr,
                                       data_Len_In_Bytes,
                                       data_Out);
    return status;
}

qapi_Status_t cmd53_read_bytes_dma(qapi_SDCC_Handle sdio_Handle, uint32 reg_Addr, uint8 *buf1_ptr, uint32 buf1_sz, 
                    uint8 *buf2_ptr, uint32 buf2_sz, uint32_t data_Len_In_Bytes, uint32 block_flag)
{
    qapi_Status_t  status;
    uint32 flags; 
    uint32 dev_Fn;
    qapi_SDCC_Dma_Item dma_Tab[2];
    
    flags = QAPI_SDCC_DIR_READ_E | QAPI_SDCC_OP_INC_ADDR_E | QAPI_SDCC_NON_BLOCKING_E;
    if (block_flag)
        flags |= QAPI_SDCC_BLOCK_MODE_E;
    
    dev_Fn = 0;
    
    dma_Tab[0].data_Buf_Ptr = buf1_ptr;
    dma_Tab[0].size = buf1_sz;
    dma_Tab[1].data_Buf_Ptr = buf2_ptr;
    dma_Tab[1].size = buf2_sz;
    
    status = qapi_SDCCM_Send_Receive_Data_Scatter_Gather (sdio_Handle, flags, dev_Fn, reg_Addr, 
                                                    data_Len_In_Bytes, 2, dma_Tab);

    return status;
}


qapi_Status_t cmd53_write_bytes_dma(qapi_SDCC_Handle sdio_Handle, uint32 dev_Fn,uint32 reg_Addr, uint8 *buf1_ptr, uint32 buf1_sz, 
                    uint32_t data_Len_In_Bytes, uint32 block_flag)
{
    qapi_Status_t  status;
    uint32 flags; 
    qapi_SDCC_Dma_Item dma_Tab[1];
    
    flags = QAPI_SDCC_DIR_WRITE_E | QAPI_SDCC_OP_INC_ADDR_E | QAPI_SDCC_NON_BLOCKING_E;
    if (block_flag)
        flags |= QAPI_SDCC_BLOCK_MODE_E;
    

    dma_Tab[0].data_Buf_Ptr = buf1_ptr;
    dma_Tab[0].size = buf1_sz;

    
    status = qapi_SDCCM_Send_Receive_Data_Scatter_Gather (sdio_Handle, flags, dev_Fn, reg_Addr, 
                                                    data_Len_In_Bytes, 1, dma_Tab);

    return status;
}

qapi_Status_t cmd53_read_CCCR_sg(qapi_SDCC_Handle sdio_Handle, uint8 *buf1_ptr, uint32 buf1_sz, 
                uint8 *buf2_ptr, uint32 buf2_sz, uint32_t data_Len_In_Bytes, uint32_t block_flag)
{
    qapi_Status_t  status;
    uint32 flags; 
    uint32 reg_Addr;
    
    flags = QAPI_SDCC_DIR_READ_E | QAPI_SDCC_OP_INC_ADDR_E;
    if (block_flag)
        flags |= QAPI_SDCC_BLOCK_MODE_E;
    
    reg_Addr = 0;
    status = cmd53_read_bytes_dma(sdio_Handle, reg_Addr, buf1_ptr, buf1_sz, 
                                            buf2_ptr, buf2_sz, data_Len_In_Bytes, block_flag);
    return status;
}

qapi_Status_t cmd53_read_FBR_sg(qapi_SDCC_Handle sdio_Handle, uint8 *buf1_ptr, uint32 buf1_sz, 
                uint8 *buf2_ptr, uint32 buf2_sz, uint32_t data_Len_In_Bytes, uint32_t block_flag)
{
    qapi_Status_t  status;
    uint32 flags; 
    uint32 reg_Addr;

    flags = QAPI_SDCC_DIR_READ_E | QAPI_SDCC_OP_INC_ADDR_E | QAPI_SDCC_NON_BLOCKING_E;
    if (block_flag)
        flags |= QAPI_SDCC_BLOCK_MODE_E;
    reg_Addr = 0x100;
    
    status = cmd53_read_bytes_dma(sdio_Handle, reg_Addr, buf1_ptr, buf1_sz, 
                                            buf2_ptr, buf2_sz, data_Len_In_Bytes, block_flag);
    return status;
}

qapi_Status_t cmd52_read_sdio_registers(qapi_SDCC_Handle sdio_Handle)
{
    uint32  reg_Addr;
    uint8   data_Out;
    uint16  u16_data;
    qapi_Status_t  status;
    
    QCLI_Printf(qcli_sdio_group, "Master SDIO CMD52\n");
    reg_Addr = 0;
    status = cmd52_read_one_byte(sdio_Handle, reg_Addr, &data_Out);
    if (status != QAPI_OK)
    {
        QCLI_Printf(qcli_sdio_group, "Master SDIO CMD52 read fails  result:%d\n", status);      
        return status;
    }   
    QCLI_Printf(qcli_sdio_group, "Master SDIO Reg[0x%x]=0x%02x\n", reg_Addr, data_Out);     

    reg_Addr = 1;
    status = cmd52_read_one_byte(sdio_Handle, reg_Addr, &data_Out);
    if (status != QAPI_OK)
    {
        QCLI_Printf(qcli_sdio_group, "Master SDIO CMD52 read fails  result:%d\n", status);      
        return status;
    }   
    QCLI_Printf(qcli_sdio_group, "Master SDIO Reg[0x%x]=0x%02x\n", reg_Addr, data_Out);     

    reg_Addr = SDIO_CCCR_SPEED;
    status = cmd52_read_one_byte(sdio_Handle, reg_Addr, &data_Out);
    if (status != QAPI_OK)
    {
        QCLI_Printf(qcli_sdio_group, "Master SDIO CMD52 read fails  result:%d\n", status);      
        return status;
    }   
    QCLI_Printf(qcli_sdio_group, "Master SDIO Reg[0x%x]=0x%02x\n", reg_Addr, data_Out);     

    reg_Addr = SDIO_CCCR_IOEx;
    status = cmd52_read_one_byte(sdio_Handle, reg_Addr, &data_Out);
    QCLI_Printf(qcli_sdio_group, "Master SDIO Reg[0x%x]=0x%02x\n", reg_Addr, data_Out);     


    reg_Addr = SDIO_CCCR_IORx;
    status = cmd52_read_one_byte(sdio_Handle, reg_Addr, &data_Out);
    QCLI_Printf(qcli_sdio_group, "Master SDIO Reg[0x%x]=0x%02x\n", reg_Addr, data_Out);     

    reg_Addr = SDIO_FBR_0;
    status = cmd52_read_one_byte(sdio_Handle, reg_Addr, &data_Out);
    QCLI_Printf(qcli_sdio_group, "Master SDIO FBR Reg0[0x%x]=0x%02x\n", reg_Addr, data_Out);        

    reg_Addr = SDIO_FBR_BLOCK_SIZE;
    status = cmd52_read_one_byte(sdio_Handle, reg_Addr, &data_Out);
    u16_data = data_Out;
    status = cmd52_read_one_byte(sdio_Handle, reg_Addr+1, &data_Out);
    u16_data |= (data_Out << 8);
    QCLI_Printf(qcli_sdio_group, "Master SDIO FBR Block Size[0x%x]=0x%02x\n", reg_Addr, u16_data);      

    reg_Addr = 0x418;
    status = cmd52_read_one_byte(sdio_Handle, reg_Addr, &data_Out);
    u16_data = data_Out;

    
    QCLI_Printf(qcli_sdio_group, "Master SDIO host ctrl regs[0x%x]=0x%02x\n", reg_Addr, data_Out);  

    return QAPI_OK;
}

qapi_Status_t cmd53_write_sdio_reg(qapi_SDCC_Handle sdio_Handle, uint32 dev_Fn, uint32 reg_Addr,  uint8 *data_Out, uint32_t block_flag)
{
    qapi_Status_t  status;
    uint32 flags; 
   
    flags = QAPI_SDCC_DIR_WRITE_E | QAPI_SDCC_OP_INC_ADDR_E;
    if (block_flag)
        flags |= QAPI_SDCC_BLOCK_MODE_E;

    QCLI_Printf(qcli_sdio_group, "Writing Reg[0x%x]=0x%02x\n", reg_Addr, *data_Out);

    status = qapi_SDCCM_Receive_Data_Ext (sdio_Handle, flags, dev_Fn, reg_Addr, 1, data_Out);
    return status;
}
qapi_Status_t cmd53_write_sdio_reg_word(qapi_SDCC_Handle sdio_Handle, uint32 dev_Fn, uint32 reg_Addr,  uint8 *data_Out, uint32_t block_flag)
{
    qapi_Status_t  status;
    uint32 flags; 
    
    flags = QAPI_SDCC_DIR_WRITE_E | QAPI_SDCC_OP_INC_ADDR_E;
    if (block_flag)
        flags |= QAPI_SDCC_BLOCK_MODE_E;
    
    if (block_flag)
        QCLI_Printf(qcli_sdio_group, "Writing Reg[0x%x]=0x%08x\n", reg_Addr, *(uint32*)data_Out);
    else
        QCLI_Printf(qcli_sdio_group, "Writing Reg[0x%x]=0x%02x\n", reg_Addr, *data_Out);

    status = qapi_SDCCM_Receive_Data_Ext (sdio_Handle, flags, dev_Fn, reg_Addr, 1, data_Out);
    return status;
}

qapi_Status_t cmd53_read_sdio_reg(qapi_SDCC_Handle sdio_Handle, uint32 dev_Fn, uint32 reg_Addr,  uint8 *data_Out, uint32_t block_flag)
{
    qapi_Status_t  status;
    uint32 flags; 
    
    flags = QAPI_SDCC_DIR_READ_E /*| QAPI_SDCC_OP_INC_ADDR_E*/;
    if (block_flag)
        flags |= QAPI_SDCC_BLOCK_MODE_E;

    *data_Out = 0;

    status = qapi_SDCCM_Receive_Data_Ext (sdio_Handle, flags, dev_Fn, reg_Addr, 1, data_Out);
    QCLI_Printf(qcli_sdio_group, "Reg[0x%x]=0x%02x\n", reg_Addr, *data_Out);
    return status;
}
qapi_Status_t cmd53_read_CCCR(qapi_SDCC_Handle sdio_Handle, uint8 *data_Out, uint32_t data_Len_In_Bytes, uint32_t block_flag)
{
    qapi_Status_t  status;
    uint32 flags; 
    uint32 dev_Fn;
    uint32 reg_Addr;
    
    flags = QAPI_SDCC_DIR_READ_E | QAPI_SDCC_OP_INC_ADDR_E;
    if (block_flag)
        flags |= QAPI_SDCC_BLOCK_MODE_E;
    
    dev_Fn = 0;
    reg_Addr = 0;
    
    status = qapi_SDCCM_Receive_Data_Ext (sdio_Handle, flags, dev_Fn, reg_Addr, data_Len_In_Bytes, data_Out);
    return status;
}

qapi_Status_t cmd53_read_FBR(qapi_SDCC_Handle sdio_Handle, uint8 *data_Out, uint32_t data_Len_In_Bytes, uint32_t block_flag)
{
    qapi_Status_t  status;
    uint32 flags; 
    uint32 dev_Fn;
    uint32 reg_Addr;

    flags = QAPI_SDCC_DIR_READ_E | QAPI_SDCC_OP_INC_ADDR_E | QAPI_SDCC_NON_BLOCKING_E;
    if (block_flag)
        flags |= QAPI_SDCC_BLOCK_MODE_E;

    dev_Fn = 0;
    reg_Addr = 0x100;
    
    status = qapi_SDCCM_Receive_Data_Ext (sdio_Handle, flags, dev_Fn, reg_Addr, data_Len_In_Bytes, data_Out);
    return status;
}

qapi_Status_t cmd53_read_HostControlRegs(qapi_SDCC_Handle sdio_Handle, uint32 reg_Addr, uint8 *data_Out, uint32_t data_Len_In_Bytes, uint32_t block_flag)
{
    qapi_Status_t  status;
    uint32 flags; 
    uint32 dev_Fn;
    
    flags = QAPI_SDCC_DIR_READ_E | QAPI_SDCC_OP_INC_ADDR_E | QAPI_SDCC_NON_BLOCKING_E;
    if (block_flag)
        flags |= QAPI_SDCC_BLOCK_MODE_E;

    dev_Fn = 1;
    
    status = qapi_SDCCM_Receive_Data_Ext (sdio_Handle, flags, dev_Fn, reg_Addr, data_Len_In_Bytes, data_Out);
    return status;
}

qapi_Status_t cmd53_write_HostControlRegs(qapi_SDCC_Handle sdio_Handle, uint32 reg_Addr, uint8 *data_Out, uint32_t data_Len_In_Bytes, uint32_t block_flag)
{
    qapi_Status_t  status;
    uint32 flags; 
    uint32 dev_Fn;
    
    flags = QAPI_SDCC_DIR_WRITE_E | QAPI_SDCC_OP_INC_ADDR_E | QAPI_SDCC_NON_BLOCKING_E;
    if (block_flag)
        flags |= QAPI_SDCC_BLOCK_MODE_E;
    
    dev_Fn = 1;
    status = qapi_SDCCM_Receive_Data_Ext (sdio_Handle, flags, dev_Fn, reg_Addr, data_Len_In_Bytes, data_Out);
    return status;
}

qapi_Status_t cmd53_read_Common_CIS(qapi_SDCC_Handle sdio_Handle, uint8 *data_Out, uint32_t data_Len_In_Bytes, uint32_t block_flag)
{
    qapi_Status_t  status;
    uint32 flags; 
    uint32 dev_Fn;
    uint32 reg_Addr, i =0;

    flags = QAPI_SDCC_DIR_READ_E | QAPI_SDCC_OP_INC_ADDR_E | QAPI_SDCC_NON_BLOCKING_E;
    if (block_flag)
        flags |= QAPI_SDCC_BLOCK_MODE_E;

    dev_Fn = 0;
    reg_Addr = 0x1000;

    do {
        status = qapi_SDCCM_Receive_Data_Ext (sdio_Handle, flags, dev_Fn, reg_Addr, 2, data_Out);
        reg_Addr += 2;

        if ( data_Out[0] == 0xFF )
        {
            QCLI_Printf(qcli_sdio_group, "End of common tuple\n");
            break;
        }
        
        if ((data_Out[1] == 0xFF))
        {
            QCLI_Printf(qcli_sdio_group, "End of common tuple - 2\n");
            break;
        }

        if ( data_Out[1] != 0)
            status = qapi_SDCCM_Receive_Data_Ext (sdio_Handle, flags, dev_Fn, reg_Addr, data_Out[1], &data_Out[2]);
        
        QCLI_Printf(qcli_sdio_group, "Dumping Tuple %x of len %x\n", data_Out[0], data_Out[1]);
        for (i =0; i<(data_Out[1] ); i++)
        {
            QCLI_Printf(qcli_sdio_group, "%2x\n", data_Out[2+i] );
        }
        
        /* Get ready to read next tuple */
        reg_Addr += data_Out[1];
        data_Out += (data_Out[1] + 2 );

    }while (1);
    
    return status;
}


qapi_Status_t cmd53_read_func_one_CIS(qapi_SDCC_Handle sdio_Handle, uint8 *data_Out, uint32_t data_Len_In_Bytes, uint32_t block_flag)
{
    qapi_Status_t  status;
    uint32 flags; 
    uint32 dev_Fn;
    uint32 reg_Addr, i =0;

    flags = QAPI_SDCC_DIR_READ_E | QAPI_SDCC_OP_INC_ADDR_E | QAPI_SDCC_NON_BLOCKING_E;
    if (block_flag)
        flags |= QAPI_SDCC_BLOCK_MODE_E;

    dev_Fn = 0;
    reg_Addr = 0x1100;
    
    do {
        status = qapi_SDCCM_Receive_Data_Ext (sdio_Handle, flags, dev_Fn, reg_Addr, 2, data_Out);
        reg_Addr += 2;

        if ( data_Out[0] == 0xFF )
        {
            QCLI_Printf(qcli_sdio_group, "End of func one tuple\n");
            break;
        }
        
        if ((data_Out[1] == 0xFF))
        {
            QCLI_Printf(qcli_sdio_group, "End of func one tuple - 2\n");
            break;
        }

        if ( data_Out[1] != 0)
            status = qapi_SDCCM_Receive_Data_Ext (sdio_Handle, flags, dev_Fn, reg_Addr, data_Out[1], &data_Out[2]);
        
        QCLI_Printf(qcli_sdio_group, "Dumping Tuple %x of len %x\n", data_Out[0], data_Out[1]);
        for (i =0; i<(data_Out[1] ); i++)
        {
            QCLI_Printf(qcli_sdio_group, "%2x\n", data_Out[2+i] );
        }
        
        /* Get ready to read next tuple */
        reg_Addr += data_Out[1];
        data_Out += (data_Out[1] + 2 );
    }while (1);
    
    return status;
}

qapi_Status_t cmd53_read_function_0_bytes(qapi_SDCC_Handle sdio_Handle)
{
    qapi_Status_t  status;
    uint32     i;
    uint32  reg_Addr;
    uint8   data_Out;

    QCLI_Printf(qcli_sdio_group, "Master SDIO CMD53 byte \n");
    status = cmd53_read_CCCR( sdio_Handle, data, 0x17, 0);
    if (status != QAPI_OK)
    {
        QCLI_Printf(qcli_sdio_group, "Master SDIO CMD53 read CCCR fails  result:%d\n", status);     
        return status;
    }   

    for (i=0; i <= 8; i++)
    {
       QCLI_Printf(qcli_sdio_group, "%02x   %02x\n", i, data[i]);               
    }
    QCLI_Printf(qcli_sdio_group, "[09H~0BH] = %02x%02x%02X\n", data[9], data[10], data[11]);                
    
    for (i=0x0c; i <= 0x0F; i++)
    {
       QCLI_Printf(qcli_sdio_group, "%02x   %02x\n", i, data[i]);               
    }
    QCLI_Printf(qcli_sdio_group, "[10H~11H] = %02X%02X\n", data[0x10], data[0x11]);             
    
    for (i=0x12; i <= 0x16; i++)
    {
       QCLI_Printf(qcli_sdio_group, "%02x   %02x\n", i, data[i]);               
    }
    
    QCLI_Printf(qcli_sdio_group, "\n");             
    status = cmd53_read_FBR(sdio_Handle, data, 0x12, 0);
    if (status != QAPI_OK)
    {
        QCLI_Printf(qcli_sdio_group, "Master SDIO CMD53 read CCCR fails  result:%d\n", status);     
        return status;
    }

    QCLI_Printf(qcli_sdio_group, "\n");             
    
    
    for (i=0; i <= 2; i++)
    {
       QCLI_Printf(qcli_sdio_group, "%02x   %02x\n", i+0x100, data[i]);             
    }
    QCLI_Printf(qcli_sdio_group, "[109H~10BH] = %02x%02x%02X\n", data[9], data[10], data[11]);              
    QCLI_Printf(qcli_sdio_group, "[10CH~10EH] = %02x%02x%02X\n", data[0xc], data[0xd], data[0xe]);              
    QCLI_Printf(qcli_sdio_group, "[10FH] = %02x\n", data[0xf]);             
    QCLI_Printf(qcli_sdio_group, "[110H~111H] = %02x%02x\n", data[0x10], data[0x11]);                   

    QCLI_Printf(qcli_sdio_group, "Enable Func 1 \n");
    reg_Addr = SDIO_CCCR_IOEx;
    data_Out = 0x3;
    status = cmd52_write_one_byte(sdio_Handle, reg_Addr, &data_Out);

    reg_Addr = SDIO_CCCR_IOEx;
    status = cmd52_read_one_byte(sdio_Handle, reg_Addr, &data_Out);
    QCLI_Printf(qcli_sdio_group, "Master SDIO Reg[0x%x]=0x%02x\n", reg_Addr, data_Out);     

    QCLI_Printf(qcli_sdio_group, "Dumping HCR\n");    
    memset(data, 0, 1024);
    reg_Addr = 0x418;
    status = cmd53_read_HostControlRegs(sdio_Handle, reg_Addr, data, 0x4, 0);
    if (status != QAPI_OK)
    {
        QCLI_Printf(qcli_sdio_group, "Master SDIO CMD53 read Host control registers fails  result:%d\n", status);       
        return status;
    }  
    for (i=0; i <= 0x3; i++)
        {
           QCLI_Printf(qcli_sdio_group, "%02x   %02x\n", i+0x418, data[i]);
        }

    data[0] = 0xFF;
    data[1] = 0xFF;
    data[2] = 0xFF;
    data[3] = 0xFF;    
    reg_Addr = 0x418;

    status = cmd53_write_HostControlRegs(sdio_Handle, reg_Addr, data, 0x4, 0);
    if (status != QAPI_OK)
    {
        QCLI_Printf(qcli_sdio_group, "Master SDIO CMD53 read Host control registers fails  result:%d\n", status);       
        return status;
    }  
    for (i=0; i <= 0x3; i++)
        {
           QCLI_Printf(qcli_sdio_group, "%02x   %02x\n", i+0x418, data[i]);
        }    

    memset(data, 0, 1024);
    reg_Addr = 0x418;
    status = cmd53_read_HostControlRegs(sdio_Handle, reg_Addr, data, 0x4, 0);
    if (status != QAPI_OK)
    {
        QCLI_Printf(qcli_sdio_group, "Master SDIO CMD53 read Host control registers fails  result:%d\n", status);       
        return status;
    }  
    for (i=0; i <= 0x3; i++)
        {
           QCLI_Printf(qcli_sdio_group, "%02x   %02x\n", i+0x418, data[i]);
        }
    
    
    QCLI_Printf(qcli_sdio_group, "Dumping common cis\n");
    memset(cis, 0, 4096);
    cmd53_read_Common_CIS(sdio_Handle, cis, 0, 0);

    QCLI_Printf(qcli_sdio_group, "Dumping func 1 cis\n");    
    memset(cis, 0, 4096);
    cmd53_read_func_one_CIS(sdio_Handle, cis, 0, 0);

    return QAPI_OK;
}   

qapi_Status_t cmd53_read_function_0_blocks(qapi_SDCC_Handle sdio_Handle)
{
    qapi_Status_t  status;
    uint32     i;
    
    QCLI_Printf(qcli_sdio_group, "Master SDIO CMD53 block\n");
    status = cmd53_read_CCCR( sdio_Handle, data, 0x20, 1);
    if (status != QAPI_OK)
    {
        QCLI_Printf(qcli_sdio_group, "Master SDIO CMD53 read CCCR fails  result:%d\n", status);     
        return status;
    }   

    for (i=0; i <= 8; i++)
    {
       QCLI_Printf(qcli_sdio_group, "%02x   %02x\n", i, data[i]);               
    }
    QCLI_Printf(qcli_sdio_group, "[09H~0BH] = %02x%02x%02X\n", data[9], data[10], data[11]);                
    
    for (i=0x0c; i <= 0x0F; i++)
    {
       QCLI_Printf(qcli_sdio_group, "%02x   %02x\n", i, data[i]);               
    }
    QCLI_Printf(qcli_sdio_group, "[10H~11H] = %02X%02X\n", data[0x10], data[0x11]);             
    
    for (i=0x12; i <= 0x16; i++)
    {
       QCLI_Printf(qcli_sdio_group, "%02x   %02x\n", i, data[i]);               
    }
    
    QCLI_Printf(qcli_sdio_group, "\n");             
    status = cmd53_read_FBR(sdio_Handle, data, 0x20, 1);
    if (status != QAPI_OK)
    {
        QCLI_Printf(qcli_sdio_group, "Master SDIO CMD53 read CCCR fails  result:%d\n", status);     
        return status;
    }
    
    for (i=0; i <= 2; i++)
    {
       QCLI_Printf(qcli_sdio_group, "%02x   %02x\n", i+0x100, data[i]);             
    }
    QCLI_Printf(qcli_sdio_group, "[109H~10BH] = %02x%02x%02X\n", data[9], data[10], data[11]);              
    QCLI_Printf(qcli_sdio_group, "[10CH~10EH] = %02x%02x%02X\n", data[0xc], data[0xd], data[0xe]);              
    QCLI_Printf(qcli_sdio_group, "[10FH] = %02x\n", data[0xf]);             
    QCLI_Printf(qcli_sdio_group, "[110H~111H] = %02x%02x\n", data[0x10], data[0x11]);

    return status;
}   

qapi_Status_t cmd53_read_function_0_scatter_gather(qapi_SDCC_Handle sdio_Handle)
{
    qapi_Status_t  status;
    uint32     i;
    uint32     buf1[3], buf2[5];
    uint32     buf1_sz = 12 , buf2_sz = 20;
    uint32     data_Len_In_Bytes = 32;
    uint8      *pBuf;
    
    QCLI_Printf(qcli_sdio_group, "Master SDIO CMD53 byte Scatter & Gather\n");
    status = cmd53_read_CCCR_sg(sdio_Handle, (uint8 *)buf1, buf1_sz, (uint8 *)buf2, buf2_sz, data_Len_In_Bytes, 0);

    if (status != QAPI_OK)
    {
        QCLI_Printf(qcli_sdio_group, "Master SDIO CMD53 read CCCR fails  result:%d\n", status);     
        return status;
    }   

    pBuf = (uint8 *)buf1;
    for (i=0; i <= 8; i++)
    {
       QCLI_Printf(qcli_sdio_group, "%02x   %02x\n", i, pBuf[i]);               
    }
    QCLI_Printf(qcli_sdio_group, "[09H~0BH] = %02x%02x%02X\n", pBuf[9], pBuf[10], pBuf[11]);                
    
    pBuf = (uint8 *)buf2;
    for (i=0; i <= 3; i++)
    {
       QCLI_Printf(qcli_sdio_group, "%02x   %02x\n", i, pBuf[i]);               
    }
    QCLI_Printf(qcli_sdio_group, "[10H~11H] = %02X%02X\n", pBuf[4], pBuf[5]);               
    
    for (i=6; i <= 10; i++)
    {
       QCLI_Printf(qcli_sdio_group, "%02x   %02x\n", i, pBuf[i]);               
    }
    
    QCLI_Printf(qcli_sdio_group, "\n");             
    status = cmd53_read_FBR_sg(sdio_Handle, (uint8 *)buf2, buf2_sz, (uint8 *)buf1, buf1_sz, data_Len_In_Bytes, 0);
    if (status != QAPI_OK)
    {
        QCLI_Printf(qcli_sdio_group, "Master SDIO CMD53 read CCCR fails  result:%d\n", status);     
        return status;
    }
    
    pBuf = (uint8 *)buf2;
    for (i=0; i <= 2; i++)
    {
       QCLI_Printf(qcli_sdio_group, "%02x   %02x\n", i+0x100, pBuf[i]);             
    }
    QCLI_Printf(qcli_sdio_group, "[109H~10BH] = %02x%02x%02X\n", pBuf[9], pBuf[10], pBuf[11]);              
    QCLI_Printf(qcli_sdio_group, "[10CH~10EH] = %02x%02x%02X\n", pBuf[0xc], pBuf[0xd], pBuf[0xe]);              
    QCLI_Printf(qcli_sdio_group, "[10FH] = %02x\n", pBuf[0xf]);             
    QCLI_Printf(qcli_sdio_group, "[110H~111H] = %02x%02x\n", pBuf[0x10], pBuf[0x11]);

    return status;  
}
