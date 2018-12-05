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

static uint32_t  TDServiceID;
static uint8_t   op_mode[OP_MODE_SIZE];
static uint8_t   passphrase[PASSPHRASE_KEY_SIZE+1];

/*********************************************************************/
/**                     Onboard Service Table                       **/
/*********************************************************************/

static const qapi_BLE_GATT_Primary_Service_16_Entry_t Thread_Service_UUID =
{
    THREAD_SERVICE_UUID_CONSTANT
} ;

static const qapi_BLE_GATT_Characteristic_Declaration_16_Entry_t Thread_Status_Declaration =
{
    (QAPI_BLE_GATT_CHARACTERISTIC_PROPERTIES_READ),
    THREAD_STATUS_UUID_CONSTANT
} ;

static const qapi_BLE_GATT_Characteristic_Declaration_16_Entry_t Thread_Support_Mode =
{
    (QAPI_BLE_GATT_CHARACTERISTIC_PROPERTIES_READ),
    THREAD_SUPPORT_MODE_UUID_CONSTANT
} ;

static const qapi_BLE_GATT_Characteristic_Declaration_16_Entry_t Thread_Operate_Mode =
{
    (QAPI_BLE_GATT_CHARACTERISTIC_PROPERTIES_WRITE | QAPI_BLE_GATT_CHARACTERISTIC_PROPERTIES_READ),
    THREAD_OPERATE_MODE_UUID_CONSTANT
} ;

static const qapi_BLE_GATT_Characteristic_Declaration_16_Entry_t Thread_Passphrase_Declaration =
{
    (QAPI_BLE_GATT_CHARACTERISTIC_PROPERTIES_WRITE | QAPI_BLE_GATT_CHARACTERISTIC_PROPERTIES_READ),
    THREAD_PASSPHRASE_UUID_CONSTANT
} ;

static const qapi_BLE_GATT_Characteristic_Declaration_16_Entry_t Thread_Notify_Declaration =
{
    (QAPI_BLE_GATT_CHARACTERISTIC_PROPERTIES_NOTIFY | QAPI_BLE_GATT_CHARACTERISTIC_PROPERTIES_WRITE),
    THREAD_NOTIFY_UUID_CONSTANT
} ;

static const qapi_BLE_GATT_Characteristic_Value_16_Entry_t  Thread_Status_Value =
{
    THREAD_STATUS_UUID_CONSTANT,
    0,
    NULL
} ;

static const qapi_BLE_GATT_Characteristic_Value_16_Entry_t  Thread_Support_Mode_Value =
{
    THREAD_SUPPORT_MODE_UUID_CONSTANT,
    0,
    NULL
} ;

static const qapi_BLE_GATT_Characteristic_Value_16_Entry_t  Thread_Operate_Mode_Value =
{
    THREAD_OPERATE_MODE_UUID_CONSTANT,
    0,
    NULL
} ;

static const qapi_BLE_GATT_Characteristic_Value_16_Entry_t  Thread_Passphrase_Value =
{
    THREAD_PASSPHRASE_UUID_CONSTANT,
    0,
    NULL
} ;

static  qapi_BLE_GATT_Characteristic_Value_16_Entry_t  Thread_Notify_Value =
{
    THREAD_NOTIFY_UUID_CONSTANT,
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
static const qapi_BLE_GATT_Service_Attribute_Entry_t TD_Service[] =
{
    { QAPI_BLE_GATT_ATTRIBUTE_FLAGS_READABLE, QAPI_BLE_AET_PRIMARY_SERVICE_16_E, (uint8_t *)&Thread_Service_UUID },

    { QAPI_BLE_GATT_ATTRIBUTE_FLAGS_READABLE, QAPI_BLE_AET_CHARACTERISTIC_DECLARATION_16_E, (uint8_t *)&Thread_Status_Declaration },

    { QAPI_BLE_GATT_ATTRIBUTE_FLAGS_READABLE, QAPI_BLE_AET_CHARACTERISTIC_VALUE_16_E, (uint8_t *)&Thread_Status_Value },

    { QAPI_BLE_GATT_ATTRIBUTE_FLAGS_READABLE, QAPI_BLE_AET_CHARACTERISTIC_DECLARATION_16_E, (uint8_t *)&Thread_Support_Mode },

    { QAPI_BLE_GATT_ATTRIBUTE_FLAGS_READABLE, QAPI_BLE_AET_CHARACTERISTIC_VALUE_16_E, (uint8_t *)&Thread_Support_Mode_Value },

    { QAPI_BLE_GATT_ATTRIBUTE_FLAGS_READABLE, QAPI_BLE_AET_CHARACTERISTIC_DECLARATION_16_E, (uint8_t *)&Thread_Operate_Mode },

    { QAPI_BLE_GATT_ATTRIBUTE_FLAGS_READABLE_WRITABLE, QAPI_BLE_AET_CHARACTERISTIC_VALUE_16_E, (uint8_t *)&Thread_Operate_Mode_Value },
    { QAPI_BLE_GATT_ATTRIBUTE_FLAGS_READABLE, QAPI_BLE_AET_CHARACTERISTIC_DECLARATION_16_E, (uint8_t *)&Thread_Passphrase_Declaration },

    { QAPI_BLE_GATT_ATTRIBUTE_FLAGS_READABLE_WRITABLE, QAPI_BLE_AET_CHARACTERISTIC_VALUE_16_E, (uint8_t *)&Thread_Passphrase_Value },

    { QAPI_BLE_GATT_ATTRIBUTE_FLAGS_READABLE, QAPI_BLE_AET_CHARACTERISTIC_DECLARATION_16_E, (uint8_t *)&Thread_Notify_Declaration },

    { QAPI_BLE_GATT_ATTRIBUTE_FLAGS_READABLE_WRITABLE, QAPI_BLE_AET_CHARACTERISTIC_VALUE_16_E, (uint8_t *)&Thread_Notify_Value },

    { QAPI_BLE_GATT_ATTRIBUTE_FLAGS_READABLE_WRITABLE, QAPI_BLE_AET_CHARACTERISTIC_DESCRIPTOR_16_E, (uint8_t *)&Client_Characteristic_Configuration }

} ;

#define THREAD_SERVICE_ATTRIBUTE_COUNT               (sizeof(TD_Service)/sizeof(qapi_BLE_GATT_Service_Attribute_Entry_t))

/**
 * @func : validate_td_data
 * @param c - operating mode
 * @param link_key - security link_key
 *
 * @return - status it will return success or failure
 */
static int32_t validate_td_data(char c, uint8_t *passphrase)
{
    switch (c)
    {
        case 'b':
        case 'B':
        case 'r':
        case 'R':
        case 'j':
        case 'J':
            if (strlen((char *)passphrase) == 16)
                return SUCCESS;
            break;
    }
    return FAILURE;
}

/**
 * @func : initiate_Thread_onboard
 * @breif : function will initiate the thread onboarding
 *          if the data is validated sucessfully.
 */
static void initiate_thread_onboard(uint8_t *op_mode, uint8_t *passphrase, uint32_t ConnectionID)
{
    int  status=0;
    char td_status[THREAD_STR_NOTIFY_LENGTH] = "";

    status = validate_td_data(op_mode[0], passphrase);
    snprintf(td_status, sizeof(td_status), "%d", !status);

    switch (status)
    {
        case SUCCESS:
            set_thread_info(op_mode[0], passphrase);
            break;

        case FAILURE:
            LOG_WARN("Data Is not Valid Please Onboard Again!!!\n");
            break;
    }
    qapi_BLE_GATT_Handle_Value_Notification(GetBluetoothStackID(), TDServiceID, ConnectionID,
            THREAD_NOTIFICATION_CREDITS_ATTRIBUTE_OFFSET, (uint16_t)THREAD_STR_NOTIFY_LENGTH,
            (uint8_t *)td_status);
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
static void QAPI_BLE_BTPSAPI GATT_ServerEventCallback_thread_service(uint32_t BluetoothStackID, qapi_BLE_GATT_Server_Event_Data_t *GATT_ServerEventData, uint32_t CallbackParameter)
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
                                case THREAD_CREDITS_STATUS_ATTRIBUTE_OFFSET:
                                    snprintf(onb_status, sizeof(onb_status), "%d", is_thread_onboarded());
                                    memcpy(Temp, onb_status, sizeof(onb_status));
                                    break;

                                case THREAD_CREDITS_SUPPORT_MODE_ATTRIBUTE_OFFSET:
                                    memset(mode, 0x0, sizeof(mode));
                                    snprintf(mode, sizeof(mode), "%d", get_thread_mode());
                                    memcpy(Temp, mode, sizeof(mode));
                                    break;

                                case THREAD_CREDITS_OPERATE_MODE_ATTRIBUTE_OFFSET:
                                    //TODO future implement to handle read request
                                    break;

                                case THREAD_CREDITS_PASSPHRASE_ATTRIBUTE_OFFSET:
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
                                case THREAD_CREDITS_OPERATE_MODE_ATTRIBUTE_OFFSET:
                                    memset(op_mode, 0x0, OP_MODE_SIZE);
                                    memcpy(op_mode, GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->AttributeValue, AttributeLength );
                                    LOG_VERBOSE("Thread Mode %s \n", op_mode );
                                    break;
                                case THREAD_CREDITS_PASSPHRASE_ATTRIBUTE_OFFSET:
                                    memset(passphrase, 0x0, sizeof(passphrase));
                                    memcpy(passphrase, GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->AttributeValue, AttributeLength );
                                    LOG_VERBOSE("Thread passphrase %s \n",passphrase );
                                    break;
                                case THREAD_NOTIFICATION_CREDITS_ATTRIBUTE_OFFSET:
                                    LOG_VERBOSE("Initiate thread Onboarding\n");
                                    initiate_thread_onboard(op_mode, passphrase, DeviceInfo->ConnectionID);
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

int Register_thread_service(void)
{
    int                                    Result;
    QCLI_Command_Status_t                  ret_val;
    qapi_BLE_GATT_Attribute_Handle_Group_t ServiceHandleGroup;

    if (!GetConnectionCount())
    {
        if (!TDServiceID)
        {
            ServiceHandleGroup.Starting_Handle = 0;
            ServiceHandleGroup.Ending_Handle   = 0;

            Result = qapi_BLE_GATT_Register_Service(GetBluetoothStackID(), ONBOARD_SERVICE_FLAGS,
                    THREAD_SERVICE_ATTRIBUTE_COUNT, (qapi_BLE_GATT_Service_Attribute_Entry_t *)TD_Service,
                    &ServiceHandleGroup, GATT_ServerEventCallback_thread_service, 0);

            if (Result > 0)
            {
                LOG_INFO("Successfully registered thread Onboard Service, ServiceID = %u.\n", Result);
                TDServiceID = (unsigned int)Result;
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
            LOG_WARN("thread Service already registered.\n");
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
void Cleanup_thread_service()
{
    if (TDServiceID)
    {
        qapi_BLE_GATT_Un_Register_Service(GetBluetoothStackID(), TDServiceID);
        TDServiceID = 0;
    }
    return;
}
