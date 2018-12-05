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

#ifndef __ZIGBEE_UTIL_H__
#define __ZIGBEE_UTIL_H__

#include "qcli_api.h"
#include "qurt_thread.h"

#include "qapi_zb.h"
#include "qapi_zb_cl_identify.h"
#include "log_util.h"

#define CO_ORDINATOR_DISTRIBUTION                             0    /* 0 for Centralized mode 1 is for Distribution */
#define ZIGBEE_REJOIN                                         0
#define ENABLE_ROUTER                                         1
#define ENABLE_ENDDEVICE                                      0
#define LIGHT_CLUSTER_ENDPOINT                              111 
#define CUSTOM_CLUSTER_ENDPOINT                             112
#define DIMMER_CLUSTER_ENDPOINT                             113
#define CLUSTER_ENDPOINT_TYPE_LIGHT                       1   /* Cluster Endpoint Type - 1: Light */
#define CLUSTER_ENDPOINT_TYPE_LIGHT_SWITCH                    2   /* Cluster Endpoint Type - 2: Light Switch */
#define CLUSTER_ENDPOINT_TYPE_DIMMABLE_LIGHT                  3
#define CLUSTER_ENDPOINT_TYPE_DIMMER_SWITCH                   4
#define CUSTOM_CLUSTER_SERVER                                12
#define CUSTOM_CLUSTER_CLIENT                                13
#define TRUE                                                  1
#define FALSE                                                 0
#define DELAY				                               3000
#define DEVICE_MODE_GROUP		                              1
#define DEVICE_MODE_NETWORK		                              2
#define DEVICE_MODE_EXTENDED		                        3
#define LIGHT_OFF			                                  0
#define LIGHT_ON                                              1
#define JSON_BUF_SIZE                                       400
#define CUSTOM_DATA_SIZE 				                    512
#define ENDDEVICE                                             1
/* The default PAN ID used by the ZigBee demo application. */
#define DEFAULT_ZIGBEE_PAN_ID                         (0xB89B)

/* The default end device time out value. */
#define DEFAULT_END_DEVICE_TIME_OUT                   (0xFF)

/* Channel mask used when forming a network. */
#define FORM_CHANNEL_MASK                             (0x07FFF800)

/* Channel mask used when joining a network. */
#define JOIN_CHANNEL_MASK                             (0x07FFF800)

/* The maximum number of registered devices for the demo's device list. */
#define DEV_ID_LIST_SIZE                              (4)

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

/* MAX ZB Key Size */
#define MAX_ZB_KEY_SIZE      QAPI_ZB_KEY_SIZE

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
  //  uint8_t          	EndDevBuf[CUSTOM_DATA_SIZE];
  //  uint8_t          	BreachBuf[CUSTOM_DATA_SIZE];
    uint8_t             *EndDevBuf;
    uint8_t             *BreachBuf;
    uint8_t             data_refresh_count;
} ZB_Device_ID_t;

typedef struct ZIGBEE_info {
    char mode;
    char master_key[MAX_ZB_KEY_SIZE + 1];
} ZIGBEE_info_t;


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
void DisplayVariableLengthValue(QCLI_Group_Handle_t Group_Handle, uint16_t Data_Length, const uint8_t *Data);



/** Zigbee function declarions */
int32_t Start_ZB_Coordinator(char *masterkey);
int32_t Start_ZB_Router(char *masterkey);
int32_t Start_ZB_EndDev(char *masterkey);

int32_t Zigbee_Initialize(void);
int32_t Zigbee_Form(uint32_t Security, uint32_t Distributed, uint32_t Mask, char *masterkey);
int32_t Zigbee_PermitJoin(uint8_t PermitTime);
int32_t Zigbee_SetExtAddress(uint64_t ExtAddr);
int32_t Zigbee_GetAddresses(void);
int32_t Zigbee_Join(uint32_t Coordinator, uint32_t Security, uint32_t ReJoin, uint32_t Mask, char *masterkey);
int32_t Zigbee_ShowDeviceList();
int32_t Zigbee_AddDevice(uint8_t device_mode, uint64_t ExtAddr, uint8_t Endpoint);
int32_t Zigbee_Light_OnOff(uint64_t ExtAddr, uint8_t state);
int32_t Zigbee_Light_Toggle(uint64_t ExtAddr);
void Zigbee_Read_Devices_Data(char *jsonbuf, uint32_t max_size);
void Zigbee_Get_Enddev_Sensor_Data(const uint8_t *Enddevbuf);
void zb_add_device_crd_signal(void);
int32_t Get_Enddev_Join_Confirm_Status(void);
void Read_zigbee_Devices_Data(char *data,uint32_t size);
int32_t Notify_breach_update_from_remote_device(char *);
int32_t Notify_breach_update_from_remote_device(char *);
int32_t Send_Remote_device_update_to_aws(char *);
void Zigbee_Set_Enddev_Sensor_Data(const uint8_t *);
void Zigbee_device_update(char *device_name, char *jsonbuf, uint32_t size);
void check_zigbee_devices_state(void);
int32_t Zigbee_PIR_Data_Send(void);
int Process_light(char *board_name, char *val);
int32_t rejoin_status();
void set_rejoin_status();
int32_t Set_Enddev_Join_Confirm_Status(uint32_t join_state);
#endif

