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

#ifndef __QC_DRV_BLE_H
#define __QC_DRV_BLE_H

#include "qc_drv_main.h"
#include "qapi_ble_bttypes.h"
#include "qapi_ble.h"

qapi_Status_t qc_drv_ble_BSC_Query_Host_Version(qc_drv_context *qc_drv_ctx, char *HostVersion);

qapi_Status_t qc_drv_ble_HCI_Read_Local_Version_Information(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, uint8_t *StatusResult, uint8_t *HCI_VersionResult, uint16_t *HCI_RevisionResult, uint8_t *LMP_VersionResult, uint16_t *Manufacturer_NameResult, uint16_t *LMP_SubversionResult);

qapi_Status_t qc_drv_ble_GAP_LE_Set_Pairability_Mode(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, qapi_BLE_GAP_LE_Pairability_Mode_t PairabilityMode);

qapi_Status_t qc_drv_ble_GAP_LE_Authentication_Response(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, qapi_BLE_BD_ADDR_t SecurityRemoteBD_ADDR, qapi_BLE_GAP_LE_Authentication_Response_Information_t *GAP_LE_Authentication_Response_Information);

qapi_Status_t qc_drv_ble_GAP_LE_Query_Encryption_Mode(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, qapi_BLE_BD_ADDR_t BD_ADDR, qapi_BLE_GAP_Encryption_Mode_t *GAP_Encryption_Mode);

qapi_Status_t qc_drv_ble_GAP_LE_Set_Fixed_Passkey(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, uint32_t *Fixed_Display_Passkey);

qapi_Status_t qc_drv_ble_GAP_Query_Local_BD_ADDR(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, qapi_BLE_BD_ADDR_t *BD_ADDR);

qapi_Status_t qc_drv_ble_GAP_LE_Advertising_Disable(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID);

qapi_Status_t qc_drv_ble_GAPS_Query_Device_Appearance(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, uint32_t InstanceID, uint16_t *DeviceAppearance);

qapi_Status_t qc_drv_ble_GAPS_Query_Device_Name(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, uint32_t InstanceID, char *NameBuffer);

qapi_Status_t qc_drv_ble_GAP_LE_Set_Advertising_Data(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, uint32_t Length, qapi_BLE_Advertising_Data_t *Advertising_Data);

qapi_Status_t qc_drv_ble_GAP_LE_Set_Scan_Response_Data(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, uint32_t Length, qapi_BLE_Scan_Response_Data_t *Scan_Response_Data);

qapi_Status_t qc_drv_ble_BSC_LockBluetoothStack(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID);

qapi_Status_t qc_drv_ble_BSC_UnLockBluetoothStack(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID);

qapi_Status_t qc_drv_ble_GAP_LE_Advertising_Enable(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, boolean_t EnableScanResponse, qapi_BLE_GAP_LE_Advertising_Parameters_t *GAP_LE_Advertising_Parameters, qapi_BLE_GAP_LE_Connectability_Parameters_t *GAP_LE_Connectability_Parameters, qapi_BLE_GAP_LE_Event_Callback_t GAP_LE_Event_Callback, uint32_t CallbackParameter);

qapi_Status_t qc_drv_ble_GAP_LE_Disconnect(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, qapi_BLE_BD_ADDR_t BD_ADDR);

qapi_Status_t qc_drv_ble_GAP_LE_Cancel_Create_Connection(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID);

qapi_Status_t qc_drv_ble_GATT_Start_Service_Discovery_Handle_Range(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, uint32_t ConnectionID, qapi_BLE_GATT_Attribute_Handle_Group_t *DiscoveryHandleRange, uint32_t NumberOfUUID, qapi_BLE_GATT_UUID_t *UUIDList, qapi_BLE_GATT_Service_Discovery_Event_Callback_t ServiceDiscoveryCallback, uint32_t CallbackParameter);

qapi_Status_t qc_drv_ble_GATT_Start_Service_Discovery(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, uint32_t ConnectionID, uint32_t NumberOfUUID, qapi_BLE_GATT_UUID_t *UUIDList, qapi_BLE_GATT_Service_Discovery_Event_Callback_t ServiceDiscoveryCallback, uint32_t CallbackParameter);

qapi_Status_t qc_drv_ble_GAP_LE_Add_Device_To_White_List(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, uint32_t DeviceCount, qapi_BLE_GAP_LE_White_List_Entry_t *WhiteListEntries, uint32_t *AddedDeviceCount);

qapi_Status_t qc_drv_ble_GAP_LE_Remove_Device_From_White_List(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, uint32_t DeviceCount, qapi_BLE_GAP_LE_White_List_Entry_t *WhiteListEntries, uint32_t *RemovedDeviceCount);

qapi_Status_t qc_drv_ble_GAP_LE_Add_Device_To_Resolving_List(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, uint32_t DeviceCount, qapi_BLE_GAP_LE_Resolving_List_Entry_t *ResolvingListEntries, uint32_t *AddedDeviceCount);

qapi_Status_t qc_drv_ble_GAP_LE_Remove_Device_From_Resolving_List(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, uint32_t DeviceCount, qapi_BLE_GAP_LE_Resolving_List_Entry_t *ResolvingListEntries, uint32_t *RemovedDeviceCount);

qapi_Status_t qc_drv_ble_GAP_LE_Set_Authenticated_Payload_Timeout(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, qapi_BLE_BD_ADDR_t BD_ADDR, uint16_t AuthenticatedPayloadTimeout);

qapi_Status_t qc_drv_ble_GATT_Change_Maximum_Supported_MTU(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, uint16_t MTU);

qapi_Status_t qc_drv_ble_GATT_Query_Maximum_Supported_MTU(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, uint16_t *MTU);

qapi_Status_t qc_drv_ble_GAP_LE_Query_Connection_Handle(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, qapi_BLE_BD_ADDR_t BD_ADDR, uint16_t *Connection_Handle);

qapi_Status_t qc_drv_ble_GAP_LE_Set_Data_Length(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, qapi_BLE_BD_ADDR_t BD_ADDR, uint16_t SuggestedTxPacketSize, uint16_t SuggestedTxPacketTime);

qapi_Status_t qc_drv_ble_GAP_LE_Query_Local_Secure_Connections_OOB_Data(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, qapi_BLE_Secure_Connections_Randomizer_t *Randomizer, qapi_BLE_Secure_Connections_Confirmation_t *Confirmation);

qapi_Status_t qc_drv_ble_BSC_SetTxPower(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, boolean_t Connection, int8_t TxPower);

qapi_Status_t qc_drv_ble_BSC_Set_FEM_Control_Override(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, boolean_t Enable, uint16_t FEM_Ctrl_0_1, uint16_t FEM_Ctrl_2_3);

qapi_Status_t qc_drv_ble_AIOS_Initialize_Service(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, uint32_t Service_Flags, qapi_BLE_AIOS_Initialize_Data_t *InitializeData, qapi_BLE_AIOS_Event_Callback_t EventCallback, uint32_t CallbackParameter, uint32_t *ServiceID);

qapi_Status_t qc_drv_ble_AIOS_Cleanup_Service(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, uint32_t InstanceID);

qapi_Status_t  qc_drv_ble_GATT_Read_Value_Request(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, uint32_t ConnectionID, uint16_t AttributeHandle, qapi_BLE_GATT_Client_Event_Callback_t ClientEventCallback, uint32_t CallbackParameter);

qapi_Status_t qc_drv_ble_AIOS_Notify_Characteristic(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, uint32_t InstanceID, uint32_t ConnectionID, qapi_BLE_AIOS_Characteristic_Info_t *CharacteristicInfo, qapi_BLE_AIOS_Characteristic_Data_t *CharacteristicData);

qapi_Status_t qc_drv_ble_GATT_Write_Request(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, uint32_t ConnectionID, uint16_t AttributeHandle, uint16_t AttributeLength, void *AttributeValue, qapi_BLE_GATT_Client_Event_Callback_t ClientEventCallback, uint32_t CallbackParameter);

qapi_Status_t qc_drv_ble_HIDS_Initialize_Service(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, uint8_t Flags, qapi_BLE_HIDS_HID_Information_Data_t *HIDInformation, uint32_t NumIncludedServices, uint32_t *ServiceIDList, uint32_t NumExternalReportReferences, qapi_BLE_GATT_UUID_t *ReferenceUUID, uint32_t NumReports, qapi_BLE_HIDS_Report_Reference_Data_t *ReportReference, qapi_BLE_HIDS_Event_Callback_t EventCallback, uint32_t CallbackParameter, uint32_t *ServiceID);

qapi_Status_t qc_drv_ble_HIDS_Cleanup_Service(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, uint32_t InstanceID);

qapi_Status_t qc_drv_ble_HIDS_Notify_Input_Report(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, uint32_t InstanceID, uint32_t ConnectionID, qapi_BLE_HIDS_Report_Type_t ReportType, qapi_BLE_HIDS_Report_Reference_Data_t *ReportReferenceData, uint16_t InputReportLength, uint8_t *InputReportData);

qapi_Status_t qc_drv_ble_HIDS_Format_Control_Point_Command(qc_drv_context *qc_drv_ctx, qapi_BLE_HIDS_Control_Point_Command_t Command, uint32_t BufferLength, uint8_t *Buffer);

qapi_Status_t qc_drv_ble_GATT_Write_Without_Response_Request(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, uint32_t ConnectionID, uint16_t AttributeHandle, uint16_t AttributeLength, void *AttributeValue);

qapi_Status_t qc_drv_ble_HIDS_Format_Protocol_Mode(qc_drv_context *qc_drv_ctx, qapi_BLE_HIDS_Protocol_Mode_t ProtocolMode, uint32_t BufferLength, uint8_t *Buffer);

qapi_Status_t qc_drv_ble_SCPS_Initialize_Service(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, qapi_BLE_SCPS_Event_Callback_t EventCallback, uint32_t CallbackParameter, uint32_t *ServiceID);

qapi_Status_t qc_drv_ble_SCPS_Cleanup_Service(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, uint32_t InstanceID);

qapi_Status_t qc_drv_ble_SCPS_Format_Scan_Interval_Window(qc_drv_context *qc_drv_ctx, qapi_BLE_SCPS_Scan_Interval_Window_Data_t *Scan_Interval_Window, uint32_t BufferLength, uint8_t *Buffer);

qapi_Status_t qc_drv_ble_SCPS_Notify_Scan_Refresh(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, uint32_t InstanceID, uint32_t ConnectionID, uint8_t ScanRefreshValue);

qapi_Status_t qc_drv_ble_GATT_Register_Service(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, uint8_t ServiceFlags, uint32_t NumberOfServiceAttributeEntries, qapi_BLE_GATT_Service_Attribute_Entry_t *ServiceTable, qapi_BLE_GATT_Attribute_Handle_Group_t *ServiceHandleGroupResult, qapi_BLE_GATT_Server_Event_Callback_t ServerEventCallback, uint32_t CallbackParameter);

void qc_drv_ble_GATT_Un_Register_Service(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, uint32_t ServiceID);


qapi_Status_t qc_drv_ble_BAS_Initialize_Service(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, qapi_BLE_BAS_Event_Callback_t EventCallback, uint32_t CallbackParameter, uint32_t *ServiceID);

qapi_Status_t qc_drv_ble_BAS_Set_Characteristic_Presentation_Format(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, uint32_t InstanceID, qapi_BLE_BAS_Presentation_Format_Data_t *CharacteristicPresentationFormat);

qapi_Status_t qc_drv_ble_BAS_Cleanup_Service(qc_drv_context *qc_drv_ctx,uint32_t BluetoothStackID, uint32_t InstanceID);

qapi_Status_t qc_drv_ble_BAS_Notify_Battery_Level(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, uint32_t InstanceID, uint32_t ConnectionID, uint8_t BatteryLevel);

qapi_Status_t qc_drv_ble_BAS_Query_Characteristic_Presentation_Format(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, uint32_t InstanceID, qapi_BLE_BAS_Presentation_Format_Data_t *CharacteristicPresentationFormat);

qapi_Status_t qc_drv_ble_GAPS_Set_Device_Name(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, uint32_t InstanceID, char *DeviceName);

qapi_Status_t qc_drv_ble_GAPS_Set_Device_Appearance(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, uint32_t InstanceID, uint16_t DeviceAppearance);

qapi_Status_t qc_drv_ble_DIS_Initialize_Service(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, uint32_t *ServiceID);
qapi_Status_t qc_drv_ble_DIS_Set_Manufacturer_Name(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, uint32_t InstanceID, char *ManufacturerName);

qapi_Status_t qc_drv_ble_DIS_Set_Model_Number(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, uint32_t InstanceID, char *ModelNumber);
qapi_Status_t qc_drv_ble_DIS_Set_Software_Revision(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, uint32_t InstanceID, char *SoftwareRevision);
qapi_Status_t qc_drv_ble_DIS_Set_Hardware_Revision(qc_drv_context *qc_drv_ctx,uint32_t BluetoothStackID, uint32_t InstanceID, char *Hardware_Revision);
qapi_Status_t qc_drv_ble_DIS_Set_Firmware_Revision(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, uint32_t InstanceID, char *FirmwareRevision);
qapi_Status_t qc_drv_ble_DIS_Cleanup_Service(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, uint32_t InstanceID);

qapi_Status_t qc_drv_ble_TPS_Initialize_Service(qc_drv_context *qc_drv_ctx, int32_t BluetoothStackID, uint32_t *ServiceID);
qapi_Status_t qc_drv_ble_TPS_Set_Tx_Power_Level(qc_drv_context *qc_drv_ctx, int32_t BluetoothStackID, uint32_t InstanceID, int8_t Tx_Power_Level);
qapi_Status_t qc_drv_ble_TPS_Cleanup_Service(qc_drv_context *qc_drv_ctx, int32_t BluetoothStackID, uint32_t InstanceID);


qapi_Status_t qc_drv_ble_GAP_LE_Query_Connection_PHY(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, qapi_BLE_BD_ADDR_t BD_ADDR, qapi_BLE_GAP_LE_PHY_Type_t *TxPHY, qapi_BLE_GAP_LE_PHY_Type_t *RxPHY);

qapi_Status_t qc_drv_ble_GAP_LE_Set_Connection_PHY(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, qapi_BLE_BD_ADDR_t BD_ADDR, uint32_t TxPHYSPreference, uint32_t RxPHYSPreference);

qapi_Status_t qc_drv_ble_GAP_LE_Set_Extended_Advertising_Parameters(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, uint8_t AdvertisingHandle, qapi_BLE_GAP_LE_Extended_Advertising_Parameters_t *AdvertisingParameters, int8_t *SelectedTxPower);

qapi_Status_t qc_drv_ble_GAP_LE_Enable_Extended_Advertising(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, boolean_t Enable, uint8_t NumberOfSets, uint8_t *AdvertisingHandleList, uint32_t *DurationList, uint8_t *MaxExtendedAdvertisingEventList, qapi_BLE_GAP_LE_Event_Callback_t GAP_LE_Event_Callback, uint32_t CallbackParameter);

qapi_Status_t qc_drv_ble_GAP_LE_Set_Extended_Scan_Parameters(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, qapi_BLE_GAP_LE_Address_Type_t LocalAddressType, qapi_BLE_GAP_LE_Filter_Policy_t FilterPolicy, uint32_t NumberScanningPHYs, qapi_BLE_GAP_LE_Extended_Scanning_PHY_Parameters_t *ScanningParameterList);

qapi_Status_t qc_drv_ble_GAP_LE_Enable_Extended_Scan(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, boolean_t Enable, qapi_BLE_GAP_LE_Extended_Scan_Filter_Duplicates_Type_t FilterDuplicates, uint32_t Duration, uint32_t Period, qapi_BLE_GAP_LE_Event_Callback_t GAP_LE_Event_Callback, uint32_t CallbackParameter);

qapi_Status_t qc_drv_ble_GAP_LE_Extended_Create_Connection(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, qapi_BLE_GAP_LE_Filter_Policy_t InitatorFilterPolicy, qapi_BLE_GAP_LE_Address_Type_t RemoteAddressType, qapi_BLE_BD_ADDR_t *RemoteDevice, qapi_BLE_GAP_LE_Address_Type_t LocalAddressType, uint32_t NumberOfConnectionParameters, qapi_BLE_GAP_LE_Extended_Connection_Parameters_t *ConnectionParameterList, qapi_BLE_GAP_LE_Event_Callback_t GAP_LE_Event_Callback, uint32_t CallbackParameter);


qapi_Status_t qc_drv_ble_BSC_AddGenericListEntry_Actual(qc_drv_context *qc_drv_ctx, qapi_BLE_BSC_Generic_List_Entry_Key_t GenericListEntryKey, uint32_t ListEntryKeyOffset, uint32_t ListEntryNextPointerOffset, void **ListHead, void *ListEntryToAdd);

qapi_Status_t qc_drv_ble_BSC_Initialize(qc_drv_context *qc_drv_ctx, qapi_BLE_HCI_DriverInformation_t *HCI_DriverInformation, uint32_t Flags);

qapi_Status_t qc_drv_ble_HCI_LE_Read_Buffer_Size(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, uint8_t *StatusResult, uint16_t *HC_LE_ACL_Data_Packet_Length, uint8_t *HC_Total_Num_LE_ACL_Data_Packets);

qapi_Status_t qc_drv_ble_GATT_Initialize(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, uint32_t Flags, qapi_BLE_GATT_Connection_Event_Callback_t ConnectionEventCallback, uint32_t CallbackParameter);

qapi_Status_t qc_drv_ble_GAPS_Initialize_Service(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, uint32_t *ServiceID);

qapi_Status_t qc_drv_ble_GAP_LE_Set_Address_Resolution_Enable(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, boolean_t EnableAddressResolution);

qapi_Status_t qc_drv_ble_GAP_LE_Set_Resolvable_Private_Address_Timeout(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, uint32_t RPA_Timeout);

qapi_Status_t qc_drv_ble_HCI_Register_Event_Callback(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, qapi_BLE_HCI_Event_Callback_t HCI_EventCallback, uint32_t CallbackParameter);

qapi_Status_t qc_drv_ble_HCI_Register_ACL_Data_Callback(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, qapi_BLE_HCI_ACL_Data_Callback_t HCI_ACLDataCallback, uint32_t CallbackParameter);

qapi_Status_t qc_drv_ble_HCI_Send_ACL_Data(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, uint16_t Connection_Handle, uint16_t Flags, uint16_t ACLDataLength, uint8_t *ACLData);

qapi_Status_t qc_drv_ble_GAP_LE_Register_Remote_Authentication(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, qapi_BLE_GAP_LE_Event_Callback_t GAP_LE_Event_Callback, uint32_t CallbackParameter);

qapi_Status_t qc_drv_ble_BSC_StartTimer(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, uint32_t Timeout, qapi_BLE_BSC_Timer_Callback_t TimerCallback, uint32_t CallbackParameter);

qapi_Status_t qc_drv_ble_GAP_LE_Perform_Scan(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, qapi_BLE_GAP_LE_Scan_Type_t ScanType, uint32_t ScanInterval, uint32_t ScanWindow, qapi_BLE_GAP_LE_Address_Type_t LocalAddressType, qapi_BLE_GAP_LE_Filter_Policy_t FilterPolicy, boolean_t FilterDuplicates, qapi_BLE_GAP_LE_Event_Callback_t GAP_LE_Event_Callback, uint32_t CallbackParameter);

qapi_Status_t qc_drv_ble_GAP_LE_Cancel_Scan(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID);

qapi_Status_t qc_drv_ble_GAP_LE_Create_Connection(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, uint32_t ScanInterval, uint32_t ScanWindow, qapi_BLE_GAP_LE_Filter_Policy_t InitatorFilterPolicy, qapi_BLE_GAP_LE_Address_Type_t RemoteAddressType, qapi_BLE_BD_ADDR_t *RemoteDevice, qapi_BLE_GAP_LE_Address_Type_t LocalAddressType, qapi_BLE_GAP_LE_Connection_Parameters_t *ConnectionParameters, qapi_BLE_GAP_LE_Event_Callback_t GAP_LE_Event_Callback, uint32_t CallbackParameter);

qapi_Status_t qc_drv_ble_HRS_Decode_Heart_Rate_Measurement(qc_drv_context *qc_drv_ctx, uint32_t ValueLength, uint8_t *Value, qapi_BLE_HRS_Heart_Rate_Measurement_Data_t *HeartRateMeasurement);

qapi_Status_t qc_drv_ble_GATT_Handle_Value_Notification(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, uint32_t ServiceID, uint32_t ConnectionID, uint16_t AttributeOffset, uint16_t AttributeValueLength, uint8_t *AttributeValue);

qapi_Status_t qc_drv_ble_BSC_GetTxPower(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, boolean_t Connection, int8_t *TxPower);

qapi_Status_t qc_drv_ble_GAP_LE_Reestablish_Security(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, qapi_BLE_BD_ADDR_t BD_ADDR, qapi_BLE_GAP_LE_Security_Information_t *SecurityInformation, qapi_BLE_GAP_LE_Event_Callback_t GAP_LE_Event_Callback, uint32_t CallbackParameter);

qapi_Status_t qc_drv_ble_GAP_LE_Regenerate_Long_Term_Key(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, qapi_BLE_Encryption_Key_t *DHK, qapi_BLE_Encryption_Key_t *ER, uint16_t EDIV, qapi_BLE_Random_Number_t *Rand, qapi_BLE_Long_Term_Key_t *LTK_Result);

qapi_Status_t qc_drv_ble_AIOS_Read_Characteristic_Request_Response(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, uint32_t InstanceID, uint32_t ConnectionID, uint32_t TransactionID, uint8_t ErrorCode, qapi_BLE_AIOS_Characteristic_Info_t *CharacteristicInfo, qapi_BLE_AIOS_Characteristic_Data_t *CharacteristicData);

qapi_Status_t qc_drv_ble_AIOS_Write_Characteristic_Request_Response(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, uint32_t InstanceID, uint32_t TransactionID, uint8_t ErrorCode, qapi_BLE_AIOS_Characteristic_Info_t *CharacteristicInfo);

qapi_Status_t qc_drv_ble_AIOS_Read_CCCD_Request_Response(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, uint32_t InstanceID, uint32_t TransactionID, uint8_t ErrorCode, qapi_BLE_AIOS_Characteristic_Info_t *CharacteristicInfo, uint16_t ClientConfiguration);

qapi_Status_t qc_drv_ble_AIOS_Write_CCCD_Request_Response(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, uint32_t InstanceID, uint32_t TransactionID, uint8_t ErrorCode, qapi_BLE_AIOS_Characteristic_Info_t *CharacteristicInfo);

qapi_Status_t qc_drv_ble_AIOS_Read_Presentation_Format_Request_Response(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, uint32_t InstanceID, uint32_t TransactionID, uint8_t ErrorCode, qapi_BLE_AIOS_Characteristic_Info_t *CharacteristicInfo, qapi_BLE_AIOS_Presentation_Format_Data_t *PresentationFormatData);

qapi_Status_t qc_drv_ble_AIOS_Read_Number_Of_Digitals_Request_Response(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, uint32_t InstanceID, uint32_t TransactionID, uint8_t ErrorCode, qapi_BLE_AIOS_Characteristic_Info_t *CharacteristicInfo, uint8_t NumberOfDigitals);

qapi_Status_t qc_drv_ble_BAS_Read_Client_Configuration_Response(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, uint32_t InstanceID, uint32_t TransactionID, uint16_t Client_Configuration);

qapi_Status_t qc_drv_ble_HIDS_Read_Client_Configuration_Response(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, uint32_t InstanceID, uint32_t TransactionID, uint16_t Client_Configuration);

qapi_Status_t qc_drv_ble_HIDS_Get_Protocol_Mode_Response(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, uint32_t InstanceID, uint32_t TransactionID, uint8_t ErrorCode, qapi_BLE_HIDS_Protocol_Mode_t CurrentProtocolMode);

qapi_Status_t qc_drv_ble_HIDS_Get_Report_Map_Response(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, uint32_t InstanceID, uint32_t TransactionID, uint8_t ErrorCode, uint32_t ReportMapLength, uint8_t *ReportMap);

qapi_Status_t qc_drv_ble_HIDS_Get_Report_Response(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, uint32_t InstanceID, uint32_t TransactionID, qapi_BLE_HIDS_Report_Type_t ReportType, qapi_BLE_HIDS_Report_Reference_Data_t *ReportReferenceData, uint8_t ErrorCode, uint32_t ReportLength, uint8_t *Report);

qapi_Status_t qc_drv_ble_HIDS_Set_Report_Response(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, uint32_t InstanceID, uint32_t TransactionID, qapi_BLE_HIDS_Report_Type_t ReportType, qapi_BLE_HIDS_Report_Reference_Data_t *ReportReferenceData, uint8_t ErrorCode);

qapi_Status_t qc_drv_ble_SCPS_Read_Client_Configuration_Response(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, uint32_t InstanceID, uint32_t TransactionID, uint16_t Client_Configuration);

qapi_Status_t qc_drv_ble_AIOS_Decode_Presentation_Format(qc_drv_context *qc_drv_ctx, uint32_t ValueLength, uint8_t *Value, qapi_BLE_AIOS_Presentation_Format_Data_t *PresentationFormatData);

qapi_Status_t qc_drv_ble_BAS_Decode_Characteristic_Presentation_Format(qc_drv_context *qc_drv_ctx, uint32_t ValueLength, uint8_t *Value, qapi_BLE_BAS_Presentation_Format_Data_t *CharacteristicPresentationFormat);

qapi_Status_t qc_drv_ble_GATT_Read_Long_Value_Request(qc_drv_context *qc_drv_ctx, uint32_t BluetoothStackID, uint32_t ConnectionID, uint16_t AttributeHandle, uint16_t AttributeOffset, qapi_BLE_GATT_Client_Event_Callback_t ClientEventCallback, uint32_t CallbackParameter);

qapi_Status_t qc_drv_ble_HIDS_Decode_HID_Information(qc_drv_context *qc_drv_ctx, uint32_t ValueLength, uint8_t *Value, qapi_BLE_HIDS_HID_Information_Data_t *HIDSHIDInformation);

qapi_Status_t qc_drv_ble_HIDS_Decode_Report_Reference(qc_drv_context *qc_drv_ctx, uint32_t ValueLength, uint8_t *Value, qapi_BLE_HIDS_Report_Reference_Data_t *ReportReferenceData);

qapi_Status_t qc_drv_ble_HIDS_Decode_External_Report_Reference(qc_drv_context *qc_drv_ctx, uint32_t ValueLength, uint8_t *Value, qapi_BLE_GATT_UUID_t *ExternalReportReferenceUUID);

#endif
