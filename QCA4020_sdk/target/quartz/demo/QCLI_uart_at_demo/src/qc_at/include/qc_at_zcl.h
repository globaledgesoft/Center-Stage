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

#ifndef __ZCL_DEMO_H__
#define __ZCL_DEMO_H__

#include "pal.h"
#include "qapi_zb.h"
#include "qapi_zb_cl.h"

#define ZCL_DEMO_IGNORE_CLUSTER_ID                                      (0xFFFF)

#define ZCL_DEMO_MAX_ATTRIBUTES_PER_CLUSTER                             (20)

typedef enum
{
   ZCL_DEMO_CLUSTERTYPE_UNKNOWN,
   ZCL_DEMO_CLUSTERTYPE_CLIENT,
   ZCL_DEMO_CLUSTERTYPE_SERVER
} ZCL_Demo_ClusterType_t;

/*
   Forward define the ZCL_Demo_Cluster_Info_t structure.
*/
typedef struct ZCL_Demo_Cluster_Info_s ZCL_Demo_Cluster_Info_t;

/**
   @brief Prototype for a function to initialize a cluster demo.

   @param ZigBee_QCLI_Handle is the parent QCLI handle for demo.

   @return true if the ZigBee demo was initialized successfully, false
           otherwise.
*/
typedef qbool_t (*ZCL_Cluster_Demo_Init_Func_t)(QCLI_Group_Handle_t ZigBee_QCLI_Handle);

/**
   @brief Prototype for a function to create a cluster.

   @param Endpoint is the endpoint the cluster will be part of.
   @param PrivData is a pointer to the private data allocated for the cluster
                   demo.  This will be initaialized to NULL before the create
                   function is called so can be ignored if the demo has no
                   private data.  If set to a non-NULL value, the data will be
                   freed when the cluster is removed.

   @return The handle for the newly created function or NULL if there was an
           error.
*/
typedef qapi_ZB_Cluster_t (*ZCL_Cluster_Demo_Create_Func_t)(uint8_t Endpoint, void **PrivData);

/**
   @brief Prototype for a function that is called when a cluster is removed.

   Provided to allow the cluster demo to cleanup any extra resources.

   @param Cluster_Info is the information for the cluster being removed.
*/
typedef void (*ZCL_Cluster_Cleanup_CB_t)(ZCL_Demo_Cluster_Info_t *ClusterInfo);

/*
   Structure represents the information for a cluster.
*/
typedef struct ZCL_Demo_Cluster_Info_s
{
   qapi_ZB_Cluster_t         Handle;      /** Handle of the cluster. */
   uint16_t                  ClusterID;   /** ID of the cluster. */
   uint8_t                   Endpoint;    /** endpoint used by the cluster. */
   ZCL_Demo_ClusterType_t    ClusterType; /** Indicates the type of cluster as either server or client. */
   const char               *ClusterName; /** Name of the cluster being added. */
   const char               *DeviceName;  /** Device Name for the cluster. */
   void                     *PrivData;    /** Private data for the cluster. */
   ZCL_Cluster_Cleanup_CB_t  Cleanup_CB;  /** Function called when cluster is removed. */
} ZCL_Demo_Cluster_Info_t;

/**
   @brief Registers the ZigBee cluster commands with QCLI.
*/
qbool_t Initialize_ZCL_Demo(QCLI_Group_Handle_t ZigBee_QCLI_Handle);

/**
   @brief Function to add a cluster entry to a cluster list.

   @param Cluster_Info is the information for the cluster to add.

   @return The ClusterIndex of the newly added cluster or a negative value if
           there was an error.
*/
int16_t ZB_Cluster_AddCluster(const ZCL_Demo_Cluster_Info_t *Cluster_Info);

/**
   @brief Called when the stack is shutdown to cleanup the cluster list.
*/
void ZB_Cluster_Cleanup(void);

/**
   @brief Gets the cluster handle for a specified index.

   @param ClusterIndex is the index of the cluster being requested.
   @param ClusterID    is the expected ID of the cluster being requested.  Set
                       to 0xFFFF to ignore.

   @return The info structure for the cluster or NULL if it was not found.
*/
ZCL_Demo_Cluster_Info_t *ZCL_FindClusterByIndex(uint16_t ClusterIndex, uint16_t ClusterID);

/**
   @brief Finds a cluster with a matching ID and endpoint.

   @param Endpoint    is the endpoint for the cluster to find.
   @param ClusterID   is the ID fo the cluster to find.
   @param ClusterType is the type of cluster (server or client).

   @return The handle for the requested cluster or NULL if it was not found.
*/
ZCL_Demo_Cluster_Info_t *ZCL_FindClusterByEndpoint(uint8_t Endpoint, uint16_t ClusterID, ZCL_Demo_ClusterType_t ClusterType);

/**
   @brief Finds a cluster with a matching ID and endpoint.

   @param Handle is the handle of the cluster to find.

   @return The handle for the requested cluster or NULL if it was not found.
*/
ZCL_Demo_Cluster_Info_t *ZCL_FindClusterByHandle(qapi_ZB_Cluster_t Handle);
#if 1
QCLI_Command_Status_t cmd_ZB_CL_Help(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t cmd_ZB_CL_ListClusterTypes(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t cmd_ZB_CL_ListEndpointTypes(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t cmd_ZB_CL_CreateEndpoint(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t cmd_ZB_CL_RemoveEndpoint(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t cmd_ZB_CL_ListClusters(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t cmd_ZB_CL_ReadLocalAttribute(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t cmd_ZB_CL_WriteLocalAttribute(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t cmd_ZB_CL_ReadAttribute(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t cmd_ZB_CL_WriteAttribute(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t cmd_ZB_CL_ConfigReport(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t cmd_ZB_CL_ReadReportConfig(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t cmd_ZB_CL_MatchDescriptor(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t cmd_ZB_CL_DiscoverAttributes(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);

QCLI_Command_Status_t cmd_ZCL_Basic_Help(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t cmd_ZCL_Basic_Reset(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t cmd_ZCL_Basic_Read(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t cmd_ZCL_Basic_Write(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);

QCLI_Command_Status_t cmd_ZCL_Identify_Help(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t cmd_ZCL_Identify_Identify(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t cmd_ZCL_Identify_IdentifyQuery(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);

QCLI_Command_Status_t cmd_ZCL_Groups_Help(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t cmd_ZCL_Groups_AddGroup(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t cmd_ZCL_Groups_ViewGroup(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t cmd_ZCL_Groups_GetGroupMembership(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t cmd_ZCL_Groups_RemoveGroup(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t cmd_ZCL_Groups_RemoveAllGroups(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);

QCLI_Command_Status_t cmd_ZCL_Scenes_Help(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t cmd_ZCL_Scenes_AddScene(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t cmd_ZCL_Scenes_ViewScene(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t cmd_ZCL_Scenes_RemoveScene(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t cmd_ZCL_Scenes_RemoveAllScenes(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t cmd_ZCL_Scenes_StoreScenes(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t cmd_ZCL_Scenes_RecallScenes(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t cmd_ZCL_Scenes_GetSceneMembership(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t cmd_ZCL_Scenes_CopyScene(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);

QCLI_Command_Status_t cmd_ZCL_OnOff_Help(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t cmd_ZCL_OnOff_On(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t cmd_ZCL_OnOff_Off(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t cmd_ZCL_OnOff_Toggle(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t cmd_ZCL_OnOff_SetSceneData(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);

QCLI_Command_Status_t cmd_ZCL_LevelControl_Help(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t cmd_ZCL_LevelControl_MoveToLevel(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t cmd_ZCL_LevelControl_Move(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t cmd_ZCL_LevelControl_Step(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t cmd_ZCL_LevelControl_Stop(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t cmd_ZCL_LevelControl_SetSceneData(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);

QCLI_Command_Status_t cmd_ZCL_Alarms_Help(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t cmd_ZCL_Alarms_ResetAlarm(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t cmd_ZCL_Alarms_ResetAllAlarms(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t cmd_ZCL_Alarms_GetAlarm(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t cmd_ZCL_Alarms_ResetAlarmLog(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t cmd_ZCL_Alarms_Alarm(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);

QCLI_Command_Status_t cmd_ZCL_Time_Help(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t cmd_ZCL_Time_Read(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t cmd_ZCL_Time_Write(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);

QCLI_Command_Status_t cmd_ZCL_Touchlink_Help(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t cmd_ZCL_Touchlink_Start(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t cmd_ZCL_Touchlink_Scan(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t cmd_ZCL_Touchlink_FactoryReset(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);

QCLI_Command_Status_t cmd_ZCL_ColorControl_Help(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t cmd_ZCL_ColorControl_MoveToHue(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t cmd_ZCL_ColorControl_MoveHue(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t cmd_ZCL_ColorControl_StepHue(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t cmd_ZCL_ColorControl_MoveToSaturation(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t cmd_ZCL_ColorControl_MoveSaturation(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t cmd_ZCL_ColorControl_StepSaturation(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t cmd_ZCL_ColorControl_MoveToHueAndSaturation(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t cmd_ZCL_ColorControl_MoveToColor(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t cmd_ZCL_ColorControl_MoveColor(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t cmd_ZCL_ColorControl_StepColor(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t cmd_ZCL_ColorControl_MoveToColorTemp(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t cmd_ZCL_ColorControl_MoveColorTemp(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t cmd_ZCL_ColorControl_StepColorTemp(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t cmd_ZCL_ColorControl_StopMoveStep(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t cmd_ZCL_ColorControl_ColorLoopSet(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t cmd_ZCL_ColorControl_SetSceneData(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);

#endif
#endif

