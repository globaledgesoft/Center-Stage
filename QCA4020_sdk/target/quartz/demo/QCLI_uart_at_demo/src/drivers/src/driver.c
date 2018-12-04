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

#include "driver.h"

/** Start of wlan */

#ifndef CONFIG_PLATFORM_CDB24
static qapi_Status_t qc_drv_wlan_enable()
{
    return qapi_WLAN_Enable(QAPI_WLAN_ENABLE_E);
}

static qapi_Status_t qc_drv_wlan_disable()
{
    return qapi_WLAN_Enable(QAPI_WLAN_DISABLE_E);
}

static qapi_Status_t qc_drv_wlan_set_power_param(uint32_t device_Id,
        void *pwrMode,
        int size,
        int flag)
{
    return qapi_WLAN_Set_Param (device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_POWER_MODE_PARAMS,
            pwrMode,
            size,
            flag);
}

static qapi_Status_t qc_drv_wlan_get_power_param(uint32_t device_Id,
        uint32_t *mode, uint32_t *dataLen)
{
    return qapi_WLAN_Get_Param(device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_POWER_MODE_PARAMS,
            mode,
            dataLen);
}

static qapi_Status_t qc_drv_wlan_set_encr_type (uint32_t device_Id,
        void *encrType,
        int size,
        int flag)
{
    return qapi_WLAN_Set_Param(device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_SECURITY,
            __QAPI_WLAN_PARAM_GROUP_SECURITY_ENCRYPTION_TYPE,
            encrType,
            size,
            flag);
}

static qapi_Status_t qc_drv_wlan_set_auth_mode(uint32_t device_Id,
        qapi_WLAN_Auth_Mode_e *authMode,
        int size,
        int flag)
{
    return qapi_WLAN_Set_Param(device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_SECURITY,
            __QAPI_WLAN_PARAM_GROUP_SECURITY_AUTH_MODE,
            authMode,
            size,
            flag);
}

static qapi_Status_t qc_drv_wlan_get_op_mode(uint32_t device_Id,
        uint32_t *wifimode,
        uint32_t *dataLen)
{
    return qapi_WLAN_Get_Param (device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_OPERATION_MODE,
            wifimode,
            dataLen);
}

static qapi_Status_t qc_drv_wlan_get_mac_addr (uint32_t device_Id,
        uint8_t *own_Interface_Addr,
        uint32_t *dataLen)
{
    return qapi_WLAN_Get_Param (device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_MAC_ADDRESS,
            own_Interface_Addr,
            dataLen);
}

static qapi_Status_t qc_drv_wlan_set_mac_addr (uint32_t device_Id,
        uint8_t *own_Interface_Addr,
        uint32_t dataLen)
{
    return qapi_WLAN_Set_Param(device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_MAC_ADDRESS,
            own_Interface_Addr,
            dataLen,
            0);
}

static qapi_Status_t qc_drv_wlan_set_ssid(uint32_t device_Id,
        uint8_t *ssid, uint32_t ssid_len, uint8_t flag)
{
    return qapi_WLAN_Set_Param(device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_SSID,
            (int8_t *) ssid,
            ssid_len,
            flag);
}

static qapi_Status_t qc_drv_wlan_set_op_mode (uint32_t device_Id,
        qapi_WLAN_Dev_Mode_e *opMode,
        int size,
        int flag)
{
    return qapi_WLAN_Set_Param(device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_OPERATION_MODE,
            opMode,
            size,
            flag);
}

static qapi_Status_t qc_drv_wlan_set_passphrase(uint32_t device_Id,
        void *passphrase, uint32_t passphrase_len, uint8_t flag)
{
    return qapi_WLAN_Set_Param(device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_SECURITY,
            __QAPI_WLAN_PARAM_GROUP_SECURITY_PASSPHRASE,
            passphrase,
            passphrase_len,
            flag);
}

static qapi_Status_t qc_drv_wlan_set_security_pmk(uint32_t device_Id,
        void *passphrase, uint32_t passphrase_len, uint8_t flag)
{
    return qapi_WLAN_Set_Param(device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_SECURITY,
            __QAPI_WLAN_PARAM_GROUP_SECURITY_PMK,
            passphrase,
            passphrase_len,
            flag);
}

static qapi_Status_t qc_drv_wlan_set_wps_credentials(uint32_t device_Id,
        qapi_WLAN_WPS_Credentials_t *wpsCred, uint32_t size, uint8_t flag)
{
    return qapi_WLAN_Set_Param (device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_SECURITY,
            __QAPI_WLAN_PARAM_GROUP_SECURITY_WPS_CREDENTIALS,
            wpsCred,
            size,
            flag);
}

static qapi_Status_t qc_drv_wlan_set_channel(uint32_t device_Id,
        uint32_t *channel_val, uint32_t size, uint8_t flag)
{
    return qapi_WLAN_Set_Param(device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_CHANNEL,
            channel_val,
            size,
            flag);
}

static qapi_Status_t qc_drv_wlan_set_listen_interval(uint32_t device_Id,
        int32_t *listen_time, uint32_t size, uint8_t flag)
{
    return qapi_WLAN_Set_Param(device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_STA_LISTEN_INTERVAL_IN_TU,
            (void *)listen_time,
            size,
            flag);
}

static qapi_Status_t qc_drv_wlan_set_sta_keep_alive(uint32_t device_Id,
        int32_t *keep_alive, uint32_t size, uint8_t flag)
{
    return qapi_WLAN_Set_Param(device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_STA_KEEP_ALIVE_IN_SEC,
            keep_alive,
            size,
            flag);
}

static qapi_Status_t qc_drv_wlan_set_rssi_threshold(uint32_t device_Id,
        qapi_WLAN_Rssi_Threshold_Params_t *qrthresh, uint32_t size, uint8_t flag)
{
    return qapi_WLAN_Set_Param(device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_RSSI_THRESHOLD,
            &qrthresh,
            size,
            flag);
}

static qapi_Status_t qc_drv_wlan_set_probe_req_fwd_to_host(uint32_t device_Id,
        int32_t *enable, uint32_t size, uint8_t flag)
{
    return qapi_WLAN_Set_Param(device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_ENABLE_PROBE_REQ_FWD_TO_HOST,
            enable,
            size,
            flag);
}

static qapi_Status_t qc_drv_wlan_set_wep_key_index(uint32_t device_Id,
        int32_t *key_idx, uint32_t size, uint8_t flag)
{
    return qapi_WLAN_Set_Param (device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_SECURITY,
            __QAPI_WLAN_PARAM_GROUP_SECURITY_WEP_KEY_INDEX,
            (void *) key_idx,
            size,
            flag);
}

static qapi_Status_t qc_drv_wlan_set_wep_key_pair(uint32_t device_Id,
        qapi_WLAN_Security_Wep_Key_Pair_Params_t *keyPair, uint32_t size, uint8_t flag)
{
    return qapi_WLAN_Set_Param (device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_SECURITY,
            __QAPI_WLAN_PARAM_GROUP_SECURITY_WEP_KEY_PAIR,
            (void *) keyPair,
            size,
            flag);
}

static qapi_Status_t qc_drv_wlan_set_rate(uint32_t device_Id,
        qapi_WLAN_Bit_Rate_t *rateIndex, uint32_t size, uint8_t flag)
{
    return qapi_WLAN_Set_Param(device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_RATE,
            rateIndex,
            size,
            flag);
}

static qapi_Status_t qc_drv_wlan_set_ap_wps_flag(uint32_t device_Id,
        uint8_t *wps_flag, uint32_t size, uint8_t flag)
{
    return qapi_WLAN_Set_Param(device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_AP_WPS_FLAG,
            wps_flag,
            size,
            flag);
}

static qapi_Status_t qc_drv_wlan_set_tx_power_in_dbm(uint32_t device_Id,
        int32_t *power_in_dBm, uint32_t size, uint8_t flag)
{
    return qapi_WLAN_Set_Param(device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_TX_POWER_IN_DBM,
            power_in_dBm,
            size,
            flag);
}

static qapi_Status_t qc_drv_wlan_set_phy_mode(uint32_t device_Id,
        qapi_WLAN_Phy_Mode_e *phyMode, uint32_t size, uint8_t flag)
{
    return qapi_WLAN_Set_Param(device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_PHY_MODE,
            phyMode,
            size,
            flag);
}

static qapi_Status_t qc_drv_wlan_set_ht_config(uint32_t device_Id,
        qapi_WLAN_11n_HT_Config_e *htconfig, uint32_t size, uint8_t flag)
{
    return qapi_WLAN_Set_Param(device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_11N_HT,
            htconfig,
            size,
            flag);
}

static qapi_Status_t qc_drv_wlan_set_ap_enable_hidden_mode(uint32_t device_Id,
        uint8_t *hidden_flag, uint32_t size, uint8_t flag)
{
    return qapi_WLAN_Set_Param(device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_AP_ENABLE_HIDDEN_MODE,
            hidden_flag,
            size,
            flag);
}

static qapi_Status_t qc_drv_wlan_set_beacon_interval_in_tu(uint32_t device_Id,
        uint32_t *beacon_int_in_tu, uint32_t size, uint8_t flag)
{
    return qapi_WLAN_Set_Param (device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_AP_BEACON_INTERVAL_IN_TU,
            beacon_int_in_tu,
            size,
            flag);
}

static qapi_Status_t qc_drv_wlan_set_ap_dtim_interval(uint32_t device_Id,
        uint32_t *dtim_period, uint32_t size, uint8_t flag)
{
    return qapi_WLAN_Set_Param (device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_AP_DTIM_INTERVAL,
            dtim_period,
            size,
            flag);
}

static qapi_Status_t qc_drv_wlan_set_country_code(uint32_t device_Id,
        char *country_code, uint32_t size, uint8_t flag)
{
    return qapi_WLAN_Set_Param(device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_COUNTRY_CODE,
            country_code,
            size,
            flag);
}

static qapi_Status_t qc_drv_wlan_set_ap_country_code(uint32_t device_Id,
        char *country_code, uint32_t size, uint8_t flag)
{
    return qapi_WLAN_Set_Param(device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_AP_COUNTRY_CODE,
            country_code,
            size,
            flag);
}

static qapi_Status_t qc_drv_wlan_add_device(uint8_t device_ID)
{
    return qapi_WLAN_Add_Device(device_ID);
}

static qapi_Status_t qc_drv_wlan_remove_device(uint8_t device_ID)
{
    return qapi_WLAN_Remove_Device(device_ID);
}


static qapi_Status_t qc_drv_wlan_start_scan(uint8_t device_Id,
        const qapi_WLAN_Start_Scan_Params_t  *scan_Params,
        qapi_WLAN_Store_Scan_Results_e store_Scan_Results)
{
    return qapi_WLAN_Start_Scan(device_Id,
            (qapi_WLAN_Start_Scan_Params_t *) scan_Params,
            store_Scan_Results);
}

static qapi_Status_t qc_drv_wlan_get_scan_results(uint8_t device_Id,
        qapi_WLAN_BSS_Scan_Info_t  *scan_Info,
        int16_t  *num_Results)
{
    return qapi_WLAN_Get_Scan_Results(device_Id,
            scan_Info,
            num_Results);
}

static qapi_Status_t qc_drv_wlan_get_phy_mode(uint8_t device_Id,
        qapi_WLAN_Phy_Mode_e *phyMode,
        uint32_t  *dataLen)
{
    return qapi_WLAN_Get_Param (device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_PHY_MODE,
            phyMode,
            dataLen);
}

static qapi_Status_t qc_drv_wlan_get_ht_config(uint8_t device_Id,
        qapi_WLAN_11n_HT_Config_e *htConfig,
        uint32_t  *dataLen)
{
    return qapi_WLAN_Get_Param(device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_11N_HT,
            htConfig,
            dataLen);
}

static qapi_Status_t qc_drv_wlan_get_ssid(uint8_t device_Id,
        void * ssid,
        uint32_t  *dataLen)
{
    return qapi_WLAN_Get_Param (device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_SSID,
            ssid,
            dataLen);
}

static qapi_Status_t qc_drv_wlan_get_channel(uint8_t device_Id,
        uint16_t *channel_val,
        uint32_t  *dataLen)
{
    return qapi_WLAN_Get_Param (device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_CHANNEL,
            channel_val,
            dataLen);
}

static qapi_Status_t qc_drv_wlan_get_wep_key_pair(uint8_t device_Id,
        qapi_WLAN_Security_Wep_Key_Pair_Params_t *key_pair,
        uint32_t  *dataLen)
{
    return qapi_WLAN_Get_Param (device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_SECURITY,
            __QAPI_WLAN_PARAM_GROUP_SECURITY_WEP_KEY_PAIR,
            key_pair, dataLen);
}

static qapi_Status_t qc_drv_wlan_get_wep_key_index(uint8_t device_Id,
        uint32_t *key_index,
        uint32_t  *dataLen)
{
    return  qapi_WLAN_Get_Param (device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_SECURITY,
            __QAPI_WLAN_PARAM_GROUP_SECURITY_WEP_KEY_INDEX,
            key_index, dataLen);
}

static qapi_Status_t qc_drv_wlan_get_rssi(uint8_t device_Id,
        uint8_t *rssi,
        uint32_t  *dataLen)
{
    return qapi_WLAN_Get_Param (device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_RSSI,
            rssi,
            dataLen);
}

static qapi_Status_t qc_drv_wlan_get_reg_domain_channel_list(uint8_t device_Id,
        qapi_WLAN_Get_Channel_List_t *wlanChannelList,
        uint32_t  *dataLen)
{
    return qapi_WLAN_Get_Param (device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_GET_CURR_REGDOMAIN_CHANNEL_LIST,
            wlanChannelList,
            dataLen);
}

static qapi_Status_t qc_drv_wlan_get_wireless_stats(uint8_t device_Id,
        qapi_WLAN_Get_Statistics_t *getStats,
        uint32_t  *dataLen)
{
    return qapi_WLAN_Get_Param (device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_STATS,
            getStats,
            dataLen);
}

static qapi_Status_t qc_drv_wlan_get_rate(uint8_t device_Id,
        int32_t *rate_index,
        uint32_t  *dataLen)
{
    return qapi_WLAN_Get_Param(device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_RATE,
            rate_index,
            dataLen);
}

static qapi_Status_t qc_drv_wlan_get_firmware_version(uint8_t device_Id,
        qapi_WLAN_Firmware_Version_String_t *versionstr,
        uint32_t  *dataLen)
{
    return qapi_WLAN_Get_Param(device_Id,
            __QAPI_WLAN_PARAM_GROUP_SYSTEM,
            __QAPI_WLAN_PARAM_GROUP_SYSTEM_FIRMWARE_VERSION,
            versionstr,
            dataLen);
}

static qapi_Status_t qc_drv_wlan_get_reg_domain(uint8_t device_Id,
        uint32_t *regDomain,
        uint32_t  *dataLen)
{
    return qapi_WLAN_Get_Param(device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_REG_DOMAIN,
            regDomain,
            dataLen);
}

static qapi_Status_t qc_drv_wlan_get_last_error(uint8_t device_Id,
        int32_t *err,
        uint32_t  *dataLen)
{
    return qapi_WLAN_Get_Param(device_Id,
            __QAPI_WLAN_PARAM_GROUP_SYSTEM,
            __QAPI_WLAN_PARAM_GROUP_SYSTEM_LAST_ERROR,
            err,
            dataLen);
}

static qapi_Status_t qc_drv_wlan_get_tx_status(uint8_t device_Id,
        uint32_t *tx_status,
        uint32_t  *dataLen)
{
    return qapi_WLAN_Get_Param (device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_TX_STATUS,
            tx_status,
            dataLen);
}

static qapi_Status_t qc_drv_wlan_set_callback(uint8_t device_Id,
        qapi_WLAN_Callback_t callback,
        const void *application_Context)
{
    return qapi_WLAN_Set_Callback(device_Id, callback, application_Context);
}

static qapi_Status_t qc_drv_wlan_wps_connect(uint8_t device_ID)
{
    return qapi_WLAN_WPS_Connect(device_ID);
}

static qapi_Status_t qc_drv_wlan_wps_await_completion(uint32_t device_Id,
        qapi_WLAN_Netparams_t *net_Params)
{
    return qapi_WLAN_WPS_Await_Completion(device_Id, net_Params);
}

static qapi_Status_t qc_drv_wlan_disconnect(uint8_t device_ID)
{
    return qapi_WLAN_Disconnect(device_ID);
}

static qapi_Status_t qc_drv_wlan_commit(uint8_t device_ID)
{
    return qapi_WLAN_Commit(device_ID);
}

static qapi_Status_t qc_drv_wlan_raw_send(uint8_t device_Id,
        const qapi_WLAN_Raw_Send_Params_t  *raw_Params)
{
    return qapi_WLAN_Raw_Send(device_Id, raw_Params);
}

static qapi_Status_t qc_drv_wlan_suspend_start(uint32_t suspend_Time_In_Ms)
{
    return qapi_WLAN_Suspend_Start(suspend_Time_In_Ms);
}

static qapi_Status_t qc_drv_wlan_wps_start(uint8_t device_Id,
        qapi_WLAN_WPS_Connect_Action_e connect_Action,
        qapi_WLAN_WPS_Mode_e mode,
        const char  *pin)
{
    return qapi_WLAN_WPS_Start(device_Id, connect_Action, mode, pin);
}

static qapi_Status_t qc_drv_wlan_set_get_auth_mode(uint8_t device_Id,
        void *authMode, size_t size, int flag)
{
    return qapi_WLAN_Set_Param (device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_SECURITY,
            __QAPI_WLAN_PARAM_GROUP_SECURITY_AUTH_MODE,
            authMode,
            size,
            flag);
}

static qapi_Status_t qc_drv_wlan_promiscuous_mode(uint8_t device_Id,
        void *mode, size_t size, int flag)
{
    return qapi_WLAN_Set_Param (device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_ENABLE_PROMISCUOUS_MODE,
            mode,
            size,
            flag);
}

static qapi_Status_t qc_drv_wlan_force_assert(uint8_t device_Id,
        void *data, size_t size, int flag)
{
    return qapi_WLAN_Set_Param (device_Id,
            __QAPI_WLAN_PARAM_GROUP_SYSTEM,
            __QAPI_WLAN_PARAM_GROUP_SYSTEM_DRIVER_FORCE_ASSERT,
            data,
            size,
            flag);
}

static qapi_Status_t qc_drv_wlan_scan_param(uint8_t device_Id,
        void *scanParam, size_t size, int flag)
{
    return qapi_WLAN_Set_Param (device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_SCAN_PARAMS,
            scanParam,
            size,
            flag);
}

static qapi_Status_t qc_drv_wlan_set_inactivity_time(uint8_t device_Id,
        void *time, size_t size, int flag)
{
    return qapi_WLAN_Set_Param (device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_AP_INACTIVITY_TIME_IN_MINS,
            time,
            size,
            flag);
}

static qapi_Status_t qc_drv_wlan_set_ap_ps_buf(uint8_t device_Id,
        void *ps_val, size_t size, int flag)
{
    return qapi_WLAN_Set_Param (device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_AP_PS_BUF,
            ps_val,
            size,
            flag);
}

static qapi_Status_t qc_drv_wlan_set_ap_enable_uapsd(uint8_t device_Id,
        void *uapsd, size_t size, int flag)
{
    return qapi_WLAN_Set_Param (device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_AP_ENABLE_UAPSD,
            uapsd,
            size,
            flag);
}

static qapi_Status_t qc_drv_wlan_set_sta_uapsd(uint8_t device_Id,
        void *uapsd, size_t size, int flag)
{
    return qapi_WLAN_Set_Param (device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_STA_UAPSD,
            uapsd,
            size,
            flag);
}

static qapi_Status_t qc_drv_wlan_set_sta_max_sp_len(uint8_t device_Id,
        void *maxsp, size_t size, int flag)
{
    return qapi_WLAN_Set_Param (device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_STA_MAX_SP_LEN,
            maxsp,
            size,
            flag);
}

static qapi_Status_t qc_drv_wlan_set_wmm_config(uint8_t device_Id,
        void *wmm, size_t size, int flag)
{
    return qapi_WLAN_Set_Param (device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_WNM_CONFIG,
            wmm,
            size,
            flag);
}

static qapi_Status_t qc_drv_wlan_set_ap_bss_mip(uint8_t device_Id,
        void *period, size_t size, int flag)
{
    return qapi_WLAN_Set_Param (device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_WNM_AP_BSS_MAX_IDLE_PERIOD,
            period,
            size,
            flag);
}

static qapi_Status_t qc_drv_wlan_set_sta_sp(uint8_t device_Id,
        void *period, size_t size, int flag)
{
    return qapi_WLAN_Set_Param (device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_WNM_STA_SLEEP_PERIOD,
            period,
            size,
            flag);
}

static qapi_Status_t qc_drv_wlan_set_ap_sr(uint8_t device_Id,
        void *response, size_t size, int flag)
{
    return qapi_WLAN_Set_Param (device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_WNM_AP_SLEEP_RESPONSE,
            response,
            size,
            flag);
}

static qapi_Status_t qc_drv_wlan_set_app_ie(uint8_t device_Id,
        void *ie_params, size_t size, int flag)
{
    return qapi_WLAN_Set_Param (device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_APP_IE,
            ie_params,
            size,
            flag);
}

static qapi_Status_t qc_drv_wlan_set_sta_bmiss_config(uint8_t device_Id,
        void *bmiss, size_t size, int flag)
{
    return qapi_WLAN_Set_Param (device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_STA_BMISS_CONFIG,
            bmiss,
            size,
            flag);
}

static qapi_Status_t qc_drv_wlan_set_suspend_resume(uint8_t device_Id,
        void *suspend, size_t size, int flag)
{
    return qapi_WLAN_Set_Param (device_Id,
            __QAPI_WLAN_PARAM_GROUP_SYSTEM,
            __QAPI_WLAN_PARAM_GROUP_SYSTEM_ENABLE_SUSPEND_RESUME,
            suspend,
            size,
            flag);
}

static qapi_Status_t qc_drv_wlan_set_ka_offload_scfg(uint8_t device_Id,
        qapi_WLAN_TCP_Offload_Config_Params_t *cfg, size_t size, int flag)
{
    return qapi_WLAN_Set_Param (device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_TCP_KEEPALIVE_OFFLOAD_SESSION_CFG,
            cfg,
            size,
            flag);
}

static qapi_Status_t qc_drv_wlan_set_ka_offload_enable(uint8_t device_Id,
        qapi_WLAN_TCP_Offload_Enable_t *enable, size_t size, int flag)
{
    return qapi_WLAN_Set_Param (device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_TCP_KEEPALIVE_OFFLOAD_ENABLE,
            enable,
            size,
            flag);
}

static qapi_Status_t qc_drv_wlan_set_nw_offload_enable(uint8_t device_Id,
        qapi_WLAN_Preferred_Network_Offload_Config_t *param, size_t size, int flag)
{
    return qapi_WLAN_Set_Param (device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_PREFERRED_NETWORK_OFFLOAD_ENABLE,
            param,
            size,
            flag);
}

static qapi_Status_t qc_drv_wlan_set_nw_profile(uint8_t device_Id,
        qapi_WLAN_Preferred_Network_Profile_t *param, size_t size, int flag)
{
    return qapi_WLAN_Set_Param (device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_PREFERRED_NETWORK_PROFILE,
            param,
            size,
            flag);
}

static qapi_Status_t qc_drv_wlan_set_aggr_tid(uint8_t device_Id,
        qapi_WLAN_Aggregation_Params_t *param, size_t size, int flag)
{
    return qapi_WLAN_Set_Param (device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_ALLOW_TX_RX_AGGR_SET_TID,
            param,
            size,
            flag);
}

static qapi_Status_t qc_drv_wlan_set_aggrx_config(uint8_t device_Id,
        qapi_WLAN_Rx_Aggrx_Params_t *param, size_t size, int flag)
{
    return qapi_WLAN_Set_Param (device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_AGGRX_CONFIG,
            param,
            size,
            flag);
}

static qapi_Status_t qc_drv_wlan_set_pktlog_enable(uint8_t device_Id,
        qapi_WLAN_Pktlog_Enable_t *pktlog, size_t size, int flag)
{
    return qapi_WLAN_Set_Param (device_Id,
            __QAPI_WLAN_PARAM_GROUP_SYSTEM,
            __QAPI_WLAN_PARAM_GROUP_SYSTEM_PKTLOG_ENABLE,
            pktlog,
            size,
            flag);
}

static qapi_Status_t qc_drv_wlan_set_pktlog_start(uint8_t device_Id,
        qapi_WLAN_Pktlog_Start_Params_t *pktlog, size_t size, int flag)
{
    return qapi_WLAN_Set_Param (device_Id,
            __QAPI_WLAN_PARAM_GROUP_SYSTEM,
            __QAPI_WLAN_PARAM_GROUP_SYSTEM_PKTLOG_START,
            pktlog,
            size,
            flag);
}

static qapi_Status_t qc_drv_wlan_set_dblog_enable(uint8_t device_Id,
        qapi_WLAN_Dbglog_Enable_t *dbglog, size_t size, int flag)
{
    return qapi_WLAN_Set_Param (device_Id,
            __QAPI_WLAN_PARAM_GROUP_SYSTEM,
            __QAPI_WLAN_PARAM_GROUP_SYSTEM_DBGLOG_ENABLE,
            dbglog,
            size,
            flag);
}

static qapi_Status_t qc_drv_wlan_set_dblog_config(uint8_t device_Id,
        qapi_WLAN_Dbglog_Config_t *config, size_t size, int flag)
{
    return qapi_WLAN_Set_Param (device_Id,
            __QAPI_WLAN_PARAM_GROUP_SYSTEM,
            __QAPI_WLAN_PARAM_GROUP_SYSTEM_DBGLOG_CONFIG,
            config,
            size,
            flag);
}

static qapi_Status_t qc_drv_wlan_set_dblog_module_conf(uint8_t device_Id,
        qapi_WLAN_Dbglog_Module_Config_t *config, size_t size, int flag)
{
    return qapi_WLAN_Set_Param (device_Id,
            __QAPI_WLAN_PARAM_GROUP_SYSTEM,
            __QAPI_WLAN_PARAM_GROUP_SYSTEM_DBGLOG_MODULE_CONFIG,
            config,
            size,
            flag);
}

static qapi_Status_t qc_drv_wlan_set_drv_reg_query(uint8_t device_Id,
        qapi_WLAN_Driver_RegQuery_Params_t *query, size_t size, int flag)
{
    return qapi_WLAN_Set_Param (device_Id,
            __QAPI_WLAN_PARAM_GROUP_SYSTEM,
            __QAPI_WLAN_PARAM_GROUP_SYSTEM_DRIVER_REG_QUERY,
            query,
            size,
            flag);
}

static qapi_Status_t qc_drv_wlan_set_ap_channel_switch(uint8_t device_Id,
        qapi_WLAN_Channel_Switch_t *chnl, size_t size, int flag)
{
    return qapi_WLAN_Set_Param (device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_AP_CHANNEL_SWITCH,
            chnl,
            size,
            flag);
}

static qapi_Status_t qc_drv_wlan_set_event_filter(uint8_t device_Id,
        qapi_WLAN_Event_Filter_t *filter, size_t size, int flag)
{
    return qapi_WLAN_Set_Param (device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_EVENT_FILTER,
            filter,
            size,
            flag);
}

static qapi_Status_t qc_drv_wlan_set_pw_mode_policy(uint8_t device_Id,
        qapi_WLAN_Power_Policy_Params_t *pm, size_t size, int flag)
{
    return qapi_WLAN_Set_Param (device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_POWER_MODE_POLICY,
            pm,
            size,
            flag);
}

static qapi_Status_t qc_drv_wlan_set_enable_roaming(uint8_t device_Id,
        uint32_t *roaming, size_t size, int flag)
{
    return qapi_WLAN_Set_Param (device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_ENABLE_ROAMING,
            roaming,
            size,
            flag);
}

static qapi_Status_t qc_drv_wlan_set_enable_green_tx(uint8_t device_Id,
        uint32_t *entx, size_t size, int flag)
{
    return qapi_WLAN_Set_Param (device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_ENABLE_GREEN_TX,
            entx,
            size,
            flag);
}

static qapi_Status_t qc_drv_wlan_set_low_pw_listen(uint8_t device_Id,
        uint32_t *enlpw, size_t size, int flag)
{
    return qapi_WLAN_Set_Param (device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_ENABLE_LOW_POWER_LISTEN,
            enlpw,
            size,
            flag);
}

static qapi_Status_t qc_drv_wlan_set_enable_wake_wireless(uint8_t device_Id,
        uint32_t *wake, size_t size, int flag)
{
    return qapi_WLAN_Set_Param (device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_ENABLE_WAKE_ON_WIRELESS,
            wake,
            size,
            flag);
}

static qapi_Status_t qc_drv_wlan_set_enable_pkt_filter(uint8_t device_Id,
        uint32_t *filter, size_t size, int flag)
{
    return qapi_WLAN_Set_Param (device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_ENABLE_PKT_FILTER,
            filter,
            size,
            flag);
}

static qapi_Status_t qc_drv_wlan_set_add_pattern(uint8_t device_Id,
        qapi_WLAN_Add_Pattern_t *pattern, size_t size, int flag)
{
    return qapi_WLAN_Set_Param (device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_ADD_PATTERN,
            pattern,
            size,
            flag);
}

static qapi_Status_t qc_drv_wlan_set_delete_pattern(uint8_t device_Id,
        qapi_WLAN_Delete_Pattern_t *pattern, size_t size, int flag)
{
    return qapi_WLAN_Set_Param (device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_DELETE_PATTERN,
            pattern,
            size,
            flag);
}

static qapi_Status_t qc_drv_wlan_set_change_def_filter_action(uint8_t device_Id,
        qapi_WLAN_Change_Default_Filter_Action_t *filter, size_t size, int flag)
{
    return qapi_WLAN_Set_Param (device_Id,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_CHANGE_DEFAULT_FILTER_ACTION,
            filter,
            size,
            flag);
}


/** Start of p2p */
static qapi_Status_t qc_drv_p2p_enable(uint32_t device_Id, int flag)
{
    return qapi_WLAN_P2P_Enable(device_Id, flag);
}

static qapi_Status_t qc_drv_p2p_disable(uint32_t device_Id, int flag)
{
    return qapi_WLAN_P2P_Enable(device_Id, flag);
}

static qapi_Status_t qc_drv_p2p_set_config(uint32_t device_Id,
        qapi_WLAN_P2P_Config_Params_t *p2pConfig,
        int size,
        int flag)
{
    return qapi_WLAN_Set_Param(device_Id,
            __QAPI_WLAN_PARAM_GROUP_P2P,
            __QAPI_WLAN_PARAM_GROUP_P2P_CONFIG_PARAMS,
            p2pConfig,
            sizeof(p2pConfig),
            flag);
}

static qapi_Status_t qc_drv_p2p_connect(uint32_t device_Id,
        uint8_t wps_Method,
        uint8_t *peer_Addr,
        uint8_t p2p_pers_go)
{
    return qapi_WLAN_P2P_Connect(device_Id,
            wps_Method,
            peer_Addr,
            p2p_pers_go);
}

static qapi_Status_t qc_drv_p2p_provision(uint32_t device_Id,
        uint16_t wps_Method,
        uint8_t *peer)
{
    return qapi_WLAN_P2P_Prov(device_Id,
            wps_Method,
            peer);
}

static qapi_Status_t qc_drv_p2p_listen(uint32_t device_Id, uint32_t timeout_val)
{
    return qapi_WLAN_P2P_Listen(device_Id, timeout_val);
}

static qapi_Status_t qc_drv_p2p_cancel(uint32_t deviceId)
{
    return qapi_WLAN_P2P_Cancel(deviceId);
}

static qapi_Status_t qc_drv_p2p_join(uint32_t device_Id,
        uint8_t wps_Method,
        uint8_t *p2p_join_mac_addr,
        char *p2p_wps_pin,
        uint16_t go_Oper_Freq)
{
    return qapi_WLAN_P2P_Join(device_Id,
            wps_Method,
            p2p_join_mac_addr,
            p2p_wps_pin,
            go_Oper_Freq);
}

static qapi_Status_t qc_drv_p2p_auth(uint32_t device_Id,
        uint8_t dev_Auth,
        qapi_WLAN_P2P_WPS_Method_e wps_Method,
        uint8_t *peer_Addr,
        uint8_t p2p_persistent_go)
{
    return qapi_WLAN_P2P_Auth(device_Id,
            dev_Auth,
            wps_Method,
            peer_Addr,
            p2p_persistent_go);
}

static qapi_Status_t qc_drv_p2p_invite(uint32_t device_Id,
        const char *string,
        uint8_t wps_Method,
        uint8_t *peer_Addr,
        uint8_t is_Persistent,
        uint8_t p2p_invite_role)
{
    return qapi_WLAN_P2P_Invite(device_Id,
            string,
            wps_Method,
            peer_Addr,
            is_Persistent,
            p2p_invite_role);
}

static qapi_Status_t qc_drv_p2p_find(uint32_t device_Id, uint8_t type, uint32_t timeout)
{
    return qapi_WLAN_P2P_Find(device_Id, type, timeout);
}

static qapi_Status_t qc_drv_p2p_get_node_list(uint32_t device_Id,
        qapi_WLAN_P2P_Node_List_Params_t *p2pNodeList,
        uint32_t *dataLen)
{
    return qapi_WLAN_Get_Param(device_Id,
            __QAPI_WLAN_PARAM_GROUP_P2P,
            __QAPI_WLAN_PARAM_GROUP_P2P_NODE_LIST,
            p2pNodeList,
            dataLen);
}

static qapi_Status_t qc_drv_p2p_get_network_list(uint32_t device_Id,
        qapi_WLAN_P2P_Network_List_Params_t *p2pNetworkList,
        uint32_t *dataLen)
{
    return qapi_WLAN_Get_Param(device_Id,
            __QAPI_WLAN_PARAM_GROUP_P2P,
            __QAPI_WLAN_PARAM_GROUP_P2P_NETWORK_LIST,
            p2pNetworkList,
            dataLen);
}

static qapi_Status_t qc_drv_p2p_set_oops_params(uint32_t device_Id,
        qapi_WLAN_P2P_Opps_Params_t *opps,
        int size,
        int flag)
{
    return qapi_WLAN_Set_Param (device_Id,
            __QAPI_WLAN_PARAM_GROUP_P2P,
            __QAPI_WLAN_PARAM_GROUP_P2P_OPPS_PARAMS,
            opps,
            size,
            flag);
}

static qapi_Status_t qc_drv_p2p_set_noa_params(uint32_t device_Id,
        qapi_WLAN_P2P_Noa_Params_t *noaParams,
        int size,
        int flag)
{
    return qapi_WLAN_Set_Param (device_Id,
            __QAPI_WLAN_PARAM_GROUP_P2P,
            __QAPI_WLAN_PARAM_GROUP_P2P_NOA_PARAMS,
            noaParams,
            size,
            flag);
}

static qapi_Status_t qc_drv_p2p_set_operating_class(uint32_t device_Id,
        qapi_WLAN_P2P_Config_Params_t *p2pConfig,
        int size,
        int flag)
{
    return qapi_WLAN_Set_Param(device_Id,
            __QAPI_WLAN_PARAM_GROUP_P2P,
            __QAPI_WLAN_PARAM_GROUP_P2P_CONFIG_PARAMS,
            p2pConfig,
            size,
            flag);
}

static qapi_Status_t qc_drv_p2p_stop_find (uint32_t deviceId)
{
    return qapi_WLAN_P2P_Stop_Find(deviceId);
}

static qapi_Status_t qc_drv_p2p_set_ssidparam(uint32_t device_Id,
        qapi_WLAN_P2P_Go_Params_t *ssidParams,
        int size,
        int flag)
{
    return qapi_WLAN_Set_Param(device_Id,
            __QAPI_WLAN_PARAM_GROUP_P2P,
            __QAPI_WLAN_PARAM_GROUP_P2P_GO_PARAMS,
            ssidParams,
            size,
            flag);
}

static qapi_Status_t qc_drv_p2p_set(uint32_t device_Id,
        uint8_t config_Id,
        int *val,
        uint32_t len,
        int flag)
{
    return qapi_WLAN_Set_Param (device_Id,
            __QAPI_WLAN_PARAM_GROUP_P2P,
            config_Id,
            val,
            len,
            flag);
}


static qapi_Status_t qc_drv_p2p_start_go(uint32_t device_Id,
        qapi_WLAN_P2P_Go_Params_t *goParams,
        int32_t go_chan,
        uint8_t persistent_Group)
{
    return qapi_WLAN_P2P_Start_Go(device_Id,
            goParams,
            go_chan,
            persistent_Group);
}


static qapi_Status_t qc_drv_p2p_set_param(uint32_t device_Id,
        uint8_t *p2pmode,
        int size,
        int flag)
{
    return qapi_WLAN_Set_Param(device_Id,
            __QAPI_WLAN_PARAM_GROUP_P2P,
            __QAPI_WLAN_PARAM_GROUP_P2P_OP_MODE,
            p2pmode,
            size,
            flag);
}

static qapi_Status_t qc_drv_p2p_invite_auth(uint8_t device_Id,
        const qapi_WLAN_P2P_Invite_Info_t  *invite_Info)
{
    return qapi_WLAN_P2P_Invite_Auth(device_Id, invite_Info);
}

/* Start of Mqtt */
static qapi_Status_t qc_drv_net_mqtt_init(char *ca_file)
{
    return qapi_Net_MQTTc_Init(ca_file);
}

static qapi_Status_t qc_drv_net_mqtt_shutdown()
{
    return qapi_Net_MQTTc_Shutdown();
}

static qapi_Status_t qc_drv_net_mqtt_new(const char *client_id,
        qbool_t clean_session,
        qapi_Status_t *status)
{
    return qapi_Net_MQTTc_New(client_id,
            clean_session,
            status);
}

static qapi_Status_t qc_drv_net_mqtt_destroy(int32_t handle)
{
    return qapi_Net_MQTTc_Destroy(handle);
}

static qapi_Status_t qc_drv_net_mqtt_set_username_password(int32_t handle,
        const char *user, size_t user_len,
        const char *pw,	size_t pw_len,
        qapi_Status_t *status)
{
    return qapi_Net_MQTTc_Set_Username_Password(handle,
            user, user_len,
            pw, pw_len,
            status);
}

static qapi_Status_t qc_drv_net_mqtt_set_will(int32_t handle,
        const char *topic, size_t topic_len,
        const char *msg, size_t msg_len,
        uint32_t qos,
        qbool_t retained,
        qapi_Status_t *status)
{
    return qapi_Net_MQTTc_Set_Will(handle,
            topic, topic_len,
            msg, msg_len,
            qos,
            retained,
            status);
}

static qapi_Status_t qc_drv_net_mqtt_set_keep_alive(int32_t handle,
        uint16_t keepalive_sec)
{
    return qapi_Net_MQTTc_Set_Keep_Alive(handle,
            keepalive_sec);
}

static qapi_Status_t qc_drv_net_mqtt_set_connack_wait_time(int32_t handle,
        uint16_t max_conn_pending_sec)
{
    return qapi_Net_MQTTc_Set_Connack_Wait_Time(handle,
            max_conn_pending_sec);
}


static qapi_Status_t qc_drv_net_mqtt_set_ssl_config(int32_t handle,
        qapi_Net_SSL_Config_t *mqttc_sslcfg,
        qapi_Status_t *status)
{
    return qapi_Net_MQTTc_Set_SSL_Config(handle,
            mqttc_sslcfg,
            status);
}

static qapi_Status_t qc_drv_net_mqtt_connect(int32_t handle,
        char *host,
        qbool_t secure_session,
        qbool_t nonblocking_connect,
        char *bind_if,
        qapi_Status_t *status)
{
    return qapi_Net_MQTTc_Connect(handle,
            host,
            secure_session,
            nonblocking_connect,
            bind_if,
            status);
}

static qapi_Status_t qc_drv_net_mqtt_subscribe(int32_t handle,
        const char *topic,
        uint32_t qos,
        qapi_Status_t *status)
{
    return qapi_Net_MQTTc_Subscribe(handle,
            topic,
            qos,
            status);
}

static qapi_Status_t qc_drv_net_mqtt_publish(int32_t handle,
        const char *topic, size_t topic_len,
        const char *msg, uint32_t msg_len,
        uint32_t qos,
        qbool_t retained,
        qbool_t dup,
        qapi_Status_t *status)
{
    return qapi_Net_MQTTc_Publish(handle,
            topic, topic_len,
            msg, msg_len,
            qos,
            retained,
            dup,
            status);
}

static qapi_Status_t qc_drv_net_mqtt_unsubscribe(int32_t handle,
        const char *topic,
        qapi_Status_t *status)
{
    return qapi_Net_MQTTc_Unsubscribe(handle,
            topic,
            status);
}

static qapi_Status_t qc_drv_net_mqtt_disconnect(int32_t handle,
        qapi_Status_t *status)
{
    return qapi_Net_MQTTc_Disconnect(handle,
            status);
}

/* Start of WiFi OTA */
static qapi_Status_t qc_drv_Fw_Upgrade_Get_Image_ID(qapi_Part_Hdl_t hdl,
        uint32_t *result)
{
    return qapi_Fw_Upgrade_Get_Image_ID(hdl, result);
}
static qapi_Status_t qc_drv_Fw_Upgrade_Get_Image_Version(qapi_Part_Hdl_t hdl,
        uint32_t *result)
{
    return qapi_Fw_Upgrade_Get_Image_Version(hdl, result);
}
static qapi_Status_t qc_drv_Fw_Upgrade_Get_Partition_Start(qapi_Part_Hdl_t hdl,
        uint32_t *result)
{
    return qapi_Fw_Upgrade_Get_Partition_Start(hdl, result);
}
static qapi_Status_t qc_drv_Fw_Upgrade_Get_Partition_Size(qapi_Part_Hdl_t hdl,
        uint32_t *size)
{
    return qapi_Fw_Upgrade_Get_Partition_Size(hdl, size);
}
static qapi_Status_t qc_drv_Fw_Upgrade_Read_Partition(qapi_Part_Hdl_t hdl,
        int32_t val, char *buf, int32_t buf_size, uint32_t *size)
{
    return qapi_Fw_Upgrade_Read_Partition(hdl, val, buf,
            buf_size, size);
}
static qapi_Status_t qc_drv_Fw_Upgrade_init(void)
{
    return qapi_Fw_Upgrade_init();
}
static qapi_Status_t qc_drv_Fw_Upgrade_Get_Active_FWD(uint32_t *boot_type,
        uint32_t *fwd_present)
{
    return qapi_Fw_Upgrade_Get_Active_FWD(boot_type,
            fwd_present);
}
static qapi_Status_t qc_drv_Fw_Upgrade_Get_FWD_Magic(int32_t Index,
        uint32_t *magic)
{
    return qapi_Fw_Upgrade_Get_FWD_Magic(Index, magic);
}
static qapi_Status_t qc_drv_Fw_Upgrade_Get_FWD_Rank(int32_t Index,
        uint32_t *Result_u32)
{
    return qapi_Fw_Upgrade_Get_FWD_Rank(Index,
            Result_u32);
}
static qapi_Status_t qc_drv_Fw_Upgrade_Get_FWD_Version(int32_t Index,
        uint32_t *Result_u32)
{
    return qapi_Fw_Upgrade_Get_FWD_Version(Index,
            Result_u32);
}
static qapi_Status_t qc_drv_Fw_Upgrade_Get_FWD_Status(int32_t Index,
        uint8_t *Result_u8)
{
    return qapi_Fw_Upgrade_Get_FWD_Status(Index,
            Result_u8);
}
static qapi_Status_t qc_drv_Fw_Upgrade_Get_FWD_Total_Images(int32_t Index,
        uint8_t *Result_u8)
{
    return qapi_Fw_Upgrade_Get_FWD_Total_Images(Index,
            Result_u8);
}
static qapi_Status_t qc_drv_Fw_Upgrade_First_Partition(int32_t Index,
        qapi_Part_Hdl_t *hdl)
{
    return qapi_Fw_Upgrade_First_Partition(Index, hdl);
}
static qapi_Status_t qc_drv_Fw_Upgrade_Next_Partition(qapi_Part_Hdl_t hdl,
        qapi_Part_Hdl_t *hdl_next)
{
    return qapi_Fw_Upgrade_Next_Partition(hdl,
            hdl_next);
}
static qapi_Status_t qc_drv_Fw_Upgrade_Close_Partition(qapi_Part_Hdl_t hdl)
{
    return qapi_Fw_Upgrade_Close_Partition(hdl);
}
static qapi_Status_t qc_drv_Fw_Upgrade_Erase_FWD(int32_t fwd_num)
{
    return qapi_Fw_Upgrade_Erase_FWD(fwd_num);
}
static qapi_Status_t qc_drv_Fw_Upgrade_Done(int32_t accept, int32_t flags)
{
    return qapi_Fw_Upgrade_Done(accept, flags);
}
static qapi_Status_t qc_drv_Fw_Upgrade_Find_Partition(uint8_t Index,
        int32_t img_id, qapi_Part_Hdl_t *hdl)
{
    return qapi_Fw_Upgrade_Find_Partition(Index,
            img_id, hdl);
}
static qapi_Status_t qc_drv_Fw_Upgrade(char *iface_name,
        qapi_Fw_Upgrade_Plugin_t *plugin, char *url, char *cfg_file,
        int32_t flags, qapi_Fw_Upgrade_CB_t cb, void *init_param)
{
    return qapi_Fw_Upgrade(iface_name, plugin, url,
            cfg_file, flags, cb, init_param);
}
#endif

/* Start of Net */
static int32_t qc_drv_net_errno(int32_t sock_id)
{
	return qapi_errno(sock_id);
}

static int32_t qc_drv_net_DNSc_Reshost(char *hostname, struct ip46addr *ipaddr)
{
	return qapi_Net_DNSc_Reshost(hostname, ipaddr);
}

static int32_t qc_drv_net_Ping(uint32_t ipv4_Addr,
		                uint32_t size)
{
	    return qapi_Net_Ping(ipv4_Addr, size);
}

static int32_t qc_drv_net_Get_All_Ifnames(qapi_Net_Ifnameindex_t *if_Name_Index)
{
	    return qapi_Net_Get_All_Ifnames(if_Name_Index);
}

static int32_t qc_drv_net_Interface_Exist(const char *interface_Name, qbool_t *if_Is_Up)
{
	    return qapi_Net_Interface_Exist(interface_Name, if_Is_Up);
}

static int32_t qc_drv_net_IPv4_Config (const char *interface_Name,
		            qapi_Net_IPv4cfg_Command_t cmd, uint32_t *ipv4_Addr,
					            uint32_t *subnet_Mask, uint32_t *gateway)
{

	    return qapi_Net_IPv4_Config(interface_Name, cmd, ipv4_Addr,
				            subnet_Mask, gateway);
}

static int32_t qc_drv_net_Bridge_Show_MACs (qapi_Net_Bridge_DB_Entry_t **ptr, uint32_t *count)
{
            return qapi_Net_Bridge_Show_MACs(ptr, count);
}

static int32_t qc_drv_net_Interface_Get_Physical_Address(const char *interface_Name, const uint8_t **address, uint32_t *address_Len)
{
            return qapi_Net_Interface_Get_Physical_Address(interface_Name, address, address_Len);
}

static int32_t qc_drv_net_IPv6_Get_Address(const char *interface_Name,
        uint8_t *link_Local,
        uint8_t *global,
        uint8_t *default_Gateway,
        uint8_t *global_Second,
        uint32_t *link_Local_Prefix,
        uint32_t *global_Prefix,
        uint32_t *default_Gateway_Prefix,
        uint32_t *global_Second_Prefix)
{
        return qapi_Net_IPv6_Get_Address(interface_Name, link_Local, global, default_Gateway, global_Second, link_Local_Prefix, global_Prefix, default_Gateway_Prefix, global_Second_Prefix);
}

static int32_t qc_drv_net_socketclose(int32_t handle)
{
    return qapi_socketclose(handle);
}

static int32_t qc_drv_net_socket(int32_t family, int32_t type, int32_t protocol)
{
    return qapi_socket(family, type, protocol);
}

static int32_t qc_drv_net_connect(int32_t handle, struct sockaddr *srvaddr, int32_t addrlen)
{
       return qapi_connect(handle, srvaddr, addrlen);
}

static int32_t qc_drv_net_send(int32_t handle, char *buf, int32_t len, int32_t flags)
{
        return qapi_send(handle, buf, len, flags);
}

static int32_t qc_drv_net_recv(int32_t handle, char *buf, int32_t len, int32_t flags)
{
       return qapi_recv(handle, buf, len, flags);
}

static int32_t qc_drv_net_recvfrom(int32_t handle, char *buf, int32_t len, int32_t flags, struct sockaddr *from, int32_t *fromlen)
{
    return qapi_recvfrom(handle, buf, len, flags, from, fromlen);
}

static int32_t qc_drv_net_sendto(int32_t handle, char *buf, int32_t len, int32_t flags, struct sockaddr *to, int32_t tolen)
{
    return qapi_sendto(handle, buf, len, flags, to, tolen);
}

static int32_t qc_drv_net_bind(int32_t handle, struct sockaddr *addr, int32_t addrlen)
{
        return qapi_bind(handle, addr, addrlen);
}

static int32_t qc_drv_net_listen(int32_t handle, int32_t backlog)
{
    return qapi_listen(handle, backlog);
}

static int32_t qc_drv_net_accept(int32_t handle, struct sockaddr *cliaddr, int32_t *addrlen)
{
      return qapi_accept(handle, cliaddr, addrlen);
}

static int32_t qc_drv_net_IPv6_Get_Scope_ID(const char *interface_Name, int32_t *scope_ID)
{
    return qapi_Net_IPv6_Get_Scope_ID(interface_Name, scope_ID);
}

static int32_t qc_drv_net_setsockopt(int32_t handle, int32_t level, int32_t optname, void *optval, int32_t optlen)
{
    return qapi_setsockopt(handle, level, optname, optval, optlen);
}

static int32_t qc_drv_net_SSL_Write_To(qapi_Net_SSL_Con_Hdl_t ssl, void *buf, uint32_t num,
                struct sockaddr *to, int32_t to_Len)
{
    return qapi_Net_SSL_Write_To(ssl, buf, num, to, to_Len);
}

static int32_t qc_drv_net_select(qapi_fd_set_t *rd, qapi_fd_set_t *wr, qapi_fd_set_t *ex, int32_t timeout_ms)
{
       return qapi_select(rd, wr, ex, timeout_ms);
}

static int32_t qc_drv_net_SSL_Read(qapi_Net_SSL_Con_Hdl_t hdl, void *buf, uint32_t size)
{
    return qapi_Net_SSL_Read(hdl, buf, size);
}

static void *qc_drv_net_Buf_Alloc(uint32_t size, uint32_t id)
{
    void * result;
   result = qapi_Net_Buf_Alloc(size, id);
   return result;
}

static int32_t qc_drv_net_fd_zero(qapi_fd_set_t *set)
{
    return qapi_fd_zero(set);
}

static int32_t qc_drv_net_fd_set(int32_t handle, qapi_fd_set_t *set)
{
    return qapi_fd_set(handle, set);
}

static int32_t qc_drv_net_Buf_Free(void *buf, uint32_t id)
{
     return qapi_Net_Buf_Free(buf, id);
}

static int32_t qc_drv_net_fd_clr(int32_t handle, qapi_fd_set_t *set)
{
    return qapi_fd_clr(handle, set);
}

static int32_t qc_drv_net_fd_isset(int32_t handle, qapi_fd_set_t *set)
{
    return qapi_fd_isset(handle, set);
}

static int32_t qc_drv_net_SSL_Obj_New(qapi_Net_SSL_Role_t role)
{
     return qapi_Net_SSL_Obj_New(role);
}

static int32_t qc_drv_net_SSL_Shutdown(qapi_Net_SSL_Con_Hdl_t ssl)
{
    return qapi_Net_SSL_Shutdown(ssl);
}

static int32_t qc_drv_net_SSL_Obj_Free(qapi_Net_SSL_Obj_Hdl_t hdl)
{
    return qapi_Net_SSL_Obj_Free(hdl);
}

static int32_t qc_drv_net_SSL_Cipher_Add(qapi_Net_SSL_Config_t * cfg, uint16_t cipher)
{
    return qapi_Net_SSL_Cipher_Add(cfg,cipher);
}

static int32_t qc_drv_net_SSL_ALPN_Protocol_Add(qapi_Net_SSL_Obj_Hdl_t hdl, const char *protocol)
{
    return qapi_Net_SSL_ALPN_Protocol_Add(hdl, protocol);
}

static int32_t qc_drv_net_SSL_Con_New(qapi_Net_SSL_Obj_Hdl_t hdl, qapi_Net_SSL_Protocol_t prot)
{
    return qapi_Net_SSL_Con_New(hdl, prot);
}

static int32_t qc_drv_net_SSL_Configure(qapi_Net_SSL_Con_Hdl_t ssl, qapi_Net_SSL_Config_t *cfg)
{
    return qapi_Net_SSL_Configure(ssl, cfg);
}

static int32_t qc_drv_net_SSL_Cert_Load(qapi_Net_SSL_Obj_Hdl_t hdl, qapi_Net_SSL_Cert_Type_t type, const char * name)
{
    return qapi_Net_SSL_Cert_Load(hdl, type, name);
}

static int32_t qc_drv_net_SSL_Max_Clients_Set(qapi_Net_SSL_Obj_Hdl_t hdl, uint32_t max_Clients)
{
    return qapi_Net_SSL_Max_Clients_Set(hdl, max_Clients);
}

static int32_t qc_drv_net_SSL_Idle_Timeout_Set(qapi_Net_SSL_Obj_Hdl_t hdl, uint32_t idle_Timeout)
{
    return qapi_Net_SSL_Idle_Timeout_Set(hdl, idle_Timeout);
}

static int32_t qc_drv_net_SSL_ECJPAKE_Parameters_Set(qapi_Net_SSL_Obj_Hdl_t hdl, qapi_Net_SSL_ECJPAKE_Params_t *cfg)
{
    return qapi_Net_SSL_ECJPAKE_Parameters_Set(hdl, cfg);
}

static int32_t qc_drv_net_SSL_PSK_Table_Set(qapi_Net_SSL_Obj_Hdl_t hdl, qapi_Net_SSL_PSK_t *psk_Entries, uint16_t num_PSK_Entries)
{
    return qapi_Net_SSL_PSK_Table_Set(hdl, psk_Entries, num_PSK_Entries);
}

static int32_t qc_drv_net_SSL_Con_Get_Status(qapi_Net_SSL_Con_Hdl_t ssl)
{
    return qapi_Net_SSL_Con_Get_Status(ssl);
}

static int32_t qc_drv_net_SSL_Fd_Set(qapi_Net_SSL_Con_Hdl_t ssl, uint32_t fd)
{
    return qapi_Net_SSL_Fd_Set(ssl, fd);
}

static int32_t qc_drv_net_SSL_Accept(qapi_Net_SSL_Con_Hdl_t ssl)
{
    return qapi_Net_SSL_Accept(ssl);
}

static int32_t qc_drv_net_Ping6(uint8_t ipv6_Addr[16], uint32_t size, const char * interface_Name)
{
    return qapi_Net_Ping6(ipv6_Addr, size, interface_Name);
}

static int32_t qc_drv_net_DHCPv4c_Release(const char * interface_Name)
{
    return qapi_Net_DHCPv4c_Release(interface_Name);
}

static int32_t qc_drv_net_IPv6_Config_Router_Prefix(
        const char *interface_Name,
        uint8_t *ipv6_Addr,
        uint32_t prefix_Length,
        uint32_t preferred_Lifetime,
        uint32_t valid_Lifetime)
{
    return qapi_Net_IPv6_Config_Router_Prefix(interface_Name, ipv6_Addr, prefix_Length, preferred_Lifetime, valid_Lifetime);
}

static int32_t qc_drv_net_IPv6_Route_Add(const char *interface_Name, ip6_addr *dest, uint32_t prefix_Length, ip6_addr *next_Hop)
{
    return qapi_Net_IPv6_Route_Add(interface_Name, dest, prefix_Length, next_Hop);
}

static int32_t qc_drv_net_IPv6_Route_Del(const char *interface_Name, ip6_addr *dest, uint32_t prefix_Length)
{
    return qapi_Net_IPv6_Route_Del(interface_Name, dest, prefix_Length);
}

static int32_t qc_drv_net_IPv6_Routing_Table_Get(qapi_Net_IPv6_Route_t *buf, uint32_t *pcount)
{
    return qapi_Net_IPv6_Routing_Table_Get(buf, pcount);
}

static int32_t qc_drv_net_Profile_Set_Custom(qapi_Net_Profile_Custom_Pool_t *pNet_buf, uint8_t net_bufq_size)
{
    return qapi_Net_Profile_Set_Custom(pNet_buf, net_bufq_size);
}

static int32_t qc_drv_net_Profile_Set_Active(qapi_Net_Profile_Type_t profile)
{
    return qapi_Net_Profile_Set_Active(profile);
}

static int32_t qc_drv_net_Profile_Get_Active(qapi_Net_Profile_Custom_Pool_t **pNet_buf, uint8_t *net_bufq_size, qapi_Net_Profile_Type_t *profile)
{
    return qapi_Net_Profile_Get_Active(pNet_buf, net_bufq_size, profile);
}

static int32_t qc_drv_net_OMTM_Switch_Operating_Mode( uint32_t mode_Id, qapi_OMTM_Switch_At_t when )
{
    return qapi_OMTM_Switch_Operating_Mode(mode_Id, when);
}

static int32_t qc_drv_net_DNSs_Is_Started(void)
{
    return qapi_Net_DNSs_Is_Started();
}

static int32_t qc_drv_net_DNSs_Get_Host_List(int32_t *n, qapi_Net_DNS_Host_t *hostlist)
{
    return qapi_Net_DNSs_Get_Host_List(n, hostlist);
}

static int32_t qc_drv_net_DNSs_Add_Host(const char *host_Name, struct ip46addr *host_Addr, uint32_t ttl)
{
    return qapi_Net_DNSs_Add_Host(host_Name, host_Addr, ttl);
}

static int32_t qc_drv_net_DNSs_Del_Host(const char *hostname)
{
    return qapi_Net_DNSs_Del_Host(hostname);
}

static int32_t qc_drv_net_DNSs_Command(qapi_Net_DNS_Server_Command_t cmd)
{
    return qapi_Net_DNSs_Command(cmd);
}

static int32_t qc_drv_net_mDNS_Command(qapi_Net_mDNS_Command_t cmd, void *input, uint8_t blocking, qapi_Net_mDNS_CB_t app_CB)
{
    return qapi_Net_mDNS_Command(cmd, input, blocking, app_CB);
}

static int32_t qc_drv_net_DNSSD_Start(qapi_Net_DNSSD_Start_t *start)
{
    return qapi_Net_DNSSD_Start(start);
}

static int32_t qc_drv_net_DNSSD_Init(qapi_Net_DNSSD_Init_t *init)
{
    return qapi_Net_DNSSD_Init(init);
}

static int32_t qc_drv_net_DNSSD_Stop(qapi_Net_DNSSD_Ctxt_t *ctxt)
{
    return qapi_Net_DNSSD_Stop(ctxt);
}

static int32_t qc_drv_net_DNSSD_Discover(const char *svcName)
{
    return qapi_Net_DNSSD_Discover(svcName);
}

static int32_t qc_drv_net_DNSSD_Get_Target_Info(const char *svcName)
{
    return qapi_Net_DNSSD_Get_Target_Info(svcName);
}

static int32_t qc_drv_net_Buf_Free_Queue_Status(qapi_Net_Buf_Queue_Status_t *arg)
{
    return qapi_Net_Buf_Free_Queue_Status(arg);
}

static int32_t qc_drv_net_Get_Socket_Status(qapi_Net_Socket_Status_t *status)
{
   return qapi_Net_Get_Socket_Status(status);
}

static int32_t qc_drv_net_IPv4_Route(
        const char *interface_Name,
        qapi_Net_Route_Command_t cmd,
        uint32_t *ipv4_Addr,
        uint32_t *subnet_Mask,
        uint32_t *gateway,
        qapi_Net_IPv4_Route_List_t *route_List)
{
    return qapi_Net_IPv4_Route(interface_Name, cmd, ipv4_Addr, subnet_Mask, gateway, route_List);
}

static int32_t qc_drv_net_HTTPc_Start(void)
{
    return qapi_Net_HTTPc_Start();
}

static int32_t qc_drv_net_HTTPc_Stop(void)
{
    return qapi_Net_HTTPc_Stop();
}

static void *qc_drv_net_HTTPc_New_sess(
        uint32_t timeout,
        qapi_Net_SSL_Obj_Hdl_t ssl_Object_Handle,
        qapi_HTTPc_CB_t callback,
        void* arg,
        uint16_t httpc_Max_Body_Length,
        uint16_t httpc_Max_Header_Length) {
    void * result;
    result =  qapi_Net_HTTPc_New_sess(timeout, ssl_Object_Handle, callback, arg, httpc_Max_Body_Length, httpc_Max_Header_Length);
    return result;
}

static int32_t qc_drv_net_HTTPc_Configure_SSL(qapi_Net_HTTPc_handle_t handle, qapi_Net_SSL_Config_t *ssl_Cfg)
{
    return qapi_Net_HTTPc_Configure_SSL(handle, ssl_Cfg);
}

static int32_t qc_drv_net_HTTPc_Connect(qapi_Net_HTTPc_handle_t handle, const char *URL, uint16_t port)
{
    return qapi_Net_HTTPc_Connect(handle, URL, port);
}

static int32_t qc_drv_net_HTTPc_Free_sess(qapi_Net_HTTPc_handle_t handle)
{
    return qapi_Net_HTTPc_Free_sess(handle);
}

static int32_t qc_drv_net_HTTPc_Request(qapi_Net_HTTPc_handle_t handle, qapi_Net_HTTPc_Method_e cmd, const char *URL)
{
    return qapi_Net_HTTPc_Request(handle, cmd, URL);
}

static int32_t qc_drv_net_HTTPc_Set_Body(qapi_Net_HTTPc_handle_t handle, const char *body, uint32_t body_Length)
{
    return qapi_Net_HTTPc_Set_Body(handle, body, body_Length);
}

static int32_t qc_drv_net_HTTPc_Add_Header_Field(qapi_Net_HTTPc_handle_t handle, const char *type, const char *value)
{
    return qapi_Net_HTTPc_Add_Header_Field(handle, type, value);
}

static int32_t qc_drv_net_HTTPc_Clear_Header(qapi_Net_HTTPc_handle_t handle)
{
    return qapi_Net_HTTPc_Clear_Header(handle);
}

static int32_t qc_drv_net_HTTPc_Set_Param(qapi_Net_HTTPc_handle_t handle, const char *key, const char *value)
{
    return qapi_Net_HTTPc_Set_Param(handle, key, value);
}
static int32_t qc_drv_net_HTTPc_CB_Enable_Adding_Header(qapi_Net_HTTPc_handle_t handle, uint16_t enable)
{
    return qapi_Net_HTTPc_CB_Enable_Adding_Header(handle, enable);
}

static int32_t qc_drv_net_Bridge_Set_Aging_Timeout (int32_t Integer_Value)
{
            return qapi_Net_Bridge_Set_Aging_Timeout(Integer_Value);
}

static int32_t qc_drv_net_Bridge_Enable (uint8_t enable)
{
            return qapi_Net_Bridge_Enable(enable);
}

static int32_t qc_drv_net_HTTPs_Set_Buffer_Size (uint16_t txbufsize, uint16_t rxbufsize)
{
            return qapi_Net_HTTPs_Set_Buffer_Size(txbufsize, rxbufsize);
}

static int32_t qc_drv_net_HTTPs_Unregister_Content_Type (char *String_Value, int String_len)
{
            return qapi_Net_HTTPs_Unregister_Content_Type(String_Value, String_len);
}

static int32_t qc_drv_net_HTTPs_Register_Content_Type (char *String_Value, int String_len)
{
            return qapi_Net_HTTPs_Register_Content_Type(String_Value, String_len);
}

static int32_t qc_drv_net_HTTPs_Shutdown ()
{
            return qapi_Net_HTTPs_Shutdown();
}

static int32_t qc_drv_net_HTTPs_Stop ()
{
            return qapi_Net_HTTPs_Stop();
}

static int32_t qc_drv_net_HTTPs_Start ()
{
            return qapi_Net_HTTPs_Start();
}

static int32_t qc_drv_net_HTTPs_Set_SSL_Config (qapi_Net_SSL_Config_t * httpsvr_sslcfg)
{
            return qapi_Net_HTTPs_Set_SSL_Config(httpsvr_sslcfg);
}

static int32_t qc_drv_net_HTTPs_Init (qapi_Net_HTTPs_Config_t *cfg)
{
            return qapi_Net_HTTPs_Init(cfg) ;
}

static int32_t qc_api_get_HTTPs_Is_Started()
{
            return qapi_Net_HTTPs_Is_Started();
}

static int32_t qc_drv_net_HTTPs_Get_Status (qapi_Net_HTTPs_Status_t *status)
{
            return qapi_Net_HTTPs_Get_Status(status);
}

static int32_t qc_drv_net_DHCPv4c_Register_Success_Callback(
		        const char *interface_Name, qapi_Net_DHCPv4c_Success_CB_t CB)
{
	    return qapi_Net_DHCPv4c_Register_Success_Callback(interface_Name, CB);
}

static int32_t qc_drv_net_DHCPv4s_Register_Success_Callback(const char *interface_Name, qapi_Net_DHCPv4s_Success_CB_t CB)
{
	    return qapi_Net_DHCPv4s_Register_Success_Callback(interface_Name, CB);
}

static int32_t qc_drv_net_DHCPv4s_Set_Pool(const char *interface_Name,
		        uint32_t start_IP, uint32_t end_IP, uint32_t lease_Time)
{
	    return qapi_Net_DHCPv4s_Set_Pool(interface_Name, start_IP, end_IP, lease_Time);
}

static int32_t qc_drv_net_DHCPv6c_Register_New_IPv6_Lease_Complete_Callback(const char * interface_Name, qapi_Net_DHCPv6c_New_IPv6_Lease_Complete_CB_t cb, void * cb_Ctxt)
{
	    return qapi_Net_DHCPv6c_Register_New_IPv6_Lease_Complete_Callback( interface_Name, cb, cb_Ctxt);
}

static int32_t qc_drv_net_DHCPv6c_Register_Release_IPv6_Lease_Complete_Callback(const char * interface_Name, qapi_Net_DHCPv6c_Release_IPv6_Lease_Complete_CB_t cb, void *cb_Ctxt)
{
	    return qapi_Net_DHCPv6c_Register_Release_IPv6_Lease_Complete_Callback(interface_Name, cb, cb_Ctxt);
}

static int32_t qc_drv_net_DHCPv6c_Enable(const char *interface_Name)
{
	    return qapi_Net_DHCPv6c_Enable(interface_Name);
}

static int32_t qc_drv_net_DHCPv6c_Disable(const char *interface_Name)
{
	    return qapi_Net_DHCPv6c_Disable(interface_Name);
}

static int32_t qc_drv_net_DHCPv6c_Register_New_Prefix_Lease_Complete_Callback(const char * interface_Name,  qapi_Net_DHCPv6c_New_Prefix_Lease_Complete_CB_t cb, void *cb_Ctxt)
{
	    return qapi_Net_DHCPv6c_Register_New_Prefix_Lease_Complete_Callback(interface_Name, cb, cb_Ctxt);
}

static int32_t qc_drv_net_DHCPv6c_Register_Release_Prefix_Lease_Complete_Callback(const char * interface_Name, qapi_Net_DHCPv6c_Release_Prefix_Lease_Complete_CB_t cb, void *cb_Ctxt)
{
	    return qapi_Net_DHCPv6c_Register_Release_Prefix_Lease_Complete_Callback(interface_Name, cb, cb_Ctxt);
}

static int32_t  qc_drv_net_DHCPv6c_New_Lease(const char * dhcpv6c_Client_Interface_Name, const char * interface_Name)
{
	    return qapi_Net_DHCPv6c_New_Lease(dhcpv6c_Client_Interface_Name, interface_Name);
}
static int32_t qc_drv_net_DHCPv6c_Release_Lease(const char * dhcpv6c_Client_Interface_Name, const char * interface_Name)
{
	    return qapi_Net_DHCPv6c_Release_Lease(dhcpv6c_Client_Interface_Name, interface_Name);
}

static int32_t qc_drv_net_DHCPv6c_Confirm_Lease(const char * dhcpv6c_Client_Interface_Name, const char * interface_Name)
{
	    return qapi_Net_DHCPv6c_Confirm_Lease(dhcpv6c_Client_Interface_Name, interface_Name);
}

static int32_t qc_drv_net_SNTPc_start (void)
{
	    return qapi_Net_SNTPc_Is_Started();
}


static int32_t qc_drv_net_SNTPc_get_srvr_list(qapi_Net_SNTP_Server_List_t *svr_list)
{
	    return qapi_Net_SNTPc_Get_Server_List(svr_list);
}

static int32_t qc_drv_net_SNTPc_add_srvr(char *name, uint32_t ID)
{
	    return qapi_Net_SNTPc_Add_Server(name, ID);
}

static int32_t qc_drv_net_SNTPc_del_srvr(uint32_t id)
{
	    return qapi_Net_SNTPc_Del_Server(id);
}

static int32_t qc_drv_net_SNTPc_cmd(qapi_Net_SNTP_Command_t sntp_flag)
{
	    return qapi_Net_SNTPc_Command(sntp_flag);
}

static int32_t qc_drv_net_SNTPc_get_broke_down_time(qapi_Net_SNTP_Tm_t *tm)
{
	    return qapi_Net_SNTPc_Get_Brokendown_Time(tm);
}

static int32_t qc_drv_net_DNSc_get_srvr_list(qapi_Net_DNS_Server_List_t *svr_list)
{
	    return qapi_Net_DNSc_Get_Server_List(svr_list);
}

static int32_t qc_drv_net_DNSc_cmd(qapi_Net_DNS_Command_t dns_flag)
{
	    return qapi_Net_DNSc_Command(dns_flag);
}


static int32_t qc_drv_net_DNSc_start(void)
{
	    return qapi_Net_DNSc_Is_Started();
}

static int32_t qc_drv_net_DNSc_add_srvr(char *svr, uint32_t id)
{
	    return qapi_Net_DNSc_Add_Server(svr, id);
}

static int32_t qc_drv_net_DNSc_del_srvr(uint32_t id)
{
	    return qapi_Net_DNSc_Del_Server(id);
}


/* Start of BLE */
static qapi_Status_t qc_drv_ble_BSC_Query_Host_Version(char *HostVersion)
{
    HostVersion = qapi_BLE_BSC_Query_Host_Version();
    return QCLI_STATUS_SUCCESS_E;
}

static qapi_Status_t qc_drv_ble_HCI_Read_Local_Version_Information( uint32_t BluetoothStackID, uint8_t *Status, uint8_t *Version, uint16_t *Revision, uint8_t *LMPVersion, uint16_t *ManufacturerName, uint16_t *LMPSubversion)
{
    return qapi_BLE_HCI_Read_Local_Version_Information(BluetoothStackID, Status, Version, Revision, LMPVersion, ManufacturerName, LMPSubversion);
}

static qapi_Status_t qc_drv_ble_GAP_LE_Set_Pairability_Mode(uint32_t BluetoothStackID, qapi_BLE_GAP_LE_Pairability_Mode_t PairabilityMode)
{
    return qapi_BLE_GAP_LE_Set_Pairability_Mode(BluetoothStackID, PairabilityMode);
}

static qapi_Status_t qc_drv_ble_GAP_LE_Authentication_Response(uint32_t BluetoothStackID, qapi_BLE_BD_ADDR_t SecurityRemoteBD_ADDR, qapi_BLE_GAP_LE_Authentication_Response_Information_t *GAP_LE_Authentication_Response_Information)
{
    return qapi_BLE_GAP_LE_Authentication_Response(BluetoothStackID, SecurityRemoteBD_ADDR, GAP_LE_Authentication_Response_Information);
}

static qapi_Status_t qc_drv_ble_GAP_LE_Query_Encryption_Mode(uint32_t BluetoothStackID, qapi_BLE_BD_ADDR_t BD_ADDR, qapi_BLE_GAP_Encryption_Mode_t *GAP_Encryption_Mode)
{
    return qapi_BLE_GAP_LE_Query_Encryption_Mode(BluetoothStackID, BD_ADDR, GAP_Encryption_Mode);
}

static qapi_Status_t qc_drv_ble_GAP_LE_Set_Fixed_Passkey(uint32_t BluetoothStackID, uint32_t *Fixed_Display_Passkey)
{
    return qapi_BLE_GAP_LE_Set_Fixed_Passkey(BluetoothStackID, Fixed_Display_Passkey);
}

static qapi_Status_t qc_drv_ble_GAP_Query_Local_BD_ADDR(uint32_t BluetoothStackID, qapi_BLE_BD_ADDR_t *BD_ADDR)
{
    return qapi_BLE_GAP_Query_Local_BD_ADDR(BluetoothStackID, BD_ADDR);
}

static qapi_Status_t qc_drv_ble_GAP_LE_Advertising_Disable(uint32_t BluetoothStackID)
{
    return qapi_BLE_GAP_LE_Advertising_Disable(BluetoothStackID);
}

static qapi_Status_t qc_drv_ble_GAPS_Query_Device_Appearance(uint32_t BluetoothStackID, uint32_t InstanceID, uint16_t *DeviceAppearance)
{
    return qapi_BLE_GAPS_Query_Device_Appearance(BluetoothStackID, InstanceID, DeviceAppearance);
}

static qapi_Status_t qc_drv_ble_GAPS_Query_Device_Name(uint32_t BluetoothStackID, uint32_t InstanceID, char *NameBuffer)
{
    return qapi_BLE_GAPS_Query_Device_Name(BluetoothStackID, InstanceID, NameBuffer);
}

static qapi_Status_t qc_drv_ble_GAP_LE_Set_Advertising_Data(uint32_t BluetoothStackID, uint32_t Length, qapi_BLE_Advertising_Data_t *Advertising_Data)
{
    return qapi_BLE_GAP_LE_Set_Advertising_Data(BluetoothStackID, Length, Advertising_Data);
}

static qapi_Status_t qc_drv_ble_GAP_LE_Set_Scan_Response_Data(uint32_t BluetoothStackID, uint32_t Length, qapi_BLE_Scan_Response_Data_t *Scan_Response_Data)
{
    return qapi_BLE_GAP_LE_Set_Scan_Response_Data(BluetoothStackID, Length, Scan_Response_Data);
}

static qapi_Status_t qc_drv_ble_BSC_LockBluetoothStack(uint32_t BluetoothStackID)
{
    return qapi_BLE_BSC_LockBluetoothStack(BluetoothStackID);
}

static void qc_drv_ble_BSC_UnLockBluetoothStack(uint32_t BluetoothStackID)
{
    qapi_BLE_BSC_UnLockBluetoothStack(BluetoothStackID);
}

static qapi_Status_t qc_drv_ble_GAP_LE_Advertising_Enable(uint32_t BluetoothStackID, boolean_t EnableScanResponse, qapi_BLE_GAP_LE_Advertising_Parameters_t *GAP_LE_Advertising_Parameters, qapi_BLE_GAP_LE_Connectability_Parameters_t *GAP_LE_Connectability_Parameters, qapi_BLE_GAP_LE_Event_Callback_t GAP_LE_Event_Callback, uint32_t CallbackParameter)
{
    return qapi_BLE_GAP_LE_Advertising_Enable(BluetoothStackID, EnableScanResponse, GAP_LE_Advertising_Parameters, GAP_LE_Connectability_Parameters, GAP_LE_Event_Callback, CallbackParameter);
}

static qapi_Status_t qc_drv_ble_GAP_LE_Disconnect(uint32_t BluetoothStackID, qapi_BLE_BD_ADDR_t BD_ADDR)
{
    return qapi_BLE_GAP_LE_Disconnect(BluetoothStackID, BD_ADDR);
}

static qapi_Status_t qc_drv_ble_GAP_LE_Cancel_Create_Connection(uint32_t BluetoothStackID)
{
    return qapi_BLE_GAP_LE_Cancel_Create_Connection(BluetoothStackID);
}

static qapi_Status_t qc_drv_ble_GATT_Start_Service_Discovery_Handle_Range(uint32_t BluetoothStackID, uint32_t ConnectionID, qapi_BLE_GATT_Attribute_Handle_Group_t *DiscoveryHandleRange, uint32_t NumberOfUUID, qapi_BLE_GATT_UUID_t *UUIDList, qapi_BLE_GATT_Service_Discovery_Event_Callback_t ServiceDiscoveryCallback, uint32_t CallbackParameter)
{
    return qapi_BLE_GATT_Start_Service_Discovery_Handle_Range(BluetoothStackID, ConnectionID, DiscoveryHandleRange, NumberOfUUID, UUIDList, ServiceDiscoveryCallback, CallbackParameter);
}

static qapi_Status_t qc_drv_ble_GATT_Start_Service_Discovery(uint32_t BluetoothStackID, uint32_t ConnectionID, uint32_t NumberOfUUID, qapi_BLE_GATT_UUID_t *UUIDList, qapi_BLE_GATT_Service_Discovery_Event_Callback_t ServiceDiscoveryCallback, uint32_t CallbackParameter)
{
    return qapi_BLE_GATT_Start_Service_Discovery(BluetoothStackID, ConnectionID, NumberOfUUID, UUIDList, ServiceDiscoveryCallback, CallbackParameter);
}

static qapi_Status_t qc_drv_ble_GAP_LE_Add_Device_To_White_List(uint32_t BluetoothStackID, uint32_t DeviceCount, qapi_BLE_GAP_LE_White_List_Entry_t *WhiteListEntries, uint32_t *AddedDeviceCount)
{
    return qapi_BLE_GAP_LE_Add_Device_To_White_List(BluetoothStackID, DeviceCount, WhiteListEntries, AddedDeviceCount);
}

static qapi_Status_t qc_drv_ble_GAP_LE_Remove_Device_From_White_List(uint32_t BluetoothStackID, uint32_t DeviceCount, qapi_BLE_GAP_LE_White_List_Entry_t *WhiteListEntries, uint32_t *RemovedDeviceCount)
{
    return qapi_BLE_GAP_LE_Remove_Device_From_White_List(BluetoothStackID, DeviceCount, WhiteListEntries, RemovedDeviceCount);
}

static qapi_Status_t qc_drv_ble_GAP_LE_Add_Device_To_Resolving_List(uint32_t BluetoothStackID, uint32_t DeviceCount, qapi_BLE_GAP_LE_Resolving_List_Entry_t *ResolvingListEntries, uint32_t *AddedDeviceCount)
{
    return qapi_BLE_GAP_LE_Add_Device_To_Resolving_List(BluetoothStackID, DeviceCount, ResolvingListEntries, AddedDeviceCount);
}

static qapi_Status_t qc_drv_ble_GAP_LE_Remove_Device_From_Resolving_List(uint32_t BluetoothStackID, uint32_t DeviceCount, qapi_BLE_GAP_LE_Resolving_List_Entry_t *ResolvingListEntries, uint32_t *RemovedDeviceCount)
{
    return qapi_BLE_GAP_LE_Remove_Device_From_Resolving_List(BluetoothStackID, DeviceCount, ResolvingListEntries, RemovedDeviceCount);
}

static qapi_Status_t qc_drv_ble_GAP_LE_Set_Authenticated_Payload_Timeout(uint32_t BluetoothStackID, qapi_BLE_BD_ADDR_t BD_ADDR, uint16_t AuthenticatedPayloadTimeout)
{
    return qapi_BLE_GAP_LE_Set_Authenticated_Payload_Timeout(BluetoothStackID, BD_ADDR, AuthenticatedPayloadTimeout);
}

static qapi_Status_t qc_drv_ble_GATT_Change_Maximum_Supported_MTU(uint32_t BluetoothStackID, uint16_t MTU)
{
    return qapi_BLE_GATT_Change_Maximum_Supported_MTU(BluetoothStackID, MTU);
}

static qapi_Status_t qc_drv_ble_GATT_Query_Maximum_Supported_MTU(uint32_t BluetoothStackID, uint16_t *MTU)
{
    return qapi_BLE_GATT_Query_Maximum_Supported_MTU(BluetoothStackID, MTU);
}

static qapi_Status_t qc_drv_ble_GAP_LE_Query_Connection_Handle(uint32_t BluetoothStackID, qapi_BLE_BD_ADDR_t BD_ADDR, uint16_t *Connection_Handle)
{
    return qapi_BLE_GAP_LE_Query_Connection_Handle(BluetoothStackID, BD_ADDR, Connection_Handle);
}

static qapi_Status_t qc_drv_ble_GAP_LE_Set_Data_Length(uint32_t BluetoothStackID, qapi_BLE_BD_ADDR_t BD_ADDR, uint16_t SuggestedTxPacketSize, uint16_t SuggestedTxPacketTime)
{
    return qapi_BLE_GAP_LE_Set_Data_Length(BluetoothStackID, BD_ADDR, SuggestedTxPacketSize, SuggestedTxPacketTime);
}

static qapi_Status_t qc_drv_ble_GAP_LE_Query_Local_Secure_Connections_OOB_Data(uint32_t BluetoothStackID, qapi_BLE_Secure_Connections_Randomizer_t *Randomizer, qapi_BLE_Secure_Connections_Confirmation_t *Confirmation)
{
    return qapi_BLE_GAP_LE_Query_Local_Secure_Connections_OOB_Data(BluetoothStackID, Randomizer, Confirmation);
}

static qapi_Status_t qc_drv_ble_BSC_SetTxPower(uint32_t BluetoothStackID, boolean_t Connection, int8_t TxPower)
{
    return qapi_BLE_BSC_SetTxPower(BluetoothStackID, Connection, TxPower);
}

static qapi_Status_t qc_drv_ble_BSC_Set_FEM_Control_Override(uint32_t BluetoothStackID, boolean_t Enable, uint16_t FEM_Ctrl_0_1, uint16_t FEM_Ctrl_2_3)
{
    return qapi_BLE_BSC_Set_FEM_Control_Override(BluetoothStackID, Enable, FEM_Ctrl_0_1, FEM_Ctrl_2_3);
}

static qapi_Status_t qc_drv_ble_AIOS_Initialize_Service(uint32_t BluetoothStackID, uint32_t Service_Flags, qapi_BLE_AIOS_Initialize_Data_t *InitializeData, qapi_BLE_AIOS_Event_Callback_t EventCallback, uint32_t CallbackParameter, uint32_t *ServiceID)
{
    return qapi_BLE_AIOS_Initialize_Service(BluetoothStackID, Service_Flags, InitializeData, EventCallback, CallbackParameter, ServiceID);
}

static qapi_Status_t qc_drv_ble_AIOS_Cleanup_Service(uint32_t BluetoothStackID, uint32_t InstanceID)
{
    return qapi_BLE_AIOS_Cleanup_Service(BluetoothStackID, InstanceID);
}

static qapi_Status_t qc_drv_ble_GATT_Read_Value_Request(uint32_t BluetoothStackID, uint32_t ConnectionID, uint16_t AttributeHandle, qapi_BLE_GATT_Client_Event_Callback_t ClientEventCallback, uint32_t CallbackParameter)
{
    return qapi_BLE_GATT_Read_Value_Request(BluetoothStackID, ConnectionID, AttributeHandle, ClientEventCallback, CallbackParameter);
}

static qapi_Status_t qc_drv_ble_AIOS_Notify_Characteristic(uint32_t BluetoothStackID, uint32_t InstanceID, uint32_t ConnectionID, qapi_BLE_AIOS_Characteristic_Info_t *CharacteristicInfo, qapi_BLE_AIOS_Characteristic_Data_t *CharacteristicData)
{
    return qapi_BLE_AIOS_Notify_Characteristic(BluetoothStackID, InstanceID, ConnectionID, CharacteristicInfo, CharacteristicData);
}

static qapi_Status_t qc_drv_ble_GATT_Write_Request(uint32_t BluetoothStackID, uint32_t ConnectionID, uint16_t AttributeHandle, uint16_t AttributeLength, void *AttributeValue, qapi_BLE_GATT_Client_Event_Callback_t ClientEventCallback, uint32_t CallbackParameter)
{
    return qapi_BLE_GATT_Write_Request(BluetoothStackID, ConnectionID, AttributeHandle, AttributeLength, AttributeValue, ClientEventCallback, CallbackParameter);
}

static qapi_Status_t qc_drv_ble_HIDS_Initialize_Service(uint32_t BluetoothStackID, uint8_t Flags, qapi_BLE_HIDS_HID_Information_Data_t *HIDInformation, uint32_t NumIncludedServices, uint32_t *ServiceIDList, uint32_t NumExternalReportReferences, qapi_BLE_GATT_UUID_t *ReferenceUUID, uint32_t NumReports, qapi_BLE_HIDS_Report_Reference_Data_t *ReportReference, qapi_BLE_HIDS_Event_Callback_t EventCallback, uint32_t CallbackParameter, uint32_t *ServiceID)
{
    return qapi_BLE_HIDS_Initialize_Service(BluetoothStackID, Flags, HIDInformation, NumIncludedServices, ServiceIDList, NumExternalReportReferences, ReferenceUUID, NumReports, ReportReference, EventCallback, CallbackParameter, ServiceID);
}

static qapi_Status_t qc_drv_ble_HIDS_Cleanup_Service(uint32_t BluetoothStackID, uint32_t InstanceID)
{
    return qapi_BLE_HIDS_Cleanup_Service(BluetoothStackID, InstanceID);
}

static qapi_Status_t qc_drv_ble_HIDS_Notify_Input_Report(uint32_t BluetoothStackID, uint32_t InstanceID, uint32_t ConnectionID, qapi_BLE_HIDS_Report_Type_t ReportType, qapi_BLE_HIDS_Report_Reference_Data_t *ReportReferenceData, uint16_t InputReportLength, uint8_t *InputReportData)
{
    return qapi_BLE_HIDS_Notify_Input_Report(BluetoothStackID, InstanceID, ConnectionID, ReportType, ReportReferenceData, InputReportLength, InputReportData);
}

static qapi_Status_t qc_drv_ble_HIDS_Format_Control_Point_Command(qapi_BLE_HIDS_Control_Point_Command_t Command, uint32_t BufferLength, uint8_t *Buffer)
{
    return qapi_BLE_HIDS_Format_Control_Point_Command(Command, BufferLength, Buffer);
}

static qapi_Status_t qc_drv_ble_GATT_Write_Without_Response_Request(uint32_t BluetoothStackID, uint32_t ConnectionID, uint16_t AttributeHandle, uint16_t AttributeLength, void *AttributeValue)
{
    return qapi_BLE_GATT_Write_Without_Response_Request(BluetoothStackID, ConnectionID, AttributeHandle, AttributeLength, AttributeValue);
}

static qapi_Status_t qc_drv_ble_HIDS_Format_Protocol_Mode (qapi_BLE_HIDS_Protocol_Mode_t ProtocolMode, uint32_t BufferLength, uint8_t *Buffer)
{
    return  qapi_BLE_HIDS_Format_Protocol_Mode( ProtocolMode, BufferLength, Buffer);
}

static qapi_Status_t qc_drv_ble_SCPS_Initialize_Service(uint32_t BluetoothStackID, qapi_BLE_SCPS_Event_Callback_t EventCallback, uint32_t CallbackParameter, uint32_t *ServiceID)
{
    return qapi_BLE_SCPS_Initialize_Service(BluetoothStackID, EventCallback, CallbackParameter, ServiceID);
}

static qapi_Status_t qc_drv_ble_SCPS_Cleanup_Service(uint32_t BluetoothStackID, uint32_t InstanceID)
{
    return qapi_BLE_SCPS_Cleanup_Service(BluetoothStackID, InstanceID);
}

static qapi_Status_t qc_drv_ble_SCPS_Format_Scan_Interval_Window(qapi_BLE_SCPS_Scan_Interval_Window_Data_t *Scan_Interval_Window, uint32_t BufferLength, uint8_t *Buffer)
{
    return qapi_BLE_SCPS_Format_Scan_Interval_Window(Scan_Interval_Window, BufferLength, Buffer);
}

static  qapi_Status_t qc_drv_ble_SCPS_Notify_Scan_Refresh(uint32_t BluetoothStackID, uint32_t InstanceID, uint32_t ConnectionID, uint8_t ScanRefreshValue)
{
    return qapi_BLE_SCPS_Notify_Scan_Refresh(BluetoothStackID, InstanceID, ConnectionID, ScanRefreshValue);
}

static qapi_Status_t qc_drv_ble_GATT_Register_Service(uint32_t BluetoothStackID, uint8_t ServiceFlags, uint32_t NumberOfServiceAttributeEntries, qapi_BLE_GATT_Service_Attribute_Entry_t *ServiceTable, qapi_BLE_GATT_Attribute_Handle_Group_t *ServiceHandleGroupResult, qapi_BLE_GATT_Server_Event_Callback_t ServerEventCallback, uint32_t CallbackParameter)
{
    return qapi_BLE_GATT_Register_Service(BluetoothStackID, ServiceFlags, NumberOfServiceAttributeEntries, ServiceTable, ServiceHandleGroupResult, ServerEventCallback, CallbackParameter);
}

static void qc_drv_ble_GATT_Un_Register_Service(uint32_t BluetoothStackID, uint32_t ServiceID)
{
    qapi_BLE_GATT_Un_Register_Service(BluetoothStackID, ServiceID);
}


static qapi_Status_t qc_drv_ble_BAS_Initialize_Service(uint32_t BluetoothStackID, qapi_BLE_BAS_Event_Callback_t EventCallback, uint32_t CallbackParameter, uint32_t *ServiceID)
{
    return qapi_BLE_BAS_Initialize_Service(BluetoothStackID, EventCallback, CallbackParameter, ServiceID);
}

static qapi_Status_t qc_drv_ble_BAS_Set_Characteristic_Presentation_Format(uint32_t BluetoothStackID, uint32_t InstanceID, qapi_BLE_BAS_Presentation_Format_Data_t *CharacteristicPresentationFormat)
{
    return qapi_BLE_BAS_Set_Characteristic_Presentation_Format(BluetoothStackID, InstanceID, CharacteristicPresentationFormat);
}

static qapi_Status_t qc_drv_ble_BAS_Cleanup_Service(uint32_t BluetoothStackID, uint32_t InstanceID)
{
    return qapi_BLE_BAS_Cleanup_Service(BluetoothStackID, InstanceID);
}

static qapi_Status_t qc_drv_ble_BAS_Notify_Battery_Level(uint32_t BluetoothStackID, uint32_t InstanceID, uint32_t ConnectionID, uint8_t BatteryLevel)
{
    return qapi_BLE_BAS_Notify_Battery_Level(BluetoothStackID, InstanceID, ConnectionID, BatteryLevel);
}
static qapi_Status_t qc_drv_ble_BAS_Query_Characteristic_Presentation_Format(uint32_t BluetoothStackID, uint32_t InstanceID, qapi_BLE_BAS_Presentation_Format_Data_t *CharacteristicPresentationFormat)
{
    return qapi_BLE_BAS_Query_Characteristic_Presentation_Format(BluetoothStackID, InstanceID, CharacteristicPresentationFormat);
}

static qapi_Status_t qc_drv_ble_GAPS_Set_Device_Name(uint32_t BluetoothStackID, uint32_t InstanceID, char *DeviceName)
{
    return qapi_BLE_GAPS_Set_Device_Name(BluetoothStackID, InstanceID, DeviceName);
}

static qapi_Status_t qc_drv_ble_GAPS_Set_Device_Appearance(uint32_t BluetoothStackID, uint32_t InstanceID, uint16_t DeviceAppearance)
{
    return qapi_BLE_GAPS_Set_Device_Appearance(BluetoothStackID, InstanceID, DeviceAppearance);
}

static qapi_Status_t qc_drv_ble_DIS_Initialize_Service(uint32_t BluetoothStackID, uint32_t *ServiceID)
{
    return qapi_BLE_DIS_Initialize_Service(BluetoothStackID, ServiceID);
}

static qapi_Status_t qc_drv_ble_DIS_Set_Manufacturer_Name(uint32_t BluetoothStackID, uint32_t InstanceID, char *ManufacturerName)
{
    return qapi_BLE_DIS_Set_Manufacturer_Name(BluetoothStackID, InstanceID, ManufacturerName);
}

static qapi_Status_t qc_drv_ble_DIS_Set_Model_Number(uint32_t BluetoothStackID, uint32_t InstanceID, char *ModelNumber)
{
    return qapi_BLE_DIS_Set_Model_Number(BluetoothStackID,InstanceID, ModelNumber);
}

static qapi_Status_t qc_drv_ble_DIS_Set_Software_Revision(uint32_t BluetoothStackID, uint32_t InstanceID, char *SoftwareRevision)
{
    return qapi_BLE_DIS_Set_Software_Revision(BluetoothStackID, InstanceID, SoftwareRevision);
}

static qapi_Status_t qc_drv_ble_DIS_Set_Hardware_Revision(uint32_t BluetoothStackID, uint32_t InstanceID, char *Hardware_Revision)
{

    return qapi_BLE_DIS_Set_Hardware_Revision(BluetoothStackID, InstanceID, Hardware_Revision);
}

static qapi_Status_t qc_drv_ble_DIS_Set_Firmware_Revision(uint32_t BluetoothStackID, uint32_t InstanceID, char *FirmwareRevision)
{
    return qapi_BLE_DIS_Set_Firmware_Revision(BluetoothStackID, InstanceID, FirmwareRevision);
}

static qapi_Status_t qc_drv_ble_DIS_Cleanup_Service(uint32_t BluetoothStackID, uint32_t InstanceID)
{
    return qapi_BLE_DIS_Cleanup_Service(BluetoothStackID, InstanceID);
}

static qapi_Status_t qc_drv_ble_TPS_Initialize_Service(int32_t BluetoothStackID, uint32_t *ServiceID)
{
    return qapi_BLE_TPS_Initialize_Service(BluetoothStackID, ServiceID);
}

static qapi_Status_t qc_drv_ble_TPS_Set_Tx_Power_Level(int32_t BluetoothStackID, uint32_t InstanceID, int8_t Tx_Power_Level)
{
    return qapi_BLE_TPS_Set_Tx_Power_Level(BluetoothStackID, InstanceID, Tx_Power_Level);
}

static qapi_Status_t qc_drv_ble_TPS_Cleanup_Service(int32_t BluetoothStackID, uint32_t InstanceID)
{
    return qapi_BLE_TPS_Cleanup_Service(BluetoothStackID, InstanceID);
}

static qapi_Status_t qc_drv_ble_GAP_LE_Query_Connection_PHY(uint32_t BluetoothStackID, qapi_BLE_BD_ADDR_t BD_ADDR, qapi_BLE_GAP_LE_PHY_Type_t *TxPHY, qapi_BLE_GAP_LE_PHY_Type_t *RxPHY)
{
    return qapi_BLE_GAP_LE_Query_Connection_PHY(BluetoothStackID, BD_ADDR, TxPHY, RxPHY);
}

static qapi_Status_t qc_drv_ble_GAP_LE_Set_Connection_PHY(uint32_t BluetoothStackID, qapi_BLE_BD_ADDR_t BD_ADDR, uint32_t TxPHYSPreference, uint32_t RxPHYSPreference)
{
    return qapi_BLE_GAP_LE_Set_Connection_PHY(BluetoothStackID, BD_ADDR, TxPHYSPreference, RxPHYSPreference);
}

static qapi_Status_t qc_drv_ble_GAP_LE_Set_Extended_Advertising_Parameters(uint32_t BluetoothStackID, uint8_t AdvertisingHandle, qapi_BLE_GAP_LE_Extended_Advertising_Parameters_t *AdvertisingParameters, int8_t *SelectedTxPower)
{
    return qapi_BLE_GAP_LE_Set_Extended_Advertising_Parameters(BluetoothStackID, AdvertisingHandle, AdvertisingParameters, SelectedTxPower);
}

static qapi_Status_t qc_drv_ble_GAP_LE_Enable_Extended_Advertising(uint32_t BluetoothStackID, boolean_t Enable, uint8_t NumberOfSets, uint8_t *AdvertisingHandleList, uint32_t *DurationList, uint8_t *MaxExtendedAdvertisingEventList, qapi_BLE_GAP_LE_Event_Callback_t GAP_LE_Event_Callback, uint32_t CallbackParameter)
{
    return qapi_BLE_GAP_LE_Enable_Extended_Advertising(BluetoothStackID, Enable, NumberOfSets, AdvertisingHandleList, DurationList, MaxExtendedAdvertisingEventList, GAP_LE_Event_Callback, CallbackParameter);

}

static qapi_Status_t qc_drv_ble_GAP_LE_Set_Extended_Scan_Parameters(uint32_t BluetoothStackID, qapi_BLE_GAP_LE_Address_Type_t LocalAddressType, qapi_BLE_GAP_LE_Filter_Policy_t FilterPolicy, uint32_t NumberScanningPHYs, qapi_BLE_GAP_LE_Extended_Scanning_PHY_Parameters_t *ScanningParameterList)
{
    return  qapi_BLE_GAP_LE_Set_Extended_Scan_Parameters(BluetoothStackID, LocalAddressType, FilterPolicy, NumberScanningPHYs, ScanningParameterList);

}

static qapi_Status_t qc_drv_ble_GAP_LE_Enable_Extended_Scan(uint32_t BluetoothStackID, boolean_t Enable, qapi_BLE_GAP_LE_Extended_Scan_Filter_Duplicates_Type_t FilterDuplicates, uint32_t Duration, uint32_t Period, qapi_BLE_GAP_LE_Event_Callback_t GAP_LE_Event_Callback, uint32_t CallbackParameter)
{
    return qapi_BLE_GAP_LE_Enable_Extended_Scan(BluetoothStackID, Enable, FilterDuplicates, Duration, Period, GAP_LE_Event_Callback, CallbackParameter);

}

static qapi_Status_t qc_drv_ble_GAP_LE_Extended_Create_Connection(uint32_t BluetoothStackID, qapi_BLE_GAP_LE_Filter_Policy_t InitatorFilterPolicy, qapi_BLE_GAP_LE_Address_Type_t RemoteAddressType, qapi_BLE_BD_ADDR_t *RemoteDevice, qapi_BLE_GAP_LE_Address_Type_t LocalAddressType, uint32_t NumberOfConnectionParameters, qapi_BLE_GAP_LE_Extended_Connection_Parameters_t *ConnectionParameterList, qapi_BLE_GAP_LE_Event_Callback_t GAP_LE_Event_Callback, uint32_t CallbackParameter)
{
    return qapi_BLE_GAP_LE_Extended_Create_Connection(BluetoothStackID, InitatorFilterPolicy, RemoteAddressType, RemoteDevice, LocalAddressType, NumberOfConnectionParameters, ConnectionParameterList, GAP_LE_Event_Callback, CallbackParameter);

}

static qapi_Status_t qc_drv_ble_BSC_AddGenericListEntry_Actual(qapi_BLE_BSC_Generic_List_Entry_Key_t GenericListEntryKey, uint32_t ListEntryKeyOffset, uint32_t ListEntryNextPointerOffset, void **ListHead, void *ListEntryToAdd)
{
    return qapi_BLE_BSC_AddGenericListEntry_Actual(GenericListEntryKey, ListEntryKeyOffset, ListEntryNextPointerOffset, ListHead, ListEntryToAdd);
}

static qapi_Status_t qc_drv_ble_BSC_Initialize(qapi_BLE_HCI_DriverInformation_t *HCI_DriverInformation, uint32_t Flags)
{
    return qapi_BLE_BSC_Initialize(HCI_DriverInformation, Flags);
}

static qapi_Status_t qc_drv_ble_HCI_LE_Read_Buffer_Size(uint32_t BluetoothStackID, uint8_t *StatusResult, uint16_t *HC_LE_ACL_Data_Packet_Length, uint8_t *HC_Total_Num_LE_ACL_Data_Packets)
{
    return qapi_BLE_HCI_LE_Read_Buffer_Size(BluetoothStackID, StatusResult, HC_LE_ACL_Data_Packet_Length, HC_Total_Num_LE_ACL_Data_Packets);
}

static qapi_Status_t qc_drv_ble_GATT_Initialize(uint32_t BluetoothStackID, uint32_t Flags, qapi_BLE_GATT_Connection_Event_Callback_t ConnectionEventCallback, uint32_t CallbackParameter)
{
    return qapi_BLE_GATT_Initialize(BluetoothStackID, Flags, ConnectionEventCallback, CallbackParameter);
}

static qapi_Status_t qc_drv_ble_GAPS_Initialize_Service(uint32_t BluetoothStackID, uint32_t *ServiceID)
{
    return qapi_BLE_GAPS_Initialize_Service(BluetoothStackID, ServiceID);
}

static qapi_Status_t qc_drv_ble_GAP_LE_Set_Address_Resolution_Enable(uint32_t BluetoothStackID, boolean_t EnableAddressResolution)
{
    return qapi_BLE_GAP_LE_Set_Address_Resolution_Enable(BluetoothStackID, EnableAddressResolution);
}

static qapi_Status_t qc_drv_ble_GAP_LE_Set_Resolvable_Private_Address_Timeout(uint32_t BluetoothStackID, uint32_t RPA_Timeout)
{
    return qapi_BLE_GAP_LE_Set_Resolvable_Private_Address_Timeout(BluetoothStackID, RPA_Timeout);
}

static qapi_Status_t qc_drv_ble_HCI_Register_Event_Callback(uint32_t BluetoothStackID, qapi_BLE_HCI_Event_Callback_t HCI_EventCallback, uint32_t CallbackParameter)
{
    return qapi_BLE_HCI_Register_Event_Callback(BluetoothStackID, HCI_EventCallback, CallbackParameter);
}

static qapi_Status_t qc_drv_ble_HCI_Register_ACL_Data_Callback(uint32_t BluetoothStackID, qapi_BLE_HCI_ACL_Data_Callback_t HCI_ACLDataCallback, uint32_t CallbackParameter)
{
    return qapi_BLE_HCI_Register_ACL_Data_Callback(BluetoothStackID, HCI_ACLDataCallback, CallbackParameter);
}

static qapi_Status_t qc_drv_ble_HCI_Send_ACL_Data(uint32_t BluetoothStackID, uint16_t Connection_Handle, uint16_t Flags, uint16_t ACLDataLength, uint8_t *ACLData)
{
    return qapi_BLE_HCI_Send_ACL_Data(BluetoothStackID, Connection_Handle, Flags, ACLDataLength, ACLData);
}

static qapi_Status_t qc_drv_ble_GAP_LE_Register_Remote_Authentication(uint32_t BluetoothStackID, qapi_BLE_GAP_LE_Event_Callback_t GAP_LE_Event_Callback, uint32_t CallbackParameter)
{
    return qapi_BLE_GAP_LE_Register_Remote_Authentication(BluetoothStackID, GAP_LE_Event_Callback, CallbackParameter);
}

static qapi_Status_t qc_drv_ble_BSC_StartTimer(uint32_t BluetoothStackID, uint32_t Timeout, qapi_BLE_BSC_Timer_Callback_t TimerCallback, uint32_t CallbackParameter)
{
    return qapi_BLE_BSC_StartTimer(BluetoothStackID, Timeout, TimerCallback, CallbackParameter);
}

static qapi_Status_t qc_drv_ble_GAP_LE_Perform_Scan(uint32_t BluetoothStackID, qapi_BLE_GAP_LE_Scan_Type_t ScanType, uint32_t ScanInterval, uint32_t ScanWindow, qapi_BLE_GAP_LE_Address_Type_t LocalAddressType, qapi_BLE_GAP_LE_Filter_Policy_t FilterPolicy, boolean_t FilterDuplicates, qapi_BLE_GAP_LE_Event_Callback_t GAP_LE_Event_Callback, uint32_t CallbackParameter)
{
    return qapi_BLE_GAP_LE_Perform_Scan(BluetoothStackID, ScanType, ScanInterval, ScanWindow, LocalAddressType, FilterPolicy, FilterDuplicates, GAP_LE_Event_Callback, CallbackParameter);
}

static qapi_Status_t qc_drv_ble_GAP_LE_Cancel_Scan(uint32_t BluetoothStackID)
{
    return qapi_BLE_GAP_LE_Cancel_Scan(BluetoothStackID);
}

static qapi_Status_t qc_drv_ble_GAP_LE_Create_Connection(uint32_t BluetoothStackID, uint32_t ScanInterval, uint32_t ScanWindow, qapi_BLE_GAP_LE_Filter_Policy_t InitatorFilterPolicy, qapi_BLE_GAP_LE_Address_Type_t RemoteAddressType, qapi_BLE_BD_ADDR_t *RemoteDevice, qapi_BLE_GAP_LE_Address_Type_t LocalAddressType, qapi_BLE_GAP_LE_Connection_Parameters_t *ConnectionParameters, qapi_BLE_GAP_LE_Event_Callback_t GAP_LE_Event_Callback, uint32_t CallbackParameter)
{
    return qapi_BLE_GAP_LE_Create_Connection(BluetoothStackID, ScanInterval, ScanWindow, InitatorFilterPolicy, RemoteAddressType, RemoteDevice, LocalAddressType, ConnectionParameters, GAP_LE_Event_Callback, CallbackParameter);
}

static qapi_Status_t qc_drv_ble_HRS_Decode_Heart_Rate_Measurement(uint32_t ValueLength, uint8_t *Value, qapi_BLE_HRS_Heart_Rate_Measurement_Data_t *HeartRateMeasurement)
{
    return qapi_BLE_HRS_Decode_Heart_Rate_Measurement(ValueLength, Value, HeartRateMeasurement);
}

static qapi_Status_t qc_drv_ble_GATT_Handle_Value_Notification(uint32_t BluetoothStackID, uint32_t ServiceID, uint32_t ConnectionID, uint16_t AttributeOffset, uint16_t AttributeValueLength, uint8_t *AttributeValue)
{
    return qapi_BLE_GATT_Handle_Value_Notification(BluetoothStackID, ServiceID, ConnectionID, AttributeOffset, AttributeValueLength, AttributeValue);
}

static qapi_Status_t qc_drv_ble_BSC_GetTxPower(uint32_t BluetoothStackID, boolean_t Connection, int8_t *TxPower)
{
    return qapi_BLE_BSC_GetTxPower(BluetoothStackID, Connection, TxPower);
}

static qapi_Status_t qc_drv_ble_GAP_LE_Reestablish_Security(uint32_t BluetoothStackID, qapi_BLE_BD_ADDR_t BD_ADDR, qapi_BLE_GAP_LE_Security_Information_t *SecurityInformation, qapi_BLE_GAP_LE_Event_Callback_t GAP_LE_Event_Callback, uint32_t CallbackParameter)
{
    return qapi_BLE_GAP_LE_Reestablish_Security(BluetoothStackID, BD_ADDR, SecurityInformation, GAP_LE_Event_Callback, CallbackParameter);
}

static qapi_Status_t qc_drv_ble_GAP_LE_Regenerate_Long_Term_Key(uint32_t BluetoothStackID, qapi_BLE_Encryption_Key_t *DHK, qapi_BLE_Encryption_Key_t *ER, uint16_t EDIV, qapi_BLE_Random_Number_t *Rand, qapi_BLE_Long_Term_Key_t *LTK_Result)
{
    return qapi_BLE_GAP_LE_Regenerate_Long_Term_Key(BluetoothStackID, DHK, ER, EDIV, Rand, LTK_Result);
}

static qapi_Status_t qc_drv_ble_AIOS_Read_Characteristic_Request_Response(uint32_t BluetoothStackID, uint32_t InstanceID, uint32_t ConnectionID, uint32_t TransactionID, uint8_t ErrorCode, qapi_BLE_AIOS_Characteristic_Info_t *CharacteristicInfo, qapi_BLE_AIOS_Characteristic_Data_t *CharacteristicData)
{
    return qapi_BLE_AIOS_Read_Characteristic_Request_Response(BluetoothStackID, InstanceID, ConnectionID, TransactionID, ErrorCode, CharacteristicInfo, CharacteristicData);
}

static qapi_Status_t qc_drv_ble_AIOS_Write_Characteristic_Request_Response(uint32_t BluetoothStackID, uint32_t InstanceID, uint32_t TransactionID, uint8_t ErrorCode, qapi_BLE_AIOS_Characteristic_Info_t *CharacteristicInfo)
{
    return qapi_BLE_AIOS_Write_Characteristic_Request_Response(BluetoothStackID, InstanceID, TransactionID, ErrorCode, CharacteristicInfo);
}

static qapi_Status_t qc_drv_ble_AIOS_Read_CCCD_Request_Response(uint32_t BluetoothStackID, uint32_t InstanceID, uint32_t TransactionID, uint8_t ErrorCode, qapi_BLE_AIOS_Characteristic_Info_t *CharacteristicInfo, uint16_t ClientConfiguration)
{
    return qapi_BLE_AIOS_Read_CCCD_Request_Response(BluetoothStackID, InstanceID, TransactionID, ErrorCode, CharacteristicInfo, ClientConfiguration);
}

static qapi_Status_t qc_drv_ble_AIOS_Write_CCCD_Request_Response(uint32_t BluetoothStackID, uint32_t InstanceID, uint32_t TransactionID, uint8_t ErrorCode, qapi_BLE_AIOS_Characteristic_Info_t *CharacteristicInfo)
{
    return qapi_BLE_AIOS_Write_CCCD_Request_Response(BluetoothStackID, InstanceID, TransactionID, ErrorCode, CharacteristicInfo);
}

static qapi_Status_t qc_drv_ble_AIOS_Read_Presentation_Format_Request_Response(uint32_t BluetoothStackID, uint32_t InstanceID, uint32_t TransactionID, uint8_t ErrorCode, qapi_BLE_AIOS_Characteristic_Info_t *CharacteristicInfo, qapi_BLE_AIOS_Presentation_Format_Data_t *PresentationFormatData)
{
    return qapi_BLE_AIOS_Read_Presentation_Format_Request_Response(BluetoothStackID, InstanceID, TransactionID, ErrorCode, CharacteristicInfo, PresentationFormatData);
}

static qapi_Status_t qc_drv_ble_AIOS_Read_Number_Of_Digitals_Request_Response(uint32_t BluetoothStackID, uint32_t InstanceID, uint32_t TransactionID, uint8_t ErrorCode, qapi_BLE_AIOS_Characteristic_Info_t *CharacteristicInfo, uint8_t NumberOfDigitals)
{
    return qapi_BLE_AIOS_Read_Number_Of_Digitals_Request_Response(BluetoothStackID, InstanceID, TransactionID, ErrorCode, CharacteristicInfo, NumberOfDigitals);
}

static qapi_Status_t qc_drv_ble_BAS_Read_Client_Configuration_Response(uint32_t BluetoothStackID, uint32_t InstanceID, uint32_t TransactionID, uint16_t Client_Configuration)
{
    return qapi_BLE_BAS_Read_Client_Configuration_Response(BluetoothStackID, InstanceID, TransactionID, Client_Configuration);
}

static qapi_Status_t qc_drv_ble_HIDS_Read_Client_Configuration_Response(uint32_t BluetoothStackID, uint32_t InstanceID, uint32_t TransactionID, uint16_t Client_Configuration)
{
    return qapi_BLE_HIDS_Read_Client_Configuration_Response(BluetoothStackID, InstanceID, TransactionID, Client_Configuration);
}

static qapi_Status_t qc_drv_ble_HIDS_Get_Protocol_Mode_Response(uint32_t BluetoothStackID, uint32_t InstanceID, uint32_t TransactionID, uint8_t ErrorCode, qapi_BLE_HIDS_Protocol_Mode_t CurrentProtocolMode)
{
    return qapi_BLE_HIDS_Get_Protocol_Mode_Response(BluetoothStackID, InstanceID, TransactionID, ErrorCode, CurrentProtocolMode);
}

static qapi_Status_t qc_drv_ble_HIDS_Get_Report_Map_Response(uint32_t BluetoothStackID, uint32_t InstanceID, uint32_t TransactionID, uint8_t ErrorCode, uint32_t ReportMapLength, uint8_t *ReportMap)
{
    return qapi_BLE_HIDS_Get_Report_Map_Response(BluetoothStackID, InstanceID, TransactionID, ErrorCode, ReportMapLength, ReportMap);
}

static qapi_Status_t qc_drv_ble_HIDS_Get_Report_Response(uint32_t BluetoothStackID, uint32_t InstanceID, uint32_t TransactionID, qapi_BLE_HIDS_Report_Type_t ReportType, qapi_BLE_HIDS_Report_Reference_Data_t *ReportReferenceData, uint8_t ErrorCode, uint32_t ReportLength, uint8_t *Report)
{
    return qapi_BLE_HIDS_Get_Report_Response(BluetoothStackID, InstanceID, TransactionID, ReportType, ReportReferenceData, ErrorCode, ReportLength, Report);
}

static qapi_Status_t qc_drv_ble_HIDS_Set_Report_Response(uint32_t BluetoothStackID, uint32_t InstanceID, uint32_t TransactionID, qapi_BLE_HIDS_Report_Type_t ReportType, qapi_BLE_HIDS_Report_Reference_Data_t *ReportReferenceData, uint8_t ErrorCode)
{
    return qapi_BLE_HIDS_Set_Report_Response(BluetoothStackID, InstanceID, TransactionID, ReportType, ReportReferenceData, ErrorCode);
}

static qapi_Status_t qc_drv_ble_SCPS_Read_Client_Configuration_Response(uint32_t BluetoothStackID, uint32_t InstanceID, uint32_t TransactionID, uint16_t Client_Configuration)
{
    return qapi_BLE_SCPS_Read_Client_Configuration_Response(BluetoothStackID, InstanceID, TransactionID, Client_Configuration);
}

static qapi_Status_t qc_drv_ble_AIOS_Decode_Presentation_Format(uint32_t ValueLength, uint8_t *Value, qapi_BLE_AIOS_Presentation_Format_Data_t *PresentationFormatData)
{
    return qapi_BLE_AIOS_Decode_Presentation_Format(ValueLength, Value, PresentationFormatData);
}

static qapi_Status_t qc_drv_ble_BAS_Decode_Characteristic_Presentation_Format(uint32_t ValueLength, uint8_t *Value, qapi_BLE_BAS_Presentation_Format_Data_t *CharacteristicPresentationFormat)
{
    return qapi_BLE_BAS_Decode_Characteristic_Presentation_Format(ValueLength, Value, CharacteristicPresentationFormat);
}

static qapi_Status_t qc_drv_ble_GATT_Read_Long_Value_Request(uint32_t BluetoothStackID, uint32_t ConnectionID, uint16_t AttributeHandle, uint16_t AttributeOffset, qapi_BLE_GATT_Client_Event_Callback_t ClientEventCallback, uint32_t CallbackParameter)
{
    return qapi_BLE_GATT_Read_Long_Value_Request(BluetoothStackID, ConnectionID, AttributeHandle, AttributeOffset, ClientEventCallback, CallbackParameter);
}

static qapi_Status_t qc_drv_ble_HIDS_Decode_HID_Information(uint32_t ValueLength, uint8_t *Value, qapi_BLE_HIDS_HID_Information_Data_t *HIDSHIDInformation)
{
    return qapi_BLE_HIDS_Decode_HID_Information(ValueLength, Value, HIDSHIDInformation);
}

static qapi_Status_t qc_drv_ble_HIDS_Decode_Report_Reference(uint32_t ValueLength, uint8_t *Value, qapi_BLE_HIDS_Report_Reference_Data_t *ReportReferenceData)
{
    return qapi_BLE_HIDS_Decode_Report_Reference(ValueLength, Value, ReportReferenceData);
}

static qapi_Status_t qc_drv_ble_HIDS_Decode_External_Report_Reference(uint32_t ValueLength, uint8_t *Value, qapi_BLE_GATT_UUID_t *ExternalReportReferenceUUID)
{
    return qapi_BLE_HIDS_Decode_External_Report_Reference(ValueLength, Value, ExternalReportReferenceUUID);
}






static qapi_Status_t qc_drv_ZB_Initialize(qapi_ZB_Handle_t *ZB_Handle, qapi_ZB_Event_CB_t ZB_Event_CB, uint32_t CB_Param)
{
    return qapi_ZB_Initialize(ZB_Handle, ZB_Event_CB, CB_Param);
}

static qapi_Status_t qc_drv_ZB_Register_Persist_Notify_CB(qapi_ZB_Handle_t ZB_Handle, qapi_ZB_Persist_Notify_CB_t ZB_Persist_Notify_CB, uint32_t CB_Param)
{
    return qapi_ZB_Register_Persist_Notify_CB(ZB_Handle, ZB_Persist_Notify_CB, CB_Param);
}

static qapi_Status_t qc_drv_ZB_ZDP_Register_Callback(qapi_ZB_Handle_t ZB_Handle, qapi_ZB_ZDP_Event_CB_t ZDP_Event_CB, uint32_t CB_Param)
{
    return qapi_ZB_ZDP_Register_Callback(ZB_Handle, ZDP_Event_CB, CB_Param);
}

static qapi_Status_t qc_drv_ZB_Get_Extended_Address(qapi_ZB_Handle_t ZB_Handle, uint64_t *Extended_Address)
{
    return qapi_ZB_Get_Extended_Address(ZB_Handle, Extended_Address);
}

static qapi_Status_t qc_drv_ZB_Shutdown(qapi_ZB_Handle_t ZB_Handle)
{
    qapi_ZB_Shutdown(ZB_Handle);
    return QCLI_STATUS_SUCCESS_E;
}

static qapi_Status_t qc_drv_ZB_Form(qapi_ZB_Handle_t ZB_Handle, const qapi_ZB_NetworkConfig_t *Config)
{
    return qapi_ZB_Form(ZB_Handle, Config);
}

static qapi_Status_t qc_drv_ZB_Join(qapi_ZB_Handle_t ZB_Handle, const qapi_ZB_Join_t *Config)
{
    return qapi_ZB_Join(ZB_Handle, Config);
}

static qapi_Status_t qc_drv_ZB_Leave(qapi_ZB_Handle_t ZB_Handle)
{
    return qapi_ZB_Leave(ZB_Handle);
}

static qapi_Status_t qc_drv_ZB_ZDP_Mgmt_Leave_Req(qapi_ZB_Handle_t ZB_Handle, uint16_t DstNwkAddr, uint64_t DeviceAddress, qbool_t RemoveChildren, qbool_t Rejoin)
{
    return qapi_ZB_ZDP_Mgmt_Leave_Req(ZB_Handle, DstNwkAddr, DeviceAddress, RemoveChildren, Rejoin);
}

static qapi_Status_t qc_drv_ZB_Permit_Join(qapi_ZB_Handle_t ZB_Handle, uint8_t Duration)
{
    return qapi_ZB_Permit_Join(ZB_Handle, Duration);
}

static qapi_Status_t qc_drv_ZB_ZDP_Bind_Req(qapi_ZB_Handle_t ZB_Handle, const qapi_ZB_ZDP_Bind_Req_t *RequestData)
{
    return qapi_ZB_ZDP_Bind_Req(ZB_Handle, RequestData);
}

static qapi_Status_t qc_drv_ZB_ZDP_End_Device_Bind_Req(qapi_ZB_Handle_t ZB_Handle, const qapi_ZB_ZDP_End_Device_Bind_Req_t *RequestData)
{
    return qapi_ZB_ZDP_End_Device_Bind_Req(ZB_Handle, RequestData);
}

static qapi_Status_t qc_drv_ZB_APSME_Get_Request(qapi_ZB_Handle_t ZB_Handle, qapi_ZB_AIB_Attribute_ID_t AIBAttribute, uint8_t AIBAttributeIndex, uint16_t *AIBAttributeLength, void *AIBAttributeValue)
{
    return qapi_ZB_APSME_Get_Request(ZB_Handle, AIBAttribute, AIBAttributeIndex, AIBAttributeLength, (uint8_t *)AIBAttributeValue);
}

static qapi_Status_t qc_drv_ZB_APSME_Set_Request(qapi_ZB_Handle_t ZB_Handle, qapi_ZB_AIB_Attribute_ID_t AIBAttribute, uint8_t AIBAttributeIndex, uint16_t *AIBAttributeLength, const void *AIBAttributeValue)
{
    return qapi_ZB_APSME_Set_Request(ZB_Handle, AIBAttribute, AIBAttributeIndex, *AIBAttributeLength, AIBAttributeValue);
}

static qapi_Status_t qc_drv_ZB_Set_Extended_Address(qapi_ZB_Handle_t ZB_Handle, uint64_t Extended_Address)
{
    return qapi_ZB_Set_Extended_Address(ZB_Handle, Extended_Address);
}

static qapi_Status_t qc_drv_ZB_APS_Add_Endpoint(qapi_ZB_Handle_t ZB_Handle, qapi_ZB_APS_Add_Endpoint_t *RequestData)
{
    return qapi_ZB_APS_Add_Endpoint(ZB_Handle, RequestData);
}

static qapi_Status_t qc_drv_ZB_APS_Remove_Endpoint(qapi_ZB_Handle_t ZB_Handle, uint8_t Endpoint)
{
    return qapi_ZB_APS_Remove_Endpoint(ZB_Handle, Endpoint);
}

static qapi_Status_t qc_drv_ZB_CL_Read_Local_Attribute(qapi_ZB_Cluster_t Cluster, uint16_t AttrId, uint16_t *Length, uint8_t *Data)
{
    return qapi_ZB_CL_Read_Local_Attribute(Cluster, AttrId, Length, Data);
}

static qapi_Status_t qc_drv_ZB_CL_Write_Local_Attribute(qapi_ZB_Cluster_t Cluster, uint16_t AttrId, uint16_t Length, uint8_t *Data)
{
    return qapi_ZB_CL_Write_Local_Attribute(Cluster, AttrId, Length, Data);
}

static qapi_Status_t qc_drv_ZB_CL_Read_Attributes(qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, uint8_t AttrCount, const uint16_t *AttrIdList)
{
    return qapi_ZB_CL_Read_Attributes(Cluster, SendInfo, AttrCount, AttrIdList);
}

static qapi_Status_t qc_drv_ZB_CL_Write_Attributes(qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, const uint8_t AttrCount, const qapi_ZB_CL_Write_Attr_Record_t *AttrRecoredList)
{
    return qapi_ZB_CL_Write_Attributes(Cluster, SendInfo, AttrCount, AttrRecoredList);
}


static qapi_Status_t qc_drv_ZB_CL_Discover_Attributes(qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, const uint16_t StartAttrId, const uint8_t AttrIdCount)
{
    return qapi_ZB_CL_Discover_Attributes(Cluster, SendInfo, StartAttrId, AttrIdCount);
}

static qapi_Status_t qc_drv_ZB_CL_Configure_Reporting(qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, uint8_t ReportCount, const qapi_ZB_CL_Attr_Reporting_Config_Record_t *ReportRecordList)
{
    return qapi_ZB_CL_Configure_Reporting(Cluster, SendInfo, ReportCount, ReportRecordList);
}

static qapi_Status_t qc_drv_ZB_CL_Read_Reporting(qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, uint8_t ReportCount, const qapi_ZB_CL_Attr_Record_t *ReportConfigList)
{
    return qapi_ZB_CL_Read_Reporting_Config(Cluster, SendInfo, ReportCount, ReportConfigList);
}

static qapi_Status_t qc_drv_ZB_CL_OnOff_Send_On(qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, qbool_t RecallGlobalScene)
{
    return qapi_ZB_CL_OnOff_Send_On(Cluster, SendInfo, RecallGlobalScene);
}

static qapi_Status_t qc_drv_ZB_CL_OnOff_Send_Off(qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo)
{
    return qapi_ZB_CL_OnOff_Send_Off(Cluster, SendInfo);
}

static qapi_Status_t qc_drv_ZB_CL_OnOff_Send_Toggle(qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo)
{
    return qapi_ZB_CL_OnOff_Send_Toggle(Cluster, SendInfo);
}

static qapi_Status_t qc_drv_ZB_CL_LevelControl_Send_Move_To_Level(qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, uint8_t Level, uint16_t TransitionTime, qbool_t WithOnOff)
{
    return qapi_ZB_CL_LevelControl_Send_Move_To_Level(Cluster, SendInfo, Level, TransitionTime, WithOnOff);
}

static qapi_Status_t qc_drv_ZB_CL_LevelControl_Send_Move(qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, qbool_t MoveDown, uint8_t Rate, qbool_t WithOnOff)
{
    return qapi_ZB_CL_LevelControl_Send_Move(Cluster, SendInfo, MoveDown, Rate, WithOnOff);
}

static qapi_Status_t qc_drv_ZB_CL_LevelControl_Send_Step(qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, qbool_t MoveDown, uint8_t StepSize, uint16_t TransitionTime, qbool_t WithOnOff)
{
    return qapi_ZB_CL_LevelControl_Send_Step(Cluster, SendInfo, MoveDown, StepSize, TransitionTime, WithOnOff);
}

static qapi_Status_t qc_drv_ZB_CL_LevelControl_Send_Stop(qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo)
{
    return qapi_ZB_CL_LevelControl_Send_Stop(Cluster, SendInfo);
}

/*Start of Thread */
static qapi_Status_t qc_drv_TWN_Initialize(qapi_TWN_Handle_t *TWN_Handle, qapi_TWN_Event_CB_t TWN_Event_CB, uint32_t CB_Param)
{
    return qapi_TWN_Initialize(TWN_Handle, TWN_Event_CB, CB_Param);
}

static void qc_drv_TWN_Shutdown(qapi_TWN_Handle_t TWN_Handle)
{
    qapi_TWN_Shutdown(TWN_Handle);
}

static qapi_Status_t qc_drv_TWN_Start(qapi_TWN_Handle_t TWN_Handle)
{
    return qapi_TWN_Start(TWN_Handle);
}

static qapi_Status_t qc_drv_TWN_Stop(qapi_TWN_Handle_t TWN_Handle)
{
    return qapi_TWN_Stop(TWN_Handle);
}

static qapi_Status_t qc_drv_TWN_Get_Device_Configuration(qapi_TWN_Handle_t TWN_Handle, qapi_TWN_Device_Configuration_t *Configuration)
{
    return qapi_TWN_Get_Device_Configuration(TWN_Handle, Configuration);
}

static qapi_Status_t qc_drv_TWN_Set_Device_Configuration(qapi_TWN_Handle_t TWN_Handle, const qapi_TWN_Device_Configuration_t *Configuration)
{
    return qapi_TWN_Set_Device_Configuration(TWN_Handle, Configuration);
}

static qapi_Status_t qc_drv_TWN_Get_Network_Configuration(qapi_TWN_Handle_t TWN_Handle, qapi_TWN_Network_Configuration_t *Configuration)
{
    return qapi_TWN_Get_Network_Configuration(TWN_Handle, Configuration);
}

static qapi_Status_t qc_drv_TWN_Set_Network_Configuration(qapi_TWN_Handle_t TWN_Handle, const qapi_TWN_Network_Configuration_t *Configuration)
{
    return qapi_TWN_Set_Network_Configuration(TWN_Handle, Configuration);
}

static qapi_Status_t qc_drv_TWN_Add_Border_Router(qapi_TWN_Handle_t TWN_Handle, const qapi_TWN_Border_Router_t *Border_Router)
{
    return qapi_TWN_Add_Border_Router(TWN_Handle, Border_Router);
}

static qapi_Status_t qc_drv_TWN_Remove_Border_Router(qapi_TWN_Handle_t TWN_Handle, const qapi_TWN_IPv6_Prefix_t *Prefix)
{
    return qapi_TWN_Remove_Border_Router(TWN_Handle, Prefix);
}

static qapi_Status_t qc_drv_TWN_Add_External_Route(qapi_TWN_Handle_t TWN_Handle, const qapi_TWN_External_Route_t *External_Route)
{
    return qapi_TWN_Add_External_Route(TWN_Handle, External_Route);
}

static qapi_Status_t qc_drv_TWN_Remove_External_Route(qapi_TWN_Handle_t TWN_Handle, const qapi_TWN_IPv6_Prefix_t *Prefix)
{
    return qapi_TWN_Remove_External_Route(TWN_Handle, Prefix);
}

static qapi_Status_t qc_drv_TWN_Register_Server_Data(qapi_TWN_Handle_t TWN_Handle)
{
    return qapi_TWN_Register_Server_Data(TWN_Handle);
}

static qapi_Status_t qc_drv_TWN_Set_IP_Stack_Integration(qapi_TWN_Handle_t TWN_Handle, qbool_t Enabled)
{
    return qapi_TWN_Set_IP_Stack_Integration(TWN_Handle, Enabled);
}

static qapi_Status_t qc_drv_TWN_Commissioner_Start(qapi_TWN_Handle_t TWN_Handle)
{
    return qapi_TWN_Commissioner_Start(TWN_Handle);
}

static qapi_Status_t qc_drv_TWN_Commissioner_Stop(qapi_TWN_Handle_t TWN_Handle)
{
    return qapi_TWN_Commissioner_Stop(TWN_Handle);
}

static qapi_Status_t qc_drv_TWN_Commissioner_Add_Joiner(qapi_TWN_Handle_t TWN_Handle, uint64_t Extended_Address, const char *PSKd, uint32_t Timeout)
{
    return qapi_TWN_Commissioner_Add_Joiner(TWN_Handle, Extended_Address, PSKd, Timeout);
}

static qapi_Status_t qc_drv_TWN_Commissioner_Remove_Joiner(qapi_TWN_Handle_t TWN_Handle, uint64_t Extended_Address)
{
    return qapi_TWN_Commissioner_Remove_Joiner(TWN_Handle, Extended_Address);
}

static qapi_Status_t qc_drv_TWN_Commissioner_Set_Provisioning_URL(qapi_TWN_Handle_t TWN_Handle, const char *Provisioning_URL)
{
    return qapi_TWN_Commissioner_Set_Provisioning_URL(TWN_Handle, Provisioning_URL);
}

static qapi_Status_t qc_drv_TWN_Commissioner_Generate_PSKc(qapi_TWN_Handle_t TWN_Handle, const char *Passphrase, const char *Network_Name, uint64_t Extended_PAN_ID, uint8_t *PSKc)
{
    return qapi_TWN_Commissioner_Generate_PSKc(TWN_Handle, Passphrase, Network_Name, Extended_PAN_ID, PSKc);
}

static qapi_Status_t qc_drv_TWN_Joiner_Start(qapi_TWN_Handle_t TWN_Handle, const qapi_TWN_Joiner_Info_t *Joiner_Info)
{
    return qapi_TWN_Joiner_Start(TWN_Handle, Joiner_Info);
}

static qapi_Status_t qc_drv_TWN_Joiner_Stop(qapi_TWN_Handle_t TWN_Handle)
{
    return qapi_TWN_Joiner_Stop(TWN_Handle);
}

static qapi_Status_t qc_drv_TWN_Set_PSKc(qapi_TWN_Handle_t TWN_Handle, const uint8_t *PSKc)
{
    return qapi_TWN_Set_PSKc(TWN_Handle, PSKc);
}

static qapi_Status_t qc_drv_TWN_IPv6_Add_Unicast_Address(qapi_TWN_Handle_t TWN_Handle, qapi_TWN_IPv6_Prefix_t *Prefix, qbool_t Preferred)
{
    return qapi_TWN_IPv6_Add_Unicast_Address(TWN_Handle, Prefix, Preferred);
}

static qapi_Status_t qc_drv_TWN_IPv6_Remove_Unicast_Address(qapi_TWN_Handle_t TWN_Handle, qapi_TWN_IPv6_Address_t *Address)
{
    return qapi_TWN_IPv6_Remove_Unicast_Address(TWN_Handle, Address);
}

static qapi_Status_t qc_drv_TWN_IPv6_Subscribe_Multicast_Address(qapi_TWN_Handle_t TWN_Handle, qapi_TWN_IPv6_Address_t *Address)
{
    return qapi_TWN_IPv6_Subscribe_Multicast_Address(TWN_Handle, Address);
}

static qapi_Status_t qc_drv_TWN_IPv6_Unsubscribe_Multicast_Address(qapi_TWN_Handle_t TWN_Handle, qapi_TWN_IPv6_Address_t *Address)
{
    return qapi_TWN_IPv6_Unsubscribe_Multicast_Address(TWN_Handle, Address);
}

static qapi_Status_t qc_drv_TWN_Set_Ping_Response_Enabled(qapi_TWN_Handle_t TWN_Handle, qbool_t Enabled)
{
    return qapi_TWN_Set_Ping_Response_Enabled(TWN_Handle, Enabled);
}

static qapi_Status_t qc_drv_TWN_Become_Router(qapi_TWN_Handle_t TWN_Handle)
{
    return qapi_TWN_Become_Router(TWN_Handle);
}

static qapi_Status_t qc_drv_TWN_Become_Leader(qapi_TWN_Handle_t TWN_Handle)
{
    return qapi_TWN_Become_Leader(TWN_Handle);
}

static qapi_Status_t qc_drv_TWN_Start_Border_Agent(qapi_TWN_Handle_t TWN_Handle, int AddressFamily, const char *DisplayName, const char *Hostname, const char *Interface)
{
    return qapi_TWN_Start_Border_Agent(TWN_Handle, AddressFamily, DisplayName, Hostname, Interface);
}

static qapi_Status_t qc_drv_TWN_Stop_Border_Agent(qapi_TWN_Handle_t TWN_Handle)
{
    return qapi_TWN_Stop_Border_Agent(TWN_Handle);
}

static qapi_Status_t qc_drv_TWN_Clear_Persistent_Data(qapi_TWN_Handle_t TWN_Handle)
{
    return qapi_TWN_Clear_Persistent_Data(TWN_Handle);
}

static qapi_Status_t qc_drv_TWN_Set_Max_Poll_Period(qapi_TWN_Handle_t TWN_Handle, uint32_t Period)
{
    return qapi_TWN_Set_Max_Poll_Period(TWN_Handle, Period);
}

static qapi_Status_t qc_drv_TWN_Commissioner_Send_Mgmt_Get(qapi_TWN_Handle_t TWN_Handle, const uint8_t *TlvBuffer, uint8_t Length)
{
    return qapi_TWN_Commissioner_Send_Mgmt_Get(TWN_Handle, TlvBuffer, Length);
}

static qapi_Status_t qc_drv_TWN_Commissioner_Send_Mgmt_Set(qapi_TWN_Handle_t TWN_Handle, const qapi_TWN_Commissioning_Dataset_t *Dataset, const uint8_t *TlvBuffer, uint8_t Length)
{
    return qapi_TWN_Commissioner_Send_Mgmt_Set(TWN_Handle, Dataset, TlvBuffer, Length);
}

static qapi_Status_t qc_drv_TWN_Commissioner_Send_PanId_Query(qapi_TWN_Handle_t TWN_Handle, uint16_t PanId, uint32_t ChannelMask, const qapi_TWN_IPv6_Address_t *Address)
{
    return qapi_TWN_Commissioner_Send_PanId_Query(TWN_Handle, PanId, ChannelMask, Address);
}

static qapi_Status_t qc_drv_TWN_Commissioner_Get_Session_Id(qapi_TWN_Handle_t TWN_Handle, uint16_t *SessionId)
{
    return qapi_TWN_Commissioner_Get_Session_Id(TWN_Handle, SessionId);
}

static qapi_Status_t qc_drv_TWN_Commissioner_Send_Mgmt_Active_Get(qapi_TWN_Handle_t TWN_Handle, const qapi_TWN_IPv6_Address_t *Address, const uint8_t *TlvBuffer, uint8_t Length)
{
    return qapi_TWN_Commissioner_Send_Mgmt_Active_Get(TWN_Handle, Address, TlvBuffer, Length);
}

static qapi_Status_t qc_drv_TWN_Commissioner_Send_Mgmt_Active_Set(qapi_TWN_Handle_t TWN_Handle, const qapi_TWN_Operational_Dataset_t *Dataset, const uint8_t *TlvBuffer, uint8_t Length)
{
    return qapi_TWN_Commissioner_Send_Mgmt_Active_Set(TWN_Handle, Dataset, TlvBuffer, Length);
}

static qapi_Status_t qc_drv_ZB_BDB_Get_Request(qapi_ZB_Handle_t ZB_Handle, qapi_ZB_BDB_Attribute_ID_t AttributeId, uint8_t AttributeIndex, uint16_t *AttributeLength, uint8_t *AttributeValue)
{
    return qapi_ZB_BDB_Get_Request(ZB_Handle, AttributeId, AttributeIndex, AttributeLength, AttributeValue);
}

static qapi_Status_t qc_drv_ZB_BDB_Set_Request(qapi_ZB_Handle_t ZB_Handle, qapi_ZB_BDB_Attribute_ID_t AttributeId, uint8_t AttributeIndex, uint16_t AttributeLength, const uint8_t *AttributeValue)
{
    return qapi_ZB_BDB_Set_Request(ZB_Handle, AttributeId, AttributeIndex, AttributeLength, AttributeValue);
}

static qapi_Status_t qc_drv_ZB_ZDP_Match_Desc_Req(qapi_ZB_Handle_t ZB_Handle, uint16_t DstNwkAddr, const qapi_ZB_ZDP_Match_Desc_Req_t *RequestData)
{
    return qapi_ZB_ZDP_Match_Desc_Req(ZB_Handle, DstNwkAddr, RequestData);
}

static qapi_Status_t qc_drv_ZB_CL_Basic_Send_Reset_To_Factory(qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo)
{
    return qapi_ZB_CL_Basic_Send_Reset_To_Factory(Cluster, SendInfo);
}

static qapi_Status_t qc_drv_ZB_CL_Basic_Server_Read_Attribute(qapi_ZB_Handle_t ZB_Handle, uint16_t AttrId, uint16_t *Length, uint8_t *Data)
{
    return qapi_ZB_CL_Basic_Server_Read_Attribute(ZB_Handle, AttrId, Length, Data);
}

static qapi_Status_t qc_drv_ZB_CL_Basic_Server_Write_Attribute(qapi_ZB_Handle_t ZB_Handle, uint16_t AttrId, uint16_t Length, const uint8_t *Data){
    return qapi_ZB_CL_Basic_Server_Write_Attribute(ZB_Handle, AttrId, Length, Data);
}

static qapi_Status_t qc_drv_ZB_CL_Identify_Send_Identify(qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, uint16_t IdentifyTime)
{
    return qapi_ZB_CL_Identify_Send_Identify(Cluster, SendInfo, IdentifyTime);

}

static qapi_Status_t qc_drv_ZB_CL_Identify_Send_Identify_Query(qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo)
{
    return qapi_ZB_CL_Identify_Send_Identify_Query(Cluster, SendInfo);
}

static qapi_Status_t qc_drv_ZB_CL_Groups_Send_Add_Group(qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, uint16_t GroupId, const char *GroupName, qbool_t Identifying)
{
    return qapi_ZB_CL_Groups_Send_Add_Group(Cluster, SendInfo, GroupId, GroupName, Identifying);
}

static qapi_Status_t qc_drv_ZB_CL_Groups_Send_View_Group(qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, uint16_t GroupId)
{
    return qapi_ZB_CL_Groups_Send_View_Group(Cluster, SendInfo, GroupId);
}

static qapi_Status_t qc_drv_ZB_CL_Groups_Send_Remove_Group(qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, uint16_t GroupId)
{
    return qapi_ZB_CL_Groups_Send_Remove_Group(Cluster, SendInfo, GroupId);
}

static qapi_Status_t qc_drv_ZB_CL_Groups_Send_Get_Group_Membership(qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, uint8_t GroupCount, const uint16_t *GroupList)
{
    return qapi_ZB_CL_Groups_Send_Get_Group_Membership(Cluster, SendInfo, GroupCount, GroupList);
}

static qapi_Status_t qc_drv_ZB_CL_Groups_Send_Remove_All_Groups(qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo)
{
    return qapi_ZB_CL_Groups_Send_Remove_All_Groups(Cluster, SendInfo);
}

static qapi_Status_t qc_drv_ZB_CL_Scenes_Send_Add_Scene(qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, const qapi_ZB_CL_Scenes_Add_Scene_t *AddScene)
{
    return qapi_ZB_CL_Scenes_Send_Add_Scene(Cluster, SendInfo, AddScene);
}

static qapi_Status_t qc_drv_ZB_CL_Scenes_Send_View_Scene(qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, uint16_t GroupId, uint8_t SceneId, qbool_t IsEnhanced)
{
    return qapi_ZB_CL_Scenes_Send_View_Scene(Cluster, SendInfo, GroupId, SceneId, IsEnhanced);
}

static qapi_Status_t qc_drv_ZB_CL_Scenes_Send_Remove_Scene(qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, uint16_t GroupId, uint8_t SceneId)
{
    return qapi_ZB_CL_Scenes_Send_Remove_Scene(Cluster, SendInfo, GroupId, SceneId);
}

static qapi_Status_t qc_drv_ZB_CL_Scenes_Send_Remove_All_Scenes(qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, uint16_t GroupId)
{
    return qapi_ZB_CL_Scenes_Send_Remove_All_Scenes(Cluster, SendInfo, GroupId);
}

static qapi_Status_t qc_drv_ZB_CL_Scenes_Send_Store_Scene(qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, uint16_t GroupId, uint8_t SceneId)
{
    return qapi_ZB_CL_Scenes_Send_Store_Scene(Cluster, SendInfo, GroupId, SceneId);
}

static qapi_Status_t qc_drv_ZB_CL_Scenes_Send_Recall_Scene(qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, uint16_t GroupId, uint8_t SceneId)
{
    return qapi_ZB_CL_Scenes_Send_Recall_Scene(Cluster, SendInfo, GroupId, SceneId);
}

static qapi_Status_t qc_drv_ZB_CL_Scenes_Send_Get_Scene_Membership(qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, uint16_t GroupId)
{
    return qapi_ZB_CL_Scenes_Send_Get_Scene_Membership(Cluster, SendInfo, GroupId);
}

static qapi_Status_t  qc_drv_ZB_CL_Scenes_Send_Copy_Scene(qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, const qapi_ZB_CL_Scenes_Copy_Scene_t *CopyScene)
{
    return qapi_ZB_CL_Scenes_Send_Copy_Scene(Cluster, SendInfo, CopyScene);
}

static qapi_Status_t qc_drv_ZB_CL_Time_Server_Read_Attribute(qapi_ZB_Handle_t ZB_Handle, uint16_t AttrId, uint16_t *Length, uint8_t *Data)
{
    return qapi_ZB_CL_Time_Server_Read_Attribute(ZB_Handle, AttrId, Length, Data);
}

static qapi_Status_t qc_drv_ZB_CL_Time_Server_Write_Attribute(qapi_ZB_Handle_t ZB_Handle, uint16_t AttrId, uint16_t Length, const uint8_t *Data)
{
    return qapi_ZB_CL_Time_Server_Write_Attribute(ZB_Handle, AttrId, Length, Data);
}

static qapi_Status_t qc_drv_ZB_CL_Alarm_Send_Reset_Alarm(qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, uint16_t SourceClusterId, uint8_t AlarmCode)
{
    return qapi_ZB_CL_Alarm_Send_Reset_Alarm(Cluster, SendInfo, SourceClusterId, AlarmCode);
}

static qapi_Status_t qc_drv_ZB_CL_Alarm_Send_Reset_All_Alarms(qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo)
{
    return qapi_ZB_CL_Alarm_Send_Reset_All_Alarms(Cluster, SendInfo);
}

static qapi_Status_t qc_drv_ZB_CL_Alarm_Send_Get_Alarm(qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo)
{
    return qapi_ZB_CL_Alarm_Send_Get_Alarm(Cluster, SendInfo);
}

static qapi_Status_t qc_drv_ZB_CL_Alarm_Send_Reset_Alarm_Log(qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo)
{
    return qapi_ZB_CL_Alarm_Send_Reset_Alarm_Log(Cluster, SendInfo);
}

static qapi_Status_t qc_drv_ZB_CL_Alarm_Send_Alarm(qapi_ZB_Cluster_t Cluster, qapi_ZB_Cluster_t SourceCluster, uint8_t AlarmCode)
{
    return qapi_ZB_CL_Alarm_Send_Alarm(Cluster, SourceCluster, AlarmCode);
}

static qapi_Status_t qc_drv_ZB_CL_ColorControl_Send_Move_To_Hue(qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, uint16_t Hue, qapi_ZB_CL_ColorControl_Move_Mode_t Direction, uint16_t TransitionTime, qbool_t IsEnhanced)
{
    return qapi_ZB_CL_ColorControl_Send_Move_To_Hue(Cluster, SendInfo, Hue, Direction, TransitionTime, IsEnhanced);
}

static qapi_Status_t qc_drv_ZB_CL_ColorControl_Send_Move_Hue(qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, qapi_ZB_CL_ColorControl_Move_Mode_t MoveMode, uint16_t Rate, qbool_t IsEnhanced)
{
    return qapi_ZB_CL_ColorControl_Send_Move_Hue(Cluster, SendInfo, MoveMode, Rate, IsEnhanced);
}

static qapi_Status_t qc_drv_ZB_CL_ColorControl_Send_Step_Hue(qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, qapi_ZB_CL_ColorControl_Move_Mode_t StepMode, uint16_t StepSize, uint16_t TransitionTime, qbool_t IsEnhanced)
{
    return qapi_ZB_CL_ColorControl_Send_Step_Hue(Cluster, SendInfo, StepMode, StepSize, TransitionTime, IsEnhanced);
}

static qapi_Status_t qc_drv_ZB_CL_ColorControl_Send_Move_To_Saturation(qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, uint8_t Saturation, uint16_t TransitionTime)
{
    return qapi_ZB_CL_ColorControl_Send_Move_To_Saturation(Cluster, SendInfo, Saturation, TransitionTime);
}

static qapi_Status_t qc_drv_ZB_CL_ColorControl_Send_Move_Saturation(qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, qapi_ZB_CL_ColorControl_Move_Mode_t MoveMode, uint8_t Rate)
{
    return qapi_ZB_CL_ColorControl_Send_Move_Saturation(Cluster, SendInfo, MoveMode, Rate);
}

static qapi_Status_t qc_drv_ZB_CL_ColorControl_Send_Step_Saturation(qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, qapi_ZB_CL_ColorControl_Move_Mode_t StepMode, uint8_t StepSize, uint8_t TransitionTime)
{
    return qapi_ZB_CL_ColorControl_Send_Step_Saturation(Cluster, SendInfo, StepMode, StepSize, TransitionTime);
}

static qapi_Status_t qc_drv_ZB_CL_ColorControl_Send_Move_To_HueAndSaturation(qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, uint16_t Hue, uint8_t Saturation, uint16_t TransitionTime, qbool_t IsEnhanced)
{
    return qapi_ZB_CL_ColorControl_Send_Move_To_HueAndSaturation(Cluster, SendInfo, Hue, Saturation, TransitionTime, IsEnhanced);
}

static qapi_Status_t qc_drv_ZB_CL_ColorControl_Send_Move_To_Color(qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, uint16_t ColorX, uint16_t ColorY, uint16_t TransitionTime)
{
    return qapi_ZB_CL_ColorControl_Send_Move_To_Color(Cluster, SendInfo, ColorX, ColorY, TransitionTime);
}

static qapi_Status_t qc_drv_ZB_CL_ColorControl_Send_Move_Color(qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, int16_t RateX, int16_t RateY)
{
    return qapi_ZB_CL_ColorControl_Send_Move_Color(Cluster, SendInfo, RateX, RateY);
}

static qapi_Status_t qc_drv_ZB_CL_ColorControl_Send_Step_Color(qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, int16_t StepX, int16_t StepY, uint16_t TransitionTime)
{
    return qapi_ZB_CL_ColorControl_Send_Step_Color(Cluster, SendInfo, StepX, StepY, TransitionTime);
}

static qapi_Status_t qc_drv_ZB_CL_ColorControl_Send_Move_To_Color_Temp(qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, uint16_t ColorTempMireds, uint16_t TransitionTime)
{
    return qapi_ZB_CL_ColorControl_Send_Move_To_Color_Temp(Cluster, SendInfo, ColorTempMireds, TransitionTime);
}

static qapi_Status_t qc_drv_ZB_CL_ColorControl_Send_Move_Color_Temp(qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, qapi_ZB_CL_ColorControl_Move_Mode_t MoveMode, uint16_t Rate, uint16_t Limit)
{
    return qapi_ZB_CL_ColorControl_Send_Move_Color_Temp(Cluster, SendInfo, MoveMode, Rate, Limit);
}

static qapi_Status_t qc_drv_ZB_CL_ColorControl_Send_Step_Color_Temp(qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, const qapi_ZB_CL_ColorControl_Step_Color_Temp_t *StepColorTemp)
{
    return qapi_ZB_CL_ColorControl_Send_Step_Color_Temp(Cluster, SendInfo, StepColorTemp);
}

static qapi_Status_t qc_drv_ZB_CL_ColorControl_Send_Stop_Move_Step(qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo)
{
    return qapi_ZB_CL_ColorControl_Send_Stop_Move_Step(Cluster, SendInfo);
}

static qapi_Status_t qc_drv_ZB_CL_ColorControl_Send_Color_Loop_Set(qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, const qapi_ZB_CL_ColorControl_Color_Loop_Set_t *ColorLoopSet)
{
    return qapi_ZB_CL_ColorControl_Send_Color_Loop_Set(Cluster, SendInfo, ColorLoopSet);
}

static qapi_Status_t qc_drv_ZB_Get_Persistent_Data(qapi_ZB_Handle_t ZB_Handle, uint8_t *Buffer, uint32_t *Length)
{
    return qapi_ZB_Get_Persistent_Data(ZB_Handle, Buffer, Length);
}

static qapi_Status_t qc_drv_ZB_Restore_Persistent_Data(qapi_ZB_Handle_t ZB_Handle, const uint8_t *Buffer, uint32_t Length)
{
    return qapi_ZB_Restore_Persistent_Data(ZB_Handle, Buffer, Length);
}

static qapi_Status_t qc_drv_ZB_NLME_Get_Request(qapi_ZB_Handle_t ZB_Handle, qapi_ZB_NIB_Attribute_ID_t AttributeId, uint8_t AttributeIndex, uint16_t *AttributeLength, uint8_t *AttributeValue)
{
    return qapi_ZB_NLME_Get_Request(ZB_Handle, AttributeId, AttributeIndex, AttributeLength, AttributeValue);
}

static qapi_Status_t qc_drv_ZB_NLME_Set_Request(qapi_ZB_Handle_t ZB_Handle, qapi_ZB_NIB_Attribute_ID_t AttributeId, uint8_t AttributeIndex, uint16_t AttributeLength, const uint8_t *AttributeValue)
{
    return qapi_ZB_NLME_Set_Request(ZB_Handle, AttributeId, AttributeIndex, AttributeLength, AttributeValue);
}

static qapi_Status_t qc_drv_ZB_CL_Touchlink_Start(qapi_ZB_Cluster_t Cluster, qapi_ZB_CL_Touchlink_Device_Type_t DeviceType, const uint8_t *PersistData, uint32_t PersistLength)
{
    return qapi_ZB_CL_Touchlink_Start(Cluster, DeviceType, PersistData, PersistLength);
}

static qapi_Status_t qc_drv_ZB_CL_Touchlink_Scan_Request(qapi_ZB_Cluster_t ClientCluster)
{
    return qapi_ZB_CL_Touchlink_Scan_Request(ClientCluster);
}

static qapi_Status_t qc_drv_ZB_CL_Touchlink_Factory_Reset(qapi_ZB_Cluster_t ClientCluster)
{
    return qapi_ZB_CL_Touchlink_Factory_Reset(ClientCluster);
}

static qapi_Status_t qc_drv_ZB_CL_Destroy_Cluster(qapi_ZB_Cluster_t Cluster)
{
    return qapi_ZB_CL_Destroy_Cluster(Cluster);
}

static qapi_Status_t qc_drv_Persist_Initialize(qapi_Persist_Handle_t *Handle, char *Directory, char *NamePrefix, char *NameSuffix, uint8_t *Password, uint32_t PasswordSize)
{
    return qapi_Persist_Initialize(Handle, Directory, NamePrefix, NameSuffix, Password, PasswordSize);
}

static void qc_drv_Persist_Delete(qapi_Persist_Handle_t Handle)
{
     qapi_Persist_Delete(Handle);
}

static void qc_drv_Persist_Cleanup(qapi_Persist_Handle_t Handle)
{
     qapi_Persist_Cleanup(Handle);
}

static qc_drv_cb_t qc_drv_ops = {
    .name = "driver_ops",
    .desc = "platform 402X driver ops",
#ifndef CONFIG_PLATFORM_CDB24
    /** Start of wifi */
    .qc_drv_wlan_enable = qc_drv_wlan_enable,
    .qc_drv_wlan_disable = qc_drv_wlan_disable,

    .qc_drv_wlan_set_encr_type = qc_drv_wlan_set_encr_type,
    .qc_drv_wlan_set_auth_mode = qc_drv_wlan_set_auth_mode,
    .qc_drv_wlan_set_op_mode = qc_drv_wlan_set_op_mode,
    .qc_drv_wlan_set_mac_addr = qc_drv_wlan_set_mac_addr,
    .qc_drv_wlan_set_ssid = qc_drv_wlan_set_ssid,
    .qc_drv_wlan_set_passphrase = qc_drv_wlan_set_passphrase,
    .qc_drv_wlan_set_security_pmk = qc_drv_wlan_set_security_pmk,
    .qc_drv_wlan_set_wps_credentials = qc_drv_wlan_set_wps_credentials,
    .qc_drv_wlan_set_channel = qc_drv_wlan_set_channel,
    .qc_drv_wlan_set_listen_interval = qc_drv_wlan_set_listen_interval,
    .qc_drv_wlan_set_sta_keep_alive = qc_drv_wlan_set_sta_keep_alive,
    .qc_drv_wlan_set_rssi_threshold = qc_drv_wlan_set_rssi_threshold,
    .qc_drv_wlan_set_probe_req_fwd_to_host = qc_drv_wlan_set_probe_req_fwd_to_host,
    .qc_drv_wlan_set_wep_key_index = qc_drv_wlan_set_wep_key_index,
    .qc_drv_wlan_set_wep_key_pair = qc_drv_wlan_set_wep_key_pair,
    .qc_drv_wlan_set_rate = qc_drv_wlan_set_rate,
    .qc_drv_wlan_set_ap_wps_flag = qc_drv_wlan_set_ap_wps_flag,
    .qc_drv_wlan_set_tx_power_in_dbm = qc_drv_wlan_set_tx_power_in_dbm,
    .qc_drv_wlan_set_power_param = qc_drv_wlan_set_power_param,
    .qc_drv_wlan_set_phy_mode = qc_drv_wlan_set_phy_mode,
    .qc_drv_wlan_set_ht_config = qc_drv_wlan_set_ht_config,
    .qc_drv_wlan_set_ap_enable_hidden_mode = qc_drv_wlan_set_ap_enable_hidden_mode,
    .qc_drv_wlan_set_beacon_interval_in_tu = qc_drv_wlan_set_beacon_interval_in_tu,
    .qc_drv_wlan_set_ap_dtim_interval = qc_drv_wlan_set_ap_dtim_interval,
    .qc_drv_wlan_set_country_code = qc_drv_wlan_set_country_code,
    .qc_drv_wlan_set_ap_country_code = qc_drv_wlan_set_ap_country_code,

    .qc_drv_wlan_get_mac_addr = qc_drv_wlan_get_mac_addr,
    .qc_drv_wlan_get_op_mode = qc_drv_wlan_get_op_mode,
    .qc_drv_wlan_get_power_param = qc_drv_wlan_get_power_param,
    .qc_drv_wlan_add_device = qc_drv_wlan_add_device,
    .qc_drv_wlan_remove_device = qc_drv_wlan_remove_device,
    .qc_drv_wlan_start_scan = qc_drv_wlan_start_scan,
    .qc_drv_wlan_get_scan_results = qc_drv_wlan_get_scan_results,
    .qc_drv_wlan_get_phy_mode = qc_drv_wlan_get_phy_mode,
    .qc_drv_wlan_get_ht_config = qc_drv_wlan_get_ht_config,
    .qc_drv_wlan_get_ssid = qc_drv_wlan_get_ssid,
    .qc_drv_wlan_get_channel = qc_drv_wlan_get_channel,
    .qc_drv_wlan_get_wep_key_pair = qc_drv_wlan_get_wep_key_pair,
    .qc_drv_wlan_get_wep_key_index = qc_drv_wlan_get_wep_key_index,
    .qc_drv_wlan_get_rssi = qc_drv_wlan_get_rssi,
    .qc_drv_wlan_get_reg_domain_channel_list = qc_drv_wlan_get_reg_domain_channel_list,
    .qc_drv_wlan_get_wireless_stats = qc_drv_wlan_get_wireless_stats,
    .qc_drv_wlan_get_rate = qc_drv_wlan_get_rate,
    .qc_drv_wlan_get_firmware_version = qc_drv_wlan_get_firmware_version,
    .qc_drv_wlan_get_reg_domain = qc_drv_wlan_get_reg_domain,
    .qc_drv_wlan_get_last_error = qc_drv_wlan_get_last_error,
    .qc_drv_wlan_get_tx_status = qc_drv_wlan_get_tx_status,

    .qc_drv_wlan_set_callback = qc_drv_wlan_set_callback,
    .qc_drv_wlan_wps_connect = qc_drv_wlan_wps_connect,
    .qc_drv_wlan_wps_await_completion = qc_drv_wlan_wps_await_completion,
    .qc_drv_wlan_disconnect = qc_drv_wlan_disconnect,
    .qc_drv_wlan_get_scan_results = qc_drv_wlan_get_scan_results,
    .qc_drv_wlan_commit = qc_drv_wlan_commit,
    .qc_drv_wlan_raw_send = qc_drv_wlan_raw_send,
    .qc_drv_wlan_suspend_start = qc_drv_wlan_suspend_start,
    .qc_drv_wlan_wps_start = qc_drv_wlan_wps_start,
    .qc_drv_wlan_set_get_auth_mode = qc_drv_wlan_set_get_auth_mode,
    .qc_drv_wlan_promiscuous_mode = qc_drv_wlan_promiscuous_mode,
    .qc_drv_wlan_force_assert = qc_drv_wlan_force_assert,
    .qc_drv_wlan_scan_param = qc_drv_wlan_scan_param,
    .qc_drv_wlan_set_inactivity_time = qc_drv_wlan_set_inactivity_time,
    .qc_drv_wlan_set_ap_ps_buf = qc_drv_wlan_set_ap_ps_buf,
    .qc_drv_wlan_set_ap_enable_uapsd = qc_drv_wlan_set_ap_enable_uapsd,
    .qc_drv_wlan_set_sta_uapsd = qc_drv_wlan_set_sta_uapsd,
    .qc_drv_wlan_set_sta_max_sp_len = qc_drv_wlan_set_sta_max_sp_len,
    .qc_drv_wlan_set_wmm_config = qc_drv_wlan_set_wmm_config,
    .qc_drv_wlan_set_ap_bss_mip = qc_drv_wlan_set_ap_bss_mip,
    .qc_drv_wlan_set_sta_sp =  qc_drv_wlan_set_sta_sp,
    .qc_drv_wlan_set_ap_sr = qc_drv_wlan_set_ap_sr,
    .qc_drv_wlan_set_app_ie = qc_drv_wlan_set_app_ie,
    .qc_drv_wlan_set_sta_bmiss_config = qc_drv_wlan_set_sta_bmiss_config,
    .qc_drv_wlan_set_suspend_resume = qc_drv_wlan_set_suspend_resume,
    .qc_drv_wlan_set_ka_offload_scfg = qc_drv_wlan_set_ka_offload_scfg,
    .qc_drv_wlan_set_ka_offload_enable = qc_drv_wlan_set_ka_offload_enable,
    .qc_drv_wlan_set_nw_offload_enable = qc_drv_wlan_set_nw_offload_enable,
    .qc_drv_wlan_set_nw_profile = qc_drv_wlan_set_nw_profile,
    .qc_drv_wlan_set_aggr_tid = qc_drv_wlan_set_aggr_tid,
    .qc_drv_wlan_set_aggrx_config = qc_drv_wlan_set_aggrx_config,
    .qc_drv_wlan_set_pktlog_enable = qc_drv_wlan_set_pktlog_enable,
    .qc_drv_wlan_set_pktlog_start = qc_drv_wlan_set_pktlog_start,
    .qc_drv_wlan_set_dblog_enable = qc_drv_wlan_set_dblog_enable,
    .qc_drv_wlan_set_dblog_config = qc_drv_wlan_set_dblog_config,
    .qc_drv_wlan_set_dblog_module_conf = qc_drv_wlan_set_dblog_module_conf,
    .qc_drv_wlan_set_drv_reg_query = qc_drv_wlan_set_drv_reg_query,
    .qc_drv_wlan_set_ap_channel_switch = qc_drv_wlan_set_ap_channel_switch,
    .qc_drv_wlan_set_event_filter = qc_drv_wlan_set_event_filter,
    .qc_drv_wlan_set_pw_mode_policy = qc_drv_wlan_set_pw_mode_policy,
    .qc_drv_wlan_set_enable_roaming = qc_drv_wlan_set_enable_roaming,
    .qc_drv_wlan_set_enable_green_tx = qc_drv_wlan_set_enable_green_tx,
    .qc_drv_wlan_set_low_pw_listen = qc_drv_wlan_set_low_pw_listen,
    .qc_drv_wlan_set_enable_wake_wireless = qc_drv_wlan_set_enable_wake_wireless,
    .qc_drv_wlan_set_enable_pkt_filter = qc_drv_wlan_set_enable_pkt_filter,
    .qc_drv_wlan_set_add_pattern = qc_drv_wlan_set_add_pattern,
    .qc_drv_wlan_set_delete_pattern = qc_drv_wlan_set_delete_pattern,
    .qc_drv_wlan_set_change_def_filter_action = qc_drv_wlan_set_change_def_filter_action,

    /** Start of p2p */
    .qc_drv_p2p_enable = qc_drv_p2p_enable,
    .qc_drv_p2p_disable = qc_drv_p2p_disable,
    .qc_drv_p2p_set_config = qc_drv_p2p_set_config,
    .qc_drv_p2p_connect = qc_drv_p2p_connect,
    .qc_drv_p2p_provision = qc_drv_p2p_provision,
    .qc_drv_p2p_listen = qc_drv_p2p_listen,
    .qc_drv_p2p_cancel = qc_drv_p2p_cancel,
    .qc_drv_p2p_set_param = qc_drv_p2p_set_param,
    .qc_drv_p2p_join = qc_drv_p2p_join,
    .qc_drv_p2p_auth = qc_drv_p2p_auth,
    .qc_drv_p2p_start_go = qc_drv_p2p_start_go,
    .qc_drv_p2p_invite = qc_drv_p2p_invite,
    .qc_drv_p2p_get_node_list = qc_drv_p2p_get_node_list,
    .qc_drv_p2p_get_network_list = qc_drv_p2p_get_network_list,
    .qc_drv_p2p_set_oops_params = qc_drv_p2p_set_oops_params,
    .qc_drv_p2p_set_noa_params = qc_drv_p2p_set_noa_params,
    .qc_drv_p2p_set_operating_class = qc_drv_p2p_set_operating_class,
    .qc_drv_p2p_stop_find = qc_drv_p2p_stop_find,
    .qc_drv_p2p_set_ssidparam = qc_drv_p2p_set_ssidparam,
    .qc_drv_p2p_set = qc_drv_p2p_set,
    .qc_drv_p2p_find = qc_drv_p2p_find,
    .qc_drv_p2p_invite_auth = qc_drv_p2p_invite_auth,


    /** Start of Mqtt */
    .qc_drv_net_mqtt_init = qc_drv_net_mqtt_init,
    .qc_drv_net_mqtt_shutdown = qc_drv_net_mqtt_shutdown,
    .qc_drv_net_mqtt_new = qc_drv_net_mqtt_new,
    .qc_drv_net_mqtt_destroy = qc_drv_net_mqtt_destroy,
    .qc_drv_net_mqtt_set_username_password = qc_drv_net_mqtt_set_username_password,
    .qc_drv_net_mqtt_set_will = qc_drv_net_mqtt_set_will,
    .qc_drv_net_mqtt_set_keep_alive = qc_drv_net_mqtt_set_keep_alive,
    .qc_drv_net_mqtt_set_connack_wait_time = qc_drv_net_mqtt_set_connack_wait_time,
    .qc_drv_net_mqtt_set_ssl_config = qc_drv_net_mqtt_set_ssl_config,
    .qc_drv_net_mqtt_connect = qc_drv_net_mqtt_connect,
    .qc_drv_net_mqtt_subscribe = qc_drv_net_mqtt_subscribe,
    .qc_drv_net_mqtt_publish = qc_drv_net_mqtt_publish,
    .qc_drv_net_mqtt_unsubscribe = qc_drv_net_mqtt_unsubscribe,
    .qc_drv_net_mqtt_disconnect = qc_drv_net_mqtt_disconnect,

    /* Start of OTA */
    .qc_drv_Fw_Upgrade_Get_Image_ID = qc_drv_Fw_Upgrade_Get_Image_ID,
    .qc_drv_Fw_Upgrade_Get_Image_Version = qc_drv_Fw_Upgrade_Get_Image_Version,
    .qc_drv_Fw_Upgrade_Get_Partition_Start = qc_drv_Fw_Upgrade_Get_Partition_Start,
    .qc_drv_Fw_Upgrade_Get_Partition_Size = qc_drv_Fw_Upgrade_Get_Partition_Size,
    .qc_drv_Fw_Upgrade_Read_Partition = qc_drv_Fw_Upgrade_Read_Partition,
    .qc_drv_Fw_Upgrade_init = qc_drv_Fw_Upgrade_init,
    .qc_drv_Fw_Upgrade_Get_Active_FWD = qc_drv_Fw_Upgrade_Get_Active_FWD,
    .qc_drv_Fw_Upgrade_Get_FWD_Magic = qc_drv_Fw_Upgrade_Get_FWD_Magic,
    .qc_drv_Fw_Upgrade_Get_FWD_Rank = qc_drv_Fw_Upgrade_Get_FWD_Rank,
    .qc_drv_Fw_Upgrade_Get_FWD_Version = qc_drv_Fw_Upgrade_Get_FWD_Version,
    .qc_drv_Fw_Upgrade_Get_FWD_Status = qc_drv_Fw_Upgrade_Get_FWD_Status,
    .qc_drv_Fw_Upgrade_Get_FWD_Total_Images = qc_drv_Fw_Upgrade_Get_FWD_Total_Images,
    .qc_drv_Fw_Upgrade_First_Partition = qc_drv_Fw_Upgrade_First_Partition,
    .qc_drv_Fw_Upgrade_Next_Partition = qc_drv_Fw_Upgrade_Next_Partition,
    .qc_drv_Fw_Upgrade_Close_Partition = qc_drv_Fw_Upgrade_Close_Partition,
    .qc_drv_Fw_Upgrade_Erase_FWD = qc_drv_Fw_Upgrade_Erase_FWD,
    .qc_drv_Fw_Upgrade_Done = qc_drv_Fw_Upgrade_Done,
    .qc_drv_Fw_Upgrade_Find_Partition = qc_drv_Fw_Upgrade_Find_Partition,
    .qc_drv_Fw_Upgrade = qc_drv_Fw_Upgrade,
#endif
	/* Start of Net */
	.qc_drv_net_errno = qc_drv_net_errno,
	.qc_drv_net_DNSc_Reshost = qc_drv_net_DNSc_Reshost,
	.qc_drv_net_Ping = qc_drv_net_Ping,
	.qc_drv_net_Get_All_Ifnames = qc_drv_net_Get_All_Ifnames,
	.qc_drv_net_Interface_Exist = qc_drv_net_Interface_Exist,
	.qc_drv_net_IPv4_Config = qc_drv_net_IPv4_Config,
	.qc_drv_net_DHCPv4c_Register_Success_Callback= qc_drv_net_DHCPv4c_Register_Success_Callback,
	.qc_drv_net_DHCPv4c_Release = qc_drv_net_DHCPv4c_Release,
	.qc_drv_net_Ping6 = qc_drv_net_Ping6,
	.qc_drv_net_DHCPv4c_Register_Success_Callback = qc_drv_net_DHCPv4c_Register_Success_Callback,
	.qc_drv_net_DHCPv4s_Set_Pool = qc_drv_net_DHCPv4s_Set_Pool,

	.qc_drv_net_DHCPv6c_Register_New_IPv6_Lease_Complete_Callback = qc_drv_net_DHCPv6c_Register_New_IPv6_Lease_Complete_Callback,
	.qc_drv_net_DHCPv6c_Register_Release_IPv6_Lease_Complete_Callback = qc_drv_net_DHCPv6c_Register_Release_IPv6_Lease_Complete_Callback,
	.qc_drv_net_DHCPv6c_Enable  = qc_drv_net_DHCPv6c_Enable,
	.qc_drv_net_DHCPv6c_Disable = qc_drv_net_DHCPv6c_Disable,
	.qc_drv_net_DHCPv6c_Register_New_Prefix_Lease_Complete_Callback = qc_drv_net_DHCPv6c_Register_New_Prefix_Lease_Complete_Callback,
	.qc_drv_net_DHCPv6c_Register_Release_Prefix_Lease_Complete_Callback = qc_drv_net_DHCPv6c_Register_Release_Prefix_Lease_Complete_Callback,
	.qc_drv_net_DHCPv6c_New_Lease = qc_drv_net_DHCPv6c_New_Lease,
	.qc_drv_net_DHCPv6c_Release_Lease = qc_drv_net_DHCPv6c_Release_Lease,
	.qc_drv_net_DHCPv6c_Confirm_Lease = qc_drv_net_DHCPv6c_Confirm_Lease,

	.qc_drv_net_SNTPc_start = qc_drv_net_SNTPc_start,
	.qc_drv_net_SNTPc_get_srvr_list = qc_drv_net_SNTPc_get_srvr_list,
	.qc_drv_net_SNTPc_del_srvr = qc_drv_net_SNTPc_del_srvr,
	.qc_drv_net_SNTPc_cmd = qc_drv_net_SNTPc_cmd,
	.qc_drv_net_SNTPc_get_broke_down_time = qc_drv_net_SNTPc_get_broke_down_time,
	.qc_drv_net_SNTPc_add_srvr = qc_drv_net_SNTPc_add_srvr,

	.qc_drv_net_DNSc_get_srvr_list = qc_drv_net_DNSc_get_srvr_list,
	.qc_drv_net_DNSc_start = qc_drv_net_DNSc_start,
	.qc_drv_net_DNSc_add_srvr = qc_drv_net_DNSc_add_srvr,
	.qc_drv_net_DNSc_del_srvr = qc_drv_net_DNSc_del_srvr,
	.qc_drv_net_DNSc_cmd = qc_drv_net_DNSc_cmd,
	.qc_drv_net_HTTPs_Get_Status = qc_drv_net_HTTPs_Get_Status,
	.qc_drv_net_DHCPv4s_Register_Success_Callback = qc_drv_net_DHCPv4s_Register_Success_Callback,
	.qc_api_get_HTTPs_Is_Started = qc_api_get_HTTPs_Is_Started,
	.qc_drv_net_HTTPs_Init = qc_drv_net_HTTPs_Init,
	.qc_drv_net_HTTPs_Set_SSL_Config = qc_drv_net_HTTPs_Set_SSL_Config,
	.qc_drv_net_HTTPs_Start = qc_drv_net_HTTPs_Start,
	.qc_drv_net_HTTPs_Stop = qc_drv_net_HTTPs_Stop,
	.qc_drv_net_HTTPs_Shutdown = qc_drv_net_HTTPs_Shutdown,
	.qc_drv_net_HTTPs_Register_Content_Type = qc_drv_net_HTTPs_Register_Content_Type,
	.qc_drv_net_HTTPs_Unregister_Content_Type = qc_drv_net_HTTPs_Unregister_Content_Type,
	.qc_drv_net_HTTPs_Set_Buffer_Size = qc_drv_net_HTTPs_Set_Buffer_Size,
	.qc_drv_net_Bridge_Enable = qc_drv_net_Bridge_Enable,
	.qc_drv_net_Bridge_Set_Aging_Timeout = qc_drv_net_Bridge_Set_Aging_Timeout,
	.qc_drv_net_Bridge_Show_MACs = qc_drv_net_Bridge_Show_MACs,
    .qc_drv_net_Interface_Get_Physical_Address = qc_drv_net_Interface_Get_Physical_Address,
    .qc_drv_net_IPv6_Get_Address = qc_drv_net_IPv6_Get_Address,
    .qc_drv_net_socketclose = qc_drv_net_socketclose,
    .qc_drv_net_socket = qc_drv_net_socket,
    .qc_drv_net_connect = qc_drv_net_connect,
    .qc_drv_net_send = qc_drv_net_send,
    .qc_drv_net_recv =qc_drv_net_recv,
    .qc_drv_net_recvfrom = qc_drv_net_recvfrom,
    .qc_drv_net_sendto = qc_drv_net_sendto,
    .qc_drv_net_bind = qc_drv_net_bind,
    .qc_drv_net_listen = qc_drv_net_listen,
    .qc_drv_net_accept = qc_drv_net_accept,
    .qc_drv_net_IPv6_Get_Scope_ID = qc_drv_net_IPv6_Get_Scope_ID,
    .qc_drv_net_setsockopt = qc_drv_net_setsockopt,
    .qc_drv_net_SSL_Write_To = qc_drv_net_SSL_Write_To,
    .qc_drv_net_select = qc_drv_net_select,
    .qc_drv_net_SSL_Read = qc_drv_net_SSL_Read,
    .qc_drv_net_Buf_Alloc = qc_drv_net_Buf_Alloc,
    .qc_drv_net_fd_zero = qc_drv_net_fd_zero,
    .qc_drv_net_fd_set = qc_drv_net_fd_set,
    .qc_drv_net_Buf_Free = qc_drv_net_Buf_Free,
    .qc_drv_net_fd_clr = qc_drv_net_fd_clr,
    .qc_drv_net_fd_isset = qc_drv_net_fd_isset,
    .qc_drv_net_SSL_Obj_New = qc_drv_net_SSL_Obj_New,
    .qc_drv_net_SSL_Obj_Free = qc_drv_net_SSL_Obj_Free,
    .qc_drv_net_SSL_Cipher_Add = qc_drv_net_SSL_Cipher_Add,
    .qc_drv_net_SSL_ALPN_Protocol_Add = qc_drv_net_SSL_ALPN_Protocol_Add,
    .qc_drv_net_SSL_Con_New = qc_drv_net_SSL_Con_New,
    .qc_drv_net_SSL_Configure = qc_drv_net_SSL_Configure,
    .qc_drv_net_SSL_Cert_Load = qc_drv_net_SSL_Cert_Load,
    .qc_drv_net_SSL_Max_Clients_Set = qc_drv_net_SSL_Max_Clients_Set,
    .qc_drv_net_SSL_Idle_Timeout_Set = qc_drv_net_SSL_Idle_Timeout_Set,
    .qc_drv_net_SSL_ECJPAKE_Parameters_Set = qc_drv_net_SSL_ECJPAKE_Parameters_Set,
    .qc_drv_net_SSL_PSK_Table_Set = qc_drv_net_SSL_PSK_Table_Set,
    .qc_drv_net_SSL_Con_Get_Status = qc_drv_net_SSL_Con_Get_Status,
    .qc_drv_net_SSL_Fd_Set = qc_drv_net_SSL_Fd_Set,
    .qc_drv_net_SSL_Accept = qc_drv_net_SSL_Accept,
    .qc_drv_net_SSL_Shutdown = qc_drv_net_SSL_Shutdown,
    .qc_drv_net_IPv6_Config_Router_Prefix = qc_drv_net_IPv6_Config_Router_Prefix,
    .qc_drv_net_IPv6_Route_Add = qc_drv_net_IPv6_Route_Add,
    .qc_drv_net_IPv6_Route_Del = qc_drv_net_IPv6_Route_Del,
    .qc_drv_net_IPv6_Routing_Table_Get = qc_drv_net_IPv6_Routing_Table_Get,
    .qc_drv_net_Profile_Set_Custom = qc_drv_net_Profile_Set_Custom,
    .qc_drv_net_Profile_Set_Active = qc_drv_net_Profile_Set_Active,
    .qc_drv_net_Profile_Get_Active = qc_drv_net_Profile_Get_Active,
    .qc_drv_net_OMTM_Switch_Operating_Mode = qc_drv_net_OMTM_Switch_Operating_Mode,
    .qc_drv_net_DNSs_Is_Started = qc_drv_net_DNSs_Is_Started,
    .qc_drv_net_DNSs_Get_Host_List = qc_drv_net_DNSs_Get_Host_List,
    .qc_drv_net_DNSs_Add_Host = qc_drv_net_DNSs_Add_Host,
    .qc_drv_net_DNSs_Del_Host = qc_drv_net_DNSs_Del_Host,
    .qc_drv_net_DNSs_Command = qc_drv_net_DNSs_Command,
    .qc_drv_net_mDNS_Command = qc_drv_net_mDNS_Command,
    .qc_drv_net_DNSSD_Start = qc_drv_net_DNSSD_Start,
    .qc_drv_net_DNSSD_Init = qc_drv_net_DNSSD_Init,
    .qc_drv_net_DNSSD_Stop = qc_drv_net_DNSSD_Stop,
    .qc_drv_net_DNSSD_Discover = qc_drv_net_DNSSD_Discover,
    .qc_drv_net_DNSSD_Get_Target_Info = qc_drv_net_DNSSD_Get_Target_Info,
    .qc_drv_net_Buf_Free_Queue_Status = qc_drv_net_Buf_Free_Queue_Status,
    .qc_drv_net_Get_Socket_Status = qc_drv_net_Get_Socket_Status,
    .qc_drv_net_IPv4_Route = qc_drv_net_IPv4_Route,
    .qc_drv_net_HTTPc_Start = qc_drv_net_HTTPc_Start,
    .qc_drv_net_HTTPc_Stop = qc_drv_net_HTTPc_Stop,
    .qc_drv_net_HTTPc_New_sess = qc_drv_net_HTTPc_New_sess,
    .qc_drv_net_HTTPc_Configure_SSL = qc_drv_net_HTTPc_Configure_SSL,
    .qc_drv_net_HTTPc_Connect = qc_drv_net_HTTPc_Connect,
    .qc_drv_net_HTTPc_Free_sess = qc_drv_net_HTTPc_Free_sess,
    .qc_drv_net_HTTPc_Request = qc_drv_net_HTTPc_Request,
    .qc_drv_net_HTTPc_Set_Body = qc_drv_net_HTTPc_Set_Body,
    .qc_drv_net_HTTPc_Add_Header_Field = qc_drv_net_HTTPc_Add_Header_Field,
    .qc_drv_net_HTTPc_Clear_Header = qc_drv_net_HTTPc_Clear_Header,
    .qc_drv_net_HTTPc_Set_Param = qc_drv_net_HTTPc_Set_Param,
    .qc_drv_net_HTTPc_CB_Enable_Adding_Header = qc_drv_net_HTTPc_CB_Enable_Adding_Header,

    /* Start of Ble */
    .qc_drv_ble_BSC_Query_Host_Version = qc_drv_ble_BSC_Query_Host_Version,
    .qc_drv_ble_HCI_Read_Local_Version_Information = qc_drv_ble_HCI_Read_Local_Version_Information,
    .qc_drv_ble_GAP_LE_Set_Pairability_Mode = qc_drv_ble_GAP_LE_Set_Pairability_Mode,
    .qc_drv_ble_GAP_LE_Authentication_Response = qc_drv_ble_GAP_LE_Authentication_Response,
    .qc_drv_ble_GAP_LE_Query_Encryption_Mode = qc_drv_ble_GAP_LE_Query_Encryption_Mode,
    .qc_drv_ble_GAP_LE_Set_Fixed_Passkey = qc_drv_ble_GAP_LE_Set_Fixed_Passkey,
    .qc_drv_ble_GAP_Query_Local_BD_ADDR = qc_drv_ble_GAP_Query_Local_BD_ADDR,
    .qc_drv_ble_GAP_LE_Advertising_Disable = qc_drv_ble_GAP_LE_Advertising_Disable,
    .qc_drv_ble_GAPS_Query_Device_Appearance = qc_drv_ble_GAPS_Query_Device_Appearance,
    .qc_drv_ble_GAPS_Query_Device_Name = qc_drv_ble_GAPS_Query_Device_Name,
    .qc_drv_ble_GAP_LE_Set_Advertising_Data = qc_drv_ble_GAP_LE_Set_Advertising_Data,
    .qc_drv_ble_GAP_LE_Set_Scan_Response_Data = qc_drv_ble_GAP_LE_Set_Scan_Response_Data,
    .qc_drv_ble_BSC_LockBluetoothStack = qc_drv_ble_BSC_LockBluetoothStack,
    .qc_drv_ble_BSC_UnLockBluetoothStack = qc_drv_ble_BSC_UnLockBluetoothStack,
    .qc_drv_ble_GAP_LE_Advertising_Enable = qc_drv_ble_GAP_LE_Advertising_Enable,
    .qc_drv_ble_GAP_LE_Disconnect = qc_drv_ble_GAP_LE_Disconnect,
    .qc_drv_ble_GAP_LE_Cancel_Create_Connection = qc_drv_ble_GAP_LE_Cancel_Create_Connection,
    .qc_drv_ble_GATT_Start_Service_Discovery_Handle_Range = qc_drv_ble_GATT_Start_Service_Discovery_Handle_Range,
    .qc_drv_ble_GATT_Start_Service_Discovery = qc_drv_ble_GATT_Start_Service_Discovery,
    .qc_drv_ble_GAP_LE_Add_Device_To_White_List = qc_drv_ble_GAP_LE_Add_Device_To_White_List,
    .qc_drv_ble_GAP_LE_Remove_Device_From_White_List = qc_drv_ble_GAP_LE_Remove_Device_From_White_List,
    .qc_drv_ble_GAP_LE_Add_Device_To_Resolving_List = qc_drv_ble_GAP_LE_Add_Device_To_Resolving_List,
    .qc_drv_ble_GAP_LE_Remove_Device_From_Resolving_List = qc_drv_ble_GAP_LE_Remove_Device_From_Resolving_List,
    .qc_drv_ble_GAP_LE_Set_Authenticated_Payload_Timeout = qc_drv_ble_GAP_LE_Set_Authenticated_Payload_Timeout,
    .qc_drv_ble_GATT_Change_Maximum_Supported_MTU = qc_drv_ble_GATT_Change_Maximum_Supported_MTU,
    .qc_drv_ble_GATT_Query_Maximum_Supported_MTU = qc_drv_ble_GATT_Query_Maximum_Supported_MTU,
    .qc_drv_ble_GAP_LE_Query_Connection_Handle = qc_drv_ble_GAP_LE_Query_Connection_Handle,
    .qc_drv_ble_GAP_LE_Set_Data_Length = qc_drv_ble_GAP_LE_Set_Data_Length,
    .qc_drv_ble_GAP_LE_Query_Local_Secure_Connections_OOB_Data = qc_drv_ble_GAP_LE_Query_Local_Secure_Connections_OOB_Data,
    .qc_drv_ble_BSC_SetTxPower = qc_drv_ble_BSC_SetTxPower,
    .qc_drv_ble_BSC_Set_FEM_Control_Override = qc_drv_ble_BSC_Set_FEM_Control_Override,
    .qc_drv_ble_AIOS_Initialize_Service = qc_drv_ble_AIOS_Initialize_Service,
    .qc_drv_ble_AIOS_Cleanup_Service    = qc_drv_ble_AIOS_Cleanup_Service,
    .qc_drv_ble_GATT_Read_Value_Request = qc_drv_ble_GATT_Read_Value_Request,
    .qc_drv_ble_AIOS_Notify_Characteristic = qc_drv_ble_AIOS_Notify_Characteristic,
    .qc_drv_ble_GATT_Write_Request =    qc_drv_ble_GATT_Write_Request,
    .qc_drv_ble_HIDS_Initialize_Service = qc_drv_ble_HIDS_Initialize_Service,
    .qc_drv_ble_HIDS_Cleanup_Service    =   qc_drv_ble_HIDS_Cleanup_Service,
    .qc_drv_ble_HIDS_Notify_Input_Report = qc_drv_ble_HIDS_Notify_Input_Report,
    .qc_drv_ble_HIDS_Format_Control_Point_Command = qc_drv_ble_HIDS_Format_Control_Point_Command,
    .qc_drv_ble_GATT_Write_Without_Response_Request = qc_drv_ble_GATT_Write_Without_Response_Request,
    .qc_drv_ble_HIDS_Format_Protocol_Mode = qc_drv_ble_HIDS_Format_Protocol_Mode,
    .qc_drv_ble_SCPS_Initialize_Service = qc_drv_ble_SCPS_Initialize_Service,
    .qc_drv_ble_SCPS_Cleanup_Service = qc_drv_ble_SCPS_Cleanup_Service ,
    .qc_drv_ble_SCPS_Format_Scan_Interval_Window = qc_drv_ble_SCPS_Format_Scan_Interval_Window,
    .qc_drv_ble_SCPS_Notify_Scan_Refresh = qc_drv_ble_SCPS_Notify_Scan_Refresh,
    .qc_drv_ble_GATT_Register_Service = qc_drv_ble_GATT_Register_Service,
    .qc_drv_ble_GATT_Un_Register_Service =  qc_drv_ble_GATT_Un_Register_Service,
    .qc_drv_ble_BAS_Initialize_Service  = qc_drv_ble_BAS_Initialize_Service,
    .qc_drv_ble_BAS_Set_Characteristic_Presentation_Format = qc_drv_ble_BAS_Set_Characteristic_Presentation_Format,
    .qc_drv_ble_BAS_Cleanup_Service = qc_drv_ble_BAS_Cleanup_Service,
    .qc_drv_ble_BAS_Notify_Battery_Level = qc_drv_ble_BAS_Notify_Battery_Level,
    .qc_drv_ble_BAS_Query_Characteristic_Presentation_Format = qc_drv_ble_BAS_Query_Characteristic_Presentation_Format,
    .qc_drv_ble_GAPS_Set_Device_Name = qc_drv_ble_GAPS_Set_Device_Name,
    .qc_drv_ble_GAPS_Set_Device_Appearance = qc_drv_ble_GAPS_Set_Device_Appearance,
    .qc_drv_ble_DIS_Initialize_Service  = qc_drv_ble_DIS_Initialize_Service,
    .qc_drv_ble_DIS_Set_Manufacturer_Name = qc_drv_ble_DIS_Set_Manufacturer_Name,
    .qc_drv_ble_DIS_Set_Model_Number = qc_drv_ble_DIS_Set_Model_Number,
    .qc_drv_ble_DIS_Set_Software_Revision = qc_drv_ble_DIS_Set_Software_Revision,
    .qc_drv_ble_DIS_Set_Hardware_Revision = qc_drv_ble_DIS_Set_Hardware_Revision,
    .qc_drv_ble_DIS_Set_Firmware_Revision = qc_drv_ble_DIS_Set_Firmware_Revision,
    .qc_drv_ble_DIS_Cleanup_Service = qc_drv_ble_DIS_Cleanup_Service,
    .qc_drv_ble_TPS_Initialize_Service = qc_drv_ble_TPS_Initialize_Service,
    .qc_drv_ble_TPS_Set_Tx_Power_Level = qc_drv_ble_TPS_Set_Tx_Power_Level,
    .qc_drv_ble_TPS_Cleanup_Service = qc_drv_ble_TPS_Cleanup_Service,
    .qc_drv_ble_GAP_LE_Query_Connection_PHY = qc_drv_ble_GAP_LE_Query_Connection_PHY,
    .qc_drv_ble_GAP_LE_Set_Connection_PHY = qc_drv_ble_GAP_LE_Set_Connection_PHY,
    .qc_drv_ble_GAP_LE_Set_Extended_Advertising_Parameters = qc_drv_ble_GAP_LE_Set_Extended_Advertising_Parameters,
    .qc_drv_ble_GAP_LE_Enable_Extended_Advertising = qc_drv_ble_GAP_LE_Enable_Extended_Advertising,
    .qc_drv_ble_GAP_LE_Set_Extended_Scan_Parameters = qc_drv_ble_GAP_LE_Set_Extended_Scan_Parameters,
    .qc_drv_ble_GAP_LE_Enable_Extended_Scan = qc_drv_ble_GAP_LE_Enable_Extended_Scan,
    .qc_drv_ble_GAP_LE_Extended_Create_Connection = qc_drv_ble_GAP_LE_Extended_Create_Connection,
    .qc_drv_ble_BSC_AddGenericListEntry_Actual = qc_drv_ble_BSC_AddGenericListEntry_Actual,
    .qc_drv_ble_BSC_Initialize  = qc_drv_ble_BSC_Initialize,
    .qc_drv_ble_HCI_LE_Read_Buffer_Size = qc_drv_ble_HCI_LE_Read_Buffer_Size,
    .qc_drv_ble_GATT_Initialize = qc_drv_ble_GATT_Initialize,
    .qc_drv_ble_GAPS_Initialize_Service = qc_drv_ble_GAPS_Initialize_Service,
    .qc_drv_ble_GAP_LE_Set_Address_Resolution_Enable = qc_drv_ble_GAP_LE_Set_Address_Resolution_Enable,
    .qc_drv_ble_GAP_LE_Set_Resolvable_Private_Address_Timeout = qc_drv_ble_GAP_LE_Set_Resolvable_Private_Address_Timeout,
    .qc_drv_ble_HCI_Register_Event_Callback = qc_drv_ble_HCI_Register_Event_Callback,
    .qc_drv_ble_HCI_Register_ACL_Data_Callback = qc_drv_ble_HCI_Register_ACL_Data_Callback,
    .qc_drv_ble_HCI_Send_ACL_Data = qc_drv_ble_HCI_Send_ACL_Data,
    .qc_drv_ble_GAP_LE_Register_Remote_Authentication = qc_drv_ble_GAP_LE_Register_Remote_Authentication,
    .qc_drv_ble_BSC_StartTimer = qc_drv_ble_BSC_StartTimer,
    .qc_drv_ble_GAP_LE_Perform_Scan = qc_drv_ble_GAP_LE_Perform_Scan,
    .qc_drv_ble_GAP_LE_Cancel_Scan = qc_drv_ble_GAP_LE_Cancel_Scan,
    .qc_drv_ble_GAP_LE_Create_Connection = qc_drv_ble_GAP_LE_Create_Connection,
    .qc_drv_ble_HRS_Decode_Heart_Rate_Measurement = qc_drv_ble_HRS_Decode_Heart_Rate_Measurement,
    .qc_drv_ble_GATT_Handle_Value_Notification = qc_drv_ble_GATT_Handle_Value_Notification,
    .qc_drv_ble_BSC_GetTxPower = qc_drv_ble_BSC_GetTxPower,
    .qc_drv_ble_GAP_LE_Reestablish_Security = qc_drv_ble_GAP_LE_Reestablish_Security,
    .qc_drv_ble_GAP_LE_Regenerate_Long_Term_Key = qc_drv_ble_GAP_LE_Regenerate_Long_Term_Key,
    .qc_drv_ble_AIOS_Read_Characteristic_Request_Response = qc_drv_ble_AIOS_Read_Characteristic_Request_Response,
    .qc_drv_ble_AIOS_Write_Characteristic_Request_Response = qc_drv_ble_AIOS_Write_Characteristic_Request_Response,
    .qc_drv_ble_AIOS_Read_CCCD_Request_Response = qc_drv_ble_AIOS_Read_CCCD_Request_Response,
    .qc_drv_ble_AIOS_Write_CCCD_Request_Response = qc_drv_ble_AIOS_Write_CCCD_Request_Response,
    .qc_drv_ble_AIOS_Read_Presentation_Format_Request_Response = qc_drv_ble_AIOS_Read_Presentation_Format_Request_Response,
    .qc_drv_ble_AIOS_Read_Number_Of_Digitals_Request_Response = qc_drv_ble_AIOS_Read_Number_Of_Digitals_Request_Response,
    .qc_drv_ble_BAS_Read_Client_Configuration_Response = qc_drv_ble_BAS_Read_Client_Configuration_Response,
    .qc_drv_ble_HIDS_Read_Client_Configuration_Response = qc_drv_ble_HIDS_Read_Client_Configuration_Response,
    .qc_drv_ble_HIDS_Get_Protocol_Mode_Response = qc_drv_ble_HIDS_Get_Protocol_Mode_Response,
    .qc_drv_ble_HIDS_Get_Report_Map_Response = qc_drv_ble_HIDS_Get_Report_Map_Response,
    .qc_drv_ble_HIDS_Get_Report_Response = qc_drv_ble_HIDS_Get_Report_Response,
    .qc_drv_ble_HIDS_Set_Report_Response = qc_drv_ble_HIDS_Set_Report_Response,
    .qc_drv_ble_SCPS_Read_Client_Configuration_Response = qc_drv_ble_SCPS_Read_Client_Configuration_Response,
    .qc_drv_ble_AIOS_Decode_Presentation_Format = qc_drv_ble_AIOS_Decode_Presentation_Format,
    .qc_drv_ble_BAS_Decode_Characteristic_Presentation_Format = qc_drv_ble_BAS_Decode_Characteristic_Presentation_Format,
    .qc_drv_ble_GATT_Read_Long_Value_Request = qc_drv_ble_GATT_Read_Long_Value_Request,
    .qc_drv_ble_HIDS_Decode_HID_Information = qc_drv_ble_HIDS_Decode_HID_Information,
    .qc_drv_ble_HIDS_Decode_Report_Reference = qc_drv_ble_HIDS_Decode_Report_Reference,
    .qc_drv_ble_HIDS_Decode_External_Report_Reference = qc_drv_ble_HIDS_Decode_External_Report_Reference,


    /* Start of Zigbee*/
    .qc_drv_ZB_Initialize = qc_drv_ZB_Initialize,
    .qc_drv_ZB_Register_Persist_Notify_CB = qc_drv_ZB_Register_Persist_Notify_CB,
    .qc_drv_ZB_ZDP_Register_Callback = qc_drv_ZB_ZDP_Register_Callback,
    .qc_drv_ZB_Get_Extended_Address = qc_drv_ZB_Get_Extended_Address,
    .qc_drv_ZB_Shutdown = qc_drv_ZB_Shutdown,
    .qc_drv_ZB_Form = qc_drv_ZB_Form,
    .qc_drv_ZB_Join = qc_drv_ZB_Join,
    .qc_drv_ZB_Leave = qc_drv_ZB_Leave,
    .qc_drv_ZB_ZDP_Mgmt_Leave_Req = qc_drv_ZB_ZDP_Mgmt_Leave_Req,
    .qc_drv_ZB_Permit_Join = qc_drv_ZB_Permit_Join,
    .qc_drv_ZB_ZDP_Bind_Req = qc_drv_ZB_ZDP_Bind_Req,
    .qc_drv_ZB_ZDP_End_Device_Bind_Req = qc_drv_ZB_ZDP_End_Device_Bind_Req,
    .qc_drv_ZB_APSME_Get_Request = qc_drv_ZB_APSME_Get_Request,
    .qc_drv_ZB_APSME_Set_Request = qc_drv_ZB_APSME_Set_Request,
    .qc_drv_ZB_Set_Extended_Address = qc_drv_ZB_Set_Extended_Address,
    .qc_drv_ZB_APS_Add_Endpoint = qc_drv_ZB_APS_Add_Endpoint,
    .qc_drv_ZB_APS_Remove_Endpoint = qc_drv_ZB_APS_Remove_Endpoint,
    .qc_drv_ZB_CL_Read_Local_Attribute = qc_drv_ZB_CL_Read_Local_Attribute,
    .qc_drv_ZB_CL_Write_Local_Attribute = qc_drv_ZB_CL_Write_Local_Attribute,
    .qc_drv_ZB_CL_Read_Attributes = qc_drv_ZB_CL_Read_Attributes,
    .qc_drv_ZB_CL_Write_Attributes = qc_drv_ZB_CL_Write_Attributes,
    .qc_drv_ZB_CL_Discover_Attributes = qc_drv_ZB_CL_Discover_Attributes,
    .qc_drv_ZB_CL_Configure_Reporting = qc_drv_ZB_CL_Configure_Reporting,
    .qc_drv_ZB_CL_Read_Reporting = qc_drv_ZB_CL_Read_Reporting,
    .qc_drv_ZB_CL_OnOff_Send_On = qc_drv_ZB_CL_OnOff_Send_On,
    .qc_drv_ZB_CL_OnOff_Send_Off = qc_drv_ZB_CL_OnOff_Send_Off,
    .qc_drv_ZB_CL_OnOff_Send_Toggle = qc_drv_ZB_CL_OnOff_Send_Toggle,
    .qc_drv_ZB_CL_LevelControl_Send_Move_To_Level = qc_drv_ZB_CL_LevelControl_Send_Move_To_Level,
    .qc_drv_ZB_CL_LevelControl_Send_Move = qc_drv_ZB_CL_LevelControl_Send_Move,
    .qc_drv_ZB_CL_LevelControl_Send_Step = qc_drv_ZB_CL_LevelControl_Send_Step,
    .qc_drv_ZB_CL_LevelControl_Send_Stop = qc_drv_ZB_CL_LevelControl_Send_Stop,

    .qc_drv_ZB_BDB_Get_Request		= qc_drv_ZB_BDB_Get_Request,
    .qc_drv_ZB_BDB_Set_Request		= qc_drv_ZB_BDB_Set_Request,
    .qc_drv_ZB_ZDP_Match_Desc_Req	= qc_drv_ZB_ZDP_Match_Desc_Req,
    .qc_drv_ZB_CL_Basic_Send_Reset_To_Factory = qc_drv_ZB_CL_Basic_Send_Reset_To_Factory,
    .qc_drv_ZB_CL_Basic_Server_Read_Attribute =qc_drv_ZB_CL_Basic_Server_Read_Attribute,
    .qc_drv_ZB_CL_Basic_Server_Write_Attribute = qc_drv_ZB_CL_Basic_Server_Write_Attribute,
    .qc_drv_ZB_CL_Identify_Send_Identify = qc_drv_ZB_CL_Identify_Send_Identify,
    .qc_drv_ZB_CL_Identify_Send_Identify_Query	= qc_drv_ZB_CL_Identify_Send_Identify_Query,

    .qc_drv_ZB_CL_Groups_Send_Add_Group	=	qc_drv_ZB_CL_Groups_Send_Add_Group,
    .qc_drv_ZB_CL_Groups_Send_View_Group = qc_drv_ZB_CL_Groups_Send_View_Group,
    .qc_drv_ZB_CL_Groups_Send_Remove_Group	= qc_drv_ZB_CL_Groups_Send_Remove_Group,
    .qc_drv_ZB_CL_Groups_Send_Remove_All_Groups = qc_drv_ZB_CL_Groups_Send_Remove_All_Groups,
    .qc_drv_ZB_CL_Groups_Send_Get_Group_Membership = qc_drv_ZB_CL_Groups_Send_Get_Group_Membership,

    .qc_drv_ZB_CL_Scenes_Send_Add_Scene = qc_drv_ZB_CL_Scenes_Send_Add_Scene,
    .qc_drv_ZB_CL_Scenes_Send_View_Scene = qc_drv_ZB_CL_Scenes_Send_View_Scene,
    .qc_drv_ZB_CL_Scenes_Send_Remove_Scene = qc_drv_ZB_CL_Scenes_Send_Remove_Scene,
    .qc_drv_ZB_CL_Scenes_Send_Remove_All_Scenes = qc_drv_ZB_CL_Scenes_Send_Remove_All_Scenes,
    .qc_drv_ZB_CL_Scenes_Send_Store_Scene = qc_drv_ZB_CL_Scenes_Send_Store_Scene,
    .qc_drv_ZB_CL_Scenes_Send_Recall_Scene = qc_drv_ZB_CL_Scenes_Send_Recall_Scene,
    .qc_drv_ZB_CL_Scenes_Send_Get_Scene_Membership = qc_drv_ZB_CL_Scenes_Send_Get_Scene_Membership,
    .qc_drv_ZB_CL_Scenes_Send_Copy_Scene = qc_drv_ZB_CL_Scenes_Send_Copy_Scene,
    .qc_drv_ZB_CL_Time_Server_Read_Attribute = qc_drv_ZB_CL_Time_Server_Read_Attribute,
    .qc_drv_ZB_CL_Time_Server_Write_Attribute = qc_drv_ZB_CL_Time_Server_Write_Attribute,

    .qc_drv_ZB_CL_Alarm_Send_Reset_Alarm = qc_drv_ZB_CL_Alarm_Send_Reset_Alarm,
    .qc_drv_ZB_CL_Alarm_Send_Reset_All_Alarms = qc_drv_ZB_CL_Alarm_Send_Reset_All_Alarms,
    .qc_drv_ZB_CL_Alarm_Send_Get_Alarm = qc_drv_ZB_CL_Alarm_Send_Get_Alarm,
    .qc_drv_ZB_CL_Alarm_Send_Reset_Alarm_Log = qc_drv_ZB_CL_Alarm_Send_Reset_Alarm_Log,
    .qc_drv_ZB_CL_Alarm_Send_Alarm = qc_drv_ZB_CL_Alarm_Send_Alarm,

    .qc_drv_ZB_CL_ColorControl_Send_Move_To_Hue = qc_drv_ZB_CL_ColorControl_Send_Move_To_Hue,
    .qc_drv_ZB_CL_ColorControl_Send_Move_To_HueAndSaturation = qc_drv_ZB_CL_ColorControl_Send_Move_To_HueAndSaturation,
    .qc_drv_ZB_CL_ColorControl_Send_Move_Hue = qc_drv_ZB_CL_ColorControl_Send_Move_Hue,
    .qc_drv_ZB_CL_ColorControl_Send_Step_Hue = qc_drv_ZB_CL_ColorControl_Send_Step_Hue,
    .qc_drv_ZB_CL_ColorControl_Send_Move_To_Saturation = qc_drv_ZB_CL_ColorControl_Send_Move_To_Saturation,
    .qc_drv_ZB_CL_ColorControl_Send_Move_Saturation = qc_drv_ZB_CL_ColorControl_Send_Move_Saturation,
    .qc_drv_ZB_CL_ColorControl_Send_Step_Saturation = qc_drv_ZB_CL_ColorControl_Send_Step_Saturation,
    .qc_drv_ZB_CL_ColorControl_Send_Move_To_Color = qc_drv_ZB_CL_ColorControl_Send_Move_To_Color,
    .qc_drv_ZB_CL_ColorControl_Send_Move_Color = qc_drv_ZB_CL_ColorControl_Send_Move_Color,
    .qc_drv_ZB_CL_ColorControl_Send_Step_Color = qc_drv_ZB_CL_ColorControl_Send_Step_Color,
    .qc_drv_ZB_CL_ColorControl_Send_Move_To_Color_Temp = qc_drv_ZB_CL_ColorControl_Send_Move_To_Color_Temp,
    .qc_drv_ZB_CL_ColorControl_Send_Move_Color_Temp = qc_drv_ZB_CL_ColorControl_Send_Move_Color_Temp,
    .qc_drv_ZB_CL_ColorControl_Send_Step_Color_Temp = qc_drv_ZB_CL_ColorControl_Send_Step_Color_Temp,
    .qc_drv_ZB_CL_ColorControl_Send_Stop_Move_Step = qc_drv_ZB_CL_ColorControl_Send_Stop_Move_Step,
    .qc_drv_ZB_CL_ColorControl_Send_Color_Loop_Set = qc_drv_ZB_CL_ColorControl_Send_Color_Loop_Set,

	.qc_drv_ZB_Get_Persistent_Data  =   qc_drv_ZB_Get_Persistent_Data,
    .qc_drv_ZB_Restore_Persistent_Data = qc_drv_ZB_Restore_Persistent_Data,
    .qc_drv_ZB_NLME_Get_Request = qc_drv_ZB_NLME_Get_Request,
    .qc_drv_ZB_NLME_Set_Request = qc_drv_ZB_NLME_Set_Request,
    .qc_drv_ZB_CL_Touchlink_Start = qc_drv_ZB_CL_Touchlink_Start,
    .qc_drv_ZB_CL_Touchlink_Scan_Request = qc_drv_ZB_CL_Touchlink_Scan_Request,
    .qc_drv_ZB_CL_Touchlink_Factory_Reset = qc_drv_ZB_CL_Touchlink_Factory_Reset,
    .qc_drv_ZB_CL_Destroy_Cluster = qc_drv_ZB_CL_Destroy_Cluster,
    .qc_drv_Persist_Initialize = qc_drv_Persist_Initialize,
    .qc_drv_Persist_Delete = qc_drv_Persist_Delete,
    .qc_drv_Persist_Cleanup = qc_drv_Persist_Cleanup,


    /* Start of Thread */
    .qc_drv_TWN_Initialize = qc_drv_TWN_Initialize,
    .qc_drv_TWN_Shutdown = qc_drv_TWN_Shutdown,
    .qc_drv_TWN_Start = qc_drv_TWN_Start,
    .qc_drv_TWN_Stop = qc_drv_TWN_Stop,
    .qc_drv_TWN_Get_Device_Configuration = qc_drv_TWN_Get_Device_Configuration,
    .qc_drv_TWN_Set_Device_Configuration = qc_drv_TWN_Set_Device_Configuration,
    .qc_drv_TWN_Get_Network_Configuration = qc_drv_TWN_Get_Network_Configuration,
    .qc_drv_TWN_Set_Network_Configuration = qc_drv_TWN_Set_Network_Configuration,
    .qc_drv_TWN_Add_Border_Router = qc_drv_TWN_Add_Border_Router,
    .qc_drv_TWN_Remove_Border_Router = qc_drv_TWN_Remove_Border_Router,
    .qc_drv_TWN_Add_External_Route = qc_drv_TWN_Add_External_Route,
    .qc_drv_TWN_Remove_External_Route = qc_drv_TWN_Remove_External_Route,
    .qc_drv_TWN_Register_Server_Data = qc_drv_TWN_Register_Server_Data,
    .qc_drv_TWN_Set_IP_Stack_Integration = qc_drv_TWN_Set_IP_Stack_Integration,
    .qc_drv_TWN_Commissioner_Start = qc_drv_TWN_Commissioner_Start,
    .qc_drv_TWN_Commissioner_Stop = qc_drv_TWN_Commissioner_Stop,
    .qc_drv_TWN_Commissioner_Add_Joiner = qc_drv_TWN_Commissioner_Add_Joiner,
    .qc_drv_TWN_Commissioner_Remove_Joiner = qc_drv_TWN_Commissioner_Remove_Joiner,
    .qc_drv_TWN_Commissioner_Set_Provisioning_URL = qc_drv_TWN_Commissioner_Set_Provisioning_URL,
    .qc_drv_TWN_Commissioner_Generate_PSKc = qc_drv_TWN_Commissioner_Generate_PSKc,
    .qc_drv_TWN_Joiner_Start = qc_drv_TWN_Joiner_Start,
    .qc_drv_TWN_Joiner_Stop = qc_drv_TWN_Joiner_Stop,
    .qc_drv_TWN_Set_PSKc = qc_drv_TWN_Set_PSKc,
    .qc_drv_TWN_IPv6_Add_Unicast_Address = qc_drv_TWN_IPv6_Add_Unicast_Address,
    .qc_drv_TWN_IPv6_Remove_Unicast_Address = qc_drv_TWN_IPv6_Remove_Unicast_Address,
    .qc_drv_TWN_IPv6_Subscribe_Multicast_Address = qc_drv_TWN_IPv6_Subscribe_Multicast_Address,
    .qc_drv_TWN_IPv6_Unsubscribe_Multicast_Address = qc_drv_TWN_IPv6_Unsubscribe_Multicast_Address,
    .qc_drv_TWN_Set_Ping_Response_Enabled = qc_drv_TWN_Set_Ping_Response_Enabled,
    .qc_drv_TWN_Become_Router = qc_drv_TWN_Become_Router,
    .qc_drv_TWN_Become_Leader = qc_drv_TWN_Become_Leader,
    .qc_drv_TWN_Start_Border_Agent = qc_drv_TWN_Start_Border_Agent,
    .qc_drv_TWN_Stop_Border_Agent = qc_drv_TWN_Stop_Border_Agent,
    .qc_drv_TWN_Clear_Persistent_Data = qc_drv_TWN_Clear_Persistent_Data,
    .qc_drv_TWN_Set_Max_Poll_Period = qc_drv_TWN_Set_Max_Poll_Period,
    .qc_drv_TWN_Commissioner_Send_Mgmt_Get = qc_drv_TWN_Commissioner_Send_Mgmt_Get,
    .qc_drv_TWN_Commissioner_Send_Mgmt_Set = qc_drv_TWN_Commissioner_Send_Mgmt_Set,
    .qc_drv_TWN_Commissioner_Send_PanId_Query = qc_drv_TWN_Commissioner_Send_PanId_Query,
    .qc_drv_TWN_Commissioner_Get_Session_Id = qc_drv_TWN_Commissioner_Get_Session_Id,
    .qc_drv_TWN_Commissioner_Send_Mgmt_Active_Get = qc_drv_TWN_Commissioner_Send_Mgmt_Active_Get,
    .qc_drv_TWN_Commissioner_Send_Mgmt_Active_Set = qc_drv_TWN_Commissioner_Send_Mgmt_Active_Set,
};

qc_drv_context *driver_init()
{
    qc_drv_context *drv_ctx = qc_drv_register(&qc_drv_ops);
    if (!drv_ctx) {
        return NULL;
    }

    return drv_ctx;
}
