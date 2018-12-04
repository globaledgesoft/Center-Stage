#ifndef __QAPI_MOM_H__
#define __QAPI_MOM_H__

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

/*=================================================================================
 *
 *                       MINIMAL OPERATING MODE OEM FUNCTIONS
 *
 *================================================================================*/
 /**
 * @file qapi_mom.h
 */

 /*
 * Minimal Operating Mode (MOM): This is the lowest power mode i,e when the processor power collapses
 * all the RAM banks (except the always ON memory) is collapsed. The periodic wake-up time 
 * can be configured by using the mom_set_wakeup_timer API. When the processor wakes up from power collapsed
 * the system can enter into any of the modes specified by OEM using wakeup_callback_register.
 *
 * @code
 *
 * 
 *
 * @endcode
 *
 */

/*==================================================================================

                           EDIT HISTORY FOR FILE

This section contains comments describing changes made to this file.
Notice that changes are listed in reverse chronological order.

$Header: //components/rel/core.ioe/1.0/api/platform/qapi_mom.h#5 $

when       who     what, where, why
--------   ---     -----------------------------------------------------------------
02/11/17   leo     (Tech Comm) Edited/added Doxygen comments and markup.
05/10/16   vj      Initial version
==================================================================================*/


/*==================================================================================

                               INCLUDE FILES

==================================================================================*/

/*==================================================================================

                                   MACROS

==================================================================================*/

/*==================================================================================

                               TYPE DEFINITIONS

==================================================================================*/

/** @addtogroup qapi_mom
@{ */


/**
  Wakeup handler callback.

  Declares the type of callback function that is to be defined by the client (OEM).
  The callback is called when the system wakes up from minimal operating mode (MOM) deep sleep.
  
  The client passes the callback function pointer and the callback context to the
  driver in the ::qapi_MOM_Wakeup_Callback_Register.

  
  @param[in] Dest_OM        Destination mode, which is registered using the
                            Register_Operating_modes call.

  @param[in] callback_Ctxt  Callback context that was passed in the call
                            to ::qapi_MOM_Wakeup_Callback_Register.

  @return
  None.

*/
typedef void (*qapi_MOM_Wakeup_Handler_CB_t)(uint32_t *Dest_OM, void *callback_Ctxt);

/** @} */ /* end_addtogroup qapi_mom */


/** @addtogroup qapi_mom
@{ */

/*=============================================================================

                      FUNCTION DECLARATIONS

=============================================================================*/

/*===========================================================================
**  FUNCTION :  qapi_MOM_Wakeup_Callback_Register
** ==========================================================================
*/
/**
* @brief
*  Enables clients to register a callback for the MOM wakeup handler.
*
* @param[in] CB Client callback function.
*
* @param[out] callback_Ctxt Pointer to the client context.
*
* @return
*  None.
* 
*/
void qapi_MOM_Wakeup_Callback_Register(qapi_MOM_Wakeup_Handler_CB_t CB, void *callback_Ctxt); 


/*===========================================================================
**  FUNCTION :  qapi_MOM_Set_Wakeup_Timer
** ==========================================================================
*/
/**
* @brief
*  Sets the MOM wakeup timer duration. 
*
* @param[in] Duration_Ticks Wakeup timer duration in ticks.
*
* @param[in] Backoff_Ticks  Backoff adjustment in ticks.
*
* @return
*  None.
* 
*/
void qapi_MOM_Set_Wakeup_Timer(uint32_t Duration_Ticks, uint32_t Backoff_Ticks);

/** @} */ /* end_addtogroup qapi_mom */

#endif /* __QAPI_MOM_H__ */
