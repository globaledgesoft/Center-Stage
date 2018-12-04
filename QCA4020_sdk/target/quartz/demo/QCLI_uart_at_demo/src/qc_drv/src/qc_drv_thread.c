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

#include  "qc_drv_thread.h"

qapi_Status_t qc_drv_TWN_Initialize(qc_drv_context *qc_drv_ctx, qapi_TWN_Handle_t *TWN_Handle, qapi_TWN_Event_CB_t TWN_Event_CB, uint32_t CB_Param)
{
	if (is_drv_cb_valid(qc_drv_TWN_Initialize)) {
		return qc_drv_ctx->drv_ops->qc_drv_TWN_Initialize(TWN_Handle, TWN_Event_CB, CB_Param);
	}
	return QCLI_STATUS_ERROR_E;
}

void qc_drv_TWN_Shutdown(qc_drv_context *qc_drv_ctx, qapi_TWN_Handle_t TWN_Handle)
{
	if (is_drv_cb_valid(qc_drv_TWN_Shutdown)) {
	    qc_drv_ctx->drv_ops->qc_drv_TWN_Shutdown(TWN_Handle);
	}
}

qapi_Status_t qc_drv_TWN_Start(qc_drv_context *qc_drv_ctx, qapi_TWN_Handle_t TWN_Handle)
{
	if (is_drv_cb_valid(qc_drv_TWN_Start)) {
		return qc_drv_ctx->drv_ops->qc_drv_TWN_Start(TWN_Handle);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_TWN_Stop(qc_drv_context *qc_drv_ctx, qapi_TWN_Handle_t TWN_Handle)
{
	if (is_drv_cb_valid(qc_drv_TWN_Stop)) {
		return qc_drv_ctx->drv_ops->qc_drv_TWN_Stop(TWN_Handle);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_TWN_Get_Device_Configuration(qc_drv_context *qc_drv_ctx, qapi_TWN_Handle_t TWN_Handle, qapi_TWN_Device_Configuration_t *Configuration)
{
	if (is_drv_cb_valid(qc_drv_TWN_Get_Device_Configuration)) {
		return qc_drv_ctx->drv_ops->qc_drv_TWN_Get_Device_Configuration(TWN_Handle, Configuration);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_TWN_Set_Device_Configuration(qc_drv_context *qc_drv_ctx, qapi_TWN_Handle_t TWN_Handle, const qapi_TWN_Device_Configuration_t *Configuration)
{
	if (is_drv_cb_valid(qc_drv_TWN_Set_Device_Configuration)) {
		return qc_drv_ctx->drv_ops->qc_drv_TWN_Set_Device_Configuration(TWN_Handle, Configuration);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_TWN_Get_Network_Configuration(qc_drv_context *qc_drv_ctx, qapi_TWN_Handle_t TWN_Handle, qapi_TWN_Network_Configuration_t *Configuration)
{
	if (is_drv_cb_valid(qc_drv_TWN_Get_Network_Configuration)) {
		return qc_drv_ctx->drv_ops->qc_drv_TWN_Get_Network_Configuration(TWN_Handle, Configuration);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_TWN_Set_Network_Configuration(qc_drv_context *qc_drv_ctx, qapi_TWN_Handle_t TWN_Handle, const qapi_TWN_Network_Configuration_t *Configuration)
{
	if (is_drv_cb_valid(qc_drv_TWN_Set_Network_Configuration)) {
		return qc_drv_ctx->drv_ops->qc_drv_TWN_Set_Network_Configuration(TWN_Handle, Configuration);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_TWN_Add_Border_Router(qc_drv_context *qc_drv_ctx, qapi_TWN_Handle_t TWN_Handle, const qapi_TWN_Border_Router_t *Border_Router)
{
	if (is_drv_cb_valid(qc_drv_TWN_Add_Border_Router)) {
		return qc_drv_ctx->drv_ops->qc_drv_TWN_Add_Border_Router(TWN_Handle, Border_Router);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_TWN_Remove_Border_Router(qc_drv_context *qc_drv_ctx, qapi_TWN_Handle_t TWN_Handle, const qapi_TWN_IPv6_Prefix_t *Prefix)
{
	if (is_drv_cb_valid(qc_drv_TWN_Remove_Border_Router)) {
		return qc_drv_ctx->drv_ops->qc_drv_TWN_Remove_Border_Router(TWN_Handle, Prefix);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_TWN_Add_External_Route(qc_drv_context *qc_drv_ctx, qapi_TWN_Handle_t TWN_Handle, const qapi_TWN_External_Route_t *External_Route)
{
	if (is_drv_cb_valid(qc_drv_TWN_Add_External_Route)) {
		return qc_drv_ctx->drv_ops->qc_drv_TWN_Add_External_Route(TWN_Handle, External_Route);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_TWN_Remove_External_Route(qc_drv_context *qc_drv_ctx, qapi_TWN_Handle_t TWN_Handle, const qapi_TWN_IPv6_Prefix_t *Prefix)
{
	if (is_drv_cb_valid(qc_drv_TWN_Remove_External_Route)) {
		return qc_drv_ctx->drv_ops->qc_drv_TWN_Remove_External_Route(TWN_Handle, Prefix);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_TWN_Register_Server_Data(qc_drv_context *qc_drv_ctx, qapi_TWN_Handle_t TWN_Handle)
{
	if (is_drv_cb_valid(qc_drv_TWN_Register_Server_Data)) {
		return qc_drv_ctx->drv_ops->qc_drv_TWN_Register_Server_Data(TWN_Handle);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_TWN_Set_IP_Stack_Integration(qc_drv_context *qc_drv_ctx, qapi_TWN_Handle_t TWN_Handle, qbool_t Enabled)
{
	if (is_drv_cb_valid(qc_drv_TWN_Set_IP_Stack_Integration)) {
		return qc_drv_ctx->drv_ops->qc_drv_TWN_Set_IP_Stack_Integration(TWN_Handle, Enabled);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_TWN_Commissioner_Start(qc_drv_context *qc_drv_ctx, qapi_TWN_Handle_t TWN_Handle)
{
	if (is_drv_cb_valid(qc_drv_TWN_Commissioner_Start)) {
		return qc_drv_ctx->drv_ops->qc_drv_TWN_Commissioner_Start(TWN_Handle);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_TWN_Commissioner_Stop(qc_drv_context *qc_drv_ctx, qapi_TWN_Handle_t TWN_Handle)
{
	if (is_drv_cb_valid(qc_drv_TWN_Commissioner_Stop)) {
		return qc_drv_ctx->drv_ops->qc_drv_TWN_Commissioner_Stop(TWN_Handle);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_TWN_Commissioner_Add_Joiner(qc_drv_context *qc_drv_ctx, qapi_TWN_Handle_t TWN_Handle, uint64_t Extended_Address, const char *PSKd, uint32_t Timeout)
{
	if (is_drv_cb_valid(qc_drv_TWN_Commissioner_Add_Joiner)) {
		return qc_drv_ctx->drv_ops->qc_drv_TWN_Commissioner_Add_Joiner(TWN_Handle, Extended_Address, PSKd, Timeout);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_TWN_Commissioner_Remove_Joiner(qc_drv_context *qc_drv_ctx, qapi_TWN_Handle_t TWN_Handle, uint64_t Extended_Address)
{
	if (is_drv_cb_valid(qc_drv_TWN_Commissioner_Remove_Joiner)) {
		return qc_drv_ctx->drv_ops->qc_drv_TWN_Commissioner_Remove_Joiner(TWN_Handle, Extended_Address);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_TWN_Commissioner_Set_Provisioning_URL(qc_drv_context *qc_drv_ctx, qapi_TWN_Handle_t TWN_Handle, const char *Provisioning_URL)
{
	if (is_drv_cb_valid(qc_drv_TWN_Commissioner_Set_Provisioning_URL)) {
		return qc_drv_ctx->drv_ops->qc_drv_TWN_Commissioner_Set_Provisioning_URL(TWN_Handle, Provisioning_URL);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_TWN_Commissioner_Generate_PSKc(qc_drv_context *qc_drv_ctx, qapi_TWN_Handle_t TWN_Handle, const char *Passphrase, const char *Network_Name, uint64_t Extended_PAN_ID, uint8_t *PSKc)
{
	if (is_drv_cb_valid(qc_drv_TWN_Commissioner_Generate_PSKc)) {
		return qc_drv_ctx->drv_ops->qc_drv_TWN_Commissioner_Generate_PSKc(TWN_Handle, Passphrase, Network_Name, Extended_PAN_ID, PSKc);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_TWN_Joiner_Start(qc_drv_context *qc_drv_ctx, qapi_TWN_Handle_t TWN_Handle, const qapi_TWN_Joiner_Info_t *Joiner_Info)
{
	if (is_drv_cb_valid(qc_drv_TWN_Joiner_Start)) {
		return qc_drv_ctx->drv_ops->qc_drv_TWN_Joiner_Start(TWN_Handle, Joiner_Info);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_TWN_Joiner_Stop(qc_drv_context *qc_drv_ctx, qapi_TWN_Handle_t TWN_Handle)
{
	if (is_drv_cb_valid(qc_drv_TWN_Joiner_Stop)) {
		return qc_drv_ctx->drv_ops->qc_drv_TWN_Joiner_Stop(TWN_Handle);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_TWN_Set_PSKc(qc_drv_context *qc_drv_ctx, qapi_TWN_Handle_t TWN_Handle, const uint8_t *PSKc)
{
	if (is_drv_cb_valid(qc_drv_TWN_Set_PSKc)) {
		return qc_drv_ctx->drv_ops->qc_drv_TWN_Set_PSKc(TWN_Handle, PSKc);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_TWN_IPv6_Add_Unicast_Address(qc_drv_context *qc_drv_ctx, qapi_TWN_Handle_t TWN_Handle, qapi_TWN_IPv6_Prefix_t *Prefix, qbool_t Preferred)
{
	if (is_drv_cb_valid(qc_drv_TWN_IPv6_Add_Unicast_Address)) {
		return qc_drv_ctx->drv_ops->qc_drv_TWN_IPv6_Add_Unicast_Address(TWN_Handle, Prefix, Preferred);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_TWN_IPv6_Remove_Unicast_Address(qc_drv_context *qc_drv_ctx, qapi_TWN_Handle_t TWN_Handle, qapi_TWN_IPv6_Address_t *Address)
{
	if (is_drv_cb_valid(qc_drv_TWN_IPv6_Remove_Unicast_Address)) {
		return qc_drv_ctx->drv_ops->qc_drv_TWN_IPv6_Remove_Unicast_Address(TWN_Handle, Address);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_TWN_IPv6_Subscribe_Multicast_Address(qc_drv_context *qc_drv_ctx, qapi_TWN_Handle_t TWN_Handle, qapi_TWN_IPv6_Address_t *Address)
{
	if (is_drv_cb_valid(qc_drv_TWN_IPv6_Subscribe_Multicast_Address)) {
		return qc_drv_ctx->drv_ops->qc_drv_TWN_IPv6_Subscribe_Multicast_Address(TWN_Handle, Address);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_TWN_IPv6_Unsubscribe_Multicast_Address(qc_drv_context *qc_drv_ctx, qapi_TWN_Handle_t TWN_Handle, qapi_TWN_IPv6_Address_t *Address)
{
	if (is_drv_cb_valid(qc_drv_TWN_IPv6_Unsubscribe_Multicast_Address)) {
		return qc_drv_ctx->drv_ops->qc_drv_TWN_IPv6_Unsubscribe_Multicast_Address(TWN_Handle, Address);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_TWN_Set_Ping_Response_Enabled(qc_drv_context *qc_drv_ctx, qapi_TWN_Handle_t TWN_Handle, qbool_t Enabled)
{
	if (is_drv_cb_valid(qc_drv_TWN_Set_Ping_Response_Enabled)) {
		return qc_drv_ctx->drv_ops->qc_drv_TWN_Set_Ping_Response_Enabled(TWN_Handle, Enabled);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_TWN_Become_Router(qc_drv_context *qc_drv_ctx, qapi_TWN_Handle_t TWN_Handle)
{
	if (is_drv_cb_valid(qc_drv_TWN_Become_Router)) {
		return qc_drv_ctx->drv_ops->qc_drv_TWN_Become_Router(TWN_Handle);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_TWN_Become_Leader(qc_drv_context *qc_drv_ctx, qapi_TWN_Handle_t TWN_Handle)
{
	if (is_drv_cb_valid(qc_drv_TWN_Become_Leader)) {
		return qc_drv_ctx->drv_ops->qc_drv_TWN_Become_Leader(TWN_Handle);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_TWN_Start_Border_Agent(qc_drv_context *qc_drv_ctx, qapi_TWN_Handle_t TWN_Handle, int AddressFamily, const char *DisplayName, const char *Hostname, const char *Interface)
{
	if (is_drv_cb_valid(qc_drv_TWN_Start_Border_Agent)) {
		return qc_drv_ctx->drv_ops->qc_drv_TWN_Start_Border_Agent(TWN_Handle, AddressFamily, DisplayName, Hostname, Interface);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_TWN_Stop_Border_Agent(qc_drv_context *qc_drv_ctx, qapi_TWN_Handle_t TWN_Handle)
{
	if (is_drv_cb_valid(qc_drv_TWN_Stop_Border_Agent)) {
		return qc_drv_ctx->drv_ops->qc_drv_TWN_Stop_Border_Agent(TWN_Handle);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_TWN_Clear_Persistent_Data(qc_drv_context *qc_drv_ctx, qapi_TWN_Handle_t TWN_Handle)
{
	if (is_drv_cb_valid(qc_drv_TWN_Clear_Persistent_Data)) {
		return qc_drv_ctx->drv_ops->qc_drv_TWN_Clear_Persistent_Data(TWN_Handle);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_TWN_Set_Max_Poll_Period(qc_drv_context *qc_drv_ctx, qapi_TWN_Handle_t TWN_Handle, uint32_t Period)
{
	if (is_drv_cb_valid(qc_drv_TWN_Set_Max_Poll_Period)) {
		return qc_drv_ctx->drv_ops->qc_drv_TWN_Set_Max_Poll_Period(TWN_Handle, Period);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_TWN_Commissioner_Send_Mgmt_Get(qc_drv_context *qc_drv_ctx, qapi_TWN_Handle_t TWN_Handle, const uint8_t *TlvBuffer, uint8_t Length)
{
	if (is_drv_cb_valid(qc_drv_TWN_Commissioner_Send_Mgmt_Get)) {
		return qc_drv_ctx->drv_ops->qc_drv_TWN_Commissioner_Send_Mgmt_Get(TWN_Handle, TlvBuffer, Length);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_TWN_Commissioner_Send_Mgmt_Set(qc_drv_context *qc_drv_ctx, qapi_TWN_Handle_t TWN_Handle, const qapi_TWN_Commissioning_Dataset_t *Dataset, const uint8_t *TlvBuffer, uint8_t Length)
{
	if (is_drv_cb_valid(qc_drv_TWN_Commissioner_Send_Mgmt_Set)) {
		return qc_drv_ctx->drv_ops->qc_drv_TWN_Commissioner_Send_Mgmt_Set(TWN_Handle, Dataset, TlvBuffer, Length);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_TWN_Commissioner_Send_PanId_Query(qc_drv_context *qc_drv_ctx, qapi_TWN_Handle_t TWN_Handle, uint16_t PanId, uint32_t ChannelMask, const qapi_TWN_IPv6_Address_t *Address)
{
	if (is_drv_cb_valid(qc_drv_TWN_Commissioner_Send_PanId_Query)) {
		return qc_drv_ctx->drv_ops->qc_drv_TWN_Commissioner_Send_PanId_Query(TWN_Handle, PanId, ChannelMask, Address);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_TWN_Commissioner_Get_Session_Id(qc_drv_context *qc_drv_ctx, qapi_TWN_Handle_t TWN_Handle, uint16_t *SessionId)
{
	if (is_drv_cb_valid(qc_drv_TWN_Commissioner_Get_Session_Id)) {
		return qc_drv_ctx->drv_ops->qc_drv_TWN_Commissioner_Get_Session_Id(TWN_Handle, SessionId);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_TWN_Commissioner_Send_Mgmt_Active_Get(qc_drv_context *qc_drv_ctx, qapi_TWN_Handle_t TWN_Handle, const qapi_TWN_IPv6_Address_t *Address, const uint8_t *TlvBuffer, uint8_t Length)
{
	if (is_drv_cb_valid(qc_drv_TWN_Commissioner_Send_Mgmt_Active_Get)) {
		return qc_drv_ctx->drv_ops->qc_drv_TWN_Commissioner_Send_Mgmt_Active_Get(TWN_Handle, Address, TlvBuffer, Length);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_TWN_Commissioner_Send_Mgmt_Active_Set(qc_drv_context *qc_drv_ctx, qapi_TWN_Handle_t TWN_Handle, const qapi_TWN_Operational_Dataset_t *Dataset, const uint8_t *TlvBuffer, uint8_t Length)
{
	if (is_drv_cb_valid(qc_drv_TWN_Commissioner_Send_Mgmt_Active_Set)) {
		return qc_drv_ctx->drv_ops->qc_drv_TWN_Commissioner_Send_Mgmt_Active_Set(TWN_Handle, Dataset, TlvBuffer, Length);
	}
	return QCLI_STATUS_ERROR_E;
}

