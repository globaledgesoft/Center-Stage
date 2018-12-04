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
#include "stringl.h"
#include "qapi/qapi_types.h"
#include "qapi/qapi_status.h"
#include "qapi/qapi_firmware_upgrade.h"
#include "malloc.h"
#include "ota_zigbee.h"
#include "qurt_error.h"
#include "qurt_thread.h"
#include "qurt_signal.h"
#include "qurt_mutex.h"

#include "qapi_zb.h"
#include "qapi_zb_zdp.h"
#include "qapi_zb_cl.h"
#include "qapi_zb_cl_ota.h"

#include "zigbee_demo.h"
#include "zcl_ota_demo.h"

extern QCLI_Group_Handle_t qcli_fw_upgrade_group;

#undef DEBUG_FW_UPGRADE_ZIGBEE_DEBUG_PRINTF
#if defined(DEBUG_FW_UPGRADE_ZIGBEE_DEBUG_PRINTF)
#define FW_UPGRADE_ZIGBEE_DEBUG_PRINTF(args...)    QCLI_Printf(qcli_fw_upgrade_group, args)
#else
#define FW_UPGRADE_ZIGBEE_DEBUG_PRINTF(args...)
#endif
#define FW_UPGRADE_ZIGBEE_PRINTF(args...)          QCLI_Printf(qcli_fw_upgrade_group, args)

/*****************************************************************************************************************************/
/*                                                                                                                           */
/*       Firmware_Upgrade_Zigbee                                                                                             */
/*                                                                                                                           */
/*****************************************************************************************************************************/
typedef enum Zigbee_OTA_Plugin_State_s
{
   ZIGBEE_OTA_PLUGIN_STATE_DEINIT_E,
   ZIGBEE_OTA_PLUGIN_STATE_RUNNING_E,
   ZIGBEE_OTA_PLUGIN_STATE_PAUSED_E,
} Zigbee_OTA_Plugin_State_t;

typedef struct Zigbee_OTA_Plugin_Context_s
{
   Zigbee_OTA_Plugin_State_t  State;
   qurt_mutex_t               Mutex;
   qurt_signal_t              Signal;
   uint8_t                   *DataBuffer;
   uint16_t                   DataLength;
   uint16_t                   WriteIndex;
   uint16_t                   ReadIndex;
   uint32_t                   FWImageSize;
   uint32_t                   FWImageBytesRemaining;
   uint32_t                   FWImageDownloaded;
} Zigbee_OTA_Plugin_Context_t;

static Zigbee_OTA_Plugin_Context_t ZB_OTA_Ctxt;

/*
   Defines plugin parameters.
*/
#define ZIGBEE_OTA_PLUGIN_TIMEOUT                     5000
#define ZIGBEE_OTA_PLUGIN_BUFFER_SIZE                 256

/*
   Defines event signals.
*/
#define ZIGBEE_OTA_PLUGIN_EVENT_QUERY_IMAGE_SUCCESS   0x00000001
#define ZIGBEE_OTA_PLUGIN_EVENT_QUERY_IMAGE_FAILURE   0x00000002
#define ZIGBEE_OTA_PLUGIN_EVENT_DATA_READ             0x00000004
#define ZIGBEE_OTA_PLUGIN_EVENT_DATA_WRITE            0x00000008

/*
   Declare static functions.
*/
static uint16_t Zigbee_OTA_Plugin_Buffer_Write_Bytes_Available(void);
static uint16_t Zigbee_OTA_Plugin_Buffer_Read_Bytes_Available(void);
static int      Zigbee_OTA_Plugin_Buffer_Read(uint8_t *DstBuffer, uint32_t Length);
static int      Zigbee_OTA_Plugin_Buffer_Write(uint8_t *SrcBuffer, uint16_t Length);

static void     Zigbee_OTA_Plugin_Client_CB(qapi_ZB_Handle_t ZB_Handle, qapi_ZB_Cluster_t Cluster, qapi_ZB_CL_OTA_Client_Event_Data_t *Event_Data, uint32_t CB_Param);
static void     Zigbee_OTA_Plugin_Deinit(void);

/*
   Gets the number of bytes available to be written in the context buffer.

   Returns:    number of bytes available for write
*/
static uint16_t Zigbee_OTA_Plugin_Buffer_Write_Bytes_Available(void)
{
   return ((ZB_OTA_Ctxt.ReadIndex - ZB_OTA_Ctxt.WriteIndex) + ((ZB_OTA_Ctxt.WriteIndex < ZB_OTA_Ctxt.ReadIndex) ? 0 : ZIGBEE_OTA_PLUGIN_BUFFER_SIZE));
}

/*
   Gets the number of bytes available to be read in the context buffer.

   Returns:    number of bytes available for read
*/
static uint16_t Zigbee_OTA_Plugin_Buffer_Read_Bytes_Available(void)
{
   return ((ZB_OTA_Ctxt.WriteIndex - ZB_OTA_Ctxt.ReadIndex + ZB_OTA_Ctxt.DataLength) % ZB_OTA_Ctxt.DataLength);
}

/*
   Reads from the context buffer into the FW upgrade buffer.

   Returns:    -1 upon error
               number of bytes read upon success
*/
static int Zigbee_OTA_Plugin_Buffer_Read(uint8_t *DstBuffer, uint32_t Length)
{
   uint16_t ReadLength;
   int      RetVal;

   if((DstBuffer) && (Length))
   {
      if(ZB_OTA_Ctxt.WriteIndex == ZB_OTA_Ctxt.ReadIndex)
      {
         ReadLength = 0;
      }
      else
      {
         /* Check for buffer wrap. */
         if((ZB_OTA_Ctxt.WriteIndex > ZB_OTA_Ctxt.ReadIndex) || ((ZB_OTA_Ctxt.DataLength - ZB_OTA_Ctxt.ReadIndex) >= Length))
         {
            /* Handle non-wrapping buffer. */
            ReadLength = Zigbee_OTA_Plugin_Buffer_Read_Bytes_Available();
            if(ReadLength > Length)
               ReadLength = Length;
            /* Copy to the buffer. */
            memscpy(DstBuffer, Length, &ZB_OTA_Ctxt.DataBuffer[ZB_OTA_Ctxt.ReadIndex], ReadLength);
         }
         else
         {
            /* Handle buffer wrap. */
            ReadLength = (ZB_OTA_Ctxt.DataLength - ZB_OTA_Ctxt.ReadIndex + ZB_OTA_Ctxt.WriteIndex);
            if(ReadLength > Length)
               ReadLength = Length;

            /* Copy from the end of the context buffer. */
            memscpy(DstBuffer, Length, &ZB_OTA_Ctxt.DataBuffer[ZB_OTA_Ctxt.ReadIndex], (ZB_OTA_Ctxt.DataLength - ZB_OTA_Ctxt.ReadIndex));

            /* Copy from the beginning of the context buffer. */
            memscpy(&DstBuffer[ZB_OTA_Ctxt.DataLength - ZB_OTA_Ctxt.ReadIndex], (Length - (ZB_OTA_Ctxt.DataLength - ZB_OTA_Ctxt.ReadIndex)), ZB_OTA_Ctxt.DataBuffer, (ReadLength - (ZB_OTA_Ctxt.DataLength - ZB_OTA_Ctxt.ReadIndex)));
         }
      }

      /* Set the return value. */
      RetVal = ReadLength;

      /* Increment the read index. */
      ZB_OTA_Ctxt.ReadIndex = (ZB_OTA_Ctxt.ReadIndex + ReadLength) % ZB_OTA_Ctxt.DataLength;
   }
   else
   {
      RetVal = -1;
   }

   return(RetVal);
}

/*
   Writes to the context buffer from the callback buffer. Performs an
   "all or nothing" write.

   Returns:    -1 upon error
                0 upon success
*/
static int Zigbee_OTA_Plugin_Buffer_Write(uint8_t *SrcBuffer, uint16_t Length)
{
   uint16_t WriteLength;
   uint16_t BytesAvailable;
   int      RetVal;

   if((SrcBuffer) && (Length))
   {
      BytesAvailable = Zigbee_OTA_Plugin_Buffer_Write_Bytes_Available();

      /* Copy all or nothing. */
      if(BytesAvailable < Length)
      {
         WriteLength =  0;
         RetVal      = -1;
      }
      else
      {
         /* Check for buffer wrap. */
         if((ZB_OTA_Ctxt.ReadIndex > ZB_OTA_Ctxt.WriteIndex) || ((ZB_OTA_Ctxt.DataLength - ZB_OTA_Ctxt.WriteIndex) >= Length))
         {
            /* Handle non-wrapping buffer. */
            memscpy(&ZB_OTA_Ctxt.DataBuffer[ZB_OTA_Ctxt.WriteIndex], (ZB_OTA_Ctxt.DataLength - ZB_OTA_Ctxt.WriteIndex), SrcBuffer, Length);
         }
         else
         {
            /* Handle buffer wrap - copy to the end of the context buffer first. */
            memscpy(&ZB_OTA_Ctxt.DataBuffer[ZB_OTA_Ctxt.WriteIndex], (ZB_OTA_Ctxt.DataLength - ZB_OTA_Ctxt.WriteIndex), SrcBuffer, Length);
            /* Copy to the beginning of the context buffer. */
            memscpy(ZB_OTA_Ctxt.DataBuffer, ZB_OTA_Ctxt.DataLength, &SrcBuffer[ZB_OTA_Ctxt.DataLength - ZB_OTA_Ctxt.WriteIndex], (Length - (ZB_OTA_Ctxt.DataLength - ZB_OTA_Ctxt.WriteIndex)));
         }

         /* Set the return value. */
         WriteLength = Length;
         RetVal      = 0;
      }

      /* Increment the write index. */
      ZB_OTA_Ctxt.WriteIndex = (ZB_OTA_Ctxt.WriteIndex + WriteLength) % ZB_OTA_Ctxt.DataLength;
   }
   else
   {
      RetVal = -1;
   }

   return(RetVal);
}

/*
   Handles Zigbee OTA cluster client events.
*/
static void Zigbee_OTA_Plugin_Client_CB(qapi_ZB_Handle_t ZB_Handle, qapi_ZB_Cluster_t Cluster, qapi_ZB_CL_OTA_Client_Event_Data_t *Event_Data, uint32_t CB_Param)
{
   uint32_t CurrSignals;
   int      Result = QURT_EOK;
   uint16_t RetVal;

   if(Event_Data)
   {
      switch(Event_Data->Event_Type)
      {
         case QAPI_ZB_CL_OTA_CLIENT_EVENT_TYPE_DEFAULT_RESPONSE_E:
            break;
         case QAPI_ZB_CL_OTA_CLIENT_EVENT_TYPE_COMMAND_COMPLETE_E:
            break;
         case QAPI_ZB_CL_OTA_CLIENT_EVENT_TYPE_UNPARSED_RESPONSE_E:
            break;
         case QAPI_ZB_CL_OTA_CLIENT_EVENT_TYPE_QUERY_NEXT_IMAGE_E:
            if(Event_Data->Data.Query_Next_Image.Status == QAPI_OK)
            {
               FW_UPGRADE_ZIGBEE_DEBUG_PRINTF("QAPI_ZB_CL_OTA_CLIENT_EVENT_TYPE_QUERY_NEXT_IMAGE_E callback success, status %d, size %d, FW image size: %d bytes.\r\n", Event_Data->Data.Query_Next_Image.Status, Event_Data->Data.Query_Next_Image.ImageSize, (Event_Data->Data.Query_Next_Image.ImageSize - ZIGBEE_OTA_DEMO_PREPEND_SIZE));
               ZB_OTA_Ctxt.FWImageSize       = (Event_Data->Data.Query_Next_Image.ImageSize - ZIGBEE_OTA_DEMO_PREPEND_SIZE);
               ZB_OTA_Ctxt.FWImageDownloaded = 0;
               /* Set the signal for image query success. */
               qurt_signal_set(&ZB_OTA_Ctxt.Signal, ZIGBEE_OTA_PLUGIN_EVENT_QUERY_IMAGE_SUCCESS);
            }
            else
            {
               FW_UPGRADE_ZIGBEE_DEBUG_PRINTF("QAPI_ZB_CL_OTA_CLIENT_EVENT_TYPE_QUERY_NEXT_IMAGE_E callback failure, status %d.\r\n", Event_Data->Data.Query_Next_Image.Status);
               /* Set the signal for image query failure. */
               qurt_signal_set(&ZB_OTA_Ctxt.Signal, ZIGBEE_OTA_PLUGIN_EVENT_QUERY_IMAGE_FAILURE);
            }
            break;
         case QAPI_ZB_CL_OTA_CLIENT_EVENT_TYPE_RAW_WRITE_E:
            break;
         case QAPI_ZB_CL_OTA_CLIENT_EVENT_TYPE_WRITE_E:
            /* Make sure the plugin is running. */
            if(ZB_OTA_Ctxt.State == ZIGBEE_OTA_PLUGIN_STATE_RUNNING_E)
            {
               while((Zigbee_OTA_Plugin_Buffer_Write_Bytes_Available() < Event_Data->Data.Write.DataLength) && (Result == QURT_EOK))
               {
                  /* Wait for buffer space if necessary. */
                  Result = qurt_signal_wait_timed(&ZB_OTA_Ctxt.Signal, ZIGBEE_OTA_PLUGIN_EVENT_DATA_READ, QURT_SIGNAL_ATTR_CLEAR_MASK, &CurrSignals, ZIGBEE_OTA_PLUGIN_TIMEOUT);
               }

               /* Write the bytes into the context buffer. */
               if(Result == QURT_EOK)
               {
                  /* Get the context mutex. */
                  if(qurt_mutex_lock_timed(&ZB_OTA_Ctxt.Mutex, ZIGBEE_OTA_PLUGIN_TIMEOUT) == QURT_EOK)
                  {
                     /* Copy data from the buffer. */
                     RetVal = Zigbee_OTA_Plugin_Buffer_Write(Event_Data->Data.Write.Data, Event_Data->Data.Write.DataLength);
                     ZB_OTA_Ctxt.FWImageDownloaded += Event_Data->Data.Write.DataLength;

                     if(RetVal == 0)
                     {
                        FW_UPGRADE_ZIGBEE_DEBUG_PRINTF("QAPI_ZB_CL_OTA_CLIENT_EVENT_TYPE_WRITE_E callback success, wrote %d bytes.\r\n", Event_Data->Data.Write.DataLength);
                        if(ZB_OTA_Ctxt.FWImageDownloaded < ZB_OTA_Ctxt.FWImageSize)
                        {
                           /* Set the signal that data has been written. If this
                              is the last packet, the signal will be set when
                              the upgrade end is received. */
                           qurt_signal_set(&ZB_OTA_Ctxt.Signal, ZIGBEE_OTA_PLUGIN_EVENT_DATA_WRITE);
                        }
                     }
                     else
                     {
                        FW_UPGRADE_ZIGBEE_DEBUG_PRINTF("QAPI_ZB_CL_OTA_CLIENT_EVENT_TYPE_WRITE_E callback error, could not write data to buffer.\r\n");
                     }

                     /* Release the mutex. */
                     qurt_mutex_unlock(&ZB_OTA_Ctxt.Mutex);
                  }
                  else
                  {
                     FW_UPGRADE_ZIGBEE_DEBUG_PRINTF("QAPI_ZB_CL_OTA_CLIENT_EVENT_TYPE_WRITE_E callback error, could not get mutex.\r\n");
                  }
               }
               else
               {
                  FW_UPGRADE_ZIGBEE_DEBUG_PRINTF("QAPI_ZB_CL_OTA_CLIENT_EVENT_TYPE_WRITE_E callback error, no buffer space.\r\n");
               }
            }
            else
            {
               FW_UPGRADE_ZIGBEE_DEBUG_PRINTF("QAPI_ZB_CL_OTA_CLIENT_EVENT_TYPE_WRITE_E callback error, plugin not running.\r\n");
            }
            break;
         case QAPI_ZB_CL_OTA_CLIENT_EVENT_TYPE_VALIDATE_E:
            *Event_Data->Data.Validate.ReturnStatus = QAPI_OK;
            FW_UPGRADE_ZIGBEE_DEBUG_PRINTF("QAPI_ZB_CL_OTA_CLIENT_EVENT_TYPE_VALIDATE_E callback received.\r\n");
            break;
         case QAPI_ZB_CL_OTA_CLIENT_EVENT_TYPE_UPGRADE_E:
            FW_UPGRADE_ZIGBEE_DEBUG_PRINTF("QAPI_ZB_CL_OTA_CLIENT_EVENT_TYPE_UPGRADE_E callback received.\r\n");

            qurt_signal_set(&ZB_OTA_Ctxt.Signal, ZIGBEE_OTA_PLUGIN_EVENT_DATA_WRITE);
            break;
         case QAPI_ZB_CL_OTA_CLIENT_EVENT_TYPE_IMAGE_NOTIFY_E:
            FW_UPGRADE_ZIGBEE_PRINTF("ZigBee OTA client image notify callback received.\n");
            FW_UPGRADE_ZIGBEE_PRINTF("  PayloadType: %d.\n", Event_Data->Data.Image_Notify.PayloadType);
            FW_UPGRADE_ZIGBEE_PRINTF("  Jitter:      %d.\n", Event_Data->Data.Image_Notify.Jitter);

            if (Event_Data->Data.Image_Notify.PayloadType >= QAPI_ZB_CL_OTA_NOTIFY_TYPE_JITTER_MFG_CODE)
            {
               FW_UPGRADE_ZIGBEE_PRINTF("  MfgCode: %d.\n", Event_Data->Data.Image_Notify.ImageDefinition.ManufacturerCode);
            }

            if (Event_Data->Data.Image_Notify.PayloadType >= QAPI_ZB_CL_OTA_NOTIFY_TYPE_JITTER_MFG_CODE_IMAGE_TYPE)
            {
               FW_UPGRADE_ZIGBEE_PRINTF("  ImageType: %d.\n", Event_Data->Data.Image_Notify.ImageDefinition.ImageType);
            }

            if (Event_Data->Data.Image_Notify.PayloadType >= QAPI_ZB_CL_OTA_NOTIFY_TYPE_JITTER_MFG_CODE_IMAGE_TYPE_FILE_VERSION)
            {
               FW_UPGRADE_ZIGBEE_PRINTF("  FileVersion: %d.\n", Event_Data->Data.Image_Notify.ImageDefinition.FileVersion);
            }
            break;

         case QAPI_ZB_CL_OTA_CLIENT_EVENT_TYPE_ABORT_E:
            FW_UPGRADE_ZIGBEE_DEBUG_PRINTF("QAPI_ZB_CL_OTA_CLIENT_EVENT_TYPE_ABORT_E: OTA client abort callback received.\n");
            break;
         default:
            break;
      }
   }
}

/*
   Deinitializes the Zigbee OTA plugin and resets state.
*/
static void Zigbee_OTA_Plugin_Deinit(void)
{
   if(ZB_OTA_Ctxt.State != ZIGBEE_OTA_PLUGIN_STATE_DEINIT_E)
   {
      /* Clean up the signal and mutex. */
      qurt_mutex_destroy(&ZB_OTA_Ctxt.Mutex);
      qurt_signal_destroy(&ZB_OTA_Ctxt.Signal);

      /* Clean up all allocs. */
      if(ZB_OTA_Ctxt.DataBuffer)
      {
         free(ZB_OTA_Ctxt.DataBuffer);
         ZB_OTA_Ctxt.DataBuffer = NULL;
      }

      /* Unregister the callback. */
      ZCL_OTA_Demo_Unregister_Client_Callback();

      ZB_OTA_Ctxt.State = ZIGBEE_OTA_PLUGIN_STATE_DEINIT_E;
   }
}

/*
   Initializes the Zigbee OTA plugin by creating the cluster and
   starting the image transfer.
*/
qapi_Fw_Upgrade_Status_Code_t plugin_Zigbee_Init(const char* interface_name, const char *url, void *init_param)
{
   qapi_Fw_Upgrade_Status_Code_t  RetVal = QAPI_FW_UPGRADE_ERROR_E;
   plugin_Zigbee_Init_t          *InitInfo;
   uint32_t                       CurrSignals;
   int                            Result;

   InitInfo = (plugin_Zigbee_Init_t *)init_param;

   FW_UPGRADE_ZIGBEE_DEBUG_PRINTF("Initializing Zigbee OTA plugin.\r\n");

   if(url)
   {
      /* Make sure the plugin is not already running. */
      if(ZB_OTA_Ctxt.State == ZIGBEE_OTA_PLUGIN_STATE_DEINIT_E)
      {
         /* Create mutex/signal. */
         qurt_mutex_init(&ZB_OTA_Ctxt.Mutex);
         qurt_signal_init(&ZB_OTA_Ctxt.Signal);

         /* Allocate the data buffer. */
         if((ZB_OTA_Ctxt.DataBuffer = (uint8_t *)malloc(ZIGBEE_OTA_PLUGIN_BUFFER_SIZE)) != NULL)
         {
            ZB_OTA_Ctxt.DataLength = ZIGBEE_OTA_PLUGIN_BUFFER_SIZE;

            ZCL_OTA_Demo_Register_Client_Callback(Zigbee_OTA_Plugin_Client_CB, 0);

            if(ZCL_OTA_Demo_Query_Image(InitInfo->Endpoint, url) == true)
            {
               /* Wait for the data callback if needed. */
               Result = qurt_signal_wait_timed(&ZB_OTA_Ctxt.Signal, (ZIGBEE_OTA_PLUGIN_EVENT_QUERY_IMAGE_SUCCESS | ZIGBEE_OTA_PLUGIN_EVENT_QUERY_IMAGE_FAILURE), QURT_SIGNAL_ATTR_CLEAR_MASK, &CurrSignals, ZIGBEE_OTA_PLUGIN_TIMEOUT);

               if((Result == QURT_EOK) && (CurrSignals & ZIGBEE_OTA_PLUGIN_EVENT_QUERY_IMAGE_SUCCESS))
               {
                  /* Set the number of bytes remaining. */
                  ZB_OTA_Ctxt.FWImageBytesRemaining = ZB_OTA_Ctxt.FWImageSize;

                  if(ZCL_OTA_Demo_Start_Transfer(InitInfo->Endpoint) == true)
                  {
                     ZB_OTA_Ctxt.State = ZIGBEE_OTA_PLUGIN_STATE_RUNNING_E;
                     RetVal            = QAPI_FW_UPGRADE_OK_E;
                     FW_UPGRADE_ZIGBEE_DEBUG_PRINTF("plugin_Zigbee_Init() success, transfer started.\r\n");
                  }
                  else
                  {
                     FW_UPGRADE_ZIGBEE_DEBUG_PRINTF("plugin_Zigbee_Init() error, could not start transfer.\r\n");
                  }
               }
               else
               {
                  FW_UPGRADE_ZIGBEE_DEBUG_PRINTF("plugin_Zigbee_Init() error, no image available.\r\n");
               }
            }
            else
            {
               FW_UPGRADE_ZIGBEE_DEBUG_PRINTF("plugin_Zigbee_Init() error, could not query image.\r\n");
            }
         }
         else
         {
            FW_UPGRADE_ZIGBEE_DEBUG_PRINTF("plugin_Zigbee_Init() error, out of resources.\r\n");
         }

         if(RetVal != QAPI_FW_UPGRADE_OK_E)
         {
            Zigbee_OTA_Plugin_Deinit();
         }
      }
      else
      {
         FW_UPGRADE_ZIGBEE_DEBUG_PRINTF("plugin_Zigbee_Init() error, already initialized.\r\n");
      }
   }
   else
   {
      FW_UPGRADE_ZIGBEE_DEBUG_PRINTF("plugin_Zigbee_Init() error, NULL parameter.\r\n");
   }

   return RetVal;
}

/*
   Handles completion of the upgrade. There is no operation needed from
   a Zigbee protocol standpoint, just need to clean up this module.
*/
qapi_Fw_Upgrade_Status_Code_t plugin_Zigbee_Finish(void)
{
   /* Clean up the module. */
   Zigbee_OTA_Plugin_Deinit();
   FW_UPGRADE_ZIGBEE_DEBUG_PRINTF("plugin_Zigbee_Finish() success.\r\n");

   return QAPI_FW_UPGRADE_OK_E;
}

/*
   Handles image data. The Zigbee stack already handles retrieving
   this data and provides it in callbacks. This function should copy
   from the buffer and wait for more data if necessary.
*/
qapi_Fw_Upgrade_Status_Code_t plugin_Zigbee_Recv_Data(uint8_t *buffer, uint32_t buf_len, uint32_t *ret_size)
{
   qapi_Fw_Upgrade_Status_Code_t RetVal = QAPI_FW_UPGRADE_OK_E;
   int                           Result;
   uint32_t                      CurrSignals;

   FW_UPGRADE_ZIGBEE_DEBUG_PRINTF("plugin_Zigbee_Recv_Data(), buf_len %d.\r\n", buf_len);

   /* Check the input parameters. */
   if((buffer) && (buf_len) && (buffer))
   {
      /* Make sure the plugin is running. */
      if(ZB_OTA_Ctxt.State == ZIGBEE_OTA_PLUGIN_STATE_RUNNING_E)
      {
         *ret_size = 0;

         while((RetVal == QAPI_FW_UPGRADE_OK_E) && (buf_len) && (ZB_OTA_Ctxt.FWImageBytesRemaining))
         {
            /* Check if there is data in the context buffer available for read. */
            if(!Zigbee_OTA_Plugin_Buffer_Read_Bytes_Available())
            {
               /* Wait for the data callback if needed. */
               qurt_signal_clear(&ZB_OTA_Ctxt.Signal, ZIGBEE_OTA_PLUGIN_EVENT_DATA_WRITE);
               Result = qurt_signal_wait_timed(&ZB_OTA_Ctxt.Signal, ZIGBEE_OTA_PLUGIN_EVENT_DATA_WRITE, QURT_SIGNAL_ATTR_CLEAR_MASK, &CurrSignals, ZIGBEE_OTA_PLUGIN_TIMEOUT);
               FW_UPGRADE_ZIGBEE_DEBUG_PRINTF("plugin_Zigbee_Recv_Data() wait, qurt_signal_wait_timed() returned %d.\r\n", Result);
            }
            else
            {
               Result = QURT_EOK;
            }

            if(Result == QURT_EOK)
            {
               /* Get the context mutex. */
               if(qurt_mutex_lock_timed(&ZB_OTA_Ctxt.Mutex, ZIGBEE_OTA_PLUGIN_TIMEOUT) == QURT_EOK)
               {
                  /* Read data into the provided buffer. */
                  Result = Zigbee_OTA_Plugin_Buffer_Read(&buffer[*ret_size], buf_len);

                  /* Release the mutex. */
                  qurt_mutex_unlock(&ZB_OTA_Ctxt.Mutex);

                  if(Result > 0)
                  {
                     /* Update variables. */
                     *ret_size                         += Result;
                     buf_len                           -= Result;
                     ZB_OTA_Ctxt.FWImageBytesRemaining -= Result;
                     FW_UPGRADE_ZIGBEE_PRINTF("Zigbee OTA plugin: read %d bytes, %d bytes remaining.\r\n", Result, ZB_OTA_Ctxt.FWImageBytesRemaining);

                     /* Signal that data has been read and buffer space is available. */
                     qurt_signal_set(&ZB_OTA_Ctxt.Signal, ZIGBEE_OTA_PLUGIN_EVENT_DATA_READ);
                  }
                  else
                  {
                     RetVal = QAPI_FW_UPGRADE_ERROR_E;
                     FW_UPGRADE_ZIGBEE_DEBUG_PRINTF("plugin_Zigbee_Recv_Data() error, could not read data.\r\n");
                  }
               }
               else
               {
                  RetVal = QAPI_FW_UPGRADE_ERROR_E;
                  FW_UPGRADE_ZIGBEE_DEBUG_PRINTF("plugin_Zigbee_Recv_Data() error, could not take mutex.\r\n");
               }
            }
            else
            {
               RetVal = QAPI_FW_UPGRADE_ERROR_E;
               FW_UPGRADE_ZIGBEE_DEBUG_PRINTF("plugin_Zigbee_Recv_Data() error, timed out waiting for read data.\r\n");
            }
         }
      }
      else
      {
         RetVal = QAPI_FW_UPGRADE_ERROR_E;
         FW_UPGRADE_ZIGBEE_DEBUG_PRINTF("plugin_Zigbee_Recv_Data() error, plugin not running.\r\n");
      }
   }
   else
   {
      RetVal = QAPI_FW_UPGRADE_ERROR_E;
      FW_UPGRADE_ZIGBEE_DEBUG_PRINTF("plugin_Zigbee_Recv_Data() error, invalid parameter.\r\n");
   }

   return RetVal;
}

/*
   Handles aborting the upgrade. Since the Zigbee stack automatically
   polls for data, we will just need to flag to discard any additional
   image data received after this.
*/
qapi_Fw_Upgrade_Status_Code_t plugin_Zigbee_Abort(void)
{
   /* Clean up the module. */
   Zigbee_OTA_Plugin_Deinit();

   return QAPI_FW_UPGRADE_OK_E;
}

/*
   Handles resuming the upgrade. There is no mechanism to pause the
   upgrade with the Zigbee plugin, so no logic is needed.
*/
qapi_Fw_Upgrade_Status_Code_t plugin_Zigbee_Resume(const char* interface_name, const char *url, uint32_t offset)
{
   /* No action needed. */
   return QAPI_FW_UPGRADE_OK_E;
}

