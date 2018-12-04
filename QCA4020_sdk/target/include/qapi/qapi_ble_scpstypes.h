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

#ifndef __QAPI_BLE_SCPS_TYPES_H__
#define __QAPI_BLE_SCPS_TYPES_H__

#include "./qapi_ble_bttypes.h" /* Bluetooth Type Definitions.          */

   /* The following MACRO is a utility MACRO that assigns the Scan      */
   /* Parameter Service 16 bit UUID to the specified qapi_BLE_UUID_16_t */
   /* variable.  This MACRO accepts one parameter which is a pointer to */
   /* a qapi_BLE_UUID_16_t variable that is to receive the SCPS UUID    */
   /* Constant value.                                                   */
   /* * NOTE * The UUID will be assigned into the qapi_BLE_UUID_16_t    */
   /*          variable in Little-Endian format.                        */
#define QAPI_BLE_SCPS_ASSIGN_SCPS_SERVICE_UUID_16(_x)             QAPI_BLE_ASSIGN_BLUETOOTH_UUID_16(*((qapi_BLE_UUID_16_t *)(_x)), 0x18, 0x13)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined SCPS Service UUID in UUID16 form.  This    */
   /* MACRO only returns whether the qapi_BLE_UUID_16_t variable is     */
   /* equal to the SCPS Service UUID (MACRO returns boolean result) NOT */
   /* less than/greater than.  The first parameter is the               */
   /* qapi_BLE_UUID_16_t variable to compare to the SCPS Service UUID.  */
#define QAPI_BLE_SCPS_COMPARE_SCPS_SERVICE_UUID_TO_UUID_16(_x)    QAPI_BLE_COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x18, 0x13)

   /* The following defines the Scan Parameter Service UUID that is     */
   /* used when building the SCPS Service Table.                        */
#define QAPI_BLE_SCPS_SERVICE_BLUETOOTH_UUID_CONSTANT             { 0x13, 0x18 }

   /* The following MACRO is a utility MACRO that assigns the SCPS Scan */
   /* Interval Window Characteristic 16 bit UUID to the specified       */
   /* qapi_BLE_UUID_16_t variable.  This MACRO accepts one parameter    */
   /* which is the qapi_BLE_UUID_16_t variable that is to receive the   */
   /* SCPS Scan Parameter UUID Constant value.                          */
   /* * NOTE * The UUID will be assigned into the qapi_BLE_UUID_16_t    */
   /*          variable in Little-Endian format.                        */
#define QAPI_BLE_SCPS_ASSIGN_SCAN_INTERVAL_WINDOW_UUID_16(_x)                      QAPI_BLE_ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0x4F)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined SCPS Scan Interval Window UUID in UUID16   */
   /* form.  This MACRO only returns whether the qapi_BLE_UUID_16_t     */
   /* variable is equal to the Scan Interval Window UUID (MACRO returns */
   /* boolean result) NOT less than/greater than.  The first parameter  */
   /* is the qapi_BLE_UUID_16_t variable to compare to the SCPS Scan    */
   /* Inerval Window UUID                                               */
#define QAPI_BLE_SCPS_COMPARE_SCAN_INTERVAL_WINDOW_UUID_TO_UUID_16(_x)             QAPI_BLE_COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0x4F)

   /* The following defines the SCPS Scan Interval Window               */
   /* Characteristic UUID that is used when building the SCPS Service   */
   /* Table.                                                            */
#define QAPI_BLE_SCPS_SCAN_INTERVAL_WINDOW_CHARACTERISTIC_BLUETOOTH_UUID_CONSTANT  { 0x4F, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the SCPS      */
   /* Scan_Refresh information Characteristic 16 bit UUID to the        */
   /* specified qapi_BLE_UUID_16_t variable.  This MACRO accepts one    */
   /* parameter which is the qapi_BLE_UUID_16_t variable that is to     */
   /* receive the SCPS Scan Refresh information UUID Constant value.    */
   /* * NOTE * The UUID will be assigned into the qapi_BLE_UUID_16_t    */
   /*          variable in Little-Endian format.                        */
#define QAPI_BLE_SCPS_ASSIGN_SCAN_REFRESH_UUID_16(_x)                      QAPI_BLE_ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0x31)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined SCPS Scan Refresh Information UUID in      */
   /* UUID16 form.  This MACRO only returns whether the                 */
   /* qapi_BLE_UUID_16_t variable is equal to the Scan Refresh          */
   /* Information UUID (MACRO returns boolean result) NOT less          */
   /* than/greater than.  The first parameter is the qapi_BLE_UUID_16_t */
   /* variable to compare to the SCPS Scan Refresh UUID                 */
#define QAPI_BLE_SCPS_COMPARE_SCAN_REFRESH_UUID_TO_UUID_16(_x)             QAPI_BLE_COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0x31)

  /* The following defines the SCPS Scan Refresh information            */
  /* Characteristic UUID that is used when building the SCPS Service    */
  /* Table.                                                             */
#define QAPI_BLE_SCPS_SCAN_REFRESH_CHARACTERISTIC_BLUETOOTH_UUID_CONSTANT  { 0x31, 0x2A }

  /* The following macro defines the valid Scan Refresh value that may  */
  /* be set as a value for the Scan Refresh Value field of Scan         */
  /*  Parameter Characteristic.                                         */
#define QAPI_BLE_SCPS_SCAN_REFRESH_VALUE_SERVER_REQUIRES_REFRESH  0x00

  /* The followng defines the format of                                 */
  /* qapi_BLE_SCPS_Scan_Interval_Window_s The first member specifies    */
  /* information on LE_Scan_Interval.  The second member specifies      */
  /* LE_Scan_Window .                                                   */
typedef __QAPI_BLE_PACKED_STRUCT_BEGIN__ struct qapi_BLE_SCPS_Scan_Interval_Window_s
{
   qapi_BLE_NonAlignedWord_t LE_Scan_Interval;
   qapi_BLE_NonAlignedWord_t LE_Scan_Window;
}__QAPI_BLE_PACKED_STRUCT_END__ qapi_BLE_SCPS_Scan_Interval_Window_t;

#define QAPI_BLE_SCPS_SCAN_INTERVAL_WINDOW_SIZE                   (sizeof(qapi_BLE_SCPS_Scan_Interval_Window_t))

  /* The following defines the SCPS GATT Service Flags MASK that should */
  /* be passed into qapi_BLE_GATT_Register_Service() function when the  */
  /* SCPS Service is registered.                                        */
#define QAPI_BLE_SCPS_SERVICE_FLAGS                               (QAPI_BLE_GATT_SERVICE_FLAGS_LE_SERVICE)

#endif
