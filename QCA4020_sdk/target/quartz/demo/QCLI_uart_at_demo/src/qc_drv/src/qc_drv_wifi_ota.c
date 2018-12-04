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

#include  "qc_drv_wifi_ota.h"

qapi_Status_t qc_drv_Fw_Upgrade_Get_Image_ID(qc_drv_context *qc_drv_ctx,
        qapi_Part_Hdl_t hdl, uint32_t *result)
{
	if (is_drv_cb_valid(qc_drv_Fw_Upgrade_Get_Image_ID)) {
		return qc_drv_ctx->drv_ops->qc_drv_Fw_Upgrade_Get_Image_ID(hdl, result);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_Fw_Upgrade_Get_Image_Version(qc_drv_context *qc_drv_ctx,
        qapi_Part_Hdl_t hdl, uint32_t *result)
{
	if (is_drv_cb_valid(qc_drv_Fw_Upgrade_Get_Image_Version)) {
		return qc_drv_ctx->drv_ops->qc_drv_Fw_Upgrade_Get_Image_Version(hdl, result);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_Fw_Upgrade_Get_Partition_Start(qc_drv_context *qc_drv_ctx,
        qapi_Part_Hdl_t hdl, uint32_t *result)
{
	if (is_drv_cb_valid(qc_drv_Fw_Upgrade_Get_Partition_Start)) {
		return qc_drv_ctx->drv_ops->qc_drv_Fw_Upgrade_Get_Partition_Start(hdl, result);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_Fw_Upgrade_Get_Partition_Size(qc_drv_context *qc_drv_ctx,
        qapi_Part_Hdl_t hdl, uint32_t *size)
{
	if (is_drv_cb_valid(qc_drv_Fw_Upgrade_Get_Partition_Size)) {
		return qc_drv_ctx->drv_ops->qc_drv_Fw_Upgrade_Get_Partition_Size(hdl, size);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_Fw_Upgrade_Read_Partition(qc_drv_context *qc_drv_ctx,
        qapi_Part_Hdl_t hdl, int32_t val, char *buf, int32_t buf_size,
        uint32_t *size)
{
	if (is_drv_cb_valid(qc_drv_Fw_Upgrade_Read_Partition)) {
		return qc_drv_ctx->drv_ops->qc_drv_Fw_Upgrade_Read_Partition(hdl, val, buf,
                buf_size, size);
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_Fw_Upgrade_init(qc_drv_context *qc_drv_ctx)
{
	if (is_drv_cb_valid(qc_drv_Fw_Upgrade_init)) {
		return qc_drv_ctx->drv_ops->qc_drv_Fw_Upgrade_init();
	}
	return QCLI_STATUS_ERROR_E;
}

qapi_Status_t qc_drv_Fw_Upgrade_Get_Active_FWD(qc_drv_context *qc_drv_ctx,
        uint32_t *boot_type, uint32_t *fwd_present)
{
	if (is_drv_cb_valid(qc_drv_Fw_Upgrade_Get_Active_FWD)) {
		return qc_drv_ctx->drv_ops->qc_drv_Fw_Upgrade_Get_Active_FWD(boot_type,
                fwd_present);
	}
	return QCLI_STATUS_ERROR_E;
}
qapi_Status_t qc_drv_Fw_Upgrade_Get_FWD_Magic(qc_drv_context *qc_drv_ctx,
        int32_t Index, uint32_t *magic)
{
	if (is_drv_cb_valid(qc_drv_Fw_Upgrade_Get_FWD_Magic)) {
		return qc_drv_ctx->drv_ops->qc_drv_Fw_Upgrade_Get_FWD_Magic(Index, magic);
	}
	return QCLI_STATUS_ERROR_E;
}
qapi_Status_t qc_drv_Fw_Upgrade_Get_FWD_Rank(qc_drv_context *qc_drv_ctx,
        int32_t Index, uint32_t *Result_u32)
{
	if (is_drv_cb_valid(qc_drv_Fw_Upgrade_Get_FWD_Rank)) {
		return qc_drv_ctx->drv_ops->qc_drv_Fw_Upgrade_Get_FWD_Rank(Index,
                Result_u32);
	}
	return QCLI_STATUS_ERROR_E;
}
qapi_Status_t qc_drv_Fw_Upgrade_Get_FWD_Version(qc_drv_context *qc_drv_ctx,
        int32_t Index, uint32_t *Result_u32)
{
	if (is_drv_cb_valid(qc_drv_Fw_Upgrade_Get_FWD_Version)) {
		return qc_drv_ctx->drv_ops->qc_drv_Fw_Upgrade_Get_FWD_Version(Index,
                Result_u32);
	}
	return QCLI_STATUS_ERROR_E;
}
qapi_Status_t qc_drv_Fw_Upgrade_Get_FWD_Status(qc_drv_context *qc_drv_ctx,
        int32_t Index, uint8_t *Result_u8)
{
	if (is_drv_cb_valid(qc_drv_Fw_Upgrade_Get_FWD_Status)) {
		return qc_drv_ctx->drv_ops->qc_drv_Fw_Upgrade_Get_FWD_Status(Index,
                Result_u8);
	}
	return QCLI_STATUS_ERROR_E;
}
qapi_Status_t qc_drv_Fw_Upgrade_Get_FWD_Total_Images(qc_drv_context *qc_drv_ctx,
        int32_t Index, uint8_t *Result_u8)
{
	if (is_drv_cb_valid(qc_drv_Fw_Upgrade_Get_FWD_Total_Images)) {
		return qc_drv_ctx->drv_ops->qc_drv_Fw_Upgrade_Get_FWD_Total_Images(Index,
                Result_u8);
	}
	return QCLI_STATUS_ERROR_E;
}
qapi_Status_t qc_drv_Fw_Upgrade_First_Partition(qc_drv_context *qc_drv_ctx,
        int32_t Index, qapi_Part_Hdl_t *hdl)
{
	if (is_drv_cb_valid(qc_drv_Fw_Upgrade_First_Partition)) {
		return qc_drv_ctx->drv_ops->qc_drv_Fw_Upgrade_First_Partition(Index, hdl);
	}
	return QCLI_STATUS_ERROR_E;
}
qapi_Status_t qc_drv_Fw_Upgrade_Next_Partition(qc_drv_context *qc_drv_ctx,
        qapi_Part_Hdl_t hdl, qapi_Part_Hdl_t *hdl_next)
{
	if (is_drv_cb_valid(qc_drv_Fw_Upgrade_Next_Partition)) {
		return qc_drv_ctx->drv_ops->qc_drv_Fw_Upgrade_Next_Partition(hdl,
                hdl_next);
	}
	return QCLI_STATUS_ERROR_E;
}
qapi_Status_t qc_drv_Fw_Upgrade_Close_Partition(qc_drv_context *qc_drv_ctx,
        qapi_Part_Hdl_t hdl)
{
	if (is_drv_cb_valid(qc_drv_Fw_Upgrade_Close_Partition)) {
		return qc_drv_ctx->drv_ops->qc_drv_Fw_Upgrade_Close_Partition(hdl);
	}
	return QCLI_STATUS_ERROR_E;
}
qapi_Status_t qc_drv_Fw_Upgrade_Erase_FWD(qc_drv_context *qc_drv_ctx,
        int32_t fwd_num)
{
	if (is_drv_cb_valid(qc_drv_Fw_Upgrade_Erase_FWD)) {
		return qc_drv_ctx->drv_ops->qc_drv_Fw_Upgrade_Erase_FWD(fwd_num);
	}
	return QCLI_STATUS_ERROR_E;
}
qapi_Status_t qc_drv_Fw_Upgrade_Done(qc_drv_context *qc_drv_ctx,
        int32_t accept, int32_t flags)
{
	if (is_drv_cb_valid(qc_drv_Fw_Upgrade_Done)) {
		return qc_drv_ctx->drv_ops->qc_drv_Fw_Upgrade_Done(accept, flags);
	}
	return QCLI_STATUS_ERROR_E;
}
qapi_Status_t qc_drv_Fw_Upgrade_Find_Partition(qc_drv_context *qc_drv_ctx,
        uint8_t Index, int32_t img_id, qapi_Part_Hdl_t *hdl)
{
	if (is_drv_cb_valid(qc_drv_Fw_Upgrade_Find_Partition)) {
		return qc_drv_ctx->drv_ops->qc_drv_Fw_Upgrade_Find_Partition(Index,
                img_id, hdl);
	}
	return QCLI_STATUS_ERROR_E;
}
qapi_Status_t qc_drv_Fw_Upgrade(qc_drv_context *qc_drv_ctx, char *iface_name,
        qapi_Fw_Upgrade_Plugin_t *plugin, char *url, char *cfg_file,
        int32_t flags, qapi_Fw_Upgrade_CB_t cb, void *init_param)
{
	if (is_drv_cb_valid(qc_drv_Fw_Upgrade)) {
		return qc_drv_ctx->drv_ops->qc_drv_Fw_Upgrade(iface_name, plugin, url,
                cfg_file, flags, cb, init_param);
	}
	return QCLI_STATUS_ERROR_E;
}

