/*
* Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
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
#include <string.h>

#include <qcli_api.h>
#include "qapi/qapi_status.h"

#include "qurt_signal.h"
#include <qapi_spi_master.h>

/* Handle for our QCLI Command Group. */
extern  QCLI_Group_Handle_t qcli_spi_group;

#define SPI_SIGNAL_TRNSFR_COMPLETE                     (1<<0)
#define SPI_DATA_SIZE                                  256

static void *spi_handle;
static qurt_signal_t spi_signal;

void spi_test_cb(uint32_t status, void *callback_ctx)
{
    qurt_signal_set(&spi_signal, SPI_SIGNAL_TRNSFR_COMPLETE);
}

QCLI_Command_Status_t spi_test(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    QCLI_Command_Status_t ret = QCLI_STATUS_ERROR_E;
    int32_t en_loopback = 1, i = 0, data_matches = 1;
    qapi_SPIM_Transfer_t tx_info;
    qapi_SPIM_Transfer_t rx_info;
    qapi_SPIM_Config_t config;
    char *spi_tx_buf;
    char *spi_rx_buf;

    if (Parameter_Count != 1)
    {
        QCLI_Printf(qcli_spi_group, "USAGE: SPI_test mode\n");
        QCLI_Printf(qcli_spi_group, "mode 0/1  0: disable loopback  1: enable loopback\n");
        return QCLI_STATUS_SUCCESS_E;
    }

    en_loopback = Parameter_List[0].Integer_Value;

    config.SPIM_Clk_Polarity = QAPI_SPIM_CLK_IDLE_LOW_E;
    config.SPIM_Shift_Mode = QAPI_SPIM_INPUT_FIRST_MODE_E;
    config.SPIM_CS_Polarity = QAPI_SPIM_CS_ACTIVE_LOW_E;
    config.SPIM_CS_Mode = QAPI_SPIM_CS_DEASSERT_E;
    config.SPIM_Clk_Always_On = QAPI_SPIM_CLK_NORMAL_E;
    config.SPIM_Bits_Per_Word = 8;
    config.SPIM_Slave_Index = 0;
    config.min_Slave_Freq_Hz = 0;
    config.max_Slave_Freq_Hz = 50000000;
    config.deassertion_Time_Ns = 0;
    config.loopback_Mode = en_loopback;
    config.hs_Mode = 0;

    qurt_signal_init(&spi_signal);

    spi_tx_buf = malloc(SPI_DATA_SIZE);
    if (spi_tx_buf == NULL)
          return -1;

    spi_rx_buf = malloc(SPI_DATA_SIZE);
    if (spi_rx_buf == NULL)
          return -1;

    memset(spi_tx_buf, 0xA5, SPI_DATA_SIZE);
    memset(spi_rx_buf, 0, SPI_DATA_SIZE);

    tx_info.buf_phys_addr = spi_tx_buf;
    tx_info.buf_len = SPI_DATA_SIZE;

    rx_info.buf_phys_addr = spi_rx_buf;
    rx_info.buf_len = SPI_DATA_SIZE;

    if (QCLI_STATUS_SUCCESS_E != qapi_SPIM_Open(QAPI_SPIM_INSTANCE_1_E, &spi_handle))
    {
        QCLI_Printf(qcli_spi_group, "SPI Open failed!\n");
        goto end;
    }

    if (QCLI_STATUS_SUCCESS_E != qapi_SPIM_Enable(spi_handle))
    {
        QCLI_Printf(qcli_spi_group, "SPI Enable failed!\n");
        goto end;
    }

    if (qapi_SPIM_Full_Duplex(spi_handle, &config, &tx_info, &rx_info, spi_test_cb, NULL) != QAPI_OK)
    {
        QCLI_Printf(qcli_spi_group, "SPI FullDuplex failed!\n");
        goto end;
    }

    qurt_signal_wait(&spi_signal, SPI_SIGNAL_TRNSFR_COMPLETE, QURT_SIGNAL_ATTR_CLEAR_MASK);

    for (i = 0; i < SPI_DATA_SIZE; i++)
    {
        if (spi_tx_buf[i] != spi_rx_buf[i])
        { 
            data_matches = 0;
            break;
        }
    }

    if (data_matches)
        QCLI_Printf(qcli_spi_group,"Data Transfered and Received are Same\n");
    else
        QCLI_Printf(qcli_spi_group,"Data Transfered and Received are Not Same\n");

    if (QAPI_OK != qapi_SPIM_Disable(spi_handle))
    {
        QCLI_Printf(qcli_spi_group, "SPI Disable failed\n");
        goto end;
    }

    if (QAPI_OK != qapi_SPIM_Close(spi_handle)) 
    {
        QCLI_Printf(qcli_spi_group, "SPI close failed\n");
        goto end;
    }

    ret = QCLI_STATUS_SUCCESS_E;

end:
    qurt_signal_delete(&spi_signal);

    free(spi_tx_buf);
    free(spi_rx_buf);

    return ret;
}
