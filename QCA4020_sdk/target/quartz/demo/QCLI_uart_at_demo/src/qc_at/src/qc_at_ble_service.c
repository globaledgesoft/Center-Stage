/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
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
#include <string.h>
#include "qc_at_ble_service.h"
#include "qc_at_ble_service_types.h"

#include "qapi.h"

#include "malloc.h"
#include "qurt_signal.h"
#include "qurt_timer.h"
#include "qurt_mutex.h"
#include "qurt_error.h"

#include "string.h"

#define BLE_OTA_TIMEOUT                               (qurt_timer_convert_time_to_ticks(180000, QURT_TIME_MSEC))

#define READ_UNALIGNED_BYTE_LITTLE_ENDIAN(_x)   (((uint8_t *)(_x))[0])
#define READ_UNALIGNED_WORD_LITTLE_ENDIAN(_x)   ((uint16_t)((((uint16_t)(((uint8_t *)(_x))[1])) << 8) | ((uint16_t)(((uint8_t *)(_x))[0]))))
#define READ_UNALIGNED_DWORD_LITTLE_ENDIAN(_x)  ((uint32_t)((((uint32_t)(((uint8_t *)(_x))[3])) << 24) | (((uint32_t)(((uint8_t *)(_x))[2])) << 16) | (((uint32_t)(((uint8_t *)(_x))[1])) << 8) | ((uint32_t)(((uint8_t *)(_x))[0]))))

#define ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(_x, _y)       \
{                                                                      \
  ((uint8_t *)(_x))[0] = ((uint8_t)(_y));                              \
}

#define ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(_x, _y)       \
{                                                                      \
  ((uint8_t *)(_x))[0] = ((uint8_t)(((uint16_t)(_y)) & 0xFF));         \
  ((uint8_t *)(_x))[1] = ((uint8_t)((((uint16_t)(_y)) >> 8) & 0xFF));  \
}

#define ASSIGN_HOST_DWORD_TO_LITTLE_ENDIAN_UNALIGNED_DWORD(_x, _y)     \
{                                                                      \
  ((uint8_t *)(_x))[0] = ((uint8_t)(((uint32_t)(_y)) & 0xFF));         \
  ((uint8_t *)(_x))[1] = ((uint8_t)((((uint32_t)(_y)) >> 8) & 0xFF));  \
  ((uint8_t *)(_x))[2] = ((uint8_t)((((uint32_t)(_y)) >> 16) & 0xFF)); \
  ((uint8_t *)(_x))[3] = ((uint8_t)((((uint32_t)(_y)) >> 24) & 0xFF)); \
}

typedef struct BLE_OTA_Image_s
{
   uint32_t ImageID;
   char     FileName[BLE_OTA_MAXIMUM_FILE_NAME_LENGTH];
   uint32_t Version;
   uint32_t ImageLength;
} BLE_OTA_Image_t;

#define BLE_OTA_SERVER_CONTEXT_FLAGS_INITIALIZED                        0x0001

typedef struct BLE_OTA_Server_Client_Information_s
{
   uint32_t ClientConnectionID;
   uint16_t CCD_Value;
} BLE_OTA_Server_Client_Information_t;

typedef struct BLE_OTA_Server_Context_s
{
   BLE_OTA_Server_Client_Information_t ClientInformation[BLE_OTA_MAXIMUM_NUMBER_CLIENTS];
   uint32_t                            ServiceID;
   uint16_t                            Flags;
   qurt_mutex_t                        Mutex;
   BLE_OTA_Server_Event_Callback_t     EventCallback;
   void                               *CallbackParameter;
} BLE_OTA_Server_Context_t;

#define BLE_OTA_CLIENT_CONTEXT_FLAGS_INITIALIZED                        0x0001

#define BLE_OTA_CLIENT_EVENT_FLAGS_DISCOVER_SERVICE_SUCCESS             0x0001
#define BLE_OTA_CLIENT_EVENT_FLAGS_DISCOVER_SERVICE_FAILURE             0x0002
#define BLE_OTA_CLIENT_EVENT_FLAGS_QUERY_IMAGE_SUCCESS                  0x0004
#define BLE_OTA_CLIENT_EVENT_FLAGS_QUERY_IMAGE_FAILURE                  0x0008
#define BLE_OTA_CLIENT_EVENT_FLAGS_READ_IMAGE_DATA_SUCCESS              0x0010
#define BLE_OTA_CLIENT_EVENT_FLAGS_READ_IMAGE_DATA_FAILURE              0x0020

typedef struct BLE_OTA_Client_Service_Information_s
{
   uint32_t ServerConnectionID;
   uint16_t MTU;
   uint16_t Control_Point_Characteristic;
   uint16_t Control_Point_CCD;
} BLE_OTA_Client_Service_Information_t;

typedef struct BLE_OTA_Client_Context_s
{
   BLE_OTA_Client_Service_Information_t ServiceInformation[BLE_OTA_MAXIMUM_NUMBER_SERVERS];
   int                                  ConnectionCallbackID;
   uint16_t                             Flags;
   uint16_t                             Discovered_Control_Point_Characteristic;
   uint16_t                             Discovered_Control_Point_CCD;
   qurt_signal_t                        Signal;
   qurt_mutex_t                         Mutex;
   uint8_t                             *DataBuffer;
   uint32_t                             DataBufferLength;
   uint32_t                             CurrentImageOffset;
   uint32_t                             BytesReceived;
   uint32_t                             QueriedImageID;
   uint32_t                             QueriedImageLength;
   uint32_t                             QueriedImageVersion;
} BLE_OTA_Client_Context_t;

#define BLE_OTA_ASSIGN_BLE_OTA_SERVICE_UUID_128(_x)                 QAPI_BLE_ASSIGN_BLUETOOTH_UUID_128(*((qapi_BLE_UUID_128_t *)(_x)), 0x71, 0xDC, 0xEC, 0x60, 0x66, 0x14, 0x54, 0x86, 0x78, 0x4F, 0x84, 0x82, 0x98, 0x11, 0xF3, 0xA8)
#define BLE_OTA_COMPARE_BLE_OTA_SERVICE_UUID_TO_UUID_128(_x)        QAPI_BLE_COMPARE_BLUETOOTH_UUID_128_TO_CONSTANT((_x), 0x71, 0xDC, 0xEC, 0x60, 0x66, 0x14, 0x54, 0x86, 0x78, 0x4F, 0x84, 0x82, 0x98, 0x11, 0xF3, 0xA8)
#define BLE_OTA_SERVICE_UUID_CONSTANT                               { 0xA8, 0xF3, 0x11, 0x98, 0x82, 0x84, 0x4F, 0x78, 0x86, 0x54, 0x14, 0x66, 0x60, 0xEC, 0xDC, 0x71 }
#define BLE_OTA_ASSIGN_CP_UUID_128(_x)                              QAPI_BLE_ASSIGN_BLUETOOTH_UUID_128((_x), 0x8A, 0xF9, 0xB6, 0x71, 0xE8, 0x78, 0x94, 0xAB, 0xA8, 0x4B, 0xDA, 0x98, 0x44, 0x89, 0x71, 0x51)
#define BLE_OTA_COMPARE_BLE_OTA_CP_UUID_TO_UUID_128(_x)             QAPI_BLE_COMPARE_BLUETOOTH_UUID_128_TO_CONSTANT((_x), 0x8A, 0xF9, 0xB6, 0x71, 0xE8, 0x78, 0x94, 0xAB, 0xA8, 0x4B, 0xDA, 0x98, 0x44, 0x89, 0x71, 0x51)
#define BLE_OTA_CONTROL_POINT_CHARACTERISTIC_UUID_CONSTANT          { 0x51, 0x71, 0x89, 0x44, 0x98, 0xDA, 0x4B, 0xA8, 0xAB, 0x94, 0x78, 0xE8, 0x71, 0xB6, 0xF9, 0x8A }

   /*********************************************************************/
   /**                       OTA Service Table                         **/
   /*********************************************************************/

   /* The OTA Service Declaration UUID. */
static const qapi_BLE_GATT_Primary_Service_128_Entry_t BLE_OTA_Service_UUID =
{
   BLE_OTA_SERVICE_UUID_CONSTANT
} ;

   /* The Tx Characteristic Declaration. */
static const qapi_BLE_GATT_Characteristic_Declaration_128_Entry_t BLE_OTA_Control_Point_Declaration =
{
   (QAPI_BLE_GATT_CHARACTERISTIC_PROPERTIES_WRITE | QAPI_BLE_GATT_CHARACTERISTIC_PROPERTIES_INDICATE),
   BLE_OTA_CONTROL_POINT_CHARACTERISTIC_UUID_CONSTANT
} ;

   /* The Tx Characteristic Value. */
static const qapi_BLE_GATT_Characteristic_Value_128_Entry_t  BLE_OTA_Control_Point_Value =
{
   BLE_OTA_CONTROL_POINT_CHARACTERISTIC_UUID_CONSTANT,
   0,
   NULL
} ;

   /* Client Characteristic Configuration Descriptor. */
static qapi_BLE_GATT_Characteristic_Descriptor_16_Entry_t Client_Characteristic_Configuration =
{
   QAPI_BLE_GATT_CLIENT_CHARACTERISTIC_CONFIGURATION_BLUETOOTH_UUID_CONSTANT,
   QAPI_BLE_GATT_CLIENT_CHARACTERISTIC_CONFIGURATION_LENGTH,
   NULL
};

   /* The following defines the OTA service that is registered with   */
   /* the qapi_BLE_GATT_Register_Service function call.               */
   /* * NOTE * This array will be registered with GATT in the call to */
   /*          qapi_BLE_GATT_Register_Service.                        */
const qapi_BLE_GATT_Service_Attribute_Entry_t BLE_OTA_Service[] =
{
   {QAPI_BLE_GATT_ATTRIBUTE_FLAGS_READABLE,          QAPI_BLE_AET_PRIMARY_SERVICE_128_E,            (uint8_t *)&BLE_OTA_Service_UUID               },
   {QAPI_BLE_GATT_ATTRIBUTE_FLAGS_READABLE,          QAPI_BLE_AET_CHARACTERISTIC_DECLARATION_128_E, (uint8_t *)&BLE_OTA_Control_Point_Declaration  },
   {QAPI_BLE_GATT_ATTRIBUTE_FLAGS_WRITABLE,          QAPI_BLE_AET_CHARACTERISTIC_VALUE_128_E,       (uint8_t *)&BLE_OTA_Control_Point_Value        },
   {QAPI_BLE_GATT_ATTRIBUTE_FLAGS_READABLE_WRITABLE, QAPI_BLE_AET_CHARACTERISTIC_DESCRIPTOR_16_E,   (uint8_t *)&Client_Characteristic_Configuration},
} ;

#define BLE_OTA_SERVICE_ATTRIBUTE_COUNT                  (sizeof(BLE_OTA_Service)/sizeof(qapi_BLE_GATT_Service_Attribute_Entry_t))

#define BLE_OTA_CONTROL_POINT_ATTRIBUTE_OFFSET           2
#define BLE_OTA_CONTROL_POINT_CCD_ATTRIBUTE_OFFSET       3

static BLE_OTA_Image_t          BLEOTAImageList[BLE_OTA_MAXIMUM_NUMBER_OF_IMAGES];
static uint32_t                 BLEOTANumberRegisteredImages;
static uint32_t                 BLEOTAImageID;
static BLE_OTA_Server_Context_t BLEOTAServerContext;
static BLE_OTA_Client_Context_t BLEOTAClientContext;

static BLE_OTA_Server_Client_Information_t *FindClientInformation(uint32_t ClientConnectionID);
static BLE_OTA_Client_Service_Information_t *FindServiceInformation(uint32_t ServerConnectionID);
static uint32_t FindUpdateImage(char *FileName, uint32_t *Version, uint32_t *ImageLength);
static void HandleImageReadRequest(uint32_t BluetoothStackID, uint32_t ServiceID, uint32_t ConnectionID, uint32_t TransactionID, uint8_t *Data, uint16_t Length);
static void HandleImageQueryRequest(uint32_t BluetoothStackID, uint32_t ServiceID, uint32_t ConnectionID, uint32_t TransactionID, uint8_t *Data, uint16_t Length);
static void GATT_ServerEventCallback_OTA(uint32_t BluetoothStackID, qapi_BLE_GATT_Server_Event_Data_t *GATT_ServerEventData, uint32_t CallbackParameter);
static void BLE_OTA_Parse_Service_Discovery_Indication(qapi_BLE_GATT_Service_Discovery_Indication_Data_t *IndicationData);
static void BLE_OTA_Service_Discovery_Callback(uint32_t BluetoothStackID, qapi_BLE_GATT_Service_Discovery_Event_Data_t *ServiceDiscoveryEventData, uint32_t CallbackParameter);

static void HandleImageReadResponse(uint8_t *Data, uint16_t Length);
static void HandleImageQueryResponse(uint8_t *Data, uint16_t Length);
static void OTA_Client_Connection_Event_Callback(uint32_t BluetoothStackID, qapi_BLE_GATT_Connection_Event_Data_t *GATT_Connection_Event_Data, uint32_t CallbackParameter);
static void OTA_ClientCallback(uint32_t BluetoothStackID, qapi_BLE_GATT_Client_Event_Data_t *GATT_Client_Event_Data, uint32_t CallbackParameter);

static uint32_t GetNextImageID(void);

static uint8_t  SendImageDataRequest(uint32_t BluetoothStackID, BLE_OTA_Client_Service_Information_t *ServiceInfo, uint32_t ImageID, uint32_t DataLength, uint32_t FileOffset);

static uint32_t GetImageDataResponse(void);

static BLE_OTA_Server_Client_Information_t *FindClientInformation(uint32_t ClientConnectionID)
{
   uint8_t                              Index;
   BLE_OTA_Server_Client_Information_t *ClientInfo = NULL;

   for(Index = 0; Index < BLE_OTA_MAXIMUM_NUMBER_SERVERS; Index++)
   {
      if(BLEOTAServerContext.ClientInformation[Index].ClientConnectionID == ClientConnectionID)
      {
         ClientInfo = &BLEOTAServerContext.ClientInformation[Index];
         break;
      }
   }

   return(ClientInfo);
}

static BLE_OTA_Client_Service_Information_t *FindServiceInformation(uint32_t ServerConnectionID)
{
   uint8_t                               Index;
   BLE_OTA_Client_Service_Information_t *ServiceInfo = NULL;

   for(Index = 0; Index < BLE_OTA_MAXIMUM_NUMBER_SERVERS; Index++)
   {
      if(BLEOTAClientContext.ServiceInformation[Index].ServerConnectionID == ServerConnectionID)
      {
         ServiceInfo = &BLEOTAClientContext.ServiceInformation[Index];
         break;
      }
   }

   return(ServiceInfo);
}

static void HandleImageReadRequest(uint32_t BluetoothStackID, uint32_t ServiceID, uint32_t ConnectionID, uint32_t TransactionID, uint8_t *Data, uint16_t Length)
{
   uint32_t                                        ImageID;
   uint32_t                                        FileOffset;
   uint16_t                                        DataLength;
   BLE_OTA_Server_Image_Data_Request_Event_Data_t  ImageDataRequest;
   BLE_OTA_Server_Event_Data_t                     EventData;

   if((Data) && (FindClientInformation(ConnectionID) != NULL))
   {
      /* Get the packet data. */
      ImageID    = READ_UNALIGNED_DWORD_LITTLE_ENDIAN(&((BLE_OTA_Command_Read_Image_Data_Request_t *)Data)->ImageID);
      FileOffset = READ_UNALIGNED_DWORD_LITTLE_ENDIAN(&((BLE_OTA_Command_Read_Image_Data_Request_t *)Data)->FileOffset);
      DataLength = READ_UNALIGNED_WORD_LITTLE_ENDIAN( &((BLE_OTA_Command_Read_Image_Data_Request_t *)Data)->DataLength);

      /* Set the event variables. */
      EventData.Event_Data_Type = BLE_OTA_SERVER_EVENT_TYPE_IMAGE_DATA_REQUEST_E;
      EventData.Event_Data.BLE_OTA_Server_Image_Data_Request_Event_Data = &ImageDataRequest;

      ImageDataRequest.ConnectionID  = ConnectionID;
      ImageDataRequest.TransactionID = TransactionID;
      ImageDataRequest.ImageID       = ImageID;
      ImageDataRequest.FileOffset    = FileOffset;
      ImageDataRequest.DataLength    = DataLength;

      /* Issue the callback. */
      if(BLEOTAServerContext.EventCallback)
      {
         (BLEOTAServerContext.EventCallback)(BluetoothStackID, &EventData, BLEOTAServerContext.CallbackParameter);
      }
   }
}

static uint32_t FindUpdateImage(char *FileName, uint32_t *Version, uint32_t *ImageLength)
{
   uint8_t  Index;
   uint32_t ImageID = 0;

   if(ImageLength)
   {
      *ImageLength = 0;

      /* Search the list of registered images for the same filename and updated version. */
      for(Index = 0; Index < BLE_OTA_MAXIMUM_NUMBER_OF_IMAGES; Index++)
      {
         if((strcmp(BLEOTAImageList[Index].FileName, FileName) == 0) && \
            (BLEOTAImageList[Index].Version > *Version))
         {
            ImageID      = BLEOTAImageList[Index].ImageID;
            *ImageLength = BLEOTAImageList[Index].ImageLength;
            *Version     = BLEOTAImageList[Index].Version;
            break;
         }
      }
   }

   return(ImageID);
}

static void HandleImageQueryRequest(uint32_t BluetoothStackID, uint32_t ServiceID, uint32_t ConnectionID, uint32_t TransactionID, uint8_t *Data, uint16_t Length)
{
   uint32_t  ImageID;
   uint32_t  Version;
   uint32_t  ImageLength;
   uint8_t   FileNameLength;
   uint8_t   Status;
   char     *FileName;
   uint8_t  *PacketBuffer;

   if((Data) && (FindClientInformation(ConnectionID) != NULL))
   {
      Version        = READ_UNALIGNED_DWORD_LITTLE_ENDIAN(&((BLE_OTA_Command_Query_Image_Request_t *)Data)->Version);
      FileNameLength = READ_UNALIGNED_BYTE_LITTLE_ENDIAN(&((BLE_OTA_Command_Query_Image_Request_t *)Data)->FileNameLength);

      if(FileNameLength <= BLE_OTA_MAXIMUM_FILE_NAME_LENGTH)
      {
         FileName = (char *)malloc(FileNameLength+1);

         if(FileName)
         {
            strcpy(FileName, (char *)((BLE_OTA_Command_Query_Image_Request_t *)Data)->FileName);

            ImageID = FindUpdateImage(FileName, &Version, &ImageLength);

            /* Allocate a buffer to send. */
            PacketBuffer = (uint8_t *)malloc(BLE_OTA_COMMAND_QUERY_IMAGE_RESPONSE_SIZE);

            if(ImageID)
               Status = BLE_OTA_STATUS_SUCCESS;
            else
            {
               Version     = 0;
               ImageLength = 0;
               Status = BLE_OTA_STATUS_NO_UPDATE_AVAILABLE;
            }

            if(PacketBuffer)
            {
               /* Format the packet. */
               ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(  &((BLE_OTA_Command_Query_Image_Response_t *)PacketBuffer)->Command,     BLE_OTA_COMMAND_QUERY_IMAGE_RESPONSE);
               ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(  &((BLE_OTA_Command_Query_Image_Response_t *)PacketBuffer)->Status,      Status);
               ASSIGN_HOST_DWORD_TO_LITTLE_ENDIAN_UNALIGNED_DWORD(&((BLE_OTA_Command_Query_Image_Response_t *)PacketBuffer)->ImageID,     ImageID);
               ASSIGN_HOST_DWORD_TO_LITTLE_ENDIAN_UNALIGNED_DWORD(&((BLE_OTA_Command_Query_Image_Response_t *)PacketBuffer)->ImageLength, ImageLength);
               ASSIGN_HOST_DWORD_TO_LITTLE_ENDIAN_UNALIGNED_DWORD(&((BLE_OTA_Command_Query_Image_Response_t *)PacketBuffer)->Version,     Version);

               /* Send the indication. */
               qapi_BLE_GATT_Handle_Value_Indication(BluetoothStackID, ServiceID, ConnectionID, BLE_OTA_CONTROL_POINT_ATTRIBUTE_OFFSET, BLE_OTA_COMMAND_QUERY_IMAGE_RESPONSE_SIZE, PacketBuffer);

               /* Free the packet buffer. */
               free(PacketBuffer);
            }

            free(FileName);
         }
      }
   }
}

static void GATT_ServerEventCallback_OTA(uint32_t BluetoothStackID, qapi_BLE_GATT_Server_Event_Data_t *GATT_ServerEventData, uint32_t CallbackParameter)
{
   uint16_t                             Value;
   uint16_t                             AttributeOffset;
   uint16_t                             AttributeLength;
   uint32_t                             ConnectionID;
   uint32_t                             ServiceID;
   uint32_t                             TransactionID;
   BLE_OTA_Server_Client_Information_t *ClientInfo;

   /* Verify that all parameters to this callback are Semi-Valid.       */
   if((BluetoothStackID) && (GATT_ServerEventData))
   {
      switch(GATT_ServerEventData->Event_Data_Type)
      {
         case QAPI_BLE_ET_GATT_SERVER_READ_REQUEST_E:
            /* Verify that the Event Data is valid.                  */
            if(GATT_ServerEventData->Event_Data.GATT_Read_Request_Data)
            {
               if(GATT_ServerEventData->Event_Data.GATT_Read_Request_Data->AttributeValueOffset == 0)
               {
                  /* Determine which request this read is coming for.*/
                  switch(GATT_ServerEventData->Event_Data.GATT_Read_Request_Data->AttributeOffset)
                  {
                     case BLE_OTA_CONTROL_POINT_CCD_ATTRIBUTE_OFFSET:
                        if((ClientInfo = FindClientInformation(GATT_ServerEventData->Event_Data.GATT_Read_Request_Data->ConnectionID)) != NULL)
                        {
                           ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&Value, ClientInfo->CCD_Value);
                        }
                        else
                           Value = 0;

                        qapi_BLE_GATT_Read_Response(BluetoothStackID, GATT_ServerEventData->Event_Data.GATT_Read_Request_Data->TransactionID, sizeof(Value), (uint8_t *)&Value);
                        break;
                     default:
                        qapi_BLE_GATT_Error_Response(BluetoothStackID, GATT_ServerEventData->Event_Data.GATT_Read_Request_Data->TransactionID, GATT_ServerEventData->Event_Data.GATT_Read_Request_Data->AttributeOffset, QAPI_BLE_ATT_PROTOCOL_ERROR_CODE_READ_NOT_PERMITTED);
                  }
               }
               else
                  qapi_BLE_GATT_Error_Response(BluetoothStackID, GATT_ServerEventData->Event_Data.GATT_Read_Request_Data->TransactionID, GATT_ServerEventData->Event_Data.GATT_Read_Request_Data->AttributeOffset, QAPI_BLE_ATT_PROTOCOL_ERROR_CODE_ATTRIBUTE_NOT_LONG);
            }
            break;
         case QAPI_BLE_ET_GATT_SERVER_WRITE_REQUEST_E:
            /* Verify that the Event Data is valid.                  */
            if(GATT_ServerEventData->Event_Data.GATT_Write_Request_Data)
            {
               if(GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->AttributeValueOffset == 0)
               {
                  /* Cache the Attribute Offset.                     */
                  AttributeOffset = GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->AttributeOffset;
                  AttributeLength = GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->AttributeValueLength;

                  ConnectionID  = GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->ConnectionID;
                  ServiceID     = GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->ServiceID;
                  TransactionID = GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->TransactionID;

                  /* Handle a control point write.  */
                  if(AttributeOffset == BLE_OTA_CONTROL_POINT_ATTRIBUTE_OFFSET)
                  {
                     Value = READ_UNALIGNED_BYTE_LITTLE_ENDIAN(GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->AttributeValue);

                     switch(Value)
                     {
                        case BLE_OTA_COMMAND_QUERY_IMAGE_REQUEST:
                           qapi_BLE_GATT_Write_Response(BluetoothStackID, GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->TransactionID);
                           HandleImageQueryRequest(BluetoothStackID, ServiceID, ConnectionID, TransactionID, GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->AttributeValue, AttributeLength);
                           break;
                        case BLE_OTA_COMMAND_READ_IMAGE_DATA_REQUEST:
                           qapi_BLE_GATT_Write_Response(BluetoothStackID, GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->TransactionID);
                           HandleImageReadRequest(BluetoothStackID, ServiceID, ConnectionID, TransactionID, GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->AttributeValue, AttributeLength);
                           break;
                        default:
                           qapi_BLE_GATT_Error_Response(BluetoothStackID, GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->TransactionID, GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->AttributeOffset, QAPI_BLE_ATT_PROTOCOL_ERROR_CODE_REQUEST_NOT_SUPPORTED);
                           break;
                     }
                  }
                  else if(AttributeOffset == BLE_OTA_CONTROL_POINT_CCD_ATTRIBUTE_OFFSET)
                  {
                     /* Handle a CCD write. */
                     Value = READ_UNALIGNED_WORD_LITTLE_ENDIAN(GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->AttributeValue);

                     if((ClientInfo = FindClientInformation(GATT_ServerEventData->Event_Data.GATT_Read_Request_Data->ConnectionID)) != NULL)
                     {
                        /* Handle rewrite of an existing stored value. */
                        if(Value != QAPI_BLE_GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_INDICATE_ENABLE)
                        {
                           /* Remove client information if unsubscribing from indications. */
                           ClientInfo->ClientConnectionID = 0;
                           ClientInfo->CCD_Value          = 0;
                        }

                        qapi_BLE_GATT_Write_Response(BluetoothStackID, GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->TransactionID);
                     }
                     else
                     {
                        /* Get a new client info pointer. */
                        if((ClientInfo = FindClientInformation(0)) != NULL)
                        {
                           if(Value == QAPI_BLE_GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_INDICATE_ENABLE)
                           {
                              ClientInfo->ClientConnectionID = GATT_ServerEventData->Event_Data.GATT_Read_Request_Data->ConnectionID;
                              ClientInfo->CCD_Value          = Value;
                           }
                           qapi_BLE_GATT_Write_Response(BluetoothStackID, GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->TransactionID);
                        }
                        else
                        {
                           qapi_BLE_GATT_Error_Response(BluetoothStackID, GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->TransactionID, GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->AttributeOffset, QAPI_BLE_ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_RESOURCES);
                        }
                     }
                  }
                  else
                     qapi_BLE_GATT_Error_Response(BluetoothStackID, GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->TransactionID, GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->AttributeOffset, QAPI_BLE_ATT_PROTOCOL_ERROR_CODE_INVALID_HANDLE);
               }
               else
                  qapi_BLE_GATT_Error_Response(BluetoothStackID, GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->TransactionID, GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->AttributeOffset, QAPI_BLE_ATT_PROTOCOL_ERROR_CODE_ATTRIBUTE_NOT_LONG);
            }
            break;
         default:
            break;
      }
   }
}

static void BLE_OTA_Parse_Service_Discovery_Indication(qapi_BLE_GATT_Service_Discovery_Indication_Data_t *IndicationData)
{
   unsigned int                                           Index1;
   unsigned int                                           Index2;
   qapi_BLE_GATT_Characteristic_Information_t            *CurrentCharacteristic;
   qapi_BLE_GATT_Characteristic_Descriptor_Information_t *CurrentDescriptor;

   /* Verify that the input parameters are semi-valid. */
   if((IndicationData) && (IndicationData->ServiceInformation.UUID.UUID_Type == QAPI_BLE_GU_UUID_128_E) && (BLE_OTA_COMPARE_BLE_OTA_SERVICE_UUID_TO_UUID_128(IndicationData->ServiceInformation.UUID.UUID.UUID_128)))
   {
      /* Loop through all characteristics discovered in the service */
      /* and populate the correct entry.                            */
      CurrentCharacteristic = IndicationData->CharacteristicInformationList;
      if(CurrentCharacteristic)
      {
         for(Index1 = 0; Index1 < IndicationData->NumberOfCharacteristics; Index1++, CurrentCharacteristic++)
         {
            if(BLE_OTA_COMPARE_BLE_OTA_CP_UUID_TO_UUID_128(CurrentCharacteristic->Characteristic_UUID.UUID.UUID_128))
            {
               /* All BLE OTA UUIDs are defined to be 128 bit UUIDs. */
               if(CurrentCharacteristic->Characteristic_UUID.UUID_Type == QAPI_BLE_GU_UUID_128_E)
               {
                  BLEOTAClientContext.Discovered_Control_Point_Characteristic = CurrentCharacteristic->Characteristic_Handle;

                  /* Loop through the Descriptor List. */
                  CurrentDescriptor = CurrentCharacteristic->DescriptorList;
                  for(Index2 = 0; Index2 < CurrentCharacteristic->NumberOfDescriptors; Index2++)
                  {
                     if(CurrentDescriptor->Characteristic_Descriptor_UUID.UUID_Type == QAPI_BLE_GU_UUID_16_E)
                     {
                        if(QAPI_BLE_GATT_COMPARE_CLIENT_CHARACTERISTIC_CONFIGURATION_ATTRIBUTE_TYPE_TO_BLUETOOTH_UUID_16(CurrentCharacteristic->DescriptorList[Index2].Characteristic_Descriptor_UUID.UUID.UUID_16))
                           BLEOTAClientContext.Discovered_Control_Point_CCD = CurrentCharacteristic->DescriptorList[Index2].Characteristic_Descriptor_Handle;
                     }
                  }
               }
            }
         }
      }
   }
}

static void BLE_OTA_Service_Discovery_Callback(uint32_t BluetoothStackID, qapi_BLE_GATT_Service_Discovery_Event_Data_t *ServiceDiscoveryEventData, uint32_t CallbackParameter)
{
   if((BluetoothStackID) && (ServiceDiscoveryEventData))
   {
      switch(ServiceDiscoveryEventData->Event_Data_Type)
      {
         case QAPI_BLE_ET_GATT_SERVICE_DISCOVERY_INDICATION_E:
            if(ServiceDiscoveryEventData->Event_Data.GATT_Service_Discovery_Indication_Data)
            {
               /* Parse the indication data. */
               BLE_OTA_Parse_Service_Discovery_Indication(ServiceDiscoveryEventData->Event_Data.GATT_Service_Discovery_Indication_Data);
            }
            break;
         case QAPI_BLE_ET_GATT_SERVICE_DISCOVERY_COMPLETE_E:

            if(BLEOTAClientContext.Discovered_Control_Point_Characteristic)
            {
               /* Success if we found the characteristic. */
               qurt_signal_set(&BLEOTAClientContext.Signal, BLE_OTA_CLIENT_EVENT_FLAGS_DISCOVER_SERVICE_SUCCESS);
            }
            else
            {
               /* Failure if a service was not found. */
               qurt_signal_set(&BLEOTAClientContext.Signal, BLE_OTA_CLIENT_EVENT_FLAGS_DISCOVER_SERVICE_FAILURE);
            }
            break;
         default:
            break;
      }
   }
}

static void HandleImageQueryResponse(uint8_t *Data, uint16_t Length)
{
   if(READ_UNALIGNED_BYTE_LITTLE_ENDIAN(&((BLE_OTA_Command_Query_Image_Response_t *)Data)->Status) == BLE_OTA_STATUS_SUCCESS)
   {
      BLEOTAClientContext.QueriedImageID      = READ_UNALIGNED_DWORD_LITTLE_ENDIAN(&((BLE_OTA_Command_Query_Image_Response_t *)Data)->ImageID);
      BLEOTAClientContext.QueriedImageLength  = READ_UNALIGNED_DWORD_LITTLE_ENDIAN(&((BLE_OTA_Command_Query_Image_Response_t *)Data)->ImageLength);
      BLEOTAClientContext.QueriedImageVersion = READ_UNALIGNED_DWORD_LITTLE_ENDIAN(&((BLE_OTA_Command_Query_Image_Response_t *)Data)->Version);

      /* Success if we found the image. */
      qurt_signal_set(&BLEOTAClientContext.Signal, BLE_OTA_CLIENT_EVENT_FLAGS_QUERY_IMAGE_SUCCESS);
   }
   else
   {
      /* Signal failure if the image was not found. */
      qurt_signal_set(&BLEOTAClientContext.Signal, BLE_OTA_CLIENT_EVENT_FLAGS_QUERY_IMAGE_FAILURE);
   }
}

static void HandleImageReadResponse(uint8_t *Data, uint16_t Length)
{
   uint32_t FileOffset;
   uint32_t DataLength;
   uint32_t CurrentImageOffset;

   if(READ_UNALIGNED_BYTE_LITTLE_ENDIAN(&((BLE_OTA_Command_Read_Image_Data_Response_t *)Data)->Status) == BLE_OTA_STATUS_SUCCESS)
   {
      FileOffset = READ_UNALIGNED_DWORD_LITTLE_ENDIAN(&((BLE_OTA_Command_Read_Image_Data_Response_t *)Data)->FileOffset);
      DataLength = READ_UNALIGNED_WORD_LITTLE_ENDIAN( &((BLE_OTA_Command_Read_Image_Data_Response_t *)Data)->DataLength);

      CurrentImageOffset = BLEOTAClientContext.CurrentImageOffset;

      if((FileOffset >= CurrentImageOffset) && ((FileOffset - CurrentImageOffset + DataLength) <= BLEOTAClientContext.DataBufferLength))
      {
         /* Copy the data. */
         memcpy(&BLEOTAClientContext.DataBuffer[FileOffset - CurrentImageOffset], &((BLE_OTA_Command_Read_Image_Data_Response_t *)Data)->Data, DataLength);

         BLEOTAClientContext.BytesReceived = DataLength;

         /* Success if we found the image. */
         qurt_signal_set(&BLEOTAClientContext.Signal, BLE_OTA_CLIENT_EVENT_FLAGS_READ_IMAGE_DATA_SUCCESS);
      }
      else
      {
         /* Signal failure if we got an unexpected value. */
         qurt_signal_set(&BLEOTAClientContext.Signal, BLE_OTA_CLIENT_EVENT_FLAGS_READ_IMAGE_DATA_FAILURE);
      }
   }
   else
   {
      /* Signal failure if the image was not found. */
      qurt_signal_set(&BLEOTAClientContext.Signal, BLE_OTA_CLIENT_EVENT_FLAGS_READ_IMAGE_DATA_FAILURE);
   }
}

static void OTA_Client_Connection_Event_Callback(uint32_t BluetoothStackID, qapi_BLE_GATT_Connection_Event_Data_t *GATT_Connection_Event_Data, uint32_t CallbackParameter)
{
   uint8_t                              *Value;
   uint16_t                              Length;
   uint8_t                               Command;
   BLE_OTA_Client_Service_Information_t *ServiceInfo;

   if((BluetoothStackID) && (GATT_Connection_Event_Data))
   {
      switch(GATT_Connection_Event_Data->Event_Data_Type)
      {
         case QAPI_BLE_ET_GATT_CONNECTION_SERVER_INDICATION_E:
            if(GATT_Connection_Event_Data->Event_Data.GATT_Server_Indication_Data)
            {
               /* Confirm receipt. */
               qapi_BLE_GATT_Handle_Value_Confirmation(BluetoothStackID, GATT_Connection_Event_Data->Event_Data.GATT_Server_Indication_Data->ConnectionID, GATT_Connection_Event_Data->Event_Data.GATT_Server_Indication_Data->TransactionID);

               /* Get the service information. */
               if((ServiceInfo = FindServiceInformation(GATT_Connection_Event_Data->Event_Data.GATT_Server_Indication_Data->ConnectionID)) != NULL)
               {
                  if(GATT_Connection_Event_Data->Event_Data.GATT_Server_Indication_Data->AttributeHandle == ServiceInfo->Control_Point_Characteristic)
                  {
                     Length = GATT_Connection_Event_Data->Event_Data.GATT_Server_Indication_Data->AttributeValueLength;
                     Value  = GATT_Connection_Event_Data->Event_Data.GATT_Server_Indication_Data->AttributeValue;

                     Command = READ_UNALIGNED_BYTE_LITTLE_ENDIAN(Value);

                     /* Parse each command. */
                     switch(Command)
                     {
                        case BLE_OTA_COMMAND_QUERY_IMAGE_RESPONSE:
                           HandleImageQueryResponse(Value, Length);
                           break;
                        case BLE_OTA_COMMAND_READ_IMAGE_DATA_RESPONSE:
                           HandleImageReadResponse(Value, Length);
                           break;
                        default:
                           break;
                     }
                  }
               }
            }
            break;
         default:
            break;
      }
   }
}

static void OTA_ClientCallback(uint32_t BluetoothStackID, qapi_BLE_GATT_Client_Event_Data_t *GATT_Client_Event_Data, uint32_t CallbackParameter)
{
   if((BluetoothStackID) && (GATT_Client_Event_Data))
   {
      switch(GATT_Client_Event_Data->Event_Data_Type)
      {
         case QAPI_BLE_ET_GATT_CLIENT_WRITE_RESPONSE_E:
            // xxx
            break;
         default:
            break;
      }
   }
}

static uint32_t GetNextImageID(void)
{
   boolean_t IndexFound;
   uint16_t  Index;

   BLEOTAImageID++;

   /* Iterate BLEOTAImageID until it is unique to the list. */
   while(1)
   {
      IndexFound = FALSE;

      /* Make sure the current ID doesn't exist yet. */
      for(Index = 0; Index < BLE_OTA_MAXIMUM_NUMBER_OF_IMAGES; Index++)
      {
         if(BLEOTAImageList[Index].ImageID == BLEOTAImageID)
         {
            IndexFound = TRUE;
            break;
         }
      }

      if(IndexFound)
         BLEOTAImageID++;
      else
         break;
   }

   return BLEOTAImageID;
}

uint8_t BLE_OTA_Initialize_Service(uint32_t BluetoothStackID, uint32_t Flags, BLE_OTA_Server_Image_Data_t *ImageData, uint8_t ImageDataCount, BLE_OTA_Server_Event_Callback_t EventCallback, void *CallbackParameter, uint32_t *ServiceID)
{
   int                                    Result;
   uint8_t                                RetVal;
   uint8_t                                Index;
   uint8_t                                Index2;
   qapi_BLE_GATT_Attribute_Handle_Group_t ServiceHandleGroup;

   /* Verify that the Service is not already registered. */
   if((!(BLEOTAServerContext.Flags & BLE_OTA_SERVER_CONTEXT_FLAGS_INITIALIZED)) && ((!ImageData) || ((ImageData) && (ImageDataCount <= BLE_OTA_MAXIMUM_NUMBER_OF_IMAGES))))
   {
      RetVal = BLE_OTA_STATUS_SUCCESS;

      /* Look for incorrect filenames and non-unique Image IDs. */
      if(ImageData)
      {
         for(Index = 0; Index < ImageDataCount; Index++)
         {
            if((ImageData[Index].FileName) && (strlen(ImageData[Index].FileName) < BLE_OTA_MAXIMUM_FILE_NAME_LENGTH))
            {
               for(Index2 = Index; Index2 < ImageDataCount; Index2++)
               {
                  if((Index != Index2) && (ImageData[Index].ImageID != 0) && \
                     (ImageData[Index2].ImageID != 0) && (ImageData[Index].ImageID == ImageData[Index2].ImageID))
                  {
                     RetVal = BLE_OTA_STATUS_INVALID_PARAMETER;
                     break;
                  }
               }
            }
            else
            {
               RetVal = BLE_OTA_STATUS_INVALID_PARAMETER;
            }
         }
      }

      if(RetVal == BLE_OTA_STATUS_SUCCESS)
      {
         /* Lock the stack. */
         if(!qapi_BLE_BSC_LockBluetoothStack(BluetoothStackID))
         {
            /* Create mutex/signal. */
            qurt_mutex_init(&BLEOTAServerContext.Mutex);

            /* Initialize the Service Handle Group to 0 since we do not    */
            /* require a specific location in the service table.           */
            ServiceHandleGroup.Starting_Handle = 0;
            ServiceHandleGroup.Ending_Handle   = 0;

            /* Register the OTA Service. */
            Result = qapi_BLE_GATT_Register_Service(BluetoothStackID, QAPI_BLE_GATT_SERVICE_FLAGS_LE_SERVICE, BLE_OTA_SERVICE_ATTRIBUTE_COUNT, (qapi_BLE_GATT_Service_Attribute_Entry_t *)BLE_OTA_Service, &ServiceHandleGroup, GATT_ServerEventCallback_OTA, 0);
            if(Result > 0)
            {
               /* Save the ServiceID of the registered service. */
               BLEOTAServerContext.ServiceID          = (unsigned int)Result;
               BLEOTAServerContext.Flags             |= BLE_OTA_SERVER_CONTEXT_FLAGS_INITIALIZED;
               BLEOTAServerContext.EventCallback      = EventCallback;
               BLEOTAServerContext.CallbackParameter  = CallbackParameter;
               BLEOTANumberRegisteredImages           = 0;

               BLEOTAImageID = 0;

               *ServiceID = (unsigned int)Result;

               if(ImageData)
               {
                  for(Index = 0; Index < ImageDataCount; Index++)
                  {
                     /* Set the image ID to the provided value or the incremented one. */
                     if(ImageData[Index].ImageID)
                        BLEOTAImageList[Index].ImageID = ImageData[Index].ImageID;
                     else
                     {
                        BLEOTAImageList[Index].ImageID = GetNextImageID();
                        ImageData[Index].ImageID = BLEOTAImageList[Index].ImageID;
                     }

                     /* Copy the file name. */
                     memcpy(BLEOTAImageList[Index].FileName, ImageData[Index].FileName, strlen(ImageData[Index].FileName)+1);

                     /* Set the other variables. */
                     BLEOTAImageList[Index].Version     = ImageData[Index].Version;
                     BLEOTAImageList[Index].ImageLength = ImageData[Index].ImageLength;

                     BLEOTANumberRegisteredImages++;
                     RetVal = BLE_OTA_STATUS_SUCCESS;
                  }
               }

               /* Return success to the caller. */
               RetVal = BLE_OTA_STATUS_SUCCESS;
            }
            else
            {
               qurt_mutex_destroy(&BLEOTAServerContext.Mutex);
               RetVal = BLE_OTA_STATUS_FAILURE;
            }

            /* Unlock the stack. */
            qapi_BLE_BSC_UnLockBluetoothStack(BluetoothStackID);
        }
        else
           RetVal = BLE_OTA_STATUS_FAILURE;
      }
  }
  else
     RetVal = BLE_OTA_STATUS_FAILURE;

   return(RetVal);
}

void BLE_OTA_Cleanup_Service(uint32_t BluetoothStackID, uint32_t ServiceID)
{
   if((BluetoothStackID) && (BLEOTAServerContext.Flags & BLE_OTA_SERVER_CONTEXT_FLAGS_INITIALIZED))
   {
      /* Lock the Bluetooth stack. */
      if(!qapi_BLE_BSC_LockBluetoothStack(BluetoothStackID))
      {
         /* Destroy the mutex. */
         qurt_mutex_destroy(&BLEOTAServerContext.Mutex);

         /* Unregister the service and clear variables. */
         qapi_BLE_GATT_Un_Register_Service(BluetoothStackID, ServiceID);

         BLEOTAServerContext.ServiceID = 0;
         BLEOTAServerContext.Flags     = 0;

         /* Clear the image values. */
         memset(&BLEOTAImageList, 0, sizeof(BLEOTAImageList));

         /* Unlock the Bluetooth Stack. */
         qapi_BLE_BSC_UnLockBluetoothStack(BluetoothStackID);
      }
   }
}

uint8_t BLE_OTA_Register_Image(uint32_t BluetoothStackID, uint32_t ServiceID, BLE_OTA_Server_Image_Data_t *ImageData)
{
   uint8_t RetVal = BLE_OTA_STATUS_SUCCESS;
   uint8_t Index;
   uint8_t FileNameLength;

   if((ImageData) && (ImageData->FileName) && ((strlen(ImageData->FileName)) < BLE_OTA_MAXIMUM_FILE_NAME_LENGTH) && (BLEOTAServerContext.Flags & BLE_OTA_SERVER_CONTEXT_FLAGS_INITIALIZED))
   {
      /* Get the context mutex. */
      if(qurt_mutex_lock_timed(&BLEOTAServerContext.Mutex, QURT_TIME_WAIT_FOREVER) == QURT_EOK)
      {
         if(BLEOTANumberRegisteredImages < BLE_OTA_MAXIMUM_NUMBER_OF_IMAGES)
         {
            if(ImageData->ImageID)
            {
               /* Make sure the provided image ID is not already in use if it was specified. */
               for(Index = 0; Index < BLE_OTA_MAXIMUM_NUMBER_OF_IMAGES; Index++)
               {
                  if(BLEOTAImageList[Index].ImageID == ImageData->ImageID)
                  {
                     RetVal = BLE_OTA_STATUS_INVALID_PARAMETER;
                     break;
                  }
               }
            }

            if(RetVal == BLE_OTA_STATUS_SUCCESS)
            {
               /* Set the return value to failure in case a zero entry isn't found. */
               RetVal = BLE_OTA_STATUS_FAILURE;

               for(Index = 0; Index < BLE_OTA_MAXIMUM_NUMBER_OF_IMAGES; Index++)
               {
                  if(BLEOTAImageList[Index].ImageID == 0)
                  {
                     /* Set the image ID to the provided value or the incremented one. */
                     if(ImageData->ImageID)
                        BLEOTAImageList[Index].ImageID = ImageData->ImageID;
                     else
                     {
                        BLEOTAImageList[Index].ImageID = GetNextImageID();
                        ImageData->ImageID = BLEOTAImageList[Index].ImageID;
                     }

                     /* Copy the file name. */
                     FileNameLength = strlen(ImageData->FileName)+1;
                     memcpy(BLEOTAImageList[Index].FileName, ImageData->FileName, FileNameLength);

                     /* Set the other variables. */
                     BLEOTAImageList[Index].Version     = ImageData->Version;
                     BLEOTAImageList[Index].ImageLength = ImageData->ImageLength;

                     BLEOTANumberRegisteredImages++;
                     RetVal = BLE_OTA_STATUS_SUCCESS;
                     break;
                  }
               }
            }
         }
         else
         {
            RetVal = BLE_OTA_STATUS_OUT_OF_MEMORY;
         }

         /* Release the mutex. */
         qurt_mutex_unlock(&BLEOTAServerContext.Mutex);
      }
      else
      {
         RetVal = BLE_OTA_STATUS_OUT_OF_MEMORY;
      }
   }
   else
   {
      RetVal = BLE_OTA_STATUS_INVALID_PARAMETER;
   }

   return(RetVal);
}

uint8_t BLE_OTA_Unregister_Image(uint32_t BluetoothStackID, uint32_t ServiceID, uint32_t ImageID)
{
   uint8_t RetVal = BLE_OTA_STATUS_INVALID_PARAMETER;
   uint8_t Index;

   if((ImageID) && (BLEOTAServerContext.Flags & BLE_OTA_SERVER_CONTEXT_FLAGS_INITIALIZED))
   {
      /* Get the context mutex. */
      if(qurt_mutex_lock_timed(&BLEOTAServerContext.Mutex, QURT_TIME_WAIT_FOREVER) == QURT_EOK)
      {
         /* Find the image ID. */
         for(Index = 0; Index < BLE_OTA_MAXIMUM_NUMBER_OF_IMAGES; Index++)
         {
            if(BLEOTAImageList[Index].ImageID == ImageID)
            {
               /* Zero the entry. */
               memset(&BLEOTAImageList[Index], 0, sizeof(BLE_OTA_Image_t));

               if(BLEOTANumberRegisteredImages > 0)
                  BLEOTANumberRegisteredImages--;
               RetVal = BLE_OTA_STATUS_SUCCESS;
               break;
            }
         }

         /* Release the mutex. */
         qurt_mutex_unlock(&BLEOTAServerContext.Mutex);
      }
      else
      {
         RetVal = BLE_OTA_STATUS_OUT_OF_MEMORY;
      }
   }

   return(RetVal);
}

uint8_t BLE_OTA_Get_Registered_Images(uint32_t BluetoothStackID, uint32_t ServiceID, BLE_OTA_Server_Image_Data_t *ImageData, uint8_t *ImageDataCount)
{
   uint8_t RetVal = BLE_OTA_STATUS_INVALID_PARAMETER;
   uint8_t Index;
   uint8_t ImageDataIndex;

   if(BLEOTAServerContext.Flags & BLE_OTA_SERVER_CONTEXT_FLAGS_INITIALIZED)
   {
      /* Get the context mutex. */
      if(qurt_mutex_lock_timed(&BLEOTAServerContext.Mutex, QURT_TIME_WAIT_FOREVER) == QURT_EOK)
      {
         /* Set the image information. */
         for(Index = 0, ImageDataIndex = 0; (Index < BLE_OTA_MAXIMUM_NUMBER_OF_IMAGES) && (ImageDataIndex < *ImageDataCount); Index++)
         {
            if(BLEOTAImageList[Index].ImageID)
            {
               ImageData[ImageDataIndex].FileName    = BLEOTAImageList[Index].FileName;
               ImageData[ImageDataIndex].Version     = BLEOTAImageList[Index].Version;
               ImageData[ImageDataIndex].ImageLength = BLEOTAImageList[Index].ImageLength;
               ImageData[ImageDataIndex].ImageID     = BLEOTAImageList[Index].ImageID;
               ImageDataIndex++;
            }
         }

         *ImageDataCount = ImageDataIndex;

         RetVal = BLE_OTA_STATUS_SUCCESS;

         /* Release the mutex. */
         qurt_mutex_unlock(&BLEOTAServerContext.Mutex);
      }
      else
      {
         RetVal = BLE_OTA_STATUS_OUT_OF_MEMORY;
      }
   }

   return(RetVal);
}

uint8_t BLE_OTA_Image_Data_Response(uint32_t BluetoothStackID, uint32_t ServiceID, uint32_t ConnectionID, uint32_t TransactionID, uint32_t ImageID, uint8_t Status, uint8_t *DataBuffer, uint16_t DataLength, uint32_t FileOffset)
{
   int      Result;
   uint8_t  RetVal;
   uint8_t *PacketBuffer;

   if((BluetoothStackID) && (ServiceID) && (ConnectionID) && (BLEOTAServerContext.Flags & BLE_OTA_SERVER_CONTEXT_FLAGS_INITIALIZED) && (((Status == BLE_OTA_STATUS_SUCCESS) && (DataBuffer)) || (Status != BLE_OTA_STATUS_SUCCESS)))
   {
      /* Allocate a buffer to send. */
      PacketBuffer = (uint8_t *)malloc(BLE_OTA_COMMAND_READ_IMAGE_DATA_RESPONSE_SIZE(DataLength));

      if(PacketBuffer)
      {
         /* Format the packet. */
         ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(  &((BLE_OTA_Command_Read_Image_Data_Response_t *)PacketBuffer)->Command,    BLE_OTA_COMMAND_READ_IMAGE_DATA_RESPONSE);
         ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(  &((BLE_OTA_Command_Read_Image_Data_Response_t *)PacketBuffer)->Status,     Status);
         ASSIGN_HOST_DWORD_TO_LITTLE_ENDIAN_UNALIGNED_DWORD(&((BLE_OTA_Command_Read_Image_Data_Response_t *)PacketBuffer)->ImageID,    ImageID);
         ASSIGN_HOST_DWORD_TO_LITTLE_ENDIAN_UNALIGNED_DWORD(&((BLE_OTA_Command_Read_Image_Data_Response_t *)PacketBuffer)->FileOffset, FileOffset);

         if(Status == BLE_OTA_STATUS_SUCCESS)
         {
            ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(  &((BLE_OTA_Command_Read_Image_Data_Response_t *)PacketBuffer)->DataLength, DataLength);
            memcpy(&((BLE_OTA_Command_Read_Image_Data_Response_t *)PacketBuffer)->Data, DataBuffer, DataLength);
         }
         else
         {
            ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(  &((BLE_OTA_Command_Read_Image_Data_Response_t *)PacketBuffer)->DataLength, 0);
         }

         /* Send the indication. */
         Result = qapi_BLE_GATT_Handle_Value_Indication(BluetoothStackID, ServiceID, ConnectionID, BLE_OTA_CONTROL_POINT_ATTRIBUTE_OFFSET, BLE_OTA_COMMAND_READ_IMAGE_DATA_RESPONSE_SIZE(DataLength), PacketBuffer);

         if(Result > 0)
            RetVal = BLE_OTA_STATUS_SUCCESS;
         else
            RetVal = BLE_OTA_STATUS_FAILURE;

         /* Free the packet buffer. */
         free(PacketBuffer);
      }
      else
         RetVal = BLE_OTA_STATUS_OUT_OF_MEMORY;
   }
   else
      RetVal = BLE_OTA_STATUS_INVALID_PARAMETER;

   return(RetVal);
}

uint8_t BLE_OTA_Initialize_Client(uint32_t BluetoothStackID)
{
   uint8_t RetVal;
   int     Result;

   if(!(BLEOTAClientContext.Flags & BLE_OTA_CLIENT_CONTEXT_FLAGS_INITIALIZED))
   {
      /* Create mutex/signal. */
      qurt_mutex_init(&BLEOTAClientContext.Mutex);
      qurt_signal_init(&BLEOTAClientContext.Signal);

      /* Register for connection events. */
      Result = qapi_BLE_GATT_Register_Connection_Events(BluetoothStackID, OTA_Client_Connection_Event_Callback, 0);

      if(Result > 0)
      {
         BLEOTAClientContext.ConnectionCallbackID = Result;

         /* Flag as being initialized. */
         BLEOTAClientContext.Flags |= BLE_OTA_CLIENT_CONTEXT_FLAGS_INITIALIZED;

         RetVal = BLE_OTA_STATUS_SUCCESS;
      }
      else
      {
         qurt_mutex_destroy(&BLEOTAClientContext.Mutex);
         qurt_signal_destroy(&BLEOTAClientContext.Signal);
         RetVal = BLE_OTA_STATUS_FAILURE;
      }
   }
   else
      RetVal = BLE_OTA_STATUS_SUCCESS;

   return(RetVal);
}

void BLE_OTA_Cleanup_Client(uint32_t BluetoothStackID)
{
   if(BLEOTAClientContext.Flags & BLE_OTA_CLIENT_CONTEXT_FLAGS_INITIALIZED)
   {
      /* Cleanup mutex/signal. */
      qurt_mutex_destroy(&BLEOTAClientContext.Mutex);
      qurt_signal_destroy(&BLEOTAClientContext.Signal);

      /* Unregister for events. */
      qapi_BLE_GATT_Un_Register_Connection_Events(BluetoothStackID, BLEOTAClientContext.ConnectionCallbackID);

      BLEOTAClientContext.ConnectionCallbackID = 0;

      /* Flag as being deinitialized. */
      BLEOTAClientContext.Flags &= ~BLE_OTA_CLIENT_CONTEXT_FLAGS_INITIALIZED;
   }
}

uint8_t BLE_OTA_Discover_OTA_Service(uint32_t BluetoothStackID, uint32_t ConnectionID)
{
   int                                   Result;
   uint32_t                              CurrSignals;
   qapi_BLE_GATT_UUID_t                  OTAServiceUUID[1];
   uint8_t                               RetVal;
   qapi_BLE_NonAlignedWord_t             Buffer;
   BLE_OTA_Client_Service_Information_t *ServiceInfo;

   if(BluetoothStackID)
   {
      if(BLEOTAClientContext.Flags & BLE_OTA_CLIENT_CONTEXT_FLAGS_INITIALIZED)
      {
         /* Get the context mutex. */
         if(qurt_mutex_lock_timed(&BLEOTAClientContext.Mutex, QURT_TIME_WAIT_FOREVER) == QURT_EOK)
         {
            /* Clear any previous service information. */
            if((ServiceInfo = FindServiceInformation(ConnectionID)) != NULL)
            {
               ServiceInfo->ServerConnectionID           = 0;
               ServiceInfo->Control_Point_Characteristic = 0;
               ServiceInfo->Control_Point_CCD            = 0;
            }

            /* Clear the context variables. */
            BLEOTAClientContext.Discovered_Control_Point_Characteristic = 0;
            BLEOTAClientContext.Discovered_Control_Point_CCD            = 0;

            OTAServiceUUID[0].UUID_Type = QAPI_BLE_GU_UUID_128_E;
            BLE_OTA_ASSIGN_BLE_OTA_SERVICE_UUID_128(&OTAServiceUUID[0].UUID.UUID_128);

            /* Start the service discovery. */
            Result = qapi_BLE_GATT_Start_Service_Discovery(BluetoothStackID, ConnectionID, (sizeof(OTAServiceUUID)/sizeof(qapi_BLE_GATT_UUID_t)), OTAServiceUUID, BLE_OTA_Service_Discovery_Callback, 0);

            if(!Result)
            {
               /* Wait on the result. */
               Result = qurt_signal_wait_timed(&BLEOTAClientContext.Signal, (BLE_OTA_CLIENT_EVENT_FLAGS_DISCOVER_SERVICE_SUCCESS | BLE_OTA_CLIENT_EVENT_FLAGS_DISCOVER_SERVICE_FAILURE), QURT_SIGNAL_ATTR_CLEAR_MASK, &CurrSignals, BLE_OTA_TIMEOUT);

               /* Check if an image was found. */
               if((Result == QURT_EOK) && (CurrSignals & BLE_OTA_CLIENT_EVENT_FLAGS_DISCOVER_SERVICE_SUCCESS))
               {
                  /* Subscribe to indications. */
                  ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&Buffer, QAPI_BLE_GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_INDICATE_ENABLE);

                  Result = qapi_BLE_GATT_Write_Request(BluetoothStackID, ConnectionID, BLEOTAClientContext.Discovered_Control_Point_CCD, sizeof(Buffer), &Buffer, OTA_ClientCallback, 0);

                  if(Result > 0)
                  {
                     /* Save the service information to a new entry. */
                     if((ServiceInfo = FindServiceInformation(0)) != NULL)
                     {
                        ServiceInfo->ServerConnectionID           = ConnectionID;
                        ServiceInfo->Control_Point_Characteristic = BLEOTAClientContext.Discovered_Control_Point_Characteristic;
                        ServiceInfo->Control_Point_CCD            = BLEOTAClientContext.Discovered_Control_Point_CCD;

                        RetVal = BLE_OTA_STATUS_SUCCESS;
                     }
                     else
                        RetVal = BLE_OTA_STATUS_FAILURE;
                  }
                  else
                     RetVal = BLE_OTA_STATUS_FAILURE;
               }
               else
               {
                  RetVal = BLE_OTA_STATUS_NO_SERVICE_FOUND;
               }
            }
            else
            {
               RetVal = BLE_OTA_STATUS_FAILURE;
            }

            /* Release the mutex. */
            qurt_mutex_unlock(&BLEOTAClientContext.Mutex);
         }
         else
         {
            RetVal = BLE_OTA_STATUS_OUT_OF_MEMORY;
         }
      }
      else
      {
         RetVal = BLE_OTA_STATUS_FAILURE;
      }
   }
   else
   {
      RetVal = BLE_OTA_STATUS_INVALID_PARAMETER;
   }

   return(RetVal);
}

uint8_t BLE_OTA_Query_Image(uint32_t BluetoothStackID, uint32_t ConnectionID, const char *FileName, uint32_t *Version, uint32_t *ImageLength, uint32_t *ImageID)
{
   int                                   Result;
   uint8_t                               RetVal;
   uint8_t                              *PacketBuffer;
   uint16_t                              FileNameLength;
   uint32_t                              CurrSignals;
   BLE_OTA_Client_Service_Information_t *ServiceInfo;

   if((BluetoothStackID) && (FileName) && (ImageLength) && (ImageID) && (BLEOTAClientContext.Flags & BLE_OTA_CLIENT_CONTEXT_FLAGS_INITIALIZED))
   {
      /* Make sure the server information exists. */
      if((ServiceInfo = FindServiceInformation(ConnectionID)) != NULL)
      {
         /* Get the file name length. */
         FileNameLength = strlen(FileName)+1;

         if(FileNameLength <= BLE_OTA_MAXIMUM_FILE_NAME_LENGTH)
         {
            /* Get the context mutex. */
            if(qurt_mutex_lock_timed(&BLEOTAClientContext.Mutex, QURT_TIME_WAIT_FOREVER) == QURT_EOK)
            {
               /* Allocate a buffer to send. */
               PacketBuffer = (uint8_t *)malloc(BLE_OTA_COMMAND_QUERY_IMAGE_REQUEST_SIZE(FileNameLength));

               if(PacketBuffer)
               {
                  /* Format the packet. */
                  ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(  &((BLE_OTA_Command_Query_Image_Request_t *)PacketBuffer)->Command,         BLE_OTA_COMMAND_QUERY_IMAGE_REQUEST);
                  ASSIGN_HOST_DWORD_TO_LITTLE_ENDIAN_UNALIGNED_DWORD(&((BLE_OTA_Command_Query_Image_Request_t *)PacketBuffer)->Version,        *Version);
                  ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(  &((BLE_OTA_Command_Query_Image_Request_t *)PacketBuffer)->FileNameLength,  FileNameLength);
                  memcpy(&((BLE_OTA_Command_Query_Image_Request_t *)PacketBuffer)->FileName, FileName, FileNameLength);

                  /* Send the indication. */
                  Result = qapi_BLE_GATT_Write_Request(BluetoothStackID, ConnectionID, ServiceInfo->Control_Point_Characteristic, BLE_OTA_COMMAND_QUERY_IMAGE_REQUEST_SIZE(FileNameLength), PacketBuffer, OTA_ClientCallback, 0);

                  if(Result > 0)
                  {
                     /* Wait on the result. */
                     Result = qurt_signal_wait_timed(&BLEOTAClientContext.Signal, (BLE_OTA_CLIENT_EVENT_FLAGS_QUERY_IMAGE_SUCCESS | BLE_OTA_CLIENT_EVENT_FLAGS_QUERY_IMAGE_FAILURE), QURT_SIGNAL_ATTR_CLEAR_MASK, &CurrSignals, BLE_OTA_TIMEOUT);

                     /* Check if an image was found. */
                     if((Result == QURT_EOK) && (CurrSignals & BLE_OTA_CLIENT_EVENT_FLAGS_QUERY_IMAGE_SUCCESS))
                     {
                        /* Return success. */
                        RetVal = BLE_OTA_STATUS_SUCCESS;

                        *ImageLength = BLEOTAClientContext.QueriedImageLength;
                        *ImageID     = BLEOTAClientContext.QueriedImageID;
                        *Version     = BLEOTAClientContext.QueriedImageVersion;
                     }
                     else
                     {
                        if(CurrSignals & BLE_OTA_CLIENT_EVENT_FLAGS_QUERY_IMAGE_FAILURE)
                           RetVal = BLE_OTA_STATUS_NO_SERVICE_FOUND;
                        else
                           RetVal = BLE_OTA_STATUS_FAILURE;
                     }
                  }
                  else
                     RetVal = BLE_OTA_STATUS_FAILURE;

                  /* Free the buffer. */
                  free(PacketBuffer);
               }
               else
                  RetVal = BLE_OTA_STATUS_OUT_OF_MEMORY;

               /* Release the mutex. */
               qurt_mutex_unlock(&BLEOTAClientContext.Mutex);
            }
            else
               RetVal = BLE_OTA_STATUS_FAILURE;
         }
         else
            RetVal = BLE_OTA_STATUS_INVALID_PARAMETER;
      }
      else
         RetVal = BLE_OTA_STATUS_INVALID_PARAMETER;
   }
   else
      RetVal = BLE_OTA_STATUS_INVALID_PARAMETER;

   return(RetVal);
}

static uint8_t SendImageDataRequest(uint32_t BluetoothStackID, BLE_OTA_Client_Service_Information_t *ServiceInfo, uint32_t ImageID, uint32_t DataLength, uint32_t FileOffset)
{
   uint8_t  *PacketBuffer;
   int       Result;
   uint8_t   RetVal;

   /* Allocate a buffer to send. */
   PacketBuffer = (uint8_t *)malloc(BLE_OTA_COMMAND_READ_IMAGE_DATA_REQUEST_SIZE);

   if(PacketBuffer)
   {
      /* Format the packet. */
      ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(  &((BLE_OTA_Command_Read_Image_Data_Request_t *)PacketBuffer)->Command,    BLE_OTA_COMMAND_READ_IMAGE_DATA_REQUEST);
      ASSIGN_HOST_DWORD_TO_LITTLE_ENDIAN_UNALIGNED_DWORD(&((BLE_OTA_Command_Read_Image_Data_Request_t *)PacketBuffer)->ImageID,    ImageID);
      ASSIGN_HOST_DWORD_TO_LITTLE_ENDIAN_UNALIGNED_DWORD(&((BLE_OTA_Command_Read_Image_Data_Request_t *)PacketBuffer)->FileOffset, FileOffset);
      ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(  &((BLE_OTA_Command_Read_Image_Data_Request_t *)PacketBuffer)->DataLength, DataLength);

      /* Send the write request. */
      Result = qapi_BLE_GATT_Write_Request(BluetoothStackID, ServiceInfo->ServerConnectionID, ServiceInfo->Control_Point_Characteristic, BLE_OTA_COMMAND_READ_IMAGE_DATA_REQUEST_SIZE, PacketBuffer, OTA_ClientCallback, 0);

      if(Result > 0)
         RetVal = BLE_OTA_STATUS_SUCCESS;
      else
         RetVal = BLE_OTA_STATUS_FAILURE;

      /* Free the buffer. */
      free(PacketBuffer);

   }
   else
      RetVal = BLE_OTA_STATUS_OUT_OF_MEMORY;

   return(RetVal);
}

static uint32_t GetImageDataResponse(void)
{
   int       Result;
   uint32_t  CurrSignals;
   uint32_t  RetVal;

   /* Wait on the result. */
   Result = qurt_signal_wait_timed(&BLEOTAClientContext.Signal, (BLE_OTA_CLIENT_EVENT_FLAGS_READ_IMAGE_DATA_SUCCESS | BLE_OTA_CLIENT_EVENT_FLAGS_READ_IMAGE_DATA_FAILURE), QURT_SIGNAL_ATTR_CLEAR_MASK, &CurrSignals, BLE_OTA_TIMEOUT);

   /* Check if an image was found. */
   if((Result == QURT_EOK) && (CurrSignals & BLE_OTA_CLIENT_EVENT_FLAGS_READ_IMAGE_DATA_SUCCESS))
   {
      /* Return success. */
      RetVal = BLEOTAClientContext.BytesReceived;
   }
   else
   {
      RetVal = 0;
   }

   return(RetVal);
}

uint8_t BLE_OTA_Read_Image_Data(uint32_t BluetoothStackID, uint32_t ConnectionID, uint32_t ImageID, uint8_t *DataBuffer, uint32_t *DataLength, uint32_t FileOffset)
{
   uint8_t                               RetVal;
   uint32_t                              BytesRemaining;
   uint32_t                              RequestDataLength;
   uint32_t                              ResponseDataLength;
   BLE_OTA_Client_Service_Information_t *ServiceInfo;

   if((BluetoothStackID) && (ImageID) && (DataBuffer) && (DataLength) && (BLEOTAClientContext.Flags & BLE_OTA_CLIENT_CONTEXT_FLAGS_INITIALIZED))
   {
      /* Make sure the server information exists. */
      if((ServiceInfo = FindServiceInformation(ConnectionID)) != NULL)
      {
         /* Get the context mutex. */
         if(qurt_mutex_lock_timed(&BLEOTAClientContext.Mutex, QURT_TIME_WAIT_FOREVER) == QURT_EOK)
         {
            /* Initialize variables. */
            BLEOTAClientContext.CurrentImageOffset =  FileOffset;
            BLEOTAClientContext.DataBuffer         =  DataBuffer;
            BLEOTAClientContext.DataBufferLength   = *DataLength;
            BytesRemaining                         = *DataLength;

            if(qapi_BLE_GATT_Query_Connection_MTU(BluetoothStackID, ConnectionID, &ServiceInfo->MTU) == 0)
            {
               while(BytesRemaining)
               {
                  /* Make sure the response will be equal to the MTU size when possible. */
                  if(BLE_OTA_COMMAND_READ_IMAGE_DATA_RESPONSE_SIZE(BytesRemaining) > (ServiceInfo->MTU-3))
                     RequestDataLength = (ServiceInfo->MTU-3) - BLE_OTA_COMMAND_READ_IMAGE_DATA_RESPONSE_SIZE(0);
                  else
                     RequestDataLength = BytesRemaining;

                  /* Send a data request. */
                  if(SendImageDataRequest(BluetoothStackID, ServiceInfo, ImageID, RequestDataLength, (BLEOTAClientContext.CurrentImageOffset + (*DataLength - BytesRemaining))) == BLE_OTA_STATUS_SUCCESS)
                  {
                     /* Wait for the response. */
                     if((ResponseDataLength = GetImageDataResponse()) > 0)
                     {
                        BytesRemaining -= RequestDataLength;
                     }
                     else
                     {
                        RetVal = BLE_OTA_STATUS_FAILURE;
                        break;
                     }
                  }
                  else
                  {
                     RetVal = BLE_OTA_STATUS_OUT_OF_MEMORY;
                     break;
                  }
               }

               /* Flag success if all data was read. */
               if(!BytesRemaining)
                  RetVal = BLE_OTA_STATUS_SUCCESS;

               /* Decrement the number of bytes read. */
               *DataLength = (*DataLength - BytesRemaining);
            }
            else
               RetVal = BLE_OTA_STATUS_FAILURE;

            /* Release the mutex. */
            qurt_mutex_unlock(&BLEOTAClientContext.Mutex);
         }
         else
            RetVal = BLE_OTA_STATUS_FAILURE;
      }
      else
         RetVal = BLE_OTA_STATUS_INVALID_PARAMETER;
   }
   else
      RetVal = BLE_OTA_STATUS_INVALID_PARAMETER;

   return(RetVal);
}
