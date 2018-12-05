#ifndef __QAPI_HTC_SLAVE_H__
#define __QAPI_HTC_SLAVE_H__

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
 * @file  qapi_htc_slave.h
 * @brief HTC Slave QAPI APIs
*/

/*==================================================================================

                           EDIT HISTORY FOR FILE

This section contains comments describing changes made to this file.
Notice that changes are listed in reverse chronological order.

when       who     what, where, why
--------   ---     -----------------------------------------------------------------
01/25/18   leo     (Tech Comm) Edited/added Doxygen comments and markup.
10/3/17    leo     (Tech Comm) Edited/added Doxygen comments and markup.
07/26/17   mmtd    Initial version
==================================================================================*/

/** @addtogroup qapi_htc_slave
@{ */

/**
 * The maximum number of bytes to which the caller can set the actual_length
 * field when sending a message to the host, and the maximum size
 * that can be specified to HTC_endpoint_bufsz_set.
 */
#define QAPI_HTC_BUFSZ_MAX_SEND 2048


#define QAPI_NUM_BUFFERS_PER_ENDPOINT 8


#define QAPI_HTC_HDR_SZ 2

/**
 * Structure for use by HTC to track buffers, and for the caller and
 * HTC to communicate about buffers. The caller passes in a
 * pointer to one of these buffers on every send and every receive. There
 * must be a unique HTC_BUFFER for each buffer that is passed
 * in. HTC owns the HTC_BUFFER from the time it is passed in
 * until the time a send/receive completion is indicated, and the
 * caller must not modify it during that time.
 */

/*
 * Note: Application shall not have more than QAPI_NUM_BUFFERS_PER_ENDPOINT
 * instances qapi_HTC_bufinfo_t. But application can recycle these instances
 * once its done with its callback handling for transmission/reception.
 */
typedef struct qapi_HTC_bufinfo_s {
    struct qapi_HTC_bufinfo_s  *next;    /**< HTC_BUFFER list linkage field;
                                              used by HTC when HTC owns bufinfo,
                                              used by the caller when the caller
                                              owns bufinfo. */
    uint8                 *buffer;       /**< Pointer to the data buffer.
                                              Reserves headroom for HTC header
                                              at the start of message
                                              sent/received to/from Host.
                                              This should not be modified by HTC. */
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
    uint8               htc_flags;       /**< HTC flags; internal to HTC only */
    uint8               end_point;       /**< Endpoint to which this buffer
                                              belongs. */
    uint16              buffer_offset;   /**< Buffer offset reserves headroom
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
    uint16              app_context;     /**< Application context. */
} qapi_HTC_bufinfo_t;


/** HTC slave Endpoint hardware ID. */
typedef enum
{
    QAPI_HTC_SLAVE_ENDPOINT_ID0 = 0,  /**< Endpoint ID is 0. */
    QAPI_HTC_SLAVE_ENDPOINT_ID1 = 1,  /**< Endpoint ID is 1. */
    QAPI_HTC_SLAVE_ENDPOINT_ID2 = 2,  /**< Endpoint ID is 2. */
    QAPI_HTC_SLAVE_ENDPOINT_ID3 = 3,  /**< Endpoint ID is 3. */
    QAPI_HTC_SLAVE_MAX_ENDPOINTS    
} qapi_HTCSlaveEndpointID_t;


/* HTC Slave Event ID */
typedef enum
{
    QAPI_HTC_SLAVE_BUFFER_RECEIVED,
    QAPI_HTC_SLAVE_BUFFER_SENT,
    QAPI_HTC_SLAVE_INIT_COMPLETE,
    QAPI_HTC_SLAVE_RESET_COMPLETE,     
    QAPI_HTC_SLAVE_MAX_EVENTS
} qapi_HTCSlave_event_id_t;

/* HTC instance IDs */
typedef enum {
    QAPI_HTC_SDIO_SLAVE_INSTANCE_ID = 0,
    QAPI_HTC_SPI_SLAVE_INSTANCE_ID,
    QAPI_HTC_MAX_INSTANCES
} qapi_HTC_Instance_id_t;


typedef void (*qapi_HTC_callback_pfn_t)(uint32 Instance, uint32 arg1, uint32 arg2);



/*-------------------------------------------------------------------------
 * Function Declarations and Documentation
 * ----------------------------------------------------------------------*/
/**
* Initializes a slave. This function is called once before any other HTC slave calls are made.
* This is a common initialization function for all endpoints.
*
*  @param[in] Instance  Instance ID of the HTC slave.
*
*  @return
*  QAPI_OK                -- Call succeeded. \n
*  QAPI_ERROR             -- Call failed. \n
*  QAPI_ERR_INVALID_PARAM -- Invalid parameters were specified.
*
*/

qapi_Status_t qapi_HTC_Slave_Init (uint32 Instance);

/**
* Shutsdown the slave. This function is called once before any other HTC slave calls are made.
* This is a common shutdown function for all endpoints.
*
*  @param[in] Instance  Instance ID of the HTC slave.
*
*  @return
*  QAPI_OK                -- Call succeeded. \n
*  QAPI_ERROR             -- Call failed. \n
*  QAPI_ERR_INVALID_PARAM -- Invalid parameters were specified.
*
*/

qapi_Status_t qapi_HTC_Slave_Shutdown (uint32 Instance);


/**
* Starts the slave. This function is called once before any other HTC slave calls are made.
* This is a common initialization function for all Endpoint endpoints.
*
*  @param[in] Instance  Instance ID of the HTC slave.
*
*  @return
*  QAPI_OK                -- Call succeeded. \n
*  QAPI_ERROR             -- Call failed. \n
*  QAPI_ERR_INVALID_PARAM -- Invalid parameters were specified.
*
*/

qapi_Status_t qapi_HTC_Slave_Start (uint32 Instance);

/**
*  Stops a slave. This function disables all the Endpoint endpoints, frees all memory resources,
* interrupts and deinitializes the Endpoint hardware, and terminates the HTC connection.
*
*  @param[in] Instance  Instance ID of the HTC slave.
*  @return
*  QAPI_OK                -- Call succeeded. \n
*  QAPI_ERROR             -- Call failed. \n
*  QAPI_ERR_INVALID_PARAM -- Invalid parameters were specified.
*/

qapi_Status_t qapi_HTC_Slave_Stop (uint32 Instance);


/**
* Sends data from the target to the host.
*
* This function queues a chain of buffer descriptors that are to be sent to the host.
* All buffers and the buffer descriptor memory must be provided by the application.
* The driver does not have any internal buffers.
*
* The function sends a message over the specified mailbox.
* A message is specified as a NULL-terminated list of bufinfo entries.
* The caller's Send Done callback is invoked for each buffer of the message.
* HTC may split and aggregate notification for completed buffers, as long
* as each sent buffer is included in some buffer list sent to the Send
* Done callback.

*
*  @param[in] Instance  Instance id of HTC slave
*  @param[in] inst      Endpoint ID on which the transfer is to be done.
*  @param[in] buf       Pointer to the head of the buffer descriptor list
*                       that is to be sent on the specified Endpoint ID. \n
*                       See #qapi_HTC_bufinfo_t for a description of the structure.
* 
*
*  @return
*  QAPI_OK                -- Call succeeded. \n
*  QAPI_ERROR             -- Call failed. \n
*  QAPI_ERR_INVALID_PARAM -- Invalid parameters were specified.
*/
qapi_Status_t qapi_HTC_Slave_Send_Data(uint32 Instance, qapi_HTCSlaveEndpointID_t inst,
                                       qapi_HTC_bufinfo_t *buf);

/**
* Receives data from the host to the target over the specified mailbox.
*
* A message is specified as a NULL-terminated list of bufinfo entries.
* The caller's Receive Done callback is invoked for each buffer of the message.
* HTC may split and aggregate notification for completed buffers, as long
* as each sent buffer is included in some buffer list sent to the Receive
* Done callback.
*
* This function queues a chain of empty buffer descriptors that can receive the data from the host.
* All buffers and the buffer descriptor memory must be provided by the application.
* The driver does not have any internal buffers.
*
*
*  @param[in] Instance  Instance ID of the HTC slave.
*  @param[in] inst      Endpoint ID on which the transfer is to be done.
*  @param[in] buf       Pointer to the head of the buffer descriptor list
*                       that is used when the data from the host is received and, on completion,
*                       the corresponding callback function is called with the specified Endpoint ID. \n
*                       See #qapi_HTC_bufinfo_t for a description of the structure.
*
*  @return
*  QAPI_OK                -- Call succeeded. \n
*  QAPI_ERROR             -- Call failed. \n
*  QAPI_ERR_INVALID_PARAM -- Invalid parameters were specified.
*/
qapi_Status_t qapi_HTC_Slave_Receive_Data(uint32 Instance, qapi_HTCSlaveEndpointID_t inst,
                                       qapi_HTC_bufinfo_t *buf);

/**
* Sets the maximum message size that is sent/received.
* The message size = n*buffer size. It is greater than the buffer size.
*
*  @param[in] Instance   Instance ID of the HTC slave.
*  @param[in] inst       Endpoint ID on which the transfer is to be done.
*  @param[in] max_msgsz  Maximum message size.
*
*  @return
*  QAPI_OK                -- Call succeeded. \n
*  QAPI_ERROR             -- Call failed. \n
*  QAPI_ERR_INVALID_PARAM -- Invalid parameters were specified.
*/

qapi_Status_t qapi_HTC_Slave_Max_MsgSize_Set(uint32 Instance, qapi_HTCSlaveEndpointID_t inst,
                                        uint32 max_msgsz);

/**
 * Returns the maximum message size that is allowed to be received from the host
 * over a given mailbox. This value is negotiated with the host during
 * startup. The target side caller may use this during initialization in
 * order to choose an appropriate buffer size for each mailbox.
 *
 *  @param[in] Instance    Instance ID of the HTC slave.
 *  @param[in] inst        Endpoint ID on which the transfer is to be done.
 *  @param[out] max_msgsz  Get the maximum message size.
 *
 *  @return
 *  QAPI_OK                -- Call succeeded. \n
 *  QAPI_ERROR             -- Call failed. \n
 *  QAPI_ERR_INVALID_PARAM -- Invalid parameters were specified.
 */
qapi_Status_t qapi_HTC_Slave_Max_MsgSize_Get(uint32 Instance, qapi_HTCSlaveEndpointID_t inst,
                                        uint32 *max_msgsz);

/**
 * Sets the slave buffer sizes.
 *
 * During initialization, the caller must specify the size of the buffers
 * to be used for receiving on each mailbox. This should be done before
 * an attempt is made to receive (before qapi_HTC_Slave_Receive_Data() is called).
 *
 *  @param[in] Instance  Instance ID of the HTC slave.
 *  @param[in] inst      Endpoint ID on which the transfer is to be done. 
 *  @param[in] bufsz     Set the buffer size of the buffer supplied for receiving.
 *
 *  @return
 *  QAPI_OK                -- Call succeeded. \n
 *  QAPI_ERROR             -- Call failed. \n
 *  QAPI_ERR_INVALID_PARAM -- Invalid parameters were specified.
 */
qapi_Status_t qapi_HTC_Slave_BlockSize_Set(uint32 Instance, qapi_HTCSlaveEndpointID_t inst,
                                        uint32 bufsz);


/**
 * Gets the slave block size.
 *
 * During initialization, the HTC host negotiates the block sizes with target. 
 * This QAPI provides the negotiated size of block size.
 *
 *  @param[in] Instance  Instance Id of the HTC slave.
 *  @param[in] inst      Endpoint ID on which the transfer has to be done. 
 *  @param[in] *pbufsz   Get the block size of the buffer supplied for receiving.
 *
 *  @return
 *  QAPI_OK                -- Call succeeded. \n
 *  QAPI_ERROR             -- Call failed. \n
 *  QAPI_ERR_INVALID_PARAM -- Invalid parameters were specified.
 */
qapi_Status_t qapi_HTC_Slave_BlockSize_Get(uint32 Instance, qapi_HTCSlaveEndpointID_t inst,
                                        uint32* pbufsz);

/**
 * Pauses a receive (Rx) activity over a mailbox.
 *
 * While receives are paused, the target hardware may continue to
 * gather messages from the host, but they will not be reported to
 * the caller until the caller resumes the receive activity.
 *
 *  @param[in] Instance  Instance ID of the HTC slave.
 *  @param[in] inst      Endpoint ID on which the transfer is to be done.
 *
 *  @return
 *  QAPI_OK                -- Call succeeded. \n
 *  QAPI_ERROR             -- Call failed. \n
 *  QAPI_ERR_INVALID_PARAM -- Invalid parameters were specified.
 */
qapi_Status_t qapi_HTC_Slave_Recv_Pause (uint32 Instance, qapi_HTCSlaveEndpointID_t inst);

/**
 * Resumes receive (Rx) activity over a mailbox.
 *
 * While receives are paused, the target hardware may continue to
 * gather messages from the host, but they will not be reported to
 * the caller until the caller resumes the receive activity.
 *
 *  @param[in] Instance  Instance ID of the HTC slave.
 *  @param[in] inst      Endpoint ID on which the transfer is to be done.
 *
 *  @return
 *  QAPI_OK                -- Call succeeded. \n
 *  QAPI_ERROR             -- Call failed. \n
 *  QAPI_ERR_INVALID_PARAM -- Invalid parameters were specified.
 */
qapi_Status_t qapi_HTC_Slave_Recv_Resume (uint32 Instance, qapi_HTCSlaveEndpointID_t inst);

/**
 * Returns the maximum number of endpoints supported/enabled by the HTC layer.
 *
 *  @param[in] Instance       Instance ID of the HTC slave.
 *  @param[out] pNumEndpoint  Pointer to the number of endpoints supported. The application
 *                            should use this API to determine the number of endpoints and
 *                            use it for IO.
 *
 *  @return
 *  QAPI_OK                -- Call succeeded. \n
 *  QAPI_ERROR             -- Call failed. \n
 *  QAPI_ERR_INVALID_PARAM -- Invalid parameters were specified.
 */
qapi_Status_t qapi_HTC_Slave_Get_Num_Endpoints(uint32 Instance, uint8 *pNumEndpoint);


/**
 * Registers the application's handler for HTC events.
 * This is common for all endpoints. The endpoint ID must be passed as arg.
 *
 *  @param[in] Instance       Instance ID of the HTC slave.
 *  @param[in] eventId        Event ID for which the handler is registered.
 *  @param[in] eventHandler   Application event handler ID pointer.
 *  @param[in] param          Parameter that passed the event handler.
 *
 *  @return
 *  QAPI_OK                -- Call succeeded. \n
 *  QAPI_ERROR             -- Call failed. \n
 *  QAPI_ERR_INVALID_PARAM -- Invalid parameters were specified.
 */

qapi_Status_t qapi_HTC_Event_Register (uint32 Instance, 
                                       qapi_HTCSlave_event_id_t eventId,
                                       qapi_HTC_callback_pfn_t eventHandler,
                                       void *param);
/**
 * Deregisters the application's handler for HTC events.
 * This is common for all endpoints. The endpoint ID must be passed as arg.
 *
 *  @param[in] Instance       Instance ID of the HTC slave.
 *  @param[in] eventId        Event ID for which the handler is registered.
 *
 *  @return
 *  QAPI_OK                -- Call succeeded. \n
 *  QAPI_ERROR             -- Call failed. \n
 *  QAPI_ERR_INVALID_PARAM -- Invalid parameters were specified.
 */

qapi_Status_t qapi_HTC_Event_Deregister (uint32 Instance,
                                         qapi_HTCSlave_event_id_t eventId);


/**
 * Gets the number of interfaces on which the HTC slave can be made to communicate with the host.
 *
 *  @param[out] pNumInterfaces   Number of interfaces.
 *
 *  @return
 *  QAPI_OK                -- Call succeeded. \n
 *  QAPI_ERROR             -- Call failed. \n
 *  QAPI_ERR_INVALID_PARAM -- Invalid parameters were specified.
 */

qapi_Status_t qapi_HTC_slave_get_num_interfaces (uint32 *pNumInterfaces);


/**
 * Gets the interface IDs of the interfaces on which the HTC slave can be made to communicate with the host.
 *
 *  @param[out] Interface_array     Array of interface numbers which is returned by this QAPI.
 *  @param[in] pNumInterfaces       Number of interfaces.
 *
 *  @return
 *  QAPI_OK                -- Call succeeded. \n
 *  QAPI_ERROR             -- Call failed. \n
 *  QAPI_ERR_INVALID_PARAM -- Invalid parameters were specified.
 */

qapi_Status_t qapi_HTC_slave_get_interface_ids ( uint32 *Interface_array,  uint32 pNumInterfaces);



/**
 * Dumps the list of queued buffers for receiving data from the host.
 * This is used to mainly for debuggging.
 *
 *  @param[in] InstanceId  Instance ID of the HTC slave.
 *  @param[in] endpoint    Endpoint ID.
 *
 *  @return
 *  QAPI_OK                -- Call succeeded. \n
 *  QAPI_ERROR             -- Call failed. \n
 *  QAPI_ERR_INVALID_PARAM -- Invalid parameters were specified.
 */

qapi_Status_t qapi_HTC_slave_dump_recv_queued_buffers(uint32 InstanceId,
                                                                              qapi_HTCSlaveEndpointID_t endpoint);


/**
 * Dumps the list of queued buffers for sending data to the host.
 * This is used to mainly for debuggging.
 *
 *  @param[in] InstanceId  Instance ID of the HTC slave.
 *  @param[in] endpoint    Endpoint ID.
 *
 *  @return
 *  QAPI_OK                -- Call succeeded. \n
 *  QAPI_ERROR             -- Call failed. \n
 *  QAPI_ERR_INVALID_PARAM -- Invalid parameters were specified.
 */

qapi_Status_t qapi_HTC_slave_dump_send_queued_buffers(uint32 InstanceId,
                                                                                qapi_HTCSlaveEndpointID_t endpoint);

 /**
 * Increments the credits for TX operation.
 * This is used to mainly for debuggging.
 *
 *  @param[in] InstanceId  Instance ID of the HTC slave.
 *  @param[in] endpoint    Endpoint ID.
 *
 *  @return
 *  QAPI_OK                -- Call succeeded. \n
 *  QAPI_ERROR             -- Call failed. \n
 *  QAPI_ERR_INVALID_PARAM -- Invalid parameters were specified.
 */

qapi_Status_t qapi_HTC_slave_tx_credit_inc (uint32 InstanceId, qapi_HTCSlaveEndpointID_t endpoint);

#endif /* __QAPI_HTC_SLAVE_H__ */

/** @} */ /* end_addtogroup qapi_htc_slave */
