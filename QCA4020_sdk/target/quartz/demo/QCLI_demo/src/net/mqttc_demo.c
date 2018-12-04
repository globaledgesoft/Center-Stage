/*
* Copyright (c) 2017 Qualcomm Technologies, Inc.
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

#include <stdlib.h>
#include <string.h>
#include "qcli_api.h"
#include "qurt_signal.h"
#include "bench.h"
#include "netutils.h"
#include "qapi_mqttc.h"

#ifdef CONFIG_NET_MQTTC_DEMO
extern QCLI_Group_Handle_t qcli_net_handle;

#define PRINTF(fmt, args...)                QCLI_Printf(qcli_net_handle, fmt, ## args)
#define HEXDUMP(inbuf, inlen, ascii, addr)  app_hexdump(inbuf, inlen, ascii, addr)

/* topic_0 = "quartz/+/status" */
static const char topic_0[] =
{
   'q', 'u', 'a', 'r', 't', 'z', '/', '+', '/', 's', 't', 'a', 't', 'u', 's'
};
/* topic_1 = "quartz/dev1/+" */
static const char topic_1[] =
{
   'q', 'u', 'a', 'r', 't', 'z', '/', 'd', 'e', 'v', '1', '/', '+'
};
static const char *topics[] = {topic_0, topic_1};
static uint32_t topics_len[] = {sizeof(topic_0), sizeof(topic_1)};
static uint32_t topics_qos[] = {2, 1};

static const char pubmsg_0[] =  /* "dev0 is up." */
{
   'd', 'e', 'v', '0', ' ', 'i', 's', ' ', 'u', 'p', '.'
};
static const char *pubmsg_1 = "dev1 config is done.";

static const char *pubtopic_0 = "quartz/dev0/status";
static const char *pubtopic_1 = "quartz/dev1/config";

static const char *MQTT_subscription_cbk_reasons[] =
{
   "subscription denied",
   "subscription granted",
   "received app message from broker"
};

/* Support 4 instances */
#define NUM_SESSIONS    4
static int mqttc_handle[NUM_SESSIONS];
#ifdef CONFIG_NET_SSL_DEMO
static qapi_Net_SSL_Config_t * mqttc_sslcfg[NUM_SESSIONS];
#endif

#define CONNECT_OK      0x1
#define CONNECT_ERROR   0x2
#define SUBACK_EVENT    0x4
#define PUBLISH_EVENT   0x8

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

#ifdef CONFIG_NET_SSL_DEMO
        case QAPI_NET_STATUS_MQTTC_SSL_HANDSHAKE_FAILED:
            str = "SSL connect failed";
            break;
#endif
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
    PRINTF("\n%s: handle %x arg %x status %d (%s)\n", __func__,
            handle, arg, status, get_status_string(status));

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
    PRINTF("\n%s: handle %x arg %x reason %d (%s) topic %p topiclen %d msg %p msglen %d qos %d\n",
            __func__, handle, arg, reason, MQTT_subscription_cbk_reasons[reason],
            topic, topic_length, msg, msg_length, qos);

    PRINTF("Topic:\n");
    HEXDUMP((char *)topic, topic_length, true, false);

    if (msg != NULL)
    {
        PRINTF("Received message:\n");
        HEXDUMP((char *)msg, msg_length, true, false);
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

/*****************************************************************************
 *****************************************************************************/
static void mqttc_help(void)
{
    PRINTF("mqttc init [<calistfile>]\n");
    PRINTF("mqttc shutdown\n");
    PRINTF("mqttc new [<clientid_string>] [<clean_session_flag>]\n");
    PRINTF("mqttc destroy <session_id>\n");
    PRINTF("mqttc config <session_id> [-u <user> -P <pw>] [-t <will topic> -m <will msg> -q <will QOS> -r]\n");
    PRINTF("mqttc connect <session_id> <svr> [-s] [-n] [-k <keepalive_sec>] [-w <connack_wait_sec>] [-i <bind_if>]\n");
    PRINTF("mqttc subscribe <session_id> -t <topic filter> [-q <requested QOS>]\n");
    PRINTF("mqttc publish <session_id> -t <topic> [-q <QOS level>] [-m <message>] [-r] [-d]\n");
    PRINTF("mqttc unsubscribe <session_id> -t <topic filter>\n");
    PRINTF("mqttc disconnect <session_id>\n");
#ifdef CONFIG_NET_SSL_DEMO
    sslconfig_help("mqttc sslconfig <session_id>");
#endif

    PRINTF("Examples:\n");
    PRINTF(" mqttc init\n");
    PRINTF(" mqttc new quartz-mqtt-client 1\n");
    PRINTF(" mqttc destroy 0\n");
    PRINTF(" mqttc config 0 -u admin -P admin\n");
    PRINTF(" mqttc connect 0 192.168.1.30 -n -k 60 -w 5\n");
    PRINTF(" mqttc subscribe 0 -t quartz/dev0/status -q 2\n");
    PRINTF(" mqttc publish 0 -t quartz/dev0/status -q 1 -m \"Hello, World!\"\n");
    PRINTF(" mqttc unsubscribe 0 -t quartz/dev0/status\n");
    PRINTF(" mqttc disconnect 0\n");
}

/*****************************************************************************
 *       [0]   [1]            
 * mqttc init [<calist file>]
 * mqttc new
 * mqttc shut
 * mqttc config <id>
 * mqttc sslconfig <id>
 * mqttc connect <id>
 * mqttc subscribe <id>
 * mqttc unsubscribe <id>
 * mqttc publish <id>
 * mqttc disconnect <id>
 *****************************************************************************/
QCLI_Command_Status_t mqttc(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    qapi_Status_t status;
    char *cmd;
    int32_t id, i;
    int32_t e = QAPI_ERROR;
    int32_t handle = 0;

    if (Parameter_Count == 0 || Parameter_List == NULL)
    {
        mqttc_help();
        return QCLI_STATUS_SUCCESS_E;
    }

    cmd = Parameter_List[0].String_Value;

    /*       [0]   [1]
     * mqttc init  calist.bin
     */
    if (strncmp(cmd, "init", 3) == 0)
    {
        char *ca_file = NULL;

        if (Parameter_Count >= 2)
        {
            ca_file = Parameter_List[1].String_Value;
        }

        e = qapi_Net_MQTTc_Init(ca_file);
        if (e == QAPI_ERROR)
        {
            PRINTF("MQTTC init failed\n");
            goto end;
        }
    }

    /******************************************************
     *       [0]  [1]                 [2]
     * mqttc new  [<clientid string>] [<is_clean_session>]
     ******************************************************/
    else if (strncmp(cmd, "new", 3) == 0)
    {
        char *client_id = NULL;
        qbool_t clean_session = true; 

        id = find_session(0);
        if (id == QAPI_ERROR)
        {
            PRINTF("Cannot create more than %d sessions\n", NUM_SESSIONS);
            goto end;
        }

        if (Parameter_Count >= 2)
        {
            client_id = Parameter_List[1].String_Value;
            if (Parameter_Count == 3)
            {
                clean_session = !!Parameter_List[2].Integer_Value;
            }
        }

        e = qapi_Net_MQTTc_New((const char *)client_id, clean_session, &status);
        if (e == QAPI_ERROR)
        {
            PRINTF("Failed to create an session. err %d (%s)\n", status, get_status_string(status));
            goto end;
        }

        mqttc_handle[id] = e;
        PRINTF("Session created. Id %d handle %x\n", id, e);
    }

    /******************************************************
     *       [0]
     * mqttc shut
     ******************************************************/
    else if (strncmp(cmd, "shutdown", 3) == 0)
    {
        qapi_Net_MQTTc_Shutdown();
        memset(mqttc_handle, 0, sizeof(mqttc_handle));
    }

    /******************************************************
     *       [0] [1]  [2]
     * mqttc app host [-s] [-n]
     ******************************************************/
    else if (strncmp(cmd, "application", 3) == 0)
    {
        qurt_signal_t ev_mqttc;
        uint32_t set_signals = 0;
        const char *host;
        const char *client_id = NULL;
        qbool_t secure_session = false;
        qbool_t nonblocking_connect = false;
        char *bind_if = NULL;
        uint16_t keepalive_sec = 0;         /* disable sending keepalive packet */ 
        uint16_t max_conn_pending_sec = 5;  /* default = 5 sec */

        if (Parameter_Count < 2)
        {
            mqttc_help();
            goto end;
        }

        /* Create an instance */
        PRINTF("\nMQTTC NEW ..\n");
        e = qapi_Net_MQTTc_New(client_id, true, &status);
        if (e == QAPI_ERROR)
        {
            PRINTF("Failed to create an session. err %d (%s)\n", status, get_status_string(status));
            goto end;
        }
        handle = e;

        PRINTF("Session created. handle %x\n", handle);

        memset(&ev_mqttc, 0, sizeof(qurt_signal_t));
        qurt_signal_init(&ev_mqttc);

        host = Parameter_List[1].String_Value;

        for (i = 2; i < Parameter_Count; i++)
        {
            if (Parameter_List[i].String_Value[0] == '-')
            {
                switch (Parameter_List[i].String_Value[1])
                {
                    case 'i':   /* -i wlan1 */
                        i++;
                        bind_if = Parameter_List[i].String_Value;
                        break;

                    case 'k':   /* -k 30 */
                        i++;
                        if (!Parameter_List[i].Integer_Is_Valid)
                        {
                            PRINTF("Invalid keepalive: %s\n",
                                    Parameter_List[i].String_Value);
                            goto end;
                        }
                        keepalive_sec = Parameter_List[i].Integer_Value;
                        break;

                    case 'n':   /* -n */
                        nonblocking_connect = true;
                        break;

#ifdef CONFIG_NET_SSL_DEMO
                    case 's':   /* -s */
                        secure_session = true;
                        break;
#endif
                    case 'w':   /* -w 5 */
                        i++;
                        if (!Parameter_List[i].Integer_Is_Valid)
                        {
                            PRINTF("Invalid connack wait time: %s\n",
                                    Parameter_List[i].String_Value);
                            goto end;
                        }
                        max_conn_pending_sec = Parameter_List[i].Integer_Value;
                        break;

                    default:
                        PRINTF("Unknown option: %s\n", Parameter_List[i].String_Value);
                        goto end;
                }
            }
        }   /* for */

        /* Connect */
        qapi_Net_MQTTc_Register_Connect_Callback(handle, mqtt_connect_cbk, (void *)&ev_mqttc);
        qapi_Net_MQTTc_Register_Subscribe_Callback(handle, mqtt_subscribe_cbk, (void *)&ev_mqttc);
        qapi_Net_MQTTc_Set_Keep_Alive(handle, keepalive_sec);
        qapi_Net_MQTTc_Set_Connack_Wait_Time(handle, max_conn_pending_sec); 
        PRINTF("\nMQTTC (%s) CONNECT ..\n", nonblocking_connect ? "nonblocking" : "blocking");
        e = qapi_Net_MQTTc_Connect(handle, host, secure_session, nonblocking_connect, bind_if, &status);
        if (e == QAPI_ERROR)
        {
            PRINTF("MQTTC connect failed. err %d (%s)\n", status, get_status_string(status));
            goto cleanup;
        }

        if (nonblocking_connect)
        {
            /* Waiting .. */
            e = qurt_signal_wait_timed(&ev_mqttc,
                    CONNECT_OK | CONNECT_ERROR,
                    QURT_SIGNAL_ATTR_CLEAR_MASK | QURT_SIGNAL_ATTR_WAIT_ANY,
                    &set_signals,
                    1000);       /* 10 sec */
            if (e != QAPI_OK ||                 /* timed out */
                set_signals & CONNECT_ERROR)    /* CONNECT failed */
            {
                goto cleanup;
            }
        }

        /* Subscribe */
        PRINTF("\nMQTTC SUBSCRIBE ..\n");
        e = qapi_Net_MQTTc_Subscribe_Multiple(handle, topics, topics_len, topics_qos, sizeof(topics)/sizeof(topics[0]), &status);
        if (e == QAPI_ERROR)
        {
            PRINTF("MQTTC subscribe failed. err %d (%s)\n", status, get_status_string(status));
            goto cleanup;
        }

        /* Waiting .. */
        e = qurt_signal_wait_timed(&ev_mqttc,
                                   SUBACK_EVENT,
                                   QURT_SIGNAL_ATTR_CLEAR_MASK | QURT_SIGNAL_ATTR_WAIT_ANY,
                                   &set_signals,
                                   1000);       /* 10 sec */
        if (e != QAPI_OK)
        {
            /* timed out */
            PRINTF("ERROR: SUBACK not received!\n");
            goto cleanup;
        }

        /* Publish 1st message */
        PRINTF("\nMQTTC PUBLISH (1st message) ..\n");
        e = qapi_Net_MQTTc_Publish(handle, pubtopic_0, strlen(pubtopic_0), pubmsg_0, sizeof(pubmsg_0), 2, false, false, &status);
        if (e == QAPI_ERROR)
        {
            PRINTF("MQTTC publish_0 failed. err %d (%s)\n", status, get_status_string(status));
            goto cleanup;
        }

        /* Waiting .. */
        e = qurt_signal_wait_timed(&ev_mqttc,
                                   PUBLISH_EVENT,
                                   QURT_SIGNAL_ATTR_CLEAR_MASK | QURT_SIGNAL_ATTR_WAIT_ANY,
                                   &set_signals,
                                   1000);       /* 10 sec */
        if (e != QAPI_OK)
        {
            /* timed out */
            PRINTF("ERROR: PUBLISH_0 not received!\n");
            goto cleanup;
        }

        /* Publish 2nd message */
        PRINTF("\nMQTTC PUBLISH (2nd message) ..\n");
        e = qapi_Net_MQTTc_Publish(handle, pubtopic_1, strlen(pubtopic_1), pubmsg_1, strlen(pubmsg_1), 1, false, false, &status);
        if (e == QAPI_ERROR)
        {
            PRINTF("MQTTC publish_1 failed. err %d (%s)\n", status, get_status_string(status));
            goto cleanup;
        }

        /* Waiting .. */
        e = qurt_signal_wait_timed(&ev_mqttc,
                                   PUBLISH_EVENT,
                                   QURT_SIGNAL_ATTR_CLEAR_MASK | QURT_SIGNAL_ATTR_WAIT_ANY,
                                   &set_signals,
                                   1000);       /* 10 sec */
        if (e != QAPI_OK)
        {
            /* timed out */
            PRINTF("ERROR: PUBLISH_1 not received!\n");
            goto cleanup;
        }

        /* Unsubscribe */
        PRINTF("\nMQTTC UNSUBSCRIBE ..\n");
        e = qapi_Net_MQTTc_Unsubscribe_Multiple(handle, topics, topics_len, sizeof(topics)/sizeof(topics[0]), &status);
        if (e == QAPI_ERROR)
        {
            PRINTF("MQTTC unsubscribe failed. err %d (%s)\n", status, get_status_string(status));
            goto cleanup;
        }

        /* Disconnect */
        PRINTF("\nMQTTC DISCONNECT ..\n");
        e = qapi_Net_MQTTc_Disconnect(handle, &status);
        if (e == QAPI_ERROR)
        {
            PRINTF("MQTTC disconnect failed. err %d (%s)\n", status, get_status_string(status));
            goto cleanup;
        }

cleanup:
        /* Destroy */
        PRINTF("\nMQTTC DESTROY ..\n");
        e = qapi_Net_MQTTc_Destroy(handle);
        if (e == QAPI_ERROR)
        {
            PRINTF("MQTTC destroy failed. handle %x\n", handle);
        }
        else
        {
            PRINTF("Session deleted. handle %x\n", handle);
        }

        qurt_signal_destroy(&ev_mqttc);
    }

    /* The following commands need <session_id> parameter */
    else
    {
        if (Parameter_Count < 2)
        {
            mqttc_help();
            goto end;
        }

        id = Parameter_List[1].Integer_Value;

        if (!Parameter_List[1].Integer_Is_Valid ||
            id < 0 || id >= NUM_SESSIONS ||
            (handle = mqttc_handle[id]) == 0)
        {
            PRINTF("Invalid session id %d\n", id);
            goto end;
        }

        /********************************************************
         *       [0]    [1]   [2] [3]
         * mqttc config <id>  -u  <username>
         *                    -P  <password>
         *                    -t  <will topic>
         *                    -m  <will message>
         *                    -q  <will QOS>
         *                    -r
         ********************************************************/
        if (strncmp(cmd, "config", 4) == 0 || strcmp(cmd, "cfg") == 0)
        {
            const char *user = NULL, *pw = NULL;
            const char *topic = NULL;
            const char *msg = NULL;
            uint32_t qos = 0;
            qbool_t retained = false;

            if (Parameter_Count < 3)
            {
                PRINTF("What are config parameters?\n");
                mqttc_help();
                goto end;
            }

            for (i = 2; i < Parameter_Count; i++)
            {
                if (Parameter_List[i].String_Value[0] == '-')
                {
                    switch (Parameter_List[i].String_Value[1])
                    {
                        case 'u':   /* -u admin */
                            i++;
                            user = Parameter_List[i].String_Value;
                            break;

                        case 'P':   /* -P admin */
                            i++;
                            pw = Parameter_List[i].String_Value;
                            break;

                        case 't':   /* -t will/topic/ */
                            i++;
                            topic = Parameter_List[i].String_Value;
                            break;

                        case 'm':   /* -m "Will Messages" */
                            i++;
                            msg = Parameter_List[i].String_Value;
                            break;

                        case 'q':   /* -q <will QOS> */
                            i++;
                            if (!Parameter_List[i].Integer_Is_Valid ||
                                Parameter_List[i].Integer_Value < 0 ||
                                Parameter_List[i].Integer_Value > 2)
                            {
                                PRINTF("Invalid QOS: %s\n", Parameter_List[i].String_Value);
                                goto end;
                            }
                            qos = Parameter_List[i].Integer_Value;
                            break;

                        case 'r':   /* -r */
                            retained = true;
                            break;

                        default:
                            PRINTF("Unknown option: %s\n", Parameter_List[i].String_Value);
                            goto end;
                    }
                }
                else
                {
                    PRINTF("Unknown option: %s\n", Parameter_List[i].String_Value);
                    goto end;
                }

                if (i == Parameter_Count)
                {
                    PRINTF("What is value of %s?\n", Parameter_List[i-1].String_Value);
                    goto end;
                }
            }   /* for */

            e = qapi_Net_MQTTc_Set_Username_Password(handle, user, strlen(user), pw, strlen(pw), &status);
            if (e == QAPI_ERROR)
            {
                PRINTF("Failed to set username/password: %s/%s. err %d (%s)\n",
                        user, pw, status, get_status_string(status));
                goto end;
            }

            e = qapi_Net_MQTTc_Set_Will(handle, topic, strlen(topic), msg, strlen(msg), qos, retained, &status); 
            if (e == QAPI_ERROR)
            {
                PRINTF("Failed to set Will. err %d (%s)\n", status, get_status_string(status));
                goto end;
            }
        }
#ifdef CONFIG_NET_SSL_DEMO
        /********************************************************
         *       [0]       [1]  [2]      [3]   
         * mqttc sslconfig <id> protocol TLS1.2
         *                      time     1
         *                      cipher   QAPI_NET_TLS_RSA_WITH_AES_256_GCM_SHA384
         *                      ..
         ********************************************************/
        else if (strncmp(cmd, "sslconfig", 3) == 0 || strcmp(cmd, "sslcfg") == 0)
        {
            QCLI_Command_Status_t sts;
            qapi_Net_SSL_Config_t *cfg;

            if (Parameter_Count < 3)
            {
                PRINTF("What are SSL parameters?\n");
                mqttc_help();
                goto end;
            }

            cfg = mqttc_sslcfg[id];
            if (cfg == NULL)
            {
                cfg = malloc(sizeof(qapi_Net_SSL_Config_t));
                if (cfg == NULL)
                {
                    PRINTF("Allocation failure\n");
                    goto end;
                }
                memset(cfg, 0, sizeof(qapi_Net_SSL_Config_t));
                mqttc_sslcfg[id] = cfg;
            }
            else
            {
                /* free previous ssl parameters */
                ssl_free_config_parameters(cfg);
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
                goto end;
            }

            /* MQTTC always uses TLS. If configured for DTLS, set it to TLS */
            if (cfg->protocol <= QAPI_NET_SSL_DTLS_E ||
                cfg->protocol > QAPI_NET_SSL_PROTOCOL_TLS_1_2)
            {
                cfg->protocol = 0;
            }
        }
#endif
        /********************************************************
         *       [0] [1]  [2]
         * mqttc con <id> 192.168.1.30
         *
         * The following parameters are optional.
         *       -k  <keepalive sec> 
         *       -w  <connack_wait sec> 
         *       -i  <ifname>           //bind interface
         *       -s                     //secure connection
         *       -n                     //non-blocking connect
         ********************************************************/
        else if (strncmp(cmd, "connect", 3) == 0)
        {
            char *host;
            char *bind_if = NULL;
            qbool_t secure_session = false;
            qbool_t nonblocking_connect = false;
            uint16_t keepalive_sec = 0;         /* disable sending keepalive packet */ 
            uint16_t max_conn_pending_sec = 5;  /* default = 5 sec */

            if (Parameter_Count < 3)
            {
                mqttc_help();
                goto end;
            }

            host = Parameter_List[2].String_Value;

            for (i = 3; i < Parameter_Count; i++)
            {
                if (Parameter_List[i].String_Value[0] == '-')
                {
                    switch (Parameter_List[i].String_Value[1])
                    {
                        case 'i':   /* -i wlan1 */
                            i++;
                            bind_if = Parameter_List[i].String_Value;
                            break;

                        case 'k':   /* -k 30 */
                            i++;
                            if (!Parameter_List[i].Integer_Is_Valid)
                            {
                                PRINTF("Invalid keepalive: %s\n", Parameter_List[i].String_Value);
                                goto end;
                            }
                            keepalive_sec = Parameter_List[i].Integer_Value;
                            break;

                        case 'n':   /* -n */
                            nonblocking_connect = true;
                            break;

                        case 's':   /* -s */
                            secure_session = true;
                            break;

                        case 'w':   /* -w 5 */
                            i++;
                            if (!Parameter_List[i].Integer_Is_Valid)
                            {
                                PRINTF("Invalid connack wait time: %s\n", Parameter_List[i].String_Value);
                                goto end;
                            }
                            max_conn_pending_sec = Parameter_List[i].Integer_Value;
                            break;

                        default:
                            PRINTF("Unknown option: %s\n", Parameter_List[i].String_Value);
                            goto end;
                    }
                }
                else
                {
                    PRINTF("Unknown option: %s\n", Parameter_List[i].String_Value);
                    goto end;
                }

                if (i == Parameter_Count)
                {
                    PRINTF("What is value of %s?\n", Parameter_List[i-1].String_Value);
                    goto end;
                }
            }   /* for */

            qapi_Net_MQTTc_Register_Connect_Callback(handle, mqtt_connect_cbk, (void *)NULL);
            qapi_Net_MQTTc_Register_Subscribe_Callback(handle, mqtt_subscribe_cbk, (void *)NULL);
            qapi_Net_MQTTc_Set_Keep_Alive(handle, keepalive_sec);
            qapi_Net_MQTTc_Set_Connack_Wait_Time(handle, max_conn_pending_sec); 
#ifdef CONFIG_NET_SSL_DEMO
            if (secure_session && mqttc_sslcfg[id] != NULL)
            {
                e = qapi_Net_MQTTc_Set_SSL_Config(handle, mqttc_sslcfg[id], &status);
                if (e == QAPI_ERROR)
                {
                    PRINTF("SSL config failed. err %d (%s)\n", status, get_status_string(status));
                    goto end;
                }
            }
#endif
            e = qapi_Net_MQTTc_Connect(handle, host, secure_session, nonblocking_connect, bind_if, &status);
            if (e != QAPI_OK)
            {
                PRINTF("MQTTC connect failed. err %d (%s)\n", status, get_status_string(status));
                goto end;
            }
            else if (!nonblocking_connect)
            {
                PRINTF("status %d (%s)\n", status, get_status_string(status));
            }
        }

        /********************************************************
         *       [0] [1]  [2] [3]
         * mqttc sub <id> -t  <topic>  [-q  <QOS>]
         ********************************************************/
        else if (strncmp(cmd, "subscribe", 3) == 0)
        {
            const char *topic = NULL;
            uint32_t qos = 2;

            if (Parameter_Count < 4)
            {
                mqttc_help();
                goto end;
            }

            for (i = 2; i < Parameter_Count; i++)
            {
                if (i == Parameter_Count-1)
                {
                    PRINTF("What is value of %s?\n", Parameter_List[i].String_Value);
                    goto end;
                }

                if (Parameter_List[i].String_Value[0] == '-')
                {
                    switch (Parameter_List[i].String_Value[1])
                    {
                        case 't':   /* -t quartz/dev0/status */
                            i++;
                            topic = Parameter_List[i].String_Value;
                            break;

                        case 'q':   /* -q 2 */
                            i++;
                            if (!Parameter_List[i].Integer_Is_Valid ||
                                Parameter_List[i].Integer_Value < 0 ||
                                Parameter_List[i].Integer_Value > 2)
                            {
                                PRINTF("Invalid QOS: %s\n", Parameter_List[i].String_Value);
                                goto end;
                            }
                            qos = Parameter_List[i].Integer_Value;
                            break;

                        default:
                            PRINTF("Unknown option: %s\n", Parameter_List[i].String_Value);
                            goto end;
                    }
                }
                else
                {
                    PRINTF("Unknown option: %s\n", Parameter_List[i].String_Value);
                    goto end;
                }

                if (i == Parameter_Count)
                {
                    PRINTF("What is value of %s?\n", Parameter_List[i-1].String_Value);
                    goto end;
                }
            }   /* for */
            e = qapi_Net_MQTTc_Subscribe(handle, topic, qos, &status);
            if (e == QAPI_ERROR)
            {
                PRINTF("MQTTC subscribe failed. err %d (%s)\n", status, get_status_string(status));
                goto end;
            }
        }

        /********************************************************
         *       [0] [1]  [2] [3] 
         * mqttc pub <id> -t  <topic> [-m <msg>] [-q <QOS>] [-r] [-d]
         ********************************************************/
        else if (strncmp(cmd, "publish", 3) == 0)
        {
            const char *topic = NULL;
            const char *msg = NULL;
            uint32_t msg_len = 0;
            uint32_t qos = 2;
            qbool_t retained = false;
            qbool_t dup = false;

            if (Parameter_Count < 4)
            {
                mqttc_help();
                goto end;
            }

            for (i = 2; i < Parameter_Count; i++)
            {
                if (Parameter_List[i].String_Value[0] == '-')
                {
                    switch (Parameter_List[i].String_Value[1])
                    {
                        case 't':   /* -t quartz/dev0/status */
                            i++;
                            topic = Parameter_List[i].String_Value;
                            break;

                        case 'm':   /* -m "Hello World!" */
                            i++;
                            msg = Parameter_List[i].String_Value;
                            msg_len = strlen(msg);
                            break;

                        case 'q':   /* -q 2 */
                            i++;
                            if (!Parameter_List[i].Integer_Is_Valid ||
                                Parameter_List[i].Integer_Value < 0 ||
                                Parameter_List[i].Integer_Value > 2)
                            {
                                PRINTF("Invalid QOS: %s\n", Parameter_List[i].String_Value);
                                goto end;
                            }
                            qos = Parameter_List[i].Integer_Value;
                            break;

                        case 'r':   /* -r */
                            retained = true;
                            break;

                        case 'd':   /* -d */
                            dup = true;
                            break;

                        default:
                            PRINTF("Unknown option: %s\n", Parameter_List[i].String_Value);
                            goto end;
                    }
                }
                else
                {
                    PRINTF("Unknown option: %s\n", Parameter_List[i].String_Value);
                    goto end;
                }

                if (i == Parameter_Count)
                {
                    PRINTF("What is value of %s?\n", Parameter_List[i-1].String_Value);
                    goto end;
                }
            }   /* for */

            if (strchr(topic, '#') != NULL ||
                strchr(topic, '+') != NULL)
            {
                PRINTF("ERROR: Topic name cannot contain wildcard characters.\n");
                goto end;
            }

            e = qapi_Net_MQTTc_Publish(handle, topic, strlen(topic), msg, msg_len, qos, retained, dup, &status);
            if (e == QAPI_ERROR)
            {
                PRINTF("MQTTc publish failed. err %d (%s)\n", status, get_status_string(status));
                goto end;
            }
        }

        /********************************************************
         *       [0] [1]  [2] [3]
         * mqttc uns <id> -t <topic>
         ********************************************************/
        else if (strncmp(cmd, "unsubscribe", 3) == 0)
        {
            const char *topic = NULL;

            if (Parameter_Count < 4)
            {
                mqttc_help();
                goto end;
            }

            for (i = 2; i < Parameter_Count; i++)
            {
                if (Parameter_List[i].String_Value[0] == '-')
                {
                    switch (Parameter_List[i].String_Value[1])
                    {
                        case 't':   /* -t quartz/dev0/status */
                            i++;
                            topic = Parameter_List[i].String_Value;
                            break;

                        default:
                            PRINTF("Unknown option: %s\n", Parameter_List[i].String_Value);
                            goto end;
                    }
                }
                else
                {
                    PRINTF("Unknown option: %s\n", Parameter_List[i].String_Value);
                    goto end;
                }

                if (i == Parameter_Count)
                {
                    PRINTF("What is value of %s?\n", Parameter_List[i-1].String_Value);
                    goto end;
                }
            }   /* for */
            e = qapi_Net_MQTTc_Unsubscribe(handle, topic, &status);
            if (e == QAPI_ERROR)
            {
                PRINTF("MQTTC unsubscribe failed. err %d (%s)\n", status, get_status_string(status));
                goto end;
            }
        }

        /********************************************************
         *       [0] [1]
         * mqttc dis <id>
         ********************************************************/
        else if (strncmp(cmd, "disconnect", 3) == 0)
        {
            e = qapi_Net_MQTTc_Disconnect(handle, &status);
            if (e == QAPI_ERROR)
            {
                PRINTF("MQTTC disconnect failed. err %d (%s)\n", status, get_status_string(status));
                goto end;
            }
        }

        /********************************************************
         *       [0] [1]
         * mqttc des <id>
         ********************************************************/
        else if (strncmp(cmd, "destroy", 3) == 0)
        {
            e = qapi_Net_MQTTc_Destroy(handle);
            if (e == QAPI_ERROR)
            {
                PRINTF("MQTTC destroy failed. err %d (%s)\n", status, get_status_string(status));
                goto end;
            }
            mqttc_handle[id] = 0;
#ifdef CONFIG_NET_SSL_DEMO
            if (mqttc_sslcfg[id])
            {
                /* sslcfg block may have other memory allocation.
                 * Need to free them first.
                 */
                ssl_free_config_parameters(mqttc_sslcfg[id]);
                free(mqttc_sslcfg[id]);
                mqttc_sslcfg[id] = NULL;
            }
#endif
            PRINTF("Session %d is deleted. handle %x\n", id, handle);
        }

        else
        {
            PRINTF("Not supported cmd: \"%s\"\n", cmd);
        }
    }

end:
    if (e)
    {
        return QCLI_STATUS_ERROR_E;
    }
    return QCLI_STATUS_SUCCESS_E;
}
#endif
