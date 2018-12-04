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

/********************************************************* 
 *
 * HTC internal specific declarations and prototypes
 *
 ********************************************************/
 
#ifndef _HTC_INTERNAL_H_
#define _HTC_INTERNAL_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/* HTC operational parameters */
#define HTC_GLOBAL_EVENT_NUM_MAX           2 /* Target available/unavailable */
#define HTC_ENDPOINT_EVENT_NUM_MAX         5 /* Endpoint specific */
#define HTC_REG_REQUEST_LIST_SIZE          16
#define HTC_MESSAGE_SIZE_MAX               0x800 /* Default maximum message size for each mailbox */
#define HTC_TX_CREDITS_NUM_MAX             64
#define HTC_TARGET_RESPONSE_TIMEOUT        2000 /* in ms */
#define HTC_TARGET_DEBUG_INTR_MASK         0x01
#define HTC_TARGET_CREDIT_INTR_MASK        0xF0

#define HTC_BLK_SIZE_NEG_COMPLETE_EVT     0x01

/* Useful macros */
#define GET_ENDPOINT_ID(endPoint) (endPoint - endPoint->target->endPoint)

/* ------- Event Related Data Structures ------- */
typedef struct htc_event_map HTC_EVENT_MAP;
typedef struct event_table_element EVENT_TABLE_ELEMENT;
typedef struct htc_endpoint_event_table HTC_ENDPOINT_EVENT_TABLE;
typedef struct htc_global_event_table HTC_GLOBAL_EVENT_TABLE;

#define FRAME_EVENT(_eventInfo, _buffer, _bufferLength,   \
                    _actualLength, _status, _cookie) do { \
    _eventInfo.buffer  = _buffer;                         \
    _eventInfo.bufferLength = _bufferLength;              \
    _eventInfo.actualLength = _actualLength;              \
    _eventInfo.status = _status;                          \
    _eventInfo.cookie = _cookie;                          \
} while (0)

struct event_table_element {
    HTC_EVENT_ID         id;
    HTC_EVENT_HANDLER    handler;
    void                *param;
};

struct htc_endpoint_event_table {
    EVENT_TABLE_ELEMENT element[HTC_ENDPOINT_EVENT_NUM_MAX];
};

struct htc_global_event_table {
    EVENT_TABLE_ELEMENT element[HTC_GLOBAL_EVENT_NUM_MAX];

};

/* ------ Mailbox Related Data Structures ------ */
typedef struct htc_queue_element HTC_QUEUE_ELEMENT, HTC_REG_REQUEST_ELEMENT, HTC_DATA_REQUEST_ELEMENT;
typedef struct htc_endpoint_buffer HTC_ENDPOINT_BUFFER;
typedef struct htc_reg_buffer HTC_REG_BUFFER;
typedef struct htc_data_request_queue HTC_DATA_REQUEST_QUEUE;
typedef struct htc_reg_request_list HTC_REG_REQUEST_LIST;
typedef struct htc_endpoint HTC_ENDPOINT;



typedef enum {
    INT_STATUS_REG,
    ERROR_INT_STATUS_REG,
    CPU_INT_STATUS_REG,
    RX_LOOKAHEAD_VALID_REG,
    RX_LOOKAHEAD0_REG,
    RX_LOOKAHEAD1_REG,
    RX_LOOKAHEAD2_REG,
    RX_LOOKAHEAD3_REG,
    TX_CREDIT_COUNTER_REG,
    TX_CREDIT_COUNTER_RESET_REG,
    TX_CREDIT_COUNTER_DECREMENT_REG,
    TX_DEBUG_COUNTER_REG,
    SCRATCH_REG,
    INT_STATUS_ENABLE_REG,
    CPU_INT_STATUS_ENABLE_REG,
    ERROR_STATUS_ENABLE_REG,
    COUNTER_INT_STATUS_READ_REG,
    COUNTER_INT_STATUS_ENABLE_REG,
    COUNTER_INT_STATUS_DISABLE_REG,
    INT_WLAN_REG,
    WINDOW_DATA_REG,
    WINDOW_WRITE_ADDR_REG,
    WINDOW_READ_ADDR_REG,
    SPI_STATUS_READ_REG,
    SPI_STATUS_WRITE_REG,
    DEBUG_COUNTER_REG
} TARGET_REGISTERS;

typedef enum htc_scratch_type
{
	HTC_SCRATCH_ENDPOINT_ID=0,
	HTC_SCRATCH_TARGET_STATE,
    HTC_UNUSED_2,
    HTC_UNUSED_3,
    HTC_UNUSED_4,
    HTC_UNUSED_5,
    HTC_UNUSED_6,
    HTC_UNUSED_7
} htc_scratch_type_t;

typedef enum htc_host_int
{
	HTC_INT_ENDPOINT_0=0,
	HTC_INT_ENDPOINT_1,
    HTC_INT_ENDPOINT_2,
    HTC_INT_ENDPOINT_3,
    HTC_INT_TARGET_SHUTDOWN_HOST_ACK,
    HTC_INT_TARGET_RESET_HOST_REQ,
    HTC_INT_UNUSED_6,
    HTC_INT_UNUSED_7
} htc_host_int_t;

#define SET_TX_CREDITS_AVAILABLE(endPoint, credits) \
                                    endPoint->txCreditsAvailable[0] = (credits)
#define SET_TX_CREDITS_CONSUMED(endPoint, credits) \
                                    endPoint->txCreditsConsumed = (credits)
#define GET_TX_CREDITS_AVAILABLE(endPoint) \
                                    endPoint->txCreditsAvailable[0]
#define GET_TX_CREDITS_CONSUMED(endPoint) \
                                    endPoint->txCreditsConsumed

#define IS_ELEMENT_FREE(element)	element->buffer.free
#define GET_ENDPOINT_BUFFER(element)	&((element)->buffer.u.endpointBuffer)
#define GET_REG_BUFFER(element)		&((element)->buffer.u.regBuffer)
#define GET_QUEUE_TAIL(queue)		&queue->element[(queue->head + queue->size) % HTC_DATA_REQUEST_RING_BUFFER_SIZE]
#define GET_QUEUE_HEAD(queue)		&queue->element[queue->head]
#define IS_DATA_QUEUE_EMPTY(queue)      (!queue->size)
#define IS_DATA_QUEUE_FULL(queue)       (!(HTC_DATA_REQUEST_RING_BUFFER_SIZE - queue->size))

#define RECYCLE_DATA_REQUEST_ELEMENT(element) do { \
    FILL_ENDPOINT_BUFFER(element, NULL, 0, 0, NULL); \
    (element)->buffer.free = TRUE; \
} while (0)

#define FILL_ENDPOINT_BUFFER(element, _buffer, _bufferLength, _actualLength, _cookie) do { \
    (GET_ENDPOINT_BUFFER(element))->buffer = _buffer; \
    (GET_ENDPOINT_BUFFER(element))->bufferLength = _bufferLength; \
    (GET_ENDPOINT_BUFFER(element))->actualLength = _actualLength; \
    (GET_ENDPOINT_BUFFER(element))->cookie = _cookie; \
} while (0)

#define FILL_REG_BUFFER(element, _buffer, _length, _base, _offset) do { \
    (GET_REG_BUFFER(element))->buffer = _buffer; \
    (GET_REG_BUFFER(element))->length = _length; \
    (GET_REG_BUFFER(element))->base = _base; \
    (GET_REG_BUFFER(element))->offset = _offset; \
} while (0)

struct htc_queue_element {
    htc_status_t	(*completionCB)(HTC_QUEUE_ELEMENT *element, htc_status_t status);
    struct htc_buffer {
        /* In use or available */
        OSAL_BOOL	free;
        union {
            struct htc_endpoint_buffer {
                /* 
                 * Given by the caller and is associated with the buffer being 
                 * queued up.
                 */
                void			*cookie;

                /* 
                 * Pointer to the start of the buffer. In the transmit 
                 * direction this points to the start of the payload. In the 
                 * receive direction, however, the buffer when queued up 
                 * points to the start of the HTC header but when returned 
                 * to the caller points to the start of the payload 
                 */
                OSAL_UINT8			*buffer;

                /* Holds the length of the buffer */
                OSAL_UINT32		bufferLength;

                /* Holds the length of the payload */
                OSAL_UINT32		actualLength;

                HTC_ENDPOINT    *endPoint;
            } endpointBuffer;
            struct htc_reg_buffer {
                HTC_TARGET		*target;
                OSAL_UINT8			*buffer;
                OSAL_UINT32		length;
                TARGET_REGISTERS	base;
                OSAL_UINT32		offset;
            } regBuffer;
        } u;
    } buffer;
};

/* This is a FIFO queue of the pending data read/write requests. When a request
has to be issued, the element at the head of the queue is dequeued and
processed. New requests are added at the tail of the queue. The queue can only
support a fixed number of requests and stops adding new requests once the total
number of requests that are pending to be processed and the ones that are still
under process reach the queue capacity */
struct htc_data_request_queue {
    OSAL_UINT32                  head;
    OSAL_UINT32                  size;
    HTC_DATA_REQUEST_ELEMENT  element[HTC_DATA_REQUEST_RING_BUFFER_SIZE];
};

/* This is a list of 'free' register read/write requests. When a request has to 
be issued an element is taken from this list and after the completion of the 
request is added back to the list */
struct htc_reg_request_list {
    HTC_REG_REQUEST_ELEMENT  element[HTC_REG_REQUEST_LIST_SIZE];
};

struct htc_endpoint {
    /* Enabled or Disabled */
    OSAL_BOOL                   enabled;

    /*
     * Used to hold the length of the frame received from the target in 
     * case there are no buffers that have been queued up to receive the 
     * data.
     */
    OSAL_UINT32                 rxLengthPending; 

    /* Number of frames for which the target has space for at any time */
    OSAL_UINT8                  txCreditsAvailable[1 + HTC_TX_CREDITS_NUM_MAX];

    /* 
     * Number of frames that have been sent since the transmit credits 
     * were last updated.
     */
    OSAL_UINT8                  txCreditsConsumed; 

    OSAL_BOOL                   txCreditsIntrEnable;

    /* Pending Send Queue */
    HTC_DATA_REQUEST_QUEUE   sendQueue; 

    /* Pending Receive Queue */
    HTC_DATA_REQUEST_QUEUE   recvQueue; 

    /* Inverse reference to the target */
    HTC_TARGET              *target;

    /* Block size configured for the endpoint */
    OSAL_UINT32                 blockSize;

    /* Event Table */
    HTC_ENDPOINT_EVENT_TABLE eventTable; 

    /* Stating address of the endpoint */
    OSAL_UINT32                 address;
};

/* ------- Target Related Data structures ------- */
typedef struct htc_register_table HTC_REGISTER_TABLE;

/* 
 * The following Register table only contain those registers that are used 
 * in HTC. It does not reflect the actual register layout in the hardware 
 */
struct htc_register_table {
    OSAL_UINT8                      host_int_status;
    OSAL_UINT8                      cpu_int_status;
    OSAL_UINT8                      error_int_status;
    OSAL_UINT8                      counter_int_status;
    OSAL_UINT8                      endpoint_frame;
    OSAL_UINT8                      rx_lookahead_valid;
    OSAL_UINT8                      hole[2];
    OSAL_UINT32                     rx_lookahead[HTC_MAILBOX_NUM_MAX];
    OSAL_UINT8                      int_status_enable;
    OSAL_UINT8                      cpu_int_status_enable;
    OSAL_UINT8                      error_status_enable;
    OSAL_UINT8                      counter_int_status_enable;
    OSAL_UINT8                      int_wlan;
    OSAL_UINT32                     spi_status;
    OSAL_UINT8                      scratch[8];    
};

typedef enum htc_target_state
{
	HTC_TARGET_INITIALIZED = 1,
	HTC_TARGET_SHUTDOWN,
	HTC_TARGET_STARTED,
	HTC_TARGET_STOPPED,
	HTC_TARGET_SHUTDOWN_READY,
} htc_target_state_t;

struct htc_target {
    OSAL_BOOL                       ready;
    void                        *device; /* Handle to the device instance 
                                            reported by the bus driver */
    HTC_ENDPOINT                 endPoint[HTC_MAILBOX_NUM_MAX];
    HTC_REGISTER_TABLE           table;
    HTC_REG_REQUEST_LIST         regList;
	
	qurt_signal_t  				 htcEvent; /* Blk negotiation completion event */
	OSAL_MUTEX_T instanceCS, counterCS, creditCS;
	OSAL_BOOL                  	hotplug;

	htc_target_state_t          target_state;
};

struct htc_callbacks {
    void      *context;     /* context to pass to the dsrhandler
                               note : rwCompletionHandler is provided the context passed to HIFReadWrite  */
    int (* rwCompletionHandler)(void *rwContext, int status);
    int (* dsrHandler)(void *context);
};

typedef struct htc_callbacks HTC_CALLBACKS;

/* ------- Function Prototypes for Receive -------- */
htc_status_t
htcReceiveFrame(HTC_ENDPOINT *endPoint);

OSAL_UINT32
htcGetFrameLength(HTC_ENDPOINT *endPoint);


/* ------- Function Prototypes for Transmit -------- */
void 
htcSendFrame(HTC_ENDPOINT *endPoint);

void
htcSendBlkSize(HTC_ENDPOINT *endPoint);


/* ------- Function Prototypes for Events and Callbacks  ------- */
htc_status_t
htcRWCompletionHandler(void *element, 
                       htc_status_t status);

htc_status_t 
htcTxCompletionCB(HTC_DATA_REQUEST_ELEMENT *element,
                  htc_status_t status);

htc_status_t
htcBlkSzNegCompletionCB(HTC_DATA_REQUEST_ELEMENT *element,
                        htc_status_t status);

htc_status_t
htcRxCompletionCB(HTC_DATA_REQUEST_ELEMENT *element, 
                  htc_status_t status);

htc_status_t
htcRegCompletionCB(HTC_REG_REQUEST_ELEMENT *element,
                   htc_status_t status);

htc_status_t
htcTargetInsertedHandler(void *context, void *hif_handle, OSAL_BOOL hotplug);

htc_status_t
htcTargetRemovedHandler(void *context, void *htc_handle, OSAL_BOOL hotplug);

htc_status_t
htcDSRHandler(void *htc_handle);

void
htcServiceCPUInterrupt(HTC_TARGET *target);

void
htcServiceErrorInterrupt(HTC_TARGET *target);

void
htcServiceCounterInterrupt(HTC_TARGET *target);

void
htcServiceDebugInterrupt(HTC_TARGET *target);

void
htcServiceCreditInterrupt(HTC_TARGET *target);

void
htcServiceMailboxInterrupt(HTC_TARGET *target);

void
htcEnableCreditCounterInterrupt(HTC_TARGET *target, 
                                HTC_ENDPOINT_ID endPointId);

void
htcDisableCreditCounterInterrupt(HTC_TARGET *target, 
                                 HTC_ENDPOINT_ID endPointId);

/* ------- Function Prototypes for Utility routines ------- */
void
printEndpointQueueElement(HTC_DATA_REQUEST_ELEMENT *element);

void
printEndpointQueue(HTC_DATA_REQUEST_QUEUE *queue);

htc_status_t
addToEndpointQueue(HTC_TARGET *target, HTC_DATA_REQUEST_QUEUE *queue,
               OSAL_UINT8        *buffer,
               OSAL_UINT32        bufferLength,
               OSAL_UINT32        actualLength,
               void           *cookie);

HTC_DATA_REQUEST_ELEMENT *
removeFromEndpointQueue(HTC_TARGET *target, HTC_DATA_REQUEST_QUEUE *queue);

void
flushEndpointQueue(HTC_ENDPOINT *endPoint,
               HTC_DATA_REQUEST_QUEUE *queue, 
               HTC_EVENT_ID eventId);

HTC_REG_REQUEST_ELEMENT *
allocateRegRequestElement(HTC_TARGET *target);

void
freeRegRequestElement(HTC_REG_REQUEST_ELEMENT *element);

htc_status_t 
addToEventTable(HTC_TARGET       *target,
                HTC_ENDPOINT_ID   endPointId,
                HTC_EVENT_ID      eventId,
                HTC_EVENT_HANDLER handler, 
                void             *param);

htc_status_t 
removeFromEventTable(HTC_TARGET *target,
                     HTC_ENDPOINT_ID endPointId,
                     HTC_EVENT_ID  eventId);

void
dispatchEvent(HTC_TARGET     *target, 
              HTC_ENDPOINT_ID endPointId,
              HTC_EVENT_ID    eventId, 
              HTC_EVENT_INFO *eventInfo);

void
htcReportFailure(htc_status_t status);



htc_status_t addTargetInstance (HTC_TARGET *target);


htc_status_t delTargetInstance (HTC_TARGET *target);


OSAL_UINT32 
getRegAddr(TARGET_REGISTERS base, 
           HTC_ENDPOINT_ID endPointId);

OSAL_UINT8
htcGetBitNumSet(OSAL_UINT32 data);

void
dumpBytes(OSAL_UINT8 *buffer, OSAL_UINT16 length);

void
dumpRegisters(HTC_TARGET *target);

htc_status_t getTargetTransport (HTC_TARGET *target, HTC_TARGET_TRANSPORT *trans);

#ifdef __cplusplus
}
#endif

#endif /* _HTC_INTERNAL_H_ */
