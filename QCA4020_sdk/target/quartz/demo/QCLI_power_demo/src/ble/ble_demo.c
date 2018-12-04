/*
 * Copyright (c) 2015-2017 Qualcomm Technologies, Inc.
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

#include "malloc.h"
#include "string.h"
#include "stringl.h"
#include "qapi_timer.h"
#include "qurt_error.h"
#include "qurt_timer.h"    /* Timer for Throughput Calculation.         */

#include "qcli_api.h"
#include "qcli_util.h"
#include "pal.h"
#include "qapi_om_smem.h"
#include "qapi_omtm.h"
#include "ble_demo.h"

#define DEFAULT_IO_CAPABILITY                    (QAPI_BLE_LIC_NO_INPUT_NO_OUTPUT_E)
                                                         /* Denotes the       */
                                                         /* default I/O       */
                                                         /* Capability that is*/
                                                         /* used with Pairing.*/

#define DEFAULT_MITM_PROTECTION                  (TRUE)  /* Denotes the       */
                                                         /* default value used*/
                                                         /* for Man in the    */
                                                         /* Middle (MITM)     */
                                                         /* protection used   */
                                                         /* with Secure Simple*/
                                                         /* Pairing.          */

#define DEFAULT_SECURE_CONNECTIONS               (TRUE)  /* Denotes default   */
                                                         /* value for if SC   */
                                                         /* is requested.     */

#define MAXIMUM_PAIRED_DEVICES                   (2)     /* Maximum paired    */
                                                         /* devices this app  */
                                                         /* will save.        */

   /* The multiplier for calculating the maximum SDU size for L2CAP LE  */
   /* CoC.  The maximum SDU we will receive is a                        */
   /* (BLE_DEMO_SDU_MULTIPLIER * MaximumPacketSize) - 2.                */
#define BLE_DEMO_SDU_MULTIPLIER                 (4)

   /* The following defines the maximum local credit count to use when  */
   /* creating a L2CAP LE CoC.                                          */
#define BLE_DEMO_MAXIMUM_LOCAL_CREDITS          (10)

   /* The following defines the maximum PDU Queue Depth for the L2CAP LE*/
   /* CoC.                                                              */
#define BLE_DEMO_PDU_QUEUE_DEPTH                (5)

   /* The following defines the maximum SDU Queue Depth for the L2CAP LE*/
   /* CoC.                                                              */
#define BLE_DEMO_SDU_QUEUE_DEPTH                ((BLE_DEMO_PDU_QUEUE_DEPTH / BLE_DEMO_SDU_MULTIPLIER) > 0) ? (BLE_DEMO_PDU_QUEUE_DEPTH/BLE_DEMO_SDU_MULTIPLIER) : 1;

   /* The following MACRO is a utility MACRO that exists to calculate   */
   /* the offset position of a particular structure member from the     */
   /* start of the structure.  This MACRO accepts as the first          */
   /* parameter, the physical name of the structure (the type name, NOT */
   /* the variable name).  The second parameter to this MACRO represents*/
   /* the actual structure member that the offset is to be determined.  */
   /* This MACRO returns an unsigned integer that represents the offset */
   /* (in bytes) of the structure member.                               */
#define STRUCTURE_OFFSET(_x, _y)                   ((unsigned int)&(((_x *)0)->_y))

   /* Some MACROs for accessing little-endian unaligned values.         */
#define READ_UNALIGNED_WORD_LITTLE_ENDIAN(_x)      ((uint16_t)((((uint16_t)(((uint8_t *)(_x))[1])) << 8) | ((uint16_t)(((uint8_t *)(_x))[0]))))

   /* Determine the Name we will use for this compilation.              */
#define DEVICE_FRIENDLY_NAME                       "qclipwr-ble-demo"

   /* Structure used to hold all of the GAP LE Parameters.              */
typedef struct _tagGAPLE_Parameters_t
{
   qapi_BLE_GAP_LE_Connectability_Mode_t ConnectableMode;
   qapi_BLE_GAP_Discoverability_Mode_t   DiscoverabilityMode;
   qapi_BLE_GAP_LE_IO_Capability_t       IOCapability;
   boolean_t                             MITMProtection;
   boolean_t                             SecureConnections;
   boolean_t                             OOBDataPresent;
} GAPLE_Parameters_t;

   /* The following structure represents the information we will store  */
   /* on a Discovered GAP Service.                                      */
typedef struct _tagGAPS_Client_Info_t
{
   uint16_t DeviceNameHandle;
   uint16_t DeviceAppearanceHandle;
} GAPS_Client_Info_t;

   /* The following structure holds information on known Device         */
   /* Appearance Values.                                                */
typedef struct _tagGAPS_Device_Appearance_Mapping_t
{
   uint16_t  Appearance;
   char     *String;
} GAPS_Device_Appearance_Mapping_t;

   /* The following structure is used to hold the Scan Window and       */
   /* Interval parameters for LE Scanning.                              */
typedef struct _tagBLEScanParameters_t
{
   uint16_t ScanInterval;
   uint16_t ScanWindow;
} BLEScanParameters_t;

   /* The following structure is used to hold information on the        */
   /* configured Scan/Advertising/Connection Parameters.                */
typedef struct _tagBLEParameters_t
{
   unsigned long                            Flags;
   boolean_t                                EnableAdvScanResponse;
   boolean_t                                EnableActiveScanning;
   boolean_t                                ScanningFilterDuplicates;
   qapi_BLE_GAP_LE_Advertising_Parameters_t AdvertisingParameters;
   qapi_BLE_GAP_LE_Connection_Parameters_t  ConnectionParameters;
   BLEScanParameters_t                      ScanParameters;
} BLEParameters_t;

#define BLE_PARAMETERS_FLAGS_ADVERTISING_PARAMETERS_VALID   0x00000001
#define BLE_PARAMETERS_FLAGS_SCAN_PARAMETERS_VALID          0x00000002
#define BLE_PARAMETERS_FLAGS_CONNECTION_PARAMETERS_VALID    0x00000004

   /* The following structure for a Master is used to hold a list of    */
   /* information on all paired devices. For slave we will not use this */
   /* structure.                                                        */
typedef struct _tagDeviceInfo_t
{
   uint8_t                                 Flags;
   uint8_t                                 EncryptionKeySize;
   qapi_BLE_GAP_LE_Address_Type_t          ConnectionAddressType;
   qapi_BLE_BD_ADDR_t                      ConnectionBD_ADDR;
   qapi_BLE_Encryption_Key_t               IRK;
   qapi_BLE_GAP_LE_Address_Type_t          IdentityAddressType;
   qapi_BLE_BD_ADDR_t                      IdentityAddressBD_ADDR;
   qapi_BLE_GAP_LE_White_List_Entry_t      WhiteListEntry;
   qapi_BLE_GAP_LE_Resolving_List_Entry_t  ResolvingListEntry;
   qapi_BLE_Long_Term_Key_t                LTK;
   qapi_BLE_Random_Number_t                Rand;
   uint16_t                                EDIV;
   GAPS_Client_Info_t                      GAPSClientInfo;
   struct _tagDeviceInfo_t                *NextDeviceInfoInfoPtr;
} DeviceInfo_t;

#define DEVICE_INFO_DATA_SIZE                            (sizeof(DeviceInfo_t))

   /* Defines the bitmask flags that may be set in the DeviceInfo_t     */
   /* structure.                                                        */
#define DEVICE_INFO_FLAGS_LTK_VALID                         0x01
#define DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_OUTSTANDING     0x08
#define DEVICE_INFO_FLAGS_IRK_VALID                         0x10
#define DEVICE_INFO_FLAGS_ADDED_TO_WHITE_LIST               0x20
#define DEVICE_INFO_FLAGS_ADDED_TO_RESOLVING_LIST           0x40

   /* This structure stores the information for mode transitions.       */
typedef struct BLE_Demo_Transition_Data_s
{
   uint32_t                  BluetoothStackID;
   unsigned int              GAPSInstanceID;
   BLEParameters_t           BLEParameters;
   GAPLE_Parameters_t        LE_Parameters;
   unsigned long             ContextFlags;
   qapi_BLE_Encryption_Key_t ER;
   qapi_BLE_Encryption_Key_t IR;
   unsigned int              NumberPairedDevices;
   boolean_t                 ScanInProgress;
   DeviceInfo_t              PairedDevices[MAXIMUM_PAIRED_DEVICES];
} BLE_Demo_Transition_Data_t;

   /* This structure represents the contextual information for the ble  */
   /* demo application.                                                 */
typedef struct BLE_Demo_Context_s
{
   uint32_t                   BluetoothStackID;
   unsigned int               GAPSInstanceID;
   QCLI_Group_Handle_t        QCLI_Handle;
   QCLI_Group_Handle_t        QCLI_DUT_Handle;
   unsigned long              ContextFlags;
   unsigned int               ConnectionID;
   unsigned int               TestAdvertisingDataLength;
   uint32_t                   TransmitPeriod;
   qapi_TIMER_handle_t        Timer;
   uint16_t                   ConnMaxTxOcets;
   uint16_t                   ConnMaxRxOcets;
   BLEParameters_t            BLEParameters;
   GAPLE_Parameters_t         LE_Parameters;
   qapi_BLE_BD_ADDR_t         PublicAddress;
   qapi_BLE_Encryption_Key_t  ER;
   qapi_BLE_Encryption_Key_t  IR;
   qapi_BLE_Encryption_Key_t  DHK;
   qapi_BLE_Encryption_Key_t  IRK;
   qapi_BLE_BD_ADDR_t         ConnectionBD_ADDR;
   qapi_BLE_BD_ADDR_t         CurrentRemoteBD_ADDR;
   boolean_t                  LocalDeviceIsMaster;
   boolean_t                  ScanInProgress;
   DeviceInfo_t              *DeviceInfoList;
} BLE_Demo_Context_t;

#define BLE_DEMO_CONTEXT_FLAGS_ER_VALID      0x00000001
#define BLE_DEMO_CONTEXT_FLAGS_IR_VALID      0x00000002

   /* User to represent a structure to hold a BD_ADDR return from       */
   /* BD_ADDRToStr.                                                     */
typedef char BoardStr_t[16];

   /* The following holds all of the context stored for this            */
   /* application.                                                      */
static BLE_Demo_Context_t BLE_Demo_Context;

   /* The following is used to map from ATT Error Codes to a printable  */
   /* string.                                                           */
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
} ;

#define NUMBER_OF_ERROR_CODES     (sizeof(ErrorCodeStr)/sizeof(char *))

   /* The following array is used to map Device Appearance Values to    */
   /* strings.                                                          */
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

   /* The following string table is used to map HCI Version information */
   /* to an easily displayable version string.                          */
static const char *HCIVersionStrings[] =
{
   "1.0b",
   "1.1",
   "1.2",
   "2.0",
   "2.1",
   "3.0",
   "4.0",
   "4.1",
   "4.2",
   "5.0",
   "Unknown (greater 5.0)"
} ;

#define NUM_SUPPORTED_HCI_VERSIONS              (sizeof(HCIVersionStrings)/sizeof(char *) - 1)

   /* The following string table is used to map the API I/O Capabilities*/
   /* values to an easily displayable string.                           */
static const char *IOCapabilitiesStrings[] =
{
   "Display Only",
   "Display Yes/No",
   "Keyboard Only",
   "No Input/Output",
   "Keyboard/Display"
} ;

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the         */
   /* compiler as part of standard C/C++).                              */
#define CURRENT_TEST_NONE                       0
#define CURRENT_TEST_TX_ACL                     1
#define CURRENT_TEST_RX_ACL                     3
#define CURRENT_TEST_PERIODIC                   4

#define MAXIMUM_TEST_BUFFER                  1024

static unsigned int  CurrentTest = CURRENT_TEST_NONE;
static unsigned int  NumberACLPackets;
static unsigned int  NumberOutstandingACLPackets;
static unsigned int  MaxACLPacketSize;
static unsigned long StartTime;
static unsigned long EndTime;
static unsigned long NumberBytes;
static unsigned long PacketLength;
static unsigned char TestBuffer[MAXIMUM_TEST_BUFFER];
static uint16_t      ConnectionHandle;

   /* External function prototypes.                                     */
extern int HCI_VS_GetPatchVersion(unsigned int BluetoothStackID, uint32_t *ProductID, uint32_t *BuildVersion);
extern int HCI_VS_EnableBBIF(unsigned int BluetoothStackID, boolean_t Enable);
extern int HCI_VS_SetRadio(unsigned int BluetoothStackID, unsigned int RadioNumber);

   /* Internal function prototypes.                                     */
static boolean_t CreateNewDeviceInfoEntry(DeviceInfo_t **ListHead, qapi_BLE_GAP_LE_Address_Type_t ConnectionAddressType, qapi_BLE_BD_ADDR_t ConnectionBD_ADDR);
static DeviceInfo_t *SearchDeviceInfoEntryByBD_ADDR(DeviceInfo_t **ListHead, qapi_BLE_BD_ADDR_t RemoteAddress);
static DeviceInfo_t *SearchDeviceInfoEntryTypeAddress(DeviceInfo_t **ListHead, qapi_BLE_GAP_LE_Address_Type_t AddressType, qapi_BLE_BD_ADDR_t RemoteAddress);
static DeviceInfo_t *DeleteDeviceInfoEntry(DeviceInfo_t **ListHead, qapi_BLE_BD_ADDR_t BD_ADDR);
static void FreeDeviceInfoEntryMemory(DeviceInfo_t *EntryToFree);
static void FreeDeviceInfoList(DeviceInfo_t **ListHead);

static void BD_ADDRToStr(qapi_BLE_BD_ADDR_t Board_Address, BoardStr_t BoardStr);
static unsigned int StringToUnsignedInteger(char *StringInteger);
static void StrToBD_ADDR(char *BoardStr, qapi_BLE_BD_ADDR_t *Board_Address);

static void DisplayIOCapabilities(void);
static void DisplayAdvertisingData(qapi_BLE_GAP_LE_Advertising_Data_t *Advertising_Data);
static void DisplayLegacyPairingInformation(qapi_BLE_GAP_LE_Pairing_Capabilities_t *Pairing_Capabilities);
static void DisplayPairingInformation(qapi_BLE_GAP_LE_Extended_Pairing_Capabilities_t *Pairing_Capabilities);
static void DisplayUUID(qapi_BLE_GATT_UUID_t *UUID);
static void DisplayFunctionError(char *Function,int Status);
static void GenerateRandomKeys(void);

static int OpenStack(void);
static int CloseStack(void);
static int RegisterForHCIEvents(void);
static int RegisterForACLData(void);

static int SendACLData(unsigned int Length, unsigned char *Data);

static void EventCallback(uint32_t BluetoothStackID, qapi_BLE_HCI_Event_Data_t *HCI_Event_Data, uint32_t CallbackParameter);
static void ACLDataCallback(uint32_t BluetoothStackID, uint16_t Connection_Handle, uint16_t Flags, uint16_t ACLDataLength, uint8_t *ACLData, uint32_t CallbackParameter);

static QCLI_Command_Status_t StartTXTest(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t StartRXTest(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t StopTXRXTest(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);

static int SetPairable(void);

static void DumpAppearanceMappings(void);
static boolean_t AppearanceToString(uint16_t Appearance, char **String);
static boolean_t AppearanceIndexToAppearance(unsigned int Index, uint16_t *Appearance);

static void GAPSPopulateHandles(GAPS_Client_Info_t *ClientInfo, qapi_BLE_GATT_Service_Discovery_Indication_Data_t *ServiceInfo);

static int StartScan(uint32_t BluetoothStackID, qapi_BLE_GAP_LE_Filter_Policy_t FilterPolicy);
static int StopScan(uint32_t BluetoothStackID);

static int ConnectLEDevice(uint32_t BluetoothStackID, boolean_t UseWhiteList, qapi_BLE_BD_ADDR_t *BD_ADDR, qapi_BLE_GAP_LE_Address_Type_t AddressType);
static int DisconnectLEDevice(uint32_t BluetoothStackID, qapi_BLE_BD_ADDR_t BD_ADDR);

static void GenerateData2Send(unsigned int Length, uint8_t *Data);

static void Period_Tx_Callback(uint32_t data);

static void ConfigureCapabilities(qapi_BLE_GAP_LE_Extended_Pairing_Capabilities_t *Capabilities);
static int SendPairingRequest(qapi_BLE_BD_ADDR_t BD_ADDR, boolean_t ConnectionMaster);
static int SlavePairingRequestResponse(qapi_BLE_BD_ADDR_t BD_ADDR);
static int EncryptionInformationRequestResponse(qapi_BLE_BD_ADDR_t BD_ADDR, uint8_t KeySize, qapi_BLE_GAP_LE_Authentication_Response_Information_t *GAP_LE_Authentication_Response_Information);

static QCLI_Command_Status_t InitializeBluetooth(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t ShutdownBluetooth(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t GetVersion(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t EnableBBIF(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t SetBLERadio(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t CryptoTest(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t SetDiscoverabilityMode(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t SetConnectabilityMode(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t SetPairabilityMode(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t ChangePairingParameters(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t LEPassKeyResponse(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t LEQueryEncryption(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t LESetPasskey(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t GetLocalAddress(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t AdvertiseLE(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t ScanLE(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t ConnectLE(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t DisconnectLE(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t CancelConnectLE(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t PairLE(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t DiscoverGAPS(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t ReadLocalName(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t SetLocalName(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t ReadRemoteName(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t ReadLocalAppearance(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t SetLocalAppearance(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t ReadRemoteAppearance(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t GetGATTMTU(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t SetGATTMTU(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t GetScanParameters(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t SetScanParameters(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t GetAdvertisingParameters(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t SetAdvertisingParameters(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t SetTestAdvertisingDataLength(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t GetConnectionParameters(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t SetConnectionParameters(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t SetDefaultPacketSize(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t SetTestDataPeriod(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t AddDeviceToWhiteList(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t RemoveDeviceFromWhiteList(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t AddDeviceToResolvingList(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t RemoveDeviceFromResolvingList(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);

#ifdef V2
static QCLI_Command_Status_t M0_LMM_Mode_Switch(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
#endif

static QCLI_Command_Status_t RecieverTest(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t TransmitterTest(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t TestEnd(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);

   /* BTPS Callback function prototypes.                                */
static void QAPI_BLE_BTPSAPI GAP_LE_Event_Callback(uint32_t BluetoothStackID, qapi_BLE_GAP_LE_Event_Data_t *GAP_LE_Event_Data, uint32_t CallbackParameter);
static void QAPI_BLE_BTPSAPI GATT_ClientEventCallback_GAPS(uint32_t BluetoothStackID, qapi_BLE_GATT_Client_Event_Data_t *GATT_Client_Event_Data, uint32_t CallbackParameter);
static void QAPI_BLE_BTPSAPI GATT_Connection_Event_Callback(uint32_t BluetoothStackID, qapi_BLE_GATT_Connection_Event_Data_t *GATT_Connection_Event_Data, uint32_t CallbackParameter);
static void QAPI_BLE_BTPSAPI GATT_Service_Discovery_Event_Callback(uint32_t BluetoothStackID, qapi_BLE_GATT_Service_Discovery_Event_Data_t *GATT_Service_Discovery_Event_Data, uint32_t CallbackParameter);

   /*********************************************************************/
   /* qcli Command Group Definitions                                    */
   /*********************************************************************/
const QCLI_Command_t BLE_CMD_List[] =
{
   // cmd_function       start_thread      cmd_string                 usage_string description
   { InitializeBluetooth,      false, "InitializeBluetooth",          "",                                                                                                                                                                           "Initializes the Bluetopia stack (must be called before any other commands)."  },
   { ShutdownBluetooth,        false, "ShutdownBluetooth",            "",                                                                                                                                                                           "Shuts down the Bluetopia stack."                                              },
   { GetVersion,               false, "QueryVersion",                 "",                                                                                                                                                                           "Queries version of Bluetopia and Bluetooth Controller."                       },
   { EnableBBIF,               false, "EnableBBIF",                   "[Enable (0/1)]",                                                                                                                                                             "Enables or disables BLE BBIF."                                                },
   { SetBLERadio,              false, "SetBLERadio",                  "[Radio (1/2)]",                                                                                                                                                              "Sets the radio that BLE uses."                                                },
   { CryptoTest,               false, "CryptoTest",                   "",                                                                                                                                                                           "Runs test of HCI exposed crypto functionality."                               },
   { SetDiscoverabilityMode,   false, "SetDiscoverabilityMode",       "[Mode(0 = Non Discoverable, 1 = Limited Discoverable, 2 = General Discoverable)]",                                                                                           "Set device discoverability mode (for use during advertising)."                },
   { SetConnectabilityMode,    false, "SetConnectabilityMode",        "[(0 = NonConectable, 1 = Connectable)]",                                                                                                                                     "Set device connectability mode (for use during advertising)."                 },
   { SetPairabilityMode,       false, "SetPairabilityMode",           "[Mode (0 = Non Pairable, 1 = Pairable, 2 = Pairable w/ Secure Connections]",                                                                                                 "Set device pairability mode."                                                 },
   { ChangePairingParameters,  false, "ChangePairingParameters",      "[I/O Capability (0=DisplayOnly, 1=Display Yes/No, 2=Keyboard Only, 3=No Input/Output, 4=Keyboard/Display)] [MITM Req (0=No,1=Yes)] [Secure Connections (0 = None, 1 = Yes)", "Changes IO Capabilities used during pairing."                                 },
   { LEPassKeyResponse,        false, "LEPassKeyResponse",            "[Numeric Passkey(0 - 999999)]",                                                                                                                                              "Responds to passkey request."                                                 },
   { LEQueryEncryption,        false, "LEQueryEncryption",            "",                                                                                                                                                                           "Queries encryption state of LE connection."                                   },
   { LESetPasskey,             false, "LESetPasskey",                 "[(0 = Clear, 1 = Set) Passkey] [6 Digit Passkey (optional)]",                                                                                                                "Sets fixed passkey to use when IO caps are Display Only."                     },
   { GetLocalAddress,          false, "GetLocalAddress",              "",                                                                                                                                                                           "Querys Bluetooth Address of local Bluetooth controller."                      },
   { AdvertiseLE,              false, "AdvertiseLE",                  "[(0 = Disable, 1 = Enable)]",                                                                                                                                                "Starts/Stops Advertising Process."                                            },
   { ScanLE,                   false, "ScanLE",                       "[(0 = Disable, 1 = Enable)] [Filter Policy (0=No Filter, 1=White List, 2=No White List Directed RPA, 3=White List Directed RPA) (Optional)]",                                "Starts/Stops Scan Process."                                                   },
   { ConnectLE,                false, "ConnectLE",                    "[Use White List (0=False, 1=True)] [BD_ADDR (If Use White List=0)] [ADDR Type (0 = Public, 1 = Random, 2 = Public Identity, 3 = Random Identity) (If Use White List=0)] ",   "Sends an LE connection request to a remote device."                           },
   { DisconnectLE,             false, "DisconnectLE",                 "",                                                                                                                                                                           "Disconnects active LE connection."                                            },
   { CancelConnectLE,          false, "CancelConnectLE",              "",                                                                                                                                                                           "Cancels active LE connection process."                                        },
   { PairLE,                   false, "PairLE",                       "",                                                                                                                                                                           "Starts pairing process with active LE connection."                            },
   { DiscoverGAPS,             false, "DiscoverGAPS",                 "",                                                                                                                                                                           "Starts process to discover GAP Service on active LE connection."              },
   { ReadLocalName,            false, "ReadLocalName",                "",                                                                                                                                                                           "Reads local friendly name."                                                   },
   { SetLocalName,             false, "SetLocalName",                 "[Name]",                                                                                                                                                                     "Sets local friendly name."                                                    },
   { ReadRemoteName,           false, "ReadRemoteName",               "",                                                                                                                                                                           "Read friendly name of remote device."                                         },
   { ReadLocalAppearance,      false, "ReadLocalAppearance",          "",                                                                                                                                                                           "Reads local device apperance value."                                          },
   { SetLocalAppearance,       false, "SetLocalAppearance",           "[Appearance Index]",                                                                                                                                                         "Set local device appearance value."                                           },
   { ReadRemoteAppearance,     false, "ReadRemoteAppearance",         "",                                                                                                                                                                           "Reads device appearance of remote device."                                    },
   { GetGATTMTU,               false, "GetGATTMTU",                   "",                                                                                                                                                                           "Queries maximum supported GATT MTU."                                          },
   { SetGATTMTU,               false, "SetGATTMTU",                   "[MTU]",                                                                                                                                                                      "Changes the maximum supported GATT MTU."                                      },
   { GetScanParameters,        false, "GetScanParameters",            "",                                                                                                                                                                           "Query the current BLE scan parameters."                                       },
   { SetScanParameters,        false, "SetScanParameters",            "[Scan Interval (ms)] [Scan Window (ms)] [Enable Active Scanning(bool)] [Filter Duplicates (bool)]",                                                                          "Set BLE scan parameters."                                                     },
   { GetAdvertisingParameters, false, "GetAdvertisingParameters",     "",                                                                                                                                                                           "Query the current BLE advertising parameters."                                },
   { SetAdvertisingParameters, false, "SetAdvertisingParameters",     "[Minimum Advertising Interval (ms)] [Maximum Advertising Interval (ms)] [Enable Scan Response(1=enabled,0=disabled)] [Whilte List Filter (optional 1=enabled,0=disabled)]",  "Set BLE advertising parameters."                                              },
   { SetTestAdvertisingDataLength, false, "SetTestAdvertisingData",     "[Advertising Data Length (Bytes, 3 - 31)]",                                                                                                                                "Sets length of advertising data packet for testing."                          },
   { GetConnectionParameters,  false, "GetConnectionParameters",      "",                                                                                                                                                                           "Query the current BLE connection parameters."                                 },
   { SetConnectionParameters,  false, "SetConnectionParameters",      "[Minimum Connection Interval (ms)] [Maximum Connection Interval (ms)] [Slave Latency (number of Connection Events)]",                                                        "Set BLE connection parameters."                                               },
   { SetTestDataPeriod,        false, "SetTestDataPeriod",            "[Period to wakeup and send data (milliseconds)]",                                                                                                                            "Sets a the periodic interval to wakeup and send data on connected device."    },
   { AddDeviceToWhiteList,     false, "AddDeviceToWhiteList",         "[BD_ADDR] [Address Type (0=Public, 1=Random, 2=Public Identity, 3=Random Identity)]",                                                                                        "Adds a device to the white list in the controller."                           },
   { RemoveDeviceFromWhiteList,false, "RemoveDeviceFromWhiteList",    "[BD_ADDR] [Address Type (0=Public, 1=Random, 2=Public Identity, 3=Random Identity)]",                                                                                                                                                                  "Removes a device from the white list in the controller."                      },
   { AddDeviceToResolvingList,     false, "AddDeviceToResolvingList",      "[BD_ADDR]",                                                                                                                                                             "Adds a device to the resolving list in the controller."                       },
   { RemoveDeviceFromResolvingList,false, "RemoveDeviceFromResolvingList", "[BD_ADDR]",                                                                                                                                                             "Removes a device from the resolving list in the controller."                  },
   { SetDefaultPacketSize,     false, "SetDefaultPacketSize",         "[Packet Size (Bytes, >= 27]",                                                                                                                                                "Set Maximum over the air packet size in bytes (DLE testing)."                 },
   { SetTestDataPeriod,        false, "SetTestDataPeriod",            "[BD_ADDR] [Period to wakeup and send data (milliseconds)] [Size of data to send (Bytes, >= 1)]",                                                                             "Sets a the periodic interval to wakeup and send data on connected device."    },
#ifdef V2
   { M0_LMM_Mode_Switch,       false, "M0_LMM_Mode_Switch",           "[1 = Enter LMM, 0 = Exit LMM]",                                                                                                                                              "Enter/Exit LMM on M0."                                                        },
#endif
   { StartTXTest,              false, "StartACLTxTest",               "[BD_ADDR] [Packet Size (Bytes, >= 1)]",                                                                                                                                      "Starts HCI Transmit Packet Test."                                             },
   { StartRXTest,              false, "StartRxTest",                  "[BD_ADDR]",                                                                                                                                                                  "Starts Data Receive Packet Test."                                             },
   { StopTXRXTest,             false, "StopTxRxTest",                 "",                                                                                                                                                                           "Stops Data Transmit/Receive Packet Test."                                     }
};

const QCLI_Command_Group_t BLE_CMD_Group = {"BLE", (sizeof(BLE_CMD_List) / sizeof(QCLI_Command_t)), BLE_CMD_List};

const QCLI_Command_t BLE_DUT_CMD_List[] =
{
   // cmd_function    start_thread      cmd_string  usage_string description
   { RecieverTest,    false, "RecieverTest",    "[Frequency]",                        "Start a receiver test." },
   { TransmitterTest, false, "TransmitterTest", "[Frequency] [Length] [Payload 0-7]", "Start a transmitter test." },
   { TestEnd,         false, "TestEnd",         "",                                   "Ends a receiver or transmitter test." },
};

const QCLI_Command_Group_t BLE_DUT_CMD_Group = {"DUT", (sizeof(BLE_DUT_CMD_List) / sizeof(QCLI_Command_t)), BLE_DUT_CMD_List};

   /* The following function adds the specified Entry to the specified  */
   /* List.  This function allocates and adds an entry to the list that */
   /* has the same attributes as parameters to this function.  This     */
   /* function will return FALSE if NO Entry was added.  This can occur */
   /* if the element passed in was deemed invalid or the actual List    */
   /* Head was invalid.                                                 */
   /* ** NOTE ** This function does not insert duplicate entries into   */
   /*            the list.  An element is considered a duplicate if the */
   /*            Connection BD_ADDR.  When this occurs, this function   */
   /*            returns NULL.                                          */
static boolean_t CreateNewDeviceInfoEntry(DeviceInfo_t **ListHead, qapi_BLE_GAP_LE_Address_Type_t ConnectionAddressType, qapi_BLE_BD_ADDR_t ConnectionBD_ADDR)
{
   boolean_t     ret_val = FALSE;
   DeviceInfo_t *DeviceInfoPtr;

   /* Verify that the passed in parameters seem semi-valid.             */
   if((ListHead) && (!QAPI_BLE_COMPARE_NULL_BD_ADDR(ConnectionBD_ADDR)))
   {
      /* Allocate the memory for the entry.                             */
      if((DeviceInfoPtr = malloc(sizeof(DeviceInfo_t))) != NULL)
      {
         /* Initialize the entry.                                       */
         memset(DeviceInfoPtr, 0, sizeof(DeviceInfo_t));
         DeviceInfoPtr->ConnectionAddressType = ConnectionAddressType;
         memcpy(&(DeviceInfoPtr->ConnectionBD_ADDR), &ConnectionBD_ADDR, sizeof(ConnectionBD_ADDR));

         ret_val = qapi_BLE_BSC_AddGenericListEntry_Actual(QAPI_BLE_EK_BD_ADDR_T_E, STRUCTURE_OFFSET(DeviceInfo_t, ConnectionBD_ADDR), STRUCTURE_OFFSET(DeviceInfo_t, NextDeviceInfoInfoPtr), (void **)(ListHead), (void *)(DeviceInfoPtr));
         if(!ret_val)
         {
            /* Failed to add to list so we should free the memory that  */
            /* we allocated for the entry.                              */
            free(DeviceInfoPtr);
         }
      }
   }

   return(ret_val);
}

   /* The following function searches the specified List for the        */
   /* specified Connection BD_ADDR.  This function returns NULL if      */
   /* either the List Head is invalid, the BD_ADDR is invalid, or the   */
   /* Connection BD_ADDR was NOT found.                                 */
static DeviceInfo_t *SearchDeviceInfoEntryByBD_ADDR(DeviceInfo_t **ListHead, qapi_BLE_BD_ADDR_t RemoteAddress)
{
   BoardStr_t    BoardStr;
   DeviceInfo_t *ret_val = NULL;
   DeviceInfo_t *DeviceInfo;

   /* Verify the list head.                                             */
   if(ListHead)
   {
      /* Loop through the device information.                           */
      DeviceInfo = *ListHead;
      while(DeviceInfo)
      {
         /* If the BD_ADDR is a match then we found the remote device   */
         /* information.                                                */
         if(QAPI_BLE_COMPARE_BD_ADDR(DeviceInfo->ConnectionBD_ADDR, RemoteAddress))
         {
            /* Set the remote device information pointer to the return  */
            /* value and break since we are done.                       */
            ret_val = DeviceInfo;
            break;
         }
         else
         {
            /* Determine if the remote device is using a resolvable     */
            /* private address (RPA).                                   */
            if(QAPI_BLE_GAP_LE_TEST_RESOLVABLE_ADDRESS_BITS(RemoteAddress))
            {
               /* Check if we stored the Identity Resolving Key (IRK)   */
               /* for the remote device.                                */
               if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_IRK_VALID)
               {
                  /* Use the IRK to resolve the address.                */
                  if(qapi_BLE_GAP_LE_Resolve_Address(BLE_Demo_Context.BluetoothStackID, &(DeviceInfo->IRK), RemoteAddress))
                  {
                     /* If we resolved the address let's update the     */
                     /* Bluetooth address stored for the remote device. */
                     /* * NOTE * We are doing this so we don't have to  */
                     /*          re-resolve the remote device address   */
                     /*          for future connections.  However, if   */
                     /*          the resolvable address changes we will */
                     /*          need to resolve it again.              */
                     DeviceInfo->ConnectionAddressType = QAPI_BLE_LAT_RANDOM_E;
                     DeviceInfo->ConnectionBD_ADDR     = RemoteAddress;

                     /* Inform the user we resolved the address.        */
                     QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "\n");
                     QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Resolved Address (");
                     BD_ADDRToStr(DeviceInfo->ConnectionBD_ADDR, BoardStr);
                     QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "%s", BoardStr);
                     QCLI_Printf(BLE_Demo_Context.QCLI_Handle, ")\n");
                     QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   Identity Address:       ");
                     BD_ADDRToStr(DeviceInfo->IdentityAddressBD_ADDR, BoardStr);
                     QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "%s\n", BoardStr);
                     QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   Identity Address Type:  %s\n", ((DeviceInfo->IdentityAddressType == QAPI_BLE_LAT_PUBLIC_IDENTITY_E) ? "Public Identity" : "Random Identity"));

                     /* Set the remote device information pointer to the*/
                     /* return value and break since we are done.       */
                     ret_val = DeviceInfo;
                     break;
                  }
               }
            }
         }

         DeviceInfo = DeviceInfo->NextDeviceInfoInfoPtr;
      }
   }

   return(ret_val);
}

   /* The following function searches the specified List for the        */
   /* specified Address and Type.  This function returns NULL if        */
   /* either the List Head is invalid, the BD_ADDR is invalid, or the   */
   /* Connection BD_ADDR was NOT found.                                 */
static DeviceInfo_t *SearchDeviceInfoEntryTypeAddress(DeviceInfo_t **ListHead, qapi_BLE_GAP_LE_Address_Type_t AddressType, qapi_BLE_BD_ADDR_t RemoteAddress)
{
   BoardStr_t                      BoardStr;
   DeviceInfo_t                   *ret_val = NULL;
   DeviceInfo_t                   *DeviceInfo;
   qapi_BLE_GAP_LE_Address_Type_t  TempType;

   /* Verify the list head.                                             */
   if(ListHead)
   {
      /* Loop through the device information.                           */
      DeviceInfo = *ListHead;
      while(DeviceInfo)
      {
         /* If the BD_ADDR is a match then we found the remote device   */
         /* information.                                                */
         if((DeviceInfo->ConnectionAddressType == AddressType) && (QAPI_BLE_COMPARE_BD_ADDR(DeviceInfo->ConnectionBD_ADDR, RemoteAddress)))
         {
            /* Set the remote device information pointer to the return  */
            /* value and break since we are done.                       */
            ret_val = DeviceInfo;
            break;
         }
         else
         {
            /* Check to see if this is an identity address.             */
            if((AddressType == QAPI_BLE_LAT_PUBLIC_IDENTITY_E) || (AddressType == QAPI_BLE_LAT_RANDOM_IDENTITY_E))
            {
               /* Convert the address type.                             */
               if(AddressType == QAPI_BLE_LAT_PUBLIC_IDENTITY_E)
                  TempType = QAPI_BLE_LAT_PUBLIC_E;
               else
                  TempType = QAPI_BLE_LAT_RANDOM_E;

               /* Check if the identity address field is valid.         */
               if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_IRK_VALID)
               {
                  /* Compare the identity address and type.             */
                  if((DeviceInfo->IdentityAddressType == TempType) && (QAPI_BLE_COMPARE_BD_ADDR(DeviceInfo->IdentityAddressBD_ADDR, RemoteAddress)))
                  {
                     /* Update the address field for this entry.        */
                     DeviceInfo->ConnectionAddressType = AddressType;
                     DeviceInfo->ConnectionBD_ADDR     = DeviceInfo->IdentityAddressBD_ADDR;

                     /* Set the remote device information pointer to the*/
                     /* return value and break since we are done.       */
                     ret_val = DeviceInfo;
                     break;
                  }
               }
            }
            else
            {
                /* Determine if the remote device is using a resolvable     */
                /* private address (RPA).                                   */
                if((AddressType == QAPI_BLE_LAT_RANDOM_E) && (QAPI_BLE_GAP_LE_TEST_RESOLVABLE_ADDRESS_BITS(RemoteAddress)))
                {
                   /* Check if we stored the Identity Resolving Key (IRK)   */
                   /* for the remote device.                                */
                   if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_IRK_VALID)
                   {
                      /* Use the IRK to resolve the address.                */
                      if(qapi_BLE_GAP_LE_Resolve_Address(BLE_Demo_Context.BluetoothStackID, &(DeviceInfo->IRK), RemoteAddress))
                      {
                         /* If we resolved the address let's update the     */
                         /* Bluetooth address stored for the remote device. */
                         /* * NOTE * We are doing this so we don't have to  */
                         /*          re-resolve the remote device address   */
                         /*          for future connections.  However, if   */
                         /*          the resolvable address changes we will */
                         /*          need to resolve it again.              */
                         DeviceInfo->ConnectionAddressType = QAPI_BLE_LAT_RANDOM_E;
                         DeviceInfo->ConnectionBD_ADDR     = RemoteAddress;

                         /* Inform the user we resolved the address.        */
                         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "\n");
                         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Resolved Address (");
                         BD_ADDRToStr(DeviceInfo->ConnectionBD_ADDR, BoardStr);
                         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "%s", BoardStr);
                         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, ")\n");
                         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   Identity Address:       ");
                         BD_ADDRToStr(DeviceInfo->IdentityAddressBD_ADDR, BoardStr);
                         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "%s\n", BoardStr);
                         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   Identity Address Type:  %s\n", ((DeviceInfo->IdentityAddressType == QAPI_BLE_LAT_PUBLIC_IDENTITY_E) ? "Public Identity" : "Random Identity"));

                         /* Set the remote device information pointer to the*/
                         /* return value and break since we are done.       */
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

   return(ret_val);
}

   /* The following function searches the specified Key Info List for   */
   /* the specified BD_ADDR and removes it from the List.  This function*/
   /* returns NULL if either the List Head is invalid, the BD_ADDR is   */
   /* invalid, or the specified Entry was NOT present in the list.  The */
   /* entry returned will have the Next Entry field set to NULL, and    */
   /* the caller is responsible for deleting the memory associated with */
   /* this entry by calling the FreeKeyEntryMemory() function.          */
static DeviceInfo_t *DeleteDeviceInfoEntry(DeviceInfo_t **ListHead, qapi_BLE_BD_ADDR_t BD_ADDR)
{
   return(qapi_BLE_BSC_DeleteGenericListEntry(QAPI_BLE_EK_BD_ADDR_T_E, (void *)(&BD_ADDR), STRUCTURE_OFFSET(DeviceInfo_t, ConnectionBD_ADDR), STRUCTURE_OFFSET(DeviceInfo_t, NextDeviceInfoInfoPtr), (void **)(ListHead)));
}

   /* This function frees the specified Key Info Information member     */
   /* memory.                                                           */
static void FreeDeviceInfoEntryMemory(DeviceInfo_t *EntryToFree)
{
   qapi_BLE_BSC_FreeGenericListEntryMemory((void *)(EntryToFree));
}

   /* The following function deletes (and frees all memory) every       */
   /* element of the specified Key Info List. Upon return of this       */
   /* function, the Head Pointer is set to NULL.                        */
static void FreeDeviceInfoList(DeviceInfo_t **ListHead)
{
   qapi_BLE_BSC_FreeGenericListEntryList((void **)(ListHead), STRUCTURE_OFFSET(DeviceInfo_t, NextDeviceInfoInfoPtr));
}

   /* The following function is responsible for converting data of type */
   /* BD_ADDR to a string.  The first parameter of this function is the */
   /* BD_ADDR to be converted to a string.  The second parameter of this*/
   /* function is a pointer to the string in which the converted BD_ADDR*/
   /* is to be stored.                                                  */
static void BD_ADDRToStr(qapi_BLE_BD_ADDR_t Board_Address, BoardStr_t BoardStr)
{
   snprintf((char *)BoardStr, (sizeof(BoardStr_t)/sizeof(char)), "0x%02X%02X%02X%02X%02X%02X", Board_Address.BD_ADDR5, Board_Address.BD_ADDR4, Board_Address.BD_ADDR3, Board_Address.BD_ADDR2, Board_Address.BD_ADDR1, Board_Address.BD_ADDR0);
}

   /* The following function is responsible for converting number       */
   /* strings to there unsigned integer equivalent.  This function can  */
   /* handle leading and tailing white space, however it does not handle*/
   /* signed or comma delimited values.  This function takes as its     */
   /* input the string which is to be converted.  The function returns  */
   /* zero if an error occurs otherwise it returns the value parsed from*/
   /* the string passed as the input parameter.                         */
static unsigned int StringToUnsignedInteger(char *StringInteger)
{
   int          IsHex;
   unsigned int Index;
   unsigned int ret_val = 0;

   /* Before proceeding make sure that the parameter that was passed as */
   /* an input appears to be at least semi-valid.                       */
   if((StringInteger) && (strlen(StringInteger)))
   {
      /* Initialize the variable.                                       */
      Index = 0;

      /* Next check to see if this is a hexadecimal number.             */
      if(strlen(StringInteger) > 2)
      {
         if((StringInteger[0] == '0') && ((StringInteger[1] == 'x') || (StringInteger[1] == 'X')))
         {
            IsHex = 1;

            /* Increment the String passed the Hexadecimal prefix.      */
            StringInteger += 2;
         }
         else
            IsHex = 0;
      }
      else
         IsHex = 0;

      /* Process the value differently depending on whether or not a    */
      /* Hexadecimal Number has been specified.                         */
      if(!IsHex)
      {
         /* Decimal Number has been specified.                          */
         while(1)
         {
            /* First check to make sure that this is a valid decimal    */
            /* digit.                                                   */
            if((StringInteger[Index] >= '0') && (StringInteger[Index] <= '9'))
            {
               /* This is a valid digit, add it to the value being      */
               /* built.                                                */
               ret_val += (StringInteger[Index] & 0xF);

               /* Determine if the next digit is valid.                 */
               if(((Index + 1) < strlen(StringInteger)) && (StringInteger[Index+1] >= '0') && (StringInteger[Index+1] <= '9'))
               {
                  /* The next digit is valid so multiply the current    */
                  /* return value by 10.                                */
                  ret_val *= 10;
               }
               else
               {
                  /* The next value is invalid so break out of the loop.*/
                  break;
               }
            }

            Index++;
         }
      }
      else
      {
         /* Hexadecimal Number has been specified.                      */
         while(1)
         {
            /* First check to make sure that this is a valid Hexadecimal*/
            /* digit.                                                   */
            if(((StringInteger[Index] >= '0') && (StringInteger[Index] <= '9')) || ((StringInteger[Index] >= 'a') && (StringInteger[Index] <= 'f')) || ((StringInteger[Index] >= 'A') && (StringInteger[Index] <= 'F')))
            {
               /* This is a valid digit, add it to the value being      */
               /* built.                                                */
               if((StringInteger[Index] >= '0') && (StringInteger[Index] <= '9'))
                  ret_val += (StringInteger[Index] & 0xF);
               else
               {
                  if((StringInteger[Index] >= 'a') && (StringInteger[Index] <= 'f'))
                     ret_val += (StringInteger[Index] - 'a' + 10);
                  else
                     ret_val += (StringInteger[Index] - 'A' + 10);
               }

               /* Determine if the next digit is valid.                 */
               if(((Index + 1) < strlen(StringInteger)) && (((StringInteger[Index+1] >= '0') && (StringInteger[Index+1] <= '9')) || ((StringInteger[Index+1] >= 'a') && (StringInteger[Index+1] <= 'f')) || ((StringInteger[Index+1] >= 'A') && (StringInteger[Index+1] <= 'F'))))
               {
                  /* The next digit is valid so multiply the current    */
                  /* return value by 16.                                */
                  ret_val *= 16;
               }
               else
               {
                  /* The next value is invalid so break out of the loop.*/
                  break;
               }
            }

            Index++;
         }
      }
   }

   return(ret_val);
}

   /* The following function is responsible for the specified string    */
   /* into data of type BD_ADDR.  The first parameter of this function  */
   /* is the BD_ADDR string to be converted to a BD_ADDR.  The second   */
   /* parameter of this function is a pointer to the BD_ADDR in which   */
   /* the converted BD_ADDR String is to be stored.                     */
static void StrToBD_ADDR(char *BoardStr, qapi_BLE_BD_ADDR_t *Board_Address)
{
   char Buffer[5];

   if((BoardStr) && (strlen(BoardStr) == sizeof(qapi_BLE_BD_ADDR_t)*2) && (Board_Address))
   {
      Buffer[0] = '0';
      Buffer[1] = 'x';
      Buffer[4] = '\0';

      Buffer[2] = BoardStr[0];
      Buffer[3] = BoardStr[1];
      Board_Address->BD_ADDR5 = (uint8_t)StringToUnsignedInteger(Buffer);

      Buffer[2] = BoardStr[2];
      Buffer[3] = BoardStr[3];
      Board_Address->BD_ADDR4 = (uint8_t)StringToUnsignedInteger(Buffer);

      Buffer[2] = BoardStr[4];
      Buffer[3] = BoardStr[5];
      Board_Address->BD_ADDR3 = (uint8_t)StringToUnsignedInteger(Buffer);

      Buffer[2] = BoardStr[6];
      Buffer[3] = BoardStr[7];
      Board_Address->BD_ADDR2 = (uint8_t)StringToUnsignedInteger(Buffer);

      Buffer[2] = BoardStr[8];
      Buffer[3] = BoardStr[9];
      Board_Address->BD_ADDR1 = (uint8_t)StringToUnsignedInteger(Buffer);

      Buffer[2] = BoardStr[10];
      Buffer[3] = BoardStr[11];
      Board_Address->BD_ADDR0 = (uint8_t)StringToUnsignedInteger(Buffer);
   }
   else
   {
      if(Board_Address)
         memset(Board_Address, 0, sizeof(qapi_BLE_BD_ADDR_t));
   }
}

   /* Displays the current I/O Capabilities.                            */
static void DisplayIOCapabilities(void)
{
   QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "I/O Capabilities: %s, MITM: %s. Secure Connections: %s.\n", IOCapabilitiesStrings[(unsigned int)(BLE_Demo_Context.LE_Parameters.IOCapability - QAPI_BLE_LIC_DISPLAY_ONLY_E)], BLE_Demo_Context.LE_Parameters.MITMProtection?"TRUE":"FALSE", BLE_Demo_Context.LE_Parameters.SecureConnections?"TRUE":"FALSE");
}

   /* Utility function to display advertising data.                     */
static void DisplayAdvertisingData(qapi_BLE_GAP_LE_Advertising_Data_t *Advertising_Data)
{
   unsigned int Index;
   unsigned int Index2;

   /* Verify that the input parameters seem semi-valid.                 */
   if(Advertising_Data)
   {
      for(Index = 0; Index < Advertising_Data->Number_Data_Entries; Index++)
      {
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "  AD Type:   0x%02X.\n", (unsigned int)(Advertising_Data->Data_Entries[Index].AD_Type));
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "  AD Length: 0x%02X.\n", (unsigned int)(Advertising_Data->Data_Entries[Index].AD_Data_Length));
         if(Advertising_Data->Data_Entries[Index].AD_Data_Buffer)
         {
            QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "  AD Data: ");
            for(Index2 = 0; Index2 < Advertising_Data->Data_Entries[Index].AD_Data_Length; Index2++)
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "0x%02X ", Advertising_Data->Data_Entries[Index].AD_Data_Buffer[Index2]);

            QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "\n");
         }
      }
   }
}

   /* The following function displays the pairing capabilities that is  */
   /* passed into this function.                                        */
static void DisplayPairingInformation(qapi_BLE_GAP_LE_Extended_Pairing_Capabilities_t *Pairing_Capabilities)
{
   /* Display the IO Capability.                                        */
   switch(Pairing_Capabilities->IO_Capability)
   {
      case QAPI_BLE_LIC_DISPLAY_ONLY_E:
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   IO Capability:       Display Only.\n");
         break;
      case QAPI_BLE_LIC_DISPLAY_YES_NO_E:
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   IO Capability:       Display Yes/No.\n");
         break;
      case QAPI_BLE_LIC_KEYBOARD_ONLY_E:
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   IO Capability:       Keyboard Only.\n");
         break;
      case QAPI_BLE_LIC_NO_INPUT_NO_OUTPUT_E:
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   IO Capability:       No Input No Output.\n");
         break;
      case QAPI_BLE_LIC_KEYBOARD_DISPLAY_E:
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   IO Capability:       Keyboard/Display.\n");
         break;
   }

   QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   Bonding Type:        %s.\n", (Pairing_Capabilities->Bonding_Type == QAPI_BLE_LBT_BONDING_E)?"Bonding":"No Bonding");
   QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   MITM:                %s.\n", (Pairing_Capabilities->Flags & QAPI_BLE_GAP_LE_EXTENDED_PAIRING_CAPABILITIES_FLAGS_MITM_REQUESTED)?"TRUE":"FALSE");
   QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   Secure Connections:  %s.\n", (Pairing_Capabilities->Flags & QAPI_BLE_GAP_LE_EXTENDED_PAIRING_CAPABILITIES_FLAGS_SECURE_CONNECTIONS)?"TRUE":"FALSE");
   QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   OOB:                 %s.\n", (Pairing_Capabilities->Flags & QAPI_BLE_GAP_LE_EXTENDED_PAIRING_CAPABILITIES_FLAGS_OOB_DATA_PRESENT)?"OOB":"OOB Not Present");
   QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   Encryption Key Size: %d.\n", Pairing_Capabilities->Maximum_Encryption_Key_Size);
   QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   Sending Keys: \n");
   QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "      LTK:              %s.\n", ((Pairing_Capabilities->Sending_Keys.Encryption_Key)?"YES":"NO"));
   QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "      IRK:              %s.\n", ((Pairing_Capabilities->Sending_Keys.Identification_Key)?"YES":"NO"));
   QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "      CSRK:             %s.\n", ((Pairing_Capabilities->Sending_Keys.Signing_Key)?"YES":"NO"));
   QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "      Link Key:         %s.\n", ((Pairing_Capabilities->Sending_Keys.Link_Key)?"YES":"NO"));
   QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   Receiving Keys: \n");
   QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "      LTK:              %s.\n", ((Pairing_Capabilities->Receiving_Keys.Encryption_Key)?"YES":"NO"));
   QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "      IRK:              %s.\n", ((Pairing_Capabilities->Receiving_Keys.Identification_Key)?"YES":"NO"));
   QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "      CSRK:             %s.\n", ((Pairing_Capabilities->Receiving_Keys.Signing_Key)?"YES":"NO"));
   QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "      Link Key:         %s.\n", ((Pairing_Capabilities->Receiving_Keys.Link_Key)?"YES":"NO"));
}

   /* The following function displays the pairing capabilities that is  */
   /* passed into this function.                                        */
static void DisplayLegacyPairingInformation(qapi_BLE_GAP_LE_Pairing_Capabilities_t *Pairing_Capabilities)
{
   /* Display the IO Capability.                                        */
   switch(Pairing_Capabilities->IO_Capability)
   {
      case QAPI_BLE_LIC_DISPLAY_ONLY_E:
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   IO Capability:       Display Only.\n");
         break;
      case QAPI_BLE_LIC_DISPLAY_YES_NO_E:
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   IO Capability:       Display Yes/No.\n");
         break;
      case QAPI_BLE_LIC_KEYBOARD_ONLY_E:
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   IO Capability:       Keyboard Only.\n");
         break;
      case QAPI_BLE_LIC_NO_INPUT_NO_OUTPUT_E:
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   IO Capability:       No Input No Output.\n");
         break;
      case QAPI_BLE_LIC_KEYBOARD_DISPLAY_E:
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   IO Capability:       Keyboard/Display.\n");
         break;
   }

   QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   MITM:                %s.\n", (Pairing_Capabilities->MITM)?"TRUE":"FALSE");
   QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   Bonding Type:        %s.\n", (Pairing_Capabilities->Bonding_Type == QAPI_BLE_LBT_BONDING_E)?"Bonding":"No Bonding");
   QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   OOB:                 %s.\n", (Pairing_Capabilities->OOB_Present)?"OOB":"OOB Not Present");
   QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   Encryption Key Size: %d.\n", Pairing_Capabilities->Maximum_Encryption_Key_Size);
   QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   Sending Keys: \n");
   QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "      LTK:              %s.\n", ((Pairing_Capabilities->Sending_Keys.Encryption_Key)?"YES":"NO"));
   QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "      IRK:              %s.\n", ((Pairing_Capabilities->Sending_Keys.Identification_Key)?"YES":"NO"));
   QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "      CSRK:             %s.\n", ((Pairing_Capabilities->Sending_Keys.Signing_Key)?"YES":"NO"));
   QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   Receiving Keys: \n");
   QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "      LTK:              %s.\n", ((Pairing_Capabilities->Receiving_Keys.Encryption_Key)?"YES":"NO"));
   QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "      IRK:              %s.\n", ((Pairing_Capabilities->Receiving_Keys.Identification_Key)?"YES":"NO"));
   QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "      CSRK:             %s.\n", ((Pairing_Capabilities->Receiving_Keys.Signing_Key)?"YES":"NO"));
}

   /* The following function is provided to properly print a UUID.      */
static void DisplayUUID(qapi_BLE_GATT_UUID_t *UUID)
{
   if(UUID)
   {
      if(UUID->UUID_Type == QAPI_BLE_GU_UUID_16_E)
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "%02X%02X\n", UUID->UUID.UUID_16.UUID_Byte1, UUID->UUID.UUID_16.UUID_Byte0);
      else
      {
         if(UUID->UUID_Type == QAPI_BLE_GU_UUID_128_E)
         {
            QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X\n", UUID->UUID.UUID_128.UUID_Byte15, UUID->UUID.UUID_128.UUID_Byte14, UUID->UUID.UUID_128.UUID_Byte13,
                                                                                                    UUID->UUID.UUID_128.UUID_Byte12, UUID->UUID.UUID_128.UUID_Byte11, UUID->UUID.UUID_128.UUID_Byte10,
                                                                                                    UUID->UUID.UUID_128.UUID_Byte9,  UUID->UUID.UUID_128.UUID_Byte8,  UUID->UUID.UUID_128.UUID_Byte7,
                                                                                                    UUID->UUID.UUID_128.UUID_Byte6,  UUID->UUID.UUID_128.UUID_Byte5,  UUID->UUID.UUID_128.UUID_Byte4,
                                                                                                    UUID->UUID.UUID_128.UUID_Byte3,  UUID->UUID.UUID_128.UUID_Byte2,  UUID->UUID.UUID_128.UUID_Byte1,
                                                                                                    UUID->UUID.UUID_128.UUID_Byte0);
         }
      }
   }
}

   /* Displays a function error message.                                */
static void DisplayFunctionError(char *Function, int Status)
{
   QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "%s Failed: %d.\n", Function, Status);
}

   /* The following function is a utility function which is used to     */
   /* generate random values for the ER and the IR.                     */
static void GenerateRandomKeys(void)
{
   uint8_t                  Status;
   unsigned int             MaxSize;
   qapi_BLE_Random_Number_t RandomNumber;

   /* Initialize the max size.                                          */
   MaxSize = (sizeof(qapi_BLE_Random_Number_t) > (sizeof(qapi_BLE_Encryption_Key_t) / 2)) ? (sizeof(qapi_BLE_Encryption_Key_t) / 2) : sizeof(qapi_BLE_Random_Number_t);

   /* If the ER is not valid generate a random ER key.                  */
   if(!(BLE_Demo_Context.ContextFlags & BLE_DEMO_CONTEXT_FLAGS_ER_VALID))
   {
      if((!qapi_BLE_HCI_LE_Rand(BLE_Demo_Context.BluetoothStackID,  &Status,  &RandomNumber)) && (!Status))
         memcpy(&(BLE_Demo_Context.ER), &RandomNumber, MaxSize);

      if((!qapi_BLE_HCI_LE_Rand(BLE_Demo_Context.BluetoothStackID,  &Status,  &RandomNumber)) && (!Status))
         memcpy(&(((uint8_t *)&(BLE_Demo_Context.ER))[sizeof(qapi_BLE_Encryption_Key_t) / 2]), &RandomNumber, MaxSize);

      BLE_Demo_Context.ContextFlags |= BLE_DEMO_CONTEXT_FLAGS_ER_VALID;
   }

   /* If the ER is not valid generate a random ER key.                  */
   if(!(BLE_Demo_Context.ContextFlags & BLE_DEMO_CONTEXT_FLAGS_IR_VALID))
   {
      if((!qapi_BLE_HCI_LE_Rand(BLE_Demo_Context.BluetoothStackID,  &Status,  &RandomNumber)) && (!Status))
         memcpy(&(BLE_Demo_Context.IR), &RandomNumber, MaxSize);

      if((!qapi_BLE_HCI_LE_Rand(BLE_Demo_Context.BluetoothStackID,  &Status,  &RandomNumber)) && (!Status))
         memcpy(&(((uint8_t *)&(BLE_Demo_Context.IR))[sizeof(qapi_BLE_Encryption_Key_t) / 2]), &RandomNumber, MaxSize);

      BLE_Demo_Context.ContextFlags |= BLE_DEMO_CONTEXT_FLAGS_IR_VALID;
   }
}

   /* The following function is responsible for opening the SS1         */
   /* Bluetooth Protocol Stack.  This function accepts a pre-populated  */
   /* HCI Driver Information structure that contains the HCI Driver     */
   /* Transport Information.  This function returns zero on successful  */
   /* execution and a negative value on all errors.                     */
static int OpenStack(void)
{
   int                              Result;
   int                              ret_val = 0;
   uint8_t                          TempData;
   BoardStr_t                       AddressString;
   uint8_t                          Status;
   uint8_t                          HC_SCO_Data_Packet_Length;
   uint16_t                         HC_Total_Num_SCO_Data_Packets;
   uint16_t                         HC_Total_Num_LE_Data_Packets;
   uint16_t                         HC_LE_Data_Packet_Length;
   uint32_t                         ServiceID;
   qapi_BLE_HCI_Version_t           HCIVersion;
   qapi_BLE_HCI_DriverInformation_t HCI_DriverInformation;

   /* First check to see if the Stack has already been opened.          */
   if(!BLE_Demo_Context.BluetoothStackID)
   {
      QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "\n");
      QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "OpenStack().\n");

      /* Set the HCI driver information.                                */
      QAPI_BLE_HCI_DRIVER_SET_COMM_INFORMATION(&HCI_DriverInformation, 1, 115200, QAPI_BLE_COMM_PROTOCOL_UART_E);

      /* Initialize the Stack                                           */
      Result = qapi_BLE_BSC_Initialize(&HCI_DriverInformation, 0);

      /* Next, check the return value of the initialization to see if it*/
      /* was successful.                                                */
      if(Result > 0)
      {
         /* The Stack was initialized successfully, inform the user and */
         /* set the return value of the initialization function to the  */
         /* Bluetooth Stack ID.                                         */
         BLE_Demo_Context.BluetoothStackID = Result;
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Bluetooth Stack ID: %d.\n", BLE_Demo_Context.BluetoothStackID);

         if(!qapi_BLE_HCI_Version_Supported(BLE_Demo_Context.BluetoothStackID, &HCIVersion))
            QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Device Chipset: %s.\n", (HCIVersion <= NUM_SUPPORTED_HCI_VERSIONS)?HCIVersionStrings[HCIVersion]:HCIVersionStrings[NUM_SUPPORTED_HCI_VERSIONS]);

         /* Read the LE Buffer Size.                                    */
         Result = qapi_BLE_HCI_LE_Read_Buffer_Size(BLE_Demo_Context.BluetoothStackID, &Status, &HC_LE_Data_Packet_Length, &TempData);

         /* If HCI_LE_Read_Buffer_Size returned an error OR it returned */
         /* ZERO for the LE Data Packet Buffers we need to call         */
         /* HCI_Read_Buffer_Size to get the Data Packet Buffers (which  */
         /* must be shared between Classic and LE Bluetooth).           */
         if((Result) || (Status != 0) || (!HC_LE_Data_Packet_Length) || (!TempData))
         {
            if((!qapi_BLE_HCI_Read_Buffer_Size(BLE_Demo_Context.BluetoothStackID, &Status, &HC_LE_Data_Packet_Length, &HC_SCO_Data_Packet_Length, &HC_Total_Num_LE_Data_Packets, &HC_Total_Num_SCO_Data_Packets)) && (!Status))
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

         if(!Result)
            QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Number ACL Buffers: %d, ACL Buffer Size: %d\n", NumberACLPackets, MaxACLPacketSize);
         else
            QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Unable to determine ACL Buffer Size.\n");

         RegisterForHCIEvents();
         RegisterForACLData();

         GenerateData2Send(MAXIMUM_TEST_BUFFER, TestBuffer);

         /* Let's output the Bluetooth Device Address so that the user  */
         /* knows what the Device Address is.                           */
         if(!qapi_BLE_GAP_Query_Local_BD_ADDR(BLE_Demo_Context.BluetoothStackID, &(BLE_Demo_Context.PublicAddress)))
         {
            BD_ADDRToStr(BLE_Demo_Context.PublicAddress, AddressString);

            QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "BD_ADDR: %s\n", AddressString);
         }

         /* Flag that no connection is currently active.                */
         QAPI_BLE_ASSIGN_BD_ADDR(BLE_Demo_Context.ConnectionBD_ADDR, 0, 0, 0, 0, 0, 0);
         QAPI_BLE_ASSIGN_BD_ADDR(BLE_Demo_Context.CurrentRemoteBD_ADDR, 0, 0, 0, 0, 0, 0);
         BLE_Demo_Context.LocalDeviceIsMaster = FALSE;

         /* Generate some random values for IR and ER (normally would be*/
         /* in flash).                                                  */
         GenerateRandomKeys();

         /* Regenerate IRK and DHK from the constant Identity Root Key. */
         qapi_BLE_GAP_LE_Diversify_Function(BLE_Demo_Context.BluetoothStackID, (qapi_BLE_Encryption_Key_t *)(&(BLE_Demo_Context.IR)), 1, 0, &(BLE_Demo_Context.IRK));
         qapi_BLE_GAP_LE_Diversify_Function(BLE_Demo_Context.BluetoothStackID, (qapi_BLE_Encryption_Key_t *)(&(BLE_Demo_Context.IR)), 3, 0, &(BLE_Demo_Context.DHK));

         /* Flag that we have no Key Information in the Key List.       */
         BLE_Demo_Context.DeviceInfoList = NULL;

         /* Initialize the GATT Service.                                */
         if((Result = qapi_BLE_GATT_Initialize(BLE_Demo_Context.BluetoothStackID, (QAPI_BLE_GATT_INITIALIZATION_FLAGS_SUPPORT_LE | QAPI_BLE_GATT_INITIALIZATION_FLAGS_DISABLE_SERVICE_CHANGED_CHARACTERISTIC), GATT_Connection_Event_Callback, 0)) == 0)
         {
            /* Initialize the GAPS Service.                             */
            Result = qapi_BLE_GAPS_Initialize_Service(BLE_Demo_Context.BluetoothStackID, &ServiceID);
            if(Result > 0)
            {
               /* Save the Instance ID of the GAP Service.              */
               BLE_Demo_Context.GAPSInstanceID = (unsigned int)Result;

               /* Set the GAP Device Name and Device Appearance.        */
               qapi_BLE_GAPS_Set_Device_Name(BLE_Demo_Context.BluetoothStackID, BLE_Demo_Context.GAPSInstanceID, DEVICE_FRIENDLY_NAME);
               qapi_BLE_GAPS_Set_Device_Appearance(BLE_Demo_Context.BluetoothStackID, BLE_Demo_Context.GAPSInstanceID, QAPI_BLE_GAP_DEVICE_APPEARANCE_VALUE_GENERIC_COMPUTER);

               /* Let's enable address resolution in the controller.       */
               /* * NOTE * This function MUST be called to enable the use  */
               /*          of the resolving list in the controller.        */
               Result = qapi_BLE_GAP_LE_Set_Address_Resolution_Enable(BLE_Demo_Context.BluetoothStackID, TRUE);
               if(!Result)
               {
                  /* Enable the GAPS Central Device Resolution.         */
                  /* * NOTE * This MUST be enabled since the local      */
                  /*          device supports privacy with address      */
                  /*          resolution.  A remote peripheral MUST read*/
                  /*          the Central Address Resolution            */
                  /*          Characteristic to determine if the local  */
                  /*          device supports privacy with address      */
                  /*          resolution before using direct            */
                  /*          advertising, where the initiator address  */
                  /*          is a resolvable private address (RPA).    */
                  qapi_BLE_GAPS_Set_Central_Address_Resolution(BLE_Demo_Context.BluetoothStackID, BLE_Demo_Context.GAPSInstanceID, QAPI_BLE_GAR_ENABLED_E);

                  /* Let's set the Resolvable Private Address (RPA) Timeout*/
                  /* to 60 seconds.                                        */
                  /* * NOTE * An new RPA will be generated and used for a  */
                  /*          remote device when the timeout occurs.       */
                  Result = qapi_BLE_GAP_LE_Set_Resolvable_Private_Address_Timeout(BLE_Demo_Context.BluetoothStackID, 60);
                  if(Result)
                     QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Error - qapi_BLE_GAP_LE_Set_Resolvable_Private_Address_Timeout() returned %d.\n", Result);
               }
               else
                  QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Error - qapi_BLE_GAP_LE_Set_Address_Resolution_Enable() returned %d.\n", Result);

               /* Return success to the caller.                         */
               ret_val        = 0;
            }
            else
            {
               /* The Stack was NOT initialized successfully, inform the*/
               /* user and set the return value of the initialization   */
               /* function to an error.                                 */
               DisplayFunctionError("qapi_BLE_GAPS_Initialize_Service", Result);

               /* Cleanup GATT Module.                                  */
               qapi_BLE_GATT_Cleanup(BLE_Demo_Context.BluetoothStackID);

               BLE_Demo_Context.BluetoothStackID = 0;

               ret_val          = -1;
            }
         }
         else
         {
            /* The Stack was NOT initialized successfully, inform the   */
            /* user and set the return value of the initialization      */
            /* function to an error.                                    */
            DisplayFunctionError("GATT_Initialize", Result);

            BLE_Demo_Context.BluetoothStackID = 0;

            ret_val                           = -1;
         }
      }
      else
      {
         /* The Stack was NOT initialized successfully, inform the user */
         /* and set the return value of the initialization function to  */
         /* an error.                                                   */
         DisplayFunctionError("qapi_BLE_BSC_Initialize", Result);

         BLE_Demo_Context.BluetoothStackID = 0;

         ret_val                           = -1;
      }
   }
   else
   {
      /* Stack is already opened.                                       */
      ret_val = 0;
   }

   return(ret_val);
}

   /* The following function is responsible for closing the SS1         */
   /* Bluetooth Protocol Stack.  This function requires that the        */
   /* Bluetooth Protocol stack previously have been initialized via the */
   /* OpenStack() function.  This function returns zero on successful   */
   /* execution and a negative value on all errors.                     */
static int CloseStack(void)
{
   int ret_val = 0;

   /* First check to see if the Stack has been opened.                  */
   if(BLE_Demo_Context.BluetoothStackID)
   {
      /* Clear test related variables.                                  */
      CurrentTest                       = CURRENT_TEST_NONE;
      NumberACLPackets                  = 0;
      NumberOutstandingACLPackets       = 0;
      MaxACLPacketSize                  = 0;
      StartTime                         = 0;
      EndTime                           = 0;
      NumberBytes                       = 0;
      PacketLength                      = 0;
      ConnectionHandle                  = QAPI_BLE_HCI_CONNECTION_HANDLE_INVALID_VALUE;

      /* Cleanup GAP Service Module.                                    */
      if(BLE_Demo_Context.GAPSInstanceID)
      {
         qapi_BLE_GAPS_Cleanup_Service(BLE_Demo_Context.BluetoothStackID, BLE_Demo_Context.GAPSInstanceID);

         BLE_Demo_Context.GAPSInstanceID = 0;
      }

      /* Cleanup GATT Module.                                           */
      qapi_BLE_GATT_Cleanup(BLE_Demo_Context.BluetoothStackID);

      /* Simply close the Stack                                         */
      qapi_BLE_BSC_Shutdown(BLE_Demo_Context.BluetoothStackID);

      QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Stack Shutdown.\n");

      /* Free the Key List.                                             */
      FreeDeviceInfoList(&(BLE_Demo_Context.DeviceInfoList));
      BLE_Demo_Context.DeviceInfoList   = NULL;

      /* Flag that the Stack is no longer initialized.                  */
      BLE_Demo_Context.BluetoothStackID = 0;

      /* Flag success to the caller.                                    */
      ret_val                           = 0;
   }
   else
   {
      /* A valid Stack ID does not exist, inform to user.               */
      ret_val = -1;
   }

   return(ret_val);
}

/******************************************************************************/
/**  RegisterForHCIEvents(void)                                              **/
/******************************************************************************/
/**  Register for HCI Events.                                                **/
/******************************************************************************/
static int RegisterForHCIEvents(void)
{
   int Result;

   if(BLE_Demo_Context.BluetoothStackID)
   {
      Result = qapi_BLE_HCI_Register_Event_Callback(BLE_Demo_Context.BluetoothStackID, EventCallback, 1);

      if(Result)
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "HCI Event Registration Successful.\n");
      else
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "HCI Event Registration Failed.\n");
   }
   else
      QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Stack ID Invalid.\n");

   return(0);
}

/******************************************************************************/
/**  RegisterForACLData(void)                                                **/
/******************************************************************************/
/**  Register for ACL Data.                                                  **/
/******************************************************************************/
static int RegisterForACLData(void)
{
   int Result;

   if(BLE_Demo_Context.BluetoothStackID)
   {
      Result = qapi_BLE_HCI_Register_ACL_Data_Callback(BLE_Demo_Context.BluetoothStackID, ACLDataCallback, 1);

      if(Result)
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "ACL Data Callback Registration Successful.\n");
      else
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "ACL Data Callback Registration Failed.\n");
   }
   else
      QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Stack ID Invalid.\n");

   return(0);
}

/******************************************************************************/
/**  SendACLData(char *)                                                     **/
/******************************************************************************/
/**  Send ACL Data.                                                          **/
/******************************************************************************/
static int SendACLData(unsigned int Length, unsigned char *Data)
{
   int           Result;
   int           ret_val = 0;
   unsigned long Pass;

   if(BLE_Demo_Context.BluetoothStackID)
   {
      if((Data) && (Length <= MAXIMUM_TEST_BUFFER))
      {
         if(CurrentTest == CURRENT_TEST_TX_ACL)
            Pass = 0xFFFFFFFF;
         else
            Pass = 1;

         if(!qapi_BLE_BSC_LockBluetoothStack(BLE_Demo_Context.BluetoothStackID))
         {
            while((Pass--) && (!ret_val))
            {
               if(NumberOutstandingACLPackets < NumberACLPackets)
               {
                  Result = qapi_BLE_HCI_Send_ACL_Data(BLE_Demo_Context.BluetoothStackID,
                                             ConnectionHandle,
                                             QAPI_BLE_HCI_ACL_FLAGS_PACKET_BOUNDARY_FIRST_PACKET_NON_FLUSHABLE,
                                             (uint16_t)Length,
                                             Data);

                  if(!Result)
                  {
                     NumberOutstandingACLPackets++;

                     NumberBytes += Length;
                     ret_val      = 0;
                  }
                  else
                  {
                     QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   Error - Function Returned %d.\n", Result);

                     ret_val = -4;
                  }
               }
               else
                  ret_val = -3;
            }

            qapi_BLE_BSC_UnLockBluetoothStack(BLE_Demo_Context.BluetoothStackID);
         }
      }
      else
         ret_val = -2;
   }
   else
   {
      QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Stack ID Invalid.\n");

      ret_val = -1;
   }

   return(ret_val);
}

static QCLI_Command_Status_t StartTXTest(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   uint16_t              _ConnectionHandle;
   qapi_BLE_BD_ADDR_t    BD_ADDR;
   QCLI_Command_Status_t ret_val;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BLE_Demo_Context.BluetoothStackID)
   {
      if((Parameter_Count > 0) && (Parameter_List) && (strlen((char *)(Parameter_List[0].String_Value)) == (sizeof(qapi_BLE_BD_ADDR_t) * 2)))
      {
         /* Convert the parameter to a Bluetooth Device Address.        */
         StrToBD_ADDR((char *)(Parameter_List[0].String_Value), &BD_ADDR);

         PacketLength = 0;

         if((Parameter_Count > 1) && (Parameter_List[1].Integer_Is_Valid) && (Parameter_List[1].Integer_Value >= 0) && (Parameter_List[1].Integer_Value <= MaxACLPacketSize) && (Parameter_List[1].Integer_Value <= MAXIMUM_TEST_BUFFER))
            PacketLength = (uint32_t)Parameter_List[1].Integer_Value;
         else
         {
            if(Parameter_Count == 1)
               PacketLength = (uint32_t)27;
         }

         if((PacketLength) && (PacketLength <= MaxACLPacketSize))
         {
            if(CurrentTest == CURRENT_TEST_NONE)
            {
               if(!qapi_BLE_GAP_LE_Query_Connection_Handle(BLE_Demo_Context.BluetoothStackID, BD_ADDR, &_ConnectionHandle))
               {
                  CurrentTest                 = CURRENT_TEST_TX_ACL;
                  StartTime                   = qurt_timer_get_ticks();
                  NumberBytes                 = 0;
                  NumberOutstandingACLPackets = 0;
                  ConnectionHandle            = _ConnectionHandle;

                  QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Starting ACL TX Test with packet size: %u.\n", PacketLength);

                  SendACLData(PacketLength, TestBuffer);
               }
               else
                  QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Device is not currently connected.\n");
            }
            else
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Test already in progress.\n");

            /* Flag success to the caller.                              */
            ret_val = QCLI_STATUS_SUCCESS_E;
         }
         else
         {
            QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Data Packet Size must be between 1 and  %u.\n", MaxACLPacketSize);

            /* One or more of the necessary parameters is/are invalid.  */
            ret_val = QCLI_STATUS_USAGE_E;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         ret_val = QCLI_STATUS_USAGE_E;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = QCLI_STATUS_ERROR_E;
   }

   return(ret_val);
}

static QCLI_Command_Status_t StartRXTest(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   uint16_t              _ConnectionHandle;
   qapi_BLE_BD_ADDR_t    BD_ADDR;
   QCLI_Command_Status_t ret_val;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BLE_Demo_Context.BluetoothStackID)
   {
      if(CurrentTest == CURRENT_TEST_NONE)
      {
         _ConnectionHandle = QAPI_BLE_HCI_CONNECTION_HANDLE_INVALID_VALUE;

         /* Note we will treat the RX Test BD_ADDR as optional - we will*/
         /* not display statistics though.                              */
         if((Parameter_Count > 0) && (Parameter_List) && (strlen((char *)(Parameter_List[0].String_Value)) == (sizeof(qapi_BLE_BD_ADDR_t) * 2)))
         {
            /* Convert the parameter to a Bluetooth Device Address.        */
            StrToBD_ADDR((char *)(Parameter_List[0].String_Value), &BD_ADDR);

            qapi_BLE_GAP_LE_Query_Connection_Handle(BLE_Demo_Context.BluetoothStackID, BD_ADDR, &_ConnectionHandle);
         }

         StartTime        = 0;
         NumberBytes      = 0;
         CurrentTest      = CURRENT_TEST_RX_ACL;
         ConnectionHandle = _ConnectionHandle;

         ret_val          = QCLI_STATUS_SUCCESS_E;

         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Waiting to Receive ACL Data.\n");
      }
      else
      {
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Test already in progress.\n");
         ret_val = QCLI_STATUS_ERROR_E;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = QCLI_STATUS_ERROR_E;
   }

   return(ret_val);
}

static QCLI_Command_Status_t StopTXRXTest(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   uint32_t              ElapsedTime;
   uint32_t              Bandwidth;
   QCLI_Command_Status_t ret_val;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BLE_Demo_Context.BluetoothStackID)
   {
      if(CurrentTest != CURRENT_TEST_NONE)
      {
         /* Do not display statistics if an RX Test was specified (with */
         /* no BD_ADDR specified).                                      */
         if(ConnectionHandle != QAPI_BLE_HCI_CONNECTION_HANDLE_INVALID_VALUE)
         {
            EndTime      = qurt_timer_get_ticks();

            ElapsedTime  = qurt_timer_convert_ticks_to_time(EndTime - StartTime, QURT_TIME_MSEC) / 1000;

            if(ElapsedTime)
            {
               Bandwidth    = NumberBytes/ElapsedTime;

               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Test Time    : %u\n", ElapsedTime);
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Number Bytes : %u\n", NumberBytes);
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Bytes/Second : %u\n", Bandwidth);
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "KBytes/Second: %u\n", Bandwidth/1024);
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "KBits/Second : %u\n", (Bandwidth*8)/1024);
            }
         }

         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Test Complete.\n");

         if(CurrentTest == CURRENT_TEST_PERIODIC)
         {
            /* Stop the timer.                                          */
            qapi_Timer_Stop(BLE_Demo_Context.Timer);

            /* Clean up the timer.                                      */
            qapi_Timer_Undef(BLE_Demo_Context.Timer);

            BLE_Demo_Context.TransmitPeriod = 0;

            QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Timer stopped.\n");
         }

         ConnectionHandle            = QAPI_BLE_HCI_CONNECTION_HANDLE_INVALID_VALUE;
         CurrentTest                 = CURRENT_TEST_NONE;
         NumberOutstandingACLPackets = 0;
      }
      else
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "No Test in progress.\n");

      /* Flag success to the caller.                                    */
      ret_val = QCLI_STATUS_SUCCESS_E;
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = QCLI_STATUS_ERROR_E;
   }

   return(ret_val);
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
   if(BLE_Demo_Context.BluetoothStackID)
   {
      /* Attempt to set the attached device to be pairable.             */
      Result = qapi_BLE_GAP_LE_Set_Pairability_Mode(BLE_Demo_Context.BluetoothStackID, QAPI_BLE_LPM_PAIRABLE_MODE_ENABLE_EXTENDED_EVENTS_E);

      /* Next, check the return value of the GAP Set Pairability mode   */
      /* command for successful execution.                              */
      if(!Result)
      {
         /* The device has been set to pairable mode, now register an   */
         /* Authentication Callback to handle the Authentication events */
         /* if required.                                                */
         Result = qapi_BLE_GAP_LE_Register_Remote_Authentication(BLE_Demo_Context.BluetoothStackID, GAP_LE_Event_Callback, (unsigned long)0);

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

   /* The following function is a utility function that is used to dump */
   /* the Appearance to String Mapping Table.                           */
static void DumpAppearanceMappings(void)
{
   unsigned int Index;

   for(Index=0;Index<NUMBER_OF_APPEARANCE_MAPPINGS;++Index)
      QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   %u = %s.\n", Index, AppearanceMappings[Index].String);
}

   /* The following function is used to map a Appearance Value to it's  */
   /* string representation.  This function returns TRUE on success or  */
   /* FALSE otherwise.                                                  */
static boolean_t AppearanceToString(uint16_t Appearance, char **String)
{
   boolean_t    ret_val;
   unsigned int Index;

   /* Verify that the input parameters are semi-valid.                  */
   if(String)
   {
      for(Index=0,ret_val=FALSE;Index<NUMBER_OF_APPEARANCE_MAPPINGS;++Index)
      {
         if(AppearanceMappings[Index].Appearance == Appearance)
         {
            *String = AppearanceMappings[Index].String;
            ret_val = TRUE;
            break;
         }
      }
   }
   else
      ret_val = FALSE;

   return(ret_val);
}

   /* The following function is used to map an Index into the Appearance*/
   /* Mapping table to it's Appearance Value.  This function returns    */
   /* TRUE on success or FALSE otherwise.                               */
static boolean_t AppearanceIndexToAppearance(unsigned int Index, uint16_t *Appearance)
{
   boolean_t ret_val;

   if((Index < NUMBER_OF_APPEARANCE_MAPPINGS) && (Appearance))
   {
      *Appearance = AppearanceMappings[Index].Appearance;
      ret_val     = TRUE;
   }
   else
      ret_val = FALSE;

   return(ret_val);
}

   /* The following function is a utility function that provides a      */
   /* mechanism of populating discovered GAP Service Handles.           */
static void GAPSPopulateHandles(GAPS_Client_Info_t *ClientInfo, qapi_BLE_GATT_Service_Discovery_Indication_Data_t *ServiceInfo)
{
   unsigned int                                Index1;
   qapi_BLE_GATT_Characteristic_Information_t *CurrentCharacteristic;

   /* Verify that the input parameters are semi-valid.                  */
   if((ClientInfo) && (ServiceInfo) && (ServiceInfo->ServiceInformation.UUID.UUID_Type == QAPI_BLE_GU_UUID_16_E) && (QAPI_BLE_GAP_COMPARE_GAP_SERVICE_UUID_TO_UUID_16(ServiceInfo->ServiceInformation.UUID.UUID.UUID_16)))
   {
      /* Loop through all characteristics discovered in the service     */
      /* and populate the correct entry.                                */
      CurrentCharacteristic = ServiceInfo->CharacteristicInformationList;
      if(CurrentCharacteristic)
      {
         for(Index1=0;Index1<ServiceInfo->NumberOfCharacteristics;Index1++,CurrentCharacteristic++)
         {
            /* All GAP Service UUIDs are defined to be 16 bit UUIDs.    */
            if(CurrentCharacteristic->Characteristic_UUID.UUID_Type == QAPI_BLE_GU_UUID_16_E)
            {
               /* Determine which characteristic this is.               */
               if(!QAPI_BLE_GAP_COMPARE_GAP_DEVICE_NAME_UUID_TO_UUID_16(CurrentCharacteristic->Characteristic_UUID.UUID.UUID_16))
               {
                  if(!QAPI_BLE_GAP_COMPARE_GAP_DEVICE_APPEARANCE_UUID_TO_UUID_16(CurrentCharacteristic->Characteristic_UUID.UUID.UUID_16))
                     continue;
                  else
                  {
                     ClientInfo->DeviceAppearanceHandle = CurrentCharacteristic->Characteristic_Handle;
                     continue;
                  }
               }
               else
               {
                  ClientInfo->DeviceNameHandle = CurrentCharacteristic->Characteristic_Handle;
                  continue;
               }
            }
         }
      }
   }
}

   /* The following function is responsible for starting a scan.        */
static int StartScan(uint32_t BluetoothStackID, qapi_BLE_GAP_LE_Filter_Policy_t FilterPolicy)
{
   int Result;

   /* First, determine if the input parameters appear to be semi-valid. */
   if(BLE_Demo_Context.BluetoothStackID)
   {
      /* Check to see if we need to configure the default Scan          */
      /* Parameters.                                                    */
      if(!(BLE_Demo_Context.BLEParameters.Flags & BLE_PARAMETERS_FLAGS_SCAN_PARAMETERS_VALID))
      {
         /* Configure the default Scan Window and Scan Interval.        */
         BLE_Demo_Context.BLEParameters.ScanParameters.ScanWindow   = 50;
         BLE_Demo_Context.BLEParameters.ScanParameters.ScanInterval = 100;

         /* Flag that the scan parameters are valid so that we do not   */
         /* re-configure the defaults un-necessarily.                   */
         BLE_Demo_Context.BLEParameters.Flags |= BLE_PARAMETERS_FLAGS_SCAN_PARAMETERS_VALID;
      }

      /* Not currently scanning, go ahead and attempt to perform the    */
      /* scan.                                                          */
      Result = qapi_BLE_GAP_LE_Perform_Scan(BluetoothStackID, QAPI_BLE_ST_ACTIVE_E, BLE_Demo_Context.BLEParameters.ScanParameters.ScanInterval, BLE_Demo_Context.BLEParameters.ScanParameters.ScanWindow, QAPI_BLE_LAT_PUBLIC_E, FilterPolicy, TRUE, GAP_LE_Event_Callback, 0);

      if(!Result)
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Scan started successfully. Scan Window: %u, Scan Interval: %u.\n", (unsigned int)BLE_Demo_Context.BLEParameters.ScanParameters.ScanWindow, (unsigned int)BLE_Demo_Context.BLEParameters.ScanParameters.ScanInterval);
      else
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Unable to perform scan: %d\n", Result);
   }
   else
      Result = -1;

   return(Result);
}

   /* The following function is responsible for stopping on on-going    */
   /* scan.                                                             */
static int StopScan(uint32_t BluetoothStackID)
{
   int Result;

   /* First, determine if the input parameters appear to be semi-valid. */
   if(BluetoothStackID)
   {
      Result = qapi_BLE_GAP_LE_Cancel_Scan(BluetoothStackID);
      if(!Result)
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Scan stopped successfully.\n");
      else
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Unable to stop scan: %d\n", Result);
   }
   else
      Result = -1;

   return(Result);
}

   /* The following function is responsible for creating an LE          */
   /* connection to the specified Remote Device.                        */
   /* * NOTE * If UseWhiteList is TRUE, then the BD_ADDR may be excluded*/
   /*          (NULL), and the address type will be ignored.  Otherwise,*/
   /*          they MUST be specified.  A remote device address and     */
   /*          address type MUST have been added to the White List in   */
   /*          the controller to use this functionality.                */
   /* * NOTE * If UseWhiteList is FALSE, then this function will perform*/
   /*          no checks on the remote addresss or address type.        */
   /* * NOTE * If Address Resolution has been enabled in the controller */
   /*          and the remote device's identity information has been    */
   /*          added to the Resolving List in the controller, then the  */
   /*          local controller will generate a resolvable private      */
   /*          address (RPA) to send in the connection request.         */
   /* * NOTE * If the user wishes to use the White List and the         */
   /*          Resolving List in the controller, then the remote device */
   /*          address and address type specified for the added White   */
   /*          List entry MUST correspond to the identity address and   */
   /*          identity address type for the remote device.             */
   /* * NOTE * If the user wishes to ONLY use the Resolving List in the */
   /*          controller (Not White List), then the remote device      */
   /*          address and address type specified to this function MUST */
   /*          correspond to the identity address and identity address  */
   /*          type for the remote device.                              */
static int ConnectLEDevice(uint32_t BluetoothStackID, boolean_t UseWhiteList, qapi_BLE_BD_ADDR_t *BD_ADDR, qapi_BLE_GAP_LE_Address_Type_t AddressType)
{
   int Result;

   /* First, determine if the input parameters appear to be semi-valid. */
   if(BLE_Demo_Context.BluetoothStackID)
   {
      /* Check to see if we need to configure the default Scan          */
      /* Parameters.                                                    */
      if(!(BLE_Demo_Context.BLEParameters.Flags & BLE_PARAMETERS_FLAGS_SCAN_PARAMETERS_VALID))
      {
         /* Configure the default Scan Window and Scan Interval.        */
         BLE_Demo_Context.BLEParameters.ScanParameters.ScanWindow   = 50;
         BLE_Demo_Context.BLEParameters.ScanParameters.ScanInterval = 100;

         /* Flag that the scan parameters are valid so that we do not   */
         /* re-configure the defaults un-necessarily.                   */
         BLE_Demo_Context.BLEParameters.Flags |= BLE_PARAMETERS_FLAGS_SCAN_PARAMETERS_VALID;
      }

      /* Check to see if we need to configure the default Connection    */
      /* Parameters.                                                    */
      if(!(BLE_Demo_Context.BLEParameters.Flags & BLE_PARAMETERS_FLAGS_CONNECTION_PARAMETERS_VALID))
      {
         /* Initialize the default connection parameters.               */
         BLE_Demo_Context.BLEParameters.ConnectionParameters.Connection_Interval_Min    = 50;
         BLE_Demo_Context.BLEParameters.ConnectionParameters.Connection_Interval_Max    = 200;
         BLE_Demo_Context.BLEParameters.ConnectionParameters.Minimum_Connection_Length  = 0;
         BLE_Demo_Context.BLEParameters.ConnectionParameters.Maximum_Connection_Length  = 10000;
         BLE_Demo_Context.BLEParameters.ConnectionParameters.Slave_Latency              = 0;
         BLE_Demo_Context.BLEParameters.ConnectionParameters.Supervision_Timeout        = 20000;

         /* Flag that the connection parameters are valid so that we do */
         /* not re-configure the defaults un-necessarily.               */
         BLE_Demo_Context.BLEParameters.Flags |= BLE_PARAMETERS_FLAGS_CONNECTION_PARAMETERS_VALID;
      }

      /* Everything appears correct, go ahead and attempt to make the   */
      /* connection.                                                    */
      /* * NOTE * Our local address type will ALWAYS be resolvable      */
      /*          fallback public, in case the Resolving List in the    */
      /*          controller is used for connecting.  It is worth noting*/
      /*          that this demo uses its local/public address as the   */
      /*          public identity address for simplicity.               */
      Result = qapi_BLE_GAP_LE_Create_Connection(BluetoothStackID, BLE_Demo_Context.BLEParameters.ScanParameters.ScanInterval, BLE_Demo_Context.BLEParameters.ScanParameters.ScanWindow, (UseWhiteList ? QAPI_BLE_FP_WHITE_LIST_E : QAPI_BLE_FP_NO_FILTER_E), AddressType, BD_ADDR, QAPI_BLE_LAT_RESOLVABLE_FALLBACK_PUBLIC_E, &(BLE_Demo_Context.BLEParameters.ConnectionParameters), GAP_LE_Event_Callback, 0);
      if(!Result)
      {
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Connection Request successful.\n");
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Scan Parameters:       Window %u, Interval %u.\n", (unsigned int)BLE_Demo_Context.BLEParameters.ScanParameters.ScanWindow,
                                                                                     (unsigned int)BLE_Demo_Context.BLEParameters.ScanParameters.ScanInterval);
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Connection Parameters: Interval Range %u - %u, Slave Latency %u.\n", (unsigned int)BLE_Demo_Context.BLEParameters.ConnectionParameters.Connection_Interval_Min,
                                                                                                       (unsigned int)BLE_Demo_Context.BLEParameters.ConnectionParameters.Connection_Interval_Max,
                                                                                                       (unsigned int)BLE_Demo_Context.BLEParameters.ConnectionParameters.Slave_Latency);
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Using White List:      %s.\n", (UseWhiteList ? "Yes" : "No"));
      }
      else
      {
         /* Unable to create connection.                                */
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Unable to create connection: %d.\n", Result);
      }
   }
   else
      Result = -1;

   return(Result);
}

   /* The following function is provided to allow a mechanism to        */
   /* disconnect a currently connected device.                          */
static int DisconnectLEDevice(uint32_t BluetoothStackID, qapi_BLE_BD_ADDR_t BD_ADDR)
{
   int Result;

   /* First, determine if the input parameters appear to be semi-valid. */
   if((BluetoothStackID) && (!QAPI_BLE_COMPARE_NULL_BD_ADDR(BD_ADDR)))
   {
      if(!QAPI_BLE_COMPARE_NULL_BD_ADDR(BLE_Demo_Context.ConnectionBD_ADDR))
      {
         Result = qapi_BLE_GAP_LE_Disconnect(BluetoothStackID, BD_ADDR);
         if(!Result)
            QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Disconnect Request successful.\n");
         else
            QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Unable to disconnect device: %d.\n", Result);
      }
      else
      {
         /* Device not connected.                                       */
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Device is not connected.\n");

         Result = 0;
      }
   }
   else
      Result = -1;

   return(Result);
}

   /* The following function is used to generate a pattern of data to   */
   /* send to the remote device.                                        */
static void GenerateData2Send(unsigned int Length,  uint8_t *Data)
{
   unsigned int         Index;
   static unsigned char Cntr = '0';

   /* Make sure the input parameters are valid.                         */
   if((Length) && (Data))
   {
      for(Index = 0; Index < Length; Index++)
      {
         Data[Index] = Cntr++;

         if((Cntr > '9') && (Cntr < 'A'))
            Cntr = 'A';
         else
         {
            if(Cntr > 'F')
               Cntr = '0';
         }
      }
   }
}

   /* The following callback is the period timer callback that is       */
   /* started for periodically transmitting data.                       */
static void Period_Tx_Callback(uint32_t data)
{
   qapi_Status_t         Result;
   unsigned long         Temp;
   static unsigned long  DataIndex = 0;
   qapi_TIMER_set_attr_t Set_Timer_Attr;

   /* Verify that the stack is opened.                                  */
   if(BLE_Demo_Context.BluetoothStackID != 0)
   {
      /* Verify that a LE Connection Oriented Channel is open for data  */
      /* transfer.                                                      */
      if(CurrentTest == CURRENT_TEST_PERIODIC)
      {
         Temp = MAXIMUM_TEST_BUFFER - DataIndex;

         if(PacketLength <= Temp)
            Temp = PacketLength;

         /* Send the data.                                              */
         SendACLData(Temp, &(TestBuffer[DataIndex]));

         DataIndex += Temp;

         if(DataIndex >= MAXIMUM_TEST_BUFFER)
            DataIndex = 0;
      }
   }

   /* Restart the timer if necessary.                                   */
   if(CurrentTest == CURRENT_TEST_PERIODIC)
   {
      /* Timer restart necessary, so configure according to the saved   */
      /* parameters.                                                    */
      Set_Timer_Attr.time                   = (uint64_t)BLE_Demo_Context.TransmitPeriod;
      Set_Timer_Attr.reload                 = true;
      Set_Timer_Attr.max_deferrable_timeout = (uint64_t)BLE_Demo_Context.TransmitPeriod;
      Set_Timer_Attr.unit                   = QAPI_TIMER_UNIT_MSEC;

      Result = qapi_Timer_Set(BLE_Demo_Context.Timer, &Set_Timer_Attr);
      if(Result != QAPI_OK)
         Display_Function_Error(BLE_Demo_Context.QCLI_Handle, "qapi_Timer_Set", Result);
   }
   else
      DataIndex = 0;
}

   /* The following function provides a mechanism to configure a        */
   /* Pairing Capabilities structure with the application's pairing     */
   /* parameters.                                                       */
static void ConfigureCapabilities(qapi_BLE_GAP_LE_Extended_Pairing_Capabilities_t *Capabilities)
{
   /* Make sure the Capabilities pointer is semi-valid.                 */
   if(Capabilities)
   {
      /* Initialize the capabilities.                                   */
      memset(Capabilities, 0, QAPI_BLE_GAP_LE_EXTENDED_PAIRING_CAPABILITIES_SIZE);

      /* Configure the Pairing Capabilities structure.                  */
      Capabilities->Bonding_Type                    = QAPI_BLE_LBT_BONDING_E;
      Capabilities->IO_Capability                   = BLE_Demo_Context.LE_Parameters.IOCapability;
      Capabilities->Flags                           = 0;

      if(BLE_Demo_Context.LE_Parameters.MITMProtection)
         Capabilities->Flags |= QAPI_BLE_GAP_LE_EXTENDED_PAIRING_CAPABILITIES_FLAGS_MITM_REQUESTED;

      if(BLE_Demo_Context.LE_Parameters.SecureConnections)
         Capabilities->Flags |= QAPI_BLE_GAP_LE_EXTENDED_PAIRING_CAPABILITIES_FLAGS_SECURE_CONNECTIONS;

      if(BLE_Demo_Context.LE_Parameters.OOBDataPresent)
         Capabilities->Flags |= QAPI_BLE_GAP_LE_EXTENDED_PAIRING_CAPABILITIES_FLAGS_OOB_DATA_PRESENT;

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

      /* This application only demonstrates using Long Term Key's (LTK) */
      /* for encryption of a LE Link, however we could request and send */
      /* all possible keys here if we wanted to.                        */
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

   /* The following function provides a mechanism for sending a pairing */
   /* request to a device that is connected on an LE Link.              */
static int SendPairingRequest(qapi_BLE_BD_ADDR_t BD_ADDR, boolean_t ConnectionMaster)
{
   int                                             ret_val;
   BoardStr_t                                      BoardStr;
   qapi_BLE_GAP_LE_Extended_Pairing_Capabilities_t ExtendedCapabilities;

   /* Make sure a Bluetooth Stack is open.                              */
   if(BLE_Demo_Context.BluetoothStackID)
   {
      /* Make sure the BD_ADDR is valid.                                */
      if(!QAPI_BLE_COMPARE_NULL_BD_ADDR(BD_ADDR))
      {
         /* Configure the application pairing parameters.               */
         ConfigureCapabilities(&ExtendedCapabilities);

         /* Set the BD_ADDR of the device that we are attempting to pair*/
         /* with.                                                       */
         BLE_Demo_Context.CurrentRemoteBD_ADDR = BD_ADDR;

         BD_ADDRToStr(BD_ADDR, BoardStr);
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Attempting to Pair to %s.\n", BoardStr);

         DisplayPairingInformation(&ExtendedCapabilities);

         /* Attempt to pair to the remote device.                       */
         if(ConnectionMaster)
         {
            /* Start the pairing process.                               */
            if((ret_val = qapi_BLE_GAP_LE_Extended_Pair_Remote_Device(BLE_Demo_Context.BluetoothStackID, BD_ADDR, &ExtendedCapabilities, GAP_LE_Event_Callback, 0)) == QAPI_BLE_BTPS_ERROR_SECURE_CONNECTIONS_NOT_SUPPORTED)
            {
               /* Since Secure Connections isn't supported go ahead and */
               /* disable our request for Secure Connections and        */
               /* re-submit our request.                                */
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Secure Connections not supported, disabling Secure Connections.\r\n");

               ExtendedCapabilities.Flags &= ~QAPI_BLE_GAP_LE_EXTENDED_PAIRING_CAPABILITIES_FLAGS_SECURE_CONNECTIONS;

               /* Try this again.                                       */
               ret_val = qapi_BLE_GAP_LE_Extended_Pair_Remote_Device(BLE_Demo_Context.BluetoothStackID, BD_ADDR, &ExtendedCapabilities, GAP_LE_Event_Callback, 0);
            }

            QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "     GAP_LE_Extended_Pair_Remote_Device returned %d.\r\n", ret_val);
         }
         else
         {
            /* As a slave we can only request that the Master start     */
            /* the pairing process.                                     */
            ret_val = qapi_BLE_GAP_LE_Extended_Request_Security(BLE_Demo_Context.BluetoothStackID, BD_ADDR, &ExtendedCapabilities, GAP_LE_Event_Callback, 0);

            QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "     GAP_LE_Request_Security returned %d.\n", ret_val);
         }
      }
      else
      {
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Invalid Parameters.\n");

         ret_val = -1;
      }
   }
   else
   {
      QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Stack ID Invalid.\n");

      ret_val = -1;
   }

   return(ret_val);
}

   /* The following function provides a mechanism of sending a Slave    */
   /* Pairing Response to a Master's Pairing Request.                   */
static int SlavePairingRequestResponse(qapi_BLE_BD_ADDR_t BD_ADDR)
{
   int                                                   ret_val;
   BoardStr_t                                            BoardStr;
   qapi_BLE_GAP_LE_Authentication_Response_Information_t AuthenticationResponseData;

   /* Make sure a Bluetooth Stack is open.                              */
   if(BLE_Demo_Context.BluetoothStackID)
   {
      BD_ADDRToStr(BD_ADDR, BoardStr);
      QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Sending Pairing Response to %s.\n", BoardStr);

      /* We must be the slave if we have received a Pairing Request     */
      /* thus we will respond with our capabilities.                    */
      AuthenticationResponseData.GAP_LE_Authentication_Type = QAPI_BLE_LAR_PAIRING_CAPABILITIES_E;
      AuthenticationResponseData.Authentication_Data_Length = QAPI_BLE_GAP_LE_EXTENDED_PAIRING_CAPABILITIES_SIZE;

      /* Configure the Application Pairing Parameters.                  */
      ConfigureCapabilities(&(AuthenticationResponseData.Authentication_Data.Extended_Pairing_Capabilities));

      /* Attempt to pair to the remote device.                          */
      if((ret_val = qapi_BLE_GAP_LE_Authentication_Response(BLE_Demo_Context.BluetoothStackID, BD_ADDR, &AuthenticationResponseData)) == QAPI_BLE_BTPS_ERROR_SECURE_CONNECTIONS_NOT_SUPPORTED)
      {
         /* Since Secure Connections isn't supported go ahead and       */
         /* disable our request for Secure Connections and re-submit our*/
         /* request.                                                    */
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Secure Connections not supported, disabling Secure Connections.\r\n");

         AuthenticationResponseData.Authentication_Data.Extended_Pairing_Capabilities.Flags &= ~QAPI_BLE_GAP_LE_EXTENDED_PAIRING_CAPABILITIES_FLAGS_SECURE_CONNECTIONS;

         /* Try this again.                                             */
         ret_val = qapi_BLE_GAP_LE_Authentication_Response(BLE_Demo_Context.BluetoothStackID, BD_ADDR, &AuthenticationResponseData);
      }

      QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "GAP_LE_Authentication_Response returned %d.\r\n", ret_val);
   }
   else
   {
      QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Stack ID Invalid.\n");

      ret_val = -1;
   }

   return(ret_val);
}

   /* The following function is provided to allow a mechanism of        */
   /* responding to a request for Encryption Information to send to a   */
   /* remote device.                                                    */
static int EncryptionInformationRequestResponse(qapi_BLE_BD_ADDR_t BD_ADDR, uint8_t KeySize, qapi_BLE_GAP_LE_Authentication_Response_Information_t *GAP_LE_Authentication_Response_Information)
{
   int      ret_val;
   uint16_t LocalDiv;

   /* Make sure a Bluetooth Stack is open.                              */
   if(BLE_Demo_Context.BluetoothStackID)
   {
      /* Make sure the input parameters are semi-valid.                 */
      if((!QAPI_BLE_COMPARE_NULL_BD_ADDR(BD_ADDR)) && (GAP_LE_Authentication_Response_Information))
      {
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   Calling GAP_LE_Generate_Long_Term_Key.\n");

         /* Generate a new LTK, EDIV and Rand tuple.                    */
         ret_val = qapi_BLE_GAP_LE_Generate_Long_Term_Key(BLE_Demo_Context.BluetoothStackID, (qapi_BLE_Encryption_Key_t *)(&(BLE_Demo_Context.DHK)), (qapi_BLE_Encryption_Key_t *)(&(BLE_Demo_Context.ER)), &(GAP_LE_Authentication_Response_Information->Authentication_Data.Encryption_Information.LTK), &LocalDiv, &(GAP_LE_Authentication_Response_Information->Authentication_Data.Encryption_Information.EDIV), &(GAP_LE_Authentication_Response_Information->Authentication_Data.Encryption_Information.Rand));
         if(!ret_val)
         {
            QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   Encryption Information Request Response.\n");

            /* Response to the request with the LTK, EDIV and Rand      */
            /* values.                                                  */
            GAP_LE_Authentication_Response_Information->GAP_LE_Authentication_Type                                     = QAPI_BLE_LAR_ENCRYPTION_INFORMATION_E;
            GAP_LE_Authentication_Response_Information->Authentication_Data_Length                                     = QAPI_BLE_GAP_LE_ENCRYPTION_INFORMATION_DATA_SIZE;
            GAP_LE_Authentication_Response_Information->Authentication_Data.Encryption_Information.Encryption_Key_Size = KeySize;

            ret_val = qapi_BLE_GAP_LE_Authentication_Response(BLE_Demo_Context.BluetoothStackID, BD_ADDR, GAP_LE_Authentication_Response_Information);
            if(!ret_val)
            {
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   qapi_BLE_GAP_LE_Authentication_Response (larEncryptionInformation) success.\n");
            }
            else
            {
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   Error - SM_Generate_Long_Term_Key returned %d.\n", ret_val);
            }
         }
         else
         {
            QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   Error - SM_Generate_Long_Term_Key returned %d.\n", ret_val);
         }
      }
      else
      {
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Invalid Parameters.\n");

         ret_val = -1;
      }
   }
   else
   {
      QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Stack ID Invalid.\n");

      ret_val = -1;
   }

   return(ret_val);
}

   /* The following function is responsible for initializing the stack. */
   /* This function returns QCLI_STATUS_SUCCESS_E on success or an error*/
   /* enumeration if an error occurs.                                   */
static QCLI_Command_Status_t InitializeBluetooth(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   int                   Result;
   QCLI_Command_Status_t ret_val;

   /* First, check that the stack is not currently initialized.         */
   if(!BLE_Demo_Context.BluetoothStackID)
   {
      /* Initialize the BLE parameters.                                 */
      BLE_Demo_Context.BLEParameters.Flags               = 0;
      BLE_Demo_Context.TestAdvertisingDataLength         = 0;
      BLE_Demo_Context.TransmitPeriod                    = 0;

      /* Initialize the Default Pairing Parameters.                     */
      BLE_Demo_Context.LE_Parameters.ConnectableMode     = QAPI_BLE_LCM_CONNECTABLE_E;
      BLE_Demo_Context.LE_Parameters.DiscoverabilityMode = QAPI_BLE_DM_GENERAL_DISCOVERABLE_MODE_E;
      BLE_Demo_Context.LE_Parameters.IOCapability        = DEFAULT_IO_CAPABILITY;
      BLE_Demo_Context.LE_Parameters.MITMProtection      = DEFAULT_MITM_PROTECTION;
      BLE_Demo_Context.LE_Parameters.SecureConnections   = DEFAULT_SECURE_CONNECTIONS;
      BLE_Demo_Context.LE_Parameters.OOBDataPresent      = FALSE;

      /* Attempt to open the stack.                                     */
      Result = OpenStack();
      if(!Result)
      {
         /* Set the default pairability.                                */
         Result = SetPairable();

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
      QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Bluetooth stack is already initialized.");

      ret_val = QCLI_STATUS_ERROR_E;
   }

   return(ret_val);
}

   /* The following function is responsible for shutting down the stack.*/
   /* This function returns QCLI_STATUS_SUCCESS_E on success or an error*/
   /* enumeration if an error occurs.                                   */
static QCLI_Command_Status_t ShutdownBluetooth(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t ret_val;

   /* First, check that the stack is currently initialized.             */
   if(BLE_Demo_Context.BluetoothStackID)
   {
      /* Close the stack and set the QCLI error code.                   */
      if(!CloseStack())
         ret_val = QCLI_STATUS_SUCCESS_E;
      else
         ret_val = QCLI_STATUS_ERROR_E;
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Bluetooth stack is not initialized.");

      ret_val = QCLI_STATUS_ERROR_E;
   }

   return(ret_val);
}

   /* The following function is responsible for returning the current   */
   /* version of the stack and the controller.  This function returns   */
   /* QCLI_STATUS_SUCCESS_E on success or an error enumeration if an    */
   /* error occurs.                                                     */
static QCLI_Command_Status_t GetVersion(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   int                    Result;
   char                  *HostVersion;
   uint8_t                Status;
   uint8_t                Version;
   uint16_t               Revision;
   uint8_t                LMPVersion;
   uint16_t               ManufacturerName;
   uint16_t               LMPSubversion;
   uint32_t               ProductID;
   uint32_t               BuildVersion;
   QCLI_Command_Status_t  ret_val;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BLE_Demo_Context.BluetoothStackID)
   {
      /* Print the Bluetopia Version first..                            */
      HostVersion = qapi_BLE_BSC_Query_Host_Version();

      QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "        Bluetopia        : %s.\n", HostVersion?HostVersion:"Unknown");

      /* Print the Controller Version.                                  */
      if(((Result = qapi_BLE_HCI_Read_Local_Version_Information(BLE_Demo_Context.BluetoothStackID, &Status, &Version, &Revision, &LMPVersion, &ManufacturerName, &LMPSubversion)) == 0) && (!Status))
      {
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "\n        HCI Version      : 0x%02X.\n", Version);
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "        HCI Revision     : 0x%04X.\n", Revision);
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "        LMP Version      : 0x%02X.\n", LMPVersion);
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "        LMP Sub Version  : 0x%04X.\n", LMPSubversion);
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "        Manufacturer Name: 0x%04X.\n", ManufacturerName);
      }
      else
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "\n        Error retrieving Version Information: %d.\n", (!Result?Status:Result));

      /* Now attempt to query the patch version information.            */
      Result = HCI_VS_GetPatchVersion(BLE_Demo_Context.BluetoothStackID, &ProductID, &BuildVersion);
      if(!Result)
      {
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "\n        Product ID       : 0x%08X.\n", ProductID);
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "        Build Version    : 0x%08X.\n", BuildVersion);
      }
      else
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "\n        Error retrieving Patch Version Information: %d.\n", Result);

      /* Flag success to the caller.                                    */
      ret_val = QCLI_STATUS_SUCCESS_E;
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = QCLI_STATUS_ERROR_E;
   }

   return(ret_val);
}

   /* The following function is responsible for enabling or disabling   */
   /* the BBIF interface for BLE.  This function returns                */
   /* QCLI_STATUS_SUCCESS_E on success or an error enumeration if an    */
   /* error occurs.                                                     */
static QCLI_Command_Status_t EnableBBIF(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   int                   Result;
   QCLI_Command_Status_t ret_val;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BLE_Demo_Context.BluetoothStackID)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((Parameter_Count > 0) && (Parameter_List) && (Parameter_List[0].Integer_Is_Valid))
      {
         /* Now attempt to query the version of the Bluetooth           */
         /* Controller.                                                 */
         Result = HCI_VS_EnableBBIF(BLE_Demo_Context.BluetoothStackID, (boolean_t)(Parameter_List[0].Integer_Value?TRUE:FALSE));
         if(!Result)
            QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "        Success %s BBIF.\n", Parameter_List[0].Integer_Value?"Enabling":"Disabling");
         else
            QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "        Error %s BBIF: %d.\n", Parameter_List[0].Integer_Value?"Enabling":"Disabling", Result);

         /* Flag success to the caller.                                 */
         ret_val = QCLI_STATUS_SUCCESS_E;
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         ret_val = QCLI_STATUS_USAGE_E;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = QCLI_STATUS_ERROR_E;
   }

   return(ret_val);
}

   /* The following function is responsible for setting the radio that  */
   /* is used for BLE.  This function returns QCLI_STATUS_SUCCESS_E on  */
   /* success or an error enumeration if an error occurs.               */
static QCLI_Command_Status_t SetBLERadio(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   int                   Result;
   QCLI_Command_Status_t ret_val;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BLE_Demo_Context.BluetoothStackID)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((Parameter_Count > 0) && (Parameter_List) && (Parameter_List[0].Integer_Is_Valid) && ((Parameter_List[0].Integer_Value >= 1) && (Parameter_List[0].Integer_Value <= 2)))
      {
         /* Now attempt to query the version of the Bluetooth           */
         /* Controller.                                                 */
         Result = HCI_VS_SetRadio(BLE_Demo_Context.BluetoothStackID, Parameter_List[0].Integer_Value);
         if(!Result)
            QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "        Success setting Radio %d.\n", Parameter_List[0].Integer_Value);
         else
            QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "        Error %d setting Radio %d.\n", Result, Parameter_List[0].Integer_Value);

         /* Flag success to the caller.                                 */
         ret_val = QCLI_STATUS_SUCCESS_E;
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         ret_val = QCLI_STATUS_USAGE_E;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = QCLI_STATUS_ERROR_E;
   }

   return(ret_val);
}

   /* The following function is responsible for running a test of the   */
   /* HCI exposed crypto functionality.  This function returns          */
   /* QCLI_STATUS_SUCCESS_E on success or an error enumeration if an    */
   /* error occurs.                                                     */
   /* * NOTE * The AES test key and plaintext data are pulled from Vol  */
   /*          6, Part C, Section 1.1 of the Bluetooth 4.2 Core spec (as*/
   /*          is the expected result).                                 */
static QCLI_Command_Status_t CryptoTest(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   int                        Result;
   uint8_t                    Status;
   uint32_t                   Index;
   uint32_t                   RandomCount;
   QCLI_Command_Status_t      ret_val;
   qapi_BLE_Random_Number_t   Rand;
   qapi_BLE_Encryption_Key_t  Key                   = { 0xbf, 0x01, 0xfb, 0x9d, 0x4e, 0xf3, 0xbc, 0x36, 0xd8, 0x74, 0xf5, 0x39, 0x41, 0x38, 0x68, 0x4c };
   qapi_BLE_Plain_Text_Data_t Plain_Text_Data       = {{0x13, 0x02, 0xf1, 0xe0, 0xdf, 0xce, 0xbd, 0xac, 0x79, 0x68, 0x57, 0x46, 0x35, 0x24, 0x13, 0x02}};
   qapi_BLE_Encrypted_Data_t  ExpectedEncryptedData = {{0x66, 0xc6, 0xc2, 0x27, 0x8e, 0x3b, 0x8e, 0x05, 0x3e, 0x7e, 0xa3, 0x26, 0x52, 0x1b, 0xad, 0x99}};
   qapi_BLE_Encrypted_Data_t  EncryptedData;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BLE_Demo_Context.BluetoothStackID)
   {
      /* Get a sequence of random numbers.                              */
      Index       = 0;
      ret_val     = QCLI_STATUS_SUCCESS_E;
      RandomCount = 10;
      do
      {
         /* Attempt to get a random number.                             */
         if((Result = qapi_BLE_HCI_LE_Rand(BLE_Demo_Context.BluetoothStackID,  &Status,  &Rand)) == 0)
         {
            /* Verify the HCI Status.                                   */
            if(Status == QAPI_BLE_HCI_ERROR_CODE_SUCCESS)
            {
               /* if this is the first iteration get a random count for */
               /* how many times we will get a random number, between   */
               /* 10-20.                                                */
               if(Index == 0)
               {
                  RandomCount  = Rand.Random_Number0 | (Rand.Random_Number1 << 8) | (Rand.Random_Number2 << 16) | (Rand.Random_Number3 << 24);
                  RandomCount %= 10;
                  RandomCount += 10;
               }
               else
               {
                  QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Rand(%u of %u):%s0x%02X%02X%02X%02X%02X%02X%02X%02X.\n", (unsigned int)Index, (unsigned int)RandomCount, (Index < 10)?"  ":" ", Rand.Random_Number7, Rand.Random_Number6, Rand.Random_Number5, Rand.Random_Number4, Rand.Random_Number3, Rand.Random_Number2, Rand.Random_Number1, Rand.Random_Number0);
               }
            }
            else
            {
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Error - qapi_BLE_HCI_LE_Rand() HCI Status: 0x%02X.\n", Status);

               ret_val = QCLI_STATUS_ERROR_E;
            }
         }
         else
         {
            QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Error - qapi_BLE_HCI_LE_Rand(): %d.\n", Result);

            ret_val = QCLI_STATUS_ERROR_E;
         }
      }
      while((ret_val == QCLI_STATUS_SUCCESS_E) && (Index++ < RandomCount));

      /* Run a test of HCI_LE_Encrypt.                                  */
      if((Result = qapi_BLE_HCI_LE_Encrypt(BLE_Demo_Context.BluetoothStackID, Key, Plain_Text_Data, &Status,  &EncryptedData)) == 0)
      {
         /* Verify the HCI Status.                                      */
         if(Status == QAPI_BLE_HCI_ERROR_CODE_SUCCESS)
         {
            /* Verify that we received the expected result.             */
            if(!memcmp(EncryptedData.Encrypted_Data, ExpectedEncryptedData.Encrypted_Data, QAPI_BLE_ENCRYPTED_DATA_MAXIMUM_SIZE))
            {
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "qapi_BLE_HCI_LE_Encrypt() success, generated expected data.\n");
            }
            else
            {
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Error - encrypted data mismatch.\n");

               ret_val = QCLI_STATUS_ERROR_E;
            }

            /* Print the expected data and the actual data.             */
            QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Expected Cipher Text: 0x");
            for(Index = 0;Index < QAPI_BLE_ENCRYPTED_DATA_MAXIMUM_SIZE; Index++)
            {
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "%02X", ExpectedEncryptedData.Encrypted_Data[(QAPI_BLE_ENCRYPTED_DATA_MAXIMUM_SIZE - 1) - Index]);
            }
            QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "\n");

            QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Actual Cipher Text:   0x");
            for(Index = 0;Index < QAPI_BLE_ENCRYPTED_DATA_MAXIMUM_SIZE; Index++)
            {
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "%02X", EncryptedData.Encrypted_Data[(QAPI_BLE_ENCRYPTED_DATA_MAXIMUM_SIZE - 1) - Index]);
            }
            QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "\n");
         }
         else
         {
            QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Error - qapi_BLE_HCI_LE_Encrypt() HCI Status: 0x%02X.\n", Status);

            ret_val = QCLI_STATUS_ERROR_E;
         }
      }
      else
      {
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Error - qapi_BLE_HCI_LE_Encrypt(): %d.\n", Result);

         ret_val = QCLI_STATUS_ERROR_E;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = QCLI_STATUS_ERROR_E;
   }

   return(ret_val);
}

   /* The following function is responsible for setting the             */
   /* Discoverability Mode of the local device.  This function returns  */
   /* zero on successful execution and a negative value on all errors.  */
static QCLI_Command_Status_t SetDiscoverabilityMode(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t               ret_val;
   qapi_BLE_GAP_Discoverability_Mode_t DiscoverabilityMode;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BLE_Demo_Context.BluetoothStackID)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((Parameter_Count > 0) && (Parameter_List) && (Parameter_List[0].Integer_Is_Valid) && (Parameter_List[0].Integer_Value >= 0) && (Parameter_List[0].Integer_Value <= 2))
      {
         /* Parameters appear to be valid, map the specified parameters */
         /* into the API specific parameters.                           */
         if(Parameter_List[0].Integer_Value == 1)
            DiscoverabilityMode = QAPI_BLE_DM_LIMITED_DISCOVERABLE_MODE_E;
         else
         {
            if(Parameter_List[0].Integer_Value == 2)
               DiscoverabilityMode = QAPI_BLE_DM_GENERAL_DISCOVERABLE_MODE_E;
            else
               DiscoverabilityMode = QAPI_BLE_DM_NON_DISCOVERABLE_MODE_E;
         }

         /* Set the LE Discoveryability Mode.                           */
         BLE_Demo_Context.LE_Parameters.DiscoverabilityMode = DiscoverabilityMode;

         /* The Mode was changed successfully.                          */
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Discoverability: %s.\n", (DiscoverabilityMode == QAPI_BLE_DM_NON_DISCOVERABLE_MODE_E)?"Non":((DiscoverabilityMode == QAPI_BLE_DM_GENERAL_DISCOVERABLE_MODE_E)?"General":"Limited"));

         /* Flag success to the caller.                                 */
         ret_val = QCLI_STATUS_SUCCESS_E;
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         ret_val = QCLI_STATUS_USAGE_E;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = QCLI_STATUS_ERROR_E;
   }

   return(ret_val);
}

   /* The following function is responsible for setting the             */
   /* Connectability Mode of the local device.  This function returns   */
   /* zero on successful execution and a negative value on all errors.  */
static QCLI_Command_Status_t SetConnectabilityMode(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t ret_val;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BLE_Demo_Context.BluetoothStackID)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((Parameter_Count >= 1) && (Parameter_List) && (Parameter_List[0].Integer_Is_Valid) && (Parameter_List[0].Integer_Value >= 0) && (Parameter_List[0].Integer_Value <= 1))
      {
         /* Parameters appear to be valid, map the specified parameters */
         /* into the API specific parameters.                           */
         /* * NOTE * The Connectability Mode in LE is only applicable   */
         /*          when advertising, if a device is not advertising   */
         /*          it is not connectable.                             */
         if(Parameter_List[0].Integer_Value == 0)
            BLE_Demo_Context.LE_Parameters.ConnectableMode = QAPI_BLE_LCM_NON_CONNECTABLE_E;
         else
            BLE_Demo_Context.LE_Parameters.ConnectableMode = QAPI_BLE_LCM_CONNECTABLE_E;

         /* The Mode was changed successfully.                          */
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Connectability Mode: %s.\n", (BLE_Demo_Context.LE_Parameters.ConnectableMode == QAPI_BLE_LCM_NON_CONNECTABLE_E)?"Non Connectable":"Connectable");

         /* Flag success to the caller.                                 */
         ret_val = QCLI_STATUS_SUCCESS_E;
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         ret_val = QCLI_STATUS_USAGE_E;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = QCLI_STATUS_ERROR_E;
   }

   return(ret_val);
}

   /* The following function is responsible for setting the Pairability */
   /* Mode of the local device.  This function returns zero on          */
   /* successful execution and a negative value on all errors.          */
static QCLI_Command_Status_t SetPairabilityMode(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   int                                 Result;
   char                               *Mode;
   QCLI_Command_Status_t               ret_val;
   qapi_BLE_GAP_LE_Pairability_Mode_t  PairabilityMode;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BLE_Demo_Context.BluetoothStackID)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((Parameter_Count > 0) && (Parameter_List) && (Parameter_List[0].Integer_Is_Valid) && (Parameter_List[0].Integer_Value >= 0) && (Parameter_List[0].Integer_Value <= 2))
      {
         /* Parameters appear to be valid, map the specified parameters */
         /* into the API specific parameters.                           */
         if(Parameter_List[0].Integer_Value == 0)
         {
            PairabilityMode = QAPI_BLE_LPM_NON_PAIRABLEMODE_E;
            Mode            = "QAPI_BLE_LPM_NON_PAIRABLEMODE_E";
         }
         else
         {
            if(Parameter_List[0].Integer_Value == 1)
            {
               PairabilityMode = QAPI_BLE_LPM_PAIRABLE_MODE_E;
               Mode            = "QAPI_BLE_LPM_PAIRABLE_MODE_E";
            }
            else
            {
               PairabilityMode = QAPI_BLE_LPM_PAIRABLE_MODE_ENABLE_EXTENDED_EVENTS_E;
               Mode            = "QAPI_BLE_LPM_PAIRABLE_MODE_ENABLE_EXTENDED_EVENTS_E";
            }
         }

         /* Parameters mapped, now set the Pairability Mode.            */
         Result = qapi_BLE_GAP_LE_Set_Pairability_Mode(BLE_Demo_Context.BluetoothStackID, PairabilityMode);

         /* Next, check the return value to see if the command was      */
         /* issued successfully.                                        */
         if(Result >= 0)
         {
            /* The Mode was changed successfully.                       */
            QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Pairability Mode Changed to %s.\n", Mode);

            /* If Secure Simple Pairing has been enabled, inform the    */
            /* user of the current Secure Simple Pairing parameters.    */
            if(PairabilityMode == QAPI_BLE_LPM_PAIRABLE_MODE_E)
               DisplayIOCapabilities();

            /* Flag success to the caller.                              */
            ret_val = QCLI_STATUS_SUCCESS_E;
         }
         else
         {
            /* There was an error setting the Mode.                     */
            DisplayFunctionError("GAP_Set_Pairability_Mode", Result);

            /* Flag that an error occurred while submitting the command.*/
            ret_val = QCLI_STATUS_ERROR_E;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         ret_val = QCLI_STATUS_USAGE_E;
      }
   }
   else
      ret_val = QCLI_STATUS_ERROR_E;

   return(ret_val);
}


   /* The following function is responsible for changing the Secure     */
   /* Simple Pairing Parameters that are exchanged during the Pairing   */
   /* procedure when Secure Simple Pairing (Security Level 4) is used.  */
   /* This function returns zero on successful execution and a negative */
   /* value on all errors.                                              */
static QCLI_Command_Status_t ChangePairingParameters(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t ret_val;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BLE_Demo_Context.BluetoothStackID)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((Parameter_Count >= 3) && (Parameter_List) && (Parameter_List[0].Integer_Is_Valid) && (Parameter_List[1].Integer_Is_Valid) && (Parameter_List[0].Integer_Value >= 0) && (Parameter_List[0].Integer_Value <= 4))
      {
         /* Parameters appear to be valid, map the specified parameters */
         /* into the API specific parameters.                           */
         if(Parameter_List[0].Integer_Value == 0)
            BLE_Demo_Context.LE_Parameters.IOCapability = QAPI_BLE_LIC_DISPLAY_ONLY_E;
         else
         {
            if(Parameter_List[0].Integer_Value == 1)
               BLE_Demo_Context.LE_Parameters.IOCapability = QAPI_BLE_LIC_DISPLAY_YES_NO_E;
            else
            {
               if(Parameter_List[0].Integer_Value == 2)
                  BLE_Demo_Context.LE_Parameters.IOCapability = QAPI_BLE_LIC_KEYBOARD_ONLY_E;
               else
               {
                  if(Parameter_List[0].Integer_Value == 3)
                     BLE_Demo_Context.LE_Parameters.IOCapability = QAPI_BLE_LIC_NO_INPUT_NO_OUTPUT_E;
                  else
                     BLE_Demo_Context.LE_Parameters.IOCapability = QAPI_BLE_LIC_KEYBOARD_DISPLAY_E;
               }
            }
         }

         /* Finally map the Man in the Middle (MITM) Protection valid.  */
         BLE_Demo_Context.LE_Parameters.MITMProtection    = (boolean_t)(Parameter_List[1].Integer_Value ? TRUE : FALSE);

         /* Also, map the secure connections.                           */
         BLE_Demo_Context.LE_Parameters.SecureConnections = (boolean_t)(Parameter_List[2].Integer_Value ? TRUE : FALSE);

         /* Inform the user of the New I/O Capabilities.                */
         DisplayIOCapabilities();

         /* Flag success to the caller.                                 */
         ret_val = QCLI_STATUS_SUCCESS_E;
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         ret_val = QCLI_STATUS_USAGE_E;
      }
   }
   else
      ret_val = QCLI_STATUS_ERROR_E;

   return(ret_val);
}

   /* The following function is responsible for issuing a GAP           */
   /* Authentication Response with a Pass Key value specified via the   */
   /* input parameter.  This function returns zero on successful        */
   /* execution and a negative value on all errors.                     */
static QCLI_Command_Status_t LEPassKeyResponse(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   int                                                   Result;
   QCLI_Command_Status_t                                 ret_val;
   qapi_BLE_GAP_LE_Authentication_Response_Information_t GAP_LE_Authentication_Response_Information;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BLE_Demo_Context.BluetoothStackID)
   {
      /* First, check to see if there is an on-going Pairing operation  */
      /* active.                                                        */
      if(!QAPI_BLE_COMPARE_NULL_BD_ADDR(BLE_Demo_Context.CurrentRemoteBD_ADDR))
      {
         /* Make sure that all of the parameters required for this      */
         /* function appear to be at least semi-valid.                  */
         if((Parameter_Count > 0) && (Parameter_List) && (Parameter_List[0].String_Value) && (strlen((char *)(Parameter_List[0].String_Value)) <= QAPI_BLE_GAP_LE_PASSKEY_MAXIMUM_NUMBER_OF_DIGITS))
         {
            /* Parameters appear to be valid, go ahead and populate the */
            /* response structure.                                      */
            GAP_LE_Authentication_Response_Information.GAP_LE_Authentication_Type  = QAPI_BLE_LAR_PASSKEY_E;
            GAP_LE_Authentication_Response_Information.Authentication_Data_Length  = (uint8_t)(sizeof(uint32_t));
            GAP_LE_Authentication_Response_Information.Authentication_Data.Passkey = (uint32_t)(Parameter_List[0].Integer_Value);

            /* Submit the Authentication Response.                      */
            Result = qapi_BLE_GAP_LE_Authentication_Response(BLE_Demo_Context.BluetoothStackID, BLE_Demo_Context.CurrentRemoteBD_ADDR, &GAP_LE_Authentication_Response_Information);

            /* Check the return value for the submitted command for     */
            /* success.                                                 */
            if(!Result)
            {
               /* Operation was successful, inform the user.            */
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Passkey Response success.\n");

               /* Flag success to the caller.                           */
               ret_val = QCLI_STATUS_SUCCESS_E;
            }
            else
            {
               /* Inform the user that the Authentication Response was  */
               /* not successful.                                       */
               DisplayFunctionError("GAP_LE_Authentication_Response", Result);

               ret_val = QCLI_STATUS_ERROR_E;
            }

            /* Flag that there is no longer a current Authentication    */
            /* procedure in progress.                                   */
            QAPI_BLE_ASSIGN_BD_ADDR(BLE_Demo_Context.CurrentRemoteBD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
         }
         else
         {
            /* One or more of the necessary parameters is/are invalid.  */
            ret_val = QCLI_STATUS_USAGE_E;
         }
      }
      else
      {
         /* There is not currently an on-going authentication operation,*/
         /* inform the user of this error condition.                    */
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Pass Key Authentication Response: Authentication not in progress.\n");

         ret_val = QCLI_STATUS_ERROR_E;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = QCLI_STATUS_ERROR_E;
   }

   return(ret_val);
}

   /* The following function is responsible for querying the Encryption */
   /* Mode for an LE Connection.  This function returns zero on         */
   /* successful execution and a negative value on all errors.          */
static QCLI_Command_Status_t LEQueryEncryption(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   int                            Result;
   QCLI_Command_Status_t          ret_val;
   qapi_BLE_GAP_Encryption_Mode_t GAP_Encryption_Mode;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BLE_Demo_Context.BluetoothStackID)
   {
      /* First, check to see if there is an on-going Pairing operation  */
      /* active.                                                        */
      if(!QAPI_BLE_COMPARE_NULL_BD_ADDR(BLE_Demo_Context.ConnectionBD_ADDR))
      {
         /* Query the current Encryption Mode for this Connection.      */
         Result = qapi_BLE_GAP_LE_Query_Encryption_Mode(BLE_Demo_Context.BluetoothStackID, BLE_Demo_Context.ConnectionBD_ADDR, &GAP_Encryption_Mode);
         if(!Result)
         {
            QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Current Encryption Mode: %s.\n", (GAP_Encryption_Mode == QAPI_BLE_EM_ENABLED_E)?"Enabled":"Disabled");

            ret_val = QCLI_STATUS_SUCCESS_E;
         }
         else
         {
            QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Error - GAP_LE_Query_Encryption_Mode returned %d.\n", Result);

            ret_val = QCLI_STATUS_ERROR_E;
         }
      }
      else
      {
         /* There is not currently an on-going authentication operation,*/
         /* inform the user of this error condition.                    */
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Not Connected.\n");

         ret_val = QCLI_STATUS_ERROR_E;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = QCLI_STATUS_ERROR_E;
   }

   return(ret_val);
}

   /* The following function is responsible for querying the Encryption */
   /* Mode for an LE Connection.  This function returns zero on         */
   /* successful execution and a negative value on all errors.          */
static QCLI_Command_Status_t LESetPasskey(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   int                   Result;
   uint32_t              Passkey;
   QCLI_Command_Status_t ret_val;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BLE_Demo_Context.BluetoothStackID)
   {
      /* Make sure that all of the parameters required for this         */
      /* function appear to be at least semi-valid.                     */
      if((Parameter_Count >= 1) && (Parameter_List) && (Parameter_List[0].Integer_Is_Valid) &&  ((Parameter_List[0].Integer_Value == 0) || (Parameter_List[0].Integer_Value == 1)))
      {
         /* Check to see if we are setting or clearing the LE Fixed     */
         /* Passkey.                                                    */
         if(Parameter_List[0].Integer_Value == 1)
         {
            /* We are setting the passkey so make sure it is valid.     */
            if((Parameter_Count >= 2) && (Parameter_List[1].String_Value) && (strlen((char *)(Parameter_List[1].String_Value)) <= QAPI_BLE_GAP_LE_PASSKEY_MAXIMUM_NUMBER_OF_DIGITS))
            {
               /* Store the passkey.                                    */
               Passkey = (uint32_t)(Parameter_List[1].String_Value);

               /* Attempt to set the fixed passkey.                     */
               Result  = qapi_BLE_GAP_LE_Set_Fixed_Passkey(BLE_Demo_Context.BluetoothStackID, &Passkey);
               if(!Result)
               {
                  QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Fixed Passkey set to %06u.\n", (unsigned int)Passkey);

                  ret_val = QCLI_STATUS_SUCCESS_E;
               }
               else
               {
                  QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Error - GAP_LE_Set_Fixed_Passkey returned %d.\n", Result);

                  ret_val = QCLI_STATUS_ERROR_E;
               }
            }
            else
            {
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Error - Invalid Passkey.\n");

               ret_val = QCLI_STATUS_USAGE_E;
            }
         }
         else
         {
            /* Un-set the fixed passkey that we previously configured.  */
            Result = qapi_BLE_GAP_LE_Set_Fixed_Passkey(BLE_Demo_Context.BluetoothStackID, NULL);
            if(!Result)
            {
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Fixed Passkey no longer configured.\n");

               ret_val = QCLI_STATUS_SUCCESS_E;
            }
            else
            {
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Error - GAP_LE_Set_Fixed_Passkey returned %d.\n", Result);

               ret_val = QCLI_STATUS_ERROR_E;
            }
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         ret_val = QCLI_STATUS_USAGE_E;
      }
   }
   else
      ret_val = QCLI_STATUS_ERROR_E;

   return(ret_val);
}

   /* The following function is responsible for querying the Bluetooth  */
   /* Device Address of the local Bluetooth Device.  This function      */
   /* returns zero on successful execution and a negative value on all  */
   /* errors.                                                           */
static QCLI_Command_Status_t GetLocalAddress(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   int                   Result;
   BoardStr_t            BoardStr;
   qapi_BLE_BD_ADDR_t    BD_ADDR;
   QCLI_Command_Status_t ret_val;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BLE_Demo_Context.BluetoothStackID)
   {
      /* Attempt to submit the command.                                 */
      Result = qapi_BLE_GAP_Query_Local_BD_ADDR(BLE_Demo_Context.BluetoothStackID, &BD_ADDR);

      /* Check the return value of the submitted command for success.   */
      if(!Result)
      {
         BD_ADDRToStr(BD_ADDR, BoardStr);

         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "BD_ADDR of Local Device is: %s.\n", BoardStr);

         /* Flag success to the caller.                                 */
         ret_val = QCLI_STATUS_SUCCESS_E;
      }
      else
      {
         /* Display a message indicating that an error occurred while   */
         /* attempting to query the Local Device Address.               */
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "GAP_Query_Local_BD_ADDR() Failure: %d.\n", Result);

         ret_val = QCLI_STATUS_ERROR_E;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = QCLI_STATUS_ERROR_E;
   }

   return(ret_val);
}

   /* The following function is responsible for enabling LE             */
   /* Advertisements.  This function returns zero on successful         */
   /* execution and a negative value on all errors.                     */
static QCLI_Command_Status_t AdvertiseLE(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   int                                         Length;
   int                                         Result;
   uint32_t                                    AdvDataLength;
   uint32_t                                    Index;
   QCLI_Command_Status_t                       ret_val;
   qapi_BLE_GAP_LE_Connectability_Parameters_t ConnectabilityParameters;
   union
   {
      qapi_BLE_Advertising_Data_t              AdvertisingData;
      qapi_BLE_Scan_Response_Data_t            ScanResponseData;
   } Advertisement_Data_Buffer;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BLE_Demo_Context.BluetoothStackID)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((Parameter_Count > 0) && (Parameter_List) && (Parameter_List[0].Integer_Is_Valid) && (Parameter_List[0].Integer_Value >= 0) && (Parameter_List[0].Integer_Value <= 1))
      {
         /* Determine whether to enable or disable Advertising.         */
         if(Parameter_List[0].Integer_Value == 0)
         {
            /* Disable Advertising.                                     */
            Result = qapi_BLE_GAP_LE_Advertising_Disable(BLE_Demo_Context.BluetoothStackID);
            if(!Result)
            {
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   GAP_LE_Advertising_Disable success.\n");

               ret_val = QCLI_STATUS_SUCCESS_E;
            }
            else
            {
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   GAP_LE_Advertising_Disable returned %d.\n", Result);

               ret_val = QCLI_STATUS_ERROR_E;
            }
         }
         else
         {
            /* Set the Advertising Data.                                */
            memset(&(Advertisement_Data_Buffer.AdvertisingData), 0, sizeof(qapi_BLE_Advertising_Data_t));

            /* Set the Flags A/D Field (1 byte type and 1 byte Flags.   */
            Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[0] = 2;
            Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[1] = QAPI_BLE_HCI_LE_ADVERTISING_REPORT_DATA_TYPE_FLAGS;
            Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[2] = 0;

            /* Configure the flags field based on the Discoverability   */
            /* Mode.                                                    */
            if(BLE_Demo_Context.LE_Parameters.DiscoverabilityMode == QAPI_BLE_DM_GENERAL_DISCOVERABLE_MODE_E)
               Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[2] = QAPI_BLE_HCI_LE_ADVERTISING_FLAGS_GENERAL_DISCOVERABLE_MODE_FLAGS_BIT_MASK;
            else
            {
               if(BLE_Demo_Context.LE_Parameters.DiscoverabilityMode == QAPI_BLE_DM_LIMITED_DISCOVERABLE_MODE_E)
                  Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[2] = QAPI_BLE_HCI_LE_ADVERTISING_FLAGS_LIMITED_DISCOVERABLE_MODE_FLAGS_BIT_MASK;
            }

            /* Set the advertising data length.                         */
            AdvDataLength = (Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[0] + 1);

            /* Make sure we advertise enough data according to the test */
            /* settings assuming we have room for a TLV structure.      */
            if((BLE_Demo_Context.TestAdvertisingDataLength) && (AdvDataLength <= (BLE_Demo_Context.TestAdvertisingDataLength - 2)) && (BLE_Demo_Context.TestAdvertisingDataLength <= QAPI_BLE_ADVERTISING_DATA_MAXIMUM_SIZE))
            {
               /* Pad the advertising data to match the test length.    */
               Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[3] = (BLE_Demo_Context.TestAdvertisingDataLength - AdvDataLength - 1);
               Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[4] = QAPI_BLE_HCI_LE_ADVERTISING_REPORT_DATA_TYPE_MANUFACTURER_SPECIFIC;

               for(Index=0;Index<(BLE_Demo_Context.TestAdvertisingDataLength - AdvDataLength - 2);Index++)
                  Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[5 + Index] = 0xA5 ^ Index;

               /* Set the advertising data length.                      */
               AdvDataLength = BLE_Demo_Context.TestAdvertisingDataLength;
            }

            /* Print the advertising data length.                       */
            QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   Advertising Data Length: %u", (unsigned int)AdvDataLength);

            /* Write thee advertising data to the chip.                 */
            Result = qapi_BLE_GAP_LE_Set_Advertising_Data(BLE_Demo_Context.BluetoothStackID, AdvDataLength, &(Advertisement_Data_Buffer.AdvertisingData));
            if(!Result)
            {
               /* Configure the scan response data.                     */
               memset(&(Advertisement_Data_Buffer.ScanResponseData), 0, sizeof(qapi_BLE_Scan_Response_Data_t));

               /* Set the Scan Response Data.                           */
               Length = strlen(DEVICE_FRIENDLY_NAME);
               if(Length < (QAPI_BLE_ADVERTISING_DATA_MAXIMUM_SIZE - 2))
                  Advertisement_Data_Buffer.ScanResponseData.Scan_Response_Data[1] = QAPI_BLE_HCI_LE_ADVERTISING_REPORT_DATA_TYPE_LOCAL_NAME_COMPLETE;
               else
               {
                  Advertisement_Data_Buffer.ScanResponseData.Scan_Response_Data[1] = QAPI_BLE_HCI_LE_ADVERTISING_REPORT_DATA_TYPE_LOCAL_NAME_SHORTENED;
                  Length = (QAPI_BLE_ADVERTISING_DATA_MAXIMUM_SIZE - 2);
               }

               Advertisement_Data_Buffer.ScanResponseData.Scan_Response_Data[0] = (uint8_t)(1 + Length);
               memcpy(&(Advertisement_Data_Buffer.ScanResponseData.Scan_Response_Data[2]), DEVICE_FRIENDLY_NAME, Length);

               Result = qapi_BLE_GAP_LE_Set_Scan_Response_Data(BLE_Demo_Context.BluetoothStackID, (Advertisement_Data_Buffer.ScanResponseData.Scan_Response_Data[0] + 1), &(Advertisement_Data_Buffer.ScanResponseData));
               if(!Result)
               {
                  /* Set up the default advertising parameters if they  */
                  /* have not been configured at the CLI.               */
                  if(!(BLE_Demo_Context.BLEParameters.Flags & BLE_PARAMETERS_FLAGS_ADVERTISING_PARAMETERS_VALID))
                  {
                     /* Configure the default advertising parameters.   */
                     BLE_Demo_Context.BLEParameters.EnableAdvScanResponse                           = TRUE;
                     BLE_Demo_Context.BLEParameters.AdvertisingParameters.Advertising_Channel_Map   = QAPI_BLE_HCI_LE_ADVERTISING_CHANNEL_MAP_DEFAULT;
                     BLE_Demo_Context.BLEParameters.AdvertisingParameters.Scan_Request_Filter       = QAPI_BLE_FP_NO_FILTER_E;
                     BLE_Demo_Context.BLEParameters.AdvertisingParameters.Connect_Request_Filter    = QAPI_BLE_FP_NO_FILTER_E;
                     BLE_Demo_Context.BLEParameters.AdvertisingParameters.Advertising_Interval_Min  = 100;
                     BLE_Demo_Context.BLEParameters.AdvertisingParameters.Advertising_Interval_Max  = 200;

                     /* Flag that the parameters are valid so we don't  */
                     /* set them unnecessarily.                         */
                     BLE_Demo_Context.BLEParameters.Flags |= BLE_PARAMETERS_FLAGS_ADVERTISING_PARAMETERS_VALID;
                  }

                  /* Configure the Connectability Parameters.           */
                  /* * NOTE * Since we do not ever put ourselves to be  */
                  /*          direct connectable then we will set the   */
                  /*          DirectAddress to all 0s.                  */
                  ConnectabilityParameters.Connectability_Mode   = BLE_Demo_Context.LE_Parameters.ConnectableMode;
                  ConnectabilityParameters.Own_Address_Type      = QAPI_BLE_LAT_PUBLIC_E;
                  ConnectabilityParameters.Direct_Address_Type   = QAPI_BLE_LAT_PUBLIC_E;
                  QAPI_BLE_ASSIGN_BD_ADDR(ConnectabilityParameters.Direct_Address, 0, 0, 0, 0, 0, 0);

                  /* Now enable advertising.                            */
                  Result = qapi_BLE_GAP_LE_Advertising_Enable(BLE_Demo_Context.BluetoothStackID, BLE_Demo_Context.BLEParameters.EnableAdvScanResponse, &(BLE_Demo_Context.BLEParameters.AdvertisingParameters), &(ConnectabilityParameters), GAP_LE_Event_Callback, 0);
                  if(!Result)
                  {
                     QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   GAP_LE_Advertising_Enable success, Advertising Interval Range: %u - %u.\n", (unsigned int)BLE_Demo_Context.BLEParameters.AdvertisingParameters.Advertising_Interval_Min, (unsigned int)BLE_Demo_Context.BLEParameters.AdvertisingParameters.Advertising_Interval_Max);

                     ret_val = QCLI_STATUS_SUCCESS_E;
                  }
                  else
                  {
                     QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   GAP_LE_Advertising_Enable returned %d.\n", Result);

                     ret_val = QCLI_STATUS_ERROR_E;
                  }
               }
               else
               {
                  QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   qapi_BLE_GAP_LE_Set_Advertising_Data(dtScanResponse) returned %d.\n", Result);

                  ret_val = QCLI_STATUS_ERROR_E;
               }

            }
            else
            {
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   qapi_BLE_GAP_LE_Set_Advertising_Data(dtAdvertising) returned %d.\n", Result);

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


   /* The following function is responsible for starting an LE scan     */
   /* procedure.  This function returns zero if successful and a        */
   /* negative value if an error occurred.                              */
static QCLI_Command_Status_t ScanLE(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t           ret_val;
   qapi_BLE_GAP_LE_Filter_Policy_t FilterPolicy;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BLE_Demo_Context.BluetoothStackID)
   {
      /* Verify the command line parameters.                            */
      if((Parameter_Count >= 1) && (Parameter_List) && (Parameter_List[0].Integer_Is_Valid) && ((Parameter_List[0].Integer_Value == 0) || (Parameter_List[0].Integer_Value == 1)))
      {
         /* Check to see if we are enabling or disabling scanning.      */
         if(Parameter_List[0].Integer_Value == 0)
         {
            /* Check to see if we are in fact in the process of         */
            /* scanning.                                                */
            if(BLE_Demo_Context.ScanInProgress)
            {
               /* Simply stop scanning.                                 */
               if(!StopScan(BLE_Demo_Context.BluetoothStackID))
               {
                  /* Flag that scanning is not in progess.              */
                  BLE_Demo_Context.ScanInProgress = FALSE;

                  /* Return success to the caller.                      */
                  ret_val                         = QCLI_STATUS_SUCCESS_E;
               }
               else
                  ret_val = QCLI_STATUS_ERROR_E;
            }
            else
            {
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Scanning is not in progress.\n");

               ret_val = QCLI_STATUS_SUCCESS_E;
            }
         }
         else
         {
            /* Check to see if scanning is in progress.                 */
            if(!(BLE_Demo_Context.ScanInProgress))
            {
               /* Determine if the optional filter policy is specified. */
               if((Parameter_Count >= 2) && (Parameter_List) && (Parameter_List[1].Integer_Is_Valid) && (Parameter_List[1].Integer_Value >= QAPI_BLE_FP_NO_FILTER_E) && (Parameter_List[1].Integer_Value <= QAPI_BLE_FP_WHITE_LIST_DIRECTED_RPA_E))
               {
                  /* Store the user specified filter policy.            */
                  FilterPolicy = (qapi_BLE_GAP_LE_Filter_Policy_t)Parameter_List[1].Integer_Value;
               }
               else
               {
                  /* Otherwise we will use no filter.                   */
                  FilterPolicy = QAPI_BLE_FP_NO_FILTER_E;
               }

               /* Simply start scanning.                                */
               if(!StartScan(BLE_Demo_Context.BluetoothStackID, FilterPolicy))
               {
                  /* Flag that scanning is in progess.                  */
                  BLE_Demo_Context.ScanInProgress = TRUE;

                  /* Return success to the caller.                      */
                  ret_val                         = QCLI_STATUS_SUCCESS_E;
               }
               else
                  ret_val = QCLI_STATUS_ERROR_E;
            }
            else
            {
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Scan already in progress.\n");

               ret_val = QCLI_STATUS_SUCCESS_E;
            }
         }
      }
      else
         ret_val = QCLI_STATUS_USAGE_E;
   }
   else
      ret_val = QCLI_STATUS_ERROR_E;

   return(ret_val);
}

   /* The following function is responsible for connecting to a remote  */
   /* LE device.  This function returns zero if successful and a        */
   /* negative value if an error occurred.                              */
static QCLI_Command_Status_t ConnectLE(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   qapi_BLE_BD_ADDR_t    BD_ADDR;
   QCLI_Command_Status_t ret_val;
   boolean_t             UseWhiteList;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BLE_Demo_Context.BluetoothStackID)
   {
      /* Next, make sure that a valid device address exists.            */
      if((Parameter_Count >= 1) && (Parameter_List) && (Parameter_List[0].Integer_Is_Valid) && (Parameter_List[0].Integer_Value >= 0) && (Parameter_List[0].Integer_Value <= 1))
      {
         /* Store if the user has requested to use the White List in the*/
         /* controller.                                                 */
         if(Parameter_List[0].Integer_Value == 1)
            UseWhiteList = TRUE;
         else
            UseWhiteList = FALSE;

         /* Check to see if scanning is in progress, if so cancel it.   */
         if(BLE_Demo_Context.ScanInProgress)
         {
            /* Attempt to stop scanning.                                */
            if(!StopScan(BLE_Demo_Context.BluetoothStackID))
               BLE_Demo_Context.ScanInProgress = FALSE;

            QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Scan stopped before making LE Connection\n");
         }

         /* If we are NOT using the White List in the controller, then  */
         /* the local device will send a connection request when a      */
         /* connectable advertisement is received from a remote device  */
         /* in the White List.                                          */
         /* * NOTE * A remote device and address type MUST have         */
         /*          previously been added to the White List in the     */
         /*          controller.                                        */
         if(UseWhiteList)
         {
            /* Attempt to connect using the White List in the           */
            /* controller.                                              */
            /* * NOTE * The remote address and address type are NOT     */
            /*          used.                                           */
            if(!ConnectLEDevice(BLE_Demo_Context.BluetoothStackID, UseWhiteList, NULL, 0))
               ret_val = QCLI_STATUS_SUCCESS_E;
            else
               ret_val = QCLI_STATUS_ERROR_E;
         }
         else
         {
            /* Make sure the remote address and address type have been  */
            /* specified.                                               */
            if((Parameter_Count >= 3) && (Parameter_List[1].String_Value) && (strlen((char *)(Parameter_List[1].String_Value)) == (sizeof(qapi_BLE_BD_ADDR_t) * 2)) && (Parameter_List[2].Integer_Is_Valid) && (Parameter_List[2].Integer_Value >= QAPI_BLE_LAT_PUBLIC_E) && (Parameter_List[2].Integer_Value <= QAPI_BLE_LAT_RANDOM_IDENTITY_E))
            {
               /* Convert the parameter to a Bluetooth Device Address.  */
               StrToBD_ADDR((char *)(Parameter_List[1].String_Value), &BD_ADDR);

               /* Attempt to connect to the specified remote address and*/
               /* address type.                                         */
               if(!ConnectLEDevice(BLE_Demo_Context.BluetoothStackID, UseWhiteList, &BD_ADDR, (qapi_BLE_GAP_LE_Address_Type_t)Parameter_List[2].Integer_Value))
                  ret_val = QCLI_STATUS_SUCCESS_E;
               else
                  ret_val = QCLI_STATUS_ERROR_E;
            }
            else
               ret_val = QCLI_STATUS_USAGE_E;
         }
      }
      else
         ret_val = QCLI_STATUS_USAGE_E;
   }
   else
      ret_val = QCLI_STATUS_ERROR_E;

   return(ret_val);
}

   /* The following function is responsible for disconnecting to an LE  */
   /* device.  This function returns zero if successful and a negative  */
   /* value if an error occurred.                                       */
static QCLI_Command_Status_t DisconnectLE(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t ret_val;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BLE_Demo_Context.BluetoothStackID)
   {
      /* Next, check to make sure we are currently connected.           */
      if(!QAPI_BLE_COMPARE_NULL_BD_ADDR(BLE_Demo_Context.ConnectionBD_ADDR))
      {
         /* Attempt to disconnect the device.                           */
         if(!DisconnectLEDevice(BLE_Demo_Context.BluetoothStackID, BLE_Demo_Context.ConnectionBD_ADDR))
            ret_val = QCLI_STATUS_SUCCESS_E;
         else
            ret_val = QCLI_STATUS_ERROR_E;
      }
      else
      {
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Device is not connected.\n");

         ret_val = QCLI_STATUS_SUCCESS_E;
      }
   }
   else
      ret_val = QCLI_STATUS_ERROR_E;

   return(ret_val);
}

   /* The following function is responsible for canceling an active LE  */
   /* connection process to an LE device.  This function returns zero if*/
   /* successful and a negative value if an error occurred.             */
static QCLI_Command_Status_t CancelConnectLE(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   int                   Result;
   QCLI_Command_Status_t ret_val;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BLE_Demo_Context.BluetoothStackID)
   {
      /* Attempt to cancel the connection process.                      */
      Result = qapi_BLE_GAP_LE_Cancel_Create_Connection(BLE_Demo_Context.BluetoothStackID);
      if(!Result)
         ret_val = QCLI_STATUS_SUCCESS_E;
      else
      {
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Unable to cancel LE connection process: %d.\n", Result);

         ret_val = QCLI_STATUS_ERROR_E;
      }
   }
   else
      ret_val = QCLI_STATUS_ERROR_E;

   return(ret_val);
}

   /* The following function is provided to allow a mechanism of        */
   /* Pairing (or requesting security if a slave) to the connected      */
   /* device.                                                           */
static QCLI_Command_Status_t PairLE(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t ret_val;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BLE_Demo_Context.BluetoothStackID)
   {
      /* Next, check to make sure we are currently connected.           */
      if(!QAPI_BLE_COMPARE_NULL_BD_ADDR(BLE_Demo_Context.ConnectionBD_ADDR))
      {
         /* Attempt to send the pairing request.                        */
         if(!SendPairingRequest(BLE_Demo_Context.ConnectionBD_ADDR, BLE_Demo_Context.LocalDeviceIsMaster))
            ret_val = QCLI_STATUS_SUCCESS_E;
         else
            ret_val = QCLI_STATUS_ERROR_E;
      }
      else
      {
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Device is not connected.\n");

         /* Flag success to the caller.                                 */
         ret_val = QCLI_STATUS_SUCCESS_E;
      }
   }
   else
      ret_val = QCLI_STATUS_ERROR_E;

   return(ret_val);
}

   /* The following function is responsible for performing a GAP Service*/
   /* Service Discovery Operation.  This function will return zero on   */
   /* successful execution and a negative value on errors.              */
static QCLI_Command_Status_t DiscoverGAPS(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   int                                     Result;
   DeviceInfo_t                           *DeviceInfo;
   qapi_BLE_GATT_UUID_t                    UUID[1];
   QCLI_Command_Status_t                   ret_val;
   qapi_BLE_GATT_Attribute_Handle_Group_t  DiscoveryHandleRange;

   /* Verify that there is a connection that is established.            */
   if(BLE_Demo_Context.ConnectionID)
   {
      /* Get the device info for the connection device.                 */
      if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&(BLE_Demo_Context.DeviceInfoList), BLE_Demo_Context.ConnectionBD_ADDR)) != NULL)
      {
         /* Verify that no service discovery is outstanding for this    */
         /* device.                                                     */
         if(!(DeviceInfo->Flags & DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_OUTSTANDING))
         {
            /* Configure the filter so that only the SPP LE Service is  */
            /* discovered.                                              */
            UUID[0].UUID_Type = QAPI_BLE_GU_UUID_16_E;
            QAPI_BLE_GAP_ASSIGN_GAP_SERVICE_UUID_16(UUID[0].UUID.UUID_16);

            memset(&DiscoveryHandleRange, 0, sizeof(DiscoveryHandleRange));

            /* Start the service discovery process.                     */
            if((Parameter_Count >= 2) && (Parameter_List) && (Parameter_List[0].Integer_Is_Valid) && (Parameter_List[0].Integer_Value) && (Parameter_List[1].Integer_Is_Valid) && (Parameter_List[1].Integer_Value) && (Parameter_List[0].Integer_Value <= Parameter_List[1].Integer_Value))
            {
               /* Configure the handle range.                           */
               DiscoveryHandleRange.Starting_Handle = Parameter_List[0].Integer_Value;
               DiscoveryHandleRange.Ending_Handle   = Parameter_List[1].Integer_Value;

               Result = qapi_BLE_GATT_Start_Service_Discovery_Handle_Range(BLE_Demo_Context.BluetoothStackID, BLE_Demo_Context.ConnectionID, &DiscoveryHandleRange, (sizeof(UUID)/sizeof(qapi_BLE_GATT_UUID_t)), UUID, GATT_Service_Discovery_Event_Callback, 0);
            }
            else
               Result = qapi_BLE_GATT_Start_Service_Discovery(BLE_Demo_Context.BluetoothStackID, BLE_Demo_Context.ConnectionID, (sizeof(UUID)/sizeof(qapi_BLE_GATT_UUID_t)), UUID, GATT_Service_Discovery_Event_Callback, 0);

            if(!Result)
            {
               /* Display success message.                              */
               if(DiscoveryHandleRange.Starting_Handle == 0)
                  QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "qapi_BLE_GATT_Service_Discovery_Start() success.\n");
               else
                  QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "qapi_BLE_GATT_Start_Service_Discovery_Handle_Range() success.\n");

               /* Flag that a Service Discovery Operation is            */
               /* outstanding.                                          */
               DeviceInfo->Flags |= DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_OUTSTANDING;

               ret_val = QCLI_STATUS_SUCCESS_E;
            }
            else
            {
               /* An error occur so just clean-up.                      */
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Error - qapi_BLE_GATT_Service_Discovery_Start() returned %d.\n", Result);

               ret_val = QCLI_STATUS_ERROR_E;
            }
         }
         else
         {
            QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Service Discovery Operation Outstanding for Device.\n");

            ret_val = QCLI_STATUS_ERROR_E;
         }
      }
      else
      {
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "No Device Info.\n");

         ret_val = QCLI_STATUS_ERROR_E;
      }
   }
   else
   {
      QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "No Connection Established\n");

      ret_val = QCLI_STATUS_ERROR_E;
   }

   return(ret_val);
}

   /* The following function is responsible for reading the current     */
   /* Local Device Name.  This function will return zero on successful  */
   /* execution and a negative value on errors.                         */
static QCLI_Command_Status_t ReadLocalName(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   int                   Result;
   char                  NameBuffer[248+1];
   QCLI_Command_Status_t ret_val;

   /* Verify that the GAP Service is registered.                        */
   if(BLE_Demo_Context.GAPSInstanceID)
   {
      /* Initialize the Name Buffer to all zeros.                       */
      memset(NameBuffer, 0, sizeof(NameBuffer));

      /* Query the Local Name.                                          */
      Result = qapi_BLE_GAPS_Query_Device_Name(BLE_Demo_Context.BluetoothStackID, BLE_Demo_Context.GAPSInstanceID, NameBuffer);
      if(!Result)
      {
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Device Name: %s.\n", NameBuffer);

         ret_val = QCLI_STATUS_SUCCESS_E;
      }
      else
      {
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Error - qapi_BLE_GAPS_Query_Device_Name returned %d.\n", Result);

         ret_val = QCLI_STATUS_ERROR_E;
      }
   }
   else
   {
      QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "GAP Service not registered.\n");

      ret_val = QCLI_STATUS_ERROR_E;
   }

   return(ret_val);
}

   /* The following function is responsible for setting the current     */
   /* Local Device Name.  This function will return zero on successful  */
   /* execution and a negative value on errors.                         */
static QCLI_Command_Status_t SetLocalName(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   int                   Result;
   QCLI_Command_Status_t ret_val;

   /* Verify that the input parameters are semi-valid.                  */
   if((Parameter_Count > 0) && (Parameter_List) && (Parameter_List[0].String_Value) && (strlen((char *)(Parameter_List[0].String_Value)) > 0) && (strlen((char *)(Parameter_List[0].String_Value)) <= 248))
   {
      /* Verify that the GAP Service is registered.                     */
      if(BLE_Demo_Context.GAPSInstanceID)
      {
         /* Query the Local Name.                                       */
         Result = qapi_BLE_GAPS_Set_Device_Name(BLE_Demo_Context.BluetoothStackID, BLE_Demo_Context.GAPSInstanceID, (char *)(Parameter_List[0].String_Value));
         if(!Result)
         {
            QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "qapi_BLE_GAPS_Set_Device_Name success.\n");

            ret_val = QCLI_STATUS_SUCCESS_E;
         }
         else
         {
            QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Error - qapi_BLE_GAPS_Query_Device_Name returned %d.\n", Result);

            ret_val = QCLI_STATUS_ERROR_E;
         }
      }
      else
      {
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "GAP Service not registered.\n");

         ret_val = QCLI_STATUS_ERROR_E;
      }
   }
   else
   {
      ret_val = QCLI_STATUS_USAGE_E;
   }

   return(ret_val);
}

   /* The following function is responsible for reading the Device Name */
   /* for the currently connected remote device.  This function will    */
   /* return zero on successful execution and a negative value on       */
   /* errors.                                                           */
static QCLI_Command_Status_t ReadRemoteName(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   int                    Result;
   DeviceInfo_t          *DeviceInfo;
   QCLI_Command_Status_t  ret_val;

   /* Verify that there is a connection that is established.            */
   if(BLE_Demo_Context.ConnectionID)
   {
      /* Get the device info for the connection device.                 */
      if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&(BLE_Demo_Context.DeviceInfoList), BLE_Demo_Context.ConnectionBD_ADDR)) != NULL)
      {
         /* Verify that we discovered the Device Name Handle.           */
         if(DeviceInfo->GAPSClientInfo.DeviceNameHandle)
         {
            /* Attempt to read the remote device name.                  */
            Result = qapi_BLE_GATT_Read_Value_Request(BLE_Demo_Context.BluetoothStackID, BLE_Demo_Context.ConnectionID, DeviceInfo->GAPSClientInfo.DeviceNameHandle, GATT_ClientEventCallback_GAPS, (unsigned long)DeviceInfo->GAPSClientInfo.DeviceNameHandle);
            if(Result > 0)
            {
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Attempting to read Remote Device Name.\n");

               ret_val = QCLI_STATUS_SUCCESS_E;
            }
            else
            {
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Error - GATT_Read_Value_Request returned %d.\n", Result);

               ret_val = QCLI_STATUS_ERROR_E;
            }
         }
         else
         {
            QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "GAP Service Device Name Handle not discovered.\n");

            ret_val = QCLI_STATUS_ERROR_E;
         }
      }
      else
      {
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "No Device Info.\n");

         ret_val = QCLI_STATUS_ERROR_E;
      }
   }
   else
   {
      QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "No Connection Established\n");

      ret_val = QCLI_STATUS_ERROR_E;
   }

   return(ret_val);
}

   /* The following function is responsible for reading the Local Device*/
   /* Appearance value.  This function will return zero on successful   */
   /* execution and a negative value on errors.                         */
static QCLI_Command_Status_t ReadLocalAppearance(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   int                    Result;
   char                  *AppearanceString;
   uint16_t               Appearance;
   QCLI_Command_Status_t  ret_val;

   /* Verify that the GAP Service is registered.                        */
   if(BLE_Demo_Context.GAPSInstanceID)
   {
      /* Query the Local Name.                                          */
      Result = qapi_BLE_GAPS_Query_Device_Appearance(BLE_Demo_Context.BluetoothStackID, BLE_Demo_Context.GAPSInstanceID, &Appearance);
      if(!Result)
      {
         /* Map the Appearance to a String.                             */
         if(AppearanceToString(Appearance, &AppearanceString))
            QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Device Appearance: %s(%u).\n", AppearanceString, Appearance);
         else
            QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Device Appearance: Unknown(%u).\n", Appearance);

         ret_val = QCLI_STATUS_SUCCESS_E;
      }
      else
      {
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Error - qapi_BLE_GAPS_Query_Device_Appearance returned %d.\n", Result);

         ret_val = QCLI_STATUS_ERROR_E;
      }
   }
   else
   {
      QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "GAP Service not registered.\n");

      ret_val = QCLI_STATUS_ERROR_E;
   }

   return(ret_val);
}

   /* The following function is responsible for setting the Local Device*/
   /* Appearance value.  This function will return zero on successful   */
   /* execution and a negative value on errors.                         */
static QCLI_Command_Status_t SetLocalAppearance(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   int                   Result;
   uint16_t              Appearance;
   QCLI_Command_Status_t ret_val;

   /* Verify that the input parameters are semi-valid.                  */
   if((Parameter_Count > 0) && (Parameter_List) && (Parameter_List[0].Integer_Is_Valid) && (Parameter_List[0].Integer_Value >= 0) && (Parameter_List[0].Integer_Value < NUMBER_OF_APPEARANCE_MAPPINGS))
   {
      /* Verify that the GAP Service is registered.                     */
      if(BLE_Demo_Context.GAPSInstanceID)
      {
         /* Map the Appearance Index to the GAP Appearance Value.       */
         if(AppearanceIndexToAppearance(Parameter_List[0].Integer_Value, &Appearance))
         {
            /* Set the Local Appearance.                                */
            Result = qapi_BLE_GAPS_Set_Device_Appearance(BLE_Demo_Context.BluetoothStackID, BLE_Demo_Context.GAPSInstanceID, Appearance);
            if(!Result)
            {
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "qapi_BLE_GAPS_Set_Device_Appearance success.\n");

               ret_val = QCLI_STATUS_SUCCESS_E;
            }
            else
            {
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Error - qapi_BLE_GAPS_Set_Device_Appearance returned %d.\n", Result);

               ret_val = QCLI_STATUS_ERROR_E;
            }
         }
         else
         {
            QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Invalid Appearance Index.\n");

            ret_val = QCLI_STATUS_ERROR_E;
         }
      }
      else
      {
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "GAP Service not registered.\n");

         ret_val = QCLI_STATUS_ERROR_E;
      }
   }
   else
   {
      QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Appearance Index = \n");
      DumpAppearanceMappings();

      ret_val = QCLI_STATUS_ERROR_E;
   }

   return(ret_val);
}

   /* The following function is responsible for reading the Device Name */
   /* for the currently connected remote device.  This function will    */
   /* return zero on successful execution and a negative value on       */
   /* errors.                                                           */
static QCLI_Command_Status_t ReadRemoteAppearance(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   int                    Result;
   DeviceInfo_t          *DeviceInfo;
   QCLI_Command_Status_t  ret_val;

   /* Verify that there is a connection that is established.            */
   if(BLE_Demo_Context.ConnectionID)
   {
      /* Get the device info for the connection device.                 */
      if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&(BLE_Demo_Context.DeviceInfoList), BLE_Demo_Context.ConnectionBD_ADDR)) != NULL)
      {
         /* Verify that we discovered the Device Name Handle.           */
         if(DeviceInfo->GAPSClientInfo.DeviceAppearanceHandle)
         {
            /* Attempt to read the remote device name.                  */
            Result = qapi_BLE_GATT_Read_Value_Request(BLE_Demo_Context.BluetoothStackID, BLE_Demo_Context.ConnectionID, DeviceInfo->GAPSClientInfo.DeviceAppearanceHandle, GATT_ClientEventCallback_GAPS, (unsigned long)DeviceInfo->GAPSClientInfo.DeviceAppearanceHandle);
            if(Result > 0)
            {
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Attempting to read Remote Device Appearance.\n");

               ret_val = QCLI_STATUS_SUCCESS_E;
            }
            else
            {
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Error - GATT_Read_Value_Request returned %d.\n", Result);

               ret_val = QCLI_STATUS_ERROR_E;
            }
         }
         else
         {
            QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "GAP Service Device Appearance Handle not discovered.\n");

            ret_val = QCLI_STATUS_ERROR_E;
         }
      }
      else
      {
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "No Device Info.\n");

         ret_val = QCLI_STATUS_ERROR_E;
      }
   }
   else
   {
      QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "No Connection Established\n");

      ret_val = QCLI_STATUS_ERROR_E;
   }

   return(ret_val);
}

   /* The following function is responsible for querying the GATT MTU.  */
   /* This function returns zero if successful and a negative value if  */
   /* an error occurred.                                                */
static QCLI_Command_Status_t GetGATTMTU(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   int                   Result;
   uint16_t              MTU;
   QCLI_Command_Status_t ret_val;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BLE_Demo_Context.BluetoothStackID)
   {
      /* Simply query the Maximum Supported MTU from the GATT layer.    */
      if((Result = qapi_BLE_GATT_Query_Maximum_Supported_MTU(BLE_Demo_Context.BluetoothStackID, &MTU)) == 0)
      {
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Maximum GATT MTU: %u.\n", (unsigned int)MTU);

         ret_val = QCLI_STATUS_SUCCESS_E;
      }
      else
      {
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Error - qapi_BLE_GATT_Query_Maximum_Supported_MTU() %d.\n", Result);

         ret_val = QCLI_STATUS_ERROR_E;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = QCLI_STATUS_ERROR_E;
   }

   return(ret_val);
}

   /* The following function is responsible for setting the GATT MTU.   */
   /* This function returns zero if successful and a negative value if  */
   /* an error occurred.                                                */
static QCLI_Command_Status_t SetGATTMTU(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   int                   Result;
   QCLI_Command_Status_t ret_val;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BLE_Demo_Context.BluetoothStackID)
   {
      /* Bluetooth Stack is initialized, go ahead and check to see if   */
      /* the parameters are valid.                                      */
      if((Parameter_Count > 0) && (Parameter_List) && (Parameter_List[0].Integer_Is_Valid) && (Parameter_List[0].Integer_Value >= QAPI_BLE_ATT_PROTOCOL_MTU_MINIMUM_LE) && (Parameter_List[0].Integer_Value <= QAPI_BLE_ATT_PROTOCOL_MTU_MAXIMUM))
      {
         /* Simply set the Maximum Supported MTU to the GATT layer.     */
         if((Result = qapi_BLE_GATT_Change_Maximum_Supported_MTU(BLE_Demo_Context.BluetoothStackID, (uint16_t)Parameter_List[0].Integer_Value)) == 0)
         {
            QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "qapi_BLE_GATT_Change_Maximum_Supported_MTU() success, new GATT Maximum Supported MTU: %u.\n", (unsigned int)Parameter_List[0].Integer_Value);

            ret_val = QCLI_STATUS_SUCCESS_E;
         }
         else
         {
            QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Error - qapi_BLE_GATT_Change_Maximum_Supported_MTU() %d.\n", Result);

            ret_val = QCLI_STATUS_ERROR_E;
         }
      }
      else
      {
         /* Flag that an error occurred while submitting the command.   */
         ret_val = QCLI_STATUS_USAGE_E;
      }
   }
   else
      ret_val = QCLI_STATUS_ERROR_E;

   return(ret_val);
}

   /* The following function is provided to get the parameters used when*/
   /* performing a BLE Scan Procedure (or outgoing connection).  This   */
   /* function returns QCLI_STATUS_SUCCESS_E on success or an error     */
   /* enumeration if an error occurs.                                   */
static QCLI_Command_Status_t GetScanParameters(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t ret_val;

   /* First check to see if the parameters required for the execution of*/
   /* this function appear to be semi-valid.                            */
   if(BLE_Demo_Context.BluetoothStackID)
   {
      /* Check to see if the scan parameters are valid.                 */
      if(BLE_Demo_Context.BLEParameters.Flags & BLE_PARAMETERS_FLAGS_SCAN_PARAMETERS_VALID)
      {
         /* Print the new parameters.                                   */
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Scan Window  : %u ms.\n", (unsigned int)BLE_Demo_Context.BLEParameters.ScanParameters.ScanWindow);
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Scan Interval: %u ms.\n", (unsigned int)BLE_Demo_Context.BLEParameters.ScanParameters.ScanInterval);
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Scan Type    : %s.\n", (BLE_Demo_Context.BLEParameters.EnableActiveScanning?"Active":"Passive"));
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Filter Dupls : %s.\n", (BLE_Demo_Context.BLEParameters.ScanningFilterDuplicates ? "Yes" : "No"));
      }
      else
      {
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Scan Parameters have not been configured.\n");
      }

      /* Flag that the function was successful.                         */
      ret_val = QCLI_STATUS_SUCCESS_E;
   }
   else
      ret_val = QCLI_STATUS_ERROR_E;

   return(ret_val);
}

   /* The following function is provided to set the parameters used when*/
   /* performing a BLE Scan Procedure (or outgoing connection).  This   */
   /* function returns QCLI_STATUS_SUCCESS_E on success or an error     */
   /* enumeration if an error occurs.                                   */
static QCLI_Command_Status_t SetScanParameters(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   uint16_t              ScanInterval;
   uint16_t              ScanWindow;
   boolean_t             EnableActiveScanning;
   boolean_t             ScanningFilterDuplicates;
   QCLI_Command_Status_t ret_val;

   /* First check to see if the parameters required for the execution of*/
   /* this function appear to be semi-valid.                            */
   if(BLE_Demo_Context.BluetoothStackID)
   {
      /* Next check to see if the parameters required for the execution */
      /* of this function appear to be semi-valid.                      */
      if((Parameter_Count >= 4) && (Parameter_List) && (Parameter_List[0].Integer_Is_Valid) && (Parameter_List[1].Integer_Is_Valid) && (Parameter_List[2].Integer_Is_Valid) && (Parameter_List[3].Integer_Is_Valid))
      {
         /* Assign the parameters into temporary variables.             */
         ScanInterval             = (uint16_t)Parameter_List[0].Integer_Value;
         ScanWindow               = (uint16_t)Parameter_List[1].Integer_Value;
         EnableActiveScanning     = (boolean_t)Parameter_List[2].Integer_Value;
         ScanningFilterDuplicates = (boolean_t)Parameter_List[3].Integer_Value;

         /* Next make sure that the Scan Window is less than the Scan   */
         /* Interval.                                                   */
         if(ScanWindow <= ScanInterval)
         {
            /* Next verify the Scan Window against the min and max in   */
            /* the specification.                                       */
            if((ScanWindow >= QAPI_BLE_MINIMUM_LE_SCAN_WINDOW) && (ScanWindow <= QAPI_BLE_MAXIMUM_LE_SCAN_WINDOW))
            {
               /* Next verify the Scan Interval against the min and max */
               /* in the specification.                                 */
               if((ScanInterval >= QAPI_BLE_MINIMUM_LE_SCAN_INTERVAL) && (ScanInterval <= QAPI_BLE_MAXIMUM_LE_SCAN_INTERVAL))
               {
                  /* Parameters are valid so store them.                */
                  BLE_Demo_Context.BLEParameters.ScanParameters.ScanInterval = ScanInterval;
                  BLE_Demo_Context.BLEParameters.ScanParameters.ScanWindow   = ScanWindow;
                  BLE_Demo_Context.BLEParameters.EnableActiveScanning        = EnableActiveScanning;
                  BLE_Demo_Context.BLEParameters.ScanningFilterDuplicates    = ScanningFilterDuplicates;

                  /* Flag that the parameters are valid.                */
                  BLE_Demo_Context.BLEParameters.Flags |= BLE_PARAMETERS_FLAGS_SCAN_PARAMETERS_VALID;

                  /* Print the new parameters.                          */
                  QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Scan Window  : %u ms.\n", (unsigned int)BLE_Demo_Context.BLEParameters.ScanParameters.ScanWindow);
                  QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Scan Interval: %u ms.\n", (unsigned int)BLE_Demo_Context.BLEParameters.ScanParameters.ScanInterval);
                  QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Scan Type    : %s.\n", (BLE_Demo_Context.BLEParameters.EnableActiveScanning?"Active":"Passive"));
                  QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Filter Dupls : %s.\n", (BLE_Demo_Context.BLEParameters.ScanningFilterDuplicates ? "Yes" : "No"));

                  /* Flag that the function was successful.             */
                  ret_val = QCLI_STATUS_SUCCESS_E;
               }
               else
               {
                  QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Scan Interval (%u ms) MUST be in range [%u, %u] ms.\n", (unsigned int)ScanInterval, (unsigned int)QAPI_BLE_MINIMUM_LE_SCAN_INTERVAL, (unsigned int)QAPI_BLE_MAXIMUM_LE_SCAN_INTERVAL);

                  ret_val = QCLI_STATUS_ERROR_E;
               }
            }
            else
            {
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Scan Window (%u ms) MUST be in range [%u, %u] ms.\n", (unsigned int)ScanWindow, (unsigned int)QAPI_BLE_MINIMUM_LE_SCAN_WINDOW, (unsigned int)QAPI_BLE_MAXIMUM_LE_SCAN_WINDOW);

               ret_val = QCLI_STATUS_ERROR_E;
            }
         }
         else
         {
            QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Scan Window (%u ms) MUST be less than or equal to Scan Interval (%u ms).\n", (unsigned int)ScanWindow, (unsigned int)ScanInterval);

            ret_val = QCLI_STATUS_ERROR_E;
         }
      }
      else
         ret_val = QCLI_STATUS_USAGE_E;
   }
   else
      ret_val = QCLI_STATUS_ERROR_E;

   return(ret_val);
}

   /* The following function is provided to query the parameters used   */
   /* when performing a BLE Advertising Procedure.  This function       */
   /* returns QCLI_STATUS_SUCCESS_E on success or an error enumeration  */
   /* if an error occurs.                                               */
static QCLI_Command_Status_t GetAdvertisingParameters(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t ret_val;

   /* First check to see if the parameters required for the execution of*/
   /* this function appear to be semi-valid.                            */
   if(BLE_Demo_Context.BluetoothStackID)
   {
      /* Check to see if the scan parameters are valid.                 */
      if(BLE_Demo_Context.BLEParameters.Flags & BLE_PARAMETERS_FLAGS_ADVERTISING_PARAMETERS_VALID)
      {
         /* Print the new parameters.                                   */
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Minimum Advertising Interval: %u ms.\n", (unsigned int)BLE_Demo_Context.BLEParameters.AdvertisingParameters.Advertising_Interval_Min);
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Maximum Advertising Interval: %u ms.\n", (unsigned int)BLE_Demo_Context.BLEParameters.AdvertisingParameters.Advertising_Interval_Max);
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Scan Response Enabled       : %s.\n",    (BLE_Demo_Context.BLEParameters.EnableAdvScanResponse?"Enabled":"Not Enabled"));
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Scan Requests White Listed  : %s.\n",    (BLE_Demo_Context.BLEParameters.AdvertisingParameters.Scan_Request_Filter == QAPI_BLE_FP_WHITE_LIST_E)    ? "Enabled" : "Not Enabled");
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Connect Request White Listed: %s.\n",    (BLE_Demo_Context.BLEParameters.AdvertisingParameters.Connect_Request_Filter == QAPI_BLE_FP_WHITE_LIST_E) ? "Enabled" : "Not Enabled");
      }
      else
      {
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Advertising Parameters have not been configured.\n");
      }

      /* Flag that the function was successful.                         */
      ret_val = QCLI_STATUS_SUCCESS_E;
   }
   else
      ret_val = QCLI_STATUS_ERROR_E;

   return(ret_val);
}

   /* The following function is provided to set the parameters used when*/
   /* performing a BLE Advertising Procedure.  This function returns    */
   /* QCLI_STATUS_SUCCESS_E on success or an error enumeration if an    */
   /* error occurs.                                                     */
static QCLI_Command_Status_t SetAdvertisingParameters(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   uint16_t              AdvIntervalMin;
   uint16_t              AdvIntervalMax;
   boolean_t             EnableAdvScanResponse;
   boolean_t             EnableWhiteListFilter;
   QCLI_Command_Status_t ret_val;

   /* First check to see if the parameters required for the execution of*/
   /* this function appear to be semi-valid.                            */
   if(BLE_Demo_Context.BluetoothStackID)
   {
      /* Next check to see if the parameters required for the execution */
      /* of this function appear to be semi-valid.                      */
      if((Parameter_Count >= 3) && (Parameter_List) && (Parameter_List[0].Integer_Is_Valid) && (Parameter_List[1].Integer_Is_Valid) && (Parameter_List[2].Integer_Is_Valid))
      {
         /* Assign the parameters into temporary variables.             */
         AdvIntervalMin        = (uint16_t)Parameter_List[0].Integer_Value;
         AdvIntervalMax        = (uint16_t)Parameter_List[1].Integer_Value;
         EnableAdvScanResponse = (boolean_t)Parameter_List[2].Integer_Value;

         /* Check to see if the optional parameter is valid.            */
         if((Parameter_Count >= 4) && (Parameter_List[3].Integer_Is_Valid) && ((Parameter_List[3].Integer_Value == 1) || (Parameter_List[3].Integer_Value == 0)))
            EnableWhiteListFilter = (boolean_t)Parameter_List[3].Integer_Value;
         else
            EnableWhiteListFilter = false;

         /* Next make sure that the Min is less than the Max.           */
         if(AdvIntervalMin <= AdvIntervalMax)
         {
            /* Next verify the Minimum Advertising Interval against the */
            /* min and max in the specification.                        */
            if((AdvIntervalMin >= QAPI_BLE_MINIMUM_ADVERTISING_INTERVAL) && (AdvIntervalMin <= QAPI_BLE_MAXIMUM_ADVERTISING_INTERVAL))
            {
               /* Next verify the Maximum Advertising Interval against  */
               /* the min and max in the specification.                 */
               if((AdvIntervalMax >= QAPI_BLE_MINIMUM_ADVERTISING_INTERVAL) && (AdvIntervalMax <= QAPI_BLE_MAXIMUM_ADVERTISING_INTERVAL))
               {
                  /* Configure the advertising parameters.              */
                  BLE_Demo_Context.BLEParameters.AdvertisingParameters.Advertising_Channel_Map   = QAPI_BLE_HCI_LE_ADVERTISING_CHANNEL_MAP_DEFAULT;
                  BLE_Demo_Context.BLEParameters.AdvertisingParameters.Scan_Request_Filter       = (EnableWhiteListFilter ? QAPI_BLE_FP_WHITE_LIST_E : QAPI_BLE_FP_NO_FILTER_E);
                  BLE_Demo_Context.BLEParameters.AdvertisingParameters.Connect_Request_Filter    = (EnableWhiteListFilter ? QAPI_BLE_FP_WHITE_LIST_E : QAPI_BLE_FP_NO_FILTER_E);
                  BLE_Demo_Context.BLEParameters.AdvertisingParameters.Advertising_Interval_Min  = AdvIntervalMin;
                  BLE_Demo_Context.BLEParameters.AdvertisingParameters.Advertising_Interval_Max  = AdvIntervalMax;
                  BLE_Demo_Context.BLEParameters.EnableAdvScanResponse                           = EnableAdvScanResponse;

#ifdef V2

                  /* Configure the wake on Scan Request based on white  */
                  /* list state.                                        */
                  qapi_BLE_GAP_LE_Wake_On_Scan_Request(BLE_Demo_Context.BluetoothStackID, EnableWhiteListFilter);

#endif

                  /* Check to see if we should only advertise on Channel*/
                  /* 38.                                                */
                  if((Parameter_Count >= 5) && (Parameter_List[4].Integer_Is_Valid) && (Parameter_List[4].Integer_Value != 0))
                  {
                     /* If enabled advertise only on Channel 38.        */
                     BLE_Demo_Context.BLEParameters.AdvertisingParameters.Advertising_Channel_Map = QAPI_BLE_HCI_LE_ADVERTISING_CHANNEL_MAP_ENABLE_CHANNEL_38;
                  }

                  /* Flag that the parameters are valid.                */
                  BLE_Demo_Context.BLEParameters.Flags |= BLE_PARAMETERS_FLAGS_ADVERTISING_PARAMETERS_VALID;

                  /* Print the new parameters.                          */
                  QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Minimum Advertising Interval: %u ms.\n", (unsigned int)BLE_Demo_Context.BLEParameters.AdvertisingParameters.Advertising_Interval_Min);
                  QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Maximum Advertising Interval: %u ms.\n", (unsigned int)BLE_Demo_Context.BLEParameters.AdvertisingParameters.Advertising_Interval_Max);
                  QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Scan Response Enabled       : %s.\n",    (BLE_Demo_Context.BLEParameters.EnableAdvScanResponse?"Enabled":"Not Enabled"));
                  QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Scan Requests White Listed  : %s.\n",    (BLE_Demo_Context.BLEParameters.AdvertisingParameters.Scan_Request_Filter == QAPI_BLE_FP_WHITE_LIST_E)    ? "Enabled" : "Not Enabled");
                  QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Connect Request White Listed: %s.\n",    (BLE_Demo_Context.BLEParameters.AdvertisingParameters.Connect_Request_Filter == QAPI_BLE_FP_WHITE_LIST_E) ? "Enabled" : "Not Enabled");

                  /* Flag that the function was successful.             */
                  ret_val = QCLI_STATUS_SUCCESS_E;
               }
               else
               {
                  QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Advertising Interval Max (%u ms) MUST be in range [%u, %u] ms.\n", (unsigned int)AdvIntervalMax, (unsigned int)QAPI_BLE_MINIMUM_ADVERTISING_INTERVAL, (unsigned int)QAPI_BLE_MAXIMUM_ADVERTISING_INTERVAL);

                  ret_val = QCLI_STATUS_ERROR_E;
               }
            }
            else
            {
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Advertising Interval Min (%u ms) MUST be in range [%u, %u] ms.\n", (unsigned int)AdvIntervalMin, (unsigned int)QAPI_BLE_MINIMUM_ADVERTISING_INTERVAL, (unsigned int)QAPI_BLE_MAXIMUM_ADVERTISING_INTERVAL);

               ret_val = QCLI_STATUS_ERROR_E;
            }
         }
         else
         {
            QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Minimum Advertising Interval (%u ms) MUST be less than or equal to the Maximum Advertising Interval (%u ms).\n", (unsigned int)AdvIntervalMin, (unsigned int)AdvIntervalMax);

            ret_val = QCLI_STATUS_ERROR_E;
         }
      }
      else
         ret_val = QCLI_STATUS_USAGE_E;
   }
   else
      ret_val = QCLI_STATUS_ERROR_E;

   return(ret_val);
}

   /* The following function sets the length of data to advertise for   */
   /* testing purposes.  This function returns QCLI_STATUS_SUCCESS_E on */
   /* success or an error enumeration if an error occurs.               */
static QCLI_Command_Status_t SetTestAdvertisingDataLength(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t ret_val;

   /* Next check to see if the parameters required for the execution of */
   /* this function appear to be semi-valid.                            */
   if((Parameter_Count >= 1) && (Parameter_List) && (Parameter_List[0].Integer_Is_Valid))
   {
      /* Verify the data length.                                        */
      if((Parameter_List[0].Integer_Value >= 3) && (Parameter_List[0].Integer_Value <= QAPI_BLE_ADVERTISING_DATA_MAXIMUM_SIZE))
      {
         /* Set the max advertising data length.                        */
         BLE_Demo_Context.TestAdvertisingDataLength = Parameter_List[0].Integer_Value;

         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Advertising Data Length : %u.\n", (unsigned int)BLE_Demo_Context.TestAdvertisingDataLength);

         ret_val = QCLI_STATUS_SUCCESS_E;
      }
      else
      {
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Test Data Length must be between 3 - %u.\n", (unsigned int)QAPI_BLE_ADVERTISING_DATA_MAXIMUM_SIZE);

         ret_val = QCLI_STATUS_ERROR_E;
      }
   }
   else
      ret_val = QCLI_STATUS_USAGE_E;

   return(ret_val);
}

   /* The following function is provided to query the parameters used   */
   /* when performing a BLE Connection Procedure.  This function returns*/
   /* QCLI_STATUS_SUCCESS_E on success or an error enumeration if an    */
   /* error occurs.                                                     */
static QCLI_Command_Status_t GetConnectionParameters(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t ret_val;

   /* First check to see if the parameters required for the execution of*/
   /* this function appear to be semi-valid.                            */
   if(BLE_Demo_Context.BluetoothStackID)
   {
      /* Check to see if the scan parameters are valid.                 */
      if(BLE_Demo_Context.BLEParameters.Flags & BLE_PARAMETERS_FLAGS_CONNECTION_PARAMETERS_VALID)
      {
         /* Print the new parameters.                                   */
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Minimum Connection Interval: %u ms.\n", (unsigned int)BLE_Demo_Context.BLEParameters.ConnectionParameters.Connection_Interval_Min);
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Maximum Connection Interval: %u ms.\n", (unsigned int)BLE_Demo_Context.BLEParameters.ConnectionParameters.Connection_Interval_Max);
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Slave Latency:               %u connection events.\n", (unsigned int)BLE_Demo_Context.BLEParameters.ConnectionParameters.Slave_Latency);
      }
      else
      {
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Connection Parameters have not been configured.\n");
      }

      /* Flag that the function was successful.                         */
      ret_val = QCLI_STATUS_SUCCESS_E;
   }
   else
      ret_val = QCLI_STATUS_ERROR_E;

   return(ret_val);
}

   /* The following function is provided to set the parameters used when*/
   /* performing a BLE Connection Procedure.  This function returns     */
   /* QCLI_STATUS_SUCCESS_E on success or an error enumeration if an    */
   /* error occurs.                                                     */
static QCLI_Command_Status_t SetConnectionParameters(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   uint16_t              ConnIntervalMin;
   uint16_t              ConnIntervalMax;
   uint16_t              SlaveLatency;
   QCLI_Command_Status_t ret_val;

   /* First check to see if the parameters required for the execution of*/
   /* this function appear to be semi-valid.                            */
   if(BLE_Demo_Context.BluetoothStackID)
   {
      /* Next check to see if the parameters required for the execution */
      /* of this function appear to be semi-valid.                      */
      if((Parameter_Count >= 3) && (Parameter_List) && (Parameter_List[0].Integer_Is_Valid) && (Parameter_List[1].Integer_Is_Valid) && (Parameter_List[2].Integer_Is_Valid))
      {
         /* Assign the parameters into temporary variables.             */
         ConnIntervalMin = (uint16_t)Parameter_List[0].Integer_Value;
         ConnIntervalMax = (uint16_t)Parameter_List[1].Integer_Value;
         SlaveLatency    = (uint16_t)Parameter_List[2].Integer_Value;

         /* Next make sure that the Min is less than the Max.           */
         if(ConnIntervalMin <= ConnIntervalMax)
         {
            /* Next verify the Minimum Connection Interval against the  */
            /* min and max in the specification.                        */
            if((ConnIntervalMin >= QAPI_BLE_MINIMUM_MINIMUM_CONNECTION_INTERVAL) && (ConnIntervalMin <= QAPI_BLE_MAXIMUM_MINIMUM_CONNECTION_INTERVAL))
            {
               /* Next verify the Maximum Connection Interval against   */
               /* the min and max in the specification.                 */
               if((ConnIntervalMax >= QAPI_BLE_MINIMUM_MAXIMUM_CONNECTION_INTERVAL) && (ConnIntervalMax <= QAPI_BLE_MAXIMUM_MAXIMUM_CONNECTION_INTERVAL))
               {
                  /* Finally verify that the Slave Latency value is     */
                  /* valid.                                             */
                  if(SlaveLatency <= QAPI_BLE_MAXIMUM_SLAVE_LATENCY)
                  {
                     /* Configure the Connection parameters.            */
                     BLE_Demo_Context.BLEParameters.ConnectionParameters.Connection_Interval_Min    = ConnIntervalMin;
                     BLE_Demo_Context.BLEParameters.ConnectionParameters.Connection_Interval_Max    = ConnIntervalMax;
                     BLE_Demo_Context.BLEParameters.ConnectionParameters.Minimum_Connection_Length  = 0;
                     BLE_Demo_Context.BLEParameters.ConnectionParameters.Maximum_Connection_Length  = 10000;
                     BLE_Demo_Context.BLEParameters.ConnectionParameters.Slave_Latency              = SlaveLatency;
                     BLE_Demo_Context.BLEParameters.ConnectionParameters.Supervision_Timeout        = 20000;

                     /* Flag that the parameters are valid.             */
                     BLE_Demo_Context.BLEParameters.Flags |= BLE_PARAMETERS_FLAGS_CONNECTION_PARAMETERS_VALID;

                     /* Print the new parameters.                       */
                     QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Minimum Connection Interval: %u ms.\n", (unsigned int)BLE_Demo_Context.BLEParameters.ConnectionParameters.Connection_Interval_Min);
                     QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Maximum Connection Interval: %u ms.\n", (unsigned int)BLE_Demo_Context.BLEParameters.ConnectionParameters.Connection_Interval_Max);
                     QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Slave Latency:               %u connection events.\n", (unsigned int)BLE_Demo_Context.BLEParameters.ConnectionParameters.Slave_Latency);

                     /* Flag that the function was successful.          */
                     ret_val = QCLI_STATUS_SUCCESS_E;
                  }
                  else
                  {
                     QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Slave Latency (%u connection events) MUST be in range [%u, %u] connection events.\n", (unsigned int)SlaveLatency, (unsigned int)QAPI_BLE_MINIMUM_SLAVE_LATENCY, (unsigned int)QAPI_BLE_MAXIMUM_SLAVE_LATENCY);

                     ret_val = QCLI_STATUS_ERROR_E;
                  }
               }
               else
               {
                  QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Connection Interval Max (%u ms) MUST be in range [%u, %u] ms.\n", (unsigned int)ConnIntervalMax, (unsigned int)QAPI_BLE_MINIMUM_MAXIMUM_CONNECTION_INTERVAL, (unsigned int)QAPI_BLE_MAXIMUM_MAXIMUM_CONNECTION_INTERVAL);

                  ret_val = QCLI_STATUS_ERROR_E;
               }
            }
            else
            {
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Connection Interval Min (%u ms) MUST be in range [%u, %u] ms.\n", (unsigned int)ConnIntervalMin, (unsigned int)QAPI_BLE_MINIMUM_MINIMUM_CONNECTION_INTERVAL, (unsigned int)QAPI_BLE_MAXIMUM_MINIMUM_CONNECTION_INTERVAL);

               ret_val = QCLI_STATUS_ERROR_E;
            }
         }
         else
         {
            QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Minimum Connection Interval (%u ms) MUST be less than or equal to the Maximum Connection Interval (%u ms).\n", (unsigned int)ConnIntervalMin, (unsigned int)ConnIntervalMax);

            ret_val = QCLI_STATUS_ERROR_E;
         }
      }
      else
         ret_val = QCLI_STATUS_USAGE_E;
   }
   else
      ret_val = QCLI_STATUS_ERROR_E;

   return(ret_val);
}

   /* The following function is provided to allow the user to set the   */
   /* default packet size that is used (this allows DLE to be enabled). */
   /* This function returns QCLI_STATUS_SUCCESS_E on success or an error*/
   /* enumeration if an error occurs.                                   */
static QCLI_Command_Status_t SetDefaultPacketSize(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   int                   Result;
   QCLI_Command_Status_t ret_val;

   /* Next check to see if the parameters required for the execution of */
   /* this function appear to be semi-valid.                            */
   if((Parameter_Count >= 1) && (Parameter_List) && (Parameter_List[0].Integer_Is_Valid))
   {
      if((Parameter_List[0].Integer_Value >= 27) && (Parameter_List[0].Integer_Value <= ((MaxACLPacketSize > 251)?251:MaxACLPacketSize)))
      {
         /* Attempt to write the default data length.                   */
         Result = qapi_BLE_GAP_LE_Set_Default_Data_Length(BLE_Demo_Context.BluetoothStackID, Parameter_List[0].Integer_Value, (Parameter_List[0].Integer_Value + 4)*8 + 80);
         if(!Result)
         {
            QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Configured Default LE Data Packet Size: %u\n", Parameter_List[0].Integer_Value);

            ret_val = QCLI_STATUS_SUCCESS_E;
         }
         else
         {
            DisplayFunctionError("qapi_BLE_GAP_LE_Set_Default_Data_Length", Result);

            ret_val = QCLI_STATUS_ERROR_E;
         }
      }
      else
      {
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Data Packet Size must be between 27 and  %u.\n", ((MaxACLPacketSize > 251)?251:MaxACLPacketSize));

         ret_val = QCLI_STATUS_ERROR_E;
      }
   }
   else
      ret_val = QCLI_STATUS_USAGE_E;

   return(ret_val);
}

   /* The following function is provided to enable a wakeup period to   */
   /* wakeup for data transfer.  This function returns                  */
   /* QCLI_STATUS_SUCCESS_E on success or an error enumeration if an    */
   /* error occurs.                                                     */
static QCLI_Command_Status_t SetTestDataPeriod(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   uint16_t                 _ConnectionHandle;
   qapi_Status_t            Result;
   qapi_BLE_BD_ADDR_t       BD_ADDR;
   QCLI_Command_Status_t    ret_val;
   qapi_TIMER_set_attr_t    Set_Timer_Attr;
   qapi_TIMER_define_attr_t Create_Timer_Attr;

   /* Next check to see if the parameters required for the execution of */
   /* this function appear to be semi-valid.                            */
   if((Parameter_Count >= 2) && (Parameter_List) && (strlen((char *)(Parameter_List[0].String_Value)) == (sizeof(qapi_BLE_BD_ADDR_t) * 2)) && (Parameter_List[1].Integer_Is_Valid))
   {
      /* Convert the parameter to a Bluetooth Device Address.           */
      StrToBD_ADDR((char *)(Parameter_List[0].String_Value), &BD_ADDR);

      if((Parameter_Count == 2) || ((Parameter_Count >= 3) && (Parameter_List[2].Integer_Is_Valid) && (Parameter_List[2].Integer_Value >= 1) && (Parameter_List[2].Integer_Value <= MaxACLPacketSize)))
      {
         if(Parameter_Count == 2)
            PacketLength = 27;
         else
            PacketLength = Parameter_List[2].Integer_Value;

         if(!qapi_BLE_GAP_LE_Query_Connection_Handle(BLE_Demo_Context.BluetoothStackID, BD_ADDR, &_ConnectionHandle))
         {
            /* Check to see if we are starting or stopping the timer.   */
            if(Parameter_List[1].Integer_Value == 0)
            {
               /* See if the timer is started.                          */
               if(BLE_Demo_Context.TransmitPeriod != 0)
               {
                  /* Stop the timer.                                    */
                  qapi_Timer_Stop(BLE_Demo_Context.Timer);

                  /* Clean up the timer.                                */
                  qapi_Timer_Undef(BLE_Demo_Context.Timer);

                  BLE_Demo_Context.TransmitPeriod = 0;

                  QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Timer stopped.\n");
               }
               else
                  QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Timer already stopped.\n");

               CurrentTest      = CURRENT_TEST_NONE;
               ConnectionHandle = QAPI_BLE_HCI_CONNECTION_HANDLE_INVALID_VALUE;

               ret_val          = QCLI_STATUS_SUCCESS_E;
            }
            else
            {
               /* Verify that the timer isn't already started.          */
               if((BLE_Demo_Context.TransmitPeriod == 0) && (CurrentTest == CURRENT_TEST_NONE))
               {
                  /* Create the timer.                                  */
                  Create_Timer_Attr.deferrable     = false;
                  Create_Timer_Attr.cb_type        = QAPI_TIMER_FUNC1_CB_TYPE;
                  Create_Timer_Attr.sigs_func_ptr  = (void *)Period_Tx_Callback;
                  Create_Timer_Attr.sigs_mask_data = 0;
                  Result = qapi_Timer_Def(&(BLE_Demo_Context.Timer), &Create_Timer_Attr);

                  /* Verify that the timer was started successfully.    */
                  if(Result == QAPI_OK)
                  {
                     /* Start the timer for periodic transmissions.     */
                     Set_Timer_Attr.time                   = (uint64_t)Parameter_List[1].Integer_Value;
                     Set_Timer_Attr.reload                 = true;
                     Set_Timer_Attr.max_deferrable_timeout = (uint64_t)Parameter_List[1].Integer_Value;
                     Set_Timer_Attr.unit                   = QAPI_TIMER_UNIT_MSEC;
                     Result = qapi_Timer_Set(BLE_Demo_Context.Timer, &Set_Timer_Attr);

                     /* Start the timer.                                */
                     if(Result == QAPI_OK)
                     {
                        /* Set the timer period.                        */
                        BLE_Demo_Context.TransmitPeriod = Parameter_List[1].Integer_Value;

                        CurrentTest                 = CURRENT_TEST_PERIODIC;
                        StartTime                   = qurt_timer_get_ticks();
                        NumberBytes                 = 0;
                        NumberOutstandingACLPackets = 0;

                        /* Note the connection Handle.                  */
                        ConnectionHandle            = _ConnectionHandle;

                        QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Timer started with period %u.\n", (unsigned int)BLE_Demo_Context.TransmitPeriod);

                        QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Starting ACL Periodic TX Test, packet size: %u.\n", PacketLength);

                        ret_val = QCLI_STATUS_SUCCESS_E;
                     }
                     else
                     {
                        /* Clean up the timer.                          */
                        qapi_Timer_Undef(BLE_Demo_Context.Timer);

                        Display_Function_Error(BLE_Demo_Context.QCLI_Handle, "qapi_Timer_Set", Result);

                        ret_val = QCLI_STATUS_ERROR_E;
                     }
                  }
                  else
                  {
                     Display_Function_Error(BLE_Demo_Context.QCLI_Handle, "qapi_Timer_Def", Result);

                     ret_val = QCLI_STATUS_ERROR_E;
                  }
               }
               else
               {
                  if(BLE_Demo_Context.TransmitPeriod != 0)
                     QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Timer already started with period %u.\n", (unsigned int)BLE_Demo_Context.TransmitPeriod);
                  else
                     QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Test already in progress.\n");

                  ret_val = QCLI_STATUS_ERROR_E;
               }
            }
         }
         else
         {
            QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Device is not currently connected.\n");

            ret_val = QCLI_STATUS_ERROR_E;
         }
      }
      else
      {
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Data Packet Size must be between 1 and  %u.\n", MaxACLPacketSize);

         ret_val = QCLI_STATUS_ERROR_E;
      }
   }
   else
      ret_val = QCLI_STATUS_USAGE_E;

   return(ret_val);
}

   /* The following function is responsible for adding a remote device  */
   /* to the white list in the controller.  This function returns zero  */
   /* if successful and a negative value if an error occurred.          */
   /* * NOTE * If a remote device has been successfully added to the    */
   /*          white list in the controller, then the white list may be */
   /*          used when scanning or connecting to the remote device.   */
static QCLI_Command_Status_t AddDeviceToWhiteList(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   uint32_t                           AddedDeviceCount;
   qapi_BLE_BD_ADDR_t                 BD_ADDR;
   QCLI_Command_Status_t              ret_val;
   qapi_BLE_GAP_LE_White_List_Entry_t WhiteListEntry;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BLE_Demo_Context.BluetoothStackID)
   {
      /* Verify that the input parameters are semi-valid.               */
      if((Parameter_Count >= 2) && (strlen((char *)(Parameter_List[0].String_Value)) == (sizeof(qapi_BLE_BD_ADDR_t) * 2)) && (Parameter_List[1].Integer_Is_Valid) && (Parameter_List[1].Integer_Value >= (int32_t)QAPI_BLE_LAT_PUBLIC_E) && (Parameter_List[1].Integer_Value <= (int32_t)QAPI_BLE_LAT_RANDOM_IDENTITY_E))
      {
         /* Convert the parameter to a Bluetooth Device Address.        */
         StrToBD_ADDR((char *)(Parameter_List[0].String_Value), &BD_ADDR);

         /* Lock the Bluetooth stack.                                   */
         if(!qapi_BLE_BSC_LockBluetoothStack(BLE_Demo_Context.BluetoothStackID))
         {
            /* Format the white list entry.                             */
            WhiteListEntry.Address      = BD_ADDR;
            WhiteListEntry.Address_Type = (qapi_BLE_GAP_LE_Address_Type_t)Parameter_List[1].Integer_Value;

            /* Let's add the device to the white list in the controller.*/
            /* * NOTE * We will only add one device, however we         */
            /*          could add more with this function.              */
            ret_val = qapi_BLE_GAP_LE_Add_Device_To_White_List(BLE_Demo_Context.BluetoothStackID, 1, &WhiteListEntry, &AddedDeviceCount);
            if(!ret_val)
            {
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "qapi_BLE_GAP_LE_Add_Device_To_White_List() success.\n");
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   Added devices:  %u.\n", (unsigned int)AddedDeviceCount);

               ret_val = QCLI_STATUS_SUCCESS_E;
            }
            else
            {
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Error - qapi_BLE_GAP_LE_Add_Device_To_White_List() returned %d.\n", ret_val);

               ret_val = QCLI_STATUS_ERROR_E;
            }

            /* Un-lock the Bluetooth Stack.                             */
            qapi_BLE_BSC_UnLockBluetoothStack(BLE_Demo_Context.BluetoothStackID);
         }
         else
         {
            QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Unable to acquire Bluetooth Stack Lock.\n");

            ret_val = QCLI_STATUS_ERROR_E;
         }
      }
      else
         ret_val = QCLI_STATUS_USAGE_E;
   }
   else
      ret_val = QCLI_STATUS_ERROR_E;

   return(ret_val);
}

   /* The following function is responsible for removing a remote device*/
   /* from the white list in the controller.  This function returns zero*/
   /* if successful and a negative value if an error occurred.          */
static QCLI_Command_Status_t RemoveDeviceFromWhiteList(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   uint32_t                           RemovedDeviceCount;
   qapi_BLE_BD_ADDR_t                 BD_ADDR;
   QCLI_Command_Status_t              ret_val;
   qapi_BLE_GAP_LE_White_List_Entry_t WhiteListEntry;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BLE_Demo_Context.BluetoothStackID)
   {
      /* Verify that the input parameters are semi-valid.               */
      if((Parameter_Count >= 2) && (strlen((char *)(Parameter_List[0].String_Value)) == (sizeof(qapi_BLE_BD_ADDR_t) * 2)) && (Parameter_List[1].Integer_Is_Valid) && (Parameter_List[1].Integer_Value >= (int32_t)QAPI_BLE_LAT_PUBLIC_E) && (Parameter_List[1].Integer_Value <= (int32_t)QAPI_BLE_LAT_RANDOM_IDENTITY_E))
      {
         /* Convert the parameter to a Bluetooth Device Address.        */
         StrToBD_ADDR((char *)(Parameter_List[0].String_Value), &BD_ADDR);

         /* Lock the Bluetooth stack.                                   */
         if(!qapi_BLE_BSC_LockBluetoothStack(BLE_Demo_Context.BluetoothStackID))
         {
            /* Format the white list entry.                             */
            WhiteListEntry.Address      = BD_ADDR;
            WhiteListEntry.Address_Type = (qapi_BLE_GAP_LE_Address_Type_t)Parameter_List[1].Integer_Value;

            /* Let's remove the device from the white list in the       */
            /* controller.                                              */
            /* * NOTE * We will only remove one device, however we      */
            /*          could remove more with this function.           */
            ret_val = qapi_BLE_GAP_LE_Remove_Device_From_White_List(BLE_Demo_Context.BluetoothStackID, 1, &WhiteListEntry, &RemovedDeviceCount);

            if(!ret_val)
            {
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "qapi_BLE_GAP_LE_Remove_Device_From_White_List() success.\n");
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   Removed devices:  %u.\n", (unsigned int)RemovedDeviceCount);

               ret_val = QCLI_STATUS_SUCCESS_E;
            }
            else
            {
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Error - qapi_BLE_GAP_LE_Remove_Device_From_White_List() returned %d.\n", ret_val);

               ret_val = QCLI_STATUS_ERROR_E;
            }

            /* Un-lock the Bluetooth Stack.                             */
            qapi_BLE_BSC_UnLockBluetoothStack(BLE_Demo_Context.BluetoothStackID);
         }
         else
         {
            QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Unable to acquire Bluetooth Stack Lock.\n");

            ret_val = QCLI_STATUS_ERROR_E;
         }
      }
      else
         ret_val = QCLI_STATUS_USAGE_E;
   }
   else
      ret_val = QCLI_STATUS_ERROR_E;

   return(ret_val);
}

   /* The following function is responsible for adding a remote device  */
   /* to the resolving list in the controller.  This function returns   */
   /* zero if successful and a negative value if an error occurred.     */
   /* * NOTE * In order to add a device to the resolving list, the local*/
   /*          device MUST have exchanged the IRK and identity          */
   /*          information during a pairing procedure.                  */
   /* * NOTE * If a remote device has been successfully added to the    */
   /*          resolving list in the controller, then the resolving list*/
   /*          may be used when advertising, scanning, or connecting to */
   /*          the remote device.                                       */
static QCLI_Command_Status_t AddDeviceToResolvingList(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   uint32_t              AddedDeviceCount;
   DeviceInfo_t         *DeviceInfo;
   qapi_BLE_BD_ADDR_t    BD_ADDR;
   QCLI_Command_Status_t ret_val;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BLE_Demo_Context.BluetoothStackID)
   {
      /* Verify that the input parameters are semi-valid.               */
      if((Parameter_Count >= 1) && (strlen((char *)(Parameter_List[0].String_Value)) == (sizeof(qapi_BLE_BD_ADDR_t) * 2)))
      {
         /* Convert the parameter to a Bluetooth Device Address.        */
         StrToBD_ADDR((char *)(Parameter_List[0].String_Value), &BD_ADDR);

         /* Lock the Bluetooth stack.                                   */
         if(!qapi_BLE_BSC_LockBluetoothStack(BLE_Demo_Context.BluetoothStackID))
         {
            /* Get the device info for the remote device.               */
            if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&(BLE_Demo_Context.DeviceInfoList), BD_ADDR)) != NULL)
            {
               /* Make sure the remote device is NOT already in the     */
               /* resolving list.                                       */
               if(!(DeviceInfo->Flags & (uint8_t)DEVICE_INFO_FLAGS_ADDED_TO_RESOLVING_LIST))
               {
                  /* Make sure the remote device has the IRK, which was */
                  /* exchanged during pairing.                          */
                  /* * NOTE * If we have the IRK for the remote device, */
                  /*          then we also have the Identity            */
                  /*          Information.                              */
                  if(DeviceInfo->Flags & (uint8_t)DEVICE_INFO_FLAGS_IRK_VALID)
                  {
                     /* Let's add the device to the resolving list in   */
                     /* the controller.                                 */
                     /* * NOTE * We will only add one device, however we*/
                     /*          could add more with this function.     */
                     ret_val = qapi_BLE_GAP_LE_Add_Device_To_Resolving_List(BLE_Demo_Context.BluetoothStackID, 1, &(DeviceInfo->ResolvingListEntry), &AddedDeviceCount);
                     if(!ret_val)
                     {
                        QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "qapi_BLE_GAP_LE_Add_Device_To_Resolving_List() success.\n");
                        QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   Added devices:  %u.\n", (unsigned int)AddedDeviceCount);

                        /* Flag that the remote device has been added to*/
                        /* the resolving list.                          */
                        DeviceInfo->Flags |= (uint8_t)DEVICE_INFO_FLAGS_ADDED_TO_RESOLVING_LIST;

                        ret_val = QCLI_STATUS_SUCCESS_E;
                     }
                     else
                     {
                        QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Error - qapi_BLE_GAP_LE_Add_Device_To_Resolving_List() returned %d.\n", ret_val);

                        ret_val = QCLI_STATUS_ERROR_E;
                     }
                  }
                  else
                     ret_val = QCLI_STATUS_ERROR_E;
               }
               else
               {
                  QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Device already in the resolving list.\n");

                  ret_val = QCLI_STATUS_ERROR_E;
               }
            }
            else
            {
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "No Device Info.\n");

               ret_val = QCLI_STATUS_ERROR_E;
            }

            /* Un-lock the Bluetooth Stack.                             */
            qapi_BLE_BSC_UnLockBluetoothStack(BLE_Demo_Context.BluetoothStackID);
         }
         else
         {
            QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Unable to acquire Bluetooth Stack Lock.\n");

            ret_val = QCLI_STATUS_ERROR_E;
         }
      }
      else
         ret_val = QCLI_STATUS_USAGE_E;
   }
   else
      ret_val = QCLI_STATUS_ERROR_E;

   return(ret_val);
}

   /* The following function is responsible for removing a remote device*/
   /* from the white list in the controller.  This function returns zero*/
   /* if successful and a negative value if an error occurred.          */
static QCLI_Command_Status_t RemoveDeviceFromResolvingList(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t ret_val;
   DeviceInfo_t         *DeviceInfo;
   qapi_BLE_BD_ADDR_t    BD_ADDR;
   uint32_t              RemovedDeviceCount;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BLE_Demo_Context.BluetoothStackID)
   {
      /* Verify that the input parameters are semi-valid.               */
      if((Parameter_Count >= 1) && (strlen((char *)(Parameter_List[0].String_Value)) == (sizeof(qapi_BLE_BD_ADDR_t) * 2)))
      {
         /* Convert the parameter to a Bluetooth Device Address.        */
         StrToBD_ADDR((char *)(Parameter_List[0].String_Value), &BD_ADDR);

         /* Lock the Bluetooth stack.                                   */
         if(!qapi_BLE_BSC_LockBluetoothStack(BLE_Demo_Context.BluetoothStackID))
         {
            /* Get the device info for the remote device.               */
            if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&(BLE_Demo_Context.DeviceInfoList), BD_ADDR)) != NULL)
            {
               /* Make sure the remote device has been added to the     */
               /* resolving list.                                       */
               if(DeviceInfo->Flags & (uint8_t)DEVICE_INFO_FLAGS_ADDED_TO_RESOLVING_LIST)
               {
                  /* Let's remove the device from the resolving list in */
                  /* the controller.                                    */
                  /* * NOTE * We will only remove one device, however we*/
                  /*          could remove more with this function.     */
                  ret_val = qapi_BLE_GAP_LE_Remove_Device_From_Resolving_List(BLE_Demo_Context.BluetoothStackID, 1, &(DeviceInfo->ResolvingListEntry), &RemovedDeviceCount);
                  if(!ret_val)
                  {
                     QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "qapi_BLE_GAP_LE_Remove_Device_From_Resolving_List() success.\n");
                     QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   Removed devices:  %u.\n", (unsigned int)RemovedDeviceCount);

                     /* Flag that the remote device has been removed    */
                     /* from the resolving list.                        */
                     DeviceInfo->Flags &= ~((uint8_t)DEVICE_INFO_FLAGS_ADDED_TO_RESOLVING_LIST);

                     ret_val = QCLI_STATUS_SUCCESS_E;
                  }
                  else
                  {
                     QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Error - qapi_BLE_GAP_LE_Remove_Device_From_Resolving_List() returned %d.\n", ret_val);

                     ret_val = QCLI_STATUS_ERROR_E;
                  }
               }
               else
               {
                  QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Device not in the resolving list.\n");

                  ret_val = QCLI_STATUS_ERROR_E;
               }
            }
            else
            {
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "No Device Info.\n");

               ret_val = QCLI_STATUS_ERROR_E;
            }

            /* Un-lock the Bluetooth Stack.                             */
            qapi_BLE_BSC_UnLockBluetoothStack(BLE_Demo_Context.BluetoothStackID);
         }
         else
         {
            QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Unable to acquire Bluetooth Stack Lock.\n");

            ret_val = QCLI_STATUS_ERROR_E;
         }
      }
      else
         ret_val = QCLI_STATUS_USAGE_E;
   }
   else
      ret_val = QCLI_STATUS_ERROR_E;

   return(ret_val);
}

#ifdef V2

   /* The following function is provided to enter/exit LMM on the M0.   */
   /* This function returns QCLI_STATUS_SUCCESS_E on success or an error*/
   /* enumeration if an error occurs.                                   */
static QCLI_Command_Status_t M0_LMM_Mode_Switch(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t ret_val;

   /* Next check to see if the parameters required for the execution of */
   /* this function appear to be semi-valid.                            */
   if((Parameter_Count >= 1) && (Parameter_List) && (Parameter_List[0].Integer_Is_Valid))
   {
      /* Check to see if we should enter or exit LMM on the M0.         */
      if(Parameter_List[0].Integer_Value)
      {
         /* Enter LMM.                                                  */
         ret_val = qapi_OMTM_Switch_ConSS_Memory_Mode(QAPI_OMTM_CONSS_MODE_LMM1);
      }
      else
      {
         /* Enter FMM.                                                  */
         ret_val = qapi_OMTM_Switch_ConSS_Memory_Mode(QAPI_OMTM_CONSS_MODE_FMM);
      }

      if(ret_val != QAPI_OK)
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Error - qapi_OMTM_Switch_CONSS_Operating_Mode() returned %d.\n", ret_val);
   }
   else
      ret_val = QCLI_STATUS_USAGE_E;

   return(ret_val);
}

#endif

   /* The following function is provided to start a receive test. This  */
   /* function returns QCLI_STATUS_SUCCESS_E on success or an error     */
   /* enumeration if an error occurs.                                   */
static QCLI_Command_Status_t RecieverTest(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t ret_val;
   int                   Result;
   uint8_t               Status;

   /* First check to see if the parameters required for the execution of*/
   /* this function appear to be semi-valid.                            */
   if(BLE_Demo_Context.BluetoothStackID)
   {
      /* Next check to see if the parameters required for the execution */
      /* of this function appear to be semi-valid.                      */
      if((Parameter_Count >= 1) && (Parameter_List) && (Parameter_List[0].Integer_Is_Valid) && (Parameter_List[0].Integer_Value >= QAPI_BLE_HCI_LE_RECEIVER_TRANSMITTER_TEST_FREQUENCY_MINIMUM) && (Parameter_List[0].Integer_Value <= QAPI_BLE_HCI_LE_RECEIVER_TRANSMITTER_TEST_FREQUENCY_MAXIMUM))
      {
         Result = qapi_BLE_HCI_LE_Receiver_Test(BLE_Demo_Context.BluetoothStackID, Parameter_List[0].Integer_Value, &Status);

         if((Result == 0) && (Status == 0))
         {
            QCLI_Printf(BLE_Demo_Context.QCLI_DUT_Handle, "Receiver Test Started.\n");
            ret_val = QCLI_STATUS_SUCCESS_E;
         }
         else
         {
            QCLI_Printf(BLE_Demo_Context.QCLI_DUT_Handle, "Failed to start Receiver Test (%d/%d).\n", Result, Status);
            ret_val = QCLI_STATUS_ERROR_E;
         }
      }
      else
         ret_val = QCLI_STATUS_USAGE_E;
   }
   else
      ret_val = QCLI_STATUS_ERROR_E;

   return(ret_val);
}

   /* The following function is provided to start a transmit test. This */
   /* function returns QCLI_STATUS_SUCCESS_E on success or an error     */
   /* enumeration if an error occurs.                                   */
static QCLI_Command_Status_t TransmitterTest(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t ret_val;
   int                   Result;
   uint8_t               Status;

   /* First check to see if the parameters required for the execution of*/
   /* this function appear to be semi-valid.                            */
   if(BLE_Demo_Context.BluetoothStackID)
   {
      /* Next check to see if the parameters required for the execution */
      /* of this function appear to be semi-valid.                      */
      if((Parameter_Count >= 1) && (Parameter_List) &&
      (Parameter_List[0].Integer_Is_Valid) && (Parameter_List[0].Integer_Value >= QAPI_BLE_HCI_LE_RECEIVER_TRANSMITTER_TEST_FREQUENCY_MINIMUM) && (Parameter_List[0].Integer_Value <= QAPI_BLE_HCI_LE_RECEIVER_TRANSMITTER_TEST_FREQUENCY_MAXIMUM) && (Parameter_List[1].Integer_Is_Valid) && (Parameter_List[1].Integer_Value >= QAPI_BLE_HCI_LE_TRANSMITTER_TEST_LENGTH_OF_TEST_DATA_MINIMUM_LENGTH) && (Parameter_List[1].Integer_Value <= QAPI_BLE_HCI_LE_TRANSMITTER_TEST_LENGTH_OF_TEST_DATA_MAXIMUM_LENGTH) && (Parameter_List[2].Integer_Is_Valid) && (Parameter_List[2].Integer_Value >= QAPI_BLE_HCI_LE_TRANSMITTER_TEST_PAYLOAD_PSEUDO_RANDOM_BIT_SEQUENCE_9) && (Parameter_List[2].Integer_Value <= QAPI_BLE_HCI_LE_TRANSMITTER_TEST_PAYLOAD_PATTERN_ALTERNATING_BITS_0x55))
      {
         Result = qapi_BLE_HCI_LE_Transmitter_Test(BLE_Demo_Context.BluetoothStackID, Parameter_List[0].Integer_Value, Parameter_List[1].Integer_Value, Parameter_List[2].Integer_Value, &Status);

         if((Result == 0) && (Status == 0))
         {
            QCLI_Printf(BLE_Demo_Context.QCLI_DUT_Handle, "Transmitter Test Started.\n");
            ret_val = QCLI_STATUS_SUCCESS_E;
         }
         else
         {
            QCLI_Printf(BLE_Demo_Context.QCLI_DUT_Handle, "Failed to start Transmitter Test (%d/%d).\n", Result, Status);
            ret_val = QCLI_STATUS_ERROR_E;
         }
      }
      else
         ret_val = QCLI_STATUS_USAGE_E;
   }
   else
      ret_val = QCLI_STATUS_ERROR_E;

   return(ret_val);
}

   /* The following function is provided to stop a transmit orreceive   */
   /* test. This function returns QCLI_STATUS_SUCCESS_E on success or an*/
   /* error enumeration if an error occurs.                             */
static QCLI_Command_Status_t TestEnd(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t ret_val;
   int                   Result;
   uint8_t               Status;
   uint16_t              Number_Of_PacketsResult;

   /* First check to see if the parameters required for the execution of*/
   /* this function appear to be semi-valid.                            */
   if(BLE_Demo_Context.BluetoothStackID)
   {
      Result = qapi_BLE_HCI_LE_Test_End(BLE_Demo_Context.BluetoothStackID, &Status, &Number_Of_PacketsResult);

      if((Result == 0) && (Status == 0))
      {
         QCLI_Printf(BLE_Demo_Context.QCLI_DUT_Handle, "Test Ended: %d Packets.\n", Number_Of_PacketsResult);
         ret_val = QCLI_STATUS_SUCCESS_E;
      }
      else
      {
         QCLI_Printf(BLE_Demo_Context.QCLI_DUT_Handle, "Failed to end test (%d/%d).\n", Result, Status);
         ret_val = QCLI_STATUS_ERROR_E;
      }
   }
   else
      ret_val = QCLI_STATUS_ERROR_E;

   return(ret_val);
}

   /* ***************************************************************** */
   /*                         Event Callbacks                           */
   /* ***************************************************************** */

   /* The following function is for the GAP LE Event Receive Data       */
   /* Callback.  This function will be called whenever a Callback has   */
   /* been registered for the specified GAP LE Action that is associated*/
   /* with the Bluetooth Stack.  This function passes to the caller the */
   /* GAP LE Event Data of the specified Event and the GAP LE Event     */
   /* Callback Parameter that was specified when this Callback was      */
   /* installed.  The caller is free to use the contents of the GAP LE  */
   /* Event Data ONLY in the context of this callback.  If the caller   */
   /* requires the Data for a longer period of time, then the callback  */
   /* function MUST copy the data into another Data Buffer.  This       */
   /* function is guaranteed NOT to be invoked more than once           */
   /* simultaneously for the specified installed callback (i.e.  this   */
   /* function DOES NOT have be reentrant).  It Needs to be noted       */
   /* however, that if the same Callback is installed more than once,   */
   /* then the callbacks will be called serially.  Because of this, the */
   /* processing in this function should be as efficient as possible.   */
   /* It should also be noted that this function is called in the Thread*/
   /* Context of a Thread that the User does NOT own.  Therefore,       */
   /* processing in this function should be as efficient as possible    */
   /* (this argument holds anyway because other GAP Events will not be  */
   /* processed while this function call is outstanding).               */
   /* * NOTE * This function MUST NOT Block and wait for Events that can*/
   /*          only be satisfied by Receiving a Bluetooth Event         */
   /*          Callback.  A Deadlock WILL occur because NO Bluetooth    */
   /*          Callbacks will be issued while this function is currently*/
   /*          outstanding.                                             */
static void QAPI_BLE_BTPSAPI GAP_LE_Event_Callback(uint32_t BluetoothStackID, qapi_BLE_GAP_LE_Event_Data_t *GAP_LE_Event_Data, uint32_t CallbackParameter)
{
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
   qapi_BLE_GAP_LE_Authentication_Response_Information_t  GAP_LE_Authentication_Response_Information;

   /* Verify that all parameters to this callback are Semi-Valid.       */
   if((BluetoothStackID) && (GAP_LE_Event_Data))
   {
      switch(GAP_LE_Event_Data->Event_Data_Type)
      {
         case QAPI_BLE_ET_LE_ADVERTISING_REPORT_E:
            QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "etLE_Advertising_Report with size %d.\n",(int)GAP_LE_Event_Data->Event_Data_Size);
            QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "  %d Responses.\n",GAP_LE_Event_Data->Event_Data.GAP_LE_Advertising_Report_Event_Data->Number_Device_Entries);

            for(Index = 0; Index < GAP_LE_Event_Data->Event_Data.GAP_LE_Advertising_Report_Event_Data->Number_Device_Entries; Index++)
            {
               DeviceEntryPtr = &(GAP_LE_Event_Data->Event_Data.GAP_LE_Advertising_Report_Event_Data->Advertising_Data[Index]);

               /* Display the packet type for the device                */
               switch(DeviceEntryPtr->Advertising_Report_Type)
               {
                  case QAPI_BLE_RT_CONNECTABLE_UNDIRECTED_E:
                     QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "  Advertising Type: %s.\n", "QAPI_BLE_RT_CONNECTABLE_UNDIRECTED_E");
                     break;
                  case QAPI_BLE_RT_CONNECTABLE_DIRECTED_E:
                     QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "  Advertising Type: %s.\n", "QAPI_BLE_RT_CONNECTABLE_DIRECTED_E");
                     break;
                  case QAPI_BLE_RT_SCANNABLE_UNDIRECTED_E:
                     QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "  Advertising Type: %s.\n", "QAPI_BLE_RT_SCANNABLE_UNDIRECTED_E");
                     break;
                  case QAPI_BLE_RT_NON_CONNECTABLE_UNDIRECTED_E:
                     QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "  Advertising Type: %s.\n", "QAPI_BLE_RT_NON_CONNECTABLE_UNDIRECTED_E");
                     break;
                  case QAPI_BLE_RT_SCAN_RESPONSE_E:
                     QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "  Advertising Type: %s.\n", "QAPI_BLE_RT_SCAN_RESPONSE_E");
                     break;
               }

               /* Display the Address Type.                             */
               switch(DeviceEntryPtr->Address_Type)
               {
                  case QAPI_BLE_LAT_PUBLIC_E:
                     QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "  Address Type:        %s.\n", "QAPI_BLE_LAT_PUBLIC_E");
                     break;
                  case QAPI_BLE_LAT_RANDOM_E:
                     QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "  Address Type:        %s.\n", "QAPI_BLE_LAT_RANDOM_E");
                     break;
                  case QAPI_BLE_LAT_PUBLIC_IDENTITY_E:
                     QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "  Address Type:        %s.\n", "QAPI_BLE_LAT_PUBLIC_IDENTITY_E");
                     break;
                  case QAPI_BLE_LAT_RANDOM_IDENTITY_E:
                     QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "  Address Type:        %s.\n", "QAPI_BLE_LAT_RANDOM_IDENTITY_E");
                     break;
                  default:
                     QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "  Address Type:        Invalid.\n");
                     break;
               }

               /* Display the Device Address.                           */
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "  Address: 0x%02X%02X%02X%02X%02X%02X.\n", DeviceEntryPtr->BD_ADDR.BD_ADDR5, DeviceEntryPtr->BD_ADDR.BD_ADDR4, DeviceEntryPtr->BD_ADDR.BD_ADDR3, DeviceEntryPtr->BD_ADDR.BD_ADDR2, DeviceEntryPtr->BD_ADDR.BD_ADDR1, DeviceEntryPtr->BD_ADDR.BD_ADDR0);
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "  RSSI: %d.\n", (int)(DeviceEntryPtr->RSSI));
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "  Data Length: %d.\n", DeviceEntryPtr->Raw_Report_Length);

               DisplayAdvertisingData(&(DeviceEntryPtr->Advertising_Data));
            }
            break;
         case QAPI_BLE_ET_LE_DATA_LENGTH_CHANGE_E:
            QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "etLE_Data_Length_Change with size %d.\n",(int)GAP_LE_Event_Data->Event_Data_Size);

            if(GAP_LE_Event_Data->Event_Data.GAP_LE_Data_Length_Change_Event_Data)
            {
               BD_ADDRToStr(GAP_LE_Event_Data->Event_Data.GAP_LE_Data_Length_Change_Event_Data->BD_ADDR, BoardStr);

               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   Max Tx Octets: %u.\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Data_Length_Change_Event_Data->MaxTxOctets);
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   Max Rx Octets: %u.\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Data_Length_Change_Event_Data->MaxRxOctets);

               BLE_Demo_Context.ConnMaxTxOcets = GAP_LE_Event_Data->Event_Data.GAP_LE_Data_Length_Change_Event_Data->MaxTxOctets;
               BLE_Demo_Context.ConnMaxRxOcets = GAP_LE_Event_Data->Event_Data.GAP_LE_Data_Length_Change_Event_Data->MaxRxOctets;
            }
            break;
         case QAPI_BLE_ET_LE_CONNECTION_COMPLETE_E:
            QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "etLE_Connection_Complete with size %d.\n",(int)GAP_LE_Event_Data->Event_Data_Size);

            if(GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data)
            {
               BD_ADDRToStr(GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Peer_Address, BoardStr);

               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   Status:              0x%02X.\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Status);
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   Role:                %s.\n", (GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Master)?"Master":"Slave");
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   Address Type:        %s.\n", (GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Peer_Address_Type == QAPI_BLE_LAT_PUBLIC_E)?"Public":"Random");
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   BD_ADDR:             %s.\n", BoardStr);

               if(GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Status == QAPI_BLE_HCI_ERROR_CODE_NO_ERROR)
               {
                  QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   Connection Interval: %u.\n", (unsigned int)GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Current_Connection_Parameters.Connection_Interval);
                  QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   Slave Latency:       %u.\n", (unsigned int)GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Current_Connection_Parameters.Slave_Latency);

                  BLE_Demo_Context.ConnectionBD_ADDR   = GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Peer_Address;
                  BLE_Demo_Context.LocalDeviceIsMaster = GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Master;

                  /* Set the initial count for the maximum number of    */
                  /* octets we will transmit and receive.               */
                  BLE_Demo_Context.ConnMaxTxOcets      = 27;
                  BLE_Demo_Context.ConnMaxRxOcets      = 27;

                  /* Make sure that no entry already exists.            */
                  if((DeviceInfo = SearchDeviceInfoEntryTypeAddress(&(BLE_Demo_Context.DeviceInfoList), GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Peer_Address_Type, GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Peer_Address)) == NULL)
                  {
                     /* No entry exists so create one.                  */
                     if(!CreateNewDeviceInfoEntry(&(BLE_Demo_Context.DeviceInfoList), GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Peer_Address_Type, BLE_Demo_Context.ConnectionBD_ADDR))
                        QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Failed to add device to Device Info List.\n");
                  }
                  else
                  {
                     /* If we are the Master of the connection we will  */
                     /* attempt to Re-Establish Security if a LTK for   */
                     /* this device exists (i.e.  we previously paired).*/
                     if(BLE_Demo_Context.LocalDeviceIsMaster)
                     {
                        /* Re-Establish Security if there is a LTK that */
                        /* is stored for this device.                   */
                        if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_LTK_VALID)
                        {
                           /* Re-Establish Security with this LTK.      */
                           QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Attempting to Re-Establish Security.\n");

                           /* Attempt to re-establish security to this  */
                           /* device.                                   */
                           GAP_LE_Security_Information.Local_Device_Is_Master                                      = TRUE;
                           memcpy(&(GAP_LE_Security_Information.Security_Information.Master_Information.LTK), &(DeviceInfo->LTK), sizeof(DeviceInfo->LTK));
                           GAP_LE_Security_Information.Security_Information.Master_Information.EDIV                = DeviceInfo->EDIV;
                           memcpy(&(GAP_LE_Security_Information.Security_Information.Master_Information.Rand), &(DeviceInfo->Rand), sizeof(DeviceInfo->Rand));
                           GAP_LE_Security_Information.Security_Information.Master_Information.Encryption_Key_Size = DeviceInfo->EncryptionKeySize;

                           Result = qapi_BLE_GAP_LE_Reestablish_Security(BluetoothStackID, BLE_Demo_Context.ConnectionBD_ADDR, &GAP_LE_Security_Information, GAP_LE_Event_Callback, 0);
                           if(Result)
                           {
                              QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "GAP_LE_Reestablish_Security returned %d.\n",Result);
                           }
                        }
                     }
                  }
               }
               else
               {
                  /* Connection failed so store the saved BD_ADDR of the*/
                  /* remote device.                                     */
                  QAPI_BLE_ASSIGN_BD_ADDR(BLE_Demo_Context.ConnectionBD_ADDR, 0, 0, 0, 0, 0, 0);
               }
            }
            break;
         case QAPI_BLE_ET_LE_DISCONNECTION_COMPLETE_E:
            QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "etLE_Disconnection_Complete with size %d.\n", (int)GAP_LE_Event_Data->Event_Data_Size);

            if(GAP_LE_Event_Data->Event_Data.GAP_LE_Disconnection_Complete_Event_Data)
            {
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   Status: 0x%02X.\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Disconnection_Complete_Event_Data->Status);
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   Reason: 0x%02X.\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Disconnection_Complete_Event_Data->Reason);

               BD_ADDRToStr(GAP_LE_Event_Data->Event_Data.GAP_LE_Disconnection_Complete_Event_Data->Peer_Address, BoardStr);
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   BD_ADDR: %s.\n", BoardStr);

               /* Check to see if the device info is present in the     */
               /* list.                                                 */
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&(BLE_Demo_Context.DeviceInfoList), BLE_Demo_Context.ConnectionBD_ADDR)) != NULL)
               {
                  /* Flag that no service discovery operation is        */
                  /* outstanding for this device.                       */
                  DeviceInfo->Flags &= ~DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_OUTSTANDING;
               }

               /* Clear the saved Connection BD_ADDR.                   */
               QAPI_BLE_ASSIGN_BD_ADDR(BLE_Demo_Context.ConnectionBD_ADDR, 0, 0, 0, 0, 0, 0);
               BLE_Demo_Context.LocalDeviceIsMaster = FALSE;
            }
            break;
         case QAPI_BLE_ET_LE_CONNECTION_PARAMETER_UPDATE_REQUEST_E:
            QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "etLE_Connection_Parameter_Update_Request with size %d.\n",(int)GAP_LE_Event_Data->Event_Data_Size);

            if(GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Update_Request_Event_Data)
            {
               BD_ADDRToStr(GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Update_Request_Event_Data->BD_ADDR, BoardStr);
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   BD_ADDR:                     %s\r\n", BoardStr);
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   Connection Interval Minimum: %d\r\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Update_Request_Event_Data->Conn_Interval_Min);
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   Connection Interval Maximum: %d\r\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Update_Request_Event_Data->Conn_Interval_Max);
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   Slave Latency:               %d\r\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Update_Request_Event_Data->Slave_Latency);
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   Supervision Timeout:         %d\r\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Update_Request_Event_Data->Conn_Supervision_Timeout);

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
            QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "etLE_Connection_Parameter_Updated with size %d.\n",(int)GAP_LE_Event_Data->Event_Data_Size);

            if(GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Updated_Event_Data)
            {
               BD_ADDRToStr(GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Updated_Event_Data->BD_ADDR, BoardStr);
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   BD_ADDR:             %s\r\n", BoardStr);
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   Status:              %d\r\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Updated_Event_Data->Status);
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   Connection Interval: %d\r\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Updated_Event_Data->Current_Connection_Parameters.Connection_Interval);
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   Slave Latency:       %d\r\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Updated_Event_Data->Current_Connection_Parameters.Slave_Latency);
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   Supervision Timeout: %d\r\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Updated_Event_Data->Current_Connection_Parameters.Supervision_Timeout);
            }
            break;
         case QAPI_BLE_ET_LE_ENCRYPTION_CHANGE_E:
            QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "etLE_Encryption_Change with size %d.\n",(int)GAP_LE_Event_Data->Event_Data_Size);
            break;
         case QAPI_BLE_ET_LE_ENCRYPTION_REFRESH_COMPLETE_E:
            QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "etLE_Encryption_Refresh_Complete with size %d.\n", (int)GAP_LE_Event_Data->Event_Data_Size);
            break;
         case QAPI_BLE_ET_LE_AUTHENTICATION_E:
            QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "etLE_Authentication with size %d.\n", (int)GAP_LE_Event_Data->Event_Data_Size);

            /* Make sure the authentication event data is valid before  */
            /* continuing.                                              */
            if((Authentication_Event_Data = GAP_LE_Event_Data->Event_Data.GAP_LE_Authentication_Event_Data) != NULL)
            {
               BD_ADDRToStr(Authentication_Event_Data->BD_ADDR, BoardStr);

               switch(Authentication_Event_Data->GAP_LE_Authentication_Event_Type)
               {
                  case QAPI_BLE_LAT_LONG_TERM_KEY_REQUEST_E:
                     QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "    latKeyRequest: \r\n");
                     QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "      BD_ADDR: %s.\r\n", BoardStr);

                     /* Initialize the authentication response data to  */
                     /* indicate no LTK present (if we find or          */
                     /* re-generate the LTK we will update this         */
                     /* structure accordingly).                         */
                     GAP_LE_Authentication_Response_Information.GAP_LE_Authentication_Type = QAPI_BLE_LAR_LONG_TERM_KEY_E;
                     GAP_LE_Authentication_Response_Information.Authentication_Data_Length = 0;

                     /* Initialize some variables to determine if this  */
                     /* is a request for a Long Term Key generated via  */
                     /* Secure Connections (which we must store and can */
                     /* NOT re-generate).                               */
                     memset(&RandomNumber, 0, sizeof(RandomNumber));
                     EDIV = 0;

                     /* Check to see if this is a request for a SC      */
                     /* generated Long Term Key.                        */
                     if((Authentication_Event_Data->Authentication_Event_Data.Long_Term_Key_Request.EDIV == EDIV) && (QAPI_BLE_COMPARE_RANDOM_NUMBER(Authentication_Event_Data->Authentication_Event_Data.Long_Term_Key_Request.Rand, RandomNumber)))
                     {
                        /* Search for the entry for this slave to store */
                        /* the information into.                        */
                        if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&(BLE_Demo_Context.DeviceInfoList), Authentication_Event_Data->BD_ADDR)) != NULL)
                        {
                           /* Check to see if the LTK is valid.         */
                           if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_LTK_VALID)
                           {
                              /* Respond with the stored Long Term Key. */
                              GAP_LE_Authentication_Response_Information.Authentication_Data_Length                                        = QAPI_BLE_GAP_LE_LONG_TERM_KEY_INFORMATION_DATA_SIZE;
                              GAP_LE_Authentication_Response_Information.Authentication_Data.Long_Term_Key_Information.Encryption_Key_Size = DeviceInfo->EncryptionKeySize;

                              memcpy(&(GAP_LE_Authentication_Response_Information.Authentication_Data.Long_Term_Key_Information.Long_Term_Key), &(DeviceInfo->LTK), QAPI_BLE_LONG_TERM_KEY_SIZE);
                           }
                        }
                     }
                     else
                     {
                        /* The other side of a connection is requesting */
                        /* that we start encryption.  Thus we should    */
                        /* regenerate LTK for this connection and send  */
                        /* it to the chip.                              */
                        Result = qapi_BLE_GAP_LE_Regenerate_Long_Term_Key(BluetoothStackID, (qapi_BLE_Encryption_Key_t *)(&BLE_Demo_Context.DHK), (qapi_BLE_Encryption_Key_t *)(&BLE_Demo_Context.ER), Authentication_Event_Data->Authentication_Event_Data.Long_Term_Key_Request.EDIV, &(Authentication_Event_Data->Authentication_Event_Data.Long_Term_Key_Request.Rand), &GeneratedLTK);
                        if(!Result)
                        {
                           QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "      GAP_LE_Regenerate_Long_Term_Key Success.\r\n");

                           /* Respond with the Re-Generated Long Term   */
                           /* Key.                                      */
                           GAP_LE_Authentication_Response_Information.GAP_LE_Authentication_Type                                        = QAPI_BLE_LAR_LONG_TERM_KEY_E;
                           GAP_LE_Authentication_Response_Information.Authentication_Data_Length                                        = QAPI_BLE_GAP_LE_LONG_TERM_KEY_INFORMATION_DATA_SIZE;
                           GAP_LE_Authentication_Response_Information.Authentication_Data.Long_Term_Key_Information.Encryption_Key_Size = QAPI_BLE_GAP_LE_MAXIMUM_ENCRYPTION_KEY_SIZE;
                           GAP_LE_Authentication_Response_Information.Authentication_Data.Long_Term_Key_Information.Long_Term_Key       = GeneratedLTK;
                        }
                        else
                        {
                           QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "      GAP_LE_Regenerate_Long_Term_Key returned %d.\r\n",Result);
                        }
                     }

                     /* Send the Authentication Response.               */
                     Result = qapi_BLE_GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information);
                     if(Result)
                     {
                        QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "      GAP_LE_Authentication_Response returned %d.\n",Result);
                     }
                     break;
                  case QAPI_BLE_LAT_SECURITY_REQUEST_E:
                     /* Display the data for this event.                */
                     /* * NOTE * This is only sent from Slave to Master.*/
                     /*          Thus we must be the Master in this     */
                     /*          connection.                            */
                     QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "    latSecurityRequest:.\n");
                     QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "      BD_ADDR: %s.\n", BoardStr);
                     QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "      Bonding Type: %s.\n", ((Authentication_Event_Data->Authentication_Event_Data.Security_Request.Bonding_Type == QAPI_BLE_LBT_BONDING_E)?"Bonding":"No Bonding"));
                     QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "      MITM: %s.\n", ((Authentication_Event_Data->Authentication_Event_Data.Security_Request.MITM)?"YES":"NO"));

                     /* Determine if we have previously paired with the */
                     /* device. If we have paired we will attempt to    */
                     /* re-establish security using a previously        */
                     /* exchanged LTK.                                  */
                     if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&(BLE_Demo_Context.DeviceInfoList), Authentication_Event_Data->BD_ADDR)) != NULL)
                     {
                        /* Determine if a Valid Long Term Key is stored */
                        /* for this device.                             */
                        if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_LTK_VALID)
                        {
                           QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Attempting to Re-Establish Security.\n");

                           /* Attempt to re-establish security to this  */
                           /* device.                                   */
                           GAP_LE_Security_Information.Local_Device_Is_Master                                      = TRUE;
                           memcpy(&(GAP_LE_Security_Information.Security_Information.Master_Information.LTK), &(DeviceInfo->LTK), sizeof(DeviceInfo->LTK));
                           GAP_LE_Security_Information.Security_Information.Master_Information.EDIV                = DeviceInfo->EDIV;
                           memcpy(&(GAP_LE_Security_Information.Security_Information.Master_Information.Rand), &(DeviceInfo->Rand), sizeof(DeviceInfo->Rand));
                           GAP_LE_Security_Information.Security_Information.Master_Information.Encryption_Key_Size = DeviceInfo->EncryptionKeySize;

                           Result = qapi_BLE_GAP_LE_Reestablish_Security(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Security_Information, GAP_LE_Event_Callback, 0);
                           if(Result)
                           {
                              QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "GAP_LE_Reestablish_Security returned %d.\n",Result);
                           }
                        }
                        else
                        {
                           BLE_Demo_Context.CurrentRemoteBD_ADDR = Authentication_Event_Data->BD_ADDR;

                           /* We do not have a stored Link Key for this */
                           /* device so go ahead and pair to this       */
                           /* device.                                   */
                           SendPairingRequest(Authentication_Event_Data->BD_ADDR, TRUE);
                        }
                     }
                     else
                     {
                        BLE_Demo_Context.CurrentRemoteBD_ADDR = Authentication_Event_Data->BD_ADDR;

                        /* There is no Key Info Entry for this device   */
                        /* so we will just treat this as a slave        */
                        /* request and initiate pairing.                */
                        SendPairingRequest(Authentication_Event_Data->BD_ADDR, TRUE);
                     }
                     break;
                  case QAPI_BLE_LAT_PAIRING_REQUEST_E:
                     BLE_Demo_Context.CurrentRemoteBD_ADDR = Authentication_Event_Data->BD_ADDR;

                     QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Pairing Request: %s.\n",BoardStr);
                     DisplayLegacyPairingInformation(&Authentication_Event_Data->Authentication_Event_Data.Pairing_Request);

                     /* This is a pairing request. Respond with a       */
                     /* Pairing Response.                               */
                     /* * NOTE * This is only sent from Master to Slave.*/
                     /*          Thus we must be the Slave in this      */
                     /*          connection.                            */

                     /* Send the Pairing Response.                      */
                     SlavePairingRequestResponse(Authentication_Event_Data->BD_ADDR);
                     break;
                  case QAPI_BLE_LAT_EXTENDED_PAIRING_REQUEST_E:
                     BLE_Demo_Context.CurrentRemoteBD_ADDR = Authentication_Event_Data->BD_ADDR;

                     QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Extended Pairing Request: %s.\r\n", BoardStr);
                     DisplayPairingInformation(&(Authentication_Event_Data->Authentication_Event_Data.Extended_Pairing_Request));

                     /* This is a pairing request. Respond with a       */
                     /* Pairing Response.                               */
                     /* * NOTE * This is only sent from Master to Slave.*/
                     /*          Thus we must be the Slave in this      */
                     /*          connection.                            */

                     /* Send the Pairing Response.                      */
                     SlavePairingRequestResponse(Authentication_Event_Data->BD_ADDR);
                     break;
                  case QAPI_BLE_LAT_CONFIRMATION_REQUEST_E:
                     QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "latConfirmationRequest.\r\n");

                     /* Check to see what type of confirmation request  */
                     /* this is.                                        */
                     switch(Authentication_Event_Data->Authentication_Event_Data.Confirmation_Request.Request_Type)
                     {
                        case QAPI_BLE_CRT_NONE_E:
                           /* Handle the just works request.            */
                           GAP_LE_Authentication_Response_Information.GAP_LE_Authentication_Type = QAPI_BLE_LAR_CONFIRMATION_E;

                           /* By setting the Authentication_Data_Length */
                           /* to any NON-ZERO value we are informing the*/
                           /* GAP LE Layer that we are accepting Just   */
                           /* Works Pairing.                            */
                           GAP_LE_Authentication_Response_Information.Authentication_Data_Length = sizeof(uint32_t);

                           /* Handle this differently based on the local*/
                           /* IO Caps.                                  */
                           switch(BLE_Demo_Context.LE_Parameters.IOCapability)
                           {
                              case QAPI_BLE_LIC_NO_INPUT_NO_OUTPUT_E:
                                 QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Invoking Just Works.\r\n");

                                 /* By setting the                      */
                                 /* Authentication_Data_Length to any   */
                                 /* NON-ZERO value we are informing the */
                                 /* GAP LE Layer that we are accepting  */
                                 /* Just Works Pairing.                 */

                                 Result = qapi_BLE_GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information);
                                 if(Result)
                                 {
                                    QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "GAP_LE_Authentication_Response returned %d.\r\n",Result);
                                 }
                                 break;
                              case QAPI_BLE_LIC_DISPLAY_ONLY_E:
                                 QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Confirmation of Pairing.\r\n");

                                 GAP_LE_Authentication_Response_Information.Authentication_Data.Passkey = Authentication_Event_Data->Authentication_Event_Data.Confirmation_Request.Display_Passkey;

                                 /* Submit the Authentication Response. */
                                 if((Result = qapi_BLE_GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information)) != 0)
                                    DisplayFunctionError("GAP_LE_Authentication_Response", Result);
                                 break;
                              default:
                                 QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Confirmation of Pairing.\r\n");

                                 /* Submit the Authentication Response. */
                                 if((Result = qapi_BLE_GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information)) != 0)
                                    DisplayFunctionError("GAP_LE_Authentication_Response", Result);
                                 break;
                           }
                           break;
                        case QAPI_BLE_CRT_PASSKEY_E:
                           /* Inform the user to call the appropriate   */
                           /* command.                                  */
                           QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Call LEPasskeyResponse [PASSCODE].\r\n");
                           break;
                        case QAPI_BLE_CRT_DISPLAY_E:
                           QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Passkey: %06u.\r\n", (unsigned int)(Authentication_Event_Data->Authentication_Event_Data.Confirmation_Request.Display_Passkey));
                           break;
                        default:
                           /* This application doesn't support OOB and  */
                           /* Secure Connections request types will be  */
                           /* handled by the ExtendedConfirmationRequest*/
                           /* event.  So we will simply inform the user.*/
                           QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Authentication method not supported.\r\n");
                           break;
                     }
                     break;
                  case QAPI_BLE_LAT_EXTENDED_CONFIRMATION_REQUEST_E:
                     QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "latExtendedConfirmationRequest.\r\n");

                     QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   Secure Connections:     %s.\r\n", (Authentication_Event_Data->Authentication_Event_Data.Extended_Confirmation_Request.Flags & QAPI_BLE_GAP_LE_EXTENDED_CONFIRMATION_REQUEST_FLAGS_SECURE_CONNECTIONS)?"YES":"NO");
                     QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   Just Works Pairing:     %s.\r\n", (Authentication_Event_Data->Authentication_Event_Data.Extended_Confirmation_Request.Flags & QAPI_BLE_GAP_LE_EXTENDED_CONFIRMATION_REQUEST_FLAGS_JUST_WORKS_PAIRING)?"YES":"NO");
                     QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   Keypress Notifications: %s.\r\n", (Authentication_Event_Data->Authentication_Event_Data.Extended_Confirmation_Request.Flags & QAPI_BLE_GAP_LE_EXTENDED_CONFIRMATION_REQUEST_FLAGS_KEYPRESS_NOTIFICATIONS_REQUESTED)?"YES":"NO");

                     /* Check to see what type of confirmation request  */
                     /* this is.                                        */
                     switch(Authentication_Event_Data->Authentication_Event_Data.Extended_Confirmation_Request.Request_Type)
                     {
                        case QAPI_BLE_CRT_NONE_E:
                           /* Handle the just works request.            */
                           GAP_LE_Authentication_Response_Information.GAP_LE_Authentication_Type = QAPI_BLE_LAR_CONFIRMATION_E;

                           /* By setting the Authentication_Data_Length */
                           /* to any NON-ZERO value we are informing the*/
                           /* GAP LE Layer that we are accepting Just   */
                           /* Works Pairing.                            */
                           GAP_LE_Authentication_Response_Information.Authentication_Data_Length = sizeof(uint32_t);

                           /* Handle this differently based on the local*/
                           /* IO Caps.                                  */
                           switch(BLE_Demo_Context.LE_Parameters.IOCapability)
                           {
                              case QAPI_BLE_LIC_NO_INPUT_NO_OUTPUT_E:
                                 QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Invoking Just Works.\r\n");

                                 /* Just Accept Just Works Pairing.     */
                                 Result = qapi_BLE_GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information);
                                 if(Result)
                                 {
                                    QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "GAP_LE_Authentication_Response returned %d.\r\n", Result);
                                 }
                                 break;
                              case QAPI_BLE_LIC_DISPLAY_ONLY_E:
                                 QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Confirmation of Pairing.\r\n");

                                 GAP_LE_Authentication_Response_Information.Authentication_Data.Passkey = Authentication_Event_Data->Authentication_Event_Data.Confirmation_Request.Display_Passkey;

                                 /* Submit the Authentication Response. */
                                 if((Result = qapi_BLE_GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information)) != 0)
                                    DisplayFunctionError("GAP_LE_Authentication_Response", Result);
                                 break;
                              default:
                                 QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Confirmation of Pairing.\r\n");

                                 /* Submit the Authentication Response. */
                                 if((Result = qapi_BLE_GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information)) != 0)
                                    DisplayFunctionError("GAP_LE_Authentication_Response", Result);
                                 break;
                           }
                           break;
                        case QAPI_BLE_CRT_PASSKEY_E:
                           /* Inform the user to call the appropriate   */
                           /* command.                                  */
                           QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Call LEPasskeyResponse [PASSKEY].\r\n");
                           break;
                        case QAPI_BLE_CRT_DISPLAY_E:
                           QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Passkey: %06u.\r\n", Authentication_Event_Data->Authentication_Event_Data.Extended_Confirmation_Request.Display_Passkey);

                           GAP_LE_Authentication_Response_Information.GAP_LE_Authentication_Type  = QAPI_BLE_LAR_PASSKEY_E;
                           GAP_LE_Authentication_Response_Information.Authentication_Data_Length  = (uint8_t)(sizeof(uint32_t));
                           GAP_LE_Authentication_Response_Information.Authentication_Data.Passkey = Authentication_Event_Data->Authentication_Event_Data.Extended_Confirmation_Request.Display_Passkey;

                           /* Since this is in an extended confirmation */
                           /* request we need to respond to the display */
                           /* request.                                  */
                           if((Result = qapi_BLE_GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information)) != 0)
                              DisplayFunctionError("GAP_LE_Authentication_Response", Result);
                           break;
                        case QAPI_BLE_CRT_DISPLAY_YES_NO_E:
                           /* Handle the Display Yes/No request.        */
                           GAP_LE_Authentication_Response_Information.GAP_LE_Authentication_Type = QAPI_BLE_LAR_CONFIRMATION_E;

                           /* By setting the Authentication_Data_Length */
                           /* to any NON-ZERO value we are informing the*/
                           /* GAP LE Layer that we are accepting Just   */
                           /* Works Pairing.                            */
                           GAP_LE_Authentication_Response_Information.Authentication_Data_Length = sizeof(uint32_t);

                           /* Check to see if this is Just Works or     */
                           /* Numeric Comparison.                       */
                           if(Authentication_Event_Data->Authentication_Event_Data.Extended_Confirmation_Request.Flags & QAPI_BLE_GAP_LE_EXTENDED_CONFIRMATION_REQUEST_FLAGS_JUST_WORKS_PAIRING)
                           {
                              /* Handle this differently based on the   */
                              /* local IO Caps.                         */
                              switch(BLE_Demo_Context.LE_Parameters.IOCapability)
                              {
                                 case QAPI_BLE_LIC_NO_INPUT_NO_OUTPUT_E:
                                    QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Invoking Just Works.\r\n");

                                    /* Just Accept Just Works Pairing.  */
                                    Result = qapi_BLE_GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information);
                                    if(Result)
                                    {
                                       QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "GAP_LE_Authentication_Response returned %d.\r\n", Result);
                                    }
                                    break;
                                 case QAPI_BLE_LIC_DISPLAY_ONLY_E:
                                    QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Confirmation of Pairing.\r\n");

                                    GAP_LE_Authentication_Response_Information.Authentication_Data.Passkey = Authentication_Event_Data->Authentication_Event_Data.Confirmation_Request.Display_Passkey;

                                    /* Submit the Authentication        */
                                    /* Response.                        */
                                    if((Result = qapi_BLE_GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information)) != 0)
                                       DisplayFunctionError("GAP_LE_Authentication_Response", Result);
                                    break;
                                 default:
                                    QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Confirmation of Pairing.\r\n");

                                    /* Submit the Authentication        */
                                    /* Response.                        */
                                    if((Result = qapi_BLE_GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information)) != 0)
                                       DisplayFunctionError("GAP_LE_Authentication_Response", Result);
                                    break;
                              }
                           }
                           else
                           {
                              /* This is numeric comparison so go ahead */
                              /* and display the numeric value to       */
                              /* confirm.                               */
                              QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Confirmation Value: %ld\r\n", (unsigned long)Authentication_Event_Data->Authentication_Event_Data.Extended_Confirmation_Request.Display_Passkey);

                              /* Submit the Authentication Response.    */
                              if((Result = qapi_BLE_GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information)) != 0)
                                 DisplayFunctionError("GAP_LE_Authentication_Response", Result);
                              break;
                           }
                           break;
                        default:
                           /* This application doesn't support OOB so we*/
                           /* will simply inform the user.              */
                           QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Authentication method not supported.\r\n");
                           break;
                     }
                     break;
                  case QAPI_BLE_LAT_SECURITY_ESTABLISHMENT_COMPLETE_E:
                     QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Security Re-Establishment Complete: %s.\n", BoardStr);
                     QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "                            Status: 0x%02X.\n", Authentication_Event_Data->Authentication_Event_Data.Security_Establishment_Complete.Status);
                     break;
                  case QAPI_BLE_LAT_PAIRING_STATUS_E:
                     QAPI_BLE_ASSIGN_BD_ADDR(BLE_Demo_Context.CurrentRemoteBD_ADDR, 0, 0, 0, 0, 0, 0);

                     QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Pairing Status: %s.\n", BoardStr);
                     QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "        Status: 0x%02X.\n", Authentication_Event_Data->Authentication_Event_Data.Pairing_Status.Status);

                     if(Authentication_Event_Data->Authentication_Event_Data.Pairing_Status.Status == QAPI_BLE_GAP_LE_PAIRING_STATUS_NO_ERROR)
                     {
                        QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "        Key Size: %d.\n", Authentication_Event_Data->Authentication_Event_Data.Pairing_Status.Negotiated_Encryption_Key_Size);
                     }
                     else
                     {
                        /* Failed to pair so delete the key entry for   */
                        /* this device and disconnect the link.         */
                        if((DeviceInfo = DeleteDeviceInfoEntry(&(BLE_Demo_Context.DeviceInfoList), Authentication_Event_Data->BD_ADDR)) != NULL)
                           FreeDeviceInfoEntryMemory(DeviceInfo);

                        /* Disconnect the Link.                         */
                        qapi_BLE_GAP_LE_Disconnect(BluetoothStackID, Authentication_Event_Data->BD_ADDR);
                     }
                     break;
                  case QAPI_BLE_LAT_ENCRYPTION_INFORMATION_REQUEST_E:
                     QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Encryption Information Request %s.\n", BoardStr);

                     /* Generate new LTK, EDIV and Rand and respond with*/
                     /* them.                                           */
                     EncryptionInformationRequestResponse(Authentication_Event_Data->BD_ADDR, Authentication_Event_Data->Authentication_Event_Data.Encryption_Request_Information.Encryption_Key_Size, &GAP_LE_Authentication_Response_Information);
                     break;
                  case QAPI_BLE_LAT_ENCRYPTION_INFORMATION_E:
                     /* Display the information from the event.         */
                     QCLI_Printf(BLE_Demo_Context.QCLI_Handle, " Encryption Information from RemoteDevice: %s.\n", BoardStr);
                     QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "                             Key Size: %d.\n", Authentication_Event_Data->Authentication_Event_Data.Encryption_Information.Encryption_Key_Size);

                     /* Search for the entry for this slave to store the*/
                     /* information into.                               */
                     if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&(BLE_Demo_Context.DeviceInfoList), Authentication_Event_Data->BD_ADDR)) != NULL)
                     {
                        memcpy(&(DeviceInfo->LTK), &(Authentication_Event_Data->Authentication_Event_Data.Encryption_Information.LTK), sizeof(DeviceInfo->LTK));
                        DeviceInfo->EDIV              = Authentication_Event_Data->Authentication_Event_Data.Encryption_Information.EDIV;
                        memcpy(&(DeviceInfo->Rand), &(Authentication_Event_Data->Authentication_Event_Data.Encryption_Information.Rand), sizeof(DeviceInfo->Rand));
                        DeviceInfo->EncryptionKeySize = Authentication_Event_Data->Authentication_Event_Data.Encryption_Information.Encryption_Key_Size;
                        DeviceInfo->Flags            |= DEVICE_INFO_FLAGS_LTK_VALID;
                     }
                     else
                     {
                        QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "No Key Info Entry for this device.\r\n");
                     }
                     break;
                  case QAPI_BLE_LAT_IDENTITY_INFORMATION_REQUEST_E:
                     QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Identity Information Request %s.\n", BoardStr);

                     /* Store our identity information to send to the   */
                     /* remote device since it has been requested.      */
                     /* * NOTE * This demo will ALWAYS use the local    */
                     /*          public address as the identity address */
                     /*          since it is known to the user.         */
                     GAP_LE_Authentication_Response_Information.GAP_LE_Authentication_Type                            = QAPI_BLE_LAR_IDENTITY_INFORMATION_E;
                     GAP_LE_Authentication_Response_Information.Authentication_Data_Length                            = (uint8_t)QAPI_BLE_GAP_LE_IDENTITY_INFORMATION_DATA_SIZE;
                     GAP_LE_Authentication_Response_Information.Authentication_Data.Identity_Information.Address      = BLE_Demo_Context.PublicAddress;
                     GAP_LE_Authentication_Response_Information.Authentication_Data.Identity_Information.Address_Type = QAPI_BLE_LAT_PUBLIC_IDENTITY_E;
                     GAP_LE_Authentication_Response_Information.Authentication_Data.Identity_Information.IRK          = BLE_Demo_Context.IRK;

                     /* Send the authentication response.               */
                     Result = qapi_BLE_GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information);
                     if(!Result)
                     {
                        QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   qapi_BLE_GAP_LE_Authentication_Response (larEncryptionInformation) success.\n");
                     }
                     else
                     {
                        QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   Error - SM_Generate_Long_Term_Key returned %d.\n", Result);
                     }
                     break;
                  case QAPI_BLE_LAT_IDENTITY_INFORMATION_E:
                     /* Display the information from the event.         */
                     QCLI_Printf(BLE_Demo_Context.QCLI_Handle, " Identity Information from RemoteDevice: %s.\n", BoardStr);

                     /* Search for the entry for this slave to store the*/
                     /* information into.                               */
                     if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&(BLE_Demo_Context.DeviceInfoList), Authentication_Event_Data->BD_ADDR)) != NULL)
                     {
                        /* Store the identity information for the remote*/
                        /* device.                                      */
                        memcpy(&(DeviceInfo->IRK), &(Authentication_Event_Data->Authentication_Event_Data.Identity_Information.IRK), sizeof(DeviceInfo->IRK));
                        DeviceInfo->IdentityAddressBD_ADDR = Authentication_Event_Data->Authentication_Event_Data.Identity_Information.Address;
                        DeviceInfo->IdentityAddressType    = Authentication_Event_Data->Authentication_Event_Data.Identity_Information.Address_Type;
                        DeviceInfo->Flags                 |= DEVICE_INFO_FLAGS_IRK_VALID;

                        /* Setup the resolving list entry to add to the */
                        /* resolving list.                              */
                        DeviceInfo->ResolvingListEntry.Peer_Identity_Address      = DeviceInfo->IdentityAddressBD_ADDR;
                        DeviceInfo->ResolvingListEntry.Peer_Identity_Address_Type = DeviceInfo->IdentityAddressType;
                        DeviceInfo->ResolvingListEntry.Peer_IRK                   = DeviceInfo->IRK;
                        DeviceInfo->ResolvingListEntry.Local_IRK                  = BLE_Demo_Context.IRK;
                     }
                     else
                     {
                        QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "No Key Info Entry for this device.\n");
                     }
                     break;
                  default:
                     break;
               }
            }
            break;
         default:
            break;
      }
   }
}

   /* The following function is for an GATT Client Event Callback.  This*/
   /* function will be called whenever a GATT Response is received for a*/
   /* request that was made when this function was registered.  This    */
   /* function passes to the caller the GATT Client Event Data that     */
   /* occurred and the GATT Client Event Callback Parameter that was    */
   /* specified when this Callback was installed.  The caller is free to*/
   /* use the contents of the GATT Client Event Data ONLY in the context*/
   /* of this callback.  If the caller requires the Data for a longer   */
   /* period of time, then the callback function MUST copy the data into*/
   /* another Data Buffer.  This function is guaranteed NOT to be       */
   /* invoked more than once simultaneously for the specified installed */
   /* callback (i.e.  this function DOES NOT have be reentrant).  It    */
   /* Needs to be noted however, that if the same Callback is installed */
   /* more than once, then the callbacks will be called serially.       */
   /* Because of this, the processing in this function should be as     */
   /* efficient as possible.  It should also be noted that this function*/
   /* is called in the Thread Context of a Thread that the User does NOT*/
   /* own.  Therefore, processing in this function should be as         */
   /* efficient as possible (this argument holds anyway because another */
   /* GATT Event (Server/Client or Connection) will not be processed    */
   /* while this function call is outstanding).                         */
   /* * NOTE * This function MUST NOT Block and wait for Events that can*/
   /*          only be satisfied by Receiving a Bluetooth Event         */
   /*          Callback.  A Deadlock WILL occur because NO Bluetooth    */
   /*          Callbacks will be issued while this function is currently*/
   /*          outstanding.                                             */
static void QAPI_BLE_BTPSAPI GATT_ClientEventCallback_GAPS(uint32_t BluetoothStackID, qapi_BLE_GATT_Client_Event_Data_t *GATT_Client_Event_Data, uint32_t CallbackParameter)
{
   char         *NameBuffer;
   uint16_t      Appearance;
   BoardStr_t    BoardStr;
   DeviceInfo_t *DeviceInfo;

   /* Verify that all parameters to this callback are Semi-Valid.       */
   if((BluetoothStackID) && (GATT_Client_Event_Data))
   {
      /* Determine the event that occurred.                             */
      switch(GATT_Client_Event_Data->Event_Data_Type)
      {
         case QAPI_BLE_ET_GATT_CLIENT_ERROR_RESPONSE_E:
            if(GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data)
            {
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "\n");
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Error Response.\n");
               BD_ADDRToStr(GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->RemoteDevice, BoardStr);
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Connection ID:   %u.\n", GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ConnectionID);
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Transaction ID:  %u.\n", GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->TransactionID);
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Connection Type: %s.\n", (GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ConnectionType == QAPI_BLE_GCT_LE_E)?"LE":"BR/EDR");
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "BD_ADDR:         %s.\n", BoardStr);
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Error Type:      %s.\n", (GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorType == QAPI_BLE_RET_ERROR_RESPONSE_E)?"Response Error":"Response Timeout");

               /* Only print out the rest if it is valid.               */
               if(GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorType == QAPI_BLE_RET_ERROR_RESPONSE_E)
               {
                  QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Request Opcode:  0x%02X.\n", GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->RequestOpCode);
                  QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Request Handle:  0x%04X.\n", GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->RequestHandle);
                  QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Error Code:      0x%02X.\n", GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorCode);
                  QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Error Mesg:      %s.\n", ErrorCodeStr[GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorCode]);
               }
            }
            else
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Error - Null Error Response Data.\n");
            break;
         case QAPI_BLE_ET_GATT_CLIENT_READ_RESPONSE_E:
            if(GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data)
            {
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&(BLE_Demo_Context.DeviceInfoList), GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->RemoteDevice)) != NULL)
               {
                  if((uint16_t)CallbackParameter == DeviceInfo->GAPSClientInfo.DeviceNameHandle)
                  {
                     /* Display the remote device name.                 */
                     if((NameBuffer = (char *)malloc(GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength+1)) != NULL)
                     {
                        memset(NameBuffer, 0, GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength+1);
                        memcpy(NameBuffer, GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValue, GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength);

                        QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "\n");
                        QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Remote Device Name: %s.\n", NameBuffer);

                        free(NameBuffer);
                     }
                  }
                  else
                  {
                     if((uint16_t)CallbackParameter == DeviceInfo->GAPSClientInfo.DeviceAppearanceHandle)
                     {
                        if(GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength == QAPI_BLE_GAP_DEVICE_APPEARANCE_VALUE_LENGTH)
                        {
                           QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "\n");

                           Appearance = READ_UNALIGNED_WORD_LITTLE_ENDIAN(GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValue);
                           if(AppearanceToString(Appearance, &NameBuffer))
                              QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Remote Device Appearance: %s(%u).\n", NameBuffer, Appearance);
                           else
                              QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Remote Device Appearance: Unknown(%u).\n", Appearance);
                        }
                        else
                           QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Invalid Remote Appearance Value Length %u.\n", GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength);
                     }
                  }
               }
            }
            else
            {
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "\n");
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Error - Null Read Response Data.\n");
            }
            break;
         default:
            break;
      }
   }
   else
   {
      /* There was an error with one or more of the input parameters.   */
      QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "\n");

      QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "GATT Callback Data: Event_Data = NULL.\n");
   }
}

   /* The following function is for an GATT Connection Event Callback.  */
   /* This function is called for GATT Connection Events that occur on  */
   /* the specified Bluetooth Stack.  This function passes to the caller*/
   /* the GATT Connection Event Data that occurred and the GATT         */
   /* Connection Event Callback Parameter that was specified when this  */
   /* Callback was installed.  The caller is free to use the contents of*/
   /* the GATT Client Event Data ONLY in the context of this callback.  */
   /* If the caller requires the Data for a longer period of time, then */
   /* the callback function MUST copy the data into another Data Buffer.*/
   /* This function is guaranteed NOT to be invoked more than once      */
   /* simultaneously for the specified installed callback (i.e.  this   */
   /* function DOES NOT have be reentrant).  It Needs to be noted       */
   /* however, that if the same Callback is installed more than once,   */
   /* then the callbacks will be called serially.  Because of this, the */
   /* processing in this function should be as efficient as possible.   */
   /* It should also be noted that this function is called in the Thread*/
   /* Context of a Thread that the User does NOT own.  Therefore,       */
   /* processing in this function should be as efficient as possible    */
   /* (this argument holds anyway because another GATT Event            */
   /* (Server/Client or Connection) will not be processed while this    */
   /* function call is outstanding).                                    */
   /* * NOTE * This function MUST NOT Block and wait for Events that can*/
   /*          only be satisfied by Receiving a Bluetooth Event         */
   /*          Callback.  A Deadlock WILL occur because NO Bluetooth    */
   /*          Callbacks will be issued while this function is currently*/
   /*          outstanding.                                             */
static void QAPI_BLE_BTPSAPI GATT_Connection_Event_Callback(uint32_t BluetoothStackID, qapi_BLE_GATT_Connection_Event_Data_t *GATT_Connection_Event_Data, uint32_t CallbackParameter)
{
   BoardStr_t    BoardStr;
   DeviceInfo_t *DeviceInfo;

   /* Verify that all parameters to this callback are Semi-Valid.       */
   if((BluetoothStackID) && (GATT_Connection_Event_Data))
   {
      /* Determine the Connection Event that occurred.                  */
      switch(GATT_Connection_Event_Data->Event_Data_Type)
      {
         case QAPI_BLE_ET_GATT_CONNECTION_DEVICE_CONNECTION_E:
            if(GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data)
            {
               /* Save the Connection ID for later use.                 */
               BLE_Demo_Context.ConnectionID = GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data->ConnectionID;

               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "\n");
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "etGATT_Connection_Device_Connection with size %u: \n", GATT_Connection_Event_Data->Event_Data_Size);
               BD_ADDRToStr(GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data->RemoteDevice, BoardStr);
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   Connection ID:   %u.\n", GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data->ConnectionID);
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   Connection Type: %s.\n", ((GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data->ConnectionType == QAPI_BLE_GCT_LE_E)?"LE":"BR/EDR"));
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   Remote Device:   %s.\n", BoardStr);
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   Connection MTU:  %u.\n", GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data->MTU);
            }
            else
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Error - Null Connection Data.\n");
            break;
         case QAPI_BLE_ET_GATT_CONNECTION_DEVICE_DISCONNECTION_E:
            if(GATT_Connection_Event_Data->Event_Data.GATT_Device_Disconnection_Data)
            {
               /* Clear the Connection ID.                              */
               BLE_Demo_Context.ConnectionID = 0;

               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "\n");
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "etGATT_Connection_Device_Disconnection with size %u: \n", GATT_Connection_Event_Data->Event_Data_Size);
               BD_ADDRToStr(GATT_Connection_Event_Data->Event_Data.GATT_Device_Disconnection_Data->RemoteDevice, BoardStr);
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   Connection ID:   %u.\n", GATT_Connection_Event_Data->Event_Data.GATT_Device_Disconnection_Data->ConnectionID);
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   Connection Type: %s.\n", ((GATT_Connection_Event_Data->Event_Data.GATT_Device_Disconnection_Data->ConnectionType == QAPI_BLE_GCT_LE_E)?"LE":"BR/EDR"));
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "   Remote Device:   %s.\n", BoardStr);

               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&(BLE_Demo_Context.DeviceInfoList), GATT_Connection_Event_Data->Event_Data.GATT_Device_Disconnection_Data->RemoteDevice)) != NULL)
               {
                  /* If we are not bonded, the clear the states.        */
                  if(!(DeviceInfo->Flags & DEVICE_INFO_FLAGS_LTK_VALID))
                     DeviceInfo->Flags = 0;
                  else
                     DeviceInfo->Flags &= ~DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_OUTSTANDING;
               }
            }
            else
               QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Error - Null Disconnection Data.\n");
            break;
         default:
            break;
      }
   }
   else
   {
      /* There was an error with one or more of the input parameters.   */
      QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "\n");

      QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "GATT Connection Callback Data: Event_Data = NULL.\n");
   }
}

   /* The following function is for an GATT Discovery Event Callback.   */
   /* This function will be called whenever a GATT Service is discovered*/
   /* or a previously started service discovery process is completed.   */
   /* This function passes to the caller the GATT Discovery Event Data  */
   /* that occurred and the GATT Client Event Callback Parameter that   */
   /* was specified when this Callback was installed.  The caller is    */
   /* free to use the contents of the GATT Discovery Event Data ONLY in */
   /* the context of this callback.  If the caller requires the Data for*/
   /* a longer period of time, then the callback function MUST copy the */
   /* data into another Data Buffer.  This function is guaranteed NOT to*/
   /* be invoked more than once simultaneously for the specified        */
   /* installed callback (i.e.  this function DOES NOT have be          */
   /* reentrant).  It Needs to be noted however, that if the same       */
   /* Callback is installed more than once, then the callbacks will be  */
   /* called serially.  Because of this, the processing in this function*/
   /* should be as efficient as possible.  It should also be noted that */
   /* this function is called in the Thread Context of a Thread that the*/
   /* User does NOT own.  Therefore, processing in this function should */
   /* be as efficient as possible (this argument holds anyway because   */
   /* another GATT Discovery Event will not be processed while this     */
   /* function call is outstanding).                                    */
   /* * NOTE * This function MUST NOT Block and wait for Events that can*/
   /*          only be satisfied by Receiving a Bluetooth Event         */
   /*          Callback.  A Deadlock WILL occur because NO Bluetooth    */
   /*          Callbacks will be issued while this function is currently*/
   /*          outstanding.                                             */
static void QAPI_BLE_BTPSAPI GATT_Service_Discovery_Event_Callback(uint32_t BluetoothStackID, qapi_BLE_GATT_Service_Discovery_Event_Data_t *GATT_Service_Discovery_Event_Data, uint32_t CallbackParameter)
{
   DeviceInfo_t *DeviceInfo;

   if((BluetoothStackID) && (GATT_Service_Discovery_Event_Data))
   {
      if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&(BLE_Demo_Context.DeviceInfoList), BLE_Demo_Context.ConnectionBD_ADDR)) != NULL)
      {
         switch(GATT_Service_Discovery_Event_Data->Event_Data_Type)
         {
            case QAPI_BLE_ET_GATT_SERVICE_DISCOVERY_INDICATION_E:
               /* Verify the event data.                                */
               if(GATT_Service_Discovery_Event_Data->Event_Data.GATT_Service_Discovery_Indication_Data)
               {
                  QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "\n");
                  QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Service 0x%04X - 0x%04X, UUID: ", GATT_Service_Discovery_Event_Data->Event_Data.GATT_Service_Discovery_Indication_Data->ServiceInformation.Service_Handle, GATT_Service_Discovery_Event_Data->Event_Data.GATT_Service_Discovery_Indication_Data->ServiceInformation.End_Group_Handle);
                  DisplayUUID(&(GATT_Service_Discovery_Event_Data->Event_Data.GATT_Service_Discovery_Indication_Data->ServiceInformation.UUID));
                  QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "\n");

                  /* Attempt to populate the handles for the GAP        */
                  /* Service.                                           */
                  GAPSPopulateHandles(&(DeviceInfo->GAPSClientInfo), GATT_Service_Discovery_Event_Data->Event_Data.GATT_Service_Discovery_Indication_Data);
               }
               break;
            case QAPI_BLE_ET_GATT_SERVICE_DISCOVERY_COMPLETE_E:
               /* Verify the event data.                                */
               if(GATT_Service_Discovery_Event_Data->Event_Data.GATT_Service_Discovery_Complete_Data)
               {
                  QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "\n");
                  QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Service Discovery Operation Complete, Status 0x%02X.\n", GATT_Service_Discovery_Event_Data->Event_Data.GATT_Service_Discovery_Complete_Data->Status);

                  /* Flag that no service discovery operation is        */
                  /* outstanding for this device.                       */
                  DeviceInfo->Flags &= ~DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_OUTSTANDING;
               }
               break;
         }
      }
   }
   else
   {
      /* There was an error with one or more of the input parameters.   */
      QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "\n");

      QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "GATT Callback Data: Event_Data = NULL.\n");
   }
}

/******************************************************************************/
/**  EventCallback(uint32_t, HCI_Event_Data_t *, uint32_t)                   **/
/******************************************************************************/
/**  EventCallback Function.                                                 **/
/******************************************************************************/
static void EventCallback(uint32_t BluetoothStackID, qapi_BLE_HCI_Event_Data_t *HCI_Event_Data, uint32_t CallbackParameter)
{
   int          i;
   int          Event;
   uint16_t     Connection_Handle;
   unsigned int _NumberPackets;

   if((BluetoothStackID) && (HCI_Event_Data))
   {
      Event = (int)HCI_Event_Data->Event_Data_Type;
      switch(Event)
      {
         case QAPI_BLE_ET_NUMBER_OF_COMPLETED_PACKETS_EVENT_E:
            if((CurrentTest == CURRENT_TEST_TX_ACL) || (CurrentTest == CURRENT_TEST_PERIODIC))
            {
               for(i=0; i < HCI_Event_Data->Event_Data.HCI_Number_Of_Completed_Packets_Event_Data->Number_of_Handles; i++)
               {
                  /* Iterate through all of the handles and determine   */
                  /* the number of packets that have been sent for each */
                  /* connection.                                        */
                  Connection_Handle = HCI_Event_Data->Event_Data.HCI_Number_Of_Completed_Packets_Event_Data->HCI_Number_Of_Completed_Packets_Data[i].Connection_Handle;

                  if(Connection_Handle == ConnectionHandle)
                  {
                     /* Debit the number of packets sent for this       */
                     /* connection handle and Credit the number of      */
                     /* packets that can now be sent.                   */
                     _NumberPackets = HCI_Event_Data->Event_Data.HCI_Number_Of_Completed_Packets_Event_Data->HCI_Number_Of_Completed_Packets_Data[i].HC_Num_Of_Completed_Packets;

                     if(_NumberPackets > NumberOutstandingACLPackets)
                        _NumberPackets = NumberOutstandingACLPackets;

                     NumberOutstandingACLPackets -= _NumberPackets;

                     if(CurrentTest == CURRENT_TEST_TX_ACL)
                        SendACLData(PacketLength, TestBuffer);
                     break;
                  }
               }
            }
            break;
         case QAPI_BLE_ET_DISCONNECTION_COMPLETE_EVENT_E:
            if(ConnectionHandle == HCI_Event_Data->Event_Data.HCI_Disconnection_Complete_Event_Data->Connection_Handle)
            {
               if(CurrentTest == CURRENT_TEST_PERIODIC)
               {
                  /* Stop the timer.                                    */
                  qapi_Timer_Stop(BLE_Demo_Context.Timer);

                  /* Clean up the timer.                                */
                  qapi_Timer_Undef(BLE_Demo_Context.Timer);

                  BLE_Demo_Context.TransmitPeriod = 0;

                  QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Timer stopped.\n");
               }

               ConnectionHandle            = QAPI_BLE_HCI_CONNECTION_HANDLE_INVALID_VALUE;
               CurrentTest                 = CURRENT_TEST_NONE;
               NumberOutstandingACLPackets = 0;
            }
            break;
      }
   }
}

/******************************************************************************/
/**  ACLDataCallback()                                                       **/
/******************************************************************************/
/**  ACLDataCallback Function.                                               **/
/******************************************************************************/
static void ACLDataCallback(uint32_t BluetoothStackID, uint16_t Connection_Handle, uint16_t Flags, uint16_t ACLDataLength, uint8_t *ACLData, uint32_t CallbackParameter)
{
   if((BluetoothStackID) && (CurrentTest == CURRENT_TEST_RX_ACL))
   {
      /* This callback assumes the data coming in are L2CAP packets,    */
      /* since that is what it sends.                                   */

      /* First check to see if this is a ACL packet we are interested   */
      /* in.                                                            */
      if(Connection_Handle == ConnectionHandle)
      {
         if(!StartTime)
            StartTime = (uint32_t)qurt_timer_get_ticks();

         NumberBytes += ACLDataLength;
      }
   }
}

/**
   @brief Initializes the BLE demo and registers its commands with QCLI.

   @param IsColdBoot is a flag indicating if this is a cold boot (true) or warm
                     boot (false).
*/
void Initialize_BLE_Demo(qbool_t IsColdBoot)
{
   uint16_t                    Data_Size;
   unsigned int                Index;
   qapi_Status_t               Result;
   DeviceInfo_t               *DeviceInfo;
   BLE_Demo_Transition_Data_t *Transition_Data;

   memset(&BLE_Demo_Context, 0, sizeof(BLE_Demo_Context_t));

   if(!IsColdBoot)
   {
      /* Restore settings from transition memory.                       */
      Result = qapi_OMSM_Retrieve(QAPI_OMSM_DEFAULT_AON_POOL, OMSM_CLIENT_ID_BLE_E, &Data_Size, (void **)&Transition_Data);

      if((Result == QAPI_OK) && (Transition_Data != NULL))
      {
         if(Data_Size == sizeof(BLE_Demo_Transition_Data_t))
         {
            /* Clear the BLE Demo Context.                              */
            memset(&BLE_Demo_Context, 0, sizeof(BLE_Demo_Context));

            BLE_Demo_Context.BluetoothStackID    = Transition_Data->BluetoothStackID;
            BLE_Demo_Context.GAPSInstanceID      = Transition_Data->GAPSInstanceID;
            BLE_Demo_Context.BLEParameters       = Transition_Data->BLEParameters;
            BLE_Demo_Context.LE_Parameters       = Transition_Data->LE_Parameters;
            BLE_Demo_Context.ContextFlags        = Transition_Data->ContextFlags;
            BLE_Demo_Context.ScanInProgress      = Transition_Data->ScanInProgress;
            BLE_Demo_Context.ER                  = Transition_Data->ER;
            BLE_Demo_Context.IR                  = Transition_Data->IR;

            /* Regenerate IRK and DHK from the constant Identity Root   */
            /* Key.                                                     */
            qapi_BLE_GAP_LE_Diversify_Function(BLE_Demo_Context.BluetoothStackID, (qapi_BLE_Encryption_Key_t *)(&(BLE_Demo_Context.IR)), 1, 0, &(BLE_Demo_Context.IRK));
            qapi_BLE_GAP_LE_Diversify_Function(BLE_Demo_Context.BluetoothStackID, (qapi_BLE_Encryption_Key_t *)(&(BLE_Demo_Context.IR)), 3, 0, &(BLE_Demo_Context.DHK));

            /* Restore the paired devices.                              */
            for(Index=0;Index<Transition_Data->NumberPairedDevices;Index++)
            {
               /* Attempt to create the new entry.                      */
               if(CreateNewDeviceInfoEntry(&(BLE_Demo_Context.DeviceInfoList), Transition_Data->PairedDevices[Index].ConnectionAddressType, Transition_Data->PairedDevices[Index].ConnectionBD_ADDR))
               {
                  /* Find the entry we just added.                      */
                  if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&(BLE_Demo_Context.DeviceInfoList), Transition_Data->PairedDevices[Index].ConnectionBD_ADDR)) != NULL)
                  {
                     /* Copy the stored device entry over to the new    */
                     /* entry.                                          */
                     DeviceInfo->Flags             = Transition_Data->PairedDevices[Index].Flags;
                     DeviceInfo->EncryptionKeySize = Transition_Data->PairedDevices[Index].EncryptionKeySize;
                     DeviceInfo->LTK               = Transition_Data->PairedDevices[Index].LTK;
                     DeviceInfo->Rand              = Transition_Data->PairedDevices[Index].Rand;
                     DeviceInfo->EDIV              = Transition_Data->PairedDevices[Index].EDIV;
                     DeviceInfo->GAPSClientInfo    = Transition_Data->PairedDevices[Index].GAPSClientInfo;
                  }
               }
            }
         }

         qapi_OMSM_Free(QAPI_OMSM_DEFAULT_AON_POOL, OMSM_CLIENT_ID_BLE_E);
      }
   }

   BLE_Demo_Context.QCLI_Handle = QCLI_Register_Command_Group(NULL, &BLE_CMD_Group);

   if(BLE_Demo_Context.QCLI_Handle != NULL)
   {
      /* Register the DUT commands.                                     */
      BLE_Demo_Context.QCLI_DUT_Handle = QCLI_Register_Command_Group(BLE_Demo_Context.QCLI_Handle, &BLE_DUT_CMD_Group);

      if(IsColdBoot)
      {
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "BLE Demo Initialized.\n");
      }
   }
}

/**
   @brief Prepares the BLE demo for a mode change.

   @return true if the mode transition can proceed or false if it should be
           aborted.
*/
qbool_t BLE_Prepare_Mode_Switch(Operating_Mode_t Next_Mode)
{
   void          *Buffer;
   qbool_t        ret_val;
   qapi_Status_t  Result;

   /* Make sure the demo and BLE layer have been initialized            */
   /* successfully.                                                     */
   if((BLE_Demo_Context.QCLI_Handle != NULL) && (BLE_Demo_Context.BluetoothStackID != 0))
   {

#ifdef V2

      /* Make sure we are in a state where we can transition out of FOM.*/
      if(QAPI_BLE_COMPARE_NULL_BD_ADDR(BLE_Demo_Context.ConnectionBD_ADDR))

#else

      if(1)

#endif
      {
         /* Allocate the transition memory.                             */
         Result = qapi_OMSM_Alloc(QAPI_OMSM_DEFAULT_AON_POOL, OMSM_CLIENT_ID_BLE_E, sizeof(BLE_Demo_Transition_Data_t), &Buffer);
         if((Result == QAPI_OK) && (Buffer != NULL))
         {
            /* Attempt to commit the memory.                            */
            Result = qapi_OMSM_Commit(QAPI_OMSM_DEFAULT_AON_POOL, OMSM_CLIENT_ID_BLE_E);
            if(Result == QAPI_OK)
               ret_val = true;
            else
            {
               qapi_OMSM_Free(QAPI_OMSM_DEFAULT_AON_POOL, OMSM_CLIENT_ID_BLE_E);

#ifdef V2
               Display_Function_Error(BLE_Demo_Context.QCLI_Handle, "qapi_OMSM_Commit", Result);
#endif

               ret_val = false;
            }
         }
         else
         {
#ifdef V2
            Display_Function_Error(BLE_Demo_Context.QCLI_Handle, "qapi_OMSM_Alloc", Result);
#endif

            ret_val = false;
         }
      }
      else
      {
#ifdef V2
         QCLI_Printf(BLE_Demo_Context.QCLI_Handle, "Transition not allowed\n");
#endif
         ret_val = false;
      }
   }
   else
   {
      ret_val = true;
   }

   return(ret_val);
}

/**
   @brief Undoes preparation of the BLE demo for a mode change.
*/
void BLE_Cancel_Mode_Switch(void)
{
   if((BLE_Demo_Context.QCLI_Handle != NULL) && (BLE_Demo_Context.BluetoothStackID != 0))
   {
      qapi_OMSM_Free(QAPI_OMSM_DEFAULT_AON_POOL, OMSM_CLIENT_ID_BLE_E);
   }
}

/**
   @brief Finalizes the mode change for the BLE demo.
*/
void BLE_Exit_Mode(Operating_Mode_t Next_Mode)
{
   uint16_t                    Data_Size;
   DeviceInfo_t               *DeviceInfo;
   unsigned int                Index;
   qapi_Status_t               Result;
   BLE_Demo_Transition_Data_t *Transition_Data;

   if((BLE_Demo_Context.QCLI_Handle != NULL) && (BLE_Demo_Context.BluetoothStackID != 0))
   {
      Result = qapi_OMSM_Retrieve(QAPI_OMSM_DEFAULT_AON_POOL, OMSM_CLIENT_ID_BLE_E, &Data_Size, (void **)&Transition_Data);

      if((Result == QAPI_OK) && (Transition_Data != NULL))
      {
         if(Data_Size == sizeof(BLE_Demo_Transition_Data_t))
         {
            /* Save the memory needed to restore from this transition.  */
            Transition_Data->BluetoothStackID    = BLE_Demo_Context.BluetoothStackID;
            Transition_Data->GAPSInstanceID      = BLE_Demo_Context.GAPSInstanceID;
            Transition_Data->BLEParameters       = BLE_Demo_Context.BLEParameters;
            Transition_Data->LE_Parameters       = BLE_Demo_Context.LE_Parameters;
            Transition_Data->ContextFlags        = BLE_Demo_Context.ContextFlags;
            Transition_Data->ScanInProgress      = BLE_Demo_Context.ScanInProgress;
            Transition_Data->ER                  = BLE_Demo_Context.ER;
            Transition_Data->IR                  = BLE_Demo_Context.IR;
            Transition_Data->NumberPairedDevices = 0;

            /* Store as many paired devices as possible.                */
            for(Index=0,DeviceInfo=BLE_Demo_Context.DeviceInfoList;(Index<MAXIMUM_PAIRED_DEVICES)&&(DeviceInfo != NULL);DeviceInfo=DeviceInfo->NextDeviceInfoInfoPtr)
            {
               /* Check to see if this device has been paired with.     */
               if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_LTK_VALID)
               {
                  /* Copy this paired device to transition memory.      */
                  Transition_Data->PairedDevices[Index]                       = *DeviceInfo;
                  Transition_Data->PairedDevices[Index].NextDeviceInfoInfoPtr = NULL;

                  /* Increment the index and number of stored paired    */
                  /* devices.                                           */
                  Transition_Data->NumberPairedDevices++;
                  Index++;
               }
            }

            /* Attempt to commit the memory.                            */
            Result = qapi_OMSM_Commit(QAPI_OMSM_DEFAULT_AON_POOL, OMSM_CLIENT_ID_BLE_E);
         }
         else
         {
            qapi_OMSM_Free(QAPI_OMSM_DEFAULT_AON_POOL, OMSM_CLIENT_ID_BLE_E);
         }
      }
   }
}

