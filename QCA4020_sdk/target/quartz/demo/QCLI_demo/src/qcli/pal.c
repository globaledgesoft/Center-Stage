/*
 * Copyright (c) 2015-2018 Qualcomm Technologies, Inc.
 * 2015-2016 Qualcomm Atheros, Inc.
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

#include "qapi_types.h"

#include "pal.h"
#include "qcli.h"

#include "qurt_error.h"
#include "qurt_thread.h"
#include "qurt_signal.h"

#include "qapi/qapi.h"
#include "qapi/qapi_status.h"
#include "qapi/qapi_uart.h"
#include "qapi/qapi_reset.h"
#include "qapi/qapi_tlmm.h"
#include "qapi/qapi_wlan_base.h"

#include "spple_demo.h"
#include "hmi_demo.h"
#include "coex_demo.h"
#include "ota_demo.h"
#include "lp_demo.h"
#include "wifi_demo.h"
#include "net_demo.h"
#include "adss_demo.h"
#include "fs_demo.h"
#include "securefs_demo.h"
#include "crypto_demo.h"
#include "platform_demo.h"
#include "zigbee_demo.h"
#include "thread_demo.h"
#include "peripherals_demo.h"
#include "zigbee_demo.h"
#include "ecosystem_demo.h"
#include "json_demo.h"
#include "kpi_demo.h"
#ifdef CONFIG_QMESH_DEMO
#include "qmesh_demo_menu.h"

#ifdef CONFIG_QMESH_COEX_DEMO
#include "qmesh_ble_coex.h"
#endif

#endif

#ifdef ENABLE_CPU_PROFILER
   #include "cpu_profiler_demo.h"
#endif

#ifdef ENABLE_DBGCALL
   #include "dbgcall.h"
#endif
#ifdef CONFIG_FLASHLOG_DEMO
    #include "flashlog_demo.h"
#endif

/*-------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 *-----------------------------------------------------------------------*/
#ifdef CONFIG_CDB_PLATFORM
#define PAL_CONSOLE_PORT                                QAPI_UART_DEBUG_PORT_E
#else
#define PAL_CONSOLE_PORT                                QAPI_UART_HS_PORT_E
#endif

#define PAL_RECIEVE_BUFFER_SIZE                         (128)
#define PAL_RECIEVE_BUFFER_COUNT                        (2)

#define PAL_EVENT_MASK_RECEIVE                          (0x00000001)
#define PAL_EVENT_MASK_TRANSMIT                         (0x00000002)

#define PAL_THREAD_STACK_SIZE                           (3072)
#define PAL_THREAD_PRIORITY                             (10)

#define PAL_ENTER_CRITICAL()                            do { __asm("cpsid i"); } while(0)
#define PAL_EXIT_CRITICAL()                             do { __asm("cpsie i"); } while(0)

/* The following is a simple macro to facilitate printing strings directly
   to the console. As it uses the sizeof operator on the size of the string
   provided, it is intended to be used with string literals and will not
   work correctly with pointers.
*/
#define PAL_CONSOLE_WRITE_STRING_LITERAL(__String__)    do { PAL_Console_Write(sizeof(__String__) - 1, (__String__)); } while(0)


#ifndef	CFG_FEATURE_CHK_POINT_GPIO_PIN
#define  CFG_FEATURE_CHK_POINT_GPIO_PIN 56
#endif

uint32  chk_pt_gpio_pin = CFG_FEATURE_CHK_POINT_GPIO_PIN;
uint32  chk_pt_gpio_status = 0;

qapi_TLMM_Config_t chk_pt_gpio_config;
qapi_GPIO_ID_t    qapi_GPIO_ID;

/**
   @brief This function toggles GPIO for measuring boot time.
*/

void app_trigger_signal_on_GPIO (uint32 chk_pt_num)
{
	qapi_Status_t status;
	
	if ((chk_pt_gpio_status & 0x04) == 0)
	{
    /* Pin configuration */
		chk_pt_gpio_config.pin = chk_pt_gpio_pin;
		chk_pt_gpio_config.func = 0;
		chk_pt_gpio_config.dir = QAPI_GPIO_OUTPUT_E;
		chk_pt_gpio_config.pull = QAPI_GPIO_PULL_UP_E;
		chk_pt_gpio_config.drive = QAPI_GPIO_2MA_E;

		status = qapi_TLMM_Get_Gpio_ID(&chk_pt_gpio_config, &qapi_GPIO_ID);
		if ( status != QAPI_OK)
			return;
		
		status = qapi_TLMM_Config_Gpio(qapi_GPIO_ID, &chk_pt_gpio_config);
		if ( status != QAPI_OK)
			return;
		
		chk_pt_gpio_status |= 0x04;
	}
	
	if ((chk_pt_gpio_status & 0x02) == 0)
	{
		status = qapi_TLMM_Drive_Gpio(qapi_GPIO_ID, chk_pt_gpio_pin, QAPI_GPIO_HIGH_VALUE_E);
		chk_pt_gpio_status |= 0x02;
	}
	else
	{
		status = qapi_TLMM_Drive_Gpio(qapi_GPIO_ID, chk_pt_gpio_pin, QAPI_GPIO_LOW_VALUE_E);
		chk_pt_gpio_status &= ~0x02;		
	}
}

void app_trigger_signal_release_GPIO ()
{
	chk_pt_gpio_config.pin = chk_pt_gpio_pin; 
    chk_pt_gpio_config.func = 0; 
    chk_pt_gpio_config.dir = QAPI_GPIO_INPUT_E; 
    chk_pt_gpio_config.pull = QAPI_GPIO_PULL_DOWN_E; 
    chk_pt_gpio_config.drive = QAPI_GPIO_2MA_E; 
         
    qapi_TLMM_Config_Gpio(qapi_GPIO_ID, &chk_pt_gpio_config); 

    qapi_TLMM_Release_Gpio_ID(&chk_pt_gpio_config, qapi_GPIO_ID);
}

/*-------------------------------------------------------------------------
 * Type Declarations
 *-----------------------------------------------------------------------*/

typedef struct PAL_Context_s
{
   qbool_t            Initialized;
   qapi_UART_Handle_t Console_UART;
   qbool_t            Uart_Enabled;
   char               Rx_Buffer[PAL_RECIEVE_BUFFER_COUNT][PAL_RECIEVE_BUFFER_SIZE];
   char               Rx_Buffer_Length[PAL_RECIEVE_BUFFER_COUNT];
   uint8_t            Rx_In_Index;
   uint8_t            Rx_Out_Index;
   volatile uint32_t  Rx_Buffers_Free;
   volatile uint32_t  BytesToTx;
   qurt_signal_t      Event;
} PAL_Context_t;


/*-------------------------------------------------------------------------
 * Static & global Variable Declarations
 *-----------------------------------------------------------------------*/

static PAL_Context_t PAL_Context;

/*-------------------------------------------------------------------------
 * Function Declarations
 *-----------------------------------------------------------------------*/

static void Initialize_Samples(void);
static void Uart_Tx_CB(uint32 num_bytes, void* cb_data);
static void Uart_Rx_CB(uint32 num_bytes, void* cb_data);
static void QCLI_Thread(void *Param);
static qbool_t PAL_Initialize(void);

/*-------------------------------------------------------------------------
 * Function Definitions
 *-----------------------------------------------------------------------*/

/**
   @brief This function is responsible for initializing the sample
          applications.
*/
static void Initialize_Samples(void)
{
#ifdef CONFIG_SPPLE_DEMO
   Initialize_SPPLE_Demo();
#endif
#ifdef CONFIG_HMI_DEMO
   Initialize_HMI_Demo();
#endif
#ifdef CONFIG_WIFI_DEMO
   Initialize_WIFI_Demo();

   /****
   when switching back to FOM, we will check the wlan status in SOM/MOM.
   if enabled, we will call enable_wlan. this enable_wlan will not initialize
   all the wlan driver. it just initialize demo data and restore wifi driver data.
   ****/
   extern int32_t enable_wlan();
   if(qapi_WLAN_Get_OM_Status())
   {
       enable_wlan();
   }
#endif
#ifdef CONFIG_NET_DEMO
   Initialize_Net_Demo();
#endif
#ifdef CONFIG_COEX_DEMO
   Initialize_Coex_Demo();
#endif
#ifdef CONFIG_FWUP_DEMO
   Initialize_FwUpgrade_Demo();
#endif
#ifdef CONFIG_ADSS_DEMO
   Initialize_ADSS_Demo();
#endif
#ifdef CONFIG_LP_DEMO
   Initialize_LP_Demo();
#endif
#ifdef CONFIG_FS_DEMO
   Initialize_Fs_Demo();
#endif
#ifdef CONFIG_ECOSYSTEM_DEMO
   Initialize_Ecosystem_Demo();
#endif
#ifdef CONFIG_KPI_DEMO
   Initialize_KPI_Demo();
#endif
#ifdef CONFIG_SECUREFS_DEMO
   Initialize_SecureFs_Demo();
#endif
#ifdef CONFIG_CRYPTO_DEMO
   Initialize_Crypto_Demo();
#endif
#ifdef CONFIG_ZIGBEE_DEMO
   Initialize_ZigBee_Demo();
#endif
#ifdef CONFIG_THREAD_DEMO
   Initialize_Thread_Demo();
#endif
#ifdef CONFIG_PERIPHERALS_DEMO
   Initialize_Peripherals_Demo();
#endif
#ifdef CONFIG_PLATFORM_DEMO
   Initialize_Platform_Demo();
#endif
#ifdef ENABLE_CPU_PROFILER
   Initialize_CpuProfiler_Demo();
#endif
#ifdef CONFIG_JSON_DEMO
   Initialize_JSON_Demo();
#endif
#ifdef CONFIG_QMESH_DEMO
   Initialize_QMesh_Demo();

#ifdef CONFIG_QMESH_COEX_DEMO
   Initialize_Qmesh_CoEx();
#endif

#endif
#ifdef CONFIG_FLASHLOG_DEMO
   Initialize_FLASHLOG_Demo();
#endif
}

/**
   @brief This function handles transmit callbacks from the UART.

   @param Num_Bytes[in] is the number of bytes transmitted.
   @param CB_Data[in]   is the application defined callback data.
*/
static void Uart_Tx_CB(uint32 Num_Bytes, void* CB_Data)
{
   if(PAL_Context.BytesToTx != 0)
   {
      PAL_Context.BytesToTx -= Num_Bytes;
      if(PAL_Context.BytesToTx == 0)
      {
         qurt_signal_set(&(PAL_Context.Event), PAL_EVENT_MASK_TRANSMIT);
      }
   }
}

/**
   @brief This function handles receive callbacks from the UART.

   @param Num_Bytes[in] is the number of bytes received.
   @param CB_Data[in]   is the application defined callback data.  In this case
                        it is the index of the buffer received on.
*/
static void Uart_Rx_CB(uint32 Num_Bytes, void* CB_Data)
{
   uint32_t Buffer_Index;

   Buffer_Index = (uint32_t)CB_Data;

   if(PAL_Context.Rx_Buffers_Free != 0)
   {
      /* See how much data can be read. */
      if(Num_Bytes > PAL_RECIEVE_BUFFER_SIZE)
      {
         Num_Bytes = PAL_RECIEVE_BUFFER_SIZE;
      }

      /* Note the amount of data that was read. */
      PAL_Context.Rx_Buffer_Length[Buffer_Index] = Num_Bytes;

      PAL_Context.Rx_In_Index ++;
      if(PAL_Context.Rx_In_Index == PAL_RECIEVE_BUFFER_COUNT)
      {
         PAL_Context.Rx_In_Index = 0;
      }

      PAL_ENTER_CRITICAL();
      PAL_Context.Rx_Buffers_Free --;
      PAL_EXIT_CRITICAL();

      /* Signal the event that data was received. */
      qurt_signal_set(&(PAL_Context.Event), PAL_EVENT_MASK_RECEIVE);
   }
}

/**
   @brief This function represents the main thread of execution.

   It will finish initialization of the sample and then function as a
   receive thread for the console.
*/
static void QCLI_Thread(void *Param)
{
   uint32_t CurrentIndex;


   /* Display the initialize command list. */
   QCLI_Display_Command_List();

   /* Loop waiting for received data. */
   while(true)
   {
      /* Wait for data to be received. */
      while(PAL_Context.Rx_Buffers_Free == PAL_RECIEVE_BUFFER_COUNT)
      {
         qurt_signal_wait(&(PAL_Context.Event), PAL_EVENT_MASK_RECEIVE, QURT_SIGNAL_ATTR_WAIT_ANY | QURT_SIGNAL_ATTR_CLEAR_MASK);
      }

      CurrentIndex = (uint32_t)(PAL_Context.Rx_Out_Index);

      /* Send the next buffer's data to QCLI for processing. */
      QCLI_Process_Input_Data(PAL_Context.Rx_Buffer_Length[CurrentIndex], PAL_Context.Rx_Buffer[CurrentIndex]);

      /* Adjust the indexes for the received data. */
      PAL_Context.Rx_Out_Index ++;
      if(PAL_Context.Rx_Out_Index == PAL_RECIEVE_BUFFER_COUNT)
      {
         PAL_Context.Rx_Out_Index = 0;
      }

      PAL_ENTER_CRITICAL();
      PAL_Context.Rx_Buffers_Free ++;
      PAL_EXIT_CRITICAL();

      /* Re-queue the buffer with the UART driver. */
      qapi_UART_Receive(PAL_Context.Console_UART, (char *)(PAL_Context.Rx_Buffer[CurrentIndex]), PAL_RECIEVE_BUFFER_SIZE, (void *)CurrentIndex);
   }
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
   uint8_t Ret_Val;

   memset(&PAL_Context, 0, sizeof(PAL_Context));
   PAL_Context.Rx_Buffers_Free = PAL_RECIEVE_BUFFER_COUNT;

   Ret_Val = PAL_Uart_Init();

   return(Ret_Val);
}

/**
   @brief Function call to initialize the application.
*/
void app_init(qbool_t ColdBoot)
{
   /* toggling GPIO for measuring boot time */
   app_trigger_signal_on_GPIO (7);
   
   /*Log the application entry time*/
   log_app_entry_time();

#ifdef ENABLE_DBGCALL
   dbgcall_setup();
#endif

   /* Initialize the platform. */
   if(PAL_Initialize())
   {
      /* Initiailze the CLI. */
      if(QCLI_Initialize())
      {
         /* Create a receive event. */
         qurt_signal_init(&(PAL_Context.Event));

         /* Initialize the samples. */
         Initialize_Samples();

         PAL_Context.Initialized = true;
      }
      else
      {
         PAL_CONSOLE_WRITE_STRING_LITERAL("QCLI initialization failed");
         PAL_CONSOLE_WRITE_STRING_LITERAL(PAL_OUTPUT_END_OF_LINE_STRING);
         PAL_CONSOLE_WRITE_STRING_LITERAL(PAL_OUTPUT_END_OF_LINE_STRING);
      }
   }
   app_trigger_signal_release_GPIO ();
}

/**
   @brief Main entry point of the application.
*/
void app_start(qbool_t ColdBoot)
{
   qurt_thread_attr_t Thread_Attribte;
   qurt_thread_t      Thread_Handle;
   int                Result;

   if(PAL_Context.Initialized)
   {
      /* Start the main demo thread. */
      qurt_thread_attr_init(&Thread_Attribte);
      qurt_thread_attr_set_name(&Thread_Attribte, "QCLI Thread");
      qurt_thread_attr_set_priority(&Thread_Attribte, PAL_THREAD_PRIORITY);
      qurt_thread_attr_set_stack_size(&Thread_Attribte, PAL_THREAD_STACK_SIZE);
      Result = qurt_thread_create(&Thread_Handle, &Thread_Attribte, QCLI_Thread, NULL);
      if(Result != QURT_EOK)
      {
         PAL_CONSOLE_WRITE_STRING_LITERAL("Failed to start QCLI thread.");
         PAL_CONSOLE_WRITE_STRING_LITERAL(PAL_OUTPUT_END_OF_LINE_STRING);
         PAL_CONSOLE_WRITE_STRING_LITERAL(PAL_OUTPUT_END_OF_LINE_STRING);
      }
   }
}

/**
   @brief Initialize the UART used by the demo.

   @return true if the UART was initailized successfully or false if there was
           an error.
*/
qbool_t PAL_Uart_Init(void)
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
   Uart_Config.tx_CB_ISR        = Uart_Tx_CB;
   Uart_Config.rx_CB_ISR        = Uart_Rx_CB;
   PAL_Context.Uart_Enabled     = true;

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
   @brief Turns off the UART used by the demo.

   @return true if the UART was deinitailized successfully or false if there was
           an error.
*/
qbool_t PAL_Uart_Deinit(void)
{
   PAL_Context.Uart_Enabled = false;
   return(qapi_UART_Close(PAL_Context.Console_UART));
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
   if((Length != 0) && (Buffer != NULL) && (PAL_Context.Uart_Enabled))
   {
      PAL_Context.BytesToTx = Length;

      /* Transmit the data. */
      if(qapi_UART_Transmit(PAL_Context.Console_UART, (char *)Buffer, Length, NULL) == QAPI_OK)
      {
         /* Wait for the packet to be sent. */
         qurt_signal_wait(&(PAL_Context.Event), PAL_EVENT_MASK_TRANSMIT, QURT_SIGNAL_ATTR_WAIT_ANY | QURT_SIGNAL_ATTR_CLEAR_MASK);
      }
   }
}

/**
   @brief This function indicates to the PAL layer that the application
          should exit.
*/
void PAL_Exit(void)
{
   PAL_CONSOLE_WRITE_STRING_LITERAL("Exiting...");
   PAL_CONSOLE_WRITE_STRING_LITERAL(PAL_OUTPUT_END_OF_LINE_STRING);

   /* Wait for the transmit buffers to flush.                           */
   qurt_thread_sleep(10);

   /* Exit the application.                                             */
//xxx
   PAL_CONSOLE_WRITE_STRING_LITERAL("Not Implemented.");
   PAL_CONSOLE_WRITE_STRING_LITERAL(PAL_OUTPUT_END_OF_LINE_STRING);
   PAL_CONSOLE_WRITE_STRING_LITERAL(PAL_OUTPUT_END_OF_LINE_STRING);
}

/**
   @brief This function indicates to the PAL layer that the application
          should reset. For embedded applications this is typically a reboot.
*/
void PAL_Reset(void)
{
   PAL_CONSOLE_WRITE_STRING_LITERAL("Resetting...");
   PAL_CONSOLE_WRITE_STRING_LITERAL(PAL_OUTPUT_END_OF_LINE_STRING);

   /* Wait for the transmit buffers to flush.                           */
   qurt_thread_sleep(10);

   /* Reset the platform.                                               */
   qapi_System_Reset();
}
