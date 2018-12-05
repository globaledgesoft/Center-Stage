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

#ifndef __QC_API_P2P_H
#define __QC_API_P2P_H

/*-------------------------------------------------------------------------
 *  Include Files
 *-----------------------------------------------------------------------*/
#include "qapi_wlan.h"

#define QC_API_DEV_NUM    2

typedef uint32_t  boolean_t;
//boolean_t p2pMode = FALSE;
extern boolean_t p2pMode;
//#if ENABLE_P2P_MODE
#define P2P_STANDARD_TIMEOUT (300)
char p2p_wps_pin[__QAPI_WLAN_WPS_PIN_LEN];
//boolean_t autogo_newpp = FALSE;
//extern boolean_t autogo_newpp;
uint8_t p2p_join_mac_addr[__QAPI_WLAN_MAC_LEN];

extern uint8_t wps_flag;
extern volatile uint8_t wifi_state[QC_API_DEV_NUM];
extern uint8_t dev_stat[QC_API_DEV_NUM];
/* Used to remember persistent information while waiting for GO_NEG complete event */
//uint8_t p2p_persistent_go = QAPI_WLAN_P2P_NON_PERSISTENT_E;
extern uint8_t p2p_persistent_go;
extern uint8_t p2p_persistent_done;
extern uint8_t p2p_cancel_enable;
extern uint8_t p2p_session_in_progress;
extern uint8_t invitation_index;
extern uint8_t p2p_join_session_active;
extern uint8_t p2p_intent;
extern uint8_t inv_response_evt_index;
extern uint32_t set_channel_p2p;

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
//#endif /* ENABLE_P2P_MODE */

#define   P2P_CONNECT_OPERATION     1
#define   P2P_PROVISION_OPERATION   2
#define   P2P_AUTH_OPERATION        3
#define   P2P_INVITE_OPERATION      4

int32_t qc_api_p2p_enable(void);
int32_t qc_api_p2p_disable(void);
int32_t qc_api_wlan_set_config(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
int32_t qc_api_p2p_connect(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
int32_t qc_api_p2p_provision(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
int32_t qc_api_p2p_listen(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
int32_t qc_api_p2p_cancel(void);
int32_t qc_api_p2p_join(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
int32_t qc_api_p2p_auth(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
int32_t qc_api_p2p_auto_go(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
int32_t qc_api_p2p_invite_auth(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
int32_t qc_api_wlan_get_node_list(void);
int32_t qc_api_wlan_get_network_list(void);
int32_t qc_api_wlan_set_oops_params(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
int32_t qc_api_wlan_set_noa_params(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
int32_t qc_api_wlan_set_operating_class(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
int32_t qc_api_p2p_set(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
int32_t qc_api_p2p_stop_find(void);
int32_t qc_api_wlan_passphrase(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
int32_t qc_api_p2p_set(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
int32_t qc_api_p2p_find(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
int32_t qc_api_p2p_set_config(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
int32_t qc_api_p2p_set_noa_params(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
int32_t qc_api_p2p_set_oops_params(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
int32_t qc_api_p2p_set_operating_class(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
int32_t qc_api_p2p_get_node_list(void);
int32_t qc_api_p2p_get_network_list(void);
int32_t qc_api_p2p_passphrase(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
int32_t qc_api_wifi_set_app_ie(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);

#endif
