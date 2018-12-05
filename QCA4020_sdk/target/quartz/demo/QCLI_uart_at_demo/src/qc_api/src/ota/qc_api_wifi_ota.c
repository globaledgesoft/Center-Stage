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
#include "qc_api_wifi_ota.h"
#include "qc_drv_wifi_ota.h"

/*-------------------------------------------------------------------------
 * Static & global Variable Declarations
 *-----------------------------------------------------------------------*/

/*-------------------------------------------------------------------------
 * Function Definitions
 *-----------------------------------------------------------------------*/
void qc_api_fw_upgrade_callback(int32_t state, uint32_t status)
{
    LOG_AT_EVT("EVT_OTA: %d %d\r\n", state, status);
    return;
}

void qc_api_display_sub_image_info(qapi_Part_Hdl_t hdl)
{
    uint32_t result = 0;

    qc_drv_Fw_Upgrade_Get_Image_ID(qc_api_get_qc_drv_context(), hdl, &result);
    LOG_AT("              Image ID     : 0x%X\r\n", result);
    qc_drv_Fw_Upgrade_Get_Image_Version(qc_api_get_qc_drv_context(), hdl, &result);
    LOG_AT("              Image Version: 0x%X\r\n", result);
    qc_drv_Fw_Upgrade_Get_Partition_Start(qc_api_get_qc_drv_context(), hdl, &result);
    LOG_AT("              Image Start  : 0x%X\r\n", result);
    qc_drv_Fw_Upgrade_Get_Partition_Size(qc_api_get_qc_drv_context(), hdl, &result);
    LOG_AT("              Image Size   : 0x%X\r\n", result);
}

/**
 * display image info
 */
static void Display_Image_Info(qapi_Part_Hdl_t hdl)
{
    uint32_t i = 0, id = 0, size = 0, start = 0;

    //get image id
    if( qc_drv_Fw_Upgrade_Get_Image_ID(qc_api_get_qc_drv_context(), hdl, &id) != QAPI_OK )
    {
        LOG_WARN("Failed to get image id\r\n");
        return;
    } else {
        LOG_AT("Image id is 0x%x(%d)\r\n", id, id);
    }

    //get image version
    if( qc_drv_Fw_Upgrade_Get_Image_Version(qc_api_get_qc_drv_context(), hdl, &id) != QAPI_OK )
    {
        LOG_WARN("failed to get image version\r\n");
        return;
    } else {
        LOG_AT("Image version is 0x%x(%d)\r\n", id, id);
    }

    //get image start address at flash
    if( qc_drv_Fw_Upgrade_Get_Partition_Start(qc_api_get_qc_drv_context(), hdl, &start) != QAPI_OK )
    {
        LOG_WARN("Failed to get image start address\r\n");
        return;
    } else {
        LOG_AT("Image start address is 0x%x(%d)\r\n", start, start);
    }

    //get image size
    if( qc_drv_Fw_Upgrade_Get_Partition_Size(qc_api_get_qc_drv_context(), hdl, &size) != QAPI_OK )
    {
        LOG_WARN("Failed to get image size\r\n");
        return;
    } else {
        LOG_AT("Image size is 0x%x(%d)\r\n", size, size);
    }

    // read image from flash
#define MYBUF_SIZE   32
    char buf[MYBUF_SIZE] = {'\0'};;
    if( qc_drv_Fw_Upgrade_Read_Partition(qc_api_get_qc_drv_context(), hdl, 0, buf, MYBUF_SIZE, &size) != QAPI_OK )
    {
        LOG_ERR("Failed to read image\r\n");
        return;
    } else {
        LOG_AT("Image ");
        for(i = 0; i < MYBUF_SIZE; i++) {
            LOG_AT("%02x,", buf[i]);
        }
      LOG_AT("\r\n");
    }

    return;
}

int32_t qc_api_ota_Fwd(void)
{
    QCLI_Command_Status_t Ret_Val;
    int32_t               Index;
    uint32_t              boot_type, fwd_present, magic, Result_u32 = 0;
    uint8_t               Result_u8 = 0;
    qapi_Part_Hdl_t       hdl = NULL, hdl_next;

    if( qc_drv_Fw_Upgrade_init(qc_api_get_qc_drv_context()) != QAPI_OK )
    {
        LOG_ERR("FU Init Error\r\n");
        return QCLI_STATUS_ERROR_E;
    }

    //get active FWD
    Index = qc_drv_Fw_Upgrade_Get_Active_FWD(qc_api_get_qc_drv_context(), &boot_type, &fwd_present);
    LOG_AT("Active FWD: %s  index:%d, present:%d\r\n", (boot_type==QAPI_FW_UPGRADE_FWD_BOOT_TYPE_TRIAL)?"Trial":(boot_type==QAPI_FW_UPGRADE_FWD_BOOT_TYPE_CURRENT)?"Current":"Golden",Index, fwd_present);

    for(Index = 0; Index < 3; Index ++)
    {
        LOG_AT("FWD %d\r\n", Index);

        qc_drv_Fw_Upgrade_Get_FWD_Magic(qc_api_get_qc_drv_context(), Index, &magic);
        LOG_AT("Magic: 0x%X\r\n", magic);

        qc_drv_Fw_Upgrade_Get_FWD_Rank(qc_api_get_qc_drv_context(), Index, &Result_u32);
        LOG_AT("Rank: 0x%X\r\n", Result_u32);

        qc_drv_Fw_Upgrade_Get_FWD_Version(qc_api_get_qc_drv_context(), Index, &Result_u32);
        LOG_AT("Version: 0x%X\r\n", Result_u32);

        qc_drv_Fw_Upgrade_Get_FWD_Status(qc_api_get_qc_drv_context(), Index, &Result_u8);
        LOG_AT("Status: 0x%X\r\n", Result_u8);

        qc_drv_Fw_Upgrade_Get_FWD_Total_Images(qc_api_get_qc_drv_context(), Index, &Result_u8);
        LOG_AT("Total Images: 0x%X\r\n", Result_u8);

        if( magic == 0x54445746 )
        {
            if( qc_drv_Fw_Upgrade_First_Partition(qc_api_get_qc_drv_context(), Index, &hdl) == QAPI_OK )
            {
                qc_api_display_sub_image_info(hdl);
                while (qc_drv_Fw_Upgrade_Next_Partition(qc_api_get_qc_drv_context(), hdl, &hdl_next) == QAPI_OK )
                {
                    qc_drv_Fw_Upgrade_Close_Partition(qc_api_get_qc_drv_context(), hdl);
                    hdl = hdl_next;
                    qc_api_display_sub_image_info(hdl);
                }
                qc_drv_Fw_Upgrade_Close_Partition(qc_api_get_qc_drv_context(), hdl);
            }
        }
    }

    Ret_Val = QCLI_STATUS_SUCCESS_E;

    return(Ret_Val);
}

int32_t qc_api_ota_DeleteFwd(int32_t fwd_num)
{
    LOG_DEBUG("Delete FWD%d\r\n", fwd_num);
    if( qc_drv_Fw_Upgrade_init(qc_api_get_qc_drv_context()) != QAPI_OK )
    {
        LOG_ERR("\r\nFU Init Error\r\n");
        return QCLI_STATUS_ERROR_E;
    }

    if( qc_drv_Fw_Upgrade_Erase_FWD(qc_api_get_qc_drv_context(), fwd_num) != QAPI_OK )
    {
        LOG_ERR("\r\nErase Error\r\n");
    }
    LOG_INFO("done\r\n");

    return QCLI_STATUS_SUCCESS_E;
}

int32_t qc_api_ota_Trial(QCLI_Parameter_t *Parameter_List)
{
    if( Parameter_List[0].Integer_Value == 1 )
    {
        if( qc_drv_Fw_Upgrade_Done(qc_api_get_qc_drv_context(), 1, Parameter_List[1].Integer_Value) != QAPI_FW_UPGRADE_OK_E )
        {
            LOG_ERR("Fail to Accept Trial FWD\r\n");
            return QCLI_STATUS_ERROR_E;
        } else {
            LOG_INFO("Success to Accept Trial FWD\r\n");
        }
    } else {
        if( qc_drv_Fw_Upgrade_Done(qc_api_get_qc_drv_context(), 0, Parameter_List[1].Integer_Value) != QAPI_FW_UPGRADE_OK_E )
        {
            LOG_ERR("Fail to Reject Trial FWD\r\n");
            return QCLI_STATUS_ERROR_E;
        } else {
            LOG_INFO("Success to Reject Trial FWD\r\n");
        }
    }

    return QCLI_STATUS_SUCCESS_E;
}

int32_t qc_api_ota_ActiveImage(int32_t img_id)
{
    uint8_t Index;
    uint32_t boot_type, fwd_present;
    qapi_Part_Hdl_t hdl = NULL, hdl1;

    if( qc_drv_Fw_Upgrade_init(qc_api_get_qc_drv_context()) != QAPI_OK )
    {
        LOG_ERR("FU Init Error\r\n");
        return QCLI_STATUS_ERROR_E;
    }
    //get active FWD
    Index = qc_drv_Fw_Upgrade_Get_Active_FWD(qc_api_get_qc_drv_context(), &boot_type, &fwd_present);
    LOG_AT("Active FWD: %s  index:%d, present:%d\r\n", (boot_type==QAPI_FW_UPGRADE_FWD_BOOT_TYPE_TRIAL)?"Trial":(boot_type==QAPI_FW_UPGRADE_FWD_BOOT_TYPE_CURRENT)?"Current":"Golden",Index, fwd_present);

    if( img_id != 0 )
    {
        //get hdl based on img id
        if( qc_drv_Fw_Upgrade_Find_Partition(qc_api_get_qc_drv_context(), Index, img_id, &hdl) != QAPI_OK )
        {
            LOG_ERR("fail to locate the image with id %d\r\n", img_id);
            return QCLI_STATUS_ERROR_E;
        }
        Display_Image_Info(hdl);
    } else {
        //get and display all images at current FWD
        if( qc_drv_Fw_Upgrade_First_Partition(qc_api_get_qc_drv_context(), Index, &hdl) != QAPI_OK )
        {
            LOG_ERR("fail to locate the first image\r\n");
            return QCLI_STATUS_ERROR_E;
        }

        Display_Image_Info(hdl);
        while(qc_drv_Fw_Upgrade_Next_Partition(qc_api_get_qc_drv_context(), hdl, &hdl1) ==QAPI_OK )
        {
            qc_drv_Fw_Upgrade_Close_Partition(qc_api_get_qc_drv_context(), hdl);
            hdl = hdl1;
            Display_Image_Info(hdl);
        }
        qc_drv_Fw_Upgrade_Close_Partition(qc_api_get_qc_drv_context(), hdl);
        hdl = NULL;
    }

    if( hdl != NULL )
        qc_drv_Fw_Upgrade_Close_Partition(qc_api_get_qc_drv_context(), hdl);

    return QCLI_STATUS_SUCCESS_E;;
}

int32_t qc_api_ota_FtpUpgrade(QCLI_Parameter_t *Parameter_List)
{
    qapi_Fw_Upgrade_Status_Code_t resp_code;
    qapi_Fw_Upgrade_Plugin_t plugin = { plugin_Ftp_Init,
        plugin_Ftp_Recv_Data,
        plugin_Ftp_Abort,
        plugin_Ftp_Resume,
        plugin_Ftp_Fin};

    resp_code = qc_drv_Fw_Upgrade(qc_api_get_qc_drv_context(), Parameter_List[0].String_Value, &plugin, Parameter_List[1].String_Value, Parameter_List[2].String_Value, Parameter_List[3].Integer_Value, qc_api_fw_upgrade_callback, NULL);

    if(QAPI_FW_UPGRADE_OK_E != resp_code)
    {
        LOG_ERR("Firmware Upgrade Image Download Failed ERR:%d\r\n",resp_code);
        if( resp_code == QAPI_FW_UPGRADE_ERR_TRIAL_IS_RUNNING_E )
        {
            LOG_DEBUG("Trial Partition is running, need reboot to do Firmware Upgrade.\r\n");
        }
        return QCLI_STATUS_ERROR_E;
    } else {
        LOG_INFO("Firmware Upgrade Image Download Completed successfully\r\n");
    }

    return QCLI_STATUS_SUCCESS_E;
}

int32_t qc_api_ota_HttpUpgrade(QCLI_Parameter_t *Parameter_List)
{
    uint32_t  flags;

    qapi_Fw_Upgrade_Status_Code_t resp_code;
    qapi_Fw_Upgrade_Plugin_t plugin = { plugin_Http_Init,
        plugin_Http_Recv_Data,
        plugin_Http_Abort,
        plugin_Http_Resume,
        plugin_Http_Fin};


    flags = QAPI_FW_UPGRADE_FLAG_AUTO_REBOOT;

    resp_code = qc_drv_Fw_Upgrade(qc_api_get_qc_drv_context(), Parameter_List[0].String_Value, &plugin, Parameter_List[1].String_Value, Parameter_List[2].String_Value, flags, qc_api_fw_upgrade_callback, NULL );

    if(QAPI_FW_UPGRADE_OK_E != resp_code)
    {
        LOG_ERR("Firmware Upgrade Image Download Failed ERR:%d\r\n", resp_code);
        if( resp_code == QAPI_FW_UPGRADE_ERR_TRIAL_IS_RUNNING_E )
        {
            LOG_DEBUG("Trial Partition is running, need reboot to do Firmware Upgrade.\r\n");
        }
        return QCLI_STATUS_ERROR_E;
    } else {
        LOG_INFO("Firmware Upgrade Image Download Completed successfully\r\n");
    }

    return QCLI_STATUS_SUCCESS_E;
}

