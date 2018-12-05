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

/************************************************************
 * 
 * htc.h  -  HTC Module specific declarations and prototypes
 *
 ************************************************************/

#ifndef _HTC_H_
#define _HTC_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* ------ ENDPOINT ID ------ */
typedef enum
{
    ENDPOINT_UNUSED = -1,
    ENDPOINT1 = 0,
    ENDPOINT2,
    ENDPOINT3,
    ENDPOINT4,
} HTC_ENDPOINT_ID;

/* ------ Event Types ------ */
typedef enum
{
    HTC_BUFFER_RECEIVED = 0,
    HTC_SKB_RECEIVED,
    HTC_BUFFER_SENT,
    HTC_SKB_SENT,
    HTC_DATA_AVAILABLE,
    HTC_TARGET_AVAILABLE,
    HTC_TARGET_UNAVAILABLE,
} HTC_EVENT_ID;

#define HTC_MAILBOX_NUM_MAX                4
#define HTC_HEADER_LEN                     2
#define HTC_DATA_REQUEST_RING_BUFFER_SIZE  30 

/* ------- Target Specific Data structures ------- */
typedef struct htc_target HTC_TARGET;

/* ------- Event Specific Data Structures ------- */
typedef struct htc_event_info HTC_EVENT_INFO;
typedef void   (*HTC_EVENT_HANDLER) (HTC_TARGET *, 
                                     HTC_ENDPOINT_ID, 
                                     HTC_EVENT_ID, 
                                     HTC_EVENT_INFO *, 
                                     void *);

/* WMI layer extracts the relevant information from this data structure */
struct htc_event_info
{
    OSAL_UINT8             *buffer;
    void                *cookie;
    OSAL_UINT32             bufferLength;
    OSAL_UINT32             actualLength;
    htc_status_t             status;
};

/* ------ Function Prototypes ------ */
htc_status_t
htc_host_init(void);

htc_status_t
htc_host_start(HTC_TARGET *target);
/* target - IN */

htc_status_t 
htc_host_event_register(HTC_TARGET *target, HTC_ENDPOINT_ID endPointId, 
            HTC_EVENT_ID eventId, HTC_EVENT_HANDLER eventHandler, 
            void *param);
/* target - IN, endPointId - IN, eventId - IN, eventHandler - IN, param - IN */

htc_status_t htc_host_event_deregister(HTC_TARGET *target, HTC_ENDPOINT_ID endPointId,
            HTC_EVENT_ID eventId);
/* target - IN, endPointId - IN, eventId - IN */


htc_status_t 
htc_host_buffer_receive(HTC_TARGET *target, HTC_ENDPOINT_ID endPointId, 
                 OSAL_UINT8 *buffer, OSAL_UINT32 length, void *cookie);
/* target - IN, endPointId - IN, buffer - IN, length - IN, cookie - IN */


htc_status_t 
htc_host_buffer_send(HTC_TARGET *target, HTC_ENDPOINT_ID endPointId, 
              OSAL_UINT8 *buffer, OSAL_UINT32 length, void *cookie);
/* target - IN, endPointId - IN, buffer - IN, length - IN, cookie - IN */


htc_status_t
htc_host_stop(HTC_TARGET *target);
/* target - IN */

htc_status_t
htc_host_shutdown(HTC_TARGET *target);
/* target - IN */

htc_status_t
htc_host_tx_data_cleanup(HTC_TARGET *target);
/* target - IN */

htc_status_t htc_host_get_num_htctargets_detected (uint32 *htc_devices);
/* htc_devices - IN */

htc_status_t htc_host_get_device_list (HTC_TARGET **handles_buf, uint32 buf_size);
/* handles_buf - IN, buf_size - IN */

htc_status_t htc_host_dump_target_registers(HTC_TARGET *target);

#ifdef __cplusplus
}
#endif

#endif /* _HTC_H_ */
