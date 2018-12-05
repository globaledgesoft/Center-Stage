#ifndef QURT_INIT_H
#define QURT_INIT_H

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
  @file qurt_init.h 
  @brief  Prototypes of QuRT Initialization API 

EXTERNAL FUNCTIONS
   None.

INITIALIZATION AND SEQUENCING REQUIREMENTS
   None.
*/

#include "qurt_types.h"

/*=============================================================================
                                 CONSTANTS AND MACROS
=============================================================================*/ 

#define QURT_INFO_OBJ_SIZE_BYTES    64

/*=============================================================================
                                    TYPEDEFS
=============================================================================*/
/** @addtogroup init_data_types
@{ */

/** QuRT data structure. */
typedef struct qurt_data_s
{
  void *hPtr;
  /**< Pointer to the heap used by the RTOS. */
  void *rPtr;
  /**< Reserved pointer for future use. */
} qurt_data_t;

/** QuRT information type. */ /* 8 byte aligned */
typedef struct qurt_info
{  
   unsigned long long _bSpace[QURT_INFO_OBJ_SIZE_BYTES/sizeof(unsigned long long)];
   /**< Opaque OS object accessible via attribute accessor APIs. */
}qurt_info_t;



/*=============================================================================
                                    FUNCTIONS
=============================================================================*/
/**
  Initializes the RTOS and QuRT libraries. The data passed by the platform to QuRT depends
  on the underlying RTOS. 
  
  @param[in,out] arg Pointer to the initialization parameter.

  @return
  None.

  @dependencies
  None.
  
*/
int qurt_init (void *arg);

/** @} */ /* end_init data_types */

#endif /* QURT_INIT_H */

