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
 * sdio.h
 *
 * Header file for SDCC Master side transport interface layer
 *
 *
 ******************************************************************************/
/*=============================================================================


                              EDIT HISTORY 

  $Header: 

when         who     what, where, why
----------   ---     ---------------------------------------------------------- 
2017-07-11   mmtd      Initial creation


======================================================================*/
#define SDIO_CCCR_IOEx          0x02
#define SDIO_CCCR_IORx          0x03
#define SDIO_CCCR_IENx          0x04 /* Function and Master interrupt enable */

#define SDIO_CCCR_ABORT         0x06 /* function abort; card reset */
#define SDIO_IO_RESET           (1 << 3) 

#define SDIO_CCCR_IF            0x07 /* bus interface controls */
#define SDIO_CCCR_SPEED         0x13 /* sdio speed */

#define SDIO_CCCR_DRIVE_STRENGTH 0x15
#define  SDIO_SDTx_MASK         0x07
#define  SDIO_DRIVE_SDTA        (1<<0)
#define  SDIO_DRIVE_SDTC        (1<<1)
#define  SDIO_DRIVE_SDTD        (1<<2)
#define  SDIO_DRIVE_DTSx_MASK   0x03
#define  SDIO_DRIVE_DTSx_SHIFT  4
#define  SDIO_DTSx_SET_TYPE_B   (0 << SDIO_DRIVE_DTSx_SHIFT)
#define  SDIO_DTSx_SET_TYPE_A   (1 << SDIO_DRIVE_DTSx_SHIFT)
#define  SDIO_DTSx_SET_TYPE_C   (2 << SDIO_DRIVE_DTSx_SHIFT)
#define  SDIO_DTSx_SET_TYPE_D   (3 << SDIO_DRIVE_DTSx_SHIFT)

/* Async int delay */
#define CCCR_SDIO_ASYNC_INT_DELAY_ADDRESS       0xF0
#define CCCR_SDIO_ASYNC_INT_DELAY_LSB           0x06
#define CCCR_SDIO_ASYNC_INT_DELAY_MASK          0xC0

/* Vendor Specific Driver Strength Settings */
#define CCCR_SDIO_DRIVER_STRENGTH_ENABLE_ADDR   0xf2
#define CCCR_SDIO_DRIVER_STRENGTH_ENABLE_MASK   0x0e
#define CCCR_SDIO_DRIVER_STRENGTH_ENABLE_A      0x02
#define CCCR_SDIO_DRIVER_STRENGTH_ENABLE_C      0x04
#define CCCR_SDIO_DRIVER_STRENGTH_ENABLE_D      0x08

#define SDIO_FBR_0              0x100 /* sdio FBR 0 */
#define SDIO_FBR_BLOCK_SIZE     0x110 /* sdio FBR 0 */

#define  SDIO_BUS_CD_DISABLE     0x80   /* disable pull-up on DAT3 (pin 1) */
#define  SDIO_BUS_WIDTH_MASK    0x03    /* data bus width setting */
#define  SDIO_BUS_WIDTH_1BIT    0x00
#define  SDIO_BUS_WIDTH_RESERVED 0x01
#define  SDIO_BUS_WIDTH_4BIT    0x02

#define CUSTOM_HW_POWER_UP_PULSE_WIDTH  5


/* Device context */
typedef struct sdio_device {
    uint32                  id; 
    void *                  handle;
    uint32                  asyncintdelay;
    qapi_SDCC_Config_t      sdio_Config;
} sdio_device_t;
