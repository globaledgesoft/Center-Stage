/*
 * Copyright (c) 2015 Qualcomm Atheros, Inc.
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

/*-------------------------------------------------------------------------
 * Include Files
 *-----------------------------------------------------------------------*/

#include "pal_som.h"
#include "hmi_demo_som.h"

#include "qapi/qapi.h"
#include "qapi/qapi_types.h"
#include "qapi/qapi_status.h"
#include "qapi/qapi_omtm.h"
#include "qapi/qapi_om_smem.h"
#include "qapi/qapi_timer.h"

#include <string.h>

extern void platform_Entry(void);
extern void som_main(void);
extern void mom_main(void);

//xxx debug for disabling deep sleep
#include "qapi_slp.h"

#ifdef V2
//xxx uncomment to automatically exit SOM after a certain number of loops
//#define USE_SOM_EXIT_COUNTDOWN
#endif

/*-------------------------------------------------------------------------
 * Type Declarations
 *-----------------------------------------------------------------------*/

typedef void (*Platform_Function_t)(void);

/*-------------------------------------------------------------------------
 * Static & global Variable Declarations
 *-----------------------------------------------------------------------*/

static qbool_t PAL_Flag_Exit_SOM;
static qapi_TIMER_handle_t PAL_SOM_Timer_Handle;

/*-------------------------------------------------------------------------
 * Function Definitions
 *-----------------------------------------------------------------------*/

static void PAL_SOM_Init_Wakeup_Timer(void);
static void PAL_SOM_Initialize(void);
static void PAL_SOM_Exit(void);
static void PAL_SOM_Timer_CB(void *Param);

Platform_Function_t init_sensormode_functions[]   = {PAL_SOM_Initialize, NULL};
Platform_Function_t deinit_sensormode_functions[] = {PAL_SOM_Exit,       NULL};

#ifdef USE_SOM_EXIT_COUNTDOWN
//xxx
static int som_ctdn;
#endif

static void PAL_SOM_Init_Wakeup_Timer(void)
{
   qapi_Status_t  Result;
   uint32_t      *Wakeup_Seconds;
   uint16_t       Data_Size;

   union
   {
      qapi_TIMER_define_attr_t Def;
      qapi_TIMER_set_attr_t    Set;
   } Timer;

   Result = qapi_OMSM_Retrieve(QAPI_OMSM_DEFAULT_AON_POOL, OMSM_CLIENT_ID_PAL_E, &Data_Size, (void **)&Wakeup_Seconds);

   if((Result == QAPI_OK) && (Data_Size == sizeof(uint32_t)))
   {

      /* Define the timer. */
      memset(&(Timer.Def), 0, sizeof(Timer.Def));
      Timer.Def.deferrable = false;
      Timer.Def.cb_type = QAPI_TIMER_FUNC1_CB_TYPE;
      Timer.Def.sigs_func_ptr = PAL_SOM_Timer_CB;
      Timer.Def.sigs_mask_data = 0;

      Result = qapi_Timer_Def(&PAL_SOM_Timer_Handle, &(Timer.Def));
      if(Result == QAPI_OK)
      {
         /* Start the timer. */
         memset(&(Timer.Set), 0, sizeof(Timer.Set));
         Timer.Set.time = *Wakeup_Seconds;
         Timer.Set.reload = false;
         Timer.Set.unit = QAPI_TIMER_UNIT_SEC;

         Result = qapi_Timer_Set(PAL_SOM_Timer_Handle, &Timer.Set);
         if(Result != QAPI_OK)
         {
            qapi_Timer_Undef(PAL_SOM_Timer_Handle);
         }
      }

      qapi_OMSM_Free(QAPI_OMSM_DEFAULT_AON_POOL, OMSM_CLIENT_ID_PAL_E);
   }
}

/**
   @brief Function to initailize the QCLI Demo in SOM mode.
*/
static void PAL_SOM_Initialize(void)
{
#ifdef USE_SOM_EXIT_COUNTDOWN
   som_ctdn = 40000;
#endif

   PAL_Flag_Exit_SOM = false;

   /* Register the SOM operating modes. */
   qapi_OMTM_Register_Operating_Modes(omtm_operating_mode_tbl_sram, 3, 1);

   /* Initialize the demos for SOM mode. */
   Initialize_HMI_Demo_SOM();

   PAL_SOM_Init_Wakeup_Timer();
}

/**
   @brief Function to indicate a mode exit from SOM mode.
*/
static void PAL_SOM_Exit(void)
{
   HMI_Demo_SOM_Exit_Mode();
}

static void PAL_SOM_Timer_CB(void *Param)
{
   PAL_Flag_Exit_SOM = true;
}


/**
   @brief Function called in idle task loop.
*/
void som_app_entry(void)
{

#ifdef USE_SOM_EXIT_COUNTDOWN
   if(som_ctdn == 0)
      PAL_Flag_Exit_SOM = true;
#endif

   if(PAL_Flag_Exit_SOM)
   {
      qapi_OMTM_Switch_Operating_Mode((uint32_t)OPERATING_MODE_FOM_E, QAPI_OMTM_SWITCH_NOW_E);
   }

#ifdef USE_SOM_EXIT_COUNTDOWN
   som_ctdn -= 1;
#endif

}

/**
   @brief This function tells the PAL layer to back to the FOM operation mode.
*/
void PAL_Exit_SOM(void)
{
   PAL_Flag_Exit_SOM = true;
}

