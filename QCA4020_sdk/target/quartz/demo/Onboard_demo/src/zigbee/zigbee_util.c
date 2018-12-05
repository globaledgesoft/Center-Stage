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

#include "malloc.h"
#include "string.h"
#include "stringl.h"
#include "qcli_api.h"
#include "qcli_util.h"
#include "zigbee_util.h"
#include "zcl_util.h"
#include "qapi_zb.h"
#include "qapi_zb_nwk.h"
#include "qapi_zb_bdb.h"
#include "qapi_zb_zdp.h"
#include "qapi_zb_cl_basic.h"
#include "qapi_zb_cl_identify.h"
#include "qapi_persist.h"
#include "qurt_thread.h"
#include "onboard.h"
#include "led_utils.h"
#include "qurt_mutex.h"
#include "sensor_json.h"
#include "offline.h"
#include "ble_util.h"

#ifdef AWS_IOT
#include "aws_util.h"
#endif

/* The default PAN ID used by the ZigBee demo application. */
#define DEFAULT_ZIGBEE_PAN_ID                         (0xB89B)

/* The default end device time out value. */
#define DEFAULT_END_DEVICE_TIME_OUT                   (0xFF)

/* Channel mask used when forming a network. */
#define FORM_CHANNEL_MASK                             (0x07FFF800)

/* Channel mask used when joining a network. */
#define JOIN_CHANNEL_MASK                             (0x07FFF800)


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

#define MONITOR_COUNT                                 45

/* Structure representing the main ZigBee demo context information. */
typedef struct ZigBee_Demo_Context_s
{
    QCLI_Group_Handle_t   QCLI_Handle;                     /*< QCLI handle for the main ZigBee demo. */
    qapi_ZB_Handle_t      ZigBee_Handle;                   /*< Handle provided by the ZigBee stack when initialized. */
    ZB_Device_ID_t        DevIDList[DEV_ID_LIST_SIZE + 1]; /*< List of devices */
    uint8_t               ZCL_Sequence_Num;                /*< Sequence number used for sending packets. */
    qapi_Persist_Handle_t PersistHandle;                   /*< Persist handle used by the ZigBee demo. */
} ZigBee_Demo_Context_t;

/* The ZigBee demo context. */
static ZigBee_Demo_Context_t ZigBee_Demo_Context;

/* Security information used by the demo when joining or forming a network. */
static qapi_ZB_Security_t Default_ZB_Secuity =
{
    QAPI_ZB_SECURITY_LEVEL_NONE_E,                     /* Security level 0.       */
    true,                                              /* Use Insecure Rejoin.    */
    0,                                                 /* Trust center address.   */
    DEFAULT_ZIGBEE_LINK_KEY,                           /* Preconfigured Link Key. */
    {0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7,
        0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF},  /* Distributed global key. */
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  /* Network Key.            */
};
int32_t rejoin=0;


#ifdef ZIGBEE_COMMAND_LIST
static QCLI_Command_Status_t cmd_ZB_RemoveDevice(uint32_t Parameter_Count,  QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t cmd_ZB_Reconnect(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t cmd_ZB_Leave(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t cmd_ZB_LeaveReq(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t cmd_ZB_BindRequest(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t cmd_ZB_EndBind(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t cmd_ZB_UnbindRequest(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t cmd_ZB_GetNIB(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t cmd_ZB_SetNIB(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t cmd_ZB_GetAIB(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t cmd_ZB_SetAIB(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t cmd_ZB_SetBIB(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t cmd_ZB_GetBIB(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t cmd_ZB_ClearPersist(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static void ZB_Persist_Notify_CB(qapi_ZB_Handle_t ZB_Handle, uint32_t CB_Param);
#endif
static void ZB_ZDP_Event_CB(qapi_ZB_Handle_t ZB_Handle, const qapi_ZB_ZDP_Event_t *ZDP_Event_Data, uint32_t CB_Param);
static void ZB_Event_CB(qapi_ZB_Handle_t ZB_Handle, const qapi_ZB_Event_t *ZB_Event, uint32_t CB_Param);

static uint32_t EndDev_Join_Confirm_Status;
static uint64_t CrdExtAddr;
/**
  @brief Starts zigbee device based on mode.

  @param :  Mode is a zigbee device mode Coordinator/Router/Enddevice
  @Param :  Master_key is a security key to be verified at the time of joining

  @return
  - SUCCESS indicates the command is executed successfully.
  - FAILURE indicates the command is failed to execute.
 */
qurt_mutex_t Json_buf_lock;

int32_t Start_zigbee(char Mode, char *Master_key)
{
    int32_t Ret_Val = FAILURE;

    LOG_INFO("Starting Zigbee\n");
    switch (Mode)
    {
        case 'c':
        case 'C':
            LOG_INFO("Starting ZB CO-Ordinator\n");
#ifdef OFFLINE
            enable_cordinator_fn();
#endif
            qurt_mutex_create(&Json_buf_lock);
            Ret_Val = Start_ZB_Coordinator(Master_key);
            if (Ret_Val == FAILURE)
                qurt_mutex_delete(&Json_buf_lock);
            break;

        case 'r' :
        case 'R' :
            LOG_INFO("\n\n Starting ZB Router/ \n\n");
#ifdef OFFLINE
            enable_advertise();
#endif
            Ret_Val = Start_ZB_Router(Master_key);
            break;

        case 'e' :
        case 'E' :
            LOG_INFO("\n\n Starting ZB End Device/\n\n");
#ifdef OFFLINE
                        enable_advertise();
#endif
            Ret_Val =  Start_ZB_EndDev(Master_key);
            break;

        default :
            LOG_INFO("\n\n wrong option sent for ZB opeartating mode/\n\n");

    }

    return Ret_Val;
}

/** 
  @brief Get rejoin status
 */
int32_t rejoin_status(void)
{
    return rejoin;
}   

/**
  @brief Get rejoin status
 */
void set_rejoin_status(void)
{
    rejoin = 1;
}
/**
  @brief Rejoins the device with zigbee network.

  @param :  Mode is a zigbee device mode Coordinator/Router/Enddevice
  @Param :  Master_key is a security key to be verified at the time of joining

  @return
  - SUCCESS indicates the command is executed successfully.
  - FAILURE indicates the command is failed to execute.
 */
int32_t rejoin_zigbee(char Mode, char *Master_key)
{
    qurt_thread_sleep(300);
    switch (Mode)
    {
        case 'r':
        case 'R':
            return Zigbee_Join(ENABLE_ROUTER, ZIGBEE_SECURITY, rejoin^=ZIGBEE_REJOIN, ZIGBEE_CHANNEL_MASK, Master_key);
            break;
        case 'e':
        case 'E':
            return Zigbee_Join(ENABLE_ENDDEVICE, ZIGBEE_SECURITY, rejoin^=ZIGBEE_REJOIN, ZIGBEE_CHANNEL_MASK, Master_key);
            break;
    }
    return SUCCESS;
}


int32_t Get_Enddev_Join_Confirm_Status()
{
    if(EndDev_Join_Confirm_Status)
        return SUCCESS;
    else
        return FAILURE;
}

int32_t Set_Enddev_Join_Confirm_Status(uint32_t join_state)
{
    EndDev_Join_Confirm_Status = join_state;

    return EndDev_Join_Confirm_Status;
}

/**
  @brief Initializes the ZigBee interface.
  @param :  void

  @return
  - SUCCESS indicates the command is executed successfully.
  - FAILURE indicates the command is failed to execute.
 */
int32_t Zigbee_Initialize()
{
    int32_t             Ret_Val;
    qapi_Status_t       Result;
    uint64_t            Extended_Address;

    /* Verify the ZigBee layer had not been initialized yet. */
    if (ZigBee_Demo_Context.ZigBee_Handle == NULL)
    {
        Result = qapi_ZB_Initialize(&(ZigBee_Demo_Context.ZigBee_Handle), ZB_Event_CB, 0);

        if ((Result == SUCCESS) && (ZigBee_Demo_Context.ZigBee_Handle != NULL))
        {
            /* Register the ZDP callback. */
            Result = qapi_ZB_ZDP_Register_Callback(ZigBee_Demo_Context.ZigBee_Handle, ZB_ZDP_Event_CB, 0);
            if (Result == SUCCESS)
            {
                Result = qapi_ZB_Get_Extended_Address(ZigBee_Demo_Context.ZigBee_Handle, &Extended_Address);
                if (Result == SUCCESS)
                {

                    LOG_INFO("ZigBee stack initialized.\n");
                    LOG_INFO("Extended Address: %08X%08X\n", (uint32_t)(Extended_Address >> 32), (uint32_t)Extended_Address);
                    LOG_INFO(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
                    LOG_INFO("Zigbehandle: %p\n", ZigBee_Demo_Context.ZigBee_Handle);
                    Ret_Val = SUCCESS;
                }
                else
                {
                    LOG_ERROR("Failed qapi_ZB_Get_Extended_Address\n");
                    Ret_Val = FAILURE;
                }
            }
            else
            {
                LOG_ERROR("Failed qapi_ZB_ZDP_Register_Callback\n");
                Ret_Val = FAILURE;
            }

            if (Ret_Val != SUCCESS)
            {
                qapi_ZB_Shutdown(ZigBee_Demo_Context.ZigBee_Handle);
                ZigBee_Demo_Context.ZigBee_Handle = NULL;

            }
        }
        else
        {
            LOG_ERROR("FAILED qapi_ZB_Initialize\n");
            Ret_Val = FAILURE;
        }
    }
    else
    {
        LOG_WARN("ZigBee stack already initialized.\n");
        Ret_Val = FAILURE;
    }

    return (Ret_Val);
}

#ifdef AWS_IOT
int32_t Read_Zigbee_Devices_Data(char *jsonbuf, uint32_t max_size)
{
    int Index;

    LOG_INFO("\n-----Remote Device Data--------\n");
    for(Index = 1; Index <= DEV_ID_LIST_SIZE; Index++)
    {
        if ((ZigBee_Demo_Context.DevIDList[Index].InUse == TRUE) && (ZigBee_Demo_Context.DevIDList[Index].Endpoint == CUSTOM_CLUSTER_ENDPOINT))
        {
            qurt_mutex_lock(&Json_buf_lock);
            if (strlen((const char *)ZigBee_Demo_Context.DevIDList[Index].EndDevBuf) > max_size)
            {
                qurt_mutex_unlock(&Json_buf_lock);
                break;
            }
            if (strlen((const char *)ZigBee_Demo_Context.DevIDList[Index].BreachBuf)) 
            {
                memset(jsonbuf, 0, max_size);
                snprintf(jsonbuf, max_size, "%s", ZigBee_Demo_Context.DevIDList[Index].BreachBuf);
                memset(ZigBee_Demo_Context.DevIDList[Index].BreachBuf, 0, CUSTOM_DATA_SIZE);
                Notify_breach_update_from_remote_device(jsonbuf + strlen(BREACHED));


            }
            if (strlen((const char *)ZigBee_Demo_Context.DevIDList[Index].EndDevBuf)) 
            {
                memset(jsonbuf, 0, max_size);
                snprintf(jsonbuf, max_size, "%s", ZigBee_Demo_Context.DevIDList[Index].EndDevBuf);
                memset(ZigBee_Demo_Context.DevIDList[Index].EndDevBuf, 0, CUSTOM_DATA_SIZE);
                Send_Remote_device_update_to_aws(jsonbuf);
            }
            qurt_mutex_unlock(&Json_buf_lock);
        }
    }
    LOG_INFO("\n----------------------------\n");
    return SUCCESS;
}
#endif

void Get_Enddev_Short_Mac_Addr(const uint8_t *Custom_Enddevbuf, uint8_t *EndDevShortAddr)
{
    uint8_t i;

    for (i = 0; i < strlen((const char *)Custom_Enddevbuf); i++ )
    {
        if (Custom_Enddevbuf[i] == '_')
        {
            snprintf((char *)EndDevShortAddr, 7,"%s", (Custom_Enddevbuf + i + 1));
            break;
        }
    }
    LOG_INFO("\n\n last 3 Bytes from custom  EndDevShortAddr:%s\n ",EndDevShortAddr);

}

void Zigbee_device_update(char *device_name, char *jsonbuf, uint32_t size)
{   
    uint8_t EndDevShortAddr[7] = {0};
    uint8_t Enddev_shortmac[7] = {0};
    uint32_t Index;

    Get_Enddev_Short_Mac_Addr((const uint8_t *)device_name, EndDevShortAddr);
    for(Index = 1; Index <= DEV_ID_LIST_SIZE; Index++)
    {
        snprintf((char *) Enddev_shortmac, 7, "%06llx", ((~(~0 << 24)) & ZigBee_Demo_Context.DevIDList[Index].Address.ExtendedAddress));
        LOG_INFO("Enddev shortmac: %s\n", Enddev_shortmac);
        if((ZigBee_Demo_Context.DevIDList[Index].InUse == TRUE) && !strcmp((const char *)Enddev_shortmac, (const char *)EndDevShortAddr))
        {
            if (ZigBee_Demo_Context.DevIDList[Index].Endpoint == CUSTOM_CLUSTER_ENDPOINT)
            {
                snprintf(jsonbuf, size, "%s", (char *)ZigBee_Demo_Context.DevIDList[Index].EndDevBuf);
                memset(ZigBee_Demo_Context.DevIDList[Index].EndDevBuf, 0, CUSTOM_DATA_SIZE);
            }
        }
    }
}

int32_t fill_remote_device_info(char *json_buf, uint32_t Max_size)
{
    uint8_t Enddev_shortmac[7] = {0};
    char device_name[128] = { 0 };
    uint32_t Index;
    uint32_t rem_size;
    for(Index = 1; Index <= DEV_ID_LIST_SIZE; Index++)
    {
        if ((ZigBee_Demo_Context.DevIDList[Index].InUse == TRUE) && ZigBee_Demo_Context.DevIDList[Index].Endpoint == CUSTOM_CLUSTER_ENDPOINT)
        {
            rem_size = Max_size - strlen(json_buf);
            memset(device_name, 0, sizeof(device_name));
            memset(Enddev_shortmac, 0, sizeof(Enddev_shortmac));
            snprintf((char *) Enddev_shortmac, 7, "%06llx", ((~(~0 << 24)) & ZigBee_Demo_Context.DevIDList[Index].Address.ExtendedAddress));
//            snprintf(device_name, sizeof(device_name), "QCA4024_%s", Enddev_shortmac);
            if (strlen((const char *)ZigBee_Demo_Context.DevIDList[Index].EndDevBuf))
            {
                extract_device_name((char *)ZigBee_Demo_Context.DevIDList[Index].EndDevBuf, device_name);
                LOG_INFO("device_name : %s\n", device_name);
                snprintf((json_buf + strlen(json_buf)), rem_size,"{\"dName\":\"%s\"},", device_name);
            }
        }
    }
    return SUCCESS;
}

void check_zigbee_devices_state(void)
{
    uint32_t Index;
    
    for(Index = 1; Index <= DEV_ID_LIST_SIZE; Index++)
    {
        if((ZigBee_Demo_Context.DevIDList[Index].InUse == TRUE) && ZigBee_Demo_Context.DevIDList[Index].Endpoint == CUSTOM_CLUSTER_ENDPOINT)
        {
            if(ZigBee_Demo_Context.DevIDList[Index].data_refresh_count <= 0)
            {
                LOG_INFO("Device removed: %d\n", ZigBee_Demo_Context.DevIDList[Index].Address.ExtendedAddress);
                qurt_thread_sleep(100);
                ZigBee_Demo_Context.DevIDList[Index].InUse = FALSE;
                ZigBee_Demo_Context.DevIDList[Index].Address.ExtendedAddress = 0;
                if (ZigBee_Demo_Context.DevIDList[Index].EndDevBuf)
                {
                    free(ZigBee_Demo_Context.DevIDList[Index].EndDevBuf);
                    ZigBee_Demo_Context.DevIDList[Index].EndDevBuf = NULL;
                }
                if (ZigBee_Demo_Context.DevIDList[Index].BreachBuf)
                {
                    free(ZigBee_Demo_Context.DevIDList[Index].BreachBuf);
                    ZigBee_Demo_Context.DevIDList[Index].BreachBuf = NULL;
                }
                LOG_INFO("Device removed: ");
                qurt_thread_sleep(100);

            }
            else
            {
                LOG_INFO("Data refrest count = %d\n", ZigBee_Demo_Context.DevIDList[Index].data_refresh_count);
                ZigBee_Demo_Context.DevIDList[Index].data_refresh_count = 0;
            } 
        }
    }
}

void Zigbee_Set_Enddev_Sensor_Data(const uint8_t *Custom_Enddevbuf)
{

    uint8_t EndDevShortAddr[7] = {0};
    uint8_t Enddev_shortmac[7] = {0};
    uint32_t Index;
    uint32_t str_len = strlen((const char *)Custom_Enddevbuf);
    uint32_t breach_buf = 0;
    if (strstr((char *)Custom_Enddevbuf, BREACHED))
    {
        LOG_INFO("Breach value is set\n");
        breach_buf = 1;

    }
    /*    extract DevMacAddr 3 byte from Custom_EndDevBuf  and fill EndDevShortAddr[]*/
    Get_Enddev_Short_Mac_Addr(Custom_Enddevbuf, EndDevShortAddr);

    for(Index = 1; Index <= DEV_ID_LIST_SIZE; Index++)
    {

        snprintf((char *) Enddev_shortmac, 7, "%06llx", ((~(~0 << 24)) & ZigBee_Demo_Context.DevIDList[Index].Address.ExtendedAddress));
        LOG_INFO("Enddev short mac: %s\n", Enddev_shortmac);
        if((ZigBee_Demo_Context.DevIDList[Index].InUse == TRUE) && !strcmp((const char *)Enddev_shortmac, (const char *)EndDevShortAddr))
        {
            LOG_INFO("End point: %d\n", ZigBee_Demo_Context.DevIDList[Index].Endpoint);
            LOG_INFO("Cluster endpont: %d\n", CUSTOM_CLUSTER_ENDPOINT);
            if (ZigBee_Demo_Context.DevIDList[Index].Endpoint == CUSTOM_CLUSTER_ENDPOINT)
            {

                LOG_INFO("Endevice_Buffer_from_co ordinator: %s len %d \n", Custom_Enddevbuf, str_len);


                qurt_mutex_lock(&Json_buf_lock);

                if (breach_buf)
                {
                    memset(ZigBee_Demo_Context.DevIDList[Index].BreachBuf, 0, CUSTOM_DATA_SIZE);
                    snprintf((char *)ZigBee_Demo_Context.DevIDList[Index].BreachBuf, str_len+1, "%s", Custom_Enddevbuf);
#ifdef OFFLINE
                    signal_set_breach_update((char *)ZigBee_Demo_Context.DevIDList[Index].BreachBuf);  
#endif
                }
                else
                {
                    memset(ZigBee_Demo_Context.DevIDList[Index].EndDevBuf, 0, CUSTOM_DATA_SIZE);
                    snprintf((char *)ZigBee_Demo_Context.DevIDList[Index].EndDevBuf, str_len+1, "%s", Custom_Enddevbuf);
                    ZigBee_Demo_Context.DevIDList[Index].data_refresh_count++;
                }
                qurt_mutex_unlock(&Json_buf_lock);
                break;
            }
        }

    }
}



#ifdef ZIGBEE_COMMAND_LIST
int32_t shutdown_zigbee(void)
{
    return ZB_Shutdown();
}

/**
  @brief Executes the "Shutdown" command to shut down the ZigBee interface.

  @param Parameter_Count is number of elements in Parameter_List.
  @param Parameter_List is list of parsed arguments associate with this
  command.

  @return
  - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
  - QCLI_STATUS_ERROR_E   indicates the command is failed to execute.
 */
static int32_t ZB_Shutdown(void)
{
    int32_t Ret_Val;

    LOG_INFO("%s %d\n", __func__, __LINE__);
    LOG_INFO("Zigbee handle: %p\n", ZigBee_Demo_Context.ZigBee_Handle);
    /* Verify the ZigBee layer had been initialized. */
    if(ZigBee_Demo_Context.ZigBee_Handle != NULL)
    {
        /* Cleanup the clusters have have been created. */
    LOG_INFO("%s %d\n", __func__, __LINE__);
        ZB_Cluster_Cleanup();

    LOG_INFO("%s %d\n", __func__, __LINE__);
        /* Shutdown the ZigBee stack. */
        qapi_ZB_Shutdown(ZigBee_Demo_Context.ZigBee_Handle);
    LOG_INFO("%s %d\n", __func__, __LINE__);
        ZigBee_Demo_Context.ZigBee_Handle = NULL;

        if(ZigBee_Demo_Context.PersistHandle != NULL)
        {
            qapi_Persist_Cleanup(ZigBee_Demo_Context.PersistHandle);
        }

        LOG_INFO("Succesfully shutdown\n");
        Ret_Val = SUCCESS;
    }
    else
    {
        LOG_INFO("ZigBee not initialized.\n");
        Ret_Val = FAILURE;
    }

    return(Ret_Val);
}

/**
  @brief Executes the "RemoveDevice" command to remove an address to the demo's
  device list.

  Parameter_List[0] (1-DEV_ID_LIST_SIZE) is the index of the device to remove.
  Note that address zero is reserved for the NULL address and
  cannot be removed.

  @param Parameter_Count is number of elements in Parameter_List.
  @param Parameter_List is list of parsed arguments associate with this
  command.

  @return
  - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
  - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
  - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
  command.
 */
static QCLI_Command_Status_t cmd_ZB_RemoveDevice(uint32_t Parameter_Count,  QCLI_Parameter_t *Parameter_List)
{
    QCLI_Command_Status_t Ret_Val;
    uint32_t              Index;

    if(ZigBee_Demo_Context.ZigBee_Handle != 0)
    {
        /* Check the parameters. */
        if((Parameter_Count >= 1) &&
                (Verify_Integer_Parameter(&Parameter_List[0], 1, DEV_ID_LIST_SIZE)))
        {
            Index = Parameter_List[0].Integer_Value;

            /* Make sure the device is actually in use. */
            if(ZigBee_Demo_Context.DevIDList[Index].InUse)
            {
                /* Clear the device mapping. */
                memset(&ZigBee_Demo_Context.DevIDList[Index], 0, sizeof(ZB_Device_ID_t));
                Ret_Val = QCLI_STATUS_SUCCESS_E;
            }
            else
            {
                QCLI_Printf(ZigBee_Demo_Context.QCLI_Handle, "Device ID is not in use.\n");
                Ret_Val = QCLI_STATUS_ERROR_E;
            }
        }
        else
        {
            Ret_Val = QCLI_STATUS_USAGE_E;
        }
    }
    else
    {
        QCLI_Printf(ZigBee_Demo_Context.QCLI_Handle, "ZigBee stack is not initialized.\n");
        Ret_Val = QCLI_STATUS_ERROR_E;
    }

    return(Ret_Val);
}

/**
  @brief Executes the "Reconnect" command to reconnect to the ZigBee network.

  @param Parameter_Count is number of elements in Parameter_List.
  @param Parameter_List is list of parsed arguments associate with this
  command.

  @return
  - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
  - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
  - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
  command.
 */
static QCLI_Command_Status_t cmd_ZB_Reconnect(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    QCLI_Command_Status_t Ret_Val;
    qapi_Status_t         Result;
    qbool_t               TCRejoin;

    /* Verify the ZigBee layer had been initialized. */
    if(ZigBee_Demo_Context.ZigBee_Handle != NULL)
    {
        TCRejoin = false;
        Ret_Val = QCLI_STATUS_SUCCESS_E;
        if(Parameter_Count >= 1)
        {
            if(Verify_Integer_Parameter(&(Parameter_List[0]), 0, 1))
            {
                TCRejoin = (qbool_t)(Parameter_List[0].Integer_Value != 0);
            }
            else
            {
                TCRejoin = QCLI_STATUS_USAGE_E;
            }
        }

        if(Ret_Val == QCLI_STATUS_SUCCESS_E)
        {
            Result = qapi_ZB_Reconnect(ZigBee_Demo_Context.ZigBee_Handle, TCRejoin);

            if(Result == QAPI_OK)
            {
                Display_Function_Success(ZigBee_Demo_Context.QCLI_Handle, "qapi_ZB_Reconnect");
                Ret_Val = QCLI_STATUS_SUCCESS_E;
            }
            else
            {
                Display_Function_Error(ZigBee_Demo_Context.QCLI_Handle, "qapi_ZB_Reconnect", Result);
                Ret_Val = QCLI_STATUS_ERROR_E;
            }
        }
    }
    else
    {
        QCLI_Printf(ZigBee_Demo_Context.QCLI_Handle, "ZigBee stack is not initialized.\n");
        Ret_Val = QCLI_STATUS_ERROR_E;
    }

    return(Ret_Val);
}

/**
  @brief Executes the "Leave" to tell the local device to leave the network.

  @param Parameter_Count is number of elements in Parameter_List.
  @param Parameter_List is list of parsed arguments associate with this
  command.

  @return
  - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
  - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
  - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
  command.
 */
static QCLI_Command_Status_t cmd_ZB_Leave(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    QCLI_Command_Status_t Ret_Val;
    qapi_Status_t         Result;

    /* Verify the ZigBee layer had been initialized. */
    if(ZigBee_Demo_Context.ZigBee_Handle != NULL)
    {
        Result = qapi_ZB_Leave(ZigBee_Demo_Context.ZigBee_Handle);

        if(Result == QAPI_OK)
        {
            Display_Function_Success(ZigBee_Demo_Context.QCLI_Handle, "qapi_ZB_Leave");
            Ret_Val = QCLI_STATUS_SUCCESS_E;
        }
        else
        {
            Display_Function_Error(ZigBee_Demo_Context.QCLI_Handle, "qapi_ZB_Leave", Result);
            Ret_Val = QCLI_STATUS_ERROR_E;
        }
    }
    else
    {
        QCLI_Printf(ZigBee_Demo_Context.QCLI_Handle, "ZigBee stack is not initialized.\n");
        Ret_Val = QCLI_STATUS_ERROR_E;
    }

    return(Ret_Val);
}

/**
  @brief Executes the "LeaveReq" command to send a leave request.

  Parameter_List[0] is the device ID to send the request to (NWK Address)
  Parameter_List[1] is the device ID for the device that needs to leave
  (Extended Address).
  Parameter_List[1] is a flag indicating if the device should remove its
  children.
  Parameter_List[2] is a flag indicating if the device should attempt to rejoin
  the network.

  @param Parameter_Count is number of elements in Parameter_List.
  @param Parameter_List is list of parsed arguments associate with this
  command.

  @return
  - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
  - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
  - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
  command.
 */
static QCLI_Command_Status_t cmd_ZB_LeaveReq(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    QCLI_Command_Status_t   Ret_Val;
    qapi_Status_t           Result;
    ZB_Device_ID_t         *TargetDevice;
    ZB_Device_ID_t         *LeaveDevice;
    qbool_t                 RemoveChildren;
    qbool_t                 Rejoin;

    if(ZigBee_Demo_Context.ZigBee_Handle != NULL)
    {
        /* Check the parameters. */
        if((Parameter_Count >= 4) &&
                (Verify_Integer_Parameter(&(Parameter_List[2]), 0, 1)) &&
                (Verify_Integer_Parameter(&(Parameter_List[3]), 0, 1)))
        {
            /* Get the device information. */
            TargetDevice   = GetDeviceListEntry(Parameter_List[0].Integer_Value);
            LeaveDevice    = GetDeviceListEntry(Parameter_List[1].Integer_Value);
            RemoveChildren = (qbool_t)(Parameter_List[2].Integer_Value);
            Rejoin         = (qbool_t)(Parameter_List[3].Integer_Value);

            if((TargetDevice != NULL) && (LeaveDevice != NULL))
            {
                /* Verify the address types for each specified device is valid. */
                if((TargetDevice->Type == QAPI_ZB_ADDRESS_MODE_SHORT_ADDRESS_E) && (LeaveDevice->Type == QAPI_ZB_ADDRESS_MODE_EXTENDED_ADDRESS_E))
                {
                    Result = qapi_ZB_ZDP_Mgmt_Leave_Req(ZigBee_Demo_Context.ZigBee_Handle, TargetDevice->Address.ShortAddress, LeaveDevice->Address.ExtendedAddress, RemoveChildren, Rejoin);
                    if(Result == QAPI_OK)
                    {
                        Display_Function_Success(ZigBee_Demo_Context.QCLI_Handle, "qapi_ZB_ZDP_Mgmt_Leave_Req");
                        Ret_Val = QCLI_STATUS_SUCCESS_E;
                    }
                    else
                    {
                        Display_Function_Error(ZigBee_Demo_Context.QCLI_Handle, "qapi_ZB_ZDP_Mgmt_Leave_Req", Result);
                        Ret_Val = QCLI_STATUS_ERROR_E;
                    }
                }
                else
                {
                    QCLI_Printf(ZigBee_Demo_Context.QCLI_Handle, "Specified device is not a valid type.\n");
                    Ret_Val = QCLI_STATUS_USAGE_E;
                }
            }
            else
            {
                QCLI_Printf(ZigBee_Demo_Context.QCLI_Handle, "Device ID is not valid.\n");
                Ret_Val = QCLI_STATUS_USAGE_E;
            }
        }
        else
        {
            Ret_Val = QCLI_STATUS_USAGE_E;
        }
    }
    else
    {
        QCLI_Printf(ZigBee_Demo_Context.QCLI_Handle, "ZigBee stack is not initialized.\n");
        Ret_Val = QCLI_STATUS_ERROR_E;
    }

    return(Ret_Val);
}

/**
  @brief Executes the "BindRequest" to create a binding between two endpoints.

  Parameter_List[0] is the device ID for the target device.  The address for
  this device be a network address.
  Parameter_List[1] is the device ID for the source device of the bind.  The
  address for this device be an extended address.
  Parameter_List[2] is the device ID for the destination device of the bind.
  The address for this device be a group or extended address.
  Parameter_List[3] is the cluster ID for the bind.

  @param Parameter_Count is number of elements in Parameter_List.
  @param Parameter_List is list of parsed arguments associate with this
  command.

  @return
  - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
  - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
  - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
  command.
 */
static QCLI_Command_Status_t cmd_ZB_BindRequest(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    QCLI_Command_Status_t   Ret_Val;
    qapi_ZB_ZDP_Bind_Req_t  Request;
    qapi_Status_t           Result;
    ZB_Device_ID_t         *TargetDevice;
    ZB_Device_ID_t         *SrcDevice;
    ZB_Device_ID_t         *DstDevice;

    if(ZigBee_Demo_Context.ZigBee_Handle != NULL)
    {
        /* Check the parameters. */
        if((Parameter_Count >= 4) &&
                (Verify_Integer_Parameter(&(Parameter_List[3]), 0x0000, 0xFFFF)))
        {
            /* Get the device information. */
            TargetDevice = GetDeviceListEntry(Parameter_List[0].Integer_Value);
            SrcDevice    = GetDeviceListEntry(Parameter_List[1].Integer_Value);
            DstDevice    = GetDeviceListEntry(Parameter_List[2].Integer_Value);

            if((TargetDevice != NULL) && (SrcDevice != NULL) && (DstDevice != NULL))
            {
                /* Set Request to a known state. */
                memset((void *)&Request, 0 , sizeof(qapi_ZB_ZDP_Bind_Req_t));

                /* Verify the address types for each specified device is valid. */
                if((TargetDevice->Type == QAPI_ZB_ADDRESS_MODE_SHORT_ADDRESS_E) &&
                        (SrcDevice->Type == QAPI_ZB_ADDRESS_MODE_EXTENDED_ADDRESS_E) &&
                        ((DstDevice->Type == QAPI_ZB_ADDRESS_MODE_GROUP_ADDRESS_E) || (DstDevice->Type == QAPI_ZB_ADDRESS_MODE_EXTENDED_ADDRESS_E)))
                {
                    /* Set up the Request. */
                    Request.DestNwkAddr          = TargetDevice->Address.ShortAddress;
                    Request.BindData.SrcAddress  = SrcDevice->Address.ExtendedAddress;
                    Request.BindData.SrcEndpoint = SrcDevice->Endpoint;
                    Request.BindData.ClusterId   = (uint16_t)(Parameter_List[3].Integer_Value);
                    Request.BindData.DstAddrMode = DstDevice->Type;
                    Request.BindData.DstAddress  = DstDevice->Address;
                    Request.BindData.DstEndpoint = DstDevice->Endpoint;

                    /* Issue the bind request. */
                    Result = qapi_ZB_ZDP_Bind_Req(ZigBee_Demo_Context.ZigBee_Handle, &Request);
                    if(Result == QAPI_OK)
                    {
                        Display_Function_Success(ZigBee_Demo_Context.QCLI_Handle, "qapi_ZB_ZDP_Bind_Req");
                        Ret_Val = QCLI_STATUS_SUCCESS_E;
                    }
                    else
                    {
                        Display_Function_Error(ZigBee_Demo_Context.QCLI_Handle, "qapi_ZB_ZDP_Bind_Req", Result);
                        Ret_Val = QCLI_STATUS_ERROR_E;
                    }
                }
                else
                {
                    QCLI_Printf(ZigBee_Demo_Context.QCLI_Handle, "Specified device is not a valid type.\n");
                    Ret_Val = QCLI_STATUS_USAGE_E;
                }
            }
            else
            {
                QCLI_Printf(ZigBee_Demo_Context.QCLI_Handle, "Device ID is not valid.\n");
                Ret_Val = QCLI_STATUS_USAGE_E;
            }
        }
        else
        {
            Ret_Val = QCLI_STATUS_USAGE_E;
        }
    }
    else
    {
        QCLI_Printf(ZigBee_Demo_Context.QCLI_Handle, "ZigBee stack is not initialized.\n");
        Ret_Val = QCLI_STATUS_ERROR_E;
    }

    return(Ret_Val);
}

/**
  @brief Executes the "EndBind" to create a binding between two endpoints.

  Parameter_List[0] is the device ID for the target device.  The address for
  this device be a network address.
  Parameter_List[1] is the device ID for the source device of the bind.  The
  address for this device be an extended address.
  Parameter_List[2] is the Cluster ID for the bind.
  Parameter_List[3] is a flag indicating if it is a server or client cluster..

  @param Parameter_Count is number of elements in Parameter_List.
  @param Parameter_List is list of parsed arguments associate with this
  command.

  @return
  - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
  - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
  - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
  command.
 */
static QCLI_Command_Status_t cmd_ZB_EndBind(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    QCLI_Command_Status_t              Ret_Val;
    qapi_ZB_ZDP_End_Device_Bind_Req_t  Request;
    qapi_Status_t                      Result;
    ZB_Device_ID_t                    *TargetDevice;
    ZB_Device_ID_t                    *SrcDevice;
    uint16_t                           ClusterID;
    qbool_t                            IsServer;

    if(ZigBee_Demo_Context.ZigBee_Handle != NULL)
    {
        /* Check the parameters. */
        if((Parameter_Count >= 4) &&
                (Verify_Integer_Parameter(&(Parameter_List[2]), 0x0000, 0xFFFF)) &&
                (Verify_Integer_Parameter(&(Parameter_List[3]), 0, 1)))
        {
            /* Get the device information. */
            TargetDevice = GetDeviceListEntry(Parameter_List[0].Integer_Value);
            SrcDevice    = GetDeviceListEntry(Parameter_List[1].Integer_Value);
            ClusterID    = (uint16_t)(Parameter_List[2].Integer_Value);
            IsServer     = (qbool_t)(Parameter_List[3].Integer_Value != 0);

            if((TargetDevice != NULL) && (SrcDevice != NULL))
            {
                /* Set Request to a known state. */
                memset((void *)&Request, 0 , sizeof(qapi_ZB_ZDP_End_Device_Bind_Req_t));

                /* Verify the address types for each specified device is valid. */
                if((TargetDevice->Type == QAPI_ZB_ADDRESS_MODE_SHORT_ADDRESS_E) &&
                        (SrcDevice->Type == QAPI_ZB_ADDRESS_MODE_EXTENDED_ADDRESS_E))
                {
                    /* Set up the Request. */
                    Request.BindingTarget  = TargetDevice->Address.ShortAddress;
                    Request.SrcIEEEAddress = SrcDevice->Address.ExtendedAddress;
                    Request.SrcEndpoint    = SrcDevice->Endpoint;
                    Request.ProfileID      = QAPI_ZB_CL_PROFILE_ID_HOME_AUTOMATION;

                    if(IsServer)
                    {
                        Request.NumInClusters  = 1;
                        Request.InClusterList  = &ClusterID;
                    }
                    else
                    {
                        Request.NumOutClusters = 1;
                        Request.OutClusterList = &ClusterID;
                    }

                    /* Issue the bind request. */
                    Result = qapi_ZB_ZDP_End_Device_Bind_Req(ZigBee_Demo_Context.ZigBee_Handle, &Request);
                    if(Result == QAPI_OK)
                    {
                        Display_Function_Success(ZigBee_Demo_Context.QCLI_Handle, "qapi_ZB_ZDP_End_Device_Bind_Req");
                        Ret_Val = QCLI_STATUS_SUCCESS_E;
                    }
                    else
                    {
                        Display_Function_Error(ZigBee_Demo_Context.QCLI_Handle, "qapi_ZB_ZDP_End_Device_Bind_Req", Result);
                        Ret_Val = QCLI_STATUS_ERROR_E;
                    }
                }
                else
                {
                    QCLI_Printf(ZigBee_Demo_Context.QCLI_Handle, "Specified device is not a valid type.\n");
                    Ret_Val = QCLI_STATUS_USAGE_E;
                }
            }
            else
            {
                QCLI_Printf(ZigBee_Demo_Context.QCLI_Handle, "Device ID is not valid.\n");
                Ret_Val = QCLI_STATUS_USAGE_E;
            }
        }
        else
        {
            Ret_Val = QCLI_STATUS_USAGE_E;
        }
    }
    else
    {
        QCLI_Printf(ZigBee_Demo_Context.QCLI_Handle, "ZigBee stack is not initialized.\n");
        Ret_Val = QCLI_STATUS_ERROR_E;
    }

    return(Ret_Val);
}

/**
  @brief Executes the "UnbindRequest" to unbind two endpoints.

  Parameter_List[0] is the device ID for the target device.  The address for
  this device be a network address.
  Parameter_List[1] is the device ID for the source device of the bind.  The
  address for this device be an extended address.
  Parameter_List[2] is the device ID for the destination device of the bind.
  The address for this device be a group or extended address.
  Parameter_List[3] is the cluster ID for the bind.

  @param Parameter_Count is number of elements in Parameter_List.
  @param Parameter_List is list of parsed arguments associate with this
  command.

  @return
  - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
  - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
  - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
  command.
 */
static QCLI_Command_Status_t cmd_ZB_UnbindRequest(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    QCLI_Command_Status_t     Ret_Val;
    qapi_ZB_ZDP_Unbind_Req_t  Request;
    qapi_Status_t             Result;
    ZB_Device_ID_t           *TargetDevice;
    ZB_Device_ID_t           *SrcDevice;
    ZB_Device_ID_t           *DstDevice;

    /* Check the parameters. */
    if(ZigBee_Demo_Context.ZigBee_Handle != NULL)
    {
        /* Check the parameters. */
        if((Parameter_Count >= 4) &&
                (Verify_Integer_Parameter(&(Parameter_List[3]), 0x0000, 0xFFFF)))
        {
            /* Get the device information. */
            TargetDevice = GetDeviceListEntry(Parameter_List[0].Integer_Value);
            SrcDevice    = GetDeviceListEntry(Parameter_List[1].Integer_Value);
            DstDevice    = GetDeviceListEntry(Parameter_List[2].Integer_Value);

            if((TargetDevice != NULL) && (SrcDevice != NULL) && (DstDevice != NULL))
            {
                /* Set Request to a known state. */
                memset((void *)&Request, 0 , sizeof(qapi_ZB_ZDP_Bind_Req_t));

                /* Verify the address types for each specified device is valid. */
                if((TargetDevice->Type == QAPI_ZB_ADDRESS_MODE_SHORT_ADDRESS_E) &&
                        (SrcDevice->Type == QAPI_ZB_ADDRESS_MODE_EXTENDED_ADDRESS_E) &&
                        ((DstDevice->Type == QAPI_ZB_ADDRESS_MODE_GROUP_ADDRESS_E) || (DstDevice->Type == QAPI_ZB_ADDRESS_MODE_EXTENDED_ADDRESS_E)))
                {
                    /* Set up the Request. */
                    Request.DestNwkAddr            = TargetDevice->Address.ShortAddress;
                    Request.UnbindData.SrcAddress  = SrcDevice->Address.ExtendedAddress;
                    Request.UnbindData.SrcEndpoint = SrcDevice->Endpoint;
                    Request.UnbindData.ClusterId   = (uint16_t)(Parameter_List[3].Integer_Value);
                    Request.UnbindData.DstAddrMode = DstDevice->Type;
                    Request.UnbindData.DstAddress  = DstDevice->Address;
                    Request.UnbindData.DstEndpoint = DstDevice->Endpoint;

                    /* Issue the unbind request. */
                    Result = qapi_ZB_ZDP_Unbind_Req(ZigBee_Demo_Context.ZigBee_Handle, &Request);
                    if(Result == QAPI_OK)
                    {
                        Display_Function_Success(ZigBee_Demo_Context.QCLI_Handle, "qapi_ZB_ZDP_Unbind_Req");
                        Ret_Val = QCLI_STATUS_SUCCESS_E;
                    }
                    else
                    {
                        Display_Function_Error(ZigBee_Demo_Context.QCLI_Handle, "qapi_ZB_ZDP_Unbind_Req", Result);
                        Ret_Val = QCLI_STATUS_ERROR_E;
                    }
                }
                else
                {
                    QCLI_Printf(ZigBee_Demo_Context.QCLI_Handle, "Specified device is not a valid type.\n");
                    Ret_Val = QCLI_STATUS_USAGE_E;
                }
            }
            else
            {
                QCLI_Printf(ZigBee_Demo_Context.QCLI_Handle, "Device ID is not valid.\n");
                Ret_Val = QCLI_STATUS_USAGE_E;
            }
        }
        else
        {
            Ret_Val = QCLI_STATUS_USAGE_E;
        }
    }
    else
    {
        QCLI_Printf(ZigBee_Demo_Context.QCLI_Handle, "ZigBee stack is not initialized.\n");
        Ret_Val = QCLI_STATUS_ERROR_E;
    }

    return(Ret_Val);
}

/**
  @brief Executes the "GetNIB" command to read a NWK attribute.

  Parameter_List[0] Attribute ID to be read.
  Parameter_List[1] Attribute Index to read (default = 0).
  Parameter_List[2] Maximum size of the attribute (default = 128).

  @param Parameter_Count is number of elements in Parameter_List.
  @param Parameter_List is list of parsed arguments associate with this
  command.

  @return
  - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
  - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
  - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
  command.
 */
static QCLI_Command_Status_t cmd_ZB_GetNIB(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    QCLI_Command_Status_t       Ret_Val;
    qapi_Status_t               Result;
    qapi_ZB_NIB_Attribute_ID_t  AttributeId;
    uint8_t                     AttributeIndex;
    uint16_t                    AttributeLength;
    void                       *AttributeValue;

    /* Check the parameters. */
    if(ZigBee_Demo_Context.ZigBee_Handle != NULL)
    {
        /* Check the parameters. */
        if((Parameter_Count >= 1) &&
                (Verify_Integer_Parameter(&(Parameter_List[0]), 0, 0xFFFF)))
        {
            AttributeId     = (qapi_ZB_NIB_Attribute_ID_t)(Parameter_List[0].Integer_Value);
            AttributeIndex  = 0;
            AttributeLength = 128;

            if((Parameter_Count >= 2) && (Verify_Integer_Parameter(&(Parameter_List[1]), 0, 0xFF)))
            {
                AttributeIndex = (uint8_t)(Parameter_List[1].Integer_Value);

                if((Parameter_Count >= 3) && (Verify_Integer_Parameter(&(Parameter_List[2]), 0, 0xFFFF)))
                {
                    AttributeLength = (uint16_t)(Parameter_List[2].Integer_Value);
                }
            }
         }

            /* Allocate a buffer for the attribute data. */
            AttributeValue = malloc(AttributeLength);
            if(AttributeValue != NULL)
            {
                Result = qapi_ZB_NLME_Get_Request(ZigBee_Demo_Context.ZigBee_Handle, AttributeId, AttributeIndex, &AttributeLength, (uint8_t *)AttributeValue);

                if(Result == QAPI_OK)
                {
                    QCLI_Printf(ZigBee_Demo_Context.QCLI_Handle, "Attribute 0x%04X (%d bytes): ", AttributeId, AttributeLength);
                    switch(AttributeLength)
                    {
                        case sizeof(uint8_t):
                            QCLI_Printf(ZigBee_Demo_Context.QCLI_Handle, "0x%02X", *(uint8_t *)AttributeValue);
                            break;

                        case sizeof(uint16_t):
                            QCLI_Printf(ZigBee_Demo_Context.QCLI_Handle, "0x%04X", *(uint16_t *)AttributeValue);
                            break;

                        case sizeof(uint32_t):
                            QCLI_Printf(ZigBee_Demo_Context.QCLI_Handle, "0x%08X", *(uint32_t *)AttributeValue);
                            break;

                        default:
                            QCLI_Printf(ZigBee_Demo_Context.QCLI_Handle, "\n");
                            Dump_Data(ZigBee_Demo_Context.QCLI_Handle, "  ", AttributeLength, (uint8_t *)AttributeValue);
                            break;
                    }
                }
                else
                {
                    Display_Function_Error(ZigBee_Demo_Context.QCLI_Handle, "qapi_ZB_NLME_Get_Request", Result);
                    Ret_Val = QCLI_STATUS_ERROR_E;
                }

                free(AttributeValue);
            }
            else
            {
                QCLI_Printf(ZigBee_Demo_Context.QCLI_Handle, "Failed to allocate a buffer for the attribute.\n");
                Ret_Val = QCLI_STATUS_ERROR_E;
            }
        }
        else
        {
            Ret_Val = QCLI_STATUS_USAGE_E;
        }
    }
    else
    {
        QCLI_Printf(ZigBee_Demo_Context.QCLI_Handle, "ZigBee stack is not initialized.\n");
        Ret_Val = QCLI_STATUS_ERROR_E;
    }

    return(Ret_Val);
}

/**
  @brief Executes the "GetNIB" command to write a NWK attribute.

  Parameter_List[0] Attribute ID to be write.
  Parameter_List[1] Attribute Index to write.
  Parameter_List[2] Size of the attribute being written in bytes.
  Parameter_List[3] Value for the attribute.

  @param Parameter_Count is number of elements in Parameter_List.
  @param Parameter_List is list of parsed arguments associate with this
  command.

  @return
  - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
  - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
  - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
  command.
 */
static QCLI_Command_Status_t cmd_ZB_SetNIB(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    QCLI_Command_Status_t       Ret_Val;
    qapi_Status_t               Result;
    qapi_ZB_NIB_Attribute_ID_t  AttributeId;
    uint8_t                     AttributeIndex;
    uint32_t                    AttributeLength;
    uint32_t                    TempLength;
    uint8_t                    *AttributeValue;

    /* Check the parameters. */
    if(ZigBee_Demo_Context.ZigBee_Handle != NULL)
    {
        /* Check the parameters. */
        if((Parameter_Count >= 4) &&
                (Verify_Integer_Parameter(&(Parameter_List[0]), 0, 0xFFFF)) &&
                (Verify_Integer_Parameter(&(Parameter_List[1]), 0, 0xFF)) &&
                (Verify_Integer_Parameter(&(Parameter_List[2]), 0, 0xFFFF)))
        {
            AttributeId     = (qapi_ZB_NIB_Attribute_ID_t)(Parameter_List[0].Integer_Value);
            AttributeIndex  = (uint8_t)(Parameter_List[1].Integer_Value);
            AttributeLength = (uint32_t)(Parameter_List[2].Integer_Value);

            /* Allocate a buffer for the attribute data. */
            AttributeValue = malloc(AttributeLength);
            if(AttributeValue != NULL)
            {
                TempLength = AttributeLength;
                if(Hex_String_To_Array(Parameter_List[3].String_Value, &TempLength, AttributeValue))
                {
                    Result = qapi_ZB_NLME_Set_Request(ZigBee_Demo_Context.ZigBee_Handle, AttributeId, AttributeIndex, AttributeLength, AttributeValue);

                    if(Result == QAPI_OK)
                    {
                        Display_Function_Success(ZigBee_Demo_Context.QCLI_Handle, "qapi_ZB_NLME_Set_Request");
                        Ret_Val = QCLI_STATUS_SUCCESS_E;
                    }
                    else
                    {
                        Display_Function_Error(ZigBee_Demo_Context.QCLI_Handle, "qapi_ZB_NLME_Set_Request", Result);
                        Ret_Val = QCLI_STATUS_ERROR_E;
                    }
                }
                else
                {
                    QCLI_Printf(ZigBee_Demo_Context.QCLI_Handle, "Failed to parse the attribute value.\n");
                    Ret_Val = QCLI_STATUS_ERROR_E;
                }

                free(AttributeValue);
            }
            else
            {
                QCLI_Printf(ZigBee_Demo_Context.QCLI_Handle, "Failed to allocate a buffer for the attribute.\n");
                Ret_Val = QCLI_STATUS_ERROR_E;
            }
        }
        else
        {
            Ret_Val = QCLI_STATUS_USAGE_E;
        }
    }
    else
    {
        QCLI_Printf(ZigBee_Demo_Context.QCLI_Handle, "ZigBee stack is not initialized.\n");
        Ret_Val = QCLI_STATUS_ERROR_E;
    }

    return(Ret_Val);
}

/**
  @brief Executes the "GetAIB" command to read a APS attribute.

  Parameter_List[0] Attribute ID to be read.
  Parameter_List[1] Attribute Index to read.
  Parameter_List[2] Maximum size of the attribute.  Defaults to 128 if not
  specified.

  @param Parameter_Count is number of elements in Parameter_List.
  @param Parameter_List is list of parsed arguments associate with this
  command.

  @return
  - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
  - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
  - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
  command.
 */
static QCLI_Command_Status_t cmd_ZB_GetAIB(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    QCLI_Command_Status_t       Ret_Val;
    qapi_Status_t               Result;
    qapi_ZB_AIB_Attribute_ID_t  AttributeId;
    uint8_t                     AttributeIndex;
    uint16_t                    AttributeLength;
    void                       *AttributeValue;

    /* Check the parameters. */
    if(ZigBee_Demo_Context.ZigBee_Handle != NULL)
    {
        /* Check the parameters. */
        if((Parameter_Count >= 1) &&
                (Verify_Integer_Parameter(&(Parameter_List[0]), 0, 0xFFFF)))
        {
            AttributeId     = (qapi_ZB_AIB_Attribute_ID_t)(Parameter_List[0].Integer_Value);
            AttributeIndex  = 0;
            AttributeLength = 128;

            if((Parameter_Count >= 2) && (Verify_Integer_Parameter(&(Parameter_List[1]), 0, 0xFF)))
            {
                AttributeIndex = (uint8_t)(Parameter_List[1].Integer_Value);

                if((Parameter_Count >= 3) && (Verify_Integer_Parameter(&(Parameter_List[2]), 0, 0xFFFF)))
                {
                    AttributeLength = (uint16_t)(Parameter_List[2].Integer_Value);
                }
            }
         }

            /* Allocate a buffer for the attribute data. */
            AttributeValue = malloc(AttributeLength);
            if(AttributeValue != NULL)
            {
                Result = qapi_ZB_APSME_Get_Request(ZigBee_Demo_Context.ZigBee_Handle, AttributeId, AttributeIndex, &AttributeLength, (uint8_t *)AttributeValue);

                if(Result == QAPI_OK)
                {
                    QCLI_Printf(ZigBee_Demo_Context.QCLI_Handle, "Attribute %d (%d bytes): ", AttributeId, AttributeLength);
                    switch(AttributeLength)
                    {
                        case sizeof(uint8_t):
                            QCLI_Printf(ZigBee_Demo_Context.QCLI_Handle, "0x%02X", *(uint8_t *)AttributeValue);
                            break;

                        case sizeof(uint16_t):
                            QCLI_Printf(ZigBee_Demo_Context.QCLI_Handle, "0x%04X", *(uint16_t *)AttributeValue);
                            break;

                        case sizeof(uint32_t):
                            QCLI_Printf(ZigBee_Demo_Context.QCLI_Handle, "0x%08X", *(uint32_t *)AttributeValue);
                            break;

                        default:
                            QCLI_Printf(ZigBee_Demo_Context.QCLI_Handle, "\n");
                            Dump_Data(ZigBee_Demo_Context.QCLI_Handle, "  ", AttributeLength, (uint8_t *)AttributeValue);
                            break;
                    }
                }
                else
                {
                    Display_Function_Error(ZigBee_Demo_Context.QCLI_Handle, "qapi_ZB_APSME_Get_Request", Result);
                    Ret_Val = QCLI_STATUS_ERROR_E;
                }

                free(AttributeValue);
            }
            else
            {
                QCLI_Printf(ZigBee_Demo_Context.QCLI_Handle, "Failed to allocate a buffer for the attribute.\n");
                Ret_Val = QCLI_STATUS_ERROR_E;
            }
        }
        else
        {
            Ret_Val = QCLI_STATUS_USAGE_E;
        }
    }
    else
    {
        QCLI_Printf(ZigBee_Demo_Context.QCLI_Handle, "ZigBee stack is not initialized.\n");
        Ret_Val = QCLI_STATUS_ERROR_E;
    }

    return(Ret_Val);
}

/**
  @brief Executes the "GetAIB" command to write a APS attribute.

  Parameter_List[0] Attribute ID to be write.
  Parameter_List[1] Attribute Index to write.
  Parameter_List[2] Size of the attribute being written in bytes.
  Parameter_List[3] Value for the attribute.

  @param Parameter_Count is number of elements in Parameter_List.
  @param Parameter_List is list of parsed arguments associate with this
  command.

  @return
  - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
  - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
  - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
  command.
 */
static QCLI_Command_Status_t cmd_ZB_SetAIB(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    QCLI_Command_Status_t       Ret_Val;
    qapi_Status_t               Result;
    qapi_ZB_AIB_Attribute_ID_t  AttributeId;
    uint8_t                     AttributeIndex;
    uint32_t                    AttributeLength;
    uint32_t                    TempLength;
    uint8_t                    *AttributeValue;

    /* Check the parameters. */
    if(ZigBee_Demo_Context.ZigBee_Handle != NULL)
    {
        /* Check the parameters. */
        if((Parameter_Count >= 4) &&
                (Verify_Integer_Parameter(&(Parameter_List[0]), 0, 0xFFFF)) &&
                (Verify_Integer_Parameter(&(Parameter_List[1]), 0, 0xFF)) &&
                (Verify_Integer_Parameter(&(Parameter_List[2]), 0, 0xFFFF)))
        {
            AttributeId     = (qapi_ZB_AIB_Attribute_ID_t)(Parameter_List[0].Integer_Value);
            AttributeIndex  = (uint8_t)(Parameter_List[1].Integer_Value);
            AttributeLength = (uint32_t)(Parameter_List[2].Integer_Value);

            /* Allocate a buffer for the attribute data. */
            AttributeValue = malloc(AttributeLength);
            if(AttributeValue != NULL)
            {
                TempLength = AttributeLength;
                if(Hex_String_To_Array(Parameter_List[3].String_Value, &TempLength, AttributeValue))
                {
                    Result = qapi_ZB_APSME_Set_Request(ZigBee_Demo_Context.ZigBee_Handle, AttributeId, AttributeIndex, AttributeLength, AttributeValue);

                    if(Result == QAPI_OK)
                    {
                        Display_Function_Success(ZigBee_Demo_Context.QCLI_Handle, "qapi_ZB_APSME_Set_Request");
                        Ret_Val = QCLI_STATUS_SUCCESS_E;
                    }
                    else
                    {
                        Display_Function_Error(ZigBee_Demo_Context.QCLI_Handle, "qapi_ZB_APSME_Set_Request", Result);
                        Ret_Val = QCLI_STATUS_ERROR_E;
                    }
                }
                else
                {
                    QCLI_Printf(ZigBee_Demo_Context.QCLI_Handle, "Failed to parse the attribute value.\n");
                    Ret_Val = QCLI_STATUS_ERROR_E;
                }

                free(AttributeValue);
            }
            else
            {
                QCLI_Printf(ZigBee_Demo_Context.QCLI_Handle, "Failed to allocate a buffer for the attribute.\n");
                Ret_Val = QCLI_STATUS_ERROR_E;
            }
        }
        else
        {
            Ret_Val = QCLI_STATUS_USAGE_E;
        }
    }
    else
    {
        QCLI_Printf(ZigBee_Demo_Context.QCLI_Handle, "ZigBee stack is not initialized.\n");
        Ret_Val = QCLI_STATUS_ERROR_E;
    }

    return(Ret_Val);
}

/**
  @brief Executes the "SetBIB" command.

  Parameter_List[0] is the ID of BDB attributes. (AttrId)
  Parameter_List[1] is the index within an attribute identified by the AttrId.
  (AttrIndex)
  Parameter_List[2] is the length of the attribute. (AttrLength)
  Parameter_List[3] is the value of the attribute. (AttrValue)

  @param Parameter_Count is number of elements in Parameter_List.
  @param Parameter_List is list of parsed arguments associate with this
  command.

  @return
  - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
  - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
  - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
  command.
 */
static QCLI_Command_Status_t cmd_ZB_SetBIB(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    QCLI_Command_Status_t  Ret_Val;
    qapi_Status_t          Result;
    uint32_t               MaxValue;
    uint64_t               AttrValueULL;
    uint16_t               AttrId;
    uint8_t                AttrIndex;
    uint16_t               AttrLength;
    uint8_t               *AttrValue;

    if(ZigBee_Demo_Context.ZigBee_Handle != NULL)
    {
        if((Parameter_Count >= 4) &&
                (Verify_Integer_Parameter(&Parameter_List[0], 0x0000, 0xFFFF)) &&
                (Verify_Integer_Parameter(&Parameter_List[1], 0x00, 0xFF)) &&
                (Verify_Integer_Parameter(&Parameter_List[2], 1, 0xFF))
          )
        {
            AttrId     = (uint16_t)(Parameter_List[0].Integer_Value);
            AttrIndex  = (uint8_t)(Parameter_List[1].Integer_Value);
            AttrLength = (uint8_t)(Parameter_List[2].Integer_Value);
            switch(AttrLength)
            {
                case sizeof(uint8_t):
                case sizeof(uint16_t):
                case sizeof(uint32_t):
                    /* Handle the basic integer types.                       */
                    MaxValue = 0xFFFFFFFF >> ((sizeof(uint32_t) - AttrLength) * 8);

                    if(Verify_Unsigned_Integer_Parameter(&(Parameter_List[3]), 0, MaxValue))
                    {
                        AttrValue = (uint8_t *)&(Parameter_List[3].Integer_Value);

                        Ret_Val = QCLI_STATUS_SUCCESS_E;
                    }
                    else
                    {
                        Ret_Val = QCLI_STATUS_ERROR_E;
                    }
                    break;

                case sizeof(uint64_t):
                    /* Attempt to convert the string to a 64-bit integer. */
                    if(Hex_String_To_ULL(Parameter_List[3].String_Value, &AttrValueULL))
                {
                    AttrValue = (uint8_t *)&AttrValueULL;

                    Ret_Val = QCLI_STATUS_SUCCESS_E;
                }
                    else
                    {
                        Ret_Val = QCLI_STATUS_ERROR_E;
                    }
                    break;

                default:
                    Ret_Val = QCLI_STATUS_ERROR_E;
                    break;
            }
               else
               {
                  Ret_Val = QCLI_STATUS_ERROR_E;
               }
               break;

            if(Ret_Val == QCLI_STATUS_SUCCESS_E)
            {
                Result = qapi_ZB_BDB_Set_Request(ZigBee_Demo_Context.ZigBee_Handle , AttrId, AttrIndex, AttrLength, AttrValue);
                if(Result == QAPI_OK)
                {
                    Display_Function_Success(ZigBee_Demo_Context.QCLI_Handle, "qapi_ZB_BDB_Set_Request");
                }
                else
                {
                    Ret_Val = QCLI_STATUS_ERROR_E;
                    Display_Function_Error(ZigBee_Demo_Context.QCLI_Handle, "qapi_ZB_BDB_Set_Request", Result);
                }

         if(Ret_Val == QCLI_STATUS_SUCCESS_E)
         {
            Result = qapi_ZB_BDB_Set_Request(ZigBee_Demo_Context.ZigBee_Handle , AttrId, AttrIndex, AttrLength, AttrValue);
            if(Result == QAPI_OK)
            {
               Display_Function_Success(ZigBee_Demo_Context.QCLI_Handle, "qapi_ZB_BDB_Set_Request");
            }
        }
        else
        {
            Ret_Val = QCLI_STATUS_USAGE_E;
        }
    }
    else
    {
        QCLI_Printf(ZigBee_Demo_Context.QCLI_Handle, "Zigbee stack is not initialized.\n");
        Ret_Val = QCLI_STATUS_ERROR_E;
    }

    return(Ret_Val);
}

/**
  @brief Executes the "GetBIB" command.

  Parameter_List[0] is the ID of BDB attributes. (AttrId)
  Parameter_List[1] is the index within an attribute identified by the AttrId.
  (AttrIndex)
  Parameter_List[2] is the length of the attribute. (AttrLength)

  @param Parameter_Count is number of elements in Parameter_List.
  @param Parameter_List is list of parsed arguments associate with this
  command.

  @return
  - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
  - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
  - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
  command.
 */
static QCLI_Command_Status_t cmd_ZB_GetBIB(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    QCLI_Command_Status_t  Ret_Val;
    qapi_Status_t          Result;
    uint16_t               AttrId;
    uint8_t                AttrIndex;
    uint16_t               AttrLength;
    uint8_t                AttrValue[MAXIMUM_ATTRIUBTE_LENGTH];

    if(ZigBee_Demo_Context.ZigBee_Handle != NULL)
    {
        /* Check the parameters.                                          */
        if((Parameter_Count >= 3) &&
                (Verify_Integer_Parameter(&(Parameter_List[0]), 0x0000, 0xFFFF)) &&
                (Verify_Integer_Parameter(&(Parameter_List[1]), 0x00, 0xFF)) &&
                (Verify_Integer_Parameter(&(Parameter_List[2]), 0, sizeof(AttrValue)))
          )
        {
            /* Get the device information.                                 */
            AttrId     = (uint16_t)(Parameter_List[0].Integer_Value);
            AttrIndex  = (uint8_t)(Parameter_List[1].Integer_Value);
            AttrLength = (uint16_t)(Parameter_List[2].Integer_Value);

            Result = qapi_ZB_BDB_Get_Request(ZigBee_Demo_Context.ZigBee_Handle, (qapi_ZB_BDB_Attribute_ID_t)AttrId, AttrIndex, &AttrLength, AttrValue);
            switch(Result)
            {
                case QAPI_ERR_BOUNDS:
                    Display_Function_Error(ZigBee_Demo_Context.QCLI_Handle, "qapi_ZB_BDB_Get_Request", Result);
                    QCLI_Printf(ZigBee_Demo_Context.QCLI_Handle, "AttrLength: %d\n", AttrLength);
                    Ret_Val = QCLI_STATUS_USAGE_E;
                    break;

                case QAPI_OK:
                    Display_Function_Success(ZigBee_Demo_Context.QCLI_Handle, "qapi_ZB_BDB_Get_Request");
                    QCLI_Printf(ZigBee_Demo_Context.QCLI_Handle, "   AttrId:     0x%04X\n", AttrId);
                    QCLI_Printf(ZigBee_Demo_Context.QCLI_Handle, "   AttrLength: %d\n", AttrLength);
                    QCLI_Printf(ZigBee_Demo_Context.QCLI_Handle, "   AttrValue:  ");
                    DisplayVariableLengthValue(ZigBee_Demo_Context.QCLI_Handle, AttrLength, AttrValue);
                    Ret_Val = QCLI_STATUS_SUCCESS_E;
                    break;

                default:
                    Display_Function_Error(ZigBee_Demo_Context.QCLI_Handle, "qapi_ZB_BDB_Get_Request", Result);
                    Ret_Val = QCLI_STATUS_ERROR_E;
                    break;
            }
        }
        else
        {
            Ret_Val = QCLI_STATUS_USAGE_E;
        }
    }
    else
    {
        QCLI_Printf(ZigBee_Demo_Context.QCLI_Handle, "ZigBee stack is not initialized.\n");
        Ret_Val = QCLI_STATUS_ERROR_E;
    }

    return(Ret_Val);
}

/**
  @brief Executes the "ClearPersist" command to clear the perisistent ZigBee
  data from flash.

  @param Parameter_Count is number of elements in Parameter_List.
  @param Parameter_List is list of parsed arguments associate with this
  command.

  @return
  - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
  - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
  - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
  command.
 */
static QCLI_Command_Status_t cmd_ZB_ClearPersist(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    QCLI_Command_Status_t Ret_Val;
    qapi_Status_t         Result;
    qapi_Persist_Handle_t PersistHandle;

    /* Get the handle for the persistent data. */
    if(ZigBee_Demo_Context.PersistHandle != NULL)
    {
        /* Use the persist handle we already have. */
        PersistHandle = ZigBee_Demo_Context.PersistHandle;
    }
    else
    {
        /* Initialize a temporary persist instance. */
        Result = qapi_Persist_Initialize(&PersistHandle, ZIGBEE_PERSIST_DIRECTORY, ZIGBEE_PERSIST_PREFIX, ZIGBEE_PERSIST_SUFFIX, NULL, 0);
        if(Result != QAPI_OK)
        {
            PersistHandle = NULL;
            Display_Function_Error(ZigBee_Demo_Context.QCLI_Handle, "qapi_Persist_Initialize", Result);
        }
    }

    if(PersistHandle != NULL)
    {
        qapi_Persist_Delete(PersistHandle);

        QCLI_Printf(ZigBee_Demo_Context.QCLI_Handle, "Persistent data cleared.\n");

        /* Cleanup the persist instance if it was temporary. */
        if(ZigBee_Demo_Context.PersistHandle == NULL)
        {
            qapi_Persist_Cleanup(PersistHandle);
        }

        Ret_Val = QCLI_STATUS_SUCCESS_E;
    }
    else
    {
        Ret_Val = QCLI_STATUS_ERROR_E;
    }

    return(Ret_Val);
}

/**
  @brief Callback handler for persist notify indications.

  @param[in] ZB_Handle Handle of the ZigBee instance that was returned by a
  successful call to qapi_ZB_Initialize().
  @param[in] CB_Param  User-specified parameter for the callback function.
 */
static void ZB_Persist_Notify_CB(qapi_ZB_Handle_t ZB_Handle, uint32_t CB_Param)
{
    qapi_Status_t          Result;
    qapi_Persist_Handle_t  PersistHandle;
    uint32_t               PersistLength;
    uint8_t               *PersistData;

    if((ZB_Handle == ZigBee_Demo_Context.ZigBee_Handle) && (CB_Param != 0))
    {
        PersistHandle = (qapi_Persist_Handle_t)CB_Param;

        PersistLength = 0;
        Result = qapi_ZB_Get_Persistent_Data(ZB_Handle, NULL, &PersistLength);

        if((Result == QAPI_ERR_BOUNDS) && (PersistLength > 0))
        {
            PersistData = (uint8_t *)malloc(PersistLength);
            if(PersistData != NULL)
            {
                Result = qapi_ZB_Get_Persistent_Data(ZB_Handle, PersistData, &PersistLength);
                if(Result == QAPI_OK)
                {
                    Result = qapi_Persist_Put(PersistHandle, PersistLength, PersistData);
                    if(Result != QAPI_OK)
                    {
                        Display_Function_Error(ZigBee_Demo_Context.QCLI_Handle, "qapi_Persist_Put", Result);
                    }
                }
                else
                {
                    Display_Function_Error(ZigBee_Demo_Context.QCLI_Handle, "qapi_ZB_Get_Persistent_Data", Result);
                }

                free(PersistData);
            }
            else
            {
                QCLI_Printf(ZigBee_Demo_Context.QCLI_Handle, "Failed to allocate persist data buffer.", Result);
            }
        }
        else
        {
            Display_Function_Error(ZigBee_Demo_Context.QCLI_Handle, "qapi_ZB_Get_Persistent_Data", Result);
        }
    }
}


#endif
/*
   void get_last_three_bytes(uint8_t *DevMacAddr, uint64_t DevAddr)
   {
   char temp[17] = {0};

//    memcpy(temp, (char *)&DevAddr, 3);
//    snprintf((char *)DevMacAddr, 7, "%x%x%x",*(temp),*(temp+1),*(temp+2));
//LOG_INFO("\n\n last 3 BytesDevMacAddr:%s\n ",DevMacAddr);

int i;
int j;

snprintf (temp, 17,"%lld", DevAddr) ;
LOG_INFO("\n\n******* Temp::%s   ***\n\n ", temp);
//intf ("%d: %s\n",strlen(temp),temp);

i = strlen((const char *)temp)-1;

for (j = 5; j>=0; j--, i--)
DevMacAddr[j] = temp[i];
DevMacAddr[6] = '\0';

LOG_INFO("\n\n get_last_three_bytes:%s\n\n ", DevMacAddr);

//printf ("data: %s\n",data);

}
 */
/**
  @brief Adds an address to the demo's device list.

  The addresses added with this command can be either a group address, a
  network address + endpoint or a extended address + endpoint.  Once
  registered, these addresses can be used to send commands to a remote device.

  @return
  - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
  - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
 */
int32_t Zigbee_AddDevice(uint8_t AddrMode, uint64_t DevAddr, uint8_t EndPointNum)
{
    uint32_t              Index;
    int32_t               Ret_Val;
    qapi_ZB_Addr_Mode_t   DevType;
    uint8_t               DevEndpoint;

    Ret_Val = SUCCESS;

    LOG_INFO(" In Zigbee_AddDevice() ");

    if(ZigBee_Demo_Context.ZigBee_Handle != 0)
    {
        LOG_INFO(" In Zigbeei_Handle is TRUE ");

        /** Assigning Address Mode */
        DevType = AddrMode;
        /* Record the endpoint parameter. */
        DevEndpoint = EndPointNum;
        /* Confirm the device address is valid for the type. */
        if((DevType == QAPI_ZB_ADDRESS_MODE_EXTENDED_ADDRESS_E) || (DevAddr <= 0xFFFF))
        {

            /* deleting previous entry if any for same Extendded Address */
            for(Index = 1; Index <= DEV_ID_LIST_SIZE; Index++)
            {
                LOG_INFO("Ex:%x, Dx:%x\n", ZigBee_Demo_Context.DevIDList[Index].Address.ExtendedAddress, DevAddr);
                if(((ZigBee_Demo_Context.DevIDList[Index].Address.ExtendedAddress) == DevAddr) && (DevEndpoint == ZigBee_Demo_Context.DevIDList[Index].Endpoint))
                {
                    LOG_INFO("Clearing the entry");
                    ZigBee_Demo_Context.DevIDList[Index].InUse = FALSE;
                    ZigBee_Demo_Context.DevIDList[Index].Address.ExtendedAddress = 0;
                    if (ZigBee_Demo_Context.DevIDList[Index].Endpoint == CUSTOM_CLUSTER_ENDPOINT)
                    {
                        if (ZigBee_Demo_Context.DevIDList[Index].EndDevBuf)
                        {
                              free(ZigBee_Demo_Context.DevIDList[Index].EndDevBuf);
                              ZigBee_Demo_Context.DevIDList[Index].EndDevBuf = NULL;
                        }
                        if (ZigBee_Demo_Context.DevIDList[Index].BreachBuf)
                        {
                            free(ZigBee_Demo_Context.DevIDList[Index].BreachBuf);
                            ZigBee_Demo_Context.DevIDList[Index].BreachBuf = NULL;
                        }
                    }
                }
            }
         
            /* Find an unused spot in the array. */
            for(Index = 1; Index <= DEV_ID_LIST_SIZE; Index++)
            {
                if(!(ZigBee_Demo_Context.DevIDList[Index].InUse))
                {
                    /* Found a spare slot. */
                    ZigBee_Demo_Context.DevIDList[Index].Type                    = (qapi_ZB_Addr_Mode_t) DevType;
                    ZigBee_Demo_Context.DevIDList[Index].Address.ExtendedAddress = DevAddr;
                    ZigBee_Demo_Context.DevIDList[Index].Endpoint = DevEndpoint;
                    if (ZigBee_Demo_Context.DevIDList[Index].Endpoint == CUSTOM_CLUSTER_ENDPOINT)
                    {
                        ZigBee_Demo_Context.DevIDList[Index].EndDevBuf = malloc(CUSTOM_DATA_SIZE);
                        ZigBee_Demo_Context.DevIDList[Index].BreachBuf = malloc(CUSTOM_DATA_SIZE);
                        LOG_INFO("\n Enddev buf : Index : %d \t address : %p\n", Index, ZigBee_Demo_Context.DevIDList[Index].EndDevBuf);
                        LOG_INFO("Memset befor\n");
                        memset(ZigBee_Demo_Context.DevIDList[Index].EndDevBuf, 0, CUSTOM_DATA_SIZE);
                        memset(ZigBee_Demo_Context.DevIDList[Index].BreachBuf, 0, CUSTOM_DATA_SIZE);
                        LOG_INFO("Memset after\n");
                    }
                    else
                    {
                        ZigBee_Demo_Context.DevIDList[Index].EndDevBuf = NULL;
                        ZigBee_Demo_Context.DevIDList[Index].BreachBuf = NULL;
                    }
                    //   ZigBee_Demo_Context.DevIDList[Index].EndDevBuf = NULL;
                    //   ZigBee_Demo_Context.DevIDList[Index].EndDevBuf = (uint8_t *) malloc(CUSTOM_DATA_SIZE);

                    ZigBee_Demo_Context.DevIDList[Index].InUse = true;
                    ZigBee_Demo_Context.DevIDList[Index].data_refresh_count = 1;

                    LOG_INFO("Registered device ID: %d\n", Index);
                    break;
                }
            }
         
            if(Index > DEV_ID_LIST_SIZE)
            {
                LOG_ERROR("Could not find a spare device entry.\n");
                Ret_Val = FAILURE;
            }
        }
        else
        {
            LOG_ERROR("Address is too large for type specified.\n");
            Ret_Val = FAILURE;
        }
    }
    else
    {
        LOG_ERROR("ZigBee stack is not initialized.\n");
        Ret_Val = FAILURE;
    }

    return(Ret_Val);
}

/**
  @brief Displays the list of registered devices in the demo's device list.

  @return
  - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
  - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
 */
int32_t Zigbee_ShowDeviceList()
{
    int32_t                Ret_Val;
    uint32_t               Index;
    uint8_t                AddressString[17];
    const char            *TypeString;

    if(ZigBee_Demo_Context.ZigBee_Handle != 0)
    {
        LOG_INFO("Device List:\n");
        LOG_INFO(" ID | Type | Address          | Endpoint\n");
        LOG_INFO("----+------+------------------+----------\n");

        for(Index = 0; Index <= DEV_ID_LIST_SIZE; Index++)
        {
            if(ZigBee_Demo_Context.DevIDList[Index].InUse)
            {
                switch(ZigBee_Demo_Context.DevIDList[Index].Type)
                {
                    case QAPI_ZB_ADDRESS_MODE_GROUP_ADDRESS_E:
                        TypeString = "Grp ";
                        snprintf((char *)(AddressString), sizeof(AddressString), "%04X", ZigBee_Demo_Context.DevIDList[Index].Address.ShortAddress);
                        break;

                    case QAPI_ZB_ADDRESS_MODE_SHORT_ADDRESS_E:
                        TypeString = "Nwk ";
                        snprintf((char *)(AddressString), sizeof(AddressString), "%04X", ZigBee_Demo_Context.DevIDList[Index].Address.ShortAddress);
                        break;

                    case QAPI_ZB_ADDRESS_MODE_EXTENDED_ADDRESS_E:
                        TypeString = "Ext ";
                        snprintf((char *)(AddressString), sizeof(AddressString), "%08X%08X", (unsigned int)(ZigBee_Demo_Context.DevIDList[Index].Address.ExtendedAddress >> 32), (unsigned int)(ZigBee_Demo_Context.DevIDList[Index].Address.ExtendedAddress));
                        break;


                    case QAPI_ZB_ADDRESS_MODE_NONE_E:
                    default:
                        TypeString = "None";
                        AddressString[0] = '\0';
                        break;
                }

                LOG_INFO(" %2d | %4s | %16s | %d\n", Index, TypeString, AddressString, ZigBee_Demo_Context.DevIDList[Index].Endpoint);
            }
        }

        Ret_Val = SUCCESS;
    }
    else
    {
        LOG_ERROR("ZigBee stack is not initialized.\n");
        Ret_Val = FAILURE;
    }

    return(Ret_Val);
}

/**
  @brief Starts a ZigBee network.

  @param : Security    - Enable/disable security
  @param : Distributed - Distribution mode or Centralized default co-ordinator is Centralized(value set to zero)
  @param : Mask        - Channel Selection, we can give desired channel for advertising
  @param : Master_key  - Master_Key is a security key to be verified at the time of joining
  @return
  - SUCCESS indicates the command is executed successfully.
  - FAILURE indicates the command is failed to execute.
 */
int32_t Zigbee_Form(uint32_t Security, uint32_t Distributed, uint32_t Mask, char *Master_key)
{
    int32_t                 Ret_Val;
    qapi_Status_t           Result;
    qapi_ZB_NetworkConfig_t NetworkConfig;
    uint32_t                ChannelMask;

    /* Verify the ZigBee layer had been initialized. */
    if (ZigBee_Demo_Context.ZigBee_Handle != NULL)
    {
        /* Assigning the parameters. */
        if ( (Mask != 0) &&  (Mask >= 11  && Mask <= 26) )
            ChannelMask = 1 << Mask;
        else
            ChannelMask = FORM_CHANNEL_MASK;

        memset(&NetworkConfig, 0, sizeof(qapi_ZB_NetworkConfig_t));

        NetworkConfig.ExtendedPanId = 0ULL;
        NetworkConfig.StackProfile  = QAPI_ZB_STACK_PROFILE_PRO_E;
        NetworkConfig.Page          = 0;
        NetworkConfig.ChannelMask   = ChannelMask;
        NetworkConfig.ScanAttempts  = SCAN_ATTEMPT;
        NetworkConfig.Capability    = COORDINATOR_CAPABILITIES;

        memscpy(&(NetworkConfig.Security), sizeof(qapi_ZB_Security_t), &Default_ZB_Secuity, sizeof(qapi_ZB_Security_t));

        /* Overwrite the default security level and trust center address. */
        NetworkConfig.Security.Security_Level       = Security ? QAPI_ZB_SECURITY_LEVEL_ENC_MIC32_E : QAPI_ZB_SECURITY_LEVEL_NONE_E;
        NetworkConfig.Security.Trust_Center_Address = Distributed ? QAPI_ZB_INVALID_EXTENDED_ADDRESS : 0;

        memcpy(NetworkConfig.Security.Preconfigured_Link_Key, Master_key, 16);

        Result = qapi_ZB_Form(ZigBee_Demo_Context.ZigBee_Handle, &NetworkConfig);

        if (Result == SUCCESS)
        {
            Display_Function_Success(ZigBee_Demo_Context.QCLI_Handle, "qapi_ZB_Form");
            Ret_Val = SUCCESS;
        }
        else
        {
            Display_Function_Error(ZigBee_Demo_Context.QCLI_Handle, "qapi_ZB_Form", Result);
            Ret_Val = FAILURE;
        }
    }
    else
    {
        LOG_ERROR("ZigBee stack is not initialized.\n");
        Ret_Val = FAILURE;
    }

    return (Ret_Val);
}

/**
  @brief Executes the "Join" command to join a ZigBee network.

  @param  Coordinator - choose mode Router or End Device
  @param  Security    - Enable or Disable security
  @param  Rejoin      - Device in Join or Rejoin mode
  @param  Mask        - Channel mask, we can send the desired channel number to advertise
  @Param: master key is the security key to verified at the time of joining
  @return
  - SUCCESS indicates the command is executed successfully.
  - FAILURE indicates the command is failed to execute.
 */
int32_t Zigbee_Join(uint32_t Coordinator, uint32_t Security, uint32_t ReJoin, uint32_t Mask, char *Master_key)
{
    int32_t               Ret_Val;
    qapi_Status_t         Result;
    qapi_ZB_Join_t        JoinConfig;
    uint32_t              ChannelMask;

    Ret_Val = SUCCESS;

    /* Verify the ZigBee layer had been initialized. */
    if (ZigBee_Demo_Context.ZigBee_Handle != NULL)
    {
        if ( (Mask != 0) &&  (Mask >= 11  && Mask <= 26) )

            ChannelMask = 1 << Mask;
        else
            ChannelMask = JOIN_CHANNEL_MASK;

        memset(&JoinConfig, 0, sizeof(qapi_ZB_Join_t));
        JoinConfig.IsRejoin                    = ReJoin;
        JoinConfig.EndDeviceTimeout            = Coordinator ? 0 : DEFAULT_END_DEVICE_TIME_OUT;
        JoinConfig.NetworkConfig.ExtendedPanId = 0ULL;
        JoinConfig.NetworkConfig.StackProfile  = QAPI_ZB_STACK_PROFILE_PRO_E;
        JoinConfig.NetworkConfig.ScanAttempts  = SCAN_ATTEMPT;
        JoinConfig.NetworkConfig.Page          = 0;
        JoinConfig.NetworkConfig.ChannelMask   = ChannelMask;
        JoinConfig.NetworkConfig.Capability    = Coordinator ? COORDINATOR_CAPABILITIES : END_DEVICE_CAPABILITIES;

        memscpy(&(JoinConfig.NetworkConfig.Security), sizeof(qapi_ZB_Security_t), &Default_ZB_Secuity, sizeof(qapi_ZB_Security_t));

        /* Overwrite the default security level. */
        JoinConfig.NetworkConfig.Security.Security_Level = Security ? QAPI_ZB_SECURITY_LEVEL_ENC_MIC32_E : QAPI_ZB_SECURITY_LEVEL_NONE_E;

        memcpy(JoinConfig.NetworkConfig.Security.Preconfigured_Link_Key, Master_key, 16);

        if (Ret_Val == SUCCESS)
        {
            Result = qapi_ZB_Join(ZigBee_Demo_Context.ZigBee_Handle, &JoinConfig);
            LOG_INFO("In Join Link Key  :%s\n", JoinConfig.NetworkConfig.Security.Preconfigured_Link_Key);
            if (Result == SUCCESS)
            {
                LOG_INFO("Suuccessfully joined Zigbee network\n");
            }
            else
            {
                LOG_ERROR("Failed to join zIgbee network\n");
                Ret_Val = FAILURE;
            }
        }
    }
    else
    {
        LOG_ERROR("ZigBee stack is not initialized.\n");
        Ret_Val = FAILURE;
    }
    return (Ret_Val);
}

qapi_Status_t qapi_ZB_ZDP_Mgmt_Leave_Req(qapi_ZB_Handle_t ZB_Handle, uint16_t DstNwkAddr, uint64_t DeviceAddress, qbool_t RemoveChildren, qbool_t Rejoin);


/**
  @brief Allows new devices to join the network.

  @param  PermitTime  - Time in seconds to allow device in Permit mode for joining the Network

  @return
  - SUCCESS indicates the command is executed successfully.
  - FAILURE indicates the command is failed to execute.
 */
int32_t Zigbee_PermitJoin(uint8_t PermitTime)
{
    int32_t             Ret_Val;
    qapi_Status_t       Result;
    /* Verify the ZigBee layer had been initialized. */
    if (ZigBee_Demo_Context.ZigBee_Handle != NULL)
    {
        Result = qapi_ZB_Permit_Join(ZigBee_Demo_Context.ZigBee_Handle, PermitTime);
        if (Result == SUCCESS)
        {
            LOG_INFO("Permitted to join\n");
            Ret_Val = SUCCESS;
        }
        else
        {
            LOG_ERROR("Failed to permit\n");
            Ret_Val = FAILURE;
        }
    }
    else
    {
        LOG_ERROR("ZigBee stack is not initialized.\n");
        Ret_Val = FAILURE;
    }

    return (Ret_Val);
}

/**
  @brief Displays the list of registered devices in the demo's device list.

  @param ExtAddr - Extendded address of a device which DeviceId to be found
  @param *DeviceId - Pointer to a Device ID to be get
  @param *ClEndPoint -  Pointer to a Cluster End point

  @return
  - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
  - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
 */
QCLI_Command_Status_t Zigbee_FindDeviceId(uint64_t ExtAddr, uint8_t *DeviceId)
{
    QCLI_Command_Status_t  Ret_Val;
    uint32_t               Index;

    Ret_Val = FAILURE;

    if(ZigBee_Demo_Context.ZigBee_Handle != 0)
    {

        for(Index = 0; Index <= DEV_ID_LIST_SIZE; Index++)
        {

            if(ZigBee_Demo_Context.DevIDList[Index].InUse)
            {

                if(ZigBee_Demo_Context.DevIDList[Index].Address.ExtendedAddress == ExtAddr)
                {
                    LOG_INFO(" Index= %d\n",Index);
                    *DeviceId = Index;
                    LOG_INFO("Device is registered with ExtAddr = %08X%08X \n",(unsigned int)(ExtAddr >> 32), (unsigned int)ExtAddr);
                    Ret_Val = SUCCESS;
                }
            }
        }
    }
    else
    {
        LOG_INFO("ZigBee stack is not initialized.\n");
        Ret_Val = FAILURE;
    }

    return Ret_Val;
}

int32_t Zigbee_PIR_Data_Send()
{

    uint8_t             CrdDeviceId;
    int32_t             Ret_Val;
    qapi_Status_t       Result;

    Zigbee_ShowDeviceList();
    Result = Zigbee_FindDeviceId(CrdExtAddr, &CrdDeviceId);
    if(Result == SUCCESS)
    {
        LOG_INFO("Device Id = %d\n",CrdDeviceId);
        Ret_Val = ZCL_PIR_SendCommand(CrdDeviceId, CUSTOM_CLUSTER_ENDPOINT);
    }
    else
    {
        LOG_ERROR("Failed to get Device Id\n");
        Ret_Val = FAILURE;
    }

    return Ret_Val;

}


int Process_Dimmable_Light(char *board_name, uint8_t level)
{

    uint32_t   Index;
    int BdMacLength = 6;
    uint8_t EndDevShortMac[8] = {0};
    char BdMac[7] = {0};
    uint32_t DevId;
    uint32_t EndPoint;
    uint32_t ret = FAILURE;

    EndPoint = DIMMER_CLUSTER_ENDPOINT;

    memcpy(BdMac, ((board_name+strlen(board_name))-6), BdMacLength);
    LOG_INFO(" /********** Dimmer BDMac = %s *********************/\n",BdMac);
    LOG_INFO(" Level value: %d\n", level);

    if(ZigBee_Demo_Context.ZigBee_Handle != 0)
    {

        for(Index = 0; Index <= DEV_ID_LIST_SIZE; Index++)
        {

            if(ZigBee_Demo_Context.DevIDList[Index].InUse)
            {
                snprintf ((char *)EndDevShortMac, 7, "%06llx",((~(~0 << 24)) & ZigBee_Demo_Context.DevIDList[Index].Address.ExtendedAddress));
                LOG_INFO(" /********** EndDevShortMac= %s *********************/\n",EndDevShortMac);
                LOG_INFO("Endpoint : %d\n",ZigBee_Demo_Context.DevIDList[Index].Endpoint);
                LOG_INFO("Endpoint : %d\n", EndPoint);
                LOG_INFO("Strcmp value: %d\n", strcmp((const char *) EndDevShortMac, (const char *)BdMac));

                if((!strcmp((const char *)EndDevShortMac, (const char *)BdMac)) && (EndPoint == ZigBee_Demo_Context.DevIDList[Index].Endpoint))
                {
                    DevId = Index;
                    LOG_INFO("/********* Calling Level Control Dimmable light**************/\n");
                    //ZCL_OnOff_On(DevId, DIMMER_CLUSTER_ENDPOINT); /* Dimmer light endpoint*/
                    ZCL_LevelControl_MoveToLevel (DevId, DIMMER_CLUSTER_ENDPOINT, true, level, 2);
                    ret = SUCCESS;
                }
            }
        }
    }

    return ret;
}

int Process_light(char *board_name, char *val)
{

    uint32_t   Index;
    int BdMacLength = 6;
    uint8_t EndDevShortMac[8] = "";
    char BdMac[7] = {0};
    uint8_t DevId;
    uint8_t state;

    state = *val - '0';

    memcpy(BdMac, board_name+(strlen(board_name)-7), BdMacLength);
    LOG_INFO("BDMac = %s\n",BdMac);

    if(ZigBee_Demo_Context.ZigBee_Handle != 0)
    {

        for(Index = 0; Index <= DEV_ID_LIST_SIZE; Index++)
        {

            if(ZigBee_Demo_Context.DevIDList[Index].InUse)
            {
                snprintf ((char *)EndDevShortMac, 7, "%06llx",((~(~0 << 24)) & ZigBee_Demo_Context.DevIDList[Index].Address.ExtendedAddress));
                if(!strcmp((const char *)EndDevShortMac, (const char *)BdMac))
                {
                    DevId = Index;
                    if(state == TRUE)
                    {
                        LOG_INFO("Calling ZCL_OnOff_On\n");
                        ZCL_OnOff_On(DevId,LIGHT_CLUSTER_ENDPOINT); /* light endpoint*/
                    }
                    else
                    {
                        LOG_INFO("Calling ZCL_OnOff_Off\n");
                        ZCL_OnOff_Off(DevId,LIGHT_CLUSTER_ENDPOINT);
                    }
                }
            }
        }
    }

    return SUCCESS;
}

/**
  @brief Displays the list of registered devices in the demo's device list.

  @param ExtAddr - Extendded address of a device which DeviceId to be found
  @param *DeviceId - Pointer to a Device ID to be get
  @param *ClEndPoint -  Pointer to a Cluster End point

  @return
  - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
  - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
 */
/** Coomand to be sent from AWS */


int32_t Zigbee_read_sensors(void)
{
    uint8_t             CrdDeviceId;
    int32_t             Ret_Val;
    qapi_Status_t       Result;

    Zigbee_ShowDeviceList();
    Result = Zigbee_FindDeviceId(CrdExtAddr, &CrdDeviceId);
    if(Result == SUCCESS)
    {
        LOG_INFO("Device Id = %d\n",CrdDeviceId);
        Ret_Val=ZCL_Custom_SendCommand(CrdDeviceId, CUSTOM_CLUSTER_ENDPOINT);
    }
    else
    {
        LOG_ERROR("Failed to get Device Id\n");
        Ret_Val = FAILURE;
    }

    return Ret_Val;
}



int32_t Zigbee_SetExtAddress(uint64_t ExtAddr)
{
    int32_t               Ret_Val;
    qapi_Status_t         Result;

    /* Verify the ZigBee layer had been initialized. */
    if (ZigBee_Demo_Context.ZigBee_Handle != NULL)
    {
        Result = qapi_ZB_Set_Extended_Address(ZigBee_Demo_Context.ZigBee_Handle, (0x00000000FFFFFFFFFF &ExtAddr));

        if(Result == SUCCESS)
        {
            LOG_INFO("Successfully set Extended Address\n");
            Ret_Val = SUCCESS;
        }
        else
        {
            LOG_ERROR("Failed to set Extended Address\n");
            Ret_Val = FAILURE;
        }
    }
    else
    {
        LOG_ERROR("ZigBee stack is not initialized.\n");
        Ret_Val = FAILURE;
    }

    return (Ret_Val);
}

/**
  @brief gets he addresses of the local ZigBee interface.

  @return
  - SUCCESS indicates the command is executed successfully.
  - FAILURE indicates the command is failed to execute.
 */
int32_t Zigbee_GetAddresses(void)
{
    int32_t               Ret_Val;
    qapi_Status_t         Result;
    uint64_t              ExtendedAddress;
    uint16_t              ShortAddress;
    uint16_t              AttributeLength;

    /* Verify the ZigBee layer had been initialized. */
    if (ZigBee_Demo_Context.ZigBee_Handle != NULL)
    {
        Result = qapi_ZB_Get_Extended_Address(ZigBee_Demo_Context.ZigBee_Handle, &ExtendedAddress);
        if (Result == SUCCESS)
        {
            AttributeLength = sizeof(uint16_t);
            Result = qapi_ZB_NLME_Get_Request(ZigBee_Demo_Context.ZigBee_Handle, QAPI_ZB_NIB_ATTRIBUTE_ID_NWK_NETWORK_ADDRESS_E, 0, &AttributeLength, (uint8_t *)&ShortAddress);
            if ((Result == SUCCESS) && (AttributeLength == sizeof(uint16_t)))
            {
                LOG_INFO("Extended Address: %08X%08X\n", (uint32_t)(ExtendedAddress >> 32), (uint32_t)ExtendedAddress);
                LOG_INFO("Short Address:    0x%04X\n", ShortAddress);
                Ret_Val = SUCCESS;
            }
            else
            {
                LOG_ERROR("Failed qapi_ZB_NLME_Get_Request\n");
                Ret_Val = FAILURE;
            }
        }
        else
        {
            LOG_ERROR("Failed qapi_ZB_Get_Extended_Address\n");
            Ret_Val = FAILURE;
        }
    }
    else
    {
        LOG_ERROR("ZigBee stack is not initialized.\n");
        Ret_Val = FAILURE;
    }

    return (Ret_Val);
}

/**
  @brief Callback handler for the ZigBee stack.

  @param ZB_Handle is the handle of the ZigBee instance was returned by a
  successful call to qapi_ZB_Initialize().
  @param CB_Param  is the user specified parameter for the callback function.
 */
static void ZB_Event_CB(qapi_ZB_Handle_t ZB_Handle, const qapi_ZB_Event_t *ZB_Event, uint32_t CB_Param)
{
    if ((ZigBee_Demo_Context.ZigBee_Handle != NULL) && (ZB_Handle == ZigBee_Demo_Context.ZigBee_Handle) && (ZB_Event != NULL))
    {
        switch (ZB_Event->Event_Type)
        {
            case QAPI_ZB_EVENT_TYPE_FORM_CONFIRM_E:
                LOG_INFO("Form confirm\n");
                LOG_INFO("  Status:  %d\n", ZB_Event->Event_Data.Form_Confirm.Status);
                LOG_INFO("  Channel: %d\n", ZB_Event->Event_Data.Form_Confirm.ActiveChannel);
                if(!ZB_Event->Event_Data.Form_Confirm.Status)
                {
                    /** Zigbee LED Blink */
                    R15_4_LED_CONFIG(1,50);
                }
                break;

            case QAPI_ZB_EVENT_TYPE_JOIN_CONFIRM_E:
                LOG_INFO("Join confirm:\n");
                LOG_INFO("  Status:        %d\n", ZB_Event->Event_Data.Join_Confirm.Status);
                LOG_INFO("  NwkAddress:    0x%04X\n", ZB_Event->Event_Data.Join_Confirm.NwkAddress);
                LOG_INFO("  ExtendedPanId: %08X%08X\n", (uint32_t)(ZB_Event->Event_Data.Join_Confirm.ExtendedPanId >> 32), (uint32_t)(ZB_Event->Event_Data.Join_Confirm.ExtendedPanId));
                LOG_INFO("  Channel:       %d\n", ZB_Event->Event_Data.Join_Confirm.ActiveChannel);

                if(!ZB_Event->Event_Data.Join_Confirm.Status)
                {
                    EndDev_Join_Confirm_Status = 1;
                    CrdExtAddr = ZB_Event->Event_Data.Join_Confirm.ExtendedPanId;
                    /** Zigbee LED Constant GLOW */
                    R15_4_LED_CONFIG(1,100);
                 //   Zigbee_AddDevice(DEVICE_MODE_EXTENDED, CrdExtAddr, DIMMER_CLUSTER_ENDPOINT);
                 //   Zigbee_AddDevice(DEVICE_MODE_EXTENDED, CrdExtAddr, LIGHT_CLUSTER_ENDPOINT);
                    Zigbee_AddDevice(DEVICE_MODE_EXTENDED, CrdExtAddr, CUSTOM_CLUSTER_ENDPOINT);
                    Zigbee_ShowDeviceList();
                }
                if (ZB_Event->Event_Data.Join_Confirm.Status)
                    zb_raise_rejoin_event();

                /** Adding Coordinator to End Device List */
                //Zigbee_AddDevice(DEVICE_MODE_EXTENDED, ZB_Event->Event_Data.Join_Confirm.ExtendedPanId, LIGHT_CLUSTER_ENDPOINT);
                //Zigbee_AddDevice(DEVICE_MODE_EXTENDED, ZB_Event->Event_Data.Join_Confirm.ExtendedPanId, DIMMER_CLUSTER_ENDPOINT);
                //Zigbee_ShowDeviceList();  
                break;
            default:
                LOG_WARN("Unhandled ZigBee Event: %d\n", ZB_Event->Event_Type);
                break;
        }

        QCLI_Display_Prompt();
    }
}

/**
  @brief Callback handler for the ZigBee Device Profile.

  @param ZB_Handle      is the handle of the ZigBee instance.
  @param ZDP_Event_Data is the information for the event.
  @param CB_Param       is the user specified parameter for the callback
  function.
 */
static void ZB_ZDP_Event_CB(qapi_ZB_Handle_t ZB_Handle, const qapi_ZB_ZDP_Event_t *ZDP_Event_Data, uint32_t CB_Param)
{
    if ((ZB_Handle == ZigBee_Demo_Context.ZigBee_Handle) && (ZDP_Event_Data != NULL))
    {
        switch (ZDP_Event_Data->Event_Type)
        {
            case QAPI_ZB_ZDP_EVENT_TYPE_DEVICE_ANNCE_E:
                LOG_INFO("Device Annce:\n");
                LOG_INFO("  ExtendedAddress: %08X%08X\n", (uint32_t)(ZDP_Event_Data->Event_Data.Device_Annce.IEEEAddr >> 32), (uint32_t)(ZDP_Event_Data->Event_Data.Device_Annce.IEEEAddr));
                LOG_INFO("  NetworkAddress:  0x%04X\n", ZDP_Event_Data->Event_Data.Device_Annce.NwkAddr);
                LOG_INFO("  Capability:      0x%02X\n", ZDP_Event_Data->Event_Data.Device_Annce.Capability);

                /** Zigbee LED Constant GLOW */
                R15_4_LED_CONFIG(1,100);

                /** Adding End device to Coordinator Device List */
                Zigbee_AddDevice(DEVICE_MODE_EXTENDED, ZDP_Event_Data->Event_Data.Device_Annce.IEEEAddr, CUSTOM_CLUSTER_ENDPOINT);
              //Zigbee_AddDevice(DEVICE_MODE_EXTENDED, ZDP_Event_Data->Event_Data.Device_Annce.IEEEAddr, LIGHT_CLUSTER_ENDPOINT);
                Zigbee_AddDevice(DEVICE_MODE_EXTENDED, ZDP_Event_Data->Event_Data.Device_Annce.IEEEAddr, DIMMER_CLUSTER_ENDPOINT);
                Zigbee_ShowDeviceList();  
                break;

            default:
                LOG_WARN("Unhandled ZDP Event: %d.\n", ZDP_Event_Data->Event_Type);
                break;
        }

        QCLI_Display_Prompt();
    }
}

/**
  @brief Helper function to format the send information for a packet.

  @param DeviceIndex is the index of the device to be sent.
  @param SendInfo    is a pointer to where the send information will be
  formatted upon successful return.

  @return true if the send info was formatted successfully, false otherwise.
 */
qbool_t Format_Send_Info_By_Device(uint32_t DeviceIndex, qapi_ZB_CL_General_Send_Info_t *SendInfo)
{
    qbool_t         Ret_Val;
    ZB_Device_ID_t *DeviceEntry;

    DeviceEntry = GetDeviceListEntry(DeviceIndex);
    if((SendInfo != NULL) && (DeviceEntry != NULL))
    {
        /* Found the device, now determine if it is a short address or a
           group address (no extended addresses used at this layer). */
        SendInfo->DstAddrMode = DeviceEntry->Type;
        SendInfo->DstAddress  = DeviceEntry->Address;
        SendInfo->DstEndpoint = DeviceEntry->Endpoint;
        SendInfo->SeqNum      = GetNextSeqNum();
		
		Ret_Val = true;
    }
    else
    {
        Ret_Val = false;
    }

    return(Ret_Val);
}

/**
  @brief Helper function to format the send information for a packet.

  @param ReceiveInfo is the receive information for an event.
  @param SendInfo    is a pointer to where the send information will be
  formatted upon successful return.

  @return true if the send info was formatted successfully, false otherwise.
 */
qbool_t Format_Send_Info_By_Receive_Info(const qapi_ZB_CL_General_Receive_Info_t *ReceiveInfo, qapi_ZB_CL_General_Send_Info_t *SendInfo)
{
    qbool_t Ret_Val;

    if((SendInfo != NULL) && (ReceiveInfo != NULL))
    {
        /* Found the device, now determine if it is a short address or a
           group address (no extended addresses used at this layer). */
        SendInfo->DstAddrMode             = QAPI_ZB_ADDRESS_MODE_SHORT_ADDRESS_E;
        SendInfo->DstAddress.ShortAddress = ReceiveInfo->SrcNwkAddress;
        SendInfo->DstEndpoint             = ReceiveInfo->SrcEndpoint;
        SendInfo->SeqNum                  = GetNextSeqNum();

        Ret_Val = true;
    }
    else
    {
        Ret_Val = false;
    }

    return(Ret_Val);
}

/**
  @brief Function to get a specified entry from the ZigBee demo's device list.

  @param DeviceID is the index of the device to retrieve.

  @return a pointer to the device list entry or NULL if either the DeviceID was
  not valid or not in use.
 */
ZB_Device_ID_t *GetDeviceListEntry(uint32_t DeviceID)
{
    ZB_Device_ID_t *Ret_Val;

    if(DeviceID <= DEV_ID_LIST_SIZE)
    {
        if(ZigBee_Demo_Context.DevIDList[DeviceID].InUse)
        {
            Ret_Val = &(ZigBee_Demo_Context.DevIDList[DeviceID]);
        }
        else
        {
            Ret_Val = NULL;
        }
    }
    else
    {
        Ret_Val = NULL;
    }

    return(Ret_Val);
}

/**
  @brief Function to get the ZigBee stack's handle.

  @return The handle of the ZigBee stack.
 */
qapi_ZB_Handle_t GetZigBeeHandle(void)
{
    return(ZigBee_Demo_Context.ZigBee_Handle);
}

/**
  @brief Function to get the next sequence number for sending packets.

  @return the next sequence number to be used for sending packets.
 */
uint8_t GetNextSeqNum(void)
{
    uint8_t Ret_Val = ZigBee_Demo_Context.ZCL_Sequence_Num;

    ZigBee_Demo_Context.ZCL_Sequence_Num++;
    return Ret_Val;
}

/**
  @brief Function to get the QCLI handle for the ZigBee demo.

  @return The QCLI handled used by the ZigBee demo.
 */
QCLI_Group_Handle_t GetZigBeeQCLIHandle(void)
{
    return(ZigBee_Demo_Context.QCLI_Handle);
}

/**
  @brief Helper function that displays variable length value.

  @param Group_Handle is the QCLI group handle.
  @param Data_Length  is the length of the data to be displayed.
  @param Data         is the data to be displayed.
 */
void DisplayVariableLengthValue(QCLI_Group_Handle_t Group_Handle, uint16_t Data_Length, const uint8_t *Data)
{
    union
    {
        uint8_t  ByteValue;
        uint16_t Unsigned16BitsValue;
        uint32_t Unsigned32BitsValue;
        uint64_t Unsigned64BitsValue;
    }VariableLengthValue;

    if(Data != NULL)
    {
        switch(Data_Length)
        {
            case sizeof(uint8_t):
                VariableLengthValue.ByteValue = READ_UNALIGNED_LITTLE_ENDIAN_UINT8(Data);
                QCLI_Printf(Group_Handle, "0x%02X\n", VariableLengthValue.ByteValue);
                break;

            case sizeof(uint16_t):
                VariableLengthValue.Unsigned16BitsValue = READ_UNALIGNED_LITTLE_ENDIAN_UINT16(Data);
                QCLI_Printf(Group_Handle, "0x%04X\n", VariableLengthValue.Unsigned16BitsValue);
                break;

            case sizeof(uint32_t):
                VariableLengthValue.Unsigned32BitsValue = READ_UNALIGNED_LITTLE_ENDIAN_UINT32(Data);
                QCLI_Printf(Group_Handle, "0x%04X\n", VariableLengthValue.Unsigned32BitsValue);
                break;

            case sizeof(uint64_t):
                VariableLengthValue.Unsigned64BitsValue = READ_UNALIGNED_LITTLE_ENDIAN_UINT64(Data);
                QCLI_Printf(Group_Handle, "%08X%08X\n", (uint32_t)(VariableLengthValue.Unsigned64BitsValue >> 32), (uint32_t)(VariableLengthValue.Unsigned64BitsValue));
                break;

            default:
                QCLI_Printf(Group_Handle, "Unsurport Data Length.\n");
                break;
        }
    }
}

