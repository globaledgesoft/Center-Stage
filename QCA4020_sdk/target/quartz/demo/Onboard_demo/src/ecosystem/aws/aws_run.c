/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 * NOT A CONTRIBUTION
 *
 * Copyright 2010-2015 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *  http://aws.amazon.com/apache2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

/**
 * @file aws_run.c
 * @brief File contains the logic to handle the sending and receiving data
 * to and from Aws server. 
 * Implements thread
 * aws_run: Handles sending the data to aws from all zigbee/thread devices.
 *    Uses the configutration received from the Mobile app to onboard
 *    the radios to the network
 * Implement Aws subscription callback
 * parse_shadowRxBuf : Parses the data recived from aws server and send event
 * to the corresponding device in zigbee/thread 
 */

/*-------------------------------------------------------------------------
 * Include Files
 *-----------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <limits.h>
#include <stdarg.h>
#include <inttypes.h>

#include "aws_iot_config.h"
#include "aws_iot_log.h"
#include "aws_iot_version.h"
#include "aws_iot_mqtt_client_interface.h"
#include "aws_iot_shadow_interface.h"
#include "aws_iot_shadow_json.h"
#include "jsmn.h"
#include "shadow_sample.h"

#include "netutils.h"
#include "qapi/qurt_thread.h"
#include "qapi/qurt_types.h"
#include "qapi/qurt_error.h"
#include "qapi/qurt_signal.h"
#include "qapi/qurt_mutex.h"
#include "qapi/qurt_timer.h"
#include "qapi_fs.h"
#include "qapi_crypto.h"
#include "qapi_ssl_cert.h"
#include "cert_buf.h"
#include "aws_util.h"
#include "sensor_json.h"
#include "util.h"
#include "onboard.h"
#include "thread_util.h"

/*-------------------------------------------------------------------------
  - Preprocessor Definitions, Constants and Global Varibles
  ------------------------------------------------------------------------*/

int notify_thermo_breach = 0;
volatile uint32_t aws_thread_stop;
static int32_t aws_thread_exited = 0;

char JsonDocumentBuffer[MAX_LENGTH_OF_UPDATE_JSON_BUFFER];
qurt_thread_t aws_thread_handle;
AWS_IoT_Client *mqttClient = NULL;
ShadowInitParameters_t *sp = NULL;
volatile boolean aws_running;
volatile uint32_t breach_val = 0;
qurt_signal_t aws_signal;
qurt_mutex_t  shadow_update_lock;
char remote_buf[256] = { 0 };
static int32_t Remote_delta_update(char *device_name, char *jsonbuf);
int32_t read_onboard_sensors(char *json_buffer, sensor_info_t *sensor_val);
uint32_t Process_Dimmable_Light(char *, int);
char remote_breach_buf[256];
char remote_shdow_buf[256];
char localdevice_name[64];
char remote_device_name[64];
char remote_update[256];

char shadowRxBuf[MAX_LENGTH_OF_RX_JSON_BUFFER];

#define PIR_BREACH_EVENT                   (1)
#define TOPIC_BREACH      "threshold_breached"
#define THRMSTAT          "thermostat"
#define LIGHT_ID          "light_id_1"
#define DIMMER_ID         "dimmer_id_1"

#define VALIDATE_AND_RETURN(js_buf, ret_val)  \
{ \
        if (ret_val < 0) \
        { \
                    IOT_ERROR("%s:%d JSON BUF error\n", __func__, __LINE__); \
                    return FAILURE; \
                } \
        if (strlen(js_buf) >= MAX_LENGTH_OF_UPDATE_JSON_BUFFER) \
        { \
                    IOT_ERROR("%s:%d JSON BUF overun\n", __func__, __LINE__); \
                    return FAILURE; \
                } \
}
/*-------------------------------------------------------------------------
  - Functions
  ------------------------------------------------------------------------*/

/**
 * @func  : Update_Remote_devices_data
 * @breif : Get the the date from remote devices 
 */
static int32_t Update_Remote_devices_data(char *data,uint32_t size)
{
    if (is_zigbee_onboarded())
    {
        Read_Zigbee_Devices_Data(data,size);
    }

    if (is_thread_onboarded())
    {
        Read_Thread_Devices_Data(data,size);
    }
    return 0;
}

/**
 * @func  : Process_remote_data 
 * @breif : Send the remote data to zigbee/thread devices 
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
    jsmntok_t t[128]; /* We expect no more than 128 tokens */
    char token_name[128];
    char device_name1[128];
    char sub_token[32];

    jsmn_init(&p);
    tokens = jsmn_parse(&p, jsonbuf, strlen(jsonbuf), t, sizeof(t)/sizeof(t[0]));

    if (tokens < 0)
    {
        IOT_ERROR("Failed to parse json\n");
        return FAILURE;
    }

    for (i =1 ; i < tokens; i++)
    {
        if (t[i].parent == 0)
        {
            memset(device_name1, 0, sizeof(device_name1));
//            IOT_INFO("Device_name :%.*s\n", t[i].end-t[i].start, jsonbuf + t[i].start);
            snprintf(device_name1, (t[i].end-t[i].start)+1,"%s", jsonbuf + t[i].start);
            IOT_INFO("Device_name: %s\n", device_name1);
        }

        if (t[i].parent == 2)
        {
            i = i +2;
            memset(token_name, 0, sizeof(token_name));
            snprintf(token_name, (t[i].end-t[i].start)+1,"%s", jsonbuf + t[i].start);
            memset(sub_token, 0, sizeof(sub_token));
            snprintf(sub_token, (t[i+1].end-t[i+1].start)+1,"%s", jsonbuf + t[i+1].start);
            rc = strncmp(DIMMER_ID, token_name, strlen(DIMMER_ID));

            IOT_INFO(" key : %s\t", token_name);
            IOT_INFO(" Val : %s\n", sub_token);
            if (!rc)
            {
                rc = Process_Dimmable_Light(device_name1, atoi(sub_token));

            }
        }
        i++;
    }
    return SUCCESS;
}

/**
 * @func  : ShadowUpdateStatusCallback 
 * @breif : Handles the Staus of the shadow whether is it updated in the Aws console or not 
 */
void ShadowUpdateStatusCallback(const char *pThingName, ShadowActions_t action, Shadow_Ack_Status_t status,
        const char *pReceivedJsonDocument, void *pContextData)
{
    IOT_UNUSED(pThingName);
    IOT_UNUSED(action);
    IOT_UNUSED(pReceivedJsonDocument);
    IOT_UNUSED(pContextData);

    if(SHADOW_ACK_TIMEOUT == status)
    {
        IOT_INFO("Update Timeout--\n");
    }
    else if(SHADOW_ACK_REJECTED == status)
    {
        IOT_INFO("Update RejectedXX\n");
    }
    else if(SHADOW_ACK_ACCEPTED == status)
    {
        IOT_INFO("Update Accepted !!\n");
    }
}

/**
 * @func  : parse_shadowRxBuf 
 * @breif : Parse the delta buffer get from aws take corresponding action if
 * it is local device or send to remote device if it is a remote device 
 * Eg: Light on command came from aws server. Then this function will execute
 */
jsmntok_t t[128]; /* We expect no more than 128 tokens */
char token_name[128];
char sub_token[32];
int32_t parse_shadowRxBuf(char *jsonbuf, uint32_t token)
{
    int i;
    int tokens;
    jsmn_parser p;
    int32_t thermo_flag = 0;
    int32_t local_flag = 0;
    int32_t remote_flag = 0;
    int32_t rc;
    char *start = NULL;
    char *end = NULL;
    int sensor_parent = -1;
    int parent_id = -1;
 
    jsmn_init(&p);
    tokens = jsmn_parse(&p, jsonbuf, strlen(jsonbuf), t, sizeof(t)/sizeof(t[0]));
    if (tokens < 0)
    {
        IOT_ERROR("Failed to parse json\n");
        return FAILURE;
    }

        for ( i = 1 ; i < tokens ; i++)
    {
        snprintf(token_name, t[i].end-t[i].start+1, "%s", jsonbuf + t[i].start);
        if (!strcmp("state", token_name))
        {
            parent_id = i + 1;
            break;
        }
        if ( i == tokens )
        {
            IOT_WARN("Tokens are not there to get data\n");
        }
    }

    for (i = parent_id; i < tokens ; i++)
    {
        if (t[i].parent == 0)
        {
            end = jsonbuf + t[i].start -2; 
            break;
        }
        if( t[i].parent == parent_id)
        {
            start = jsonbuf + t[i].start -2; 
            memset(localdevice_name, 0, sizeof(localdevice_name));
            get_localdevice_name(localdevice_name, sizeof(localdevice_name));
            snprintf(token_name, t[i].end-t[i].start+3, "\"%s\"", jsonbuf + t[i].start);
            IOT_INFO(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
            IOT_INFO("Device_name :%.*s\n", t[i].end-t[i].start, jsonbuf + t[i].start);
            rc = strcmp(localdevice_name, token_name);

            if (!rc)
            {
                local_flag = 1;
            }
            else
            {
                IOT_INFO("process_remote data\n");   
                if (!remote_flag) 
                {
                    memset(remote_device_name, 0, sizeof(remote_device_name));
                    memcpy(remote_device_name, token_name, strlen(token_name));
                    IOT_INFO("........................................................................................\n");
                    IOT_INFO("Remote device name : %s\n", remote_device_name);
                    remote_flag = 1;
                }
            }

            i +=2;
            sensor_parent = i - 1;
            for(  ; i < tokens  ; i++)
            {
                if(t[i].parent <= parent_id )
                {
                    i--;
                    break;
                }

                if( sensor_parent == t[i]. parent)
                {
                    i +=2;
                    for( ; i<tokens ; i++)
                    {
                        if( t[i].parent <= parent_id)
                        {
                            i--;
                            break;
                        }
                        if( t[i].parent == sensor_parent)
                        {
                            i--;
                            break;
                        }
                        memset(token_name, 0, sizeof(token));
                        snprintf(token_name, (t[i].end-t[i].start)+1,"%s", jsonbuf + t[i].start);

                        IOT_INFO("key : %.*s\t", t[i].end-t[i].start,
                                jsonbuf + t[i].start);
                        IOT_INFO("value : %.*s\n", t[i+1].end-t[i+1].start,
                                jsonbuf + t[i+1].start);

                        rc = strncmp(THRMSTAT, token_name, strlen(THRMSTAT));
                        memset(sub_token, 0, sizeof(sub_token));
                        snprintf(sub_token, (t[i+1].end-t[i+1].start)+1,"%s", jsonbuf + t[i+1].start);

                        if (!thermo_flag && !rc) 
                        {
                            thermo_flag = 1;
                        }
                        if (thermo_flag && local_flag)
                        {
                            rc = Get_thermostat_threshhold_values(token_name,sub_token); 
                            if (rc == 3)
                                thermo_flag = 0;
                        }

                        rc = strncmp(LIGHT_ID, token_name, strlen(LIGHT_ID));
                        if (local_flag && !rc)
                        {
                            rc = process_light_localdevice(sub_token);
                            local_flag = 0;
                        }
                        i++;
                    }
                }
            }
        }
        if (remote_flag)
        {
            end = jsonbuf + t[i+1].start - 2;
            memset(remote_buf, 0, sizeof(remote_buf));
            memcpy(remote_buf, start, (end-start));
            if (*remote_buf != '{')
            {
                *remote_buf = '{';
            }
            if (t[i+1].parent != 0)
            {
               *(remote_buf + strlen(remote_buf)) = '}';
            }
         
            IOT_INFO("Remote_buffer :%s\n", remote_buf);
            IOT_INFO("Remote_Device_name: %s\n", remote_device_name);
            Process_remote_data(remote_buf, remote_device_name);
            remote_flag = 0;
        }

    }

    return SUCCESS;
}

/**
 * @func  : shadow_delta_callback 
 * @breif : Subrciption call back for shadow update callback
 */
void shadow_delta_callback(AWS_IoT_Client *pClient, char *topicName,
        uint16_t topicNameLen, IoT_Publish_Message_Params *params, void *pData)
{
    int32_t tokenCount;
    void **pJsonHandler = NULL;

    if(params->payloadLen > SHADOW_MAX_SIZE_OF_RX_BUFFER) {
        IOT_WARN("Payload larger than RX Buffer");
        return;
    }

    memcpy(shadowRxBuf, params->payload, params->payloadLen);
    shadowRxBuf[params->payloadLen] = '\0';    // jsmn_parse relies on a string

    IOT_INFO("Received buf :%s\n", shadowRxBuf);

    if(!isJsonValidAndParse(shadowRxBuf, (void **)&pJsonHandler, &tokenCount))
    {
        IOT_WARN("Received JSON is not valid");
        return;
    }
    parse_shadowRxBuf(shadowRxBuf, tokenCount);


    return ;

}

/**
 * @func  : Update_shadow 
 * @breif : updating the shadow for localdevice 
 */

int32_t Update_shadow(char *JsonDocumentBuffer)
{
    int32_t rc;
     rc = aws_iot_shadow_update(mqttClient, THING_NAME, JsonDocumentBuffer,
                            ShadowUpdateStatusCallback, NULL, 4, true);
     return rc;
}

/**
 * @func  : Notify_breach_update_to_aws 
 * @breif : notified the breach update message 
 */

int32_t Notify_breach_update_to_aws(char *buf)
{
    uint32_t ret_val = 0;

    if (aws_running) 
    {
        IoT_Publish_Message_Params msgParams;

        IOT_INFO("---Breach message : %s\n", buf);
        msgParams.qos = QOS0;
        msgParams.payloadLen = strlen(buf);
        msgParams.payload = (char *)buf;
        ret_val = aws_iot_mqtt_publish(mqttClient, TOPIC_BREACH, (uint16_t)strlen(TOPIC_BREACH), &msgParams);

        IOT_INFO("publish return value:%d\n", ret_val);
    }
    return ret_val;
}

/**
 * @func  : Notify_sensors_update_from_remote_device 
 * @breif : Notifies the remote devices update 
 */
int32_t Notify_sensors_update_from_remote_device(char *buf)
{
    int32_t tokenCount;
    int32_t ret_val = -1;
    void **pJsonHandler = NULL;
    if(!isJsonValidAndParse(buf, (void **)&pJsonHandler, &tokenCount))
    {
        IOT_WARN("JSON  Received from remote device is not valid");
        return -1;
    }
    else 
    {
        if(qurt_mutex_try_lock(&shadow_update_lock))
            return ret_val; 
        memset(remote_shdow_buf, 0, sizeof(remote_shdow_buf));
        ret_val = snprintf(remote_shdow_buf, sizeof(remote_shdow_buf), "%s", buf);
        qurt_mutex_unlock(&shadow_update_lock);
    }
    return ret_val;
}

/**
 * @func  : Notify_breach_update_from_remote_device
 * @breif : Notifies the remote breach messages 
 */
int32_t Notify_breach_update_from_remote_device(char *buf)
{
    int32_t rc = 0;

    memset(remote_breach_buf, 0, sizeof(remote_breach_buf));
    snprintf(remote_breach_buf, sizeof(remote_breach_buf),"{\"%s\"", THING_NAME);
    rc = snprintf(remote_breach_buf+strlen(remote_breach_buf), sizeof(remote_breach_buf)-strlen(remote_breach_buf), "%s",buf);
    Notify_breach_update_to_aws(remote_breach_buf);

    return rc;
} 

/**
 * @func  : notify_thermostat_breach 
 * @breif : notified the breach update message 
 */

int32_t notify_thermostat_breach(char *buf, uint32_t size)
{
    snprintf(buf, size, "{\"%s\":{", THING_NAME);
    size = size - strlen(buf); 
    fill_breach_message(buf,"Thermostat threshold breached",size);
    return Notify_breach_update_to_aws(buf); 
}

/**
 * @func  : Send_Remote_device_update_to_aws 
 * @breif : sends remote devices data to aws 
 */
int32_t Send_Remote_device_update_to_aws(char *buf)
{
    int32_t rc;
    IOT_INFO("%s\n", buf); 
    rc = Update_shadow(buf);
    if(rc)
    {
        IOT_ERROR("shadow update failed : %d\n", rc);
    }
    return rc;
}

/**
 * @func  : subscribe_aws 
 * @breif : subscription to the aws server 
 */
int32_t subscribe_aws(AWS_IoT_Client *pMqttClient, char *myThingName)
{
    char *Topic_name = NULL;

    Topic_name = malloc(MAX_SHADOW_TOPIC_LENGTH_BYTES);
    if (NULL == Topic_name)
    {
        return FAILURE;
    }
    memset(Topic_name, 0, MAX_SHADOW_TOPIC_LENGTH_BYTES);
    snprintf(Topic_name, MAX_SHADOW_TOPIC_LENGTH_BYTES, "$aws/things/%s/shadow/update/delta", myThingName);
    IOT_INFO("Topic_name : %s\n", Topic_name);
    aws_iot_mqtt_subscribe(pMqttClient, Topic_name, (uint16_t) strlen(Topic_name), QOS0, shadow_delta_callback, NULL);

      return SUCCESS;
}

/**
 * @func  : load_cert_file
 * @breif : SSL Certificates loading 
 */
int32_t load_cert_file(int32_t cert_load_forceful)
{
     int32_t status;
    uint8_t *key_buf = aws_thing_privkey;
    uint8_t *cert_buf = aws_thing_cert;
    uint32_t cert_buf_size = sizeof (aws_thing_cert);
    uint32_t key_buf_size = sizeof (aws_thing_privkey);
    qapi_Net_SSL_Cert_Info_t cert_info;
    qapi_CA_Info_t ca_info;
    struct qapi_fs_stat_type fstat;

    if (!cert_load_forceful)
    {
        char file_name[128] = { 0 };
        snprintf(file_name,sizeof(file_name),"/spinor/ssl/certs/%s",AWS_CERT_LOC);
        if (QAPI_OK == qapi_Fs_Stat(file_name, &fstat))
        {
            if (fstat.st_size != 0)
            {
                IOT_INFO("ssl certificate already loaded");
                return SUCCESS;
            }
        }

        memset(file_name,0,128);
        snprintf(file_name,sizeof(file_name),"/spinor/ssl/ca_lists/%s",AWS_CALIST_LOC);
        if (QAPI_OK == qapi_Fs_Stat(file_name, &fstat))
        {
            if (fstat.st_size != 0)
            {
                IOT_INFO("calist certificate already loaded");
                return SUCCESS;
            }
        }
    }
    /* storing into cert */
    memset(&cert_info, 0, sizeof(cert_info));
    cert_info.cert_Type = QAPI_NET_SSL_PEM_CERTIFICATE_WITH_PEM_KEY_E;
    cert_info.info.pem_Cert.cert_Buf = cert_buf;
    cert_info.info.pem_Cert.cert_Size = cert_buf_size;
    cert_info.info.pem_Cert.key_Buf = key_buf;
    cert_info.info.pem_Cert.key_Size = key_buf_size;
    status = qapi_Net_SSL_Cert_Store(&cert_info, AWS_CERT_LOC);
    if (QAPI_OK != status)
    {
        IOT_INFO("certficate store is failed\n");
        return FAILURE;
    }
    else
    {
        IOT_INFO("certificate store is success ......................\n");
    }

    cert_buf = aws_calist;
    cert_buf_size = sizeof(aws_calist);

    memset(&cert_info, 0, sizeof(cert_info));
    cert_info.cert_Type = QAPI_NET_SSL_PEM_CA_LIST_E;
    ca_info.ca_Buf =  cert_buf;
    ca_info.ca_Size = cert_buf_size;
    cert_info.info.pem_CA_List.ca_Cnt = 1;
    cert_info.info.pem_CA_List.ca_Info[0] = &ca_info;
    status = qapi_Net_SSL_Cert_Store(&cert_info, AWS_CALIST_LOC);
    if (QAPI_OK != status)
    {
        IOT_INFO("certficate store is failed\n");
        return FAILURE;
    }
    else
    {
        IOT_INFO("certificate store is success ......................\n");
    }

    return SUCCESS;

}

/**
 * @func  : aws_thread 
 * @breif : Handles all aws intialization and subscription 
 */
void aws_thread(void *param)
{
    IoT_Error_t rc = FAILURE;
    uint32_t shutdown = 0;
    char cert_stub[] = "this is a stub cert";
    ShadowInitParameters_t *sp = NULL;
    ShadowConnectParameters_t *scp = NULL;
    Timer sendUpdateTimer;
    char *ca_list = AWS_CALIST_LOC;
    uint32_t sigmask = 0;
    uint32_t rised_signal = 0;
    int32_t ret;
    size_t sizeOfJsonDocumentBuffer = MAX_LENGTH_OF_UPDATE_JSON_BUFFER;

    IOT_INFO("\nAWS IoT SDK Version %d.%d.%d-%s\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_TAG);

    qurt_mutex_create(&shadow_update_lock);
 
    IOT_INFO("Stack rc=%x ret=%x\n", &rc, &ret);

    mqttClient = malloc(sizeof(AWS_IoT_Client));

    if(mqttClient == NULL)
        goto aws_conn_error;

    IOT_INFO("Malloc mqttClient=%x\n", mqttClient);
    memset(mqttClient, 0, sizeof(AWS_IoT_Client));
    memset(JsonDocumentBuffer, 0, sizeOfJsonDocumentBuffer);

    sp = malloc(sizeof(ShadowInitParameters_t));
    if(sp == NULL)
        goto aws_conn_error;

    *sp = ShadowInitParametersDefault;
    IOT_INFO("\n AWS shadow_init done\n");
    sp->pHost = malloc(strlen(HOST_ID) + 1);
    if (NULL == sp->pHost)
    {
        goto aws_conn_error;
    }

    memcpy(sp->pHost, HOST_ID, strlen(HOST_ID) +1);

    sp->port = AWS_IOT_MQTT_PORT;
    sp->pClientCRT = malloc(AWS_MAX_CERT_NAME);
    if (NULL == sp->pClientCRT)
    {
        goto aws_conn_error;
    }
    memcpy(sp->pClientCRT, AWS_CERT_LOC, (strlen(AWS_CERT_LOC)+1));
    sp->pClientKey = cert_stub;
    sp->pRootCA = ca_list;
    sp->enableAutoReconnect = false;
    sp->disconnectHandler = NULL;

    scp = malloc(sizeof(ShadowConnectParameters_t));

    if(scp == NULL)
        goto aws_conn_error;
    scp->pMyThingName = malloc(strlen(THING_NAME) + 1);

    if(scp->pMyThingName == NULL)
        goto aws_conn_error;


    memcpy(scp->pMyThingName, THING_NAME, strlen(THING_NAME) + 1);
    scp->pMqttClientId = AWS_IOT_MQTT_CLIENT_ID;
    scp->mqttClientIdLen = (uint16_t) strlen(AWS_IOT_MQTT_CLIENT_ID);
    IOT_INFO("Hostname:%s\n", sp->pHost);
    IOT_INFO("Client crt file name:%s\n", sp->pClientCRT);
    IOT_INFO("Client Thing name:%s\n", scp->pMyThingName);
    if( FAILURE == load_cert_file(1))
    {
        IOT_INFO("Certfication copy is failed\n");
        goto aws_conn_error;
    }
    sigmask = SIGNAL_AWS_START_EVENT;

    while(1)
    {
        rised_signal = 0;

        ret = qurt_signal_wait_timed(&aws_signal, sigmask, (QURT_SIGNAL_ATTR_WAIT_ANY |
                    QURT_SIGNAL_ATTR_CLEAR_MASK) , &rised_signal, QURT_TIME_WAIT_FOREVER);
        if (ret == QURT_EINVALID)
        {
            IOT_INFO("Failed handling AWS signal!!!\n");
            break;
        }

        IOT_INFO("Shadow Init \n\n");
        rc = aws_iot_shadow_init(mqttClient, sp);
        if(SUCCESS != rc)
        {
            IOT_ERROR("Shadow Connection Error \n\n");
            continue;
        }
        
       
        if (is_thread_onboarded())
        {
            if (!shutdown)
            {
                Shutdown_thread_network();
                shutdown = 1; 
            }
        }
    
        IOT_INFO("Shadow Connect \n\n");
        rc = aws_iot_shadow_connect(mqttClient, scp);
        if(SUCCESS != rc)
        {
            IOT_ERROR("Shadow Connection Error \n\n: rc = %d\n", rc);
            if (rc == -17)
            { 
                IOT_WARN("Please restart the device\n");

            }
            if (rc == -5)
            {
                restart_device();
            }
            continue;
        }
        IOT_INFO("Shadow Connection successful \n\n");

       if (shutdown)
       {
            Start_thread_network();
            shutdown = 0;
       }

       subscribe_aws(mqttClient, scp->pMyThingName);
        /*
         * Enable Auto Reconnect functionality. Minimum and Maximum time of Exponential backoff are set in aws_iot_config.h
         *  #AWS_IOT_MQTT_MIN_RECONNECT_WAIT_INTERVAL
         *  #AWS_IOT_MQTT_MAX_RECONNECT_WAIT_INTERVAL
         */
        rc = aws_iot_shadow_set_autoreconnect_status(mqttClient, true);

        if(SUCCESS != rc)
        {
            IOT_ERROR("Unable to set Auto Reconnect to true - %d", rc);
            goto aws_conn_error;
        }

        IOT_INFO("shadow reconnect status done\n");
        init_timer(&sendUpdateTimer);
        countdown_ms(&sendUpdateTimer, 0);
        aws_running = 1;
        while((NETWORK_ATTEMPTING_RECONNECT == rc || NETWORK_RECONNECTED == rc || SUCCESS == rc) && aws_running)
        {
            rc = aws_iot_shadow_yield(mqttClient, 1000);
            IOT_INFO("In while RC Value:%d\n",rc);
            if(NETWORK_ATTEMPTING_RECONNECT == rc)
            {
                app_msec_delay(1000);
                // If the client is attempting to reconnect we will skip the rest of the loop.
                aws_running = 0;
                continue;
            }
            if (has_timer_expired(&sendUpdateTimer))
            {
                IOT_INFO("\n=======================================================================================\n");
                if(SUCCESS == rc)
                {
                    memset(JsonDocumentBuffer, 0, sizeOfJsonDocumentBuffer);
                    Update_json(JsonDocumentBuffer, sizeOfJsonDocumentBuffer);
                    IOT_INFO("Shadow updated : %s\n", JsonDocumentBuffer);
                    qurt_mutex_lock(&shadow_update_lock);
                    rc = aws_iot_shadow_update(mqttClient, (const char *)scp->pMyThingName, JsonDocumentBuffer,
                            ShadowUpdateStatusCallback, NULL, 4, true);
                    { 
                        Update_Remote_devices_data(JsonDocumentBuffer, MAX_LENGTH_OF_UPDATE_JSON_BUFFER);
                    }

                    if (notify_thermo_breach)
                    {
                        memset(JsonDocumentBuffer,0, MAX_LENGTH_OF_UPDATE_JSON_BUFFER);
                        notify_thermostat_breach(JsonDocumentBuffer,MAX_LENGTH_OF_UPDATE_JSON_BUFFER);
                        notify_thermo_breach = 0;
                    }
                    
                    qurt_mutex_unlock(&shadow_update_lock);

#define AWS_UPDATE_TIMEOUT 3000
                    countdown_ms(&sendUpdateTimer, AWS_UPDATE_TIMEOUT);
                }
                IOT_INFO("*****************************************************************************************\n");

                app_msec_delay(1000);
            }

            if(SUCCESS != rc)
            {
                IOT_ERROR("An error occurred in the loop %d", rc);
            }
        }
        IOT_INFO("Disconnecting");
        rc = aws_iot_shadow_disconnect(mqttClient);
        if(SUCCESS != rc)
        {
            IOT_ERROR("Disconnect error %d", rc);
        }
        aws_running = 0;
    }

aws_conn_error:

    IOT_INFO("AWS THREAD EXITED !!!\n");
    aws_thread_exited = 1;
    /* Clean up connection again as sometimes aws fails to do it */
    qurt_signal_delete(&aws_signal);
    if(sp->pHost != NULL)
        free(sp->pHost);

    if(sp->pClientCRT != NULL)
        free(sp->pClientCRT);

    if(scp->pMyThingName != NULL)
        free(scp->pMyThingName);

    if(scp != NULL)
        free(scp);

    if(mqttClient != NULL)
        free(mqttClient);

    if(sp != NULL)
        free(sp);

    qurt_mutex_delete(&shadow_update_lock);
    /* clean up the thread */

    qurt_thread_stop();
}

/*
 * function@: is_aws_running
 * breif@: gives the status of aws send status is it running or stop
 */
boolean is_aws_running(void)
{
    return !!aws_running;
}

/*
 * function@: Start_aws
 * breif@: Set the signal to start aws  
 */
int32_t Start_aws(void)
{
    IOT_INFO("START AWS: running(%d)\n", aws_running);
    if (!aws_thread_exited && !aws_running)
    {
        qurt_signal_set(&aws_signal, SIGNAL_AWS_START_EVENT);
    }

    return SUCCESS;
}

/*
 * function@: Stop_aws
 * breif@: Set the signal to stop aws  
 */
int32_t Stop_aws(void)
{
    aws_running = 0;
    return SUCCESS;
}

/*
 * function@:Initialize_aws 
 * breif@: Initializes the aws thread 
 */
int32_t Initialize_aws(void)
{
    unsigned long task_priority = AWS_TASK_PRIORITY;
    int stack_size = AWS_TASK_STACK_SIZE;

    if (QURT_EOK != qurt_signal_init(&aws_signal))
    {
        IOT_INFO("Aws signal intialization failed\n");
        return FAILURE;
    }

   //create a thread
    qurt_thread_attr_t attr;
    qurt_thread_attr_init(&attr);
    qurt_thread_attr_set_name(&attr, "aws_thread");
    qurt_thread_attr_set_priority (&attr, task_priority);
    qurt_thread_attr_set_stack_size (&attr, stack_size);

    if (qurt_thread_create(&aws_thread_handle, &attr, aws_thread, NULL) != QURT_EOK)
    {
        IOT_ERROR("Failed to create aws thread");
        return FAILURE;
    }

    IOT_INFO("Initialized AWS\n");
    return SUCCESS;
}
