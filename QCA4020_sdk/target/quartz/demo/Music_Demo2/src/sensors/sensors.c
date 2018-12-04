/*
 * Copyright (c) 2015-2018 Qualcomm Technologies, Inc.
 * 2015-2016 Qualcomm Atheros, Inc.
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

#include <stdlib.h>
#include <stdio.h>
#include "qurt_signal.h"
#include "qapi/qurt_thread.h"
#include "stdint.h"
#include <qcli.h>
#include <qcli_api.h>
#include <qurt_timer.h>
#include <qurt_pipe.h>
#include "qapi_timer.h"
#include "qapi/qapi_status.h"
#include <qapi_i2c_master.h>
#include "qapi_tlmm.h"
#include "qapi_gpioint.h"
#include "sensors_demo.h"
#define PIR_THREAD_STACK_SIZE		(1024)
#define PIR_THREAD_PRIORITY		(10)
#define PIR_PIN				27
#define PIR_THREAD_SIGNAL_INTR		(1<<0)
#define PIR_THREAD_STOP			(1<<1)
#define QC_MSC_FESTIVAL 1
#ifdef QC_MSC_FESTIVAL
#define MOTION_TIMER_SIGNAL_INTR		(1<<2)
#define FLASH_FAST_YELLOW 0xff0f000000ffe400
#define RAINBOW_FAST 0xff0f0002ff00ff00
#define PULSE_SLOW_PINK 0x787800004e00ff00
#define PULSE_BLUE 0xafaf0001ffff0000
#define PULSE_WHITE 0xafaf0001000000ff
//extern int MSCD_NUM_BULBS;
#endif

extern  QCLI_Group_Handle_t qcli_sensors_group;              /* Handle for our QCLI Command Group. */

#ifdef QC_MSC_FESTIVAL
void mot_print(char *str)
{
	QCLI_Printf(qcli_sensors_group, "%s\n", str);
}
#endif

#ifdef CONFIG_CDB_PLATFORM

static qurt_signal_t pir_int_signal;
/**
 * Int_Callback function is to handle the PIR interrupts
 */
void pir_int_callback(qapi_GPIOINT_Callback_Data_t data)
{
	qurt_signal_set(&pir_int_signal, PIR_THREAD_SIGNAL_INTR);
	return;
}

#ifdef QC_MSC_FESTIVAL
//QC-MSC-FSTVL-Start
qapi_TIMER_set_attr_t Set_Timer_Attr_M;
static qapi_TIMER_handle_t PeriodicMotionTimer;
qapi_TIMER_define_attr_t Create_Timer_Attr_M;
uint32_t motion_frequency_threshold = 3;
uint32_t duration = 3;
extern int  RegisterMotDet_Service();
uint64_t mot_rate = PULSE_WHITE;
uint64_t prev_mot_rate = 0x0;
uint8_t	is_music_on = 0;
void mscd_write_callback();
char* mot_rate_func()
{
	return (char*)&mot_rate;
}

#endif

/**
 * func(): PIR_Thread handles the interrupt genearted by
 * PIR sensors
 */

void pir_thread(void *param)
{
	int32_t gpio_pin = PIR_PIN;
	uint32_t sig;
  uint32_t motion_cnt = 0; 
 	int motion_announced = 0;
	// Necessary Data Type declarations
	qapi_GPIO_ID_t  gpio_id;
	qapi_Instance_Handle_t pH1;
	qapi_Status_t status = QAPI_OK;
	qapi_TLMM_Config_t tlmm_config;

	tlmm_config.pin = gpio_pin;
	tlmm_config.func = 0;   // Using the functionality tied to pin mux value
	tlmm_config.dir =  QAPI_GPIO_INPUT_E;
	tlmm_config.pull = QAPI_GPIO_PULL_DOWN_E;
	tlmm_config.drive = QAPI_GPIO_2MA_E; // drive is for output pins

	status = qapi_TLMM_Get_Gpio_ID(&tlmm_config, &gpio_id);

	if (status == QAPI_OK)
	{
		status = qapi_TLMM_Config_Gpio(gpio_id, &tlmm_config);
		if (status != QAPI_OK)
		{
			QCLI_Printf(qcli_sensors_group, "Failed to config GPIO\n");
			goto release_gpio;
		}
	}

	if (qapi_GPIOINT_Register_Interrupt(&pH1, gpio_pin, (qapi_GPIOINT_CB_t )pir_int_callback,
		0, QAPI_GPIOINT_TRIGGER_EDGE_DUAL_E, QAPI_GPIOINT_PRIO_MEDIUM_E, false) != QAPI_OK)
	{
		QCLI_Printf(qcli_sensors_group, "Interrupt Registeration failed\n");
		goto release_gpio;
	}

	QCLI_Printf(qcli_sensors_group, "Starting PIR thread - Timer init\n");


#ifdef QC_MSC_FESTIVAL

  Create_Timer_Attr_M.deferrable     = false;
  Create_Timer_Attr_M.cb_type        = QAPI_TIMER_NATIVE_OS_SIGNAL_TYPE;
  Create_Timer_Attr_M.sigs_func_ptr  = (void *)&pir_int_signal;
  Create_Timer_Attr_M.sigs_mask_data = MOTION_TIMER_SIGNAL_INTR;
  qapi_Timer_Def(&(PeriodicMotionTimer), &Create_Timer_Attr_M);

  /* Start the timer for periodic transmissions.     */
  Set_Timer_Attr_M.time                   = duration * 1000; 
  Set_Timer_Attr_M.reload                 = true;
  Set_Timer_Attr_M.max_deferrable_timeout = 0; 
  Set_Timer_Attr_M.unit                   = QAPI_TIMER_UNIT_MSEC;
  qapi_Timer_Set(PeriodicMotionTimer, &Set_Timer_Attr_M);

#endif

	while(1)
	{
		sig = qurt_signal_wait(&pir_int_signal, (PIR_THREAD_STOP | PIR_THREAD_SIGNAL_INTR | MOTION_TIMER_SIGNAL_INTR),
				QURT_SIGNAL_ATTR_WAIT_ANY | QURT_SIGNAL_ATTR_CLEAR_MASK);

		if (sig & PIR_THREAD_STOP)
		{
			break;
		}

		if(!is_music_on)
			continue;

		if (sig & PIR_THREAD_SIGNAL_INTR)
		{
			QCLI_Printf(qcli_sensors_group, "PIR sensor detected motion = %d\n", (motion_cnt + 1));
      motion_cnt++;

			if((motion_cnt > motion_frequency_threshold * 2) && (motion_announced < 2))
			{
				mot_rate = FLASH_FAST_YELLOW; 
				QCLI_Printf(qcli_sensors_group, "Fast motion detected : *****\n");
				mscd_write_callback();
				motion_announced = 2;
			}
			else if((motion_cnt <= (motion_frequency_threshold * 2)) && (motion_cnt >= motion_frequency_threshold) && !motion_announced)
			{
				mot_rate = RAINBOW_FAST;
				QCLI_Printf(qcli_sensors_group, "Mediam motion detected : **\n");
				mscd_write_callback();
				motion_announced = 1;

			}	
		}

#ifdef QC_MSC_FESTIVAL

	if (sig & MOTION_TIMER_SIGNAL_INTR)
	{
		motion_announced = 0;
		if(motion_cnt < motion_frequency_threshold)
    {
			mot_rate = PULSE_SLOW_PINK;
			QCLI_Printf(qcli_sensors_group, "Slow motion detected : *\n");
			mscd_write_callback();
		}
		motion_cnt = 0;
	}
#endif
	}

	QCLI_Printf(qcli_sensors_group, "Signal received to disable PIR\n");
	// Deregister the GPIO Interrupt
	status = qapi_GPIOINT_Deregister_Interrupt(&pH1, gpio_pin);
	if (status != QAPI_OK)
		QCLI_Printf(qcli_sensors_group, "Deregistering the Interrupt failed\n");

release_gpio:
	if (qapi_TLMM_Release_Gpio_ID(&tlmm_config, gpio_id ) != QAPI_OK)
		QCLI_Printf(qcli_sensors_group, "GPIO pin %d release Failed\n", gpio_pin);
  qapi_Timer_Undef(PeriodicMotionTimer);
	qurt_signal_delete(&pir_int_signal);
	qurt_thread_stop();
	return;
}

#ifdef QC_MSC_FESTIVAL

#define MSCD_THREAD_STACK_SIZE		(4096)
#define MSCD_THREAD_PRIORITY		(12)
#define MSCD_THREAD_STOP			(1<<5)

#define MSCD_SCAN_RESULT_SIGNAL_INTR		(1)
#define MSCD_CONNECTION_SUCCESS_SIGNAL_INTR		(2)
#define MSCD_SERVICE_DISCOVERY_SIGNAL_INTR (3)
#define MSCD_PERIODIC_TIMER_SIGNAL_INTR (4)
#define MSCD_SCAN_STOPPED_SIGNAL_INTR (5)

#define MSCD_SCAN_RESULT		(6)
#define MSCD_CONNECTION_RESULT		(7)
#define MSCD_SERVICE_DISCOVERY_RESULT (8)
#define MSCD_DISCONNECTION_RESULT (9)
#define MSCD_CONNECTION_FAILED_SIGNAL_INTR (10)
#define MSCD_WRITE_SIGNAL_INTR			(11)
//wait 20 secs (1 tick = 5 secs) for connection response
//if no response is received within 20 secs
//close connection request, clear scan data
//and try to connect to next device in scanned list
#define MSCD_CONNECTION_TICKS 4
typedef struct MSCD_Q_s
{
   int event_type;
   void *data;
} MSCD_Q_t;


qapi_TIMER_set_attr_t MSCD_Set_Timer_Attr;
static qapi_TIMER_handle_t PeriodicScanTimer;
qapi_TIMER_define_attr_t MSCD_Create_Timer_Attr;
uint32_t mscd_duration = 5000;
//static qurt_signal_t mscd_int_signal;
static qurt_pipe_attr_t mscd_qattr;
static qurt_pipe_t mscd_q;
extern QCLI_Command_Status_t mscd_InitializeBluetooth();
extern int mscd_start_scan();
extern int mscd_start_connect();
extern void mscd_discover_services();
extern void mscd_add_scan_entry(void *);
extern int mscd_assign_connection_info(void * data);
extern void mscd_attach_handles( void * data);
extern int mscd_handle_disconnection(void *data);
extern int ResetMSCDSelectedDeviceData();
extern void mscd_write_motion_data();

MSCD_Q_t *mscd_qdata;
MSCD_Q_t timer_signal = {MSCD_PERIODIC_TIMER_SIGNAL_INTR, 0};
MSCD_Q_t scan_result_signal = {MSCD_SCAN_RESULT_SIGNAL_INTR, 0};
MSCD_Q_t scan_stopped_signal = {MSCD_SCAN_STOPPED_SIGNAL_INTR, 0};
MSCD_Q_t services_discovered_signal = {MSCD_SERVICE_DISCOVERY_SIGNAL_INTR, 0};
MSCD_Q_t connection_result_signal = {MSCD_CONNECTION_SUCCESS_SIGNAL_INTR, 0};
MSCD_Q_t disconnection_result_signal = {MSCD_CONNECTION_FAILED_SIGNAL_INTR, 0};
MSCD_Q_t mscd_thread_stop_signal = {MSCD_THREAD_STOP, 0};
MSCD_Q_t mscd_write_signal = {MSCD_WRITE_SIGNAL_INTR, 0};

void mscd_scan_stopped_callback()
{
	MSCD_Q_t *mscd_sig = &scan_stopped_signal;
  qurt_pipe_send (mscd_q, &mscd_sig);
	return;
}


void mscd_disconnection_result_callback()
{
	MSCD_Q_t *mscd_sig = &disconnection_result_signal;
  qurt_pipe_send (mscd_q, &mscd_sig);
	return;
}


void mscd_scan_result_callback()
{
	MSCD_Q_t *mscd_sig = &scan_result_signal;
  qurt_pipe_send (mscd_q, &mscd_sig);
}

void mscd_connection_result_callback()
{
	MSCD_Q_t *mscd_sig = &connection_result_signal;
  qurt_pipe_send (mscd_q, &mscd_sig);
}

void mscd_service_discovery_callback()
{
	MSCD_Q_t *mscd_sig = &services_discovered_signal;
  qurt_pipe_send (mscd_q, &mscd_sig);
}

void mscd_scan_result(void *scan_data)
{
  MSCD_Q_t *t_qdata;
  t_qdata = malloc(sizeof(MSCD_Q_t));
  memset(t_qdata, 0, sizeof(MSCD_Q_t));
  t_qdata->event_type = MSCD_SCAN_RESULT;
  t_qdata->data = scan_data;
  qurt_pipe_send (mscd_q, &t_qdata);
	return;
}

void mscd_connection_result(void *conn_data)
{
  MSCD_Q_t *t_qdata;
  t_qdata = malloc(sizeof(MSCD_Q_t));
  memset(t_qdata, 0, sizeof(MSCD_Q_t));
  t_qdata->event_type = MSCD_CONNECTION_RESULT;
  t_qdata->data = conn_data;
  qurt_pipe_send (mscd_q, &t_qdata);
	return;
}

void mscd_disconnection_result(void *disconn_data)
{
  MSCD_Q_t *t_qdata;
  t_qdata = malloc(sizeof(MSCD_Q_t));
  memset(t_qdata, 0, sizeof(MSCD_Q_t));
  t_qdata->event_type = MSCD_DISCONNECTION_RESULT;
  t_qdata->data = disconn_data;
  qurt_pipe_send (mscd_q, &t_qdata);
	return;
}

void mscd_service_discovery_result(void *service_data)
{
  MSCD_Q_t *t_qdata;
  t_qdata = malloc(sizeof(MSCD_Q_t));
  memset(t_qdata, 0, sizeof(MSCD_Q_t));
  t_qdata->event_type = MSCD_SERVICE_DISCOVERY_RESULT;
  t_qdata->data = service_data;
  qurt_pipe_send (mscd_q, &t_qdata);
	return;
}

void mscd_thread_stop_callback()
{
	MSCD_Q_t *mscd_sig = &mscd_thread_stop_signal;
  qurt_pipe_send (mscd_q, &mscd_sig);
	return;
}

void mscd_write_callback()
{
	MSCD_Q_t *mscd_sig = &mscd_write_signal;
  qurt_pipe_send (mscd_q, &mscd_sig);
	return;
}

void mscd_timer_callback()
{
	MSCD_Q_t *mscd_sig = &timer_signal;
  qurt_pipe_send (mscd_q, &mscd_sig);
}

void mscd_thread(void *param)
{
  int mscd_scan_permitted = 1;
  int cnt = 0;
  int connection_timer_on = 0;
  int connection_ticks = 0;

  MSCD_Create_Timer_Attr.deferrable     = false;
  MSCD_Create_Timer_Attr.cb_type        = QAPI_TIMER_FUNC1_CB_TYPE;
  MSCD_Create_Timer_Attr.sigs_func_ptr  = mscd_timer_callback; 
  MSCD_Create_Timer_Attr.sigs_mask_data = MSCD_PERIODIC_TIMER_SIGNAL_INTR;
  qapi_Timer_Def(&(PeriodicScanTimer), &MSCD_Create_Timer_Attr);

  /* Start the timer for periodic transmissions.     */
  MSCD_Set_Timer_Attr.time                   = mscd_duration; 
  MSCD_Set_Timer_Attr.reload                 = true;
  MSCD_Set_Timer_Attr.max_deferrable_timeout = 0; 
  MSCD_Set_Timer_Attr.unit                   = QAPI_TIMER_UNIT_MSEC;
  qapi_Timer_Set(PeriodicScanTimer, &MSCD_Set_Timer_Attr);

	QCLI_Printf(qcli_sensors_group, "MSCD Thread started\n");

	while(1)
	{
    qurt_pipe_receive (mscd_q, &mscd_qdata);
 		QCLI_Printf(qcli_sensors_group, "MSCD signal received %u\n", mscd_qdata->event_type);
 		if(mscd_qdata->event_type == MSCD_WRITE_SIGNAL_INTR)
		{
			QCLI_Printf(qcli_sensors_group, "Received Write Signal\n");
	   if(mscd_scan_permitted && (prev_mot_rate != mot_rate))
	   	{
	   			mscd_write_motion_data();
	   			prev_mot_rate = mot_rate;
	   	}
		}
		else if(mscd_qdata->event_type == MSCD_PERIODIC_TIMER_SIGNAL_INTR)
		{
	 		QCLI_Printf(qcli_sensors_group, "MSCD timer signal received 0x%d\n", mscd_scan_permitted);
	 		if(connection_timer_on)
	 		{
				QCLI_Printf(qcli_sensors_group, "MSCD connection timer on\n");
	 			connection_ticks++;
	 			if(connection_ticks == MSCD_CONNECTION_TICKS)
				{
					ResetMSCDSelectedDeviceData();
					connection_timer_on = 0;
					connection_ticks = 0;
	      	mscd_disconnection_result_callback();
				}
	 		}
	 		else if(mscd_scan_permitted)
	 		{
	 			//scan at an interval of 20 secs
				switch (cnt%4)
	      {
	      case 0:
		 			cnt = 0;
		 			//fall through if scan is not needed
					if(mscd_start_scan())
					{
			   		QCLI_Printf(qcli_sensors_group, "MSCD timer/scan event received\n");
			   		mscd_scan_permitted = 0;
						break;
					}
				default:
      		mscd_write_motion_data();
      		break;
      	}
			}
	   	cnt++;
		}		
		else if(mscd_qdata->event_type == MSCD_SCAN_STOPPED_SIGNAL_INTR)
		{
			//make sure all devices we are looking for have not been found
			//before enabling scan
	   	mscd_scan_permitted = 1;
		}
		else if(mscd_qdata->event_type == MSCD_SCAN_RESULT_SIGNAL_INTR || 
			mscd_qdata->event_type == MSCD_CONNECTION_FAILED_SIGNAL_INTR)
		{
			//process the scan list - connect to devices in list 
			//do not permit scan as we need to connect and configure 
			//devices found in previous scan
			connection_ticks = 0;
			QCLI_Printf(qcli_sensors_group, "MSCD scan completed event received\n");
			if((connection_timer_on = mscd_start_connect()))
				mscd_scan_permitted = 0;
			else
				mscd_scan_permitted = 1;
		}
    else if(mscd_qdata->event_type ==  MSCD_CONNECTION_SUCCESS_SIGNAL_INTR)
		{
			//process the scan list - connect to devices in list 
			QCLI_Printf(qcli_sensors_group, "MSCD connection completed event received\n");
			connection_timer_on = 0;
			mscd_discover_services();
		}		
		else if(mscd_qdata->event_type == MSCD_SERVICE_DISCOVERY_SIGNAL_INTR)
		{

			//process the scan list - connect to devices in list 
			connection_ticks = 0;
			QCLI_Printf(qcli_sensors_group, "MSCD service discovery completed event received\n");
			if((connection_timer_on = mscd_start_connect()))
				mscd_scan_permitted = 0;
			else
				mscd_scan_permitted = 1;

		}		
		else if(mscd_qdata->event_type == MSCD_SCAN_RESULT)
		{
      mscd_add_scan_entry(mscd_qdata->data);
      free(mscd_qdata);
		}
		else if(mscd_qdata->event_type == MSCD_CONNECTION_RESULT)
		{
      mscd_assign_connection_info(mscd_qdata->data);
      free(mscd_qdata);
		}
		else if(mscd_qdata->event_type == MSCD_DISCONNECTION_RESULT)
		{
      if(mscd_handle_disconnection(mscd_qdata->data))
      	mscd_disconnection_result_callback();
      free(mscd_qdata);
		}
		else if(mscd_qdata->event_type == MSCD_SERVICE_DISCOVERY_RESULT)
		{
		  mscd_attach_handles(mscd_qdata->data);	
      free(mscd_qdata);
		}
		else if (mscd_qdata->event_type == MSCD_THREAD_STOP)
		{
			break;
		}
	}

	QCLI_Printf(qcli_sensors_group, "Signal received to disable MSCD thread\n");
	qurt_pipe_delete(mscd_q);
  qapi_Timer_Undef(PeriodicScanTimer);	
	qurt_thread_stop();
	return;
}

void mscd_demo_init()
{
	int ret;
	qurt_thread_attr_t thread_attribute;
	qurt_thread_t      thread_handle;
  qurt_pipe_attr_init (&mscd_qattr);
  qurt_pipe_attr_set_elements (&mscd_qattr, 10);
  qurt_pipe_attr_set_element_size (&mscd_qattr, sizeof(int *));
	if (0 != qurt_pipe_create (&mscd_q, &mscd_qattr))
	{
		QCLI_Printf(qcli_sensors_group, "Not able to initialize mscd q\n");
		return;
	}
	qurt_thread_attr_init(&thread_attribute);
	qurt_thread_attr_set_name(&thread_attribute, "mscd2_thrd");
	qurt_thread_attr_set_priority(&thread_attribute, MSCD_THREAD_PRIORITY);
	qurt_thread_attr_set_stack_size(&thread_attribute, MSCD_THREAD_STACK_SIZE);
	ret = qurt_thread_create(&thread_handle, &thread_attribute, mscd_thread, NULL);
	if (ret != QAPI_OK)
	{
		QCLI_Printf(qcli_sensors_group, "Music demo thread creation failed\n");
		return;
	}

}
#endif
/**
 * Start the PIR thread to handle signal.
 */
QCLI_Command_Status_t sensors_pir_driver_test(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	int ret;
	static int pir_enabled = 0;
	qurt_thread_attr_t thread_attribute;
	qurt_thread_t      thread_handle;

	if (Parameter_Count == 0)
	{
		QCLI_Printf(qcli_sensors_group, "USAGE: number <duration frequency_threshold>\n"
			"\tnumber = 0:disable pir | 1:enable pir \n  duration = sec (%d by default)\n"
			"\tfrequency_threshold: number of detections to define motion speed (%d by default)\n", 
							duration, motion_frequency_threshold);
		return 0;
	}

	if (pir_enabled == Parameter_List[0].Integer_Value)
	{
		QCLI_Printf(qcli_sensors_group, "PIR Interrupt is already %s\n", pir_enabled ? "enabled" : "disabled");
		return 0;
	}

	if(2 == Parameter_List[0].Integer_Value)
	{
		is_music_on = 0;
		if(pir_enabled){
			mot_rate = PULSE_WHITE;
			mscd_write_callback();
			QCLI_Printf(qcli_sensors_group, "Music OFF \n");
		}
		else{
			QCLI_Printf(qcli_sensors_group, "USAGE: number <duration frequency_threshold>\n"
			"\tnumber = 0:disable pir | 1:enable pir \n  duration = sec (%d by default)\n"
			"\tfrequency_threshold: number of detections to define motion speed (%d by default)\n", 
							duration, motion_frequency_threshold);
		}
	}		

	else if(3 == Parameter_List[0].Integer_Value)
	{
		is_music_on = 1;
		if(pir_enabled){
			mot_rate = PULSE_SLOW_PINK;
			mscd_write_callback();
			QCLI_Printf(qcli_sensors_group, "Music ON \n");
		}
		else{
			QCLI_Printf(qcli_sensors_group, "USAGE: number <duration frequency_threshold>\n"
			"\tnumber = 0:disable pir | 1:enable pir \n  duration = sec (%d by default)\n"
			"\tfrequency_threshold: number of detections to define motion speed (%d by default)\n",
							duration, motion_frequency_threshold);
		}
	}		

	else if (1 == Parameter_List[0].Integer_Value)
	{
		if (Parameter_Count == 2)
			duration = Parameter_List[1].Integer_Value ;
		if (Parameter_Count == 3) {
			duration = Parameter_List[1].Integer_Value;
      motion_frequency_threshold = Parameter_List[2].Integer_Value;
      /*if (Parameter_List[3].Integer_Value > 5)
      {
      	QCLI_Printf(qcli_sensors_group, "Number of Bulbs can't be more than 5");
      	return 0;
      }*/
		}
		mscd_InitializeBluetooth();
    mscd_demo_init();

		//QC-MSC-FSTVL-End
		if (0 != qurt_signal_init(&pir_int_signal))
		{
			QCLI_Printf(qcli_sensors_group, "Not able to initialize signal\n");
			return -1;
		}
		qurt_thread_attr_init(&thread_attribute);
		qurt_thread_attr_set_name(&thread_attribute, "pir_thread");
		qurt_thread_attr_set_priority(&thread_attribute, PIR_THREAD_PRIORITY);
		qurt_thread_attr_set_stack_size(&thread_attribute, PIR_THREAD_STACK_SIZE);

		ret = qurt_thread_create(&thread_handle, &thread_attribute, pir_thread, NULL);
		if (ret != QAPI_OK)
		{
			QCLI_Printf(qcli_sensors_group, "PIR thread creation failed\n");
			return -1;
		}
		pir_enabled = 1;
	}
	else if(!Parameter_List[0].Integer_Value)
	{
		is_music_on = 0;
		mot_rate = PULSE_WHITE;
		mscd_write_callback();
		qurt_signal_set(&pir_int_signal, PIR_THREAD_STOP);
		qurt_thread_sleep(5);
		pir_enabled = 0;
		mscd_thread_stop_callback();
	}
	else{
		QCLI_Printf(qcli_sensors_group, "USAGE: number <duration frequency_threshold>\n"
			"\tnumber = 0:disable pir | 1:enable pir \n  duration = sec (%d by default)\n"
			"\tfrequency_threshold: number of detections to define motion speed (%d by default)\n", 
							duration, motion_frequency_threshold);
	}
	return 0;
}
#endif

