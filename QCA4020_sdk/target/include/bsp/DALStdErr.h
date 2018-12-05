#ifndef DALSTDERR_H
#define DALSTDERR_H

/*
 * Copyright (c) 2015,2018 Qualcomm Technologies, Inc.
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

/*==================================================================================

                             EDIT HISTORY FOR FILE

This section contains comments describing changes made to this file. Notice that
changes are listed in reverse chronological order.

$Header: //components/rel/core.ioe/1.0/v2/rom/release/api/dal/DALStdErr.h#2 $
==============================================================================*/

/*-----------------------------------------------------------------------
** Defined Values for DALResult
**-----------------------------------------------------------------------*/

#define  DAL_SUCCESS                      0  // No error

// all standard DAL error codes are negative and have unique values
#define  DAL_ERROR                        -1  // General failure
#define  DAL_ERROR_TIMEOUT                -2
#define  DAL_ERROR_INTERNAL               -3
#define  DAL_ERROR_INVALID_HANDLE         -20
#define  DAL_ERROR_INVALID_POINTER        -21
#define  DAL_ERROR_INVALID_ADDRESS        -22
#define  DAL_ERROR_INVALID_DATA           -23
#define  DAL_ERROR_INVALID_PARAMETER      -24
#define  DAL_ERROR_INVALID_TYPE           -25
#define  DAL_ERROR_NOT_INITIALIZED        -40
#define  DAL_ERROR_NOT_FOUND              -41
#define  DAL_ERROR_NOT_SUPPORTED          -42
#define  DAL_ERROR_NOT_ALLOWED            -43
#define  DAL_ERROR_INSUFFICIENT_MEMORY    -60
#define  DAL_ERROR_DEVICE_ACCESS_DENIED   -80
#define  DAL_ERROR_PORT_CONN_BROKEN       -81
#define  DAL_ERROR_DEVICE_NOT_FOUND       -82
#define  DAL_ERROR_DDI_VERSION            -83
#define  DAL_ERROR_OUT_OF_RANGE_PARAMETER -100
#define  DAL_ERROR_BUSY_RESOURCE          -120

// driver error codes are interface specific and 
// codes from different interfaces may have the same value
// driver error codes must be equal or less than the offset below
#define  DAL_DRIVER_ERROR_OFFSET          -1000

// all positive DAL result codes are interface specific and 
// do not necessarily have unique values 
// NOTE: DAL_INTERRUPT_SET uses the same value as DAL_SUCCESS because this is a carry
// over from the legacy DALInt driver which returns 0 when an interrupt fires and -1
// when not. The following definition is used to maintain backward compatibility.
#define  DAL_INTERRUPT_SET                DAL_SUCCESS  // Interrupt is set
#define  DAL_WORK_LOOP_EXIT               1  // workloop termination
#define  DAL_SYNC_ENTER_FAILED            3

// evaluate if the result is an error
#define  DAL_ERROR_RETURNED(result)       ((result) < 0)
// evaluate if the result is not an error
#define  DAL_NO_ERROR_RETURNED(result)    (!DAL_ERROR_RETURNED(result))

#endif /* #ifndef DALSTDERR_H */
