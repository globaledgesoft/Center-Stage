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

#include <stdio.h>
#include <stdlib.h>
#include "qapi_types.h"
#include "qurt_error.h"
#include "qurt_thread.h"
#include "qurt_signal.h"
#include "qapi/qapi.h"
#include "qapi/qapi_status.h"
#include "qapi/qapi_uart.h"

#include "app_main.h"
#include "PtUart.h"
#include "QPKTZR.h"

   /* The following define the type of HCI callback being disptached.   */
#define HCI_DRIVER_CALLBACK_TYPE_EVENT    (0)
#define HCI_DRIVER_CALLBACK_TYPE_ACL      (1)
#define HCI_DRIVER_CALLBACK_TYPE_SCO      (2)

   /* The following defines the baud rate at which the HS UART to the   */
   /* host will be configured.                                          */
#if !defined(QUARTZ_EMULATION_V1)
#define UART_BAUD_RATE                    (1000000)
#else
#define UART_BAUD_RATE                    (2000000)
#endif

   /* The following defines this module's context.                      */
typedef struct PassthroughContext_s
{
   unsigned int Flags;
   unsigned int HCIDriverID;
   unsigned int HCIEventCallbackID;
   unsigned int HCIACLCallbackID;
   unsigned int HCISCOCallbackID;
} PassthroughContext_t;

#define PASSTHROUGH_CONTEXT_FLAG_BLE_INITIALIZED                        (0x00000001)
#define PASSTHROUGH_CONTEXT_FLAG_I15P4_INITIALIZED                      (0x00000002)

   /* Local module variables.                                           */

static PassthroughContext_t PassthroughContext;

   /* Local module function definitions.                                */
static void WakeAndSendQPacket(QPKTZR_Packet_t *QPacket);
static void Passthrough_Thread(void *Param);

static void HCI_DriverCallback(unsigned int HCIDriverID, HCI_Packet_t *HCIPacket, unsigned long CallbackParameter);
static int32_t HMI_Transmit_Callback(uint32_t Length, const uint8_t *Data, void *Callback_Parameter);
static void PtUart_Callback(unsigned int DataLength, unsigned char *DataBuffer, unsigned long CallbackParameter);

#if defined(V1) && !defined(QUARTZ_EMULATION_V1)

typedef struct RegisterOverride_s
{
   uint32_t Address;
   uint32_t Value;
   uint32_t Mask;
} RegisterOverride_t;

#define NUM_OVERRIDES                        10

static RegisterOverride_t BleTxFreqRegisterOverrides[40][NUM_OVERRIDES];
static RegisterOverride_t BleRxFreqRegisterOverrides[40][NUM_OVERRIDES];
static RegisterOverride_t I15p4TxFreqRegisterOverrides[16][NUM_OVERRIDES];
static RegisterOverride_t I15p4RxFreqRegisterOverrides[16][NUM_OVERRIDES];

static RegisterOverride_t BleTxRegisterOverrides[NUM_OVERRIDES];
static RegisterOverride_t BleRxRegisterOverrides[NUM_OVERRIDES];
static RegisterOverride_t I15p4TxRegisterOverrides[NUM_OVERRIDES];
static RegisterOverride_t I15p4RxRegisterOverrides[NUM_OVERRIDES];
#endif

   /* This function sends a Q Packet from the packetizer to the host.   */
   /* The appropriate header is prepended before transmission.  This    */
   /* function is thread safe and handles serializing packets from      */
   /* different controller interfaces.                                  */
static void WakeAndSendQPacket(QPKTZR_Packet_t *QPacket)
{
   unsigned int     PacketHeaderLength;
   unsigned int     PacketLength;
   uint8_t         *Packet;
   int              Result;

   if(PtUart_Wake() >= 0)
   {
      /* Only continue if the UART wake didn't error.                   */
      SendQPacket(QPacket);
   }
}

   /* HCI Callback that is dispatched when a HCI packet is received from*/
   /* the controller.                                                   */
static void HCI_DriverCallback(unsigned int HCIDriverID, HCI_Packet_t *HCIPacket, unsigned long CallbackParameter)
{
   QPKTZR_Packet_t  QPacket;

   /* Semi-validate the input parameters.                               */
   if(HCIPacket)
   {
      /* Fill in the Q Packet.                                          */
      QPacket.PacketType   = (unsigned int)HCIPacket->HCIPacketType;
      QPacket.PacketLength = HCIPacket->HCIPacketLength;
      QPacket.PacketData   = HCIPacket->HCIPacketData;

      /* Send the Q packet.                                             */
      WakeAndSendQPacket(&QPacket);

      /* Free the packet that was given to us by the HCI Driver.        */
      BTPS_FreeMemory(HCIPacket);
   }
}

   /* HMI Callback that is dispatched when a HMI packet is received from*/
   /* the controller.                                                   */
static int32_t HMI_Transmit_Callback(uint32_t Length, const uint8_t *Data, void *Callback_Parameter)
{
   QPKTZR_Packet_t  QPacket;
   unsigned int     PacketHeaderLength;
   unsigned int     PacketLength;
   uint8_t         *Packet;

   /* Semi-validate the input parameters.                               */
   if((Length) && (Data))
   {
      /* Fill in the Q Packet.                                          */
      QPacket.PacketType   = QPKTZR_PACKET_TYPE_15_4;
      QPacket.PacketLength = Length;
      QPacket.PacketData   = (uint8_t *)Data;

      /* Send the Q packet.                                             */
      WakeAndSendQPacket(&QPacket);
   }
}

   /* UART Callback that is dispatched when data is received from the   */
   /* host.                                                             */
static void PtUart_Callback(unsigned int DataLength, unsigned char *DataBuffer, unsigned long CallbackParameter)
{
   unsigned int         BytesLeft;
   QPKTZR_Packet_t     *ParsedPacket = NULL;
   HCI_Packet_Header_t  HCIPacket;

   /* Submit the stream to the packetizer, and check to see if a packet */
   /* is returned.                                                      */
   while((((int)(BytesLeft = (unsigned int)QPKTZR_Packetize_Stream(DataLength, DataBuffer, &ParsedPacket))) >= 0) && (BytesLeft <= DataLength))
   {
      DataBuffer += (DataLength - BytesLeft);
      DataLength  = BytesLeft;

      if(ParsedPacket)
      {
         /* A packet has been returned.  Process based on packet type.  */
         switch(ParsedPacket->PacketType)
         {
            case QPKTZR_PACKET_TYPE_HCI_COMMAND:
            case QPKTZR_PACKET_TYPE_HCI_ACL_DATA:
            case QPKTZR_PACKET_TYPE_HCI_SCO_DATA:
               if(PassthroughContext.Flags & PASSTHROUGH_CONTEXT_FLAG_BLE_INITIALIZED)
               {
                  /* The packet is HCI protocol.  Simply format up the  */
                  /* packet and send it to the driver.                  */

                  /* Format the HCI Packet.                             */
                  HCIPacket.HCIPacketType   = (HCI_PacketType_t)ParsedPacket->PacketType;
                  HCIPacket.HCIPacketLength = (ParsedPacket->PacketLength | HCI_PACKET_PACKET_LENGTH_HCI_PACKET_HEADER_MASK);
                  HCIPacket.HCIPacketData   = ParsedPacket->PacketData;

                  /* Submit the HCI Packet.                             */
                  HCI_SendPacket(PassthroughContext.HCIDriverID, (HCI_Packet_t *)&HCIPacket, NULL, 0);
               }
               break;
            case QPKTZR_PACKET_TYPE_15_4:
               if(PassthroughContext.Flags & PASSTHROUGH_CONTEXT_FLAG_I15P4_INITIALIZED)
               {
                  /* The packet is HMI protocol.  The HMI Layer simply     */
                  /* takes the packet length and pointer to the data.      */
                  HMI_Process_Packet(ParsedPacket->PacketLength, ParsedPacket->PacketData);
               }
               break;
            case QPKTZR_PACKET_TYPE_COEX:
               /* The packet is Coex protocol.  The Coex Layer simply   */
               /* takes the packet length and pointer to the data.      */

               /* Note we are currently not processing Coex TABI        */
               /* packets.                                              */
               break;
            case QPKTZR_PACKET_TYPE_UART:
               PtUart_Process_Packet(ParsedPacket->PacketLength, ParsedPacket->PacketData);
               break;
            default:
               break;
         }

         /* Free the packet from the packetizer.                        */
         QPKTZR_Free_Parsed_Packet(ParsedPacket);
      }
   }
}

static void HCITR_COMDataCallback(unsigned int HCITransportID, unsigned int DataLength, unsigned char *DataBuffer, unsigned long CallbackParameter)
{
   PtUart_Write(DataLength, DataBuffer);
}

   /* Thread used to initialize this module.                            */
static void Passthrough_Thread(void *Param)
{
   PtUart_Initialization_Data_t  InitializationData;
   HCI_DriverInformation_t       DriverInformation;
   void                         *UNPAClientUSLP;
   int                           Result;

   /* Do not allow deep sleep in the passthrough demo.                  */
   if((UNPAClientUSLP = unpa_create_client("demo", UNPA_CLIENT_REQUIRED, SLEEP_LATENCY_USEC)) != NULL)
      unpa_issue_request(UNPAClientUSLP, SLEEP_LATENCY_PERF);

#ifndef DISABLE_BLE

   /* Initialize Bluetopia RTOS shim since we will be using HCIDRV.     */
   BTPS_Init(NULL);

   /* Set up a standard COM.                                            */
   HCI_DRIVER_SET_COMM_INFORMATION(&DriverInformation, 1, 115200, cpUART);

   InitializeBSCModule();

   /* Open the HCI Driver.                                              */
   Result = HCI_OpenDriver(&DriverInformation);
   if(Result > 0)
   {
      PassthroughContext.HCIDriverID = (unsigned int)Result;

      /* Register the HCI Event Callback.                               */
      Result = HCI_RegisterEventCallback(PassthroughContext.HCIDriverID, HCI_DriverCallback, HCI_DRIVER_CALLBACK_TYPE_EVENT);
      if(Result > 0)
      {
         PassthroughContext.HCIEventCallbackID = (unsigned int)Result;

         /* Register the HCI Event Callback.                               */
         Result = HCI_RegisterACLDataCallback(PassthroughContext.HCIDriverID, HCI_DriverCallback, HCI_DRIVER_CALLBACK_TYPE_ACL);
         if(Result > 0)
         {
            PassthroughContext.HCIACLCallbackID = (unsigned int)Result;

            /* Register the HCI Event Callback.                               */
            Result = HCI_RegisterSCODataCallback(PassthroughContext.HCIDriverID, HCI_DriverCallback, HCI_DRIVER_CALLBACK_TYPE_SCO);
            if(Result > 0)
            {
               PassthroughContext.HCISCOCallbackID = (unsigned int)Result;

               PassthroughContext.Flags |= PASSTHROUGH_CONTEXT_FLAG_BLE_INITIALIZED;
            }
         }
      }

      if(Result <= 0)
      {
         if(PassthroughContext.HCIACLCallbackID)
            HCI_UnRegisterCallback(PassthroughContext.HCIDriverID, PassthroughContext.HCIACLCallbackID);

         if(PassthroughContext.HCIEventCallbackID)
            HCI_UnRegisterCallback(PassthroughContext.HCIDriverID, PassthroughContext.HCIEventCallbackID);

         HCI_CloseDriver(PassthroughContext.HCIDriverID);
      }
   }

#endif

#ifndef DISABLE_I15P4

   /* Initialize HMI and register the HMI Tx Callback.      */
   if(HMI_Initialize(HMI_Transmit_Callback, (void *)0, false))
   {
      PassthroughContext.Flags |= PASSTHROUGH_CONTEXT_FLAG_I15P4_INITIALIZED;
   }

#endif

   /* Initialize the packetizer.                         */
   if(QPKTZR_Initialize() == 0)
   {
      InitializationData.Flags    = PT_UART_INITIALIZATION_DATA_FLAG_USE_HS_UART | PT_UART_INITIALIZATION_DATA_FLAG_USE_HW_FLOW;
      InitializationData.BaudRate = UART_BAUD_RATE;

      /* Initialize the UART to the host.                */
      if(PtUart_Initialize(&InitializationData, PtUart_Callback, 0) != PT_UART_ERROR_SUCCESS)
      {
         QPKTZR_Cleanup();

         if(PassthroughContext.Flags & PASSTHROUGH_CONTEXT_FLAG_I15P4_INITIALIZED)
         {
            HMI_Cleanup();
            PassthroughContext.Flags &= ~PASSTHROUGH_CONTEXT_FLAG_I15P4_INITIALIZED;
         }

         if(PassthroughContext.Flags & PASSTHROUGH_CONTEXT_FLAG_BLE_INITIALIZED)
         {
            HCI_UnRegisterCallback(PassthroughContext.HCIDriverID, PassthroughContext.HCISCOCallbackID);
            HCI_UnRegisterCallback(PassthroughContext.HCIDriverID, PassthroughContext.HCIACLCallbackID);
            HCI_UnRegisterCallback(PassthroughContext.HCIDriverID, PassthroughContext.HCIEventCallbackID);
            HCI_CloseDriver(PassthroughContext.HCIDriverID);
            PassthroughContext.Flags &= ~PASSTHROUGH_CONTEXT_FLAG_BLE_INITIALIZED;
         }
      }
   }

   /* Call into QuRT to free the memory associated with the task and    */
   /* schedule another task to run.                                     */
   qurt_thread_stop();
}

   /* Entry point into the application.                                 */
int app_start(void)
{
   qurt_thread_attr_t  Thread_Attribte;
   qurt_thread_t       Thread_Handle;

   qurt_thread_attr_init(&Thread_Attribte);
   qurt_thread_attr_set_name(&Thread_Attribte, "Pt Thread");
   qurt_thread_attr_set_priority(&Thread_Attribte, 10);
   qurt_thread_attr_set_stack_size(&Thread_Attribte, 4096);

   /* Create a thread in order to initialize in thread (RTOS) context.  */
   qurt_thread_create(&Thread_Handle, &Thread_Attribte, Passthrough_Thread, NULL);

   return(QURT_EOK);
}

   /* This function sends a Q Packet from the packetizer to the host.   */
   /* The appropriate header is prepended before transmission.  This    */
   /* function is thread safe and handles serializing packets from      */
   /* different controller interfaces.                                  */
void SendQPacket(QPKTZR_Packet_t *QPacket)
{
   unsigned int     PacketHeaderLength;
   unsigned int     PacketLength;
   uint8_t         *Packet;
   int              Result;

   /* Only continue if the UART wake didn't error.                      */
   if(Result >= 0)
   {
      /* Get the size of the packet header.                             */
      if(((int)(PacketHeaderLength = (unsigned int)QPKTZR_Build_Stream_Header(QPacket, 0, NULL))) > 0)
      {
         /* Store the entire packet length (header + payload).          */
         PacketLength = PacketHeaderLength + QPacket->PacketLength;

         /* Allocate a packet buffer.                                   */
         if((Packet = (uint8_t *)malloc(PacketLength)) != NULL)
         {
            /* Parse the packet header.                                 */
            if(QPKTZR_Build_Stream_Header(QPacket, PacketHeaderLength, Packet) > 0)
            {
               /* Copy over the packet payload.                         */
               memcpy(&(Packet[PacketHeaderLength]), QPacket->PacketData, QPacket->PacketLength);

               /* Send the packet over the UART.  This call will        */
               /* serialize packets being submitted from different      */
               /* threads so long as the entire packet is submitted at  */
               /* once.                                                 */
               PtUart_Write(PacketLength, Packet);
            }

            /* Free the memory used for the packet.                     */
            free(Packet);
         }
      }
   }
}

