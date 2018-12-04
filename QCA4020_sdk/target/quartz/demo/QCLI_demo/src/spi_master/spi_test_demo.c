/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
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
#include "qurt_error.h"
#include "qurt_signal.h"
#include "qurt_thread.h"
#include "qapi_status.h"
#include "qapi_spi_master.h"

/* Defines */
#define THREAD_STACK_SIZE  1024

extern QCLI_Group_Handle_t qcli_peripherals_group; /* Handle for our peripherals subgroup. */

/* Globals */
QCLI_Group_Handle_t qcli_spim_group;               /* Handle for our QCLI Command Group. */
qurt_thread_attr_t spi_thread_attr;

/* Static variables */
static qapi_SPIM_Config_t spi_device_config;
static qurt_signal_t spi_Tx_done;
static qurt_signal_t start_spi, end_spi;
static qapi_SPIM_Transfer_t tx_info,rx_info;
static void *hSPI;
static uint8 *write_buf,*read_buf;
static uint32 sig_mask = 0x1111;
static qurt_thread_t spi_id;
static uint32 sig_start = 0x1, mask_end = 0x10000;


/* Prototypes of the send and receive functions */
QCLI_Command_Status_t spi_master_receive_data (uint32_t parameters_count, QCLI_Parameter_t * parameters);
QCLI_Command_Status_t spi_master_send_data (uint32_t parameters_count, QCLI_Parameter_t * parameters);
QCLI_Command_Status_t spi_master_local_loopback (uint32_t parameters_count, QCLI_Parameter_t * parameters);


const QCLI_Command_t spi_master_cmd_list[] =
{
   // cmd_function        start_thread          cmd_string               usage_string                   description
    { spi_master_send_data,       false,        "spim_snd_data",       "spim_snd_data 1/2 size",        "Send data on initialized spi master"},
    { spi_master_receive_data,    false,        "spim_rcv_data",       "spim_rcv_data 1/2 size",        "Receive data on specified spi master"},
    { spi_master_local_loopback,  false,        "spim_local_loopback", "spim_local_loopback 1/2 size",  "Local loop back function"},

};

const QCLI_Command_Group_t spi_master_cmd_group =
{
    "SPI Master",
    (sizeof(spi_master_cmd_list) / sizeof(spi_master_cmd_list[0])),
    spi_master_cmd_list
};

/* This function is used to register the wlan Command Group with    */
/* QCLI.                                                             */
void Initialize_SPI_Master_Demo(void)
{
   /* Attempt to reqister the Command Groups with the qcli framework.*/
   qcli_spim_group = QCLI_Register_Command_Group(qcli_peripherals_group, &spi_master_cmd_group);
   if(qcli_spim_group)
   {
      QCLI_Printf(qcli_spim_group, "Master SDCC Registered \n");
   }
}

uint32 spi_get_deviceid_func(uint32 u32InputParam, qapi_SPIM_Instance_t *Dev_id)
{
    switch (u32InputParam)
    {
        case 1:
        {
            *Dev_id = QAPI_SPIM_INSTANCE_1_E;
            return QCLI_STATUS_SUCCESS_E;
        }
        case 2:
        {
            *Dev_id = QAPI_SPIM_INSTANCE_2_E;
            return QCLI_STATUS_SUCCESS_E;
        }
        case 3:
        {
            *Dev_id = QAPI_SPIM_INSTANCE_3_E;
            return QCLI_STATUS_SUCCESS_E;
        }
        case 4:
        {
            *Dev_id = QAPI_SPIM_INSTANCE_4_E;
            return QCLI_STATUS_SUCCESS_E;
        }
        default:
        {
            QCLI_Printf(qcli_spim_group,"Invalid parmater for Dev ID \n");
            return QCLI_STATUS_USAGE_E;
        }
    }
}

static qapi_Status_t spi_config_func(qbool_t loopback_mode)
{
  spi_device_config.SPIM_Clk_Polarity =  QAPI_SPIM_CLK_IDLE_HIGH_E;
  spi_device_config.SPIM_Shift_Mode = QAPI_SPIM_INPUT_FIRST_MODE_E;
  spi_device_config.SPIM_CS_Polarity = QAPI_SPIM_CS_ACTIVE_LOW_E;
  spi_device_config.SPIM_CS_Mode = QAPI_SPIM_CS_DEASSERT_E;
  spi_device_config.SPIM_Clk_Always_On =  QAPI_SPIM_CLK_NORMAL_E;
  spi_device_config.SPIM_Bits_Per_Word = 8;
  spi_device_config.SPIM_Slave_Index = 0;
  spi_device_config.min_Slave_Freq_Hz = 0;
  spi_device_config.max_Slave_Freq_Hz = 50000000;
  spi_device_config.deassertion_Time_Ns = 0;
  spi_device_config.loopback_Mode = loopback_mode;
  spi_device_config.hs_Mode = 0;

  return QCLI_STATUS_SUCCESS_E;
}

void spi_io_complete_cb (uint32_t i_status, void *i_ctxt)
{
    qurt_signal_set(&spi_Tx_done, sig_mask);
}

uint8 write_buff_alloc(uint32 data_size)
{
    uint32 k;
    write_buf = (uint8 *) malloc (data_size);
    if (write_buf == NULL)
    {
        QCLI_Printf(qcli_spim_group,"failed to alloc Write transfer buffers\n");
        return QCLI_STATUS_ERROR_E;
    }
    for (k =0; k<data_size; k++)
    {
        write_buf[k] = k;
    }
    return QCLI_STATUS_SUCCESS_E;
}

uint8 read_buff_alloc(uint32 data_size)
{
    read_buf = (uint8 *) malloc (data_size);
    if (read_buf == NULL)
    {
        QCLI_Printf(qcli_spim_group,"failed to alloc Read transfer buffers\n");
        return QCLI_STATUS_USAGE_E;
    }
    memset (read_buf, 0XAA, data_size);
    return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t spi_master_send_data (uint32_t parameters_count, QCLI_Parameter_t * parameters)
{
    uint32 Test_Result = QCLI_STATUS_SUCCESS_E;
    uint32 instance, result = 0;
    uint32 data_size = 0;
    qapi_SPIM_Instance_t qup_instance;

    if (parameters_count < 2)
    {
        return QCLI_STATUS_ERROR_E;
    }
    instance = (uint32)parameters[0].Integer_Value;

    result = spi_get_deviceid_func(instance,&qup_instance);

    if(QCLI_STATUS_SUCCESS_E != result)
    {
        QCLI_Printf(qcli_spim_group,"Invalid SPI device ID - %u", instance);
        return QCLI_STATUS_ERROR_E;
    }
    QCLI_Printf(qcli_spim_group,"Opening handle to %d \n", qup_instance);

    /* Populate the spi config params */
    result = spi_config_func(0); /* Disable loopback */
    if(QCLI_STATUS_SUCCESS_E != result )
    {
        QCLI_Printf(qcli_spim_group,"Error in configuring SPI config params", NULL);
        return QCLI_STATUS_ERROR_E;
    }
    data_size = (uint32) parameters[1].Integer_Value;
    QCLI_Printf(qcli_spim_group,"Data size %d \n", data_size);

    qurt_signal_init(&spi_Tx_done);

    // Allocate the desc for write and read
    tx_info.buf_len = data_size;

    result = qapi_SPIM_Open(qup_instance, &hSPI);
    if (result != 0)
    {
        QCLI_Printf(qcli_spim_group,"spi open failed with err code %d \n",result);
        Test_Result = QCLI_STATUS_ERROR_E;
        return Test_Result;
    }

    result = qapi_SPIM_Enable(hSPI);
    if (result != 0)
    {
        QCLI_Printf(qcli_spim_group,"spi enable failed with err code %d \n",result);
        Test_Result = QCLI_STATUS_ERROR_E;
        return Test_Result;
    }

    result = write_buff_alloc(data_size);
    if (result != 0)
    {
        Test_Result = QCLI_STATUS_ERROR_E;
        return Test_Result;
    }

    tx_info.buf_phys_addr = write_buf;

    //Perform only Write (rx info = NULL)
    result = qapi_SPIM_Full_Duplex(hSPI, &spi_device_config, &tx_info, NULL, spi_io_complete_cb, NULL);
    if (result != 0)
    {
        QCLI_Printf(qcli_spim_group,"spi write failed with err %x\n",result);
        Test_Result = QCLI_STATUS_ERROR_E;
        return Test_Result;
    }

    // wait for test thread finish
    result = qurt_signal_wait(&spi_Tx_done, sig_mask, QURT_SIGNAL_ATTR_WAIT_ALL);
    if (result != sig_mask)
    {
        QCLI_Printf(qcli_spim_group,"waiting for test thread to finish - failed\n");
        Test_Result = QCLI_STATUS_ERROR_E;
        return Test_Result;
    }

    // free the tx buffers
    free(write_buf);
    qurt_signal_clear(&spi_Tx_done, sig_mask);

    result = qapi_SPIM_Disable(hSPI);
    if (result != 0)
    {
        QCLI_Printf(qcli_spim_group,"spi disable failed with err code %d \n",result);
        Test_Result = QCLI_STATUS_ERROR_E;
        return Test_Result;
    }

    QCLI_Printf(qcli_spim_group,"spi Tx tests Passed\n");

    result = qapi_SPIM_Close(hSPI);
    if (result != 0)
    {
        QCLI_Printf(qcli_spim_group,"spi close failed with err code %d \n",result);
        Test_Result = QCLI_STATUS_ERROR_E;
        return Test_Result;
    }
    qurt_signal_destroy(&spi_Tx_done);
    QCLI_Printf(qcli_spim_group,"spi Write only tests passed successfully\n" );
    return Test_Result;
}


QCLI_Command_Status_t spi_master_receive_data (uint32_t parameters_count, QCLI_Parameter_t * parameters)
{
    uint32 Test_Result = QCLI_STATUS_SUCCESS_E;
    uint32 l, instance, result = 0;
    uint32 data_size = 0;
    qapi_SPIM_Instance_t qup_instance;

    // First parameter - device ID
    if ( parameters_count < 2 )
    {
        return QCLI_STATUS_ERROR_E;
    }
    instance = (uint32)parameters[0].Integer_Value;

    result = spi_get_deviceid_func(instance,&qup_instance);

    if(QCLI_STATUS_SUCCESS_E != result)
    {
        QCLI_Printf(qcli_spim_group,"Invalid SPI device ID - %u", instance);
        return QCLI_STATUS_ERROR_E;
    }
    QCLI_Printf(qcli_spim_group,"Opening handle to %d \n", qup_instance);
    /* Populate the spi config params */
    result = spi_config_func(0); /* Disable loopback */
    if(QCLI_STATUS_SUCCESS_E != result )
    {
        QCLI_Printf(qcli_spim_group,"Error in configuring SPI config params", NULL);
        return QCLI_STATUS_ERROR_E;
    }
    data_size = (uint32) parameters[1].Integer_Value;
    QCLI_Printf(qcli_spim_group,"Data size %d \n", data_size);

    qurt_signal_init(&spi_Tx_done);

    // Allocate the desc for read

    rx_info.buf_len = data_size;

    result = qapi_SPIM_Open(qup_instance, &hSPI);
    if (result != 0)
    {
        QCLI_Printf(qcli_spim_group,"spi open failed with err code %d\n",result);
        Test_Result = QCLI_STATUS_ERROR_E;
        return Test_Result;
    }

    result = qapi_SPIM_Enable(hSPI);
    if (result != 0)
    {
        QCLI_Printf(qcli_spim_group,"spi enable failed with err code %d\n",result);
        Test_Result = QCLI_STATUS_ERROR_E;
        return Test_Result;
    }

    result = read_buff_alloc(data_size);
    if (result != 0)
    {
        Test_Result = QCLI_STATUS_ERROR_E;
        return Test_Result;
    }

    rx_info.buf_phys_addr = read_buf;

    //Perform only Read (tx info = NULL)
    result = qapi_SPIM_Full_Duplex(hSPI, &spi_device_config, NULL, &rx_info, spi_io_complete_cb, NULL);
    if (result != 0)
    {
        QCLI_Printf(qcli_spim_group,"spi read failed with err %x\n",result);
        Test_Result = QCLI_STATUS_ERROR_E;
        return Test_Result;
    }

    // wait for test thread finish
    result = qurt_signal_wait(&spi_Tx_done, sig_mask, QURT_SIGNAL_ATTR_WAIT_ALL);
    if (result != sig_mask)
    {
        QCLI_Printf(qcli_spim_group,"waiting for test thread to finish - failed\n");
        Test_Result = QCLI_STATUS_ERROR_E;
        return Test_Result;
    }
    for (l = 0; l < data_size; l++)
    {
        if (read_buf[l] != l)
        {
            QCLI_Printf(qcli_spim_group,"Expected - %d :But Read:%d\n", l, read_buf[l] );
            Test_Result = QCLI_STATUS_ERROR_E;
            return Test_Result;
        }
    }
    // free the tx buffers
    free(read_buf);
    qurt_signal_clear(&spi_Tx_done, sig_mask);

    result = qapi_SPIM_Disable(hSPI);
    if (result != 0)
    {
        QCLI_Printf(qcli_spim_group,"spi disable failed with err code %d\n",result);
        Test_Result = QCLI_STATUS_ERROR_E;
        return Test_Result;
    }

    result = qapi_SPIM_Close(hSPI);
    if (result != 0)
    {
        QCLI_Printf(qcli_spim_group,"spi close failed with err code %d\n",result);
        Test_Result = QCLI_STATUS_ERROR_E;
        return Test_Result;
    }

    qurt_signal_destroy(&spi_Tx_done);
    QCLI_Printf(qcli_spim_group,"spi Read only tests passed successfully\n" );
    return Test_Result;
}


void spi_tx_thread(void *arg)
{
    uint32 result = 0;
    while (1)
    {
        result = qurt_signal_wait(&start_spi, sig_start | mask_end, QURT_SIGNAL_ATTR_WAIT_ANY);
        qurt_signal_clear(&start_spi, result);
        if (result & mask_end)
        {
            break;
        }
        result = qapi_SPIM_Full_Duplex(hSPI, &spi_device_config, &tx_info, &rx_info, spi_io_complete_cb, NULL);
        if (result != 0)
        {
            QCLI_Printf(qcli_spim_group,"failed spi_transfer_1 %d\n", result);
        }
    }
    QCLI_Printf(qcli_spim_group,"Completed spi_tx_thread : exiting\n");
    qurt_signal_set(&end_spi, mask_end);
    qurt_thread_stop();
}


QCLI_Command_Status_t spi_master_local_loopback (uint32_t parameters_count, QCLI_Parameter_t * parameters)
{
    QCLI_Command_Status_t Test_Result = QCLI_STATUS_SUCCESS_E;
    uint32 instance, result = 0;
    uint32 k, data_size = 0;
    uint16 curr_priority;
    qapi_SPIM_Instance_t qup_instance;

    if (parameters_count < 2)
    {
        return QCLI_STATUS_ERROR_E;
    }

    instance = (uint32)parameters[0].Integer_Value;

    result = spi_get_deviceid_func(instance,&qup_instance);

    if(QCLI_STATUS_SUCCESS_E != result)
    {
        QCLI_Printf(qcli_spim_group,"Invalid SPI device ID - %u", instance);
        return QCLI_STATUS_ERROR_E;
    }
    QCLI_Printf(qcli_spim_group,"Opening handle to %d \n", qup_instance);

    /* Populate the spi config params */
    result = spi_config_func(1); /* Enable local loopback */
    if(QCLI_STATUS_SUCCESS_E != result )
    {
        QCLI_Printf(qcli_spim_group,"Error in configuring SPI config params", NULL);
        return QCLI_STATUS_ERROR_E;
    }
    data_size = (uint32) parameters[1].Integer_Value;
    QCLI_Printf(qcli_spim_group,"Data size %d \n", data_size);

    curr_priority = qurt_thread_get_priority(qurt_thread_get_id());
    qurt_thread_attr_init(&spi_thread_attr);
    qurt_thread_attr_set_name(&spi_thread_attr, "spi");
    qurt_thread_attr_set_priority(&spi_thread_attr, curr_priority + 5);
    qurt_thread_attr_set_stack_size(&spi_thread_attr, THREAD_STACK_SIZE);
    qurt_signal_init(&spi_Tx_done);
    qurt_signal_init(&start_spi);
    qurt_signal_init(&end_spi);

    result = qurt_thread_create(&spi_id, &spi_thread_attr, spi_tx_thread, NULL);
    if (result != QURT_EOK)
    {
        QCLI_Printf(qcli_spim_group,"err: failed to create %s\n", "spi");
        Test_Result = QCLI_STATUS_ERROR_E;
        return Test_Result;
    }
    else
    {
        QCLI_Printf(qcli_spim_group,"created %s successfully\n", "spi");
    }


    //Opening handle to the 1st instance
    result = qapi_SPIM_Open(qup_instance, &hSPI);
    if (result != 0)
    {
        QCLI_Printf(qcli_spim_group,"spi open failed with err code %d for instance %d\n",result,qup_instance);
        Test_Result = QCLI_STATUS_ERROR_E;
        return Test_Result;
    }

    //Clear the bit mask and destroy the qurt signals
    qurt_signal_clear(&spi_Tx_done, sig_mask);
    qurt_signal_clear(&start_spi, sig_start);

    // Allocate the desc for write and read
    tx_info.buf_len = rx_info.buf_len  =  data_size;

    result = write_buff_alloc(data_size);
    if (result != 0)
    {
        Test_Result = QCLI_STATUS_ERROR_E;
        return Test_Result;
    }
    result = read_buff_alloc(data_size);
    if (result != 0)
    {
        Test_Result = QCLI_STATUS_ERROR_E;
        return Test_Result;
    }

    tx_info.buf_phys_addr = write_buf;
    rx_info.buf_phys_addr = read_buf;

    result = qapi_SPIM_Enable(hSPI);
    if (result != 0)
    {
        QCLI_Printf(qcli_spim_group,"spi enable failed with err code %d for instance %d\n",result,qup_instance);
        Test_Result = QCLI_STATUS_ERROR_E;
        return Test_Result;
    }

    //Perform Write Tx

    // Set the signals to start the tx in both the threads
    qurt_signal_set(&start_spi, sig_start);

    // wait for test thread finish
    result = qurt_signal_wait(&spi_Tx_done, sig_mask, QURT_SIGNAL_ATTR_WAIT_ALL);
    if (result != sig_mask)
    {
        QCLI_Printf(qcli_spim_group,"waiting for 1st test thread to finish - failed\n");
        Test_Result = QCLI_STATUS_ERROR_E;
        return Test_Result;
    }

    for (k = 0; k < data_size; k++)
    {
        if (write_buf[k] != read_buf[k])
        {
            QCLI_Printf(qcli_spim_group,"Tx 1 : Mismatch btw Read & Wrtie data @ %dth byte for \n", k);
            QCLI_Printf(qcli_spim_group,"Wrote:%d : Read:%d\n",write_buf[k], read_buf[k] );
            Test_Result = QCLI_STATUS_ERROR_E;
            return Test_Result;
        }
    }
    // free the tx buffers
    free(write_buf);
    free(read_buf);

    result = qapi_SPIM_Disable(hSPI);
    if (result != 0)
    {
        QCLI_Printf(qcli_spim_group,"spi disable failed with err code %d\n",result);
        Test_Result = QCLI_STATUS_ERROR_E;
        return Test_Result;
    }

    result = qapi_SPIM_Close(hSPI);
    if (result != 0)
    {
        QCLI_Printf(qcli_spim_group,"spi close failed with err code %d for instance %d\n",result, qup_instance);
        Test_Result = QCLI_STATUS_ERROR_E;
        return Test_Result;

    }

    /* inform threads that test is done */
    qurt_signal_set(&start_spi, mask_end);
    /* wait for response the signal objects are not needed */
    qurt_signal_wait(&end_spi, mask_end, QURT_SIGNAL_ATTR_WAIT_ANY);


    /* allow other threads to finish */
    qurt_thread_sleep(200);
    qurt_signal_destroy(&spi_Tx_done);
    qurt_signal_destroy(&start_spi);
    qurt_signal_destroy(&end_spi);
    QCLI_Printf(qcli_spim_group,"spi full duplex Read Write tests passed successfully\n" );
    return Test_Result;
}

