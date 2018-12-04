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
 * @file  htc_demo.c
 * @brief Test application for HTC Host
*/

/*==================================================================================

                           EDIT HISTORY FOR FILE

This section contains comments describing changes made to this file.
Notice that changes are listed in reverse chronological order.

when       who     what, where, why
--------   ---     -----------------------------------------------------------------
08/11/2017   mmtd    Initial version
==================================================================================*/

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "com_dtypes.h"
#include "qcli_api.h"
#include "qapi_status.h"
#include "htc_demo.h"
#include "malloc.h"
#include "qapi_types.h"


/* Header files */
#include "osal_types.h"
#include "htc_defs.h"
#include "htc.h"



extern QCLI_Group_Handle_t qcli_peripherals_group;              /* Handle for our peripherals subgroup. */
QCLI_Group_Handle_t qcli_htcdemo_hdl;

/* Prototypes */
QCLI_Command_Status_t htc_raw_init(uint32_t parameters_count,
                                     QCLI_Parameter_t * parameters);
QCLI_Command_Status_t htc_raw_shutdown(uint32_t parameters_count,
                                    QCLI_Parameter_t * parameters);
QCLI_Command_Status_t htc_raw_list_devices(uint32_t parameters_count,
                                     QCLI_Parameter_t * parameters);
QCLI_Command_Status_t htc_raw_open(uint32_t parameters_count,
                                      QCLI_Parameter_t * parameters);
QCLI_Command_Status_t htc_raw_close(uint32_t parameters_count,
                                      QCLI_Parameter_t * parameters);
QCLI_Command_Status_t htc_raw_read(uint32_t parameters_count,
                                      QCLI_Parameter_t * parameters);
QCLI_Command_Status_t htc_raw_write(uint32_t parameters_count,
                                      QCLI_Parameter_t * parameters);
QCLI_Command_Status_t htc_raw_dump_target_regs(uint32_t parameters_count,
                                     QCLI_Parameter_t * parameters);


#ifndef HTC_NR_TARGETS
#define HTC_NR_TARGETS 2 /* Arbitrary number to test multiple instances support */
#endif

#define HTC_RAW_READ_BUF_AVAILABLE_EVT       0x01
#define HTC_RAW_WRITE_BUF_AVAILABLE_EVT       0x01

typedef struct {
    int currPtr;
    int length;
    unsigned char data[HTCTARGET_BUFFER_SIZE];
} raw_htc_buffer;

unsigned char *app_buffer;


OSAL_UINT32 debughtc = (HTC_DEBUG_ERR | HTC_DEBUG_WARN |HTC_DEBUG_INF | HTC_DEBUG_DUMP | HTC_DEBUG_SEND | HTC_DEBUG_TRC | HTC_DEBUG_ALERT);
/*OSAL_UINT32 debughtc = (HTC_DEBUG_ERR | HTC_DEBUG_WARN | HTC_DEBUG_ALERT); */


typedef  raw_htc_buffer (*raw_htc_read_buffer_t)[RAW_HTC_READ_BUFFERS_NUM];
typedef  raw_htc_buffer (*raw_htc_write_buffer_t)[RAW_HTC_WRITE_BUFFERS_NUM];
static raw_htc_read_buffer_t raw_htc_read_buffer = NULL;
static raw_htc_write_buffer_t raw_htc_write_buffer = NULL;

static OSAL_BOOL write_buffer_available[HTC_MAILBOX_NUM_MAX];
static OSAL_BOOL read_buffer_available[HTC_MAILBOX_NUM_MAX];


static qurt_mutex_t htc_raw_read_mutex[HTC_MAILBOX_NUM_MAX];
static qurt_mutex_t htc_raw_write_mutex[HTC_MAILBOX_NUM_MAX];
static qurt_signal_t raw_htc_read_queue[HTC_MAILBOX_NUM_MAX];
static qurt_signal_t raw_htc_write_queue[HTC_MAILBOX_NUM_MAX];


static  HTC_TARGET *htc_target_devices[HTC_NR_TARGETS];

const QCLI_Command_t htc_demo_cmd_list[] =
{
   /* cmd_function      thread  cmd_string  usage_string    description */
    {htc_raw_init,    false,  "init",    "\n",           "init\n"},
    {htc_raw_shutdown,     false,  "shutdown",     "\n",           "shutdown\n"},
    {htc_raw_list_devices,    false,  "listtargets",    "\n",           "listtargets\n"},
    {htc_raw_dump_target_regs,    false,  "dump target regs",    "\n",           "dump target regs\n"},
    {htc_raw_open,   false,  "open",   "\n",           "open\n"},
    {htc_raw_close,   false,  "close",   "\n",           "close\n"},
    {htc_raw_read,   false,  "read",   "\n",           "read\n"},
    {htc_raw_write,   false,  "write",   "\n",           "write\n"},
};

const QCLI_Command_Group_t htc_demo_cmd_group =
{
    "HTCHost",
    sizeof(htc_demo_cmd_list)/sizeof(htc_demo_cmd_list[0]), /* Command_Count */
    htc_demo_cmd_list        /* Command_List */
};

QCLI_Command_Status_t HTCStatus_To_QCLIStatus(htc_status_t status)
{
	QCLI_Command_Status_t retval = QCLI_STATUS_SUCCESS_E;
	switch (status) {
		case HTC_OK:
			retval = QCLI_STATUS_SUCCESS_E;
			break;
		case HTC_ERROR:
			retval = QCLI_STATUS_ERROR_E;
			break;
		default:
			retval = QCLI_STATUS_ERROR_E;
			break;
	}
	return retval;
}

void htc_init_txbuffer(uint8 *buf, uint32 len)
{
    uint32 i=0;
    for (i=0; i<len; i++)
        buf[i]=i;
    return ;
}

void htc_dump_buffer(uint8 *buf, uint32 len)
{
    uint32 i=0;
    for (i=0; i<len; i++)
		QCLI_Printf(qcli_htcdemo_hdl, "%x ",buf[i]);
    return ;
}

raw_htc_buffer *
get_filled_buffer(HTC_ENDPOINT_ID endPointId)
{
    int count;
    raw_htc_buffer *busy;

    /* Check for data */
    for (count = 0; count < RAW_HTC_READ_BUFFERS_NUM; count ++) {
        busy = &raw_htc_read_buffer[endPointId][count];
        if (busy->length) {
            break;
        }
    }
    if (busy->length) {
        read_buffer_available[endPointId] = TRUE;
    } else {
        read_buffer_available[endPointId] = FALSE;
    }

    return busy;
}

raw_htc_buffer *
get_free_buffer(HTC_ENDPOINT_ID endPointId)
{
    int count;
    raw_htc_buffer *free;

    free = NULL;
    for (count = 0; count < RAW_HTC_WRITE_BUFFERS_NUM; count ++) {
        free = &raw_htc_write_buffer[endPointId][count];
        if (free->length == 0) {
            break;
        }
    }
    if (!free->length) {
        write_buffer_available[endPointId] = TRUE;
    } else {
        write_buffer_available[endPointId] = FALSE;
    }

    return free;
}

static void
htc_target_destroy(HTC_TARGET *htcTarget, OSAL_BOOL surprise)
{
    if(htcTarget == NULL) {
		QCLI_Printf(qcli_htcdemo_hdl, "%s(): Failed to get device structure.\n", __func__);
        return;
    }

	if (surprise) {
		
		QCLI_Printf(qcli_htcdemo_hdl, "%s(): Surprise removal detected\n", __func__);    
    	/* Cleanup HTC */
	    htc_host_stop(htcTarget);
        //htc_host_shutdown(htcTarget);
    }
}

static void htc_dump_rxbuffer(raw_htc_buffer *buffer)
{
	uint32 length;
	int readPtr, i;

	length = buffer->length - HTC_HEADER_LEN;
	readPtr = buffer->currPtr;

	QCLI_Printf(qcli_htcdemo_hdl, "%s Received Packet of Length %d \n", __func__, length);
	
	/* Packet has been completely read. Queue it with HTC */
	for (i=0; i < length; i++) 
	{
		QCLI_Printf(qcli_htcdemo_hdl, "%x ",buffer->data[readPtr+i]);
	}
	QCLI_Printf(qcli_htcdemo_hdl, "\n");
}

static void htc_drop_rxbuffer(HTC_TARGET *htcTarget, uint8 endPointId )
{
	raw_htc_buffer *busy;
	uint32 length;
	
	QCLI_Printf(qcli_htcdemo_hdl, "%s NO READ REQUESTS in Queue, DROPPING PACKET\n", __func__);

	qurt_mutex_lock(&htc_raw_read_mutex[endPointId]);
	busy = get_filled_buffer(endPointId);

	htc_dump_rxbuffer(busy);

	length = busy->length - HTC_HEADER_LEN;

	
	busy->currPtr += length;
	if (busy->currPtr == busy->length)
	{
		QCLI_Printf(qcli_htcdemo_hdl, "%s PACKET DROPPED COMPLETELY \n", __func__);
		
		/* Packet has been completely read. Queue it with HTC */
		memset(busy, 0, sizeof(raw_htc_buffer));
		htc_host_buffer_receive(htcTarget, endPointId, busy->data,
						 HTCTARGET_BUFFER_SIZE, busy);
	} else {
		QCLI_Printf(qcli_htcdemo_hdl, "%s INComplete PACKET RECEIVED \n", __func__);
	}

	read_buffer_available[endPointId] = FALSE;
	qurt_mutex_unlock(&htc_raw_read_mutex[endPointId]);

}

static void
htc_raw_read_complete_cb(HTC_TARGET *htcTarget, HTC_ENDPOINT_ID endPointId,
                       HTC_EVENT_ID evId, HTC_EVENT_INFO *evInfo, void *arg)
{
    HTC_TARGET *target;
    raw_htc_buffer *busy;

    target = (HTC_TARGET *)arg;
    ASSERT(target != NULL);
    busy = (raw_htc_buffer *)evInfo->cookie;
    ASSERT(busy != NULL);

    if (evInfo->status == HTC_ECANCELED) {
        /*
         * HTC provides HTC_ECANCELED status when it doesn't want to be refilled
         * (probably due to a shutdown)
         */
        memset(busy, 0, sizeof(raw_htc_buffer));
        return;
    }

	qurt_mutex_lock(&htc_raw_read_mutex[endPointId]);
    

    ASSERT(evId == HTC_BUFFER_RECEIVED);
    ASSERT((evInfo->status != HTC_OK) || 
             (evInfo->buffer == (busy->data + HTC_HEADER_LEN)));

    busy->length = evInfo->actualLength + HTC_HEADER_LEN;
    busy->currPtr = HTC_HEADER_LEN;
    read_buffer_available[endPointId] = TRUE;
	qurt_mutex_unlock(&htc_raw_read_mutex[endPointId]);

#ifdef QCLI_HTC_DEBUG
	QCLI_Printf(qcli_htcdemo_hdl,"buffer length %d of endpoint(%d) \n",busy->length,  endPointId);
    /* Signal the waiting process */
    QCLI_Printf(qcli_htcdemo_hdl,"Waking up the endpoint(%d) read process\n", endPointId);
#endif

	htc_dump_rxbuffer(busy);
	
    qurt_signal_set(&raw_htc_read_queue[endPointId],HTC_RAW_READ_BUF_AVAILABLE_EVT);
}

static void
htc_raw_write_complete_cb(HTC_TARGET *htcTarget, HTC_ENDPOINT_ID endPointId,
                        HTC_EVENT_ID evId, HTC_EVENT_INFO *evInfo, void *arg)
{
    HTC_TARGET *target;
    raw_htc_buffer *free;

    target = (HTC_TARGET *)arg;
    ASSERT(target != NULL);
    free = (raw_htc_buffer *)evInfo->cookie;
    ASSERT(free != NULL);

    if (evInfo->status == HTC_ECANCELED) {
        /*
         * HTC provides HTC_ECANCELED status when it doesn't want to be refilled
         * (probably due to a shutdown)
         */
        memset(free, 0, sizeof(raw_htc_buffer));
        return;
    }

    qurt_mutex_lock(&htc_raw_write_mutex[endPointId]);
	
#ifdef QCLI_HTC_DEBUG
    QCLI_Printf(qcli_htcdemo_hdl," %s took semaphore endPointId %d\n", __func__, endPointId);
#endif
    ASSERT(evId == HTC_BUFFER_SENT);
    ASSERT(evInfo->buffer == (free->data + HTC_HEADER_LEN));

    free->length = 0;
    write_buffer_available[endPointId] = TRUE;
    qurt_mutex_unlock(&htc_raw_write_mutex[endPointId]);
	
	QCLI_Printf(qcli_htcdemo_hdl, "HTCHost Write: Sent Packet Successfully\n");
	
#ifdef QCLI_HTC_DEBUG
    /* Signal the waiting process */
    QCLI_Printf(qcli_htcdemo_hdl,"Waking up the endpoint(%d) write process\n", endPointId);
#endif
    qurt_signal_set(&raw_htc_write_queue[endPointId], HTC_RAW_WRITE_BUF_AVAILABLE_EVT);
}

static void
htc_raw_data_available_cb(HTC_TARGET *htcTarget, HTC_ENDPOINT_ID eid,
                         HTC_EVENT_ID evId, HTC_EVENT_INFO *evInfo, void *arg)
{
    HTC_TARGET *target;

    target = (HTC_TARGET *)arg;
    ASSERT(target != NULL);
	
    QCLI_Printf(qcli_htcdemo_hdl,"%s App Buffer not available, dropping packet\n", __func__);
	
	htc_drop_rxbuffer(htcTarget, eid);
}


/* HTC event handlers */
static void
htc_target_available_handler(HTC_TARGET *htcTarget, HTC_ENDPOINT_ID eid, HTC_EVENT_ID event,
                HTC_EVENT_INFO *evInfo,
                void *arg)
{
	int i;
	
	/* Allocate the devices */
	QCLI_Printf(qcli_htcdemo_hdl, "htc_target_available\n");

	for (i=0; i < HTC_NR_TARGETS; i++) {
		if (htc_target_devices[i] == NULL) {
			break;
		}
	}
	if (i == HTC_NR_TARGETS) {
		QCLI_Printf(qcli_htcdemo_hdl, "htc_target_available: max devices reached\n");
		return;
	}

	/* Allocate the devices */
	htc_target_devices[i] = htcTarget;
}


static void
htc_target_removed_handler(HTC_TARGET *htcTarget, HTC_ENDPOINT_ID eid,
                  HTC_EVENT_ID event, HTC_EVENT_INFO *evInfo, void *arg)
{
	int i;
	
	/* Allocate the devices */
	for (i=0; i < HTC_NR_TARGETS; i++) {
		if (htc_target_devices[i] != NULL) {
			if (htc_target_devices[i] == htcTarget) {
				htc_target_devices[i] = NULL;
				QCLI_Printf(qcli_htcdemo_hdl, "htc_target %d removed\n", i);
			
				htc_target_destroy(htcTarget, *((OSAL_BOOL *)evInfo->cookie));
			}
		}
	}
}

/*****************************************************************************
 * This function is used to register the Fs Command Group with QCLI.
 *****************************************************************************/
void Initialize_HTCHost_Demo(void)
{
    /* Attempt to reqister the Command Groups with the qcli framework.*/
    qcli_htcdemo_hdl = QCLI_Register_Command_Group(qcli_peripherals_group, &htc_demo_cmd_group);
    if (qcli_htcdemo_hdl)
    {
        QCLI_Printf(qcli_htcdemo_hdl, "HTCHost Registered\n");
    }
    return;
}


QCLI_Command_Status_t htc_raw_init(uint32_t parameters_count,
                                     QCLI_Parameter_t * parameters)
{
	htc_status_t status = HTC_OK;

	raw_htc_read_buffer = (raw_htc_read_buffer_t) malloc((HTC_MAILBOX_NUM_MAX * RAW_HTC_READ_BUFFERS_NUM) * sizeof(raw_htc_buffer));
	if (raw_htc_read_buffer == NULL) return QCLI_STATUS_ERROR_E;

	raw_htc_write_buffer = (raw_htc_write_buffer_t) malloc((HTC_MAILBOX_NUM_MAX * RAW_HTC_WRITE_BUFFERS_NUM) * sizeof(raw_htc_buffer));
	if (raw_htc_write_buffer == NULL) return QCLI_STATUS_ERROR_E;

	app_buffer = (unsigned char *) malloc(HTCTARGET_BUFFER_SIZE);
	if (app_buffer == NULL) return QCLI_STATUS_ERROR_E;

	htc_init_txbuffer(app_buffer, HTCTARGET_BUFFER_SIZE);

	status = htc_host_init();
	if(status != HTC_OK)
	{
		QCLI_Printf(qcli_htcdemo_hdl, "HTCHost Initialization Failed!\n");
        if (raw_htc_read_buffer != NULL)
        {
            free(raw_htc_read_buffer);
            raw_htc_read_buffer = NULL;
        }

        if (raw_htc_write_buffer != NULL)
        {
            free(raw_htc_write_buffer);
            raw_htc_write_buffer = NULL;
        }

        if (app_buffer != NULL)
        {
            free(app_buffer);
            app_buffer = NULL;
        }
		return QCLI_STATUS_ERROR_E;
	}

	htc_host_event_register(NULL, ENDPOINT_UNUSED, HTC_TARGET_AVAILABLE,
			htc_target_available_handler, NULL);
	htc_host_event_register(NULL, ENDPOINT_UNUSED, HTC_TARGET_UNAVAILABLE,
			htc_target_removed_handler, NULL);
	QCLI_Printf(qcli_htcdemo_hdl, "HTCHost Initialization Successfull\n");

    return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t htc_raw_shutdown(uint32_t parameters_count,
                                     QCLI_Parameter_t * parameters)
{
    uint32 i;

    /* Allocate the devices */
    for (i=0; i < HTC_NR_TARGETS; i++) {
        if (htc_target_devices[i] != NULL) {
            QCLI_Printf(qcli_htcdemo_hdl, "Shutting down target id %d \n", i);
            htc_host_shutdown(htc_target_devices[i]);
        }
    }

    if (raw_htc_read_buffer != NULL)
    {
        free(raw_htc_read_buffer);
        raw_htc_read_buffer = NULL;
    }

    if (raw_htc_write_buffer != NULL)
    {
        free(raw_htc_write_buffer);
        raw_htc_write_buffer = NULL;
    }

    if (app_buffer != NULL)
    {
        free(app_buffer);
        app_buffer = NULL;
    }

    QCLI_Printf(qcli_htcdemo_hdl, "HTCHost Shutdown Successfull\n");
    /* Freed successfully */
    return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t htc_raw_list_devices(uint32_t parameters_count,
                                     QCLI_Parameter_t * parameters)
{
	uint32 i=0;
	
	/* Allocate the devices */
	for (i=0; i < HTC_NR_TARGETS; i++) {
		if (htc_target_devices[i] != NULL) {
			QCLI_Printf(qcli_htcdemo_hdl, "Detected target id %d \n", i);
		}
	}
    return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t htc_raw_open(uint32_t parameters_count,
                                     QCLI_Parameter_t * parameters)
{
	htc_status_t status;
	int ep_id, i;
	raw_htc_buffer *buffer;
    uint32_t num_htc_tgt_found = 0;
    HTC_TARGET *htcTarget;
    uint8 htc_target_id = (uint8)parameters[0].Integer_Value;

    htc_host_get_num_htctargets_detected(&num_htc_tgt_found);
    QCLI_Printf(qcli_htcdemo_hdl, "HTC targets found %d\n", num_htc_tgt_found);

    if(!num_htc_tgt_found || htc_target_id > (num_htc_tgt_found-1))
    {
        QCLI_Printf(qcli_htcdemo_hdl, "Invalid target ID !\n");
        return QCLI_STATUS_ERROR_E;
    }
    else
    {
        htcTarget = htc_target_devices[htc_target_id];
    }

	if (htcTarget == NULL) {
		
		QCLI_Printf(qcli_htcdemo_hdl, "HTCHost Open Failed, HTCTarget NULL!\n");
	    return QCLI_STATUS_ERROR_E;
	}
	
	for (ep_id = 0; ep_id < HTC_MAILBOX_NUM_MAX; ep_id ++) {
		/* Initialize the data structures */
		qurt_mutex_init(&htc_raw_read_mutex[ep_id]);
		qurt_mutex_init(&htc_raw_write_mutex[ep_id]);

		qurt_signal_init(&raw_htc_read_queue[ep_id]);
		qurt_signal_init(&raw_htc_write_queue[ep_id]);

		/* Register the event handlers */
		if ((status = htc_host_event_register(htcTarget, ep_id, HTC_BUFFER_RECEIVED,
								  htc_raw_read_complete_cb, htcTarget)) != HTC_OK)
		{
		
			QCLI_Printf(qcli_htcdemo_hdl, "HTCHost Open Buf Rx Event Registration Failed !\n");
			return QCLI_STATUS_ERROR_E;
		}
		if ((status = htc_host_event_register(htcTarget, ep_id, HTC_DATA_AVAILABLE,
								  htc_raw_data_available_cb, htcTarget)) != HTC_OK)
		{
		
			QCLI_Printf(qcli_htcdemo_hdl, "HTCHost Open Data Avail. Event Registration Failed !\n");
			return QCLI_STATUS_ERROR_E;
		}
		if ((status = htc_host_event_register(htcTarget, ep_id, HTC_BUFFER_SENT,
								  htc_raw_write_complete_cb, htcTarget)) != HTC_OK)
		{
		
			QCLI_Printf(qcli_htcdemo_hdl, "HTCHost Open Buf. Sent. Event Registration Failed !\n");
			return QCLI_STATUS_ERROR_E;
		}

		for (i = 0; i < RAW_HTC_READ_BUFFERS_NUM; i++) {
			/* Initialize the receive buffers */
			buffer = &raw_htc_write_buffer[ep_id][i];
			memset(buffer, 0, sizeof(raw_htc_buffer));
			buffer = &raw_htc_read_buffer[ep_id][i];
			memset(buffer, 0, sizeof(raw_htc_buffer));

			/* Queue buffers to HTC for receive */
			if ((status = htc_host_buffer_receive(htcTarget, ep_id, buffer->data,
										   HTCTARGET_BUFFER_SIZE, buffer)) != HTC_OK)
			{
				QCLI_Printf(qcli_htcdemo_hdl, "HTCHost Open Recv Buf. queuing Failed !\n");			
				return QCLI_STATUS_ERROR_E;
			}
		}

		for (i = 0; i < RAW_HTC_WRITE_BUFFERS_NUM; i ++) {
			/* Initialize the send buffers */
			buffer = &raw_htc_write_buffer[ep_id][i];
			memset(buffer, 0, sizeof(raw_htc_buffer));
		}

		read_buffer_available[ep_id] = FALSE;
		write_buffer_available[ep_id] = TRUE;
	}
	/* Start the HTC component */
	if ((status = htc_host_start(htcTarget)) != HTC_OK) {
		QCLI_Printf(qcli_htcdemo_hdl, "HTCHost Open htc_host_start Failed !\n");
    	return QCLI_STATUS_ERROR_E;
	}
	QCLI_Printf(qcli_htcdemo_hdl, "HTCHost Open Successfull\n");
    return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t htc_raw_close(uint32_t parameters_count,
                                     QCLI_Parameter_t * parameters)
{
	int count;
	htc_status_t status;
    uint8 htc_target_id = (uint8)parameters[0].Integer_Value;
	HTC_TARGET *htcTarget = htc_target_devices[htc_target_id];

	if (htcTarget == NULL) {
		QCLI_Printf(qcli_htcdemo_hdl, "HTCHost Close Failed, HTCTarget NULL!\n");
		return QCLI_STATUS_ERROR_E;
	}

	/* Stop the HTC */
	htc_host_stop(htcTarget);

	/* Unregister the event handlers */
	for (count = 0; count < HTC_MAILBOX_NUM_MAX; count ++) {
		status = htc_host_event_deregister(htcTarget, count, HTC_BUFFER_RECEIVED);
		status = htc_host_event_deregister(htcTarget, count, HTC_DATA_AVAILABLE);
		status = htc_host_event_deregister(htcTarget, count, HTC_BUFFER_SENT);
		
		qurt_mutex_destroy(&htc_raw_write_mutex[count]);
		qurt_mutex_destroy(&htc_raw_read_mutex[count]);
		qurt_signal_destroy(&raw_htc_read_queue[count]);
		qurt_signal_destroy(&raw_htc_write_queue[count]);
	}
	QCLI_Printf(qcli_htcdemo_hdl, "HTCHost Close Successfull!\n");

	return HTCStatus_To_QCLIStatus(status);
}

QCLI_Command_Status_t htc_raw_read(uint32_t parameters_count,
                                     QCLI_Parameter_t * parameters)
{
	int readPtr;
	raw_htc_buffer *busy;
    uint8 htc_target_id = (uint8)parameters[0].Integer_Value;
	HTC_TARGET *htcTarget = htc_target_devices[htc_target_id];
    uint8 endPointId = (uint8)parameters[1].Integer_Value;
    uint32 length = (uint32)parameters[2].Integer_Value;
    uint32 signalled = 0;

	if (htcTarget == NULL) {
		QCLI_Printf(qcli_htcdemo_hdl, "HTCHost Read Failed, HTCTarget NULL!\n");
		
	    return QCLI_STATUS_ERROR_E;
	}


	qurt_mutex_lock(&htc_raw_read_mutex[endPointId]);

	busy = get_filled_buffer(endPointId);
	while (!read_buffer_available[endPointId]) {
		qurt_mutex_unlock(&htc_raw_read_mutex[endPointId]);

		/* Wait for the data */
		QCLI_Printf(qcli_htcdemo_hdl,"Sleeping endpoint(%d) read process\n", endPointId);

		signalled = qurt_signal_wait(&raw_htc_read_queue[endPointId],
												  HTC_RAW_READ_BUF_AVAILABLE_EVT,
												  QURT_SIGNAL_ATTR_WAIT_ANY);

        qurt_signal_clear(&raw_htc_read_queue[endPointId], signalled);                                                  
		
		QCLI_Printf(qcli_htcdemo_hdl,"Read waked up endpoint(%d) read process\n", endPointId);
									 
		qurt_mutex_lock(&htc_raw_read_mutex[endPointId]);
		busy = get_filled_buffer(endPointId);
	}

	/* Read the data */
	readPtr = busy->currPtr;
	if (length > busy->length - HTC_HEADER_LEN) {
		length = busy->length - HTC_HEADER_LEN;
	}
	
    memset(app_buffer, 0, HTCTARGET_BUFFER_SIZE);
	memcpy(app_buffer, &busy->data[readPtr], length);
	
	busy->currPtr += length;
	if (busy->currPtr == busy->length)
	{
		QCLI_Printf(qcli_htcdemo_hdl, "%s PACKET RECEIVED COMPLETELY \n", __func__);
		/* Packet has been completely read. Queue it with HTC */
		memset(busy, 0, sizeof(raw_htc_buffer));
		htc_host_buffer_receive(htcTarget, endPointId, busy->data,
						 HTCTARGET_BUFFER_SIZE, busy);
	} else {
	QCLI_Printf(qcli_htcdemo_hdl, "%s INComplete PACKET RECEIVED \n", __func__);
	}

	read_buffer_available[endPointId] = FALSE;
	qurt_mutex_unlock(&htc_raw_read_mutex[endPointId]);
	htc_dump_buffer(app_buffer, length);
	
	QCLI_Printf(qcli_htcdemo_hdl,"HTCHost Read Successfull\n");

    return QCLI_STATUS_SUCCESS_E;
}

									 
QCLI_Command_Status_t htc_raw_write(uint32_t parameters_count,
                                     QCLI_Parameter_t * parameters)
{
	int writePtr;
	raw_htc_buffer *free;
	
    uint8 htc_target_id = (uint8)parameters[0].Integer_Value;
	HTC_TARGET *htcTarget = htc_target_devices[htc_target_id];
    uint8 endPointId = (uint8)parameters[1].Integer_Value;
    uint32 length = (uint32)parameters[2].Integer_Value;

	if (htcTarget == NULL) {
		QCLI_Printf(qcli_htcdemo_hdl, "HTCHost Write Failed, HTCTarget NULL!\n");
		
	    return QCLI_STATUS_ERROR_E;
	}

    qurt_mutex_lock(&htc_raw_write_mutex[endPointId]);

	/* Search for a free buffer */
	free = get_free_buffer(endPointId);

	/* Check if there is space to write else wait */
	while (!write_buffer_available[endPointId]) {
		qurt_mutex_unlock(&htc_raw_write_mutex[endPointId]);

		/* Wait for buffer to become free */
		qurt_signal_wait(&raw_htc_write_queue[endPointId],
												  HTC_RAW_WRITE_BUF_AVAILABLE_EVT,
												  QURT_SIGNAL_ATTR_WAIT_ANY);	

		qurt_mutex_lock(&htc_raw_write_mutex[endPointId]);

		free = get_free_buffer(endPointId);
	}

	/* Send the data */
	writePtr = HTC_HEADER_LEN;
	if (length > (HTCTARGET_BUFFER_SIZE - HTC_HEADER_LEN)) {
		length = HTCTARGET_BUFFER_SIZE - HTC_HEADER_LEN;
	}
	
	htc_init_txbuffer(app_buffer, HTCTARGET_BUFFER_SIZE);
	
	QCLI_Printf(qcli_htcdemo_hdl, "HTCHost Write: Sending Packet of length %d on Endpoint %d\n", length, endPointId);
	htc_dump_buffer(app_buffer, length);

	memcpy(&free->data[writePtr], app_buffer, length);

	free->length = length;
    if(htc_host_buffer_send(htcTarget, endPointId, &free->data[writePtr],
                            length, free) != HTC_OK)
    {
        free->length = 0;
        write_buffer_available[endPointId] = TRUE;
        return QCLI_STATUS_ERROR_E;
    }
	write_buffer_available[endPointId] = FALSE;
	qurt_mutex_unlock(&htc_raw_write_mutex[endPointId]);
	
	
    return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t htc_raw_dump_target_regs(uint32_t parameters_count,
                                     QCLI_Parameter_t * parameters)
{
    uint8 htc_target_id = (uint8)parameters[0].Integer_Value;
	HTC_TARGET *htcTarget = htc_target_devices[htc_target_id];
	
	htc_host_dump_target_registers(htcTarget);
	
    return QCLI_STATUS_SUCCESS_E;
}
