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

/*======================================================================
                     mom_lp_test.c

GENERAL DESCRIPTION
  This file implements the OM transition demo tests for MOM. The RO/RW/ZI
  from this compile should be placed in MOM memory banks

 EXTERNALIZED FUNCTIONS


 INITIALIZATION AND SEQUENCING REQUIREMENTS

 ======================================================================*/
/*======================================================================
 *
 *                       EDIT HISTORY FOR FILE
 *
 *   This section contains comments describing changes made to the
 *   module. Notice that changes are listed in reverse chronological
 *   order.
 *
 *
 *
 *
 * when         who     what, where, why
 * ----------   ---     ------------------------------------------------
 * 05/03/2016   sj      Initial Version
 ======================================================================*/

 /*======================================================================
                          INCLUDES
 ======================================================================*/
#include "stdlib.h"
#include "qapi.h"
#include "qapi_omtm.h"
#include "qapi_om_smem.h"
#include "om_lp_test.h"
#include "stdint.h"
#include "qapi_gpioint.h"


#include "qcli.h"
#include "qcli_api.h"

/*======================================================================
                          EXTERNAL
 ======================================================================*/



/*======================================================================
                          GLOBALS
 ======================================================================*/

/*  Test Code */
uint32_t mom_oem_counter=0;

void mom_oem_wakeup_handler(uint32_t *dest_om, void *data)
{

#ifdef V1
  /* Register OMTM operating mode table during MOM init
  */
  qapi_OMTM_MOM_Register_Operating_Modes((qapi_OMTM_Operating_Mode_t*)&omtm_operating_mode_tbl_sram, 3, 1);
#endif

  (*((uint32_t *)data))++;
}



#if 0
timer_type som_test_timers[SOM_TEST_MAX_TIMERS];
timer_def_attribute_type som_timer_def_attr[SOM_TEST_MAX_TIMERS];
timer_set_attribute_type som_timer_set_attr[SOM_TEST_MAX_TIMERS];
timetick_type64 timer_set_time[SOM_TEST_MAX_TIMERS] = {300, 400, 500};
time_osal_notify_obj_ptr som_test_timer_cb[SOM_TEST_MAX_TIMERS] = {som_timer1_cb, som_timer2_cb, som_timer3_cb};

dsr_obj_t *som_test_dsr[SOM_TEST_MAX_DSRS];
dsr_priority_t dsr_prio[SOM_TEST_MAX_DSRS];
uint32_t som_dsr_init_data = 0;
som_test_app_info_t som_test_app_info;
boolean som_test_app_force_exit = FALSE;

#ifdef FEATURE_GPIO_CORE_TEST
gpio_test_t gpio_test_info;
#endif
som_test_ctxt_t som_test_ctxt[] =
{
  {0, &som_test_timers[0]},
  {1, &som_test_timers[1]},
  {2, &som_test_timers[2]}
};

/*======================================================================
                          FUNCTIONS
 ======================================================================*/


void som_dsr_default_handler(dsr_obj_t *dsr_obj, void *ctxt, void *payload)
{
  som_test_app_info.som_dsr_count[((som_test_ctxt_t *)ctxt)->dsr_cnt_idx]++;
  timer_set(((som_test_ctxt_t*)ctxt)->timer, (timer_set_attribute_type*)payload);
}

#ifdef FEATURE_GPIO_CORE_TEST
void som_dsr_gpio_handler(dsr_obj_t *dsr_obj, void *ctxt, void *payload)
{
  som_test_app_info.som_dsr_count[((som_test_ctxt_t *)ctxt)->dsr_cnt_idx]++;
  ASSERT( GPIOINT_SUCCESS == GPIOInt_TriggerInterrupt(SOM_TEST_GPIO_NUM) );
}
#endif

void som_timer1_cb(uint32_t data)
{
  som_test_app_info.som_timer_count[0]++;
  dsr_enqueue(som_test_dsr[0],(void*)data);
}

void som_timer2_cb(uint32_t data)
{
  som_test_app_info.som_timer_count[1]++;
  dsr_enqueue(som_test_dsr[1],(void*)data);
}

void som_timer3_cb(uint32_t data)
{
  som_test_app_info.som_timer_count[2]++;
  dsr_enqueue(som_test_dsr[2],(void*)data);
}

#ifdef FEATURE_GPIO_CORE_TEST
void fom_test_gpio_isr(void *data)
{
  gpio_test_t *gpio_ptr = (gpio_test_t *)data;
  gpio_ptr->fom_gpio_cnt++;
  gpio_ptr->gpio_event_pending = 0;
}

void som_test_gpio_isr(void *data)
{
  gpio_test_t *gpio_ptr = (gpio_test_t *)data;
  gpio_ptr->som_gpio_cnt++;
  gpio_ptr->gpio_event_pending = 1;
  GPIOInt_Disable(SOM_TEST_GPIO_NUM);
  GPIOInt_TriggerInterrupt(SOM_TEST_GPIO_NUM);
}
#endif


void som_app_init(void)
{

#if 0
  /* 1. Create 4 timers each with a different callback.
   * 2. Each timer callback will trigger a DSR of different priorities (LOW, MED, HIGH)
   * 3. Two out of the four DSRs will increment the DSR counter and set the timer to run again.
   * 4. Register GPIO Interrupt to test interrupt replay logic
   */
#ifdef FEATURE_GPIO_CORE_TEST
  uint32_t nFlags, nRet;
#endif
  uint8 i;

  memset(&som_test_app_info,0,(size_t)sizeof(som_test_app_info));

#ifdef FEATURE_GPIO_CORE_TEST
  /* Register GPIO interrupt */
  nFlags = GPIOINT_FLAG_CFG(GPIOINT_TRIGGER_RISING_EDGE, GPIOINT_PRIO_HIGH, GPIOINT_NMI_FALSE);
  nRet = GPIOInt_RegisterInterrupt(SOM_TEST_GPIO_NUM, som_test_gpio_isr, (void*)&gpio_test_info, nFlags);
#endif
  for(i=0; i<SOM_TEST_MAX_DSRS - 1; i++)
  {
    dsr_create(&som_test_dsr[i], som_dsr_default_handler, &som_test_ctxt[i], dsr_prio[i]);
  }

#ifdef FEATURE_GPIO_CORE_TEST
  /* Define DSR for GPIO Interrupt Handling */
  dsr_create(&som_test_dsr[SOM_TEST_MAX_DSRS-1], som_dsr_gpio_handler, &som_test_ctxt[SOM_TEST_MAX_DSRS-1], DSR_PRI_HIGH);
#endif

  /* define and set timers  */
  for(i=0; i<SOM_TEST_MAX_TIMERS; i++)
  {
    som_timer_def_attr[i].cb_type = TIMER_FUNC1_CB_TYPE;
    som_timer_def_attr[i].deferrable = FALSE;
    som_timer_def_attr[i].sigs_func_ptr = som_test_timer_cb[i];
    som_timer_def_attr[i].sigs_mask_data = (time_osal_notify_data)&som_timer_set_attr[i];

    som_timer_set_attr[i].time = timer_set_time[i];
    som_timer_set_attr[i].unit = T_MSEC;
    som_timer_set_attr[i].reload = FALSE;
    som_timer_set_attr[i].max_deferrable_timeout = 0;

    ASSERT( TE_SUCCESS == timer_def(&som_test_timers[i], &som_timer_def_attr[i]) );
    ASSERT( TE_SUCCESS == timer_set(&som_test_timers[i], &som_timer_set_attr[i]) );
  }
  som_test_app_info.som_app_init_done = 1;
#endif
}

#if 0
void som_app_exit(void)
{
  static boolean dest_mode = 0;
  uint8 i;

  if(!som_test_app_info.som_app_exit_done)
  {
    for(i=0; i<SOM_TEST_MAX_TIMERS; i++)
    {
      ASSERT( TE_SUCCESS == timer_undef(&som_test_timers[i]) );
    }
    for(i=0; i<SOM_TEST_MAX_TIMERS; i++)
    {
      dsr_destroy(som_test_dsr[i]);
    }
    som_test_app_info.som_app_exit_done = 1;

    om_transition_test();
  }
}

#endif


void som_app_entry(void)
{

#if 0
  /* Application can run any algorithm or processing it wants here */
  /* Here we set a dummy criteria for exiting the application which involves OM transition */
  if ((som_test_app_info.som_dsr_count[0]>= 3) &&
      (som_test_app_info.som_dsr_count[1]>= 3)
#ifdef FEATURE_GPIO_CORE_TEST
      &&(gpio_test_info.gpio_event_pending)
#endif
      ||som_test_app_force_exit == TRUE)
  {
    som_app_exit();
  }

#endif
}


om_test_map_t test_map[] = {
                            {0,1}, /* FOM_SOM */
                            {0,1}, /* FOM_SOM */
                            {0,1} /* FOM_SOM */
                            //{0,2}, /* FOM_MOM */
                            //{1,2} /* SOM_MOM */
						   };


#endif

