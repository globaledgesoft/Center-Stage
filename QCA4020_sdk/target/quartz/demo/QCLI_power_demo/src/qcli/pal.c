/*
 * Copyright (c) 2015-2017 Qualcomm Technologies, Inc.
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
#include "qurt_timer.h"

#include "qapi/qapi.h"
#include "qapi/qapi_status.h"
#include "qapi/qapi_uart.h"
#include "qapi/qapi_omtm.h"
#include "qapi/qapi_om_smem.h"
#include "qapi/qapi_mom.h"

#include "ble_demo.h"
#include "hmi_demo.h"
#include "coex_demo.h"

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
#define PAL_EVENT_MASK_SWITCH_OPERATING_MODE            (0x00000004)

#define PAL_THREAD_STACK_SIZE                           (3072)
#define PAL_THREAD_PRIORITY                             (10)

#define PAL_ENTER_CRITICAL()                            do { __asm("cpsid i"); } while(0)
#define PAL_EXIT_CRITICAL()                             do { __asm("cpsie i"); } while(0)

#define FOM_OPERATION_MODE                              (0)

#define PAL_MODE_TRANSITION_WAIT_MS                     (10)
#define PAL_MODE_TRANSITION_ATTEMPTS                    (100)

/* The following is a simple macro to facilitate printing strings directly to
   the console. As it uses the sizeof operator on the size of the string
   provided, it is intended to be used with string literals and will not work
   correctly with pointers. */
#define PAL_CONSOLE_WRITE_STRING_LITERAL(__String__)    do { PAL_Console_Write(sizeof(__String__) - 1, (__String__)); } while(0)

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
   uint32_t           Switch_Mode;
   qbool_t            In_Transition;
   uint32_t           Transition_Attempts;
} PAL_Context_t;

typedef struct Demo_Function_List_s
{
   Initialize_Function_t         Initialize;
   Initialize_Function_t         Start;
   Prepare_Mode_Swith_Function_t Prepare_Mode_Swith;
   Cancel_Mode_Switch_Function_t Cancel_Mode_Switch;
   Exit_Mode_Function_t          Exit_Mode;
} Demo_Function_List_t;

typedef void (*PlatformFunction_t)(void);

/*-------------------------------------------------------------------------
 * Static & global Variable Declarations
 *-----------------------------------------------------------------------*/

extern void qca_module_init(void);
PlatformFunction_t init_coldboot_functions[]   = {qca_module_init, NULL};
PlatformFunction_t deinit_coldboot_functions[] = {NULL};

static PAL_Context_t PAL_Context;

static const Demo_Function_List_t Demo_Function_List[] =
{
   /* Initialize,         Start,          Prepare_Mode_Swith,       Cancel_Mode_Switch,      Exit_Mode */
   {NULL,                 NULL,           QCLI_Prepare_Mode_Switch, QCLI_Cancel_Mode_Switch, QCLI_Exit_Mode},
   {Initialize_HMI_Demo,  Start_HMI_Demo, HMI_Prepare_Mode_Switch,  HMI_Cancel_Mode_Switch,  HMI_Exit_Mode},
   {Initialize_BLE_Demo,  NULL,           BLE_Prepare_Mode_Switch,  BLE_Cancel_Mode_Switch,  BLE_Exit_Mode},
   {Initialize_Coex_Demo, NULL,           Coex_Prepare_Mode_Switch, Coex_Cancel_Mode_Switch, Coex_Exit_Mode},
};

#define QCLI_DEMO_COUNT                                 (sizeof(Demo_Function_List) / sizeof(Demo_Function_List_t))

extern uint8_t  Reenable_UART;
extern uint32_t Wakeup_Seconds;

/*-------------------------------------------------------------------------
 * Function Declarations
 *-----------------------------------------------------------------------*/

static void Initialize_Samples(qbool_t ColdBoot);
static void Start_Samples(qbool_t ColdBoot);
static void Exit_Mode_CB(uint32_t dest_mode_id, void *user_data);
static void Uart_Tx_CB(uint32 num_bytes, void* cb_data);
static void Uart_Rx_CB(uint32 num_bytes, void* cb_data);
static void QCLI_Thread(void *Param);
static qbool_t PAL_Uart_Init(void);
static qbool_t PAL_Uart_Deinit(void);
static qbool_t PAL_Initialize(qbool_t ColdBoot);
static qbool_t Setup_SOM_Wake_Timer(void);
static qbool_t Store_UART_Reenable_Setting(void);

/*-------------------------------------------------------------------------
 * Function Definitions
 *-----------------------------------------------------------------------*/

/**
   @brief This function is responsible for initializing the sample
          applications.
*/
static void Initialize_Samples(qbool_t ColdBoot)
{
   uint32_t Index;

   for(Index = 0; Index < QCLI_DEMO_COUNT; Index ++)
   {
      if(Demo_Function_List[Index].Initialize != NULL)
      {
         (*(Demo_Function_List[Index].Initialize))(ColdBoot);
      }
   }

   if(!ColdBoot)
   {
      QCLI_Restore_State();
   }
}

/**
   @brief This function is responsible for initializing the sample
          applications.
*/
static void Start_Samples(qbool_t ColdBoot)
{
   uint32_t Index;

   for(Index = 0; Index < QCLI_DEMO_COUNT; Index ++)
   {
      if(Demo_Function_List[Index].Start != NULL)
      {
         (*(Demo_Function_List[Index].Start))(ColdBoot);
      }
   }

   if(!ColdBoot)
   {
      QCLI_Restore_State();
   }
}


/**
   @brief This function handles the mode exit callback from the OMTM.


   @param dest_mode_id identifies the mode we are going to switch to.
   @param user_data    is the user specified parameter for the callback.
*/
static void Exit_Mode_CB(uint32_t dest_mode_id, void *user_data)
{
   uint32_t Index;

   /* Disable the UART. */
   PAL_Uart_Deinit();

   /* Since there isn't a seperate prepare callback for v1 ROM, call it here. */
   for(Index = 0; Index < QCLI_DEMO_COUNT; Index ++)
   {
      if(Demo_Function_List[Index].Prepare_Mode_Swith != NULL)
      {
         (*(Demo_Function_List[Index].Prepare_Mode_Swith))((Operating_Mode_t)dest_mode_id);
      }
   }

   Store_UART_Reenable_Setting();

   if(Wakeup_Seconds > 0)
   {
      /* Set up our wakeup timers. */
      if(dest_mode_id == 1) /* SOM */
      {
         Setup_SOM_Wake_Timer();

      }
      else if(dest_mode_id == 2) /* MOM */
      {
         /* Convert from seconds to ticks. */
         qapi_MOM_Set_Wakeup_Timer(Wakeup_Seconds * 32768, 0);
      }
   }

   for(Index = 0; Index < QCLI_DEMO_COUNT; Index ++)
   {
      if(Demo_Function_List[Index].Exit_Mode != NULL)
      {
         (*(Demo_Function_List[Index].Exit_Mode))((Operating_Mode_t)dest_mode_id);
      }
   }
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
   uint32_t      CurrentIndex;
   qbool_t       ColdBoot;
   qapi_Status_t Result;

   ColdBoot = (qbool_t)Param;

   Start_Samples(ColdBoot);

   /* Display the initialize command list. */
   if(ColdBoot)
   {
      QCLI_Display_Command_List();
   }

   /* Loop waiting for received data. */
   while(true)
   {
      /* Wait for data to be received. */
      while((PAL_Context.Rx_Buffers_Free == PAL_RECIEVE_BUFFER_COUNT) && (PAL_Context.Switch_Mode == (uint32_t)OPERATING_MODE_FOM_E))
      {
         qurt_signal_wait(&(PAL_Context.Event), PAL_EVENT_MASK_RECEIVE | PAL_EVENT_MASK_SWITCH_OPERATING_MODE, QURT_SIGNAL_ATTR_WAIT_ANY | QURT_SIGNAL_ATTR_CLEAR_MASK);
      }

      if(PAL_Context.Switch_Mode != (uint32_t)OPERATING_MODE_FOM_E)
      {

         /* Switch operating modes. */
         Result = qapi_OMTM_Switch_Operating_Mode(PAL_Context.Switch_Mode, QAPI_OMTM_SWITCH_NOW_E);

         if(Result == QAPI_OK)
         {
            /* Set the switch mode back so that the thread can go idle again. */
            PAL_Context.Switch_Mode = (uint32_t)OPERATING_MODE_FOM_E;
         }
         else
         {
            /* Sleep the Thread for a short period then try again. */
            PAL_Context.Transition_Attempts ++;

            if(PAL_Context.Transition_Attempts >= PAL_MODE_TRANSITION_ATTEMPTS)
            {
               PAL_CONSOLE_WRITE_STRING_LITERAL("Mode transition failed.");
               PAL_CONSOLE_WRITE_STRING_LITERAL(PAL_OUTPUT_END_OF_LINE_STRING);
               PAL_CONSOLE_WRITE_STRING_LITERAL(PAL_OUTPUT_END_OF_LINE_STRING);

               PAL_Context.Switch_Mode = (uint32_t)OPERATING_MODE_FOM_E;
            }

            qurt_thread_sleep(qurt_timer_convert_time_to_ticks((qurt_time_t)PAL_MODE_TRANSITION_WAIT_MS, QURT_TIME_MSEC));
         }
      }
      else
      {
         CurrentIndex = (uint32_t)(PAL_Context.Rx_Out_Index);

         /* Send the next buffer's data to QCLI for processing only if not
            currently changing operating modes. This keeps new commands from
            being executed while in a mode change. */
         if(!PAL_Context.In_Transition)
         {
            QCLI_Process_Input_Data(PAL_Context.Rx_Buffer_Length[CurrentIndex], PAL_Context.Rx_Buffer[CurrentIndex]);
         }

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
}

/**
   @brief Initialize the UART used by the demo.

   @return true if the UART was initailized successfully or false if there was
           an error.
*/
static qbool_t PAL_Uart_Init(void)
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
static qbool_t PAL_Uart_Deinit(void)
{
   qbool_t RetVal;

   PAL_Context.Uart_Enabled = false;

   RetVal = qapi_UART_Close(PAL_Context.Console_UART);

   /* Signal any threads waiting on a Tx that it is done. */
   qurt_signal_set(&(PAL_Context.Event), PAL_EVENT_MASK_TRANSMIT);

   return(RetVal);
}

/**
   @brief This function is used to initialize the Platform, predominately
          the console port.

   @return
    - true if the platform was initialized successfully.
    - false if initialization failed.
*/
static qbool_t PAL_Initialize(qbool_t ColdBoot)
{
   qapi_Status_t  Result;
   uint8_t        Ret_Val;
   uint8_t       *Reenable_UART;
   uint16_t       Data_Size;

   memset(&PAL_Context, 0, sizeof(PAL_Context));
   PAL_Context.Rx_Buffers_Free = PAL_RECIEVE_BUFFER_COUNT;

   /* Retrieve the AON memory. */
   Result = qapi_OMSM_Retrieve(QAPI_OMSM_DEFAULT_AON_POOL, OMSM_CLIENT_ID_QCLI_E, &Data_Size, (void **)&Reenable_UART);

   if((ColdBoot) || ((Result == QAPI_OK) && (Data_Size == sizeof(uint8_t))))
   {
      /* Reenable the UART if required. */
      if((ColdBoot) || (!(ColdBoot) && (*Reenable_UART)))
         Ret_Val = PAL_Uart_Init();
      else
         Ret_Val = true;
   }
   else
      Ret_Val = true;

   /* Free the previously allocated AON memory. */
   if(Result == QAPI_OK)
      qapi_OMSM_Free(QAPI_OMSM_DEFAULT_AON_POOL, OMSM_CLIENT_ID_QCLI_E);

   return(Ret_Val);
}

/**
   @brief Function call to initialize the application.
*/
void app_init(qbool_t ColdBoot)
{
   int Result;

   /* Initialize the platform. */
   if(PAL_Initialize(ColdBoot))
   {
      PAL_Context.Switch_Mode = (uint32_t)OPERATING_MODE_FOM_E;

      /* Register the FOM operating modes. */
      Result = qapi_OMTM_Register_Operating_Modes(omtm_operating_mode_tbl_sram, 3, 0);

      if(Result == QAPI_OK)
      {
         /* Register the OMTM callbacks. */
         Result = qapi_OMTM_Register_Mode_Exit_Callback(Exit_Mode_CB, NULL, 0);

         if(Result == QAPI_OK)
         {
            /* Initiailze the CLI. */
            if(QCLI_Initialize())
            {
               /* Create a receive event. */
               qurt_signal_init(&(PAL_Context.Event));

               /* Initialize the samples. */
               Initialize_Samples(ColdBoot);

               PAL_Context.Initialized = true;
            }
            else
            {
               PAL_CONSOLE_WRITE_STRING_LITERAL("QCLI initialization failed");
               PAL_CONSOLE_WRITE_STRING_LITERAL(PAL_OUTPUT_END_OF_LINE_STRING);
               PAL_CONSOLE_WRITE_STRING_LITERAL(PAL_OUTPUT_END_OF_LINE_STRING);
            }
         }
         else
         {
            PAL_CONSOLE_WRITE_STRING_LITERAL("Failed to register mode change callbacks.");
            PAL_CONSOLE_WRITE_STRING_LITERAL(PAL_OUTPUT_END_OF_LINE_STRING);
         }
      }
      else
      {
         PAL_CONSOLE_WRITE_STRING_LITERAL("Failed to register operating modes.");
         PAL_CONSOLE_WRITE_STRING_LITERAL(PAL_OUTPUT_END_OF_LINE_STRING);
      }
   }
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
      Result = qurt_thread_create(&Thread_Handle, &Thread_Attribte, QCLI_Thread, (void *)ColdBoot);
      if(Result != QURT_EOK)
      {
         PAL_CONSOLE_WRITE_STRING_LITERAL("Failed to start QCLI thread.");
         PAL_CONSOLE_WRITE_STRING_LITERAL(PAL_OUTPUT_END_OF_LINE_STRING);
         PAL_CONSOLE_WRITE_STRING_LITERAL(PAL_OUTPUT_END_OF_LINE_STRING);
      }
   }
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
   @brief This function disables the UART for a specified period of time.

   @param Time is the time to disable the UART in milliseconds. Specify
          0xFFFFFFFF to disable indefinitely.
*/
void PAL_DisableUART(uint32_t Time)
{
   qurt_time_t Ticks;

   PAL_CONSOLE_WRITE_STRING_LITERAL("Disabling UART...");
   PAL_CONSOLE_WRITE_STRING_LITERAL(PAL_OUTPUT_END_OF_LINE_STRING);

   /* Wait for the transmit buffers to flush. */
   while(PAL_Context.BytesToTx != 0)
   {
      qurt_thread_sleep(1);
   }

   /* Turn off the UART. */
   PAL_Uart_Deinit();

   if(Time == 0xFFFFFFFF)
   {
      Ticks = QURT_TIME_WAIT_FOREVER;
   }
   else
   {
      Ticks = qurt_timer_convert_time_to_ticks(Time, QURT_TIME_MSEC);
   }

   /* Block for the specified amount of time. */
   qurt_thread_sleep(Ticks);

   /* Reenable the UART. */
   PAL_Uart_Init();
}

/**
   @brief This function tells the PAL layer to change the current operating
          mode.

   @param Operating_Mode is the mode to switch to.
*/
void PAL_Switch_Operating_Mode(Operating_Mode_t Operating_Mode)
{
   PAL_Context.Transition_Attempts = 0;
   PAL_Context.Switch_Mode         = (uint32_t)Operating_Mode;
   qurt_signal_set(&(PAL_Context.Event), PAL_EVENT_MASK_SWITCH_OPERATING_MODE);
}

static qbool_t Setup_SOM_Wake_Timer(void)
{
   qbool_t   Ret_Val;
   uint32_t *Transition_Secs;

   Ret_Val = true;

   if(Wakeup_Seconds > 0)
   {
      if((qapi_OMSM_Alloc(QAPI_OMSM_DEFAULT_AON_POOL, OMSM_CLIENT_ID_PAL_E, sizeof(uint32_t), (void **)(&Transition_Secs)) == QAPI_OK) && (Transition_Secs != NULL))
      {
         *Transition_Secs = Wakeup_Seconds;

         if(qapi_OMSM_Commit(QAPI_OMSM_DEFAULT_AON_POOL, OMSM_CLIENT_ID_PAL_E) != QAPI_OK)
         {
            qapi_OMSM_Free(QAPI_OMSM_DEFAULT_AON_POOL, OMSM_CLIENT_ID_PAL_E);

            /* Cancel the transition if we cannot wake up. */
            Ret_Val = false;
         }
      }
      else
      {
         Ret_Val = false;
      }
   }

   return Ret_Val;
}

static qbool_t Store_UART_Reenable_Setting(void)
{
   qbool_t  Ret_Val;
   uint8_t *UART_Reenable;

   Ret_Val = true;

   if((qapi_OMSM_Alloc(QAPI_OMSM_DEFAULT_AON_POOL, OMSM_CLIENT_ID_QCLI_E, sizeof(uint8_t), (void **)(&UART_Reenable)) == QAPI_OK) && (UART_Reenable != NULL))
   {
      *UART_Reenable = Reenable_UART;

      if(qapi_OMSM_Commit(QAPI_OMSM_DEFAULT_AON_POOL, OMSM_CLIENT_ID_QCLI_E) != QAPI_OK)
      {
         qapi_OMSM_Free(QAPI_OMSM_DEFAULT_AON_POOL, OMSM_CLIENT_ID_QCLI_E);
         Ret_Val = false;
      }
   }
   else
   {
      Ret_Val = false;
   }

   return Ret_Val;
}
