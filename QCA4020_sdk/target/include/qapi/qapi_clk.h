#ifndef __QAPI_CLK_H__
#define __QAPI_CLK_H__

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


/*=================================================================================
 *
 *                       CLOCK QAPI
 *
 *================================================================================*/
 /**
 * @file qapi_clk.h
 */

/*==================================================================================

                           EDIT HISTORY FOR FILE

This section contains comments describing changes made to this file.
Notice that changes are listed in reverse chronological order.

$Header: //components/rel/core.ioe/1.0/v2/rom/release/api/systemdrivers/clock/qapi_clk.h#5 $

when       who     what, where, why
--------   ---     -----------------------------------------------------------------
01/30/17   vph     Changed frequency level to performance index.
01/21/16   vph      Initial version
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

/** @addtogroup qapi_clk
@{ */


/*=============================================================================

                      FUNCTION DECLARATIONS

=============================================================================*/
/*=============================================================================
  FUNCTION      qapi_CLK_Set_AP_Frequency
==============================================================================*/

/**
* Sets the application processor (AP) frequency based on the performance index
* range from (0 to 255) for power saving. Clients can switch to a higher 
* frequency mode when more performance is required, but it then consumes more power.
*
* Depending on the AP frequency levels supported, the clock driver takes a maximum 
* index of 255 divided by the number of levels and then sets the frequency according to that.
*
* For example, if the AP has four frequency levels (Economy, Low, Medium, and High),
*  Index:   0 to  63 -- Economy
*          64 to 127 -- Low
*         128 to 191 -- Medium
*         192 to 255 -- High
*
* @param[in] nPerfIndex - Performance index.
 *  
 * @return
 * QAPI_OK    -- Frequency level is supported. APP frequency setting success \n
 * QAPI_ERROR -- Frequency setting failure.
 */

qapi_Status_t qapi_CLK_Set_AP_Frequency
(
    uint8 nPerfIndex
);

/** @} */ /* end_addtogroup qapi_clk */

#endif /* __QAPI_CLK_H__ */
