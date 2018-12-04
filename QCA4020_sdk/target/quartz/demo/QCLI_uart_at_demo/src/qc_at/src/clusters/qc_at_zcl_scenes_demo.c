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
#include "zcl_scenes_demo.h"
#include "zcl_onoff_demo.h"
#include "zcl_levelcontrol_demo.h"
#include "zcl_colorcontrol_demo.h"

#include "qapi_zb.h"
#include "qapi_zb_cl.h"
#include "qapi_zb_cl_scenes.h"
#include "qc_api_main.h"
#include "qosa_util.h"

#define ZCL_SCENES_DEMO_MAX_SCENES                                      (4)

static QCLI_Group_Handle_t ZCL_Scenes_QCLI_Handle;
extern QCLI_Context_t QCLI_Context;
/* Structure to describe a cluster that contains scenes data. */
typedef struct ZCL_Scenes_Cluster_Descriptor_s
{
   uint16_t                  ClusterID;      /* ID of the cluster. */
   ZCL_Scenes_GetData_Func_t GetExtDataFunc; /* Function called to get the scenes data for the cluster. */
} ZCL_Scenes_Cluster_Descriptor_t;

/* The list of clusters that are scene capable. */
static const ZCL_Scenes_Cluster_Descriptor_t SceneClusterDescriptorList[] =
{
   {QAPI_ZB_CL_CLUSTER_ID_ONOFF,         ZCL_OnOff_GetScenesData},
   {QAPI_ZB_CL_CLUSTER_ID_LEVEL_CONTROL, ZCL_LevelControl_GetScenesData},
   {QAPI_ZB_CL_CLUSTER_ID_COLOR_CONTROL, ZCL_ColorControl_GetScenesData}
};

#define SCENES_CLUSTER_DESCRIPTOR_LIST_SIZE     (sizeof(SceneClusterDescriptorList) / sizeof(ZCL_Scenes_Cluster_Descriptor_t))

/* Function prototypes. */
static QCLI_Command_Status_t ZCL_Scene_PopulateSceneData(uint8_t Endpoint, uint8_t *ExtFieldCount, qapi_ZB_CL_Scenes_Extension_Field_Set_t *ExtFieldSet);
static void ZCL_Scenes_Demo_Server_CB(qapi_ZB_Handle_t ZB_Handle, qapi_ZB_Cluster_t Cluster, qapi_ZB_CL_Scenes_Server_Event_Data_t *EventData, uint32_t CB_Param);
static void ZCL_Scenes_Demo_Client_CB(qapi_ZB_Handle_t ZB_Handle, qapi_ZB_Cluster_t Cluster, qapi_ZB_CL_Scenes_Client_Event_Data_t *EventData, uint32_t CB_Param);

/* Command list for the ZigBee light demo. */
static const QCLI_Command_t ZigBee_Scenes_CMD_List[] =
{
   /* cmd_function                     thread  cmd_string            usage_string                                                                    description */
   {cmd_ZCL_Scenes_Help,               false,  "Help",               "",                                                                             "Display the all ZCL scenes commands."},
   {cmd_ZCL_Scenes_AddScene,           false,  "Add",           "[DevId][ClientEndpoint][GroupID][SceneID][SceneName][TransTime][IsEnhanced]",  "Sends an Add Scene command to a Scenes server."},
   {cmd_ZCL_Scenes_ViewScene,          false,  "view",          "[DevId][ClientEndpoint][GroupID][SceneID][IsEnhanced]",                        "Sends a View Scene command to a Scenes server."},
   {cmd_ZCL_Scenes_RemoveScene,        false,  "remove",        "[DevId][ClientEndpoint][GroupID][SceneID]",                                    "Sends a Remove Scene command to a Scenes server."},
   {cmd_ZCL_Scenes_RemoveAllScenes,    false,  "RemAllScenes",    "[DevId][ClientEndpoint][GroupID]",                                             "Sends a Remove All Scenes command to a Scenes server."},
   {cmd_ZCL_Scenes_StoreScenes,        false,  "store",        "[DevId][ClientEndpoint][GroupID][SceneID]",                                    "Sends a Store Scene command to a Scenes server."},
   {cmd_ZCL_Scenes_RecallScenes,       false,  "RecallScenes",       "[DevId][ClientEndpoint][GroupID][SceneID]",                                    "Sends a Recall Scene command to a Scenes server."},
   {cmd_ZCL_Scenes_GetSceneMembership, false,  "GetSceneMem", "[DevId][ClientEndpoint][GroupID]",                                             "Sends a Get Scene Membership command to a Scenes server."},
   {cmd_ZCL_Scenes_CopyScene,          false,  "CopyScene",          "[DevId][ClientEndpoint][CopyAll][GroupFrom][SceneFrom][GroupTo][SceneTo]",     "Sends a Copy Scene command to a Scenes server."}
};

const QCLI_Command_Group_t ZCL_Scenes_Cmd_Group = {"Scenes", sizeof(ZigBee_Scenes_CMD_List) / sizeof(QCLI_Command_t), ZigBee_Scenes_CMD_List};

/**
   @brief Populate the extention field set.

   @param[in]  Endpoint      is the end point possibely holds the scene capable
                             clusters
   @param[out] ExtFieldCount is the number of the extension field.
   @paran[in]  ExtFieldSet   is the extension field set.

   @return
     - QAPI_OK    if the function executed successfully.
     - QAPI_ERROR if there was an error.
*/
static QCLI_Command_Status_t ZCL_Scene_PopulateSceneData(uint8_t Endpoint, uint8_t *ExtFieldCount, qapi_ZB_CL_Scenes_Extension_Field_Set_t *ExtFieldSet)
{
   QCLI_Command_Status_t    Ret_Val;
   ZCL_Demo_Cluster_Info_t *ClusterInfo;
   uint8_t                  Index;
   uint8_t                  LocalFieldCount;

   Ret_Val         = QCLI_STATUS_SUCCESS_E;
   LocalFieldCount = 0;

   /* Go throught the SceneClusterDescriptorList to check if there is a scene
      capable cluster on the given endpoint. */
   for(Index = 0; (Index < SCENES_CLUSTER_DESCRIPTOR_LIST_SIZE) && (Ret_Val == QCLI_STATUS_SUCCESS_E); Index ++)
   {
      ClusterInfo = ZCL_FindClusterByEndpoint(Endpoint, SceneClusterDescriptorList[Index].ClusterID, ZCL_DEMO_CLUSTERTYPE_CLIENT);
      if(ClusterInfo != NULL)
      {
         if(LocalFieldCount < *ExtFieldCount)
         {
            /* The scene capable cluster is found on the given endpoint. Updtae
               the value ExtFieldCount and populate the ExtFieldSet.*/
            if((*(SceneClusterDescriptorList[Index].GetExtDataFunc))(ExtFieldSet))
            {
               /* Adjust LocalExtFieldSet to point to the next entry. */
               LocalFieldCount ++;
               ExtFieldSet ++;
            }
            else
            {
               LOG_ERR("Failed to get scenes data for cluster %d.\n", SceneClusterDescriptorList[Index].ClusterID);
               Ret_Val = QCLI_STATUS_ERROR_E;
            }
         }
         else
         {
            LOG_ERR("ExtField overflowed.\n");
            Ret_Val = QCLI_STATUS_ERROR_E;
         }
      }
   }

   if(Ret_Val == QCLI_STATUS_SUCCESS_E)
   {
      *ExtFieldCount = LocalFieldCount;
   }

   return(Ret_Val);
}

QCLI_Command_Status_t cmd_ZCL_Scenes_Help(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
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
   @brief Executes the "AddScene" command add a scene.

   Parameter_List[0] ID of the device to send the command to.
   Parameter_List[1] is the endpoint the scenes client cluster is on.
   Parameter_List[2] is the group ID.
   Parameter_List[3] is the scene ID.
   Parameter_List[4] is scene name.
   Parameter_List[5] is the trans time.
   Parameter_List[6] is the flag indicates if the enhanced add scene will be
                     used.
                     0=No, 1=Yes.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t cmd_ZCL_Scenes_AddScene(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t                    Ret_Val;
   qapi_Status_t                            Result;
   qapi_ZB_CL_General_Send_Info_t           SendInfo;
   ZCL_Demo_Cluster_Info_t                 *SceneCluster;
   uint8_t                                  DeviceId;
   uint8_t                                  Endpoint;
   qapi_ZB_CL_Scenes_Extension_Field_Set_t  ExtFieldSet[SCENES_CLUSTER_DESCRIPTOR_LIST_SIZE];
   qapi_ZB_CL_Scenes_Add_Scene_t            Request;

   /* Ensure both the stack is initialized. */
   if(GetZigBeeHandle() != NULL)
   {
      Ret_Val = QCLI_STATUS_SUCCESS_E;

      if((Parameter_Count >= 7) &&
         (Parameter_List[0].Integer_Is_Valid) &&
         (Verify_Integer_Parameter(&(Parameter_List[1]), QAPI_ZB_APS_MIN_ENDPOINT, QAPI_ZB_APS_MAX_ENDPOINT)) &&
         (Verify_Integer_Parameter(&(Parameter_List[2]), 0, 0xFFFF)) &&
         (Verify_Integer_Parameter(&(Parameter_List[3]), 0, 0xFF)) &&
         (Verify_Integer_Parameter(&(Parameter_List[5]), 0, 0xFFFF)) &&
         (Verify_Integer_Parameter(&(Parameter_List[6]), 0, 1)))
      {
         DeviceId = (uint8_t)(Parameter_List[0].Integer_Value);
         Endpoint = (uint8_t)(Parameter_List[1].Integer_Value);

         /* Determine if the scene client cluster is on the given endpoint. */
         SceneCluster = ZCL_FindClusterByEndpoint(Endpoint, QAPI_ZB_CL_CLUSTER_ID_SCENES, ZCL_DEMO_CLUSTERTYPE_CLIENT);
         if(SceneCluster != NULL)
         {
            /* Set the Request to a known state. */
            memset(&Request, 0, sizeof(qapi_ZB_CL_Scenes_Add_Scene_t));

            /* Set up ExtensionFieldCount and populate the ExtFieldSet. */
            Request.ExtensionFieldCount = SCENES_CLUSTER_DESCRIPTOR_LIST_SIZE;
            Ret_Val = ZCL_Scene_PopulateSceneData(Endpoint, &(Request.ExtensionFieldCount), ExtFieldSet);
            if(Ret_Val == QCLI_STATUS_SUCCESS_E)
            {
               /* Setup the request. */
               Request.GroupId            = (uint16_t)(Parameter_List[2].Integer_Value);
               Request.SceneId            = (uint8_t)(Parameter_List[3].Integer_Value);
               Request.SceneName          = (uint8_t *)(Parameter_List[4].String_Value);
               Request.TransitionTime     = (uint16_t)(Parameter_List[5].Integer_Value);
               Request.IsEnhanced         = (qbool_t)(Parameter_List[6].Integer_Value != 0);
               Request.ExtensionFieldSets = ExtFieldSet;

               /* Format the destination info. */
               memset(&SendInfo, 0, sizeof(qapi_ZB_CL_General_Send_Info_t));
               if(Format_Send_Info_By_Device(DeviceId, &SendInfo))
               {
                  Result = qc_drv_ZB_CL_Scenes_Send_Add_Scene(qc_api_get_qc_drv_context(), SceneCluster->Handle, &SendInfo, &Request);
                  if(Result == QAPI_OK)
                  {
                     LOG_INFO("qc_drv_ZB_CL_Scenes_Send_Add_Scene");
                  }
                  else
                  {
                     Ret_Val = QCLI_STATUS_ERROR_E;
                     LOG_ERR("qc_drv_ZB_CL_Scenes_Send_Add_Scene", Result);
                  }
               }
               else
               {
                  LOG_ERR("Invalid device ID.\n");
                  Ret_Val = QCLI_STATUS_ERROR_E;
               }
            }
         }
         else
         {
            LOG_ERR("Scene client cluster is not found on the given endpoint.\n");
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
      LOG_WARN("Zigbee stack is not initialized.\n");
      Ret_Val = QCLI_STATUS_ERROR_E;
   }

   return(Ret_Val);
}

/**
   @brief Executes the "ViewScene" command to view a scene.

   Parameter_List[0] ID of the device to send the command to.
   Parameter_List[1] Endpoint of the Scenes client cluster to use to send the
                     command.
   Parameter_List[2] ID of the group of the scene to view.
   Parameter_List[3] ID of the scene to view.
   Parameter_List[4] Flag indicating if this is an enhanced view scenes command.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t cmd_ZCL_Scenes_ViewScene(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t           Ret_Val;
   qapi_Status_t                   Result;
   qapi_ZB_CL_General_Send_Info_t  SendInfo;
   ZCL_Demo_Cluster_Info_t        *ClusterInfo;
   uint32_t                        DeviceId;
   uint16_t                        GroupId;
   uint8_t                         SceneId;
   qbool_t                         IsEnhanced;

   /* Ensure both the stack is initialized. */
   if(GetZigBeeHandle() != NULL)
   {
      Ret_Val = QCLI_STATUS_SUCCESS_E;

      if((Parameter_Count >= 5) &&
         (Parameter_List[0].Integer_Is_Valid) &&
         (Verify_Integer_Parameter(&(Parameter_List[1]), QAPI_ZB_APS_MIN_ENDPOINT, QAPI_ZB_APS_MAX_ENDPOINT)) &&
         (Verify_Integer_Parameter(&(Parameter_List[2]), 0, 0xFFFF)) &&
         (Verify_Integer_Parameter(&(Parameter_List[3]), 0, 0xFF)) &&
         (Verify_Integer_Parameter(&(Parameter_List[4]), 0, 1)))
      {
         DeviceId    = Parameter_List[0].Integer_Value;
         ClusterInfo = ZCL_FindClusterByEndpoint((uint8_t)(Parameter_List[1].Integer_Value), QAPI_ZB_CL_CLUSTER_ID_SCENES, ZCL_DEMO_CLUSTERTYPE_CLIENT);
         GroupId     = (uint16_t)(Parameter_List[2].Integer_Value);
         SceneId     = (uint8_t)(Parameter_List[3].Integer_Value);
         IsEnhanced  = (qbool_t)(Parameter_List[4].Integer_Value != 0);

         if(ClusterInfo != NULL)
         {
            /* Format the destination info. */
            memset(&SendInfo, 0, sizeof(qapi_ZB_CL_General_Send_Info_t));
            if(Format_Send_Info_By_Device(DeviceId, &SendInfo))
            {
               Result = qc_drv_ZB_CL_Scenes_Send_View_Scene(qc_api_get_qc_drv_context(), ClusterInfo->Handle, &SendInfo, GroupId, SceneId, IsEnhanced);
               if(Result == QAPI_OK)
               {
                  LOG_INFO("qc_drv_ZB_CL_Scenes_Send_View_Scene");
               }
               else
               {
                  Ret_Val = QCLI_STATUS_ERROR_E;
                  LOG_ERR("qc_drv_ZB_CL_Scenes_Send_View_Scene", Result);
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
      LOG_WARN("Zigbee stack is not initialized.\n");
      Ret_Val = QCLI_STATUS_ERROR_E;
   }

   return(Ret_Val);
}

/**
   @brief Executes the "RemoveScene" command to remove a scene.

   Parameter_List[0] ID of the device to send the command to.
   Parameter_List[1] Endpoint of the Scenes client cluster to use to send the
                     command.
   Parameter_List[2] ID of the group of the scene to remove.
   Parameter_List[3] ID of the scene to remove.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t cmd_ZCL_Scenes_RemoveScene(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t           Ret_Val;
   qapi_Status_t                   Result;
   qapi_ZB_CL_General_Send_Info_t  SendInfo;
   ZCL_Demo_Cluster_Info_t        *ClusterInfo;
   uint32_t                        DeviceId;
   uint16_t                        GroupId;
   uint8_t                         SceneId;

   /* Ensure both the stack is initialized. */
   if(GetZigBeeHandle() != NULL)
   {
      Ret_Val = QCLI_STATUS_SUCCESS_E;

      if((Parameter_Count >= 4) &&
         (Parameter_List[0].Integer_Is_Valid) &&
         (Verify_Integer_Parameter(&(Parameter_List[1]), QAPI_ZB_APS_MIN_ENDPOINT, QAPI_ZB_APS_MAX_ENDPOINT)) &&
         (Verify_Integer_Parameter(&(Parameter_List[2]), 0, 0xFFFF)) &&
         (Verify_Integer_Parameter(&(Parameter_List[3]), 0, 0xFF)))
      {
         DeviceId    = Parameter_List[0].Integer_Value;
         ClusterInfo = ZCL_FindClusterByEndpoint((uint8_t)(Parameter_List[1].Integer_Value), QAPI_ZB_CL_CLUSTER_ID_SCENES, ZCL_DEMO_CLUSTERTYPE_CLIENT);
         GroupId     = (uint16_t)(Parameter_List[2].Integer_Value);
         SceneId     = (uint8_t)(Parameter_List[3].Integer_Value);

         if(ClusterInfo != NULL)
         {
            /* Format the destination info. */
            memset(&SendInfo, 0, sizeof(qapi_ZB_CL_General_Send_Info_t));
            if(Format_Send_Info_By_Device(DeviceId, &SendInfo))
            {
               Result = qc_drv_ZB_CL_Scenes_Send_Remove_Scene(qc_api_get_qc_drv_context(), ClusterInfo->Handle, &SendInfo, GroupId, SceneId);
               if(Result == QAPI_OK)
               {
                  LOG_INFO("qc_drv_ZB_CL_Scenes_Send_Remove_Scene");
               }
               else
               {
                  Ret_Val = QCLI_STATUS_ERROR_E;
                  LOG_ERR("qc_drv_ZB_CL_Scenes_Send_Remove_Scene", Result);
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
      LOG_WARN("Zigbee stack is not initialized.\n");
      Ret_Val = QCLI_STATUS_ERROR_E;
   }

   return(Ret_Val);
}

/**
   @brief Executes the "RemoveAllScenes" command to all scenes.

   Parameter_List[0] ID of the device to send the command to.
   Parameter_List[1] Endpoint of the Scenes client cluster to use to send the
                     command.
   Parameter_List[2] ID of the group of the scenes to remove.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t cmd_ZCL_Scenes_RemoveAllScenes(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t           Ret_Val;
   qapi_Status_t                   Result;
   qapi_ZB_CL_General_Send_Info_t  SendInfo;
   ZCL_Demo_Cluster_Info_t        *ClusterInfo;
   uint32_t                        DeviceId;
   uint16_t                        GroupId;

   /* Ensure both the stack is initialized. */
   if(GetZigBeeHandle() != NULL)
   {
      Ret_Val = QCLI_STATUS_SUCCESS_E;

      if((Parameter_Count >= 3) &&
         (Parameter_List[0].Integer_Is_Valid) &&
         (Verify_Integer_Parameter(&(Parameter_List[1]), QAPI_ZB_APS_MIN_ENDPOINT, QAPI_ZB_APS_MAX_ENDPOINT)) &&
         (Verify_Integer_Parameter(&(Parameter_List[2]), 0, 0xFFFF)))
      {
         DeviceId    = Parameter_List[0].Integer_Value;
         ClusterInfo = ZCL_FindClusterByEndpoint((uint8_t)(Parameter_List[1].Integer_Value), QAPI_ZB_CL_CLUSTER_ID_SCENES, ZCL_DEMO_CLUSTERTYPE_CLIENT);
         GroupId     = (uint16_t)(Parameter_List[2].Integer_Value);

         if(ClusterInfo != NULL)
         {
            /* Format the destination info. */
            memset(&SendInfo, 0, sizeof(qapi_ZB_CL_General_Send_Info_t));
            if(Format_Send_Info_By_Device(DeviceId, &SendInfo))
            {
               Result = qc_drv_ZB_CL_Scenes_Send_Remove_All_Scenes(qc_api_get_qc_drv_context(), ClusterInfo->Handle, &SendInfo, GroupId);
               if(Result == QAPI_OK)
               {
                  LOG_INFO("qc_drv_ZB_CL_Scenes_Send_Remove_All_Scenes");
               }
               else
               {
                  Ret_Val = QCLI_STATUS_ERROR_E;
                  LOG_ERR("qc_drv_ZB_CL_Scenes_Send_Remove_All_Scenes", Result);
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
      LOG_WARN("Zigbee stack is not initialized.\n");
      Ret_Val = QCLI_STATUS_ERROR_E;
   }

   return(Ret_Val);
}

/**
   @brief Executes the "StoreScene" command to store a scene.

   Parameter_List[0] ID of the device to send the command to.
   Parameter_List[1] Endpoint of the Scenes client cluster to use to send the
                     command.
   Parameter_List[2] ID of the group of the scene to store.
   Parameter_List[3] ID of the scene to store.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t cmd_ZCL_Scenes_StoreScenes(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t           Ret_Val;
   qapi_Status_t                   Result;
   qapi_ZB_CL_General_Send_Info_t  SendInfo;
   ZCL_Demo_Cluster_Info_t        *ClusterInfo;
   uint32_t                        DeviceId;
   uint16_t                        GroupId;
   uint8_t                         SceneId;

   /* Ensure both the stack is initialized. */
   if(GetZigBeeHandle() != NULL)
   {
      Ret_Val = QCLI_STATUS_SUCCESS_E;

      if((Parameter_Count >= 4) &&
         (Parameter_List[0].Integer_Is_Valid) &&
         (Verify_Integer_Parameter(&(Parameter_List[1]), QAPI_ZB_APS_MIN_ENDPOINT, QAPI_ZB_APS_MAX_ENDPOINT)) &&
         (Verify_Integer_Parameter(&(Parameter_List[2]), 0, 0xFFFF)) &&
         (Verify_Integer_Parameter(&(Parameter_List[3]), 0, 0xFF)))
      {
         DeviceId    = Parameter_List[0].Integer_Value;
         ClusterInfo = ZCL_FindClusterByEndpoint((uint8_t)(Parameter_List[1].Integer_Value), QAPI_ZB_CL_CLUSTER_ID_SCENES, ZCL_DEMO_CLUSTERTYPE_CLIENT);
         GroupId     = (uint16_t)(Parameter_List[2].Integer_Value);
         SceneId     = (uint8_t)(Parameter_List[3].Integer_Value);

         if(ClusterInfo != NULL)
         {
            /* Format the destination info. */
            memset(&SendInfo, 0, sizeof(qapi_ZB_CL_General_Send_Info_t));
            if(Format_Send_Info_By_Device(DeviceId, &SendInfo))
            {
               Result = qc_drv_ZB_CL_Scenes_Send_Store_Scene(qc_api_get_qc_drv_context(), ClusterInfo->Handle, &SendInfo, GroupId, SceneId);
               if(Result == QAPI_OK)
               {
                  LOG_INFO("qc_drv_ZB_CL_Scenes_Send_Store_Scene");
               }
               else
               {
                  Ret_Val = QCLI_STATUS_ERROR_E;
                  LOG_ERR("qc_drv_ZB_CL_Scenes_Send_Store_Scene", Result);
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
      LOG_WARN("Zigbee stack is not initialized.\n");
      Ret_Val = QCLI_STATUS_ERROR_E;
   }

   return(Ret_Val);
}

/**
   @brief Executes the "RecallScene" command to recall a scene.

   Parameter_List[0] ID of the device to send the command to.
   Parameter_List[1] Endpoint of the Scenes client cluster to use to send the
                     command.
   Parameter_List[2] ID of the group of the scene to recall.
   Parameter_List[3] ID of the scene to recall.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t cmd_ZCL_Scenes_RecallScenes(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t           Ret_Val;
   qapi_Status_t                   Result;
   qapi_ZB_CL_General_Send_Info_t  SendInfo;
   ZCL_Demo_Cluster_Info_t        *ClusterInfo;
   uint32_t                        DeviceId;
   uint16_t                        GroupId;
   uint8_t                         SceneId;

   /* Ensure both the stack is initialized. */
   if(GetZigBeeHandle() != NULL)
   {
      Ret_Val = QCLI_STATUS_SUCCESS_E;

      if((Parameter_Count >= 4) &&
         (Parameter_List[0].Integer_Is_Valid) &&
         (Verify_Integer_Parameter(&(Parameter_List[1]), QAPI_ZB_APS_MIN_ENDPOINT, QAPI_ZB_APS_MAX_ENDPOINT)) &&
         (Verify_Integer_Parameter(&(Parameter_List[2]), 0, 0xFFFF)) &&
         (Verify_Integer_Parameter(&(Parameter_List[3]), 0, 0xFF)))
      {
         DeviceId    = Parameter_List[0].Integer_Value;
         ClusterInfo = ZCL_FindClusterByEndpoint((uint8_t)(Parameter_List[1].Integer_Value), QAPI_ZB_CL_CLUSTER_ID_SCENES, ZCL_DEMO_CLUSTERTYPE_CLIENT);
         GroupId     = (uint16_t)(Parameter_List[2].Integer_Value);
         SceneId     = (uint8_t)(Parameter_List[3].Integer_Value);

         if(ClusterInfo != NULL)
         {
            /* Format the destination info. */
            memset(&SendInfo, 0, sizeof(qapi_ZB_CL_General_Send_Info_t));
            if(Format_Send_Info_By_Device(DeviceId, &SendInfo))
            {
               Result = qc_drv_ZB_CL_Scenes_Send_Recall_Scene(qc_api_get_qc_drv_context(), ClusterInfo->Handle, &SendInfo, GroupId, SceneId);
               if(Result == QAPI_OK)
               {
                  LOG_INFO("qc_drv_ZB_CL_Scenes_Send_Recall_Scene");
               }
               else
               {
                  Ret_Val = QCLI_STATUS_ERROR_E;
                  LOG_ERR("qc_drv_ZB_CL_Scenes_Send_Recall_Scene", Result);
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
      LOG_WARN("Zigbee stack is not initialized.\n");
      Ret_Val = QCLI_STATUS_ERROR_E;
   }

   return(Ret_Val);
}

/**
   @brief Executes the "GetSceneMembership" command to get the membership of a
          scene.

   Parameter_List[0] ID of the device to send the command to.
   Parameter_List[1] Endpoint of the Scenes client cluster to use to send the
                     command.
   Parameter_List[2] ID of the group to get scene membership for.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t cmd_ZCL_Scenes_GetSceneMembership(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t           Ret_Val;
   qapi_Status_t                   Result;
   qapi_ZB_CL_General_Send_Info_t  SendInfo;
   ZCL_Demo_Cluster_Info_t        *ClusterInfo;
   uint32_t                        DeviceId;
   uint16_t                        GroupId;

   /* Ensure both the stack is initialized. */
   if(GetZigBeeHandle() != NULL)
   {
      Ret_Val = QCLI_STATUS_SUCCESS_E;

      if((Parameter_Count >= 3) &&
         (Parameter_List[0].Integer_Is_Valid) &&
         (Verify_Integer_Parameter(&(Parameter_List[1]), QAPI_ZB_APS_MIN_ENDPOINT, QAPI_ZB_APS_MAX_ENDPOINT)) &&
         (Verify_Integer_Parameter(&(Parameter_List[2]), 0, 0xFFFF)))
      {
         DeviceId    = Parameter_List[0].Integer_Value;
         ClusterInfo = ZCL_FindClusterByEndpoint((uint8_t)(Parameter_List[1].Integer_Value), QAPI_ZB_CL_CLUSTER_ID_SCENES, ZCL_DEMO_CLUSTERTYPE_CLIENT);
         GroupId     = (uint16_t)(Parameter_List[2].Integer_Value);

         if(ClusterInfo != NULL)
         {
            /* Format the destination info. */
            memset(&SendInfo, 0, sizeof(qapi_ZB_CL_General_Send_Info_t));
            if(Format_Send_Info_By_Device(DeviceId, &SendInfo))
            {
               Result = qc_drv_ZB_CL_Scenes_Send_Get_Scene_Membership(qc_api_get_qc_drv_context(), ClusterInfo->Handle, &SendInfo, GroupId);
               if(Result == QAPI_OK)
               {
                  LOG_INFO("qc_drv_ZB_CL_Scenes_Send_Get_Scene_Membership");
               }
               else
               {
                  Ret_Val = QCLI_STATUS_ERROR_E;
                  LOG_ERR("qc_drv_ZB_CL_Scenes_Send_Get_Scene_Membership", Result);
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
      LOG_WARN("Zigbee stack is not initialized.\n");
      Ret_Val = QCLI_STATUS_ERROR_E;
   }

   return(Ret_Val);
}

/**
   @brief Executes the "CopyScene" command to copy one scene to another.

   Parameter_List[0] ID of the device to send the command to.
   Parameter_List[1] Endpoint of the Scenes client cluster to use to send the
                     command.
   Parameter_List[2] A flag to indicate if all scenes should be copied.
   Parameter_List[3] ID of the group to copy scenes from.
   Parameter_List[4] ID of the scene to copy from.
   Parameter_List[5] ID of the group to copy scenes to.
   Parameter_List[6] ID of the scene to copy to.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t cmd_ZCL_Scenes_CopyScene(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t           Ret_Val;
   qapi_Status_t                   Result;
   qapi_ZB_CL_General_Send_Info_t  SendInfo;
   ZCL_Demo_Cluster_Info_t        *ClusterInfo;
   uint32_t                        DeviceId;
   qapi_ZB_CL_Scenes_Copy_Scene_t  CopyScene;

   /* Ensure both the stack is initialized. */
   if(GetZigBeeHandle() != NULL)
   {
      Ret_Val = QCLI_STATUS_SUCCESS_E;

      if((Parameter_Count >= 7) &&
         (Parameter_List[0].Integer_Is_Valid) &&
         (Verify_Integer_Parameter(&(Parameter_List[1]), QAPI_ZB_APS_MIN_ENDPOINT, QAPI_ZB_APS_MAX_ENDPOINT)) &&
         (Verify_Integer_Parameter(&(Parameter_List[2]), 0, 1)) &&
         (Verify_Integer_Parameter(&(Parameter_List[3]), 0, 0xFFFF)) &&
         (Verify_Integer_Parameter(&(Parameter_List[4]), 0, 0xFF)) &&
         (Verify_Integer_Parameter(&(Parameter_List[5]), 0, 0xFFFF)) &&
         (Verify_Integer_Parameter(&(Parameter_List[6]), 0, 0xFF)))
      {
         DeviceId                = Parameter_List[0].Integer_Value;
         ClusterInfo             = ZCL_FindClusterByEndpoint((uint8_t)(Parameter_List[1].Integer_Value), QAPI_ZB_CL_CLUSTER_ID_SCENES, ZCL_DEMO_CLUSTERTYPE_CLIENT);
         CopyScene.CopyAllScenes = (qbool_t)(Parameter_List[2].Integer_Value != 0);
         CopyScene.GroupIdFrom   = (uint16_t)(Parameter_List[3].Integer_Value);
         CopyScene.SceneIdFrom   = (uint8_t)(Parameter_List[4].Integer_Value);
         CopyScene.GroupIdTo     = (uint16_t)(Parameter_List[5].Integer_Value);
         CopyScene.SceneIdTo     = (uint8_t)(Parameter_List[6].Integer_Value);

         if(ClusterInfo != NULL)
         {
            /* Format the destination info. */
            memset(&SendInfo, 0, sizeof(qapi_ZB_CL_General_Send_Info_t));
            if(Format_Send_Info_By_Device(DeviceId, &SendInfo))
            {
               Result = qc_drv_ZB_CL_Scenes_Send_Copy_Scene(qc_api_get_qc_drv_context(), ClusterInfo->Handle, &SendInfo, &CopyScene);
               if(Result == QAPI_OK)
               {
                  LOG_INFO("qc_drv_ZB_CL_Scenes_Send_Copy_Scene");
               }
               else
               {
                  Ret_Val = QCLI_STATUS_ERROR_E;
                  LOG_ERR("qc_drv_ZB_CL_Scenes_Send_Copy_Scene", Result);
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
      LOG_WARN("Zigbee stack is not initialized.\n");
      Ret_Val = QCLI_STATUS_ERROR_E;
   }

   return(Ret_Val);
}

/**
   @brief Handles callbacks for the Scenes server cluster.

   @param ZB_Handle is the handle of the ZigBee instance.
   @param EventData is the information for the cluster event.
   @param CB_Param  is the user specified parameter for the callback function.
*/
static void ZCL_Scenes_Demo_Server_CB(qapi_ZB_Handle_t ZB_Handle, qapi_ZB_Cluster_t Cluster, qapi_ZB_CL_Scenes_Server_Event_Data_t *EventData, uint32_t CB_Param)
{
   if((ZB_Handle != NULL) && (Cluster != NULL) && (EventData != NULL))
   {
      LOG_AT_EVT("EVT_ZB: Unhandled Scenes server event %d.\n", EventData->Event_Type);
      QCLI_Display_Prompt();
   }
}

/**
   @brief Handles callbacks for the Scenes client cluster.

   @param ZB_Handle is the handle of the ZigBee instance.
   @param EventData is the information for the cluster event.
   @param CB_Param  is the user specified parameter for the callback function.
*/
static void ZCL_Scenes_Demo_Client_CB(qapi_ZB_Handle_t ZB_Handle, qapi_ZB_Cluster_t Cluster, qapi_ZB_CL_Scenes_Client_Event_Data_t *EventData, uint32_t CB_Param)
{
   uint8_t Index;

   if((ZB_Handle != NULL) && (Cluster != NULL) && (EventData != NULL))
   {
      switch(EventData->Event_Type)
      {
         case QAPI_ZB_CL_SCENES_CLIENT_EVENT_TYPE_DEFAULT_RESPONSE_E:
            LOG_AT_EVT("EVT_ZB: Default Response:\n");
            LOG_AT_EVT("EVT_ZB: Status:        %d\n", EventData->Data.Default_Response.Status);
            LOG_AT_EVT("EVT_ZB: CommandId:     %d\n", EventData->Data.Default_Response.CommandId);
            LOG_AT_EVT("EVT_ZB: CommandStatus: %d\n", EventData->Data.Default_Response.CommandStatus);
            break;

         case QAPI_ZB_CL_SCENES_CLIENT_EVENT_TYPE_COMMAND_COMPLETE_E:
            LOG_AT_EVT("EVT_ZB: Command Complete:\n");
            LOG_AT_EVT("EVT_ZB: CommandStatus: %d\n", EventData->Data.Command_Complete.CommandStatus);
            break;

         case QAPI_ZB_CL_SCENES_CLIENT_EVENT_TYPE_ADD_SCENE_RESPONSE_E:
            LOG_AT_EVT("EVT_ZB: Add Scene Response:\n");
            LOG_AT_EVT("EVT_ZB: Status:     %d\n", EventData->Data.Add_Scene_Response.Status);
            LOG_AT_EVT("EVT_ZB: GroupId:    %d\n", EventData->Data.Add_Scene_Response.GroupId);
            LOG_AT_EVT("EVT_ZB: SceneId:    %d\n", EventData->Data.Add_Scene_Response.SceneId);
            LOG_AT_EVT("EVT_ZB: IsEnhanced: %s\n", EventData->Data.Add_Scene_Response.IsEnhanced ? "Yes" : "No");
            break;

         case QAPI_ZB_CL_SCENES_CLIENT_EVENT_TYPE_VIEW_SCENE_RESPONSE_E:
            LOG_AT_EVT("EVT_ZB: View Scene Response:\n");
            LOG_AT_EVT("EVT_ZB: Status:          %d\n", EventData->Data.View_Scene_Response.Status);
            LOG_AT_EVT("EVT_ZB: GroupId:         %d\n", EventData->Data.View_Scene_Response.GroupId);
            LOG_AT_EVT("EVT_ZB: SceneId:         %d\n", EventData->Data.View_Scene_Response.SceneId);
            LOG_AT_EVT("EVT_ZB: IsEnhanced:      %s\n", EventData->Data.View_Scene_Response.IsEnhanced ? "Yes" : "No");
            LOG_AT_EVT("EVT_ZB: TransitionTime:  %d\n", EventData->Data.View_Scene_Response.TransitionTime);
            LOG_AT_EVT("EVT_ZB: SceneName:       %s\n", EventData->Data.View_Scene_Response.SceneName);
            LOG_AT_EVT("EVT_ZB: ExtFieldCount:   %d\n", EventData->Data.View_Scene_Response.ExtensionFieldCount);

            if(EventData->Data.View_Scene_Response.ExtensionFieldCount != 0)
            {
               LOG_AT_EVT("EVT_ZB: ExtFildSet:    %d", EventData->Data.View_Scene_Response.ExtensionFieldSets[0].ClusterId);

               for(Index = 1; Index < EventData->Data.View_Scene_Response.ExtensionFieldCount; Index ++)
               {
                  LOG_AT_EVT(", %d", EventData->Data.View_Scene_Response.ExtensionFieldSets[Index].ClusterId);
               }

               LOG_AT("\n");
            }

            break;

         case QAPI_ZB_CL_SCENES_CLIENT_EVENT_TYPE_REMOVE_SCENE_RESPONSE_E:
            LOG_AT_EVT("EVT_ZB: Remove Scene Response:\n");
            LOG_AT_EVT("EVT_ZB: Status:  %d\n", EventData->Data.Remove_Scene_Response.Status);
            LOG_AT_EVT("EVT_ZB: GroupId: %d\n", EventData->Data.Remove_Scene_Response.GroupId);
            LOG_AT_EVT("EVT_ZB: SceneId: %d\n", EventData->Data.Remove_Scene_Response.SceneId);
            break;

         case QAPI_ZB_CL_SCENES_CLIENT_EVENT_TYPE_REMOVE_ALL_SCENES_RESPONSE_E:
            LOG_AT_EVT("EVT_ZB: Remove All Scenes Response:\n");
            LOG_AT_EVT("EVT_ZB: Status:  %d\n", EventData->Data.Remove_All_Scenes_Response.Status);
            LOG_AT_EVT("EVT_ZB: GroupId: %d\n", EventData->Data.Remove_All_Scenes_Response.GroupId);
            break;

         case QAPI_ZB_CL_SCENES_CLIENT_EVENT_TYPE_STORE_SCENE_RESPONSE_E:
            LOG_AT_EVT("EVT_ZB: Store Scene Response:\n");
            LOG_AT_EVT("EVT_ZB: Status:  %d\n", EventData->Data.Store_Scene_Response.Status);
            LOG_AT_EVT("EVT_ZB: GroupId: %d\n", EventData->Data.Store_Scene_Response.GroupId);
            LOG_AT_EVT("EVT_ZB: SceneId: %d\n", EventData->Data.Store_Scene_Response.SceneId);
            break;

         case QAPI_ZB_CL_SCENES_CLIENT_EVENT_TYPE_GET_SCENE_MEMBERSHIP_RESPONSE_E:
            LOG_AT_EVT("EVT_ZB: Get Scene Membership Response:\n");
            LOG_AT_EVT("EVT_ZB: Status:     %d\n", EventData->Data.Get_Scene_Membership_Response.Status);
            LOG_AT_EVT("EVT_ZB: Capacity:   %d\n", EventData->Data.Get_Scene_Membership_Response.Capacity);
            LOG_AT_EVT("EVT_ZB: GroupID:    %d\n", EventData->Data.Get_Scene_Membership_Response.GroupId);
            LOG_AT_EVT("EVT_ZB: SceneCount: %d\n", EventData->Data.Get_Scene_Membership_Response.SceneCount);

            if(EventData->Data.Get_Scene_Membership_Response.SceneCount != 0)
            {
               LOG_AT_EVT("EVT_ZB: SceneList:  %d\n", EventData->Data.Get_Scene_Membership_Response.SceneList[0]);
               for(Index = 1; Index < EventData->Data.Get_Scene_Membership_Response.SceneCount; Index ++)
               {
                  LOG_AT_EVT(", %d", EventData->Data.Get_Scene_Membership_Response.SceneList[Index]);
               }
            }
            break;

         case QAPI_ZB_CL_SCENES_CLIENT_EVENT_TYPE_COPY_SCENE_RESPONSE_E:
            LOG_AT_EVT("EVT_ZB: Copy Scene Response:\n");
            LOG_AT_EVT("EVT_ZB: Status:      %d\n", EventData->Data.Copy_Scene_Response.Status);
            LOG_AT_EVT("EVT_ZB: GroupIdFrom: %d\n", EventData->Data.Copy_Scene_Response.GroupIdFrom);
            LOG_AT_EVT("EVT_ZB: SceneIdFrom: %d\n", EventData->Data.Copy_Scene_Response.SceneIdFrom);
            break;

         default:
            LOG_AT_EVT("EVT_ZB: Unhandled Scenes client event %d.\n", EventData->Event_Type);
            break;
      }

      QCLI_Display_Prompt();
   }
}

/**
   @brief Initializes the ZCL Scenes demo.

   @param ZigBee_QCLI_Handle is the parent QCLI handle for the Scenes demo.

   @return true if the ZigBee light demo initialized successfully, false
           otherwise.
*/
qbool_t Initialize_ZCL_Scenes_Demo(QCLI_Group_Handle_t ZigBee_QCLI_Handle)
{
   qbool_t Ret_Val;

   /* Register Scenes command group. */
   ZCL_Scenes_QCLI_Handle = QCLI_Register_Command_Group(ZigBee_QCLI_Handle, &ZCL_Scenes_Cmd_Group);
   if(ZCL_Scenes_QCLI_Handle != NULL)
   {
      Ret_Val = true;
   }
   else
   {
      LOG_WARN("Failed to register ZCL Scenes command group.\n");
      Ret_Val = false;
   }

   return(Ret_Val);
}

/**
   @brief Creates an Scenes server cluster.

   @param Endpoint is the endpoint the cluster will be part of.
   @param PrivData is a pointer to the private data for the cluster demo.  This
                   will be initaialized to NULL before the create function is
                   called so can be ignored if the demo has no private data.

   @return The handle for the newly created function or NULL if there was an
           error.
*/
qapi_ZB_Cluster_t ZCL_Scenes_Demo_Create_Server(uint8_t Endpoint, void **PrivData)
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

      Result = qapi_ZB_CL_Scenes_Populate_Attributes(true, &(ClusterInfo.AttributeCount), (qapi_ZB_CL_Attribute_t*) ClusterInfo.AttributeList);
      if(Result == QAPI_OK)
      {
         Result = qapi_ZB_CL_Scenes_Create_Server(ZigBee_Handle, &Ret_Val, &ClusterInfo, ZCL_SCENES_DEMO_MAX_SCENES, ZCL_Scenes_Demo_Server_CB, 0);
         if(Result != QAPI_OK)
         {
            LOG_ERR("qapi_ZB_CL_Scenes_Create_Server", Result);
            Ret_Val = NULL;
         }
      }
      else
      {
         LOG_ERR("qapi_ZB_CL_Scenes_Populate_Attributes", Result);
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
   @brief Creates an Scenes client cluster.

   @param Endpoint is the endpoint the cluster will be part of.
   @param PrivData is a pointer to the private data for the cluster demo.  This
                   will be initaialized to NULL before the create function is
                   called so can be ignored if the demo has no private data.

   @return The handle for the newly created function or NULL if there was an
           error.
*/
qapi_ZB_Cluster_t ZCL_Scenes_Demo_Create_Client(uint8_t Endpoint, void **PrivData)
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

      Result = qapi_ZB_CL_Scenes_Create_Client(ZigBee_Handle, &Ret_Val, &ClusterInfo, ZCL_Scenes_Demo_Client_CB, 0);
      if(Result != QAPI_OK)
      {
         LOG_ERR("qapi_ZB_CL_Scenes_Create_Client", Result);
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

