#ifndef QURT_INTERRUPT_H
#define QURT_INTERRUPT_H

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
  @file qurt_interrupt.h 
  @brief  Prototypes of QuRT Interrupt handler API 

EXTERNAL FUNCTIONS
   None.

INITIALIZATION AND SEQUENCING REQUIREMENTS
   None.

*/

#include "qurt_types.h"

/*=============================================================================
                                 CONSTANTS AND MACROS
=============================================================================*/ 
/*=============================================================================
                                    TYPEDEFS
=============================================================================*/

/** @addtogroup interrupt_types
@{ */

/**
Function pointer to an application-specified interrupt handler function.

  @param[in] n_irq   IRQ number.

  @return
  None.

  @dependencies
  None.
*/
typedef void (*qurt_interrupt_handler_func_ptr_t) ( uint32 n_irq );


/*=============================================================================
                                    FUNCTIONS
=============================================================================*/
/**
  Initial handler when an interrupt is invoked. 

  @return
  None.

  @dependencies
  None.
  
*/
void qurt_interrupt_irq_handler ( void );

/**
  Common IRQ handler that is invoked when each interrupt comes in.
  
  @param[in] qurt_interrupt_default_irq_handler_ptr Pointer to the IRQ handler. 

  @return
  None.

  @dependencies
  None.
  
*/

void qurt_interrupt_register_default_irq( qurt_interrupt_handler_func_ptr_t 
                             qurt_interrupt_default_irq_handler_ptr );

/** @} */ /* end_addtogroup interrupt_types */
						 
#endif /* QURT_INIT_H */

