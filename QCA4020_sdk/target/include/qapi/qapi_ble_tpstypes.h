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

#ifndef __QAPI_BLE_TPS_TYPES_H__
#define __QAPI_BLE_TPS_TYPES_H__

#include "./qapi_ble_bttypes.h" /* Bluetooth Type Definitions.          */

   /* The following MACRO is a utility MACRO that assigns the Tx Power  */
   /* Service 16 bit UUID to the specified qapi_BLE_UUID_16_t variable. */
   /* This MACRO accepts one parameter which is a pointer to a          */
   /* qapi_BLE_UUID_16_t variable that is to receive the TPS UUID       */
   /* Constant value.                                                   */
   /* * NOTE * The UUID will be assigned into the qapi_BLE_UUID_16_t    */
   /*          variable in Little-Endian format.                        */
#define QAPI_BLE_TPS_ASSIGN_TPS_SERVICE_UUID_16(_x)               QAPI_BLE_ASSIGN_BLUETOOTH_UUID_16(*((qapi_BLE_UUID_16_t *)(_x)), 0x18, 0x04)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined TPS Service UUID in UUID16 form.  This     */
   /* MACRO only returns whether the qapi_BLE_UUID_16_t variable is     */
   /* equal to the TPS Service UUID (MACRO returns boolean result) NOT  */
   /* less than / greater than.  The first parameter is the             */
   /* qapi_BLE_UUID_16_t variable to compare to the TPS Service UUID.   */
#define QAPI_BLE_TPS_COMPARE_TPS_SERVICE_UUID_TO_UUID_16(_x)      QAPI_BLE_COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x18, 0x04)

   /* The following defines the Tx Power Service UUID that is used when */
   /* building the TPS Service Table.                                   */
#define QAPI_BLE_TPS_SERVICE_BLUETOOTH_UUID_CONSTANT              { 0x04, 0x18 }

   /* The following MACRO is a utility MACRO that assigns the TPS Tx    */
   /* Power Level Characteristic 16 bit UUID to the specified           */
   /* qapi_BLE_UUID_16_t variable.  This MACRO accepts one parameter    */
   /* which is the qapi_BLE_UUID_16_t variable that is to receive the   */
   /* TPS Tx Power Level UUID Constant value.                           */
   /* * NOTE * The UUID will be assigned into the qapi_BLE_UUID_16_t    */
   /*          variable in Little-Endian format.                        */
#define QAPI_BLE_TPS_ASSIGN_TX_POWER_LEVEL_UUID_16(_x)                      QAPI_BLE_ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0x07)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined TPS Tx Power Level UUID in UUID16 form .   */
   /* This MACRO only returns whether the qapi_BLE_UUID_16_t variable is*/
   /* equal to the Tx Power Level UUID (MACRO returns boolean result)   */
   /* NOT less than/greater than.  The first parameter is the           */
   /* qapi_BLE_UUID_16_t variable to compare to the Tx Power Level UUID.*/
#define QAPI_BLE_TPS_COMPARE_TPS_TX_POWER_LEVEL_UUID_TO_UUID_16(_x)         QAPI_BLE_COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0x07)

   /* The following defines the TPS Tx Power Level Characteristic UUID  */
   /* that is used when building the TPS Service Table.                 */
#define QAPI_BLE_TPS_TX_POWER_LEVEL_CHARACTERISTIC_BLUETOOTH_UUID_CONSTANT  { 0x07, 0x2A }

   /* The following defines the length of the Measurement Interval      */
   /* characteristic value.                                             */
#define QAPI_BLE_TPS_TX_POWER_LEVEL_LENGTH                        (QAPI_BLE_NON_ALIGNED_BYTE_SIZE)

   /* The following values define the minimum and maximum acceptable    */
   /* values for the Tx Power Level characteristic.                     */
#define QAPI_BLE_TPS_TX_POWER_MIN_VALUE                           (-100)
#define QAPI_BLE_TPS_TX_POWER_MAX_VALUE                           (20)

   /* The following MACRO is a utility MACRO that can be used to verify */
   /* that a given Tx Power Level value is within the valid range (as   */
   /* defined above.)                                                   */
#define QAPI_BLE_TPS_TX_POWER_LEVEL_VALID(_x)                     (((_x) >= QAPI_BLE_TPS_TX_POWER_MIN_VALUE) && ((_x) <= QAPI_BLE_TPS_TX_POWER_MAX_VALUE))

   /* The following defines the TPS GATT Service Flags MASK that should */
   /* be passed into qapi_BLE_GATT_Register_Service() when the TPS      */
   /* Service is registered.                                            */
#define QAPI_BLE_TPS_SERVICE_FLAGS                                (QAPI_BLE_GATT_SERVICE_FLAGS_LE_SERVICE)

#endif
