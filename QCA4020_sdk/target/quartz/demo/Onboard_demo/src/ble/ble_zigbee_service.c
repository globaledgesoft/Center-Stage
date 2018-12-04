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

static uint32_t  ZBServiceID;
static uint8_t   op_mode[OP_MODE_SIZE];
static uint8_t   link_key[LINK_KEY_SIZE+1];

/*********************************************************************/
/**                     Onboard Service Table                       **/
/*********************************************************************/

static const qapi_BLE_GATT_Primary_Service_16_Entry_t Zigbee_Service_UUID =
{
    ZIGBEE_SERVICE_UUID_CONSTANT
} ;

static const qapi_BLE_GATT_Characteristic_Declaration_16_Entry_t Zigbee_Status_Declaration =
{
    (QAPI_BLE_GATT_CHARACTERISTIC_PROPERTIES_READ),
    ZIGBEE_STATUS_UUID_CONSTANT
} ;

static const qapi_BLE_GATT_Characteristic_Declaration_16_Entry_t Zigbee_Support_Mode =
{
    (QAPI_BLE_GATT_CHARACTERISTIC_PROPERTIES_READ),
    ZIGBEE_SUPPORT_MODE_UUID_CONSTANT
} ;

static const qapi_BLE_GATT_Characteristic_Declaration_16_Entry_t Zigbee_Operate_Mode =
{
    (QAPI_BLE_GATT_CHARACTERISTIC_PROPERTIES_WRITE | QAPI_BLE_GATT_CHARACTERISTIC_PROPERTIES_READ),
    ZIGBEE_OPERATE_MODE_UUID_CONSTANT
} ;

static const qapi_BLE_GATT_Characteristic_Declaration_16_Entry_t Zigbee_Link_Key_Declaration =
{
    (QAPI_BLE_GATT_CHARACTERISTIC_PROPERTIES_WRITE | QAPI_BLE_GATT_CHARACTERISTIC_PROPERTIES_READ),
    ZIGBEE_LINK_KEY_UUID_CONSTANT
} ;

static const qapi_BLE_GATT_Characteristic_Declaration_16_Entry_t Zigbee_Notify_Declaration =
{
    (QAPI_BLE_GATT_CHARACTERISTIC_PROPERTIES_NOTIFY | QAPI_BLE_GATT_CHARACTERISTIC_PROPERTIES_WRITE),
    ZIGBEE_NOTIFY_UUID_CONSTANT
} ;

static const qapi_BLE_GATT_Characteristic_Value_16_Entry_t  Zigbee_Status_Value =
{
    ZIGBEE_STATUS_UUID_CONSTANT,
    0,
    NULL
} ;

static const qapi_BLE_GATT_Characteristic_Value_16_Entry_t  Zigbee_Support_Mode_Value =
{
    ZIGBEE_SUPPORT_MODE_UUID_CONSTANT,
    0,
    NULL
} ;

static const qapi_BLE_GATT_Characteristic_Value_16_Entry_t  Zigbee_Operate_Mode_Value =
{
    ZIGBEE_OPERATE_MODE_UUID_CONSTANT,
    0,
    NULL
} ;

static const qapi_BLE_GATT_Characteristic_Value_16_Entry_t  Zigbee_Link_Key_Value =
{
    ZIGBEE_LINK_KEY_UUID_CONSTANT,
    0,
    NULL
} ;

static  qapi_BLE_GATT_Characteristic_Value_16_Entry_t  Zigbee_Notify_Value =
{
    ZIGBEE_NOTIFY_UUID_CONSTANT,
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
static const qapi_BLE_GATT_Service_Attribute_Entry_t ZB_Service[] =
{
    { QAPI_BLE_GATT_ATTRIBUTE_FLAGS_READABLE, QAPI_BLE_AET_PRIMARY_SERVICE_16_E, (uint8_t *)&Zigbee_Service_UUID },

    { QAPI_BLE_GATT_ATTRIBUTE_FLAGS_READABLE, QAPI_BLE_AET_CHARACTERISTIC_DECLARATION_16_E, (uint8_t *)&Zigbee_Status_Declaration },

    { QAPI_BLE_GATT_ATTRIBUTE_FLAGS_READABLE, QAPI_BLE_AET_CHARACTERISTIC_VALUE_16_E, (uint8_t *)&Zigbee_Status_Value },

    { QAPI_BLE_GATT_ATTRIBUTE_FLAGS_READABLE, QAPI_BLE_AET_CHARACTERISTIC_DECLARATION_16_E, (uint8_t *)&Zigbee_Support_Mode },

    { QAPI_BLE_GATT_ATTRIBUTE_FLAGS_READABLE, QAPI_BLE_AET_CHARACTERISTIC_VALUE_16_E, (uint8_t *)&Zigbee_Support_Mode_Value },

    { QAPI_BLE_GATT_ATTRIBUTE_FLAGS_READABLE, QAPI_BLE_AET_CHARACTERISTIC_DECLARATION_16_E, (uint8_t *)&Zigbee_Operate_Mode },

    { QAPI_BLE_GATT_ATTRIBUTE_FLAGS_READABLE_WRITABLE, QAPI_BLE_AET_CHARACTERISTIC_VALUE_16_E, (uint8_t *)&Zigbee_Operate_Mode_Value },
    { QAPI_BLE_GATT_ATTRIBUTE_FLAGS_READABLE, QAPI_BLE_AET_CHARACTERISTIC_DECLARATION_16_E, (uint8_t *)&Zigbee_Link_Key_Declaration },

    { QAPI_BLE_GATT_ATTRIBUTE_FLAGS_READABLE_WRITABLE, QAPI_BLE_AET_CHARACTERISTIC_VALUE_16_E, (uint8_t *)&Zigbee_Link_Key_Value },

    { QAPI_BLE_GATT_ATTRIBUTE_FLAGS_READABLE, QAPI_BLE_AET_CHARACTERISTIC_DECLARATION_16_E, (uint8_t *)&Zigbee_Notify_Declaration },

    { QAPI_BLE_GATT_ATTRIBUTE_FLAGS_READABLE_WRITABLE, QAPI_BLE_AET_CHARACTERISTIC_VALUE_16_E, (uint8_t *)&Zigbee_Notify_Value },

    { QAPI_BLE_GATT_ATTRIBUTE_FLAGS_READABLE_WRITABLE, QAPI_BLE_AET_CHARACTERISTIC_DESCRIPTOR_16_E, (uint8_t *)&Client_Characteristic_Configuration }

} ;

#define ZIGBEE_SERVICE_ATTRIBUTE_COUNT               (sizeof(ZB_Service)/sizeof(qapi_BLE_GATT_Service_Attribute_Entry_t))

/**
 * @func : validate_zb_data
 * @param c - operating mode
 * @param link_key - security link_key
 *
 * @return - status it will return success or failure
 */
static int32_t validate_zb_data(char c, uint8_t *link_key)
{
    switch (c)
    {
        case 'c':
        case 'C':
        case 'r':
        case 'R':
        case 'e':
        case 'E':
            if (strlen((char *)link_key) == 16)
                return SUCCESS;
            break;
    }
    return FAILURE;
}

/**
 * @func : initiate_ZB_onboard
 * @breif : function will initiate the zigbee onboarding
 *          if the data is validated sucessfully.
 */
static void initiate_ZB_onboard(uint8_t *op_mode, uint8_t *link_key, uint32_t ConnectionID)
{
    int  status=0;
    char zb_status[ZIGBEE_STR_NOTIFY_LENGTH] = "";

    status = validate_zb_data(op_mode[0], link_key);
    snprintf(zb_status, sizeof(zb_status), "%d", !status);

    switch (status)
    {
        case SUCCESS:
            set_zigbee_info(op_mode[0], link_key);
            break;

        case FAILURE:
            LOG_WARN("Data Is not Valid Please Onboard Again!!!\n");
            break;
    }
    qapi_BLE_GATT_Handle_Value_Notification(GetBluetoothStackID(), ZBServiceID, ConnectionID,
        ZIGBEE_NOTIFICATION_CREDITS_ATTRIBUTE_OFFSET, (uint16_t)ZIGBEE_STR_NOTIFY_LENGTH,
        (uint8_t *)zb_status);
    return;
}

/**
 * @func : GATT_ServerEventCallback_Onboard
 * @breif : The following function is for an GATT Server Event Callback.
 *         This function will be called whenever a GATT Request is made to the
 *         server who registers this function that cannot be handled
 *         internally by GATT.  This function passes to the caller the GATT
 *         Server Event Data that occurred and the GATT Server Event Callback
 */
static void QAPI_BLE_BTPSAPI GATT_ServerEventCallback_zigbee_service(uint32_t BluetoothStackID, qapi_BLE_GATT_Server_Event_Data_t *GATT_ServerEventData, uint32_t CallbackParameter)
{
    boolean_t     DisplayPrompt;
    uint8_t       Temp[2];
    uint16_t      AttributeOffset;
    uint16_t      AttributeLength;
    DeviceInfo_t *DeviceInfo;
    char mode[2];
    char onb_status[2];

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
                                case ZIGBEE_CREDITS_STATUS_ATTRIBUTE_OFFSET:
                                    snprintf(onb_status, sizeof(onb_status), "%d", is_zigbee_onboarded());
                                    memcpy(Temp, onb_status, sizeof(onb_status));
                                    break;

                                case ZIGBEE_CREDITS_SUPPORT_MODE_ATTRIBUTE_OFFSET:
                                    memset(mode, 0x0, sizeof(mode));
                                    snprintf(mode, sizeof(mode), "%d", get_zigbee_mode());
                                    memcpy(Temp, mode, sizeof(mode));
                                    break;

                                case ZIGBEE_CREDITS_OPERATE_MODE_ATTRIBUTE_OFFSET:
                                    //TODO future implement to handle read request
                                    break;

                                case ZIGBEE_CREDITS_LINK_KEY_ATTRIBUTE_OFFSET:
                                    //TODO future implement to handle read request
                                    break;
                            }
                            qapi_BLE_GATT_Read_Response(BluetoothStackID,
                                    GATT_ServerEventData->Event_Data.GATT_Read_Request_Data->TransactionID,
                                    sizeof(Temp), Temp);
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
                                case ZIGBEE_CREDITS_OPERATE_MODE_ATTRIBUTE_OFFSET:
                                    memset(op_mode, 0x0, OP_MODE_SIZE);
                                    memcpy(op_mode, GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->AttributeValue, AttributeLength );
                                    LOG_VERBOSE("Zigbee Mode %s \n", op_mode );
                                    break;
                                case ZIGBEE_CREDITS_LINK_KEY_ATTRIBUTE_OFFSET:
                                    memset(link_key, 0x0, sizeof(link_key));
                                    memcpy(link_key, GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->AttributeValue, AttributeLength );
                                    LOG_VERBOSE("Zigbee LinkKey %s \n",link_key );
                                    break;
                                case ZIGBEE_NOTIFICATION_CREDITS_ATTRIBUTE_OFFSET:
                                    LOG_VERBOSE("Initiate Zigbee Onboarding\n");
                                    initiate_ZB_onboard(op_mode, link_key, DeviceInfo->ConnectionID);
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

int Register_zigbee_service(void)
{
    int                                    Result;
    QCLI_Command_Status_t                  ret_val;
    qapi_BLE_GATT_Attribute_Handle_Group_t ServiceHandleGroup;

    if (!GetConnectionCount())
    {
        if (!ZBServiceID)
        {
            ServiceHandleGroup.Starting_Handle = 0;
            ServiceHandleGroup.Ending_Handle   = 0;

            Result = qapi_BLE_GATT_Register_Service(GetBluetoothStackID(), ONBOARD_SERVICE_FLAGS,
                    ZIGBEE_SERVICE_ATTRIBUTE_COUNT, (qapi_BLE_GATT_Service_Attribute_Entry_t *)ZB_Service,
                    &ServiceHandleGroup, GATT_ServerEventCallback_zigbee_service, 0);

            if (Result > 0)
            {
                LOG_INFO("Successfully registered Zigbee Onboard Service, ServiceID = %u.\n", Result);
                ZBServiceID = (unsigned int)Result;
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
            LOG_WARN("Zigbee Service already registered.\n");
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
void Cleanup_zigbee_service()
{
    if (ZBServiceID)
    {
        qapi_BLE_GATT_Un_Register_Service(GetBluetoothStackID(), ZBServiceID);
        ZBServiceID = 0;
    }
    return;
}
