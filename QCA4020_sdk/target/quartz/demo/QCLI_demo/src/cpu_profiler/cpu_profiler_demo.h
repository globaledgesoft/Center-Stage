/*
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
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

#include <stdint.h>
#include <stdlib.h>

#define ENABLE_MOST_USED_FUNCTIONS_THREAD_ID_LOGGING 1

#define MAX_FUNCTION_INFO_LIST_LENGTH 10
#define MAX_THREAD_IDS_TO_RECORD 5


typedef struct thread_info_s {
    uint32_t thread_id;
    uint32_t count;
} thread_info_t;


typedef struct function_info_s {
    uint32_t index;
    uint32_t count;
#if ENABLE_MOST_USED_FUNCTIONS_THREAD_ID_LOGGING
    thread_info_t thread_info_list[MAX_THREAD_IDS_TO_RECORD];
    uint32_t      thread_info_list_length;
#endif
} function_info_t;


typedef struct cpu_profiler_ctxt_s {
    int32_t sock;
    uint32_t functions_count;
    uint32_t * function_addresses;
    uint16_t * function_samples;

    function_info_t function_info_list[MAX_FUNCTION_INFO_LIST_LENGTH];
    uint32_t function_info_list_length;

    uint32_t count_of_pcs_to_capture;
    uint32_t count_of_pcs_captured;

    uint32_t original_isr;
} cpu_profiler_ctxt_t;



void Initialize_CpuProfiler_Demo(void);
