/*
 *  Copyright (c) 2018 Qualcomm Technologies, Inc.
 *  All Rights Reserved.
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
#include "qc_api_zigbee.h"

/*-------------------------------------------------------------------------
 * Static & global Variable Declarations
 *-----------------------------------------------------------------------*/
/* The ZigBee demo context. */
static ZigBee_Demo_Context_t ZigBee_Demo_Context;
#if 0
static ZCL_Demo_Context_t ZCL_Demo_Context;
/* The ZigBee color contorl demo context. */
static ZigBee_ColorControl_Demo_Context_t ZigBee_ColorControl_Demo_Context;
static QCLI_Group_Handle_t ZCL_Touchlink_QCLI_Handle;
static QCLI_Group_Handle_t ZCL_Time_QCLI_Handle;
static QCLI_Group_Handle_t ZCL_Alarms_QCLI_Handle;

/* The ZigBee level control demo context. */
static ZigBee_LevelControl_Demo_Context_t ZigBee_LevelControl_Demo_Context;

/* The ZigBee OnOff demo context. */
static ZigBee_Onoff_Demo_Context_t ZigBee_OnOff_Demo_Context;

static QCLI_Group_Handle_t ZCL_Scenes_QCLI_Handle;
static QCLI_Group_Handle_t ZCL_Groups_QCLI_Handle;
static QCLI_Group_Handle_t ZCL_Identify_QCLI_Handle;
static QCLI_Group_Handle_t ZCL_Basic_QCLI_Handle;

/* The list of clusters that are scene capable. */
static const ZCL_Scenes_Cluster_Descriptor_t SceneClusterDescriptorList[] =
{
};

static const ZCL_Basic_Demo_Attr_Info_t BasicAttrInfoList[] =
{
};

static const ZCL_Time_Demo_Attr_Info_t TimeAttrInfoList[] =
{
};
#define SCENES_CLUSTER_DESCRIPTOR_LIST_SIZE     (sizeof(SceneClusterDescriptorList) / sizeof(ZCL_Scenes_Cluster_Descriptor_t))
#endif
/* Security information used by the demo when joining or forming a network. */
const static qapi_ZB_Security_t Default_ZB_Secuity =
{
   QAPI_ZB_SECURITY_LEVEL_NONE_E,                     /* Security level 0.       */
   true,                                              /* Use Insecure Rejoin.    */
   0,                                                 /* Trust center address.   */
   DEFAULT_ZIGBEE_LINK_KEY,                           /* Preconfigured Link Key. */
   {0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7,
    0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF},  /* Distributed global key. */
   {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  /* Network Key.            */
};
#if 0
/**
   @brief Gets the type of attribute given its ID.

   @param AttrId is the ID of the attribute to find.

   @return The type of attribute or batUnknown if the ID wasn't found.
*/
static ZCL_Basic_Demo_Attr_Type_t ZCL_Basic_Demo_GetAttrType(uint16_t AttrId)
{
   ZCL_Basic_Demo_Attr_Type_t Ret_Val;
   uint8_t                    Index;

   Ret_Val = batUnknown;
   for(Index = 0; Index < 0; Index ++)
   {
      if(BasicAttrInfoList[Index].AttrId == AttrId)
      {
         Ret_Val = BasicAttrInfoList[Index].AttrType;
         break;
      }
   }

   return(Ret_Val);
}

/**
   @brief Finds a cluster with a matching ID and endpoint.

   @param Endpoint    is the endpoint for the cluster to find.
   @param ClusterID   is the ID fo the cluster to find.
   @param ClusterType is the type of cluster (server or client).

   @return The handle for the requested cluster or NULL if it was not found.
*/
ZCL_Demo_Cluster_Info_t *ZCL_FindClusterByEndpoint(uint8_t Endpoint, uint16_t ClusterID, ZCL_Demo_ClusterType_t ClusterType)
{
   ZCL_Demo_Cluster_Info_t *Ret_Val;
   uint16_t           Index;

   Ret_Val = NULL;

   /* Try to find the cluster entry to be moved. */
   for(Index = 0; Index < ZCL_Demo_Context.Cluster_Count; Index ++)
   {
      if((ZCL_Demo_Context.Cluster_List[Index].Endpoint == Endpoint) &&
         ((ClusterID == ZCL_DEMO_IGNORE_CLUSTER_ID) || (ClusterID == ZCL_Demo_Context.Cluster_List[Index].ClusterID)) &&
         ((ClusterType == ZCL_DEMO_CLUSTERTYPE_UNKNOWN) || (ClusterType == ZCL_Demo_Context.Cluster_List[Index].ClusterType)))
      {
         Ret_Val = &(ZCL_Demo_Context.Cluster_List[Index]);
         break;
      }
   }

   return(Ret_Val);
}

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

/**
   @brief Gets the type of attribute given its ID.

   @param AttrId is the ID of the attribute to find.

   @return The type of attribute or tatUnknown if the ID wasn't found.
*/
static ZCL_Time_Demo_Attr_Type_t ZCL_Time_Demo_GetAttrType(uint16_t AttrId)
{
   ZCL_Time_Demo_Attr_Type_t Ret_Val;
   uint8_t                   Index;

   Ret_Val = tatUnknown;
   for(Index = 0; Index < 0; Index ++)
   {
      if(TimeAttrInfoList[Index].AttrId == AttrId)
      {
         Ret_Val = TimeAttrInfoList[Index].AttrType;
         break;
      }
   }

   return(Ret_Val);
}

/**
   @brief Removes all clusters associated with an endpoint.

   @param Endpoint is the endpoint to remove all clusters for.
*/
static void ZCL_RemoveClusterByEndpoint(uint8_t Endpoint)
{
   uint16_t Index;

   /* Try to find the cluster entry to be moved. */
   Index = 0;
   while(Index < ZCL_Demo_Context.Cluster_Count)
   {
      if(ZCL_Demo_Context.Cluster_List[Index].Endpoint == Endpoint)
      {
         /* Delete the cluster. */
         qc_drv_ZB_CL_Destroy_Cluster(qc_api_get_qc_drv_context(), ZCL_Demo_Context.Cluster_List[Index].Handle);

         /* Free the priv data if allocated.. */
         if(ZCL_Demo_Context.Cluster_List[Index].PrivData != NULL)
         {
            free(ZCL_Demo_Context.Cluster_List[Index].PrivData);
         }

         memmove(&(ZCL_Demo_Context.Cluster_List[Index]), &(ZCL_Demo_Context.Cluster_List[Index + 1]), (ZCL_Demo_Context.Cluster_Count - Index - 1) * sizeof(ZCL_Demo_Cluster_Info_t));
         (ZCL_Demo_Context.Cluster_Count)--;
      }
      else
      {
         /* Only increment the index if an item wasn't removed. */
         Index ++;
      }
   }
}

/**
   @brief Gets the cluster handle for a specified index.

   @param ClusterIndex is the index of the cluster being requested.
   @param ClusterID    is the expected ID of the cluster being requested.  Set
                       to ZCL_DEMO_IGNORE_CLUSTER_ID to ignore.

   @return The info structure for the cluster or NULL if it was not found.
*/
ZCL_Demo_Cluster_Info_t *ZCL_FindClusterByIndex(uint16_t ClusterIndex, uint16_t ClusterID)
{
   ZCL_Demo_Cluster_Info_t *Ret_Val;

   if((ClusterIndex < ZCL_Demo_Context.Cluster_Count) && ((ClusterID == ZCL_DEMO_IGNORE_CLUSTER_ID) || (ZCL_Demo_Context.Cluster_List[ClusterIndex].ClusterID == ClusterID)))
   {
      Ret_Val = &(ZCL_Demo_Context.Cluster_List[ClusterIndex]);
   }
   else
   {
      Ret_Val = NULL;
   }

   return(Ret_Val);
}
#endif
/**
   @brief Callback handler for the ZigBee stack.

   @param ZB_Handle is the handle of the ZigBee instance was returned by a
                    successful call to qapi_ZB_Initialize().
   @param CB_Param  is the user specified parameter for the callback function.
*/
static void ZB_Event_CB(qapi_ZB_Handle_t ZB_Handle, const qapi_ZB_Event_t *ZB_Event, uint32_t CB_Param)
{
   if((ZigBee_Demo_Context.ZigBee_Handle != NULL) && (ZB_Handle == ZigBee_Demo_Context.ZigBee_Handle) && (ZB_Event != NULL))
   {
      switch(ZB_Event->Event_Type)
      {
         case QAPI_ZB_EVENT_TYPE_FORM_CONFIRM_E:
            LOG_AT_EVT("EVT_ZB: Form confirm:\n");
            LOG_AT_EVT("EVT_ZB: Status:  %d\n", ZB_Event->Event_Data.Form_Confirm.Status);
            LOG_AT_EVT("EVT_ZB: Channel: %d\n", ZB_Event->Event_Data.Form_Confirm.ActiveChannel);
            break;

         case QAPI_ZB_EVENT_TYPE_JOIN_CONFIRM_E:
            LOG_AT_EVT("EVT_ZB: Join confirm:\n");
            LOG_AT_EVT("EVT_ZB: Status:        %d\n", ZB_Event->Event_Data.Join_Confirm.Status);
            LOG_AT_EVT("EVT_ZB: NwkAddress:    0x%04X\n", ZB_Event->Event_Data.Join_Confirm.NwkAddress);
            LOG_AT_EVT("EVT_ZB: ExtendedPanId: %08X%08X\n", (uint32_t)(ZB_Event->Event_Data.Join_Confirm.ExtendedPanId >> 32), (uint32_t)(ZB_Event->Event_Data.Join_Confirm.ExtendedPanId));
            LOG_AT_EVT("EVT_ZB: Channel:       %d\n", ZB_Event->Event_Data.Join_Confirm.ActiveChannel);
            break;

         case QAPI_ZB_EVENT_TYPE_RECONNECT_CONFIRM_E:
            LOG_AT_EVT("EVT_ZB: Reconnect confirm:\n");
            LOG_AT_EVT("EVT_ZB: Status:          %d\n", ZB_Event->Event_Data.Reconnect_Confirm.Status);
            LOG_AT_EVT("EVT_ZB: Network Address: 0x%04X\n", ZB_Event->Event_Data.Reconnect_Confirm.NwkAddress);
            break;

         case QAPI_ZB_EVENT_TYPE_LEAVE_CONFIRM_E:
            LOG_AT_EVT("EVT_ZB: Leave confirm:\n");
            LOG_AT_EVT("EVT_ZB: Status: %d\n", ZB_Event->Event_Data.Leave_Confirm.Status);
            break;

         case QAPI_ZB_EVENT_TYPE_LEAVE_IND_E:
            LOG_AT_EVT("EVT_ZB: Leave indication:\n");
            LOG_AT_EVT("EVT_ZB: ExtendedAddress: %08X%08X\n", (uint32_t)(ZB_Event->Event_Data.Leave_Ind.ExtendedAddress >> 32), (uint32_t)(ZB_Event->Event_Data.Leave_Ind.ExtendedAddress));
            LOG_AT_EVT("EVT_ZB: NetworkAddress:  0x%04X\n", ZB_Event->Event_Data.Leave_Ind.NwkAddress);
            LOG_AT_EVT("EVT_ZB: Rejoin:          %s\n", (ZB_Event->Event_Data.Leave_Ind.Rejoin) ? "true" : "false");
            break;

         default:
            LOG_AT_EVT("EVT_ZB: Unhandled ZigBee Event: %d\n", ZB_Event->Event_Type);
            break;
      }

      QCLI_Display_Prompt();
   }
}

/**
   @brief Callback handler for persist notify indications.

   @param[in] ZB_Handle Handle of the ZigBee instance that was returned by a
                        successful call to qapi_ZB_Initialize().
   @param[in] CB_Param  User-specified parameter for the callback function.
*/
static void ZB_Persist_Notify_CB(qapi_ZB_Handle_t ZB_Handle, uint32_t CB_Param)
{
   qapi_Status_t          Result;
   qapi_Persist_Handle_t  PersistHandle;
   uint32_t               PersistLength;
   uint8_t               *PersistData;

   if((ZB_Handle == ZigBee_Demo_Context.ZigBee_Handle) && (CB_Param != 0))
   {
      PersistHandle = (qapi_Persist_Handle_t)CB_Param;

      PersistLength = 0;
      Result = qc_drv_ZB_Get_Persistent_Data(qc_api_get_qc_drv_context(), ZB_Handle, NULL, &PersistLength);

      if((Result == QAPI_ERR_BOUNDS) && (PersistLength > 0))
      {
         PersistData = (uint8_t *)malloc(PersistLength);
         if(PersistData != NULL)
         {
            Result = qc_drv_ZB_Get_Persistent_Data(qc_api_get_qc_drv_context(), ZB_Handle, PersistData, &PersistLength);
            if(Result == QAPI_OK)
            {
               Result = qapi_Persist_Put(PersistHandle, PersistLength, PersistData);
               if(Result != QAPI_OK)
               {
                  Display_Function_Error("qapi_Persist_Put", Result);
               }
            }
            else
            {
               Display_Function_Error("qc_drv_ZB_Get_Persistent_Data", Result);
            }

            free(PersistData);
         }
         else
         {
            LOG_AT_EVT("EVT_ZB: Failed to allocate persist data buffer.", Result);
         }
      }
      else
      {
         Display_Function_Error("qc_drv_ZB_Get_Persistent_Data", Result);
      }
   }
}

/**
   @brief Callback handler for the ZigBee Device Profile.

   @param ZB_Handle      is the handle of the ZigBee instance.
   @param ZDP_Event_Data is the information for the event.
   @param CB_Param       is the user specified parameter for the callback
                         function.
*/
static void ZB_ZDP_Event_CB(qapi_ZB_Handle_t ZB_Handle, const qapi_ZB_ZDP_Event_t *ZDP_Event_Data, uint32_t CB_Param)
{
   uint8_t Index;

   if((ZB_Handle == ZigBee_Demo_Context.ZigBee_Handle) && (ZDP_Event_Data != NULL))
   {
      switch(ZDP_Event_Data->Event_Type)
      {
         case QAPI_ZB_ZDP_EVENT_TYPE_MATCH_DESC_RSP_E:
            LOG_AT_EVT("EVT_ZB: Match Descriptor Response:\n");
            LOG_AT_EVT("EVT_ZB: Status:              %d\n", ZDP_Event_Data->Event_Data.Match_Desc_Rsp.Status);
            if(ZDP_Event_Data->Event_Data.Match_Desc_Rsp.Status == QAPI_OK)
            {
               LOG_AT_EVT("EVT_ZB: Address:             0x%04X\n", ZDP_Event_Data->Event_Data.Match_Desc_Rsp.NwkAddrOfInterest);
               LOG_AT_EVT("EVT_ZB: Number Of Endpoints: %d\n",     ZDP_Event_Data->Event_Data.Match_Desc_Rsp.MatchLength);
               LOG_AT_EVT("EVT_ZB: Matched Endpoints:   ");
               for(Index = 0; Index < ZDP_Event_Data->Event_Data.Match_Desc_Rsp.MatchLength; Index++)
               {
                  LOG_AT_EVT("EVT_ZB: %d%s", (ZDP_Event_Data->Event_Data.Match_Desc_Rsp.MatchList)[Index], (Index == (ZDP_Event_Data->Event_Data.Match_Desc_Rsp.MatchLength - 1)) ? "\n" : ", ");
               }
            }
            break;

         case QAPI_ZB_ZDP_EVENT_TYPE_DEVICE_ANNCE_E:
            LOG_AT_EVT("EVT_ZB: Device Annce:\n");
            LOG_AT_EVT("EVT_ZB: ExtendedAddress: %08X%08X\n", (uint32_t)(ZDP_Event_Data->Event_Data.Device_Annce.IEEEAddr >> 32), (uint32_t)(ZDP_Event_Data->Event_Data.Device_Annce.IEEEAddr));
            LOG_AT_EVT("EVT_ZB: NetworkAddress:  0x%04X\n", ZDP_Event_Data->Event_Data.Device_Annce.NwkAddr);
            LOG_AT_EVT("EVT_ZB: Capability:      0x%02X\n", ZDP_Event_Data->Event_Data.Device_Annce.Capability);
            break;

         case QAPI_ZB_ZDP_EVENT_TYPE_BIND_RSP_E:
            LOG_AT_EVT("EVT_ZB: ZDP Bind Response:\n");
            LOG_AT_EVT("EVT_ZB: Status: %d\n", ZDP_Event_Data->Event_Data.Bind_Rsp.Status);
            break;

         case QAPI_ZB_ZDP_EVENT_TYPE_END_DEVICE_BIND_RSP_E:
            LOG_AT_EVT("EVT_ZB: ZDP End Device Bind Response:\n");
            LOG_AT_EVT("EVT_ZB: Status: %d\n", ZDP_Event_Data->Event_Data.End_Device_Bind_Rsp.Status);
            break;

         case QAPI_ZB_ZDP_EVENT_TYPE_UNBIND_RSP_E:
            LOG_AT_EVT("EVT_ZB: ZDP Unbind Response:\n");
            LOG_AT_EVT("EVT_ZB: Status: %d\n", ZDP_Event_Data->Event_Data.Unbind_Rsp.Status);
            break;

         default:
            LOG_AT_EVT("EVT_ZB: Unhandled ZDP Event: %d.\n", ZDP_Event_Data->Event_Type);
            break;
      }

      QCLI_Display_Prompt();
   }
}

/**
   @brief Executes the "Initialize" command to initialize the ZigBee interface.

   Parameter_List[0] (0-1) is a flag indicating if persistense should be used.
                     If set to 1, the intialize function will try to load any
                     existing persistence data and register the callback to
                     periodically store persistent data.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_api_cmd_ZB_Initialize(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t  Ret_Val;
   qapi_Status_t          Result;
   uint64_t               Extended_Address;
   qbool_t                UsePersist;
   uint32_t               PersistLength;
   uint8_t               *PersistData;

   /* Verify the ZigBee layer had not been initialized yet. */
   if(ZigBee_Demo_Context.ZigBee_Handle == NULL)
   {
      UsePersist = false;

      if(Parameter_Count >= 1)
      {
         if(Verify_Integer_Parameter(&Parameter_List[0], 0, 1))
         {
            UsePersist = (qbool_t)(Parameter_List[0].Integer_Value != 0);
            Ret_Val = QCLI_STATUS_SUCCESS_E;
         }
         else
         {
            Ret_Val = QCLI_STATUS_USAGE_E;
         }
      }
      else
      {
         Ret_Val = QCLI_STATUS_SUCCESS_E;
      }

      if(Ret_Val == QCLI_STATUS_SUCCESS_E)
      {
         Result = qc_drv_ZB_Initialize(qc_api_get_qc_drv_context(), &(ZigBee_Demo_Context.ZigBee_Handle), ZB_Event_CB, 0);

         if((Result == QAPI_OK) && (ZigBee_Demo_Context.ZigBee_Handle != NULL))
         {
            if(UsePersist)
            {
               /* Attempt to load the persist data. */
               Result = qc_drv_Persist_Initialize(qc_api_get_qc_drv_context(), &(ZigBee_Demo_Context.PersistHandle), ZIGBEE_PERSIST_DIRECTORY, ZIGBEE_PERSIST_PREFIX, ZIGBEE_PERSIST_SUFFIX, NULL, 0);
               if(Result == QAPI_OK)
               {
                  Result = qapi_Persist_Get(ZigBee_Demo_Context.PersistHandle, &PersistLength, &PersistData);
                  if(Result == QAPI_OK)
                  {
                     Result = qc_drv_ZB_Restore_Persistent_Data(qc_api_get_qc_drv_context(), ZigBee_Demo_Context.ZigBee_Handle, PersistData, PersistLength);
                     if(Result == QAPI_OK)
                     {
                        LOG_INFO("Persist Data Loaded.\n");
                     }
                     else
                     {
                        Display_Function_Error("qc_drv_ZB_Set_Persistent_Data", Result);
                     }

                     qapi_Persist_Free(ZigBee_Demo_Context.PersistHandle, PersistData);
                  }
                  else if(Result != QAPI_ERR_NO_ENTRY)
                  {
                     Display_Function_Error("qapi_Persist_Get", Result);
                  }

                  /* If it got this far, register the persist data callback. */
                  Result = qc_drv_ZB_Register_Persist_Notify_CB(qc_api_get_qc_drv_context(), ZigBee_Demo_Context.ZigBee_Handle, ZB_Persist_Notify_CB, (uint32_t)(ZigBee_Demo_Context.PersistHandle));
                  if(Result != QAPI_OK)
                  {
                     Display_Function_Error("qc_drv_ZB_Register_Persist_Notify_CB", Result);
                  }
               }
               else
               {
                  Display_Function_Error("qapi_Persist_Initialize", Result);
               }
            }

            /* Register the ZDP callback. */
            Result = qc_drv_ZB_ZDP_Register_Callback(qc_api_get_qc_drv_context(), ZigBee_Demo_Context.ZigBee_Handle, ZB_ZDP_Event_CB, 0);
            if(Result == QAPI_OK)
            {
               Result = qc_drv_ZB_Get_Extended_Address(qc_api_get_qc_drv_context(), ZigBee_Demo_Context.ZigBee_Handle, &Extended_Address);
               if(Result == QAPI_OK)
               {

                  LOG_INFO("ZigBee stack initialized.\n");
                  LOG_INFO("Extended Address: %08X%08X\n", (uint32_t)(Extended_Address >> 32), (uint32_t)Extended_Address);
               }
               else
               {
                  Display_Function_Error("qc_drv_ZB_Get_Extended_Address", Result);
                  Ret_Val = QCLI_STATUS_ERROR_E;
               }
            }
            else
            {
               Display_Function_Error("qc_drv_ZB_ZDP_Register_Callback", Result);
               Ret_Val = QCLI_STATUS_ERROR_E;
            }

            if(Ret_Val != QCLI_STATUS_SUCCESS_E)
            {
               qc_drv_ZB_Shutdown(qc_api_get_qc_drv_context(), ZigBee_Demo_Context.ZigBee_Handle);
               ZigBee_Demo_Context.ZigBee_Handle = NULL;

               if(ZigBee_Demo_Context.PersistHandle != NULL)
               {
                  qc_drv_Persist_Cleanup(qc_api_get_qc_drv_context(), ZigBee_Demo_Context.PersistHandle);
               }
            }
         }
         else
         {
            Display_Function_Error("qc_drv_ZB_Initialize", Result);
            Ret_Val = QCLI_STATUS_ERROR_E;
         }
      }
   }
   else
   {
      LOG_WARN("ZigBee stack already initialized.\n");
      Ret_Val = QCLI_STATUS_ERROR_E;
   }

   return(Ret_Val);
}

/**
   @brief Executes the "Shutdown" command to shut down the ZigBee interface.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E   indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E   indicates there is usage error associated with this
                            command.
*/
QCLI_Command_Status_t qc_api_cmd_ZB_Shutdown(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t Ret_Val;

   /* Verify the ZigBee layer had been initialized. */
   if(ZigBee_Demo_Context.ZigBee_Handle != NULL)
   {
      /* Cleanup the clusters have have been created. */
      //ZB_Cluster_Cleanup();

      /* Shutdown the ZigBee stack. */
      qc_drv_ZB_Shutdown(qc_api_get_qc_drv_context(), ZigBee_Demo_Context.ZigBee_Handle);
      ZigBee_Demo_Context.ZigBee_Handle = NULL;

      if(ZigBee_Demo_Context.PersistHandle != NULL)
      {
         qc_drv_Persist_Cleanup(qc_api_get_qc_drv_context(), ZigBee_Demo_Context.PersistHandle);
      }

      LOG_INFO("ZigBee shutdown.\n");

      Ret_Val = QCLI_STATUS_SUCCESS_E;
   }
   else
   {
      LOG_WARN("ZigBee not initialized.\n");
      Ret_Val = QCLI_STATUS_ERROR_E;
   }

   return(Ret_Val);
}

/**
   @brief Executes the "AddDevice" command to add an address to the demo's
          device list.

   The addresses added with this command can be either a group address, a
   network address + endpoint or a extended address + endpoint.  Once
   registered, these addresses can be used to send commands to a remote device.

   Parameter_List[0] (1-3) is the type of address to be added. 0 for a group
                     address, 1 for a network address or 2 for an extended
                     address.
   Parameter_List[1] is the address to be added.  For group and network
                     addresses, this must be a valid 16-bit value and for
                     extended addresses, it will be a 64-bit value.
   Parameter_List[2] is the endpoint for the address. Note this isn't used for
                     group addresses.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_api_cmd_ZB_AddDevice(uint32_t Parameter_Count,  QCLI_Parameter_t *Parameter_List)
{
   uint32_t              Index;
   QCLI_Command_Status_t Ret_Val;
   qapi_ZB_Addr_Mode_t   DevType;
   uint8_t               DevEndpoint;
   uint64_t              DevAddr;

   if(ZigBee_Demo_Context.ZigBee_Handle != 0)
   {
      if((Parameter_Count >= 2) &&
         (Verify_Integer_Parameter(&Parameter_List[0], QAPI_ZB_ADDRESS_MODE_GROUP_ADDRESS_E, QAPI_ZB_ADDRESS_MODE_EXTENDED_ADDRESS_E)) &&
         (Hex_String_To_ULL(Parameter_List[1].String_Value, &DevAddr)))
      {
         DevType = (qapi_ZB_Addr_Mode_t)Parameter_List[0].Integer_Value;

         /* Assume success unless something goes wrong. */
         Ret_Val = QCLI_STATUS_SUCCESS_E;

         /* We either need a group address or an endpoint needs to also be
            specified with this command. */
         if(DevType != QAPI_ZB_ADDRESS_MODE_GROUP_ADDRESS_E)
         {
            if((Parameter_Count >= 3) &&
               (Verify_Integer_Parameter(&Parameter_List[2], 0, 0xFF)))
            {
               /* Record the endpoint parameter. */
               DevEndpoint = Parameter_List[2].Integer_Value;
            }
            else
            {
               Ret_Val = QCLI_STATUS_USAGE_E;
            }
         }
         else
         {
            /* Groups do not require endpoints. */
            DevEndpoint = 0;
         }

         /* Continue if all parameters validated successfully. */
         if(Ret_Val == QCLI_STATUS_SUCCESS_E)
         {
            /* Confirm the device address is valid for the type. */
            if((DevType == QAPI_ZB_ADDRESS_MODE_EXTENDED_ADDRESS_E) || (DevAddr <= 0xFFFF))
            {
               /* Find an unused spot in the array. */
               for(Index = 1; Index <= DEV_ID_LIST_SIZE; Index++)
               {
                  if(!(ZigBee_Demo_Context.DevIDList[Index].InUse))
                  {
                     /* Found a spare slot. */
                     ZigBee_Demo_Context.DevIDList[Index].Type                    = (qapi_ZB_Addr_Mode_t)DevType;
                     ZigBee_Demo_Context.DevIDList[Index].Endpoint                = DevEndpoint;
                     ZigBee_Demo_Context.DevIDList[Index].Address.ExtendedAddress = DevAddr;
                     ZigBee_Demo_Context.DevIDList[Index].InUse                   = true;

                     LOG_INFO("Registered device ID: %d\n", Index);
                     break;
                  }
               }

               if(Index > DEV_ID_LIST_SIZE)
               {
                  LOG_ERR("Could not find a spare device entry.\n");
                  Ret_Val = QCLI_STATUS_ERROR_E;
               }
            }
            else
            {
               LOG_ERR("Address is too large for type specified.\n");
               Ret_Val = QCLI_STATUS_ERROR_E;
            }
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
   @brief Executes the "RemoveDevice" command to remove an address to the demo's
          device list.

   Parameter_List[0] (1-DEV_ID_LIST_SIZE) is the index of the device to remove.
                     Note that address zero is reserved for the NULL address and
                     cannot be removed.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_api_cmd_ZB_RemoveDevice(uint32_t Parameter_Count,  QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t Ret_Val;
   uint32_t              Index;

   if(ZigBee_Demo_Context.ZigBee_Handle != 0)
   {
      /* Check the parameters. */
      if((Parameter_Count >= 1) &&
         (Verify_Integer_Parameter(&Parameter_List[0], 1, DEV_ID_LIST_SIZE)))
      {
         Index = Parameter_List[0].Integer_Value;

         /* Make sure the device is actually in use. */
         if(ZigBee_Demo_Context.DevIDList[Index].InUse)
         {
            /* Clear the device mapping. */
            memset(&ZigBee_Demo_Context.DevIDList[Index], 0, sizeof(ZB_Device_ID_t));
            Ret_Val = QCLI_STATUS_SUCCESS_E;
         }
         else
         {
            LOG_ERR("Device ID is not in use.\n");
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
   @brief Executes the "ShowDeviceList" command to display the list of
          registered devices in the demo's device list.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_api_cmd_ZB_ShowDeviceList(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t  Ret_Val;
   uint32_t               Index;
   uint8_t                AddressString[17];
   const char            *TypeString;

   if(ZigBee_Demo_Context.ZigBee_Handle != 0)
   {
      LOG_INFO("Device List:\n");
      LOG_INFO(" ID | Type | Address          | Endpoint\n");
      LOG_INFO("----+------+------------------+----------\n");

      for(Index = 0; Index <= DEV_ID_LIST_SIZE; Index++)
      {
         if(ZigBee_Demo_Context.DevIDList[Index].InUse)
         {
            switch(ZigBee_Demo_Context.DevIDList[Index].Type)
            {
               case QAPI_ZB_ADDRESS_MODE_GROUP_ADDRESS_E:
                  TypeString = "Grp ";
                  snprintf((char *)(AddressString), sizeof(AddressString), "%04X", ZigBee_Demo_Context.DevIDList[Index].Address.ShortAddress);
                  break;

               case QAPI_ZB_ADDRESS_MODE_SHORT_ADDRESS_E:
                  TypeString = "Nwk ";
                  snprintf((char *)(AddressString), sizeof(AddressString), "%04X", ZigBee_Demo_Context.DevIDList[Index].Address.ShortAddress);
                  break;

               case QAPI_ZB_ADDRESS_MODE_EXTENDED_ADDRESS_E:
                  TypeString = "Ext ";
                  snprintf((char *)(AddressString), sizeof(AddressString), "%08X%08X", (unsigned int)(ZigBee_Demo_Context.DevIDList[Index].Address.ExtendedAddress >> 32), (unsigned int)(ZigBee_Demo_Context.DevIDList[Index].Address.ExtendedAddress));
                  break;

               case QAPI_ZB_ADDRESS_MODE_NONE_E:
               default:
                  TypeString = "None";
                  AddressString[0] = '\0';
                  break;
            }

            LOG_INFO(" %2d | %4s | %16s | %d\n", Index, TypeString, AddressString, ZigBee_Demo_Context.DevIDList[Index].Endpoint);
         }
      }

      LOG_AT("\n");
      Ret_Val = QCLI_STATUS_SUCCESS_E;
   }
   else
   {
      LOG_WARN("ZigBee stack is not initialized.\n");
      Ret_Val = QCLI_STATUS_ERROR_E;
   }

   return(Ret_Val);
}

/**
   @brief Executes the "Form" command to start a ZigBee network.

   Parameter_List[0] (0-1) indicates if security should be used on the network.
                           1 will enable security and 0 will disable security.
   Parameter_List[1] (0-1) indicates if the device should form a distributed (1)
                           or centralized (0) network. Defaults to centralized.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_api_cmd_ZB_Form(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t   Ret_Val;
   qapi_Status_t           Result;
   qapi_ZB_NetworkConfig_t NetworkConfig;
   qbool_t                 UseSecurity;
   qbool_t                 Distributed;
   uint32_t                ChannelMask;

   /* Verify the ZigBee layer had been initialized. */
   if(ZigBee_Demo_Context.ZigBee_Handle != NULL)
   {
      /* Check the parameters. */
      if((Parameter_Count >= 1) &&
         (Verify_Integer_Parameter(&(Parameter_List[0]), 0, 1)))
      {
         UseSecurity = (qbool_t)(Parameter_List[0].Integer_Value != 0);
         Distributed = false;
         ChannelMask = FORM_CHANNEL_MASK;

         Ret_Val = QCLI_STATUS_SUCCESS_E;
         if(Parameter_Count >= 2)
         {
            if(Verify_Integer_Parameter(&(Parameter_List[1]), 0, 1))
            {
               Distributed = (qbool_t)(Parameter_List[1].Integer_Value != 0);
            }
            else
            {
               Ret_Val = QCLI_STATUS_USAGE_E;
            }
         }

         if(Parameter_Count >= 3)
         {
            if(Verify_Integer_Parameter(&(Parameter_List[2]), 11, 26))
            {
               ChannelMask = 1 << Parameter_List[2].Integer_Value;
            }
            else
            {
               Ret_Val = QCLI_STATUS_USAGE_E;
            }
         }

         if(Ret_Val == QCLI_STATUS_SUCCESS_E)
         {
            memset(&NetworkConfig, 0, sizeof(qapi_ZB_NetworkConfig_t));

            NetworkConfig.ExtendedPanId = 0ULL;
            NetworkConfig.StackProfile  = QAPI_ZB_STACK_PROFILE_PRO_E;
            NetworkConfig.Page          = 0;
            NetworkConfig.ChannelMask   = ChannelMask;
            NetworkConfig.ScanAttempts  = 2;
            NetworkConfig.Capability    = COORDINATOR_CAPABILITIES;

            memscpy(&(NetworkConfig.Security), sizeof(qapi_ZB_Security_t), &Default_ZB_Secuity, sizeof(qapi_ZB_Security_t));

            /* Overwrite the default security level and trust center address. */
            NetworkConfig.Security.Security_Level       = UseSecurity ? QAPI_ZB_SECURITY_LEVEL_ENC_MIC32_E : QAPI_ZB_SECURITY_LEVEL_NONE_E;
            NetworkConfig.Security.Trust_Center_Address = Distributed ? QAPI_ZB_INVALID_EXTENDED_ADDRESS : 0;

            Result = qc_drv_ZB_Form(qc_api_get_qc_drv_context(), ZigBee_Demo_Context.ZigBee_Handle, &NetworkConfig);

            if(Result == QAPI_OK)
            {
               Display_Function_Success("qc_drv_ZB_Form");
               Ret_Val = QCLI_STATUS_SUCCESS_E;
            }
            else
            {
               Display_Function_Error("qc_drv_ZB_Form", Result);
               Ret_Val = QCLI_STATUS_ERROR_E;
            }
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
   @brief Executes the "Join" command to join a ZigBee network.

   Parameter_List[0] (0-1) indicates if the device should join as a coordinator
                           (1) or as an end device (0).
   Parameter_List[1] (0-1) indicates if security should be used on the network
                           (1) or not (0).
   Parameter_List[2] (0-1) indicates if this is a join operation (0) or a rejoin
                           operation. Defaults to join if not specified.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_api_ZB_Join(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
       QCLI_Command_Status_t Ret_Val;
   qapi_Status_t         Result;
   qapi_ZB_Join_t        JoinConfig;
   qbool_t               IsCoordinator;
   qbool_t               UseSecurity;
   qbool_t               IsRejoin;
   uint32_t              ChannelMask;

   /* Verify the ZigBee layer had been initialized. */
   if(ZigBee_Demo_Context.ZigBee_Handle != NULL)
   {
      /* Check the parameters. */
      if((Parameter_Count >= 2) &&
         (Verify_Integer_Parameter(&(Parameter_List[0]), 0, 1)) &&
         (Verify_Integer_Parameter(&(Parameter_List[1]), 0, 1)))
      {
         IsCoordinator = (qbool_t)(Parameter_List[0].Integer_Value != 0);
         UseSecurity   = (qbool_t)(Parameter_List[1].Integer_Value != 0);
         IsRejoin      = false;
         ChannelMask   = JOIN_CHANNEL_MASK;

         Ret_Val = QCLI_STATUS_SUCCESS_E;

         if(Parameter_Count >= 3)
         {
            if(Verify_Integer_Parameter(&(Parameter_List[2]), 0, 1))
            {
               IsRejoin = (qbool_t)(Parameter_List[2].Integer_Value != 0);
            }
            else
            {
               Ret_Val = QCLI_STATUS_USAGE_E;
            }
         }

         if(Parameter_Count >= 4)
         {
            if(Verify_Integer_Parameter(&(Parameter_List[3]), 11, 26))
            {
               ChannelMask = 1 << Parameter_List[3].Integer_Value;
            }
            else
            {
               Ret_Val = QCLI_STATUS_USAGE_E;
            }
         }

         if(Ret_Val == QCLI_STATUS_SUCCESS_E)
         {
            memset(&JoinConfig, 0, sizeof(qapi_ZB_Join_t));
            JoinConfig.IsRejoin                    = IsRejoin;
            JoinConfig.EndDeviceTimeout            = IsCoordinator ? 0 : DEFAULT_END_DEVICE_TIME_OUT;
            JoinConfig.NetworkConfig.ExtendedPanId = 0ULL;
            JoinConfig.NetworkConfig.StackProfile  = QAPI_ZB_STACK_PROFILE_PRO_E;
            JoinConfig.NetworkConfig.ScanAttempts  = 2;
            JoinConfig.NetworkConfig.Page          = 0;
            JoinConfig.NetworkConfig.ChannelMask   = ChannelMask;
            JoinConfig.NetworkConfig.Capability    = IsCoordinator ? COORDINATOR_CAPABILITIES : END_DEVICE_CAPABILITIES;

            memscpy(&(JoinConfig.NetworkConfig.Security), sizeof(qapi_ZB_Security_t), &Default_ZB_Secuity, sizeof(qapi_ZB_Security_t));

            /* Overwrite the default security level. */
            JoinConfig.NetworkConfig.Security.Security_Level = UseSecurity ? QAPI_ZB_SECURITY_LEVEL_ENC_MIC32_E : QAPI_ZB_SECURITY_LEVEL_NONE_E;

            if(Ret_Val == QCLI_STATUS_SUCCESS_E)
            {
               Result = qc_drv_ZB_Join(qc_api_get_qc_drv_context(), ZigBee_Demo_Context.ZigBee_Handle, &JoinConfig);

               if(Result == QAPI_OK)
               {
                  Display_Function_Success("qc_drv_ZB_Join");
               }
               else
               {
                  Display_Function_Error("qc_drv_ZB_Join", Result);
                  Ret_Val = QCLI_STATUS_ERROR_E;
               }
            }
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
   @brief Executes the "Leave" to tell the local device to leave the network.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_api_cmd_ZB_Leave(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t Ret_Val;
   qapi_Status_t         Result;

   /* Verify the ZigBee layer had been initialized. */
   if(ZigBee_Demo_Context.ZigBee_Handle != NULL)
   {
      Result = qc_drv_ZB_Leave(qc_api_get_qc_drv_context(), ZigBee_Demo_Context.ZigBee_Handle);

      if(Result == QAPI_OK)
      {
         Display_Function_Success("qc_drv_ZB_Leave");
         Ret_Val = QCLI_STATUS_SUCCESS_E;
      }
      else
      {
         Display_Function_Error("qc_drv_ZB_Leave", Result);
         Ret_Val = QCLI_STATUS_ERROR_E;
      }
   }
   else
   {
      LOG_INFO("ZigBee stack is not initialized.\n");
      Ret_Val = QCLI_STATUS_ERROR_E;
   }

   return(Ret_Val);
}

/**
   @brief Executes the "LeaveReq" command to send a leave request.

   Parameter_List[0] is the device ID to send the request to (NWK Address)
   Parameter_List[1] is the device ID for the device that needs to leave
                     (Extended Address).
   Parameter_List[1] is a flag indicating if the device should remove its
                     children.
   Parameter_List[2] is a flag indicating if the device should attempt to rejoin
                     the network.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_api_cmd_ZB_LeaveReq(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t   Ret_Val;
   qapi_Status_t           Result;
   ZB_Device_ID_t         *TargetDevice;
   ZB_Device_ID_t         *LeaveDevice;
   qbool_t                 RemoveChildren;
   qbool_t                 Rejoin;

   if(ZigBee_Demo_Context.ZigBee_Handle != NULL)
   {
      /* Check the parameters. */
      if((Parameter_Count >= 4) &&
         (Verify_Integer_Parameter(&(Parameter_List[2]), 0, 1)) &&
         (Verify_Integer_Parameter(&(Parameter_List[3]), 0, 1)))
      {
         /* Get the device information. */
         TargetDevice   = GetDeviceListEntry(Parameter_List[0].Integer_Value);
         LeaveDevice    = GetDeviceListEntry(Parameter_List[1].Integer_Value);
         RemoveChildren = (qbool_t)(Parameter_List[2].Integer_Value);
         Rejoin         = (qbool_t)(Parameter_List[3].Integer_Value);

         if((TargetDevice != NULL) && (LeaveDevice != NULL))
         {
            /* Verify the address types for each specified device is valid. */
            if((TargetDevice->Type == QAPI_ZB_ADDRESS_MODE_SHORT_ADDRESS_E) && (LeaveDevice->Type == QAPI_ZB_ADDRESS_MODE_EXTENDED_ADDRESS_E))
            {
               Result = qc_drv_ZB_ZDP_Mgmt_Leave_Req(qc_api_get_qc_drv_context(), ZigBee_Demo_Context.ZigBee_Handle, TargetDevice->Address.ShortAddress, LeaveDevice->Address.ExtendedAddress, RemoveChildren, Rejoin);
               if(Result == QAPI_OK)
               {
                  Display_Function_Success("qc_drv_ZB_ZDP_Mgmt_Leave_Req");
                  Ret_Val = QCLI_STATUS_SUCCESS_E;
               }
               else
               {
                  Display_Function_Error("qc_drv_ZB_ZDP_Mgmt_Leave_Req", Result);
                  Ret_Val = QCLI_STATUS_ERROR_E;
               }
            }
            else
            {
               LOG_ERR("Specified device is not a valid type.\n");
               Ret_Val = QCLI_STATUS_USAGE_E;
            }
         }
         else
         {
            LOG_ERR("Device ID is not valid.\n");
            Ret_Val = QCLI_STATUS_USAGE_E;
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
   @brief Executes the "PermitJoin" command to allow new devices to join the
          network.


   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_api_ZB_PermitJoin(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t Ret_Val;
   qapi_Status_t         Result;

   /* Verify the ZigBee layer had been initialized. */
   if(ZigBee_Demo_Context.ZigBee_Handle != NULL)
   {
      if((Parameter_Count >= 1) &&
         (Verify_Integer_Parameter(&(Parameter_List[0]), 0x00, 0xFF)))
      {
         Result = qc_drv_ZB_Permit_Join(qc_api_get_qc_drv_context(), ZigBee_Demo_Context.ZigBee_Handle, ((uint8_t)(Parameter_List[0].Integer_Value)));

         if(Result == QAPI_OK)
         {
            Display_Function_Success("qc_drv_ZB_Permit_Join");
            Ret_Val = QCLI_STATUS_SUCCESS_E;
         }
         else
         {
            Display_Function_Error("qc_drv_ZB_Permit_Join", Result);
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
   @brief Executes the "BindRequest" to create a binding between two endpoints.

   Parameter_List[0] is the device ID for the target device.  The address for
                     this device be a network address.
   Parameter_List[1] is the device ID for the source device of the bind.  The
                     address for this device be an extended address.
   Parameter_List[2] is the device ID for the destination device of the bind.
                     The address for this device be a group or extended address.
   Parameter_List[3] is the cluster ID for the bind.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_api_cmd_ZB_BindRequest(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t   Ret_Val;
   qapi_ZB_ZDP_Bind_Req_t  Request;
   qapi_Status_t           Result;
   ZB_Device_ID_t         *TargetDevice;
   ZB_Device_ID_t         *SrcDevice;
   ZB_Device_ID_t         *DstDevice;

   if(ZigBee_Demo_Context.ZigBee_Handle != NULL)
   {
      /* Check the parameters. */
      if((Parameter_Count >= 4) &&
         (Verify_Integer_Parameter(&(Parameter_List[3]), 0x0000, 0xFFFF)))
      {
         /* Get the device information. */
         TargetDevice = GetDeviceListEntry(Parameter_List[0].Integer_Value);
         SrcDevice    = GetDeviceListEntry(Parameter_List[1].Integer_Value);
         DstDevice    = GetDeviceListEntry(Parameter_List[2].Integer_Value);

         if((TargetDevice != NULL) && (SrcDevice != NULL) && (DstDevice != NULL))
         {
            /* Set Request to a known state. */
            memset((void *)&Request, 0 , sizeof(qapi_ZB_ZDP_Bind_Req_t));

            /* Verify the address types for each specified device is valid. */
            if((TargetDevice->Type == QAPI_ZB_ADDRESS_MODE_SHORT_ADDRESS_E) &&
               (SrcDevice->Type == QAPI_ZB_ADDRESS_MODE_EXTENDED_ADDRESS_E) &&
               ((DstDevice->Type == QAPI_ZB_ADDRESS_MODE_GROUP_ADDRESS_E) || (DstDevice->Type == QAPI_ZB_ADDRESS_MODE_EXTENDED_ADDRESS_E)))
            {
               /* Set up the Request. */
               Request.DestNwkAddr          = TargetDevice->Address.ShortAddress;
               Request.BindData.SrcAddress  = SrcDevice->Address.ExtendedAddress;
               Request.BindData.SrcEndpoint = SrcDevice->Endpoint;
               Request.BindData.ClusterId   = (uint16_t)(Parameter_List[3].Integer_Value);
               Request.BindData.DstAddrMode = DstDevice->Type;
               Request.BindData.DstAddress  = DstDevice->Address;
               Request.BindData.DstEndpoint = DstDevice->Endpoint;

               /* Issue the bind request. */
               Result = qc_drv_ZB_ZDP_Bind_Req(qc_api_get_qc_drv_context(), ZigBee_Demo_Context.ZigBee_Handle, &Request);
               if(Result == QAPI_OK)
               {
                  Display_Function_Success("qc_drv_ZB_ZDP_Bind_Req");
                  Ret_Val = QCLI_STATUS_SUCCESS_E;
               }
               else
               {
                  Display_Function_Error("qc_drv_ZB_ZDP_Bind_Req", Result);
                  Ret_Val = QCLI_STATUS_ERROR_E;
               }
            }
            else
            {
               LOG_ERR("Specified device is not a valid type.\n");
               Ret_Val = QCLI_STATUS_USAGE_E;
            }
         }
         else
         {
            LOG_ERR("Device ID is not valid.\n");
            Ret_Val = QCLI_STATUS_USAGE_E;
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
   @brief Executes the "EndBind" to create a binding between two endpoints.

   Parameter_List[0] is the device ID for the target device.  The address for
                     this device be a network address.
   Parameter_List[1] is the device ID for the source device of the bind.  The
                     address for this device be an extended address.
   Parameter_List[2] is the Cluster ID for the bind.
   Parameter_List[3] is a flag indicating if it is a server or client cluster..

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_api_cmd_ZB_EndBind(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t              Ret_Val;
   qapi_ZB_ZDP_End_Device_Bind_Req_t  Request;
   qapi_Status_t                      Result;
   ZB_Device_ID_t                    *TargetDevice;
   ZB_Device_ID_t                    *SrcDevice;
   uint16_t                           ClusterID;
   qbool_t                            IsServer;

   if(ZigBee_Demo_Context.ZigBee_Handle != NULL)
   {
      /* Check the parameters. */
      if((Parameter_Count >= 4) &&
         (Verify_Integer_Parameter(&(Parameter_List[2]), 0x0000, 0xFFFF)) &&
         (Verify_Integer_Parameter(&(Parameter_List[3]), 0, 1)))
      {
         /* Get the device information. */
         TargetDevice = GetDeviceListEntry(Parameter_List[0].Integer_Value);
         SrcDevice    = GetDeviceListEntry(Parameter_List[1].Integer_Value);
         ClusterID    = (uint16_t)(Parameter_List[2].Integer_Value);
         IsServer     = (qbool_t)(Parameter_List[3].Integer_Value != 0);

         if((TargetDevice != NULL) && (SrcDevice != NULL))
         {
            /* Set Request to a known state. */
            memset((void *)&Request, 0 , sizeof(qapi_ZB_ZDP_End_Device_Bind_Req_t));

            /* Verify the address types for each specified device is valid. */
            if((TargetDevice->Type == QAPI_ZB_ADDRESS_MODE_SHORT_ADDRESS_E) &&
               (SrcDevice->Type == QAPI_ZB_ADDRESS_MODE_EXTENDED_ADDRESS_E))
            {
               /* Set up the Request. */
               Request.BindingTarget  = TargetDevice->Address.ShortAddress;
               Request.SrcIEEEAddress = SrcDevice->Address.ExtendedAddress;
               Request.SrcEndpoint    = SrcDevice->Endpoint;
               Request.ProfileID      = QAPI_ZB_CL_PROFILE_ID_HOME_AUTOMATION;

               if(IsServer)
               {
                  Request.NumInClusters  = 1;
                  Request.InClusterList  = &ClusterID;
               }
               else
               {
                  Request.NumOutClusters = 1;
                  Request.OutClusterList = &ClusterID;
               }

               /* Issue the bind request. */
               Result = qc_drv_ZB_ZDP_End_Device_Bind_Req(qc_api_get_qc_drv_context(), ZigBee_Demo_Context.ZigBee_Handle, &Request);
               if(Result == QAPI_OK)
               {
                  Display_Function_Success("qc_drv_ZB_ZDP_End_Device_Bind_Req");
                  Ret_Val = QCLI_STATUS_SUCCESS_E;
               }
               else
               {
                  Display_Function_Error("qc_drv_ZB_ZDP_End_Device_Bind_Req", Result);
                  Ret_Val = QCLI_STATUS_ERROR_E;
               }
            }
            else
            {
               LOG_ERR("Specified device is not a valid type.\n");
               Ret_Val = QCLI_STATUS_USAGE_E;
            }
         }
         else
         {
            LOG_ERR("Device ID is not valid.\n");
            Ret_Val = QCLI_STATUS_USAGE_E;
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
   @brief Executes the "GetNIB" command to read a NWK attribute.

   Parameter_List[0] Attribute ID to be read.
   Parameter_List[1] Attribute Index to read (default = 0).
   Parameter_List[2] Maximum size of the attribute (default = 128).

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_api_cmd_ZB_GetNIB(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t       Ret_Val;
   qapi_Status_t               Result;
   qapi_ZB_NIB_Attribute_ID_t  AttributeId;
   uint8_t                     AttributeIndex;
   uint16_t                    AttributeLength;
   void                       *AttributeValue;

   /* Check the parameters. */
   if(ZigBee_Demo_Context.ZigBee_Handle != NULL)
   {
      /* Check the parameters. */
      if((Parameter_Count >= 1) &&
         (Verify_Integer_Parameter(&(Parameter_List[0]), 0, 0xFFFF)))
      {
         AttributeId     = (qapi_ZB_NIB_Attribute_ID_t)(Parameter_List[0].Integer_Value);
         AttributeIndex  = 0;
         AttributeLength = 128;

         if((Parameter_Count >= 2) && (Verify_Integer_Parameter(&(Parameter_List[1]), 0, 0xFF)))
         {
            AttributeIndex = (uint8_t)(Parameter_List[1].Integer_Value);

            if((Parameter_Count >= 3) && (Verify_Integer_Parameter(&(Parameter_List[2]), 0, 0xFFFF)))
            {
               AttributeLength = (uint16_t)(Parameter_List[2].Integer_Value);
            }
         }

         /* Allocate a buffer for the attribute data. */
         AttributeValue = malloc(AttributeLength);
         if(AttributeValue != NULL)
         {
            Result = qc_drv_ZB_NLME_Get_Request(qc_api_get_qc_drv_context(), ZigBee_Demo_Context.ZigBee_Handle, AttributeId, AttributeIndex, &AttributeLength, (uint8_t *)AttributeValue);

            if(Result == QAPI_OK)
            {
               LOG_INFO("Attribute 0x%04X (%d bytes): ", AttributeId, AttributeLength);
               switch(AttributeLength)
               {
                  case sizeof(uint8_t):
                     LOG_INFO("0x%02X", *(uint8_t *)AttributeValue);
                     break;

                  case sizeof(uint16_t):
                     LOG_INFO("0x%04X", *(uint16_t *)AttributeValue);
                     break;

                  case sizeof(uint32_t):
                     LOG_INFO("0x%08X", *(uint32_t *)AttributeValue);
                     break;

                  default:
                     LOG_AT("\n");
                     Dump_Data("  ", AttributeLength, (uint8_t *)AttributeValue);
                     break;
               }
            }
            else
            {
               Display_Function_Error("qc_drv_ZB_NLME_Get_Request", Result);
               Ret_Val = QCLI_STATUS_ERROR_E;
            }

            free(AttributeValue);
         }
         else
         {
            LOG_ERR("Failed to allocate a buffer for the attribute.\n");
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
   @brief Executes the "GetNIB" command to write a NWK attribute.

   Parameter_List[0] Attribute ID to be write.
   Parameter_List[1] Attribute Index to write.
   Parameter_List[2] Size of the attribute being written in bytes.
   Parameter_List[3] Value for the attribute.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_api_cmd_ZB_SetNIB(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t       Ret_Val;
   qapi_Status_t               Result;
   qapi_ZB_NIB_Attribute_ID_t  AttributeId;
   uint8_t                     AttributeIndex;
   uint32_t                    AttributeLength;
   uint32_t                    TempLength;
   uint8_t                    *AttributeValue;

   /* Check the parameters. */
   if(ZigBee_Demo_Context.ZigBee_Handle != NULL)
   {
      /* Check the parameters. */
      if((Parameter_Count >= 4) &&
         (Verify_Integer_Parameter(&(Parameter_List[0]), 0, 0xFFFF)) &&
         (Verify_Integer_Parameter(&(Parameter_List[1]), 0, 0xFF)) &&
         (Verify_Integer_Parameter(&(Parameter_List[2]), 0, 0xFFFF)))
      {
         AttributeId     = (qapi_ZB_NIB_Attribute_ID_t)(Parameter_List[0].Integer_Value);
         AttributeIndex  = (uint8_t)(Parameter_List[1].Integer_Value);
         AttributeLength = (uint32_t)(Parameter_List[2].Integer_Value);

         /* Allocate a buffer for the attribute data. */
         AttributeValue = malloc(AttributeLength);
         if(AttributeValue != NULL)
         {
            TempLength = AttributeLength;
            if(Hex_String_To_Array(Parameter_List[3].String_Value, &TempLength, AttributeValue))
            {
               Result = qc_drv_ZB_NLME_Set_Request(qc_api_get_qc_drv_context(), ZigBee_Demo_Context.ZigBee_Handle, AttributeId, AttributeIndex, AttributeLength, AttributeValue);

               if(Result == QAPI_OK)
               {
                  Display_Function_Success("qc_drv_ZB_NLME_Set_Request");
                  Ret_Val = QCLI_STATUS_SUCCESS_E;
               }
               else
               {
                  Display_Function_Error("qc_drv_ZB_NLME_Set_Request", Result);
                  Ret_Val = QCLI_STATUS_ERROR_E;
               }
            }
            else
            {
               LOG_ERR("Failed to parse the attribute value.\n");
               Ret_Val = QCLI_STATUS_ERROR_E;
            }

            free(AttributeValue);
         }
         else
         {
            LOG_ERR("Failed to allocate a buffer for the attribute.\n");
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
   @brief Executes the "GetAIB" command to read a APS attribute.

   Parameter_List[0] Attribute ID to be read.
   Parameter_List[1] Attribute Index to read.
   Parameter_List[2] Maximum size of the attribute.  Defaults to 128 if not
                     specified.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_api_cmd_ZB_GetAIB(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t       Ret_Val;
   qapi_Status_t               Result;
   qapi_ZB_AIB_Attribute_ID_t  AttributeId;
   uint8_t                     AttributeIndex;
   uint16_t                    AttributeLength;
   void                       *AttributeValue;

   /* Check the parameters. */
   if(ZigBee_Demo_Context.ZigBee_Handle != NULL)
   {
      /* Check the parameters. */
      if((Parameter_Count >= 1) &&
         (Verify_Integer_Parameter(&(Parameter_List[0]), 0, 0xFFFF)))
      {
         AttributeId     = (qapi_ZB_AIB_Attribute_ID_t)(Parameter_List[0].Integer_Value);
         AttributeIndex  = 0;
         AttributeLength = 128;

         if((Parameter_Count >= 2) && (Verify_Integer_Parameter(&(Parameter_List[1]), 0, 0xFF)))
         {
            AttributeIndex = (uint8_t)(Parameter_List[1].Integer_Value);

            if((Parameter_Count >= 3) && (Verify_Integer_Parameter(&(Parameter_List[2]), 0, 0xFFFF)))
            {
               AttributeLength = (uint16_t)(Parameter_List[2].Integer_Value);
            }
         }

         /* Allocate a buffer for the attribute data. */
         AttributeValue = malloc(AttributeLength);
         if(AttributeValue != NULL)
         {
            Result = qc_drv_ZB_APSME_Get_Request(qc_api_get_qc_drv_context(), ZigBee_Demo_Context.ZigBee_Handle, AttributeId, AttributeIndex, &AttributeLength, (uint8_t *)AttributeValue);

            if(Result == QAPI_OK)
            {
               LOG_INFO("Attribute %d (%d bytes): ", AttributeId, AttributeLength);
               switch(AttributeLength)
               {
                  case sizeof(uint8_t):
                     LOG_INFO("0x%02X", *(uint8_t *)AttributeValue);
                     break;

                  case sizeof(uint16_t):
                     LOG_INFO("0x%04X", *(uint16_t *)AttributeValue);
                     break;

                  case sizeof(uint32_t):
                     LOG_INFO("0x%08X", *(uint32_t *)AttributeValue);
                     break;

                  default:
                     LOG_AT("\n");
                     Dump_Data("  ", AttributeLength, (uint8_t *)AttributeValue);
                     break;
               }
            }
            else
            {
               Display_Function_Error("qc_drv_ZB_APSME_Get_Request", Result);
               Ret_Val = QCLI_STATUS_ERROR_E;
            }

            free(AttributeValue);
         }
         else
         {
            LOG_ERR("Failed to allocate a buffer for the attribute.\n");
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
   @brief Executes the "GetAIB" command to write a APS attribute.

   Parameter_List[0] Attribute ID to be write.
   Parameter_List[1] Attribute Index to write.
   Parameter_List[2] Size of the attribute being written in bytes.
   Parameter_List[3] Value for the attribute.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_api_cmd_ZB_SetAIB(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t       Ret_Val;
   qapi_Status_t               Result;
   qapi_ZB_AIB_Attribute_ID_t  AttributeId;
   uint8_t                     AttributeIndex;
   uint16_t                    AttributeLength;
   uint32_t                    TempLength;
   uint8_t                    *AttributeValue;

   /* Check the parameters. */
   if(ZigBee_Demo_Context.ZigBee_Handle != NULL)
   {
      /* Check the parameters. */
      if((Parameter_Count >= 4) &&
         (Verify_Integer_Parameter(&(Parameter_List[0]), 0, 0xFFFF)) &&
         (Verify_Integer_Parameter(&(Parameter_List[1]), 0, 0xFF)) &&
         (Verify_Integer_Parameter(&(Parameter_List[2]), 0, 0xFFFF)))
      {
         AttributeId     = (qapi_ZB_AIB_Attribute_ID_t)(Parameter_List[0].Integer_Value);
         AttributeIndex  = (uint8_t)(Parameter_List[1].Integer_Value);
         AttributeLength = (uint32_t)(Parameter_List[2].Integer_Value);

         /* Allocate a buffer for the attribute data. */
         AttributeValue = malloc(AttributeLength);
         if(AttributeValue != NULL)
         {
            TempLength = AttributeLength;
            if(Hex_String_To_Array(Parameter_List[3].String_Value, &TempLength, AttributeValue))
            {
               Result = qc_drv_ZB_APSME_Set_Request(qc_api_get_qc_drv_context(), ZigBee_Demo_Context.ZigBee_Handle, AttributeId, AttributeIndex, &AttributeLength, AttributeValue);

               if(Result == QAPI_OK)
               {
                  Display_Function_Success("qc_drv_ZB_APSME_Set_Request");
                  Ret_Val = QCLI_STATUS_SUCCESS_E;
               }
               else
               {
                  Display_Function_Error("qc_drv_ZB_APSME_Set_Request", Result);
                  Ret_Val = QCLI_STATUS_ERROR_E;
               }
            }
            else
            {
               LOG_ERR("Failed to parse the attribute value.\n");
               Ret_Val = QCLI_STATUS_ERROR_E;
            }

            free(AttributeValue);
         }
         else
         {
            LOG_ERR("Failed to allocate a buffer for the attribute.\n");
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
   @brief Executes the "SetBIB" command.

   Parameter_List[0] is the ID of BDB attributes. (AttrId)
   Parameter_List[1] is the index within an attribute identified by the AttrId.
                     (AttrIndex)
   Parameter_List[2] is the length of the attribute. (AttrLength)
   Parameter_List[3] is the value of the attribute. (AttrValue)

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_api_cmd_ZB_SetBIB(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t  Ret_Val;
   qapi_Status_t          Result;
   uint32_t               MaxValue;
   uint64_t               AttrValueULL;
   uint16_t               AttrId;
   uint8_t                AttrIndex;
   uint16_t               AttrLength;
   uint8_t               *AttrValue;

   if(ZigBee_Demo_Context.ZigBee_Handle != NULL)
   {
      if((Parameter_Count >= 4) &&
         (Verify_Integer_Parameter(&Parameter_List[0], 0x0000, 0xFFFF)) &&
         (Verify_Integer_Parameter(&Parameter_List[1], 0x00, 0xFF)) &&
         (Verify_Integer_Parameter(&Parameter_List[2], 1, 0xFF))
         )
      {
         AttrId     = (uint16_t)(Parameter_List[0].Integer_Value);
         AttrIndex  = (uint8_t)(Parameter_List[1].Integer_Value);
         AttrLength = (uint8_t)(Parameter_List[2].Integer_Value);
         switch(AttrLength)
         {
            case sizeof(uint8_t):
            case sizeof(uint16_t):
            case sizeof(uint32_t):
               /* Handle the basic integer types.                       */
               MaxValue = 0xFFFFFFFF >> ((sizeof(uint32_t) - AttrLength) * 8);

               if(Verify_Unsigned_Integer_Parameter(&(Parameter_List[3]), 0, MaxValue))
               {
                  AttrValue = (uint8_t *)&(Parameter_List[3].Integer_Value);

                  Ret_Val = QCLI_STATUS_SUCCESS_E;
               }
               else
               {
                  Ret_Val = QCLI_STATUS_ERROR_E;
               }
               break;

            case sizeof(uint64_t):
                  /* Attempt to convert the string to a 64-bit integer. */
                  if(Hex_String_To_ULL(Parameter_List[3].String_Value, &AttrValueULL))
                  {
                     AttrValue = (uint8_t *)&AttrValueULL;

                     Ret_Val = QCLI_STATUS_SUCCESS_E;
                  }
                  else
                  {
                     Ret_Val = QCLI_STATUS_ERROR_E;
                  }
               break;

            default:
               Ret_Val = QCLI_STATUS_ERROR_E;
               break;
         }

         if(Ret_Val == QCLI_STATUS_SUCCESS_E)
         {
            Result = qc_drv_ZB_BDB_Set_Request(qc_api_get_qc_drv_context(), ZigBee_Demo_Context.ZigBee_Handle , AttrId, AttrIndex, AttrLength, AttrValue);
            if(Result == QAPI_OK)
            {
               Display_Function_Success("qc_drv_ZB_BDB_Set_Request");
            }
            else
            {
               Ret_Val = QCLI_STATUS_ERROR_E;
               Display_Function_Error("qc_drv_ZB_BDB_Set_Request", Result);
            }

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
   @brief Executes the "GetBIB" command.

   Parameter_List[0] is the ID of BDB attributes. (AttrId)
   Parameter_List[1] is the index within an attribute identified by the AttrId.
                     (AttrIndex)
   Parameter_List[2] is the length of the attribute. (AttrLength)

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_api_cmd_ZB_GetBIB(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t  Ret_Val;
   qapi_Status_t          Result;
   uint16_t               AttrId;
   uint8_t                AttrIndex;
   uint16_t               AttrLength;
   uint8_t                AttrValue[MAXIMUM_ATTRIUBTE_LENGTH];

   if(ZigBee_Demo_Context.ZigBee_Handle != NULL)
   {
      /* Check the parameters.                                          */
      if((Parameter_Count >= 3) &&
         (Verify_Integer_Parameter(&(Parameter_List[0]), 0x0000, 0xFFFF)) &&
         (Verify_Integer_Parameter(&(Parameter_List[1]), 0x00, 0xFF)) &&
         (Verify_Integer_Parameter(&(Parameter_List[2]), 0, sizeof(AttrValue)))
        )
      {
         /* Get the device information.                                 */
         AttrId     = (uint16_t)(Parameter_List[0].Integer_Value);
         AttrIndex  = (uint8_t)(Parameter_List[1].Integer_Value);
         AttrLength = (uint16_t)(Parameter_List[2].Integer_Value);

         Result = qc_drv_ZB_BDB_Get_Request(qc_api_get_qc_drv_context(), ZigBee_Demo_Context.ZigBee_Handle, (qapi_ZB_BDB_Attribute_ID_t)AttrId, AttrIndex, &AttrLength, AttrValue);
         switch(Result)
         {
            case QAPI_ERR_BOUNDS:
               Display_Function_Error("qc_drv_ZB_BDB_Get_Request", Result);


               LOG_INFO("AttrLength: %d\n", AttrLength);
               Ret_Val = QCLI_STATUS_USAGE_E;
               break;

            case QAPI_OK:
               Display_Function_Success("qc_drv_ZB_BDB_Get_Request");

               LOG_INFO("   AttrId:     0x%04X\n", AttrId);
               LOG_INFO("   AttrLength: %d\n", AttrLength);
               LOG_INFO("   AttrValue:  ");
               DisplayVariableLengthValue(AttrLength, AttrValue);
               Ret_Val = QCLI_STATUS_SUCCESS_E;
               break;

            default:
               Display_Function_Error("qc_drv_ZB_BDB_Get_Request", Result);
               Ret_Val = QCLI_STATUS_ERROR_E;
               break;
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
   @brief Executes the "SetExtAddress" command to set the extended address of
          the local ZigBee interface.

   Parameter_List[0] EUI64 address to assign to the ZigBee interface.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_api_cmd_ZB_SetExtAddress(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t Ret_Val;
   uint64_t              ExtendedAddress;
   qapi_Status_t         Result;

   /* Verify the ZigBee layer had been initialized. */
   if(ZigBee_Demo_Context.ZigBee_Handle != NULL)
   {
      if((Parameter_Count >= 1) &&
         (Hex_String_To_ULL(Parameter_List[0].String_Value, &ExtendedAddress)))
      {
         Result = qc_drv_ZB_Set_Extended_Address(qc_api_get_qc_drv_context(), ZigBee_Demo_Context.ZigBee_Handle, ExtendedAddress);

         if(Result == QAPI_OK)
         {
            Display_Function_Success("qc_drv_ZB_Set_Extended_Address");
            Ret_Val = QCLI_STATUS_SUCCESS_E;
         }
         else
         {
            Display_Function_Error("qc_drv_ZB_Set_Extended_Address", Result);
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
   @brief Executes the "GetAddresses" command to get the addresses of the local
          ZigBee interface.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_api_cmd_ZB_GetAddresses(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t Ret_Val;
   qapi_Status_t         Result;
   uint64_t              ExtendedAddress;
   uint16_t              ShortAddress;
   uint16_t              AttributeLength;

   /* Verify the ZigBee layer had been initialized. */
   if(ZigBee_Demo_Context.ZigBee_Handle != NULL)
   {
      Result = qc_drv_ZB_Get_Extended_Address(qc_api_get_qc_drv_context(), ZigBee_Demo_Context.ZigBee_Handle, &ExtendedAddress);
      if(Result == QAPI_OK)
      {
         AttributeLength = sizeof(uint16_t);
         Result = qc_drv_ZB_NLME_Get_Request(qc_api_get_qc_drv_context(), ZigBee_Demo_Context.ZigBee_Handle, QAPI_ZB_NIB_ATTRIBUTE_ID_NWK_NETWORK_ADDRESS_E, 0, &AttributeLength, (uint8_t *)&ShortAddress);
         if((Result == QAPI_OK) && (AttributeLength == sizeof(uint16_t)))
         {
            LOG_INFO("Extended Address: %08X%08X\n", (uint32_t)(ExtendedAddress >> 32), (uint32_t)ExtendedAddress);
            LOG_INFO("Short Address:    0x%04X\n", ShortAddress);
            Ret_Val = QCLI_STATUS_SUCCESS_E;
         }
         else
         {
            Display_Function_Error("qc_drv_ZB_NLME_Get_Request", Result);
            Ret_Val = QCLI_STATUS_ERROR_E;
         }
      }
      else
      {
         Display_Function_Error("qc_drv_ZB_Get_Extended_Address", Result);
         Ret_Val = QCLI_STATUS_ERROR_E;
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
   @brief Executes the "ListClusterTypes" command to list the clusters that are
          supported by the demo.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
#if 0
QCLI_Command_Status_t qc_api_cmd_ZB_CL_ListClusterTypes(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   uint16_t Index;

   LOG_INFO("Clusters:\n");

   for(Index = 0; Index < CLUSTER_DECRIPTOR_LIST_SIZE; Index++)
   {
      LOG_INFO("  0x%04X: %s\n", ClusterDescriptorList[Index].ClusterID, ClusterDescriptorList[Index].ClusterName);
   }

   return(QCLI_STATUS_SUCCESS_E);
}

/**
   @brief Executes the "ListEndpointTypes" command to list the endpoint types
          that can be created by the demo.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_api_cmd_ZB_CL_ListEndpointTypes(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   uint16_t Index;

   LOG_INFO("Endpoint Types:\n");

   for(Index = 0; Index < ENDPOINT_DECRIPTOR_LIST_SIZE; Index++)
   {
      LOG_INFO(" %2d. %s\n", Index + ZCL_DEMO_ENDPOINT_TYPE_START_INDEX, EndpointDescriptorList[Index].EndpointName);
   }

   return(QCLI_STATUS_SUCCESS_E);
}

/**
   @brief Executes the "CreateEndpiont" command to create a new endpoint.

   Parameter_List[0] Number to use for the endpoint.
   Parameter_List[1] Type of endpoint to create.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_api_cmd_ZB_CL_CreateEndpoint(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t            Ret_Val;
   qapi_Status_t                    Result;
   qapi_ZB_Handle_t                 ZigBee_Handle;
   qapi_ZB_APS_Add_Endpoint_t       Endpoint_Data;
   uint8_t                          Endpoint;
   const ZCL_Endpoint_Descriptor_t *EndpointDescriptor;
   uint8_t                          Capability;
   uint16_t                         AttributeLength;

   ZigBee_Handle = GetZigBeeHandle();
   if(ZigBee_Handle != NULL)
   {
      if((Parameter_Count >= 2) &&
         (Verify_Integer_Parameter(&Parameter_List[0], QAPI_ZB_APS_MIN_ENDPOINT, QAPI_ZB_APS_MAX_ENDPOINT)) &&
         (Verify_Integer_Parameter(&Parameter_List[1], ZCL_DEMO_ENDPOINT_TYPE_START_INDEX, ENDPOINT_DECRIPTOR_LIST_SIZE + ZCL_DEMO_ENDPOINT_TYPE_START_INDEX)))
      {
         Endpoint           = Parameter_List[0].Integer_Value;
         EndpointDescriptor = &(EndpointDescriptorList[Parameter_List[1].Integer_Value - ZCL_DEMO_ENDPOINT_TYPE_START_INDEX]);

         /* Make sure the endpoint doesn't already exist. */
         if(ZCL_FindClusterByEndpoint(Endpoint, ZCL_DEMO_IGNORE_CLUSTER_ID, ZCL_DEMO_CLUSTERTYPE_UNKNOWN) == NULL)
         {
            memset(&Endpoint_Data, 0, sizeof(qapi_ZB_APS_Add_Endpoint_t));
            Endpoint_Data.Endpoint                = Endpoint;
            Endpoint_Data.Version                 = 1;
            Endpoint_Data.DeviceId                = EndpointDescriptor->DeviceID;
            Endpoint_Data.OutputClusterCount      = EndpointDescriptor->ClientClusterCount;
            Endpoint_Data.OutputClusterList       = EndpointDescriptor->ClientClusterList;
            Endpoint_Data.InputClusterCount       = EndpointDescriptor->ServerClusterCount;
            Endpoint_Data.InputClusterList        = EndpointDescriptor->ServerClusterList;
            Endpoint_Data.BDBCommissioningGroupId = QAPI_ZB_BDB_COMMISSIONING_DFAULT_GROUP_ID;
            Endpoint_Data.BDBCommissioningMode    = QAPI_ZB_BDB_COMMISSIONING_MODE_DEFAULT;

            if(EndpointDescriptor->Touchlink)
            {
               Endpoint_Data.ProfileId             = QAPI_ZB_CL_PROFILE_ID_ZIGBEE_LIGHT_LINK;
               Endpoint_Data.BDBCommissioningMode |= QAPI_ZB_BDB_COMMISSIONING_MODE_TOUCHLINK;
            }
            else
            {
               Endpoint_Data.ProfileId = QAPI_ZB_CL_PROFILE_ID_HOME_AUTOMATION;
            }

            /* Initialize the clusters for the endpoint. */
            if(ZCL_InitializeClusters(Endpoint, EndpointDescriptor->EndpointName, true, Endpoint_Data.InputClusterList, Endpoint_Data.InputClusterCount))
            {
               if(ZCL_InitializeClusters(Endpoint, EndpointDescriptor->EndpointName, false, Endpoint_Data.OutputClusterList, Endpoint_Data.OutputClusterCount))
               {
                  /* Create the endpoint. */
                  Result = qc_drv_ZB_APS_Add_Endpoint(qc_api_get_qc_drv_context(), ZigBee_Handle, &Endpoint_Data);
                  if(Result == QAPI_OK)
                  {
                     if(EndpointDescriptor->Touchlink)
                     {
                        /* Set touchlink support in the commissioning
                           capabilities BIB. */
                        AttributeLength = sizeof(Capability);
                        Result = qc_drv_ZB_BDB_Get_Request(qc_api_get_qc_drv_context(), ZigBee_Handle, QAPI_ZB_BDB_ATTRIBUTE_ID_BDB_NODE_COMMISSIONING_CAPABILITY_E, 0, &AttributeLength, &Capability);
                        if(Result == QAPI_OK)
                        {
                           Capability |= QAPI_ZB_BDB_COMMISSIONING_CAPABILITY_TOUCHLINK;

                           Result = qc_drv_ZB_BDB_Set_Request(qc_api_get_qc_drv_context(), ZigBee_Handle, QAPI_ZB_BDB_ATTRIBUTE_ID_BDB_NODE_COMMISSIONING_CAPABILITY_E, 0, sizeof(Capability), &Capability);
                           if(Result == QAPI_OK)
                           {
                              Ret_Val = QCLI_STATUS_SUCCESS_E;
                           }
                           else
                           {
                              Display_Function_Error("qc_drv_ZB_BDB_Set_Request", Result);
                              Ret_Val = QCLI_STATUS_ERROR_E;
                           }
                        }
                        else
                        {
                           Display_Function_Error("qc_drv_ZB_BDB_Get_Request", Result);
                           Ret_Val = QCLI_STATUS_ERROR_E;
                        }
                     }
                     else
                     {
                        Ret_Val = QCLI_STATUS_SUCCESS_E;
                     }

                     if(Ret_Val == QCLI_STATUS_SUCCESS_E)
                     {
                        LOG_INFO("Endpoint %d Initialized as %s.\n", Endpoint, EndpointDescriptor->EndpointName);
                     }
                  }
                  else
                  {
                     Display_Function_Error("qapi_ZB_APS_Add_Endpoint", Result);
                     Ret_Val = QCLI_STATUS_ERROR_E;
                  }
               }
               else
               {
                  Ret_Val = QCLI_STATUS_ERROR_E;
               }
            }
            else
            {
               Ret_Val = QCLI_STATUS_ERROR_E;
            }

            if(Ret_Val != QCLI_STATUS_SUCCESS_E)
            {
               /* Cleanup the endpoint clusters if creation failed. */
               ZCL_RemoveClusterByEndpoint(Endpoint);
            }
         }
         else
         {
            LOG_INFO("Endpoint already exists.\n");
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
   @brief Executes the "RemoveEndpoint" command to remove an endpoint.

   Parameter_List[0] Number to use for the endpoint.
   Parameter_List[1] Type of endpoint to create.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_api_cmd_ZB_CL_RemoveEndpoint(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t  Ret_Val;
   qapi_Status_t          Result;
   qapi_ZB_Handle_t ZigBee_Handle;
   uint8_t                Endpoint;

   ZigBee_Handle = GetZigBeeHandle();
   if(ZigBee_Handle != NULL)
   {
      if((Parameter_Count >= 1) &&
         (Verify_Integer_Parameter(&(Parameter_List[0]), QAPI_ZB_APS_MIN_ENDPOINT, QAPI_ZB_APS_MAX_ENDPOINT)))
      {
         Endpoint = Parameter_List[0].Integer_Value;

         /* Remove the endpoint. */
         Result = qc_drv_ZB_APS_Remove_Endpoint(qc_api_get_qc_drv_context(), ZigBee_Handle, Endpoint);
         if(Result == QAPI_OK)
         {
            Ret_Val = QCLI_STATUS_SUCCESS_E;
            Display_Function_Success("qc_drv_ZB_APS_Remove_Endpoint");
         }
         else
         {
            Display_Function_Error("qc_drv_ZB_APS_Remove_Endpoint", Result);
            Ret_Val = QCLI_STATUS_ERROR_E;
         }

         /* Remove the clusters for the endpoint. */
         ZCL_RemoveClusterByEndpoint(Endpoint);
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
   @brief Executes the "ListClusters" command to create a switch endpoint.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_api_cmd_ZB_CL_ListClusters(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t Ret_Val;
   uint16_t              Index;

   Ret_Val = QCLI_STATUS_SUCCESS_E;
   if(ZCL_Demo_Context.Cluster_Count > 0)
   {
      LOG_INFO("Cluster List:\n");
      LOG_INFO(" ID | EP  | Cluster Name     | Type   | Device Name\n");
      LOG_INFO("----+-----+------------------+--------+--------------------------\n");

      for(Index = 0; Index < ZCL_Demo_Context.Cluster_Count; Index++)
      {
         LOG_INFO(" %2d | %3d | %-16s | %-6s | %s\n", Index, ZCL_Demo_Context.Cluster_List[Index].Endpoint, ZCL_Demo_Context.Cluster_List[Index].ClusterName, (ZCL_Demo_Context.Cluster_List[Index].ClusterType == ZCL_DEMO_CLUSTERTYPE_SERVER) ? "Server" : "Client", ZCL_Demo_Context.Cluster_List[Index].DeviceName);
      }
   }
   else
   {
      LOG_INFO("The cluster list is empty.\n");
   }

   return(Ret_Val);
}

/**
   @brief Executes the "ReadLocalAttribute" command to read a local attribute.

   Parameter_List[0] Index of the local cluster in the cluster list whose
                     attribute will be read.
   Parameter_List[1] ID of the attribute to be read.
   Parameter_List[2] Length of the attribute to read.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_api_cmd_ZB_CL_ReadLocalAttribute(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t    Ret_Val;
   qapi_Status_t            Result;
   ZCL_Demo_Cluster_Info_t *ClusterInfo;
   uint16_t                 AttrId;
   uint16_t                 AttrLength;
   uint8_t                  AttrValue[MAXIMUM_ATTRIUBTE_LENGTH];

   if(GetZigBeeHandle() != NULL)
   {
      if((Parameter_Count >= 3) &&
         (Verify_Integer_Parameter(&Parameter_List[0], 0, ZCL_Demo_Context.Cluster_Count - 1)) &&
         (Verify_Integer_Parameter(&Parameter_List[1], 0x0000, 0xFFFF)) &&
         (Verify_Integer_Parameter(&Parameter_List[2], 0, sizeof(AttrValue))))
      {
         ClusterInfo = ZCL_FindClusterByIndex((uint16_t)(Parameter_List[0].Integer_Value), ZCL_DEMO_IGNORE_CLUSTER_ID);
         AttrId      = (uint16_t)(Parameter_List[1].Integer_Value);
         AttrLength  = (uint16_t)(Parameter_List[2].Integer_Value);

         if(ClusterInfo != NULL)
         {
            Result = qc_drv_ZB_CL_Read_Local_Attribute(qc_api_get_qc_drv_context(), ClusterInfo->Handle, AttrId, &AttrLength, AttrValue);
            switch(Result)
            {
               case QAPI_ERR_BOUNDS:
                  Display_Function_Error("qc_drv_ZB_CL_Read_Local_Attribute", Result);

                  LOG_INFO("AttrLength: %d\n", AttrLength);
                  Ret_Val = QCLI_STATUS_USAGE_E;
                  break;

               case QAPI_OK:
                  Display_Function_Success("qc_drv_ZB_CL_Read_Local_Attribute");

                  LOG_INFO("   AttrId:     0x%04X\n", AttrId);
                  LOG_INFO("   AttrLength: %d\n", AttrLength);
                  LOG_INFO("   AttrValue:  ");
                  DisplayVariableLengthValue(AttrLength, AttrValue);
                  Ret_Val = QCLI_STATUS_SUCCESS_E;
                  break;

               default:
                  Display_Function_Error("qc_drv_ZB_CL_Read_Local_Attribute", Result);

                  Ret_Val = QCLI_STATUS_ERROR_E;
                  break;
            }
         }
         else
         {
            LOG_ERR("Invalid ClusterIndex.\n");
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
   @brief Executes the "WriteLocalAttribute" command to write a local attribute.

   Parameter_List[0] Index of the local cluster in the cluster list whose
                     attribute will be written.
   Parameter_List[1] ID of the attribute to be written.
   Parameter_List[2] Length of the attribute to be written.
   Parameter_List[3] Value that will be wriiten into the attribute.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_api_cmd_ZB_CL_WriteLocalAttribute(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t    Ret_Val;
   qapi_Status_t            Result;
   ZCL_Demo_Cluster_Info_t *ClusterInfo;
   uint32_t                 MaxValue;
   uint64_t                 AttrValueULL;
   uint16_t                 AttrId;
   uint16_t                 AttrLength;
   uint8_t                 *AttrValue;

   if(GetZigBeeHandle() != NULL)
   {
      if((Parameter_Count >= 4) &&
         (Verify_Integer_Parameter(&Parameter_List[0], 0, ZCL_Demo_Context.Cluster_Count - 1)) &&
         (Verify_Integer_Parameter(&Parameter_List[1], 0x0000, 0xFFFF)) &&
         (Verify_Integer_Parameter(&Parameter_List[2], 1, 0xFF)))
      {
         ClusterInfo = ZCL_FindClusterByIndex((uint16_t)(Parameter_List[0].Integer_Value), ZCL_DEMO_IGNORE_CLUSTER_ID);
         AttrId      = (uint16_t)(Parameter_List[1].Integer_Value);
         AttrLength  = (uint8_t)(Parameter_List[2].Integer_Value);

         if(ClusterInfo != NULL)
         {
            switch(AttrLength)
            {
               case sizeof(uint8_t):
               case sizeof(uint16_t):
               case sizeof(uint32_t):
                  /* Handle the basic integer types. */
                  MaxValue = 0xFFFFFFFF >> ((sizeof(uint32_t) - AttrLength) * 8);

                  if(Verify_Unsigned_Integer_Parameter(&(Parameter_List[3]), 0, MaxValue))
                  {
                     AttrValue = (uint8_t *)&(Parameter_List[3].Integer_Value);

                     Ret_Val = QCLI_STATUS_SUCCESS_E;
                  }
                  else
                  {
                     Ret_Val = QCLI_STATUS_ERROR_E;
                  }
                  break;

               case sizeof(uint64_t):
                     /* Attempt to convert the string to a 64-bit integer. */
                     if(Hex_String_To_ULL(Parameter_List[3].String_Value, &AttrValueULL))
                     {
                        AttrValue = (uint8_t *)&AttrValueULL;

                        Ret_Val = QCLI_STATUS_SUCCESS_E;
                     }
                     else
                     {
                        Ret_Val = QCLI_STATUS_ERROR_E;
                     }
                  break;

               default:
                  Ret_Val = QCLI_STATUS_ERROR_E;
                  break;
            }

            if(Ret_Val == QCLI_STATUS_SUCCESS_E)
            {
               Result = qc_drv_ZB_CL_Write_Local_Attribute(qc_api_get_qc_drv_context(), ClusterInfo->Handle, AttrId, &AttrLength, AttrValue);
               if(Result == QAPI_OK)
               {
                  Display_Function_Success("qc_drv_ZB_CL_Write_Local_Attribute");
               }
               else
               {
                  Ret_Val = QCLI_STATUS_ERROR_E;
                  Display_Function_Error("qc_drv_ZB_CL_Write_Local_Attribute", Result);
               }

            }
         }
         else
         {
            LOG_ERR("Invalid ClusterIndex.\n");
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
   @brief Executes the "ReadAttribute" command to read attribute.

   Parameter_List[0] ID of the device on which the attribute will be read.
   Parameter_List[1] Index of the cluster on the device which contains the
                     attribute to be read.
   Parameter_List[2] ID of the attribute to be read.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_api_cmd_ZB_CL_ReadAttribute(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t           Ret_Val;
   qapi_Status_t                   Result;
   qapi_ZB_CL_General_Send_Info_t  SendInfo;
   ZCL_Demo_Cluster_Info_t        *ClusterInfo;
   uint8_t                         AttrCount;
   uint16_t                        AttrID;
   uint32_t                        DeviceId;

   if(GetZigBeeHandle() != NULL)
   {
      Ret_Val = QCLI_STATUS_SUCCESS_E;

      if((Parameter_Count >= 3) &&
         (Parameter_List[0].Integer_Is_Valid) &&
         (Verify_Integer_Parameter(&(Parameter_List[1]), 0, ZCL_Demo_Context.Cluster_Count - 1)) &&
         (Verify_Integer_Parameter(&(Parameter_List[2]), 0x0000, 0xFFFF)))
      {
         DeviceId    = Parameter_List[0].Integer_Value;
         ClusterInfo = ZCL_FindClusterByIndex((uint16_t)(Parameter_List[1].Integer_Value), ZCL_DEMO_IGNORE_CLUSTER_ID);

         if(ClusterInfo != NULL)
         {
            memset(&SendInfo, 0, sizeof(qapi_ZB_CL_General_Send_Info_t));

            /* Format the destination addr. mode, address, and endpoint. */
            if(Format_Send_Info_By_Device(DeviceId, &SendInfo))
            {
               AttrCount = 1;
               AttrID    = (uint16_t)(Parameter_List[2].Integer_Value);

               Result = qc_drv_ZB_CL_Read_Attributes(qc_api_get_qc_drv_context(), ClusterInfo->Handle, &SendInfo, AttrCount, &AttrID);
               if(Result == QAPI_OK)
               {
                  Display_Function_Success("qc_drv_ZB_CL_Read_Attributes");
               }
               else
               {
                  Ret_Val = QCLI_STATUS_ERROR_E;
                  Display_Function_Error("qc_drv_ZB_CL_Read_Attributes", Result);
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
            LOG_ERR("Invalid ClusterIndex.\n");
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
   @brief Executes the "WriteAttribute" command to write attribute.

   Parameter_List[0] ID of the device on which the attribute will be written.
   Parameter_List[1] Index of the cluster on the device which contains the
                     attribute to be written.
   Parameter_List[2] ID of the attribute to be written.
   Parameter_List[3] Type of the attribute to be written.
   Parameter_List[4] Length of the attribute to be written.
   Parameter_List[5] Value that will be wriiten into the attribute.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_api_cmd_ZB_CL_WriteAttribute(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t           Ret_Val;
   qapi_Status_t                   Result;
   qapi_ZB_CL_General_Send_Info_t  SendInfo;
   ZCL_Demo_Cluster_Info_t        *ClusterInfo;
   uint32_t                        MaxValue;
   uint64_t                        AttrValueULL;
   qapi_ZB_CL_Write_Attr_Record_t  AttrRecord;
   uint32_t                        DeviceId;

   if(GetZigBeeHandle() != NULL)
   {
      Ret_Val = QCLI_STATUS_SUCCESS_E;

      if((Parameter_Count >= 6) &&
         (Parameter_List[0].Integer_Is_Valid) &&
         (Verify_Integer_Parameter(&(Parameter_List[1]), 0, ZCL_Demo_Context.Cluster_Count - 1)) &&
         (Verify_Integer_Parameter(&(Parameter_List[2]), 0x0000, 0xFFFF)) &&
         (Verify_Integer_Parameter(&(Parameter_List[3]), 0, 0xFF)) &&
         (Verify_Integer_Parameter(&(Parameter_List[4]), 0, 0xFF)))
      {
         memset(&AttrRecord, 0, sizeof(qapi_ZB_CL_Write_Attr_Record_t));

         DeviceId              = Parameter_List[0].Integer_Value;
         ClusterInfo           = ZCL_FindClusterByIndex((uint16_t)(Parameter_List[1].Integer_Value), ZCL_DEMO_IGNORE_CLUSTER_ID);
         AttrRecord.AttrId     = (uint16_t)(Parameter_List[2].Integer_Value);
         AttrRecord.DataType   = (qapi_ZB_CL_Data_Type_t)(Parameter_List[3].Integer_Value);
         AttrRecord.AttrLength = (uint16_t)(Parameter_List[4].Integer_Value);

         if(ClusterInfo != NULL)
         {
            switch(AttrRecord.AttrLength)
            {
               case sizeof(uint8_t):
               case sizeof(uint16_t):
               case sizeof(uint32_t):
                  /* Handle the basic integer types. */
                  MaxValue = 0xFFFFFFFF >> ((sizeof(uint32_t) - AttrRecord.AttrLength) * 8);
                  if(Verify_Unsigned_Integer_Parameter(&(Parameter_List[5]), 0, MaxValue))
                  {
                     AttrRecord.AttrValue = (uint8_t *)&(Parameter_List[5].Integer_Value);

                     Ret_Val = QCLI_STATUS_SUCCESS_E;
                  }
                  else
                  {
                     Ret_Val = QCLI_STATUS_ERROR_E;
                  }
                  break;

               case sizeof(uint64_t):
                     /* Attempt to convert the string to a 64-bit integer. */
                     if(Hex_String_To_ULL(Parameter_List[5].String_Value, &AttrValueULL))
                     {
                        AttrRecord.AttrValue = (uint8_t *)&AttrValueULL;

                        Ret_Val = QCLI_STATUS_SUCCESS_E;
                     }
                     else
                     {
                        Ret_Val = QCLI_STATUS_ERROR_E;
                     }
                  break;

               default:
                  Ret_Val = QCLI_STATUS_ERROR_E;
                  break;
            }

            if(Ret_Val == QCLI_STATUS_SUCCESS_E)
            {
               memset(&SendInfo, 0, sizeof(qapi_ZB_CL_General_Send_Info_t));

               if(Format_Send_Info_By_Device(DeviceId, &SendInfo))
               {
                  Result = qc_drv_ZB_CL_Write_Attributes(qc_api_get_qc_drv_context(), ClusterInfo->Handle, &SendInfo, 1, &AttrRecord);
                  if(Result == QAPI_OK)
                  {
                     Display_Function_Success("qc_drv_ZB_CL_Write_Attributes");
                  }
                  else
                  {
                     Ret_Val = QCLI_STATUS_ERROR_E;
                     Display_Function_Error("qc_drv_ZB_CL_Write_Attributes", Result);
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
            LOG_ERR("Invalid ClusterIndex.\n");
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
   @brief Executes the "DiscoverAttributes" command to discover the attributes
          on a device.

   Parameter_List[0] ID of the device whose attributes will be discovered.
   Parameter_List[1] Index of the cluster on the device whose attributes will be
                     discovered.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_api_cmd_ZB_CL_DiscoverAttributes(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t           Ret_Val;
   qapi_Status_t                   Result;
   qapi_ZB_CL_General_Send_Info_t  SendInfo;
   ZCL_Demo_Cluster_Info_t        *ClusterInfo;
   uint8_t                         AttrCount;
   uint16_t                        StartAttrID;
   uint32_t                        DeviceId;

   /* Ensure both the stack is initialized and the switch endpoint. */
   if(GetZigBeeHandle() != NULL)
   {
      Ret_Val = QCLI_STATUS_SUCCESS_E;

      if((Parameter_Count >= 2) &&
         (Parameter_List[0].Integer_Is_Valid) &&
         (Verify_Integer_Parameter(&(Parameter_List[1]), 0, ZCL_Demo_Context.Cluster_Count - 1)))
      {
         if(ZCL_Demo_Context.DiscoverAttr_NextId == 0)
         {
            DeviceId    = Parameter_List[0].Integer_Value;
            ClusterInfo = ZCL_FindClusterByIndex((uint16_t)(Parameter_List[1].Integer_Value), ZCL_DEMO_IGNORE_CLUSTER_ID);

            if(ClusterInfo != NULL)
            {
               /* Set SendInfo to a known state. */
               memset(&SendInfo, 0, sizeof(qapi_ZB_CL_General_Send_Info_t));

               if(Format_Send_Info_By_Device(DeviceId, &SendInfo))
               {
                  StartAttrID = 0;
                  AttrCount   = MAXIMUM_DISCOVER_LENGTH;

                  Result = qc_drv_ZB_CL_Discover_Attributes(qc_api_get_qc_drv_context(), ClusterInfo->Handle, &SendInfo, StartAttrID, AttrCount);
                  if(Result == QAPI_OK)
                  {
                     ZCL_Demo_Context.DiscoverAttr_NextId = StartAttrID + MAXIMUM_DISCOVER_LENGTH;

                     Display_Function_Success("qc_drv_ZB_CL_Discover_Attributes");
                  }
                  else
                  {
                     Ret_Val = QCLI_STATUS_ERROR_E;
                     Display_Function_Error("qc_drv_ZB_CL_Discover_Attributes", Result);
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
               LOG_ERR("Demo only supports one attribute discovery at a time.\n");
               Ret_Val = QCLI_STATUS_ERROR_E;
            }
         }
         else
         {
            LOG_ERR("Invalid ClusterIndex.\n");
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
   @brief Executes the "ConfigReport" command to config the reporting mechanism
          of an attribute.

   Parameter_List[0] ID of the device on which the attribute will be configured.
   Parameter_List[1] Index of the cluster on the device which contains the
                     attribute to be configured.
   Parameter_List[2] ID of the attribute to be configured.
   Parameter_List[3] Type of the attribute to be configured.
   Parameter_List[4] Min reporting interval.
   Parameter_List[5] Max reporting interval.
   Parameter_List[6] Reportable change value.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_api_cmd_ZB_CL_ConfigReport(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t                      Ret_Val;
   qapi_Status_t                              Result;
   qapi_ZB_CL_General_Send_Info_t             SendInfo;
   ZCL_Demo_Cluster_Info_t                   *ClusterInfo;
   uint8_t                                    AttrCount;
   uint64_t                                   ReportableValueULL;
   uint32_t                                   DeviceId;
   qapi_ZB_CL_Attr_Reporting_Config_Record_t  ReportConfig;

   if(GetZigBeeHandle() != NULL)
   {
      if((Parameter_Count >= 7) &&
         (Parameter_List[0].Integer_Is_Valid) &&
         (Verify_Integer_Parameter(&(Parameter_List[1]), 0, ZCL_Demo_Context.Cluster_Count - 1)) &&
         (Verify_Integer_Parameter(&(Parameter_List[2]), 0, 0xFFFF)) &&
         (Verify_Integer_Parameter(&(Parameter_List[3]), 0x00, 0xFF)) &&
         (Verify_Integer_Parameter(&(Parameter_List[4]), 0x0000, 0xFFFF)) &&
         (Verify_Integer_Parameter(&(Parameter_List[5]), 0x0000, 0xFFFF)) &&
         (Hex_String_To_ULL(Parameter_List[6].String_Value, &ReportableValueULL)))
      {
         DeviceId    = Parameter_List[0].Integer_Value;
         ClusterInfo = ZCL_FindClusterByIndex((uint16_t)(Parameter_List[1].Integer_Value), ZCL_DEMO_IGNORE_CLUSTER_ID);

         if(ClusterInfo != NULL)
         {
            /* Set SendInfo to a known state. */
            memset(&SendInfo, 0, sizeof(qapi_ZB_CL_General_Send_Info_t));

            if(Format_Send_Info_By_Device(DeviceId, &SendInfo))
            {
               memset(&ReportConfig, 0, sizeof(qapi_ZB_CL_Attr_Reporting_Config_Record_t));

               AttrCount = 1;

               ReportConfig.Direction            = QAPI_ZB_CL_ATTR_REPORT_DIRECTION_TO_REPORTER_E;
               ReportConfig.AttrId               = (uint16_t)(Parameter_List[2].Integer_Value);
               ReportConfig.DataType             = (qapi_ZB_CL_Data_Type_t)(Parameter_List[3].Integer_Value);
               ReportConfig.MinReportingInterval = (uint16_t)(Parameter_List[4].Integer_Value);
               ReportConfig.MaxReportingInterval = (uint16_t)(Parameter_List[5].Integer_Value);
               ReportConfig.ReportableChange     = ReportableValueULL;

               Result = qc_drv_ZB_CL_Configure_Reporting(qc_api_get_qc_drv_context(), ClusterInfo->Handle, &SendInfo, AttrCount, &ReportConfig);
               if(Result == QAPI_OK)
               {
                  Display_Function_Success("qc_drv_ZB_CL_Configure_Reporting");
                  Ret_Val = QCLI_STATUS_SUCCESS_E;
               }
               else
               {
                  Display_Function_Error("qc_drv_ZB_CL_Configure_Reporting", Result);
                  Ret_Val = QCLI_STATUS_ERROR_E;
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
            LOG_ERR("Invalid ClusterIndex.\n");
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
   @brief Executes the "ReadReportConfig" command to read the reporting
          configuration of an attribute.

   Parameter_List[0] ID of the device on which the reporting configuration
                     of attribute will be read.
   Parameter_List[1] Index of the cluster on the device which contains the
                     attribute to be read.
   Parameter_List[2] ID of the attribute to be read.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_api_cmd_ZB_CL_ReadReportConfig(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t           Ret_Val;
   qapi_Status_t                   Result;
   qapi_ZB_CL_General_Send_Info_t  SendInfo;
   ZCL_Demo_Cluster_Info_t        *ClusterInfo;
   uint8_t                         AttrCount;
   uint32_t                        DeviceId;
   qapi_ZB_CL_Attr_Record_t        ReportConfig;

   /* Ensure both the stack is initialized and the switch endpoint. */
   if(GetZigBeeHandle() != NULL)
   {
      if((Parameter_Count >= 3) &&
         (Parameter_List[0].Integer_Is_Valid) &&
         (Verify_Integer_Parameter(&(Parameter_List[1]), 0, ZCL_Demo_Context.Cluster_Count - 1)) &&
         (Verify_Integer_Parameter(&(Parameter_List[2]), 0, 0xFFFF)))
      {
         DeviceId    = Parameter_List[0].Integer_Value;
         ClusterInfo = ZCL_FindClusterByIndex((uint16_t)(Parameter_List[1].Integer_Value), ZCL_DEMO_IGNORE_CLUSTER_ID);

         if(ClusterInfo != NULL)
         {
            /* Set SendInfo to a known state. */
            memset(&SendInfo, 0, sizeof(qapi_ZB_CL_General_Send_Info_t));

            if(Format_Send_Info_By_Device(DeviceId, &SendInfo))
            {
               memset(&ReportConfig, 0, sizeof(qapi_ZB_CL_Attr_Record_t));

               AttrCount = 1;

               ReportConfig.AttrId    = (uint16_t)(Parameter_List[2].Integer_Value);
               ReportConfig.Direction = QAPI_ZB_CL_ATTR_REPORT_DIRECTION_TO_REPORTER_E;
               Result = qc_drv_ZB_CL_Read_Reporting(qc_api_get_qc_drv_context(), ClusterInfo->Handle, &SendInfo, AttrCount, &ReportConfig);
               if(Result == QAPI_OK)
               {
                  Display_Function_Success("qc_drv_ZB_CL_Read_Reporting_Config");
                  Ret_Val = QCLI_STATUS_SUCCESS_E;
               }
               else
               {
                  Display_Function_Error("qc_drv_ZB_CL_Read_Reporting_Config", Result);
                  Ret_Val = QCLI_STATUS_ERROR_E;
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
            LOG_ERR("Invalid ClusterIndex.\n");
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
   @brief Executes the "Reset" command to send a Reset To Factory command to a
          basic server.

   Parameter_List[0] is the index of the device to send to. Use index zero to
                     use the binding table (if setup).
   Parameter_List[1] Endpoint of the Basic client cluster to use to send the
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
QCLI_Command_Status_t qc_api_cmd_ZCL_Basic_Reset(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
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
         ClusterInfo = ZCL_FindClusterByEndpoint((uint8_t)(Parameter_List[1].Integer_Value), QAPI_ZB_CL_CLUSTER_ID_BASIC, ZCL_DEMO_CLUSTERTYPE_CLIENT);

         if(ClusterInfo != NULL)
         {
            memset(&SendInfo, 0, sizeof(SendInfo));

            /* Format the destination addr. mode, address, and endpoint. */
            if(Format_Send_Info_By_Device(DeviceId, &SendInfo))
            {
               Result = qc_drv_ZB_CL_Basic_Send_Reset_To_Factory(qc_api_get_qc_drv_context(), ClusterInfo->Handle, &SendInfo);
               if(Result == QAPI_OK)
               {
                  Display_Function_Success("qc_drv_ZB_CL_Basic_Send_Reset_To_Factory");
               }
               else
               {
                  Ret_Val = QCLI_STATUS_ERROR_E;
                  Display_Function_Error("qc_drv_ZB_CL_Basic_Send_Reset_To_Factory", Result);
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
   @brief Executes the "Read" command to read an attribute from the local basic
          server.

   Parameter_List[0] is the index of the device to send to. Use index zero to
                     use the binding table (if setup).

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_api_cmd_ZCL_Basic_Read(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   qapi_Status_t              Result;
   QCLI_Command_Status_t      Ret_Val;
   qapi_ZB_Handle_t           ZB_Handle;
   uint16_t                   AttrId;
   uint16_t                   Length;
   uint8_t                    Data[36];
   ZCL_Basic_Demo_Attr_Type_t AttrType;

   ZB_Handle = GetZigBeeHandle();
   if(ZB_Handle != NULL)
   {
      /* Validate the device ID. */
      if((Parameter_Count >= 1) &&
         (Verify_Integer_Parameter(&(Parameter_List[0]), 0, 0xFFFF)))
      {
         AttrId   = Parameter_List[0].Integer_Value;
         AttrType = ZCL_Basic_Demo_GetAttrType(AttrId);

         Result = qc_drv_ZB_CL_Basic_Server_Read_Attribute(qc_api_get_qc_drv_context(), ZB_Handle, AttrId, &Length, Data);
         if(Result == QAPI_OK)
         {
            Display_Function_Success("qc_drv_ZB_CL_Basic_Server_Read_Attribute");

            switch(AttrType)
            {
               case batUint8:
                  LOG_INFO("Attribute Value: %d\n", Data[0]);
                  break;
               case batString:
                  LOG_INFO("Attribute Value: %s\n", Data);
                  break;
               case batUnknown:
               default:
                  LOG_INFO("Attribute Value:\n", Data);
                  Dump_Data("  ", Length, Data);
                  break;
            }

            Ret_Val = QCLI_STATUS_SUCCESS_E;
         }
         else
         {
            Ret_Val = QCLI_STATUS_ERROR_E;
            Display_Function_Error("qc_drv_ZB_CL_Basic_Server_Read_Attribute", Result);
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
   @brief Executes the "Write" command to write an attribute to the local Basic
          server.

   Parameter_List[0] is the ID of the attribute to write.
   Parameter_List[1] is the value for the attribute.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_api_cmd_ZCL_Basic_Write(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t       Ret_Val;
   qapi_Status_t               Result;
   qapi_ZB_Handle_t            ZB_Handle;
   uint16_t                    AttrId;
   uint16_t                    Length;
   uint8_t                     IntData;
   uint8_t                    *Data;
   ZCL_Basic_Demo_Attr_Type_t  AttrType;

   ZB_Handle = GetZigBeeHandle();
   if(ZB_Handle != NULL)
   {
      /* Validate the device ID. */
      if((Parameter_Count >= 2) &&
         (Verify_Integer_Parameter(&(Parameter_List[0]), 0, 0xFFFF)))
      {
         AttrId   = Parameter_List[0].Integer_Value;
         AttrType = ZCL_Basic_Demo_GetAttrType(AttrId);

         Data    = NULL;
         Length  = 0;
         Ret_Val = QCLI_STATUS_SUCCESS_E;

         switch(AttrType)
         {
            case batUint8:
               if(Verify_Integer_Parameter(&(Parameter_List[1]), 0, 0xFF))
               {
                  IntData = (uint8_t)(Parameter_List[1].Integer_Value);
                  Data    = &IntData;
                  Length  = sizeof(uint8_t);
                  Ret_Val = QCLI_STATUS_SUCCESS_E;
               }
               else
               {
                  Ret_Val = QCLI_STATUS_ERROR_E;
               }
               break;
            case batString:
               Data    = (uint8_t *)(Parameter_List[1].String_Value);
               Length  = strlen(Parameter_List[1].String_Value);
               Ret_Val = QCLI_STATUS_SUCCESS_E;
               break;
            case batUnknown:
            default:
               Display_Function_Success("Unknown attribute type.\n");
               Ret_Val = QCLI_STATUS_ERROR_E;
               Data    = NULL;
               Length  = 0;
               break;
         }

         /* Format the destination addr. mode, address, and endpoint. */
         if(Ret_Val == QCLI_STATUS_SUCCESS_E)
         {
            Result = qc_drv_ZB_CL_Basic_Server_Write_Attribute(qc_api_get_qc_drv_context(), ZB_Handle, AttrId, Length, Data);
            if(Result == QAPI_OK)
            {
               Display_Function_Success("qc_drv_ZB_CL_Basic_Server_Write_Attribute");
            }
            else
            {
               Ret_Val = QCLI_STATUS_ERROR_E;
               Display_Function_Error("qc_drv_ZB_CL_Basic_Server_Write_Attribute", Result);
            }
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
   @brief Executes the "Identify" command to request an endpoint identify
          itself.

   Parameter_List[0] ID of the device to send the command to.
   Parameter_List[1] Endpoint of the Identify client cluster to use to send the
                     command.
   Parameter_List[2] Time for the device to Identify.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_api_cmd_ZCL_Identify_Identify(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t           Ret_Val;
   qapi_Status_t                   Result;
   qapi_ZB_CL_General_Send_Info_t  SendInfo;
   ZCL_Demo_Cluster_Info_t        *ClusterInfo;
   uint32_t                        DeviceId;
   uint16_t                        Time;

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
         ClusterInfo = ZCL_FindClusterByEndpoint((uint8_t)(Parameter_List[1].Integer_Value), QAPI_ZB_CL_CLUSTER_ID_IDENTIFY, ZCL_DEMO_CLUSTERTYPE_CLIENT);
         Time        = Parameter_List[2].Integer_Value;

         if(ClusterInfo != NULL)
         {
            /* Format the destination info. */
            memset(&SendInfo, 0, sizeof(qapi_ZB_CL_General_Send_Info_t));
            if(Format_Send_Info_By_Device(DeviceId, &SendInfo))
            {
               Result = qc_drv_ZB_CL_Identify_Send_Identify(qc_api_get_qc_drv_context(), ClusterInfo->Handle, &SendInfo, Time);
               if(Result == QAPI_OK)
               {
                  Display_Function_Success("qc_drv_ZB_CL_Identify_Send_Identify");
               }
               else
               {
                  Ret_Val = QCLI_STATUS_ERROR_E;
                  Display_Function_Error("qc_drv_ZB_CL_Identify_Send_Identify", Result);
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
   @brief Executes the "IdentifyQuery" command to query its identify status.

   Parameter_List[0] ID of the device to send the command to.
   Parameter_List[1] Endpoint of the Identify client cluster to use to send the
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
QCLI_Command_Status_t qc_api_cmd_ZCL_Identify_IdentifyQuery(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t           Ret_Val;
   qapi_Status_t                   Result;
   qapi_ZB_CL_General_Send_Info_t  SendInfo;
   ZCL_Demo_Cluster_Info_t        *ClusterInfo;
   uint32_t                        DeviceId;

   /* Ensure both the stack is initialized. */
   if(GetZigBeeHandle() != NULL)
   {
      Ret_Val = QCLI_STATUS_SUCCESS_E;

      if((Parameter_Count >= 2) &&
         (Parameter_List[0].Integer_Is_Valid) &&
         (Verify_Integer_Parameter(&(Parameter_List[1]), QAPI_ZB_APS_MIN_ENDPOINT, QAPI_ZB_APS_MAX_ENDPOINT)))
      {
         DeviceId    = Parameter_List[0].Integer_Value;
         ClusterInfo = ZCL_FindClusterByEndpoint((uint8_t)(Parameter_List[1].Integer_Value), QAPI_ZB_CL_CLUSTER_ID_IDENTIFY, ZCL_DEMO_CLUSTERTYPE_CLIENT);

         if(ClusterInfo != NULL)
         {
            /* Format the destination info. */
            memset(&SendInfo, 0, sizeof(qapi_ZB_CL_General_Send_Info_t));
            if(Format_Send_Info_By_Device(DeviceId, &SendInfo))
            {
               Result = qc_drv_ZB_CL_Identify_Send_Identify_Query(qc_api_get_qc_drv_context(), ClusterInfo->Handle, &SendInfo);
               if(Result == QAPI_OK)
               {
                  Display_Function_Success("qc_drv_ZB_CL_Identify_Send_Identify_Query");
               }
               else
               {
                  Ret_Val = QCLI_STATUS_ERROR_E;
                  Display_Function_Error("qc_drv_ZB_CL_Identify_Send_Identify_Query", Result);
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
   @brief Executes the "AddGroup" command to add a group.

   Parameter_List[0] ID of the device to send the command to.
   Parameter_List[1] Endpoint of the Groups client cluster to use to send the
                     command.
   Parameter_List[2] ID of the group to add.
   Parameter_List[3] Name of the group to add.
   Parameter_List[4] Flag indicating if groups should be added only if the
                     device is identifying.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_api_cmd_ZCL_Groups_AddGroup(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t           Ret_Val;
   qapi_Status_t                   Result;
   qapi_ZB_CL_General_Send_Info_t  SendInfo;
   ZCL_Demo_Cluster_Info_t        *ClusterInfo;
   uint32_t                        DeviceId;
   uint16_t                        GroupId;
   char                           *GroupName;
   qbool_t                         Identifying;

   /* Ensure both the stack is initialized. */
   if(GetZigBeeHandle() != NULL)
   {
      Ret_Val = QCLI_STATUS_SUCCESS_E;

      if((Parameter_Count >= 5) &&
         (Parameter_List[0].Integer_Is_Valid) &&
         (Verify_Integer_Parameter(&(Parameter_List[1]), QAPI_ZB_APS_MIN_ENDPOINT, QAPI_ZB_APS_MAX_ENDPOINT)) &&
         (Verify_Integer_Parameter(&(Parameter_List[2]), 0, 0xFFFF)) &&
         (Verify_Integer_Parameter(&(Parameter_List[4]), 0, 1)))
      {
         DeviceId    = Parameter_List[0].Integer_Value;
         ClusterInfo = ZCL_FindClusterByEndpoint((uint8_t)(Parameter_List[1].Integer_Value), QAPI_ZB_CL_CLUSTER_ID_GROUPS, ZCL_DEMO_CLUSTERTYPE_CLIENT);
         GroupId     = Parameter_List[2].Integer_Value;
         GroupName   = Parameter_List[3].String_Value;
         Identifying = (qbool_t)(Parameter_List[4].Integer_Value != 0);

         if(ClusterInfo != NULL)
         {
            /* Format the destination info. */
            memset(&SendInfo, 0, sizeof(qapi_ZB_CL_General_Send_Info_t));
            if(Format_Send_Info_By_Device(DeviceId, &SendInfo))
            {
               Result = qc_drv_ZB_CL_Groups_Send_Add_Group(qc_api_get_qc_drv_context(), ClusterInfo->Handle, &SendInfo, GroupId, GroupName, Identifying);
               if(Result == QAPI_OK)
               {
                  Display_Function_Success("qc_drv_ZB_CL_Groups_Send_Add_Group");
               }
               else
               {
                  Ret_Val = QCLI_STATUS_ERROR_E;
                  Display_Function_Error("qc_drv_ZB_CL_Groups_Send_Add_Group", Result);
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
   @brief Executes the "ViewGroup" command to view a group.

   Parameter_List[0] ID of the device to send the command to.
   Parameter_List[1] Endpoint of the Groups client cluster to use to send the
                     command.
   Parameter_List[2] ID of the group to view.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_api_cmd_ZCL_Groups_ViewGroup(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
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
         ClusterInfo = ZCL_FindClusterByEndpoint((uint8_t)(Parameter_List[1].Integer_Value), QAPI_ZB_CL_CLUSTER_ID_GROUPS, ZCL_DEMO_CLUSTERTYPE_CLIENT);
         GroupId     = Parameter_List[2].Integer_Value;

         if(ClusterInfo != NULL)
         {
            /* Format the destination info. */
            memset(&SendInfo, 0, sizeof(qapi_ZB_CL_General_Send_Info_t));
            if(Format_Send_Info_By_Device(DeviceId, &SendInfo))
            {
               Result = qc_drv_ZB_CL_Groups_Send_View_Group(qc_api_get_qc_drv_context(), ClusterInfo->Handle, &SendInfo, GroupId);
               if(Result == QAPI_OK)
               {
                  Display_Function_Success("qc_drv_ZB_CL_Groups_Send_View_Group");
               }
               else
               {
                  Ret_Val = QCLI_STATUS_ERROR_E;
                  Display_Function_Error("qc_drv_ZB_CL_Groups_Send_View_Group", Result);
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
   @brief Executes the "GetGroupMembership" command to get a groups membership.

   Parameter_List[0] ID of the device to send the command to.
   Parameter_List[1] Endpoint of the Groups client cluster to use to send the
                     command.
   Parameter_List[2] ID of the group to request membership of Leave blank to
                     query all groups.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_api_cmd_ZCL_Groups_GetGroupMembership(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t           Ret_Val;
   qapi_Status_t                   Result;
   qapi_ZB_CL_General_Send_Info_t  SendInfo;
   ZCL_Demo_Cluster_Info_t        *ClusterInfo;
   uint32_t                        DeviceId;
   uint8_t                         GroupCount;
   uint16_t                        GroupId;

   /* Ensure both the stack is initialized. */
   if(GetZigBeeHandle() != NULL)
   {
      Ret_Val = QCLI_STATUS_SUCCESS_E;

      if((Parameter_Count >= 2) &&
         (Parameter_List[0].Integer_Is_Valid) &&
         (Verify_Integer_Parameter(&(Parameter_List[1]), QAPI_ZB_APS_MIN_ENDPOINT, QAPI_ZB_APS_MAX_ENDPOINT)))
      {
         DeviceId    = Parameter_List[0].Integer_Value;
         ClusterInfo = ZCL_FindClusterByEndpoint((uint8_t)(Parameter_List[1].Integer_Value), QAPI_ZB_CL_CLUSTER_ID_GROUPS, ZCL_DEMO_CLUSTERTYPE_CLIENT);
         GroupId     = 0;
         GroupCount  = 0;

         if((Parameter_Count >= 3) &&
            (Verify_Integer_Parameter(&(Parameter_List[2]), 0, 0xFFFF)))
         {
            GroupCount = 1;
            GroupId    = Parameter_List[2].Integer_Value;
         }

         if(ClusterInfo != NULL)
         {
            /* Format the destination info. */
            memset(&SendInfo, 0, sizeof(qapi_ZB_CL_General_Send_Info_t));
            if(Format_Send_Info_By_Device(DeviceId, &SendInfo))
            {
               Result = qc_drv_ZB_CL_Groups_Send_Get_Group_Membership(qc_api_get_qc_drv_context(), ClusterInfo->Handle, &SendInfo, GroupCount, (GroupCount != 0) ? &GroupId : NULL);
               if(Result == QAPI_OK)
               {
                  Display_Function_Success("qc_drv_ZB_CL_Groups_Send_Get_Group_Membership");
               }
               else
               {
                  Ret_Val = QCLI_STATUS_ERROR_E;
                  Display_Function_Error("qc_drv_ZB_CL_Groups_Send_Get_Group_Membership", Result);
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
   @brief Executes the "RemoveGroup" command to remove a group.

   Parameter_List[0] ID of the device to send the command to.
   Parameter_List[1] Endpoint of the Groups client cluster to use to send the
                     command.
   Parameter_List[2] ID of the group to remove.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_api_cmd_ZCL_Groups_RemoveGroup(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
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
         ClusterInfo = ZCL_FindClusterByEndpoint((uint8_t)(Parameter_List[1].Integer_Value), QAPI_ZB_CL_CLUSTER_ID_GROUPS, ZCL_DEMO_CLUSTERTYPE_CLIENT);
         GroupId     = Parameter_List[2].Integer_Value;

         if(ClusterInfo != NULL)
         {
            /* Format the destination info. */
            memset(&SendInfo, 0, sizeof(qapi_ZB_CL_General_Send_Info_t));
            if(Format_Send_Info_By_Device(DeviceId, &SendInfo))
            {
               Result = qc_drv_ZB_CL_Groups_Send_Remove_Group(qc_api_get_qc_drv_context(), ClusterInfo->Handle, &SendInfo, GroupId);
               if(Result == QAPI_OK)
               {
                  Display_Function_Success("qc_drv_ZB_CL_Groups_Send_Remove_Group");
               }
               else
               {
                  Ret_Val = QCLI_STATUS_ERROR_E;
                  Display_Function_Error("qc_drv_ZB_CL_Groups_Send_Remove_Group", Result);
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
   @brief Executes the "RemoveAllGroups" command to remove all groups.

   Parameter_List[0] ID of the device to send the command to.
   Parameter_List[1] Endpoint of the Groups client cluster to use to send the
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
QCLI_Command_Status_t qc_api_cmd_ZCL_Groups_RemoveAllGroups(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t           Ret_Val;
   qapi_Status_t                   Result;
   qapi_ZB_CL_General_Send_Info_t  SendInfo;
   ZCL_Demo_Cluster_Info_t        *ClusterInfo;
   uint32_t                        DeviceId;

   /* Ensure both the stack is initialized. */
   if(GetZigBeeHandle() != NULL)
   {
      Ret_Val = QCLI_STATUS_SUCCESS_E;

      if((Parameter_Count >= 2) &&
         (Parameter_List[0].Integer_Is_Valid) &&
         (Verify_Integer_Parameter(&(Parameter_List[1]), QAPI_ZB_APS_MIN_ENDPOINT, QAPI_ZB_APS_MAX_ENDPOINT)))
      {
         DeviceId    = Parameter_List[0].Integer_Value;
         ClusterInfo = ZCL_FindClusterByEndpoint((uint8_t)(Parameter_List[1].Integer_Value), QAPI_ZB_CL_CLUSTER_ID_GROUPS, ZCL_DEMO_CLUSTERTYPE_CLIENT);

         if(ClusterInfo != NULL)
         {
            /* Format the destination info. */
            memset(&SendInfo, 0, sizeof(qapi_ZB_CL_General_Send_Info_t));
            if(Format_Send_Info_By_Device(DeviceId, &SendInfo))
            {
               Result = qc_drv_ZB_CL_Groups_Send_Remove_All_Groups(qc_api_get_qc_drv_context(), ClusterInfo->Handle, &SendInfo);
               if(Result == QAPI_OK)
               {
                  Display_Function_Success("qc_drv_ZB_CL_Groups_Send_Remove_All_Groups");
               }
               else
               {
                  Ret_Val = QCLI_STATUS_ERROR_E;
                  Display_Function_Error("qc_drv_ZB_CL_Groups_Send_Remove_All_Groups", Result);
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
QCLI_Command_Status_t qc_api_cmd_ZCL_Scenes_AddScene(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
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
                     Display_Function_Success("qc_drv_ZB_CL_Scenes_Send_Add_Scene");
                  }
                  else
                  {
                     Ret_Val = QCLI_STATUS_ERROR_E;
                     Display_Function_Error("qc_drv_ZB_CL_Scenes_Send_Add_Scene", Result);
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
QCLI_Command_Status_t qc_api_cmd_ZCL_Scenes_ViewScene(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
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
                  Display_Function_Success("qc_drv_ZB_CL_Scenes_Send_View_Scene");
               }
               else
               {
                  Ret_Val = QCLI_STATUS_ERROR_E;
                  Display_Function_Error("qc_drv_ZB_CL_Scenes_Send_View_Scene", Result);
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
QCLI_Command_Status_t qc_api_cmd_ZCL_Scenes_RemoveScene(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
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
                  Display_Function_Success("qc_drv_ZB_CL_Scenes_Send_Remove_Scene");
               }
               else
               {
                  Ret_Val = QCLI_STATUS_ERROR_E;
                  Display_Function_Error("qc_drv_ZB_CL_Scenes_Send_Remove_Scene", Result);
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
QCLI_Command_Status_t qc_api_cmd_ZCL_Scenes_StoreScenes(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
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
                  Display_Function_Success("qc_drv_ZB_CL_Scenes_Send_Store_Scene");
               }
               else
               {
                  Ret_Val = QCLI_STATUS_ERROR_E;
                  Display_Function_Error("qc_drv_ZB_CL_Scenes_Send_Store_Scene", Result);
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
QCLI_Command_Status_t qc_api_cmd_ZCL_Scenes_RemoveAllScenes(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
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
                  Display_Function_Success("qc_drv_ZB_CL_Scenes_Send_Remove_All_Scenes");
               }
               else
               {
                  Ret_Val = QCLI_STATUS_ERROR_E;
                  Display_Function_Error("qc_drv_ZB_CL_Scenes_Send_Remove_All_Scenes", Result);
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
QCLI_Command_Status_t qc_api_cmd_ZCL_Scenes_RecallScenes(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
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
                  Display_Function_Success("qc_drv_ZB_CL_Scenes_Send_Recall_Scene");
               }
               else
               {
                  Ret_Val = QCLI_STATUS_ERROR_E;
                  Display_Function_Error("qc_drv_ZB_CL_Scenes_Send_Recall_Scene", Result);
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
QCLI_Command_Status_t qc_api_cmd_ZCL_Scenes_GetSceneMembership(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
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
                  Display_Function_Success("qc_drv_ZB_CL_Scenes_Send_Get_Scene_Membership");
               }
               else
               {
                  Ret_Val = QCLI_STATUS_ERROR_E;
                  Display_Function_Error("qc_drv_ZB_CL_Scenes_Send_Get_Scene_Membership", Result);
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
QCLI_Command_Status_t qc_api_cmd_ZCL_Scenes_CopyScene(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
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
                  Display_Function_Success("qc_drv_ZB_CL_Scenes_Send_Copy_Scene");
               }
               else
               {
                  Ret_Val = QCLI_STATUS_ERROR_E;
                  Display_Function_Error("qc_drv_ZB_CL_Scenes_Send_Copy_Scene", Result);
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
QCLI_Command_Status_t qc_api_cmd_ZCL_OnOff_On(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
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
                  Display_Function_Success("qc_drv_ZB_CL_OnOff_Send_On");
               }
               else
               {
                  Ret_Val = QCLI_STATUS_ERROR_E;
                  Display_Function_Error("qc_drv_ZB_CL_OnOff_Send_On", Result);
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
QCLI_Command_Status_t qc_api_cmd_ZCL_OnOff_Off(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
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
                  Display_Function_Success("qc_drv_ZB_CL_OnOff_Send_Off");
               }
               else
               {
                  Ret_Val = QCLI_STATUS_ERROR_E;
                  Display_Function_Error("qc_drv_ZB_CL_OnOff_Send_Off", Result);
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
QCLI_Command_Status_t qc_api_cmd_ZCL_OnOff_Toggle(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
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
                  Display_Function_Success("qc_drv_ZB_CL_OnOff_Send_Toggle");
               }
               else
               {
                  Ret_Val = QCLI_STATUS_ERROR_E;
                  Display_Function_Error("qc_drv_ZB_CL_OnOff_Send_Toggle", Result);
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
      LOG_WARN("zigBee stack is not initialized.\n");
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
QCLI_Command_Status_t qc_api_cmd_ZCL_OnOff_SetSceneData(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
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
QCLI_Command_Status_t qc_api_cmd_ZCL_LevelControl_MoveToLevel(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
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
                  Display_Function_Success("qc_drv_ZB_CL_LevelControl_Send_Move_To_Level");
               }
               else
               {
                  Ret_Val = QCLI_STATUS_ERROR_E;
                  Display_Function_Error("qc_drv_ZB_CL_LevelControl_Send_Move_To_Level", Result);
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
QCLI_Command_Status_t qc_api_cmd_ZCL_LevelControl_Move(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
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
                  Display_Function_Success("qc_drv_ZB_CL_LevelControl_Send_Move");
               }
               else
               {
                  Ret_Val = QCLI_STATUS_ERROR_E;
                  Display_Function_Error("qc_drv_ZB_CL_LevelControl_Send_Move", Result);
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
QCLI_Command_Status_t qc_api_cmd_ZCL_LevelControl_Step(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
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
                  Display_Function_Success("qc_drv_ZB_CL_LevelControl_Send_Step");
               }
               else
               {
                  Ret_Val = QCLI_STATUS_ERROR_E;
                  Display_Function_Error("qc_drv_ZB_CL_LevelControl_Send_Step", Result);
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
QCLI_Command_Status_t qc_api_cmd_ZCL_LevelControl_Stop(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
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
                  Display_Function_Success("qc_drv_ZB_CL_LevelControl_Send_Stop");
               }
               else
               {
                  Ret_Val = QCLI_STATUS_ERROR_E;
                  Display_Function_Error("qc_drv_ZB_CL_LevelControl_Send_Stop", Result);
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
QCLI_Command_Status_t qc_api_cmd_ZCL_Alarms_ResetAlarm(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
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
                  Display_Function_Success("qc_drv_ZB_CL_Alarm_Send_Reset_Alarm");
               }
               else
               {
                  Ret_Val = QCLI_STATUS_ERROR_E;
                  Display_Function_Error("qc_drv_ZB_CL_Alarm_Send_Reset_Alarm", Result);
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
QCLI_Command_Status_t qc_api_cmd_ZCL_Alarms_ResetAllAlarms(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
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
                  Display_Function_Success("qc_drv_ZB_CL_Alarm_Send_Reset_All_Alarms");
               }
               else
               {
                  Ret_Val = QCLI_STATUS_ERROR_E;
                  Display_Function_Error("qc_drv_ZB_CL_Alarm_Send_Reset_All_Alarms", Result);
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
QCLI_Command_Status_t qc_api_cmd_ZCL_Alarms_GetAlarm(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
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
                  Display_Function_Success("qc_drv_ZB_CL_Alarm_Send_Get_Alarm");
               }
               else
               {
                  Ret_Val = QCLI_STATUS_ERROR_E;
                  Display_Function_Error("qc_drv_ZB_CL_Alarm_Send_Get_Alarm", Result);
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
QCLI_Command_Status_t qc_api_cmd_ZCL_Alarms_ResetAlarmLog(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
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
                  Display_Function_Success("qc_drv_ZB_CL_Alarm_Send_Reset_Alarm_Log");
               }
               else
               {
                  Ret_Val = QCLI_STATUS_ERROR_E;
                  Display_Function_Error("qc_drv_ZB_CL_Alarm_Send_Reset_Alarm_Log", Result);
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
QCLI_Command_Status_t qc_api_cmd_ZCL_Alarms_Alarm(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
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
               Display_Function_Success("qc_drv_ZB_CL_Alarm_Send_Alarm");
            }
            else
            {
               Ret_Val = QCLI_STATUS_ERROR_E;
               Display_Function_Error("qc_drv_ZB_CL_Alarm_Send_Alarm", Result);
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
   @brief Executes the "Read" command to read an attribute from the local time
          server.

   Parameter_List[0] is the index of the device to send to. Use index zero to
                     use the binding table (if setup).

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_api_cmd_ZCL_Time_Read(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   qapi_Status_t             Result;
   QCLI_Command_Status_t     Ret_Val;
   qapi_ZB_Handle_t          ZB_Handle;
   uint16_t                  AttrId;
   uint16_t                  Length;
   ZCL_Time_Demo_Attr_Type_t AttrType;

   union
   {
      uint8_t  Value8;
      uint32_t Value32;
   } Data;

   ZB_Handle = GetZigBeeHandle();
   if(ZB_Handle != NULL)
   {
      /* Validate the device ID. */
      if((Parameter_Count >= 1) &&
         (Verify_Integer_Parameter(&(Parameter_List[0]), 0, 0xFFFF)))
      {
         AttrId = Parameter_List[0].Integer_Value;

         AttrType = ZCL_Time_Demo_GetAttrType(AttrId);

         Length = sizeof(Data);
         Result = qc_drv_ZB_CL_Time_Server_Read_Attribute(qc_api_get_qc_drv_context(), ZB_Handle, AttrId, &Length, (uint8_t *)&Data);
         if(Result == QAPI_OK)
         {
            Display_Function_Error("qc_drv_ZB_CL_Time_Server_Read_Attribute");

            LOG_INFO("Attribute Value: ");
            switch(AttrType)
            {
               case tatUint8:
                  LOG_INFO("0x%02X\n", Data.Value8);
                  break;

               case tatUint32:
                  LOG_INFO("%u\n", Data.Value32);
                  break;

               case tatInt32:
                  LOG_INFO("%d\n", Data.Value32);
                  break;

               case tatUnknown:
               default:
                  LOG_AT("\n");
                  Dump_Data("  ", Length, (uint8_t *)&Data);
                  break;
            }

            Ret_Val = QCLI_STATUS_SUCCESS_E;
         }
         else
         {
            Ret_Val = QCLI_STATUS_ERROR_E;
            Display_Function_Error("qc_drv_ZB_CL_Time_Server_Read_Attribute", Result);
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
   @brief Executes the "Write" command to write an attribute to the local Time
          server.

   Parameter_List[0] is the ID of the attribute to write.
   Parameter_List[1] is the value for the attribute.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_api_cmd_ZCL_Time_Write(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t      Ret_Val;
   qapi_Status_t              Result;
   qapi_ZB_Handle_t           ZB_Handle;
   uint16_t                   AttrId;
   uint16_t                   Length;
   ZCL_Time_Demo_Attr_Type_t  AttrType;

   union
   {
      uint8_t  Value8;
      uint32_t Value32;
   } Data;

   ZB_Handle = GetZigBeeHandle();
   if(ZB_Handle != NULL)
   {
      /* Validate the device ID. */
      if((Parameter_Count >= 2) &&
         (Verify_Integer_Parameter(&(Parameter_List[0]), 0, 0xFFFF)))
      {
         AttrId   = Parameter_List[0].Integer_Value;
         AttrType = ZCL_Time_Demo_GetAttrType(AttrId);

         Ret_Val      = QCLI_STATUS_SUCCESS_E;
         Data.Value32 = 0;
         Length       = 0;

         switch(AttrType)
         {
            case tatUint8:
               if(Verify_Integer_Parameter(&(Parameter_List[1]), 0, 0xFF))
               {
                  Data.Value8 = (uint8_t)(Parameter_List[1].Integer_Value);
                  Length  = sizeof(uint8_t);
               }
               else
               {
                  LOG_ERR("Invalid value for attribute.\n");
                  Ret_Val = QCLI_STATUS_ERROR_E;
               }
               break;
            case tatUint32:
            case tatInt32:
               if(Parameter_List[1].Integer_Is_Valid)
               {
                  Data.Value32 = (uint32_t)(Parameter_List[1].Integer_Value);
                  Length       = sizeof(uint32_t);
               }
               else
               {
                  LOG_ERR("Invalid value for attribute.\n");
                  Ret_Val = QCLI_STATUS_ERROR_E;
               }
               break;
            case tatUnknown:
            default:
               Display_Function_Error("Unknown attribute type.\n");
               Ret_Val = QCLI_STATUS_ERROR_E;
               break;
         }

         /* Format the destination addr. mode, address, and endpoint. */
         if(Ret_Val == QCLI_STATUS_SUCCESS_E)
         {
            Result = qc_drv_ZB_CL_Time_Server_Write_Attribute(qc_api_get_qc_drv_context(), ZB_Handle, AttrId, Length, (uint8_t *)&Data);
            if(Result == QAPI_OK)
            {
               Display_Function_Error("qc_drv_ZB_CL_Time_Server_Write_Attribute");
            }
            else
            {
               Ret_Val = QCLI_STATUS_ERROR_E;
               Display_Function_Error("qc_drv_ZB_CL_Time_Server_Write_Attribute", Result);
            }
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
   @brief Executes the "Start" command to view a scene.

   Parameter_List[0] Endpoint of the Touchlink cluster to start commissioning
                     on.
   Parameter_List[1] Type of device to initialize touchlink as.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_api_cmd_ZCL_Touchlink_Start(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t               Ret_Val;
   qapi_Status_t                       Result;
   ZCL_Demo_Cluster_Info_t            *ClusterInfo;
   qapi_ZB_CL_Touchlink_Device_Type_t  DeviceType;

   /* Ensure both the stack is initialized. */
   if(GetZigBeeHandle() != NULL)
   {
      if((Parameter_Count >= 2) &&
         (Verify_Integer_Parameter(&(Parameter_List[0]), QAPI_ZB_APS_MIN_ENDPOINT, QAPI_ZB_APS_MAX_ENDPOINT)) &&
         (Verify_Integer_Parameter(&(Parameter_List[1]), 0, 3)))
      {
         ClusterInfo = ZCL_FindClusterByEndpoint((uint8_t)(Parameter_List[0].Integer_Value), QAPI_ZB_CL_CLUSTER_ID_TOUCHLINK_COMMISSIONING, ZCL_DEMO_CLUSTERTYPE_UNKNOWN);
         DeviceType  = (qapi_ZB_CL_Touchlink_Device_Type_t)(Parameter_List[1].Integer_Value);

         if(ClusterInfo != NULL)
         {
            Ret_Val = QCLI_STATUS_SUCCESS_E;

//xxx add persist support once added to the demo
            Result = qc_drv_ZB_CL_Touchlink_Start(qc_api_get_qc_drv_context(), ClusterInfo->Handle, DeviceType, NULL, 0);
            if(Result == QAPI_OK)
            {
               Display_Function_Success("qc_drv_ZB_CL_Touchlink_Start");
            }
            else
            {
               Ret_Val = QCLI_STATUS_ERROR_E;
               Display_Function_Error("qc_drv_ZB_CL_Touchlink_Start", Result);
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
   @brief Executes the "Scan" command to perform scan operation.

   Parameter_List[0] Endpoint of the Touchlink client cluster to use.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_api_cmd_ZCL_Touchlink_Scan(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t               Ret_Val;
   qapi_Status_t                       Result;
   ZCL_Demo_Cluster_Info_t            *ClusterInfo;

   /* Ensure both the stack is initialized. */
   if(GetZigBeeHandle() != NULL)
   {
      Ret_Val = QCLI_STATUS_SUCCESS_E;

      if((Parameter_Count >= 1) &&
         (Verify_Integer_Parameter(&(Parameter_List[0]), QAPI_ZB_APS_MIN_ENDPOINT, QAPI_ZB_APS_MAX_ENDPOINT)))
      {
         ClusterInfo = ZCL_FindClusterByEndpoint((uint8_t)(Parameter_List[0].Integer_Value), QAPI_ZB_CL_CLUSTER_ID_TOUCHLINK_COMMISSIONING, ZCL_DEMO_CLUSTERTYPE_CLIENT);
         if(ClusterInfo != NULL)
         {
            Result = qc_drv_ZB_CL_Touchlink_Scan_Request(qc_api_get_qc_drv_context(), ClusterInfo->Handle);
            if(Result == QAPI_OK)
            {
               Display_Function_Success("qc_drv_ZB_CL_Touchlink_Scan_Request");
            }
            else
            {
               Ret_Val = QCLI_STATUS_ERROR_E;
               Display_Function_Error("qc_drv_ZB_CL_Touchlink_Scan_Request", Result);
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
   @brief Executes the "FactoryReset" command to remove a scene.

   Parameter_List[0] Endpoint of the Touchlink client cluster to use.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_api_cmd_ZCL_Touchlink_FactoryReset(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t    Ret_Val;
   qapi_Status_t            Result;
   ZCL_Demo_Cluster_Info_t *ClusterInfo;

   /* Ensure both the stack is initialized. */
   if(GetZigBeeHandle() != NULL)
   {
      Ret_Val = QCLI_STATUS_SUCCESS_E;

      if((Parameter_Count >= 1) &&
         (Verify_Integer_Parameter(&(Parameter_List[0]), QAPI_ZB_APS_MIN_ENDPOINT, QAPI_ZB_APS_MAX_ENDPOINT)))
      {
         ClusterInfo = ZCL_FindClusterByEndpoint((uint8_t)(Parameter_List[0].Integer_Value), QAPI_ZB_CL_CLUSTER_ID_TOUCHLINK_COMMISSIONING, ZCL_DEMO_CLUSTERTYPE_CLIENT);

         if(ClusterInfo != NULL)
         {
            Result = qc_drv_ZB_CL_Touchlink_Factory_Reset(qc_api_get_qc_drv_context(), ClusterInfo->Handle);
            if(Result == QAPI_OK)
            {
               Display_Function_Success("qc_drv_ZB_CL_Touchlink_Factory_Reset");
            }
            else
            {
               Ret_Val = QCLI_STATUS_ERROR_E;
               Display_Function_Error("qc_drv_ZB_CL_Touchlink_Factory_Reset", Result);
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
   @brief Executes the "SendMoveToHue" command.

   Parameter_List[0] is the index of the device to send to. Use index zero to
                     use the binding table (if setup).
   Parameter_List[1] Endpoint of the Color Control client cluster to use to send
                     the command.
   Parameter_List[2] is the hue that is moved to.
   Parameter_List[3] is the direction of the move.
   Parameter_List[4] is the transition time.
   Parameter_List[5] is a flag indicating if this is an enhanced hue command
                     (1) or a normal one (0).

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_api_cmd_ZCL_ColorControl_MoveToHue(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t                Ret_Val;
   qapi_Status_t                        Result;
   ZCL_Demo_Cluster_Info_t             *ClusterInfo;
   qapi_ZB_CL_General_Send_Info_t       SendInfo;
   uint32_t                             DeviceId;
   uint16_t                             Hue;
   qapi_ZB_CL_ColorControl_Move_Mode_t  MoveDirection;
   uint16_t                             TransTime;
   qbool_t                              IsEnhanced;

   if(GetZigBeeHandle() != NULL)
   {
      if((Parameter_Count >= 5) &&
         (Parameter_List[0].Integer_Is_Valid) &&
         (Verify_Integer_Parameter(&(Parameter_List[1]), QAPI_ZB_APS_MIN_ENDPOINT, QAPI_ZB_APS_MAX_ENDPOINT)) &&
         (Verify_Integer_Parameter(&(Parameter_List[2]), 0, 3)) &&
         (Verify_Integer_Parameter(&(Parameter_List[3]), 0, 0xFFFF)) &&
         (Verify_Integer_Parameter(&(Parameter_List[4]), 0, 0xFFFF)) &&
         (Verify_Integer_Parameter(&(Parameter_List[5]), 0, 1)))
      {
         DeviceId      = Parameter_List[0].Integer_Value;
         ClusterInfo   = ZCL_FindClusterByEndpoint((uint8_t)(Parameter_List[1].Integer_Value), QAPI_ZB_CL_CLUSTER_ID_COLOR_CONTROL, ZCL_DEMO_CLUSTERTYPE_CLIENT);
         MoveDirection = (qapi_ZB_CL_ColorControl_Move_Mode_t)(Parameter_List[2].Integer_Value);
         Hue           = (uint16_t)Parameter_List[3].Integer_Value;
         TransTime     = Parameter_List[4].Integer_Value;
         IsEnhanced    = (Parameter_List[5].Integer_Value != 0) ? true : false;

         if(ClusterInfo != NULL)
         {
            memset(&SendInfo, 0, sizeof(SendInfo));

            /* Format the destination addr. mode, address, and endpoint. */
            if(Format_Send_Info_By_Device(DeviceId, &SendInfo))
            {
               Result = qc_drv_ZB_CL_ColorControl_Send_Move_To_Hue(qc_api_get_qc_drv_context(), ClusterInfo->Handle, &SendInfo, Hue, MoveDirection, TransTime, IsEnhanced);
               if(Result == QAPI_OK)
               {
                  Ret_Val = QCLI_STATUS_SUCCESS_E;
                  Display_Function_Success("qc_drv_ZB_CL_ColorControl_Send_Move_To_Hue");
               }
               else
               {
                  Ret_Val = QCLI_STATUS_ERROR_E;
                  Display_Function_Error("qc_drv_ZB_CL_ColorControl_Send_Move_To_Hue", Result);
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
   @brief Executes the "SendMoveHue" command.

   Parameter_List[0] is the index of the device to send to. Use index zero to
                     use the binding table (if setup).
   Parameter_List[1] Endpoint of the Color Control client cluster to use to send
                     the command.
   Parameter_List[2] is the move mode.
   Parameter_List[3] is the move rate.
   Parameter_List[4] is a flag indicating if this is an enhanced hue command
                     (1) or a normal one (0).

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_api_cmd_ZCL_ColorControl_MoveHue(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t                Ret_Val;
   qapi_Status_t                        Result;
   ZCL_Demo_Cluster_Info_t             *ClusterInfo;
   qapi_ZB_CL_General_Send_Info_t       SendInfo;
   uint32_t                             DeviceId;
   qapi_ZB_CL_ColorControl_Move_Mode_t  MoveMode;
   uint16_t                             Rate;
   qbool_t                              IsEnhanced;

   if(GetZigBeeHandle() != NULL)
   {
      if((Parameter_Count >= 4) &&
         (Parameter_List[0].Integer_Is_Valid) &&
         (Verify_Integer_Parameter(&(Parameter_List[1]), QAPI_ZB_APS_MIN_ENDPOINT, QAPI_ZB_APS_MAX_ENDPOINT)) &&
         (Verify_Integer_Parameter(&(Parameter_List[2]), 2, 4)) &&
         (Verify_Integer_Parameter(&(Parameter_List[3]), 0, 0xFFFF)) &&
         (Verify_Integer_Parameter(&(Parameter_List[4]), 0, 1)))
      {
         DeviceId    = Parameter_List[0].Integer_Value;
         ClusterInfo = ZCL_FindClusterByEndpoint((uint8_t)(Parameter_List[1].Integer_Value), QAPI_ZB_CL_CLUSTER_ID_COLOR_CONTROL, ZCL_DEMO_CLUSTERTYPE_CLIENT);
         MoveMode    = (qapi_ZB_CL_ColorControl_Move_Mode_t)(Parameter_List[2].Integer_Value);
         Rate        = (uint16_t)(Parameter_List[3].Integer_Value);
         IsEnhanced  = (Parameter_List[4].Integer_Value != 0) ? true : false;

         if(ClusterInfo != NULL)
         {
            memset(&SendInfo, 0, sizeof(SendInfo));

            /* Format the destination addr. mode, address, and endpoint. */
            if(Format_Send_Info_By_Device(DeviceId, &SendInfo))
            {
               Result = qc_drv_ZB_CL_ColorControl_Send_Move_Hue(qc_api_get_qc_drv_context(), ClusterInfo->Handle, &SendInfo, MoveMode, Rate, IsEnhanced);
               if(Result == QAPI_OK)
               {
                  Ret_Val = QCLI_STATUS_SUCCESS_E;
                  Display_Function_Success("qc_drv_ZB_CL_ColorControl_Send_Move_Hue");
               }
               else
               {
                  Ret_Val = QCLI_STATUS_ERROR_E;
                  Display_Function_Error("qc_drv_ZB_CL_ColorControl_Send_Move_Hue", Result);
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
            LOG_ERR("Invalid ClusterIndex.\n");
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
   @brief Executes the "SendStepHue" command.

   Parameter_List[0] is the index of the device to send to. Use index zero to
                     use the binding table (if setup).
   Parameter_List[1] Endpoint of the Color Control client cluster to use to send
                     the command.
   Parameter_List[2] is the step mode.
   Parameter_List[3] is the step size.
   Parameter_List[4] is the transition time.
   Parameter_List[5] is a flag indicating if this is an enhanced hue command
                     (1) or a normal one (0).

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_api_cmd_ZCL_ColorControl_StepHue(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t                Ret_Val;
   qapi_Status_t                        Result;
   ZCL_Demo_Cluster_Info_t             *ClusterInfo;
   qapi_ZB_CL_General_Send_Info_t       SendInfo;
   uint32_t                             DeviceId;
   qapi_ZB_CL_ColorControl_Move_Mode_t  StepMode;
   uint16_t                             StepSize;
   uint16_t                             TransTime;
   qbool_t                              IsEnhanced;

   if(GetZigBeeHandle() != NULL)
   {
      if((Parameter_Count >= 5) &&
         (Parameter_List[0].Integer_Is_Valid) &&
         (Verify_Integer_Parameter(&(Parameter_List[1]), QAPI_ZB_APS_MIN_ENDPOINT, QAPI_ZB_APS_MAX_ENDPOINT)) &&
         (Verify_Integer_Parameter(&(Parameter_List[2]), 2, 3)) &&
         (Verify_Integer_Parameter(&(Parameter_List[3]), 0, 0xFFFF)) &&
         (Verify_Integer_Parameter(&(Parameter_List[4]), 0, 0xFFFF)) &&
         (Verify_Integer_Parameter(&(Parameter_List[5]), 0, 1)))
      {
         DeviceId    = Parameter_List[0].Integer_Value;
         ClusterInfo = ZCL_FindClusterByEndpoint((uint8_t)(Parameter_List[1].Integer_Value), QAPI_ZB_CL_CLUSTER_ID_COLOR_CONTROL, ZCL_DEMO_CLUSTERTYPE_CLIENT);
         StepMode    = (qapi_ZB_CL_ColorControl_Move_Mode_t)(Parameter_List[2].Integer_Value);
         StepSize    = (uint16_t)(Parameter_List[3].Integer_Value);
         TransTime   = (uint16_t)(Parameter_List[4].Integer_Value);
         IsEnhanced  = (Parameter_List[5].Integer_Value != 0) ? true : false;

         if(ClusterInfo != NULL)
         {
            memset(&SendInfo, 0, sizeof(SendInfo));

            /* Format the destination addr. mode, address, and endpoint. */
            if(Format_Send_Info_By_Device(DeviceId, &SendInfo))
            {
               Result = qc_drv_ZB_CL_ColorControl_Send_Step_Hue(qc_api_get_qc_drv_context(), ClusterInfo->Handle, &SendInfo, StepMode, StepSize, TransTime, IsEnhanced);
               if(Result == QAPI_OK)
               {
                  Ret_Val = QCLI_STATUS_SUCCESS_E;
                  Display_Function_Success("qc_drv_ZB_CL_ColorControl_Send_Step_Hue");
               }
               else
               {
                  Ret_Val = QCLI_STATUS_ERROR_E;
                  Display_Function_Error("qc_drv_ZB_CL_ColorControl_Send_Step_Hue", Result);
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
            LOG_ERR("Invalid ClusterIndex.\n");
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
   @brief Executes the "SendMoveToSaturation" command.

   Parameter_List[0] is the index of the device to send to. Use index zero to
                     use the binding table (if setup).
   Parameter_List[1] Endpoint of the Color Control client cluster to use to send
                     the command.
   Parameter_List[2] is the saturation that is moved to.
   Parameter_List[3] is the transition time.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_api_cmd_ZCL_ColorControl_MoveToSaturation(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t           Ret_Val;
   qapi_Status_t                   Result;
   ZCL_Demo_Cluster_Info_t        *ClusterInfo;
   qapi_ZB_CL_General_Send_Info_t  SendInfo;
   uint32_t                        DeviceId;
   uint8_t                         Saturation;
   uint16_t                        TransTime;

   if(GetZigBeeHandle() != NULL)
   {
      if((Parameter_Count >= 3) &&
         (Parameter_List[0].Integer_Is_Valid) &&
         (Verify_Integer_Parameter(&(Parameter_List[1]), QAPI_ZB_APS_MIN_ENDPOINT, QAPI_ZB_APS_MAX_ENDPOINT)) &&
         (Verify_Integer_Parameter(&(Parameter_List[2]), 0, 0xFF)) &&
         (Verify_Integer_Parameter(&(Parameter_List[3]), 0, 0xFFFF)))
      {
         DeviceId    = Parameter_List[0].Integer_Value;
         ClusterInfo = ZCL_FindClusterByEndpoint((uint8_t)(Parameter_List[1].Integer_Value), QAPI_ZB_CL_CLUSTER_ID_COLOR_CONTROL, ZCL_DEMO_CLUSTERTYPE_CLIENT);
         Saturation  = (uint8_t)(Parameter_List[2].Integer_Value);
         TransTime   = (uint16_t)(Parameter_List[3].Integer_Value);

         if(ClusterInfo != NULL)
         {
            memset(&SendInfo, 0, sizeof(SendInfo));

            /* Format the destination addr. mode, address, and endpoint. */
            if(Format_Send_Info_By_Device(DeviceId, &SendInfo))
            {
               Result = qc_drv_ZB_CL_ColorControl_Send_Move_To_Saturation(qc_api_get_qc_drv_context(), ClusterInfo->Handle, &SendInfo, Saturation, TransTime);
               if(Result == QAPI_OK)
               {
                  Ret_Val = QCLI_STATUS_SUCCESS_E;
                  Display_Function_Success("qc_drv_ZB_CL_ColorControl_Send_Move_To_Saturation");
               }
               else
               {
                  Ret_Val = QCLI_STATUS_ERROR_E;
                  Display_Function_Error("qc_drv_ZB_CL_ColorControl_Send_Move_To_Saturation", Result);
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
            LOG_ERR("Invalid ClusterIndex.\n");
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
   @brief Executes the "SendMoveSaturation" command.

   Parameter_List[0] is the index of the device to send to. Use index zero to
                     use the binding table (if setup).
   Parameter_List[1] Endpoint of the Color Control client cluster to use to send
                     the command.
   Parameter_List[2] is the move mode.
   Parameter_List[3] is the rate.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_api_cmd_ZCL_ColorControl_MoveSaturation(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t                Ret_Val;
   qapi_Status_t                        Result;
   ZCL_Demo_Cluster_Info_t             *ClusterInfo;
   qapi_ZB_CL_General_Send_Info_t       SendInfo;
   uint32_t                             DeviceId;
   qapi_ZB_CL_ColorControl_Move_Mode_t  MoveMode;
   uint8_t                              Rate;

   if(GetZigBeeHandle() != NULL)
   {
      if((Parameter_Count >= 3) &&
         (Parameter_List[0].Integer_Is_Valid) &&
         (Verify_Integer_Parameter(&(Parameter_List[1]), QAPI_ZB_APS_MIN_ENDPOINT, QAPI_ZB_APS_MAX_ENDPOINT)) &&
         (Verify_Integer_Parameter(&(Parameter_List[2]), 2, 4)) &&
         (Verify_Integer_Parameter(&(Parameter_List[3]), 0, 0xFF)))
      {
         DeviceId    = Parameter_List[0].Integer_Value;
         ClusterInfo = ZCL_FindClusterByEndpoint((uint8_t)(Parameter_List[1].Integer_Value), QAPI_ZB_CL_CLUSTER_ID_COLOR_CONTROL, ZCL_DEMO_CLUSTERTYPE_CLIENT);
         MoveMode    = (qapi_ZB_CL_ColorControl_Move_Mode_t)(Parameter_List[2].Integer_Value);
         Rate        = (uint8_t)(Parameter_List[3].Integer_Value);

         if(ClusterInfo != NULL)
         {
            memset(&SendInfo, 0, sizeof(SendInfo));

            /* Format the destination addr. mode, address, and endpoint. */
            if(Format_Send_Info_By_Device(DeviceId, &SendInfo))
            {
               Result = qc_drv_ZB_CL_ColorControl_Send_Move_Saturation(qc_api_get_qc_drv_context(), ClusterInfo->Handle, &SendInfo, MoveMode, Rate);
               if(Result == QAPI_OK)
               {
                  Ret_Val = QCLI_STATUS_SUCCESS_E;
                  Display_Function_Success("qc_drv_ZB_CL_ColorControl_Send_Move_Saturation");
               }
               else
               {
                  Ret_Val = QCLI_STATUS_ERROR_E;
                  Display_Function_Error("qc_drv_ZB_CL_ColorControl_Send_Move_Saturation", Result);
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
            LOG_ERR("Invalid ClusterIndex.\n");
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
   @brief Executes the "SendStepSaturation" command.

   Parameter_List[0] is the index of the device to send to. Use index zero to
                     use the binding table (if setup).
   Parameter_List[1] Endpoint of the Color Control client cluster to use to send
                     the command.
   Parameter_List[2] is the step mode.
   Parameter_List[3] is the step size.
   Parameter_List[4] is the transition time.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_api_cmd_ZCL_ColorControl_StepSaturation(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t                Ret_Val;
   qapi_Status_t                        Result;
   ZCL_Demo_Cluster_Info_t             *ClusterInfo;
   qapi_ZB_CL_General_Send_Info_t       SendInfo;
   uint32_t                             DeviceId;
   qapi_ZB_CL_ColorControl_Move_Mode_t  StepMode;
   uint8_t                              StepSize;
   uint8_t                              TransTime;

   if(GetZigBeeHandle() != NULL)
   {
      if((Parameter_Count >= 4) &&
         (Parameter_List[0].Integer_Is_Valid) &&
         (Verify_Integer_Parameter(&(Parameter_List[1]), QAPI_ZB_APS_MIN_ENDPOINT, QAPI_ZB_APS_MAX_ENDPOINT)) &&
         (Verify_Integer_Parameter(&(Parameter_List[2]), 2, 3)) &&
         (Verify_Integer_Parameter(&(Parameter_List[3]), 0, 0xFF)) &&
         (Verify_Integer_Parameter(&(Parameter_List[4]), 0, 0xFF)))
      {
         DeviceId    = Parameter_List[0].Integer_Value;
         ClusterInfo = ZCL_FindClusterByEndpoint((uint8_t)(Parameter_List[1].Integer_Value), QAPI_ZB_CL_CLUSTER_ID_COLOR_CONTROL, ZCL_DEMO_CLUSTERTYPE_CLIENT);
         StepMode    = (qapi_ZB_CL_ColorControl_Move_Mode_t)(Parameter_List[2].Integer_Value);
         StepSize    = (uint8_t)(Parameter_List[3].Integer_Value);
         TransTime   = (uint8_t)(Parameter_List[4].Integer_Value);

         if(ClusterInfo != NULL)
         {
            memset(&SendInfo, 0, sizeof(SendInfo));

            /* Format the destination addr. mode, address, and endpoint. */
            if(Format_Send_Info_By_Device(DeviceId, &SendInfo))
            {
               Result = qc_drv_ZB_CL_ColorControl_Send_Step_Saturation(qc_api_get_qc_drv_context(), ClusterInfo->Handle, &SendInfo, StepMode, StepSize, TransTime);
               if(Result == QAPI_OK)
               {
                  Ret_Val = QCLI_STATUS_SUCCESS_E;
                  Display_Function_Success("qc_drv_ZB_CL_ColorControl_Send_Step_Saturation");
               }
               else
               {
                  Ret_Val = QCLI_STATUS_ERROR_E;
                  Display_Function_Error("qc_drv_ZB_CL_ColorControl_Send_Step_Saturation", Result);
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
            LOG_ERR("Invalid ClusterIndex.\n");
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
   @brief Executes the "SendMoveToHueAndSaturation" command.

   Parameter_List[0] is the index of the device to send to. Use index zero to
                     use the binding table (if setup).
   Parameter_List[1] Endpoint of the Color Control client cluster to use to send
                     the command.
   Parameter_List[2] is the hue that is moved to.
   Parameter_List[3] is the saturation that is moved to.
   Parameter_List[4] is the transition time.
   Parameter_List[5] is a flag indicating if this is an enhanced hue command
                     (1) or a normal one (0).

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_api_cmd_ZCL_ColorControl_MoveToHueAndSaturation(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t           Ret_Val;
   qapi_Status_t                   Result;
   ZCL_Demo_Cluster_Info_t        *ClusterInfo;
   qapi_ZB_CL_General_Send_Info_t  SendInfo;
   uint32_t                        DeviceId;
   uint16_t                        Hue;
   uint8_t                         Saturation;
   uint16_t                        TransTime;
   qbool_t                         IsEnhanced;

   if(GetZigBeeHandle() != NULL)
   {
      if((Parameter_Count >= 5) &&
         (Parameter_List[0].Integer_Is_Valid) &&
         (Verify_Integer_Parameter(&(Parameter_List[1]), QAPI_ZB_APS_MIN_ENDPOINT, QAPI_ZB_APS_MAX_ENDPOINT)) &&
         (Verify_Integer_Parameter(&(Parameter_List[2]), 0, 0xFFFF)) &&
         (Verify_Integer_Parameter(&(Parameter_List[3]), 0, 0xFF)) &&
         (Verify_Integer_Parameter(&(Parameter_List[4]), 0, 0xFFFF)) &&
         (Verify_Integer_Parameter(&(Parameter_List[5]), 0, 1)))
      {
         DeviceId    = Parameter_List[0].Integer_Value;
         ClusterInfo = ZCL_FindClusterByEndpoint((uint8_t)(Parameter_List[1].Integer_Value), QAPI_ZB_CL_CLUSTER_ID_COLOR_CONTROL, ZCL_DEMO_CLUSTERTYPE_CLIENT);
         Hue         = (uint16_t)(Parameter_List[2].Integer_Value);
         Saturation  = (uint8_t)(Parameter_List[3].Integer_Value);
         TransTime   = (uint16_t)(Parameter_List[4].Integer_Value);
         IsEnhanced  = (Parameter_List[5].Integer_Value != 0) ? true : false;

         if(ClusterInfo != NULL)
         {
            memset(&SendInfo, 0, sizeof(SendInfo));

            /* Format the destination addr. mode, address, and endpoint. */
            if(Format_Send_Info_By_Device(DeviceId, &SendInfo))
            {
               Result = qc_drv_ZB_CL_ColorControl_Send_Move_To_HueAndSaturation(qc_api_get_qc_drv_context(), ClusterInfo->Handle, &SendInfo, Hue, Saturation, TransTime, IsEnhanced);
               if(Result == QAPI_OK)
               {
                  Ret_Val = QCLI_STATUS_SUCCESS_E;
                  Display_Function_Success("qc_drv_ZB_CL_ColorControl_Send_Move_To_HueAndSaturation");
               }
               else
               {
                  Ret_Val = QCLI_STATUS_ERROR_E;
                  Display_Function_Error("qc_drv_ZB_CL_ColorControl_Send_Move_To_HueAndSaturation", Result);
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
            LOG_ERR("Invalid ClusterIndex.\n");
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
   @brief Executes the "SendMoveToColor" command.

   Parameter_List[0] is the index of the device to send to. Use index zero to
                     use the binding table (if setup).
   Parameter_List[1] Endpoint of the Color Control client cluster to use to send
                     the command.
   Parameter_List[2] is the color X.
   Parameter_List[3] is the color Y.
   Parameter_List[4] is the transition time.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_api_cmd_ZCL_ColorControl_MoveToColor(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t           Ret_Val;
   qapi_Status_t                   Result;
   ZCL_Demo_Cluster_Info_t        *ClusterInfo;
   qapi_ZB_CL_General_Send_Info_t  SendInfo;
   uint32_t                        DeviceId;
   uint16_t                        ColorX;
   uint16_t                        ColorY;
   uint16_t                        TransTime;

   if(GetZigBeeHandle() != NULL)
   {
      if((Parameter_Count >= 4) &&
         (Parameter_List[0].Integer_Is_Valid) &&
         (Verify_Integer_Parameter(&(Parameter_List[1]), QAPI_ZB_APS_MIN_ENDPOINT, QAPI_ZB_APS_MAX_ENDPOINT)) &&
         (Verify_Integer_Parameter(&(Parameter_List[2]), 0, 0xFFFF)) &&
         (Verify_Integer_Parameter(&(Parameter_List[3]), 0, 0xFFFF)) &&
         (Verify_Integer_Parameter(&(Parameter_List[4]), 0, 0xFFFF)))
      {
         DeviceId    = Parameter_List[0].Integer_Value;
         ClusterInfo = ZCL_FindClusterByEndpoint((uint8_t)(Parameter_List[1].Integer_Value), QAPI_ZB_CL_CLUSTER_ID_COLOR_CONTROL, ZCL_DEMO_CLUSTERTYPE_CLIENT);
         ColorX      = (uint16_t)(Parameter_List[2].Integer_Value);
         ColorY      = (uint16_t)(Parameter_List[3].Integer_Value);
         TransTime   = (uint16_t)(Parameter_List[4].Integer_Value);

         if(ClusterInfo != NULL)
         {
            memset(&SendInfo, 0, sizeof(SendInfo));

            /* Format the destination addr. mode, address, and endpoint. */
            if(Format_Send_Info_By_Device(DeviceId, &SendInfo))
            {
               Result = qc_drv_ZB_CL_ColorControl_Send_Move_To_Color(qc_api_get_qc_drv_context(), ClusterInfo->Handle, &SendInfo, ColorX, ColorY, TransTime);
               if(Result == QAPI_OK)
               {
                  Ret_Val = QCLI_STATUS_SUCCESS_E;
                  Display_Function_Success("qc_drv_ZB_CL_ColorControl_Send_Move_To_Color");
               }
               else
               {
                  Ret_Val = QCLI_STATUS_ERROR_E;
                  Display_Function_Error("qc_drv_ZB_CL_ColorControl_Send_Move_To_Color", Result);
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
            LOG_ERR("Invalid ClusterIndex.\n");
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
   @brief Executes the "SendMoveColor" command.

   Parameter_List[0] is the index of the device to send to. Use index zero to
                     use the binding table (if setup).
   Parameter_List[1] Endpoint of the Color Control client cluster to use to send
                     the command.
   Parameter_List[2] is the rate X.
   Parameter_List[3] is the rate Y.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_api_cmd_ZCL_ColorControl_MoveColor(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t           Ret_Val;
   qapi_Status_t                   Result;
   ZCL_Demo_Cluster_Info_t        *ClusterInfo;
   qapi_ZB_CL_General_Send_Info_t  SendInfo;
   uint32_t                        DeviceId;
   int16_t                         RateX;
   int16_t                         RateY;

   if(GetZigBeeHandle() != NULL)
   {
      if((Parameter_Count >= 3) &&
         (Parameter_List[0].Integer_Is_Valid) &&
         (Verify_Integer_Parameter(&(Parameter_List[1]), QAPI_ZB_APS_MIN_ENDPOINT, QAPI_ZB_APS_MAX_ENDPOINT)) &&
         (Verify_Integer_Parameter(&(Parameter_List[2]), -32768, 32767)) &&
         (Verify_Integer_Parameter(&(Parameter_List[3]), -32768, 32767)))
      {
         DeviceId    = Parameter_List[0].Integer_Value;
         ClusterInfo = ZCL_FindClusterByEndpoint((uint8_t)(Parameter_List[1].Integer_Value), QAPI_ZB_CL_CLUSTER_ID_COLOR_CONTROL, ZCL_DEMO_CLUSTERTYPE_CLIENT);
         RateX       = (int16_t)(Parameter_List[2].Integer_Value);
         RateY       = (int16_t)(Parameter_List[3].Integer_Value);

         if(ClusterInfo != NULL)
         {
            memset(&SendInfo, 0, sizeof(SendInfo));

            /* Format the destination addr. mode, address, and endpoint. */
            if(Format_Send_Info_By_Device(DeviceId, &SendInfo))
            {
               Result = qc_drv_ZB_CL_ColorControl_Send_Move_Color(qc_api_get_qc_drv_context(), ClusterInfo->Handle, &SendInfo, RateX, RateY);
               if(Result == QAPI_OK)
               {
                  Ret_Val = QCLI_STATUS_SUCCESS_E;
                  Display_Function_Success("qc_drv_ZB_CL_ColorControl_Send_Move_Color");
               }
               else
               {
                  Ret_Val = QCLI_STATUS_ERROR_E;
                  Display_Function_Error("qc_drv_ZB_CL_ColorControl_Send_Move_Color", Result);
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
            LOG_ERR("Invalid ClusterIndex.\n");
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
   @brief Executes the "SendStepColor" command.

   Parameter_List[0] is the index of the device to send to. Use index zero to
                     use the binding table (if setup).
   Parameter_List[1] Endpoint of the Color Control client cluster to use to send
                     the command.
   Parameter_List[2] is the step X.
   Parameter_List[3] is the step Y.
   Parameter_List[4] is the transition time.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_api_cmd_ZCL_ColorControl_StepColor(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t           Ret_Val;
   qapi_Status_t                   Result;
   ZCL_Demo_Cluster_Info_t        *ClusterInfo;
   qapi_ZB_CL_General_Send_Info_t  SendInfo;
   uint32_t                        DeviceId;
   int16_t                         StepX;
   int16_t                         StepY;
   uint16_t                        TransTime;

   if(GetZigBeeHandle() != NULL)
   {
      if((Parameter_Count >= 4) &&
         (Parameter_List[0].Integer_Is_Valid) &&
         (Verify_Integer_Parameter(&(Parameter_List[1]), QAPI_ZB_APS_MIN_ENDPOINT, QAPI_ZB_APS_MAX_ENDPOINT)) &&
         (Verify_Integer_Parameter(&(Parameter_List[2]), -32768, 32767)) &&
         (Verify_Integer_Parameter(&(Parameter_List[3]), -32768, 32767)) &&
         (Verify_Integer_Parameter(&(Parameter_List[4]), 0, 0xFFFF)))
      {
         DeviceId    = Parameter_List[0].Integer_Value;
         ClusterInfo = ZCL_FindClusterByEndpoint((uint8_t)(Parameter_List[1].Integer_Value), QAPI_ZB_CL_CLUSTER_ID_COLOR_CONTROL, ZCL_DEMO_CLUSTERTYPE_CLIENT);
         StepX       = (int16_t)(Parameter_List[2].Integer_Value);
         StepY       = (int16_t)(Parameter_List[3].Integer_Value);
         TransTime   = (uint16_t)(Parameter_List[4].Integer_Value);

         if(ClusterInfo != NULL)
         {
            memset(&SendInfo, 0, sizeof(SendInfo));

            /* Format the destination addr. mode, address, and endpoint. */
            if(Format_Send_Info_By_Device(DeviceId, &SendInfo))
            {
               Result = qc_drv_ZB_CL_ColorControl_Send_Step_Color(qc_api_get_qc_drv_context(), ClusterInfo->Handle, &SendInfo, StepX, StepY, TransTime);
               if(Result == QAPI_OK)
               {
                  Ret_Val = QCLI_STATUS_SUCCESS_E;
                  Display_Function_Success("qc_drv_ZB_CL_ColorControl_Send_Step_Color");
               }
               else
               {
                  Ret_Val = QCLI_STATUS_ERROR_E;
                  Display_Function_Error("qc_drv_ZB_CL_ColorControl_Send_Step_Color", Result);
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
            LOG_ERR("Invalid ClusterIndex.\n");
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
   @brief Executes the "SendMoveToColorTemp" command.

   Parameter_List[0] is the index of the device to send to. Use index zero to
                     use the binding table (if setup).
   Parameter_List[1] Endpoint of the Color Control client cluster to use to send
                     the command.
   Parameter_List[2] is the color temp mireds.
   Parameter_List[3] is the transition time.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_api_cmd_ZCL_ColorControl_MoveToColorTemp(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t           Ret_Val;
   qapi_Status_t                   Result;
   ZCL_Demo_Cluster_Info_t        *ClusterInfo;
   qapi_ZB_CL_General_Send_Info_t  SendInfo;
   uint32_t                        DeviceId;
   uint16_t                        Mireds;
   uint16_t                        TransTime;

   if(GetZigBeeHandle() != NULL)
   {
      if((Parameter_Count >= 3) &&
         (Parameter_List[0].Integer_Is_Valid) &&
         (Verify_Integer_Parameter(&(Parameter_List[1]), QAPI_ZB_APS_MIN_ENDPOINT, QAPI_ZB_APS_MAX_ENDPOINT)) &&
         (Verify_Integer_Parameter(&(Parameter_List[2]), 0, 0xFFFF)) &&
         (Verify_Integer_Parameter(&(Parameter_List[3]), 0, 0xFFFF)))
      {
         DeviceId    = Parameter_List[0].Integer_Value;
         ClusterInfo = ZCL_FindClusterByEndpoint((uint8_t)(Parameter_List[1].Integer_Value), QAPI_ZB_CL_CLUSTER_ID_COLOR_CONTROL, ZCL_DEMO_CLUSTERTYPE_CLIENT);
         Mireds      = (uint16_t)(Parameter_List[2].Integer_Value);
         TransTime   = (uint16_t)(Parameter_List[3].Integer_Value);

         if(ClusterInfo != NULL)
         {
            memset(&SendInfo, 0, sizeof(SendInfo));

            /* Format the destination addr. mode, address, and endpoint. */
            if(Format_Send_Info_By_Device(DeviceId, &SendInfo))
            {
               Result = qc_drv_ZB_CL_ColorControl_Send_Move_To_Color_Temp(qc_api_get_qc_drv_context(), ClusterInfo->Handle, &SendInfo, Mireds, TransTime);
               if(Result == QAPI_OK)
               {
                  Ret_Val = QCLI_STATUS_SUCCESS_E;
                  Display_Function_Success("qc_drv_ZB_CL_ColorControl_Send_Move_To_Color_Temp");
               }
               else
               {
                  Ret_Val = QCLI_STATUS_ERROR_E;
                  Display_Function_Error("qc_drv_ZB_CL_ColorControl_Send_Move_To_Color_Temp", Result);
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
            LOG_ERR("Invalid ClusterIndex.\n");
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
   @brief Executes the "SendMoveColorTemp" command.

   Parameter_List[0] is the index of the device to send to. Use index zero to
                     use the binding table (if setup).
   Parameter_List[1] Endpoint of the Color Control client cluster to use to send
                     the command.
   Parameter_List[2] is the move mode.
   Parameter_List[3] is the rate.
   Parameter_List[4] is the limit for the color temp.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_api_cmd_ZCL_ColorControl_MoveColorTemp(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t                Ret_Val;
   qapi_Status_t                        Result;
   ZCL_Demo_Cluster_Info_t             *ClusterInfo;
   qapi_ZB_CL_General_Send_Info_t       SendInfo;
   uint32_t                             DeviceId;
   qapi_ZB_CL_ColorControl_Move_Mode_t  MoveMode;
   uint16_t                             Rate;
   uint16_t                             Limit;

   if(GetZigBeeHandle() != NULL)
   {
      if((Parameter_Count >= 4) &&
         (Parameter_List[0].Integer_Is_Valid) &&
         (Verify_Integer_Parameter(&(Parameter_List[1]), QAPI_ZB_APS_MIN_ENDPOINT, QAPI_ZB_APS_MAX_ENDPOINT)) &&
         (Verify_Integer_Parameter(&(Parameter_List[2]), 2, 4)) &&
         (Verify_Integer_Parameter(&(Parameter_List[3]), 0, 0xFFFF)) &&
         (Verify_Integer_Parameter(&(Parameter_List[4]), 0, 0xFFFF)))
      {
         DeviceId    = Parameter_List[0].Integer_Value;
         ClusterInfo = ZCL_FindClusterByEndpoint((uint8_t)(Parameter_List[1].Integer_Value), QAPI_ZB_CL_CLUSTER_ID_COLOR_CONTROL, ZCL_DEMO_CLUSTERTYPE_CLIENT);
         MoveMode    = (qapi_ZB_CL_ColorControl_Move_Mode_t)(Parameter_List[2].Integer_Value);
         Rate        = (uint16_t)(Parameter_List[3].Integer_Value);
         Limit       = (uint16_t)(Parameter_List[4].Integer_Value);

         if(ClusterInfo != NULL)
         {
            memset(&SendInfo, 0, sizeof(SendInfo));

            /* Format the destination addr. mode, address, and endpoint. */
            if(Format_Send_Info_By_Device(DeviceId, &SendInfo))
            {
               Result = qc_drv_ZB_CL_ColorControl_Send_Move_Color_Temp(qc_api_get_qc_drv_context(), ClusterInfo->Handle, &SendInfo, MoveMode, Rate, Limit);
               if(Result == QAPI_OK)
               {
                  Ret_Val = QCLI_STATUS_SUCCESS_E;
                  Display_Function_Success("qc_drv_ZB_CL_ColorControl_Send_Move_Color_Temp");
               }
               else
               {
                  Ret_Val = QCLI_STATUS_ERROR_E;
                  Display_Function_Error("qc_drv_ZB_CL_ColorControl_Send_Move_Color_Temp", Result);
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
            LOG_ERR("Invalid ClusterIndex.\n");
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
   @brief Executes the "SendStepColorTemp" command.

   Parameter_List[0] is the index of the device to send to. Use index zero to
                     use the binding table (if setup).
   Parameter_List[1] Endpoint of the Color Control client cluster to use to send
                     the command.
   Parameter_List[2] is the step mode.
   Parameter_List[3] is the step size.
   Parameter_List[4] is the transition time.
   Parameter_List[5] is the min color temp mireds.
   Parameter_List[6] is the max color temp mireds.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_api_cmd_ZCL_ColorControl_StepColorTemp(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t                      Ret_Val;
   qapi_Status_t                              Result;
   ZCL_Demo_Cluster_Info_t                   *ClusterInfo;
   qapi_ZB_CL_General_Send_Info_t             SendInfo;
   uint32_t                                   DeviceId;
   qapi_ZB_CL_ColorControl_Step_Color_Temp_t  StepColorTemp;

   if(GetZigBeeHandle() != NULL)
   {
      if((Parameter_Count >= 5) &&
         (Parameter_List[0].Integer_Is_Valid) &&
         (Verify_Integer_Parameter(&(Parameter_List[1]), QAPI_ZB_APS_MIN_ENDPOINT, QAPI_ZB_APS_MAX_ENDPOINT)) &&
         (Verify_Integer_Parameter(&(Parameter_List[2]), 2, 3)) &&
         (Verify_Integer_Parameter(&(Parameter_List[3]), 0, 0xFFFF)) &&
         (Verify_Integer_Parameter(&(Parameter_List[4]), 0, 0xFFFF)) &&
         (Verify_Integer_Parameter(&(Parameter_List[5]), 0, 0xFFFF)))
      {
         memset(&StepColorTemp, 0, sizeof(qapi_ZB_CL_ColorControl_Step_Color_Temp_t));
         DeviceId                     = Parameter_List[0].Integer_Value;
         ClusterInfo                  = ZCL_FindClusterByEndpoint((uint8_t)(Parameter_List[1].Integer_Value), QAPI_ZB_CL_CLUSTER_ID_COLOR_CONTROL, ZCL_DEMO_CLUSTERTYPE_CLIENT);
         StepColorTemp.StepMode       = (qapi_ZB_CL_ColorControl_Move_Mode_t)(Parameter_List[2].Integer_Value);
         StepColorTemp.StepSize       = (uint16_t)(Parameter_List[3].Integer_Value);
         StepColorTemp.TransitionTime = (uint16_t)(Parameter_List[4].Integer_Value);
         StepColorTemp.Limit          = (uint16_t)(Parameter_List[5].Integer_Value);

         if(ClusterInfo != NULL)
         {
            memset(&SendInfo, 0, sizeof(SendInfo));

            /* Format the destination addr. mode, address, and endpoint. */
            if(Format_Send_Info_By_Device(DeviceId, &SendInfo))
            {
               Result = qc_drv_ZB_CL_ColorControl_Send_Step_Color_Temp(qc_api_get_qc_drv_context(), ClusterInfo->Handle, &SendInfo, &StepColorTemp);
               if(Result == QAPI_OK)
               {
                  Ret_Val = QCLI_STATUS_SUCCESS_E;
                  Display_Function_Success("qc_drv_ZB_CL_ColorControl_Send_Step_Color_Temp");
               }
               else
               {
                  Ret_Val = QCLI_STATUS_ERROR_E;
                  Display_Function_Error("qc_drv_ZB_CL_ColorControl_Send_Step_Color_Temp", Result);
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
            LOG_ERR("Invalid ClusterIndex.\n");
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
   @brief Executes the "SendStopMoveStep" command.

   Parameter_List[0] is the index of the device to send to. Use index zero to
                     use the binding table (if setup).
   Parameter_List[1] Endpoint of the Color Control client cluster to use to send
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
QCLI_Command_Status_t qc_api_cmd_ZCL_ColorControl_StopMoveStep(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t           Ret_Val;
   qapi_Status_t                   Result;
   ZCL_Demo_Cluster_Info_t        *ClusterInfo;
   qapi_ZB_CL_General_Send_Info_t  SendInfo;
   uint32_t                        DeviceId;

   if(GetZigBeeHandle() != NULL)
   {
      if((Parameter_Count >= 1) &&
         (Parameter_List[0].Integer_Is_Valid) &&
         (Verify_Integer_Parameter(&(Parameter_List[1]), QAPI_ZB_APS_MIN_ENDPOINT, QAPI_ZB_APS_MAX_ENDPOINT)))
      {
         DeviceId    = Parameter_List[0].Integer_Value;
         ClusterInfo = ZCL_FindClusterByEndpoint((uint8_t)(Parameter_List[1].Integer_Value), QAPI_ZB_CL_CLUSTER_ID_COLOR_CONTROL, ZCL_DEMO_CLUSTERTYPE_CLIENT);

         if(ClusterInfo != NULL)
         {
            memset(&SendInfo, 0, sizeof(SendInfo));

            /* Format the destination addr. mode, address, and endpoint. */
            if(Format_Send_Info_By_Device(DeviceId, &SendInfo))
            {
               Result = qc_drv_ZB_CL_ColorControl_Send_Stop_Move_Step(qc_api_get_qc_drv_context(), ClusterInfo->Handle, &SendInfo);
               if(Result == QAPI_OK)
               {
                  Ret_Val = QCLI_STATUS_SUCCESS_E;
                  Display_Function_Success("qc_drv_ZB_CL_ColorControl_Send_Stop_Move_Step");
               }
               else
               {
                  Ret_Val = QCLI_STATUS_ERROR_E;
                  Display_Function_Error("qc_drv_ZB_CL_ColorControl_Send_Stop_Move_Step", Result);
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
            LOG_ERR("Invalid ClusterIndex.\n");
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
   @brief Executes the "SendColorLoopSet" command.

   Parameter_List[0] is the index of the device to send to. Use index zero to
                     use the binding table (if setup).
   Parameter_List[1] Endpoint of the Color Control client cluster to use to send
                     the command.
   Parameter_List[2] is the update flag.
   Parameter_List[3] is the action mode.
   Parameter_List[4] is the direction mode.
   Parameter_List[5] is the time.
   Parameter_List[6] is the start hue.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_api_cmd_ZCL_ColorControl_ColorLoopSet(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t                     Ret_Val;
   qapi_Status_t                             Result;
   ZCL_Demo_Cluster_Info_t                  *ClusterInfo;
   qapi_ZB_CL_General_Send_Info_t            SendInfo;
   uint32_t                                  DeviceId;
   qapi_ZB_CL_ColorControl_Color_Loop_Set_t  ColorLoopSet;

   if(GetZigBeeHandle() != NULL)
   {
      if((Parameter_Count >= 7) &&
         (Parameter_List[0].Integer_Is_Valid) &&
         (Verify_Integer_Parameter(&(Parameter_List[1]), QAPI_ZB_APS_MIN_ENDPOINT, QAPI_ZB_APS_MAX_ENDPOINT)) &&
         (Verify_Integer_Parameter(&(Parameter_List[2]), 0, 0xFF)) &&
         (Verify_Integer_Parameter(&(Parameter_List[3]), 0, 2)) &&
         (Verify_Integer_Parameter(&(Parameter_List[4]), 2, 3)) &&
         (Verify_Integer_Parameter(&(Parameter_List[5]), 0, 0xFFFF)) &&
         (Verify_Integer_Parameter(&(Parameter_List[6]), 0, 0xFFFF)))
      {
         memset(&ColorLoopSet, 0, sizeof(qapi_ZB_CL_ColorControl_Color_Loop_Set_t));

         DeviceId                = Parameter_List[0].Integer_Value;
         ClusterInfo             = ZCL_FindClusterByEndpoint((uint8_t)(Parameter_List[1].Integer_Value), QAPI_ZB_CL_CLUSTER_ID_COLOR_CONTROL, ZCL_DEMO_CLUSTERTYPE_CLIENT);
         ColorLoopSet.UpdateFlag = (uint8_t)(Parameter_List[2].Integer_Value);
         ColorLoopSet.Action     = (qapi_ZB_CL_ColorControl_Loop_Action_t)(Parameter_List[3].Integer_Value);
         ColorLoopSet.Direction  = (qapi_ZB_CL_ColorControl_Move_Mode_t)(Parameter_List[4].Integer_Value);
         ColorLoopSet.Time       = (uint16_t)(Parameter_List[5].Integer_Value);
         ColorLoopSet.StartHue   = (uint16_t)(Parameter_List[6].Integer_Value);

         if(ClusterInfo != NULL)
         {
            memset(&SendInfo, 0, sizeof(SendInfo));

            /* Format the destination addr. mode, address, and endpoint. */
            if(Format_Send_Info_By_Device(DeviceId, &SendInfo))
            {
               Result = qc_drv_ZB_CL_ColorControl_Send_Color_Loop_Set(qc_api_get_qc_drv_context(), ClusterInfo->Handle, &SendInfo, &ColorLoopSet);
               if(Result == QAPI_OK)
               {
                  Ret_Val = QCLI_STATUS_SUCCESS_E;
                  Display_Function_Success("qc_drv_ZB_CL_ColorControl_Send_Color_Loop_Set");
               }
               else
               {
                  Ret_Val = QCLI_STATUS_ERROR_E;
                  Display_Function_Error("qc_drv_ZB_CL_ColorControl_Send_Color_Loop_Set", Result);
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
            LOG_ERR("Invalid ClusterIndex.\n");
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
#endif
QCLI_Command_Status_t qc_api_ZB_ClearPersist(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t Ret_Val;
   qapi_Status_t         Result;
   qapi_Persist_Handle_t PersistHandle;

   /* Get the handle for the persistent data. */
   if(ZigBee_Demo_Context.PersistHandle != NULL)
   {
      /* Use the persist handle we already have. */
      PersistHandle = ZigBee_Demo_Context.PersistHandle;
   }
   else
   {
      /* Initialize a temporary persist instance. */
      Result = qc_drv_Persist_Initialize(qc_api_get_qc_drv_context(), &PersistHandle, ZIGBEE_PERSIST_DIRECTORY, ZIGBEE_PERSIST_PREFIX, ZIGBEE_PERSIST_SUFFIX, NULL, 0);
      if(Result != QAPI_OK)
      {
         PersistHandle = NULL;
         Display_Function_Error("qc_drv_Persist_Initialize", Result);
      }
   }

   if(PersistHandle != NULL)
   {
      qc_drv_Persist_Delete(qc_api_get_qc_drv_context(), PersistHandle);

      LOG_INFO("Persistent data cleared.\n");

      /* Cleanup the persist instance if it was temporary. */
      if(ZigBee_Demo_Context.PersistHandle == NULL)
      {
         qc_drv_Persist_Cleanup(qc_api_get_qc_drv_context(), PersistHandle);
      }

      Ret_Val = QCLI_STATUS_SUCCESS_E;
   }
   else
   {
      Ret_Val = QCLI_STATUS_ERROR_E;
   }

   return(Ret_Val);
}

/**
   @brief Helper function that displays variable length value.

   @param Group_Handle is the QCLI group handle.
   @param Data_Length  is the length of the data to be displayed.
   @param Data         is the data to be displayed.
*/
void DisplayVariableLengthValue(uint16_t Data_Length, uint8_t *Data)
{
   union
   {
      uint8_t  ByteValue;
      uint16_t Unsigned16BitsValue;
      uint32_t Unsigned32BitsValue;
      uint64_t Unsigned64BitsValue;
   }VariableLengthValue;

   if(Data != NULL)
   {
      switch(Data_Length)
      {
         case sizeof(uint8_t):
            VariableLengthValue.ByteValue = READ_UNALIGNED_LITTLE_ENDIAN_UINT8(Data);
            LOG_INFO("0x%02X\n", VariableLengthValue.ByteValue);
            break;

         case sizeof(uint16_t):
            VariableLengthValue.Unsigned16BitsValue = READ_UNALIGNED_LITTLE_ENDIAN_UINT16(Data);
            LOG_INFO("0x%04X\n", VariableLengthValue.Unsigned16BitsValue);
            break;

         case sizeof(uint32_t):
            VariableLengthValue.Unsigned32BitsValue = READ_UNALIGNED_LITTLE_ENDIAN_UINT32(Data);
            LOG_INFO("0x%04X\n", VariableLengthValue.Unsigned32BitsValue);
            break;

         case sizeof(uint64_t):
            VariableLengthValue.Unsigned64BitsValue = READ_UNALIGNED_LITTLE_ENDIAN_UINT64(Data);
            LOG_INFO("%08X%08X\n", (uint32_t)(VariableLengthValue.Unsigned64BitsValue >> 32), (uint32_t)(VariableLengthValue.Unsigned64BitsValue));
            break;

         default:
            LOG_INFO("Unsurport Data Length.\n");
            break;
      }
   }
}

/**
   @brief Function to get a specified entry from the ZigBee demo's device list.

   @param DeviceID is the index of the device to retrieve.

   @return a pointer to the device list entry or NULL if either the DeviceID was
           not valid or not in use.
*/
ZB_Device_ID_t *GetDeviceListEntry(uint32_t DeviceID)
{
   ZB_Device_ID_t *Ret_Val;

   if(DeviceID <= DEV_ID_LIST_SIZE)
   {
      if(ZigBee_Demo_Context.DevIDList[DeviceID].InUse)
      {
         Ret_Val = &(ZigBee_Demo_Context.DevIDList[DeviceID]);
      }
      else
      {
         Ret_Val = NULL;
      }
   }
   else
   {
      Ret_Val = NULL;
   }

   return(Ret_Val);
}

/**
   @brief Function to get the ZigBee stack's handle.

   @return The handle of the ZigBee stack.
*/
qapi_ZB_Handle_t GetZigBeeHandle(void)
{
   return(ZigBee_Demo_Context.ZigBee_Handle);
}

/**
   @brief Function to get the next sequence number for sending packets.

   @return the next sequence number to be used for sending packets.
*/
uint8_t GetNextSeqNum(void)
{
   uint8_t Ret_Val = ZigBee_Demo_Context.ZCL_Sequence_Num;

   ZigBee_Demo_Context.ZCL_Sequence_Num++;
   return Ret_Val;
}

/**
   @brief Helper function to format the send information for a packet.

   @param ReceiveInfo is the receive information for an event.
   @param SendInfo    is a pointer to where the send information will be
                      formatted upon successful return.

   @return true if the send info was formatted successfully, false otherwise.
*/
qbool_t Format_Send_Info_By_Receive_Info(const qapi_ZB_CL_General_Receive_Info_t *ReceiveInfo, qapi_ZB_CL_General_Send_Info_t *SendInfo)
{
   qbool_t Ret_Val;

   if((SendInfo != NULL) && (ReceiveInfo != NULL))
   {
      /* Found the device, now determine if it is a short address or a
         group address (no extended addresses used at this layer). */
      SendInfo->DstAddrMode             = QAPI_ZB_ADDRESS_MODE_SHORT_ADDRESS_E;
      SendInfo->DstAddress.ShortAddress = ReceiveInfo->SrcNwkAddress;
      SendInfo->DstEndpoint             = ReceiveInfo->SrcEndpoint;
      SendInfo->SeqNum                  = GetNextSeqNum();

   }
   else
   {
      Ret_Val = false;
   }

   return(Ret_Val);
}

/**
   @brief Helper function to format the send information for a packet.

   @param DeviceIndex is the index of the device to be sent.
   @param SendInfo    is a pointer to where the send information will be
                      formatted upon successful return.

   @return true if the send info was formatted successfully, false otherwise.
*/
qbool_t Format_Send_Info_By_Device(uint32_t DeviceIndex, qapi_ZB_CL_General_Send_Info_t *SendInfo)
{
   qbool_t         Ret_Val;
   ZB_Device_ID_t *DeviceEntry;

   DeviceEntry = GetDeviceListEntry(DeviceIndex);
   if((SendInfo != NULL) && (DeviceEntry != NULL))
   {
      /* Found the device, now determine if it is a short address or a
         group address (no extended addresses used at this layer). */
      SendInfo->DstAddrMode = DeviceEntry->Type;
      SendInfo->DstAddress  = DeviceEntry->Address;
      SendInfo->DstEndpoint = DeviceEntry->Endpoint;
      SendInfo->SeqNum      = GetNextSeqNum();

      Ret_Val = true;
   }
   else
   {
      Ret_Val = false;
   }

   return(Ret_Val);
}

/**
   @brief Function to get the QCLI handle for the ZigBee demo.

   @return The QCLI handled used by the ZigBee demo.
*/
QCLI_Group_Handle_t GetZigBeeQCLIHandle(void)
{
   return(ZigBee_Demo_Context.QCLI_Handle);
}
