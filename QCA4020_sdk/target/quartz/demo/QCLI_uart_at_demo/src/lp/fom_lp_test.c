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
                     fom_lp_test.c

GENERAL DESCRIPTION
  This file implements the OM transition demo tests for FOM. The RO/RW/ZI
  from this compile should be placed in FOM memory banks

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
#include "stringl.h"
#include "qapi_types.h"
#include "qapi_status.h"
#include "qapi.h"
#include "qapi_omtm.h"
#include "qapi_om_smem.h"
#include "om_lp_test.h"
#include "stdint.h"
#include "qapi_gpioint.h"
#include "qapi_timer.h"
#include "qurt_types.h"
#include "pal.h"

/*======================================================================
                          EXTERNAL
 ======================================================================*/

extern boolean g_sleepAllowLowPowerModes;


/*======================================================================
                          GLOBALS
 ======================================================================*/

om_test_map_t test_map[] = {
                            {0,1}, /* FOM_SOM */
                            {0,2}, /* FOM_SOM */
                            {0,1}  /* FOM_SOM */
                           };
qapi_TIMER_handle_t lp_timer_handle;
qapi_TIMER_handle_t lp_uart_timer_handle;

/*======================================================================
                          FUNCTIONS
 ======================================================================*/

void om_transition_test_cb(uint32_t dest_mode_id, void *data)
{
  uint32_t *count = (uint32_t *)data;
  (*count)++;
}

void initialize_test_vectors(om_test_info_t *om_test_ptr)
{
  int i;
  memset(om_test_ptr, 0, sizeof(om_test_info_t));
  om_test_ptr->test_initialized = TEST_INITIALIZED_ID;
  om_test_ptr->vector_cnt = 0;
  for(i=0; i<MAX_OM_TRANSITION_TYPES; i++)
  {
    om_test_ptr->test_map[i].curr_om = test_map[i].curr_om;
    om_test_ptr->test_map[i].dest_om = test_map[i].dest_om;
  }
}

qbool_t reset_OM_Test_Vectors(void)
{
  qapi_OMSM_alloc_status_t alloc_status;
  om_test_info_t *om_test_ptr = NULL;
  uint16_t buff_size;


  /* Check if test vectors are allocated */
  qapi_OMSM_Check_Status(QAPI_OMSM_DEFAULT_AON_POOL, OM_SMEM_OM_TEST_CLIENT_ID, &alloc_status);

  if (alloc_status == QAPI_OMSM_BUF_ALLOC_ERROR )
  {
    QCLI_Printf("Reset_OM_Test_Vectors: not allocated \n");
    return false;
  }

  /* TODO::Need to retrieve before freeing test vectors */
  if ( QAPI_OK != qapi_OMSM_Retrieve(QAPI_OMSM_DEFAULT_AON_POOL, OM_SMEM_OM_TEST_CLIENT_ID, &buff_size, (void**)&om_test_ptr) )
  {
    QCLI_Printf("Reset_OM_Test_Vectors: retrieve failed\n");
    return false;
  }

  /* TODO::free the test vectors */
  if ( QAPI_OK != qapi_OMSM_Free(QAPI_OMSM_DEFAULT_AON_POOL, OM_SMEM_OM_TEST_CLIENT_ID) )
  {
    QCLI_Printf("Reset_OM_Test_Vectors: free failed\n");
    return false;
  }

  /* Successful */
  QCLI_Printf("Reset_OM_Test_Vectors: done\n");
  return true;
}

om_transition_status_e om_transition_test(void)
{
  om_test_info_t *om_test_ptr = NULL;
  uint32_t dest_om;
  qapi_OMSM_alloc_status_t alloc_status;
  uint16_t buff_size;
  om_transition_status_e status = FAIL;

  qapi_OMSM_Check_Status(QAPI_OMSM_DEFAULT_AON_POOL, OM_SMEM_OM_TEST_CLIENT_ID, &alloc_status);

  //don't panic that it returns "ALLOC_ERROR", this is by design (need to check with Nirmal to change return code)
  if (alloc_status == QAPI_OMSM_BUF_COMMITTED_E)
  {
    if ( QAPI_OK != qapi_OMSM_Retrieve(QAPI_OMSM_DEFAULT_AON_POOL, OM_SMEM_OM_TEST_CLIENT_ID, &buff_size, (void**)&om_test_ptr) )
    {
      QCLI_Printf("Issue with OMSM_Retrieve\n");
      return false;
    }
    /* Test logic starts here */
    if (om_test_ptr->transition_count[om_test_ptr->vector_cnt] >= 2) /* 1 transition each direciton */
    {
      if (om_test_ptr->vector_cnt == MAX_OM_TRANSITION_TYPES - 1)
      {
        /* reset the test vector */
        QCLI_Printf("OM_Transition_test: completed, freeing test vectors\n");
        if ( QAPI_OK != qapi_OMSM_Free(QAPI_OMSM_DEFAULT_AON_POOL, OM_SMEM_OM_TEST_CLIENT_ID) )
        {
          /* unable to free*/
          return false;
        }
        return true;
      }
      else
      {
       /* pick the next transtion defined in test map */
        QCLI_Printf("OM_Transition_test: Starting next test vector\n");
        om_test_ptr->vector_cnt++;
      }
    }
    else
    {
      /* Check that our previous destination mode matches the current mode */
#ifdef V1
      if (om_test_ptr->test_map[om_test_ptr->vector_cnt].dest_om == omtm_get_curr_mode_id())
#else
      if (om_test_ptr->test_map[om_test_ptr->vector_cnt].dest_om == qapi_OMTM_Get_Current_Operating_Mode_Id())
#endif
      {
        /* Swap the curr_om and dest_om */
        dest_om = om_test_ptr->test_map[om_test_ptr->vector_cnt].dest_om;
        om_test_ptr->test_map[om_test_ptr->vector_cnt].dest_om = om_test_ptr->test_map[om_test_ptr->vector_cnt].curr_om;
        om_test_ptr->test_map[om_test_ptr->vector_cnt].curr_om = dest_om;
      }
      else
      {
        return false;
      }
    }
    if ( QAPI_OK != qapi_OMSM_Commit(QAPI_OMSM_DEFAULT_AON_POOL, OM_SMEM_OM_TEST_CLIENT_ID) )
    {
      return false;
    }
    qapi_OMTM_Register_Mode_Exit_Callback(om_transition_test_cb, &(om_test_ptr->transition_count[om_test_ptr->vector_cnt]), -50);
    if (om_test_ptr->test_map[om_test_ptr->vector_cnt].dest_om == 2 /*SOM_MOM*/)
    {
      QCLI_Printf("OM_Transition_test: OM test vector - transition to MOM \n");
      mom_wakeup_callback_register(mom_oem_wakeup_handler, &(om_test_ptr->transition_count[om_test_ptr->vector_cnt]));
      mom_set_wakeup_timer(0x10000, 0);
    }
    /*Due to a bug in V1 OMTM code, QAPI_OMTM_SWITCH_NOW_E is mapped to OMTM_SWITCH_AT_IDLE. Until that is fixed, swapping with QAPI_OMTM_SWITCH_AT_IDLE_E*/

    qapi_OMTM_Switch_Operating_Mode(om_test_ptr->test_map[om_test_ptr->vector_cnt].dest_om, QAPI_OMTM_SWITCH_NOW_E);
  }

  /* Initialize test vectors */
  else
  {
    QCLI_Printf("OM_Transition_test: Initializing OM test vector\n");
    if ( QAPI_OK != qapi_OMSM_Alloc(QAPI_OMSM_DEFAULT_AON_POOL, OM_SMEM_OM_TEST_CLIENT_ID, sizeof(om_test_info_t), (void**)&om_test_ptr) )
    {
      return false;
    }
    initialize_test_vectors(om_test_ptr);
    if ( QAPI_OK != qapi_OMSM_Commit(QAPI_OMSM_DEFAULT_AON_POOL, OM_SMEM_OM_TEST_CLIENT_ID) ) { return false; }
    /* Kick-off transition! */
    qapi_OMTM_Register_Mode_Exit_Callback(om_transition_test_cb, &(om_test_ptr->transition_count[om_test_ptr->vector_cnt]), -50);
    if (om_test_ptr->test_map[om_test_ptr->vector_cnt].dest_om == 2 /*OM_MOM*/)
    {
      QCLI_Printf("OM_Transition_test: OM test vector - transition to MOM \n");
      mom_wakeup_callback_register(mom_oem_wakeup_handler, &(om_test_ptr->transition_count[om_test_ptr->vector_cnt]));
      mom_set_wakeup_timer(0x10000, 0);
    }
    QCLI_Printf("OM_Transition_test: OM test vector - transition to mode=%d \n", om_test_ptr->test_map[om_test_ptr->vector_cnt].dest_om);
    /*Due to a bug in V1 OMTM code, QAPI_OMTM_SWITCH_NOW_E is mapped to OMTM_SWITCH_AT_IDLE. Until that is fixed, swapping with QAPI_OMTM_SWITCH_AT_IDLE_E*/

    qapi_OMTM_Switch_Operating_Mode(om_test_ptr->test_map[om_test_ptr->vector_cnt].dest_om, QAPI_OMTM_SWITCH_NOW_E);
  }

  if (om_test_ptr->vector_cnt > MAX_OM_TRANSITION_TYPES)
  {
    status = SUCCESS;
  }
  return status;
}

void fom_register_operating_modes(void)
{
  /* Register OMTM operating mode table for FOM */
  qapi_OMTM_Register_Operating_Modes((qapi_OMTM_Operating_Mode_t*)&omtm_operating_mode_tbl_sram, 3, 0);
}

void lp_timer_sleep_cb(uint32_t data)
{
  //stop a running timer
  qapi_Timer_Stop( lp_timer_handle);

  //Undef the timer. Releases memory allocated in qapi_Timer_Def()
  qapi_Timer_Undef( lp_timer_handle);
}

QCLI_Command_Status_t set_sleep(uint32_t sleep)
{
  qapi_TIMER_define_attr_t timer_def_attr;
  timer_def_attr.cb_type = QAPI_TIMER_FUNC1_CB_TYPE; //notification type
  timer_def_attr.sigs_func_ptr = (void*)&lp_timer_sleep_cb; //callback to call when timer expires
  timer_def_attr.sigs_mask_data = 0x1; //this data will be returned in the callback
  timer_def_attr.deferrable = false; //set to true for non-deferrable timer

  if(sleep == 0)
    g_sleepAllowLowPowerModes = FALSE;
  else if(sleep == 1)
    g_sleepAllowLowPowerModes = TRUE;
  else{
    QCLI_Printf("Incorrect params, should be 0 or 1\r\n");
    return QCLI_STATUS_ERROR_E;
  }

  //define the timer. Note: This call allocates memory and hence qapi_Timer_Undef()
  //should be called whenever the timer usage is done.
  qapi_Timer_Def( &lp_timer_handle, &timer_def_attr);

  qapi_TIMER_set_attr_t timer_set_attr;
  timer_set_attr.reload = FALSE; //Do not restart timer after it expires
  timer_set_attr.time = 1;
  timer_set_attr.unit = QAPI_TIMER_UNIT_MSEC;
  timer_set_attr.max_deferrable_timeout = 0; //Don't care. Not used. For future support

  //set or start the timer
  qapi_Timer_Set( lp_timer_handle, &timer_set_attr);
  return QCLI_STATUS_SUCCESS_E;
}

void lp_uart_cb(uint32_t data)
{
  //stop a running timer
  qapi_Timer_Stop( lp_uart_timer_handle);

  //Undef the timer. Releases memory allocated in qapi_Timer_Def()
  qapi_Timer_Undef( lp_uart_timer_handle);
  PAL_Uart_Init();
}

QCLI_Command_Status_t disable_uart(uint32_t sleep)
{
  qapi_TIMER_define_attr_t timer_def_attr;
  timer_def_attr.cb_type = QAPI_TIMER_FUNC1_CB_TYPE; //notification type
  timer_def_attr.sigs_func_ptr = (void*)&lp_uart_cb; //callback to call when timer expires
  timer_def_attr.sigs_mask_data = 0x1; //this data will be returned in the callback
  timer_def_attr.deferrable = false; //set to true for non-deferrable timer

  PAL_Uart_Deinit();
  qapi_Timer_Def( &lp_uart_timer_handle, &timer_def_attr);

  qapi_TIMER_set_attr_t timer_set_attr;
  timer_set_attr.reload = FALSE; //Do not restart timer after it expires
  timer_set_attr.time = sleep;
  timer_set_attr.unit = QAPI_TIMER_UNIT_MSEC;
  timer_set_attr.max_deferrable_timeout = 0; //Don't care. Not used. For future support

  //set or start the timer
  qapi_Timer_Set( lp_uart_timer_handle, &timer_set_attr);
  return QCLI_STATUS_SUCCESS_E;
}
