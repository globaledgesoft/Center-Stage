#ifndef __HTC_SLAVE_API_H__
#define __HTC_SLAVE_API_H__

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

/*********************************************************************
 *
 * @file  htc_slave_api.h
 * @brief HTC Slave APIs
 * Copyright (c) 2017
 * Qualcomm Technologies Incorporated.
 * All Rights Reserved.
 */

/*====================================================================

                              EDIT HISTORY 

  $Header: 

====================================================================*/


/*
 * Structure for use by HTC to track buffers, and for caller and
 * HTC to communicate about buffers.  The caller passes in a
 * pointer to one of these on every send and every receive.  There
 * must be a unique HTC_BUFFER for each buffer that is passed
 * in.  HTC owns the HTC_BUFFER from the time it is passed in
 * until the time a send/receive completion is indicated, and the
 * caller must not modify it during that time.
 */

/*
 * Note: Application shall not have more than NUM_BUFFERS_PER_ENDPOINT instances
 * HTC_BUFFER. But application can recycle these instances once its done with
 * its callback handling for transmission/reception.
 */
typedef struct HTC_bufinfo_s {
    struct HTC_bufinfo_s  *next;          /**< HTC_BUFFER list linkage field;
                                              used by HTC when HTC owns bufinfo,
                                              used by the caller when the caller
                                              owns bufinfo. */
    uint8                 *buffer;        /**< Pointer to the data buffer.
                                              Reserves headroom for HTC header
                                              at the start of message
                                              sent/received to/from Host.
                                              Shall not be modified by HTC. */
    uint16                actual_length; /**< Actual payload length in bytes.
                                              In recv_done, this indicates the
                                              valid payload bytes in the buffer,
                                              which is filled by HTC layer.
                                              In send, this indicates to HTC the 
                                              number of payload bytes to be sent
                                              to the host, and shall be
                                              appropriately filled by the
                                              caller. Also considering the
                                              cases when message sent/received
                                              is more than block size. */
    uint8               htc_flags;      /**< HTC flags; internal to HTC only */
    uint8               end_point;      /**< Endpoint to which this buffer
                                              belongs. */
    uint16              buffer_offset;  /**< Buffer offset reserves headroom
                                              for the HTC header.
                                              buffer_offset set to
                                              QAPI_HTC_HDR_SZ only in the start
                                              frame of the message whereas for
                                              subsequent frames of the same
                                              message it is set to zero.
                                              In send, the caller shall set
                                              buffer_offset to QAPI_HTC_HDR_SZ
                                              for message bigger than block size
                                              to be sent to the Host.
                                              In receive, the caller shall be
                                              indicated about the headroom
                                              reserved for HTC header when a
                                              message bigger than block size is
                                              received from the Host.
                                              The offset is the distance between
                                              the current position and the area
                                              reserved for the HTC headroom. */
    uint16              app_context;    /**< Application context. */
} HTC_BUFFER;

/* Function status codes. */
typedef enum
{
    HTC_SLAVE_SUCCESS = 0,
    HTC_SLAVE_ERROR_INVALID_PARAMETER,
    HTC_SLAVE_ERROR_ALLOCATION,
    HTC_SLAVE_ENDPOINT_ERROR
} HTC_SLAVE_STATUS;


/* HTC Slave Mobx HW ID */
typedef enum
{
    HTC_SLAVE_ENDPOINT_ID0 = 0,
    HTC_SLAVE_ENDPOINT_ID1 = 1,
    HTC_SLAVE_ENDPOINT_ID2 = 2,
    HTC_SLAVE_ENDPOINT_ID3 = 3,
    HTC_SLAVE_MAX_ENDPOINTS
} HTC_SLAVE_ENDPOINT_ID;

 /* HTC Slave Event ID */
 typedef enum
 {
     HTC_SLAVE_BUFFER_RECEIVED,
     HTC_SLAVE_BUFFER_SENT,
     HTC_SLAVE_INIT_COMPLETE,
     HTC_SLAVE_RESET_COMPLETE,     
     HTC_SLAVE_MAX_EVENTS
 } HTC_SLAVE_EVENT_ID;

 /* HTC instance IDs */
 typedef enum {
     HTC_SDIO_SLAVE_INSTANCE_ID = 0,   /* default host interface mailbox */
     HTC_SPI_SLAVE_INSTANCE_ID,
     HTC_MAX_INSTANCES
 } HTC_INSTANCE_IDS;

 /*
 * Structure for use by HTC to track buffers, and for caller and
 * HTC to communicate about buffers.  The caller passes in a
 * pointer to one of these on every send and every receive.  There
 * must be a unique HTC_bufinfo_s for each buffer that is passed
 * in.  HTC owns the HTC_bufinfo_s from the time it is passed in
 * until the time a send/receive completion is indicated, and the
 * caller must not modify it during that time.
 */
typedef HTC_BUFFER HTC_SLAVE_BUFFER;

typedef void (*htc_callback_pfn_t)(uint32 Instance, uint32 arg1, uint32 arg2);


#define NUM_BUFFERS_PER_ENDPOINT    8
#endif /* __HTC_SLAVE_API_H__ */
