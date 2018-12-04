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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "qapi/qapi_types.h"
#include "qapi/qapi_status.h"
#include "qapi/qapi_firmware_upgrade.h"
#include "malloc.h"
#include "ota_ble.h"
#include "qurt_error.h"
#include "qurt_thread.h"
#include "qurt_signal.h"
#include "qurt_mutex.h"

#include "qcli_api.h"

#include "ble_ota_service.h"

extern QCLI_Group_Handle_t qcli_fw_upgrade_group;

#undef DEBUG_FW_UPGRADE_BLE_DEBUG_PRINTF
#if defined(DEBUG_FW_UPGRADE_BLE_DEBUG_PRINTF)
#define FW_UPGRADE_BLE_DEBUG_PRINTF(args...)    QCLI_Printf(qcli_fw_upgrade_group, args)
#else
#define FW_UPGRADE_BLE_DEBUG_PRINTF(args...)
#endif
#define FW_UPGRADE_BLE_PRINTF(args...)          QCLI_Printf(qcli_fw_upgrade_group, args)

/*****************************************************************************************************************************/
/*                                                                                                                           */
/*       Firmware_Upgrade_BLE                                                                                                */
/*                                                                                                                           */
/*****************************************************************************************************************************/
typedef enum BLE_OTA_Plugin_State_s
{
   BLE_OTA_PLUGIN_STATE_DEINIT_E,
   BLE_OTA_PLUGIN_STATE_RUNNING_E,
   BLE_OTA_PLUGIN_STATE_PAUSED_E,
} BLE_OTA_Plugin_State_t;

typedef struct BLE_OTA_Plugin_Context_s
{
   BLE_OTA_Plugin_State_t  State;
   qapi_BLE_BD_ADDR_t      RemoteDevice;
   uint32_t                ConnectionID;
   uint32_t                FWImageSize;
   uint32_t                BluetoothStackID;
   uint32_t                FWImageOffset;
   uint32_t                CurrImageID;
} BLE_OTA_Plugin_Context_t;

static BLE_OTA_Plugin_Context_t BLE_OTA_Ctxt;

/*
   Defines plugin parameters.
*/
#define BLE_OTA_PLUGIN_TIMEOUT                     5000

/*
   Declare static functions.
*/
static void BLE_OTA_Plugin_Deinit(void);

/*
   Deinitializes the BLE OTA plugin and resets state.
*/
static void BLE_OTA_Plugin_Deinit(void)
{
   if(BLE_OTA_Ctxt.State != BLE_OTA_PLUGIN_STATE_DEINIT_E)
   {
      BLE_OTA_Cleanup_Client(BLE_OTA_Ctxt.BluetoothStackID);
      BLE_OTA_Ctxt.State = BLE_OTA_PLUGIN_STATE_DEINIT_E;
   }
}

/*
   Initializes the BLE OTA plugin by creating the cluster and
   starting the image transfer.
*/
qapi_Fw_Upgrade_Status_Code_t plugin_BLE_Init(const char* interface_name, const char *url, void *init_param)
{
   qapi_Fw_Upgrade_Status_Code_t  RetVal = QAPI_FW_UPGRADE_ERROR_E;
   plugin_BLE_Init_t             *InitInfo;
   uint8_t                        Result;
   uint32_t                       Version;

   InitInfo = (plugin_BLE_Init_t *)init_param;

   FW_UPGRADE_BLE_DEBUG_PRINTF("Initializing BLE OTA plugin.\r\n");

   if((url) && (InitInfo))
   {
      /* Make sure the plugin is not already running. */
      if(BLE_OTA_Ctxt.State == BLE_OTA_PLUGIN_STATE_DEINIT_E)
      {
         if((Result = BLE_OTA_Initialize_Client(InitInfo->BluetoothStackID)) == BLE_OTA_STATUS_SUCCESS)
         {
            if((Result = BLE_OTA_Discover_OTA_Service(InitInfo->BluetoothStackID, InitInfo->ConnectionID)) == BLE_OTA_STATUS_SUCCESS)
            {
               /* Use a local variable for the queried version number rather than overwriting the initialization data. */
               Version = InitInfo->Version;

               if((Result = BLE_OTA_Query_Image(InitInfo->BluetoothStackID, InitInfo->ConnectionID, url, &Version, &BLE_OTA_Ctxt.FWImageSize, &BLE_OTA_Ctxt.CurrImageID)) == BLE_OTA_STATUS_SUCCESS)
               {
                  BLE_OTA_Ctxt.State            = BLE_OTA_PLUGIN_STATE_RUNNING_E;
                  BLE_OTA_Ctxt.BluetoothStackID = InitInfo->BluetoothStackID;
                  BLE_OTA_Ctxt.ConnectionID     = InitInfo->ConnectionID;
                  BLE_OTA_Ctxt.FWImageOffset    = 0;
                  BLE_OTA_Ctxt.RemoteDevice     = InitInfo->RemoteDevice;
                  RetVal                        = QAPI_FW_UPGRADE_OK_E;
                  FW_UPGRADE_BLE_DEBUG_PRINTF("plugin_BLE_Init() success, transfer started.\r\n");
               }
               else
               {
                  FW_UPGRADE_BLE_DEBUG_PRINTF("BLE_OTA_Check_Update_Available() returned %d, no image found.\r\n", Result);
               }
            }
            else
            {
               FW_UPGRADE_BLE_DEBUG_PRINTF("BLE_OTA_Discover_OTA_Service() returned %d, no service discovered.\r\n", Result);
            }
         }
         else
         {
            FW_UPGRADE_BLE_DEBUG_PRINTF("BLE_OTA_Initialize_Client() returned %d.\r\n", Result);
         }
      }
      else
      {
         FW_UPGRADE_BLE_DEBUG_PRINTF("plugin_BLE_Init() error, already initialized.\r\n");
      }
   }
   else
   {
      FW_UPGRADE_BLE_DEBUG_PRINTF("plugin_BLE_Init() error, NULL parameter.\r\n");
   }

   return RetVal;
}

/*
   Handles completion of the upgrade. There is no operation needed from
   a BLE protocol standpoint, just need to clean up this module.
*/
qapi_Fw_Upgrade_Status_Code_t plugin_BLE_Finish(void)
{
   /* Clean up the module. */
   BLE_OTA_Plugin_Deinit();
   FW_UPGRADE_BLE_DEBUG_PRINTF("plugin_BLE_Finish() success.\r\n");

   return QAPI_FW_UPGRADE_OK_E;
}

/*
   Handles image data. The BLE stack already handles retrieving
   this data and provides it in callbacks. This function should copy
   from the buffer and wait for more data if necessary.
*/
qapi_Fw_Upgrade_Status_Code_t plugin_BLE_Recv_Data(uint8_t *buffer, uint32_t buf_len, uint32_t *ret_size)
{
   qapi_Fw_Upgrade_Status_Code_t RetVal = QAPI_FW_UPGRADE_ERROR_E;
   uint8_t                       Result;

   FW_UPGRADE_BLE_DEBUG_PRINTF("plugin_BLE_Recv_Data(), buf_len %d.\r\n", buf_len);

   /* Check the input parameters. */
   if((buffer) && (buf_len) && (buffer))
   {
      /* Make sure the plugin is running. */
      if(BLE_OTA_Ctxt.State == BLE_OTA_PLUGIN_STATE_RUNNING_E)
      {
         if(buf_len > (BLE_OTA_Ctxt.FWImageSize - BLE_OTA_Ctxt.FWImageOffset))
            *ret_size = BLE_OTA_Ctxt.FWImageSize - BLE_OTA_Ctxt.FWImageOffset;
         else
            *ret_size = buf_len;

         if((Result = BLE_OTA_Read_Image_Data(BLE_OTA_Ctxt.BluetoothStackID, BLE_OTA_Ctxt.ConnectionID, BLE_OTA_Ctxt.CurrImageID, buffer, ret_size, BLE_OTA_Ctxt.FWImageOffset)) == BLE_OTA_STATUS_SUCCESS)
         {
            BLE_OTA_Ctxt.FWImageOffset += *ret_size;
            RetVal = QAPI_FW_UPGRADE_OK_E;
            FW_UPGRADE_BLE_PRINTF("BLE OTA plugin: read %d bytes, %d bytes remaining.\r\n", *ret_size, (BLE_OTA_Ctxt.FWImageSize - BLE_OTA_Ctxt.FWImageOffset));
         }
         else
         {
            FW_UPGRADE_BLE_DEBUG_PRINTF("plugin_BLE_Recv_Data() error, BLE_OTA_Read_Image_Data() failed, %d.\r\n", Result);
         }
      }
      else
      {
         FW_UPGRADE_BLE_DEBUG_PRINTF("plugin_BLE_Recv_Data() error, plugin not running.\r\n");
      }
   }
   else
   {
      FW_UPGRADE_BLE_DEBUG_PRINTF("plugin_BLE_Recv_Data() error, invalid parameter.\r\n");
   }

   return RetVal;
}

/*
   Handles aborting the upgrade. Since the BLE stack automatically
   polls for data, we will just need to flag to discard any additional
   image data received after this.
*/
qapi_Fw_Upgrade_Status_Code_t plugin_BLE_Abort(void)
{
   /* Clean up the module. */
   BLE_OTA_Plugin_Deinit();

   return QAPI_FW_UPGRADE_OK_E;
}

/*
   Handles resuming the upgrade. There is no mechanism to pause the
   upgrade with the BLE plugin, so no logic is needed.
*/
qapi_Fw_Upgrade_Status_Code_t plugin_BLE_Resume(const char* interface_name, const char *url, uint32_t offset)
{
   /* No action needed. */
   return QAPI_FW_UPGRADE_OK_E;
}
