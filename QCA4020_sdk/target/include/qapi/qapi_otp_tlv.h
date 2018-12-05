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

#ifndef __QAPI_OTP_TLV_H__
#define __QAPI_OTP_TLV_H__

#ifdef   __cplusplus
extern "C" {
#endif

/* TODO: doxygenate */

/*
 * The OTP_TLV module supports Tag/Length/Value storage within One Time Programmable memory.
 *
 * This module allows writes, typically manufacturing, of "TLVs" to OTP
 * and run-time retrieval of TLV value by tag. TLVs are stored sequentially
 * in the order in which they are written, from low to high offsets within a
 * designated area of OTP. A given tag should appear at most once within the
 * area. If a tag appears more than once, only the first instance may be
 * retrieved. If a TLV is deleted, its tag is set to a sentinal "deleted"
 * tag value but the TLV continues to consume space in OTP. Currently tags
 * are 4-bit values:
 *   0 --> Unused TLV
 *   1..10 --> System tag
 *   11..14 --> Application tag
 *   15 --> Deleted TLV
 *
 * This module is divided into several object files. This allows APIs to
 * be included or excluded as desired. For instance, one may choose to
 * include OTP_TLV write in manufacturing firmware but not in  Mission
 * Mode firmware.
 */

#include "qapi/qapi_status.h"

/* The following definitions are the tag values used by the system. */
#define QAPI_OTP_TLV_TAG_BLE_ADDRESS            (1)  /**< OTP TLV TAG for the BLE address (BD_ADDR). */
#define QAPI_OTP_TLV_TAG_I15P4_ADDRESS          (2)  /**< OTP TLV TAG for the 15.4 address (EUI64). */
#define QAPI_OTP_TLV_TAG_XTAL_CAL  				(3)  /**< OTP TLV TAG for the share XTAL value. */

#define QAPI_OTP_TLV_TAG_APP_MIN                (11) /**< Minimum tag value available for application specific OTP tags. Tags less
                                                          than this value are reserved. */

#define QAPI_OTP_TLV_TAG_APP_MAX                (14) /**< Minimum tag value available for application specific OTP tags. Tags
                                                          greater than this value are reserved. */

#define QAPI_OTL_TLV_TAG_BLE_ADDRESS_LENGTH     (6)  /**< Length of the OTP BLE address. */
#define QAPI_OTP_TLV_TAG_I15P4_ADDRESS_LENGTH   (8)  /**< Length of the OTP 15.4 address. */


/*
 * qzpi_OTP_TLV_Init initializes the OTP_TLV module to work within a given range of OTP.
 *
 * If one or both parameters are 0, the module uses system default values.
 * If the Read, Write or Status APIs are called before this function is
 * called, the module auto-initializes to platform default values.
 *
 * Each time this API is called after the first time, it REinitializes,
 * using the new parameters.
 *
 * Returns
 *    QAPI_ERR_INVALID_PARAM if start+num_bytes is too large
 *    QAPI_OK otherwise
 */
extern qapi_Status_t qapi_OTP_TLV_Init(
        unsigned int otp_start,
        unsigned int otp_num_bytes);


/*
 * qapi_OTP_TLV_Read searches for an OTP_TLV with the given tag and return up to
 * max_length bytes in the supplied buffer. The actual length of the OTP_TLV is
 * returned in the actual_length parameter regardless of whether the actual length
 * of the OTP_TLV is larger than or smaller than max_length.
 *
 * Returns
 *    QAPI_ERR_INVALID_PARAM if the tag is not valid
 *    QAPI_ERR_NO_ENTRY if the tag is not found
 *    QAPI_OK otherwise
 */
extern qapi_Status_t qapi_OTP_TLV_Read(
        unsigned int tag,
        uint8_t *buffer,
        unsigned int max_length,
        unsigned int *actual_length);



/*
 * qapi_OTP_TLV_Find searches for an OTP_TLV with the given tag and return the
 * offset in OTP of the start of the data value along with the length of the
 * value according to the TLV header in OTP. This QAPI always finds the FIRST
 * matching tag.
 *
 * Returns
 *    QAPI_ERR_INVALID_PARAM if the tag is not valid
 *    QAPI_ERR_NO_ENTRY if the tag is not found
 *    QAPI_ERROR if the module is not Inited
 *    QAPI_OK otherwise
 *
 * If the tag IS found, pOffset is updated to the starting offset
 * of Value/Data associated with the tag. If the tag is NOT found
 * pOffset is updated to the offset where data for the NEXT OTP_TLV
 * would be placed. Note that if the OTP_TLV area is full, this may
 * be beyond the end of the OTP_TLV area.
 *
 * pLength is updated only if the tag is found. It reflects the length
 * of the OTP_TLV.
 *
 * This API is NOT intended for typical Mission Mode use; but it may
 * be useful for special situations and for debug.
 */
extern qapi_Status_t qapi_OTP_TLV_Find(
        unsigned int tag,
        unsigned int *pOffset,
        unsigned int *pLength);



/*
 * qapi_OTP_TLV_Write appends a new OTP_TLV to the OTP_TLV area. It is the caller's
 * responsibility to manage or avoid duplicate tags. This QAPI simply appends the new
 * OTP_TLV to the end of the OTP_TLV list.
 *
 * Returns
 *    QAPI_ERROR if the module is not Inited
 *    QAPI_ERR_NOT_SUPPORTED if the OTP_TLV area is currently invalid
 *    QAPI_ERR_INVALID_PARAM if the length of this OTP_TLV exceeds
 *      the size of the OTP_TLV area
 *    QAPI_ERR_NO_RESOURCE if there is insufficient remaining space in
 *      the OTP_TLV area to hold this OTP_TLV
 *    QAPI_OK if the new OTP_TLV was committed
 */
extern qapi_Status_t qapi_OTP_TLV_Write(
        unsigned int tag,
        uint8_t *buffer,
        unsigned int length);



/*
 * qapi_OTP_TLV_Delete deletes an OTP_TLV from the OTP_TLV area.
 *
 * Returns
 *    QAPI_ERR_INVALID_PARAM if the tag is not valid
 *    QAPI_ERR_NO_ENTRY if the tag was not found
 *    QAPI_OK if the tag was found the the corresponding OTP_TLV was deleted
 *
 * Deleted OTP_TLVs continue to consume space indefinitely in the OTP_TLV area.
 */
extern qapi_Status_t qapi_OTP_TLV_Delete(unsigned int tag);

/* Reflects current status of OTP_TLV area. */
struct qapi_OTP_TLV_Status_s {
    uint32_t area_start_offset; /* Offset into OTP of start of OTP_TLV area */
    uint32_t area_size;         /* Size in bytes of the OTP_TLV area */
    uint32_t bytes_consumed;    /* Bytes of OTP consumed in the OTP_TLV area */

    uint32_t num_SYS_TLV;       /* Number of SYS-tagged TLVs */
    uint32_t num_SYS_byte;      /* Number of bytes in SYS-tagged TLVs */

    uint32_t num_APP_TLV;       /* Number of APP-tagged TLVs */
    uint32_t num_APP_byte;      /* Number of bytes in APP-tagged TLVs */

    uint32_t num_DEL_TLV;       /* Number of deleted TLVs */
    uint32_t num_DEL_byte;      /* Number of bytes in deleted TLVs */
};

/* Reflects XTAL value for capin and capout. */
typedef struct OTP_TLV_xtal_cal_s {
    uint8_t capin;
    uint8_t capout;
}OTP_TLV_XTAL_CAL;

/*
 * qapi_OTP_TLV_Status returns current status about the OTP_TLV area.
 *
 * Returns
 *    QAPI_ERROR if the module is not Inited
 *    QAPI_ERR_NOT_SUPPORTED if the OTP_TLV area is currently invalid
 *                           (otp_tlv_status is updated regardless)
 *    QAPI_OK if the OTP_TLV area is in good condition
 *
 */
extern qapi_Status_t qapi_OTP_TLV_Status(struct qapi_OTP_TLV_Status_s *otp_tlv_status);

#ifdef   __cplusplus
}
#endif

#endif /* __QAPI_OTP_TLV_H__ */
