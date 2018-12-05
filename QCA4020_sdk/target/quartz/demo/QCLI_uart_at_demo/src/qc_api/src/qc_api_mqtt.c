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

/*-------------------------------------------------------------------------
 *  Include Files
 *-----------------------------------------------------------------------*/
#include "qc_api_mqtt.h"
#include "qc_api_main.h"
#include "qc_drv_mqtt.h"
#include "qosa_util.h"

/*-------------------------------------------------------------------------
 * Static & global Variable Declarations
 *-----------------------------------------------------------------------*/
#define HEXDUMP(inbuf, inlen, ascii, addr)  app_hexdump(inbuf, inlen, ascii, addr)

static const char *mqtt_connect_cbk_status[] =
{
    "MQTT connect succeeded",
    "Invalid protocol version",
    "Client id not allowed",
    "Server unavailable",
    "Bad user name or password",
    "Client not authorized",
    "Sending CONNECT failed",
    "Reading CONNACK failed",
    "Invalid CONNACK received",
    "CONNACK not received",
    "TCP connect failed",
    "SSL connect failed",
};

static qapi_Status_t status;

static const char *MQTT_subscription_cbk_reasons[] =
{
    "subscription denied",
    "subscription granted",
    "received app message from broker"
};

/* Support 4 instances */
#define NUM_SESSIONS    4
static int mqttc_handle[NUM_SESSIONS];
static qapi_Net_SSL_Config_t * mqttc_sslcfg[NUM_SESSIONS];

#define CONNECT_OK      0x1
#define CONNECT_ERROR   0x2
#define SUBACK_EVENT    0x3
#define PUBLISH_EVENT   0x4

/*****************************************************************************
 * RETURN
 * -1   cannot find it
 *  >0  find it!!
 *****************************************************************************/
static int find_session(int handle)
{
    int i;

    for (i = 0; i < NUM_SESSIONS; ++i)
    {
        if (mqttc_handle[i] == handle)
        {
            return i;
        }
    }
    return QAPI_ERROR;
}

/*****************************************************************************
 *****************************************************************************/
static char * get_status_string(qapi_Status_t status)
{
    char *str;

    switch (status)
    {
        case QAPI_NET_STATUS_MQTTC_CONNECT_SUCCEEDED:
            str = "MQTT connect succeeded";
            break;

        case QAPI_NET_STATUS_MQTTC_INVALID_PROTOCOL_VERSION:
            str = "Invalid protocol version";
            break;

        case QAPI_NET_STATUS_MQTTC_CLIENT_ID_REJECTED:
            str = "Client id not allowed";
            break;

        case QAPI_NET_STATUS_MQTTC_MQTT_SERVICE_UNAVAILABLE:
            str = "Server unavailable";
            break;

        case QAPI_NET_STATUS_MQTTC_INVALID_USERNAME_PASSWORD:
            str = "Bad user name or password";
            break;

        case QAPI_NET_STATUS_MQTTC_NOT_AUTHORIZED:
            str = "Client not authorized";
            break;

        case QAPI_NET_STATUS_MQTTC_CONNACK_NOT_RECEIVED:
            str = "CONNACK not received";
            break;

        case QAPI_NET_STATUS_MQTTC_TCP_CONNECT_FAILED:
            str = "TCP connect failed";
            break;

        case QAPI_NET_STATUS_MQTTC_SSL_HANDSHAKE_FAILED:
            str = "SSL connect failed";
            break;

        case QAPI_NET_STATUS_MQTTC_CLIENT_NOT_STARTED:
            str = "Client not inited yet";
            break;

        case QAPI_NET_STATUS_MQTTC_CLIENT_EXISTED:
            str = "Client existed already";
            break;

        case QAPI_NET_STATUS_MQTTC_TCP_CONNECTION_CLOSED:
            str = "Server closed connection";
            break;

        default:
            str = "";
            break;
    }

    return str;
}

/*****************************************************************************
 *****************************************************************************/
static void mqtt_connect_cbk(int32_t handle, void *arg, qapi_Status_t status)
{
    LOG_AT_EVT("\r\nEVT_MQTT: %s: handle %x arg %x status %d (%s)\n", __func__,
            handle, arg, status, mqtt_connect_cbk_status[status]);

    if (arg != NULL)
    {
        if (status == QAPI_NET_STATUS_MQTTC_CONNECT_SUCCEEDED)
        {
            qurt_signal_set((qurt_signal_t *)arg, CONNECT_OK);
        }
        else
        {
            qurt_signal_set((qurt_signal_t *)arg, CONNECT_ERROR);
        }
    }

    return;
}

/****************************************************************************
 ***************************************************************************/
static void mqtt_subscribe_cbk(int32_t handle, void *arg, int32_t reason,
        const char *topic, uint16_t topic_length,
        const char *msg, uint32_t msg_length,
        uint32_t qos)
{
    LOG_AT_EVT("\r\nEVT_MQTT: %s: handle %x arg %x reason %d (%s) topic %p topiclen %d msg %p msglen %d qos %d\n",
            __func__, handle, arg, reason, MQTT_subscription_cbk_reasons[reason],
            topic, topic_length, msg, msg_length, qos);

    LOG_AT_EVT("EVT_MQTT: Topic: ");
    HEXDUMP((char *)topic, topic_length, true, true);

    if (msg != NULL)
    {
        LOG_AT_EVT("EVT_MQTT: Received message: ");
        HEXDUMP((char *)msg, msg_length, true, true);
    }

    if (arg != NULL)
    {
        if (reason == QAPI_NET_MQTTC_SUBSCRIPTION_DENIED ||
                reason == QAPI_NET_MQTTC_SUBSCRIPTION_GRANTED)
        {
            qurt_signal_set((qurt_signal_t *)arg, SUBACK_EVENT);
        }
        else
        {
            qurt_signal_set((qurt_signal_t *)arg, PUBLISH_EVENT);
        }
    }
}

/***********************************************************************************************
 * Function Name  : qc_api_mqttc_init
 * Returned Value : 0 on success or 1 on error
 * Comments       : Initialize the MQTT service
 **********************************************************************************************/
QCLI_Command_Status_t qc_api_mqttc_init(uint32_t Parameter_Count, char *ca_file)
{
    if (0 != qc_drv_net_mqtt_init(qc_api_get_qc_drv_context(), ca_file))
    {
        LOG_ERR("MQTTC init failed\r\n");
        return QCLI_STATUS_ERROR_E;
    }

    return QCLI_STATUS_SUCCESS_E;
}

/***********************************************************************************************
 * Function Name  : qc_api_mqttc_shut
 * Returned Value : 0 on success or 1 on error
 * Comments       : Shutdown MQTT service
 **********************************************************************************************/
QCLI_Command_Status_t qc_api_mqttc_shut(void)
{
    qc_drv_net_mqtt_shutdown(qc_api_get_qc_drv_context());
    memset(mqttc_handle, 0, sizeof(mqttc_handle));

    return QCLI_STATUS_SUCCESS_E;
}

/***********************************************************************************************
 * Function Name  : qc_api_mqttc_new
 * Returned Value : 0 on success or 1 on error
 * Comments       : Creates a MQTT session, taking the client name and the session ID
 **********************************************************************************************/
QCLI_Command_Status_t qc_api_mqttc_new(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int32_t e, id;
    char *client_id = NULL;
    qbool_t clean_session = true;

    id = find_session(0);
    if (id == QAPI_ERROR)
    {
        LOG_WARN("Cannot create more than %d sessions\r\n", NUM_SESSIONS);
        return QCLI_STATUS_ERROR_E;
    }

    if (Parameter_Count >= 1)
    {
        client_id = Parameter_List[0].String_Value;
        if (Parameter_Count == 2)
        {
            clean_session = !!Parameter_List[1].Integer_Value;
        }
    }

    e = qc_drv_net_mqtt_new(qc_api_get_qc_drv_context(), (const char *)client_id, clean_session,
            &status);
    if (e == QAPI_ERROR)
    {
        LOG_ERR("Failed to create an session. err %d (%s)\n", status, get_status_string(status));
        return QCLI_STATUS_ERROR_E;
    }

    mqttc_handle[id] = e;
    LOG_INFO("Session created. Id %d handle %x\n", id, e);

    return QCLI_STATUS_SUCCESS_E;
}

/***********************************************************************************************
 * Function Name  : qc_api_mqttc_destroy
 * Returned Value : 0 on success or 1 on error
 * Comments       : Destroys the MQTT session, taking the session ID as the parameter for
                    which session to destroy
 **********************************************************************************************/
QCLI_Command_Status_t qc_api_mqttc_destroy(uint32_t Parameter_Count, int32_t id)
{
    int32_t handle;

    handle = mqttc_handle[id];
    if (0 != qc_drv_net_mqtt_destroy(qc_api_get_qc_drv_context(), handle))
    {
        LOG_ERR("MQTTC destroy failed. err %d\n", status);
        return QCLI_STATUS_ERROR_E;
    }
    mqttc_handle[id] = 0;
    if (mqttc_sslcfg[id])
    {
        /* sslcfg block may have other memory allocation.
         * Need to free them first.
         */
        ssl_free_config_parameters(mqttc_sslcfg[id]);
        free(mqttc_sslcfg[id]);
        mqttc_sslcfg[id] = NULL;
    }
    LOG_INFO("Session %d is deleted. handle %x\n", id, handle);

    return QCLI_STATUS_SUCCESS_E;
}

/***********************************************************************************************
 * Function Name  : qc_api_mqttc_config
 * Returned Value : 0 on success or 1 on error
 * Comments       : Configures the MQTT client parameters like username, password, topic,
                    message, QOS
 **********************************************************************************************/
QCLI_Command_Status_t qc_api_mqttc_config(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    const char *user = NULL, *pw = NULL;
    const char *topic = NULL;
    const char *msg = NULL;
    uint32_t qos = 0;
    int32_t id, handle, e;
    qbool_t retained = false;


    id     = Parameter_List[0].Integer_Value;
    handle = mqttc_handle[id];
    user   = Parameter_List[1].String_Value;
    pw     =   Parameter_List[2].String_Value;
    if (Parameter_Count > 3)
    {
        topic = Parameter_List[3].String_Value;
        msg   = Parameter_List[4].String_Value;
        qos   = Parameter_List[5].Integer_Value;
    }
    e = qc_drv_net_mqtt_set_username_password(qc_api_get_qc_drv_context(), handle, user,
                            strlen(user), pw, strlen(pw), &status);
    if (e == QAPI_ERROR)
    {
        LOG_ERR("Failed to set username/password: %s/%s. err %d\n", user, pw, status);
        return QCLI_STATUS_ERROR_E;
    }
    e = qc_drv_net_mqtt_set_will(qc_api_get_qc_drv_context(), handle, topic, strlen(topic),
                            msg, strlen(msg), qos, retained, &status);
    if (e == QAPI_ERROR)
    {
        LOG_ERR("Failed to set Will. err %d\n", status);
        return QCLI_STATUS_ERROR_E;
    }

    return QCLI_STATUS_SUCCESS_E;
}

/***********************************************************************************************
 * Function Name  : qc_api_mqttc_connect
 * Returned Value : 0 on success or 1 on error
 * Comments       : Connect to the MQTT server
 **********************************************************************************************/
QCLI_Command_Status_t qc_api_mqttc_connect(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    char *host;
    char *bind_if = NULL;
    int32_t id, handle, e;
    qbool_t secure_session = false;
    qbool_t nonblocking_connect = false;
    uint16_t keepalive_sec = 0;         /* disable sending keepalive packet */
    uint16_t max_conn_pending_sec = 5;  /* default = 5 sec */

    /* session_ID */
    id = Parameter_List[0].Integer_Value;
    if (id < 0 || id >= NUM_SESSIONS)
    {
        LOG_WARN("Session id should be less than %d\n", NUM_SESSIONS);
        return QCLI_STATUS_ERROR_E;
    }
    handle = mqttc_handle[id];
    /* Server_IP */
    host = Parameter_List[1].String_Value;
    /* SS */
    secure_session = !!Parameter_List[2].Integer_Value;
    /* NB */
    nonblocking_connect = !!Parameter_List[3].Integer_Value;
    /* Keep_alive */
    keepalive_sec = Parameter_List[4].Integer_Value;
    /* Wait_time */
    max_conn_pending_sec = Parameter_List[5].Integer_Value;
    /* Interface_name */
    bind_if = Parameter_List[6].String_Value;

    qapi_Net_MQTTc_Register_Connect_Callback(handle, mqtt_connect_cbk, (void *)NULL);
    qapi_Net_MQTTc_Register_Subscribe_Callback(handle, mqtt_subscribe_cbk, (void *)NULL);
    qc_drv_net_mqtt_set_keep_alive(qc_api_get_qc_drv_context(), handle, keepalive_sec);
    qc_drv_net_mqtt_set_connack_wait_time(qc_api_get_qc_drv_context(), handle, max_conn_pending_sec);

    if (secure_session && mqttc_sslcfg[id] != NULL)
    {
        e = qc_drv_net_mqtt_set_ssl_config(qc_api_get_qc_drv_context(), handle,
                mqttc_sslcfg[id], &status);
        if (e == QAPI_ERROR)
        {
            LOG_WARN("SSL config failed. err %d\n", status);
            return QCLI_STATUS_ERROR_E;
        }
    }
    e = qc_drv_net_mqtt_connect(qc_api_get_qc_drv_context(), handle, host, secure_session,
            nonblocking_connect, bind_if, &status);
    if (e == QAPI_ERROR)
    {
        LOG_ERR("MQTTC connect failed. err %d\n", status);
        return QCLI_STATUS_ERROR_E;
    }

    return QCLI_STATUS_SUCCESS_E;
}

/***********************************************************************************************
 * Function Name  : qc_api_mqttc_sub
 * Returned Value : 0 on success or 1 on error
 * Comments       : Subscribe the topic
 **********************************************************************************************/
QCLI_Command_Status_t qc_api_mqttc_sub(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    const char *topic = NULL;
    uint32_t qos = 2;
    int32_t id, handle, e;

    id    = Parameter_List[0].Integer_Value;
    topic = Parameter_List[1].String_Value;
    qos   = Parameter_List[2].Integer_Value;
    handle = mqttc_handle[id];

    e = qc_drv_net_mqtt_subscribe(qc_api_get_qc_drv_context(), handle, topic, qos, &status);
    if (e == QAPI_ERROR)
    {
        LOG_ERR("MQTTc subscribe failed. err %d\n", status);
        return QCLI_STATUS_ERROR_E;
    }

    return QCLI_STATUS_SUCCESS_E;
}
/***********************************************************************************************
 * Function Name  : qc_api_mqttc_pub
 * Returned Value : 0 on success or 1 on error
 * Comments       : Publish the message about some certain topics
 **********************************************************************************************/
QCLI_Command_Status_t qc_api_mqttc_pub(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    const char *topic = NULL;
    const char *msg = NULL;
    uint32_t msg_len = 0;
    uint32_t qos = 2;
    int32_t id, handle, e;
    qbool_t retained = false;
    qbool_t dup = false;


    id       = Parameter_List[0].Integer_Value;
    handle   = mqttc_handle[id];
    topic    = Parameter_List[1].String_Value;
    if (Parameter_Count > 2)
    {
        qos      = Parameter_List[2].Integer_Value;
        msg      = Parameter_List[3].String_Value;
        msg_len  = strlen(msg);
        retained = !!Parameter_List[4].Integer_Value;
        dup      = !!Parameter_List[5].Integer_Value;
    }
    e = qc_drv_net_mqtt_publish(qc_api_get_qc_drv_context(), handle, topic, strlen(topic),
                            msg, msg_len, qos, retained, dup, &status);
    if (e == QAPI_ERROR)
    {
        LOG_ERR("MQTTc publish failed. err %d\n", status);
        return QCLI_STATUS_ERROR_E;
    }

    return QCLI_STATUS_SUCCESS_E;
}

/***********************************************************************************************
 * Function Name  : qc_api_mqttc_unsub
 * Returned Value : 0 on success or 1 on error
 * Comments       : Un-subscribe from a topic
 **********************************************************************************************/
QCLI_Command_Status_t qc_api_mqttc_unsub(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int32_t id, handle, e;
    const char *topic = NULL;

    id     = Parameter_List[0].Integer_Value;
    handle = mqttc_handle[id];
    topic  = Parameter_List[1].String_Value;

    e = qc_drv_net_mqtt_unsubscribe(qc_api_get_qc_drv_context(), handle, topic, &status);
    if (e == QAPI_ERROR)
    {
        LOG_ERR("MQTTC unsubscribe failed. err %d\n", status);
        return QCLI_STATUS_ERROR_E;
    }

    return QCLI_STATUS_SUCCESS_E;
}

/***********************************************************************************************
 * Function Name  : qc_api_mqttc_disconnect
 * Returned Value : 0 on success or 1 on error
 * Comments       : Disconnect from the MQTT server
 **********************************************************************************************/
QCLI_Command_Status_t qc_api_mqttc_disconnect(uint32_t Parameter_Count, int32_t id)
{
    int32_t handle, e;

    handle = mqttc_handle[id];
    e = qc_drv_net_mqtt_disconnect(qc_api_get_qc_drv_context(), handle, &status);
    if (e == QAPI_ERROR)
    {
        LOG_ERR("MQTTC disconnect failed. err %d\n", status);
        return QCLI_STATUS_ERROR_E;
    }

    return QCLI_STATUS_SUCCESS_E;
}

/***********************************************************************************************
 * Function Name  : qc_api_mqttc_sslconfig
 * Returned Value : 0 on success or 1 on error
 * Comments       : For configuring the SSL server/client parameters
 **********************************************************************************************/
QCLI_Command_Status_t qc_api_mqttc_sslconfig(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int32_t id;
    QCLI_Command_Status_t sts;
    qapi_Net_SSL_Config_t *cfg;

    id  = Parameter_List[1].Integer_Value;
    cfg = mqttc_sslcfg[id];
    if (cfg == NULL)
    {
        cfg = malloc(sizeof(qapi_Net_SSL_Config_t));
        if (cfg == NULL)
        {
            LOG_ERR("Allocation failure\n");
            return QCLI_STATUS_ERROR_E;
        }
        memset(cfg, 0, sizeof(qapi_Net_SSL_Config_t));
        mqttc_sslcfg[id] = cfg;
    }

    /* Parse SSL config parameters from command line */
    sts = ssl_parse_config_parameters(
            Parameter_Count-2,
            &Parameter_List[2],
            cfg,
            0,
            false);
    if (sts == QCLI_STATUS_ERROR_E)
    {
        /* sslcfg block may have other memory allocation.
         * Need to free them first.
         */
        ssl_free_config_parameters(cfg);
        free(cfg);
        mqttc_sslcfg[id] = NULL;
        return QCLI_STATUS_ERROR_E;
    }

    /* MQTTC always uses TLS. If configured for DTLS, set it to TLS */
    if (cfg->protocol <= QAPI_NET_SSL_DTLS_E ||
            cfg->protocol > QAPI_NET_SSL_PROTOCOL_TLS_1_2)
    {
        cfg->protocol = 0;
    }

    return QCLI_STATUS_SUCCESS_E;
}

