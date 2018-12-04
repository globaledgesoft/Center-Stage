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

#ifndef __QC_DRV_ZIGBEE_H
#define __QC_DRV_ZIGBEE_H

#include "qc_drv_main.h"

qapi_Status_t qc_drv_ZB_Initialize(qc_drv_context *qc_drv_ctx, qapi_ZB_Handle_t *ZB_Handle, qapi_ZB_Event_CB_t ZB_Event_CB, uint32_t CB_Param);
qapi_Status_t qc_drv_ZB_Register_Persist_Notify_CB(qc_drv_context *qc_drv_ctx, qapi_ZB_Handle_t ZB_Handle, qapi_ZB_Persist_Notify_CB_t ZB_Persist_Notify_CB, uint32_t CB_Param);
qapi_Status_t qc_drv_ZB_ZDP_Register_Callback(qc_drv_context *qc_drv_ctx, qapi_ZB_Handle_t ZB_Handle, qapi_ZB_ZDP_Event_CB_t ZDP_Event_CB, uint32_t CB_Param);
qapi_Status_t qc_drv_ZB_Get_Extended_Address(qc_drv_context *qc_drv_ctx, qapi_ZB_Handle_t ZB_Handle, uint64_t *Extended_Address);
qapi_Status_t qc_drv_ZB_Shutdown(qc_drv_context *qc_drv_ctx, qapi_ZB_Handle_t ZB_Handle);
qapi_Status_t qc_drv_ZB_Form(qc_drv_context *qc_drv_ctx, qapi_ZB_Handle_t ZB_Handle, const qapi_ZB_NetworkConfig_t *Config);
qapi_Status_t qc_drv_ZB_Join(qc_drv_context *qc_drv_ctx, qapi_ZB_Handle_t ZB_Handle, const qapi_ZB_Join_t *Config);
qapi_Status_t qc_drv_ZB_Leave(qc_drv_context *qc_drv_ctx, qapi_ZB_Handle_t ZB_Handle);
qapi_Status_t qc_drv_ZB_ZDP_Mgmt_Leave_Req(qc_drv_context *qc_drv_ctx, qapi_ZB_Handle_t ZB_Handle, uint16_t DstNwkAddr, uint64_t DeviceAddress, qbool_t RemoveChildren, qbool_t Rejoin);
qapi_Status_t qc_drv_ZB_Permit_Join(qc_drv_context *qc_drv_ctx, qapi_ZB_Handle_t ZB_Handle, uint8_t Duration);
qapi_Status_t qc_drv_ZB_ZDP_Bind_Req(qc_drv_context *qc_drv_ctx, qapi_ZB_Handle_t ZB_Handle, const qapi_ZB_ZDP_Bind_Req_t *RequestData);
qapi_Status_t qc_drv_ZB_ZDP_End_Device_Bind_Req(qc_drv_context *qc_drv_ctx, qapi_ZB_Handle_t ZB_Handle, const qapi_ZB_ZDP_End_Device_Bind_Req_t *RequestData);
qapi_Status_t qc_drv_ZB_APSME_Get_Request(qc_drv_context *qc_drv_ctx, qapi_ZB_Handle_t ZB_Handle, qapi_ZB_AIB_Attribute_ID_t AIBAttribute, uint8_t AIBAttributeIndex, uint16_t *AIBAttributeLength, void *AIBAttributeValue);
qapi_Status_t qc_drv_ZB_APSME_Set_Request(qc_drv_context *qc_drv_ctx, qapi_ZB_Handle_t ZB_Handle, qapi_ZB_AIB_Attribute_ID_t AIBAttribute, uint8_t AIBAttributeIndex, uint16_t *AIBAttributeLength, const void *AIBAttributeValue);
qapi_Status_t qc_drv_ZB_Set_Extended_Address(qc_drv_context *qc_drv_ctx, qapi_ZB_Handle_t ZB_Handle, uint64_t Extended_Address);
qapi_Status_t qc_drv_ZB_APS_Add_Endpoint(qc_drv_context *qc_drv_ctx, qapi_ZB_Handle_t ZB_Handle, qapi_ZB_APS_Add_Endpoint_t *RequestData);
qapi_Status_t qc_drv_ZB_APS_Remove_Endpoint(qc_drv_context *qc_drv_ctx, qapi_ZB_Handle_t ZB_Handle, uint8_t Endpoint);
qapi_Status_t qc_drv_ZB_CL_Read_Local_Attribute(qc_drv_context *qc_drv_ctx, qapi_ZB_Cluster_t Cluster, uint16_t AttrId, uint16_t *Length, uint8_t *Data);
qapi_Status_t qc_drv_ZB_CL_Write_Local_Attribute(qc_drv_context *qc_drv_ctx, qapi_ZB_Cluster_t Cluster, uint16_t AttrId, uint16_t Length, uint8_t *Data);
qapi_Status_t qc_drv_ZB_CL_Read_Attributes(qc_drv_context *qc_drv_ctx, qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, uint8_t AttrCount, const uint16_t *AttrIdList);
qapi_Status_t qc_drv_ZB_CL_Write_Attributes(qc_drv_context *qc_drv_ctx, qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, const uint8_t AttrCount, const qapi_ZB_CL_Write_Attr_Record_t *AttrStructuredList);
qapi_Status_t qc_drv_ZB_CL_Discover_Attributes(qc_drv_context *qc_drv_ctx, qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, const uint16_t StartAttrId, const uint8_t AttrIdCount);
qapi_Status_t qc_drv_ZB_CL_Configure_Reporting(qc_drv_context *qc_drv_ctx, qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, uint8_t ReportCount, const qapi_ZB_CL_Attr_Reporting_Config_Record_t *ReportRecordList);
qapi_Status_t qc_drv_ZB_CL_Read_Reporting(qc_drv_context *qc_drv_ctx, qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, uint8_t ReportCount, const qapi_ZB_CL_Attr_Record_t *ReportConfigList);
qapi_Status_t qc_drv_ZB_CL_OnOff_Send_On(qc_drv_context *qc_drv_ctx, qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, qbool_t RecallGlobalScene);
qapi_Status_t qc_drv_ZB_CL_OnOff_Send_Off(qc_drv_context *qc_drv_ctx, qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo);
qapi_Status_t qc_drv_ZB_CL_OnOff_Send_Toggle(qc_drv_context *qc_drv_ctx, qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo);
qapi_Status_t qc_drv_ZB_CL_LevelControl_Send_Move_To_Level(qc_drv_context *qc_drv_ctx, qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, uint8_t Level, uint16_t TransitionTime, qbool_t WithOnOff);
qapi_Status_t qc_drv_ZB_CL_LevelControl_Send_Move(qc_drv_context *qc_drv_ctx, qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, qbool_t MoveDown, uint8_t Rate, qbool_t WithOnOff);
qapi_Status_t qc_drv_ZB_CL_LevelControl_Send_Step(qc_drv_context *qc_drv_ctx, qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, qbool_t MoveDown, uint8_t StepSize, uint16_t TransitionTime, qbool_t WithOnOff);
qapi_Status_t qc_drv_ZB_CL_LevelControl_Send_Stop(qc_drv_context *qc_drv_ctx, qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo);

qapi_Status_t qc_drv_ZB_BDB_Get_Request(qc_drv_context *qc_drv_ctx, qapi_ZB_Handle_t ZB_Handle, qapi_ZB_BDB_Attribute_ID_t AttributeId, uint8_t AttributeIndex, uint16_t *AttributeLength, uint8_t *AttributeValue);
qapi_Status_t qc_drv_ZB_BDB_Set_Request(qc_drv_context *qc_drv_ctx, qapi_ZB_Handle_t ZB_Handle, qapi_ZB_BDB_Attribute_ID_t AttributeId, uint8_t AttributeIndex, uint16_t AttributeLength, const uint8_t *AttributeValue);
qapi_Status_t qc_drv_ZB_ZDP_Match_Desc_Req(qc_drv_context *qc_drv_ctx, qapi_ZB_Handle_t ZB_Handle, uint16_t DstNwkAddr, const qapi_ZB_ZDP_Match_Desc_Req_t *RequestData);
qapi_Status_t qc_drv_ZB_CL_Basic_Send_Reset_To_Factory(qc_drv_context *qc_drv_ctx, qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo);
qapi_Status_t qc_drv_ZB_CL_Basic_Server_Read_Attribute(qc_drv_context *qc_drv_ctx, qapi_ZB_Handle_t ZB_Handle, uint16_t AttrId, uint16_t *Length, uint8_t *Data);
qapi_Status_t qc_drv_ZB_CL_Basic_Server_Write_Attribute(qc_drv_context *qc_drv_ctx, qapi_ZB_Handle_t ZB_Handle, uint16_t AttrId, uint16_t Length, const uint8_t *Data);
qapi_Status_t qc_drv_ZB_CL_Identify_Send_Identify(qc_drv_context *qc_drv_ctx, qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, uint16_t IdentifyTime);
qapi_Status_t qc_drv_ZB_CL_Identify_Send_Identify_Query(qc_drv_context *qc_drv_ctx, qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo);

qapi_Status_t qc_drv_ZB_CL_Groups_Send_Add_Group(qc_drv_context *qc_drv_ctx, qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, uint16_t GroupId, const char *GroupName, qbool_t Identifying);
qapi_Status_t qc_drv_ZB_CL_Groups_Send_View_Group(qc_drv_context *qc_drv_ctx, qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, uint16_t GroupId);
qapi_Status_t qc_drv_ZB_CL_Groups_Send_Remove_Group(qc_drv_context *qc_drv_ctx, qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, uint16_t GroupId);
qapi_Status_t qc_drv_ZB_CL_Groups_Send_Remove_All_Groups(qc_drv_context *qc_drv_ctx, qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo);
qapi_Status_t qc_drv_ZB_CL_Groups_Send_Get_Group_Membership(qc_drv_context *qc_drv_ctx, qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, uint8_t GroupCount, const uint16_t *GroupList);

qapi_Status_t qc_drv_ZB_CL_Scenes_Send_Remove_Scene(qc_drv_context *qc_drv_ctx, qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, uint16_t GroupId, uint8_t SceneId);
qapi_Status_t qc_drv_ZB_CL_Scenes_Send_Remove_All_Scenes(qc_drv_context *qc_drv_ctx, qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, uint16_t GroupId);
qapi_Status_t qc_drv_ZB_CL_Scenes_Send_Store_Scene(qc_drv_context *qc_drv_ctx, qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, uint16_t GroupId, uint8_t SceneId);
qapi_Status_t qc_drv_ZB_CL_Scenes_Send_Recall_Scene(qc_drv_context *qc_drv_ctx, qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, uint16_t GroupId, uint8_t SceneId);
qapi_Status_t qc_drv_ZB_CL_Scenes_Send_Get_Scene_Membership(qc_drv_context *qc_drv_ctx, qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, uint16_t GroupId);
qapi_Status_t  qc_drv_ZB_CL_Scenes_Send_Copy_Scene(qc_drv_context *qc_drv_ctx, qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, const qapi_ZB_CL_Scenes_Copy_Scene_t *CopyScene);
qapi_Status_t qc_drv_ZB_CL_Scenes_Send_Add_Scene(qc_drv_context *qc_drv_ctx, qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, const qapi_ZB_CL_Scenes_Add_Scene_t *AddScene);
qapi_Status_t qc_drv_ZB_CL_Scenes_Send_View_Scene(qc_drv_context *qc_drv_ctx, qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, uint16_t GroupId, uint8_t SceneId, qbool_t IsEnhanced);

qapi_Status_t qc_drv_ZB_CL_Time_Server_Read_Attribute(qc_drv_context *qc_drv_ctx, qapi_ZB_Handle_t ZB_Handle, uint16_t AttrId, uint16_t *Length, uint8_t *Data);
qapi_Status_t qc_drv_ZB_CL_Time_Server_Write_Attribute(qc_drv_context *qc_drv_ctx, qapi_ZB_Handle_t ZB_Handle, uint16_t AttrId, uint16_t Length, const uint8_t *Data);

qapi_Status_t qc_drv_ZB_CL_Alarm_Send_Reset_Alarm(qc_drv_context *qc_drv_ctx, qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, uint16_t SourceClusterId, uint8_t AlarmCode);
qapi_Status_t qc_drv_ZB_CL_Alarm_Send_Reset_All_Alarms(qc_drv_context *qc_drv_ctx, qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo);
qapi_Status_t qc_drv_ZB_CL_Alarm_Send_Get_Alarm(qc_drv_context *qc_drv_ctx, qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo);
qapi_Status_t qc_drv_ZB_CL_Alarm_Send_Reset_Alarm_Log(qc_drv_context *qc_drv_ctx, qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo);
qapi_Status_t qc_drv_ZB_CL_Alarm_Send_Alarm(qc_drv_context *qc_drv_ctx, qapi_ZB_Cluster_t Cluster, qapi_ZB_Cluster_t SourceCluster, uint8_t AlarmCode);

qapi_Status_t qc_drv_ZB_CL_ColorControl_Send_Move_To_Hue(qc_drv_context *qc_drv_ctx, qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, uint16_t Hue, qapi_ZB_CL_ColorControl_Move_Mode_t Direction, uint16_t TransitionTime, qbool_t IsEnhanced);
qapi_Status_t qc_drv_ZB_CL_ColorControl_Send_Move_To_HueAndSaturation(qc_drv_context *qc_drv_ctx, qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, uint16_t Hue, uint8_t Saturation, uint16_t TransitionTime, qbool_t IsEnhanced);
qapi_Status_t qc_drv_ZB_CL_ColorControl_Send_Move_Hue(qc_drv_context *qc_drv_ctx, qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, qapi_ZB_CL_ColorControl_Move_Mode_t MoveMode, uint16_t Rate, qbool_t IsEnhanced);
qapi_Status_t qc_drv_ZB_CL_ColorControl_Send_Step_Hue(qc_drv_context *qc_drv_ctx, qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, qapi_ZB_CL_ColorControl_Move_Mode_t StepMode, uint16_t StepSize, uint16_t TransitionTime, qbool_t IsEnhanced);
qapi_Status_t qc_drv_ZB_CL_ColorControl_Send_Move_To_Saturation(qc_drv_context *qc_drv_ctx, qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, uint8_t Saturation, uint16_t TransitionTime);
qapi_Status_t qc_drv_ZB_CL_ColorControl_Send_Move_Saturation(qc_drv_context *qc_drv_ctx, qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, qapi_ZB_CL_ColorControl_Move_Mode_t MoveMode, uint8_t Rate);
qapi_Status_t qc_drv_ZB_CL_ColorControl_Send_Step_Saturation(qc_drv_context *qc_drv_ctx, qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, qapi_ZB_CL_ColorControl_Move_Mode_t StepMode, uint8_t StepSize, uint8_t TransitionTime);
qapi_Status_t qc_drv_ZB_CL_ColorControl_Send_Move_To_Color(qc_drv_context *qc_drv_ctx, qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, uint16_t ColorX, uint16_t ColorY, uint16_t TransitionTime);
qapi_Status_t qc_drv_ZB_CL_ColorControl_Send_Move_Color(qc_drv_context *qc_drv_ctx, qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, int16_t RateX, int16_t RateY);
qapi_Status_t qc_drv_ZB_CL_ColorControl_Send_Step_Color(qc_drv_context *qc_drv_ctx, qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, int16_t StepX, int16_t StepY, uint16_t TransitionTime);
qapi_Status_t qc_drv_ZB_CL_ColorControl_Send_Move_To_Color_Temp(qc_drv_context *qc_drv_ctx, qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, uint16_t ColorTempMireds, uint16_t TransitionTime);
qapi_Status_t qc_drv_ZB_CL_ColorControl_Send_Move_Color_Temp(qc_drv_context *qc_drv_ctx, qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, qapi_ZB_CL_ColorControl_Move_Mode_t MoveMode, uint16_t Rate, uint16_t Limit);
qapi_Status_t qc_drv_ZB_CL_ColorControl_Send_Step_Color_Temp(qc_drv_context *qc_drv_ctx, qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, const qapi_ZB_CL_ColorControl_Step_Color_Temp_t *StepColorTemp);
qapi_Status_t qc_drv_ZB_CL_ColorControl_Send_Stop_Move_Step(qc_drv_context *qc_drv_ctx, qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo);
qapi_Status_t qc_drv_ZB_CL_ColorControl_Send_Color_Loop_Set(qc_drv_context *qc_drv_ctx, qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_General_Send_Info_t *SendInfo, const qapi_ZB_CL_ColorControl_Color_Loop_Set_t *ColorLoopSet);

qapi_Status_t qc_drv_ZB_Get_Persistent_Data(qc_drv_context *qc_drv_ctx, qapi_ZB_Handle_t ZB_Handle, uint8_t *Buffer, uint32_t *Length);
qapi_Status_t qc_drv_ZB_Restore_Persistent_Data(qc_drv_context *qc_drv_ctx, qapi_ZB_Handle_t ZB_Handle, const uint8_t *Buffer, uint32_t Length);
qapi_Status_t qc_drv_ZB_NLME_Get_Request(qc_drv_context *qc_drv_ctx, qapi_ZB_Handle_t ZB_Handle, qapi_ZB_NIB_Attribute_ID_t AttributeId, uint8_t AttributeIndex, uint16_t *AttributeLength, uint8_t *AttributeValue);
qapi_Status_t qc_drv_ZB_NLME_Set_Request(qc_drv_context *qc_drv_ctx, qapi_ZB_Handle_t ZB_Handle, qapi_ZB_NIB_Attribute_ID_t AttributeId, uint8_t AttributeIndex, uint16_t AttributeLength, const uint8_t *AttributeValue);
qapi_Status_t qc_drv_ZB_CL_Touchlink_Start(qc_drv_context *qc_drv_ctx, qapi_ZB_Cluster_t Cluster, qapi_ZB_CL_Touchlink_Device_Type_t DeviceType, const uint8_t *PersistData, uint32_t PersistLength);
qapi_Status_t qc_drv_ZB_CL_Touchlink_Scan_Request(qc_drv_context *qc_drv_ctx, qapi_ZB_Cluster_t ClientCluster);
qapi_Status_t qc_drv_ZB_CL_Touchlink_Factory_Reset(qc_drv_context *qc_drv_ctx, qapi_ZB_Cluster_t ClientCluster);
qapi_Status_t qc_drv_ZB_CL_Destroy_Cluster(qc_drv_context *qc_drv_ctx, qapi_ZB_Cluster_t Cluster);
qapi_Status_t qc_drv_Persist_Initialize(qc_drv_context *qc_drv_ctx, qapi_Persist_Handle_t *Handle, char *Directory, char *NamePrefix, char *NameSuffix, uint8_t *Password, uint32_t PasswordSize);
void qc_drv_Persist_Delete(qc_drv_context *qc_drv_ctx, qapi_Persist_Handle_t Handle);
void qc_drv_Persist_Cleanup(qc_drv_context *qc_drv_ctx, qapi_Persist_Handle_t Handle);
#endif
