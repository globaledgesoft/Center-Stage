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

/**
 * @file util.c
 * @brief File contains utility functions.
 */

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <malloc.h>
#include <qapi_ver.h>
#include <qcli_api.h>
#include <qapi_ble_gap.h>
#include <qapi_ble_bttypes.h>
#include <qapi_reset.h>

#include "util.h"
#include "log_util.h"
#include "ble_util.h"

#define IOT_INFO          LOG_INFO
#define IOT_ERROR         LOG_ERROR
#define IOT_WARN          LOG_WARN
#define IOT_VERBOSE       LOG_VERBOSE

#define BOARD_NAME        CHIPSET_VARIANT
#define DEVID_STA          1

int32_t wlan_get_device_mac_address(uint32_t mode, char *mac);

void change_to_upper(char *s)
{
    while(*s)
    {
        if (*s >='a' && *s <='z')
            *s = *s - ('a' - 'A');
        s++;
    }
}

void get_dev_fw_version(char *buf, int32_t buf_size)
{
    qapi_FW_Info_t  info;
    if (qapi_Get_FW_Info(&info) == QAPI_OK)
    {
        snprintf(buf, buf_size, "\"QAPI Ver: %d.%d.%d CRM Num: %d\"",
                (int) (info.qapi_Version_Number&__QAPI_VERSION_MAJOR_MASK)>>__QAPI_VERSION_MAJOR_SHIFT,
                ((int) info.qapi_Version_Number&__QAPI_VERSION_MINOR_MASK)>>__QAPI_VERSION_MINOR_SHIFT,
                ((int) info.qapi_Version_Number&__QAPI_VERSION_NIT_MASK)>>__QAPI_VERSION_NIT_SHIFT, (unsigned int) info.crm_Build_Number);
    }
}

int32_t get_localdevice_name(char *buf, int32_t buf_size)
{
    char board_name[FW_VER_BUF] = {0};
    int rc = SUCCESS;

    memcpy(board_name, BOARD_NAME, sizeof(BOARD_NAME));
    change_to_upper(board_name);
#ifdef ONBOARD_VIA_WIFI
    {
        char mac[6] = { 0 };
        rc = wlan_get_device_mac_address(DEVID_STA, mac); 
        snprintf(buf+strlen(buf), buf_size,"\"%s_%02x%02x%02x\"", board_name, mac[3], mac[4], mac[5]);
    }
#endif
#ifdef ONBOARD_VIA_BLE
    {
        qapi_BLE_BD_ADDR_t BD_ADDR;
        if (!qapi_BLE_GAP_Query_Local_BD_ADDR(GetBluetoothStackID(), &BD_ADDR))
        {
			LOG_INFO("BD_ADDR: %02x:%02x:%02x:%02x:%02x:%02x\n", BD_ADDR.BD_ADDR5, BD_ADDR.BD_ADDR4, BD_ADDR.BD_ADDR3,
                        BD_ADDR.BD_ADDR2, BD_ADDR.BD_ADDR1, BD_ADDR.BD_ADDR0);
            snprintf(buf+strlen(buf), buf_size,"\"%s_%02x%02x%02x\"", board_name,
                       BD_ADDR.BD_ADDR2, BD_ADDR.BD_ADDR1, BD_ADDR.BD_ADDR0);
        }
	else
            rc = FAILURE;
    }
#endif
    
    return rc;
}

int32_t get_ble_localdevice_name(char *buf, int32_t buf_size)
{
    char board_name[FW_VER_BUF] = {0};
    int rc = SUCCESS;

    memcpy(board_name, BOARD_NAME, sizeof(BOARD_NAME));
    change_to_upper(board_name);
#ifdef ONBOARD_VIA_WIFI
    {
        char mac[6] = { 0 };
        rc = wlan_get_device_mac_address(DEVID_STA, mac);
        snprintf(buf+strlen(buf), buf_size,"\"%s_%02x%02x%02x\"", board_name, mac[3], mac[4], mac[5]);
    }
#endif
#ifdef ONBOARD_VIA_BLE
    {
        qapi_BLE_BD_ADDR_t BD_ADDR;
        if (!qapi_BLE_GAP_Query_Local_BD_ADDR(GetBluetoothStackID(), &BD_ADDR))
        {
            LOG_INFO("BD_ADDR: %02x:%02x:%02x:%02x:%02x:%02x\n", BD_ADDR.BD_ADDR5, BD_ADDR.BD_ADDR4, BD_ADDR.BD_ADDR3,
                    BD_ADDR.BD_ADDR2, BD_ADDR.BD_ADDR1, BD_ADDR.BD_ADDR0);
            snprintf(buf+strlen(buf), buf_size,"%s_%02x%02x%02x", board_name,
                    BD_ADDR.BD_ADDR2, BD_ADDR.BD_ADDR1, BD_ADDR.BD_ADDR0);
        }
        else
            rc = FAILURE;
    }
#endif

    return rc;
}

void restart_device(void)
{
    qapi_System_Reset();
}

