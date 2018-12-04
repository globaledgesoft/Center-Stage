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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <qcli.h>
#include <qcli_api.h>
#include <qapi_wlan.h>
#include "qapi_i2s.h"
#include "qapi_pcm.h"
#include "adss_demo.h"
#include "adss_pcm.h"

QCLI_Group_Handle_t qcli_adss_group;              /* Handle for our QCLI Command Group. */

volatile int	audio_echo_loop_flag = 0;

QCLI_Command_Status_t adss_test_send(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t adss_test_send_receive(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t adss_vi_send_receive(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t adss_test_driver(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t adss_play_audio_wifi(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t adss_rec_audio_wifi(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t adss_test_driver_loop(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t adss_drv_audio_snd_rcv(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t adss_drv_audio_stop(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t adss_drv_pcm_echo(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t adss_drv_pcm_send_or_receive(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);

const QCLI_Command_t adss_cmd_list[] =
{
   // cmd_function        start_thread          cmd_string             usage_string                   description
   { adss_play_audio_wifi,      true,           "drvsend",               "",                     "drvsend url buf_size pkt_count"},
   { adss_rec_audio_wifi,       true,           "drvrcv",                "",                     "drvrcv svr_ip port buf_size pkt_count"},
   { adss_drv_audio_snd_rcv,    true,           "drvsndrcv",             "",                    "drvsndrcv buf_size pkt_count freq(1-6)"},
   { adss_drv_audio_stop,       false,          "stopdrvsndrcv",         "",                    "stopdrvsndrcv flag"   },
   { adss_drv_pcm_send_or_receive, true,        "pcmsr",                 "",            "PCM send & receive with Driver"},
};

const QCLI_Command_Group_t adss_cmd_group =
{
    "ADSS",
    (sizeof(adss_cmd_list) / sizeof(adss_cmd_list[0])),
    adss_cmd_list
};

   /* This function is used to register the wlan Command Group with    */
   /* QCLI.                                                             */
void Initialize_ADSS_Demo(void)
{
   /* Attempt to reqister the Command Groups with the qcli framework.*/
   qcli_adss_group = QCLI_Register_Command_Group(NULL, &adss_cmd_group);
   if(qcli_adss_group)
   {
      QCLI_Printf(qcli_adss_group, "ADSS Registered \n");
   }
}

QCLI_Command_Status_t adss_play_audio_wifi(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    ADSS_RET_STATUS adss_Ftp_play_wifi(const char* interface_name, char *url, uint32_t buf_len, uint32_t pkt_count);
	char *interface_name = "wlan1";	
	char *url;	
    ADSS_RET_STATUS  result;
	uint32_t buf_size = 1024, pkt_count = 9;
 	
    if (Parameter_Count < 1)
    {
        QCLI_Printf(qcli_adss_group, "audio play missing url\n");
		return QCLI_STATUS_ERROR_E;
	}
	
	url = Parameter_List->String_Value;
	Parameter_List++;

	if (Parameter_Count >= 2)
	{
		if (Parameter_List->Integer_Value != 0)
			buf_size = Parameter_List->Integer_Value;		
		Parameter_List++;
	}

	if (Parameter_Count >= 3)
	{
		if (Parameter_List->Integer_Value != 0)
			pkt_count = Parameter_List->Integer_Value;
		Parameter_List++;
	}
		
	audio_echo_loop_flag = 1;
	
    result = adss_Ftp_play_wifi(interface_name, url, buf_size, pkt_count);
    if (result != ADSS_SUCCESS)
    {
        QCLI_Printf(qcli_adss_group, "audio play fails\n");
        return  QCLI_STATUS_ERROR_E;
    }
    return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t adss_rec_audio_wifi(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    ADSS_RET_STATUS adss_Tcp_socket_rec_wifi(char* srv_ip_addr, uint16_t port, uint32_t buf_len, uint32_t pkt_count);
	char *svr_ip_addr;
    uint16_t    port = 7890;	
    ADSS_RET_STATUS  result;
	uint32_t buf_size = 1024, pkt_count = 4;
 	
	if (Parameter_Count < 1)
	{
        QCLI_Printf(qcli_adss_group, "missing server IP\n");
		return QCLI_STATUS_ERROR_E;
	}

	svr_ip_addr = Parameter_List->String_Value;		
	Parameter_List++;

	if (Parameter_Count >= 2)
	{
		if (Parameter_List->Integer_Value != 0)
			port = Parameter_List->Integer_Value;
		Parameter_List++;
	}
	
	if (Parameter_Count >= 3)
	{
		if (Parameter_List->Integer_Value != 0)
			buf_size = Parameter_List->Integer_Value;
		Parameter_List++;
	}
	
    if (Parameter_Count >= 4)
    {
		if (Parameter_List->Integer_Value != 0)
			pkt_count = Parameter_List->Integer_Value;
		Parameter_List++;
	}
	audio_echo_loop_flag = 1;
	
    result = adss_Tcp_socket_rec_wifi(svr_ip_addr, port, buf_size, pkt_count);
    if (result != ADSS_SUCCESS)
    {
        QCLI_Printf(qcli_adss_group, "audio record fails\n");
        return  QCLI_STATUS_ERROR_E;
    }
    return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t adss_drv_audio_snd_rcv(uint32_t Parameter_Count, QCLI_Parameter_t *pvParameters)
{
    int32_t adss_driver_data_loop(qapi_I2S_Freq_e freq, uint32_t buf_size, uint32_t pkt_count );
	qapi_I2S_Freq_e freq = QAPI_I2S_FREQ_32_KHZ_E;
	uint32_t buf_size = 1024;
	uint32_t pkt_count = 4;
	
	if  (audio_echo_loop_flag)
	{
       QCLI_Printf(qcli_adss_group, "data/audio echo is running\n");
       return QCLI_STATUS_SUCCESS_E;
	}
	
	if (Parameter_Count >= 1)
	{
		if (pvParameters->Integer_Value != 0)
			buf_size = pvParameters->Integer_Value;
		
		pvParameters++;
	}

	if (Parameter_Count >= 2)
	{
		if (pvParameters->Integer_Value != 0)
			pkt_count = pvParameters->Integer_Value;
		pvParameters++;
	}

	if (Parameter_Count >= 3)
	{
		if (pvParameters->Integer_Value >= 1 && pvParameters->Integer_Value <= 6)
			freq = (qapi_I2S_Freq_e)pvParameters->Integer_Value;
		pvParameters++;
	}
	
    QCLI_Printf(qcli_adss_group, "buf size=%d pkt count=%d\n", buf_size, pkt_count);
	
    if (adss_driver_data_loop(freq, buf_size, pkt_count) == -1)
		QCLI_Printf(qcli_adss_group, "test fail\n");
	
    return QCLI_STATUS_SUCCESS_E;	
}

QCLI_Command_Status_t adss_drv_audio_stop(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	audio_echo_loop_flag = 0;
    return QCLI_STATUS_SUCCESS_E;		
}

QCLI_Command_Status_t adss_drv_pcm_echo(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	int err_count;
    int adss_pcm_echo(uint32_t Parameter_Count, void *pvParameters );

    err_count = adss_pcm_echo(Parameter_Count, Parameter_List);
    if(err_count)
    {
       QCLI_Printf(qcli_adss_group, "test error = %d \n", err_count);
    }
	else
	{
       QCLI_Printf(qcli_adss_group, "PCM test Successful \n");		
	}

   return QCLI_STATUS_SUCCESS_E;	
}

extern uint32_t    u32_pkt_len;
extern uint32_t    pcm_slot_bitmap;

uint8_t	   default_fixed_slots[]={0,1};

QCLI_Command_Status_t adss_drv_pcm_send_or_receive(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	uint32_t i, iVal;
	QCLI_Parameter_t *p;
	char     *str, *pSlot;
	
	uint32_t pkt_count=6;
	uint32_t buf_size = 1024;
	uint32_t sample_freq = 0;                         // 0 - 8KHz  1- 16KHz  2 - 32KHz
	
	void adss_pcm_play(uint32_t pkt_count, uint32_t buf_size );

	memcpy(fixed_slots, default_fixed_slots, sizeof(default_fixed_slots));
	max_slots = sizeof(default_fixed_slots) / sizeof(default_fixed_slots[0]);
		
	for (i=0, p = Parameter_List; i < Parameter_Count; i++, p++)
	{
		if (strncmp(p->String_Value, "buf_size", 8) == 0)
			buf_size = atoi(p->String_Value+9);
		else if (strncmp(p->String_Value, "pkt_count", 9) == 0)
			pkt_count = atoi(p->String_Value+10);
		else if (strncmp(p->String_Value, "sample_freq", 11) == 0)
			sample_freq = atoi(p->String_Value+12);
		else if (strncmp(p->String_Value, "slots", 5) == 0)
		{
			max_slots = 0;
			str = p->String_Value+6;
			QCLI_Printf(qcli_adss_group, "slots str=%s\n", str);
			
			while (*str && max_slots < MAX_SLOTS)
			{
				pSlot = strchr(str, ',');
				if (pSlot != NULL)
				   *pSlot++ = '\0';
				iVal = atoi(str);
				
				fixed_slots[max_slots++] = (uint8_t)iVal;
				if (pSlot == NULL)
					break;
				str = pSlot;
			}					
		}
	}

	pcm_slot_bitmap = 0;
	for (i=0; i < max_slots; i++)
	{
		QCLI_Printf(qcli_adss_group, "fixed_slots[%d]=%d\n", i, fixed_slots[i]);
		pcm_slot_bitmap |= (1L << fixed_slots[i]);		
	}
		
	adss_pcm_send_receive(pkt_count, buf_size, sample_freq);

    return QCLI_STATUS_SUCCESS_E;	
}

