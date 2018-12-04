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

 
#ifndef _QAPI_FIRMWARE_IMAGE_ID_H_
#define _QAPI_FIRMWARE_IMAGE_ID_H_

/*
 * This is the master database of Flash Image IDs used by Quartz (QCA 402x).
 *
 * Allocation of ID values and selection of names is handled by
 * first-come-first-serve stake-your-claim. It is expected that
 * the number of users will remain small relative to the size of
 * the Flash Image ID space.
 */

/* 0..127 are reserved for IDs used by CoreTech software */
/* Matches header file flash_image_id.h */

#if !defined(NO_IMG_ID)
#define NO_IMG_ID        0 /* Unusable Image ID */
#endif

#if !defined(RESERVE1_IMG_ID)
#define RESERVE1_IMG_ID  1
#endif

#if !defined(RESERVE2_IMG_ID)
#define RESERVE2_IMG_ID  2
#endif

#if !defined(RESERVE3_IMG_ID)
#define RESERVE3_IMG_ID  3
#endif

#if !defined(RESERVE4_IMG_ID)
#define RESERVE4_IMG_ID  4
#endif


#if !defined(FS1_IMG_ID)
#define FS1_IMG_ID       5 /* Filesystem Image #1 */
#endif

#if !defined(M4_IMG_ID)
#define M4_IMG_ID       10 /* Quartz M4F Application SubSystem Firmware Image */
#endif

#if !defined(M0_IMG_ID)
#define M0_IMG_ID       11 /* Quartz M0 Connectivity Subsystem Firmware Image */
#endif

#if !defined(PATCH_IMG_ID)
#define PATCH_IMG_ID    12 /* Quartz ROM patches */
#endif

#if !defined(WLAN_IMG_ID)
#define WLAN_IMG_ID     13 /* Wireless LAN Firmware Image ID */
#endif

#if !defined(RAMDUMP_IMG_ID)
#define RAMDUMP_IMG_ID  99 /* 0x63 -- RAMDUMP partition ID */
#endif

#if !defined(FLOG_IMG_ID)
#define FLOG_IMG_ID    100 /* 0x64 -- Flash logging partition ID */
#endif

/* 128..195 are reserved for QCOM IDs not known to CoreTech software */
#if !defined(FS2_IMG_ID)
#define FS2_IMG_ID     128 /* 0x80 -- Quartz Filesystem Image #2 */
#endif

#if !defined(UNUSED_IMG_ID)
#define UNUSED_IMG_ID   129 /* 0x81 -- Unused partition ID; never looked up */
#endif

#if !defined(PRE_ROT_IMG_ID)
#define PRE_ROT_IMG_ID     138 /* 0x8a -- Pre-ROT Image ID */
#endif


/* 196..255 are reserved for OEM use */

#if !defined(FLASH_ALL)
#define  FLASH_ALL      0x7FFF /* The entire flash part */
#endif

#endif /* _QAPI_FIRMWARE_IMAGE_ID_H_ */
