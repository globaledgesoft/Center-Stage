#ifndef DALSYS_H
#define DALSYS_H

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

$Header: //components/rel/core.ioe/1.0/v2/rom/release/api/dal/DALSys.h#2 $
==============================================================================*/

/*------------------------------------------------------------------------------
* Include Files
*-----------------------------------------------------------------------------*/
#include "com_dtypes.h"
#include "DALSysTypes.h"
#include "string.h"

/*------------------------------------------------------------------------------
* Function declaration and documentation
*-----------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif /* #ifdef __cplusplus */

/*
  @brief Initialize/Register Module with the DALSYS library

  DAL Modules must invoke this API prior to any DALSYS usage.

  @param pCfg: DALSYS config struct pointer

  @return None
*/
void
DALSYS_InitMod(DALSYSConfig * pCfg);

/*
  @brief De-Initialize/De-Register Module with the DALSYS library

  DAL Modules can use this API once done using DALSYS.

  @param None
  @return None
*/
void
DALSYS_DeInitMod(void);

/**
  @brief Get the DAL Properties handle

  User(s)/client(s) of DAL drivers must use this API to get the DAL Properties
  Handle

  @param DeviceId   : Desired Device Id
       phDALProps : Pointer to DALPropertyHandle, only valid if return code is
                 DAL_SUCCESS
*
* @return Return Code, DAL_SUCCESS on successful completion, error code otherwise
*/

DALResult
DALSYS_GetDALPropertyHandle(DALDEVICEID DeviceId,DALSYSPropertyHandle hDALProps);

/*
  @brief Get the Property Value. User must pass a ptr to the PropVariable.
  The User must also initialize the name field, this API will populate the
  name and value. The value is a "union" and must be used depending upon
  the appropriate type.

  @param hDALProps   : DAL Propery Handle
       pszName     : Prop Name ( if searching for uint32 name, this MUST be
                                 set to NULL)
        dwId        : Prop Id   ( integer prop name)
        pDALProp    : Ptr to the Property variable

  @return Return Code, DAL_SUCCESS on successful completion, error code otherwise
*/
DALResult
DALSYS_GetPropertyValue(DALSYSPropertyHandle hDALProps, const char *pszName,
                  uint32 dwId,
                   DALSYSPropertyVar *pDALPropVar);

#ifdef __cplusplus
}
#endif /* #ifdef __cplusplus */

#endif /* DALSYS_H */
