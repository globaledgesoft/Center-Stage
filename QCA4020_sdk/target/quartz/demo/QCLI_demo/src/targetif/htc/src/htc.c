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

/*
* This file contains the HTC APIs that are exposed to higher layers.
*/

#include "com_dtypes.h"
#include "qapi_types.h"
#include "qapi_status.h"

/* Header files */
#include "osal_types.h"
#include "htc_defs.h"
#include "htc_host_reg.h"
#include "htc.h"
#include "transport.h"
#include "htc_internal.h"
#include "hif.h"


/* ------ Global Variable Declarations ------- */
HTC_TARGET *htc_target_dev[HIF_MAX_DEVICES];
HTC_GLOBAL_EVENT_TABLE HTCEventTable; /* Global DS, used only for Target Insert/Remove usecase*/
static OSAL_BOOL is_initialised = OSAL_FALSE;

static OSAL_BOOL isTargetFound(HTC_TARGET *target)
{
    OSAL_UINT32 i=0;

    for (i = 0; i < HIF_MAX_DEVICES; i++)
    {
        if (htc_target_dev[i] == target)
            return OSAL_TRUE;
    }
    return OSAL_FALSE;
}

/* Initializes the HTC module */
htc_status_t htc_host_init(void)
{
    OSDRV_CALLBACKS htcCallbacks;
	htc_status_t status;

    HTC_DEBUG_PRINTF(HTC_DEBUG_TRC, ("htc_host_init: Enter\n"));
    HTC_DEBUG_PRINTF(HTC_DEBUG_ERR, ("htc_host_init: Enter\n"));

    OSAL_MEMZERO(&HTCEventTable, sizeof(HTC_GLOBAL_EVENT_TABLE));
    OSAL_MEMZERO(&htcCallbacks, sizeof(OSDRV_CALLBACKS));

    htcCallbacks.deviceInsertedHandler = htcTargetInsertedHandler;
    htcCallbacks.deviceRemovedHandler = htcTargetRemovedHandler;

    status = HIFInit(&htcCallbacks);
    if (status != HTC_OK)
	{
		OSAL_MEMZERO(&htcCallbacks, sizeof(OSDRV_CALLBACKS));
        return status;	
	}
    is_initialised = OSAL_TRUE;
    HTC_DEBUG_PRINTF(HTC_DEBUG_TRC, ("htc_host_init: Exit\n"));

    return HTC_OK;
}

/* Enables Dragon interrupts */
htc_status_t htc_host_start(HTC_TARGET *target)
{
    htc_status_t status;
    OSAL_UINT32 address;

    HTC_DEBUG_PRINTF(HTC_DEBUG_TRC, ("htc_host_start Enter\n"));

    if (!is_initialised || !isTargetFound(target))
        return HTC_ERROR;

    /* Unmask the host controller interrupts */
    HIFUnMaskInterrupt(target->device);

    /* Enable all the interrupts except for the dragon interrupt */
    target->table.int_status_enable = INT_STATUS_ENABLE_ERROR_SET(0x01) |
        INT_STATUS_ENABLE_CPU_SET(0x01) |
        INT_STATUS_ENABLE_COUNTER_SET(0x01) |
        INT_STATUS_ENABLE_ENDPOINT_DATA_SET(0x0F);

    /* Set up the CPU Interrupt Status Register */
    target->table.cpu_int_status_enable = CPU_INT_STATUS_ENABLE_BIT_SET(0xFF);

    /* Set up the Error Interrupt Status Register */
    target->table.error_status_enable =
        ERROR_STATUS_ENABLE_RX_UNDERFLOW_SET(0x01) |
        ERROR_STATUS_ENABLE_TX_OVERFLOW_SET(0x01);


	/* Enable credit interrupt only if target is not ready */
	/* This can happen if htc is temporarily started/stopped instead of doing init/shutdown */
    if (!target->ready)
    {
	    /* Set up the Counter Interrupt Status Register */
	    target->table.counter_int_status_enable =
	        COUNTER_INT_STATUS_ENABLE_BIT_SET(0xFF);

    }

    /* Write to the register */
    address = getRegAddr(INT_STATUS_ENABLE_REG, ENDPOINT_UNUSED);
    status = HIFReadWrite(target->device, address,
            &target->table.int_status_enable, 4, HIF_WR_SYNC_BYTE_INC, NULL);
    if (status != HTC_OK)
    {
        /* Can't write it for some reason */
        HTC_DEBUG_PRINTF(HTC_DEBUG_ERR,
                ("Failed to enable INT_STATUS_ENABLE | CPU_INT_STATUS_ENABLE | ERROR_STATUS_ENABLE | COUNTER_INT_STATUS_ENABLE, err: %d\n", status));
        htc_host_stop(target);
        return status;
    }

    /* Wait on a timed semaphore that will get signalled once the block
       size negotiation with the target has completed. Furthermore, we have
       to do it only once during the lifetime of the target detection */
    if (!target->ready)
    {
        HTC_DEBUG_PRINTF(HTC_DEBUG_INF,
                ("Waiting for the block size negotiation to finish\n"));

        uint32 signalled = qurt_signal_wait(&(target->htcEvent),
                HTC_BLK_SIZE_NEG_COMPLETE_EVT,
                QURT_SIGNAL_ATTR_WAIT_ANY);

        qurt_signal_clear(&(target->htcEvent), signalled);

        if (signalled & HTC_BLK_SIZE_NEG_COMPLETE_EVT)
        {
            if (target->ready)
            {
                status = HTC_OK;
            } else
            {
                status = HTC_ERROR;
                HTC_DEBUG_PRINTF(HTC_DEBUG_ERR,
                        ("Failed to negotiate the block sizes\n"));
                htc_host_stop(target);
            }
        }
    }

    HTC_DEBUG_PRINTF(HTC_DEBUG_TRC, ("htc_host_start Exit\n"));

    return status;
}

/*
 * Provides an interface for the higher layer module to register for
 * different events supported by the HTC module
 */
htc_status_t htc_host_event_register(HTC_TARGET *target, HTC_ENDPOINT_ID endPointId,
            HTC_EVENT_ID eventId, HTC_EVENT_HANDLER eventHandler,
            void *param)
{
    /*
     * Add the event handler against the specified event and store it in
     * the event table
     */
    htc_status_t status;
    HTC_ENDPOINT *endPoint;
    HTC_EVENT_INFO eventInfo;
    HTC_DATA_REQUEST_QUEUE *sendQueue, *recvQueue;
    OSAL_UINT32 i=0;

	if (!is_initialised)
		return HTC_ERROR;
    HTC_DEBUG_PRINTF(HTC_DEBUG_TRC,
            ("htc_host_event_register: Enter (eventId: 0x%x, endPointId: %d)\n",
             eventId, endPointId));

    if (eventHandler)
    {
        if ((status = addToEventTable(target, endPointId, eventId,
                        eventHandler, param)) != HTC_OK)
        {
            HTC_DEBUG_PRINTF(HTC_DEBUG_ERR,
                    ("Could not add the event 0x%x to the event table\n", eventId));
            return status;
        }
    }

    switch(eventId)
    {
        case HTC_TARGET_AVAILABLE:

            if (eventHandler != NULL)
            {
                /*
                 * Dispatch a Target Available event for all the targets
                 * present. Iterate through the global list of targets.
                 */
                for (i=0; i<HIF_MAX_DEVICES; i++)
                {
                    if (htc_target_dev[i] != NULL) {
                        target = htc_target_dev[i];
                        FRAME_EVENT(eventInfo, (OSAL_UINT8 *)target->device,
                                sizeof(HIF_DEVICE *), sizeof(HIF_DEVICE *),
                                HTC_OK, NULL);
                        dispatchEvent(target, ENDPOINT_UNUSED, eventId, &eventInfo);
                    }
                }
            } else
            {
                /* Initiate a shut down procedure */
            }

            break;

        case HTC_TARGET_UNAVAILABLE:
            break;

        case HTC_BUFFER_RECEIVED:
            if (eventHandler == NULL && isTargetFound(target))
            {
                /* Flush the queue before unregistering the event handler */
                endPoint = &target->endPoint[endPointId];
                recvQueue = &endPoint->recvQueue;
                flushEndpointQueue(endPoint, recvQueue, HTC_BUFFER_RECEIVED);
            }
            break;

        case HTC_SKB_RECEIVED:
            HTC_DEBUG_PRINTF(HTC_DEBUG_ERR, ("skb not handled currently\n"));
            break;

        case HTC_BUFFER_SENT:
            if (eventHandler == NULL && isTargetFound(target))
            {
                /* Flush the queue before unregistering the event handler */
                endPoint = &target->endPoint[endPointId];
                sendQueue = &endPoint->sendQueue;
                flushEndpointQueue(endPoint, sendQueue, HTC_BUFFER_SENT);
            }
            break;

        case HTC_SKB_SENT:
            HTC_DEBUG_PRINTF(HTC_DEBUG_ERR, ("skb not handled currently\n"));
            break;

        case HTC_DATA_AVAILABLE:
            /*
             * Dispatch a data available event with the length. We are
             * not handling this specific case currently because registering
             * for HTC_DATA_AVAILABLE event is a part of the discipline
             * that is imposed before one starts using HTC
             */
            break;

        default:
            HTC_DEBUG_PRINTF(HTC_DEBUG_ERR,
                    ("Unknown Event ID: 0x%x\n", eventId));
            return HTC_EINVAL;
    }

    /* Check if its a call for registering the event or unregistering it */
    if (eventHandler == NULL && isTargetFound(target)) 
    {
        if ((status = removeFromEventTable(target, endPointId,
                        eventId)) != HTC_OK)
        {
            HTC_DEBUG_PRINTF(HTC_DEBUG_ERR,
                    ("Could not remove the event 0x%x from the event table\n", eventId));
            return status;
        }
    }

    HTC_DEBUG_PRINTF(HTC_DEBUG_TRC, ("htc_host_event_register: Exit\n"));
    return HTC_OK;
}


/*
 * Provides an interface for the higher layer module to de-register for
 * different events supported by the HTC module
 *
 * TODO: htc_host_event_register can be used to deregister as well so this
 *       can be obsoleted.
 */
htc_status_t htc_host_event_deregister(HTC_TARGET *target, HTC_ENDPOINT_ID endPointId,
            HTC_EVENT_ID eventId)
{
    htc_status_t status;
    HTC_ENDPOINT *endPoint;
    HTC_DATA_REQUEST_QUEUE *sendQueue, *recvQueue;

    if (!is_initialised || !isTargetFound(target))
        return HTC_ERROR;

    HTC_DEBUG_PRINTF(HTC_DEBUG_TRC,
            ("htc_host_event_deregister: Enter (eventId: 0x%x, endPointId: %d)\n",
             eventId, endPointId));

    switch(eventId)
    {
        case HTC_TARGET_AVAILABLE:
            break;

        case HTC_TARGET_UNAVAILABLE:
            break;

        case HTC_BUFFER_RECEIVED:
            /* Flush the queue before unregistering the event handler */
            endPoint = &target->endPoint[endPointId];
            recvQueue = &endPoint->recvQueue;
            flushEndpointQueue(endPoint, recvQueue, HTC_BUFFER_RECEIVED);
            break;

        case HTC_SKB_RECEIVED:
            HTC_DEBUG_PRINTF(HTC_DEBUG_ERR, ("skb not handled currently\n"));
            break;

        case HTC_BUFFER_SENT:
            /* Flush the queue before unregistering the event handler */
            endPoint = &target->endPoint[endPointId];
            sendQueue = &endPoint->sendQueue;
            flushEndpointQueue(endPoint, sendQueue, HTC_BUFFER_SENT);
            break;

        case HTC_SKB_SENT:
            HTC_DEBUG_PRINTF(HTC_DEBUG_ERR, ("skb not handled currently\n"));
            break;

        case HTC_DATA_AVAILABLE:
            break;

        default:
            HTC_DEBUG_PRINTF(HTC_DEBUG_ERR,
                    ("Unknown Event ID: 0x%x\n", eventId));
            return HTC_EINVAL;
    }

    if ((status = removeFromEventTable(target, endPointId,
                    eventId)) != HTC_OK)
    {
        HTC_DEBUG_PRINTF(HTC_DEBUG_ERR,
                ("Could not remove the event 0x%x from the event table\n", eventId));
        return status;
    }

    HTC_DEBUG_PRINTF(HTC_DEBUG_TRC, ("htc_host_event_deregister: Exit\n"));
    return HTC_OK;
}

htc_status_t htc_host_stop(HTC_TARGET *target)
{
    htc_status_t status;
    OSAL_UINT32 address;

    HTC_DEBUG_PRINTF(HTC_DEBUG_TRC, ("htc_host_stop: Enter"));

    if (!is_initialised || !isTargetFound(target))
        return HTC_ERROR;

    /* Disable all the dragon interrupts */
    target->table.int_status_enable = 0;
    target->table.cpu_int_status_enable = 0;
    target->table.error_status_enable = 0;
    target->table.counter_int_status_enable = 0;
    address = getRegAddr(INT_STATUS_ENABLE_REG, ENDPOINT_UNUSED);
    status = HIFReadWrite(target->device, address,
            &target->table.int_status_enable, 4, HIF_WR_SYNC_BYTE_INC, NULL);
    HTC_DEBUG_PRINTF(HTC_DEBUG_TRC, ("%s status = %d line %d\n",__func__,  status, __LINE__));
    HTC_DEBUG_ASSERT(status == HTC_OK);

    /* Disable the host controller interrupts */
    HIFMaskInterrupt(target->device);
	
    HTC_DEBUG_PRINTF(HTC_DEBUG_TRC, ("htc_host_stop: Exit"));

    return status;
}


htc_status_t htc_host_restart_target(HTC_TARGET *target)
{
    OSAL_UINT32 address;
    HTC_REG_REQUEST_ELEMENT *regElement;
    htc_status_t status;

    if (!is_initialised || !isTargetFound(target))
        return HTC_ERROR;

    /* Send the INT_WLAN interrupt to the target */
    target->table.int_wlan = (1<< HTC_INT_TARGET_RESET_HOST_REQ);
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
    return status;
}

htc_status_t htc_host_shutdown(HTC_TARGET *target)
{
    OSAL_UINT32 count;
    HTC_TARGET *handle;
    HTC_REG_REQUEST_LIST *regList;
    HTC_REG_REQUEST_ELEMENT *element;
    static OSAL_BOOL funcDrvUnregistered = FALSE;
    htc_status_t status;
    OSAL_UINT32 address;
    HTC_ENDPOINT *endPoint;
    HTC_DATA_REQUEST_QUEUE *sendQueue;
    HTC_DATA_REQUEST_QUEUE *recvQueue;
	
    HTC_DEBUG_PRINTF(HTC_DEBUG_TRC, ("htc_host_shutdown: Enter\n"));

    if (!is_initialised || !isTargetFound(target))
        return HTC_ERROR;

    /* Flush all the queues and return the buffers to their owner */
    for (count = ENDPOINT1; count <= ENDPOINT4; count ++)
    {
        endPoint = &target->endPoint[count];

        /* Decrement the number of credits consumed */
        /* Avoid any IO if Target is shutdown, to prevent undefined behaviour.*/
        if (target->target_state != HTC_TARGET_SHUTDOWN && endPoint->txCreditsConsumed)
        {
            address = getRegAddr(TX_CREDIT_COUNTER_DECREMENT_REG, count);
            status = HIFReadWrite(target->device, address,
                    (OSAL_UINT8*)endPoint->txCreditsAvailable, endPoint->txCreditsConsumed,
                    HIF_WR_SYNC_BYTE_FIX, NULL);
            HTC_DEBUG_PRINTF(HTC_DEBUG_TRC, ("%s status = %d line %d\n",__func__,  status, __LINE__));
            HTC_DEBUG_ASSERT(status == HTC_OK);
        }

        SET_TX_CREDITS_AVAILABLE(endPoint, 0);
        SET_TX_CREDITS_CONSUMED(endPoint, 0);

        endPoint->rxLengthPending = 0;

        /* Flush the Pending Receive Queue */
        HTC_DEBUG_PRINTF(HTC_DEBUG_INF,
                ("Flushing the recv queue & returning the buffers\n"));

        recvQueue = &endPoint->recvQueue;
        flushEndpointQueue(endPoint, recvQueue, HTC_BUFFER_RECEIVED);

        /* Flush the Pending Send Queue */
        HTC_DEBUG_PRINTF(HTC_DEBUG_INF,
                ("Flushing the send queue & returning the buffers\n"));

        sendQueue = &endPoint->sendQueue;
        flushEndpointQueue(endPoint, sendQueue, HTC_BUFFER_SENT);
    }

    /*
     * Instead of taking an argument the API should bring down all the
     * devices. It should be a counterpart of htc_host_init. The API should be
     * enhanced to be able to interpret when the upper layer driver wants
     * the function driver to be unregistered. Currently we are using the
     * input parameter as a flag. An API definition change would be too
     * drastic at this point and can be addressed in the 2.0 timeframe.
     */
    handle = target;

    /* Restart the target, so that it is in correct state when htc_host_init is called again */
    /* Avoid any IO if Target is shutdown, to prevent undefined behaviour.*/
    if (target->target_state != HTC_TARGET_SHUTDOWN)
        htc_host_restart_target(target);

    if (handle != NULL)
    {
        HIFDetachHTC(handle->device);
        HIFShutDownDevice(handle->device);
    }

    /*
     * Releases the pending requests for the device in the same context. It
     * also results in calling of the pRemove function of the function driver.
     */
    if ((target == NULL) && (!funcDrvUnregistered))
    {
        funcDrvUnregistered = TRUE;
        HIFShutDownDevice(NULL);
    }
    if (handle != NULL)
    {
        /*
         * Ensure that all the pending asynchronous register read/writes
         * have finished.
         */
        regList = &handle->regList;
        HTC_DEBUG_ASSERT(regList != NULL);
        for (count = 0; count < HTC_REG_REQUEST_LIST_SIZE; count ++)
        {
            element = &regList->element[count];
            HTC_DEBUG_ASSERT(IS_ELEMENT_FREE(element));
        }

        /* Initialize the shadow copy of the handle register table */
        OSAL_MEMZERO(&handle->table, sizeof(HTC_REGISTER_TABLE));
        handle->ready = FALSE;


        /* Freeing handle memory */
        delTargetInstance(handle);


        OSAL_MEMZERO(handle, sizeof(HTC_TARGET));
        OSAL_FREE(handle);
    }

    is_initialised = OSAL_FALSE;

    HTC_DEBUG_PRINTF(HTC_DEBUG_TRC, ("htc_host_shutdown: Exit\n"));

    return HTC_OK;
}

htc_status_t htc_host_txdata_cleanup(HTC_TARGET *target)
{
    HTC_ENDPOINT *endPoint;
    HTC_ENDPOINT_ID endPointId;
    HTC_DATA_REQUEST_QUEUE *sendQueue;

    if (!isTargetFound(target))
        return HTC_ERROR;

    for (endPointId = ENDPOINT2; endPointId < HTC_MAILBOX_NUM_MAX; endPointId++)
    {
        endPoint = &target->endPoint[endPointId];
        sendQueue = &endPoint->sendQueue;
        flushEndpointQueue(endPoint, sendQueue, HTC_BUFFER_SENT);
    }
    return HTC_OK;
}

htc_status_t htc_host_get_num_htctargets_detected (uint32 *htc_devices)
{
    OSAL_UINT32 i=0, count=0;

	if (!is_initialised)
		return HTC_ERROR;
	
    if (htc_devices == NULL) return HTC_EINVAL;
    
	for (i=0; i < HIF_MAX_DEVICES; i++) {
		if (htc_target_dev[i] != NULL) {
			count++;
		}
	}
    *htc_devices = count;
    return HTC_OK;
}

htc_status_t htc_host_get_device_list (HTC_TARGET **handles_buf, uint32 buf_size)
{
    OSAL_UINT32 i=0, j=0;

	if (!is_initialised)
		return HTC_ERROR;
	
    if (buf_size > HIF_MAX_DEVICES) return HTC_EINVAL;
    
    for (j=0; j < HIF_MAX_DEVICES; j++) {
        if (htc_target_dev[j] != NULL) {
            if (i < buf_size)
                handles_buf[i++] = htc_target_dev[j];
        }
    }
    
    return HTC_OK;
}


htc_status_t htc_host_dump_target_registers(HTC_TARGET *target)
{
    htc_status_t status;
    OSAL_UINT32 address;

    if (!is_initialised || !isTargetFound(target))
        return HTC_ERROR;

    address = getRegAddr(INT_STATUS_REG, ENDPOINT_UNUSED);
    status = HIFReadWrite(target->device, address,
            &target->table.host_int_status, 28, HIF_RD_SYNC_BYTE_INC, NULL);

    dumpRegisters(target);

    return status;
}
