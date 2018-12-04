/*
* Copyright (c) 2016 Qualcomm Technologies, Inc.
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

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "qapi_status.h"
#include "bench.h"
#include "qapi_crypto.h"

#ifdef CONFIG_NET_SSL_DEMO

#define MAX_PASSWORD_SIZE_IN_BYTES 256

extern QCLI_Group_Handle_t qcli_net_handle; /* Handle for Net Command Group. */

/* TODO Create default certificate/CA list to be used for testing */
const uint16_t ssl_default_calist_len;

QCLI_Command_Status_t ssl_set_dtls_server_max_clients(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t ssl_set_dtls_server_idle_timer(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t ssl_set_ecjpake_params(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);

#define SSL_PRINTF(...) QCLI_Printf(qcli_net_handle, __VA_ARGS__)
QCLI_Command_Status_t ssl_command_handler(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if(Parameter_Count < 1)
    {
        return QCLI_STATUS_USAGE_E;
    }

    if (0 == strncmp(Parameter_List[0].String_Value, "start", 3)) {
        return ssl_start(Parameter_Count - 1, &Parameter_List[1]);
    }
    else if (0 == strncmp(Parameter_List[0].String_Value, "stop", 3)) {
        return ssl_stop(Parameter_Count - 1, &Parameter_List[1]);
    }
    else if (0 == strncmp(Parameter_List[0].String_Value, "config", 3)) {
        return ssl_config(Parameter_Count - 1, &Parameter_List[1]);
    }
    else if (0 == strncmp(Parameter_List[0].String_Value, "cert", 3)) {
        return ssl_add_cert(Parameter_Count - 1, &Parameter_List[1]);
    }
    else if (0 == strncmp(Parameter_List[0].String_Value, "psk", 3)) {
        return ssl_add_psk_table(Parameter_Count - 1, &Parameter_List[1]);
    }
    else if (0 == strncmp(Parameter_List[0].String_Value, "ecjpake", 7)) {
        return ssl_set_ecjpake_params(Parameter_Count - 1, &Parameter_List[1]);
    }
    else if (0 == strncmp(Parameter_List[0].String_Value, "max_clients", 10)) {
        return ssl_set_dtls_server_max_clients(Parameter_Count - 1, &Parameter_List[1]);
    }
    else if (0 == strncmp(Parameter_List[0].String_Value, "idle_timer", 10)) {
        return ssl_set_dtls_server_idle_timer(Parameter_Count - 1, &Parameter_List[1]);
    }
	else
    {
        return QCLI_STATUS_USAGE_E;
    }
}

QCLI_Command_Status_t ssl_start(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    SSL_INST *ssl;
    qapi_Net_SSL_Role_t role;

    if(Parameter_Count < 1)
    {
usage:
        SSL_PRINTF("\nUsage: ssl start <server|client>\n");
        return QCLI_STATUS_ERROR_E;
    }

    if (0 == strncmp(Parameter_List[0].String_Value, "server", 3))
    {
        ssl = &ssl_inst[SSL_SERVER_INST];
        role = QAPI_NET_SSL_SERVER_E;
    }
    else if (0 == strncmp(Parameter_List[0].String_Value, "client", 3))
    {
        ssl = &ssl_inst[SSL_CLIENT_INST];
        role = QAPI_NET_SSL_CLIENT_E;
    }
    else
    {
       goto usage;
    }

    if (ssl->sslCtx != QAPI_NET_SSL_INVALID_HANDLE)
    {
        SSL_PRINTF("ERROR: one SSL context only!\n");
        return QCLI_STATUS_ERROR_E;
    }

    // Create SSL context
    memset(ssl, 0, sizeof(SSL_INST));
    ssl->role = role;
    ssl->sslCtx = qapi_Net_SSL_Obj_New(role);

    if (ssl->sslCtx == QAPI_NET_SSL_INVALID_HANDLE)
    {
        SSL_PRINTF("ERROR: Unable to create SSL context\n");
        return QCLI_STATUS_ERROR_E;
    }

    // Reset config struct
    memset(&ssl->config, 0, sizeof(qapi_Net_SSL_Config_t));

    // Done
    SSL_PRINTF("SSL %s started\n", (role == QAPI_NET_SSL_CLIENT_E) ? "client" : "server");
    return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t ssl_stop(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    SSL_INST *ssl;
    qapi_Net_SSL_Role_t role;

    if(Parameter_Count < 1)
    {
usage:
        SSL_PRINTF("\nUsage: ssl stop <server|client>\n");
        return QCLI_STATUS_ERROR_E;
    }

    if (0 == strncmp(Parameter_List[0].String_Value, "server", 3))
    {
        ssl = &ssl_inst[SSL_SERVER_INST];
        role = QAPI_NET_SSL_SERVER_E;
    }
    else if (0 == strncmp(Parameter_List[0].String_Value, "client", 3))
    {
        ssl = &ssl_inst[SSL_CLIENT_INST];
        role = QAPI_NET_SSL_CLIENT_E;
    }
    else
    {
        goto usage;
    }

    if (ssl->sslCtx == QAPI_NET_SSL_INVALID_HANDLE || role != ssl->role)
    {
        SSL_PRINTF("ERROR: SSL %s not started\n", (role == QAPI_NET_SSL_CLIENT_E) ? "client" : "server");
        return QCLI_STATUS_ERROR_E;
    }

    if (ssl->ssl)
    {
        qapi_Net_SSL_Shutdown(ssl->ssl);
        ssl->ssl = QAPI_NET_SSL_INVALID_HANDLE;
    }

    if (ssl->sslCtx)
    {
        qapi_Net_SSL_Obj_Free(ssl->sslCtx);
        ssl->sslCtx = QAPI_NET_SSL_INVALID_HANDLE;
    }

    ssl_free_config_parameters(&ssl->config);

    SSL_PRINTF("SSL %s stopped\n", (role == QAPI_NET_SSL_CLIENT_E) ? "client" : "server");
    return QCLI_STATUS_SUCCESS_E;
}

void sslconfig_help(const char *str)
{
    SSL_PRINTF("%s [<keyword_1> <value_1>] [<keyword_2> <value_2>] ...\n", str);
    SSL_PRINTF(" where <keyword> <value> are:\n");
    SSL_PRINTF("       protocol <protocol> = select protocol: TLS (default), TLS1.0, TLS1.1, TLS1.2, DTLS, DTLS1.0, DTLS1.2\n");
    SSL_PRINTF("       time 0|1            = disable|enable certificate time validation (optional)\n");
    SSL_PRINTF("       domain 0|<name>     = disable|enable validation of peer's domain name against <name>\n");
    SSL_PRINTF("       alert 0|1           = disable|enable sending of SSL alert if certificate validation fails.\n");
    SSL_PRINTF("       cipher <cipher>     = select <cipher> (enum name) suite to use, can be repeated 8 times (optional)\n");
    SSL_PRINTF("       max_frag_len <bytes>= max fragment length in bytes\n");
    SSL_PRINTF("       neg_disable 0|1     = disable|enable maximum fragment length negotiation\n");
    SSL_PRINTF("       sni <name>          = configure name for server name indication (SNI)\n");
    SSL_PRINTF("       alpn <protocol_name>= specify protocol name for ALPN, e.g. \"h2\", \"http/1.1\"\n");
}

void ssl_free_config_parameters(qapi_Net_SSL_Config_t *cfg)
{
    if (cfg == NULL)
    {
        return;
    }

	if (cfg->sni_Name != NULL)
    {
		free(cfg->sni_Name);
        cfg->sni_Name = NULL;
        cfg->sni_Name_Size = 0;
	}

    /* ignore 'alpn' and 'extensions' for now */
}

/*****************************************************************************
 * [0]      [1]     [2]   [3] [4]    [5]
 * protocol TLS1.1  time   1  cipher QAPI_NET_TLS_RSA_WITH_AES_256_GCM_SHA384
 *****************************************************************************/
QCLI_Command_Status_t ssl_parse_config_parameters(
        uint32_t Parameter_Count,
        QCLI_Parameter_t *Parameter_List,
        qapi_Net_SSL_Config_t *cfg,
        qapi_Net_SSL_Obj_Hdl_t ssl_hdl,
        qbool_t server)
{
    uint32_t argn;

    memset(cfg, 0, sizeof(*cfg));

    /* Set the default protocol to TLS */
    cfg->protocol = QAPI_NET_SSL_TLS_E;

    for (argn = 0; argn < Parameter_Count; ++argn)
    {
        if (argn == Parameter_Count-1)
        {
            SSL_PRINTF("What is value of %s?\n", Parameter_List[argn].String_Value);
            return QCLI_STATUS_ERROR_E;
        }

        /* 'protocol TLS1.2' */
        if (0 == strncmp("protocol", Parameter_List[argn].String_Value, 8))
        {
            argn++;

            // Setting of protocol version is supported for SSL client only,
            // but SSL server uses this to set the protocol type (DTLS/TLS).
            if (server)
            {
                SSL_PRINTF("Warning: Server uses only protocol type (DTLS/TLS) and ignores version (1.1, 1.2, ...).\n");
            }

            if (0 == strcmp("TLS1.0", Parameter_List[argn].String_Value))
            {
                cfg->protocol = QAPI_NET_SSL_PROTOCOL_TLS_1_0;
            }
            else if (0 == strcmp("TLS1.1", Parameter_List[argn].String_Value))
            {
                cfg->protocol = QAPI_NET_SSL_PROTOCOL_TLS_1_1;
            }
            else if (0 == strcmp("TLS1.2", Parameter_List[argn].String_Value))
            {
                cfg->protocol = QAPI_NET_SSL_PROTOCOL_TLS_1_2;
            }
            else if (0 == strcmp("TLS", Parameter_List[argn].String_Value))
            {
                cfg->protocol = QAPI_NET_SSL_TLS_E;
            }
            else if (0 == strcmp("DTLS1.0", Parameter_List[argn].String_Value))
            {
                cfg->protocol = QAPI_NET_SSL_PROTOCOL_DTLS_1_0;
            }
            else if (0 == strcmp("DTLS1.2", Parameter_List[argn].String_Value))
            {
                cfg->protocol = QAPI_NET_SSL_PROTOCOL_DTLS_1_2;
            }
            else if (0 == strcmp("DTLS", Parameter_List[argn].String_Value))
            {
                cfg->protocol = QAPI_NET_SSL_DTLS_E;
            }
            else
            {
                SSL_PRINTF("ERROR: Invalid protocol: %s\n", Parameter_List[argn].String_Value);
                return QCLI_STATUS_ERROR_E;
            }
        }

        /* 'time 1' */
        else if (0 == strncmp("time", Parameter_List[argn].String_Value, 4))
        {
            argn++;

            if (0 == strcmp("1", Parameter_List[argn].String_Value))
            {
                cfg->verify.time_Validity = 1;
            }
            else if (0 == strcmp("0", Parameter_List[argn].String_Value))
            {
                cfg->verify.time_Validity = 0;
            }
            else
            {
                SSL_PRINTF("ERROR: Invalid option: %s\n", Parameter_List[argn].String_Value);
                return QCLI_STATUS_ERROR_E;
            }
        }

        /* 'alert 0 ' */
        else if (0 == strncmp("alert", Parameter_List[argn].String_Value, 5))
        {
            argn++;

            if (0 == strcmp("1", Parameter_List[argn].String_Value))
            {
                cfg->verify.send_Alert = 1;
            }
            else if (0 == strcmp("0", Parameter_List[argn].String_Value))
            {
                cfg->verify.send_Alert = 0;
            }
            else
            {
                SSL_PRINTF("ERROR: Invalid option: %s\n", Parameter_List[argn].String_Value);
                return QCLI_STATUS_ERROR_E;
            }
        }

        /* 'domain 0' */
        else if (0 == strncmp("domain", Parameter_List[argn].String_Value, 6))
        {
            argn++;

            if (0 == strcmp("0", Parameter_List[argn].String_Value))
            {
                cfg->verify.domain = 0;
                cfg->verify.match_Name[0] = '\0';
            }
            else
            {
                cfg->verify.domain = 1;
                if (strlen(Parameter_List[argn].String_Value) >= sizeof(cfg->verify.match_Name))
                {
                    SSL_PRINTF("ERROR: %s too long (max %d chars)\n", Parameter_List[argn].String_Value, sizeof(cfg->verify.match_Name));
                    return QCLI_STATUS_ERROR_E;
                }
                strcpy(cfg->verify.match_Name, Parameter_List[argn].String_Value);
            }
        }

        /* 'cipher QAPI_NET_TLS_RSA_WITH_AES_256_GCM_SHA384' */
        else if (0 == strncmp("cipher", Parameter_List[argn].String_Value, 6))
        {
            qbool_t is_valid_cipher = true;
            qapi_Status_t cipher_selection_status = QAPI_OK;

            argn++;

            if (0 == strcmp("QAPI_NET_TLS_RSA_WITH_AES_256_GCM_SHA384", Parameter_List[argn].String_Value))
            {
                cipher_selection_status = qapi_Net_SSL_Cipher_Add(cfg, QAPI_NET_TLS_RSA_WITH_AES_256_GCM_SHA384);
            }
            else if (0 == strcmp("QAPI_NET_TLS_RSA_WITH_AES_256_CBC_SHA256", Parameter_List[argn].String_Value))
            {
                cipher_selection_status = qapi_Net_SSL_Cipher_Add(cfg, QAPI_NET_TLS_RSA_WITH_AES_256_CBC_SHA256);
            }
            else if (0 == strcmp("QAPI_NET_TLS_DHE_RSA_WITH_AES_128_CBC_SHA256", Parameter_List[argn].String_Value))
            {
                cipher_selection_status = qapi_Net_SSL_Cipher_Add(cfg, QAPI_NET_TLS_DHE_RSA_WITH_AES_128_CBC_SHA256);
            }
            else if (0 == strcmp("QAPI_NET_TLS_RSA_WITH_AES_256_CBC_SHA", Parameter_List[argn].String_Value))
            {
                cipher_selection_status = qapi_Net_SSL_Cipher_Add(cfg, QAPI_NET_TLS_RSA_WITH_AES_256_CBC_SHA);
            }
            else if (0 == strcmp("QAPI_NET_TLS_DHE_RSA_WITH_AES_256_CBC_SHA", Parameter_List[argn].String_Value))
            {
                cipher_selection_status = qapi_Net_SSL_Cipher_Add(cfg, QAPI_NET_TLS_DHE_RSA_WITH_AES_256_CBC_SHA);
            }
            else if (0 == strcmp("QAPI_NET_TLS_DHE_RSA_WITH_AES_256_CBC_SHA256", Parameter_List[argn].String_Value))
            {
                cipher_selection_status = qapi_Net_SSL_Cipher_Add(cfg, QAPI_NET_TLS_DHE_RSA_WITH_AES_256_CBC_SHA256);
            }
            else if (0 == strcmp("QAPI_NET_TLS_RSA_WITH_AES_128_GCM_SHA256", Parameter_List[argn].String_Value))
            {
                cipher_selection_status = qapi_Net_SSL_Cipher_Add(cfg, QAPI_NET_TLS_RSA_WITH_AES_128_GCM_SHA256);
            }
            else if (0 == strcmp("QAPI_NET_TLS_RSA_WITH_AES_128_CBC_SHA256", Parameter_List[argn].String_Value))
            {
                cipher_selection_status = qapi_Net_SSL_Cipher_Add(cfg, QAPI_NET_TLS_RSA_WITH_AES_128_CBC_SHA256);
            }
            else if (0 == strcmp("QAPI_NET_TLS_RSA_WITH_AES_128_CBC_SHA", Parameter_List[argn].String_Value))
            {
                cipher_selection_status = qapi_Net_SSL_Cipher_Add(cfg, QAPI_NET_TLS_RSA_WITH_AES_128_CBC_SHA);
            }
            else if (0 == strcmp("QAPI_NET_TLS_DHE_RSA_WITH_AES_128_CBC_SHA", Parameter_List[argn].String_Value))
            {
                cipher_selection_status = qapi_Net_SSL_Cipher_Add(cfg, QAPI_NET_TLS_DHE_RSA_WITH_AES_128_CBC_SHA);
            }
            else if (0 == strcmp("QAPI_NET_TLS_DHE_RSA_WITH_CHACHA20_POLY1305_SHA256", Parameter_List[argn].String_Value))
            {
                cipher_selection_status = qapi_Net_SSL_Cipher_Add(cfg, QAPI_NET_TLS_DHE_RSA_WITH_CHACHA20_POLY1305_SHA256);
            }
            else if (0 == strcmp("QAPI_NET_TLS_ECDHE_ECDSA_WITH_CHACHA20_POLY1305_SHA256", Parameter_List[argn].String_Value))
            {
                cipher_selection_status = qapi_Net_SSL_Cipher_Add(cfg, QAPI_NET_TLS_ECDHE_ECDSA_WITH_CHACHA20_POLY1305_SHA256);
            }
            else if (0 == strcmp("QAPI_NET_TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256", Parameter_List[argn].String_Value))
            {
                cipher_selection_status = qapi_Net_SSL_Cipher_Add(cfg, QAPI_NET_TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256);
            }
            else if (0 == strcmp("QAPI_NET_TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA", Parameter_List[argn].String_Value))
            {
                cipher_selection_status = qapi_Net_SSL_Cipher_Add(cfg, QAPI_NET_TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA);
            }
            else if (0 == strcmp("QAPI_NET_TLS_DHE_RSA_WITH_AES_128_GCM_SHA256", Parameter_List[argn].String_Value))
            {
                cipher_selection_status = qapi_Net_SSL_Cipher_Add(cfg, QAPI_NET_TLS_DHE_RSA_WITH_AES_128_GCM_SHA256);
            }
            else if (0 == strcmp("QAPI_NET_TLS_DHE_RSA_WITH_AES_256_GCM_SHA384", Parameter_List[argn].String_Value))
            {
                cipher_selection_status = qapi_Net_SSL_Cipher_Add(cfg, QAPI_NET_TLS_DHE_RSA_WITH_AES_256_GCM_SHA384);
            }
            else if (0 == strcmp("QAPI_NET_TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA", Parameter_List[argn].String_Value))
            {
                cipher_selection_status = qapi_Net_SSL_Cipher_Add(cfg, QAPI_NET_TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA);
            }
            else if (0 == strcmp("QAPI_NET_TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA", Parameter_List[argn].String_Value))
            {
                cipher_selection_status = qapi_Net_SSL_Cipher_Add(cfg, QAPI_NET_TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA);
            }
            else if (0 == strcmp("QAPI_NET_TLS_ECDH_RSA_WITH_AES_128_CBC_SHA", Parameter_List[argn].String_Value))
            {
                cipher_selection_status = qapi_Net_SSL_Cipher_Add(cfg, QAPI_NET_TLS_ECDH_RSA_WITH_AES_128_CBC_SHA);
            }
            else if (0 == strcmp("QAPI_NET_TLS_ECDH_RSA_WITH_AES_256_CBC_SHA", Parameter_List[argn].String_Value))
            {
                cipher_selection_status = qapi_Net_SSL_Cipher_Add(cfg, QAPI_NET_TLS_ECDH_RSA_WITH_AES_256_CBC_SHA);
            }
            else if (0 == strcmp("QAPI_NET_TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA", Parameter_List[argn].String_Value))
            {
                cipher_selection_status = qapi_Net_SSL_Cipher_Add(cfg, QAPI_NET_TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA);
            }
            else if (0 == strcmp("QAPI_NET_TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA", Parameter_List[argn].String_Value))
            {
                cipher_selection_status = qapi_Net_SSL_Cipher_Add(cfg, QAPI_NET_TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA);
            }
            else if (0 == strcmp("QAPI_NET_TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256", Parameter_List[argn].String_Value))
            {
                cipher_selection_status = qapi_Net_SSL_Cipher_Add(cfg, QAPI_NET_TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256);
            }
            else if (0 == strcmp("QAPI_NET_TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA384", Parameter_List[argn].String_Value))
            {
                cipher_selection_status = qapi_Net_SSL_Cipher_Add(cfg, QAPI_NET_TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA384);
            }
            else if (0 == strcmp("QAPI_NET_TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA256", Parameter_List[argn].String_Value))
            {
                cipher_selection_status = qapi_Net_SSL_Cipher_Add(cfg, QAPI_NET_TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA256);
            }
            else if (0 == strcmp("QAPI_NET_TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA384", Parameter_List[argn].String_Value))
            {
                cipher_selection_status = qapi_Net_SSL_Cipher_Add(cfg, QAPI_NET_TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA384);
            }
            else if (0 == strcmp("QAPI_NET_TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256", Parameter_List[argn].String_Value))
            {
                cipher_selection_status = qapi_Net_SSL_Cipher_Add(cfg, QAPI_NET_TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256);
            }
            else if (0 == strcmp("QAPI_NET_TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA384", Parameter_List[argn].String_Value))
            {
                cipher_selection_status = qapi_Net_SSL_Cipher_Add(cfg, QAPI_NET_TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA384);
            }
            else if (0 == strcmp("QAPI_NET_TLS_ECDH_RSA_WITH_AES_128_CBC_SHA256", Parameter_List[argn].String_Value))
            {
                cipher_selection_status = qapi_Net_SSL_Cipher_Add(cfg, QAPI_NET_TLS_ECDH_RSA_WITH_AES_128_CBC_SHA256);
            }
            else if (0 == strcmp("QAPI_NET_TLS_ECDH_RSA_WITH_AES_256_CBC_SHA384", Parameter_List[argn].String_Value))
            {
                cipher_selection_status = qapi_Net_SSL_Cipher_Add(cfg, QAPI_NET_TLS_ECDH_RSA_WITH_AES_256_CBC_SHA384);
            }
            else if (0 == strcmp("QAPI_NET_TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384", Parameter_List[argn].String_Value))
            {
                cipher_selection_status = qapi_Net_SSL_Cipher_Add(cfg, QAPI_NET_TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384);
            }
            else if (0 == strcmp("QAPI_NET_TLS_ECDH_ECDSA_WITH_AES_128_GCM_SHA256", Parameter_List[argn].String_Value))
            {
                cipher_selection_status = qapi_Net_SSL_Cipher_Add(cfg, QAPI_NET_TLS_ECDH_ECDSA_WITH_AES_128_GCM_SHA256);
            }
            else if (0 == strcmp("QAPI_NET_TLS_ECDH_ECDSA_WITH_AES_256_GCM_SHA384", Parameter_List[argn].String_Value))
            {
                cipher_selection_status = qapi_Net_SSL_Cipher_Add(cfg, QAPI_NET_TLS_ECDH_ECDSA_WITH_AES_256_GCM_SHA384);
            }
            else if (0 == strcmp("QAPI_NET_TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256", Parameter_List[argn].String_Value))
            {
                cipher_selection_status = qapi_Net_SSL_Cipher_Add(cfg, QAPI_NET_TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256);
            }
            else if (0 == strcmp("QAPI_NET_TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384", Parameter_List[argn].String_Value))
            {
                cipher_selection_status = qapi_Net_SSL_Cipher_Add(cfg, QAPI_NET_TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384);
            }
            else if (0 == strcmp("QAPI_NET_TLS_ECDH_RSA_WITH_AES_128_GCM_SHA256", Parameter_List[argn].String_Value))
            {
                cipher_selection_status = qapi_Net_SSL_Cipher_Add(cfg, QAPI_NET_TLS_ECDH_RSA_WITH_AES_128_GCM_SHA256);
            }
            else if (0 == strcmp("QAPI_NET_TLS_ECDH_RSA_WITH_AES_256_GCM_SHA384", Parameter_List[argn].String_Value))
            {
                cipher_selection_status = qapi_Net_SSL_Cipher_Add(cfg, QAPI_NET_TLS_ECDH_RSA_WITH_AES_256_GCM_SHA384);
            }
            else if (0 == strcmp("QAPI_NET_TLS_RSA_WITH_AES_128_CCM", Parameter_List[argn].String_Value))
            {
                cipher_selection_status = qapi_Net_SSL_Cipher_Add(cfg, QAPI_NET_TLS_RSA_WITH_AES_128_CCM);
            }
            else if (0 == strcmp("QAPI_NET_TLS_RSA_WITH_AES_256_CCM", Parameter_List[argn].String_Value))
            {
                cipher_selection_status = qapi_Net_SSL_Cipher_Add(cfg, QAPI_NET_TLS_RSA_WITH_AES_256_CCM);
            }
            else if (0 == strcmp("QAPI_NET_TLS_DHE_RSA_WITH_AES_128_CCM", Parameter_List[argn].String_Value))
            {
                cipher_selection_status = qapi_Net_SSL_Cipher_Add(cfg, QAPI_NET_TLS_DHE_RSA_WITH_AES_128_CCM);
            }
            else if (0 == strcmp("QAPI_NET_TLS_DHE_RSA_WITH_AES_256_CCM", Parameter_List[argn].String_Value))
            {
                cipher_selection_status = qapi_Net_SSL_Cipher_Add(cfg, QAPI_NET_TLS_DHE_RSA_WITH_AES_256_CCM);
            }
            else if (0 == strcmp("QAPI_NET_TLS_RSA_WITH_AES_128_CCM_8", Parameter_List[argn].String_Value))
            {
                cipher_selection_status = qapi_Net_SSL_Cipher_Add(cfg, QAPI_NET_TLS_RSA_WITH_AES_128_CCM_8);
            }
            else if (0 == strcmp("QAPI_NET_TLS_RSA_WITH_AES_256_CCM_8", Parameter_List[argn].String_Value))
            {
                cipher_selection_status = qapi_Net_SSL_Cipher_Add(cfg, QAPI_NET_TLS_RSA_WITH_AES_256_CCM_8);
            }
            else if (0 == strcmp("QAPI_NET_TLS_DHE_RSA_WITH_AES_128_CCM_8", Parameter_List[argn].String_Value))
            {
                cipher_selection_status = qapi_Net_SSL_Cipher_Add(cfg, QAPI_NET_TLS_DHE_RSA_WITH_AES_128_CCM_8);
            }
            else if (0 == strcmp("QAPI_NET_TLS_DHE_RSA_WITH_AES_256_CCM_8", Parameter_List[argn].String_Value))
            {
                cipher_selection_status = qapi_Net_SSL_Cipher_Add(cfg, QAPI_NET_TLS_DHE_RSA_WITH_AES_256_CCM_8);
            }
            else if (0 == strcmp("QAPI_NET_TLS_ECDHE_RSA_WITH_CHACHA20_POLY1305_SHA256", Parameter_List[argn].String_Value))
            {
                cipher_selection_status = qapi_Net_SSL_Cipher_Add(cfg, QAPI_NET_TLS_ECDHE_RSA_WITH_CHACHA20_POLY1305_SHA256);
            }
            else if (0 == strcmp("QAPI_NET_TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA", Parameter_List[argn].String_Value))
            {
                cipher_selection_status = qapi_Net_SSL_Cipher_Add(cfg, QAPI_NET_TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA);
            }
            else if (0 == strcmp("QAPI_NET_TLS_ECJPAKE_WITH_AES_128_CCM_8", Parameter_List[argn].String_Value))
            {
                cipher_selection_status = qapi_Net_SSL_Cipher_Add(cfg, QAPI_NET_TLS_ECJPAKE_WITH_AES_128_CCM_8);
            }
            else if (0 == strcmp("QAPI_NET_TLS_PSK_WITH_AES_128_GCM_SHA256", Parameter_List[argn].String_Value))
            {
                cipher_selection_status = qapi_Net_SSL_Cipher_Add(cfg, QAPI_NET_TLS_PSK_WITH_AES_128_GCM_SHA256);
            }
            else if (0 == strcmp("QAPI_NET_TLS_PSK_WITH_AES_256_GCM_SHA384", Parameter_List[argn].String_Value))
            {
                cipher_selection_status = qapi_Net_SSL_Cipher_Add(cfg, QAPI_NET_TLS_PSK_WITH_AES_256_GCM_SHA384);
            }
            else if (0 == strcmp("QAPI_NET_TLS_PSK_WITH_AES_128_CBC_SHA256", Parameter_List[argn].String_Value))
            {
                cipher_selection_status = qapi_Net_SSL_Cipher_Add(cfg, QAPI_NET_TLS_PSK_WITH_AES_128_CBC_SHA256);
            }
            else if (0 == strcmp("QAPI_NET_TLS_PSK_WITH_AES_256_CBC_SHA384", Parameter_List[argn].String_Value))
            {
                cipher_selection_status = qapi_Net_SSL_Cipher_Add(cfg, QAPI_NET_TLS_PSK_WITH_AES_256_CBC_SHA384);
            }
            else if (0 == strcmp("QAPI_NET_TLS_PSK_WITH_AES_128_CBC_SHA", Parameter_List[argn].String_Value))
            {
                cipher_selection_status = qapi_Net_SSL_Cipher_Add(cfg, QAPI_NET_TLS_PSK_WITH_AES_128_CBC_SHA);
            }
            else if (0 == strcmp("QAPI_NET_TLS_PSK_WITH_AES_256_CBC_SHA", Parameter_List[argn].String_Value))
            {
                cipher_selection_status = qapi_Net_SSL_Cipher_Add(cfg, QAPI_NET_TLS_PSK_WITH_AES_256_CBC_SHA);
            }
            else if (0 == strcmp("QAPI_NET_TLS_PSK_WITH_AES_128_CCM_8", Parameter_List[argn].String_Value))
            {
                cipher_selection_status = qapi_Net_SSL_Cipher_Add(cfg, QAPI_NET_TLS_PSK_WITH_AES_128_CCM_8);
            }
            else if (0 == strcmp("QAPI_NET_TLS_ECDHE_PSK_WITH_AES_128_CBC_SHA256", Parameter_List[argn].String_Value))
            {
                cipher_selection_status = qapi_Net_SSL_Cipher_Add(cfg, QAPI_NET_TLS_ECDHE_PSK_WITH_AES_128_CBC_SHA256);
            }
            else
            {
                SSL_PRINTF("Warning: Ignoring invalid cipher: %s\n", Parameter_List[argn].String_Value);
                is_valid_cipher = false;
            }

            if (is_valid_cipher && QAPI_OK != cipher_selection_status)
            {
                SSL_PRINTF("Warning: Unsupported cipher: %s\n", Parameter_List[argn].String_Value);
            }
        }

        /* 'max_frag_len 1000' */
        else if (0 == strncmp("max_frag_len", Parameter_List[argn].String_Value, 12))
        {
            argn++;
            cfg->max_Frag_Len = atoi(Parameter_List[argn].String_Value);
        }

        /* 'neg_disable 1' */
        else
        if (0 == strncmp("neg_disable", Parameter_List[argn].String_Value, 11))
        {
            argn++;
            cfg->max_Frag_Len_Neg_Disable = atoi(Parameter_List[argn].String_Value);
        }

        /* 'sni xxxx' */
        else if(0 == strncmp("sni", Parameter_List[argn].String_Value, 3))
        {
            argn++;

            if (server)
            {
                SSL_PRINTF("Warning: ignoring SNI option, this only valid for clients\n");
            }
            else
            {
                cfg->sni_Name_Size = strlen(Parameter_List[argn].String_Value);
                cfg->sni_Name = malloc(cfg->sni_Name_Size);
                if (cfg->sni_Name == NULL)
                {
                    SSL_PRINTF("Error: unable to allocate memory for password\n");
                    return QCLI_STATUS_ERROR_E;
                }
                memcpy(cfg->sni_Name, Parameter_List[argn].String_Value, cfg->sni_Name_Size);
            }
        }

        /* 'alpn http/1.1' */
        else if (strncmp("alpn", Parameter_List[argn].String_Value, 4) == 0) {
        	int ret = 0;
        	argn++;

            /* ignore 'alpn' parameter if ssl object handle is zero */
            if (ssl_hdl != 0)
            {
                ret = qapi_Net_SSL_ALPN_Protocol_Add(ssl_hdl, Parameter_List[argn].String_Value);
                if (ret != 0) {
                    SSL_PRINTF("Error: ALPN Protocol add failed %d\n", ret);
                    return QCLI_STATUS_ERROR_E;
                }
            }
        }

    } /*for*/

    return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t ssl_config(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    SSL_INST *ssl;
    int32_t res;
    qbool_t server = false;
    QCLI_Command_Status_t status;

    if(Parameter_Count < 1)
    {
        goto usage;
    }

    if (0 == strncmp(Parameter_List[0].String_Value, "server", 3))
    {
        ssl = &ssl_inst[SSL_SERVER_INST];
        server = true;
    }
    else if (0 == strncmp(Parameter_List[0].String_Value, "client", 3))
    {
        ssl = &ssl_inst[SSL_CLIENT_INST];
    }
    else
    {
        goto usage;
    }

    if (ssl->sslCtx == QAPI_NET_SSL_INVALID_HANDLE)
    {
        SSL_PRINTF("ERROR: SSL %s not started\n", Parameter_List[0].String_Value);
        return QCLI_STATUS_ERROR_E;
    }

    ssl->config_set = 0;
    /* Parse SSL config parameters from command line */
    status = ssl_parse_config_parameters(
                    Parameter_Count-1,
                    &Parameter_List[1],
                    &ssl->config,
                    ssl->sslCtx,
                    server);
    if (status == QCLI_STATUS_USAGE_E)
    {
        goto usage;
    }
    else if (status != QCLI_STATUS_SUCCESS_E)
    {
        return QCLI_STATUS_ERROR_E;
    }

    if (ssl->ssl == QAPI_NET_SSL_INVALID_HANDLE)
    {
        // Create SSL connection object
        qapi_Net_SSL_Protocol_t proto;
        switch(ssl->config.protocol)
        {
            case QAPI_NET_SSL_PROTOCOL_TLS_1_0:
            case QAPI_NET_SSL_PROTOCOL_TLS_1_1:
            case QAPI_NET_SSL_PROTOCOL_TLS_1_2:
                proto = QAPI_NET_SSL_TLS_E;
                break;
            case QAPI_NET_SSL_PROTOCOL_DTLS_1_0:
            case QAPI_NET_SSL_PROTOCOL_DTLS_1_2:
                proto = QAPI_NET_SSL_DTLS_E;
                break;
            default:
                /* No specific protocol, negotiate */
                proto = ssl->config.protocol;
                ssl->config.protocol = 0;
                break;
        }
        ssl->ssl = qapi_Net_SSL_Con_New(ssl->sslCtx, proto);
        if (ssl->ssl == QAPI_NET_SSL_INVALID_HANDLE)
        {
            SSL_PRINTF("ERROR: SSL configure failed (Unable to create SSL context)\n");
            return QCLI_STATUS_ERROR_E;
        }
    }
    else
    {
        if (ssl->config.protocol <= QAPI_NET_SSL_DTLS_E)
            ssl->config.protocol = 0;
    }

    // configure the SSL connection
    ssl->config_set = 1;
    res = qapi_Net_SSL_Configure(ssl->ssl, &ssl->config);
    if (res < 0)
    {
        SSL_PRINTF("ERROR: SSL configure failed (%d)\n", res);
        return QCLI_STATUS_ERROR_E;
    }
    SSL_PRINTF("SSL %s configuration changed\n", Parameter_List[0].String_Value);
    return QCLI_STATUS_SUCCESS_E;

usage:
    sslconfig_help("\nssl config <server|client>");
    return QCLI_STATUS_ERROR_E;
}

QCLI_Command_Status_t ssl_add_cert(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    SSL_INST *ssl;
    qapi_Net_SSL_Cert_Type_t type;
    char *name = NULL;

    if(Parameter_Count < 3)
    {
usage:
        SSL_PRINTF("\nUsage: ssl cert <server|client> <certificate|calist> <name>\r\n"
                    " This adds a certificate or CA list to either SSL server or client.\r\n"
                    "      where <name> = name of file to load from secure storage repository.\r\n");
        return QCLI_STATUS_ERROR_E;
    }

    if (0 == strncmp(Parameter_List[0].String_Value, "server", 3))
    {
        ssl = &ssl_inst[SSL_SERVER_INST];
        if (ssl->sslCtx == QAPI_NET_SSL_INVALID_HANDLE)
        {
            SSL_PRINTF("ERROR: SSL server not started\n");
            return QCLI_STATUS_ERROR_E;
        }
    }
    else if (0 == strncmp(Parameter_List[0].String_Value, "client", 3))
    {
        ssl = &ssl_inst[SSL_CLIENT_INST];
        if (ssl->sslCtx == QAPI_NET_SSL_INVALID_HANDLE)
        {
            SSL_PRINTF("ERROR: SSL client not started\n");
            return QCLI_STATUS_ERROR_E;
        }
    }
    else
    {
        goto usage;
    }

    if (0 == strncmp("certificate", Parameter_List[1].String_Value, 3))
    {
        type = QAPI_NET_SSL_CERTIFICATE_E;
    }
    else if (0 == strncmp("calist", Parameter_List[1].String_Value, 3))
    {
        type = QAPI_NET_SSL_CA_LIST_E;
    }
    else
    {
        goto usage;
    }

    name = Parameter_List[2].String_Value;

    // Load/add certificate
    if (name != NULL)
    {
        if (qapi_Net_SSL_Cert_Load(ssl->sslCtx, type, name) < 0)
        {
            SSL_PRINTF("ERROR: Unable to load %s from secure storage repository\n" , name);
            return QCLI_STATUS_ERROR_E;
        }
        SSL_PRINTF("%s loaded from secure storage repository\n", name);
    }
    else
    {
        SSL_PRINTF("ERROR: Must specify name for certificate / CA list\r\n");
        goto usage;
    }
    return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t ssl_set_dtls_server_max_clients(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if(Parameter_Count < 1)
    {
        SSL_PRINTF("\nUsage: ssl max_clients <max_Clients> \r\n"
                    " Sets DTLS maximum number of clients per server connection.  Defaults to 1 if not set.\r\n");
        return QCLI_STATUS_ERROR_E;
    }

    SSL_INST *ssl;
    ssl = &ssl_inst[SSL_SERVER_INST];
    if (ssl->sslCtx == QAPI_NET_SSL_INVALID_HANDLE)
    {
        SSL_PRINTF("ERROR: SSL server not started\n");
        return QCLI_STATUS_ERROR_E;
    }

    qapi_Status_t status = qapi_Net_SSL_Max_Clients_Set(ssl->sslCtx, Parameter_List[0].Integer_Value);
	if(status != QAPI_OK) {
        SSL_PRINTF("ERROR: error setting max_clients, (status = %d)\n", status);
        return QCLI_STATUS_ERROR_E;
	}

	return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t ssl_set_dtls_server_idle_timer(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if(Parameter_Count < 1)
    {
        SSL_PRINTF("\nUsage: ssl idle_timer <idle_timeout> \r\n"
                    " For DTLS servers, sets the number of seconds to wait before closing an idle client connection.\r\n");
        return QCLI_STATUS_ERROR_E;
    }

    SSL_INST *ssl;
    ssl = &ssl_inst[SSL_SERVER_INST];
    if (ssl->sslCtx == QAPI_NET_SSL_INVALID_HANDLE)
    {
        SSL_PRINTF("ERROR: SSL server not started\n");
        return QCLI_STATUS_ERROR_E;
    }

    qapi_Status_t status = qapi_Net_SSL_Idle_Timeout_Set(ssl->sslCtx, Parameter_List[0].Integer_Value);
	if(status != QAPI_OK) {
        SSL_PRINTF("ERROR: error setting idle_timeout, (status = %d)\n", status);
        return QCLI_STATUS_ERROR_E;
	}

	return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t ssl_set_ecjpake_params(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if(Parameter_Count < 2)
    {
usage:
        SSL_PRINTF("\nUsage: ssl ecjpake <server|client> <password>\r\n"
                    " Sets the password for ECJPAKE cipher suite (ECJPAKE).\r\n");
        return QCLI_STATUS_ERROR_E;
    }

	SSL_INST *ssl;
    if (0 == strncmp(Parameter_List[0].String_Value, "server", 3))
    {
        ssl = &ssl_inst[SSL_SERVER_INST];
        if (ssl->sslCtx == QAPI_NET_SSL_INVALID_HANDLE)
        {
            SSL_PRINTF("ERROR: SSL server not started\n");
            return QCLI_STATUS_ERROR_E;
        }
    }
    else if (0 == strncmp(Parameter_List[0].String_Value, "client", 3))
    {
        ssl = &ssl_inst[SSL_CLIENT_INST];
        if (ssl->sslCtx == QAPI_NET_SSL_INVALID_HANDLE)
        {
            SSL_PRINTF("ERROR: SSL client not started\n");
            return QCLI_STATUS_ERROR_E;
        }
    }
    else
    {
        goto usage;
    }

    qapi_Net_SSL_ECJPAKE_Params_t ecjpake_params = {0};
    ecjpake_params.password = (uint8_t *)Parameter_List[1].String_Value;
    ecjpake_params.password_Size = strlen(Parameter_List[1].String_Value);
    qapi_Status_t status = qapi_Net_SSL_ECJPAKE_Parameters_Set(ssl->sslCtx, &ecjpake_params);
	if(status != QAPI_OK) {
        SSL_PRINTF("ERROR: error setting ecjpake parameters (status = %d)\n", status);
        return QCLI_STATUS_ERROR_E;
	}

	return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t ssl_add_psk_table(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if(Parameter_Count < 3)
    {
usage:
        SSL_PRINTF("\nUsage: ssl psk <server|client> <identity> <psk>\r\n"
                    " This creates a psk table for either the SSL server or client.\r\n");
        return QCLI_STATUS_ERROR_E;
    }

	SSL_INST *ssl;
    if (0 == strncmp(Parameter_List[0].String_Value, "server", 3))
    {
        ssl = &ssl_inst[SSL_SERVER_INST];
        if (ssl->sslCtx == QAPI_NET_SSL_INVALID_HANDLE)
        {
            SSL_PRINTF("ERROR: SSL server not started\n");
            return QCLI_STATUS_ERROR_E;
        }
    }
    else if (0 == strncmp(Parameter_List[0].String_Value, "client", 3))
    {
        ssl = &ssl_inst[SSL_CLIENT_INST];
        if (ssl->sslCtx == QAPI_NET_SSL_INVALID_HANDLE)
        {
            SSL_PRINTF("ERROR: SSL client not started\n");
            return QCLI_STATUS_ERROR_E;
        }
    }
    else
    {
        goto usage;
    }

	qapi_Net_SSL_PSK_t psk_list =
	{
		.identity = (uint8_t *)Parameter_List[1].String_Value,
		.identity_Size = strlen(Parameter_List[1].String_Value),
		.psk = (uint8_t *)Parameter_List[2].String_Value,
		.psk_Size = strlen(Parameter_List[2].String_Value)
	};
	if(qapi_Net_SSL_PSK_Table_Set(ssl->sslCtx, &psk_list, 1) != QAPI_OK) {
        SSL_PRINTF("ERROR: error setting psk table\n");
        return QCLI_STATUS_ERROR_E;
	}

	return QCLI_STATUS_SUCCESS_E;
}
#endif
