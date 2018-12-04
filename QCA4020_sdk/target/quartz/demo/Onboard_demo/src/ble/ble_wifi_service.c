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

#include <stdio.h>
#include "qcli_util.h"
#include "qapi_ble_gatt.h"
#include "ble_util.h"
#include "onboard_ble.h"
#include "onboard.h"
#include "wifi_util.h"

static uint32_t                   wifiServiceID;
static char                       wifi_ssid[MAX_SSID_LEN];
static char                       wifi_passwd[MAX_PASSPHRASE_LEN];

/*********************************************************************/
/**                     Wi-Fi Service Table                         **/
/*********************************************************************/

static const qapi_BLE_GATT_Primary_Service_16_Entry_t Wifi_Service_UUID =
{
    WIFI_SERVICE_UUID_CONSTANT
} ;

static const qapi_BLE_GATT_Characteristic_Declaration_16_Entry_t Wifi_Status_Declaration =
{
    (QAPI_BLE_GATT_CHARACTERISTIC_PROPERTIES_READ ),
    WIFI_STATUS_CHARACTERISTIC_UUID_CONSTANT
} ;

static const qapi_BLE_GATT_Characteristic_Declaration_16_Entry_t Wifi_Ssid_Declaration =
{
    (QAPI_BLE_GATT_CHARACTERISTIC_PROPERTIES_WRITE | QAPI_BLE_GATT_CHARACTERISTIC_PROPERTIES_READ),
    WIFI_SSID_CHARACTERISTIC_UUID_CONSTANT
} ;

static const qapi_BLE_GATT_Characteristic_Declaration_16_Entry_t Wifi_Passwd_Declaration =
{
    (QAPI_BLE_GATT_CHARACTERISTIC_PROPERTIES_READ | QAPI_BLE_GATT_CHARACTERISTIC_PROPERTIES_WRITE_WITHOUT_RESPONSE | QAPI_BLE_GATT_CHARACTERISTIC_PROPERTIES_WRITE),
    WIFI_PASSWD_CHARACTERISTIC_UUID_CONSTANT
} ;

static const qapi_BLE_GATT_Characteristic_Declaration_16_Entry_t Wifi_Notify_Declaration =
{
    (QAPI_BLE_GATT_CHARACTERISTIC_PROPERTIES_NOTIFY | QAPI_BLE_GATT_CHARACTERISTIC_PROPERTIES_WRITE),
    WIFI_NOTIFY_CHARACTERISTIC_UUID_CONSTANT
} ;

static const qapi_BLE_GATT_Characteristic_Value_16_Entry_t  Wifi_Status_Value =
{
    WIFI_STATUS_CHARACTERISTIC_UUID_CONSTANT,
    0,
    NULL
} ;

static const qapi_BLE_GATT_Characteristic_Value_16_Entry_t  Wifi_Ssid_Value =
{
    WIFI_SSID_CHARACTERISTIC_UUID_CONSTANT,
    0,
    NULL
} ;

static const qapi_BLE_GATT_Characteristic_Value_16_Entry_t  Wifi_Passwd_Value =
{
    WIFI_PASSWD_CHARACTERISTIC_UUID_CONSTANT,
    0,
    NULL
} ;

static  qapi_BLE_GATT_Characteristic_Value_16_Entry_t  Wifi_Notify_Value =
{
    WIFI_NOTIFY_CHARACTERISTIC_UUID_CONSTANT,
    0,
    NULL
} ;

static qapi_BLE_GATT_Characteristic_Descriptor_16_Entry_t Client_Characteristic_Configuration =
{
    QAPI_BLE_GATT_CLIENT_CHARACTERISTIC_CONFIGURATION_BLUETOOTH_UUID_CONSTANT,
    QAPI_BLE_GATT_CLIENT_CHARACTERISTIC_CONFIGURATION_LENGTH,
    NULL
};

/**
 * Wi-Fi Onboard Service Characteristics and Value declaration Table
 */
static const qapi_BLE_GATT_Service_Attribute_Entry_t Wifi_Service[] =
{
    { QAPI_BLE_GATT_ATTRIBUTE_FLAGS_READABLE, QAPI_BLE_AET_PRIMARY_SERVICE_16_E, (uint8_t *)&Wifi_Service_UUID },

    { QAPI_BLE_GATT_ATTRIBUTE_FLAGS_READABLE, QAPI_BLE_AET_CHARACTERISTIC_DECLARATION_16_E, (uint8_t *)&Wifi_Status_Declaration },

    { QAPI_BLE_GATT_ATTRIBUTE_FLAGS_READABLE, QAPI_BLE_AET_CHARACTERISTIC_VALUE_16_E, (uint8_t *)&Wifi_Status_Value },

    { QAPI_BLE_GATT_ATTRIBUTE_FLAGS_READABLE, QAPI_BLE_AET_CHARACTERISTIC_DECLARATION_16_E, (uint8_t *)&Wifi_Ssid_Declaration },

    { QAPI_BLE_GATT_ATTRIBUTE_FLAGS_READABLE_WRITABLE, QAPI_BLE_AET_CHARACTERISTIC_VALUE_16_E, (uint8_t *)&Wifi_Ssid_Value },

    { QAPI_BLE_GATT_ATTRIBUTE_FLAGS_READABLE, QAPI_BLE_AET_CHARACTERISTIC_DECLARATION_16_E, (uint8_t *)&Wifi_Passwd_Declaration },

    { QAPI_BLE_GATT_ATTRIBUTE_FLAGS_READABLE_WRITABLE, QAPI_BLE_AET_CHARACTERISTIC_VALUE_16_E, (uint8_t *)&Wifi_Passwd_Value },

    { QAPI_BLE_GATT_ATTRIBUTE_FLAGS_READABLE, QAPI_BLE_AET_CHARACTERISTIC_DECLARATION_16_E, (uint8_t *)&Wifi_Notify_Declaration },

    { QAPI_BLE_GATT_ATTRIBUTE_FLAGS_READABLE_WRITABLE, QAPI_BLE_AET_CHARACTERISTIC_VALUE_16_E, (uint8_t *)&Wifi_Notify_Value },

    { QAPI_BLE_GATT_ATTRIBUTE_FLAGS_READABLE_WRITABLE, QAPI_BLE_AET_CHARACTERISTIC_DESCRIPTOR_16_E, (uint8_t *)&Client_Characteristic_Configuration }

};

#define ONBOARD_SERVICE_ATTRIBUTE_COUNT               (sizeof(Wifi_Service)/sizeof(qapi_BLE_GATT_Service_Attribute_Entry_t))

static void initiate_wifi_onboard(char *wifi_ssid, char *wifi_passwd, uint32_t ConnectionID)
{
    set_wlan_sta_info(wifi_ssid, wifi_passwd);
    //TODO:  future to implement Notification in this block
    return ;
}

/**
 * @func : GATT_ServerEventCallback_wifi_service
 * @Desc : The following function is for an GATT Server Event Callback.
 *         This function will be called whenever a GATT Request is made to the
 *         server who registers this function that cannot be handled
 *         internally by GATT.  This function passes to the caller the GATT
 *         Server Event Data that occurred and the GATT Server Event Callback
 */
static void QAPI_BLE_BTPSAPI GATT_ServerEventCallback_wifi_service(uint32_t BluetoothStackID, qapi_BLE_GATT_Server_Event_Data_t *GATT_ServerEventData, uint32_t CallbackParameter)
{
    boolean_t     DisplayPrompt;
    uint8_t       Temp[BLE_BUFFR_SIZE];
    uint16_t      AttributeOffset;
    uint16_t      AttributeLength;
    int32_t       Len = 0;
    DeviceInfo_t *DeviceInfo;
    char          onb_status[2];

    if ((BluetoothStackID) && (GATT_ServerEventData))
    {
        DisplayPrompt = false;

        if ((DeviceInfo = GetCurrentPeerDeviceInfo()) != NULL)
        {
            memset(Temp, 0x0, sizeof(Temp));
            switch (GATT_ServerEventData->Event_Data_Type)
            {
                case QAPI_BLE_ET_GATT_SERVER_READ_REQUEST_E:
                    if (GATT_ServerEventData->Event_Data.GATT_Read_Request_Data)
                    {
                        if (GATT_ServerEventData->Event_Data.GATT_Read_Request_Data->AttributeValueOffset == 0)
                        {
                            switch (GATT_ServerEventData->Event_Data.GATT_Read_Request_Data->AttributeOffset)
                            {
                                case WIFI_STATUS_CREDITS_CHARACTERISTIC_ATTRIBUTE_OFFSET:
                                    memset(onb_status, 0x0, sizeof(onb_status));
                                    snprintf(onb_status, sizeof(onb_status), "%d", is_wifi_onboarded());
                                    memcpy(Temp, onb_status , sizeof(Temp));
                                    Len = sizeof(onb_status);
                                    break;
                                case WIFI_SSID_CREDITS_CHARACTERISTIC_ATTRIBUTE_OFFSET:
                                    memcpy(Temp, wifi_ssid, sizeof(Temp));
                                    Len = strlen((char *)wifi_ssid);
                                    break;
                                case WIFI_PASSWD_CREDITS_CHARACTERISTIC_ATTRIBUTE_OFFSET:
                                    memcpy(Temp, wifi_passwd, sizeof(Temp));
                                    Len = strlen((char *)wifi_passwd);
                                    break;
                            }
                            qapi_BLE_GATT_Read_Response(BluetoothStackID,
                                    GATT_ServerEventData->Event_Data.GATT_Read_Request_Data->TransactionID,
                                    Len, Temp);
                        }
                        else
                            qapi_BLE_GATT_Error_Response(BluetoothStackID, GATT_ServerEventData->Event_Data.GATT_Read_Request_Data->TransactionID, GATT_ServerEventData->Event_Data.GATT_Read_Request_Data->AttributeOffset, QAPI_BLE_ATT_PROTOCOL_ERROR_CODE_ATTRIBUTE_NOT_LONG);
                    }
                    else
                    {
                        LOG_ERROR("Invalid Read Request Event Data.\n");
                        DisplayPrompt = true;
                    }
                    break;
                case QAPI_BLE_ET_GATT_SERVER_WRITE_REQUEST_E:
                    if (GATT_ServerEventData->Event_Data.GATT_Write_Request_Data)
                    {
                        if (GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->AttributeValueOffset == 0)
                        {
                            AttributeOffset = GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->AttributeOffset;
                            AttributeLength = GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->AttributeValueLength;

                            qapi_BLE_GATT_Write_Response(BluetoothStackID, GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->TransactionID);

                            switch (AttributeOffset)
                            {
                                case WIFI_SSID_CREDITS_CHARACTERISTIC_ATTRIBUTE_OFFSET:
                                    memset(wifi_ssid, 0x0, BLE_BUFFR_SIZE);
                                    memcpy(wifi_ssid, GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->AttributeValue, AttributeLength );
                                    LOG_INFO("SSID Received %s \n", wifi_ssid);
                                    break;
                                case WIFI_PASSWD_CREDITS_CHARACTERISTIC_ATTRIBUTE_OFFSET:
                                    memset(wifi_passwd, 0x0, BLE_BUFFR_SIZE);
                                    memcpy(wifi_passwd, GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->AttributeValue, AttributeLength );
                                    if ( AttributeLength < 8 )
                                        memset(wifi_passwd, 0x0, BLE_BUFFR_SIZE);

                                    LOG_INFO("Password Received %s \n", *wifi_passwd==0 ? "Open Network" : wifi_passwd);
                                    break;
                                case WIFI_NOTIFICATION_CREDITS_CHARACTERISTIC_ATTRIBUTE_OFFSET:
                                    LOG_INFO("Initiate WiFi onboard\n");

                                    initiate_wifi_onboard(wifi_ssid, wifi_passwd, DeviceInfo->ConnectionID);
                                    break;
                            }
                        }
                        else
                            qapi_BLE_GATT_Error_Response(BluetoothStackID, GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->TransactionID, GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->AttributeOffset, QAPI_BLE_ATT_PROTOCOL_ERROR_CODE_ATTRIBUTE_NOT_LONG);
                    }
                    else
                    {
                        LOG_ERROR("Invalid Write Request Event Data.\n");
                        DisplayPrompt = true;
                    }
                    break;
                default:
                    break;
            }
        }

        if (DisplayPrompt)
            QCLI_Display_Prompt();
    }
}

/**
 * @func : register_wifi_service
 * @Desc : Register the Wi-Fi Services required for Wi-Fi onboarding via BLE
 */
int Register_wifi_service(void)
{
    int                                    Result;
    QCLI_Command_Status_t                  ret_val;
    qapi_BLE_GATT_Attribute_Handle_Group_t ServiceHandleGroup;

    if (!GetConnectionCount())
    {
        if (!wifiServiceID)
        {
            ServiceHandleGroup.Starting_Handle = 0;
            ServiceHandleGroup.Ending_Handle   = 0;

            Result = qapi_BLE_GATT_Register_Service(GetBluetoothStackID(), ONBOARD_SERVICE_FLAGS,
                    ONBOARD_SERVICE_ATTRIBUTE_COUNT, (qapi_BLE_GATT_Service_Attribute_Entry_t *)Wifi_Service,
                    &ServiceHandleGroup, GATT_ServerEventCallback_wifi_service, 0);
            if (Result > 0)
            {
                LOG_INFO("Successfully registered Wi-Fi Service, ServiceID = %u.\n", Result);
                wifiServiceID = (unsigned int)Result;
                ret_val        = QCLI_STATUS_SUCCESS_E;
            }
            else
            {
                LOG_ERROR("Error - qapi_BLE_GATT_Register_Service() returned %d.\n", Result);
                ret_val = QCLI_STATUS_ERROR_E;
            }
        }
        else
        {
            LOG_WARN("Wi-Fi Service already registered.\n");
            ret_val = QCLI_STATUS_ERROR_E;
        }
    }
    else
    {
        LOG_WARN("Connection currently active.\n");
        ret_val = QCLI_STATUS_ERROR_E;
    }

    return (ret_val);
}

/**
 * @func : cleanup_onboard_service
 * @Desc : cleans the registered onboard services
 */
void Cleanup_wifi_service()
{
    if (wifiServiceID)
    {
        qapi_BLE_GATT_Un_Register_Service(GetBluetoothStackID(), wifiServiceID);
        wifiServiceID = 0;
    }
    return;
}
