/*
 * Copyright (c) 2016-2017 Qualcomm Technologies, Inc.
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
#include "util.h"
#include "qcli_api.h"
#include "qcli_util.h"
#include "zigbee_util.h"
#include "zcl_util.h"

#include "qapi_zb.h"
#include "qapi_zb_cl.h"
#include "qapi_zb_cl_device_temp.h"

static void ZCL_DeviceTemp_Demo_Server_CB(qapi_ZB_Handle_t ZB_Handle, qapi_ZB_Cluster_t Cluster, qapi_ZB_CL_DeviceTemp_Server_Event_Data_t *EventData, uint32_t CB_Param);
static void ZCL_DeviceTemp_Demo_Client_CB(qapi_ZB_Handle_t ZB_Handle, qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_Custom_Cluster_Event_Data_t *EventData, uint32_t CB_Param);

/**
   @brief Handles callbacks for the device temp server cluster.

   @param ZB_Handle is the handle of the ZigBee instance.
   @param EventData is the information for the cluster event.
   @param CB_Param  is the user specified parameter for the callback function.
*/
static void ZCL_DeviceTemp_Demo_Server_CB(qapi_ZB_Handle_t ZB_Handle, qapi_ZB_Cluster_t Cluster, qapi_ZB_CL_DeviceTemp_Server_Event_Data_t *EventData, uint32_t CB_Param)
{
   if((ZB_Handle != NULL) && (Cluster != NULL) && (EventData != NULL))
   {
      QCLI_Printf(GetZigBeeQCLIHandle(), "Unhandled DeviceTemp server event %d.\n", EventData->Event_Type);
      QCLI_Display_Prompt();
   }
}

/**
   @brief Handles callbacks for the device temp client cluster.

   @param ZB_Handle is the handle of the ZigBee instance.
   @param EventData is the information for the cluster event.
   @param CB_Param  is the user specified parameter for the callback function.
*/
static void ZCL_DeviceTemp_Demo_Client_CB(qapi_ZB_Handle_t ZB_Handle, qapi_ZB_Cluster_t Cluster, const qapi_ZB_CL_Custom_Cluster_Event_Data_t *EventData, uint32_t CB_Param)
{
   if((ZB_Handle != NULL) && (Cluster != NULL) && (EventData != NULL))
   {
      QCLI_Printf(GetZigBeeQCLIHandle(), "Unhandled DeviceTemp client event %d.\n", EventData->Event_Type);
      QCLI_Display_Prompt();
   }
}

/**
   @brief Creates an DeviceTemp server cluster.

   @param Endpoint is the endpoint the cluster will be part of.
   @param PrivData is a pointer to the private data for the cluster demo.  This
                   will be initaialized to NULL before the create function is
                   called so can be ignored if the demo has no private data.

   @return The handle for the newly created function or NULL if there was an
           error.
*/
qapi_ZB_Cluster_t ZCL_DeviceTemp_Demo_Create_Server(uint8_t Endpoint, void **PrivData)
{
    qapi_ZB_Cluster_t         Ret_Val;
    qapi_Status_t             Result;
    qapi_ZB_Handle_t          ZigBee_Handle;
    qapi_ZB_CL_Cluster_Info_t ClusterInfo;

    ZigBee_Handle = GetZigBeeHandle();
    if(ZigBee_Handle != NULL)
    {
        memset(&ClusterInfo, 0, sizeof(qapi_ZB_CL_Cluster_Info_t));
        ClusterInfo.Endpoint = Endpoint;

        Result = qapi_ZB_CL_DeviceTemp_Create_Server(ZigBee_Handle, &Ret_Val, &ClusterInfo, ZCL_DeviceTemp_Demo_Server_CB, 0);
        if(Result != QAPI_OK)
        {
            Display_Function_Error(GetZigBeeQCLIHandle(), "qapi_ZB_CL_DeviceTemp_Create_Server", Result);
            Ret_Val = NULL;
        }
    }
    else
    {
        QCLI_Printf(GetZigBeeQCLIHandle(), "ZigBee stack is not initialized.\n");
        Ret_Val = NULL;
    }

    return(Ret_Val);
}

/**
   @brief Creates an DeviceTemp client cluster.

   @param Endpoint is the endpoint the cluster will be part of.
   @param PrivData is a pointer to the private data for the cluster demo.  This
                   will be initaialized to NULL before the create function is
                   called so can be ignored if the demo has no private data.

   @return The handle for the newly created function or NULL if there was an
           error.
*/
qapi_ZB_Cluster_t ZCL_DeviceTemp_Demo_Create_Client(uint8_t Endpoint, void **PrivData)
{
   qapi_ZB_Cluster_t         Ret_Val;
   qapi_Status_t             Result;
   qapi_ZB_Handle_t          ZigBee_Handle;
   qapi_ZB_CL_Cluster_Info_t ClusterInfo;

   ZigBee_Handle = GetZigBeeHandle();
   if(ZigBee_Handle != NULL)
   {
      memset(&ClusterInfo, 0, sizeof(qapi_ZB_CL_Cluster_Info_t));
      ClusterInfo.Endpoint       = Endpoint;
      ClusterInfo.AttributeCount = 0;
      ClusterInfo.AttributeList  = NULL;

      Result = qapi_ZB_CL_Create_Cluster(ZigBee_Handle, &Ret_Val, QAPI_ZB_CL_CLUSTER_ID_TEMPERATURE_CONFIG, &ClusterInfo, QAPI_ZB_CL_FRAME_DIRECTION_TO_CLIENT_E, ZCL_DeviceTemp_Demo_Client_CB, 0);
      if(Result != QAPI_OK)
      {
         Display_Function_Error(GetZigBeeQCLIHandle(), "qapi_ZB_CL_Create_Cluster", Result);
         Ret_Val = NULL;
      }
   }
   else
   {
      QCLI_Printf(GetZigBeeQCLIHandle(), "ZigBee stack is not initialized.\n");
      Ret_Val = NULL;
   }

   return(Ret_Val);
}

