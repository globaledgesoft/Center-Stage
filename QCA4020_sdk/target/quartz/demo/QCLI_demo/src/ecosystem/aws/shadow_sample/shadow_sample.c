/*
 * Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
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

/*
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
 * @file shadow_sample.c
 * @brief A simple connected window example demonstrating the use of Thing Shadow
 */

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
#include "qapi_fs.h"
#include "qapi_crypto.h"

typedef struct jsonStruct_q jsonStruct_qt;
jsonStruct_qt jsonStructArr[10];

bool stop_aws = FALSE;

struct aws_params *aws_conn_data = NULL;
qurt_thread_t aws_thread;
int and_mask[MAX_JSON_OBJ_COUNT];
int or_mask[MAX_JSON_OBJ_COUNT];

extern QCLI_Group_Handle_t qcli_ecosystem_handle; /* Handle for ecosystem Command Group. */

#define IOT_INFO(...) QCLI_Printf(qcli_ecosystem_handle, __VA_ARGS__)
#define IOT_ERROR(...) QCLI_Printf(qcli_ecosystem_handle, __VA_ARGS__)

#define MAX_LENGTH_OF_UPDATE_JSON_BUFFER 2000
char *JsonDocumentBuffer;

void ShadowUpdateStatusCallback(const char *pThingName, ShadowActions_t action, Shadow_Ack_Status_t status,
                        const char *pReceivedJsonDocument, void *pContextData) {
    IOT_UNUSED(pThingName);
    IOT_UNUSED(action);
    IOT_UNUSED(pReceivedJsonDocument);
    IOT_UNUSED(pContextData);

    if(SHADOW_ACK_TIMEOUT == status) {
        IOT_INFO("Update Timeout-- \n");
    } else if(SHADOW_ACK_REJECTED == status) {
        IOT_INFO("Update RejectedXX \n");
    } else if(SHADOW_ACK_ACCEPTED == status) {
        IOT_INFO("Update Accepted !! \n");
    }
}

void windowActuate_Callback(const char *pJsonString, uint32_t JsonStringDataLen, jsonStruct_t *pContext) {

    if(pContext != NULL) {

    IOT_INFO("\n\n");

    switch(pContext->type) {

        case SHADOW_JSON_BOOL:
            IOT_INFO("Updated value recieved key=%s , Data=%s \n", pContext->pKey, (*(bool *)pContext->pData)?"true":"false");
            break;

        case SHADOW_JSON_UINT8:
            IOT_INFO("Updated value recieved key=%s , Data=%d \n", pContext->pKey, *(uint8_t *)pContext->pData);
            break;

        case SHADOW_JSON_UINT16:
            IOT_INFO("Updated value recieved key=%s , Data=%d \n", pContext->pKey, *(uint16_t *)pContext->pData);
            break;

        case SHADOW_JSON_UINT32:
            IOT_INFO("Updated value recieved key=%s , Data=%d \n", pContext->pKey, *(uint32_t *)pContext->pData);
            break;

        case SHADOW_JSON_INT8:
            IOT_INFO("Updated value recieved key=%s , Data=%d \n", pContext->pKey, *(int8_t *)pContext->pData);
            break;

        case SHADOW_JSON_INT16:
            IOT_INFO("Updated value recieved key=%s , Data=%d \n", pContext->pKey, *(int16_t *)pContext->pData);
            break;

        case SHADOW_JSON_INT32:
            IOT_INFO("Updated value recieved key=%s , Data=%d \n", pContext->pKey, *(int32_t *)pContext->pData);
            break;

        case SHADOW_JSON_FLOAT:
            IOT_INFO("Updated value recieved key=%s , Data=%f \n", pContext->pKey, *(float *)pContext->pData);
            break;

        case SHADOW_JSON_STRING:
            IOT_INFO("String value updates are not supported by AWS 2.1.1, recieved data key=%s ", pContext->pKey);
            break;

        case SHADOW_JSON_DOUBLE:
            IOT_INFO("Updated value recieved key=%s , Data=%ld \n", pContext->pKey, *(double *)pContext->pData);
            break;

        case SHADOW_JSON_OBJECT:
            IOT_ERROR("Json object is not supported in this demo \n");
            break;

        default:
            IOT_ERROR("Error: Invalid Json type \n");
            break;
    }
  }
}

void shadow_sample() {
    IoT_Error_t rc = FAILURE;
    char cert_stub[] = "this is a stub cert";
    ShadowInitParameters_t *sp = NULL;
    ShadowConnectParameters_t *scp = NULL;
    AWS_IoT_Client *mqttClient = NULL;
    int i = 0;
    Timer sendUpdateTimer;
    char *ca_list = "aws_ca_list.bin";

    IOT_INFO("\nAWS IoT SDK Version %d.%d.%d-%s\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_TAG);

    JsonDocumentBuffer = malloc(MAX_LENGTH_OF_UPDATE_JSON_BUFFER);

    if(JsonDocumentBuffer == NULL)
        goto aws_conn_error;

    size_t sizeOfJsonDocumentBuffer = MAX_LENGTH_OF_UPDATE_JSON_BUFFER;

    // initialize the mqtt client
    mqttClient = malloc(sizeof(AWS_IoT_Client));

    if(mqttClient == NULL)
        goto aws_conn_error;

    memset(mqttClient, 0, sizeof(AWS_IoT_Client));
    memset(JsonDocumentBuffer, 0, sizeOfJsonDocumentBuffer);

    sp = malloc(sizeof(ShadowInitParameters_t));

    if(sp == NULL)
        goto aws_conn_error;

    *sp = ShadowInitParametersDefault;
    sp->pHost = malloc(strlen(aws_conn_data->host_name) + 1);
    memcpy(sp->pHost, aws_conn_data->host_name, strlen(aws_conn_data->host_name) + 1);
    sp->port = AWS_IOT_MQTT_PORT;
    sp->pClientCRT = aws_conn_data->client_cert;
    sp->pClientKey = cert_stub;
    sp->pRootCA = ca_list;
    sp->enableAutoReconnect = false;
    sp->disconnectHandler = NULL;

    IOT_INFO("Shadow Init \n\n");
    rc = aws_iot_shadow_init(mqttClient, sp);

    if(SUCCESS != rc) {
        IOT_ERROR("Shadow Connection Error \n\n");
        goto aws_conn_error;
    }

    scp = malloc(sizeof(ShadowConnectParameters_t));

    if(scp == NULL)
        goto aws_conn_error;

    scp->pMyThingName = malloc(strlen(aws_conn_data->thing_name) + 1);

    if(scp->pMyThingName == NULL)
        goto aws_conn_error;

    memcpy(scp->pMyThingName, aws_conn_data->thing_name, strlen(aws_conn_data->thing_name) + 1);
    scp->pMqttClientId = AWS_IOT_MQTT_CLIENT_ID;
    scp->mqttClientIdLen = (uint16_t) strlen(AWS_IOT_MQTT_CLIENT_ID);

    IOT_INFO("Shadow Connect \n\n");
    rc = aws_iot_shadow_connect(mqttClient, scp);
    if(SUCCESS != rc) {
        IOT_ERROR("Shadow Connection Error \n\n");
        goto aws_conn_error;
    }

    IOT_INFO("Shadow Connection successful \n\n");

    for(i=0;i < aws_conn_data->num_objects;i++)
    {
        rc = aws_iot_shadow_register_delta(mqttClient, (jsonStruct_t *)&jsonStructArr[i]);

        if(SUCCESS != rc) {
            IOT_ERROR("Shadow Register Delta Error");
        }
    }

    /*
     * Enable Auto Reconnect functionality. Minimum and Maximum time of Exponential backoff are set in aws_iot_config.h
     *  #AWS_IOT_MQTT_MIN_RECONNECT_WAIT_INTERVAL
     *  #AWS_IOT_MQTT_MAX_RECONNECT_WAIT_INTERVAL
     */
    rc = aws_iot_shadow_set_autoreconnect_status(mqttClient, true);

    if(SUCCESS != rc) {
        IOT_ERROR("Unable to set Auto Reconnect to true - %d", rc);
        goto aws_conn_error;
    }

    init_timer(&sendUpdateTimer);
    countdown_ms(&sendUpdateTimer,0);

    stop_aws = FALSE;

    // loop and publish new data
    while(NETWORK_ATTEMPTING_RECONNECT == rc || NETWORK_RECONNECTED == rc || SUCCESS == rc) {

        rc = aws_iot_shadow_yield(mqttClient, 1000);
        if(NETWORK_ATTEMPTING_RECONNECT == rc) {
            app_msec_delay(1000);
            // If the client is attempting to reconnect we will skip the rest of the loop.
            continue;
        }

        if(has_timer_expired(&sendUpdateTimer)) {

            IOT_INFO("\n=======================================================================================\n");

            rc = aws_iot_shadow_init_json_document(JsonDocumentBuffer, sizeOfJsonDocumentBuffer);
            if(SUCCESS == rc) {

                rc = aws_iot_shadow_add_reported(JsonDocumentBuffer, sizeOfJsonDocumentBuffer, aws_conn_data->num_objects,
                     &jsonStructArr[0], &jsonStructArr[1], &jsonStructArr[2], &jsonStructArr[3], &jsonStructArr[4], &jsonStructArr[5],
                     &jsonStructArr[6], &jsonStructArr[7], &jsonStructArr[8], &jsonStructArr[9]);

                if(SUCCESS == rc) {
                     rc = aws_iot_finalize_json_document(JsonDocumentBuffer, sizeOfJsonDocumentBuffer);

                    if(SUCCESS == rc) {
                        IOT_INFO("Update Shadow: %s \n\n", JsonDocumentBuffer);
                        rc = aws_iot_shadow_update(mqttClient,(const char *)scp->pMyThingName, JsonDocumentBuffer,
                                           ShadowUpdateStatusCallback, NULL, (aws_conn_data->update_timer*2), true);
                    }
                }
            }
            IOT_INFO("*****************************************************************************************\n");
            IOT_INFO("Randomizing Data \n");
            shadow_sample_randomize_data();
            countdown_ms(&sendUpdateTimer,aws_conn_data->update_timer);
        }

        app_msec_delay(1000);

        if(stop_aws == TRUE)
            break;
    }

    if(SUCCESS != rc) {
        IOT_ERROR("An error occurred in the loop %d", rc);
    }

    IOT_INFO("Disconnecting");
    rc = aws_iot_shadow_disconnect(mqttClient);

    if(SUCCESS != rc) {
        IOT_ERROR("Disconnect error %d", rc);
    }

    aws_conn_error:

       /*clean up connection again as sometimes aws fails to do it */

       if(sp->pHost != NULL)
           free(sp->pHost);

       if(scp->pMyThingName != NULL)
           free(scp->pMyThingName);

       if(scp != NULL)
           free(scp);

       if(mqttClient != NULL)
           free(mqttClient);

       if(sp != NULL)
           free(sp);

       shadow_sample_cleanup();
       /* clean up the thread */
       qurt_thread_stop();
}

void shadow_sample_cleanup()
{

    int i = 0;

    if(aws_conn_data == NULL)
        return;

    /* free json objects and assigned data memory */
    for(i = 0; i < aws_conn_data->num_objects; i++)
    {
        if(jsonStructArr[i].pKey != NULL)
            free(jsonStructArr[i].pKey);

        if(jsonStructArr[i].pData != NULL)
            free(jsonStructArr[i].pData);
    }

    /* free aws conn context */
    if(aws_conn_data->host_name != NULL)
        free(aws_conn_data->host_name);

    if(aws_conn_data->thing_name != NULL)
        free(aws_conn_data->thing_name);

    if(aws_conn_data->file_name != NULL)
        free(aws_conn_data->file_name);

    if(aws_conn_data->client_cert != NULL)
        free(aws_conn_data->client_cert);

    if(aws_conn_data->schema_string != NULL)
        free(aws_conn_data->schema_string);

    if(aws_conn_data->sbuf != NULL)
        free(aws_conn_data->sbuf);

    if(aws_conn_data != NULL)
        free(aws_conn_data);

    aws_conn_data = NULL;

    for(i=0;i<MAX_JSON_OBJ_COUNT;i++)
    {
        jsonStructArr[i].pKey = NULL;
        jsonStructArr[i].pData = NULL;
    }

}

void shadow_sample_randomize_data() {

    int i = 0, temp = 0;

    for(i = 0; i < aws_conn_data->num_objects;i++)
    {
        temp = 0;
        /* switch data type */    
        switch(jsonStructArr[i].type) {

            case SHADOW_JSON_UINT8:
                qapi_Crypto_Random_Get(&temp, sizeof(uint8_t));
                *(uint8_t *)jsonStructArr[i].pData = (uint8)(((uint8_t)temp & (uint8_t)and_mask[i]) | (uint8_t)or_mask[i]); 
                break;

            case SHADOW_JSON_UINT16:
                qapi_Crypto_Random_Get(&temp, sizeof(uint16_t));
                *(uint16_t *)jsonStructArr[i].pData = (uint16_t)(((uint16_t)temp & (uint16_t)and_mask[i]) | (uint16_t)or_mask[i]); 
                break;

            case SHADOW_JSON_UINT32:
                qapi_Crypto_Random_Get(&temp, sizeof(uint32_t));
                *(uint32_t *)jsonStructArr[i].pData = (uint32_t)(((uint32_t)temp & (uint32_t)and_mask[i]) | (uint32_t)or_mask[i]); 
                break;

            case SHADOW_JSON_INT8:
                qapi_Crypto_Random_Get(&temp, sizeof(int8_t));
                *(int8_t *)jsonStructArr[i].pData = (int8_t)(((int8_t)temp & (int8_t)and_mask[i]) | (int8_t)or_mask[i]); 
                break;

            case SHADOW_JSON_INT16:
                qapi_Crypto_Random_Get(&temp, sizeof(int16_t));
                *(int16_t *)jsonStructArr[i].pData = (int16_t)(((int16_t)temp & (int16_t)and_mask[i]) | (int16_t)or_mask[i]); 
                break;

            case SHADOW_JSON_INT32:
                qapi_Crypto_Random_Get(&temp, sizeof(int32_t));
                *(int32_t *)jsonStructArr[i].pData = (int32_t)(((int32_t)temp & (int32_t)and_mask[i]) | (int32_t)or_mask[i]);
                break;

            case SHADOW_JSON_FLOAT:
            case SHADOW_JSON_BOOL:
            case SHADOW_JSON_DOUBLE:
            case SHADOW_JSON_STRING:
            case SHADOW_JSON_OBJECT:
                 break;

        }

    }

}

/* sample command aws_set_params "key" json_type value and_mask or_mask */
QCLI_Command_Status_t shadow_sample_set_params(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List) {

    int i = 0, index = 0;
    char *key;
    JsonPrimitiveType json_type;
    unsigned long temp_num;

    if(Parameter_Count < 3) {
        IOT_ERROR("All parameters are required \n");
        IOT_ERROR("Usage: aws_set_params json_key json_type value and_mask or_mask \n");
        return QCLI_STATUS_SUCCESS_E;
    }

    /* parse command line params */
    key = (char *)Parameter_List[0].String_Value;

    if(Parameter_List[1].Integer_Is_Valid == TRUE) {
        json_type = Parameter_List[1].Integer_Value;
    }
    else {
        IOT_ERROR("Please provide valid Json data type \n");
        return QCLI_STATUS_SUCCESS_E;
    }

    if(json_type > 10 || json_type == SHADOW_JSON_DOUBLE || json_type == SHADOW_JSON_OBJECT) {
        IOT_ERROR("Unsupported Json Data type \n");
        return QCLI_STATUS_SUCCESS_E;
    }

    index = -1;

    /* Find the key */
    for(i = 0;i < MAX_JSON_OBJ_COUNT; i++) {

        if(strlen(jsonStructArr[i].pKey) == strlen(key)) {
            if(memcmp(jsonStructArr[i].pKey, key, strlen(key)) == 0) {
                index = i;
                break;
            }
        }
    }

    if(index < 0) {
        IOT_ERROR("Key not found. Please provide a key which matches the provided schema \n"); 
        return QCLI_STATUS_SUCCESS_E;
    }

    if(Parameter_Count > 3 && Parameter_Count == 5 && json_type != SHADOW_JSON_STRING && json_type != SHADOW_JSON_BOOL) {
        and_mask[index] = Parameter_List[3].Integer_Value;
        or_mask[index]  = Parameter_List[4].Integer_Value;
    }
    else if ((json_type != SHADOW_JSON_STRING) && (json_type != SHADOW_JSON_BOOL)){
        IOT_ERROR("Parameter Count %d \n", Parameter_Count);
        IOT_ERROR("Both And and Or mask are required \n");
        IOT_ERROR("Mask are not applicable for string json data type \n");
        IOT_ERROR("Usage: aws_set_params json_key json_type value and_mask or_mask \n");
        return QCLI_STATUS_SUCCESS_E;
    }

    if(json_type != SHADOW_JSON_STRING && json_type != SHADOW_JSON_FLOAT && json_type != SHADOW_JSON_UINT32) {

        /* check if value is valid */
        if(Parameter_List[2].Integer_Is_Valid == FALSE) {
            IOT_ERROR("Please provide valid value \n");
            return QCLI_STATUS_SUCCESS_E;
        }
    }

    if(jsonStructArr[index].pData != NULL) {

        IOT_ERROR("Key was already assigned data, freeing current data & reassigning \n");
        free(jsonStructArr[index].pData);
        jsonStructArr[index].pData = NULL;
    }
    else {
        aws_conn_data->num_objects++;
    }

    switch(json_type) {

        case SHADOW_JSON_BOOL:
            jsonStructArr[index].pData = (bool *)malloc(sizeof(bool));
            *(bool *)jsonStructArr[index].pData = (bool)Parameter_List[2].Integer_Value;
            jsonStructArr[index].type  = json_type;
            break;

        case SHADOW_JSON_UINT8:
            jsonStructArr[index].pData = (uint8_t *)malloc(sizeof(uint8_t));
            *(uint8_t *)jsonStructArr[index].pData = (uint8_t)Parameter_List[2].Integer_Value;
            jsonStructArr[index].type  = json_type;
            break;

        case SHADOW_JSON_UINT16:
            jsonStructArr[index].pData = (uint16_t *)malloc(sizeof(uint16_t));
            *(uint16_t *)jsonStructArr[index].pData = (uint16_t)Parameter_List[2].Integer_Value;
            jsonStructArr[index].type  = json_type;
            break;

        case SHADOW_JSON_UINT32:
            temp_num = strtoul(Parameter_List[2].String_Value, NULL, 10);
            if (temp_num == ULONG_MAX)
            {
                IOT_ERROR("Invalid input provided, please provide valid uint32 value \n");
                return QCLI_STATUS_SUCCESS_E;
            }
            else if(temp_num == 0)
                IOT_INFO("Either invalid data provided or set to zero, continuing with zero \n");

            jsonStructArr[index].pData = (uint32_t *)malloc(sizeof(uint32_t));
            *(uint32_t *)jsonStructArr[index].pData = (uint32_t)temp_num;
            jsonStructArr[index].type  = json_type;
            break;

        case SHADOW_JSON_INT8:
            jsonStructArr[index].pData = (int8_t *)malloc(sizeof(int8_t));
            *(int8_t *)jsonStructArr[index].pData = (int8_t)Parameter_List[2].Integer_Value;
            jsonStructArr[index].type  = json_type;
            break;

        case SHADOW_JSON_INT16:
            jsonStructArr[index].pData = (int16_t *)malloc(sizeof(int16_t));
            *(int16_t *)jsonStructArr[index].pData = (int16_t)Parameter_List[2].Integer_Value;
            jsonStructArr[index].type  = json_type;
            break;

        case SHADOW_JSON_INT32:
            jsonStructArr[index].pData = (int32_t *)malloc(sizeof(int32_t));
            *(int32_t *)jsonStructArr[index].pData = (int32_t)Parameter_List[2].Integer_Value;
            jsonStructArr[index].type  = json_type;
            break;

        case SHADOW_JSON_FLOAT:
            jsonStructArr[index].pData = (float *)malloc(sizeof(float));
            *(float *)jsonStructArr[index].pData = (float)atof(Parameter_List[2].String_Value);
            jsonStructArr[index].type  = json_type;
            break;

        case SHADOW_JSON_STRING:
            jsonStructArr[index].pData = (char *)malloc(AWS_MAX_STRING_SIZE);
            memcpy(jsonStructArr[index].pData, (void *)Parameter_List[2].String_Value, strlen(Parameter_List[2].String_Value) + 1);
            jsonStructArr[index].type  = json_type;
            break;

        case SHADOW_JSON_DOUBLE:
            jsonStructArr[index].pData = (double *)malloc(sizeof(double));
            *(float *)jsonStructArr[index].pData = (double)Parameter_List[2].Integer_Value;
            jsonStructArr[index].type  = json_type;
            break;

        case SHADOW_JSON_OBJECT:
            IOT_ERROR("Json object is not supported in this demo \n");
            break;

        default:
            IOT_ERROR("Error: Invalid Json type \n");
            break;

    }

    jsonStructArr[index].cb = windowActuate_Callback;

    return QCLI_STATUS_SUCCESS_E;

}

QCLI_Command_Status_t shadow_sample_init(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List) {

    int i = 0, j = 0;
    int fd;
    jsmntok_t *aJsonHandler = NULL;
    int32_t aTokenCount;
    uint32_t bReadPtr;

    if(Parameter_Count < 5) {
        IOT_ERROR("Please provide Host_Name, Thing_Name, File_Name, Dev_cert_name and update_time \n");
        IOT_ERROR("Usage: aws_set_schema host_name thing_name file_name dev_cert_name update_time \n");
        goto cleanup_aws;
    }

    /* flag to know if main loop is running */
    stop_aws = TRUE;

    aws_conn_data = malloc(sizeof(struct aws_params));

    if(aws_conn_data == NULL)
        goto cleanup_aws;

    aws_conn_data->schema_string = NULL;
    aws_conn_data->num_objects = 0;
    aws_conn_data->update_timer = 0;

    aws_conn_data->host_name   = (char *)malloc(AWS_MAX_DOMAIN_SIZE);

    if(aws_conn_data->host_name == NULL)
        goto cleanup_aws;

    aws_conn_data->thing_name  = (char *)malloc(AWS_MAX_THING_SIZE);

    if(aws_conn_data->thing_name == NULL)
        goto cleanup_aws;

    aws_conn_data->file_name   = (char *)malloc(AWS_MAX_FILE_NAME);

    if(aws_conn_data->file_name == NULL)
        goto cleanup_aws;

    aws_conn_data->client_cert = (char *)malloc(AWS_MAX_CERT_NAME);

    if(aws_conn_data->client_cert == NULL)
        goto cleanup_aws;

    aws_conn_data->sbuf        = malloc(sizeof(struct qapi_fs_stat_type));

    if(aws_conn_data->sbuf == NULL)
        goto cleanup_aws;

    for(i=0;i<MAX_JSON_OBJ_COUNT;i++)
    {
        jsonStructArr[i].pKey = NULL;
        jsonStructArr[i].pData = NULL;
        and_mask[i] = ~(0);
        or_mask[i]  = 0;
    }

    memcpy((void *)aws_conn_data->host_name, (void *)Parameter_List[0].String_Value, strlen(Parameter_List[0].String_Value) + 1);
    memcpy((void *)aws_conn_data->thing_name, (void *)Parameter_List[1].String_Value, strlen(Parameter_List[1].String_Value) + 1);
    memcpy((void *)aws_conn_data->file_name, (void *)Parameter_List[2].String_Value, strlen(Parameter_List[2].String_Value) + 1);
    memcpy((void *)aws_conn_data->client_cert, (void *)Parameter_List[3].String_Value, strlen(Parameter_List[3].String_Value) + 1);
    aws_conn_data->update_timer = Parameter_List[4].Integer_Value;

    /* open the schema file & get stats */
    if(qapi_Fs_Open(aws_conn_data->file_name, QAPI_FS_O_RDONLY, &fd) != QAPI_OK) {
        IOT_ERROR("Unable to open file %s \n", aws_conn_data->file_name);
        goto cleanup_aws;
    }

    if(qapi_Fs_Stat(aws_conn_data->file_name, aws_conn_data->sbuf) != QAPI_OK) {
        IOT_ERROR("Unable to stat file %s \n", aws_conn_data->file_name);
        goto cleanup_aws;
    }

    aws_conn_data->schema_string = (char *)malloc(aws_conn_data->sbuf->st_size);

    if(aws_conn_data->schema_string == NULL)
        goto cleanup_aws;

    /* read the schema file */
    if(qapi_Fs_Read(fd, (uint8_t *) aws_conn_data->schema_string, aws_conn_data->sbuf->st_size, &bReadPtr) != QAPI_OK) {
        IOT_ERROR("Unable to read file %s \n", aws_conn_data->file_name);
        goto cleanup_aws; 
    }

    /* close schema file */
    qapi_Fs_Close(fd);

    if(!isReceivedJsonValid((const char *)aws_conn_data->schema_string)) {
        IOT_ERROR("Not a valid json document \n");
        goto cleanup_aws;
    }

    /*parse schema file */
    if(!isJsonValidAndParse((const char *)aws_conn_data->schema_string, (void **)&aJsonHandler,&aTokenCount)) {
        IOT_ERROR("invalid json document \n");
        goto cleanup_aws;
    }

    IOT_WARN("Host Name: %s \n",  aws_conn_data->host_name);
    IOT_WARN("Thing Name: %s \n", aws_conn_data->thing_name);
    IOT_WARN("File Name: %s \n",  aws_conn_data->file_name);
    IOT_WARN("Certificate Name: %s \n", aws_conn_data->client_cert);
    IOT_WARN("Update time: %d \n", aws_conn_data->update_timer);
    

    /* create json objects, skip first token since its root object */
    for(i = 1;i < aTokenCount;i++) {

        if(aJsonHandler[i].size != 0) {

            if(j == MAX_JSON_OBJ_COUNT) {
                IOT_ERROR("ERROR: Max 10 JSON objects supported \n");
                goto cleanup_aws;
            }
        
            jsonStructArr[j].pKey = malloc(MAX_JSON_KEY_SIZE);
            memcpy((void *)jsonStructArr[j].pKey, (void *)&aws_conn_data->schema_string[aJsonHandler[i].start], (aJsonHandler[i].end - aJsonHandler[i].start));
            jsonStructArr[j].pKey[((aJsonHandler[i].end - aJsonHandler[i].start))] = '\0';
            jsonStructArr[j].type = aJsonHandler[i].type;

            IOT_WARN("New Object Created Key = %s\n", jsonStructArr[j].pKey);
            j++;
        }

    }

    return QCLI_STATUS_SUCCESS_E;

    cleanup_aws:

    shadow_sample_cleanup();

    return QCLI_STATUS_SUCCESS_E;
}
QCLI_Command_Status_t shadow_sample_run(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List) {

    unsigned long task_priority = AWS_TASK_PRIORITY;
    int stack_size = AWS_TASK_STACK_SIZE, i = 0;

    if(aws_conn_data->thing_name == NULL || aws_conn_data->host_name == NULL) {
        IOT_ERROR("Please setup aws before connecting, use aws_set_schema command \n");
        return QCLI_STATUS_SUCCESS_E;
    }

    for(i=0;i<MAX_JSON_OBJ_COUNT;i++) {

        if(jsonStructArr[i].pKey != NULL) {
            if(jsonStructArr[i].pData == NULL) {
                IOT_ERROR("Data for key %s not assigned, all keys should have data assigned to it \n", jsonStructArr[i].pKey);
                return QCLI_STATUS_SUCCESS_E;
            }
        }
    }

    //create a thread
    qurt_thread_attr_t attr;
    //qurt_thread_t handle;
    qurt_thread_attr_init (&attr);
    qurt_thread_attr_set_name (&attr, "shadow_sample");
    qurt_thread_attr_set_priority (&attr, task_priority);
    qurt_thread_attr_set_stack_size (&attr, stack_size);

    if(qurt_thread_create(&aws_thread, &attr, (void *)shadow_sample, NULL) != QURT_EOK)
        IOT_ERROR("failed to create aws thread");

    return QCLI_STATUS_SUCCESS_E;
}


QCLI_Command_Status_t shadow_sample_destroy(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List) {

   /*demo is not running, treat as cleanup command */
   if(stop_aws == TRUE) {
       shadow_sample_cleanup();
   }
   else {
       //Stop the shadow sample demo, this clean up everything if sample is running
       stop_aws = TRUE;
   }

   return TRUE;
}


