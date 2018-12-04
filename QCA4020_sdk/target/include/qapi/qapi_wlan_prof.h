/*
 * Copyright (c) 2011-2018 Qualcomm Technologies, Inc.
 * 2011-2016 Qualcomm Atheros, Inc.
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

/**
* @file qapi_wlan_prof.h
*
* @brief WLAN API for Profiling tool only available with integrated IPStack
*
* @details Provide API to install performance measurement, record stats in data path and measure delay in thread context switch
*
*/

#ifndef QAPI_PROFILER_H
#define QAPI_PROFILER_H

/**
* @brief Enum of Command passed to .qapi_Prof_Cmd_Handler
* @details Define command enum that is passed to driver
*/
typedef enum {
    QAPI_PROF_INSTALL_E = 0, /** INSTALL performance tool */
    QAPI_PROF_UNINSTALL_E,   /** Uninstall tool */
    QAPI_PROF_START_E,       /** Start measuring performance*/
    QAPI_PROF_STOP_E,        /** Stop measuring */
    QAPI_PROF_RESET_E,       /** Reset the stats */
    QAPI_PROF_GET_STATS_E,   /** Get recording stats */
    QAPI_PROF_MAX_CMD_E,     
} qapi_Prof_Cmd_t;

/**
* @brief Structure to hold all performance stats
* @details This structure is used hold different stats vertically across data path layers
*/
typedef struct
{
    uint32_t    avg_Time_Outside_Freeq;    /**< avg time netbuf in usage: enqueueTimestamp - dequeueTimestamp to/from freeQueue */
    uint32_t    avg_Wait_Time_In_Tx_Queue; /**< netbuf time waiting in wlan txqueue */
    uint32_t    avg_Socket_BlockTime;      /**< how long was avg socket block call */
    uint32_t    netbuf_Low_Watermark_0;    /**< netbuf queue0 low watermark hit any time since system up time */
    uint32_t    netbuf_Low_Watermark_1;    /**< netbuf queue1 low watermark hit any time since system up time */
    uint32_t    netbuf_Low_Watermark_2;    /**< netbuf queue2 low watermark hit any time since system up time */
    uint32_t    test_Runtime;              /**< Total time profiling test run */
    uint32_t    rx_Pkt_Count;              /**< Number of received packets */
    uint32_t    tx_Pkt_Count;              /**< Number of transmitted packets */
    uint32_t    wlan_Rx_Bytes;             /**< Number of bytes received by WLAN driver */
    uint32_t    wlan_Tx_Bytes;             /**< Number of bytes transmitted from WLAN driver */
    uint32_t    bus_Rx_Bytes;              /**< Number of bytes received by the bus */
    uint32_t    bus_Tx_Bytes;              /**< Number of bytes transmitted from the bus */
    uint32_t    bus_Rw_Error;              /**< Read/Write error on the bus */
} qapi_Prof_Stats_t;

/**
* @brief API that handle all Profiling Command.
*
* @details Call this API to execute any Profiling command defined in "qapi_Prof_Cmd_t".
*
* @param[in]      cmd  Command defined in qapi_Prof_Cmd_t.
* @param[in]      data Data pointer using to pass information from/to caller and driver
*
* @return       0 if operation succeeded, -1 otherwise.
*/
int8_t qapi_Prof_Cmd_Handler(qapi_Prof_Cmd_t cmd, qapi_Prof_Stats_t *data);

#endif //QAPI_PROFILER_H
