/*
 * Copyright (c) 2016-2018 Qualcomm Technologies, Inc.
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

 
#include "qc_drv_net.h"

qapi_Status_t qc_drv_net_errno(qc_drv_context *qc_drv_ctx, int32_t sock_id)
{
        if (is_drv_cb_valid(qc_drv_net_errno)) {
                return qc_drv_ctx->drv_ops->qc_drv_net_errno(sock_id);
        }
        return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_DNSc_Reshost(qc_drv_context *qc_drv_ctx,
		char *hostname, struct ip46addr *ipaddr)
{
        if (is_drv_cb_valid(qc_drv_net_DNSc_Reshost)) {
                return qc_drv_ctx->drv_ops->qc_drv_net_DNSc_Reshost(hostname, ipaddr);
        }
        return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_Ping(qc_drv_context *qc_drv_ctx, uint32_t ipv4_Addr,
		uint32_t size)
{
        if (is_drv_cb_valid(qc_drv_net_Ping)) {
                return qc_drv_ctx->drv_ops->qc_drv_net_Ping(ipv4_Addr, size);
        }
        return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_Get_All_Ifnames(qc_drv_context *qc_drv_ctx,
		qapi_Net_Ifnameindex_t *if_Name_Index)
{
        if (is_drv_cb_valid(qc_drv_net_Get_All_Ifnames)) {
                return qc_drv_ctx->drv_ops->qc_drv_net_Get_All_Ifnames(if_Name_Index);
        }
        return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_Interface_Exist(qc_drv_context *qc_drv_ctx,
		const char * interface_Name, qbool_t *if_Is_Up)
{
        if (is_drv_cb_valid(qc_drv_net_Interface_Exist)) {
                return qc_drv_ctx->drv_ops->qc_drv_net_Interface_Exist(interface_Name, if_Is_Up);
        }
        return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_IPv4_Config(qc_drv_context *qc_drv_ctx, const char *interface_Name,
			qapi_Net_IPv4cfg_Command_t cmd, uint32_t *ipv4_Addr,
			uint32_t *subnet_Mask, uint32_t *gateway)
{
        if (is_drv_cb_valid(qc_drv_net_IPv4_Config)) {
                return qc_drv_ctx->drv_ops->qc_drv_net_IPv4_Config(
						interface_Name, cmd, ipv4_Addr, subnet_Mask, gateway);
        }
        return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_Bridge_Show_MACs(qc_drv_context *qc_drv_ctx,
				qapi_Net_Bridge_DB_Entry_t **ptr,
				uint32_t *count)
{
        if (is_drv_cb_valid(qc_drv_net_Bridge_Show_MACs)) {
                return qc_drv_ctx->drv_ops->qc_drv_net_Bridge_Show_MACs(ptr, count);
        }
        return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_Interface_Get_Physical_Address(qc_drv_context *qc_drv_ctx, const char *interface_Name, const uint8_t **address, uint32_t *address_Len)
{
        if (is_drv_cb_valid(qc_drv_net_Interface_Get_Physical_Address)) {
                return qc_drv_ctx->drv_ops->qc_drv_net_Interface_Get_Physical_Address(interface_Name, address, address_Len);
        }
        return QCLI_STATUS_ERROR_E;
}
qapi_Status_t  qc_drv_net_IPv6_Get_Address(qc_drv_context *qc_drv_ctx, const char *interface_Name,
        uint8_t *link_Local,
        uint8_t *global,
        uint8_t *default_Gateway,
        uint8_t *global_Second,
        uint32_t *link_Local_Prefix,
        uint32_t *global_Prefix,
        uint32_t *default_Gateway_Prefix,
        uint32_t *global_Second_Prefix)
{
    if (is_drv_cb_valid(qc_drv_net_IPv6_Get_Address)) {
        return qc_drv_ctx->drv_ops->qc_drv_net_IPv6_Get_Address(interface_Name, link_Local, global, default_Gateway, global_Second, link_Local_Prefix, global_Prefix, default_Gateway_Prefix, global_Second_Prefix);
    }
        return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_socket(qc_drv_context *qc_drv_ctx, int32_t family, int32_t type, int32_t protocol)
{
    if (is_drv_cb_valid(qc_drv_net_socket)) {
            return qc_drv_ctx->drv_ops->qc_drv_net_socket(family, type, protocol);
            }
        return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_socketclose(qc_drv_context *qc_drv_ctx, int32_t handle)
{
    if (is_drv_cb_valid(qc_drv_net_socketclose)) {
            return qc_drv_ctx->drv_ops->qc_drv_net_socketclose(handle);
            }
        return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_connect(qc_drv_context *qc_drv_ctx, int32_t handle, struct sockaddr *srvaddr, int32_t addrlen)
{
    if (is_drv_cb_valid(qc_drv_net_connect)) {
            return qc_drv_ctx->drv_ops->qc_drv_net_connect(handle, srvaddr, addrlen);
    }
        return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_send(qc_drv_context *qc_drv_ctx, int32_t handle, char *buf, int32_t len, int32_t flags)
{
    if (is_drv_cb_valid(qc_drv_net_send)) {
            return qc_drv_ctx->drv_ops->qc_drv_net_send(handle, buf, len, flags);
    }
        return QCLI_STATUS_ERROR_E;

}

qapi_Status_t qc_drv_net_recv(qc_drv_context *qc_drv_ctx, int32_t handle, char *buf, int32_t len, int32_t flags)
{
    if (is_drv_cb_valid(qc_drv_net_recv)) {
            return qc_drv_ctx->drv_ops->qc_drv_net_recv(handle, buf, len, flags);
    }
        return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_recvfrom(qc_drv_context *qc_drv_ctx, int32_t handle, char *buf, int32_t len, int32_t flags, struct sockaddr *from, int32_t *fromlen)
{
    if (is_drv_cb_valid(qc_drv_net_recvfrom)) {
            return qc_drv_ctx->drv_ops->qc_drv_net_recvfrom(handle, buf, len, flags, from, fromlen);
    }
        return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_sendto(qc_drv_context *qc_drv_ctx, int32_t handle, char *buf, int32_t len, int32_t flags, struct sockaddr *to, int32_t tolen)
{
    if (is_drv_cb_valid(qc_drv_net_sendto)) {
            return qc_drv_ctx->drv_ops->qc_drv_net_sendto(handle, buf, len, flags, to, tolen);
    }
    return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_bind(qc_drv_context *qc_drv_ctx, int32_t handle, struct sockaddr *addr, int32_t addrlen)
{
    if (is_drv_cb_valid(qc_drv_net_bind)) {
            return qc_drv_ctx->drv_ops->qc_drv_net_bind(handle, addr, addrlen);
    }
        return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_listen(qc_drv_context *qc_drv_ctx, int32_t handle, int32_t backlog)
{
    if (is_drv_cb_valid(qc_drv_net_listen)) {
            return qc_drv_ctx->drv_ops->qc_drv_net_listen(handle, backlog);
    }
        return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_accept(qc_drv_context *qc_drv_ctx, int32_t handle, struct sockaddr *cliaddr, int32_t *addrlen)
{
    if (is_drv_cb_valid(qc_drv_net_accept)) {
            return qc_drv_ctx->drv_ops->qc_drv_net_accept(handle, cliaddr, addrlen);
    }
        return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_Bridge_Set_Aging_Timeout(qc_drv_context *qc_drv_ctx, int32_t Integer_Value)
{
        if (is_drv_cb_valid(qc_drv_net_Bridge_Set_Aging_Timeout)) {
                return qc_drv_ctx->drv_ops->qc_drv_net_Bridge_Set_Aging_Timeout(Integer_Value);
        }
        return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_Bridge_Enable(qc_drv_context *qc_drv_ctx, uint8_t enable)
{
        if (is_drv_cb_valid(qc_drv_net_Bridge_Enable)) {
                return qc_drv_ctx->drv_ops->qc_drv_net_Bridge_Enable(enable);
        }
        return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_HTTPs_Set_Buffer_Size(qc_drv_context *qc_drv_ctx,
                                uint16_t txbufsize, uint16_t rxbufsize)
{
        if (is_drv_cb_valid(qc_drv_net_HTTPs_Set_Buffer_Size)) {
                return qc_drv_ctx->drv_ops->qc_drv_net_HTTPs_Set_Buffer_Size(
                                                txbufsize, rxbufsize);
        }
        return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_HTTPs_Unregister_Content_Type(qc_drv_context *qc_drv_ctx,
				char *String_Value, int String_len)
{
        if (is_drv_cb_valid(qc_drv_net_HTTPs_Unregister_Content_Type)) {
                return qc_drv_ctx->drv_ops->qc_drv_net_HTTPs_Unregister_Content_Type(
						String_Value, String_len);
        }
        return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_HTTPs_Register_Content_Type(qc_drv_context *qc_drv_ctx,
				char *String_Value, int String_len)
{
        if (is_drv_cb_valid(qc_drv_net_HTTPs_Register_Content_Type)) {
                return qc_drv_ctx->drv_ops->qc_drv_net_HTTPs_Register_Content_Type(
						String_Value, String_len);
        }
        return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_HTTPs_Shutdown(qc_drv_context *qc_drv_ctx)
{
        if (is_drv_cb_valid(qc_drv_net_HTTPs_Shutdown)) {
                return qc_drv_ctx->drv_ops->qc_drv_net_HTTPs_Shutdown();
        }
        return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_HTTPs_Stop(qc_drv_context *qc_drv_ctx)
{
        if (is_drv_cb_valid(qc_drv_net_HTTPs_Stop)) {
                return qc_drv_ctx->drv_ops->qc_drv_net_HTTPs_Stop();
        }
        return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_HTTPs_Start(qc_drv_context *qc_drv_ctx)
{
        if (is_drv_cb_valid(qc_drv_net_HTTPs_Start)) {
                return qc_drv_ctx->drv_ops->qc_drv_net_HTTPs_Start();
        }
        return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_HTTPs_Set_SSL_Config(qc_drv_context *qc_drv_ctx, qapi_Net_SSL_Config_t * httpsvr_sslcfg)
{
        if (is_drv_cb_valid(qc_drv_net_HTTPs_Set_SSL_Config)) {
                return qc_drv_ctx->drv_ops->qc_drv_net_HTTPs_Set_SSL_Config(httpsvr_sslcfg);
        }
        return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_HTTPs_Init(qc_drv_context *qc_drv_ctx, qapi_Net_HTTPs_Config_t *cfg)
{
        if (is_drv_cb_valid(qc_drv_net_HTTPs_Init)) {
                return qc_drv_ctx->drv_ops->qc_drv_net_HTTPs_Init(cfg);
        }
        return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_api_get_HTTPs_Is_Started(qc_drv_context *qc_drv_ctx)
{
        if (is_drv_cb_valid(qc_api_get_HTTPs_Is_Started)) {
                return qc_drv_ctx->drv_ops->qc_api_get_HTTPs_Is_Started();
        }
        return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_HTTPs_Get_Status(qc_drv_context *qc_drv_ctx, qapi_Net_HTTPs_Status_t *status)
{
        if (is_drv_cb_valid(qc_drv_net_HTTPs_Get_Status)) {
                return qc_drv_ctx->drv_ops->qc_drv_net_HTTPs_Get_Status(status);
        }
        return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_DHCPv4c_Register_Success_Callback (
		qc_drv_context *qc_drv_ctx, const char *interface_Name,
		qapi_Net_DHCPv4c_Success_CB_t CB)
{
        if (is_drv_cb_valid(qc_drv_net_DHCPv4c_Register_Success_Callback)) {
                return qc_drv_ctx->drv_ops->qc_drv_net_DHCPv4c_Register_Success_Callback(interface_Name, CB);
        }
        return QCLI_STATUS_ERROR_E;

}

qapi_Status_t qc_drv_net_DHCPv4c_Release(qc_drv_context *qc_drv_ctx,
		const char *interface_Name)
{
        if (is_drv_cb_valid(qc_drv_net_DHCPv4c_Release)) {
                return qc_drv_ctx->drv_ops->qc_drv_net_DHCPv4c_Release(interface_Name);
        }
        return QCLI_STATUS_ERROR_E;

}

qapi_Status_t qc_drv_net_Ping6(qc_drv_context *qc_drv_ctx,
		uint8_t ipv6_Addr[16], uint32_t size, const char * interface_Name)
{
        if (is_drv_cb_valid(qc_drv_net_Ping6)) {
                return qc_drv_ctx->drv_ops->qc_drv_net_Ping6(ipv6_Addr,
						size, interface_Name);
        }
        return QCLI_STATUS_ERROR_E;
}
qapi_Status_t  qc_drv_net_DHCPv4s_Register_Success_Callback(
		qc_drv_context *qc_drv_ctx, const char *interface_Name,
		        qapi_Net_DHCPv4s_Success_CB_t CB)
{
        if (is_drv_cb_valid(qc_drv_net_DHCPv4s_Register_Success_Callback)) {
                return qc_drv_ctx->drv_ops->qc_drv_net_DHCPv4s_Register_Success_Callback(interface_Name, CB);
        }
        return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_DHCPv4s_Set_Pool(qc_drv_context *qc_drv_ctx,
	   	const char *interface_Name, uint32_t start_IP,
		uint32_t end_IP, uint32_t lease_Time)
{
        if (is_drv_cb_valid(qc_drv_net_DHCPv4s_Set_Pool)) {
                return qc_drv_ctx->drv_ops->qc_drv_net_DHCPv4s_Set_Pool(
						interface_Name, start_IP, end_IP, lease_Time);
        }
        return QCLI_STATUS_ERROR_E;
}


qapi_Status_t qc_drv_net_DHCPv6c_Register_New_IPv6_Lease_Complete_Callback(qc_drv_context *qc_drv_ctx, const char * interface_Name, qapi_Net_DHCPv6c_New_IPv6_Lease_Complete_CB_t cb, void * cb_Ctxt)
{
        if (is_drv_cb_valid(qc_drv_net_DHCPv6c_Register_New_IPv6_Lease_Complete_Callback)) {
                return qc_drv_ctx->drv_ops->qc_drv_net_DHCPv6c_Register_New_IPv6_Lease_Complete_Callback(
						interface_Name, cb, cb_Ctxt);
        }
        return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_DHCPv6c_Register_Release_IPv6_Lease_Complete_Callback(qc_drv_context *qc_drv_ctx, const char * interface_Name, qapi_Net_DHCPv6c_Release_IPv6_Lease_Complete_CB_t cb, void *cb_Ctxt)
{
        if (is_drv_cb_valid(qc_drv_net_DHCPv6c_Register_Release_IPv6_Lease_Complete_Callback)) {
                return qc_drv_ctx->drv_ops->qc_drv_net_DHCPv6c_Register_Release_IPv6_Lease_Complete_Callback(
						interface_Name, cb, cb_Ctxt);
        }
        return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_DHCPv6c_Enable(qc_drv_context *qc_drv_ctx, const char * interface_Name)
{
        if (is_drv_cb_valid(qc_drv_net_DHCPv6c_Enable)) {
                return qc_drv_ctx->drv_ops->qc_drv_net_DHCPv6c_Enable(
						interface_Name);
        }
        return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_DHCPv6c_Disable(qc_drv_context *qc_drv_ctx, const char * interface_Name)
{
        if (is_drv_cb_valid(qc_drv_net_DHCPv6c_Disable)) {
                return qc_drv_ctx->drv_ops->qc_drv_net_DHCPv6c_Disable(
						interface_Name);
        }
        return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_DHCPv6c_Register_New_Prefix_Lease_Complete_Callback(qc_drv_context *qc_drv_ctx, const char * interface_Name,  qapi_Net_DHCPv6c_New_Prefix_Lease_Complete_CB_t cb, void *cb_Ctxt)
{
        if (is_drv_cb_valid(qc_drv_net_DHCPv6c_Register_New_Prefix_Lease_Complete_Callback)) {
                return qc_drv_ctx->drv_ops->qc_drv_net_DHCPv6c_Register_New_Prefix_Lease_Complete_Callback(
						interface_Name, cb, cb_Ctxt);
        }
        return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_DHCPv6c_Register_Release_Prefix_Lease_Complete_Callback(qc_drv_context *qc_drv_ctx, const char * interface_Name, qapi_Net_DHCPv6c_Release_Prefix_Lease_Complete_CB_t cb, void *cb_Ctxt)
{
        if (is_drv_cb_valid(qc_drv_net_DHCPv6c_Register_Release_Prefix_Lease_Complete_Callback)) {
                return qc_drv_ctx->drv_ops->qc_drv_net_DHCPv6c_Register_Release_Prefix_Lease_Complete_Callback(
						interface_Name, cb, cb_Ctxt);
        }
        return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_DHCPv6c_New_Lease(qc_drv_context *qc_drv_ctx, const char *dhcpv6c_Client_Interface_Name, const char * interface_Name)
{
        if (is_drv_cb_valid(qc_drv_net_DHCPv6c_New_Lease)) {
                return qc_drv_ctx->drv_ops->qc_drv_net_DHCPv6c_New_Lease(
						dhcpv6c_Client_Interface_Name, interface_Name);
        }
        return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_DHCPv6c_Release_Lease(qc_drv_context *qc_drv_ctx, const char *dhcpv6c_Client_Interface_Name, const char * interface_Name)
{
        if (is_drv_cb_valid(qc_drv_net_DHCPv6c_Release_Lease)) {
                return qc_drv_ctx->drv_ops->qc_drv_net_DHCPv6c_Release_Lease(
						dhcpv6c_Client_Interface_Name, interface_Name);
        }
        return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_DHCPv6c_Confirm_Lease(qc_drv_context *qc_drv_ctx, const char *dhcpv6c_Client_Interface_Name, const char * interface_Name)
{
        if (is_drv_cb_valid(qc_drv_net_DHCPv6c_Confirm_Lease)) {
                return qc_drv_ctx->drv_ops->qc_drv_net_DHCPv6c_Confirm_Lease(
						dhcpv6c_Client_Interface_Name, interface_Name);
        }
        return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_SNTPc_get_srvr_list(qc_drv_context *qc_drv_ctx,
		qapi_Net_SNTP_Server_List_t *svr_list)
{
        if (is_drv_cb_valid(qc_drv_net_SNTPc_get_srvr_list)) {
                return qc_drv_ctx->drv_ops->qc_drv_net_SNTPc_get_srvr_list(
						svr_list);
        }
		return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_SNTPc_del_srvr(qc_drv_context *qc_drv_ctx,
		uint32_t id)
{
        if (is_drv_cb_valid(qc_drv_net_SNTPc_del_srvr)) {
                return qc_drv_ctx->drv_ops->qc_drv_net_SNTPc_del_srvr(id);
        }
		return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_SNTPc_cmd(qc_drv_context *qc_drv_ctx,
		qapi_Net_SNTP_Command_t sntp_flag)
{
        if (is_drv_cb_valid(qc_drv_net_SNTPc_cmd)) {
                return qc_drv_ctx->drv_ops->qc_drv_net_SNTPc_cmd(sntp_flag);
        }
		return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_SNTPc_get_broke_down_time(qc_drv_context *qc_drv_ctx,
		qapi_Net_SNTP_Tm_t *tm)
{
        if (is_drv_cb_valid(qc_drv_net_SNTPc_get_broke_down_time)) {
                return qc_drv_ctx->drv_ops->qc_drv_net_SNTPc_get_broke_down_time(tm);
        }
		return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_SNTPc_start(qc_drv_context *qc_drv_ctx)
{
	if (is_drv_cb_valid(qc_drv_net_SNTPc_start)) {
		return qc_drv_ctx->drv_ops->qc_drv_net_SNTPc_start();
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_SNTPc_add_srvr(qc_drv_context *qc_drv_ctx,
		char *name, uint32_t ID)
{
	if (is_drv_cb_valid(qc_drv_net_SNTPc_add_srvr)) {
		return qc_drv_ctx->drv_ops->qc_drv_net_SNTPc_add_srvr(name, ID);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_DNSc_get_srvr_list(qc_drv_context *qc_drv_ctx,
		qapi_Net_DNS_Server_List_t *svr_list)
{
	if (is_drv_cb_valid(qc_drv_net_DNSc_get_srvr_list)) {
		return qc_drv_ctx->drv_ops->qc_drv_net_DNSc_get_srvr_list(svr_list);
	}

	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_DNSc_start(qc_drv_context *qc_drv_ctx)
{
	if (is_drv_cb_valid(qc_drv_net_DNSc_start)) {
		return qc_drv_ctx->drv_ops->qc_drv_net_DNSc_start();
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_DNSc_add_srvr(qc_drv_context *qc_drv_ctx,
		char *svr, uint32_t id)
{
	if (is_drv_cb_valid(qc_drv_net_DNSc_add_srvr)) {
		return qc_drv_ctx->drv_ops->qc_drv_net_DNSc_add_srvr(svr, id);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_DNSc_del_srvr(qc_drv_context *qc_drv_ctx, uint32_t id)
{
	if (is_drv_cb_valid(qc_drv_net_DNSc_del_srvr)) {
		return qc_drv_ctx->drv_ops->qc_drv_net_DNSc_del_srvr(id);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_DNSc_cmd(qc_drv_context *qc_drv_ctx,
		qapi_Net_DNS_Command_t dns_flag)
{
	if (is_drv_cb_valid(qc_drv_net_DNSc_cmd)) {
		return qc_drv_ctx->drv_ops->qc_drv_net_DNSc_cmd(dns_flag);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_IPv6_Get_Scope_ID(qc_drv_context *qc_drv_ctx, const char *interface_Name, int32_t *scope_ID)
{
	if (is_drv_cb_valid(qc_drv_net_IPv6_Get_Scope_ID)) {
		return qc_drv_ctx->drv_ops->qc_drv_net_IPv6_Get_Scope_ID(interface_Name, scope_ID);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_setsockopt(qc_drv_context *qc_drv_ctx, int32_t handle, int32_t level, int32_t optname, void *optval, int32_t optlen)
{

	if (is_drv_cb_valid(qc_drv_net_setsockopt)) {
		return qc_drv_ctx->drv_ops->qc_drv_net_setsockopt(handle, level, optname, optval, optlen);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_SSL_Write_To(qc_drv_context *qc_drv_ctx, qapi_Net_SSL_Con_Hdl_t ssl, void *buf, uint32_t num,
        struct sockaddr *to, int32_t to_Len)
{

	if (is_drv_cb_valid(qc_drv_net_SSL_Write_To)) {
		return qc_drv_ctx->drv_ops->qc_drv_net_SSL_Write_To(ssl, buf, num, to, to_Len);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_select(qc_drv_context *qc_drv_ctx, qapi_fd_set_t *rd, qapi_fd_set_t *wr, qapi_fd_set_t *ex, int32_t timeout_ms)
{

	if (is_drv_cb_valid(qc_drv_net_select)) {
		return qc_drv_ctx->drv_ops->qc_drv_net_select(rd, wr, ex, timeout_ms);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_SSL_Read(qc_drv_context *qc_drv_ctx, qapi_Net_SSL_Con_Hdl_t hdl, void *buf, uint32_t size)
{

	if (is_drv_cb_valid(qc_drv_net_SSL_Read)) {
		return qc_drv_ctx->drv_ops->qc_drv_net_SSL_Read(hdl, buf, size);
	}
	return QCLI_STATUS_ERROR_E;
}

void *qc_drv_net_Buf_Alloc(qc_drv_context *qc_drv_ctx, uint32_t size, uint32_t id)
{
    void *result;
	result = qc_drv_ctx->drv_ops->qc_drv_net_Buf_Alloc(size, id);
    return result;
//	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_fd_zero(qc_drv_context *qc_drv_ctx, qapi_fd_set_t *set)
{

	if (is_drv_cb_valid(qc_drv_net_fd_zero)) {
		return qc_drv_ctx->drv_ops->qc_drv_net_fd_zero(set);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_fd_set(qc_drv_context *qc_drv_ctx, int32_t handle, qapi_fd_set_t *set)
{

	if (is_drv_cb_valid(qc_drv_net_fd_set)) {
		return qc_drv_ctx->drv_ops->qc_drv_net_fd_set(handle, set);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_Buf_Free(qc_drv_context *qc_drv_ctx, void *buf, uint32_t id)
{
	if (is_drv_cb_valid(qc_drv_net_Buf_Free)) {
		return qc_drv_ctx->drv_ops->qc_drv_net_Buf_Free(buf, id);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_fd_clr(qc_drv_context *qc_drv_ctx, int32_t handle, qapi_fd_set_t *set)
{
	if (is_drv_cb_valid(qc_drv_net_fd_clr)) {
		return qc_drv_ctx->drv_ops->qc_drv_net_fd_clr(handle, set);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_fd_isset(qc_drv_context *qc_drv_ctx, int32_t handle, qapi_fd_set_t *set)
{
	if (is_drv_cb_valid(qc_drv_net_fd_isset)) {
		return qc_drv_ctx->drv_ops->qc_drv_net_fd_isset(handle, set);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_SSL_Obj_New(qc_drv_context *qc_drv_ctx, qapi_Net_SSL_Role_t role)
{
	if (is_drv_cb_valid(qc_drv_net_SSL_Obj_New)) {
		return qc_drv_ctx->drv_ops->qc_drv_net_SSL_Obj_New(role);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_SSL_Shutdown(qc_drv_context *qc_drv_ctx, qapi_Net_SSL_Con_Hdl_t ssl)
{
	if (is_drv_cb_valid(qc_drv_net_SSL_Shutdown)) {
		return qc_drv_ctx->drv_ops->qc_drv_net_SSL_Shutdown(ssl);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_SSL_Obj_Free(qc_drv_context *qc_drv_ctx, qapi_Net_SSL_Obj_Hdl_t hdl)
{
	if (is_drv_cb_valid(qc_drv_net_SSL_Obj_Free)) {
		return qc_drv_ctx->drv_ops->qc_drv_net_SSL_Obj_Free(hdl);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_SSL_Cipher_Add(qc_drv_context *qc_drv_ctx, qapi_Net_SSL_Config_t * cfg, uint16_t cipher)
{
	if (is_drv_cb_valid(qc_drv_net_SSL_Cipher_Add)) {
		return qc_drv_ctx->drv_ops->qc_drv_net_SSL_Cipher_Add(cfg, cipher);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_SSL_ALPN_Protocol_Add(qc_drv_context *qc_drv_ctx, qapi_Net_SSL_Obj_Hdl_t hdl, const char *protocol)
{
	if (is_drv_cb_valid(qc_drv_net_SSL_ALPN_Protocol_Add)) {
		return qc_drv_ctx->drv_ops->qc_drv_net_SSL_ALPN_Protocol_Add(hdl, protocol);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_SSL_Con_New(qc_drv_context *qc_drv_ctx, qapi_Net_SSL_Obj_Hdl_t hdl, qapi_Net_SSL_Protocol_t prot)
{
	if (is_drv_cb_valid(qc_drv_net_SSL_Con_New)) {
		return qc_drv_ctx->drv_ops->qc_drv_net_SSL_Con_New(hdl, prot);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_SSL_Configure(qc_drv_context *qc_drv_ctx, qapi_Net_SSL_Con_Hdl_t ssl, qapi_Net_SSL_Config_t *cfg)
{
	if (is_drv_cb_valid(qc_drv_net_SSL_Configure)) {
		return qc_drv_ctx->drv_ops->qc_drv_net_SSL_Configure(ssl, cfg);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_SSL_Cert_Load(qc_drv_context *qc_drv_ctx, qapi_Net_SSL_Obj_Hdl_t hdl, qapi_Net_SSL_Cert_Type_t type, const char * name)
{
	if (is_drv_cb_valid(qc_drv_net_SSL_Cert_Load)) {
		return qc_drv_ctx->drv_ops->qc_drv_net_SSL_Cert_Load(hdl, type, name);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_SSL_Max_Clients_Set(qc_drv_context *qc_drv_ctx, qapi_Net_SSL_Obj_Hdl_t hdl, uint32_t max_Clients)
{
	if (is_drv_cb_valid(qc_drv_net_SSL_Max_Clients_Set)) {
		return qc_drv_ctx->drv_ops->qc_drv_net_SSL_Max_Clients_Set(hdl, max_Clients);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_SSL_Idle_Timeout_Set(qc_drv_context *qc_drv_ctx, qapi_Net_SSL_Obj_Hdl_t hdl, uint32_t idle_Timeout)
{
	if (is_drv_cb_valid(qc_drv_net_SSL_Idle_Timeout_Set)) {
		return qc_drv_ctx->drv_ops->qc_drv_net_SSL_Idle_Timeout_Set(hdl, idle_Timeout);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_SSL_ECJPAKE_Parameters_Set(qc_drv_context *qc_drv_ctx, qapi_Net_SSL_Obj_Hdl_t hdl, qapi_Net_SSL_ECJPAKE_Params_t *cfg)
{
	if (is_drv_cb_valid(qc_drv_net_SSL_ECJPAKE_Parameters_Set)) {
		return qc_drv_ctx->drv_ops->qc_drv_net_SSL_ECJPAKE_Parameters_Set(hdl, cfg);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_SSL_PSK_Table_Set(qc_drv_context *qc_drv_ctx, qapi_Net_SSL_Obj_Hdl_t hdl, qapi_Net_SSL_PSK_t *psk_Entries, uint16_t num_PSK_Entries)
{
	if (is_drv_cb_valid(qc_drv_net_SSL_PSK_Table_Set)) {
		return qc_drv_ctx->drv_ops->qc_drv_net_SSL_PSK_Table_Set(hdl, psk_Entries, num_PSK_Entries);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_SSL_Con_Get_Status(qc_drv_context *qc_drv_ctx, qapi_Net_SSL_Con_Hdl_t ssl)
{
	if (is_drv_cb_valid(qc_drv_net_SSL_Con_Get_Status)) {
		return qc_drv_ctx->drv_ops->qc_drv_net_SSL_Con_Get_Status(ssl);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_SSL_Fd_Set(qc_drv_context *qc_drv_ctx, qapi_Net_SSL_Con_Hdl_t ssl, uint32_t fd)
{
	if (is_drv_cb_valid(qc_drv_net_SSL_Fd_Set)) {
		return qc_drv_ctx->drv_ops->qc_drv_net_SSL_Fd_Set(ssl, fd);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_SSL_Accept(qc_drv_context *qc_drv_ctx, qapi_Net_SSL_Con_Hdl_t ssl)
{
	if (is_drv_cb_valid(qc_drv_net_SSL_Accept)) {
		return qc_drv_ctx->drv_ops->qc_drv_net_SSL_Accept(ssl);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_IPv6_Config_Router_Prefix(qc_drv_context *qc_drv_ctx,
        const char *interface_Name,
        uint8_t *ipv6_Addr,
        uint32_t prefix_Length,
        uint32_t preferred_Lifetime,
        uint32_t valid_Lifetime)
{
	if (is_drv_cb_valid(qc_drv_net_IPv6_Config_Router_Prefix)) {
		return qc_drv_ctx->drv_ops->qc_drv_net_IPv6_Config_Router_Prefix(interface_Name, ipv6_Addr, prefix_Length, preferred_Lifetime, valid_Lifetime);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_IPv6_Route_Add(qc_drv_context *qc_drv_ctx, const char *interface_Name, ip6_addr *dest, uint32_t prefix_Length, ip6_addr *next_Hop)
{
	if (is_drv_cb_valid(qc_drv_net_IPv6_Route_Add)) {
		return qc_drv_ctx->drv_ops->qc_drv_net_IPv6_Route_Add(interface_Name, dest, prefix_Length, next_Hop);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_IPv6_Route_Del(qc_drv_context *qc_drv_ctx, const char *interface_Name, ip6_addr *dest, uint32_t prefix_Length)
{
	if (is_drv_cb_valid(qc_drv_net_IPv6_Route_Del)) {
		return qc_drv_ctx->drv_ops->qc_drv_net_IPv6_Route_Del(interface_Name, dest, prefix_Length);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_IPv6_Routing_Table_Get(qc_drv_context *qc_drv_ctx, qapi_Net_IPv6_Route_t *buf, uint32_t *pcount)
{
	if (is_drv_cb_valid(qc_drv_net_IPv6_Routing_Table_Get)) {
		return qc_drv_ctx->drv_ops->qc_drv_net_IPv6_Routing_Table_Get(buf, pcount);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_Profile_Set_Custom(qc_drv_context *qc_drv_ctx, qapi_Net_Profile_Custom_Pool_t *pNet_buf, uint8_t net_bufq_size)
{
	if (is_drv_cb_valid(qc_drv_net_Profile_Set_Custom)) {
		return qc_drv_ctx->drv_ops->qc_drv_net_Profile_Set_Custom(pNet_buf, net_bufq_size);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_Profile_Set_Active(qc_drv_context *qc_drv_ctx, qapi_Net_Profile_Type_t profile)
{
	if (is_drv_cb_valid(qc_drv_net_Profile_Set_Active)) {
		return qc_drv_ctx->drv_ops->qc_drv_net_Profile_Set_Active(profile);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_Profile_Get_Active(qc_drv_context *qc_drv_ctx, qapi_Net_Profile_Custom_Pool_t **pNet_buf, uint8_t *net_bufq_size, qapi_Net_Profile_Type_t *profile)
{
	if (is_drv_cb_valid(qc_drv_net_Profile_Get_Active)) {
		return qc_drv_ctx->drv_ops->qc_drv_net_Profile_Get_Active(pNet_buf, net_bufq_size, profile);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_OMTM_Switch_Operating_Mode(qc_drv_context *qc_drv_ctx, uint32_t mode_Id, qapi_OMTM_Switch_At_t when)
{
	if (is_drv_cb_valid(qc_drv_net_OMTM_Switch_Operating_Mode)) {
		return qc_drv_ctx->drv_ops->qc_drv_net_OMTM_Switch_Operating_Mode(mode_Id, when);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_DNSs_Is_Started(qc_drv_context *qc_drv_ctx)
{
	if (is_drv_cb_valid(qc_drv_net_DNSs_Is_Started)) {
		return qc_drv_ctx->drv_ops->qc_drv_net_DNSs_Is_Started();
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_DNSs_Get_Host_List(qc_drv_context *qc_drv_ctx, int32_t *n, qapi_Net_DNS_Host_t *hostlist)
{
	if (is_drv_cb_valid(qc_drv_net_DNSs_Get_Host_List)) {
		return qc_drv_ctx->drv_ops->qc_drv_net_DNSs_Get_Host_List(n, hostlist);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_DNSs_Add_Host(qc_drv_context *qc_drv_ctx, const char *host_Name, struct ip46addr *host_Addr, uint32_t ttl)
{
	if (is_drv_cb_valid(qc_drv_net_DNSs_Add_Host)) {
		return qc_drv_ctx->drv_ops->qc_drv_net_DNSs_Add_Host(host_Name, host_Addr, ttl);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_DNSs_Del_Host(qc_drv_context *qc_drv_ctx, const char *hostname)
{
	if (is_drv_cb_valid(qc_drv_net_DNSs_Del_Host)) {
		return qc_drv_ctx->drv_ops->qc_drv_net_DNSs_Del_Host(hostname);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_DNSs_Command(qc_drv_context *qc_drv_ctx, qapi_Net_DNS_Server_Command_t cmd)
{
	if (is_drv_cb_valid(qc_drv_net_DNSs_Command)) {
		return qc_drv_ctx->drv_ops->qc_drv_net_DNSs_Command(cmd);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_mDNS_Command(qc_drv_context *qc_drv_ctx, qapi_Net_mDNS_Command_t cmd, void *input, uint8_t blocking, qapi_Net_mDNS_CB_t app_CB)
{
	if (is_drv_cb_valid(qc_drv_net_mDNS_Command)) {
		return qc_drv_ctx->drv_ops->qc_drv_net_mDNS_Command(cmd, input, blocking, app_CB);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_DNSSD_Start(qc_drv_context *qc_drv_ctx, qapi_Net_DNSSD_Start_t *start)
{
	if (is_drv_cb_valid(qc_drv_net_DNSSD_Start)) {
		return qc_drv_ctx->drv_ops->qc_drv_net_DNSSD_Start(start);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_DNSSD_Init(qc_drv_context *qc_drv_ctx, qapi_Net_DNSSD_Init_t *init)
{
	if (is_drv_cb_valid(qc_drv_net_DNSSD_Init)) {
		return qc_drv_ctx->drv_ops->qc_drv_net_DNSSD_Init(init);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_DNSSD_Stop(qc_drv_context *qc_drv_ctx, qapi_Net_DNSSD_Ctxt_t *ctxt)
{
	if (is_drv_cb_valid(qc_drv_net_DNSSD_Stop)) {
		return qc_drv_ctx->drv_ops->qc_drv_net_DNSSD_Stop(ctxt);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_DNSSD_Discover(qc_drv_context *qc_drv_ctx, const char *svcName)
{
	if (is_drv_cb_valid(qc_drv_net_DNSSD_Discover)) {
		return qc_drv_ctx->drv_ops->qc_drv_net_DNSSD_Discover(svcName);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_DNSSD_Get_Target_Info(qc_drv_context *qc_drv_ctx, const char *svcName)
{
	if (is_drv_cb_valid(qc_drv_net_DNSSD_Get_Target_Info)) {
		return qc_drv_ctx->drv_ops->qc_drv_net_DNSSD_Get_Target_Info(svcName);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_Buf_Free_Queue_Status(qc_drv_context *qc_drv_ctx, qapi_Net_Buf_Queue_Status_t *arg)
{
	if (is_drv_cb_valid(qc_drv_net_Buf_Free_Queue_Status)) {
		return qc_drv_ctx->drv_ops->qc_drv_net_Buf_Free_Queue_Status(arg);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_Get_Socket_Status(qc_drv_context *qc_drv_ctx, qapi_Net_Socket_Status_t *status)
{
	if (is_drv_cb_valid(qc_drv_net_Get_Socket_Status)) {
		return qc_drv_ctx->drv_ops->qc_drv_net_Get_Socket_Status(status);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_IPv4_Route(qc_drv_context *qc_drv_ctx,
        const char *interface_Name,
        qapi_Net_Route_Command_t cmd,
        uint32_t *ipv4_Addr,
        uint32_t *subnet_Mask,
        uint32_t *gateway,
        qapi_Net_IPv4_Route_List_t *route_List)
{
	if (is_drv_cb_valid(qc_drv_net_IPv4_Route)) {
		return qc_drv_ctx->drv_ops->qc_drv_net_IPv4_Route(interface_Name, cmd, ipv4_Addr, subnet_Mask, gateway, route_List);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_HTTPc_Start(qc_drv_context *qc_drv_ctx)
{
	if (is_drv_cb_valid(qc_drv_net_HTTPc_Start)) {
		return qc_drv_ctx->drv_ops->qc_drv_net_HTTPc_Start();
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_HTTPc_Stop(qc_drv_context *qc_drv_ctx)
{
	if (is_drv_cb_valid(qc_drv_net_HTTPc_Stop)) {
		return qc_drv_ctx->drv_ops->qc_drv_net_HTTPc_Stop();
	}
	return QCLI_STATUS_ERROR_E;
}

void* qc_drv_net_HTTPc_New_sess(qc_drv_context *qc_drv_ctx,
        uint32_t timeout,
        qapi_Net_SSL_Obj_Hdl_t ssl_Object_Handle,
        qapi_HTTPc_CB_t callback,
        void* arg,
        uint16_t httpc_Max_Body_Length,
        uint16_t httpc_Max_Header_Length) {
    void *result;
		result = qc_drv_ctx->drv_ops->qc_drv_net_HTTPc_New_sess(timeout, ssl_Object_Handle, callback, arg, httpc_Max_Body_Length, httpc_Max_Header_Length);
        return result;
}

qapi_Status_t qc_drv_net_HTTPc_Configure_SSL(qc_drv_context *qc_drv_ctx, qapi_Net_HTTPc_handle_t handle, qapi_Net_SSL_Config_t *ssl_Cfg)
{
	if (is_drv_cb_valid(qc_drv_net_HTTPc_Configure_SSL)) {
		return qc_drv_ctx->drv_ops->qc_drv_net_HTTPc_Configure_SSL(handle, ssl_Cfg);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_HTTPc_Connect(qc_drv_context *qc_drv_ctx, qapi_Net_HTTPc_handle_t handle, const char *URL, uint16_t port)
{
	if (is_drv_cb_valid(qc_drv_net_HTTPc_Connect)) {
		return qc_drv_ctx->drv_ops->qc_drv_net_HTTPc_Connect(handle, URL, port);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_HTTPc_Free_sess(qc_drv_context *qc_drv_ctx, qapi_Net_HTTPc_handle_t handle)
{
	if (is_drv_cb_valid(qc_drv_net_HTTPc_Free_sess)) {
		return qc_drv_ctx->drv_ops->qc_drv_net_HTTPc_Free_sess(handle);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_HTTPc_Request(qc_drv_context *qc_drv_ctx, qapi_Net_HTTPc_handle_t handle, qapi_Net_HTTPc_Method_e cmd, const char *URL)
{
	if (is_drv_cb_valid(qc_drv_net_HTTPc_Request)) {
		return qc_drv_ctx->drv_ops->qc_drv_net_HTTPc_Request(handle, cmd, URL);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_HTTPc_Set_Body(qc_drv_context *qc_drv_ctx, qapi_Net_HTTPc_handle_t handle, const char *body, uint32_t body_Length)
{
	if (is_drv_cb_valid(qc_drv_net_HTTPc_Set_Body)) {
		return qc_drv_ctx->drv_ops->qc_drv_net_HTTPc_Set_Body(handle, body, body_Length);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_HTTPc_Add_Header_Field(qc_drv_context *qc_drv_ctx, qapi_Net_HTTPc_handle_t handle, const char *type, const char *value)
{
	if (is_drv_cb_valid(qc_drv_net_HTTPc_Add_Header_Field)) {
		return qc_drv_ctx->drv_ops->qc_drv_net_HTTPc_Add_Header_Field(handle, type, value);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_HTTPc_Clear_Header(qc_drv_context *qc_drv_ctx, qapi_Net_HTTPc_handle_t handle)
{
	if (is_drv_cb_valid(qc_drv_net_HTTPc_Clear_Header)) {
		return qc_drv_ctx->drv_ops->qc_drv_net_HTTPc_Clear_Header(handle);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_HTTPc_Set_Param(qc_drv_context *qc_drv_ctx, qapi_Net_HTTPc_handle_t handle, const char *key, const char *value)
{
	if (is_drv_cb_valid(qc_drv_net_HTTPc_Set_Param)) {
		return qc_drv_ctx->drv_ops->qc_drv_net_HTTPc_Set_Param(handle, key, value);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_net_HTTPc_CB_Enable_Adding_Header(qc_drv_context *qc_drv_ctx, qapi_Net_HTTPc_handle_t handle, uint16_t enable)
{
	if (is_drv_cb_valid(qc_drv_net_HTTPc_CB_Enable_Adding_Header)) {
		return qc_drv_ctx->drv_ops->qc_drv_net_HTTPc_CB_Enable_Adding_Header(handle, enable);
	}
	return QCLI_STATUS_ERROR_E;
}

