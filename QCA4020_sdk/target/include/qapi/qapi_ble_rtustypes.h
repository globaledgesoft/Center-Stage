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

#ifndef __QAPI_BLE_RTUS_TYPES_H__
#define __QAPI_BLE_RTUS_TYPES_H__

#include "./qapi_ble_bttypes.h" /* Bluetooth Type Definitions.          */

   /* The following MACRO is a utility MACRO that assigns the Reference */
   /* Time Update Service 16 bit UUID to the specified                  */
   /* qapi_BLE_UUID_16_t variable.  This MACRO accepts one parameter    */
   /* which is a pointer to a qapi_BLE_UUID_16_t variable that is to    */
   /* receive the RTUS UUID Constant value.                             */
   /* * NOTE * The UUID will be assigned into the qapi_BLE_UUID_16_t    */
   /*          variable in Little-Endian format.                        */
#define QAPI_BLE_RTUS_ASSIGN_RTUS_SERVICE_UUID_16(_x)             QAPI_BLE_ASSIGN_BLUETOOTH_UUID_16(*((qapi_BLE_UUID_16_t *)(_x)), 0x18, 0x06)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined RTUS Service UUID in UUID16 form.  This    */
   /* MACRO only returns whether the qapi_BLE_UUID_16_t variable is     */
   /* equal to the RTUS Service UUID (MACRO returns boolean result) NOT */
   /* less than/greater than.  The first parameter is the               */
   /* qapi_BLE_UUID_16_t variable to compare to the RTUS Service UUID.  */
#define QAPI_BLE_RTUS_COMPARE_RTUS_SERVICE_UUID_TO_UUID_16(_x)    QAPI_BLE_COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x18, 0x06)

   /* The following defines the Reference Time Update Service UUID that */
   /* is used when building the RTUS Service Table.                     */
#define QAPI_BLE_RTUS_SERVICE_BLUETOOTH_UUID_CONSTANT             { 0x06, 0x18 }

   /* The following MACRO is a utility MACRO that assigns the RTUS Time */
   /* Update State Characteristic 16 bit UUID to the specified          */
   /* qapi_BLE_UUID_16_t variable.  This MACRO accepts one parameter    */
   /* which is the qapi_BLE_UUID_16_t variable that is to receive the   */
   /* RTUS Time Update State UUID Constant value.                       */
   /* * NOTE * The UUID will be assigned into the qapi_BLE_UUID_16_t    */
   /*          variable in Little-Endian format.                        */
#define QAPI_BLE_RTUS_ASSIGN_TIME_UPDATE_STATE_UUID_16(_x)                      QAPI_BLE_ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0x17)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined RTUS Time Update State UUID in UUID16 form.*/
   /* This MACRO only returns whether the qapi_BLE_UUID_16_t variable is*/
   /* equal to the Time Update State UUID (MACRO returns boolean result)*/
   /* NOT less than/greater than.  The first parameter is the           */
   /* qapi_BLE_UUID_16_t variable to compare to the RTUS Time Update    */
   /* State UUID                                                        */
#define QAPI_BLE_RTUS_COMPARE_RTUS_TIME_UPDATE_STATE_UUID_TO_UUID_16(_x)        QAPI_BLE_COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0x17)

   /* The following defines the RTUS Time Update State                  */
   /* Characteristic UUID that is used when building the rtus Service   */
   /* Table.                                                            */
#define QAPI_BLE_RTUS_TIME_UPDATE_STATE_CHARACTERISTIC_BLUETOOTH_UUID_CONSTANT  { 0x17, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the RTUS Time */
   /* Update Control Point Characteristic 16 bit UUID to the specified  */
   /* qapi_BLE_UUID_16_t variable.  This MACRO accepts one parameter    */
   /* which is the qapi_BLE_UUID_16_t variable that is to receive the   */
   /* RTUS Time Update Control Point UUID Constant value.               */
   /* * NOTE * The UUID will be assigned into the qapi_BLE_UUID_16_t    */
   /*          variable in Little-Endian format.                        */
#define QAPI_BLE_RTUS_ASSIGN_TIME_UPDATE_CONTROL_POINT_UUID_16(_x)                      QAPI_BLE_ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0x16)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID16 to the defined RTUS Time Update Control Point UUID in      */
   /* UUID16 form.  This MACRO only returns whether the                 */
   /* qapi_BLE_UUID_16_t variable is equal to the RTUS Time Update      */
   /* Control Point UUID (MACRO returns boolean result) NOT less        */
   /* than/greater than.  The first parameter is the qapi_BLE_UUID_16_t */
   /* variable to compare to the RTUS Time Update Control Point UUID.   */
#define QAPI_BLE_RTUS_COMPARE_RTUS_TIME_UPDATE_CONTROL_POINT_UUID_TO_UUID_16(_x)        QAPI_BLE_COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0x16)

   /* The following defines the RTUS Time Update Control Point          */
   /* Characteristic UUID that is used when building the RTUS Service   */
   /* Table.                                                            */
#define QAPI_BLE_RTUS_TIME_UPDATE_CONTROL_POINT_CHARACTERISTIC_BLUETOOTH_UUID_CONSTANT  { 0x16, 0x2A }

   /* The following define the valid Time Update Control Point commands */
   /* that may be written to the Time Update Control Point              */
   /* characteristic value.                                             */
#define QAPI_BLE_RTUS_TIME_UPDATE_CONTROL_POINT_GET_REFERENCE_UPDATE     0x01
#define QAPI_BLE_RTUS_TIME_UPDATE_CONTROL_POINT_CANCEL_REFERENCE_UPDATE  0x02

   /* The following defines the valid Current State that may be read    */
   /* from the Time Update State characteristic.                        */
#define QAPI_BLE_RTUS_CURRENT_STATE_IDLE                          0x00
#define QAPI_BLE_RTUS_CURRENT_STATE_UPDATE_PENDING                0X01

   /* The following defines the valid Result that may be read from the  */
   /* Time Update State characteristic.                                 */
#define QAPI_BLE_RTUS_RESULT_SUCCESSFUL                           0x00
#define QAPI_BLE_RTUS_RESULT_CANCELED                             0x01
#define QAPI_BLE_RTUS_RESULT_NO_CONNECTION_TO_REFERENCE           0x02
#define QAPI_BLE_RTUS_RESULT_REFERENCE_RESPONDED_WITH_AN_ERROR    0x03
#define QAPI_BLE_RTUS_RESULT_TIMEOUT                              0x04
#define QAPI_BLE_RTUS_RESULT_UPDATE_NOT_ATTEMPTED_AFTER_RESET     0x05

   /* The following MACRO is a utility MACRO that exists to determine   */
   /* if the value written to the Time Update Control Point is a valid  */
   /* command.                                                          */
#define QAPI_BLE_RTUS_TIME_UPDATE_CONTROL_POINT_VALID_COMMAND(_x)  ((((uint8_t)(_x)) >= QAPI_BLE_RTUS_TIME_UPDATE_CONTROL_POINT_GET_REFERENCE_UPDATE) &&   \
                                                                    (((uint8_t)(_x)) <= QAPI_BLE_RTUS_TIME_UPDATE_CONTROL_POINT_CANCEL_REFERENCE_UPDATE))

   /* The following MACRO is a utility MACRO that exists to validate the*/
   /* specified Time Update State.  The parameters to this function is  */
   /* CurrentState and Result field of                                  */
   /* qapi_BLE_RTUS_Time_Update_State_Data_t structure.  This MACRO     */
   /* returns TRUE if the Time Update State is valid or FALSE otherwise.*/
#define QAPI_BLE_RTUS_TIME_UPDATE_STATE_VALID(_x,_y)     (((((uint8_t)(_x)) >= QAPI_BLE_RTUS_CURRENT_STATE_IDLE) && (((uint8_t)(_x)) <= QAPI_BLE_RTUS_CURRENT_STATE_UPDATE_PENDING)) && \
                                                          ((((uint8_t)(_y)) >= QAPI_BLE_RTUS_RESULT_SUCCESSFUL) && (((uint8_t)(_y)) <= QAPI_BLE_RTUS_RESULT_UPDATE_NOT_ATTEMPTED_AFTER_RESET)))

   /* The following defines the length of the Reference Time Update     */
   /* Control Point characteristic value.                               */
#define QAPI_BLE_RTUS_TIME_UPDATE_CONTROL_POINT_VALUE_LENGTH      (sizeof(uint8_t))

  /* The following defines the RTUS GATT Service Flags MASK that should */
  /* be passed into the qapi_BLE_GATT_Register_Service() function when  */
  /* the RTUS Service is registered.                                    */
#define QAPI_BLE_RTUS_SERVICE_FLAGS                               (QAPI_BLE_GATT_SERVICE_FLAGS_LE_SERVICE)

   /* The followng defines the format of a Time Update State.           */
   /* The first member specifies information of Current State.          */
   /* The second member specifies Result of Time Update State.          */
typedef __QAPI_BLE_PACKED_STRUCT_BEGIN__ struct qapi_BLE_RTUS_Time_Update_State_s
{
   qapi_BLE_NonAlignedByte_t CurrentState;
   qapi_BLE_NonAlignedByte_t Result;
} __QAPI_BLE_PACKED_STRUCT_END__ qapi_BLE_RTUS_Time_Update_State_t;

#define QAPI_BLE_RTUS_TIME_UPDATE_STATE_SIZE                      (sizeof(qapi_BLE_RTUS_Time_Update_State_t))

#endif
