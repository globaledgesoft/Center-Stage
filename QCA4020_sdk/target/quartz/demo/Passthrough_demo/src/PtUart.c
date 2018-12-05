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

#include <stdint.h>
#include "PtUart.h"
#include "QPKTZR.h"

#include "qurt_error.h"
#include "qurt_thread.h"
#include "qurt_signal.h"
#include "qurt_mutex.h"
#include "qurt_timer.h"

   /* PT_UART Buffer and Flow Control Configuration constants.             */
   /* * NOTE * The underlying driver only supports ONE pending Rx or Tx,*/
   /*          so do not change RX_MAX_PENDING or TX_MAX_PENDING unless */
   /*          you use this implementation with a different driver.     */
#define RX_BUFFER_WORD_SIZE               (64)
#define RX_BUFFER_COUNT                   (16)
#define RX_MAX_PENDING                    (2)
#define TX_BUFFER_WORD_SIZE               (64)
#define TX_BUFFER_COUNT                   (16)
#define TX_MAX_PENDING                    (1)
#define PROCESS_DATA_THREAD_STACK_SIZE    (4098)
#define PROCESS_DATA_THREAD_STACK_PRIO    (6)
#define CALLBACK_THREAD_STACK_SIZE        (4098)
#define CALLBACK_THREAD_STACK_PRIO        (10)

   /* The following define how interrupts are enabled and disabled. */
#define DisableInterrupts()               do { __asm("cpsid i"); } while(0)
#define EnableInterrupts()                do { __asm("cpsie i"); } while(0)

#define PT_UART_EVENT_TERMINATE_PROCESS_THREAD  (0x000001)
#define PT_UART_EVENT_TERMINATE_CALLBACK_THREAD (0x000002)
#define PT_UART_EVENT_PROCESS_DATA              (0x000004)
#define PT_UART_EVENT_CALLBACK                  (0x000008)
#define PT_UART_EVENT_WAKE                      (0x000010)

#define PT_UART_COMMAND_ID_SLEEP_UART     (0x01)

#define PT_UART_EVENT_ID_WAKE_UART        (0x01)

#define PT_UART_WAKE_REASON_TIMEOUT       (0x00)
#define PT_UART_WAKE_REASON_DATA_PENDING  (0x01)
#define PT_UART_WAKE_REASON_SLEEP_FAILED  (0x02)

   /* The following are the defined bits for the local module flags. */
#define PT_UART_FLAGS_TRANSPORT_OPEN      (0x01)
#define PT_UART_FLAGS_THREAD_EXIT         (0x02)
#define PT_UART_FLAGS_PD_THREAD_EXITED    (0x04)
#define PT_UART_FLAGS_CB_THREAD_EXITED    (0x08)
#define PT_UART_FLAGS_USE_HS_UART         (0x10)
#define PT_UART_FLAGS_USE_HW_FLOW         (0x20)
#define PT_UART_FLAGS_TRANSPORT_ASLEEP    (0x40)
#define PT_UART_FLAGS_TIMER_ACTIVE        (0x80)

typedef struct _tagPtUartContext_t
{
   /* Local flags for the module. */
   volatile uint32_t    Flags;

   /* Local module resources. */
   qurt_signal_t        Event;
   qurt_mutex_t         ApiMutex;
   uart_handle          UartHandle;

   /* Pt Uart Callback Function and Callback Parameter information.        */
   PtUart_Callback_t    PtUartCallback;
   unsigned long        PtUartCallbackParameter;

   /* Pt Uart buffer descriptors. */
   volatile int         RxBuffersFree;
   volatile int         RxInIndex;
   int                  RxOutIndex;
   volatile uint32_t    RxPendCount;
   volatile int         TxBuffersFree;
   int                  TxInIndex;
   int                  TxOutIndex;
   volatile uint32_t    TxPendCount;

   /* Pt Uart buffers and counts. */
   uint32_t RxBuffers[RX_BUFFER_COUNT][RX_BUFFER_WORD_SIZE];
   uint32_t RxBufferFill[RX_BUFFER_COUNT];
   uint32_t TxBuffers[TX_BUFFER_COUNT][TX_BUFFER_WORD_SIZE];
   uint32_t TxBufferFill[TX_BUFFER_COUNT];

   unsigned int         BaudRate;
   qurt_timer_t         Timer;
} PtUartContext_t;

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the         */
   /* compiler as part of standard C/C++).                              */

static PtUartContext_t  PtUartContext;

unsigned char UARTHistoryBuffer[2048];
unsigned int UARTHistoryIndex;

   /* Local Function Prototypes.                                        */
static void CallbackThread(void *Param);
static void ProcessDataThread(void *Param);
static void PtUart_TxHandler(uint32 num_bytes, void *cb_data);
static void PtUart_RxHandler(uint32 num_bytes, void *cb_data);
static void PtUart_TimerHandle(void *Param);
static int AllocateLocalResources(void);
static void FreeLocalResources(void);
static void SendWake(uint8_t Reason);
static uart_result EnableUART(void);
static void DisableUART(void);

   /* External Function Prototypes.                                     */
extern void SendQPacket(QPKTZR_Packet_t *QPacket);

static void CallbackThread(void *Param)
{
   unsigned int NoBlock = 1;
   uint32       CurrentSigs;

   /* This thread will loop forever.                                    */
   while(TRUE)
   {
      /* Check to see if there are any characters in the receive buffer.*/
      if(!NoBlock)
      {
         /* Wait for a Received Character event;                        */
         while(qurt_signal_wait_timed(&(PtUartContext.Event), PT_UART_EVENT_CALLBACK | PT_UART_EVENT_TERMINATE_CALLBACK_THREAD, QURT_SIGNAL_ATTR_CLEAR_MASK | QURT_SIGNAL_ATTR_WAIT_ANY, &CurrentSigs, QURT_TIME_WAIT_FOREVER) != QURT_EOK) {}
      }
      else
         NoBlock = 0;

      /* Check for the exit condition. */
      if(PtUartContext.Flags & PT_UART_FLAGS_THREAD_EXIT)
         break;

      /* If a buffer is not pending and is not free, then send it to the callback. */
      DisableInterrupts();
      if((PtUartContext.RxBuffersFree + PtUartContext.RxPendCount) != RX_BUFFER_COUNT)
      {
         EnableInterrupts();

         /* Go ahead and log any data received.                         */
         if(PtUartContext.RxBufferFill[PtUartContext.RxInIndex] <= (sizeof(UARTHistoryBuffer) - UARTHistoryIndex))
            memcpy(&(UARTHistoryBuffer[UARTHistoryIndex]), PtUartContext.RxBuffers[PtUartContext.RxInIndex], PtUartContext.RxBufferFill[PtUartContext.RxInIndex]);
         else
         {
            memcpy(&(UARTHistoryBuffer[UARTHistoryIndex]), PtUartContext.RxBuffers[PtUartContext.RxInIndex], (sizeof(UARTHistoryBuffer) - UARTHistoryIndex));

            memcpy(UARTHistoryBuffer, &(PtUartContext.RxBuffers[PtUartContext.RxInIndex][(sizeof(UARTHistoryBuffer) - UARTHistoryIndex)]), PtUartContext.RxBufferFill[PtUartContext.RxInIndex] - (sizeof(UARTHistoryBuffer) - UARTHistoryIndex));
         }

         UARTHistoryIndex = (UARTHistoryIndex + PtUartContext.RxBufferFill[PtUartContext.RxInIndex]) % sizeof(UARTHistoryBuffer);

         if(PtUartContext.PtUartCallback)
            (*PtUartContext.PtUartCallback)(PtUartContext.RxBufferFill[PtUartContext.RxInIndex], (unsigned char *)PtUartContext.RxBuffers[PtUartContext.RxInIndex], PtUartContext.PtUartCallbackParameter);

         /* Indicate that the buffer at the In index is now free. */
         DisableInterrupts();

         PtUartContext.RxBuffersFree += 1;
         PtUartContext.RxInIndex     += 1;

         /* Check for wrap. */
         if(PtUartContext.RxInIndex == RX_BUFFER_COUNT)
            PtUartContext.RxInIndex = 0;

         EnableInterrupts();

         /* A buffer has been freed, so trigger the process data thread to check for resubmission. */
         qurt_signal_set(&(PtUartContext.Event), PT_UART_EVENT_PROCESS_DATA);

         /* Indicate not to block in order to check if another callback needs to be processed. */
         NoBlock  = 1;
      }
      else
         EnableInterrupts();
   }

   DisableInterrupts();
   PtUartContext.Flags |= PT_UART_FLAGS_CB_THREAD_EXITED;
   EnableInterrupts();

   /* Call into QuRT to free the memory associated with the task and */
   /* schedule another task to run.                                  */
   qurt_thread_stop();
}

static void ProcessDataThread(void *Param)
{
   unsigned int Temp;
   unsigned int NoBlock = 1;
   uint32       CurrentSigs;

   /* This thread will loop forever.                                    */
   while(TRUE)
   {
      /* Check to see if there is something to process.*/
      if(!NoBlock)
      {
         /* Wait for data to process;                                   */
         while(qurt_signal_wait_timed(&(PtUartContext.Event), PT_UART_EVENT_PROCESS_DATA | PT_UART_EVENT_WAKE | PT_UART_EVENT_TERMINATE_PROCESS_THREAD, QURT_SIGNAL_ATTR_CLEAR_MASK | QURT_SIGNAL_ATTR_WAIT_ANY, &CurrentSigs, QURT_TIME_WAIT_FOREVER) != QURT_EOK) {}
      }
      else
         NoBlock = 0;

      /* Check for the exit condition. */
      if(PtUartContext.Flags & PT_UART_FLAGS_THREAD_EXIT)
         break;

      if(CurrentSigs & PT_UART_EVENT_WAKE)
      {
         CurrentSigs &= ~PT_UART_EVENT_WAKE;

         /* Wake the UART.                                              */
         if(qurt_mutex_lock_timed(&(PtUartContext.ApiMutex), QURT_TIME_WAIT_FOREVER) == QURT_EOK)
         {
            if(PtUartContext.Flags & PT_UART_FLAGS_TIMER_ACTIVE)
            {
               PtUartContext.Flags &= ~PT_UART_FLAGS_TIMER_ACTIVE;
               qurt_timer_delete(PtUartContext.Timer);
            }

            if(PtUartContext.Flags & PT_UART_FLAGS_TRANSPORT_ASLEEP)
            {
               if(EnableUART() == UART_SUCCESS)
                  SendWake(PT_UART_WAKE_REASON_TIMEOUT);
            }

            qurt_mutex_unlock(&(PtUartContext.ApiMutex));
         }
      }

      if(!(PtUartContext.Flags & PT_UART_FLAGS_TRANSPORT_ASLEEP))
      {
         /* If all buffers are not either pending or free, then an      */
         /* occupied buffer needs to be submitted to the driver if the  */
         /* max pending threshold will not be exceeded.                 */
         DisableInterrupts();
         Temp = PtUartContext.TxPendCount;
         if(((PtUartContext.TxBuffersFree + Temp) != TX_BUFFER_COUNT) && (Temp < TX_MAX_PENDING))
         {
            EnableInterrupts();

            /* Transmit the buffer at the current out index. */
            uart_transmit(PtUartContext.UartHandle, (char *)PtUartContext.TxBuffers[PtUartContext.TxOutIndex], PtUartContext.TxBufferFill[PtUartContext.TxOutIndex], (void *)0);

            /* Increase the out index by one to indicate that the buffer*/
            /* has been submitted.                                      */
            PtUartContext.TxOutIndex += 1;
            if(PtUartContext.TxOutIndex == TX_BUFFER_COUNT)
               PtUartContext.TxOutIndex = 0;

            /* Increment the pending submission count. */
            DisableInterrupts();
            PtUartContext.TxPendCount += 1;
            EnableInterrupts();

            /* Indicate not to block in order to check if another Tx    */
            /* needs to be processed.                                   */
            NoBlock  = 1;
         }
         else
            EnableInterrupts();

         /* If there is a free buffer and the pend count will not be    */
         /* exceeded, then submit the buffer for receive.               */
         DisableInterrupts();
         if((PtUartContext.RxBuffersFree != 0) && (PtUartContext.RxPendCount < RX_MAX_PENDING))
         {
            EnableInterrupts();

            /* Submit the receive buffer at the current out index.      */
            if(uart_receive(PtUartContext.UartHandle, (char *)PtUartContext.RxBuffers[PtUartContext.RxOutIndex], RX_BUFFER_WORD_SIZE*4, (void *)PtUartContext.RxOutIndex) != UART_SUCCESS)
            {
               while(1);
            }

            /* If no receive buffers were pending in the driver before  */
            /* the currently submitted buffer, then trigger the callback*/
            /* thread in case it needs to dispatch. This is to handle   */
            /* special condition in which the process data thread runs  */
            /* first to keep the driver saturated with buffers.         */
            if(!(PtUartContext.RxPendCount))
               qurt_signal_set(&(PtUartContext.Event), PT_UART_EVENT_CALLBACK);

            /* Increase the out index by one to indicate that the buffer*/
            /* has been submitted.                                      */
            PtUartContext.RxOutIndex += 1;
            if(PtUartContext.RxOutIndex == RX_BUFFER_COUNT)
               PtUartContext.RxOutIndex = 0;

            /* Increment the pending submission count and decrease the  */
            /* free count.                                              */
            DisableInterrupts();
            PtUartContext.RxPendCount   += 1;
            PtUartContext.RxBuffersFree -= 1;
            EnableInterrupts();

            /* Indicate not to block in order to check if another Rx    */
            /* needs to be processed.                                   */
            NoBlock  = 1;
         }
         else
            EnableInterrupts();
      }
   }

   DisableInterrupts();
   PtUartContext.Flags |= PT_UART_FLAGS_PD_THREAD_EXITED;
   EnableInterrupts();

   /* Call into QuRT to free the memory associated with the task and    */
   /* schedule another task to run.                                     */
   qurt_thread_stop();
}

static void PtUart_TxHandler(uint32 num_bytes, void *cb_data)
{
   /* Decrement the pend count and increment the free buffers since the driver has relinquished the buffer and it can be used again. */
   PtUartContext.TxPendCount   -= 1;
   PtUartContext.TxBuffersFree += 1;

   /* The transmit buffer has been sent, so signal the process data thread to so that it can be used to send another buffer if necessary. */
   qurt_signal_set(&(PtUartContext.Event), PT_UART_EVENT_PROCESS_DATA);
}

static void PtUart_RxHandler(uint32 num_bytes, void *cb_data)
{
   unsigned int CurrentBufferIndex;

   /* Current buffer index can be determined by the in index and the    */
   /* number of buffers that are waiting to be freed or submitted.      */
   CurrentBufferIndex = (uint32)cb_data;

   /* Increment the pend count since the driver has relinquished the buffer, but do not indicate that it is free until after it has been dispatched. Note the number of bytes received in the buffer. */
   PtUartContext.RxPendCount                      -= 1;
   PtUartContext.RxBufferFill[CurrentBufferIndex]  = num_bytes;

   /* The thread that gets triggered depends on the urgency of          */
   /* resubmitting a receive buffer.  If there is a receive bufffer that*/
   /* is free and there are no pending receive buffers in the driver,   */
   /* then the process data thread should run first in order to         */
   /* re-submit a buffer as soon as possible.  Otherwise, received bytes*/
   /* could be dropped.  In this case, the process data thread will then*/
   /* be responsible for triggering the callback thread after submitting*/
   /* the recieve buffer.  If the driver still has a pending buffer,    */
   /* then run the callback thread first and allow it to trigger the    */
   /* process data thread.                                              */
   qurt_signal_set(&(PtUartContext.Event), ((PtUartContext.RxBuffersFree) && !(PtUartContext.RxPendCount)) ? PT_UART_EVENT_PROCESS_DATA : PT_UART_EVENT_CALLBACK);
}

   /* * NOTE * PtUartContext should be zero initialized before*/
static int AllocateLocalResources(void)
{
   qurt_thread_attr_t ThreadAttributes;
   qurt_thread_t      ThreadID;
   unsigned int       Flags;
   unsigned int       i;
   int                Result;
   int                ret_val;
   qurt_thread_t      ThreadHandle;

   ret_val = PT_UART_ERROR_UNABLE_TO_OPEN_TRANSPORT;

   /* Initialize the signals.                                         */
   qurt_signal_init(&(PtUartContext.Event));

   /* Initialize the mutex.                                         */
   qurt_mutex_init(&PtUartContext.ApiMutex);

   /* Initialize the thread attributes.                           */
   qurt_thread_attr_init(&ThreadAttributes);
   qurt_thread_attr_set_name(&ThreadAttributes, "P");
   qurt_thread_attr_set_priority(&ThreadAttributes, PROCESS_DATA_THREAD_STACK_PRIO);
   qurt_thread_attr_set_stack_size(&ThreadAttributes, PROCESS_DATA_THREAD_STACK_SIZE);

   /* Attempt to create the thread via the QuRT API.              */
   Result = qurt_thread_create(&ThreadHandle, &ThreadAttributes, ProcessDataThread, NULL);
   if(Result == QURT_EOK)
   {
      /* Initialize the thread attributes.                           */
      qurt_thread_attr_init(&ThreadAttributes);
      qurt_thread_attr_set_name(&ThreadAttributes, "C");
      qurt_thread_attr_set_priority(&ThreadAttributes, CALLBACK_THREAD_STACK_PRIO);
      qurt_thread_attr_set_stack_size(&ThreadAttributes, CALLBACK_THREAD_STACK_SIZE);

      /* Attempt to create the thread via the QuRT API.              */
      Result = qurt_thread_create(&ThreadHandle, &ThreadAttributes, CallbackThread, NULL);
      if(Result == QURT_EOK)
      {
         ret_val = 0;
      }
      else
      {
         qurt_signal_set(&(PtUartContext.Event), PT_UART_EVENT_PROCESS_DATA);

         /* Allow time for shutdown.                                       */
         for(i=0; i<10; i++)
         {
            Flags = PtUartContext.Flags;
            if(Flags & PT_UART_FLAGS_PD_THREAD_EXITED)
               break;

            qurt_thread_sleep(qurt_timer_convert_time_to_ticks(50, QURT_TIME_MSEC));
         }

         qurt_signal_destroy(&(PtUartContext.Event));
         qurt_mutex_destroy(&(PtUartContext.ApiMutex));
      }
   }
   else
   {
      qurt_signal_destroy(&(PtUartContext.Event));
      qurt_mutex_destroy(&(PtUartContext.ApiMutex));
   }

   return(ret_val);
}

static void FreeLocalResources(void)
{
   unsigned int Flags;
   unsigned int i;

   /* Request to shut down the Rx thread.                            */
   DisableInterrupts();
   PtUartContext.Flags |= PT_UART_FLAGS_THREAD_EXIT;
   EnableInterrupts();

   qurt_signal_set(&(PtUartContext.Event), PT_UART_EVENT_TERMINATE_PROCESS_THREAD | PT_UART_EVENT_TERMINATE_CALLBACK_THREAD);

   /* Allow time for shutdown.                                       */
   for(i=0; i<10; i++)
   {
      Flags = PtUartContext.Flags;
      if((Flags & PT_UART_FLAGS_PD_THREAD_EXITED) && (Flags & PT_UART_FLAGS_CB_THREAD_EXITED))
         break;

      qurt_thread_sleep(qurt_timer_convert_time_to_ticks(50, QURT_TIME_MSEC));
   }

   qurt_signal_destroy(&(PtUartContext.Event));

   qurt_mutex_destroy(&(PtUartContext.ApiMutex));
}

static void SendWake(uint8_t Reason)
{
   QPKTZR_Packet_t WakePacket;
   uint8_t         WakePayload[2];

   /* Send the UART wake event.                                      */
   WakePacket.PacketType   = QPKTZR_PACKET_TYPE_UART;
   WakePacket.PacketLength = sizeof(WakePayload);
   WakePacket.PacketData   = WakePayload;
   WakePayload[0]          = PT_UART_EVENT_ID_WAKE_UART;
   WakePayload[1]          = Reason;

   SendQPacket(&WakePacket);
}

static uart_result EnableUART(void)
{
   uart_result      ret_val;
   uart_open_config UartDriverConfig;
   uart_port_id     PortId;

   PtUartContext.RxBuffersFree = RX_BUFFER_COUNT;
   PtUartContext.TxBuffersFree = TX_BUFFER_COUNT;
   PtUartContext.RxPendCount   = 0;
   PtUartContext.RxInIndex     = 0;
   PtUartContext.RxOutIndex    = 0;
   PtUartContext.TxInIndex     = 0;
   PtUartContext.TxOutIndex    = 0;
   PtUartContext.TxPendCount   = 0;

   memset(&UartDriverConfig, 0, sizeof(UartDriverConfig));

   UartDriverConfig.baud_rate        = PtUartContext.BaudRate;
   UartDriverConfig.parity_mode      = UART_NO_PARITY;
   UartDriverConfig.num_stop_bits    = UART_1_0_STOP_BITS;
   UartDriverConfig.bits_per_char    = UART_8_BITS_PER_CHAR;
   UartDriverConfig.tx_cb_isr        = PtUart_TxHandler;
   UartDriverConfig.rx_cb_isr        = PtUart_RxHandler;

   if(PtUartContext.Flags & PT_UART_FLAGS_USE_HW_FLOW)
      UartDriverConfig.enable_flow_ctrl = 1;

   if(PtUartContext.Flags & PT_UART_FLAGS_USE_HS_UART)
      PortId = UART_HS_PORT;
   else
      PortId = UART_DEBUG_PORT;

   ret_val = uart_open(&(PtUartContext.UartHandle), PortId, &UartDriverConfig);
   PtUartContext.Flags &= ~PT_UART_FLAGS_TRANSPORT_ASLEEP;

   return(ret_val);
}

static void DisableUART(void)
{
   PtUartContext.Flags |= PT_UART_FLAGS_TRANSPORT_ASLEEP;

   uart_close(PtUartContext.UartHandle);
}

int PtUart_Initialize(PtUart_Initialization_Data_t *InitializationData, PtUart_Callback_t PtUartCallback, unsigned long CallbackParameter)
{
   uart_result Result;
   int         ret_val;

   /* First, make sure that the port is not already open and make sure  */
   /* that valid Initialization Data was specified.                     */
   if(!(PtUartContext.Flags & PT_UART_FLAGS_TRANSPORT_OPEN) && (InitializationData) && (PtUartCallback))
   {
      /* Initialize the return value for success.                       */
      ret_val = 0;

      /* Initialize the PT_UART Context Structure.                         */
      memset(&PtUartContext, 0, sizeof(PtUartContext_t));

      /* Note the Callback information.                                 */
      PtUartContext.PtUartCallback          = PtUartCallback;
      PtUartContext.PtUartCallbackParameter = CallbackParameter;
      PtUartContext.BaudRate                = InitializationData->BaudRate;
      if(InitializationData->Flags & PT_UART_INITIALIZATION_DATA_FLAG_USE_HW_FLOW)
         PtUartContext.Flags |= PT_UART_FLAGS_USE_HW_FLOW;

      if(InitializationData->Flags & PT_UART_INITIALIZATION_DATA_FLAG_USE_HS_UART)
         PtUartContext.Flags |= PT_UART_FLAGS_USE_HS_UART;

      /* For debugging purposes we will log any data received from the  */
      /* UART.                                                          */
      memset(UARTHistoryBuffer, 0, sizeof(UARTHistoryBuffer));
      UARTHistoryIndex = 0;

      Result = EnableUART();
      if(Result == UART_SUCCESS)
      {
         if((ret_val = AllocateLocalResources()) != 0)
            uart_close(PtUartContext.UartHandle);
      }
      else
         ret_val = PT_UART_ERROR_UNABLE_TO_OPEN_TRANSPORT;

      /* If there was no error, then continue to setup the port.        */
      if(!ret_val)
      {
         /* Flag that the HCI Transport is open.                        */
         PtUartContext.Flags |= PT_UART_FLAGS_TRANSPORT_OPEN;
      }
   }
   else
      ret_val = PT_UART_ERROR_UNABLE_TO_OPEN_TRANSPORT;

   return(ret_val);
}

void PtUart_UnInitialize(void)
{
   PtUart_Callback_t PtUartCallback;
   unsigned long     CallbackParameter;

   /* Check to make sure that the specified Transport ID is valid.      */
   if(PtUartContext.Flags & PT_UART_FLAGS_TRANSPORT_OPEN)
   {
      FreeLocalResources();

      DisableUART();

      /* Note the Callback information.                                 */
      PtUartCallback    = PtUartContext.PtUartCallback;
      CallbackParameter = PtUartContext.PtUartCallbackParameter;

      /* Flag that the HCI Transport is no longer open.                 */
      PtUartContext.Flags &= ~PT_UART_FLAGS_TRANSPORT_OPEN;

      /* Flag that there is no callback information present.            */
      PtUartContext.PtUartCallback          = NULL;
      PtUartContext.PtUartCallbackParameter = 0;

      /* All finished, perform the callback to let the upper layer know */
      /* that this module will no longer issue data callbacks and is    */
      /* completely cleaned up.                                         */
      if(PtUartCallback)
         (*PtUartCallback)(0, NULL, CallbackParameter);
   }
}

int PtUart_Write(unsigned int Length, unsigned char *Buffer)
{
   unsigned int Count;
   int          ret_val;

   /* Check to make sure that the specified Transport ID is valid and   */
   /* the output buffer appears to be valid as well.                    */
   if((PtUartContext.Flags & PT_UART_FLAGS_TRANSPORT_OPEN) && (!(PtUartContext.Flags & PT_UART_FLAGS_TRANSPORT_ASLEEP)) && (Length) && (Buffer))
   {
      if(qurt_mutex_lock_timed(&(PtUartContext.ApiMutex), QURT_TIME_WAIT_FOREVER) == QURT_EOK)
      {
         /* Process all of the data.                                       */
         while((Length) && !(PtUartContext.Flags & PT_UART_FLAGS_THREAD_EXIT))
         {
            /* Check to see if any buffers are free to copy the data into. */
            if(PtUartContext.TxBuffersFree)
            {
               Count = TX_BUFFER_WORD_SIZE*4;

               if(Count > Length)
                  Count = Length;

               /* Copy the data into the Tx buffer and note how much was copied.                        */
               memcpy((void *)PtUartContext.TxBuffers[PtUartContext.TxInIndex], Buffer, Count);
               PtUartContext.TxBufferFill[PtUartContext.TxInIndex] = Count;

               /* Adjust the count and index values.                       */
               Buffer                += Count;
               Length                -= Count;

               PtUartContext.TxInIndex += 1;
               if(PtUartContext.TxInIndex >= TX_BUFFER_COUNT)
                  PtUartContext.TxInIndex = 0;

               /* Disable context switching while updating the number of free buffers. */
               DisableInterrupts();
               PtUartContext.TxBuffersFree -= 1;
               EnableInterrupts();

               /* Indicate to the processing thread that a buffer has been filled for transmit. */
               qurt_signal_set(&(PtUartContext.Event), PT_UART_EVENT_PROCESS_DATA);
            }
            else
            {
               qurt_thread_sleep(qurt_timer_convert_time_to_ticks(10, QURT_TIME_MSEC));
            }
         }
         ret_val = 0;

         qurt_mutex_unlock(&(PtUartContext.ApiMutex));
      }
      else
         ret_val = PT_UART_ERROR_WRITING_TO_PORT;
   }
   else
      ret_val = PT_UART_ERROR_WRITING_TO_PORT;

   return(ret_val);
}

   /* Processes command packets for the UART interface.                 */
void PtUart_Process_Packet(unsigned int Length, const uint8_t *Data)
{
   qurt_timer_attr_t TimerAttr;
   unsigned int      SleepTime;
   int               Success;

   /* Check to make sure that the specified Transport ID is valid and   */
   /* the output buffer appears to be valid as well.                    */
   if(PtUartContext.Flags & PT_UART_FLAGS_TRANSPORT_OPEN)
   {
      /* Currently only the sleep command is supported.  The packet     */
      /* contains one byte for the command, 4 bytes of flags (currently */
      /* unused) and 4 bytes for the time.                              */
      if((Length >= (sizeof(uint8_t) + sizeof(uint32_t) + sizeof(uint32_t))) && (Data[0] == PT_UART_COMMAND_ID_SLEEP_UART))
      {
         /* The flags word is currently unused (4 byts) so skip it and  */
         /* read the sleep time from the packet.                        */
         SleepTime = (unsigned int)(Data[5]) | ((unsigned int)(Data[6]) << 8) | ((unsigned int)(Data[7]) << 16) | ((unsigned int)(Data[8]) << 24);

         if(qurt_mutex_lock_timed(&(PtUartContext.ApiMutex), QURT_TIME_WAIT_FOREVER) == QURT_EOK)
         {
            if(!(PtUartContext.Flags & PT_UART_FLAGS_TRANSPORT_ASLEEP))
            {
               /* Make sure data isn't currently being transmitted.     */
               if(PtUartContext.TxBuffersFree == TX_BUFFER_COUNT)
               {
                  /* Start the time only if it doesn't indicate an      */
                  /* indefinite timeout.                                */
                  if(SleepTime != 0xFFFFFFFF)
                  {
                     /* Start the timer. */
                     qurt_timer_attr_init(&TimerAttr);
                     qurt_timer_attr_set_duration(&TimerAttr, qurt_timer_convert_time_to_ticks((qurt_time_t)SleepTime, QURT_TIME_MSEC));
                     qurt_timer_attr_set_signal(&TimerAttr, &(PtUartContext.Event), PT_UART_EVENT_WAKE);
                     qurt_timer_attr_set_option(&TimerAttr, QURT_TIMER_AUTO_START);
                     if(qurt_timer_create(&(PtUartContext.Timer), &TimerAttr) == QURT_EOK)
                     {
                        PtUartContext.Flags |= PT_UART_FLAGS_TIMER_ACTIVE;
                        Success = 1;
                     }
                     else
                        Success = 0;
                  }
                  else
                     Success = 1;

                  if(Success != 0)
                  {
                     DisableUART();
                  }
               }
               else
               {
                  /* Cannot sleep at this time, simply send the wake    */
                  /* event.                                             */
                  SendWake(PT_UART_WAKE_REASON_SLEEP_FAILED);
               }
            }

            qurt_mutex_unlock(&(PtUartContext.ApiMutex));
         }
      }
   }
}

   /* Instructs the UART to wake.  A return value of zero indicates that*/
   /* the UART was already awake, a positive value indicates the UART   */
   /* was previously asleep and successfully woken by this command and a*/
   /* negative value indicates an error occured.                        */
int PtUart_Wake(void)
{
   int ret_val;

   /* Check to make sure that the specified Transport ID is valid and   */
   /* the output buffer appears to be valid as well.                    */
   if(PtUartContext.Flags & PT_UART_FLAGS_TRANSPORT_OPEN)
   {
      if(qurt_mutex_lock_timed(&(PtUartContext.ApiMutex), QURT_TIME_WAIT_FOREVER) == QURT_EOK)
      {
         if(PtUartContext.Flags & PT_UART_FLAGS_TIMER_ACTIVE)
         {
            PtUartContext.Flags &= ~PT_UART_FLAGS_TIMER_ACTIVE;
            qurt_timer_delete(PtUartContext.Timer);
         }

         if(PtUartContext.Flags & PT_UART_FLAGS_TRANSPORT_ASLEEP)
         {
            /* Wake the UART. */
            if(EnableUART() == UART_SUCCESS)
            {
               SendWake(PT_UART_WAKE_REASON_DATA_PENDING);

               ret_val = 1;
            }
            else
               ret_val = -1;
         }
         else
            ret_val = 0;

         qurt_mutex_unlock(&(PtUartContext.ApiMutex));
      }
      else
         ret_val = -1;
   }
   else
      ret_val = -1;

   return(ret_val);
}

