/*
 * Copyright (c) 2016-2018 Qualcomm Technologies, Inc.
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

#ifndef __BLE_OTA_SERVICE__
#define __BLE_OTA_SERVICE__

#define QAPI_USE_BLE

#include "qapi.h"

/* BLE OTA status codes. */
#define BLE_OTA_STATUS_SUCCESS                      (0x00)
#define BLE_OTA_STATUS_NO_SERVICE_FOUND             (0x01)
#define BLE_OTA_STATUS_NO_UPDATE_AVAILABLE          (0x02)
#define BLE_OTA_STATUS_INVALID_PARAMETER            (0x03)
#define BLE_OTA_STATUS_OUT_OF_MEMORY                (0x04)
#define BLE_OTA_STATUS_FAILURE                      (0x05)

/* BLE OTA configuration. */
#define BLE_OTA_MAXIMUM_NUMBER_OF_IMAGES            (5)
#define BLE_OTA_MAXIMUM_FILE_NAME_LENGTH            (50)
#define BLE_OTA_MAXIMUM_NUMBER_SERVERS              (5)
#define BLE_OTA_MAXIMUM_NUMBER_CLIENTS              (5)

/* Structure for describing an image registered with an OTA server. */
typedef struct BLE_OTA_Server_Image_Data_s
{
   char     *FileName;
   uint32_t  Version;
   uint32_t  ImageLength;
   uint32_t  ImageID;
} BLE_OTA_Server_Image_Data_t;

/* BLE OTA server event types. */
typedef enum
{
   BLE_OTA_SERVER_EVENT_TYPE_IMAGE_DATA_REQUEST_E
} BLE_OTA_Server_Event_Type_t;

/* BLE OTA server image data request event data structure. */
typedef struct BLE_OTA_Server_Image_Data_Request_Event_Data_s
{
   uint32_t ConnectionID;
   uint32_t TransactionID;
   uint32_t ImageID;
   uint32_t FileOffset;
   uint32_t DataLength;
} BLE_OTA_Server_Image_Data_Request_Event_Data_t;

/* BLE OTA server event data structure. */
typedef struct BLE_OTA_Server_Event_Data_s
{
   BLE_OTA_Server_Event_Type_t Event_Data_Type;
   union
   {
      BLE_OTA_Server_Image_Data_Request_Event_Data_t  *BLE_OTA_Server_Image_Data_Request_Event_Data;
   } Event_Data;
} BLE_OTA_Server_Event_Data_t;

/* Type definition of a BLE OTA server callback. */
typedef void (*BLE_OTA_Server_Event_Callback_t)(uint32_t BluetoothStackID, BLE_OTA_Server_Event_Data_t *BLE_OTA_Server_Event_Data, void *CallbackParameter);

/*
   @brief Initializes an OTA service.

   @param BluetoothStackID  is the stack ID used for this service.

   @param Flags             optionally indicates service parameters.

   @param ImageData         is the optional pointer to an array of
                            images to register with the service.

   @param ImageDataCount    indicates the length of the ImageData
                            array.

   @param EventCallback     is the callback for this service.

   @param CallbackParameter is the parameter that will be passed to the
                            event callback.

   @param ServiceID         is a pointer that will hold the ID of the
                            new service upon success.

   @return BLE_OTA_STATUS
*/
uint8_t BLE_OTA_Initialize_Service(uint32_t BluetoothStackID, uint32_t Flags, BLE_OTA_Server_Image_Data_t *ImageData, uint8_t ImageDataCount, BLE_OTA_Server_Event_Callback_t EventCallback, void *CallbackParameter, uint32_t *ServiceID);

/*
   @brief Deinitializes an OTA service.

   @param BluetoothStackID  is the stack ID used for this service.

   @param ServiceID         is the ID of the service, returned when it
                            was registered.

   @return BLE_OTA_STATUS
*/
void    BLE_OTA_Cleanup_Service(uint32_t BluetoothStackID, uint32_t ServiceID);

/*
   @brief Registers an image with an OTA service.

   @param BluetoothStackID  is the stack ID used for this service.

   @param ServiceID         is the ID of the service, returned when it
                            was registered.

   @param ImageData         is a pointer to the image data to register.
                            The ImageID may either be zero or a unique ID.
                            When provided as zero, the ImageID in this
                            structure will be set to a unique value upon
                            successful image registration.

   @return BLE_OTA_STATUS
*/
uint8_t BLE_OTA_Register_Image(uint32_t BluetoothStackID, uint32_t ServiceID, BLE_OTA_Server_Image_Data_t *ImageData);

/*
   @brief Unregisters an image with an OTA service.

   @param BluetoothStackID  is the stack ID used for this service.

   @param ServiceID         is the ID of the service, returned when it
                            was registered.

   @param ImageID           is the ID of the image to unregister.

   @return BLE_OTA_STATUS
*/
uint8_t BLE_OTA_Unregister_Image(uint32_t BluetoothStackID, uint32_t ServiceID, uint32_t ImageID);

/*
   @brief Gets a list of images registered with an OTA service.

   @param BluetoothStackID  is the stack ID used for this service.

   @param ServiceID         is the ID of the service, returned when it
                            was registered.

   @param ImageData         is a pointer to an array of image data whose
                            values are to be set by a successful call to
                            this function.

   @param ImageDataCount    is a pointer holding the length of the
                            ImageData array. Upon function success, this
                            value will contain the number of images
                            copied into the array.

   @return BLE_OTA_STATUS
*/
uint8_t BLE_OTA_Get_Registered_Images(uint32_t BluetoothStackID, uint32_t ServiceID, BLE_OTA_Server_Image_Data_t *ImageData, uint8_t *ImageDataCount);

/*
   @brief Sends a response to a BLE_OTA_SERVER_EVENT_TYPE_IMAGE_DATA_REQUEST_E event.

   @param BluetoothStackID  is the stack ID used for this service.

   @param ServiceID         is the ID of the service, returned when it
                            was registered.

   @param ConnectionID      is the connection ID of the remote device.

   @param TransactionID     is the transaction ID of the data request.

   @param ImageID           is the ID of the image for which data was
                            requested.

   @param Status            is the status of the response, one of type
                            BLE_OTA_STATUS_XXX.

   @param DataBuffer        is a buffer holding the response data if
                            the request was successful.

   @param DataLength        is the length of DataBuffer if the
                            request was successful.

   @param FileOffset        is the image file offset if the request
                            was successful.

   @return BLE_OTA_STATUS
*/
uint8_t BLE_OTA_Image_Data_Response(uint32_t BluetoothStackID, uint32_t ServiceID, uint32_t ConnectionID, uint32_t TransactionID, uint32_t ImageID, uint8_t Status, uint8_t *DataBuffer, uint16_t DataLength, uint32_t FileOffset);

/*
   @brief Initializes an OTA client.

   @param BluetoothStackID  is the stack ID used for the client.

   @return BLE_OTA_STATUS
*/
uint8_t BLE_OTA_Initialize_Client(uint32_t BluetoothStackID);

/*
   @brief Deinitializes an OTA client.

   @param BluetoothStackID  is the stack ID used for the client.
*/
void BLE_OTA_Cleanup_Client(uint32_t BluetoothStackID);

/*
   @brief Discovers an OTA service.

   @param BluetoothStackID  is the stack ID used for the client.

   @param ConnectionID      is the connection ID of the remote device.

   @return BLE_OTA_STATUS
*/
uint8_t BLE_OTA_Discover_OTA_Service(uint32_t BluetoothStackID, uint32_t ConnectionID);

/*
   @brief Queries for an image on an OTA service.

   @param BluetoothStackID  is the stack ID used for the client.

   @param ConnectionID      is the connection ID of the remote device.

   @param FileName          is the name of the file to query.

   @param Version           is a pointer to the current image version on
                            the client. Upon function success it will
                            contain the version of the server's image.

   @param ImageLength       is a pointer that will contain the image
                            length upon function success.

   @param ImageID           is a pointer that will contain the image
                            ID upon function success.

   @return BLE_OTA_STATUS
*/
uint8_t BLE_OTA_Query_Image(uint32_t BluetoothStackID, uint32_t ConnectionID, const char *FileName, uint32_t *Version, uint32_t *ImageLength, uint32_t *ImageID);

/*
   @brief Reads image data from an OTA service.

   @param BluetoothStackID  is the stack ID used for the client.

   @param ConnectionID      is the connection ID of the remote device.

   @param ImageID           is the ID of the image to read, returned
                            previously from a successful image query.

   @param DataBuffer        is the buffer to fill with image data.

   @param DataLength        is a pointer to the length of DataBuffer.
                            Upon function success, DataLength will
                            indicate the number of bytes copied to
                            DataBuffer.

   @param FileOffset        is the file offset from which to read image
                            data.

   @return BLE_OTA_STATUS
*/
uint8_t BLE_OTA_Read_Image_Data(uint32_t BluetoothStackID, uint32_t ConnectionID, uint32_t ImageID, uint8_t *DataBuffer, uint32_t *DataLength, uint32_t FileOffset);

#endif // __BLE_OTA_SERVICE__
