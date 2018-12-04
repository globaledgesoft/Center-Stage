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

#ifndef __QC_DRV_WIFI_H
#define __QC_DRV_WIFI_H

#include "qc_drv_main.h"

qapi_Status_t qc_drv_wlan_enable(qc_drv_context *qc_drv_ctx);
qapi_Status_t qc_drv_wlan_disable(qc_drv_context *qc_drv_ctx);
qapi_Status_t qc_drv_wlan_set_encr_type(qc_drv_context *qc_drv_ctx, uint32_t deviceId,
        void *encrType, int size, int flag);
qapi_Status_t qc_drv_wlan_set_auth_mode(qc_drv_context *qc_drv_ctx, uint32_t deviceId,
        qapi_WLAN_Auth_Mode_e *authMode, int size, int flag);
qapi_Status_t qc_drv_wlan_get_op_mode(qc_drv_context *qc_drv_ctx, uint32_t deviceId,
        uint32_t *wifimode, uint32_t *dataLen);
qapi_Status_t qc_drv_wlan_get_mac_addr(qc_drv_context *qc_drv_ctx, uint32_t deviceId,
        uint8_t *own_Interface_Addr, uint32_t *dataLen);
qapi_Status_t qc_drv_wlan_set_op_mode(qc_drv_context *qc_drv_ctx, uint32_t deviceId,
        qapi_WLAN_Dev_Mode_e *opMode, int size, int flag);
qapi_Status_t qc_drv_wlan_set_power_param(qc_drv_context *qc_drv_ctx, uint32_t deviceId,
        void *pwrMode, int size, int flag);
qapi_Status_t qc_drv_wlan_get_power_param(qc_drv_context *qc_drv_ctx, uint32_t deviceId,
        uint32_t *mode, uint32_t *dataLen);
qapi_Status_t qc_drv_wlan_add_device(qc_drv_context *qc_drv_ctx, uint8_t device_ID);
qapi_Status_t qc_drv_wlan_remove_device(qc_drv_context *qc_drv_ctx, uint8_t device_ID);

qapi_Status_t qc_drv_wlan_start_scan(qc_drv_context *qc_drv_ctx, uint8_t device_ID,
        const qapi_WLAN_Start_Scan_Params_t  *scan_Params,
        qapi_WLAN_Store_Scan_Results_e store_Scan_Results);

qapi_Status_t qc_drv_wlan_set_mac_addr(qc_drv_context *qc_drv_ctx, uint32_t deviceId,
        uint8_t *own_Interface_Addr, uint32_t dataLen);

qapi_Status_t qc_drv_wlan_set_ssid(qc_drv_context *qc_drv_ctx, uint32_t deviceId,
        uint8_t *ssid, uint32_t ssid_len, uint8_t flag);

qapi_Status_t qc_drv_wlan_set_passphrase(qc_drv_context *qc_drv_ctx, uint32_t deviceId,
        void *passphrase, uint32_t passphrase_len, uint8_t flag);

qapi_Status_t qc_drv_wlan_set_security_pmk(qc_drv_context *qc_drv_ctx, uint32_t deviceId,
        void *passphrase, uint32_t passphrase_len, uint8_t flag);

qapi_Status_t qc_drv_wlan_set_wps_credentials(qc_drv_context *qc_drv_ctx, uint32_t deviceId,
        qapi_WLAN_WPS_Credentials_t *wpsCred, uint32_t size, uint8_t flag);

qapi_Status_t qc_drv_wlan_set_channel(qc_drv_context *qc_drv_ctx, uint32_t deviceId,
        uint32_t *channel_val, uint32_t size, uint8_t flag);

qapi_Status_t qc_drv_wlan_set_listen_interval(qc_drv_context *qc_drv_ctx, uint32_t deviceId,
        int32_t *listen_time, uint32_t size, uint8_t flag);

qapi_Status_t qc_drv_wlan_set_sta_keep_alive(qc_drv_context *qc_drv_ctx, uint32_t deviceId,
        int32_t *keep_alive, uint32_t size, uint8_t flag);

qapi_Status_t qc_drv_wlan_set_rssi_threshold(qc_drv_context *qc_drv_ctx, uint32_t deviceId,
        qapi_WLAN_Rssi_Threshold_Params_t *qrthresh, uint32_t size, uint8_t flag);

qapi_Status_t qc_drv_wlan_set_probe_req_fwd_to_host(qc_drv_context *qc_drv_ctx, uint32_t deviceId,
        int32_t *enable, uint32_t size, uint8_t flag);

qapi_Status_t qc_drv_wlan_set_wep_key_index(qc_drv_context *qc_drv_ctx, uint32_t deviceId,
        int32_t *key_idx, uint32_t size, uint8_t flag);

qapi_Status_t qc_drv_wlan_set_wep_key_pair(qc_drv_context *qc_drv_ctx, uint32_t deviceId,
        qapi_WLAN_Security_Wep_Key_Pair_Params_t *keyPair, uint32_t size, uint8_t flag);

qapi_Status_t qc_drv_wlan_set_rate(qc_drv_context *qc_drv_ctx, uint32_t deviceId,
        qapi_WLAN_Bit_Rate_t *rateIndex, uint32_t size, uint8_t flag);

qapi_Status_t qc_drv_wlan_set_ap_wps_flag(qc_drv_context *qc_drv_ctx, uint32_t deviceId,
        uint8_t *wps_flag, uint32_t size, uint8_t flag);

qapi_Status_t qc_drv_wlan_set_tx_power_in_dbm(qc_drv_context *qc_drv_ctx, uint32_t deviceId,
        int32_t *power_in_dBm, uint32_t size, uint8_t flag);

qapi_Status_t qc_drv_wlan_set_phy_mode(qc_drv_context *qc_drv_ctx, uint32_t deviceId,
        qapi_WLAN_Phy_Mode_e *phyMode, uint32_t size, uint8_t flag);

qapi_Status_t qc_drv_wlan_set_ht_config(qc_drv_context *qc_drv_ctx, uint32_t deviceid,
        qapi_WLAN_11n_HT_Config_e *htconfig, uint32_t size, uint8_t flag);

qapi_Status_t qc_drv_wlan_set_ap_enable_hidden_mode(qc_drv_context *qc_drv_ctx, uint32_t deviceid,
        uint8_t *hidden_flag, uint32_t size, uint8_t flag);

qapi_Status_t qc_drv_wlan_set_beacon_interval_in_tu(qc_drv_context *qc_drv_ctx, uint32_t deviceid,
        uint32_t *beacon_int_in_tu, uint32_t size, uint8_t flag);

qapi_Status_t qc_drv_wlan_set_ap_dtim_interval(qc_drv_context *qc_drv_ctx, uint32_t deviceid,
        uint32_t *dtim_period, uint32_t size, uint8_t flag);

qapi_Status_t qc_drv_wlan_set_country_code(qc_drv_context *qc_drv_ctx, uint32_t deviceid,
        char *country_code, uint32_t size, uint8_t flag);

qapi_Status_t qc_drv_wlan_set_ap_country_code(qc_drv_context *qc_drv_ctx, uint32_t deviceid,
        char *country_code, uint32_t size, uint8_t flag);

qapi_Status_t qc_drv_wlan_get_scan_results(qc_drv_context *qc_drv_ctx, uint8_t device_ID,
        qapi_WLAN_BSS_Scan_Info_t  *scan_Info,
        int16_t  *num_Results);

qapi_Status_t qc_drv_wlan_get_phy_mode(qc_drv_context *qc_drv_ctx, uint8_t device_ID,
        qapi_WLAN_Phy_Mode_e *phyMode,
        uint32_t  *dataLen);

qapi_Status_t qc_drv_wlan_get_ht_config(qc_drv_context *qc_drv_ctx, uint8_t device_ID,
        qapi_WLAN_11n_HT_Config_e *htConfig,
        uint32_t  *dataLen);

qapi_Status_t qc_drv_wlan_get_ssid(qc_drv_context *qc_drv_ctx, uint8_t device_ID,
        void * ssid,
        uint32_t  *dataLen);

qapi_Status_t qc_drv_wlan_get_channel(qc_drv_context *qc_drv_ctx, uint8_t device_ID,
        uint16_t *channel_val,
        uint32_t  *dataLen);

qapi_Status_t qc_drv_wlan_get_wep_key_pair(qc_drv_context *qc_drv_ctx, uint8_t device_ID,
        qapi_WLAN_Security_Wep_Key_Pair_Params_t *key_pair,
        uint32_t  *dataLen);

qapi_Status_t qc_drv_wlan_get_wep_key_index(qc_drv_context *qc_drv_ctx, uint8_t device_ID,
        uint32_t *key_index,
        uint32_t  *dataLen);

qapi_Status_t qc_drv_wlan_get_rssi(qc_drv_context *qc_drv_ctx, uint8_t device_ID,
        uint8_t *rssi,
        uint32_t  *dataLen);

qapi_Status_t qc_drv_wlan_get_reg_domain_channel_list(qc_drv_context *qc_drv_ctx, uint8_t device_ID,
        qapi_WLAN_Get_Channel_List_t *wlanChannelList,
        uint32_t  *dataLen);

qapi_Status_t qc_drv_wlan_get_wireless_stats(qc_drv_context *qc_drv_ctx, uint8_t device_ID,
        qapi_WLAN_Get_Statistics_t *getStats,
        uint32_t  *dataLen);

qapi_Status_t qc_drv_wlan_get_rate(qc_drv_context *qc_drv_ctx, uint8_t device_ID,
        int32_t *rate_index,
        uint32_t  *dataLen);

qapi_Status_t qc_drv_wlan_get_firmware_version(qc_drv_context *qc_drv_ctx, uint8_t device_ID,
        qapi_WLAN_Firmware_Version_String_t *versionstr,
        uint32_t  *dataLen);

qapi_Status_t qc_drv_wlan_get_reg_domain(qc_drv_context *qc_drv_ctx, uint8_t device_ID,
        uint32_t *regDomain,
        uint32_t  *dataLen);

qapi_Status_t qc_drv_wlan_get_last_error(qc_drv_context *qc_drv_ctx, uint8_t device_ID,
        int32_t *err,
        uint32_t  *dataLen);

qapi_Status_t qc_drv_wlan_get_tx_status(qc_drv_context *qc_drv_ctx, uint8_t device_ID,
        uint32_t *tx_status,
        uint32_t  *dataLen);

qapi_Status_t qc_drv_wlan_set_callback(qc_drv_context *qc_drv_ctx, uint8_t device_ID,
        qapi_WLAN_Callback_t callback,
        const void *application_Context);

qapi_Status_t qc_drv_wlan_wps_connect(qc_drv_context *qc_drv_ctx, uint8_t device_ID);

qapi_Status_t qc_drv_wlan_wps_await_completion(qc_drv_context *qc_drv_ctx, uint32_t device_ID,
        qapi_WLAN_Netparams_t *net_Params);

qapi_Status_t qc_drv_wlan_disconnect(qc_drv_context *qc_drv_ctx, uint8_t device_ID);

qapi_Status_t qc_drv_wlan_commit(qc_drv_context *qc_drv_ctx, uint8_t device_ID);

qapi_Status_t qc_drv_wlan_raw_send(qc_drv_context *qc_drv_ctx, uint8_t device_ID,
        const qapi_WLAN_Raw_Send_Params_t  *raw_Params);

qapi_Status_t qc_drv_wlan_suspend_start(qc_drv_context *qc_drv_ctx, uint32_t suspend_Time_In_Ms);

qapi_Status_t qc_drv_wlan_wps_start(qc_drv_context *qc_drv_ctx, uint8_t device_ID,
        qapi_WLAN_WPS_Connect_Action_e connect_Action,
        qapi_WLAN_WPS_Mode_e mode,
        const char  *pin);

qapi_Status_t qc_drv_wlan_set_get_auth_mode(qc_drv_context *qc_drv_ctx, uint8_t device_ID,
        void *authMode, size_t size, int flag);

qapi_Status_t qc_drv_wlan_promiscuous_mode(qc_drv_context *qc_drv_ctx, uint8_t device_ID,
        void *mode, size_t size, int flag);

qapi_Status_t qc_drv_wlan_force_assert(qc_drv_context *qc_drv_ctx, uint8_t device_ID,
        void *data, size_t size, int flag);

qapi_Status_t qc_drv_wlan_scan_param(qc_drv_context *qc_drv_ctx, uint8_t device_ID,
        void *scanParam, size_t size, int flag);

qapi_Status_t qc_drv_wlan_set_inactivity_time(qc_drv_context *qc_drv_ctx, uint8_t device_ID,
        void *time, size_t size, int flag);

qapi_Status_t qc_drv_wlan_set_ap_ps_buf(qc_drv_context *qc_drv_ctx, uint8_t device_ID,
        void *ps_val, size_t size, int flag);

qapi_Status_t qc_drv_wlan_set_ap_enable_uapsd(qc_drv_context *qc_drv_ctx, uint8_t device_ID,
        void *uapsd, size_t size, int flag);

qapi_Status_t qc_drv_wlan_set_sta_uapsd(qc_drv_context *qc_drv_ctx, uint8_t device_ID,
        void *uapsd, size_t size, int flag);

qapi_Status_t qc_drv_wlan_set_sta_max_sp_len(qc_drv_context *qc_drv_ctx, uint8_t device_ID,
        void *maxsp, size_t size, int flag);

qapi_Status_t qc_drv_wlan_set_wmm_config(qc_drv_context *qc_drv_ctx, uint8_t device_ID,
        void *wmm, size_t size, int flag);

qapi_Status_t qc_drv_wlan_set_ap_bss_mip(qc_drv_context *qc_drv_ctx, uint8_t device_ID,
        void *period, size_t size, int flag);

qapi_Status_t qc_drv_wlan_set_sta_sp(qc_drv_context *qc_drv_ctx, uint8_t device_ID,
        void *period, size_t size, int flag);

qapi_Status_t qc_drv_wlan_set_ap_sr(qc_drv_context *qc_drv_ctx, uint8_t device_ID,
        void *response, size_t size, int flag);

qapi_Status_t qc_drv_wlan_set_app_ie(qc_drv_context *qc_drv_ctx, uint8_t device_ID,
        void *ie_params, size_t size, int flag);

qapi_Status_t qc_drv_wlan_set_sta_bmiss_config(qc_drv_context *qc_drv_ctx, uint8_t device_ID,
        void *bmiss, size_t size, int flag);

qapi_Status_t qc_drv_wlan_set_suspend_resume(qc_drv_context *qc_drv_ctx, uint8_t device_ID,
        void *suspend, size_t size, int flag);

qapi_Status_t qc_drv_wlan_set_ka_offload_scfg(qc_drv_context *qc_drv_ctx, uint8_t device_Id,
        qapi_WLAN_TCP_Offload_Config_Params_t *cfg, size_t size, int flag);


qapi_Status_t qc_drv_wlan_set_ka_offload_enable(qc_drv_context *qc_drv_ctx, uint8_t device_Id,
        qapi_WLAN_TCP_Offload_Enable_t *enable, size_t size, int flag);


qapi_Status_t qc_drv_wlan_set_nw_offload_enable(qc_drv_context *qc_drv_ctx, uint8_t device_Id,
        qapi_WLAN_Preferred_Network_Offload_Config_t *param, size_t size, int flag);


qapi_Status_t qc_drv_wlan_set_nw_profile(qc_drv_context *qc_drv_ctx, uint8_t device_Id,
        qapi_WLAN_Preferred_Network_Profile_t *param, size_t size, int flag);


qapi_Status_t qc_drv_wlan_set_aggr_tid(qc_drv_context *qc_drv_ctx, uint8_t device_Id,
        qapi_WLAN_Aggregation_Params_t *param, size_t size, int flag);


qapi_Status_t qc_drv_wlan_set_aggrx_config(qc_drv_context *qc_drv_ctx, uint8_t device_Id,
        qapi_WLAN_Rx_Aggrx_Params_t *param, size_t size, int flag);


qapi_Status_t qc_drv_wlan_set_pktlog_enable(qc_drv_context *qc_drv_ctx, uint8_t device_Id,
        qapi_WLAN_Pktlog_Enable_t *pktlog, size_t size, int flag);


qapi_Status_t qc_drv_wlan_set_pktlog_start(qc_drv_context *qc_drv_ctx, uint8_t device_Id,
        qapi_WLAN_Pktlog_Start_Params_t *pktlog, size_t size, int flag);


qapi_Status_t qc_drv_wlan_set_dblog_enable(qc_drv_context *qc_drv_ctx, uint8_t device_Id,
        qapi_WLAN_Dbglog_Enable_t *dbglog, size_t size, int flag);


qapi_Status_t qc_drv_wlan_set_dblog_config(qc_drv_context *qc_drv_ctx, uint8_t device_Id,
        qapi_WLAN_Dbglog_Config_t *config, size_t size, int flag);


qapi_Status_t qc_drv_wlan_set_dblog_module_conf(qc_drv_context *qc_drv_ctx, uint8_t device_Id,
        qapi_WLAN_Dbglog_Module_Config_t *config, size_t size, int flag);


qapi_Status_t qc_drv_wlan_set_drv_reg_query(qc_drv_context *qc_drv_ctx, uint8_t device_Id,
        qapi_WLAN_Driver_RegQuery_Params_t *query, size_t size, int flag);


qapi_Status_t qc_drv_wlan_set_ap_channel_switch(qc_drv_context *qc_drv_ctx, uint8_t device_Id,
        qapi_WLAN_Channel_Switch_t *chnl, size_t size, int flag);


qapi_Status_t qc_drv_wlan_set_event_filter(qc_drv_context *qc_drv_ctx, uint8_t device_Id,
        qapi_WLAN_Event_Filter_t *filter, size_t size, int flag);


qapi_Status_t qc_drv_wlan_set_pw_mode_policy(qc_drv_context *qc_drv_ctx, uint8_t device_Id,
        qapi_WLAN_Power_Policy_Params_t *pm, size_t size, int flag);


qapi_Status_t qc_drv_wlan_set_enable_roaming(qc_drv_context *qc_drv_ctx, uint8_t device_Id,
        uint32_t *roaming, size_t size, int flag);


qapi_Status_t qc_drv_wlan_set_enable_green_tx(qc_drv_context *qc_drv_ctx, uint8_t device_Id,
        uint32_t *entx, size_t size, int flag);


qapi_Status_t qc_drv_wlan_set_low_pw_listen(qc_drv_context *qc_drv_ctx, uint8_t device_Id,
        uint32_t *enlpw, size_t size, int flag);


qapi_Status_t qc_drv_wlan_set_enable_wake_wireless(qc_drv_context *qc_drv_ctx, uint8_t device_Id,
        uint32_t *wake, size_t size, int flag);


qapi_Status_t qc_drv_wlan_set_enable_pkt_filter(qc_drv_context *qc_drv_ctx, uint8_t device_Id,
        uint32_t *filter, size_t size, int flag);


qapi_Status_t qc_drv_wlan_set_add_pattern(qc_drv_context *qc_drv_ctx, uint8_t device_Id,
        qapi_WLAN_Add_Pattern_t *pattern, size_t size, int flag);


qapi_Status_t qc_drv_wlan_set_delete_pattern(qc_drv_context *qc_drv_ctx, uint8_t device_Id,
        qapi_WLAN_Delete_Pattern_t *pattern, size_t size, int flag);


qapi_Status_t qc_drv_wlan_set_change_def_filter_action(qc_drv_context *qc_drv_ctx, uint8_t device_Id,
        qapi_WLAN_Change_Default_Filter_Action_t *filter, size_t size, int flag);


#endif
