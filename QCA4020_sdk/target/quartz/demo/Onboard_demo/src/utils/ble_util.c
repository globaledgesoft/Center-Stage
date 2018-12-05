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
#include <string.h>
#include <stdlib.h>

#include "qcli_api.h"

#include "ble_util.h"    /* Main Application Prototypes and Constants.*/
#include "util.h"

#define MANUFACTURER_NAME          "Qualcomm Technologies, Inc"
#define BLE_MODEL_NUMBER           "4.2.1.1"
#define BLE_SW_REVISION            "4.2"
#define BLE_HW_REVISION            "4.2"
#define BLE_FW_REVISION            "1.0"

#define ONBOARD_DATA_CREDITS       (ONBOARD_DATA_BUFFER_LENGTH*3)

#define READ_UNALIGNED_BYTE_LITTLE_ENDIAN(_x)  (((uint8_t *)(_x))[0])
#define READ_UNALIGNED_WORD_LITTLE_ENDIAN(_x)  ((uint16_t)((((uint16_t)(((uint8_t *)(_x))[1])) << 8) | ((uint16_t)(((uint8_t *)(_x))[0]))))

#define ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(_x, _y)        \
{                                                                       \
    ((uint8_t *)(_x))[0] = ((uint8_t)(((uint16_t)(_y)) & 0xFF));          \
    ((uint8_t *)(_x))[1] = ((uint8_t)((((uint16_t)(_y)) >> 8) & 0xFF));   \
}

static char *ErrorCodeStr[] =
{
    "QAPI_BLE_ATT_PROTOCOL_ERROR_CODE_NO_ERROR",
    "QAPI_BLE_ATT_PROTOCOL_ERROR_CODE_INVALID_HANDLE",
    "QAPI_BLE_ATT_PROTOCOL_ERROR_CODE_READ_NOT_PERMITTED",
    "QAPI_BLE_ATT_PROTOCOL_ERROR_CODE_WRITE_NOT_PERMITTED",
    "QAPI_BLE_ATT_PROTOCOL_ERROR_CODE_INVALID_PDU",
    "QAPI_BLE_ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_AUTHENTICATION",
    "QAPI_BLE_ATT_PROTOCOL_ERROR_CODE_REQUEST_NOT_SUPPORTED",
    "QAPI_BLE_ATT_PROTOCOL_ERROR_CODE_INVALID_OFFSET",
    "QAPI_BLE_ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_AUTHORIZATION",
    "QAPI_BLE_ATT_PROTOCOL_ERROR_CODE_PREPARE_QUEUE_FULL",
    "QAPI_BLE_ATT_PROTOCOL_ERROR_CODE_ATTRIBUTE_NOT_FOUND",
    "QAPI_BLE_ATT_PROTOCOL_ERROR_CODE_ATTRIBUTE_NOT_LONG",
    "QAPI_BLE_ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_ENCRYPTION_KEY_SIZE",
    "QAPI_BLE_ATT_PROTOCOL_ERROR_CODE_INVALID_ATTRIBUTE_VALUE_LENGTH",
    "QAPI_BLE_ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR",
    "QAPI_BLE_ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_ENCRYPTION",
    "QAPI_BLE_ATT_PROTOCOL_ERROR_CODE_UNSUPPORTED_GROUP_TYPE",
    "QAPI_BLE_ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_RESOURCES"
};

#define NUMBER_OF_ERROR_CODES     (sizeof(ErrorCodeStr)/sizeof(char *))

static GAPS_Device_Appearance_Mapping_t AppearanceMappings[] =
{
    { QAPI_BLE_GAP_DEVICE_APPEARANCE_VALUE_UNKNOWN,                        "Unknown"                   },
    { QAPI_BLE_GAP_DEVICE_APPEARANCE_VALUE_GENERIC_PHONE,                  "Generic Phone"             },
    { QAPI_BLE_GAP_DEVICE_APPEARANCE_VALUE_GENERIC_COMPUTER,               "Generic Computer"          },
    { QAPI_BLE_GAP_DEVICE_APPEARANCE_VALUE_GENERIC_WATCH,                  "Generic Watch"             },
    { QAPI_BLE_GAP_DEVICE_APPEARANCE_VALUE_SPORTS_WATCH,                   "Sports Watch"              },
    { QAPI_BLE_GAP_DEVICE_APPEARANCE_VALUE_GENERIC_CLOCK,                  "Generic Clock"             },
    { QAPI_BLE_GAP_DEVICE_APPEARANCE_VALUE_GENERIC_DISPLAY,                "Generic Display"           },
    { QAPI_BLE_GAP_DEVICE_APPEARANCE_VALUE_GENERIC_GENERIC_REMOTE_CONTROL, "Generic Remote Control"    },
    { QAPI_BLE_GAP_DEVICE_APPEARANCE_VALUE_GENERIC_EYE_GLASSES,            "Eye Glasses"               },
    { QAPI_BLE_GAP_DEVICE_APPEARANCE_VALUE_GENERIC_TAG,                    "Generic Tag"               },
    { QAPI_BLE_GAP_DEVICE_APPEARANCE_VALUE_GENERIC_KEYRING,                "Generic Keyring"           },
    { QAPI_BLE_GAP_DEVICE_APPEARANCE_VALUE_GENERIC_MEDIA_PLAYER,           "Generic Media Player"      },
    { QAPI_BLE_GAP_DEVICE_APPEARANCE_VALUE_GENERIC_BARCODE_SCANNER,        "Generic Barcode Scanner"   },
    { QAPI_BLE_GAP_DEVICE_APPEARANCE_VALUE_GENERIC_THERMOMETER,            "Generic Thermometer"       },
    { QAPI_BLE_GAP_DEVICE_APPEARANCE_VALUE_THERMOMETER_EAR,                "Ear Thermometer"           },
    { QAPI_BLE_GAP_DEVICE_APPEARANCE_VALUE_GENERIC_HEART_RATE_SENSOR,      "Generic Heart Rate Sensor" },
    { QAPI_BLE_GAP_DEVICE_APPEARANCE_VALUE_BELT_HEART_RATE_SENSOR,         "Belt Heart Rate Sensor"    },
    { QAPI_BLE_GAP_DEVICE_APPEARANCE_VALUE_GENERIC_BLOOD_PRESSURE,         "Generic Blood Pressure"    },
    { QAPI_BLE_GAP_DEVICE_APPEARANCE_VALUE_BLOOD_PRESSURE_ARM,             "Blood Pressure: ARM"       },
    { QAPI_BLE_GAP_DEVICE_APPEARANCE_VALUE_BLOOD_PRESSURE_WRIST,           "Blood Pressure: Wrist"     },
    { QAPI_BLE_GAP_DEVICE_APPEARANCE_VALUE_HUMAN_INTERFACE_DEVICE,         "Human Interface Device"    },
    { QAPI_BLE_GAP_DEVICE_APPEARANCE_VALUE_HID_KEYBOARD,                   "HID Keyboard"              },
    { QAPI_BLE_GAP_DEVICE_APPEARANCE_VALUE_HID_MOUSE,                      "HID Mouse"                 },
    { QAPI_BLE_GAP_DEVICE_APPEARANCE_VALUE_HID_JOYSTICK,                   "HID Joystick"              },
    { QAPI_BLE_GAP_DEVICE_APPEARANCE_VALUE_HID_GAMEPAD,                    "HID Gamepad"               },
    { QAPI_BLE_GAP_DEVICE_APPEARANCE_VALUE_HID_DIGITIZER_TABLET,           "HID Digitizer Tablet"      },
    { QAPI_BLE_GAP_DEVICE_APPEARANCE_VALUE_HID_CARD_READER,                "HID Card Reader"           },
    { QAPI_BLE_GAP_DEVICE_APPEARANCE_VALUE_HID_DIGITAL_PEN,                "HID Digitizer Pen"         },
    { QAPI_BLE_GAP_DEVICE_APPEARANCE_VALUE_HID_BARCODE_SCANNER,            "HID Bardcode Scanner"      },
    { QAPI_BLE_GAP_DEVICE_APPEARANCE_VALUE_GENERIC_GLUCOSE_METER,          "Generic Glucose Meter"     }
} ;

#define NUMBER_OF_APPEARANCE_MAPPINGS     (sizeof(AppearanceMappings)/sizeof(GAPS_Device_Appearance_Mapping_t))

enum
{
    AET_DISABLE_E,
    AET_ENABLE_ALL_E,
    AET_ENABLE_CHANNEL_37_E,
    AET_ENABLE_CHANNEL_38_E,
    AET_ENABLE_CHANNEL_39_E
};

typedef char               BoardStr_t[16];

static uint32_t            BluetoothStackID = 0;
static GAPLE_Parameters_t  LE_Parameters;
static BLEParameters_t     BLEParameters;

static uint32_t offline_onboard = 0;
static uint32_t enable_cordinator = 0;
static qapi_BLE_BD_ADDR_t  LocalBD_ADDR;
volatile qapi_BLE_BD_ADDR_t  SelectedRemoteBD_ADDR;
static qapi_BLE_BD_ADDR_t  SecurityRemoteBD_ADDR;
static qapi_BLE_Encryption_Key_t ER = {0x28, 0xBA, 0xE1, 0x37, 0x13, 0xB2, 0x20, 0x45, 0x16, 0xB2, 0x19, 0xD0, 0x80, 0xEE, 0x4A, 0x51};
static qapi_BLE_Encryption_Key_t IR = {0x41, 0x09, 0xA0, 0x88, 0x09, 0x6B, 0x70, 0xC0, 0x95, 0x23, 0x3C, 0x8C, 0x48, 0xFC, 0xC9, 0xFE};
static qapi_BLE_Encryption_Key_t DHK;
static qapi_BLE_Encryption_Key_t IRK;
static qapi_Persist_Handle_t PersistHandle;
static uint32_t            GAPSInstanceID;
static uint32_t            DISInstanceID;
volatile unsigned int      ConnectionCount;

static unsigned int        NumberACLPackets;
static unsigned int        NumberOutstandingACLPackets;
static unsigned int        MaxACLPacketSize;
static boolean_t           LocalDeviceIsMaster;
static qapi_BLE_GAP_LE_Address_Type_t  RemoteAddressType;
static boolean_t           DisplayAdvertisingEventData;
static Send_Info_t         SendInfo;
static boolean_t           LocalOOBValid;
static boolean_t           RemoteOOBValid;
static qapi_BLE_Secure_Connections_Randomizer_t   RemoteOOBRandomizer;
static qapi_BLE_Secure_Connections_Confirmation_t RemoteOOBConfirmation;
static qapi_BLE_Secure_Connections_Confirmation_t RemoteOOBConfirmation;
static qapi_BLE_Secure_Connections_Randomizer_t   LocalOOBRandomizer;
static qapi_BLE_Secure_Connections_Confirmation_t LocalOOBConfirmation;
DeviceInfo_t       *DeviceInfoList;
qapi_BLE_HCI_DriverInformation_t HCI_DriverInformation;


static char *PHYMapping[] =
{
    "LE 1M PHY",
    "LE 2M PHY",
    "LE Coded PHY",
    "Unknown PHY"
};

/**
 * Function Prototype
 */
static int OpenStack(qapi_BLE_HCI_DriverInformation_t *HCI_DriverInformation);
static int SetDisc(void);
static int SetConnect(void);
static int SetPairable(void);

static void BD_ADDRToStr(qapi_BLE_BD_ADDR_t Board_Address, BoardStr_t BoardStr);
static void DisplayFunctionError(char *Function, int Status);
static DeviceInfo_t *SearchDeviceInfoEntryByBD_ADDR(DeviceInfo_t **ListHead, qapi_BLE_BD_ADDR_t RemoteAddress);
static void QAPI_BLE_BTPSAPI GATT_Connection_Event_Callback(uint32_t BluetoothStackID,
        qapi_BLE_GATT_Connection_Event_Data_t *GATT_Connection_Event_Data, uint32_t CallbackParameter);
static DeviceInfo_t *CreateNewDeviceInfoEntry(DeviceInfo_t **ListHead, qapi_BLE_BD_ADDR_t RemoteAddress);
static void InitializeBuffer(Onboard_Data_Buffer_t *DataBuffer);
static boolean_t AppearanceToString(uint16_t Appearance, char **String);
static DeviceInfo_t *SearchDeviceInfoEntryTypeAddress(DeviceInfo_t **ListHead,
        qapi_BLE_GAP_LE_Address_Type_t AddressType, qapi_BLE_BD_ADDR_t RemoteAddress);
static void DisplayLegacyPairingInformation(qapi_BLE_GAP_LE_Pairing_Capabilities_t *Pairing_Capabilities);
static void DisplayPairingInformation(qapi_BLE_GAP_LE_Extended_Pairing_Capabilities_t *Pairing_Capabilities);
static void ConfigureCapabilities(qapi_BLE_GAP_LE_Extended_Pairing_Capabilities_t *Capabilities);
static int SlavePairingRequestResponse(qapi_BLE_BD_ADDR_t BD_ADDR);
static void QAPI_BLE_BTPSAPI GAP_LE_Event_Callback(uint32_t BluetoothStackID,
        qapi_BLE_GAP_LE_Event_Data_t *GAP_LE_Event_Data, uint32_t CallbackParameter);

/**
 * @func : PHYToString
 * @Desc : The following function converts the PHY to a string for printing
 */
char *PHYToString(qapi_BLE_GAP_LE_PHY_Type_t PHY)
{
    if ((PHY >= QAPI_BLE_LPT_PHY_LE_1M_E) && (PHY <= QAPI_BLE_LPT_PHY_LE_CODED_E))
        return(PHYMapping[PHY - QAPI_BLE_LPT_PHY_LE_1M_E]);
    else
        return (PHYMapping[3]);
}

/**
 * @func : BD_ADDRToStr
 * @Desc : The following function is responsible for converting data of type
 *         BD_ADDR to a string.
 */
static void BD_ADDRToStr(qapi_BLE_BD_ADDR_t Board_Address, BoardStr_t BoardStr)
{
    snprintf((char *)BoardStr, (sizeof(BoardStr_t)/sizeof(char)), "0x%02X%02X%02X%02X%02X%02X", Board_Address.BD_ADDR5, Board_Address.BD_ADDR4, Board_Address.BD_ADDR3, Board_Address.BD_ADDR2, Board_Address.BD_ADDR1, Board_Address.BD_ADDR0);
}

/**
 * @func : DisplayFunctionError
 * @Desc : Displays a function error message
 */
static void DisplayFunctionError(char *Function, int Status)
{
    LOG_ERROR("%s Failed: %d.\n", Function, Status);
}



/**
 * @func : SearchDeviceInfoEntryByBD_ADDR
 * @Desc : The following function searches the specified List for the
 *         specified Connection BD_ADDR
 */
static DeviceInfo_t *SearchDeviceInfoEntryByBD_ADDR(DeviceInfo_t **ListHead, qapi_BLE_BD_ADDR_t RemoteAddress)
{
    BoardStr_t    BoardStr;
    DeviceInfo_t *ret_val = NULL;
    DeviceInfo_t *DeviceInfo;

    if (ListHead)
    {
        DeviceInfo = *ListHead;
        while (DeviceInfo)
        {
            if (QAPI_BLE_COMPARE_BD_ADDR(DeviceInfo->RemoteAddress, RemoteAddress))
            {
                ret_val = DeviceInfo;
                break;
            }
            else
            {
                if(QAPI_BLE_GAP_LE_TEST_RESOLVABLE_ADDRESS_BITS(RemoteAddress))
                {
                    if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_IRK_VALID)
                    {
                        if(qapi_BLE_GAP_LE_Resolve_Address(BluetoothStackID, &(DeviceInfo->IRK), RemoteAddress))
                        {
                            DeviceInfo->RemoteAddress     = RemoteAddress;
                            DeviceInfo->RemoteAddressType = QAPI_BLE_LAT_RANDOM_E;

                            LOG_VERBOSE("\n");
                            LOG_VERBOSE("Resolved Address (");
                            BD_ADDRToStr(DeviceInfo->RemoteAddress, BoardStr);
                            LOG_VERBOSE("%s", BoardStr);
                            LOG_VERBOSE(")\n");
                            LOG_VERBOSE("   Identity Address:       ");
                            BD_ADDRToStr(DeviceInfo->IdentityAddressBD_ADDR, BoardStr);
                            LOG_VERBOSE("%s\n", BoardStr);
                            LOG_VERBOSE("   Identity Address Type:  %s\n", ((DeviceInfo->IdentityAddressType == QAPI_BLE_LAT_PUBLIC_IDENTITY_E) ? "Public Identity" : "Random Identity"));

                            ret_val = DeviceInfo;
                            break;
                        }
                    }
                }
            }
            DeviceInfo = DeviceInfo->NextDeviceInfoInfoPtr;
        }
    }

    return (ret_val);
}

/**
 * @func : SearchDeviceInfoEntryByBD_ADDR
 * @Desc : The following function searches the specified List for the
 *         specified Connection BD_ADDR
 */
DeviceInfo_t *GetCurrentPeerDeviceInfo(void)
{
    return SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, SelectedRemoteBD_ADDR);
}

/**
 * @func : GetConnectionCount
 * @Desc : The following function returns current connection count
 */
int GetConnectionCount(void)
{
    return ConnectionCount;
}

/**
 * @func : DeleteDeviceInfoEntry
 * #Desc : The following function searches the specified Key Info List for
 *         the specified BD_ADDR and removes it from the List
 */
static DeviceInfo_t *DeleteDeviceInfoEntry(DeviceInfo_t **ListHead, qapi_BLE_BD_ADDR_t RemoteAddress)
{
    return(qapi_BLE_BSC_DeleteGenericListEntry(QAPI_BLE_EK_BD_ADDR_T_E, (void *)(&RemoteAddress), QAPI_BLE_BTPS_STRUCTURE_OFFSET(DeviceInfo_t, RemoteAddress), QAPI_BLE_BTPS_STRUCTURE_OFFSET(DeviceInfo_t, NextDeviceInfoInfoPtr), (void **)(ListHead)));
}

/**
 * @func : CreateNewDeviceInfoEntry
 * @Desc : he following function will create a device information entry and
 *         add it to the specified List
 */
static DeviceInfo_t *CreateNewDeviceInfoEntry(DeviceInfo_t **ListHead, qapi_BLE_BD_ADDR_t RemoteAddress)
{
    DeviceInfo_t *ret_val = NULL;
    boolean_t     Result;

    if ((ListHead) && (!QAPI_BLE_COMPARE_NULL_BD_ADDR(RemoteAddress)))
    {
        if ((ret_val = malloc(sizeof(DeviceInfo_t))) != NULL)
        {
            memset(ret_val, 0, sizeof(DeviceInfo_t));

            ret_val->RemoteAddress = RemoteAddress;

            Result = qapi_BLE_BSC_AddGenericListEntry_Actual(QAPI_BLE_EK_BD_ADDR_T_E, QAPI_BLE_BTPS_STRUCTURE_OFFSET(DeviceInfo_t, RemoteAddress), QAPI_BLE_BTPS_STRUCTURE_OFFSET(DeviceInfo_t, NextDeviceInfoInfoPtr), (void **)(ListHead), (void *)(ret_val));
            if (!Result)
            {
                free(ret_val);
            }
        }
    }

    return (ret_val);
}

/**
 * @func : AppearanceToString
 * @Desc : The following function is used to map a Appearance Value to it's
 *         string representation.
 */
static boolean_t AppearanceToString(uint16_t Appearance, char **String)
{
    boolean_t    ret_val;
    unsigned int Index;

    if (String)
    {
        for (Index=0,ret_val=FALSE;Index<NUMBER_OF_APPEARANCE_MAPPINGS;++Index)
        {
            if (AppearanceMappings[Index].Appearance == Appearance)
            {
                *String = AppearanceMappings[Index].String;
                ret_val = TRUE;
                break;
            }
        }
    }
    else
        ret_val = FALSE;
    return (ret_val);
}

/**
 * @func : InitializeBuffer
 * @Desc : The following function is used to initialize the specified buffer
 *         to the defaults
 */
static void InitializeBuffer(Onboard_Data_Buffer_t *DataBuffer)
{
    if (DataBuffer)
    {
        DataBuffer->BufferSize = ONBOARD_DATA_CREDITS;
        DataBuffer->BytesFree  = ONBOARD_DATA_CREDITS;
        DataBuffer->InIndex    = 0;
        DataBuffer->OutIndex   = 0;
    }
}

/**
 * @func : GATT_ClientEventCallback_GAPS
 * @Desc : This function will be called whenever a GATT Response is received for
 *         request that was made when this function was registered
 */
static void QAPI_BLE_BTPSAPI GATT_ClientEventCallback_GAPS(uint32_t BluetoothStackID, qapi_BLE_GATT_Client_Event_Data_t *GATT_Client_Event_Data, uint32_t CallbackParameter)
{
    boolean_t     DisplayPrompt;
    char         *NameBuffer;
    uint16_t      Appearance;
    BoardStr_t    BoardStr;
    DeviceInfo_t *DeviceInfo;

    if ((BluetoothStackID) && (GATT_Client_Event_Data))
    {
        DisplayPrompt = true;

        switch (GATT_Client_Event_Data->Event_Data_Type)
        {
            case QAPI_BLE_ET_GATT_CLIENT_ERROR_RESPONSE_E:
                if (GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data)
                {
                    LOG_ERROR("Error Response.\n");
                    BD_ADDRToStr(GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->RemoteDevice, BoardStr);
                    LOG_ERROR("Connection ID:   %u.\n", GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ConnectionID);
                    LOG_ERROR("Transaction ID:  %u.\n", GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->TransactionID);
                    LOG_ERROR("Connection Type: %s.\n", (GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ConnectionType == QAPI_BLE_GCT_LE_E)?"LE":"BR/EDR");
                    LOG_ERROR("BD_ADDR:         %s.\n", BoardStr);
                    LOG_ERROR("Error Type:      %s.\n", (GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorType == QAPI_BLE_RET_ERROR_RESPONSE_E)?"Response Error":"Response Timeout");

                    if (GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorType == QAPI_BLE_RET_ERROR_RESPONSE_E)
                    {
                        LOG_ERROR("Request Opcode:  0x%02X.\n", GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->RequestOpCode);
                        LOG_ERROR("Request Handle:  0x%04X.\n", GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->RequestHandle);
                        LOG_ERROR("Error Code:      0x%02X.\n", GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorCode);
                        LOG_ERROR("Error Mesg:      %s.\n", ErrorCodeStr[GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorCode]);
                    }
                }
                else
                    LOG_ERROR("Error - Null Error Response Data.\n");
                break;
            case QAPI_BLE_ET_GATT_CLIENT_EXCHANGE_MTU_RESPONSE_E:
                if(GATT_Client_Event_Data->Event_Data.GATT_Exchange_MTU_Response_Data)
                {
                    LOG_VERBOSE("Exchange MTU Response.\n");
                    BD_ADDRToStr(GATT_Client_Event_Data->Event_Data.GATT_Exchange_MTU_Response_Data->RemoteDevice, BoardStr);
                    LOG_VERBOSE("Connection ID:   %u.\n", GATT_Client_Event_Data->Event_Data.GATT_Exchange_MTU_Response_Data->ConnectionID);
                    LOG_VERBOSE("Transaction ID:  %u.\n", GATT_Client_Event_Data->Event_Data.GATT_Exchange_MTU_Response_Data->TransactionID);
                    LOG_VERBOSE("Connection Type: %s.\n", (GATT_Client_Event_Data->Event_Data.GATT_Exchange_MTU_Response_Data->ConnectionType == QAPI_BLE_GCT_LE_E)?"LE":"BR/EDR");
                    LOG_VERBOSE("BD_ADDR:         %s.\n", BoardStr);
                    LOG_VERBOSE("MTU:             %u.\n", GATT_Client_Event_Data->Event_Data.GATT_Exchange_MTU_Response_Data->ServerMTU);
                }
                else
                {
                    LOG_ERROR("Error - Null Write Response Data.\n");
                }
                break;
            case QAPI_BLE_ET_GATT_CLIENT_READ_RESPONSE_E:
                if (GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data)
                {
                    DisplayPrompt = false;
                    if ((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->RemoteDevice)) != NULL)
                    {
                        if ((uint16_t)CallbackParameter == DeviceInfo->GAPSClientInfo.DeviceNameHandle)
                        {
                            if ((NameBuffer = (char *)malloc(GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength+1)) != NULL)
                            {
                                memset (NameBuffer, 0, GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength+1);
                                memcpy (NameBuffer, GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValue, GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength);
                                LOG_VERBOSE("Remote Device Name: %s.\n", NameBuffer);
                                DisplayPrompt = true;

                                free(NameBuffer);
                            }
                        }
                        else
                        {
                            if ((uint16_t)CallbackParameter == DeviceInfo->GAPSClientInfo.DeviceAppearanceHandle)
                            {
                                if (GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength == QAPI_BLE_GAP_DEVICE_APPEARANCE_VALUE_LENGTH)
                                {
                                    Appearance = READ_UNALIGNED_WORD_LITTLE_ENDIAN(GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValue);
                                    if (AppearanceToString(Appearance, &NameBuffer))
                                        LOG_VERBOSE("Remote Device Appearance: %s(%u).\n", NameBuffer, Appearance);
                                    else
                                        LOG_VERBOSE("Remote Device Appearance: Unknown(%u).\n", Appearance);
                                }
                                else
                                    LOG_VERBOSE("Invalid Remote Appearance Value Length %u.\n", GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength);

                                DisplayPrompt = true;
                            }
                        }
                    }
                }
                else
                {
                    LOG_ERROR("Error - Null Read Response Data.\n");
                }
                break;
            default:
                break;
        }

        if (DisplayPrompt)
            QCLI_Display_Prompt();
    }
}

/**
 * @func : GATT_Connection_Event_Callback
 * @Desc : Generic Attribute Profile (GATT) Event Callback function
 *         prototypes.The following function is for an GATT Connection Event Callback.
 *         This function is called for GATT Connection Events that occur on
 *         the specified Bluetooth Stack.
 */
static void QAPI_BLE_BTPSAPI GATT_Connection_Event_Callback(uint32_t BluetoothStackID, qapi_BLE_GATT_Connection_Event_Data_t *GATT_Connection_Event_Data, uint32_t CallbackParameter)
{
    boolean_t                    DisplayPrompt;
    int                          Result;
    uint16_t                     MTU;
    BoardStr_t                   BoardStr;
    DeviceInfo_t                *DeviceInfo = NULL;
    uint32_t                     ConnectionID;
    boolean_t                    FoundMatch = FALSE;

    if ((BluetoothStackID) && (GATT_Connection_Event_Data))
    {
        DisplayPrompt = true;

        switch (GATT_Connection_Event_Data->Event_Data_Type)
        {
            case QAPI_BLE_ET_GATT_CONNECTION_DEVICE_CONNECTION_E:
                if (GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data)
                {
                    ConnectionID = GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data->ConnectionID;
                    LOG_INFO("etGATT_Connection_Device_Connection with size %u: \n", GATT_Connection_Event_Data->Event_Data_Size);
                    BD_ADDRToStr(GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data->RemoteDevice, BoardStr);
                    LOG_INFO("   Connection ID:   %u.\n", GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data->ConnectionID);
                    LOG_INFO("   Connection Type: %s.\n", ((GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data->ConnectionType == QAPI_BLE_GCT_LE_E)?"LE":"BR/EDR"));
                    LOG_INFO("   Remote Device:   %s.\n", BoardStr);
                    LOG_INFO("   Connection MTU:  %u.\n", GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data->MTU);
                    if ((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data->RemoteDevice)) == NULL)
                    {
                        if ((DeviceInfo = CreateNewDeviceInfoEntry(&DeviceInfoList, GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data->RemoteDevice)) == NULL)
                            LOG_ERROR("Failed to create remote device information.\n");
                    }

                    if (DeviceInfo)
                    {
                        ConnectionCount++;
                        SelectedRemoteBD_ADDR = GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data->RemoteDevice;
                        LOG_VERBOSE("\nSelected Remote Device:\n");
                        LOG_VERBOSE("   Address:       %s\n", BoardStr);
                        LOG_VERBOSE("   ID:            %u\n", ConnectionID);
                        DeviceInfo->RemoteDeviceIsMaster = (LocalDeviceIsMaster) ? FALSE : TRUE;
                        DeviceInfo->RemoteAddressType    = RemoteAddressType;
                        DeviceInfo->ConnectionID         = ConnectionID;

                        InitializeBuffer(&(DeviceInfo->ReceiveBuffer));
                        InitializeBuffer(&(DeviceInfo->TransmitBuffer));
                        DeviceInfo->TransmitCredits = 0;

                        if (!qapi_BLE_GATT_Query_Maximum_Supported_MTU(BluetoothStackID, &MTU))
                            qapi_BLE_GATT_Exchange_MTU_Request(BluetoothStackID, DeviceInfo->ConnectionID, MTU, GATT_ClientEventCallback_GAPS, 0);
                    }
                    else
                    {
                        if ((Result = qapi_BLE_GAP_LE_Disconnect(BluetoothStackID, GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data->RemoteDevice)) != 0)
                            LOG_ERROR("qapi_BLE_GAP_LE_Disconnect returned %d.\n", Result);
                    }
                }
                else
                    LOG_ERROR("Error - Null Connection Data.\n");
                break;
            case QAPI_BLE_ET_GATT_CONNECTION_DEVICE_DISCONNECTION_E:
                if (GATT_Connection_Event_Data->Event_Data.GATT_Device_Disconnection_Data)
                {
                    LOG_INFO("etGATT_Connection_Device_Disconnection with size %u: \n", GATT_Connection_Event_Data->Event_Data_Size);
                    BD_ADDRToStr(GATT_Connection_Event_Data->Event_Data.GATT_Device_Disconnection_Data->RemoteDevice, BoardStr);
                    LOG_INFO("   Connection ID:   %u.\n", GATT_Connection_Event_Data->Event_Data.GATT_Device_Disconnection_Data->ConnectionID);
                    LOG_INFO("   Connection Type: %s.\n", ((GATT_Connection_Event_Data->Event_Data.GATT_Device_Disconnection_Data->ConnectionType == QAPI_BLE_GCT_LE_E)?"LE":"BR/EDR"));
                    LOG_INFO("   Remote Device:   %s.\n", BoardStr);
                    if ((ConnectionCount) && (DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, GATT_Connection_Event_Data->Event_Data.GATT_Device_Disconnection_Data->RemoteDevice)) != NULL)
                    {
                        ConnectionCount--;

                        if (DeviceInfo->Flags & DEVICE_INFO_FLAGS_LTK_VALID)
                        {
                            DeviceInfo->Flags &= ~DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_OUTSTANDING;
                            DeviceInfo->ConnectionID = 0;
                        }
                        else
                        {
                            if ((DeviceInfo = DeleteDeviceInfoEntry(&DeviceInfoList, SelectedRemoteBD_ADDR)) != NULL)
                            {
                                LOG_VERBOSE("\nThe remote device information has been deleted.\n", BoardStr);
                                DeviceInfo = NULL;
                            }
                        }
                    }

                    if (ConnectionCount)
                    {
                        DeviceInfo = DeviceInfoList;
                        while (DeviceInfo)
                        {
                            if (DeviceInfo->ConnectionID)
                            {
                                SelectedRemoteBD_ADDR = DeviceInfo->RemoteAddress;
                                LOG_INFO("\nSelected Remote Device:\n");
                                BD_ADDRToStr(DeviceInfo->RemoteAddress, BoardStr);
                                LOG_INFO("   Address:       %s\n", BoardStr);
                                LOG_INFO("   ID:            %u\n", DeviceInfo->ConnectionID);
                                break;
                            }

                            DeviceInfo = DeviceInfo->NextDeviceInfoInfoPtr;
                        }
                    }
                }
                else
                    LOG_ERROR("Error - Null Disconnection Data.\n");
                break;
            case QAPI_BLE_ET_GATT_CONNECTION_SERVER_NOTIFICATION_E:
                if (GATT_Connection_Event_Data->Event_Data.GATT_Server_Notification_Data)
                {
                    DisplayPrompt = false;
                    if ((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, GATT_Connection_Event_Data->Event_Data.GATT_Server_Notification_Data->RemoteDevice)) != NULL)
                    {
                        if (FoundMatch == TRUE)
                        {
                            break;
                        }
                        if (FoundMatch == TRUE)
                        {
                            break;
                        }
                        if (FoundMatch == TRUE)
                        {
                            break;
                        }

                        LOG_ERROR("Error - Unknown attribute handle for notification.\n");
                        DisplayPrompt = true;
                    }
                }
                else
                    LOG_ERROR("Error - Null Server Notification Data.\n");
                break;
            default:
                DisplayPrompt = false;
                break;
        }

        if (DisplayPrompt)
            QCLI_Display_Prompt();
    }
}

/**
 * @func : OpenStack
 * @Desc : The following function is responsible for opening the SS1
 *         Bluetooth Protocol Stack.  This function accepts a pre-populated
 *         HCI Driver Information structure that contains the HCI Driver
 *         Transport Information.  This function returns zero on successful
 *         execution and a negative value on all errors.
 */
static int OpenStack(qapi_BLE_HCI_DriverInformation_t *HCI_DriverInformation)
{
    int                    Result;
    int                    ret_val = 0;
    char                   BluetoothAddress[16];
    uint32_t               ServiceID;
    uint8_t                HC_SCO_Data_Packet_Length;
    uint16_t               HC_Total_Num_SCO_Data_Packets;
    uint16_t               HC_Total_Num_LE_Data_Packets;
    uint16_t               HC_LE_Data_Packet_Length;
    uint8_t                TempData;
    uint8_t                Status;
    uint32_t               passkey = BLE_PASSKEY; /*Setting the default passkey*/
    char                   BLE_NAME[BLE_NAME_SIZE] = "";

    if (!BluetoothStackID)
    {
        if(HCI_DriverInformation)
        {
            LOG_INFO("OpenStack().\n");

            Result = qapi_BLE_BSC_Initialize(HCI_DriverInformation, 0);
            if ( Result > 0 )
            {
                BluetoothStackID = Result;
                LOG_INFO("Bluetooth Stack ID: %d.\n", BluetoothStackID);

                LE_Parameters.IOCapability      = QAPI_BLE_LIC_DISPLAY_ONLY_E;
                LE_Parameters.OOBDataPresent    = FALSE;
                LE_Parameters.MITMProtection    = (LE_Parameters.IOCapability != QAPI_BLE_LIC_NO_INPUT_NO_OUTPUT_E) ? DEFAULT_MITM_PROTECTION : FALSE;
                LE_Parameters.SecureConnections = DEFAULT_SECURE_CONNECTIONS;

                qapi_BLE_GAP_LE_Set_Fixed_Passkey(BluetoothStackID,&passkey);  //PASSKEY is set to default value

                Result = qapi_BLE_HCI_LE_Read_Buffer_Size(BluetoothStackID, &Status, &HC_LE_Data_Packet_Length, &TempData);

                if ((Result) || (Status != 0) || (!HC_LE_Data_Packet_Length) || (!TempData))
                {
                    if ((!qapi_BLE_HCI_Read_Buffer_Size(BluetoothStackID, &Status, &HC_LE_Data_Packet_Length, &HC_SCO_Data_Packet_Length, &HC_Total_Num_LE_Data_Packets, &HC_Total_Num_SCO_Data_Packets)) && (!Status))
                        Result = 0;
                    else
                        Result = -1;
                }
                else
                {
                    HC_Total_Num_LE_Data_Packets = (uint16_t)TempData;
                    Result                       = 0;
                }

                NumberACLPackets            = HC_Total_Num_LE_Data_Packets;
                NumberOutstandingACLPackets = 0;
                MaxACLPacketSize            = HC_LE_Data_Packet_Length;

                if (!Result)
                    LOG_VERBOSE("Number ACL Buffers: %d, ACL Buffer Size: %d\n", NumberACLPackets, MaxACLPacketSize);
                else
                    LOG_ERROR("Unable to determine ACL Buffer Size.\n");

                if (!qapi_BLE_GAP_Query_Local_BD_ADDR(BluetoothStackID, &LocalBD_ADDR))
                {
                    BD_ADDRToStr(LocalBD_ADDR, BluetoothAddress);

                    LOG_INFO("BD_ADDR: %s\n", BluetoothAddress);
                }

                get_ble_localdevice_name(BLE_NAME, sizeof(BLE_NAME));

                QAPI_BLE_ASSIGN_BD_ADDR(SelectedRemoteBD_ADDR, 0, 0, 0, 0, 0, 0);
                QAPI_BLE_ASSIGN_BD_ADDR(SecurityRemoteBD_ADDR, 0, 0, 0, 0, 0, 0);

                qapi_BLE_GAP_LE_Diversify_Function(BluetoothStackID, (qapi_BLE_Encryption_Key_t *)(&IR), 1,0, &IRK);
                qapi_BLE_GAP_LE_Diversify_Function(BluetoothStackID, (qapi_BLE_Encryption_Key_t *)(&IR), 3, 0, &DHK);

                DeviceInfoList = NULL;

                Result = qapi_Persist_Initialize(&PersistHandle, "/spinor/ble", "ble_data", ".bin", NULL, 0);
                if (Result < 0)
                    LOG_ERROR("Persistent Storage Initialization Failed: %d\n", Result);

                /* Initialize the GATT Service. */
                if ((Result = qapi_BLE_GATT_Initialize(BluetoothStackID, (QAPI_BLE_GATT_INITIALIZATION_FLAGS_SUPPORT_LE | QAPI_BLE_GATT_INITIALIZATION_FLAGS_DISABLE_SERVICE_CHANGED_CHARACTERISTIC), GATT_Connection_Event_Callback, 0)) == 0)
                {
                    /* Initialize the DIS Service. */
                    Result = qapi_BLE_DIS_Initialize_Service(BluetoothStackID, &ServiceID);
                    if (Result > 0)
                    {
                        /* Save the DIS Instance ID. */
                        DISInstanceID = (unsigned int)Result;

                        qapi_BLE_DIS_Set_Manufacturer_Name(BluetoothStackID, DISInstanceID, MANUFACTURER_NAME);
                        qapi_BLE_DIS_Set_Model_Number(BluetoothStackID, DISInstanceID, BLE_MODEL_NUMBER);
                        qapi_BLE_DIS_Set_Software_Revision(BluetoothStackID, DISInstanceID, BLE_SW_REVISION);
                        qapi_BLE_DIS_Set_Hardware_Revision(BluetoothStackID, DISInstanceID, BLE_HW_REVISION);
                        qapi_BLE_DIS_Set_Firmware_Revision(BluetoothStackID, DISInstanceID, BLE_FW_REVISION);
                    }
                    else
                        DisplayFunctionError("DIS_Initialize_Service()", Result);

                    Result = qapi_BLE_GAPS_Initialize_Service(BluetoothStackID, &ServiceID);
                    if (Result > 0)
                    {
                        GAPSInstanceID = (unsigned int)Result;

                        qapi_BLE_GAPS_Set_Device_Name(BluetoothStackID, GAPSInstanceID, BLE_NAME);
                        qapi_BLE_GAPS_Set_Device_Appearance(BluetoothStackID, GAPSInstanceID, QAPI_BLE_GAP_DEVICE_APPEARANCE_VALUE_GENERIC_COMPUTER);

                        Result = qapi_BLE_GAP_LE_Set_Address_Resolution_Enable(BluetoothStackID, TRUE);
                        if (!Result)
                        {
                            qapi_BLE_GAPS_Set_Central_Address_Resolution(BluetoothStackID, GAPSInstanceID, QAPI_BLE_GAR_ENABLED_E);
                            Result = qapi_BLE_GAP_LE_Set_Resolvable_Private_Address_Timeout(BluetoothStackID, 60);
                            if (Result)
                                LOG_ERROR("Error - qapi_BLE_GAP_LE_Set_Resolvable_Private_Address_Timeout() returned %d.\n", Result);
                        }
                        else
                            LOG_ERROR("Error - qapi_BLE_GAP_LE_Set_Address_Resolution_Enable() returned %d.\n", Result);
                    }
                    else
                        DisplayFunctionError("qapi_BLE_GAPS_Initialize_Service()", Result);

                    //               Result = qapi_BLE_SLoWP_Initialize(BluetoothStackID);

                    ret_val = 0;
                }
                else
                {
                    DisplayFunctionError("qapi_BLE_GATT_Initialize()", Result);

                    qapi_BLE_BSC_Shutdown(BluetoothStackID);
                    BluetoothStackID = 0;
                    ret_val          = -1;
                }
            }
            else
            {
                DisplayFunctionError("qapi_BLE_BSC_Initialize()", Result);
                BluetoothStackID = 0;
                ret_val          = -1;
            }
        }
        else
        {
            ret_val = -1;
        }
    }
    else
    {
        ret_val = 0;
    }

    return (ret_val);
}

/* The following function is responsible for placing the local       */
/* Bluetooth device into Pairable mode.  Once in this mode the device*/
/* will response to pairing requests from other Bluetooth devices.   */
/* This function returns zero on successful execution and a negative */
/* value on all errors.                                              */
static int SetPairable(void)
{
    int Result;
    int ret_val = 0;

    /* First, check that a valid Bluetooth Stack ID exists.              */
    if(BluetoothStackID)
    {
        /* Attempt to set the attached device to be pairable.             */
        Result = qapi_BLE_GAP_LE_Set_Pairability_Mode(BluetoothStackID, QAPI_BLE_LPM_PAIRABLE_MODE_ENABLE_EXTENDED_EVENTS_E);

        /* Next, check the return value of the GAP Set Pairability mode   */
        /* command for successful execution.                              */
        if(!Result)
        {
            /* The device has been set to pairable mode, now register an   */
            /* Authentication Callback to handle the Authentication events */
            /* if required.                                                */
            Result = qapi_BLE_GAP_LE_Register_Remote_Authentication(BluetoothStackID, GAP_LE_Event_Callback, (unsigned long)0);

            /* Next, check the return value of the GAP Register Remote     */
            /* Authentication command for successful execution.            */
            if(Result)
            {
                /* An error occurred while trying to execute this function. */
                DisplayFunctionError("GAP_LE_Register_Remote_Authentication", Result);

                ret_val = Result;
            }
        }
        else
        {
            /* An error occurred while trying to make the device pairable. */
            DisplayFunctionError("GAP_LE_Set_Pairability_Mode", Result);

            ret_val = Result;
        }
    }
    else
    {
        /* No valid Bluetooth Stack ID exists.                            */
        ret_val = QCLI_STATUS_ERROR_E;
    }

    return(ret_val);
}

/* Generic Access Profile (GAPLE) helper functions.                  */

/* The following function is responsible for placing the Local       */
/* Bluetooth Device into General Discoverablity Mode.  Once in this  */
/* mode the Device will respond to Inquiry Scans from other Bluetooth*/
/* Devices.  This function requires that a valid Bluetooth Stack ID  */
/* exists before running.  This function returns zero on successful  */
/* execution and a negative value if an error occurred.              */
static int SetDisc(void)
{
    int ret_val = 0;

    /* First, check that a valid Bluetooth Stack ID exists.              */
    if(BluetoothStackID)
    {
        /* * NOTE * Discoverability is only applicable when we are        */
        /*          advertising so save the default Discoverability Mode  */
        /*          for later.                                            */
        LE_Parameters.DiscoverabilityMode = QAPI_BLE_DM_GENERAL_DISCOVERABLE_MODE_E;
    }
    else
    {
        /* No valid Bluetooth Stack ID exists.                            */
        ret_val = QCLI_STATUS_ERROR_E;
    }

    return(ret_val);
}

/* The following function is responsible for placing the Local       */
/* Bluetooth Device into Connectable Mode.  Once in this mode the    */
/* Device will respond to Page Scans from other Bluetooth Devices.   */
/* This function requires that a valid Bluetooth Stack ID exists     */
/* before running.  This function returns zero on success and a      */
/* negative value if an error occurred.                              */
static int SetConnect(void)
{
    int ret_val = 0;

    /* First, check that a valid Bluetooth Stack ID exists.              */
    if(BluetoothStackID)
    {
        /* * NOTE * Connectability is only an applicable when advertising */
        /*          so we will just save the default connectability for   */
        /*          the next time we enable advertising.                  */
        LE_Parameters.ConnectableMode = QAPI_BLE_LCM_CONNECTABLE_E;
    }
    else
    {
        /* No valid Bluetooth Stack ID exists.                            */
        ret_val = QCLI_STATUS_ERROR_E;
    }

    return(ret_val);
}

static int CloseStack(void);
int close_stack(void)
{
    return CloseStack();
}
/**
 * @func : CloseStack
 * @Desc : The following function is responsible for closing the SS1
 *         Bluetooth Protocol Stack.  This function requires that the
 *         Bluetooth Protocol stack previously have been initialized via the
 *         OpenStack() function.
 */
static int CloseStack(void)
{
    int           ret_val = 0;
    DeviceInfo_t *DeviceInfo;

    if (BluetoothStackID)
    {
        /* If there are remote devices connected.                         */
        if(ConnectionCount)
        {
            /* Go ahead and flag that we are no longer connected to any    */
            /* remote devices.                                             */
            ConnectionCount = 0;

            /* Lock the Bluetooth stack.                                   */
            if(!qapi_BLE_BSC_LockBluetoothStack(BluetoothStackID))
            {
                /* We need to loop through the remote device information    */
                /* entries and disconnect any remote devices that are still */
                /* connected.                                               */
                DeviceInfo = DeviceInfoList;
                while(DeviceInfo)
                {
                    /* If the GATT Connection ID is valid, then we are       */
                    /* connected to the remote device.                       */
                    if(DeviceInfo->ConnectionID)
                    {
                        /* Flag that the remote device is no longer connected.*/
                        DeviceInfo->ConnectionID = 0;

                        /* Send the disconnection request.                    */
                        qapi_BLE_GAP_LE_Disconnect(BluetoothStackID, DeviceInfo->RemoteAddress);
                    }

                    DeviceInfo = DeviceInfo->NextDeviceInfoInfoPtr;
                }

                /* Un-lock the Bluetooth Stack.                             */
                qapi_BLE_BSC_UnLockBluetoothStack(BluetoothStackID);
            }
        }

        if (DISInstanceID)
        {
            qapi_BLE_DIS_Cleanup_Service(BluetoothStackID, DISInstanceID);

            DISInstanceID = 0;
        }

        if (GAPSInstanceID)
        {
            qapi_BLE_GAPS_Cleanup_Service(BluetoothStackID, GAPSInstanceID);

            GAPSInstanceID = 0;
        }

        /* Release onboard service*/
        Cleanup_wifi_service();
        Cleanup_zigbee_service();

        /* Release the storage instance.                                  */
        qapi_Persist_Cleanup(PersistHandle);
        PersistHandle = NULL;

        /* Cleanup GATT Module.                                           */
        qapi_BLE_GATT_Cleanup(BluetoothStackID);

        /* Simply close the Stack                                         */
        qapi_BLE_BSC_Shutdown(BluetoothStackID);

        /* Inform the user that the stack has been shut down.             */
        LOG_INFO("Stack Shutdown.\n");

        /* Free all remote device information entries.                    */
        //FreeDeviceInfoList(&DeviceInfoList);
        DeviceInfoList   = NULL;

        /* Flag that the Stack is no longer initialized.                  */
        BluetoothStackID = 0;
        RemoteOOBValid   = FALSE;
        LocalOOBValid    = FALSE;

        /* Flag success to the caller.                                    */
        ret_val          = 0;
    }
    else
    {
        ret_val = -1;
    }

    return (ret_val);
}

/**
 * @func : InitializeBluetooth
 * @Desc : The following function is responsible for initializing the stack
 */
int InitializeBluetooth(void)
{
    int Result;
    int ret_val;

    QAPI_BLE_HCI_DRIVER_SET_COMM_INFORMATION(&HCI_DriverInformation, 1, 115200, QAPI_BLE_COMM_PROTOCOL_UART_E);

    /* First, check that the stack is not currently initialized.         */
    if(!BluetoothStackID)
    {
        /* Attempt to open the stack.                                     */
        Result = OpenStack(&HCI_DriverInformation);
        if(!Result)
        {
            /* Set the default pairability.                                */
            Result = SetPairable();
            if(!Result)
            {
                /* Set the default discoverability.                         */
                Result = SetDisc();
                if(!Result)
                {
                    /* Set the default connectability.                       */
                    Result = SetConnect();
                }
            }

            /* If the failure occurred after the stack initialized then    */
            /* shut it down.                                               */
            if(Result)
                CloseStack();
        }

        /* Set the QCLI error type appropriately.                         */
        if(!Result)
            ret_val = QCLI_STATUS_SUCCESS_E;
        else
            ret_val = QCLI_STATUS_ERROR_E;
    }
    else
    {
        /* No valid Bluetooth Stack ID exists.                            */
        LOG_WARN("Bluetooth stack is already initialized.");

        ret_val = QCLI_STATUS_SUCCESS_E;
    }

    return(ret_val);
}

/**
 * @func : SearchDeviceInfoEntryTypeAddress
 * @Desc : The following function searches the specified List for the
 *         specified Address and Type.  This function returns NULL if
 *         either the List Head is invalid, the BD_ADDR is invalid, or the
 *         Connection BD_ADDR was NOT found.
 */
static DeviceInfo_t *SearchDeviceInfoEntryTypeAddress(DeviceInfo_t **ListHead, qapi_BLE_GAP_LE_Address_Type_t AddressType, qapi_BLE_BD_ADDR_t RemoteAddress)
{
    BoardStr_t                      BoardStr;
    DeviceInfo_t                   *ret_val = NULL;
    DeviceInfo_t                   *DeviceInfo;
    qapi_BLE_GAP_LE_Address_Type_t  TempType;

    if (ListHead)
    {
        DeviceInfo = *ListHead;
        while (DeviceInfo)
        {
            if ((DeviceInfo->RemoteAddressType == AddressType) && (QAPI_BLE_COMPARE_BD_ADDR(DeviceInfo->RemoteAddress, RemoteAddress)))
            {
                ret_val = DeviceInfo;
                break;
            }
            else
            {
                if ((AddressType == QAPI_BLE_LAT_PUBLIC_IDENTITY_E) || (AddressType == QAPI_BLE_LAT_RANDOM_IDENTITY_E))
                {
                    if (AddressType == QAPI_BLE_LAT_PUBLIC_IDENTITY_E)
                        TempType = QAPI_BLE_LAT_PUBLIC_E;
                    else
                        TempType = QAPI_BLE_LAT_RANDOM_E;
                    if (DeviceInfo->Flags & DEVICE_INFO_FLAGS_IRK_VALID)
                    {
                        if ((DeviceInfo->IdentityAddressType == TempType) && (QAPI_BLE_COMPARE_BD_ADDR(DeviceInfo->IdentityAddressBD_ADDR, RemoteAddress)))
                        {
                            DeviceInfo->RemoteAddressType = AddressType;
                            DeviceInfo->RemoteAddress     = DeviceInfo->IdentityAddressBD_ADDR;

                            ret_val = DeviceInfo;
                            break;
                        }
                    }
                }
                else
                {
                    if ((AddressType == QAPI_BLE_LAT_RANDOM_E) && (QAPI_BLE_GAP_LE_TEST_RESOLVABLE_ADDRESS_BITS(RemoteAddress)))
                    {
                        if (DeviceInfo->Flags & DEVICE_INFO_FLAGS_IRK_VALID)
                        {
                            if (qapi_BLE_GAP_LE_Resolve_Address(BluetoothStackID, &(DeviceInfo->IRK), RemoteAddress))
                            {
                                DeviceInfo->RemoteAddress     = RemoteAddress;
                                DeviceInfo->RemoteAddressType = QAPI_BLE_LAT_RANDOM_E;

                                LOG_VERBOSE("\n");
                                LOG_VERBOSE("Resolved Address (");
                                BD_ADDRToStr(DeviceInfo->RemoteAddress, BoardStr);
                                LOG_VERBOSE("%s", BoardStr);
                                LOG_VERBOSE(")\n");
                                LOG_VERBOSE("   Identity Address:       ");
                                BD_ADDRToStr(DeviceInfo->IdentityAddressBD_ADDR, BoardStr);
                                LOG_VERBOSE("%s\n", BoardStr);
                                LOG_VERBOSE("   Identity Address Type:  %s\n", ((DeviceInfo->IdentityAddressType == QAPI_BLE_LAT_PUBLIC_IDENTITY_E) ? "Public Identity" : "Random Identity"));

                                ret_val = DeviceInfo;
                                break;
                            }
                        }
                    }
                }
            }

            DeviceInfo = DeviceInfo->NextDeviceInfoInfoPtr;
        }
    }

    return (ret_val);
}

/**
 * @func : ConfigureCapabilities
 * @Desc : This Function Configures for the Bonding(Pair) information
 */
static void ConfigureCapabilities(qapi_BLE_GAP_LE_Extended_Pairing_Capabilities_t *Capabilities)
{
    /* Make sure the Capabilities pointer is semi-valid.                 */
    if(Capabilities)
    {
        /* Initialize the capabilities.                                   */
        memset(Capabilities, 0, QAPI_BLE_GAP_LE_EXTENDED_PAIRING_CAPABILITIES_SIZE);

        /* Configure the Pairing Capabilities structure.                  */
        Capabilities->Bonding_Type                    = QAPI_BLE_LBT_BONDING_E;
        Capabilities->IO_Capability                   = LE_Parameters.IOCapability;
        Capabilities->Flags                           = 0;

        if(LE_Parameters.MITMProtection)
            Capabilities->Flags |= QAPI_BLE_GAP_LE_EXTENDED_PAIRING_CAPABILITIES_FLAGS_MITM_REQUESTED;

        if(LE_Parameters.SecureConnections)
            Capabilities->Flags |= QAPI_BLE_GAP_LE_EXTENDED_PAIRING_CAPABILITIES_FLAGS_SECURE_CONNECTIONS;

        if(RemoteOOBValid)
            Capabilities->Flags |= QAPI_BLE_GAP_LE_EXTENDED_PAIRING_CAPABILITIES_FLAGS_OOB_DATA_PRESENT;

        if(LocalOOBValid)
        {
            Capabilities->Flags                     |= QAPI_BLE_GAP_LE_EXTENDED_PAIRING_CAPABILITIES_FLAGS_LOCAL_OOB_DATA_VALID;
            Capabilities->LocalOOBData.Flags         = 0;
            Capabilities->LocalOOBData.Confirmation  = LocalOOBConfirmation;
            Capabilities->LocalOOBData.Randomizer    = LocalOOBRandomizer;
        }

        /* ** NOTE ** This application always requests that we use the    */
        /*            maximum encryption because this feature is not a    */
        /*            very good one, if we set less than the maximum we   */
        /*            will internally in GAP generate a key of the        */
        /*            maximum size (we have to do it this way) and then   */
        /*            we will zero out how ever many of the MSBs          */
        /*            necessary to get the maximum size.  Also as a slave */
        /*            we will have to use Non-Volatile Memory (per device */
        /*            we are paired to) to store the negotiated Key Size. */
        /*            By requesting the maximum (and by not storing the   */
        /*            negotiated key size if less than the maximum) we    */
        /*            allow the slave to power cycle and regenerate the   */
        /*            LTK for each device it is paired to WITHOUT storing */
        /*            any information on the individual devices we are    */
        /*            paired to.                                          */
        Capabilities->Maximum_Encryption_Key_Size        = QAPI_BLE_GAP_LE_MAXIMUM_ENCRYPTION_KEY_SIZE;

        /* This application only demonstrates using the Long Term Key's   */
        /* (LTK) for encryption of a LE Link and the Identity Resolving   */
        /* Key (IRK) for resolving resovable private addresses (RPA's),   */
        /* however we could request and send all possible keys here if we */
        /* wanted to.                                                     */
        Capabilities->Receiving_Keys.Encryption_Key     = TRUE;
        Capabilities->Receiving_Keys.Identification_Key = TRUE;
        Capabilities->Receiving_Keys.Signing_Key        = FALSE;
        Capabilities->Receiving_Keys.Link_Key           = FALSE;

        Capabilities->Sending_Keys.Encryption_Key       = TRUE;
        Capabilities->Sending_Keys.Identification_Key   = TRUE;
        Capabilities->Sending_Keys.Signing_Key          = FALSE;
        Capabilities->Sending_Keys.Link_Key             = FALSE;
    }
}

/**
 * @func : SlavePairingRequestResponse
 * @Desc : Function Responsible to Send the Pairing response
 */
static int SlavePairingRequestResponse(qapi_BLE_BD_ADDR_t BD_ADDR)
{
    int                                                   ret_val;
    BoardStr_t                                            BoardStr;
    qapi_BLE_GAP_LE_Authentication_Response_Information_t AuthenticationResponseData;

    if (BluetoothStackID)
    {
        BD_ADDRToStr(BD_ADDR, BoardStr);
        LOG_INFO("Sending Pairing Response to %s.\n", BoardStr);

        AuthenticationResponseData.GAP_LE_Authentication_Type = QAPI_BLE_LAR_PAIRING_CAPABILITIES_E;
        AuthenticationResponseData.Authentication_Data_Length = QAPI_BLE_GAP_LE_EXTENDED_PAIRING_CAPABILITIES_SIZE;

        ConfigureCapabilities(&(AuthenticationResponseData.Authentication_Data.Extended_Pairing_Capabilities));
        if ((ret_val = qapi_BLE_GAP_LE_Authentication_Response(BluetoothStackID, BD_ADDR, &AuthenticationResponseData)) == QAPI_BLE_BTPS_ERROR_SECURE_CONNECTIONS_NOT_SUPPORTED)
        {
            LOG_WARN("Secure Connections not supported, disabling Secure Connections.\n");

            AuthenticationResponseData.Authentication_Data.Extended_Pairing_Capabilities.Flags &= ~QAPI_BLE_GAP_LE_EXTENDED_PAIRING_CAPABILITIES_FLAGS_SECURE_CONNECTIONS;

            ret_val = qapi_BLE_GAP_LE_Authentication_Response(BluetoothStackID, BD_ADDR, &AuthenticationResponseData);
        }
        LOG_INFO("GAP_LE_Authentication_Response returned %d.\n", ret_val);
    }
    else
    {
        LOG_ERROR("Stack ID Invalid.\n");
        ret_val = -1;
    }

    return (ret_val);
}

/**
 * @func : DisplayLegacyPairingInformation
 * @Desc :  The following function displays the pairing capabilities that is
 *          passed into this function.
 */
static void DisplayLegacyPairingInformation(qapi_BLE_GAP_LE_Pairing_Capabilities_t *Pairing_Capabilities)
{
    switch (Pairing_Capabilities->IO_Capability)
    {
        case QAPI_BLE_LIC_DISPLAY_ONLY_E:
            LOG_VERBOSE("   IO Capability:       Display Only.\n");
            break;
        case QAPI_BLE_LIC_DISPLAY_YES_NO_E:
            LOG_VERBOSE("   IO Capability:       Display Yes/No.\n");
            break;
        case QAPI_BLE_LIC_KEYBOARD_ONLY_E:
            LOG_VERBOSE("   IO Capability:       Keyboard Only.\n");
            break;
        case QAPI_BLE_LIC_NO_INPUT_NO_OUTPUT_E:
            LOG_VERBOSE("   IO Capability:       No Input No Output.\n");
            break;
        case QAPI_BLE_LIC_KEYBOARD_DISPLAY_E:
            LOG_VERBOSE("   IO Capability:       Keyboard/Display.\n");
            break;
    }

    LOG_VERBOSE("   MITM:                %s.\n", (Pairing_Capabilities->MITM)?"TRUE":"FALSE");
    LOG_VERBOSE("   Bonding Type:        %s.\n", (Pairing_Capabilities->Bonding_Type == QAPI_BLE_LBT_BONDING_E)?"Bonding":"No Bonding");
    LOG_VERBOSE("   OOB:                 %s.\n", (Pairing_Capabilities->OOB_Present)?"OOB":"OOB Not Present");
    LOG_VERBOSE("   Encryption Key Size: %d.\n", Pairing_Capabilities->Maximum_Encryption_Key_Size);
    LOG_VERBOSE("   Sending Keys: \n");
    LOG_VERBOSE("      LTK:              %s.\n", ((Pairing_Capabilities->Sending_Keys.Encryption_Key)?"YES":"NO"));
    LOG_VERBOSE("      IRK:              %s.\n", ((Pairing_Capabilities->Sending_Keys.Identification_Key)?"YES":"NO"));
    LOG_VERBOSE("      CSRK:             %s.\n", ((Pairing_Capabilities->Sending_Keys.Signing_Key)?"YES":"NO"));
    LOG_VERBOSE("   Receiving Keys: \n");
    LOG_VERBOSE("      LTK:              %s.\n", ((Pairing_Capabilities->Receiving_Keys.Encryption_Key)?"YES":"NO"));
    LOG_VERBOSE("      IRK:              %s.\n", ((Pairing_Capabilities->Receiving_Keys.Identification_Key)?"YES":"NO"));
    LOG_VERBOSE("      CSRK:             %s.\n", ((Pairing_Capabilities->Receiving_Keys.Signing_Key)?"YES":"NO"));
}

/**
 * @func : DisplayPairingInformation
 * @Desc : The following function displays the pairing capabilities that is
 *         passed into this function.
 */
static void DisplayPairingInformation(qapi_BLE_GAP_LE_Extended_Pairing_Capabilities_t *Pairing_Capabilities)
{
    switch (Pairing_Capabilities->IO_Capability)
    {
        case QAPI_BLE_LIC_DISPLAY_ONLY_E:
            LOG_VERBOSE("   IO Capability:       Display Only.\n");
            break;
        case QAPI_BLE_LIC_DISPLAY_YES_NO_E:
            LOG_VERBOSE("   IO Capability:       Display Yes/No.\n");
            break;
        case QAPI_BLE_LIC_KEYBOARD_ONLY_E:
            LOG_VERBOSE("   IO Capability:       Keyboard Only.\n");
            break;
        case QAPI_BLE_LIC_NO_INPUT_NO_OUTPUT_E:
            LOG_VERBOSE("   IO Capability:       No Input No Output.\n");
            break;
        case QAPI_BLE_LIC_KEYBOARD_DISPLAY_E:
            LOG_VERBOSE("   IO Capability:       Keyboard/Display.\n");
            break;
    }

    LOG_INFO("   Bonding Type:        %s.\n", (Pairing_Capabilities->Bonding_Type == QAPI_BLE_LBT_BONDING_E)?"Bonding":"No Bonding");
    LOG_INFO("   MITM:                %s.\n", (Pairing_Capabilities->Flags & QAPI_BLE_GAP_LE_EXTENDED_PAIRING_CAPABILITIES_FLAGS_MITM_REQUESTED)?"TRUE":"FALSE");
    LOG_INFO("   Secure Connections:  %s.\n", (Pairing_Capabilities->Flags & QAPI_BLE_GAP_LE_EXTENDED_PAIRING_CAPABILITIES_FLAGS_SECURE_CONNECTIONS)?"TRUE":"FALSE");
    LOG_INFO("   OOB:                 %s.\n", (Pairing_Capabilities->Flags & QAPI_BLE_GAP_LE_EXTENDED_PAIRING_CAPABILITIES_FLAGS_OOB_DATA_PRESENT)?"OOB":"OOB Not Present");
    LOG_INFO("   Encryption Key Size: %d.\n", Pairing_Capabilities->Maximum_Encryption_Key_Size);
    LOG_VERBOSE("   Sending Keys: \n");
    LOG_VERBOSE("      LTK:              %s.\n", ((Pairing_Capabilities->Sending_Keys.Encryption_Key)?"YES":"NO"));
    LOG_VERBOSE("      IRK:              %s.\n", ((Pairing_Capabilities->Sending_Keys.Identification_Key)?"YES":"NO"));
    LOG_VERBOSE("      CSRK:             %s.\n", ((Pairing_Capabilities->Sending_Keys.Signing_Key)?"YES":"NO"));
    LOG_VERBOSE("      Link Key:         %s.\n", ((Pairing_Capabilities->Sending_Keys.Link_Key)?"YES":"NO"));
    LOG_VERBOSE("   Receiving Keys: \n");
    LOG_VERBOSE("      LTK:              %s.\n", ((Pairing_Capabilities->Receiving_Keys.Encryption_Key)?"YES":"NO"));
    LOG_VERBOSE("      IRK:              %s.\n", ((Pairing_Capabilities->Receiving_Keys.Identification_Key)?"YES":"NO"));
    LOG_VERBOSE("      CSRK:             %s.\n", ((Pairing_Capabilities->Receiving_Keys.Signing_Key)?"YES":"NO"));
    LOG_VERBOSE("      Link Key:         %s.\n", ((Pairing_Capabilities->Receiving_Keys.Link_Key)?"YES":"NO"));
}

/**
 * @func : GAP_LE_Event_Callback
 * @Desc : The following function is for the GAP LE Event Receive Data
 *         Callback.  This function will be called whenever a Callback has
 *         been registered for the specified GAP LE Action that is associated
 *         with the Bluetooth Stack.
 */
static void QAPI_BLE_BTPSAPI GAP_LE_Event_Callback(uint32_t BluetoothStackID, qapi_BLE_GAP_LE_Event_Data_t *GAP_LE_Event_Data, uint32_t CallbackParameter)
{
    boolean_t                                              DisplayPrompt;
    int                                                    Result;
    uint16_t                                               EDIV;
    BoardStr_t                                             BoardStr;
    unsigned int                                           Index;
    DeviceInfo_t                                          *DeviceInfo;
    qapi_BLE_Random_Number_t                               RandomNumber;
    qapi_BLE_Long_Term_Key_t                               GeneratedLTK;
    qapi_BLE_GAP_LE_Security_Information_t                 GAP_LE_Security_Information;
    qapi_BLE_GAP_LE_Connection_Parameters_t                ConnectionParams;
    qapi_BLE_GAP_LE_Advertising_Report_Data_t             *DeviceEntryPtr;
    qapi_BLE_GAP_LE_Authentication_Event_Data_t           *Authentication_Event_Data;
    qapi_BLE_GAP_LE_Direct_Advertising_Report_Data_t      *DirectDeviceEntryPtr;
#ifdef V2
    qapi_BLE_GAP_LE_Extended_Advertising_Report_Data_t    *ExtDeviceEntryPtr;
#endif
    qapi_BLE_GAP_LE_Authentication_Response_Information_t  GAP_LE_Authentication_Response_Information;

    if((BluetoothStackID) && (GAP_LE_Event_Data))
    {
        DisplayPrompt = true;

        switch(GAP_LE_Event_Data->Event_Data_Type)
        {
#ifdef V2
            case QAPI_BLE_ET_LE_SCAN_TIMEOUT_E:
                LOG_WARN("etLE_Scan_Timeout with size %d.\n",(int)GAP_LE_Event_Data->Event_Data_Size);
                break;
            case QAPI_BLE_ET_LE_PHY_UPDATE_COMPLETE_E:
                LOG_INFO("etLE_PHY_Update_Complete with size %d.\n",(int)GAP_LE_Event_Data->Event_Data_Size);

                LOG_VERBOSE("  Status:  %d.\n", (int)(GAP_LE_Event_Data->Event_Data.GAP_LE_Phy_Update_Complete_Event_Data->Status));
                LOG_VERBOSE("  Address: 0x%02X%02X%02X%02X%02X%02X.\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Phy_Update_Complete_Event_Data->BD_ADDR.BD_ADDR5,
                        GAP_LE_Event_Data->Event_Data.GAP_LE_Phy_Update_Complete_Event_Data->BD_ADDR.BD_ADDR4,
                        GAP_LE_Event_Data->Event_Data.GAP_LE_Phy_Update_Complete_Event_Data->BD_ADDR.BD_ADDR3,
                        GAP_LE_Event_Data->Event_Data.GAP_LE_Phy_Update_Complete_Event_Data->BD_ADDR.BD_ADDR2,
                        GAP_LE_Event_Data->Event_Data.GAP_LE_Phy_Update_Complete_Event_Data->BD_ADDR.BD_ADDR1,
                        GAP_LE_Event_Data->Event_Data.GAP_LE_Phy_Update_Complete_Event_Data->BD_ADDR.BD_ADDR0);
                LOG_VERBOSE("  Tx PHY:  %s.\n", PHYToString(GAP_LE_Event_Data->Event_Data.GAP_LE_Phy_Update_Complete_Event_Data->TX_PHY));
                LOG_VERBOSE("  Rx PHY:  %s.\n", PHYToString(GAP_LE_Event_Data->Event_Data.GAP_LE_Phy_Update_Complete_Event_Data->RX_PHY));

                break;
            case QAPI_BLE_ET_LE_ADVERTISING_SET_TERMINATED_E:
                LOG_INFO("etLE_Advertising_Set_Terminated with size %d.\n",(int)GAP_LE_Event_Data->Event_Data_Size);

                LOG_VERBOSE("  Status:                                  %d.\n", (int)(GAP_LE_Event_Data->Event_Data.GAP_LE_Advertising_Set_Terminated_Event_Data->Status));
                LOG_VERBOSE("  Advertising Handle:                      %u.\n", (unsigned int)(GAP_LE_Event_Data->Event_Data.GAP_LE_Advertising_Set_Terminated_Event_Data->Advertising_Handle));
                LOG_VERBOSE("  Number of Completed Advertising Events:  %u.\n", (unsigned int)(GAP_LE_Event_Data->Event_Data.GAP_LE_Advertising_Set_Terminated_Event_Data->Num_Completed_Ext_Advertising_Events));

                if(GAP_LE_Event_Data->Event_Data.GAP_LE_Advertising_Set_Terminated_Event_Data->Status == QAPI_BLE_HCI_ERROR_CODE_SUCCESS)
                {
                    switch(GAP_LE_Event_Data->Event_Data.GAP_LE_Advertising_Set_Terminated_Event_Data->Connection_Address_Type)
                    {
                        case QAPI_BLE_LAT_PUBLIC_E:
                            LOG_VERBOSE("  Connection Address Type:                 %s.\n", "QAPI_BLE_LAT_PUBLIC_E");
                            break;
                        case QAPI_BLE_LAT_RANDOM_E:
                            LOG_VERBOSE("  Connection Address Type:                 %s.\n", "QAPI_BLE_LAT_RANDOM_E");
                            break;
                        case QAPI_BLE_LAT_PUBLIC_IDENTITY_E:
                            LOG_VERBOSE("  Connection Address Type:                 %s.\n", "QAPI_BLE_LAT_PUBLIC_IDENTITY_E");
                            break;
                        case QAPI_BLE_LAT_RANDOM_IDENTITY_E:
                            LOG_VERBOSE("  Connection Address Type:                 %s.\n", "QAPI_BLE_LAT_RANDOM_IDENTITY_E");
                            break;
                        default:
                            LOG_VERBOSE("  Connection Address Type:                 Invalid.\n");
                            break;
                    }

                    BD_ADDRToStr(GAP_LE_Event_Data->Event_Data.GAP_LE_Advertising_Set_Terminated_Event_Data->Connection_Address, BoardStr);
                    LOG_VERBOSE("  Connection Address:                      %s.\n", BoardStr);
                }
                break;
            case QAPI_BLE_ET_LE_SCAN_REQUEST_RECEIVED_E:
                LOG_INFO("etLE_Scan_Request_Received with size %d.\n",(int)GAP_LE_Event_Data->Event_Data_Size);

                LOG_INFO("  Advertising Handle:          %d.\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Scan_Request_Received_Event_Data->Advertising_Handle);

                switch(GAP_LE_Event_Data->Event_Data.GAP_LE_Scan_Request_Received_Event_Data->Scanner_Address_Type)
                {
                    case QAPI_BLE_LAT_PUBLIC_E:
                        LOG_VERBOSE("  Scanner Address Type:        %s.\n", "QAPI_BLE_LAT_PUBLIC_E");
                        break;
                    case QAPI_BLE_LAT_RANDOM_E:
                        LOG_VERBOSE("  Scanner Address Type:        %s.\n", "QAPI_BLE_LAT_RANDOM_E");
                        break;
                    case QAPI_BLE_LAT_PUBLIC_IDENTITY_E:
                        LOG_VERBOSE("  Scanner Address Type:        %s.\n", "QAPI_BLE_LAT_PUBLIC_IDENTITY_E");
                        break;
                    case QAPI_BLE_LAT_RANDOM_IDENTITY_E:
                        LOG_VERBOSE("  Scanner Address Type:        %s.\n", "QAPI_BLE_LAT_RANDOM_IDENTITY_E");
                        break;
                    default:
                        LOG_VERBOSE("  Scanner Address Type:        Invalid.\n");
                        break;
                }

                BD_ADDRToStr(GAP_LE_Event_Data->Event_Data.GAP_LE_Scan_Request_Received_Event_Data->Scanner_Address, BoardStr);
                LOG_VERBOSE("  Scanner Address:             %s.\n", BoardStr);
                break;
            case QAPI_BLE_ET_LE_CHANNEL_SELECTION_ALGORITHM_UPDATE_E:
                LOG_VERBOSE("etLE_Channel_Selection_Algorithm_Update with size %d.\n", (int)GAP_LE_Event_Data->Event_Data_Size);

                switch(GAP_LE_Event_Data->Event_Data.GAP_LE_Channel_Selection_Algorithm_Update_Event_Data->Channel_Selection_Algorithm)
                {
                    case QAPI_BLE_SA_ALGORITHM_NUM1_E:
                        LOG_VERBOSE("  Channel Selection Algorithm:        %s.\n", "CSA #1");
                        break;
                    case QAPI_BLE_SA_ALGORITHM_NUM2_E:
                        LOG_VERBOSE("  Channel Selection Algorithm:        %s.\n", "CSA #2");
                        break;
                    default:
                        LOG_VERBOSE("  Channel Selection Algorithm:        %s.\n", "CSA Unkown");
                        break;
                }

                switch(GAP_LE_Event_Data->Event_Data.GAP_LE_Channel_Selection_Algorithm_Update_Event_Data->Connection_Address_Type)
                {
                    case QAPI_BLE_LAT_PUBLIC_E:
                        LOG_VERBOSE("  Connection Address Type:            %s.\n", "QAPI_BLE_LAT_PUBLIC_E");
                        break;
                    case QAPI_BLE_LAT_RANDOM_E:
                        LOG_VERBOSE("  Connection Address Type:            %s.\n", "QAPI_BLE_LAT_RANDOM_E");
                        break;
                    case QAPI_BLE_LAT_PUBLIC_IDENTITY_E:
                        LOG_VERBOSE("  Connection Address Type:            %s.\n", "QAPI_BLE_LAT_PUBLIC_IDENTITY_E");
                        break;
                    case QAPI_BLE_LAT_RANDOM_IDENTITY_E:
                        LOG_VERBOSE("  Connection Address Type:            %s.\n", "QAPI_BLE_LAT_RANDOM_IDENTITY_E");
                        break;
                    default:
                        LOG_VERBOSE("  Connection Address Type:            Invalid.\n");
                        break;
                }

                BD_ADDRToStr(GAP_LE_Event_Data->Event_Data.GAP_LE_Channel_Selection_Algorithm_Update_Event_Data->Connection_Address, BoardStr);
                LOG_VERBOSE("  Connection Address:                 %s.\n", BoardStr);
                break;
            case QAPI_BLE_ET_LE_EXTENDED_ADVERTISING_REPORT_E:
                LOG_VERBOSE("etLE_Extended_Advertising_Report with size %d.\n",(int)GAP_LE_Event_Data->Event_Data_Size);
                LOG_VERBOSE("  %d Responses.\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Extended_Advertising_Report_Event_Data->Number_Device_Entries);

                for(Index = 0; Index < GAP_LE_Event_Data->Event_Data.GAP_LE_Extended_Advertising_Report_Event_Data->Number_Device_Entries; Index++)
                {
                    ExtDeviceEntryPtr = &(GAP_LE_Event_Data->Event_Data.GAP_LE_Extended_Advertising_Report_Event_Data->Extended_Advertising_Data[Index]);

                    switch(ExtDeviceEntryPtr->Address_Type)
                    {
                        case QAPI_BLE_LAT_PUBLIC_E:
                            LOG_VERBOSE("  Address Type:     %s.\n", "QAPI_BLE_LAT_PUBLIC_E");
                            break;
                        case QAPI_BLE_LAT_RANDOM_E:
                            LOG_VERBOSE("  Address Type:     %s.\n", "QAPI_BLE_LAT_RANDOM_E");
                            break;
                        case QAPI_BLE_LAT_PUBLIC_IDENTITY_E:
                            LOG_VERBOSE("  Address Type:     %s.\n", "QAPI_BLE_LAT_PUBLIC_IDENTITY_E");
                            break;
                        case QAPI_BLE_LAT_RANDOM_IDENTITY_E:
                            LOG_VERBOSE("  Address Type:     %s.\n", "QAPI_BLE_LAT_RANDOM_IDENTITY_E");
                            break;
                        case QAPI_BLE_LAT_ANONYMOUS_E:
                            LOG_VERBOSE("  Address Type:     %s.\n", "Anonymous");
                            break;
                        default:
                            LOG_VERBOSE("  Address Type:     Invalid.\n");
                            break;
                    }

                    LOG_VERBOSE("  Address:          0x%02X%02X%02X%02X%02X%02X.\n", ExtDeviceEntryPtr->BD_ADDR.BD_ADDR5, ExtDeviceEntryPtr->BD_ADDR.BD_ADDR4, ExtDeviceEntryPtr->BD_ADDR.BD_ADDR3, ExtDeviceEntryPtr->BD_ADDR.BD_ADDR2, ExtDeviceEntryPtr->BD_ADDR.BD_ADDR1, ExtDeviceEntryPtr->BD_ADDR.BD_ADDR0);

                    if(DisplayAdvertisingEventData)
                    {
                        LOG_VERBOSE("  Event Type Flags: 0x%08X.\n", ExtDeviceEntryPtr->Event_Type_Flags);
                        LOG_VERBOSE("  Tx Power:         %d.\n", (int)ExtDeviceEntryPtr->Tx_Power);
                        LOG_VERBOSE("  RSSI:             %d.\n", (int)ExtDeviceEntryPtr->RSSI);
                        LOG_VERBOSE("  Advertising SID:  %d.\n", (int)ExtDeviceEntryPtr->Advertising_SID);
                        LOG_VERBOSE("  Primary PHY:      %s.\n", PHYToString(ExtDeviceEntryPtr->Primary_PHY));

                        switch(ExtDeviceEntryPtr->Data_Status)
                        {
                            case QAPI_BLE_DS_COMPLETE_E:
                                LOG_VERBOSE("  Data Status:      %s.\n", "Complete");
                                break;
                            case QAPI_BLE_DS_INCOMPLETE_DATA_PENDING_E:
                                LOG_VERBOSE("  Data Status:      %s.\n", "Incomplete - More data pending");
                                break;
                            default:
                            case QAPI_BLE_DS_INCOMPLETE_DATA_TRUNCATED_E:
                                LOG_VERBOSE("  Data Status:      %s.\n", "Incomplete - data truncated");
                                break;
                        }

                        if(ExtDeviceEntryPtr->Event_Type_Flags & GAP_LE_EXTENDED_ADVERTISING_EVENT_TYPE_SECONDARY_PHY_VALID)
                            LOG_VERBOSE("  Secondary PHY:    %s.\n", PHYToString(ExtDeviceEntryPtr->Secondary_PHY));

                        LOG_VERBOSE("  Data Length:      %u.\n", (unsigned int)ExtDeviceEntryPtr->Raw_Report_Length);
                    }
                }
                break;
#endif

            case QAPI_BLE_ET_LE_DATA_LENGTH_CHANGE_E:
                LOG_VERBOSE("etLE_Data_Length_Change with size %d.\n", (int)GAP_LE_Event_Data->Event_Data_Size);

                BD_ADDRToStr(GAP_LE_Event_Data->Event_Data.GAP_LE_Data_Length_Change_Event_Data->BD_ADDR, BoardStr);
                LOG_VERBOSE("  Connection Address:                 %s.\n", BoardStr);
                LOG_VERBOSE("  Max Tx Octets:                      %u.\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Data_Length_Change_Event_Data->MaxTxOctets);
                LOG_VERBOSE("  Max Tx Time:                        %u.\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Data_Length_Change_Event_Data->MaxTxTime);
                LOG_VERBOSE("  Max Rx Octets:                      %u.\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Data_Length_Change_Event_Data->MaxRxOctets);
                LOG_VERBOSE("  Max Rx Time:                        %u.\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Data_Length_Change_Event_Data->MaxRxTime);
                break;
            case QAPI_BLE_ET_LE_ADVERTISING_REPORT_E:
                LOG_VERBOSE("etLE_Advertising_Report with size %d.\n",(int)GAP_LE_Event_Data->Event_Data_Size);
                LOG_VERBOSE("  %d Responses.\n",GAP_LE_Event_Data->Event_Data.GAP_LE_Advertising_Report_Event_Data->Number_Device_Entries);

                for(Index = 0; Index < GAP_LE_Event_Data->Event_Data.GAP_LE_Advertising_Report_Event_Data->Number_Device_Entries; Index++)
                {
                    DeviceEntryPtr = &(GAP_LE_Event_Data->Event_Data.GAP_LE_Advertising_Report_Event_Data->Advertising_Data[Index]);

                    switch(DeviceEntryPtr->Address_Type)
                    {
                        case QAPI_BLE_LAT_PUBLIC_E:
                            LOG_VERBOSE("  Address Type:        %s.\n", "QAPI_BLE_LAT_PUBLIC_E");
                            break;
                        case QAPI_BLE_LAT_RANDOM_E:
                            LOG_VERBOSE("  Address Type:        %s.\n", "QAPI_BLE_LAT_RANDOM_E");
                            break;
                        case QAPI_BLE_LAT_PUBLIC_IDENTITY_E:
                            LOG_VERBOSE("  Address Type:        %s.\n", "QAPI_BLE_LAT_PUBLIC_IDENTITY_E");
                            break;
                        case QAPI_BLE_LAT_RANDOM_IDENTITY_E:
                            LOG_VERBOSE("  Address Type:        %s.\n", "QAPI_BLE_LAT_RANDOM_IDENTITY_E");
                            break;
                        default:
                            LOG_VERBOSE("  Address Type:        Invalid.\n");
                            break;
                    }

                    LOG_VERBOSE("  Address: 0x%02X%02X%02X%02X%02X%02X.\n", DeviceEntryPtr->BD_ADDR.BD_ADDR5, DeviceEntryPtr->BD_ADDR.BD_ADDR4, DeviceEntryPtr->BD_ADDR.BD_ADDR3, DeviceEntryPtr->BD_ADDR.BD_ADDR2, DeviceEntryPtr->BD_ADDR.BD_ADDR1, DeviceEntryPtr->BD_ADDR.BD_ADDR0);

                    if(DisplayAdvertisingEventData)
                    {
                        switch(DeviceEntryPtr->Advertising_Report_Type)
                        {
                            case QAPI_BLE_RT_CONNECTABLE_UNDIRECTED_E:
                                LOG_VERBOSE("  Advertising Type: %s.\n", "QAPI_BLE_RT_CONNECTABLE_UNDIRECTED_E");
                                break;
                            case QAPI_BLE_RT_CONNECTABLE_DIRECTED_E:
                                LOG_VERBOSE("  Advertising Type: %s.\n", "QAPI_BLE_RT_CONNECTABLE_DIRECTED_E");
                                break;
                            case QAPI_BLE_RT_SCANNABLE_UNDIRECTED_E:
                                LOG_VERBOSE("  Advertising Type: %s.\n", "QAPI_BLE_RT_SCANNABLE_UNDIRECTED_E");
                                break;
                            case QAPI_BLE_RT_NON_CONNECTABLE_UNDIRECTED_E:
                                LOG_VERBOSE("  Advertising Type: %s.\n", "QAPI_BLE_RT_NON_CONNECTABLE_UNDIRECTED_E");
                                break;
                            case QAPI_BLE_RT_SCAN_RESPONSE_E:
                                LOG_VERBOSE("  Advertising Type: %s.\n", "QAPI_BLE_RT_SCAN_RESPONSE_E");
                                break;
                        }

                        LOG_VERBOSE("  RSSI: %d.\n", (int)(DeviceEntryPtr->RSSI));
                        LOG_VERBOSE("  Data Length: %d.\n", DeviceEntryPtr->Raw_Report_Length);
                    }
                }
                break;
            case QAPI_BLE_ET_LE_CONNECTION_COMPLETE_E:
                LOG_INFO("etLE_Connection_Complete with size %d.\n",(int)GAP_LE_Event_Data->Event_Data_Size);

                if(GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data)
                {
                    BD_ADDRToStr(GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Peer_Address, BoardStr);

                    LOG_VERBOSE("   Status:              0x%02X.\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Status);
                    LOG_VERBOSE("   Role:                %s.\n", (GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Master)?"Master":"Slave");
                    switch(GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Peer_Address_Type)
                    {
                        case QAPI_BLE_LAT_PUBLIC_E:
                            LOG_VERBOSE("   Address Type:        %s.\n", "QAPI_BLE_LAT_PUBLIC_E");
                            break;
                        case QAPI_BLE_LAT_RANDOM_E:
                            LOG_VERBOSE("   Address Type:        %s.\n", "QAPI_BLE_LAT_RANDOM_E");
                            break;
                        case QAPI_BLE_LAT_PUBLIC_IDENTITY_E:
                            LOG_VERBOSE("   Address Type:        %s.\n", "QAPI_BLE_LAT_PUBLIC_IDENTITY_E");
                            break;
                        case QAPI_BLE_LAT_RANDOM_IDENTITY_E:
                            LOG_VERBOSE("   Address Type:        %s.\n", "QAPI_BLE_LAT_RANDOM_IDENTITY_E");
                            break;
                        default:
                            LOG_VERBOSE("   Address Type:        Invalid.\n");
                            break;
                    }
                    LOG_VERBOSE("   BD_ADDR:             %s.\n", BoardStr);

                    if(GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Status == QAPI_BLE_HCI_ERROR_CODE_NO_ERROR)
                    {
                        LOG_VERBOSE("   Connection Interval: %u.\n", (unsigned int)GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Current_Connection_Parameters.Connection_Interval);
                        LOG_VERBOSE("   Slave Latency:       %u.\n", (unsigned int)GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Current_Connection_Parameters.Slave_Latency);

                        LocalDeviceIsMaster =  GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Master;
                        RemoteAddressType   = GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Peer_Address_Type;

                        if((DeviceInfo = SearchDeviceInfoEntryTypeAddress(&DeviceInfoList, GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Peer_Address_Type, GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Peer_Address)) != NULL)
                        {
                            uint8_t            Peer_Identity_Address_Type;
                            uint8_t            StatusResult;
                            qapi_BLE_BD_ADDR_t Peer_Identity_Address;
                            qapi_BLE_BD_ADDR_t Local_Resolvable_Address;

                            if(GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Peer_Address_Type == QAPI_BLE_LAT_PUBLIC_IDENTITY_E)
                                Peer_Identity_Address_Type = 0x00;
                            else if(GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Peer_Address_Type == QAPI_BLE_LAT_RANDOM_IDENTITY_E)
                                Peer_Identity_Address_Type = 0x01;
                            else
                                Peer_Identity_Address_Type = 0x02;

                            if(Peer_Identity_Address_Type != 0x02)
                            {
                                Peer_Identity_Address = GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Peer_Address;

                                if(!qapi_BLE_HCI_LE_Read_Local_Resolvable_Address(BluetoothStackID, Peer_Identity_Address_Type, Peer_Identity_Address, &StatusResult, &Local_Resolvable_Address))
                                {
                                    LOG_VERBOSE("   qapi_BLE_HCI_LE_Read_Local_Resolvable_Address(): 0x%02X.\n", StatusResult);
                                    if(!StatusResult)
                                    {
                                        BD_ADDRToStr(Local_Resolvable_Address, BoardStr);
                                        LOG_VERBOSE("   Local RPA:           %s.\n", BoardStr);
                                    }
                                }
                            }

                            if(LocalDeviceIsMaster)
                            {
                                if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_LTK_VALID)
                                {
                                    LOG_INFO("Attempting to Re-Establish Security.\n");

                                    GAP_LE_Security_Information.Local_Device_Is_Master                                      = TRUE;
                                    memcpy(&(GAP_LE_Security_Information.Security_Information.Master_Information.LTK), &(DeviceInfo->LTK), sizeof(DeviceInfo->LTK));
                                    GAP_LE_Security_Information.Security_Information.Master_Information.EDIV                = DeviceInfo->EDIV;
                                    memcpy(&(GAP_LE_Security_Information.Security_Information.Master_Information.Rand), &(DeviceInfo->Rand), sizeof(DeviceInfo->Rand));
                                    GAP_LE_Security_Information.Security_Information.Master_Information.Encryption_Key_Size = DeviceInfo->EncryptionKeySize;

                                    Result = qapi_BLE_GAP_LE_Reestablish_Security(BluetoothStackID, GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Peer_Address, &GAP_LE_Security_Information, GAP_LE_Event_Callback, 0);
                                    if(Result)
                                        LOG_INFO("GAP_LE_Reestablish_Security returned %d.\n", Result);
                                }
                                else
                                {
                                    LOG_ERROR("Can't re-establish security: LTK is missing.\n");
                                }
                            }
                        }
                    }
                }
                break;
            case QAPI_BLE_ET_LE_DISCONNECTION_COMPLETE_E:
                LOG_INFO("etLE_Disconnection_Complete with size %d.\n", (int)GAP_LE_Event_Data->Event_Data_Size);

                if(GAP_LE_Event_Data->Event_Data.GAP_LE_Disconnection_Complete_Event_Data)
                {
                    LOG_VERBOSE("   Status: 0x%02X.\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Disconnection_Complete_Event_Data->Status);
                    LOG_VERBOSE("   Reason: 0x%02X.\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Disconnection_Complete_Event_Data->Reason);

                    BD_ADDRToStr(GAP_LE_Event_Data->Event_Data.GAP_LE_Disconnection_Complete_Event_Data->Peer_Address, BoardStr);
                    LOG_VERBOSE("   BD_ADDR: %s.\n", BoardStr);

                    SendInfo.BytesToSend = 0;
                    SendInfo.BytesSent   = 0;
                }
                // To advertise after disconnection
                AdvertiseLE(AET_ENABLE_ALL_E);
                break;
            case QAPI_BLE_ET_LE_CONNECTION_PARAMETER_UPDATE_REQUEST_E:
                LOG_VERBOSE("etLE_Connection_Parameter_Update_Request with size %d.\n",(int)GAP_LE_Event_Data->Event_Data_Size);

                if(GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Update_Request_Event_Data)
                {
                    BD_ADDRToStr(GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Update_Request_Event_Data->BD_ADDR, BoardStr);
                    LOG_VERBOSE("   BD_ADDR:                     %s\n", BoardStr);
                    LOG_VERBOSE("   Connection Interval Minimum: %d\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Update_Request_Event_Data->Conn_Interval_Min);
                    LOG_VERBOSE("   Connection Interval Maximum: %d\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Update_Request_Event_Data->Conn_Interval_Max);
                    LOG_VERBOSE("   Slave Latency:               %d\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Update_Request_Event_Data->Slave_Latency);
                    LOG_VERBOSE("   Supervision Timeout:         %d\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Update_Request_Event_Data->Conn_Supervision_Timeout);

                    ConnectionParams.Connection_Interval_Min    = GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Update_Request_Event_Data->Conn_Interval_Min;
                    ConnectionParams.Connection_Interval_Max    = GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Update_Request_Event_Data->Conn_Interval_Max;
                    ConnectionParams.Slave_Latency              = GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Update_Request_Event_Data->Slave_Latency;
                    ConnectionParams.Supervision_Timeout        = GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Update_Request_Event_Data->Conn_Supervision_Timeout;
                    ConnectionParams.Minimum_Connection_Length  = 0;
                    ConnectionParams.Maximum_Connection_Length  = 10000;

                    qapi_BLE_GAP_LE_Connection_Parameter_Update_Response(BluetoothStackID, GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Update_Request_Event_Data->BD_ADDR, TRUE, &ConnectionParams);
                }
                break;
            case QAPI_BLE_ET_LE_CONNECTION_PARAMETER_UPDATED_E:
                LOG_VERBOSE("etLE_Connection_Parameter_Updated with size %d.\n",(int)GAP_LE_Event_Data->Event_Data_Size);

                if(GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Updated_Event_Data)
                {
                    BD_ADDRToStr(GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Updated_Event_Data->BD_ADDR, BoardStr);
                    LOG_VERBOSE("   BD_ADDR:             %s\n", BoardStr);
                    LOG_VERBOSE("   Status:              %d\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Updated_Event_Data->Status);
                    LOG_VERBOSE("   Connection Interval: %d\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Updated_Event_Data->Current_Connection_Parameters.Connection_Interval);
                    LOG_VERBOSE("   Slave Latency:       %d\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Updated_Event_Data->Current_Connection_Parameters.Slave_Latency);
                    LOG_VERBOSE("   Supervision Timeout: %d\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Updated_Event_Data->Current_Connection_Parameters.Supervision_Timeout);
                }
                break;
            case QAPI_BLE_ET_LE_ENCRYPTION_CHANGE_E:
                LOG_VERBOSE("etLE_Encryption_Change with size %d.\n",(int)GAP_LE_Event_Data->Event_Data_Size);
                break;
            case QAPI_BLE_ET_LE_ENCRYPTION_REFRESH_COMPLETE_E:
                LOG_VERBOSE("etLE_Encryption_Refresh_Complete with size %d.\n", (int)GAP_LE_Event_Data->Event_Data_Size);
                break;
            case QAPI_BLE_ET_LE_AUTHENTICATION_E:
                LOG_VERBOSE("etLE_Authentication with size %d.\n", (int)GAP_LE_Event_Data->Event_Data_Size);

                if((Authentication_Event_Data = GAP_LE_Event_Data->Event_Data.GAP_LE_Authentication_Event_Data) != NULL)
                {
                    BD_ADDRToStr(Authentication_Event_Data->BD_ADDR, BoardStr);

                    switch(Authentication_Event_Data->GAP_LE_Authentication_Event_Type)
                    {
                        case QAPI_BLE_LAT_LONG_TERM_KEY_REQUEST_E:
                            LOG_VERBOSE("    latKeyRequest: \n");
                            LOG_VERBOSE("      BD_ADDR: %s.\n", BoardStr);

                            GAP_LE_Authentication_Response_Information.GAP_LE_Authentication_Type = QAPI_BLE_LAR_LONG_TERM_KEY_E;
                            GAP_LE_Authentication_Response_Information.Authentication_Data_Length = 0;

                            memset(&RandomNumber, 0, sizeof(RandomNumber));
                            EDIV = 0;

                            if((Authentication_Event_Data->Authentication_Event_Data.Long_Term_Key_Request.EDIV == EDIV) && (QAPI_BLE_COMPARE_RANDOM_NUMBER(Authentication_Event_Data->Authentication_Event_Data.Long_Term_Key_Request.Rand, RandomNumber)))
                            {
                                if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, Authentication_Event_Data->BD_ADDR)) != NULL)
                                {
                                    if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_LTK_VALID)
                                    {
                                        GAP_LE_Authentication_Response_Information.Authentication_Data_Length                                        = QAPI_BLE_GAP_LE_LONG_TERM_KEY_INFORMATION_DATA_SIZE;
                                        GAP_LE_Authentication_Response_Information.Authentication_Data.Long_Term_Key_Information.Encryption_Key_Size = DeviceInfo->EncryptionKeySize;

                                        memcpy(&(GAP_LE_Authentication_Response_Information.Authentication_Data.Long_Term_Key_Information.Long_Term_Key), &(DeviceInfo->LTK), QAPI_BLE_LONG_TERM_KEY_SIZE);
                                    }
                                }
                            }
                            else
                            {
                                Result = qapi_BLE_GAP_LE_Regenerate_Long_Term_Key(BluetoothStackID, (qapi_BLE_Encryption_Key_t *)(&DHK), (qapi_BLE_Encryption_Key_t *)(&ER), Authentication_Event_Data->Authentication_Event_Data.Long_Term_Key_Request.EDIV, &(Authentication_Event_Data->Authentication_Event_Data.Long_Term_Key_Request.Rand), &GeneratedLTK);
                                if(!Result)
                                {
                                    LOG_VERBOSE("      GAP_LE_Regenerate_Long_Term_Key Success.\n");

                                    GAP_LE_Authentication_Response_Information.GAP_LE_Authentication_Type                                        = QAPI_BLE_LAR_LONG_TERM_KEY_E;
                                    GAP_LE_Authentication_Response_Information.Authentication_Data_Length                                        = QAPI_BLE_GAP_LE_LONG_TERM_KEY_INFORMATION_DATA_SIZE;
                                    GAP_LE_Authentication_Response_Information.Authentication_Data.Long_Term_Key_Information.Encryption_Key_Size = QAPI_BLE_GAP_LE_MAXIMUM_ENCRYPTION_KEY_SIZE;
                                    GAP_LE_Authentication_Response_Information.Authentication_Data.Long_Term_Key_Information.Long_Term_Key       = GeneratedLTK;
                                }
                                else
                                {
                                    LOG_VERBOSE("      GAP_LE_Regenerate_Long_Term_Key returned %d.\n",Result);
                                }
                            }

                            Result = qapi_BLE_GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information);
                            if(Result)
                            {
                                LOG_VERBOSE("      GAP_LE_Authentication_Response returned %d.\n",Result);
                            }
                            break;
                        case QAPI_BLE_LAT_SECURITY_REQUEST_E:
                            LOG_VERBOSE("    latSecurityRequest:.\n");
                            LOG_VERBOSE("      BD_ADDR: %s.\n", BoardStr);
                            LOG_VERBOSE("      Bonding Type: %s.\n", ((Authentication_Event_Data->Authentication_Event_Data.Security_Request.Bonding_Type == QAPI_BLE_LBT_BONDING_E)?"Bonding":"No Bonding"));
                            LOG_VERBOSE("      MITM: %s.\n", ((Authentication_Event_Data->Authentication_Event_Data.Security_Request.MITM)?"YES":"NO"));

                            if(QAPI_BLE_COMPARE_NULL_BD_ADDR(SecurityRemoteBD_ADDR))
                            {
                                SecurityRemoteBD_ADDR = Authentication_Event_Data->BD_ADDR;

                                if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, Authentication_Event_Data->BD_ADDR)) != NULL)
                                {
                                    if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_LTK_VALID)
                                    {
                                        LOG_VERBOSE("Attempting to Re-Establish Security.\n");

                                        GAP_LE_Security_Information.Local_Device_Is_Master                                      = TRUE;
                                        memcpy(&(GAP_LE_Security_Information.Security_Information.Master_Information.LTK), &(DeviceInfo->LTK), sizeof(DeviceInfo->LTK));
                                        GAP_LE_Security_Information.Security_Information.Master_Information.EDIV                = DeviceInfo->EDIV;
                                        memcpy(&(GAP_LE_Security_Information.Security_Information.Master_Information.Rand), &(DeviceInfo->Rand), sizeof(DeviceInfo->Rand));
                                        GAP_LE_Security_Information.Security_Information.Master_Information.Encryption_Key_Size = DeviceInfo->EncryptionKeySize;

                                        Result = qapi_BLE_GAP_LE_Reestablish_Security(BluetoothStackID, SecurityRemoteBD_ADDR, &GAP_LE_Security_Information, GAP_LE_Event_Callback, 0);
                                        if(Result)
                                        {
                                            LOG_VERBOSE("GAP_LE_Reestablish_Security returned %d.\n", Result);
                                        }
                                    }
                                }
                            }
                            else
                            {
                                LOG_WARN("\nSecurity is already in progress with another remote device.\n");

                                GAP_LE_Authentication_Response_Information.GAP_LE_Authentication_Type = QAPI_BLE_LAR_ERROR_E;
                                GAP_LE_Authentication_Response_Information.Authentication_Data_Length = 0;

                                if((Result = qapi_BLE_GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information)) != 0)
                                    DisplayFunctionError("qapi_BLE_GAP_LE_Authentication_Response", Result);
                            }
                            break;
                        case QAPI_BLE_LAT_PAIRING_REQUEST_E:
                            LOG_INFO("Pairing Request: %s.\n", BoardStr);
                            DisplayLegacyPairingInformation(&Authentication_Event_Data->Authentication_Event_Data.Pairing_Request);
                            if(QAPI_BLE_COMPARE_NULL_BD_ADDR(SecurityRemoteBD_ADDR))
                            {
                                SecurityRemoteBD_ADDR = Authentication_Event_Data->BD_ADDR;
                                SlavePairingRequestResponse(SecurityRemoteBD_ADDR);
                            }
                            else
                            {
                                LOG_WARN("\nSecurity is already in progress with another remote device.\n");

                                GAP_LE_Authentication_Response_Information.GAP_LE_Authentication_Type = QAPI_BLE_LAR_ERROR_E;
                                GAP_LE_Authentication_Response_Information.Authentication_Data_Length = 0;

                                if((Result = qapi_BLE_GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information)) != 0)
                                    DisplayFunctionError("qapi_BLE_GAP_LE_Authentication_Response", Result);
                            }
                            break;
                        case QAPI_BLE_LAT_EXTENDED_PAIRING_REQUEST_E:
                            LOG_INFO("Extended Pairing Request: %s.\n", BoardStr);
                            DisplayPairingInformation(&(Authentication_Event_Data->Authentication_Event_Data.Extended_Pairing_Request));

                            if(QAPI_BLE_COMPARE_NULL_BD_ADDR(SecurityRemoteBD_ADDR))
                            {
                                SecurityRemoteBD_ADDR = Authentication_Event_Data->BD_ADDR;
                                SlavePairingRequestResponse(Authentication_Event_Data->BD_ADDR);
                            }
                            else
                            {
                                LOG_WARN("\nSecurity is already in progress with another remote device.\n");

                                GAP_LE_Authentication_Response_Information.GAP_LE_Authentication_Type = QAPI_BLE_LAR_ERROR_E;
                                GAP_LE_Authentication_Response_Information.Authentication_Data_Length = 0;

                                if((Result = qapi_BLE_GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information)) != 0)
                                    DisplayFunctionError("qapi_BLE_GAP_LE_Authentication_Response", Result);
                            }
                            break;
                        case QAPI_BLE_LAT_CONFIRMATION_REQUEST_E:
                            LOG_VERBOSE("latConfirmationRequest.\n");

                            switch(Authentication_Event_Data->Authentication_Event_Data.Confirmation_Request.Request_Type)
                            {
                                case QAPI_BLE_CRT_NONE_E:
                                    GAP_LE_Authentication_Response_Information.GAP_LE_Authentication_Type = QAPI_BLE_LAR_CONFIRMATION_E;

                                    GAP_LE_Authentication_Response_Information.Authentication_Data_Length = sizeof(uint32_t);

                                    switch(LE_Parameters.IOCapability)
                                    {
                                        case QAPI_BLE_LIC_NO_INPUT_NO_OUTPUT_E:
                                            LOG_VERBOSE("Invoking Just Works.\n");

                                            Result = qapi_BLE_GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information);
                                            if(Result)
                                            {
                                                LOG_VERBOSE("qapi_BLE_GAP_LE_Authentication_Response returned %d.\n", Result);
                                            }
                                            break;
                                        case QAPI_BLE_LIC_DISPLAY_ONLY_E:
                                            LOG_VERBOSE("Confirmation of Pairing.\n");

                                            GAP_LE_Authentication_Response_Information.Authentication_Data.Passkey = Authentication_Event_Data->Authentication_Event_Data.Confirmation_Request.Display_Passkey;

                                            if((Result = qapi_BLE_GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information)) != 0)
                                                DisplayFunctionError("qapi_BLE_GAP_LE_Authentication_Response", Result);
                                            break;
                                        default:
                                            LOG_INFO("Confirmation of Pairing.\n");

                                            if((Result = qapi_BLE_GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information)) != 0)
                                                DisplayFunctionError("qapi_BLE_GAP_LE_Authentication_Response", Result);
                                            break;
                                    }
                                    break;
                                case QAPI_BLE_CRT_PASSKEY_E:
                                    LOG_VERBOSE("Call LEPasskeyResponse [PASSCODE].\n");
                                    break;
                                case QAPI_BLE_CRT_DISPLAY_E:
                                    LOG_INFO("Passkey: %06u.\n", (unsigned int)(Authentication_Event_Data->Authentication_Event_Data.Confirmation_Request.Display_Passkey));
                                    break;
                                default:
                                    LOG_VERBOSE("Authentication method not supported.\n");
                                    break;
                            }
                            break;
                        case QAPI_BLE_LAT_EXTENDED_CONFIRMATION_REQUEST_E:
                            LOG_VERBOSE("latExtendedConfirmationRequest.\n");

                            LOG_VERBOSE("   Secure Connections:     %s.\n", (Authentication_Event_Data->Authentication_Event_Data.Extended_Confirmation_Request.Flags & QAPI_BLE_GAP_LE_EXTENDED_CONFIRMATION_REQUEST_FLAGS_SECURE_CONNECTIONS)?"YES":"NO");
                            LOG_VERBOSE("   Just Works Pairing:     %s.\n", (Authentication_Event_Data->Authentication_Event_Data.Extended_Confirmation_Request.Flags & QAPI_BLE_GAP_LE_EXTENDED_CONFIRMATION_REQUEST_FLAGS_JUST_WORKS_PAIRING)?"YES":"NO");
                            LOG_VERBOSE("   Keypress Notifications: %s.\n", (Authentication_Event_Data->Authentication_Event_Data.Extended_Confirmation_Request.Flags & QAPI_BLE_GAP_LE_EXTENDED_CONFIRMATION_REQUEST_FLAGS_KEYPRESS_NOTIFICATIONS_REQUESTED)?"YES":"NO");

                            switch(Authentication_Event_Data->Authentication_Event_Data.Extended_Confirmation_Request.Request_Type)
                            {
                                case QAPI_BLE_CRT_NONE_E:
                                    GAP_LE_Authentication_Response_Information.GAP_LE_Authentication_Type = QAPI_BLE_LAR_CONFIRMATION_E;

                                    GAP_LE_Authentication_Response_Information.Authentication_Data_Length = sizeof(uint32_t);

                                    switch(LE_Parameters.IOCapability)
                                    {
                                        case QAPI_BLE_LIC_NO_INPUT_NO_OUTPUT_E:
                                            LOG_VERBOSE("Invoking Just Works.\n");

                                            Result = qapi_BLE_GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information);
                                            if(Result)
                                            {
                                                LOG_VERBOSE("qapi_BLE_GAP_LE_Authentication_Response returned %d.\n", Result);
                                            }
                                            break;
                                        case QAPI_BLE_LIC_DISPLAY_ONLY_E:
                                            LOG_VERBOSE("Confirmation of Pairing.\n");

                                            GAP_LE_Authentication_Response_Information.Authentication_Data.Passkey = Authentication_Event_Data->Authentication_Event_Data.Confirmation_Request.Display_Passkey;

                                            if((Result = qapi_BLE_GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information)) != 0)
                                                DisplayFunctionError("qapi_BLE_GAP_LE_Authentication_Response", Result);
                                            break;
                                        default:
                                            LOG_VERBOSE("Confirmation of Pairing.\n");

                                            if((Result = qapi_BLE_GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information)) != 0)
                                                DisplayFunctionError("qapi_BLE_GAP_LE_Authentication_Response", Result);
                                            break;
                                    }
                                    break;
                                case QAPI_BLE_CRT_PASSKEY_E:
                                    LOG_VERBOSE("Call LEPasskeyResponse [PASSKEY].\n");
                                    break;
                                case QAPI_BLE_CRT_DISPLAY_E:
                                    LOG_INFO("Passkey: %06u.\n", Authentication_Event_Data->Authentication_Event_Data.Extended_Confirmation_Request.Display_Passkey);

                                    GAP_LE_Authentication_Response_Information.GAP_LE_Authentication_Type  = QAPI_BLE_LAR_PASSKEY_E;
                                    GAP_LE_Authentication_Response_Information.Authentication_Data_Length  = (uint8_t)(sizeof(uint32_t));
                                    GAP_LE_Authentication_Response_Information.Authentication_Data.Passkey = Authentication_Event_Data->Authentication_Event_Data.Extended_Confirmation_Request.Display_Passkey;

                                    if((Result = qapi_BLE_GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information)) != 0)
                                        DisplayFunctionError("qapi_BLE_GAP_LE_Authentication_Response", Result);
                                    break;
                                case QAPI_BLE_CRT_DISPLAY_YES_NO_E:
                                    GAP_LE_Authentication_Response_Information.GAP_LE_Authentication_Type = QAPI_BLE_LAR_CONFIRMATION_E;

                                    GAP_LE_Authentication_Response_Information.Authentication_Data_Length = sizeof(uint32_t);

                                    if(Authentication_Event_Data->Authentication_Event_Data.Extended_Confirmation_Request.Flags & QAPI_BLE_GAP_LE_EXTENDED_CONFIRMATION_REQUEST_FLAGS_JUST_WORKS_PAIRING)
                                    {
                                        switch(LE_Parameters.IOCapability)
                                        {
                                            case QAPI_BLE_LIC_NO_INPUT_NO_OUTPUT_E:
                                                LOG_VERBOSE("Invoking Just Works.\n");

                                                Result = qapi_BLE_GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information);
                                                if(Result)
                                                {
                                                    LOG_VERBOSE("qapi_BLE_GAP_LE_Authentication_Response returned %d.\n", Result);
                                                }
                                                break;
                                            case QAPI_BLE_LIC_DISPLAY_ONLY_E:
                                                LOG_VERBOSE("Confirmation of Pairing.\n");

                                                GAP_LE_Authentication_Response_Information.Authentication_Data.Passkey = Authentication_Event_Data->Authentication_Event_Data.Confirmation_Request.Display_Passkey;

                                                if((Result = qapi_BLE_GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information)) != 0)
                                                    DisplayFunctionError("qapi_BLE_GAP_LE_Authentication_Response", Result);
                                                break;
                                            default:
                                                LOG_VERBOSE("Confirmation of Pairing.\n");

                                                if((Result = qapi_BLE_GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information)) != 0)
                                                    DisplayFunctionError("qapi_BLE_GAP_LE_Authentication_Response", Result);
                                                break;
                                        }
                                    }
                                    else
                                    {
                                        LOG_INFO("Confirmation Value: %ld\n", (unsigned long)Authentication_Event_Data->Authentication_Event_Data.Extended_Confirmation_Request.Display_Passkey);

                                        if((Result = qapi_BLE_GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information)) != 0)
                                            DisplayFunctionError("qapi_BLE_GAP_LE_Authentication_Response", Result);
                                        break;
                                    }
                                    break;
                                case QAPI_BLE_CRT_OOB_SECURE_CONNECTIONS_E:
                                    LOG_VERBOSE("OOB Data Request.\n");

                                    GAP_LE_Authentication_Response_Information.GAP_LE_Authentication_Type = QAPI_BLE_LAR_OUT_OF_BAND_DATA_E;
                                    GAP_LE_Authentication_Response_Information.Authentication_Data_Length = QAPI_BLE_GAP_LE_SECURE_CONNECTIONS_OOB_DATA_SIZE;
                                    if(RemoteOOBValid)
                                    {
                                        GAP_LE_Authentication_Response_Information.Authentication_Data.Secure_Connections_OOB_Data.Flags        = 0;
                                        GAP_LE_Authentication_Response_Information.Authentication_Data.Secure_Connections_OOB_Data.Confirmation = RemoteOOBConfirmation;
                                        GAP_LE_Authentication_Response_Information.Authentication_Data.Secure_Connections_OOB_Data.Randomizer   = RemoteOOBRandomizer;
                                    }
                                    else
                                        GAP_LE_Authentication_Response_Information.Authentication_Data.Secure_Connections_OOB_Data.Flags = QAPI_BLE_GAP_LE_SECURE_CONNECTIONS_OOB_DATA_FLAGS_OOB_NOT_RECEIVED;

                                    if((Result = qapi_BLE_GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information)) != 0)
                                        DisplayFunctionError("qapi_BLE_GAP_LE_Authentication_Response", Result);
                                    break;
                                default:
                                    LOG_ERROR("Authentication method not supported.\n");
                                    break;
                            }
                            break;
                        case QAPI_BLE_LAT_SECURITY_ESTABLISHMENT_COMPLETE_E:
                            LOG_VERBOSE("Security Re-Establishment Complete: %s.\n", BoardStr);
                            LOG_VERBOSE("                            Status: 0x%02X.\n", Authentication_Event_Data->Authentication_Event_Data.Security_Establishment_Complete.Status);

                            if(Authentication_Event_Data->Authentication_Event_Data.Security_Establishment_Complete.Status == QAPI_BLE_GAP_LE_SECURITY_ESTABLISHMENT_STATUS_CODE_LONG_TERM_KEY_ERROR)
                            {
                                if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, Authentication_Event_Data->BD_ADDR)) != NULL)
                                {
                                    DeviceInfo->Flags &= ~DEVICE_INFO_FLAGS_LTK_VALID;
                                }
                            }

                            QAPI_BLE_ASSIGN_BD_ADDR(SecurityRemoteBD_ADDR, 0, 0, 0, 0, 0, 0);
                            break;
                        case QAPI_BLE_LAT_PAIRING_STATUS_E:
                            LOG_VERBOSE("Pairing Status: %s.\n", BoardStr);
                            LOG_VERBOSE("        Status: 0x%02X.\n", Authentication_Event_Data->Authentication_Event_Data.Pairing_Status.Status);

                            if(Authentication_Event_Data->Authentication_Event_Data.Pairing_Status.Status == QAPI_BLE_GAP_LE_PAIRING_STATUS_NO_ERROR)
                            {
                                LOG_VERBOSE("        Key Size: %d.\n", Authentication_Event_Data->Authentication_Event_Data.Pairing_Status.Negotiated_Encryption_Key_Size);
                            }
                            else
                            {
                                qapi_BLE_GAP_LE_Disconnect(BluetoothStackID, Authentication_Event_Data->BD_ADDR);
                            }

                            QAPI_BLE_ASSIGN_BD_ADDR(SecurityRemoteBD_ADDR, 0, 0, 0, 0, 0, 0);
                            break;
                        case QAPI_BLE_LAT_ENCRYPTION_INFORMATION_REQUEST_E:
                            LOG_VERBOSE("Encryption Information Request %s.\n", BoardStr);
                            break;
                        case QAPI_BLE_LAT_IDENTITY_INFORMATION_REQUEST_E:
                            LOG_VERBOSE("Identity Information Request %s.\n", BoardStr);

                            GAP_LE_Authentication_Response_Information.GAP_LE_Authentication_Type                            = QAPI_BLE_LAR_IDENTITY_INFORMATION_E;
                            GAP_LE_Authentication_Response_Information.Authentication_Data_Length                            = (uint8_t)QAPI_BLE_GAP_LE_IDENTITY_INFORMATION_DATA_SIZE;
                            GAP_LE_Authentication_Response_Information.Authentication_Data.Identity_Information.Address      = LocalBD_ADDR;
                            GAP_LE_Authentication_Response_Information.Authentication_Data.Identity_Information.Address_Type = QAPI_BLE_LAT_PUBLIC_IDENTITY_E;
                            GAP_LE_Authentication_Response_Information.Authentication_Data.Identity_Information.IRK          = IRK;

                            Result = qapi_BLE_GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information);
                            if(!Result)
                            {
                                LOG_INFO("   qapi_BLE_GAP_LE_Authentication_Response (larEncryptionInformation) success.\n");
                            }
                            else
                            {
                                LOG_ERROR("   Error - SM_Generate_Long_Term_Key returned %d.\n", Result);
                            }
                            break;
                        case QAPI_BLE_LAT_ENCRYPTION_INFORMATION_E:
                            LOG_VERBOSE(" Encryption Information from RemoteDevice: %s.\n", BoardStr);
                            LOG_VERBOSE("                             Key Size: %d.\n", Authentication_Event_Data->Authentication_Event_Data.Encryption_Information.Encryption_Key_Size);

                            if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, Authentication_Event_Data->BD_ADDR)) != NULL)
                            {
                                memcpy(&(DeviceInfo->LTK), &(Authentication_Event_Data->Authentication_Event_Data.Encryption_Information.LTK), sizeof(DeviceInfo->LTK));
                                DeviceInfo->EDIV              = Authentication_Event_Data->Authentication_Event_Data.Encryption_Information.EDIV;
                                memcpy(&(DeviceInfo->Rand), &(Authentication_Event_Data->Authentication_Event_Data.Encryption_Information.Rand), sizeof(DeviceInfo->Rand));
                                DeviceInfo->EncryptionKeySize = Authentication_Event_Data->Authentication_Event_Data.Encryption_Information.Encryption_Key_Size;
                                DeviceInfo->Flags            |= DEVICE_INFO_FLAGS_LTK_VALID;
                            }
                            else
                            {
                                LOG_ERROR("No Key Info Entry for this device.\n");
                            }
                            break;
                        case QAPI_BLE_LAT_IDENTITY_INFORMATION_E:
                            LOG_VERBOSE(" Identity Information from RemoteDevice: %s.\n", BoardStr);

                            if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, Authentication_Event_Data->BD_ADDR)) != NULL)
                            {
                                memcpy(&(DeviceInfo->IRK), &(Authentication_Event_Data->Authentication_Event_Data.Identity_Information.IRK), sizeof(DeviceInfo->IRK));
                                DeviceInfo->IdentityAddressBD_ADDR = Authentication_Event_Data->Authentication_Event_Data.Identity_Information.Address;
                                DeviceInfo->IdentityAddressType    = Authentication_Event_Data->Authentication_Event_Data.Identity_Information.Address_Type;
                                DeviceInfo->Flags                 |= DEVICE_INFO_FLAGS_IRK_VALID;

                                DeviceInfo->ResolvingListEntry.Peer_Identity_Address      = DeviceInfo->IdentityAddressBD_ADDR;
                                DeviceInfo->ResolvingListEntry.Peer_Identity_Address_Type = DeviceInfo->IdentityAddressType;
                                DeviceInfo->ResolvingListEntry.Peer_IRK                   = DeviceInfo->IRK;
                                DeviceInfo->ResolvingListEntry.Local_IRK                  = IRK;
                            }
                            else
                            {
                                LOG_ERROR("No Key Info Entry for this device.\n");
                            }
                            break;
                        default:
                            LOG_VERBOSE("Unhandled event: %u\n", Authentication_Event_Data->GAP_LE_Authentication_Event_Type);
                            break;
                    }
                }
                break;
            case QAPI_BLE_ET_LE_DIRECT_ADVERTISING_REPORT_E:
                LOG_INFO("etLE_Direct_Advertising_Report with size %d.\n", (int)GAP_LE_Event_Data->Event_Data_Size);
                LOG_VERBOSE("  %d Responses.\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Direct_Advertising_Report_Event_Data->Number_Device_Entries);

                for(Index = 0; Index < GAP_LE_Event_Data->Event_Data.GAP_LE_Direct_Advertising_Report_Event_Data->Number_Device_Entries; Index++)
                {
                    DirectDeviceEntryPtr = &(GAP_LE_Event_Data->Event_Data.GAP_LE_Direct_Advertising_Report_Event_Data->Direct_Advertising_Data[Index]);

                    switch(DirectDeviceEntryPtr->Address_Type)
                    {
                        case QAPI_BLE_LAT_PUBLIC_E:
                            LOG_VERBOSE("  Address Type: %s.\n", "QAPI_BLE_LAT_PUBLIC_E");
                            break;
                        case QAPI_BLE_LAT_RANDOM_E:
                            LOG_VERBOSE("  Address Type: %s.\n", "QAPI_BLE_LAT_RANDOM_E");
                            break;
                        case QAPI_BLE_LAT_PUBLIC_IDENTITY_E:
                            LOG_VERBOSE("  Address Type: %s.\n", "QAPI_BLE_LAT_PUBLIC_IDENTITY_E");
                            break;
                        case QAPI_BLE_LAT_RANDOM_IDENTITY_E:
                            LOG_VERBOSE("  Address Type: %s.\n", "QAPI_BLE_LAT_RANDOM_IDENTITY_E");
                            break;
                        default:
                            LOG_VERBOSE("  Address Type: Invalid.\n");
                            break;
                    }

                    LOG_VERBOSE("  Address:      0x%02X%02X%02X%02X%02X%02X.\n", DirectDeviceEntryPtr->BD_ADDR.BD_ADDR5, DirectDeviceEntryPtr->BD_ADDR.BD_ADDR4, DirectDeviceEntryPtr->BD_ADDR.BD_ADDR3, DirectDeviceEntryPtr->BD_ADDR.BD_ADDR2, DirectDeviceEntryPtr->BD_ADDR.BD_ADDR1, DirectDeviceEntryPtr->BD_ADDR.BD_ADDR0);
                    LOG_VERBOSE("  RSSI:         %d.\n", (int)(DirectDeviceEntryPtr->RSSI));
                }
                break;
            default:
                DisplayPrompt = false;
                break;
        }

        if(DisplayPrompt)
            QCLI_Display_Prompt();
    }
}

int32_t enable_advertise(void)
{
    offline_onboard=1;    /* Take care of Resetting the value, While Deboarding feature is implemented, */
    AdvertiseLE(0);
    AdvertiseLE(1);
    return 1;
}

int32_t enable_cordinator_fn(void)
{
    enable_cordinator=1;
    AdvertiseLE(0);
    AdvertiseLE(1);
    return 1;
}
/**
 * @func : AdvertiseLE
 * @Desc : The following function is responsible for enabling/disabling LE
 *         Advertisements
 */
int AdvertiseLE(uint32_t enable)
{
    int                                         Result;
    int                                         ret_val;
    unsigned int                                Length;
    unsigned int                                UUIDIndex;
    char                                        Name[QAPI_BLE_GAP_MAXIMUM_DEVICE_NAME_LENGTH+1];
    unsigned int                                NameLength;
    uint16_t                                    DeviceAppearance;
    qapi_BLE_GAP_LE_Connectability_Parameters_t ConnectabilityParameters;
    union
    {
        qapi_BLE_Advertising_Data_t              AdvertisingData;
        qapi_BLE_Scan_Response_Data_t            ScanResponseData;
    } Advertisement_Data_Buffer;


    /* First, check that valid Bluetooth Stack ID exists.                */
    if (BluetoothStackID)
    {
        /* Make sure that all of the parameters required for this function*/
        /* appear to be at least semi-valid.                              */
#ifndef V1
        if ((enable >= AET_DISABLE_E) && (enable <= AET_ENABLE_CHANNEL_39_E))
#else
            if ((enable >= 0) && (enable <= 1))
#endif
            {
                /* Determine whether to enable or disable Advertising.         */
#ifndef V1
                if (enable == 0)
#else
                    if (enable == AET_DISABLE_E)
#endif
                    {
                        /* Disable Advertising.                                     */
                        Result = qapi_BLE_GAP_LE_Advertising_Disable(BluetoothStackID);
                        if(!Result)
                        {
                            LOG_INFO("   GAP_LE_Advertising_Disable success.\n");

                            ret_val = QCLI_STATUS_SUCCESS_E;
                        }
                        else
                        {
                            LOG_ERROR("   GAP_LE_Advertising_Disable returned %d.\n", Result);

                            ret_val = QCLI_STATUS_ERROR_E;
                        }
                    }
                    else
                    {
                        /* Set the Advertising Data.                                */
                        memset(&(Advertisement_Data_Buffer.AdvertisingData), 0, sizeof(qapi_BLE_Advertising_Data_t));

                        /* Reset the length to zero.                                */
                        Length = 0;

                        /* Set the Flags A/D Field (1 byte type and 1 byte Flags.   */
                        Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[Length]   = 2;
                        Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[Length+1] = QAPI_BLE_HCI_LE_ADVERTISING_REPORT_DATA_TYPE_FLAGS;
                        Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[Length+2] = 0;

                        /* Configure the flags field based on the Discoverability   */
                        /* Mode.                                                    */
                        if(LE_Parameters.DiscoverabilityMode == QAPI_BLE_DM_GENERAL_DISCOVERABLE_MODE_E)
                            Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[Length+2]    = QAPI_BLE_HCI_LE_ADVERTISING_FLAGS_GENERAL_DISCOVERABLE_MODE_FLAGS_BIT_MASK;
                        else
                        {
                            if(LE_Parameters.DiscoverabilityMode == QAPI_BLE_DM_LIMITED_DISCOVERABLE_MODE_E)
                                Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[Length+2] = QAPI_BLE_HCI_LE_ADVERTISING_FLAGS_LIMITED_DISCOVERABLE_MODE_FLAGS_BIT_MASK;
                        }

                        Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[Length + 2] |= QAPI_BLE_HCI_LE_ADVERTISING_FLAGS_BR_EDR_NOT_SUPPORTED_FLAGS_BIT_MASK;

                        /* Update the current length of the advertising data.       */
                        /* * NOTE * We MUST add one to account for the length field,*/
                        /*          which does not include itself.                  */
                        Length += (Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[Length] + 1);

                        /* Include the GAPS Device Appearance.                      */
                        if((Result = qapi_BLE_GAPS_Query_Device_Appearance(BluetoothStackID, (uint32_t)GAPSInstanceID, &DeviceAppearance)) == 0)
                        {
                            /* Make sure we do not exceed the bounds of the buffer.  */
                            if((Length + (unsigned int)QAPI_BLE_NON_ALIGNED_WORD_SIZE + 2) <= (unsigned int)QAPI_BLE_ADVERTISING_DATA_MAXIMUM_SIZE)
                            {
                                ((uint8_t *)&Advertisement_Data_Buffer.AdvertisingData)[Length]   = 3;
                                ((uint8_t *)&Advertisement_Data_Buffer.AdvertisingData)[Length+1] = QAPI_BLE_HCI_LE_ADVERTISING_REPORT_DATA_TYPE_APPEARANCE;
                                ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&(((uint8_t *)&Advertisement_Data_Buffer.AdvertisingData)[Length+2]), DeviceAppearance);

                                /* Update the current length of the advertising data. */
                                /* * NOTE * We MUST add one to account for the length */
                                /*          field, which does not include itself.     */
                                Length += (((uint8_t *)&Advertisement_Data_Buffer.AdvertisingData)[Length] + 1);
                            }
                            else
                                LOG_WARN("   The device appearance CANNOT fit in the advertising data.\n", Result);
                        }
                        else
                            LOG_ERROR("   qapi_BLE_GAPS_Query_Device_Appearance(dtAdvertising) returned %d.\n", Result);

                        /* Include the 16-bit service UUIDs if the service is       */
                        /* registered.                                              */
                        /* * NOTE * SPPLE is excluded since it has a 128 bit UUID.  */

                        /* Make sure we have room in the buffer.                    */
                        /* * NOTE * We will make sure we have room for at least one */
                        /*          16-bit UUID.                                    */
                        if((Length + (unsigned int)QAPI_BLE_UUID_16_SIZE + 2) <= (unsigned int)QAPI_BLE_ADVERTISING_DATA_MAXIMUM_SIZE)
                        {
                            ((uint8_t *)&Advertisement_Data_Buffer.AdvertisingData)[Length]   = 1;
                            ((uint8_t *)&Advertisement_Data_Buffer.AdvertisingData)[Length+1] = QAPI_BLE_HCI_LE_ADVERTISING_REPORT_DATA_TYPE_16_BIT_SERVICE_UUID_COMPLETE;

                            /* Store the UUID Index location.                        */
                            /* * NOTE * We will do this to make the code more        */
                            /*          readable.                                    */
                            UUIDIndex = (Length + 2);

                            /* If DIS is registered.                                 */
                            if(DISInstanceID)
                            {
                                /* Make sure we do not exceed the bounds of the       */
                                /* buffer.                                            */
                                /* * NOTE * We MUST add one to account for the length */
                                /*          field, which does not include itself.     */
                                if((Length + ((uint8_t *)&Advertisement_Data_Buffer.AdvertisingData)[Length] + 1 + QAPI_BLE_UUID_16_SIZE) <= (unsigned int)QAPI_BLE_ADVERTISING_DATA_MAXIMUM_SIZE)
                                {
                                    /* Assign the DIS Service UUID.                    */
                                    QAPI_BLE_DIS_ASSIGN_DIS_SERVICE_UUID_16(&(((uint8_t *)&Advertisement_Data_Buffer.AdvertisingData)[UUIDIndex]));

                                    /* Update the UUID Index.                          */
                                    UUIDIndex += QAPI_BLE_UUID_16_SIZE;

                                    /* Update the advertising report data entry length */
                                    /* since we have added another UUID.               */
                                    ((uint8_t *)&Advertisement_Data_Buffer.AdvertisingData)[Length] += QAPI_BLE_UUID_16_SIZE;
#ifdef OFFLINE
#define QAPI_BLE_DIS_ASSIGN_OFFLINE_SERVICE_UUID_16(_x)                           QAPI_BLE_ASSIGN_BLUETOOTH_UUID_16(*((qapi_BLE_UUID_16_t *)(_x)), 0x18, 0x18)               

                                    QAPI_BLE_DIS_ASSIGN_OFFLINE_SERVICE_UUID_16(&(((uint8_t *)&Advertisement_Data_Buffer.AdvertisingData)[UUIDIndex]));

                                    /* Update the UUID Index.                          */
                                    UUIDIndex += QAPI_BLE_UUID_16_SIZE;

                                    /* Update the advertising report data entry length */
                                    /* since we have added another UUID.               */
                                    ((uint8_t *)&Advertisement_Data_Buffer.AdvertisingData)[Length] += QAPI_BLE_UUID_16_SIZE;
#endif

                                    if (offline_onboard)
                                    {
                                        LOG_INFO(" Offline Onboard UUID added\n");
#define QAPI_BLE_DIS_ONBOARD_OFFLINE_SERVICE_UUID_16(_x)             QAPI_BLE_ASSIGN_BLUETOOTH_UUID_16(*((qapi_BLE_UUID_16_t *)(_x)), 0x19, 0x19)
                                        QAPI_BLE_DIS_ONBOARD_OFFLINE_SERVICE_UUID_16(&(((uint8_t *)&Advertisement_Data_Buffer.AdvertisingData)[UUIDIndex]));
                                        /* Update the UUID Index.                          */
                                        UUIDIndex += QAPI_BLE_UUID_16_SIZE;

                                        /* Update the advertising report data entry length */
                                        /* since we have added another UUID.               */
                                        ((uint8_t *)&Advertisement_Data_Buffer.AdvertisingData)[Length] += QAPI_BLE_UUID_16_SIZE;
                                    }

                                    if (enable_cordinator)
                                    {
                                        LOG_INFO(" Enable Co-ordinator UUID added\n");
#define QAPI_BLE_ASSIGN_COORDINATOR_SERVICE_UUID_16(_x)                           QAPI_BLE_ASSIGN_BLUETOOTH_UUID_16(*((qapi_BLE_UUID_16_t *)(_x)), 0x20, 0x20) 
                                        QAPI_BLE_ASSIGN_COORDINATOR_SERVICE_UUID_16(&(((uint8_t *)&Advertisement_Data_Buffer.AdvertisingData)[UUIDIndex]));

                                        /* Update the UUID Index.                          */
                                        UUIDIndex += QAPI_BLE_UUID_16_SIZE;

                                        /* Update the advertising report data entry length */
                                        /* since we have added another UUID.               */
                                        ((uint8_t *)&Advertisement_Data_Buffer.AdvertisingData)[Length] += QAPI_BLE_UUID_16_SIZE;
                                    }

                                }
                                else
{
                                    /* Flag that we could not include all the service  */
                                    /* UUIDs.                                          */
                                    ((uint8_t *)&Advertisement_Data_Buffer.AdvertisingData)[Length + 1] = QAPI_BLE_HCI_LE_ADVERTISING_REPORT_DATA_TYPE_16_BIT_SERVICE_UUID_PARTIAL;
                                }
                            }

                            /* Update the current length of the advertising data.    */
                            /* * NOTE * We MUST add one to account for the length    */
                            /*          field, which does not include itself.        */
                            Length += (((uint8_t *)&Advertisement_Data_Buffer.AdvertisingData)[Length] + 1);
                        }
                        else
                            LOG_WARN("   The 16-bit service UUID's CANNOT fit in the advertising data.\n");

                        /* Write the advertising data to the chip.                  */
                        Result = qapi_BLE_GAP_LE_Set_Advertising_Data(BluetoothStackID, Length, &(Advertisement_Data_Buffer.AdvertisingData));
                        if(!Result)
                        {
                            /* Initialize the scan response data.                    */
                            memset(&(Advertisement_Data_Buffer.ScanResponseData), 0, sizeof(qapi_BLE_Scan_Response_Data_t));

                            /* Reset the length to zero.                             */
                            Length = 0;

                            /* First include the local device name.                  */
                            if((Result = qapi_BLE_GAPS_Query_Device_Name(BluetoothStackID, (uint32_t)GAPSInstanceID, Name)) == 0)
                            {
                                /* Get the name length.                               */
                                NameLength = strlen(Name);

                                /* Determine if we need to truncate the device name.  */
                                if(NameLength < ((unsigned int)QAPI_BLE_SCAN_RESPONSE_DATA_MAXIMUM_SIZE - 2))
                                {
                                    /* Format the complete device name into the scan   */
                                    /* response data.                                  */
                                    ((uint8_t *)&Advertisement_Data_Buffer.ScanResponseData)[Length] = (uint8_t)(1 + NameLength);
                                    ((uint8_t *)&Advertisement_Data_Buffer.ScanResponseData)[Length + 1] = QAPI_BLE_HCI_LE_ADVERTISING_REPORT_DATA_TYPE_LOCAL_NAME_COMPLETE;
                                }
                                else
                                {
                                    /* Format the truncated device name into the scan  */
                                    /* response data.                                  */
                                    NameLength = ((unsigned int)QAPI_BLE_SCAN_RESPONSE_DATA_MAXIMUM_SIZE - 2);
                                    ((uint8_t *)&Advertisement_Data_Buffer.ScanResponseData)[Length] = (uint8_t)(1 + NameLength);
                                    ((uint8_t *)&Advertisement_Data_Buffer.ScanResponseData)[Length + 1] = QAPI_BLE_HCI_LE_ADVERTISING_REPORT_DATA_TYPE_LOCAL_NAME_SHORTENED;
                                }

                                /* Make sure we can fit the device name into the scan */
                                /* response data.                                     */
                                if((Length + NameLength + 2) <= (unsigned int)(QAPI_BLE_SCAN_RESPONSE_DATA_MAXIMUM_SIZE - 2))
                                {
                                    /* Simply copy the name into the scan response     */
                                    /* data.                                           */
                                    memcpy(&(((uint8_t *)&Advertisement_Data_Buffer.ScanResponseData)[Length + 2]), Name, NameLength);

                                    /* Update the current length of the scan response  */
                                    /* data.                                           */
                                    /* * NOTE * We MUST add one to account for the     */
                                    /*          length field, which does not include   */
                                    /*          itself.                                */
                                    Length += (((uint8_t *)&Advertisement_Data_Buffer.ScanResponseData)[Length] + 1);
                                }
                                else
                                    LOG_WARN("   The device name CANNOT fit in the scan response data.\n", Result);
                            }
                            else
                                LOG_ERROR("   qapi_BLE_GAPS_Query_Device_Name(dtAdvertising) returned %d.\n", Result);

                            Result = qapi_BLE_GAP_LE_Set_Scan_Response_Data(BluetoothStackID, Length, &(Advertisement_Data_Buffer.ScanResponseData));
                            if(!Result)
                            {
                                /* Configure the advertising channel map for the      */
                                /* default advertising parameters.                    */
#ifndef V1
                                BLEParameters.AdvertisingParameters.Advertising_Channel_Map = QAPI_BLE_HCI_LE_ADVERTISING_CHANNEL_MAP_DEFAULT;
#else
                                switch(enable)
                                {
                                    case AET_ENABLE_ALL_E:
                                        BLEParameters.AdvertisingParameters.Advertising_Channel_Map = (QAPI_BLE_HCI_LE_ADVERTISING_CHANNEL_MAP_ENABLE_CHANNEL_37 | QAPI_BLE_HCI_LE_ADVERTISING_CHANNEL_MAP_ENABLE_CHANNEL_38);
                                        break;
                                    case AET_ENABLE_CHANNEL_37_E:
                                        BLEParameters.AdvertisingParameters.Advertising_Channel_Map = QAPI_BLE_HCI_LE_ADVERTISING_CHANNEL_MAP_ENABLE_CHANNEL_37;
                                        break;
                                    case AET_ENABLE_CHANNEL_38_E:
                                        BLEParameters.AdvertisingParameters.Advertising_Channel_Map = QAPI_BLE_HCI_LE_ADVERTISING_CHANNEL_MAP_ENABLE_CHANNEL_38;
                                        break;
                                    case AET_ENABLE_CHANNEL_39_E:
                                        BLEParameters.AdvertisingParameters.Advertising_Channel_Map = QAPI_BLE_HCI_LE_ADVERTISING_CHANNEL_MAP_ENABLE_CHANNEL_39;
                                        break;
                                    default:
                                        /* Enable all channels if the user specified and*/
                                        /* invalid enumeration.                         */
                                        BLEParameters.AdvertisingParameters.Advertising_Channel_Map = (QAPI_BLE_HCI_LE_ADVERTISING_CHANNEL_MAP_ENABLE_CHANNEL_37 | QAPI_BLE_HCI_LE_ADVERTISING_CHANNEL_MAP_ENABLE_CHANNEL_38);
                                        break;
                                }
#endif

                                /* Set up the default advertising parameters if they  */
                                /* have not been configured at the CLI.               */
                                if(!(BLEParameters.Flags & BLE_PARAMETERS_FLAGS_ADVERTISING_PARAMETERS_VALID))
                                {
                                    /* Configure the remaining default advertising     */
                                    /* parameters.                                     */
                                    BLEParameters.AdvertisingParameters.Scan_Request_Filter       = QAPI_BLE_FP_NO_FILTER_E;
                                    BLEParameters.AdvertisingParameters.Connect_Request_Filter    = QAPI_BLE_FP_NO_FILTER_E;
                                    BLEParameters.AdvertisingParameters.Advertising_Interval_Min  = 100;
                                    BLEParameters.AdvertisingParameters.Advertising_Interval_Max  = 200;

                                    /* Flag that the parameters are valid so we don't  */
                                    /* set them unnecessarily.                         */
                                    BLEParameters.Flags |= BLE_PARAMETERS_FLAGS_ADVERTISING_PARAMETERS_VALID;
                                }

                                /* Configure the Connectability Parameters.           */
                                /* * NOTE * We will ALWAYS advertise                  */
                                ConnectabilityParameters.Connectability_Mode = LE_Parameters.ConnectableMode;
                                ConnectabilityParameters.Own_Address_Type    = QAPI_BLE_LAT_PUBLIC_E;

                                /* If the connectable mode is set for direct          */
                                /* connectable.                                       */
                                if((ConnectabilityParameters.Connectability_Mode == QAPI_BLE_LCM_LOW_DUTY_CYCLE_DIRECT_CONNECTABLE_E) || (ConnectabilityParameters.Connectability_Mode == QAPI_BLE_LCM_DIRECT_CONNECTABLE_E))
                                {
                                    /* We will set our own address type to resolvable  */
                                    /* fallback public.                                */
                                    ConnectabilityParameters.Own_Address_Type = QAPI_BLE_LAT_RESOLVABLE_FALLBACK_PUBLIC_E;
                                }

                                /* Initialize the direct address to zero and the type */
                                /* to public.                                         */
                                /* * NOTE * If the ConnectableMode is set to one of   */
                                /*          the Direct Connectable types, then the    */
                                /*          direct address and type MUST be specified.*/
                                /*          If they are NOT specified, then           */
                                /*          qapi_BLE_GAP_LE_Advertising_Enable() will */
                                /*          fail.                                     */
                                ConnectabilityParameters.Direct_Address_Type   = QAPI_BLE_LAT_PUBLIC_E;
                                QAPI_BLE_ASSIGN_BD_ADDR(ConnectabilityParameters.Direct_Address, 0, 0, 0, 0, 0, 0);


                                /* If the user did NOT supply the direct address,  */
                                /* then we need to make sure we are NOT in the a   */
                                /* direct connectable mode.                        */
                                if((ConnectabilityParameters.Connectability_Mode == QAPI_BLE_LCM_LOW_DUTY_CYCLE_DIRECT_CONNECTABLE_E) || (ConnectabilityParameters.Connectability_Mode == QAPI_BLE_LCM_DIRECT_CONNECTABLE_E))
                                {
                                    /* Override the local device's connectable mode.*/
                                    /* * NOTE * If this is NOT done, then           */
                                    /*          qapi_BLE_GAP_LE_Advertising_Enable()*/
                                    /*          will fail.                          */
                                    ConnectabilityParameters.Own_Address_Type    = QAPI_BLE_LAT_PUBLIC_E;
                                    ConnectabilityParameters.Connectability_Mode = QAPI_BLE_LCM_CONNECTABLE_E;

                                    /* Inform the user.                             */
                                    LOG_WARN("Using connectable un-directed advertising with public address.\n");
                                }

                                /* Now enable advertising.                         */
                                Result = qapi_BLE_GAP_LE_Advertising_Enable(BluetoothStackID, TRUE, &(BLEParameters.AdvertisingParameters), &(ConnectabilityParameters), GAP_LE_Event_Callback, 0);
                                if(!Result)
                                {
                                    LOG_INFO("   GAP_LE_Advertising_Enable success, Advertising Interval Range: %u - %u.\n", (unsigned int)BLEParameters.AdvertisingParameters.Advertising_Interval_Min, (unsigned int)BLEParameters.AdvertisingParameters.Advertising_Interval_Max);

                                    ret_val = QCLI_STATUS_SUCCESS_E;
                                }
                                else
                                {
                                    LOG_ERROR("   GAP_LE_Advertising_Enable returned %d.\n", Result);

                                    ret_val = QCLI_STATUS_ERROR_E;
                                }
                            }
                            else
                            {
                                LOG_ERROR("   qapi_BLE_GAP_LE_Set_Advertising_Data(dtScanResponse) returned %d.\n", Result);

                                ret_val = QCLI_STATUS_ERROR_E;
                            }

                        }
                        else
                        {
                            LOG_ERROR("   qapi_BLE_GAP_LE_Set_Advertising_Data(dtAdvertising) returned %d.\n", Result);

                            ret_val = QCLI_STATUS_ERROR_E;
                        }
                    }
            }
            else
            {
                ret_val = QCLI_STATUS_USAGE_E;
            }
    }
    else
        ret_val = QCLI_STATUS_ERROR_E;

    return(ret_val);
}

/**
 * @func : GetBLEStackID
 * @Desc : The following function is responsible for enabling/disabling LE
 *         Advertisements
 */
uint32_t GetBluetoothStackID()
{
    return BluetoothStackID;
}

/**
 * @func : ble_get_device_mac_address
 * @Desc : gets the device mac address
 */
int32_t ble_get_device_mac_address(uint8_t *mac)
{
    qapi_BLE_BD_ADDR_t Bdaddr;
    uint32_t i;

    if (qapi_BLE_GAP_Query_Local_BD_ADDR(BluetoothStackID, &Bdaddr))
        return FAILURE;

    for (i = 0; i < 6; i++)
        *(mac + i) = *((uint8_t *) &Bdaddr + (5 - i));
    return SUCCESS;
}
