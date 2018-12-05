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

#include "malloc.h"
#include "string.h"
#include "stringl.h"
#include "stdarg.h"

#include <qcli_api.h>
#include <qcli_util.h>

#include "qapi_twn.h"
#include <qapi_wlan.h>
#include <qapi/qapi_socket.h>
#include <qapi_netservices.h>
#include <qapi_ns_gen_v4.h>
#include <qapi_ns_utils.h>

#include <qurt_timer.h>
#include <qurt_thread.h>
#include <qurt_error.h>

#include "wifi_util.h"
#include "netutils.h"
#include "log_util.h"

#include "onboard.h"
#include "util.h"

/** User Config */
/** Wi-Fi SAP configuartion */
/** Code will postfix by 2 LSB bytes of MAC addr _xxxx */
#define SAP_SSID                           ONB_INFO_CHIPSET             /**< SAP SSID */
#define SAP_PASSWD                         "123456789"                  /**< SAP password */

// SAP NET configuration
#define SAP_IP                             "192.168.0.1"                /**< SAP Ip address */
#define SAP_GW_IP                          "0.0.0.0"                    /**< Gateway Ip */
#define SAP_SUBNET_MASK                    "255.255.255.0"              /**< SUBNET Mask */
#define SAP_DHCPS_START_POOL               "192.168.0.11"               /**< Dhcp pool start address */
#define SAP_DHCPS_END_POOL                 "192.168.0.20"               /**< Dhcp pool end address */
#define SAP_DHCPS_LEASE_TIME               7200                         /**< Dhcp Lease time */

#define ONB_BUF_SIZE                      256                        /* Buffer size */

#define ONB_SRV_PORT                      6000                         /**< Onboard server port no */
#define ONB_THRD_STACK_SIZE               2048                         /**< Onboard thread stack size */
#define ONB_THRD_PRIO                     10                           /**< Onboard Server Thread Priority */

/*-------------------------------------------------------------------------
  - Preprocessor Definitions and Constants
  ------------------------------------------------------------------------*/

#define ONB_ERROR           LOG_ERROR
#define ONB_WARN            LOG_WARN
#define ONB_INFO            LOG_INFO
#define ONB_VERBOSE         LOG_VERBOSE

// Config from APP
#define ACTION                            "\"Action\":"
#define SSID_CMD                          "\"SSID\":"
#define PASWD_CMD                         "\"Password\":"
#define ZIGBEE_MODE                       "\"Mode\":"
#define ZIGBEE_LINK_KEY                   "\"Linkkey\":"
#define THREAD_MODE                       "\"Mode\":"
#define THREAD_PASSPHRASE                 "\"Passphrase\":"

// Action Key Values
#define HELLO                             "Hello"
#define CONFIG_WIFI                       "ConfigWifi"
#define CONFIG_ZIGBEE                     "ConfigZigbee"
#define CONFIG_THREAD                     "ConfigThread"


// Zibee mode checking

#define CHECK_ZIGBEE_MODE_IS_VALID(ch) \
    if (!(ch == 'c' || ch == 'C' || ch == 'r' || ch == 'R' || ch == 'e' || ch == 'E')) \
{ \
    ONB_ERROR("zigbee mode is not valid\n");\
    return FAILURE;\
}

#define CHECK_THREAD_MODE_IS_VALID(ch) \
    if (!(ch == 'b' || ch == 'B' || ch == 'j' || ch == 'J' || ch == 'r' || ch == 'R')) \
{\
    ONB_ERROR("Thread mode is not valid\n");\
    return FAILURE;\
}

#define XSTR(m)      STR(m)
#define STR(m)       #m
#define JSON_START_BLOCK(buf, str_name, str_val) \
    do { \
        strcat(buf, "{"); \
        strcat(buf, str_name); \
        strcat(buf, ":"); \
        strcat(buf, str_val); \
    } while (0);

#define JSON_CREATE_EMPTY_KEY(buf, key) \
    do { \
        strcat(buf, ","); \
        strcat(buf, key); \
        strcat(buf, ":"); \
    } while (0);

#define JSON_STRING(buf,key) \
do {\
    strcat(buf, "\"");\
    strcat(buf,key);\
    strcat(buf,"\"");\
} while (0);

#define JSON_START_VAL(buf) \
    strcat(buf, "\"");

#define JSON_END_VAL(buf) \
    strcat(buf, "\"");

#define JSON_ADD_ENTRY(buf, str_name, str_val) \
do { \
    strcat(buf, ","); \
    strcat(buf, str_name); \
    strcat(buf, ":"); \
    strcat(buf, str_val); \
} while (0);

#define JSON_APPEND_VAL(buf, val) \
do { \
    if (buf[strlen(buf) - 1] != ':' && buf[strlen(buf) -1] != '"') \
        strcat(buf, ";"); \
    strcat(buf, val); \
} while (0);

#define JSON_END_BLOCK(buf)     strcat(buf,"}");

typedef struct WLAN_info {
    char ssid[MAX_SSID_LEN+1];
    char password[MAX_PASSPHRASE_LEN+1];
    char ip[IPV4ADDR_STR_LEN+1];
    char netmask[IPV4ADDR_STR_LEN+1];
    char gw_ip[IPV4ADDR_STR_LEN+1];
} WLAN_info_t;

static char sta_passwd[MAX_PASSPHRASE_LEN+1];
static char sta_ssid[MAX_SSID_LEN+1];
static char zigbee_dev_mode;
static uint8_t zigbee_linkkey[MAX_LINK_KEY_SIZE +1];
static char thread_dev_mode;
static uint8_t thread_passphrase[MAX_THREAD_PASSPHRASE_SIZE +1];

static WLAN_info_t ap_info;
qurt_signal_t wlan_sigevent;

static char onboarding_buf[ONB_BUF_SIZE+1];
char tmp_buf[33];

static void skip_whitespace(const char *buf, size_t len, const char **ppos)
{
    while (!((*ppos - buf) >= len))
    {
        if ((**ppos == '\r') || **ppos == '\n' || **ppos == ' ' || **ppos == '\t')
            (*ppos)++;
        else
            break;
    }
}

static int match_key(const char *buf, size_t len, const char **ppos, const char *key)
{
    const int key_len = strlen(key);

    skip_whitespace(buf, len, ppos);

    if (((*ppos - buf) + key_len) >= len)
    {
        return FAILURE;
    }

    if (strncmp(*ppos, key, key_len) == 0)
    {
        *ppos += key_len;
        return SUCCESS;
    }
    return FAILURE;
}

static int parse_value(const char *buf, size_t len, const char **ppos, char *key_val, size_t val_len)
{
    skip_whitespace(buf, len, ppos);

    if (**ppos != '"')
        return FAILURE;

    (*ppos)++;

    while (!((*ppos - buf) >= len) && **ppos != '"' && val_len)
    {
        *key_val = **ppos;
        (*ppos)++;
        key_val++;
        val_len--;
    }

    if (((*ppos - buf) >= len) || **ppos != '"')
    {
        return FAILURE;
    }

    // Skip '"'
    (*ppos)++;

    return SUCCESS;
}
static int parse_keyvalue_pair(const char *buf, size_t len, const char **ppos, const char *key, char *key_val, size_t val_len)
{
    skip_whitespace(buf, len, ppos);

    if (match_key(buf, len, ppos, key))
    {
        ONB_ERROR("match key failed\n");
        return FAILURE;
    }

    if (parse_value(buf, len, ppos, key_val, val_len))
    {
        ONB_ERROR("parse value failed\n");
        return FAILURE;
    }

    skip_whitespace(buf, len, ppos);

    if (**ppos != ',' && **ppos != '}')
    {
        ONB_ERROR("if case\n");
        return FAILURE;
    }

    if (**ppos == ',')
        (*ppos)++;

    skip_whitespace(buf, len, ppos);

    return SUCCESS;
}
static int process_action_zigbee(const char *buf, size_t len, const char **ppos)
{
    if (parse_keyvalue_pair(buf, len, ppos, ZIGBEE_MODE, &zigbee_dev_mode, sizeof(zigbee_dev_mode)))
    {
        return FAILURE;
    }

    if (parse_keyvalue_pair(buf, len, ppos, ZIGBEE_LINK_KEY, (char *)zigbee_linkkey, sizeof(zigbee_linkkey)-1))
    {
        return FAILURE;
    }
    CHECK_ZIGBEE_MODE_IS_VALID(zigbee_dev_mode)

    if (**ppos != '}')
        return FAILURE;
    (*ppos)++;

    ONB_VERBOSE("ZIGBEE_MODE:%c\n", zigbee_dev_mode);
    ONB_VERBOSE("ZIGBEE_link_key:%s\n", zigbee_linkkey);

    return SUCCESS;
}

static int process_action_thread(const char *buf, size_t len, const char **ppos)
{
    if (parse_keyvalue_pair(buf, len, ppos, THREAD_MODE, &thread_dev_mode, sizeof(thread_dev_mode)))
    {
        ONB_ERROR("Thread mode failed\n");
        return FAILURE;
    }

    if (parse_keyvalue_pair(buf, len, ppos, THREAD_PASSPHRASE, (char *)thread_passphrase, sizeof(thread_passphrase)-1))
    {
        ONB_ERROR("Thread Passphrase failed\n");
        return FAILURE;
    }
    CHECK_THREAD_MODE_IS_VALID(thread_dev_mode)

    if (**ppos != '}')
        return FAILURE;
    (*ppos)++;

    ONB_VERBOSE("Thread_MODE:%c\n", zigbee_dev_mode);
    ONB_VERBOSE("Thread_passphrase:%s\n", zigbee_linkkey);

    return SUCCESS;
    
}

static int process_action_wifi(const char *buf, size_t len, const char **ppos)
{
    if (parse_keyvalue_pair(buf, len, ppos, SSID_CMD, sta_ssid, sizeof(sta_ssid)-1))
    {
        ONB_ERROR("%d:Parsing failed\n", __LINE__);
        return FAILURE;
    }

    if (parse_keyvalue_pair(buf, len, ppos, PASWD_CMD, sta_passwd, sizeof(sta_passwd)-1))
    {
        ONB_ERROR("%d:Parsing failed\n", __LINE__);
        return FAILURE;
    }

    if (**ppos != '}')
        return FAILURE;
    (*ppos)++;

    ONB_VERBOSE("SSID=%s\n", sta_ssid);
    ONB_VERBOSE("PassWD=%s\n", sta_passwd);

    return SUCCESS;
}


/*----------------------------------------------------------------------------
  -process_actions : Process actions received on the onboard link
        => Process the onboard info received and calls corresponding
        module to be onboarded with the info received
  ---------------------------------------------------------------------------*/
static int process_actions(const char *buf, size_t len)
{
    const char *pos = buf;

    while (!((pos - buf) >= len))
    {
        skip_whitespace(buf, len, &pos);
        if (*pos == '{')
        {
            pos++;
            memset(tmp_buf, 0, sizeof(tmp_buf));
            if (parse_keyvalue_pair(buf, len, &pos, ACTION, tmp_buf, sizeof(tmp_buf)-1))
            {
                return FAILURE;
            }

            if (!strncmp(tmp_buf, HELLO, sizeof(HELLO)))
            {
                return SUCCESS;
            }

            else if (!strncmp(tmp_buf, CONFIG_WIFI, sizeof(CONFIG_WIFI)))
            {
                if (FAILURE == process_action_wifi(buf, len, &pos))
                    return FAILURE;

                if (strlen(sta_passwd))
                    set_wlan_sta_info(sta_ssid, sta_passwd);
                else
                    set_wlan_sta_info(sta_ssid, NULL);
            }

            else if (!strncmp(tmp_buf, CONFIG_ZIGBEE, sizeof(CONFIG_ZIGBEE)))
            {
                if (FAILURE == process_action_zigbee(buf, len, &pos))
                {
                    return  FAILURE;
                }

                set_zigbee_info(zigbee_dev_mode, zigbee_linkkey);
            }
            else if (!strncmp(tmp_buf, CONFIG_THREAD, sizeof(CONFIG_THREAD)))
            {
                 if (FAILURE == process_action_thread(buf, len, &pos))
                {
                    return FAILURE;
                }
                set_thread_info(thread_dev_mode, thread_passphrase); 
            }
        }
        else
        {
            return FAILURE;
        }
    }

    return SUCCESS;
}

/*----------------------------------------------------------------------------
  - call_back function for Dhcp Server
  ----------------------------------------------------------------------------*/
static int32_t dhcpsv4_success_cb(uint8_t *macaddr, uint32_t ipaddr)
{
    if (macaddr != NULL)
    {
        char ip_str[IPV4ADDR_STR_LEN];

        ONB_INFO("DHCPv4s: Client IP=%s  Client MAC=%02x:%02x:%02x:%02x:%02x:%02x\n",
                inet_ntop(AF_INET, &ipaddr, ip_str, sizeof(ip_str)),
                macaddr[0], macaddr[1], macaddr[2], macaddr[3], macaddr[4], macaddr[5]);
    }

    return SUCCESS;
}

static void init_ap_info()
{
    char mac[6] = { 0 };
    char ssid[32] = { 0 };

    strcpy(ssid, SAP_SSID);
    change_to_upper(ssid);

    if (SUCCESS == wlan_get_device_mac_address(DEVID_SAP, mac))
    {
        snprintf(ap_info.ssid, sizeof(ap_info.ssid),"%s_%02x%02x", ssid, mac[4], mac[5]);
    }
    else
    {
        // Could not read MAC addr. Continue to provide the onboard functionality
        ONB_WARN("Could not append MAC to SSID\n");
        strcpy(ap_info.ssid, ssid);
    }
    strcpy(ap_info.password, SAP_PASSWD);
    strcpy(ap_info.ip, SAP_IP);
    strcpy(ap_info.netmask, SAP_SUBNET_MASK);
    strcpy(ap_info.gw_ip, SAP_GW_IP);
}

static int32_t start_sap()
{
    uint32_t signal = 0;
    uint32_t rised_signal = 0;

    signal = (uint32_t) AP_SIGNAL_CONNECT_EVENT;

    /* initialize ap details */
    init_ap_info(ap_info);

    /*set the Operating mode */
    if (FAILURE == wlan_set_operate_mode(DEVID_SAP))
    {
        ONB_ERROR("Failed to set Operating mode\n");
        goto error;
    }

    /* Start the Soft AP */
    if (FAILURE == wlan_connect_to_network(DEVID_SAP, ap_info.ssid, ap_info.password))
    {
        ONB_ERROR("Failed to run SoftAP\n");
        goto error;
    }

    //TODO: Need to provide timeout
    if (QURT_EOK != qurt_signal_wait_timed(&wlan_sigevent, signal,
                (QURT_SIGNAL_ATTR_WAIT_ANY | QURT_SIGNAL_ATTR_CLEAR_MASK),
                &rised_signal, QURT_TIME_WAIT_FOREVER))
    {
        ONB_ERROR("%s:Failed on signal time_wait\n", __func__);
        goto error;
    }

    /* Set the static ip Soft AP */
    if (SUCCESS != ifv4_config(INTF_AP, ap_info.ip, ap_info.netmask, ap_info.gw_ip))
    {
        ONB_ERROR("Failed to set IP to SAP\n");
        goto error;

    }

    /* To run DHCP Server in Soft AP */
    if (FAILURE ==  dhcpv4_server(INTF_AP, SAP_DHCPS_START_POOL, SAP_DHCPS_END_POOL, SAP_DHCPS_LEASE_TIME, dhcpsv4_success_cb))
    {
        ONB_ERROR("Failed to run DHCP Server\n");
        goto error;
    }

    return SUCCESS;

error:
    return FAILURE;

}
static void prepare_json_response(char *buf)
{
    char temp_buf[FW_VER_BUF] = {0};

    get_dev_fw_version(temp_buf, sizeof(temp_buf));
    JSON_START_BLOCK(buf, "\"FwVer\"", temp_buf);

    memset(temp_buf, 0, FW_VER_BUF);
    memcpy(temp_buf, ONB_INFO_CHIPSET, sizeof(ONB_INFO_CHIPSET));
    change_to_upper(temp_buf);

    JSON_ADD_ENTRY(buf, "\"Chipset\"", CHIPSET_VARIANT);

    JSON_ADD_ENTRY(buf, "\"BatMode\"", XSTR(DEVICE_BATTERY_OPERATED));

    JSON_CREATE_EMPTY_KEY(buf, "\"OperationMode\"");
    JSON_START_VAL(buf);
    {
        if(ONBOARDED_OPERATION_MODE & OPERATION_MODE_WIFI)
        {
            JSON_APPEND_VAL(buf, "WIFI");
        }
        if (ONBOARDED_OPERATION_MODE & OPERATION_MODE_THREAD)
        {
            if (!is_zigbee_onboarded())
                JSON_APPEND_VAL(buf, "Thread");
        }
        if (ONBOARDED_OPERATION_MODE & OPERATION_MODE_ZIGBEE)
        {
            if (!is_thread_onboarded())
                JSON_APPEND_VAL(buf, "Zigbee");
        }
    }
    JSON_END_VAL(buf);

    if (ONBOARDED_OPERATION_MODE & OPERATION_MODE_ZIGBEE)
    {
        JSON_CREATE_EMPTY_KEY(buf, "\"ZBModes\"");
        JSON_START_VAL(buf);
        snprintf(&buf[strlen(buf)], sizeof(buf), "%d", get_zigbee_mode());
        JSON_END_VAL(buf);
    }
    
    if (ONBOARDED_OPERATION_MODE & OPERATION_MODE_THREAD)
    {
        JSON_CREATE_EMPTY_KEY(buf, "\"ThreadModes\"");
        JSON_START_VAL(buf);
        snprintf(&buf[strlen(buf)], sizeof(buf), "%d", get_thread_mode());
        JSON_END_VAL(buf);
    }

    JSON_CREATE_EMPTY_KEY(buf, "\"OnBoarded\"");
    JSON_START_VAL(buf);

    {
        if (is_wifi_onboarded())
        {
            JSON_APPEND_VAL(buf, "WIFI");
        }

        if (is_zigbee_onboarded())
        {
            JSON_APPEND_VAL(buf, "ZIGBEE");
        }
        
        if (is_thread_onboarded())
        {
            JSON_APPEND_VAL(buf, "THREAD");
        }
    
        if (!(is_wifi_onboarded() || is_zigbee_onboarded() || is_thread_onboarded()))
        {
            JSON_APPEND_VAL(buf, "NONE");
        }
    }

    JSON_END_VAL(buf);

    JSON_END_BLOCK(buf);

    ONB_INFO("JSON BUF: %s\n", buf);
}

void handle_onboard_link_data(int32_t peerfd)
{
    int32_t bytes;
    int32_t ret;
    fd_set rset;

    qapi_fd_zero(&rset);
    qapi_fd_set(peerfd, &rset);

    while (1)
    {
        
        ret = qapi_select(&rset, NULL, NULL, 50000);
        ONB_INFO("Select return value: %d\n", ret);
        if (qapi_fd_isset(peerfd, &rset))
        {
            memset(onboarding_buf, 0, sizeof(onboarding_buf));
            bytes = qapi_recv(peerfd, onboarding_buf, sizeof(onboarding_buf) - 1, 0);
            onboarding_buf[bytes +1]='\0';
            ONB_INFO ("Received bytes are: %d\n", bytes);
            if (bytes < 0)
            {
                ONB_INFO("Onboarding link failed ! Server restarted!\n");
                qapi_socketclose(peerfd);
                break;
            }

            ret = process_actions(onboarding_buf, bytes);
            if (ret != 0)
            {
                ONB_INFO("Failed to parse the credentials\n");
            }

            memset(onboarding_buf, 0, sizeof(onboarding_buf));
            prepare_json_response(onboarding_buf);

            ONB_INFO("sending data is: %s\n", onboarding_buf);
            bytes = qapi_send(peerfd, onboarding_buf, strlen(onboarding_buf), 0);
            ONB_INFO("sent bytes are: %d\n", bytes);
        }
    }
}

/*----------------------------------------------------------------------------
  -onboarding_thread : thread for receiving the onboard details
        => Starts the SAP interface
        => Creates TCP server socket for exchanging the onboard info
        => Process the onboard information
        => Calls the corresponding radio to be onboarded with validated info
  ---------------------------------------------------------------------------*/
static void onboarding_thread(void *param)
{
    int32_t peerfd;
    int32_t serverfd;
    int32_t ret;
    int32_t len;
    struct sockaddr_in addr;
    struct sockaddr_in peer_addr;

    if (SUCCESS != start_sap())
    {
        ONB_ERROR("Onboarding failed. Restart the device\n");
        qurt_thread_stop();
        return;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(ONB_SRV_PORT);
    if (inet_pton(AF_INET, "0.0.0.0", &addr.sin_addr.s_addr) != 0)
    {
        ONB_ERROR( "Invalid IP address\n");
        return;
    }

    while(1)
    {
        serverfd = qapi_socket(AF_INET, SOCK_STREAM, 0);
        if (serverfd < 0)
        {
            ONB_ERROR( "Failed to open socket %d\n", serverfd);
            return;
        }

        len = (int32_t) sizeof(addr);
        ret = qapi_bind(serverfd, (struct sockaddr *)&addr, len);
        if (ret < 0)
        {
            ONB_ERROR( "Socket binding error %d\n", ret);
            return;
        }

        ret = qapi_listen(serverfd, 0);
        if (ret != 0)
        {
            ONB_ERROR("Listen failed\n");
            return;
        }

        len = (int32_t) sizeof(peer_addr);
        ONB_INFO( "Server started.........\n");
        ONB_INFO( "Waiting on accept ...........................\n");
        peerfd = qapi_accept(serverfd, (struct sockaddr *) &peer_addr, &len);
        if (peerfd == -1)
        {
            ONB_ERROR( "Accept failed\n");
            qurt_thread_stop();
            break;
        }
        qapi_socketclose(serverfd); //Serverfd is closed, for handling only one client
        handle_onboard_link_data(peerfd);
    }
}

/*---------------------------------------------------------------------------
  - Create  thread
  --------------------------------------------------------------------------*/
static int32_t create_thread(const char *thr_name, int32_t thr_prio, uint32_t thr_stack_size,
        void (*thread_func)(void *param))
{
    uint32_t len = 0;
    uint32_t ret = 0;
    qurt_thread_attr_t attr;

    qurt_thread_t thid = 0;
    qurt_thread_attr_init(&attr);
    qurt_thread_attr_set_name(&attr, thr_name);
    qurt_thread_attr_set_priority(&attr, thr_prio);
    qurt_thread_attr_set_stack_size(&attr, thr_stack_size);
//    if (0 != qurt_thread_create(&thid, &attr, thread_func, &len))
    ret = qurt_thread_create(&thid, &attr, thread_func, &len);
    ONB_INFO("Thread creation return value\n");
    if (ret)
    {
        ONB_ERROR("\nThread creation is failed\n");
        return FAILURE;
    }

    return SUCCESS;
}

/*---------------------------------------------------------------------------
  - Initialize_onboard_via_wifi: Initializes the wifi onboard demo
  --------------------------------------------------------------------------*/
int32_t Start_onboard_via_wifi(void)
{
    if (FAILURE == wlan_enable(WLAN_NUM_OF_DEVICES, &wlan_sigevent))
    {
        ONB_ERROR("wlan enable failed\n");
        return FAILURE;
    }

    if (FAILURE == create_thread("ONBViaWLAN", ONB_THRD_PRIO, ONB_THRD_STACK_SIZE, onboarding_thread))
    {
        ONB_ERROR("Failed to create Onboarding Thread\n");
        goto error;
    }

    return SUCCESS;

error:
    ONB_ERROR("Onboard Initialization Failed !!!\n");
    wlan_disable();
    return FAILURE;
}

/*---------------------------------------------------------------------------
  - Initialize_onboard_via_wifi: Initializes the wifi onboard demo
  --------------------------------------------------------------------------*/
int32_t Initialize_onboard_via_wifi(void)
{
    /* Create Signal object */
    if (QURT_EOK != qurt_signal_init(&wlan_sigevent))
    {
        ONB_ERROR("Signal init event failed\n");
        return FAILURE;
    }
    return SUCCESS;
}
