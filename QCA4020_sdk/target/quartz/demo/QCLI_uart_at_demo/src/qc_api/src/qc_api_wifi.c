/*
 *  Copyright (c) 2018 Qualcomm Technologies, Inc.
 *  All Rights Reserved.
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
#include "qc_api_wifi.h"
#include "qc_drv_wifi.h"
#include "qc_api_p2p.h"
#include "qc_drv_p2p.h"
#include "qosa_util.h"

/*-------------------------------------------------------------------------
 * Static & global Variable Declarations
 *-----------------------------------------------------------------------*/
/************************** Global Declarations ************************************************/

#define FALSE 0
#define TRUE  1

qurt_thread_attr_t wlan_util_thread_attr;
qurt_thread_t wlan_util_thread_handle;
qurt_signal_t wlan_util_event;
qurt_mutex_t wlan_util_mutex;
qapi_WLAN_Scan_List_t scan_param;

uint8_t wlan_util_event_Initialized = 0;
uint8_t wlan_util_thread_created = 0;
uint8_t waiting_connection_completion = 0;

#ifndef WLAN_NUM_OF_DEVICES
#define WLAN_NUM_OF_DEVICES 2
#endif

/* Utility thread macros */
#define UTILITY_THREAD_PRIORITY       10
#define UTILITY_THREAD_STACK_SIZE   2048
#define WPS_EVENT_MASK               0x1
#define P2P_EVENT_MASK               0x2
#define CONNECT_EVENT_MASK  		 0x4

#define APP_STRCMP(_a, _b) strcmp((char const*)(_a), (const char *)(_b))

uint16_t ap_channel_hint;
uint8_t ssid_str_concurrent[__QAPI_WLAN_MAX_SSID_LENGTH] = {0};
#define AUTOIP_BASE_ADDR 0xA9FE0100
#define AUTOIP_MAX_ADDR 0xA9FEFEFF

#define UDP_CONNECTION_WAIT_TIME    500
#define SMALL_TX_DELAY              1
#define SHORTSCANRATIO_DEFAULT   0x3
#define SEC_MODE_OPEN           (0)
#define SEC_MODE_WEP            (1)
#define SEC_MODE_WPA            (2)
#define DEFAULT_SCAN_CTRL_FLAGS 0x28
#define DEV_NUM 2
#define UP 1
#define DOWN 0


/* Set defaults for Base & range on AUtoIP address pool */
unsigned long   dBASE_AUTO_IP_ADDRESS = AUTOIP_BASE_ADDR;    /* 169.254.1.0 */
unsigned long   dMAX_AUTO_IP_ADDRESS  = AUTOIP_MAX_ADDR;     /* 169.254.254.255 */

typedef struct key
{
    char    key[30];
    uint8_t key_valid;
} wep_key_t;

uint8_t pmk_flag[DEV_NUM] = {0, 0}, hidden_flag = 0, wps_flag = 0;
uint8_t dev_stat[DEV_NUM] = {DOWN, DOWN};
#define set_dev_stat(id, status) (dev_stat[id]=status)
#define get_dev_stat(id) (dev_stat[id])

qapi_WLAN_Mode_e mode_flag = MODE_STATION_E;
char ssid[__QAPI_WLAN_MAX_SSID_LENGTH];
uint8_t original_ssid[__QAPI_WLAN_MAX_SSID_LENGTH];               //Used to store user specified SSID
wps_context_t wps_context;
uint32_t concurrent_connect_flag = 0;

#define DEVICE_ID_STA     1 //STA
#define DEVICE_ID_NON_STA 0 //SoftAP/P2P GO/P2P CLI

#define ATH_PROMISC_ARG_SET_LENTGH    (5)
#define MIN_HEX_WEP_KEY_SIZE         10
#define MAX_HEX_WEP_KEY_SIZE         26
#define MAX_NUM_WEP_KEYS          4
#define MAX_WPS_PIN_SIZE         32

boolean_t p2pMode = FALSE;
#if ENABLE_P2P_MODE
#define P2P_STANDARD_TIMEOUT (300)
char p2p_wps_pin[__QAPI_WLAN_WPS_PIN_LEN];
uint32_t set_channel_p2p = 0;
boolean_t autogo_newpp = FALSE;
uint8_t p2p_persistent_done = 0, p2p_cancel_enable = 0, p2p_session_in_progress = 0;
uint8_t invitation_index = 0, p2p_join_session_active = 0, p2p_intent = 0, inv_response_evt_index = 0;
uint8_t p2p_join_mac_addr[__QAPI_WLAN_MAC_LEN];

/* Used to remember persistent information while waiting for GO_NEG complete event */
uint8_t p2p_persistent_go = QAPI_WLAN_P2P_NON_PERSISTENT_E;

qapi_WLAN_P2P_Connect_Cmd_t p2p_join_profile;
qapi_WLAN_P2P_Persistent_Mac_List_t p2p_peers_data[__QAPI_WLAN_P2P_MAX_LIST_COUNT];

uint8_t p2pScratchBuff[__QAPI_WLAN_P2P_EVT_BUF_SIZE];
#endif /* ENABLE_P2P_MODE */

static int security_mode = 0;
qapi_WLAN_Crypt_Type_e cipher;
qapi_WLAN_Auth_Mode_e wpa_ver;
uint8_t key_type = 0; /* Each bit represents type of key at that index. 1: Hex characters, 0:ascii character. This variable will be used only for the WEP keys set in target. */
wep_key_t key[MAX_NUM_WEP_KEYS];
uint32_t wep_keyindex;
char wpsPin[MAX_WPS_PIN_SIZE];
char wpa_passphrase[DEV_NUM][__QAPI_WLAN_PASSPHRASE_LEN + 1];
volatile uint8_t wifi_state[DEV_NUM] = {0};
int active_device = 0, wlan_enabled = 0;
int wps_should_disable;
uint8_t g_bssid[DEV_NUM][__QAPI_WLAN_MAC_LEN] ={{0},{0}};

int user_defined_power_mode = QAPI_WLAN_POWER_MODE_MAX_PERF_E; //QAPI_WLAN_POWER_MODE_MAX_PERF_E;
#define PROBE_REQ_SRC_ADDR_OFFSET  (10)

#if ENABLE_P2P_MODE
#define   P2P_CONNECT_OPERATION     1
#define   P2P_PROVISION_OPERATION   2
#define   P2P_AUTH_OPERATION        3
#define   P2P_INVITE_OPERATION      4
#endif

uint16_t channel_array[] =
{
    0, 2412, 2417,
    2422, 2427, 2432,
    2437, 2442, 2447,
    2452, 2457, 2462,
    2467, 2472, 2482
};

/*-------------------------------------------------------------------------
 * p2p: Function Prototype
 *-----------------------------------------------------------------------*/
void qc_api_wifi_app_wlan_p2p_event_cb(uint8_t device_Id, void *pData, uint32_t *pLength);

/*-------------------------------------------------------------------------
 * Below are the APIs for the UART AT WLAN commands
 *-----------------------------------------------------------------------*/

int32_t wlan_callback_handler( uint8_t  deviceId,
        uint32_t cbId,
        void *pApplicationContext,
        void     *payload,
        uint32_t payload_Length);

int32_t qc_api_wifi_wps_query(uint8_t block);

#if ENABLE_P2P_MODE
void app_handle_p2p_pending_events();
void app_free_p2p_pending_events();
#endif /* ENABLE_P2P_MODE */

uint16_t freq_to_channel(uint16_t channel_val);
uint32_t chan_to_frequency(uint32_t channel);

/***********************************************************************************************
 * Function Name  : qc_api_wifi_Create_Wlan_Utility_Thread()N
 * Returned Value : NA
 * Comments       : Utility thread is created when WLAN is enabled and it is used to process
 *                  asynchronous events from the firmware such as WPS, P2P.
 **********************************************************************************************/
void qc_api_wifi_Wlan_Utility_Thread(void *para)
{
    uint32_t utility_signals = 0; /* Possible signal masks for this event */
    uint32_t signal_set = 0; /* Indicated which signal is set */
    int32_t result = 0;

    /* Clear all signals before waiting */
    utility_signals = QC_API_WPS_EVENT_MASK | QC_API_P2P_EVENT_MASK;

    for(;;)
    {
        result = qurt_signal_wait_timed(&wlan_util_event, utility_signals,
                (QURT_SIGNAL_ATTR_WAIT_ANY |
                 QURT_SIGNAL_ATTR_CLEAR_MASK),
                &signal_set, QURT_TIME_WAIT_FOREVER);

        if(result != QURT_EOK)
        {
            /* This case will be true when qurt_signal_destroy() is called. */
            break;
        }

        /* Process WPS profile event */
        if (signal_set & QC_API_WPS_EVENT_MASK)
        {
            qc_api_wifi_wps_query(0);
        }

        /* Process the P2P event queue. */
        if (signal_set & QC_API_P2P_EVENT_MASK)
        {
#if ENABLE_P2P_MODE
            app_handle_p2p_pending_events();
#endif /* ENABLE_P2P_MODE */
        }
    }

#if ENABLE_P2P_MODE
    /* Free the P2P event queue memory before deleting utility thread. */
    app_free_p2p_pending_events();
#endif /* ENABLE_P2P_MODE */

    qurt_mutex_lock(&wlan_util_mutex);
    wlan_util_thread_created = 0;
    wlan_util_event_Initialized = 0;
    qurt_mutex_unlock(&wlan_util_mutex);
    qurt_mutex_destroy(&wlan_util_mutex); /* Destroy the mutex as it is not needed */

    /* Stop the utility thread. Memory allocated for this thread will be deleted */
    qurt_thread_stop();
    return;
}

/***********************************************************************************************
 * Function Name  : qc_api_wifi_Create_Wlan_Utility_Thread()
 * Returned Value : 0 on success or 1 on error
 * Comments       : Creates a Utility thread
 **********************************************************************************************/
int32_t qc_api_wifi_Create_Wlan_Utility_Thread ()
{
    int32_t Utility_Thread_Result = 0;

    /* Checks for wlan_util_event_Initialized and wlan_util_thread_created are added to
     * avoid thread and event creation more than once if 'wlan enable' command is
     * given more than once. These are reset when the thread and event are destroyed. */
    if (!wlan_util_event_Initialized && !wlan_util_thread_created)
    {
        /* Initialize the utility thread event. */
        qurt_mutex_init(&wlan_util_mutex);
    }

    qurt_mutex_lock(&wlan_util_mutex);
    if (!wlan_util_event_Initialized)
    {
        qurt_signal_init(&wlan_util_event);
        wlan_util_event_Initialized = 1;
    }

    if(!wlan_util_thread_created)
    {
        /* Create a thread to process WPS and P2P events. */
        qurt_thread_attr_init(&wlan_util_thread_attr);
        qurt_thread_attr_set_name(&wlan_util_thread_attr, "WlanUtil");
        qurt_thread_attr_set_priority(&wlan_util_thread_attr, QC_API_UTILITY_THREAD_PRIORITY);
        qurt_thread_attr_set_stack_size(&wlan_util_thread_attr, QC_API_UTILITY_THREAD_STACK_SIZE);
        Utility_Thread_Result = qurt_thread_create(&wlan_util_thread_handle,
                &wlan_util_thread_attr,
                qc_api_wifi_Wlan_Utility_Thread, NULL);

        if(Utility_Thread_Result != QURT_EOK)
        {
            /* Thread creation failed. Destroy the signal and lock to
             * avoid memory leak */
            LOG_ERR("Utility thread creation failed.\r\n");
            qurt_signal_destroy(&wlan_util_event);
            wlan_util_event_Initialized = 0;

            qurt_mutex_unlock(&wlan_util_mutex);
            qurt_mutex_destroy(&wlan_util_mutex);
            return QCLI_STATUS_ERROR_E;
        }
        wlan_util_thread_created = 1;
    }

    qurt_mutex_unlock(&wlan_util_mutex);
    return QCLI_STATUS_SUCCESS_E;
}

/***********************************************************************************************
 * Function Name  : qc_api_wifi_Destroy_Wlan_Utility_Thread()
 * Returned Value : NA
 * Comments       : Destroys the Utility thread
 **********************************************************************************************/
void qc_api_wifi_Destroy_Wlan_Utility_Thread()
{
    /* Following check is added to avoid accessing an uninitialized signal and thread */
    if (!wlan_util_thread_created || !wlan_util_event_Initialized)
    {
        return;
    }

    /* qurt_signal_destroy() will wake up the threads waiting on wlan_util_event.
     * qc_api_wifi_Wlan_Utility_Thread() exits the thread after this. */
    qurt_signal_destroy(&wlan_util_event);
    return;
}

/***********************************************************************************************
 * Function Name  : deviceid_handler
 * Returned Value : 1 on error else 0
 * Comments       : Sets the Device ID in the application.
 **********************************************************************************************/
int32_t qc_api_wifi_set_active_deviceid(uint16_t deviceId)
{
    if(wps_context.wps_in_progress)
    {
        LOG_WARN("wps sesion in progress switching device not allowed:\r\n");
        return QCLI_STATUS_ERROR_E;
    }

    if (0 == wlan_enabled)
    {
        LOG_ERR("Enable WLAN before setting the device ID\r\n");
        return QCLI_STATUS_ERROR_E;
    }

    active_device = deviceId;
    return QCLI_STATUS_SUCCESS_E;
}

int32_t qc_api_wifi_get_active_device()
{
    return active_device;
}

/***********************************************************************************************
 * Function Name  : qc_api_wifi_set_callback
 * Returned Value : -1 on error else 0
 * Comments       : Sets callback function for WiFi connect/disconnect event
 ***********************************************************************************************/
uint32_t qc_api_wifi_set_callback(const void *applicationContext)
{
    int32_t deviceId = qc_api_wifi_get_active_device();
    return(qc_drv_wlan_set_callback(qc_api_get_qc_drv_context(),
                deviceId, (qapi_WLAN_Callback_t)qc_api_wifi_wlan_callback_handler, applicationContext));
}

/***********************************************************************************************
 * Function Name  : disable_wlan()
 * Returned Value : 0 on success, 1 on error
 * Comments       : Enables the WLAN stack
 **********************************************************************************************/
int32_t qc_api_wifi_enable_wlan()
{
    int32_t i;

    if (wlan_enabled) {
        return QCLI_STATUS_SUCCESS_E;
    }

    if (0 == qc_drv_wlan_enable(qc_api_get_qc_drv_context()))
    {
        /* Due to bugs in Iniche stack, the second device is created statically in
         * MCC or SCC is enabled
         */
        for(i=0;i < QC_API_WLAN_NUM_OF_DEVICES;i++)
        {
            qc_drv_wlan_add_device(qc_api_get_qc_drv_context(), i);
        }

        qc_api_wifi_set_callback(NULL);

        if (0 == qc_api_wifi_Create_Wlan_Utility_Thread())
        {
            wlan_enabled = 1;
            return QCLI_STATUS_SUCCESS_E;
        }
    }
    return QCLI_STATUS_ERROR_E;
}


/***********************************************************************************************
 * Function Name  : disable_wlan()
 * Returned Value : 0 on success, 1 on error
 * Comments       : Disables the WLAN stack
 **********************************************************************************************/
int32_t qc_api_wifi_disable_wlan()
{
    int32_t i;

    if (0 == wlan_enabled) {
        return QCLI_STATUS_SUCCESS_E;
    }

    /* Remove the WLAN device from network interface before disabling WLAN */
    for(i=0;i < QC_API_WLAN_NUM_OF_DEVICES;i++)
    {
        qc_drv_wlan_remove_device(qc_api_get_qc_drv_context(), i);
    }

    if (0 == qc_drv_wlan_disable(qc_api_get_qc_drv_context()))
    {
        qc_api_wifi_Destroy_Wlan_Utility_Thread();
        wlan_enabled = 0;
        wifi_state[0] = 0;
        wifi_state[1] = 0;
        return QCLI_STATUS_SUCCESS_E;
    }
    return QCLI_STATUS_ERROR_E;
}

/***********************************************************************************************
 * Function Name  : qc_api_wifi_promisc_frame_handler
 * Returned Value : NA
 * Comments       : This handler used in promiscous mode
 **********************************************************************************************/
void qc_api_wifi_promisc_frame_handler(void *payload, uint32_t length)
{
    uint16_t i = 0, j = 0, print_length = 0;
    uint8_t *ptr = (uint8_t *)payload;
    uint32_t devId;

    if( length > 32 )
        print_length = 32;
    else
        print_length = length;

    devId = qc_api_wifi_get_active_device();
    LOG_AT_EVT("EVT_WIFI: %d frame (%d):\r\n", devId, length);
    /* only print the first 64 bytes of each frame */
    for(i = 0 ; i < print_length ; i++){
        LOG_AT_EVT("EVT_WIFI: %d 0x%02x, ", devId, ptr[i]);
        if(j++==7){
            j=0;
            LOG_AT_EVT("\r\n");
        }
    }
    if(j){
        LOG_AT_EVT("\r\n");
    }
}

/***********************************************************************************************
 * Function Name  : set_mac
 * Returned Value : 0 on success, 1 on error
 * Comments       : Sets the MAC address
 **********************************************************************************************/
int32_t qc_api_wifi_set_mac(int8_t *mac_addr)
{
    uint32_t deviceId = qc_api_wifi_get_active_device();
    if (0 == qc_drv_wlan_set_mac_addr(qc_api_get_qc_drv_context(),
                deviceId,
                (uint8_t *)mac_addr,
                strlen((const char *)mac_addr)))
    {
        return QCLI_STATUS_SUCCESS_E;
    }
    return QCLI_STATUS_ERROR_E;
}

/***********************************************************************************************
 * Function Name  : qc_api_wifi_print_wlan_channel_list
 * Returned Value : 0 on success
 * Comments       : Prints the wLAN channel list
 **********************************************************************************************/
void qc_api_wifi_print_wlan_channel_list(qapi_WLAN_Get_Channel_List_t *wlanChannelList)
{

    int32_t i;
    uint16_t channel;
    LOG_AT("\r\n CHANNEL LIST");
    for( i = 0; i < wlanChannelList->number_Of_Channels; i++)
    {

        if (wlanChannelList->channel_List[i] !=0)
        {
            channel = qc_api_wifi_freq_to_channel(wlanChannelList->channel_List[i]);
            LOG_AT("\r\r\n Channel: %d", channel);
        }
    }
}

/***********************************************************************************************
 * Function Name  : qc_api_wifi_print_wlan_stats
 * Returned Value : 0 on success
 * Comments       : Prints the WLAN statistics
 **********************************************************************************************/
int32_t qc_api_wifi_print_wlan_stats( qapi_WLAN_Statistics_t *stats )
{
    qapi_WLAN_Device_Stats_t *pDevWlanStats = &stats->dev_Stats;
    qapi_WLAN_Common_Stats_t *pCommonWlanStats = &stats->common_Stats;

    LOG_AT("\r\n======== WLAN STATISTICS ========= \r\n\n");

    LOG_AT("***** Device Specific WLAN STATISTICS ***** \r\n");

    LOG_AT("unicast_Tx_Pkts =0x%x\r\n", pDevWlanStats->unicast_Tx_Pkts);
    LOG_AT("unicast_Rx_Pkts = 0x%x\r\n", pDevWlanStats->unicast_Rx_Pkts);
    LOG_AT("multicast_Tx_Pkts = 0x%x\r\n", pDevWlanStats->multicast_Tx_Pkts);
    LOG_AT("multicast_Rx_Pkts = 0x%x\r\n", pDevWlanStats->multicast_Rx_Pkts);
    LOG_AT("broadcast_Tx_Pkts = 0x%x\r\n", pDevWlanStats->broadcast_Tx_Pkts);
    LOG_AT("broadcast_Rx_Pkts = 0x%x\r\n", pDevWlanStats->broadcast_Rx_Pkts);

    LOG_AT("unicast_non_null_Tx_Pkts =0x%x\r\n", pDevWlanStats->unicast_Tx_Pkts);
    LOG_AT("unicast_non_null_Rx_Pkts = 0x%x\r\n", pDevWlanStats->unicast_Rx_Pkts);

    LOG_AT("unicast_Filtered_Accepted_Tx_Pkts = 0x%x\r\n", pDevWlanStats->unicast_Filtered_Accepted_Tx_Pkts);
    LOG_AT("unicast_Filtered_Accepted_Rx_Pkts = 0x%x\r\n", pDevWlanStats->unicast_Filtered_Accepted_Rx_Pkts);
    LOG_AT("multicast_Filtered_Accepted_Tx_Pkts = 0x%x\r\n", pDevWlanStats->multicast_Filtered_Accepted_Tx_Pkts);
    LOG_AT("multicast_Filtered_Accepted_Rx_Pkts = 0x%x\r\n", pDevWlanStats->multicast_Filtered_Accepted_Rx_Pkts);
    LOG_AT("broadcast_Filtered_Accepted_Tx_Pkts = 0x%x\r\n", pDevWlanStats->broadcast_Filtered_Accepted_Tx_Pkts);
    LOG_AT("broadcast_Filtered_Accepted_Rx_Pkts = 0x%x\r\n", pDevWlanStats->broadcast_Filtered_Accepted_Rx_Pkts);

    LOG_AT("unicast_Filtered_Rejected_Tx_Pkts = 0x%x\r\n", pDevWlanStats->unicast_Filtered_Rejected_Tx_Pkts);
    LOG_AT("unicast_Filtered_Rejected_Rx_Pkts = 0x%x\r\n", pDevWlanStats->unicast_Filtered_Rejected_Rx_Pkts);
    LOG_AT("multicast_Filtered_Rejected_Tx_Pkts = 0x%x\r\n", pDevWlanStats->multicast_Filtered_Rejected_Tx_Pkts);
    LOG_AT("multicast_Filtered_Rejected_Rx_Pkts = 0x%x\r\n", pDevWlanStats->multicast_Filtered_Rejected_Rx_Pkts);
    LOG_AT("broadcast_Filtered_Rejected_Tx_Pkts = 0x%x\r\n", pDevWlanStats->broadcast_Filtered_Rejected_Tx_Pkts);
    LOG_AT("broadcast_Filtered_Rejected_Rx_Pkts = 0x%x\r\n", pDevWlanStats->broadcast_Filtered_Rejected_Rx_Pkts);

    LOG_AT("null_Tx_Pkts = 0x%x\r\n", pDevWlanStats->null_Tx_Pkts);
    LOG_AT("null_Rx_Pkts = 0x%x\r\n", pDevWlanStats->null_Rx_Pkts);
    LOG_AT("qos_null_Tx_Pkts = 0x%x\r\n", pDevWlanStats->qos_Null_Tx_Pkts);
    LOG_AT("qos_null_Rx_Pkts = 0x%x\r\n", pDevWlanStats->qos_Null_Rx_Pkts);
    LOG_AT("ps_poll_Tx_Pkts = 0x%x\r\n", pDevWlanStats->ps_Poll_Tx_Pkts);
    LOG_AT("ps_poll_Rx_Pkts = 0x%x\r\n", pDevWlanStats->ps_Poll_Rx_Pkts);
    LOG_AT("tx_retry_Cnt = 0x%x\r\n", pDevWlanStats->tx_Retry_Cnt);

    LOG_AT("beacon_miss_Cnt = 0x%x\r\n", pDevWlanStats->beacon_Miss_Cnt);
    LOG_AT("beacons_received_Cnt = 0x%x\r\n", pDevWlanStats->beacons_Received_Cnt);
    LOG_AT("beacon_resync_success_Cnt = 0x%x\r\n", pDevWlanStats->beacon_Resync_Success_Cnt);
    LOG_AT("beacon_resync_failure_Cnt = 0x%x\r\n", pDevWlanStats->beacon_Resync_Failure_Cnt);
    LOG_AT("curr_early_wakeup_adj_in_ms = 0x%x\r\n", pDevWlanStats->curr_Early_Wakeup_Adj_In_Ms);
    LOG_AT("avg_early_wakeup_adj_in_ms = 0x%x\r\n", pDevWlanStats->avg_Early_Wakeup_Adj_In_Ms);
    LOG_AT("early_termination_Cnt = 0x%x\r\n", pDevWlanStats->early_Termination_Cnt);

    LOG_AT("uapsd_trigger_Rx_Cnt = 0x%x\r\n", pDevWlanStats->uapsd_Trigger_Rx_Cnt);
    LOG_AT("uapsd_trigger_Tx_Cnt = 0x%x\r\n", pDevWlanStats->uapsd_Trigger_Tx_Cnt);

    LOG_AT("\r\n **** COMMON WLAN Statistics **** \r\n" );

    LOG_AT("total_active_time_in_ms = 0x%x\r\n", pCommonWlanStats->total_Active_Time_In_Ms);
    LOG_AT("total_powersave_time_in_ms = 0x%x\r\n", pCommonWlanStats->total_Powersave_Time_In_Ms);

    return QCLI_STATUS_SUCCESS_E;
}

/***********************************************************************************************
 * Function Name  : qc_api_wifi_print_scan_results
 * Returned Value : 0 on success
 * Comments       : Prints the WLAN scan results
 **********************************************************************************************/
int32_t qc_api_wifi_print_scan_results(qapi_WLAN_Scan_List_t param, uint16_t num_scan)
{
    int32_t i = 0;
    uint8_t temp_ssid[33] = {0};
    qapi_WLAN_BSS_Scan_Info_t *list = NULL;

    LOG_AT("Scan result count:%d\r\n\n", num_scan);
    list = (qapi_WLAN_BSS_Scan_Info_t *)(param.scan_List);
    for (i = 0;i<num_scan;i++)
    {
        memcpy(temp_ssid,list[i].ssid,list[i].ssid_Length);
        temp_ssid[list[i].ssid_Length] = '\0';
        if (list[i].ssid_Length == 0)
        {
            LOG_AT("ssid = SSID Not available\r\n\n");
        }
        else
        {
#if QC_API_ENABLE_SCC_MODE
            if((concurrent_connect_flag == 0x0F) &&
                    (strcmp((int8_t const *)ssid_str_concurrent, (int8_t const *)temp_ssid) == 0))
            {
                ap_channel_hint = list[i].channel;
                LOG_AT("scan res ch : %d\r\n", ap_channel_hint);
                return QCLI_STATUS_SUCCESS_E;
            }
            else if((concurrent_connect_flag == 0x0F))
            {
                return QCLI_STATUS_SUCCESS_E;
            }
            else
#endif
            {
                LOG_AT("ssid      = %s\r\n",temp_ssid);
                LOG_AT("bssid     = %.2x:%.2x:%.2x:%.2x:%.2x:%.2x\r\n",
                        list[i].bssid[0],list[i].bssid[1],
                        list[i].bssid[2],list[i].bssid[3],
                        list[i].bssid[4],list[i].bssid[5]);
                LOG_AT("channel   = %d\r\n",list[i].channel);
                LOG_AT("indicator = %d\r\n",list[i].rssi);
                LOG_AT("security  = ");
                if(list[i].security_Enabled){
                    if(list[i].rsn_Auth || list[i].rsn_Cipher){
                        LOG_AT("RSN/WPA2 = ");
                    }
                    if(list[i].rsn_Auth){
                        LOG_AT(" { ");
                        if(list[i].rsn_Auth & __QAPI_WLAN_SECURITY_AUTH_1X){
                            LOG_AT("802.1X ");
                        }
                        if(list[i].rsn_Auth & __QAPI_WLAN_SECURITY_AUTH_PSK){
                            LOG_AT("PSK ");
                        }
                        LOG_AT("}");
                    }
                    if(list[i].rsn_Cipher){
                        LOG_AT(" { ");
                        /* AP security can support multiple options hence
                         * we check each one separately. Note rsn == wpa2 */
                        if(list[i].rsn_Cipher & __QAPI_WLAN_CIPHER_TYPE_WEP){
                            LOG_AT("WEP ");
                        }

                        if(list[i].rsn_Cipher & __QAPI_WLAN_CIPHER_TYPE_TKIP){
                            LOG_AT("TKIP ");
                        }
                        if(list[i].rsn_Cipher & __QAPI_WLAN_CIPHER_TYPE_CCMP){
                            LOG_AT("AES ");
                        }
                        LOG_AT("}\r\n");
                    }
                    if(list[i].wpa_Auth || list[i].wpa_Cipher){
                        LOG_AT("            WPA      = ");
                    }
                    if(list[i].wpa_Auth){
                        LOG_AT(" { ");
                        if(list[i].wpa_Auth & __QAPI_WLAN_SECURITY_AUTH_1X){
                            LOG_AT("802.1X ");
                        }
                        if(list[i].wpa_Auth & __QAPI_WLAN_SECURITY_AUTH_PSK){
                            LOG_AT("PSK ");
                        }
                        LOG_AT("}");
                    }

                    if(list[i].wpa_Cipher){
                        LOG_AT(" { ");
                        if(list[i].wpa_Cipher & __QAPI_WLAN_CIPHER_TYPE_WEP){
                            LOG_AT("WEP ");
                        }

                        if(list[i].wpa_Cipher & __QAPI_WLAN_CIPHER_TYPE_TKIP){
                            LOG_AT("TKIP ");
                        }

                        if(list[i].wpa_Cipher & __QAPI_WLAN_CIPHER_TYPE_CCMP){
                            LOG_AT("AES ");
                        }
                        LOG_AT("}\r\n");
                    }
                    /* it may be old-fashioned WEP this is identified by
                     * absent wpa and rsn ciphers */
                    if(list[i].rsn_Cipher == 0 &&
                            list[i].wpa_Cipher == 0){
                        LOG_AT("WEP ");
                    }
                }else{
                    LOG_AT("NONE!\r\n");
                }
            }
        }

        if(i!= param.num_Scan_Entries-1)
        {
            LOG_AT("\r\n");
        }
        else
        {
            LOG_AT("\r\n\n");
        }
    }

    return QCLI_STATUS_SUCCESS_E;
}

/***********************************************************************************************
 * Function Name  : qc_api_wifi_wmi_set_scan
 * Returned Value : 0 on success, -1 on error.
 * Comments       : Starts the WLAN scan
 **********************************************************************************************/
int32_t qc_api_wifi_wmi_set_scan(uint32_t dev_num, void *start_scan, qapi_WLAN_Store_Scan_Results_e store_scan_results)
{
    int32_t error = 0;

    qc_api_wifi_set_callback(NULL);
    // if (dev_num < BSP_ENET_DEVICE_COUNT)
    {
        qapi_WLAN_Store_Scan_Results_e storeScanResults = (qapi_WLAN_Store_Scan_Results_e) store_scan_results; //QAPI_WLAN_BUFFER_SCAN_RESULTS_BLOCKING_E;
        error = qc_drv_wlan_start_scan(qc_api_get_qc_drv_context(),
                dev_num,
                (qapi_WLAN_Start_Scan_Params_t *) start_scan,
                storeScanResults);
        if (0 != error)
        {
            return error;
        }

        if((store_scan_results == QAPI_WLAN_BUFFER_SCAN_RESULTS_NON_BLOCKING_E) ||
                (store_scan_results == QAPI_WLAN_NO_BUFFERING_E))
            return error;

        scan_param.scan_List = malloc((sizeof(qapi_WLAN_BSS_Scan_Info_t) * __QAPI_MAX_SCAN_RESULT_ENTRY));
        scan_param.num_Scan_Entries = __QAPI_MAX_SCAN_RESULT_ENTRY;
        error = qc_drv_wlan_get_scan_results(qc_api_get_qc_drv_context(), dev_num,
                (qapi_WLAN_BSS_Scan_Info_t*)(scan_param.scan_List),
                (int16_t *)&(scan_param.num_Scan_Entries));
        if (0 != error)
        {
            return error;
        }
    }
    return error;
}

/***********************************************************************************************
 * Function Name  : qc_api_wifi_set_power_mode
 * Returned Value : NA
 * Comments       : Sets the WLAN power mode
 **********************************************************************************************/
uint32_t qc_api_wifi_set_power_mode(boolean_t pwr_mode, uint8_t pwr_module)
{
    uint32_t deviceId = qc_api_wifi_get_active_device();
    qapi_WLAN_Power_Mode_Params_t pwrMode;

    pwrMode.power_Mode = pwr_mode;
    pwrMode.power_Module = pwr_module;
    return qc_drv_wlan_set_power_param(qc_api_get_qc_drv_context(),
            deviceId,
            (void *) &pwrMode,
            sizeof(pwrMode),
            FALSE);
}

/***********************************************************************************************
 * Function Name  : qc_api_wifi_strtomac
 * Returned Value : 0 on success
 * Comments       : converts a string to a 6 byte MAC address.
 **********************************************************************************************/
static int32_t qc_api_wifi_strtomac(const int8_t* arg, uint8_t* dst)
{
    int32_t i = 0, j = 0, left_nibble = 1;
    int8_t base_char;
    uint8_t base_val;

    if(dst == NULL || arg == NULL){
        return QCLI_STATUS_ERROR_E;
    }

    memset(dst, 0, 6);

    do{
        if(arg[i] >= '0' && arg[i] <= '9'){
            base_char = '0';
            base_val = 0x00;
        }else if(arg[i] == ':'){
            base_char = ':';
        }else if(arg[i] >= 'a' && arg[i] <= 'f'){
            base_char = 'a';
            base_val = 0x0a;
        }else if(arg[i] >= 'A' && arg[i] <= 'F'){
            base_char = 'A';
            base_val = 0x0a;
        }else{
            return QCLI_STATUS_ERROR_E;//error
        }

        if(base_char != ':'){
            dst[j] |= (arg[i] - base_char + base_val)<<(4*left_nibble);
            left_nibble = (left_nibble)? 0:1;

            if(left_nibble){
                j++;

                if(j>5){
                    break;
                }
            }
        }

        i++;
    }while(1);

    return QCLI_STATUS_SUCCESS_E;//success
}

/***********************************************************************************************
 * Function Name  : get_wifi_power_mode
 * Returned Value : 0 on success, 1 on error.
 * Comments       : Called from application to retrieve WiFi power mode for active device.
 ***********************************************************************************************/
int32_t qc_api_wifi_get_wifi_power_mode(char *data)
{
    uint32_t mode = 0, deviceId = 0, dataLen = 0;

    deviceId = qc_api_wifi_get_active_device();
    if (0 != qc_drv_wlan_get_power_param(qc_api_get_qc_drv_context(),
                deviceId,
                &mode,
                &dataLen))
    {
        QCLI_Printf("Command failed.\r\n");
        return QCLI_STATUS_ERROR_E;
    }
    if(mode == QAPI_WLAN_POWER_MODE_REC_POWER_E)
    {
        strcpy(data,"Power Save");
    }
    else
    {
        strcpy(data,"Max Perf");
    }
    return QCLI_STATUS_SUCCESS_E;
}

/***********************************************************************************************
 * Function Name  : get_phy_mode
 * Returned Value : 0 on success, 1 on error.
 * Comments       : Called from application to retrieve device's wireless mode.
 ***********************************************************************************************/
int32_t qc_api_wifi_get_phy_mode(char *data)
{
    uint32_t deviceId = 0, dataLen = 0;
    qapi_WLAN_Phy_Mode_e phyMode;
    qapi_WLAN_11n_HT_Config_e htconfig;

    deviceId = qc_api_wifi_get_active_device();
    if (0 != qc_drv_wlan_get_phy_mode(qc_api_get_qc_drv_context(), deviceId,
                &phyMode,
                &dataLen))
    {
        return QCLI_STATUS_ERROR_E;
    }
    if (phyMode == QAPI_WLAN_11A_MODE_E)
        strcpy(data, "a");
    else if (phyMode == QAPI_WLAN_11G_MODE_E)
        strcpy(data, "g");
    else if (phyMode == QAPI_WLAN_11AG_MODE_E)
        strcpy(data, "ag");
    else if (phyMode == QAPI_WLAN_11B_MODE_E)
        strcpy(data, "b");
    else if (phyMode == QAPI_WLAN_11GONLY_MODE_E)
        strcpy(data, "gonly");

    if (phyMode != QAPI_WLAN_11B_MODE_E) {
        qc_drv_wlan_get_ht_config(qc_api_get_qc_drv_context(), deviceId,
                &htconfig,
                &dataLen);
        strcat(data, "(HT: HT MODE ");
        if (htconfig == QAPI_WLAN_11N_DISABLED_E) {
            strcat(data, "off)");
        } else if (htconfig == QAPI_WLAN_11N_HT20_E) {
            strcat(data, "20M)");
        } else if (htconfig == QAPI_WLAN_11N_HT40_E) {
            strcat(data, "40M)");
        }
    }

    return QCLI_STATUS_SUCCESS_E;
}

/***********************************************************************************************
 * Function Name  : get_ssid
 * Returned Value : 0 on success, 1 on error.
 * Comments       : Called from application to retrieve SSID of the network if the active device
 *                  is connected.
 ***********************************************************************************************/
int32_t qc_api_wifi_get_ssid(char *ssid)
{
    uint32_t deviceId = 0, dataLen = 0;

    deviceId = qc_api_wifi_get_active_device();
    if (0 != qc_drv_wlan_get_ssid(qc_api_get_qc_drv_context(),
                deviceId,
                (void *) ssid,
                &dataLen))
    {
        return QCLI_STATUS_ERROR_E;
    }

    if(!((qc_api_wifi_get_dev_stat(deviceId) == UP) && (*ssid != '\0')))
    {
        strcpy(ssid, "Device not connected");
    }

    return QCLI_STATUS_SUCCESS_E;
}

/***********************************************************************************************
 * Function Name  : get_device_mac_address
 * Returned Value : 0 on success, 1 on error.
 * Comments       : Called from application to retrieve MAC address for the active device.
 ***********************************************************************************************/
int32_t qc_api_wifi_get_device_mac_address(char *mac)
{
    uint32_t deviceId = 0, macLen = 0;

    deviceId = qc_api_wifi_get_active_device();

    if (0 != qc_drv_wlan_get_mac_addr((qc_drv_context *)qc_api_get_qc_drv_context(),
                deviceId,
                (uint8_t *)mac,
                &macLen))
    {
        return QCLI_STATUS_ERROR_E;
    }

    return QCLI_STATUS_SUCCESS_E;
}

/***********************************************************************************************
 * Function Name  : get_op_mode
 * Returned Value : 0 on success, 1 on error.
 * Comments       : Called from application to retrieve operating mode for the active device.
 ***********************************************************************************************/
int32_t qc_api_wifi_get_op_mode()
{
    uint32_t mode = 0, deviceId = 0, dataLen = 0;

    deviceId = qc_api_wifi_get_active_device();
    if (0 != qc_drv_wlan_get_op_mode(qc_api_get_qc_drv_context(),
                deviceId,
                &mode,
                &dataLen))
    {
        return (-QCLI_STATUS_ERROR_E);
    }

    return mode;
}

/***********************************************************************************************
 * Function Name  : get_channel
 * Returned Value : -1 on error and channel number on success.
 * Comments       : Called from application to retrieve channel for the active device if
 *                  connected.
 ***********************************************************************************************/
int32_t qc_api_wifi_get_channel()
{
    uint16_t channel_val = 0;
    uint32_t deviceId = 0, dataLen = 0;

    deviceId = qc_api_wifi_get_active_device();
    if(qc_api_wifi_get_dev_stat(deviceId) == UP)
    {
        if (0 != qc_drv_wlan_get_channel(qc_api_get_qc_drv_context(), deviceId,
                    &channel_val,
                    &dataLen))
        {
            return (-QCLI_STATUS_ERROR_E);
        } else {
            return channel_val;
        }
    }
    else
    {
        LOG_AT("Channel      =   Interface is down.\r\n");
        return (-QCLI_STATUS_ERROR_E);
    }
}

/***********************************************************************************************
 * Function Name  : print_wlan_info
 * Returned Value : 0 on success
 * Comments       : Called from application to retrieve information for active device.
 ***********************************************************************************************/
int32_t qc_api_wifi_print_wlan_info()
{
    uint32_t deviceId = 0, op_mode = 0;
    uint16_t channel_val = 0;
    char ssid[32+1] = {'\0'};
    char phy_mode[32+1] = {'\0'};
    char pw_mode[32+1] = {'\0'};
    char mac[32+1]     = {'\0'};

    deviceId = qc_api_wifi_get_active_device();
    if (0 != qc_api_wifi_get_ssid(ssid))
    {
        return QCLI_STATUS_ERROR_E;
    }
    channel_val = qc_api_wifi_get_channel();
    if (-1 == channel_val)
    {
        return QCLI_STATUS_ERROR_E;
    }

    if (0 != qc_api_wifi_get_phy_mode(phy_mode))
    {
        return QCLI_STATUS_ERROR_E;
    }
    if (0 != qc_api_wifi_get_wifi_power_mode(pw_mode))
    {
        return QCLI_STATUS_ERROR_E;
    }
    if (0 != qc_api_wifi_get_device_mac_address(mac))
    {
        return QCLI_STATUS_ERROR_E;
    }
    op_mode = qc_api_wifi_get_op_mode();
    if (op_mode == -1)
    {
        return QCLI_STATUS_ERROR_E;
    }

    LOG_AT("Interface    =   Wlan%d\r\n", deviceId);
    LOG_AT("Mac Addr     =   %02x:%02x:%02x:%02x:%02x:%02x\r\n",
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
#if QC_API_ENABLE_AP_MODE
    if(op_mode == QAPI_WLAN_DEV_MODE_AP_E)
    {
        LOG_AT("Mode         =   softap\r\n");
        if(pmk_flag[deviceId])
        {
            LOG_AT("Passphrase   =   %s\r\n",wpa_passphrase[deviceId]);
        }
    }
    else
#endif
        if(op_mode == QAPI_WLAN_DEV_MODE_STATION_E)
        {
            LOG_AT("Mode         =   station \r\n");
        }
        else
        {
            LOG_AT("Mode         =   adhoc \r\n");
        }
    if(qc_api_wifi_get_dev_stat(deviceId) == UP)
    {
        LOG_AT("Ssid         =   %s\r\n", ssid);
        LOG_AT("Channel      =   %d\r\n", channel_val);
    }
    LOG_AT("Phy Mode     =   %s\r\n", phy_mode);
    LOG_AT("Power mode   =   %s\r\n", pw_mode);

    return QCLI_STATUS_SUCCESS_E;
}

/***********************************************************************************************
 * Function Name  : get_wep_key
 * Returned Value : -1 on error else 0
 * Comments       : Get WEP key at given index.
 *                  Hex characters
 _STATUS_SUCCESS_E***********************************************************************************************/
int32_t qc_api_wifi_get_wep_key(uint8_t index)
{
    uint32_t error = 0, deviceId = 0, dataLen = 0, hex_i = 0, ascii_i = 0;
    qapi_WLAN_Security_Wep_Key_Pair_Params_t keyPair;
    int8_t get_key[__QAPI_WLAN_MAX_WEP_KEY_SZ+1] = {'\0'};

    keyPair.key_Index = index;
    keyPair.key_Length = 0;
    keyPair.key = (int8_t *)malloc(__QAPI_WLAN_MAX_WEP_KEY_SZ+1);
    memset(keyPair.key, 0, __QAPI_WLAN_MAX_WEP_KEY_SZ+1);
    memset(get_key, 0, __QAPI_WLAN_MAX_WEP_KEY_SZ+1);

    deviceId = qc_api_wifi_get_active_device();
    error = qc_drv_wlan_get_wep_key_pair(qc_api_get_qc_drv_context(),
            deviceId,
            &keyPair, &dataLen);
    if(error != 0)
    {
        free(keyPair.key);
        return error;
    }

    if (key_type & (1 << (index - 1))) /* Hex input */
    {
        for (ascii_i = 0; hex_i < keyPair.key_Length ; ascii_i++,hex_i++)
        {
            get_key[ascii_i++] = hex_to_ascii((uint8_t)keyPair.key[hex_i] >> 4);
            get_key[ascii_i] = hex_to_ascii((uint8_t)keyPair.key[hex_i] & 0xF);
        }
    }
    else /* Ascii input */
    {
        memcpy((int8_t *)get_key, keyPair.key, keyPair.key_Length);
    }

    QCLI_Printf("Key: %s\r\n", get_key);
    free(keyPair.key);
    return QCLI_STATUS_SUCCESS_E;
}

/***********************************************************************************************
 * Function Name   : get_wep_keyix()
 * Returned Value  : 0 - successful completion or
 *                   -1 - failed.
 * Comments        : Get index of default WEP key.
 ***********************************************************************************************/
int32_t qc_api_wifi_get_wep_keyix()
{
    uint32_t error = 0, keyIndex = 0, deviceId = 0, dataLen = 0;
    deviceId = qc_api_wifi_get_active_device();
    error = qc_drv_wlan_get_wep_key_index(qc_api_get_qc_drv_context(),
            deviceId,
            &keyIndex, &dataLen);
    if(error != 0)
    {
        return error;
    }
    LOG_AT("Key index: %d\n", keyIndex);
    return QCLI_STATUS_SUCCESS_E;
}

/***********************************************************************************************
 * Function Name  : qc_api_wifi_wifi_connect_handler
 * Returned Value : N/A
 * Comments       : Called from driver on a WiFI connection event
 ***********************************************************************************************/
void qc_api_wifi_connect_handler(int32_t val, uint8_t devId, uint8_t * mac, boolean_t bssConn)
{
    uint8_t disc_bss[6] ={0xff,0xff,0xff,0xff,0xff,0xff};
    uint8_t temp_bss[__QAPI_WLAN_MAC_LEN] ={0};

    if(devId < QC_API_DEV_NUM)
        wifi_state[devId] = val;
    else
        LOG_AT_EVT("EVT_WIFI: The devid exceeds the max device number.\r\n");

    if(val == TRUE)
    {
        LOG_AT_EVT("EVT_WIFI: %d %s %s MAC addr %02x:%02x:%02x:%02x:%02x:%02x \r\n",
                devId, (bssConn == 0) ? "AP" : "STA", "CONNECTED",
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        if(bssConn ){
            qc_api_wifi_set_dev_stat(devId,UP);
            concurrent_connect_flag = 0x0E;
            memcpy(g_bssid[devId],mac,__QAPI_WLAN_MAC_LEN);
        }
        //TURN_LED_ON;
    }
    else if(val == QAPI_WLAN_INVALID_PROFILE_E) // this event is used to indicate RSNA failure
    {
        LOG_DEBUG("4 way handshake failure for device = %d \r\n",devId);
    }
    else if(val == 0x10 /*PEER_FIRST_NODE_JOIN_EVENT*/) //this event is used to RSNA success
    {
        LOG_DEBUG("4 way handshake success for device = %d \r\n",devId);
    }
    else if(val == FALSE)
    {
        /*   dev status for disconnect events
             MODE          wmi --disc event     REF_STA Disconnect
             STA MODE        DOWN               INVALID
             AP  MODE        DOWN                UP
             P2P CLIENT......DOWN               INVALID
             P2P GO..........DOWN                UP
             */
        if (bssConn ){
            qc_api_wifi_set_dev_stat(devId,DOWN);
            if( memcmp(mac,disc_bss,__QAPI_WLAN_MAC_LEN) == 0){
                /*disabling flags in case of AP/GO mode*/
                hidden_flag = 0;
                wps_flag = 0;
            }
            if((memcmp(mac,temp_bss,__QAPI_WLAN_MAC_LEN) == 0) &&
                    (memcmp(g_bssid[devId],temp_bss,__QAPI_WLAN_MAC_LEN) != 0))
            {
                LOG_AT_EVT("EVT_WIFI: %d %s Disconnected MAC addr %02x:%02x:%02x:%02x:%02x:%02x \r\n",
                        devId, (bssConn == 0) ? "AP" : "STA", g_bssid[devId][0], g_bssid[devId][1],
                        g_bssid[devId][2], g_bssid[devId][3],
                        g_bssid[devId][4], g_bssid[devId][5]);
            }
            else
            {
                LOG_AT_EVT("EVT_WIFI: %d %s Disconnected MAC addr %02x:%02x:%02x:%02x:%02x:%02x \r\n",
                        devId, (bssConn == 0) ? "AP" : "STA", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
            }

        }
        /* bssConn is flase for REF-STA disconnected from AP/p2pGO: dont update the dev stat*/
        else
        {
            LOG_AT_EVT("EVT_WIFI: %d REF_STA Disconnected MAC addr %02x:%02x:%02x:%02x:%02x:%02x \r\n",
                    devId, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        }
        concurrent_connect_flag = 0x00;
        //TURN_LED_OFF;
        memcpy(g_bssid[devId],temp_bss,__QAPI_WLAN_MAC_LEN);
    }
    else
    {
        LOG_AT_EVT("EVT_WIFI: %d last tx rate : %d kbps\r\n", devId, val);
    }
    if(wps_context.wps_in_progress && devId==qc_api_wifi_get_active_device())
    {
        wps_context.wps_in_progress=0;
        qc_api_wifi_set_power_mode(QAPI_WLAN_POWER_MODE_REC_POWER_E, QAPI_WLAN_POWER_MODULE_WPS_E);
    }
    QCLI_Display_Prompt();
}

/***********************************************************************************************
 * Function Name   : NetConnect()
 * Returned Value  : 0 : success, -1 : failure
 * Comments        : Handles connection to AP after WPS
 ***********************************************************************************************/
int32_t qc_api_wifi_NetConnect(qapi_WLAN_Netparams_t *pNetparams)
{
    int32_t status = -1;
    qapi_WLAN_WPS_Credentials_t wpsCred;
    uint32_t deviceId = qc_api_wifi_get_active_device();
    uint32_t passphraseLen = 0 ;
    memset(&wpsCred, 0, sizeof(qapi_WLAN_WPS_Credentials_t));

    do{
        if(pNetparams->ssid_Len == 0)
        {
            LOG_AT_EVT("EVT_WIFI: %d WPS failed\r\n", deviceId);
            break;
        }
        else
        {
            LOG_DEBUG("SSID: %s\r\n",pNetparams->ssid);
            if(pNetparams->sec_Type == QAPI_WLAN_AUTH_WPA2_PSK_E)
            {
                LOG_DEBUG("Security Type: WPA2\r\n");
                LOG_DEBUG("Passphrase: %s\r\n",(int8_t*)pNetparams->u.passphrase);
            }
            else if(pNetparams->sec_Type == QAPI_WLAN_AUTH_WPA_PSK_E)
            {
                LOG_DEBUG("Security Type: WPA\r\n");
                LOG_DEBUG("Passphrase: %s\r\n",(int8_t*)pNetparams->u.passphrase);
            }
            else if(pNetparams->sec_Type == QAPI_WLAN_AUTH_WEP_E)
            {
                LOG_DEBUG("Security Type: WEP\r\n");
                LOG_DEBUG("WEP key: %s\r\n",(int8_t*)pNetparams->u.wepkey);
                LOG_DEBUG("Key index: %d\r\n",pNetparams->key_Index);
            }
            else if(pNetparams->sec_Type == QAPI_WLAN_AUTH_NONE_E)
            {
                LOG_DEBUG("Security Type: None\r\n");
            }

            qc_api_wifi_set_callback(NULL);


            wpsCred.ssid_Length = strlen((const char *)pNetparams->ssid);
            strncpy((char *) wpsCred.ssid, (char *)pNetparams->ssid, wpsCred.ssid_Length);
            strncpy((char *)original_ssid,(char *)pNetparams->ssid, wpsCred.ssid_Length);

            qc_drv_wlan_set_ssid(qc_api_get_qc_drv_context(),
                    deviceId,
                    (uint8_t *)pNetparams->ssid,
                    sizeof(pNetparams->ssid),
                    0);

            if(pNetparams->sec_Type == QAPI_WLAN_AUTH_WPA2_PSK_E){
                wpsCred.auth_Mode = QAPI_WLAN_AUTH_WPA2_PSK_E;
                wpsCred.encryption_Type = pNetparams->cipher.ucipher;
                passphraseLen = strlen((const char *)pNetparams->u.passphrase);
                if(passphraseLen != 64)
                {
                    if (0 != qc_drv_wlan_set_passphrase(qc_api_get_qc_drv_context(),
                                deviceId,
                                (void *) pNetparams->u.passphrase,
                                passphraseLen,
                                FALSE))
                        break;
                }
                else
                {
                    if(0 != qc_drv_wlan_set_security_pmk(qc_api_get_qc_drv_context(),
                                deviceId,
                                (void *) pNetparams->u.passphrase,
                                passphraseLen,
                                FALSE))
                        break;
                }
            }else if (pNetparams->sec_Type == QAPI_WLAN_AUTH_WPA_PSK_E){
                wpsCred.auth_Mode = QAPI_WLAN_AUTH_WPA_PSK_E;
                wpsCred.encryption_Type = pNetparams->cipher.ucipher;
                if(passphraseLen != 64)
                {
                    if (0 != qc_drv_wlan_set_passphrase(qc_api_get_qc_drv_context(),
                                deviceId,
                                (void *) pNetparams->u.passphrase,
                                passphraseLen,
                                FALSE))
                        break;
                }
                else
                {
                    if(0 != qc_drv_wlan_set_security_pmk(qc_api_get_qc_drv_context(),
                                deviceId,
                                (void *) pNetparams->u.passphrase,
                                passphraseLen,
                                FALSE))
                        break;
                }
            }else if (pNetparams->sec_Type == QAPI_WLAN_AUTH_WEP_E){
                wpsCred.key_Index = pNetparams->key_Index;
                strcpy((char *)wpsCred.key, (const char *)pNetparams->u.wepkey);
                wpsCred.key_Length = strlen((const char *)pNetparams->u.wepkey);
            }else if (pNetparams->sec_Type == QAPI_WLAN_AUTH_NONE_E){
                wpsCred.auth_Mode = QAPI_WLAN_AUTH_NONE_E;
            }
            qc_drv_wlan_set_wps_credentials(qc_api_get_qc_drv_context(),
                    deviceId,
                    &wpsCred,
                    sizeof(qapi_WLAN_WPS_Credentials_t),
                    FALSE);
            qc_drv_wlan_wps_connect(qc_api_get_qc_drv_context(), deviceId);
            status = 0;
        }
    }while(0);

    return status;
}

/***********************************************************************************************
 * Function Name   : CompleteWPS()
 * Returned Value  : 0 - success, 1 - failure
 * Comments        : Complete the WPS operation
 ***********************************************************************************************/
uint32_t qc_api_wifi_CompleteWPS(uint32_t deviceId, qapi_WLAN_Netparams_t *pNetparams, uint8_t block)
{
    uint32_t status = 0, error = 0, wifimode = 0, dataLen = 0;

    pNetparams->error = 0;
    pNetparams->dont_Block = (block)? 0:1;

    error = qc_drv_wlan_wps_await_completion(qc_api_get_qc_drv_context(),
            deviceId, pNetparams);

    qc_drv_wlan_get_op_mode(qc_api_get_qc_drv_context(),
            deviceId,
            &wifimode,
            &dataLen);

    if (wifimode == QAPI_WLAN_DEV_MODE_STATION_E) {
        if(error != 0)
        {
            /* This case occurs when event from target is not received yet
             * and qc_drv_WLAN_WPS_Await_Completion returns PENDING */
            pNetparams->error = pNetparams->error ? pNetparams->error : 13;//A_EBUSY;
        }
    }

    do{
        if (error == QAPI_WLAN_ERR_PENDING)
        {
            break;
        }

        status = 1;

        if((error != 0) && (pNetparams->error == 0)){
            pNetparams->error = error;
            break;
        }

        if(pNetparams->error != 0)
        {
            switch (pNetparams->error)
            {
                case QAPI_WLAN_WPS_ERROR_INVALID_START_INFO_E:
                    LOG_AT_EVT("EVT_WIFI: %d WPS error: invalid start info\r\n", deviceId);
                    break;

                case QAPI_WLAN_WPS_ERROR_MULTIPLE_PBC_SESSIONS_E:
                    LOG_AT_EVT("EVT_WIFI: %d WPS error: Multiple PBC Sessions\r\n", deviceId);
                    break;

                case QAPI_WLAN_WPS_ERROR_WALKTIMER_TIMEOUT_E:
                    LOG_AT_EVT("EVT_WIFI: %d WPS error: Walktimer Timeout\r\n", deviceId);
                    break;

                case QAPI_WLAN_WPS_ERROR_M2D_RCVD_E:
                    LOG_AT_EVT("EVT_WIFI: %d WPS error: M2D RCVD\r\n", deviceId);
                    break;

                case QAPI_WLAN_WPS_ERROR_PWD_AUTH_FAIL_E:
                    LOG_AT_EVT("EVT_WIFI: %d WPS error: AUTH FAIL\r\n", deviceId);
                    break;

                case QAPI_WLAN_WPS_ERROR_CANCELLED_E:
                    LOG_AT_EVT("EVT_WIFI: %d WPS error: WPS CANCEL\r\n", deviceId);
                    break;

                case QAPI_WLAN_WPS_ERROR_INVALID_PIN_E:
                    LOG_AT_EVT("EVT_WIFI: %d WPS error: INVALID PIN\r\n", deviceId);
                    break;

                default:
                    LOG_AT_EVT("EVT_WIFI: %d WPS error: unknown %d\r\n",pNetparams->error, deviceId);
                    break;
            }
        }
    }while(0);

    if (wifimode == QAPI_WLAN_DEV_MODE_AP_E)
    {
        if (0x00 == pNetparams->error && 0 == error) {
            LOG_DEBUG("***** WPS PROFILE ****\r\n");
            LOG_DEBUG("SSID: %s\r\n", pNetparams->ssid);
            if (pNetparams->sec_Type == QAPI_WLAN_AUTH_WPA2_PSK_E)
            {
                LOG_DEBUG("Security Type: WPA2\r\n");
                LOG_DEBUG("Encryption Type: AES\r\n");
                LOG_DEBUG("Passphrase: %s\r\n",(int8_t*)pNetparams->u.passphrase);
            }else if(pNetparams->sec_Type == QAPI_WLAN_AUTH_WPA_PSK_E)
            {
                LOG_DEBUG("Security Type: WPA\r\n");
                LOG_DEBUG("Encryption Type: AES\r\n");
                LOG_DEBUG("Passphrase: %s\r\n",(int8_t*)pNetparams->u.passphrase);
            }
            else
            {
                LOG_DEBUG("Security Type: Open\r\n");
            }
            status = 0;
        }
        else
        {
            pNetparams->error = (pNetparams->error)?pNetparams->error: 13; //A_EBUSY;
            status = 0;
        }

    }
    QCLI_Display_Prompt();
    return status;
}

/***********************************************************************************************
 * Function Name   : qc_api_wifi_wps_query()
 * Returned Value  : 0 - success, 1 - failure
 * Comments        : Queries WPS status
 ***********************************************************************************************/
int32_t qc_api_wifi_wps_query(uint8_t block)
{
    qapi_WLAN_Netparams_t netparams;
    int32_t status = 1;
    uint32_t deviceId = 0, wifimode = 0, dataLen = 0;

    deviceId = qc_api_wifi_get_active_device();
    qc_drv_wlan_get_op_mode(qc_api_get_qc_drv_context(),
            deviceId,
            &wifimode,
            &dataLen);

    if (qc_api_wifi_CompleteWPS(deviceId, &netparams, block))
    {
        memcpy(&wps_context.netparams, &netparams, sizeof(qapi_WLAN_Netparams_t));

        if(netparams.error)
        {
            wps_context.connect_flag=0;
        }

        if(wps_context.connect_flag)
        {
#if QC_API_ENABLE_SCC_MODE
            do
            {
                if((concurrent_connect_flag == 0x0E) && (deviceId == 1) )
                {
                    LOG_AT_EVT("EVT_WIFI: %d ap_ch:%d\r\n", deviceId, netparams.ap_channel);

                    if ( 0 != qc_drv_wlan_set_channel(qc_api_get_qc_drv_context(),
                                deviceId,
                                (void *) &netparams.ap_channel,
                                sizeof(netparams.ap_channel),
                                FALSE))
                    {
                        LOG_AT_EVT("EVT_WIFI: %d Unable to set to AP channel:%d\r\n", deviceId, netparams.ap_channel);
                        break;
                    }
                    A_MDELAY(50);

                }

                if (qc_api_wifi_(NetConnect(&netparams)))
                {
                    LOG_AT_EVT("EVT_WIFI: %d wps connection failed\r\n", deviceId);
                }
            } while(0);
#else
            if (qc_api_wifi_NetConnect(&netparams))
            {
                LOG_AT_EVT("EVT_WIFI: %d connection failed\r\n", deviceId);
            }
#endif
        }

        status = 0;
        qc_api_wifi_set_power_mode( QAPI_WLAN_POWER_MODE_REC_POWER_E, QAPI_WLAN_POWER_MODULE_WPS_E);

        LOG_AT_EVT("EVT_WIFI: %d WPS completed\r\n", deviceId);
    }
    else
    {
        /* Driver has not received the WPS event from the firmware yet. */
        if (netparams.error == 13)
        {
            /* Signal will be set when the WPS profile event is received from the driv er */
        }
        else
        {
            qc_api_wifi_set_power_mode( QAPI_WLAN_POWER_MODE_REC_POWER_E, QAPI_WLAN_POWER_MODULE_WPS_E);
            memcpy(&wps_context.netparams,&netparams,sizeof(qapi_WLAN_Netparams_t));
            status = 0;
        }
    }

    wps_context.wps_in_progress=0;
    return status;
}

/***********************************************************************************************
 * Function Name   : disconnect_from_network()
 * Returned Value  : NA
 * Comments        : Queries WPS status
 ***********************************************************************************************/
int32_t qc_api_wifi_disconnect_from_network()
{
    uint32_t deviceId = qc_api_wifi_get_active_device();
    if ( 0 == qc_drv_wlan_disconnect(qc_api_get_qc_drv_context(), deviceId))
        return QCLI_STATUS_SUCCESS_E;

    return QCLI_STATUS_ERROR_E;
}

/***********************************************************************************************
 * Function Name  : qc_api_wifi_set_passphrase
 * Returned Value : 0 on success, 1 on failure
 * Comments       : Store WPA Passphrase for later use
 ***********************************************************************************************/
int32_t qc_api_wifi_set_passphrase(char* passphrase)
{
    uint8_t deviceId = 0;
    uint32_t i = 0;
    deviceId = qc_api_wifi_get_active_device();

    if((strlen(passphrase) >= 8) && (strlen(passphrase) <=64))
    {
        if(strlen(passphrase) == 64)
        {
            for (i = 0; i < strlen(passphrase); i++)
            {
                if(ishexdigit(passphrase[i]))
                {
                    continue;
                }
                else
                {
                    LOG_WARN("passphrase in hex, please enter [0-9] or [A-F]\r\n\r\n");
                    return QCLI_STATUS_ERROR_E;
                }
            }
        }
        memset(wpa_passphrase[deviceId], 0, (__QAPI_WLAN_PASSPHRASE_LEN + 1));
        strcpy((char *)wpa_passphrase[deviceId], (const char *)passphrase);
        return QCLI_STATUS_SUCCESS_E;
    }
    else
    {
        LOG_WARN("Wrong passphrase length, the length should be between 8 and 64. \r\n\r\n");
        return QCLI_STATUS_ERROR_E;
    }

}

/***********************************************************************************************
 * Function Name  : get_rssi()
 * Returned Value : 0 on success, 1 on failure
 * Comments       : Get link quality indicator - RSSI
 ***********************************************************************************************/
int32_t qc_api_wifi_get_rssi()
{
    uint8_t rssi = 0;
    uint32_t dataLen = 0;
    if (0 == qc_drv_wlan_get_rssi(qc_api_get_qc_drv_context(), qc_api_wifi_get_active_device(),
                &rssi,
                &dataLen))
    {
        LOG_AT("\r\nindicator = %d dB\r\n",rssi);
        return QCLI_STATUS_SUCCESS_E;
    }
    return QCLI_STATUS_ERROR_E;
}

/***********************************************************************************************
 * Function Name  : set_sta_listen_interval()
 * Returned Value : 0 on success, 1 on failure
 * Comments       : Sets the station mode listen interval
 ***********************************************************************************************/
int32_t qc_api_wifi_set_sta_listen_interval(int32_t listen_time)
{
    if (0 == qc_drv_wlan_set_listen_interval(qc_api_get_qc_drv_context(),
                qc_api_wifi_get_active_device(),
                &listen_time,
                sizeof(listen_time),
                FALSE))
    {
        return QCLI_STATUS_SUCCESS_E;
    }
    return QCLI_STATUS_ERROR_E;
}

/***********************************************************************************************
 * Function Name  : set_sta_mac_keep_alive_time()
 * Returned Value : 0 on success, 1 on failure
 * Comments       : Sets the Station MAC keep alive timer
 ***********************************************************************************************/
int32_t qc_api_wifi_set_sta_mac_keep_alive_time(int32_t keep_alive_time_sec)
{
    if (keep_alive_time_sec ==0 )
    {
        LOG_INFO("keep_alive_timer will be disabled.\n");
    }
    if (0 == qc_drv_wlan_set_sta_keep_alive(qc_api_get_qc_drv_context(),
                qc_api_wifi_get_active_device(),
                &keep_alive_time_sec,
                sizeof(keep_alive_time_sec),
                FALSE))
    {
        return QCLI_STATUS_SUCCESS_E;
    }
    return QCLI_STATUS_ERROR_E;
}

/***********************************************************************************************
 * Function Name  : set_roam_thresh
 * Returned Value : -1 on error else 0
 * Comments       : Set lower and upper roam thresholds
 ***********************************************************************************************/
int32_t qc_api_wifi_set_roam_thresh(int32_t lower_thresh, int32_t upper_thresh, int32_t wt, int32_t pollTime)
{
    qapi_WLAN_Rssi_Threshold_Params_t qrthresh;
    /*These values are positive input values*/
    qrthresh.threshold_Above1_Val     = upper_thresh; //Lower positive value (high rssi)
    qrthresh.threshold_Below1_Val     = lower_thresh; //Higher positive value (low rssi)
    qrthresh.weight                   = wt;
    qrthresh.poll_Time                = pollTime;

    return qc_drv_wlan_set_rssi_threshold (qc_api_get_qc_drv_context(), qc_api_wifi_get_active_device(),
            &qrthresh,
            sizeof(qrthresh),
            FALSE);
}

/***********************************************************************************************
 * Function Name   : tcp_keepalive_offload_session_config()
 * Returned Value  : 0 if success else -1
 *
 * Comments        : This function configures the session info to offload the TCP Keepalive functionality to the target.
 *                   Sets static IPv4 parameters.
 *
 *       wmiconfig --tcpoffloadsession <tcp_src_port> <tcp_dst_port> <src_ip> <dst_ip> <dest_mac_addr> <seq_num> <ack_seq> <ip protocol> ; 4 - ipv4, 6 - ipv6
 *  e.g. wmiconfig --tcpoffloadsession 801 901 192.168.1.2 192.168.1.3 <dest mac addr> 1 1 4
 ***********************************************************************************************/
int32_t qc_api_wifi_tcp_keepalive_offload_session_config(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    uint32_t s_addr = 0, d_addr = 0;
    uint8_t  s_v6addr[16] = { '\0'}, d_v6addr[16] = { '\0'};
    const int8_t *dest_mac_addr = NULL;
    uint32_t seq_num = 0, ack_seq = 0;
    int32_t src_port = 0, dst_port = 0;
    qapi_WLAN_IP_Protocol_Type_e protocol;
    unsigned int sbits = 0;
    qapi_WLAN_TCP_Offload_Config_Params_t tcp_offload_sess_cfg;
    int32_t retval = -1, sockid = 0;


    if(Parameter_Count < 7 || Parameter_Count > 8)
    {
        return QCLI_STATUS_ERROR_E;
    }

    //sockid   = Parameter_List[0].Integer_Value;
    src_port = Parameter_List[0].Integer_Value;
    dst_port = Parameter_List[1].Integer_Value;
    seq_num  = qc_api_wifi_atoll(Parameter_List[5].String_Value);
    ack_seq  = qc_api_wifi_atoll(Parameter_List[6].String_Value);
    protocol = (qapi_WLAN_IP_Protocol_Type_e)Parameter_List[7].Integer_Value;

    dest_mac_addr   = (const int8_t *) Parameter_List[4].String_Value;

    if(protocol == QAPI_WLAN_IP_PROTOCOL_V4_E)
    {
        if( parse_ipv4_ad((unsigned long *)&s_addr, &sbits, (char *) Parameter_List[2].String_Value) )
        {
            LOG_WARN("Invalid Source address\r\n");
            return QCLI_STATUS_ERROR_E;
        }
        s_addr = BE2CPU32(s_addr);

        if( parse_ipv4_ad((unsigned long *)&d_addr, &sbits, (char *) Parameter_List[3].String_Value) )
        {
            LOG_WARN("Invalid Destination address\r\n");
            return QCLI_STATUS_ERROR_E;
        }
        d_addr = BE2CPU32(d_addr);

        tcp_offload_sess_cfg.src_IP           = s_addr;
        tcp_offload_sess_cfg.dst_IP           = d_addr;
    }
    else if(protocol == QAPI_WLAN_IP_PROTOCOL_V6_E)
    {
        retval = inet_pton(AF_INET6, (const char *) Parameter_List[2].String_Value, s_v6addr);
        if(retval == 1) {
            LOG_ERR("Invalid ipv6 prefix\r\n");
            return QCLI_STATUS_ERROR_E;
        }

        retval = inet_pton(AF_INET6, (const char *) Parameter_List[3].String_Value, d_v6addr);
        if(retval == 1) {
            LOG_ERR("Invalid ipv6 prefix\r\n");
            return QCLI_STATUS_ERROR_E;
        }

        memcpy(tcp_offload_sess_cfg.src_IP_v6addr, s_v6addr, __QAPI_WLAN_IPV6_ADDR_LEN);
        memcpy(tcp_offload_sess_cfg.dst_IP_v6addr, d_v6addr, __QAPI_WLAN_IPV6_ADDR_LEN);
    }

    tcp_offload_sess_cfg.sock_Id          = sockid;
    tcp_offload_sess_cfg.src_Port         = src_port;
    tcp_offload_sess_cfg.dst_Port         = dst_port;
    tcp_offload_sess_cfg.sequence_Num     = seq_num;
    tcp_offload_sess_cfg.ack_Sequence_Num = ack_seq;
    tcp_offload_sess_cfg.ip_Protocol_Type = protocol;

    if(qc_api_wifi_strtomac((const int8_t*)dest_mac_addr, tcp_offload_sess_cfg.dest_MAC)) {
        LOG_DEBUG("Destination MAC address translation failed.\r\n");
        return QCLI_STATUS_SUCCESS_E;
    }

    /* Set tcp keep alive offload session cfg */
    return qc_drv_wlan_set_ka_offload_scfg(qc_api_get_qc_drv_context(),
            qc_api_wifi_get_active_device(),
            &tcp_offload_sess_cfg,
            sizeof(tcp_offload_sess_cfg),
            FALSE);
}

/***********************************************************************************************
 * Function Name  : tcp_keepalive_offload_enable_disable
 * Returned Value : -1 on error else 0
 * Comments       : Enable/Disable, TCPKeepaliveInterval, keep_Alive_Ack_Recv_Threshold
 ***********************************************************************************************/
int32_t qc_api_wifi_tcp_keepalive_offload_enable_disable(uint8_t enable, uint16_t keepalive_intvl, uint16_t keepalive_ack_recv_threshold )
{
    qapi_WLAN_TCP_Offload_Enable_t tcp_offload_enable;
    uint32_t deviceId = qc_api_wifi_get_active_device();

    qc_api_wifi_set_callback(NULL);

    tcp_offload_enable.enable = enable;
    tcp_offload_enable.keepalive_Interval = keepalive_intvl;
    tcp_offload_enable.keepalive_Ack_Recv_Threshold = keepalive_ack_recv_threshold;

    return qc_drv_wlan_set_ka_offload_enable (qc_api_get_qc_drv_context(),
            deviceId,
            &tcp_offload_enable,
            sizeof(tcp_offload_enable),
            FALSE);
}

/*****************************************************
 *Function- qc_api_wifi_probe_req_handler- processes incoming probe requests.
 * buf- points to begining of MAC header in the probe request frame.
 * len- size of buffer
 * freq- channel on which Probe req is received

 * Structure of MAC Header-
 *   uint8_t fc[2];
 *   uint8_t dur[2]; //Duration in us
 *   uint8_t addr1[IEEE80211_ADDR_LEN]; //Source address
 *   uint8_t addr2[IEEE80211_ADDR_LEN]; //Destination address
 *   uint8_t addr3[IEEE80211_ADDR_LEN];
 *   uint8_t seq[2];  //Sequence number
 *   uint8_t qos[2];  //QoS field
 ******************************************************/
void qc_api_wifi_probe_req_handler(void* buf, int32_t len, int32_t freq)
{
    int32_t i = 0;
    uint32_t devId;

    devId = qc_api_wifi_get_active_device();
    LOG_AT_EVT("EVT_WIFI: %d RX: ProbeRequest from freq: %d MAC address: ", devId, freq);
    for(i=0; i< 6; i++){
        LOG_AT_EVT("%x ",*((uint8_t*)buf + QC_API_PROBE_REQ_SRC_ADDR_OFFSET + i));
    }
    LOG_AT_EVT("\r\n");
}

/***********************************************************************************************
 * Function Name  : wnm_event_cb
 * Returned Value : NA
 * Comments       : WLAN power mode call back function
 ***********************************************************************************************/
void qc_api_wifi_wnm_event_cb(uint8_t deviceId, void *payload, uint32_t payloadLength)
{
    if(!payload)
    {
        return;
    }
    qapi_WLAN_WNM_Cb_Info_t *wnmEvent = (qapi_WLAN_WNM_Cb_Info_t *)payload;

    switch(wnmEvent->cmd_Type)
    {
        case __QAPI_WLAN_PARAM_GROUP_WIRELESS_WNM_CONFIG:
            if (wnmEvent->response == 0) {
                LOG_AT_EVT("EVT_WIFI: %u 802.11v configuration succeeded\r\n", deviceId);
            }
            else {
                LOG_AT_EVT("EVT_WIFI: %u 802.11v configuration failed\r\n", deviceId);
            }
            break;

        case __QAPI_WLAN_PARAM_GROUP_WIRELESS_WNM_AP_BSS_MAX_IDLE_PERIOD:
            if (wnmEvent->response == 0) {
                LOG_AT_EVT("EVT_WIFI: %u 802.11v bss max idle period set\r\n", deviceId);
            }
            else {
                LOG_AT_EVT("EVT_WIFI: %u BSS max idle period not set\r\n",deviceId);
            }
            break;

        case __QAPI_WLAN_PARAM_GROUP_WIRELESS_WNM_STA_SLEEP_PERIOD:
            if (wnmEvent->response == 0) {
                LOG_AT_EVT("EVT_WIFI: %u 802.11v WNM sleep command succeeded\r\n", deviceId);
            }
            else {
                LOG_AT_EVT("EVT_WIFI: %u 802.11v WNM sleep command failed\r\n", deviceId);
            }
            break;

        case __QAPI_WLAN_PARAM_GROUP_WIRELESS_WNM_AP_SLEEP_RESPONSE:
            if (wnmEvent->response == 0) {
                LOG_AT_EVT("EVT_WIFI: %u 802.11v WNM sleep response set\r\n", deviceId);
            }
            else {
                LOG_AT_EVT("EVT_WIFI: %u 802.11v WNM sleep response command failed\r\n", deviceId);
            }
            break;
    }
}

/***********************************************************************************************
 * Function Name  : wps_profile_event_cb
 * Returned Value : NA
 * Comments       : WLAN callback for WPS profile event
 ***********************************************************************************************/
void qc_api_wifi_wps_profile_event_cb(uint8_t deviceId, void *payload, uint32_t payloadLength)
{
    if(!payload)
    {
        LOG_AT_EVT("EVT_WIFI: %d WPS failed\r\n", deviceId);
        return;
    }

    qapi_WLAN_WPS_Cb_Info_t *event = (qapi_WLAN_WPS_Cb_Info_t *)payload;

    if(event->status == QAPI_WLAN_WPS_STATUS_SUCCESS)
    {
        LOG_AT_EVT("EVT_WIFI: %d Waiting for WPS completion\r\n", deviceId);
    }
    else if(event->status == QAPI_WLAN_WPS_STATUS_FAILURE)
    {
        LOG_AT_EVT("EVT_WIFI: %d WPS failed\r\n", deviceId);
    }

    /*
     * WPS process is completed. Create a timer to retrieve WPS profile event.
     * In station mode, after receiving the WPS event, the application should send connect request to the AP, if connect flag is set by the user.
     * In soft-AP mode, it waits for the connect request from the station.
     */
    qurt_signal_set(&wlan_util_event, QC_API_WPS_EVENT_MASK);
    return;
}

/***********************************************************************************************
 * Function Name  : qc_api_wifi_wlan_callback_handler
 * Returned Value : NA
 * Comments       : WLAN functionality callbacks handler
 ***********************************************************************************************/
int32_t qc_api_wifi_wlan_callback_handler( uint8_t  deviceId,
        uint32_t cbId,
        void *pApplicationContext,
        void *payload,
        uint32_t payload_Length)
{
    static uint16_t total_scan_count = 0;

    switch(cbId)
    {
        case QAPI_WLAN_CONNECT_CB_E:
            {
                qapi_WLAN_Connect_Cb_Info_t *cxnInfo  = (qapi_WLAN_Connect_Cb_Info_t *)(payload);
                qc_api_wifi_connect_handler(cxnInfo->value, deviceId, cxnInfo->mac_Addr, cxnInfo->bss_Connection_Status);
                if(waiting_connection_completion)
                {
                    qurt_signal_set(&wlan_util_event, QC_API_CONNECT_EVENT_MASK);
                    waiting_connection_completion = 0;
                }
            }
            break;
        case QAPI_WLAN_FWD_PROBE_REQUEST_INFO_CB_E:
            {
                qapi_WLAN_Fwd_Probe_Request_Cb_Info_t *probeReqInfo = (qapi_WLAN_Fwd_Probe_Request_Cb_Info_t *) (payload);
                qc_api_wifi_probe_req_handler(probeReqInfo->probe_Req_Buffer, probeReqInfo->buffer_Length, probeReqInfo->frequency);
            }
            break;
        case QAPI_WLAN_PROMISCUOUS_MODE_CB_INFO_CB_E:
            {
                qc_api_wifi_promisc_frame_handler(payload, payload_Length);
            }
            break;
        case QAPI_WLAN_SCAN_COMPLETE_CB_E:
            {
                int32_t error = 0;
                uint16_t scan_mode = *((uint16_t*)(payload));

                if(scan_mode == QAPI_WLAN_NO_BUFFERING_E)
                {
                    //LOG_AT_EVT("EVT_WIFI: %d Scan result count:%d\r\n", total_scan_count, deviceId);
                    total_scan_count = 0;
                    return error;
                }
                scan_param.scan_List = malloc((sizeof(qapi_WLAN_BSS_Scan_Info_t) * __QAPI_MAX_SCAN_RESULT_ENTRY));
                scan_param.num_Scan_Entries = __QAPI_MAX_SCAN_RESULT_ENTRY;
                error = qc_drv_wlan_get_scan_results(qc_api_get_qc_drv_context(),
                        deviceId, (qapi_WLAN_BSS_Scan_Info_t *)(scan_param.scan_List), (int16_t *)&(scan_param.num_Scan_Entries));
                if (0 != error){
                    return error;
                }
                LOG_AT_EVT("EVT_WIFI: %d Scan Complete!\r\n", deviceId);
                QCLI_Display_Prompt();
                return error;
            }
        case QAPI_WLAN_BSS_INFO_CB_E:
            {
                int32_t error = 0;
                qapi_WLAN_Scan_List_t ScanInfo;
                uint16_t scan_count_received = (uint16_t)(payload_Length/sizeof(qapi_WLAN_BSS_Scan_Info_t));

                total_scan_count += scan_count_received;
                ScanInfo.scan_List = (qapi_WLAN_Scan_List_t *)(payload);
                error = qc_api_wifi_print_scan_results(ScanInfo, scan_count_received);
                return error;
            }
            break;
        case QAPI_WLAN_TCP_KEEPALIVE_OFFLOAD_CB_E:
            {
                qapi_WLAN_TCP_Keepalive_Event_t *event_ptr = (qapi_WLAN_TCP_Keepalive_Event_t*)payload;
                qapi_WLAN_TCP_Keepalive_Event_Info_t *event_info_ptr;
                uint8_t i = 0;

                event_info_ptr = &event_ptr->event_info[0];
                for(i=0; i<event_ptr->session_cnt; i++)
                {
                    LOG_AT_EVT("\r\nEVT_WIFI: %d Seq num = %u, Ack seq = %u, srcPort = %d dstPort = %d status = %d", deviceId, event_info_ptr->sequence_Num, event_info_ptr->ack_Sequence_Num, event_info_ptr->src_Port, event_info_ptr->dst_Port, event_info_ptr->status);
                    event_info_ptr++;
                }
            }
            break;
#if ENABLE_P2P_MODE
        case QAPI_WLAN_P2P_CB_E:
            {
                qc_api_wifi_app_wlan_p2p_event_cb(deviceId, payload, &payload_Length);
                break;
            }
#endif
        case QAPI_WLAN_WNM_CB_E:
            qc_api_wifi_wnm_event_cb(deviceId, payload, payload_Length);
            break;

        case QAPI_WLAN_WPS_CB_E:
            qc_api_wifi_wps_profile_event_cb(deviceId, payload, payload_Length);
            break;

        case QAPI_WLAN_PREFERRED_NETWORK_OFFLOAD_CB_E:
            {
                qapi_WLAN_Preferred_Network_Offload_Info_t *profile = NULL;
                profile = (qapi_WLAN_Preferred_Network_Offload_Info_t *) payload;
                if(profile->profile_Matched)
                {
                    LOG_AT_EVT("\r\nEVT_WIFI: %d Rx PNO Event: Profile matched, Profile Index: %u, RSSI: %d, Remaining Fast Scans: %u\n",
                            deviceId, profile->matched_Index, profile->rssi, profile->num_Fast_Scans_Remaining);
                }
                else
                {
                    LOG_AT_EVT("\r\nEVT_WIFI: %d Rx PNO Event: Profile not matched, Profile Index: %u, RSSI: %d, Remaining Fast Scans: %u\n",
                            deviceId, profile->matched_Index, profile->rssi, profile->num_Fast_Scans_Remaining);
                }
                break;
            }
        case QAPI_WLAN_ERROR_HANDLER_CB_E:
            {
                LOG_AT_EVT("EVT_WIFI: %d Fatal error occured. Error= ", deviceId);

                /* take necessary action based on error */
                QAPI_FATAL_ERR(0,0,0);
            }
        case QAPI_WLAN_RESUME_HANDLER_CB_E:
            {
                LOG_AT_EVT("\r\nEVT_WIFI: %d WLAN FIRMWARE RESUME Completed", deviceId);
            }

        default:
            break;
    }
    return QCLI_STATUS_SUCCESS_E;
}

/***********************************************************************************************
 * Function Name  : enable_probe_req_event
 * Returned Value : return -1 on error, 0 on success
 * Comments       : enable the probe request
 ***********************************************************************************************/
uint32_t qc_api_wifi_enable_probe_req_event(int32_t enable)
{
    int32_t deviceId = qc_api_wifi_get_active_device();
    return qc_drv_wlan_set_probe_req_fwd_to_host(qc_api_get_qc_drv_context(),
            deviceId,
            &enable,
            sizeof(uint32_t),
            FALSE);
}

/***********************************************************************************************
 * Function Name  : qc_api_wifi_freq_to_channel
 * Returned Value : returns channel number
 * Comments       : validates the frequency
 **********************************************************************************************/
uint16_t qc_api_wifi_freq_to_channel(uint16_t channel_val)
{
    if(channel_val < 3000)
    {
        channel_val -= __QAPI_WLAN_CHAN_FREQ_1;
        if((channel_val/5) == 14)
        {
            channel_val = 14;
        }
        else
        {
            channel_val = (channel_val/5) + 1;
        }
    }
    else
    {
        channel_val -= __QAPI_WLAN_CHAN_FREQ_36;
        channel_val = 36 + (channel_val/5); // since in 11a channel 36 is the starting number
    }
    return channel_val;
}

/***********************************************************************************************
 * Function Name  : qc_api_wifi_handle_connect_for_concurrent
 * Returned Value : 0 on success, 1 on error
 * Comments       : handles the connect in concurrent mode
 **********************************************************************************************/
#if QC_API_ENABLE_SCC_MODE
static int32_t qc_api_wifi_handle_connect_for_concurrent(uint32_t deviceId, int8_t* ssid)
{
    uint8_t scan_ssid[__QAPI_WLAN_MAX_SSID_LENGTH];

    memset(scan_ssid,0,__QAPI_WLAN_MAX_SSID_LENGTH);

    strcpy((char*)scan_ssid,ssid);
    strcpy((char*)ssid_str_concurrent, ssid);
    LOG_DEBUG("remote ap ssid: %s\r\n", ssid_str_concurrent);
    if (0 != qc_drv_wlan_set_ssid(qc_api_get_qc_drv_context(),
                deviceId,
                scan_ssid,
                strlen((const char *) scan_ssid),
                FALSE))
    {
        LOG_ERR("Unable to set SSID for scan\r\n");
        return QCLI_STATUS_ERROR_E;
    }

    ap_channel_hint = 0;
    concurrent_connect_flag = 0x0F;
    /*Do the actual scan*/
    qc_api_wifi_wmi_set_scan(deviceId, NULL, QAPI_WLAN_BUFFER_SCAN_RESULTS_BLOCKING_E);
    concurrent_connect_flag = 0x0E;
    if( ap_channel_hint == 0)
    {
        LOG_ERR("Unable to find AP: %s.....\r\n", ssid);
        return QCLI_STATUS_ERROR_E;
    }
    LOG_INFO("find AP...\r\n");
    A_MDELAY(50);

    if (0 != qc_drv_wlan_set_channel(qc_api_get_qc_drv_context(),
                deviceId,
                (void *) &ap_channel_hint,
                sizeof(ap_channel_hint),
                FALSE))
    {
        LOG_DEBUG("Unable to set AP channel:%d\r\n",ap_channel_hint);
        return QCLI_STATUS_ERROR_E;
    }
    A_MDELAY(50);
    return QCLI_STATUS_SUCCESS_E;
}

#endif

/**********************************************************************************************
 * Function Name   : qc_api_wifi_connect_to_network()
 * Returned Value  : 0 - successful completion or
 *                    -1 - failed.
 * Comments        : Handles Connect commands for infrastructure mode, Open
 *                   WEP,WPA/WPA2 security is supported
 **********************************************************************************************/
int32_t qc_api_wifi_connect_to_network(const int8_t* ssid)
{
    int32_t error = 0;
    int32_t ssidLength = 0;
    uint32_t deviceId = 0, temp_mode = 0, dataLen = 0;

    deviceId = qc_api_wifi_get_active_device();
    qc_api_wifi_set_callback(NULL);
    ssidLength = strlen((const char *)ssid);

#if QC_API_ENABLE_SCC_MODE
    if((concurrent_connect_flag == 0x0E) && (deviceId == 1))
    {
        error = qc_api_wifi_handle_connect_for_concurrent(deviceId, ssid);
        if(error != 0)
        {
            LOG_ERR("cannot connect for concurrent mode\r\n");
            return QCLI_STATUS_ERROR_E;
        }
    }
#endif

    if(wps_should_disable)
    {
        if (0 != qc_drv_wlan_set_ap_wps_flag(qc_api_get_qc_drv_context(),
                    deviceId,
                    &wps_flag,
                    sizeof(wps_flag),
                    FALSE))
        {
            LOG_WARN("Disable wps mode failed \r\n");
            return QCLI_STATUS_ERROR_E;
        }
    }

    qc_drv_wlan_get_op_mode(qc_api_get_qc_drv_context(),
            deviceId,
            &temp_mode,
            &dataLen);

    if (temp_mode == QAPI_WLAN_DEV_MODE_AP_E)
    {
        LOG_DEBUG("setting to ap mode \r\n");
    }
    if ((QC_API_SEC_MODE_OPEN == security_mode) && (temp_mode == QAPI_WLAN_DEV_MODE_AP_E))
    {
        LOG_DEBUG("AP in OPEN mode!\r\n");
    }

    if (ssidLength > __QAPI_WLAN_MAX_SSID_LENGTH || ssidLength < 0)
    {
        LOG_ERR("SSID length exceeds Maximum value\r\n");
        return QCLI_STATUS_ERROR_E;
    }
    error = qc_drv_wlan_set_ssid(qc_api_get_qc_drv_context(),
            deviceId,
            (uint8_t *)ssid,
            ssidLength,
            FALSE);
    if (error != 0)
    {
        LOG_ERR("Error during setting of ssid %s error=%08x!\r\n", ssid, error);
        return error;
    }

    LOG_INFO("Setting SSID to %s \r\n\r\n",ssid);
    strcpy((char *)original_ssid, (const char *)ssid);

    if (QC_API_SEC_MODE_WEP == security_mode) {
        cipher = QAPI_WLAN_CRYPT_WEP_CRYPT_E;
        if ( 0 != qc_drv_wlan_set_encr_type(qc_api_get_qc_drv_context(),
                    deviceId,
                    &cipher,
                    sizeof(qapi_WLAN_Crypt_Type_e),
                    FALSE))

        {
            return QCLI_STATUS_ERROR_E;
        }
        security_mode = QC_API_SEC_MODE_OPEN;
    }
    else if(QC_API_SEC_MODE_WPA == security_mode)
    {
        uint32_t passphraseLen = 0;

        if(0 != qc_drv_wlan_set_encr_type(qc_api_get_qc_drv_context(),
                    deviceId,
                    (void *) &cipher, //cipher is set in set_wpa
                    sizeof(qapi_WLAN_Crypt_Type_e),
                    FALSE))
        {
            return QCLI_STATUS_ERROR_E;
        }

        if( 0 != qc_drv_wlan_set_auth_mode(qc_api_get_qc_drv_context(),
                    deviceId,
                    (void *) &wpa_ver,
                    sizeof(qapi_WLAN_Auth_Mode_e),
                    FALSE))
        {
            return QCLI_STATUS_ERROR_E;
        }
        passphraseLen  = strlen((char *)wpa_passphrase[deviceId]);
        if((passphraseLen >= 8) && (passphraseLen <= 63))
        {
            if (0 != qc_drv_wlan_set_passphrase(qc_api_get_qc_drv_context(),
                        deviceId,
                        (void *) wpa_passphrase[deviceId],
                        passphraseLen,
                        FALSE))
            {
                LOG_ERR("Unable to set passphrase\r\n");
                return QCLI_STATUS_ERROR_E;
            }
            pmk_flag[deviceId] = 1;
        }
        else if(passphraseLen == 64)
        {
            if (0 != qc_drv_wlan_set_security_pmk(qc_api_get_qc_drv_context(),
                        deviceId,
                        (void *) wpa_passphrase[deviceId],
                        passphraseLen,
                        FALSE))
            {
                LOG_ERR("Unable to set pmk\r\n");
                return QCLI_STATUS_ERROR_E;
            }
            pmk_flag[deviceId] = 1;
        }
        else
        {
            LOG_ERR("invalid password\r\n");
            return QCLI_STATUS_ERROR_E;
        }
        security_mode = QC_API_SEC_MODE_OPEN;
    }

    error = qc_drv_wlan_commit(qc_api_get_qc_drv_context(), deviceId);
    if(error != 0)
    {
#if QC_API_ENABLE_AP_MODE
        if(temp_mode == MODE_AP_E)
        {
            qapi_WLAN_Auth_Mode_e authMode = QAPI_WLAN_AUTH_NONE_E;
            LOG_WARN("failed to AP mode \r\n");
#if QC_API_ENABLE_SCC_MODE
            LOG_WARN("support single concurrent channel only\r\n");
#endif
            // Clear Invalid configurations
            if ( 0 != qc_drv_wlan_set_auth_mode(qc_api_get_qc_drv_context(),
                        deviceId,
                        (void *) &authMode,
                        sizeof(qapi_WLAN_Auth_Mode_e),
                        FALSE))
            {
                LOG_ERR("Unable to clear Sec mode\r\n");
                return QCLI_STATUS_ERROR_E;
            }

            security_mode = QC_API_SEC_MODE_OPEN;
            pmk_flag[deviceId] = 0;
        }
#endif
        return QCLI_STATUS_ERROR_E;
    }
#if QC_API_ENABLE_AP_MODE
    if (temp_mode == MODE_AP_E)
    {
        /* setting the AP's default IP to 192.168.1.1 */
    }
#endif

    return QCLI_STATUS_SUCCESS_E;
}

/**********************************************************************************************
 * Function Name   : get_wlan_channel_list()
 * Returned Value  : 0 on success or -1 on failure
 * Comments        : Gets the WLAN channels list
 **********************************************************************************************/
uint32_t qc_api_wifi_get_wlan_channel_list()
{
    uint32_t deviceId = 0, length = 0;
    qapi_WLAN_Get_Channel_List_t *wlanChannelList;
    int32_t ret = -1;

    deviceId = qc_api_wifi_get_active_device();


    if ( NULL == (wlanChannelList = malloc(sizeof(qapi_WLAN_Get_Channel_List_t)) ) ){
        /* Failure to allocate memory will drop the event at caller */
        return QCLI_STATUS_ERROR_E;
    }
    length = sizeof(wlanChannelList->channel_List);


    ret = qc_drv_wlan_get_reg_domain_channel_list(qc_api_get_qc_drv_context(),
            deviceId,
            wlanChannelList,
            &length);

    if( ret == 0 )
    {
        qc_api_wifi_print_wlan_channel_list( wlanChannelList );
    }
    else
        LOG_ERR("Failed to Get Current Regulatory Channel List \r\n");

    free(wlanChannelList);

    return ret;

}

/**********************************************************************************************
 * Function Name   : get_wlan_stats()
 * Returned Value  : 0 on success or -1 on failure
 * Comments        : Get the WLAN statistics
 **********************************************************************************************/
uint32_t qc_api_wifi_get_wlan_stats( uint8_t flag )
{
    uint32_t deviceId = 0, length = 0;
    qapi_WLAN_Get_Statistics_t getStats;
    int32_t ret = -1;

    deviceId = qc_api_wifi_get_active_device();

    getStats.reset_Counters_Flag = flag;
    getStats.wlan_Stats_Data = (qapi_WLAN_Statistics_t*)malloc(sizeof(qapi_WLAN_Statistics_t));
    length = sizeof(getStats);

    ret = qc_drv_wlan_get_wireless_stats(qc_api_get_qc_drv_context(),
            deviceId,
            &getStats,
            &length);

    if( ret == 0 )
    {
        qc_api_wifi_print_wlan_stats(getStats.wlan_Stats_Data );
        free(getStats.wlan_Stats_Data);
    }

    return ret;
}

/**********************************************************************************************
 * Function Name   : pno_set_network_profile_handler()
 * Description: Parses PNO command
 *  command format:
 *  wmiconfig --pno <1/0> <max_profiles> <fast scan interval> <fast scan duration> <slow scan interval>
 *  Where 1 (enable) and 0 (disable) PNO
 *        and  <slow scan interval> are in milli seconds
 *
 * Returned Value  : 1 - successful completion or
 *                    0 - failed.
 * Comments          : Handles PNO related functionality
 **********************************************************************************************/
int32_t qc_api_wifi_pno_enable(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    qapi_WLAN_Preferred_Network_Offload_Config_t param;
    uint32_t enable = 0;
    memset(&param, 0, sizeof(qapi_WLAN_Preferred_Network_Offload_Config_t));

    if (!Parameter_List[0].Integer_Is_Valid ||
            ((Parameter_List[0].Integer_Value != 0) && (Parameter_List[0].Integer_Value != 1)))
    {
        return QCLI_STATUS_USAGE_E;
    }

    enable = Parameter_List[0].Integer_Value;
    if(!enable)
    {
        LOG_DEBUG("Disabling PNO\r\n");
        param.start_Network_List_Offload = FALSE;
    }
    else if (Parameter_Count < 5)
    {
        return QCLI_STATUS_USAGE_E;
    }

    if (enable)
    {
        param.start_Network_List_Offload = TRUE;
        param.max_Num_Preferred_Network_Profiles = Parameter_List[1].Integer_Value;
        param.fast_Scan_Interval_In_Ms = Parameter_List[2].Integer_Value;
        param.fast_Scan_Duration_In_Ms = Parameter_List[3].Integer_Value;
        param.slow_Scan_Interval_In_Ms = Parameter_List[4].Integer_Value;
        qc_api_wifi_set_callback(NULL);
    }

    if (0 != qc_drv_wlan_set_nw_offload_enable (qc_api_get_qc_drv_context(),
                qc_api_wifi_get_active_device(),
                &param,
                sizeof(qapi_WLAN_Preferred_Network_Offload_Config_t),
                FALSE))
    {
        LOG_ERR("ERROR: driver command failed\r\n");
        return QCLI_STATUS_ERROR_E;
    }
    return QCLI_STATUS_SUCCESS_E;
}

/**********************************************************************************************
 * Function Name   : pno_set_network_profile_handler()
 * Description: Parses PNO command
 *  command format:
 *  wmiconfig --pno_set_network_profile <index> <ssid> <mode>
 *  Where <mode> can be
 *         open
 *         wep
 *        wpa <wpa_ver> <ucipher>
 * Returned Value  : 1 - successful completion or
 *                    0 - failed.
 * Comments          : Handles PNO related functionality
 **********************************************************************************************/
 #if 0
int32_t qc_api_wifi_set_pno_network_profile(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    uint8_t networkIndex = 0;
    qapi_WLAN_Preferred_Network_Profile_t param;

    if (Parameter_Count < 3 || !Parameter_List[0].Integer_Is_Valid || Parameter_List[2].Integer_Is_Valid) {
        LOG_INFO("WLAN SetPnoNetworkProfile <index> <ssid> <mode=open|wep|wpa|wpa2> [wpa_cipher]\r\n");
        return QCLI_STATUS_USAGE_E;
    }

    memset(&param, 0, sizeof(qapi_WLAN_Preferred_Network_Profile_t));

    /* Parse network index value */
    networkIndex = Parameter_List[0].Integer_Value;
    if (networkIndex >= 0 && networkIndex < __QAPI_WLAN_PNO_MAX_NETWORK_PROFILES){
        param.index = networkIndex;
    }
    else {
        LOG_ERR("Invalid index value\r\n");
        return QCLI_STATUS_ERROR_E;
    }

    param.ssid_Len = strlen((const char *)Parameter_List[1].String_Value);
    if(param.ssid_Len > __QAPI_WLAN_MAX_SSID_LEN) {
        LOG_ERR("invalid ssid value\r ssid length is %d\n", param.ssid_Len);
        return QCLI_STATUS_ERROR_E;
    }
    strncpy((char *)&param.ssid[0], (char *)Parameter_List[1].String_Value, param.ssid_Len);

    if (QC_API_APP_STRCASECMP(Parameter_List[2].String_Value, "open") == 0) {
        param.auth_Mode = QAPI_WLAN_AUTH_NONE_E;
        param.encryption_Type = QAPI_WLAN_CRYPT_NONE_E;
    }
    else if (QC_API_APP_STRCASECMP(Parameter_List[2].String_Value, "wep") == 0) {
        param.auth_Mode =  QAPI_WLAN_AUTH_WEP_E;
        param.encryption_Type = QAPI_WLAN_CRYPT_WEP_CRYPT_E;
    }
    else if (QC_API_APP_STRCASECMP(Parameter_List[2].String_Value, "wpa") == 0) {
        /********** wpa version ****************/
        param.auth_Mode = QAPI_WLAN_AUTH_WPA_PSK_E;
        if (!QC_API_APP_STRCASECMP(Parameter_List[3].String_Value, "TKIP"))
            param.encryption_Type = QAPI_WLAN_CRYPT_TKIP_CRYPT_E;
        else if (!QC_API_APP_STRCASECMP(Parameter_List[3].String_Value, "CCMP"))
            param.encryption_Type =  QAPI_WLAN_CRYPT_AES_CRYPT_E;
        else
            LOG_ERR("invaid uchipher, should be TKIP or CCMP\r\n");
    }
    else if (QC_API_APP_STRCASECMP(Parameter_List[2].String_Value, "wpa2")==0) {
        param.auth_Mode = QAPI_WLAN_AUTH_WPA2_PSK_E;
        if (!QC_API_APP_STRCASECMP(Parameter_List[3].String_Value, "TKIP"))
            param.encryption_Type = QAPI_WLAN_CRYPT_TKIP_CRYPT_E;
        else if (!QC_API_APP_STRCASECMP(Parameter_List[3].String_Value, "CCMP"))
            param.encryption_Type =  QAPI_WLAN_CRYPT_AES_CRYPT_E;
        else
            LOG_ERR("invaid uchipher, should be TKIP or CCMP\r\n");
    }

    if (0 != qc_drv_wlan_set_nw_profile(qc_api_get_qc_drv_context(),
                qc_api_wifi_get_active_device(),
                &param,
                sizeof(qapi_WLAN_Preferred_Network_Profile_t),
                FALSE))
    {
        LOG_ERR("Command failed\r\n");
        return QCLI_STATUS_ERROR_E;
    }
    return QCLI_STATUS_SUCCESS_E;
}

/**********************************************************************************************
 * Function Name   : allow_aggr()
 * Returned Value  : 1 - successful completion or
 *                    0 - failed.
 * Comments        : allows the WLAN aggregation
 **********************************************************************************************/
int32_t qc_api_wifi_allow_aggr(int8_t *hexTxTidMask, int8_t *hexRxTidMask )
{
    qapi_WLAN_Aggregation_Params_t param;
    uint32_t deviceId = 0;
    deviceId = qc_api_wifi_get_active_device();
    param.tx_TID_Mask = qc_api_wifi_strtoul((const char *)hexTxTidMask, NULL, 16);
    param.rx_TID_Mask = qc_api_wifi_strtoul((const char *)hexRxTidMask, NULL, 16);
    if(param.tx_TID_Mask > 0xff || param.rx_TID_Mask > 0xff){
        QCLI_Printf("wmiconfig --allow_aggr <tx_tid_mask> <rx_tid_mask> Enables aggregation based on the provided bit mask where\r\n");
        QCLI_Printf("each bit represents a TID valid TID's are 0-7\r\n");
        return QCLI_STATUS_ERROR_E;
    }

    if (0 != qc_drv_wlan_set_aggr_tid(qc_api_get_qc_drv_context(),
                deviceId,
                &param,
                sizeof(qapi_WLAN_Aggregation_Params_t),
                FALSE))
    {
        QCLI_Printf("Failed to set Aggregation mask\r\n");
        return QCLI_STATUS_ERROR_E;
    }
    return QCLI_STATUS_SUCCESS_E;
}

#define AGGRX_CFG_INVAL 0xff

/**********************************************************************************************
 * Function Name   : aggrx_config()
 * Returned Value  : 0 - success
 * Comments        : set aggregation rx parameter.
 **********************************************************************************************/
int32_t qc_api_wifi_aggrx_config(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    qapi_WLAN_Rx_Aggrx_Params_t qapi_aggrx_param;
    uint32_t deviceId = qc_api_wifi_get_active_device();
    uint8_t aggrx_buffer_size = 0, param_index = 0;
    uint8_t param_index_max = Parameter_Count - 1;
    int32_t param_integer = 0;

    memset(&qapi_aggrx_param, AGGRX_CFG_INVAL, sizeof(qapi_WLAN_Rx_Aggrx_Params_t));
    if (param_index_max > 5) {
        param_index_max = 5;
    }
    while (param_index <= param_index_max) {
        if (!Parameter_List[param_index].Integer_Is_Valid) {
            return QCLI_STATUS_ERROR_E;
        }
        param_integer = Parameter_List[param_index].Integer_Value;
        switch (param_index) {
            case 0:
                aggrx_buffer_size = param_integer;
                if (!(((aggrx_buffer_size>0) && (aggrx_buffer_size < __QAPI_WLAN_AGGRX_BUFFER_SIZE_MAX))
                            || (aggrx_buffer_size==AGGRX_CFG_INVAL))) {
                    QCLI_Printf("Invalid buffer size: should be (0, %d)\n", __QAPI_WLAN_AGGRX_BUFFER_SIZE_MAX);
                    return QCLI_STATUS_ERROR_E;
                }
                qapi_aggrx_param.aggrx_Buffer_Size = aggrx_buffer_size;
                break;
            case 1:
                qapi_aggrx_param.aggrx_Reorder_Buffer_Timeout_In_Ms = param_integer;
                break;
            case 2:
                qapi_aggrx_param.aggrx_Session_Timeout_Val_In_Ms = param_integer;
                break;
            case 3:
                qapi_aggrx_param.aggrx_Reorder_Cfg = param_integer;
                break;
            case 4:
                qapi_aggrx_param.aggrx_Session_Timeout_Cfg = param_integer;
                break;
            case 5:
                qapi_aggrx_param.reserved0 = param_integer;
                break;
            default:
                break;
        }
        param_index++;
    }

    if (0 != qc_drv_wlan_set_aggrx_config (qc_api_get_qc_drv_context(),
                deviceId,
                &qapi_aggrx_param,
                sizeof(qapi_WLAN_Rx_Aggrx_Params_t),
                FALSE)) {
        return QCLI_STATUS_ERROR_E;
    }

    return QCLI_STATUS_SUCCESS_E;
}

/**********************************************************************************************
 * Function Name   : qc_api_wifi_set_promiscuous_filters()
 * Returned Value  : 0 - success
 * Comments        : Tests promiscuous mode. Channel has to be set first
 *                   or Device has to be connected to an AP to sniff on a
 *                   particular channel.
 **********************************************************************************************/
int32_t qc_api_wifi_set_promiscuous_filters(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    uint8_t i = 0, arg_cnt = 0, mask = 0, num_filters = 0;

    num_filters = Parameter_List[arg_cnt++].Integer_Value;
    if(!num_filters || num_filters > __QAPI_WLAN_PROMISC_MAX_FILTER_IDX ){
        QCLI_Printf("\nERROR: Invalid promisc_num_filters");
        return QCLI_STATUS_ERROR_E;
    }

    if( promisc_filter_config.num_filters + num_filters > __QAPI_WLAN_PROMISC_MAX_FILTER_IDX )
    {
        QCLI_Printf("\r\nExceeding Max filters for Configuration...!!!");
        QCLI_Printf("\r\nRemaining no of filters = %d", (__QAPI_WLAN_PROMISC_MAX_FILTER_IDX - promisc_filter_config.num_filters));
        return QCLI_STATUS_ERROR_E;
    }

    if( Parameter_Count< (num_filters * QC_API_ATH_PROMISC_ARG_SET_LENTGH)){
        QCLI_Printf("ERROR: Insufficient command arguments\n");
        return QCLI_STATUS_ERROR_E;
    }

    for(i = promisc_filter_config.num_filters; i < __QAPI_WLAN_PROMISC_MAX_FILTER_IDX; i++){
        if(Parameter_Count > arg_cnt){
            /* Reading fitler MASK, which enables src, dst, frametype and subtype combination */
            promisc_filter_config.filter_flags[i] = Parameter_List[arg_cnt++].Integer_Value;
            mask = promisc_filter_config.filter_flags[i];

            if(mask & 0x0F){
                if(mask & __QAPI_WLAN_PROM_FILTER_SOURCE){
                    if(qc_api_wifi_strtomac((const int8_t*)Parameter_List[arg_cnt++].String_Value,
                                &(promisc_filter_config.src_Mac[i][0]))) {
                        QCLI_Printf("ERROR: MAC address translation failed.\r\n");
                        return QCLI_STATUS_ERROR_E;
                    }
                }
                else
                    arg_cnt++;

                if(mask & __QAPI_WLAN_PROM_FILTER_DEST){
                    if(qc_api_wifi_strtomac((const int8_t*)Parameter_List[arg_cnt++].String_Value,
                                &(promisc_filter_config.dst_Mac[i][0]))) {
                        QCLI_Printf("ERROR: MAC address translation failed.\r\n");
                        return QCLI_STATUS_ERROR_E;
                    }
                }
                else
                    arg_cnt++;
                if(mask & __QAPI_WLAN_PROM_FILTER_FRAME_TYPE){
                    promisc_filter_config.promisc_frametype[i] = Parameter_List[arg_cnt++].Integer_Value;
                    if(mask & __QAPI_WLAN_PROM_FILTER_FRAME_SUB_TYPE){
                        promisc_filter_config.promisc_subtype[i] = Parameter_List[arg_cnt++].Integer_Value;
                    }
                    else
                        arg_cnt++;
                }
                else
                    arg_cnt += 2;  // To skip both type and Subtype
            }
            else{
                QCLI_Printf("ERROR: Invalid filter MASK.\r\n");
                return QCLI_STATUS_ERROR_E;
            }
        }
    }
    promisc_filter_config.num_filters += num_filters;

    return QCLI_STATUS_SUCCESS_E;
}

/**********************************************************************************************
 * Function Name   : qc_api_wifi_reset_promisc_filters()
 * Returned Value  : 0 - success
 * Comments        : reset the promiscuous mode filers
 **********************************************************************************************/
static int32_t qc_api_wifi_reset_promisc_filters(void)
{
    memset(&promisc_filter_config, 0, sizeof(promisc_filter_config));

    return QCLI_STATUS_SUCCESS_E;
}

/**********************************************************************************************
 * Function Name   : qc_api_wifi_enable_disable_promisc_mode()
 * Returned Value  : 0 - success or 1 -failure
 * Comments        : Enable/Disable the promiscuous mode
 **********************************************************************************************/
static int32_t qc_api_wifi_enable_disable_promisc_mode(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    qapi_WLAN_Promiscuous_Mode_Info_t prom_mode_details;
    uint32_t enet_device = qc_api_wifi_get_active_device();
    uint8_t i = 0;

    if( Parameter_Count == 0 )
    {
        QCLI_Printf("\n Incomplete promisc enable/disable command..!!");
        return QCLI_STATUS_ERROR_E;
    }

    /* Set to MAX PERF Mode */
    qc_api_wifi_set_power_mode( QAPI_WLAN_POWER_MODE_MAX_PERF_E, QAPI_WLAN_POWER_MODULE_PWR_MAX_E);

    prom_mode_details.enable = Parameter_List[0].Integer_Value;
    if( !prom_mode_details.enable )
        qc_api_wifi_reset_promisc_filters();	// Reset the filters configuration when promisc disabled
    else {
        prom_mode_details.promisc_num_filters = promisc_filter_config.num_filters;
        for(i=0; i<prom_mode_details.promisc_num_filters; i++) {
            memcpy(prom_mode_details.src_Mac[i], promisc_filter_config.src_Mac[i], __QAPI_WLAN_MAC_LEN);
            memcpy(prom_mode_details.dst_Mac[i], promisc_filter_config.dst_Mac[i], __QAPI_WLAN_MAC_LEN);
            prom_mode_details.filter_flags[i] = promisc_filter_config.filter_flags[i];
            prom_mode_details.promisc_frametype[i] = promisc_filter_config.promisc_frametype[i];
            prom_mode_details.promisc_subtype[i] = promisc_filter_config.promisc_subtype[i];
        }
    }

    qc_api_wifi_set_callback(NULL);

    if(0 != qc_drv_wlan_promiscuous_mode (qc_api_get_qc_drv_context(), enet_device,
                (void *)&prom_mode_details,
                sizeof(prom_mode_details),
                FALSE))
    {
        return QCLI_STATUS_ERROR_E;
    }
    return QCLI_STATUS_SUCCESS_E;
}

/**********************************************************************************************
 * Function Name   : promiscuous_mode_handler()
 * Returned Value  : 0 - success or 1 -failure
 * Comments        : handler for the promiscuous mode
 **********************************************************************************************/
int32_t qc_api_wifi_promiscuous_mode_handler(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    uint8_t index=0;

    if( Parameter_Count < 1 ) {
        QCLI_Printf("\n Incomplete command to process promisc mode..!!");
        return QCLI_STATUS_ERROR_E;
    }
    // Promisc mode filters config/reset
    if (QC_API_APP_STRCASECMP(Parameter_List[index].String_Value, "filter") == 0) {
        if (QC_API_APP_STRCASECMP(Parameter_List[index+1].String_Value, "config") == 0) {
            return qc_api_wifi_set_promiscuous_filters( Parameter_Count-2, &Parameter_List[index+2]);
        }
        else if (QC_API_APP_STRCASECMP(Parameter_List[index+1].String_Value, "reset") == 0) {
            return qc_api_wifi_reset_promisc_filters();
        }
    }  // Promisc mode enable/disable
    else if (QC_API_APP_STRCASECMP(Parameter_List[index].String_Value, "enable") == 0){
        return qc_api_wifi_enable_disable_promisc_mode(Parameter_Count - 1, &Parameter_List[index+1]);
    }

    return QCLI_STATUS_SUCCESS_E;
}

/**********************************************************************************************
 * Function Name   : pktlog_handler()
 * Returned Value  : 0 - success or 1 -failure
 * Comments        : WLAN packet log handler
 **********************************************************************************************/
int32_t qc_api_wifi_pktlog_handler(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    uint32_t deviceId = qc_api_wifi_get_active_device();

    if (QC_API_APP_STRCASECMP(Parameter_List[0].String_Value, "enable") == 0)
    {
        qapi_WLAN_Pktlog_Enable_t PktlogEn;

        if( Parameter_Count != 3)
        {
            QCLI_Printf("Incomplete parameters for pktlog enable command\r\n");
            return QCLI_STATUS_ERROR_E;
        }
        PktlogEn.enable = Parameter_List[1].Integer_Value;
        if( PktlogEn.enable )
            PktlogEn.num_Of_Buffers = Parameter_List[2].Integer_Value;

        if (0 != qc_drv_wlan_set_pktlog_enable(qc_api_get_qc_drv_context(),
                    deviceId,
                    &PktlogEn,
                    sizeof(qapi_WLAN_Dbglog_Enable_t),
                    FALSE))
        {
            QCLI_Printf("\n Pktlog enable failed..\n");
            return QCLI_STATUS_ERROR_E;
        }
    }
    else if (QC_API_APP_STRCASECMP(Parameter_List[0].String_Value, "start") == 0)
    {
        qapi_WLAN_Pktlog_Start_Params_t PktlogStartCfg;

        if( Parameter_Count != 6 )
        {
            QCLI_Printf("Incomplete parameters for pktlog start command\r\n");
            return QCLI_STATUS_ERROR_E;
        }

        PktlogStartCfg.event_List = Parameter_List[1].Integer_Value;
        PktlogStartCfg.log_Options= Parameter_List[2].Integer_Value;
        PktlogStartCfg.trigger_Threshold = Parameter_List[3].Integer_Value;
        PktlogStartCfg.trigger_Interval = Parameter_List[4].Integer_Value;
        PktlogStartCfg.trigger_Tail_Count = Parameter_List[5].Integer_Value;

        if (0 != qc_drv_wlan_set_pktlog_start (qc_api_get_qc_drv_context(),
                    deviceId,
                    &PktlogStartCfg,
                    sizeof(qapi_WLAN_Pktlog_Start_Params_t),
                    FALSE))
        {
            QCLI_Printf("\n Pktlog Start failed..\n");
            return QCLI_STATUS_ERROR_E;
        }
    }
    return QCLI_STATUS_SUCCESS_E;
}

/**********************************************************************************************
 * Function Name   : ddglog_handler()
 * Returned Value  : 0 - success or 1 -failure
 * Comments        : Debug logs handler
 **********************************************************************************************/
int32_t qc_api_wifi_dbglog_handler(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    uint32_t deviceId = qc_api_wifi_get_active_device();

    if (QC_API_APP_STRCASECMP(Parameter_List[0].String_Value, "enable") == 0)
    {
        qapi_WLAN_Dbglog_Enable_t DbglogEn;
        if( Parameter_Count != 2 )
        {
            QCLI_Printf("Incomplete parameters for dbglog enable command\r\n");
            return QCLI_STATUS_ERROR_E;
        }

        DbglogEn.enable = Parameter_List[1].Integer_Value;
        if (0 != qc_drv_wlan_set_dblog_enable (qc_api_get_qc_drv_context(),
                    deviceId,
                    &DbglogEn,
                    sizeof(qapi_WLAN_Dbglog_Enable_t),
                    FALSE))
        {
            QCLI_Printf("\n Dbglog enable failed..\n");
            return QCLI_STATUS_ERROR_E;
        }
    }
    else if (QC_API_APP_STRCASECMP(Parameter_List[0].String_Value, "config") == 0)	  // Handles debug configuration
    {
        qapi_WLAN_Dbglog_Config_t DbglogCfg;
        if( Parameter_Count != 4 )
        {
            QCLI_Printf("Incomplete parameters for dbglog configuration command\r\n");
            return QCLI_STATUS_ERROR_E;
        }
        DbglogCfg.debug_Port = Parameter_List[1].Integer_Value;
        DbglogCfg.reporting_Enable = Parameter_List[2].Integer_Value;
        DbglogCfg.report_Trigger_Size_In_Bytes = Parameter_List[3].Integer_Value;
        if (0 != qc_drv_wlan_set_dblog_config (qc_api_get_qc_drv_context(),
                    deviceId,
                    &DbglogCfg,
                    sizeof(qapi_WLAN_Dbglog_Config_t),
                    FALSE))
        {
            QCLI_Printf("\n Dbglog configuration failed..\n");
            return QCLI_STATUS_ERROR_E;
        }
    }
    else if (QC_API_APP_STRCASECMP(Parameter_List[0].String_Value, "loglevel") == 0)
    {
        qapi_WLAN_Dbglog_Module_Config_t DbglogModuleCfg;
        uint8_t k, mod_id, loglevel, global_log_level_val;
        uint8_t LoglevelIndex, LoglevelBitpos;

        if( Parameter_Count < 3 )
        {
            QCLI_Printf("Incomplete parameters for dbglog Module Configuration command\r\n");
            return QCLI_STATUS_ERROR_E;
        }
        memset(&DbglogModuleCfg, 0, sizeof(DbglogModuleCfg));
        for(k=0; k < (Parameter_Count - 1); k +=2 ){
            mod_id = Parameter_List[k+1].Integer_Value;
            loglevel = ( Parameter_List[k+2].Integer_Value & 0x0F );
            if(mod_id == __QAPI_WLAN_DBGLOG_GLOBAL_MODULE_ID ){
                DbglogModuleCfg.module_Id_Mask = __QAPI_WLAN_DBGLOG_DEFAULT_MODULE_MASK;
                global_log_level_val = ( (loglevel << 4 ) | loglevel );
                memset(DbglogModuleCfg.log_Level, global_log_level_val, sizeof(DbglogModuleCfg.log_Level) );
                break;
            }
            if( mod_id > __QAPI_WLAN_DBGLOG_MAX_MODULE_ID )
            {
                QCLI_Printf("Invalid Module ID..SKipping it..!!\r\n");
                continue;	// Moving to next module
            }
            DbglogModuleCfg.module_Id_Mask |= ( (uint64_t)1 << mod_id );
            LoglevelIndex = mod_id / __QAPI_WLAN_DBGLOG_NIBBLE_CNT_IN_WORD_MEMORY;
            LoglevelBitpos = ( mod_id*sizeof(uint32_t) ) % __QAPI_WLAN_DBGLOG_BIT_CNT_IN_WORD_MEMORY;
            DbglogModuleCfg.log_Level[LoglevelIndex] |= ( loglevel << LoglevelBitpos );
        }
        if (0 != qc_drv_wlan_set_dblog_module_conf (qc_api_get_qc_drv_context(),
                    deviceId,
                    &DbglogModuleCfg,
                    sizeof(qapi_WLAN_Dbglog_Module_Config_t),
                    FALSE))
        {
            QCLI_Printf("\n Dbglog configuration failed..\n");
            return QCLI_STATUS_ERROR_E;
        }
    }
    return QCLI_STATUS_SUCCESS_E;
}

/**********************************************************************************************
 * Function Name   : regquery_handler()
 * Returned Value  : 0 - success or 1 -failure
 * Comments        : Register the query handler for WLAN debug
 **********************************************************************************************/
int32_t qc_api_wifi_regquery_handler(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
#if defined(WLAN_DEBUG)

    uint32_t deviceId = qc_api_wifi_get_active_device();
    qapi_WLAN_Driver_RegQuery_Params_t Regquery;
    if (QC_API_APP_STRCASECMP(Parameter_List[0].String_Value, "read") == 0)
    {
        if( Parameter_Count != 2 )
        {
            QCLI_Printf("Incomplete parameters for Driver reg read command\r\n");
            return QCLI_STATUS_ERROR_E;
        }
        Regquery.address = qc_api_wifi_strtoul(Parameter_List[1].String_Value, NULL, 16);
        Regquery.operation = QAPI_WLAN_REG_OP_READ;
        if (0 != qc_drv_wlan_set_drv_reg_query (qc_api_get_qc_drv_context(),
                    deviceId,
                    &Regquery,
                    sizeof(qapi_WLAN_Driver_RegQuery_Params_t),
                    FALSE))
        {
            QCLI_Printf("\n Driver reg read failed..\n");
            return QCLI_STATUS_ERROR_E;
        }
        QCLI_Printf("\n0x%08X \n",Regquery.value);
    }
    else if (QC_API_APP_STRCASECMP(Parameter_List[0].String_Value, "dump") == 0)
    {
        if( Parameter_Count != 3 )
        {
            QCLI_Printf("Incomplete parameters for Driver reg dump command\r\n");
            return QCLI_STATUS_ERROR_E;
        }
        if(Parameter_List[1].Integer_Value%4)
        {
            QCLI_Printf("Error size for Driver reg dump command\r\n");
            return QCLI_STATUS_ERROR_E;
        }
        Regquery.address = qc_api_wifi_strtoul(Parameter_List[1].String_Value, NULL, 16);
        Regquery.size = Parameter_List[2].Integer_Value;
        Regquery.operation = QAPI_WLAN_REG_OP_READ;
        uint32_t len = 0;

        QCLI_Printf("\r\n");

        for(len = 0; len < Regquery.size/4; len++){

            if (0 != qc_drv_wlan_set_drv_reg_query (qc_api_get_qc_drv_context(),
                        deviceId,
                        &Regquery,
                        sizeof(qapi_WLAN_Driver_RegQuery_Params_t),
                        FALSE))
            {
                QCLI_Printf("\n Driver reg dump failed..\n");
                return QCLI_STATUS_ERROR_E;
            }
            if(0==len%4)
                QCLI_Printf("0x%08X: ",Regquery.address);

            QCLI_Printf("0x%08X ",Regquery.value);

            if(3==len%4)
                QCLI_Printf("\r\n");
            Regquery.address += 4;
        }
    }
    else if (QC_API_APP_STRCASECMP(Parameter_List[0].String_Value, "write") == 0)
    {
        if( Parameter_Count != 3 )
        {
            QCLI_Printf("Incomplete parameters for Driver reg write command\r\n");
            return QCLI_STATUS_ERROR_E;
        }
        Regquery.address = qc_api_wifi_strtoul(Parameter_List[1].String_Value, NULL, 16);
        Regquery.value = qc_api_wifi_strtoul(Parameter_List[2].String_Value, NULL, 16);
        Regquery.operation = QAPI_WLAN_REG_OP_WRITE;
        if (0 != qc_drv_wlan_set_drv_reg_query(qc_api_get_qc_drv_context(),
                    deviceId,
                    &Regquery,
                    sizeof(qapi_WLAN_Driver_RegQuery_Params_t),
                    FALSE))
        {
            QCLI_Printf("\n Driver reg write failed..\n");
            return QCLI_STATUS_ERROR_E;
        }
    }
    else if (QC_API_APP_STRCASECMP(Parameter_List[0].String_Value, "rmw") == 0)
    {
        if( Parameter_Count != 4 )
        {
            QCLI_Printf("Incomplete parameters for Driver reg rmw command\r\n");
            return QCLI_STATUS_ERROR_E;
        }
        Regquery.address = qc_api_wifi_strtoul(Parameter_List[1].String_Value, NULL, 16);
        Regquery.value = qc_api_wifi_strtoul(Parameter_List[2].String_Value, NULL, 16);
        Regquery.mask = qc_api_wifi_strtoul(Parameter_List[3].String_Value, NULL, 16);
        Regquery.operation = QAPI_WLAN_REG_OP_RMW;
        if (0 != qc_drv_wlan_set_drv_reg_query (qc_api_get_qc_drv_context(),
                    deviceId,
                    &Regquery,
                    sizeof(qapi_WLAN_Driver_RegQuery_Params_t),
                    FALSE))
        {
            QCLI_Printf("\n Driver reg read mod write failed..\n");
            return QCLI_STATUS_ERROR_E;
        }
    }
#endif/* WLAN_DEBUG */
    return QCLI_STATUS_SUCCESS_E;
}

/**********************************************************************************************
 * Function Name   : memquery_handler()
 * Returned Value  : 0 - success or 1 -failure
 * Comments        : Memory query handler for WLAN debugs
 **********************************************************************************************/
int32_t qc_api_wifi_memquery_handler(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List){
#if defined(WLAN_DEBUG)
    uint32_t address = 0, value = 0, size = 0, mask = 0;

    if (QC_API_APP_STRCASECMP(Parameter_List[0].String_Value, "read") == 0)
    {
        if( Parameter_Count != 2 )
        {
            QCLI_Printf("Incomplete parameters for Quartz mem read command\r\n");
            return QCLI_STATUS_ERROR_E;
        }
        address = qc_api_wifi_strtoul(Parameter_List[1].String_Value, NULL, 16);
        value = *(uint32_t *)address;
        QCLI_Printf("\n0x%08X \n",value);
    }
    else if (QC_API_APP_STRCASECMP(Parameter_List[0].String_Value, "dump") == 0)
    {
        if( Parameter_Count != 3 )
        {
            QCLI_Printf("Incomplete parameters for Quartz mem dump command\r\n");
            return QCLI_STATUS_ERROR_E;
        }
        if(Parameter_List[1].Integer_Value%4)
        {
            QCLI_Printf("Error size for Quartz mem dump command\r\n");
            return QCLI_STATUS_ERROR_E;
        }
        address = qc_api_wifi_strtoul(Parameter_List[1].String_Value, NULL, 16);
        size = Parameter_List[2].Integer_Value;
        uint32_t len = 0;

        QCLI_Printf("\r\n");

        for(len = 0; len < size/4; len++){
            if(0==len%4)
                QCLI_Printf("0x%08X: ",address);

            value = *(uint32_t *)address;
            QCLI_Printf("0x%08X ",value);

            if(3==len%4)
                QCLI_Printf("\r\n");
            address += 4;
        }
    }
    else if (QC_API_APP_STRCASECMP(Parameter_List[0].String_Value, "write") == 0)
    {
        if( Parameter_Count != 3 )
        {
            QCLI_Printf("Incomplete parameters for Quartz mem write command\r\n");
            return QCLI_STATUS_ERROR_E;
        }
        address = qc_api_wifi_strtoul(Parameter_List[1].String_Value, NULL, 16);
        value = qc_api_wifi_strtoul(Parameter_List[2].String_Value, NULL, 16);
        *(uint32_t *)address = value;
    }
    else if (QC_API_APP_STRCASECMP(Parameter_List[0].String_Value, "rmw") == 0)
    {
        if( Parameter_Count != 4 )
        {
            QCLI_Printf("Incomplete parameters for Quartz mem rmw command\r\n");
            return QCLI_STATUS_ERROR_E;
        }
        uint32_t temp_value = 0;
        address = qc_api_wifi_strtoul(Parameter_List[1].String_Value, NULL, 16);
        value = qc_api_wifi_strtoul(Parameter_List[2].String_Value, NULL, 16);
        mask = qc_api_wifi_strtoul(Parameter_List[3].String_Value, NULL, 16);
        temp_value = *(uint32_t *)address;
        temp_value &= ~mask;
        temp_value |= value;
        *(uint32_t *)address = temp_value;
    }
#endif/* WLAN_DEBUG */
    return QCLI_STATUS_SUCCESS_E;
}

/**********************************************************************************************
 * Function Name   : driver_assert_handler()
 * Returned Value  : 0 - success or 1 -failure
 * Comments        : driver assert handler
 **********************************************************************************************/
int32_t qc_api_wifi_driver_assert_handler(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List){
#if defined(WLAN_DEBUG)

    uint32_t deviceId = qc_api_wifi_get_active_device();

    if( 0 != qc_drv_wlan_force_assert (qc_api_get_qc_drv_context(),
                deviceId,
                NULL,
                0,
                FALSE))
    {
        QCLI_Printf("ERROR: force set target assert failed\n");
        return QCLI_STATUS_ERROR_E;
    }
#endif/* WLAN_DEBUG */
    return QCLI_STATUS_SUCCESS_E;
}
#endif
/**********************************************************************************************
 * Function Name   : qc_api_wifi_chan_to_frequency()
 * Returned Value  : 0 : success or -1 : failure
 * Comments        : validates the channel number
 **********************************************************************************************/
uint32_t qc_api_wifi_chan_to_frequency(uint32_t channel)
{
    if (channel < 1 || channel > 165)
    {
        return (-QCLI_STATUS_ERROR_E);
    }
    if (channel < 27) {
        channel = __QAPI_WLAN_CHAN_FREQ_1 + (channel-1)*5;
    } else {
        channel = (5000 + (channel*5));
    }

    return channel;
}

/**********************************************************************************************
 * Function Name   : channel_switch()
 * Returned Value  : 0 - success or 1 -failure
 * Comments        : switch the channel
 **********************************************************************************************/
int32_t qc_api_wifi_channel_switch(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List){

    qapi_WLAN_Channel_Switch_t channel_switch;
    uint32_t channel = 0;
    uint32_t deviceId = qc_api_wifi_get_active_device();

    channel = qc_api_wifi_chan_to_frequency(Parameter_List[0].Integer_Value);
    if(channel == -1)
        return QCLI_STATUS_ERROR_E;

    channel_switch.channel = channel;
    channel_switch.tbtt_Count = Parameter_List[1].Integer_Value;
    if( 0 != qc_drv_wlan_set_ap_channel_switch (qc_api_get_qc_drv_context(),
                deviceId,
                &channel_switch,
                sizeof(channel_switch),
                FALSE))
    {
        LOG_ERR("CHANNEL_SWITCH filed\r\n");
        return QCLI_STATUS_ERROR_E;
    }
    return QCLI_STATUS_SUCCESS_E;
}

/**********************************************************************************************
 * Function Name   : send_raw_frame()
 * Returned Value  : 0 : success or -1 : failure
 * Comments        : test_raw -- makes use of special API's to send a raw MAC frame irregardless of connection state
 **********************************************************************************************/
 #if 0
int32_t qc_api_wifi_send_raw_frame(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    uint8_t rate_index = 0, tries = 0, header_type = 0, deviceId = 0;
    uint32_t i = 0, chan = 0, size = 0;
    int32_t status = -1;
    uint8_t addr[4][6];
    qapi_WLAN_Raw_Send_Params_t rawSendParams;

    /* collect user inputs if any */

    if((Parameter_Count < 5) || (Parameter_Count > 9)){
        goto raw_usage;
    }

    rate_index = Parameter_List[0].Integer_Value;
    tries = Parameter_List[1].Integer_Value;
    size = Parameter_List[2].Integer_Value;
    chan = Parameter_List[3].Integer_Value;
    header_type = Parameter_List[4].Integer_Value;
    memset (addr, 0, sizeof(addr));

    for(i = 0; i < (Parameter_Count-5);i++) {
        if(qc_api_wifi_strtomac((const int8_t *)Parameter_List[5+i].String_Value, &(addr[i][0])))
        {
            QCLI_Printf("ERROR: MAC address translation failed.\r\n");
            return status;
        }
    }

    if ( chan > 13 ) {
        goto raw_usage;
    }


    if( Parameter_Count == 5 )
    {
        addr[0][0] = 0xff;
        addr[0][1] = 0xff;
        addr[0][2] = 0xff;
        addr[0][3] = 0xff;
        addr[0][4] = 0xff;
        addr[0][5] = 0xff;
        addr[1][0] = 0x00;
        addr[1][1] = 0x03;
        addr[1][2] = 0x7f;
        addr[1][3] = 0xdd;
        addr[1][4] = 0xdd;
        addr[1][5] = 0xdd;
        addr[2][0] = 0x00;
        addr[2][1] = 0x03;
        addr[2][2] = 0x7f;
        addr[2][3] = 0xdd;
        addr[2][4] = 0xdd;
        addr[2][5] = 0xdd;
        addr[3][0] = 0x00;
        addr[3][1] = 0x03;
        addr[3][2] = 0x7f;
        addr[3][3] = 0xee;
        addr[3][4] = 0xee;
        addr[3][5] = 0xee;
        if(header_type == 1) {
            memcpy(&addr[0][0], &addr[1][0], __QAPI_WLAN_MAC_LEN);
            //change destination address
            addr[2][3] = 0xaa;
            addr[2][4] = 0xaa;
            addr[2][5] = 0xaa;
        }
    }

    rawSendParams.rate_Index = rate_index;
    rawSendParams.num_Tries = tries;
    rawSendParams.payload_Size = size;
    rawSendParams.channel = chan;
    rawSendParams.header_Type = header_type;
    rawSendParams.seq = 0;
    memcpy(&rawSendParams.addr1[0], addr[0], __QAPI_WLAN_MAC_LEN);
    memcpy(&rawSendParams.addr2[0], addr[1], __QAPI_WLAN_MAC_LEN);
    memcpy(&rawSendParams.addr3[0], addr[2], __QAPI_WLAN_MAC_LEN);
    memcpy(&rawSendParams.addr4[0], addr[3], __QAPI_WLAN_MAC_LEN);
    rawSendParams.data = NULL;
    rawSendParams.data_Length = 0;

    status = qc_drv_wlan_raw_send(qc_api_get_qc_drv_context(),
            deviceId, &rawSendParams);
    if( status != 0)
    {
raw_usage:
        QCLI_Printf("raw input error\r\n");
        QCLI_Printf("usage = WLAN SendRawFrame rate num_tries num_bytes channel header_type [addr1 [addr2 [addr3 [addr4]]]]\r\n");
        QCLI_Printf("rate = rate index where 0==1mbps; 1==2mbps; 2==5.5mbps etc\r\n");
        QCLI_Printf("num_tries = number of transmits 1 - 14\r\n");
        QCLI_Printf("num_bytes = payload size 0 to 1400\r\n");
        QCLI_Printf("channel = 0 - 11, 0: send on current channel\r\n");
        QCLI_Printf("header_type = 0==beacon frame; 1==QOS data frame; 2==4 address data frame\r\n");
        QCLI_Printf("addr1 = mac address xx:xx:xx:xx:xx:xx\r\n");
        QCLI_Printf("addr2 = mac address xx:xx:xx:xx:xx:xx\r\n");
        QCLI_Printf("addr3 = mac address xx:xx:xx:xx:xx:xx\r\n");
        QCLI_Printf("addr4 = mac address xx:xx:xx:xx:xx:xx\r\n");
    }

    return status;
}

/**********************************************************************************************
 * Function Name   : set_event_filter()
 * Returned Value  : 0 : success or 1 : failure
 * Comments        : Set the WLAN event after filtering
 **********************************************************************************************/
int32_t qc_api_wifi_set_event_filter(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    uint32_t device_id = qc_api_wifi_get_active_device();
    qapi_WLAN_Event_Filter_t evt_filter;

    if (Parameter_Count <= 0)
        return QCLI_STATUS_ERROR_E;

    if (Parameter_Count == 1 && Parameter_List[0].Integer_Value != 0) {
        QCLI_Printf("Invalid options\n");
        return QCLI_STATUS_ERROR_E;
    }

    if (Parameter_List[0].Integer_Value == 1 && Parameter_Count > (__QAPI_WLAN_MAX_NUM_FILTERED_EVENTS+1))
    {
        QCLI_Printf("Exceeded max number of supported event filters (%d)\n", __QAPI_WLAN_MAX_NUM_FILTERED_EVENTS);
        return QCLI_STATUS_ERROR_E;
    }

    if (Parameter_Count == 1 && (Parameter_List[0].Integer_Value == 0))
    {
        evt_filter.action     = QAPI_WLAN_DISABLE_E; //disable event filtering
        evt_filter.num_Events = 0;
    }

    if (Parameter_List[0].Integer_Value == 1)
    {
        int32_t num_events_in_arg_list = Parameter_Count-1;
        int32_t i = 0, arge = 1;

        if (num_events_in_arg_list == 0)
            return QCLI_STATUS_ERROR_E;

        evt_filter.action     = QAPI_WLAN_ENABLE_E; //enable event filtering
        evt_filter.num_Events = num_events_in_arg_list;

        while (num_events_in_arg_list--)
        {

            switch((qapi_WLAN_Filterable_Event_e) Parameter_List[arge].Integer_Value)
            {
                case QAPI_WLAN_EXTENSION_EVENTID_E: /* applicable for device-0 only */
                case QAPI_WLAN_PEER_NODE_EVENTID_E:  /* applicable for device-0 only */
                case QAPI_WLAN_P2P_INVITE_REQ_EVENTID_E:  /* applicable for device-0 only */
                case QAPI_WLAN_P2P_INVITE_RCVD_RESULT_EVENTID_E: /* applicable for device-0 only */
                case QAPI_WLAN_P2P_INVITE_SENT_RESULT_EVENTID_E: /* applicable for device-0 only */
                case QAPI_WLAN_P2P_PROV_DISC_RESP_EVENTID_E:  /* applicable for device-0 only */
                case QAPI_WLAN_P2P_PROV_DISC_REQ_EVENTID_E:  /* applicable for device-0 only */
                case QAPI_WLAN_P2P_START_SDPD_EVENTID_E:    /* applicable for device-0 only */
                case QAPI_WLAN_P2P_SDPD_RX_EVENTID_E:     /* applicable for device-0 only */
                case QAPI_WLAN_DBGLOG_EVENTID_E:         /* applicable for device-0 only */
                case QAPI_WLAN_PKTLOG_EVENTID_E:         /* applicable for device-0 only */
                case QAPI_WLAN_P2P_REQ_TO_AUTH_EVENTID_E:   /* applicable for device-0 only */
                case QAPI_WLAN_DIAGNOSTIC_EVENTID_E:       /* applicable for device-0 only */
                    if (device_id!=0)
                    {
                        QCLI_Printf("Event filter (%d) only applicable for device-0\n", Parameter_List[arge].Integer_Value);
                        return QCLI_STATUS_ERROR_E;
                    }
                    evt_filter.event[i++] = (qapi_WLAN_Filterable_Event_e) Parameter_List[arge++].Integer_Value;
                    break;
                case QAPI_WLAN_BSSINFO_EVENTID_E:
                case QAPI_WLAN_CHANNEL_CHANGE_EVENTID_E:
                case QAPI_WLAN_ADDBA_REQ_EVENTID_E:
                case QAPI_WLAN_ADDBA_RESP_EVENTID_E:
                case QAPI_WLAN_DELBA_REQ_EVENTID_E:
                case QAPI_WLAN_WPS_PROFILE_EVENTID_E:
                case QAPI_WLAN_FLOW_CONTROL_EVENTID_E:
                case QAPI_WLAN_WNM_EVENTID_E:
                    evt_filter.event[i++] = (qapi_WLAN_Filterable_Event_e) Parameter_List[arge++].Integer_Value;
                    break;
                default:
                    QCLI_Printf("Invalid eventId: %d. This is not a subscribable event\n", Parameter_List[arge].Integer_Value);
                    return QCLI_STATUS_ERROR_E;
            }
        }
    }

    if(qc_drv_wlan_set_event_filter (qc_api_get_qc_drv_context(),
                device_id,
                &evt_filter,
                sizeof(evt_filter),
                FALSE) != 0)
    {
        QCLI_Printf("Could not set WLAN event filter list\r\n");
        return QCLI_STATUS_ERROR_E;
    }

    return QCLI_STATUS_SUCCESS_E;
}

/**********************************************************************************************
 * Function Name   : scan_control()
 * Returned Value  : 0 - on successful completion
 *                    -1 - on any failure.
 * Comments        : Disables/Enables foreground and background scan operations
 *                    in the Atheros device.  Both params must be provided where
 *                    foreground param is first followed by background param. a
 *                    '0' param disables the scan type while a '1' enables the
 *                    scan type.  Background scan -- firmware occasionally scans
 *                    while connected to a network. Foreground scan -- firmware
 *                    occasionally scans while disconnected to a network.
 **********************************************************************************************/
int32_t qc_api_wifi_scan_control( int32_t argc, int8_t* argv[])
{
    int32_t error = 0, fg = 0, bg = 0;
    qapi_WLAN_Scan_Params_t scanParam;
    uint32_t deviceId = 0;

    do{
        if(argc != 2) break;
        fg = atoi((char *)argv[0]);
        if(fg != 0 && fg != 1) break;
        bg = atoi((char *)argv[1]);
        if(bg != 0 && bg != 1) break;

        if( fg == 1 )
        {
            scanParam.fg_Start_Period = 0;
            scanParam.fg_End_Period = 0;
        }
        else
        {
            scanParam.fg_Start_Period = 0xffff;
            scanParam.fg_End_Period = 0xffff;
        }

        if( bg == 1 )
        {
            scanParam.bg_Period = 0;
        }
        else
        {
            scanParam.bg_Period = 0xffff;
        }

        scanParam.max_Act_Chan_Dwell_Time_In_Ms = 0;
        scanParam.passive_Chan_Dwell_Time_In_Ms = 0;
        scanParam.short_Scan_Ratio = QC_API_SHORTSCANRATIO_DEFAULT;
        scanParam.scan_Ctrl_Flags = QC_API_DEFAULT_SCAN_CTRL_FLAGS;
        scanParam.min_Act_Chan_Dwell_Time_In_Ms = 0;
        scanParam.max_Act_Scan_Per_Ssid = 0;
        scanParam.max_Dfs_Chan_Act_Time_In_Ms = 0;

        deviceId = qc_api_wifi_get_active_device();
        error = qc_drv_wlan_scan_param (qc_api_get_qc_drv_context(),
                deviceId,
                (void *) &scanParam,
                sizeof(scanParam),
                FALSE);
        if(error == -1){
            QCLI_Printf("driver ioctl error\r\n");
            return QCLI_STATUS_ERROR_E;
        }
        return QCLI_STATUS_SUCCESS_E;
    }while(0);

    QCLI_Printf("param error: scan control requires 2 inputs [0|1] [0|1]\r\n");
    return QCLI_STATUS_ERROR_E;
}

/**********************************************************************************************
 * Function Name   : set_scan_para()
 * Returned Value  : 0 - on successful completion
 *                    -1 - on any failure.
 * Comments        : set scan parameters
 **********************************************************************************************/
int32_t qc_api_wifi_set_scan_parameters(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int32_t error = 0;
    qapi_WLAN_Scan_Params_t scan_params;
    uint32_t deviceId = 0;
    if(Parameter_Count != 10 || !Parameter_List)
        return QCLI_STATUS_ERROR_E;

    scan_params.max_Act_Chan_Dwell_Time_In_Ms = (uint16_t) Parameter_List[0].Integer_Value;
    scan_params.passive_Chan_Dwell_Time_In_Ms = (uint16_t) Parameter_List[1].Integer_Value;
    scan_params.fg_Start_Period = (uint16_t) Parameter_List[2].Integer_Value;
    scan_params.fg_End_Period = (uint16_t) Parameter_List[3].Integer_Value;
    scan_params.bg_Period = (uint16_t) Parameter_List[4].Integer_Value;
    scan_params.short_Scan_Ratio = (uint8_t) Parameter_List[5].Integer_Value;
    scan_params.scan_Ctrl_Flags = (uint8_t) Parameter_List[6].Integer_Value;
    scan_params.min_Act_Chan_Dwell_Time_In_Ms = (uint16_t) Parameter_List[7].Integer_Value;
    scan_params.max_Act_Scan_Per_Ssid = (uint16_t) Parameter_List[8].Integer_Value;
    scan_params.max_Dfs_Chan_Act_Time_In_Ms = (uint32_t) Parameter_List[9].Integer_Value;

    deviceId = qc_api_wifi_get_active_device();
    error = qc_drv_wlan_scan_param (qc_api_get_qc_drv_context(),
            deviceId,
            &scan_params,
            sizeof(scan_params),
            FALSE);
    if(error == -1){
        QCLI_Printf("Set scan params failed\r\n");
        return QCLI_STATUS_ERROR_E;
    }
    return QCLI_STATUS_SUCCESS_E;
}

/**********************************************************************************************
 * Function Name   : start_scan()
 * Returned Value  : 0 - on successful completion
 *                    -1 - on any failure.
 * Comments        : set scan --- do real scan
 **********************************************************************************************/
int32_t qc_api_wifi_start_scan(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int32_t error = 0;
    qapi_WLAN_Start_Scan_Params_t *scan_params = NULL;
    uint16_t maxArgCount = 0, argI = 0;
    uint32_t deviceId = 0, wifimode = 0, dataLen = 0, size = 0;
    uint8_t numChannels = 0, orig_ssid[__QAPI_WLAN_MAX_SSID_LENGTH] = {'\0'};

    if (Parameter_Count < 4 || !Parameter_List)
        return QCLI_STATUS_ERROR_E;
#if ENABLE_P2P_MODE
    if(p2p_session_in_progress)
    {
        QCLI_Printf("p2p event in progress \r\n");
        return QCLI_STATUS_ERROR_E;
    }
#endif

    memset(orig_ssid,0,__QAPI_WLAN_MAX_SSID_LENGTH);
    deviceId = qc_api_wifi_get_active_device();
    qc_drv_wlan_get_ssid(qc_api_get_qc_drv_context(),
            deviceId,
            (uint8_t *) orig_ssid,
            &dataLen);
    qc_drv_wlan_get_op_mode(qc_api_get_qc_drv_context(),
            deviceId,
            &wifimode,
            &dataLen);

    if(wifimode == QAPI_WLAN_DEV_MODE_AP_E)
    {
        QCLI_Printf("Cannot scan in AP mode\r\n");
        return QCLI_STATUS_ERROR_E;
    }

    do
    {
        numChannels = (uint8_t) Parameter_List[3].Integer_Value;
        if (numChannels > __QAPI_WLAN_START_SCAN_PARAMS_CHANNEL_LIST_MAX)
        {
            QCLI_Printf("Cannot set more than 12 channels to scan\r\n");
            return QCLI_STATUS_ERROR_E;
        }

        size = sizeof(qapi_WLAN_Start_Scan_Params_t);
        if(numChannels > 0) {
            size += (((uint8_t)Parameter_List[3].Integer_Value - 1) * sizeof(uint16_t));
        }

        if(NULL == (scan_params = malloc(size))){
            QCLI_Printf("Setting of scan parameters failed due to insufficient memory.\r\n");
            return QCLI_STATUS_ERROR_E;
        }

        char *ssid = "";
        scan_params->force_Fg_Scan = (boolean_t) Parameter_List[0].Integer_Value;
        scan_params->home_Dwell_Time_In_Ms = (uint32_t) Parameter_List[1].Integer_Value;
        scan_params->force_Scan_Interval_In_Ms = (uint32_t) Parameter_List[2].Integer_Value;
        scan_params->scan_Type = 0;
        scan_params->num_Channels = numChannels;
        maxArgCount = scan_params->num_Channels + 4;
        argI = 4;
        if (Parameter_Count != maxArgCount)
        {
            QCLI_Printf("Invalid parameter list\r\n");
            error = -1;
            break;
        }

        while (argI < maxArgCount)
        {
            scan_params->channel_List[argI-4] = (uint16_t) Parameter_List[argI].Integer_Value;
            argI++;
        }
        error = qc_drv_wlan_set_ssid (qc_api_get_qc_drv_context(),
                deviceId,
                (uint8_t *)ssid,
                strlen((char *)ssid),
                FALSE);
        if(error != 0)
        {
            QCLI_Printf("Unable to set SSID\r\n");
            error = -1;
            break;
        }

        /*Do the actual scan*/
        error = qc_api_wifi_wmi_set_scan(deviceId, scan_params, QAPI_WLAN_BUFFER_SCAN_RESULTS_BLOCKING_E);
        if(error != 0)
        {
            QCLI_Printf("Custom scan failed\r\n");
            error = -1;
            /* No break here, fall through to revert to original SSID */
        }

        /*Revert to original SSID*/
        if ('\0' !=  *orig_ssid) {
            error = qc_drv_wlan_set_ssid (qc_api_get_qc_drv_context(),
                    deviceId,
                    orig_ssid,
                    strlen((const char*) orig_ssid),
                    FALSE);
            if(error != 0)
            {
                QCLI_Printf("Unable to set SSID\r\n");
                error = -1;
                break;
            }
        }
    } while(0);

    if (NULL != scan_params) {
        free(scan_params);
    }
    return error;
}
#endif
/**********************************************************************************************
 * Function Name   : get_rate()
 * Returned Value  : 0 - on successful completion
 *                    -1 - on any failure.
 * Comments        : gets TX rate from chip
 **********************************************************************************************/
int32_t qc_api_wifi_get_rate()
{
    int32_t rateIndex = 0;
    uint32_t dataLen = 0;

    if ( 0 != qc_drv_wlan_get_rate(qc_api_get_qc_drv_context(),
                qc_api_wifi_get_active_device(),
                &rateIndex,
                &dataLen))
    {
        LOG_ERR("Command Failed\r\n");
        return QCLI_STATUS_ERROR_E;
    }
    LOG_AT("rateIdx = %d\n", rateIndex);

    return QCLI_STATUS_SUCCESS_E;
}

/**********************************************************************************************
 * Function Name   : set_rate()
 * Returned Value  : 0 - on successful completion
 *                    -1 - on any failure.
 * Comments        : sets TX data rate
 **********************************************************************************************/
int32_t qc_api_wifi_set_rate(int32_t isMcs, int32_t rateIdx)
{
    int32_t error = -1;
    qapi_WLAN_Bit_Rate_t rateIndex;

    if(!isMcs)
    {
        switch(rateIdx) {
            case 1:
                rateIndex = QAPI_WLAN_RATE_1Mb_E;
                break;
            case 2:
                rateIndex = QAPI_WLAN_RATE_2Mb_E;
                break;
            case 5:
                rateIndex = QAPI_WLAN_RATE_5_5Mb_E;
                break;
            case 11:
                rateIndex = QAPI_WLAN_RATE_11Mb_E;
                break;
            case 6:
                rateIndex = QAPI_WLAN_RATE_6Mb_E;
                break;
            case 9:
                rateIndex = QAPI_WLAN_RATE_9Mb_E;
                break;
            case 12:
                rateIndex = QAPI_WLAN_RATE_12Mb_E;
                break;
            case 18:
                rateIndex = QAPI_WLAN_RATE_18Mb_E;
                break;
            case 24:
                rateIndex = QAPI_WLAN_RATE_24Mb_E;
                break;
            case 36:
                rateIndex = QAPI_WLAN_RATE_36Mb_E;
                break;
            case 48:
                rateIndex = QAPI_WLAN_RATE_48Mb_E;
                break;
            case 54:
                rateIndex = QAPI_WLAN_RATE_54Mb_E;
                break;
            default:
                LOG_ERR("Unsupported rate\r\n");
                return error;
        }
    }
    else
    {
        switch(rateIdx) {
            case 0:
                rateIndex = QAPI_WLAN_RATE_MCS_0_20_E;
                break;
            case 1:
                rateIndex = QAPI_WLAN_RATE_MCS_1_20_E;
                break;
            case 2:
                rateIndex = QAPI_WLAN_RATE_MCS_2_20_E;
                break;
            case 3:
                rateIndex = QAPI_WLAN_RATE_MCS_3_20_E;
                break;
            case 4:
                rateIndex = QAPI_WLAN_RATE_MCS_4_20_E;
                break;
            case 5:
                rateIndex = QAPI_WLAN_RATE_MCS_5_20_E;
                break;
            case 6:
                rateIndex = QAPI_WLAN_RATE_MCS_6_20_E;
                break;
            case 7:
                rateIndex = QAPI_WLAN_RATE_MCS_7_20_E;
                break;
            default:
                LOG_ERR("undefined mcs rate \r\n");
                return error;
        }
    }


    if(0 != qc_drv_wlan_set_rate(qc_api_get_qc_drv_context(),
                qc_api_wifi_get_active_device(),
                &rateIndex,
                sizeof(rateIndex),
                FALSE))
    {
        LOG_ERR("Command Failed\r\n");
        return error;
    }
    return QCLI_STATUS_SUCCESS_E;
}

/**********************************************************************************************
 * Function Name   : set_channel_hint()
 * Returned Value  : 0 - on successful completion
 *                   1 - on any failure.
 * Comments        : Sets the channel hint
 **********************************************************************************************/
int32_t qc_api_wifi_set_channel_hint(int32_t channelNum)
{
    uint32_t deviceId = qc_api_wifi_get_active_device();
    if (0 == qc_drv_wlan_set_channel(qc_api_get_qc_drv_context(),
                deviceId,
                (void *) &channelNum,
                sizeof(channelNum),
                FALSE))
    {
        return QCLI_STATUS_SUCCESS_E;
    }
    return QCLI_STATUS_ERROR_E;
}

/**********************************************************************************************
 * Function Name   : set_tx_power()
 * Returned Value  : 0 - on successful completion
 *                   1 - on any failure.
 * Comments        : Sets the transmit power parameter
 **********************************************************************************************/
int32_t qc_api_wifi_set_tx_power(int32_t power_in_dBm)
{
    uint32_t deviceId = qc_api_wifi_get_active_device();

    if (0 == qc_drv_wlan_set_tx_power_in_dbm(qc_api_get_qc_drv_context(),
                deviceId,
                &power_in_dBm,
                sizeof(power_in_dBm),
                FALSE))
    {
        return QCLI_STATUS_SUCCESS_E;
    }
    return QCLI_STATUS_ERROR_E;
}

/**********************************************************************************************
 * Function Name   : set_phy_mode()
 * Returned Value  : 0 - on successful completion
 *                   1 - on any failure.
 * Comments        : Sets the WLAN PHY mode
 **********************************************************************************************/
int32_t qc_api_wifi_set_phy_mode(int32_t wmode)
{
    qapi_WLAN_Phy_Mode_e phyMode;
    if (wmode == 1) {
        phyMode = QAPI_WLAN_11A_MODE_E;
    } else if (wmode == 2) {
        phyMode = QAPI_WLAN_11B_MODE_E;
    } else if (wmode == 3) {
        phyMode = QAPI_WLAN_11G_MODE_E;
    } else if (wmode == 4) {
        phyMode = QAPI_WLAN_11AG_MODE_E;
    } else if (wmode == 5) {
        phyMode = QAPI_WLAN_11GONLY_MODE_E;
    } else {
        LOG_ERR("Unknown wmode, only support a/b/g/ag/gonly/\r\n");
        return QCLI_STATUS_ERROR_E;
    }

    if (0 == qc_drv_wlan_set_phy_mode(qc_api_get_qc_drv_context(),
                qc_api_wifi_get_active_device(),
                &phyMode,
                sizeof(phyMode),
                FALSE))
    {
        return QCLI_STATUS_SUCCESS_E;
    }
    return QCLI_STATUS_ERROR_E;
}

/**********************************************************************************************
 * Function Name   : set_11n_ht()
 * Returned Value  : 0 - on successful completion
 *                   1 - on any failure.
 * Comments        : Sets the WLAN interface to 11n HT mode
 **********************************************************************************************/
int32_t qc_api_wifi_set_11n_ht(int8_t *ht_config)
{
    qapi_WLAN_11n_HT_Config_e htconfig;
    if (0 == QC_API_APP_STRCASECMP(ht_config, "disable")) {
        htconfig = QAPI_WLAN_11N_DISABLED_E;
    } else if (0 == QC_API_APP_STRCASECMP(ht_config, "ht20")) {
        htconfig = QAPI_WLAN_11N_HT20_E;
    } else {
        LOG_ERR("Unknown ht config, only support disable/ht20\r\n");
        return QCLI_STATUS_ERROR_E;
    }

    if (0 == qc_drv_wlan_set_ht_config(qc_api_get_qc_drv_context(),
                qc_api_wifi_get_active_device(),
                &htconfig,
                sizeof(htconfig),
                FALSE))
    {
        return QCLI_STATUS_SUCCESS_E;
    }

    return QCLI_STATUS_ERROR_E;
}

/**********************************************************************************************
 * Function Name   : set_op_mode()
 * Returned Value  : 0 - on successful completion
 *                   1 - on any failure.
 * Comments        : Sets the WLAN operation mode
 **********************************************************************************************/
int32_t qc_api_wifi_set_op_mode(int8_t *omode, char *hiddenSsid, char *wpsEnabled)
{
    char *ssid = "";
    int32_t deviceId = qc_api_wifi_get_active_device();
    qc_drv_wlan_set_ssid(qc_api_get_qc_drv_context(),
            deviceId,
            (uint8_t *)ssid,
            strlen((const char *) ssid),
            FALSE);

    if(QC_API_APP_STRCASECMP(omode, "ap") == 0)
    {
        qapi_WLAN_Dev_Mode_e opMode;
        if (QC_API_APP_STRCASECMP(hiddenSsid, "hidden") == 0)
        {
            hidden_flag = 1;
        }
        else if(QC_API_APP_STRCASECMP(hiddenSsid, "0") == 0 || QC_API_APP_STRCASECMP(hiddenSsid, "") == 0)
        {
            hidden_flag = 0;
        }
        else
        {
            return QCLI_STATUS_USAGE_E;
        }
        if (QC_API_APP_STRCASECMP(wpsEnabled, "wps") == 0)
        {
            if(hidden_flag == 1)
            {
                wps_flag = 0;
                LOG_ERR("WPS could not be enabled in AP hidden mode.\r\n");
                return QCLI_STATUS_ERROR_E;
            }
            else
            {
                wps_flag = 1;
            }
        }
        else if(QC_API_APP_STRCASECMP(wpsEnabled, "0") == 0 || QC_API_APP_STRCASECMP(wpsEnabled, "") == 0)
        {
            wps_flag = 0;
        }
        else
        {
            return QCLI_STATUS_USAGE_E;
        }
        opMode = QAPI_WLAN_DEV_MODE_AP_E;

        if (0 != qc_drv_wlan_set_op_mode(qc_api_get_qc_drv_context(),
                    deviceId,
                    &opMode,
                    sizeof(qapi_WLAN_Dev_Mode_e),
                    FALSE))


        {
            LOG_ERR("Not able to set op mode to AP \r\n");
            return QCLI_STATUS_ERROR_E;
        }

        /* Set to MAX PERF Mode */
        qc_api_wifi_set_power_mode( QAPI_WLAN_POWER_MODE_MAX_PERF_E, QAPI_WLAN_POWER_MODULE_SOFTAP_E);

        if ( 0 != qc_drv_wlan_set_ap_enable_hidden_mode(qc_api_get_qc_drv_context(),
                    deviceId,
                    &hidden_flag,
                    sizeof(hidden_flag),
                    FALSE))
        {
            LOG_ERR("Not able to set hidden mode for AP \r\n");
            return QCLI_STATUS_ERROR_E;
        }

        if (0 != qc_drv_wlan_set_ap_wps_flag(qc_api_get_qc_drv_context(),
                    deviceId,
                    &wps_flag,
                    sizeof(wps_flag),
                    FALSE))
        {
            LOG_ERR("Not able to set wps mode for AP \r\n");
            return QCLI_STATUS_ERROR_E;
        }
        return QCLI_STATUS_SUCCESS_E;
    }
    else if(QC_API_APP_STRCASECMP(omode, "station") == 0)
    {
        qapi_WLAN_Dev_Mode_e opMode, wifimode;
        uint32_t dataLen;

        qc_drv_wlan_get_op_mode(qc_api_get_qc_drv_context(),
                deviceId,
                (uint32_t *)&wifimode,
                &dataLen);

        if (wifimode == QAPI_WLAN_DEV_MODE_AP_E) /*check previous mode*/
        {
            qc_drv_wlan_disconnect(qc_api_get_qc_drv_context(), deviceId);
            /* AP Mode is always set to maxper; if we are switching mode and prev mode is QAPI_WLAN_POWER_MODE_REC_POWER_E then
               retain the power mode for STA */

            qc_api_wifi_set_power_mode( QAPI_WLAN_POWER_MODE_REC_POWER_E, QAPI_WLAN_POWER_MODULE_SOFTAP_E);
        }
        opMode = QAPI_WLAN_DEV_MODE_STATION_E;
        if (0 == qc_drv_wlan_set_op_mode(qc_api_get_qc_drv_context(),
                    deviceId,
                    &opMode,
                    sizeof(qapi_WLAN_Dev_Mode_e),
                    FALSE))
            return QCLI_STATUS_SUCCESS_E;
        else
            return QCLI_STATUS_ERROR_E;
    }

    return QCLI_STATUS_USAGE_E;
}

/**********************************************************************************************
 * Function Name   : set_pwr_mode()
 * Returned Value  : 0 - on successful completion
 *                   1 - on any failure.
 * Comments        : Sets the WLAN power mode
 **********************************************************************************************/
int32_t qc_api_wifi_set_pwr_mode(int32_t power_mode)
{
    uint32_t dataLen = 0, wifimode = 0;
    uint32_t deviceId = qc_api_wifi_get_active_device();
    if(power_mode == 0){
        user_defined_power_mode = QAPI_WLAN_POWER_MODE_MAX_PERF_E;
        return qc_api_wifi_set_power_mode( QAPI_WLAN_POWER_MODE_MAX_PERF_E, QAPI_WLAN_POWER_MODULE_USER_E);
    }else{

        qc_drv_wlan_get_op_mode(qc_api_get_qc_drv_context(),
                deviceId,
                &wifimode,
                &dataLen);

        if(wifimode == QAPI_WLAN_DEV_MODE_AP_E && !(p2pMode)) {
            LOG_ERR("Setting REC Power is not allowed MODE_AP \r\n");
            return QCLI_STATUS_ERROR_E;
        }
        user_defined_power_mode = QAPI_WLAN_POWER_MODE_REC_POWER_E;
        return qc_api_wifi_set_power_mode( QAPI_WLAN_POWER_MODE_REC_POWER_E, QAPI_WLAN_POWER_MODULE_USER_E);
    }
}

/**********************************************************************************************
 * Function Name   : set_power_mgmt_policy_params()
 * Returned Value  : 0 - on successful completion
 *                   1 - on any failure.
 * Comments        : Sets the WLAN power management policy parameters
 **********************************************************************************************/
int32_t qc_api_wifi_set_power_mgmt_policy_params(int32_t idle_period_ms, int32_t num_ps_poll,
        int32_t dtim_policy, int32_t tx_wakeup_policy,
        int32_t num_tx_to_wake_host, int32_t ps_fail_event_policy)
{
    qapi_WLAN_Power_Policy_Params_t pm;

    /*default values
      pm.idle_Period_In_Ms = 0;
      pm.ps_Poll_Num = 10;
      pm.dtim_Policy = 3;
      pm.tx_Wakeup_Policy = 2;
      pm.num_Tx_To_Wakeup = 1;
      pm.ps_Fail_Event_Policy = 2;
      */
    pm.idle_Period_In_Ms = idle_period_ms;
    pm.ps_Poll_Num = num_ps_poll;
    pm.dtim_Policy = dtim_policy;
    pm.tx_Wakeup_Policy = tx_wakeup_policy;
    pm.num_Tx_To_Wakeup = num_tx_to_wake_host;
    pm.ps_Fail_Event_Policy = ps_fail_event_policy;

    return qc_drv_wlan_set_pw_mode_policy (qc_api_get_qc_drv_context(),
            qc_api_wifi_get_active_device(),
            &pm,
            sizeof(qapi_WLAN_Power_Policy_Params_t),
            FALSE);
}

/**********************************************************************************************
 * Function Name   : get_version()
 * Returned Value  : 0 version is retrieved successfully else ERROR CODE
 * Comments        : gets driver,firmware version.
 **********************************************************************************************/
int32_t qc_api_wifi_get_version()
{
    qapi_WLAN_Firmware_Version_String_t versionstr;
    uint32_t dataLen = 0;

    if (0 != qc_drv_wlan_get_firmware_version(qc_api_get_qc_drv_context(),
                qc_api_wifi_get_active_device(),
                &versionstr,
                &dataLen))
    {
        LOG_ERR("Command failed\n");
        return QCLI_STATUS_ERROR_E;
    }
    else
    {
        //QCLI_Printf("Host version :  %s\r\n", versionstr.host_Version);
        //QCLI_Printf("Target version   :  %s\r\n", versionstr.target_Version);
        LOG_AT("Firmware version :  %s\r\n", versionstr.wlan_Version);
        LOG_AT("Interface version:  %s\r\n", versionstr.abi_Version);
    }
    return QCLI_STATUS_SUCCESS_E;
}

/**********************************************************************************************
 * Function Name   : get_reg_domain()
 * Returned Value  : 0 - on successful completion
 *                  -1 - on any failure.
 * Comments        : gets Regulatory domain from driver
 **********************************************************************************************/
int32_t qc_api_wifi_get_reg_domain()
{
    uint32_t regDomain = 0, dataLen = 0;

    if (0 != qc_drv_wlan_get_reg_domain(qc_api_get_qc_drv_context(),
                qc_api_wifi_get_active_device(),
                &regDomain,
                &dataLen))
    {
        LOG_ERR("Command failed\n");
        return QCLI_STATUS_ERROR_E;
    }
    else
    {
        LOG_AT("Regulatory Domain 0x%x\r\n",regDomain);
    }
    return QCLI_STATUS_SUCCESS_E;
}

/**********************************************************************************************
 * Function Name   : get_last_error()
 * Returned Value  : 0 if phy_mode set successfully else ERROR CODE
 * Comments        : Gets the last error in the host driver
 **********************************************************************************************/
int32_t qc_api_wifi_get_last_error()
{

    int32_t err = 0;
    uint32_t dataLen = 0;

    if (0 != qc_drv_wlan_get_last_error(qc_api_get_qc_drv_context(),
                qc_api_wifi_get_active_device(),
                &err,
                &dataLen))
    {
        LOG_ERR("Command failed\n");
        return QCLI_STATUS_ERROR_E;
    }
    else
    {
        LOG_AT("Last driver error: 0x%x\r\n", err);
    }
    return QCLI_STATUS_SUCCESS_E;
}

/**********************************************************************************************
 * Function Name   : enable_suspend()
 * Returned Value  : 0 - on successful completion
 *                  -1 - on any failure.
 * Comments        : Suspends the WLAN
 **********************************************************************************************/
 #if 0
int32_t qc_api_wifi_enable_suspend ()
{
    int32_t suspend = 1;
    return qc_drv_wlan_set_suspend_resume (qc_api_get_qc_drv_context(), qc_api_wifi_get_active_device(),
            (void *)&suspend,
            sizeof(suspend),
            FALSE);
}

/**********************************************************************************************
 * Function Name   : dev_susp_start()
 * Returned Value  : 0 if device suspend is started
 *                   successfully else ERROR CODE
 * Comments        : Suspends device for requested time period
 **********************************************************************************************/
int32_t qc_api_wifi_dev_susp_start(int32_t susp_time)
{
    uint32_t deviceId = 0, wifimode = 0, txStatus = 0, dataLen = 0;

    deviceId = qc_api_wifi_get_active_device();
    qc_drv_wlan_get_op_mode(qc_api_get_qc_drv_context(),
            deviceId,
            &wifimode,
            &dataLen);
    if (wifimode == QAPI_WLAN_DEV_MODE_AP_E)
    {
        QCLI_Printf("Store-Recall Mechanism NOT supported in 'MODE_AP' \r\n");
        return QCLI_STATUS_ERROR_E;
    }

    /*Check if no packets are queued, if TX is pending, then wait*/
    do{
        if (0 == qc_drv_wlan_get_tx_status(qc_api_get_qc_drv_context(),
                    deviceId,
                    &txStatus, &dataLen))
        {
            if (txStatus == __QAPI_WLAN_TX_STATUS_IDLE)
                break;
        }
        else
        {
            qurt_thread_sleep(500);
            //_sched_yield();
        }
    } while (1);
    return qc_drv_wlan_suspend_start(qc_api_get_qc_drv_context(), susp_time);
}
#endif
/**********************************************************************************************
 * Function Name   : wlan_scan()
 * Returned Value  : 0 if success else -1
 * Comments        : Setup for scan command
 **********************************************************************************************/
int32_t qc_api_wifi_wlan_scan(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    uint8_t scan_ssid_flag = 0;            //Flag to decide whether to scan all ssid/channels
    uint8_t scan_ssid[__QAPI_WLAN_MAX_SSID_LENGTH], orig_ssid[__QAPI_WLAN_MAX_SSID_LENGTH];
    int32_t error = 0;
    uint32_t deviceId = 0, wifimode = 0, dataLen = 0;
    qapi_WLAN_Store_Scan_Results_e scan_mode = 0;  //Default mode is BLOCKING and BUFFERING

#if ENABLE_P2P_MODE
    if(p2p_session_in_progress)
    {
        LOG_INFO("p2p event in progress \r\n");
        return QCLI_STATUS_ERROR_E;
    }
#endif
    deviceId = qc_api_wifi_get_active_device();

    memset(scan_ssid,0,__QAPI_WLAN_MAX_SSID_LENGTH);
    memset(orig_ssid,0,__QAPI_WLAN_MAX_SSID_LENGTH);

    qc_drv_wlan_get_ssid(qc_api_get_qc_drv_context(),
            deviceId,
            (uint8_t *) orig_ssid,
            &dataLen);

    qc_drv_wlan_get_op_mode(qc_api_get_qc_drv_context(),
            deviceId,
            &wifimode,
            &dataLen);

    if (wifimode == QAPI_WLAN_DEV_MODE_AP_E)
    {
        LOG_ERR("AP mode cannot scan\r\n");
        return QCLI_STATUS_ERROR_E;
    }

    if(Parameter_Count >= 1 && Parameter_List[0].Integer_Is_Valid)
    {

        if((Parameter_List[0].Integer_Value < QAPI_WLAN_BUFFER_SCAN_RESULTS_BLOCKING_E) ||
                (Parameter_List[0].Integer_Value > QAPI_WLAN_BUFFER_SCAN_RESULTS_NON_BLOCKING_E))
        {
            LOG_WARN("Invalid scan mode - Using default\r\n");
            scan_mode = 0;
        }
        else {
            scan_mode = Parameter_List[0].Integer_Value;
        }

        if(Parameter_Count == 2 && !Parameter_List[1].Integer_Is_Valid)
        {
            if(strlen((const char *) Parameter_List[1].String_Value) > __QAPI_WLAN_MAX_SSID_LENGTH)
            {
                LOG_ERR("SSID length exceeds Maximum value\r\n");
                return QCLI_STATUS_ERROR_E;
            }
            /*Scan specified SSID*/
            scan_ssid_flag = 1;
            strcpy((char *)scan_ssid, (char *) Parameter_List[1].String_Value);
        }
    }

    /*Set SSID for scan*/
    if(scan_ssid_flag)
    {
        error = qc_drv_wlan_set_ssid(qc_api_get_qc_drv_context(),
                deviceId,
                scan_ssid,
                strlen((const char *) scan_ssid),
                FALSE);
        if(error != 0)
        {
            LOG_ERR("Unable to set SSID\r\n");
            return error;
        }
    }
    else
    {
        char *ssid = "";
        error = qc_drv_wlan_set_ssid(qc_api_get_qc_drv_context(),
                deviceId,
                (uint8_t *)ssid,
                strlen((const char *)ssid),
                FALSE);
        if(error != 0)
        {
            LOG_ERR("Unable to set SSID\r\n");
            return error;
        }
    }

    /*Do the actual scan*/
    qc_api_wifi_wmi_set_scan(deviceId, NULL, scan_mode);

    /*Revert to original SSID*/
    error = qc_drv_wlan_set_ssid(qc_api_get_qc_drv_context(),
            deviceId,
            orig_ssid,
            strlen((const char *) orig_ssid),
            FALSE);
    if(error != 0)
    {
        LOG_ERR("Unable to set SSID\r\n");
        return error;
    }
    return error;
}

/**********************************************************************************************
 * Function Name   : wlan_scan()
 * Returned Value  : 0 if success else -1
 * Comments        : Prints scan result
 **********************************************************************************************/
void qc_api_wifi_wlan_scan_result(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    qc_api_wifi_print_scan_results(scan_param, scan_param.num_Scan_Entries);
    LOG_AT("Scan result count:%d\r\n", scan_param.num_Scan_Entries);
    /* Free the scan result memory allocated by qc_drv_WLAN_Get_Scan_Results */
    free(scan_param.scan_List);
}

/**********************************************************************************************
 * Function Name   : set_ap_inactivity_period()
 * Returned Value  : 0 if success else 1
 * Comments        : Set the AP inactivity period
 **********************************************************************************************/
#if QC_API_ENABLE_AP_MODE

int32_t qc_api_wifi_set_ap_inactivity_period(uint32_t inactivity_time_in_mins)
{
    uint32_t deviceId = 0, wifimode = 0, dataLen = 0;
    deviceId = qc_api_wifi_get_active_device();

    if (0 != qc_drv_wlan_get_op_mode(qc_api_get_qc_drv_context(),
                deviceId,
                &wifimode,
                &dataLen))
    {
        return QCLI_STATUS_ERROR_E;
    }
    if (QAPI_WLAN_DEV_MODE_AP_E != wifimode)
    {
        LOG_INFO("Set AP Mode to apply AP settings\r\n");
        return QCLI_STATUS_ERROR_E;
    }
    if (0 != qc_drv_wlan_set_inactivity_time (qc_api_get_qc_drv_context(),
                deviceId,
                (void *)&inactivity_time_in_mins,
                sizeof(inactivity_time_in_mins),
                FALSE))
    {
        LOG_ERR("Unable to set inactive_time\r\n");
        return QCLI_STATUS_ERROR_E;
    }
    return QCLI_STATUS_SUCCESS_E;
}

/**********************************************************************************************
 * Function Name   : set_ap_beacon_interval()
 * Returned Value  : 0 if success else 1
 * Comments        : Set the AP beacon interval period
 **********************************************************************************************/
int32_t qc_api_wifi_set_ap_beacon_interval(uint32_t beacon_int_in_tu)
{
    uint32_t deviceId = 0, wifimode = 0, dataLen = 0;
    deviceId = qc_api_wifi_get_active_device();

    if(0 != qc_drv_wlan_get_op_mode(qc_api_get_qc_drv_context(),
                deviceId,
                &wifimode,
                &dataLen))
    {
        return QCLI_STATUS_ERROR_E;
    }

    if (QAPI_WLAN_DEV_MODE_AP_E != wifimode)
    {
        LOG_INFO("Set AP Mode to apply AP settings\r\n");
        return QCLI_STATUS_ERROR_E;
    }
    if((beacon_int_in_tu < 100) || (beacon_int_in_tu > 1000))
    {
        LOG_WARN("beacon interval has to be within 100-1000 in units of 100 \r\n");
        return QCLI_STATUS_ERROR_E;
    }

    if (0 != qc_drv_wlan_set_beacon_interval_in_tu(qc_api_get_qc_drv_context(),
                deviceId,
                &beacon_int_in_tu,
                sizeof(beacon_int_in_tu),
                FALSE))
    {
        LOG_ERR("Unable to set beacon_int_in_ms\r\n");
        return QCLI_STATUS_ERROR_E;
    }
    return QCLI_STATUS_SUCCESS_E;
}

/**********************************************************************************************
 * Function Name   : set_ap_dtim_period()
 * Returned Value  : 0 if success else 1
 * Comments        : Set the AP DTIM period
 **********************************************************************************************/
int32_t qc_api_wifi_set_ap_dtim_period(uint32_t dtim_period)
{
    uint32_t deviceId = 0, wifimode = 0, dataLen = 0;
    deviceId = qc_api_wifi_get_active_device();

    if(0 != qc_drv_wlan_get_op_mode(qc_api_get_qc_drv_context(),
                deviceId,
                &wifimode,
                &dataLen))
    {
        return QCLI_STATUS_ERROR_E;
    }
    if (QAPI_WLAN_DEV_MODE_AP_E != wifimode)
    {
        LOG_INFO("Set AP Mode to apply AP settings\r\n");
        return QCLI_STATUS_ERROR_E;
    }
    if((dtim_period < 1) || (dtim_period > 255))
    {
        return QCLI_STATUS_ERROR_E;
    }

    if (0 != qc_drv_wlan_set_ap_dtim_interval(qc_api_get_qc_drv_context(),
                deviceId,
                &dtim_period,
                sizeof(dtim_period),
                FALSE))
    {
        LOG_ERR("Unable set dtim_period\r\n");
        return QCLI_STATUS_ERROR_E;
    }

    return QCLI_STATUS_SUCCESS_E;
}

/**********************************************************************************************
 * Function Name   : set_ap_ps_buf()
 * Returned Value  : 0 if success else 1
 * Comments        : Set the buffer in AP for the power save mode
 **********************************************************************************************/
#if 0
int32_t qc_api_wifi_set_ap_ps_buf(uint32_t ps_buf_enable, uint32_t buff_count)
{
    uint32_t deviceId = 0, wifimode = 0, dataLen = 0;
    uint8_t ps_val[2] = {0};
    deviceId = qc_api_wifi_get_active_device();

    if (0 != qc_drv_wlan_get_op_mode(qc_api_get_qc_drv_context(),
                deviceId,
                &wifimode,
                &dataLen))
    {
        return QCLI_STATUS_ERROR_E;
    }
    if (QAPI_WLAN_DEV_MODE_AP_E != wifimode)
    {
        LOG_INFO("Set AP Mode to apply AP settings\r\n");
        return QCLI_STATUS_ERROR_E;
    }
    ps_val[0] = (uint8_t) ps_buf_enable;

    ps_val[1] = (ps_buf_enable == 0) ? 0: (uint8_t) buff_count;

    if (0 != qc_drv_wlan_set_ap_ps_buf (qc_api_get_qc_drv_context(),
                deviceId,
                (void *)&ps_val,
                sizeof(ps_val),
                FALSE))
    {
        LOG_ERR("Unable to set PS-Buff\r\n");
        return QCLI_STATUS_ERROR_E;
    }
    return QCLI_STATUS_SUCCESS_E;
}

/**********************************************************************************************
 * Function Name   : set_ap_uapsd()
 * Returned Value  : 0 if success else 1
 * Comments        : Enable the AP UAPSD
 **********************************************************************************************/
int32_t qc_api_wifi_set_ap_uapsd(uint32_t uapsd_enable)
{
    uint32_t deviceId = 0, wifimode = 0, dataLen = 0;

    deviceId = qc_api_wifi_get_active_device();
    if (0 != qc_drv_wlan_get_op_mode(qc_api_get_qc_drv_context(),
                deviceId,
                &wifimode,
                &dataLen))
    {
        return QCLI_STATUS_ERROR_E;
    }
    if (QAPI_WLAN_DEV_MODE_AP_E != wifimode)
    {
        return QCLI_STATUS_ERROR_E;
    }
    if((uapsd_enable != 0) && (uapsd_enable != 1))
    {
        return QCLI_STATUS_ERROR_E;
    }
    if (0 != qc_drv_wlan_set_ap_enable_uapsd (qc_api_get_qc_drv_context(),
                deviceId,
                (void *)&uapsd_enable,
                sizeof(uapsd_enable),
                FALSE))
    {
        return QCLI_STATUS_ERROR_E;
    }
    return QCLI_STATUS_SUCCESS_E;
}
#endif //if 0

#endif //QC_API_ENABLE_AP_MODE

/**********************************************************************************************
 * Function Name   : set_sta_uapsd()
 * Returned Value  : 0 if success else 1
 * Comments        : Enable the STA UAPSD
 **********************************************************************************************/
 #if 0
int32_t qc_api_wifi_set_sta_uapsd(uint32_t ac_mask)
{
    uint32_t deviceId = 0, wifimode = 0, dataLen = 0;

    deviceId = qc_api_wifi_get_active_device();
    if (0 != qc_drv_wlan_get_op_mode(qc_api_get_qc_drv_context(),
                deviceId,
                &wifimode,
                &dataLen))
    {
        return QCLI_STATUS_ERROR_E;
    }
    if (QAPI_WLAN_DEV_MODE_STATION_E != wifimode)
    {
        return QCLI_STATUS_ERROR_E;
    }
    if(ac_mask < 0 || ac_mask > 15)
    {
        return QCLI_STATUS_ERROR_E;
    }
    if (0 != qc_drv_wlan_set_sta_uapsd (qc_api_get_qc_drv_context(),
                deviceId,
                (void *)&ac_mask,
                sizeof(ac_mask),
                FALSE))
    {
        return QCLI_STATUS_ERROR_E;
    }
    return QCLI_STATUS_SUCCESS_E;
}

/**********************************************************************************************
 * Function Name   : set_max_sp_len()
 * Returned Value  : 0 if success else 1
 * Comments        : Set the maximum sp_len
 **********************************************************************************************/
int32_t qc_api_wifi_set_max_sp_len(uint32_t maxsp)
{
    uint32_t deviceId = 0, wifimode = 0, dataLen = 0;

    deviceId = qc_api_wifi_get_active_device();
    if (0 != qc_drv_wlan_get_op_mode(qc_api_get_qc_drv_context(),
                deviceId,
                &wifimode,
                &dataLen))
    {
        return QCLI_STATUS_ERROR_E;
    }
    if (QAPI_WLAN_DEV_MODE_STATION_E != wifimode)
    {
        return QCLI_STATUS_ERROR_E;
    }
    if(maxsp < 0 || maxsp > 3)
    {
        return QCLI_STATUS_ERROR_E;
    }
    if (0 != qc_drv_wlan_set_sta_max_sp_len (qc_api_get_qc_drv_context(),
                deviceId,
                (void *)&maxsp,
                sizeof(maxsp),
                FALSE))
    {
        return QCLI_STATUS_ERROR_E;
    }
    return QCLI_STATUS_SUCCESS_E;
}

/**********************************************************************************************
 * Function Name   : qcom_get_wps_net_info()
 * Parames:
 *        void
 * Returned Value  : qapi_WLAN_Netparams_t * - the structure point to netinfo
 *            NULL - on any failure.
 * Comments        :
 *                   Not supported on RTCS
 **********************************************************************************************/
qapi_WLAN_Netparams_t * qc_api_wifi_get_wps_net_info(void)
{
    qapi_WLAN_Netparams_t *pNetparams = &wps_context.netparams;
    if(pNetparams->error)
        return NULL;
    else
        return pNetparams;
}
#endif
/**********************************************************************************************
 * Function Name   : get_country_code()
 * Returned Value  : 0 if success else 1
 * Comments        : Gets the WLAN country code
 **********************************************************************************************/
int32_t qc_api_wifi_get_country_code()
{
#ifdef NEW_FEATURE_INTEGRATION //from 3.3.4
    uint8_t country_code[3] = {'\0'};
    uint32_t i = 0;
    uint32_t deviceId = qc_api_wifi_get_active_device();
    //qcom_get_country_code(deviceId,country_code);
#endif
    return QCLI_STATUS_SUCCESS_E;
}

/**********************************************************************************************
 * Function Name   : set_country_code()
 * Returned Value  : 0 if success else 1
 * Comments        : Sets the WLAN country code
 **********************************************************************************************/
int32_t qc_api_wifi_set_country_code(int8_t *country)
{
    char country_code[3] = {'\0'};

    if (strlen((const char *)country) != 2)
        return QCLI_STATUS_ERROR_E;

    memset(&country_code[0], 0, sizeof(country_code));
    memcpy(country_code, country, strlen((const char *)country));

    qc_drv_wlan_set_country_code(qc_api_get_qc_drv_context(),
            qc_api_wifi_get_active_device(),
            &country_code[0],
            sizeof(country_code),
            FALSE);

    qc_drv_wlan_set_ap_country_code(qc_api_get_qc_drv_context(),
            qc_api_wifi_get_active_device(),
            &country_code[0],
            sizeof(country_code),
            FALSE);
    return QCLI_STATUS_SUCCESS_E;
}

/**********************************************************************************************
 * Function Name   : roam()
 * Returned Value  : 0 if success else -1
 * Comments        : Enable the WLAN roaming mode
 **********************************************************************************************/
 #if 0
int32_t qc_api_wifi_roam(int32_t enable)
{
    uint32_t roamMode = 0;
    if(enable == 1)
    {
        roamMode = 1;//enable roam
    }else if(enable == 0)
    {
        roamMode = 3;//disable roam
    }

    return qc_drv_wlan_set_enable_roaming (qc_api_get_qc_drv_context(),
            qc_api_wifi_get_active_device(),
            &roamMode,
            sizeof(roamMode),
            FALSE);
}

/**********************************************************************************************
 * Function Name  : set_gtx
 * Returned Value : 0
 * Comments       : Enable/Disable GTX
 **********************************************************************************************/
int32_t qc_api_wifi_set_gtx(uint32_t enGtx)
{
    return qc_drv_wlan_set_enable_green_tx (qc_api_get_qc_drv_context(),
            qc_api_wifi_get_active_device(),
            &enGtx,
            sizeof(enGtx),
            FALSE);
}

/**********************************************************************************************
 * Function Name  : set_lpl
 * Returned Value : 0
 * Comments       : Enable/Disable LPL
 **********************************************************************************************/
int32_t qc_api_wifi_set_lpl(uint32_t enLpl)
{
    return qc_drv_wlan_set_low_pw_listen (qc_api_get_qc_drv_context(),
            qc_api_wifi_get_active_device(),
            &enLpl,
            sizeof(enLpl),
            FALSE);
}


/**********************************************************************************************
 * Function Name  : wnm_configurations
 * Returned Value : 0 on succees or QCLI_STATUS_ERROR_E
 **********************************************************************************************/
int32_t qc_api_wifi_wnm_config(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    uint32_t enet_device = qc_api_wifi_get_active_device();
    uint8_t wnm_enable = Parameter_List[0].Integer_Value;
    qc_api_wifi_set_callback(NULL);
    return qc_drv_wlan_set_wmm_config (qc_api_get_qc_drv_context(), enet_device,
            (void *)&wnm_enable,
            sizeof(uint8_t),
            FALSE);
}

/**********************************************************************************************
 * Function Name  : set_ap_bss_max_idle_period
 * Returned Value : 0 on success, else 1
 * Comments       : Set the BSS idle period
 **********************************************************************************************/
int32_t qc_api_wifi_set_ap_bss_max_idle_period(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    uint32_t enet_device = qc_api_wifi_get_active_device();
    uint32_t wifimode = 0, dataLen = 0;

    qc_drv_wlan_get_op_mode(qc_api_get_qc_drv_context(), enet_device,
            &wifimode,
            &dataLen);
    if(wifimode != QAPI_WLAN_DEV_MODE_AP_E)
    {
        QCLI_Printf("Set device Mode to AP.\r\n");
        return QCLI_STATUS_ERROR_E;
    }

    qc_api_wifi_set_callback(NULL);
    qapi_WLAN_BSS_Max_Idle_Period_t period_info;
    period_info.period = Parameter_List[0].Integer_Value;
    period_info.protected_Keep_Alive = 0;
    /* Set max idle period */
    return qc_drv_wlan_set_ap_bss_mip (qc_api_get_qc_drv_context(), enet_device,
            (void *)&period_info,
            sizeof(period_info), FALSE);
}

/**********************************************************************************************
 * Function Name  : set_ap_bss_max_idle_period
 * Returned Value : 0 on success, else 1
 * Comments       : Set the BSS idle period
 **********************************************************************************************/
int32_t qc_api_wifi_set_wnm_sleep_period(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    uint32_t enet_device = qc_api_wifi_get_active_device();
    uint32_t wifimode = 0, dataLen = 0;

    qc_api_wifi_set_callback(NULL);
    qc_drv_wlan_get_op_mode (qc_api_get_qc_drv_context(), enet_device,
            &wifimode, &dataLen);

    if (wifimode != QAPI_WLAN_DEV_MODE_STATION_E) {
        QCLI_Printf("Set device Mode to station.\r\n");
        return QCLI_STATUS_ERROR_E;
    }

    qapi_WLAN_WNM_Sleep_Period_t period_info;
    period_info.action_Type = (uint16_t) Parameter_List[0].Integer_Value;
    period_info.duration = (uint16_t) Parameter_List[1].Integer_Value;

    /* Set sta sleep period */
    return qc_drv_wlan_set_sta_sp (qc_api_get_qc_drv_context(), enet_device,
            (void *)&period_info,
            sizeof(period_info), FALSE);
}

/**********************************************************************************************
 * Function Name  : set_wnm_sleep_response
 * Returned Value : 0 on success, else 1
 * Comments       : Set the WNM sleep mode
 **********************************************************************************************/
int32_t qc_api_wifi_set_wnm_sleep_response(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    uint32_t enet_device = qc_api_wifi_get_active_device();
    uint32_t wifimode = 0, dataLen = 0;

    qc_drv_wlan_get_op_mode(qc_api_get_qc_drv_context(), enet_device,
            &wifimode,
            &dataLen);
    if(wifimode != QAPI_WLAN_DEV_MODE_AP_E)
    {
        QCLI_Printf("Set device Mode to AP.\r\n");
        return QCLI_STATUS_ERROR_E;
    }

    qc_api_wifi_set_callback(NULL);
    uint8_t response = Parameter_List[0].Integer_Value;

    /* Set AP sleep response */
    return qc_drv_wlan_set_ap_sr (qc_api_get_qc_drv_context(), enet_device,
            (void *)&response,
            sizeof(uint8_t),
            FALSE);
}

/**********************************************************************************************
 * Function Name  : set_wnm_sleep_response
 * Returned Value : 0 on success, else 1
 * Comments       : Enable the Wake on WLAN
 **********************************************************************************************/
int32_t qc_api_wifi_enable_wow(uint32_t cmd)
{
    uint32_t enet_device = qc_api_wifi_get_active_device();
    if(qc_drv_wlan_set_enable_wake_wireless (qc_api_get_qc_drv_context(),
                enet_device,
                &cmd,
                sizeof(cmd),
                FALSE) != 0)
    {
        QCLI_Printf("set wow enable error\r\n");
        return QCLI_STATUS_ERROR_E;
    }
    return QCLI_STATUS_SUCCESS_E;
}

/**********************************************************************************************
 * Function Name  : enable_pkt_filter
 * Returned Value : 0 on success, else 1
 * Comments       : Enable the packet filter
 **********************************************************************************************/
int32_t qc_api_wifi_enable_pkt_filter(uint32_t cmd)
{
    uint32_t enet_device = qc_api_wifi_get_active_device();
    if(qc_drv_wlan_set_enable_pkt_filter (qc_api_get_qc_drv_context(),
                enet_device,
                &cmd,
                sizeof(cmd),
                FALSE) != 0)
    {
        QCLI_Printf("set pkt filter enable error\r\n");
        return QCLI_STATUS_ERROR_E;
    }

    return QCLI_STATUS_SUCCESS_E;
}

/**********************************************************************************************
 * Function Name  : add_pattern
 * Returned Value : 0 on success, else 1
 * Comments       : Add the filter pattern
 **********************************************************************************************/
int32_t qc_api_wifi_add_pattern(uint32_t pattern_index, uint8_t action,
        uint8_t wow_filter, uint8_t priority,
        uint16_t header_type, uint32_t offset,
        uint32_t pattern_size, uint8_t *pattern_mask,
        uint8_t *pattern_data)
{
    uint32_t enet_device = qc_api_wifi_get_active_device();
    qapi_WLAN_Add_Pattern_t pattern;

    pattern.pattern_Index = pattern_index;
    pattern.offset = offset;
    pattern.pattern_Size = pattern_size;
    pattern.header_Type = header_type;
    pattern.pattern_Priority = priority;
    pattern.pattern_Action_Flag = action;
    pattern.pattern_Action_Flag |= (wow_filter)? __QAPI_WLAN_PATTERN_WOW_FLAG:0;
    memcpy(pattern.pattern_Mask, pattern_mask, (pattern_size/8 + 1));
    memcpy(pattern.pattern, pattern_data, pattern_size);
    if(qc_drv_wlan_set_add_pattern (qc_api_get_qc_drv_context(),
                enet_device,
                &pattern,
                sizeof(pattern),
                FALSE) != 0)
    {
        QCLI_Printf("set pattern error\r\n");
        return QCLI_STATUS_ERROR_E;
    }
    return QCLI_STATUS_SUCCESS_E;
}

/**********************************************************************************************
 * Function Name  : delete_pattern
 * Returned Value : 0 on success, else 1
 * Comments       : removes the filter pattern
 **********************************************************************************************/
int32_t qc_api_wifi_delete_pattern(uint32_t index, uint32_t header_type)
{

    uint32_t enet_device = qc_api_wifi_get_active_device();
    qapi_WLAN_Delete_Pattern_t pattern;

    pattern.pattern_Index = index;
    pattern.header_Type = header_type;
    if(qc_drv_wlan_set_delete_pattern (qc_api_get_qc_drv_context(),
                enet_device,
                &pattern,
                sizeof(pattern),
                FALSE) != 0)
    {
        QCLI_Printf("set delete pattern error\r\n");
        return QCLI_STATUS_ERROR_E;
    }

    return QCLI_STATUS_SUCCESS_E;
}

/**********************************************************************************************
 * Function Name  : change_default_filter_action
 * Returned Value : 0 on success, else 1
 * Comments       : Change Default packet filter action
 **********************************************************************************************/
int32_t qc_api_wifi_change_default_filter_action(uint32_t action, uint32_t wow_flag,uint32_t header_type)
{
    uint32_t enet_device = qc_api_wifi_get_active_device();
    qapi_WLAN_Change_Default_Filter_Action_t pattern;

    pattern.pattern_Action_Flag= action;
    pattern.pattern_Action_Flag |= (wow_flag)? __QAPI_WLAN_PATTERN_WOW_FLAG:0;
    pattern.header_Type = header_type;
    if(qc_drv_wlan_set_change_def_filter_action (qc_api_get_qc_drv_context(),
                enet_device,
                &pattern,
                sizeof(pattern),
                FALSE) != 0)
    {
        QCLI_Printf("set delete pattern error\r\n");
        return QCLI_STATUS_ERROR_E;
    }

    return QCLI_STATUS_SUCCESS_E;
}

/**********************************************************************************************
 * Function Name  : set_app_ie
 * Returned Value : 0 on success, else 1
 * Comments       : Following function is used to set application specified IE in the given
 *                  frame type. Input is taken as a string and each character is converted
 *                  into its ASCII value (in hex format) before putting it in the frame.
 * Example        : '1' will show up as '0x31' in the frame as ascii of '1' converted to hex is 31
 **********************************************************************************************/
int32_t qc_api_wifi_set_app_ie(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    uint32_t enet_device = qc_api_wifi_get_active_device();
    uint32_t wifimode = 0, dataLen = 0;
    int32_t return_code = 0;
    qapi_WLAN_App_Ie_Params_t ie_params;

    qc_drv_wlan_get_op_mode (qc_api_get_qc_drv_context(),
            enet_device,
            &wifimode, &dataLen);

    ie_params.mgmt_Frame_Type = Parameter_List[0].Integer_Value;

    if ((wifimode == QAPI_WLAN_DEV_MODE_STATION_E) && (ie_params.mgmt_Frame_Type != QAPI_WLAN_FRAME_PROBE_REQ_E))
    {
        QCLI_Printf("In station mode, application specified information element can be added only in probe request frames\r\n");
        return QCLI_STATUS_ERROR_E;
    }

    if ((wifimode == QAPI_WLAN_DEV_MODE_AP_E) &&
            ((ie_params.mgmt_Frame_Type != QAPI_WLAN_FRAME_BEACON_E) &&
             (ie_params.mgmt_Frame_Type != QAPI_WLAN_FRAME_PROBE_RESP_E)))
    {
        QCLI_Printf("In soft-AP mode, application specified information element can be added only in beacon and probe response frames\r\n");
        return QCLI_STATUS_ERROR_E;
    }

    ie_params.ie_Len = strlen((const char *)Parameter_List[1].String_Value);
    if (ie_params.ie_Len < 2)
    {
        QCLI_Printf("Invalid application specified information element length. Application specified information element must start with 'dd'\r\n");
        return QCLI_STATUS_ERROR_E;
    }

    if ((strncmp((char *)(Parameter_List[1].String_Value), "dd",2) != 0))
    {
        QCLI_Printf("Application specified information element must start with 'dd'\r\n");
        return QCLI_STATUS_ERROR_E;
    }

    ie_params.ie_Info = (uint8_t *)malloc(ie_params.ie_Len + 1);
    strncpy((char *)ie_params.ie_Info, (char *)Parameter_List[1].String_Value, ie_params.ie_Len);
    ie_params.ie_Info[ie_params.ie_Len] = '\0';
    return_code = qc_drv_wlan_set_app_ie (qc_api_get_qc_drv_context(), enet_device,
            (void *)&ie_params,
            sizeof(qapi_WLAN_App_Ie_Params_t),
            FALSE);
    free(ie_params.ie_Info);

    return return_code;
}

/**********************************************************************************************
 * Function Name  : set_sta_bmiss_config
 * Returned Value : 0 on success, else 1
 * Comments       : Set the Beacon Miss Timeout Config Value, one of the value is set which
 *                  needs to be configured and the other set to 0.
 **********************************************************************************************/
int32_t qc_api_wifi_set_sta_bmiss_config(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{

    qapi_WLAN_Sta_Config_Bmiss_Config_t bmiss_config;
    uint32_t deviceId = qc_api_wifi_get_active_device();

    memset(&bmiss_config, 0, sizeof(bmiss_config));

    bmiss_config.bmiss_Time_In_Ms = Parameter_List[0].Integer_Value;
    bmiss_config.num_Beacons = Parameter_List[1].Integer_Value;

    /* Beacon miss timeout allowed is between 1sec to 10sec*/
    if (bmiss_config.num_Beacons == 0 && (bmiss_config.bmiss_Time_In_Ms < 1000 ||
                bmiss_config.bmiss_Time_In_Ms > 10000))
    {
        QCLI_Printf("ERROR: bmiss_Time_In_Ms out of range, allowed 1000 to 10000\n");
        return QCLI_STATUS_ERROR_E;
    }

    /* Assuming 100TU as Beacon interval the accepted range for
       number of beacon miss allowed is between 10 to 100*/
    if (bmiss_config.bmiss_Time_In_Ms ==0 && (bmiss_config.num_Beacons < 10 ||
                bmiss_config.num_Beacons > 100))
    {
        QCLI_Printf("ERROR: num_Beacons out of range, allowed 10 to 100\n");
        return QCLI_STATUS_ERROR_E;
    }

    if( 0 != qc_drv_wlan_set_sta_bmiss_config (qc_api_get_qc_drv_context(),
                deviceId,
                (void *)&bmiss_config,
                sizeof(bmiss_config),
                FALSE))
    {
        QCLI_Printf("ERROR: set_sta_bmiss_config\n");
        return QCLI_STATUS_ERROR_E;
    }
    return QCLI_STATUS_SUCCESS_E;
}
#endif

/**********************************************************************************************
 * Function Name   : wep_connect()
 * Returned Value  : 0 - successful completion or
 *                  -1 - failed.
 * Comments    : Connect to AP with WEP.
 **********************************************************************************************/
int32_t qc_api_wifi_wep_connect(QCLI_Parameter_t *Parameter_List)
{
    int32_t  error= 0, i = 0, j = 0;
    uint32_t deviceId = 0;
    uint8_t  val = 0;
    int8_t   set_key[__QAPI_WLAN_MAX_WEP_KEY_SZ + 1] = {'\0'};
    char* ssid = Parameter_List[0].String_Value;
    char* key_val = Parameter_List[2].String_Value;
    int32_t key_idx = Parameter_List[1].Integer_Value;
    qapi_WLAN_Security_Wep_Key_Pair_Params_t keyPair;

    if(strlen(key_val) != QC_API_MIN_HEX_WEP_KEY_SIZE && strlen(key_val) != QC_API_MAX_HEX_WEP_KEY_SIZE &&
            strlen(key_val) != (QC_API_MIN_HEX_WEP_KEY_SIZE/2) && strlen(key_val) != (QC_API_MAX_HEX_WEP_KEY_SIZE/2))
    {
        LOG_ERR("Invalid WEP Key length, only 10 or 26 HEX characters allowed (or) 5 or 13 ascii keys allowed\r\n\r\n");
        return QCLI_STATUS_ERROR_E;
    }

    if (key_idx < 1 || key_idx > QC_API_MAX_NUM_WEP_KEYS)
    {
        LOG_ERR("Invalid key index, Please enter between 1-4\r\n\r\n");
        return QCLI_STATUS_ERROR_E;
    }

    if((strlen(key_val) == QC_API_MIN_HEX_WEP_KEY_SIZE) || (strlen(key_val) == QC_API_MAX_HEX_WEP_KEY_SIZE))
    {
        for (i = 0; i < strlen(key_val); i++)
        {
            if(ishexdigit(key_val[i]))
            {
                continue;
            }
            else
            {
                LOG_ERR("for hex enter [0-9] or [A-F]\r\n\r\n");
                return QCLI_STATUS_ERROR_E;
            }
        }
    }

    deviceId = qc_api_wifi_get_active_device();
    //Pass all the keys to the driver
    for(i = 1; i <= QC_API_MAX_NUM_WEP_KEYS; i++)
    {
        /* convert key data from string to bytes */
        memset(set_key, 0, __QAPI_WLAN_MAX_WEP_KEY_SZ + 1);
        if((strlen(key_val) == QC_API_MIN_HEX_WEP_KEY_SIZE) || (strlen(key_val) == QC_API_MAX_HEX_WEP_KEY_SIZE))
        {
            key_type |= (1 << (i - 1));
            for (j = 0; j < strlen(key_val); j++)
            {
                val = ascii_to_hex((uint8_t)key_val[j]);
                if(0xff == val)
                {
                    LOG_ERR("for hex enter [0-9] or [A-F]\r\n\r\n");
                    return QCLI_STATUS_ERROR_E;
                }
                if((j & 1) == 0)
                {
                    val <<= 4;
                }
                set_key[j >> 1] |= val;
            }
            keyPair.key_Length = strlen(key_val) / 2;
        }
        else
        {
            key_type &= ~(1 << (i - 1));
            memcpy(set_key, (int8_t *)(key_val), strlen(key_val));
            keyPair.key_Length = strlen(key_val);
        }

        keyPair.key = set_key;
        keyPair.key_Index = i;

        error = qc_drv_wlan_set_wep_key_pair(qc_api_get_qc_drv_context(),
                deviceId,
                (void *) &keyPair, sizeof(keyPair), FALSE);
        if(error != 0)
        {
            return error;
        }
    }

    error = qc_drv_wlan_set_wep_key_index(qc_api_get_qc_drv_context(),
            deviceId,
            (void *) &key_idx,
            sizeof(key_idx),
            FALSE);

    //Disable WSC if the APUT is manually configured for WEP.
    if(wps_flag == 1)
    {
        wps_should_disable = 1;
        wps_flag = 0;
    }
    security_mode = QC_API_SEC_MODE_WEP;

    if (0 == qc_api_wifi_connect_to_network((const int8_t*)ssid)) {
        return QCLI_STATUS_SUCCESS_E;
    }
    return error;
}

/**********************************************************************************************
 * Function Name   : wpa_connect()
 * Returned Value  : 0 - successful completion or
 *            -1 - failed.
 * Comments    : Connect to AP with WPA.
 **********************************************************************************************/
int32_t qc_api_wifi_wpa_connect(QCLI_Parameter_t *Parameter_List)
{
    char *ssid, *passphrase;
    uint8_t deviceId = 0;
    uint32_t i = 0;

    ssid = Parameter_List[0].String_Value;
    passphrase = Parameter_List[4].String_Value;

    /********** wpa version ****************/
    if (Parameter_List[1].Integer_Value == 1) {
        wpa_ver = QAPI_WLAN_AUTH_WPA_PSK_E;
    } else if (Parameter_List[1].Integer_Value == 2) {
        wpa_ver = QAPI_WLAN_AUTH_WPA2_PSK_E;
    } else {
        LOG_ERR("Invalid version\r\n");
        return QCLI_STATUS_ERROR_E;
    }

    /**************** cipher **********/
    if (Parameter_List[2].Integer_Value == Parameter_List[3].Integer_Value) {
        if (Parameter_List[2].Integer_Value == 1) {
            cipher = QAPI_WLAN_CRYPT_TKIP_CRYPT_E;
        } else if (Parameter_List[2].Integer_Value == 2) {
            cipher = QAPI_WLAN_CRYPT_AES_CRYPT_E;
        }else {
            LOG_ERR("invaid uchipher mcipher, should be TKIP or CCMP\r\n");
            return QCLI_STATUS_ERROR_E;
        }
    } else {
        LOG_ERR("invaid uchipher mcipher, should be same\r\n");
        return QCLI_STATUS_ERROR_E;
    }
    //Disable WSC if the APUT is manually configured for WPA/TKIP only.
    if((wps_flag == 1) && (wpa_ver == QAPI_WLAN_AUTH_WPA_PSK_E) && (cipher == QAPI_WLAN_CRYPT_TKIP_CRYPT_E))
    {
        wps_should_disable = 1;
        wps_flag = 0;
    }

    security_mode = QC_API_SEC_MODE_WPA;

    deviceId = qc_api_wifi_get_active_device();

    if((strlen(passphrase) >= 8) && (strlen(passphrase) <=64))
    {
        if(strlen(passphrase) == 64)
        {
            for (i = 0; i < strlen(passphrase); i++)
            {
                if(ishexdigit(passphrase[i]))
                {
                    continue;
                }
                else
                {
                    LOG_ERR("passphrase in hex, please enter [0-9] or [A-F]\r\n\r\n");
                    return QCLI_STATUS_ERROR_E;
                }
            }
        }
        memset(wpa_passphrase[deviceId], 0, (__QAPI_WLAN_PASSPHRASE_LEN + 1));
        strcpy((char *)wpa_passphrase[deviceId], passphrase);
    }
    else
    {
        LOG_ERR("Wrong passphrase length, the length should be between 8 and 64. \r\n\r\n");
        return QCLI_STATUS_ERROR_E;
    }

    if (0 == qc_api_wifi_connect_to_network((const int8_t*)ssid)) {
        return QCLI_STATUS_SUCCESS_E;
    }

    return QCLI_STATUS_ERROR_E;
}

/**********************************************************************************************
 * Function Name   : wps_connect()
 * Returned Value  : 0 - successful completion or
 *                  -1 - failed.
 * Comments        : Connect to AP with WPS.
 **********************************************************************************************/
int32_t qc_api_wifi_wps_connect(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int32_t  j = 0;
    uint8_t val = 0, wps_mode = 0, index = 0;
    uint32_t wifi_mode = 0, data_len = 0, error = 0, enet_device = 0;
    char wpsPin[QC_API_MAX_WPS_PIN_SIZE], *result;
    char delimter[] = "-#";
    qapi_WLAN_WPS_Credentials_t wpsScan, *wpsScan_p = NULL;

    memset(wpsPin, 0, QC_API_MAX_WPS_PIN_SIZE);
    /* Push Method*/
    if (Parameter_List[1].Integer_Value == 0)
    {
        wps_mode = QAPI_WLAN_WPS_PBC_MODE_E;
        index = 0;
    }
    else
    { /* Pin Method */
        wps_mode = QAPI_WLAN_WPS_PIN_MODE_E;
        index = 1;
        result = strtok( (char *) Parameter_List[2].String_Value, delimter);
        if(result == NULL)
        {
            strcpy(wpsPin, (const char *) Parameter_List[2].String_Value);
        }
        else
        {
            while( result != NULL )
            {
                strcat(wpsPin, (char *)result);
                result = strtok( NULL, (char *)delimter );
            }
        }
    }

    qc_api_wifi_set_callback(NULL);
    enet_device = qc_api_wifi_get_active_device();

    error = qc_drv_wlan_get_op_mode(qc_api_get_qc_drv_context(), enet_device,
            &wifi_mode,
            &data_len);

    if (error != 0)
    {
        LOG_ERR("WPS failed\r\n");
        return QCLI_STATUS_ERROR_E;
    }

    /* Initialize context */
    wps_context.wps_in_progress = 0;
    /* Connect flag */
    wps_context.connect_flag = Parameter_List[0].Integer_Value;

    if (Parameter_Count > (index + 2))
    {
        LOG_DEBUG("if para cnt is > 3\n");
        /* SSID */
        if (strlen(Parameter_List[index + 2].String_Value) > __QAPI_WLAN_MAX_SSID_LEN)
        {
            LOG_ERR("Invalid SSID length\r\n");
            return QCLI_STATUS_ERROR_E;
        }
        memset(wpsScan.ssid, 0, __QAPI_WLAN_MAX_SSID_LEN);
        strncpy((char *)(wpsScan.ssid), Parameter_List[index + 2].String_Value, strlen(Parameter_List[index + 2].String_Value));
        wpsScan.ssid_Length = strlen(Parameter_List[index + 2].String_Value);

        /* MAC address */
        if(strlen((const char *) Parameter_List[index + 3].String_Value) != 12)
        {
            LOG_ERR("Invalid MAC address length\r\n");
            return QCLI_STATUS_ERROR_E;
        }
        memset(wpsScan.mac_Addr, 0, __QAPI_WLAN_MAC_LEN);
        for(j = 0; j < strlen((const char *) Parameter_List[index + 3].String_Value); j++)
        {
            val = ascii_to_hex(Parameter_List[index + 3].String_Value[j]);
            if(val == 0xff)
            {
                LOG_ERR("Invalid character\r\n");
                return QCLI_STATUS_ERROR_E;
            }
            else
            {
                if((j & 1) == 0)
                {
                    val <<= 4;
                }
                wpsScan.mac_Addr[j >> 1] |= val;
            }
        }

        /* Wireless channel */
        wpsScan.ap_Channel = channel_array[Parameter_List[index + 4].Integer_Value];
        wpsScan_p = &wpsScan;
    }

    qc_api_wifi_set_power_mode( QAPI_WLAN_POWER_MODE_MAX_PERF_E, QAPI_WLAN_POWER_MODULE_WPS_E);

    if (0 != qc_drv_wlan_set_wps_credentials(qc_api_get_qc_drv_context(), enet_device,
                wpsScan_p,
                sizeof(qapi_WLAN_WPS_Credentials_t),
                FALSE))
    {
        LOG_ERR("WPS failed\r\n");
        return QCLI_STATUS_ERROR_E;
    }

    if(qc_drv_wlan_wps_start(qc_api_get_qc_drv_context(), enet_device, wps_context.connect_flag, wps_mode, (const char *)wpsPin) != 0)
    {
        LOG_ERR("WPS failed\r\n");
        qc_api_wifi_set_power_mode( QAPI_WLAN_POWER_MODE_REC_POWER_E, QAPI_WLAN_POWER_MODULE_WPS_E);
        return QCLI_STATUS_ERROR_E;
    }

    wps_context.wps_in_progress = 1;

    return QCLI_STATUS_SUCCESS_E;
}


/************************************************p2p**********************************************************************************/
#if ENABLE_P2P_MODE
void app_p2p_process_persistent_list_event(uint8_t *pData)
{
    uint8_t *local_ptr = NULL;
    uint32_t loop_index = 0;

    if(!pData)
    {
        return;
    }
    local_ptr = pData;

    memset(p2p_peers_data, 0,
            (__QAPI_WLAN_P2P_MAX_LIST_COUNT * sizeof(qapi_WLAN_P2P_Persistent_Mac_List_t)));

    memcpy(p2p_peers_data, local_ptr,
            (__QAPI_WLAN_P2P_MAX_LIST_COUNT * sizeof(qapi_WLAN_P2P_Persistent_Mac_List_t)));

    QCLI_Printf("\r\n");
    if(p2p_cancel_enable == 0)
    {
        for(loop_index = 0; loop_index < __QAPI_WLAN_P2P_MAX_LIST_COUNT; loop_index++)
        {
            LOG_DEBUG("mac_addr[%d] : %02x:%02x:%02x:%02x:%02x:%02x\r\n", loop_index,
                    ((qapi_WLAN_P2P_Persistent_Mac_List_t *)(local_ptr))->macaddr[0],
                    ((qapi_WLAN_P2P_Persistent_Mac_List_t *)(local_ptr))->macaddr[1],
                    ((qapi_WLAN_P2P_Persistent_Mac_List_t *)(local_ptr))->macaddr[2],
                    ((qapi_WLAN_P2P_Persistent_Mac_List_t *)(local_ptr))->macaddr[3],
                    ((qapi_WLAN_P2P_Persistent_Mac_List_t *)(local_ptr))->macaddr[4],
                    ((qapi_WLAN_P2P_Persistent_Mac_List_t *)(local_ptr))->macaddr[5]);

            LOG_DEBUG("ssid[%d] : %s\r\n", loop_index,
                    ((qapi_WLAN_P2P_Persistent_Mac_List_t *)(local_ptr))->ssid);

            if(((qapi_WLAN_P2P_Persistent_Mac_List_t *)(local_ptr))->role ==
                    QAPI_WLAN_P2P_INV_ROLE_ACTIVE_GO_E)
            {
                LOG_DEBUG("passphrase[%d] : %s\r\n", loop_index,
                        ((qapi_WLAN_P2P_Persistent_Mac_List_t *)(local_ptr))->passphrase);
            }
            local_ptr += sizeof(qapi_WLAN_P2P_Persistent_Mac_List_t);
        }
    }
    else
    {
        p2p_cancel_enable = 0;
    }
    return;
}

void app_p2p_process_node_list_event(uint8_t *pData)
{
    uint8_t *local_ptr = NULL, *temp_ptr = NULL;
    uint8_t index = 0, temp_val = 0;
    qapi_WLAN_P2P_Set_Cmd_t p2p_set_params;
    uint32_t deviceId = 0, temp_device_id = 0;

    if(!pData)
    {
        return;
    }

    local_ptr = pData;
    temp_val = *local_ptr;
    local_ptr++;

    temp_ptr = local_ptr;
    p2p_session_in_progress = 1;

    deviceId = qc_api_wifi_get_active_device();
    if(deviceId != 0)
    {
        /* P2P device should always send/receive events/commands on dev 0 if
           the app has switched to dev 1 while event is in dev 0 send command
           via dev 0 and then switch to dev 1*/
        temp_device_id = deviceId;
        deviceId = 0;
        qc_api_wifi_set_active_deviceid(deviceId);
    }
    if (temp_val > 0)
    {
        for (index = 0; index < temp_val; index++)
        {
            if(p2p_join_session_active)
            {
                if(memcmp(((qapi_WLAN_P2P_Device_Lite_t *)(local_ptr))->interface_Addr,
                            p2p_join_mac_addr, __QAPI_WLAN_MAC_LEN) == 0)
                {
                    p2p_join_profile.go_Oper_Freq = ((qapi_WLAN_P2P_Device_Lite_t*)(local_ptr))->oper_Freq;
                    break;
                }
            }
            else
            {
                LOG_AT("\r\n");
                LOG_AT("\t p2p_config_method     : %x \r\n",
                        (((qapi_WLAN_P2P_Device_Lite_t *)(local_ptr))->config_Methods));

                LOG_AT("\t p2p_device_name       : %s \r\n",
                        ((qapi_WLAN_P2P_Device_Lite_t *)(local_ptr))->device_Name);

                LOG_AT("\t p2p_primary_dev_type  : %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\r\n ",
                        ((qapi_WLAN_P2P_Device_Lite_t *)(local_ptr))->pri_Dev_Type[0],
                        ((qapi_WLAN_P2P_Device_Lite_t *)(local_ptr))->pri_Dev_Type[1],
                        ((qapi_WLAN_P2P_Device_Lite_t *)(local_ptr))->pri_Dev_Type[2],
                        ((qapi_WLAN_P2P_Device_Lite_t *)(local_ptr))->pri_Dev_Type[3],
                        ((qapi_WLAN_P2P_Device_Lite_t *)(local_ptr))->pri_Dev_Type[4],
                        ((qapi_WLAN_P2P_Device_Lite_t *)(local_ptr))->pri_Dev_Type[5],
                        ((qapi_WLAN_P2P_Device_Lite_t *)(local_ptr))->pri_Dev_Type[6],
                        ((qapi_WLAN_P2P_Device_Lite_t *)(local_ptr))->pri_Dev_Type[7]);

                LOG_AT("\t p2p_interface_addr    : %02x:%02x:%02x:%02x:%02x:%02x \r\n",
                        ((qapi_WLAN_P2P_Device_Lite_t *)(local_ptr))->interface_Addr[0],
                        ((qapi_WLAN_P2P_Device_Lite_t *)(local_ptr))->interface_Addr[1],
                        ((qapi_WLAN_P2P_Device_Lite_t *)(local_ptr))->interface_Addr[2],
                        ((qapi_WLAN_P2P_Device_Lite_t *)(local_ptr))->interface_Addr[3],
                        ((qapi_WLAN_P2P_Device_Lite_t *)(local_ptr))->interface_Addr[4],
                        ((qapi_WLAN_P2P_Device_Lite_t *)(local_ptr))->interface_Addr[5]);

                LOG_AT("\t p2p_device_addr       : %02x:%02x:%02x:%02x:%02x:%02x \r\n",
                        ((qapi_WLAN_P2P_Device_Lite_t *)(local_ptr))->p2p_Device_Addr[0],
                        ((qapi_WLAN_P2P_Device_Lite_t *)(local_ptr))->p2p_Device_Addr[1],
                        ((qapi_WLAN_P2P_Device_Lite_t *)(local_ptr))->p2p_Device_Addr[2],
                        ((qapi_WLAN_P2P_Device_Lite_t *)(local_ptr))->p2p_Device_Addr[3],
                        ((qapi_WLAN_P2P_Device_Lite_t *)(local_ptr))->p2p_Device_Addr[4],
                        ((qapi_WLAN_P2P_Device_Lite_t *)(local_ptr))->p2p_Device_Addr[5]);

                LOG_AT("\t p2p_device_capability : %x \r\n",
                        ((qapi_WLAN_P2P_Device_Lite_t *)(local_ptr))->dev_Capab);

                LOG_AT("\t p2p_group_capability  : %x \r\n",
                        ((qapi_WLAN_P2P_Device_Lite_t *)(local_ptr))->group_Capab);

                LOG_AT("\t p2p_wps_method        : %x \r\n",
                        ((qapi_WLAN_P2P_Device_Lite_t *)(local_ptr))->wps_Method);

                LOG_AT("\t Peer Oper channel     : %d \r\n",
                        ((qapi_WLAN_P2P_Device_Lite_t *)(local_ptr))->oper_Freq);
            }

            local_ptr += sizeof(qapi_WLAN_P2P_Device_Lite_t);
        }

        if(p2p_join_session_active)
        {
            p2p_join_session_active = 0;
            p2p_set_params.val.mode.p2pmode = __QAPI_WLAN_P2P_CLIENT;

            if (0 != qc_drv_p2p_set_param(qc_api_get_qc_drv_context(),
                        deviceId,
                        &p2p_set_params.val.mode.p2pmode,
                        sizeof(p2p_set_params.val.mode.p2pmode),
                        FALSE))
            {
                LOG_WARN("StartP2P JOIN SET command did not execute properly\r\n");
                goto set_temp_device;
            }

            if(     qc_drv_p2p_join(qc_api_get_qc_drv_context(),
                        deviceId, p2p_join_profile.wps_Method,
                        &p2p_join_mac_addr[0], p2p_wps_pin,
                        p2p_join_profile.go_Oper_Freq) != 0)
            {
                LOG_WARN("P2P JOIN command did not execute properly\r\n");
#if ENABLE_SCC_MODE
                LOG_WARN("support single concurrent channel only....\r\n");
#endif /* ENABLE_SCC_MODE */
                goto set_temp_device;
            }
        }
    }
    local_ptr = temp_ptr;

set_temp_device:
    if(temp_device_id != 0)
    {
        qc_api_wifi_set_active_deviceid(temp_device_id);
    }
    p2p_session_in_progress = 0;
    return;
}

void P2P_Event_Handler_Prov_Disc_Req(CMD_P2P_EVENT_INFO *pEventInfo)
{
    qapi_WLAN_P2P_Prov_Disc_Req_Event_t *local_ptr = NULL;
    uint16_t wps_method = 0;

    local_ptr = (qapi_WLAN_P2P_Prov_Disc_Req_Event_t *) pEventInfo->pBuffer;
    wps_method = local_ptr->wps_Config_Method;
    LOG_DEBUG("\r\n source addr : %02x:%02x:%02x:%02x:%02x:%02x \r\n", local_ptr->sa[0], local_ptr->sa[1],
            local_ptr->sa[2], local_ptr->sa[3],
            local_ptr->sa[4], local_ptr->sa[5]);

    QCLI_Printf("\r\n wps_config_method : %x \r\n", local_ptr->wps_Config_Method);

    if(__QAPI_WLAN_P2P_WPS_CONFIG_DISPLAY == wps_method)
    {
        LOG_DEBUG("Provisional Disc Request - Display WPS PIN [%s] \r\n",p2p_wps_pin);
    }
    else if(__QAPI_WLAN_P2P_WPS_CONFIG_KEYPAD == wps_method)
    {
        LOG_DEBUG("Provisional Disc Request - Enter WPS PIN \r\n");
    }
    else if(__QAPI_WLAN_P2P_WPS_CONFIG_PUSHBUTTON == wps_method)
    {
        LOG_DEBUG("Provisional Disc Request - Push Button \r\n");
    }
    else
    {
        LOG_DEBUG("Invalid Provisional Request \r\n");
    }
    return;
}

void P2P_Event_Handler_Prov_Disc_Resp(CMD_P2P_EVENT_INFO *pEventInfo)
{
    qapi_WLAN_P2P_Prov_Disc_Resp_Event_t *local_ptr = NULL;

    local_ptr = (qapi_WLAN_P2P_Prov_Disc_Resp_Event_t *) pEventInfo->pBuffer;
    LOG_DEBUG("\r\n peer addr : %02x:%02x:%02x:%02x:%02x:%02x \r\n",
            local_ptr->peer[0], local_ptr->peer[1], local_ptr->peer[2],
            local_ptr->peer[3], local_ptr->peer[4], local_ptr->peer[5]);

    if(__QAPI_WLAN_P2P_WPS_CONFIG_KEYPAD == local_ptr->config_Methods)
    {
        LOG_DEBUG("Provisional Disc Response Keypad - WPS PIN [%s] \r\n",p2p_wps_pin);
    }
    else if(__QAPI_WLAN_P2P_WPS_CONFIG_DISPLAY == local_ptr->config_Methods)
    {
        LOG_DEBUG("Provisional Disc Response Display \r\n");
    }
    else if(__QAPI_WLAN_P2P_WPS_CONFIG_PUSHBUTTON == local_ptr->config_Methods)
    {
        LOG_DEBUG("Provisional Disc Response Push Button.\r\n");
    }
    else
    {
        LOG_WARN("Invalid Provisional Response.\r\n");
    }
    return;
}

void P2P_Event_Handler_Req_To_Auth(CMD_P2P_EVENT_INFO *pEventInfo)
{
    qapi_WLAN_P2P_Req_To_Auth_Event_t *local_ptr = (qapi_WLAN_P2P_Req_To_Auth_Event_t *) pEventInfo->pBuffer;

    LOG_DEBUG("source addr : %02x:%02x:%02x:%02x:%02x:%02x \r\n", local_ptr->sa[0], local_ptr->sa[1],
            local_ptr->sa[2], local_ptr->sa[3], local_ptr->sa[4], local_ptr->sa[5]);

    LOG_DEBUG("dev_password_id : %x \r\n", local_ptr->dev_Password_Id);
    QCLI_Display_Prompt();
    return;
}

void P2P_Event_Handler_Sdpd_Rx(CMD_P2P_EVENT_INFO *pEventInfo)
{
    qapi_WLAN_P2P_Sdpd_Rx_Event_t *local_ptr = (qapi_WLAN_P2P_Sdpd_Rx_Event_t *) pEventInfo->pBuffer;

    LOG_DEBUG("Custom_Api_p2p_serv_disc_req event \r\n");
    LOG_DEBUG("type : %d   frag id : %x \r\n", local_ptr->type, local_ptr->frag_Id);
    LOG_DEBUG("transaction_status : %x \r\n", local_ptr->transaction_Status);
    LOG_DEBUG("freq : %d status_code : %d comeback_delay : %d tlv_length : %d update_indic : %d \r\n",
            local_ptr->freq, local_ptr->status_Code, local_ptr->comeback_Delay, local_ptr->tlv_Length, local_ptr->update_Indic);

    LOG_DEBUG("source addr : %02x:%02x:%02x:%02x:%02x:%02x \r\n",
            local_ptr->peer_Addr[0], local_ptr->peer_Addr[1],
            local_ptr->peer_Addr[2], local_ptr->peer_Addr[3],
            local_ptr->peer_Addr[4], local_ptr->peer_Addr[5]);

    return;
}

void P2P_Event_Handler_Invite_Sent_Result(CMD_P2P_EVENT_INFO *pEventInfo)
{
    qapi_WLAN_P2P_Invite_Sent_Result_Event_t *local_ptr = NULL;
    uint32_t deviceId = 0, temp_device_id = 0, dataLen = 0;
    int32_t channel = __QAPI_WLAN_P2P_AUTO_CHANNEL;

    local_ptr = (qapi_WLAN_P2P_Invite_Sent_Result_Event_t *) pEventInfo->pBuffer;
    LOG_DEBUG("Invitation Result %d\r\n", local_ptr->status);

    if(local_ptr->status == 0)
    {
        LOG_DEBUG("SSID %02x:%02x:%02x:%02x:%02x:%02x \r\n", local_ptr->bssid[0], local_ptr->bssid[1],
                local_ptr->bssid[2], local_ptr->bssid[3],
                local_ptr->bssid[4], local_ptr->bssid[5]);
    }

    if((p2p_peers_data[invitation_index].role == QAPI_WLAN_P2P_INV_ROLE_ACTIVE_GO_E) && (p2p_persistent_done == 0) && (local_ptr->status == 0))
    {
        p2p_session_in_progress = 1;
        deviceId = qc_api_wifi_get_active_device();
        if(deviceId != 0)
        {
            /* P2P device should always send/receive events/commands on dev 0 if
             * the app has switched to dev 1 while event is in dev 0 send command
             * via dev 0 and then switch to dev 1 */
            temp_device_id = deviceId;
            /*
             * If the device 1 is connected, we start GO on the home channel of device 1.
             */
            if(temp_device_id ==1 && wifi_state[temp_device_id] ==1)
            {
                qc_drv_wlan_set_channel(qc_api_get_qc_drv_context(), temp_device_id,
                        (uint32_t *)&channel,
                        dataLen,
                        FALSE);
            }
            deviceId = 0;
            qc_api_wifi_set_active_deviceid(deviceId);
        }
        else
        {
            temp_device_id = deviceId;
            deviceId = 1;
            qc_api_wifi_set_active_deviceid(deviceId);
            /*
             * If the device 1 is connected, we start GO on the home channel of device 1.
             */
            if(wifi_state[deviceId] ==1)
            {
                qc_drv_wlan_get_channel (qc_api_get_qc_drv_context(),
                        deviceId,
                        (uint16_t *)&channel,
                        &dataLen);
            }
            deviceId = temp_device_id;
            qc_api_wifi_set_active_deviceid(temp_device_id);
        }
        wps_flag = 0x01;

        LOG_DEBUG("Starting Autonomous GO \r\n");
        if(qc_drv_p2p_start_go(qc_api_get_qc_drv_context(),
                    deviceId, NULL, channel, 1) != 0)
        {
            LOG_WARN("StartP2P command did not execute properly\r\n");
            goto set_temp_device;
        }

        p2p_persistent_done = 1;

set_temp_device:
        if(temp_device_id != 0){
            qc_api_wifi_set_active_deviceid(temp_device_id);
        }
        p2p_session_in_progress = 0;
    }
    return;
}

void P2P_Event_Handler_Invite_Rcvd_Result(CMD_P2P_EVENT_INFO *pEventInfo)
{
    qapi_WLAN_P2P_Invite_Rcvd_Result_Event_t *local_ptr = NULL;
    qapi_WLAN_P2P_Go_Params_t goParams;
    uint32_t deviceId = 0, temp_device_id = 0, dataLen = 0;
    int32_t channel = __QAPI_WLAN_P2P_AUTO_CHANNEL;
    int i = 0;

    deviceId = qc_api_wifi_get_active_device();
    if(deviceId != 0)
    {
        /* P2P device should always send/receive events/commands on dev 0 if
         * the app has switched to dev 1 while event is in dev 0 send command
         * via dev 0 and then switch to dev 1 */
        temp_device_id = deviceId;
        /*
         * If the device 1 is connected, we start GO on the home channel of device 1.
         */
        if(temp_device_id ==1 && wifi_state[temp_device_id] ==1)
        {
            qc_drv_wlan_get_channel (qc_api_get_qc_drv_context(), temp_device_id,
                    (uint16_t *)&channel,
                    &dataLen);
        }
        deviceId = 0;
        qc_api_wifi_set_active_deviceid(deviceId);
    }
    else
    {
        temp_device_id = deviceId;
        deviceId = 1;
        qc_api_wifi_set_active_deviceid(deviceId);
        /*
         * If the device 1 is connected, we start GO on the home channel of device 1.
         */
        if(wifi_state[deviceId] ==1)
        {
            qc_drv_wlan_get_channel (qc_api_get_qc_drv_context(),
                    deviceId,
                    (uint16_t *)&channel,
                    &dataLen);
        }
        deviceId = temp_device_id;
        qc_api_wifi_set_active_deviceid(temp_device_id);
    }

    local_ptr = (qapi_WLAN_P2P_Invite_Rcvd_Result_Event_t *) pEventInfo->pBuffer;
    memset(&goParams,0, sizeof(qapi_WLAN_P2P_Go_Params_t));
    LOG_DEBUG("Invite Result Status : %x \r\n", local_ptr->status);

    if (local_ptr->status == 0)
    {
        for (i=0;i<__QAPI_WLAN_MAC_LEN;i++)
        {
            LOG_DEBUG(" [%x] ", local_ptr->sa[i]);
        }
    }
    else
    {
        qc_drv_p2p_stop_find(qc_api_get_qc_drv_context(), deviceId);
    }
    LOG_AT("\r\n");

    if((p2p_peers_data[inv_response_evt_index].role == QAPI_WLAN_P2P_INV_ROLE_ACTIVE_GO_E) && (p2p_persistent_done == 0) &&
            (local_ptr->status == 0))
    {
        // make AP Mode and WPS default settings for P2P GO
        p2p_session_in_progress = 1;

        wps_flag = 0x01;
        LOG_DEBUG("Starting Autonomous GO \r\n");

        goParams.ssid_Len = strlen((char *) p2p_peers_data[inv_response_evt_index].ssid);
        goParams.passphrase_Len = strlen((char *) p2p_peers_data[inv_response_evt_index].passphrase);
        memcpy(goParams.ssid, p2p_peers_data[inv_response_evt_index].ssid,
                goParams.ssid_Len);
        memcpy(goParams.passphrase, p2p_peers_data[inv_response_evt_index].passphrase,
                goParams.passphrase_Len);

        if(qc_drv_p2p_start_go(qc_api_get_qc_drv_context(),
                    deviceId, &goParams, channel, 1) != 0)
        {
            LOG_WARN("StartP2P command did not execute properly\r\n");
            goto set_temp_device;
        }

        p2p_persistent_done = 1;
        inv_response_evt_index = 0;
    }

set_temp_device:
    if(temp_device_id != 0)
    {
        qc_api_wifi_set_active_deviceid(temp_device_id);
    }
    p2p_session_in_progress = 0;
    return;
}

void P2P_Event_Handler_Invite_Req(CMD_P2P_EVENT_INFO *pEventInfo)
{
    qapi_WLAN_P2P_Invite_Req_Event_t *local_ptr = (qapi_WLAN_P2P_Invite_Req_Event_t *) pEventInfo->pBuffer;
    qapi_WLAN_P2P_Invite_Info_t invite_rsp_cmd;
    uint32_t deviceId = 0, temp_device_id = 0;
    int i = 0;

    LOG_DEBUG("Invitation Req Received From : ");
    for (i = 0; i < __QAPI_WLAN_MAC_LEN; i++)
    {
        QCLI_Printf(" %x: ",local_ptr->sa[i]);
    }
    QCLI_Printf("\r\n");

    memset(&invite_rsp_cmd, 0, sizeof(qapi_WLAN_P2P_Invite_Info_t));

    if (local_ptr->is_Persistent)
    {
        for (i = 0; i < __QAPI_WLAN_P2P_MAX_LIST_COUNT; i++)
        {
            if(memcmp(local_ptr->sa, p2p_peers_data[i].macaddr, __QAPI_WLAN_MAC_LEN) == 0)
            {
                invite_rsp_cmd.status = 0;
                inv_response_evt_index = i;
                memcpy(invite_rsp_cmd.group_Bss_ID, p2p_peers_data[i].macaddr, __QAPI_WLAN_MAC_LEN);
                break;
            }
        }

        if(i == __QAPI_WLAN_P2P_MAX_LIST_COUNT)
        {
            invite_rsp_cmd.status = 1;
            i = 0;
        }
    }
    else
    {
        invite_rsp_cmd.status = 0;
        memcpy(invite_rsp_cmd.group_Bss_ID, local_ptr->sa, __QAPI_WLAN_MAC_LEN);
    }
    p2p_session_in_progress = 1;
    deviceId = qc_api_wifi_get_active_device();
    if(deviceId != 0)
    {
        /* P2P device should always send/receive events/commands on dev 0 if
         * the app has switched to dev 1 while event is in dev 0 send command
         * via dev 0 and then switch to dev 1 */
        temp_device_id = deviceId;
        deviceId = 0;
        qc_api_wifi_set_active_deviceid(deviceId);
    }

    /* send invite auth event */
    if(qc_drv_p2p_invite_auth(qc_api_get_qc_drv_context(),
                deviceId, (qapi_WLAN_P2P_Invite_Info_t *)&invite_rsp_cmd) != 0)
    {
        LOG_WARN("StartP2P (P2P invite auth persistent)command did not execute properly\r\n");
    }

    if(temp_device_id != 0)
    {
        qc_api_wifi_set_active_deviceid(temp_device_id);
    }
    p2p_session_in_progress = 0;
    return;
}

void P2P_Event_Handler_Go_Neg_Result(CMD_P2P_EVENT_INFO *pEventInfo)
{
    qapi_WLAN_P2P_Go_Neg_Result_Event_t *p2pNeg = (qapi_WLAN_P2P_Go_Neg_Result_Event_t *) pEventInfo->pBuffer;
    qapi_WLAN_P2P_Go_Params_t goParams;
    uint32_t deviceId = 0, temp_device_id = 0, chnl = 0, signal_set = 0;
    int32_t error = 0, result = 0;
    qurt_time_t timeOutTicks = 5000;

    LOG_DEBUG("P2P GO Negotiation Result\r\n");
    LOG_DEBUG("    Status: %s\r\n",(p2pNeg->status) ? "FAILURE":"SUCCESS");

    /* If group negotiation result was a failure then stop processing further. */
    if(p2pNeg->status != 0)
    {
        p2p_persistent_go = QAPI_WLAN_P2P_NON_PERSISTENT_E;
        qc_drv_p2p_stop_find(qc_api_get_qc_drv_context(), deviceId);
        return;
    }

    LOG_DEBUG("    P2P Role   : %s\r\n",(p2pNeg->role_Go) ? "P2P GO": "P2P Client");
    LOG_DEBUG("    SSID       : %s\r\n", p2pNeg->ssid);
    LOG_DEBUG("    Channel    : %d\r\n", p2pNeg->freq);
    LOG_DEBUG("    WPS Method : %s\r\n",
            (p2pNeg->wps_Method == QAPI_WLAN_P2P_WPS_PBC_E) ? "PBC": "PIN");

    p2p_session_in_progress = 1;
    deviceId = qc_api_wifi_get_active_device();
    if(deviceId != 0)
    {

        /* P2P device should always send/receive events/commands on dev 0 if
         * the app has switched to dev 1 while event is in dev 0 send command
         * via dev 0 and then switch to dev 1 */

        temp_device_id = deviceId;
        deviceId = 0;
        qc_api_wifi_set_active_deviceid(deviceId);
    }

    memset(&goParams, 0, sizeof(qapi_WLAN_P2P_Go_Params_t));
    if(p2pNeg->role_Go == 1)
    {
        wps_flag = 0x01;
        chnl = (p2pNeg->freq-2412)/5 + 1;

        goParams.ssid_Len = p2pNeg->ssid_Len;
        goParams.passphrase_Len = p2pNeg->passphrase_Len;
        memcpy(goParams.ssid, p2pNeg->ssid, goParams.ssid_Len);
        memcpy(goParams.passphrase, p2pNeg->pass_Phrase, goParams.passphrase_Len);

        /* Reset global p2p_persistent_go variable */
        p2p_persistent_go = QAPI_WLAN_P2P_NON_PERSISTENT_E;
        if(qc_drv_p2p_start_go(qc_api_get_qc_drv_context(),
                    deviceId, &goParams,
                    chnl, p2pNeg->persistent_Grp) != 0)
        {
            LOG_AT_EVT("EVT_P2P: P2P connect command did not execute properly\r\n");
            goto set_temp_device;
        }
        waiting_connection_completion = 1;
        result = qurt_signal_wait_timed(&wlan_util_event, CONNECT_EVENT_MASK,
                (QURT_SIGNAL_ATTR_WAIT_ALL |
                 QURT_SIGNAL_ATTR_CLEAR_MASK),
                &signal_set, timeOutTicks);
        if(result != QURT_EOK)
        {

            LOG_AT_EVT("EVT_P2P: Waiting P2P connection failed\r\n");
            waiting_connection_completion = 0;
            goto set_temp_device;
        }
    }

    else if(p2pNeg->role_Go == 0)
    {
        uint8_t ssid[__QAPI_WLAN_MAX_SSID_LENGTH];
        qapi_WLAN_Dev_Mode_e opMode = QAPI_WLAN_DEV_MODE_STATION_E;
        qc_drv_wlan_set_op_mode(qc_api_get_qc_drv_context(),
                deviceId,
                &opMode,
                sizeof(qapi_WLAN_Dev_Mode_e),
                FALSE);

        memset(ssid, 0, sizeof(ssid));
        memcpy(ssid, p2pNeg->ssid, sizeof(p2pNeg->ssid));
        error = qc_drv_wlan_set_ssid (qc_api_get_qc_drv_context(),
                deviceId,
                ssid,
                sizeof(ssid),
                FALSE);
        if(error != 0)
        {
            LOG_AT_EVT("EVT_P2P: Unable to set SSID\r\n");
            goto set_temp_device;
        }
    }

    /* WPS */
    qapi_WLAN_WPS_Credentials_t wpsCreden;
    qapi_WLAN_WPS_Start_t wps_start;

    memset(&wpsCreden, 0, sizeof(qapi_WLAN_WPS_Credentials_t));
    memset(&wps_start, 0, sizeof(qapi_WLAN_WPS_Start_t));

    wps_context.connect_flag = (p2pNeg->role_Go) ? 0:8;

    if(p2pNeg->wps_Method != QAPI_WLAN_P2P_WPS_PBC_E)
    {
        wps_start.wps_Mode = QAPI_WLAN_WPS_PIN_MODE_E;
        wps_start.pin_Length = 9;

        //FIXME: This hardcoded pin value needs to be changed
        // for production to reflect what is on a sticker/label
        memcpy (wps_start.pin, p2p_wps_pin, wps_start.pin_Length);
        wps_start.pin[wps_start.pin_Length - 1] = '\0';
    }
    else
    {
        wps_start.wps_Mode = QAPI_WLAN_WPS_PBC_MODE_E;
    }

    memcpy(wpsCreden.ssid, p2pNeg->ssid, sizeof(p2pNeg->ssid));
    memcpy(wpsCreden.mac_Addr, p2pNeg->peer_Interface_Addr, __QAPI_WLAN_MAC_LEN);
    wpsCreden.ap_Channel  = p2pNeg->freq;
    wpsCreden.ssid_Length = p2pNeg->ssid_Len;

    qc_drv_wlan_set_wps_credentials (qc_api_get_qc_drv_context(),
            deviceId,
            &wpsCreden,
            sizeof(qapi_WLAN_WPS_Credentials_t),
            FALSE);

    /* Start WPS on the Aheros wifi */
    if(qc_drv_wlan_wps_start(qc_api_get_qc_drv_context(),
                deviceId, wps_context.connect_flag, wps_start.wps_Mode, (char *)wps_start.pin) != 0)
    {
        LOG_AT_EVT("EVT_P2P: devid - %d WPS failed.\r\n", deviceId);
        goto set_temp_device;
    }
    wps_context.wps_in_progress = 1;
    LOG_DEBUG("WPS started.\r\n");

set_temp_device:
    if(temp_device_id != 0)
    {
        qc_api_wifi_set_active_deviceid(temp_device_id);
    }
    p2p_session_in_progress = 0;
    return;
}

void P2P_Event_Handler(CMD_P2P_EVENT_INFO *pEventInfo)
{
    if (pEventInfo->event_id == __QAPI_WLAN_P2P_PROV_DISC_REQ_EVENTID)
    {
        P2P_Event_Handler_Prov_Disc_Req(pEventInfo);
    }
    else if (pEventInfo->event_id == __QAPI_WLAN_P2P_REQ_TO_AUTH_EVENTID)
    {
        P2P_Event_Handler_Req_To_Auth(pEventInfo);
    }
    else if (pEventInfo->event_id == __QAPI_WLAN_P2P_SDPD_RX_EVENTID)
    {
        P2P_Event_Handler_Sdpd_Rx(pEventInfo);
    }
    else if (pEventInfo->event_id == __QAPI_WLAN_P2P_PROV_DISC_RESP_EVENTID)
    {
        P2P_Event_Handler_Prov_Disc_Resp(pEventInfo);
    }
    else if (pEventInfo->event_id == __QAPI_WLAN_P2P_INVITE_SENT_RESULT_EVENTID)
    {
        P2P_Event_Handler_Invite_Sent_Result(pEventInfo);
    }
    else if (pEventInfo->event_id == __QAPI_WLAN_P2P_INVITE_RCVD_RESULT_EVENTID)
    {
        P2P_Event_Handler_Invite_Rcvd_Result(pEventInfo);
    }
    else if (pEventInfo->event_id == __QAPI_WLAN_P2P_INVITE_REQ_EVENTID)
    {
        P2P_Event_Handler_Invite_Req(pEventInfo);
    }
    else if (pEventInfo->event_id == __QAPI_WLAN_P2P_GO_NEG_RESULT_EVENTID)
    {
        P2P_Event_Handler_Go_Neg_Result(pEventInfo);
    }
    else
    {
        /* Should not come here */
        LOG_AT_EVT("EVT_P2P: Unknown P2P Event %d\n", pEventInfo->event_id);
    }
}

/* Add event info to the tail of the p2p event queue */
uint32_t app_p2p_queueP2PEventInfo(uint8_t device_id, uint16_t event_id,
        uint8_t *pBuffer, uint32_t Length)
{
    CMD_P2P_EVENT_INFO *pNewP2pEventInfo = NULL;

    /* Allocate memory for the new p2p event.
     * pNewP2pEventInfo->pBuffer[0] is pointer to the variable size pBuffer that is allocated based on the length
     */
    if((pNewP2pEventInfo = ((CMD_P2P_EVENT_INFO *)malloc(sizeof(CMD_P2P_EVENT_INFO) + Length))) == NULL){
        /* Failure to allocate memory will drop the event at caller */
        return -1;
    }

    pNewP2pEventInfo->nextEvent = NULL;
    pNewP2pEventInfo->device_id = device_id;
    pNewP2pEventInfo->event_id = event_id;
    pNewP2pEventInfo->length = Length;

    /* Copy the pBuffer to the variable length buffer pointer */
    memcpy(&(pNewP2pEventInfo->pBuffer[0]), pBuffer, pNewP2pEventInfo->length);

    /* aqucire mutex to update the p2p event queue */
    qurt_mutex_lock(&(p2pEventNode.eventQueueMutex));

    /* If empty, add to head
     * else add the event to the tail of the queue
     */
    if((NULL == p2pEventNode.pEventHead) && (NULL == p2pEventNode.pEventTail))
    {
        p2pEventNode.pEventHead = p2pEventNode.pEventTail = pNewP2pEventInfo;
    }
    else
    {
        p2pEventNode.pEventTail->nextEvent = pNewP2pEventInfo;
        p2pEventNode.pEventTail = pNewP2pEventInfo;
    }

    /* Release the mutex */
    qurt_mutex_unlock(&(p2pEventNode.eventQueueMutex));
    return 0;
}

/* Function to fetch event info at the head of the p2p event queue */
CMD_P2P_EVENT_INFO *app_p2p_getNextP2PEventInfo(void)
{
    CMD_P2P_EVENT_INFO *pTemp = NULL;

    /* Validate for empty queue */
    qurt_mutex_lock(&(p2pEventNode.eventQueueMutex));
    if(NULL != p2pEventNode.pEventHead)
    {
        /* aqucire mutex to update the p2p event queue */
        pTemp =  p2pEventNode.pEventHead;
        /* Update the queue head to the next or mark it empty */
        if(p2pEventNode.pEventHead == p2pEventNode.pEventTail)
        {
            p2pEventNode.pEventHead = p2pEventNode.pEventTail = NULL;
        }
        else
        {
            p2pEventNode.pEventHead = p2pEventNode.pEventHead->nextEvent;
        }
        /* Release the mutex */
    }
    qurt_mutex_unlock(&(p2pEventNode.eventQueueMutex));

    /* Return pointer to the event or NULL*/
    return(pTemp);
}


void app_handle_p2p_pending_events()
{
    CMD_P2P_EVENT_INFO *pTemp = NULL;

    /* Process all the pending P2P events */
    while ((pTemp = app_p2p_getNextP2PEventInfo()))
    {
        if (0 == pTemp->length)
        {
            free(pTemp);
            continue;
        }
        P2P_Event_Handler(pTemp);
        free(pTemp);
    }
    return;
}

void qc_api_wifi_app_wlan_p2p_event_cb(uint8_t device_Id, void *pData, uint32_t *pLength)
{
    qapi_WLAN_P2P_Event_Cb_Info_t *pP2p_Event_Cb_Info = (qapi_WLAN_P2P_Event_Cb_Info_t *)pData;
    uint32_t status = 0;

    switch(pP2p_Event_Cb_Info->event_ID)
    {
        case __QAPI_WLAN_P2P_GO_NEG_RESULT_EVENTID:
        case __QAPI_WLAN_P2P_REQ_TO_AUTH_EVENTID:
        case __QAPI_WLAN_P2P_PROV_DISC_RESP_EVENTID:
        case __QAPI_WLAN_P2P_PROV_DISC_REQ_EVENTID:
        case __QAPI_WLAN_P2P_INVITE_REQ_EVENTID:
        case __QAPI_WLAN_P2P_INVITE_RCVD_RESULT_EVENTID:
        case __QAPI_WLAN_P2P_INVITE_SENT_RESULT_EVENTID:
        case __QAPI_WLAN_P2P_SDPD_RX_EVENTID:
            {
                /* This callback is executed in the proxy thread context.
                 * No blocking event handler as each event is run to completion.
                 * Copy the event into queue that is process at APP context. */
                if(-1 == (status = app_p2p_queueP2PEventInfo(device_Id,
                                (uint16_t)pP2p_Event_Cb_Info->event_ID,
                                (uint8_t *)&pP2p_Event_Cb_Info->WLAN_P2P_Event_Info.go_Neg_Result_Event,
                                *pLength)))
                {
                    LOG_AT_EVT("EVT_P2P: Out of Memory: Dropping the P2P event\r\n");
                    return;
                }

                qurt_signal_set(&wlan_util_event, P2P_EVENT_MASK);
                break;
            }

        default:
            LOG_AT_EVT("EVT_P2P: Unknown P2P event %d\r\n", pP2p_Event_Cb_Info->event_ID);
            break;
    }
    return;
}

void app_free_p2p_pending_events()
{
    CMD_P2P_EVENT_INFO *pTemp = NULL;

    /* The check for 'p2pMode' is added to make sure that the mutex is
     * initialized before trying to lock it */
    if (p2pMode)
    {
        /* Free all the pending P2P events */
        qurt_mutex_lock(&(p2pEventNode.eventQueueMutex));
        while(p2pEventNode.pEventHead)
        {
            pTemp = p2pEventNode.pEventHead;
            p2pEventNode.pEventHead = p2pEventNode.pEventHead->nextEvent;
            free(pTemp);
        }
        p2pEventNode.pEventHead = NULL;
        p2pEventNode.pEventTail = NULL;
        qurt_mutex_unlock(&p2pEventNode.eventQueueMutex);
        qurt_mutex_destroy(&p2pEventNode.eventQueueMutex);
    }
    return;
}

#endif /* ENABLE_P2P_MODE */

#if ENABLE_P2P_MODE
int32_t qc_api_p2p_enable()
{
    uint32_t deviceId = qc_api_wifi_get_active_device();
    if(deviceId != 0)
    {
        LOG_INFO("Switch to device 0 to use p2p mode.\r\n");
        LOG_WARN("Current device is %d.\r\n", deviceId);
        return -1;
    }

    /* Following call sets the wlan_callback_handler() as the callback for asynchronous events */
    qc_api_wifi_set_callback(NULL);

    if(qc_drv_p2p_enable(qc_api_get_qc_drv_context(),
                deviceId, TRUE) != 0)
    {
        LOG_ERR("P2P not enabled.\r\n");
        return -1;
    }

    p2pEventNode.pEventHead = NULL;
    p2pEventNode.pEventTail = NULL;
    qurt_mutex_init(&p2pEventNode.eventQueueMutex);

    p2pMode = TRUE;
    p2p_intent = 0;    /* Default group owner intent */
    autogo_newpp = FALSE;
    p2p_cancel_enable = 0;
    p2p_persistent_go = QAPI_WLAN_P2P_NON_PERSISTENT_E;

    memset(p2p_wps_pin, 0, __QAPI_WLAN_WPS_PIN_LEN);
    strcpy(p2p_wps_pin, "12345670");

    /* Autonomous GO configurations */
    qc_api_wifi_set_passphrase("1234567890");
    memset(original_ssid, 0, __QAPI_WLAN_MAX_SSID_LENGTH);
    strcpy((char *)original_ssid,"DIRECT-iO");

    memset(p2p_join_mac_addr, 0, __QAPI_WLAN_MAC_LEN);
    memset(p2pScratchBuff, 0, sizeof(p2pScratchBuff));

    return QCLI_STATUS_SUCCESS_E;
}


int32_t qc_api_p2p_disable()
{
    qapi_WLAN_Dev_Mode_e opMode;

    uint32_t deviceId = qc_api_wifi_get_active_device();
    if(deviceId != 0)
    {
        LOG_INFO("Switch to device 0 to use p2p mode.\r\n");
        LOG_WARN("Current device is %d.\r\n", deviceId);
        return -1;
    }

    qc_api_wifi_set_callback(NULL);
    if(qc_drv_p2p_enable(qc_api_get_qc_drv_context(),
                deviceId, FALSE) != 0)
    {
        LOG_ERR("Disabling P2P mode failed.\r\n");
        return -1;
    }

    opMode = QAPI_WLAN_DEV_MODE_STATION_E;
    qc_drv_wlan_set_op_mode(qc_api_get_qc_drv_context(),
            deviceId,
            &opMode, sizeof(qapi_WLAN_Dev_Mode_e), FALSE);

    /* Free the event queue and destroy the mutex before disabling P2P. */
    app_free_p2p_pending_events();

    p2pMode = FALSE;
    set_channel_p2p = 0;
    return QCLI_STATUS_SUCCESS_E;
}

int32_t qc_api_p2p_set_config(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    qapi_WLAN_P2P_Config_Params_t p2pConfig;

    uint32_t deviceId = qc_api_wifi_get_active_device();
    if(deviceId != 0)
    {
        LOG_INFO("Switch to device 0 to use p2p mode.\r\n");
        LOG_WARN("Current device is %d.\r\n", deviceId);
        return -1;
    }

    qc_api_wifi_set_callback(NULL);
    set_channel_p2p = Parameter_List[2].Integer_Value; // for autogo

    p2p_intent = Parameter_List[0].Integer_Value;
    p2pConfig.go_Intent      = p2p_intent;
    p2pConfig.listen_Chan    = (uint8_t)Parameter_List[1].Integer_Value;
    p2pConfig.op_Chan	     = (uint8_t)Parameter_List[2].Integer_Value;
    p2pConfig.age		     = (uint32_t)Parameter_List[4].Integer_Value;
    p2pConfig.reg_Class      = 81;
    p2pConfig.op_Reg_Class   = 81;
    p2pConfig.max_Node_Count = 5;

    if (0 != qc_drv_p2p_set_config(qc_api_get_qc_drv_context(),
                deviceId,
                &p2pConfig, sizeof(p2pConfig), FALSE))
    {
        LOG_ERR("P2P configuration failed.\r\n");
        return -1;
    }

    LOG_INFO("Device configuration set successfully.\r\n");
    LOG_WARN("Note: Cannot set country code.\r\n");
    LOG_INFO("Use board data file or tuneables instead.\r\n");


    return QCLI_STATUS_SUCCESS_E;
}

uint8_t P2P_Check_Peer_Is_Found(const uint8_t *peer_addr, uint8_t p2p_operation)
{
    uint32_t deviceId = 0, dataLen = 0;
    uint8_t *local_ptr = NULL, index = 0, temp_val = 0, peer_found = FALSE;
    qapi_WLAN_P2P_Node_List_Params_t p2pNodeList;

    deviceId = qc_api_wifi_get_active_device();
    if(deviceId != 0)
    {
        LOG_INFO("Switch to device 0 to use p2p mode. ");
        LOG_WARN("Current device is %d.\r\n", deviceId);
        return FALSE;
    }

    qc_api_wifi_set_callback(NULL);

    memset(p2pScratchBuff, 0, sizeof(p2pScratchBuff));
    p2pNodeList.buffer_Length = __QAPI_WLAN_P2P_EVT_BUF_SIZE;
    p2pNodeList.node_List_Buffer = p2pScratchBuff;

    if (0 != qc_drv_p2p_get_node_list (qc_api_get_qc_drv_context(),
                deviceId,
                &p2pNodeList,
                &dataLen))
    {
        LOG_ERR("P2P node list command failed.\r\n");
        return FALSE;
    }

    if(!p2pNodeList.node_List_Buffer)
    {
        return FALSE;
    }
    local_ptr = p2pNodeList.node_List_Buffer;
    temp_val = *local_ptr;
    local_ptr++;
    if (temp_val > 0)
    {
        for (index = 0; index < temp_val; index++)
        {
            if(memcmp(((qapi_WLAN_P2P_Device_Lite_t *)(local_ptr))->p2p_Device_Addr,
                        peer_addr, __QAPI_WLAN_MAC_LEN) == 0)
            {
                peer_found = TRUE;
                break;
            }
            if(p2p_operation == P2P_PROVISION_OPERATION)
            {
                if(memcmp(((qapi_WLAN_P2P_Device_Lite_t *)(local_ptr))->interface_Addr,
                            peer_addr, __QAPI_WLAN_MAC_LEN) == 0)
                {
                    peer_found = TRUE;
                    break;
                }
            }
            local_ptr += sizeof(qapi_WLAN_P2P_Device_Lite_t);
        }
    }

    if(!peer_found)
    {
        LOG_ERR("The Peer Device is not found, please do P2P Find again.\r\n");
    }

    return peer_found;
}

int32_t qc_api_p2p_connect(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    uint32_t deviceId = 0, dataLen = 0;
    qapi_WLAN_P2P_Connect_Cmd_t p2p_connect;

    deviceId = qc_api_wifi_get_active_device();
    if(deviceId != 0)
    {
        LOG_INFO("Switch to device 0 to use p2p mode.\r\n");
        LOG_WARN("Current device is %d.\r\n", deviceId);
        return -1;
    }

    qc_api_wifi_set_callback(NULL);
    memset(&p2p_connect, 0, sizeof( qapi_WLAN_P2P_Connect_Cmd_t));

    if (0 != qc_drv_wlan_get_mac_addr (qc_api_get_qc_drv_context(),
                deviceId,
                p2p_connect.own_Interface_Addr,
                &dataLen))
    {
        LOG_ERR("Unable to obtain device mac address\r\n");
        return -1;
    }

    LOG_DEBUG("Own MAC addr : %02x:%02x:%02x:%02x:%02x:%02x \r\n",
            p2p_connect.own_Interface_Addr[0], p2p_connect.own_Interface_Addr[1],
            p2p_connect.own_Interface_Addr[2], p2p_connect.own_Interface_Addr[3],
            p2p_connect.own_Interface_Addr[4], p2p_connect.own_Interface_Addr[5]);

    if ((ether_aton((const char *)(Parameter_List[0].String_Value),p2p_connect.peer_Addr)) != 0)
    {
        QCLI_Printf("Invalid PEER MAC Address\r\n");
        return -1;
    }

    LOG_DEBUG("Peer MAC addr : %02x:%02x:%02x:%02x:%02x:%02x \r\n",
            p2p_connect.peer_Addr[0], p2p_connect.peer_Addr[1],
            p2p_connect.peer_Addr[2], p2p_connect.peer_Addr[3],
            p2p_connect.peer_Addr[4], p2p_connect.peer_Addr[5]);

    if(!P2P_Check_Peer_Is_Found(p2p_connect.peer_Addr,P2P_CONNECT_OPERATION))
    {
        return -1;
    }

    if(APP_STRCMP(Parameter_List[1].String_Value,"push") == 0)
    {
        p2p_connect.wps_Method = QAPI_WLAN_P2P_WPS_PBC_E;

        /* Check if user has given "persistent" option */
        if((Parameter_Count == 3) &&
                APP_STRCMP(Parameter_List[2].String_Value,"persistent") == 0)
        {
            p2p_connect.dev_Capab |= __QAPI_WLAN_P2P_PERSISTENT_FLAG;
        }
    }
    else
    {
        /* Check if user has provided WPS pin (8 characters) */
        if(strlen((char *)Parameter_List[2].String_Value) == 8)
        {
            memset(p2p_wps_pin, 0, __QAPI_WLAN_WPS_PIN_LEN);
            strcpy(p2p_wps_pin, (const char *)(Parameter_List[2].String_Value));

            /* Check if user has given "persistent" option */
            if((Parameter_Count == 4) &&
                    APP_STRCMP(Parameter_List[3].String_Value,"persistent") == 0)
            {
                p2p_connect.dev_Capab |= __QAPI_WLAN_P2P_PERSISTENT_FLAG;
            }
        }

        if(APP_STRCMP(Parameter_List[1].String_Value,"display") == 0)
        {
            p2p_connect.wps_Method = QAPI_WLAN_P2P_WPS_PIN_DISPLAY_E;
            LOG_INFO("WPS PIN %s \r\n",p2p_wps_pin);
        }
        else if(APP_STRCMP(Parameter_List[1].String_Value,"keypad") == 0)
        {
            p2p_connect.wps_Method = QAPI_WLAN_P2P_WPS_PIN_KEYPAD_E;
        }
    }

    /* Save P2P persistent flag as it will be needed while starting
       the group capability in beacons if role is P2P GO  */
    if (p2p_connect.dev_Capab & __QAPI_WLAN_P2P_PERSISTENT_FLAG)
    {
        p2p_persistent_go = QAPI_WLAN_P2P_PERSISTENT_E;
    }
    else
    {
        p2p_persistent_go = QAPI_WLAN_P2P_NON_PERSISTENT_E;
    }

    if(qc_drv_p2p_connect(qc_api_get_qc_drv_context(),
                deviceId, p2p_connect.wps_Method,
                p2p_connect.peer_Addr,
                p2p_persistent_go) != 0)
    {
        LOG_ERR("P2P connect command failed.\r\n");
        p2p_persistent_go = QAPI_WLAN_P2P_NON_PERSISTENT_E;
        return -1;
    }

    return QCLI_STATUS_SUCCESS_E;
}

int32_t qc_api_p2p_find(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    qapi_WLAN_P2P_Find_Cmd_t find_params;

    uint32_t deviceId = qc_api_wifi_get_active_device();
    if(deviceId != 0)
    {
        LOG_INFO("Switch to device 0 to use p2p mode.\r\n");
        LOG_WARN("Current device is %d.\r\n", deviceId);
        return -1;
    }

    qc_api_wifi_set_callback(NULL);
    memset(&find_params, 0, sizeof(qapi_WLAN_P2P_Find_Cmd_t));

    if(Parameter_Count == 1)
    {
        if(APP_STRCMP(Parameter_List[0].String_Value,"1") == 0)
        {
            find_params.type = (QAPI_WLAN_P2P_DISC_START_WITH_FULL_E);
            find_params.timeout = (P2P_STANDARD_TIMEOUT);
        }
        else if(APP_STRCMP(Parameter_List[0].String_Value,"2") == 0)
        {
            find_params.type = (QAPI_WLAN_P2P_DISC_ONLY_SOCIAL_E);
            find_params.timeout = (P2P_STANDARD_TIMEOUT);
        }
        else if(APP_STRCMP(Parameter_List[0].String_Value,"3") == 0)
        {
            find_params.type = (QAPI_WLAN_P2P_DISC_PROGRESSIVE_E);
            find_params.timeout = (P2P_STANDARD_TIMEOUT);
        }
        else
        {
            LOG_INFO("Wrong option. Enter option 1,2 or 3\r\n");
            return -1;
        }
    }

    else if(Parameter_Count == 2)
    {
        if(APP_STRCMP(Parameter_List[0].String_Value,"1") == 0)
        {
            find_params.type = (QAPI_WLAN_P2P_DISC_START_WITH_FULL_E);
            find_params.timeout = (Parameter_List[1].Integer_Value);
        }
        else if(APP_STRCMP(Parameter_List[0].String_Value,"2") == 0)
        {
            find_params.type = (QAPI_WLAN_P2P_DISC_ONLY_SOCIAL_E);
            find_params.timeout = (Parameter_List[1].Integer_Value);
        }
        else if(APP_STRCMP(Parameter_List[0].String_Value,"3") == 0)
        {
            find_params.type = (QAPI_WLAN_P2P_DISC_PROGRESSIVE_E);
            find_params.timeout = (Parameter_List[1].Integer_Value);
        }
        else
        {
            LOG_INFO("Wrong option. Enter option 1,2 or 3\r\n");
            return -1;
        }
    }

    else
    {
        find_params.type = (QAPI_WLAN_P2P_DISC_ONLY_SOCIAL_E);
        find_params.timeout = (P2P_STANDARD_TIMEOUT);
    }

    if(qc_drv_p2p_find(qc_api_get_qc_drv_context(),
                deviceId, find_params.type, find_params.timeout) != 0)
    {
        LOG_ERR("P2P find command failed.\r\n");
        return -1;
    }

    return QCLI_STATUS_SUCCESS_E;
}

int32_t qc_api_p2p_provision(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    qapi_WLAN_P2P_Prov_Disc_Req_Cmd_t p2p_prov_disc;
    qapi_WLAN_P2P_Connect_Cmd_t p2p_connect;

    uint32_t deviceId = qc_api_wifi_get_active_device();
    if(deviceId != 0)
    {
        LOG_INFO("Switch to device 0 to use p2p mode.\r\n");
        LOG_WARN("Current device is %d.\r\n", deviceId);
        return -1;
    }

    qc_api_wifi_set_callback(NULL);

    memset(&p2p_prov_disc, 0, sizeof(qapi_WLAN_P2P_Prov_Disc_Req_Cmd_t));
    if(APP_STRCMP(Parameter_List[1].String_Value, "push") == 0)
    {
        p2p_prov_disc.wps_Method = (uint16_t)__QAPI_WLAN_P2P_WPS_CONFIG_PUSHBUTTON;
    }
    else if(APP_STRCMP(Parameter_List[1].String_Value, "display") == 0)
    {
        p2p_prov_disc.wps_Method = (uint16_t)__QAPI_WLAN_P2P_WPS_CONFIG_DISPLAY;
    }
    else if(APP_STRCMP(Parameter_List[1].String_Value,"keypad") == 0)
    {
        p2p_prov_disc.wps_Method = (uint16_t)__QAPI_WLAN_P2P_WPS_CONFIG_KEYPAD;
    }
    else
    {
        LOG_ERR("Incorrect WPS method\r\n");
        return -1;
    }

    p2p_prov_disc.dialog_Token = 1;

    if ((ether_aton((const char *)(Parameter_List[0].String_Value),p2p_prov_disc.peer)) != 0)
    {
        LOG_ERR("Invalid PEER MAC Address\r\n");
        return -1;
    }

    if(!P2P_Check_Peer_Is_Found(p2p_prov_disc.peer,P2P_PROVISION_OPERATION))
    {
        return -1;
    }

    if( qc_drv_p2p_provision(qc_api_get_qc_drv_context(),
                deviceId, p2p_prov_disc.wps_Method, p2p_prov_disc.peer) != 0 )
    {
        LOG_ERR("P2P provision command failed.\r\n");
        return -1;
    }

    /* Authorize P2P Device */
    memset(&p2p_connect, 0, sizeof(qapi_WLAN_P2P_Connect_Cmd_t));
    if(APP_STRCMP(Parameter_List[1].String_Value, "push") == 0)
    {
        p2p_connect.wps_Method = QAPI_WLAN_P2P_WPS_PBC_E;
    }
    else if(APP_STRCMP(Parameter_List[1].String_Value, "display") == 0)
    {
        p2p_connect.wps_Method = QAPI_WLAN_P2P_WPS_PIN_DISPLAY_E;
    }
    else if(APP_STRCMP(Parameter_List[1].String_Value,"keypad") == 0)
    {
        p2p_connect.wps_Method = QAPI_WLAN_P2P_WPS_PIN_KEYPAD_E;
    }

    if ((ether_aton((const char *)(Parameter_List[0].String_Value),p2p_connect.peer_Addr)) != 0)
    {
        LOG_ERR("Invalid PEER MAC Address\r\n");
        return -1;
    }

    p2p_connect.go_Intent = p2p_intent;
    if(p2p_connect.wps_Method != QAPI_WLAN_P2P_WPS_NOT_READY_E)
    {
        if(qc_drv_p2p_auth(qc_api_get_qc_drv_context(),
                    deviceId, p2p_connect.dev_Auth,
                    p2p_connect.wps_Method, p2p_connect.peer_Addr,
                    ((p2p_connect.dev_Capab & __QAPI_WLAN_P2P_PERSISTENT_FLAG) ?
                     QAPI_WLAN_P2P_PERSISTENT_E : QAPI_WLAN_P2P_NON_PERSISTENT_E)) != 0)
        {
            LOG_ERR("P2P provision command failed.\r\n");
            return -1;
        }
    }
    return QCLI_STATUS_SUCCESS_E;
}

int32_t qc_api_p2p_listen(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    uint32_t timeout_val = 0;

    uint32_t deviceId = qc_api_wifi_get_active_device();
    if(deviceId != 0)
    {
        LOG_INFO("Switch to device 0 to use p2p mode.\r\n");
        LOG_WARN("Current device is %d.\r\n", deviceId);
        return -1;
    }

    qc_api_wifi_set_callback(NULL);
    if (Parameter_Count == 1 && Parameter_List[0].Integer_Is_Valid)
    {
        timeout_val = Parameter_List[0].Integer_Value;
    }
    else
    {
        timeout_val = P2P_STANDARD_TIMEOUT;
    }

    if(qc_drv_p2p_listen(qc_api_get_qc_drv_context(),
                deviceId, timeout_val) != 0)
    {
        LOG_ERR("P2P listen command failed.\r\n");
        return -1;
    }
    return QCLI_STATUS_SUCCESS_E;
}

int32_t qc_api_p2p_cancel(void)
{
    uint32_t deviceId = 0, dataLen = 0;
    qapi_WLAN_P2P_Network_List_Params_t p2pNetworkList;
    qapi_WLAN_Dev_Mode_e opMode;

    deviceId = qc_api_wifi_get_active_device();
    if(deviceId != 0)
    {
        LOG_INFO("Switch to device 0 to use p2p mode.\r\n");
        LOG_WARN("Current device is %d.\r\n", deviceId);
        return -1;
    }

    qc_api_wifi_set_callback(NULL);
    if(qc_drv_p2p_cancel(qc_api_get_qc_drv_context(), deviceId) != 0)
    {
        LOG_ERR("P2P cancel command failed.\r\n");
        return -1;
    }

    autogo_newpp = FALSE;
    p2p_cancel_enable = 1;

    memset(p2pScratchBuff, 0, sizeof(p2pScratchBuff));
    p2pNetworkList.network_List_Buffer = p2pScratchBuff;
    p2pNetworkList.buffer_Length = __QAPI_WLAN_P2P_EVT_BUF_SIZE;

    if (0 != qc_drv_p2p_get_network_list (qc_api_get_qc_drv_context(),
                deviceId,
                &p2pNetworkList,
                &dataLen))
    {
        LOG_ERR("P2P cancel command did not execute properly\r\n");
        return -1;
    }

    /* Following call to app_p2p_process_persistent_list_event() is made to make
     * p2p_cancel() blocking so that no other asynchronous p2p operations happen before
     * p2p_cancel() completes. */
    app_p2p_process_persistent_list_event(p2pNetworkList.network_List_Buffer);

    /* Reset to mode station */
    opMode = QAPI_WLAN_DEV_MODE_STATION_E;
    qc_drv_wlan_set_op_mode(qc_api_get_qc_drv_context(),
            deviceId,
            &opMode,
            sizeof(qapi_WLAN_Dev_Mode_e),
            FALSE);

    p2p_persistent_done = 0;
    qc_api_wifi_set_power_mode( QAPI_WLAN_POWER_MODE_REC_POWER_E, QAPI_WLAN_POWER_MODULE_P2P_E);

    return QCLI_STATUS_SUCCESS_E;
}

int32_t qc_api_p2p_join(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    uint32_t deviceId = 0, dataLen = 0;
    qapi_WLAN_P2P_Node_List_Params_t p2pNodeList;

    deviceId = qc_api_wifi_get_active_device();
    if(deviceId != 0)
    {
        LOG_INFO("Switch to device 0 to use p2p mode.\r\n");
        LOG_WARN("Current device is %d.\r\n", deviceId);
        return -1;
    }

    qc_api_wifi_set_callback(NULL);

    if ((ether_aton((const char *)(Parameter_List[0].String_Value),p2p_join_mac_addr)) != 0)
    {
        LOG_ERR("Invalid PEER MAC Address\r\n");
        return -1;
    }

    LOG_INFO("Interface MAC addr : %02x:%02x:%02x:%02x:%02x:%02x \r\n",
            p2p_join_mac_addr[0], p2p_join_mac_addr[1], p2p_join_mac_addr[2],
            p2p_join_mac_addr[3], p2p_join_mac_addr[4], p2p_join_mac_addr[5]);

    /* Update join profile */
    memset(&p2p_join_profile, 0, sizeof( qapi_WLAN_P2P_Connect_Cmd_t));
    if (0 != qc_drv_wlan_get_mac_addr (qc_api_get_qc_drv_context(),
                deviceId,
                p2p_join_profile.own_Interface_Addr,
                &dataLen))
    {
        LOG_ERR("Unable to obtain device mac address\r\n");
        return -1;
    }

    LOG_INFO("Own MAC addr : %02x:%02x:%02x:%02x:%02x:%02x \r\n",
            p2p_join_profile.own_Interface_Addr[0],
            p2p_join_profile.own_Interface_Addr[1],
            p2p_join_profile.own_Interface_Addr[2],
            p2p_join_profile.own_Interface_Addr[3],
            p2p_join_profile.own_Interface_Addr[4],
            p2p_join_profile.own_Interface_Addr[5]);

    if(APP_STRCMP(Parameter_List[1].String_Value,"push") == 0)
    {
        p2p_join_profile.wps_Method = QAPI_WLAN_P2P_WPS_PBC_E;
    }
    else if(APP_STRCMP(Parameter_List[1].String_Value,"display") == 0)
    {
        p2p_join_profile.wps_Method = QAPI_WLAN_P2P_WPS_PIN_DISPLAY_E;
    }
    else if(APP_STRCMP(Parameter_List[1].String_Value,"keypad") == 0)
    {
        p2p_join_profile.wps_Method = QAPI_WLAN_P2P_WPS_PIN_KEYPAD_E;
    }

    if(p2p_join_profile.wps_Method == QAPI_WLAN_P2P_WPS_PIN_DISPLAY_E ||
            p2p_join_profile.wps_Method == QAPI_WLAN_P2P_WPS_PIN_KEYPAD_E)
    {
        memset(p2p_wps_pin, 0, __QAPI_WLAN_WPS_PIN_LEN);
        strcpy(p2p_wps_pin, (const char *)(Parameter_List[2].String_Value));
    }

    memset(p2pScratchBuff, 0, sizeof(p2pScratchBuff));
    p2p_join_session_active = 1;
    p2pNodeList.buffer_Length = __QAPI_WLAN_P2P_EVT_BUF_SIZE;
    p2pNodeList.node_List_Buffer = p2pScratchBuff;

    if (0 != qc_drv_p2p_get_node_list (qc_api_get_qc_drv_context(),
                deviceId,
                &p2pNodeList,
                &dataLen))
    {
        LOG_ERR("P2P join command did not execute properly\r\n");
        return -1;
    }

    app_p2p_process_node_list_event(p2pNodeList.node_List_Buffer);
    return QCLI_STATUS_SUCCESS_E;
}

int32_t qc_api_p2p_auth(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    qapi_WLAN_P2P_Connect_Cmd_t p2p_connect;

    uint32_t deviceId = qc_api_wifi_get_active_device();
    if(deviceId != 0)
    {
        LOG_INFO("Switch to device 0 to use p2p mode.\r\n");
        LOG_WARN("Current device is %d.\r\n", deviceId);
        return -1;
    }

    qc_api_wifi_set_callback(NULL);
    memset(&p2p_connect, 0, sizeof(qapi_WLAN_P2P_Connect_Cmd_t));

    if(strlen((char *)Parameter_List[2].String_Value) == 8)
    {
        memset(p2p_wps_pin, 0, __QAPI_WLAN_WPS_PIN_LEN);
        strcpy(p2p_wps_pin, (const char *)(Parameter_List[2].String_Value));
        LOG_INFO("WPS Pin %s\r\n",p2p_wps_pin);

        /* Check if user has given "persistent" option */
        if((Parameter_Count == 4) &&
                APP_STRCMP(Parameter_List[3].String_Value,"persistent") == 0)
        {
            p2p_connect.dev_Capab |= __QAPI_WLAN_P2P_PERSISTENT_FLAG;
        }
    }
    if(APP_STRCMP(Parameter_List[1].String_Value, "deauth") == 0)
    {
        p2p_connect.dev_Auth = 1;
    }
    if(APP_STRCMP(Parameter_List[1].String_Value, "push") == 0)
    {
        p2p_connect.wps_Method = QAPI_WLAN_P2P_WPS_PBC_E;

        /* Check if user has given "persistent" option */
        if((Parameter_Count == 3) &&
                APP_STRCMP(Parameter_List[2].String_Value,"persistent") == 0)
        {
            p2p_connect.dev_Capab |= __QAPI_WLAN_P2P_PERSISTENT_FLAG;
        }
    }
    else if(APP_STRCMP(Parameter_List[1].String_Value, "display") == 0)
    {
        p2p_connect.wps_Method = QAPI_WLAN_P2P_WPS_PIN_DISPLAY_E;
        LOG_INFO("WPS PIN %s \r\n",p2p_wps_pin);
    }
    else if(APP_STRCMP(Parameter_List[1].String_Value,"keypad") == 0)
    {
        p2p_connect.wps_Method = QAPI_WLAN_P2P_WPS_PIN_KEYPAD_E;
    }

    if ((ether_aton((const char *)(Parameter_List[0].String_Value),p2p_connect.peer_Addr)) != 0)
    {
        LOG_ERR("Invalid PEER MAC Address\r\n");
        return -1;
    }

    if(!P2P_Check_Peer_Is_Found(p2p_connect.peer_Addr,P2P_AUTH_OPERATION))
    {
        return -1;
    }

    /* Save P2P persistent flag as it will be needed while starting
       the group capability in beacons if role is P2P GO  */
    if (p2p_connect.dev_Capab & __QAPI_WLAN_P2P_PERSISTENT_FLAG)
    {
        p2p_persistent_go = QAPI_WLAN_P2P_PERSISTENT_E;
    }
    else
    {
        p2p_persistent_go = QAPI_WLAN_P2P_NON_PERSISTENT_E;
    }

    if(qc_drv_p2p_auth(qc_api_get_qc_drv_context(),
                deviceId,
                p2p_connect.dev_Auth,
                (qapi_WLAN_P2P_WPS_Method_e) p2p_connect.wps_Method,
                p2p_connect.peer_Addr, p2p_persistent_go) != 0)
    {
        LOG_ERR("StartP2P command did not execute properly\r\n");
        p2p_persistent_go = QAPI_WLAN_P2P_NON_PERSISTENT_E;
        return -1;
    }

    return QCLI_STATUS_SUCCESS_E;
}

int32_t qc_api_p2p_auto_go(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    uint8_t persistent_Group = 0;
    int32_t go_chan = __QAPI_WLAN_P2P_DEFAULT_CHAN;
    uint32_t channel = 0, dataLen = 0;
    uint32_t deviceId = qc_api_wifi_get_active_device();
    if(deviceId != 0)
    {
        LOG_INFO("Switch to device 0 to use p2p mode.\r\n");
        LOG_WARN("Current device is %d.\r\n", deviceId);
        return -1;
    }

    /* if set_channel_p2p is valid, prefer to use set_channel_p2p */
    if (set_channel_p2p != 0) {
        go_chan = set_channel_p2p;
    }

    /*
     * If the device 1 is connected, we start GO on the home channel of device 1.
     */

    if(wifi_state[1] ==1)
    {
        qc_api_wifi_set_active_deviceid(1);
        qc_drv_wlan_get_channel (qc_api_get_qc_drv_context(), 1,
                (uint16_t *)&channel,
                &dataLen);
        go_chan = channel;
        qc_api_wifi_set_active_deviceid(0);
    }


    qc_api_wifi_set_callback(NULL);
    p2pMode = TRUE;
    wps_flag = 0x01;

    LOG_DEBUG("Starting Autonomous GO.\r\n");

    /* Check if user has given "persistent" option */
    if(Parameter_Count && APP_STRCMP(Parameter_List[0].String_Value,"persistent") == 0)
    {
        persistent_Group = 1;
    }
    else
    {
        persistent_Group = 0;
    }

    if(FALSE == autogo_newpp)
    {
        qapi_WLAN_P2P_Go_Params_t goParams;
        memset(&goParams, 0, sizeof(qapi_WLAN_P2P_Go_Params_t));
        goParams.ssid_Len = strlen("DIRECT-iO");
        goParams.passphrase_Len = strlen(wpa_passphrase[deviceId]);

        memcpy(goParams.ssid, "DIRECT-iO", goParams.ssid_Len);
        memcpy(goParams.passphrase, wpa_passphrase[deviceId], goParams.passphrase_Len);

        if(qc_drv_p2p_start_go(qc_api_get_qc_drv_context(),
                    deviceId, &goParams,
                    go_chan,
                    persistent_Group) != 0)
        {
            LOG_ERR("P2P auto GO command did not execute properly.\r\n");
            return -1;
        }
    }

    else
    {
        if(qc_drv_p2p_start_go(qc_api_get_qc_drv_context(),
                    deviceId, NULL, go_chan,
                    persistent_Group) != 0)
        {
            LOG_ERR("P2P auto GO command did not execute properly.\r\n");
            return -1;
        }
    }
    return QCLI_STATUS_SUCCESS_E;
}

int32_t qc_api_p2p_invite_auth(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    qapi_WLAN_P2P_Invite_Cmd_t p2pInvite;
    qapi_WLAN_P2P_Go_Params_t ssidParams;
    qapi_WLAN_Auth_Mode_e authMode = QAPI_WLAN_AUTH_WPA2_PSK_E;
    qapi_WLAN_Crypt_Type_e encrType = QAPI_WLAN_CRYPT_AES_CRYPT_E;
    uint32_t deviceId = 0, wifimode = 0, dataLen = 0, k = 0;
    uint8_t p2p_invite_role;

    deviceId = qc_api_wifi_get_active_device();
    if(deviceId != 0)
    {
        LOG_INFO("Switch to device 0 to use p2p mode.\r\n");
        LOG_WARN("Current device is %d.\r\n", deviceId);
        return -1;
    }

    qc_api_wifi_set_callback(NULL);
    memset(&p2pInvite, 0, sizeof(qapi_WLAN_P2P_Invite_Cmd_t));
    memset(&ssidParams, 0, sizeof(qapi_WLAN_P2P_Go_Params_t));

    for(k = 0; k < __QAPI_WLAN_P2P_MAX_LIST_COUNT; k++)
    {
        if(strncmp((char *)p2p_peers_data[k].ssid,
                    (const char *)(Parameter_List[0].String_Value),
                    strlen((const char *)(Parameter_List[0].String_Value))) == 0)
        {
            break;
        }

    }
    if(k == __QAPI_WLAN_P2P_MAX_LIST_COUNT)
    {
        qc_drv_wlan_get_op_mode (qc_api_get_qc_drv_context(),
                deviceId,
                &wifimode,
                &dataLen);
        if((wifimode == QAPI_WLAN_DEV_MODE_STATION_E) && (get_dev_stat(deviceId) == UP))
        {
            p2p_invite_role = QAPI_WLAN_P2P_INV_ROLE_CLIENT_E;
            p2pInvite.is_Persistent=0;
        }
        k = 0;
    }
    else
    {
        invitation_index = k;
        p2p_invite_role = p2p_peers_data[k].role;
        p2pInvite.is_Persistent=1;
    }

    if ((ether_aton((const char *)(Parameter_List[1].String_Value), p2pInvite.peer_Addr)) != 0)
    {
        LOG_ERR("Invalid Invitation MAC Address\r\n");
        return -1;
    }

    if(!P2P_Check_Peer_Is_Found(p2pInvite.peer_Addr,P2P_INVITE_OPERATION))
    {
        return -1;
    }

    if(APP_STRCMP(Parameter_List[2].String_Value, "push") == 0)
    {
        p2pInvite.wps_Method = QAPI_WLAN_P2P_WPS_PBC_E;
    }
    else if(APP_STRCMP(Parameter_List[2].String_Value, "display") == 0)
    {
        p2pInvite.wps_Method = QAPI_WLAN_P2P_WPS_PIN_KEYPAD_E;
    }
    else if(APP_STRCMP(Parameter_List[2].String_Value,"keypad") == 0)
    {
        p2pInvite.wps_Method = QAPI_WLAN_P2P_WPS_PIN_DISPLAY_E;
    }
    else
    {
        LOG_ERR("Incorrect wps method not proper P2P Invite \r\n");
        return -1;
    }

    if( qc_drv_p2p_invite(qc_api_get_qc_drv_context(),
                deviceId, (const char *)(Parameter_List[0].String_Value),
                p2pInvite.wps_Method, p2pInvite.peer_Addr,
                p2pInvite.is_Persistent, p2p_invite_role) != 0 )
    {
        LOG_ERR("P2P command did not execute properly\r\n");
        return -1;
    }

    if(p2p_invite_role == QAPI_WLAN_P2P_INV_ROLE_ACTIVE_GO_E)
    {
        if(0 != qc_drv_wlan_set_encr_type(qc_api_get_qc_drv_context(),
                    deviceId,
                    (void *) &encrType, //QCOM_WLAN_CRYPT_AES_CRYPT
                    sizeof(qapi_WLAN_Crypt_Type_e), FALSE))
        {
            return -1;
        }

        if ( 0 != qc_drv_wlan_set_auth_mode (qc_api_get_qc_drv_context(),
                    deviceId,
                    (void *) &authMode, //QCOM_WLAN_AUTH_WPA2_PSK
                    sizeof(qapi_WLAN_Auth_Mode_e), FALSE))
        {
            return -1;
        }

        memcpy(ssidParams.passphrase, (char *)p2p_peers_data[k].passphrase,
                strlen((char *)p2p_peers_data[k].passphrase));
        memcpy(ssidParams.ssid, (char *)p2p_peers_data[k].ssid,
                strlen((char *)p2p_peers_data[k].ssid));
        ssidParams.ssid_Len = strlen((char *)p2p_peers_data[k].ssid);
        ssidParams.passphrase_Len = strlen((char *)p2p_peers_data[k].passphrase);

        if ( 0 != qc_drv_p2p_set_ssidparam(qc_api_get_qc_drv_context(),
                    deviceId,
                    &ssidParams, sizeof(ssidParams), FALSE))
        {
            LOG_ERR("P2P command did not execute properly\r\n");
            return -1;
        }
    }

    return QCLI_STATUS_SUCCESS_E;
}

int32_t qc_api_p2p_get_node_list()
{
    uint32_t deviceId = 0, dataLen = 0;
    qapi_WLAN_P2P_Node_List_Params_t p2pNodeList;

    deviceId = qc_api_wifi_get_active_device();
    if(deviceId != 0)
    {
        LOG_INFO("Switch to device 0 to use p2p mode. ");
        LOG_WARN("Current device is %d.\r\n", deviceId);
        return -1;
    }

    qc_api_wifi_set_callback(NULL);

    memset(p2pScratchBuff, 0, sizeof(p2pScratchBuff));
    p2pNodeList.buffer_Length = __QAPI_WLAN_P2P_EVT_BUF_SIZE;
    p2pNodeList.node_List_Buffer = p2pScratchBuff;

    if (0 != qc_drv_p2p_get_node_list (qc_api_get_qc_drv_context(),
                deviceId,
                &p2pNodeList,
                &dataLen))
    {
        LOG_ERR("P2P node list command failed.\r\n");
        return -1;
    }

    app_p2p_process_node_list_event(p2pNodeList.node_List_Buffer);
    return QCLI_STATUS_SUCCESS_E;
}

int32_t qc_api_p2p_get_network_list()
{
    uint32_t deviceId = 0, dataLen = 0;
    qapi_WLAN_P2P_Network_List_Params_t p2pNetworkList;

    deviceId = qc_api_wifi_get_active_device();
    if(deviceId != 0)
    {
        LOG_INFO("Switch to device 0 to use p2p mode. ");
        LOG_WARN("Current device is %d.\r\n", deviceId);
        return -1;
    }

    qc_api_wifi_set_callback(NULL);
    memset(p2pScratchBuff, 0, sizeof(p2pScratchBuff));
    p2pNetworkList.network_List_Buffer = p2pScratchBuff;
    p2pNetworkList.buffer_Length = __QAPI_WLAN_P2P_EVT_BUF_SIZE;

    if (0 != qc_drv_p2p_get_network_list (qc_api_get_qc_drv_context(),
                deviceId,
                &p2pNetworkList,
                &dataLen))
    {
        LOG_ERR("P2P command did not execute properly\r\n");
        return -1;
    }

    app_p2p_process_persistent_list_event(p2pNetworkList.network_List_Buffer);

    return QCLI_STATUS_SUCCESS_E;
}

int32_t qc_api_p2p_set_noa_params(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    qapi_WLAN_P2P_Noa_Params_t noaParams;

    uint32_t deviceId = qc_api_wifi_get_active_device();
    if(deviceId != 0)
    {
        LOG_INFO("Switch to device 0 to use p2p mode.\r\n");
        LOG_WARN("Current device is %d.\r\n", deviceId);
        return -1;
    }

    qc_api_wifi_set_callback(NULL);
    memset(&noaParams, 0, sizeof(qapi_WLAN_P2P_Noa_Params_t));

    noaParams.noa_Desc_Params[0].type_Count = Parameter_List[0].Integer_Value;
    noaParams.noa_Desc_Params[0].start_Offset_Us = Parameter_List[1].Integer_Value;
    noaParams.noa_Desc_Params[0].duration_Us = Parameter_List[2].Integer_Value;
    noaParams.noa_Desc_Params[0].interval_Us = Parameter_List[3].Integer_Value;
    noaParams.enable = 1;
    noaParams.count = 1;

    if (0 != qc_drv_p2p_set_noa_params (qc_api_get_qc_drv_context(),
                deviceId,
                &noaParams,
                sizeof(noaParams),
                FALSE))
    {
        LOG_ERR("P2P command did not execute properly\r\n");
        return -1;
    }

    return QCLI_STATUS_SUCCESS_E;
}

int32_t qc_api_p2p_set_oops_params(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    qapi_WLAN_P2P_Opps_Params_t opps;

    uint32_t deviceId = qc_api_wifi_get_active_device();
    if(deviceId != 0)
    {
        LOG_INFO("Switch to device 0 to use p2p mode.\r\n");
        LOG_WARN("Current device is %d.\r\n", deviceId);
        return -1;
    }

    qc_api_wifi_set_callback(NULL);
    memset(&opps, 0, sizeof(qapi_WLAN_P2P_Opps_Params_t));
    opps.ct_Win	= Parameter_List[0].Integer_Value;
    opps.enable = Parameter_List[1].Integer_Value;
    if (0 != qc_drv_p2p_set_oops_params (qc_api_get_qc_drv_context(),
                deviceId,
                &opps, sizeof(opps),
                FALSE))
    {
        LOG_ERR("P2P command did not execute properly\r\n");
        return -1;
    }
    return QCLI_STATUS_SUCCESS_E;
}

int32_t qc_api_p2p_set_operating_class(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    qapi_WLAN_P2P_Config_Params_t p2pConfig;

    uint32_t deviceId = qc_api_wifi_get_active_device();
    if(deviceId != 0)
    {
        LOG_INFO("Switch to device 0 to use p2p mode.\r\n");
        LOG_WARN("Current device is %d.\r\n", deviceId);
        return -1;
    }

    qc_api_wifi_set_callback(NULL);
    memset(&p2pConfig, 0, sizeof(qapi_WLAN_P2P_Config_Params_t));

    p2p_intent = Parameter_List[0].Integer_Value;
    set_channel_p2p = Parameter_List[2].Integer_Value;

    p2pConfig.go_Intent      = p2p_intent;
    p2pConfig.listen_Chan    = 6;
    p2pConfig.op_Chan	     = Parameter_List[2].Integer_Value;
    p2pConfig.age            = 3000;
    p2pConfig.reg_Class      = 81;
    p2pConfig.op_Reg_Class	 = Parameter_List[1].Integer_Value;
    p2pConfig.max_Node_Count = 4;

    if (0 != qc_drv_p2p_set_config(qc_api_get_qc_drv_context(),
                deviceId,
                &p2pConfig, sizeof(p2pConfig), FALSE))
    {
        LOG_ERR("P2P command did not execute properly\r\n");
        return -1;
    }
    return QCLI_STATUS_SUCCESS_E;
}


int32_t qc_api_p2p_stop_find()
{
    uint32_t deviceId = qc_api_wifi_get_active_device();
    if(deviceId != 0)
    {
        LOG_INFO("Switch to device 0 to use p2p mode.\r\n");
        LOG_WARN("Current device is %d.\r\n", deviceId);
        return -1;
    }

    qc_api_wifi_set_callback(NULL);
    if(qc_drv_p2p_stop_find(qc_api_get_qc_drv_context(), deviceId) != 0)
    {
        LOG_ERR("P2P stop command did not execute properly\r\n");
        return -1;
    }
    return QCLI_STATUS_SUCCESS_E;
}

int32_t qc_api_p2p_passphrase(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    qapi_WLAN_Auth_Mode_e authMode = QAPI_WLAN_AUTH_WPA2_PSK_E;
    qapi_WLAN_Crypt_Type_e encrType = QAPI_WLAN_CRYPT_AES_CRYPT_E;
    qapi_WLAN_P2P_Go_Params_t ssidParams;

    uint32_t deviceId = qc_api_wifi_get_active_device();
    if(deviceId != 0)
    {
        LOG_INFO("Switch to device 0 to use p2p mode.\r\n");
        LOG_WARN("Current device is %d.\r\n", deviceId);
        return -1;
    }

    qc_api_wifi_set_callback(NULL);
    memset(&ssidParams, 0, sizeof(qapi_WLAN_P2P_Go_Params_t));

    if(0 != qc_drv_wlan_set_encr_type(qc_api_get_qc_drv_context(),
                deviceId,
                (void *) &encrType,
                sizeof(qapi_WLAN_Crypt_Type_e),
                FALSE))
    {
        return -1;
    }

    if (0 != qc_drv_wlan_set_auth_mode(qc_api_get_qc_drv_context(),
                deviceId,
                &authMode,
                sizeof(qapi_WLAN_Auth_Mode_e),
                FALSE))

    {
        return -1;
    }

    autogo_newpp = TRUE;
    memcpy(ssidParams.passphrase, (char *)(Parameter_List[0].String_Value),
            strlen((char *)(Parameter_List[0].String_Value)));
    memcpy(ssidParams.ssid, (char *)(Parameter_List[1].String_Value),
            strlen((char *)(Parameter_List[1].String_Value)));

    ssidParams.ssid_Len = strlen((char *)(Parameter_List[1].String_Value));
    ssidParams.passphrase_Len = strlen((char *)(Parameter_List[0].String_Value));

    if ( 0 != qc_drv_p2p_set_ssidparam(qc_api_get_qc_drv_context(),
                deviceId,
                &ssidParams,	sizeof(ssidParams), FALSE))
    {
        LOG_ERR("P2P command did not execute properly\r\n");
        return -1;
    }
    return QCLI_STATUS_SUCCESS_E;
}

int32_t qc_api_p2p_set(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    uint32_t deviceId = 0, len = 0;
    qapi_WLAN_P2P_Set_Cmd_t p2p_set_params;

    deviceId = qc_api_wifi_get_active_device();
    if(deviceId != 0)
    {
        LOG_INFO("Switch to device 0 to use p2p mode.\r\n");
        LOG_WARN("Current device is %d.\r\n", deviceId);
        return -1;
    }

    qc_api_wifi_set_callback(NULL);
    memset(&p2p_set_params, 0, sizeof(qapi_WLAN_P2P_Set_Cmd_t));

    if(APP_STRCMP(Parameter_List[0].String_Value,"p2pmode") == 0)
    {
        if (Parameter_Count < 2)
        {
            LOG_ERR("Incorrect parameters\r\n", deviceId);
            return -1;
        }

        p2p_set_params.config_Id = __QAPI_WLAN_PARAM_GROUP_P2P_OP_MODE;

        if(APP_STRCMP(Parameter_List[1].String_Value,"p2pdev") == 0)
        {
            p2p_set_params.val.mode.p2pmode = __QAPI_WLAN_P2P_DEV;
        }
        else if(APP_STRCMP(Parameter_List[1].String_Value,"p2pclient") == 0)
        {
            p2p_set_params.val.mode.p2pmode = __QAPI_WLAN_P2P_CLIENT;
        }
        else if(APP_STRCMP(Parameter_List[1].String_Value,"p2pgo") == 0)
        {
            p2p_set_params.val.mode.p2pmode = __QAPI_WLAN_P2P_GO;
        }
        else
        {
            LOG_INFO("Input can be \"p2pdev/p2pclient/p2pgo\"");
            return -1;
        }

        len = sizeof(p2p_set_params.val.mode);
        LOG_INFO("p2p mode :%x, Config Id %x\r\n",p2p_set_params.val.mode.p2pmode,p2p_set_params.config_Id);
    }

    else if(APP_STRCMP(Parameter_List[0].String_Value,"postfix") == 0)
    {
        p2p_set_params.config_Id = __QAPI_WLAN_PARAM_GROUP_P2P_SSID_POSTFIX;
        if(strlen((char *)Parameter_List[1].String_Value)) {
            memcpy(p2p_set_params.val.ssid_Postfix.ssid_Postfix,
                    (char *)Parameter_List[1].String_Value,
                    strlen((char *)Parameter_List[1].String_Value));

            p2p_set_params.val.ssid_Postfix.ssid_Postfix_Length = strlen((char *)Parameter_List[1].String_Value);
            len = sizeof(p2p_set_params.val.ssid_Postfix);
            LOG_INFO("PostFix string %s, Len %d\r\n",
                    p2p_set_params.val.ssid_Postfix.ssid_Postfix,
                    p2p_set_params.val.ssid_Postfix.ssid_Postfix_Length);
        }
    }

    else if(APP_STRCMP(Parameter_List[0].String_Value, "intrabss") == 0)
    {
        p2p_set_params.config_Id = __QAPI_WLAN_PARAM_GROUP_P2P_INTRA_BSS;
        p2p_set_params.val.intra_Bss.flag = Parameter_List[1].Integer_Value;
        len = sizeof(p2p_set_params.val.intra_Bss);
    }

    else if(APP_STRCMP(Parameter_List[0].String_Value, "gointent") == 0)
    {
        p2p_set_params.config_Id = __QAPI_WLAN_PARAM_GROUP_P2P_GO_INTENT;
        p2p_set_params.val.go_Intent.value = Parameter_List[1].Integer_Value;
        len = sizeof(p2p_set_params.val.go_Intent);
    }

    else if (APP_STRCMP(Parameter_List[0].String_Value, "cckrates") == 0){
        p2p_set_params.config_Id = __QAPI_WLAN_PARAM_GROUP_P2P_CCK_RATES;
        p2p_set_params.val.cck_Rates.enable = Parameter_List[1].Integer_Value;
        len = sizeof(p2p_set_params.val.cck_Rates);
    }
    else if (APP_STRCMP(Parameter_List[0].String_Value, "listenchannel") == 0) {
        if ((Parameter_Count < 3) ||
                (Parameter_List[1].Integer_Value < 0) ||
                (Parameter_List[2].Integer_Value < 0)){
            LOG_ERR("Incorrect parameters\r\n", deviceId);
            return -1;
        }

        p2p_set_params.config_Id = __QAPI_WLAN_PARAM_GROUP_P2P_LISTEN_CHANNEL;
        p2p_set_params.val.listen_Channel.reg_Class = Parameter_List[1].Integer_Value;
        p2p_set_params.val.listen_Channel.channel = Parameter_List[2].Integer_Value;
        len = sizeof(p2p_set_params.val.cck_Rates);
    }

    else if (APP_STRCMP(Parameter_List[0].String_Value, "devname") == 0) {
        if (Parameter_Count < 2) {
            LOG_ERR("Incorrect parameters\r\n");
            return -1;
        }

        len = strlen((char *)Parameter_List[1].String_Value);
        if (len > __QAPI_WLAN_P2P_WPS_MAX_DEVNAME_LEN) {
            LOG_ERR("Device name exceeds the allowed length\r\n");
            return -1;
        }

        p2p_set_params.config_Id = __QAPI_WLAN_PARAM_GROUP_P2P_DEV_NAME;
        memset(p2p_set_params.val.device_Name.dev_Name, 0, __QAPI_WLAN_P2P_WPS_MAX_DEVNAME_LEN + 1);
        memcpy(p2p_set_params.val.device_Name.dev_Name, (char *)Parameter_List[1].String_Value, len);
        p2p_set_params.val.device_Name.dev_Name_Len = len;
    }

    else
    {
        LOG_ERR("Incorrect parameters\r\n", deviceId);
        return -1;
    }

    if (0 != qc_drv_p2p_set (qc_api_get_qc_drv_context(),
                deviceId,
                p2p_set_params.config_Id,
                (int *)&p2p_set_params.val,
                len,
                FALSE))
    {
        LOG_ERR("P2P set command did not execute properly\r\n");
        return -1;
    }
    return QCLI_STATUS_SUCCESS_E;
}

#endif /* ENABLE_P2P_MODE */


