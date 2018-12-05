/*
 * Copyright (c) 2017 Qualcomm Technologies, Inc.
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

#ifndef __HTC_DEFS_H__
#define __HTC_DEFS_H__
#include "qcli_api.h"
#include "qapi_fatal_err.h"

/*
 * This file contains definitions that may be used across both
 * Host and Target software.  Nothing here is module-dependent
 * or platform-dependent.
 */

/*
 * Generic error codes that can be used by hw, sta, ap, sim, dk
 * and any other environments. Since these are enums, feel free to
 * add any more codes that you need.
 */

typedef enum {
    HTC_ERROR = -1,               /* Generic error return */
    HTC_OK = 0,                   /* success */
                                /* Following values start at 1 */
    HTC_DEVICE_NOT_FOUND,         /* not able to find PCI device */
    HTC_NO_MEMORY,                /* not able to allocate memory, not available */
    HTC_MEMORY_NOT_AVAIL,         /* memory region is not free for mapping */
    HTC_NO_FREE_DESC,             /* no free descriptors available */
    HTC_BAD_ADDRESS,              /* address does not match descriptor */
    HTC_WIN_DRIVER_ERROR,         /* used in NT_HW version, if problem at init */
    HTC_REGS_NOT_MAPPED,          /* registers not correctly mapped */
    HTC_EPERM,                    /* Not superuser */
    HTC_EACCES,                   /* Access denied */
    HTC_ENOENT,                   /* No such entry, search failed, etc. */
    HTC_EEXIST,                   /* The object already exists (can't create) */
    HTC_EFAULT,                   /* Bad address fault */
    HTC_EBUSY,                    /* Object is busy */
    HTC_EINVAL,                   /* Invalid parameter */
    HTC_EMSGSIZE,                 /* Inappropriate message buffer length */
    HTC_ECANCELED,                /* Operation canceled */
    HTC_ENOTSUP,                  /* Operation not supported */
    HTC_ECOMM,                    /* Communication error on send */
    HTC_EPROTO,                   /* Protocol error */
    HTC_ENODEV,                   /* No such device */
    HTC_EDEVNOTUP,                /* device is not UP */
    HTC_NO_RESOURCE,              /* No resources for requested operation */
    HTC_HARDWARE,                 /* Hardware failure */
    HTC_PENDING,                  /* Asynchronous routine; will send up results la
ter (typically in callback) */
    HTC_EBADCHANNEL,              /* The channel cannot be used */
    HTC_DECRYPT_ERROR,            /* Decryption error */
    HTC_PHY_ERROR,                /* RX PHY error */
    HTC_CONSUMED,                 /* Object was consumed */
    HTC_CLONE,                    /* The buffer is cloned */
    HTC_USB_ERROR,                /* Rome USB Target error */
} htc_status_t;

#define HTC_SUCCESS(x)        (x == HTC_OK)
#define HTC_FAILED(x)         (!HTC_SUCCESS(x))

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

    /* standard debug print masks bits 0..7 */
#define HTC_DEBUG_ERR   (1 << 0)   /* errors */
#define HTC_DEBUG_WARN  (1 << 1)   /* warnings */
#define HTC_DEBUG_INF   (1 << 2)   /* informational (module startup info) */
#define HTC_DEBUG_TRC   (1 << 3)   /* generic function call tracing */
#define HTC_DEBUG_SEND  (1 << 4)
#define HTC_DEBUG_RECV  (1 << 5)
#define HTC_DEBUG_SYNC  (1 << 6)
#define HTC_DEBUG_DUMP  (1 << 7)
#define HTC_DEBUG_ALERT (1 << 8)


#define HTC_DEBUG_MASK_DEFAULTS  (HTC_DEBUG_ERR | HTC_DEBUG_WARN)
#define HTC_DEBUG_ANY  0xFFFF

    /* other aliases used throughout */
#define HTC_DEBUG_ERROR   HTC_DEBUG_ERR
#define HTC_LOG_ERR       HTC_DEBUG_ERR
#define HTC_LOG_INF       HTC_DEBUG_INF
#define HTC_LOG_TRC       HTC_DEBUG_TRC
#define HTC_DEBUG_TRACE   HTC_DEBUG_TRC
#define HTC_DEBUG_INIT    HTC_DEBUG_INF

    /* bits 8..31 are module-specific masks */
#define HTC_DEBUG_MODULE_MASK_SHIFT   8

    /* macro to make a module-specific masks */
#define HTC_DEBUG_MAKE_MODULE_MASK(index)  (1 << (HTC_DEBUG_MODULE_MASK_SHIFT + (index)))

#define HTC_DEBUG_PRINTBUF(buffer, length)

extern QCLI_Group_Handle_t qcli_htcdemo_hdl;
extern  OSAL_UINT32 debughtc;

#define PRINTX_ARG(arg...) arg

#define HTC_DEBUG_PRINTF(lvl, arg)\
    do {\
        if (debughtc & (lvl)) {  \
            QCLI_Printf (qcli_htcdemo_hdl, PRINTX_ARG arg); \
        } \
    } while (0)

#define HTC_DEBUG_ASSERT(test) \
    do { \
        if (0 == (test)) { \
			QAPI_FATAL_ERR(0,0,0); \
        } \
    } while (0)


#endif /* __HTC_DEFS_H__ */
