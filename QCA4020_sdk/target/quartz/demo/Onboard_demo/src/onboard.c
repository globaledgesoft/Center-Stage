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


/**
 * @file onboard.c
 * @brief File contains the logic to handle the onboarding of radios.
 *
 * This file contains the logic to handle the onboard events received
 * with the configuration info for the radios to be onboarded
 * via Wi-Fi or BLE from the Mobile APP.
 *
 * Implements two threads:
 * process_onboard_event: Handles all the onboard event received.
 *    Uses the configutration received from the Mobile app to onboard
 *    the radios to the network
 * process_monitor_event: Handles the monitor event and wakesup at
 *    configured interval for monitoring the radios/connectivity
 */


#include "malloc.h"
#include "string.h"
#include "stringl.h"
#include "stdarg.h"

#include <qcli_api.h>
#include <qcli_util.h>

#include <qapi_wlan.h>
#include <qapi_zb.h>
#include <qapi/qapi_socket.h>
#include <qapi_ns_utils.h>

#include <qapi/qapi_reset.h>
#include "qapi_heap_status.h"
#include <qapi_fs.h>

#include <qurt_timer.h>
#include <qurt_thread.h>
#include <qurt_signal.h>
#include <qurt_error.h>
#include <qurt_mutex.h>

#include "wifi_util.h"
#include "netutils.h"
#include "util.h"
#include "thread_util.h"

#ifdef AWS_IOT
#include "aws_util.h"
#endif

#include "timer_interface.h"
#include "led_utils.h"
#include "log_util.h"
#include "zigbee_util.h"

#include "onboard.h"
#include "sensors_demo.h"
#include "sensor_json.h"
#include "offline.h"


/*-------------------------------------------------------------------------
  - Preprocessor Definitions and Constants
  ------------------------------------------------------------------------*/

#define ONB_ERROR               LOG_ERROR
#define ONB_WARN                LOG_WARN
#define ONB_INFO                LOG_INFO
#define ONB_VERBOSE             LOG_VERBOSE

#if defined BOARD_SUPPORTS_WIFI && (ONBOARDED_OPERATION_MODE & OPERATION_MODE_WIFI)
extern qurt_signal_t wlan_sigevent;
extern int32_t recv_data_process;

typedef struct WLAN_info {
    char ssid[MAX_SSID_LEN+1];
    char password[MAX_PASSPHRASE_LEN+1];
    char ip[IPV4ADDR_STR_LEN+1];
    char netmask[IPV4ADDR_STR_LEN+1];
    char gw_ip[IPV4ADDR_STR_LEN+1];
} WLAN_info_t;


static WLAN_info_t sta_info;
#endif

typedef struct Thread_info {
    uint8_t mode;
    char    passphrase[MAX_THREAD_PASSPHRASE_SIZE + 1];
} THREAD_info_t;

QCLI_Group_Handle_t qcli_onboard; /**< QCLI handle for receiving commands and printing the debug prints */

qurt_signal_t onboard_sigevent;
qurt_signal_t monitor_sigevent;
static ZIGBEE_info_t zigbee_dev_info;
static THREAD_info_t thread_dev_info;

static boolean wifi_onboard_status; /**< Indicates that the AP information is available */
static boolean zigbee_onboard_status; /**< Indicates that the ZB information is available */
static boolean thread_onboard_status; /**< Indicates that the THREAD information is available */

static qurt_mutex_t onboard_lock; /**< To avoid race condition between the onboard and monitor thread */

#ifdef SUPPORT_TESTING
static QCLI_Command_Status_t reset_onboard_info(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
#endif

const QCLI_Command_t onboard_cmd_list[]=
{
#ifdef SUPPORT_TESTING
    { reset_onboard_info,    false,    "reset_onboard_info",  "",   "resets the oboard info allowing APP to reconfigure" },
#endif
};

const QCLI_Command_Group_t onboard_cmd_group =
{
    "Onboard",
    (sizeof(onboard_cmd_list) / sizeof(onboard_cmd_list[0])),
    onboard_cmd_list
};

/*-------------------------------------------------------------------------
  - Functions
  ------------------------------------------------------------------------*/
#ifdef SUPPORT_TESTING
static void reset_wifi_onboard_status(void)
{
    if (wifi_onboard_status)
    {
        if (QAPI_OK != qapi_Fs_Unlink(WLAN_STA_CRED_FILE))
        {
            ONB_WARN("Failed to unlink %s\n", WLAN_STA_CRED_FILE);
        }
        wifi_onboard_status = false;
    }
}

#if (ONBOARDED_OPERATION_MODE & OPERATION_MODE_ZIGBEE)
static void reset_zigbee_onboard_status(void)
{
    if (zigbee_onboard_status)
    {
        if (QAPI_OK != qapi_Fs_Unlink(ZIGBEE_CRED_FILE))
        {
            ONB_WARN("Failed to unlink %s\n", ZIGBEE_CRED_FILE);
        }
        zigbee_onboard_status = false;
    }
}
#endif

#if (ONBOARDED_OPERATION_MODE & OPERATION_MODE_THREAD)
static void reset_thread_onboard_status(void)
{
    if (thread_onboard_status)
    {
        if (QAPI_OK != qapi_Fs_Unlink(THREAD_CRED_FILE))
        {
            ONB_WARN("Failed to unlink %s\n", THREAD_CRED_FILE);
        }
        thread_onboard_status = false;
    }
}
#endif

static QCLI_Command_Status_t reset_onboard_info(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{

    qurt_signal_set(&monitor_sigevent,SIGNAL_MONITOR_EVENT_EXIT);
#if (ONBOARDED_OPERATION_MODE & OPERATION_MODE_WIFI)
    reset_wifi_onboard_status();
#endif
#if (ONBOARDED_OPERATION_MODE & OPERATION_MODE_ZIGBEE)
    reset_zigbee_onboard_status();
#endif
#if (ONBOARDED_OPERATION_MODE & OPERATION_MODE_THREAD)
    reset_thread_onboard_status();
#endif
#if ONBOARD_VIA_WIFI
    wlan_disable();
#elif ONBOARD_VIA_BLE
    //TODO: Disable BLE STACK /Disable Zigbee stack
#endif

    qurt_thread_sleep(100);
    /* Reset the system */
    qapi_System_Reset();

    return QCLI_STATUS_SUCCESS_E;
}
#endif

uint16_t get_zigbee_mode()
{
    uint16_t zigbee_mode = SUPPORTED_MODE;
    return zigbee_mode;
}

uint16_t get_thread_mode()
{
    uint16_t thread_mode = SUPPORTED_MODE;
    return thread_mode;
}


char zigbee_mode()
{
    return zigbee_dev_info.mode;
}

char thread_mode()
{
    return thread_dev_info.mode;
}

void print_heap_status(void)
{
    uint32_t total;
    uint32_t free;
    if(qapi_Heap_Status(&total, &free) != QAPI_OK) {
        ONB_INFO("Error getting heap status\n");
        return;
    }

    ONB_INFO("           total       used       free\n");
    ONB_INFO("Heap:   %8d   %8d   %8d\n", total, total-free, free);

}

void update_onboard_led(void)
{
    uint32_t onboarded_radios = 0;

    onboarded_radios |= wifi_onboard_status ? OPERATION_MODE_WIFI : 0;
    onboarded_radios |= zigbee_onboard_status ? OPERATION_MODE_ZIGBEE : 0;

    if (onboarded_radios == ONBOARDED_OPERATION_MODE)
    {
        // All radio are onboarded. Indicate with Constant GREEN LED
        GREEN_LED_CONFIG(1, 100);
        RED_LED_CONFIG(0, 0);
    }
    else if (onboarded_radios)
    {
        // Subset of radios are onboarded. Indicate with BLINK GREEN LED
        GREEN_LED_CONFIG(1, 50);
        RED_LED_CONFIG(0, 0);
    }
    else
    {
        RED_LED_CONFIG(1, 100);
    }

    return;
}

/*---------------------------------------------------------------------------
  - print_banner
  --------------------------------------------------------------------------*/
void print_banner(void)
{
    LOG_INFO("\n\n/*---------------------------------------------------------------*/\n");
    LOG_INFO("\t\tImage build time: %s\n", __TIMESTAMP__);
    LOG_INFO("\t\tChipset version : %s\n", CHIPSET_VARIANT);
    LOG_INFO("\t\tRTOS            : %s\n", RTOS);
#ifdef ONBOARD_VIA_WIFI
    LOG_INFO("\t\tOnboarding VIA   : WIFI\n");
#elif ONBOARD_VIA_BLE
    LOG_INFO("\t\tOnboarding VIA   : BLE\n");
#endif

    LOG_INFO("\t\t Onboardable Radios:\n");
    if (ONBOARDED_OPERATION_MODE & OPERATION_MODE_WIFI)
        LOG_INFO("\t\t\tWIFI\n");
    if (ONBOARDED_OPERATION_MODE & OPERATION_MODE_ZIGBEE)
        LOG_INFO("\t\t\tZIGBEE\n");
    if (ONBOARDED_OPERATION_MODE & OPERATION_MODE_THREAD)
        LOG_INFO("\t\t\tTHREAD\n");
    LOG_INFO("/*-----------------------------------------------------------------*/\n");
}

/*---------------------------------------------------------------------------
  - Create  thread
  --------------------------------------------------------------------------*/
static int32_t create_thread(const char *thr_name, int32_t thr_prio, uint32_t thr_stack_size,
        void (*thread_func)(void *param))
{
    uint32_t len = 0;
    qurt_thread_attr_t attr;

    qurt_thread_t thid = 0;
    qurt_thread_attr_init(&attr);
    qurt_thread_attr_set_name(&attr, thr_name);
    qurt_thread_attr_set_priority(&attr, thr_prio);
    qurt_thread_attr_set_stack_size(&attr, thr_stack_size);
    if( 0 != qurt_thread_create(&thid, &attr, thread_func, &len))
    {
        ONB_ERROR("\nThread creation is failed\n");
        return FAILURE;
    }

    return SUCCESS;
}

#if (ONBOARDED_OPERATION_MODE & OPERATION_MODE_THREAD)
/**
 * @func  : read_thread_config
 * @breif : reads the THREAD credential stored in the file
 */
static int32_t read_thread_config(void)
{
    int fd;
    uint32_t bytes_read = 0;
    struct qapi_fs_stat_type fstat;

    ONB_VERBOSE("%s\n", __func__);
    if (QAPI_OK != qapi_Fs_Stat(THREAD_CRED_FILE, &fstat))
    {
        ONB_VERBOSE("Onboard Credentials Not stored\n");
        return SUCCESS;
    }

    if (fstat.st_size == 0)
    {
        ONB_VERBOSE("Onboard Credentials is NULL\n");
        return SUCCESS;
    }

    if (QAPI_ERROR == qapi_Fs_Open(THREAD_CRED_FILE, QAPI_FS_O_RDONLY, &fd))
    {
        ONB_VERBOSE("Onboard Credentials Not stored\n");
        return SUCCESS;
    }
    memset(&thread_dev_info, 0, sizeof(THREAD_info_t));

    (void)qapi_Fs_Read(fd, (uint8_t *)&thread_dev_info.mode, sizeof(thread_dev_info.mode), &bytes_read);
    if (bytes_read != sizeof(thread_dev_info.mode))
    {
        ONB_WARN("File seems to be corrupted. Onboard device again !!!\n");
        goto end;
    }
    ONB_VERBOSE("No of bytes read:%d\n", bytes_read);

    bytes_read = 0;
    (void)qapi_Fs_Read(fd, (uint8_t *)thread_dev_info.passphrase, MAX_THREAD_PASSPHRASE_SIZE, &bytes_read);
    if (bytes_read != MAX_THREAD_PASSPHRASE_SIZE)
    {
        ONB_WARN("File seems to be corrupted. Onboard device again !!!\n");
        goto end;
    }

    ONB_VERBOSE("No of bytes read:%d\n", bytes_read);
    ONB_VERBOSE("Thread: mode:%c\t Passphrase:%s\n", thread_dev_info.mode, thread_dev_info.passphrase);

    // Set the Thread onboard status
    thread_onboard_status = 1;
    qapi_Fs_Close(fd);

    return SUCCESS;
end:
    qapi_Fs_Close(fd);
    qapi_Fs_Unlink(THREAD_CRED_FILE);

    return SUCCESS;
}
#endif

#if (ONBOARDED_OPERATION_MODE & OPERATION_MODE_ZIGBEE)
/**
 * @func  : read_zigbee_config
 * @breif : reads the ZB credential stored in the file
 */
static int32_t read_zigbee_config(void)
{
    int fd;
    uint32_t bytes_read = 0;
    struct qapi_fs_stat_type fstat;

    ONB_VERBOSE("%s\n", __func__);
    if (QAPI_OK != qapi_Fs_Stat(ZIGBEE_CRED_FILE, &fstat))
    {
        ONB_VERBOSE("Onboard Credentials Not stored\n");
        return SUCCESS;
    }

    if (fstat.st_size == 0)
    {
        ONB_VERBOSE("Onboard Credentials is NULL\n");
        return SUCCESS;
    }

    if (QAPI_ERROR == qapi_Fs_Open(ZIGBEE_CRED_FILE, QAPI_FS_O_RDONLY, &fd))
    {
        ONB_VERBOSE("Onboard Credentials Not stored\n");
        return SUCCESS;
    }
    memset(&zigbee_dev_info, 0, sizeof(ZIGBEE_info_t));

    (void)qapi_Fs_Read(fd, (uint8_t *)&zigbee_dev_info.mode, sizeof(zigbee_dev_info.mode), &bytes_read);
    if (bytes_read != sizeof(zigbee_dev_info.mode))
    {
        ONB_WARN("File seems to be corrupted. Onboard device again !!!\n");
        goto end;
    }
    ONB_VERBOSE("No of bytes read:%d\n", bytes_read);
    bytes_read = 0;
    (void)qapi_Fs_Read(fd, (uint8_t *)zigbee_dev_info.master_key, MAX_LINK_KEY_SIZE, &bytes_read);
    if (bytes_read != MAX_LINK_KEY_SIZE)
    {
        ONB_WARN("File seems to be corrupted. Onboard device again !!!\n");
        goto end;
    }
    ONB_VERBOSE("No of bytes read:%d\n", bytes_read);
    ONB_VERBOSE("ZB: mode:%c\t LinkKey:%s\n", zigbee_dev_info.mode, zigbee_dev_info.master_key);

    // Set the Zigbee onboard status
    zigbee_onboard_status = true;
    qapi_Fs_Close(fd);

    return SUCCESS;

end:
    qapi_Fs_Close(fd);
    qapi_Fs_Unlink(ZIGBEE_CRED_FILE);

    return SUCCESS;
}
#endif

#if defined BOARD_SUPPORTS_WIFI && (ONBOARDED_OPERATION_MODE & OPERATION_MODE_WIFI)
/*----------------------------------------------------------------------------
  - call_back function for Dhcp client connection success
  ----------------------------------------------------------------------------*/
static int32_t dhcpcv4_success_cb(uint32_t addr, uint32_t mask, uint32_t gw)
{

    ONB_INFO("DHCPv4c: IP=%s  Subnet Mask=%s  Gateway=%s\n",
            inet_ntop(AF_INET, &addr, sta_info.ip, sizeof(sta_info.ip)),
            inet_ntop(AF_INET, &mask, sta_info.netmask, sizeof(sta_info.netmask)),
            inet_ntop(AF_INET, &gw, sta_info.gw_ip, sizeof(sta_info.gw_ip)));

    // Constant on
    WLAN_LED_CONFIG(1, 100);
	
	qurt_signal_set(&monitor_sigevent, SIGNAL_MONITOR_EVENT_UNBLOCK);
    qurt_thread_sleep(100);

    return SUCCESS;
}


static int32_t read_wifi_config(void)
{
    int fd;
    uint32_t bytes_read = 0;
    struct qapi_fs_stat_type fstat;

    ONB_VERBOSE("%s\n", __func__);

    if (QAPI_OK != qapi_Fs_Stat(WLAN_STA_CRED_FILE, &fstat))
    {
        ONB_VERBOSE("Onboard Credentials Not stored\n");
        return SUCCESS;
    }

    if (fstat.st_size == 0)
    {
        ONB_VERBOSE("Onboard Credentials is NULL\n");
        return SUCCESS;
    }

    if (QAPI_ERROR == qapi_Fs_Open(WLAN_STA_CRED_FILE, QAPI_FS_O_RDONLY, &fd))
    {
        ONB_VERBOSE("Onboard Credentials Not stored\n");
        return SUCCESS;
    }

    memset(&sta_info, 0, sizeof(sta_info));
    (void)qapi_Fs_Read(fd, (uint8_t *)sta_info.ssid, MAX_SSID_LEN, &bytes_read);
    if (bytes_read != MAX_SSID_LEN)
    {
        ONB_WARN("File seems to be corrupted. Reset device !!!\n");
        goto end;
    }
    ONB_VERBOSE("Read bytes:%d\n", bytes_read);
    bytes_read = 0;
    (void)qapi_Fs_Read(fd, (uint8_t *)sta_info.password, MAX_PASSPHRASE_LEN, &bytes_read);
    if (bytes_read != MAX_PASSPHRASE_LEN)
    {
        ONB_WARN("File seems to be corrupted. Reset device !!!\n");
        goto end;
    }
    ONB_VERBOSE("Read bytes:%d\n", bytes_read);
    ONB_VERBOSE("SSid : %s\n Password:%s\n", sta_info.ssid, sta_info.password);

    // Set the Wi-Fi onboard status
    wifi_onboard_status = true;
    qapi_Fs_Close(fd);

    return SUCCESS;

end:
    qapi_Fs_Close(fd);
    qapi_Fs_Unlink(WLAN_STA_CRED_FILE);

    return SUCCESS;
}

/*---------------------------------------------------------------------------
  - config_sta: Connect STA interface to an AP configured
  --------------------------------------------------------------------------*/
static int32_t config_sta(void)
{
    uint32_t sig_mask = 0;
    uint32_t rised_signal = 0;
    char *passpharse = NULL;

    WLAN_LED_CONFIG(0, 0);

    // Set the Operating mode
    if (FAILURE == wlan_set_operate_mode(DEVID_STA))
    {
        ONB_WARN("Failed to set Operating mode\n");
    }

    if (strlen(sta_info.password))
        passpharse = sta_info.password;
    else
        passpharse = NULL;

    if (SUCCESS == wlan_connect_to_network(DEVID_STA, sta_info.ssid, passpharse))
    {
        ONB_INFO("Connecting to :%s\n", sta_info.ssid);
    }
    else
    {
        ONB_ERROR("Failed connecting to %s!!!\n", sta_info.ssid);
        return FAILURE;
    }

    sig_mask = STA_SIGNAL_CONNECT_EVENT;

    // Blink WLAN LED to indicate connection in progress
    WLAN_LED_CONFIG(1, 50);

    // Wait for STA to get connected to AP
    ONB_VERBOSE("Waiting for connection success event\n");
    if (QURT_EOK != qurt_signal_wait_timed(&wlan_sigevent, sig_mask, (QURT_SIGNAL_ATTR_WAIT_ANY |
                    QURT_SIGNAL_ATTR_CLEAR_MASK), &rised_signal,
                qurt_timer_convert_time_to_ticks(WIFI_CONNECT_TIMEOUT, QURT_TIME_MSEC)))
    {
        ONB_ERROR("%s:Failed on signal time_wait\n", __func__);
        ONB_ERROR("Failed connecting to %s!!!\n", sta_info.ssid);
    }

    if (rised_signal&STA_SIGNAL_CONNECT_EVENT)
    {
        ONB_INFO("Get DHCP addr\n");
        if (dhcpv4_client(INTF_STA, dhcpcv4_success_cb) != 0)
        {
            ONB_ERROR("ERROR:DHCPv4 failed\n");
            return FAILURE;
        }
        qurt_thread_sleep(20);
    }
    return SUCCESS;
}

boolean is_wifi_onboarded(void)
{
    return !!wifi_onboard_status;
}

void Shutdown_thread_network(void)
{
    Thread_Shutdown();
}

void Start_thread_network(void)
{
    qurt_signal_set(&onboard_sigevent, SIGNAL_ONBOARD_EVENT_THREAD);
}
#endif

uint64_t GetExtenAddr()
{
    uint64_t mac_value=0;
    uint8_t mac[6];

#ifdef ONBOARD_VIA_WIFI
    if (!wlan_get_device_mac_address(DEVID_SAP,(char *)mac))
    {
        mac_value = ( mac_value | mac[5]) | ( mac_value | (mac[4]<<8)) | ( mac_value | (mac[3]<<16)) | (mac_value | (mac[2] <<24));
        ONB_VERBOSE("Mac Value %x\n", mac_value);
    }
#endif

#ifdef ONBOARD_VIA_BLE
    if (!ble_get_device_mac_address(mac))
    {
        mac_value = (mac_value | mac[5]) | (mac_value | (mac[4] << 8)) | (mac_value | (mac[3] << 16)) | (mac_value | (mac[2] << 24));
        ONB_VERBOSE("Mac Value %x\n", mac_value);
    }
#endif
    return mac_value;
}

int32_t config_zigbee()
{
    int rtn = FAILURE;
    ONB_VERBOSE("%s\n", __func__);

    if((zigbee_dev_info.mode != '\0') && zigbee_dev_info.master_key != NULL)
    {
        rtn = Start_zigbee(zigbee_dev_info.mode, zigbee_dev_info.master_key);
        if (rtn == SUCCESS)
        {
            ONB_INFO("Zigbee is configured successfully\n");
            return rtn;
        }
    }
    else
        ONB_ERROR("zigbee_dev_info Error\n");

    return FAILURE;
}

int32_t config_thread()
{
    int rtn = FAILURE;


    if((thread_dev_info.mode != '\0') && thread_dev_info.passphrase != NULL)
    {
        rtn = Start_thread(thread_dev_info.mode, thread_dev_info.passphrase);
        if (rtn == SUCCESS)
        {
            ONB_INFO("Thread is configured successfully\n");
            return rtn;
        }
    }
    else
        ONB_ERROR("thread_dev_info Error\n");

    return FAILURE;
}

boolean is_zigbee_onboarded(void)
{
    return !!zigbee_onboard_status;
}

boolean is_thread_onboarded(void)
{
    return !!thread_onboard_status;
}

#if defined BOARD_SUPPORTS_WIFI && (ONBOARDED_OPERATION_MODE & OPERATION_MODE_WIFI)
int32_t set_wlan_sta_info(const char *sta_ssid, const char *sta_password)
{
    uint32_t bytes_written = 0;
    int fd;
    int ret;

    if (!sta_ssid)
    {
        ONB_ERROR("SSID is NULL !!!!\n");
        return FAILURE;
    }

    memset(&sta_info, 0, sizeof(WLAN_info_t));
    strcpy(sta_info.ssid, sta_ssid);

    if (sta_password)
        strcpy(sta_info.password, sta_password);

    ret = qapi_Fs_Open(WLAN_STA_CRED_FILE, QAPI_FS_O_RDWR | QAPI_FS_O_CREAT, &fd);
    if (ret != QAPI_OK)
    {
        ONB_ERROR("File creation is failed !!!!\n");
        return FAILURE;
    }

    qapi_Fs_Write(fd, (uint8_t *)sta_info.ssid, MAX_SSID_LEN, &bytes_written);
    ONB_VERBOSE("bytes_written num = %d\n", bytes_written);
    if (!bytes_written)
    {
        ONB_ERROR("Failed to store SSID config\n");
        qapi_Fs_Close(fd);
        return FAILURE;
    }
    bytes_written = 0;
    qapi_Fs_Write(fd, (uint8_t *)sta_info.password, MAX_PASSPHRASE_LEN, &bytes_written);
    ONB_VERBOSE("bytes_written num = %d\n", bytes_written);

    qapi_Fs_Close(fd);

    if (!bytes_written)
    {
        ONB_ERROR("Failed to store Passwd config\n");
        return FAILURE;
    }

    wifi_onboard_status = true;
    qurt_signal_set(&onboard_sigevent, SIGNAL_ONBOARD_EVENT_WIFI);

    return SUCCESS;
}
#endif

int32_t set_zigbee_info(uint8_t mode, uint8_t *master_key)
{
    uint32_t bytes_written;
    int fd;
    int ret;

    memset(&zigbee_dev_info, 0, sizeof(ZIGBEE_info_t));
    zigbee_dev_info.mode = mode;
    memcpy(zigbee_dev_info.master_key,master_key, MAX_LINK_KEY_SIZE);

    ONB_VERBOSE("Op_Mode:%c \n", zigbee_dev_info.mode);
    ONB_VERBOSE("Master_key=%s\n", zigbee_dev_info.master_key);

    ret = qapi_Fs_Open(ZIGBEE_CRED_FILE, QAPI_FS_O_RDWR | QAPI_FS_O_CREAT, &fd);
    if (ret != QAPI_OK)
    {
        ONB_ERROR("File creation is failed !!!!\n");
        return FAILURE;
    }

    qapi_Fs_Write(fd, (uint8_t *)&(zigbee_dev_info.mode), sizeof(zigbee_dev_info.mode), &bytes_written);
    ONB_VERBOSE("bytes_written num = %d\n", bytes_written);
    if (!bytes_written)
    {
        ONB_ERROR("Failed to store Zigbee config\n");
        qapi_Fs_Close(fd);
        return FAILURE;
    }

    qapi_Fs_Write(fd, (uint8_t *)zigbee_dev_info.master_key, MAX_LINK_KEY_SIZE, &bytes_written);
    ONB_VERBOSE("bytes_written num = %d\n", bytes_written);

    qapi_Fs_Close(fd);

    if (!bytes_written)
    {
        ONB_ERROR("Failed to store Zigbee config\n");
        return FAILURE;
    }

    zigbee_onboard_status = true;
    qurt_signal_set(&onboard_sigevent, SIGNAL_ONBOARD_EVENT_ZIGBEE);

    return SUCCESS;
}

int32_t set_thread_info(uint8_t mode, uint8_t *passphrase)
{
    uint32_t bytes_written;
    int fd;
    int ret;

    memset(&thread_dev_info, 0, sizeof(THREAD_info_t));
    thread_dev_info.mode = mode;
    memcpy(thread_dev_info.passphrase, passphrase, MAX_THREAD_PASSPHRASE_SIZE);

    ONB_VERBOSE("Op_Mode:%c \n", thread_dev_info.mode);
    ONB_VERBOSE("Master_key=%s\n", thread_dev_info.passphrase);

    ret = qapi_Fs_Open(THREAD_CRED_FILE, QAPI_FS_O_RDWR | QAPI_FS_O_CREAT, &fd);
    if (ret != QAPI_OK)
    {
        ONB_ERROR("File creation is failed !!!!\n");
        return FAILURE;
    }

    qapi_Fs_Write(fd, (uint8_t *)&(thread_dev_info.mode), sizeof(thread_dev_info.mode), &bytes_written);
    ONB_VERBOSE("bytes_written num = %d\n", bytes_written);
    if (!bytes_written)
    {
        ONB_ERROR("Failed to store Zigbee config\n");
        qapi_Fs_Close(fd);
        return FAILURE;
    }

    qapi_Fs_Write(fd, (uint8_t *)thread_dev_info.passphrase, MAX_THREAD_PASSPHRASE_SIZE, &bytes_written);
    ONB_VERBOSE("bytes_written num = %d\n", bytes_written);

    qapi_Fs_Close(fd);

    if (!bytes_written)
    {
        ONB_ERROR("Failed to store Zigbee config\n");
        return FAILURE;
    }

    thread_onboard_status = true;
    qurt_signal_set(&onboard_sigevent, SIGNAL_ONBOARD_EVENT_THREAD);

    return SUCCESS;
}

void zb_raise_rejoin_event(void)
{
    set_rejoin_status();
    qurt_signal_set(&monitor_sigevent, SIGNAL_REJOIN_EVENT_ZIGBEE);
}

void process_onboard_event(void *param)
{
    uint32_t sig_mask = 0;
    uint32_t rised_signal = 0;

    if (ONBOARDED_OPERATION_MODE & OPERATION_MODE_WIFI)
        sig_mask |= SIGNAL_ONBOARD_EVENT_WIFI;
    if (ONBOARDED_OPERATION_MODE & OPERATION_MODE_THREAD)
    {
        sig_mask |= SIGNAL_ONBOARD_EVENT_THREAD;
    }
    if (ONBOARDED_OPERATION_MODE & OPERATION_MODE_ZIGBEE)
    {
        sig_mask |= SIGNAL_ONBOARD_EVENT_ZIGBEE;
    }
        sig_mask |= PIR_THREAD_SIGNAL_INTR;

    while (1)
    {
        rised_signal = 0;

        ONB_INFO("\nWaiting for Onboard events ...\n");

        update_onboard_led();

        //TODO: Add logic to exit
        if (QURT_EOK != qurt_signal_wait_timed(&onboard_sigevent, sig_mask, (QURT_SIGNAL_ATTR_WAIT_ANY |
                        QURT_SIGNAL_ATTR_CLEAR_MASK), &rised_signal, QURT_TIME_WAIT_FOREVER))
        {
            ONB_ERROR("%s:Failed on signal time_wait\n", __func__);
        }
        ONB_INFO("Rised signal: %d\n", rised_signal);
        qurt_mutex_lock(&onboard_lock);
#if defined BOARD_SUPPORTS_WIFI && (ONBOARDED_OPERATION_MODE & OPERATION_MODE_WIFI)
        if (rised_signal & SIGNAL_ONBOARD_EVENT_WIFI)
        {
            ONB_INFO("Onboard wifi event\n");
            if (SUCCESS != config_sta())
            {
                ONB_ERROR("\nFailed to config STA\n");
            }
        }
#endif
        if (!zigbee_onboard_status && (rised_signal & SIGNAL_ONBOARD_EVENT_THREAD))
        {
            ONB_INFO("Onboard thread event\n");
            if (SUCCESS != config_thread())
            {
                ONB_ERROR("\n  :Failed to config Thread\n");
            }
        }

        if (!thread_onboard_status && (rised_signal & SIGNAL_ONBOARD_EVENT_ZIGBEE))
        {
            if (SUCCESS != config_zigbee())
            {
                ONB_ERROR("\nFailed to config Zigbee\n");
            }
        }
        if (rised_signal & PIR_THREAD_SIGNAL_INTR)
        {
            ONB_INFO("pir signal reached\n");
	    //BLUE_LED_CONFIG(1, 50);
#ifdef THREAD
            if (thread_onboard_status && SUCCESS != Thread_PIR_Data_Send())
            {
                ONB_ERROR("\nFailed to send PIR data\n");
            }
#endif

#if OFFLINE
            if (!zigbee_onboard_status)
            {
                Pir_offline_breach_message();
            }
#endif
            if (zigbee_mode() == 'e' || zigbee_mode() == 'E')
            {
                if (zigbee_onboard_status && SUCCESS != Zigbee_PIR_Data_Send())
                {
                    ONB_ERROR("\nFailed to send PIR data\n");
                }
            }
        }


        qurt_mutex_unlock(&onboard_lock);
    }
}

/**
 * @brief This thread is used to monitor the status of the device.
 *
 * This function Initiate the connections if device
 * is configured but not connected.
 *      Eg. If Wi-Fi cred is available and device is not connected, initiates
 *         Wi-Fi connection
 */
static void process_monitor_event(void *param)
{
    uint32_t sig_mask = 0;
    uint32_t rised_signal = 0;
    int32_t ret;
#if defined BOARD_SUPPORTS_WIFI && (ONBOARDED_OPERATION_MODE & OPERATION_MODE_WIFI)
    int32_t rc;
    static uint32_t count_ip_lost = 0;
#endif
    qurt_time_t timeout;

    ONB_INFO("Monitor Thread is runnning ----------------------\n");

    sig_mask = SIGNAL_MONITOR_EVENT_INTERFACES | SIGNAL_MONITOR_EVENT_EXIT | SIGNAL_REJOIN_EVENT_ZIGBEE | SIGNAL_MONITOR_EVENT_UNBLOCK;
    timeout = qurt_timer_convert_time_to_ticks(MONITOR_THREAD_TIMEOUT, QURT_TIME_MSEC);

    while (1)
    {
        ONB_INFO("waiting on Monitor thread\n");
        rised_signal = 0;

        ret = qurt_signal_wait_timed(&monitor_sigevent, sig_mask, (QURT_SIGNAL_ATTR_WAIT_ANY |
                    QURT_SIGNAL_ATTR_CLEAR_MASK), &rised_signal, timeout);
        if (ret == QURT_EINVALID)
        {
            ONB_ERROR("%s:Failed on signal timewait\n", __func__);

        }
        if (rised_signal & SIGNAL_MONITOR_EVENT_EXIT)
        {
            ONB_INFO("Monitor thread exiting\n");
            qurt_thread_stop();
            break;
        }

#if OFFLINE       
        if (zigbee_dev_info.mode == 'e' || zigbee_dev_info.mode == 'E')
        {
            timeout = qurt_timer_convert_time_to_ticks(2000, QURT_TIME_MSEC); 
        }
#endif

        qurt_mutex_lock(&onboard_lock);
#if OFFLINE
        if (zigbee_dev_info.mode == 'c' || zigbee_dev_info.mode == 'C')
        {
            check_zigbee_devices_state();
        }
#endif
        update_onboard_led();

#if defined BOARD_SUPPORTS_WIFI && (ONBOARDED_OPERATION_MODE & OPERATION_MODE_WIFI)
        if (wifi_onboard_status)
        {
            if (SUCCESS == wlan_is_device_connected(DEVID_STA))
            {
                rc = iface_has_ipaddress(INTF_STA);
                if (rc == SUCCESS)
                {
                    ONB_VERBOSE("Iface has the IP addr\n");
#if AWS_IOT
                    if(!is_aws_running())
                    {
                        if(!is_sntp_started())
                        {
                            start_sntpc();
                            add_sntp_svr(SNTPC_SVR_ADDR);
                            timeout = qurt_timer_convert_time_to_ticks(10, QURT_TIME_MSEC);
                            qurt_mutex_unlock(&onboard_lock);
                            continue;
                        }
                        timeout = qurt_timer_convert_time_to_ticks(MONITOR_THREAD_TIMEOUT, QURT_TIME_MSEC);
                        if(!resolve_ip_address(HOST_ID))
                        {
                            Start_aws();

                        }
                        else
                        {
                            qurt_thread_sleep(1000);
                            ONB_VERBOSE("Start_dns_cline\n");
                            start_dns_client();
                            ONB_VERBOSE("Stop_dns_cline\n");
                            qurt_thread_sleep(2000);
                            if(!add_dns_svr_list())
                            {
                                if(!resolve_ip_address(HOST_ID))
                                {
                                    Start_aws();
                                }
                            }
                            else
                            {
                                ONB_VERBOSE("Add Dns server list is failed\n");
                            }
                        }
                    }
#endif
                }
                else
                {
                    if(rc == -1)
                        count_ip_lost ++;
                    if (count_ip_lost == 5)
                        restart_device();

                    WLAN_LED_CONFIG(0,0);
                    dhcp_release(INTF_STA);
                    // Start DHCP Client. Starting of AWS is taken care in the callback
                    ONB_INFO("Getting ip from dhcp server ......... \n");
                    if (dhcpv4_client(INTF_STA, dhcpcv4_success_cb) != 0)
                    {
                        ONB_ERROR("ERROR:DHCPv4 failed\n");
                    }
                }
            }
            else
            {
                WLAN_LED_CONFIG(0,0);
                dhcp_release(INTF_STA);
                // Disconnect from current
                wlan_disconnect_from_network(DEVID_STA);
                qurt_thread_sleep(50);
#if AWS_IOT
                Stop_aws();
#endif
                ONB_VERBOSE("Signal for onboarding Wi-Fi\n");
                qurt_signal_set(&onboard_sigevent, SIGNAL_ONBOARD_EVENT_WIFI);
            }
        }
#endif
        if (zigbee_dev_info.mode == 'e' || zigbee_dev_info.mode == 'E')
        {
            if (rised_signal & SIGNAL_REJOIN_EVENT_ZIGBEE)
            {
                if (SUCCESS != rejoin_zigbee(zigbee_dev_info.mode, zigbee_dev_info.master_key))
                {
                    ONB_ERROR("\nFailed to Rejoin Zigbee\n");
                }
            }
            if (zigbee_onboard_status && (SUCCESS == Get_Enddev_Join_Confirm_Status()))
            {

                ONB_VERBOSE("Reading End device Sensors\n");
                Zigbee_read_sensors();
            }
        }
        if (thread_dev_info.mode == 'j' || thread_dev_info.mode == 'J')
        {
            if (is_thread_onboarded() && is_recv_thread_stopped())
            {
                Thread_Udp_Joiner();
            }

            if(thread_onboard_status && (SUCCESS == Get_Joiner_Confirm_Status()))
            {

                ONB_VERBOSE("Reading Joiner device Sensors\n");
                Thread_read_sensors();
            }
        }
        print_heap_status();
        LOG_INFO("mutex unlock done ---------\n");
        qurt_thread_sleep(10);
        qurt_mutex_unlock(&onboard_lock);
    }

    return;
}

/**
 * @brief Starts the onboarding process.
 *
 * This function is called from the app_start(). This func
 * starts the onboard link for allowing the Mobile APPs
 * to configure/onboard the radios.
 */
void Start_onboard_demo(void)
{

#if defined BOARD_SUPPORTS_WIFI && (ONBOARDED_OPERATION_MODE & OPERATION_MODE_WIFI)
    /** Read stored wifi configuration */
    if (FAILURE == read_wifi_config())
    {
        ONB_ERROR("Read wifi config status is failed\n");
        goto error;
    }
#endif

#if (ONBOARDED_OPERATION_MODE & OPERATION_MODE_ZIGBEE)
    /** Read stored zigbee configuration */
    if (FAILURE == read_zigbee_config())
    {
        ONB_ERROR("Read zigbee config status is failed\n");
        goto error;
    }
#endif
#if (ONBOARDED_OPERATION_MODE & OPERATION_MODE_THREAD)
    if (FAILURE == read_thread_config())
    {
        ONB_ERROR("Read thread config status is failed\n");
        goto error;
    }
#endif
#if defined BOARD_SUPPORTS_WIFI && (ONBOARDED_OPERATION_MODE & OPERATION_MODE_WIFI)
#ifdef AWS_IOT
    if (FAILURE == Initialize_aws())
    {
        ONB_ERROR("AWS thread Intialization Failed !!\n");
        goto error;
    }

#endif
#endif
    if (FAILURE == Initialize_sensors_handle())
    {
        ONB_ERROR("Sensors Initialization failed !!\n");
        goto error;
    }
    if (FAILURE == Intial_sensor_values())
    {
        ONB_ERROR("Writing data into thermo_stat is failed\n");
        goto error;
    }

#ifndef OFFLINE
#ifdef ONBOARD_VIA_WIFI
    if (FAILURE == Start_onboard_via_wifi())
    {
        ONB_ERROR("Failed to Create SAP link\n");
        goto error;
    }
#endif
#endif

#ifdef ONBOARD_VIA_BLE
    if (FAILURE == Start_onboard_via_ble())
    {
        ONB_ERROR("Failed to Create SAP link\n");
        goto error;
    }
#endif
    update_onboard_led();

    if (FAILURE == create_thread("ProcessOnboard", ONBOARD_THRD_PRIO, ONBOARD_STACK_SIZE, process_onboard_event))
    {
        ONB_ERROR("Failed to create Onboard Thread\n");
        goto error;
    }

    /** Set the onboard signal based on wifi onboard details/status */
    if (wifi_onboard_status)
    {
        qurt_signal_set(&onboard_sigevent, SIGNAL_ONBOARD_EVENT_WIFI);
    }

    if (zigbee_onboard_status)
    {
        qurt_signal_set(&onboard_sigevent, SIGNAL_ONBOARD_EVENT_ZIGBEE);
    }

    if (thread_onboard_status)
    {
        qurt_signal_set(&onboard_sigevent, SIGNAL_ONBOARD_EVENT_THREAD);
    }

    if (FAILURE == create_thread("Monitor thrd\n", MONITOR_THREAD_PRIO, MONITOR_STACK_SIZE, process_monitor_event))
    {
        ONB_WARN("Failed to create Monitor thread\n");
    }

    return;

error:
    /** Something is wrong Blink RED LED to indicate there is an error */
    RED_LED_CONFIG(1, 50);
    return;
}

/**
 * @brief Initialization function for the demo.
 *
 * This function is called from the app_init(). This func
 * initializes onboard link for allowing the Mobile APPs
 * to configure/onboard the radios.
 */
void Initialize_onboard_demo(void)
{
    /** Initialize available LED's */
    led_initialize();

    qcli_onboard = QCLI_Register_Command_Group(NULL, &onboard_cmd_group);
    if (NULL == qcli_onboard)
    {
        ONB_ERROR("QCLI_Register_Command_Group: failed\n");
        goto end;
    }

    /** Create Signal object for onboard events */
    if (QURT_EOK != qurt_signal_init(&onboard_sigevent))
    {
        ONB_ERROR("qurt_signal_create event is failed\n");
        goto end;
    }

    /** Create Signal object for monitor events */
    if (QURT_EOK != qurt_signal_init(&monitor_sigevent))
    {
        ONB_ERROR("qurt_signal_create event is failed\n");
        goto end;
    }
    /**Create mutex object for synchronization b/w process_monitor event and
      Onboarding thread */
    if (QURT_EOK != qurt_mutex_create(&onboard_lock))
    {
        ONB_ERROR("qurt_mutex object creation is failed\n");
        goto end;
    }


#ifdef ONBOARD_VIA_WIFI
    /** Initialize for Onboarding of device Via Wi-Fi */
    if (FAILURE == Initialize_onboard_via_wifi())
    {
        ONB_ERROR("Onboard VIA WiFi could not be initialized\n");
        goto end;
    }
#endif
#ifdef ONBOARD_VIA_BLE
    /** Initialize for Onboarding of device Via BLE */
    if (FAILURE == Initialize_onboard_via_ble())
    {
        ONB_ERROR("Onboard VIA BLE could not be initialized\n");
        goto end;
    }
#endif
#if OFFLINE
    if (FAILURE == Start_offline_thread())
    {
        ONB_ERROR("Offline thread creation is failed\n");
        goto end;
    }
#endif
#if OFFLINE
      pir_register_intr();
#endif
    return;

end:
    ONB_ERROR("Restart the device!!!\n");

    /** Something is wrong Blink RED LED to indicate there is an error */
    RED_LED_CONFIG(1, 50);

    return;
}
