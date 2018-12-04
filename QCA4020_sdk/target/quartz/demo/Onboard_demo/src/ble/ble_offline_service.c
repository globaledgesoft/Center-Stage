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
#include "offline.h"

static uint32_t  OFFServiceID;

/*********************************************************************/
/**                     Onboard Service Table                       **/
/*********************************************************************/

static const qapi_BLE_GATT_Primary_Service_16_Entry_t Offline_Service_UUID =
{
    OFFLINE_SERVICE_UUID_CONSTANT
} ;

static const qapi_BLE_GATT_Characteristic_Declaration_16_Entry_t Offline_Data_Request_Declaration =
{
    (QAPI_BLE_GATT_CHARACTERISTIC_PROPERTIES_WRITE | QAPI_BLE_GATT_CHARACTERISTIC_PROPERTIES_READ),
    OFFLINE_DATA_UUID_CONSTANT
} ;

static const qapi_BLE_GATT_Characteristic_Declaration_16_Entry_t Offline_Notify_Declaration =
{
    (QAPI_BLE_GATT_CHARACTERISTIC_PROPERTIES_NOTIFY),
    OFFLINE_NOTIFY_UUID_CONSTANT
} ;

static const qapi_BLE_GATT_Characteristic_Value_16_Entry_t  Offline_Data_Request_Value =
{
    OFFLINE_DATA_UUID_CONSTANT,
    0,
    NULL
} ;

static  qapi_BLE_GATT_Characteristic_Value_16_Entry_t  Offline_Notify_Value =
{
    OFFLINE_NOTIFY_UUID_CONSTANT,
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
 * Onboard Service Characteristics and Value declaration Table
 */
static const qapi_BLE_GATT_Service_Attribute_Entry_t OFFLINE_Service[] =
{
    { QAPI_BLE_GATT_ATTRIBUTE_FLAGS_READABLE, QAPI_BLE_AET_PRIMARY_SERVICE_16_E, (uint8_t *)&Offline_Service_UUID },

    { QAPI_BLE_GATT_ATTRIBUTE_FLAGS_READABLE, QAPI_BLE_AET_CHARACTERISTIC_DECLARATION_16_E, (uint8_t *)&Offline_Data_Request_Declaration },

    { QAPI_BLE_GATT_ATTRIBUTE_FLAGS_READABLE_WRITABLE, QAPI_BLE_AET_CHARACTERISTIC_VALUE_16_E, (uint8_t *)&Offline_Data_Request_Value },

    { QAPI_BLE_GATT_ATTRIBUTE_FLAGS_READABLE, QAPI_BLE_AET_CHARACTERISTIC_DECLARATION_16_E, (uint8_t *)&Offline_Notify_Declaration },

    { QAPI_BLE_GATT_ATTRIBUTE_FLAGS_READABLE_WRITABLE, QAPI_BLE_AET_CHARACTERISTIC_VALUE_16_E, (uint8_t *)&Offline_Notify_Value },

    { QAPI_BLE_GATT_ATTRIBUTE_FLAGS_READABLE_WRITABLE, QAPI_BLE_AET_CHARACTERISTIC_DESCRIPTOR_16_E, (uint8_t *)&Client_Characteristic_Configuration }

} ;

#define OFFLINE_SERVICE_ATTRIBUTE_COUNT               (sizeof(OFFLINE_Service)/sizeof(qapi_BLE_GATT_Service_Attribute_Entry_t))

/* Update the Notification Message whenever There is PIR breach is Observed */
int update_breach_message(char *mesg)
{
    DeviceInfo_t *DeviceInfo = GetCurrentPeerDeviceInfo();
    qapi_BLE_GATT_Handle_Value_Notification(GetBluetoothStackID(), OFFServiceID, DeviceInfo->ConnectionID, OFFLINE_NOTIFICATION_CREDITS_ATTRIBUTE_OFFSET ,strlen((char *)mesg)/* Data Length*/, (uint8_t *)mesg);
    return 0;
}


/**
 * @func : GATT_ServerEventCallback_Onboard
 * @breif : The following function is for an GATT Server Event Callback.
 *         This function will be called whenever a GATT Request is made to the
 *         server who registers this function that cannot be handled
 *         internally by GATT.  This function passes to the caller the GATT
 *         Server Event Data that occurred and the GATT Server Event Callback
 */
static void QAPI_BLE_BTPSAPI GATT_ServerEventCallback_offline_service(uint32_t BluetoothStackID, qapi_BLE_GATT_Server_Event_Data_t *GATT_ServerEventData, uint32_t CallbackParameter)
{
    boolean_t     DisplayPrompt;
    uint8_t       Temp[517] = {0};
    uint16_t      AttributeOffset;
    uint16_t      AttributeLength;
    DeviceInfo_t *DeviceInfo;

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
                                case OFFLINE_CREDITS_DATA_ATTRIBUTE_OFFSET:
                                    update_temp_data((char *)Temp);
                                    break;

                            }
                            qapi_BLE_GATT_Read_Response(BluetoothStackID,
                                    GATT_ServerEventData->Event_Data.GATT_Read_Request_Data->TransactionID,
                                    strlen((const char *)Temp), Temp);
                        }
                        else
                            qapi_BLE_GATT_Error_Response(BluetoothStackID,
                                    GATT_ServerEventData->Event_Data.GATT_Read_Request_Data->TransactionID,
                                    GATT_ServerEventData->Event_Data.GATT_Read_Request_Data->AttributeOffset,
                                    QAPI_BLE_ATT_PROTOCOL_ERROR_CODE_ATTRIBUTE_NOT_LONG);
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

                            qapi_BLE_GATT_Write_Response(BluetoothStackID,
                                    GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->TransactionID);
                            switch (AttributeOffset)
                            {
                                case OFFLINE_CREDITS_DATA_ATTRIBUTE_OFFSET:
									/* Any data Written From Mobile will Write Here  Copy that to Buffer*/
				                    process_request_data((char *)GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->AttributeValue,
                                            AttributeLength);
									LOG_INFO(" Length of the Data %d\n",AttributeLength);
                                    break;
                                case OFFLINE_NOTIFICATION_CREDITS_ATTRIBUTE_OFFSET:
									/* Just Notification Enbale event will come here*/
                                    break;
                            }
                        }
                        else
                            qapi_BLE_GATT_Error_Response(BluetoothStackID,
                                    GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->TransactionID,
                                    GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->AttributeOffset,
                                    QAPI_BLE_ATT_PROTOCOL_ERROR_CODE_ATTRIBUTE_NOT_LONG);
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

int Register_offline_service(void)
{
    int                                    Result;
    QCLI_Command_Status_t                  ret_val;
    qapi_BLE_GATT_Attribute_Handle_Group_t ServiceHandleGroup;

    if (!GetConnectionCount())
    {
        if (!OFFServiceID)
        {
            ServiceHandleGroup.Starting_Handle = 0;
            ServiceHandleGroup.Ending_Handle   = 0;

            Result = qapi_BLE_GATT_Register_Service(GetBluetoothStackID(), ONBOARD_SERVICE_FLAGS,
                    OFFLINE_SERVICE_ATTRIBUTE_COUNT, (qapi_BLE_GATT_Service_Attribute_Entry_t *)OFFLINE_Service,
                    &ServiceHandleGroup, GATT_ServerEventCallback_offline_service, 0);

            if (Result > 0)
            {
                LOG_INFO("Successfully registered Offline Onboard Service, ServiceID = %u.\n", Result);
                OFFServiceID = (unsigned int)Result;
                ret_val     = SUCCESS;
            }
            else
            {
                LOG_ERROR("Error - qapi_BLE_GATT_Register_Service() returned %d.\n", Result);
                ret_val = FAILURE;
            }
        }
        else
        {
            LOG_WARN("Offline Service already registered.\n");
            ret_val = FAILURE;
        }
    }
    else
    {
        LOG_WARN("Connection currently active.\n");
        ret_val = FAILURE;
    }

    return (ret_val);
}

/**
 * @func : cleanup_onboard_service
 * @breif : cleans the registered onboard services
 */
void Cleanup_offline_service()
{
    if (OFFServiceID)
    {
        qapi_BLE_GATT_Un_Register_Service(GetBluetoothStackID(), OFFServiceID);
        OFFServiceID = 0;
    }
    return;
}
