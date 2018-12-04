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

#ifndef __QAPI_BLE_HCIUSBT_H__
#define __QAPI_BLE_HCIUSBT_H__

   /* The following constants represent the Minimum, Maximum, and Values*/
   /* that are used with the Initialization Delay member of the         */
   /* qapi_BLE_HCI_USBDriverInformation_t structure.  These Delays are  */
   /* specified in Milliseconds and represent the delay that is to be   */
   /* added between Port Initialization (Open) and the writing of any   */
   /* data to the Port.  This functionality was added because it was    */
   /* found that some USB Drivers required a delay between the time the */
   /* Device was opened and the time when the Driver was ready to accept*/
   /* data.  The default is NO Delay (0 Milliseconds).                  */
#define QAPI_BLE_HCI_USB_INFORMATION_INITIALIZATION_DELAY_MINIMUM       0
#define QAPI_BLE_HCI_USB_INFORMATION_INITIALIZATION_DELAY_MAXIMUM    5000
#define QAPI_BLE_HCI_USB_INFORMATION_INITIALIZATION_DELAY_DEFAULT       0

   /* The following type declaration defines the HCI USB Driver that    */
   /* that will be used as the physical HCI Transport Driver for the USB*/
   /* Device that is to be opened.  This type declaration is used in the*/
   /* qapi_BLE_HCI_USBDriverInformation_t structure that is required    */
   /* when an HCI USB Device is opened.                                 */
typedef enum
{
   QAPI_BLE_USB_DRIVER_SS1_E,
   QAPI_BLE_USB_DRIVER_GARMIN_E
} qapi_BLE_HCI_USB_Driver_t;

   /* The following type declaration represents the structure of all    */
   /* Data that is needed to open an HCI USB Port.                      */
typedef struct qapi_BLE_HCI_USBDriverInformation_s
{
   uint32_t                  DriverInformationSize;
                                                /* Physical Size of this      */
                                                /* structure.                 */
   qapi_BLE_HCI_USB_Driver_t DriverType;
                                                /* HCI USB Driver type that   */
                                                /* will be used for           */
                                                /* communication to the       */
                                                /* Bluetooth Device.          */
   uint32_t                  InitializationDelay;
                                                /* Time (In Milliseconds) to  */
                                                /* Delay after the Port is    */
                                                /* opened before any data is  */
                                                /* sent over the Port.  This  */
                                                /* member is present because  */
                                                /* some Drivers may require   */
                                                /* a delay because the device */
                                                /* does not function for some */
                                                /* specified period of time.  */
} qapi_BLE_HCI_USBDriverInformation_t;

   /* The following constant is used with the                           */
   /* HCI_USB_Driver_Reconfigure_Data_t structure (ReconfigureCommand   */
   /* member) to specify that the Communication parameters are required */
   /* to change.  When specified, the ReconfigureData member will point */
   /* to a valid qapi_BLE_HCI_USBDriverInformation_t structure which    */
   /* holds the new parameters.                                         */
   /* * NOTE * The underlying driver may not support changing all of    */
   /*          specified parameters.                                    */
#define QAPI_BLE_HCI_USB_DRIVER_RECONFIGURE_DATA_COMMAND_CHANGE_PARAMETERS  (QAPI_BLE_HCI_DRIVER_RECONFIGURE_DATA_RECONFIGURE_COMMAND_TRANSPORT_START)

#endif
