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

htc_status_t htc_update_target_state (HTC_TARGET *target)
{
    htc_status_t status;
    OSAL_UINT32 address;
	
    address = getRegAddr(SCRATCH_REG, ENDPOINT_UNUSED);
    status = HIFReadWrite(target->device, address, 
        (OSAL_UINT8*)&target->table.scratch[0], 2, HIF_RD_SYNC_BYTE_INC, NULL);	
	
	/* Update target state */
	target->target_state = target->table.scratch[HTC_SCRATCH_TARGET_STATE];
    return status;
}

htc_status_t htc_host_target_shutdown_ack(HTC_TARGET *target)
{
    OSAL_UINT32 address;
    HTC_REG_REQUEST_ELEMENT *regElement;
    htc_status_t status;

    /* Send the INT_WLAN interrupt to the target */
    target->table.int_wlan = (1<< HTC_INT_TARGET_SHUTDOWN_HOST_ACK);
    address = getRegAddr(INT_WLAN_REG, ENDPOINT_UNUSED);
    regElement = allocateRegRequestElement(target);
    HTC_DEBUG_ASSERT(regElement != NULL);
    FILL_REG_BUFFER(regElement, &target->table.int_wlan, 1,
            INT_WLAN_REG, ENDPOINT_UNUSED);
    status = HIFReadWrite(target->device, address,
            &target->table.int_wlan,
            1, HIF_WR_SYNC_BYTE_FIX, regElement);
#ifndef HTC_SYNC
    HTC_DEBUG_ASSERT(status == HTC_OK);
#else
    /* HTC_DEBUG_ASSERT(status == HTC_OK || status == HTC_PENDING); */
     if(regElement->completionCB != NULL)
     {
         regElement->completionCB(regElement, status);
     }
#endif

	/* Intimate target that it can shutdown as host has finished doing all the IO */
	/* This is required to avert the race condition of host continuing to do an IO on the dead target */

    target->table.scratch[HTC_SCRATCH_TARGET_STATE] = HTC_TARGET_SHUTDOWN_READY;

    address = getRegAddr(SCRATCH_REG, ENDPOINT_UNUSED);
    status = HIFReadWrite(target->device, address, 
        (OSAL_UINT8*)&target->table.scratch[0], 2, HIF_WR_SYNC_BYTE_INC, NULL);	

    return status;
}

/* ------ Functions ------ */
htc_status_t htcRWCompletionHandler(void *context, 
                       htc_status_t status)
{
    HTC_QUEUE_ELEMENT *element;

    HTC_DEBUG_PRINTF(HTC_DEBUG_TRC,("%s status %d\n", __func__, status));

    element = (HTC_QUEUE_ELEMENT *)context;
    HTC_DEBUG_ASSERT(element != NULL);

    return (element->completionCB(element, status));
}


htc_status_t htcTxCompletionCB(HTC_DATA_REQUEST_ELEMENT *element,
                  htc_status_t status) 
{
    HTC_TARGET *target;
    HTC_ENDPOINT_ID endPointId;
    HTC_ENDPOINT *endPoint;
    HTC_EVENT_INFO eventInfo;
    HTC_ENDPOINT_BUFFER *endpointBuffer;

    HTC_DEBUG_PRINTF(HTC_DEBUG_TRC | HTC_DEBUG_SEND, 
            ("htcTxCompletionCB - Enter\n"));

    /* Get the context */
    endpointBuffer = GET_ENDPOINT_BUFFER(element);
    HTC_DEBUG_ASSERT(endpointBuffer != NULL);
    endPoint = endpointBuffer->endPoint;
    HTC_DEBUG_ASSERT(endPoint != NULL);
    target = endPoint->target;
    HTC_DEBUG_ASSERT(target != NULL);
    endPointId = GET_ENDPOINT_ID(endPoint);

    HTC_DEBUG_PRINTF(HTC_DEBUG_INF | HTC_DEBUG_SEND,
            ("endpointBuffer: 0x%p, buffer: 0x%p, endPoint(%d): 0x%p, target: 0x%p\n", endpointBuffer, endpointBuffer->buffer, endPointId, endPoint, target));

    if ((endpointBuffer->buffer == NULL) || (endpointBuffer->cookie == NULL) ||
            (endpointBuffer->bufferLength == 0) || (element->buffer.free))
    {
        HTC_DEBUG_PRINTF(HTC_DEBUG_INF, ("DR: 0x%p\n", element));
        return HTC_OK;
    }

    /* Return the buffer to the user if the transmission was not successful */
    if (status != HTC_OK)
    {
        HTC_DEBUG_PRINTF(HTC_DEBUG_ERR | HTC_DEBUG_SEND, 
                ("Frame transmission failed\n"));
        HTC_DEBUG_PRINTF(HTC_DEBUG_ERR | HTC_DEBUG_SEND, 
                ("EndPoint: %d, Tx credits available: %d\n", 
                 endPointId, GET_TX_CREDITS_AVAILABLE(endPoint)));
        /* 
         * In the failure case it is possible that while queueing of the 
         * request itself it returned an error status in which case we 
         * would have dispatched an event and freed the element there 
         * itself. Ideally if it failed to queue the request then it 
         * should not generate a callback but we are being a little 
         * conservative.
         */
        if (!(IS_ELEMENT_FREE(element))) {
            endpointBuffer->buffer += HTC_HEADER_LEN;
            FRAME_EVENT(eventInfo, endpointBuffer->buffer, 
                    endpointBuffer->bufferLength, endpointBuffer->actualLength, 
                    HTC_ECANCELED, endpointBuffer->cookie);
            RECYCLE_DATA_REQUEST_ELEMENT(element);
            dispatchEvent(target, endPointId, HTC_BUFFER_SENT, &eventInfo);
            HTC_DEBUG_PRINTF(HTC_DEBUG_TRC | HTC_DEBUG_SEND, 
                    ("htcTxCompletionCB - Exit\n"));
        }
        return HTC_OK;
    }

    HTC_DEBUG_PRINTF(HTC_DEBUG_INF | HTC_DEBUG_SEND, 
            ("Frame transmission complete\n"));

    /* 
     * The user should see the actual length and buffer length
     * to be the same. In case of block mode, we use the actual length
     * parameter to reflect the total number of bytes transmitted after
     * padding.
     */
    endpointBuffer->actualLength = endpointBuffer->bufferLength;
    endpointBuffer->buffer += HTC_HEADER_LEN;

    /* 
     * Return the transmit buffer to the user through the HTC_BUFFER_SENT 
     * event indicating that the frame was transmitted successfully.
     */
    FRAME_EVENT(eventInfo, endpointBuffer->buffer, endpointBuffer->bufferLength, 
            endpointBuffer->actualLength, HTC_OK, endpointBuffer->cookie);
    RECYCLE_DATA_REQUEST_ELEMENT(element);

    dispatchEvent(target, endPointId, HTC_BUFFER_SENT, &eventInfo);

    HTC_DEBUG_PRINTF(HTC_DEBUG_TRC | HTC_DEBUG_SEND, 
            ("htcTxCompletionCB - Exit\n"));

    return HTC_OK;
}

htc_status_t htcBlkSzNegCompletionCB(HTC_DATA_REQUEST_ELEMENT *element,
                        htc_status_t status)
{
    HTC_TARGET *target;
    HTC_ENDPOINT *endPoint;
    HTC_ENDPOINT_BUFFER *endpointBuffer;
    HTC_REG_REQUEST_ELEMENT *regElement;
    OSAL_UINT32 address;


    HTC_DEBUG_PRINTF(HTC_DEBUG_TRC, ("%s status %d + \n", __func__, status));
    /* Get the context */
    endpointBuffer = GET_ENDPOINT_BUFFER(element);
    HTC_DEBUG_ASSERT(endpointBuffer != NULL);
    endPoint = endpointBuffer->endPoint;
    HTC_DEBUG_ASSERT(endPoint != NULL);
    target = endPoint->target;
    HTC_DEBUG_ASSERT(target != NULL);

    /* Recycle the request element */
    RECYCLE_DATA_REQUEST_ELEMENT(element);
    element->completionCB = htcTxCompletionCB;

    if (status == HTC_OK)
    {
        /* Mark the state to be ready */
        endPoint->enabled = TRUE;

        HTC_DEBUG_PRINTF(HTC_DEBUG_TRC,("%s endpoint address %x enabled + \n", __func__, endPoint->address));
        /* Set the state of the target as ready */

        if (target->endPoint[ENDPOINT1].enabled &&
                target->endPoint[ENDPOINT2].enabled &&
                target->endPoint[ENDPOINT3].enabled &&
                target->endPoint[ENDPOINT4].enabled )
        {
            HTC_DEBUG_PRINTF(HTC_DEBUG_TRC,("%s All endpoint enabled + \n", __func__));
            /* Send the INT_WLAN interrupt to the target */
            target->table.int_wlan = 1;
            address = getRegAddr(INT_WLAN_REG, ENDPOINT_UNUSED);
            regElement = allocateRegRequestElement(target);
            HTC_DEBUG_ASSERT(regElement != NULL);
            FILL_REG_BUFFER(regElement, &target->table.int_wlan, 1, 
                    INT_WLAN_REG, ENDPOINT_UNUSED);
            status = HIFReadWrite(target->device, address, 
                    &target->table.int_wlan, 
                    1, HIF_WR_ASYNC_BYTE_FIX, regElement);
#ifndef HTC_SYNC
            HTC_DEBUG_ASSERT(status == HTC_OK);
#else
            HTC_DEBUG_ASSERT(status == HTC_OK || status == HTC_PENDING);
            if(status == HTC_OK) {
                regElement->completionCB(regElement, status);
            }
#endif
        }
    }

    return HTC_OK;
}

htc_status_t htcRxCompletionCB(HTC_DATA_REQUEST_ELEMENT *element, 
                  htc_status_t status)
{
    HTC_TARGET *target;
    HTC_ENDPOINT *endPoint;
    HTC_EVENT_INFO eventInfo;
    HTC_ENDPOINT_ID endPointId;
    HTC_ENDPOINT_BUFFER *endpointBuffer;
	
    HTC_DEBUG_PRINTF(HTC_DEBUG_TRC | HTC_DEBUG_SEND, 
            ("htcRxCompletionCB - Enter\n"));

    /* Get the context */
    endpointBuffer = GET_ENDPOINT_BUFFER(element);
    HTC_DEBUG_ASSERT(endpointBuffer != NULL);
    endPoint = endpointBuffer->endPoint;
    HTC_DEBUG_ASSERT(endPoint != NULL);
    target = endPoint->target;
    HTC_DEBUG_ASSERT(target != NULL);
    endPointId = GET_ENDPOINT_ID(endPoint);

    HTC_DEBUG_PRINTF(HTC_DEBUG_INF | HTC_DEBUG_RECV,
            ("endpointBuffer: 0x%p, buffer: 0x%p, endPoint(%d): 0x%p, target: 0x%p\n", endpointBuffer, endpointBuffer->buffer, endPointId, endPoint, target));

    /* Return the buffer to the user if the reception was not successful */
    if (status != HTC_OK)
    {
        HTC_DEBUG_PRINTF(HTC_DEBUG_ERR | HTC_DEBUG_RECV, 
                ("Frame reception failed\n"));
        /* 
         * In the failure case it is possible that while queueing of the 
         * request itself it returned an error status in which case we 
         * would have dispatched an event and freed the element there 
         * itself. Ideally if it failed to queue the request then it 
         * should not generate a callback but we are being a little 
         * conservative.
         */
        if (!(IS_ELEMENT_FREE(element)))
        {
            endpointBuffer->actualLength = 0;
            endpointBuffer->buffer += HTC_HEADER_LEN;
            FRAME_EVENT(eventInfo, endpointBuffer->buffer, 
                    endpointBuffer->bufferLength, endpointBuffer->actualLength, 
                    HTC_ECANCELED, endpointBuffer->cookie);
            RECYCLE_DATA_REQUEST_ELEMENT(element);
            dispatchEvent(target, endPointId, HTC_BUFFER_RECEIVED, &eventInfo);
            HTC_DEBUG_PRINTF(HTC_DEBUG_TRC | HTC_DEBUG_RECV, 
                    ("htcRxCompletionCB - Exit\n"));
        }
        return HTC_OK;
    }

    HTC_DEBUG_PRINTF(HTC_DEBUG_INF | HTC_DEBUG_RECV, 
            ("Frame reception complete\n"));

    HTC_DEBUG_PRINTBUF(endpointBuffer->buffer, endpointBuffer->actualLength);

    /* 
     * Advance the pointer by the size of HTC header and pass the payload
     * pointer to the upper layer.
     */
    endpointBuffer->actualLength = ((endpointBuffer->buffer[0] << 0) | 
            (endpointBuffer->buffer[1] << 8));
    endpointBuffer->buffer += HTC_HEADER_LEN;

    /* 
     * Frame the HTC_BUFFER_RECEIVED to the upper layer indicating that the 
     * packet has been succesfully received.
     */
    FRAME_EVENT(eventInfo, endpointBuffer->buffer, endpointBuffer->bufferLength, 
            endpointBuffer->actualLength, HTC_OK, endpointBuffer->cookie);

    /* Recycle the bufferElement structure */
    RECYCLE_DATA_REQUEST_ELEMENT(element);

    /* Dispatch the event */
    dispatchEvent(target, endPointId, HTC_BUFFER_RECEIVED, &eventInfo);

    HTC_DEBUG_PRINTF(HTC_DEBUG_TRC | HTC_DEBUG_RECV, 
            ("htcRxCompletion - Exit\n"));

    return HTC_OK;
}

htc_status_t htcRegCompletionCB(HTC_REG_REQUEST_ELEMENT *element,
                   htc_status_t status) 
{
    htc_status_t ret;
    HTC_TARGET *target;
    HTC_ENDPOINT *endPoint;
    HTC_REG_BUFFER *regBuffer;
    OSAL_UINT8 txCreditsConsumed;
    OSAL_UINT8 txCreditsAvailable;
    HTC_ENDPOINT_ID endPointId;
	
    HTC_DEBUG_PRINTF(HTC_DEBUG_TRC | HTC_DEBUG_RECV | HTC_DEBUG_SEND, 
            ("htcRegCompletion - Enter\n"));

    if (status != HTC_OK)
    {
        /* Free the register request structure */
        freeRegRequestElement(element);
        HTC_DEBUG_PRINTF(HTC_DEBUG_ERR, ("Register request failed\n"));
        htcReportFailure(HTC_EPROTO);
        return HTC_OK;
    }

    /* Get the context */
    HTC_DEBUG_ASSERT(element != NULL);
    regBuffer = GET_REG_BUFFER(element);
    HTC_DEBUG_ASSERT(regBuffer != NULL);
    target = regBuffer->target;
    HTC_DEBUG_ASSERT(target != NULL);

    /* Identify the register and the operation responsible for the callback */
    ret = HTC_OK;
    switch(regBuffer->base)
    {
        case TX_CREDIT_COUNTER_DECREMENT_REG:
            HTC_DEBUG_PRINTF(HTC_DEBUG_INF, ("TX_CREDIT_COUNTER_DECREMENT_REG\n"));
            endPointId = regBuffer->offset;
            endPoint = &target->endPoint[endPointId];

            HTC_DEBUG_PRINTF(HTC_DEBUG_SYNC,
                    ("Critical Section (credit): LOCK at line %d in file %s\n", __LINE__, __FILE__));
            OSAL_MUTEX_LOCK(&target->creditCS);

            /* Calculate the number of credits available */
            HTC_DEBUG_ASSERT(GET_TX_CREDITS_CONSUMED(endPoint) == regBuffer->length);
            HTC_DEBUG_ASSERT(GET_TX_CREDITS_CONSUMED(endPoint) > 0);
            HTC_DEBUG_ASSERT(regBuffer->buffer[0] >= 
                    GET_TX_CREDITS_CONSUMED(endPoint));
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

            if (txCreditsConsumed) {}

            if (txCreditsAvailable)
            {
                htcSendFrame(endPoint);
            } else
            {
                /* 
                 * Enable the Tx credit counter interrupt so that we can get the
                 * credits posted by the target.
                 */
                htcEnableCreditCounterInterrupt(target, endPointId);
            }
            break;

        case TX_CREDIT_COUNTER_RESET_REG:
            HTC_DEBUG_PRINTF(HTC_DEBUG_INF, ("TX_CREDIT_COUNTER_RESET_REG\n"));
            endPointId = regBuffer->offset;

            /* 
             * Enable the Tx credit counter interrupt so that we can get the
             * credits posted by the target.
             */
            htcEnableCreditCounterInterrupt(target, endPointId);
            break;

        case COUNTER_INT_STATUS_ENABLE_REG:
            HTC_DEBUG_PRINTF(HTC_DEBUG_INF, ("COUNTER_INT_STATUS_ENABLE: 0x%x\n",
                        target->table.counter_int_status_enable));
            break;

        case COUNTER_INT_STATUS_DISABLE_REG:
            HTC_DEBUG_PRINTF(HTC_DEBUG_INF, ("COUNTER_INT_STATUS_DISABLE:0x%x\n",
                        target->table.counter_int_status_enable));
                HIFAckInterrupt(target->device);
                HTC_DEBUG_PRINTF(HTC_DEBUG_TRC, ("htcDSRHandler - ACK\n"));
            break;
        case INT_WLAN_REG:
            HTC_DEBUG_PRINTF(HTC_DEBUG_INF, ("INT_WLAN: 0x%x\n",
                        target->table.int_wlan));
            target->table.int_wlan = 0;
            /* This event is also used to reset target */
            /* So, if target is ready, wich means, it is reset event, so donot signal the event*/

            if ( target->ready != TRUE) {
                /* Mark the target state as ready and signal the waiting sem */
                target->ready = TRUE;

                /* Send blk size neg. completion signal to htc start task */
                qurt_signal_set(&(target->htcEvent), HTC_BLK_SIZE_NEG_COMPLETE_EVT);
            }

            break;

        case INT_STATUS_ENABLE_REG:
            HTC_DEBUG_PRINTF(HTC_DEBUG_INF,("INT_STATUS_ENABLE: 0x%x\n",
                        target->table.int_status_enable));
            break;
            
        case SCRATCH_REG:
            HTC_DEBUG_PRINTF(HTC_DEBUG_INF,("SCRATCH_REG: 0x%x\n",
                        target->table.scratch));
            break;
        default:
            HTC_DEBUG_PRINTF(HTC_DEBUG_ERR, 
                    ("Invalid register address: %d\n", regBuffer->base));
    }

    /* Free the register request structure */
    freeRegRequestElement(element);

    HTC_DEBUG_PRINTF(HTC_DEBUG_TRC, ("htcRegCompletion - Exit\n"));

    return ret;
}

htc_status_t htcTargetInsertedHandler(void *context, void *hif_handle, OSAL_BOOL hotplug)
{
    HTC_TARGET *target;
    HTC_ENDPOINT *endPoint;
    OSAL_UINT8 count1, count2;
    HTC_EVENT_INFO eventInfo;
    HTC_REG_BUFFER *regBuffer;
    HTC_QUEUE_ELEMENT *element;
    HTC_ENDPOINT_BUFFER *endpointBuffer;
    HTC_REG_REQUEST_LIST *regList;
    HTC_DATA_REQUEST_QUEUE *sendQueue, *recvQueue;
    OSAL_UINT32 address[HTC_MAILBOX_NUM_MAX];
    OSAL_UINT32 blockSize[HTC_MAILBOX_NUM_MAX];
    HTC_CALLBACKS htcCallbacks;
    htc_status_t status = HTC_OK;

    HTC_DEBUG_PRINTF(HTC_DEBUG_TRC, ("htcTargetInserted - Enter\n"));

    /* Allocate target memory */
    if ((target = (HTC_TARGET *)OSAL_MALLOC(sizeof(HTC_TARGET))) == NULL) {
        HTC_DEBUG_PRINTF(HTC_DEBUG_ERR, ("Unable to allocate memory\n"));
        return HTC_ERROR;
    }
    OSAL_MEMZERO(target, sizeof(HTC_TARGET));
    target->device = hif_handle;
    target->ready = FALSE;

    /* Init Block size negotiation completion event */
    qurt_signal_init(&target->htcEvent);

    /* Initialize the locks */
    OSAL_MUTEX_INIT(&target->counterCS);
    OSAL_MUTEX_INIT(&target->instanceCS);
    OSAL_MUTEX_INIT(&target->creditCS);

    /* Give a handle to HIF for this target */
    HIFSetHandle(hif_handle, (void *) target);



    /* Register htc_callbacks */
    OSAL_MEMZERO(&htcCallbacks, sizeof(HTC_CALLBACKS));
    /* the device layer handles these */
    htcCallbacks.rwCompletionHandler =htcRWCompletionHandler;
    htcCallbacks.dsrHandler = htcDSRHandler;
    htcCallbacks.context = target;
    status = HIFAttachHTC(hif_handle, &htcCallbacks);
    /* Claim the device */
    HIFClaimDevice(hif_handle, target);


    /* Initialize the endpoints, endpoint queues, event table */
    for (count1 = ENDPOINT1; count1 <= ENDPOINT4; count1 ++) {
        endPoint = &target->endPoint[count1];
        HTC_DEBUG_PRINTF(HTC_DEBUG_INF, 
                ("endPoint[%d]: %p\n", count1, endPoint));

        OSAL_MEMZERO(endPoint->txCreditsAvailable, HTC_TX_CREDITS_NUM_MAX);
        endPoint->txCreditsConsumed = 0;
        endPoint->txCreditsIntrEnable = FALSE;
        endPoint->rxLengthPending = 0;
        endPoint->target = target;
        endPoint->enabled = FALSE;
        for (count2 = 0; count2<HTC_DATA_REQUEST_RING_BUFFER_SIZE; count2 ++) {
            /* Send Queue */
            sendQueue = &endPoint->sendQueue;
            sendQueue->head = sendQueue->size = 0;
            element = &sendQueue->element[count2];
            OSAL_MEMZERO(element, sizeof(HTC_DATA_REQUEST_ELEMENT));
            element->buffer.free = TRUE;
            element->completionCB = htcTxCompletionCB;
            endpointBuffer = GET_ENDPOINT_BUFFER(element);
            endpointBuffer->endPoint = endPoint;

            /* Receive Queue */
            recvQueue = &endPoint->recvQueue;
            recvQueue->head = recvQueue->size = 0;
            element = &recvQueue->element[count2];
            OSAL_MEMZERO(element, sizeof(HTC_DATA_REQUEST_ELEMENT));
            element->buffer.free = TRUE;
            element->completionCB = htcRxCompletionCB;
            endpointBuffer = GET_ENDPOINT_BUFFER(element);
            endpointBuffer->endPoint = endPoint;
        }
        OSAL_MEMZERO(&target->endPoint[count1].eventTable, 
                sizeof(HTC_ENDPOINT_EVENT_TABLE));
    }

    /* Populate the block size for each of the end points */
    HIFConfigureDevice((HIF_DEVICE *) hif_handle, HIF_DEVICE_GET_ENDPOINT_BLOCK_SIZE, 
            &blockSize, sizeof(blockSize));
    HIFConfigureDevice((HIF_DEVICE *) hif_handle, HIF_DEVICE_GET_ENDPOINT_ADDR, 
            &address, sizeof(address));
    for (count1 = ENDPOINT1; count1 <= ENDPOINT4; count1 ++) {
        endPoint = &target->endPoint[count1];
        endPoint->blockSize = blockSize[count1];
        endPoint->address = address[count1];
    }

    /* Initialize the shadow copy of the target register table */
    OSAL_MEMZERO(&target->table, sizeof(HTC_REGISTER_TABLE));

    /* Initialize the register request list */
    regList = &target->regList;
    for (count1 = 0; count1 < HTC_REG_REQUEST_LIST_SIZE; count1 ++) {
        element = &regList->element[count1];
        OSAL_MEMZERO(element, sizeof(HTC_REG_REQUEST_ELEMENT));
        element->buffer.free = TRUE;
        element->completionCB = htcRegCompletionCB;
        regBuffer = GET_REG_BUFFER(element);
        regBuffer->target = target;
    }

    /* Add the target instance to the global list */
    addTargetInstance(target);
    target->hotplug = hotplug;
    /* 
     * Frame a TARGET_AVAILABLE event and send it to the host. Return the
     * HIF_DEVICE handle as a parameter with the event.
     */
    FRAME_EVENT(eventInfo, (OSAL_UINT8 *)hif_handle, sizeof(HIF_DEVICE *), 
            sizeof(HIF_DEVICE *), HTC_OK, &target->hotplug);
    dispatchEvent(target, ENDPOINT_UNUSED, HTC_TARGET_AVAILABLE, &eventInfo);

    HTC_DEBUG_PRINTF(HTC_DEBUG_TRC, ("htcTargetInserted - Exit\n"));

    return status;
}



htc_status_t htcTargetRemovedHandler(void *context, void *hif_handle, OSAL_BOOL hotplug)
{
    HIF_DEVICE *device = (HIF_DEVICE *) hif_handle;
    HTC_TARGET *target;
    HTC_EVENT_INFO eventInfo;
    HTC_ENDPOINT_ID endPointId;
    HTC_ENDPOINT *endPoint;
    htc_status_t status = HTC_OK;

    HTC_DEBUG_ASSERT(device != NULL);
    target = (HTC_TARGET*)context;
    HTC_DEBUG_ASSERT(target != NULL);

    /* Destroy Block size negotiation completion event */
    qurt_signal_destroy(&target->htcEvent);

    /* Deinit all the locks */
    OSAL_MUTEX_DEINIT(&target->counterCS);
    OSAL_MUTEX_DEINIT(&target->instanceCS);
    OSAL_MUTEX_DEINIT(&target->creditCS);

    /* Disable each of the endpoints to stop accepting new packets */
    HTC_DEBUG_PRINTF(HTC_DEBUG_TRC,("%s Disabling control and data channels (Reason: %d)\n", __func__, status));
    HTC_DEBUG_PRINTF(HTC_DEBUG_ERR, ("Disabling control and data channels (Reason: %d)\n", status));
    for (endPointId = ENDPOINT1; endPointId <= ENDPOINT4; endPointId++) {
        endPoint = &target->endPoint[endPointId];
        endPoint->enabled = FALSE;
    }

    target->hotplug = hotplug;

    if (target != NULL) {
        /* Frame a TARGET_UNAVAILABLE event and send it to the host */
        FRAME_EVENT(eventInfo, NULL, 0, 0, status, &target->hotplug);
        dispatchEvent(target, ENDPOINT_UNUSED, HTC_TARGET_UNAVAILABLE, 
                &eventInfo);
    }

    /* Release the HIF device */
    HIFReleaseDevice(device);

    return HTC_OK;
}


htc_status_t htcDSRHandler(void *htc_handle)
{
    htc_status_t status;
    OSAL_UINT32 address;
    HTC_TARGET *target = (HTC_TARGET *) htc_handle;
    OSAL_UINT8 host_int_status;

    HTC_TARGET_TRANSPORT trans;
    OSAL_UINT32 spi_status = 0;

    HTC_DEBUG_ASSERT(target != NULL);
    HTC_DEBUG_PRINTF(HTC_DEBUG_TRC, 
            ("htcDsrHandler: Enter (target: 0x%p\n", target));

    /* 
     * Read the first 28 bytes of the HTC register table. This will yield us
     * the value of different int status registers and the lookahead 
     * registers.
     *    length = sizeof(int_status) + sizeof(cpu_int_status) + 
     *             sizeof(error_int_status) + sizeof(counter_int_status) + 
     *             sizeof(endpoint_frame) + sizeof(rx_lookahead_valid) + 
     *             sizeof(hole) +  sizeof(rx_lookahead) +
     *             sizeof(int_status_enable) + sizeof(cpu_int_status_enable) +
     *             sizeof(error_status_enable) + 
     *             sizeof(counter_int_status_enable);
     * 
     * For SPI WAR, async read is called in DSR, "SPI" macro is used to divide the 
     * different code path.
     */

    address = getRegAddr(INT_STATUS_REG, ENDPOINT_UNUSED);
    status = HIFReadWrite(target->device, address, 
            &target->table.host_int_status, 28, HIF_RD_SYNC_BYTE_INC, NULL);

	/* If there is an error in transport skip ISR processing */
    if(status != HTC_OK) return status;

    HTC_DEBUG_PRINTF(HTC_DEBUG_INF, 
            ("RAW Valid interrupt source(s) in INT_STATUS: 0x%x\n", 
             target->table.host_int_status));

#ifdef DEBUG
    dumpRegisters(target);
#endif /* DEBUG */

    /* Update only those registers that are enabled */
    host_int_status = target->table.host_int_status &
        target->table.int_status_enable;


    /*    HTC_DEBUG_ASSERT(host_int_status); */ /* TODO: this is zero during init, debug */
    HTC_DEBUG_PRINTF(HTC_DEBUG_INF, 
            ("Valid interrupt source(s) in INT_STATUS: 0x%x\n", 
             host_int_status));

    getTargetTransport(target, &trans);
    if (trans == HTC_TRANSPORT_SPI)
    {
        address = getRegAddr(SPI_STATUS_READ_REG, ENDPOINT_UNUSED);
        status = HIFReadWrite(target->device, address, 
                (OSAL_UINT8*)&target->table.spi_status, 4, HIF_RD_SYNC_BYTE_INC, NULL);
        spi_status = target->table.spi_status;
		
	    HTC_DEBUG_PRINTF(HTC_DEBUG_ERR, 
            ("Valid interrupt source(s) in SPI_STATUS : 0x%x\n", 
             spi_status));		
	
        if (spi_status & SPI_SLV_REG_INT_PACKET_AVAIL) {
            /* Mailbox Interrupt */
            htcServiceMailboxInterrupt(target);
        }


        /*
         * In SPI, a GPIO is used to signal a credit counter update interrrupt
         * from the Target. But this doesn't distinguish on which endpoint-MBox
         * this credit counter is updated.
         * So rely on COUNTER_INT_STATUS to figure this out. But at times its
         * found that COUNTER_INT_STATUS for the corresponding endpoint-MBox is
         * unset in spite of the interrupt signalled via the GPIO.
         * So for SPI, don't hard fail on COUNTER_INT_STATUS being unset but
         * rather proceed further and do an aggregation of the credit counter
         * for each endpoint based on the currently read credit counter values
         * since Target has already indicated that at least on one of the
         * endpoint-MBox, the credits are updated.
         */
        if (spi_status & SPI_SLV_REG_INT_COUNTER) {
            /* Credit counter  Interrupt */
            htcServiceCounterInterrupt(target);
        }        


        if (spi_status & SPI_SLV_REG_INT_ALL_CPU) {
            /* CPU Interrupt */
            htcServiceCPUInterrupt(target);
        }   

    }
    else
    {
        if (HOST_INT_STATUS_CPU_GET(host_int_status)) {
            /* CPU Interrupt */
            htcServiceCPUInterrupt(target);
        }
        
        if (HOST_INT_STATUS_ERROR_GET(host_int_status)) {
            /* Error Interrupt */
            htcServiceErrorInterrupt(target);
        }
    
        if (HOST_INT_STATUS_ENDPOINT_DATA_GET(host_int_status)) {
            /* Mailbox Interrupt */
            htcServiceMailboxInterrupt(target);
        }

        if (HOST_INT_STATUS_COUNTER_GET(host_int_status)) {
            /* Counter Interrupt */
            htcServiceCounterInterrupt(target);
        } else {
            /* Ack the interrupt */
            HIFAckInterrupt(target->device);
            HTC_DEBUG_PRINTF(HTC_DEBUG_TRC, ("htcDSRHandler - ACK\n"));
        }
     }

    HTC_DEBUG_PRINTF(HTC_DEBUG_TRC, ("htcDSRHandler: Exit\n"));
    return HTC_OK;
}

void htcServiceCPUInterrupt(HTC_TARGET *target)
{
    htc_status_t status;
    OSAL_UINT32 address;
    OSAL_UINT8 cpu_int_status;
	HTC_ENDPOINT *endPoint;
    HTC_ENDPOINT_ID endPointId;
	

    HTC_DEBUG_PRINTF(HTC_DEBUG_INF, ("CPU Interrupt\n"));
    cpu_int_status = target->table.cpu_int_status &
        target->table.cpu_int_status_enable;
    HTC_DEBUG_ASSERT(cpu_int_status);
    HTC_DEBUG_PRINTF(HTC_DEBUG_INF, 
            ("Valid interrupt source(s) in CPU_INT_STATUS: 0x%x\n",
             cpu_int_status));

    /* Clear the interrupt */
    target->table.cpu_int_status = cpu_int_status; /* W1C */
    address = getRegAddr(CPU_INT_STATUS_REG, ENDPOINT_UNUSED);
    status = HIFReadWrite(target->device, address, 
            &target->table.cpu_int_status, 1, HIF_WR_SYNC_BYTE_FIX, NULL);

	htc_update_target_state(target);

	/* Send any outstanding requests which got queued while target was down */
	if (target->target_state == HTC_TARGET_STARTED)
	{
	    for(endPointId = 0; endPointId < HTC_MAILBOX_NUM_MAX; endPointId++)
	    {
	        endPoint = &target->endPoint[endPointId];
	        htcSendFrame(endPoint);
	    }
	}

	if (target->target_state == HTC_TARGET_SHUTDOWN)
	{
		/* Send the acknowledgement first to the target */
		htc_host_target_shutdown_ack(target);

		/*
		 * Instead of informing the application about target removal to ensure
		 * that the application invokes HTC tear down path for that target,
		 * it would be better to trigger internally the relevant part of
		 * HTC tear down sequence. to synchronize state with Target.
		 */
        htc_host_shutdown(target);
	}

    HTC_DEBUG_ASSERT(status == HTC_OK);
}


void htcServiceErrorInterrupt(HTC_TARGET *target)
{
    htc_status_t status=HTC_ERROR;
    OSAL_UINT32 address;
    OSAL_UINT8 error_int_status;

    HTC_DEBUG_PRINTF(HTC_DEBUG_INF, ("Error Interrupt\n"));

    error_int_status = target->table.error_int_status &
        target->table.error_status_enable;
    HTC_DEBUG_ASSERT(error_int_status);
    HTC_DEBUG_PRINTF(HTC_DEBUG_INF, 
            ("Valid interrupt source(s) in ERROR_INT_STATUS: 0x%x\n",
             error_int_status));

    if (ERROR_INT_STATUS_WAKEUP_GET(error_int_status)) {
        /* Wakeup */
        HTC_DEBUG_PRINTF(HTC_DEBUG_INF, ("Wakeup\n"));
    }

    if (ERROR_INT_STATUS_RX_UNDERFLOW_GET(error_int_status)) {
        /* Rx Underflow */
        HTC_DEBUG_PRINTF(HTC_DEBUG_INF, ("Rx Underflow\n"));
    }

    if (ERROR_INT_STATUS_TX_OVERFLOW_GET(error_int_status)) {
        /* Tx Overflow */
        HTC_DEBUG_PRINTF(HTC_DEBUG_INF, ("Tx Overflow\n"));
    }

    /* Clear the interrupt */
    target->table.error_int_status = error_int_status; /* W1C */
    address = getRegAddr(ERROR_INT_STATUS_REG, ENDPOINT_UNUSED);
    status = HIFReadWrite(target->device, address, 
            &target->table.error_int_status, 1, HIF_WR_SYNC_BYTE_FIX, NULL);
    HTC_DEBUG_ASSERT(status == HTC_OK);
}

/*
 * ToDo:
 *      As update to credit counters is differently managed in case of SPI,
 *      better to have different handler for SPI.
 */
void htcServiceCounterInterrupt(HTC_TARGET *target)
{
    OSAL_UINT8 counter_int_status;
    HTC_TARGET_TRANSPORT trans;

    HTC_DEBUG_PRINTF(HTC_DEBUG_INF, ("Counter Interrupt\n"));
    counter_int_status = target->table.counter_int_status &
        target->table.counter_int_status_enable;

    /*
     * For SPI, don't hard fail on COUNTER_INT_STATUS being unset but
     * rather proceed further and do an aggregation of the credit counter
     * for each endpoint based on the currently read credit counter values
     * since Target has already indicated that at least on one of the
     * endpoint-MBox, the credits are updated.
     */
    getTargetTransport(target, &trans);
    if (trans != HTC_TRANSPORT_SPI)
    {
        HTC_DEBUG_ASSERT(counter_int_status);
    }

    HTC_DEBUG_PRINTF(HTC_DEBUG_INF,
            ("Valid interrupt source(s) in COUNTER_INT_STATUS: 0x%x\n",
             counter_int_status));

    /* Check if the debug interrupt is pending */
    if (counter_int_status & HTC_TARGET_DEBUG_INTR_MASK) {
        htcServiceDebugInterrupt(target);
    }

    /* Check if the credit interrupt is pending */
    if ((counter_int_status & HTC_TARGET_CREDIT_INTR_MASK) ||
        /* For SPI, can't fully rely on COUNTER_INT_STATUS as mentioned above */
        trans == HTC_TRANSPORT_SPI) {
        htcServiceCreditInterrupt(target);
    } else {
        /* Ack the interrupt */
        HIFAckInterrupt(target->device);
        HTC_DEBUG_PRINTF(HTC_DEBUG_TRC, ("htcDSRHandler - ACK\n"));
    }
}

void htcServiceDebugInterrupt(HTC_TARGET *target)
{
    OSAL_UINT8 dummy;
    htc_status_t status;
    OSAL_UINT32 address;

    /* Send a target failure event to the application */
    HTC_DEBUG_PRINTF(HTC_DEBUG_ERR, ("Target debug interrupt\n"));
    htcReportFailure(HTC_ENODEV);

    /* Clear the interrupt */
    address = getRegAddr(DEBUG_COUNTER_REG, ENDPOINT1);
    status = HIFReadWrite(target->device, address, 
            &dummy, 1, HIF_RD_SYNC_BYTE_INC, NULL);
    HTC_DEBUG_ASSERT(status == HTC_OK);
}

/*
 * ToDo:
 *      As update to credit counters is differently managed in case of SPI,
 *      better to have different handler for SPI.
 */
void htcServiceCreditInterrupt(HTC_TARGET *target)
{
    HTC_ENDPOINT *endPoint;
    HTC_ENDPOINT_ID endPointId;
    OSAL_UINT8 counter_int_status;
    OSAL_UINT8 update_credit_int_status = 0;
    OSAL_UINT32 address;
    htc_status_t status;
    OSAL_UINT8 txCreditsAvailable = 0;
    OSAL_UINT32 creditreceived = 0;
    HTC_TARGET_TRANSPORT trans;

    counter_int_status = target->table.counter_int_status &
        target->table.counter_int_status_enable;

    getTargetTransport(target, &trans);
    if (trans != HTC_TRANSPORT_SPI)
    {
       /*
        * We dont disable credit counter interrupts in SPI as SPI credit counter
        * interrupt does not have control on individual credit counters.
        */
        HTC_DEBUG_ASSERT(counter_int_status);
    }

    /* If Host comes up after target, we might have missed target interrupts */
    htc_update_target_state(target);

    /*
     * We dont disable credit counter interrupts in SPI as SPI credit counter
     * interrupt does not have control on individual credit counters.
     */
    if (trans != HTC_TRANSPORT_SPI)
    {
        /* Disable the credit counter interrupt */
        htcDisableCreditCounterInterrupt(target, ENDPOINT_UNUSED);

        /* Service the credit counter interrupt */
        update_credit_int_status =
                        (counter_int_status & HTC_TARGET_CREDIT_INTR_MASK) >> 4;
    }

    /* If we get here, at least one bit should be set. Loop through */
    /* each mailbox/endPointID. There are only a max of 4.          */
    for(endPointId = 0; endPointId < HTC_MAILBOX_NUM_MAX; endPointId++)
    {
        endPoint = &target->endPoint[endPointId];
        HTC_DEBUG_ASSERT(endPoint != NULL);
        /*
         * SPI credit int. can occur when credits != 0, as we dont disable
         * credit int. for spi
         */
        if (trans != HTC_TRANSPORT_SPI)
        {
            if(!(update_credit_int_status & (1 << endPointId)))
            {
                continue;
            }

            /* This is the minimum number of credits that we would have got */
            HTC_DEBUG_ASSERT(GET_TX_CREDITS_AVAILABLE(endPoint) == 0);
            SET_TX_CREDITS_AVAILABLE(endPoint, 1);
        }

        if (!target->ready) {
            htcSendBlkSize(endPoint);
        } else
        {
            /* Credit Counter aggregation done for SPI */
            if (trans == HTC_TRANSPORT_SPI)
            {
                address = getRegAddr(TX_CREDIT_COUNTER_DECREMENT_REG,
                                        endPointId);
                status = HIFReadWrite(target->device, address,
                                        (OSAL_UINT8*)&txCreditsAvailable, 1,
                                        HIF_RD_SYNC_BYTE_FIX, NULL);
                HTC_DEBUG_ASSERT(status == HTC_OK);

                creditreceived = txCreditsAvailable;
                HTC_DEBUG_PRINTF(HTC_DEBUG_ALERT, ("Tx Credits received: %d\n",
                                    creditreceived));

                /*
                 * Do a read of the equivalent local registers to confirm
                 * the credits are decremented.
                 */
                address = getRegAddr(TX_CREDIT_COUNTER_REG,
                                        endPointId);
                status = HIFReadWrite(target->device, address,
                                        (OSAL_UINT8*)&txCreditsAvailable, 1,
                                        HIF_RD_SYNC_BYTE_FIX, NULL);
                HTC_DEBUG_PRINTF(HTC_DEBUG_INF,
                                    ("Tx Credits after decrement: %d\n",
                                    txCreditsAvailable));

                /*
                 * ToDo:
                 *      This seems to not required. But since it unclear why it
                 *      was added in the 1st place, retaining it until the
                 *      the following is reasoned.
                 *
                 *      Empty out the credit counter by reading it (credits -1)
                 *      number of times. We have read once just before
                 */
                if (creditreceived > 1) {
                    status = HIFReadWrite(target->device, address,
                        (OSAL_UINT8*)&txCreditsAvailable, (creditreceived-1),
                        HIF_RD_SYNC_BYTE_FIX, NULL);
                    HTC_DEBUG_ASSERT(status == HTC_OK);
                }

                /* Aggregate and update the credits available */
                creditreceived += GET_TX_CREDITS_AVAILABLE(endPoint);
                SET_TX_CREDITS_AVAILABLE(endPoint, creditreceived);

                /* Set consumed to 0, as we dont need it after credit update */
                SET_TX_CREDITS_CONSUMED(endPoint, 0);
            }

            HTC_DEBUG_PRINTF(HTC_DEBUG_ALERT, ("Tx Credits Available: %d\n",
                        GET_TX_CREDITS_AVAILABLE(endPoint)));

            htcSendFrame(endPoint);
        }
    }
}

void htcEnableCreditCounterInterrupt(HTC_TARGET *target, 
                                HTC_ENDPOINT_ID endPointId)
{
    htc_status_t status;
    OSAL_UINT32 address;
    HTC_ENDPOINT *endPoint;
    HTC_REG_REQUEST_ELEMENT *element;


    HTC_DEBUG_PRINTF(HTC_DEBUG_INF, ("%s + \n", __func__));

    endPoint = &target->endPoint[endPointId];
    HTC_DEBUG_ASSERT(endPoint != NULL);

    OSAL_MUTEX_LOCK(&target->counterCS);

    endPoint->txCreditsIntrEnable = TRUE;
    address = getRegAddr(COUNTER_INT_STATUS_ENABLE_REG, 
            ENDPOINT_UNUSED);
    element = allocateRegRequestElement(target);
    HTC_DEBUG_ASSERT(element != NULL);

    FILL_REG_BUFFER(element, NULL, 1, COUNTER_INT_STATUS_ENABLE_REG,
            (target->endPoint[0].txCreditsIntrEnable << (4)) |
            (target->endPoint[1].txCreditsIntrEnable << (5)) |
            (target->endPoint[2].txCreditsIntrEnable << (6)) |
            (target->endPoint[3].txCreditsIntrEnable << (7)) | 0x0F);
    status = HIFReadWrite(target->device, address, 
            (OSAL_UINT8 *)&((GET_REG_BUFFER(element))->offset),
            1, HIF_WR_ASYNC_BYTE_FIX, element);

#ifndef HTC_SYNC
    HTC_DEBUG_ASSERT(status == HTC_OK);
#else
    HTC_DEBUG_ASSERT(status == HTC_OK || status == HTC_PENDING);
    if(status == HTC_OK) {
        element->completionCB(element, status);
    }
#endif

    OSAL_MUTEX_UNLOCK(&target->counterCS);
}

void htcDisableCreditCounterInterrupt(HTC_TARGET *target, 
                                 HTC_ENDPOINT_ID unused)
{
    htc_status_t status;
    OSAL_UINT32 address;
    HTC_ENDPOINT *endPoint;
    HTC_ENDPOINT_ID endPointId;
    OSAL_UINT8 counter_int_status;
    OSAL_UINT8 update_credit_int_status;
    HTC_REG_REQUEST_ELEMENT *element;

    HTC_DEBUG_PRINTF(HTC_DEBUG_INF, ("%s + \n", __func__));

    OSAL_MUTEX_LOCK(&target->counterCS);

    /* The Tx credit counter update bits are reflected in the upper nibble */
    counter_int_status = target->table.counter_int_status &
        target->table.counter_int_status_enable;
    update_credit_int_status = (counter_int_status & 0xF0) >> 4;
    for(endPointId = 0; endPointId < HTC_MAILBOX_NUM_MAX; endPointId++)
    {
        if(!(update_credit_int_status & (1 << endPointId)))
        {
            continue;
        }

        endPoint = &target->endPoint[endPointId];
        HTC_DEBUG_ASSERT(endPoint != NULL);

        HTC_DEBUG_PRINTF(HTC_DEBUG_INF, 
                ("endPoint(%d): %p\n", endPointId, endPoint));

        /* Disable the tx credit interrupt */
        endPoint->txCreditsIntrEnable = FALSE;
    }

    address = getRegAddr(COUNTER_INT_STATUS_DISABLE_REG, ENDPOINT_UNUSED);
    element = allocateRegRequestElement(target);
    HTC_DEBUG_ASSERT(element != NULL);
    FILL_REG_BUFFER(element, NULL, 1,
            COUNTER_INT_STATUS_DISABLE_REG,
            (target->endPoint[0].txCreditsIntrEnable << (4)) |
            (target->endPoint[1].txCreditsIntrEnable << (5)) |
            (target->endPoint[2].txCreditsIntrEnable << (6)) |
            (target->endPoint[3].txCreditsIntrEnable << (7)) | 0x0F);
    status = HIFReadWrite(target->device, address, 
            (OSAL_UINT8 *)&((GET_REG_BUFFER(element))->offset),
            1, HIF_WR_ASYNC_BYTE_FIX, element);


#ifndef HTC_SYNC
    HTC_DEBUG_ASSERT(status == HTC_OK);
#else
    HTC_DEBUG_ASSERT(status == HTC_OK || status == HTC_PENDING);
    if ( status == HTC_OK ) {
        element->completionCB(element, status);
    }
#endif

    OSAL_MUTEX_UNLOCK(&target->counterCS);
}

void htcServiceMailboxInterrupt(HTC_TARGET *target)
{

    htc_status_t status;
    OSAL_UINT32 address;
    HTC_ENDPOINT *endPoint;
    HTC_ENDPOINT_ID endPointId;
    OSAL_UINT8 mailbox_int_status;
    HTC_TARGET_TRANSPORT trans;
    OSAL_UINT32 spi_status = 0;

    HTC_DEBUG_PRINTF(HTC_DEBUG_INF, ("Mailbox Interrupt\n"));

    getTargetTransport(target, &trans);

    if (trans == HTC_TRANSPORT_SPI)
    {
        do {
            address = getRegAddr(SCRATCH_REG, ENDPOINT_UNUSED);
            status = HIFReadWrite(target->device, address, 
                (OSAL_UINT8*)&target->table.scratch[0], 1, HIF_RD_SYNC_BYTE_FIX, NULL);
            HTC_DEBUG_ASSERT(status == HTC_OK);
            
            for(endPointId = 0; endPointId < HTC_MAILBOX_NUM_MAX; endPointId++)
            {
                HTC_DEBUG_PRINTF(HTC_DEBUG_INF, ("SCRATCH DATA of EP %d,  0x%x\n", endPointId, target->table.scratch[0]));            
                if(!(target->table.scratch[HTC_SCRATCH_ENDPOINT_ID] & (0x1 << endPointId)))
                {
                    continue;
                }

                endPoint = &target->endPoint[endPointId];
                HTC_DEBUG_ASSERT(endPoint != NULL);

                HTC_DEBUG_PRINTF(HTC_DEBUG_INF, 
                        ("endPoint(%d): %p\n", endPointId, endPoint));

                /* Service the Rx interrupt */
                htcReceiveFrame(endPoint);
            }

            /* 
             * Read the register table again. Repeat the process until there are
             * no more valid packets queued up on receive. It is assumed that
             * the following request will be serialized along with the request
             * above and will be completed in the order in which it is received
             * by the bus driver.
             */
            address = getRegAddr(SPI_STATUS_READ_REG, ENDPOINT_UNUSED);
            status = HIFReadWrite(target->device, address, 
                    (OSAL_UINT8*)&target->table.spi_status, 4, HIF_RD_SYNC_BYTE_INC, NULL);
            HTC_DEBUG_ASSERT(status == HTC_OK);
            spi_status = target->table.spi_status;
        } while (spi_status & SPI_SLV_REG_INT_PACKET_AVAIL);

        /* Clear the scratch register for all endpoints */
        for (endPointId = 0; endPointId < HTC_MAILBOX_NUM_MAX; endPointId++)
            target->table.scratch[HTC_SCRATCH_ENDPOINT_ID] &= ~(1 << endPointId);
        
        address = getRegAddr(SCRATCH_REG, ENDPOINT_UNUSED);
        status = HIFReadWrite(target->device, address, 
            (OSAL_UINT8*)&target->table.scratch[HTC_SCRATCH_ENDPOINT_ID], 1, HIF_WR_SYNC_BYTE_FIX, NULL);
        HTC_DEBUG_ASSERT(status == HTC_OK);
    }
    else
    {
        /* The Rx interrupt bits are reflected in the lower nibble */
        mailbox_int_status = target->table.host_int_status & 
            HOST_INT_STATUS_ENDPOINT_DATA_MASK;

        HTC_DEBUG_PRINTF(HTC_DEBUG_INF, 
                ("Valid mailbox interrupt source(s) in INT_STATUS: 0x%x\n",
                 mailbox_int_status));

        /* Disable the receive interrupt for all four mailboxes */
        target->table.int_status_enable &= ~(HOST_INT_STATUS_ENDPOINT_DATA_MASK);

        do {
            for(endPointId = 0; endPointId < HTC_MAILBOX_NUM_MAX; endPointId++)
            {
                if(!(mailbox_int_status & (1 << endPointId)))
                {
                    continue;
                }

                endPoint = &target->endPoint[endPointId];
                HTC_DEBUG_ASSERT(endPoint != NULL);

                HTC_DEBUG_PRINTF(HTC_DEBUG_INF, 
                        ("endPoint(%d): %p\n", endPointId, endPoint));

                /* Service the Rx interrupt */
                htcReceiveFrame(endPoint);
            }

            /* 
             * Read the register table again. Repeat the process until there are
             * no more valid packets queued up on receive. It is assumed that
             * the following request will be serialized along with the request
             * above and will be completed in the order in which it is received
             * by the bus driver.
             */
            address = getRegAddr(INT_STATUS_REG, ENDPOINT_UNUSED);
            status = HIFReadWrite(target->device, address, 
                    &target->table.host_int_status, 
                    24, HIF_RD_SYNC_BYTE_INC, NULL); /*TODO: 24 or 28? */
            HTC_DEBUG_ASSERT(status == HTC_OK);
            mailbox_int_status = target->table.host_int_status & 
                HOST_INT_STATUS_ENDPOINT_DATA_MASK;
        } while (mailbox_int_status);
    }
    target->table.int_status_enable |= HOST_INT_STATUS_ENDPOINT_DATA_MASK;
}
