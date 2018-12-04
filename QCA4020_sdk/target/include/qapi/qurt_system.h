#ifndef QURT_SYSTEM_H
#define QURT_SYSTEM_H

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
  @file qurt_system.h 
  @brief  Prototypes of QuRT System level functionality API

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

/*=============================================================================
                                    FUNCTIONS
=============================================================================*/

/** @addtogroup qurt_system
@{ */

/**
  BSP hook that is to be set if Low Power mode is intended.
  
  @param[in] entrypoint Pointer to the BSP function.

  @return
  None.

  @dependencies
  None.
  
*/
void qurt_power_register_idle_hook(void (*entrypoint) (uint32));

/**
  BSP hook that must be set before qurt_init is invoked
  
  @param[in] entrypoint Pointer to the BSP initialization function.

  @return
  None.

  @dependencies
  None.
  
*/
void qurt_system_register_bsp_hook(void (*entrypoint) (void));

/**  
  Triggers a power collapse.

  This function performs the following power collapse sequence: 
  - 1. Checks for the power collapse type for Low Power mode (light or deep sleep). \n
  - 2. Saves the CPU and peripheral context. \n
  - 3. Raises a WFI.
  
  If an interrupt is pending, the API immediately returns to the caller. \n
  The API also returns immediately in the case of an error condition.

  @param[in] type   Sleep type. Values:
                    - QURT_POWER_SHUTDOWN_TYPE_LIGHTSLEEP
                    - QURT_POWER_SHUTDOWN_TYPE_DEEPSLEEP @tablebulletend
                      
  @param[in] pcontext Pointer to the region where the system context is to be 
                        saved.
  
  @param[in] context_sz  Size of context in bytes.
                        
  @return
  QURT_POWER_SHUTDOWN_EXIT_IPEND -- Power collapse failed because of a pending interrupt. \n
  QURT_POWER_SHUTDOWN_EXIT_LIGHTSLEEP -- Operation was successfully performed. 
                                         This value indicates that the processor has
                                         returned from power collapse light sleep mode. \n
  QURT_POWER_SHUTDOWN_EXIT_DEEPSLEEP  -- Operation was successfully performed. 
                                         This value indicates that the processor has
                                         returned from power collapse deep sleep mode.
  
  @dependencies
  None.
  
*/
int qurt_power_shutdown_enter (uint32 type, void *pcontext, uint32 context_sz);

/**
  Undoes state changes made in preparing for a power collapse.

  This function unmasks the global interrupts.
 
  @param[in] pcontext  Pointer to the region from which the system 
                       context was to be restored.
                        
  @return
  None.

  @dependencies
  None.
  
*/
void qurt_power_shutdown_exit (void *pcontext); 

/**
Gets the idle time duration remaining for threads created in the system.\n
 
  @param[in]  void
                        
  @return
  Number of system ticks

  @dependencies
  None.
  
*/
uint32 qurt_system_get_idle_time( void );

/** @} */ /* end_addtogroup qurt_system */
				 
#endif /* QURT_SYSTEM_H */

