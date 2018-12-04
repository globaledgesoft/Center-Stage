/*
 * Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
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

#ifndef _OSAL_TYPES_H_
#define _OSAL_TYPES_H_

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "malloc.h"
/*
 * Global macros
 */

#define FREE_RTOS 1
#define MQX_LITE  0
#define MQX_RTOS  0
#define OSAL_QURT 1

#if OSAL_QURT
#include "qapi/qurt_thread.h"
#include "qapi/qurt_types.h"
#include "qapi/qurt_error.h"
#include "qapi/qurt_signal.h"
#include "qapi/qurt_mutex.h"
#endif
   
typedef int8_t       OSAL_INT8;
typedef int16_t      OSAL_INT16;
typedef int32_t      OSAL_INT32;
typedef int64_t      OSAL_INT64;

typedef uint8_t      OSAL_UINT8;
typedef uint16_t     OSAL_UINT16;
typedef uint32_t     OSAL_UINT32;
typedef uint64_t     OSAL_UINT64;

/* NOTE: OSAL_BOOL is a type that is used in various WMI commands and events.
 * as such it is a type that is shared between the wifi chipset and the host.
 * It is required for that reason that OSAL_BOOL be treated as a 32-bit/4-byte type.
 */
typedef int32_t      OSAL_BOOL;
typedef char        OSAL_CHAR;
typedef uint8_t      OSAL_UCHAR;
#define OSAL_VOID void 
#define OSAL_CONST const

#define OSAL_TRUE      ((OSAL_BOOL)1)
#define OSAL_FALSE     ((OSAL_BOOL)0)


#define OSAL_ULONG             uint32_t
#define OSAL_LONG              int32_t

#define boolean     OSAL_BOOL
#define  uint_8  uint8_t
#define   int_8   int8_t
#define uint_16 uint16_t
#define  int_16  int16_t
#define uint_32 uint32_t
#define  int_32  int32_t
#define uint_64 uint64_t
#define  int_64  int64_t

/* OSAL Error codes */
typedef enum 
{  
  OSAL_ERROR = -1,
  OSAL_OK = 0,
  OSAL_SUCCESS = 0,
  OSAL_INVALID_TASK_ID = 37,
  OSAL_INVALID_PARAMETER,
  OSAL_INVALID_POINTER,
  OSAL_ALREADY_EXISTS,
  OSAL_INVALID_EVENT,
  OSAL_EVENT_TIMEOUT,
  OSAL_INVALID_MUTEX,
  OSAL_TASK_ALREADY_LOCKED,
  OSAL_MUTEX_ALREADY_LOCKED,  
  OSAL_OUT_OF_MEMORY,
} OSAL_STATUS;

/* Stack Size */
#define OSAL_DRIVER_TASK_STACK_SIZE            (3000)

/* Priority setting */
#define OSAL_DRIVER_TASK_PRIORITY          OSAL_TASK_PRIORITY_HIGHEST
#define OSAL_BLOCK_FOR_RESPONSE_PRIORITY   OSAL_TASK_PRIORITY_LOWEST

#define OSAL_TASK_PRIORITY_LOWEST              0
#define OSAL_TASK_PRIORITY_ABOVE_LOWEST        1
#define OSAL_TASK_PRIORITY_BELOW_LOWER         2
#define OSAL_TASK_PRIORITY_LOWER               3
#define OSAL_TASK_PRIORITY_ABOVE_LOWER         4
#define OSAL_TASK_PRIORITY_MEDIUM              5
#define OSAL_TASK_PRIORITY_ABOVE_MEDIUM        6
#define OSAL_TASK_PRIORITY_BELOW_HIGH          7
#define OSAL_TASK_PRIORITY_HIGH                8
#define OSAL_TASK_PRIORITY_ABOVE_HIGH          9
#define OSAL_TASK_PRIORITY_HIGHER              10
#define OSAL_TASK_PRIORITY_HIGHEST             11
 
#define OSAL_NULL_TASK_ID              0
#define OSAL_THREAD_MAX_PRIORITIES              (32)

/* Various event attributes for OSAL Events */
#define OSAL_EVENT_ATTR_WAIT_FOR_ALL           (1 << 0)
#define OSAL_EVENT_ATTR_CLEAR_ENABLE           (1 << 1)

/* Mutex timeout related macros */
#define OSAL_MUTEX_BLOCK_TILL_ACQUIRE  0xffffffff
#define OSAL_MUTEX_TRY_ACQUIRE         0x0

/* Timer related macros*/
typedef uint32_t                     qosal_timer_handle;
#define OSAL_TIMER_ONESHOT              0x01   /**Default, one short timer*/
#define OSAL_TIMER_PERIODIC             0x02   /**periodic timer*/
#define OSAL_TIMER_NO_AUTO_START           0x04   /**No Auto Activate, default for auto start*/

#define OSAL_TICK_STRUCT               NULL
#define OSAL_TICK_STRUCT_PTR           void*
#define OSAL_TASK_ID                   OSAL_UINT32
#define UNUSED_ARGUMENT(arg)            ((void)arg)
#define OSAL_EVENT_AUTO_CLEAR          0x01
 typedef struct time_struct
{
   /* The number of seconds in the time.  */
   uint32_t     SECONDS;
   /* The number of milliseconds in the time. */
   uint32_t     MILLISECONDS;
} TIME_STRUCT;

#define _PTR_      *

/* OSAL_MEM -- macros that should be mapped to OS/STDLIB equivalent functions */
#define OSAL_MEMCPY(dst, src, len)         memcpy((OSAL_UINT8 *)(dst), (src), (len))
#define OSAL_MEMZERO(addr, len)            memset((addr), 0, (len))
#define OSAL_MEMCMP(addr1, addr2, len)     memcmp((addr1), (addr2), (len))
#define OSAL_MALLOC(size)					malloc(size)
#define OSAL_FREE(addr)                    free(addr)

typedef void*   qosal_event_handle;
typedef void*   qosal_mutex_handle;

#if OSAL_QURT
typedef void *                      qosal_sem_handle;
typedef void *	                    qosal_queue_handle;
typedef qurt_thread_t               qosal_task_handle;
typedef void*                       qosal_sema_handle; //TBD    
#else
/*Global Handlers type defination*/
typedef void *                      qosal_sem_handle;
typedef void *	                    qosal_queue_handle;
typedef void * 	                    qosal_task_handle;
typedef void*                       qosal_sema_handle; //TBD    
#endif

/* Defines the prototype to which task functions must conform. */
typedef void (*pTask_entry)( void * );

/* Mutual Exclusion */
typedef qurt_mutex_t  OSAL_MUTEX_T;

#define OSAL_MUTEX_INIT(mutex)             qurt_mutex_init(mutex)
#define OSAL_MUTEX_LOCK(mutex)             qurt_mutex_lock(mutex)
#define OSAL_MUTEX_UNLOCK(mutex)           qurt_mutex_unlock(mutex)
#define OSAL_MUTEX_DEINIT(mutex)           qurt_mutex_destroy(mutex)


#define mdelay(arg) Sleep(arg);

#define ASSERT( __bool ) \
    do { \
        if (0 == (__bool)) { \
            __asm volatile("bkpt 0"); \
        } \
    } while (0)

#define  panic(x) ASSERT(0)


#endif          //_OSAL_TYPES_H_
