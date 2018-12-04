/*
 * Copyright (c) 2017 Qualcomm Technologies, Inc.
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

 

#include "stdlib.h"
#include "cpu_profiler_demo.h"
#include "qurt_thread.h"


extern cpu_profiler_ctxt_t g_cpu_profiler_ctxt;


#define WriteReg(address, value) \
    (*((volatile uint32_t *) address) = value)

#define ReadReg(address) \
    (*((volatile uint32_t *) address))

#define CPU_PROFILER_TIMER_INTERRUPT_INDEX 1
#define NVIC_SYSTEM_INTERRUPTS_COUNT 16
#define CPU_PROFILER_TIMER_INTERRUPT_INDEX_IN_NVIC (NVIC_SYSTEM_INTERRUPTS_COUNT + CPU_PROFILER_TIMER_INTERRUPT_INDEX)
#define CPU_PROFILER_TIMER_LOAD      0x44001020
#define CPU_PROFILER_TIMER_CONTROL   0x44001028
#define CPU_PROFILER_TIMER_INTCLR    0x4400102c
#define CPU_PROFILER_TIMER_BGLOAD    0x44001038



#define ASSERT_BREAK(x) \
    do { \
        if ( !(x) ) { \
            __asm__ volatile("BKPT 3"); \
        }   \
    } while (0)


static inline void __disable_irq(void)
{
    __asm__ volatile("CPSIE I");
}

static inline uint32_t __get_PRIMASK(void)
{
  register uint32_t primask_reg asm("PRIMASK");
  return primask_reg;
}


static inline void __set_PRIMASK(uint32_t primask_val)
{
  register uint32_t primask_reg asm("PRIMASK");
  primask_reg = primask_val;
}


void cpu_profiler_timer_isr(
    uint32_t ipsr_register_value,
    uint32_t lr_register_value,
    uint32_t msp_register_value,
    uint32_t psp_register_value
    )
{
    if ( g_cpu_profiler_ctxt.count_of_pcs_captured >= g_cpu_profiler_ctxt.count_of_pcs_to_capture )
    {
        cpu_profiler_stop();
    }

    uint32_t value;
    if ( (lr_register_value & 0xd) == 0xd ) {
        value = ((uint32_t*) psp_register_value)[6];
    }
    else {
        value = ((uint32_t*) msp_register_value)[6];
    }


    // binary search algorithm to find the index of the function PC belongs to
    uint32_t low_index = 0;
    uint32_t high_index = g_cpu_profiler_ctxt.functions_count;
    uint32_t mid_index;
    uint32_t mid_index_value;

    while ( (high_index - low_index) > 1  ) {
        mid_index = (low_index + high_index) / 2;
        mid_index_value = g_cpu_profiler_ctxt.function_addresses[mid_index];
        if ( value >= mid_index_value ) {
            low_index = mid_index;
        }
        else {
            high_index = mid_index;
        }
    }

    ASSERT_BREAK( low_index < g_cpu_profiler_ctxt.functions_count );

    g_cpu_profiler_ctxt.function_samples[low_index]++;

    function_info_t * p_function_info = 0;

    if ( 0 == g_cpu_profiler_ctxt.function_samples[low_index] ) {
        int i;
        for ( i = 0; i < g_cpu_profiler_ctxt.function_info_list_length; i++ ) {
            if ( g_cpu_profiler_ctxt.function_info_list[i].index == low_index ) {
                p_function_info = &g_cpu_profiler_ctxt.function_info_list[i];
                p_function_info->count += 0xffff;
                break;
            }
        }
        if ( (!p_function_info) && (g_cpu_profiler_ctxt.function_info_list_length < MAX_FUNCTION_INFO_LIST_LENGTH) ) {
            p_function_info = &g_cpu_profiler_ctxt.function_info_list[g_cpu_profiler_ctxt.function_info_list_length];
            p_function_info->index = low_index;
            p_function_info->count = 0xffff;
            g_cpu_profiler_ctxt.function_info_list_length++;
        }
    }


#if ENABLE_MOST_USED_FUNCTIONS_THREAD_ID_LOGGING
    if ( !p_function_info ) {
        int i;
        for ( i = 0; i < g_cpu_profiler_ctxt.function_info_list_length; i++ ) {
            if ( g_cpu_profiler_ctxt.function_info_list[i].index == low_index ) {
                p_function_info = &g_cpu_profiler_ctxt.function_info_list[i];
            }
        }
    }

    if ( p_function_info ) {
        uint32_t thread_id = qurt_thread_get_id();
        int i;
        thread_info_t * p_thread_info = 0;
        for ( i = 0; i < p_function_info->thread_info_list_length; i++ ) {
            if ( thread_id == p_function_info->thread_info_list[i].thread_id ) {
                p_thread_info = &p_function_info->thread_info_list[i];
                p_thread_info->count++;
            }
        }
        if ( (!p_thread_info) && (p_function_info->thread_info_list_length < MAX_THREAD_IDS_TO_RECORD ) ) {
            p_thread_info = &p_function_info->thread_info_list[p_function_info->thread_info_list_length];
            p_thread_info->thread_id = thread_id;
            p_function_info->thread_info_list_length++;
        }
    }
#endif

    g_cpu_profiler_ctxt.count_of_pcs_captured++;

    uint32_t old_primask = __get_PRIMASK();
    __disable_irq();
    InterruptController_ClearPending( CPU_PROFILER_TIMER_INTERRUPT_INDEX );
    WriteReg(CPU_PROFILER_TIMER_INTCLR, 0);
    __set_PRIMASK( old_primask );

}
