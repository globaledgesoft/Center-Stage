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

#ifndef __QAPI_BLE_BTAPITYP_H__
#define __QAPI_BLE_BTAPITYP_H__

   /* The following definition defines the API Calling Convention for   */
   /* ALL the exported Bluetoth API Functions.                          */
#ifndef QAPI_BLE_BTPSAPI

   #define QAPI_BLE_BTPSAPI

#endif

   /* The following conditional declaration controls the specific       */
   /* linkage that may/may not be required to specify Bluetooth API     */
   /* functions.                                                        */
#ifndef QAPI_BLE_DECLARATION

   #define QAPI_BLE_DECLARATION

#endif

   /* This define (BTPSCONST) is used to conditionally support the use  */
   /* of the const modifier.                                            */
   /* * NOTE * This is only provided for older platforms that do not    */
   /*          support the const modifier.                              */
#ifndef QAPI_BLE_CONST

   #define QAPI_BLE_CONST const

#endif

   /* In case the compiler does not support exception handling, define  */
   /* the exception handling to do nothing.                             */
#ifndef __QAPI_BLE_TRY

   #define __QAPI_BLE_TRY

#endif

#ifndef __QAPI_BLE_EXCEPT

   #define __QAPI_BLE_EXCEPT(_x)    if(0)

#endif

   /* The following MACRO is a utility MACRO that exists to calculate   */
   /* the offset position of a particular structure member from the     */
   /* start of the structure. This MACRO accepts as the first parameter,*/
   /* the physical name of the structure (the type name, NOT the        */
   /* variable name). The second parameter to this MACRO represents the */
   /* actual structure member that the offset is to be determined. This */
   /* MACRO returns an unsigned integer that represents the offset (in  */
   /* bytes) of the structure member.                                   */
#define QAPI_BLE_BTPS_STRUCTURE_OFFSET(_x, _y)           ((unsigned int)&(((_x *)0)->_y))

#endif
