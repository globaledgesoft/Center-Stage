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
#include "zcl_levelcontrol_demo.h"
#include "zcl_scenes_demo.h"

#include "qapi_zb.h"
#include "qapi_zb_cl.h"
#include "qapi_zb_cl_level_control.h"
#include "qapi_zb_cl_onoff.h"
#include "qosa_util.h"

/* Structure representing the ZigBee light demo context information. */
typedef struct ZCL_LevelControl_Demo_Data_s
{
   uint8_t  CurrentLevel;
} ZCL_LevelControl_Demo_Data_t;

/* Structure representing the ZigBee level control demo context information. */
typedef struct ZigBee_LevelControl_Demo_Context_s
{
   QCLI_Group_Handle_t QCLI_Handle;    /*< QCLI handle for the main ZigBee demo. */
   uint8_t             LevelSceneData; /*< The level control scenes data.*/
} ZigBee_LevelControl_Demo_Context_t;

/* The ZigBee level control demo context. */
static ZigBee_LevelControl_Demo_Context_t ZigBee_LevelControl_Demo_Context;
extern QCLI_Context_t QCLI_Context;
static void ZCL_LevelControl_Demo_Server_CB(qapi_ZB_Handle_t ZB_Handle, qapi_ZB_Cluster_t Cluster, qapi_ZB_CL_LevelControl_Server_Event_Data_t *EventData, uint32_t CB_Param);
static void ZCL_LevelControl_Demo_Client_CB(qapi_ZB_Handle_t ZB_Handle, qapi_ZB_Cluster_t Cluster, qapi_ZB_CL_LevelControl_Client_Event_Data_t *EventData, uint32_t CB_Param);

/* Command list for the ZigBee light demo. */
const QCLI_Command_t ZigBee_LevelControl_CMD_List[] =
{
   /* cmd_function                     thread  cmd_string      usage_string                                                       description */
   {cmd_ZCL_LevelControl_Help,         false,  "Help",         "",                                                                "Display the all ZCL level control commands."},
   {cmd_ZCL_LevelControl_MoveToLevel,  false,  "MoveToLevel",  "[DevId][ClientEndpoint][WithOnOff][Level0-255][Time]",            "Sends a Move To Level command to a Level Control server."},
   {cmd_ZCL_LevelControl_Move,         false,  "Move",         "[DevId][ClientEndpoint][WithOnOff][Down=0,Up=1][Rate]",           "Sends a Move command to a Level Control server."},
   {cmd_ZCL_LevelControl_Step,         false,  "Step",         "[DevId][ClientEndpoint][WithOnOff][Down=0,Up=1][StepSize][Time]", "Sends a Step command to a Level Control server."},
   {cmd_ZCL_LevelControl_Stop,         false,  "Stop",         "[DevId][ClientEndpoint]",                                         "Sends a Stop command to a Level Control server."},
   {cmd_ZCL_LevelControl_SetSceneData, false,  "SCENEDATA", "[CurrentLevel]",                                                  "Set the level control scene data."}
};

const QCLI_Command_Group_t ZCL_LevelControl_Cmd_Group = {"LevelControl", sizeof(ZigBee_LevelControl_CMD_List) / sizeof(QCLI_Command_t), ZigBee_LevelControl_CMD_List};

QCLI_Command_Status_t cmd_ZCL_LevelControl_Help(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
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
   @brief Executes the "SendToLevel" command to send a level control move to
          level command to a device.

   Parameter_List[0] is the index of the device to send to. Use index zero to
                     use the binding table (if setup).
   Parameter_List[1] Endpoint of the Level Control client cluster to use to send
                     the command.
   Parameter_List[2] is a flag indicating if the command should be sent "with
                     on/off".
   Parameter_List[3] is the level the light should move to.
   Parameter_List[4] is the time the light should take to move to the level.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t cmd_ZCL_LevelControl_MoveToLevel(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t           Ret_Val;
   qapi_Status_t                   Result;
   ZCL_Demo_Cluster_Info_t        *ClusterInfo;
   qapi_ZB_CL_General_Send_Info_t  SendInfo;
   uint32_t                        DeviceId;
   qbool_t                         WithOnOff;
   uint8_t                         Level;
   uint16_t                        TransTime;

   /* Ensure both the stack is initialized and the switch endpoint. */
   if((GetZigBeeHandle() != NULL) && (ZigBee_LevelControl_Demo_Context.QCLI_Handle != NULL))
   {
      Ret_Val = QCLI_STATUS_SUCCESS_E;

      if((Parameter_Count >= 5) &&
         (Parameter_List[0].Integer_Is_Valid) &&
         (Verify_Integer_Parameter(&(Parameter_List[1]), QAPI_ZB_APS_MIN_ENDPOINT, QAPI_ZB_APS_MAX_ENDPOINT)) &&
         (Verify_Integer_Parameter(&Parameter_List[2], 0, 1)) &&
         (Verify_Integer_Parameter(&Parameter_List[3], 0, 0xFF)) &&
         (Verify_Integer_Parameter(&Parameter_List[4], 0, 0xFFFF)))
      {
         DeviceId    = Parameter_List[0].Integer_Value;
         ClusterInfo = ZCL_FindClusterByEndpoint((uint8_t)(Parameter_List[1].Integer_Value), QAPI_ZB_CL_CLUSTER_ID_LEVEL_CONTROL, ZCL_DEMO_CLUSTERTYPE_CLIENT);
         WithOnOff   = Parameter_List[2].Integer_Value;
         Level       = Parameter_List[3].Integer_Value;
         TransTime   = Parameter_List[4].Integer_Value;

         if(ClusterInfo != NULL)
         {
            memset(&SendInfo, 0, sizeof(SendInfo));

            /* Format the destination addr. mode, address, and endpoint. */
            if(Format_Send_Info_By_Device(DeviceId, &SendInfo))
            {
               Result = qc_drv_ZB_CL_LevelControl_Send_Move_To_Level(qc_api_get_qc_drv_context(), ClusterInfo->Handle, &SendInfo, Level, TransTime, WithOnOff);
               if(Result == QAPI_OK)
               {
                  LOG_INFO("qc_drv_ZB_CL_LevelControl_Send_Move_To_Level");
               }
               else
               {
                  Ret_Val = QCLI_STATUS_ERROR_E;
                  LOG_ERR("qc_drv_ZB_CL_LevelControl_Send_Move_To_Level", Result);
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
   @brief Executes the "SendMove" command to send a level control move command
          to a device.

   Parameter_List[0] is the index of the device to send to. Use index zero to
                     use the binding table (if setup).
   Parameter_List[1] Endpoint of the Level Control client cluster to use to send
                     the command.
   Parameter_List[2] is a flag indicating if the command should be sent "with
                     on/off".
   Parameter_List[3] indicates if the light should move up (1) or down (0).
   Parameter_List[4] is rate at which the light should move.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t cmd_ZCL_LevelControl_Move(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t           Ret_Val;
   qapi_Status_t                   Result;
   ZCL_Demo_Cluster_Info_t        *ClusterInfo;
   qapi_ZB_CL_General_Send_Info_t  SendInfo;
   uint32_t                        DeviceId;
   qbool_t                         WithOnOff;
   qbool_t                         MoveDown;
   uint8_t                         Rate;

   /* Ensure both the stack is initialized and the switch endpoint. */
   if(GetZigBeeHandle() != NULL)
   {
      Ret_Val = QCLI_STATUS_SUCCESS_E;

      if((Parameter_Count >= 5) &&
         (Parameter_List[0].Integer_Is_Valid) &&
         (Verify_Integer_Parameter(&(Parameter_List[1]), QAPI_ZB_APS_MIN_ENDPOINT, QAPI_ZB_APS_MAX_ENDPOINT)) &&
         (Verify_Integer_Parameter(&Parameter_List[2], 0, 1)) &&
         (Verify_Integer_Parameter(&Parameter_List[3], 0, 1)) &&
         (Verify_Integer_Parameter(&Parameter_List[4], 0, 0xFF)))
      {
         DeviceId    = Parameter_List[0].Integer_Value;
         ClusterInfo = ZCL_FindClusterByEndpoint((uint8_t)(Parameter_List[1].Integer_Value), QAPI_ZB_CL_CLUSTER_ID_LEVEL_CONTROL, ZCL_DEMO_CLUSTERTYPE_CLIENT);
         WithOnOff   = Parameter_List[2].Integer_Value;
         MoveDown    = Parameter_List[3].Integer_Value == 0;
         Rate        = Parameter_List[4].Integer_Value;

         if(ClusterInfo != NULL)
         {
            memset(&SendInfo, 0, sizeof(SendInfo));

            /* Format the destination addr. mode, address, and endpoint. */
            if(Format_Send_Info_By_Device(DeviceId, &SendInfo))
            {
               Result = qc_drv_ZB_CL_LevelControl_Send_Move(qc_api_get_qc_drv_context(), ClusterInfo->Handle, &SendInfo, MoveDown, Rate, WithOnOff);
               if(Result == QAPI_OK)
               {
                  LOG_INFO("qc_drv_ZB_CL_LevelControl_Send_Move");
               }
               else
               {
                  Ret_Val = QCLI_STATUS_ERROR_E;
                  LOG_ERR("qc_drv_ZB_CL_LevelControl_Send_Move", Result);
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
   @brief Executes the "SendStep" command to send a level control step command
          to a device.

   Parameter_List[0] is the index of the device to send to. Use index zero to
                     use the binding table (if setup).
   Parameter_List[1] Endpoint of the Level Control client cluster to use to send
                     the command.
   Parameter_List[2] is a flag indicating if the command should be sent "with
                     on/off".
   Parameter_List[3] indicates if the light should move up (1) or down (0).
   Parameter_List[4] is the step size for the command.
   Parameter_List[5] is the transition time the light should use.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t cmd_ZCL_LevelControl_Step(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t           Ret_Val;
   qapi_Status_t                   Result;
   ZCL_Demo_Cluster_Info_t        *ClusterInfo;
   qapi_ZB_CL_General_Send_Info_t  SendInfo;
   uint32_t                        DeviceId;
   qbool_t                         WithOnOff;
   qbool_t                         StepDown;
   uint8_t                         StepSize;
   uint16_t                        TransTime;

   /* Ensure both the stack is initialized and the switch endpoint. */
   if(GetZigBeeHandle() != NULL)
   {
      Ret_Val = QCLI_STATUS_SUCCESS_E;

      if((Parameter_Count >= 6) &&
         (Parameter_List[0].Integer_Is_Valid) &&
         (Verify_Integer_Parameter(&(Parameter_List[1]), QAPI_ZB_APS_MIN_ENDPOINT, QAPI_ZB_APS_MAX_ENDPOINT)) &&
         (Verify_Integer_Parameter(&Parameter_List[2], 0, 1)) &&
         (Verify_Integer_Parameter(&Parameter_List[3], 0, 1)) &&
         (Verify_Integer_Parameter(&Parameter_List[4], 0, 0xFF)) &&
         (Verify_Integer_Parameter(&Parameter_List[5], 0, 0xFFFF)))
      {
         DeviceId    = Parameter_List[0].Integer_Value;
         ClusterInfo = ZCL_FindClusterByEndpoint((uint8_t)(Parameter_List[1].Integer_Value), QAPI_ZB_CL_CLUSTER_ID_LEVEL_CONTROL, ZCL_DEMO_CLUSTERTYPE_CLIENT);
         WithOnOff   = Parameter_List[2].Integer_Value;
         StepDown    = Parameter_List[3].Integer_Value == 0;
         StepSize    = Parameter_List[4].Integer_Value;
         TransTime   = Parameter_List[5].Integer_Value;

         if(ClusterInfo != NULL)
         {
            memset(&SendInfo, 0, sizeof(SendInfo));

            /* Format the destination addr. mode, address, and endpoint. */
            if(Format_Send_Info_By_Device(DeviceId, &SendInfo))
            {
               Result = qc_drv_ZB_CL_LevelControl_Send_Step(qc_api_get_qc_drv_context(), ClusterInfo->Handle, &SendInfo, StepDown, StepSize, TransTime, WithOnOff);
               if(Result == QAPI_OK)
               {
                  LOG_INFO("qc_drv_ZB_CL_LevelControl_Send_Step");
               }
               else
               {
                  Ret_Val = QCLI_STATUS_ERROR_E;
                  LOG_ERR("qc_drv_ZB_CL_LevelControl_Send_Step", Result);
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
   @brief Executes the "SendStop" command to send a level control stop command
          to a device.

   Parameter_List[0] is the index of the device to send to. Use index zero to
                     use the binding table (if setup).
   Parameter_List[1] Endpoint of the Level Control client cluster to use to send
                     the command.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t cmd_ZCL_LevelControl_Stop(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t           Ret_Val;
   qapi_Status_t                   Result;
   ZCL_Demo_Cluster_Info_t        *ClusterInfo;
   qapi_ZB_CL_General_Send_Info_t  SendInfo;
   uint32_t                        DeviceId;

   if(GetZigBeeHandle() != NULL)
   {
      Ret_Val = QCLI_STATUS_SUCCESS_E;

      if((Parameter_Count >= 2) &&
         (Parameter_List[0].Integer_Is_Valid) &&
         (Verify_Integer_Parameter(&(Parameter_List[1]), QAPI_ZB_APS_MIN_ENDPOINT, QAPI_ZB_APS_MAX_ENDPOINT)))
      {
         DeviceId    = Parameter_List[0].Integer_Value;
         ClusterInfo = ZCL_FindClusterByEndpoint((uint8_t)(Parameter_List[1].Integer_Value), QAPI_ZB_CL_CLUSTER_ID_LEVEL_CONTROL, ZCL_DEMO_CLUSTERTYPE_CLIENT);

         if(ClusterInfo != NULL)
         {
            memset(&SendInfo, 0, sizeof(SendInfo));

            /* Format the destination addr. mode, address, and endpoint. */
            if(Format_Send_Info_By_Device(DeviceId, &SendInfo))
            {
               Result = qc_drv_ZB_CL_LevelControl_Send_Stop(qc_api_get_qc_drv_context(), ClusterInfo->Handle, &SendInfo);
               if(Result == QAPI_OK)
               {
                  LOG_INFO("qc_drv_ZB_CL_LevelControl_Send_Stop");
               }
               else
               {
                  Ret_Val = QCLI_STATUS_ERROR_E;
                  LOG_ERR("qc_drv_ZB_CL_LevelControl_Send_Stop", Result);
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
   @brief Executes the "LevelControl_SetSceneData" command.

   Parameter_List[0] is the level control scene data to be set.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t cmd_ZCL_LevelControl_SetSceneData(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t Ret_Val;

   /* Validate the device ID. */
   if((Parameter_Count >= 1) &&
      (Verify_Integer_Parameter(&(Parameter_List[0]), 0, 254)))
   {
      ZigBee_LevelControl_Demo_Context.LevelSceneData = (uint8_t)(Parameter_List[0].Integer_Value);

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
static void ZCL_LevelControl_Demo_Server_CB(qapi_ZB_Handle_t ZB_Handle, qapi_ZB_Cluster_t Cluster, qapi_ZB_CL_LevelControl_Server_Event_Data_t *EventData, uint32_t CB_Param)
{
   ZCL_Demo_Cluster_Info_t      *LevelClusterInfo;
   ZCL_Demo_Cluster_Info_t      *OnOffClusterInfo;
   ZCL_LevelControl_Demo_Data_t *LevelControlData;
   qbool_t                       DisplayPrompt;
   uint8_t                       OnOffState;

   if((ZB_Handle != NULL) && (Cluster != NULL) && (EventData != NULL))
   {
      LevelClusterInfo = ZCL_FindClusterByHandle(Cluster);
      if(LevelClusterInfo != NULL)
      {
         LevelControlData = (ZCL_LevelControl_Demo_Data_t *)(LevelClusterInfo->PrivData);

         /* Try to find an on/off cluster on the same endpoint (The demo will
            assume this is the linked on/off cluster. */
         OnOffClusterInfo = ZCL_FindClusterByEndpoint(LevelClusterInfo->Endpoint, QAPI_ZB_CL_CLUSTER_ID_ONOFF, ZCL_DEMO_CLUSTERTYPE_SERVER);
      }
      else
      {
         OnOffClusterInfo = NULL;
         LevelControlData = NULL;
      }

      DisplayPrompt = true;

      switch(EventData->Event_Type)
      {
         case QAPI_ZB_CL_LEVELCONTROL_SERVER_EVENT_TYPE_ATTR_CUSTOM_READ_E:
            if(EventData->Data.Attr_Custom_Read.AttrId == QAPI_ZB_CL_LEVELCONTROL_ATTR_ID_CURRENT_LEVEL)
            {
               if(LevelClusterInfo != NULL)
               {
                  if(*(EventData->Data.Attr_Custom_Read.DataLength) >= sizeof(uint8_t))
                  {
                     *(EventData->Data.Attr_Custom_Read.Data)   = LevelControlData->CurrentLevel;
                     *(EventData->Data.Attr_Custom_Read.Result) = QAPI_OK;
                     DisplayPrompt = false;
                  }
                  else
                  {
                     *(EventData->Data.Attr_Custom_Read.Result) = QAPI_ZB_ERR_ZCL_INVALID_DATA_TYPE;
                     LOG_AT_EVT("EVT_ZB: LevelControl Server Custom Read with invalid size (%d / %d).\n", EventData->Data.Attr_Custom_Read.AttrId, EventData->Data.Attr_Custom_Read.DataLength);
                  }
               }

               *(EventData->Data.Attr_Custom_Read.DataLength) = sizeof(uint8_t);
            }
            else
            {
               *(EventData->Data.Attr_Custom_Read.Result) = QAPI_ZB_ERR_ZCL_UNREPORTABLE_ATTRIBUTE;
               LOG_AT_EVT("EVT_ZB: LevelControl Server Custom Read for unsupported attribute (%d).\n", EventData->Data.Attr_Custom_Read.AttrId);
            }
            break;

         case QAPI_ZB_CL_LEVELCONTROL_SERVER_EVENT_TYPE_ATTR_CUSTOM_WRITE_E:
            if(EventData->Data.Attr_Custom_Write.AttrId == QAPI_ZB_CL_LEVELCONTROL_ATTR_ID_CURRENT_LEVEL)
            {
               if(LevelClusterInfo != NULL)
               {
                  if(EventData->Data.Attr_Custom_Write.DataLength == sizeof(uint8_t))
                  {
                     switch(EventData->Data.Attr_Custom_Write.Mode)
                     {
                        case QAPI_ZB_CL_WRITE_MODE_NORMAL_E:
                        case QAPI_ZB_CL_WRITE_MODE_FORCE_E:
                           LevelControlData->CurrentLevel = *(EventData->Data.Attr_Custom_Write.Data);
                        case QAPI_ZB_CL_WRITE_MODE_TEST_E:
                        default:
                           break;
                     }

                     LOG_AT_EVT("EVT_ZB: LevelControl Server CurrentLevel set to %d.\n", LevelControlData->CurrentLevel);

                     if(OnOffClusterInfo != NULL)
                     {
                        /* Update the on/off attribute. */
                        OnOffState = (LevelControlData->CurrentLevel == 0) ? 0 : 1;
                        qapi_ZB_CL_Write_Local_Attribute(OnOffClusterInfo->Handle, QAPI_ZB_CL_ONOFF_ATTR_ID_ON_OFF, sizeof(uint8_t), &OnOffState);
                     }

                     *(EventData->Data.Attr_Custom_Write.Result) = QAPI_OK;
                  }
                  else
                  {
                     *(EventData->Data.Attr_Custom_Write.Result) = QAPI_ZB_ERR_ZCL_INVALID_DATA_TYPE;
                     LOG_AT_EVT("EVT_ZB: LevelControl Server Custom Write with invalid size (%d / %d).\n", EventData->Data.Attr_Custom_Write.AttrId, EventData->Data.Attr_Custom_Write.DataLength);
                  }
               }
            }
            else
            {
               *(EventData->Data.Attr_Custom_Write.Result) = QAPI_ZB_ERR_ZCL_UNREPORTABLE_ATTRIBUTE;
               LOG_AT_EVT("EVT_ZB: LevelControl Server Custom Write for unsupported attribute (%d).\n", EventData->Data.Attr_Custom_Write.AttrId);
            }
            break;

         case QAPI_ZB_CL_LEVELCONTROL_SERVER_EVENT_TYPE_TRANSITION_E:
           LOG_AT_EVT("EVT_ZB: LevelControl Server Transistion Event:\n");
            if((EventData->Data.Transition.Level != QAPI_ZB_CL_LEVELCONTROL_INVALID_LEVEL) && (EventData->Data.Transition.TransitionTime != QAPI_ZB_CL_LEVELCONTROL_INVALID_TRANSITION_TIME))
            {
               LOG_AT_EVT("EVT_ZB: Level:          %d\n", EventData->Data.Transition.Level);
               LOG_AT_EVT("EVT_ZB: TransitionTime: %d\n", EventData->Data.Transition.TransitionTime);
               LOG_AT_EVT("EVT_ZB: WithOnOff:      %s\n", EventData->Data.Transition.WithOnOff ? "True" : "False");

               if(LevelClusterInfo != NULL)
               {
                  /* Not emulating transition times so simply set the level. */
                  LevelControlData->CurrentLevel = EventData->Data.Transition.Level;

                  if(OnOffClusterInfo != NULL)
                  {
                     /* Update the on/off attribute. */
                     OnOffState = (LevelControlData->CurrentLevel == 0) ? 0 : 1;
                     qapi_ZB_CL_Write_Local_Attribute(OnOffClusterInfo->Handle, QAPI_ZB_CL_ONOFF_ATTR_ID_ON_OFF, sizeof(uint8_t), &OnOffState);
                  }
               }
            }
            else
            {
               LOG_AT_EVT("EVT_ZB: Stop level transition.\n");
            }
            break;

         case QAPI_ZB_CL_LEVELCONTROL_SERVER_EVENT_TYPE_STOP_E:
           LOG_AT_EVT("EVT_ZB: LevelControl Server Stop Event\n");
            break;

         default:
            LOG_AT_EVT("EVT_ZB: Unhandled LevelControl server event %d.\n", EventData->Event_Type);
            break;
      }

      if(DisplayPrompt)
      {
         QCLI_Display_Prompt();
      }
   }
}

/**
   @brief Handles callbacks for the LevelControl client cluster.

   @param ZB_Handle is the handle of the ZigBee instance.
   @param EventData is the information for the cluster event.
   @param CB_Param  is the user specified parameter for the callback function.
*/
static void ZCL_LevelControl_Demo_Client_CB(qapi_ZB_Handle_t ZB_Handle, qapi_ZB_Cluster_t Cluster, qapi_ZB_CL_LevelControl_Client_Event_Data_t *EventData, uint32_t CB_Param)
{
   if((ZB_Handle != NULL) && (Cluster != NULL) && (EventData != NULL))
   {
      switch(EventData->Event_Type)
      {
         case QAPI_ZB_CL_LEVELCONTROL_CLIENT_EVENT_TYPE_DEFAULT_RESPONSE_E:
            LOG_AT_EVT("EVT_ZB: LevelControl Client Default Response:\n");
            LOG_AT_EVT("EVT_ZB: Status:        %d\n", EventData->Data.Default_Response.Status);
            LOG_AT_EVT("EVT_ZB: CommandID:     0x%02X\n", EventData->Data.Default_Response.CommandId);
            LOG_AT_EVT("EVT_ZB: CommandStatus: %d\n", EventData->Data.Default_Response.CommandStatus);
            break;

         default:
            LOG_AT_EVT("EVT_ZB: Unhandled LevelControl client event %d.\n", EventData->Event_Type);
            break;
      }

      QCLI_Display_Prompt();
   }
}

/**
   @brief Initializes the ZCL LevelControl demo.

   @param ZigBee_QCLI_Handle is the parent QCLI handle for the LevelControl demo.

   @return true if the ZigBee light demo initialized successfully, false
           otherwise.
*/
qbool_t Initialize_ZCL_LevelControl_Demo(QCLI_Group_Handle_t ZigBee_QCLI_Handle)
{
   qbool_t Ret_Val;

   /* Register LevelControl command group. */
   ZigBee_LevelControl_Demo_Context.QCLI_Handle = QCLI_Register_Command_Group(ZigBee_QCLI_Handle, &ZCL_LevelControl_Cmd_Group);
   if(ZigBee_LevelControl_Demo_Context.QCLI_Handle != NULL)
   {
      Ret_Val = true;
   }
   else
   {
      LOG_WARN("Failed to register ZCL LevelControl command group.\n");
      Ret_Val = false;
   }

   return(Ret_Val);
}

/**
   @brief Creates an LevelControl server cluster.

   @param Endpoint   is the endpoint the cluster will be part of.
   @param DeviceName is the string representation of the endpoint the cluster
                     will be part of.

   @return is the ClusterIndex of the newly created cluster or a negative value
           if there was an error.
*/
qapi_ZB_Cluster_t ZCL_LevelControl_Demo_Create_Server(uint8_t Endpoint, void **PrivData)
{
   qapi_ZB_Cluster_t          Ret_Val;
   qapi_Status_t              Result;
   qapi_ZB_Handle_t           ZigBee_Handle;
   qapi_ZB_CL_Cluster_Info_t  ClusterInfo;
   qapi_ZB_CL_Attribute_t     AttributeList[ZCL_DEMO_MAX_ATTRIBUTES_PER_CLUSTER];
   ZCL_Demo_Cluster_Info_t   *OnOffClusterInfo;

   ZigBee_Handle = GetZigBeeHandle();
   if(ZigBee_Handle != NULL)
   {
      memset(&ClusterInfo, 0, sizeof(qapi_ZB_CL_Cluster_Info_t));
      ClusterInfo.Endpoint       = Endpoint;
      ClusterInfo.AttributeCount = ZCL_DEMO_MAX_ATTRIBUTES_PER_CLUSTER;
      ClusterInfo.AttributeList  = AttributeList;

      *PrivData = malloc(sizeof(ZCL_LevelControl_Demo_Data_t));
      if(*PrivData != NULL)
      {
         memset(*PrivData, 0, sizeof(ZCL_LevelControl_Demo_Data_t));

         /* Try to find an on-off cluster on the same endpoint to link with. */
         OnOffClusterInfo = ZCL_FindClusterByEndpoint(Endpoint, QAPI_ZB_CL_CLUSTER_ID_ONOFF, ZCL_DEMO_CLUSTERTYPE_SERVER);
         if(OnOffClusterInfo != NULL)
         {
            Result = qapi_ZB_CL_LevelControl_Populate_Attributes(true, &(ClusterInfo.AttributeCount), (qapi_ZB_CL_Attribute_t*) ClusterInfo.AttributeList);
            if(Result == QAPI_OK)
            {
               Result = qapi_ZB_CL_LevelControl_Create_Server(ZigBee_Handle, &Ret_Val, &ClusterInfo, OnOffClusterInfo->Handle, ZCL_LevelControl_Demo_Server_CB, (uint32_t)*PrivData);
               if(Result != QAPI_OK)
               {
                  LOG_ERR("qapi_ZB_CL_LevelControl_Create_Server", Result);
                  Ret_Val = NULL;
               }
            }
            else
            {
               LOG_ERR("Failed to allocate cluster data.\n");
               Ret_Val = NULL;
            }
         }
         else
         {
            LOG_ERR("Failed to find On/Off cluster on endpoint.\n");
            Ret_Val = NULL;
         }

         if(Ret_Val == NULL)
         {
            free(*PrivData);
            *PrivData = NULL;
         }
      }
      else
      {
         LOG_WARN("ZigBee stack is not initialized.\n");
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
   @brief Creates an LevelControl client cluster.

   @param Endpoint is the endpoint the cluster will be part of.
   @param PrivData is a pointer to the private data for the cluster demo.  This
                   will be initaialized to NULL before the create function is
                   called so can be ignored if the demo has no private data.

   @return The handle for the newly created function or NULL if there was an
           error.
*/
qapi_ZB_Cluster_t ZCL_LevelControl_Demo_Create_Client(uint8_t Endpoint, void **PrivData)
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

      Result = qapi_ZB_CL_LevelControl_Create_Client(ZigBee_Handle, &Ret_Val, &ClusterInfo, ZCL_LevelControl_Demo_Client_CB, 0);
      if(Result != QAPI_OK)
      {
         LOG_ERR("qapi_ZB_CL_LevelControl_Create_Client", Result);
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
qbool_t ZCL_LevelControl_GetScenesData(qapi_ZB_CL_Scenes_Extension_Field_Set_t *ExtData)
{
   qbool_t Ret_Val;

   if(ExtData != NULL)
   {
      ExtData->ClusterId = QAPI_ZB_CL_CLUSTER_ID_LEVEL_CONTROL;
      ExtData->Length    = sizeof(uint8_t);
      ExtData->Data      = &(ZigBee_LevelControl_Demo_Context.LevelSceneData);
      Ret_Val            = true;
   }
   else
   {
      Ret_Val = false;
   }

   return(Ret_Val);
}

