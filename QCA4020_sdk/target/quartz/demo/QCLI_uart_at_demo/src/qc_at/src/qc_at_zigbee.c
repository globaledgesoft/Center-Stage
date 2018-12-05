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

/*-------------------------------------------------------------------------
 * Include Files
 *-----------------------------------------------------------------------*/
#include "qc_at_zigbee.h"
#include "qc_at_zcl.h"
#include "qosa_util.h"

QCLI_Group_Handle_t qc_at_zb_group;
extern QCLI_Context_t QCLI_Context;

QCLI_Command_Status_t qc_at_zb_Help(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
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

QCLI_Command_Status_t qc_at_zb_Service(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
		int ret = QCLI_STATUS_SUCCESS_E;

		if (Parameter_Count < 1)
		{
				return QCLI_STATUS_USAGE_E;
		}

		if (Parameter_List[0].Integer_Is_Valid)
		{
				return QCLI_STATUS_USAGE_E;
		}

		if (strcmp(Parameter_List[0].String_Value, "start") == 0)
		{
				ret = qc_api_cmd_ZB_Initialize(Parameter_Count - 1, Parameter_List + 1);
				if (0 == ret)
						LOG_AT_OK();
				else
						LOG_AT_ERROR();
				return ret;
		}
		else if (strcmp(Parameter_List[0].String_Value, "stop") == 0)
		{
				ret = qc_api_cmd_ZB_Shutdown(Parameter_Count - 1, Parameter_List + 1);
				if (0 == ret)
						LOG_AT_OK();
				else
						LOG_AT_ERROR();
				return ret;
		}
		else
		{
				return QCLI_STATUS_USAGE_E;
		}

		return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t qc_at_zb_Device(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
		int ret = QCLI_STATUS_SUCCESS_E;

		if (Parameter_Count < 1)
		{
				return QCLI_STATUS_USAGE_E;
		}

		if (Parameter_List[0].Integer_Is_Valid)
		{
				return QCLI_STATUS_USAGE_E;
		}

		if (strcmp(Parameter_List[0].String_Value, "add") == 0)
		{
				ret =  qc_api_cmd_ZB_AddDevice(Parameter_Count - 1, Parameter_List + 1);
				if (0 == ret)
						LOG_AT_OK();
				else
						LOG_AT_ERROR();
				return ret;
		}
		else if (strcmp(Parameter_List[0].String_Value, "delete") == 0)
		{
				ret = qc_api_cmd_ZB_RemoveDevice(Parameter_Count - 1, Parameter_List + 1);
				if (0 == ret)
						LOG_AT_OK();
				else
						LOG_AT_ERROR();
				return ret;
		}
		else if (strcmp(Parameter_List[0].String_Value, "show") == 0)
		{
				ret = qc_api_cmd_ZB_ShowDeviceList(Parameter_Count - 1, Parameter_List + 1);
				if (0 == ret)
						LOG_AT_OK();
				else
						LOG_AT_ERROR();
				return ret;
		}
		else
		{
				return QCLI_STATUS_USAGE_E;
		}
}

QCLI_Command_Status_t qc_at_zb_Form(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
		int ret = QCLI_STATUS_SUCCESS_E;

		ret = qc_api_cmd_ZB_Form(Parameter_Count, Parameter_List);
		if (0 == ret)
				LOG_AT_OK();
		else
				LOG_AT_ERROR();
		return ret;
}

QCLI_Command_Status_t qc_at_zb_Join(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
		int ret = QCLI_STATUS_SUCCESS_E;

		ret = qc_api_ZB_Join(Parameter_Count, Parameter_List);
		if (0 == ret)
				LOG_AT_OK();
		else
				LOG_AT_ERROR();
		return ret;
}

QCLI_Command_Status_t qc_at_zb_Leave(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
		int ret = QCLI_STATUS_SUCCESS_E;

		if (Parameter_Count < 1)
		{
				return QCLI_STATUS_USAGE_E;
		}

		if (Parameter_List[0].Integer_Is_Valid)
		{
				return QCLI_STATUS_USAGE_E;
		}

		if((Parameter_Count >= 1) && (strcmp(Parameter_List[0].String_Value, "Leave") == 0))
		{
				ret = qc_api_cmd_ZB_Leave(Parameter_Count - 1 , Parameter_List + 1);
				if (0 == ret)
						LOG_AT_OK();
				else
						LOG_AT_ERROR();
				return ret;
		}
		else if( (Parameter_Count >= 4) && (strcmp(Parameter_List[0].String_Value, "LeaveRequest") == 0))
		{
				ret = qc_api_cmd_ZB_LeaveReq(Parameter_Count - 1, Parameter_List + 1);
				if (0 == ret)
						LOG_AT_OK();
				else
						LOG_AT_ERROR();
				return ret;
		}
		else
		{
				return QCLI_STATUS_USAGE_E;
		}
}

QCLI_Command_Status_t qc_at_zb_PermitJoin(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
		int ret = QCLI_STATUS_SUCCESS_E;

		ret = qc_api_ZB_PermitJoin(Parameter_Count, Parameter_List);
		if (0 == ret)
				LOG_AT_OK();
		else
				LOG_AT_ERROR();
		return ret;
}

QCLI_Command_Status_t qc_at_zb_Bind(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
		int ret = QCLI_STATUS_SUCCESS_E;

		if (Parameter_Count < 1)
		{
				return QCLI_STATUS_USAGE_E;
		}

		if (Parameter_List[0].Integer_Is_Valid)
		{
				return QCLI_STATUS_USAGE_E;
		}

		if (strcmp(Parameter_List[0].String_Value, "bind") == 0)
		{
				ret = qc_api_cmd_ZB_BindRequest(Parameter_Count - 1, Parameter_List + 1);
				if (0 == ret)
						LOG_AT_OK();
				else
						LOG_AT_ERROR();
				return ret;
		}
		else if (strcmp(Parameter_List[0].String_Value, "unbind") == 0)
		{
				ret = qc_api_cmd_ZB_EndBind(Parameter_Count - 1, Parameter_List + 1);
				if (0 == ret)
						LOG_AT_OK();
				else
						LOG_AT_ERROR();
				return ret;
		}
		else
		{
				return QCLI_STATUS_USAGE_E;
		}
}

QCLI_Command_Status_t qc_at_zb_Attr(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
		int ret = QCLI_STATUS_SUCCESS_E;

		if (Parameter_Count < 1)
		{
				return QCLI_STATUS_USAGE_E;
		}

		if (Parameter_List[0].Integer_Is_Valid)
		{
				return QCLI_STATUS_USAGE_E;
		}

		if (strcmp(Parameter_List[0].String_Value, "GetNIB") == 0)
		{
				ret = qc_api_cmd_ZB_GetNIB(Parameter_Count - 1, Parameter_List + 1);
				if (0 == ret)
						LOG_AT_OK();
				else
						LOG_AT_ERROR();
				return ret;
		}
		else if (strcmp(Parameter_List[0].String_Value, "SetNIB") == 0)
		{
				ret = qc_api_cmd_ZB_SetNIB(Parameter_Count - 1, Parameter_List + 1);
				if (0 == ret)
						LOG_AT_OK();
				else
						LOG_AT_ERROR();
				return ret;
		}
		else if (strcmp(Parameter_List[0].String_Value, "GetAIB") == 0)
		{
				ret = qc_api_cmd_ZB_GetAIB(Parameter_Count - 1, Parameter_List + 1);
				if (0 == ret)
						LOG_AT_OK();
				else
						LOG_AT_ERROR();
				return ret;
		}
		else if (strcmp(Parameter_List[0].String_Value, "SetAIB") == 0)
		{
				ret = qc_api_cmd_ZB_SetAIB(Parameter_Count - 1, Parameter_List + 1);
				if (0 == ret)
						LOG_AT_OK();
				else
						LOG_AT_ERROR();
				return ret;
		}
		else if (strcmp(Parameter_List[0].String_Value, "SetBIB") == 0)
		{
				ret = qc_api_cmd_ZB_SetBIB(Parameter_Count - 1, Parameter_List + 1);
				if (0 == ret)
						LOG_AT_OK();
				else
						LOG_AT_ERROR();
				return ret;
		}
		else if (strcmp(Parameter_List[0].String_Value, "GetBIB") == 0)
		{
				ret = qc_api_cmd_ZB_GetBIB(Parameter_Count - 1, Parameter_List + 1);
				if (0 == ret)
						LOG_AT_OK();
				else
						LOG_AT_ERROR();
				return ret;
		}
		else
		{
				return QCLI_STATUS_USAGE_E;
		}
}

QCLI_Command_Status_t qc_at_zb_ExtAddr(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)

{
		int ret = QCLI_STATUS_SUCCESS_E;

		if (Parameter_Count < 1)
		{
				ret = qc_api_cmd_ZB_GetAddresses(Parameter_Count, Parameter_List);
				if (0 == ret)
						LOG_AT_OK();
				else
						LOG_AT_ERROR();
				return ret;
		}
		else
		{
				ret = qc_api_cmd_ZB_SetExtAddress(Parameter_Count, Parameter_List);
				if (0 == ret)
						LOG_AT_OK();
				else
						LOG_AT_ERROR();
				return ret;
		}
}

QCLI_Command_Status_t qc_at_zb_ClearPersist(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
		int ret = QCLI_STATUS_SUCCESS_E;

		ret = qc_api_ZB_ClearPersist(Parameter_Count, Parameter_List);
		if (0 == ret)
				LOG_AT_OK();
		else
				LOG_AT_ERROR();
		return ret;
}

QCLI_Command_Status_t qc_at_zb_cluster_Endpoints(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
		int ret = QCLI_STATUS_SUCCESS_E;

		if ((Parameter_Count >= 1) && (APP_STRCMP(Parameter_List[0].String_Value,"ListClusterTypes") == 0)) {
				ret = cmd_ZB_CL_ListClusterTypes(Parameter_Count - 1, Parameter_List + 1);
		}else if((Parameter_Count >= 1) && (APP_STRCMP(Parameter_List[0].String_Value,"ListEndpointtypes") == 0)) {
				LOG_INFO("ListEndPointTypes\n\r");
				ret = cmd_ZB_CL_ListEndpointTypes(Parameter_Count - 1, Parameter_List + 1);
		}else if((Parameter_Count >= 2) && (APP_STRCMP(Parameter_List[0].String_Value,"CreateEndpoint") == 0)) {
				ret = cmd_ZB_CL_CreateEndpoint(Parameter_Count - 1, Parameter_List + 1);
		}else if((Parameter_Count >= 1) && (APP_STRCMP(Parameter_List[0].String_Value,"RemoveEndpoint") == 0)) {
				ret = cmd_ZB_CL_RemoveEndpoint(Parameter_Count - 1, Parameter_List + 1);
		}else if((Parameter_Count >= 1) && (APP_STRCMP(Parameter_List[0].String_Value,"ListClusters") == 0)) {
				ret = cmd_ZB_CL_ListClusters(Parameter_Count - 1, Parameter_List + 1);
		}else {
				ret = QCLI_STATUS_USAGE_E;
		}
		if ((ret == QCLI_STATUS_SUCCESS_E) || (ret == QCLI_STATUS_USAGE_E))
				LOG_AT_OK();
		else
				LOG_AT_ERROR();

		return ret;
}

QCLI_Command_Status_t qc_at_zb_cluster_attributes(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
		int ret = QCLI_STATUS_SUCCESS_E;
		if ((Parameter_Count >= 3) && (APP_STRCMP(Parameter_List[0].String_Value,"ReadLocAttr") == 0)) {
				ret = cmd_ZB_CL_ReadLocalAttribute(Parameter_Count - 1, Parameter_List + 1);
		}else if((Parameter_Count >= 4) && (APP_STRCMP(Parameter_List[0].String_Value,"WriteLocAttr") == 0)) {
				ret = cmd_ZB_CL_WriteLocalAttribute(Parameter_Count - 1, Parameter_List + 1);
		}else if((Parameter_Count >= 3) && (APP_STRCMP(Parameter_List[0].String_Value,"ReadAttr") == 0)) {
				ret = cmd_ZB_CL_ReadAttribute(Parameter_Count - 1, Parameter_List + 1);
		}else if((Parameter_Count >= 6) && (APP_STRCMP(Parameter_List[0].String_Value,"WriteAttr") == 0)) {
				ret = cmd_ZB_CL_WriteAttribute(Parameter_Count - 1, Parameter_List + 1);
		}else if((Parameter_Count >= 2) && (APP_STRCMP(Parameter_List[0].String_Value,"DiscAttr") == 0)) {
				ret = cmd_ZB_CL_DiscoverAttributes(Parameter_Count - 1, Parameter_List + 1);
		}else {
				ret = QCLI_STATUS_USAGE_E;
		}
		if ((ret == QCLI_STATUS_SUCCESS_E) || (ret == QCLI_STATUS_USAGE_E))
				LOG_AT_OK();
		else
				LOG_AT_ERROR();

		return ret;
}

QCLI_Command_Status_t qc_at_zb_cluster_configreport(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
		int ret = QCLI_STATUS_SUCCESS_E;

		if ((Parameter_Count >= 3) && (APP_STRCMP(Parameter_List[0].String_Value,"ReadReportConfig") == 0)) {
				ret = cmd_ZB_CL_ReadReportConfig(Parameter_Count - 1, Parameter_List + 1);
		}else if((Parameter_Count >= 7) && (APP_STRCMP(Parameter_List[0].String_Value,"ConfigReport") == 0)) {
				ret = cmd_ZB_CL_ConfigReport(Parameter_Count - 1, Parameter_List + 1);
		}else {
				ret = QCLI_STATUS_USAGE_E;
		}
		if ((ret == QCLI_STATUS_SUCCESS_E) || (ret == QCLI_STATUS_USAGE_E))
				LOG_AT_OK();
		else
				LOG_AT_ERROR();
		return ret;
}

QCLI_Command_Status_t qc_at_zb_basic(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
		int ret = QCLI_STATUS_SUCCESS_E;
		if((Parameter_Count >= 3) && (APP_STRCMP(Parameter_List[0].String_Value,"Reset") == 0)) {
				ret = cmd_ZCL_Basic_Reset(Parameter_Count - 1, Parameter_List + 1);
		}else if((Parameter_Count >= 2) && (APP_STRCMP(Parameter_List[0].String_Value,"Read") == 0)) {
				ret = cmd_ZCL_Basic_Read(Parameter_Count -1, Parameter_List + 1);
		} else if((Parameter_Count >= 3) && (APP_STRCMP(Parameter_List[0].String_Value,"Write") == 0)) {
				ret = cmd_ZCL_Basic_Write(Parameter_Count -1, Parameter_List + 1);
		} else {
				ret = QCLI_STATUS_USAGE_E;
		}

		if ((ret == QCLI_STATUS_SUCCESS_E) || (ret == QCLI_STATUS_USAGE_E))
				LOG_AT_OK();
		else
				LOG_AT_ERROR();
		return ret;
}

QCLI_Command_Status_t qc_at_zb_identify(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
		int ret = QCLI_STATUS_SUCCESS_E;

		if((Parameter_Count >= 4) && (APP_STRCMP(Parameter_List[0].String_Value,"Ident") == 0)) {
				ret = cmd_ZCL_Identify_Identify(Parameter_Count -1, Parameter_List + 1);
		} else if((Parameter_Count >= 3) && (APP_STRCMP(Parameter_List[0].String_Value,"IdentQuery") == 0)) {
				ret = cmd_ZCL_Identify_IdentifyQuery(Parameter_Count -1, Parameter_List + 1);
		} else {
				ret = QCLI_STATUS_USAGE_E;
		}

		if ((ret == QCLI_STATUS_SUCCESS_E) || (ret == QCLI_STATUS_USAGE_E))
				LOG_AT_OK();
		else
				LOG_AT_ERROR();
		return ret;
}

QCLI_Command_Status_t qc_at_zb_group_info(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
		int ret = QCLI_STATUS_SUCCESS_E;
		if((Parameter_Count == 6) && (APP_STRCMP(Parameter_List[0].String_Value,"add") == 0)) {
				ret = cmd_ZCL_Groups_AddGroup(Parameter_Count -1, Parameter_List + 1);
		}else   if((Parameter_Count == 4) && (APP_STRCMP(Parameter_List[0].String_Value,"view") == 0)) {
				ret = cmd_ZCL_Groups_ViewGroup(Parameter_Count -1, Parameter_List + 1);
		}else   if((Parameter_Count == 4) && (APP_STRCMP(Parameter_List[0].String_Value,"remove") == 0)) {
				ret = cmd_ZCL_Groups_RemoveGroup(Parameter_Count -1, Parameter_List + 1);
		} else {
				ret = QCLI_STATUS_USAGE_E;
		}
		if ((ret == QCLI_STATUS_SUCCESS_E) || (ret == QCLI_STATUS_USAGE_E))
				LOG_AT_OK();
		else
				LOG_AT_ERROR();
		return ret;
}

QCLI_Command_Status_t qc_at_zb_group_ext(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
		int ret = QCLI_STATUS_SUCCESS_E;

		if((Parameter_Count >= 4) && (APP_STRCMP(Parameter_List[0].String_Value,"GroupMem") == 0)) {
				ret = cmd_ZCL_Groups_GetGroupMembership(Parameter_Count -1, Parameter_List + 1);
		}else if((Parameter_Count >= 3) && (APP_STRCMP(Parameter_List[0].String_Value,"RemAll") == 0)) {
				ret = cmd_ZCL_Groups_RemoveAllGroups(Parameter_Count -1, Parameter_List + 1);
		} else {
				ret = QCLI_STATUS_USAGE_E;
		}
		if ((ret == QCLI_STATUS_SUCCESS_E) || (ret == QCLI_STATUS_USAGE_E))
				LOG_AT_OK();
		else
				LOG_AT_ERROR();
		return ret;
}
QCLI_Command_Status_t qc_at_zb_scene(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
		int ret = QCLI_STATUS_SUCCESS_E;
		if((Parameter_Count >= 8) && (APP_STRCMP(Parameter_List[0].String_Value,"add") == 0)) {
				ret = cmd_ZCL_Scenes_AddScene(Parameter_Count -1, Parameter_List + 1);
		}else if((Parameter_Count >= 6) && (APP_STRCMP(Parameter_List[0].String_Value,"view") == 0)) {
				ret = cmd_ZCL_Scenes_ViewScene(Parameter_Count -1, Parameter_List + 1);
		}else if((Parameter_Count >= 5) && (APP_STRCMP(Parameter_List[0].String_Value,"remove") == 0)) {
				ret = cmd_ZCL_Scenes_RemoveScene(Parameter_Count -1, Parameter_List + 1);
		}else if((Parameter_Count >= 4) && (APP_STRCMP(Parameter_List[0].String_Value,"store") == 0)) {
				ret = cmd_ZCL_Scenes_StoreScenes(Parameter_Count -1, Parameter_List + 1);

		} else {
				ret = QCLI_STATUS_USAGE_E;
		}
		if ((ret == QCLI_STATUS_SUCCESS_E) || (ret == QCLI_STATUS_USAGE_E))
				LOG_AT_OK();
		else
				LOG_AT_ERROR();
		return ret;
}

QCLI_Command_Status_t qc_at_zb_scene_ext(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
		int ret = QCLI_STATUS_SUCCESS_E;

		if((Parameter_Count == 4) && (APP_STRCMP(Parameter_List[0].String_Value,"RemAllScenes") == 0)) {
				ret = cmd_ZCL_Scenes_RemoveAllScenes(Parameter_Count -1, Parameter_List + 1);
		}else if((Parameter_Count == 5) && (APP_STRCMP(Parameter_List[0].String_Value,"RecallScenes") == 0)) {
				ret = cmd_ZCL_Scenes_RecallScenes(Parameter_Count -1, Parameter_List + 1);
		}else if((Parameter_Count == 4) && (APP_STRCMP(Parameter_List[0].String_Value,"GetSceneMem") == 0)) {
				ret = cmd_ZCL_Scenes_GetSceneMembership(Parameter_Count -1, Parameter_List + 1);
		}else if((Parameter_Count == 8) && (APP_STRCMP(Parameter_List[0].String_Value,"CopyScene") == 0)) {
				ret = cmd_ZCL_Scenes_CopyScene(Parameter_Count -1, Parameter_List + 1);
		} else {
				ret = QCLI_STATUS_USAGE_E;
		}

		if ((ret == QCLI_STATUS_SUCCESS_E) || (ret == QCLI_STATUS_USAGE_E))
				LOG_AT_OK();
		else
				LOG_AT_ERROR();
		return ret;
}

QCLI_Command_Status_t qc_at_zb_onoff(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
		int ret = QCLI_STATUS_SUCCESS_E;
		if((Parameter_Count == 3) && (APP_STRCMP(Parameter_List[0].String_Value,"on") == 0)) {
				ret = cmd_ZCL_OnOff_On(Parameter_Count -1, Parameter_List + 1);
		}else if((Parameter_Count == 3) && (APP_STRCMP(Parameter_List[0].String_Value,"off") == 0)) {
				ret = cmd_ZCL_OnOff_Off(Parameter_Count -1, Parameter_List + 1);
		}else if((Parameter_Count == 3) && (APP_STRCMP(Parameter_List[0].String_Value,"toggle") == 0)) {
				ret = cmd_ZCL_OnOff_Toggle(Parameter_Count -1, Parameter_List + 1);
		}else if((Parameter_Count == 2) && (APP_STRCMP(Parameter_List[0].String_Value,"SetSceneData") == 0)) {
				ret = cmd_ZCL_OnOff_SetSceneData(Parameter_Count -1, Parameter_List + 1);
		} else {
				ret = QCLI_STATUS_USAGE_E;
		}
		if ((ret == QCLI_STATUS_SUCCESS_E) || (ret == QCLI_STATUS_USAGE_E))
				LOG_AT_OK();
		else
				LOG_AT_ERROR();
		return ret;
}

QCLI_Command_Status_t qc_at_zb_level_control(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
		int ret = QCLI_STATUS_SUCCESS_E;

		if((Parameter_Count == 6) && (APP_STRCMP(Parameter_List[0].String_Value,"MoveToLevel") == 0)) {
				ret = cmd_ZCL_LevelControl_MoveToLevel(Parameter_Count -1, Parameter_List + 1);
		} else if((Parameter_Count == 6) && (APP_STRCMP(Parameter_List[0].String_Value,"Move") == 0)) {
				ret = cmd_ZCL_LevelControl_Move(Parameter_Count -1, Parameter_List + 1);
		} else if((Parameter_Count == 7) && (APP_STRCMP(Parameter_List[0].String_Value,"Step") == 0)) {
				ret = cmd_ZCL_LevelControl_Step(Parameter_Count -1, Parameter_List + 1);
		} else if((Parameter_Count == 3) && (APP_STRCMP(Parameter_List[0].String_Value,"Stop") == 0)) {
				ret = cmd_ZCL_LevelControl_Stop(Parameter_Count -1, Parameter_List + 1);
		} else {
				ret = QCLI_STATUS_USAGE_E;
		}
		if ((ret == QCLI_STATUS_SUCCESS_E) || (ret == QCLI_STATUS_USAGE_E))
				LOG_AT_OK();
		else
				LOG_AT_ERROR();
		return ret;
}
QCLI_Command_Status_t qc_at_zb_level_control_scene_data(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
		int ret = QCLI_STATUS_SUCCESS_E;

		if(Parameter_Count >= 1) {
				ret =cmd_ZCL_LevelControl_SetSceneData(Parameter_Count, Parameter_List);
		} else {
				ret = QCLI_STATUS_USAGE_E;
		}
		if ((ret == QCLI_STATUS_SUCCESS_E) || (ret == QCLI_STATUS_USAGE_E))
				LOG_AT_OK();
		else
				LOG_AT_ERROR();
		return ret;
}

QCLI_Command_Status_t qc_at_zb_alarm(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
		int ret = QCLI_STATUS_SUCCESS_E;
		if((Parameter_Count == 5) && (APP_STRCMP(Parameter_List[0].String_Value,"ResetAlarm") == 0)) {
				ret = cmd_ZCL_Alarms_ResetAlarm(Parameter_Count -1, Parameter_List + 1);
		} else if((Parameter_Count == 3) && (APP_STRCMP(Parameter_List[0].String_Value,"ResetAllAlarms") == 0)) {
				ret = cmd_ZCL_Alarms_ResetAllAlarms(Parameter_Count -1, Parameter_List + 1);
		} else if((Parameter_Count == 3) && (APP_STRCMP(Parameter_List[0].String_Value,"GetAlarm") == 0)) {
				ret = cmd_ZCL_Alarms_GetAlarm(Parameter_Count -1, Parameter_List + 1);
		} else if((Parameter_Count == 3) && (APP_STRCMP(Parameter_List[0].String_Value,"ResetAlarmLog") == 0)) {
				ret = cmd_ZCL_Alarms_ResetAlarmLog(Parameter_Count -1, Parameter_List + 1);
		} else if((Parameter_Count == 4) && (APP_STRCMP(Parameter_List[0].String_Value,"Alarm") == 0)) {
				ret = cmd_ZCL_Alarms_Alarm(Parameter_Count -1, Parameter_List + 1);
		} else {
				ret = QCLI_STATUS_USAGE_E;
		}
		if ((ret == QCLI_STATUS_SUCCESS_E) || (ret == QCLI_STATUS_USAGE_E))
				LOG_AT_OK();
		else
				LOG_AT_ERROR();
		return ret;
}

QCLI_Command_Status_t qc_at_zb_time(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
		int ret = QCLI_STATUS_SUCCESS_E;
		if((Parameter_Count == 2) && (APP_STRCMP(Parameter_List[0].String_Value,"Read") == 0)) {
				ret = cmd_ZCL_Time_Read(Parameter_Count -1, Parameter_List + 1);
		}else if((Parameter_Count == 3) && (APP_STRCMP(Parameter_List[0].String_Value,"Write") == 0)) {
				ret = cmd_ZCL_Time_Write(Parameter_Count -1, Parameter_List + 1);
		} else {
				ret = QCLI_STATUS_USAGE_E;
		}
		if ((ret == QCLI_STATUS_SUCCESS_E) || (ret == QCLI_STATUS_USAGE_E))
				LOG_AT_OK();
		else
				LOG_AT_ERROR();
		return ret;

}

QCLI_Command_Status_t qc_at_zb_touchlink(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
		int ret = QCLI_STATUS_SUCCESS_E;
		if((Parameter_Count == 3) && (APP_STRCMP(Parameter_List[0].String_Value,"Start") == 0)) {
				ret = cmd_ZCL_Touchlink_Start(Parameter_Count -1, Parameter_List + 1);
		} else  if((Parameter_Count == 2) && (APP_STRCMP(Parameter_List[0].String_Value,"Scan") == 0)) {
				ret = cmd_ZCL_Touchlink_Scan(Parameter_Count -1, Parameter_List + 1);
		} else  if((Parameter_Count == 2) && (APP_STRCMP(Parameter_List[0].String_Value,"FactoryReset") == 0)) {
				ret = cmd_ZCL_Touchlink_FactoryReset(Parameter_Count -1, Parameter_List + 1);
		} else {
				ret = QCLI_STATUS_USAGE_E;
		}
		if ((ret == QCLI_STATUS_SUCCESS_E) || (ret == QCLI_STATUS_USAGE_E))
				LOG_AT_OK();
		else
				LOG_AT_ERROR();
		return ret;
}

QCLI_Command_Status_t qc_at_zb_colorcontro_Hue(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
		int ret = QCLI_STATUS_SUCCESS_E;

		if((Parameter_Count == 6) && (APP_STRCMP(Parameter_List[0].String_Value,"MoveToHue") == 0)) {
				ret = cmd_ZCL_ColorControl_MoveToHue(Parameter_Count -1, Parameter_List + 1);
		} else if((Parameter_Count == 5) && (APP_STRCMP(Parameter_List[0].String_Value,"MoveHue") == 0)) {
				ret = cmd_ZCL_ColorControl_MoveHue(Parameter_Count -1, Parameter_List + 1);
		} else if((Parameter_Count == 6) && (APP_STRCMP(Parameter_List[0].String_Value,"StepHue") == 0)) {
				ret = cmd_ZCL_ColorControl_StepHue(Parameter_Count -1, Parameter_List + 1);
		} else {
				ret = QCLI_STATUS_USAGE_E;
		}
		if ((ret == QCLI_STATUS_SUCCESS_E) || (ret == QCLI_STATUS_USAGE_E))
				LOG_AT_OK();
		else
				LOG_AT_ERROR();
		return ret;
}

QCLI_Command_Status_t qc_at_zb_colorcontro_Sat(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
		int ret = QCLI_STATUS_SUCCESS_E;
		if((Parameter_Count == 5) && (APP_STRCMP(Parameter_List[0].String_Value,"MoveToSat") == 0)) {
				ret = cmd_ZCL_ColorControl_MoveToSaturation(Parameter_Count -1, Parameter_List + 1);
		} else if((Parameter_Count == 5) && (APP_STRCMP(Parameter_List[0].String_Value,"MoveSat") == 0)) {
				ret = cmd_ZCL_ColorControl_MoveSaturation(Parameter_Count -1, Parameter_List + 1);
		} else if((Parameter_Count == 6) && (APP_STRCMP(Parameter_List[0].String_Value,"StepSat") == 0)) {
				ret = cmd_ZCL_ColorControl_StepSaturation(Parameter_Count -1, Parameter_List + 1);
		} else if((Parameter_Count == 6) && (APP_STRCMP(Parameter_List[0].String_Value,"MoveToHueSat") == 0)) {
				ret = cmd_ZCL_ColorControl_MoveToHueAndSaturation(Parameter_Count -1, Parameter_List + 1);
		} else {
				ret = QCLI_STATUS_USAGE_E;
		}
		if ((ret == QCLI_STATUS_SUCCESS_E) || (ret == QCLI_STATUS_USAGE_E))
				LOG_AT_OK();
		else
				LOG_AT_ERROR();
		return ret;
}

QCLI_Command_Status_t qc_at_zb_colorcontro_Move(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
		int ret = QCLI_STATUS_SUCCESS_E;
		if((Parameter_Count == 6) && (APP_STRCMP(Parameter_List[0].String_Value,"MoveToCol") == 0)) {
				ret = cmd_ZCL_ColorControl_MoveToColor(Parameter_Count -1, Parameter_List + 1);
		} else if((Parameter_Count == 5) && (APP_STRCMP(Parameter_List[0].String_Value,"MoveCol") == 0)) {
				ret = cmd_ZCL_ColorControl_MoveColor(Parameter_Count -1, Parameter_List + 1);
		} else if((Parameter_Count == 6) && (APP_STRCMP(Parameter_List[0].String_Value,"StepCol") == 0)) {
				ret = cmd_ZCL_ColorControl_StepColor(Parameter_Count -1, Parameter_List + 1);
		} else if((Parameter_Count == 4) && (APP_STRCMP(Parameter_List[0].String_Value,"MoveToColTemp") == 0)) {
				ret = cmd_ZCL_ColorControl_MoveToColorTemp(Parameter_Count -1, Parameter_List + 1);
		} else {
				ret = QCLI_STATUS_USAGE_E;
		}
		if ((ret == QCLI_STATUS_SUCCESS_E) || (ret == QCLI_STATUS_USAGE_E))
				LOG_AT_OK();
		else
				LOG_AT_ERROR();
		return ret;
}

QCLI_Command_Status_t qc_at_zb_colorcontro_Ext(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
		int ret = QCLI_STATUS_SUCCESS_E;
		if((Parameter_Count == 8) && (APP_STRCMP(Parameter_List[0].String_Value,"ColorLoopSet") == 0)) {
				ret = cmd_ZCL_ColorControl_ColorLoopSet(Parameter_Count -1, Parameter_List + 1);
		} else if((Parameter_Count == 3) && (APP_STRCMP(Parameter_List[0].String_Value,"StopMoveStep") == 0)) {
				ret = cmd_ZCL_ColorControl_StopMoveStep(Parameter_Count -1, Parameter_List + 1);
		} else if((Parameter_Count == 6) && (APP_STRCMP(Parameter_List[0].String_Value,"MoveColorTemp") == 0)) {
				ret = cmd_ZCL_ColorControl_MoveColorTemp(Parameter_Count -1, Parameter_List + 1);
		} else if((Parameter_Count == 8) && (APP_STRCMP(Parameter_List[0].String_Value,"StepColorTemp") == 0)) {
				ret = cmd_ZCL_ColorControl_StepColorTemp(Parameter_Count -1, Parameter_List + 1);
		} else {
				ret = QCLI_STATUS_USAGE_E;
		}
		if ((ret == QCLI_STATUS_SUCCESS_E) || (ret == QCLI_STATUS_USAGE_E))
				LOG_AT_OK();
		else
				LOG_AT_ERROR();
		return ret;
}

const QCLI_Command_t qc_at_zb_cmd_list[] =
{
		/*  cmd function           flag     cmd_string                usage_string                                 Description      */
		{qc_at_zb_Help,                 false,   "HELP",                  "",                                         "Display the available zigbee commands."
		},
		{qc_at_zb_Service,              false,   "SERVICE",               "<start/stop>",                   "This command initilize or shutdown the zigbee stack."
		},
		{qc_at_zb_Device,               false,   "DEVICE",                "<add[mode][address][endpoint] | delete[devid] | show>",                   "This command add|delete|show device in device table."
		},
		{qc_at_zb_Form,                 false,   "FORM",                  "<UseSecurity(0=No,1=Yes)>,[Distributed(0=centralized,1=distributed)],[Channel(optional)]",                   "This command tells zigbee stack to form a Zigbee network."
		},
		{qc_at_zb_Join,                 false,   "JOIN",                  "<CoOrdinator(0=No,1=Yes)>,<UseSecurity(0=No,1=Yes)>,[IsRejoin(0=join operation,1=rejoin operation)],[Channel]",                   "This command tells zigbee stack to join or rejoin to Zigbee network."
		},
		{qc_at_zb_Leave,                false,   "LEAVE",                 "<Leave> | <LeaveRequest>,<TargetDevID>,<LeaveDevId>,<RemoveChildren>,<Rejoin>",                   "This command Leave the ZigBee netork or Sends a Leave request."
		},
		{qc_at_zb_PermitJoin,           false,   "PERMITJOIN",             "<Duration(0-255)>",                   "Permit devices to join the network for a specified period."
		},
		{qc_at_zb_Bind,                 false,   "BIND",                   "<bind|unbind>,<TargetDevID><SrcDevID><DestDevID><ClusterID>",    "Issue the ZDP bind request or Issue the ZDP end device bind request."
		},
		{qc_at_zb_Attr,                 false,   "ATTR",                   "<<GetNIB><AttID>[AttrIndex][MaxLength] \n\t <SetNIB><AttID><AttrIndex><Length><Value> \n\t <GetAIB><AttID>[AttrIndex][MaxLength] \n\t <SetAIB><AttID><AttrIndex><Length><Value>  \n\t  <GetBIB><AttID><AttrIndex><MaxLength> \n\t <SetBIB><AttID><AttrIndex><Length><Value>>",    "\n\tReads/writes a network or APS or BDB information base attribute ."
		},
		{qc_at_zb_ExtAddr,           false,   "EXTADDR",                   "[ExtAddr]",    "Set the extended address of the local interface or Get the local addresses of the ZigBee interface."
		},
		{qc_at_zb_ClearPersist,      false,   "CLEARPERSIST",              "",    "Clears ZigBee persistent data."
		},
		{qc_at_zb_cluster_Endpoints,	false,	"CLENDP",	"ListClusterTypes\n\tListEndpointtypes\n\tCreateEndpoint,<EndpointNumber>,<EndpointType>\n\tRemoveEndpoint ,<EndpointNumber>\n\tListClusters","\n\tLists the zigbee clusters commands\n\tLists the clusters that are supported by the demo\n\tLists the endpoint types that can be created by the demo\n\tCreates an endpoint\ntRemoves an endpoint\n\tDisplay the current list of clusters"},
		{qc_at_zb_cluster_attributes,	false,	"CLATTR","ReadLocAttr,<ClusterIndex>,<AttrID>,<AttrLength>\n\tWriteLocAttr,<ClusterIndex>,<AttrID>,<AttrLength>,<AttrValue>\n\tReadAttr,<DevID>,<ClusterIndex>,<AttrID>,<AttrValue>\n\tWriteAttr,<DevID>,<ClusterIndex>,<AttrID>,<AttrType>,<AttrLength>\n\tDiscAttr,<DevID>,<ClusterIndex>\n","\n\tRead a local attribute\n\tWrite a local attribute\n\tRead a remote attribute\n\tWrite a remote attribute\n\tDiscover the attributes supported by a cluster"},
		{qc_at_zb_cluster_configreport, false, "CLCFGREP", "ConfigReport,<DevID>,<ClusterIndex>,<AttrID>,<AttrType>,<Mininterval>, <MaxInterval>,<ChangeValue>\n\tReadReportConfig,<DevID>,<ClusterIndex>,<AttrID>,<AttrLength>,<AttrValue>","\n\tConfigure reporting of an attribute\n\tRead the reporting configuration of an attribute"},
		{qc_at_zb_basic,			 false,	"BASIC",		"Reset,<DevID>,<ClientEndpoint>\n\tRead,<AttID>\n\tWrite,<AttID>,<Value>","\n\tSends a Reset To Factory command to a basic server\n\tReads an attribute from the local basic server\n\tWrites an attribute to the local basic server"},
		{qc_at_zb_identify,	    	false,	"IDENTIFY", "Ident,<DevID>,<ClientEndpoint>,<Time>\n\tIdentQuery,<DevID>,<ClientEndpoint>","\n\tSends an Identify command to an Identify server\n\tSends an Identify Query command to an Identify server"},
		{qc_at_zb_group_info,		 	false,	"GROUP",	"add,<DevID>,<ClientEndpoint>,<GroupID>,<Name>,<Identifying>\n\tview,<DevID>,<ClientEndpoint>,<GroupID>\n\tremove,<DevID>,<ClientEndpoint>,<GroupID>","\n\tSends an Add Groups command to a Groups server\n\tSends a View Groups command to a Groups server\n\tSends a Remove Group command to a Groups server"},
		{qc_at_zb_group_ext,		false,	"GROUPEXT",	"GroupMem,<DevID>,<ClientEndpoint>,<GroupID>\n\tRemAll,<DevID>,<ClientEndpoint>","\n\tSends a Get Group Membership command to a Groups server\n\tSends a Remove All Groups command to a Groups server"},
		{qc_at_zb_scene,			false,	"SCENE","add,<DevID>,<ClientEndpoint>,<GroupID>,<SceneID>,<SceneName>,<TransTime>,<IsEnhanced>\n\tview,<DevID>,<ClientEndpoint>,<GroupID>,<SceneID>,<IsEnhanced>\n\tremove,<DevID>,<ClientEndpoint>,<GroupID>,<SceneID>\n\tstore,<DevID>,<ClientEndpoint>,<GroupID>,<SceneID>\n","\n\tSends an Add Scene command to a Scenes server\n\tSends a View Scene command to a Scenes server\n\tSends a Remove Scene command to a Scenes server\n\tSends a Store Scene command to a Scenes server"},
		{qc_at_zb_scene_ext,		false, "SCENEEXT","RemAllScenes,<DevID>,<ClientEndpoint>,<GroupID>\n\tRecallScenes,<DevID>,<ClientEndpoint>,<GroupID>,<SceneID>\n\tGetSceneMem,<DevID>,<ClientEndpoint>,<GroupID>\n\tCopyScene,<DevID>,<ClientEndpoint>,<CopyAll>,<GroupFrom>,<SceneFrom>,<GroupTo>,<SceneTo>\n", "\n\tSends a Remove All Scenes command to a Scenes server\n\tSends a Recall Scene command to a Scenes server\n\tSends a Get Scene Membership command to a Scenes server\n\tSends a Copy Scene command to a Scenes server"},
		{qc_at_zb_onoff,			false,	"ONOFF",	"on,<DevID>,<ClientEndpoint>\n\toff,<DevID>,<ClientEndpoint>\n\ttoggle,<DevID>,<ClientEndpoint>\n\tSetSceneData,<OnOff(0 or 1)>\n","\n\tSends an On command to an OnOff server\n\tSends an Off command to an OnOff server\n\tSends a Toggle command to an OnOff server\n\tSet the On/Off scene data"},
		{qc_at_zb_level_control,	false,	"LVL","MoveToLevel,<DevID>,<ClientEndpoint>,<WithOnOff>,<Level>,<Time>\n\tMove,<DevID>,<ClientEndpoint>,<WithOnOff>,<Direction>,<Rate>\n\tStep,<DevID>,<ClientEndpoint>,<WithOnOff>,<Direction>,<StepSize>,<Time>\n\tStop,<DevID>,<ClientEndpoint>","\n\tSends a Move To Level command to a Level Control server\n\tSends a Move command to a Level Control server\n\tSends a Step command to a Level Control server\n\tSends a Stop command to a Level Control server"},
		{qc_at_zb_level_control_scene_data, false,	"LVLSCENEDATA","<CurrentLevel(value)>","Set the level control scene data"},
		{qc_at_zb_alarm,			false,	"ALARM","ResetAlarm,<DevID>,<ClientEndpoint>,<SourceClusterId>,<AlarmCode>\n\tResetAllAlarms,<DevID>,<ClientEndpoint>\n\tGetAlarm,<DevID>,<ClientEndpoint>\n\tResetAlarmLog,<DevID>,<ClientEndpoint>\n\tAlarm,<ServerEndpoint>,<SourceCluster>,<AlarmCode>","\n\tSends a ResetAlarm command to an Alarms server\n\tSends a ResetAllAlarms command to an Alarms server\n\tSends a GetAlarm command to an Alarms server\n\tSends a ResetAlarmLog command to an Alarms server\n\tSends an Alarm command to an Alarms client"},
		{qc_at_zb_time,			false,	"TIME","Read,<AttrID>\n\tWrite,<AttrID>,<Value>","\tReads an attribute from the local time server\n\tWrites an attribute to the local time server"},
		{qc_at_zb_touchlink,	false,	"TOUCHLINK","Start,<Endpoint>,<DeviceType>\n\tScan,<ClientEndpoint>\n\tFactoryReset,<ClientEndpoint>","\tStarts the touchlink commissioning process\n\tPerforms a touchlink scan operation\n\tPerforms a touchlink factory reset"},
		{qc_at_zb_colorcontro_Hue,	false,	"COLHUE", "MoveToHue,<DevId>,<ClientEndpoint>,<Hue>,<Direction>,<Time>\n\tMoveHue,<DevId>,<ClientEndpoint>,<Mode>,<Rate>\n\t StepHue,<DevId>,<ClientEndpoint>,<Mode>,<StepSize>,<Time>","\tSends a Move To Hue command to a Color Control server\n\tSends a Move Hue command to a Color Control server\n\tSends a Step Hue command to a Color Control server"},
		{qc_at_zb_colorcontro_Sat, false,	"COLSAT","MoveToSat,<DevId>,<ClientEndpoint>,<Saturation>,<Time>\n\tMoveSat,<DevId>,<ClientEndpoint>,<Mode>,<Rate>\n\tStepSat,<DevId>,<ClientEndpoint>,<Mode>,<StepSize>,<Time>\n\tMoveToHueSat,<DevId>,<ClientEndpoint>,<Hue>,<Saturation>,<Time>","\tSends a Move To Saturation command to a Color Control server\n\tSends a Move Saturation command to a Color Control server\n\tSends a Step Saturation command to a Color Control server\n\tSends a Move To Hue And Saturation command to a Color Control server"},
		{qc_at_zb_colorcontro_Move, false,	"COL","MoveToCol,<DevId>,<ClientEndpoint>,<ColorX>,<ColorY>,<Time>\n\tMoveCol,<DevId>,<ClientEndpoint>,<RateX>,<RateY>\n\tStepCol,<DevId>,<ClientEndpoint>,<StepX>,<StepY>,<Time>\n\tMoveToColTemp,<DevId>,<ColorTempMireds>,<Time>","\tSends a Move T oColor command to a Color Control server\n\tSends a Move Color command to a Color Control server\n\tSends a Step Color command to a Color Control server\n\tSends a Move To Color Temp command to a Color Control server"},
		{qc_at_zb_colorcontro_Ext,	false,	"COLEXT","ColorLoopSet,<DevId>,<ClientEndpoint>,<Flag>,<Actmode>,<DirectMode><Time>,<StartHue>\n\tStopMoveStep,<DevId>,<ClientEndpoint>\n\tMoveColorTemp,<DevId>,<ClientEndpoint>,<Rate><MinMireds>,<MaxMireds>\n\tStepColorTemp,<DevId>,<ClientEndpoint>,<Mode>,<StepSize>,<Time>,<MinMireds>,<MaxMireds>\n","\n\tSends a Color Loop Set command to a Color Control server\n\tSends a Stop Move Step command to a Color Control server\n\tSends a Move Color Temp command to a Color Control server\n\tSends a Step Color Temp command to a Color Control server"	},

};

const QCLI_Command_Group_t qc_at_zb_cmd_group =
{
		"ATZB",
		(sizeof(qc_at_zb_cmd_list)/sizeof(qc_at_zb_cmd_list[0])),
		qc_at_zb_cmd_list
};

void Initialize_ATZB_Demo(void)
{
		/* Attempt to reqister the Command Groups with the qcli framework.*/
		qc_at_zb_group = QCLI_Register_Command_Group(NULL, &qc_at_zb_cmd_group);
		if(qc_at_zb_group != NULL) {

		}
		else {
				LOG_ERR("Failed to register ZigBee commands.\n");
		}
}
