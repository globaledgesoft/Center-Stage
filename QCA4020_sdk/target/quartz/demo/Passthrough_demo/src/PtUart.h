/*
 * Copyright (c) 2016-2017 Qualcomm Technologies, Inc.
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

#ifndef __PTUART_H__
#define __PTUART_H__

#define PT_UART_ERROR_SUCCESS                      (0)
#define PT_UART_ERROR_UNABLE_TO_OPEN_TRANSPORT     (-1)
#define PT_UART_ERROR_WRITING_TO_PORT              (-2)

typedef struct PtUart_Initialization_Data_s
{
   unsigned int Flags;
   unsigned int BaudRate;
} PtUart_Initialization_Data_t;

#define PT_UART_INITIALIZATION_DATA_FLAG_USE_HS_UART     (0x1)
#define PT_UART_INITIALIZATION_DATA_FLAG_USE_HW_FLOW     (0x2)

typedef void (*PtUart_Callback_t)(unsigned int DataLength, unsigned char *DataBuffer, unsigned long CallbackParameter);

int PtUart_Initialize(PtUart_Initialization_Data_t *InitializationData, PtUart_Callback_t PtUartCallback, unsigned long CallbackParameter);
void PtUart_UnInitialize(void);
int PtUart_Write(unsigned int Length, unsigned char *Buffer);
void PtUart_Process_Packet(unsigned int Length, const uint8_t *Data);
int PtUart_Wake(void);


   /* Needed types not in QAPIs.                                        */

typedef uint32_t  uint32;

typedef enum
{
   UART_FIRST_PORT = 0,
   UART_DEBUG_PORT = UART_FIRST_PORT,

   UART_SECOND_PORT,
   UART_HS_PORT = UART_SECOND_PORT,

   UART_MAX_PORTS,
}uart_port_id;

typedef enum
{
   UART_SUCCESS = 0,
   UART_ERROR,
}uart_result;

typedef enum
{
  UART_5_BITS_PER_CHAR  = 0,
  UART_6_BITS_PER_CHAR  = 1,
  UART_7_BITS_PER_CHAR  = 2,
  UART_8_BITS_PER_CHAR  = 3,
} uart_bits_per_char;

typedef enum
{
  UART_0_5_STOP_BITS    = 0,
  UART_1_0_STOP_BITS    = 1,
  UART_1_5_STOP_BITS    = 2,
  UART_2_0_STOP_BITS    = 3,
} uart_num_stop_bits;

typedef enum
{
  UART_NO_PARITY        = 0,
  UART_ODD_PARITY       = 1,
  UART_EVEN_PARITY      = 2,
  UART_SPACE_PARITY     = 3,
} uart_parity_mode;

typedef void* uart_handle;

typedef void(*UART_CALLBACK)(uint32 num_bytes, void *cb_data);

typedef struct
{
   uint32                baud_rate;
   uart_parity_mode      parity_mode;
   uart_num_stop_bits    num_stop_bits;
   uart_bits_per_char    bits_per_char;
   uint32                enable_loopback;
   uint32                enable_flow_ctrl;
   UART_CALLBACK         tx_cb_isr;
   UART_CALLBACK         rx_cb_isr;
}uart_open_config;

extern uart_result uart_transmit(uart_handle h, char* buf, uint32 bytes_to_tx, void* cb_data);
extern uart_result uart_receive(uart_handle h, char* buf, uint32 buf_size, void* cb_data);
extern uart_result uart_open(uart_handle* h, uart_port_id id, uart_open_config* config);
extern uart_result uart_close(uart_handle h);

#endif

