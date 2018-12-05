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
#include <stdlib.h>
#include "qurt_signal.h"

#include "qapi/qurt_thread.h"
#include "stdint.h"
#include "qapi/qapi_status.h"

#include "qapi_i2s.h"
#include "qapi_pcm.h"
#include <qcli.h>
#include <qcli_api.h>
#include "malloc.h"
#include "adss_demo.h"

uint32_t  audio_rcv_data[MAX_BUFF_COUNT][MAX_DATA_BUFF_SIZE];
uint8_t   cur_rcv_ndx = 0;
uint32_t  audio_cur_ndx = 0;

qurt_signal_t   send_done_signal;

volatile  int32_t   total_count;
volatile  uint32_t   loop_count=0;
volatile  qapi_I2S_Handle  hdI2S;
volatile  uint32_t   send_count=0;
volatile  uint32_t   send_data_length=0;

volatile  uint32_t  pkt_cnt=0;
volatile  uint32_t  signal_cnt=0;

uint32_t get_audio_data(uint8_t  *buf, uint32_t size)
{
	uint32_t   i;
	uint32_t  *ptr_32_1, *ptr_32_2;
	
	if (audio_data_size - audio_cur_ndx < size)
		return 0;
	
	ptr_32_1 = (uint32_t *)buf;
	ptr_32_2 = (uint32_t *)(&audio_data[audio_cur_ndx/2]);

	for (i=0; i<size/sizeof(uint32_t); i++)
		*ptr_32_1 ++ = *ptr_32_2++;
	
	audio_cur_ndx += size;
	
	return size;
}

qapi_I2S_Channel_Config_t i2s_port0_config = {
    0,  							/* receive dma descriptors count */
    6, 								/* send dma descriptors count */
    64, 							/* data buffer size */
    I2S_FREQUENCY, 					/* transfer freq such as 32KHz, 44.1KHz, ... */
    16, 							/* PCM word size 0-8bit 1-16bit 2-24bit 3-32bit */
    16, 							/* I2S word size 0- 16bit, 1 -32bits */
    16, 							/* mic input word size 0- 16bit, 1 -32bits */
    (__QAPI_I2S_MODE_MASTER | __QAPI_I2S_MODE_DIR_OUTPUT), /* work mode master/stereo/output */
    0, 								/* channel number 0, 1, 2, 3 */
    QAPI_I2S_CLK_SRC_PLL_E, 				/* select clock source */
};

void adss_mbox_dma_callback(void *hd, uint32_t status, void *param)
{
    send_count++;

    {
		pkt_cnt++;
		
		if (pkt_cnt >= 3)
		{
			pkt_cnt = 0;		
			{
				total_count --;
				qurt_signal_set(&send_done_signal, ADSS_USR_SEND_DONE_SIG_MASK);
				signal_cnt ++;
			}
		}
    }
}

/* I2S send test */
int32_t adss_driver_test( void *pvParameters )
{
  int i;
  qapi_I2S_Instance_e inst;
  qapi_Status_t  status;
  uint32_t       sent_len, len, *pINT32;

  inst = QAPI_I2S_INSTANCE_001_E;
  status = qapi_I2S_Init(inst, &i2s_port0_config, (void **)&hdI2S);
  if ( QAPI_OK != status ) 
      return -1;

  qapi_I2S_Open(hdI2S);

  status = qapi_I2S_Intr_Register (hdI2S, adss_mbox_dma_callback, 0);
  if ( QAPI_OK != status ) 
      return -2;

  for (i=0, pINT32=audio_rcv_data[0]; i < sizeof(audio_rcv_data[0]); i++)
      *pINT32++ = 0x15;

  total_count = 2;
  
  audio_cur_ndx = 0;
  
  get_audio_data((uint8_t *)(audio_rcv_data[0]), sizeof(audio_rcv_data[0]));
  qurt_signal_init(&send_done_signal);

  do
  {
	  qapi_I2S_Send_Data(hdI2S, (uint8_t *)(audio_rcv_data[0]), sizeof(audio_rcv_data[0]), &sent_len);
	  len = get_audio_data((uint8_t *)(audio_rcv_data[0]), sizeof(audio_rcv_data[0]));
	  
      qurt_signal_wait(&send_done_signal, ADSS_USR_SEND_DONE_SIG_MASK, QURT_SIGNAL_ATTR_CLEAR_MASK);
  } while (len != 0);
	  
  total_count = 0;

  qurt_signal_delete(&send_done_signal);
  
  qapi_I2S_Deinit (hdI2S);
  return 0;
}

/*
 * I2S send & receive test
 */
qapi_I2S_Channel_Config_t i2s_port3_config = {
    0,  							/* receive dma descriptors count */
    6,  							/* send dma descriptors count */
    64,								/* data buffer size */
    I2S_FREQUENCY,					/* transfer freq such as 32KHz, 44.1KHz, ... */
    16,								/* PCM word size 0-8bit 1-16bit 2-24bit 3-32bit */
    16, 							/* I2S word size 0- 16bit, 1 -32bits */
    16,								/* mic input word size 0- 16bit, 1 -32bits */
    __QAPI_I2S_MODE_MASTER,			/* work mode master/stereo/output */
    3,								/* channel number 0, 1, 2, 3 */
    QAPI_I2S_CLK_SRC_PLL_E, 		/* select clock source */
};

qapi_I2S_Handle  hdI2S_in;
uint32_t  rcv_len;

qurt_signal_t   receive_done_signal;

void adss_mbox_dma_snd_callback(void *hd, uint32_t status, void *param)
{
    send_count++;
}

void adss_mbox_dma_rcv_callback(void *hd, uint32_t status, void *param)
{
    loop_count++;

	pkt_cnt++;
	if (pkt_cnt >= 3)
    {
		pkt_cnt = 0;

		qurt_signal_set(&receive_done_signal, ADSS_USR_SEND_DONE_SIG_MASK);
		signal_cnt ++;
    }
}

int32_t adss_driver_test_receive( void *pvParameters )
{
  int i;
  qapi_I2S_Instance_e inst;
  qapi_Status_t  status;
  uint32_t       *pINT32;

  inst = QAPI_I2S_INSTANCE_004_E;
  status = qapi_I2S_Init(inst, &i2s_port3_config, (void **)&hdI2S_in);
  if ( QAPI_OK != status ) 
     return -1;

  status = qapi_I2S_Open(hdI2S_in);
  if ( QAPI_OK != status ) 
     return -2;

  status = qapi_I2S_Intr_Register (hdI2S_in, adss_mbox_dma_rcv_callback, 0);
  if ( QAPI_OK != status ) 
     return -3;

  for (i=0, pINT32=audio_rcv_data[0]; i < sizeof(audio_rcv_data[0]); i++)
      *pINT32++ = 0x55;

  total_count = 2;

  qurt_signal_init(&receive_done_signal);

  audio_echo_loop_flag = 1;
  
  do
  {
      qapi_I2S_Receive_Data(hdI2S_in, (uint8_t *)audio_rcv_data[0], sizeof(audio_rcv_data[0]), &rcv_len);
      qurt_signal_wait(&receive_done_signal, ADSS_USR_SEND_DONE_SIG_MASK, QURT_SIGNAL_ATTR_CLEAR_MASK);
  } while (audio_echo_loop_flag);

  
  qurt_signal_delete(&receive_done_signal);
  
  qapi_I2S_Deinit (hdI2S);
  
  return 0;
}

/*
 *  audio loop test
 *
 */

qapi_I2S_Channel_Config_t  i2s_audio_port0_config = {
    0,  				/* receive dma descriptors count */
    3,  				/* send dma descriptors count */
    64,  				/* data buffer size */
    I2S_FREQUENCY,  	/* transfer freq such as 32KHz, 44.1KHz, ... */
    16, 				/* PCM word size 0-8bit 1-16bit 2-24bit 3-32bit */
    16, 				/* I2S word size 0- 16bit, 1 -32bits */
    16, 				/* mic input word size 0- 16bit, 1 -32bits */
    (__QAPI_I2S_MODE_MASTER | __QAPI_I2S_MODE_DIR_OUTPUT), /* work mode master/stereo/output */
    0, 					/* channel number 0, 1, 2, 3 */
    QAPI_I2S_CLK_SRC_PLL_E, 	/* select clock source */
};

qapi_I2S_Channel_Config_t i2s_audio_port3_config = {
    0, 				/* receive dma descriptors count */
    3, 				/* send dma descriptors count */
    64, 			/* data buffer size */
    I2S_FREQUENCY, 	/* transfer freq such as 32KHz, 44.1KHz, ... */
    16, 			/* PCM word size 0-8bit 1-16bit 2-24bit 3-32bit */
    16, 			/* I2S word size 0- 16bit, 1 -32bits */
    16, 			/* mic input word size 0- 16bit, 1 -32bits */
    0, 				/* work mode master/stereo/output */
    3, 				/* channel number 0, 1, 2, 3 */
    QAPI_I2S_CLK_SRC_PLL_E, /* select clock source */
};

void adss_mbox_audio_dma_snd_callback(void *hd, uint32_t status, void *param)
{
    send_count++;
}

void adss_mbox_audio_dma_rcv_callback(void *hd, uint32_t status, void *param)
{
    loop_count++;

    pkt_cnt++;

    if (pkt_cnt >= 3)
    {
		pkt_cnt = 0;
		qurt_signal_set(&receive_done_signal, ADSS_USR_SEND_DONE_SIG_MASK);
		signal_cnt ++;
    }
}

volatile uint8_t *pRcvBuf;

void adss_mbox_data_dma_snd_callback(void *hd, uint32_t status, void *param)
{
    send_count++;	
}

void adss_mbox_data_rcv_callback(void *hd, uint32_t status, void *param)
{
    uint32_t       sent_len;
    loop_count++;

	pRcvBuf = (uint8_t *)param;

	if (pRcvBuf != 0)
	{
		send_data_length ++;				
		qapi_I2S_Send_Data(hdI2S, (uint8_t *)pRcvBuf, 0, &sent_len);		
	}

	qurt_signal_set(&receive_done_signal, ADSS_USR_SEND_DONE_SIG_MASK);
}

int32_t adss_driver_data_loop(qapi_I2S_Freq_e freq, uint32_t buf_size, uint32_t pkt_count )
{
  uint32_t      i, j, *pINT32;
  qapi_I2S_Instance_e inst;
  qapi_Status_t  status;
  uint8_t       **pbuf_v, **rpbuf_v;
  uint8_t       *pbuf;
  
  i2s_audio_port3_config.freq = freq;
  i2s_audio_port3_config.buf_Size = buf_size;
  i2s_audio_port3_config.num_Rx_Desc = pkt_count;
  i2s_audio_port3_config.num_Tx_Desc = 0;
  
  i2s_audio_port0_config.buf_Size = buf_size;
  i2s_audio_port0_config.num_Tx_Desc = pkt_count;
  i2s_audio_port0_config.num_Rx_Desc = 0;
  i2s_audio_port0_config.freq = freq;	  
  
  pbuf_v = (uint8_t **)malloc(sizeof(uint8_t *) * pkt_count);
  if (pbuf_v == NULL)
	  return -1;
  
  rpbuf_v = (uint8_t **)malloc(sizeof(uint8_t *) * pkt_count);
  if (rpbuf_v == NULL)
  {
	  free(pbuf_v);
	  return -1;
  }
  
  inst = QAPI_I2S_INSTANCE_004_E;
  status = qapi_I2S_Init(inst, &i2s_audio_port3_config, (void **)&hdI2S_in);
  if (status != QAPI_OK)
  {
	 QCLI_Printf(qcli_adss_group, "in port fail=%d\n", status);		  
	 return -1; 
  }
  qapi_I2S_Open(hdI2S_in);
  status = qapi_I2S_Intr_Register (hdI2S_in, adss_mbox_data_rcv_callback, 0);

  inst = QAPI_I2S_INSTANCE_001_E;
  status = qapi_I2S_Init(inst, &i2s_audio_port0_config, (void **)&hdI2S);
  if (status != QAPI_OK)
  {
	 QCLI_Printf(qcli_adss_group, "output port fail=%d\n", status);		  
	 return -1; 
  } 
  qapi_I2S_Open(hdI2S);
  status = qapi_I2S_Intr_Register (hdI2S, adss_mbox_data_dma_snd_callback, 0);

  total_count = 0;

  qurt_signal_init(&receive_done_signal);
  
  pkt_cnt = 0;
  
  for (i=0; i < pkt_count; i++)
  {
		status = qapi_I2S_Get_Buffer(&pbuf_v[i]);
		pINT32 = (uint32_t *)pbuf_v[i]; 
		for (j=0; j < buf_size/4; j++)
			*pINT32++ = 0xAA55BB66; 	  
  }

  for (i=0; i < pkt_count - 1; i++)
	 status = qapi_I2S_Get_Buffer(&rpbuf_v[i]);
  
  qapi_I2S_Send_Receive(hdI2S, pbuf_v, pkt_count, hdI2S_in, rpbuf_v, pkt_count - 1);

  audio_echo_loop_flag = 1;

  do
  {
	status = qapi_I2S_Get_Buffer(&pbuf);
	if (status != QAPI_OK)
	{
		QCLI_Printf(qcli_adss_group, "alloc rcv buf fail\n");						
	}
	else
		qapi_I2S_Receive_Data(hdI2S_in, pbuf, 0, &rcv_len);

	qurt_signal_wait(&receive_done_signal, ADSS_USR_SEND_DONE_SIG_MASK, QURT_SIGNAL_ATTR_CLEAR_MASK);
	
    total_count ++;
  } while (audio_echo_loop_flag);

  total_count = 0;
  
  qurt_signal_delete(&receive_done_signal);

  qapi_I2S_Deinit (hdI2S);
  qapi_I2S_Deinit (hdI2S_in);
  
  free(pbuf_v);
  free(rpbuf_v);
  
  return 0; 
}

/*
 *
 */

qapi_I2S_Handle  hd_pcm_out;
qapi_I2S_Handle  hd_pcm_in;

#define  SLOTS    __QAPI_PCM_SLOT_00

				  
qapi_PCM_Config_t pcm_config = {
    __QAPI_PCM_MODE_DIR_OUTPUT,    				/** mode:operate mode  */          
    PCM_FREQUENCY,								/** freq:transfer freq such as 8Khz, 16KHZ, 32KHz, ... */
	SLOTS,       								/** slots:indicates PCM slots that will be used */            
	QAPI_PCM_MODE_DMA_E, 						/** cpu_mode : indicates PCM work mode */
	QAPI_PCM_CLK_MODE_SINGLE_E, 				/** clk_mode : indicates PCM clock mode */
	QAPI_PCM_SLOT_MODE_16BITS_E, 				/** slote_mode :  indicates PCM slot mode */
	QAPI_PCM_FRAME_SYNC_ONE_CLK_E, 				/** frame_sync_len : indicates PCM frame synchronous signal length */
	QAPI_PCM_GATE_CLK_ON_E, 					/** gate_clk_en : indicates PCM gate clock enable/disable */
	QAPI_PCM_TXRX_PHASE_POSITIVE_E, 			/** rx_phase : indicates PCM receive clock phase */
	QAPI_PCM_TXRX_PHASE_POSITIVE_E, 			/** tx_phase : indicates PCM transmit clock phase */
	0,  										/** loop_RX2TX : indicates PCM loop from RX to TX */
	0,  										/** loop_TX2RX : indicates PCM loop from TX to RX */	
	160, 										/** rx_threshold : indicates PCM receive data FIFO threshold */
	160, 										/** tx_threshold : indicates PCM transmit data FIFO threshold */
	0,  										/** pcm_in_offset : indicates PCM input offset */
	0,  										/** pcm_out_offset : indicates PCM output offset */		
    6,  										/** num_tx_desc:indicates PCM tx descriptor number */
    0,  										/** num_rx_desc:indicates PCM rx descriptor number */
    64,  										/** i2s_buf_size:indicates PCM data buffer size */
};

qapi_PCM_Config_t pcm_in_config = {
    0,      									/** mode:operate mode  */          
    PCM_FREQUENCY,								/** freq:transfer freq such as 8Khz, 16KHZ, 32KHz, ... */
	SLOTS,       								/** slots:indicates PCM slots that will be used */            
	QAPI_PCM_MODE_DMA_E, 						/** cpu_mode : indicates PCM work mode */
	QAPI_PCM_CLK_MODE_SINGLE_E, 				/** clk_mode : indicates PCM clock mode */
	QAPI_PCM_SLOT_MODE_16BITS_E,				/** slote_mode :  indicates PCM slot mode */
	QAPI_PCM_FRAME_SYNC_ONE_CLK_E,				/** frame_sync_len : indicates PCM frame synchronous signal length */
	QAPI_PCM_GATE_CLK_ON_E, 					/** gate_clk_en : indicates PCM gate clock enable/disable */
	QAPI_PCM_TXRX_PHASE_POSITIVE_E,				/** rx_phase : indicates PCM receive clock phase */
	QAPI_PCM_TXRX_PHASE_POSITIVE_E, 			/** tx_phase : indicates PCM transmit clock phase */
	0,  										/** loop_RX2TX : indicates PCM loop from RX to TX */
	0,  										/** loop_TX2RX : indicates PCM loop from TX to RX */	
	160, 										/** rx_threshold : indicates PCM receive data FIFO threshold */
	160, 										/** tx_threshold : indicates PCM transmit data FIFO threshold */
	0,  										/** pcm_in_offset : indicates PCM input offset */
	0,  										/** pcm_out_offset : indicates PCM output offset */		
    6,  										/** num_tx_desc:indicates PCM tx descriptor number */
    0,  										/** num_rx_desc:indicates PCM rx descriptor number */
    64,  										/** i2s_buf_size:indicates PCM data buffer size */
};

qurt_signal_t pcm_rcv_done_signal;

void adss_pcm_dma_rcv_callback(void *hd, uint32_t status, void *param)
{
    send_count++;
	
	pkt_cnt++;
    if (pkt_cnt >= 3)
    {
		pkt_cnt = 0;
		qurt_signal_set(&pcm_rcv_done_signal, ADSS_USR_SEND_DONE_SIG_MASK);
    }
}

void adss_pcm_dma_snd_callback(void *hd, uint32_t status, void *param)
{
    loop_count++;	
}

void init_pcm_data ()
{
	uint32_t   i, j;
	uint32_t   slot_mask = pcm_config.slots;
	uint32_t   cur_slot_mask = 1;
	
	j = 0;
	for (i=0; i < MAX_DATA_BUFF_SIZE; i++)
	{
		while ((slot_mask & cur_slot_mask) == 0)
		{
			j ++;
			cur_slot_mask <<= 1;	
			
			if (j >= 16)
			{
				j = 0;
				cur_slot_mask = 0;
			}
		}
		audio_rcv_data[0][i] = (j << 16) | 0xAAAA;
	}	
}

uint32_t       pkt_count = 0;
uint32_t       pkt_total = 0;

void adss_pcm_echo(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List )
{	
	qapi_Status_t  status;
    uint32_t       sent_len;
		
	status = qapi_PCM_Init(&pcm_config, &hd_pcm_out);
	if (status != QAPI_OK)
	{
		QCLI_Printf(qcli_adss_group, "pcm init fail=%d\n", status);		  
		return; 
	}
	
	status = qapi_PCM_Open(hd_pcm_out);
    status = qapi_PCM_Intr_Register(hd_pcm_out, adss_pcm_dma_snd_callback, 0);

	status = qapi_PCM_Init(&pcm_in_config, &hd_pcm_in);
	status = qapi_PCM_Open(hd_pcm_in);
    status = qapi_PCM_Intr_Register(hd_pcm_in, adss_pcm_dma_rcv_callback, 0);

    qurt_signal_init(&pcm_rcv_done_signal);
	pkt_cnt = 0;

	init_pcm_data ();

	qapi_PCM_BiDir(hd_pcm_out, (uint8_t *)(audio_rcv_data[0]), sizeof(audio_rcv_data[0]), &sent_len,
                    hd_pcm_in, (uint8_t *)audio_rcv_data[0], sizeof(audio_rcv_data[0]), &rcv_len);

    do
    {
		qapi_PCM_Receive_Data(hd_pcm_in, (uint8_t *)audio_rcv_data[0], sizeof(audio_rcv_data[0]), &rcv_len);
		qurt_signal_wait(&pcm_rcv_done_signal, ADSS_USR_SEND_DONE_SIG_MASK, QURT_SIGNAL_ATTR_CLEAR_MASK);
		
	    qapi_PCM_Send_Data(hd_pcm_out, (uint8_t *)(audio_rcv_data[0]), sizeof(audio_rcv_data[0]), &sent_len);
		pkt_count ++;
		
	} while (pkt_total == 0 || pkt_count < pkt_total);
	
    qurt_signal_delete(&pcm_rcv_done_signal);
	
	qapi_PCM_Deinit(&hd_pcm_out);	
}
