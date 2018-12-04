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
 * @file offline.c
 * @brief File contains the logic to handle offline sending and receiving.
 *
 * Implements  thread
 * monitor_offline_events: Handles all the offline sending and recivng events 
 */
#include <stdio.h>
#include <qcli_api.h>
#include <qcli_util.h>
#include "qurt_signal.h"
#include "qurt_thread.h"
#include "qurt_error.h"
#include "offline.h"
#include "onboard.h"
#include "log_util.h"
#include "util.h"
#include "sensor_json.h"
#include "jsmn.h"
#include "zigbee_util.h"
#include "stdlib.h"
#include "led_utils.h"
#include "onboard.h"

/*-------------------------------------------------------------------------
  - Preprocessor Definitions and Constants
  ------------------------------------------------------------------------*/


#define BUF_SIZE_128 128
#define BUF_SIZE_256 256
#define BUF_SIZE_512 512
#define MAX_LENGTH_OF_UPDATE_JSON_BUFFER   600

#define OFFLINE_INFO        LOG_INFO
#define OFFLINE_ERROR       LOG_ERROR
#define OFFLINE_VERBOSE     LOG_VERBOSE
#define OFFLINE_WARN        LOG_WARN

#define LIGHT_ID          "light_id_1"
#define DIMMER_ID         "dimmer_id_1"
#define THRMSTAT          "thermostat"

#define VALIDATE_AND_RETURN(js_buf, ret_val)  \
{ \
    if (ret_val < 0) \
    { \
        OFFLINE_ERROR("%s:%d JSON BUF error\n", __func__, __LINE__); \
        return FAILURE; \
    } \
    if (strlen(js_buf) >= MAX_LENGTH_OF_UPDATE_JSON_BUFFER) \
    { \
        OFFLINE_ERROR("%s:%d JSON BUF overun\n", __func__, __LINE__); \
        return FAILURE; \
    } \
}


int notify_thermo_breach = 0;
char offline_recv_buf[512];
static char breach_buf[BUF_SIZE_256];
static char remote_breach_buf[BUF_SIZE_256];
static char remote_buf[BUF_SIZE_512];
char remote_device_name[BUF_SIZE_128];
char offline_send_buf[MAX_OFFLINE_BUF_SIZE] = { 0 };
qurt_signal_t offline_event;
qurt_thread_t offline_thread;
static jsmntok_t t[128]; /* We expect no more than 128 tokens */
static    char token_name[128];
volatile int32_t process_flag = 0;
static char sub_token[BUF_SIZE_128];
static char localdevice_name[BUF_SIZE_128];
static uint8_t PIR_Payload[100] = {0};
static uint8_t board_name[32] = { 0};
static char ch;
int32_t read_onboard_sensors(char *json_buffer, sensor_info_t *sensor_val);
static int32_t Remote_delta_update(char *device_name, char *jsonbuf);
uint32_t Process_Dimmable_Light(char *, int);

/**
 * @func  : process_request_data 
 * @breif : process the request date from mobile application 
 */
void process_request_data(char *data, uint32_t len)
{
    static int flag = 0;
    if (!flag)
    {
        ch = zigbee_mode();
        flag = 0;
    }
    if (process_flag == 0 && (!is_zigbee_onboarded() || (ch == 'c' || ch == 'C')))
    {
        if (strlen(offline_recv_buf))
        {
            OFFLINE_INFO("Write request is in pending\n");
            return;
        }
        memcpy(offline_recv_buf, data, len);
        qurt_signal_set(&offline_event, OFFLINE_RECV);
        process_flag = 1;
    }
}

/**
 * @func  : fill_breach_buf  
 * @breif : fills the breach buffer 
 */
void fill_breach_buf(char *buf)
{
    memset(remote_breach_buf, 0, sizeof(remote_breach_buf));
    memcpy(remote_breach_buf, buf, strlen(buf));
    *(remote_breach_buf + strlen(remote_breach_buf) -1) = '\0';

}

/**
 * @func  : signal_set_breach_update  
 * @breif : sets the signal for breach update
 */
void signal_set_breach_update(char *buf)
{
    OFFLINE_INFO("Breach buffer: %s\n", buf);
    if (!strncmp(buf, BREACHED,strlen(BREACHED)))
    {
        fill_breach_buf(buf + strlen(BREACHED)+1);
    }
    qurt_signal_set(&offline_event, OFFLINE_BREACH);
}

/**
 * @func  : extract_device_name  
 * @breif : extract the devicename from jsonbuffer 
 */
void extract_device_name(char *jsonbuf, char *device_name)
{
    int i;
    int tokens;
    jsmn_parser p;
    jsmn_init(&p);
    tokens = jsmn_parse(&p, jsonbuf, strlen(jsonbuf), t, sizeof(t)/sizeof(t[0]));

    if (tokens < 0)
    {
        OFFLINE_ERROR("Failed to parse json\n");
        return;
    }
    for (i =2 ; i < tokens; i++)
    {
        if (t[i].parent == 2)
        {
            OFFLINE_INFO("Device_name :%.*s\n", t[i].end-t[i].start, jsonbuf + t[i].start);
            snprintf(device_name, (t[i].end-t[i].start+1), jsonbuf + t[i].start);
            break;
        }
    }
    return;
} 

/**
 * @func  : monitor_offline_events  
 * @breif : monitors the all offline events as send and recieve 
 */
void monitor_offline_events(void *arg)
{

    uint32_t rised_signal;
    int32_t sig_mask = OFFLINE_RECV | OFFLINE_BREACH;
    while(1)
    {
        OFFLINE_INFO("Waiting for OFFLINE_Recv event\n");
        if (QURT_EOK != qurt_signal_wait_timed(&offline_event, sig_mask, 
                    (QURT_SIGNAL_ATTR_WAIT_ANY|QURT_SIGNAL_ATTR_CLEAR_MASK), &rised_signal, QURT_TIME_WAIT_FOREVER))
        {
            OFFLINE_ERROR("%s:Failed on signal time_wait\n", __func__);
        }

        if (rised_signal & OFFLINE_BREACH)
        { 
            OFFLINE_INFO("pir breach buffer: %s\n", remote_breach_buf);
            update_breach_message(remote_breach_buf);    
        }
        if (rised_signal & OFFLINE_RECV)
        {
            process_offline_data(offline_recv_buf);
            memset(offline_recv_buf, 0, sizeof(offline_recv_buf));
            process_flag = 2;
        }
    }
}

/**
 * @func  : Start_offline_thread  
 * @breif : starts the offline thread for offline events 
 */
int32_t Start_offline_thread(void)
{
    unsigned long task_priority = OFFLINE_THREAD_PRIORITY;
    int stack_size = OFFLINE_THREAD_STACK_SIZE;

    if (QURT_EOK != qurt_signal_init(&offline_event))
    {
        OFFLINE_INFO("Offline signal intialization failed\n");
        return FAILURE;
    }

    //create a thread
    qurt_thread_attr_t attr;
    qurt_thread_attr_init(&attr);
    qurt_thread_attr_set_name(&attr, "offline_thread");
    qurt_thread_attr_set_priority (&attr, task_priority);
    qurt_thread_attr_set_stack_size (&attr, stack_size);

    if (qurt_thread_create(&offline_thread, &attr, monitor_offline_events, NULL) != QURT_EOK)
    {
        OFFLINE_ERROR("Failed to create aws thread");
        return FAILURE;
    }

    OFFLINE_INFO("Initialized AWS\n");
    return SUCCESS;

}

/**  
 * @func  : update_temp_data  
 * @breif : updates the temp buffer for aws 
 */
int32_t update_temp_data(char *buffer)
{
    OFFLINE_INFO("Process flag : %d\n", process_flag);
    if (process_flag == 2)
    {
        strcpy(buffer, offline_send_buf);
        memset(offline_send_buf, 0, sizeof(offline_send_buf));
        OFFLINE_INFO("buffer sending: %s\n", buffer);
        process_flag = 0;
    }
    return 0;
} 

/**  
 * @func  : fill_devices_list 
 * @breif : fills the device list info 
 */
int32_t fill_devices_list(char *JsonDocumentBuffer, uint32_t size)
{
    int32_t ret_val;
    char local_device_name[128] = { 0 };
    get_localdevice_name(local_device_name, sizeof(local_device_name));

    ret_val = snprintf((JsonDocumentBuffer + strlen(JsonDocumentBuffer)), size, "[{\"dName\":%s},", local_device_name);
    VALIDATE_AND_RETURN(JsonDocumentBuffer, ret_val);

    size = MAX_OFFLINE_BUF_SIZE - strlen(JsonDocumentBuffer);

    fill_remote_device_info(JsonDocumentBuffer, size);

    size = MAX_OFFLINE_BUF_SIZE - strlen(JsonDocumentBuffer);

    ret_val = snprintf((JsonDocumentBuffer + strlen(JsonDocumentBuffer)-1), size, "]");
    VALIDATE_AND_RETURN(JsonDocumentBuffer, ret_val);

    return SUCCESS;
}

/**  
 * @func  : construct_dlist_response 
 * @breif : constructs the device list response 
 */
int32_t construct_dlist_response(void)
{
    OFFLINE_INFO("%s\n", __func__);
    uint32_t ret_val;
    uint32_t rem_size = sizeof(offline_send_buf);
    memset(offline_send_buf, 0, sizeof(offline_send_buf));

    ret_val = snprintf(offline_send_buf, rem_size, "{\"dList\":");
    VALIDATE_AND_RETURN(offline_send_buf, ret_val);

    rem_size = MAX_OFFLINE_BUF_SIZE - strlen(offline_send_buf);
    fill_devices_list(offline_send_buf, rem_size); 
    ret_val = snprintf((offline_send_buf + strlen(offline_send_buf)), rem_size, "}");

    OFFLINE_INFO("offline_send_buf : %s\n", offline_send_buf); 
    return SUCCESS;
}

/*
 * @func  : prepare_thermostat_breach_message 
 * @breif : builds the thermostat breach message 
 *
 */
int32_t prepare_thermostat_breach_message(char *buf, int32_t size)
{
    memset(buf, 0, size);
    snprintf(buf, size, "{");
    size = size - strlen(buf);
    fill_breach_message(buf,"Thermostat threshold breached",size);
    *(buf + strlen(buf) -1) = '\0';
    OFFLINE_WARN("Breach buf: %s\n", buf);
    update_breach_message(buf); 
    return 0;
}

/*
 * @func  : fill_respone 
 * @breif : fills the response based on request 
 *
 */
int32_t fill_respone(char *device_name)
{
    char localdevice_name[BUF_SIZE_128] = { 0 };
    memset(offline_send_buf, 0, MAX_OFFLINE_BUF_SIZE);
    get_localdevice_name(localdevice_name, sizeof(localdevice_name));
    if (!strcmp(device_name, localdevice_name))
    {
        Update_json(offline_send_buf, MAX_OFFLINE_BUF_SIZE);
        if (notify_thermo_breach)
        {
            prepare_thermostat_breach_message(breach_buf, sizeof(breach_buf));
            notify_thermo_breach = 0;
        }
    }
    else
    {
        OFFLINE_INFO("Remote device info\n");
        Zigbee_device_update(device_name, offline_send_buf, MAX_OFFLINE_BUF_SIZE);
        OFFLINE_WARN("offline_send_buf: %s\n", offline_send_buf);
    }
    return SUCCESS;
}

/*
 * @func  : process_remote_data 
 * @breif :  @breif : Send the remote data to zigbee/thread devices 
 */  
static int32_t Process_remote_data(char *jsonbuf, char *Device_name)
{
    int32_t ret = -1;
    if (is_thread_onboarded())
    {
        ret = Send_data_to_joiner(Device_name, jsonbuf);
    }

    if (is_zigbee_onboarded())
    {
        ret = Remote_delta_update(Device_name, jsonbuf);
    }

    return ret;
}

/**
 * @func  : Remote_delta_update 
 * @breif : Parse the zigbee data and send values to the zigbee device 
 */
static int32_t Remote_delta_update(char *device_name, char *jsonbuf)
{
    int i;
    int rc;
    int tokens;
    jsmn_parser p;
    static char sub_token[32];
    char devicename[128] = { 0 };
    jsmn_init(&p);
    tokens = jsmn_parse(&p, jsonbuf, strlen(jsonbuf), t, sizeof(t)/sizeof(t[0]));

    if (tokens < 0)
    {
        OFFLINE_ERROR("Failed to parse json\n");
        return FAILURE;
    }

    for (i =1 ; i < tokens; i++)
    {
        if (t[i].parent == 0)
        {
            snprintf(devicename, t[i].end-t[i].start+1, jsonbuf + t[i].start);
//            OFFLINE_INFO("Device_name :%.*s\n", t[i].end-t[i].start, jsonbuf + t[i].start);
            OFFLINE_INFO("Remote device: %s\n", devicename);
        }

        if (t[i].parent == 2)
        {
            i = i +2;
            memset(token_name, 0, sizeof(token_name));
            snprintf(token_name, (t[i].end-t[i].start)+1,"%s", jsonbuf + t[i].start);
            memset(sub_token, 0, sizeof(sub_token));
            snprintf(sub_token, (t[i+1].end-t[i+1].start)+1,"%s", jsonbuf + t[i+1].start);
   //         rc = strncmp(LIGHT_ID, token_name, strlen(LIGHT_ID));
            rc = strncmp(DIMMER_ID, token_name, strlen(DIMMER_ID));

            OFFLINE_INFO(" key : %s\t", token_name);
            OFFLINE_INFO(" Val : %s\n", sub_token);
            if (!rc)
            {
               // rc = Process_light(device_name, sub_token);
                rc = Process_Dimmable_Light(devicename, atoi(sub_token));

            }
        }
        i++;
    }
    return SUCCESS;
}

/**
 * @func  : process_offline_data 
 * @breif : Process the offline data and build the message based on request   
 */
int32_t process_offline_data(char *jsonbuf)
{
    int i;
    int tokens;
    int32_t rc;
    jsmn_parser p;
    char *start = NULL;
    char *end = NULL;
    int32_t dlist_bit = 0;
    int32_t sensors_bit = 0;
    int32_t desired_bit = 0;
    int32_t local_flag = 0;
    int32_t remote_flag = 0;
    int32_t thermo_flag = 0;
    int32_t update_flag = 0;

    OFFLINE_INFO("Json buffer: %s\n", jsonbuf);
    jsmn_init(&p);
    tokens = jsmn_parse(&p, jsonbuf, strlen(jsonbuf), t, sizeof(t)/sizeof(t[0]));

    if (tokens < 0)
    {
        OFFLINE_ERROR("Failed to parse json\n");
        return FAILURE;
    }
    OFFLINE_INFO("tokens: %d\n", tokens);
    for (i =1 ; i < tokens; i++)
    {
        if (t[i].parent == 0)
        {
            OFFLINE_INFO("keyword :%.*s\n", t[i].end-t[i].start, jsonbuf + t[i].start);
            snprintf(token_name, (t[i].end-t[i].start)+1,"%s", jsonbuf + t[i].start);
            if (!strcasecmp(token_name, GETLIST))
            {
                dlist_bit = 1; 
            }
            if (!strcasecmp(token_name, SENSORS))
            {
                sensors_bit = 1;
            }
            if (!strcasecmp(token_name, DESIRED))
            {
                desired_bit = 1;
            }
            i++;
        }
        if (dlist_bit)
        {
            OFFLINE_INFO("parent_id: %d\n", t[i].parent);
            if (t[i].parent == 3)
            {
                OFFLINE_INFO("Inside_t[i].Parent: \n");
                memset(localdevice_name, 0, sizeof(localdevice_name));
                get_localdevice_name(localdevice_name, sizeof(localdevice_name));
                OFFLINE_INFO("local device name: %s\n", localdevice_name);
                snprintf(token_name, (t[i].end-t[i].start)+3,"\"%s\"", jsonbuf + t[i].start);
                OFFLINE_INFO("Device_name:%s\n", token_name);
                if (!strcmp(localdevice_name, token_name))
                {
                    construct_dlist_response();
                }
                else
                {
                    OFFLINE_INFO("Device is not matched\n");
                }
                dlist_bit = 0;
            }
        }
        if (sensors_bit)
        {
            if (t[i].parent == 5)
            {
                memset(localdevice_name, 0, sizeof(localdevice_name));
                get_localdevice_name(localdevice_name, sizeof(localdevice_name));
                snprintf(token_name, (t[i].end-t[i].start)+3,"\"%s\"", jsonbuf + t[i].start);
                OFFLINE_INFO("Device_name: %s\n", token_name);

                fill_respone(token_name);
                sensors_bit = 0;
            }

        }
        if (desired_bit)
        {   
            if ( i == 2)
                start = jsonbuf + t[i].start;
            if (t[i].parent == 2)
            {
                snprintf(token_name, (t[i].end-t[i].start)+3,"\"%s\"", jsonbuf + t[i].start);
                memset(localdevice_name,0, sizeof(localdevice_name));
                get_localdevice_name(localdevice_name, sizeof(localdevice_name));
                OFFLINE_INFO("Device_name : %s\n", token_name);
                OFFLINE_INFO("local_device_name : %s\n", localdevice_name);
                if(!strcmp(localdevice_name, token_name))
                {
                    local_flag = 1;
                }
                else
                {
                    remote_flag = 1;
                    memcpy(remote_device_name, token_name, strlen(token_name));
                } 
            }
            if (t[i].parent == 4)
            {
                i = i+2;
                update_flag = 1;
            }
            if (update_flag)
            {
                memset(token_name, 0, sizeof(token_name));
                snprintf(token_name, (t[i].end-t[i].start)+1,"%s", jsonbuf + t[i].start);

                OFFLINE_INFO("key : %.*s\t", t[i].end-t[i].start,
                        jsonbuf + t[i].start);
                OFFLINE_INFO("value : %.*s\n", t[i+1].end-t[i+1].start,
                        jsonbuf + t[i+1].start);

                rc = strncmp(THRMSTAT, token_name, strlen(THRMSTAT));
                memset(sub_token, 0, sizeof(sub_token));
                snprintf(sub_token, (t[i+1].end-t[i+1].start)+1,"%s", jsonbuf + t[i+1].start);

                if (!thermo_flag && !rc)
                {
                    thermo_flag = 1;
                }
                if (thermo_flag)
                {
                    rc = Get_thermostat_threshhold_values(token_name,sub_token);
                    if (rc == 3)
                        thermo_flag = 0;
                }

              //  rc = strncmp(LIGHT_ID, token_name, strlen(LIGHT_ID));
                rc = strncmp(DIMMER_ID, token_name, strlen(DIMMER_ID));
                OFFLINE_INFO("Local flag : %d\n", local_flag);
                OFFLINE_INFO("rc : %d\n", rc);
                if (local_flag && !rc)
                {
               //     rc = process_light_localdevice(sub_token);
                    BLUE_LED_CONFIG(50, (atoi(sub_token)/5));
                    Update_dimmer_value(atoi(sub_token));
                    local_flag = 0;
                }

                i++;
            }
        }
    }


    end = jsonbuf + t[i-1].end + 3;
    if (remote_flag)
    {
        memset(remote_buf, 0, sizeof(remote_buf));
        memcpy(remote_buf, start, (int)(end-start));
        OFFLINE_INFO("Remote_buffer :%s\n", remote_buf);
        Process_remote_data(remote_buf, remote_device_name);
    }

    return SUCCESS;

}

/**
 * @func  : Pir_offline_breach_message 
 * @breif : builds the pir offline breach message   
 */
int32_t Pir_offline_breach_message(void) 
{
    OFFLINE_INFO("pir offline breach\n");
    memset(PIR_Payload , 0, sizeof(PIR_Payload));
    memset(board_name, 0, sizeof(board_name));
    get_localdevice_name((char *)board_name, sizeof(board_name));

    snprintf((char *) PIR_Payload,15,"{");

    memcpy((char *) PIR_Payload + strlen((const char *)PIR_Payload),board_name,strlen((const char *)board_name));
    snprintf((char *) PIR_Payload + strlen((const char *)PIR_Payload), 37, ":{\"message\":\"Motion detected\"}}");

    OFFLINE_INFO("Pir message: %s\n", PIR_Payload);
    update_breach_message((char *)PIR_Payload);
    return 0;
}
