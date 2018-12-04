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
 * @file qapi_ver.h
 * @addtogroup qapi_version 
 * @{
 * @brief QAPI version information
 *
 * @details This file provides the version information for the QAPI.
 * @}  
 */

#ifndef __QAPI_VER_H__ // [
#define __QAPI_VER_H__

#include "qapi_status.h"

/** @addtogroup qapi_build_info
@{ */

#define QAPI_VERSION_MAJOR                                     (2)
#define QAPI_VERSION_MINOR                                     (0)
#define QAPI_VERSION_NIT                                       (1)

#define __QAPI_VERSION_MAJOR_MASK                             (0xff000000)
#define __QAPI_VERSION_MINOR_MASK                             (0x00ff0000)
#define __QAPI_VERSION_NIT_MASK                               (0x0000ffff)

#define __QAPI_VERSION_MAJOR_SHIFT                             (24)
#define __QAPI_VERSION_MINOR_SHIFT                             (16)
#define __QAPI_VERSION_NIT_SHIFT                               (0)

#define __QAPI_ENCODE_VERSION(__major__, __minor__, __nit__)   (((__major__) << __QAPI_VERSION_MAJOR_SHIFT) | \
                                                                ((__minor__) << __QAPI_VERSION_MINOR_SHIFT) | \
                                                                ((__nit__)   << __QAPI_VERSION_NIT_SHIFT))

/*----------------------------------------------------------------------------------
 * Type Declarations
 * -------------------------------------------------------------------------------*/
/**
 * Data structure used by application to get build information.
 */
typedef struct {
    uint32_t qapi_Version_Number;
    /**< qapi version number */
    uint32_t crm_Build_Number;
    /**< CRM build number */
} qapi_FW_Info_t;

/*----------------------------------------------------------------------------------
 * Function Declarations and Documentation
 * -------------------------------------------------------------------------------*/
/**
 * This API allows user to retrieve version information from system. \n
 *
 * @param[out]    info          Value retrieved from system.
 *
 * @return        QAPI_OK -- Requested parameter retrieved from the system. \n
 *                Non-Zero value -- Parameter retrieval failed.
 *  
 * @dependencies          None.
 */
qapi_Status_t qapi_Get_FW_Info(qapi_FW_Info_t *info);

//const uint32_t qapi_Version_Number = __QAPI_ENCODE_VERSION(QAPI_VERSION_MAJOR, QAPI_VERSION_MINOR, QAPI_VERSION_NIT);
/** @} */
#endif // ] #ifndef __QAPI_VER_H__

