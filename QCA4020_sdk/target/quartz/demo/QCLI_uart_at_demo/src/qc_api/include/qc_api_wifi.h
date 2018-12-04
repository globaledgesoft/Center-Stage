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

/*-------------------------------------------------------------------------
 *  Include Files
 *-----------------------------------------------------------------------*/
#ifndef __QC_API_WIFI_H
#define __QC_API_WIFI_H

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

#include "qc_drv_wifi.h"
#include "qc_api_main.h"
#include "qc_api_p2p.h"

/*-------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 *-----------------------------------------------------------------------*/
#define QC_API_ENABLE_AP_MODE                  1
#define QC_API_ENABLE_SCC_MODE                 0

#ifndef QC_API_WLAN_NUM_OF_DEVICES
#define QC_API_WLAN_NUM_OF_DEVICES             2
#endif

/* Utility thread macros */
#define QC_API_UTILITY_THREAD_PRIORITY         10
#define QC_API_UTILITY_THREAD_STACK_SIZE       2048
#define QC_API_WPS_EVENT_MASK                  0x1
#define QC_API_P2P_EVENT_MASK                  0x2
#define QC_API_CONNECT_EVENT_MASK              0x4

#define QC_API_APP_STRCMP(_a, _b) strcmp((const char*)(_a), (const char *)(_b))
#define QC_API_APP_STRCASECMP(_a, _b) strcasecmp((const char*)(_a), (const char *)(_b))

#define QC_API_SHORTSCANRATIO_DEFAULT          0x3
#define QC_API_SEC_MODE_OPEN                   (0)
#define QC_API_SEC_MODE_WEP                    (1)
#define QC_API_SEC_MODE_WPA                    (2)
#define QC_API_DEFAULT_SCAN_CTRL_FLAGS         0x28
#define QC_API_DEV_NUM                         	2
#define QC_API_ATH_PROMISC_ARG_SET_LENTGH      (5)
#define QC_API_MIN_HEX_WEP_KEY_SIZE            10
#define QC_API_MAX_HEX_WEP_KEY_SIZE            26
#define QC_API_MAX_NUM_WEP_KEYS                4
#define QC_API_MAX_WPS_PIN_SIZE                32
#define QC_API_PROBE_REQ_SRC_ADDR_OFFSET       (10)

#define qc_api_wifi_set_dev_stat(id, status) (dev_stat[id]=status)
#define qc_api_wifi_get_dev_stat(id) (dev_stat[id])

#define UP                              	1
#define DOWN                            	0
#define FALSE           			0
#define TRUE            			1

uint8_t original_ssid[__QAPI_WLAN_MAX_SSID_LENGTH];
//uint8_t wps_flag = 0;
char wpa_passphrase[QC_API_DEV_NUM][__QAPI_WLAN_PASSPHRASE_LEN + 1];
//volatile uint8_t wifi_state[QC_API_DEV_NUM] = {0};

#if 0
/* Used to remember persistent information while waiting for GO_NEG complete event */
uint8_t p2p_persistent_go = QAPI_WLAN_P2P_NON_PERSISTENT_E;
uint8_t p2p_persistent_done = 0;
uint8_t p2p_cancel_enable = 0;
uint8_t p2p_session_in_progress = 0;
uint8_t invitation_index = 0;
uint8_t p2p_join_session_active = 0;
uint8_t p2p_intent = 0;
uint8_t inv_response_evt_index = 0;
uint32_t set_channel_p2p = 0;
#endif

// This structure holds the WPS setup attributes
typedef struct {
	uint8_t wps_in_progress;
	uint8_t connect_flag;
	uint8_t wps_pbc_interrupt;
	qapi_WLAN_Netparams_t netparams;
} wps_context_t;

// This structure holds the Promisc mode filters configuration
struct
{
	uint8_t num_filters; /* To Keep track of how many filters user is configuring */
	uint8_t filter_flags[__QAPI_WLAN_PROMISC_MAX_FILTER_IDX]; /* filtering rules */
	uint8_t src_Mac[__QAPI_WLAN_PROMISC_MAX_FILTER_IDX][__QAPI_WLAN_MAC_LEN]; /* filter source mac address if desired. */
	uint8_t dst_Mac[__QAPI_WLAN_PROMISC_MAX_FILTER_IDX][__QAPI_WLAN_MAC_LEN]; /* filter destination mac address if desired. */
	uint8_t promisc_frametype[__QAPI_WLAN_PROMISC_MAX_FILTER_IDX];
	uint8_t promisc_subtype[__QAPI_WLAN_PROMISC_MAX_FILTER_IDX];
}promisc_filter_config;

/*-------------------------------------------------------------------------
 *  Function prototypes
 *-----------------------------------------------------------------------*/
uint32_t qc_api_wifi_strtoul(const char* arg, const char* endptr, int base);
unsigned long long qc_api_wifi_atoll(const char* arg);

void Initialize_ATW_Demo_(void);
int32_t qc_api_wifi_wlan_callback_handler(uint8_t  deviceId, uint32_t cbId,
			void *pApplicationContext, void *payload, uint32_t payload_Length);
int32_t qc_api_wifi_wps_query(uint8_t block);
uint16_t qc_api_wifi_freq_to_channel(uint16_t channel_val);
void qc_api_wifi_Wlan_Utility_Thread(void *para);
int32_t qc_api_wifi_Create_Wlan_Utility_Thread (void);
void qc_api_wifi_Destroy_Wlan_Utility_Thread(void);
int32_t qc_api_wifi_set_active_deviceid(uint16_t deviceId);
int32_t qc_api_wifi_get_active_device(void);
uint32_t qc_api_wifi_set_callback(const void *applicationContext);
int32_t qc_api_wifi_enable_wlan(void);
int32_t qc_api_wifi_disable_wlan(void);
void qc_api_wifi_promisc_frame_handler(void *payload, uint32_t length);
int32_t qc_api_wifi_set_mac(int8_t *mac_addr);
void qc_api_wifi_print_wlan_channel_list(qapi_WLAN_Get_Channel_List_t *wlanChannelList);
int32_t qc_api_wifi_print_wlan_stats( qapi_WLAN_Statistics_t *stats );
int32_t qc_api_wifi_print_scan_results(qapi_WLAN_Scan_List_t param, uint16_t num_scan);
int32_t qc_api_wifi_wmi_set_scan(uint32_t dev_num, void *start_scan,
			qapi_WLAN_Store_Scan_Results_e store_scan_results);
uint32_t qc_api_wifi_set_power_mode(boolean_t pwr_mode, uint8_t pwr_module);
int32_t qc_api_wifi_get_wifi_power_mode(char *pw_mode);
int32_t qc_api_wifi_get_phy_mode(char *phy_mode);
int32_t qc_api_wifi_get_ssid(char *ssid);
int32_t qc_api_wifi_get_device_mac_address(char *mac);
int32_t qc_api_wifi_get_op_mode(void);
int32_t qc_api_wifi_get_channel(void);
int32_t qc_api_wifi_print_wlan_info(void);
void qc_api_wifi_connect_handler(int32_t val,
			uint8_t devId,uint8_t * mac,boolean_t bssConn);
int32_t qc_api_wifi_NetConnect(qapi_WLAN_Netparams_t *pNetparams);
uint32_t qc_api_wifi_CompleteWPS(uint32_t deviceId,
			qapi_WLAN_Netparams_t *pNetparams, uint8_t block);
int32_t qc_api_wifi_wps_query(uint8_t block);
int32_t qc_api_wifi_disconnect_from_network(void);
int32_t qc_api_wifi_set_passphrase(char* passphrase);
int32_t qc_api_wifi_get_rssi(void);
void qc_api_wifi_probe_req_handler(void* buf, int32_t len, int32_t freq);
void qc_api_wifi_wnm_event_cb(uint8_t deviceId, void *payload, uint32_t payloadLength);
void qc_api_wifi_wps_profile_event_cb(uint8_t deviceId, void *payload, uint32_t payloadLength);
int32_t qc_api_wifi_connect_to_network(const int8_t* ssid);
uint32_t qc_api_wifi_get_wlan_channel_list();
uint32_t qc_api_wifi_get_wlan_stats( uint8_t flag );
int32_t qc_api_wifi_promiscuous_mode_handler(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
uint32_t qc_api_wifi_chan_to_frequency(uint32_t channel);
int32_t qc_api_wifi_get_rate(void);
int32_t qc_api_wifi_set_rate(int32_t isMcs, int32_t rateIdx);
int32_t qc_api_wifi_set_channel_hint(int32_t channelNum);
int32_t qc_api_wifi_set_tx_power(int32_t power_in_dBm);
int32_t qc_api_wifi_set_phy_mode(int32_t wmode);
int32_t qc_api_wifi_set_11n_ht(int8_t *ht_config);
int32_t qc_api_wifi_set_pwr_mode(int32_t power_mode);
int32_t qc_api_wifi_get_version(void);
int32_t qc_api_wifi_get_reg_domain(void);
int32_t qc_api_wifi_get_last_error(void);
int32_t qc_api_wifi_wlan_scan(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
void qc_api_wifi_wlan_scan_result(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
int32_t qc_api_wifi_set_ap_beacon_interval(uint32_t beacon_int_in_tu);
int32_t qc_api_wifi_set_ap_dtim_period(uint32_t dtim_period);
int32_t qc_api_wifi_get_country_code(void);
int32_t qc_api_wifi_set_country_code(int8_t *country);
int32_t qc_api_wifi_wep_connect(QCLI_Parameter_t *Parameter_List);
int32_t qc_api_wifi_wpa_connect(QCLI_Parameter_t *Parameter_List);
int32_t qc_api_wifi_wps_connect(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
int32_t qc_api_wifi_set_op_mode(int8_t *omode, char *hiddenSsid, char *wpsEnabled);

#if ENABLE_SCC_MODE
static int32_t qc_api_wifi_handle_connect_for_concurrent(uint32_t deviceId, int8_t* ssid);
#endif

#endif // _WIFI_API_H_
