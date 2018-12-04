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

 
/***********************************************************************
 *
 * This file contains the HTC APIs that are exposed to higher layers.
 *
 **********************************************************************/

//==============================================================================
// HIF specific declarations and prototypes
//
// Author(s): ="Qualcomm"
//==============================================================================
#ifndef _HIF_H_
#define _HIF_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define ENABLE_ENDPOINT_DUMMY_SPACE_FEATURE 1

typedef struct hif_device HIF_DEVICE;
typedef void  *A_target_id_t;

#define HIF_TYPE_AR6002 2
#define HIF_TYPE_AR6003 3
#define HIF_TYPE_AR6004 5
#define HIF_TYPE_AR9888 6
#define HIF_TYPE_AR6320 7
#define HIF_TYPE_AR6320V2 8
/* For attaching Peregrine 2.0 board host_reg_tbl only */
#define HIF_TYPE_AR9888V2 8

/*
 * direction - Direction of transfer (HIF_READ/HIF_WRITE).
 */
#define HIF_READ                    0x00000001
#define HIF_WRITE                   0x00000002
#define HIF_DIR_MASK                (HIF_READ | HIF_WRITE)

/*
 *     type - An interface may support different kind of read/write commands.
 *            For example: SDIO supports CMD52/CMD53s. In case of MSIO it
 *            translates to using different kinds of TPCs. The command type
 *            is thus divided into a basic and an extended command and can
 *            be specified using HIF_BASIC_IO/HIF_EXTENDED_IO.
 */
#define HIF_BASIC_IO                0x00000004
#define HIF_EXTENDED_IO             0x00000008
#define HIF_TYPE_MASK               (HIF_BASIC_IO | HIF_EXTENDED_IO)

/*
 *     emode - This indicates the whether the command is to be executed in a
 *             blocking or non-blocking fashion (HIF_SYNCHRONOUS/
 *             HIF_ASYNCHRONOUS). The read/write data paths in HTC have been
 *             implemented using the asynchronous mode allowing the the bus
 *             driver to indicate the completion of operation through the
 *             registered callback routine. The requirement primarily comes
 *             from the contexts these operations get called from (a driver's
 *             transmit context or the ISR context in case of receive).
 *             Support for both of these modes is essential.
 */
#define HIF_SYNCHRONOUS             0x00000010
#define HIF_ASYNCHRONOUS            0x00000020
#define HIF_EMODE_MASK              (HIF_SYNCHRONOUS | HIF_ASYNCHRONOUS)

/*
 *     dmode - An interface may support different kinds of commands based on
 *             the tradeoff between the amount of data it can carry and the
 *             setup time. Byte and Block modes are supported (HIF_BYTE_BASIS/
 *             HIF_BLOCK_BASIS). In case of latter, the data is rounded off
 *             to the nearest block size by padding. The size of the block is
 *             configurable at compile time using the HIF_BLOCK_SIZE and is
 *             negotiated with the target during initialization after the
 *             AR6000 interrupts are enabled.
 */
#define HIF_BYTE_BASIS              0x00000040
#define HIF_BLOCK_BASIS             0x00000080
#define HIF_DMODE_MASK              (HIF_BYTE_BASIS | HIF_BLOCK_BASIS)

/*
 *     amode - This indicates if the address has to be incremented on AR6000
 *             after every read/write operation (HIF?FIXED_ADDRESS/
 *             HIF_INCREMENTAL_ADDRESS).
 */
#define HIF_FIXED_ADDRESS           0x00000100
#define HIF_INCREMENTAL_ADDRESS     0x00000200
#define HIF_AMODE_MASK              (HIF_FIXED_ADDRESS | HIF_INCREMENTAL_ADDRESS)

/*
 *      dummy - data written into the dummy space will not put into the final endpoint FIFO
 */
#define HIF_DUMMY_SPACE_MASK              0xFFFF0000


#define HIF_WR_ASYNC_BYTE_FIX   \
    (HIF_WRITE | HIF_ASYNCHRONOUS | HIF_EXTENDED_IO | HIF_BYTE_BASIS | HIF_FIXED_ADDRESS)
#define HIF_WR_ASYNC_BYTE_INC   \
    (HIF_WRITE | HIF_ASYNCHRONOUS | HIF_EXTENDED_IO | HIF_BYTE_BASIS | HIF_INCREMENTAL_ADDRESS)
#define HIF_WR_ASYNC_BLOCK_INC  \
    (HIF_WRITE | HIF_ASYNCHRONOUS | HIF_EXTENDED_IO | HIF_BLOCK_BASIS | HIF_INCREMENTAL_ADDRESS)
#define HIF_WR_SYNC_BYTE_FIX    \
    (HIF_WRITE | HIF_SYNCHRONOUS | HIF_EXTENDED_IO | HIF_BYTE_BASIS | HIF_FIXED_ADDRESS)
#define HIF_WR_SYNC_BYTE_INC    \
    (HIF_WRITE | HIF_SYNCHRONOUS | HIF_EXTENDED_IO | HIF_BYTE_BASIS | HIF_INCREMENTAL_ADDRESS)
#define HIF_WR_SYNC_BLOCK_INC  \
    (HIF_WRITE | HIF_SYNCHRONOUS | HIF_EXTENDED_IO | HIF_BLOCK_BASIS | HIF_INCREMENTAL_ADDRESS)
#define HIF_WR_ASYNC_BLOCK_FIX \
    (HIF_WRITE | HIF_ASYNCHRONOUS | HIF_EXTENDED_IO | HIF_BLOCK_BASIS | HIF_FIXED_ADDRESS)
#define HIF_WR_SYNC_BLOCK_FIX  \
    (HIF_WRITE | HIF_SYNCHRONOUS | HIF_EXTENDED_IO | HIF_BLOCK_BASIS | HIF_FIXED_ADDRESS)
#define HIF_RD_SYNC_BYTE_INC    \
    (HIF_READ | HIF_SYNCHRONOUS | HIF_EXTENDED_IO | HIF_BYTE_BASIS | HIF_INCREMENTAL_ADDRESS)
#define HIF_RD_SYNC_BYTE_FIX    \
    (HIF_READ | HIF_SYNCHRONOUS | HIF_EXTENDED_IO | HIF_BYTE_BASIS | HIF_FIXED_ADDRESS)
#define HIF_RD_ASYNC_BYTE_FIX   \
    (HIF_READ | HIF_ASYNCHRONOUS | HIF_EXTENDED_IO | HIF_BYTE_BASIS | HIF_FIXED_ADDRESS)
#define HIF_RD_ASYNC_BLOCK_FIX  \
    (HIF_READ | HIF_ASYNCHRONOUS | HIF_EXTENDED_IO | HIF_BLOCK_BASIS | HIF_FIXED_ADDRESS)
#define HIF_RD_ASYNC_BYTE_INC   \
    (HIF_READ | HIF_ASYNCHRONOUS | HIF_EXTENDED_IO | HIF_BYTE_BASIS | HIF_INCREMENTAL_ADDRESS)
#define HIF_RD_ASYNC_BLOCK_INC  \
    (HIF_READ | HIF_ASYNCHRONOUS | HIF_EXTENDED_IO | HIF_BLOCK_BASIS | HIF_INCREMENTAL_ADDRESS)
#define HIF_RD_SYNC_BLOCK_INC  \
    (HIF_READ | HIF_SYNCHRONOUS | HIF_EXTENDED_IO | HIF_BLOCK_BASIS | HIF_INCREMENTAL_ADDRESS)
#define HIF_RD_SYNC_BLOCK_FIX  \
    (HIF_READ | HIF_SYNCHRONOUS | HIF_EXTENDED_IO | HIF_BLOCK_BASIS | HIF_FIXED_ADDRESS)

typedef enum {
    HIF_DEVICE_POWER_STATE = 0,
    HIF_DEVICE_GET_ENDPOINT_BLOCK_SIZE,
    HIF_DEVICE_GET_ENDPOINT_ADDR,
    HIF_DEVICE_GET_PENDING_EVENTS_FUNC,
    HIF_DEVICE_GET_IRQ_PROC_MODE,
    HIF_DEVICE_GET_RECV_EVENT_MASK_UNMASK_FUNC,
    HIF_DEVICE_POWER_STATE_CHANGE,
    HIF_DEVICE_GET_IRQ_YIELD_PARAMS,
    HIF_CONFIGURE_QUERY_SCATTER_REQUEST_SUPPORT,
    HIF_DEVICE_GET_OS_DEVICE,
    HIF_DEVICE_DEBUG_BUS_STATE,
    HIF_BMI_DONE,
    HIF_DEVICE_SET_TARGET_TYPE,
    HIF_DEVICE_SET_HTC_CONTEXT,
    HIF_DEVICE_GET_HTC_CONTEXT,
} HIF_DEVICE_CONFIG_OPCODE;

/*
 * HIF CONFIGURE definitions:
 *
 *   HIF_DEVICE_GET_ENDPOINT_BLOCK_SIZE
 *   input : none
 *   output : array of 4 OSAL_UINT32s
 *   notes: block size is returned for each mailbox (4)
 *
 *   HIF_DEVICE_GET_ENDPOINT_ADDR
 *   input : none
 *   output : HIF_DEVICE_ENDPOINT_INFO
 *   notes:
 *
 *   HIF_DEVICE_GET_PENDING_EVENTS_FUNC
 *   input : none
 *   output: HIF_PENDING_EVENTS_FUNC function pointer
 *   notes: this is optional for the HIF layer, if the request is
 *          not handled then it indicates that the upper layer can use
 *          the standard device methods to get pending events (IRQs, mailbox messages etc..)
 *          otherwise it can call the function pointer to check pending events.
 *
 *   HIF_DEVICE_GET_IRQ_PROC_MODE
 *   input : none
 *   output : HIF_DEVICE_IRQ_PROCESSING_MODE (interrupt processing mode)
 *   note: the hif layer interfaces with the underlying OS-specific bus driver. The HIF
 *         layer can report whether IRQ processing is requires synchronous behavior or
 *         can be processed using asynchronous bus requests (typically faster).
 *
 *   HIF_DEVICE_GET_RECV_EVENT_MASK_UNMASK_FUNC
 *   input :
 *   output : HIF_MASK_UNMASK_RECV_EVENT function pointer
 *   notes: this is optional for the HIF layer.  The HIF layer may require a special mechanism
 *          to mask receive message events.  The upper layer can call this pointer when it needs
 *          to mask/unmask receive events (in case it runs out of buffers).
 *
 *   HIF_DEVICE_POWER_STATE_CHANGE
 *
 *   input : HIF_DEVICE_POWER_CHANGE_TYPE
 *   output : none
 *   note: this is optional for the HIF layer.  The HIF layer can handle power on/off state change
 *         requests in an interconnect specific way.  This is highly OS and bus driver dependent.
 *         The caller must guarantee that no HIF read/write requests will be made after the device
 *         is powered down.
 *
 *   HIF_DEVICE_GET_IRQ_YIELD_PARAMS
 *
 *   input : none
 *   output : HIF_DEVICE_IRQ_YIELD_PARAMS
 *   note: This query checks if the HIF layer wishes to impose a processing yield count for the DSR handler.
 *   The DSR callback handler will exit after a fixed number of RX packets or events are processed.
 *   This query is only made if the device reports an IRQ processing mode of HIF_DEVICE_IRQ_SYNC_ONLY.
 *   The HIF implementation can ignore this command if it does not desire the DSR callback to yield.
 *   The HIF layer can indicate the maximum number of IRQ processing units (RX packets) before the
 *   DSR handler callback must yield and return control back to the HIF layer.  When a yield limit is
 *   used the DSR callback will not call HIFAckInterrupts() as it would normally do before returning.
 *   The HIF implementation that requires a yield count must call HIFAckInterrupt() when it is prepared
 *   to process interrupts again.
 *
 *   HIF_CONFIGURE_QUERY_SCATTER_REQUEST_SUPPORT
 *   input : none
 *   output : HIF_DEVICE_SCATTER_SUPPORT_INFO
 *   note:  This query checks if the HIF layer implements the SCATTER request interface.  Scatter requests
 *   allows upper layers to submit mailbox I/O operations using a list of buffers.  This is useful for
 *   multi-message transfers that can better utilize the bus interconnect.
 *
 *
 *   HIF_DEVICE_GET_OS_DEVICE
 *   intput : none
 *   output : HIF_DEVICE_OS_DEVICE_INFO;
 *   note: On some operating systems, the HIF layer has a parent device object for the bus.  This object
 *         may be required to register certain types of logical devices.
 *
 *   HIF_DEVICE_DEBUG_BUS_STATE
 *   input : none
 *   output : none
 *   note: This configure option triggers the HIF interface to dump as much bus interface state.  This
 *   configuration request is optional (No-OP on some HIF implementations)
 *
 *   HIF_DEVICE_SET_TARGET_TYPE
 *   input : TARGET_TYPE_*
 *   output : none
 *   note: Some HIF implementations may need to know TargetType in order to access
 *   Target registers or Host Interest Area.  (No-OP on some HIF implementations)
 */

typedef struct {
    OSAL_UINT32    ExtendedAddress;  /* extended address for larger writes */
    OSAL_UINT32    ExtendedSize;
} HIF_ENDPOINT_PROPERTIES;

#define HIF_ENDPOINT_FLAG_NO_BUNDLING   (1 << 0)   /* do not allow bundling over the mailbox */

typedef struct {
    OSAL_UINT32 EndpointAddresses[4];  /* must be first element for legacy HIFs that return the address in
                                   and ARRAY of 32-bit words */

        /* the following describe extended mailbox properties */
    HIF_ENDPOINT_PROPERTIES EndpointProp[4];
        /* if the HIF supports the GEndpoint extended address region it can report it
         * here, some interfaces cannot support the GENDPOINT address range and not set this */
    OSAL_UINT32 GEndpointAddress;
    OSAL_UINT32 GEndpointSize;
    OSAL_UINT32 Flags;             /* flags to describe endpoint behavior or usage */
} HIF_DEVICE_ENDPOINT_INFO;

typedef enum {
    HIF_DEVICE_IRQ_SYNC_ONLY,   /* for HIF implementations that require the DSR to process all
                                   interrupts before returning */
    HIF_DEVICE_IRQ_ASYNC_SYNC,  /* for HIF implementations that allow DSR to process interrupts
                                   using ASYNC I/O (that is HIFAckInterrupt can be called at a
                                   later time */
} HIF_DEVICE_IRQ_PROCESSING_MODE;

typedef enum {
    HIF_DEVICE_POWER_UP,    /* HIF layer should power up interface and/or module */
    HIF_DEVICE_POWER_DOWN,  /* HIF layer should initiate bus-specific measures to minimize power */
    HIF_DEVICE_POWER_CUT    /* HIF layer should initiate bus-specific AND/OR platform-specific measures
                               to completely power-off the module and associated hardware (i.e. cut power supplies)
                            */
} HIF_DEVICE_POWER_CHANGE_TYPE;

typedef enum {
    HIF_DEVICE_STATE_ON,
    HIF_DEVICE_STATE_DEEPSLEEP,
    HIF_DEVICE_STATE_CUTPOWER,
    HIF_DEVICE_STATE_WOW
} HIF_DEVICE_STATE;

typedef struct {
    int     RecvPacketYieldCount; /* max number of packets to force DSR to return */
} HIF_DEVICE_IRQ_YIELD_PARAMS;


typedef struct {
    void    *pOSDevice;
} HIF_DEVICE_OS_DEVICE_INFO;


#define HIF_MAX_DEVICES                 4

typedef struct osdrv_callbacks {
    void      *context;     /* context to pass for all callbacks except deviceRemovedHandler
                               the deviceRemovedHandler is only called if the device is claimed */
    int (* deviceInsertedHandler)(void *context, void *hif_handle, OSAL_BOOL hotplug);
    int (* deviceRemovedHandler)(void *claimedContext, void *hif_handle, OSAL_BOOL hotplug);
    int (* deviceSuspendHandler)(void *context);
    int (* deviceResumeHandler)(void *context);
    int (* deviceWakeupHandler)(void *context);
    int (* devicePowerChangeHandler)(void *context, HIF_DEVICE_POWER_CHANGE_TYPE config);
} OSDRV_CALLBACKS;

#define HIF_OTHER_EVENTS     (1 << 0)   /* other interrupts (non-Recv) are pending, host
                                           needs to read the register table to figure out what */
#define HIF_RECV_MSG_AVAIL   (1 << 1)   /* pending recv packet */

typedef struct _HIF_PENDING_EVENTS_INFO {
    OSAL_UINT32 Events;
    OSAL_UINT32 LookAhead;
    OSAL_UINT32 AvailableRecvBytes;
} HIF_PENDING_EVENTS_INFO;

    /* function to get pending events , some HIF modules use special mechanisms
     * to detect packet available and other interrupts */
typedef int ( *HIF_PENDING_EVENTS_FUNC)(HIF_DEVICE              *device,
                                             HIF_PENDING_EVENTS_INFO *pEvents,
                                             void                    *AsyncContext);

#define HIF_MASK_RECV    TRUE
#define HIF_UNMASK_RECV  FALSE
    /* function to mask recv events */
typedef int ( *HIF_MASK_UNMASK_RECV_EVENT)(HIF_DEVICE  *device,
                                                OSAL_BOOL    Mask,
                                                void   *AsyncContext);

/*
 * This API is used to perform any global initialization of the HIF layer
 * and to set OS driver callbacks (i.e. insertion/removal) to the HIF layer
 *
 */
int HIFInit(OSDRV_CALLBACKS *callbacks);

/* This API claims the HIF device and provides a context for handling removal.
 * The device removal callback is only called when the OSDRV layer claims
 * a device.  The claimed context must be non-NULL */
void HIFClaimDevice(HIF_DEVICE *device, void *claimedContext);
/* release the claimed device */
void HIFReleaseDevice(HIF_DEVICE *device);

/* This API allows the HTC layer to attach to the HIF device */
int HIFAttachHTC(HIF_DEVICE *device, HTC_CALLBACKS *callbacks);
/* This API detaches the HTC layer from the HIF device */
void     HIFDetachHTC(HIF_DEVICE *device);
void HIFSetHandle(void *hif_handle, void *handle);
htc_status_t
HIFSyncRead(HIF_DEVICE *device,
               OSAL_UINT32 address,
               OSAL_UINT8 *buffer,
               OSAL_UINT32 length,
               OSAL_UINT32 request,
               void *context);

/*
 * This API is used to provide the read/write interface over the specific bus
 * interface.
 * address - Starting address in the AR6000's address space. For mailbox
 *           writes, it refers to the start of the endpoint boundary. It should
 *           be ensured that the last byte falls on the mailbox's EOM. For
 *           mailbox reads, it refers to the end of the endpoint boundary.
 * buffer - Pointer to the buffer containg the data to be transmitted or
 *          received.
 * length - Amount of data to be transmitted or received.
 * request - Characterizes the attributes of the command.
 */
int
HIFReadWrite(HIF_DEVICE    *device,
             OSAL_UINT32      address,
             OSAL_UINT8        *buffer,
             OSAL_UINT32      length,
             OSAL_UINT32      request,
             void          *context);

/*
 * This can be initiated from the unload driver context when the OSDRV layer has no more use for
 * the device.
 */
void HIFShutDownDevice(HIF_DEVICE *device);
void HIFSurpriseRemoved(HIF_DEVICE *device);

/*
 * This should translate to an acknowledgment to the bus driver indicating that
 * the previous interrupt request has been serviced and the all the relevant
 * sources have been cleared. HTC is ready to process more interrupts.
 * This should prevent the bus driver from raising an interrupt unless the
 * previous one has been serviced and acknowledged using the previous API.
 */
void HIFAckInterrupt(HIF_DEVICE *device);

void HIFMaskInterrupt(HIF_DEVICE *device);

void HIFUnMaskInterrupt(HIF_DEVICE *device);

int
HIFConfigureDevice(HIF_DEVICE *device, HIF_DEVICE_CONFIG_OPCODE opcode,
                   void *config, OSAL_UINT32 configLen);

/*
 * This API wait for the remaining ENDPOINT messages to be drained
 * This should be moved to HTC AR6K layer
 */
int hifWaitForPendingRecv(HIF_DEVICE *device);

/****************************************************************/
/* BMI and Diag window abstraction                              */
/****************************************************************/

#define HIF_BMI_EXCHANGE_NO_TIMEOUT  ((OSAL_UINT32)(0))

#define DIAG_TRANSFER_LIMIT 2048U /* maximum number of bytes that can be
                                    handled atomically by DiagRead/DiagWrite */

    /* API to handle HIF-specific BMI message exchanges, this API is synchronous
     * and only allowed to be called from a context that can block (sleep) */
int HIFExchangeBMIMsg(HIF_DEVICE *device,
                           OSAL_UINT8    *pSendMessage,
                           OSAL_UINT32   Length,
                           OSAL_UINT8    *pResponseMessage,
                           OSAL_UINT32   *pResponseLength,
                           OSAL_UINT32   TimeoutMS);



    /*
     * APIs to handle HIF specific diagnostic read accesses. These APIs are
     * synchronous and only allowed to be called from a context that can block (sleep).
     * They are not high performance APIs.
     *
     * HIFDiagReadAccess reads a 4 Byte aligned/length value from a Target register
     * or memory word.
     *
     * HIFDiagReadMem reads an arbitrary length of arbitrarily aligned memory.
     */
int HIFDiagReadAccess(HIF_DEVICE *hifDevice, OSAL_UINT32 address, OSAL_UINT32 *data);
int HIFDiagReadMem(HIF_DEVICE *hif_device, OSAL_UINT32 address, OSAL_UINT8 *data, int nbytes);
void HIFDumpTargetMemory(HIF_DEVICE *hif_device, void *ramdump_base,
                           OSAL_UINT32 address, OSAL_UINT32 size);
    /*
     * APIs to handle HIF specific diagnostic write accesses. These APIs are
     * synchronous and only allowed to be called from a context that can block (sleep).
     * They are not high performance APIs.
     *
     * HIFDiagWriteAccess writes a 4 Byte aligned/length value to a Target register
     * or memory word.
     *
     * HIFDiagWriteMem writes an arbitrary length of arbitrarily aligned memory.
     */
int HIFDiagWriteAccess(HIF_DEVICE *hifDevice, OSAL_UINT32 address, OSAL_UINT32 data);
int HIFDiagWriteMem(HIF_DEVICE *hif_device, OSAL_UINT32 address, OSAL_UINT8 *data, int nbytes);


void HIFSetMailboxSwap(HIF_DEVICE  *device);

int hif_register_driver(void);
void hif_unregister_driver(void);
/* The API's check if FW can be suspended as part of cfg80211 suspend.
 * This is done for SDIO drivers, for other bus types it's NO OP, they
 * enable/disable wow in bus suspend callback.
 * In SDIO driver bus suspend host will configure 4 bit sdio mode to
 * 1 bit sdio mode and set the appropriate host flags.
 */
OSAL_BOOL hif_is_80211_fw_wow_required(void);

#ifdef FEATURE_RUNTIME_PM
/* Runtime power management API of HIF to control
 * runtime pm. During Runtime Suspend the get API
 * return -EAGAIN. The caller can queue the cmd or return.
 * The put API decrements the usage count.
 * The get API increments the usage count.
 * The API's are exposed to HTT and WMI Services only.
 */
int hif_pm_runtime_get(HIF_DEVICE *);
int hif_pm_runtime_put(HIF_DEVICE *);
void *hif_runtime_pm_prevent_suspend_init(const char *);
void hif_runtime_pm_prevent_suspend_deinit(void *data);
int hif_pm_runtime_prevent_suspend(void *ol_sc, void *data);
int hif_pm_runtime_allow_suspend(void *ol_sc, void *data);
int hif_pm_runtime_prevent_suspend_timeout(void *ol_sc, void *data,
						unsigned int delay);
void hif_request_runtime_pm_resume(void *ol_sc);
#else
static inline int hif_pm_runtime_get(HIF_DEVICE *device) { return 0; }
static inline int hif_pm_runtime_put(HIF_DEVICE *device) { return 0; }
static inline int
hif_pm_runtime_prevent_suspend(void *ol_sc, void *context) { return 0; }
static inline int
hif_pm_runtime_allow_suspend(void *ol_sc, void *context) { return 0; }
static inline int
hif_pm_runtime_prevent_suspend_timeout(void *ol_sc, void *context,
						unsigned int msec)
{
	return 0;
}
static inline void *
hif_runtime_pm_prevent_suspend_init(const char *name) { return NULL; }
static inline void
hif_runtime_pm_prevent_suspend_deinit(void *context) { }
static inline void hif_request_runtime_pm_resume(void *ol_sc)
{ }
#endif
#ifdef __cplusplus
}
#endif

OSAL_BOOL HIFIsMailBoxSwapped(HIF_DEVICE *hd);

#ifdef HIF_PCI
int hif_addr_in_boundary(HIF_DEVICE *hif_device, OSAL_UINT32 offset);
#else
static inline int hif_addr_in_boundary(HIF_DEVICE *hif_device, OSAL_UINT32 offset)
{
	return 0;
}
#endif
#endif /* _HIF_H_ */
