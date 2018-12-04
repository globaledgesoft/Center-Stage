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
                     som_lp_test.c

GENERAL DESCRIPTION
  this file implements the OM transition demo tests for SOM. The RO/RW/ZI
  from this compile should be placed in SOM memory banks

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
#include "qapi_timer.h"
#include "qapi_dsr.h"
#include "qapi_fatal_err.h"
#include "qapi_spi_master.h"
#include "qapi_diag_msg.h"
#include "stringl.h"


/*======================================================================
                          EXTERNAL
 ======================================================================*/
/*======================================================================
                          GLOBALS
 ======================================================================*/
#define FEATURE_GPIO_CORE_TEST

qapi_TIMER_handle_t som_test_timers[SOM_TEST_MAX_TIMERS];
qapi_TIMER_define_attr_t som_timer_def_attr[SOM_TEST_MAX_TIMERS];
qapi_TIMER_set_attr_t som_timer_set_attr[SOM_TEST_MAX_TIMERS];
uint64_t timer_set_time[SOM_TEST_MAX_TIMERS] = {300, 400/*, 500*/};
void* som_test_timer_cb[SOM_TEST_MAX_TIMERS] = {(void *)som_timer1_cb, (void *)som_timer2_cb/*, som_timer3_cb*/};

qapi_dsr_obj_t *som_test_dsr[SOM_TEST_MAX_DSRS];
qapi_dsr_priority_t dsr_prio[SOM_TEST_MAX_DSRS];
uint32_t som_dsr_init_data = 0;
som_test_app_info_t som_test_app_info;
boolean som_test_app_force_exit = FALSE;

#ifdef FEATURE_GPIO_CORE_TEST
gpio_test_t gpio_test_info;
qapi_Instance_Handle_t gpio_hdl;
#endif

som_test_ctxt_t som_test_ctxt[] =
{
  {0, &som_test_timers[0]},
  {1, &som_test_timers[1]},
  {2, &som_test_timers[2]}
};


/* SPI test data */
static void *hSPI1;
static uint8_t *spi_write_buf,*spi_read_buf;
static qapi_Status_t spi_status;
static qapi_SPIM_Transfer_t tx_info,rx_info;
static qapi_SPIM_Config_t spi_test_config;
static uint32_t DATA_SIZE = 256;

/*======================================================================
                          FUNCTIONS
 ======================================================================*/
extern uint32_t omtm_get_curr_mode_id( void );

void som_dsr_default_handler(qapi_dsr_obj_t *dsr_obj, void *ctxt, void *payload)
{
  som_test_app_info.som_dsr_count[((som_test_ctxt_t *)ctxt)->dsr_cnt_idx]++;
  qapi_Timer_Set(*(((som_test_ctxt_t*)ctxt)->timer), (qapi_TIMER_set_attr_t*)payload);
}

void som_dsr_spi_tx_complete_handler(qapi_dsr_obj_t *dsr_obj, void *ctxt, void *payload)
{
  uint32_t i = 0;

  /* Check if SPI tests passed */
  // wait for test thread finish
  for (i = 0; i < DATA_SIZE; i++)
  {
    if (spi_write_buf[i] != spi_read_buf[i])
    {
      QAPI_FATAL_ERR(0,0,0);
    }
  }

  if((spi_status = qapi_SPIM_Disable(hSPI1)) != QAPI_OK)
  {
    QAPI_FATAL_ERR(0,0,0);
  }

  if((spi_status = qapi_SPIM_Close(hSPI1)) != QAPI_OK)
  {
    QAPI_FATAL_ERR(0,0,0);
  }

  /* free the tx buffers
  */
  free(spi_write_buf);
  free(spi_read_buf);
  QAPI_DIAG_MSG_ARG0(QAPI_DIAG_MSG_SYSBUF_HDL, 0, QAPI_DIAG_MSG_LVL_MED, "Core Test1:spi test :passed!!\r\n");

  /* Complete DSR test vector */
  som_test_app_info.som_dsr_count[((som_test_ctxt_t *)ctxt)->dsr_cnt_idx]++;
  qapi_Timer_Set(*(((som_test_ctxt_t*)ctxt)->timer), (qapi_TIMER_set_attr_t*)payload);
}

#ifdef FEATURE_GPIO_CORE_TEST
void som_dsr_gpio_handler(qapi_dsr_obj_t *dsr_obj, void *ctxt, void *payload)
{
  som_test_app_info.som_dsr_count[((som_test_ctxt_t *)ctxt)->dsr_cnt_idx]++;
  if( QAPI_OK != qapi_GPIOINT_Trigger_Interrupt(gpio_hdl,SOM_TEST_GPIO_NUM) )
  {
    QAPI_FATAL_ERR(0,0,0);
  }
}
#endif

void som_timer1_cb(uint32_t data)
{
  som_test_app_info.som_timer_count[0]++;
  if( QAPI_OK != qapi_dsr_enqueue(som_test_dsr[0],(void*)data))
  {
    QAPI_FATAL_ERR(0,0,0);
  }
}

void spi_Tx_callback (uint32_t i_status, void *data)
{
  /* Complete the DSR test vector */
  som_test_app_info.som_timer_count[1]++;
  if( QAPI_OK != qapi_dsr_enqueue(som_test_dsr[1],(void*)data))
  {
    QAPI_FATAL_ERR(0,0,0);
  }
}

void som_timer2_cb(uint32_t data)
{
//Enable SPI test when SPI is verified on ASIC	
#ifdef SPI_TEST_ENABLED
  uint32 i = 0, result = 0;

  QAPI_DIAG_MSG_ARG0(QAPI_DIAG_MSG_SYSBUF_HDL, 0, QAPI_DIAG_MSG_LVL_MED, "Core Test1:spi test :start!!\r\n");

  spi_test_config.SPIM_Clk_Polarity = QAPI_SPIM_CLK_IDLE_LOW_E,
  spi_test_config.SPIM_Clk_Polarity =  QAPI_SPIM_CLK_IDLE_HIGH_E;
  spi_test_config.SPIM_Shift_Mode = QAPI_SPIM_INPUT_FIRST_MODE_E;
  spi_test_config.SPIM_CS_Polarity = QAPI_SPIM_CS_ACTIVE_LOW_E;
  spi_test_config.SPIM_CS_Mode = QAPI_SPIM_CS_DEASSERT_E;
  spi_test_config.SPIM_Clk_Always_On =  QAPI_SPIM_CLK_NORMAL_E;
  spi_test_config.SPIM_Bits_Per_Word = 8;
  spi_test_config.SPIM_Slave_Index = 0;
  spi_test_config.min_Slave_Freq_Hz = 0;
  spi_test_config.max_Slave_Freq_Hz = 50000000;
  spi_test_config.deassertion_Time_Ns = 0;
  spi_test_config.loopback_Mode = 1;
  spi_test_config.hs_Mode = 0;

  if ((spi_status = qapi_SPIM_Open(QAPI_SPIM_INSTANCE_1_E, &hSPI1)) != QAPI_OK)
  {
    QAPI_FATAL_ERR(0,0,0);
  }
  if ((spi_status = qapi_SPIM_Enable(hSPI1)) != QAPI_OK)
  {
    QAPI_FATAL_ERR(0,0,0);
  }
  spi_write_buf = (uint8 *) malloc (DATA_SIZE);
  if (spi_write_buf == NULL)
  {
    QAPI_FATAL_ERR(0,0,0);
  }
  spi_read_buf = (uint8 *) malloc (DATA_SIZE);
  if (spi_read_buf == NULL)
  {
    QAPI_FATAL_ERR(0,0,0);
  }
  memset (spi_write_buf, 0xAA, DATA_SIZE);
  memset (spi_read_buf, 0, DATA_SIZE);
  // Allocate the desc for write and read
  tx_info.buf_len = DATA_SIZE;
  rx_info.buf_len = DATA_SIZE;
  tx_info.buf_phys_addr = spi_write_buf;
  rx_info.buf_phys_addr = spi_read_buf;
  if((spi_status = qapi_SPIM_Full_Duplex(hSPI1, &spi_test_config, &tx_info, &rx_info, spi_Tx_callback, (void*)data)) != QAPI_OK)
  {
      QAPI_FATAL_ERR(0,0,0);
  }
#else

  som_test_app_info.som_timer_count[1]++;
  if( QAPI_OK != qapi_dsr_enqueue(som_test_dsr[1],(void*)data))
  {
    QAPI_FATAL_ERR(0,0,0);
  }	
#endif  
}



void som_timer3_cb(uint32_t data)
{
  som_test_app_info.som_timer_count[2]++;
  qapi_dsr_enqueue(som_test_dsr[2],(void*)data);
}

#ifdef FEATURE_GPIO_CORE_TEST
void fom_test_gpio_isr(qapi_GPIOINT_Callback_Data_t data)
{
  gpio_test_t *gpio_ptr = (gpio_test_t *)data;
  gpio_ptr->fom_gpio_cnt++;
  gpio_ptr->gpio_event_pending = 0;
}

void som_test_gpio_isr(qapi_GPIOINT_Callback_Data_t data)
{
  gpio_test_t *gpio_ptr = (gpio_test_t *)data;
  gpio_ptr->som_gpio_cnt++;
  gpio_ptr->gpio_event_pending = 1;
  if ( QAPI_OK != qapi_GPIOINT_Disable_Interrupt(gpio_hdl, SOM_TEST_GPIO_NUM) ||
       QAPI_OK != qapi_GPIOINT_Trigger_Interrupt(gpio_hdl, SOM_TEST_GPIO_NUM))
  {
    QAPI_FATAL_ERR(0,0,0);
  }
}
#endif


void som_app_init(void)
{

  /* 1. Create 4 timers each with a different callback.
   * 2. Each timer callback will trigger a DSR of different priorities (LOW, MED, HIGH)
   * 3. Two out of the four DSRs will increment the DSR counter and set the timer to run again.
   * 4. Register GPIO Interrupt to test interrupt replay logic
   */
#ifdef FEATURE_GPIO_CORE_TEST
  uint32_t nFlags, nRet;
#endif
  uint8 i;

 /* Register OMTM operating mode table during SOM init
 */
  qapi_OMTM_Register_Operating_Modes((qapi_OMTM_Operating_Mode_t*)&omtm_operating_mode_tbl_sram, 3, 1);


  memset(&som_test_app_info,0,(size_t)sizeof(som_test_app_info));

#ifdef FEATURE_GPIO_CORE_TEST
  /* Register GPIO interrupt */
  if (QAPI_OK !=  qapi_GPIOINT_Register_Interrupt(gpio_hdl, SOM_TEST_GPIO_NUM, som_test_gpio_isr, (qapi_GPIOINT_Callback_Data_t)&gpio_test_info, QAPI_GPIOINT_TRIGGER_EDGE_RISING_E,QAPI_GPIOINT_PRIO_HIGH_E, false ))
  {
     QAPI_FATAL_ERR(0,0,0);
  }
#endif

  /* Create the DSRs */
  if( QAPI_OK != qapi_dsr_create(&som_test_dsr[0], som_dsr_default_handler, &som_test_ctxt[0], dsr_prio[0]) ||
#ifdef SPI_TEST_ENABLED  
      QAPI_OK != qapi_dsr_create(&som_test_dsr[1], som_dsr_spi_tx_complete_handler, &som_test_ctxt[1], dsr_prio[1]) ||
#else	  
	  QAPI_OK != qapi_dsr_create(&som_test_dsr[1], som_dsr_default_handler, &som_test_ctxt[1], dsr_prio[1]) ||
#endif
      QAPI_OK != qapi_dsr_create(&som_test_dsr[2], som_dsr_default_handler, &som_test_ctxt[2], dsr_prio[2]))
  {
     QAPI_FATAL_ERR(0,0,0);
  }

#ifdef FEATURE_GPIO_CORE_TEST
  /* Define DSR for GPIO Interrupt Handling */
  if( QAPI_OK != qapi_dsr_create(&som_test_dsr[SOM_TEST_MAX_DSRS-1], som_dsr_gpio_handler, &som_test_ctxt[SOM_TEST_MAX_DSRS-1], QAPI_DSR_PRI_HIGH))
  {
      QAPI_FATAL_ERR(0,0,0);
  }
#endif

  /* define and set timers  */
  for(i=0; i<SOM_TEST_MAX_TIMERS; i++)
  {
    som_timer_def_attr[i].cb_type = QAPI_TIMER_FUNC1_CB_TYPE;
    som_timer_def_attr[i].deferrable = FALSE;
    som_timer_def_attr[i].sigs_func_ptr = som_test_timer_cb[i];
    som_timer_def_attr[i].sigs_mask_data = (uint32_t)(&som_timer_set_attr[i]);

    som_timer_set_attr[i].time = timer_set_time[i];
    som_timer_set_attr[i].unit = QAPI_TIMER_UNIT_MSEC;
    som_timer_set_attr[i].reload = FALSE;
    som_timer_set_attr[i].max_deferrable_timeout = 0;

    if ( QAPI_OK != qapi_Timer_Def(&som_test_timers[i], &som_timer_def_attr[i]) )
    {
      //err_fatal
      QAPI_FATAL_ERR(0,0,0);
      return;
    }
    if ( QAPI_OK != qapi_Timer_Set(som_test_timers[i], &som_timer_set_attr[i]) )
    {
        //err_fatal
      QAPI_FATAL_ERR(0,0,0);
      return;
    }
  }
  som_test_app_info.som_app_init_done = 1;
}

void som_app_exit(void)
{
  static boolean dest_mode = 0;
  uint8 i;

  if(!som_test_app_info.som_app_exit_done)
  {
    for(i=0; i<SOM_TEST_MAX_TIMERS; i++)
    {
      if ( QAPI_OK != qapi_Timer_Undef(som_test_timers[i]) )
      {
        //err_fatal
        QAPI_FATAL_ERR(0,0,0);
        return;
      }
    }
    for(i=0; i<SOM_TEST_MAX_TIMERS; i++)
    {
      if (QAPI_OK != qapi_dsr_destroy(som_test_dsr[i]))
      {
        QAPI_FATAL_ERR(0,0,0);
      }
    }
    som_test_app_info.som_app_exit_done = 1;

    som_transition_test();
  }
}



void som_app_entry_internal(void)
{

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

}


void som_transition_test_cb(uint32_t dest_mode_id, void *data)
{
  uint32_t *count = (uint32_t *)data;
  (*count)++;
}


om_transition_status_e som_transition_test(void)
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
       return false;
    }

    /* Test logic starts here */
    if (om_test_ptr->transition_count[om_test_ptr->vector_cnt] >= 2) /* 1 transition each direciton */
    {
      if (om_test_ptr->vector_cnt == MAX_OM_TRANSITION_TYPES - 1)
      {
        om_test_ptr->vector_cnt = 0;
        if ( QAPI_OK != qapi_OMSM_Commit(QAPI_OMSM_DEFAULT_AON_POOL, OM_SMEM_OM_TEST_CLIENT_ID) )
        {
           return false;
        }
        return true;
      }
      else
      {
        om_test_ptr->vector_cnt++;
      }
    }
    else
    {
        /* Check that our previous destination mode matches the current mode */
        if (om_test_ptr->test_map[om_test_ptr->vector_cnt].dest_om == omtm_get_curr_mode_id())
        {
           /* Swap the curr_om and dest_om */
           dest_om = om_test_ptr->test_map[om_test_ptr->vector_cnt].dest_om;
           om_test_ptr->test_map[om_test_ptr->vector_cnt].dest_om = om_test_ptr->test_map[om_test_ptr->vector_cnt].curr_om;
           om_test_ptr->test_map[om_test_ptr->vector_cnt].curr_om = dest_om;
        }
        else
        {
          QAPI_FATAL_ERR(0,0,0);
          return false;
        }
    }
    if ( QAPI_OK != qapi_OMSM_Commit(QAPI_OMSM_DEFAULT_AON_POOL, OM_SMEM_OM_TEST_CLIENT_ID) )
    {
      return false;
    }
    qapi_OMTM_Register_Mode_Exit_Callback(som_transition_test_cb, &(om_test_ptr->transition_count[om_test_ptr->vector_cnt]), -50);
    if (om_test_ptr->test_map[om_test_ptr->vector_cnt].dest_om == 2)
    {
      mom_wakeup_callback_register(mom_oem_wakeup_handler, (void*)&mom_oem_counter);
      mom_set_wakeup_timer(0x10000, 0);
    }
    qapi_OMTM_Switch_Operating_Mode(om_test_ptr->test_map[om_test_ptr->vector_cnt].dest_om, QAPI_OMTM_SWITCH_NOW_E);
  }
  /* Initialize test vectors */
  else
  {
     /* Kick-off transition! */
     qapi_OMTM_Register_Mode_Exit_Callback(som_transition_test_cb, &(om_test_ptr->transition_count[om_test_ptr->vector_cnt]), -50);
     if ( om_test_ptr->test_map[om_test_ptr->vector_cnt].dest_om == 1 /*SOM*/)
     {
       //QAPI_FATAL_ERR("Transitioning to same OM",0,0);
       return false;
     }
     if (om_test_ptr->test_map[om_test_ptr->vector_cnt].dest_om == 2 /*OM_MOM*/)
     {
       mom_wakeup_callback_register(mom_oem_wakeup_handler, (void*)&mom_oem_counter);
       mom_set_wakeup_timer(0x10000, 0);
     }
     qapi_OMTM_Switch_Operating_Mode(om_test_ptr->test_map[om_test_ptr->vector_cnt].dest_om, QAPI_OMTM_SWITCH_NOW_E);
  }

  if (om_test_ptr->vector_cnt > MAX_OM_TRANSITION_TYPES)
  {
    status = SUCCESS;
  }
  return status;

}
