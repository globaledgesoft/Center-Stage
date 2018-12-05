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
#include "hif_internal.h"

/* ------ Global Variable Declarations ------- */

#ifdef DEBUG
extern OSAL_UINT32 debughtc;
#endif

/* ------ Functions ------ */
htc_status_t 
htc_host_buffer_send(HTC_TARGET *target, 
              HTC_ENDPOINT_ID endPointId,
              OSAL_UINT8 *buffer, 
              OSAL_UINT32 length,
              void *cookie)
{
    htc_status_t status;
    HTC_ENDPOINT *endPoint;
    HTC_DATA_REQUEST_QUEUE *sendQueue;
#ifdef NO_MORE_REQUIRED
#ifdef DEBUG
    HTC_DATA_REQUEST_ELEMENT *element;
#endif /* DEBUG */
#endif /* NO_MORE_REQUIRED */

    HTC_DEBUG_PRINTF(HTC_DEBUG_TRC | HTC_DEBUG_SEND, 
            ("htc_host_buffer_send: Enter (endPointId: %d, buffer: 0x%p, length: %d, cookie: 0x%p)\n", endPointId, buffer, length, cookie));

    HTC_DEBUG_ASSERT((endPointId >= ENDPOINT1) && (endPointId <= ENDPOINT4));
    HTC_DEBUG_ASSERT(length);

    /*
     * Disallow message > Endpoint-MBOX limit
     * It could have been truncated but let the application decide on the action
     * in such erroronous cases.
     */
    if (length > (HIF_ENDPOINT_WIDTH - HTC_HEADER_LEN)) {
         HTC_DEBUG_PRINTF(HTC_DEBUG_TRC | HTC_DEBUG_ERR, 
            ("htc_host_buffer_send: Max message length (%d) exceeded\n",
                HIF_ENDPOINT_WIDTH));
         return HTC_EMSGSIZE;
    }


    /* Extract the end point instance */
    endPoint = &target->endPoint[endPointId];
    HTC_DEBUG_ASSERT(endPoint != NULL);
    sendQueue = &endPoint->sendQueue;
    HTC_DEBUG_ASSERT(sendQueue != NULL);
    HTC_DEBUG_PRINTF(HTC_DEBUG_INF | HTC_DEBUG_SEND, 
            ("endpointQueue: %p\n", sendQueue));

    /* 
     * Add this posted buffer to the pending send queue. We need to allocate 
     * a bufferElement to store the packet information and we borrow that 
     * buffer from the pending send queue. If circumstances allow us to 
     * transmit it right away then we dequeue it otherwise we let it remain 
     * to be picked in the interrupt handler context.
     */

    if (!endPoint->enabled) {
#ifdef DEBUG
        debughtc = HTC_DEBUG_ANY;
#endif /* DEBUG */
        HTC_DEBUG_PRINTF(HTC_DEBUG_ERR, ("Endpoint not enabled: %d\n", 
                    GET_ENDPOINT_ID(endPoint)));
        return HTC_ERROR;
    }

    status = addToEndpointQueue(target, sendQueue, buffer, length, 0, cookie);
    if (status != HTC_OK) {
#ifdef DEBUG
        debughtc = HTC_DEBUG_ANY;
#endif /* DEBUG */
        HTC_DEBUG_PRINTF(HTC_DEBUG_ERR | HTC_DEBUG_SEND,
                ("Mailbox (%d) PSQ full. Unable to add buffer\n", 
                 endPointId));
        return HTC_ERROR;
    }

    /* 
     * The frame shall be dequeued and sent if there are any credits 
     * available. 
     */
    htcSendFrame(endPoint);

    HTC_DEBUG_PRINTF(HTC_DEBUG_TRC | HTC_DEBUG_SEND, ("htc_host_buffer_send: Exit\n"));
    return HTC_OK;
}


void 
htcSendFrame(HTC_ENDPOINT *endPoint) 
{
    htc_status_t status;
    OSAL_UINT32 address;
    HTC_TARGET *target;
    OSAL_UINT32 frameLength;
    OSAL_UINT32 paddedLength;
    HTC_EVENT_INFO eventInfo;
    OSAL_UINT32 request;
    OSAL_UINT8 txCreditsConsumed;
    OSAL_UINT8 txCreditsAvailable;
    HTC_ENDPOINT_ID endPointId;
    HTC_QUEUE_ELEMENT *element;
    HTC_ENDPOINT_BUFFER *endpointBuffer;
    HTC_REG_REQUEST_LIST *regList;
    HTC_DATA_REQUEST_QUEUE *sendQueue;
#ifdef HTC_SYNC
    HTC_REG_BUFFER *regBuffer;
#endif
    HTC_TARGET_TRANSPORT trans;

    HTC_DEBUG_PRINTF(HTC_DEBUG_TRC | HTC_DEBUG_SEND, ("htcSendFrame - Enter\n"));

    /* Get the context */
    HTC_DEBUG_ASSERT(endPoint != NULL);
    endPointId = GET_ENDPOINT_ID(endPoint);
    target = endPoint->target;
    HTC_DEBUG_ASSERT(target != NULL);
    sendQueue = &endPoint->sendQueue;
    HTC_DEBUG_ASSERT(sendQueue != NULL);
    regList = &target->regList;
    HTC_DEBUG_ASSERT(regList != NULL);

    getTargetTransport(target, &trans);

    /* 
     * Transmit the frames as long as we have the credits available and
     * the queue is not out of them 
     */
    HTC_DEBUG_PRINTF(HTC_DEBUG_SYNC,
                    ("Critical Section (credit): LOCK at line %d in file %s\n", __LINE__, __FILE__));
    OSAL_MUTEX_LOCK(&target->creditCS);
    txCreditsAvailable = GET_TX_CREDITS_AVAILABLE(endPoint);
    txCreditsConsumed = GET_TX_CREDITS_CONSUMED(endPoint);
    /* 
     * No need to modify the 'credit available' and 'credit consumed' count
     * since their shadow copies wont be updated in the while loop below.
     */
    if ((!txCreditsAvailable) || (target->target_state != HTC_TARGET_STARTED)) {
        OSAL_MUTEX_UNLOCK(&target->creditCS);
        return;
    }


    SET_TX_CREDITS_AVAILABLE(endPoint, 0);
    SET_TX_CREDITS_CONSUMED(endPoint, txCreditsConsumed + txCreditsAvailable);
    HTC_DEBUG_PRINTF(HTC_DEBUG_SYNC,
                    ("Critical Section (credit): UNLOCK at line %d in file %s\n", __LINE__, __FILE__));
    OSAL_MUTEX_UNLOCK(&target->creditCS);

    /* 
     * Send the packet only when there are packets to be sent and there
     * are positive number of credits available.
     */
    while((!IS_DATA_QUEUE_EMPTY(sendQueue)) && txCreditsAvailable)
    {
        /* Get the request buffer from the Pending Send Queue */
        element = removeFromEndpointQueue(target, sendQueue);
        endpointBuffer = GET_ENDPOINT_BUFFER(element);

        /* 
         * Prepend the actual length in the first 2 bytes of the outgoing 
         * packet.
         */
        endpointBuffer->buffer -= HTC_HEADER_LEN;
        OSAL_MEMCPY(endpointBuffer->buffer, &endpointBuffer->bufferLength, HTC_HEADER_LEN);

        /* 
         * Adjust the length in the block mode only when its not an integral 
         * multiple of the block size. Assumption is that the block size is
         * a power of 2.
         */
        frameLength = endpointBuffer->bufferLength + HTC_HEADER_LEN;
        paddedLength = (frameLength + (endPoint->blockSize - 1)) & 
            (~(endPoint->blockSize - 1));
        endpointBuffer->actualLength = paddedLength;
        HTC_DEBUG_PRINTF(HTC_DEBUG_INF | HTC_DEBUG_SEND,  
                ("Original frame length: %d, Padded frame length: %d\n", frameLength, paddedLength));

        HTC_DEBUG_PRINTBUF(endpointBuffer->buffer, endpointBuffer->actualLength);
        HTC_DEBUG_PRINTF(HTC_DEBUG_INF | HTC_DEBUG_SEND,  
                ("Available Tx credits: %d\n", txCreditsAvailable));

        /* Create the interface request */
        request = (endPoint->blockSize > 1) ?
            HIF_WR_ASYNC_BLOCK_INC : HIF_WR_ASYNC_BYTE_INC;
        address = endPoint->address;
        /* Send the data to the bus driver */
        status = HIFReadWrite(target->device, address, endpointBuffer->buffer, 
                endpointBuffer->actualLength, request, element);
#ifndef HTC_SYNC
        if (status != HTC_OK)
#else
            if (status != HTC_OK && status != HTC_PENDING)
#endif
            {
                /* DEBUG Start */
                if ((endpointBuffer->buffer == NULL) || (endpointBuffer->cookie == NULL) ||
                        (endpointBuffer->bufferLength == 0) || (element->buffer.free))
                {
                    HTC_DEBUG_PRINTF(HTC_DEBUG_INF, ("(hSF)element: 0x%p, endpointBuffer: 0x%p, status: %d, free: %d\n", element, endpointBuffer, status, element->buffer.free));
                    printEndpointQueueElement(element);
                    printEndpointQueue(&endPoint->sendQueue);
                }
                /* DEBUG End */

                HTC_DEBUG_PRINTF(HTC_DEBUG_ERR | HTC_DEBUG_SEND, 
                        ("Frame transmission failed\n"));
                HTC_DEBUG_PRINTF(HTC_DEBUG_ERR | HTC_DEBUG_SEND, 
                        ("EndPoint: %d, Tx credits available: %d\n", 
                         endPointId, GET_TX_CREDITS_AVAILABLE(endPoint)));
                /* 
                 * We need to check just in case the callback routine was called
                 * with the error status before we reach this point and in that
                 * context we fee up the buffer so its just a conservative design.
                 */
                if (!IS_ELEMENT_FREE(element)) {
                    endpointBuffer->buffer += HTC_HEADER_LEN;
                    FRAME_EVENT(eventInfo, endpointBuffer->buffer, 
                            endpointBuffer->bufferLength, 
                            endpointBuffer->actualLength, 
                            HTC_ECANCELED, endpointBuffer->cookie);
                    RECYCLE_DATA_REQUEST_ELEMENT(element);
                    dispatchEvent(target, endPointId, HTC_BUFFER_SENT, &eventInfo);
                }
                HTC_DEBUG_PRINTF(HTC_DEBUG_TRC | HTC_DEBUG_SEND, 
                        ("htcSendFrame - Exit\n"));
                return;
            }
#ifdef HTC_SYNC
            else if (status == HTC_OK) {
                element->completionCB(element, status);
            }
#endif
        txCreditsAvailable -= 1;
        txCreditsConsumed += 1;


        if ((txCreditsAvailable == 0) && (trans != HTC_TRANSPORT_SPI))
        {
            HTC_DEBUG_ASSERT(txCreditsConsumed);

            /* 
             * Instead of taking an interrupt we can just poll for more
             * credits that might have been queued up by now.
             */
            address = getRegAddr(TX_CREDIT_COUNTER_DECREMENT_REG, endPointId);
            element = allocateRegRequestElement(target);
            HTC_DEBUG_ASSERT(element != NULL);
            FILL_REG_BUFFER(element, (OSAL_UINT8*)&endPoint->txCreditsAvailable[1], 
                    txCreditsConsumed, TX_CREDIT_COUNTER_DECREMENT_REG,
                    endPointId);
            status = HIFReadWrite(target->device, address, 
                    (OSAL_UINT8*)&endPoint->txCreditsAvailable[1],
                    txCreditsConsumed, HIF_RD_ASYNC_BYTE_FIX, element); /* Murali: FIX to INC */
#ifndef HTC_SYNC
            HTC_DEBUG_ASSERT(status == HTC_OK);
            HTC_DEBUG_PRINTF(HTC_DEBUG_TRC | HTC_DEBUG_SEND, 
                    ("htcSendFrame - Exit\n"));
            return;
#else
            HTC_DEBUG_ASSERT(status == HTC_OK || status == HTC_PENDING);
            if ( status == HTC_OK ) {

                HTC_DEBUG_PRINTF(HTC_DEBUG_SYNC,
                        ("Critical Section (credit): LOCK at line %d in file %s							 \n", __LINE__, __FILE__));
                OSAL_MUTEX_LOCK(&target->creditCS);

                regBuffer = GET_REG_BUFFER(element);
                /* Calculate the number of credits available */
                HTC_DEBUG_ASSERT(GET_TX_CREDITS_CONSUMED(endPoint) == \
                        regBuffer->length);
                SET_TX_CREDITS_AVAILABLE(endPoint, regBuffer->buffer[0] - 
                        GET_TX_CREDITS_CONSUMED(endPoint));
                SET_TX_CREDITS_CONSUMED(endPoint, 0);
                txCreditsAvailable = GET_TX_CREDITS_AVAILABLE(endPoint);
                txCreditsConsumed = GET_TX_CREDITS_CONSUMED(endPoint);
                HTC_DEBUG_PRINTF(HTC_DEBUG_SYNC,
                        ("Critical Section (credit): UNLOCK at line %d in file %s\n", __LINE__, __FILE__));
                OSAL_MUTEX_UNLOCK(&target->creditCS);

                HTC_DEBUG_PRINTF(HTC_DEBUG_INF | HTC_DEBUG_SEND, 
                        ("Pulling %d tx credits from the target\n", 
                         txCreditsAvailable));

                freeRegRequestElement(element);

                if (!txCreditsAvailable) {

                    /* Enable the Tx credit counter interrupt so that we can get 
                     * the credits posted by the target */

                    htcEnableCreditCounterInterrupt(target, endPointId);

                    /* Counter Interrupts have been enabled if 
                     * txCreditsAvailable is still 0 after polling. We need to 
                     * return here as there is nothing we can send till we get 
                     * a Counter Interrupt.
                     */
                    return;
                }
            }
#endif
        }
        else
        {
            if (!txCreditsAvailable) {

                /* Enable the Tx credit counter interrupt so that we can get 
                 * the credits posted by the target */

                htcEnableCreditCounterInterrupt(target, endPointId);

                /* Counter Interrupts have been enabled if 
                 * txCreditsAvailable is still 0 after polling. We need to 
                 * return here as there is nothing we can send till we get 
                 * a Counter Interrupt.
                 */
                return;
            }
        }
    }

    HTC_DEBUG_PRINTF(HTC_DEBUG_SYNC,
                    ("Critical Section (credit): LOCK at line %d in file %s\n", __LINE__, __FILE__));
    OSAL_MUTEX_LOCK(&target->creditCS);
    SET_TX_CREDITS_AVAILABLE(endPoint, txCreditsAvailable);
    SET_TX_CREDITS_CONSUMED(endPoint, txCreditsConsumed);
    HTC_DEBUG_PRINTF(HTC_DEBUG_SYNC,
                    ("Critical Section (credit): UNLOCK at line %d in file %s\n", __LINE__, __FILE__));
    OSAL_MUTEX_UNLOCK(&target->creditCS);

    HTC_DEBUG_PRINTF(HTC_DEBUG_TRC | HTC_DEBUG_SEND, ("htcSendFrame - Exit\n"));
}

void
htcSendBlkSize(HTC_ENDPOINT *endPoint)
{
    htc_status_t status;
    OSAL_UINT32 address;
    HTC_TARGET *target;
    HTC_ENDPOINT_ID endPointId;
    HTC_QUEUE_ELEMENT *element;
    HTC_ENDPOINT_BUFFER *endpointBuffer;
    HTC_DATA_REQUEST_QUEUE *sendQueue;
    HTC_REG_REQUEST_LIST *regList;

    /* Get the context */
    HTC_DEBUG_ASSERT(endPoint != NULL);
    target = endPoint->target;
    HTC_DEBUG_ASSERT(target != NULL);
    regList = &target->regList;
    HTC_DEBUG_ASSERT(regList != NULL);
    sendQueue = &endPoint->sendQueue;
    HTC_DEBUG_ASSERT(sendQueue != NULL);
    endPointId = GET_ENDPOINT_ID(endPoint);

    /* Decrement the tx credit count */
    HTC_DEBUG_ASSERT(endPoint->txCreditsConsumed == 0);
    endPoint->txCreditsConsumed = 1;
    address = getRegAddr(TX_CREDIT_COUNTER_DECREMENT_REG, endPointId);
    element = allocateRegRequestElement(target);
    HTC_DEBUG_ASSERT(element != NULL);
    FILL_REG_BUFFER(element, (OSAL_UINT8 *)&endPoint->txCreditsAvailable[1],
                    endPoint->txCreditsConsumed,
                    TX_CREDIT_COUNTER_DECREMENT_REG, endPointId);
    status = HIFReadWrite(target->device, address, 
        (OSAL_UINT8*)&endPoint->txCreditsAvailable[1], 
        endPoint->txCreditsConsumed, HIF_RD_SYNC_BYTE_FIX, element);
#ifndef HTC_SYNC
    HTC_DEBUG_ASSERT(status == HTC_OK);
#else
    HTC_DEBUG_ASSERT(status == HTC_OK || status == HTC_PENDING);
	if (status == HTC_OK) {
		element->completionCB(element, status);
	}
#endif

    /* Negotiate the maximum block size for the endpoint */
    addToEndpointQueue(target, sendQueue, (OSAL_UINT8 *)&endPoint->blockSize, 
                   sizeof(endPoint->blockSize), sizeof(endPoint->blockSize), 
                   NULL);
    element = removeFromEndpointQueue(target, sendQueue);
    element->completionCB = htcBlkSzNegCompletionCB;
    endpointBuffer = GET_ENDPOINT_BUFFER(element);
    address = endPoint->address;
    status = HIFReadWrite(target->device, address, endpointBuffer->buffer, 
        endpointBuffer->bufferLength, HIF_WR_SYNC_BYTE_INC, element);
#ifndef HTC_SYNC
    HTC_DEBUG_ASSERT(status == HTC_OK);
#else
	HTC_DEBUG_ASSERT(status == HTC_OK || status == HTC_PENDING);
	if (status == HTC_OK) {
		element->completionCB(element, status);
	}
#endif

    HTC_DEBUG_PRINTF(HTC_DEBUG_INF | HTC_DEBUG_SEND, 
                    ("Mailbox(%d), Block size: %d\n", 
                    endPointId, endPoint->blockSize));
}
