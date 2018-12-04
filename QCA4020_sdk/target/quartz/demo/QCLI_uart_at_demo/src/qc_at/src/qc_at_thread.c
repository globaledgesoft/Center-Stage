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

#include "qc_api_thread.h"
#include "qosa_util.h"


/*-------------------------------------------------------------------------
 * Static and Global varialble Declarations
 *-----------------------------------------------------------------------*/

extern QCLI_Context_t QCLI_Context;

/*-------------------------------------------------------------------------
 * Below are the wrapper functions for the UART AT WLAN commands
 *-----------------------------------------------------------------------*/
/**
   @brief Displays the commands supported for thread

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List  is the list of parsed arguments associated with
          this command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_at_th_Help(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if (Parameter_Count)
    {
        LOG_AT_ERROR();
        return QCLI_STATUS_USAGE_E;
    }

    if(TAKE_LOCK(QCLI_Context.CLI_Mutex))
    {
        Display_Help(QCLI_Context.Executing_Group, Parameter_Count, Parameter_List);
        RELEASE_LOCK(QCLI_Context.CLI_Mutex);
        LOG_AT_OK();
        return QCLI_STATUS_SUCCESS_E;
    }

    LOG_AT_ERROR();
    return QCLI_STATUS_ERROR_E;
}

/**
   @brief initialize/shutdown the thread interface and start/stop a thread network

   Parameter_List[0] initialize/shutdown/start/stop
   Parameter_List[1] ExtAddrSuffix - 24 bit value
   Parameter_List[2] Type - device is initialized as router or sleepy device

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List  is the list of parsed arguments associated with
          this command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_at_th_Service(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int ret = QCLI_STATUS_SUCCESS_E;

    if (Parameter_Count < 1 || Parameter_List[0].Integer_Is_Valid)
    {
        LOG_AT_ERROR();
        return QCLI_STATUS_USAGE_E;
    }

    if (strcasecmp(Parameter_List[0].String_Value, "initialize") == 0)
    {
        if (Parameter_Count < 2)
        {
            LOG_AT_ERROR();
            return QCLI_STATUS_USAGE_E;
        }

        ret = qc_api_th_Initialize(Parameter_Count - 1, Parameter_List + 1);
        if (0 == ret)
            LOG_AT_OK();
        else
            LOG_AT_ERROR();
        return ret;
    }
    else if (strcasecmp(Parameter_List[0].String_Value, "shutdown") == 0)
    {
        ret = qc_api_th_Shutdown(Parameter_Count - 1, Parameter_List + 1);
        if (0 == ret)
            LOG_AT_OK();
        else
            LOG_AT_ERROR();
        return ret;
    }
    else if (strcasecmp(Parameter_List[0].String_Value, "start") == 0)
    {
        ret = qc_api_th_Start(Parameter_Count - 1, Parameter_List + 1);
        if (0 == ret)
            LOG_AT_OK();
        else
            LOG_AT_ERROR();
        return ret;
    }
    else if (strcasecmp(Parameter_List[0].String_Value, "stop") == 0)
    {
        ret = qc_api_th_Stop(Parameter_Count - 1, Parameter_List + 1);
        if (0 == ret)
            LOG_AT_OK();
        else
            LOG_AT_ERROR();
        return ret;
    }
    else
    {
        LOG_AT_ERROR();
        return QCLI_STATUS_USAGE_E;
    }
}

/**
   @brief Set the Poll Period

   Parameter_List[0] Poll Period in Sec.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List  is the list of parsed arguments associated with
          this command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_at_th_SetPollPeriod(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if (Parameter_Count < 1 || !Parameter_List[0].Integer_Is_Valid)
    {
        LOG_AT_ERROR();
        return QCLI_STATUS_USAGE_E;
    }

    if (0 == qc_api_th_SetPollPeriod(Parameter_Count, Parameter_List))
    {
        LOG_AT_OK();
        return QCLI_STATUS_SUCCESS_E;
    }

    LOG_AT_ERROR();
    return QCLI_STATUS_ERROR_E;
}

/**
   @brief Gets the device information and the thread network information

   Parameter_List[0] getDevConfig/getNetConfig

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List  is the list of parsed arguments associated with
          this command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_at_th_Get_Config(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int ret = QCLI_STATUS_SUCCESS_E;

    if (Parameter_Count < 1 || Parameter_List[0].Integer_Is_Valid)
    {
        LOG_AT_ERROR();
        return QCLI_STATUS_USAGE_E;
    }

    if (strcasecmp(Parameter_List[0].String_Value, "device") == 0)
    {
        ret = qc_api_th_GetDeviceConfiguration(Parameter_Count - 1 , Parameter_List + 1);
        if (0 == ret)
            LOG_AT_OK();
        else
            LOG_AT_ERROR();
        return ret;
    }
    else if (strcasecmp(Parameter_List[0].String_Value, "network") == 0)
    {
        ret = qc_api_th_GetNetworkConfiguration(Parameter_Count - 1, Parameter_List + 1);
        if (0 == ret)
            LOG_AT_OK();
        else
            LOG_AT_ERROR();
        return ret;
    }
    else
    {
        LOG_AT_ERROR();
        return QCLI_STATUS_USAGE_E;
    }
}

/**
   @brief Gets the device information and the thread network information

   Parameter_List[0] setExtAdd/setChildTimeout/setChann
   Parameter_List[1] extAddr/Timeout/chann

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List  is the list of parsed arguments associated with
          this command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_at_th_Set_Config(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int ret = QCLI_STATUS_SUCCESS_E;

    if (Parameter_Count < 2 || Parameter_List[0].Integer_Is_Valid)
    {
        LOG_AT_ERROR();
        return QCLI_STATUS_USAGE_E;
    }

    if (strcasecmp(Parameter_List[0].String_Value, "extadd") == 0)
    {
        ret = qc_api_th_SetExtendedAddress(Parameter_Count - 1 , Parameter_List + 1);
        if (0 == ret)
            LOG_AT_OK();
        else
            LOG_AT_ERROR();
        return ret;
    }
    else if (strcasecmp(Parameter_List[0].String_Value, "childtimeout") == 0)
    {
        ret = qc_api_th_SetChildTimeout(Parameter_Count - 1, Parameter_List + 1);
        if (0 == ret)
            LOG_AT_OK();
        else
            LOG_AT_ERROR();
        return ret;
    }
    else if (strcasecmp(Parameter_List[0].String_Value, "channel") == 0)
    {
        ret = qc_api_th_SetChannel(Parameter_Count - 1, Parameter_List + 1);
        if (0 == ret)
            LOG_AT_OK();
        else
            LOG_AT_ERROR();
        return ret;
    }
    else
    {
        LOG_AT_ERROR();
        return QCLI_STATUS_USAGE_E;
    }
}

/**
   @brief Sets the link mode to use when connecting to thread network

   Parameter_List[0] RxOnWhenIdle (0-1)
   Parameter_List[1] UseSecurityDataRequest (0-1)
   Parameter_List[2] isFFD (0-1)
   Parameter_List[3] RequireNetworkData (0-1)

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List  is the list of parsed arguments associated with
          this command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_at_th_LinkMode(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int ret = QCLI_STATUS_SUCCESS_E;

    if (Parameter_Count < 4 || !Parameter_List[0].Integer_Is_Valid || !Parameter_List[1].Integer_Is_Valid || !Parameter_List[2].Integer_Is_Valid || !Parameter_List[3].Integer_Is_Valid)
    {
        LOG_AT_ERROR();
        return QCLI_STATUS_USAGE_E;
    }

    ret = qc_api_th_SetLinkMode(Parameter_Count, Parameter_List);
    if (0 == ret)
        LOG_AT_OK();
    else
        LOG_AT_ERROR();

    return ret;
}

QCLI_Command_Status_t qc_at_th_Set_ConfigExt(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int ret = QCLI_STATUS_SUCCESS_E;

    if (Parameter_Count < 2 || Parameter_List[0].Integer_Is_Valid)
    {
        LOG_AT_ERROR();
        return QCLI_STATUS_USAGE_E;
    }

    if (strcasecmp(Parameter_List[0].String_Value, "panid") == 0)
    {
        ret = qc_api_th_SetPANID(Parameter_Count - 1, Parameter_List + 1);
        if (0 == ret)
            LOG_AT_OK();
        else
            LOG_AT_ERROR();
        return ret;
    }
    else if (strcasecmp(Parameter_List[0].String_Value, "extpanid") == 0)
    {
        ret = qc_api_th_SetExtendedPANID(Parameter_Count - 1, Parameter_List + 1);
        if (0 == ret)
            LOG_AT_OK();
        else
            LOG_AT_ERROR();
        return ret;
    }
    else if (strcasecmp(Parameter_List[0].String_Value, "netname") == 0)
    {
        ret = qc_api_th_SetNetworkName(Parameter_Count - 1, Parameter_List + 1);
        if (0 == ret)
            LOG_AT_OK();
        else
            LOG_AT_ERROR();
        return ret;
    }
    else if (strcasecmp(Parameter_List[0].String_Value, "masterkey") == 0)
    {
        ret = qc_api_th_SetMasterKey(Parameter_Count - 1, Parameter_List + 1);
        if (0 == ret)
            LOG_AT_OK();
        else
            LOG_AT_ERROR();
        return ret;
    }
    else
    {
        LOG_AT_ERROR();
        return QCLI_STATUS_USAGE_E;
    }
}

QCLI_Command_Status_t qc_at_th_Commissioner(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int ret = QCLI_STATUS_SUCCESS_E;

    if (Parameter_Count < 1 || Parameter_List[0].Integer_Is_Valid)
    {
        LOG_AT_ERROR();
        return QCLI_STATUS_USAGE_E;
    }

    if (strcasecmp(Parameter_List[0].String_Value, "commStart") == 0)
    {
        ret = qc_api_th_MeshCoP_CommissionerStart(Parameter_Count - 1, Parameter_List + 1);
        if (0 == ret)
            LOG_AT_OK();
        else
            LOG_AT_ERROR();
        return ret;
    }
    else if (strcasecmp(Parameter_List[0].String_Value, "commStop") == 0)
    {
        ret = qc_api_th_MeshCoP_CommissionerStop(Parameter_Count - 1, Parameter_List + 1);
        if (0 == ret)
            LOG_AT_OK();
        else
            LOG_AT_ERROR();
        return ret;
    }
    else if (strcasecmp(Parameter_List[0].String_Value, "commAddJoiner") == 0)
    {
        if (Parameter_Count < 2 || Parameter_List[1].Integer_Is_Valid)
        {
            LOG_AT_ERROR();
            return QCLI_STATUS_USAGE_E;
        }

        ret = qc_api_th_MeshCoP_CommissionerAddJoiner(Parameter_Count - 1, Parameter_List + 1);
        if (0 == ret)
            LOG_AT_OK();
        else
            LOG_AT_ERROR();
        return ret;
    }
    else if (strcasecmp(Parameter_List[0].String_Value, "commDelJoiner") == 0)
    {
        ret = qc_api_th_MeshCoP_CommissionerDelJoiner(Parameter_Count - 1, Parameter_List + 1);
        if (0 == ret)
            LOG_AT_OK();
        else
            LOG_AT_ERROR();
        return ret;
    }
    else
    {
        LOG_AT_ERROR();
        return QCLI_STATUS_USAGE_E;
    }
}

QCLI_Command_Status_t qc_at_th_Joiner(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int ret = QCLI_STATUS_SUCCESS_E;

    if (Parameter_Count < 1 || Parameter_List[0].Integer_Is_Valid)
    {
        LOG_AT_ERROR();
        return QCLI_STATUS_USAGE_E;
    }

    if (strcasecmp(Parameter_List[0].String_Value, "joinerStart") == 0)
    {
        if (Parameter_Count < 2 || Parameter_List[1].Integer_Is_Valid)
        {
            LOG_AT_ERROR();
            return QCLI_STATUS_USAGE_E;
        }

        ret = qc_api_th_MeshCoP_JoinerStart(Parameter_Count - 1, Parameter_List + 1);
        if (0 == ret)
            LOG_AT_OK();
        else
            LOG_AT_ERROR();
        return ret;
    }
    else if (strcasecmp(Parameter_List[0].String_Value, "joinerStop") == 0)
    {
        ret = qc_api_th_MeshCoP_JoinerStop(Parameter_Count - 1, Parameter_List + 1);
        if (0 == ret)
            LOG_AT_OK();
        else
            LOG_AT_ERROR();
        return ret;
    }
    else
    {
        LOG_AT_ERROR();
        return QCLI_STATUS_USAGE_E;
    }
}

QCLI_Command_Status_t qc_at_th_Border_Router(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int ret = QCLI_STATUS_SUCCESS_E;

    if (Parameter_Count < 3 || Parameter_List[0].Integer_Is_Valid)
    {
        LOG_AT_ERROR();
        return QCLI_STATUS_USAGE_E;
    }

    if (strcasecmp(Parameter_List[0].String_Value, "addBordRouter") == 0)
    {
        if (Parameter_Count < 6)
        {
            LOG_AT_ERROR();
            return QCLI_STATUS_USAGE_E;
        }

        ret = qc_api_th_AddBorderRouter(Parameter_Count - 1, Parameter_List + 1);
        if (0 == ret)
            LOG_AT_OK();
        else
            LOG_AT_ERROR();
        return ret;
    }
    else if (strcasecmp(Parameter_List[0].String_Value, "delBordRouter") == 0)
    {
        ret = qc_api_th_RemoveBorderRouter(Parameter_Count - 1, Parameter_List + 1);
        if (0 == ret)
            LOG_AT_OK();
        else
            LOG_AT_ERROR();
        return ret;
    }
    else
    {
        LOG_AT_ERROR();
        return QCLI_STATUS_USAGE_E;
    }
}

QCLI_Command_Status_t qc_at_th_Ext_Route(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int ret = QCLI_STATUS_SUCCESS_E;

    if (Parameter_Count < 3 || Parameter_List[0].Integer_Is_Valid)
    {
        LOG_AT_ERROR();
        return QCLI_STATUS_USAGE_E;
    }

    if (strcasecmp(Parameter_List[0].String_Value, "addExtRoute") == 0)
    {
        if (Parameter_Count < 4)
        {
            LOG_AT_ERROR();
            return QCLI_STATUS_USAGE_E;
        }

        ret = qc_api_th_AddExternalRoute(Parameter_Count - 1, Parameter_List + 1);
        if (0 == ret)
            LOG_AT_OK();
        else
            LOG_AT_ERROR();
        return ret;
    }
    else if (strcasecmp(Parameter_List[0].String_Value, "delExtRoute") == 0)
    {
        ret = qc_api_th_RemoveExternalRoute(Parameter_Count - 1, Parameter_List + 1);
        if (0 == ret)
            LOG_AT_OK();
        else
            LOG_AT_ERROR();
        return ret;
    }
    else
    {
        LOG_AT_ERROR();
        return QCLI_STATUS_USAGE_E;
    }
}

QCLI_Command_Status_t qc_at_th_Reg_Serv_Data(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int ret = QCLI_STATUS_SUCCESS_E;

    ret = qc_api_th_RegisterServerData(Parameter_Count, Parameter_List);
    if (0 == ret)
        LOG_AT_OK();
    else
        LOG_AT_ERROR();
    return ret;
}

QCLI_Command_Status_t qc_at_th_Use_Def_Info(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int ret = QCLI_STATUS_SUCCESS_E;

    ret = qc_api_th_Thread_UseDefaultInfo(Parameter_Count, Parameter_List);
    if (0 == ret)
        LOG_AT_OK();
    else
        LOG_AT_ERROR();
    return ret;
}

QCLI_Command_Status_t qc_at_th_Border_Agent(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int ret = QCLI_STATUS_SUCCESS_E;

    if (Parameter_Count < 1 || Parameter_List[0].Integer_Is_Valid)
    {
        LOG_AT_ERROR();
        return QCLI_STATUS_USAGE_E;
    }

    if (strcasecmp(Parameter_List[0].String_Value, "start") == 0)
    {
        if (Parameter_Count < 2 || Parameter_List[1].Integer_Is_Valid)
        {
            LOG_AT_ERROR();
            return QCLI_STATUS_USAGE_E;
        }

        ret = qc_api_th_Thread_StartBorderAgent(Parameter_Count - 1, Parameter_List + 1);
        if (0 == ret)
            LOG_AT_OK();
        else
            LOG_AT_ERROR();
        return ret;
    }
    else if (strcasecmp(Parameter_List[0].String_Value, "stop") == 0)
    {
        ret = qc_api_th_Thread_StopBorderAgent(Parameter_Count - 1, Parameter_List + 1);
        if (0 == ret)
            LOG_AT_OK();
        else
            LOG_AT_ERROR();
        return ret;
    }
    else
    {
        LOG_AT_ERROR();
        return QCLI_STATUS_USAGE_E;
    }
}

QCLI_Command_Status_t qc_at_th_Update_PSKc(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int ret = QCLI_STATUS_SUCCESS_E;

    if (Parameter_Count < 1 || Parameter_List[0].Integer_Is_Valid)
    {
        LOG_AT_ERROR();
        return QCLI_STATUS_USAGE_E;
    }

    ret = qc_api_th_Thread_UpdatePSKc(Parameter_Count, Parameter_List);
    if (0 == ret)
        LOG_AT_OK();
    else
        LOG_AT_ERROR();

    return ret;
}

QCLI_Command_Status_t qc_at_th_Clear_Persist(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int ret = QCLI_STATUS_SUCCESS_E;

    ret = qc_api_th_Thread_ClearPersist(Parameter_Count, Parameter_List);
    if (0 == ret)
        LOG_AT_OK();
    else
        LOG_AT_ERROR();

    return ret;
}

QCLI_Command_Status_t qc_at_th_Leader_Router(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int ret = QCLI_STATUS_SUCCESS_E;

    if (Parameter_Count < 1 || !Parameter_List[0].Integer_Is_Valid)
    {
        LOG_AT_ERROR();
        return QCLI_STATUS_USAGE_E;
    }

    if (Parameter_List[0].Integer_Value == 0)
    {
        ret = qc_api_th_Thread_BecomeRouter(Parameter_Count - 1, Parameter_List + 1);
        if (0 == ret)
            LOG_AT_OK();
        else
            LOG_AT_ERROR();
        return ret;
    }
    else if (Parameter_List[0].Integer_Value == 1)
    {
        ret = qc_api_th_Thread_BecomeLeader(Parameter_Count - 1, Parameter_List + 1);
        if (0 == ret)
            LOG_AT_OK();
        else
            LOG_AT_ERROR();
        return ret;
    }
    else
    {
        LOG_AT_ERROR();
        return QCLI_STATUS_USAGE_E;
    }

    return ret;
}

QCLI_Command_Status_t qc_at_th_IP_Stack_Integ(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int ret = QCLI_STATUS_SUCCESS_E;

    if (Parameter_Count < 1 || !Parameter_List[0].Integer_Is_Valid)
    {
        LOG_AT_ERROR();
        return QCLI_STATUS_USAGE_E;
    }

    ret = qc_api_th_Thread_SetIPStackInteg(Parameter_Count, Parameter_List);
    if (0 == ret)
        LOG_AT_OK();
    else
        LOG_AT_ERROR();

    return ret;
}

QCLI_Command_Status_t qc_at_th_Unicast(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int ret = QCLI_STATUS_SUCCESS_E;

    if (Parameter_Count < 2 || Parameter_List[0].Integer_Is_Valid)
    {
        LOG_AT_ERROR();
        return QCLI_STATUS_USAGE_E;
    }

    if (strcasecmp(Parameter_List[0].String_Value, "add") == 0)
    {
        if (Parameter_Count < 4)
        {
            LOG_AT_ERROR();
            return QCLI_STATUS_USAGE_E;
        }

        ret = qc_api_th_Thread_AddUnicastAddress(Parameter_Count - 1, Parameter_List + 1);
        if (0 == ret)
            LOG_AT_OK();
        else
            LOG_AT_ERROR();
        return ret;
    }
    else if (strcasecmp(Parameter_List[0].String_Value, "remove") == 0)
    {
        ret = qc_api_th_Thread_RemoveUnicastAddress(Parameter_Count - 1, Parameter_List + 1);
        if (0 == ret)
            LOG_AT_OK();
        else
            LOG_AT_ERROR();
        return ret;
    }
    else
    {
        LOG_AT_ERROR();
        return QCLI_STATUS_USAGE_E;
    }
}

QCLI_Command_Status_t qc_at_th_Multicast(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int ret = QCLI_STATUS_SUCCESS_E;

    if (Parameter_Count < 2 || Parameter_List[0].Integer_Is_Valid)
    {
         LOG_AT_ERROR();
        return QCLI_STATUS_USAGE_E;
    }

    if (strcasecmp(Parameter_List[0].String_Value, "sub") == 0)
    {
        ret = qc_api_th_Thread_SubscribeMulticast(Parameter_Count - 1, Parameter_List + 1);
        if (0 == ret)
            LOG_AT_OK();
        else
            LOG_AT_ERROR();
        return ret;
    }
    else if (strcasecmp(Parameter_List[0].String_Value, "unsub") == 0)
    {
        ret = qc_api_th_Thread_UnsubscribeMulticast(Parameter_Count - 1, Parameter_List + 1);
        if (0 == ret)
            LOG_AT_OK();
        else
            LOG_AT_ERROR();
        return ret;
    }
    else
    {
        LOG_AT_ERROR();
        return QCLI_STATUS_USAGE_E;
    }
}

QCLI_Command_Status_t qc_at_th_Ping_Prov_URL(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int ret = QCLI_STATUS_SUCCESS_E;

    if (Parameter_Count < 2 || Parameter_List[0].Integer_Is_Valid)
    {
        LOG_AT_ERROR();
        return QCLI_STATUS_USAGE_E;
    }

    if (strcasecmp(Parameter_List[0].String_Value, "ping") == 0)
    {
        ret = qc_api_th_Thread_SetPingEnabled(Parameter_Count - 1, Parameter_List + 1);
        if (0 == ret)
            LOG_AT_OK();
        else
            LOG_AT_ERROR();
        return ret;
    }
    else if (strcasecmp(Parameter_List[0].String_Value, "prov") == 0)
    {
        ret = qc_api_th_Thread_SetProvisioningUrl(Parameter_Count - 1, Parameter_List + 1);
        if (0 == ret)
            LOG_AT_OK();
        else
            LOG_AT_ERROR();
        return ret;
    }
    else
    {
        LOG_AT_ERROR();
        return QCLI_STATUS_USAGE_E;
    }
}

#ifdef ALLOW_OPENTHREAD_DEBUG_LOGS
QCLI_Command_Status_t qc_at_th_Logging(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int ret = QCLI_STATUS_SUCCESS_E;

    ret = qc_api_th_Thread_SetLogging(Parameter_Count - 1, Parameter_List + 1);
    if (0 == ret)
        LOG_AT_OK();
    else
        LOG_AT_ERROR();

    return ret;
}
#endif


/* The following is the complete command list for the TWN demo. */
const QCLI_Command_t atth_cmd_list[] =
{
    /* cmd_function             thread    cmd_string         usage_string                                    description  */
    { qc_at_th_Help,            false,    "HELP",            "",                                             "Display the available thread commands." },
    { qc_at_th_Service,         false,    "SERVICE",         "initialize,<type> | shutdown | start | stop",  "Initialize/shutdown/start/stop a thread network" },
    { qc_at_th_SetPollPeriod,   false,    "SETPOLLPERIOD",   "<Period>",                                     "Set the Max Poll Period" },
    { qc_at_th_Get_Config,      false,    "GETCONFIG",       "device | Network",                  "Get the device information and the thread network information" },
    { qc_at_th_Set_Config,      false,    "SETCONFIG",       "extadd,<extAddr> | childtimeout,<Timeout> | channel,<chann>",     "Sets the extended address, child timeout, link mode to be used, channel to be used between 11 to 26" },
    { qc_at_th_LinkMode,        false,    "SETLINKMOD",      "<RxOnWhenIdle>,<UseSecureDataRequests>,<IsFFD>,<RequireNetworkData>",     "Sets the device's link mode configuration." },
    { qc_at_th_Set_ConfigExt,   false,    "SETCONFIGEXT",    "panid,<PANID> | extpanid,<ExtPANID> | netname,<name> | masterkey,<key>",     "Sets the PANID between 0x00 to 0xfffd, extended PANID, network name and master key of the appropriate network" },
    { qc_at_th_Commissioner,    false,    "COMMISSIONER",    "commStart | commStop | commAddJoiner,<Passphrase>,<ExtAddr>,<Timeout> | commDelJoiner",     "Starts/Stops acting as a Commissioner. Adds/removes the joiner device on an active network" },
    { qc_at_th_Joiner,          false,    "JOINER",          "joinerStart,<Passphrase> | joinerStop",        "Join a network or stops the joining process to a network" },
    { qc_at_th_Border_Router,   false,    "BORDERROUTER",    "addbordrouter,<prefix>,<prefixLen>,<pref>,<isstable>,<flags> | delbordrouter,<prefix>,<prefixLen>",     "Adds/removes the joiner device on an active network" },
    { qc_at_th_Ext_Route,       false,    "EXTROUTE",        "addextroute,<prefix>,<prefixLen>,<isstable> | delextroute,<extaddrsuffix>,<type>",     "Adds/removes an external route to/from the network data" },
    { qc_at_th_Reg_Serv_Data,   false,    "REGSERVDATA",     "",                                             "Register the pending network data with the network." },
    { qc_at_th_Use_Def_Info,    false,    "USEDEFINFO",      "",                                             "Populates the default network information." },
    { qc_at_th_Border_Agent,    false,    "BORDERAGENT",     "start,<interface> | stop",                     "Start/stop to act as a border agent if connected to Wi-Fi network" },
    { qc_at_th_Update_PSKc,     false,    "UPDATEPSK",       "<Passphrase>",                                 "Generates PSKc based on network info. Network must not be started." },
    { qc_at_th_Clear_Persist,   false,    "CLEARPERSIST",    "",                                             "Clears all persistent settings." },
    { qc_at_th_Leader_Router,   false,    "LEADERROUTER",    "<mode (0=Router, 1=Leader)>",                  "Attempts to become a leader/router" },
    { qc_at_th_IP_Stack_Integ,  false,    "IPSTACKINTEG",    "<Enable 0/1>",                                 "Enables or disables the use of QAPI sockets with the Thread interface." },
    { qc_at_th_Unicast,         false,    "UNICAST",         "add,<address>,<prefix_len>,<prefer> | remove,<address>",     "Add/removes static IP to/from the thread interface" },
    { qc_at_th_Multicast,       false,    "MULTICAST",       "sub,<address> | unsub,<address>",              "Subscribes/unsubscribes to/from a multicast address" },
    { qc_at_th_Ping_Prov_URL,   false,    "PINGPROVURL",     "ping,<mode> | prov,<URL>",                     "Sets the commissioner provisioning URL" },
#ifdef ALLOW_OPENTHREAD_DEBUG_LOGS
    { qc_at_th_Logging,         false,    "LOGGING",         "<mode (0=disable, 1=enable)>",                 "Enable OpenThread log messages" },
#endif
};

const QCLI_Command_Group_t atth_cmd_group =
{
    "ATTH",
    (sizeof(atth_cmd_list) / sizeof(QCLI_Command_t)),
    atth_cmd_list
};

/**
   @brief Registers Thread demo commands with QCLI.
*/
void Initialize_ATTH_Demo(void)
{

    if (NULL == QCLI_Register_Command_Group(NULL, &atth_cmd_group))
    {
        LOG_WARN("Failed to register Thread commands.\r\n");
    }
}
