/*
 * Copyright (c) 2015-2017 Qualcomm Technologies, Inc.
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

#ifndef __HMI_DEMO_COMMON_H__
#define __HMI_DEMO_COMMON_H__

/* This structure represents an entry in the remove device list. It contains
   the extended and short address of the remote device and a flag indicating
   if the device is sleepy. */
typedef struct Device_List_Entry_s
{
   uint64_t ExtAddr;
   uint16_t ShortAddr;
   uint8_t  Flags;
   uint8_t  DeviceIndex;
} Device_List_Entry_t;

#define DEVICE_LIST_ENTRY_FLAG_ENTRY_IN_USE        (0x0001)
#define DEVICE_LIST_ENTRY_FLAG_IS_SLEEPY           (0x0002)

/* This structure stores the information for mode transitions. */
typedef struct HMI_Demo_Transition_Data_s
{
   uint32_t            Interface_ID;
   uint16_t            PAN_ID;
   uint16_t            Config;
   uint8_t             KeyIndex;
   uint8_t             Device_Count;
   struct
   {
      uint64_t         Expire_Time;
      uint32_t         Period;
      uint32_t         Device_Index;
      uint32_t         Time_Remaining;
      uint8_t          Flags;
      uint8_t          MSDULength;
      uint8_t          TxOptions;
   } Send_Info;
   Device_List_Entry_t Device_List[];
} HMI_Demo_Transition_Data_t;


#endif

