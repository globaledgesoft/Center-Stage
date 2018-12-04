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
 * @file qapi_rtc.h
 *
 * @addtogroup qapi_Core_RTC
 * @{
 *
 * @details QAPIs to manually set and get the time in Julian format.
 *
 * @}
 */

#ifndef _QAPI_RTC_H_
#define _QAPI_RTC_H_

#include "stdint.h"

/** @addtogroup qapi_Core_RTC
@{ */

/**
 * @brief Time in Julian format.
 */
typedef struct qapi_Time_s {
    uint16_t year;   /**< Year [1980 through 2100]. */
    uint16_t month;  /**< Month of the year [1 through 12]. */
    uint16_t day;    /**< Day of the month [1 through 31]. */
    uint16_t hour;   /**< Hour of the day [0 through 23]. */
    uint16_t minute; /**< Minute of the hour [0 through 59]. */
    uint16_t second; /**< Second of the minute [0 through 59]. */
    uint16_t day_Of_Week; /**< Day of the week [0 through 6] or [Monday through Sunday]. */
} qapi_Time_t;


/**
 * @brief Gets the Julian time.
 *
 * @param[in] tm  Pointer to a buffer to contain the Julian time.
 *
 * @return QAPI_OK on success, or an code on error.
 */
qapi_Status_t qapi_Core_RTC_Get(qapi_Time_t * tm);

/**
 * @brief Sets the Julian time.
 *
 * @param[in] tm  Pointer to a buffer to contain the Julian time.
 *
 * @return QAPI_OK on success, or an code on error.
 */
qapi_Status_t qapi_Core_RTC_Set(qapi_Time_t * tm);


/**
 * @brief Gets the time in milliseconds since the GPS Epoch.
 *
 * @param[in] ms  Pointer to a uint64_t to contain the GPS Epoch time in ms.
 *
 * @return QAPI_OK on success, or a different code on error.
 */
qapi_Status_t qapi_Core_RTC_GPS_Epoch_Get(uint64_t *ms);

/** @} */

#endif /* _QAPI_RTC_H_ */
