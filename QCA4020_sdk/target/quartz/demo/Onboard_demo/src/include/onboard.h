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

#ifndef __ONBOARD_H__
#define __ONBOARD_H__

/**
 * @file onboard.h
 * @brief File contains the configuration macros for onboard.
 *
 * This file contains the configuration macros and functions that
 * is used for onboarding functionality
 */

/**------------------------------------------------------------------------
  - Preprocessor Definitions and Constants
  ------------------------------------------------------------------------*/

/**
This macro help in disabling and enabling the command line options
*/
#define SUPPORT_TESTING

/*------------- START OF USER CONFIG -----------------------*/

#define ONB_INFO_CHIPSET                  CHIPSET_VARIANT

#define OPERATION_MODE_WIFI               (1<<0)                /**< Indicates Wi-Fi can be onboarded */
#define OPERATION_MODE_THREAD             (1<<1)                /**< Indicates Thread can be onboarded */
#define OPERATION_MODE_ZIGBEE             (1<<2)                /**< Indicates Zigbee can be onboarded */

/** Indicates radios that can be onboarded */
#ifdef THREAD
#define ONBOARDED_OPERATION_MODE          (OPERATION_MODE_WIFI | OPERATION_MODE_THREAD)
#endif
#ifdef ZIGBEE
#define ONBOARDED_OPERATION_MODE          (OPERATION_MODE_WIFI | OPERATION_MODE_ZIGBEE)
#endif

/** Location where Wi-Fi Onboard credentials are stored */
#define WLAN_STA_CRED_FILE                "/spinor/onboard/wifi_cred.txt"
/** Location where Zigbee Onboard credentials are stored */
#define ZIGBEE_CRED_FILE                  "/spinor/onboard/zigbee_cred.txt"

/** Location where Thread Onboard credentials are stored */
#define THREAD_CRED_FILE                  "/spinor/onboard/thread_cred.txt"

/** Signal wait timeout for STA to connect to an AP */
#define WIFI_CONNECT_TIMEOUT              20000

/** Onboarding SIGNAL Events */
#define SIGNAL_ONBOARD_EVENT_WIFI         	(1<<0)       /**< Event for onboarding the Wi-Fi */
#define SIGNAL_ONBOARD_EVENT_THREAD       	(1<<1)       /**< Event for onboarding the THREAD */
#define SIGNAL_ONBOARD_EVENT_ZIGBEE       	(1<<2)       /**< Event for onboarding the Zigbee */
#define PIR_THREAD_SIGNAL_INTR              (1<<3)       /**< Event for PIR Signal */

/*Zigbee Signal Events*/
#define SIGNAL_ADD_DEVICE_EVENT_ZIGBEE    	(1<<0)       /**< Event for Adding device to device table */
#define SIGNAL_ADD_DEVICE_CRD_EVENT_ZIGBEE  (1<<1)       /**< Event for Adding Coordinator to device table */

#define ONBOARD_THRD_PRIO                 10           /**< Onboard Thread Priority */
#define ONBOARD_STACK_SIZE                2048         /**< Onboard Thread stack size */

/* Monitor Thread */
#define  MONITOR_THREAD_PRIO              10           /**< Monitor Thread Priority */
#define  MONITOR_STACK_SIZE               2048         /**< Monitor Thread stack size */

/**
Interval in seconds at which monitor thread would wakeup and monitor the system
*/
#if BOARD_SUPPORTS_WIFI
#define MONITOR_THREAD_TIMEOUT           30000
#else
#define MONITOR_THREAD_TIMEOUT           2000
#endif

/** Monitor Signal Events */
#define SIGNAL_MONITOR_EVENT_EXIT         (1<<0)       /**< Event to indicate monitor thread to exit */
#define SIGNAL_MONITOR_EVENT_INTERFACES   (1<<1)       /**< Event to indicate monitor thread to monitor the interface */
#define SIGNAL_REJOIN_EVENT_ZIGBEE        (1<<2)       /**< Event for Rejoining  the Zigbee */
#define SIGNAL_MONITOR_EVENT_UNBLOCK      (1<<3)       /**< Event to unblocl monitor thread */

#define PERMIT_TIME                       255          /**< Permit Time to join the Network */
#define TIME_DURATION                     300          /**< Time Interval to call the permit_join function */
#define ZIGBEE_SECURITY                   1            /**< enable/disable Security 1-enable 0-disable */

#define ZIGBEE_CHANNEL_MASK               0            /**< 0 will take default FORM_CHANNEL_MASK.
                                                            Valid Channels are from from 11 to 26 */  
#define SCAN_ATTEMPT                      2            /**< Allowing device to Join network with max allowed scan attempt */

/*------------- END OF USER CONFIG -----------------------*/

//Zigbee Link key
#define MAX_LINK_KEY_SIZE 16
#define MAX_THREAD_PASSPHRASE_SIZE 16

#ifdef ONBOARD_VIA_WIFI

int32_t Initialize_onboard_via_wifi(void);
int32_t Start_onboard_via_wifi(void);

#else // ONBOARD_VIA_WIFI

int32_t Initialize_onboard_via_ble(void);
int32_t Start_onboard_via_ble(void);
#endif // ONBOARD_VIA_BLE

/*-------------------------------------------------------------------------
 *Typedef and Declarations
 *------------------------------------------------------------------------*/
void Initialize_onboard_demo(void);
void Start_onboard_demo(void);
int32_t ble_get_device_mac_address(uint8_t *mac);
int32_t set_wlan_sta_info(const char *wifi_ssid, const char *wifi_passwd);
int32_t set_zigbee_info(uint8_t , uint8_t *);
int32_t set_thread_info(uint8_t , uint8_t *);
int32_t Start_zigbee(char Mode, char *Master_key);
int32_t Start_thread(char Mode, char *Master_key);
int32_t rejoin_zigbee(char Mode, char *Master_key);
void zb_raise_rejoin_event(void);
void zb_add_device_signal(void);
uint16_t get_zigbee_mode(void);
uint16_t get_thread_mode(void);
int32_t Read_Zigbee_Devices_Data(char *data, uint32_t size);
int32_t Read_Thread_Devices_Data(char *data, uint32_t size);
int32_t Zigbee_read_sensors();
void zb_raise_rejoin_event(void);
void Start_thread_network(void);
void Shutdown_thread_network(void);
uint64_t GetExtenAddr(void);
boolean is_wifi_onboarded(void);
boolean is_zigbee_onboarded(void);
boolean is_thread_onboarded(void);
int update_breach_message();
void print_heap_status();
char zigbee_mode();
char thread_mode();
#endif // __ONBOARD_H
