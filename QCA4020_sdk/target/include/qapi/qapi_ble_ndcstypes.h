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

#ifndef __QAPI_BLE_NDCS_TYPES_H__
#define __QAPI_BLE_NDCS_TYPES_H__

#include "./qapi_ble_bttypes.h" /* Bluetooth Type Definitions.          */

   /* The following MACRO is a utility MACRO that assigns the Current   */
   /* Time Service 16 bit UUID to the specified qapi_BLE_UUID_16_t      */
   /* variable.  This MACRO accepts one parameter which is a pointer to */
   /* a qapi_BLE_UUID_16_t variable that is to receive the NDCS UUID    */
   /* Constant value.                                                   */
   /* * NOTE * The UUID will be assigned into the qapi_BLE_UUID_16_t    */
   /*          variable in Little-Endian format.                        */
#define QAPI_BLE_NDCS_ASSIGN_NDCS_SERVICE_UUID_16(_x)           QAPI_BLE_ASSIGN_BLUETOOTH_UUID_16(*((qapi_BLE_UUID_16_t *)(_x)), 0x18, 0x07)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined NDCS Service UUID in UUID16 form.  This    */
   /* MACRO only returns whether the qapi_BLE_UUID_16_t variable is     */
   /* equal to the NDCS Service UUID (MACRO returns boolean result) NOT */
   /* less than/greater than.  The first parameter is the               */
   /* qapi_BLE_UUID_16_t variable to compare to the NDCS Service UUID.  */
#define QAPI_BLE_NDCS_COMPARE_NDCS_SERVICE_UUID_TO_UUID_16(_x)  QAPI_BLE_COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x18, 0x07)

   /* The following defines the Next DST Change Service UUID that is    */
   /* used when building the NDCS Service Table.                        */
#define QAPI_BLE_NDCS_SERVICE_BLUETOOTH_UUID_CONSTANT           { 0x07, 0x18 }

   /* The following MACRO is a utility MACRO that assigns the NDCS Time */
   /* with DST Characteristic 16 bit UUID to the specified              */
   /* qapi_BLE_UUID_16_t variable.  This MACRO accepts one parameter    */
   /* which is the qapi_BLE_UUID_16_t variable that is to receive the   */
   /* NDCS Time with DST UUID Constant value.                           */
   /* * NOTE * The UUID will be assigned into the qapi_BLE_UUID_16_t    */
   /*          variable in Little-Endian format.                        */
#define QAPI_BLE_NDCS_ASSIGN_TIME_WITH_DST_UUID_16(_x)                      QAPI_BLE_ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0x11)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined NDCS Time with DST UUID in UUID16 form.    */
   /* This MACRO only returns whether the qapi_BLE_UUID_16_t variable is*/
   /* equal to the Time With DST UUID (MACRO returns boolean result) NOT*/
   /* less than/greater than.  The first parameter is the               */
   /* qapi_BLE_UUID_16_t variable to compare to the NDCS Time With DST  */
   /* UUID                                                              */
#define QAPI_BLE_NDCS_COMPARE_NDCS_TIME_WITH_DST_UUID_TO_UUID_16(_x)        QAPI_BLE_COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0x11)

   /* The following defines the NDCS Time with DST Characteristic UUID  */
   /* that is used when building the NDCS Service Table.                */
#define QAPI_BLE_NDCS_TIME_WITH_DST_CHARACTERISTIC_BLUETOOTH_UUID_CONSTANT  { 0x11, 0x2A }

   /* The following define the valid Daylight Savings offsets that may  */
   /* be used.                                                          */
#define QAPI_BLE_NDCS_DST_OFFSET_STANDARD_TIME            0x00
#define QAPI_BLE_NDCS_DST_OFFSET_HALF_HOUR_DAYLIGHT_TIME  0x02
#define QAPI_BLE_NDCS_DST_OFFSET_DAYLIGHT_TIME            0x04
#define QAPI_BLE_NDCS_DST_OFFSET_DOUBLE_DAYLIGHT_TIME     0x08
#define QAPI_BLE_NDCS_DST_OFFSET_NOT_KNOWN                0xFF

   /* The following MACRO is a utility MACRO that exists to valid that a*/
   /* specified Date Time is valid.  The only parameter to this function*/
   /* is the qapi_BLE_NDCS_Date_Time_t structure to valid.  This MACRO  */
   /* returns TRUE if the Date Time is valid or FALSE otherwise.        */
#define QAPI_BLE_NDCS_DATE_TIME_VALID(_x)                ((QAPI_BLE_GATT_DATE_TIME_VALID_YEAR(((_x)).Year)) && (QAPI_BLE_GATT_DATE_TIME_VALID_MONTH(((_x)).Month)) && (QAPI_BLE_GATT_DATE_TIME_VALID_DAY(((_x)).Day)) && (QAPI_BLE_GATT_DATE_TIME_VALID_HOURS(((_x)).Hours)) && (QAPI_BLE_GATT_DATE_TIME_VALID_MINUTES(((_x)).Minutes)) && (QAPI_BLE_GATT_DATE_TIME_VALID_SECONDS(((_x)).Seconds)))

   /* The following structure defines the format of                     */
   /* qapi_BLE_NDCS_Time_With_Dst_s This is used to represent Time with */
   /* Dst The first member specifies date time The Second member        */
   /* specifies dst_offset                                              */
typedef __QAPI_BLE_PACKED_STRUCT_BEGIN__ struct qapi_BLE_NDCS_Time_With_Dst_s
{
   qapi_BLE_GATT_Date_Time_Characteristic_t Date_Time;
   qapi_BLE_NonAlignedByte_t                Dst_Offset;
}__QAPI_BLE_PACKED_STRUCT_END__  qapi_BLE_NDCS_Time_With_Dst_t;

#define QAPI_BLE_NDCS_TIME_WITH_DST_SIZE                 (sizeof(qapi_BLE_NDCS_Time_With_Dst_t))

   /* The following MACRO is a utility MACRO that exists to valid that a*/
   /* specified DST Offset is valid.  The only parameter to this        */
   /* function is the DSTOFFSET structure to valid.  This MACRO returns */
   /* TRUE if the DST Offset is valid or FALSE otherwise.               */
#define QAPI_BLE_NDCS_DST_OFFSET_VALID(_x)               ((((uint8_t)(_x)) >= QAPI_BLE_NDCS_DST_OFFSET_STANDARD_TIME) && (((uint8_t)(_x)) <= QAPI_BLE_NDCS_DST_OFFSET_DOUBLE_DAYLIGHT_TIME))
#define QAPI_BLE_NDCS_DST_OFFSET_DATA_SIZE               (QAPI_BLE_NON_ALIGNED_BYTE_SIZE)

   /* The following defines the CTS GATT Service Flags MASK that should */
   /* be passed into qapi_BLE_GATT_Register_Service() when the CTS      */
   /* Service is registered.                                            */
#define QAPI_BLE_NDCS_SERVICE_FLAGS                      (QAPI_BLE_GATT_SERVICE_FLAGS_LE_SERVICE)

#endif
