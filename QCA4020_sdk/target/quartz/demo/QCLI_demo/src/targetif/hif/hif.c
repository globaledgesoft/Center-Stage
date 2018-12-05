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

 
 /* Header files */
#include "qapi_types.h"
#include "com_dtypes.h"
#include "osal_types.h"
#include "htc_defs.h"
#include "htc.h"
#include "transport.h"
#include "htc_internal.h"
#include "dl_list.h"
#include "hif.h"
#include "hif_internal.h"

/* Prototypes */
static htc_status_t hifDeviceInserted(void  *tdev, OSAL_BOOL hotplug );
static htc_status_t hifDeviceRemoved(void *dev, OSAL_BOOL hotplug);
static HIF_DEVICE *addHifDevice(trans_device_t *tdev);
static HIF_DEVICE *getHifDevice(trans_device_t *tdev);
static void delHifDevice(HIF_DEVICE * device);
static void ResetAllCards(void);
static htc_status_t hifDisableFunc(HIF_DEVICE *device, trans_device_t *tdev);
static htc_status_t hifEnableFunc(HIF_DEVICE *device, trans_device_t *tdev, OSAL_BOOL hotplug);

OSDRV_CALLBACKS osdrvCallbacks;
static HIF_DEVICE *hif_devices[HIF_MAX_DEVICES];

/* Move it to header file */
#define HIF_ASYNC_TASK_NAME         "HIF_AsyncTask"
#define HIF_ASYNC_TASK_PRIO         10             // txe_thread_create errs w/ TX_PRIORITY_ERROR if >= TX_MAX_PRIORITIES(32)
#define HIF_ASYNC_TASK_STK_SIZE     1024           // TX_MINIMUM_STACK = 200

#define HIF_ASYNC_TASK_TERM_EVT     0x01
#define HIF_ASYNC_TASK_IO_EVT       0x02

/* Async task kill event */
#define HIF_ASYNC_TASK_COMPLETION_EVT   0x04

/* Bus request signal */
#define HIF_BUS_REQUEST_COMPLETE_EVT       0x08

#define HIF_ASYNC_TASK_STARTED      0x10


#ifdef TX_COMPLETION_THREAD

#define HIF_TX_COMPLETION_TASK_NAME         "HIF TxCompletion"
#define HIF_TX_COMPLETION_TASK_PRIO         10             
#define HIF_TX_COMPLETION_TASK_STK_SIZE     1024       

/* Event for Tx thread to complete request */
#define HIF_TX_TASK_COMPLETION_REQ_EVT       0x01

#define HIF_TX_TASK_EXIT_EVT       			 0x02

/* Confirmation event */
#define HIF_TX_TASK_EXIT_CONFIRMATION_EVT    0x01
#endif


/* ------ Functions ------ */
htc_status_t HIFInit(OSDRV_CALLBACKS *callbacks)
{
    int status;
	hif_callbacks_t hif_callbacks;

    if (callbacks == NULL)
    {
        return HTC_ERROR;
    }

	osdrvCallbacks = *callbacks;
	
	hif_callbacks.hif_trans_device_inserted = hifDeviceInserted;
	hif_callbacks.hif_trans_device_removed = hifDeviceRemoved,

    status = trans_init(&hif_callbacks);

    if (status != 0)
	{
        HTC_DEBUG_PRINTF(HTC_DEBUG_ERROR,("%s transport driver registration failed!",__func__));
        return HTC_ERROR;
    }
    else
    {
        HTC_DEBUG_PRINTF(HTC_DEBUG_TRACE,("%s transport driver registration successful",__func__));
    }

    return HTC_OK;
}

void AddToAsyncList(HIF_DEVICE *device, BUS_REQUEST *busrequest)
{
    BUS_REQUEST *async;
    BUS_REQUEST *active;

    OSAL_MUTEX_LOCK(&device->asynclock);
    active = device->asyncreq;
    if (active == NULL) {
        device->asyncreq = busrequest;
        device->asyncreq->inusenext = NULL;
    } else {
        for (async = device->asyncreq;
             async != NULL;
             async = async->inusenext) {
             active =  async;
        }
        active->inusenext = busrequest;
        busrequest->inusenext = NULL;
    }
    OSAL_MUTEX_UNLOCK(&device->asynclock);
}


static htc_status_t
__HIFReadWrite(HIF_DEVICE *device,
             OSAL_UINT32 address,
             OSAL_UINT8 *buffer,
             OSAL_UINT32 length,
             OSAL_UINT32 request,
             void *context)
{
    htc_status_t    status = HTC_OK;
    int ret = 0;
    OSAL_UINT8 *tbuffer;

    if (device == NULL || device->tdev == NULL)
        return HTC_ERROR;
    HTC_DEBUG_PRINTF(HTC_DEBUG_TRACE, ("__HIFReadWrite, addr:0X%06X, len:%08d, %s, %s\n",
                    address,
                    length,
                    request & HIF_READ ? "Read " : "Write",
                    request & HIF_ASYNCHRONOUS ? "Async" : "Sync "));

    HTC_DEBUG_PRINTBUF(buffer, length); 

    do {
        if (request & HIF_EXTENDED_IO) {
            //HTC_DEBUG_PRINTF(HTC_DEBUG_TRACE, ("HTC/HIF: Command type: CMD53\n"));
        } else {
            HTC_DEBUG_PRINTF(HTC_DEBUG_ERROR,
                            ("HTC/HIF: Invalid command type: 0x%08x\n", request));
            status = HTC_EINVAL;
            break;
        }

        if (request & HIF_BLOCK_BASIS) {
            /* round to whole block length size */
            length = (length / HIF_ENDPOINT_BLOCK_SIZE) * HIF_ENDPOINT_BLOCK_SIZE;
            HTC_DEBUG_PRINTF(HTC_DEBUG_TRACE,
                            ("HTC/HIF: Block mode (BlockLen: %d)\n",
                            length));
        } else if (request & HIF_BYTE_BASIS) {
            HTC_DEBUG_PRINTF(HTC_DEBUG_TRACE,
                            ("HTC/HIF: Byte mode (BlockLen: %d)\n",
                            length));
        } else {
            HTC_DEBUG_PRINTF(HTC_DEBUG_ERROR,
                            ("HTC/HIF: Invalid data mode: 0x%08x\n", request));
            status = HTC_EINVAL;
            break;
        }


        if (request & HIF_FIXED_ADDRESS) {
            HTC_DEBUG_PRINTF(HTC_DEBUG_TRACE, ("HTC/HIF: Address mode: Fixed 0x%X\n", address));
        } else if (request & HIF_INCREMENTAL_ADDRESS) {
            HTC_DEBUG_PRINTF(HTC_DEBUG_TRACE, ("HTC/HIF: Address mode: Incremental 0x%X\n", address));
        } else {
            HTC_DEBUG_PRINTF(HTC_DEBUG_ERROR,
                            ("HTC/HIF: Invalid address mode: 0x%08x\n", request));
            status = HTC_EINVAL;
            break;
        }

	    if ((address >= HIF_ENDPOINT_START_ADDR(0)) && 
	        (address <= HIF_ENDPOINT_END_ADDR(3)))
	    {
	        /* 
	         * Mailbox write. Adjust the address so that the last byte 
	         * falls on the EOM address.
	         */
	        address += (HIF_ENDPOINT_WIDTH - length);
	    }

        if (request & HIF_WRITE) {
            tbuffer = buffer;

            if (tbuffer != NULL) {
                if (request & HIF_FIXED_ADDRESS) {
					ret = trans_write_data(device->tdev, FIFO_MODE, address, tbuffer, length);
                    HTC_DEBUG_PRINTF(HTC_DEBUG_TRACE,
                              ("HTC/HIF: writesb ret=%d address: 0x%X, len: %d, 0x%X\n",
                              ret, address, length, *(int *)tbuffer));
                } else {
					
					ret = trans_write_data(device->tdev, MEMORY_MODE, address, tbuffer, length);

                    HTC_DEBUG_PRINTF(HTC_DEBUG_TRACE,
                              ("HTC/HIF: writeio ret=%d address: 0x%X, len: %d, 0x%X\n",
                              ret, address, length, *(int *)tbuffer));
                }
            } else {
                HTC_DEBUG_PRINTF(HTC_DEBUG_ERROR, ("HTC/HIF: tbuffer is NULL"));
                status = HTC_ERROR;
                break;
            }
        } else if (request & HIF_READ) {
            tbuffer = buffer;
            if (tbuffer != NULL) {
                if (request & HIF_FIXED_ADDRESS) {
					ret = trans_read_data(device->tdev, FIFO_MODE, address, tbuffer, length);

                    HTC_DEBUG_PRINTF(HTC_DEBUG_TRACE,
                              ("HTC/HIF: readsb ret=%d address: 0x%X, len: %d, 0x%X\n",
                              ret, address, length, *(int *)tbuffer));
                } else {
					ret = trans_read_data(device->tdev, MEMORY_MODE, address, tbuffer, length);
                    HTC_DEBUG_PRINTF(HTC_DEBUG_TRACE,
                              ("HTC/HIF: readio ret=%d address: 0x%X, len: %d, 0x%X\n",
                              ret, address, length, *(int *)tbuffer));
                }
            } else {
                HTC_DEBUG_PRINTF(HTC_DEBUG_ERROR, ("HTC/HIF: tbuffer is NULL"));
                status = HTC_ERROR;
                break;
            }
        } else {
            HTC_DEBUG_PRINTF(HTC_DEBUG_ERROR,
                            ("HTC/HIF: Invalid direction: 0x%08x\n", request));
            status = HTC_EINVAL;
            break;
        }

        if (ret) {
            HTC_DEBUG_PRINTF(HTC_DEBUG_ERROR,
                            ("HTC/HIF: Transport driver IO  operation failed!  : %d \n", ret));
            HTC_DEBUG_PRINTF(HTC_DEBUG_ERROR, ("__HIFReadWrite, addr:0X%06X, len:%08d, %s, %s\n",
                            address,
                            length,
                            request & HIF_READ ? "Read " : "Write",
                            request & HIF_ASYNCHRONOUS ? "Async" : "Sync "));
            status = HTC_ERROR;
        }
    } while (FALSE);

    return status;
}


htc_status_t
HIFSyncRead(HIF_DEVICE *device,
               OSAL_UINT32 address,
               OSAL_UINT8 *buffer,
               OSAL_UINT32 length,
               OSAL_UINT32 request,
               void *context)
{
       htc_status_t status;

       if (device == NULL || device->tdev == NULL)
           return HTC_ERROR;
	   
       status = __HIFReadWrite(device, address, buffer, length, request & ~HIF_SYNCHRONOUS, NULL);

       return status;
}

/* queue a read/write request */
htc_status_t
HIFReadWrite(HIF_DEVICE *device,
             OSAL_UINT32 address,
             OSAL_UINT8 *buffer,
             OSAL_UINT32 length,
             OSAL_UINT32 request,
             void *context)
{
    htc_status_t    status = HTC_OK;
    BUS_REQUEST *busrequest;

    if (device == NULL || device->tdev == NULL)
        return HTC_ERROR;
    HTC_DEBUG_PRINTF(HTC_DEBUG_TRACE,
        ("HTC/HIF: device 0x%p addr 0x%X buffer 0x%p len %d req 0x%X context 0x%p",
        device, address, buffer, length, request, context));

    /* sdio r/w action is not needed when suspend with cut power,so just return*/
    if((device->is_suspend == TRUE)&&(device->powerConfig == HIF_DEVICE_POWER_CUT)){
        HTC_DEBUG_PRINTF(HTC_DEBUG_TRACE, ("skip io when suspending\n"));
        return HTC_OK;
    }

    do {
        if ((request & HIF_ASYNCHRONOUS) || (request & HIF_SYNCHRONOUS)){
            /* serialize all requests through the async thread */
            HTC_DEBUG_PRINTF(HTC_DEBUG_TRACE, ("HTC/HIF: Execution mode: %s\n",
                        (request & HIF_ASYNCHRONOUS)?"Async":"Synch"));
            busrequest = hifAllocateBusRequest(device);
            if (busrequest == NULL) {
                HTC_DEBUG_PRINTF(HTC_DEBUG_ERROR,
                    ("HTC/HIF: no async bus requests available (%s, addr:0x%X, len:%d) \n",
                        request & HIF_READ ? "READ":"WRITE", address, length));
                return HTC_ERROR;
            }
            busrequest->address = address;
            busrequest->buffer = buffer;
            busrequest->length = length;
            busrequest->request = request;
            busrequest->context = context;

            AddToAsyncList(device, busrequest);

            if (request & HIF_SYNCHRONOUS) {
                HTC_DEBUG_PRINTF(HTC_DEBUG_TRACE, ("HTC/HIF: queued sync req: 0x%lX\n", (unsigned long)busrequest));

                /* wait for completion */
				/* Send IO request signal to async_task */
				qurt_signal_set(&(device->sem_async), HIF_ASYNC_TASK_IO_EVT);
			
				uint32 signalled = qurt_signal_wait(&(busrequest->sem_req),
														  HIF_BUS_REQUEST_COMPLETE_EVT ,
														  QURT_SIGNAL_ATTR_WAIT_ANY);
				
				qurt_signal_clear(&(busrequest->sem_req), signalled);
				
				if (signalled & HIF_BUS_REQUEST_COMPLETE_EVT) {
                    htc_status_t status = busrequest->status;
                    HTC_DEBUG_PRINTF(HTC_DEBUG_TRACE, ("HTC/HIF: sync return freeing 0x%lX: 0x%X\n",
                              (unsigned long)busrequest, busrequest->status));
                    HTC_DEBUG_PRINTF(HTC_DEBUG_TRACE, ("HTC/HIF: freeing req: 0x%X\n", (unsigned int)request));
                    hifFreeBusRequest(device, busrequest);
                    return status;
				} else {
                    return HTC_ERROR;
				}
            } else {
                HTC_DEBUG_PRINTF(HTC_DEBUG_TRACE, ("HTC/HIF: queued async req: 0x%lX\n", (unsigned long)busrequest));
				/* Send IO request signal to async_task */
				qurt_signal_set(&(device->sem_async), HIF_ASYNC_TASK_IO_EVT);
				
                return HTC_PENDING;
            }
        } else {
            HTC_DEBUG_PRINTF(HTC_DEBUG_ERROR,
                            ("HTC/HIF: Invalid execution mode: 0x%08x\n", (unsigned int)request));
            status = HTC_EINVAL;
            break;
        }
    } while(0);

    return status;
}

/**
 * _hif_free_bus_request() - Free the bus access request
 * @device:    device handle.
 * @request:   bus access request.
 *
 * This is the legacy method to handle an asynchronous bus request.
 *
 * Return: None.
 */
static inline void _hif_free_bus_request(HIF_DEVICE *device,
				BUS_REQUEST *request)
{
	htc_status_t status = request->status;
	void *context = request->context;

	HTC_DEBUG_PRINTF(HTC_DEBUG_TRACE,
		("HTC/HIF: async_task_id freeing req: 0x%lX\n",
		(unsigned long)request));
	hifFreeBusRequest(device, request);
	HTC_DEBUG_PRINTF(HTC_DEBUG_TRACE,
		("HTC/HIF: async_task_id completion routine req: 0x%lX\n",
		(unsigned long)request));
	device->htcCallbacks.rwCompletionHandler(context, status);
}

#ifdef TX_COMPLETION_THREAD
/**
 * add_to_tx_completion_list() - Queue a TX completion handler
 * @device:    context to the hif device.
 * @tx_comple: SDIO bus access request.
 *
 * This function adds an sdio bus access request to the
 * TX completion list.
 *
 * Return: No return.
 */
static void add_to_tx_completion_list(HIF_DEVICE *device,
		BUS_REQUEST *tx_comple)
{

	OSAL_MUTEX_LOCK(&device->tx_completion_lock);
	tx_comple->inusenext = NULL;
	*device->last_tx_completion = tx_comple;
	device->last_tx_completion = &tx_comple->inusenext;
	OSAL_MUTEX_UNLOCK(&device->tx_completion_lock);
}

/**
 * tx_clean_completion_list() - Clean the TX completion request list
 * @device:  HIF device handle.
 *
 * Function to clean the TX completion list.
 *
 * Return: No
 */
static void tx_clean_completion_list(HIF_DEVICE *device)
{
	BUS_REQUEST *comple;
	BUS_REQUEST *request;

	OSAL_MUTEX_LOCK(&device->tx_completion_lock);
	request = device->tx_completion_req;
	device->tx_completion_req = NULL;
	device->last_tx_completion = &device->tx_completion_req;
	OSAL_MUTEX_UNLOCK(&device->tx_completion_lock);

	while (request != NULL) {
		comple = request->inusenext;
		_hif_free_bus_request(device, request);
		request = comple;
	}

}

/**
 * tx_completion_task() - Thread to process TX completion
 * @param:   context to the hif device.
 *
 * This is the TX completion thread.
 *
 * Once TX completion message is received, completed TX
 * request will be queued in a tx_comple list and processed
 * in this thread.
 *
 * Return: 0 thread exits
 */
static int tx_completion_task(void *param)
{
	HIF_DEVICE *device;

	device = (HIF_DEVICE *)param;
	HTC_DEBUG_PRINTF(HTC_DEBUG_TRACE, ("HTC/HIF: tx completion task\n"));

	while (!device->tx_completion_shutdown) {
        /* wait for work */
		uint32 signalled = qurt_signal_wait(&(device->sem_tx_completion),
		                                  (HIF_TX_TASK_COMPLETION_REQ_EVT | HIF_TX_TASK_EXIT_EVT),
		                                  QURT_SIGNAL_ATTR_WAIT_ANY);
		
		qurt_signal_clear(&(device->sem_tx_completion), signalled);
		
		if ((signalled & HIF_TX_TASK_EXIT_EVT) && device->tx_completion_shutdown) {
			HTC_DEBUG_PRINTF(HTC_DEBUG_ERROR,
				("%s: tx completion task stopping\n",
				 __func__));
			break;
		}
		
		if (signalled & HIF_TX_TASK_COMPLETION_REQ_EVT) {
			while (device->tx_completion_req != NULL)
				tx_clean_completion_list(device);
		} else {
			HTC_DEBUG_PRINTF(HTC_DEBUG_ERROR,("%s: tx completion task interrupted\n", __func__));
			break;
		}
	}

	while (device->tx_completion_req != NULL)
		tx_clean_completion_list(device);

	qurt_signal_set(&(device->tx_completion_exit), HIF_TX_TASK_EXIT_CONFIRMATION_EVT);
	
	qurt_signal_destroy(&(device->sem_tx_completion));
	qurt_thread_stop();

	return 0;
}

/**
 * tx_completion_sem_init() - initialize tx completion semaphore
 * @device:  device handle.
 *
 * Initialize semaphore for TX completion thread's synchronization.
 *
 * Return: None.
 */
static inline void tx_completion_sem_init(HIF_DEVICE *device)
{
	OSAL_MUTEX_INIT(&device->tx_completion_lock);
	qurt_signal_init(&device->sem_tx_completion);
}

/**
 * hif_free_bus_request() - Function to free bus requests
 * @device:    device handle.
 * @request:   SIDO bus access request.
 *
 * If there is an completion thread, all the completed bus access requests
 * will be queued in a completion list. Otherwise, the legacy handler will
 * be called.
 *
 * Return: None.
 */
static inline void hif_free_bus_request(HIF_DEVICE *device,
			BUS_REQUEST *request)
{
	if (!device->tx_completion_shutdown) {
		add_to_tx_completion_list(device, request);
		qurt_signal_set(&(device->sem_tx_completion), HIF_TX_TASK_COMPLETION_REQ_EVT);

	} else {
		_hif_free_bus_request(device, request);
	}
}

/**
 * hif_start_tx_completion_thread() - Create and start the TX compl thread
 * @device:   device handle.
 *
 * This function will create the tx completion thread.
 *
 * Return: HTC_OK     thread created.
 *         HTC_ERROR  thread not created.
 */
static inline int hif_start_tx_completion_thread(HIF_DEVICE *device)
{
	if (!device->tx_completion_task) {
		device->tx_completion_req = NULL;
		device->last_tx_completion = &device->tx_completion_req;
		device->tx_completion_shutdown = 0;

		 qurt_thread_attr_set_name(&device->tx_completion_attr, HIF_TX_COMPLETION_TASK_NAME);
		 qurt_thread_attr_set_priority(&device->tx_completion_attr, HIF_TX_COMPLETION_TASK_PRIO);
		 qurt_thread_attr_set_stack_size(&device->tx_completion_attr, HIF_TX_COMPLETION_TASK_STK_SIZE);
		 
		 if (QURT_EOK != qurt_thread_create(&device->tx_completion_task,
		 								   &device->tx_completion_attr,
		 								   tx_completion_task, (void *)device))	 {
   			 device->tx_completion_shutdown = 1;
			 HTC_DEBUG_PRINTF(HTC_DEBUG_ERROR, ("HTC/HIF: fail to create tx_comple task\n"));
			 device->tx_completion_task = NULL;
			 return HTC_ERROR;
		 }
										   
		HTC_DEBUG_PRINTF(HTC_DEBUG_TRACE,
			("HTC/HIF: start tx_comple task\n"));
	}
	return HTC_OK;
}

/*
 * hif_stop_tx_completion_thread() - Destroy the tx compl thread
 * @device: device handle.
 *
 * This function will destroy the TX completion thread.
 *
 * Return: None.
 */
static inline void hif_stop_tx_completion_thread(HIF_DEVICE *device)
{
	uint32 signalled;

	if (device->tx_completion_task) {
		device->tx_completion_shutdown = 1;
		qurt_signal_set(&(device->sem_tx_completion), HIF_TX_TASK_EXIT_EVT);

		signalled = qurt_signal_wait(&(device->tx_completion_exit),
		                                  (HIF_TX_TASK_EXIT_CONFIRMATION_EVT),
		                                  QURT_SIGNAL_ATTR_WAIT_ANY);
		
		qurt_signal_clear(&(device->tx_completion_exit), signalled);
		
		if (signalled & HIF_TX_TASK_EXIT_CONFIRMATION_EVT) {
			HTC_DEBUG_PRINTF(HTC_LOG_TRC, ("%s: tx completion task killed\n", __func__));
			qurt_signal_destroy(&(device->tx_completion_exit));
		}		
	}
}

#else

/**
 * tx_completion_sem_init() - Dummy func to initialize semaphore
 * @device: device handle.
 *
 * This is a dummy function when TX compl thread is not created.
 *
 * Return: None.
 */
static inline void tx_completion_sem_init(HIF_DEVICE *device)
{
}

/**
 * hif_free_bus_request() - Free the bus access request
 * @device:    device handle.
 * @request:   bus access request.
 *
 * Just call the legacy handler when there is no additional completion thread.
 *
 * Return: None.
 */
static inline void hif_free_bus_request(HIF_DEVICE *device,
			BUS_REQUEST *request)
{
	_hif_free_bus_request(device, request);
}

/**
 * hif_start_tx_completion_thread() - Dummy function to start tx_compl thread.
 * @device:   device handle.
 *
 * Dummy function when tx completion thread is not created.
 *
 * Return: None.
 */
static inline void hif_start_tx_completion_thread(HIF_DEVICE *device)
{
}

/**
 * hif_stop_tx_completion_thread() - Dummy function to stop tx_compl thread.
 * @device:   device handle.
 *
 * Dummy function when tx conpletion thread is not created.
 *
 * Return: None.
 */
static inline void hif_stop_tx_completion_thread(HIF_DEVICE *device)
{
}
#endif

/* thread to serialize all requests, both sync and async */
static void async_task(void *param)
 {
    HIF_DEVICE *device;
    BUS_REQUEST *request;
    htc_status_t status;

    device = (HIF_DEVICE *)param;
    HTC_DEBUG_PRINTF(HTC_DEBUG_TRACE, ("HTC/HIF: async task\n"));
    qurt_signal_set(&(device->sem_async), HIF_ASYNC_TASK_STARTED);
    while(!device->async_shutdown) {
        /* wait for work */
		uint32 signalled = qurt_signal_wait(&(device->sem_async),
		                                  HIF_ASYNC_TASK_IO_EVT | HIF_ASYNC_TASK_TERM_EVT,
		                                  QURT_SIGNAL_ATTR_CLEAR_MASK);


		if (signalled & HIF_ASYNC_TASK_TERM_EVT) {
			/* Signal back the requested thread to kill async task */
		   qurt_signal_set(&(device->async_completion), HIF_ASYNC_TASK_COMPLETION_EVT);
		   qurt_signal_destroy(&(device->sem_async));
		   qurt_thread_stop();
		   break;
		}
		
		if (signalled & HIF_ASYNC_TASK_IO_EVT) {
	        if (device->async_shutdown) {
	            HTC_DEBUG_PRINTF(HTC_DEBUG_TRACE, ("HTC/HIF: async task stopping\n"));
	            break;
	        }

	        OSAL_MUTEX_LOCK(&device->asynclock);
	        /* pull the request to work on */
	        while (device->asyncreq != NULL) {
	            request = device->asyncreq;
	            if (request->inusenext != NULL) {
	                device->asyncreq = request->inusenext;
	            } else {
	                device->asyncreq = NULL;
	            }
	            OSAL_MUTEX_UNLOCK(&device->asynclock);
	            HTC_DEBUG_PRINTF(HTC_DEBUG_TRACE, ("HTC/HIF: async_task_id processing req: 0x%lX\n", (unsigned long)request));

	                    /* call HIFReadWrite in sync mode to do the work */
	                status = __HIFReadWrite(device, request->address, request->buffer,
	                                      request->length, request->request & ~HIF_SYNCHRONOUS, NULL);
	                if (request->request & HIF_ASYNCHRONOUS) {
	                    request->status = status;
	                    hif_free_bus_request(device, request);
	                } else {
	                    HTC_DEBUG_PRINTF(HTC_DEBUG_TRACE, ("HTC/HIF: async_task upping req: 0x%lX\n", (unsigned long)request));
	                    request->status = status;
						qurt_signal_set(&(request->sem_req), HIF_BUS_REQUEST_COMPLETE_EVT);
	                }
	            OSAL_MUTEX_LOCK(&device->asynclock);
	        }
	        OSAL_MUTEX_UNLOCK(&device->asynclock);
		}
    }
}

htc_status_t
PowerStateChangeNotify(HIF_DEVICE *device, HIF_DEVICE_POWER_CHANGE_TYPE config)
{
    htc_status_t status = HTC_OK;
    HTC_DEBUG_PRINTF(HTC_DEBUG_TRACE, ("HTC/HIF: +PowerStateChangeNotify %d\n", config));
    switch (config) {
       case HIF_DEVICE_POWER_DOWN:
            break;
       case HIF_DEVICE_POWER_CUT:
            break;
       case HIF_DEVICE_POWER_UP:
            break;
    }
    device->powerConfig = config;

    HTC_DEBUG_PRINTF(HTC_DEBUG_TRACE, ("HTC/HIF: -PowerStateChangeNotify\n"));

    return status;
}

htc_status_t
HIFConfigureDevice(HIF_DEVICE *device, HIF_DEVICE_CONFIG_OPCODE opcode,
                   void *config, OSAL_UINT32 configLen)
{
    OSAL_UINT32 count;
    htc_status_t status = HTC_OK;

    switch(opcode) {
        case HIF_DEVICE_GET_ENDPOINT_BLOCK_SIZE:
            ((OSAL_UINT32 *)config)[0] = HIF_ENDPOINT0_BLOCK_SIZE;
            ((OSAL_UINT32 *)config)[1] = HIF_ENDPOINT1_BLOCK_SIZE;
            ((OSAL_UINT32 *)config)[2] = HIF_ENDPOINT2_BLOCK_SIZE;
            ((OSAL_UINT32 *)config)[3] = HIF_ENDPOINT3_BLOCK_SIZE;
            break;

        case HIF_DEVICE_GET_ENDPOINT_ADDR:
            for (count = 0; count < 4; count ++) {
                ((OSAL_UINT32 *)config)[count] = HIF_ENDPOINT_START_ADDR(count);
            }

            break;
        case HIF_DEVICE_GET_PENDING_EVENTS_FUNC:
            HTC_DEBUG_PRINTF(HTC_DEBUG_WARN,
                            ("HTC/HIF: configuration opcode %d is not used for Linux SDIO stack \n", opcode));
            status = HTC_ERROR;
            break;
        case HIF_DEVICE_GET_IRQ_PROC_MODE:
            *((HIF_DEVICE_IRQ_PROCESSING_MODE *)config) = HIF_DEVICE_IRQ_SYNC_ONLY;
            break;
        case HIF_DEVICE_GET_RECV_EVENT_MASK_UNMASK_FUNC:
            HTC_DEBUG_PRINTF(HTC_DEBUG_WARN,
                            ("HTC/HIF: configuration opcode %d is not used for Linux SDIO stack \n", opcode));
            status = HTC_ERROR;
            break;
        case HIF_DEVICE_GET_OS_DEVICE:
                /* pass back a pointer to the SDIO function's "dev" struct */
            ((HIF_DEVICE_OS_DEVICE_INFO *)config)->pOSDevice = &device->tdev;
            break;
        case HIF_DEVICE_POWER_STATE_CHANGE:
            status = PowerStateChangeNotify(device, *(HIF_DEVICE_POWER_CHANGE_TYPE *)config);
            break;
        case HIF_DEVICE_GET_IRQ_YIELD_PARAMS:
            HTC_DEBUG_PRINTF(HTC_DEBUG_WARN,
                            ("HTC/HIF: configuration opcode %d is only used for RTOS systems, not Linux systems\n", opcode));
            status = HTC_ERROR;
            break;
        case HIF_DEVICE_SET_HTC_CONTEXT:
            device->htcContext = config;
            break;
        case HIF_DEVICE_GET_HTC_CONTEXT:
            if (config == NULL){
                HTC_DEBUG_PRINTF(HTC_DEBUG_WARN, ("Argument of HIF_DEVICE_GET_HTC_CONTEXT is NULL\n"));
                return HTC_ERROR;
            }
            *(void**)config = device->htcContext;
            break;
      case HIF_BMI_DONE:
        {
            HTC_DEBUG_PRINTF(HTC_DEBUG_ERROR, ("%s: BMI_DONE\n", __FUNCTION__)); /* TBDXXX */
            break;
        }
        default:
            HTC_DEBUG_PRINTF(HTC_DEBUG_WARN,
                            ("HTC/HIF: Unsupported configuration opcode: %d\n", opcode));
            status = HTC_ERROR;
    }

    return status;
}

void
HIFShutDownDevice(HIF_DEVICE *device)
{
    int i;

    HTC_DEBUG_PRINTF(HTC_DEBUG_TRACE, ("HTC/HIF: +HIFShutDownDevice\n"));
        /* since we are unloading the driver anyways, reset all cards in case the SDIO card
         * is externally powered and we are unloading the SDIO stack.  This avoids the problem when
         * the SDIO stack is reloaded and attempts are made to re-enumerate a card that is already
         * enumerated */
    HTC_DEBUG_PRINTF(HTC_DEBUG_TRACE, ("HTC/HIF: HIFShutDownDevice, resetting\n"));
    ResetAllCards();

	trans_deinit(device->tdev);

    for (i=0; i<HIF_MAX_DEVICES; ++i) {
        if (hif_devices[i] && hif_devices[i]->tdev == NULL) {
            HTC_DEBUG_PRINTF(HTC_DEBUG_TRACE,
                            ("HTC/HIF: Remove pending hif_device %p\n", hif_devices[i]));
            delHifDevice(hif_devices[i]);
            hif_devices[i] = NULL;
        }
    }

    HTC_DEBUG_PRINTF(HTC_DEBUG_TRACE, ("HTC/HIF: -HIFShutDownDevice\n"));
}

void  hifIRQHandler(void *tdev)
{
    HIF_DEVICE *device;
    HTC_DEBUG_PRINTF(HTC_DEBUG_TRACE, ("HTC/HIF: +hifIRQHandler\n"));

    device = getHifDevice(tdev);
    device->htcCallbacks.dsrHandler(device->htcCallbacks.context);

    HTC_DEBUG_PRINTF(HTC_DEBUG_TRACE, ("HTC/HIF: -hifIRQHandler\n"));
	return ;
}

static int hifDeviceInserted(void *dev, OSAL_BOOL hotplug)
{
    int i;
    int ret;
    HIF_DEVICE * device = NULL;
    int count;
	trans_device_t *tdev = (trans_device_t *)dev;


/* TODO: Revisit later, this is power cut use case */
#if 0
    for (i=0; i<HIF_MAX_DEVICES; ++i) {
        HIF_DEVICE *hifdevice = hif_devices[i];
        if (hifdevice && hifdevice->powerConfig == HIF_DEVICE_POWER_CUT &&
                hifdevice->host == func->card->host) {
            hifdevice->func = func;
            hifdevice->powerConfig = HIF_DEVICE_POWER_UP;
            trans_set_hifcontext(func, hifdevice); 
            device = getHifDevice(func);

            if (device->is_suspend) {
        		HTC_DEBUG_PRINTF(HTC_DEBUG_TRACE,("HTC/HIF: hifDeviceInserted, Resume from suspend, need to ReinitSDIO.\n"));
                ret = ReinitSDIO(device);
            }
            break;
        }
    }
#endif

    if (device==NULL) {
        if (addHifDevice(tdev) == NULL) {
            return -1;
        }
        device = getHifDevice(tdev);

        for (i=0; i<HIF_MAX_DEVICES; ++i) {
            if (hif_devices[i] == NULL) {
                hif_devices[i] = device;
                break;
            }
        }
        if (i==HIF_MAX_DEVICES) {
            HTC_DEBUG_PRINTF(HTC_DEBUG_ERROR,
                ("HTC/HIF: hifDeviceInserted, No more hif_devices[] slot for %p", device));
        }

        device->is_disabled = TRUE;

        OSAL_MUTEX_INIT(&device->lock);
        OSAL_MUTEX_INIT(&device->asynclock);

        /* Initialize the bus requests to be used later */
        OSAL_MEMZERO(device->busRequest, sizeof(device->busRequest));
        for (count = 0; count < BUS_REQUEST_MAX_NUM; count ++) {
			qurt_signal_init(&device->busRequest[count].sem_req);
            hifFreeBusRequest(device, &device->busRequest[count]);
        }

        qurt_signal_init(&device->sem_async);
        qurt_signal_init(&device->async_completion);

        tx_completion_sem_init(device);
    }

    ret = hifEnableFunc(device, device->tdev, hotplug);

    if (ret == HTC_OK || ret == HTC_PENDING) {
        return 0;
    } else {
        /* Error case */
        for (i = 0; i < HIF_MAX_DEVICES; ++i) {
            if (hif_devices[i] == device) {
                hif_devices[i] = NULL;
                break;
            }
        }
        trans_set_hifcontext(tdev, NULL);
        delHifDevice(device);
        return -1;
    }
}



void
HIFAckInterrupt(HIF_DEVICE *device)
{
    ASSERT(device != NULL);

    /* Acknowledge our function IRQ */
}

void
HIFUnMaskInterrupt(HIF_DEVICE *device)
{
    int ret;

    if (device == NULL || device->tdev == NULL)
    {
        return;
    }
	
	ret = trans_register_interrupt(device->tdev, hifIRQHandler);

    /* Register the IRQ Handler */
    ASSERT(ret == 0);
}

void HIFMaskInterrupt(HIF_DEVICE *device)
{
    if (device == NULL || device->tdev == NULL)
    {
        return;
    }
	trans_deregister_interrupt(device->tdev);
}

BUS_REQUEST *hifAllocateBusRequest(HIF_DEVICE *device)
{
    BUS_REQUEST *busrequest;

    /* Acquire lock */
    OSAL_MUTEX_LOCK(&device->lock);

    /* Remove first in list */
    if((busrequest = device->s_busRequestFreeQueue) != NULL)
    {
        device->s_busRequestFreeQueue = busrequest->next;
    }
    /* Release lock */
    OSAL_MUTEX_UNLOCK(&device->lock);
    HTC_DEBUG_PRINTF(HTC_DEBUG_TRACE, ("HTC/HIF: hifAllocateBusRequest: 0x%p\n", busrequest));
    return busrequest;
}

void
hifFreeBusRequest(HIF_DEVICE *device, BUS_REQUEST *busrequest)
{
    if (busrequest == NULL)
       return;
    //HTC_DEBUG_PRINTF(HTC_DEBUG_TRACE, ("HTC/HIF: hifFreeBusRequest: 0x%p\n", busrequest));
    /* Acquire lock */
    OSAL_MUTEX_LOCK(&device->lock);


    /* Insert first in list */
    busrequest->next = device->s_busRequestFreeQueue;
    busrequest->inusenext = NULL;
    device->s_busRequestFreeQueue = busrequest;

    /* Release lock */
    OSAL_MUTEX_UNLOCK(&device->lock);
}

static htc_status_t hifDisableFunc(HIF_DEVICE *device, trans_device_t *tdev)
{
    uint32 signalled;

    device = getHifDevice(tdev);

    hif_stop_tx_completion_thread(device);

    if (device->async_task_id) {
        device->async_shutdown = 1;

        /* Send terminate signal to async_task */
        qurt_signal_set(&(device->sem_async), HIF_ASYNC_TASK_TERM_EVT);

        /* Wait till the async_task gets killed */
        signalled = qurt_signal_wait(&(device->async_completion),
                                          HIF_ASYNC_TASK_COMPLETION_EVT,
                                          QURT_SIGNAL_ATTR_CLEAR_MASK);

        if (signalled & HIF_ASYNC_TASK_COMPLETION_EVT) {
            device->async_task_id = (unsigned long) NULL;
            device->async_shutdown = 0;
        } else {
            return HTC_ERROR;
        }
    }

    /* Disable the physical transport */
    trans_disable_trans(tdev);

    device->is_disabled = TRUE;
    return HTC_OK;
}

static htc_status_t hifEnableFunc(HIF_DEVICE *device, trans_device_t *tdev, OSAL_BOOL hotplug)
{
    int ret = HTC_OK;

    HTC_DEBUG_PRINTF(HTC_DEBUG_ERROR, ("HTC/HIF: +hifEnableFunc\n"));
    device = getHifDevice(tdev);

    if (!device) {
        HTC_DEBUG_PRINTF(HTC_DEBUG_ERR, ("HIF device is NULL\n"));
        return HTC_EINVAL;
    }

    if (device->is_disabled) {
		
		/* Enable physical transport */
		trans_enable_trans(tdev);
		
        device->is_disabled = FALSE;
        hif_start_tx_completion_thread(device);

        /* create async I/O thread */
        if (!device->async_task_id) {
            device->async_shutdown = 0;
		
			qurt_thread_attr_set_name(&device->async_task_attr, HIF_ASYNC_TASK_NAME);
			qurt_thread_attr_set_priority(&device->async_task_attr, HIF_ASYNC_TASK_PRIO);
			qurt_thread_attr_set_stack_size(&device->async_task_attr, HIF_ASYNC_TASK_STK_SIZE);
			
			if(QURT_EOK != qurt_thread_create(&device->async_task_id, &device->async_task_attr, async_task, (void *)device))
			{
				HTC_DEBUG_PRINTF(HTC_DEBUG_ERROR, ("HTC/HIF: %s(), to create async task\n", __FUNCTION__));
				device->async_task_id = (unsigned long) NULL;
				return HTC_ERROR;
			}

           HTC_DEBUG_PRINTF(HTC_DEBUG_TRACE, ("HTC/HIF: start async task\n"));
        }
    }

    qurt_signal_wait(&(device->sem_async), HIF_ASYNC_TASK_STARTED,
                        QURT_SIGNAL_ATTR_CLEAR_MASK);

    if (!device->claimedContext) {
        HTC_DEBUG_PRINTF(HTC_DEBUG_TRACE,
            ("AR6k: call deviceInsertedHandler\n"));
        ret = osdrvCallbacks.deviceInsertedHandler(
            osdrvCallbacks.context,device, hotplug);
        /* start  up inform DRV layer */
        if (ret != HTC_OK) {
            HTC_DEBUG_PRINTF(HTC_DEBUG_TRACE,
                ("AR6k: Device rejected error:%d \n", ret));
            /*
             * Disable the SDIO func & Reset the sdio
             * for automated tests to move ahead, where
             * the card does not need to be removed at
             * the end of the test.
             */
            hifDisableFunc(device, device->tdev);
        }
    } else {
        HTC_DEBUG_PRINTF(HTC_DEBUG_TRACE,
             ("AR6k: call devicePwrChangeApi\n"));
        /* start  up inform DRV layer */
        if (device->claimedContext &&
            osdrvCallbacks.devicePowerChangeHandler &&
            ((ret = osdrvCallbacks.devicePowerChangeHandler(
                  device->claimedContext, HIF_DEVICE_POWER_UP)) != HTC_OK))
                HTC_DEBUG_PRINTF(HTC_DEBUG_TRACE,
                    ("AR6k: Device rejected error:%d \n", ret));
    }
    HTC_DEBUG_PRINTF(HTC_DEBUG_TRACE, ("HTC/HIF: -hifEnableFunc\n"));

    /* task will call the enable func, indicate pending */
    return ret;
}

static htc_status_t hifDeviceRemoved(void *dev, OSAL_BOOL hotplug)
{
    htc_status_t status = HTC_OK;
    HIF_DEVICE *device;
    int count;
	trans_device_t *tdev = (trans_device_t *)dev;

	
    if (tdev == NULL)
	    return HTC_ERROR;

    device = getHifDevice(tdev);
    if (!device) {
        HTC_DEBUG_PRINTF(HTC_DEBUG_TRACE, ("%s: Failed to get the sdio driver private data\n", __func__));
        return HTC_ERROR;
    }

	OSAL_MUTEX_DEINIT(&device->lock);
	OSAL_MUTEX_DEINIT(&device->asynclock);
	
	for (count = 0; count < BUS_REQUEST_MAX_NUM; count ++) {
		qurt_signal_destroy(&device->busRequest[count].sem_req);
	}

    if (device->powerConfig == HIF_DEVICE_POWER_CUT) {
        device->tdev = NULL; /* func will be free by mmc stack */
        return status; /* Just return for cut-off mode */
    } else {
        int i;
        for (i=0; i<HIF_MAX_DEVICES; ++i) {
            if (hif_devices[i] == device) {
                hif_devices[i] = NULL;
            }
        }
    }

    if (device->claimedContext != NULL) {
        status = osdrvCallbacks.deviceRemovedHandler(device->claimedContext, device, hotplug);
    }

    HIFMaskInterrupt(device);

    if (device->is_disabled) {
        device->is_disabled = FALSE;
    } else {
        status = hifDisableFunc(device, device->tdev);
    }
	
	qurt_signal_destroy(&device->async_completion);

    delHifDevice(device);
    if (status != HTC_OK) {
        HTC_DEBUG_PRINTF(HTC_DEBUG_WARN,
            ("HTC/HIF: Unable to disable sdio func. Card removed?\n"));
    }
	
	return status;
}


static HIF_DEVICE *
addHifDevice(trans_device_t *tdev)
{
    HIF_DEVICE *hifdevice = NULL;

    if (tdev == NULL)
        return NULL;

    hifdevice = (HIF_DEVICE *)OSAL_MALLOC(sizeof(HIF_DEVICE));
    if (hifdevice == NULL)
        return NULL;
    OSAL_MEMZERO(hifdevice, sizeof(*hifdevice));

    hifdevice->tdev = tdev;
    hifdevice->powerConfig = HIF_DEVICE_POWER_UP;
    hifdevice->DeviceState = HIF_DEVICE_STATE_ON;
    trans_set_hifcontext(tdev, hifdevice);

    return hifdevice;
}

static HIF_DEVICE *
getHifDevice(trans_device_t *tdev)
{
    return (HIF_DEVICE *)trans_get_hifcontext(tdev);
}

static void
delHifDevice(HIF_DEVICE * device)
{
    if (device == NULL)
        return;
    HTC_DEBUG_PRINTF(HTC_DEBUG_TRACE, ("HTC/HIF: delHifDevice; 0x%p\n", device));
    OSAL_FREE(device);
}

static void ResetAllCards(void)
{
}

void HIFClaimDevice(HIF_DEVICE  *device, void *context)
{
    device->claimedContext = context;
}

void HIFReleaseDevice(HIF_DEVICE  *device)
{
    device->claimedContext = NULL;
}

htc_status_t HIFAttachHTC(HIF_DEVICE *device, HTC_CALLBACKS *callbacks)
{
    if (device->htcCallbacks.context != NULL) {
            /* already in use! */
        return HTC_ERROR;
    }
    device->htcCallbacks = *callbacks;
    return HTC_OK;
}

static void hif_flush_async_task(HIF_DEVICE *device)
{
    uint32 signalled = 0;

    if (device->async_task_id) {
        device->async_shutdown = 1;

        /* Send terminate signal to async_task */
        qurt_signal_set(&(device->sem_async), HIF_ASYNC_TASK_TERM_EVT);

        /* Wait till the async_task gets killed */
        signalled = qurt_signal_wait(&(device->async_completion),
                                          HIF_ASYNC_TASK_COMPLETION_EVT,
                                          QURT_SIGNAL_ATTR_CLEAR_MASK);

        if (signalled & HIF_ASYNC_TASK_COMPLETION_EVT) {
            device->async_task_id = (unsigned long) NULL;
            device->async_shutdown = 0;
        }
    }
}

/**
 * hif_reset_target() - Reset target device
 * @hif_device: pointer to hif_device structure
 *
 * Reset the target by invoking power off and power on
 * sequence to bring back target into active state.
 * This API shall be called only when driver load/unload
 * is in progress.
 *
 * Return: 0 on success, error for failure case.
 */
static htc_status_t hif_reset_target(HIF_DEVICE *hif_device)
{
	int ret = HTC_OK;

	if (!hif_device || !hif_device->tdev) {
		HTC_DEBUG_PRINTF(HTC_DEBUG_ERROR,
			("HTC/HIF: %s invalid HIF DEVICE \n", __func__));
		return HTC_ERROR;
	}

	ret = trans_reset_target(hif_device->tdev);

	if(ret) {
		HTC_DEBUG_PRINTF(HTC_DEBUG_ERROR,
			("HTC/HIF: %s Failed to reset target Power %d\n", ret));
		return HTC_ERROR;
	}

	return ret;
}

void HIFDetachHTC(HIF_DEVICE *device)
{
    hif_flush_async_task(device);
    if (device->ctrl_response_timeout) {
        /* Reset the target by invoking power off and power on sequence to
         * the card to bring back into active state.
         */
        if(hif_reset_target(device) != HTC_OK)
		{
            panic("BUG");
		}
        device->ctrl_response_timeout = false;
    }

    /*OSAL_MEMZERO(&device->htcCallbacks,sizeof(device->htcCallbacks));*/
}

void HIFSetHandle(void *hif_handle, void *handle)
{
    HIF_DEVICE *device = (HIF_DEVICE *) hif_handle;

    device->htc_handle = handle;

    return;
}
