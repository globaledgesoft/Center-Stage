/* 
 * Copyright (c) 2011-2018 Qualcomm Technologies, Inc.
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

/**
 * @file qapi_status.h
 *
 * @brief QAPI status defintions
 *
 * @details This file defines the QAPI status type and common status
 *          values. It also defins the module IDs that can be used by
 *          modules to add additional status codes.
 */

#ifndef __QAPI_STATUS_H__ // [
#define __QAPI_STATUS_H__

#include "qapi_types.h"

/** @addtogroup qapi_status
@{ */

typedef int32_t qapi_Status_t;

/** @name Error Code Formats
 *
 * The following definitions are used to format error codes based on their
 * module. Error codes that use these macros will be a negative value of
 * the format -((10000 * <Module ID>) + <Status Code>).
 * @{
 */

#define __QAPI_ERR_MOD_OFFSET                (10000)
#define __QAPI_ERR_ENCAP_MOD_ID(__mod_id__)  ((__mod_id__) * __QAPI_ERR_MOD_OFFSET)
#define __QAPI_ERROR(__mod_id__, __err__)    (0 - (__QAPI_ERR_ENCAP_MOD_ID(__mod_id__) + (__err__)))
/** @} */ /* end namegroup */

/** @name Module IDs
 *
 * The following definitions represent the IDs for the various modules of
 * the QAPI.
 *
 * If OEMs want to added their own module IDs, it is recommended
 * to start at 100 to avoid possible conflicts with updates to the QAPI
 * that adds in additional modules.
 * @{
 */

#define QAPI_MOD_BASE                        (0)
#define QAPI_MOD_802_15_4                    (1)
#define QAPI_MOD_NETWORKING                  (2)
#define QAPI_MOD_WIFI                        (3)
#define QAPI_MOD_BT                          (4)
#define QAPI_MOD_BSP                         (5)
#define QAPI_MOD_BSP_I2C_MASTER              (6)
#define QAPI_MOD_BSP_SPI_MASTER              (7)
#define QAPI_MOD_BSP_TLMM                    (8)
#define QAPI_MOD_BSP_GPIOINT                 (9)
#define QAPI_MOD_BSP_PWM                     (10)
#define QAPI_MOD_BSP_ERR                     (11)
#define QAPI_MOD_BSP_DIAG                    (12)
#define QAPI_MOD_BSP_OM_SMEM                 (13)
#define QAPI_MOD_CRYPTO                      (14)
#define QAPI_MOD_ZIGBEE                      (15)
#define QAPI_MOD_THREAD                      (16)
/** @} */ /* end namegroup */

/** @name Common Status Codes
 *
 * The following definitions represent the status codes common to all of
 * the QAPI modules.
 * @{
 */

#define QAPI_OK                              ((qapi_Status_t)(0))                               /**< Success.                   */
#define QAPI_ERROR                           ((qapi_Status_t)(__QAPI_ERROR(QAPI_MOD_BASE,  1))) /**< General error.             */
#define QAPI_ERR_INVALID_PARAM               ((qapi_Status_t)(__QAPI_ERROR(QAPI_MOD_BASE,  2))) /**< Invalid parameter.         */
#define QAPI_ERR_NO_MEMORY                   ((qapi_Status_t)(__QAPI_ERROR(QAPI_MOD_BASE,  3))) /**< Memory allocation error.   */
#define QAPI_ERR_NO_RESOURCE                 ((qapi_Status_t)(__QAPI_ERROR(QAPI_MOD_BASE,  4))) /**< Resource allocation error. */
#define QAPI_ERR_BUSY                        ((qapi_Status_t)(__QAPI_ERROR(QAPI_MOD_BASE,  6))) /**< Opertion is busy.          */
#define QAPI_ERR_NO_ENTRY                    ((qapi_Status_t)(__QAPI_ERROR(QAPI_MOD_BASE,  7))) /**< Entry was not found.       */
#define QAPI_ERR_NOT_SUPPORTED               ((qapi_Status_t)(__QAPI_ERROR(QAPI_MOD_BASE,  8))) /**< Feature is not supported.  */
#define QAPI_ERR_TIMEOUT                     ((qapi_Status_t)(__QAPI_ERROR(QAPI_MOD_BASE,  9))) /**< Operation timed out.       */
#define QAPI_ERR_BOUNDS                      ((qapi_Status_t)(__QAPI_ERROR(QAPI_MOD_BASE, 10))) /**< Out of bounds.             */
#define QAPI_ERR_BAD_PAYLOAD                 ((qapi_Status_t)(__QAPI_ERROR(QAPI_MOD_BASE, 11))) /**< Bad payload.               */
#define QAPI_ERR_EXISTS                      ((qapi_Status_t)(__QAPI_ERROR(QAPI_MOD_BASE, 12))) /**< Entry already exists.      */
/** @} */ /* end namegroup */

/** @} */
#endif // ] #ifndef __QAPI_STATUS_H__

