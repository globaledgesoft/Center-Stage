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

#ifndef _netutils_h_
#define _netutils_h_

#include <stdint.h>

#ifndef min
#define  min(a,b)    (((a) <= (b)) ? (a) : (b))
#endif

#define ET_ADDRLEN  6

#define HEX_BYTES_PER_LINE      16

#define INTR_DISABLE()
#define INTR_ENABLE()

/* queue element: cast to right type */
struct q_elt
{
    struct q_elt *qe_next;  /* pointer to next elt */
};
typedef struct q_elt  *qp;  /* q pointer */

/* queue header */
typedef struct queue
{
    struct q_elt *q_head;   /* first element in queue */
    struct q_elt *q_tail;   /* last element in queue */
    int  q_len;             /* number of elements in queue */
} QUEUE_T;

typedef struct
{
    uint32_t    seconds;        /* number of seconds */
    uint32_t    milliseconds;   /* number of milliseconds */
    uint32_t    ticks;          /* number of systicks */
} time_struct_t;

void enqueue(QUEUE_T *q, void *item);
void * dequeue(QUEUE_T *q);

/* Return milliseconds */
uint32_t app_get_time(time_struct_t *time);
void app_msec_delay(uint32_t ms);
uint32_t app_get_time_difference(time_struct_t *time1, time_struct_t *time2);

void app_hexdump(void *inbuf, unsigned inlen, int ascii, int addr);
int hwaddr_pton(const char *txt, uint8_t *hwaddr, size_t buflen);
int hexstr2bin(const char *hex, uint8_t *buf, size_t len);

#endif /* _netutils_h_ */
