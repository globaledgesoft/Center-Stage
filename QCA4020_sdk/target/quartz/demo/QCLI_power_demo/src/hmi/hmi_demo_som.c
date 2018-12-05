/*
 * Copyright (c) 2015-2017 Qualcomm Technologies, Inc.
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

#include "string.h"

#include "pal_som.h"
#include "hmi_demo_som.h"
#include "hmi_demo_common.h"

#include "qapi_types.h"
#include "qapi_timer.h"
#include "qapi_om_smem.h"

typedef struct
{
   qapi_TIMER_handle_t Timer;
   qbool_t             Timer_Active;
} HMI_Demo_SOM_Context_t;

static HMI_Demo_SOM_Context_t HMI_Demo_SOM_Context;

static void Periodic_Send_SOM_Timer_CB(void *Param);

/**
   @brief This function handles timer callbacks for period transmission.

   @param Param is the parameter specified when the callback was registered.
          This function treats it as the device index.
*/
static void Periodic_Send_SOM_Timer_CB(void *Param)
{
   if(HMI_Demo_SOM_Context.Timer_Active)
   {
      /* Start the transition back to FOM. */
      PAL_Exit_SOM();
   }
}

/**
   @brief Initializes the HMI demo and registers its commands with QCLI.

   @param IsColdBoot is a flag indicating if this is a cold boot (true) or warm
                     boot (false).
*/
void Initialize_HMI_Demo_SOM(void)
{
   qapi_Status_t               Result;
   uint16_t                    Data_Size;
   HMI_Demo_Transition_Data_t *Transition_Data;

   union
   {
      qapi_TIMER_define_attr_t Create;
      qapi_TIMER_set_attr_t    Set;
   } Timer_Attr;

   memset(&HMI_Demo_SOM_Context, 0, sizeof(HMI_Demo_SOM_Context_t));

   /* Restore settings from transition memory. */
   Result = qapi_OMSM_Retrieve(QAPI_OMSM_DEFAULT_AON_POOL, OMSM_CLIENT_ID_HMI_DEMO_E, &Data_Size, (void **)&Transition_Data);

   if((Result == QAPI_OK) && (Transition_Data != NULL))
   {
      if((Data_Size >= sizeof(HMI_Demo_Transition_Data_t)) && (Transition_Data->Send_Info.Expire_Time != 0))
      {
         HMI_Demo_SOM_Context.Timer_Active = true;

         /* Create the timer that will be used for the periodic transmission. */
         Timer_Attr.Create.deferrable     = false;
         Timer_Attr.Create.cb_type        = QAPI_TIMER_FUNC1_CB_TYPE;
         Timer_Attr.Create.sigs_func_ptr  = (void *)Periodic_Send_SOM_Timer_CB;
         Timer_Attr.Create.sigs_mask_data = Transition_Data->Send_Info.Device_Index;
         Result = qapi_Timer_Def(&(HMI_Demo_SOM_Context.Timer), &(Timer_Attr.Create));
         if(Result == QAPI_OK)
         {
            /* Start the timer. */
            Result = qapi_Timer_set_absolute(HMI_Demo_SOM_Context.Timer, Transition_Data->Send_Info.Expire_Time, false);
            if(Result != QAPI_OK)
            {
               qapi_Timer_Undef(HMI_Demo_SOM_Context.Timer);
            }
         }

         if(Result != QAPI_OK)
         {
            HMI_Demo_SOM_Context.Timer_Active = false;
         }
      }

      Result = qapi_OMSM_Commit(QAPI_OMSM_DEFAULT_AON_POOL, OMSM_CLIENT_ID_HMI_DEMO_E);
      if(Result != QAPI_OK)
      {
         /* Unable to commit memory so just free it. */
         qapi_OMSM_Free(QAPI_OMSM_DEFAULT_AON_POOL, OMSM_CLIENT_ID_HMI_DEMO_E);

         qapi_Timer_Undef(HMI_Demo_SOM_Context.Timer);
         HMI_Demo_SOM_Context.Timer_Active = false;
      }
   }
}

/**
   @brief called when SOM mode is exited.
*/
void HMI_Demo_SOM_Exit_Mode(void)
{
}

