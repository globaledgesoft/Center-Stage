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

/**
 * @file qapi_reset.h
 *
 * @addtogroup qapi_reset
 * @{
 *
 * @details QAPIs to support system reset functionality.
 *
 * @}
 */

#ifndef _QAPI_RESET_H_
#define _QAPI_RESET_H_

/** @addtogroup qapi_reset
@{ */

/**
* @brief System reset reasons.
*
*/

typedef enum
{
    BOOT_REASON_PON_COLD_BOOT,  /**< Power on cold boot or hardware reset. */
    BOOT_REASON_WATCHDOG_BOOT,  /**< Reset from the Watchdog. */
    BOOT_REASON_SW_COLD_BOOT,   /**< Software cold reset. */
    BOOT_REASON_UNKNOWN_BOOT,   /**< Unknown software cold boot. */
    BOOT_REASON_SIZE_ENUM  = 0x7FFFFFFF   /* Size enum to 32 bits. */
} qapi_System_Reset_Result_t;


/**
 * @brief Resets the system. This API does not return.
 *
 * @return 
 * None.
 */
void qapi_System_Reset(void);

/**
 * @brief Resets the Watchdog counter. If the watchdog count exceeds 3, 
 *        the bootloader will attempt to load the Golden image, if present.
 *
 * @return
 * None. 
 */
void qapi_System_WDTCount_Reset(void);
/**
 * @brief provides the boot or previous reset reason
 *
 * @param[out] reason returns the previous reset reason/cause of current boot
 *
 * @return status QAPI_OK on successful reset reason retrieval
 *         otherwise appropriate error.
 */
qapi_Status_t qapi_System_Get_BootReason(qapi_System_Reset_Result_t *reason);
/** @} */

#endif /* _QAPI_RESET_H_ */
