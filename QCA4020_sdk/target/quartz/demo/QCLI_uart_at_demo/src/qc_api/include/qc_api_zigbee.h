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

#ifndef __ZIGBEE_DEMO_H__
#define __ZIGBEE_DEMO_H__

#include "qcli_api.h"

#include "qapi_zb.h"
#include "qapi_zb_cl_identify.h"
#include "malloc.h"
#include "string.h"
#include "stringl.h"
#include "qcli_api.h"
#include "qcli_util.h"
#include "qc_at_zcl.h"
#include "qapi_zb.h"
#include "qapi_zb_nwk.h"
#include "qapi_zb_bdb.h"
#include "qapi_zb_zdp.h"
#include "qapi_zb_cl_basic.h"
#include "qapi_zb_cl_identify.h"
#include "qapi_persist.h"
#include "qc_drv_zigbee.h"
#include "qc_api_main.h"
#include "qosa_util.h"
/* This enumeration defines the device types that can be added to the demo's
   device list. */
typedef enum
{
   DEVICE_ID_TYPE_UNUSED,
   DEVICE_ID_TYPE_NWK_ADDR,
   DEVICE_ID_TYPE_GROUP_ADDR
} Device_ID_Type_t;

/* This structure represents a registered device/group address. */
typedef struct ZB_Device_ID_s
{
   qbool_t             InUse;
   qapi_ZB_Addr_Mode_t Type;
   qapi_ZB_Addr_t      Address;
   uint8_t             Endpoint;
} ZB_Device_ID_t;

/* The number of scenes to have space allocated for the Scenes cluster. */
#define APP_MAX_NUM_SCENES                                              (4)

/* The macro defines the TX option used by the ZigBee demo. */
#define ZIGBEE_DEMO_TX_OPTION_WITH_SECURITY                             (QAPI_ZB_APSDE_DATA_REQUEST_TX_OPTION_SECURE_TRANSMISSION | \
                                                                         QAPI_ZB_APSDE_DATA_REQUEST_TX_OPTION_USE_NWK_KEY | \
                                                                         QAPI_ZB_APSDE_DATA_REQUEST_TX_OPTION_ACKNOWLEDGED_TRANSMISSION | \
                                                                         QAPI_ZB_APSDE_DATA_REQUEST_TX_OPTION_FRAGMENTATION_PERMITTED)

#define ZIGBEE_DEMO_TX_OPTION_WITHOUT_SECURITY                          (QAPI_ZB_APSDE_DATA_REQUEST_TX_OPTION_ACKNOWLEDGED_TRANSMISSION | \
                                                                         QAPI_ZB_APSDE_DATA_REQUEST_TX_OPTION_FRAGMENTATION_PERMITTED)


/* The following macros are used to read little endian data. */
#define READ_UNALIGNED_LITTLE_ENDIAN_UINT8(__src__)                     (((uint8_t *)(__src__))[0])

#define READ_UNALIGNED_LITTLE_ENDIAN_UINT16(__src__)                    ((uint16_t)((((uint16_t)(((uint8_t *)(__src__))[1])) << 8) | \
                                                                         ((uint16_t)(((uint8_t *)(__src__))[0]))))

#define READ_UNALIGNED_LITTLE_ENDIAN_UINT32(__src__)                    ((uint32_t)((((uint32_t)(((uint8_t *)(__src__))[3])) << 24) | \
                                                                         (((uint32_t)(((uint8_t *)(__src__))[2])) << 16) | \
                                                                         (((uint32_t)(((uint8_t *)(__src__))[1])) << 8) | \
                                                                         ((uint32_t)(((uint8_t *)(__src__))[0]))))

#define READ_UNALIGNED_LITTLE_ENDIAN_UINT64(__src__)                    ((uint64_t)((((uint64_t)(((uint8_t *)(__src__))[7])) << 56) | \
                                                                         (((uint64_t)(((uint8_t *)(__src__))[6])) << 48) | \
                                                                         (((uint64_t)(((uint8_t *)(__src__))[5])) << 40) | \
                                                                         (((uint64_t)(((uint8_t *)(__src__))[4])) << 32) | \
                                                                         (((uint32_t)(((uint8_t *)(__src__))[3])) << 24) | \
                                                                         (((uint32_t)(((uint8_t *)(__src__))[2])) << 16) | \
                                                                         (((uint32_t)(((uint8_t *)(__src__))[1])) << 8) | \
                                                                         ((uint32_t)(((uint8_t *)(__src__))[0]))))

/* The default PAN ID used by the ZigBee demo application. */
#define DEFAULT_ZIGBEE_PAN_ID                         (0xB89B)

/* The default end device time out value. */
#define DEFAULT_END_DEVICE_TIME_OUT                   (0xFF)

/* Channel mask used when forming a network. */
#define FORM_CHANNEL_MASK                             (0x07FFF800)

/* Channel mask used when joining a network. */
#define JOIN_CHANNEL_MASK                             (0x07FFF800)

/* The maximum number of registered devices for the demo's device list. */
#define DEV_ID_LIST_SIZE                              (8)

/* The number of scenes to have space allocated for the Scenes cluster. */
#define APP_MAX_NUM_SCENES                            (4)

/* The maxium length of an attribute being read and written.            */
#define MAXIMUM_ATTRIUBTE_LENGTH                                        (8)

/* Default ZigBee Link Key used during commissioning. */
#define DEFAULT_ZIGBEE_LINK_KEY                       {'Z', 'i', 'g', 'B', 'e', 'e', 'A', 'l', 'l', 'i', 'a', 'n', 'c', 'e', '0', '9'}

/* Capability falgs used for coordinators. */
#define COORDINATOR_CAPABILITIES                      (QAPI_MAC_CAPABILITY_FULL_FUNCTION | QAPI_MAC_CAPABILITY_MAINS_POWER | QAPI_MAC_CAPABILITY_RX_ON_IDLE | QAPI_MAC_CAPABILITY_ALLOCATE_ADDR)

/* Capability falgs used for end devices. */
#define END_DEVICE_CAPABILITIES                       (QAPI_MAC_CAPABILITY_ALLOCATE_ADDR)

/* Location and name for the ZigBee persistent storage. */
#define ZIGBEE_PERSIST_DIRECTORY                      ("/spinor/zigbee")
#define ZIGBEE_PERSIST_PREFIX                         ("pdata")
#define ZIGBEE_PERSIST_SUFFIX                         (".bin")

/* Structure representing the main ZigBee demo context information. */
typedef struct ZigBee_Demo_Context_s
{
   QCLI_Group_Handle_t   QCLI_Handle;                     /*< QCLI handle for the main ZigBee demo. */
   qapi_ZB_Handle_t      ZigBee_Handle;                   /*< Handle provided by the ZigBee stack when initialized. */
   ZB_Device_ID_t        DevIDList[DEV_ID_LIST_SIZE + 1]; /*< List of devices */
   uint8_t               ZCL_Sequence_Num;                /*< Sequence number used for sending packets. */
   qapi_Persist_Handle_t PersistHandle;                   /*< Persist handle used by the ZigBee demo. */
} ZigBee_Demo_Context_t;

/**
   @brief Registers the ZigBee interface commands with QCLI.
*/
void Initialize_ZigBee_Demo(void);

/**
   @brief Helper function to format the send information for a packet.

   @param DeviceIndex is the index of the device to be sent.
   @param SendInfo    is a pointer to where the send information will be
                      formatted upon successful return.

   @return true if the send info was formatted successfully, false otherwise.
*/
qbool_t Format_Send_Info_By_Device(uint32_t DeviceIndex, qapi_ZB_CL_General_Send_Info_t *SendInfo);

/**
   @brief Helper function to format the send information for a packet.

   @param ReceiveInfo is the receive information for an event.
   @param SendInfo    is a pointer to where the send information will be
                      formatted upon successful return.

   @return true if the send info was formatted successfully, false otherwise.
*/
qbool_t Format_Send_Info_By_Receive_Info(const qapi_ZB_CL_General_Receive_Info_t *ReceiveInfo, qapi_ZB_CL_General_Send_Info_t *SendInfo);

/**
   @brief Function to get a specified entry from the ZigBee demo's device list.

   @param DeviceID is the index of the device to retrieve.

   @return a pointer to the device list entry or NULL if either the DeviceID was
           not valid or not in use.
*/
ZB_Device_ID_t *GetDeviceListEntry(uint32_t DeviceID);

/**
   @brief Function to get the ZigBee stack's handle.

   @return The handle of the ZigBee stack.
*/
qapi_ZB_Handle_t GetZigBeeHandle(void);

/**
   @brief Function to get the next sequence number for sending packets.

   @return the next sequence number to be used for sending packets.
*/
uint8_t GetNextSeqNum(void);

/**
   @brief Function to get the QCLI handle for the ZigBee demo.

   @return The QCLI handled used by the ZigBee demo.
*/
QCLI_Group_Handle_t GetZigBeeQCLIHandle(void);

/**
   @brief Helper function that displays variable length value.

   @param Group_Handle is the QCLI group handle.
   @param Data_Length  is the length of the data to be displayed.
   @param Data         is the data to be displayed.
*/
void DisplayVariableLengthValue(uint16_t Data_Length, uint8_t *Data);

QCLI_Command_Status_t qc_api_cmd_ZB_Initialize(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZB_Shutdown(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZB_AddDevice(uint32_t Parameter_Count,  QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZB_RemoveDevice(uint32_t Parameter_Count,  QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZB_ShowDeviceList(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZB_Form(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_ZB_Join(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZB_Leave(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZB_LeaveReq(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_ZB_PermitJoin(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZB_BindRequest(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZB_EndBind(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZB_GetNIB(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZB_SetNIB(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZB_GetAIB(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZB_SetAIB(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZB_SetBIB(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZB_GetBIB(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZB_SetExtAddress(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZB_GetAddresses(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZB_CL_ListClusterTypes(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZB_CL_ListEndpointTypes(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZB_CL_CreateEndpoint(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZB_CL_RemoveEndpoint(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZB_CL_ListClusters(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZB_CL_ReadLocalAttribute(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZB_CL_WriteLocalAttribute(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZB_CL_ReadAttribute(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZB_CL_WriteAttribute(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZB_CL_DiscoverAttributes(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZB_CL_ConfigReport(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZB_CL_ReadReportConfig(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZCL_Basic_Reset(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZCL_Basic_Read(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZCL_Basic_Write(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZCL_Identify_Identify(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZCL_Identify_IdentifyQuery(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZCL_Groups_AddGroup(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZCL_Groups_ViewGroup(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZCL_Groups_GetGroupMembership(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZCL_Groups_RemoveGroup(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZCL_Groups_RemoveAllGroups(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZCL_Scenes_AddScene(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZCL_Scenes_ViewScene(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZCL_Scenes_RemoveScene(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZCL_Scenes_StoreScenes(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZCL_Scenes_RemoveAllScenes(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZCL_Scenes_RecallScenes(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZCL_Scenes_GetSceneMembership(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZCL_Scenes_CopyScene(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZCL_OnOff_On(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZCL_OnOff_Off(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZCL_OnOff_Toggle(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZCL_OnOff_SetSceneData(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZCL_LevelControl_MoveToLevel(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZCL_LevelControl_Move(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZCL_LevelControl_Step(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZCL_LevelControl_Stop(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZCL_Alarms_ResetAlarm(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZCL_Alarms_ResetAllAlarms(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZCL_Alarms_GetAlarm(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZCL_Alarms_ResetAlarmLog(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZCL_Alarms_Alarm(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZCL_Time_Read(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZCL_Time_Write(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZCL_Touchlink_Start(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZCL_Touchlink_Scan(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZCL_Touchlink_FactoryReset(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZCL_ColorControl_MoveToHue(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZCL_ColorControl_MoveHue(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZCL_ColorControl_StepHue(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZCL_ColorControl_MoveToSaturation(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZCL_ColorControl_MoveSaturation(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZCL_ColorControl_StepSaturation(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZCL_ColorControl_MoveToHueAndSaturation(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZCL_ColorControl_MoveToColor(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZCL_ColorControl_MoveColor(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZCL_ColorControl_StepColor(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZCL_ColorControl_MoveToColorTemp(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZCL_ColorControl_MoveColorTemp(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZCL_ColorControl_StepColorTemp(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZCL_ColorControl_StopMoveStep(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_cmd_ZCL_ColorControl_ColorLoopSet(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t qc_api_ZB_ClearPersist(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
#endif

