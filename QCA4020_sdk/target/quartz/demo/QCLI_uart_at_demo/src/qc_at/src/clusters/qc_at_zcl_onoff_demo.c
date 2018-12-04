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

#include "malloc.h"
#include "string.h"
#include "stringl.h"
#include "util.h"
#include "qcli_api.h"
#include "qcli_util.h"
#include "qc_at_zigbee.h"
#include "qc_at_zcl.h"
#include "zcl_onoff_demo.h"
#include "zcl_scenes_demo.h"

#include "qapi_zb.h"
#include "qapi_zb_cl.h"
#include "qapi_zb_cl_onoff.h"
#include "qapi_zb_cl_scenes.h"
#include "qosa_util.h"

/* Structure representing the ZigBee onoff demo context information. */
typedef struct ZigBee_Onoff_Demo_Context_s
{
   QCLI_Group_Handle_t QCLI_Handle;     /*< QCLI handle for the main ZigBee demo. */
   uint8_t             OnOffSceneData;  /*< The on/off scenes data.*/
} ZigBee_Onoff_Demo_Context_t;

/* The ZigBee OnOff demo context. */
static ZigBee_Onoff_Demo_Context_t ZigBee_OnOff_Demo_Context;
static void ZCL_OnOff_Demo_Server_CB(qapi_ZB_Handle_t ZB_Handle, qapi_ZB_Cluster_t Cluster, qapi_ZB_CL_OnOff_Server_Event_Data_t *EventData, uint32_t CB_Param);
static void ZCL_OnOff_Demo_Client_CB(qapi_ZB_Handle_t ZB_Handle, qapi_ZB_Cluster_t Cluster, qapi_ZB_CL_OnOff_Client_Event_Data_t *EventData, uint32_t CB_Param);
extern QCLI_Context_t QCLI_Context;
/* Command list for the ZigBee light demo. */
const QCLI_Command_t ZigBee_OnOff_CMD_List[] =
{
   /* cmd_function              thread  cmd_string      usage_string               description */
   {cmd_ZCL_OnOff_Help,         false,  "Help",         "",                        "Display the all ZCL on off commands."},
   {cmd_ZCL_OnOff_On,           false,  "on",           "[DevId][ClientEndpoint]", "Sends an On command to an OnOff server."},
   {cmd_ZCL_OnOff_Off,          false,  "off",          "[DevId][ClientEndpoint]", "Sends an Off command to an OnOff server."},
   {cmd_ZCL_OnOff_Toggle,       false,  "toggle",       "[DevId][ClientEndpoint]", "Sends a Toggle command to an OnOff server."},
   {cmd_ZCL_OnOff_SetSceneData, false,  "SetSceneData", "[OnOff]",                 "Set the On/Off scene data."}
};

const QCLI_Command_Group_t ZCL_OnOff_Cmd_Group = {"OnOff", sizeof(ZigBee_OnOff_CMD_List) / sizeof(QCLI_Command_t), ZigBee_OnOff_CMD_List};

QCLI_Command_Status_t cmd_ZCL_OnOff_Help(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    QCLI_Command_Status_t Ret_Val;
    uint32_t              Result;
    int32_t               Index;

    if(TAKE_LOCK(QCLI_Context.CLI_Mutex))
    {
        Result = Display_Help(QCLI_Context.Executing_Group, Parameter_Count, Parameter_List);

        /* if there was an error parsing the command list, print out an error
           message here (this is in addition to the usage message that will be
           printed out). */
        if(Result > 0)
        {
            LOG_INFO("Command \"%s", Parameter_List[0].String_Value);

            for(Index = 1; Index < Result; Index ++)
            {
                LOG_INFO(" %s", Parameter_List[Index].String_Value);
            }

            LOG_WARN("\" not found.\n");

            Ret_Val = QCLI_STATUS_USAGE_E;
        }
        else
        {
            Ret_Val = QCLI_STATUS_SUCCESS_E;
            LOG_AT_OK();
        }

        RELEASE_LOCK(QCLI_Context.CLI_Mutex);
    }
    else
    {
        Ret_Val = QCLI_STATUS_ERROR_E;
        LOG_AT_ERROR();
    }

    return(Ret_Val);

}

/**
   @brief Executes the "SendOn" command to send an On/Off on command to a
          device.

   Parameter_List[0] is the index of the device to send to. Use index zero to
                     use the binding table (if setup).
   Parameter_List[1] Endpoint of the On/Off client cluster to use to send the
                     command.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t cmd_ZCL_OnOff_On(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   qapi_Status_t                   Result;
   QCLI_Command_Status_t           Ret_Val;
   ZCL_Demo_Cluster_Info_t        *ClusterInfo;
   qapi_ZB_CL_General_Send_Info_t  SendInfo;
   uint8_t                         DeviceId;

   /* Ensure both the stack is initialized and the switch endpoint. */
   if(GetZigBeeHandle() != NULL)
   {
      Ret_Val = QCLI_STATUS_SUCCESS_E;

      /* Validate the device ID. */
      if((Parameter_Count >= 2) &&
         (Parameter_List[0].Integer_Is_Valid) &&
         (Verify_Integer_Parameter(&(Parameter_List[1]), QAPI_ZB_APS_MIN_ENDPOINT, QAPI_ZB_APS_MAX_ENDPOINT)))
      {
         DeviceId    = Parameter_List[0].Integer_Value;
         ClusterInfo = ZCL_FindClusterByEndpoint((uint8_t)(Parameter_List[1].Integer_Value), QAPI_ZB_CL_CLUSTER_ID_ONOFF, ZCL_DEMO_CLUSTERTYPE_CLIENT);

         if(ClusterInfo != NULL)
         {
            memset(&SendInfo, 0, sizeof(SendInfo));

            /* Format the destination addr. mode, address, and endpoint. */
            if(Format_Send_Info_By_Device(DeviceId, &SendInfo))
            {
               Result = qc_drv_ZB_CL_OnOff_Send_On(qc_api_get_qc_drv_context(), ClusterInfo->Handle, &SendInfo, false);
               if(Result == QAPI_OK)
               {
                  LOG_INFO("qc_drv_ZB_CL_OnOff_Send_On");
               }
               else
               {
                  Ret_Val = QCLI_STATUS_ERROR_E;
                  LOG_ERR("qc_drv_ZB_CL_OnOff_Send_On", Result);
               }
            }
            else
            {
               LOG_ERR("Invalid device ID.\n");
               Ret_Val = QCLI_STATUS_ERROR_E;
            }
         }
         else
         {
            LOG_ERR("Invalid Cluster Index.\n");
            Ret_Val = QCLI_STATUS_ERROR_E;
         }
      }
      else
      {
         Ret_Val = QCLI_STATUS_USAGE_E;
      }
   }
   else
   {
      LOG_WARN("ZigBee stack is not initialized.\n");
      Ret_Val = QCLI_STATUS_ERROR_E;
   }

   return(Ret_Val);
}

/**
   @brief Executes the "SendOn" command to send an On/Off off command to a
          device.

   Parameter_List[0] is the index of the device to send to. Use index zero to
                     use the binding table (if setup).
   Parameter_List[1] Endpoint of the On/Off client cluster to use to send the
                     command.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t cmd_ZCL_OnOff_Off(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t           Ret_Val;
   qapi_Status_t                   Result;
   ZCL_Demo_Cluster_Info_t        *ClusterInfo;
   qapi_ZB_CL_General_Send_Info_t  SendInfo;
   uint8_t                         DeviceId;

   /* Ensure both the stack is initialized and the switch endpoint. */
   if(GetZigBeeHandle() != NULL)
   {
      Ret_Val = QCLI_STATUS_SUCCESS_E;

      /* Validate the device ID. */
      if((Parameter_Count >= 2) &&
         (Parameter_List[0].Integer_Is_Valid) &&
         (Verify_Integer_Parameter(&(Parameter_List[1]), QAPI_ZB_APS_MIN_ENDPOINT, QAPI_ZB_APS_MAX_ENDPOINT)))
      {
         DeviceId    = Parameter_List[0].Integer_Value;
         ClusterInfo = ZCL_FindClusterByEndpoint((uint8_t)(Parameter_List[1].Integer_Value), QAPI_ZB_CL_CLUSTER_ID_ONOFF, ZCL_DEMO_CLUSTERTYPE_CLIENT);

         if(ClusterInfo != NULL)
         {
            memset(&SendInfo, 0, sizeof(SendInfo));

            /* Format the destination addr. mode, address, and endpoint. */
            if(Format_Send_Info_By_Device(DeviceId, &SendInfo))
            {
               Result = qc_drv_ZB_CL_OnOff_Send_Off(qc_api_get_qc_drv_context(), ClusterInfo->Handle, &SendInfo);
               if(Result == QAPI_OK)
               {
                  LOG_INFO("qc_drv_ZB_CL_OnOff_Send_Off");
               }
               else
               {
                  Ret_Val = QCLI_STATUS_ERROR_E;
                  LOG_ERR("qc_drv_ZB_CL_OnOff_Send_Off", Result);
               }
            }
            else
            {
               LOG_ERR("Invalid device ID.\n");
               Ret_Val = QCLI_STATUS_ERROR_E;
            }
         }
         else
         {
            LOG_ERR("Invalid Cluster Index.\n");
            Ret_Val = QCLI_STATUS_ERROR_E;
         }
      }
      else
      {
         Ret_Val = QCLI_STATUS_USAGE_E;
      }
   }
   else
   {
      LOG_WARN("ZigBee stack is not initialized.\n");
      Ret_Val = QCLI_STATUS_ERROR_E;
   }

   return(Ret_Val);
}

/**
   @brief Executes the "SendOn" command to send an On/Off toggle command to a
          device.

   Parameter_List[0] is the index of the device to send to. Use index zero to
                     use the binding table (if setup).
   Parameter_List[1] Endpoint of the On/Off client cluster to use to send the
                     command.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t cmd_ZCL_OnOff_Toggle(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   qapi_Status_t                   Result;
   QCLI_Command_Status_t           Ret_Val;
   ZCL_Demo_Cluster_Info_t        *ClusterInfo;
   qapi_ZB_CL_General_Send_Info_t  SendInfo;
   uint8_t                         DeviceId;

   /* Ensure both the stack is initialized and the switch endpoint. */
   if(GetZigBeeHandle() != NULL)
   {
      Ret_Val = QCLI_STATUS_SUCCESS_E;

      /* Validate the device ID. */
      if((Parameter_Count >= 2) &&
         (Parameter_List[0].Integer_Is_Valid) &&
         (Verify_Integer_Parameter(&(Parameter_List[1]), QAPI_ZB_APS_MIN_ENDPOINT, QAPI_ZB_APS_MAX_ENDPOINT)))
      {
         DeviceId    = Parameter_List[0].Integer_Value;
         ClusterInfo = ZCL_FindClusterByEndpoint((uint8_t)(Parameter_List[1].Integer_Value), QAPI_ZB_CL_CLUSTER_ID_ONOFF, ZCL_DEMO_CLUSTERTYPE_CLIENT);

         if(ClusterInfo != NULL)
         {
            memset(&SendInfo, 0, sizeof(SendInfo));

            /* Format the destination addr. mode, address, and endpoint. */
            if(Format_Send_Info_By_Device(DeviceId, &SendInfo))
            {
               Result = qc_drv_ZB_CL_OnOff_Send_Toggle(qc_api_get_qc_drv_context(), ClusterInfo->Handle, &SendInfo);
               if(Result == QAPI_OK)
               {
                  LOG_INFO("qc_drv_ZB_CL_OnOff_Send_Toggle");
               }
               else
               {
                  Ret_Val = QCLI_STATUS_ERROR_E;
                  LOG_ERR("qc_drv_ZB_CL_OnOff_Send_Toggle", Result);
               }
            }
            else
            {
               LOG_ERR("Invalid device ID.\n");
               Ret_Val = QCLI_STATUS_ERROR_E;
            }
         }
         else
         {
            LOG_ERR("Invalid Cluster Index.\n");
            Ret_Val = QCLI_STATUS_ERROR_E;
         }
      }
      else
      {
         Ret_Val = QCLI_STATUS_USAGE_E;
      }
   }
   else
   {
      LOG_WARN("ZigBee stack is not initialized.\n");
      Ret_Val = QCLI_STATUS_ERROR_E;
   }

   return(Ret_Val);
}

/**
   @brief Executes the "SetSceneData" command.

   Parameter_List[0] is the on/off scene data to be set.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t cmd_ZCL_OnOff_SetSceneData(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t Ret_Val;

   /* Validate the device ID. */
   if((Parameter_Count >= 1) &&
      (Verify_Integer_Parameter(&(Parameter_List[0]), 0, 1)))
   {
      ZigBee_OnOff_Demo_Context.OnOffSceneData = (uint8_t)(Parameter_List[0].Integer_Value);

      LOG_INFO("Scene data set successfully.\n");
      Ret_Val = QCLI_STATUS_SUCCESS_E;
   }
   else
   {
      Ret_Val = QCLI_STATUS_USAGE_E;
   }

   return(Ret_Val);
}

/**
   @brief Handles callbacks for the on/off server cluster.

   @param ZB_Handle is the handle of the ZigBee instance.
   @param EventData is the information for the cluster event.
   @param CB_Param  is the user specified parameter for the callback function.
*/
static void ZCL_OnOff_Demo_Server_CB(qapi_ZB_Handle_t ZB_Handle, qapi_ZB_Cluster_t Cluster, qapi_ZB_CL_OnOff_Server_Event_Data_t *EventData, uint32_t CB_Param)
{
   if((ZB_Handle != NULL) && (Cluster != NULL) && (EventData != NULL))
   {
      switch(EventData->Event_Type)
      {
         case QAPI_ZB_CL_ONOFF_SERVER_EVENT_TYPE_STATE_CHANGE_E:
            LOG_AT_EVT("EVT_ZB: OnOff Server State Change: %s\n", EventData->Data.State_Changed.On ? "ON" : "OFF");
            break;

         default:
            LOG_AT_EVT("EVT_ZB: Unhandled OnOff server event %d.\n", EventData->Event_Type);
            break;
      }

      QCLI_Display_Prompt();
   }
}

/**
   @brief Handles callbacks for the OnOff client cluster.

   @param ZB_Handle is the handle of the ZigBee instance.
   @param EventData is the information for the cluster event.
   @param CB_Param  is the user specified parameter for the callback function.
*/
static void ZCL_OnOff_Demo_Client_CB(qapi_ZB_Handle_t ZB_Handle, qapi_ZB_Cluster_t Cluster, qapi_ZB_CL_OnOff_Client_Event_Data_t *EventData, uint32_t CB_Param)
{
   if((ZB_Handle != NULL) && (Cluster != NULL) && (EventData != NULL))
   {
      switch(EventData->Event_Type)
      {
         case QAPI_ZB_CL_ONOFF_CLIENT_EVENT_TYPE_DEFAULT_RESPONSE_E:
            LOG_AT_EVT("EVT_ZB: OnOff Client Default Response:\n");
            LOG_AT_EVT("EVT_ZB: Status:        %d\n", EventData->Data.Default_Response.Status);
            LOG_AT_EVT("EVT_ZB: CommandID:     0x%02X\n", EventData->Data.Default_Response.CommandId);
            LOG_AT_EVT("EVT_ZB: CommandStatus: %d\n", EventData->Data.Default_Response.CommandStatus);
            break;

         default:
            LOG_AT_EVT("EVT_ZB: Unhandled OnOff client event %d.\n", EventData->Event_Type);
            break;
      }

      QCLI_Display_Prompt();
   }
}

/**
   @brief Initializes the ZCL OnOff demo.

   @param ZigBee_QCLI_Handle is the parent QCLI handle for the OnOff demo.

   @return true if the ZigBee light demo initialized successfully, false
           otherwise.
*/
qbool_t Initialize_ZCL_OnOff_Demo(QCLI_Group_Handle_t ZigBee_QCLI_Handle)
{
   qbool_t Ret_Val;

   /* Register OnOff command group. */
   ZigBee_OnOff_Demo_Context.QCLI_Handle = QCLI_Register_Command_Group(ZigBee_QCLI_Handle, &ZCL_OnOff_Cmd_Group);
   if(ZigBee_OnOff_Demo_Context.QCLI_Handle != NULL)
   {
      Ret_Val = true;
   }
   else
   {
      LOG_WARN("Failed to register ZCL OnOff command group.\n");
      Ret_Val = false;
   }

   return(Ret_Val);
}

/**
   @brief Creates an OnOff server cluster.

   @param Endpoint is the endpoint the cluster will be part of.
   @param PrivData is a pointer to the private data for the cluster demo.  This
                   will be initaialized to NULL before the create function is
                   called so can be ignored if the demo has no private data.

   @return The handle for the newly created function or NULL if there was an
           error.
*/
qapi_ZB_Cluster_t ZCL_OnOff_Demo_Create_Server(uint8_t Endpoint, void **PrivData)
{
   qapi_ZB_Cluster_t         Ret_Val;
   qapi_Status_t             Result;
   qapi_ZB_Handle_t          ZigBee_Handle;
   qapi_ZB_CL_Cluster_Info_t ClusterInfo;
   qapi_ZB_CL_Attribute_t    AttributeList[ZCL_DEMO_MAX_ATTRIBUTES_PER_CLUSTER];

   ZigBee_Handle = GetZigBeeHandle();
   if(ZigBee_Handle != NULL)
   {
      memset(&ClusterInfo, 0, sizeof(qapi_ZB_CL_Cluster_Info_t));
      ClusterInfo.Endpoint       = Endpoint;
      ClusterInfo.AttributeCount = ZCL_DEMO_MAX_ATTRIBUTES_PER_CLUSTER;
      ClusterInfo.AttributeList  = AttributeList;
      Result = qapi_ZB_CL_OnOff_Populate_Attributes(true, &(ClusterInfo.AttributeCount), (qapi_ZB_CL_Attribute_t*) ClusterInfo.AttributeList);
      if(Result == QAPI_OK)
      {
         Result = qapi_ZB_CL_OnOff_Create_Server(ZigBee_Handle, &Ret_Val, &ClusterInfo, ZCL_OnOff_Demo_Server_CB, 0);
         if(Result != QAPI_OK)
         {
            LOG_ERR("qc_drv_ZB_CL_OnOff_Create_Server", Result);
            Ret_Val = NULL;
         }
      }
      else
      {
         LOG_INFO("qc_drv_ZB_CL_OnOff_Populate_Attributes", Result);
         Ret_Val = NULL;
      }
   }
   else
   {
      LOG_WARN("ZigBee stack is not initialized.\n");
      Ret_Val = NULL;
   }

   return(Ret_Val);
}

/**
   @brief Creates an OnOff client cluster.

   @param Endpoint is the endpoint the cluster will be part of.
   @param PrivData is a pointer to the private data for the cluster demo.  This
                   will be initaialized to NULL before the create function is
                   called so can be ignored if the demo has no private data.

   @return The handle for the newly created function or NULL if there was an
           error.
*/
qapi_ZB_Cluster_t ZCL_OnOff_Demo_Create_Client(uint8_t Endpoint, void **PrivData)
{
   qapi_ZB_Cluster_t         Ret_Val;
   qapi_Status_t             Result;
   qapi_ZB_Handle_t          ZigBee_Handle;
   qapi_ZB_CL_Cluster_Info_t ClusterInfo;

   ZigBee_Handle = GetZigBeeHandle();
   if(ZigBee_Handle != NULL)
   {
      memset(&ClusterInfo, 0, sizeof(qapi_ZB_CL_Cluster_Info_t));
      ClusterInfo.Endpoint = Endpoint;

      Result = qapi_ZB_CL_OnOff_Create_Client(ZigBee_Handle, &Ret_Val, &ClusterInfo, ZCL_OnOff_Demo_Client_CB, 0);
      if(Result != QAPI_OK)
      {
         LOG_ERR("qapi_ZB_CL_OnOff_Create_Client", Result);
         Ret_Val = NULL;
      }
   }
   else
   {
      LOG_WARN("ZigBee stack is not initialized.\n");
      Ret_Val = NULL;
   }

   return(Ret_Val);
}

/**
   @brief Gets the scenes data for an on/off cluster.

   @param ExtData       Buffer for the extension data of the cluster.
   @param ExtDataLength Size of the extension data for the cluster. the initial
                        value will be the size of the ExtData buffer provided.

   @return The handle for the newly created function or NULL if there was an
           error.
*/
qbool_t ZCL_OnOff_GetScenesData(qapi_ZB_CL_Scenes_Extension_Field_Set_t *ExtData)
{
   qbool_t Ret_Val;

   if(ExtData != NULL)
   {
      ExtData->ClusterId = QAPI_ZB_CL_CLUSTER_ID_ONOFF;
      ExtData->Length    = sizeof(uint8_t);
      ExtData->Data      = &(ZigBee_OnOff_Demo_Context.OnOffSceneData);
      Ret_Val            = true;
   }
   else
   {
      Ret_Val = false;
   }

   return(Ret_Val);
}

