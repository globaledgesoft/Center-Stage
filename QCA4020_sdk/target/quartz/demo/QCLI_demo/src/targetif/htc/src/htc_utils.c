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

 
/*****************************************************************************
 * 
 * This file contains the utility routines used across the entire HTC module.
 *
 ****************************************************************************/

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
extern HTC_TARGET *htc_target_dev[HIF_MAX_DEVICES];


extern HTC_GLOBAL_EVENT_TABLE HTCEventTable;

#ifdef DEBUG
extern OSAL_UINT32 debughtc;
#endif


/* ------ Static Variables ------ */

/* ------ Functions ------ */


htc_status_t getTargetTransport (HTC_TARGET *target, HTC_TARGET_TRANSPORT *trans)
{
    struct hif_device *hif_dev = NULL;
    trans_device_t *tdev = NULL;

    if (target == NULL) return HTC_EINVAL;
    if (trans == NULL) return HTC_EINVAL;
    
    hif_dev = (HIF_DEVICE*) target->device;
    if (hif_dev == NULL) return HTC_ERROR;
   
    tdev = (trans_device_t *)hif_dev->tdev;
    if (tdev == NULL) return HTC_ERROR;

    *trans = tdev->transport;

    return HTC_OK;
}

void
dispatchEvent(HTC_TARGET     *target, 
              HTC_ENDPOINT_ID endPointId,
              HTC_EVENT_ID    eventId, 
              HTC_EVENT_INFO *eventInfo)
{
    EVENT_TABLE_ELEMENT *eventElement;

    if (eventId == HTC_TARGET_AVAILABLE) {
        eventElement = &HTCEventTable.element[0];
    } else if (eventId == HTC_TARGET_UNAVAILABLE) {
        eventElement = &HTCEventTable.element[1];
    } else {
        eventElement = 
            &target->endPoint[endPointId].eventTable.element[eventId]; 
    }
    HTC_DEBUG_ASSERT(eventElement != NULL);

    HTC_DEBUG_PRINTF(HTC_DEBUG_INF, 
                    ("dispatchEvent(endpoint: %d, eventId: 0x%d, handler: 0x%p)\n", endPointId, eventElement->id, eventElement->handler));
    if (eventElement->handler) {
        eventElement->handler(target, endPointId, eventId, eventInfo,
                              eventElement->param);
    }
}


htc_status_t 
addToEventTable(HTC_TARGET       *target,
                HTC_ENDPOINT_ID   endPointId,
                HTC_EVENT_ID      eventId,
                HTC_EVENT_HANDLER handler, 
                void             *param)
{
    EVENT_TABLE_ELEMENT *new;

    if (eventId == HTC_TARGET_AVAILABLE) {
        new = &HTCEventTable.element[0];
    } else if (eventId == HTC_TARGET_UNAVAILABLE) {
        new = &HTCEventTable.element[1];
    } else {
        new = &target->endPoint[endPointId].eventTable.element[eventId]; 
    }

	/* Return error if there is already handler registered */
	if (new->handler != NULL) return HTC_ERROR;

    /* Store the event id, the corresponding handler and the param passed */
    new->id = eventId;
    new->handler = handler;
    new->param = param;

    HTC_DEBUG_PRINTF(HTC_DEBUG_INF, 
                    ("addToEventTable(endpoint: %d, eventId: 0x%d, handler: 0x%p)\n", endPointId, new->id, new->handler));

    return HTC_OK;
}


htc_status_t 
removeFromEventTable(HTC_TARGET *target,
                     HTC_ENDPOINT_ID endPointId,
                     HTC_EVENT_ID  eventId)
{
    EVENT_TABLE_ELEMENT *remove;

    if (eventId == HTC_TARGET_AVAILABLE) {
        remove = &HTCEventTable.element[0];
    } else if (eventId == HTC_TARGET_UNAVAILABLE) {
        remove = &HTCEventTable.element[1];
    } else {
        remove = &target->endPoint[endPointId].eventTable.element[eventId]; 
    }

    /* Remove the event handler */
    OSAL_MEMZERO(remove, sizeof(EVENT_TABLE_ELEMENT));

    return HTC_OK;
}

/* DEBUG Start */
void
printEndpointQueueElement(HTC_DATA_REQUEST_ELEMENT *element)
{
    HTC_DEBUG_PRINTF(HTC_DEBUG_TRC, ("element: 0x%p, endpointBuffer: 0x%p, free: %d, completionCB: 0x%p, cookie: 0x%p, buffer: 0x%p, bufferLength: %d, actualLength: %d\n", element, &element->buffer.u.endpointBuffer, element->buffer.free, element->completionCB, element->buffer.u.endpointBuffer.cookie, element->buffer.u.endpointBuffer.buffer, element->buffer.u.endpointBuffer.bufferLength, element->buffer.u.endpointBuffer.actualLength));

}

void
printEndpointQueue(HTC_DATA_REQUEST_QUEUE *queue)
{
    OSAL_UINT32 count;
    HTC_DATA_REQUEST_ELEMENT *element;

    HTC_DEBUG_PRINTF(HTC_DEBUG_TRC, ("Queue (size: %d, head: %d)\n", queue->size, queue->head));
    for (count = 0; count < HTC_DATA_REQUEST_RING_BUFFER_SIZE; count ++) {
        element = &queue->element[count];
        printEndpointQueueElement(element);
    }
}
/* DEBUG End */

htc_status_t
addToEndpointQueue(HTC_TARGET *target, HTC_DATA_REQUEST_QUEUE *queue,
               OSAL_UINT8        *buffer,
               OSAL_UINT32        bufferLength,
               OSAL_UINT32        actualLength,
               void           *cookie)
{
    htc_status_t status;
    HTC_DATA_REQUEST_ELEMENT *element;
    HTC_ENDPOINT_BUFFER *endpointBuffer;
    HTC_EVENT_INFO eventInfo;
    HTC_ENDPOINT_ID endPointId;
    HTC_ENDPOINT *endPoint;

    HTC_DEBUG_ASSERT(queue != NULL);
    HTC_DEBUG_ASSERT(bufferLength);

    HTC_DEBUG_PRINTF(HTC_DEBUG_SYNC, 
            ("Critical Section (queue): LOCK at line %d in file %s\n", __LINE__, __FILE__));
    OSAL_MUTEX_LOCK(&target->instanceCS);
    element = GET_QUEUE_TAIL(queue);
    if (!(IS_DATA_QUEUE_FULL(queue)) && IS_ELEMENT_FREE(element)) {
        element->buffer.free = FALSE;
        FILL_ENDPOINT_BUFFER(element, buffer, bufferLength, actualLength, cookie);
        queue->size += 1;

        HTC_DEBUG_PRINTF(HTC_DEBUG_INF, 
                ("addToEndpointQueue (index: %d, size: %d, bufferElement: 0x%p, bufferElement->buffer: 0x%p, bufferElement->cookie: 0x%p)\n", (queue->head + queue->size - 1) % HTC_DATA_REQUEST_RING_BUFFER_SIZE, queue->size, element, (GET_ENDPOINT_BUFFER(element))->buffer, (GET_ENDPOINT_BUFFER(element))->cookie));
        status = HTC_OK;
    } else {
        /* Free the stale request and return it to the owner */
        HTC_DEBUG_PRINTF(HTC_DEBUG_TRC, ("LR: 0x%p\n", element));
        endpointBuffer = GET_ENDPOINT_BUFFER(element);
        HTC_DEBUG_ASSERT(endpointBuffer != NULL);
        endPoint = endpointBuffer->endPoint;
        HTC_DEBUG_ASSERT(endPoint != NULL);
        target = endPoint->target;
        HTC_DEBUG_ASSERT(target != NULL);
        endPointId = GET_ENDPOINT_ID(endPoint);
        endpointBuffer->actualLength = endpointBuffer->bufferLength;
        endpointBuffer->buffer += HTC_HEADER_LEN;
        FRAME_EVENT(eventInfo, endpointBuffer->buffer, endpointBuffer->bufferLength,
                endpointBuffer->actualLength, HTC_OK, endpointBuffer->cookie);
        RECYCLE_DATA_REQUEST_ELEMENT(element);
        /* 
         * There could be a potential deadlock if another packet is 
         * queued from the context of dispatchEvent so we should give up 
         * the lock.
         */
        //OSAL_MUTEX_UNLOCK(&target->instanceCS);
        dispatchEvent(target, endPointId, HTC_BUFFER_SENT, &eventInfo);
        //OSAL_MUTEX_LOCK(&target->instanceCS);
        element->buffer.free = FALSE;
        FILL_ENDPOINT_BUFFER(element, buffer, bufferLength, actualLength, 
                cookie);
        queue->size += 1;
        status = HTC_OK;
    }
    HTC_DEBUG_PRINTF(HTC_DEBUG_SYNC, 
            ("Critical Section (queue): UNLOCK at line %d in file %s\n", __LINE__, __FILE__));
    OSAL_MUTEX_UNLOCK(&target->instanceCS);

    return status;
}

HTC_DATA_REQUEST_ELEMENT *
removeFromEndpointQueue(HTC_TARGET *target, HTC_DATA_REQUEST_QUEUE *queue) {
    HTC_DATA_REQUEST_ELEMENT *element;
    HTC_DEBUG_ASSERT(queue != NULL);

    HTC_DEBUG_PRINTF(HTC_DEBUG_SYNC, 
                    ("Critical Section (queue): LOCK at line %d in file %s\n", __LINE__, __FILE__));
    OSAL_MUTEX_LOCK(&target->instanceCS);
    if (!(IS_DATA_QUEUE_EMPTY(queue))) {
        element = GET_QUEUE_HEAD(queue);
        queue->head = ((queue->head + 1) % HTC_DATA_REQUEST_RING_BUFFER_SIZE);
        queue->size -= 1;

        HTC_DEBUG_PRINTF(HTC_DEBUG_INF, 
                        ("removeFromEndpointQueue (index: %d, size: %d, bufferElement: 0x%p, bufferElement->buffer: 0x%p, bufferElement->cookie: 0x%p)\n", queue->head, queue->size, element, (GET_ENDPOINT_BUFFER(element))->buffer, (GET_ENDPOINT_BUFFER(element))->cookie));
    } else {
        element = NULL;
    }
    HTC_DEBUG_PRINTF(HTC_DEBUG_SYNC, 
                    ("Critical Section (queue): UNLOCK at line %d in file %s\n", __LINE__, __FILE__));
    OSAL_MUTEX_UNLOCK(&target->instanceCS);

    return element;
}

void
flushEndpointQueue(HTC_ENDPOINT *endPoint,
               HTC_DATA_REQUEST_QUEUE *queue, 
               HTC_EVENT_ID eventId)
{
    HTC_DATA_REQUEST_ELEMENT *curr;
    HTC_EVENT_INFO eventInfo;
    HTC_ENDPOINT_EVENT_TABLE *eventTable;
    HTC_ENDPOINT_ID endPointId;
    EVENT_TABLE_ELEMENT *eventElement;
    HTC_ENDPOINT_BUFFER *endpointBuffer;

    eventTable = &endPoint->eventTable;
    endPointId = GET_ENDPOINT_ID(endPoint);

    /* 
     * Release the buffer to WMI using the registered event handler. If WMI 
     * has not registered any callbacks for a particular endpoint then it 
     * means that its queues will not have any buffers so we skip that 
     * endpoint.
     */
    if ((eventElement = &eventTable->element[eventId]) != NULL) {
        while ((curr = removeFromEndpointQueue(endPoint->target, queue)) != NULL) {
            /* Frame the event and dispatch it */
            endpointBuffer = GET_ENDPOINT_BUFFER(curr);
            FRAME_EVENT(eventInfo, endpointBuffer->buffer, 
                        endpointBuffer->bufferLength, endpointBuffer->actualLength, 
                        HTC_ECANCELED, endpointBuffer->cookie);
            if (eventElement->handler) {
                eventElement->handler(endPoint->target, endPointId, eventId, 
                                      &eventInfo, eventElement->param);
            }
            RECYCLE_DATA_REQUEST_ELEMENT(curr);
        }
    }

    /* Initialize the head and tail pointer */
    queue->head = 0;
    queue->size = 0;
}

HTC_REG_REQUEST_ELEMENT *
allocateRegRequestElement(HTC_TARGET *target) {
    OSAL_UINT32 count;
    HTC_REG_REQUEST_ELEMENT *element;

    OSAL_MUTEX_LOCK(&target->instanceCS);
    element = NULL;
    for (count = 0; count < HTC_REG_REQUEST_LIST_SIZE; count ++) {
        element = &target->regList.element[count];
        if (IS_ELEMENT_FREE(element)) {
            element->buffer.free = FALSE;
            break;
        }
    }
    if (count == HTC_REG_REQUEST_LIST_SIZE) {
    	element = NULL;
    }
    OSAL_MUTEX_UNLOCK(&target->instanceCS);

    return element;
}

void
freeRegRequestElement(HTC_REG_REQUEST_ELEMENT *element) {
    HTC_TARGET *target;
    HTC_REG_BUFFER *regBuffer;
	
	regBuffer = GET_REG_BUFFER(element);
	HTC_DEBUG_ASSERT(regBuffer != NULL);
	target = regBuffer->target;

    OSAL_MUTEX_LOCK(&target->instanceCS);
    FILL_REG_BUFFER(element, NULL, 0, 0, 0);
    element->buffer.free = TRUE;
    OSAL_MUTEX_UNLOCK(&target->instanceCS);
}

void
htcReportFailure(htc_status_t status)
{
#if 0 /* TODO: fix this as we need to claim device in hif */
    HTC_TARGET *target = htc_target_dev[0];

    /*
     * When we have true multiple device support then we would expect the 
     * device handler to be passed in this function. Until then, we are
     * directly going to grab hold of the target device from the global
     * array.
     */
    htcTargetRemovedHandler(target, status);
#endif
}


htc_status_t addTargetInstance (HTC_TARGET *target)
{
    OSAL_UINT32 i=0;
    
	for (i=0; i < HIF_MAX_DEVICES; i++) {
		if (htc_target_dev[i] == NULL) {
			break;
		}
	}
	if (i == HIF_MAX_DEVICES) {
		return HTC_NO_RESOURCE;
	}

	/* Allocate the devices */
	htc_target_dev[i] = target;
    return HTC_OK;
}

htc_status_t delTargetInstance (HTC_TARGET *target)
{
    OSAL_UINT32 i=0;
    
    if (target == NULL) return HTC_EINVAL;
    
	for (i=0; i < HIF_MAX_DEVICES; i++) {
		if (htc_target_dev[i] != NULL) {
			if (htc_target_dev[i] == target) {
				htc_target_dev[i] = NULL;
                return HTC_OK;
			}
		}
	}
    return HTC_ENOENT;
}

OSAL_UINT32 
getRegAddr(TARGET_REGISTERS base, 
           HTC_ENDPOINT_ID endPointId) 
{
    OSAL_UINT32 address;

    switch(base) {
    case TX_CREDIT_COUNTER_RESET_REG:
    case DEBUG_COUNTER_REG:
        address = COUNT_DEC_ADDRESS + endPointId * 4;
        break;

    case TX_CREDIT_COUNTER_DECREMENT_REG:
        address = COUNT_DEC_ADDRESS + (HTC_MAILBOX_NUM_MAX + endPointId) * 4;
        break;

    case TX_CREDIT_COUNTER_REG:
        address = COUNT_ADDRESS + (HTC_MAILBOX_NUM_MAX + endPointId) * 4;
        break;

    case INT_STATUS_ENABLE_REG:
        address = INT_STATUS_ENABLE_ADDRESS;
        break;

    case COUNTER_INT_STATUS_ENABLE_REG:
    case COUNTER_INT_STATUS_DISABLE_REG:
        address = COUNTER_INT_STATUS_ENABLE_ADDRESS;
        break;

    case INT_STATUS_REG:
        address = HOST_INT_STATUS_ADDRESS;
        break;

    case CPU_INT_STATUS_REG:
        address = CPU_INT_STATUS_ADDRESS;
        break;

    case ERROR_INT_STATUS_REG:
        address = ERROR_INT_STATUS_ADDRESS;
        break;

    case INT_WLAN_REG:
        address = INT_WLAN_ADDRESS;
        break;

    case WINDOW_DATA_REG:
        address = WINDOW_DATA_ADDRESS;
        break;

    case WINDOW_WRITE_ADDR_REG:
        address = WINDOW_WRITE_ADDR_ADDRESS;
        break;

    case WINDOW_READ_ADDR_REG:
        address = WINDOW_READ_ADDR_ADDRESS;
        break;
    
    case SPI_STATUS_READ_REG:
    case SPI_STATUS_WRITE_REG:
        address = SPI_STATUS_ADDRESS;
        break;
    
    case SCRATCH_REG:
        address = SCRATCH_ADDRESS;
        break;  
    
    case RX_LOOKAHEAD_VALID_REG:
        address = RX_LOOKAHEAD_VALID_ADDRESS;
        break;  
    
    case RX_LOOKAHEAD0_REG:
        address = RX_LOOKAHEAD0_ADDRESS;
        break;     
    
    case RX_LOOKAHEAD1_REG:
        address = RX_LOOKAHEAD1_ADDRESS;
        break; 
    
    case RX_LOOKAHEAD2_REG:
        address = RX_LOOKAHEAD2_ADDRESS;
        break; 
    
    case RX_LOOKAHEAD3_REG:
        address = RX_LOOKAHEAD3_ADDRESS;
        break;     
    
    default:
        HTC_DEBUG_PRINTF(HTC_DEBUG_ERR, ("Invalid register: %d\n", base));
        HTC_DEBUG_ASSERT(0);
        address = 0;
        break;
    }

    return address;
}

void
dumpBytes(OSAL_UINT8 *buffer, OSAL_UINT16 length)
{
    OSAL_CHAR stream[60];
    OSAL_UINT32 i;
    OSAL_UINT16 offset, count;

    HTC_DEBUG_PRINTF(HTC_DEBUG_DUMP, ("Dumping %d Bytes : ------>\n", length));

    count = 0;
    offset = 0;
    for(i = 0; i < length; i++) {
        sprintf(stream + offset, "%2x ", buffer[i]);
	count ++;
	offset += 3;

	if(count == 16) {
	    count = 0;
	    offset = 0;
	    HTC_DEBUG_PRINTF(HTC_DEBUG_DUMP, ("[H]: %s\n", stream));
	    OSAL_MEMZERO(stream, 60);
	}
    }

    if(offset != 0) {
	HTC_DEBUG_PRINTF(HTC_DEBUG_DUMP, ("[H]: %s\n", stream));
    }
}

void
dumpRegisters(HTC_TARGET *target)
{
    HTC_REGISTER_TABLE *reg;

    reg = &target->table;
    HTC_DEBUG_PRINTF(HTC_DEBUG_DUMP, ("\n<------- Register Table -------->\nInt Status:                0x%x\nCPU Int Status:            0x%x\nError Int Status:          0x%x\nCounter Int Status:        0x%x\nEndpoint Frame:                0x%x\nRx Lookahead Valid:        0x%x\nRx Lookahead 0:            0x%x\nRx Lookahead 1:            0x%x\nRx Lookahead 2:            0x%x\nRx Lookahead 3:            0x%x\n\n", reg->host_int_status, reg->cpu_int_status, reg->error_int_status, reg->counter_int_status, reg->endpoint_frame, reg->rx_lookahead_valid, reg->rx_lookahead[ENDPOINT1], reg->rx_lookahead[ENDPOINT2], reg->rx_lookahead[ENDPOINT3], reg->rx_lookahead[ENDPOINT4]));
    HTC_DEBUG_PRINTF(HTC_DEBUG_DUMP, ("\nInt Status Enable:         0x%x\nCounter Int Status Enable: 0x%x\n<------------------------------->\n", reg->int_status_enable, reg->counter_int_status_enable));
	if (reg) {}
}
