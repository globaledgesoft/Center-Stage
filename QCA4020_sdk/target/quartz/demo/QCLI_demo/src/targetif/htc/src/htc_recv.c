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


#include "com_dtypes.h"
#include "qapi_types.h"
#include "osal_types.h"
#include "htc_defs.h"
#include "htc_host_reg.h"
#include "htc.h"
#include "transport.h"
#include "htc_internal.h"
#include "hif.h"



/* ------ Global Variable Declarations ------- */
#ifdef DEBUG
extern OSAL_UINT32 debughtc;
#endif

/* ------ Static Variables ------ */


/* ------ Functions ------ */
/* Makes a buffer available to the HTC module */
htc_status_t htc_host_buffer_receive(HTC_TARGET *target, 
                 HTC_ENDPOINT_ID endPointId,
                 OSAL_UINT8 *buffer, 
                 OSAL_UINT32 length,
                 void *cookie)
{
    htc_status_t status;
    HTC_ENDPOINT *endPoint;
    HTC_DATA_REQUEST_QUEUE *recvQueue;

    HTC_DEBUG_PRINTF(HTC_DEBUG_TRC | HTC_DEBUG_RECV, 
            ("htc_host_buffer_receive: Enter (endPointId: %d, buffer: 0x%p, length: %d, cookie: 0x%p)\n", endPointId, buffer, length, cookie));

    HTC_DEBUG_ASSERT((endPointId >= ENDPOINT1) && (endPointId <= ENDPOINT4));

    /* Extract the end point instance */
    endPoint = &target->endPoint[endPointId];
    HTC_DEBUG_ASSERT(endPoint != NULL);

    recvQueue = &endPoint->recvQueue;
    HTC_DEBUG_PRINTF(HTC_DEBUG_INF | HTC_DEBUG_RECV, ("recvQueue: %p\n", 
                recvQueue));

    /* Add this posted buffer to the pending receive queue */
    status = addToEndpointQueue(target, recvQueue, buffer, length, 0, cookie);
    if (status != HTC_OK) {
        HTC_DEBUG_PRINTF(HTC_DEBUG_ERR | HTC_DEBUG_RECV,
                ("Mailbox (%d) Send queue full. Unable to add buffer\n", 
                 GET_ENDPOINT_ID(endPoint)));
        return HTC_ERROR;
    }

    /* 
     * If this API was called as a result of a HTC_DATA_AVAILABLE event to
     * the upper layer, indicating that HTC is out of buffers, then we should
     * receive the frame in the buffer supplied otherwise we simply add the 
     * buffer to the Pending Receive Queue 
     */
    if (endPoint->rxLengthPending) {
        htcReceiveFrame(endPoint);
    }

    HTC_DEBUG_PRINTF(HTC_DEBUG_TRC | HTC_DEBUG_RECV, 
            ("htc_host_buffer_receive: Exit\n"));
    return HTC_OK;
}

htc_status_t htcReceiveFrame(HTC_ENDPOINT *endPoint)
{
    htc_status_t status = HTC_OK;
    OSAL_UINT32 address;
    OSAL_UINT32 paddedLength;
    OSAL_UINT32 frameLength;
    OSAL_UINT32 request;
    HTC_ENDPOINT_ID endPointId;
    HTC_QUEUE_ELEMENT *element;
    HTC_ENDPOINT_BUFFER *endpointBuffer;
    HTC_DATA_REQUEST_QUEUE *recvQueue;
    HTC_TARGET *target;
    HTC_EVENT_INFO eventInfo;

    HTC_DEBUG_PRINTF(HTC_DEBUG_TRC | HTC_DEBUG_RECV, 
            ("htcReceiveFrame - Enter\n"));

    /* Get the context */
    HTC_DEBUG_ASSERT(endPoint != NULL);
    endPointId = GET_ENDPOINT_ID(endPoint);
    target = endPoint->target;
    HTC_DEBUG_ASSERT(target != NULL);
    recvQueue = &endPoint->recvQueue;
    HTC_DEBUG_ASSERT(recvQueue != NULL);

    /* 
     * Receive the frame if we have any pending frames and a buffer to
     * receive it into.
     */
    if (IS_DATA_QUEUE_EMPTY(recvQueue))
    {
        HTC_DEBUG_PRINTF(HTC_DEBUG_WARN | HTC_DEBUG_RECV,
                ("Mailbox (%d) recv queue empty. Unable to remove buffer\n", endPointId));

        /* 
         * Communicate this situation to the host via the HTC_DATA_AVAILABLE
         * event to request some buffers in the queue.
         */
        endPoint->rxLengthPending = htcGetFrameLength(endPoint);
        HTC_DEBUG_ASSERT(endPoint->rxLengthPending);
        FRAME_EVENT(eventInfo, NULL, endPoint->rxLengthPending, 
                0, HTC_OK, NULL);
        dispatchEvent(target, endPointId, HTC_DATA_AVAILABLE, &eventInfo);
        return status;
    }

    /* 
     * Get the length from the lookahead register if there is nothing 
     * pending.
     */
    if (endPoint->rxLengthPending)
    {
        frameLength = endPoint->rxLengthPending;
        endPoint->rxLengthPending = 0;
    } else
    {
        frameLength = htcGetFrameLength(endPoint);
    }
    HTC_DEBUG_ASSERT((frameLength > 0) && 
            (frameLength <= HTC_MESSAGE_SIZE_MAX));
    HTC_DEBUG_PRINTF(HTC_DEBUG_INF | HTC_DEBUG_RECV, ("Frame Length: %d\n", 
                frameLength));

    /* Adjust the length to be a multiple of block size if appropriate */
    paddedLength = (frameLength + (endPoint->blockSize - 1)) &
        (~(endPoint->blockSize - 1));

    /* 
     * Receive the frame(s). Pull an empty buffer from the head of the 
     * Pending Receive Queue.
     */
    element = removeFromEndpointQueue(target, recvQueue);
    endpointBuffer = GET_ENDPOINT_BUFFER(element);
    endpointBuffer->actualLength = paddedLength;
    request = (endPoint->blockSize > 1) ?
        HIF_RD_ASYNC_BLOCK_INC : HIF_RD_ASYNC_BYTE_INC;
    address = endPoint->address;
    status = HIFReadWrite(target->device, address, endpointBuffer->buffer, 
            endpointBuffer->actualLength, request, element);
#ifndef HTC_SYNC
    if (status != HTC_OK)
#else
    if (status != HTC_OK && status != HTC_PENDING)
#endif
    {
        HTC_DEBUG_PRINTF(HTC_DEBUG_ERR | HTC_DEBUG_RECV, 
                ("Frame reception failed\n"));
        if (!IS_ELEMENT_FREE(element))
        {
            endpointBuffer->actualLength = 0;
            FRAME_EVENT(eventInfo, endpointBuffer->buffer, 
                    endpointBuffer->bufferLength, endpointBuffer->actualLength, 
                    HTC_ECANCELED, endpointBuffer->cookie);
            RECYCLE_DATA_REQUEST_ELEMENT(element);
            dispatchEvent(target, endPointId, HTC_BUFFER_RECEIVED, 
                    &eventInfo);
            HTC_DEBUG_PRINTF(HTC_DEBUG_TRC | HTC_DEBUG_RECV, 
                    ("htcReceiveFrame - Exit\n"));
            return status;
        }
    }
#ifdef HTC_SYNC
    else if (status == HTC_OK)
    {
        element->completionCB(element, status);
    }
#endif

    HTC_DEBUG_PRINTF(HTC_DEBUG_TRC | HTC_DEBUG_RECV, 
            ("htcReceiveFrame - Exit\n"));

    return status;
}

OSAL_UINT32 htcGetFrameLength(HTC_ENDPOINT *endPoint)
{
    HTC_TARGET *target;
    OSAL_UINT32 frameLength;
    HTC_ENDPOINT_ID endPointId;
    HTC_TARGET_TRANSPORT trans;
    htc_status_t status;
    OSAL_UINT32 address;

    /* Get the context */
    HTC_DEBUG_ASSERT(endPoint != NULL);
    target = endPoint->target;
    HTC_DEBUG_ASSERT(target != NULL);
    endPointId = GET_ENDPOINT_ID(endPoint);

    getTargetTransport(target, &trans);

    if (trans == HTC_TRANSPORT_SPI)
    {
        address = getRegAddr(RX_LOOKAHEAD0_REG, ENDPOINT_UNUSED);
        status = HIFReadWrite(target->device, address, 
            (OSAL_UINT8*)&target->table.rx_lookahead[endPointId], 4, HIF_RD_SYNC_BYTE_INC, NULL);
        HTC_DEBUG_ASSERT(status == HTC_OK);
    }
    else
    {
        HTC_DEBUG_ASSERT(target->table.rx_lookahead_valid & (1 << endPointId));
    }  

    /* The length is contained in the first two bytes - HTC_HEADER_LEN */
    frameLength = (target->table.rx_lookahead[endPointId] & 0xFFFF) +
        HTC_HEADER_LEN;
    HTC_DEBUG_ASSERT(frameLength);
    HTC_DEBUG_PRINTF(HTC_DEBUG_WARN | HTC_DEBUG_RECV,
            ("frameLength = %x\n", frameLength));

    return frameLength;
}
