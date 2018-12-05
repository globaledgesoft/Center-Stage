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

#include "qc_drv_p2p.h"

qapi_Status_t
qc_drv_p2p_set_config(qc_drv_context *qc_drv_ctx, uint32_t deviceId,
		qapi_WLAN_P2P_Config_Params_t *p2pConfig, int size, int flag)
{
	if (is_drv_cb_valid(qc_drv_p2p_set_config)) {
        return qc_drv_ctx->drv_ops->qc_drv_p2p_set_config(deviceId, p2pConfig, size, flag);
    }

    return QCLI_STATUS_ERROR_E;
}

qapi_Status_t
qc_drv_p2p_connect(qc_drv_context *qc_drv_ctx, uint32_t deviceId, uint8_t wps_Method,
        uint8_t *peer_Addr, uint8_t p2p_pers_go)
{
	if (is_drv_cb_valid(qc_drv_p2p_connect)) {
        return qc_drv_ctx->drv_ops->qc_drv_p2p_connect(deviceId, wps_Method, peer_Addr, p2p_pers_go);
    }

    return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_p2p_listen(qc_drv_context *qc_drv_ctx, uint32_t deviceId, uint32_t timeout_val)
{
	if (is_drv_cb_valid(qc_drv_p2p_listen)) {
        return qc_drv_ctx->drv_ops->qc_drv_p2p_listen(deviceId, timeout_val);
    }

    return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_p2p_cancel(qc_drv_context *qc_drv_ctx, uint32_t deviceId)
{
	if (is_drv_cb_valid(qc_drv_p2p_cancel)) {
        return qc_drv_ctx->drv_ops->qc_drv_p2p_cancel(deviceId);
    }

    return QCLI_STATUS_ERROR_E;
}


qapi_Status_t
qc_drv_p2p_enable(qc_drv_context *qc_drv_ctx, uint32_t deviceId, int flag)
{
	if (is_drv_cb_valid(qc_drv_p2p_enable)) {
		return qc_drv_ctx->drv_ops->qc_drv_p2p_enable(deviceId, flag);
	}

	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t
qc_drv_p2p_disable(qc_drv_context *qc_drv_ctx, uint32_t deviceId, int flag)
{
	if (is_drv_cb_valid(qc_drv_p2p_disable)) {
		return qc_drv_ctx->drv_ops->qc_drv_p2p_disable(deviceId, flag);
	}

	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t
qc_drv_p2p_find(qc_drv_context *qc_drv_ctx, uint32_t deviceId, uint8_t type, uint32_t timeout)
{
	if (is_drv_cb_valid(qc_drv_p2p_find)) {
		return qc_drv_ctx->drv_ops->qc_drv_p2p_find(deviceId, type, timeout);
	}

	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t
qc_drv_p2p_provision(qc_drv_context *qc_drv_ctx, uint32_t deviceId, uint16_t wps_Method, uint8_t *peer)
{
	if (is_drv_cb_valid(qc_drv_p2p_provision)) {
        return qc_drv_ctx->drv_ops->qc_drv_p2p_provision(deviceId, wps_Method, peer);
    }

    return QCLI_STATUS_ERROR_E;
}

qapi_Status_t
qc_drv_p2p_join(qc_drv_context *qc_drv_ctx, uint32_t deviceId, uint8_t wps_Method,
        uint8_t *p2p_join_mac_addr, char *p2p_wps_pin, uint16_t go_Oper_Freq)
{
	if (is_drv_cb_valid(qc_drv_p2p_join)) {
        return qc_drv_ctx->drv_ops->qc_drv_p2p_join(deviceId, wps_Method, p2p_join_mac_addr, p2p_wps_pin, go_Oper_Freq);
    }

    return QCLI_STATUS_ERROR_E;

}

qapi_Status_t
qc_drv_p2p_auth(qc_drv_context *qc_drv_ctx, uint32_t deviceId, uint8_t dev_Auth,
        qapi_WLAN_P2P_WPS_Method_e wps_Method, uint8_t *peer_Addr, uint8_t p2p_persistent_go)
{
	if (is_drv_cb_valid(qc_drv_p2p_auth)) {
        return qc_drv_ctx->drv_ops->qc_drv_p2p_auth(deviceId, dev_Auth, wps_Method, peer_Addr, p2p_persistent_go);
    }

    return QCLI_STATUS_ERROR_E;
}

qapi_Status_t
qc_drv_p2p_invite(qc_drv_context *qc_drv_ctx, uint32_t deviceId, const char *string,
        uint8_t wps_Method, uint8_t *peer_Addr, uint8_t is_Persistent, uint8_t p2p_invite_role)
{
	if (is_drv_cb_valid(qc_drv_p2p_invite)) {
        return qc_drv_ctx->drv_ops->qc_drv_p2p_invite(deviceId, string, wps_Method, peer_Addr, is_Persistent, p2p_invite_role);
    }

    return QCLI_STATUS_ERROR_E;
}

qapi_Status_t
qc_drv_p2p_get_node_list(qc_drv_context *qc_drv_ctx, uint32_t deviceId, qapi_WLAN_P2P_Node_List_Params_t *p2pNodeList, uint32_t *dataLen)
{
	if (is_drv_cb_valid(qc_drv_p2p_get_node_list)) {
        return qc_drv_ctx->drv_ops->qc_drv_p2p_get_node_list(deviceId, p2pNodeList, dataLen);
    }

    return QCLI_STATUS_ERROR_E;
}

qapi_Status_t
qc_drv_p2p_get_network_list(qc_drv_context *qc_drv_ctx, uint32_t deviceId,
			qapi_WLAN_P2P_Network_List_Params_t *p2pNetworkList, uint32_t *dataLen)
{
	if (is_drv_cb_valid(qc_drv_p2p_get_network_list)) {
        return qc_drv_ctx->drv_ops->qc_drv_p2p_get_network_list(deviceId, p2pNetworkList, dataLen);
    }

    return QCLI_STATUS_ERROR_E;
}

qapi_Status_t
qc_drv_p2p_set_oops_params(qc_drv_context *qc_drv_ctx, uint32_t deviceId, qapi_WLAN_P2P_Opps_Params_t *opps, int size, int flag)
{
	if (is_drv_cb_valid(qc_drv_p2p_set_oops_params)) {
        return qc_drv_ctx->drv_ops->qc_drv_p2p_set_oops_params(deviceId, opps, size, flag);
    }

    return QCLI_STATUS_ERROR_E;
}

qapi_Status_t
qc_drv_p2p_set_noa_params(qc_drv_context *qc_drv_ctx, uint32_t deviceId, qapi_WLAN_P2P_Noa_Params_t *noaParams, int size, int flag)
{
	if (is_drv_cb_valid(qc_drv_p2p_set_noa_params)) {
        return qc_drv_ctx->drv_ops->qc_drv_p2p_set_noa_params(deviceId, noaParams, size, flag);
    }

    return QCLI_STATUS_ERROR_E;
}

qapi_Status_t
qc_drv_p2p_set_operating_class(qc_drv_context *qc_drv_ctx, uint32_t deviceId, qapi_WLAN_P2P_Config_Params_t *p2pConfig, int size, int flag)
{
	if (is_drv_cb_valid(qc_drv_p2p_set_operating_class)) {
        return qc_drv_ctx->drv_ops->qc_drv_p2p_set_operating_class(deviceId, p2pConfig, size, flag);
    }

    return QCLI_STATUS_ERROR_E;
}

qapi_Status_t
qc_drv_p2p_stop_find(qc_drv_context *qc_drv_ctx, uint32_t deviceId)
{
	if (is_drv_cb_valid(qc_drv_p2p_stop_find)) {
        return qc_drv_ctx->drv_ops->qc_drv_p2p_stop_find(deviceId);
    }

    return QCLI_STATUS_ERROR_E;
}

qapi_Status_t
qc_drv_p2p_set_ssidparam(qc_drv_context *qc_drv_ctx, uint32_t deviceId, qapi_WLAN_P2P_Go_Params_t *ssidParams, int size, int flag)
{
	if (is_drv_cb_valid(qc_drv_p2p_set_ssidparam)) {
        return qc_drv_ctx->drv_ops->qc_drv_p2p_set_ssidparam(deviceId, ssidParams, size, flag);
    }

    return QCLI_STATUS_ERROR_E;
}

qapi_Status_t
qc_drv_p2p_set(qc_drv_context *qc_drv_ctx, uint32_t deviceId,
        uint8_t config_Id, int *val, uint32_t len, int flag)
{
	if (is_drv_cb_valid(qc_drv_p2p_set)) {
        return qc_drv_ctx->drv_ops->qc_drv_p2p_set(deviceId, config_Id, val, len, flag);
    }

    return QCLI_STATUS_ERROR_E;
}

qapi_Status_t
qc_drv_p2p_start_go(qc_drv_context *qc_drv_ctx, uint32_t deviceId,
        qapi_WLAN_P2P_Go_Params_t *goParams, int32_t go_chan, uint8_t persistent_Group)
{
	if (is_drv_cb_valid(qc_drv_p2p_start_go)) {
        return qc_drv_ctx->drv_ops->qc_drv_p2p_start_go(deviceId, goParams, go_chan, persistent_Group);
    }

    return QCLI_STATUS_ERROR_E;
}

qapi_Status_t
qc_drv_p2p_set_param(qc_drv_context *qc_drv_ctx, uint32_t deviceId,
		uint8_t *p2pmode, int size, int flag)
{
	if (is_drv_cb_valid(qc_drv_p2p_set_param)) {
        return qc_drv_ctx->drv_ops->qc_drv_p2p_set_param(deviceId, p2pmode, size, flag);
    }

    return QCLI_STATUS_ERROR_E;
}

qapi_Status_t
qc_drv_p2p_invite_auth(qc_drv_context *qc_drv_ctx, uint8_t deviceId, 
        const qapi_WLAN_P2P_Invite_Info_t  *invite_Info)
{
	if (is_drv_cb_valid(qc_drv_p2p_invite_auth)) {
        return qc_drv_ctx->drv_ops->qc_drv_p2p_invite_auth(deviceId, invite_Info);
    }

    return QCLI_STATUS_ERROR_E;
}

