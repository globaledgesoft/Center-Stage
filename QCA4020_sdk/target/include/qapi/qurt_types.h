#ifndef QURT_TYPES_H
#define QURT_TYPES_H

/*
 * Copyright (c) 2015-2018 Qualcomm Technologies, Inc.
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

@file qurt_types.h

@brief  definition of basic types, constants, preprocessor macros
*/

#include "qurt_stddef.h"

/*=============================================================================
                        CONSTANTS AND MACROS
=============================================================================*/
/** @addtogroup qurt_types
@{ */

#define QURT_TIME_NO_WAIT       0x00000000 /**< Return immediately without any waiting. */
#define QURT_TIME_WAIT_FOREVER  0xFFFFFFFF /**< Block until the operation is successful. */

/*=============================================================================
                        TYPEDEFS
=============================================================================*/

/** QuRT time types. */
typedef uint32 qurt_time_t;

/** QuRT time unit types. */
typedef enum {
  QURT_TIME_TICK,                  /**< Return time in ticks. */
  QURT_TIME_MSEC,                  /**< Return time in milliseconds. */
  QURT_TIME_NONE=0x7FFFFFF        /**< Identifier to use if no particular return type is needed. */ 
}qurt_time_unit_t;

/** @} */ /* end_addtogroup qurt_types */
#endif /* QURT_TYPES_H */
