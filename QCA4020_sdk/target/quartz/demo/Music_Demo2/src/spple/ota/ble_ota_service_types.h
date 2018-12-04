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

#ifndef __BLE_OTA_SERVICE_TYPES__
#define __BLE_OTA_SERVICE_TYPES__

#if !defined(__PACKED_STRUCT_BEGIN__) && !defined(__PACKED_STRUCT_END__)
      /* Packed structure macros. */
   #if (defined(__CC_ARM) || defined(__GNUC__))
      /* Packed macros for ARM and GCC. */
      #define __PACKED_STRUCT_BEGIN__
      #define __PACKED_STRUCT_END__       __attribute__ ((packed))
   #elif (defined(__ICCARM__))
      /* Packed macros for IAR. */
      #define __PACKED_STRUCT_BEGIN__     __packed
      #define __PACKED_STRUCT_END__
   #else
      #define __PACKED_STRUCT_BEGIN__
      #define __PACKED_STRUCT_END__
   #endif
#endif

#define BLE_OTA_COMMAND_QUERY_IMAGE_REQUEST           0x01
#define BLE_OTA_COMMAND_QUERY_IMAGE_RESPONSE          0x02
#define BLE_OTA_COMMAND_READ_IMAGE_DATA_REQUEST       0x03
#define BLE_OTA_COMMAND_READ_IMAGE_DATA_RESPONSE      0x04

typedef __PACKED_STRUCT_BEGIN__ struct BLE_OTA_Command_Query_Image_Request_s
{
   uint8_t  Command;
   uint32_t Version;
   uint8_t  FileNameLength;
   uint8_t  FileName[];
} __PACKED_STRUCT_END__ BLE_OTA_Command_Query_Image_Request_t;

#define BLE_OTA_COMMAND_QUERY_IMAGE_REQUEST_SIZE(FileNameLength)           (sizeof(BLE_OTA_Command_Query_Image_Request_t) + FileNameLength)

typedef __PACKED_STRUCT_BEGIN__ struct BLE_OTA_Command_Query_Image_Response_s
{
   uint8_t  Command;
   uint8_t  Status;
   uint32_t ImageID;
   uint32_t ImageLength;
   uint32_t Version;
} __PACKED_STRUCT_END__ BLE_OTA_Command_Query_Image_Response_t;

#define BLE_OTA_COMMAND_QUERY_IMAGE_RESPONSE_SIZE                          (sizeof(BLE_OTA_Command_Query_Image_Response_t))

typedef __PACKED_STRUCT_BEGIN__ struct BLE_OTA_Command_Read_Image_Data_Request_s
{
   uint8_t  Command;
   uint32_t ImageID;
   uint32_t FileOffset;
   uint16_t DataLength;
} __PACKED_STRUCT_END__ BLE_OTA_Command_Read_Image_Data_Request_t;

#define BLE_OTA_COMMAND_READ_IMAGE_DATA_REQUEST_SIZE                       (sizeof(BLE_OTA_Command_Read_Image_Data_Request_t))

typedef __PACKED_STRUCT_BEGIN__ struct BLE_OTA_Command_Read_Image_Data_s
{
   uint8_t  Command;
   uint8_t  Status;
   uint32_t ImageID;
   uint32_t FileOffset;
   uint16_t DataLength;
   uint8_t  Data[];
} __PACKED_STRUCT_END__ BLE_OTA_Command_Read_Image_Data_Response_t;

#define BLE_OTA_COMMAND_READ_IMAGE_DATA_RESPONSE_SIZE(DataLength)          (sizeof(BLE_OTA_Command_Read_Image_Data_Response_t) + DataLength)

#endif // __BLE_OTA_SERVICE_TYPES__
