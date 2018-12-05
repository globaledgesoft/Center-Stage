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
#include <string.h>
#include "util.h"
#include <qcli.h>
#include <qcli_api.h>
#include "qurt_thread.h"
#include "qurt_timer.h"
#include "qurt_error.h"
#include "qurt_mutex.h"
#include "qurt_signal.h"
#include "qapi_fatal_err.h"
#include "qapi_ns_utils.h"

#define ENABLE_AP_MODE       1 
#define ENABLE_SCC_MODE      0

#include "qapi_wlan.h"
#include "qapi_wlan_lib.h"

int wlan_dbg_init (void *Group_Handle, void *cb_printf_f);
void wlan_dbg_exit (void);

#if defined(ENABLE_PER_FN_PROFILING)
#include "qapi_cpuprofile.h"
#endif

/************************** Global Declarations ************************************************/

#define FALSE 0
#define TRUE  1
typedef int  boolean_t;

extern QCLI_Group_Handle_t qcli_wlan_group;
extern QCLI_Group_Handle_t qcli_p2p_group;

#ifndef WLAN_NUM_OF_DEVICES
    #define WLAN_NUM_OF_DEVICES 2
#endif

/* Utility thread macros */
#define UTILITY_THREAD_PRIORITY       10
#define UTILITY_THREAD_STACK_SIZE   2048
#define WPS_EVENT_MASK               0x1
#define P2P_EVENT_MASK               0x2
#define CONNECT_EVENT_MASK  		 0x4
qurt_thread_attr_t Utility_Thread_Attribute;
qurt_thread_t Utility_Thread_Handle;
qurt_signal_t Utility_Event;
qurt_mutex_t UtilityMutex;
uint8_t Utility_Event_Initialized = 0, Utility_Thread_Created = 0;
uint8_t Waiting_Connection_Completion = 0;

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

typedef struct {
    uint8_t wps_in_progress;
    uint8_t connect_flag;
    uint8_t wps_pbc_interrupt;
    qapi_WLAN_Netparams_t netparams;
} wps_context_t;

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

/* P2P Event Queue Nodes */
typedef struct _CMD_P2P_EVENT_INFO_{
   struct _CMD_P2P_EVENT_INFO_ *nextEvent; // link to next event in the queue
   uint8_t device_id;
   uint16_t event_id;
   uint32_t length;
   uint8_t pBuffer[1]; //variable size buffer. Allocation based on the Length
} CMD_P2P_EVENT_INFO;

/* P2P Event Queue */
typedef struct {
    qurt_mutex_t eventQueueMutex; //queue mutex
    CMD_P2P_EVENT_INFO *pEventHead; //queue head
    CMD_P2P_EVENT_INFO *pEventTail; // queue tail
} CMD_P2P_EVENT_LIST;

CMD_P2P_EVENT_LIST p2pEventNode;

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

int user_defined_power_mode = QAPI_WLAN_POWER_MODE_MAX_PERF_E; //QAPI_WLAN_POWER_MODE_MAX_PERF_E;
#define PROBE_REQ_SRC_ADDR_OFFSET  (10)

#if ENABLE_P2P_MODE
#define   P2P_CONNECT_OPERATION     1
#define   P2P_PROVISION_OPERATION   2
#define   P2P_AUTH_OPERATION        3
#define   P2P_INVITE_OPERATION      4 
#endif

/************ Function Prototypes *********************************************/
int32_t wlan_callback_handler( uint8_t  deviceId, 
                               uint32_t cbId, 
                               void *pApplicationContext,
                               void     *payload,
                               uint32_t payload_Length);

int32_t wps_query(uint8_t block);

#if ENABLE_P2P_MODE
void app_handle_p2p_pending_events();
void app_free_p2p_pending_events();
#endif /* ENABLE_P2P_MODE */

uint16_t freq_to_channel(uint16_t channel_val);
uint32_t chan_to_frequency(uint32_t channel);

/***********************************************************************************************/

/* Utility thread is created when WLAN is enabled and it is used to process asynchronous events
 * from the firmware such as WPS, P2P.
 */
void Wlan_Utility_Thread(void *para)
{
	uint32_t utility_signals = 0; /* Possible signal masks for this event */
    uint32_t signal_set = 0; /* Indicated which signal is set */
    int32_t result = 0;

    /* Clear all signals before waiting */
	utility_signals = WPS_EVENT_MASK | P2P_EVENT_MASK;

    for(;;)
    {
        result = qurt_signal_wait_timed(&Utility_Event, utility_signals,
                                        (QURT_SIGNAL_ATTR_WAIT_ANY |
                                         QURT_SIGNAL_ATTR_CLEAR_MASK),
                                        &signal_set, QURT_TIME_WAIT_FOREVER);

        if(result != QURT_EOK)
        {
             /* This case will be true when qurt_signal_destroy() is called. */
             break;
        }

		/* Process WPS profile event */
        if (signal_set & WPS_EVENT_MASK)
        {
            wps_query(0);
        }

        /* Process the P2P event queue. */
        if (signal_set & P2P_EVENT_MASK)
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

    qurt_mutex_lock(&UtilityMutex);
    Utility_Thread_Created = 0;
    Utility_Event_Initialized = 0;
    qurt_mutex_unlock(&UtilityMutex);
    qurt_mutex_destroy(&UtilityMutex); /* Destroy the mutex as it is not needed */

    /* Stop the utility thread. Memory allocated for this thread will be deleted */
    qurt_thread_stop();
    return;
}

int Create_Wlan_Utility_Thread ()
{
    int Utility_Thread_Result = 0;

    /* Checks for Utility_Event_Initialized and Utility_Thread_Created are added to
     * avoid thread and event creation more than once if 'wlan enable' command is
     * given more than once. These are reset when the thread and event are destroyed. */
    if (!Utility_Event_Initialized && !Utility_Thread_Created)
    {
        /* Initialize the utility thread event. */
		qurt_mutex_init(&UtilityMutex);
    }

	qurt_mutex_lock(&UtilityMutex);
    if (!Utility_Event_Initialized)
    {
        qurt_signal_init(&Utility_Event);
        Utility_Event_Initialized = 1;
    }

    if(!Utility_Thread_Created)
    {
		/* Create a thread to process WPS and P2P events. */
		qurt_thread_attr_init(&Utility_Thread_Attribute);
		qurt_thread_attr_set_name(&Utility_Thread_Attribute, "WlanUtil");
		qurt_thread_attr_set_priority(&Utility_Thread_Attribute, UTILITY_THREAD_PRIORITY);
		qurt_thread_attr_set_stack_size(&Utility_Thread_Attribute, UTILITY_THREAD_STACK_SIZE);
		Utility_Thread_Result = qurt_thread_create(&Utility_Thread_Handle,
                                                   &Utility_Thread_Attribute,
                                                   Wlan_Utility_Thread, NULL);

        if(Utility_Thread_Result != QURT_EOK)
        {
            /* Thread creation failed. Destroy the signal and lock to
             * avoid memory leak */
            QCLI_Printf(qcli_wlan_group, "Utility thread creation failed.\n");
            qurt_signal_destroy(&Utility_Event);
            Utility_Event_Initialized = 0;

			qurt_mutex_unlock(&UtilityMutex);
            qurt_mutex_destroy(&UtilityMutex);
            return -1;
        }
        Utility_Thread_Created = 1;
    }

	qurt_mutex_unlock(&UtilityMutex);
    return 0;
}

void Destroy_Wlan_Utility_Thread()
{
    /* Following check is added to avoid accessing an uninitialized signal and thread */
    if (!Utility_Thread_Created || !Utility_Event_Initialized)
    {
        return;
    }

    /* qurt_signal_destroy() will wake up the threads waiting on Utility_Event.
     * Wlan_Utility_Thread() exits the thread after this. */
    qurt_signal_destroy(&Utility_Event);
    return;
}


/*FUNCTION*-----------------------------------------------------------------
*
* Function Name  : deviceid_handler
* Returned Value : -1 on error else 0
* Comments       : Sets the Device ID in the application.
*
*END------------------------------------------------------------------*/
int32_t set_active_deviceid(uint16_t deviceId)
{
    if(wps_context.wps_in_progress)
    {
       QCLI_Printf(qcli_wlan_group, "wps sesion in progress switching device not allowed:\r\n");
       return -1;
    }

    if (0 == wlan_enabled)
    {
        QCLI_Printf(qcli_wlan_group, "Enable WLAN before setting the device ID\r\n");
        return -1;
    }

    active_device = deviceId;
    return 0;
}


int32_t get_active_device()
{
    return active_device;
}

/*FUNCTION*-----------------------------------------------------------------
*
* Function Name  : set_callback
* Returned Value : -1 on error else 0
* Comments       : Sets callback function for WiFi connect/disconnect event
*
*END------------------------------------------------------------------*/

uint32_t set_callback(const void *applicationContext)
{
    int deviceId = get_active_device();
    return(qapi_WLAN_Set_Callback(deviceId, (qapi_WLAN_Callback_t)wlan_callback_handler, applicationContext));
}

int32_t enable_wlan()
{
    int i;

    if (wlan_enabled) {
        return 0;
    }

    wlan_dbg_init(qcli_wlan_group, QCLI_Printf);

    if (qapi_WLAN_Enable(QAPI_WLAN_ENABLE_E) != QAPI_OK) {
        return -1;
    }
	
	/* Due to bugs in Iniche stack, the second device is created statically in
	 * MCC or SCC is enabled
	 */
	for(i=0;i < WLAN_NUM_OF_DEVICES;i++)
	{
		qapi_WLAN_Add_Device(i);
	}
    
#ifdef CONFIG_WLAN_8021X
    if (qapi_WLAN_8021x_Enable(QAPI_WLAN_8021X_ENABLE_E) != QAPI_OK) {
        goto wlan_8021x_fail;
    }
#endif

	set_callback(NULL);

	if (0 != Create_Wlan_Utility_Thread())
	{
	    goto Wlan_Utility_fail;
	}

    wlan_enabled = 1;
    return 0;

Wlan_Utility_fail:

#ifdef CONFIG_WLAN_8021X
    qapi_WLAN_8021x_Enable(QAPI_WLAN_8021X_DISABLE_E);
wlan_8021x_fail:
#endif

    for(i=0;i < WLAN_NUM_OF_DEVICES;i++)
    {
        qapi_WLAN_Remove_Device(i);
    }

    qapi_WLAN_Enable(QAPI_WLAN_DISABLE_E);
    wlan_dbg_exit();
    return -1;
}

int disable_wlan()
{
    int i;
    qapi_Net_Socket_Status_t socket_status;

    if (0 == wlan_enabled) {
        return 0;
    }

#ifdef CONFIG_WLAN_8021X
    qapi_WLAN_8021x_Enable(QAPI_WLAN_8021X_DISABLE_E);
#endif

    if (qapi_Net_Get_Socket_Status(&socket_status) == QAPI_OK)
    {
        if (socket_status.open_af_inet > 0)
        {
            QCLI_Printf(qcli_wlan_group, "ERROR: Cannot disable WLAN while AF_INET sockets are open.\n");
            return -2;
        }
    }

    /* Remove the WLAN device from network interface before disabling WLAN */
    for(i=0;i < WLAN_NUM_OF_DEVICES;i++)
    {
        qapi_WLAN_Remove_Device(i);
    }

    if (0 == qapi_WLAN_Enable(QAPI_WLAN_DISABLE_E))
    {
        Destroy_Wlan_Utility_Thread();
		wlan_dbg_exit();
        /*Delay 300ms to ensure that KF gets ready before next 'wlan enable' without delay.*/
        qosal_time_delay(300);
        wlan_enabled = 0;
		wifi_state[0] = 0;
		wifi_state[1] = 0;
        return 0;
    }
    return -1;
}

#define APPEND_PAYLOAD_IN_PROMISCUOUS 0x8000

void promisc_frame_handler(void *payload, uint32_t length)
{
      uint16_t i = 0, j = 0, print_length = 0;
      uint8_t *ptr = (uint8_t *)payload;
	  /* If the length>0x8000, it means that this payload is part of the frame and it has no 802.11 header*/
	  if(length & APPEND_PAYLOAD_IN_PROMISCUOUS)
	  {
	  	length &= ~APPEND_PAYLOAD_IN_PROMISCUOUS;
		QCLI_Printf(qcli_wlan_group, "this frame should be append to the last frame\r\n");
	  }
      if( length > 64 )
          print_length = 64;
      else
          print_length = length;
      
      QCLI_Printf(qcli_wlan_group, "frame (%d):\r\n", length);
      /* only print the first 64 bytes of each frame */
      for(i = 0 ; i < print_length ; i++){
        QCLI_Printf(qcli_wlan_group, "0x%02x, ", ptr[i]);
        if(j++==7){
          j=0;
          QCLI_Printf(qcli_wlan_group, "\r\n");
        }
      }
      if(j){
        QCLI_Printf(qcli_wlan_group, "\r\n");
      }
}

int32_t set_mac (char *mac_addr)
{
   uint32_t deviceId = get_active_device();
   if (0 == qapi_WLAN_Set_Param(deviceId,
                                       __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                                       __QAPI_WLAN_PARAM_GROUP_WIRELESS_MAC_ADDRESS, 
                                       mac_addr,
                                       strlen(mac_addr),
                                       0))
   {
       return 0;
   }
   return -1;
}

void print_wlan_channel_list(qapi_WLAN_Get_Channel_List_t *wlanChannelList)
{

     int i;
     uint16_t channel;
	 QCLI_Printf(qcli_wlan_group, "\r\n CHANNEL LIST");
     for( i = 0; i < wlanChannelList->number_Of_Channels; i++)
     {

		if (wlanChannelList->channel_List[i] !=0)
		{
	       channel = freq_to_channel(wlanChannelList->channel_List[i]);		
		   QCLI_Printf(qcli_wlan_group, "\r\r\n Channel: %d", channel);
		}
		
     }

}

int32_t print_wlan_stats( qapi_WLAN_Statistics_t *stats )
{
    //qapi_WLAN_Statistics_t *pWlanStats = (qapi_WLAN_Statistics_t *)stats;
    qapi_WLAN_Device_Stats_t *pDevWlanStats = &stats->dev_Stats;
    qapi_WLAN_Common_Stats_t *pCommonWlanStats = &stats->common_Stats;
	qapi_WLAN_Common_TxRx_Buffer_Info *pTxrxBufferInfo = &(pCommonWlanStats->txrx_buffer_info);
	qapi_WLAN_Device_Stats_Ext_t *pDevWlanStatsExt = &stats->dev_Stats_Ext;
    qapi_WLAN_Device_Stats_Ext2_t *pDevWlanStatsExt2 = &stats->dev_Stats_Ext2;

    QCLI_Printf(qcli_wlan_group, " ======== WLAN STATISTICS ========= \r\n");

    QCLI_Printf(qcli_wlan_group, " ***** Device Specific WLAN STATISTICS ***** \r\n");
    
    QCLI_Printf(qcli_wlan_group, "\r\n unicast_Tx_Pkts =0x%x", pDevWlanStats->unicast_Tx_Pkts);
    QCLI_Printf(qcli_wlan_group, "\r\n unicast_Rx_Pkts = 0x%x", pDevWlanStats->unicast_Rx_Pkts);
    QCLI_Printf(qcli_wlan_group, "\r\n multicast_Tx_Pkts = 0x%x", pDevWlanStats->multicast_Tx_Pkts);
    QCLI_Printf(qcli_wlan_group, "\r\n multicast_Rx_Pkts = 0x%x", pDevWlanStats->multicast_Rx_Pkts);
    QCLI_Printf(qcli_wlan_group, "\r\n broadcast_Tx_Pkts = 0x%x", pDevWlanStats->broadcast_Tx_Pkts);
    QCLI_Printf(qcli_wlan_group, "\r\n broadcast_Rx_Pkts = 0x%x", pDevWlanStats->broadcast_Rx_Pkts);

    QCLI_Printf(qcli_wlan_group, "\r\n unicast_non_null_Tx_Pkts =0x%x", pDevWlanStats->unicast_Tx_Pkts);
    QCLI_Printf(qcli_wlan_group, "\r\n unicast_non_null_Rx_Pkts = 0x%x", pDevWlanStats->unicast_Rx_Pkts);    
  
    QCLI_Printf(qcli_wlan_group, "\r\n unicast_Filtered_Accepted_Tx_Pkts = 0x%x", pDevWlanStats->unicast_Filtered_Accepted_Tx_Pkts);
    QCLI_Printf(qcli_wlan_group, "\r\n unicast_Filtered_Accepted_Rx_Pkts = 0x%x", pDevWlanStats->unicast_Filtered_Accepted_Rx_Pkts);
    QCLI_Printf(qcli_wlan_group, "\r\n multicast_Filtered_Accepted_Tx_Pkts = 0x%x", pDevWlanStats->multicast_Filtered_Accepted_Tx_Pkts);
    QCLI_Printf(qcli_wlan_group, "\r\n multicast_Filtered_Accepted_Rx_Pkts = 0x%x", pDevWlanStats->multicast_Filtered_Accepted_Rx_Pkts);
    QCLI_Printf(qcli_wlan_group, "\r\n broadcast_Filtered_Accepted_Tx_Pkts = 0x%x", pDevWlanStats->broadcast_Filtered_Accepted_Tx_Pkts);
    QCLI_Printf(qcli_wlan_group, "\r\n broadcast_Filtered_Accepted_Rx_Pkts = 0x%x", pDevWlanStats->broadcast_Filtered_Accepted_Rx_Pkts);

    QCLI_Printf(qcli_wlan_group, "\r\n unicast_Filtered_Rejected_Tx_Pkts = 0x%x", pDevWlanStats->unicast_Filtered_Rejected_Tx_Pkts);
    QCLI_Printf(qcli_wlan_group, "\r\n unicast_Filtered_Rejected_Rx_Pkts = 0x%x", pDevWlanStats->unicast_Filtered_Rejected_Rx_Pkts);
    QCLI_Printf(qcli_wlan_group, "\r\n multicast_Filtered_Rejected_Tx_Pkts = 0x%x", pDevWlanStats->multicast_Filtered_Rejected_Tx_Pkts);
    QCLI_Printf(qcli_wlan_group, "\r\n multicast_Filtered_Rejected_Rx_Pkts = 0x%x", pDevWlanStats->multicast_Filtered_Rejected_Rx_Pkts);
    QCLI_Printf(qcli_wlan_group, "\r\n broadcast_Filtered_Rejected_Tx_Pkts = 0x%x", pDevWlanStats->broadcast_Filtered_Rejected_Tx_Pkts);
    QCLI_Printf(qcli_wlan_group, "\r\n broadcast_Filtered_Rejected_Rx_Pkts = 0x%x", pDevWlanStats->broadcast_Filtered_Rejected_Rx_Pkts);

    QCLI_Printf(qcli_wlan_group, "\r\n null_Tx_Pkts = 0x%x", pDevWlanStats->null_Tx_Pkts);
    QCLI_Printf(qcli_wlan_group, "\r\n null_Rx_Pkts = 0x%x", pDevWlanStats->null_Rx_Pkts);
    QCLI_Printf(qcli_wlan_group, "\r\n qos_null_Tx_Pkts = 0x%x", pDevWlanStats->qos_Null_Tx_Pkts);
    QCLI_Printf(qcli_wlan_group, "\r\n qos_null_Rx_Pkts = 0x%x", pDevWlanStats->qos_Null_Rx_Pkts);
    QCLI_Printf(qcli_wlan_group, "\r\n ps_poll_Tx_Pkts = 0x%x", pDevWlanStats->ps_Poll_Tx_Pkts);
    QCLI_Printf(qcli_wlan_group, "\r\n ps_poll_Rx_Pkts = 0x%x", pDevWlanStats->ps_Poll_Rx_Pkts);
    QCLI_Printf(qcli_wlan_group, "\r\n tx_retry_Cnt = 0x%x", pDevWlanStats->tx_Retry_Cnt);

    QCLI_Printf(qcli_wlan_group, "\r\n beacon_miss_Cnt = 0x%x", pDevWlanStats->beacon_Miss_Cnt);
    QCLI_Printf(qcli_wlan_group, "\r\n beacons_received_Cnt = 0x%x", pDevWlanStats->beacons_Received_Cnt);
    QCLI_Printf(qcli_wlan_group, "\r\n beacon_resync_success_Cnt = 0x%x", pDevWlanStats->beacon_Resync_Success_Cnt);
    QCLI_Printf(qcli_wlan_group, "\r\n beacon_resync_failure_Cnt = 0x%x", pDevWlanStats->beacon_Resync_Failure_Cnt); 
    QCLI_Printf(qcli_wlan_group, "\r\n curr_early_wakeup_adj_in_ms = 0x%x", pDevWlanStats->curr_Early_Wakeup_Adj_In_Ms);
    QCLI_Printf(qcli_wlan_group, "\r\n avg_early_wakeup_adj_in_ms = 0x%x", pDevWlanStats->avg_Early_Wakeup_Adj_In_Ms);
    QCLI_Printf(qcli_wlan_group, "\r\n early_termination_Cnt = 0x%x", pDevWlanStats->early_Termination_Cnt);

    QCLI_Printf(qcli_wlan_group, "\r\n uapsd_trigger_Rx_Cnt = 0x%x", pDevWlanStats->uapsd_Trigger_Rx_Cnt);
    QCLI_Printf(qcli_wlan_group, "\r\n uapsd_trigger_Tx_Cnt = 0x%x", pDevWlanStats->uapsd_Trigger_Tx_Cnt);
	QCLI_Printf(qcli_wlan_group, "\r\n rx_amsdu_Pkts =0x%x", pDevWlanStatsExt->rx_amsdu_pkts);
    QCLI_Printf(qcli_wlan_group, "\r\n wmi_event_missed_last =%d", pDevWlanStatsExt2->wmi_event_missed_last);
    QCLI_Printf(qcli_wlan_group, "\r\n wmi_event_missed_bitmap =%d", pDevWlanStatsExt2->wmi_event_missed_bitmap);
    QCLI_Printf(qcli_wlan_group, "\r\n wmi_event_missed_cnt =%d", pDevWlanStatsExt2->wmi_event_missed_cnt);
	
    QCLI_Printf(qcli_wlan_group, "\r\n\n **** COMMON WLAN Statistics **** " );
    
    QCLI_Printf(qcli_wlan_group, "\r\n total_active_time_in_ms = 0x%x", pCommonWlanStats->total_Active_Time_In_Ms);
    QCLI_Printf(qcli_wlan_group, "\r\n total_powersave_time_in_ms = 0x%x", pCommonWlanStats->total_Powersave_Time_In_Ms);
	QCLI_Printf(qcli_wlan_group, "\r\n WLAN TXRX Buffer Information");
	QCLI_Printf(qcli_wlan_group, "\r\n HTC (%d + %d)", pTxrxBufferInfo->htc_inf_cur_cnt, pTxrxBufferInfo->htc_inf_reaped_cnt);
	QCLI_Printf(qcli_wlan_group, " MAC (%d + %d)", pTxrxBufferInfo->mac_inf_cur_cnt, pTxrxBufferInfo->mac_inf_reaped_cnt);
	QCLI_Printf(qcli_wlan_group, " FW (%d + %d)", pTxrxBufferInfo->fw_inf_cur_cnt, pTxrxBufferInfo->fw_inf_reaped_cnt);
	QCLI_Printf(qcli_wlan_group, " FreeBufCnt (%d) MgmtCnt(%d) SmMgmtCnt(%d)", pTxrxBufferInfo->free_buf_cnt, pTxrxBufferInfo->mgmt_buf_cnt, pTxrxBufferInfo->smmgmt_buf_cnt);
	QCLI_Printf(qcli_wlan_group, " QueuedBufCnt (TX: %d RX: %d)", pTxrxBufferInfo->num_txbuf_queued, pTxrxBufferInfo->num_rxbuf_queued);
    QCLI_Printf(qcli_wlan_group, "\r\n\n");

    return 0;
}

int32_t print_scan_results(qapi_WLAN_Scan_List_t param, uint16_t num_scan)
{

    int32_t i = 0;
    uint8_t temp_ssid[33] = {0};
    qapi_WLAN_BSS_Scan_Info_t *list = NULL;

    //QCLI_Printf(qcli_wlan_group, "Scan result count:%d\r\n", num_scan);
    list = (qapi_WLAN_BSS_Scan_Info_t *)(param.scan_List);     
    for (i = 0;i<num_scan;i++)
    {
        memcpy(temp_ssid,list[i].ssid,list[i].ssid_Length);
        temp_ssid[list[i].ssid_Length] = '\0';
        if (list[i].ssid_Length == 0)
        {                           
            QCLI_Printf(qcli_wlan_group, "ssid = SSID Not available\r\n\n");
        }
        else 
        {
#if ENABLE_SCC_MODE 
            if((concurrent_connect_flag == 0x0F) && (strcmp((char const *)ssid_str_concurrent, (char const *)temp_ssid) == 0))
            {
                ap_channel_hint = list[i].channel;
                QCLI_Printf(qcli_wlan_group, "scan res ch : %d \r\n", ap_channel_hint);
                return 0;
            }
            else if((concurrent_connect_flag == 0x0F))
            {
                return 0;
            }
            else 
#endif
            {
                QCLI_Printf(qcli_wlan_group, "ssid = %s\r\n",temp_ssid);
                QCLI_Printf(qcli_wlan_group, "bssid = %.2x:%.2x:%.2x:%.2x:%.2x:%.2x\r\n",list[i].bssid[0],list[i].bssid[1],list[i].bssid[2],list[i].bssid[3],list[i].bssid[4],list[i].bssid[5]);
                QCLI_Printf(qcli_wlan_group, "channel = %d\r\n",list[i].channel);
                QCLI_Printf(qcli_wlan_group, "indicator = %d\r\n",list[i].rssi);
                QCLI_Printf(qcli_wlan_group, "security = ");
                if(list[i].security_Enabled){
                    if(list[i].rsn_Auth || list[i].rsn_Cipher){
                        QCLI_Printf(qcli_wlan_group, "\r\n\r");
                        QCLI_Printf(qcli_wlan_group, "RSN/WPA2= ");
                    }
                    if(list[i].rsn_Auth){
                        QCLI_Printf(qcli_wlan_group, " {");
                        if(list[i].rsn_Auth & __QAPI_WLAN_SECURITY_AUTH_1X){
                             QCLI_Printf(qcli_wlan_group, "802.1X ");
                    }
                 if(list[i].rsn_Auth & __QAPI_WLAN_SECURITY_AUTH_PSK){
                     QCLI_Printf(qcli_wlan_group, "PSK ");
                 }
                 QCLI_Printf(qcli_wlan_group, "}");
            }
            if(list[i].rsn_Cipher){
                QCLI_Printf(qcli_wlan_group, " {");
                /* AP security can support multiple options hence
                 * we check each one separately. Note rsn == wpa2 */
                  if(list[i].rsn_Cipher & __QAPI_WLAN_CIPHER_TYPE_WEP){
                      QCLI_Printf(qcli_wlan_group, "WEP ");
                  }

                   if(list[i].rsn_Cipher & __QAPI_WLAN_CIPHER_TYPE_TKIP){
                        QCLI_Printf(qcli_wlan_group, "TKIP ");
                   }
                   if(list[i].rsn_Cipher & __QAPI_WLAN_CIPHER_TYPE_CCMP){
                        QCLI_Printf(qcli_wlan_group, "AES ");
                   }
                        QCLI_Printf(qcli_wlan_group, "}");
            }
            if(list[i].wpa_Auth || list[i].wpa_Cipher){
                QCLI_Printf(qcli_wlan_group, "\r\n\r");
                QCLI_Printf(qcli_wlan_group, "WPA= ");
            }
            if(list[i].wpa_Auth){
                 QCLI_Printf(qcli_wlan_group, " {");
                 if(list[i].wpa_Auth & __QAPI_WLAN_SECURITY_AUTH_1X){
                     QCLI_Printf(qcli_wlan_group, "802.1X ");
                 }
                 if(list[i].wpa_Auth & __QAPI_WLAN_SECURITY_AUTH_PSK){
                     QCLI_Printf(qcli_wlan_group, "PSK ");
                 }
                 QCLI_Printf(qcli_wlan_group, "}");
            }

            if(list[i].wpa_Cipher){
                QCLI_Printf(qcli_wlan_group, " {");
                if(list[i].wpa_Cipher & __QAPI_WLAN_CIPHER_TYPE_WEP){
                    QCLI_Printf(qcli_wlan_group, "WEP ");
                }

                if(list[i].wpa_Cipher & __QAPI_WLAN_CIPHER_TYPE_TKIP){
                    QCLI_Printf(qcli_wlan_group, "TKIP ");
                }
    
                if(list[i].wpa_Cipher & __QAPI_WLAN_CIPHER_TYPE_CCMP){
                    QCLI_Printf(qcli_wlan_group, "AES ");
                }
                QCLI_Printf(qcli_wlan_group, "}");
            }
            /* it may be old-fashioned WEP this is identified by
             * absent wpa and rsn ciphers */
             if(list[i].rsn_Cipher == 0 &&
                list[i].wpa_Cipher == 0){
                     QCLI_Printf(qcli_wlan_group, "WEP ");
             }
        }else{
              QCLI_Printf(qcli_wlan_group, "NONE! ");
         }
       }
     }

         if(i!= param.num_Scan_Entries-1)
         {
             QCLI_Printf(qcli_wlan_group, "\r\n ");
             QCLI_Printf(qcli_wlan_group, "\r\n \r");
         }
         else
         {
              QCLI_Printf(qcli_wlan_group, "\r\nshell> ");
         }
     }
     return 0;
}

int32_t wmi_set_scan(uint32_t dev_num, void *start_scan, qapi_WLAN_Store_Scan_Results_e store_scan_results)
{
    int32_t error = 0;
    qapi_WLAN_Scan_List_t param;
    
    set_callback(NULL);
    // if (dev_num < BSP_ENET_DEVICE_COUNT)
    {
        qapi_WLAN_Store_Scan_Results_e storeScanResults = (qapi_WLAN_Store_Scan_Results_e) store_scan_results; //QAPI_WLAN_BUFFER_SCAN_RESULTS_BLOCKING_E;
        error = qapi_WLAN_Start_Scan(dev_num, (qapi_WLAN_Start_Scan_Params_t *) start_scan, storeScanResults); 
        if (0 != error)
        {
            return error;
        }

        if((store_scan_results == QAPI_WLAN_BUFFER_SCAN_RESULTS_NON_BLOCKING_E) || (store_scan_results == QAPI_WLAN_NO_BUFFERING_E))
            return error;

        param.scan_List = malloc((sizeof(qapi_WLAN_BSS_Scan_Info_t) * __QAPI_MAX_SCAN_RESULT_ENTRY));
        param.num_Scan_Entries = __QAPI_MAX_SCAN_RESULT_ENTRY;
        error = qapi_WLAN_Get_Scan_Results(dev_num, (qapi_WLAN_BSS_Scan_Info_t*)(param.scan_List), (int16_t *)&(param.num_Scan_Entries));

		if (0 != error)
        {
            QCLI_Printf(qcli_wlan_group,"Require to scan again.\r\n");
			free(param.scan_List);
			return error;
        }
		
        print_scan_results(param, param.num_Scan_Entries);
    	QCLI_Printf(qcli_wlan_group, "Scan result count:%d\r\n", param.num_Scan_Entries);

        /* Free the scan result memory allocated by qapi_WLAN_Get_Scan_Results */
        free(param.scan_List);
    }    
    return error;
}


uint32_t set_power_mode(boolean_t pwr_mode, uint8_t pwr_module)
{
    uint32_t deviceId = get_active_device();
    qapi_WLAN_Power_Mode_Params_t pwrMode;

    pwrMode.power_Mode = pwr_mode;
    pwrMode.power_Module = pwr_module;
    return qapi_WLAN_Set_Param (deviceId,
                               __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                               __QAPI_WLAN_PARAM_GROUP_WIRELESS_POWER_MODE_PARAMS,
                               (void *) &pwrMode,
                               sizeof(pwrMode),
                               FALSE);

}

/*FUNCTION*-----------------------------------------------------------------
*
* Function Name  : mystrtomac
* Returned Value : NA
* Comments       : converts a string to a 6 byte MAC address.
*
*END------------------------------------------------------------------*/
static int mystrtomac(const char* arg, unsigned char* dst)
{
    int i = 0, j = 0, left_nibble = 1;
    char base_char;
    unsigned char base_val;

    if(dst == NULL || arg == NULL){
        return -1;
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
            return -1;//error
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

    return 0;//success
}

/*FUNCTION*---------------------------------------------------------------------------------
* Function Name  : get_wifi_power_mode
* Returned Value : 0 on success, -1 on error.
* Comments       : Called from application to retrieve WiFi power mode for active device.
*END----------------------------------------------------------------------------------------*/
int32_t get_wifi_power_mode()
{
    uint32_t mode = 0, deviceId = 0, dataLen = 0;
	char data[32+1] = {'\0'};

    deviceId = get_active_device();
    if (0 != qapi_WLAN_Get_Param (deviceId, 
                         __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                         __QAPI_WLAN_PARAM_GROUP_WIRELESS_POWER_MODE_PARAMS,
                         &mode,
                         &dataLen))
    {
        QCLI_Printf(qcli_wlan_group, "Command failed.\r\n");
        return -1;
	}
	if(mode == QAPI_WLAN_POWER_MODE_REC_POWER_E)
	{
		strcpy(data,"Power Save");
	}
	else
	{
		strcpy(data,"Max Perf");
	}
	QCLI_Printf(qcli_wlan_group, "Power mode   =   %s\r\n",data);
    return 0;
}

/*FUNCTION*---------------------------------------------------------------------------------
* Function Name  : get_phy_mode
* Returned Value : 0 on success, -1 on error.
* Comments       : Called from application to retrieve device's wireless mode.
*END----------------------------------------------------------------------------------------*/
int32_t get_phy_mode()
{
    uint32_t deviceId = 0, dataLen = 0;
    qapi_WLAN_Phy_Mode_e phyMode;
	char data[32+1] = {'\0'};
	char ht[10] = {'\0'};
	qapi_WLAN_11n_HT_Config_e htconfig;

    deviceId = get_active_device();
    if (0 != qapi_WLAN_Get_Param (deviceId,
                         __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                         __QAPI_WLAN_PARAM_GROUP_WIRELESS_PHY_MODE,
                         &phyMode,
                         &dataLen))
    {
        QCLI_Printf(qcli_wlan_group, "Command failed.\r\n");
        return -1;
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
	QCLI_Printf(qcli_wlan_group, "Phy mode	   =   %s",data);

    if (phyMode != QAPI_WLAN_11B_MODE_E) {
        qapi_WLAN_Get_Param (deviceId, 
                             __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                             __QAPI_WLAN_PARAM_GROUP_WIRELESS_11N_HT,
                             &htconfig,
                             &dataLen);
        QCLI_Printf(qcli_wlan_group, "(HT:");
        if (htconfig == QAPI_WLAN_11N_DISABLED_E) {
            strcpy(ht, "off");
        } else if (htconfig == QAPI_WLAN_11N_HT20_E) {
            strcpy(ht, "20M");
        } else if (htconfig == QAPI_WLAN_11N_HT40_E) {
            strcpy(ht, "40M");
        }
        QCLI_Printf(qcli_wlan_group, "HT MODE %s)", ht);
    }
    QCLI_Printf(qcli_wlan_group, "\r\n");
    return 0;
}

/*FUNCTION*---------------------------------------------------------------------------------------------------
*
* Function Name  : get_ssid
* Returned Value : 0 on success, -1 on error.
* Comments       : Called from application to retrieve SSID of the network if the active device is connected.
*
*END----------------------------------------------------------------------------------------------------------*/
int32_t get_ssid()
{
    char data[32+1] = {'\0'};
    uint32_t deviceId = 0, dataLen = 0;

    deviceId = get_active_device();
    if (0 != qapi_WLAN_Get_Param (deviceId,
                                 __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                                 __QAPI_WLAN_PARAM_GROUP_WIRELESS_SSID,
                                 (void *) data,
                                 &dataLen))
    {
        QCLI_Printf(qcli_wlan_group, "Command failed.\r\n");
        return -1;
    }

    if((get_dev_stat(deviceId) == UP) && (*data != '\0'))
    {
        QCLI_Printf(qcli_wlan_group, "ssid         =   %s\r\n",data);
    }
	else
    {
        QCLI_Printf(qcli_wlan_group, "ssid = Device not connected\r\n");		
    }
    return 0;
}

/*FUNCTION*-----------------------------------------------------------------------------------------
*
* Function Name  : get_device_mac_address
* Returned Value : 0 on success, -1 on error.
* Comments       : Called from application to retrieve MAC address for the active device.
*
*END------------------------------------------------------------------------------------------------*/
int32_t get_device_mac_address()
{
    char data[32+1] = {'\0'};
    uint32_t deviceId = 0, dataLen = 0;

    deviceId = get_active_device();
    if (0 != qapi_WLAN_Get_Param (deviceId, 
                         __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                         __QAPI_WLAN_PARAM_GROUP_WIRELESS_MAC_ADDRESS,
                         &data,
                         &dataLen))
    {
        QCLI_Printf(qcli_wlan_group, "Command failed.\r\n");
        return -1;
    }
       
    QCLI_Printf(qcli_wlan_group, "Mac Addr     =   %02x:%02x:%02x:%02x:%02x:%02x \r\n", data[0], data[1], data[2], data[3], data[4], data[5]);
    return 0;
}

/*FUNCTION*----------------------------------------------------------------------------------
*
* Function Name  : get_op_mode
* Returned Value : 0 on success, -1 on error.
* Comments       : Called from application to retrieve operating mode for the active device.
*
*END-----------------------------------------------------------------------------------------*/
int32_t get_op_mode()
{
    uint32_t mode = 0, deviceId = 0, dataLen = 0;

    deviceId = get_active_device();
    if (0 != qapi_WLAN_Get_Param (deviceId, 
                         __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                         __QAPI_WLAN_PARAM_GROUP_WIRELESS_OPERATION_MODE,
                         &mode,
                         &dataLen))
    {
        QCLI_Printf(qcli_wlan_group, "Command failed.\r\n");
        return -1;
    }

#if ENABLE_AP_MODE
    if(mode == QAPI_WLAN_DEV_MODE_AP_E)
    {
		QCLI_Printf(qcli_wlan_group, "mode         =   softap \r\n");
        if(pmk_flag[deviceId])
        {
			QCLI_Printf(qcli_wlan_group, "passphrase   =   %s \r\n",wpa_passphrase[deviceId]);
        }
    }
    else
#endif
    if(mode == QAPI_WLAN_DEV_MODE_STATION_E)
    {
        QCLI_Printf(qcli_wlan_group, "mode         =   station \r\n");
    }
    else
    {
        QCLI_Printf(qcli_wlan_group, "mode	       =   adhoc \r\n");
    }
    return 0;
}

/*FUNCTION*----------------------------------------------------------------------------------------
*
* Function Name  : get_channel
* Returned Value : 0 on success, -1 on error.
* Comments       : Called from application to retrieve channel for the active device if connected.
*
*END-----------------------------------------------------------------------------------------------*/
int32_t get_channel()
{
    uint16_t channel_val = 0;
    uint32_t deviceId = 0, dataLen = 0;

    deviceId = get_active_device();
    if(get_dev_stat(deviceId) == UP)
    {
        if (0 != qapi_WLAN_Get_Param (deviceId, 
                             __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                             __QAPI_WLAN_PARAM_GROUP_WIRELESS_CHANNEL,
                             &channel_val,
                             &dataLen))
        {
            QCLI_Printf(qcli_wlan_group, "Command failed.\r\n");
            return -1;
        } else {
            QCLI_Printf(qcli_wlan_group, "channel      =   %d \r\n", channel_val);
            return 0;
        }
    }
    else
    {
        QCLI_Printf(qcli_wlan_group, "Device is down.\r\n");
        return 0;
    }
}

/*FUNCTION*--------------------------------------------------------------------------
*
* Function Name  : print_wlan_info
* Returned Value : 0 on success, -1 on error.
* Comments       : Called from application to retrieve information for active device.
*
*END---------------------------------------------------------------------------------*/
void print_wlan_info()
{
    uint32_t deviceId = 0;

    deviceId = get_active_device();
    if(get_dev_stat(deviceId) == UP)
    {
        get_ssid();
        get_channel();
    }

    get_phy_mode();
    get_wifi_power_mode();
    get_device_mac_address();
    get_op_mode();
    return;
}

/*FUNCTION*-----------------------------------------------------------------
*
* Function Name  : get_wep_key
* Returned Value : -1 on error else 0
* Comments       : Get WEP key at given index.
*                  Hex characters
*
*END------------------------------------------------------------------*/
int32_t get_wep_key(uint8_t index)
{
    uint32_t error = 0, deviceId = 0, dataLen = 0, hex_i = 0, ascii_i = 0;
    qapi_WLAN_Security_Wep_Key_Pair_Params_t keyPair;
    char get_key[__QAPI_WLAN_MAX_WEP_KEY_SZ+1] = {'\0'};
	
    keyPair.key_Index = index;
    keyPair.key_Length = 0;
    keyPair.key = (int8_t *)malloc(__QAPI_WLAN_MAX_WEP_KEY_SZ+1);
    memset(keyPair.key, 0, __QAPI_WLAN_MAX_WEP_KEY_SZ+1);
    memset(get_key, 0, __QAPI_WLAN_MAX_WEP_KEY_SZ+1);

    deviceId = get_active_device();
    error = qapi_WLAN_Get_Param (deviceId, __QAPI_WLAN_PARAM_GROUP_WIRELESS_SECURITY,
                                 __QAPI_WLAN_PARAM_GROUP_SECURITY_WEP_KEY_PAIR, 
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

    QCLI_Printf(qcli_wlan_group, "Key: %s\r\n", get_key);
    free(keyPair.key);
    return 0;
}

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : get_wep_keyix()
* Returned Value  : 0 - successful completion or
*                   -1 - failed.
* Comments    : Get index of default WEP key.
*
*END*-----------------------------------------------------------------*/
int32_t get_wep_keyix()
{
    uint32_t error = 0, keyIndex = 0, deviceId = 0, dataLen = 0;
    deviceId = get_active_device();
    error = qapi_WLAN_Get_Param (deviceId,
                                 __QAPI_WLAN_PARAM_GROUP_WIRELESS_SECURITY,
                                 __QAPI_WLAN_PARAM_GROUP_SECURITY_WEP_KEY_INDEX,
                                 &keyIndex, &dataLen);
    if(error != 0)
    {
        return error;
    }
    QCLI_Printf(qcli_wlan_group, "Key index: %d\n", keyIndex);    
    return 0;
}

/*FUNCTION*-----------------------------------------------------------------
*
* Function Name  : wifi_connect_handler
* Returned Value : N/A
* Comments       : Called from driver on a WiFI connection event
*
*END------------------------------------------------------------------*/

void wifi_connect_handler(int val, uint8_t devId,uint8_t * mac,boolean_t bssConn)
{
    uint8_t disc_bss[6] ={0xff,0xff,0xff,0xff,0xff,0xff};
	uint8_t temp_bss[__QAPI_WLAN_MAC_LEN] ={0};
	
	if(devId < DEV_NUM)	 
		wifi_state[devId] = val;
	else
		 QCLI_Printf(qcli_wlan_group, "The devid exceeds the max device number.\r\n");
	
    if(val == TRUE)
    {
        QCLI_Printf(qcli_wlan_group, "devid - %d %d %s MAC addr %02x:%02x:%02x:%02x:%02x:%02x \r\n",
              devId, bssConn,"CONNECTED", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        if(bssConn ){
          set_dev_stat(devId,UP);
          concurrent_connect_flag = 0x0E;
		  memcpy(g_bssid[devId],mac,__QAPI_WLAN_MAC_LEN);
        }              
        //TURN_LED_ON;
    }
    else if(val == QAPI_WLAN_INVALID_PROFILE_E) // this event is used to indicate RSNA failure
    {
        QCLI_Printf(qcli_wlan_group, "4 way handshake failure for device=%d \r\n",devId);
    }
    else if(val == QAPI_WLAN_SEC_HS_TO_RECV_M3_E) // this event is used to indicate RSNA failure
    {
        QCLI_Printf(qcli_wlan_group, "4 way handshake failure due to timed out waiting for M3 for device=%d \r\n",devId);
    }	
    else if(val == 0x10 /*PEER_FIRST_NODE_JOIN_EVENT*/) //this event is used to RSNA success
    {
        QCLI_Printf(qcli_wlan_group, "4 way handshake success for device=%d \r\n",devId);
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
          set_dev_stat(devId,DOWN);
          if( memcmp(mac,disc_bss,__QAPI_WLAN_MAC_LEN) == 0){
            /*disabling flags in case of AP/GO mode*/
              hidden_flag = 0;
              wps_flag = 0;
          }
		  if((memcmp(mac,temp_bss,__QAPI_WLAN_MAC_LEN) == 0) && (memcmp(g_bssid[devId],temp_bss,__QAPI_WLAN_MAC_LEN) != 0))
		  {
		  	  QCLI_Printf(qcli_wlan_group,"devId %d Disconnected MAC addr %02x:%02x:%02x:%02x:%02x:%02x \r\n", 
	                     devId,g_bssid[devId][0], g_bssid[devId][1], g_bssid[devId][2], g_bssid[devId][3], g_bssid[devId][4], g_bssid[devId][5]);
		  }
		  else
		  {
	          QCLI_Printf(qcli_wlan_group,"devId %d Disconnected MAC addr %02x:%02x:%02x:%02x:%02x:%02x \r\n", 
	                     devId,mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		  }
		  
        }
        /* bssConn is flase for REF-STA disconnected from AP/p2pGO: dont update the dev stat*/
        else
        {
          QCLI_Printf (qcli_wlan_group,"REF_STA Disconnected MAC addr %02x:%02x:%02x:%02x:%02x:%02x devId %d\r\n",
                     mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],devId);
        }
        concurrent_connect_flag = 0x00;
       //TURN_LED_OFF;
       memcpy(g_bssid[devId],temp_bss,__QAPI_WLAN_MAC_LEN);
    }
    else
    {
        QCLI_Printf(qcli_wlan_group, "last tx rate : %d kbps--for device=%d\r\n", val, devId);
    }
    if(wps_context.wps_in_progress && devId==get_active_device())
    {
        wps_context.wps_in_progress=0;
        set_power_mode(QAPI_WLAN_POWER_MODE_REC_POWER_E, QAPI_WLAN_POWER_MODULE_WPS_E);          
    }
}

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : NetConnect()
* Returned Value  : 0 - success, 1 - failure
* Comments    : Handles connection to AP after WPS
*
*END*-----------------------------------------------------------------*/
int32_t NetConnect(qapi_WLAN_Netparams_t *pNetparams)
{
    int32_t status = -1;
    qapi_WLAN_WPS_Credentials_t wpsCred;
    uint32_t deviceId = get_active_device();
    uint32_t passphraseLen = 0 ;
    memset(&wpsCred, 0, sizeof(qapi_WLAN_WPS_Credentials_t));          

    do{
        if(pNetparams->ssid_Len == 0)
        {
            QCLI_Printf(qcli_wlan_group, "WPS failed\r\n");
            break;
        }
        else
        {    
            QCLI_Printf(qcli_wlan_group, "SSID: %s\r\n",pNetparams->ssid);
            if(pNetparams->sec_Type == QAPI_WLAN_AUTH_WPA2_PSK_E)
            {
                QCLI_Printf(qcli_wlan_group, "Security Type: WPA2\r\n");
                QCLI_Printf(qcli_wlan_group, "Passphrase: %s\r\n",(char*)pNetparams->u.passphrase);
            }
            else if(pNetparams->sec_Type == QAPI_WLAN_AUTH_WPA_PSK_E)
            {
                QCLI_Printf(qcli_wlan_group, "Security Type: WPA\r\n");
                QCLI_Printf(qcli_wlan_group, "Passphrase: %s\r\n",(char*)pNetparams->u.passphrase);
            }
            else if(pNetparams->sec_Type == QAPI_WLAN_AUTH_WEP_E)
            {
                QCLI_Printf(qcli_wlan_group, "Security Type: WEP\r\n");
                QCLI_Printf(qcli_wlan_group, "WEP key: %s\r\n",(char*)pNetparams->u.wepkey);
                QCLI_Printf(qcli_wlan_group, "Key index: %d\r\n",pNetparams->key_Index);
            }
            else if(pNetparams->sec_Type == QAPI_WLAN_AUTH_NONE_E)
            {
                QCLI_Printf(qcli_wlan_group, "Security Type: None\r\n");
            }

            set_callback(NULL);

            
            wpsCred.ssid_Length = strlen((char*)pNetparams->ssid);
            strncpy((char *) wpsCred.ssid, (char*)pNetparams->ssid, wpsCred.ssid_Length);
            strncpy((char*)original_ssid,(char*)pNetparams->ssid, wpsCred.ssid_Length);
            qapi_WLAN_Set_Param(deviceId,  
                    __QAPI_WLAN_PARAM_GROUP_WIRELESS, 
                    __QAPI_WLAN_PARAM_GROUP_WIRELESS_SSID,
                    (char *) pNetparams->ssid,
                    sizeof(pNetparams->ssid),
                    0);

            if(pNetparams->sec_Type == QAPI_WLAN_AUTH_WPA2_PSK_E){
                /* 
                 * Some AP's May take 1 second to switch group cipher, 
                 * we should not connect until this switch has happened.
                 */
                qurt_thread_sleep(120);

                wpsCred.auth_Mode = QAPI_WLAN_AUTH_WPA2_PSK_E;
                wpsCred.encryption_Type = pNetparams->cipher.ucipher;
                passphraseLen = strlen((char*)pNetparams->u.passphrase);    
                if(passphraseLen != 64)
                {                                
                    if (0 != qapi_WLAN_Set_Param(deviceId,
                                __QAPI_WLAN_PARAM_GROUP_WIRELESS_SECURITY,
                                __QAPI_WLAN_PARAM_GROUP_SECURITY_PASSPHRASE,
                                (void *) pNetparams->u.passphrase,
                                passphraseLen,
                                FALSE))
                        break;
                }
                else
                {
                    if(0 != qapi_WLAN_Set_Param(deviceId,
                                __QAPI_WLAN_PARAM_GROUP_WIRELESS_SECURITY,
                                __QAPI_WLAN_PARAM_GROUP_SECURITY_PMK,
                                (void *) pNetparams->u.passphrase,
                                passphraseLen,
                                FALSE))                                        
                        break;
                }
            }else if (pNetparams->sec_Type == QAPI_WLAN_AUTH_WPA_PSK_E){
                wpsCred.auth_Mode = QAPI_WLAN_AUTH_WPA_PSK_E;
                wpsCred.encryption_Type = pNetparams->cipher.ucipher;
				passphraseLen = strlen((char*)pNetparams->u.passphrase); 
                if(passphraseLen != 64)
                {                                
                    if (0 != qapi_WLAN_Set_Param(deviceId,
                                __QAPI_WLAN_PARAM_GROUP_WIRELESS_SECURITY,
                                __QAPI_WLAN_PARAM_GROUP_SECURITY_PASSPHRASE,
                                (void *) pNetparams->u.passphrase,
                                passphraseLen,
                                FALSE))
                        break;
                }
                else
                {
                    if(0 != qapi_WLAN_Set_Param(deviceId,
                                __QAPI_WLAN_PARAM_GROUP_WIRELESS_SECURITY,
                                __QAPI_WLAN_PARAM_GROUP_SECURITY_PMK,
                                (void *) pNetparams->u.passphrase,
                                passphraseLen,
                                FALSE))
                        break;
                }
            }else if (pNetparams->sec_Type == QAPI_WLAN_AUTH_WEP_E){
                wpsCred.key_Index = pNetparams->key_Index;
                strcpy((char *)wpsCred.key, (char*)pNetparams->u.wepkey); 
                wpsCred.key_Length = strlen((char*)pNetparams->u.wepkey);
            }else if (pNetparams->sec_Type == QAPI_WLAN_AUTH_NONE_E){
                wpsCred.auth_Mode = QAPI_WLAN_AUTH_NONE_E;
            }
            qapi_WLAN_Set_Param (deviceId,
                    __QAPI_WLAN_PARAM_GROUP_WIRELESS_SECURITY,
                    __QAPI_WLAN_PARAM_GROUP_SECURITY_WPS_CREDENTIALS,
                    &wpsCred,
                    sizeof(qapi_WLAN_WPS_Credentials_t),
                    FALSE);
            qapi_WLAN_WPS_Connect(deviceId);
            status = 0;
        }
    }while(0);

    return status;
}

uint32_t CompleteWPS(uint32_t deviceId, qapi_WLAN_Netparams_t *pNetparams, uint8_t block)
{
    uint32_t status = 0, error = 0, wifimode = 0, dataLen = 0;

    pNetparams->error = 0;
    pNetparams->dont_Block = (block)? 0:1;

    error = qapi_WLAN_WPS_Await_Completion(deviceId, pNetparams);

    qapi_WLAN_Get_Param (deviceId, 
            __QAPI_WLAN_PARAM_GROUP_WIRELESS,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_OPERATION_MODE,
            &wifimode,
            &dataLen);

    if (wifimode == QAPI_WLAN_DEV_MODE_STATION_E) {
        if(error != 0)
        {
            /* This case occurs when event from target is not received yet
             * and qapi_WLAN_WPS_Await_Completion returns PENDING */
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
                    QCLI_Printf(qcli_wlan_group, "WPS error: invalid start info\r\n");
                    break;

                case QAPI_WLAN_WPS_ERROR_MULTIPLE_PBC_SESSIONS_E:
                    QCLI_Printf(qcli_wlan_group, "WPS error: Multiple PBC Sessions\r\n");
                    break;

                case QAPI_WLAN_WPS_ERROR_WALKTIMER_TIMEOUT_E:
                    QCLI_Printf(qcli_wlan_group, "WPS error: Walktimer Timeout\r\n");
                    break;

                case QAPI_WLAN_WPS_ERROR_M2D_RCVD_E:
                    QCLI_Printf(qcli_wlan_group, "WPS error: M2D RCVD\r\n");
                    break;

                case QAPI_WLAN_WPS_ERROR_PWD_AUTH_FAIL_E:
                    QCLI_Printf(qcli_wlan_group, "WPS error: AUTH FAIL\r\n");
                    break;

                case QAPI_WLAN_WPS_ERROR_CANCELLED_E:
                    QCLI_Printf(qcli_wlan_group, "WPS error: WPS CANCEL\r\n");
                    break;

                case QAPI_WLAN_WPS_ERROR_INVALID_PIN_E:
                    QCLI_Printf(qcli_wlan_group, "WPS error: INVALID PIN\r\n");
                    break;

                default:
                    //QCLI_Printf(qcli_wlan_group, "WPS error: unknown %d\r\n",pNetparams->error);
                    break;
            }
        }
    }while(0);

    if (wifimode == QAPI_WLAN_DEV_MODE_AP_E)
    {
        if (0x00 == pNetparams->error && 0 == error) {
            QCLI_Printf(qcli_wlan_group, "\r\n***** WPS PROFILE ****\r\n");
            QCLI_Printf(qcli_wlan_group, "SSID: %s\r\n", pNetparams->ssid);
            if (pNetparams->sec_Type == QAPI_WLAN_AUTH_WPA2_PSK_E)
            {
                QCLI_Printf(qcli_wlan_group,"Security Type: WPA2\r\nEncryption Type: AES\r\n");
                QCLI_Printf(qcli_wlan_group,"Passphrase: %s\r\n",(char*)pNetparams->u.passphrase);
            }else if(pNetparams->sec_Type == QAPI_WLAN_AUTH_WPA_PSK_E)
            {
                QCLI_Printf(qcli_wlan_group,"Security Type: WPA\r\nEncryption Type: AES\r\n");
                QCLI_Printf(qcli_wlan_group,"Passphrase: %s\r\n",(char*)pNetparams->u.passphrase);
            }
            else
            {
                QCLI_Printf(qcli_wlan_group,"Security Type: Open\r\n");
            }
            status = 0;
        }
        else
        {
            pNetparams->error = (pNetparams->error)?pNetparams->error: 13; //A_EBUSY;
            status = 0;
        }

    }

    return status;
}


/*FUNCTION*-------------------------------------------------------------
 *
 * Function Name   : wps_query()
 * Returned Value  : 0 - success, 1 - failure
 * Comments    : Queries WPS status
 *
 *END*-----------------------------------------------------------------*/
int32_t wps_query(uint8_t block)
{
    qapi_WLAN_Netparams_t netparams;
    int32_t status = 1;
    uint32_t deviceId = 0, wifimode = 0, dataLen = 0;

    deviceId = get_active_device();
    qapi_WLAN_Get_Param (deviceId, 
                         __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                         __QAPI_WLAN_PARAM_GROUP_WIRELESS_OPERATION_MODE,
                         &wifimode,
                         &dataLen);

    if (CompleteWPS(deviceId, &netparams, block))
    {
        memcpy(&wps_context.netparams, &netparams, sizeof(qapi_WLAN_Netparams_t));

        if(netparams.error)
        {
            wps_context.connect_flag=0;
        }

        if(wps_context.connect_flag)
        {
#if ENABLE_SCC_MODE
            do
            {
                if((concurrent_connect_flag == 0x0E) && (deviceId == 1) )
                {
                    QCLI_Printf(qcli_wlan_group, "ap_ch:%d\r\n", netparams.ap_channel);           

                    if ( 0 != qapi_WLAN_Set_Param(deviceId,
                                                   __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                                                   __QAPI_WLAN_PARAM_GROUP_WIRELESS_CHANNEL,
                                                   (void *) &netparams.ap_channel,
                                                   sizeof(netparams.ap_channel),
                                                   FALSE))
                    {
                        QCLI_Printf(qcli_wlan_group, "Unable to set to AP channel:%d\r\n",netparams.ap_channel); 
                        break;
                    }
                    A_MDELAY(50);

                }

                if(NetConnect(&netparams))
                {
                    QCLI_Printf(qcli_wlan_group, "wps connection failed\r\n");
                }
            }while(0);
#else
            if(NetConnect(&netparams))
            {
                QCLI_Printf(qcli_wlan_group, "connection failed\r\n");
            }
#endif         
        }

        status = 0;
        set_power_mode( QAPI_WLAN_POWER_MODE_REC_POWER_E, QAPI_WLAN_POWER_MODULE_WPS_E);

        QCLI_Printf(qcli_wlan_group, "WPS completed\r\n");
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
            set_power_mode( QAPI_WLAN_POWER_MODE_REC_POWER_E, QAPI_WLAN_POWER_MODULE_WPS_E);          
            memcpy(&wps_context.netparams,&netparams,sizeof(qapi_WLAN_Netparams_t));
            status = 0;
        }
    }
	
	wps_context.wps_in_progress=0;
	return status;
}

int32_t wps_pin_setup(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    uint8_t val = 0;
    qapi_WLAN_WPS_Credentials_t wpsScan, *wpsScan_p = NULL;
    int j = 0;
    uint32_t wifi_mode = 0, data_len = 0, error = 0, enet_device = 0;
    char data[32+1] = {'\0'};

    set_callback(NULL);
    enet_device = get_active_device();
    error = qapi_WLAN_Get_Param (enet_device, 
                         __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                         __QAPI_WLAN_PARAM_GROUP_WIRELESS_OPERATION_MODE,
                         &wifi_mode,
                         &data_len);
    if (error != 0)
    {
        QCLI_Printf(qcli_wlan_group, "WPS failed\r\n");
        return QCLI_STATUS_ERROR_E;
    }
    if (wifi_mode == QAPI_WLAN_DEV_MODE_AP_E)
    {
        error = qapi_WLAN_Get_Param (enet_device,
                             __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                             __QAPI_WLAN_PARAM_GROUP_WIRELESS_SSID,
                             (void *) data,
                             &data_len);
        if (data_len == 0)
        {
            QCLI_Printf(qcli_wlan_group, "Set up soft-ap connection before starting WPS\r\n");
            return QCLI_STATUS_ERROR_E;
        }
        if (error != 0)
        {
            QCLI_Printf(qcli_wlan_group, "WPS failed\r\n");
            return QCLI_STATUS_ERROR_E;
        }
    }

    /* Initialize context */
    wps_context.wps_in_progress = 0;
    /* Connect flag */
    wps_context.connect_flag = Parameter_List[0].Integer_Value;
    /* Mode */
    uint8_t wps_mode = QAPI_WLAN_WPS_PIN_MODE_E;
    /* Pin */
    memset(wpsPin, 0, MAX_WPS_PIN_SIZE);
    char delimter[] = "-#";
    char *result = strtok( (char *) Parameter_List[1].String_Value, delimter);
    if(result == NULL)
    {
        strcpy(wpsPin, (char *) Parameter_List[1].String_Value);
    }
    else
    {
        while( result != NULL )
        {
            strcat(wpsPin, result);
            result = strtok( NULL, delimter );
        }
    }

    if (Parameter_Count > 2)
    {
        /* SSID */
        if (strlen(Parameter_List[2].String_Value) > __QAPI_WLAN_MAX_SSID_LEN)
        {
                QCLI_Printf(qcli_wlan_group, "Invalid SSID length\r\n");
                return QCLI_STATUS_ERROR_E;
        }
        memset(wpsScan.ssid, 0, __QAPI_WLAN_MAX_SSID_LEN);
        strncpy((char*)(wpsScan.ssid), Parameter_List[2].String_Value, strlen(Parameter_List[2].String_Value));
        wpsScan.ssid_Length = strlen(Parameter_List[2].String_Value);

        /* MAC address */
        if(strlen((char *) Parameter_List[3].String_Value) != 12)
        {
            QCLI_Printf(qcli_wlan_group, "Invalid MAC address length\r\n");
            return QCLI_STATUS_ERROR_E;
        }
        memset(wpsScan.mac_Addr, 0, __QAPI_WLAN_MAC_LEN);
        for(j=0; j < strlen((char *) Parameter_List[3].String_Value); j++)
        {
            val = ascii_to_hex(Parameter_List[3].String_Value[j]);
            if(val == 0xff)
            {
                QCLI_Printf(qcli_wlan_group, "Invalid character\r\n");
                return QCLI_STATUS_ERROR_E;
            }
            else
            {
                if((j&1) == 0)
                {
                    val <<= 4;
                }
                wpsScan.mac_Addr[j>>1] |= val;
            }
        }

        /* Wireless channel */
        wpsScan.ap_Channel = chan_to_frequency(Parameter_List[4].Integer_Value);
        wpsScan_p = &wpsScan;
    }

    set_power_mode( QAPI_WLAN_POWER_MODE_MAX_PERF_E, QAPI_WLAN_POWER_MODULE_WPS_E);

    qapi_WLAN_Set_Param (enet_device,
            __QAPI_WLAN_PARAM_GROUP_WIRELESS_SECURITY,
            __QAPI_WLAN_PARAM_GROUP_SECURITY_WPS_CREDENTIALS,
            wpsScan_p,
            sizeof(qapi_WLAN_WPS_Credentials_t),
            FALSE);

    if(qapi_WLAN_WPS_Start(enet_device, wps_context.connect_flag, wps_mode, wpsPin) != 0)
    {
        QCLI_Printf(qcli_wlan_group, "WPS failed\r\n");
        set_power_mode( QAPI_WLAN_POWER_MODE_REC_POWER_E, QAPI_WLAN_POWER_MODULE_WPS_E);
        return QCLI_STATUS_ERROR_E;
    }

    wps_context.wps_in_progress = 1;

    return QCLI_STATUS_SUCCESS_E;
}

int32_t wps_push_setup(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    uint8_t val = 0, wps_mode = 0;
    qapi_WLAN_WPS_Credentials_t wpsScan, *wpsScan_p = NULL;
    int j = 0;
    uint32_t wifi_mode = 0, data_len = 0, error = 0, enet_device = 0;
    char data[32+1] = {'\0'};

    set_callback(NULL);
    enet_device = get_active_device();
    error = qapi_WLAN_Get_Param (enet_device, 
                         __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                         __QAPI_WLAN_PARAM_GROUP_WIRELESS_OPERATION_MODE,
                         &wifi_mode,
                         &data_len);
    if (error != 0)
    {
        QCLI_Printf(qcli_wlan_group, "WPS failed\r\n");
        return QCLI_STATUS_ERROR_E;
    }
    if (wifi_mode == QAPI_WLAN_DEV_MODE_AP_E)
    {
        error = qapi_WLAN_Get_Param (enet_device,
                             __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                             __QAPI_WLAN_PARAM_GROUP_WIRELESS_SSID,
                             (void *) data,
                             &data_len);
        if (data_len == 0)
        {
            QCLI_Printf(qcli_wlan_group, "Set up soft-ap connection before starting WPS\r\n");
            return QCLI_STATUS_ERROR_E;
        }
        if (error != 0)
        {
            QCLI_Printf(qcli_wlan_group, "WPS failed\r\n");
            return QCLI_STATUS_ERROR_E;
        }
    }

    /* Initialize context */
    wps_context.wps_in_progress = 0;
    /* Connect flag */
    wps_context.connect_flag = Parameter_List[0].Integer_Value;
    /* Mode */
    wps_mode = QAPI_WLAN_WPS_PBC_MODE_E;
    /* Pin */
    memset(wpsPin, 0, MAX_WPS_PIN_SIZE);

    if (Parameter_Count > 1)
    {
        /* SSID */
        if (strlen(Parameter_List[1].String_Value) > __QAPI_WLAN_MAX_SSID_LEN)
        {
                QCLI_Printf(qcli_wlan_group, "Invalid SSID length\r\n");
                return QCLI_STATUS_ERROR_E;
        }
        memset(wpsScan.ssid, 0, __QAPI_WLAN_MAX_SSID_LEN);
        strncpy((char*)(wpsScan.ssid), Parameter_List[1].String_Value, strlen(Parameter_List[1].String_Value));
        wpsScan.ssid_Length = strlen(Parameter_List[1].String_Value);

        /* MAC address */
        if(strlen((char *) Parameter_List[2].String_Value) != 12)
        {
            QCLI_Printf(qcli_wlan_group, "Invalid MAC address length\r\n");
            return QCLI_STATUS_ERROR_E;
        }
        memset(wpsScan.mac_Addr, 0, __QAPI_WLAN_MAC_LEN);
        for(j=0; j < strlen((char *) Parameter_List[2].String_Value); j++)
        {
            val = ascii_to_hex(Parameter_List[2].String_Value[j]);
            if(val == 0xff)
            {
                QCLI_Printf(qcli_wlan_group, "Invalid character\r\n");
                return QCLI_STATUS_ERROR_E;
            }
            else
            {
                if((j&1) == 0)
                {
                    val <<= 4;
                }
                wpsScan.mac_Addr[j>>1] |= val;
            }
        }  

        /* Wireless channel */
        wpsScan.ap_Channel = chan_to_frequency(Parameter_List[3].Integer_Value);
        wpsScan_p = &wpsScan;
    }

    set_power_mode( QAPI_WLAN_POWER_MODE_MAX_PERF_E, QAPI_WLAN_POWER_MODULE_WPS_E);

    if (0 != qapi_WLAN_Set_Param (enet_device,
                __QAPI_WLAN_PARAM_GROUP_WIRELESS_SECURITY,
                __QAPI_WLAN_PARAM_GROUP_SECURITY_WPS_CREDENTIALS,
                wpsScan_p,
                sizeof(qapi_WLAN_WPS_Credentials_t),
                FALSE))
    {
        QCLI_Printf(qcli_wlan_group, "WPS failed\r\n");
        return QCLI_STATUS_ERROR_E;
    }

    /* Pin not used for WPS push mode */
    memset(wpsPin, 0, MAX_WPS_PIN_SIZE);
    if(qapi_WLAN_WPS_Start(enet_device, wps_context.connect_flag, wps_mode, wpsPin) != 0)
    {
        QCLI_Printf(qcli_wlan_group, "WPS failed\r\n");
        set_power_mode( QAPI_WLAN_POWER_MODE_REC_POWER_E, QAPI_WLAN_POWER_MODULE_WPS_E);
        return QCLI_STATUS_ERROR_E;
    }

    wps_context.wps_in_progress = 1;

    return QCLI_STATUS_SUCCESS_E;
}

int32_t disconnect_from_network()
{
   uint32_t deviceId = get_active_device();
   if ( 0 == qapi_WLAN_Disconnect(deviceId))
      return 0;
   return -1;
}
/*FUNCTION*-----------------------------------------------------------------
*
* Function Name  : set_passphrase
* Returned Value : 0
* Comments       : Store WPA Passphrase for later use
*
*END------------------------------------------------------------------*/
int32_t set_passphrase(char* passphrase)
{
	uint8_t deviceId = 0;
	uint32_t i = 0;
	deviceId = get_active_device();
	
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
		           QCLI_Printf(qcli_wlan_group, "passphrase in hex, please enter [0-9] or [A-F]\r\n\r\n");
		           return -1;
		        }
 			}
    	}
		memset(wpa_passphrase[deviceId], 0, (__QAPI_WLAN_PASSPHRASE_LEN + 1));
   		strcpy(wpa_passphrase[deviceId],passphrase);
   		return 0;
    }
	else
	{
		QCLI_Printf(qcli_wlan_group, "Wrong passphrase length, the length should be between 8 and 64. \r\n\r\n");
		return -1;
	}

}

int32_t get_rssi()
{
   uint8_t rssi = 0;
   uint32_t dataLen = 0;
   if (0 == qapi_WLAN_Get_Param (get_active_device(), 
               __QAPI_WLAN_PARAM_GROUP_WIRELESS,
               __QAPI_WLAN_PARAM_GROUP_WIRELESS_RSSI,
               &rssi,
               &dataLen))
   {
      QCLI_Printf(qcli_wlan_group, "\r\nindicator = %d dB\r\n",rssi);
      return 0;
   }
   return -1;
}



int32_t set_sta_listen_interval(int32_t listen_time)
{
   if (0 == qapi_WLAN_Set_Param(get_active_device(),
                                __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                                __QAPI_WLAN_PARAM_GROUP_WIRELESS_STA_LISTEN_INTERVAL_IN_TU,
                                (void *) &listen_time,
                                sizeof(listen_time),
                                FALSE))
   {
       return 0;
   }
   return -1;
}

int32_t set_sta_mac_keep_alive_time(int32_t keep_alive_time_sec)
{
   if (keep_alive_time_sec ==0 )
   {
      QCLI_Printf(qcli_wlan_group, "keep_alive_timer will be disabled.\n");
   }
   if (0 == qapi_WLAN_Set_Param(get_active_device(),
                              __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                              __QAPI_WLAN_PARAM_GROUP_WIRELESS_STA_KEEP_ALIVE_IN_SEC,
	                      &keep_alive_time_sec,
                              sizeof(keep_alive_time_sec),
	                      FALSE))
   {
       return 0;
   }
   return -1;
}
/*FUNCTION*-----------------------------------------------------------------
*
* Function Name  : set_wep_key
* Returned Value : -1 on error else 0
* Comments       : Store WEP key for later use. Size of Key must be 10 or 26
*                  Hex characters
*
*END------------------------------------------------------------------*/
int32_t set_wep_key(int32_t key_index, char* key_val)
{
    int key_idx = key_index;

    int i = 0;
    if(strlen(key_val) != MIN_HEX_WEP_KEY_SIZE && strlen(key_val) != MAX_HEX_WEP_KEY_SIZE &&
        strlen(key_val) != (MIN_HEX_WEP_KEY_SIZE/2) && strlen(key_val) != (MAX_HEX_WEP_KEY_SIZE/2))
    {
    QCLI_Printf(qcli_wlan_group, "Invalid WEP Key length, only 10 or 26 HEX characters allowed (or) 5 or 13 ascii keys allowed\r\n\r\n");
    return -1;
    }

    if(key_idx < 1 || key_idx > MAX_NUM_WEP_KEYS)
    {
        QCLI_Printf(qcli_wlan_group, "Invalid key index, Please enter between 1-4\r\n\r\n");
        return -1;
    }

    if((strlen(key_val) == MIN_HEX_WEP_KEY_SIZE) || (strlen(key_val) == MAX_HEX_WEP_KEY_SIZE))
    {
    for (i = 0; i < strlen(key_val); i++)
    {
        if(ishexdigit(key_val[i]))
        {
           continue;
        }
        else
        {
           QCLI_Printf(qcli_wlan_group, "for hex enter [0-9] or [A-F]\r\n\r\n");
           return -1;
        }
    }
    }

    key[key_idx-1].key_valid = 1;        //set flag to indicate occupancy
    //clear and set with new key
    memset(key[key_idx-1].key, 0, strlen(key[key_idx-1].key));
    memcpy(key[key_idx-1].key, key_val, strlen(key_val));
    return 0;
}

/*FUNCTION*-----------------------------------------------------------------
*
* Function Name  : set_roam_thresh
* Returned Value : -1 on error else 0
* Comments       : Set lower and upper roam thresholds
*
*END------------------------------------------------------------------*/

int32_t set_roam_thresh(int32_t lower_thresh, int32_t upper_thresh, int32_t wt, int32_t pollTime)
{
    qapi_WLAN_Rssi_Threshold_Params_t qrthresh; 
    /*These values are positive input values*/ 
    qrthresh.threshold_Above1_Val     = upper_thresh; //Lower positive value (high rssi)
    qrthresh.threshold_Below1_Val     = lower_thresh; //Higher positive value (low rssi)
    qrthresh.weight                   = wt; 
    qrthresh.poll_Time                = pollTime;

    return qapi_WLAN_Set_Param(get_active_device(),
                               __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                               __QAPI_WLAN_PARAM_GROUP_WIRELESS_RSSI_THRESHOLD, 
                               &qrthresh,
                               sizeof(qrthresh),
                               FALSE);
}

int my_isdigit( int c)
{
   if ( c >= '0' && c <='9')
       return 1;
   else
       return 0;
}

int my_isspace( int c)
{
    if ( c == ' ')
        return 1;
    else
        return 0;
}

unsigned long long
my_atoll(const char *p)
{
    unsigned long long n = 0;
    int c = 0, neg = 0;
    unsigned char *up = (unsigned char *)p;

    if (!my_isdigit(c = *up)) {
            while (my_isspace(c))
                    c = *++up;
            switch (c) {
            case '-':
                    neg++;
                    /* FALLTHROUGH */
            case '+':
                    c = *++up;
            }
            if (!my_isdigit(c))
                return (0);
    }
    for (n = '0' - c; my_isdigit(c = *++up); ) {
        n *= 10; /* two steps to avoid unnecessary overflow */
        n += '0' - c; /* accum neg to avoid surprises at MAX */
    }
	
    return (neg ? n : -n);
}


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : tcp_keepalive_offload_session_config()
* Returned Value  : 0 if success else -1
*
* Comments        : This function configures the session info to offload the TCP Keepalive functionality to the target.
*                   Sets static IPv4 parameters.
*
*       wmiconfig --tcpoffloadsession <tcp_src_port> <tcp_dst_port> <src_ip> <dst_ip> <dest_mac_addr> <seq_num> <ack_seq> <ip protocol> ; 4 - ipv4, 6 - ipv6
*  e.g. wmiconfig --tcpoffloadsession 801 901 192.168.1.2 192.168.1.3 <dest mac addr> 1 1 4  
*
*END*-----------------------------------------------------------------*/
int32_t tcp_keepalive_offload_session_config(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    uint32_t s_addr = 0, d_addr = 0; 
    unsigned char  s_v6addr[16] = { '\0'}, d_v6addr[16] = { '\0'};
    const char *dest_mac_addr = NULL;
    uint32_t seq_num = 0, ack_seq = 0;
    int src_port = 0, dst_port = 0; 
    qapi_WLAN_IP_Protocol_Type_e protocol; 
    unsigned int sbits = 0; 
    qapi_WLAN_TCP_Offload_Config_Params_t tcp_offload_sess_cfg;
    int32_t retval = -1, sockid = 0; 

	
    if(Parameter_Count < 7 || Parameter_Count > 8)
    {
        return -1;
    }

    //sockid   = Parameter_List[0].Integer_Value;
    src_port = Parameter_List[0].Integer_Value;
    dst_port = Parameter_List[1].Integer_Value;	
    seq_num  = my_atoll(Parameter_List[5].String_Value);
    ack_seq  = my_atoll(Parameter_List[6].String_Value);
    protocol = (qapi_WLAN_IP_Protocol_Type_e)Parameter_List[7].Integer_Value;
	
    dest_mac_addr   = (const char *) Parameter_List[4].String_Value;

    if(protocol == QAPI_WLAN_IP_PROTOCOL_V4_E)
    {
        if( parse_ipv4_ad((unsigned long *)&s_addr, &sbits, (char *) Parameter_List[2].String_Value) )
        {
            QCLI_Printf(qcli_wlan_group, "Invalid Source address\r\n");
            return -1;
        }
        s_addr = BE2CPU32(s_addr);
		
        if( parse_ipv4_ad((unsigned long *)&d_addr, &sbits, (char *) Parameter_List[3].String_Value) )
        {
            QCLI_Printf(qcli_wlan_group, "Invalid Destination address\r\n");
            return -1;
        }
        d_addr = BE2CPU32(d_addr);

	tcp_offload_sess_cfg.src_IP           = s_addr; 
	tcp_offload_sess_cfg.dst_IP           = d_addr;
    }
    else if(protocol == QAPI_WLAN_IP_PROTOCOL_V6_E)
    {
	retval = inet_pton(AF_INET6, (const char *) Parameter_List[2].String_Value, s_v6addr);
	if(retval == 1) {
	    QCLI_Printf(qcli_wlan_group, "Invalid ipv6 prefix\r\n");
	    return -1; 
	}

	retval = inet_pton(AF_INET6, (const char *) Parameter_List[3].String_Value, d_v6addr);
	if(retval == 1) {
	    QCLI_Printf(qcli_wlan_group, "Invalid ipv6 prefix\r\n");
	    return -1; 
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
	
    if(mystrtomac(dest_mac_addr, tcp_offload_sess_cfg.dest_MAC)) {
	   QCLI_Printf(qcli_wlan_group, "Destination MAC address translation failed.\r\n");
	   return 0;
    }
	
    return qapi_WLAN_Set_Param(get_active_device(),
                               __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                               __QAPI_WLAN_PARAM_GROUP_WIRELESS_TCP_KEEPALIVE_OFFLOAD_SESSION_CFG,
                               &tcp_offload_sess_cfg,
                               sizeof(tcp_offload_sess_cfg),
                               FALSE);
}


/*FUNCTION*-----------------------------------------------------------------
*
* Function Name  : tcp_keepalive_offload_enable_disable
* Returned Value : -1 on error else 0
* Comments       : Enable/Disable, TCPKeepaliveInterval, keep_Alive_Ack_Recv_Threshold
*
*END------------------------------------------------------------------*/
int32_t tcp_keepalive_offload_enable_disable( uint8_t enable, uint16_t keepalive_intvl, uint16_t keepalive_ack_recv_threshold )
{
    qapi_WLAN_TCP_Offload_Enable_t tcp_offload_enable;
    uint32_t deviceId = get_active_device();

    set_callback(NULL);
    
    tcp_offload_enable.enable = enable; 
    tcp_offload_enable.keepalive_Interval = keepalive_intvl; 
    tcp_offload_enable.keepalive_Ack_Recv_Threshold = keepalive_ack_recv_threshold; 
    
    return qapi_WLAN_Set_Param(deviceId,
                               __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                               __QAPI_WLAN_PARAM_GROUP_WIRELESS_TCP_KEEPALIVE_OFFLOAD_ENABLE, 
                                       &tcp_offload_enable,
                                       sizeof(tcp_offload_enable),
                                       FALSE);
}

/*FUNCTION*-----------------------------------------------------------------
*
* Function Name  : clear_wep_keys
* Returned Value : Success
* Comments       : Clear wep keys
*
*END------------------------------------------------------------------*/
int32_t clear_wep_keys()
{
    int i = 0;
    for(i=0;i<MAX_NUM_WEP_KEYS;i++)
    {
        key[i].key_valid = 0;
		memset(key[i].key, 0, sizeof(key[i].key));
    }
        return 0;
}

/*****************************************************
*Function- probe_req_handler- processes incoming probe requests.
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
void probe_req_handler(void* buf, int len, int freq)
{
    int i = 0;
    QCLI_Printf(qcli_wlan_group, "RX: ProbeRequest from freq: %d MAC address: ", freq);
    for(i=0; i< 6; i++){
        QCLI_Printf(qcli_wlan_group, "%x ",*((uint8_t*)buf + PROBE_REQ_SRC_ADDR_OFFSET + i));
    }
    QCLI_Printf(qcli_wlan_group, "\r\n");
}

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

    QCLI_Printf(qcli_p2p_group, "\r\n");
    if(p2p_cancel_enable == 0)
    {
        for(loop_index = 0; loop_index < __QAPI_WLAN_P2P_MAX_LIST_COUNT; loop_index++)
        {
            QCLI_Printf(qcli_p2p_group, "mac_addr[%d] : %02x:%02x:%02x:%02x:%02x:%02x\r\n", loop_index, 
                        ((qapi_WLAN_P2P_Persistent_Mac_List_t *)(local_ptr))->macaddr[0],
                        ((qapi_WLAN_P2P_Persistent_Mac_List_t *)(local_ptr))->macaddr[1],
                        ((qapi_WLAN_P2P_Persistent_Mac_List_t *)(local_ptr))->macaddr[2],
                        ((qapi_WLAN_P2P_Persistent_Mac_List_t *)(local_ptr))->macaddr[3],
                        ((qapi_WLAN_P2P_Persistent_Mac_List_t *)(local_ptr))->macaddr[4],
                        ((qapi_WLAN_P2P_Persistent_Mac_List_t *)(local_ptr))->macaddr[5]);

            QCLI_Printf(qcli_p2p_group, "ssid[%d] : %s\r\n", loop_index, 
                        ((qapi_WLAN_P2P_Persistent_Mac_List_t *)(local_ptr))->ssid);

            if(((qapi_WLAN_P2P_Persistent_Mac_List_t *)(local_ptr))->role == 
                                            QAPI_WLAN_P2P_INV_ROLE_ACTIVE_GO_E)
            {
                QCLI_Printf(qcli_p2p_group, "passphrase[%d] : %s\r\n", loop_index, 
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

    deviceId = get_active_device();
    if(deviceId != 0)
    {
        /* P2P device should always send/receive events/commands on dev 0 if
        the app has switched to dev 1 while event is in dev 0 send command
        via dev 0 and then switch to dev 1*/
        temp_device_id = deviceId;
        deviceId = 0;
        set_active_deviceid(deviceId);
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
                QCLI_Printf(qcli_p2p_group, "\r\n\t p2p_config_method     : %x \r\n",
                            (((qapi_WLAN_P2P_Device_Lite_t *)(local_ptr))->config_Methods));

                QCLI_Printf(qcli_p2p_group, " \t p2p_device_name       : %s \r\n",
                            ((qapi_WLAN_P2P_Device_Lite_t *)(local_ptr))->device_Name);
                
                QCLI_Printf(qcli_p2p_group, "\t p2p_primary_dev_type  : %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\r\n ",
                            ((qapi_WLAN_P2P_Device_Lite_t *)(local_ptr))->pri_Dev_Type[0],
                            ((qapi_WLAN_P2P_Device_Lite_t *)(local_ptr))->pri_Dev_Type[1],
                            ((qapi_WLAN_P2P_Device_Lite_t *)(local_ptr))->pri_Dev_Type[2],
                            ((qapi_WLAN_P2P_Device_Lite_t *)(local_ptr))->pri_Dev_Type[3],
                            ((qapi_WLAN_P2P_Device_Lite_t *)(local_ptr))->pri_Dev_Type[4],
                            ((qapi_WLAN_P2P_Device_Lite_t *)(local_ptr))->pri_Dev_Type[5],
                            ((qapi_WLAN_P2P_Device_Lite_t *)(local_ptr))->pri_Dev_Type[6],
                            ((qapi_WLAN_P2P_Device_Lite_t *)(local_ptr))->pri_Dev_Type[7]);

                QCLI_Printf(qcli_p2p_group, "\t p2p_interface_addr    : %02x:%02x:%02x:%02x:%02x:%02x \r\n",
                            ((qapi_WLAN_P2P_Device_Lite_t *)(local_ptr))->interface_Addr[0],
                            ((qapi_WLAN_P2P_Device_Lite_t *)(local_ptr))->interface_Addr[1],
                            ((qapi_WLAN_P2P_Device_Lite_t *)(local_ptr))->interface_Addr[2],
                            ((qapi_WLAN_P2P_Device_Lite_t *)(local_ptr))->interface_Addr[3],
                            ((qapi_WLAN_P2P_Device_Lite_t *)(local_ptr))->interface_Addr[4],
                            ((qapi_WLAN_P2P_Device_Lite_t *)(local_ptr))->interface_Addr[5]);

                QCLI_Printf(qcli_p2p_group, "\t p2p_device_addr       : %02x:%02x:%02x:%02x:%02x:%02x \r\n",
                            ((qapi_WLAN_P2P_Device_Lite_t *)(local_ptr))->p2p_Device_Addr[0],
                            ((qapi_WLAN_P2P_Device_Lite_t *)(local_ptr))->p2p_Device_Addr[1],
                            ((qapi_WLAN_P2P_Device_Lite_t *)(local_ptr))->p2p_Device_Addr[2],
                            ((qapi_WLAN_P2P_Device_Lite_t *)(local_ptr))->p2p_Device_Addr[3],
                            ((qapi_WLAN_P2P_Device_Lite_t *)(local_ptr))->p2p_Device_Addr[4],
                            ((qapi_WLAN_P2P_Device_Lite_t *)(local_ptr))->p2p_Device_Addr[5]);

                QCLI_Printf(qcli_p2p_group, "\t p2p_device_capability : %x \r\n",
                            ((qapi_WLAN_P2P_Device_Lite_t *)(local_ptr))->dev_Capab);

                QCLI_Printf(qcli_p2p_group, "\t p2p_group_capability  : %x \r\n",
                            ((qapi_WLAN_P2P_Device_Lite_t *)(local_ptr))->group_Capab);

                QCLI_Printf(qcli_p2p_group, "\t p2p_wps_method        : %x \r\n",
                            ((qapi_WLAN_P2P_Device_Lite_t *)(local_ptr))->wps_Method);

                QCLI_Printf(qcli_p2p_group, "\t Peer Oper   channel   : %d \r\n",
                            ((qapi_WLAN_P2P_Device_Lite_t *)(local_ptr))->oper_Freq); 
            }

            local_ptr += sizeof(qapi_WLAN_P2P_Device_Lite_t);
        }

        if(p2p_join_session_active) 
        {        
            p2p_join_session_active = 0;
            p2p_set_params.val.mode.p2pmode = __QAPI_WLAN_P2P_CLIENT;

            if (0 != qapi_WLAN_Set_Param(deviceId, __QAPI_WLAN_PARAM_GROUP_P2P,
                                          __QAPI_WLAN_PARAM_GROUP_P2P_OP_MODE,
                                          &p2p_set_params.val.mode.p2pmode,
                                          sizeof(p2p_set_params.val.mode.p2pmode),
                                          FALSE))
            {
                QCLI_Printf(qcli_p2p_group, "\r\nStartP2P JOIN SET command did not execute properly\r\n");
                goto set_temp_device;
            } 

            if(qapi_WLAN_P2P_Join(deviceId, p2p_join_profile.wps_Method,
                                  &p2p_join_mac_addr[0], p2p_wps_pin,
                                  p2p_join_profile.go_Oper_Freq) != 0)
            {
                QCLI_Printf(qcli_p2p_group, "\r\nP2P JOIN command did not execute properly\r\n");
#if ENABLE_SCC_MODE
                QCLI_Printf(qcli_p2p_group, "support single concurrent channel only....\r\n");
#endif /* ENABLE_SCC_MODE */
                goto set_temp_device;
            }        
        }
    }
    local_ptr = temp_ptr;

    set_temp_device:
    if(temp_device_id != 0)
    {
        set_active_deviceid(temp_device_id);
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
	QCLI_Printf(qcli_p2p_group, "\r\n source addr : %02x:%02x:%02x:%02x:%02x:%02x \r\n", local_ptr->sa[0], local_ptr->sa[1],
                                                                              local_ptr->sa[2], local_ptr->sa[3],
                                                                              local_ptr->sa[4], local_ptr->sa[5]);
	
    QCLI_Printf(qcli_p2p_group, "\r\n wps_config_method : %x \r\n", local_ptr->wps_Config_Method);

	if(__QAPI_WLAN_P2P_WPS_CONFIG_DISPLAY == wps_method)
    {
        QCLI_Printf(qcli_p2p_group, "Provisional Disc Request - Display WPS PIN [%s] \r\n",p2p_wps_pin);
	}
	else if(__QAPI_WLAN_P2P_WPS_CONFIG_KEYPAD == wps_method)
    {
        QCLI_Printf(qcli_p2p_group, "Provisional Disc Request - Enter WPS PIN \r\n");
	}
	else if(__QAPI_WLAN_P2P_WPS_CONFIG_PUSHBUTTON == wps_method)
    {
        QCLI_Printf(qcli_p2p_group, "Provisional Disc Request - Push Button \r\n");
	}
	else
	{
	    QCLI_Printf(qcli_p2p_group, "Invalid Provisional Request \r\n");
	}
	return;
}

void P2P_Event_Handler_Prov_Disc_Resp(CMD_P2P_EVENT_INFO *pEventInfo)
{
    qapi_WLAN_P2P_Prov_Disc_Resp_Event_t *local_ptr = NULL;

    local_ptr = (qapi_WLAN_P2P_Prov_Disc_Resp_Event_t *) pEventInfo->pBuffer;
    QCLI_Printf(qcli_p2p_group, "\r\n peer addr : %02x:%02x:%02x:%02x:%02x:%02x \r\n",
                local_ptr->peer[0], local_ptr->peer[1], local_ptr->peer[2],
                local_ptr->peer[3], local_ptr->peer[4], local_ptr->peer[5]);

    if(__QAPI_WLAN_P2P_WPS_CONFIG_KEYPAD == local_ptr->config_Methods)
	{
	    QCLI_Printf(qcli_p2p_group, "Provisional Disc Response Keypad - WPS PIN [%s] \r\n",p2p_wps_pin);
	}
	else if(__QAPI_WLAN_P2P_WPS_CONFIG_DISPLAY == local_ptr->config_Methods)
	{
	    QCLI_Printf(qcli_p2p_group, "Provisional Disc Response Display \r\n");
    }
	else if(__QAPI_WLAN_P2P_WPS_CONFIG_PUSHBUTTON == local_ptr->config_Methods)
	{
	    QCLI_Printf(qcli_p2p_group, "Provisional Disc Response Push Button.\r\n");
	}
	else
	{
	    QCLI_Printf(qcli_p2p_group, "Invalid Provisional Response.\r\n");
    }
    return;
}

void P2P_Event_Handler_Req_To_Auth(CMD_P2P_EVENT_INFO *pEventInfo)
{
    qapi_WLAN_P2P_Req_To_Auth_Event_t *local_ptr = (qapi_WLAN_P2P_Req_To_Auth_Event_t *) pEventInfo->pBuffer;

    QCLI_Printf(qcli_p2p_group, "\r\n source addr : %02x:%02x:%02x:%02x:%02x:%02x \r\n", local_ptr->sa[0], local_ptr->sa[1],
		        local_ptr->sa[2], local_ptr->sa[3], local_ptr->sa[4], local_ptr->sa[5]);
	
    QCLI_Printf(qcli_p2p_group, "\r\n dev_password_id : %x \r\n", local_ptr->dev_Password_Id);
	return;
}

void P2P_Event_Handler_Sdpd_Rx(CMD_P2P_EVENT_INFO *pEventInfo)
{
    qapi_WLAN_P2P_Sdpd_Rx_Event_t *local_ptr = (qapi_WLAN_P2P_Sdpd_Rx_Event_t *) pEventInfo->pBuffer;

    QCLI_Printf(qcli_p2p_group, "Custom_Api_p2p_serv_disc_req event \r\n");
	QCLI_Printf(qcli_p2p_group, "type : %d   frag id : %x \r\n", local_ptr->type, local_ptr->frag_Id);
	QCLI_Printf(qcli_p2p_group, "transaction_status : %x \r\n", local_ptr->transaction_Status);
	QCLI_Printf(qcli_p2p_group, "freq : %d status_code : %d comeback_delay : %d tlv_length : %d update_indic : %d \r\n",
		        local_ptr->freq, local_ptr->status_Code, local_ptr->comeback_Delay, local_ptr->tlv_Length, local_ptr->update_Indic);

	QCLI_Printf(qcli_p2p_group, "source addr : %02x:%02x:%02x:%02x:%02x:%02x \r\n",
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
    QCLI_Printf(qcli_p2p_group, "Invitation Result %d\r\n", local_ptr->status);

	if(local_ptr->status == 0) 
	{
	    QCLI_Printf(qcli_p2p_group, "SSID %02x:%02x:%02x:%02x:%02x:%02x \r\n", local_ptr->bssid[0], local_ptr->bssid[1],
			                                                        local_ptr->bssid[2], local_ptr->bssid[3],
			                                                        local_ptr->bssid[4], local_ptr->bssid[5]);
	}

	if((p2p_peers_data[invitation_index].role == QAPI_WLAN_P2P_INV_ROLE_ACTIVE_GO_E) && (p2p_persistent_done == 0) && (local_ptr->status == 0))
	{
	    p2p_session_in_progress = 1;
		deviceId = get_active_device();
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
				qapi_WLAN_Get_Param (temp_device_id, 
				 __QAPI_WLAN_PARAM_GROUP_WIRELESS,
				 __QAPI_WLAN_PARAM_GROUP_WIRELESS_CHANNEL,
				 &channel,
				 &dataLen);            
			}
			deviceId = 0;
			set_active_deviceid(deviceId);
        }
		else
		{
			temp_device_id = deviceId;
			deviceId = 1;
			set_active_deviceid(deviceId);
			/*
			 * If the device 1 is connected, we start GO on the home channel of device 1.
			 */
			if(wifi_state[deviceId] ==1)
			{
				qapi_WLAN_Get_Param (deviceId, 
				 __QAPI_WLAN_PARAM_GROUP_WIRELESS,
				 __QAPI_WLAN_PARAM_GROUP_WIRELESS_CHANNEL,
				 &channel,
				 &dataLen);            
			}
			deviceId = temp_device_id;
			set_active_deviceid(temp_device_id);			
		}
		wps_flag = 0x01;
	
		QCLI_Printf(qcli_p2p_group, "Starting Autonomous GO \r\n");
		if(qapi_WLAN_P2P_Start_Go(deviceId, NULL, channel, 1) != 0)
		{
			QCLI_Printf(qcli_p2p_group, "\r\nStartP2P command did not execute properly\r\n");
            goto set_temp_device;
		}
	
		p2p_persistent_done = 1;

        set_temp_device:
		if(temp_device_id != 0){
			set_active_deviceid(temp_device_id);
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

	deviceId = get_active_device();
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
			qapi_WLAN_Get_Param (temp_device_id, 
			 __QAPI_WLAN_PARAM_GROUP_WIRELESS,
			 __QAPI_WLAN_PARAM_GROUP_WIRELESS_CHANNEL,
			 &channel,
			 &dataLen);            
		}
		deviceId = 0;
		set_active_deviceid(deviceId);
	}
	else
	{
		temp_device_id = deviceId;
		deviceId = 1;
		set_active_deviceid(deviceId);
		/*
		 * If the device 1 is connected, we start GO on the home channel of device 1.
		 */
		if(wifi_state[deviceId] ==1)
		{
			qapi_WLAN_Get_Param (deviceId, 
			 __QAPI_WLAN_PARAM_GROUP_WIRELESS,
			 __QAPI_WLAN_PARAM_GROUP_WIRELESS_CHANNEL,
			 &channel,
			 &dataLen);            
		}
		deviceId = temp_device_id;
		set_active_deviceid(temp_device_id);			
	}

    local_ptr = (qapi_WLAN_P2P_Invite_Rcvd_Result_Event_t *) pEventInfo->pBuffer;
    memset(&goParams,0, sizeof(qapi_WLAN_P2P_Go_Params_t));
    QCLI_Printf(qcli_p2p_group, "Invite Result Status : %x \r\n", local_ptr->status);

    if (local_ptr->status == 0)
    {
        for (i=0;i<__QAPI_WLAN_MAC_LEN;i++)
        {
            QCLI_Printf(qcli_p2p_group, " [%x] ", local_ptr->sa[i]);
		}
	}
	else
	{
	    qapi_WLAN_P2P_Stop_Find(deviceId);
	}
	QCLI_Printf(qcli_p2p_group, "\r\n");

    if((p2p_peers_data[inv_response_evt_index].role == QAPI_WLAN_P2P_INV_ROLE_ACTIVE_GO_E) && (p2p_persistent_done == 0) &&
		(local_ptr->status == 0))
	{
        // make AP Mode and WPS default settings for P2P GO
        p2p_session_in_progress = 1;

		wps_flag = 0x01;
		QCLI_Printf(qcli_p2p_group, "Starting Autonomous GO \r\n");
   
        goParams.ssid_Len = strlen((char *) p2p_peers_data[inv_response_evt_index].ssid);
		goParams.passphrase_Len = strlen((char *) p2p_peers_data[inv_response_evt_index].passphrase);
		memcpy(goParams.ssid, p2p_peers_data[inv_response_evt_index].ssid, 
											  goParams.ssid_Len);
		memcpy(goParams.passphrase, p2p_peers_data[inv_response_evt_index].passphrase, 
											  goParams.passphrase_Len);

		if(qapi_WLAN_P2P_Start_Go(deviceId, &goParams, channel, 1) != 0)
	    {
	        QCLI_Printf(qcli_p2p_group, "\r\nStartP2P command did not execute properly\r\n");
            goto set_temp_device;
		}

		p2p_persistent_done = 1;
		inv_response_evt_index = 0;
	}

    set_temp_device:
	if(temp_device_id != 0)
	{
		set_active_deviceid(temp_device_id);
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

	QCLI_Printf(qcli_p2p_group, "Invitation Req Received From : ");
	for (i = 0; i < __QAPI_WLAN_MAC_LEN; i++)
    {
        QCLI_Printf(qcli_p2p_group, " %x: ",local_ptr->sa[i]);
	}
    QCLI_Printf(qcli_p2p_group, "\r\n");

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
	deviceId = get_active_device();
	if(deviceId != 0)
	{
	    /* P2P device should always send/receive events/commands on dev 0 if
	     * the app has switched to dev 1 while event is in dev 0 send command
	     * via dev 0 and then switch to dev 1 */
	    temp_device_id = deviceId;
		deviceId = 0;
		set_active_deviceid(deviceId);
	}

    /* send invite auth event */
	if(qapi_WLAN_P2P_Invite_Auth(deviceId, (qapi_WLAN_P2P_Invite_Info_t *)&invite_rsp_cmd) != 0)
    {
        QCLI_Printf(qcli_p2p_group, "\r\nStartP2P (P2P invite auth persistent)command did not execute properly\r\n");
    }
   
    if(temp_device_id != 0)
	{
        set_active_deviceid(temp_device_id);
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

    QCLI_Printf(qcli_p2p_group, "P2P GO Negotiation Result\r\n");
    QCLI_Printf(qcli_p2p_group, "      Status: %s\r\n",(p2pNeg->status) ? "FAILURE":"SUCCESS");

    /* If group negotiation result was a failure then stop processing further. */
	if(p2pNeg->status != 0)
    {
        p2p_persistent_go = QAPI_WLAN_P2P_NON_PERSISTENT_E;
        qapi_WLAN_P2P_Stop_Find(deviceId);
        return;
    }

    QCLI_Printf(qcli_p2p_group, "    P2P Role: %s\r\n",(p2pNeg->role_Go) ? "P2P GO": "P2P Client");
    QCLI_Printf(qcli_p2p_group, "        SSID: %s\r\n", p2pNeg->ssid);
    QCLI_Printf(qcli_p2p_group, "     Channel: %d\r\n", p2pNeg->freq);
    QCLI_Printf(qcli_p2p_group, "  WPS Method: %s\r\n",
		        (p2pNeg->wps_Method == QAPI_WLAN_P2P_WPS_PBC_E) ? "PBC": "PIN");

    p2p_session_in_progress = 1;
    deviceId = get_active_device();
    if(deviceId != 0)
	{

	    /* P2P device should always send/receive events/commands on dev 0 if
		 * the app has switched to dev 1 while event is in dev 0 send command
		 * via dev 0 and then switch to dev 1 */

		temp_device_id = deviceId;
		deviceId = 0;
		set_active_deviceid(deviceId);
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
        if(qapi_WLAN_P2P_Start_Go(deviceId, &goParams, 
                                  chnl, p2pNeg->persistent_Grp) != 0)
        {
            QCLI_Printf(qcli_p2p_group, "\r\nP2P connect command did not execute properly\r\n");
            goto set_temp_device;
        }
		Waiting_Connection_Completion = 1;
        result = qurt_signal_wait_timed(&Utility_Event, CONNECT_EVENT_MASK,
                                        (QURT_SIGNAL_ATTR_WAIT_ALL |
                                         QURT_SIGNAL_ATTR_CLEAR_MASK),
                                        &signal_set, timeOutTicks);
		if(result != QURT_EOK)
		{
			
            QCLI_Printf(qcli_p2p_group, "\r\n Waiting P2P connection failed\r\n");
            Waiting_Connection_Completion = 0;
            goto set_temp_device;
		}
    }

	else if(p2pNeg->role_Go == 0)
	{
        uint8_t ssid[__QAPI_WLAN_MAX_SSID_LENGTH];
		qapi_WLAN_Dev_Mode_e opMode = QAPI_WLAN_DEV_MODE_STATION_E;
		qapi_WLAN_Set_Param(deviceId, __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                        __QAPI_WLAN_PARAM_GROUP_WIRELESS_OPERATION_MODE,
                        &opMode, sizeof(qapi_WLAN_Dev_Mode_e), FALSE);

        memset(ssid, 0, sizeof(ssid));
        memcpy(ssid, p2pNeg->ssid, sizeof(p2pNeg->ssid));
	    error = qapi_WLAN_Set_Param (deviceId,
	                                  __QAPI_WLAN_PARAM_GROUP_WIRELESS,
	                                  __QAPI_WLAN_PARAM_GROUP_WIRELESS_SSID,
	                                  (void *) ssid,
	                                  sizeof(ssid),
	                                  FALSE);
		if(error != 0)
		{
		    QCLI_Printf(qcli_p2p_group, "Unable to set SSID\r\n");
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

    qapi_WLAN_Set_Param (deviceId,
                         __QAPI_WLAN_PARAM_GROUP_WIRELESS_SECURITY,
                         __QAPI_WLAN_PARAM_GROUP_SECURITY_WPS_CREDENTIALS,
                         &wpsCreden,
                         sizeof(qapi_WLAN_WPS_Credentials_t),
                         FALSE);

    /* Start WPS on the Aheros wifi */
    if(qapi_WLAN_WPS_Start(deviceId, wps_context.connect_flag, wps_start.wps_Mode, (char *)wps_start.pin) != 0)
    {
		QCLI_Printf(qcli_p2p_group, "WPS failed.\r\n");
        goto set_temp_device;
    }
    wps_context.wps_in_progress = 1;
	QCLI_Printf(qcli_p2p_group, "WPS started.\r\n");

    set_temp_device:
	if(temp_device_id != 0)
	{
	    set_active_deviceid(temp_device_id);
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
        QCLI_Printf(qcli_p2p_group, "Unknown P2P Event %d\n", pEventInfo->event_id);
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

void app_wlan_p2p_event_cb(uint8_t device_Id, void *pData, uint32_t *pLength)
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
                QCLI_Printf(qcli_p2p_group, " Out of Memory: Dropping the P2P event\n");
                return;
            }

			qurt_signal_set(&Utility_Event, P2P_EVENT_MASK);
            break;
        }

        default:
            QCLI_Printf(qcli_p2p_group, "Unknown P2P event %d\n", pP2p_Event_Cb_Info->event_ID);
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

void wnm_event_cb(uint8_t deviceId, void *payload, uint32_t payloadLength)
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
                QCLI_Printf(qcli_wlan_group, "802.11v configuration succeeded for device %u\r\n",deviceId);
            }
            else {
                QCLI_Printf(qcli_wlan_group, "802.11v configuration failed for device %u\r\n",deviceId);
            }
            break;
          
        case __QAPI_WLAN_PARAM_GROUP_WIRELESS_WNM_AP_BSS_MAX_IDLE_PERIOD:
            if (wnmEvent->response == 0) {
                QCLI_Printf(qcli_wlan_group, "802.11v bss max idle period set for device %u\r\n",deviceId);
            }
            else {
                QCLI_Printf(qcli_wlan_group, "BSS max idle period not set for device %u\r\n",deviceId);
            }
            break;
      
        case __QAPI_WLAN_PARAM_GROUP_WIRELESS_WNM_STA_SLEEP_PERIOD:
            if (wnmEvent->response == 0) {
                QCLI_Printf(qcli_wlan_group, "802.11v WNM sleep command succeeded for device %u\r\n",deviceId);
            }
            else {
                QCLI_Printf(qcli_wlan_group, "802.11v WNM sleep command failed for device %u\r\n",deviceId);
            }
            break;
      
        case __QAPI_WLAN_PARAM_GROUP_WIRELESS_WNM_AP_SLEEP_RESPONSE:
            if (wnmEvent->response == 0) {
                QCLI_Printf(qcli_wlan_group, "802.11v WNM sleep response set for device %u\r\n",deviceId);
            }
            else {
                QCLI_Printf(qcli_wlan_group, "802.11v WNM sleep response command failed for device %u\r\n",deviceId);
            }
            break;
    }
}

void wps_profile_event_cb(uint8_t deviceId, void *payload, uint32_t payloadLength)
{
    if(!payload)
    {
        QCLI_Printf(qcli_wlan_group, "WPS failed\r\n");
        return;
    }

    qapi_WLAN_WPS_Cb_Info_t *event = (qapi_WLAN_WPS_Cb_Info_t *)payload;

    if(event->status == QAPI_WLAN_WPS_STATUS_SUCCESS)
    {
        QCLI_Printf(qcli_wlan_group, "Waiting for WPS completion\r\n");
    }
    else if(event->status == QAPI_WLAN_WPS_STATUS_FAILURE)
    {
        QCLI_Printf(qcli_wlan_group, "WPS failed\r\n");
    }

    /*
     * WPS process is completed. Create a timer to retrieve WPS profile event.
     * In station mode, after receiving the WPS event, the application should send connect request to the AP, if connect flag is set by the user.
     * In soft-AP mode, it waits for the connect request from the station.
     */
	qurt_signal_set(&Utility_Event, WPS_EVENT_MASK);
    return;
}

int32_t wlan_callback_handler( uint8_t  deviceId, 
                               uint32_t cbId, 
                               void *pApplicationContext,
                               void     *payload,
                               uint32_t payload_Length)
{
    static uint16_t total_scan_count = 0;
   
    switch(cbId)
    {
     case QAPI_WLAN_CONNECT_CB_E:  
       {
         qapi_WLAN_Connect_Cb_Info_t *cxnInfo  = (qapi_WLAN_Connect_Cb_Info_t *)(payload);
         wifi_connect_handler(cxnInfo->value, deviceId, cxnInfo->mac_Addr, cxnInfo->bss_Connection_Status);
		 if(Waiting_Connection_Completion)
		 {	
		 	qurt_signal_set(&Utility_Event, CONNECT_EVENT_MASK);
		 	Waiting_Connection_Completion = 0;
		 }
         if(payload_Length >= sizeof(qapi_WLAN_Connect_Cb_Info_ExtV1_t))
         {
            //qapi_WLAN_Connect_Cb_Info_ExtV1_t *pCxnInfo_Ext = (qapi_WLAN_Connect_Cb_Info_ExtV1_t *)(payload);
            //QCLI_Printf(qcli_wlan_group, "Connect_Cb_Ext channel=%d beacon_Interval=%d listen_Interval=%d\r\n",
             //   pCxnInfo_Ext->channel, pCxnInfo_Ext->beacon_Interval, pCxnInfo_Ext->listen_Interval);
            //Dump_Data(qcli_wlan_group, "[wpaie]", pCxnInfo_Ext->beacon_IEs_Len, pCxnInfo_Ext->beacon_IEs);
            //Dump_Data(qcli_wlan_group, "[assreq]", pCxnInfo_Ext->req_IEs_Len, pCxnInfo_Ext->req_IEs);
            //Dump_Data(qcli_wlan_group, "[assrsp]", pCxnInfo_Ext->resp_IEs_Len, pCxnInfo_Ext->resp_IEs);
         }
       }
       break;
    case QAPI_WLAN_FWD_PROBE_REQUEST_INFO_CB_E:
       {
         qapi_WLAN_Fwd_Probe_Request_Cb_Info_t *probeReqInfo = (qapi_WLAN_Fwd_Probe_Request_Cb_Info_t *) (payload);
         probe_req_handler(probeReqInfo->probe_Req_Buffer, probeReqInfo->buffer_Length, probeReqInfo->frequency);
       }
       break;
    case QAPI_WLAN_PROMISCUOUS_MODE_CB_INFO_CB_E:
       {
           promisc_frame_handler(payload, payload_Length);
       }
       break;
    case QAPI_WLAN_SCAN_COMPLETE_CB_E:
      {
        qapi_WLAN_Scan_List_t scan_results;
        int32_t error = 0;
		qapi_WLAN_Scan_Status_t *pScanModeStatus = (qapi_WLAN_Scan_Status_t*)payload;
        uint16_t scan_Mode = pScanModeStatus->scan_Mode;
		uint16_t scan_Status = pScanModeStatus->scan_Status;
        
        if(scan_Mode == QAPI_WLAN_NO_BUFFERING_E)
        {
            QCLI_Printf(qcli_wlan_group, "Scan result count:%d\r\n", total_scan_count);
            total_scan_count = 0;
			if(scan_Status == QAPI_WLAN_SCAN_STATUS_REQUIRE_RESCAN_E)
			{
				QCLI_Printf(qcli_wlan_group,"Require to scan again.\r\n");
			}
            return error;
        }
        scan_results.scan_List = malloc((sizeof(qapi_WLAN_BSS_Scan_Info_t) * __QAPI_MAX_SCAN_RESULT_ENTRY));
        scan_results.num_Scan_Entries = __QAPI_MAX_SCAN_RESULT_ENTRY;
        error = qapi_WLAN_Get_Scan_Results(deviceId, (qapi_WLAN_BSS_Scan_Info_t *)(scan_results.scan_List), (int16_t *)&(scan_results.num_Scan_Entries));
        if (0 != error){
		  QCLI_Printf(qcli_wlan_group,"Failed to get scan results.\r\n");
		  free(scan_results.scan_List);
          return error;
        }
        error = print_scan_results(scan_results, scan_results.num_Scan_Entries);
        QCLI_Printf(qcli_wlan_group, "Scan result count:%d\r\n", scan_results.num_Scan_Entries);
		if(scan_Status == QAPI_WLAN_SCAN_STATUS_REQUIRE_RESCAN_E)
		{
			QCLI_Printf(qcli_wlan_group,"Require to scan again.\r\n");
		}
        //A_FREE(scan_results.scan_List, );
        free(scan_results.scan_List);
        return error;
      }
    case QAPI_WLAN_BSS_INFO_CB_E:
      {
        int32_t error = 0;
        qapi_WLAN_Scan_List_t ScanInfo;
        uint16_t scan_count_received = (uint16_t)(payload_Length/sizeof(qapi_WLAN_BSS_Scan_Info_t));
        
        total_scan_count += scan_count_received;
        ScanInfo.scan_List = (qapi_WLAN_Scan_List_t *)(payload);
        error = print_scan_results(ScanInfo, scan_count_received);

        if (payload_Length >= sizeof(qapi_WLAN_BSS_Scan_Info_ExtV1_t))
        {
			
            //qapi_WLAN_BSS_Scan_Info_ExtV1_t *pScanInfo_Ext = (qapi_WLAN_BSS_Scan_Info_ExtV1_t *)payload;
            //QCLI_Printf(qcli_wlan_group, "Found one BSS beacon=%d len=%d\r\n",
            //    pScanInfo_Ext->is_beacon, pScanInfo_Ext->beacon_IEs_Len);
            //Dump_Data(qcli_wlan_group, "[BeaCon]", pScanInfo_Ext->beacon_IEs_Len, pScanInfo_Ext->beacon_IEs);
        }
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
	    QCLI_Printf(qcli_wlan_group, "\r\nSeq num = %u, Ack seq = %u, srcPort = %d dstPort = %d status = %d", event_info_ptr->sequence_Num, event_info_ptr->ack_Sequence_Num, event_info_ptr->src_Port, event_info_ptr->dst_Port, event_info_ptr->status);
			event_info_ptr++;
         }
      }
      break;    
#if ENABLE_P2P_MODE
    case QAPI_WLAN_P2P_CB_E:
       {
           app_wlan_p2p_event_cb(deviceId, payload, &payload_Length);
           break;
       }
#endif
	 case QAPI_WLAN_WNM_CB_E:
		 wnm_event_cb(deviceId, payload, payload_Length);
         break;

     case QAPI_WLAN_WPS_CB_E:
         wps_profile_event_cb(deviceId, payload, payload_Length);
         break;

	 case QAPI_WLAN_PREFERRED_NETWORK_OFFLOAD_CB_E:
     {
         qapi_WLAN_Preferred_Network_Offload_Info_t *profile = NULL;
		 profile = (qapi_WLAN_Preferred_Network_Offload_Info_t *) payload;
         if(profile->profile_Matched)
         {
             QCLI_Printf(qcli_wlan_group, "\r\n Rx PNO Event: Profile matched, Profile Index: %u, RSSI: %d, Remaining Fast Scans: %u\n",
			 	                          profile->matched_Index, profile->rssi, profile->num_Fast_Scans_Remaining);
         }
         else
         {
             QCLI_Printf(qcli_wlan_group, "\r\n Rx PNO Event: Profile not matched, Profile Index: %u, RSSI: %d, Remaining Fast Scans: %u\n",
			 	                          profile->matched_Index, profile->rssi, profile->num_Fast_Scans_Remaining);
         }
         break;
     }
	 case QAPI_WLAN_ERROR_HANDLER_CB_E:
     {
         QCLI_Printf(qcli_wlan_group,"Fatal error occured. Error= ");

         /* take necessary action based on error */
         QAPI_FATAL_ERR(0,0,0);
     }
     case QAPI_WLAN_RESUME_HANDLER_CB_E:
     {
         QCLI_Printf(qcli_wlan_group, "\r\n WLAN FIRMWARE RESUME Completed");
     }

     default:
         break;
    } 
    return 0;
}

uint32_t enable_probe_req_event(int32_t enable)
{
    int deviceId = get_active_device();
    return qapi_WLAN_Set_Param(deviceId, 
               __QAPI_WLAN_PARAM_GROUP_WIRELESS, 
               __QAPI_WLAN_PARAM_GROUP_WIRELESS_ENABLE_PROBE_REQ_FWD_TO_HOST, 
               &enable, 
               sizeof(uint32_t),
               FALSE);
}
uint16_t freq_to_channel(uint16_t channel_val)
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

#if ENABLE_SCC_MODE 
static int32_t handle_connect_for_concurrent(uint32_t deviceId, char* ssid)
{
    uint8_t scan_ssid[__QAPI_WLAN_MAX_SSID_LENGTH];

    memset(scan_ssid,0,__QAPI_WLAN_MAX_SSID_LENGTH);    

    strcpy((char*)scan_ssid,ssid);
    strcpy((char*)ssid_str_concurrent, ssid);
    QCLI_Printf(qcli_wlan_group, "remote ap ssid: %s\r\n", ssid_str_concurrent);
    if (0 != qapi_WLAN_Set_Param (deviceId,
                                     __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                                     __QAPI_WLAN_PARAM_GROUP_WIRELESS_SSID,
                                     (void *) scan_ssid,
                                     strlen((char *) scan_ssid),
                                     FALSE))
    {
      QCLI_Printf(qcli_wlan_group, "Unable to set SSID for scan\r\n");
      return -1;
    }  

    ap_channel_hint = 0;    
    concurrent_connect_flag = 0x0F;        
    /*Do the actual scan*/
    wmi_set_scan(deviceId, NULL, QAPI_WLAN_BUFFER_SCAN_RESULTS_BLOCKING_E); 
    concurrent_connect_flag = 0x0E;    
    if( ap_channel_hint == 0)
    {
        QCLI_Printf(qcli_wlan_group, "Unable to find AP: %s.....\r\n", ssid);
        return -1;
    }
    QCLI_Printf(qcli_wlan_group, "find AP...\r\n");
    A_MDELAY(50);
         
    if ( 0 != qapi_WLAN_Set_Param(deviceId,
                                     __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                                     __QAPI_WLAN_PARAM_GROUP_WIRELESS_CHANNEL,
                                     (void *) &ap_channel_hint,
                                     sizeof(ap_channel_hint),
                                     FALSE))
    {
        QCLI_Printf(qcli_wlan_group, "Unable to set AP channel:%d\r\n",ap_channel_hint);
        return -1;
    }
    A_MDELAY(50);
    return 0;
}



#endif

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : connect_handler()
* Returned Value  : 0 - successful completion or
*                    -1 - failed.
* Comments          : Handles Connect commands for infrastructure mode, Open
*                   WEP,WPA/WPA2 security is supported
*
*END*-----------------------------------------------------------------*/

int32_t connect_to_network(const char* ssid, const char* bssid)
{
    int32_t error = 0;
    int ssidLength = 0;
    uint32_t deviceId = 0, temp_mode = 0, dataLen = 0;           
	
    deviceId = get_active_device();
    set_callback(NULL);
    ssidLength = strlen(ssid);
#if ENABLE_SCC_MODE     
    if((concurrent_connect_flag == 0x0E) && (deviceId == 1))
    {
        error = handle_connect_for_concurrent(deviceId, ssid);
        if(error != 0)
        {
           QCLI_Printf(qcli_wlan_group, "cannot connect for concurrent mode\r\n");
           return -1;
        }
    }
#endif

    if(wps_should_disable)
    {
        if (0 != qapi_WLAN_Set_Param(deviceId, __QAPI_WLAN_PARAM_GROUP_WIRELESS,__QAPI_WLAN_PARAM_GROUP_WIRELESS_AP_WPS_FLAG, 
            &wps_flag, sizeof(wps_flag),FALSE))
        {
            QCLI_Printf(qcli_wlan_group, "Disable wps mode failed \r\n");
            return QCLI_STATUS_ERROR_E;
        }
	}
    
    qapi_WLAN_Get_Param (deviceId, 
                         __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                         __QAPI_WLAN_PARAM_GROUP_WIRELESS_OPERATION_MODE,
                         &temp_mode,
                         &dataLen);
    if (temp_mode == QAPI_WLAN_DEV_MODE_AP_E)
    {
        QCLI_Printf(qcli_wlan_group, "setting to ap mode \r\n");
    }
    if ((SEC_MODE_OPEN == security_mode) && (temp_mode == QAPI_WLAN_DEV_MODE_AP_E))
    {
        QCLI_Printf(qcli_wlan_group, "AP in OPEN mode!\r\n");
    }

    /*
    if(argc > 2)
    {
        for(i=index;i<argc;i++)
        {
            if((ssidLength < 0) || (strlen(argv[i]) > ssidLength))
            {
                QCLI_Printf(qcli_wlan_group, "SSID length exceeds Maximum value\r\n");
                return -1;
            }
            strcpy(&ssid_str[offset], argv[i]);
            offset += strlen(argv[i]);
            ssidLength -= strlen(argv[i]);

            //Add empty space in ssid except for last word
            if(i != argc-1)
            {
                strcpy(&ssid_str[offset], " ");
                offset += 1;
                ssidLength -= 1;
            }
        }
    }*/
    if (ssidLength > __QAPI_WLAN_MAX_SSID_LENGTH || ssidLength < 0) 
    {
       QCLI_Printf(qcli_wlan_group, "SSID length exceeds Maximum value\r\n");
       return -1;
    }
    error = qapi_WLAN_Set_Param (deviceId,
                                 __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                                 __QAPI_WLAN_PARAM_GROUP_WIRELESS_SSID,
                                 (void *) ssid,
                                 ssidLength,
                                 FALSE);
    if (error != 0)
    {
        QCLI_Printf(qcli_wlan_group,"Error during setting of ssid %s error=%08x!\r\n", ssid, error);
        return error;
    }

    QCLI_Printf(qcli_wlan_group,"\r\nSetting SSID to %s \r\n\r\n",ssid);
    strcpy((char*)original_ssid, ssid);

	if (temp_mode == QAPI_WLAN_DEV_MODE_STATION_E)
    {
    	uint8_t bssidToConnect[__QAPI_WLAN_MAC_LEN] = {0};
		if (bssid && (ether_aton(bssid, bssidToConnect) != 0))
		{
			QCLI_Printf(qcli_wlan_group, "Invalid BSSID to connect\r\n");
			return QCLI_STATUS_ERROR_E;
		}
	    error = qapi_WLAN_Set_Param (deviceId,
                             __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                             __QAPI_WLAN_PARAM_GROUP_WIRELESS_BSSID,
                             bssidToConnect,
                             __QAPI_WLAN_MAC_LEN,
                             FALSE);
	    if (error != 0)
	    {
	        QCLI_Printf(qcli_wlan_group,"Error during setting of bssid  error=%08x!\r\n", error);
	        return error;
	    }

		if(bssid)
			QCLI_Printf(qcli_wlan_group,"\r\nSetting BSSID to %s \r\n",bssid);
		
	}
	
    if (SEC_MODE_WEP == security_mode) {
        cipher = QAPI_WLAN_CRYPT_WEP_CRYPT_E;
        if ( 0 != qapi_WLAN_Set_Param(deviceId,
                                         __QAPI_WLAN_PARAM_GROUP_WIRELESS_SECURITY,
                                         __QAPI_WLAN_PARAM_GROUP_SECURITY_ENCRYPTION_TYPE,
                                         &cipher,
                                         sizeof(qapi_WLAN_Crypt_Type_e),
                                         FALSE))

        {
                return -1;
        }
        security_mode = SEC_MODE_OPEN;
    }      
    else if(SEC_MODE_WPA == security_mode)
    {
        uint32_t passphraseLen = 0;

        if(0 != qapi_WLAN_Set_Param(deviceId,
                                           __QAPI_WLAN_PARAM_GROUP_WIRELESS_SECURITY,
                                           __QAPI_WLAN_PARAM_GROUP_SECURITY_ENCRYPTION_TYPE,
                                           (void *) &cipher, //cipher is set in set_wpa
                                           sizeof(qapi_WLAN_Crypt_Type_e), 
                                           FALSE))
        {
            return -1;
        }

        if( 0 != qapi_WLAN_Set_Param (deviceId,
                                            __QAPI_WLAN_PARAM_GROUP_WIRELESS_SECURITY,
                                            __QAPI_WLAN_PARAM_GROUP_SECURITY_AUTH_MODE,
                                            (void *) &wpa_ver,
                                            sizeof(qapi_WLAN_Auth_Mode_e),
                                            FALSE))
        {
            return -1;
        }
        if ((wpa_ver!=QAPI_WLAN_AUTH_WPA_E) && (wpa_ver!=QAPI_WLAN_AUTH_WPA2_E)) {
            passphraseLen  = strlen(wpa_passphrase[deviceId]);
            if((passphraseLen >= 8) && (passphraseLen <= 63))
            {
                    if (0 != qapi_WLAN_Set_Param(deviceId,
                                                    __QAPI_WLAN_PARAM_GROUP_WIRELESS_SECURITY,
                                                    __QAPI_WLAN_PARAM_GROUP_SECURITY_PASSPHRASE,
                                                    (void *) wpa_passphrase[deviceId],
                                                     passphraseLen,
                                                     FALSE))
                {
                    QCLI_Printf(qcli_wlan_group, "Unable to set passphrase\r\n");
                    return -1;
                }
                pmk_flag[deviceId] = 1;
            }
            else if(passphraseLen == 64)
            {
                if (0 != qapi_WLAN_Set_Param(deviceId,
                                                    __QAPI_WLAN_PARAM_GROUP_WIRELESS_SECURITY,
                                                    __QAPI_WLAN_PARAM_GROUP_SECURITY_PMK,
                                                    (void *) wpa_passphrase[deviceId],
                                                    passphraseLen,
                                                    FALSE))
                {
                    QCLI_Printf(qcli_wlan_group, "Unable to set pmk\r\n");
                    return -1;
                }
                pmk_flag[deviceId] = 1;
            }
            else
            {
                QCLI_Printf(qcli_wlan_group, "invalid password\r\n");
                return -1;
            }
        }
        security_mode = SEC_MODE_OPEN;
    }

    error = qapi_WLAN_Commit(deviceId);
    if(error != 0)
    {
#if ENABLE_AP_MODE
        if(temp_mode == MODE_AP_E)
        {
            qapi_WLAN_Auth_Mode_e authMode = QAPI_WLAN_AUTH_NONE_E;
            QCLI_Printf(qcli_wlan_group, "failed to AP mode \r\n");
#if ENABLE_SCC_MODE 
            QCLI_Printf(qcli_wlan_group, "support single concurrent channel only\r\n");
#endif                        
        // Clear Invalid configurations
        if ( 0 != qapi_WLAN_Set_Param (deviceId,
                                              __QAPI_WLAN_PARAM_GROUP_WIRELESS_SECURITY,
                                              __QAPI_WLAN_PARAM_GROUP_SECURITY_AUTH_MODE,
                                              (void *) &authMode,
                                              sizeof(qapi_WLAN_Auth_Mode_e),
                                              FALSE))
        {
        QCLI_Printf(qcli_wlan_group, "Unable to clear Sec mode\r\n");
        return -1;
        }
            
        security_mode = SEC_MODE_OPEN;
        pmk_flag[deviceId] = 0;
        }
#endif
        return -1;
    }
#if ENABLE_AP_MODE
    if (temp_mode == MODE_AP_E)
    {
        /* setting the AP's default IP to 192.168.1.1 */
    }
#endif

    return 0;
}

uint32_t get_wlan_channel_list()
{
	uint32_t deviceId = 0, length = 0;
	qapi_WLAN_Get_Channel_List_t *wlanChannelList;
	int32_t ret = -1;

	deviceId = get_active_device();
	
	if ( NULL == (wlanChannelList = malloc(sizeof(qapi_WLAN_Get_Channel_List_t)) ) ){		
     	/* Failure to allocate memory will drop the event at caller */
		return -1;
	}
	length = sizeof(wlanChannelList->channel_List);
			
	ret = qapi_WLAN_Get_Param (deviceId, 
							 __QAPI_WLAN_PARAM_GROUP_WIRELESS,
							 __QAPI_WLAN_PARAM_GROUP_WIRELESS_GET_CURR_REGDOMAIN_CHANNEL_LIST,
							 wlanChannelList,
							 &length);
	
    if( ret == 0 )
    {
        print_wlan_channel_list( wlanChannelList );
    }
	else
        QCLI_Printf(qcli_wlan_group, "Failed to Get Current Regulatory Channel List \r\n");

	free(wlanChannelList);
	
    return ret;

}

uint32_t get_wlan_stats( uint8_t flag )
{
    uint32_t deviceId = 0, length = 0;
    qapi_WLAN_Get_Statistics_t getStats;
    int32_t ret = -1;
    
    deviceId = get_active_device();

    getStats.reset_Counters_Flag = flag;
    getStats.wlan_Stats_Data = (qapi_WLAN_Statistics_t*)malloc(sizeof(qapi_WLAN_Statistics_t));
    length = sizeof(getStats);
    
    ret = qapi_WLAN_Get_Param (deviceId, 
                         __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                         __QAPI_WLAN_PARAM_GROUP_WIRELESS_STATS,
                         &getStats,
                         &length);

    
    if( ret == 0 )
    {
        print_wlan_stats(getStats.wlan_Stats_Data );    // To do to copy data to Application buffer
        free(getStats.wlan_Stats_Data);
    }
   
    return ret;
}
/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : ad_hoc_connect_handler()
* Returned Value  : 1 - successful completion or
*                    0 - failed.
* Comments          : Handles Connect commands for ad-hoc mode, Open
*                   & WEP security is supported
*
*END*-----------------------------------------------------------------*/
int32_t set_wep_keyix(int32_t key_idx);
int32_t ad_hoc_connect_handler(char *ssid, char *sec_mode, int32_t key_index, int32_t channel)
{
    int32_t error= 0;
    uint32_t deviceId = 0;
    qapi_WLAN_Dev_Mode_e opMode;
    char ssid_str[__QAPI_WLAN_MAX_SSID_LENGTH] = {'\0'};
    
    deviceId = get_active_device();
    set_callback(NULL);
    
    opMode = QAPI_WLAN_DEV_MODE_ADHOC_E;
    qapi_WLAN_Set_Param(deviceId, 
                __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                __QAPI_WLAN_PARAM_GROUP_WIRELESS_OPERATION_MODE,
                &opMode,
                sizeof(qapi_WLAN_Dev_Mode_e),
                FALSE);

    if( strlen(ssid) > __QAPI_WLAN_MAX_SSID_LEN )
    {
        QCLI_Printf(qcli_wlan_group, "Invalid SSID length\r\n");
        return -1;
    }
    strcpy(&ssid_str[0], ssid);
    error = qapi_WLAN_Set_Param (deviceId,
                             __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                             __QAPI_WLAN_PARAM_GROUP_WIRELESS_SSID,
                             (void *) ssid_str,
                             strlen((char *) ssid_str),
                             FALSE);
    if (error != 0)
    {
        QCLI_Printf(qcli_wlan_group, "Error during setting of ssid %s error=%08x!\r\n", ssid, error);
        return error;
    }

    QCLI_Printf(qcli_wlan_group, "Setting SSID to %s \r\n\r\n", ssid);
    strcpy((char*)original_ssid, ssid);

    if(APP_STRCMP(sec_mode, "wep") == 0)
    {
		if(set_wep_keyix(key_index) != 0)
		{
			QCLI_Printf(qcli_wlan_group, "Setting wep key to driver is failed!\r\n");
			return -1;
		}

    }

    if ( 0 != qapi_WLAN_Set_Param(deviceId,
                                  __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                                  __QAPI_WLAN_PARAM_GROUP_WIRELESS_CHANNEL,
                                  (void *) &channel,
                                  sizeof(channel),
                                  FALSE))
    {
       QCLI_Printf(qcli_wlan_group, "Invalid channel!\r\n");
       return -1;
    }
    else
        QCLI_Printf(qcli_wlan_group, "Setting Channel to %d\r\n", channel);
        
    if ( 0 != qapi_WLAN_Commit(deviceId)){
        return -1;
    }
    return 0;
}


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : set_wpa()
* Returned Value  : 0 - successful completion or
*            -1 - failed.
* Comments    : Sets WPA mode
*
*END*-----------------------------------------------------------------*/
int32_t set_wpa(char *wpaVer, char *ucipher, char *mcipher)
{
    uint32_t wifi_mode = 0, deviceId = 0, dataLen = 0;

    deviceId = get_active_device();
    if (0 != qapi_WLAN_Get_Param (deviceId, 
                         __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                         __QAPI_WLAN_PARAM_GROUP_WIRELESS_OPERATION_MODE,
                         &wifi_mode,
                         &dataLen))
    {
        QCLI_Printf(qcli_wlan_group, "set_wpa failed.\r\n");
        return -1;
    }

    /********** wpa version ****************/
    if(APP_STRCMP(wpaVer,"WPA")==0)
    {
       wpa_ver = QAPI_WLAN_AUTH_WPA_PSK_E;
    }
    else if(APP_STRCMP(wpaVer,"WPA2")==0)
    {
       wpa_ver = QAPI_WLAN_AUTH_WPA2_PSK_E;
    }
    else if((wifi_mode == QAPI_WLAN_DEV_MODE_STATION_E) && (APP_STRCMP(wpaVer,"WPACERT")==0))
    {
       wpa_ver = QAPI_WLAN_AUTH_WPA_E;
    }
    else if((wifi_mode == QAPI_WLAN_DEV_MODE_STATION_E) && (APP_STRCMP(wpaVer,"WPA2CERT")==0))
    {
       wpa_ver = QAPI_WLAN_AUTH_WPA2_E;
    }
    else
    {
       QCLI_Printf(qcli_wlan_group, "Invalid version\r\n");
       return -1;
    }

    /**************** cipher **********/
    if (!APP_STRCMP(ucipher, mcipher))
    {
        if (!APP_STRCMP(ucipher, "TKIP"))
        {
            cipher = QAPI_WLAN_CRYPT_TKIP_CRYPT_E;
        }
        else if (!APP_STRCMP(ucipher, "CCMP"))
        {
            cipher = QAPI_WLAN_CRYPT_AES_CRYPT_E;
        }
        else
        {
            QCLI_Printf(qcli_wlan_group, "invaid uchipher mcipher, should be TKIP or CCMP\r\n");
            return -1;
        }
    }
    else
    {
        QCLI_Printf(qcli_wlan_group, "invaid uchipher mcipher, should be same\r\n");
        return -1;
    }

    //Disable WSC if the APUT is manually configured for WPA/TKIP only.
    if((wps_flag == 1) && (wpa_ver == QAPI_WLAN_AUTH_WPA_PSK_E) && (cipher == QAPI_WLAN_CRYPT_TKIP_CRYPT_E))
    {
        wps_should_disable = 1;
        wps_flag = 0;
    }
                                
    security_mode = SEC_MODE_WPA;
    return 0;
}

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : set_wpa_cert()
* Returned Value  : 0 - successful completion or
*            -1 - failed.
* Comments    : Sets WPA enterprise mode
*
*END*-----------------------------------------------------------------*/
int32_t set_wpa_cert(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    uint32_t deviceId = get_active_device();
    uint8_t param_index = 0;
    uint8_t param_index_max = Parameter_Count - 1;
    char *method = NULL, *id = NULL, *username = NULL, *password = NULL;
    char *param_str = NULL;
    qapi_WLAN_8021X_Method_e q_method = QAPI_WLAN_8021X_METHOD_UNKNOWN;
    int debug_level = -1;

    if (param_index_max > 4) {
        param_index_max = 4;
    }
    while (param_index <= param_index_max) {
        param_str = Parameter_List[param_index].String_Value;
        switch (param_index) {
            case 0:
                method = param_str;
                break;
            case 1:
                id = param_str;
                break;
            case 2:
                username = param_str;
                break;
            case 3:
                password = param_str;
                break;
            case 4:
                debug_level = Parameter_List[param_index].Integer_Value;
                break;
            default:
                break;
        }
        param_index++;
    }

    if (debug_level >= 0) {
        qapi_WLAN_Set_Param (deviceId,
                             __QAPI_WLAN_PARAM_GROUP_WIRELESS_SECURITY,
                             __QAPI_WLAN_PARAM_GROUP_SECURITY_DEBUG_LEVEL,
                             (void *)&debug_level, sizeof(int), FALSE);
    }

    /********** wpa version ****************/
    if(APP_STRCMP(method, "TLS")==0) {
        q_method = QAPI_WLAN_8021X_METHOD_EAP_TLS_E;
    } else if(APP_STRCMP(method,"TTLS-MSCHAPV2")==0) {
        q_method = QAPI_WLAN_8021X_METHOD_EAP_TTLS_MSCHAPV2_E;
    } else if(APP_STRCMP(method,"PEAP-MSCHAPV2")==0) {
        q_method = QAPI_WLAN_8021X_METHOD_EAP_PEAP_MSCHAPV2_E;
    } else if(APP_STRCMP(method,"TTLS-MD5")==0) {
        q_method = QAPI_WLAN_8021X_METHOD_EAP_TTLS_MD5_E;
    } else {
        QCLI_Printf(qcli_wlan_group, "invaid method\r\n");
        return -1;
    }

    if ((q_method > QAPI_WLAN_8021X_METHOD_EAP_TLS_E)
        && (!username || !password)) {
        QCLI_Printf(qcli_wlan_group, "Need username and password\r\n");
        return -1;
    }

    if (0 != qapi_WLAN_Set_Param (deviceId,
                         __QAPI_WLAN_PARAM_GROUP_WIRELESS_SECURITY,
                         __QAPI_WLAN_PARAM_GROUP_SECURITY_8021X_METHOD,
                         (void *) &q_method, sizeof(q_method), FALSE)) {
        QCLI_Printf(qcli_wlan_group, "set method failed.\r\n");
        return -1;
    }

    if (0 != qapi_WLAN_Set_Param (deviceId,
                         __QAPI_WLAN_PARAM_GROUP_WIRELESS_SECURITY,
                         __QAPI_WLAN_PARAM_GROUP_SECURITY_8021X_IDENTITY,
                         (void *) id, strlen(id)+1, FALSE)) {
        QCLI_Printf(qcli_wlan_group, "set id failed.\r\n");
        return -1;
    }

    if (q_method > QAPI_WLAN_8021X_METHOD_EAP_TLS_E) {
        if (0 != qapi_WLAN_Set_Param (deviceId,
                             __QAPI_WLAN_PARAM_GROUP_WIRELESS_SECURITY,
                             __QAPI_WLAN_PARAM_GROUP_SECURITY_8021X_USERNAME,
                             (void *) username, strlen(username)+1, FALSE)) {
            QCLI_Printf(qcli_wlan_group, "set username failed.\r\n");
            return -1;
        }
        if (0 != qapi_WLAN_Set_Param (deviceId,
                             __QAPI_WLAN_PARAM_GROUP_WIRELESS_SECURITY,
                             __QAPI_WLAN_PARAM_GROUP_SECURITY_8021X_PASSWORD,
                             (void *) password, strlen(password)+1, FALSE)) {
            QCLI_Printf(qcli_wlan_group, "set password failed.\r\n");
            return -1;
        }
    }

    return 0;
}

/*FUNCTION*-------------------------------------------------------------
*
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
*
*END*-----------------------------------------------------------------*/

int32_t pno_enable(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
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
        QCLI_Printf(qcli_wlan_group, "Disabling PNO\r\n");
        param.start_Network_List_Offload = FALSE;
    }
	else if (Parameter_Count < 5)
	{
        QCLI_Printf(qcli_wlan_group, "Invalid Params.");
        return QCLI_STATUS_USAGE_E;
    }

    if (enable)
    {
        param.start_Network_List_Offload = TRUE;
        param.max_Num_Preferred_Network_Profiles = Parameter_List[1].Integer_Value;
        param.fast_Scan_Interval_In_Ms = Parameter_List[2].Integer_Value;
        param.fast_Scan_Duration_In_Ms = Parameter_List[3].Integer_Value;
        param.slow_Scan_Interval_In_Ms = Parameter_List[4].Integer_Value;
        set_callback(NULL);
    }

    if (0 != qapi_WLAN_Set_Param(get_active_device(),
                                    __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                                    __QAPI_WLAN_PARAM_GROUP_WIRELESS_PREFERRED_NETWORK_OFFLOAD_ENABLE,
                                    (void *) &param,
                                    sizeof(qapi_WLAN_Preferred_Network_Offload_Config_t),
                                    FALSE))
    {
        QCLI_Printf(qcli_wlan_group, "ERROR: driver command failed\r\n");
        return -1;
    }
    return 0;
}

/*FUNCTION*-------------------------------------------------------------
*
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
*
*END*-----------------------------------------------------------------*/
int32_t set_pno_network_profile(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    uint8_t networkIndex = 0;
    qapi_WLAN_Preferred_Network_Profile_t param;
    uint32_t wifi_mode = 0, deviceId = 0, dataLen = 0;

    deviceId = get_active_device();
    if (0 != qapi_WLAN_Get_Param (deviceId, 
                         __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                         __QAPI_WLAN_PARAM_GROUP_WIRELESS_OPERATION_MODE,
                         &wifi_mode,
                         &dataLen))
    {
        QCLI_Printf(qcli_wlan_group, "set_pno_network_profile failed.\r\n");
        return -1;
    }

    if (Parameter_Count < 3 || !Parameter_List[0].Integer_Is_Valid || Parameter_List[2].Integer_Is_Valid) {
        QCLI_Printf(qcli_wlan_group, "Invalid Params. \r\nUsage: WLAN SetPnoNetworkProfile <index> <ssid> <mode=open|wep|wpa|wpa2> [wpa_cipher]\r\n");
        return QCLI_STATUS_USAGE_E;
    }

    memset(&param, 0, sizeof(qapi_WLAN_Preferred_Network_Profile_t));

    /* Parse network index value */
    networkIndex = Parameter_List[0].Integer_Value;
    if (networkIndex >= 0 && networkIndex < __QAPI_WLAN_PNO_MAX_NETWORK_PROFILES){
        param.index = networkIndex;
    } 
    else {
        QCLI_Printf(qcli_wlan_group, "Invalid index value\r\n");
        return -1;
    }
        
    param.ssid_Len = strlen((char *)Parameter_List[1].String_Value);
    if(param.ssid_Len > __QAPI_WLAN_MAX_SSID_LEN) {
        QCLI_Printf(qcli_wlan_group, "invalid ssid value\r ssid length is %d\n", param.ssid_Len);
        return -1;        
    }
    strncpy((char*)&param.ssid[0], (char *)Parameter_List[1].String_Value, param.ssid_Len);
    
    if (APP_STRCMP(Parameter_List[2].String_Value, "open") == 0) {
        param.auth_Mode = QAPI_WLAN_AUTH_NONE_E;
        param.encryption_Type = QAPI_WLAN_CRYPT_NONE_E;
    }
    else if (APP_STRCMP(Parameter_List[2].String_Value, "wep") == 0) {
        param.auth_Mode =  QAPI_WLAN_AUTH_WEP_E;
        param.encryption_Type = QAPI_WLAN_CRYPT_WEP_CRYPT_E;
    }
    else if (APP_STRCMP(Parameter_List[2].String_Value, "wpa") == 0) {
        /********** wpa version ****************/
        param.auth_Mode = QAPI_WLAN_AUTH_WPA_PSK_E;
        if (!APP_STRCMP(Parameter_List[3].String_Value, "TKIP")) 
          param.encryption_Type = QAPI_WLAN_CRYPT_TKIP_CRYPT_E;
        else if (!APP_STRCMP(Parameter_List[3].String_Value, "CCMP"))
           param.encryption_Type =  QAPI_WLAN_CRYPT_AES_CRYPT_E;
        else
           QCLI_Printf(qcli_wlan_group, "invaid uchipher, should be TKIP or CCMP\r\n");
    }
    else if (APP_STRCMP(Parameter_List[2].String_Value, "wpa2")==0) {
        param.auth_Mode = QAPI_WLAN_AUTH_WPA2_PSK_E;
        if (!APP_STRCMP(Parameter_List[3].String_Value, "TKIP")) 
          param.encryption_Type = QAPI_WLAN_CRYPT_TKIP_CRYPT_E;
        else if (!APP_STRCMP(Parameter_List[3].String_Value, "CCMP"))
           param.encryption_Type =  QAPI_WLAN_CRYPT_AES_CRYPT_E;
        else
           QCLI_Printf(qcli_wlan_group, "invaid uchipher, should be TKIP or CCMP\r\n");
    }
    else if ((wifi_mode == QAPI_WLAN_DEV_MODE_STATION_E) && APP_STRCMP(Parameter_List[2].String_Value, "wpacert")==0) {
        param.auth_Mode = QAPI_WLAN_AUTH_WPA_E;
        if (!APP_STRCMP(Parameter_List[3].String_Value, "TKIP")) 
          param.encryption_Type = QAPI_WLAN_CRYPT_TKIP_CRYPT_E;
        else if (!APP_STRCMP(Parameter_List[3].String_Value, "CCMP"))
           param.encryption_Type =  QAPI_WLAN_CRYPT_AES_CRYPT_E;
        else
           QCLI_Printf(qcli_wlan_group, "invaid uchipher, should be TKIP or CCMP\r\n");
    }
    else if ((wifi_mode == QAPI_WLAN_DEV_MODE_STATION_E) && APP_STRCMP(Parameter_List[2].String_Value, "wpa2cert")==0) {
        param.auth_Mode = QAPI_WLAN_AUTH_WPA2_E;
        if (!APP_STRCMP(Parameter_List[3].String_Value, "TKIP")) 
          param.encryption_Type = QAPI_WLAN_CRYPT_TKIP_CRYPT_E;
        else if (!APP_STRCMP(Parameter_List[3].String_Value, "CCMP"))
           param.encryption_Type =  QAPI_WLAN_CRYPT_AES_CRYPT_E;
        else
           QCLI_Printf(qcli_wlan_group, "invaid uchipher, should be TKIP or CCMP\r\n");
    }
    else {
        QCLI_Printf(qcli_wlan_group, "Invalid auth_Mode\r\n");
        return -1;
    }

    if (0 != qapi_WLAN_Set_Param(get_active_device(),
                                    __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                                    __QAPI_WLAN_PARAM_GROUP_WIRELESS_PREFERRED_NETWORK_PROFILE,
                                    (void *) &param,
                                    sizeof(qapi_WLAN_Preferred_Network_Profile_t),
                                    FALSE))
    {
        QCLI_Printf(qcli_wlan_group, "ERROR: Command failed\r\n");
        return -1;
    }
    return 0;
}


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : set_wep_keyix()
* Returned Value  : 0 - successful completion or
*            -1 - failed.
* Comments    : Sets WEP parameters
*
*END*-----------------------------------------------------------------*/
int32_t set_wep_keyix(int32_t key_idx)
{
    int32_t error= 0, i = 0, j = 0;
    uint32_t deviceId = 0;
    uint8_t val = 0;
    int8_t set_key[__QAPI_WLAN_MAX_WEP_KEY_SZ + 1] = {'\0'};
    qapi_WLAN_Security_Wep_Key_Pair_Params_t keyPair;

    deviceId = get_active_device();
    if(key_idx < 1 || key_idx > 4)
    {
       QCLI_Printf(qcli_wlan_group, "Invalid default key index, Please enter between 1-4\r\n");
       return -1;
    }

    if(key[key_idx-1].key_valid != 1)
    {
       QCLI_Printf(qcli_wlan_group, "This key is not present, please enter WEP key first\r\n");
       return -1;
    }

    //Pass all the keys to the driver
    for(i = 1; i <= MAX_NUM_WEP_KEYS; i++)
    {
        if(key[i-1].key_valid != 1)
        {
            continue;
        }

        /* convert key data from string to bytes */
        memset(set_key, 0, __QAPI_WLAN_MAX_WEP_KEY_SZ + 1);
        if((strlen(key[i-1].key) == MIN_HEX_WEP_KEY_SIZE) || (strlen(key[i-1].key) == MAX_HEX_WEP_KEY_SIZE))
        {
            key_type |= (1 << (i - 1));
            for (j = 0; j < strlen(key[i-1].key); j++)
            {
                val = ascii_to_hex((uint8_t)key[i-1].key[j]);
                if(0xff == val)
                {
                   QCLI_Printf(qcli_wlan_group, "for hex enter [0-9] or [A-F]\r\n\r\n");
                   return -1;
                }
                if((j & 1) == 0)
                {
                    val <<= 4;
                }
                set_key[j >> 1] |= val;
            }
			keyPair.key_Length = strlen(key[i-1].key) / 2;
        }
        else
        {
            key_type &= ~(1 << (i - 1));
            memcpy(set_key, (int8_t *)(key[i-1].key), strlen(key[i-1].key));
			keyPair.key_Length = strlen(key[i-1].key);
	    }

        keyPair.key = set_key;
        keyPair.key_Index = i;

        error = qapi_WLAN_Set_Param (deviceId,
                                     __QAPI_WLAN_PARAM_GROUP_WIRELESS_SECURITY,
                                     __QAPI_WLAN_PARAM_GROUP_SECURITY_WEP_KEY_PAIR,
                                     (void *) &keyPair, sizeof(keyPair), FALSE);
        if(error != 0)
        {
            clear_wep_keys();
            return error;
        }
    }

    error = qapi_WLAN_Set_Param (deviceId,
                                 __QAPI_WLAN_PARAM_GROUP_WIRELESS_SECURITY,
                                 __QAPI_WLAN_PARAM_GROUP_SECURITY_WEP_KEY_INDEX,
                                 (void *) &key_idx,
                                 sizeof(key_idx),
                                 FALSE);
    //Disable WSC if the APUT is manually configured for WEP.
    if(wps_flag == 1)
    {
        wps_should_disable = 1;
        wps_flag = 0;
    }
    security_mode = SEC_MODE_WEP;
    
    return error;
}

int32_t allow_aggr(char *hexTxTidMask, char *hexRxTidMask )
{
    qapi_WLAN_Aggregation_Params_t param;
    uint32_t deviceId = 0;
    deviceId = get_active_device();
    param.tx_TID_Mask = mystrtoul(hexTxTidMask, NULL, 16);
    param.rx_TID_Mask = mystrtoul(hexRxTidMask, NULL, 16);

    if(param.tx_TID_Mask > 0xff || param.rx_TID_Mask > 0xff){
       QCLI_Printf(qcli_wlan_group,"wmiconfig --allow_aggr <tx_tid_mask> <rx_tid_mask> Enables aggregation based on the provided bit mask where\r\n");
       QCLI_Printf(qcli_wlan_group,"each bit represents a TID valid TID's are 0-7\r\n");
       return -1;
    }

    if (0 != qapi_WLAN_Set_Param(deviceId,
                                 __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                                 __QAPI_WLAN_PARAM_GROUP_WIRELESS_ALLOW_TX_RX_AGGR_SET_TID,
                                 (void *) &param,
                                 sizeof(qapi_WLAN_Aggregation_Params_t),
                                 FALSE))
    {
        QCLI_Printf(qcli_wlan_group, "Failed to set Aggregation mask\r\n");
        return -1;
    }
    return 0;
}

#define AGGRX_CFG_INVAL 0xff

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : aggrx_config()
* Returned Value  : 0 - success
* Comments    : set aggregation rx parameter.
*END*-----------------------------------------------------------------*/
int32_t aggrx_config(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    qapi_WLAN_Rx_Aggrx_Params_t qapi_aggrx_param;
    uint32_t deviceId = get_active_device();
    uint8_t aggrx_buffer_size = 0, param_index = 0;
    uint8_t param_index_max = Parameter_Count - 1;
    int32_t param_integer = 0;

    memset(&qapi_aggrx_param, AGGRX_CFG_INVAL, sizeof(qapi_WLAN_Rx_Aggrx_Params_t));
    if (param_index_max > 5) {
        param_index_max = 5;
    }
    while (param_index <= param_index_max) {
        if (!Parameter_List[param_index].Integer_Is_Valid) {
            return -1;
        }
        param_integer = Parameter_List[param_index].Integer_Value;
        switch (param_index) {
            case 0:
                aggrx_buffer_size = param_integer;
                if (!(((aggrx_buffer_size>0) && (aggrx_buffer_size < __QAPI_WLAN_AGGRX_BUFFER_SIZE_MAX))
                    || (aggrx_buffer_size==AGGRX_CFG_INVAL))) {
                    QCLI_Printf(qcli_wlan_group, "Invalid buffer size: should be (0, %d)\n", __QAPI_WLAN_AGGRX_BUFFER_SIZE_MAX);
                    return -1;
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

    if (0 != qapi_WLAN_Set_Param(deviceId,
                                 __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                                 __QAPI_WLAN_PARAM_GROUP_WIRELESS_AGGRX_CONFIG,
                                 (void *) &qapi_aggrx_param,
                                 sizeof(qapi_WLAN_Rx_Aggrx_Params_t),
                                 FALSE)) {
        return -1;
    }
    return 0;
}

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : set_promiscuous_filters()
* Returned Value  : 0 - success
* Comments    : Tests promiscuous mode. Channel has to be set first 
*                 or Device has to be connected to an AP to sniff on a 
*                 particular channel.
*                 
*END*-----------------------------------------------------------------*/
static int32_t set_promiscuous_filters(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    uint8_t i = 0, arg_cnt = 0, mask = 0, num_filters = 0;
    
	num_filters = Parameter_List[arg_cnt++].Integer_Value;
    if(!num_filters || num_filters > __QAPI_WLAN_PROMISC_MAX_FILTER_IDX ){
        QCLI_Printf(qcli_wlan_group, "\nERROR: Invalid promisc_num_filters");
        return -1;
    }

    if( promisc_filter_config.num_filters + num_filters > __QAPI_WLAN_PROMISC_MAX_FILTER_IDX )
    {
        QCLI_Printf(qcli_wlan_group, "\r\nExceeding Max filters for Configuration...!!!");
        QCLI_Printf(qcli_wlan_group, "\r\nRemaining no of filters = %d", (__QAPI_WLAN_PROMISC_MAX_FILTER_IDX - promisc_filter_config.num_filters));
        return -1;
    }
    
    if( Parameter_Count< (num_filters * ATH_PROMISC_ARG_SET_LENTGH)){
        QCLI_Printf(qcli_wlan_group, "ERROR: Insufficient command arguments\n");
        return -1;
    }

    for(i = promisc_filter_config.num_filters; i < __QAPI_WLAN_PROMISC_MAX_FILTER_IDX; i++){
        if(Parameter_Count > arg_cnt){
            /* Reading fitler MASK, which enables src, dst, frametype and subtype combination */   
            promisc_filter_config.filter_flags[i] = Parameter_List[arg_cnt++].Integer_Value;
            mask = promisc_filter_config.filter_flags[i];
    
            if(mask & 0x0F){
                if(mask & __QAPI_WLAN_PROM_FILTER_SOURCE){
                    if(mystrtomac(Parameter_List[arg_cnt++].String_Value, &(promisc_filter_config.src_Mac[i][0]))) {
                        QCLI_Printf(qcli_wlan_group, "ERROR: MAC address translation failed.\r\n");
                        return -1;
                    }
                }
                else
                    arg_cnt++;
				
                if(mask & __QAPI_WLAN_PROM_FILTER_DEST){
                    if(mystrtomac(Parameter_List[arg_cnt++].String_Value, &(promisc_filter_config.dst_Mac[i][0]))) {
                        QCLI_Printf(qcli_wlan_group, "ERROR: MAC address translation failed.\r\n");
                        return -1;
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
                QCLI_Printf(qcli_wlan_group, "ERROR: Invalid filter MASK.\r\n");
                return -1;
            }
        } 
    }
    promisc_filter_config.num_filters += num_filters;

    return 0;
}

static int32_t reset_promisc_filters(void)
{
    memset(&promisc_filter_config, 0, sizeof(promisc_filter_config));
    
    return 0;
}

static int32_t enable_disable_promisc_mode(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    qapi_WLAN_Promiscuous_Mode_Info_t prom_mode_details;
    uint32_t enet_device = get_active_device();
    uint8_t i = 0;    

    if( Parameter_Count == 0 )
    {
        QCLI_Printf(qcli_wlan_group, "\n Incomplete promisc enable/disable command..!!");
        return -1;
    }
  
    /* Set to MAX PERF Mode */
    set_power_mode( QAPI_WLAN_POWER_MODE_MAX_PERF_E, QAPI_WLAN_POWER_MODULE_PWR_MAX_E);

    prom_mode_details.enable = Parameter_List[0].Integer_Value;
	if( !prom_mode_details.enable )
		reset_promisc_filters();	// Reset the filters configuration when promisc disabled
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
    
    set_callback(NULL);

    if(0 != qapi_WLAN_Set_Param (enet_device, 
                                    __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                                    __QAPI_WLAN_PARAM_GROUP_WIRELESS_ENABLE_PROMISCUOUS_MODE,
                                    (void *)&prom_mode_details,
                                    sizeof(prom_mode_details),
                                    FALSE))
    {
        return -1;
    }
    return 0;
}

int32_t promiscuous_mode_handler(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    uint8_t index=0;
	
    if( Parameter_Count < 1 ) {
        QCLI_Printf(qcli_wlan_group, "\n Incomplete command to process promisc mode..!!");
        return -1;
    }
      // Promisc mode filters config/reset
    if (APP_STRCMP(Parameter_List[index].String_Value, "filter") == 0) {
        if (APP_STRCMP(Parameter_List[index+1].String_Value, "config") == 0) {
            return set_promiscuous_filters( Parameter_Count-2, &Parameter_List[index+2]);
        }
        else if (APP_STRCMP(Parameter_List[index+1].String_Value, "reset") == 0) {
            return reset_promisc_filters();
        }
    }  // Promisc mode enable/disable
    else if (APP_STRCMP(Parameter_List[index].String_Value, "enable") == 0){
        return enable_disable_promisc_mode(Parameter_Count - 1, &Parameter_List[index+1]);
    }

    return 0;
}

int32_t pktlog_handler(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    uint32_t deviceId = get_active_device();

	if (APP_STRCMP(Parameter_List[0].String_Value, "enable") == 0)
	{
		qapi_WLAN_Pktlog_Enable_t PktlogEn;

		if( Parameter_Count != 3)
		{
			QCLI_Printf(qcli_wlan_group, "Incomplete parameters for pktlog enable command\r\n");
			return -1;    
		}
		PktlogEn.enable = Parameter_List[1].Integer_Value;
		if( PktlogEn.enable )
				PktlogEn.num_Of_Buffers = Parameter_List[2].Integer_Value;
		
		if (0 != qapi_WLAN_Set_Param(deviceId, 
						  __QAPI_WLAN_PARAM_GROUP_SYSTEM,
						  __QAPI_WLAN_PARAM_GROUP_SYSTEM_PKTLOG_ENABLE,
						  &PktlogEn,
						  sizeof(qapi_WLAN_Dbglog_Enable_t),
						  FALSE))
		{
			QCLI_Printf(qcli_wlan_group, "\n Pktlog enable failed..\n");
			return -1;
		}					 
	}			 
	else if (APP_STRCMP(Parameter_List[0].String_Value, "start") == 0)
	{
		qapi_WLAN_Pktlog_Start_Params_t PktlogStartCfg;

		if( Parameter_Count != 6 )
		{
			QCLI_Printf(qcli_wlan_group, "Incomplete parameters for pktlog start command\r\n");
			return -1;
		}
		PktlogStartCfg.event_List = Parameter_List[1].Integer_Value;
		PktlogStartCfg.log_Options= Parameter_List[2].Integer_Value;
		PktlogStartCfg.trigger_Threshold = Parameter_List[3].Integer_Value;
		PktlogStartCfg.trigger_Interval = Parameter_List[4].Integer_Value;
		PktlogStartCfg.trigger_Tail_Count = Parameter_List[5].Integer_Value;
		if (0 != qapi_WLAN_Set_Param(deviceId, 
						  __QAPI_WLAN_PARAM_GROUP_SYSTEM,
						  __QAPI_WLAN_PARAM_GROUP_SYSTEM_PKTLOG_START,
						  &PktlogStartCfg,
						  sizeof(qapi_WLAN_Pktlog_Start_Params_t),
						  FALSE))
		{
			QCLI_Printf(qcli_wlan_group, "\n Pktlog Start failed..\n");
			return -1;
		}
	}
	return QCLI_STATUS_SUCCESS_E;
}

int32_t dbglog_handler(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    uint32_t deviceId = get_active_device();

	if (APP_STRCMP(Parameter_List[0].String_Value, "enable") == 0)
	{
		qapi_WLAN_Dbglog_Enable_t DbglogEn;
		if( Parameter_Count != 2 )
		{
			QCLI_Printf(qcli_wlan_group, "Incomplete parameters for dbglog enable command\r\n");
			return -1;	  
		}

		DbglogEn.enable = Parameter_List[1].Integer_Value;
		if (0 != qapi_WLAN_Set_Param(deviceId, 
							__QAPI_WLAN_PARAM_GROUP_SYSTEM,
							__QAPI_WLAN_PARAM_GROUP_SYSTEM_DBGLOG_ENABLE,
							&DbglogEn,
							sizeof(qapi_WLAN_Dbglog_Enable_t),
							FALSE))
		{
			QCLI_Printf(qcli_wlan_group, "\n Dbglog enable failed..\n");
			return -1;
		}
	}
	else if (APP_STRCMP(Parameter_List[0].String_Value, "config") == 0)	  // Handles debug configuration
	{
		qapi_WLAN_Dbglog_Config_t DbglogCfg;
		if( Parameter_Count != 4 )
		{
			QCLI_Printf(qcli_wlan_group, "Incomplete parameters for dbglog configuration command\r\n");
			return -1;	  
		}				 
		DbglogCfg.debug_Port = Parameter_List[1].Integer_Value;
		DbglogCfg.reporting_Enable = Parameter_List[2].Integer_Value;
		DbglogCfg.report_Trigger_Size_In_Bytes = Parameter_List[3].Integer_Value;
		if (0 != qapi_WLAN_Set_Param(deviceId, 
							__QAPI_WLAN_PARAM_GROUP_SYSTEM,
							__QAPI_WLAN_PARAM_GROUP_SYSTEM_DBGLOG_CONFIG,
							&DbglogCfg,
							sizeof(qapi_WLAN_Dbglog_Config_t),
							FALSE))
		{
			QCLI_Printf(qcli_wlan_group, "\n Dbglog configuration failed..\n");
			return -1;
		}					 
	}
	else if (APP_STRCMP(Parameter_List[0].String_Value, "loglevel") == 0)
	{
		qapi_WLAN_Dbglog_Module_Config_t DbglogModuleCfg;				 
		uint8_t k, mod_id, loglevel, global_log_level_val;
		uint8_t LoglevelIndex, LoglevelBitpos;
		
		if( Parameter_Count < 3 )
		{
			QCLI_Printf(qcli_wlan_group, "Incomplete parameters for dbglog Module Configuration command\r\n");
			return -1;	  
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
				QCLI_Printf(qcli_wlan_group, "Invalid Module ID..SKipping it..!!\r\n");
				continue;	// Moving to next module
			}
			DbglogModuleCfg.module_Id_Mask |= ( (uint64_t)1 << mod_id );					
			LoglevelIndex = mod_id / __QAPI_WLAN_DBGLOG_NIBBLE_CNT_IN_WORD_MEMORY;
			LoglevelBitpos = ( mod_id*sizeof(uint32_t) ) % __QAPI_WLAN_DBGLOG_BIT_CNT_IN_WORD_MEMORY;
			DbglogModuleCfg.log_Level[LoglevelIndex] |= ( loglevel << LoglevelBitpos ); 				  
		}
		if (0 != qapi_WLAN_Set_Param(deviceId, 
							__QAPI_WLAN_PARAM_GROUP_SYSTEM,
							__QAPI_WLAN_PARAM_GROUP_SYSTEM_DBGLOG_MODULE_CONFIG,
							&DbglogModuleCfg,
							sizeof(qapi_WLAN_Dbglog_Module_Config_t),
							FALSE))
		{
			QCLI_Printf(qcli_wlan_group, "\n Dbglog configuration failed..\n");
			return -1;
		}					 
	}
	return 0;
}

int32_t regquery_handler(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
#if defined(WLAN_DEBUG)

    uint32_t deviceId = get_active_device();
	qapi_WLAN_Driver_RegQuery_Params_t Regquery;
	if (APP_STRCMP(Parameter_List[0].String_Value, "read") == 0)
	{
		if( Parameter_Count != 2 )
		{
			QCLI_Printf(qcli_wlan_group, "Incomplete parameters for Driver reg read command\r\n");
			return -1;	  
		}
		Regquery.address = mystrtoul(Parameter_List[1].String_Value, NULL, 16);
		Regquery.operation = QAPI_WLAN_REG_OP_READ;
		if (0 != qapi_WLAN_Set_Param(deviceId, 
							__QAPI_WLAN_PARAM_GROUP_SYSTEM,
							__QAPI_WLAN_PARAM_GROUP_SYSTEM_DRIVER_REG_QUERY,
							&Regquery,
							sizeof(qapi_WLAN_Driver_RegQuery_Params_t),
							FALSE))
		{
			QCLI_Printf(qcli_wlan_group, "\n Driver reg read failed..\n");
			return -1;
		}
		QCLI_Printf(qcli_wlan_group, "\n0x%08X \n",Regquery.value);
	}
	else if (APP_STRCMP(Parameter_List[0].String_Value, "dump") == 0)
	{
		if( Parameter_Count != 3 )
		{
			QCLI_Printf(qcli_wlan_group, "Incomplete parameters for Driver reg dump command\r\n");
			return -1;	  
		}
		if(Parameter_List[1].Integer_Value%4)
		{
			QCLI_Printf(qcli_wlan_group, "Error size for Driver reg dump command\r\n");
			return -1;	  
		}
		Regquery.address = mystrtoul(Parameter_List[1].String_Value, NULL, 16);
		Regquery.size = Parameter_List[2].Integer_Value;
		Regquery.operation = QAPI_WLAN_REG_OP_READ;
		uint32_t len = 0;
		
		QCLI_Printf(qcli_wlan_group, "\r\n");
		
		for(len = 0; len < Regquery.size/4; len++){

		    if (0 != qapi_WLAN_Set_Param(deviceId, 
							__QAPI_WLAN_PARAM_GROUP_SYSTEM,
							__QAPI_WLAN_PARAM_GROUP_SYSTEM_DRIVER_REG_QUERY,
							&Regquery,
							sizeof(qapi_WLAN_Driver_RegQuery_Params_t),
							FALSE))
		    {
			QCLI_Printf(qcli_wlan_group, "\n Driver reg dump failed..\n");
			return -1;
		    }
		    if(0==len%4)
		        QCLI_Printf(qcli_wlan_group, "0x%08X: ",Regquery.address);
		
		    QCLI_Printf(qcli_wlan_group, "0x%08X ",Regquery.value);
		
		    if(3==len%4)
		        QCLI_Printf(qcli_wlan_group, "\r\n");
		    Regquery.address += 4;
		}
	}
	else if (APP_STRCMP(Parameter_List[0].String_Value, "write") == 0)
	{
		if( Parameter_Count != 3 )
		{
			QCLI_Printf(qcli_wlan_group, "Incomplete parameters for Driver reg write command\r\n");
			return -1;	  
		}
		Regquery.address = mystrtoul(Parameter_List[1].String_Value, NULL, 16);
		Regquery.value = mystrtoul(Parameter_List[2].String_Value, NULL, 16);
		Regquery.operation = QAPI_WLAN_REG_OP_WRITE;
		if (0 != qapi_WLAN_Set_Param(deviceId, 
							__QAPI_WLAN_PARAM_GROUP_SYSTEM,
							__QAPI_WLAN_PARAM_GROUP_SYSTEM_DRIVER_REG_QUERY,
							&Regquery,
							sizeof(qapi_WLAN_Driver_RegQuery_Params_t),
							FALSE))
		{
			QCLI_Printf(qcli_wlan_group, "\n Driver reg write failed..\n");
			return -1;
		}
	}
       else if (APP_STRCMP(Parameter_List[0].String_Value, "rmw") == 0)
	{
		if( Parameter_Count != 4 )
		{
			QCLI_Printf(qcli_wlan_group, "Incomplete parameters for Driver reg rmw command\r\n");
			return -1;	  
		}
		Regquery.address = mystrtoul(Parameter_List[1].String_Value, NULL, 16);
		Regquery.value = mystrtoul(Parameter_List[2].String_Value, NULL, 16);
		Regquery.mask = mystrtoul(Parameter_List[3].String_Value, NULL, 16);
		Regquery.operation = QAPI_WLAN_REG_OP_RMW;
		if (0 != qapi_WLAN_Set_Param(deviceId, 
							__QAPI_WLAN_PARAM_GROUP_SYSTEM,
							__QAPI_WLAN_PARAM_GROUP_SYSTEM_DRIVER_REG_QUERY,
							&Regquery,
							sizeof(qapi_WLAN_Driver_RegQuery_Params_t),
							FALSE))
		{
			QCLI_Printf(qcli_wlan_group, "\n Driver reg read mod write failed..\n");
			return -1;
		}
	}
#endif/* WLAN_DEBUG */
	return 0;
}

int32_t memquery_handler(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List){
#if defined(WLAN_DEBUG)
    uint32_t address = 0, value = 0, size = 0, mask = 0;
  
       if (APP_STRCMP(Parameter_List[0].String_Value, "read") == 0)
	{
		if( Parameter_Count != 2 )
		{
			QCLI_Printf(qcli_wlan_group, "Incomplete parameters for Quartz mem read command\r\n");
			return -1;	  
		}
		address = mystrtoul(Parameter_List[1].String_Value, NULL, 16);
		value = *(uint32_t *)address;
		QCLI_Printf(qcli_wlan_group, "\n0x%08X \n",value);
	}
	else if (APP_STRCMP(Parameter_List[0].String_Value, "dump") == 0)
	{
		if( Parameter_Count != 3 )
		{
			QCLI_Printf(qcli_wlan_group, "Incomplete parameters for Quartz mem dump command\r\n");
			return -1;	  
		}
		if(Parameter_List[1].Integer_Value%4)
		{
			QCLI_Printf(qcli_wlan_group, "Error size for Quartz mem dump command\r\n");
			return -1;	  
		}
		address = mystrtoul(Parameter_List[1].String_Value, NULL, 16);
		size = Parameter_List[2].Integer_Value;
		uint32_t len = 0;
		
		QCLI_Printf(qcli_wlan_group, "\r\n");
		
		for(len = 0; len < size/4; len++){
		    if(0==len%4)
		        QCLI_Printf(qcli_wlan_group, "0x%08X: ",address);
			
		    value = *(uint32_t *)address;
		    QCLI_Printf(qcli_wlan_group, "0x%08X ",value);
		
		    if(3==len%4)
		        QCLI_Printf(qcli_wlan_group, "\r\n");
		    address += 4;
		}
	}
	else if (APP_STRCMP(Parameter_List[0].String_Value, "write") == 0)
	{
		if( Parameter_Count != 3 )
		{
			QCLI_Printf(qcli_wlan_group, "Incomplete parameters for Quartz mem write command\r\n");
			return -1;	  
		}
		address = mystrtoul(Parameter_List[1].String_Value, NULL, 16);
		value = mystrtoul(Parameter_List[2].String_Value, NULL, 16);
              *(uint32_t *)address = value;
	}
       else if (APP_STRCMP(Parameter_List[0].String_Value, "rmw") == 0)
	{
		if( Parameter_Count != 4 )
		{
			QCLI_Printf(qcli_wlan_group, "Incomplete parameters for Quartz mem rmw command\r\n");
			return -1;	  
		}
		uint32_t temp_value = 0;
		address = mystrtoul(Parameter_List[1].String_Value, NULL, 16);
		value = mystrtoul(Parameter_List[2].String_Value, NULL, 16);
		mask = mystrtoul(Parameter_List[3].String_Value, NULL, 16);
		temp_value = *(uint32_t *)address;
		temp_value &= ~mask;
		temp_value |= value;
		*(uint32_t *)address = temp_value;
	}
#endif/* WLAN_DEBUG */
	return 0;
}

int32_t driver_assert_handler(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List){
#if defined(WLAN_DEBUG)
	
    uint32_t deviceId = get_active_device();

	if( 0 != qapi_WLAN_Set_Param (deviceId, 
					   __QAPI_WLAN_PARAM_GROUP_SYSTEM,
		   			   __QAPI_WLAN_PARAM_GROUP_SYSTEM_DRIVER_FORCE_ASSERT,
					   NULL,
		   			   0,
		   			   FALSE))
	{
	       QCLI_Printf(qcli_wlan_group, "ERROR: force set target assert failed\n");
		   return -1;
	}
#endif/* WLAN_DEBUG */
	return 0;
}

uint32_t chan_to_frequency(uint32_t channel)
{
	if (channel < 1 || channel > 165)
	{
		return 0;
	}
	if (channel < 27) {
		channel = __QAPI_WLAN_CHAN_FREQ_1 + (channel-1)*5;
	} else {
		channel = (5000 + (channel*5));
	}
	return channel;

}

int32_t channel_switch(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List){
	
	qapi_WLAN_Channel_Switch_t channel_switch;
    uint32_t channel = 0;
    uint32_t deviceId = get_active_device();
	
	channel = chan_to_frequency(Parameter_List[0].Integer_Value);
	if(channel == 0)
		return -1; 	

	channel_switch.channel = channel;
	channel_switch.tbtt_Count = Parameter_List[1].Integer_Value;
	if( 0 != qapi_WLAN_Set_Param (deviceId, 
					   __QAPI_WLAN_PARAM_GROUP_WIRELESS,
		   			   __QAPI_WLAN_PARAM_GROUP_WIRELESS_AP_CHANNEL_SWITCH,
					   &channel_switch,
		   			   sizeof(channel_switch),
		   			   FALSE))
	{
	       QCLI_Printf(qcli_wlan_group, "ERROR: CHANNEL_SWITCH filed\n");
		   return -1;
	}
    return 0;
}

int32_t disable_channel(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List){
	
	
    uint32_t channel = 0;
    uint32_t deviceId = get_active_device();
	
	channel = chan_to_frequency(Parameter_List[0].Integer_Value);
	if(channel == 0)
		return -1; 	

	
	if( 0 != qapi_WLAN_Set_Param (deviceId, 
					   __QAPI_WLAN_PARAM_GROUP_WIRELESS,
		   			   __QAPI_WLAN_PARAM_GROUP_WIRELESS_DISABLE_CHANNEL,
					   &channel,
		   			   sizeof(channel),
		   			   FALSE))
	{
	       QCLI_Printf(qcli_wlan_group, "ERROR: CHANNEL_DISABLE filed\n");
		   return -1;
	}
    return 0;

}

int32_t arp_offload(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int32_t error= 0, j = 0;
    uint32_t device_id = 0, dataLen = 0, enable = 0,  t_ip_addr = 0;
    qapi_WLAN_ARP_Offload_Config_t arpdata;
    char *t_ip_addr_str = NULL;
    unsigned int sbits = 0;
    uint8_t val = 0;
    
    device_id = get_active_device();

	enable = Parameter_List[0].Integer_Value;
	t_ip_addr_str = Parameter_List[1].String_Value;
		
    if (enable)
    {
        arpdata.enable = enable;

        error = parse_ipv4_ad((unsigned long *)&t_ip_addr, &sbits,  t_ip_addr_str);
        if(error != 0)
            return -1;
               
        memcpy(&arpdata.target_IP, (void const*)&t_ip_addr, sizeof(t_ip_addr));
                    
        if(strlen(Parameter_List[2].String_Value) != 12)/*mac validation*/
            return -1;

        for(j = 0; j < 6 ;j++)
        {
            arpdata.target_Mac[j] = 0;
        }

        for(j = 0; j < strlen(Parameter_List[2].String_Value); j++)
        {
            val = ascii_to_hex(Parameter_List[2].String_Value[j]);
            if(val == 0xff)
            {
                 return -1;
            }
            else
            {
                if((j & 1) == 0)
                    val <<= 4;
                arpdata.target_Mac[j>>1] |= val;
            }
        }
    }
    else
        arpdata.enable = 0;

    dataLen = sizeof(arpdata);
    qapi_WLAN_Set_Param (device_id, 
                   __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                   __QAPI_WLAN_PARAM_GROUP_WIRELESS_ARP_OFFLOAD_PARAMS,
                   &arpdata, dataLen, 0);
    return error;
}

int32_t ns_offload(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int32_t error= 0, j = 0, retval = -1;
    uint32_t device_id = 0, dataLen = 0, enable = 0;
    qapi_WLAN_NS_Offload_Config_t ns_data;
    unsigned char g_ip_addr[__QAPI_WLAN_IPV6_ADDR_LEN] = {'\0'}; 
    unsigned char l_ip_addr[__QAPI_WLAN_IPV6_ADDR_LEN] = {'\0'};
    unsigned char s_ip_addr[__QAPI_WLAN_IPV6_ADDR_LEN] = {'\0'};
    char *g_ip_addr_str = NULL, *l_ip_addr_str = NULL, *s_ip_addr_str = NULL;
    uint8_t val = 0;
    
    device_id = get_active_device();

	enable = Parameter_List[0].Integer_Value;
	g_ip_addr_str = (char *) Parameter_List[1].String_Value;
	l_ip_addr_str = (char *) Parameter_List[2].String_Value;
    memset(&ns_data, 0, sizeof(ns_data));
    if (enable) 
    {
        ns_data.enable = 1;
        if (Parameter_Count == 7)
        {
        	s_ip_addr_str = (char *) Parameter_List[4].String_Value;
            retval = inet_pton(AF_INET6, (const char *) s_ip_addr_str, s_ip_addr);
            if(retval != 0)
                return -1;
            memcpy(&ns_data.solicitation_IP, (void const*)&s_ip_addr, sizeof(s_ip_addr));    
        }

		if(check_empty_ip_addr_string(g_ip_addr_str) == 0)
	       	retval = inet_pton(AF_INET6, (const char *) l_ip_addr_str, g_ip_addr);
		else
			retval = inet_pton(AF_INET6, (const char *) g_ip_addr_str, g_ip_addr);
		
   	    if(retval != 0)
            return -1;
               
       	memcpy(&ns_data.target_IP[0], (void const*)&g_ip_addr, sizeof(g_ip_addr));
		
	    retval = inet_pton(AF_INET6, (const char *) l_ip_addr_str, l_ip_addr);

        if(retval != 0)
            return -1;
      
        memcpy(&ns_data.target_IP[1], (void const*)&l_ip_addr, sizeof(l_ip_addr));
                    
        if(strlen(Parameter_List[3].String_Value) != 12)/*mac validation*/
            return -1;

        for(j = 0; j < 6; j++)
        {
            ns_data.target_Mac[j] = 0;
        }

        for(j = 0; j < strlen(Parameter_List[3].String_Value); j++)
        {
            val = ascii_to_hex(Parameter_List[3].String_Value[j]);
            if(val == 0xff)
            {
                return -1;
            }
            else
            {
                if((j & 1) == 0)
                    val <<= 4;
                ns_data.target_Mac[j >> 1] |= val;
            }
        }
    }
    else
        ns_data.enable = 0;

    dataLen = sizeof(ns_data);
    qapi_WLAN_Set_Param (device_id, 
                     __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                     __QAPI_WLAN_PARAM_GROUP_WIRELESS_NS_OFFLOAD_PARAMS,
                     &ns_data,dataLen,0);
    return error;
}

/* test_raw -- makes use of special API's to send a raw MAC frame irregardless of connection state
 */
int32_t send_raw_frame(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
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
        if(mystrtomac((const char *)Parameter_List[5+i].String_Value, &(addr[i][0])))
        {
            QCLI_Printf(qcli_wlan_group, "ERROR: MAC address translation failed.\r\n");
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

    status = qapi_WLAN_Raw_Send(deviceId, &rawSendParams);
    if( status != 0)
    {
raw_usage:
       QCLI_Printf(qcli_wlan_group, "raw input error\r\n");
       QCLI_Printf(qcli_wlan_group, "usage = WLAN SendRawFrame rate num_tries num_bytes channel header_type [addr1 [addr2 [addr3 [addr4]]]]\r\n");
       QCLI_Printf(qcli_wlan_group, "rate = rate index where 0==1mbps; 1==2mbps; 2==5.5mbps etc\r\n");
       QCLI_Printf(qcli_wlan_group, "num_tries = number of transmits 1 - 14\r\n");
       QCLI_Printf(qcli_wlan_group, "num_bytes = payload size 0 to 1400\r\n");
       QCLI_Printf(qcli_wlan_group, "channel = 0 - 11, 0: send on current channel\r\n");
       QCLI_Printf(qcli_wlan_group, "header_type = 0==beacon frame; 1==QOS data frame; 2==4 address data frame\r\n");
       QCLI_Printf(qcli_wlan_group, "addr1 = mac address xx:xx:xx:xx:xx:xx\r\n");
       QCLI_Printf(qcli_wlan_group, "addr2 = mac address xx:xx:xx:xx:xx:xx\r\n");
       QCLI_Printf(qcli_wlan_group, "addr3 = mac address xx:xx:xx:xx:xx:xx\r\n");
       QCLI_Printf(qcli_wlan_group, "addr4 = mac address xx:xx:xx:xx:xx:xx\r\n");        
    }
    
    return status;
}

int32_t set_event_filter(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    uint32_t device_id = get_active_device();
    qapi_WLAN_Event_Filter_t evt_filter;

    if (Parameter_Count <= 0)
       return -1;
    
    if (Parameter_Count == 1 && Parameter_List[0].Integer_Value != 0) {
	   QCLI_Printf(qcli_wlan_group, "Invalid options\n");
       return -1;
	}
    
    if (Parameter_List[0].Integer_Value == 1 && Parameter_Count > (__QAPI_WLAN_MAX_NUM_FILTERED_EVENTS+1))
    {
	   QCLI_Printf(qcli_wlan_group, "Exceeded max number of supported event filters (%d)\n", __QAPI_WLAN_MAX_NUM_FILTERED_EVENTS);
       return -1;   
    }
    
    if (Parameter_Count == 1 && (Parameter_List[0].Integer_Value == 0))
    {
        evt_filter.action     = QAPI_WLAN_DISABLE_E; //disable event filtering
        evt_filter.num_Events = 0;
    }
    
    if (Parameter_List[0].Integer_Value == 1)
    {
        int num_events_in_arg_list = Parameter_Count-1;
        int i = 0, arge = 1;

        if (num_events_in_arg_list == 0)
           return -1;

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
				  QCLI_Printf(qcli_wlan_group, "Event filter (%d) only applicable for device-0\n", Parameter_List[arge].Integer_Value);
				  return -1;
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
               QCLI_Printf(qcli_wlan_group, "Invalid eventId: %d. This is not a subscribable event\n", Parameter_List[arge].Integer_Value);
               return -1;
          }        
        }
    }
    
    if(qapi_WLAN_Set_Param(device_id, 
                         __QAPI_WLAN_PARAM_GROUP_WIRELESS, 
                         __QAPI_WLAN_PARAM_GROUP_WIRELESS_EVENT_FILTER, 
                         &evt_filter,
                         sizeof(evt_filter), 
                         FALSE) != 0)
        {
            QCLI_Printf(qcli_wlan_group, "Could not set WLAN event filter list\r\n");
            return -1;
        }
    return 0;
   
}


/*FUNCTION*--------------------------------------------------------------------
*
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
*
*END*------------------------------------------------------------------------*/
int32_t scan_control( int32_t argc, char* argv[])
{
    int32_t error = 0, fg = 0, bg = 0;
    qapi_WLAN_Scan_Params_t scanParam;
    uint32_t deviceId = 0;

    do{
        if(argc != 2) break;
        fg = atoi(argv[0]);
        if(fg != 0 && fg != 1) break;
        bg = atoi(argv[1]);
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
                scanParam.short_Scan_Ratio = SHORTSCANRATIO_DEFAULT;
                scanParam.scan_Ctrl_Flags = DEFAULT_SCAN_CTRL_FLAGS;
                scanParam.min_Act_Chan_Dwell_Time_In_Ms = 0;
                scanParam.max_Act_Scan_Per_Ssid = 0;
                scanParam.max_Dfs_Chan_Act_Time_In_Ms = 0;
                
                deviceId = get_active_device();
                error = qapi_WLAN_Set_Param (deviceId,
                             __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                             __QAPI_WLAN_PARAM_GROUP_WIRELESS_SCAN_PARAMS,
                             (void *) &scanParam,
                             sizeof(scanParam),
                             FALSE);
        if(error == -1){
            QCLI_Printf(qcli_wlan_group, "driver ioctl error\r\n");
            return -1;
        }
            return 0;
    }while(0);

    QCLI_Printf(qcli_wlan_group, "param error: scan control requires 2 inputs [0|1] [0|1]\r\n");
    return -1;
}

/*FUNCTION*--------------------------------------------------------------------
*
* Function Name   : set_scan_para()
* Returned Value  : 0 - on successful completion
*                    -1 - on any failure.
* Comments        : set scan parameters
*
*END*------------------------------------------------------------------------*/
int32_t set_scan_parameters(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int32_t error = 0;
    qapi_WLAN_Scan_Params_t scan_params;
    uint32_t deviceId = 0;
    if(Parameter_Count != 10 || !Parameter_List) 
        return -1;
              
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
                
    deviceId = get_active_device();
    error = qapi_WLAN_Set_Param (deviceId,
                             __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                             __QAPI_WLAN_PARAM_GROUP_WIRELESS_SCAN_PARAMS,
                             &scan_params,
                             sizeof(scan_params),
                             FALSE);
     if(error == -1){
         QCLI_Printf(qcli_wlan_group, "Set scan params failed\r\n");
         return -1;
     }
     return 0;
}

/*FUNCTION*--------------------------------------------------------------------
*
* Function Name   : start_scan()
* Returned Value  : 0 - on successful completion
*                    -1 - on any failure.
* Comments        : set scan --- do real scan
*
*END*------------------------------------------------------------------------*/
int32_t start_scan(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int32_t error = 0;
    qapi_WLAN_Start_Scan_Params_t *scan_params = NULL;
    uint16_t maxArgCount = 0, argI = 0;
    uint32_t deviceId = 0, wifimode = 0, dataLen = 0, size = 0;
    uint8_t numChannels = 0, orig_ssid[__QAPI_WLAN_MAX_SSID_LENGTH] = {'\0'};

    if (Parameter_Count < 4 || !Parameter_List)
        return -1;
#if ENABLE_P2P_MODE
    if(p2p_session_in_progress)
    {
        QCLI_Printf(qcli_wlan_group, "p2p event in progress \r\n");
        return -1;
    }
#endif  

    memset(orig_ssid,0,__QAPI_WLAN_MAX_SSID_LENGTH);
    deviceId = get_active_device();
    qapi_WLAN_Get_Param (deviceId,
                         __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                         __QAPI_WLAN_PARAM_GROUP_WIRELESS_SSID,
                         (uint8_t *) orig_ssid,
                         &dataLen);
    qapi_WLAN_Get_Param (deviceId, 
         __QAPI_WLAN_PARAM_GROUP_WIRELESS,
         __QAPI_WLAN_PARAM_GROUP_WIRELESS_OPERATION_MODE,
         &wifimode,
         &dataLen);

    if(wifimode == QAPI_WLAN_DEV_MODE_AP_E)
    {
        QCLI_Printf(qcli_wlan_group, "Cannot scan in AP mode\r\n");
        return -1;
    }    
    
    do
    {
        numChannels = (uint8_t) Parameter_List[3].Integer_Value;
        if (numChannels > __QAPI_WLAN_START_SCAN_PARAMS_CHANNEL_LIST_MAX)
        {
            QCLI_Printf(qcli_wlan_group, "Cannot set more than 12 channels to scan\r\n");
            return -1;
        }

        size = sizeof(qapi_WLAN_Start_Scan_Params_t);
        if(numChannels > 0) {
            size += (((uint8_t)Parameter_List[3].Integer_Value - 1) * sizeof(uint16_t));              
        }

        if(NULL == (scan_params = malloc(size))){
            QCLI_Printf(qcli_wlan_group, "Setting of scan parameters failed due to insufficient memory.\r\n");
            return -1;
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
           QCLI_Printf(qcli_wlan_group, "Invalid parameter list\r\n");
           error = -1;
           break;
        }

        while (argI < maxArgCount)
        {
           scan_params->channel_List[argI-4] = (uint16_t) Parameter_List[argI].Integer_Value;
           argI++;
        }
        error = qapi_WLAN_Set_Param (deviceId,
                             __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                             __QAPI_WLAN_PARAM_GROUP_WIRELESS_SSID,
                             (void *) ssid,
                             strlen(ssid),
                             FALSE);
        if(error != 0)
        {
           QCLI_Printf(qcli_wlan_group, "Unable to set SSID\r\n");
           error = -1;
           break;
        }
            
        /*Do the actual scan*/
        error = wmi_set_scan(deviceId, scan_params, QAPI_WLAN_BUFFER_SCAN_RESULTS_BLOCKING_E);
        if(error != 0)
        {
           QCLI_Printf(qcli_wlan_group, "Custom scan failed\r\n");
           error = -1;
           /* No break here, fall through to revert to original SSID */
        }
           
        /*Revert to original SSID*/
        if ('\0' !=  *orig_ssid) {
            error = qapi_WLAN_Set_Param (deviceId,
                                        __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                                        __QAPI_WLAN_PARAM_GROUP_WIRELESS_SSID,
                                        (void *) orig_ssid,
                                        strlen((char*) orig_ssid),
                                        FALSE);
            if(error != 0)
            {
                QCLI_Printf(qcli_wlan_group, "Unable to set SSID\r\n");
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


/*FUNCTION*--------------------------------------------------------------------
*
* Function Name   : get_rate()
* Returned Value  : 0 - on successful completion
*                    -1 - on any failure.
* Comments        : gets TX rate from chip
*
*END*------------------------------------------------------------------------*/
int32_t get_rate()
{
    int32_t rateIndex = 0;
    uint32_t dataLen = 0;

    if ( 0 != qapi_WLAN_Get_Param(get_active_device(),
                               __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                               __QAPI_WLAN_PARAM_GROUP_WIRELESS_RATE,
                               &rateIndex,
                               &dataLen) )
    {
       QCLI_Printf(qcli_wlan_group, "Command Failed\r\n");
       return -1;
    }
    QCLI_Printf(qcli_wlan_group, "rateIdx = %d\n", rateIndex);
    return 0;
}


/*FUNCTION*--------------------------------------------------------------------
*
* Function Name   : set_rate()
* Returned Value  : 0 - on successful completion
*                    -1 - on any failure.
* Comments        : sets TX data rate 
*
*END*------------------------------------------------------------------------*/
int32_t set_rate(int32_t isMcs, int32_t rateIdx)
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
          QCLI_Printf(qcli_wlan_group, "Unsupported rate\r\n");
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
           QCLI_Printf(qcli_wlan_group, "undefined mcs rate \r\n");
           return error;
       }      
    }
    if(0 != qapi_WLAN_Set_Param(get_active_device(), 
                                   __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                                   __QAPI_WLAN_PARAM_GROUP_WIRELESS_RATE,
                                   &rateIndex,
                                   sizeof(rateIndex),
                                   FALSE))
    {
        QCLI_Printf(qcli_wlan_group, "Command Failed\r\n");
        return error;
    }
    return 0;
}


int32_t set_channel_hint(int32_t channelNum)
{
    uint32_t deviceId = get_active_device();
    if (0 == qapi_WLAN_Set_Param(deviceId,
                                       __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                                       __QAPI_WLAN_PARAM_GROUP_WIRELESS_CHANNEL,
                                       (void *) &channelNum,
                                       sizeof(channelNum),
                                       FALSE))
    {
        return 0;
    }
    return -1;
}

int32_t set_tx_power(int32_t power_in_dBm)
{
    uint32_t deviceId = get_active_device();
      
    if (0 == qapi_WLAN_Set_Param(deviceId, 
                                        __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                                        __QAPI_WLAN_PARAM_GROUP_WIRELESS_TX_POWER_IN_DBM,
                                        &power_in_dBm,
                                        sizeof(power_in_dBm),
                                        FALSE))
    {
       return 0;
    }
    return -1;
}

int32_t set_phy_mode(char *wmode)
{
    qapi_WLAN_Phy_Mode_e phyMode;
    if (0 == APP_STRCMP(wmode, "a")) {
       phyMode = QAPI_WLAN_11A_MODE_E;
    } else if (0 == APP_STRCMP(wmode, "b")) {
       phyMode = QAPI_WLAN_11B_MODE_E;
    } else if (0 == APP_STRCMP(wmode, "g")) {
       phyMode = QAPI_WLAN_11G_MODE_E;
    } else if (0 == APP_STRCMP(wmode, "ag")) {
       phyMode = QAPI_WLAN_11AG_MODE_E;
    } else if (0 == APP_STRCMP(wmode, "gonly")) {
       phyMode = QAPI_WLAN_11GONLY_MODE_E;
    } else {
       QCLI_Printf(qcli_wlan_group, "Unknown wmode, only support a/b/g/ag/gonly/\r\n");
       return -1;
    }
    if (0 == qapi_WLAN_Set_Param (get_active_device(), 
               __QAPI_WLAN_PARAM_GROUP_WIRELESS,
               __QAPI_WLAN_PARAM_GROUP_WIRELESS_PHY_MODE,
               &phyMode,
               sizeof(phyMode),
               FALSE))
    {
        return 0;
    }
    return -1;
}

int32_t set_11n_ht(char *ht_config)
{
    qapi_WLAN_11n_HT_Config_e htconfig;
    if (0 == APP_STRCMP(ht_config, "disable")) {
        htconfig = QAPI_WLAN_11N_DISABLED_E;
    } else if (0 == APP_STRCMP(ht_config, "ht20")) {
        htconfig = QAPI_WLAN_11N_HT20_E;
    } else {
        QCLI_Printf(qcli_wlan_group, "Unknown ht config, only support disable/ht20\r\n");
        return -1;
    }
    if (0 == qapi_WLAN_Set_Param (get_active_device(), 
               __QAPI_WLAN_PARAM_GROUP_WIRELESS,
               __QAPI_WLAN_PARAM_GROUP_WIRELESS_11N_HT,
               &htconfig,
               sizeof(htconfig),
               FALSE))
    {
        return 0;
    }
    return -1;
}

int32_t set_op_mode(char *omode, char *hiddenSsid, char *wpsEnabled)
{
            char* ssid = "";       
            int32_t deviceId = get_active_device();
            qapi_WLAN_Set_Param (deviceId,
                             __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                             __QAPI_WLAN_PARAM_GROUP_WIRELESS_SSID,
                             (void *) ssid,
                             strlen((char *) ssid),
                             FALSE);
            
            if(APP_STRCMP(omode, "ap") == 0)
            {
                qapi_WLAN_Dev_Mode_e opMode;
                if (APP_STRCMP(hiddenSsid, "hidden") == 0)
                {
                    hidden_flag = 1;
                }
                else if(APP_STRCMP(hiddenSsid, "0") == 0 || APP_STRCMP(hiddenSsid, "") == 0)
                {
                    hidden_flag = 0;
                }
                else
                {
                    return QCLI_STATUS_USAGE_E;
                }
                if (APP_STRCMP(wpsEnabled, "wps") == 0)
                {
                    if(hidden_flag == 1)
                    {
                        wps_flag = 0;
                        QCLI_Printf(qcli_wlan_group, "WPS could not be enabled in AP hidden mode.\r\n");
                    }
                    else
                    {
                        wps_flag = 1;
                    }
                }
                else if(APP_STRCMP(wpsEnabled, "0") == 0 || APP_STRCMP(wpsEnabled, "") == 0)
                {
                    wps_flag = 0;
                }
                else
                {
                    return QCLI_STATUS_USAGE_E;
                }
                opMode = QAPI_WLAN_DEV_MODE_AP_E;
                    
                if (0 != qapi_WLAN_Set_Param(deviceId, 
                                    __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                                    __QAPI_WLAN_PARAM_GROUP_WIRELESS_OPERATION_MODE,
                                    &opMode,
                                    sizeof(qapi_WLAN_Dev_Mode_e),
                                    FALSE))
                {
                    QCLI_Printf(qcli_wlan_group, "Not able to set op mode to AP \r\n");
                    return QCLI_STATUS_ERROR_E;
                }
                
                /* Set to MAX PERF Mode */
                set_power_mode( QAPI_WLAN_POWER_MODE_MAX_PERF_E, QAPI_WLAN_POWER_MODULE_SOFTAP_E);

                if ( 0 != qapi_WLAN_Set_Param(deviceId, 
                                                __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                                                __QAPI_WLAN_PARAM_GROUP_WIRELESS_AP_ENABLE_HIDDEN_MODE,
                                                &hidden_flag,
                                                sizeof(hidden_flag),
                                                FALSE))
                {
                    QCLI_Printf(qcli_wlan_group, "Not able to set hidden mode for AP \r\n");
                    return QCLI_STATUS_ERROR_E;
                }

               if (0 != qapi_WLAN_Set_Param(deviceId, 
                                                __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                                                __QAPI_WLAN_PARAM_GROUP_WIRELESS_AP_WPS_FLAG,
                                                &wps_flag,
                                                sizeof(wps_flag),
                                                FALSE))
                {
                    QCLI_Printf(qcli_wlan_group, "Not able to set wps mode for AP \r\n");
                    return QCLI_STATUS_ERROR_E;
                }
                return QCLI_STATUS_SUCCESS_E;
            }
            else if(APP_STRCMP(omode, "station") == 0)
            {
                qapi_WLAN_Dev_Mode_e opMode, wifimode;
                uint32_t dataLen;
                qapi_WLAN_Get_Param (deviceId, 
                                     __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                                     __QAPI_WLAN_PARAM_GROUP_WIRELESS_OPERATION_MODE,
                                     &wifimode,
                                     &dataLen);
                if (wifimode == QAPI_WLAN_DEV_MODE_AP_E) /*check previous mode*/
                {
                   qapi_WLAN_Disconnect(deviceId);
                    /* AP Mode is always set to maxper; if we are switching mode and prev mode is QAPI_WLAN_POWER_MODE_REC_POWER_E then
                       retain the power mode for STA */
                    
                   set_power_mode( QAPI_WLAN_POWER_MODE_REC_POWER_E, QAPI_WLAN_POWER_MODULE_SOFTAP_E);
                }
                opMode = QAPI_WLAN_DEV_MODE_STATION_E;
                if (0 == qapi_WLAN_Set_Param(deviceId, 
                                    __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                                    __QAPI_WLAN_PARAM_GROUP_WIRELESS_OPERATION_MODE,
                                    &opMode,
                                    sizeof(qapi_WLAN_Dev_Mode_e),
                                    FALSE))
                    return QCLI_STATUS_SUCCESS_E;
                else
                    return QCLI_STATUS_ERROR_E;
            }
            return QCLI_STATUS_USAGE_E;

}
//called from the command line
int32_t set_pwr_mode(int32_t power_mode)
{
    uint32_t dataLen = 0, wifimode = 0;
    uint32_t deviceId = get_active_device();
    if(power_mode == 0){
       user_defined_power_mode = QAPI_WLAN_POWER_MODE_MAX_PERF_E;
       return set_power_mode( QAPI_WLAN_POWER_MODE_MAX_PERF_E, QAPI_WLAN_POWER_MODULE_USER_E);             
    }else{
       qapi_WLAN_Get_Param (deviceId, 
                            __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                            __QAPI_WLAN_PARAM_GROUP_WIRELESS_OPERATION_MODE,
                            &wifimode,
                            &dataLen);
       if(wifimode == QAPI_WLAN_DEV_MODE_AP_E && !(p2pMode)) {
           QCLI_Printf(qcli_wlan_group, "Setting REC Power is not allowed MODE_AP \r\n");
           return -1;
       }
       user_defined_power_mode = QAPI_WLAN_POWER_MODE_REC_POWER_E;
       return set_power_mode( QAPI_WLAN_POWER_MODE_REC_POWER_E, QAPI_WLAN_POWER_MODULE_USER_E);             
   }
}

int32_t set_power_mgmt_policy_params(int32_t idle_period_ms, int32_t num_ps_poll, 
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

    return qapi_WLAN_Set_Param(get_active_device(),
                                __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                                __QAPI_WLAN_PARAM_GROUP_WIRELESS_POWER_MODE_POLICY,
                                &pm,
                                sizeof(qapi_WLAN_Power_Policy_Params_t),
                                FALSE);
}


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : get_version()
* Returned Value  : 0 version is retrieved successfully else ERROR CODE
* Comments        : gets driver,firmware version.
*
*END*-----------------------------------------------------------------*/
int32_t get_version()
{
    qapi_WLAN_Firmware_Version_String_t versionstr;
    uint32_t dataLen = 0;
    
    if (0 != qapi_WLAN_Get_Param(get_active_device(),
                                    __QAPI_WLAN_PARAM_GROUP_SYSTEM,
                                    __QAPI_WLAN_PARAM_GROUP_SYSTEM_FIRMWARE_VERSION,
                                    &versionstr,
                                    &dataLen))
    {
        QCLI_Printf(qcli_wlan_group, "Command failed\n");
        return -1;
    }
    else
    {
    //QCLI_Printf(qcli_wlan_group, "Host version :  %s\r\n", versionstr.host_Version);
    //QCLI_Printf(qcli_wlan_group, "Target version   :  %s\r\n", versionstr.target_Version);
    QCLI_Printf(qcli_wlan_group, "Firmware version :  %s\r\n", versionstr.wlan_Version);
    QCLI_Printf(qcli_wlan_group, "Interface version:  %s\r\n", versionstr.abi_Version);
    } 
    return 0;
}

/*FUNCTION*--------------------------------------------------------------------
*
* Function Name   : get_reg_domain()
* Returned Value  : 0 - on successful completion
*            -1 - on any failure.
* Comments        : gets Regulatory domain from driver
*
*END*------------------------------------------------------------------------*/
int32_t get_reg_domain()
{
    uint32_t regDomain = 0, dataLen = 0;
    
    if (0 != qapi_WLAN_Get_Param(get_active_device(),
                                    __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                                    __QAPI_WLAN_PARAM_GROUP_WIRELESS_REG_DOMAIN,
                                    &regDomain,
                                    &dataLen))
    {
        QCLI_Printf(qcli_wlan_group, "Command failed\n");
        return -1;
    }
    else
    {
        QCLI_Printf(qcli_wlan_group, "Regulatory Domain 0x%x\r\n",regDomain);    
    } 
    return 0;
}


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : get_last_error()
* Returned Value  : 0 if phy_mode set successfully else ERROR CODE
* Comments        : Gets the last error in the host driver
*
*END*-----------------------------------------------------------------*/

int32_t get_last_error()
{

    int32_t err = 0;
    uint32_t dataLen = 0;

    if (0 != qapi_WLAN_Get_Param(get_active_device(),
                                    __QAPI_WLAN_PARAM_GROUP_SYSTEM,
                                    __QAPI_WLAN_PARAM_GROUP_SYSTEM_LAST_ERROR,
                                    &err,
                                    &dataLen))
    {
        QCLI_Printf(qcli_wlan_group, "Command failed\n");
        return -1;
    }
    else
    {
        QCLI_Printf(qcli_wlan_group, "Last driver error: 0x%x\r\n", err);    
    } 
    return 0;
}


int32_t enable_suspend ()
{
   int32_t suspend = 1;
   return qapi_WLAN_Set_Param (get_active_device(),
               __QAPI_WLAN_PARAM_GROUP_SYSTEM,
               __QAPI_WLAN_PARAM_GROUP_SYSTEM_ENABLE_SUSPEND_RESUME,
               &suspend,
               sizeof(suspend),
               FALSE);
}

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : dev_susp_start()
* Returned Value  : 0 if device suspend is started
*                   successfully else ERROR CODE
* Comments        : Suspends device for requested time period
*
*END*-----------------------------------------------------------------*/
int32_t dev_susp_start(int32_t susp_time)
{
    uint32_t deviceId = 0, wifimode = 0, txStatus = 0, dataLen = 0;

    deviceId = get_active_device();
    qapi_WLAN_Get_Param (deviceId, 
                        __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                        __QAPI_WLAN_PARAM_GROUP_WIRELESS_OPERATION_MODE,
                        &wifimode,
                        &dataLen);
    if (wifimode == QAPI_WLAN_DEV_MODE_AP_E)    
    {
        QCLI_Printf(qcli_wlan_group, "Store-Recall Mechanism NOT supported in 'MODE_AP' \r\n");
        return -1;
    }

    /*Check if no packets are queued, if TX is pending, then wait*/
    do{
       if (0 == qapi_WLAN_Get_Param (deviceId, 
                                __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                                __QAPI_WLAN_PARAM_GROUP_WIRELESS_TX_STATUS,
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
    return qapi_WLAN_Suspend_Start(susp_time);
}



/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : wmi_iwconfig()
* Returned Value  : 0 if success else -1
*
* Comments        : Setup for scan command
*
*END*-----------------------------------------------------------------*/
int32_t wlan_scan(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    uint8_t scan_ssid_flag = 0;            //Flag to decide whether to scan all ssid/channels
    uint8_t scan_ssid[__QAPI_WLAN_MAX_SSID_LENGTH], orig_ssid[__QAPI_WLAN_MAX_SSID_LENGTH];
    int32_t error = 0;
    uint32_t deviceId = 0, wifimode = 0, dataLen = 0;
    qapi_WLAN_Store_Scan_Results_e scan_mode = 0;  //Default mode is BLOCKING and BUFFERING
    
#if ENABLE_P2P_MODE
    if(p2p_session_in_progress)
    {
        QCLI_Printf(qcli_wlan_group, "p2p event in progress \r\n");
        return -1;
    }
#endif    
    deviceId = get_active_device();

    memset(scan_ssid,0,__QAPI_WLAN_MAX_SSID_LENGTH);
    memset(orig_ssid,0,__QAPI_WLAN_MAX_SSID_LENGTH);

    qapi_WLAN_Get_Param (deviceId,
                         __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                         __QAPI_WLAN_PARAM_GROUP_WIRELESS_SSID,
                         (uint8_t *) orig_ssid,
                         &dataLen);
    
    qapi_WLAN_Get_Param (deviceId, 
                         __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                         __QAPI_WLAN_PARAM_GROUP_WIRELESS_OPERATION_MODE,
                         &wifimode,
                         &dataLen);

    if (wifimode == QAPI_WLAN_DEV_MODE_AP_E)
    {
        QCLI_Printf(qcli_wlan_group, "AP mode cannot scan\r\n");
        return -1;
    }
    
    if(Parameter_Count >= 1 && Parameter_List[0].Integer_Is_Valid)
    {
          
        if((Parameter_List[0].Integer_Value < QAPI_WLAN_BUFFER_SCAN_RESULTS_BLOCKING_E) || (Parameter_List[0].Integer_Value > QAPI_WLAN_NO_BUFFERING_E))
        {
           QCLI_Printf(qcli_wlan_group,qcli_wlan_group, "Invalid scan mode - Using default\r\n");
           scan_mode = 0;
        }
        scan_mode = Parameter_List[0].Integer_Value;
        
        if(Parameter_Count == 2 && !Parameter_List[1].Integer_Is_Valid)
        {
            if(strlen((char *) Parameter_List[1].String_Value) > __QAPI_WLAN_MAX_SSID_LENGTH)
            {
                QCLI_Printf(qcli_wlan_group,qcli_wlan_group, "SSID length exceeds Maximum value\r\n");
                return -1;
            }
            /*Scan specified SSID*/
            scan_ssid_flag = 1;
            strcpy((char*)scan_ssid, (char *) Parameter_List[1].String_Value);
        }
    }

    /*Set SSID for scan*/
    if(scan_ssid_flag)
    {
        error = qapi_WLAN_Set_Param (deviceId,
                                     __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                                     __QAPI_WLAN_PARAM_GROUP_WIRELESS_SSID,
                                     (void *) scan_ssid,
                                     strlen((char*) scan_ssid),
                                     FALSE);
    if(error != 0)
    {
        QCLI_Printf(qcli_wlan_group,qcli_wlan_group, "Unable to set SSID\r\n");
        return error;
    }
    }
    else
    {
        char *ssid = "";
        error = qapi_WLAN_Set_Param (deviceId,
                                     __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                                     __QAPI_WLAN_PARAM_GROUP_WIRELESS_SSID,
                                     (void *) ssid,
                                     strlen(ssid),
                                     FALSE);
    if(error != 0)
    {
        QCLI_Printf(qcli_wlan_group,qcli_wlan_group, "Unable to set SSID\r\n");
        return error;
    }
    }

    /*Do the actual scan*/
    wmi_set_scan(deviceId, NULL, scan_mode);

    /*Revert to original SSID*/
    error = qapi_WLAN_Set_Param (deviceId,
                                 __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                                 __QAPI_WLAN_PARAM_GROUP_WIRELESS_SSID,
                                 (void *) orig_ssid,
                                 strlen((char*) orig_ssid),
                                 FALSE);
    if(error != 0)
    {
    QCLI_Printf(qcli_wlan_group,qcli_wlan_group, "Unable to set SSID\r\n");
    return error;
    }
    return error;
}

#if ENABLE_AP_MODE

int32_t set_ap_inactivity_period(uint32_t inactivity_time_in_mins)
{
    uint32_t deviceId = 0, wifimode = 0, dataLen = 0;
    deviceId = get_active_device();

    if (0 != qapi_WLAN_Get_Param (deviceId, 
                         __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                         __QAPI_WLAN_PARAM_GROUP_WIRELESS_OPERATION_MODE,
                         &wifimode,
                         &dataLen))
    {
        return -1;
    }
    if (QAPI_WLAN_DEV_MODE_AP_E != wifimode)
    {
        //QCLI_Printf(qcli_wlan_group, "Err:Set AP Mode to apply AP settings\r\n");
        return -1;
    }
    if (0 != qapi_WLAN_Set_Param (deviceId,
                                  __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                                  __QAPI_WLAN_PARAM_GROUP_WIRELESS_AP_INACTIVITY_TIME_IN_MINS,
                                  &inactivity_time_in_mins,
                                  sizeof(inactivity_time_in_mins),  
                                  FALSE))
    {
        //QCLI_Printf(qcli_wlan_group, "Err:set inactive_time\r\n");
          return -1;
    }
    return 0;
}

int32_t set_ap_beacon_interval(uint32_t beacon_int_in_tu)
{
    uint32_t deviceId = 0, wifimode = 0, dataLen = 0;
    deviceId = get_active_device();

    if (0 != qapi_WLAN_Get_Param (deviceId, 
                         __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                         __QAPI_WLAN_PARAM_GROUP_WIRELESS_OPERATION_MODE,
                         &wifimode,
                         &dataLen))
    {
        return -1;
    }
    if (QAPI_WLAN_DEV_MODE_AP_E != wifimode)
    {
        //QCLI_Printf(qcli_wlan_group, "Err:Set AP Mode to apply AP settings\r\n");
        return -1;
    }
    if((beacon_int_in_tu < 100) || (beacon_int_in_tu > 1000))
    {
        //QCLI_Printf(qcli_wlan_group, "beacon interval has to be within 100-1000 in units of 100 \r\n");
         return -1;
    }
    if (0 != qapi_WLAN_Set_Param (deviceId,
                                  __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                                  __QAPI_WLAN_PARAM_GROUP_WIRELESS_AP_BEACON_INTERVAL_IN_TU,
                                  &beacon_int_in_tu,
                                  sizeof(beacon_int_in_tu),  
                                  FALSE))
    {
        //QCLI_Printf(qcli_wlan_group, "Err:set beacon_int_in_ms\r\n");
          return -1;
    }
    return 0;
}

int32_t set_ap_dtim_period(uint32_t dtim_period)
{
    uint32_t deviceId = 0, wifimode = 0, dataLen = 0;
    deviceId = get_active_device();

    if (0 != qapi_WLAN_Get_Param (deviceId, 
                         __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                         __QAPI_WLAN_PARAM_GROUP_WIRELESS_OPERATION_MODE,
                         &wifimode,
                         &dataLen))
    {
        return -1;
    }
    if (QAPI_WLAN_DEV_MODE_AP_E != wifimode)
    {
        //QCLI_Printf(qcli_wlan_group, "Err:Set AP Mode to apply AP settings\r\n");
        return -1;
    }
    if((dtim_period < 1) || (dtim_period > 255))
    {
         return -1;
    }
    if (0 != qapi_WLAN_Set_Param (deviceId,
                                  __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                                  __QAPI_WLAN_PARAM_GROUP_WIRELESS_AP_DTIM_INTERVAL,
                                  &dtim_period,
                                  sizeof(dtim_period),  
                                  FALSE))
    {
        //QCLI_Printf(qcli_wlan_group, "Err:set dtim_period\r\n");
          return -1;
    }
    return 0;
}

int32_t set_ap_ps_buf(uint32_t ps_buf_enable, uint32_t buff_count)
{
    uint32_t deviceId = 0, wifimode = 0, dataLen = 0;
    uint8_t ps_val[2] = {0};
    deviceId = get_active_device();

    if (0 != qapi_WLAN_Get_Param (deviceId, 
                         __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                         __QAPI_WLAN_PARAM_GROUP_WIRELESS_OPERATION_MODE,
                         &wifimode,
                         &dataLen))
    {
        return -1;
    }
    if (QAPI_WLAN_DEV_MODE_AP_E != wifimode)
    {
        //QCLI_Printf(qcli_wlan_group, "Err:Set AP Mode to apply AP settings\r\n");
        return -1;
    }
    ps_val[0] = (uint8_t) ps_buf_enable;

    ps_val[1] = (ps_buf_enable == 0) ? 0: (uint8_t) buff_count;

    if (0 != qapi_WLAN_Set_Param (deviceId,
                                  __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                                  __QAPI_WLAN_PARAM_GROUP_WIRELESS_AP_PS_BUF,
                                  &ps_val,
                                  sizeof(ps_val),  
                                  FALSE))
    {
        //QCLI_Printf(qcli_wlan_group, "Err:set PS-Buff\r\n");
          return -1;
    }
    return 0;
}

int32_t set_ap_uapsd(uint32_t uapsd_enable)
{
	uint32_t deviceId = 0, wifimode = 0, dataLen = 0;
	
	deviceId = get_active_device();
	if (0 != qapi_WLAN_Get_Param (deviceId, 
                         __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                         __QAPI_WLAN_PARAM_GROUP_WIRELESS_OPERATION_MODE,
                         &wifimode,
                         &dataLen))
    {
        return -1;
    }
    if (QAPI_WLAN_DEV_MODE_AP_E != wifimode)
    {
        return -1;
    }
	if((uapsd_enable != 0) && (uapsd_enable != 1))
	{
		return -1;
	}
    if (0 != qapi_WLAN_Set_Param (deviceId,
                              __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                              __QAPI_WLAN_PARAM_GROUP_WIRELESS_AP_ENABLE_UAPSD,
                              &uapsd_enable,
                              sizeof(uapsd_enable),  
                              FALSE))
    {
          return -1;
    }
    return 0;
}

#endif


int32_t set_sta_uapsd(uint32_t ac_mask)
{
	uint32_t deviceId = 0, wifimode = 0, dataLen = 0;
	
	deviceId = get_active_device();
	if (0 != qapi_WLAN_Get_Param (deviceId, 
                         __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                         __QAPI_WLAN_PARAM_GROUP_WIRELESS_OPERATION_MODE,
                         &wifimode,
                         &dataLen))
    {
        return -1;
    }
    if (QAPI_WLAN_DEV_MODE_STATION_E != wifimode)
    {
        return -1;
    }
	if(ac_mask < 0 || ac_mask > 15)
	{
		return -1;
	}
    if (0 != qapi_WLAN_Set_Param (deviceId,
                              __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                              __QAPI_WLAN_PARAM_GROUP_WIRELESS_STA_UAPSD,
                              &ac_mask,
                              sizeof(ac_mask),  
                              FALSE))
    {
          return -1;
    }
    return 0;
}

int32_t set_max_sp_len(uint32_t maxsp)
{
	uint32_t deviceId = 0, wifimode = 0, dataLen = 0;
	
	deviceId = get_active_device();
	if (0 != qapi_WLAN_Get_Param (deviceId, 
                         __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                         __QAPI_WLAN_PARAM_GROUP_WIRELESS_OPERATION_MODE,
                         &wifimode,
                         &dataLen))
    {
        return -1;
    }
    if (QAPI_WLAN_DEV_MODE_STATION_E != wifimode)
    {
        return -1;
    }
	if(maxsp < 0 || maxsp > 3)
	{
		return -1;
	}
    if (0 != qapi_WLAN_Set_Param (deviceId,
                              __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                              __QAPI_WLAN_PARAM_GROUP_WIRELESS_STA_MAX_SP_LEN,
                              &maxsp,
                              sizeof(maxsp),  
                              FALSE))
    {
          return -1;
    }
    return 0;
}


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : qcom_get_wps_net_info()
* Parames:      
*        void
* Returned Value  : qapi_WLAN_Netparams_t * - the structure point to netinfo
*            NULL - on any failure. 
* Comments        : 
*                   Not supported on RTCS
*END*-----------------------------------------------------------------*/
qapi_WLAN_Netparams_t * qcom_get_wps_net_info(void)
{
    qapi_WLAN_Netparams_t *pNetparams = &wps_context.netparams;
    if(pNetparams->error)
      return NULL;
    else
      return pNetparams;
}

int32_t get_country_code()
{
#ifdef NEW_FEATURE_INTEGRATION //from 3.3.4
    unsigned char country_code[3] = {'\0'};
    int i = 0;
    uint32_t deviceId = get_active_device();
    //qcom_get_country_code(deviceId,country_code);
#endif
    return 0;
}

int32_t set_country_code(char *country)
{
    char country_code[4] = {'\0'};
	
    if (strlen(country) > 3)
        return QCLI_STATUS_ERROR_E;

    memset(&country_code[0], 0, sizeof(country_code));    
    memcpy(country_code, country, strlen(country));

    qapi_WLAN_Set_Param(get_active_device(),
                        __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                        __QAPI_WLAN_PARAM_GROUP_WIRELESS_COUNTRY_CODE,
                        &country_code[0],
                        sizeof(country_code),
                        FALSE);
    qapi_WLAN_Set_Param(get_active_device(),
                        __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                        __QAPI_WLAN_PARAM_GROUP_WIRELESS_AP_COUNTRY_CODE,
                        &country_code[0],
                        sizeof(country_code),
                        FALSE);
    return QCLI_STATUS_SUCCESS_E;
}

int32_t roam(int32_t enable)
{
    uint32_t roamMode = 0;
    if(enable == 1)
    {
       roamMode = 1;//enable roam 
    }else if(enable == 0)
    {
       roamMode = 3;//disable roam
    }

    return qapi_WLAN_Set_Param(get_active_device(),
                        __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                        __QAPI_WLAN_PARAM_GROUP_WIRELESS_ENABLE_ROAMING,
                        &roamMode,
                        sizeof(roamMode),
                        FALSE);
}


/*FUNCTION*-----------------------------------------------------------------
*
* Function Name  : set_gtx
* Returned Value : 0
* Comments       : Enable/Disable GTX
*
*END------------------------------------------------------------------*/
int32_t set_gtx(uint32_t enGtx)
{
    return qapi_WLAN_Set_Param(get_active_device(),
                               __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                               __QAPI_WLAN_PARAM_GROUP_WIRELESS_ENABLE_GREEN_TX,
                               &enGtx,
                               sizeof(enGtx),
                               FALSE);
}

/*FUNCTION*-----------------------------------------------------------------
*
* Function Name  : set_lpl
* Returned Value : 0
* Comments       : Enable/Disable LPL
*
*END------------------------------------------------------------------*/
int32_t set_lpl(uint32_t enLpl)
{
    /*For LPL feature it doesn't depend on devices*/
    return qapi_WLAN_Set_Param(0,
                               __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                               __QAPI_WLAN_PARAM_GROUP_WIRELESS_ENABLE_LOW_POWER_LISTEN,
                               &enLpl,
                               sizeof(enLpl),
                               FALSE);
}


/*FUNCTION*-----------------------------------------------------------------
*
* Function Name  : wnm_configurations
* Returned Value : 0 on succees or QCLI_STATUS_ERROR_E
*END------------------------------------------------------------------*/
int32_t wnm_config(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    uint32_t enet_device = get_active_device();
	uint8_t wnm_enable = Parameter_List[0].Integer_Value;
    set_callback(NULL);
	return qapi_WLAN_Set_Param (enet_device,
		                        __QAPI_WLAN_PARAM_GROUP_WIRELESS,
						        __QAPI_WLAN_PARAM_GROUP_WIRELESS_WNM_CONFIG,
						        &wnm_enable, sizeof(uint8_t), FALSE);
}

int32_t set_ap_bss_max_idle_period(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	uint32_t enet_device = get_active_device();
    uint32_t wifimode = 0, dataLen = 0;
    
    qapi_WLAN_Get_Param (enet_device, 
                        __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                        __QAPI_WLAN_PARAM_GROUP_WIRELESS_OPERATION_MODE,
                        &wifimode,
                        &dataLen);
    if(wifimode != QAPI_WLAN_DEV_MODE_AP_E)
    {
        QCLI_Printf(qcli_wlan_group, "Set device Mode to AP.\r\n");
        return QCLI_STATUS_ERROR_E;
    }

    set_callback(NULL); 
	qapi_WLAN_BSS_Max_Idle_Period_t period_info;
	period_info.period = Parameter_List[0].Integer_Value;
	period_info.protected_Keep_Alive = 0;
	return qapi_WLAN_Set_Param (enet_device,
					 __QAPI_WLAN_PARAM_GROUP_WIRELESS,
					 __QAPI_WLAN_PARAM_GROUP_WIRELESS_WNM_AP_BSS_MAX_IDLE_PERIOD,
					 &period_info,
					 sizeof(period_info), FALSE);
}

int32_t set_wnm_sleep_period(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	uint32_t enet_device = get_active_device();
	uint32_t wifimode = 0, dataLen = 0;
    set_callback(NULL);
	qapi_WLAN_Get_Param(enet_device,
		                __QAPI_WLAN_PARAM_GROUP_WIRELESS,
						 __QAPI_WLAN_PARAM_GROUP_WIRELESS_OPERATION_MODE,
						 &wifimode, &dataLen);

	if (wifimode != QAPI_WLAN_DEV_MODE_STATION_E) {
		QCLI_Printf(qcli_wlan_group, "Set device Mode to station.\r\n");
		return QCLI_STATUS_ERROR_E;
    }

	qapi_WLAN_WNM_Sleep_Period_t period_info;
	period_info.action_Type = (uint16_t) Parameter_List[0].Integer_Value;
	period_info.duration = (uint16_t) Parameter_List[1].Integer_Value;
	return qapi_WLAN_Set_Param (enet_device,
									__QAPI_WLAN_PARAM_GROUP_WIRELESS,
								__QAPI_WLAN_PARAM_GROUP_WIRELESS_WNM_STA_SLEEP_PERIOD,
									&period_info,
									sizeof(period_info), FALSE);
}

int32_t set_wnm_sleep_response(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    uint32_t enet_device = get_active_device();
    uint32_t wifimode = 0, dataLen = 0;
    
    qapi_WLAN_Get_Param (enet_device, 
                        __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                        __QAPI_WLAN_PARAM_GROUP_WIRELESS_OPERATION_MODE,
                        &wifimode,
                        &dataLen);
    if(wifimode != QAPI_WLAN_DEV_MODE_AP_E)
    {
        QCLI_Printf(qcli_wlan_group, "Set device Mode to AP.\r\n");
        return QCLI_STATUS_ERROR_E;
    }

    set_callback(NULL); 
    uint8_t response = Parameter_List[0].Integer_Value;
    return qapi_WLAN_Set_Param (enet_device,
                                        __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                                        __QAPI_WLAN_PARAM_GROUP_WIRELESS_WNM_AP_SLEEP_RESPONSE,
                                        &response,
                                        sizeof(uint8_t), FALSE);
}

int32_t enable_wow(uint32_t cmd)
{
    uint32_t enet_device = get_active_device();
        if(qapi_WLAN_Set_Param(enet_device, 
                                  __QAPI_WLAN_PARAM_GROUP_WIRELESS, 
                                  __QAPI_WLAN_PARAM_GROUP_WIRELESS_ENABLE_WAKE_ON_WIRELESS, 
                          &cmd, 
                          sizeof(cmd), 
                          FALSE) != 0)
        {
            QCLI_Printf(qcli_wlan_group,"set wow enable error\r\n");
            return -1;
        }
    return 0;
}

int32_t enable_pkt_filter(uint32_t cmd)
{
    uint32_t enet_device = get_active_device();
    if(qapi_WLAN_Set_Param(enet_device, 
                          __QAPI_WLAN_PARAM_GROUP_WIRELESS, 
                          __QAPI_WLAN_PARAM_GROUP_WIRELESS_ENABLE_PKT_FILTER, 
                          &cmd, 
                          sizeof(cmd), 
                          FALSE) != 0)
    {
        QCLI_Printf(qcli_wlan_group,"set pkt filter enable error\r\n");
                return -1;
    }

    return 0;
}

int32_t add_pattern(uint32_t pattern_index, uint8_t action, uint8_t wow_filter, uint8_t priority, uint16_t header_type, uint32_t offset, 
                                uint32_t pattern_size, uint8_t *pattern_mask, uint8_t *pattern_data)
{
    uint32_t enet_device = get_active_device();
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
        if(qapi_WLAN_Set_Param(enet_device, 
                                  __QAPI_WLAN_PARAM_GROUP_WIRELESS, 
                          __QAPI_WLAN_PARAM_GROUP_WIRELESS_ADD_PATTERN, 
                          &pattern,
          sizeof(pattern), 
                        FALSE) != 0)
        {
            QCLI_Printf(qcli_wlan_group,"set pattern error\r\n");
            return -1;
        }
    return 0;
}

int32_t delete_pattern(uint32_t index, uint32_t header_type)
{

    uint32_t enet_device = get_active_device();
	qapi_WLAN_Delete_Pattern_t pattern;

    pattern.pattern_Index = index;
	pattern.header_Type = header_type;
    if(qapi_WLAN_Set_Param(enet_device, 
                               __QAPI_WLAN_PARAM_GROUP_WIRELESS, 
                               __QAPI_WLAN_PARAM_GROUP_WIRELESS_DELETE_PATTERN, 
                               &pattern,
                               sizeof(pattern), 
                               FALSE) != 0)
    {
        QCLI_Printf(qcli_wlan_group, "set delete pattern error\r\n");
        return -1;
    }

    return 0;
}

int32_t change_default_filter_action(uint32_t action, uint32_t wow_flag,uint32_t header_type)
{

    uint32_t enet_device = get_active_device();
	qapi_WLAN_Change_Default_Filter_Action_t pattern;
	
    pattern.pattern_Action_Flag= action;
    pattern.pattern_Action_Flag |= (wow_flag)? __QAPI_WLAN_PATTERN_WOW_FLAG:0;
	pattern.header_Type = header_type;
    if(qapi_WLAN_Set_Param(enet_device, 
                               __QAPI_WLAN_PARAM_GROUP_WIRELESS, 
                               __QAPI_WLAN_PARAM_GROUP_WIRELESS_CHANGE_DEFAULT_FILTER_ACTION, 
                               &pattern,
                               sizeof(pattern), 
                               FALSE) != 0)
    {
        QCLI_Printf(qcli_wlan_group, "set delete pattern error\r\n");
        return -1;
    }

    return 0;
}

/*
Following function is used to set application specified IE in the given frame type.
Input is taken as a string and every input character is a nibble which means every 2 character 
is a byte, two characters are converted into a hex number before putting it in the frame.
Example: the string '12' will show up as 0x12 in hex format in the frame
The application information element should follow the format as below:
Element ID(1 byte)|Length(1 byte)|OUI(3 bytes)|Vendor-specific content((Length-3)bytes)
The Element ID must be 0xdd and length >=3 and length<=255.
*/
int32_t set_app_ie(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    uint32_t enet_device = get_active_device();
    uint32_t wifimode = 0, dataLen = 0;
	int32_t return_code = 0;
	uint32_t length = 0, i = 0;
	uint8 tmpvalue1 = 0, tmpvalue2 = 0;
	qapi_WLAN_App_Ie_Params_t ie_params;

	qapi_WLAN_Get_Param(enet_device,
		                __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                        __QAPI_WLAN_PARAM_GROUP_WIRELESS_OPERATION_MODE,
                        &wifimode, &dataLen);

	ie_params.mgmt_Frame_Type = Parameter_List[0].Integer_Value;

	if ((wifimode == QAPI_WLAN_DEV_MODE_STATION_E) && (ie_params.mgmt_Frame_Type != QAPI_WLAN_FRAME_PROBE_REQ_E))
	{
        QCLI_Printf(qcli_wlan_group, "In station mode, application specified information element can be added only in probe request frames\r\n");
        return -1;
	}

	if ((wifimode == QAPI_WLAN_DEV_MODE_AP_E) &&
		 ((ie_params.mgmt_Frame_Type != QAPI_WLAN_FRAME_BEACON_E) && (ie_params.mgmt_Frame_Type != QAPI_WLAN_FRAME_PROBE_RESP_E)))
	{
		QCLI_Printf(qcli_wlan_group, "In soft-AP mode, application specified information element can be added only in beacon and probe response frames\r\n");
		return -1;
	}

	length= strlen((char *)Parameter_List[1].String_Value);
    if (length < 2)
    {
		QCLI_Printf(qcli_wlan_group, "Invalid application specified information element length. Application specified information element must start with 'dd'\r\n");
		return -1;
	}

	if (length%2 !=0)
    {
		QCLI_Printf(qcli_wlan_group, "Invalid application specified information element length. The length must be a multiple of two.\r\n");
		return -1;
	}

	/* The length must be not less than 10 as every two input characters are converted into a hex number 
	 * and a valid application information element at least has element ID, length and OUI per 802.11 spec.
	 */
	if(length > 2 && length < 10)
	{
		QCLI_Printf(qcli_wlan_group, "The input characters cannot be converted into a valid application element information.\r\n");
		QCLI_Printf(qcli_wlan_group, "The input characters should follow the format:Element ID(1 byte)|Length(1 byte)|OUI(3 bytes)|Vendor-specific content((Length-3)bytes).\r\n");
		return -1;
	}

	ie_params.ie_Len = length/2;
	
    if ((strncmp((char *)(Parameter_List[1].String_Value), "dd",2) != 0))
    {
		QCLI_Printf(qcli_wlan_group, "Application specified information element must start with 'dd'\r\n");
		return -1;
    }

	ie_params.ie_Info = (uint8_t *)malloc(ie_params.ie_Len + 1);
	for(i = 0; i < ie_params.ie_Len; i++)
	{
		tmpvalue1 = ascii_to_hex(Parameter_List[1].String_Value[2*i]);
		tmpvalue2 = ascii_to_hex(Parameter_List[1].String_Value[2*i+1]);
		if(tmpvalue1 == 0xff ||tmpvalue2 == 0xff)
		{
			free(ie_params.ie_Info);
			QCLI_Printf(qcli_wlan_group, "The characters of Application specified information element only be '0-9', 'a-f' and 'A-F'.\r\n");
			return -1;
		}
		ie_params.ie_Info[i] = ((tmpvalue1<<4)&0xf0)|(tmpvalue2&0xf);
	}

	/* The length in application information element should be the length of OUI + vendor-specific content*/
	if((ie_params.ie_Len > 1) && (ie_params.ie_Info[1] != (ie_params.ie_Len -2)))
	{	
		free(ie_params.ie_Info);
		QCLI_Printf(qcli_wlan_group, "The length in application information element is not correct, it should be the length of OUI + vendor-specific content. \r\n");
		return -1;
	}
	
	ie_params.ie_Info[ie_params.ie_Len] = '\0';
	return_code = qapi_WLAN_Set_Param (enet_device,
                                       __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                                       __QAPI_WLAN_PARAM_GROUP_WIRELESS_APP_IE,
                                       &ie_params,
                                       sizeof(qapi_WLAN_App_Ie_Params_t),
                                       FALSE);
    free(ie_params.ie_Info);
	return return_code;
}

int32_t set_sta_bmiss_config(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{

    qapi_WLAN_Sta_Config_Bmiss_Config_t bmiss_config;
    uint32_t deviceId = get_active_device();

    memset(&bmiss_config, 0, sizeof(bmiss_config));

    bmiss_config.bmiss_Time_In_Ms = Parameter_List[0].Integer_Value;
    bmiss_config.num_Beacons = Parameter_List[1].Integer_Value;

    /* Beacon miss timeout allowed is between 1sec to 10sec*/
    if (bmiss_config.num_Beacons == 0 && (bmiss_config.bmiss_Time_In_Ms < 1000 ||
        bmiss_config.bmiss_Time_In_Ms > 10000))
    {
        QCLI_Printf(qcli_wlan_group, "ERROR: bmiss_Time_In_Ms out of range, allowed 1000 to 10000\n");
        return -1;
    }

    /* Assuming 100TU as Beacon interval the accepted range for
        number of beacon miss allowed is between 10 to 100*/
    if (bmiss_config.bmiss_Time_In_Ms ==0 && (bmiss_config.num_Beacons < 10 ||
        bmiss_config.num_Beacons > 100))
    {
        QCLI_Printf(qcli_wlan_group, "ERROR: num_Beacons out of range, allowed 10 to 100\n");
        return -1;
    }

    if( 0 != qapi_WLAN_Set_Param (deviceId, 
                        __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                        __QAPI_WLAN_PARAM_GROUP_WIRELESS_STA_BMISS_CONFIG,
                        &bmiss_config,
                        sizeof(bmiss_config),
                        FALSE))
    {
        QCLI_Printf(qcli_wlan_group, "ERROR: set_sta_bmiss_config\n");
        return -1;
    }
    return 0;
}

void print_ant_div_info(qapi_WLAN_Get_Ant_Div_t *wlanAntDivInfo)
{
	 QCLI_Printf(qcli_wlan_group, "\r\n HW Ant Div Info\r\n");
	 QCLI_Printf(qcli_wlan_group, "enable_Ant_Div=%d\r\n",wlanAntDivInfo->enable_Ant_Div);
	 QCLI_Printf(qcli_wlan_group, "tx_Follow_Rx=%d\r\n",wlanAntDivInfo->tx_Follow_Rx);
	 QCLI_Printf(qcli_wlan_group, "curr_RX_Ant_2g=%d\r\n",wlanAntDivInfo->curr_Rx_Ant_2g);
	 QCLI_Printf(qcli_wlan_group, "curr_Tx_Ant_2g=%d\r\n",wlanAntDivInfo->curr_Tx_Ant_2g);
	 QCLI_Printf(qcli_wlan_group, "curr_RX_Ant_5g=%d\r\n",wlanAntDivInfo->curr_Rx_Ant_5g);
	 QCLI_Printf(qcli_wlan_group, "curr_Tx_Ant_5g=%d\r\n",wlanAntDivInfo->curr_Tx_Ant_5g);
	 QCLI_Printf(qcli_wlan_group, "avg_Main_Rssi=%d\r\n",wlanAntDivInfo->avg_Main_Rssi);
	 QCLI_Printf(qcli_wlan_group, "avg_Alt_Rssi=%d\r\n",wlanAntDivInfo->avg_Alt_Rssi); 
	 QCLI_Printf(qcli_wlan_group, "total_Pkt_Cnt=%d\r\n",wlanAntDivInfo->total_Pkt_Cnt);
	 QCLI_Printf(qcli_wlan_group, "ant_Switch_Cnt=%d\r\n",wlanAntDivInfo->ant_Switch_Cnt);
}

int32_t set_ant_div(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{

    qapi_WLAN_Ant_Div_Config_t wlan_ant_div_config;
    uint32_t deviceId = get_active_device();

    memset(&wlan_ant_div_config, 0, sizeof(wlan_ant_div_config));

    wlan_ant_div_config.enable_Ant_Div = Parameter_List[0].Integer_Value;

	if(wlan_ant_div_config.enable_Ant_Div != QAPI_WLAN_DISABLE_ANT_DIV_E)
	{
		if(Parameter_Count > 2)
		{	
			wlan_ant_div_config.tx_Follow_Rx= Parameter_List[1].Integer_Value;
			wlan_ant_div_config.ant_div_config.hw_ant_div_config.mode = Parameter_List[2].Integer_Value;
			if(wlan_ant_div_config.ant_div_config.hw_ant_div_config.mode != QAPI_WLAN_ANT_DIV_AUTO_MODE_E)
			{
				if(Parameter_Count > 3)
				{
					wlan_ant_div_config.ant_div_config.hw_ant_div_config.param = Parameter_List[3].Integer_Value;
				}
				else
				{
					QCLI_Printf(qcli_wlan_group, "ERROR: miss paratemers when enable ant div\r\n");
					return -1;
				}
			}
		}
		else
		{
			QCLI_Printf(qcli_wlan_group, "ERROR: miss paratemers when enable ant div\r\n");
			return -1;
		}
	}

    if( 0 != qapi_WLAN_Set_Param (deviceId, 
                        __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                        __QAPI_WLAN_PARAM_GROUP_WIRELESS_SET_ANT_DIV,
                        &wlan_ant_div_config,
                        sizeof(wlan_ant_div_config),
                        FALSE))
    {
        QCLI_Printf(qcli_wlan_group, "ERROR: set_hw_ant_div\r\n");
        return -1;
    }
    return 0;
}

int32_t get_ant_div_status()
{
	uint32_t deviceId = 0, length = 0;
	qapi_WLAN_Get_Ant_Div_t *wlanAntDivInfo;
	int32_t ret = -1;

	deviceId = get_active_device();
	
	if ( NULL == (wlanAntDivInfo = malloc(sizeof(qapi_WLAN_Get_Ant_Div_t)) ) ){		
		/* Failure to allocate memory will drop the event at caller */
		return -1;
	}
	length = sizeof(wlanAntDivInfo);
			
	ret = qapi_WLAN_Get_Param (deviceId, 
							 __QAPI_WLAN_PARAM_GROUP_WIRELESS,
							 __QAPI_WLAN_PARAM_GROUP_WIRELESS_GET_ANT_DIV,
							 wlanAntDivInfo,
							 &length);
	
	if( ret == 0 )
	{
		print_ant_div_info( wlanAntDivInfo );
	}
	else
		QCLI_Printf(qcli_wlan_group, "Failed to Get HW Ant Div Info \r\n");

	free(wlanAntDivInfo);
	
	return ret;
}

int32_t set_antenna(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{

	uint8_t  antNum = 0;
    uint32_t deviceId = get_active_device();

	antNum = Parameter_List[0].Integer_Value;

    if( 0 != qapi_WLAN_Set_Param (deviceId, 
                        __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                        __QAPI_WLAN_PARAM_GROUP_WIRELESS_SET_ANTENNA,
                        &antNum,
                        sizeof(uint8_t),
                        FALSE))
    {
        QCLI_Printf(qcli_wlan_group, "ERROR: set_antenna\r\n");
        return -1;
    }
    return 0;
}


#if ENABLE_P2P_MODE
int32_t p2p_enable()
{
	uint32_t deviceId = get_active_device();
	if(deviceId != 0)
	{
		QCLI_Printf(qcli_p2p_group, "Switch to device 0 to use p2p mode.\r\n");
		QCLI_Printf(qcli_p2p_group, "Current device is %d.\r\n", deviceId);
		return -1;
	}

    /* Following call sets the wlan_callback_handler() as the callback for asynchronous events */
	set_callback(NULL);

	if(qapi_WLAN_P2P_Enable(deviceId, TRUE) != 0)
	{
		QCLI_Printf(qcli_p2p_group, "P2P not enabled.\r\n");
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
	set_passphrase("1234567890");
	memset(original_ssid, 0, __QAPI_WLAN_MAX_SSID_LENGTH);
	strcpy((char *)original_ssid,"DIRECT-iO");

	memset(p2p_join_mac_addr, 0, __QAPI_WLAN_MAC_LEN);
	memset(p2pScratchBuff, 0, sizeof(p2pScratchBuff));

	return QCLI_STATUS_SUCCESS_E;
}


int32_t p2p_disable()
{
	qapi_WLAN_Dev_Mode_e opMode;

	uint32_t deviceId = get_active_device();
	if(deviceId != 0)
	{
		QCLI_Printf(qcli_p2p_group, "Switch to device 0 to use p2p mode.\r\n");
		QCLI_Printf(qcli_p2p_group, "Current device is %d.\r\n", deviceId);
		return -1;
	}

	set_callback(NULL);
	if(qapi_WLAN_P2P_Enable(deviceId, FALSE) != 0)
	{
		QCLI_Printf(qcli_p2p_group, "Disabling P2P mode failed.\r\n");
		return -1;
	}

	opMode = QAPI_WLAN_DEV_MODE_STATION_E;
	qapi_WLAN_Set_Param(deviceId, __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                        __QAPI_WLAN_PARAM_GROUP_WIRELESS_OPERATION_MODE,
                        &opMode, sizeof(qapi_WLAN_Dev_Mode_e), FALSE);

    /* Free the event queue and destroy the mutex before disabling P2P. */
    app_free_p2p_pending_events();

	p2pMode = FALSE;
    set_channel_p2p = 0;
	return QCLI_STATUS_SUCCESS_E;
}

int32_t p2p_set_config(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	qapi_WLAN_P2P_Config_Params_t p2pConfig;

	uint32_t deviceId = get_active_device();
	if(deviceId != 0)
	{
		QCLI_Printf(qcli_p2p_group, "Switch to device 0 to use p2p mode.\r\n");
		QCLI_Printf(qcli_p2p_group, "Current device is %d.\r\n", deviceId);
		return -1;
	}

	set_callback(NULL);
    set_channel_p2p = Parameter_List[2].Integer_Value; // for autogo

	p2p_intent = Parameter_List[0].Integer_Value;
	p2pConfig.go_Intent      = p2p_intent;
	p2pConfig.listen_Chan    = (uint8_t)Parameter_List[1].Integer_Value;
	p2pConfig.op_Chan	     = (uint8_t)Parameter_List[2].Integer_Value;
	p2pConfig.age		     = (uint32_t)Parameter_List[4].Integer_Value;
	p2pConfig.reg_Class      = 81;
	p2pConfig.op_Reg_Class   = 81;
	p2pConfig.max_Node_Count = 5;

	if (0 != qapi_WLAN_Set_Param(deviceId, __QAPI_WLAN_PARAM_GROUP_P2P,
                                 __QAPI_WLAN_PARAM_GROUP_P2P_CONFIG_PARAMS,
                                 &p2pConfig, sizeof(p2pConfig), FALSE))
	{
		QCLI_Printf(qcli_p2p_group, "P2P configuration failed.\r\n");
		return -1;
	}

	QCLI_Printf(qcli_p2p_group, "Device configuration set successfully.\r\n");
	QCLI_Printf(qcli_p2p_group, "Note: Cannot set country code.\r\n");
	QCLI_Printf(qcli_p2p_group, "Use board data file or tuneables instead.\r\n");


	return QCLI_STATUS_SUCCESS_E;
}

uint8_t P2P_Check_Peer_Is_Found(const uint8_t *peer_addr, uint8_t p2p_operation)
{
	uint32_t deviceId = 0, dataLen = 0;
	uint8_t *local_ptr = NULL, index = 0, temp_val = 0, peer_found = FALSE;
	qapi_WLAN_P2P_Node_List_Params_t p2pNodeList;

	deviceId = get_active_device();
	if(deviceId != 0)
	{
		QCLI_Printf(qcli_p2p_group, "Switch to device 0 to use p2p mode. ");
		QCLI_Printf(qcli_p2p_group, "Current device is %d.\r\n", deviceId);
		return FALSE;
	}

	set_callback(NULL);

	memset(p2pScratchBuff, 0, sizeof(p2pScratchBuff));
	p2pNodeList.buffer_Length = __QAPI_WLAN_P2P_EVT_BUF_SIZE;
	p2pNodeList.node_List_Buffer = p2pScratchBuff;
	
	if (0 != qapi_WLAN_Get_Param(deviceId,
                                 __QAPI_WLAN_PARAM_GROUP_P2P,
                                 __QAPI_WLAN_PARAM_GROUP_P2P_NODE_LIST,
                                 &p2pNodeList, &dataLen))
	{
		QCLI_Printf(qcli_p2p_group, "P2P node list command failed.\r\n");
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
   		QCLI_Printf(qcli_p2p_group, "The Peer Device is not found, please do P2P Find again.\r\n");
   	}
   
   return peer_found;
}

int32_t p2p_connect(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	uint32_t deviceId = 0, dataLen = 0;
    qapi_WLAN_P2P_Connect_Cmd_t p2p_connect;

	deviceId = get_active_device();
	if(deviceId != 0)
	{
		QCLI_Printf(qcli_p2p_group, "Switch to device 0 to use p2p mode.\r\n");
		QCLI_Printf(qcli_p2p_group, "Current device is %d.\r\n", deviceId);
		return -1;
	}

	set_callback(NULL);
	memset(&p2p_connect, 0, sizeof( qapi_WLAN_P2P_Connect_Cmd_t));

	if (0 != qapi_WLAN_Get_Param (deviceId, 
                                  __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                                  __QAPI_WLAN_PARAM_GROUP_WIRELESS_MAC_ADDRESS,
                                  p2p_connect.own_Interface_Addr, &dataLen))
	{
		QCLI_Printf(qcli_p2p_group, "Unable to obtain device mac address\r\n");
		return -1;
	}

	QCLI_Printf(qcli_p2p_group, "Own MAC addr : %02x:%02x:%02x:%02x:%02x:%02x \r\n",
                p2p_connect.own_Interface_Addr[0], p2p_connect.own_Interface_Addr[1],
                p2p_connect.own_Interface_Addr[2], p2p_connect.own_Interface_Addr[3],
                p2p_connect.own_Interface_Addr[4], p2p_connect.own_Interface_Addr[5]);

	if ((ether_aton((const char *)(Parameter_List[0].String_Value),p2p_connect.peer_Addr)) != 0)
	{
		QCLI_Printf(qcli_p2p_group, "Invalid PEER MAC Address\r\n");
		return -1;
	}

	QCLI_Printf(qcli_p2p_group, "Peer MAC addr : %02x:%02x:%02x:%02x:%02x:%02x \r\n",
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
            QCLI_Printf(qcli_p2p_group, "WPS PIN %s \r\n",p2p_wps_pin);
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

	if(qapi_WLAN_P2P_Connect(deviceId, p2p_connect.wps_Method,
		                     p2p_connect.peer_Addr,
		                     p2p_persistent_go) != 0)
	{
		QCLI_Printf(qcli_p2p_group, "P2P connect command failed.\r\n");
		p2p_persistent_go = QAPI_WLAN_P2P_NON_PERSISTENT_E;
		return -1;
	}

	return QCLI_STATUS_SUCCESS_E;
}

int32_t p2p_find(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    qapi_WLAN_P2P_Find_Cmd_t find_params;

	uint32_t deviceId = get_active_device();
	if(deviceId != 0)
	{
		QCLI_Printf(qcli_p2p_group, "Switch to device 0 to use p2p mode.\r\n");
		QCLI_Printf(qcli_p2p_group, "Current device is %d.\r\n", deviceId);
		return -1;
	}

	set_callback(NULL);
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
			QCLI_Printf(qcli_p2p_group, "Wrong option. Enter option 1,2 or 3\r\n");
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
			QCLI_Printf(qcli_p2p_group, "Wrong option. Enter option 1,2 or 3\r\n");
			return -1;
		}
	}

	else
	{
		find_params.type = (QAPI_WLAN_P2P_DISC_ONLY_SOCIAL_E);
		find_params.timeout = (P2P_STANDARD_TIMEOUT);
	}

	if(qapi_WLAN_P2P_Find(deviceId, find_params.type, find_params.timeout) != 0)
	{
		QCLI_Printf(qcli_p2p_group, "P2P find command failed.\r\n");
		return -1;
	}

	return QCLI_STATUS_SUCCESS_E;
}

int32_t p2p_provision(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    qapi_WLAN_P2P_Prov_Disc_Req_Cmd_t p2p_prov_disc;
    qapi_WLAN_P2P_Connect_Cmd_t p2p_connect;

	uint32_t deviceId = get_active_device();
	if(deviceId != 0)
	{
		QCLI_Printf(qcli_p2p_group, "Switch to device 0 to use p2p mode.\r\n");
		QCLI_Printf(qcli_p2p_group, "Current device is %d.\r\n", deviceId);
		return -1;
	}

	set_callback(NULL);

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
        QCLI_Printf(qcli_p2p_group, "Incorrect WPS method\r\n");
        return -1;
    }

	p2p_prov_disc.dialog_Token = 1;

	if ((ether_aton((const char *)(Parameter_List[0].String_Value),p2p_prov_disc.peer)) != 0)
	{
		QCLI_Printf(qcli_p2p_group, "Invalid PEER MAC Address\r\n");
		return -1;
	}        

	if(!P2P_Check_Peer_Is_Found(p2p_prov_disc.peer,P2P_PROVISION_OPERATION))
	{
		return -1;
	}

	if( qapi_WLAN_P2P_Prov(deviceId, p2p_prov_disc.wps_Method, p2p_prov_disc.peer) != 0 ) 	   
	{
		QCLI_Printf(qcli_p2p_group, "P2P provision command failed.\r\n");
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
		QCLI_Printf(qcli_p2p_group, "Invalid PEER MAC Address\r\n");
		return -1;
	}

	p2p_connect.go_Intent = p2p_intent;
	if(p2p_connect.wps_Method != QAPI_WLAN_P2P_WPS_NOT_READY_E)
    {
        if(qapi_WLAN_P2P_Auth(deviceId, p2p_connect.dev_Auth,
                               p2p_connect.wps_Method, p2p_connect.peer_Addr,
                               ((p2p_connect.dev_Capab & __QAPI_WLAN_P2P_PERSISTENT_FLAG) ?
                               QAPI_WLAN_P2P_PERSISTENT_E : QAPI_WLAN_P2P_NON_PERSISTENT_E)) != 0)
		{
			QCLI_Printf(qcli_p2p_group, "P2P provision command failed.\r\n");
			return -1;
		}
	}
	return QCLI_STATUS_SUCCESS_E;
}

int32_t p2p_listen(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	uint32_t timeout_val = 0;

	uint32_t deviceId = get_active_device();
	if(deviceId != 0)
	{
		QCLI_Printf(qcli_p2p_group, "Switch to device 0 to use p2p mode.\r\n");
		QCLI_Printf(qcli_p2p_group, "Current device is %d.\r\n", deviceId);
		return -1;
	}

	set_callback(NULL);
	if (Parameter_Count == 1 && Parameter_List[0].Integer_Is_Valid)
	{
		timeout_val = Parameter_List[0].Integer_Value;
	}
	else
	{
		timeout_val = P2P_STANDARD_TIMEOUT;
	}

	if(qapi_WLAN_P2P_Listen(deviceId, timeout_val) != 0)
	{
		QCLI_Printf(qcli_p2p_group, "P2P listen command failed.\r\n");
		return -1;
	}
	return QCLI_STATUS_SUCCESS_E;
}

int32_t p2p_cancel()
{
	uint32_t deviceId = 0, dataLen = 0;
	qapi_WLAN_P2P_Network_List_Params_t p2pNetworkList;
    qapi_WLAN_Dev_Mode_e opMode;

	deviceId = get_active_device();
	if(deviceId != 0)
	{
		QCLI_Printf(qcli_p2p_group, "Switch to device 0 to use p2p mode.\r\n");
		QCLI_Printf(qcli_p2p_group, "Current device is %d.\r\n", deviceId);
		return -1;
	}

	set_callback(NULL);
	if(qapi_WLAN_P2P_Cancel(deviceId) != 0)
	{
		QCLI_Printf(qcli_p2p_group, "P2P cancel command failed.\r\n");
		return -1;
	}

	autogo_newpp = FALSE;
	p2p_cancel_enable = 1;

	memset(p2pScratchBuff, 0, sizeof(p2pScratchBuff));
	p2pNetworkList.network_List_Buffer = p2pScratchBuff;
	p2pNetworkList.buffer_Length = __QAPI_WLAN_P2P_EVT_BUF_SIZE;

	if (0 != qapi_WLAN_Get_Param(deviceId,
                                 __QAPI_WLAN_PARAM_GROUP_P2P,
                                 __QAPI_WLAN_PARAM_GROUP_P2P_NETWORK_LIST,
                                 &p2pNetworkList, &dataLen))
	{
		QCLI_Printf(qcli_p2p_group, "P2P cancel command did not execute properly\r\n");
		return -1;
	}

    /* Following call to app_p2p_process_persistent_list_event() is made to make
     * p2p_cancel() blocking so that no other asynchronous p2p operations happen before
     * p2p_cancel() completes. */
	app_p2p_process_persistent_list_event(p2pNetworkList.network_List_Buffer);

	/* Reset to mode station */
	opMode = QAPI_WLAN_DEV_MODE_STATION_E;
	qapi_WLAN_Set_Param(deviceId, 
			__QAPI_WLAN_PARAM_GROUP_WIRELESS,
			__QAPI_WLAN_PARAM_GROUP_WIRELESS_OPERATION_MODE,
			&opMode,
			sizeof(qapi_WLAN_Dev_Mode_e),
			FALSE);               

	p2p_persistent_done = 0;
	set_power_mode( QAPI_WLAN_POWER_MODE_REC_POWER_E, QAPI_WLAN_POWER_MODULE_P2P_E);

	return QCLI_STATUS_SUCCESS_E;
}

int32_t p2p_join(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	uint32_t deviceId = 0, dataLen = 0;
    qapi_WLAN_P2P_Node_List_Params_t p2pNodeList;

	deviceId = get_active_device();
	if(deviceId != 0)
	{
		QCLI_Printf(qcli_p2p_group, "Switch to device 0 to use p2p mode.\r\n");
		QCLI_Printf(qcli_p2p_group, "Current device is %d.\r\n", deviceId);
		return -1;
	}

	set_callback(NULL);

	if ((ether_aton((const char *)(Parameter_List[0].String_Value),p2p_join_mac_addr)) != 0)
	{
		QCLI_Printf(qcli_p2p_group, "Invalid PEER MAC Address\r\n");
		return -1;
	}

	QCLI_Printf(qcli_p2p_group, "Interface MAC addr : %02x:%02x:%02x:%02x:%02x:%02x \r\n",
                p2p_join_mac_addr[0], p2p_join_mac_addr[1], p2p_join_mac_addr[2],
                p2p_join_mac_addr[3], p2p_join_mac_addr[4], p2p_join_mac_addr[5]);

	/* Update join profile */
	memset(&p2p_join_profile, 0, sizeof( qapi_WLAN_P2P_Connect_Cmd_t));
	if (0 != qapi_WLAN_Get_Param (deviceId,
                                  __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                                  __QAPI_WLAN_PARAM_GROUP_WIRELESS_MAC_ADDRESS,
                                  p2p_join_profile.own_Interface_Addr,
                                  &dataLen))
	{
		QCLI_Printf(qcli_p2p_group, "Unable to obtain device mac address\r\n");
		return -1;
	}

	QCLI_Printf(qcli_p2p_group, "Own MAC addr : %02x:%02x:%02x:%02x:%02x:%02x \r\n",
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

	if (0 != qapi_WLAN_Get_Param(deviceId,
                                 __QAPI_WLAN_PARAM_GROUP_P2P,
                                 __QAPI_WLAN_PARAM_GROUP_P2P_NODE_LIST,
                                 &p2pNodeList, &dataLen))
	{
		QCLI_Printf(qcli_p2p_group, "P2P join command did not execute properly\r\n");
		return -1;
	}

	app_p2p_process_node_list_event(p2pNodeList.node_List_Buffer);
	return QCLI_STATUS_SUCCESS_E;
}

int32_t p2p_auth(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    qapi_WLAN_P2P_Connect_Cmd_t p2p_connect;

	uint32_t deviceId = get_active_device();
	if(deviceId != 0)
	{
		QCLI_Printf(qcli_p2p_group, "Switch to device 0 to use p2p mode.\r\n");
		QCLI_Printf(qcli_p2p_group, "Current device is %d.\r\n", deviceId);
		return -1;
	}

	set_callback(NULL);
	memset(&p2p_connect, 0, sizeof(qapi_WLAN_P2P_Connect_Cmd_t));

	if(strlen((char *)Parameter_List[2].String_Value) == 8)
    {
		memset(p2p_wps_pin, 0, __QAPI_WLAN_WPS_PIN_LEN);
		strcpy(p2p_wps_pin, (const char *)(Parameter_List[2].String_Value));
		QCLI_Printf(qcli_p2p_group, "WPS Pin %s\r\n",p2p_wps_pin);

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
		QCLI_Printf(qcli_p2p_group, "WPS PIN %s \r\n",p2p_wps_pin);
	}
	else if(APP_STRCMP(Parameter_List[1].String_Value,"keypad") == 0)
	{
		p2p_connect.wps_Method = QAPI_WLAN_P2P_WPS_PIN_KEYPAD_E;
	}

	if ((ether_aton((const char *)(Parameter_List[0].String_Value),p2p_connect.peer_Addr)) != 0)
	{
		QCLI_Printf(qcli_p2p_group, "Invalid PEER MAC Address\r\n");
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

	if(qapi_WLAN_P2P_Auth(deviceId, 
                          p2p_connect.dev_Auth, 
                          (qapi_WLAN_P2P_WPS_Method_e) p2p_connect.wps_Method, 
                          p2p_connect.peer_Addr, p2p_persistent_go) != 0)
	{
		QCLI_Printf(qcli_p2p_group, "StartP2P command did not execute properly\r\n");
		p2p_persistent_go = QAPI_WLAN_P2P_NON_PERSISTENT_E;
		return -1;
	}

	return QCLI_STATUS_SUCCESS_E;
}

int32_t p2p_auto_go(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    uint8_t persistent_Group = 0;
    int32_t go_chan = __QAPI_WLAN_P2P_DEFAULT_CHAN;
    uint32_t channel = 0, dataLen = 0;
	uint32_t deviceId = get_active_device();
	if(deviceId != 0)
	{
		QCLI_Printf(qcli_p2p_group, "Switch to device 0 to use p2p mode.\r\n");
		QCLI_Printf(qcli_p2p_group, "Current device is %d.\r\n", deviceId);
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
		set_active_deviceid(1);
		qapi_WLAN_Get_Param (1, 
		 __QAPI_WLAN_PARAM_GROUP_WIRELESS,
		 __QAPI_WLAN_PARAM_GROUP_WIRELESS_CHANNEL,
		 &channel,
		 &dataLen); 
		go_chan = channel;
		set_active_deviceid(0); 
	}
		
	
	set_callback(NULL);
	p2pMode = TRUE;
	wps_flag = 0x01;

	QCLI_Printf(qcli_p2p_group, "Starting Autonomous GO.\r\n");

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

		if(qapi_WLAN_P2P_Start_Go(deviceId, &goParams,  
                                  go_chan, 
                                  persistent_Group) != 0)
		{
			QCLI_Printf(qcli_p2p_group, "P2P auto GO command did not execute properly.\r\n");
			return -1;
		}
	}

	else
	{    
		if(qapi_WLAN_P2P_Start_Go(deviceId, NULL, go_chan,
                                  persistent_Group) != 0)
		{
			QCLI_Printf(qcli_p2p_group, "P2P auto GO command did not execute properly.\r\n");
			return -1;
		}
	}
	return QCLI_STATUS_SUCCESS_E;
}

int32_t p2p_invite_auth(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    qapi_WLAN_P2P_Invite_Cmd_t p2pInvite;
	qapi_WLAN_P2P_Go_Params_t ssidParams;
	qapi_WLAN_Auth_Mode_e authMode = QAPI_WLAN_AUTH_WPA2_PSK_E;
	qapi_WLAN_Crypt_Type_e encrType = QAPI_WLAN_CRYPT_AES_CRYPT_E;
	uint32_t deviceId = 0, wifimode = 0, dataLen = 0, k = 0;
	uint8_t p2p_invite_role;

	deviceId = get_active_device();
	if(deviceId != 0)
	{
		QCLI_Printf(qcli_p2p_group, "Switch to device 0 to use p2p mode.\r\n");
		QCLI_Printf(qcli_p2p_group, "Current device is %d.\r\n", deviceId);
		return -1;
	}

	set_callback(NULL);
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
		qapi_WLAN_Get_Param (deviceId, 
                             __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                             __QAPI_WLAN_PARAM_GROUP_WIRELESS_OPERATION_MODE,
                             &wifimode, &dataLen);
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
		QCLI_Printf(qcli_p2p_group, "Invalid Invitation MAC Address\r\n");
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
		QCLI_Printf(qcli_p2p_group, "Incorrect wps method not proper P2P Invite \r\n");
		return -1;
	}

	if( qapi_WLAN_P2P_Invite(deviceId, (const char *)(Parameter_List[0].String_Value),
		                      p2pInvite.wps_Method, p2pInvite.peer_Addr,
		                      p2pInvite.is_Persistent, p2p_invite_role) != 0 )			  
	{
		QCLI_Printf(qcli_p2p_group, "P2P command did not execute properly\r\n");
		return -1;
	}

	if(p2p_invite_role == QAPI_WLAN_P2P_INV_ROLE_ACTIVE_GO_E)
	{
		if(0 != qapi_WLAN_Set_Param(deviceId,
                                     __QAPI_WLAN_PARAM_GROUP_WIRELESS_SECURITY,
                                     __QAPI_WLAN_PARAM_GROUP_SECURITY_ENCRYPTION_TYPE,
                                     (void *) &encrType, //QCOM_WLAN_CRYPT_AES_CRYPT
                                     sizeof(qapi_WLAN_Crypt_Type_e), FALSE))
		{
			return -1;
		}

		if ( 0 != qapi_WLAN_Set_Param (deviceId,
                                       __QAPI_WLAN_PARAM_GROUP_WIRELESS_SECURITY,
                                       __QAPI_WLAN_PARAM_GROUP_SECURITY_AUTH_MODE,
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

		if ( 0 != qapi_WLAN_Set_Param(deviceId,
                                      __QAPI_WLAN_PARAM_GROUP_P2P,
                                      __QAPI_WLAN_PARAM_GROUP_P2P_GO_PARAMS,
                                      &ssidParams, sizeof(ssidParams), FALSE))
		{
			QCLI_Printf(qcli_p2p_group, "P2P command did not execute properly\r\n");
			return -1;
		}
	}

	return QCLI_STATUS_SUCCESS_E;
}

int32_t p2p_get_node_list()
{
	uint32_t deviceId = 0, dataLen = 0;
	qapi_WLAN_P2P_Node_List_Params_t p2pNodeList;

	deviceId = get_active_device();
	if(deviceId != 0)
	{
		QCLI_Printf(qcli_p2p_group, "Switch to device 0 to use p2p mode. ");
		QCLI_Printf(qcli_p2p_group, "Current device is %d.\r\n", deviceId);
		return -1;
	}

	set_callback(NULL);

	memset(p2pScratchBuff, 0, sizeof(p2pScratchBuff));
	p2pNodeList.buffer_Length = __QAPI_WLAN_P2P_EVT_BUF_SIZE;
	p2pNodeList.node_List_Buffer = p2pScratchBuff;
	
	if (0 != qapi_WLAN_Get_Param(deviceId,
                                 __QAPI_WLAN_PARAM_GROUP_P2P,
                                 __QAPI_WLAN_PARAM_GROUP_P2P_NODE_LIST,
                                 &p2pNodeList, &dataLen))
	{
		QCLI_Printf(qcli_p2p_group, "P2P node list command failed.\r\n");
		return -1;
	}
	
	app_p2p_process_node_list_event(p2pNodeList.node_List_Buffer);
	return QCLI_STATUS_SUCCESS_E;
}

int32_t p2p_get_network_list()
{
	uint32_t deviceId = 0, dataLen = 0;
	qapi_WLAN_P2P_Network_List_Params_t p2pNetworkList;

	deviceId = get_active_device();
	if(deviceId != 0)
	{
		QCLI_Printf(qcli_p2p_group, "Switch to device 0 to use p2p mode. ");
		QCLI_Printf(qcli_p2p_group, "Current device is %d.\r\n", deviceId);
		return -1;
	}

	set_callback(NULL);
	memset(p2pScratchBuff, 0, sizeof(p2pScratchBuff));			 
	p2pNetworkList.network_List_Buffer = p2pScratchBuff;
	p2pNetworkList.buffer_Length = __QAPI_WLAN_P2P_EVT_BUF_SIZE;

	if (0 != qapi_WLAN_Get_Param(deviceId,
                                 __QAPI_WLAN_PARAM_GROUP_P2P,
                                 __QAPI_WLAN_PARAM_GROUP_P2P_NETWORK_LIST,
                                 &p2pNetworkList, &dataLen))
	{
		QCLI_Printf(qcli_p2p_group, "P2P command did not execute properly\r\n");
		return -1;
	}

	app_p2p_process_persistent_list_event(p2pNetworkList.network_List_Buffer);

	return QCLI_STATUS_SUCCESS_E;
}

int32_t p2p_set_noa_params(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	qapi_WLAN_P2P_Noa_Params_t noaParams;

	uint32_t deviceId = get_active_device();
	if(deviceId != 0)
	{
		QCLI_Printf(qcli_p2p_group, "Switch to device 0 to use p2p mode.\r\n");
		QCLI_Printf(qcli_p2p_group, "Current device is %d.\r\n", deviceId);
		return -1;
	}

	set_callback(NULL);
    memset(&noaParams, 0, sizeof(qapi_WLAN_P2P_Noa_Params_t));

	noaParams.noa_Desc_Params[0].type_Count = Parameter_List[0].Integer_Value;
	noaParams.noa_Desc_Params[0].start_Offset_Us = Parameter_List[1].Integer_Value;
	noaParams.noa_Desc_Params[0].duration_Us = Parameter_List[2].Integer_Value;
	noaParams.noa_Desc_Params[0].interval_Us = Parameter_List[3].Integer_Value;
	noaParams.enable = 1;
	noaParams.count = 1;

	if (0 != qapi_WLAN_Set_Param (deviceId,
                                  __QAPI_WLAN_PARAM_GROUP_P2P,
                                  __QAPI_WLAN_PARAM_GROUP_P2P_NOA_PARAMS,
                                  &noaParams,
                                  sizeof(noaParams),
                                  FALSE))
	{
		QCLI_Printf(qcli_p2p_group, "P2P command did not execute properly\r\n");
		return -1;
	}

	return QCLI_STATUS_SUCCESS_E;
}

int32_t p2p_set_oops_params(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	qapi_WLAN_P2P_Opps_Params_t opps;

	uint32_t deviceId = get_active_device();
	if(deviceId != 0)
	{
		QCLI_Printf(qcli_p2p_group, "Switch to device 0 to use p2p mode.\r\n");
		QCLI_Printf(qcli_p2p_group, "Current device is %d.\r\n", deviceId);
		return -1;
	}

	set_callback(NULL);
	memset(&opps, 0, sizeof(qapi_WLAN_P2P_Opps_Params_t));
	opps.ct_Win	= Parameter_List[0].Integer_Value;
	opps.enable = Parameter_List[1].Integer_Value;
	if (0 != qapi_WLAN_Set_Param (deviceId,
                                  __QAPI_WLAN_PARAM_GROUP_P2P,
                                  __QAPI_WLAN_PARAM_GROUP_P2P_OPPS_PARAMS,
                                  &opps, sizeof(opps), FALSE))
	{
		QCLI_Printf(qcli_p2p_group, "P2P command did not execute properly\r\n");
		return -1;
	} 
	return QCLI_STATUS_SUCCESS_E;
}

int32_t p2p_set_operating_class(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	qapi_WLAN_P2P_Config_Params_t p2pConfig;

	uint32_t deviceId = get_active_device();
	if(deviceId != 0)
	{
		QCLI_Printf(qcli_p2p_group, "Switch to device 0 to use p2p mode.\r\n");
		QCLI_Printf(qcli_p2p_group, "Current device is %d.\r\n", deviceId);
		return -1;
	}

	set_callback(NULL);
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

	if (0 != qapi_WLAN_Set_Param(deviceId,
                                 __QAPI_WLAN_PARAM_GROUP_P2P,
                                 __QAPI_WLAN_PARAM_GROUP_P2P_CONFIG_PARAMS,
                                 &p2pConfig, sizeof(p2pConfig), FALSE))
	{
		QCLI_Printf(qcli_p2p_group, "P2P command did not execute properly\r\n");
		return -1;
	}
	return QCLI_STATUS_SUCCESS_E;
}


int32_t p2p_stop_find()
{
	uint32_t deviceId = get_active_device();
	if(deviceId != 0)
	{
		QCLI_Printf(qcli_p2p_group, "Switch to device 0 to use p2p mode.\r\n");
		QCLI_Printf(qcli_p2p_group, "Current device is %d.\r\n", deviceId);
		return -1;
	}

	set_callback(NULL);
	if(qapi_WLAN_P2P_Stop_Find(deviceId) != 0)
	{
		QCLI_Printf(qcli_p2p_group, "P2P stop command did not execute properly\r\n");
		return -1;
	}
	return QCLI_STATUS_SUCCESS_E;
}

int32_t p2p_passphrase(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	qapi_WLAN_Auth_Mode_e authMode = QAPI_WLAN_AUTH_WPA2_PSK_E;
	qapi_WLAN_Crypt_Type_e encrType = QAPI_WLAN_CRYPT_AES_CRYPT_E;
	qapi_WLAN_P2P_Go_Params_t ssidParams;

	uint32_t deviceId = get_active_device();
	if(deviceId != 0)
	{
		QCLI_Printf(qcli_p2p_group, "Switch to device 0 to use p2p mode.\r\n");
		QCLI_Printf(qcli_p2p_group, "Current device is %d.\r\n", deviceId);
		return -1;
	}

	set_callback(NULL);
    memset(&ssidParams, 0, sizeof(qapi_WLAN_P2P_Go_Params_t));

	if(0 != qapi_WLAN_Set_Param(deviceId,
                                __QAPI_WLAN_PARAM_GROUP_WIRELESS_SECURITY,
                                __QAPI_WLAN_PARAM_GROUP_SECURITY_ENCRYPTION_TYPE,
                                (void *) &encrType,
                                sizeof(qapi_WLAN_Crypt_Type_e),
                                FALSE))
	{
		return -1;
	}

	if (0 != qapi_WLAN_Set_Param(deviceId,
                                 __QAPI_WLAN_PARAM_GROUP_WIRELESS_SECURITY,
                                 __QAPI_WLAN_PARAM_GROUP_SECURITY_AUTH_MODE,
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

	if ( 0 != qapi_WLAN_Set_Param(deviceId,
                                   __QAPI_WLAN_PARAM_GROUP_P2P,
                                   __QAPI_WLAN_PARAM_GROUP_P2P_GO_PARAMS,
                                   &ssidParams,	sizeof(ssidParams), FALSE))
	{
		QCLI_Printf(qcli_p2p_group, "P2P command did not execute properly\r\n");
		return -1;
	}
	return QCLI_STATUS_SUCCESS_E;
}

int32_t p2p_set(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	uint32_t deviceId = 0, len = 0;
    qapi_WLAN_P2P_Set_Cmd_t p2p_set_params;

	deviceId = get_active_device();
	if(deviceId != 0)
	{
		QCLI_Printf(qcli_p2p_group, "Switch to device 0 to use p2p mode.\r\n");
		QCLI_Printf(qcli_p2p_group, "Current device is %d.\r\n", deviceId);
		return -1;
	}

	set_callback(NULL);
	memset(&p2p_set_params, 0, sizeof(qapi_WLAN_P2P_Set_Cmd_t));

	if(APP_STRCMP(Parameter_List[0].String_Value,"p2pmode") == 0)
	{
	    if (Parameter_Count < 2)
	    {
			QCLI_Printf(qcli_p2p_group, "Incorrect parameters\r\n", deviceId);
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
			QCLI_Printf(qcli_p2p_group, "Input can be \"p2pdev/p2pclient/p2pgo\"");
			return -1;
		}

		len = sizeof(p2p_set_params.val.mode);
		QCLI_Printf(qcli_p2p_group, "p2p mode :%x, Config Id %x\r\n",p2p_set_params.val.mode.p2pmode,p2p_set_params.config_Id);
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
			QCLI_Printf(qcli_p2p_group, "PostFix string %s, Len %d\r\n",
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
            QCLI_Printf(qcli_p2p_group, "Incorrect parameters\r\n", deviceId);
            return -1;
        }

		p2p_set_params.config_Id = __QAPI_WLAN_PARAM_GROUP_P2P_LISTEN_CHANNEL;
		p2p_set_params.val.listen_Channel.reg_Class = Parameter_List[1].Integer_Value;
		p2p_set_params.val.listen_Channel.channel = Parameter_List[2].Integer_Value;
		len = sizeof(p2p_set_params.val.cck_Rates);
    }

    else if (APP_STRCMP(Parameter_List[0].String_Value, "devname") == 0) {
        if (Parameter_Count < 2) {
             QCLI_Printf(qcli_p2p_group, "Incorrect parameters\r\n");
            return -1;
        }

		len = strlen((char *)Parameter_List[1].String_Value);
		if (len > __QAPI_WLAN_P2P_WPS_MAX_DEVNAME_LEN) {
             QCLI_Printf(qcli_p2p_group, "Device name exceeds the allowed length\r\n");
            return -1;
        }

		p2p_set_params.config_Id = __QAPI_WLAN_PARAM_GROUP_P2P_DEV_NAME;
        memset(p2p_set_params.val.device_Name.dev_Name, 0, __QAPI_WLAN_P2P_WPS_MAX_DEVNAME_LEN + 1);
        memcpy(p2p_set_params.val.device_Name.dev_Name, (char *)Parameter_List[1].String_Value, len);
        p2p_set_params.val.device_Name.dev_Name_Len = len;
    }

	else
	{
		QCLI_Printf(qcli_p2p_group, "Incorrect parameters\r\n", deviceId);
		return -1;
	}  

	if (0 != qapi_WLAN_Set_Param (deviceId, __QAPI_WLAN_PARAM_GROUP_P2P,
                                  p2p_set_params.config_Id,	&p2p_set_params.val,
                                  len, FALSE))
	{
		QCLI_Printf(qcli_p2p_group, "P2P set command did not execute properly\r\n");
		return -1;
	}
	return QCLI_STATUS_SUCCESS_E;
}

#endif /* ENABLE_P2P_MODE */



