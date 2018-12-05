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

#ifndef __QC_DRV_MQTT_H
#define __QC_DRV_MQTT_H

#include "qc_drv_main.h"

qapi_Status_t qc_drv_net_mqtt_init(qc_drv_context *qc_drv_ctx, char *ca_file);

qapi_Status_t qc_drv_net_mqtt_shutdown(qc_drv_context *qc_drv_ctx);

qapi_Status_t qc_drv_net_mqtt_new(qc_drv_context *qc_drv_ctx, const char *client_id, qbool_t clean_session, qapi_Status_t *status);

qapi_Status_t qc_drv_net_mqtt_destroy(qc_drv_context *qc_drv_ctx, int32_t handle);

qapi_Status_t qc_drv_net_mqtt_set_username_password(qc_drv_context *qc_drv_ctx, int32_t handle, const char *user, size_t user_len, const char *pw, size_t pw_len, qapi_Status_t *status);

qapi_Status_t qc_drv_net_mqtt_set_will(qc_drv_context *qc_drv_ctx, int32_t handle, const char *topic, size_t topic_len, const char *msg, size_t msg_len, uint32_t qos, qbool_t retained, qapi_Status_t *status);

qapi_Status_t qc_drv_net_mqtt_set_keep_alive(qc_drv_context *qc_drv_ctx, int32_t handle, uint16_t keepalive_sec);

qapi_Status_t qc_drv_net_mqtt_set_connack_wait_time(qc_drv_context *qc_drv_ctx, int32_t handle, uint16_t max_conn_pending_sec);

qapi_Status_t qc_drv_net_mqtt_set_ssl_config(qc_drv_context *qc_drv_ctx, int32_t handle, qapi_Net_SSL_Config_t *mqttc_sslcfg, qapi_Status_t *status);

qapi_Status_t qc_drv_net_mqtt_connect(qc_drv_context *qc_drv_ctx, int32_t handle, char *host, qbool_t secure_session, qbool_t nonblocking_connect, char *bind_if, qapi_Status_t *status);

qapi_Status_t qc_drv_net_mqtt_subscribe(qc_drv_context *qc_drv_ctx, int32_t handle, const char *topic, uint32_t qos, qapi_Status_t *status);

qapi_Status_t qc_drv_net_mqtt_publish(qc_drv_context *qc_drv_ctx, int32_t handle, const char *topic, size_t topic_len, const char *msg, uint32_t msg_len, uint32_t qos, qbool_t retained, qbool_t dup, qapi_Status_t *status);

qapi_Status_t qc_drv_net_mqtt_unsubscribe(qc_drv_context *qc_drv_ctx, int32_t handle, const char *topic, qapi_Status_t *status);

qapi_Status_t qc_drv_net_mqtt_disconnect(qc_drv_context *qc_drv_ctx, int32_t handle, qapi_Status_t *status);

#endif
