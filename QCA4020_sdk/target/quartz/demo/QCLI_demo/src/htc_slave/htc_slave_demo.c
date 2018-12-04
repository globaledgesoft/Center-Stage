/*==================================================================================
       Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
                       All Rights Reserved.
==================================================================================*/
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
 * @file  htc_slave_demo.c
 * @brief Test application for HTC Slave
*/

/*==================================================================================

                           EDIT HISTORY FOR FILE

This section contains comments describing changes made to this file.
Notice that changes are listed in reverse chronological order.

when       who     what, where, why
--------   ---     -----------------------------------------------------------------
21/09/17   mmtd    Initial version
==================================================================================*/

/*
 * htc slave ping is a test/demo application, mainly for use during bringup.
 * This application runs on the Target and sits on top of the Host Target
 * Communications (HTC) module.  It waits for messages from the Host to
 * be received, and it immediately echoes each message back to the Host.  
 *
 */

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "com_dtypes.h"
#include "qcli_api.h"
#include "qapi_status.h"
#include "qapi_slp.h"
#include "qapi_htc_slave.h"
#include "htc_slave_demo.h"
#include "malloc.h"

/*
 * Maximum size of a complete HTC message that includes the size of HTC header
 * i.e. QAPI_HTC_HDR_SZ.
 */
#define HTC_MSG_SIZE 1536
#define NUM_PING_BUFFERS_PER_ENDPOINT  QAPI_NUM_BUFFERS_PER_ENDPOINT
#define PING_BUFSZ 256

extern QCLI_Group_Handle_t qcli_peripherals_group;              /* Handle for our peripherals subgroup. */
QCLI_Group_Handle_t qcli_htc_slave_hdl;


/* Prototypes */
QCLI_Command_Status_t htc_slave_demo_start(uint32_t parameters_count,
                                     QCLI_Parameter_t * parameters);
QCLI_Command_Status_t htc_slave_demo_stop(uint32_t parameters_count,
                                    QCLI_Parameter_t * parameters);
QCLI_Command_Status_t htc_slave_demo_pause(uint32_t parameters_count,
                                     QCLI_Parameter_t * parameters);
QCLI_Command_Status_t htc_slave_demo_resume(uint32_t parameters_count,
                                      QCLI_Parameter_t * parameters);
QCLI_Command_Status_t htc_slave_demo_inc_txcredit(uint32_t parameters_count,
                                     QCLI_Parameter_t * parameters);
QCLI_Command_Status_t htc_slave_demo_recycle_htcbuf(uint32_t parameters_count,
                                     QCLI_Parameter_t * parameters);
QCLI_Command_Status_t htc_slave_demo_dump_recv_queued_buffers(uint32_t parameters_count,
                                     QCLI_Parameter_t * parameters);
QCLI_Command_Status_t htc_slave_demo_dump_send_queued_buffers(uint32_t parameters_count,
                                     QCLI_Parameter_t * parameters);
QCLI_Command_Status_t htc_slave_demo_dump_recycle_queue_htcbuf(uint32_t parameters_count,
                                     QCLI_Parameter_t * parameters);


qapi_HTC_bufinfo_t *htc_buf_list[QAPI_HTC_SLAVE_MAX_ENDPOINTS];
static  qapi_HTC_bufinfo_t    *HTC_buf_free_pool = NULL;


static uint32 transId = QAPI_HTC_SDIO_SLAVE_INSTANCE_ID;


const QCLI_Command_t htc_slave_cmd_list[] =
{
   /* cmd_function      thread  cmd_string  usage_string    description */
    {htc_slave_demo_start,    false,  "Start",    "\n",           "Start Loopback test\n"},
    {htc_slave_demo_stop,     false,  "Stop",     "\n",           "Stop HTC_Slave application\n"},
    {htc_slave_demo_pause,    false,  "Pause",    "\n",           "Pause MBox\n"},
    {htc_slave_demo_resume,   false,  "Resume",   "\n",           "Resume MBox\n"},
    {htc_slave_demo_inc_txcredit,   false,  "TxCreditInc",   "\n",           "Increment Host TX credit\n"},
    {htc_slave_demo_recycle_htcbuf,   false,  "Recycle htc bufs",   "\n",    "Recycle free HTC buffers\n"},
    {htc_slave_demo_dump_recv_queued_buffers,   false,  "dump htc_bufs  rx queue",   "\n",    "dump htc_bufs  rx queue\n"},
    {htc_slave_demo_dump_send_queued_buffers,   false,  "dump htc_bufs tx queue",   "\n",    "dump htc_bufs  rx queue\n"},    
    {htc_slave_demo_dump_recycle_queue_htcbuf,   false,  "dump recycle htc_bufs queue",   "\n",    "dump recycle htc_bufs queue\n"},
};

const QCLI_Command_Group_t htc_slave_cmd_group =
{
    "HTCSlave",   /* Group_String: will display cmd prompt as "HTC_Slave> " */
    sizeof(htc_slave_cmd_list)/sizeof(htc_slave_cmd_list[0]), /* Command_Count */
    htc_slave_cmd_list        /* Command_List */
};

/*****************************************************************************
 * This function is used to register the Fs Command Group with QCLI.
 *****************************************************************************/
void Initialize_HTCSlave_Demo(void)
{
    /* Attempt to reqister the Command Groups with the qcli framework.*/
    qcli_htc_slave_hdl = QCLI_Register_Command_Group(qcli_peripherals_group, &htc_slave_cmd_group);
    if (qcli_htc_slave_hdl)
    {
        QCLI_Printf(qcli_htc_slave_hdl, "MboxPing Registered\n");
    }

    return;
}

void debug_buffer_print(qapi_HTC_bufinfo_t *buf, const char *str, int endpoint)
{
    int num = 0;
    QCLI_Printf(qcli_htc_slave_hdl,"BUFS-%d-%s: ", endpoint, str);
    if (buf == NULL) {
        QCLI_Printf(qcli_htc_slave_hdl, "none");
    }

    while (buf) {
        QCLI_Printf(qcli_htc_slave_hdl, "[%d:0x%x:0x%x/0x%x] ",
                 num,
                 buf,
                 buf->buffer,
                 buf->actual_length);
        buf = buf->next;
        num++;
    }

    QCLI_Printf(qcli_htc_slave_hdl, "\n");
}




QCLI_Command_Status_t htc_slave_demo_alloc_msg_buflist (uint32 Instance,
   											qapi_HTCSlaveEndpointID_t mbox, 
   											qapi_HTC_bufinfo_t **htc_buf_list, 
   											uint32 msg_size)
{
	uint32 block_size = 0;
	uint32 size = 0, i=0;
	qapi_HTC_bufinfo_t *temp = NULL;
	qapi_HTC_bufinfo_t *prev = NULL;
	qapi_HTC_bufinfo_t *head = NULL;	
	uint8 *buf = NULL;
	
	if (htc_buf_list == NULL)   return QCLI_STATUS_USAGE_E;

	qapi_HTC_Slave_BlockSize_Get(transId, mbox, &block_size);

	size = msg_size + (block_size - 1); /* round to multiples of block size */
	size /= block_size;

	for (i=0; i<size; i++)
	{
		temp = malloc (sizeof(qapi_HTC_bufinfo_t));
		if (temp == NULL) return QCLI_STATUS_ERROR_E;
		
		buf = malloc (block_size);
		if (buf == NULL) return QCLI_STATUS_ERROR_E;
		
		temp->buffer_offset = 0;
		temp->buffer = buf;
		temp->actual_length = block_size;
		temp->next = NULL;
		
		if (i==0)
		{
			head = prev = temp;
		}
	
		if (prev != NULL) {
			prev->next = temp;
			prev = temp;
		}
	}
	*htc_buf_list = head;

    return QCLI_STATUS_SUCCESS_E;
}



QCLI_Command_Status_t htc_slave_demo_free_msg_buflist (uint32 Instance,
											qapi_HTC_bufinfo_t *htc_buf_list)
{
	qapi_HTC_bufinfo_t *node = htc_buf_list;
	qapi_HTC_bufinfo_t *h = htc_buf_list;	
	
	if (htc_buf_list == NULL)   return QCLI_STATUS_USAGE_E;

	while (h)
	{
		node = h;
		h = h->next;
		
		if (node->buffer != NULL)
			free(node->buffer);
		free(node);
	}
    return QCLI_STATUS_SUCCESS_E;
}

void htc_slave_enqueue_htcbuf(qapi_HTC_bufinfo_t *htc_buf)
{
    qapi_HTC_bufinfo_t *queue_tail = NULL;

    if (HTC_buf_free_pool == NULL) {
        HTC_buf_free_pool = htc_buf;
    } else {
        /* Traverse till end of the queue */
        queue_tail = HTC_buf_free_pool;
        while(queue_tail->next != NULL)
        {
            queue_tail = queue_tail->next;
        }
        queue_tail->next = htc_buf;
    }
}
qapi_HTC_bufinfo_t* htc_slave_dequeue_htcbuf(int num_bufs, int *pout_num_bufs)
{
    int cnt = 0;
    qapi_HTC_bufinfo_t *htc_buf = HTC_buf_free_pool;
    qapi_HTC_bufinfo_t *last_htc_buf = HTC_buf_free_pool;
    if ((htc_buf == NULL) || (num_bufs == 0)) {
        *pout_num_bufs = cnt;
        return NULL;
    } else {
        while ((HTC_buf_free_pool) && (cnt < num_bufs))
        {
            last_htc_buf = HTC_buf_free_pool;
            HTC_buf_free_pool = HTC_buf_free_pool->next;
            cnt++;
        }
        last_htc_buf->next = NULL;
        *pout_num_bufs = cnt;
        return htc_buf;
    }
}


static void htc_slave_initdone(uint32 Instance, uint32 arg1, uint32 arg2)
{
    qapi_HTCSlaveEndpointID_t endpoint = (qapi_HTCSlaveEndpointID_t) arg1;

    QCLI_Printf(qcli_htc_slave_hdl,
                "Host/Target Communications are initialized for endpoint %d+ \n", endpoint);

    /* override MBOX 0 max message size (defaults to 256) to be the same as the other
    * 3 mailboxes. This allows the host side mailbox 
    * application to perform 1500+ byte pings to verify mailbox 0 operation
    */
    
    qapi_HTC_Slave_Max_MsgSize_Set(transId, endpoint, HTC_MSG_SIZE);

    /* Allocate htc_buf for each endpoint */
	htc_slave_demo_alloc_msg_buflist (transId, 
									endpoint,
									&htc_buf_list[endpoint],
									HTC_MSG_SIZE);
	
    qapi_HTC_Slave_Receive_Data(transId, endpoint, htc_buf_list[endpoint]);

    QCLI_Printf(qcli_htc_slave_hdl,
                "Host/Target Communications are initialized for endpoint %d -\n", endpoint);    

    QCLI_Printf(qcli_htc_slave_hdl, "-\n");
}

static void htc_slave_senddone(uint32 Instance, uint32 arg1, uint32 arg2)
{
    qapi_HTCSlaveEndpointID_t endpoint = (qapi_HTCSlaveEndpointID_t) arg1;
    qapi_HTC_bufinfo_t *bufinfo = (qapi_HTC_bufinfo_t *) arg2;    
    qapi_HTC_bufinfo_t *cur = bufinfo;
    uint16 msg_len = 0;
    uint32 block_size = 0;
    uint32 i=0;

    QCLI_Printf(qcli_htc_slave_hdl,
        ">>>htc_slave_senddone htcbuffer:0x%x buffer:0x%x endpoint:%d\n",
         bufinfo, bufinfo->buffer, endpoint);

    qapi_HTC_Slave_BlockSize_Get(Instance, endpoint, &block_size);

    /* cur->next == NULL implies end of buffer chain succcessfully sent */
    while(cur)
    {
        msg_len += cur->actual_length;
        for(i=cur->buffer_offset; i<(cur->actual_length+cur->buffer_offset); i++)
        {
            QCLI_Printf(qcli_htc_slave_hdl, "%x ", cur->buffer[i]);
        }
        /* Reset the buffer elements to default values as at allocation time.*/
        cur->actual_length = block_size;
        /*
         * Ideally buffer_offset too needs to be reset or set appropriately.
         * But since we are doing a loopback test (i.e. receive and then send)
         * buffer_offset is appropriately set in the buffer chain provided
         * to application by HTC layer for the corresponding complete message
         * received from the Host.
         */
        cur->buffer_offset = 0;
        cur = cur->next;
    }

    QCLI_Printf(qcli_htc_slave_hdl, "Total message length: length %d\n",
                msg_len);

    /* Recycle the received buffer chain back to HTC layer */
    qapi_HTC_Slave_Receive_Data(Instance, endpoint, bufinfo);

	
	
#if 0
    /* Recycle buffers */
    qapi_HTC_Slave_Receive_Data(transId, endpoint, bufinfo);
#endif

}

static void htc_slave_recvdone(uint32 Instance, uint32 arg1, uint32 arg2)
{
    uint32 i=0;
    uint16 msg_len = 0;
    qapi_HTCSlaveEndpointID_t endpoint = (qapi_HTCSlaveEndpointID_t) arg1;
    qapi_HTC_bufinfo_t *bufinfo = (qapi_HTC_bufinfo_t *) arg2;    
    qapi_HTC_bufinfo_t *cur = (qapi_HTC_bufinfo_t *) arg2;    

    QCLI_Printf(qcli_htc_slave_hdl,
        ">>>htc_slave_recvdone (htcbuffer:0x%x) buffer=0x%x, endpoint=%d\n",
        bufinfo, bufinfo->buffer, endpoint);

    /* cur->next == NULL implies end of buffer chain succcessfully received */
    while(cur)
    {
        msg_len += cur->actual_length;
        for(i=0; i<cur->actual_length; i++)
        {
            QCLI_Printf(qcli_htc_slave_hdl, "%x ",
                        cur->buffer[cur->buffer_offset + i]);
        }
        cur = cur->next;
    }

    QCLI_Printf(qcli_htc_slave_hdl, "Total message length: length %d\n",
                msg_len);

    /*
     * Ideally here the buffer elements corresponding to the buffer chain
     * comprising the received message can be reset to default values.
     * But since we do a loopback test (i.e. transmit the received message
     * back to Host),
     * this reset is deferred to send done callback.
     */
    /* Now reflect this message */
    qapi_HTC_Slave_Send_Data(transId, endpoint, bufinfo);
}

static void htc_slave_resetdone(uint32 Instance, uint32 arg1, uint32 arg2)
{
    uint8 endpoint_used = 0;
    uint8 endpoint;
    uint32 msgsize=0; 
    qapi_Status_t status;
    
    status = qapi_HTC_Slave_Get_Num_Endpoints(transId, &endpoint_used);
    
    status = qapi_HTC_Slave_Stop(transId);
    
    qapi_HTC_Slave_Shutdown (transId);

   
    status = qapi_HTC_Slave_Init(transId);

    status = qapi_HTC_Event_Register (transId, 
                                      QAPI_HTC_SLAVE_INIT_COMPLETE,
                                      htc_slave_initdone,
                                      NULL);

    status = qapi_HTC_Event_Register (transId, 
                                      QAPI_HTC_SLAVE_BUFFER_RECEIVED,
                                      htc_slave_recvdone,
                                      NULL);

    status = qapi_HTC_Event_Register (transId, 
                                      QAPI_HTC_SLAVE_BUFFER_SENT,
                                      htc_slave_senddone,
                                      NULL);

    status = qapi_HTC_Event_Register (transId, 
                                      QAPI_HTC_SLAVE_RESET_COMPLETE,
                                      htc_slave_resetdone,
                                      NULL);


    status = qapi_HTC_Slave_Start(transId);

    for (endpoint=0; endpoint < endpoint_used; endpoint++) {
        QCLI_Printf(qcli_htc_slave_hdl,"Restart Mailbox Ping test on endpoint %d\n", endpoint);
        QCLI_Printf(qcli_htc_slave_hdl,"Buffers Per MBOX: %d\n", NUM_PING_BUFFERS_PER_ENDPOINT);
        QCLI_Printf(qcli_htc_slave_hdl,"Buffer Size: %d\n", PING_BUFSZ); 
        status = qapi_HTC_Slave_Max_MsgSize_Get(transId, (qapi_HTCSlaveEndpointID_t)endpoint, &msgsize);
        QCLI_Printf(qcli_htc_slave_hdl,"Max Message Size: %d\n", msgsize);        
        QCLI_Printf(qcli_htc_slave_hdl,"Max Message Size: %d\n", msgsize);
        QCLI_Printf(qcli_htc_slave_hdl,"Credits Per MBOX: %d\n", 
            (NUM_PING_BUFFERS_PER_ENDPOINT * PING_BUFSZ) / msgsize);
    }
    if(status){}
    
}

QCLI_Command_Status_t htc_slave_demo_start(uint32_t parameters_count,
                                     QCLI_Parameter_t * parameters)
{
    int endpoint;
    uint32 msgsize=0;
    uint8 endpoint_used;
    qapi_Status_t status;
    
    transId = (uint8)parameters[0].Integer_Value;

    if (transId >= QAPI_HTC_MAX_INSTANCES)
    {
        QCLI_Printf(qcli_htc_slave_hdl,"Invalid transport id\n");
        QCLI_Printf(qcli_htc_slave_hdl,"Enter SDIO: 0, SPI: 1\n");
        return QCLI_STATUS_ERROR_E;
    }

    /*qapi_Slp_Set_Max_Latency(QAPI_SLP_LAT_PERF); */

    status = qapi_HTC_Slave_Init(transId);

    /* Get number of endpoint supported */
    qapi_HTC_Slave_Get_Num_Endpoints(transId, &endpoint_used);
	

    status = qapi_HTC_Event_Register (transId, 
                                      QAPI_HTC_SLAVE_INIT_COMPLETE,
                                      htc_slave_initdone,
                                      NULL);

    status = qapi_HTC_Event_Register (transId, 
                                      QAPI_HTC_SLAVE_BUFFER_RECEIVED,
                                      htc_slave_recvdone,
                                      NULL);

    status = qapi_HTC_Event_Register (transId, 
                                      QAPI_HTC_SLAVE_BUFFER_SENT,
                                      htc_slave_senddone,
                                      NULL);

    status = qapi_HTC_Event_Register (transId, 
                                      QAPI_HTC_SLAVE_RESET_COMPLETE,
                                      htc_slave_resetdone,
                                      NULL);


    qapi_HTC_Slave_Start(transId);

    for (endpoint=0; endpoint < endpoint_used; endpoint++) {
        QCLI_Printf(qcli_htc_slave_hdl,"Start Mailbox Ping test on endpoint %d\n", endpoint);
        QCLI_Printf(qcli_htc_slave_hdl,"Buffers Per MBOX: %d\n", NUM_PING_BUFFERS_PER_ENDPOINT);
        QCLI_Printf(qcli_htc_slave_hdl,"Buffer Size: %d\n", PING_BUFSZ); 
        qapi_HTC_Slave_Max_MsgSize_Get(transId, (qapi_HTCSlaveEndpointID_t)endpoint, &msgsize);
        QCLI_Printf(qcli_htc_slave_hdl,"Max Message Size: %d\n", msgsize);        
        QCLI_Printf(qcli_htc_slave_hdl,"Max Message Size: %d\n", msgsize);
        QCLI_Printf(qcli_htc_slave_hdl,"Credits Per MBOX: %d\n", 
            (NUM_PING_BUFFERS_PER_ENDPOINT * PING_BUFSZ) / msgsize);
    }
    if(status){}    
    return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t htc_slave_demo_stop(uint32_t parameters_count,
                                     QCLI_Parameter_t * parameters)
{
    uint8 endpoint_used = 0;
    uint8 endpoint = 0;
    
    qapi_HTC_Slave_Get_Num_Endpoints(transId, &endpoint_used);
    
    qapi_HTC_Slave_Stop(transId);

    qapi_HTC_Event_Deregister (transId, 
                                      QAPI_HTC_SLAVE_INIT_COMPLETE);

    qapi_HTC_Event_Deregister (transId, 
                                      QAPI_HTC_SLAVE_BUFFER_RECEIVED);


    qapi_HTC_Event_Deregister (transId, 
                                      QAPI_HTC_SLAVE_BUFFER_SENT);


    qapi_HTC_Event_Deregister (transId, 
                                      QAPI_HTC_SLAVE_RESET_COMPLETE);

    
    qapi_HTC_Slave_Shutdown (transId);

    /* De-allocate htc_buf for each endpoint */
    for ( endpoint = 0; endpoint < endpoint_used; endpoint++)
    {
		htc_slave_demo_free_msg_buflist (transId, 
									htc_buf_list[endpoint]);
    }
    
    /* Freed successfully */
    return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t htc_slave_demo_pause(uint32_t parameters_count,
                                     QCLI_Parameter_t * parameters)
{
    uint8 endpoint = (uint8)parameters[0].Integer_Value;
    qapi_HTC_Slave_Recv_Pause(transId, (qapi_HTCSlaveEndpointID_t)endpoint);
    return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t htc_slave_demo_resume(uint32_t parameters_count,
                                     QCLI_Parameter_t * parameters)
{
    uint8 endpoint = (uint8)parameters[0].Integer_Value;
    qapi_HTC_Slave_Recv_Resume(transId, (qapi_HTCSlaveEndpointID_t)endpoint);
    return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t htc_slave_demo_inc_txcredit(uint32_t parameters_count,
                                     QCLI_Parameter_t * parameters)
{
    uint8 endpoint = (uint8)parameters[0].Integer_Value;
   /* Allow Host to send sizing information. */
    qapi_HTC_slave_tx_credit_inc(transId, endpoint);

    return QCLI_STATUS_SUCCESS_E;
}
QCLI_Command_Status_t htc_slave_demo_recycle_htcbuf(uint32_t parameters_count,
                                     QCLI_Parameter_t * parameters)
{
    qapi_HTC_bufinfo_t    *htc_buf;
    int num_out_bufs = 0;
    int endpoint = (int)parameters[0].Integer_Value;
    int num_bufs = (int)parameters[1].Integer_Value;

    /* Dequeue from recycle queue */
    htc_buf = htc_slave_dequeue_htcbuf(num_bufs, &num_out_bufs);
    if ( htc_buf != NULL) {
        /* Recycle buffers */
        qapi_HTC_Slave_Receive_Data(transId, (qapi_HTCSlaveEndpointID_t)endpoint, htc_buf);
    }

    return QCLI_STATUS_SUCCESS_E;
}
QCLI_Command_Status_t htc_slave_demo_dump_recv_queued_buffers(uint32_t parameters_count,
                                     QCLI_Parameter_t * parameters)
{
    int endpoint = (int)parameters[0].Integer_Value;
    
    qapi_HTC_slave_dump_recv_queued_buffers(transId, endpoint);    
    return QCLI_STATUS_SUCCESS_E;
}
QCLI_Command_Status_t htc_slave_demo_dump_send_queued_buffers(uint32_t parameters_count,
                                     QCLI_Parameter_t * parameters)
{
    int endpoint = (int)parameters[0].Integer_Value;
    
    qapi_HTC_slave_dump_send_queued_buffers(transId, endpoint);
    return QCLI_STATUS_SUCCESS_E;
}
QCLI_Command_Status_t htc_slave_demo_dump_recycle_queue_htcbuf(uint32_t parameters_count,
                                     QCLI_Parameter_t * parameters)
{
    int endpoint = (int)parameters[0].Integer_Value;
    
    if(HTC_buf_free_pool != NULL)
        debug_buffer_print(HTC_buf_free_pool, "recycle bufs pool", endpoint);
    else
        QCLI_Printf(qcli_htc_slave_hdl, "recycle queue empty\n");
    return QCLI_STATUS_SUCCESS_E;
}
