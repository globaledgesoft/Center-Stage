/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * 2015-2016 Qualcomm Atheros, Inc.
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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <qcli.h>
#include <qcli_api.h>
#include <qapi_wlan.h>
#include "qc_at_wifi.h"
#include "qc_at_p2p.h"

#include "qc_at_wifi.h"
#include "qc_api_wifi.h"
#include "qc_at_wifi_ota.h"
#include "qosa_util.h"

/*-------------------------------------------------------------------------
 * Type Declarations
 *-----------------------------------------------------------------------*/
QCLI_Group_Handle_t atw_group; /* shall we make it local if no where it is using ? */
QCLI_Group_Handle_t qcli_p2p_group;
extern QCLI_Context_t QCLI_Context;

/*-------------------------------------------------------------------------
 * Below are the wrapper functions for the UART AT WLAN commands
 *-----------------------------------------------------------------------*/

/* WLAN General Commands */
QCLI_Command_Status_t qc_at_wifi_Help(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
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

QCLI_Command_Status_t qc_at_wifi_Query(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	if (0 == qc_api_wifi_get_version())
	{
		LOG_AT_OK();
		return QCLI_STATUS_SUCCESS_E;
	}

	LOG_AT_ERROR();

	return QCLI_STATUS_ERROR_E;
}

QCLI_Command_Status_t qc_at_wifi_Enable(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	if (0 == qc_api_wifi_enable_wlan()) {
		LOG_AT_OK();
		return QCLI_STATUS_SUCCESS_E;
	}

	LOG_AT_ERROR();
	return QCLI_STATUS_ERROR_E;
}

QCLI_Command_Status_t qc_at_wifi_Disable(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	if (0 == qc_api_wifi_disable_wlan()) {
		LOG_AT_OK();
		return QCLI_STATUS_SUCCESS_E;
	}

	LOG_AT_ERROR();
	return QCLI_STATUS_ERROR_E;
}

QCLI_Command_Status_t qc_at_wifi_Details(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	if (0 == qc_api_wifi_print_wlan_info()) {
		LOG_AT_OK();
		return QCLI_STATUS_SUCCESS_E;
	}

	LOG_AT_ERROR();
	return QCLI_STATUS_ERROR_E;
}

QCLI_Command_Status_t qc_at_wifi_SetDevice(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	if( Parameter_Count < 1 || !Parameter_List || !Parameter_List[0].Integer_Is_Valid){
		return QCLI_STATUS_USAGE_E;
	}
	if (0 == qc_api_wifi_set_active_deviceid(Parameter_List[0].Integer_Value)) {
		LOG_AT_OK();
		return QCLI_STATUS_SUCCESS_E;
	}

	LOG_AT_ERROR();
	return QCLI_STATUS_ERROR_E;
}

QCLI_Command_Status_t qc_at_wifi_PhyMode(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	if( Parameter_Count != 1 || !Parameter_List || !Parameter_List[0].Integer_Is_Valid) {
		return QCLI_STATUS_USAGE_E;
	}
	if (0 == qc_api_wifi_set_phy_mode(Parameter_List[0].Integer_Value)) {
		LOG_AT_OK();
		return QCLI_STATUS_SUCCESS_E;
	}

	LOG_AT_ERROR();
	return QCLI_STATUS_ERROR_E;
}

QCLI_Command_Status_t qc_at_wifi_Reset(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	if (0 == qc_api_wifi_disable_wlan())
	{
		qurt_thread_sleep(100);
		if (0 == qc_api_wifi_enable_wlan())
		{
			LOG_AT_OK();
			return QCLI_STATUS_SUCCESS_E;
		}
	}

	LOG_AT_ERROR();
	return QCLI_STATUS_ERROR_E;
}

QCLI_Command_Status_t qc_at_wifi_OperatingMode(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	char *hidden = "";
	char *wps    = "";

	if(Parameter_Count < 1 || Parameter_Count > 3 || !Parameter_List || Parameter_List[0].Integer_Is_Valid  )
	{
		return QCLI_STATUS_USAGE_E;
	}
	if (Parameter_Count >= 2)
		hidden = Parameter_List[1].String_Value;
	if (Parameter_Count == 3)
		wps =  Parameter_List[2].String_Value;
	if (0 == qc_api_wifi_set_op_mode((int8_t *)Parameter_List[0].String_Value, hidden, wps))
	{
		LOG_AT_OK();
		return QCLI_STATUS_SUCCESS_E;
	}

	LOG_AT_ERROR();
	return QCLI_STATUS_ERROR_E;
}

QCLI_Command_Status_t qc_at_wifi_RegDomain(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	if (0 == qc_api_wifi_get_reg_domain()) {
		LOG_AT_OK();
		return QCLI_STATUS_SUCCESS_E;
	}

	LOG_AT_ERROR();
	return QCLI_STATUS_ERROR_E;
}

QCLI_Command_Status_t qc_at_wifi_CountryCode(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	if( Parameter_Count != 1 || !Parameter_List || Parameter_List[0].Integer_Is_Valid) {
		if (Parameter_Count == 0) {
			if (0 == qc_api_wifi_get_country_code()) {
				LOG_AT_OK();
				return QCLI_STATUS_SUCCESS_E;
			}
		}
		return QCLI_STATUS_USAGE_E;
	} else if (0 == qc_api_wifi_set_country_code((int8_t *) Parameter_List[0].String_Value)) {
		LOG_AT_OK();
		return QCLI_STATUS_SUCCESS_E;
	}

	LOG_AT_ERROR();
	return QCLI_STATUS_ERROR_E;
}

QCLI_Command_Status_t qc_at_wifi_LastErrInfo(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	if (0 == qc_api_wifi_get_last_error()) {
		LOG_AT_OK();
		return QCLI_STATUS_SUCCESS_E;
	}

	LOG_AT_ERROR();
	return QCLI_STATUS_ERROR_E;
}

QCLI_Command_Status_t qc_at_wifi_Channel(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int16_t channel_val;

    if (Parameter_Count == 0)
    {
        channel_val = qc_api_wifi_get_channel();
        if (channel_val > 0) {
            LOG_AT("Channel      =   %d\r\n", channel_val);
            LOG_AT_OK();
            return QCLI_STATUS_SUCCESS_E;
        }
    }
    else if (0 == qc_api_wifi_set_channel_hint(Parameter_List[0].Integer_Value)) {
        LOG_AT_OK();
		return QCLI_STATUS_SUCCESS_E;
	}

	LOG_AT_ERROR();
	return QCLI_STATUS_ERROR_E;
}

QCLI_Command_Status_t qc_at_wifi_11nCapab(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	if (Parameter_Count != 1 || !Parameter_List || Parameter_List[0].Integer_Is_Valid) {
		return QCLI_STATUS_USAGE_E;
	}
	if (0 == qc_api_wifi_set_11n_ht((int8_t *) Parameter_List[0].String_Value)) {
		LOG_AT_OK();
		return QCLI_STATUS_SUCCESS_E;
	}

	LOG_AT_ERROR();
	return QCLI_STATUS_ERROR_E;
}

QCLI_Command_Status_t qc_at_wifi_PowerMode(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	char pw_mode[32+1] = {'\0'};

	if( Parameter_Count != 1 || !Parameter_List || !Parameter_List[0].Integer_Is_Valid) {
		if (Parameter_Count == 0) {
			if (0 == qc_api_wifi_get_wifi_power_mode(pw_mode)) {
				LOG_AT("Power mode   =   %s\r\n", pw_mode);
				LOG_AT_OK();
				return QCLI_STATUS_SUCCESS_E;
			}
		}
		return QCLI_STATUS_USAGE_E;
	} else if (0 == qc_api_wifi_set_pwr_mode(Parameter_List[0].Integer_Value)) {
		LOG_AT_OK();
		return QCLI_STATUS_SUCCESS_E;
	}

	LOG_AT_ERROR();
	return QCLI_STATUS_ERROR_E;
}

QCLI_Command_Status_t qc_at_wifi_TxRate(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	int32_t isMcs = -1, rateIdx = 0;

	if( Parameter_Count != 1 || !Parameter_List || !Parameter_List[0].Integer_Is_Valid) {
		if (Parameter_Count == 0) {
			if (0 == qc_api_wifi_get_rate())
			{
				LOG_AT_OK();
				return QCLI_STATUS_SUCCESS_E;
			}
			LOG_AT_ERROR();
			return QCLI_STATUS_ERROR_E;
		}
	}
	else if( Parameter_Count == 1 && Parameter_List[0].Integer_Is_Valid )
	{
		isMcs = 0;
		rateIdx = Parameter_List[0].Integer_Value;
	}
	else if( Parameter_Count == 2 && (!Parameter_List[0].Integer_Is_Valid) )
	{
		isMcs = 1;
		rateIdx = Parameter_List[1].Integer_Value;
	}
	else
	{
		return QCLI_STATUS_USAGE_E;
	}

	if (0 == qc_api_wifi_set_rate(isMcs, rateIdx))
	{
		LOG_AT_OK();
		return QCLI_STATUS_SUCCESS_E;
	}
	else
	{
		LOG_AT_ERROR();
		return QCLI_STATUS_ERROR_E;
	}

	return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t qc_at_wifi_TxPower(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	if( Parameter_Count != 1 || !Parameter_List || !Parameter_List[0].Integer_Is_Valid) {
		return QCLI_STATUS_USAGE_E;
	}
	if (0 == qc_api_wifi_set_tx_power(Parameter_List[0].Integer_Value)) {
		LOG_AT_OK();
		return QCLI_STATUS_SUCCESS_E;
	}

	LOG_AT_ERROR();
	return QCLI_STATUS_ERROR_E;
}

QCLI_Command_Status_t qc_at_wifi_GetStats(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	if( Parameter_Count < 1 || !Parameter_List || !Parameter_List[0].Integer_Is_Valid){
		return QCLI_STATUS_USAGE_E;
	}
	if (0 == qc_api_wifi_get_wlan_stats(Parameter_List[0].Integer_Value)) {
		LOG_AT_OK();
		return QCLI_STATUS_SUCCESS_E;
	}

	LOG_AT_ERROR();
	return QCLI_STATUS_ERROR_E;
}

/* WLAN STA Commands */
QCLI_Command_Status_t qc_at_wifi_Scan(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	if (0 ==  qc_api_wifi_wlan_scan(Parameter_Count, Parameter_List)) {
		LOG_AT_OK();
		return QCLI_STATUS_SUCCESS_E;
	}

	LOG_AT_ERROR();
	return QCLI_STATUS_ERROR_E;
}

QCLI_Command_Status_t qc_at_wifi_ScanResult(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    qc_api_wifi_wlan_scan_result(Parameter_Count, Parameter_List);
    LOG_AT_OK();
    return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t qc_at_wifi_Disconnect(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	if (0 == qc_api_wifi_disconnect_from_network()) {
		LOG_AT_OK();
		return QCLI_STATUS_SUCCESS_E;
	}

	LOG_AT_ERROR();
	return QCLI_STATUS_ERROR_E;
}

QCLI_Command_Status_t qc_at_wifi_GetRssi(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	if( Parameter_Count != 0) {
		return QCLI_STATUS_USAGE_E;
	}
	if (0 == qc_api_wifi_get_rssi()) {
		LOG_AT_OK();
		return QCLI_STATUS_SUCCESS_E;
	}

	LOG_AT_ERROR();
	return QCLI_STATUS_ERROR_E;
}

QCLI_Command_Status_t qc_at_wifi_OpenConnect(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	if( Parameter_Count < 1 || !Parameter_List ){
		return QCLI_STATUS_USAGE_E;
	}
	if (0 ==  qc_api_wifi_connect_to_network((const int8_t*) Parameter_List[0].String_Value)){
		LOG_AT_OK();
		return QCLI_STATUS_SUCCESS_E;
	}

	LOG_AT_ERROR();
	return QCLI_STATUS_ERROR_E;
}

QCLI_Command_Status_t qc_at_wifi_WepConnect(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	if (Parameter_Count != 3 || !Parameter_List || !Parameter_List[1].Integer_Is_Valid) {
		return QCLI_STATUS_USAGE_E;
	}
	if (0 == qc_api_wifi_wep_connect(Parameter_List)) {
		LOG_AT_OK();
		return QCLI_STATUS_SUCCESS_E;
	}

	LOG_AT_ERROR();
	return QCLI_STATUS_ERROR_E;
}

QCLI_Command_Status_t qc_at_wifi_WpaConnect(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	if (Parameter_Count != 5 || !Parameter_List ||
			!Parameter_List[1].Integer_Is_Valid ||
			!Parameter_List[2].Integer_Is_Valid ||
			!Parameter_List[3].Integer_Is_Valid ) {
		return QCLI_STATUS_USAGE_E;
	}
	if (0 == qc_api_wifi_wpa_connect(Parameter_List)) {
		LOG_AT_OK();
		return QCLI_STATUS_SUCCESS_E;
	}

	LOG_AT_ERROR();
	return QCLI_STATUS_ERROR_E;
}

QCLI_Command_Status_t qc_at_wifi_WpsConnect(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	if (Parameter_Count < 2) {
		return QCLI_STATUS_USAGE_E;
	} else if (Parameter_Count >= 2) {
		if (Parameter_List[1].Integer_Is_Valid && Parameter_List[1].Integer_Value == 0) { /* Push method */
			if (Parameter_Count != 2 && Parameter_Count != 5) {
				return QCLI_STATUS_USAGE_E;
			}
		} else if (Parameter_List[1].Integer_Value == 1){ /* Pin method */
			if (Parameter_Count != 3 && Parameter_Count != 6) {
				return QCLI_STATUS_USAGE_E;
			}
		} else {
			LOG_AT_ERROR();
			LOG_ERR("Invalid WPS mode\n\r");
			return QCLI_STATUS_ERROR_E;
		}
	}

	if (0 == qc_api_wifi_wps_connect(Parameter_Count, Parameter_List))
	{
		LOG_AT_OK();
		return QCLI_STATUS_SUCCESS_E;
	}

	LOG_AT_ERROR();
	return QCLI_STATUS_ERROR_E;
}

/* Wireless SoftAP commands */
QCLI_Command_Status_t qc_at_wifi_ApOpen(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	if( Parameter_Count < 1 || !Parameter_List ){
		return QCLI_STATUS_USAGE_E;
	}
	if (0 ==  qc_api_wifi_connect_to_network((const int8_t*) Parameter_List[0].String_Value)){
		LOG_AT_OK();
		return QCLI_STATUS_SUCCESS_E;
	}

	LOG_AT_ERROR();
	return QCLI_STATUS_ERROR_E;

}

QCLI_Command_Status_t qc_at_wifi_ApWpa(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	if (Parameter_Count != 5 || !Parameter_List ||
			!Parameter_List[1].Integer_Is_Valid ||
			!Parameter_List[2].Integer_Is_Valid ||
			!Parameter_List[3].Integer_Is_Valid ) {
		return QCLI_STATUS_USAGE_E;
	}
	if (0 == qc_api_wifi_wpa_connect(Parameter_List)) {
		LOG_AT_OK();
		return QCLI_STATUS_SUCCESS_E;
	}

	LOG_AT_ERROR();
	return QCLI_STATUS_ERROR_E;
}

QCLI_Command_Status_t qc_at_wifi_ApWep(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	if (Parameter_Count != 3 || !Parameter_List || !Parameter_List[1].Integer_Is_Valid) {
		return QCLI_STATUS_USAGE_E;
	}
	if (0 == qc_api_wifi_wep_connect(Parameter_List)) {
		LOG_AT_OK();
		return QCLI_STATUS_SUCCESS_E;
	}

	LOG_AT_ERROR();
	return QCLI_STATUS_ERROR_E;
}

QCLI_Command_Status_t qc_at_wifi_BconIntvl(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	if( Parameter_Count < 1 || !Parameter_List || !Parameter_List[0].Integer_Is_Valid){
		return QCLI_STATUS_USAGE_E;
	}
	if (0 == qc_api_wifi_set_ap_beacon_interval(Parameter_List[0].Integer_Value)) {
		LOG_AT_OK();
		return QCLI_STATUS_SUCCESS_E;
	}

	LOG_AT_ERROR();
	return QCLI_STATUS_ERROR_E;
}

QCLI_Command_Status_t qc_at_wifi_DtimPeriod(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	if( Parameter_Count < 1 || !Parameter_List || !Parameter_List[0].Integer_Is_Valid){
		return QCLI_STATUS_USAGE_E;
	}
	if (0 == qc_api_wifi_set_ap_dtim_period(Parameter_List[0].Integer_Value)) {
		LOG_AT_OK();
		return QCLI_STATUS_SUCCESS_E;
	}

	LOG_AT_ERROR();
	return QCLI_STATUS_ERROR_E;
}

QCLI_Command_Status_t qc_at_p2p_Help(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
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

#if ENABLE_P2P_MODE

QCLI_Command_Status_t qc_at_p2p_Service(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if ( Parameter_Count < 1 || !Parameter_List || !Parameter_List[0].Integer_Is_Valid){
        return QCLI_STATUS_USAGE_E;
    }

    if (Parameter_List[0].Integer_Value == 0) {
        if (0 == qc_api_p2p_disable()) {
            LOG_AT_OK();
            return QCLI_STATUS_SUCCESS_E;
        } else
            return QCLI_STATUS_ERROR_E;
    } else if (Parameter_List[0].Integer_Value == 1) {
        if (0 == qc_api_p2p_enable())
        {
            LOG_AT_OK();
            return QCLI_STATUS_SUCCESS_E;
        } else
            return QCLI_STATUS_ERROR_E;
    } else {
        LOG_AT_ERROR();
        return QCLI_STATUS_USAGE_E;
    }

}

QCLI_Command_Status_t qc_at_p2p_SetConfig(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if((Parameter_Count < 5))
    {
         return QCLI_STATUS_USAGE_E;
    }

    if (0 == qc_api_p2p_set_config(Parameter_Count, Parameter_List))
    {
        LOG_AT_OK();
        return QCLI_STATUS_SUCCESS_E;
    }

    LOG_AT_ERROR();
    return QCLI_STATUS_ERROR_E;
}

QCLI_Command_Status_t qc_at_p2p_Connect(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if((Parameter_Count < 2))
    {
         return QCLI_STATUS_USAGE_E;
    }
    if (0 == qc_api_p2p_connect(Parameter_Count, Parameter_List))
    {
        LOG_AT_OK();
        return QCLI_STATUS_SUCCESS_E;
    }

    LOG_AT_ERROR();
    return QCLI_STATUS_ERROR_E;
}

QCLI_Command_Status_t qc_at_p2p_Find(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if (0 == qc_api_p2p_find(Parameter_Count, Parameter_List))
    {
        LOG_AT_OK();
        return QCLI_STATUS_SUCCESS_E;
    }

    LOG_AT_ERROR();
   return QCLI_STATUS_ERROR_E;
}

QCLI_Command_Status_t qc_at_p2p_Provision(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if(Parameter_Count != 2)
    {
        return QCLI_STATUS_USAGE_E;
    }

    if (0 == qc_api_p2p_provision(Parameter_Count, Parameter_List))
    {
        LOG_AT_OK();
        return QCLI_STATUS_SUCCESS_E;
    }

    LOG_AT_ERROR();
    return QCLI_STATUS_ERROR_E;
}

QCLI_Command_Status_t qc_at_p2p_Listen(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if(Parameter_Count > 1)
    {
        return QCLI_STATUS_USAGE_E;
    }
    if (0 == qc_api_p2p_listen(Parameter_Count, Parameter_List))
    {
        LOG_AT_OK();
        return QCLI_STATUS_SUCCESS_E;
    }

    LOG_AT_ERROR();
    return QCLI_STATUS_ERROR_E;
}

QCLI_Command_Status_t qc_at_p2p_Cancel(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if(Parameter_Count > 0)
    {
        return QCLI_STATUS_USAGE_E;
    }
    if (0 == qc_api_p2p_cancel())
    {
        LOG_AT_OK();
        return QCLI_STATUS_SUCCESS_E;
    }

    LOG_AT_ERROR();
    return QCLI_STATUS_ERROR_E;
}

QCLI_Command_Status_t qc_at_p2p_Join(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if(Parameter_Count < 2)
    {
        return QCLI_STATUS_USAGE_E;
    }
    if (0 == qc_api_p2p_join(Parameter_Count, Parameter_List))
    {
        LOG_AT_OK();
        return QCLI_STATUS_SUCCESS_E;
    }

    LOG_AT_ERROR();
   return QCLI_STATUS_ERROR_E;
}

QCLI_Command_Status_t qc_at_p2p_Auth(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if(Parameter_Count < 2)
    {
        return QCLI_STATUS_USAGE_E;
    }
    if (0 == qc_api_p2p_auth(Parameter_Count, Parameter_List))
    {
        LOG_AT_OK();
        return QCLI_STATUS_SUCCESS_E;
    }

    LOG_AT_ERROR();
    return QCLI_STATUS_ERROR_E;
}

QCLI_Command_Status_t qc_at_p2p_AutoGO(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if(Parameter_Count > 1)
    {
        return QCLI_STATUS_USAGE_E;
    }
    if (0 == qc_api_p2p_auto_go(Parameter_Count, Parameter_List))
    {
        LOG_AT_OK();
        return QCLI_STATUS_SUCCESS_E;
    }

    LOG_AT_ERROR();
    return QCLI_STATUS_ERROR_E;
}

QCLI_Command_Status_t qc_at_p2p_InviteAuth(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if(Parameter_Count < 3)
    {
        return QCLI_STATUS_USAGE_E;
    }
    if (0 == qc_api_p2p_invite_auth(Parameter_Count, Parameter_List))
    {
        LOG_AT_OK();
        return QCLI_STATUS_SUCCESS_E;
    }

    LOG_AT_ERROR();
    return QCLI_STATUS_ERROR_E;
}

QCLI_Command_Status_t qc_at_p2p_SetNOAParams(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if(Parameter_Count < 4)
    {
        return QCLI_STATUS_USAGE_E;
    }
    if (0 == qc_api_p2p_set_noa_params(Parameter_Count, Parameter_List))
    {
        LOG_AT_OK();
        return QCLI_STATUS_SUCCESS_E;
    }

    LOG_AT_ERROR();
    return QCLI_STATUS_ERROR_E;
}

QCLI_Command_Status_t qc_at_p2p_SetOPPSParams(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if(Parameter_Count < 2)
    {
        return QCLI_STATUS_USAGE_E;
    }
    if (0 == qc_api_p2p_set_oops_params(Parameter_Count, Parameter_List))
    {
        LOG_AT_OK();
        return QCLI_STATUS_SUCCESS_E;
    }

    LOG_AT_ERROR();
    return QCLI_STATUS_ERROR_E;
}

QCLI_Command_Status_t qc_at_p2p_SetOperatingClass(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if(Parameter_Count < 3)
    {
        return QCLI_STATUS_USAGE_E;
    }
    if (0 == qc_api_p2p_set_operating_class(Parameter_Count, Parameter_List))
    {
        LOG_AT_OK();
        return QCLI_STATUS_SUCCESS_E;
    }

    LOG_AT_ERROR();
    return QCLI_STATUS_ERROR_E;
}

QCLI_Command_Status_t qc_at_p2p_GetNodeList(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if (0 == qc_api_p2p_get_node_list())
    {
        LOG_AT_OK();
        return QCLI_STATUS_SUCCESS_E;
    }

    LOG_AT_ERROR();
    return QCLI_STATUS_ERROR_E;
}

QCLI_Command_Status_t qc_at_p2p_GetNetworkList(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if (0 == qc_api_p2p_get_network_list())
    {
        LOG_AT_OK();
        return QCLI_STATUS_SUCCESS_E;
    }

    LOG_AT_ERROR();
    return QCLI_STATUS_ERROR_E;
}

QCLI_Command_Status_t qc_at_p2p_Set(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if(Parameter_Count < 2)
    {
        return QCLI_STATUS_USAGE_E;
    }
    if (0 == qc_api_p2p_set(Parameter_Count, Parameter_List))
    {
        LOG_AT_OK();
        return QCLI_STATUS_SUCCESS_E;
    }

    LOG_AT_ERROR();
    return QCLI_STATUS_ERROR_E;
}

QCLI_Command_Status_t qc_at_p2p_StopFind(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if (0 == qc_api_p2p_stop_find())
    {
        LOG_AT_OK();
        return QCLI_STATUS_SUCCESS_E;
    }

    LOG_AT_ERROR();
    return QCLI_STATUS_ERROR_E;
}

QCLI_Command_Status_t qc_at_p2p_Passphrase(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if(Parameter_Count < 2)
    {
        return QCLI_STATUS_USAGE_E;
    }
    if (0 == qc_api_p2p_passphrase(Parameter_Count, Parameter_List))
    {
        LOG_AT_OK();
        return QCLI_STATUS_SUCCESS_E;
    }

    LOG_AT_ERROR();
    return QCLI_STATUS_ERROR_E;
}

#endif /* ENABLE_P2P_MODE */

const QCLI_Command_t atw_cmd_list[] =
{
	/*	cmd function            flag     cmd_string 	        usage_string	                             Description      */
	/* Wireless Common Commands */
	{ qc_at_wifi_Help,                 false,   "HELP",                "",                                         "Display the wifi commands."
	},
	{ qc_at_wifi_Query,                false,   "QVERSION",            "",                                         "Query WALN software version and interface version."
	},
	{ qc_at_wifi_Enable,               false,   "ENABLE",              "",                                         "Enables Wlan module."
	},
	{ qc_at_wifi_Disable,              false,   "DISABLE",             "",                                         "Disables Wlan module."
	},
	{ qc_at_wifi_Details,              false,   "INFO",                "",                                         "Display the WLAN details."
	},
	{ qc_at_wifi_SetDevice,            false,   "SETDEV",              "<device = 0:AP|GO, 1:STA|P2P client>",     "Set the operating DevID."
	},
	{ qc_at_wifi_PhyMode,              false,   "PHYMODE",             "<mode= 1:a|2:b|3:g|4:ag|5:gonly>",         "Set the wireless physical mode."
	},
	{ qc_at_wifi_Reset,                false,   "RESET",               "",                                         "Reset Wlan stack."
	},
	{ qc_at_wifi_OperatingMode,        false,   "SETOPMODE",           "<ap|station>,[<hidden|0>,<wps|0>]",        "Set the operating mode to either Soft-AP or STA. Hidden and wps parameters only apply to AP mode."
	},
	{ qc_at_wifi_RegDomain,            false,   "GETRGDOM",			 "", 										 "Retrieve current regulatory domain"
	},
	{ qc_at_wifi_CountryCode,          false,   "CCODE",			     "[<country_code_string>]", 				 "Get/set Country code to be set."
	},
	{ qc_at_wifi_LastErrInfo,          false,   "GETLASTERR",			 "", 					           			 "Query the last error information in WLAN driver"
	},
	{ qc_at_wifi_Channel,              false,   "CHANNEL",			     "", 					 					 "Get/set the channel for WLAN to operate."
	},
	{ qc_at_wifi_11nCapab,             false,   "SET11NHTCAP",		     "<mode(disable|ht20)>", 				 "Enable/disable 11n."
	},
	{ qc_at_wifi_PowerMode,            false,   "SETPOWMOD",		         "[<power-mode(0:disable|1:enable)>]", 		 "Enable/disable power save mode."
	},
	{ qc_at_wifi_TxRate,               false,   "TXRATE",		 	     "[[mcs],<fix_tx_rate_to_set>]", 		 	 "Get/set the tx rates."
	},
	{ qc_at_wifi_TxPower,              false,   "SETTXPOWER",		 	 "<txpower>", 		 	 					 "Set the transmit power level. Range 0-63. Default = 63 (max)."
	},
	{ qc_at_wifi_GetStats,    		    false,   "GETSTATS",		 	 "<resetCounters= 0: continue counters ,1: reset counters>",		"Get firmware statistics."
	},
	/* WLAN STA Commands */
	{ qc_at_wifi_Scan,                 false,   "SCAN",                 "<mode(0: blocking, 1: non-blocking, 2:non-buffering)>,[ssid]", 	"Scan for networks using blocking/non-blocking/non-buffering modes. If ssid is provided, scan for specific ssid only."
	},
	{ qc_at_wifi_ScanResult,           false,   "SCANRESULT",           "",                                        "Get the Scan result."
	},
	{ qc_at_wifi_Disconnect,           false,   "DISCONN",        	     "",										 "Disconnect for currently connected AP."
	},
	{ qc_at_wifi_GetRssi,              false,   "RSSI",                 "",                                         "Get link quality indicator (SNR in dB) between AP and STA."
	},
	{ qc_at_wifi_OpenConnect,          false,   "CONN",               	 "<ssid>",                                   "Associate with SSID, no security, open mode."
	},
	{ qc_at_wifi_WepConnect,           false,   "WEPCONN",              "<ssid>,<key_index>,<key>",          	    	  		 "Associate with SSID, using WEP Mode."
	},
	{ qc_at_wifi_WpaConnect,           false,   "WPACONN", 	         "<ssid>,<wpa_ver>,<ucipher>,<mcipher>,<passphrase>",		"Associate with SSID, using WPA."
	},
	{ qc_at_wifi_WpsConnect,           false,   "WPSCONN",              "<connect>,<mode>,[pin],[<ssid>,<mac>,<channel>]",         "Setup and start WPS connection using pin or push method."
	},
	/* Wireless SoftAP commands */
	{ qc_at_wifi_ApOpen,               false,   "STARTOPEN",           "<ssid>",                                   "Start AP with OPEN mode."
	},
	{ qc_at_wifi_ApWpa,                false,   "STARTWPA", 			 "<ssid>,<wpa_ver>,<ucipher>,<mcipher>,<passphrase>",		"Start AP with WPA/WPA2."
	},
	{ qc_at_wifi_ApWep,                false,   "STARTWEP",            "<ssid>,<index>,<key>",                     "Start AP with WEP."
	},
	{ qc_at_wifi_BconIntvl,            false,   "SETBCONINT",          "<interval(100-15000)>",                    "Set AP beacon Interval."
	},
	{ qc_at_wifi_DtimPeriod,           false,   "SETDTIMINT",          "<interval(1-255)>",                        "Set AP DTIM period."
	},
	/* Wi-Fi OTA Commands */
    { qc_at_ota_Fwd,                   false,   "OTAFWD",          	  "",                                      	  "Display FWD."
    },
    { qc_at_ota_DeleteFwd,             false,   "OTADEL",          	  "<fwd num>",                                "Erase FWD."
    },
    { qc_at_ota_Trial,                 false,   "OTATRIAL",        	  "<0|1>,<reboot flag>",                  	  "Accept/Reject Trial FWD."
    },
    { qc_at_ota_ActiveImage,           false,   "OTAIMG",          	  "<id>",                                 	  "Display Active FWD Image Info."
    },
    { qc_at_ota_FtpUpgrade,            true,    "OTAFTP",          	  "<if_name>,<<user>:<pwd>@<[ipv4|ipv6]>:<port>>,<file name>,<flag>",       "Download package from FTP server."
    },
    { qc_at_ota_HttpUpgrade,           true,    "OTAHTTP",         	  "<if_name>,<<timeout>:<http_server>:<port>>,<fw filename>",      "Download package from HTTP server."
    },
};

#if ENABLE_P2P_MODE
const QCLI_Command_t p2p_cmd_list[] =
{
   // cmd_function    start_thread   cmd_string         usage_string         description
	{ qc_at_p2p_Help,                 false,   "HELP",                "",                                         "Display the wifi commands."
    },
	{ qc_at_p2p_Service,           false,    "SERVICE",              "<value = 0|1>",                  "Enable/Disable P2P, 0-disable, 1-enable"   },
	{ qc_at_p2p_SetConfig,         false,    "SETCONFIG",       "<GO_intent> <listen channel> <operating channel> <country> <node_timeout>", "Disable/Enable P2P"   },
	{ qc_at_p2p_Connect,           false,    "CONNECT",         "<peer_dev_mac> <wps_method = push|display|keypad> [WPS pin if keypad] [persistent]",      "Initiate connection request with a given peer MAC address using given WPS configuration method."   },
	{ qc_at_p2p_Find,              false,    "FIND",            "<channel_options = 1|2|3> <timeoutInSecs>",   "Initiates search for P2P peers. Channel_options = { 1: Scan all the channels from regulatory domain channel list,  2: Scan only the social channels (default), 3: Continue channel scan from the last scanned channel index}. Default value for timeoutInSecs = 60. When the timeout period expires, the find operation is stopped. "   },
	{ qc_at_p2p_Provision,         false,    "WPSPROV",       "<peer_dev_mac> <wps_method = push|display|keypad>",   "Provision the WPS configuration method between the DUT and the peer."   },
	{ qc_at_p2p_Listen,            false,    "LISTEN",          "<timeout>",               "Initiate P2P listen process.When the timeout period expires, the listen operation is stopped. Default value is 300 seconds."   },
	{ qc_at_p2p_Cancel,            false,    "CANCEL",          "",                  "Cancels ongoing P2P operation"   },
	 { qc_at_p2p_Join,             false,    "JOIN",            "<GO_intf_mac> <wps_method = push|display|keypad> [WPS pin if keypad] [persistent]",   "Join a P2P client to an existing P2P Group Owner."   },
	{ qc_at_p2p_Auth,              false,    "AUTH",            "<peer_dev_mac> <wps_method = push|display|keypad|deauth> [WPS pin if keypad] [persistent]",   "Authenticate/Reject a connection request from a given peer MAC address using the given WPS configuration method."   },
	{ qc_at_p2p_AutoGO,            false,    "AUTOGO",          "[persistent]",      "Start P2P device in Autonomous Group Owner mode."   },
	{ qc_at_p2p_InviteAuth,        false,    "INVITE",          "<ssid> <peer_dev_mac> <wps_method= push|display|keypad> [persistent]",   "Invite a peer, from persistent database, to connect"   },
	{ qc_at_p2p_GetNodeList,       false,    "LIST",       "",                  "Display the results of P2P find operation."   },
	{ qc_at_p2p_GetNetworkList,    false,    "LISTPERSIST",    "",                  "Display the list of persistent P2P connections that are saved in the persistent media."   },
	{ qc_at_p2p_SetOPPSParams,     false,    "SETOPPS",   "<ctwin> <enable>",  "Set Opportunistics Power Save parameters."   },
	{ qc_at_p2p_SetNOAParams,      false,    "SETNOA",    "<count> <start_offset_in_usec> <duration_in_usec> <interval_in_usec> ",      "Set NOA parameters."   },
	{ qc_at_p2p_SetOperatingClass, false,    "OPERATINGCLASS",      "<GO_intent> <oper_reg_class> <oper_reg_channel>",      "Set Operating class parameters."   },
	{ qc_at_p2p_Set,               false,    "SET",             "p2pmode <p2pdev|p2pclient|p2pgo> | postfix <postfix_string> | intrabss <flag> | gointent <Intent> | cckrates <1:Enable|0:Disable> >", "Set P2P parameters" },
	{ qc_at_p2p_StopFind,          false,    "STOPFIND",        "",                  "Stop P2P operation" },
	{ qc_at_p2p_Passphrase,        false,    "PASSPHRASE",   "<passphrase> <SSID>",  "Set P2P passphrase" },
};

/* This function is used to register the wlan Command Group with    */
/* QCLI.                                                             */
const QCLI_Command_Group_t atw_cmd_group =
{
	"ATW",
	(sizeof(atw_cmd_list)/sizeof(atw_cmd_list[0])),
	atw_cmd_list
};

const QCLI_Command_Group_t p2p_cmd_group =
{
    "P2P",
    (sizeof(p2p_cmd_list) / sizeof(p2p_cmd_list[0])),
    p2p_cmd_list
};

#endif /* ENABLE_P2P_MODE */

/* This function is used to register the ATWLAN Command Group  */
void Initialize_ATW_Demo(void)
{
	/* Attempt to register the Command Groups with the UART AT framework.*/
	atw_group = QCLI_Register_Command_Group(NULL, &atw_cmd_group);

#if ENABLE_P2P_MODE
   qcli_p2p_group = QCLI_Register_Command_Group(atw_group, &p2p_cmd_group);
#endif  /* ENABLE_P2P_MODE */
}


