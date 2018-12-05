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

#include <qcli.h>
#include <qcli_api.h>
#include <qapi/qapi_socket.h>
#include <qapi_netservices.h>
#include <qapi_ns_gen_v4.h>
#include <qapi_ns_utils.h>
#include <qapi_wlan.h>

#include <qurt_thread.h>
#include <qurt_timer.h>
#include <qurt_error.h>
#include <qurt_signal.h>

#include "qapi_fatal_err.h"

#include "wifi_util.h"
#include "log_util.h"

/*-------------------------------------------------------------------------
  - Preprocessor Definitions and Constants  Global Variables
  ------------------------------------------------------------------------*/

typedef int boolean_t;
uint8_t wlan_enabled = 0;
uint8_t g_bssid[WLAN_NUM_OF_DEVICES][__QAPI_WLAN_MAC_LEN];
qurt_signal_t *pwlan_sigevent;

/* TODO: Assumption is 2 VDEV are supported.
 * VDEV 0 is used as SAP for onboarding
 * VDEV 1 is used to connect to HomeAP
 */

/*FUNCTION*-----------------------------------------------------------------
 *
 * Function Name  : wlan_connect_handler
 * Returned Value : N/A
 * Comments       : Called from driver on a WiFI connection event
 *
 *END------------------------------------------------------------------*/

static void wlan_connect_handler(uint8_t deviceId, int val, uint8_t *mac, boolean_t bssConn)
{
    uint8_t temp_bss[__QAPI_WLAN_MAC_LEN] = {0};

    LOG_INFO("Connect event on devId:%d val:%d\r\n", deviceId, val);

    if (val == TRUE)
    {
        LOG_INFO("devid - %d %d %s MAC addr %02x:%02x:%02x:%02x:%02x:%02x \n",
                deviceId, bssConn, "CONNECTED", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        if(bssConn)
        {
            memcpy(g_bssid[deviceId], mac, __QAPI_WLAN_MAC_LEN);
        }

        if (deviceId == DEVID_STA)
        {
            qurt_signal_set(pwlan_sigevent, STA_SIGNAL_CONNECT_EVENT);
        }
        else if (deviceId == DEVID_SAP)
            qurt_signal_set(pwlan_sigevent, AP_SIGNAL_CONNECT_EVENT);

        //TODO: TURN_LED_ON;
    }
    else if (val == QAPI_WLAN_INVALID_PROFILE_E) // this event is used to indicate RSNA failure
    {
        LOG_ERROR("4 way handshake failure for device=%d \n",deviceId);
    }
    else if (val == 0x10 /*PEER_FIRST_NODE_JOIN_EVENT*/) //this event is used to RSNA success
    {
        LOG_INFO("4 way handshake success for device=%d \n",deviceId);
    }
    else if (val == FALSE)
    {
        /*   dev status for disconnect events
             MODE          wmi --disc event     REF_STA Disconnect
             STA MODE        DOWN               INVALID
             AP  MODE        DOWN                UP
             P2P CLIENT......DOWN               INVALID
             P2P GO..........DOWN                UP
             */
        if (bssConn)
        {
            if((memcmp(mac, temp_bss,__QAPI_WLAN_MAC_LEN) == 0) && (memcmp(g_bssid[deviceId], temp_bss,__QAPI_WLAN_MAC_LEN) != 0))
            {
                LOG_INFO("devId %d Disconnected MAC addr %02x:%02x:%02x:%02x:%02x:%02x \r\n",
                        deviceId, g_bssid[deviceId][0], g_bssid[deviceId][1], g_bssid[deviceId][2],
                        g_bssid[deviceId][3], g_bssid[deviceId][4], g_bssid[deviceId][5]);
            }
            else
            {
                LOG_INFO("devId %d Disconnected MAC addr %02x:%02x:%02x:%02x:%02x:%02x \r\n",
                        deviceId,mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
            }

        }
        /* bssConn is flase for REF-STA disconnected from AP/p2pGO: dont update the dev stat*/
        else
        {
            LOG_INFO("REF_STA Disconnected MAC addr %02x:%02x:%02x:%02x:%02x:%02x devId %d\r\n",
                    mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], deviceId);
        }
        memcpy(g_bssid[deviceId],temp_bss,__QAPI_WLAN_MAC_LEN);

        if (deviceId == DEVID_STA)
        {
            qurt_signal_set(pwlan_sigevent, STA_SIGNAL_DISCONNECT_EVENT);
            LOG_INFO("DEVID_STA: %s\n", g_bssid[deviceId]);
        }
    }
    else
    {
        LOG_INFO("last tx rate : %d kbps--for device=%d\r\n", val, deviceId);
    }
}

static void wlan_callback_handler(uint8_t deviceId,
        uint32_t cbID,
        void *application_Context,
        void  *payload,
        uint32_t payload_Length)
{
    qapi_WLAN_Connect_Cb_Info_t *cxnInfo  = (qapi_WLAN_Connect_Cb_Info_t *)(payload);

    LOG_INFO("WLANCB: dID:%d, cbID:%u, val:%d\n", deviceId, cbID, cxnInfo->value);

    switch(cbID)
    {
        case QAPI_WLAN_CONNECT_CB_E:
            {
                wlan_connect_handler(deviceId, cxnInfo->value, cxnInfo->mac_Addr, cxnInfo->bss_Connection_Status);
            }
            break;
        default:
            break;
    }
}

/*FUNCTION*-----------------------------------------------------------------
 *
 * Function Name  : wlan_set_callback
 * Returned Value : -1 on error else 0
 * Comments       : Sets callback function for WiFi connect/disconnect event
 *
 *END------------------------------------------------------------------*/

static int32_t wlan_set_callback(uint32_t deviceId, const void *appCtx)
{
    int32_t ret;

    ret = qapi_WLAN_Set_Callback(deviceId, (qapi_WLAN_Callback_t)wlan_callback_handler, appCtx);
    if (QAPI_OK != ret)
    {
        LOG_ERROR("Set Callback ret:%d\n", ret);
        return FAILURE;
    }
    return SUCCESS;
}

static int32_t wlan_set_power_mode(uint32_t deviceId, boolean_t pwr_mode, uint8_t pwr_module)
{
    qapi_WLAN_Power_Mode_Params_t pwrMode;

    pwrMode.power_Mode = pwr_mode;
    pwrMode.power_Module = pwr_module;
    return ((qapi_WLAN_Set_Param (deviceId,
                    __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                    __QAPI_WLAN_PARAM_GROUP_WIRELESS_POWER_MODE_PARAMS,
                    (void *) &pwrMode,
                    sizeof(pwrMode),
                    FALSE) == QAPI_OK) ? SUCCESS : FAILURE);

}

/*FUNCTION*---------------------------------------------------------------------------------------------------
*
* Function Name  : wlan_is_device_connected
* Returned Value : 0 on success, -1 on error.
* Comments       : Called from application to retrieve if device is connected to a network.
*
*END----------------------------------------------------------------------------------------------------------*/
int32_t wlan_is_device_connected(uint32_t deviceId)
{
    uint8_t temp_bss[__QAPI_WLAN_MAC_LEN] = {0};
    if(memcmp(g_bssid[deviceId], temp_bss, __QAPI_WLAN_MAC_LEN))
    {
        return SUCCESS;
    }
    return FAILURE;
}


int32_t wlan_get_device_mac_address(uint8_t devId, char *mac)
{
    char data[32+1] = {'\0'};
    uint32_t data_len = 0;

    if (0 != qapi_WLAN_Get_Param (devId,
                         __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                         __QAPI_WLAN_PARAM_GROUP_WIRELESS_MAC_ADDRESS,
                         &data,
                         &data_len))
    {
        LOG_ERROR("Command failed.\r\n");
        return FAILURE;
    }
    LOG_VERBOSE("Mac Addr = %02x:%02x:%02x:%02x:%02x:%02x\n",
              data[0], data[1], data[2], data[3], data[4], data[5]);
    memcpy(mac, data, __QAPI_WLAN_MAC_LEN);

    return SUCCESS;
}

/*----------------------------------------------------------------------------
  - set_operate_mode: Set the operation mode either STA or AP
  ---------------------------------------------------------------------------*/
int32_t wlan_set_operate_mode(uint32_t deviceId)
{
    qapi_WLAN_Dev_Mode_e opMode;
    uint32_t len;

    if (QAPI_OK != qapi_WLAN_Get_Param(deviceId,
                __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                __QAPI_WLAN_PARAM_GROUP_WIRELESS_OPERATION_MODE,
                &opMode,
                &len ))
    {
        LOG_ERROR("Not able to get operation mode\n");
        //return FAILURE;
    }
    else
        LOG_ERROR("Current operation mode:%u\n", opMode);

    if (deviceId == DEVID_SAP)
    {
        opMode = QAPI_WLAN_DEV_MODE_AP_E;
        if (wlan_set_power_mode(deviceId, QAPI_WLAN_POWER_MODE_REC_POWER_E, QAPI_WLAN_POWER_MODULE_SOFTAP_E))
        {
            LOG_ERROR("Power mode setting failed\n");
            return FAILURE;
        }
    }
    else if (deviceId == DEVID_STA)
    {
        opMode = QAPI_WLAN_DEV_MODE_STATION_E;
    }
    else
    {
        LOG_ERROR("Invaid Operating mode\n");
        return FAILURE;
    }

    if (QAPI_OK != qapi_WLAN_Set_Param(deviceId,
                __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                __QAPI_WLAN_PARAM_GROUP_WIRELESS_OPERATION_MODE,
                &opMode,
                sizeof(qapi_WLAN_Dev_Mode_e),
                FALSE))
    {
        LOG_ERROR("Not able to set operation mode\n");
        return FAILURE;
    }

    return SUCCESS;
}


/*FUNCTION*-------------------------------------------------------------
 *
 * Function Name   : connect_handler()
 * Returned Value  : 0 - successful completion or
 *                    -1 - failed.
 * Comments          : Handles Connect commands for infrastructure mode, Open
 *                   WPA2 security is supported
 *
 *END*-----------------------------------------------------------------*/

int32_t wlan_connect_to_network(uint32_t deviceId, const char *ssid, const char *passphrase)
{
    int ssidLength = 0;
    uint32_t temp_mode = 0, dataLen = 0;

    if (FAILURE == wlan_set_callback(deviceId, NULL))
    {
        LOG_ERROR("Set callback failed");
        return FAILURE;
    }

    LOG_VERBOSE("CONNECTING to SSID:%s, pwd:%s\n", ssid, passphrase?passphrase:"----");
    //TODO: Need to check for SC and MCC cases

    if (QAPI_OK != qapi_WLAN_Get_Param(deviceId,
                __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                __QAPI_WLAN_PARAM_GROUP_WIRELESS_OPERATION_MODE,
                &temp_mode,
                &dataLen))
    {
        LOG_ERROR("Failed to get mode\n");
        return FAILURE;
    }

    if (temp_mode == QAPI_WLAN_DEV_MODE_AP_E)
    {
        LOG_INFO("setting to ap mode \r\n");
    }
    if ((!passphrase) && (temp_mode == QAPI_WLAN_DEV_MODE_AP_E))
    {
        LOG_INFO("AP in OPEN mode!\r\n");
    }

    ssidLength = strlen(ssid);
    if (QAPI_OK != qapi_WLAN_Set_Param(deviceId,
                __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                __QAPI_WLAN_PARAM_GROUP_WIRELESS_SSID,
                (void *) ssid,
                ssidLength,
                FALSE))
    {
        LOG_ERROR("Error during setting of ssid %s \n", ssid);
        return FAILURE;
    }

    LOG_INFO("\r\nSetting SSID to %s \r\n\r\n", ssid);

    if(passphrase)
    {
        uint32_t passphraseLen = 0;
        qapi_WLAN_Auth_Mode_e wpa_ver;
        qapi_WLAN_Crypt_Type_e cipher;

        wpa_ver = QAPI_WLAN_AUTH_WPA2_PSK_E;
        cipher = QAPI_WLAN_CRYPT_AES_CRYPT_E;

        if(0 != qapi_WLAN_Set_Param(deviceId,
                    __QAPI_WLAN_PARAM_GROUP_WIRELESS_SECURITY,
                    __QAPI_WLAN_PARAM_GROUP_SECURITY_ENCRYPTION_TYPE,
                    (void *) &cipher, //cipher is set in set_wpa
                    sizeof(qapi_WLAN_Crypt_Type_e),
                    FALSE))
        {
            return FAILURE;
        }

        if( 0 != qapi_WLAN_Set_Param (deviceId,
                    __QAPI_WLAN_PARAM_GROUP_WIRELESS_SECURITY,
                    __QAPI_WLAN_PARAM_GROUP_SECURITY_AUTH_MODE,
                    (void *) &wpa_ver,
                    sizeof(qapi_WLAN_Auth_Mode_e),
                    FALSE))
        {
            return FAILURE;
        }

        passphraseLen  = strlen(passphrase);
        if((passphraseLen >= 8) && (passphraseLen <= 63))
        {
            if (0 != qapi_WLAN_Set_Param(deviceId,
                        __QAPI_WLAN_PARAM_GROUP_WIRELESS_SECURITY,
                        __QAPI_WLAN_PARAM_GROUP_SECURITY_PASSPHRASE,
                        (void *)passphrase,
                        passphraseLen,
                        FALSE))
            {
                LOG_ERROR("Unable to set passphrase\r\n");
                return FAILURE;
            }
        }
        else if(passphraseLen == 64)
        {
            if (0 != qapi_WLAN_Set_Param(deviceId,
                        __QAPI_WLAN_PARAM_GROUP_WIRELESS_SECURITY,
                        __QAPI_WLAN_PARAM_GROUP_SECURITY_PMK,
                        (void *)passphrase,
                        passphraseLen,
                        FALSE))
            {
                LOG_ERROR("Unable to set pmk\r\n");
                return FAILURE;
            }
        }
        else
        {
            LOG_ERROR("invalid password\r\n");
            return FAILURE;
        }
    }

    if (QAPI_OK != qapi_WLAN_Commit(deviceId))
    {
        if(temp_mode == MODE_AP_E)
        {
            qapi_WLAN_Auth_Mode_e authMode = QAPI_WLAN_AUTH_NONE_E;
            LOG_ERROR("failed to Set AP mode\r\n");

            // Clear Invalid configurations
            if ( QAPI_OK != qapi_WLAN_Set_Param(deviceId,
                        __QAPI_WLAN_PARAM_GROUP_WIRELESS_SECURITY,
                        __QAPI_WLAN_PARAM_GROUP_SECURITY_AUTH_MODE,
                        (void *) &authMode,
                        sizeof(qapi_WLAN_Auth_Mode_e),
                        FALSE))
            {
                LOG_ERROR("Unable to clear Sec mode\r\n");
                return FAILURE;
            }
        }
        return FAILURE;
    }

    return SUCCESS;
}

int32_t wlan_disconnect_from_network(uint32_t deviceId)
{
    if (QAPI_OK == qapi_WLAN_Disconnect(deviceId))
        return SUCCESS;
    
    LOG_WARN("Disconnect Failed\n");

    return FAILURE;
}


int32_t wlan_enable(uint32_t num_dev, qurt_signal_t *wlan_sig)
{
    LOG_INFO("Enable WLAN numVDEV:%d", num_dev);

    if (QAPI_OK == qapi_WLAN_Enable(QAPI_WLAN_ENABLE_E))
    {
        int i;
        LOG_INFO("Wlan enable_success\n");
        for (i=0; i < num_dev; i++)
        {
            if (QAPI_OK != qapi_WLAN_Add_Device(i))
            {
                LOG_ERROR("Add device failed");
                qapi_WLAN_Enable(QAPI_WLAN_DISABLE_E);
                return FAILURE;
            }
        }
        pwlan_sigevent = wlan_sig;
        wlan_enabled = 1;
        return SUCCESS;
    }
        LOG_INFO("Wlan enable_FAILURE\n");
    return FAILURE;
}

int32_t wlan_disable()
{
    int i;

    LOG_INFO("Disable WLAN");
    if (0 == wlan_enabled)
    {
        return SUCCESS;
    }

    /* Remove the WLAN device from network interface before disabling WLAN */
    for (i=0; i < WLAN_NUM_OF_DEVICES; i++)
    {
        qapi_WLAN_Remove_Device(i);
    }

    qapi_WLAN_Enable(QAPI_WLAN_DISABLE_E);
    wlan_enabled = 0;

    return SUCCESS;
}
