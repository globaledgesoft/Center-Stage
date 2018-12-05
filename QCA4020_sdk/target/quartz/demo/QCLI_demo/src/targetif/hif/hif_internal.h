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

#ifndef _HIF_INTERNAL_H_
#define _HIF_INTERNAL_H_


#define BUS_REQUEST_MAX_NUM                64
#define HIF_DEFAULT_IO_BLOCK_SIZE          256

#define HIF_ENDPOINT_BLOCK_SIZE                HIF_DEFAULT_IO_BLOCK_SIZE
#define HIF_ENDPOINT0_BLOCK_SIZE               HIF_ENDPOINT_BLOCK_SIZE
#define HIF_ENDPOINT1_BLOCK_SIZE               HIF_ENDPOINT_BLOCK_SIZE
#define HIF_ENDPOINT2_BLOCK_SIZE               HIF_ENDPOINT_BLOCK_SIZE
#define HIF_ENDPOINT3_BLOCK_SIZE               HIF_ENDPOINT_BLOCK_SIZE

#define HIF_ENDPOINT_BASE_ADDR                 0x800
#define HIF_ENDPOINT_DUMMY_WIDTH               0
#define HIF_ENDPOINT_WIDTH                     0x800

#define HIF_ENDPOINT_START_ADDR(endpoint)                        \
    HIF_ENDPOINT_BASE_ADDR + endpoint * HIF_ENDPOINT_WIDTH

#define HIF_ENDPOINT_END_ADDR(endpoint)	                         \
    HIF_ENDPOINT_START_ADDR(endpoint) + HIF_ENDPOINT_WIDTH - 1

#define HIF_HOST_CONTROL_REG_START_ADDR  0x400
#define HIF_HOST_CONTROL_REG_END_ADDR    0x5FF

typedef struct bus_request {
    struct bus_request *next;       /* link list of available requests */
    struct bus_request *inusenext;  /* link list of in use requests */
    qurt_signal_t sem_req;
    OSAL_UINT32 address;               /* request data */
    OSAL_UINT8 *buffer;
    OSAL_UINT32 length;
    OSAL_UINT32 request;
    void *context;
    htc_status_t status;
} BUS_REQUEST;


struct hif_device {
    trans_device_t *tdev;
    OSAL_MUTEX_T asynclock;
	qurt_thread_attr_t async_task_attr;
	qurt_thread_t async_task_id;	         /* task to handle async commands */
    qurt_signal_t sem_async;                 /* wake up for async task */
    int    async_shutdown;                   /* stop the async task */
    qurt_signal_t async_completion;      /* thread completion */
    BUS_REQUEST   *asyncreq;                 /* request for async tasklet */
    BUS_REQUEST *taskreq;                    /*  async tasklet data */
#ifdef TX_COMPLETION_THREAD
	qurt_thread_attr_t tx_completion_attr;
    qurt_thread_t tx_completion_task;
    qurt_signal_t sem_tx_completion;
    int    tx_completion_shutdown;
    qurt_signal_t tx_completion_exit;
    OSAL_MUTEX_T tx_completion_lock;
    BUS_REQUEST *tx_completion_req;
    BUS_REQUEST **last_tx_completion;
#endif
    OSAL_MUTEX_T lock;
    BUS_REQUEST *s_busRequestFreeQueue;         /* free list */
    BUS_REQUEST busRequest[BUS_REQUEST_MAX_NUM]; /* available bus requests */
    void     *claimedContext;
    HTC_CALLBACKS htcCallbacks;
    OSAL_BOOL   is_suspend;
    OSAL_BOOL   is_disabled;
    HIF_DEVICE_POWER_CHANGE_TYPE powerConfig;
    HIF_DEVICE_STATE DeviceState;
    void *htcContext;
    OSAL_BOOL ctrl_response_timeout;
    void *htc_handle;
};

BUS_REQUEST *hifAllocateBusRequest(HIF_DEVICE *device);
void hifFreeBusRequest(HIF_DEVICE *device, BUS_REQUEST *busrequest);
void AddToAsyncList(HIF_DEVICE *device, BUS_REQUEST *busrequest);
void HIFDumpCCCR(HIF_DEVICE *hif_device);

#endif // _HIF_INTERNAL_H_

