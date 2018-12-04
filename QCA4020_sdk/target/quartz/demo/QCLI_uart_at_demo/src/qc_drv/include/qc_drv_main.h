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

#ifndef __QC_DRV_MAIN_H
#define __QC_DRV_MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <qcli.h>
#include <qcli_api.h>
#include "qurt_thread.h"
#include "qurt_timer.h"
#include "qurt_error.h"
#include "qurt_mutex.h"
#include "qurt_signal.h"
#include "qapi_fatal_err.h"
#include "qapi_wlan.h"
#include "qc_util.h"

#if defined(ENABLE_PER_FN_PROFILING)
#include "qapi_cpuprofile.h"
#endif

#include "qapi_wlan_base.h"
#include "qapi_ble_bttypes.h"
#include "qapi_wlan_p2p.h"

#define P2P_STANDARD_TIMEOUT (300)
#include "qapi_socket.h"
#include "qapi_dnsc.h"
#include "qapi_sntpc.h"
#include "qapi_wlan_bridge.h"
#include "qapi_ns_utils.h"
#include "qapi_netservices.h"
#include "qapi_ns_gen_v4.h"
#include "qapi_dhcpv6c.h"
#include "qapi_httpsvr.h"
#include "qapi_ble.h"
#include "qapi_mqttc.h"
#include "qapi_firmware_upgrade.h"

#include <qapi_ssl.h>

#include "qapi_zb.h"
#include "qapi_zb_cl.h"
#include "qapi_zb_cl_identify.h"
#include "qapi_zb_nwk.h"
#include "qapi_zb_bdb.h"
#include "qapi_zb_zdp.h"
#include "qapi_zb_cl_basic.h"
#include "qapi_zb_cl_identify.h"
#include "qapi_persist.h"
#include "pal.h"
#include "qapi_zb_cl_scenes.h"
#include "qapi_zb_cl_onoff.h"
#include "qapi_zb_cl_level_control.h"
#include "qapi_zb_cl_color_control.h"
#include "qapi_zb_cl_time.h"
#include "qapi_zb_cl_alarm.h"
#include "qapi_zb_cl_groups.h"
#include "qapi_netprofile.h"
#include "qapi_omtm.h"
#include "qapi_httpc.h"
#include "qapi_zb_cl_touchlink.h"

#include "qapi_twn.h"

#define is_drv_cb_valid(cb) (qc_drv_ctx && qc_drv_ctx->drv_ops && qc_drv_ctx->drv_ops->cb)
#define APP_STRCMP(_a, _b) strcmp((char const*)(_a), (const char *)(_b))

typedef struct qc_drv_cb {
    char *name;
    char *desc;

#ifndef CONFIG_PLATFORM_CDB24
    /** Start of wlan */
    qapi_Status_t (*qc_drv_wlan_enable)();

    qapi_Status_t (*qc_drv_wlan_disable)();

    qapi_Status_t (*qc_drv_wlan_get_op_mode)(uint32_t deviceId,
            uint32_t *wifimode, uint32_t *dataLen);

    qapi_Status_t (*qc_drv_wlan_set_encr_type)(uint32_t deviceId, void *encrType,
            int size, int flag);

    qapi_Status_t (*qc_drv_wlan_set_auth_mode)(uint32_t deviceId,
            qapi_WLAN_Auth_Mode_e *authMode,
            int size, int flag);

    qapi_Status_t (*qc_drv_wlan_set_op_mode)(uint32_t deviceId,
            qapi_WLAN_Dev_Mode_e *opMode, int size, int flag);

    qapi_Status_t (*qc_drv_wlan_get_mac_addr)(uint32_t deviceId,
            uint8_t *own_Interface_Addr, uint32_t *dataLen);

    qapi_Status_t (*qc_drv_wlan_set_mac_addr)(uint32_t deviceId,
            uint8_t *own_Interface_Addr, uint32_t dataLen);

    qapi_Status_t (*qc_drv_wlan_set_ssid)(uint32_t deviceId,
            uint8_t *ssid, uint32_t ssid_len, uint8_t flag);

    qapi_Status_t (*qc_drv_wlan_set_power_param)(uint32_t deviceId,
            void *pwrMode, int size, int flag);

    qapi_Status_t (*qc_drv_wlan_set_passphrase)(uint32_t deviceId,
            void *passphrase, uint32_t passphrase_len, uint8_t flag);

    qapi_Status_t (*qc_drv_wlan_set_security_pmk)(uint32_t deviceId,
            void *passphrase, uint32_t passphrase_len, uint8_t flag);

    qapi_Status_t (*qc_drv_wlan_set_wps_credentials)(uint32_t deviceId,
            qapi_WLAN_WPS_Credentials_t *wpsCred, uint32_t size, uint8_t flag);

    qapi_Status_t (*qc_drv_wlan_set_channel)(uint32_t deviceId,
            uint32_t *channel_val, uint32_t size, uint8_t flag);

    qapi_Status_t (*qc_drv_wlan_set_listen_interval)(uint32_t deviceId,
            int32_t *listen_time, uint32_t size, uint8_t flag);


    qapi_Status_t (*qc_drv_wlan_set_sta_keep_alive)(uint32_t deviceId,
            int32_t *keep_alive, uint32_t size, uint8_t flag);

    qapi_Status_t (*qc_drv_wlan_set_rssi_threshold)(uint32_t deviceId,
            qapi_WLAN_Rssi_Threshold_Params_t *qrthresh, uint32_t size, uint8_t flag);

    qapi_Status_t (*qc_drv_wlan_set_probe_req_fwd_to_host)(uint32_t deviceId,
            int32_t *enable, uint32_t size, uint8_t flag);

    qapi_Status_t (*qc_drv_wlan_set_wep_key_index)(uint32_t deviceId,
            int32_t *key_idx, uint32_t size, uint8_t flag);

    qapi_Status_t (*qc_drv_wlan_set_wep_key_pair)(uint32_t deviceId,
            qapi_WLAN_Security_Wep_Key_Pair_Params_t *keyPair, uint32_t size, uint8_t flag);

    qapi_Status_t (*qc_drv_wlan_set_rate)(uint32_t deviceId,
            qapi_WLAN_Bit_Rate_t *rateIndex, uint32_t size, uint8_t flag);

    qapi_Status_t (*qc_drv_wlan_set_ap_wps_flag)(uint32_t deviceId,
            uint8_t *wps_flag, uint32_t size, uint8_t flag);

    qapi_Status_t (*qc_drv_wlan_set_tx_power_in_dbm)(uint32_t deviceId,
            int32_t *power_in_dBm, uint32_t size, uint8_t flag);

    qapi_Status_t (*qc_drv_wlan_set_phy_mode)(uint32_t deviceId,
            qapi_WLAN_Phy_Mode_e *phyMode, uint32_t size, uint8_t flag);

    qapi_Status_t (*qc_drv_wlan_set_ht_config)(uint32_t deviceId,
            qapi_WLAN_11n_HT_Config_e *htconfig, uint32_t size, uint8_t flag);

    qapi_Status_t (*qc_drv_wlan_set_ap_enable_hidden_mode)(uint32_t deviceid,
            uint8_t *hidden_flag, uint32_t size, uint8_t flag);

    qapi_Status_t (*qc_drv_wlan_set_beacon_interval_in_tu)(uint32_t deviceid,
            uint32_t *beacon_int_in_tu, uint32_t size, uint8_t flag);

    qapi_Status_t (*qc_drv_wlan_set_ap_dtim_interval)(uint32_t deviceid,
            uint32_t *dtim_period, uint32_t size, uint8_t flag);

    qapi_Status_t (*qc_drv_wlan_set_country_code)(uint32_t deviceid,
            char *country_code, uint32_t size, uint8_t flag);

    qapi_Status_t (*qc_drv_wlan_set_ap_country_code)(uint32_t deviceid,
            char *country_code, uint32_t size, uint8_t flag);

    qapi_Status_t (*qc_drv_wlan_get_power_param)(uint32_t deviceId,
            uint32_t *mode, uint32_t *dataLen);

    qapi_Status_t (*qc_drv_wlan_add_device)(uint8_t device_ID);

    qapi_Status_t (*qc_drv_wlan_remove_device)(uint8_t device_ID);

    qapi_Status_t (*qc_drv_wlan_start_scan)(uint8_t device_ID,
            const qapi_WLAN_Start_Scan_Params_t  *scan_Params,
            qapi_WLAN_Store_Scan_Results_e store_Scan_Results);

    qapi_Status_t (*qc_drv_wlan_get_scan_results)(uint8_t deviceId,
            qapi_WLAN_BSS_Scan_Info_t  *scan_Info,
            int16_t  *num_Results);

    qapi_Status_t (*qc_drv_wlan_get_phy_mode)(uint8_t device_ID,
            qapi_WLAN_Phy_Mode_e *phyMode,
            uint32_t  *dataLen);

    qapi_Status_t (*qc_drv_wlan_get_ht_config)(uint8_t device_ID,
            qapi_WLAN_11n_HT_Config_e *htConfig,
            uint32_t  *dataLen);

    qapi_Status_t (*qc_drv_wlan_get_ssid)(uint8_t device_ID,
            void * ssid,
            uint32_t  *dataLen);

    qapi_Status_t (*qc_drv_wlan_get_channel)(uint8_t deviceId,
            uint16_t *channel_val,
            uint32_t  *dataLen);

    qapi_Status_t (*qc_drv_wlan_get_wep_key_pair)(uint8_t deviceId,
            qapi_WLAN_Security_Wep_Key_Pair_Params_t *key_pair,
            uint32_t  *dataLen);

    qapi_Status_t (*qc_drv_wlan_get_wep_key_index)(uint8_t device_ID,
            uint32_t *key_index,
            uint32_t  *dataLen);

    qapi_Status_t (*qc_drv_wlan_get_rssi)(uint8_t device_ID,
            uint8_t *rssi,
            uint32_t  *dataLen);

    qapi_Status_t (*qc_drv_wlan_get_reg_domain_channel_list)(uint8_t deviceId,
            qapi_WLAN_Get_Channel_List_t *wlanChannelList,
            uint32_t  *dataLen);

    qapi_Status_t (*qc_drv_wlan_get_wireless_stats)(uint8_t device_ID,
            qapi_WLAN_Get_Statistics_t *getStats,
            uint32_t  *dataLen);

    qapi_Status_t (*qc_drv_wlan_get_rate)(uint8_t device_ID,
            int32_t *rate_index,
            uint32_t  *dataLen);

    qapi_Status_t (*qc_drv_wlan_get_firmware_version)(uint8_t device_ID,
            qapi_WLAN_Firmware_Version_String_t *versionstr,
            uint32_t  *dataLen);


    qapi_Status_t (*qc_drv_wlan_get_reg_domain)(uint8_t device_ID,
            uint32_t *regDomain,
            uint32_t  *dataLen);

    qapi_Status_t (*qc_drv_wlan_get_last_error)(uint8_t device_ID,
            int32_t *err,
            uint32_t  *dataLen);

    qapi_Status_t (*qc_drv_wlan_get_tx_status)(uint8_t device_ID,
            uint32_t *tx_status,
            uint32_t  *dataLen);

    qapi_Status_t (*qc_drv_wlan_set_callback)(uint8_t device_ID,
            qapi_WLAN_Callback_t callback,
            const void *application_Context);

    qapi_Status_t (*qc_drv_wlan_wps_connect)(uint8_t device_ID);

    qapi_Status_t (*qc_drv_wlan_wps_await_completion)(uint32_t device_ID,
            qapi_WLAN_Netparams_t *net_Params);

    qapi_Status_t (*qc_drv_wlan_disconnect)(uint8_t device_ID);

    qapi_Status_t (*qc_drv_wlan_commit)(uint8_t device_ID);

    qapi_Status_t (*qc_drv_wlan_raw_send)(uint8_t device_ID,
            const qapi_WLAN_Raw_Send_Params_t  *raw_Params);

    qapi_Status_t (*qc_drv_wlan_suspend_start)(uint32_t suspend_Time_In_Ms);

    qapi_Status_t (*qc_drv_wlan_wps_start)(uint8_t device_ID,
            qapi_WLAN_WPS_Connect_Action_e connect_Action,
            qapi_WLAN_WPS_Mode_e mode,
            const char  *pin);

    qapi_Status_t (*qc_drv_wlan_set_get_auth_mode)(uint8_t device_ID,
            void *authMode, size_t size, int flag);

    qapi_Status_t (*qc_drv_wlan_promiscuous_mode)(uint8_t device_ID,
            void *mode, size_t size, int flag);

    qapi_Status_t (*qc_drv_wlan_force_assert)(uint8_t device_ID,
            void *data, size_t size, int flag);

    qapi_Status_t (*qc_drv_wlan_scan_param)(uint8_t device_ID,
            void *scanParam, size_t size, int flag);

    qapi_Status_t (*qc_drv_wlan_set_inactivity_time)(uint8_t device_ID,
            void *time, size_t size, int flag);

    qapi_Status_t (*qc_drv_wlan_set_ap_ps_buf)(uint8_t device_ID,
            void *ps_val, size_t size, int flag);

    qapi_Status_t (*qc_drv_wlan_set_ap_enable_uapsd)(uint8_t device_ID,
            void *uapsd, size_t size, int flag);

    qapi_Status_t (*qc_drv_wlan_set_sta_uapsd)(uint8_t device_ID,
            void *uapsd, size_t size, int flag);

    qapi_Status_t (*qc_drv_wlan_set_sta_max_sp_len)(uint8_t device_ID,
            void *maxsp, size_t size, int flag);

    qapi_Status_t (*qc_drv_wlan_set_wmm_config)(uint8_t device_ID,
            void *wmm, size_t size, int flag);

    qapi_Status_t (*qc_drv_wlan_set_ap_bss_mip)(uint8_t device_ID,
            void *period, size_t size, int flag);

    qapi_Status_t (*qc_drv_wlan_set_sta_sp)(uint8_t device_ID,
            void *period, size_t size, int flag);

    qapi_Status_t (*qc_drv_wlan_set_ap_sr)(uint8_t device_ID,
            void *response, size_t size, int flag);

    qapi_Status_t (*qc_drv_wlan_set_app_ie)(uint8_t device_ID,
            void *ie_params, size_t size, int flag);

    qapi_Status_t (*qc_drv_wlan_set_sta_bmiss_config)(uint8_t device_ID,
            void *bmiss, size_t size, int flag);

    qapi_Status_t (*qc_drv_wlan_set_suspend_resume)(uint8_t device_ID,
            void *suspend, size_t size, int flag);

    qapi_Status_t (*qc_drv_wlan_set_ka_offload_scfg)(uint8_t device_Id,
            qapi_WLAN_TCP_Offload_Config_Params_t *cfg, size_t size, int flag);

    qapi_Status_t (*qc_drv_wlan_set_ka_offload_enable)(uint8_t device_Id,
            qapi_WLAN_TCP_Offload_Enable_t *enable, size_t size, int flag);

    qapi_Status_t (*qc_drv_wlan_set_nw_offload_enable)(uint8_t device_Id,
            qapi_WLAN_Preferred_Network_Offload_Config_t *param, size_t size, int flag);

    qapi_Status_t (*qc_drv_wlan_set_nw_profile)(uint8_t device_Id,
            qapi_WLAN_Preferred_Network_Profile_t *param, size_t size, int flag);

    qapi_Status_t (*qc_drv_wlan_set_aggr_tid)(uint8_t device_Id,
            qapi_WLAN_Aggregation_Params_t *param, size_t size, int flag);

    qapi_Status_t (*qc_drv_wlan_set_aggrx_config)(uint8_t device_Id,
            qapi_WLAN_Rx_Aggrx_Params_t *param, size_t size, int flag);

    qapi_Status_t (*qc_drv_wlan_set_pktlog_enable)(uint8_t device_Id,
            qapi_WLAN_Pktlog_Enable_t *pktlog, size_t size, int flag);

    qapi_Status_t (*qc_drv_wlan_set_pktlog_start)(uint8_t device_Id,
            qapi_WLAN_Pktlog_Start_Params_t *pktlog, size_t size, int flag);

    qapi_Status_t (*qc_drv_wlan_set_dblog_enable)(uint8_t device_Id,
            qapi_WLAN_Dbglog_Enable_t *dbglog, size_t size, int flag);

    qapi_Status_t (*qc_drv_wlan_set_dblog_config)(uint8_t device_Id,
            qapi_WLAN_Dbglog_Config_t *config, size_t size, int flag);

    qapi_Status_t (*qc_drv_wlan_set_dblog_module_conf)(uint8_t device_Id,
            qapi_WLAN_Dbglog_Module_Config_t *config, size_t size, int flag);

    qapi_Status_t (*qc_drv_wlan_set_drv_reg_query)(uint8_t device_Id,
            qapi_WLAN_Driver_RegQuery_Params_t *query, size_t size, int flag);

    qapi_Status_t (*qc_drv_wlan_set_ap_channel_switch)(uint8_t device_Id,
            qapi_WLAN_Channel_Switch_t *chnl, size_t size, int flag);

    qapi_Status_t (*qc_drv_wlan_set_event_filter)(uint8_t device_Id,
            qapi_WLAN_Event_Filter_t *filter, size_t size, int flag);

    qapi_Status_t (*qc_drv_wlan_set_pw_mode_policy)(uint8_t device_Id,
            qapi_WLAN_Power_Policy_Params_t *pm, size_t size, int flag);

    qapi_Status_t (*qc_drv_wlan_set_enable_roaming)(uint8_t device_Id,
            uint32_t *roaming, size_t size, int flag);

    qapi_Status_t (*qc_drv_wlan_set_enable_green_tx)(uint8_t device_Id,
            uint32_t *entx, size_t size, int flag);

    qapi_Status_t (*qc_drv_wlan_set_low_pw_listen)(uint8_t device_Id,
            uint32_t *enlpw, size_t size, int flag);

    qapi_Status_t (*qc_drv_wlan_set_enable_wake_wireless)(uint8_t device_Id,
            uint32_t *wake, size_t size, int flag);

    qapi_Status_t (*qc_drv_wlan_set_enable_pkt_filter)(uint8_t device_Id,
            uint32_t *filter, size_t size, int flag);

    qapi_Status_t (*qc_drv_wlan_set_add_pattern)(uint8_t device_Id,
            qapi_WLAN_Add_Pattern_t *pattern, size_t size, int flag);

    qapi_Status_t (*qc_drv_wlan_set_delete_pattern)(uint8_t device_Id,
            qapi_WLAN_Delete_Pattern_t *pattern, size_t size, int flag);

    qapi_Status_t (*qc_drv_wlan_set_change_def_filter_action)(uint8_t device_Id,
            qapi_WLAN_Change_Default_Filter_Action_t *filter, size_t size, int flag);


    /** Start of p2p */
    qapi_Status_t (*qc_drv_p2p_enable)(uint32_t deviceId, int flag);

    qapi_Status_t (*qc_drv_p2p_disable)(uint32_t deviceId, int flag);

    qapi_Status_t (*qc_drv_p2p_set_config)(uint32_t deviceId,
            qapi_WLAN_P2P_Config_Params_t *p2pConfig,
            int size, int flag);

    qapi_Status_t (*qc_drv_p2p_connect)(uint32_t deviceId, uint8_t wps_Method,
            uint8_t *peer_Addr, uint8_t p2p_pers_go);

    qapi_Status_t (*qc_drv_p2p_provision)(uint32_t deviceId,
            uint16_t wps_Method, uint8_t *peer);

    qapi_Status_t (*qc_drv_p2p_listen)(uint32_t deviceId, uint32_t timeout_val);

    qapi_Status_t (*qc_drv_p2p_cancel)(uint32_t deviceId);

    qapi_Status_t (*qc_drv_p2p_join)(uint32_t deviceId,
            uint8_t wps_Method, uint8_t *p2p_join_mac_addr,
            char *p2p_wps_pin, uint16_t go_Oper_Freq);

    qapi_Status_t (*qc_drv_p2p_auth)(uint32_t deviceId, uint8_t dev_Auth,
            qapi_WLAN_P2P_WPS_Method_e wps_Method,
            uint8_t *peer_Addr, uint8_t p2p_persistent_go);

    qapi_Status_t (*qc_drv_p2p_start_go)(uint32_t deviceId,
            qapi_WLAN_P2P_Go_Params_t *goParams,
            int32_t go_chan, uint8_t persistent_Group);

    qapi_Status_t (*qc_drv_p2p_invite)(uint32_t deviceId, const char *string,
            uint8_t wps_Method, uint8_t *peer_Addr,
            uint8_t is_Persistent, uint8_t p2p_invite_role);

    qapi_Status_t (*qc_drv_p2p_get_node_list)(uint32_t deviceId,
            qapi_WLAN_P2P_Node_List_Params_t *p2pNodeList,
            uint32_t *dataLen);

    qapi_Status_t (*qc_drv_p2p_get_network_list)(uint32_t deviceId,
            qapi_WLAN_P2P_Network_List_Params_t *p2pNetworkList,
            uint32_t *dataLen);

    qapi_Status_t (*qc_drv_p2p_set_oops_params)(uint32_t deviceId,
            qapi_WLAN_P2P_Opps_Params_t *opps,
            int size, int flag);

    qapi_Status_t (*qc_drv_p2p_set_noa_params)(uint32_t deviceId,
            qapi_WLAN_P2P_Noa_Params_t *noaParams, int size, int flag);

    qapi_Status_t (*qc_drv_p2p_set_operating_class)(uint32_t deviceId,
            qapi_WLAN_P2P_Config_Params_t *p2pConfig,
            int size, int flag);

    qapi_Status_t (*qc_drv_p2p_stop_find)(uint32_t deviceId);

    qapi_Status_t (*qc_drv_p2p_set_ssidparam)(uint32_t deviceId,
            qapi_WLAN_P2P_Go_Params_t *ssidParams,
            int size, int flag);

    qapi_Status_t (*qc_drv_p2p_set)(uint32_t deviceId,
            uint8_t config_Id, int *val, uint32_t len, int flag);

    qapi_Status_t (*qc_drv_p2p_set_param)(uint32_t deviceId,
            uint8_t *p2pmode, int size, int flag);

    qapi_Status_t (*qc_drv_p2p_find)(uint32_t deviceId,
            uint8_t type, uint32_t timeout);

    qapi_Status_t (*qc_drv_p2p_invite_auth)(uint8_t device_Id,
            const qapi_WLAN_P2P_Invite_Info_t  *invite_Info);


    /* Start of Mqtt */
    qapi_Status_t (*qc_drv_net_mqtt_init)(char *ca_file);

    qapi_Status_t (*qc_drv_net_mqtt_shutdown)(void);

    qapi_Status_t (*qc_drv_net_mqtt_new)(const char *client_id, qbool_t clean_session,
            qapi_Status_t *status);

    qapi_Status_t (*qc_drv_net_mqtt_destroy)(int32_t handle);

    qapi_Status_t (*qc_drv_net_mqtt_set_username_password)(int32_t handle, const char *user,
            size_t user_len, const char *pw, size_t pw_len, qapi_Status_t *status);

    qapi_Status_t (*qc_drv_net_mqtt_set_will)(int32_t handle, const char *topic,
            size_t topic_len, const char *msg, size_t msg_len, uint32_t qos,
            qbool_t retained, qapi_Status_t *status);

    qapi_Status_t (*qc_drv_net_mqtt_set_keep_alive)(int32_t handle, uint16_t keepalive_sec);

    qapi_Status_t (*qc_drv_net_mqtt_set_connack_wait_time)(int32_t handle,
            uint16_t max_conn_pending_sec);

    qapi_Status_t (*qc_drv_net_mqtt_set_ssl_config)(int32_t handle,
            qapi_Net_SSL_Config_t *mqttc_sslcfg, qapi_Status_t *status);

    qapi_Status_t (*qc_drv_net_mqtt_connect)(int32_t handle, char *host, qbool_t secure_session,
            qbool_t nonblocking_connect, char *bind_if, qapi_Status_t *status);

    qapi_Status_t (*qc_drv_net_mqtt_subscribe)(int32_t handle, const char *topic, uint32_t qos,
            qapi_Status_t *status);

    qapi_Status_t (*qc_drv_net_mqtt_publish)(int32_t handle, const char *topic,
            size_t topic_len, const char *msg, uint32_t msg_len, uint32_t qos,
            qbool_t retained, qbool_t dup, qapi_Status_t *status);

    qapi_Status_t (*qc_drv_net_mqtt_unsubscribe)(int32_t handle, const char *topic,
            qapi_Status_t *status);

    qapi_Status_t (*qc_drv_net_mqtt_disconnect)(int32_t handle, qapi_Status_t *status);


    /* Start of Wi-Fi OTA */
    qapi_Status_t (*qc_drv_Fw_Upgrade_Get_Image_ID)(qapi_Part_Hdl_t hdl,
            uint32_t *result);
    qapi_Status_t (*qc_drv_Fw_Upgrade_Get_Image_Version)(qapi_Part_Hdl_t hdl,
            uint32_t *result);
    qapi_Status_t (*qc_drv_Fw_Upgrade_Get_Partition_Start)(qapi_Part_Hdl_t hdl,
            uint32_t *result);
    qapi_Status_t (*qc_drv_Fw_Upgrade_Get_Partition_Size)(qapi_Part_Hdl_t hdl,
            uint32_t *size);
    qapi_Status_t (*qc_drv_Fw_Upgrade_Read_Partition)(qapi_Part_Hdl_t hdl,
            int32_t val, char *buf, int32_t buf_size, uint32_t *size);
    qapi_Status_t (*qc_drv_Fw_Upgrade_init)(void);
    qapi_Status_t (*qc_drv_Fw_Upgrade_Get_Active_FWD)(uint32_t *boot_type,
            uint32_t *fwd_present);
    qapi_Status_t (*qc_drv_Fw_Upgrade_Get_FWD_Magic)(int32_t Index,
            uint32_t *magic);
    qapi_Status_t (*qc_drv_Fw_Upgrade_Get_FWD_Rank)(int32_t Index,
            uint32_t *Result_u32);
    qapi_Status_t (*qc_drv_Fw_Upgrade_Get_FWD_Version)(int32_t Index,
            uint32_t *Result_u32);
    qapi_Status_t (*qc_drv_Fw_Upgrade_Get_FWD_Status)(int32_t Index,
            uint8_t *Result_u8);
    qapi_Status_t (*qc_drv_Fw_Upgrade_Get_FWD_Total_Images)(int32_t Index,
            uint8_t *Result_u8);
    qapi_Status_t (*qc_drv_Fw_Upgrade_First_Partition)(int32_t Index,
            qapi_Part_Hdl_t *hdl);
    qapi_Status_t (*qc_drv_Fw_Upgrade_Next_Partition)(qapi_Part_Hdl_t hdl,
            qapi_Part_Hdl_t *hdl_next);
    qapi_Status_t (*qc_drv_Fw_Upgrade_Close_Partition)(qapi_Part_Hdl_t hdl);
    qapi_Status_t (*qc_drv_Fw_Upgrade_Erase_FWD)(int32_t fwd_num);
    qapi_Status_t (*qc_drv_Fw_Upgrade_Done)(int32_t accept, int32_t flags);
    qapi_Status_t (*qc_drv_Fw_Upgrade_Find_Partition)(uint8_t Index,
            int32_t img_id, qapi_Part_Hdl_t *hdl);
    qapi_Status_t (*qc_drv_Fw_Upgrade)(char *iface_name,
            qapi_Fw_Upgrade_Plugin_t *plugin, char *url, char *cfg_file,
            int32_t flags, qapi_Fw_Upgrade_CB_t cb, void *init_param);
#endif

    /** Start of Net */
    int32_t (*qc_drv_net_SNTPc_get_srvr_list)(
            qapi_Net_SNTP_Server_List_t *svr_list);

    int32_t (*qc_drv_net_SNTPc_start)(void);

    int32_t (*qc_drv_net_SNTPc_add_srvr)(char *name, uint32_t ID);

    int32_t (*qc_drv_net_SNTPc_del_srvr)(uint32_t id);

    int32_t (*qc_drv_net_SNTPc_cmd)(qapi_Net_SNTP_Command_t sntp_flag);

    int32_t (*qc_drv_net_SNTPc_get_broke_down_time)(qapi_Net_SNTP_Tm_t *tm);

    int32_t (*qc_drv_net_DNSc_start)(void);

    int32_t (*qc_drv_net_DNSc_get_srvr_list)(qapi_Net_DNS_Server_List_t *svr_list);

    int32_t (*qc_drv_net_DNSc_cmd) (qapi_Net_DNS_Command_t dns_flag);

    int32_t (*qc_drv_net_DNSc_add_srvr) (char *svr, uint32_t id);

    int32_t (*qc_drv_net_DNSc_del_srvr) (uint32_t id);

    int32_t (*qc_drv_net_DNSc_Reshost)(char *hostname,
            struct ip46addr *ipaddr);

    int32_t (*qc_drv_net_errno)(int32_t sock_id);

    int32_t (*qc_drv_net_Ping)(uint32_t ipv4_Addr,
            uint32_t size);
    int32_t (*qc_drv_net_Get_All_Ifnames) (qapi_Net_Ifnameindex_t *if_Name_Index);
    int32_t (*qc_drv_net_Interface_Exist) (const char * interface_Name,
            qbool_t *if_Is_Up);
    int32_t (*qc_drv_net_IPv4_Config) (const char *interface_Name,
            qapi_Net_IPv4cfg_Command_t cmd, uint32_t *ipv4_Addr,
            uint32_t *subnet_Mask,uint32_t *gateway);
    int32_t (*qc_drv_net_DHCPv4c_Register_Success_Callback) (
            const char * interface_Name, qapi_Net_DHCPv4c_Success_CB_t CB);
    int32_t (*qc_drv_net_DHCPv4c_Release) (const char *interface_Name);
    int32_t (*qc_drv_net_Ping6) (uint8_t ipv6_Addr[16], uint32_t size,
            const char * interface_Name);
    int32_t (*qc_drv_net_DHCPv4s_Register_Success_Callback) (
            const char * interface_Name, qapi_Net_DHCPv4s_Success_CB_t CB);
    int32_t (*qc_drv_net_DHCPv4s_Set_Pool) (const char *interface_Name,
            uint32_t start_IP, uint32_t end_IP, uint32_t lease_Time);

    int32_t (*qc_drv_net_DHCPv6c_Register_New_IPv6_Lease_Complete_Callback) (const char * interface_Name, qapi_Net_DHCPv6c_New_IPv6_Lease_Complete_CB_t cb, void * cb_Ctxt);
    int32_t (*qc_drv_net_DHCPv6c_Register_Release_IPv6_Lease_Complete_Callback)(const char * interface_Name, qapi_Net_DHCPv6c_Release_IPv6_Lease_Complete_CB_t cb, void *cb_Ctxt);
    int32_t (*qc_drv_net_DHCPv6c_Enable) (const char *interface_Name);
    int32_t (*qc_drv_net_DHCPv6c_Disable) (const char *interface_Name);
    int32_t (*qc_drv_net_DHCPv6c_Register_New_Prefix_Lease_Complete_Callback) (const char * interface_Name,  qapi_Net_DHCPv6c_New_Prefix_Lease_Complete_CB_t cb, void *cb_Ctxt);
    int32_t (*qc_drv_net_DHCPv6c_Register_Release_Prefix_Lease_Complete_Callback) (const char * interface_Name, qapi_Net_DHCPv6c_Release_Prefix_Lease_Complete_CB_t cb, void *cb_Ctxt);
    int32_t (*qc_drv_net_DHCPv6c_New_Lease)(const char * dhcpv6c_Client_Interface_Name, const char * interface_Name);
    int32_t (*qc_drv_net_DHCPv6c_Release_Lease)(const char * dhcpv6c_Client_Interface_Name, const char * interface_Name);
    int32_t (*qc_drv_net_DHCPv6c_Confirm_Lease)(const char * dhcpv6c_Client_Interface_Name, const char * interface_Name);
    int32_t (*qc_drv_net_Bridge_Show_MACs) (qapi_Net_Bridge_DB_Entry_t **ptr, uint32_t *count);
    int32_t (*qc_drv_net_Bridge_Set_Aging_Timeout) (int32_t Integer_Value);
    int32_t (*qc_drv_net_Bridge_Enable) (uint8_t enable);
    int32_t (*qc_drv_net_HTTPs_Set_Buffer_Size) (uint16_t txbufsize, uint16_t rxbufsize);
    int32_t (*qc_drv_net_HTTPs_Unregister_Content_Type) (char *String_Value, int String_len);
    int32_t (*qc_drv_net_HTTPs_Register_Content_Type) (char *String_Value, int String_len);
    int32_t (*qc_drv_net_HTTPs_Shutdown) ();
    int32_t (*qc_drv_net_HTTPs_Stop) ();
    int32_t (*qc_drv_net_HTTPs_Start) ();
    int32_t (*qc_drv_net_HTTPs_Set_SSL_Config) (qapi_Net_SSL_Config_t * httpsvr_sslcfg);
    int32_t (*qc_drv_net_HTTPs_Init) (qapi_Net_HTTPs_Config_t *cfg);
    int32_t (*qc_api_get_HTTPs_Is_Started) ();
    int32_t (*qc_drv_net_HTTPs_Get_Status) (qapi_Net_HTTPs_Status_t *status);
    int32_t (*qc_drv_net_Interface_Get_Physical_Address) (const char *interface_Name, const uint8_t **address, uint32_t *address_Len);
    int32_t (*qc_drv_net_IPv6_Get_Address)(const char *interface_Name,
            uint8_t *link_Local,
            uint8_t *global,
            uint8_t *default_Gateway,
            uint8_t *global_Second,
            uint32_t *link_Local_Prefix,
            uint32_t *global_Prefix,
            uint32_t *default_Gateway_Prefix,
            uint32_t *global_Second_Prefix);
    int32_t (*qc_drv_net_socketclose) (int32_t handle);
    int32_t (*qc_drv_net_socket) (int32_t family, int32_t type, int32_t protocol);
    int32_t (*qc_drv_net_connect) (int32_t handle, struct sockaddr *srvaddr, int32_t addrlen);
    int32_t (*qc_drv_net_send) (int32_t handle, char *buf, int32_t len, int32_t flags);
    int32_t (*qc_drv_net_recv)(int32_t handle, char *buf, int32_t len, int32_t flags);
    int32_t (*qc_drv_net_recvfrom)(int32_t handle, char *buf, int32_t len, int32_t flags, struct sockaddr *from, int32_t *fromlen);
    int32_t (*qc_drv_net_sendto)(int32_t handle, char *buf, int32_t len, int32_t flags, struct sockaddr *to, int32_t tolen);
    int32_t (*qc_drv_net_bind)(int32_t handle, struct sockaddr *addr, int32_t addrlen);
    int32_t (*qc_drv_net_listen)(int32_t handle, int32_t backlog);
    int32_t (*qc_drv_net_accept) (int32_t handle, struct sockaddr *cliaddr, int32_t *addrlen);

    int32_t (*qc_drv_net_IPv6_Get_Scope_ID) (const char *interface_Name, int32_t *scope_ID);
    int32_t (*qc_drv_net_setsockopt) (int32_t handle, int32_t level, int32_t optname, void *optval, int32_t optlen);
    int32_t (*qc_drv_net_SSL_Write_To) (qapi_Net_SSL_Con_Hdl_t ssl, void *buf, uint32_t num,
            struct sockaddr *to, int32_t to_Len);
    int32_t (*qc_drv_net_select) (qapi_fd_set_t *rd, qapi_fd_set_t *wr, qapi_fd_set_t *ex, int32_t timeout_ms);
    int32_t (*qc_drv_net_SSL_Read) (qapi_Net_SSL_Con_Hdl_t hdl, void *buf, uint32_t size);
    void *(*qc_drv_net_Buf_Alloc) (uint32_t size, uint32_t id);
    int32_t (*qc_drv_net_fd_zero) (qapi_fd_set_t *set);
    int32_t (*qc_drv_net_fd_set) (int32_t handle, qapi_fd_set_t *set);
    int32_t (*qc_drv_net_Buf_Free) (void *buf, uint32_t id);

    int32_t (*qc_drv_net_fd_clr)(int32_t handle, qapi_fd_set_t *set);
    int32_t (*qc_drv_net_fd_isset)(int32_t handle, qapi_fd_set_t *set);
    int32_t (*qc_drv_net_SSL_Obj_New)(qapi_Net_SSL_Role_t role);
    int32_t (*qc_drv_net_SSL_Shutdown)(qapi_Net_SSL_Con_Hdl_t ssl);
    int32_t (*qc_drv_net_SSL_Obj_Free)(qapi_Net_SSL_Obj_Hdl_t hdl);
    int32_t (*qc_drv_net_SSL_Cipher_Add)(qapi_Net_SSL_Config_t * cfg, uint16_t cipher);
    int32_t (*qc_drv_net_SSL_ALPN_Protocol_Add)(qapi_Net_SSL_Obj_Hdl_t hdl, const char *protocol);
    int32_t (*qc_drv_net_SSL_Con_New)(qapi_Net_SSL_Obj_Hdl_t hdl, qapi_Net_SSL_Protocol_t prot);
    int32_t (*qc_drv_net_SSL_Configure)(qapi_Net_SSL_Con_Hdl_t ssl, qapi_Net_SSL_Config_t *cfg);
    int32_t (*qc_drv_net_SSL_Cert_Load)(qapi_Net_SSL_Obj_Hdl_t hdl, qapi_Net_SSL_Cert_Type_t type, const char * name);
    int32_t (*qc_drv_net_SSL_Max_Clients_Set)(qapi_Net_SSL_Obj_Hdl_t hdl, uint32_t max_Clients);
    int32_t (*qc_drv_net_SSL_Idle_Timeout_Set)(qapi_Net_SSL_Obj_Hdl_t hdl, uint32_t idle_Timeout);
    int32_t (*qc_drv_net_SSL_ECJPAKE_Parameters_Set)(qapi_Net_SSL_Obj_Hdl_t hdl, qapi_Net_SSL_ECJPAKE_Params_t *cfg);
    int32_t (*qc_drv_net_SSL_PSK_Table_Set)(qapi_Net_SSL_Obj_Hdl_t hdl, qapi_Net_SSL_PSK_t *psk_Entries, uint16_t num_PSK_Entries);
    int32_t (*qc_drv_net_SSL_Con_Get_Status)(qapi_Net_SSL_Con_Hdl_t ssl);
    int32_t (*qc_drv_net_SSL_Fd_Set)(qapi_Net_SSL_Con_Hdl_t ssl, uint32_t fd);
    int32_t (*qc_drv_net_SSL_Accept)(qapi_Net_SSL_Con_Hdl_t ssl);
    int32_t (*qc_drv_net_IPv6_Config_Router_Prefix)(
            const char *interface_Name,
            uint8_t *ipv6_Addr,
            uint32_t prefix_Length,
            uint32_t preferred_Lifetime,
            uint32_t valid_Lifetime);
    int32_t (*qc_drv_net_IPv6_Route_Add)(const char *interface_Name, ip6_addr *dest, uint32_t prefix_Length, ip6_addr *next_Hop);
    int32_t (*qc_drv_net_IPv6_Route_Del)(const char *interface_Name, ip6_addr *dest, uint32_t prefix_Length);
    int32_t (*qc_drv_net_IPv6_Routing_Table_Get)(qapi_Net_IPv6_Route_t *buf, uint32_t *pcount);
    int32_t (*qc_drv_net_Profile_Set_Custom)(qapi_Net_Profile_Custom_Pool_t *pNet_buf, uint8_t net_bufq_size);
    int32_t (*qc_drv_net_Profile_Set_Active)(qapi_Net_Profile_Type_t profile);
    int32_t (*qc_drv_net_Profile_Get_Active)(qapi_Net_Profile_Custom_Pool_t **pNet_buf, uint8_t *net_bufq_size, qapi_Net_Profile_Type_t *profile);
    int32_t (*qc_drv_net_OMTM_Switch_Operating_Mode)( uint32_t mode_Id, qapi_OMTM_Switch_At_t when);
    int32_t (*qc_drv_net_DNSs_Is_Started)(void);
    int32_t (*qc_drv_net_DNSs_Get_Host_List)(int32_t *n, qapi_Net_DNS_Host_t *hostlist);
    int32_t (*qc_drv_net_DNSs_Add_Host)(const char *host_Name, struct ip46addr *host_Addr, uint32_t ttl);
    int32_t (*qc_drv_net_DNSs_Del_Host)(const char *hostname);
    int32_t (*qc_drv_net_DNSs_Command)(qapi_Net_DNS_Server_Command_t cmd);
    int32_t (*qc_drv_net_mDNS_Command)(qapi_Net_mDNS_Command_t cmd, void *input, uint8_t blocking, qapi_Net_mDNS_CB_t app_CB);
    int32_t (*qc_drv_net_DNSSD_Start)(qapi_Net_DNSSD_Start_t *start);
    int32_t (*qc_drv_net_DNSSD_Init)(qapi_Net_DNSSD_Init_t *init);
    int32_t (*qc_drv_net_DNSSD_Stop)(qapi_Net_DNSSD_Ctxt_t *ctxt);
    int32_t (*qc_drv_net_DNSSD_Discover)(const char *svcName);
    int32_t (*qc_drv_net_DNSSD_Get_Target_Info)(const char *svcName);
    int32_t (*qc_drv_net_Buf_Free_Queue_Status)(qapi_Net_Buf_Queue_Status_t *arg);
    int32_t (*qc_drv_net_Get_Socket_Status)(qapi_Net_Socket_Status_t *status);
    int32_t (*qc_drv_net_IPv4_Route)(
            const char *interface_Name,
            qapi_Net_Route_Command_t cmd,
            uint32_t *ipv4_Addr,
            uint32_t *subnet_Mask,
            uint32_t *gateway,
            qapi_Net_IPv4_Route_List_t *route_List);
    int32_t (*qc_drv_net_HTTPc_Start)(void);
    int32_t (*qc_drv_net_HTTPc_Stop)(void);
    void * (*qc_drv_net_HTTPc_New_sess)(
            uint32_t timeout,
            qapi_Net_SSL_Obj_Hdl_t ssl_Object_Handle,
            qapi_HTTPc_CB_t callback,
            void* arg,
            uint16_t httpc_Max_Body_Length,
            uint16_t httpc_Max_Header_Length);
    int32_t (*qc_drv_net_HTTPc_Configure_SSL)(qapi_Net_HTTPc_handle_t handle, qapi_Net_SSL_Config_t *ssl_Cfg);
    int32_t (*qc_drv_net_HTTPc_Connect)(qapi_Net_HTTPc_handle_t handle, const char *URL, uint16_t port);
    int32_t (*qc_drv_net_HTTPc_Free_sess)(qapi_Net_HTTPc_handle_t handle);
    int32_t (*qc_drv_net_HTTPc_Request)(qapi_Net_HTTPc_handle_t handle, qapi_Net_HTTPc_Method_e cmd, const char *URL);
    int32_t (*qc_drv_net_HTTPc_Set_Body)(qapi_Net_HTTPc_handle_t handle, const char *body, uint32_t body_Length);
    int32_t (*qc_drv_net_HTTPc_Add_Header_Field)(qapi_Net_HTTPc_handle_t handle, const char *type, const char *value);
    int32_t (*qc_drv_net_HTTPc_Clear_Header)(qapi_Net_HTTPc_handle_t handle);
    int32_t (*qc_drv_net_HTTPc_Set_Param) (qapi_Net_HTTPc_handle_t handle, const char *key, const char *value);
    int32_t (*qc_drv_net_HTTPc_CB_Enable_Adding_Header) (qapi_Net_HTTPc_handle_t handle, uint16_t enable);

  /* Start of Ble */
    qapi_Status_t (*qc_drv_ble_BSC_Query_Host_Version)(char *HostVersion);

    qapi_Status_t (*qc_drv_ble_HCI_Read_Local_Version_Information)(uint32_t BluetoothStackID, uint8_t *StatusResult, uint8_t *HCI_VersionResult, uint16_t *HCI_RevisionResult, uint8_t *LMP_VersionResult, uint16_t *Manufacturer_NameResult, uint16_t *LMP_SubversionResult);

    qapi_Status_t (*qc_drv_ble_GAP_LE_Set_Pairability_Mode)(uint32_t BluetoothStackID, qapi_BLE_GAP_LE_Pairability_Mode_t PairabilityMode);

    qapi_Status_t (*qc_drv_ble_GAP_LE_Authentication_Response)(uint32_t BluetoothStackID, qapi_BLE_BD_ADDR_t SecurityRemoteBD_ADDR, qapi_BLE_GAP_LE_Authentication_Response_Information_t *GAP_LE_Authentication_Response_Information);

    qapi_Status_t (*qc_drv_ble_GAP_LE_Query_Encryption_Mode)(uint32_t BluetoothStackID, qapi_BLE_BD_ADDR_t BD_ADDR, qapi_BLE_GAP_Encryption_Mode_t *GAP_Encryption_Mode);

    qapi_Status_t (*qc_drv_ble_GAP_LE_Set_Fixed_Passkey)(uint32_t BluetoothStackID, uint32_t *Fixed_Display_Passkey);

    qapi_Status_t (*qc_drv_ble_GAP_Query_Local_BD_ADDR)(uint32_t BluetoothStackID, qapi_BLE_BD_ADDR_t *BD_ADDR);

    qapi_Status_t (*qc_drv_ble_GAP_LE_Advertising_Disable)(uint32_t BluetoothStackID);

    qapi_Status_t (*qc_drv_ble_GAPS_Query_Device_Appearance)(uint32_t BluetoothStackID, uint32_t InstanceID, uint16_t *DeviceAppearance);

    qapi_Status_t (*qc_drv_ble_GAPS_Query_Device_Name)(uint32_t BluetoothStackID, uint32_t InstanceID, char *NameBuffer);

    qapi_Status_t (*qc_drv_ble_GAP_LE_Set_Advertising_Data)(uint32_t BluetoothStackID, uint32_t Length, qapi_BLE_Advertising_Data_t *Advertising_Data);

    qapi_Status_t (*qc_drv_ble_GAP_LE_Set_Scan_Response_Data)(uint32_t BluetoothStackID, uint32_t Length, qapi_BLE_Scan_Response_Data_t *Scan_Response_Data);

    qapi_Status_t (*qc_drv_ble_BSC_LockBluetoothStack)(uint32_t BluetoothStackID);

    void (*qc_drv_ble_BSC_UnLockBluetoothStack)(uint32_t BluetoothStackID);

    qapi_Status_t (*qc_drv_ble_GAP_LE_Advertising_Enable)(uint32_t BluetoothStackID, boolean_t EnableScanResponse, qapi_BLE_GAP_LE_Advertising_Parameters_t *GAP_LE_Advertising_Parameters, qapi_BLE_GAP_LE_Connectability_Parameters_t *GAP_LE_Connectability_Parameters, qapi_BLE_GAP_LE_Event_Callback_t GAP_LE_Event_Callback, uint32_t CallbackParameter);

    qapi_Status_t (*qc_drv_ble_GAP_LE_Disconnect)(uint32_t BluetoothStackID, qapi_BLE_BD_ADDR_t BD_ADDR);

    qapi_Status_t (*qc_drv_ble_GAP_LE_Cancel_Create_Connection)(uint32_t BluetoothStackID);

    qapi_Status_t (*qc_drv_ble_GATT_Start_Service_Discovery_Handle_Range)(uint32_t BluetoothStackID, uint32_t ConnectionID, qapi_BLE_GATT_Attribute_Handle_Group_t *DiscoveryHandleRange, uint32_t NumberOfUUID, qapi_BLE_GATT_UUID_t *UUIDList, qapi_BLE_GATT_Service_Discovery_Event_Callback_t ServiceDiscoveryCallback, uint32_t CallbackParameter);

    qapi_Status_t (*qc_drv_ble_GATT_Start_Service_Discovery)(uint32_t BluetoothStackID, uint32_t ConnectionID, uint32_t NumberOfUUID, qapi_BLE_GATT_UUID_t *UUIDList, qapi_BLE_GATT_Service_Discovery_Event_Callback_t ServiceDiscoveryCallback, uint32_t CallbackParameter);

    qapi_Status_t (*qc_drv_ble_GAP_LE_Add_Device_To_White_List)(uint32_t BluetoothStackID, uint32_t DeviceCount, qapi_BLE_GAP_LE_White_List_Entry_t *WhiteListEntries, uint32_t *AddedDeviceCount);

    qapi_Status_t (*qc_drv_ble_GAP_LE_Remove_Device_From_White_List)(uint32_t BluetoothStackID, uint32_t DeviceCount, qapi_BLE_GAP_LE_White_List_Entry_t *WhiteListEntries, uint32_t *RemovedDeviceCount);

    qapi_Status_t (*qc_drv_ble_GAP_LE_Add_Device_To_Resolving_List)(uint32_t BluetoothStackID, uint32_t DeviceCount, qapi_BLE_GAP_LE_Resolving_List_Entry_t *ResolvingListEntries, uint32_t *AddedDeviceCount);

    qapi_Status_t (*qc_drv_ble_GAP_LE_Remove_Device_From_Resolving_List)(uint32_t BluetoothStackID, uint32_t DeviceCount, qapi_BLE_GAP_LE_Resolving_List_Entry_t *ResolvingListEntries, uint32_t *RemovedDeviceCount);

    qapi_Status_t (*qc_drv_ble_GAP_LE_Set_Authenticated_Payload_Timeout)(uint32_t BluetoothStackID, qapi_BLE_BD_ADDR_t BD_ADDR, uint16_t AuthenticatedPayloadTimeout);

    qapi_Status_t (*qc_drv_ble_GATT_Change_Maximum_Supported_MTU)(uint32_t BluetoothStackID, uint16_t MTU);

    qapi_Status_t (*qc_drv_ble_GATT_Query_Maximum_Supported_MTU)(uint32_t BluetoothStackID, uint16_t *MTU);

    qapi_Status_t (*qc_drv_ble_GAP_LE_Query_Connection_Handle)(uint32_t BluetoothStackID, qapi_BLE_BD_ADDR_t BD_ADDR, uint16_t *Connection_Handle);

    qapi_Status_t (*qc_drv_ble_GAP_LE_Set_Data_Length)(uint32_t BluetoothStackID, qapi_BLE_BD_ADDR_t BD_ADDR, uint16_t SuggestedTxPacketSize, uint16_t SuggestedTxPacketTime);

    qapi_Status_t (*qc_drv_ble_GAP_LE_Query_Local_Secure_Connections_OOB_Data)(uint32_t BluetoothStackID, qapi_BLE_Secure_Connections_Randomizer_t *Randomizer, qapi_BLE_Secure_Connections_Confirmation_t *Confirmation);

    qapi_Status_t (*qc_drv_ble_BSC_SetTxPower)(uint32_t BluetoothStackID, boolean_t Connection, int8_t TxPower);

    qapi_Status_t (*qc_drv_ble_BSC_Set_FEM_Control_Override)(uint32_t BluetoothStackID, boolean_t Enable, uint16_t FEM_Ctrl_0_1, uint16_t FEM_Ctrl_2_3);

    qapi_Status_t (*qc_drv_ble_AIOS_Initialize_Service) (uint32_t BluetoothStackID, uint32_t Service_Flags, qapi_BLE_AIOS_Initialize_Data_t *InitializeData, qapi_BLE_AIOS_Event_Callback_t EventCallback, uint32_t CallbackParameter, uint32_t *ServiceID);

    qapi_Status_t (*qc_drv_ble_AIOS_Cleanup_Service) (uint32_t BluetoothStackID, uint32_t InstanceID);

    qapi_Status_t (*qc_drv_ble_GATT_Read_Value_Request) (uint32_t BluetoothStackID, uint32_t ConnectionID, uint16_t AttributeHandle, qapi_BLE_GATT_Client_Event_Callback_t ClientEventCallback, uint32_t CallbackParameter);

    qapi_Status_t (*qc_drv_ble_AIOS_Notify_Characteristic) (uint32_t BluetoothStackID, uint32_t InstanceID, uint32_t ConnectionID, qapi_BLE_AIOS_Characteristic_Info_t *CharacteristicInfo, qapi_BLE_AIOS_Characteristic_Data_t *CharacteristicData);

    qapi_Status_t (*qc_drv_ble_GATT_Write_Request) (uint32_t BluetoothStackID, uint32_t ConnectionID, uint16_t AttributeHandle, uint16_t AttributeLength, void *AttributeValue, qapi_BLE_GATT_Client_Event_Callback_t ClientEventCallback, uint32_t CallbackParameter);

    qapi_Status_t (*qc_drv_ble_HIDS_Initialize_Service) (uint32_t BluetoothStackID, uint8_t Flags, qapi_BLE_HIDS_HID_Information_Data_t *HIDInformation, uint32_t NumIncludedServices, uint32_t *ServiceIDList, uint32_t NumExternalReportReferences, qapi_BLE_GATT_UUID_t *ReferenceUUID, uint32_t NumReports, qapi_BLE_HIDS_Report_Reference_Data_t *ReportReference, qapi_BLE_HIDS_Event_Callback_t EventCallback, uint32_t CallbackParameter, uint32_t *ServiceID);

    qapi_Status_t (*qc_drv_ble_HIDS_Cleanup_Service) (uint32_t BluetoothStackID, uint32_t InstanceID);

    qapi_Status_t (*qc_drv_ble_HIDS_Notify_Input_Report) (uint32_t BluetoothStackID, uint32_t InstanceID, uint32_t ConnectionID, qapi_BLE_HIDS_Report_Type_t ReportType, qapi_BLE_HIDS_Report_Reference_Data_t *ReportReferenceData, uint16_t InputReportLength, uint8_t *InputReportData);

    qapi_Status_t (*qc_drv_ble_HIDS_Format_Control_Point_Command) (qapi_BLE_HIDS_Control_Point_Command_t Command, uint32_t BufferLength, uint8_t *Buffer);

    qapi_Status_t (*qc_drv_ble_GATT_Write_Without_Response_Request)(uint32_t BluetoothStackID, uint32_t ConnectionID, uint16_t AttributeHandle, uint16_t AttributeLength, void *AttributeValue);

    qapi_Status_t (*qc_drv_ble_HIDS_Format_Protocol_Mode) (qapi_BLE_HIDS_Protocol_Mode_t ProtocolMode, uint32_t BufferLength, uint8_t *Buffer);
    qapi_Status_t (*qc_drv_ble_SCPS_Initialize_Service) (uint32_t BluetoothStackID, qapi_BLE_SCPS_Event_Callback_t EventCallback, uint32_t CallbackParameter, uint32_t *ServiceID);

    qapi_Status_t (*qc_drv_ble_SCPS_Cleanup_Service) (uint32_t BluetoothStackID, uint32_t InstanceID);

    qapi_Status_t (*qc_drv_ble_SCPS_Format_Scan_Interval_Window) (qapi_BLE_SCPS_Scan_Interval_Window_Data_t *Scan_Interval_Window, uint32_t BufferLength, uint8_t *Buffer);

    qapi_Status_t (*qc_drv_ble_SCPS_Notify_Scan_Refresh) (uint32_t BluetoothStackID, uint32_t InstanceID, uint32_t ConnectionID, uint8_t ScanRefreshValue);

    qapi_Status_t (*qc_drv_ble_GATT_Register_Service) (uint32_t BluetoothStackID, uint8_t ServiceFlags, uint32_t NumberOfServiceAttributeEntries, qapi_BLE_GATT_Service_Attribute_Entry_t *ServiceTable, qapi_BLE_GATT_Attribute_Handle_Group_t *ServiceHandleGroupResult, qapi_BLE_GATT_Server_Event_Callback_t ServerEventCallback, uint32_t CallbackParameter);

    void (*qc_drv_ble_GATT_Un_Register_Service) (uint32_t BluetoothStackID, uint32_t ServiceID);

    qapi_Status_t (*qc_drv_ble_BAS_Initialize_Service) (uint32_t BluetoothStackID, qapi_BLE_BAS_Event_Callback_t EventCallback, uint32_t CallbackParameter, uint32_t *ServiceID);

    qapi_Status_t (*qc_drv_ble_BAS_Set_Characteristic_Presentation_Format)(uint32_t BluetoothStackID, uint32_t InstanceID, qapi_BLE_BAS_Presentation_Format_Data_t *CharacteristicPresentationFormat);

    qapi_Status_t (*qc_drv_ble_BAS_Cleanup_Service) (uint32_t BluetoothStackID, uint32_t InstanceID);

    qapi_Status_t (*qc_drv_ble_BAS_Notify_Battery_Level) (uint32_t BluetoothStackID, uint32_t InstanceID, uint32_t ConnectionID, uint8_t BatteryLevel);

    qapi_Status_t (*qc_drv_ble_BAS_Query_Characteristic_Presentation_Format) (uint32_t BluetoothStackID, uint32_t InstanceID, qapi_BLE_BAS_Presentation_Format_Data_t *CharacteristicPresentationFormat);

    qapi_Status_t (*qc_drv_ble_GAPS_Set_Device_Name) (uint32_t BluetoothStackID, uint32_t InstanceID, char *DeviceName);
    qapi_Status_t (*qc_drv_ble_GAPS_Set_Device_Appearance) (uint32_t BluetoothStackID, uint32_t InstanceID, uint16_t DeviceAppearance);

    qapi_Status_t (*qc_drv_ble_DIS_Initialize_Service) (uint32_t BluetoothStackID, uint32_t *ServiceID);
    qapi_Status_t (*qc_drv_ble_DIS_Set_Manufacturer_Name) (uint32_t BluetoothStackID, uint32_t InstanceID, char *ManufacturerName);
    qapi_Status_t (*qc_drv_ble_DIS_Set_Model_Number) (uint32_t BluetoothStackID, uint32_t InstanceID, char *ModelNumber);
    qapi_Status_t (*qc_drv_ble_DIS_Set_Software_Revision) (uint32_t BluetoothStackID, uint32_t InstanceID, char *SoftwareRevision);
    qapi_Status_t (*qc_drv_ble_DIS_Set_Hardware_Revision) (uint32_t BluetoothStackID, uint32_t InstanceID, char *Hardware_Revision);
    qapi_Status_t (*qc_drv_ble_DIS_Set_Firmware_Revision) (uint32_t BluetoothStackID, uint32_t InstanceID, char *FirmwareRevision);
    qapi_Status_t (*qc_drv_ble_DIS_Cleanup_Service) (uint32_t BluetoothStackID, uint32_t InstanceID);

    qapi_Status_t (*qc_drv_ble_TPS_Initialize_Service) (int32_t BluetoothStackID, uint32_t *ServiceID);
    qapi_Status_t (*qc_drv_ble_TPS_Set_Tx_Power_Level) (int32_t BluetoothStackID, uint32_t InstanceID, int8_t Tx_Power_Level);
    qapi_Status_t (*qc_drv_ble_TPS_Cleanup_Service) (int32_t BluetoothStackID, uint32_t InstanceID);

    qapi_Status_t (*qc_drv_ble_GAP_LE_Query_Connection_PHY) (uint32_t BluetoothStackID, qapi_BLE_BD_ADDR_t BD_ADDR, qapi_BLE_GAP_LE_PHY_Type_t *TxPHY, qapi_BLE_GAP_LE_PHY_Type_t *RxPHY);

    qapi_Status_t (*qc_drv_ble_GAP_LE_Set_Connection_PHY) (uint32_t BluetoothStackID, qapi_BLE_BD_ADDR_t BD_ADDR, uint32_t TxPHYSPreference, uint32_t RxPHYSPreference);

    qapi_Status_t (*qc_drv_ble_GAP_LE_Set_Extended_Advertising_Parameters) (uint32_t BluetoothStackID, uint8_t AdvertisingHandle, qapi_BLE_GAP_LE_Extended_Advertising_Parameters_t *AdvertisingParameters, int8_t *SelectedTxPower);

    qapi_Status_t (*qc_drv_ble_GAP_LE_Enable_Extended_Advertising) (uint32_t BluetoothStackID, boolean_t Enable, uint8_t NumberOfSets, uint8_t *AdvertisingHandleList, uint32_t *DurationList, uint8_t *MaxExtendedAdvertisingEventList, qapi_BLE_GAP_LE_Event_Callback_t GAP_LE_Event_Callback, uint32_t CallbackParameter);

    qapi_Status_t (*qc_drv_ble_GAP_LE_Set_Extended_Scan_Parameters) (uint32_t BluetoothStackID, qapi_BLE_GAP_LE_Address_Type_t LocalAddressType, qapi_BLE_GAP_LE_Filter_Policy_t FilterPolicy, uint32_t NumberScanningPHYs, qapi_BLE_GAP_LE_Extended_Scanning_PHY_Parameters_t *ScanningParameterList);

    qapi_Status_t (*qc_drv_ble_GAP_LE_Enable_Extended_Scan) (uint32_t BluetoothStackID, boolean_t Enable, qapi_BLE_GAP_LE_Extended_Scan_Filter_Duplicates_Type_t FilterDuplicates, uint32_t Duration, uint32_t Period, qapi_BLE_GAP_LE_Event_Callback_t GAP_LE_Event_Callback, uint32_t CallbackParameter);

    qapi_Status_t (*qc_drv_ble_GAP_LE_Extended_Create_Connection) (uint32_t BluetoothStackID, qapi_BLE_GAP_LE_Filter_Policy_t InitatorFilterPolicy, qapi_BLE_GAP_LE_Address_Type_t RemoteAddressType, qapi_BLE_BD_ADDR_t *RemoteDevice, qapi_BLE_GAP_LE_Address_Type_t LocalAddressType, uint32_t NumberOfConnectionParameters, qapi_BLE_GAP_LE_Extended_Connection_Parameters_t *ConnectionParameterList, qapi_BLE_GAP_LE_Event_Callback_t GAP_LE_Event_Callback, uint32_t CallbackParameter);

    qapi_Status_t (*qc_drv_ble_BSC_AddGenericListEntry_Actual)(qapi_BLE_BSC_Generic_List_Entry_Key_t GenericListEntryKey, uint32_t ListEntryKeyOffset, uint32_t ListEntryNextPointerOffset, void **ListHead, void *ListEntryToAdd);

    qapi_Status_t (*qc_drv_ble_BSC_Initialize)(qapi_BLE_HCI_DriverInformation_t *HCI_DriverInformation, uint32_t Flags);

    qapi_Status_t (*qc_drv_ble_HCI_LE_Read_Buffer_Size)(uint32_t BluetoothStackID, uint8_t *StatusResult, uint16_t *HC_LE_ACL_Data_Packet_Length, uint8_t *HC_Total_Num_LE_ACL_Data_Packets);

    qapi_Status_t (*qc_drv_ble_GATT_Initialize)(uint32_t BluetoothStackID, uint32_t Flags, qapi_BLE_GATT_Connection_Event_Callback_t ConnectionEventCallback, uint32_t CallbackParameter);

    qapi_Status_t (*qc_drv_ble_GAPS_Initialize_Service)(uint32_t BluetoothStackID, uint32_t *ServiceID);

    qapi_Status_t (*qc_drv_ble_GAP_LE_Set_Address_Resolution_Enable)(uint32_t BluetoothStackID, boolean_t EnableAddressResolution);

    qapi_Status_t (*qc_drv_ble_GAP_LE_Set_Resolvable_Private_Address_Timeout)(uint32_t BluetoothStackID, uint32_t RPA_Timeout);

    qapi_Status_t (*qc_drv_ble_HCI_Register_Event_Callback)(uint32_t BluetoothStackID, qapi_BLE_HCI_Event_Callback_t HCI_EventCallback, uint32_t CallbackParameter);

    qapi_Status_t (*qc_drv_ble_HCI_Register_ACL_Data_Callback)(uint32_t BluetoothStackID, qapi_BLE_HCI_ACL_Data_Callback_t HCI_ACLDataCallback, uint32_t CallbackParameter);

    qapi_Status_t (*qc_drv_ble_HCI_Send_ACL_Data)(uint32_t BluetoothStackID, uint16_t Connection_Handle, uint16_t Flags, uint16_t ACLDataLength, uint8_t *ACLData);

    qapi_Status_t (*qc_drv_ble_GAP_LE_Register_Remote_Authentication)(uint32_t BluetoothStackID, qapi_BLE_GAP_LE_Event_Callback_t GAP_LE_Event_Callback, uint32_t CallbackParameter);

    qapi_Status_t (*qc_drv_ble_BSC_StartTimer)(uint32_t BluetoothStackID, uint32_t Timeout, qapi_BLE_BSC_Timer_Callback_t TimerCallback, uint32_t CallbackParameter);

    qapi_Status_t (*qc_drv_ble_GAP_LE_Perform_Scan)(uint32_t BluetoothStackID, qapi_BLE_GAP_LE_Scan_Type_t ScanType, uint32_t ScanInterval, uint32_t ScanWindow, qapi_BLE_GAP_LE_Address_Type_t LocalAddressType, qapi_BLE_GAP_LE_Filter_Policy_t FilterPolicy, boolean_t FilterDuplicates, qapi_BLE_GAP_LE_Event_Callback_t GAP_LE_Event_Callback, uint32_t CallbackParameter);

    qapi_Status_t (*qc_drv_ble_GAP_LE_Cancel_Scan)(uint32_t BluetoothStackID);

    qapi_Status_t (*qc_drv_ble_GAP_LE_Create_Connection)(uint32_t BluetoothStackID, uint32_t ScanInterval, uint32_t ScanWindow, qapi_BLE_GAP_LE_Filter_Policy_t InitatorFilterPolicy, qapi_BLE_GAP_LE_Address_Type_t RemoteAddressType, qapi_BLE_BD_ADDR_t *RemoteDevice, qapi_BLE_GAP_LE_Address_Type_t LocalAddressType, qapi_BLE_GAP_LE_Connection_Parameters_t *ConnectionParameters, qapi_BLE_GAP_LE_Event_Callback_t GAP_LE_Event_Callback, uint32_t CallbackParameter);

    qapi_Status_t (*qc_drv_ble_HRS_Decode_Heart_Rate_Measurement)(uint32_t ValueLength, uint8_t *Value, qapi_BLE_HRS_Heart_Rate_Measurement_Data_t *HeartRateMeasurement);

    qapi_Status_t (*qc_drv_ble_GATT_Handle_Value_Notification)(uint32_t BluetoothStackID, uint32_t ServiceID, uint32_t ConnectionID, uint16_t AttributeOffset, uint16_t AttributeValueLength, uint8_t *AttributeValue);

    qapi_Status_t (*qc_drv_ble_BSC_GetTxPower)(uint32_t BluetoothStackID, boolean_t Connection, int8_t *TxPower);

    qapi_Status_t (*qc_drv_ble_GAP_LE_Reestablish_Security)(uint32_t BluetoothStackID, qapi_BLE_BD_ADDR_t BD_ADDR, qapi_BLE_GAP_LE_Security_Information_t *SecurityInformation, qapi_BLE_GAP_LE_Event_Callback_t GAP_LE_Event_Callback, uint32_t CallbackParameter);

    qapi_Status_t (*qc_drv_ble_GAP_LE_Regenerate_Long_Term_Key)(uint32_t BluetoothStackID, qapi_BLE_Encryption_Key_t *DHK, qapi_BLE_Encryption_Key_t *ER, uint16_t EDIV, qapi_BLE_Random_Number_t *Rand, qapi_BLE_Long_Term_Key_t *LTK_Result);

    qapi_Status_t (*qc_drv_ble_AIOS_Read_Characteristic_Request_Response)(uint32_t BluetoothStackID, uint32_t InstanceID, uint32_t ConnectionID, uint32_t TransactionID, uint8_t ErrorCode, qapi_BLE_AIOS_Characteristic_Info_t *CharacteristicInfo, qapi_BLE_AIOS_Characteristic_Data_t *CharacteristicData);

    qapi_Status_t (*qc_drv_ble_AIOS_Write_Characteristic_Request_Response)(uint32_t BluetoothStackID, uint32_t InstanceID, uint32_t TransactionID, uint8_t ErrorCode, qapi_BLE_AIOS_Characteristic_Info_t *CharacteristicInfo);

    qapi_Status_t (*qc_drv_ble_AIOS_Read_CCCD_Request_Response)(uint32_t BluetoothStackID, uint32_t InstanceID, uint32_t TransactionID, uint8_t ErrorCode, qapi_BLE_AIOS_Characteristic_Info_t *CharacteristicInfo, uint16_t ClientConfiguration);

    qapi_Status_t (*qc_drv_ble_AIOS_Write_CCCD_Request_Response)(uint32_t BluetoothStackID, uint32_t InstanceID, uint32_t TransactionID, uint8_t ErrorCode, qapi_BLE_AIOS_Characteristic_Info_t *CharacteristicInfo);

    qapi_Status_t (*qc_drv_ble_AIOS_Read_Presentation_Format_Request_Response)(uint32_t BluetoothStackID, uint32_t InstanceID, uint32_t TransactionID, uint8_t ErrorCode, qapi_BLE_AIOS_Characteristic_Info_t *CharacteristicInfo, qapi_BLE_AIOS_Presentation_Format_Data_t *PresentationFormatData);

    qapi_Status_t (*qc_drv_ble_AIOS_Read_Number_Of_Digitals_Request_Response)(uint32_t BluetoothStackID, uint32_t InstanceID, uint32_t TransactionID, uint8_t ErrorCode, qapi_BLE_AIOS_Characteristic_Info_t *CharacteristicInfo, uint8_t NumberOfDigitals);

    qapi_Status_t (*qc_drv_ble_BAS_Read_Client_Configuration_Response)(uint32_t BluetoothStackID, uint32_t InstanceID, uint32_t TransactionID, uint16_t Client_Configuration);

    qapi_Status_t (*qc_drv_ble_HIDS_Read_Client_Configuration_Response)(uint32_t BluetoothStackID, uint32_t InstanceID, uint32_t TransactionID, uint16_t Client_Configuration);

    qapi_Status_t (*qc_drv_ble_HIDS_Get_Protocol_Mode_Response)(uint32_t BluetoothStackID, uint32_t InstanceID, uint32_t TransactionID, uint8_t ErrorCode, qapi_BLE_HIDS_Protocol_Mode_t CurrentProtocolMode);

    qapi_Status_t (*qc_drv_ble_HIDS_Get_Report_Map_Response)(uint32_t BluetoothStackID, uint32_t InstanceID, uint32_t TransactionID, uint8_t ErrorCode, uint32_t ReportMapLength, uint8_t *ReportMap);

    qapi_Status_t (*qc_drv_ble_HIDS_Get_Report_Response)(uint32_t BluetoothStackID, uint32_t InstanceID, uint32_t TransactionID, qapi_BLE_HIDS_Report_Type_t ReportType, qapi_BLE_HIDS_Report_Reference_Data_t *ReportReferenceData, uint8_t ErrorCode, uint32_t ReportLength, uint8_t *Report);

    qapi_Status_t (*qc_drv_ble_HIDS_Set_Report_Response)(uint32_t BluetoothStackID, uint32_t InstanceID, uint32_t TransactionID, qapi_BLE_HIDS_Report_Type_t ReportType, qapi_BLE_HIDS_Report_Reference_Data_t *ReportReferenceData, uint8_t ErrorCode);

    qapi_Status_t (*qc_drv_ble_SCPS_Read_Client_Configuration_Response)(uint32_t BluetoothStackID, uint32_t InstanceID, uint32_t TransactionID, uint16_t Client_Configuration);

    qapi_Status_t (*qc_drv_ble_AIOS_Decode_Presentation_Format)(uint32_t ValueLength, uint8_t *Value, qapi_BLE_AIOS_Presentation_Format_Data_t *PresentationFormatData);

    qapi_Status_t (*qc_drv_ble_BAS_Decode_Characteristic_Presentation_Format)(uint32_t ValueLength, uint8_t *Value, qapi_BLE_BAS_Presentation_Format_Data_t *CharacteristicPresentationFormat);

    qapi_Status_t (*qc_drv_ble_GATT_Read_Long_Value_Request)(uint32_t BluetoothStackID, uint32_t ConnectionID, uint16_t AttributeHandle, uint16_t AttributeOffset, qapi_BLE_GATT_Client_Event_Callback_t ClientEventCallback, uint32_t CallbackParameter);

    qapi_Status_t (*qc_drv_ble_HIDS_Decode_HID_Information)(uint32_t ValueLength, uint8_t *Value, qapi_BLE_HIDS_HID_Information_Data_t *HIDSHIDInformation);

    qapi_Status_t (*qc_drv_ble_HIDS_Decode_Report_Reference)(uint32_t ValueLength, uint8_t *Value, qapi_BLE_HIDS_Report_Reference_Data_t *ReportReferenceData);

    qapi_Status_t (*qc_drv_ble_HIDS_Decode_External_Report_Reference)(uint32_t ValueLength, uint8_t *Value, qapi_BLE_GATT_UUID_t *ExternalReportReferenceUUID);


    /** Start of Zigbee */
    qapi_Status_t (*qc_drv_ZB_Initialize)(qapi_ZB_Handle_t *ZB_Handle, qapi_ZB_Event_CB_t ZB_Event_CB, uint32_t CB_Param);
    qapi_Status_t (*qc_drv_ZB_Register_Persist_Notify_CB)(qapi_ZB_Handle_t ZB_Handle, qapi_ZB_Persist_Notify_CB_t ZB_Persist_Notify_CB, uint32_t CB_Param);
    qapi_Status_t (*qc_drv_ZB_ZDP_Register_Callback)(qapi_ZB_Handle_t ZB_Handle, qapi_ZB_ZDP_Event_CB_t ZDP_Event_CB, uint32_t CB_Param);
    qapi_Status_t (*qc_drv_ZB_Get_Extended_Address)(qapi_ZB_Handle_t ZB_Handle, uint64_t *Extended_Address);
    qapi_Status_t (*qc_drv_ZB_Shutdown)(qapi_ZB_Handle_t ZB_Handle);
    qapi_Status_t (*qc_drv_ZB_Form)(qapi_ZB_Handle_t ZB_Handle, const qapi_ZB_NetworkConfig_t *Config);
    qapi_Status_t (*qc_drv_ZB_Join)(qapi_ZB_Handle_t ZB_Handle, const qapi_ZB_Join_t *Config);
    qapi_Status_t (*qc_drv_ZB_Leave)(qapi_ZB_Handle_t ZB_Handle);
    qapi_Status_t (*qc_drv_ZB_ZDP_Mgmt_Leave_Req)(qapi_ZB_Handle_t ZB_Handle, uint16_t DstNwkAddr, uint64_t DeviceAddress, qbool_t RemoveChildren, qbool_t Rejoin);
    qapi_Status_t (*qc_drv_ZB_Permit_Join)(qapi_ZB_Handle_t ZB_Handle, uint8_t Duration);
    qapi_Status_t (*qc_drv_ZB_ZDP_Bind_Req)(qapi_ZB_Handle_t ZB_Handle, const qapi_ZB_ZDP_Bind_Req_t *RequestData);
    qapi_Status_t (*qc_drv_ZB_ZDP_End_Device_Bind_Req)(qapi_ZB_Handle_t ZB_Handle, const qapi_ZB_ZDP_End_Device_Bind_Req_t *RequestData);
    qapi_Status_t (*qc_drv_ZB_APSME_Get_Request)(qapi_ZB_Handle_t ZB_Handle, qapi_ZB_AIB_Attribute_ID_t AIBAttribute, uint8_t AIBAttributeIndex, uint16_t *AIBAttributeLength, void *AIBAttributeValue);
    qapi_Status_t (*qc_drv_ZB_APSME_Set_Request)(qapi_ZB_Handle_t ZB_Handle, qapi_ZB_AIB_Attribute_ID_t AIBAttribute, uint8_t AIBAttributeIndex, uint16_t *AIBAttributeLength, const void *AIBAttributeValue);
    qapi_Status_t (*qc_drv_ZB_Set_Extended_Address)(qapi_ZB_Handle_t ZB_Handle, uint64_t Extended_Address);
    qapi_Status_t (*qc_drv_ZB_APS_Add_Endpoint)(qapi_ZB_Handle_t ZB_Handle, qapi_ZB_APS_Add_Endpoint_t *RequestData);
    qapi_Status_t (*qc_drv_ZB_APS_Remove_Endpoint)(qapi_ZB_Handle_t ZB_Handle, uint8_t Endpoint);
    qapi_Status_t (*qc_drv_ZB_CL_Read_Local_Attribute)(qapi_ZB_Cluster_t Cluster, uint16_t AttrId, uint16_t *Length, uint8_t *Data);
    qapi_Status_t (*qc_drv_ZB_CL_Write_Local_Attribute)(qapi_ZB_Cluster_t Cluster, uint16_t AttrId, uint16_t Length, uint8_t *Data);
    qapi_Status_t (*qc_drv_ZB_CL_Read_Attributes)(qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, uint8_t AttrCount, const uint16_t *AttrIdList);
    qapi_Status_t (*qc_drv_ZB_CL_Write_Attributes)(qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, const uint8_t AttrCount, const qapi_ZB_CL_Write_Attr_Record_t *AttrStructuredList);
    qapi_Status_t (*qc_drv_ZB_CL_Discover_Attributes)(qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, const uint16_t StartAttrId, const uint8_t AttrIdCount);
    qapi_Status_t (*qc_drv_ZB_CL_Configure_Reporting)(qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, uint8_t ReportCount, const qapi_ZB_CL_Attr_Reporting_Config_Record_t *ReportRecordList);
    qapi_Status_t (*qc_drv_ZB_CL_Read_Reporting)(qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, uint8_t ReportCount, const qapi_ZB_CL_Attr_Record_t *ReportConfigList);
    qapi_Status_t (*qc_drv_ZB_CL_OnOff_Send_On)(qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, qbool_t RecallGlobalScene);
    qapi_Status_t (*qc_drv_ZB_CL_OnOff_Send_Off)(qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo);
    qapi_Status_t (*qc_drv_ZB_CL_OnOff_Send_Toggle)(qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo);
    qapi_Status_t (*qc_drv_ZB_CL_LevelControl_Send_Move_To_Level)(qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, uint8_t Level, uint16_t TransitionTime, qbool_t WithOnOff);
    qapi_Status_t (*qc_drv_ZB_CL_LevelControl_Send_Move)(qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, qbool_t MoveDown, uint8_t Rate, qbool_t WithOnOff);
    qapi_Status_t (*qc_drv_ZB_CL_LevelControl_Send_Step)(qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, qbool_t MoveDown, uint8_t StepSize, uint16_t TransitionTime, qbool_t WithOnOff);
    qapi_Status_t (*qc_drv_ZB_CL_LevelControl_Send_Stop)(qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo);

    qapi_Status_t (*qc_drv_ZB_BDB_Set_Request) (qapi_ZB_Handle_t ZB_Handle, qapi_ZB_BDB_Attribute_ID_t AttributeId, uint8_t AttributeIndex, uint16_t AttributeLength, const uint8_t *AttributeValue);
    qapi_Status_t (*qc_drv_ZB_BDB_Get_Request) (qapi_ZB_Handle_t ZB_Handle, qapi_ZB_BDB_Attribute_ID_t AttributeId, uint8_t AttributeIndex, uint16_t *AttributeLength, uint8_t *AttributeValue);
    qapi_Status_t (*qc_drv_ZB_ZDP_Match_Desc_Req) (qapi_ZB_Handle_t ZB_Handle, uint16_t DstNwkAddr, const qapi_ZB_ZDP_Match_Desc_Req_t *RequestData);
    qapi_Status_t (*qc_drv_ZB_CL_Basic_Send_Reset_To_Factory) (qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo);
    qapi_Status_t (*qc_drv_ZB_CL_Basic_Server_Read_Attribute) (qapi_ZB_Handle_t ZB_Handle, uint16_t AttrId, uint16_t *Length, uint8_t *Data);
    qapi_Status_t (*qc_drv_ZB_CL_Basic_Server_Write_Attribute) (qapi_ZB_Handle_t ZB_Handle, uint16_t AttrId, uint16_t Length, const uint8_t *Data);
    qapi_Status_t (*qc_drv_ZB_CL_Identify_Send_Identify) (qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, uint16_t IdentifyTime);
    qapi_Status_t (*qc_drv_ZB_CL_Identify_Send_Identify_Query) (qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo);
    qapi_Status_t (*qc_drv_ZB_CL_Groups_Send_Add_Group) (qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, uint16_t GroupId, const char *GroupName, qbool_t Identifying);
    qapi_Status_t (*qc_drv_ZB_CL_Groups_Send_View_Group) (qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, uint16_t GroupId);
    qapi_Status_t (*qc_drv_ZB_CL_Groups_Send_Remove_Group) (qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, uint16_t GroupId);
    qapi_Status_t (*qc_drv_ZB_CL_Groups_Send_Remove_All_Groups) (qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo);
    qapi_Status_t (*qc_drv_ZB_CL_Groups_Send_Get_Group_Membership) (qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, uint8_t GroupCount, const uint16_t *GroupList);

    qapi_Status_t (*qc_drv_ZB_CL_Scenes_Send_Remove_Scene) (qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, uint16_t GroupId, uint8_t SceneId);
    qapi_Status_t (*qc_drv_ZB_CL_Scenes_Send_Remove_All_Scenes) (qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, uint16_t GroupId);
    qapi_Status_t (*qc_drv_ZB_CL_Scenes_Send_Store_Scene) (qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, uint16_t GroupId, uint8_t SceneId);
    qapi_Status_t (*qc_drv_ZB_CL_Scenes_Send_Recall_Scene) (qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, uint16_t GroupId, uint8_t SceneId);
    qapi_Status_t (*qc_drv_ZB_CL_Scenes_Send_Get_Scene_Membership) (qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, uint16_t GroupId);
    qapi_Status_t (*qc_drv_ZB_CL_Scenes_Send_Copy_Scene) (qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, const qapi_ZB_CL_Scenes_Copy_Scene_t *CopyScene);
    qapi_Status_t (*qc_drv_ZB_CL_Scenes_Send_Add_Scene) (qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, const qapi_ZB_CL_Scenes_Add_Scene_t *AddScene);
    qapi_Status_t (*qc_drv_ZB_CL_Scenes_Send_View_Scene) (qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, uint16_t GroupId, uint8_t SceneId, qbool_t IsEnhanced);

    qapi_Status_t (*qc_drv_ZB_CL_Time_Server_Read_Attribute) (qapi_ZB_Handle_t ZB_Handle, uint16_t AttrId, uint16_t *Length, uint8_t *Data);
    qapi_Status_t (*qc_drv_ZB_CL_Time_Server_Write_Attribute) (qapi_ZB_Handle_t ZB_Handle, uint16_t AttrId, uint16_t Length, const uint8_t *Data);

    qapi_Status_t (*qc_drv_ZB_CL_Alarm_Send_Reset_Alarm) (qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, uint16_t SourceClusterId, uint8_t AlarmCode);
    qapi_Status_t (*qc_drv_ZB_CL_Alarm_Send_Reset_All_Alarms) (qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo);
    qapi_Status_t (*qc_drv_ZB_CL_Alarm_Send_Get_Alarm) (qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo);
    qapi_Status_t (*qc_drv_ZB_CL_Alarm_Send_Reset_Alarm_Log) (qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo);
    qapi_Status_t (*qc_drv_ZB_CL_Alarm_Send_Alarm) (qapi_ZB_Cluster_t Cluster, qapi_ZB_Cluster_t SourceCluster, uint8_t AlarmCode);

    qapi_Status_t (*qc_drv_ZB_CL_ColorControl_Send_Move_To_Hue) (qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, uint16_t Hue, qapi_ZB_CL_ColorControl_Move_Mode_t Direction, uint16_t TransitionTime, qbool_t IsEnhanced);
    qapi_Status_t (*qc_drv_ZB_CL_ColorControl_Send_Move_To_HueAndSaturation) (qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, uint16_t Hue, uint8_t Saturation, uint16_t TransitionTime, qbool_t IsEnhanced);
    qapi_Status_t (*qc_drv_ZB_CL_ColorControl_Send_Move_Hue) (qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, qapi_ZB_CL_ColorControl_Move_Mode_t MoveMode, uint16_t Rate, qbool_t IsEnhanced);
    qapi_Status_t (*qc_drv_ZB_CL_ColorControl_Send_Step_Hue) (qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, qapi_ZB_CL_ColorControl_Move_Mode_t StepMode, uint16_t StepSize, uint16_t TransitionTime, qbool_t IsEnhanced);
    qapi_Status_t (*qc_drv_ZB_CL_ColorControl_Send_Move_To_Saturation) (qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, uint8_t Saturation, uint16_t TransitionTime);
    qapi_Status_t (*qc_drv_ZB_CL_ColorControl_Send_Move_Saturation) (qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, qapi_ZB_CL_ColorControl_Move_Mode_t MoveMode, uint8_t Rate);
    qapi_Status_t (*qc_drv_ZB_CL_ColorControl_Send_Step_Saturation) (qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, qapi_ZB_CL_ColorControl_Move_Mode_t StepMode, uint8_t StepSize, uint8_t TransitionTime);
    qapi_Status_t (*qc_drv_ZB_CL_ColorControl_Send_Move_To_Color) (qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, uint16_t ColorX, uint16_t ColorY, uint16_t TransitionTime);
    qapi_Status_t (*qc_drv_ZB_CL_ColorControl_Send_Move_Color) (qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, int16_t RateX, int16_t RateY);
    qapi_Status_t (*qc_drv_ZB_CL_ColorControl_Send_Step_Color) (qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, int16_t StepX, int16_t StepY, uint16_t TransitionTime);
    qapi_Status_t (*qc_drv_ZB_CL_ColorControl_Send_Move_To_Color_Temp) (qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, uint16_t ColorTempMireds, uint16_t TransitionTime);
    qapi_Status_t (*qc_drv_ZB_CL_ColorControl_Send_Move_Color_Temp) (qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, qapi_ZB_CL_ColorControl_Move_Mode_t MoveMode, uint16_t Rate, uint16_t Limit);
    qapi_Status_t (*qc_drv_ZB_CL_ColorControl_Send_Step_Color_Temp) (qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, const qapi_ZB_CL_ColorControl_Step_Color_Temp_t *StepColorTemp);
    qapi_Status_t (*qc_drv_ZB_CL_ColorControl_Send_Stop_Move_Step) (qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo);
    qapi_Status_t (*qc_drv_ZB_CL_ColorControl_Send_Color_Loop_Set) (qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, const qapi_ZB_CL_ColorControl_Color_Loop_Set_t *ColorLoopSet);

	 qapi_Status_t (*qc_drv_ZB_Get_Persistent_Data) (qapi_ZB_Handle_t ZB_Handle, uint8_t *Buffer, uint32_t *Length);
        qapi_Status_t (*qc_drv_ZB_Restore_Persistent_Data) (qapi_ZB_Handle_t ZB_Handle, const uint8_t *Buffer, uint32_t Length);
        qapi_Status_t (*qc_drv_ZB_NLME_Get_Request) (qapi_ZB_Handle_t ZB_Handle, qapi_ZB_NIB_Attribute_ID_t AttributeId, uint8_t AttributeIndex, uint16_t *AttributeLength, uint8_t *AttributeValue);
        qapi_Status_t (*qc_drv_ZB_NLME_Set_Request) (qapi_ZB_Handle_t ZB_Handle, qapi_ZB_NIB_Attribute_ID_t AttributeId, uint8_t AttributeIndex, uint16_t AttributeLength, const uint8_t *AttributeValue);
        qapi_Status_t (*qc_drv_ZB_CL_Touchlink_Start) (qapi_ZB_Cluster_t Cluster, qapi_ZB_CL_Touchlink_Device_Type_t DeviceType, const uint8_t *PersistData, uint32_t PersistLength);
        qapi_Status_t (*qc_drv_ZB_CL_Touchlink_Scan_Request) (qapi_ZB_Cluster_t ClientCluster);
        qapi_Status_t (*qc_drv_ZB_CL_Touchlink_Factory_Reset) (qapi_ZB_Cluster_t ClientCluster);
        qapi_Status_t (*qc_drv_ZB_CL_Destroy_Cluster) (qapi_ZB_Cluster_t Cluster);
        qapi_Status_t (*qc_drv_Persist_Initialize) (qapi_Persist_Handle_t *Handle, char *Directory, char *NamePrefix, char *NameSuffix, uint8_t *Password, uint32_t PasswordSize);
        void (*qc_drv_Persist_Delete) (qapi_Persist_Handle_t Handle);
        void (*qc_drv_Persist_Cleanup) (qapi_Persist_Handle_t Handle);

    /* Start of Thread */
    qapi_Status_t (*qc_drv_TWN_Initialize)(qapi_TWN_Handle_t *TWN_Handle, qapi_TWN_Event_CB_t TWN_Event_CB, uint32_t CB_Param);

    qapi_Status_t (*qc_drv_TWN_Start)(qapi_TWN_Handle_t TWN_Handle);

    qapi_Status_t (*qc_drv_TWN_Stop)(qapi_TWN_Handle_t TWN_Handle);

    void (*qc_drv_TWN_Shutdown)(qapi_TWN_Handle_t TWN_Handle);

    qapi_Status_t (*qc_drv_TWN_Get_Device_Configuration)(qapi_TWN_Handle_t TWN_Handle, qapi_TWN_Device_Configuration_t *Configuration);

    qapi_Status_t (*qc_drv_TWN_Set_Device_Configuration)(qapi_TWN_Handle_t TWN_Handle, const qapi_TWN_Device_Configuration_t *Configuration);

    qapi_Status_t (*qc_drv_TWN_Get_Network_Configuration)(qapi_TWN_Handle_t TWN_Handle, qapi_TWN_Network_Configuration_t *Configuration);

    qapi_Status_t (*qc_drv_TWN_Set_Network_Configuration)(qapi_TWN_Handle_t TWN_Handle, const qapi_TWN_Network_Configuration_t *Configuration);

    qapi_Status_t (*qc_drv_TWN_Add_Border_Router)(qapi_TWN_Handle_t TWN_Handle, const qapi_TWN_Border_Router_t *Border_Router);

    qapi_Status_t (*qc_drv_TWN_Remove_Border_Router)(qapi_TWN_Handle_t TWN_Handle, const qapi_TWN_IPv6_Prefix_t *Prefix);

    qapi_Status_t (*qc_drv_TWN_Add_External_Route)(qapi_TWN_Handle_t TWN_Handle, const qapi_TWN_External_Route_t *External_Route);

    qapi_Status_t (*qc_drv_TWN_Remove_External_Route)(qapi_TWN_Handle_t TWN_Handle, const qapi_TWN_IPv6_Prefix_t *Prefix);

    qapi_Status_t (*qc_drv_TWN_Register_Server_Data)(qapi_TWN_Handle_t TWN_Handle);

    qapi_Status_t (*qc_drv_TWN_Set_IP_Stack_Integration)(qapi_TWN_Handle_t TWN_Handle, qbool_t Enabled);

    qapi_Status_t (*qc_drv_TWN_Commissioner_Start)(qapi_TWN_Handle_t TWN_Handle);

    qapi_Status_t (*qc_drv_TWN_Commissioner_Stop)(qapi_TWN_Handle_t TWN_Handle);

    qapi_Status_t (*qc_drv_TWN_Commissioner_Add_Joiner)(qapi_TWN_Handle_t TWN_Handle, uint64_t Extended_Address, const char *PSKd, uint32_t Timeout);

    qapi_Status_t (*qc_drv_TWN_Commissioner_Remove_Joiner)(qapi_TWN_Handle_t TWN_Handle, uint64_t Extended_Address);

    qapi_Status_t (*qc_drv_TWN_Commissioner_Set_Provisioning_URL)(qapi_TWN_Handle_t TWN_Handle, const char *Provisioning_URL);

    qapi_Status_t (*qc_drv_TWN_Commissioner_Generate_PSKc)(qapi_TWN_Handle_t TWN_Handle, const char *Passphrase, const char *Network_Name, uint64_t Extended_PAN_ID, uint8_t *PSKc);

    qapi_Status_t (*qc_drv_TWN_Joiner_Start)(qapi_TWN_Handle_t TWN_Handle, const qapi_TWN_Joiner_Info_t *Joiner_Info);

    qapi_Status_t (*qc_drv_TWN_Joiner_Stop)(qapi_TWN_Handle_t TWN_Handle);

    qapi_Status_t (*qc_drv_TWN_Set_PSKc)(qapi_TWN_Handle_t TWN_Handle, const uint8_t *PSKc);

    qapi_Status_t (*qc_drv_TWN_IPv6_Add_Unicast_Address)(qapi_TWN_Handle_t TWN_Handle, qapi_TWN_IPv6_Prefix_t *Prefix, qbool_t Preferred);

    qapi_Status_t (*qc_drv_TWN_IPv6_Remove_Unicast_Address)(qapi_TWN_Handle_t TWN_Handle, qapi_TWN_IPv6_Address_t *Address);

    qapi_Status_t (*qc_drv_TWN_IPv6_Subscribe_Multicast_Address)(qapi_TWN_Handle_t TWN_Handle, qapi_TWN_IPv6_Address_t *Address);

    qapi_Status_t (*qc_drv_TWN_IPv6_Unsubscribe_Multicast_Address)(qapi_TWN_Handle_t TWN_Handle, qapi_TWN_IPv6_Address_t *Address);

    qapi_Status_t (*qc_drv_TWN_Set_Ping_Response_Enabled)(qapi_TWN_Handle_t TWN_Handle, qbool_t Enabled);

    qapi_Status_t (*qc_drv_TWN_Become_Router)(qapi_TWN_Handle_t TWN_Handle);

    qapi_Status_t (*qc_drv_TWN_Become_Leader)(qapi_TWN_Handle_t TWN_Handle);

    qapi_Status_t (*qc_drv_TWN_Start_Border_Agent)(qapi_TWN_Handle_t TWN_Handle, int AddressFamily, const char *DisplayName, const char *Hostname, const char *Interface);

    qapi_Status_t (*qc_drv_TWN_Stop_Border_Agent)(qapi_TWN_Handle_t TWN_Handle);

    qapi_Status_t (*qc_drv_TWN_Clear_Persistent_Data)(qapi_TWN_Handle_t TWN_Handle);

    qapi_Status_t (*qc_drv_TWN_Set_Max_Poll_Period)(qapi_TWN_Handle_t TWN_Handle, uint32_t Period);

    qapi_Status_t (*qc_drv_TWN_Commissioner_Send_Mgmt_Get)(qapi_TWN_Handle_t TWN_Handle, const uint8_t *TlvBuffer, uint8_t Length);

    qapi_Status_t (*qc_drv_TWN_Commissioner_Send_Mgmt_Set)(qapi_TWN_Handle_t TWN_Handle, const qapi_TWN_Commissioning_Dataset_t *Dataset, const uint8_t *TlvBuffer, uint8_t Length);

    qapi_Status_t (*qc_drv_TWN_Commissioner_Send_PanId_Query)(qapi_TWN_Handle_t TWN_Handle, uint16_t PanId, uint32_t ChannelMask, const qapi_TWN_IPv6_Address_t *Address);

    qapi_Status_t (*qc_drv_TWN_Commissioner_Get_Session_Id)(qapi_TWN_Handle_t TWN_Handle, uint16_t *SessionId);

    qapi_Status_t (*qc_drv_TWN_Commissioner_Send_Mgmt_Active_Get)(qapi_TWN_Handle_t TWN_Handle, const qapi_TWN_IPv6_Address_t *Address, const uint8_t *TlvBuffer, uint8_t Length);

    qapi_Status_t (*qc_drv_TWN_Commissioner_Send_Mgmt_Active_Set)(qapi_TWN_Handle_t TWN_Handle, const qapi_TWN_Operational_Dataset_t *Dataset, const uint8_t *TlvBuffer, uint8_t Length);

}qc_drv_cb_t;

typedef struct qc_drv_context {
    qc_drv_cb_t *drv_ops;
}qc_drv_context;

qc_drv_context *qc_drv_register(qc_drv_cb_t *);
void qc_drv_deregister(qc_drv_context *);
#endif
