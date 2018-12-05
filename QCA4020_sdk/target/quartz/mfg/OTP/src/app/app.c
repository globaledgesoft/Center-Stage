/*
 * Copyright (c) 2017 Qualcomm Technologies, Inc.
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

/*-------------------------------------------------------------------------
 * Include Files
 *-----------------------------------------------------------------------*/
#include <stdio.h>
#include <stdarg.h>
#include "stringl.h"
#include "otp_params.h"
#include "app.h"


/*-------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 *-----------------------------------------------------------------------*/
#define OTP_DBG_ENABLE 1


/*-------------------------------------------------------------------------
 * Static & global Variable Declarations
 *-----------------------------------------------------------------------*/
extern uint32_t kdf_enable;
extern uint32_t secure_boot_enable;
extern uint32_t firmware_region_write_disable;
extern uint32_t model_id;
extern uint8_t pk_hash[];
extern uint32_t pk_hash_size;
extern uint8_t enc_key[];
extern uint32_t enc_key_size;
extern uint32_t otp_profile;
 
static PAL_Context_t PAL_Context;
uint8_t Printf_Buffer[256];

/*-------------------------------------------------------------------------
 * Function Definitions
 *-----------------------------------------------------------------------*/
/**
   @brief This function handles transmit callbacks from the UART.

   @param Num_Bytes[in] is the number of bytes transmitted.
   @param CB_Data[in]   is the application defined callback data.
*/
static void Uart_Tx_CB(uint32 Num_Bytes, void* CB_Data)
{
}

/**
   @brief This function handles receive callbacks from the UART.

   @param Num_Bytes[in] is the number of bytes received.
   @param CB_Data[in]   is the application defined callback data.  In this case
                        it is the index of the buffer received on.
*/
static void Uart_Rx_CB(uint32 Num_Bytes, void* CB_Data)
{
}

qbool_t PAL_Uart_Init(int reinit_flag)
{
    qapi_UART_Open_Config_t Uart_Config;
    uint8_t                 Ret_Val;
    uint32_t                Index;

    Uart_Config.baud_Rate        = 115200;
    Uart_Config.parity_Mode      = QAPI_UART_NO_PARITY_E;
    Uart_Config.num_Stop_Bits    = QAPI_UART_1_0_STOP_BITS_E;
    Uart_Config.bits_Per_Char    = QAPI_UART_8_BITS_PER_CHAR_E;
    Uart_Config.enable_Loopback  = FALSE;
    Uart_Config.enable_Flow_Ctrl = FALSE;
    Uart_Config.tx_CB_ISR        = (qapi_UART_Callback_Fn_t) Uart_Tx_CB;
    Uart_Config.rx_CB_ISR        = (qapi_UART_Callback_Fn_t) Uart_Rx_CB;
	PAL_Context.Uart_Enable      = 1;
	
      if(qapi_UART_Open(&(PAL_Context.Console_UART), PAL_CONSOLE_PORT, &Uart_Config) == QAPI_OK)
      {
         /* Queue the receives. */
         for(Index = 0; Index < PAL_RECIEVE_BUFFER_COUNT; Index ++)
         {
            qapi_UART_Receive(PAL_Context.Console_UART, (char *)(PAL_Context.Rx_Buffer[Index]), PAL_RECIEVE_BUFFER_SIZE, (void *)Index);
         }

         Ret_Val = true;
      }
      else
      {
         Ret_Val = false;
      }

   return(Ret_Val);
}
/**
   @brief This function is used to initialize the Platform, predominately
          the console port.

   @return
    - true if the platform was initialized successfully.
    - false if initialization failed.
*/
static qbool_t PAL_Initialize(void)
{
   uint8_t                 Ret_Val;

   memset(&PAL_Context, 0, sizeof(PAL_Context));
   PAL_Context.Rx_Buffers_Free = PAL_RECIEVE_BUFFER_COUNT;

   Ret_Val = PAL_Uart_Init(0);

   return(Ret_Val);
}

void OTP_Printf(const char *Format, ...){
	int Length;
	va_list             Arg_List;
	   
/* Print the string to the buffer. */
    va_start(Arg_List, Format);
    Length = vsnprintf((char *)(Printf_Buffer), sizeof(Printf_Buffer), (char *)Format, Arg_List);
    va_end(Arg_List);

    /* Make sure the length is not greater than the buffer size (taking the
            NULL terminator into account). */
    if(Length > sizeof(Printf_Buffer) - 1)
    {
       Length = sizeof(Printf_Buffer) - 1;
    }
#if OTP_DBG_ENABLE == 1
    PAL_Console_Write(Length, &(Printf_Buffer[0]));

    PAL_Console_Write(sizeof(PAL_OUTPUT_END_OF_LINE_STRING) - 1, PAL_OUTPUT_END_OF_LINE_STRING);
#endif	
}

/**
   @brief This function is used to write a buffer to the console. Note
          that when this function returns, all data from the buffer will
          be written to the console or buffered locally.

   @param Length is the length of the data to be written.
   @param Buffer is a pointer to the buffer to be written to the console.
*/
void PAL_Console_Write(uint32_t Length, const char *Buffer)
{
    int i=0;
    if((Length) && (Buffer) && (PAL_Context.Uart_Enable))
    {
        /* Transmit the data. */
        qapi_UART_Transmit(PAL_Context.Console_UART, (char *)Buffer, Length, NULL);
    }
    
    /* Add delay to allow UART TX to complete */
    while(i < 0x8000)
	   i++;
}

void Display_Failure(char *info, uint32_t reason)
{
    char *error_message[] ={
        "success",
        "otp ops error",
        "enc key len error",
        "pk hash len error",
        "invalid otp profile setting",
    };
    if( reason < OTP_STATUS_ERR_LAST)
        OTP_Printf("%s: %s\n", info, error_message[reason]);
    else
        OTP_Printf("%s: reason is invalid number\n", info);
    return;
}

/**
   @brief This function is the main entry point for the application.
*/
void app_start(qbool_t ColdBoot)
{
    uint32_t rtn, tag;
    
#if OTP_DBG_ENABLE == 1   
   PAL_Initialize();
#endif
    tag = 0;

    if( (rtn = OTP_set_kdf(kdf_enable, enc_key, enc_key_size )) != OTP_STATUS_SUCCESS )
    {
        Display_Failure("Fail to set KDF", rtn);
        tag = 1;
    }
    
    if(  (rtn = OTP_set_secure_boot(secure_boot_enable, pk_hash, pk_hash_size)) != OTP_STATUS_SUCCESS )
    {
        Display_Failure("Fail to set secure boot", rtn);
        tag = 1;
    }

    if( (rtn = OTP_set_firmware_region_write_disable(firmware_region_write_disable)) != OTP_STATUS_SUCCESS ) 
    {
        Display_Failure("Fail to set firmware region write enable", rtn);
        tag = 1;
    }

    if( (rtn = OTP_set_model_id(model_id)) != OTP_STATUS_SUCCESS ) 
    {
        Display_Failure("Fail to set model id", rtn);
        tag = 1;
    }

    if( (rtn = OTP_set_profile(otp_profile)) != OTP_STATUS_SUCCESS )
    {
        Display_Failure("Fail to set otp profile", rtn);
        tag = 1;
    }        

    if( tag != 1 )
    {
        OTP_Printf("OTP update success\n");
    }
otp_end:
    return; 
}
