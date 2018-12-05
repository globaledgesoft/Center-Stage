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

#ifndef __APP_MAIN_H__
#define __APP_MAIN_H__

#include "QPKTZR.h"

   /* Entry point into the application.                                 */
int app_start(void);

   /* This function sends a Q Packet from the packetizer to the host.   */
   /* The appropriate header is prepended before transmission.  This    */
   /* function is thread safe and handles serializing packets from      */
   /* different controller interfaces.                                  */
void SendQPacket(QPKTZR_Packet_t *QPacket);

   /* Needed types not in QAPIs.                                        */

typedef enum
{
   ptHCICommandPacket = 0x01,

   ptHCIACLDataPacket = 0x02,
   ptHCISCODataPacket = 0x03,
   ptHCIeSCODataPacket= 0x03,
   ptHCIEventPacket   = 0x04,
   ptHCIAdditional    = 0x05
} HCI_PacketType_t;

typedef struct _tagHCI_Packet_t
{
   HCI_PacketType_t HCIPacketType;
   unsigned int     HCIPacketLength;
   unsigned char    HCIPacketData[1];
} HCI_Packet_t;

#define HCI_PACKET_PACKET_LENGTH_HCI_PACKET_DATA_LENGTH_MASK   (((unsigned int)(-1)) >> 1)
#define HCI_PACKET_PACKET_LENGTH_HCI_PACKET_HEADER_MASK        (~HCI_PACKET_PACKET_LENGTH_HCI_PACKET_DATA_LENGTH_MASK)

typedef struct _tagHCI_Packet_Header_t
{
   HCI_PacketType_t  HCIPacketType;
   unsigned int      HCIPacketLength;
   unsigned char    *HCIPacketData;
} HCI_Packet_Header_t;

#define HCI_PACKET_HEADER_HEADER_SIZE                          (sizeof(HCI_Packet_Header_t))

typedef enum
{
   cpUART,
   cpUART_RTS_CTS,
   cpBCSP,
   cpBCSP_Muzzled,
   cpH4DS,
   cpH4DS_RTS_CTS,
   cpHCILL,
   cpHCILL_RTS_CTS,
   cp3Wire,
   cp3Wire_RTS_CTS,
   cpSIBS,
   cpSIBS_RTS_CTS
} HCI_COMM_Protocol_t;

typedef struct _tagHCI_COMMDriverInformation_t
{
   unsigned int         DriverInformationSize;
   unsigned int         COMPortNumber;
   unsigned long        BaudRate;
   HCI_COMM_Protocol_t  Protocol;
   unsigned int         InitializationDelay;
   char                *COMDeviceName;
  unsigned int          Flags;
} HCI_COMMDriverInformation_t;

#define HCI_COMM_DRIVER_INFORMATION_FLAGS_USE_EVEN_PARITY_FLAG    0x0001
typedef enum
{
   dtStonestreetOne,
   dtGarmin
} HCI_USB_Driver_t;

typedef struct _tagHCI_USBDriverInformation_t
{
   unsigned int     DriverInformationSize;
   HCI_USB_Driver_t DriverType;
   unsigned int     InitializationDelay;
} HCI_USBDriverInformation_t;

typedef enum
{
   hdtCOMM,
   hdtUSB
} HCI_DriverType_t;

typedef struct _tagHCI_DriverInformation_t
{
   HCI_DriverType_t DriverType;
   union
   {
     HCI_COMMDriverInformation_t COMMDriverInformation;
     HCI_USBDriverInformation_t  USBDriverInformation;
   } DriverInformation;
} HCI_DriverInformation_t;

#define HCI_DRIVER_SET_COMM_INFORMATION(_w, _x, _y, _z)                                                                         \
{                                                                                                                               \
   (_w)->DriverType                                                    = hdtCOMM;                                               \
   (_w)->DriverInformation.COMMDriverInformation.DriverInformationSize = sizeof((_w)->DriverInformation.COMMDriverInformation); \
   (_w)->DriverInformation.COMMDriverInformation.COMPortNumber         = (unsigned int)(_x);                                    \
   (_w)->DriverInformation.COMMDriverInformation.BaudRate              = (unsigned long)(_y);                                   \
   (_w)->DriverInformation.COMMDriverInformation.Protocol              = (_z);                                                  \
   (_w)->DriverInformation.COMMDriverInformation.InitializationDelay   = (unsigned int)0;                                       \
   (_w)->DriverInformation.COMMDriverInformation.COMDeviceName         = NULL;                                                  \
   (_w)->DriverInformation.COMMDriverInformation.Flags                 = 0;                                                     \
}

typedef void (*HCI_DriverCallback_t)(unsigned int HCIDriverID, HCI_Packet_t *HCIPacket, unsigned long CallbackParameter);

typedef uint32_t unpa_resource_state;

typedef union unpa_request
{
  unpa_resource_state val;
} unpa_request;

#define UNPA_CLIENT_REQUIRED  0x1
#define SLEEP_LATENCY_USEC    "slplat"
#define SLEEP_LATENCY_PERF    1

typedef int32_t (*HMI_Transmit_Callback_t)(uint32_t Length, const uint8_t *Data, void *Callback_Parameter);

extern void BTPS_FreeMemory(void *MemoryPointer);
extern int HCI_SendPacket(unsigned int HCIDriverID, HCI_Packet_t *HCIPacket, HCI_DriverCallback_t SendPacketCallback, unsigned long CallbackParameter);
extern void HMI_Process_Packet(uint32_t Length, const uint8_t *Data);
extern void *unpa_create_client( const char *client_name, uint32_t client_type, const char *resource_name );
extern void unpa_issue_request( void *client, unpa_resource_state request );

extern int InitializeBSCModule(void);
extern void BTPS_Init(void *UserParam);
extern int HCI_OpenDriver(HCI_DriverInformation_t *DriverInformation);
extern int HCI_RegisterEventCallback(unsigned int HCIDriverID, HCI_DriverCallback_t EventCallback, unsigned long CallbackParameter);
extern int HCI_RegisterACLDataCallback(unsigned int HCIDriverID, HCI_DriverCallback_t ACLDataCallback, unsigned long CallbackParameter);
extern int HCI_RegisterSCODataCallback(unsigned int HCIDriverID, HCI_DriverCallback_t SCODataCallback, unsigned long CallbackParameter);
extern void HCI_UnRegisterCallback(unsigned int HCIDriverID, unsigned int CallbackID);
extern void HCI_CloseDriver(unsigned int HCIDriverID);

extern qbool_t HMI_Initialize(HMI_Transmit_Callback_t Callback_Function, void *Callback_Parameter, qbool_t LatchNVM);
extern void HMI_Cleanup(void);

#endif

