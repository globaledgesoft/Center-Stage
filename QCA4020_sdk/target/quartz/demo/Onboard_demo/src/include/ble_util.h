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

#ifndef __BLE_UTIL_H__
#define __BLE_UTIL_H__

#define QAPI_USE_BLE

#include "qapi.h"
#include "qapi_ble_bttypes.h"
#include "qcli_api.h"
#include "qapi_persist.h"
#include "log_util.h"

#ifndef ONBOARD_DATA_BUFFER_LENGTH
#define ONBOARD_DATA_BUFFER_LENGTH                                       (517)
#endif

#define BLE_PASSKEY                                                      123456

#define BLE_NAME_SIZE                                                    32

#define DEFAULT_MITM_PROTECTION                                          (TRUE)
#define DEFAULT_SECURE_CONNECTIONS                                       (TRUE)
#define DEVICE_FRIENDLY_NAME                                             "QCA402x_BLE"

#define DEVICE_INFO_FLAGS_LTK_VALID                                      0x01
#define DEVICE_INFO_FLAGS_ONBOARD_SERVER                                 0x02
#define DEVICE_INFO_FLAGS_ONBOARD_CLIENT                                 0x04
#define DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_OUTSTANDING                  0x08
#define DEVICE_INFO_FLAGS_IRK_VALID                                      0x10
#define DEVICE_INFO_FLAGS_ADDED_TO_WHITE_LIST                            0x20
#define DEVICE_INFO_FLAGS_ADDED_TO_RESOLVING_LIST                        0x40

#define BLE_PARAMETERS_FLAGS_ADVERTISING_PARAMETERS_VALID                0x00000001
#define BLE_PARAMETERS_FLAGS_SCAN_PARAMETERS_VALID                       0x00000002
#define BLE_PARAMETERS_FLAGS_CONNECTION_PARAMETERS_VALID                 0x00000004


typedef struct _tagOnboard_Client_Info_t
{
    uint16_t Tx_Characteristic;
    uint16_t Tx_Client_Configuration_Descriptor;
    uint16_t Rx_Characteristic;
    uint16_t Tx_Credit_Characteristic;
    uint16_t Rx_Credit_Characteristic;
    uint16_t Rx_Credit_Client_Configuration_Descriptor;
} Onboard_Client_Info_t;

#define ONBOARD_CLIENT_INFO_DATA_SIZE                      (sizeof(Onboard_Client_Info_t))

typedef struct _tagOnboard_Server_Info_t
{
    uint16_t Tx_Client_Configuration_Descriptor;
    uint16_t Rx_Credit_Client_Configuration_Descriptor;
} Onboard_Server_Info_t;

#define ONBOARD_SERVER_INFO_DATA_SIZE                      (sizeof(Onboard_Server_Info_t))

#define ONBOARD_SERVICE_FLAGS                              (QAPI_BLE_GATT_SERVICE_FLAGS_LE_SERVICE)

/**
 * Necessary Structure Declarations
 */
typedef struct _tagGAPLE_Parameters_t
{
    qapi_BLE_GAP_LE_Connectability_Mode_t ConnectableMode;
    qapi_BLE_GAP_Discoverability_Mode_t   DiscoverabilityMode;
    qapi_BLE_GAP_LE_IO_Capability_t       IOCapability;
    boolean_t                             MITMProtection;
    boolean_t                             SecureConnections;
    boolean_t                             OOBDataPresent;
} GAPLE_Parameters_t;

#define GAPLE_PARAMETERS_DATA_SIZE                       (sizeof(GAPLE_Parameters_t))

typedef struct _tagGAPS_Client_Info_t
{
    uint16_t DeviceNameHandle;
    uint16_t DeviceAppearanceHandle;
} GAPS_Client_Info_t;

typedef struct _tagSend_Info_t
{
    uint32_t BytesToSend;
    uint32_t BytesSent;
} Send_Info_t;

typedef struct _tagGAPS_Device_Appearance_Mapping_t
{
    uint16_t  Appearance;
    char     *String;
} GAPS_Device_Appearance_Mapping_t;

typedef struct __tagOnboard_Data_Buffer_t
{
    unsigned int  InIndex;
    unsigned int  OutIndex;
    unsigned int  BytesFree;
    unsigned int  BufferSize;
    uint8_t       Buffer[ONBOARD_DATA_BUFFER_LENGTH*3];
} Onboard_Data_Buffer_t;


typedef struct _tagDeviceInfo_t
{
    uint8_t                                Flags;
    unsigned int                           ConnectionID;
    boolean_t                              RemoteDeviceIsMaster;
    qapi_BLE_BD_ADDR_t                     RemoteAddress;
    qapi_BLE_GAP_LE_Address_Type_t         RemoteAddressType;
    qapi_BLE_GAP_LE_Address_Type_t         IdentityAddressType;
    qapi_BLE_BD_ADDR_t                     IdentityAddressBD_ADDR;
    uint8_t                                EncryptionKeySize;
    qapi_BLE_Long_Term_Key_t               LTK;
    qapi_BLE_Encryption_Key_t              IRK;
    qapi_BLE_Random_Number_t               Rand;
    uint16_t                               EDIV;
    qapi_BLE_GAP_LE_White_List_Entry_t     WhiteListEntry;
    qapi_BLE_GAP_LE_Resolving_List_Entry_t ResolvingListEntry;
    GAPS_Client_Info_t                     GAPSClientInfo;
    Onboard_Client_Info_t                  ClientInfo;
    Onboard_Server_Info_t                  ServerInfo;
    unsigned int                           TransmitCredits;
    Onboard_Data_Buffer_t                  ReceiveBuffer;
    Onboard_Data_Buffer_t                  TransmitBuffer;
    boolean_t                              ThroughputModeActive;
    struct _tagDeviceInfo_t               *NextDeviceInfoInfoPtr;
} DeviceInfo_t;

#define DEVICE_INFO_DATA_SIZE                            (sizeof(DeviceInfo_t))

typedef struct _tagBLEParameters_t
{
    unsigned long                            Flags;
    qapi_BLE_GAP_LE_Advertising_Parameters_t AdvertisingParameters;
    qapi_BLE_GAP_LE_Connection_Parameters_t  ConnectionParameters;
}BLEParameters_t;

/**
 * Function Prototype
 */
int InitializeBluetooth(void);
int AdvertiseLE(uint32_t enable);
uint32_t GetBluetoothStackID(void);
DeviceInfo_t *GetCurrentPeerDeviceInfo(void);
int GetConnectionCount(void);
void Cleanup_wifi_service(void);
void Cleanup_zigbee_service(void);
int Register_offline_service(void);
int32_t enable_advertise(void);
int32_t enable_cordinator_fn(void);
#endif
