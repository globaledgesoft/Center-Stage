/*
 * Copyright (c) 2016-2018 Qualcomm Technologies, Inc.
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
* @file qapi_ns_suspend_resume.h
*
*/

#ifndef _QAPI_NS_SUSPEND_RESUME_H_
#define _QAPI_NS_SUSPEND_RESUME_H_

#include "stdint.h"

/** @addtogroup qapi_networking_strrcl
@{ */

/**
* @brief Prepare function to calculate total size required by all registered clients.
*
* @details This function is called before intiating a suspend to precalculate the total size required by all clients.
*
* @param[in] *size: Pointer to where the total size is updated.
*
* @return
* 0 if the operation succeeded, -1 otherwise.
*/
int32_t qapi_Net_Suspend_Resume_Prepare(uint32_t *size);

/**
* @brief API to trigger suspend/resume.
*
* @param[in] type Type: 1 -- Suspend; 0 -- Resume.
* @param[in] mem_Pool_ID  SMEM pool region where the data is stored.
*
* @return
* 0 if the operation succeeded, -1 otherwise.
*/
int32_t qapi_Net_Suspend_Resume_Trigger(uint8_t type, uint32_t mem_Pool_ID);

/** @} */
#endif /* _QAPI_NS_SUSPEND_RESUME_H_ */
