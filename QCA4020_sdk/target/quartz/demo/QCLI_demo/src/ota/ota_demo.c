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

#include "string.h"
#include "stringl.h"

#include "qapi_types.h"
#include "qapi_status.h"
#include "qapi_firmware_upgrade.h"
#include "qapi_fs.h"
#include "qapi_pmu.h"
#include "qurt_mutex.h"

#include "malloc.h"
#include "qurt_error.h"
#include "qurt_mutex.h"
#include "qurt_signal.h"
#include "qurt_thread.h"
#include "qurt_types.h"
#include "string.h"
#include "pal.h"

#include "qcli.h"
#include "qcli_api.h"
#include "plugins/ftp/ota_ftp.h"
#include "plugins/http/ota_http.h"
#include "plugins/zigbee/ota_zigbee.h"
#include "plugins/ble/ota_ble.h"
#include "spple_demo.h"

/*-------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 *-----------------------------------------------------------------------*/

#define FW_UPGRADE_PRINTF_HANDLE  qcli_fw_upgrade_group

#define BLE_FIRMWARE_VERSION      1

/*-------------------------------------------------------------------------
 * Type Declarations
 *-----------------------------------------------------------------------*/

QCLI_Group_Handle_t qcli_fw_upgrade_group;              /* Handle for our QCLI Command Group. */


static plugin_BLE_Init_t    BLEInitInfo;
static plugin_Zigbee_Init_t ZigbeeInitInfo;

/*-------------------------------------------------------------------------
 * Function Declarations
 *-----------------------------------------------------------------------*/
static QCLI_Command_Status_t Command_Display_FWD(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t Command_Delete_FWD(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t Command_Done_Trial(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t Command_Display_ActiveImage(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t Command_Fw_Upgrade_Cancel(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t Command_Fw_Upgrade_Suspend(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t Command_Fw_Upgrade_Resume(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
#ifdef CONFIG_NET_DEMO
static QCLI_Command_Status_t Command_Fw_Upgrade_FTP_Upgrade(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t Command_Fw_Upgrade_HTTP_Upgrade(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
#endif
#ifdef CONFIG_ZIGBEE_DEMO
static QCLI_Command_Status_t Command_Fw_Upgrade_Zigbee_Upgrade(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
#endif
#ifdef CONFIG_SPPLE_DEMO
static QCLI_Command_Status_t Command_Fw_Upgrade_BLE_Upgrade(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
#endif
/* The following is the complete command list for the Firmware Upgrade demo. */
const QCLI_Command_t Fw_Upgrade_Command_List[] =
{
   /* cmd_function                     thread cmd_string usage_string           description */
   {Command_Display_FWD,               false, "fwd",     "",                    "Display FWD"},
   {Command_Delete_FWD,                false, "del",     "[fwd]",               "Erase FWD"},
   {Command_Done_Trial,                false, "trial",   "[0|1] [reboot flag]", "Accept/Reject Trial FWD"},
   {Command_Display_ActiveImage,       false, "img",     "[id]",                "Display Active FWD Image Info"},
#ifdef CONFIG_NET_DEMO
   {Command_Fw_Upgrade_FTP_Upgrade,    true,  "ftp",     "[ftp info]",          "ftp [if_name] [user]:[pwd]@[[ipv4]|[|ipv6|]:[port] [file name] [flag]\r\n"},
   {Command_Fw_Upgrade_HTTP_Upgrade,   true,  "http",    "",                    "http [if_name] [timeout]:[http_server]:[port] [fw filename]\r\n"},
#endif
#ifdef CONFIG_ZIGBEE_DEMO
   {Command_Fw_Upgrade_Zigbee_Upgrade, true,  "zigbee",  "[Endpoint]",          "Zigbee upgrade\r\n"},
#endif
#ifdef CONFIG_SPPLE_DEMO
   {Command_Fw_Upgrade_BLE_Upgrade,    true,  "ble",     "[BD_ADDR]",           "BLE upgrade\r\n"},
#endif
   {Command_Fw_Upgrade_Cancel,         false, "cancel",  "",                    "cancel fw upgrade\r\n"},
   {Command_Fw_Upgrade_Suspend,        false, "suspend", "",                    "suspend fw upgrade\r\n"},
   {Command_Fw_Upgrade_Resume,         true,  "resume",  "",                    "resume fw upgrade\r\n"},
};

const QCLI_Command_Group_t Fw_Upgrade_Command_Group =
{
    "FwUp",  /* Firmware Upgrade */
    sizeof(Fw_Upgrade_Command_List) / sizeof(QCLI_Command_t),
    Fw_Upgrade_Command_List,
};

/*-------------------------------------------------------------------------
 * Function Definitions
 *-----------------------------------------------------------------------*/

/* This function is used to register the Firmware Upgrade Command Group with QCLI   */
void Initialize_FwUpgrade_Demo(void)
{
   /* Attempt to reqister the Command Groups with the qcli framework.*/
   FW_UPGRADE_PRINTF_HANDLE = QCLI_Register_Command_Group(NULL, &Fw_Upgrade_Command_Group);
   if(FW_UPGRADE_PRINTF_HANDLE)
   {
      QCLI_Printf(FW_UPGRADE_PRINTF_HANDLE, "Firmware Upgrade Registered \n");
   }
}

/*=================================================================================================*/

void  display_sub_image_info(qapi_Part_Hdl_t hdl)
{
    uint32_t result = 0;

    qapi_Fw_Upgrade_Get_Image_ID(hdl, &result);
    QCLI_Printf(FW_UPGRADE_PRINTF_HANDLE, "        Image ID: 0x%X\r\n", result);
    qapi_Fw_Upgrade_Get_Image_Version(hdl, &result);
    QCLI_Printf(FW_UPGRADE_PRINTF_HANDLE, "   Image Version: 0x%X\r\n", result);
    qapi_Fw_Upgrade_Get_Partition_Start(hdl, &result);
    QCLI_Printf(FW_UPGRADE_PRINTF_HANDLE, "     Image Start: 0x%X\r\n", result);
    qapi_Fw_Upgrade_Get_Partition_Size(hdl, &result);
    QCLI_Printf(FW_UPGRADE_PRINTF_HANDLE, "      Image Size: 0x%X\r\n", result);
}

/**
   @brief This function processes the "FWD" command from the CLI.

*/

static QCLI_Command_Status_t Command_Display_FWD(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t Ret_Val;
   int32_t               Index;
   uint32_t              boot_type, fwd_present, magic, Result_u32 = 0;
   uint8_t               Result_u8 = 0;
   qapi_Part_Hdl_t       hdl = NULL, hdl_next;

   {
        if( qapi_Fw_Upgrade_init() != QAPI_OK )
        {
            QCLI_Printf(FW_UPGRADE_PRINTF_HANDLE, "FU Init Error\r\n");
            return QCLI_STATUS_SUCCESS_E;
        }

        //get active FWD
        Index = qapi_Fw_Upgrade_Get_Active_FWD(&boot_type, &fwd_present);
        QCLI_Printf(FW_UPGRADE_PRINTF_HANDLE, "Active FWD: %s  index:%d, present:%d\r\n", (boot_type==QAPI_FW_UPGRADE_FWD_BOOT_TYPE_TRIAL)?"Trial":(boot_type==QAPI_FW_UPGRADE_FWD_BOOT_TYPE_CURRENT)?"Current":"Golden",Index, fwd_present);

        for(Index = 0; Index < 3; Index ++)
        {
            QCLI_Printf(FW_UPGRADE_PRINTF_HANDLE, "FWD %d\r\n", Index);

            qapi_Fw_Upgrade_Get_FWD_Magic(Index, &magic);
            QCLI_Printf(FW_UPGRADE_PRINTF_HANDLE, "Magic: 0x%X\r\n", magic);

            qapi_Fw_Upgrade_Get_FWD_Rank(Index, &Result_u32);
            QCLI_Printf(FW_UPGRADE_PRINTF_HANDLE, "Rank: 0x%X\r\n", Result_u32);

            qapi_Fw_Upgrade_Get_FWD_Version(Index, &Result_u32);
            QCLI_Printf(FW_UPGRADE_PRINTF_HANDLE, "Version: 0x%X\r\n", Result_u32);

            qapi_Fw_Upgrade_Get_FWD_Status(Index, &Result_u8);
            QCLI_Printf(FW_UPGRADE_PRINTF_HANDLE, "Status: 0x%X\r\n", Result_u8);

            qapi_Fw_Upgrade_Get_FWD_Total_Images(Index, &Result_u8);
            QCLI_Printf(FW_UPGRADE_PRINTF_HANDLE, "Total Images: 0x%X\r\n", Result_u8);

            if( magic == 0x54445746 )
            {
                if( qapi_Fw_Upgrade_First_Partition(Index, &hdl) == QAPI_OK )
                {
                    display_sub_image_info(hdl);
                    while (qapi_Fw_Upgrade_Next_Partition(hdl, &hdl_next) == QAPI_OK )
                    {
                        qapi_Fw_Upgrade_Close_Partition(hdl);
                        hdl = hdl_next;
                        display_sub_image_info(hdl);
                    }
                    qapi_Fw_Upgrade_Close_Partition(hdl);
                }
            }

            QCLI_Printf(FW_UPGRADE_PRINTF_HANDLE, "\r\n");

       }

       Ret_Val = QCLI_STATUS_SUCCESS_E;
   }

   return(Ret_Val);
}


/**
   @brief This function processes the "DEL" command from the CLI.

*/
static QCLI_Command_Status_t Command_Delete_FWD(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t Ret_Val;
   int32_t               Index;

   {
        if( Parameter_Count != 1 )
        {
            QCLI_Printf(FW_UPGRADE_PRINTF_HANDLE, "usage: del [fwd num]\r\n");
            goto cmd_del_end;
        }

        if( Parameter_List[0].Integer_Is_Valid == 0 )
        {
            QCLI_Printf(FW_UPGRADE_PRINTF_HANDLE, "usage: del [fwd num]\r\n");
            goto cmd_del_end;
        }

        if( Parameter_List[0].Integer_Value > 2 )
        {
            QCLI_Printf(FW_UPGRADE_PRINTF_HANDLE, "invalid fwd number\r\n");
            goto cmd_del_end;
        }

        Index = Parameter_List[0].Integer_Value;
        QCLI_Printf(FW_UPGRADE_PRINTF_HANDLE, "Delete FWD%d\r\n", Index );

        if( qapi_Fw_Upgrade_init() != QAPI_OK )
        {
            QCLI_Printf(FW_UPGRADE_PRINTF_HANDLE, "\r\nFU Init Error\r\n");
            return QCLI_STATUS_SUCCESS_E;
        }

        if( qapi_Fw_Upgrade_Erase_FWD(Index) != QAPI_OK )
        {
            QCLI_Printf(FW_UPGRADE_PRINTF_HANDLE, "\r\nErase Error\r\n");
        }

       QCLI_Printf(FW_UPGRADE_PRINTF_HANDLE, "done\r\n");

cmd_del_end:
       Ret_Val = QCLI_STATUS_SUCCESS_E;
   }

   return(Ret_Val);
}

/**
   @brief This function processes the "Trial" command from the CLI.

*/
static QCLI_Command_Status_t Command_Done_Trial(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if( (Parameter_Count != 2)  || (Parameter_List[0].Integer_Is_Valid == 0) || (Parameter_List[1].Integer_Is_Valid == 0) )
    {
        return QCLI_STATUS_USAGE_E;
    }

    if( (Parameter_List[0].Integer_Value > 1)  || (Parameter_List[1].Integer_Value > 1) )
    {
        QCLI_Printf(FW_UPGRADE_PRINTF_HANDLE, "Invalid parameter\r\n");
        return QCLI_STATUS_USAGE_E;
    }

    if( Parameter_List[0].Integer_Value == 1 )
    {
        if( qapi_Fw_Upgrade_Done(1, Parameter_List[1].Integer_Value) != QAPI_FW_UPGRADE_OK_E )
        {
            QCLI_Printf(FW_UPGRADE_PRINTF_HANDLE, "Fail to Accept Trial FWD\r\n");
            return QCLI_STATUS_ERROR_E;
        } else {
            QCLI_Printf(FW_UPGRADE_PRINTF_HANDLE, "Success to Accept Trial FWD\r\n");
        }
    } else {
        if( qapi_Fw_Upgrade_Done(0, Parameter_List[1].Integer_Value) != QAPI_FW_UPGRADE_OK_E )
        {
            QCLI_Printf(FW_UPGRADE_PRINTF_HANDLE, "Fail to Reject Trial FWD\r\n");
            return QCLI_STATUS_ERROR_E;
        } else {
            QCLI_Printf(FW_UPGRADE_PRINTF_HANDLE, "Success to Reject Trial FWD\r\n");
        }
    }
    return QCLI_STATUS_SUCCESS_E;
}

/**
 * display image info
 */
static void Display_Image_Info(qapi_Part_Hdl_t hdl)
{
    uint32_t i = 0, id = 0, size = 0, start = 0;

    //get image id
    if( qapi_Fw_Upgrade_Get_Image_ID(hdl, &id) != QAPI_OK )
    {
        QCLI_Printf(FW_UPGRADE_PRINTF_HANDLE, "Failed to get image id\r\n");
        return;
    } else {
        QCLI_Printf(FW_UPGRADE_PRINTF_HANDLE, "Image id is 0x%x(%d)\r\n", id, id);
    }

    //get image version
    if( qapi_Fw_Upgrade_Get_Image_Version(hdl, &id) != QAPI_OK )
    {
        QCLI_Printf(FW_UPGRADE_PRINTF_HANDLE, "failed to get image version\r\n");
        return;
    } else {
        QCLI_Printf(FW_UPGRADE_PRINTF_HANDLE, "Image version is 0x%x(%d)\r\n", id, id);
    }

    //get image start address at flash
    if( qapi_Fw_Upgrade_Get_Partition_Start(hdl, &start) != QAPI_OK )
    {
        QCLI_Printf(FW_UPGRADE_PRINTF_HANDLE, "Failed to get image start address\r\n");
        return;
    } else {
        QCLI_Printf(FW_UPGRADE_PRINTF_HANDLE, "Image start address is 0x%x(%d)\r\n", start, start);
    }

    //get image size
    if( qapi_Fw_Upgrade_Get_Partition_Size(hdl, &size) != QAPI_OK )
    {
        QCLI_Printf(FW_UPGRADE_PRINTF_HANDLE, "Failed to get image size\r\n");
        return;
    } else {
        QCLI_Printf(FW_UPGRADE_PRINTF_HANDLE, "Image size is 0x%x(%d)\r\n", size, size);
    }

    {
    // read image from flash
#define MYBUF_SIZE   32
        char buf[MYBUF_SIZE] = {'\0'};;
        if( qapi_Fw_Upgrade_Read_Partition(hdl, 0, buf, MYBUF_SIZE, &size) != QAPI_OK )
        {
            QCLI_Printf(FW_UPGRADE_PRINTF_HANDLE, "Failed to read image\r\n");
            return;
        } else {
            QCLI_Printf(FW_UPGRADE_PRINTF_HANDLE, "Image:\r\n");
            for(i = 0; i < MYBUF_SIZE; i++)
                QCLI_Printf(FW_UPGRADE_PRINTF_HANDLE, "%02x,", buf[i]);
            QCLI_Printf(FW_UPGRADE_PRINTF_HANDLE, "\r\n");
        }
    }
    return;
}
/**
   @brief This function processes the "Img" command from the CLI.

*/
static QCLI_Command_Status_t Command_Display_ActiveImage(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t Ret_Val;
   uint8_t Index;
   uint32_t id;
   uint32_t boot_type, fwd_present;
   qapi_Part_Hdl_t hdl = NULL, hdl1;

    {
        if( qapi_Fw_Upgrade_init() != QAPI_OK )
        {
            QCLI_Printf(FW_UPGRADE_PRINTF_HANDLE, "FU Init Error\r\n");
            return QCLI_STATUS_SUCCESS_E;
        }

        if( Parameter_Count != 1 )
        {
            QCLI_Printf(FW_UPGRADE_PRINTF_HANDLE, "usage: img [id]\r\n");
            goto cmd_img_end;
        }

        if( Parameter_List[0].Integer_Is_Valid == 0 )
        {
            QCLI_Printf(FW_UPGRADE_PRINTF_HANDLE, "usage: img [id]\r\n");
            goto cmd_img_end;
        }

        id = Parameter_List[0].Integer_Value;

        //get active FWD
        Index = qapi_Fw_Upgrade_Get_Active_FWD(&boot_type, &fwd_present);
        QCLI_Printf(FW_UPGRADE_PRINTF_HANDLE, "Active FWD: %s  index:%d, present:%d\r\n", (boot_type==QAPI_FW_UPGRADE_FWD_BOOT_TYPE_TRIAL)?"Trial":(boot_type==QAPI_FW_UPGRADE_FWD_BOOT_TYPE_CURRENT)?"Current":"Golden",Index, fwd_present);

        if( id != 0 )
        {
            //get hdl based on img id
            if( qapi_Fw_Upgrade_Find_Partition(Index, id, &hdl) != QAPI_OK )
            {
                QCLI_Printf(FW_UPGRADE_PRINTF_HANDLE, "fail to locate the image with id %d\r\n", id);
                goto cmd_img_end;
            }
            Display_Image_Info(hdl);
        } else {
            //get and display all images at current FWD
            if( qapi_Fw_Upgrade_First_Partition(Index, &hdl) != QAPI_OK )
            {
                QCLI_Printf(FW_UPGRADE_PRINTF_HANDLE, "fail to locate the first image\r\n");
                goto cmd_img_end;
            }

            Display_Image_Info(hdl);
            while(qapi_Fw_Upgrade_Next_Partition(hdl, &hdl1) ==QAPI_OK )
            {
                qapi_Fw_Upgrade_Close_Partition(hdl);
                hdl = hdl1;
                Display_Image_Info(hdl);
            }
            qapi_Fw_Upgrade_Close_Partition(hdl);
            hdl = NULL;
        }
cmd_img_end:
       Ret_Val = QCLI_STATUS_SUCCESS_E;
   }

   if( hdl != NULL )
       qapi_Fw_Upgrade_Close_Partition(hdl);

   return(Ret_Val);
}

/****************************************************************************************/

void fw_upgrade_callback(int32_t state, uint32_t status)
{
    QCLI_Printf(FW_UPGRADE_PRINTF_HANDLE, "%d %d\r\n", state, status);
    return;
}

static QCLI_Command_Status_t Command_Fw_Upgrade_Cancel(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	qapi_Fw_Upgrade_Status_Code_t resp_code;
	resp_code = qapi_Fw_Upgrade_Cancel();

    if(QAPI_FW_UPGRADE_OK_E != resp_code)
    {
        QCLI_Printf(FW_UPGRADE_PRINTF_HANDLE, "Fail to cancel Firmware Upgrade Download: ERR:%d\r\n",resp_code);
    } else {
        QCLI_Printf(FW_UPGRADE_PRINTF_HANDLE, "OK to cancel Firmware Upgrade Download\r\n");
    }

    return QCLI_STATUS_SUCCESS_E;
}

static QCLI_Command_Status_t Command_Fw_Upgrade_Suspend(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	qapi_Fw_Upgrade_Status_Code_t resp_code;
	resp_code = qapi_Fw_Upgrade_Suspend();

    if(QAPI_FW_UPGRADE_OK_E != resp_code)
    {
        QCLI_Printf(FW_UPGRADE_PRINTF_HANDLE, "Fail to suspend Firmware Upgrade Download: ERR:%d\r\n",resp_code);
    } else {
        QCLI_Printf(FW_UPGRADE_PRINTF_HANDLE, "OK to suspend Firmware Upgrade Download\r\n");
    }
    return QCLI_STATUS_SUCCESS_E;
}

static QCLI_Command_Status_t Command_Fw_Upgrade_Resume(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	qapi_Fw_Upgrade_Status_Code_t resp_code;
	resp_code = qapi_Fw_Upgrade_Resume();

    if(QAPI_FW_UPGRADE_OK_E != resp_code)
    {
        QCLI_Printf(FW_UPGRADE_PRINTF_HANDLE, "Fail to resume Firmware Upgrade Download: ERR:%d\r\n",resp_code);
    } else {
        QCLI_Printf(FW_UPGRADE_PRINTF_HANDLE, "Firmware Upgrade Image Download Completed successfully\r\n");
    }
    return QCLI_STATUS_SUCCESS_E;
}

#ifdef CONFIG_NET_DEMO

/**
   @brief This function processes the "FTP Firmware Upgrade" command from the CLI.

*/
static QCLI_Command_Status_t Command_Fw_Upgrade_FTP_Upgrade(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    qapi_Fw_Upgrade_Status_Code_t resp_code;
    qapi_Fw_Upgrade_Plugin_t plugin = { plugin_Ftp_Init,
                                plugin_Ftp_Recv_Data,
                                plugin_Ftp_Abort,
								plugin_Ftp_Resume,
                                plugin_Ftp_Fin};

    if( (Parameter_Count != 4) || (Parameter_List[3].Integer_Is_Valid == 0) )
    {
        return QCLI_STATUS_USAGE_E;
    }

    resp_code = qapi_Fw_Upgrade(Parameter_List[0].String_Value, &plugin, Parameter_List[1].String_Value, Parameter_List[2].String_Value, Parameter_List[3].Integer_Value, fw_upgrade_callback, NULL );

    if(QAPI_FW_UPGRADE_OK_E != resp_code)
    {
        QCLI_Printf(FW_UPGRADE_PRINTF_HANDLE, "Firmware Upgrade Image Download Failed ERR:%d\r\n",resp_code);
        if( resp_code == QAPI_FW_UPGRADE_ERR_TRIAL_IS_RUNNING_E )
        {
            QCLI_Printf(FW_UPGRADE_PRINTF_HANDLE, "Trial Partition is running, need reboot to do Firmware Upgrade.\r\n");
        }
    } else {
        QCLI_Printf(FW_UPGRADE_PRINTF_HANDLE, "Firmware Upgrade Image Download Completed successfully\r\n");
    }

   return QCLI_STATUS_SUCCESS_E;
}

/**
   @brief This function processes the "HTTP Firmware Upgrade" command from the CLI.

*/
static QCLI_Command_Status_t Command_Fw_Upgrade_HTTP_Upgrade(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	uint32_t  flags;

    qapi_Fw_Upgrade_Status_Code_t resp_code;
    qapi_Fw_Upgrade_Plugin_t plugin = { plugin_Http_Init,
                                plugin_Http_Recv_Data,
                                plugin_Http_Abort,
								plugin_Http_Resume,
                                plugin_Http_Fin};

    if (Parameter_Count != 3)
    {
        return QCLI_STATUS_USAGE_E;
    }
    flags = QAPI_FW_UPGRADE_FLAG_AUTO_REBOOT;

    resp_code = qapi_Fw_Upgrade(Parameter_List[0].String_Value, &plugin, Parameter_List[1].String_Value, Parameter_List[2].String_Value, flags, fw_upgrade_callback, NULL );

    if(QAPI_FW_UPGRADE_OK_E != resp_code)
    {
        QCLI_Printf(FW_UPGRADE_PRINTF_HANDLE, "Firmware Upgrade Image Download Failed ERR:%d\r\n",resp_code);
        if( resp_code == QAPI_FW_UPGRADE_ERR_TRIAL_IS_RUNNING_E )
        {
            QCLI_Printf(FW_UPGRADE_PRINTF_HANDLE, "Trial Partition is running, need reboot to do Firmware Upgrade.\r\n");
        }
    } else {
        QCLI_Printf(FW_UPGRADE_PRINTF_HANDLE, "Firmware Upgrade Image Download Completed successfully\r\n");
    }

   return QCLI_STATUS_SUCCESS_E;
}

#endif //CONFIG_NET_DEMO

#ifdef CONFIG_ZIGBEE_DEMO
/**
   @brief This function processes the "Zigbee Firmware Upgrade" command from the CLI.

*/
static QCLI_Command_Status_t Command_Fw_Upgrade_Zigbee_Upgrade(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t         Ret_Val;
   qapi_Fw_Upgrade_Status_Code_t resp_code;
   uint32_t                      flags;
   qapi_Fw_Upgrade_Plugin_t      plugin = {plugin_Zigbee_Init,
                                           plugin_Zigbee_Recv_Data,
                                           plugin_Zigbee_Abort,
                                           plugin_Zigbee_Resume,
                                           plugin_Zigbee_Finish};

   flags = QAPI_FW_UPGRADE_FLAG_AUTO_REBOOT;

   if(Parameter_Count >= 1)
   {
      ZigbeeInitInfo.Endpoint = Parameter_List[0].Integer_Value;

      /* Start the FW upgrade. */
      resp_code = qapi_Fw_Upgrade("Zigbee", &plugin, "", "ImgConfig.bin", flags, fw_upgrade_callback, &ZigbeeInitInfo);

      if(QAPI_FW_UPGRADE_OK_E != resp_code)
      {
         /* Handle FW upgrade errors. */
         QCLI_Printf(FW_UPGRADE_PRINTF_HANDLE, "Firmware Upgrade Image Download Failed ERR:%d\r\n", resp_code);

         if( resp_code == QAPI_FW_UPGRADE_ERR_TRIAL_IS_RUNNING_E )
         {
            QCLI_Printf(FW_UPGRADE_PRINTF_HANDLE, "Trial Partition is running, need reboot to do Firmware Upgrade.\r\n");
         }

         Ret_Val = QCLI_STATUS_ERROR_E;
      }
      else
      {
         /* Handle upgrade success. */
         QCLI_Printf(FW_UPGRADE_PRINTF_HANDLE, "Firmware Upgrade Image Download Completed successfully\r\n");
         Ret_Val = QCLI_STATUS_SUCCESS_E;
      }
   }
   else
      Ret_Val = QCLI_STATUS_USAGE_E;

   return Ret_Val;
}
#endif  //CONFIG_ZIGBEE_DEMO

#ifdef CONFIG_SPPLE_DEMO

/**
   @brief This function processes the "BLE Firmware Upgrade" command from the CLI.

*/
static QCLI_Command_Status_t Command_Fw_Upgrade_BLE_Upgrade(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t         Ret_Val;
   qapi_Fw_Upgrade_Status_Code_t resp_code;
   uint32_t                      flags;
   qapi_Fw_Upgrade_Plugin_t      plugin = {plugin_BLE_Init,
                                           plugin_BLE_Recv_Data,
                                           plugin_BLE_Abort,
                                           plugin_BLE_Resume,
                                           plugin_BLE_Finish};

   flags = QAPI_FW_UPGRADE_FLAG_AUTO_REBOOT;

   if((Parameter_Count >= 1) && (strlen((char *)(Parameter_List[0].String_Value)) == (sizeof(qapi_BLE_BD_ADDR_t) * 2)))
   {
      /* Get the Bluetooth address. */
      StrToBD_ADDR(Parameter_List[0].String_Value, &BLEInitInfo.RemoteDevice);

      BLEInitInfo.BluetoothStackID = GetBluetoothStackID();
      BLEInitInfo.Version          = BLE_FIRMWARE_VERSION;
      BLEInitInfo.ConnectionID     = GetConnectionID(BLEInitInfo.RemoteDevice);

      /* Start the FW upgrade. */
      resp_code = qapi_Fw_Upgrade("BLE", &plugin, "", "ImgConfig.bin", flags, fw_upgrade_callback, &BLEInitInfo);

      if(QAPI_FW_UPGRADE_OK_E != resp_code)
      {
         /* Handle FW upgrade errors. */
         QCLI_Printf(FW_UPGRADE_PRINTF_HANDLE, "Firmware Upgrade Image Download Failed ERR:%d\r\n", resp_code);

         if( resp_code == QAPI_FW_UPGRADE_ERR_TRIAL_IS_RUNNING_E )
         {
            QCLI_Printf(FW_UPGRADE_PRINTF_HANDLE, "Trial Partition is running, need reboot to do Firmware Upgrade.\r\n");
         }

         Ret_Val = QCLI_STATUS_ERROR_E;
      }
      else
      {
         /* Handle upgrade success. */
         QCLI_Printf(FW_UPGRADE_PRINTF_HANDLE, "Firmware Upgrade Image Download Completed successfully\r\n");
         Ret_Val = QCLI_STATUS_SUCCESS_E;
      }
   }
   else
      Ret_Val = QCLI_STATUS_USAGE_E;

   return Ret_Val;
}

#endif //CONFIG_SPPLE_DEMO
