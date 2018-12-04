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
 * @file qapi_device_info.h
 *
 * @addtogroup qapi_device_info
 * @{
 *
 * @details Contains QAPIs used to retrieve device-specific data.
 *
 * @}
 */

#ifndef _QAPI_DEVICE_INFO_H_
#define _QAPI_DEVICE_INFO_H_

#include "stdint.h"
#include "qapi/qapi_status.h"

/** @addtogroup qapi_device_info
@{ */


/**
 * @brief Device Specific Data Type
 */
typedef enum qapi_Device_Specific_Data_e {
    QAPI_DEVICE_DATA_INVALID_E,
    /**< Placeholder for the invalid device data type. */
    QAPI_DEVICE_DATA_MAKE_E,
    /**< Device make type. */
    QAPI_DEVICE_DATA_MODEL_E,
    /**< Device model type. */
    QAPI_DEVICE_DATA_VERSION_E,
    /**< Device version type. */
    QAPI_DEVICE_DATA_OEM_ID_E,
    /**< OEM ID type. */
    QAPI_DEVICE_DATA_SERIAL_NUMBER_E,
    /**< Serial Number type. */
    QAPI_DEVICE_DATA_SECURITY_STATE_E,
    /**< Security state of the device type. */
    QAPI_DEVICE_DATA_SOC_HW_VERSION_E,
    /**< SoC HW version type. */
    QAPI_DEVICE_DATA_MAX_E
    /**< Placeholder representing maximum types. */
} qapi_Device_Specific_Data_t;



/**
 * @brief Retrieve device specific data.
 *
 * @param[in]   data_Type       Type of device specific data to retrieve.
 * @param[out]  data_Buffer     Pointer to the buffer where the device specific data will be written.
 *                              If data_Buffer is NULL, this function will simply return
 *                              the size of the required data in *data_Size.  If
 *                              *data_Buffer is NULL, the function will allocate the data_Buffer,
 *                              and copy the required data into it.  If *data_Buffer is not NULL,
 *                              the function will copy the required data into it, first checking the buffer size
 *                              specified in the *data_Size parameter.  Please note that all
 *                              the data is given back in the network order.
 * @param[inout]  data_Size     Pointer to the size of the device specific data.
 *                              If *data_Buffer is not NULL, the *data_Buffer size and hence the *data_Size value
 *                              must be greater than the device specific data to be retrieved.
 *                              If data_Size is NULL, this function will return an error.
 *
 * @return QAPI_OK on success, or an error code on failure.
 */
qapi_Status_t
qapi_Core_Device_Info_Get_Specific_Data(
    qapi_Device_Specific_Data_t data_Type,
    void ** data_Buffer,
    size_t * data_Size
    );



/** @} */

#endif /* _QAPI_DEVICE_INFO_H_ */
