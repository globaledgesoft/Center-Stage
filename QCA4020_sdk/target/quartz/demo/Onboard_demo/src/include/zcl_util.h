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

#ifndef __ZCL_DEMO_H__
#define __ZCL_DEMO_H__

#include "pal.h"
#include "qapi_zb.h"
#include "qapi_zb_cl.h"

#define ZCL_DEMO_IGNORE_CLUSTER_ID                                      (0xFFFF)

#define ZCL_DEMO_MAX_ATTRIBUTES_PER_CLUSTER                             (20)
#define ZCL_CUSTOM_DEMO_CLUSTER_CLUSTER_ID                              (0xFC00)
#define CUSTOM_PAYLOAD_LENGTH                                           600

typedef enum
{
   ZCL_DEMO_CLUSTERTYPE_UNKNOWN,
   ZCL_DEMO_CLUSTERTYPE_CLIENT,
   ZCL_DEMO_CLUSTERTYPE_SERVER
} ZCL_Demo_ClusterType_t;

/*
   Forward define the ZCL_Demo_Cluster_Info_t structure.
*/
typedef struct ZCL_Demo_Cluster_Info_s ZCL_Demo_Cluster_Info_t;

/**
   @brief Prototype for a function to initialize a cluster demo.

   @param ZigBee_QCLI_Handle is the parent QCLI handle for demo.

   @return true if the ZigBee demo was initialized successfully, false
           otherwise.
*/
typedef qbool_t (*ZCL_Cluster_Demo_Init_Func_t)(QCLI_Group_Handle_t ZigBee_QCLI_Handle);

/**
   @brief Prototype for a function to create a cluster.

   @param Endpoint is the endpoint the cluster will be part of.
   @param PrivData is a pointer to the private data allocated for the cluster
                   demo.  This will be initaialized to NULL before the create
                   function is called so can be ignored if the demo has no
                   private data.  If set to a non-NULL value, the data will be
                   freed when the cluster is removed.

   @return The handle for the newly created function or NULL if there was an
           error.
*/
typedef qapi_ZB_Cluster_t (*ZCL_Cluster_Demo_Create_Func_t)(uint8_t Endpoint, void **PrivData);

/**
   @brief Prototype for a function that is called when a cluster is removed.

   Provided to allow the cluster demo to cleanup any extra resources.

   @param Cluster_Info is the information for the cluster being removed.
*/
typedef void (*ZCL_Cluster_Cleanup_CB_t)(ZCL_Demo_Cluster_Info_t *ClusterInfo);

/*
   Structure represents the information for a cluster.
*/
typedef struct ZCL_Demo_Cluster_Info_s
{
   qapi_ZB_Cluster_t         Handle;      /** Handle of the cluster. */
   uint16_t                  ClusterID;   /** ID of the cluster. */
   uint8_t                   Endpoint;    /** endpoint used by the cluster. */
   ZCL_Demo_ClusterType_t    ClusterType; /** Indicates the type of cluster as either server or client. */
   const char               *ClusterName; /** Name of the cluster being added. */
   const char               *DeviceName;  /** Device Name for the cluster. */
   void                     *PrivData;    /** Private data for the cluster. */
   ZCL_Cluster_Cleanup_CB_t  Cleanup_CB;  /** Function called when cluster is removed. */
} ZCL_Demo_Cluster_Info_t;

/**
   @brief Registers the ZigBee cluster commands with QCLI.
*/
qbool_t Initialize_ZCL_Demo(QCLI_Group_Handle_t ZigBee_QCLI_Handle);

/**
   @brief Function to add a cluster entry to a cluster list.

   @param Cluster_Info is the information for the cluster to add.

   @return The ClusterIndex of the newly added cluster or a negative value if
           there was an error.
*/
int16_t ZB_Cluster_AddCluster(const ZCL_Demo_Cluster_Info_t *Cluster_Info);

/**
   @brief Called when the stack is shutdown to cleanup the cluster list.
*/
void ZB_Cluster_Cleanup(void);

/**
   @brief Gets the cluster handle for a specified index.

   @param ClusterIndex is the index of the cluster being requested.
   @param ClusterID    is the expected ID of the cluster being requested.  Set
                       to 0xFFFF to ignore.

   @return The info structure for the cluster or NULL if it was not found.
*/
ZCL_Demo_Cluster_Info_t *ZCL_FindClusterByIndex(uint16_t ClusterIndex, uint16_t ClusterID);

/**
   @brief Finds a cluster with a matching ID and endpoint.

   @param Endpoint    is the endpoint for the cluster to find.
   @param ClusterID   is the ID fo the cluster to find.
   @param ClusterType is the type of cluster (server or client).

   @return The handle for the requested cluster or NULL if it was not found.
*/
ZCL_Demo_Cluster_Info_t *ZCL_FindClusterByEndpoint(uint8_t Endpoint, uint16_t ClusterID, ZCL_Demo_ClusterType_t ClusterType);

/**
   @brief Finds a cluster with a matching ID and endpoint.

   @param Handle is the handle of the cluster to find.

   @return The handle for the requested cluster or NULL if it was not found.
*/
ZCL_Demo_Cluster_Info_t *ZCL_FindClusterByHandle(qapi_ZB_Cluster_t Handle);

uint32_t ZCL_OnOff_On(uint8_t DeviceId, uint8_t ClEndPoint);
uint32_t ZCL_OnOff_Off(uint8_t DeviceId, uint8_t ClEndPoint);
uint32_t ZCL_OnOff_Toggle(uint8_t DeviceId, uint8_t ClEndPoint);
uint32_t Zigbee_CL_CreateEndPoint(uint8_t ClEndPoint, uint8_t ClEndPointType);
uint32_t ZCL_Custom_SendCommand(uint8_t CrdDeviceId, uint8_t CustomClEndPoint );
uint32_t ZCL_LevelControl_MoveToLevel(uint32_t DevId, uint32_t  DimmerEndPoint, qbool_t OnOff, uint8_t level, uint16_t Time);
int32_t ZCL_PIR_SendCommand(uint8_t CrdDeviceId, uint8_t CustomClEndPoint);
#endif

