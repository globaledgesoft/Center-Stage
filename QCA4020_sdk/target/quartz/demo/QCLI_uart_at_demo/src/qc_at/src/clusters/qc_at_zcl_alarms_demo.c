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

#include "qapi_zb.h"
#include "qapi_zb_cl.h"
#include "qapi_zb_cl_alarm.h"
#include "qosa_util.h"

#define ALARM_LOG_SIZE           (4)

static QCLI_Group_Handle_t ZCL_Alarms_QCLI_Handle;
extern QCLI_Context_t QCLI_Context;

static void ZCL_Alarms_Demo_Server_CB(qapi_ZB_Handle_t ZB_Handle, qapi_ZB_Cluster_t Cluster, qapi_ZB_CL_Alarm_Server_Event_Data_t *EventData, uint32_t CB_Param);
static void ZCL_Alarms_Demo_Client_CB(qapi_ZB_Handle_t ZB_Handle, qapi_ZB_Cluster_t Cluster, qapi_ZB_CL_Alarm_Client_Event_Data_t *EventData, uint32_t CB_Param);

/* Command list for the ZigBee light demo. */
const QCLI_Command_t ZigBee_Alarms_CMD_List[] =
{
   /* cmd_function                 thread  cmd_string        usage_string                                           description */
   {cmd_ZCL_Alarms_Help,           false,  "Help",           "",                                                    "Display the available ZCL alarm commands."},
   {cmd_ZCL_Alarms_ResetAlarm,     false,  "ResetAlarm",     "[DevId][ClientEndpoint][SourceClusterId][AlarmCode]", "Sends a ResetAlarm command to an Alarms server."},
   {cmd_ZCL_Alarms_ResetAllAlarms, false,  "ResetAllAlarms", "[DevId][ClientEndpoint]",                             "Sends a ResetAllAlarms command to an Alarms server."},
   {cmd_ZCL_Alarms_GetAlarm,       false,  "GetAlarm",       "[DevId][ClientEndpoint]",                             "Sends a GetAlarm command to an Alarms server."},
   {cmd_ZCL_Alarms_ResetAlarmLog,  false,  "ResetAlarmLog",  "[DevId][ClientEndpoint]",                             "Sends a ResetAlarmLog command to an Alarms server."},
   {cmd_ZCL_Alarms_Alarm,          false,  "Alarm",          "[ServerEndpoint][SourceCluster][AlarmCode]",          "Sends an Alarm command to an Alarms client."},
};

const QCLI_Command_Group_t ZCL_Alarms_Cmd_Group = {"Alarms", sizeof(ZigBee_Alarms_CMD_List) / sizeof(QCLI_Command_t), ZigBee_Alarms_CMD_List};

QCLI_Command_Status_t cmd_ZCL_Alarms_Help(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
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
   @brief Executes the "ResetAlarm" command to send a ResetAlarm on command to a
          device.

   Parameter_List[0] Index of the device to send to. Use index zero to use the
                     binding table (if setup).
   Parameter_List[1] Endpoint of the Alarms client cluster to use to send the
                     command.
   Parameter_List[2] ID of the cluster which generated the alarm.
   Parameter_List[3] The Alarm code to reset

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t cmd_ZCL_Alarms_ResetAlarm(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   qapi_Status_t                   Result;
   QCLI_Command_Status_t           Ret_Val;
   ZCL_Demo_Cluster_Info_t        *ClusterInfo;
   qapi_ZB_CL_General_Send_Info_t  SendInfo;
   uint8_t                         DeviceId;
   uint16_t                        SourceClusterId;
   uint8_t                         AlarmCode;

   /* Ensure both the stack is initialized and the switch endpoint. */
   if(GetZigBeeHandle() != NULL)
   {
      Ret_Val = QCLI_STATUS_SUCCESS_E;

      /* Validate the device ID. */
      if((Parameter_Count >= 4) &&
         (Parameter_List[0].Integer_Is_Valid) &&
         (Verify_Integer_Parameter(&(Parameter_List[1]), QAPI_ZB_APS_MIN_ENDPOINT, QAPI_ZB_APS_MAX_ENDPOINT)) &&
         (Verify_Integer_Parameter(&(Parameter_List[2]), 0, 0xFFFF)) &&
         (Verify_Integer_Parameter(&(Parameter_List[3]), 0, 0xFF)))
      {
         DeviceId        = (uint8_t)(Parameter_List[0].Integer_Value);
         ClusterInfo     = ZCL_FindClusterByEndpoint((uint8_t)(Parameter_List[1].Integer_Value), QAPI_ZB_CL_CLUSTER_ID_ALARMS, ZCL_DEMO_CLUSTERTYPE_CLIENT);
         SourceClusterId = (uint16_t)(Parameter_List[2].Integer_Value);
         AlarmCode       = (uint8_t)(Parameter_List[3].Integer_Value);

         if(ClusterInfo != NULL)
         {
            memset(&SendInfo, 0, sizeof(SendInfo));

            /* Format the destination addr. mode, address, and endpoint. */
            if(Format_Send_Info_By_Device(DeviceId, &SendInfo))
            {
               Result = qc_drv_ZB_CL_Alarm_Send_Reset_Alarm(qc_api_get_qc_drv_context(), ClusterInfo->Handle, &SendInfo, SourceClusterId, AlarmCode);
               if(Result == QAPI_OK)
               {
                  LOG_INFO("qc_drv_ZB_CL_Alarm_Send_Reset_Alarm");
               }
               else
               {
                  Ret_Val = QCLI_STATUS_ERROR_E;
                  LOG_ERR("qc_drv_ZB_CL_Alarm_Send_Reset_Alarm", Result);
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
   @brief Executes the "ResetAllAlarms" command to send a ResetAllAlarms on
          command to a device.

   Parameter_List[0] is the index of the device to send to. Use index zero to
                     use the binding table (if setup).
   Parameter_List[1] Endpoint of the Alarms client cluster to use to send the
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
QCLI_Command_Status_t cmd_ZCL_Alarms_ResetAllAlarms(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
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
         DeviceId    = (uint8_t)(Parameter_List[0].Integer_Value);
         ClusterInfo = ZCL_FindClusterByEndpoint((uint8_t)(Parameter_List[1].Integer_Value), QAPI_ZB_CL_CLUSTER_ID_ALARMS, ZCL_DEMO_CLUSTERTYPE_CLIENT);

         if(ClusterInfo != NULL)
         {
            memset(&SendInfo, 0, sizeof(SendInfo));

            /* Format the destination addr. mode, address, and endpoint. */
            if(Format_Send_Info_By_Device(DeviceId, &SendInfo))
            {
               Result = qc_drv_ZB_CL_Alarm_Send_Reset_All_Alarms(qc_api_get_qc_drv_context(), ClusterInfo->Handle, &SendInfo);
               if(Result == QAPI_OK)
               {
                  LOG_INFO("qc_drv_ZB_CL_Alarm_Send_Reset_All_Alarms");
               }
               else
               {
                  Ret_Val = QCLI_STATUS_ERROR_E;
                  LOG_ERR("qc_drv_ZB_CL_Alarm_Send_Reset_All_Alarms", Result);
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
   @brief Executes the "GetAlarm" command to send a GetAlarm on command to a
          device.

   Parameter_List[0] is the index of the device to send to. Use index zero to
                     use the binding table (if setup).
   Parameter_List[1] Endpoint of the Alarms client cluster to use to send the
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
QCLI_Command_Status_t cmd_ZCL_Alarms_GetAlarm(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
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
         DeviceId    = (uint8_t)(Parameter_List[0].Integer_Value);
         ClusterInfo = ZCL_FindClusterByEndpoint((uint8_t)(Parameter_List[1].Integer_Value), QAPI_ZB_CL_CLUSTER_ID_ALARMS, ZCL_DEMO_CLUSTERTYPE_CLIENT);

         if(ClusterInfo != NULL)
         {
            memset(&SendInfo, 0, sizeof(SendInfo));

            /* Format the destination addr. mode, address, and endpoint. */
            if(Format_Send_Info_By_Device(DeviceId, &SendInfo))
            {
               Result = qc_drv_ZB_CL_Alarm_Send_Get_Alarm(qc_api_get_qc_drv_context(), ClusterInfo->Handle, &SendInfo);
               if(Result == QAPI_OK)
               {
                  LOG_INFO("qc_drv_ZB_CL_Alarm_Send_Get_Alarm");
               }
               else
               {
                  Ret_Val = QCLI_STATUS_ERROR_E;
                  LOG_ERR("qc_drv_ZB_CL_Alarm_Send_Get_Alarm", Result);
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
   @brief Executes the "ResetAlarmLog" command to send a ResetAlarmLog on
          command to a device.

   Parameter_List[0] is the index of the device to send to. Use index zero to
                     use the binding table (if setup).
   Parameter_List[1] Endpoint of the Alarms client cluster to use to send the
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
QCLI_Command_Status_t cmd_ZCL_Alarms_ResetAlarmLog(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
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
         DeviceId    = (uint8_t)(Parameter_List[0].Integer_Value);
         ClusterInfo = ZCL_FindClusterByEndpoint((uint8_t)(Parameter_List[1].Integer_Value), QAPI_ZB_CL_CLUSTER_ID_ALARMS, ZCL_DEMO_CLUSTERTYPE_CLIENT);

         if(ClusterInfo != NULL)
         {
            memset(&SendInfo, 0, sizeof(SendInfo));

            /* Format the destination addr. mode, address, and endpoint. */
            if(Format_Send_Info_By_Device(DeviceId, &SendInfo))
            {
               Result = qc_drv_ZB_CL_Alarm_Send_Reset_Alarm_Log(qc_api_get_qc_drv_context(), ClusterInfo->Handle, &SendInfo);
               if(Result == QAPI_OK)
               {
                  LOG_INFO("qc_drv_ZB_CL_Alarm_Send_Reset_Alarm_Log");
               }
               else
               {
                  Ret_Val = QCLI_STATUS_ERROR_E;
                  LOG_ERR("qc_drv_ZB_CL_Alarm_Send_Reset_Alarm_Log", Result);
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
   @brief Executes the "Alarm" command to send an alarm to a client device.

   Parameter_List[0] Endpoint of the Alarms client cluster to use to send the
                     command.
   Parameter_List[1] Index of the Source cluster for the alarm.
   Parameter_List[2] The Alarm code to reset

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t cmd_ZCL_Alarms_Alarm(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   qapi_Status_t                   Result;
   QCLI_Command_Status_t           Ret_Val;
   ZCL_Demo_Cluster_Info_t        *ClusterInfo;
   ZCL_Demo_Cluster_Info_t        *SourceClusterInfo;
   uint8_t                         AlarmCode;

   /* Ensure both the stack is initialized and the switch endpoint. */
   if(GetZigBeeHandle() != NULL)
   {
      Ret_Val = QCLI_STATUS_SUCCESS_E;

      /* Validate the device ID. */
      if((Parameter_Count >= 3) &&
         (Verify_Integer_Parameter(&(Parameter_List[0]), QAPI_ZB_APS_MIN_ENDPOINT, QAPI_ZB_APS_MAX_ENDPOINT)) &&
         (Verify_Integer_Parameter(&(Parameter_List[1]), 0, 0xFFFF)) &&
         (Verify_Integer_Parameter(&(Parameter_List[2]), 0, 0xFF)))
      {
         ClusterInfo       = ZCL_FindClusterByEndpoint((uint8_t)(Parameter_List[0].Integer_Value), QAPI_ZB_CL_CLUSTER_ID_ALARMS, ZCL_DEMO_CLUSTERTYPE_SERVER);
         SourceClusterInfo = ZCL_FindClusterByIndex((uint16_t)(Parameter_List[1].Integer_Value), ZCL_DEMO_IGNORE_CLUSTER_ID);
         AlarmCode         = (uint8_t)(Parameter_List[2].Integer_Value);

         if((ClusterInfo != NULL) && (SourceClusterInfo != NULL))
         {
            Result = qc_drv_ZB_CL_Alarm_Send_Alarm(qc_api_get_qc_drv_context(), ClusterInfo->Handle, SourceClusterInfo->Handle, AlarmCode);
            if(Result == QAPI_OK)
            {
               LOG_INFO("qc_drv_ZB_CL_Alarm_Send_Alarm");
            }
            else
            {
               Ret_Val = QCLI_STATUS_ERROR_E;
               LOG_ERR("qc_drv_ZB_CL_Alarm_Send_Alarm", Result);
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
   @brief Handles callbacks for the Alarms server cluster.

   @param ZB_Handle is the handle of the ZigBee instance.
   @param EventData is the information for the cluster event.
   @param CB_Param  is the user specified parameter for the callback function.
*/
static void ZCL_Alarms_Demo_Server_CB(qapi_ZB_Handle_t ZB_Handle, qapi_ZB_Cluster_t Cluster, qapi_ZB_CL_Alarm_Server_Event_Data_t *EventData, uint32_t CB_Param)
{
   if((ZB_Handle != NULL) && (Cluster != NULL) && (EventData != NULL))
   {
      LOG_AT_EVT("EVT_ZB: Unhandled Alarms server event %d.\n", EventData->Event_Type);
      QCLI_Display_Prompt();
   }
}

/**
   @brief Handles callbacks for the Alarms client cluster.

   @param ZB_Handle is the handle of the ZigBee instance.
   @param EventData is the information for the cluster event.
   @param CB_Param  is the user specified parameter for the callback function.
*/
static void ZCL_Alarms_Demo_Client_CB(qapi_ZB_Handle_t ZB_Handle, qapi_ZB_Cluster_t Cluster, qapi_ZB_CL_Alarm_Client_Event_Data_t *EventData, uint32_t CB_Param)
{
   if((ZB_Handle != NULL) && (Cluster != NULL) && (EventData != NULL))
   {
      switch(EventData->Event_Type)
      {
         case QAPI_ZB_CL_ALARM_CLIENT_EVENT_TYPE_DEFAULT_RESPONSE_E:
            LOG_AT_EVT("EVT_ZB: Alarms Client Default Response:\n");
            LOG_AT_EVT("EVT_ZB: Status:        %d\n", EventData->Data.Default_Response.Status);
            LOG_AT_EVT("EVT_ZB: CommandID:     0x%02X\n", EventData->Data.Default_Response.CommandId);
            LOG_AT_EVT("EVT_ZB: CommandStatus: %d\n", EventData->Data.Default_Response.CommandStatus);
            break;

         case QAPI_ZB_CL_ALARM_CLIENT_EVENT_TYPE_ALARM_E:
            LOG_AT_EVT("EVT_ZB: Alarm:\n");
            LOG_AT_EVT("EVT_ZB: ClusterId: %d\n", EventData->Data.Alarm_Command.ClusterId);
            LOG_AT_EVT("EVT_ZB: AlarmCode: %d\n", EventData->Data.Alarm_Command.AlarmCode);
            break;

         case QAPI_ZB_CL_ALARM_CLIENT_EVENT_TYPE_GET_ALARM_RESPONSE_E:
            LOG_AT_EVT("EVT_ZB: Get Alarm Response:\n");
            LOG_AT_EVT("EVT_ZB: Status:    %d\n", EventData->Data.Get_Alarm_Response.Status);
            LOG_AT_EVT("EVT_ZB: ClusterId: %d\n", EventData->Data.Get_Alarm_Response.ClusterId);
            LOG_AT_EVT("EVT_ZB: AlarmCode: %d\n", EventData->Data.Get_Alarm_Response.AlarmCode);
            LOG_AT_EVT("EVT_ZB: Timestamp: %d\n", EventData->Data.Get_Alarm_Response.TimeStamp);
            break;

         default:
            LOG_AT_EVT("EVT_ZB: Unhandled Alarms client event %d.\n", EventData->Event_Type);
            break;
      }

      QCLI_Display_Prompt();
   }
}

/**
   @brief Initializes the ZCL Alarms demo.

   @param ZigBee_QCLI_Handle is the parent QCLI handle for the Alarms demo.

   @return true if the ZigBee light demo initialized successfully, false
           otherwise.
*/
qbool_t Initialize_ZCL_Alarms_Demo(QCLI_Group_Handle_t ZigBee_QCLI_Handle)
{
   qbool_t Ret_Val;

   /* Register Alarms command group. */
   ZCL_Alarms_QCLI_Handle = QCLI_Register_Command_Group(ZigBee_QCLI_Handle, &ZCL_Alarms_Cmd_Group);
   if(ZCL_Alarms_QCLI_Handle != NULL)
   {
      Ret_Val = true;
   }
   else
   {
      LOG_ERR("Failed to register ZCL Alarms command group.\n");
      Ret_Val = false;
   }

   return(Ret_Val);
}

/**
   @brief Creates an Alarms server cluster.

   @param Endpoint is the endpoint the cluster will be part of.
   @param PrivData is a pointer to the private data for the cluster demo.  This
                   will be initaialized to NULL before the create function is
                   called so can be ignored if the demo has no private data.

   @return The handle for the newly created function or NULL if there was an
           error.
*/
qapi_ZB_Cluster_t ZCL_Alarms_Demo_Create_Server(uint8_t Endpoint, void **PrivData)
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
      Result = qapi_ZB_CL_Alarm_Populate_Attributes(true, &(ClusterInfo.AttributeCount), (qapi_ZB_CL_Attribute_t*) ClusterInfo.AttributeList);
      if(Result == QAPI_OK)
      {
         Result = qapi_ZB_CL_Alarm_Create_Server(ZigBee_Handle, &Ret_Val, &ClusterInfo, ALARM_LOG_SIZE, ZCL_Alarms_Demo_Server_CB, 0);
         if(Result != QAPI_OK)
         {
            LOG_INFO("qc_drv_ZB_CL_Alarm_Create_Server", Result);
            Ret_Val = NULL;
         }
      }
      else
      {
         LOG_INFO("qc_drv_ZB_CL_Alarm_Populate_Attributes", Result);
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
   @brief Creates an Alarms client cluster.

   @param Endpoint is the endpoint the cluster will be part of.
   @param PrivData is a pointer to the private data for the cluster demo.  This
                   will be initaialized to NULL before the create function is
                   called so can be ignored if the demo has no private data.

   @return The handle for the newly created function or NULL if there was an
           error.
*/
qapi_ZB_Cluster_t ZCL_Alarms_Demo_Create_Client(uint8_t Endpoint, void **PrivData)
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

      Result = qapi_ZB_CL_Alarm_Create_Client(ZigBee_Handle, &Ret_Val, &ClusterInfo, ZCL_Alarms_Demo_Client_CB, 0);
      if(Result != QAPI_OK)
      {
         LOG_INFO("qc_drv_ZB_CL_Alarm_Create_Client", Result);
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

