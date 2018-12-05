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
#include <string.h>
#include <stdlib.h>
#include <qcli.h>
#include <qcli_api.h>
#include <qapi_wlan.h>
#include "wifi_demo.h"

QCLI_Group_Handle_t qcli_wlan_group;              /* Handle for our QCLI Command Group. */
QCLI_Group_Handle_t qcli_p2p_group;                                                    

QCLI_Command_Status_t queryVersion(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t enableWlan(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t disableWlan(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t reset(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t info(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t setDevice(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t setScanParameters(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t scan(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t customScan(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t connect(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t getRegulatoryDomain(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t setCountryCode(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t getCountryCode(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t getLastError(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t setMacAddress(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t setWpaPassphrase(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t setWpaParams(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t setWepKeyPair(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t setWepKey(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t adhocConnect(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t adhocWepConnect(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t wpsPinSetup(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t wpsPushSetup(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t disconnect(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t setTxRate(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t getTxRate(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t setTxPower(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t setPowerMode(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t setChannel(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t setStaListenInterval(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t setPhyMode(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t set11nHTCap(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t getRssi(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t setRoamThreshold(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t roaming(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t suspendEnable(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t suspend(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t setPowerPolicy(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t setAggregationParameters(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t setAggregationRxReorderParameters(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t setOperatingMode(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t setAPBeaconInterval(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t setAPDtimPeriod(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t setAPPowerSaveBufferParameters(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t setAPInactivityPeriod(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t setAPUapsd(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);  
QCLI_Command_Status_t setSTAUapsd(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);  
QCLI_Command_Status_t setMaxSPLen(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);  
QCLI_Command_Status_t rxProbeReq(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t getStats(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t sendRawFrame(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t enableLpl(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t enableGtx(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t enablePreferredNetworkOffload(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);  
QCLI_Command_Status_t setPnoNetworkProfile(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t offloadTcpSessionKeepAlive(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);  
QCLI_Command_Status_t enableTcpKeepaliveOffload(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t setSTAMacKeepAliveTimeout(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t promiscHandler(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List); 
QCLI_Command_Status_t set11v(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t setWnmSleepPeriod(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t setAPBssMaxIdlePeriod(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t setWnmSleepResponse(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t enableWow(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t enablePktFilter(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t addFilterPattern(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t deleteFilterPattern(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t changeDefaultFilterAction(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t dbglogHandler(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t pktlogHandler(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t regqueryHandler(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t memqueryHandler(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t driverassertHandler(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t setEventFilter(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t channelSwitch(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);  
QCLI_Command_Status_t disableChannel(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);  
QCLI_Command_Status_t arpOffload(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t nsOffload(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t setApplicationIe(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t setStaBmissConfig(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t getSSID(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t getPhyMode(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t getWiFiPowerMode(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t getMacAddress(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t getOperatingMode(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t getChannel(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t getWepKeyPair(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t getDefaultWepKeyIndex(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t getRegDomainChannelList(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t setWpaCertParams(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t setAntDiv(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t getAntDivStatus(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t setAntenna(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);


#if ENABLE_P2P_MODE
QCLI_Command_Status_t p2pEnable(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t p2pDisable(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t p2pSetConfig(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t p2pConnect(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t p2pFind(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t p2pProvision(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t p2pListen(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t p2pCancel(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t p2pJoin(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t p2pAuth(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t p2pAutoGO(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t p2pInvite(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t p2pGetNodeList(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t p2pGetNetworkList(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t p2pInviteAuth(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t p2pSetNOAParams(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t p2pSetOPPSParams(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t p2pSetOperatingClass(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t p2pSet(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t p2pStopFind(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t p2pPassphrase(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
#endif /* ENABLE_P2P_MODE */


const QCLI_Command_t wlan_cmd_list[] =
{
   // cmd_function    start_thread          cmd_string               usage_string                   description
   { queryVersion,         false,          "QueryVersion",                 "",                      "Query firmware and host driver revision"   },
   { enableWlan,           false,          "Enable",                       "",                      "Enables WLAN module"   },
   { disableWlan,          false,          "Disable",                      "",                      "Disables WLAN module"   },
   { reset,                false,          "Reset",                        "",                      "Reset WLAN"   },
   { info,                false,           "Info",                         "",                      "Info on WLAN state"   },     
   { setDevice,            false,          "SetDevice",                    "<device = 0:AP|GO, 1:STA|P2P client",    "Set the active device"   },
   { scan,                 false,          "Scan",                         "<mode = 0: blocking| 1: non-blocking| 2:non-buffering> [ssid]",    "Scan for networks, using blocking/non-blocking/non-buffering modes. If ssid is provided, scan for specific ssid only."   },
   { setScanParameters,    false,          "SetScanParameters",            "<max_active_chan_dwell_time_ms> <passive_chan_dwell_time_ms> [<fg_start_period_in_sec> <fg_end_period_in_sec> <bg_period_in_sec> <short_scan_ratio> <scan_ctrl_flags>  <min_active_chan_dwell_time_ms> <max_active_scan_per_ssid> <max_dfs_chan_active_time_in_ms>]",    "Sets scan parameters"   },
   { customScan,           false,          "CustomScan",                   "<force_fg_scan> <home_dwell_time_in_ms> <force_scan_interval_in_ms> <num_channels> [<channel> <channel>... upto num_channels]",    "Customize scan to specific channels"   },
   { connect,              false,          "Connect",                      "<ssid> [bssid]",                 "Connect to a given ssid and given bssid(bssid option applicable to STA mode only. if AP mode connect command shouldnt take BSSID)"   },
   { getRegulatoryDomain,  false,          "GetRegulatoryDomain",          "",                       "Query regulatory domain"   },
   { setCountryCode,       false,          "SetCountryCode",               "<country_code_string>",  "Set country code"   },
   { getCountryCode,       false,          "GetCountryCode",               "",                       "Query country code from OTP"   },
   { getLastError,         false,          "GetLastError",                 "",                       "Query last error in wlan driver"   },
   { setMacAddress,        false,          "SetMacAddress",                "<mac_address>",          "Programs MAC address in OTP"   },
   { setWpaPassphrase,     false,          "SetWpaPassphrase",             "<passphrase>",           "Set WPA passphrase"   },
   { setWpaParams,         false,          "SetWpaParameters",             "<version=WPA|WPA2|WPACERT|WPA2CERT> <ucipher> <mcipher>",    "Set WPA specific parameters"   },
   { setWepKeyPair,        false,          "SetWepKeyPair",                "<keyindex> <key>",       "Set a WEP key-keyindex pair"   },
   { setWepKey, 		   false,		   "SetWepKey", 				   "<keyindex>",			 "Set default WEP key index"   },
   { adhocConnect,         false,          "AdhocConnect",                 "<ssid> <channel(optional)>",       "Connect to unsecured Adhoc network on specific channel(optional)"   },
   { adhocWepConnect,      false,          "AdhocWepConnect",              "<ssid> <def_keyix> <channel(optional)>", "Connect to WEP secured adhoc network on specific channel(optional)"   },
   { wpsPinSetup,          false,          "WpsPin",                       "<connectFlag> <pin> [<ssid> <mac> <channel>]",    "Setup and start a WPS connection using the Pin method"   },
   { wpsPushSetup,         false,          "WpsPush",                      "<connectFlag> [<ssid> <mac> <channel>]",    "Setup and start a WPS connection using the Push method"   },
   { disconnect,           false,          "Disconnect",                   "",                        "Disconnect from AP or peer"   },
   { setTxRate,            false,          "SetTxRate",                    "['mcs'] <rate_in_Mbps|mcsIndex>", " SetTxRate <rate_in_Mbps: 1|2|5|6|9|11|12|18|24|36|48|54>  OR  SetTxRate mcs <mcsIndex: 0|1|2|3|4|5|6|7>"   },
   { getTxRate,            false,          "GetTxRate",                    "",                        "Get rate of the last transmitted packet"   },
   { setTxPower,           false,          "SetTxPower",                   "<txPower = 0-63>",                  "Set the transmit power level. Range 0-63. Default = 63 (max)"   },
   { setPowerMode,         false,          "SetPowerMode",                 "<mode = 0: Max performance, 1: Power Save>",    "Set the device power mode."   },
   { setChannel,           false,          "SetChannel",                   "<channel>",               "Set a channel hint."   },
   { setStaListenInterval, false,          "SetStaListenInterval",         "<interval_in_TUs>",       "Set the listen interval in TUs. Range: 15-5000. Default 100."   },
   { setPhyMode,           false,          "SetPhyMode",                   "<mode = a|b|g|ag|gonly>",          "Set the wireless mode"   },
   { set11nHTCap,          false,          "Set11nHTCap",                  "<HTCap = disable|ht20>",      "Set 11n HT parameter"   },
   { getRssi,              false,          "GetRssi",                      "",                        "Get link quality indicator (SNR in dB) between AP and STA."   },
   { setRoamThreshold,     false,          "SetRoamThreshold",             "<lower_threshold> <upper_threshold> <weight> <poll-time>",                        "Configure roaming parameters"   },
   { roaming,              false,          "Roaming",                      "<1: enable |0: disable>",  "Enable/Disable roaming"   },
   { suspendEnable,        false,          "EnableSuspend",                "",                        "Enable WLAN Suspend. Should be done before connecting to a network."   },
   { suspend,              false,          "Suspend",                      "<time_in_ms>",            "Suspends the WLAN"   },
   { setPowerPolicy,       false,          "SetPowerPolicy",               "<idle_time_in_ms> <pspollnum> <dtim_policy= 1:Ignore, 2:Normal, 3:Stick, 4:Auto> <txwakeuppolicy= 1:wake host, 2: do not wake> <numTxToWakeup> <PsFailEventPolicy= 1:Send fail event 2:Ignore event>",                        "Set power management parameters"   },
   { setAggregationParameters, false,      "SetAggregationParameters",     "<tx_tid_mask> <rx_tid_mask>",    "Set aggregation on RX or TX or both. Enabled via TID bit mask (0x00-0xff)"   },
   { setAggregationRxReorderParameters, false, "SetAggrx",                 "<buffer size: frame count> <reorder timeout: ms> <session timeout: ms> <aggrx enable> <session timer enable>", "Set aggregation rx reorder parameters."   },
   { setOperatingMode,     false,          "SetOperatingMode",             "<ap|station> [<hidden|0> <wps|0>]",  "Set the operating mode to either Soft-AP or STA. Hidden and wps parameters only apply to AP mode."   },
   { setAPBeaconInterval,  false,          "SetAPBeaconInterval",          "<beacon_interval_in_ms>", "Set the beacon interval in ms."   },
   { setAPDtimPeriod,      false,          "SetAPDtimPeriod",              "<dtim_period>",           "Set the DTIM period"   },
   { setAPPowerSaveBufferParameters, false,"SetAPPowerSaveBufferParameters","<1: enable| 0: disable>  <buffer count>",    "Configure AP power save buffers"   },
   { setAPInactivityPeriod, false,         "SetAPInactivityPeriod",        "<inactivity_period_in_mins>",  "Set inactivity period "   },    
   { setAPUapsd,            false,         "setAPUapsd",                  "<0/1 enable/disable>",       "Enable/disable AP UAPSD "   },  
   { setSTAUapsd,           false,         "setSTAUapsd",                 "<AC_mask>",                  "Enable/disable corresponding AC UAPSD capability"   },   
   { setMaxSPLen,           false,         "setMaxSPLen",                 "<maxSP_Len>",                "Set the Max SP Len in QoS Info field"   },  
   { rxProbeReq,           false,          "ForwardProbeReqToHost",        "<1: forward |0: skip forward to host>",    "Forward probe requests to Host"   },
   { getStats,             false,          "GetStats",                     "<resetCounters= 0: continue counters ,1: reset counters>",        "Get  firmware statistics"   },
   { sendRawFrame,         false,          "SendRawFrame",                 "<rate_index> <num_tries = 1-14> <num_bytes = 0-1400> <channel = 1-11> <type = 0:Beacon, 1: QoS Data, 2: 4-addr data> [addr1 [addr2 [addr3 [addr4]]]]",        "Send a raw packet w"   },
   { enableLpl,            false,          "EnableLPL",                    "<1: enable| 0: disable>",    "Enable Low Power Listen mode"   },
   { enableGtx,            false,          "EnableGTX",                    "<1: enable| 0: disable>",    "Enable Green TX mode"   },
   { enablePreferredNetworkOffload, false, "EnablePreferredNetworkOffload","<1: enable| 0: disable> [<max_profiles> <fast_scan_interval> <fast_scan_duration> <slow_scan_interval>]",    "Enable Preferred Network Offload. Parameters after enable/disable are valid only for enable."   },
   { setPnoNetworkProfile, false,          "SetPnoNetworkProfile",         "<index> <ssid> <mode=open|wep|wpa|wpa2|wpacert|wpa2cert> [wpa_cipher]",    "Register a PNO network profile with the WLAN"   },
   { offloadTcpSessionKeepAlive,false,     "OffloadTcpSessionKeepAlive",   "<tcp_src_port> <tcp_dst_port> <src_ip> <dst_ip> <dest_mac_addr> <seq_num> <ack_seq> <ip protocol = 4 for IPv4|6 for IPv6>",    "Register a session with TCP KeepAlive offload manager"   },
   { enableTcpKeepaliveOffload, false,     "EnableTcpKeepaliveOffload",    "<1: enable| 0: disable> [<keepalive_interval> <keepalive_ack_threshold>]",    "Enable/Disable TCP Keepalive offload to WLAN"   },
   { setSTAMacKeepAliveTimeout, false,     "SetStaMacKeepAliveTimeout",    "<timeout_in_secs>",          "Set keep alive time for STA, value of 0 disables the timer"   },  
   { promiscHandler,            false,     "SetPromiscuous",               "<enable|filter> [config|reset]",    "Enable/disable promoscuous mode and configure, reset filters."   },     
   { set11v,            false,             "Enable80211v",                 "<1: enable| 0: disable>", "Enable/Disable 802.11v features"   },
   { setWnmSleepPeriod,    false,          "SetWnmSleepPeriod",            "<action = 1: enter| 0: exit> <period_in_DTIM_intervals>",     "Enter/Exit WNM sleep mode"     },
   { setAPBssMaxIdlePeriod, false,         "SetAPBssMaxIdlePeriod",        "<period_in_1000TUs>",    "Configure BSS max idle period, applicable only for soft-AP mode."   },
   { setWnmSleepResponse,   false,         "SetWnmSleepResponse",          "<0:Enter/Exit accepted|1:Exit accepted,GTK/IGTK update required|3:Denied temporarily|4:Denied|5:Denied due to other WNM services in use>", "Set AP's response to WNM sleep request"},
   
   { enableWow,             false,         "EnableWow",                    "<1: enable | 0: disable>", "Enable/Disable WoW"},
   { enablePktFilter,       false,         "EnablePktFilter",              "<1: Enable | 0: disable>", "Enable/Disable Packet Filter"},
   { addFilterPattern,      false,         "AddFilterPattern",             "<index [1-8]> <action = 1:Reject|2:Accept|4:Defer> <wow_flag = 1:Wow filter | 0: not Wow Filter> <priority [1-255]> <headerType> <offset> <pattern_size> <pattern_mask [hex]> <pattern>", "Add Packet filter pattern"},
   { deleteFilterPattern,   false,         "DeleteFilterPattern",          "<index> <header_type>", "Delete Pattern"},
   { changeDefaultFilterAction,		false, "ChangeDefaultFilterAction",	   "<action = 1: Reject|2: Accept|4:Defer> <wow_flag = 1:enable 0:disable> <header_type>", " Change Default packet filter action"},
    
   { dbglogHandler,        false,          "Dbglog",                       "cmd1 - enable <1/0>, cmd2 - config <debug port> <report enable> <report size>, cmd3 -  loglevel <moduleid> <loglevel>", "Enable, Configure and set loglevels for DBGLOGS in KF"},
   { pktlogHandler,        false,          "Pktlog",                       "cmd1 - enable <1/0> <no of buffers>(max 10), cmd2 - start <evt mask> <options mask> <trigger threshold> <trigger Intvl> <trigger_tail_count>", "Enable and start Packet logging in KF"},
   { regqueryHandler,        false,          "Regquery",                       "cmd1 - >read/write/dump/rmw> , cmd2 - <address[HEX]> , cmd3 - <size[DEC]|value[HEX]> <mask[HEX]>","target driver regester read/write/dump/rmw, debug only"},
   { memqueryHandler,        false,          "Memquery",                       "cmd1 - >read/write/dump/rmw> , cmd2 - <address[HEX]> , cmd3 - <size[DEC]|value[HEX]> <mask[HEX]>","Quartz  regester read/write/dump/rmw, debug only"},
   { driverassertHandler,        false,      "SetDriverAssert",      "",                       "force set target driver assert, debug only"},
   { setEventFilter,       false,          "SetEventFilter",               "Enable/Disable events from WLAN FW <1:enable | 0:disable> [<eventId> <eventId>...]" , "Enable/Disable events from WLAN FW"},     
   { channelSwitch,        false,          "ChannelSwitch",                "<channel> <tbtt_count>",            "Switch the Channel"   },
   { disableChannel,        false,          "DisableChannel",                "<channel number> ",            "Channel to disable"   },
   { arpOffload,        false,          "ARPOffload",                "<1:enable/0:disable> <targetip> <mac>",  "ARP Offload"},
   { nsOffload,        false,          "nsOffload",                "<1:enable/0:disable> <target_global_ip> <target_link_ip> <mac> [solicitationip]", "NS Offload"}, 
   { setApplicationIe, false, "setApplicationIe", "<0:beacon/1:probe request/2:probe response> <IE starting with dd>",  "Set application specified IE in specified management frame. Every input character is a nibble which means every 2 character is a byte, two characters are converted into a hex number before putting it in the frame. The length of application specified IE should be multiple of 2. if user has single digit value he need to prepend with 0 for ex: 0x5 should be 0x05. To remove IE, input only 'dd'"   },
   { setStaBmissConfig,        false,          "setStaBmissConfig",                "<bmiss_Time_in_ms> <num_Beacons>",            "Set the Beacon Miss Timeout Config Value, one of the value is set which needs to be configured and the other set to 0. Both value input is mandatory"   },
   { getSSID,          false,         "GetSSID",             "",    "Get SSID." },
   { getPhyMode,       false,         "GetPhyMode",          "",    "Get wireless mode." },
   { getWiFiPowerMode, false,         "GetPowerMode"         "",    "Get device power mode." },
   { getMacAddress,    false,         "GetMacAddress"        "",    "Get device mac address." },
   { getOperatingMode, false,         "GetOperatingMode"     "",    "Get device operating mode" },
   { getChannel,       false,         "GetChannel"           "",    "Get operating channel for the device." },
   { getWepKeyPair,    false,         "GetWepKeyPair",       "<keyindex>",   "Get a WEP key at given index"   },
   { getDefaultWepKeyIndex,   false,  "GetDefaultWepKeyIndex",           "",             "Get default WEP key index"   },
   { getRegDomainChannelList,        false,          "getRegDomainChannelList",                "getRegDomainChannelList",            "Get the channel list for the current regulatory setting"   },
   { setWpaCertParams,         false,          "SetWpaCertParameters",             "<method=TLS|TTLS-MSCHAPV2|PEAP-MSCHAPV2|TTLS-MD5> <id> <username> <password> <dbglevel>",    "Set WPA enterprise specific parameters"   },
   { setAntDiv,       false,         "SetAntDiv",        "<1: enable| 0: disable> <1:0 tx follow rx or not> <ant div mode>[<the number of packets or time interval to diversity>]",   "Enable/disable Antenna Diversity and set related to parameters"   },  
   { getAntDivStatus , false,         "getAntDivStatus"         "",    "Get the information of Antenna Diversity" },
   { setAntenna,      false,         "SetAnt",           "<antenna number>",   "Set antenna, the antenna number depends on the total antenna count on hw board."   },  
   //{ ,              false,          "",                    "",    ""   },     
};

const QCLI_Command_Group_t wlan_cmd_group =
{
    "WLAN",
    (sizeof(wlan_cmd_list) / sizeof(wlan_cmd_list[0])),
    wlan_cmd_list
};


#if ENABLE_P2P_MODE
const QCLI_Command_t p2p_cmd_list[] =
{
   // cmd_function    start_thread   cmd_string         usage_string         description
   { p2pEnable,            false,    "On",              "",                  "Enable P2P"   },
   { p2pDisable,           false,    "Off",             "",                  "Disable P2P"   },
   { p2pSetConfig,         false,    "SetConfig",       "<GO_intent> <listen channel> <operating channel> <country> <node_timeout>", "Disable/Enable P2P"   },
   { p2pConnect,           false,    "Connect",         "<peer_dev_mac> <wps_method = push|display|keypad> [WPS pin if keypad] [persistent]",      "Initiate connection request with a given peer MAC address using given WPS configuration method."   },
   { p2pFind,              false,    "Find",            "<channel_options = 1|2|3> <timeoutInSecs>",   "Initiates search for P2P peers. Channel_options = { 1: Scan all the channels from regulatory domain channel list,  2: Scan only the social channels (default), 3: Continue channel scan from the last scanned channel index}. Default value for timeoutInSecs = 60. When the timeout period expires, the find operation is stopped. "   },
   { p2pProvision,         false,    "Provision",       "<peer_dev_mac> <wps_method = push|display|keypad>",   "Provision the WPS configuration method between the DUT and the peer."   },
   { p2pListen,            false,    "Listen",          "<timeout>",               "Initiate P2P listen process.When the timeout period expires, the listen operation is stopped. Default value is 300 seconds."   },
   { p2pCancel,            false,    "Cancel",          "",                  "Cancels ongoing P2P operation"   },
   { p2pJoin,              false,    "Join",            "<GO_intf_mac> <wps_method = push|display|keypad> [WPS pin if keypad] [persistent]",   "Join a P2P client to an existing P2P Group Owner."   },
   { p2pAuth,              false,    "Auth",            "<peer_dev_mac> <wps_method = push|display|keypad|deauth> [WPS pin if keypad] [persistent]",   "Authenticate/Reject a connection request from a given peer MAC address using the given WPS configuration method."   },
   { p2pAutoGO,            false,    "AutoGO",          "[persistent]",      "Start P2P device in Autonomous Group Owner mode."   },
   { p2pInviteAuth,        false,    "Invite",          "<ssid> <peer_dev_mac> <wps_method= push|display|keypad> [persistent]",   "Invite a peer, from persistent database, to connect"   },
   { p2pGetNodeList,       false,    "ListNodes",       "",                  "Display the results of P2P find operation."   },
   { p2pGetNetworkList,    false,    "ListNetworks",    "",                  "Display the list of persistent P2P connections that are saved in the persistent media."   },
   { p2pSetOPPSParams,	   false,	 "SetOPPSParams",	"<ctwin> <enable>",  "Set Opportunistics Power Save parameters."   },
   { p2pSetNOAParams,      false,    "SetNOAParams",    "<count> <start_offset_in_usec> <duration_in_usec> <interval_in_usec> ",      "Set NOA parameters."   },
   { p2pSetOperatingClass, false,    "SetOpClass",      "<GO_intent> <oper_reg_class> <oper_reg_channel>",      "Set Operating class parameters."   },
   { p2pSet,               false,    "Set",             "p2pmode <p2pdev|p2pclient|p2pgo> | postfix <postfix_string> | intrabss <flag> | gointent <Intent> | cckrates <1:Enable|0:Disable> >", "Set P2P parameters" },
   { p2pStopFind,          false,    "StopFind",        "",                  "Stop P2P operation" },
   { p2pPassphrase,        false,    "SetPassphrase",   "<passphrase> <SSID>",  "Set P2P passphrase" },
};


const QCLI_Command_Group_t p2p_cmd_group =
{
    "P2P",
    (sizeof(p2p_cmd_list) / sizeof(p2p_cmd_list[0])),
    p2p_cmd_list
};

#endif /* ENABLE_P2P_MODE */

/* This function is used to register the wlan Command Group with    */
/* QCLI.                                                             */
void Initialize_WIFI_Demo(void)
{
   /* Attempt to reqister the Command Groups with the qcli framework.*/
   qcli_wlan_group = QCLI_Register_Command_Group(NULL, &wlan_cmd_group);
   if(qcli_wlan_group)
   {
      QCLI_Printf(qcli_wlan_group, "WLAN Registered \n");
   }
   
#if ENABLE_P2P_MODE
   qcli_p2p_group = QCLI_Register_Command_Group(qcli_wlan_group, &p2p_cmd_group);
    if(qcli_p2p_group)
   {
      QCLI_Printf(qcli_p2p_group, "WLAN P2P Registered \n");
   }

#endif /* ENABLE_P2P_MODE */   
}

extern int32_t get_version();
QCLI_Command_Status_t queryVersion(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   //QCLI_Printf(qcli_wlan_group, "WLAN cmd %d %s\n", Parameter_Count, Parameter_List[0].String_Value);
   if (0 == get_version())
        return QCLI_STATUS_SUCCESS_E;
  return QCLI_STATUS_ERROR_E;
}
extern int32_t enable_wlan();
QCLI_Command_Status_t enableWlan(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
  if (0 == enable_wlan())
  {
      return QCLI_STATUS_SUCCESS_E;
  }

  QCLI_Printf(qcli_wlan_group, "ERROR: Operation failed\n");
  return QCLI_STATUS_ERROR_E;
}

extern int disable_wlan();
QCLI_Command_Status_t disableWlan(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	if (0 == disable_wlan())
	{
		return QCLI_STATUS_SUCCESS_E;
	}

	QCLI_Printf(qcli_wlan_group, "ERROR: Operation failed\n");
	return QCLI_STATUS_ERROR_E;
}


QCLI_Command_Status_t reset(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
  QCLI_Printf(qcli_wlan_group, "Deprecated command. Use WLAN Disable + WLAN Enable separately\n");   
  return QCLI_STATUS_ERROR_E;
}

extern void print_wlan_info();
QCLI_Command_Status_t info(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
  print_wlan_info();
  return QCLI_STATUS_SUCCESS_E;  
}

extern int32_t set_active_deviceid(uint16_t deviceId);
QCLI_Command_Status_t setDevice(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
  if( Parameter_Count < 1 || !Parameter_List || !Parameter_List[0].Integer_Is_Valid){
     return QCLI_STATUS_USAGE_E;
  }
  if (0 == set_active_deviceid(Parameter_List[0].Integer_Value))
     return QCLI_STATUS_SUCCESS_E;
  return QCLI_STATUS_ERROR_E;
}

extern int32_t set_scan_parameters(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t setScanParameters(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
  if( Parameter_Count != 10 || !Parameter_List) {
     return QCLI_STATUS_USAGE_E;
  }

  if (0 == set_scan_parameters(Parameter_Count, Parameter_List))
     return QCLI_STATUS_SUCCESS_E;
  return QCLI_STATUS_ERROR_E;  
}

extern int32_t wlan_scan(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t scan(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
  if (0 ==  wlan_scan(Parameter_Count, Parameter_List))
         return QCLI_STATUS_SUCCESS_E;
  return QCLI_STATUS_ERROR_E;
}

extern int32_t start_scan(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t customScan(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
  if (Parameter_Count < 4 || !Parameter_List) {
	 return QCLI_STATUS_USAGE_E;
  }

  if (0 ==  start_scan(Parameter_Count, Parameter_List))
         return QCLI_STATUS_SUCCESS_E;
  return QCLI_STATUS_ERROR_E;
}

int32_t connect_to_network(const char* ssid, const char* bssid);
QCLI_Command_Status_t connect(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
  char *bssid = NULL;
  
  if( Parameter_Count < 1 || !Parameter_List ){
      return QCLI_STATUS_USAGE_E;
  }
  
  if (Parameter_Count == 2)
     bssid =  (char *) Parameter_List[1].String_Value;
  
  if (0 ==  connect_to_network((char*) Parameter_List[0].String_Value, bssid)){
      return QCLI_STATUS_SUCCESS_E;
  }
  return QCLI_STATUS_ERROR_E;
}

extern int32_t get_reg_domain();
QCLI_Command_Status_t getRegulatoryDomain(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   if (0 == get_reg_domain())
     return QCLI_STATUS_SUCCESS_E;
  return QCLI_STATUS_ERROR_E;
}

extern int32_t set_country_code(char *country);
QCLI_Command_Status_t setCountryCode(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if( Parameter_Count != 1 || !Parameter_List || Parameter_List[0].Integer_Is_Valid) {
      return QCLI_STATUS_USAGE_E;
    }
    if (0 == set_country_code((char *) Parameter_List[0].String_Value))
      return QCLI_STATUS_SUCCESS_E;
    return QCLI_STATUS_ERROR_E;
}

extern int32_t get_country_code();
QCLI_Command_Status_t getCountryCode(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
  if (0 == get_country_code())
     return QCLI_STATUS_SUCCESS_E;
  return QCLI_STATUS_ERROR_E;
}

extern int32_t get_last_error();
QCLI_Command_Status_t getLastError(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
  if (0 == get_last_error())
     return QCLI_STATUS_SUCCESS_E;
  return QCLI_STATUS_ERROR_E;
}

extern int32_t set_mac(char *mac_addr);
QCLI_Command_Status_t setMacAddress(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List) 
{ 
    if( Parameter_Count < 1 || !Parameter_List || Parameter_List[0].Integer_Is_Valid){
      return QCLI_STATUS_USAGE_E;
  }
  if (0 == set_mac((char *)Parameter_List[0].String_Value))
     return QCLI_STATUS_SUCCESS_E;
  return QCLI_STATUS_ERROR_E;
}

extern int32_t set_passphrase(char* passphrase);
QCLI_Command_Status_t setWpaPassphrase(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List) 
{ 
  if( Parameter_Count < 1 || !Parameter_List){
      return QCLI_STATUS_USAGE_E;
  }
  if (0 == set_passphrase( (char *) Parameter_List[0].String_Value))
      return QCLI_STATUS_SUCCESS_E;
  return QCLI_STATUS_ERROR_E;
}

extern int32_t set_wpa(char *wpaVer, char *ucipher, char *mcipher);
QCLI_Command_Status_t setWpaParams(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)  
{ 
  if( Parameter_Count != 3 || !Parameter_List || Parameter_List[0].Integer_Is_Valid || Parameter_List[1].Integer_Is_Valid || Parameter_List[2].Integer_Is_Valid){
      return QCLI_STATUS_USAGE_E;
  }
  if (0 == set_wpa( (char *) Parameter_List[0].String_Value, (char *) Parameter_List[1].String_Value, (char *) Parameter_List[2].String_Value))
      return QCLI_STATUS_SUCCESS_E;
  return QCLI_STATUS_ERROR_E;
}

extern int32_t set_wpa_cert(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t setWpaCertParams(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if(Parameter_Count < 2 || !Parameter_List)
       return QCLI_STATUS_USAGE_E;
    if (0 == set_wpa_cert(Parameter_Count, Parameter_List))
        return QCLI_STATUS_SUCCESS_E;
    return QCLI_STATUS_ERROR_E;
}

extern int32_t set_wep_key(int32_t key_index, char* key_val);
QCLI_Command_Status_t setWepKeyPair(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List) 
{ 
  if( Parameter_Count != 2 || !Parameter_List || !Parameter_List[0].Integer_Is_Valid){
      return QCLI_STATUS_USAGE_E;
  }
  if (0 == set_wep_key(Parameter_List[0].Integer_Value, (char *) Parameter_List[1].String_Value))
      return QCLI_STATUS_SUCCESS_E;
  return QCLI_STATUS_ERROR_E;
}

extern int32_t set_wep_keyix(int32_t key_index);
QCLI_Command_Status_t setWepKey(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List) 
{ 
  if( Parameter_Count < 1 || !Parameter_List || !Parameter_List[0].Integer_Is_Valid){
      return QCLI_STATUS_USAGE_E;
  }
  if (0 == set_wep_keyix(Parameter_List[0].Integer_Value))
      return QCLI_STATUS_SUCCESS_E;
  return QCLI_STATUS_ERROR_E;
}

extern int32_t ad_hoc_connect_handler(char *ssid, char *sec_mode, int32_t key_index, int32_t channel);
QCLI_Command_Status_t adhocConnect(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List) 
{ 
  int32_t channel = 10;
  if( Parameter_Count < 1 || !Parameter_List){
      return QCLI_STATUS_USAGE_E;
  }
  if((Parameter_Count >1) && (Parameter_List[1].Integer_Is_Valid))
  		channel = Parameter_List[1].Integer_Value;
  	
  if (0 == ad_hoc_connect_handler( (char *)Parameter_List[0].String_Value, "open", 0, channel))
      return QCLI_STATUS_SUCCESS_E;
  return QCLI_STATUS_ERROR_E;
}

QCLI_Command_Status_t adhocWepConnect(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)  
{ 
  int32_t channel = 10;
  if( Parameter_Count < 2 || !Parameter_List || !Parameter_List[1].Integer_Is_Valid ){
      return QCLI_STATUS_USAGE_E;
  }
  
  if((Parameter_Count >2) && (Parameter_List[2].Integer_Is_Valid))
  		channel = Parameter_List[2].Integer_Value;
  
  if (0 == ad_hoc_connect_handler( (char *)Parameter_List[0].String_Value, "wep", Parameter_List[1].Integer_Value, channel))
      return QCLI_STATUS_SUCCESS_E;
  return QCLI_STATUS_ERROR_E;
}

extern int32_t wps_pin_setup(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t wpsPinSetup(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)  
{ 
    if( Parameter_Count < 2 || !Parameter_List || !Parameter_List[0].Integer_Is_Valid) {
        return QCLI_STATUS_USAGE_E;
    }
    if (Parameter_Count > 2) {
        if (Parameter_Count < 5) {
            return QCLI_STATUS_USAGE_E;
        }

    }
    if (0 == wps_pin_setup(Parameter_Count, Parameter_List))
    {
        return QCLI_STATUS_SUCCESS_E;
    }
    return QCLI_STATUS_ERROR_E;
}

int32_t wps_push_setup(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t wpsPushSetup(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)  
{ 
    if( Parameter_Count < 1 || !Parameter_List || !Parameter_List[0].Integer_Is_Valid) {
        return QCLI_STATUS_USAGE_E;
    }
    if ((Parameter_Count > 1) && (Parameter_Count < 4)) {
        return QCLI_STATUS_USAGE_E;
    }
    if((Parameter_List[0].Integer_Value != 0) && (Parameter_List[0].Integer_Value != 1)) {
        return QCLI_STATUS_USAGE_E;
    }

    if (0 == wps_push_setup(Parameter_Count, Parameter_List))
    {
        return QCLI_STATUS_SUCCESS_E;
    }
    return QCLI_STATUS_ERROR_E;
}

extern int32_t disconnect_from_network();
QCLI_Command_Status_t disconnect(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List) 
{ 
  if (0 == disconnect_from_network())
      return QCLI_STATUS_SUCCESS_E;
  return QCLI_STATUS_ERROR_E;
}

extern int32_t set_rate(int32_t isMcs, int32_t rateIndex);
QCLI_Command_Status_t setTxRate(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)  
{ 
  if (Parameter_Count == 0 ||  !Parameter_List)
     return QCLI_STATUS_USAGE_E;
  
  if (Parameter_Count == 1 && Parameter_List[0].Integer_Is_Valid)
  {
     if (0 == set_rate(0, Parameter_List[0].Integer_Value))
        return QCLI_STATUS_SUCCESS_E;
     return QCLI_STATUS_ERROR_E;
  }
  if( Parameter_Count == 2 && !Parameter_List[0].Integer_Is_Valid && 
        ( (strcmp((char *) Parameter_List[0].String_Value, "mcs")==0) || (strcmp( (char *) Parameter_List[0].String_Value, "MCS")==0) ) )
  {
     if (0 == set_rate(1, Parameter_List[1].Integer_Value))
        return QCLI_STATUS_SUCCESS_E;
     return QCLI_STATUS_ERROR_E;
  }  
  return QCLI_STATUS_USAGE_E;
}
extern int32_t get_rate();
QCLI_Command_Status_t getTxRate(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)  
{ 
  if (0 == get_rate()) //prints rateIndex 
     return QCLI_STATUS_SUCCESS_E;
  return QCLI_STATUS_ERROR_E;
}

extern int32_t set_tx_power(int32_t power_in_dBm);
QCLI_Command_Status_t setTxPower(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)  
{ 
  if( Parameter_Count != 1 || !Parameter_List || !Parameter_List[0].Integer_Is_Valid) {
    return QCLI_STATUS_USAGE_E;
  }
  if (0 == set_tx_power(Parameter_List[0].Integer_Value))
    return QCLI_STATUS_SUCCESS_E;
  return QCLI_STATUS_ERROR_E;
}

extern int32_t set_pwr_mode(int32_t power_mode);
QCLI_Command_Status_t setPowerMode(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)  
{ 
  if( Parameter_Count != 1 || !Parameter_List || !Parameter_List[0].Integer_Is_Valid) {
    return QCLI_STATUS_USAGE_E;
  }
  if (0 == set_pwr_mode(Parameter_List[0].Integer_Value))
    return QCLI_STATUS_SUCCESS_E;
  return QCLI_STATUS_ERROR_E;
}

extern int32_t set_channel_hint(int32_t channelNum);
QCLI_Command_Status_t setChannel(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)  
{ 
  if( Parameter_Count != 1 || !Parameter_List || !Parameter_List[0].Integer_Is_Valid) {
    return QCLI_STATUS_USAGE_E;
  }
  if (0 == set_channel_hint(Parameter_List[0].Integer_Value))
    return QCLI_STATUS_SUCCESS_E;
  return QCLI_STATUS_ERROR_E;
}

extern int32_t set_sta_listen_interval(int32_t listen_time);
QCLI_Command_Status_t setStaListenInterval(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)  
{ 
  if( Parameter_Count != 1 || !Parameter_List || !Parameter_List[0].Integer_Is_Valid) {
    return QCLI_STATUS_USAGE_E;
  }
  if (0 == set_sta_listen_interval(Parameter_List[0].Integer_Value))
    return QCLI_STATUS_SUCCESS_E;
  return QCLI_STATUS_ERROR_E;
}

extern int32_t set_phy_mode(char *wmode);
QCLI_Command_Status_t setPhyMode(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)  
{ 
  if( Parameter_Count != 1 || !Parameter_List || Parameter_List[0].Integer_Is_Valid) {
    return QCLI_STATUS_USAGE_E;
  }
  if (0 == set_phy_mode((char *) Parameter_List[0].String_Value))
    return QCLI_STATUS_SUCCESS_E;
  return QCLI_STATUS_ERROR_E;
}

extern int32_t set_11n_ht(char *htconfig);
QCLI_Command_Status_t set11nHTCap(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)  
{ 
  if( Parameter_Count != 1 || !Parameter_List || Parameter_List[0].Integer_Is_Valid) {
    return QCLI_STATUS_USAGE_E;
  }
  if (0 == set_11n_ht((char *) Parameter_List[0].String_Value))
    return QCLI_STATUS_SUCCESS_E;
  return QCLI_STATUS_ERROR_E;
}

extern int32_t get_rssi();
QCLI_Command_Status_t getRssi(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)  
{ 
  if( Parameter_Count != 0) {
    return QCLI_STATUS_USAGE_E;
  }
  if (0 == get_rssi())
    return QCLI_STATUS_SUCCESS_E;
  return QCLI_STATUS_ERROR_E;
}

extern int32_t  set_roam_thresh(int32_t lower_thresh, int32_t upper_thresh, int32_t wt, int32_t pollTime);
QCLI_Command_Status_t setRoamThreshold(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)  
{ 
  if( Parameter_Count != 4 || !Parameter_List || !Parameter_List[0].Integer_Is_Valid || 
     !Parameter_List[1].Integer_Is_Valid || !Parameter_List[2].Integer_Is_Valid || 
     !Parameter_List[3].Integer_Is_Valid) {
      return QCLI_STATUS_USAGE_E;
  }
  if (0 == set_roam_thresh(Parameter_List[0].Integer_Value, Parameter_List[1].Integer_Value, 
                            Parameter_List[2].Integer_Value, Parameter_List[3].Integer_Value))
      return QCLI_STATUS_SUCCESS_E;
  return QCLI_STATUS_ERROR_E;
}

extern int32_t roam(int32_t enable);
QCLI_Command_Status_t roaming(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)  
{ 
  if( Parameter_Count != 1 || !Parameter_List || !Parameter_List[0].Integer_Is_Valid) {
    return QCLI_STATUS_USAGE_E;
  }
  if (0 == roam(Parameter_List[0].Integer_Value))
    return QCLI_STATUS_SUCCESS_E;
  return QCLI_STATUS_ERROR_E;
}

extern int32_t enable_suspend ();
QCLI_Command_Status_t suspendEnable(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)  
{ 
  if( Parameter_Count > 0 || Parameter_List){
    return QCLI_STATUS_USAGE_E;
  }
  if (0 == enable_suspend())
    return QCLI_STATUS_SUCCESS_E;
  return QCLI_STATUS_ERROR_E;
}

extern int32_t dev_susp_start(int32_t susp_time);
QCLI_Command_Status_t suspend(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)  
{ 
  if( Parameter_Count != 1 || !Parameter_List || !Parameter_List[0].Integer_Is_Valid) {
    return QCLI_STATUS_USAGE_E;
  }
  if (0 == dev_susp_start(Parameter_List[0].Integer_Value))
    return QCLI_STATUS_SUCCESS_E;
  return QCLI_STATUS_ERROR_E;
}

extern int32_t channel_switch(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t channelSwitch(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)  
{

  if( Parameter_Count != 2 || !Parameter_List ) {
    return QCLI_STATUS_USAGE_E;
  }
  if (0 == channel_switch(Parameter_Count, Parameter_List))
     return QCLI_STATUS_SUCCESS_E;
  return QCLI_STATUS_ERROR_E;
}

extern int32_t disable_channel(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t disableChannel(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)  
{

  if( Parameter_Count != 1 || !Parameter_List ) {
    return QCLI_STATUS_USAGE_E;
  }
  if (0 == disable_channel(Parameter_Count, Parameter_List))
     return QCLI_STATUS_SUCCESS_E;
  return QCLI_STATUS_ERROR_E;
}

QCLI_Command_Status_t arpOffload(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	if( !Parameter_List ) {
	  return QCLI_STATUS_USAGE_E;
	}
	if (0 == arp_offload(Parameter_Count, Parameter_List))
	   return QCLI_STATUS_SUCCESS_E;
	return QCLI_STATUS_ERROR_E;
}

QCLI_Command_Status_t nsOffload(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	if( !Parameter_List ) {
	  return QCLI_STATUS_USAGE_E;
	}
	if (0 == ns_offload(Parameter_Count, Parameter_List))
	   return QCLI_STATUS_SUCCESS_E;
	return QCLI_STATUS_ERROR_E;
}


extern int32_t set_power_mgmt_policy_params(int32_t idle_period_ms, int32_t num_ps_poll, 
                            int32_t dtim_policy, int32_t tx_wakeup_policy, 
                            int32_t num_tx_to_wake_host, int32_t ps_fail_event_policy);
QCLI_Command_Status_t setPowerPolicy(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)  
{ 
  if( Parameter_Count != 6 || !Parameter_List || !Parameter_List[0].Integer_Is_Valid || 
     !Parameter_List[1].Integer_Is_Valid || !Parameter_List[2].Integer_Is_Valid || 
     !Parameter_List[3].Integer_Is_Valid || !Parameter_List[4].Integer_Is_Valid ||
     !Parameter_List[5].Integer_Is_Valid) {
      return QCLI_STATUS_USAGE_E;
  }
  if (0 == set_power_mgmt_policy_params(Parameter_List[0].Integer_Value, Parameter_List[1].Integer_Value, 
                                        Parameter_List[2].Integer_Value, Parameter_List[3].Integer_Value,
                                        Parameter_List[4].Integer_Value, Parameter_List[5].Integer_Value))
      return QCLI_STATUS_SUCCESS_E;
  return QCLI_STATUS_ERROR_E;
}

extern int32_t allow_aggr(char *hexTxTidMask, char *hexRxTidMask);
QCLI_Command_Status_t setAggregationParameters(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)  
{ 
  if(Parameter_Count != 2  || !(Parameter_List[0].Integer_Is_Valid) || !(Parameter_List[1].Integer_Is_Valid))
     return QCLI_STATUS_USAGE_E;
  if (0 == allow_aggr((char *)Parameter_List[0].String_Value, (char *)Parameter_List[1].String_Value))
      return QCLI_STATUS_SUCCESS_E;
  return QCLI_STATUS_ERROR_E;
}

extern int32_t aggrx_config(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t setAggregationRxReorderParameters(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
  if(Parameter_Count < 1 || !Parameter_List)
     return QCLI_STATUS_USAGE_E;
  if (0 == aggrx_config(Parameter_Count, Parameter_List))
      return QCLI_STATUS_SUCCESS_E;
  return QCLI_STATUS_ERROR_E;
}

extern int32_t set_op_mode(char *omode, char *hiddenSsid, char *wpsEnabled);
QCLI_Command_Status_t setOperatingMode(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)  
{ 
  char *hidden = "";
  char *wps    = "";
  if(Parameter_Count < 1 || Parameter_Count > 3 || !Parameter_List || Parameter_List[0].Integer_Is_Valid  )
  {
       return QCLI_STATUS_USAGE_E;
  }
  if (Parameter_Count >= 2)
     hidden = (char *) Parameter_List[1].String_Value;
  if (Parameter_Count == 3)
     wps =  (char *) Parameter_List[2].String_Value;
  return (QCLI_Command_Status_t) set_op_mode((char *)Parameter_List[0].String_Value, hidden, wps);
}

extern int32_t set_ap_dtim_period(uint32_t dtim_period);
QCLI_Command_Status_t setAPDtimPeriod(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)  
{ 
  if( Parameter_Count < 1 || !Parameter_List || !Parameter_List[0].Integer_Is_Valid){
     return QCLI_STATUS_USAGE_E;
  }
  if (0 == set_ap_dtim_period(Parameter_List[0].Integer_Value))
     return QCLI_STATUS_SUCCESS_E;
  return QCLI_STATUS_ERROR_E;  
}

extern int32_t set_ap_inactivity_period(uint32_t inactivity_time_in_mins);
QCLI_Command_Status_t setAPInactivityPeriod(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
  if( Parameter_Count < 1 || !Parameter_List || !Parameter_List[0].Integer_Is_Valid){
     return QCLI_STATUS_USAGE_E;
  }
  if (0 == set_ap_inactivity_period(Parameter_List[0].Integer_Value))
     return QCLI_STATUS_SUCCESS_E;
  return QCLI_STATUS_ERROR_E;  
}

extern int32_t set_ap_beacon_interval(uint32_t beacon_int_in_tu);
QCLI_Command_Status_t setAPBeaconInterval(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)  
{ 
  if( Parameter_Count < 1 || !Parameter_List || !Parameter_List[0].Integer_Is_Valid){
    return QCLI_STATUS_USAGE_E;
  }
  if (0 == set_ap_beacon_interval(Parameter_List[0].Integer_Value))
     return QCLI_STATUS_SUCCESS_E;
  return QCLI_STATUS_ERROR_E;  
}

extern int32_t set_ap_ps_buf(uint32_t ps_buf_enable, uint32_t buff_count);
QCLI_Command_Status_t setAPPowerSaveBufferParameters(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)  
{ 
  uint32_t buff_count;
  if( Parameter_Count < 1 || !Parameter_List || !Parameter_List[0].Integer_Is_Valid){
     return QCLI_STATUS_USAGE_E;
  }
  if (Parameter_Count == 2 && !Parameter_List[0].Integer_Is_Valid){
     return QCLI_STATUS_USAGE_E;
  }     
  buff_count = (Parameter_Count == 1)? 0 : Parameter_List[1].Integer_Value;
  if (0 == set_ap_ps_buf(Parameter_List[0].Integer_Value, buff_count))
     return QCLI_STATUS_SUCCESS_E;
  return QCLI_STATUS_ERROR_E;  
}

extern int32_t set_ap_uapsd(uint32_t uapsd_enable);
QCLI_Command_Status_t setAPUapsd(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)  
{ 
  if( Parameter_Count < 1 || !Parameter_List || !Parameter_List[0].Integer_Is_Valid){
    return QCLI_STATUS_USAGE_E;
  }
  if (0 == set_ap_uapsd(Parameter_List[0].Integer_Value))
     return QCLI_STATUS_SUCCESS_E;
  return QCLI_STATUS_ERROR_E;  
}

extern int32_t set_sta_uapsd(uint32_t ac_mask);
QCLI_Command_Status_t setSTAUapsd(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)  
{ 
  if( Parameter_Count < 1 || !Parameter_List || !Parameter_List[0].Integer_Is_Valid){
    return QCLI_STATUS_USAGE_E;
  }
  if (0 == set_sta_uapsd(Parameter_List[0].Integer_Value))
     return QCLI_STATUS_SUCCESS_E;
  return QCLI_STATUS_ERROR_E;  
}

extern int32_t set_max_sp_len(uint32_t maxsp);
QCLI_Command_Status_t setMaxSPLen(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)  
{ 
  if( Parameter_Count < 1 || !Parameter_List || !Parameter_List[0].Integer_Is_Valid){
    return QCLI_STATUS_USAGE_E;
  }
  if (0 == set_max_sp_len(Parameter_List[0].Integer_Value))
     return QCLI_STATUS_SUCCESS_E;
  return QCLI_STATUS_ERROR_E;  
}

extern uint32_t enable_probe_req_event(int32_t enable);
QCLI_Command_Status_t rxProbeReq(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List) 
{ 
  if( Parameter_Count < 1 || !Parameter_List || !Parameter_List[0].Integer_Is_Valid){
    return QCLI_STATUS_USAGE_E;
  }
  if (0 == enable_probe_req_event(Parameter_List[0].Integer_Value))
     return QCLI_STATUS_SUCCESS_E;
  return QCLI_STATUS_ERROR_E;  
}

extern uint32_t get_wlan_stats( uint8_t flag );
QCLI_Command_Status_t getStats(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)  
{ 
   if( Parameter_Count < 1 || !Parameter_List || !Parameter_List[0].Integer_Is_Valid){
      return QCLI_STATUS_USAGE_E;
  }
  if (0 == get_wlan_stats(Parameter_List[0].Integer_Value))
     return QCLI_STATUS_SUCCESS_E;
  return QCLI_STATUS_ERROR_E;
}

extern uint32_t get_wlan_channel_list();
QCLI_Command_Status_t getRegDomainChannelList(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
  if (0 == get_wlan_channel_list())
  	return QCLI_STATUS_SUCCESS_E;
  return QCLI_STATUS_ERROR_E;  
}


extern int32_t dbglog_handler(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t dbglogHandler(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{ 
   if( Parameter_Count < 1 || !Parameter_List ){
      return QCLI_STATUS_USAGE_E;
  }
  if (0 == dbglog_handler(Parameter_Count, Parameter_List))
     return QCLI_STATUS_SUCCESS_E;
  return QCLI_STATUS_ERROR_E;
}

extern int32_t pktlog_handler(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t pktlogHandler(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{ 
   if( Parameter_Count < 1 || !Parameter_List ){
      return QCLI_STATUS_USAGE_E;
  }
  if (0 == pktlog_handler(Parameter_Count, Parameter_List))
     return QCLI_STATUS_SUCCESS_E;
  return QCLI_STATUS_ERROR_E;
}

extern int32_t regquery_handler(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t regqueryHandler(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{ 
#if defined(WLAN_DEBUG)
   if( Parameter_Count < 1 || !Parameter_List ){
      return QCLI_STATUS_USAGE_E;
  }
  if (0 == regquery_handler(Parameter_Count, Parameter_List))
     return QCLI_STATUS_SUCCESS_E;
#endif

  return QCLI_STATUS_ERROR_E;
}
extern int32_t memquery_handler(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t memqueryHandler(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
#if defined(WLAN_DEBUG)
   if( Parameter_Count < 1 || !Parameter_List ){
      return QCLI_STATUS_USAGE_E;
  }
  if (0 == memquery_handler(Parameter_Count, Parameter_List))
     return QCLI_STATUS_SUCCESS_E;
#endif

  return QCLI_STATUS_ERROR_E;
}
extern int32_t driver_assert_handler(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t driverassertHandler(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{ 
#if defined(WLAN_DEBUG)
  if (0 == driver_assert_handler(Parameter_Count, Parameter_List))
     return QCLI_STATUS_SUCCESS_E;
#endif
  return QCLI_STATUS_ERROR_E;
}

extern int32_t send_raw_frame(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t sendRawFrame(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List) 
{ 
  if(!Parameter_List) {
      return QCLI_STATUS_USAGE_E;
  }
  if (0 == send_raw_frame(Parameter_Count, Parameter_List))
      return QCLI_STATUS_SUCCESS_E;
  return QCLI_STATUS_ERROR_E;
}

extern int32_t promiscuous_mode_handler(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t promiscHandler(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List) 
{ 
  if(!Parameter_List) {
      return QCLI_STATUS_USAGE_E;
  }
  if (0 == promiscuous_mode_handler(Parameter_Count, Parameter_List))
      return QCLI_STATUS_SUCCESS_E;
  return QCLI_STATUS_ERROR_E;
}

extern int32_t set_lpl(uint32_t enLpl);
QCLI_Command_Status_t enableLpl(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{ 
  if( Parameter_Count < 1 || !Parameter_List || !Parameter_List[0].Integer_Is_Valid){
     return QCLI_STATUS_USAGE_E;
  }
  if (0 == set_lpl(Parameter_List[0].Integer_Value))
     return QCLI_STATUS_SUCCESS_E;
  return QCLI_STATUS_ERROR_E;  
}

extern int32_t set_gtx(uint32_t enGtx);
QCLI_Command_Status_t enableGtx(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{ 
  if( Parameter_Count < 1 || !Parameter_List || !Parameter_List[0].Integer_Is_Valid){
     return QCLI_STATUS_USAGE_E;
  }
  if (0 == set_gtx(Parameter_List[0].Integer_Value))
     return QCLI_STATUS_SUCCESS_E;
  return QCLI_STATUS_ERROR_E; 
}

extern int32_t set_sta_mac_keep_alive_time(int32_t keep_alive_in_secs);
QCLI_Command_Status_t setSTAMacKeepAliveTimeout(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
  if( Parameter_Count < 1 || !Parameter_List || !Parameter_List[0].Integer_Is_Valid){
     return QCLI_STATUS_USAGE_E;
  }
  if (0 == set_sta_mac_keep_alive_time(Parameter_List[0].Integer_Value))
     return QCLI_STATUS_SUCCESS_E;
  return QCLI_STATUS_ERROR_E; 
}

extern int32_t tcp_keepalive_offload_enable_disable( uint8_t enable, uint16_t keepalive_intvl, uint16_t keepalive_ack_recv_threshold );
QCLI_Command_Status_t enableTcpKeepaliveOffload(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)  
{ 
  if( Parameter_Count != 3 || !Parameter_List || !Parameter_List[0].Integer_Is_Valid || 
     !Parameter_List[1].Integer_Is_Valid || !Parameter_List[2].Integer_Is_Valid) {
      return QCLI_STATUS_USAGE_E;
  }
  if (0 == tcp_keepalive_offload_enable_disable(Parameter_List[0].Integer_Value, Parameter_List[1].Integer_Value, 
                                        Parameter_List[2].Integer_Value))
      return QCLI_STATUS_SUCCESS_E;
  return QCLI_STATUS_ERROR_E;
}

extern int32_t tcp_keepalive_offload_session_config(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t offloadTcpSessionKeepAlive(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{ 
  if(!Parameter_List) {
      return QCLI_STATUS_USAGE_E;
  }
  if (0 == tcp_keepalive_offload_session_config(Parameter_Count, Parameter_List))
      return QCLI_STATUS_SUCCESS_E;
  return QCLI_STATUS_ERROR_E;
}

extern int32_t pno_enable(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t enablePreferredNetworkOffload(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)  
{ 
  if(!Parameter_List) {
      return QCLI_STATUS_USAGE_E;
  }
  if (0 == pno_enable(Parameter_Count, Parameter_List))
      return QCLI_STATUS_SUCCESS_E;
  return QCLI_STATUS_ERROR_E;
}

int32_t set_pno_network_profile(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t setPnoNetworkProfile(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)  
{ 
  if(!Parameter_List) {
      return QCLI_STATUS_USAGE_E;
  }
  if (0 == set_pno_network_profile(Parameter_Count, Parameter_List))
      return QCLI_STATUS_SUCCESS_E;
  return QCLI_STATUS_ERROR_E;
}

extern int32_t wnm_config(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t set11v(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if(Parameter_Count < 1 || !Parameter_List || !Parameter_List[0].Integer_Is_Valid) {
        return QCLI_STATUS_USAGE_E;
    }
    if((Parameter_List[0].Integer_Value != 0) && (Parameter_List[0].Integer_Value != 1)) {
        return QCLI_STATUS_USAGE_E;
    }
    if (0 == wnm_config(Parameter_Count, Parameter_List)){
      return QCLI_STATUS_SUCCESS_E;
    }
    return QCLI_STATUS_ERROR_E;
}

extern int32_t set_ap_bss_max_idle_period(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t setAPBssMaxIdlePeriod(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{ 
    if(Parameter_Count < 1 || !Parameter_List) {
        return QCLI_STATUS_USAGE_E;
    }
    if(!Parameter_List[0].Integer_Is_Valid || (Parameter_List[0].Integer_Value < 1)){
        return QCLI_STATUS_USAGE_E;
    }
    
    if (0 == set_ap_bss_max_idle_period(Parameter_Count, Parameter_List)) {
        return QCLI_STATUS_SUCCESS_E;
    }
    return QCLI_STATUS_ERROR_E;
}

extern int32_t set_wnm_sleep_period(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t setWnmSleepPeriod(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{ 
    if(Parameter_Count < 2 || !Parameter_List ||
        !Parameter_List[0].Integer_Is_Valid || !Parameter_List[1].Integer_Is_Valid) {
        return QCLI_STATUS_USAGE_E;
    }
    if((Parameter_List[0].Integer_Value != 0) && (Parameter_List[0].Integer_Value != 1)) {
        return QCLI_STATUS_USAGE_E;
    }
    if (Parameter_List[1].Integer_Value < 0) {
            return QCLI_STATUS_USAGE_E;
    }   
    if (0 == set_wnm_sleep_period(Parameter_Count, Parameter_List)) {
        return QCLI_STATUS_SUCCESS_E;
    }
    return QCLI_STATUS_ERROR_E;
}

extern int32_t set_wnm_sleep_response(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t setWnmSleepResponse(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{ 
    if(Parameter_Count < 1 || !Parameter_List || !Parameter_List[0].Integer_Is_Valid){
        return QCLI_STATUS_USAGE_E;
    }
    if((Parameter_List[0].Integer_Value < 0) || (Parameter_List[0].Integer_Value > 5)) {
        return QCLI_STATUS_USAGE_E;
    }
    if (0 == set_wnm_sleep_response(Parameter_Count, Parameter_List)){
        return QCLI_STATUS_SUCCESS_E;
    }
    return QCLI_STATUS_ERROR_E;
}

extern int32_t enable_wow(uint32_t cmd);
QCLI_Command_Status_t enableWow(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if(Parameter_Count < 1 || !Parameter_List){
        return QCLI_STATUS_USAGE_E;
    }
    if(-1 == enable_wow((uint32_t)Parameter_List[0].Integer_Value))
        return QCLI_STATUS_ERROR_E;
    
    return QCLI_STATUS_SUCCESS_E;
}

extern int32_t enable_pkt_filter(uint32_t cmd);
QCLI_Command_Status_t enablePktFilter(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if(Parameter_Count < 1 || !Parameter_List){
        return QCLI_STATUS_USAGE_E;
    }
    if(-1 == enable_pkt_filter((uint32_t)Parameter_List[0].Integer_Value))
        return QCLI_STATUS_ERROR_E;
    
    return QCLI_STATUS_SUCCESS_E;
}

extern int32_t add_pattern(uint32_t pattern_index, uint8_t action, uint8_t wow_filter, uint8_t priority, uint16_t header_type, uint32_t offset, 
                                uint32_t pattern_size, uint8_t *pattern_mask, uint8_t *pattern_data);
QCLI_Command_Status_t addFilterPattern(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    uint8_t *pattern_mask;
    uint8_t *pattern_data;
    uint32_t pattern_size;
    QCLI_Command_Status_t status = QCLI_STATUS_SUCCESS_E;
    
    if(Parameter_Count < 9 || !Parameter_List){
        return QCLI_STATUS_USAGE_E;
    }
    pattern_size = Parameter_List[6].Integer_Value;

    if (((pattern_size/sizeof(uint8_t)) + 1) < (strlen(Parameter_List[7].String_Value) / 2))
		return QCLI_STATUS_USAGE_E;
	
	if(pattern_size < (strlen(Parameter_List[8].String_Value) / 2))
		return QCLI_STATUS_USAGE_E;
	
    pattern_mask = (uint8_t *)malloc((pattern_size/sizeof(uint8_t)) + 1);
    if(NULL == pattern_mask)
        return QCLI_STATUS_ERROR_E;
    
    pattern_data = (uint8_t *)malloc(pattern_size);
    if(NULL == pattern_data){
        free(pattern_mask);
        return QCLI_STATUS_ERROR_E;
    }
	
    if(!Util_String_To_Hex(pattern_mask, Parameter_List[7].String_Value, strlen(Parameter_List[7].String_Value))){
        status = QCLI_STATUS_ERROR_E;
        goto error;
    }
    
    if(!Util_String_To_Hex(pattern_data, Parameter_List[8].String_Value,strlen(Parameter_List[8].String_Value))){
        status = QCLI_STATUS_ERROR_E;
        goto error;
    }
    if(-1 == add_pattern(Parameter_List[0].Integer_Value,
                         (uint8_t)Parameter_List[1].Integer_Value, 
                         (uint8_t)Parameter_List[2].Integer_Value,
                         (uint8_t)Parameter_List[3].Integer_Value,
                         (uint16_t)Parameter_List[4].Integer_Value,
                         Parameter_List[5].Integer_Value,
                         pattern_size,
                         pattern_mask,
                         pattern_data))
         status =  QCLI_STATUS_ERROR_E;
error:
    free(pattern_mask);
    free(pattern_data);
    return status;
}

int32_t delete_pattern(uint32_t index, uint32_t header_type);
QCLI_Command_Status_t deleteFilterPattern(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if(Parameter_Count < 2 || !Parameter_List){
        return QCLI_STATUS_USAGE_E;
    }
    if(-1 == delete_pattern((uint32_t)Parameter_List[0].Integer_Value,(uint32_t)Parameter_List[1].Integer_Value))
        return QCLI_STATUS_ERROR_E;
    
    return QCLI_STATUS_SUCCESS_E;
}

int32_t change_default_filter_action(uint32_t action, uint32_t wow_flag,uint32_t header_type);
QCLI_Command_Status_t changeDefaultFilterAction(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if(Parameter_Count < 3 || !Parameter_List){
        return QCLI_STATUS_USAGE_E;
    }
    if(-1 == change_default_filter_action((uint32_t)Parameter_List[0].Integer_Value,(uint32_t)Parameter_List[1].Integer_Value,(uint32_t)Parameter_List[2].Integer_Value)) {
        return QCLI_STATUS_ERROR_E;
    }
    return QCLI_STATUS_SUCCESS_E;
}

int32_t set_event_filter(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t setEventFilter(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{ 
   uint16_t param;
   if (Parameter_Count < 1 || !Parameter_List){
	   goto EVENTFILTER_COMMAND_USAGE;
   }
   for (param=0; param <Parameter_Count; param++)
   {
      if (!Parameter_List[param].Integer_Is_Valid)
	   goto EVENTFILTER_COMMAND_USAGE;
   }

   if (0 == set_event_filter(Parameter_Count, Parameter_List))
      return QCLI_STATUS_SUCCESS_E;

   return QCLI_STATUS_ERROR_E;

   EVENTFILTER_COMMAND_USAGE:
       QCLI_Printf(qcli_wlan_group, "setEventFilter <1:enable | 0:disable> [<eventId> <eventId>...]\n");
	   QCLI_Printf(qcli_wlan_group, "Event ID options:\n");
	   QCLI_Printf(qcli_wlan_group, "\t 4100: BSS info event\n");
	   QCLI_Printf(qcli_wlan_group, "\t 4112: WMI extension event (device 0 only)\n");
	   QCLI_Printf(qcli_wlan_group, "\t 4122: Channel change event\n");
	   QCLI_Printf(qcli_wlan_group, "\t 4123: IBSS connection event (device 0 only)\n");
	   QCLI_Printf(qcli_wlan_group, "\t 4128: Event indicating ADDBA request sent\n");
	   QCLI_Printf(qcli_wlan_group, "\t 4129: ADDBA response reception event\n");
	   QCLI_Printf(qcli_wlan_group, "\t 4130: DELBA request event\n");
	   QCLI_Printf(qcli_wlan_group, "\t 4158: P2P invitation request reception event (device 0 only)\n");
	   QCLI_Printf(qcli_wlan_group, "\t 4159: Event indicating P2P invitation response sent (device 0 only)\n");
	   QCLI_Printf(qcli_wlan_group, "\t 4160: P2P invitation response reception event (device 0 only)\n");
	   QCLI_Printf(qcli_wlan_group, "\t 4161: P2P provision discovery response reception event (device 0 only)\n");
	   QCLI_Printf(qcli_wlan_group, "\t 4162: P2P provision discovery request reception event (device 0 only)\n");
	   QCLI_Printf(qcli_wlan_group, "\t 4165: P2P event for service discovery, provision discovery start (device 0 only)\n");
	   QCLI_Printf(qcli_wlan_group, "\t 4166: P2P service discovery, provision discovery reception event (device 0 only)\n");
	   QCLI_Printf(qcli_wlan_group, "\t12296: DBGLOG event to indicate buffer full or threadshold reached (device 0 only)\n");
	   QCLI_Printf(qcli_wlan_group, "\t12298: PKTLOG event to indicate buffer full or threadshold reached (device 0 only)\n");
	   QCLI_Printf(qcli_wlan_group, "\t36879: WPS profile event\n");
	   QCLI_Printf(qcli_wlan_group, "\t36884: Bus flow control event\n");
	   QCLI_Printf(qcli_wlan_group, "\t36891: Event indicating pending authentication for a group owner negotiation request received from a peer device (device 0 only)\n");
	   QCLI_Printf(qcli_wlan_group, "\t36892: Diagnositc event (device 0 only)\n");
	   QCLI_Printf(qcli_wlan_group, "\t36936: WNM event\n");
       return QCLI_STATUS_ERROR_E;
}

extern int32_t set_sta_bmiss_config(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t setStaBmissConfig(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)  
{

  if( Parameter_Count != 2 || !Parameter_List ) {
    return QCLI_STATUS_USAGE_E;
  }
  if (0 == set_sta_bmiss_config(Parameter_Count, Parameter_List))
     return QCLI_STATUS_SUCCESS_E;
  return QCLI_STATUS_ERROR_E;
}

extern int32_t get_ssid();
QCLI_Command_Status_t getSSID(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if (0 == get_ssid()) {
        return QCLI_STATUS_SUCCESS_E;
    }
    return QCLI_STATUS_ERROR_E;
}

extern int32_t get_phy_mode();
QCLI_Command_Status_t getPhyMode(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if (0 == get_phy_mode()) {
        return QCLI_STATUS_SUCCESS_E;
    }
    return QCLI_STATUS_ERROR_E;
}

extern int32_t get_wifi_power_mode();
QCLI_Command_Status_t getWiFiPowerMode(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if (0 == get_wifi_power_mode()) {
        return QCLI_STATUS_SUCCESS_E;
    }
    return QCLI_STATUS_ERROR_E;
}

extern int32_t get_device_mac_address();
QCLI_Command_Status_t getMacAddress(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if (0 == get_device_mac_address()) {
        return QCLI_STATUS_SUCCESS_E;
    }
    return QCLI_STATUS_ERROR_E;
}

extern int32_t get_op_mode();
QCLI_Command_Status_t getOperatingMode(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if (0 == get_op_mode()) {
        return QCLI_STATUS_SUCCESS_E;
    }
    return QCLI_STATUS_ERROR_E;
}

extern int32_t get_channel();
QCLI_Command_Status_t getChannel(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if (0 == get_channel()) {
        return QCLI_STATUS_SUCCESS_E;
    }
    return QCLI_STATUS_ERROR_E;
}

extern int32_t get_wep_key(uint8_t);
QCLI_Command_Status_t getWepKeyPair(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if (Parameter_Count < 1 || !Parameter_List[0].Integer_Is_Valid)
    {
        return QCLI_STATUS_USAGE_E;
    }
    if (0 == get_wep_key(Parameter_List[0].Integer_Value))
    {
        return QCLI_STATUS_SUCCESS_E;
    }
    return QCLI_STATUS_ERROR_E;
}

extern int32_t get_wep_keyix();
QCLI_Command_Status_t getDefaultWepKeyIndex(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if (0 == get_wep_keyix())
    {
        return QCLI_STATUS_SUCCESS_E;		
    }
    return QCLI_STATUS_ERROR_E;
}

#if ENABLE_P2P_MODE

extern int32_t p2p_enable();
QCLI_Command_Status_t p2pEnable(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	if (0 == p2p_enable())
	{
        return QCLI_STATUS_SUCCESS_E;
    }
    return QCLI_STATUS_ERROR_E;
}

extern int32_t p2p_set_config(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t p2pSetConfig(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	if((Parameter_Count < 5))
    {
		 return QCLI_STATUS_USAGE_E;
    }

    if (0 == p2p_set_config(Parameter_Count, Parameter_List))
    {
        return QCLI_STATUS_SUCCESS_E;
    }
	return QCLI_STATUS_ERROR_E;
}

extern int32_t p2p_connect(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t p2pConnect(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	if((Parameter_Count < 2))
    {
		 return QCLI_STATUS_USAGE_E;
    }
    if (0 == p2p_connect(Parameter_Count, Parameter_List))
    {
        return QCLI_STATUS_SUCCESS_E;
    }
	return QCLI_STATUS_ERROR_E;
}

extern int32_t p2p_find(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t p2pFind(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   if (0 == p2p_find(Parameter_Count, Parameter_List))
   {
	   return QCLI_STATUS_SUCCESS_E;
   }
   
   return QCLI_STATUS_ERROR_E;
}

extern int32_t p2p_provision(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t p2pProvision(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if(Parameter_Count != 2)
    {
        return QCLI_STATUS_USAGE_E;
    }

    if (0 == p2p_provision(Parameter_Count, Parameter_List))
    {
        return QCLI_STATUS_SUCCESS_E;
    }
	return QCLI_STATUS_ERROR_E;
}

extern int32_t p2p_listen(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t p2pListen(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	if(Parameter_Count > 1)
	{
		return QCLI_STATUS_USAGE_E;
	}
	if (0 == p2p_listen(Parameter_Count, Parameter_List))
	{
        return QCLI_STATUS_SUCCESS_E;
    }

    return QCLI_STATUS_ERROR_E;
}

extern int32_t p2p_cancel();
QCLI_Command_Status_t p2pCancel(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	if(Parameter_Count > 0)
	{
		return QCLI_STATUS_USAGE_E;
	}
	if (0 == p2p_cancel())
	{
        return QCLI_STATUS_SUCCESS_E;
    }

    return QCLI_STATUS_ERROR_E;
}

extern int32_t p2p_join(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t p2pJoin(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	if(Parameter_Count < 2)
	{
		return QCLI_STATUS_USAGE_E;
	}
	if (0 == p2p_join(Parameter_Count, Parameter_List))
	{
        return QCLI_STATUS_SUCCESS_E;
    }
   return QCLI_STATUS_ERROR_E;
}

extern int32_t p2p_auth(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t p2pAuth(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if(Parameter_Count < 2)
    {
        return QCLI_STATUS_USAGE_E;
    }
	if (0 == p2p_auth(Parameter_Count, Parameter_List))
    {
        return QCLI_STATUS_SUCCESS_E;
    }
	return QCLI_STATUS_ERROR_E;
}

extern int32_t p2p_auto_go(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t p2pAutoGO(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	if(Parameter_Count > 1)
	{
		return QCLI_STATUS_USAGE_E;
	}
	if (0 == p2p_auto_go(Parameter_Count, Parameter_List))
	{
        return QCLI_STATUS_SUCCESS_E;
    }

    return QCLI_STATUS_ERROR_E;
}

extern int32_t p2p_invite_auth(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t p2pInviteAuth(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	if(Parameter_Count < 4)
	{
		return QCLI_STATUS_USAGE_E;
	}
	if (0 == p2p_invite_auth(Parameter_Count, Parameter_List))
	{
        return QCLI_STATUS_SUCCESS_E;
    }

    return QCLI_STATUS_ERROR_E;
}

extern int32_t p2p_disable();
QCLI_Command_Status_t p2pDisable(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	if (0 == p2p_disable())
	{
        return QCLI_STATUS_SUCCESS_E;
    }
    return QCLI_STATUS_ERROR_E;
}

extern int32_t p2p_set_noa_params(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t p2pSetNOAParams(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	if(Parameter_Count < 4)
	{
		return QCLI_STATUS_USAGE_E;
	}
	if (0 == p2p_set_noa_params(Parameter_Count, Parameter_List))
	{
        return QCLI_STATUS_SUCCESS_E;
    }

    return QCLI_STATUS_ERROR_E;
}

extern int32_t p2p_set_oops_params(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t p2pSetOPPSParams(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	if(Parameter_Count < 2)
	{
		return QCLI_STATUS_USAGE_E;
	}
	if (0 == p2p_set_oops_params(Parameter_Count, Parameter_List))
	{
        return QCLI_STATUS_SUCCESS_E;
    }

    return QCLI_STATUS_ERROR_E;
}

extern int32_t p2p_set_operating_class(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t p2pSetOperatingClass(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	if(Parameter_Count < 3)
	{
		return QCLI_STATUS_USAGE_E;
	}
	if (0 == p2p_set_operating_class(Parameter_Count, Parameter_List))
	{
        return QCLI_STATUS_SUCCESS_E;
    }

    return QCLI_STATUS_ERROR_E;
}

extern int32_t p2p_get_node_list();
QCLI_Command_Status_t p2pGetNodeList(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	if (0 == p2p_get_node_list())
	{
        return QCLI_STATUS_SUCCESS_E;
    }
    return QCLI_STATUS_ERROR_E;
}

extern int32_t p2p_get_network_list();
QCLI_Command_Status_t p2pGetNetworkList(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	if (0 == p2p_get_network_list())
	{
        return QCLI_STATUS_SUCCESS_E;
    }
    return QCLI_STATUS_ERROR_E;
}

extern int32_t p2p_set(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t p2pSet(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	if(Parameter_Count < 2)
	{
		return QCLI_STATUS_USAGE_E;
	}
	if (0 == p2p_set(Parameter_Count, Parameter_List))
	{
        return QCLI_STATUS_SUCCESS_E;
    }

    return QCLI_STATUS_ERROR_E;
}

extern int32_t p2p_stop_find();
QCLI_Command_Status_t p2pStopFind(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	if (0 == p2p_stop_find())
	{
        return QCLI_STATUS_SUCCESS_E;
    }
    return QCLI_STATUS_ERROR_E;
}

//extern int32_t p2p_passphrase(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t p2pPassphrase(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	if(Parameter_Count < 2)
	{
		return QCLI_STATUS_USAGE_E;
	}
	if (0 == p2p_passphrase(Parameter_Count, Parameter_List))
	{
        return QCLI_STATUS_SUCCESS_E;
    }

    return QCLI_STATUS_ERROR_E;
}

#endif /* ENABLE_P2P_MODE */

extern int32_t set_app_ie(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t setApplicationIe(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if(Parameter_Count < 2 || !Parameter_List || !Parameter_List[0].Integer_Is_Valid){
        return QCLI_STATUS_USAGE_E;
    }

	if (0 == set_app_ie(Parameter_Count, Parameter_List))
	{
	   return QCLI_STATUS_SUCCESS_E;
	}
	return QCLI_STATUS_ERROR_E;
}

extern int32_t set_ant_div(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t setAntDiv(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	if(Parameter_Count < 1 || !Parameter_List || !Parameter_List[0].Integer_Is_Valid)
	{
        return QCLI_STATUS_USAGE_E;
    }

	if(((Parameter_Count > 1) && (!Parameter_List[1].Integer_Is_Valid))
		||((Parameter_Count > 2) && (!Parameter_List[2].Integer_Is_Valid))
		||((Parameter_Count > 3) && (!Parameter_List[3].Integer_Is_Valid))
		||(Parameter_Count > 4))
	{
		return QCLI_STATUS_USAGE_E;
	}
	
	if (0 == set_ant_div(Parameter_Count, Parameter_List))
	{
	   return QCLI_STATUS_SUCCESS_E;
	}
	return QCLI_STATUS_ERROR_E;
}

extern int32_t get_ant_div_status();
QCLI_Command_Status_t getAntDivStatus(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	if (0 == get_ant_div_status()) {
        return QCLI_STATUS_SUCCESS_E;
    }
    return QCLI_STATUS_ERROR_E;
}

extern int32_t set_antenna(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t setAntenna(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	if(Parameter_Count !=1 || !Parameter_List || !Parameter_List[0].Integer_Is_Valid)
	{
        return QCLI_STATUS_USAGE_E;
    }
	
	if (0 == set_antenna(Parameter_Count, Parameter_List))
	{
	   return QCLI_STATUS_SUCCESS_E;
	}
	return QCLI_STATUS_ERROR_E;
}

