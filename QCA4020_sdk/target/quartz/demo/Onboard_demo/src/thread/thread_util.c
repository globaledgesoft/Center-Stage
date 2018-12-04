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
#include "qurt_timer.h"

#include "qapi_twn.h"
#include "qcli_api.h"
#include "qcli_util.h"

#include "qapi_socket.h"
#include "qapi_ns_utils.h"
#include "log_util.h"
#include "thread_util.h"
#include "sensor_json.h"

#include <stdarg.h>

#include "qapi_netservices.h"
#include "qapi/qapi_reset.h"

/* The prefix used for the default EUI64 address for the 802.15.4 MAC. The
   actual default EUI64 address is determined when the Initialize command is
   called by appending the short address. */
#define DEFAULT_EUI64_PREFIX                 (0x000000FFFE000000ULL)

/* This value is the default timeout for this device as a child. */
#define DEFAULT_CHILD_TIMEOUT                (60)

/* Uncomment this to enable printing logs from OpenThread. */
//#define ALLOW_OPENTHREAD_DEBUG_LOGS

extern int32_t thread_sockid;
extern struct sockaddr_in6 client6_addr;
sensor_info_t sensor_data;
static int32_t Joiner_Confirm_Status;
extern struct sockaddr_in6 border_router_addr;
static uint8_t PIR_Payload[PIR_PAYLOAD_LENGTH] = {0};
static uint32_t Joiner_Connection_Status = 0;
static uint8_t Thread_Payload[THREAD_PAYLOAD_LENGTH] = {0};

typedef struct Thread_Demo_Context_s
{
    QCLI_Group_Handle_t              QCLI_Handle;
    qapi_TWN_Handle_t                TWN_Handle;
    qapi_TWN_Device_Configuration_t  Device_Configuration;
    qapi_TWN_Network_Configuration_t Network_Configuration;
#ifdef ALLOW_OPENTHREAD_DEBUG_LOGS
    qbool_t                          EnableLogging;
#endif
} Thread_Demo_Context_t;

Thread_Demo_Context_t Thread_Demo_Context;
Thread_Context_t Thread_Context;

typedef struct QAPI_Status_String_Map_s
{
    qapi_Status_t  QapiStatus;
    const uint8_t *String;
} QAPI_Status_String_Map_t;

qapi_TWN_Network_Configuration_t Default_Network_Configuration =
{
    16,                                              /* Channel */
    0x8DA8,                                          /* PAN_ID */
    0x0001020304050607ULL,                           /* Extended_PAN_ID */
    "Test Network",                                  /* NetworkName */
    {0xF3, 0x2B, 0x7B, 0x51, 0x5A, 0xD6, 0x1B, 0xFA, /* MasterKey */
        0xF3, 0x2B, 0x7B, 0x51, 0x5A, 0xD6, 0x1B, 0xFA}
};

/* This is the test PSKc from the Thread Spec for the network configuration
   provided above. It is based on the Network Name, Ext. PAN ID and a passphrase
   of "12SECRETPASSWORD34" (this is entered in a commissioning app). */
const uint8_t Default_PSKc[16] = {
    0xc3, 0xf5, 0x93, 0x68,
    0x44, 0x5a, 0x1b, 0x61,
    0x06, 0xbe, 0x42, 0x0a,
    0x70, 0x6d, 0x4c, 0xc9
};

static const char *Vendor_Name  = "Qualcomm";
static const char *Vendor_Model = "Quartz OpenThread";
static const char *Vendor_SwVer = "0.0.1";
static const char *Vendor_Data  = "[Vendor Data]";

static void Print_Network_State_Event(qapi_TWN_Network_State_t State);
static void Print_Joiner_Result_Event(qapi_TWN_Joiner_Result_t Result);
static void DisplayNetworkInfo(void);
static void TWN_Event_CB(qapi_TWN_Handle_t TWN_Handle, const qapi_TWN_Event_t *TWN_Event, uint32_t CB_Param);

#ifdef THREAD_COMMAND_LIST
static QCLI_Command_Status_t cmd_Shutdown(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);

static QCLI_Command_Status_t cmd_Stop(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);

static QCLI_Command_Status_t cmd_GetDeviceConfiguration(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t cmd_GetNetworkConfiguration(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t cmd_SetExtendedAddress(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t cmd_SetChildTimeout(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t cmd_SetLinkMode(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t cmd_SetChannel(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t cmd_SetPANID(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t cmd_SetExtendedPANID(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t cmd_SetNetworkName(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t cmd_SetMasterKey(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);

static QCLI_Command_Status_t cmd_MeshCoP_CommissionerStart(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t cmd_MeshCoP_CommissionerStop(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t cmd_MeshCoP_CommissionerAddJoiner(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t cmd_MeshCoP_CommissionerDelJoiner(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);

static QCLI_Command_Status_t cmd_MeshCoP_JoinerStart(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t cmd_MeshCoP_JoinerStop(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);

static QCLI_Command_Status_t cmd_AddBorderRouter(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t cmd_RemoveBorderRouter(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t cmd_AddExternalRoute(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t cmd_RemoveExternalRoute(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t cmd_RegisterServerData(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);

static QCLI_Command_Status_t cmd_Thread_StartBorderAgent(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t cmd_Thread_StopBorderAgent(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);

static QCLI_Command_Status_t cmd_Thread_UpdatePSKc(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);

static QCLI_Command_Status_t cmd_Thread_ClearPersist(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);

static QCLI_Command_Status_t cmd_Thread_BecomeRouter(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t cmd_Thread_BecomeLeader(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);

static QCLI_Command_Status_t cmd_Thread_SetIPStackInteg(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t cmd_Thread_AddUnicastAddress(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t cmd_Thread_RemoveUnicastAddress(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t cmd_Thread_SubscribeMulticast(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t cmd_Thread_UnsubscribeMulticast(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t cmd_Thread_SetPingEnabled(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);

static QCLI_Command_Status_t cmd_Thread_SetProvisioningUrl(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);

#endif

#ifdef ALLOW_OPENTHREAD_DEBUG_LOGS
static QCLI_Command_Status_t cmd_Thread_SetLogging(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
#endif

static void DisplayNetworkInfo(void);
/* The following is the complete command list for the TWN demo. */
const QCLI_Command_t Thread_CMD_List[] =
{
    // cmd_function                     thread cmd_string                 usage_string                                                           description
#ifdef THREAD_COMMAND_LIST
    {Thread_Initialize,                    false, "Initialize",              "[Type (0=Router, 1=Sleepy)]",                                         "Initialize the Thread Wireless Interface."}
    {cmd_Shutdown,                      false, "Shutdown",                "",                                                                    "Shutdown the Thread Wireless Interface."},

        {cmd_Start,                         false, "Start",                   "",                                                                    "Start the Thread Interface, connecting or starting a network."},
        {cmd_Stop,                          false, "Stop",                    "",                                                                    "Stop the Thread Interface."},

        {cmd_GetDeviceConfiguration,        false, "GetDeviceConfiguration",  "",                                                                    "Get the device configuration information."},
        {cmd_GetNetworkConfiguration,       false, "GetNetworkConfiguration", "",                                                                    "Get the network configuration information."},
        {cmd_SetExtendedAddress,            false, "SetExtendedAddress",      "[ExtAddr]",                                                           "Set the device's extended address."},
        {cmd_SetChildTimeout,               false, "SetChildTimeout",         "[Timeout]",                                                           "Set the device's child timeout in seconds."},
        {cmd_SetLinkMode,                   false, "SetLinkMode",             "[RxOnWhenIdle] [UseSecureDataRequests] [IsFFD] [RequireNetworkData]", "Set the device's link mode configuration."},
        {cmd_SetChannel,                    false, "SetChannel",              "[Channel]",                                                           "Set the network's operating channel."},
        {cmd_SetPANID,                      false, "SetPANID",                "[PANID]",                                                             "Set the network's PAN ID."},
        {cmd_SetExtendedPANID,              false, "SetExtendedPANID",        "[ExtPANID]",                                                          "Set the network's extended PAN ID."},
        {cmd_SetNetworkName,                false, "SetNetworkName",          "[Name]",                                                              "Set the network's name."},
        {cmd_SetMasterKey,                  false, "SetMasterKey",            "[Key]",                                                               "Set the network's master key."},

        {cmd_MeshCoP_CommissionerStart,     false, "CommissionerStart",       "",                                                                    "Starts acting as a Commissioner."},
        {cmd_MeshCoP_CommissionerStop,      false, "CommissionerStop",        "",                                                                    "Stops acting as a Commissioner."},
        {cmd_MeshCoP_CommissionerAddJoiner, false, "CommissionerAddJoiner",   "[Passphrase] [ExtAddr/*] [Timeout]",                                  "Adds a joiner with passphrase. Extended address filter or * for any."},
        {cmd_MeshCoP_CommissionerDelJoiner, false, "CommissionerDelJoiner",   "[ExtAddr/*]",                                                         "Removes a specific joiner, * if not specified."},
        {cmd_MeshCoP_JoinerStart,           false, "JoinerStart",             "[Passphrase]",                                                        "Joins a network using a passphrase."},
        {cmd_MeshCoP_JoinerStop,            false, "JoinerStop",              "",                                                                    "Stops joining a network."},

        {cmd_AddBorderRouter,               false, "AddBorderRouter",         "[Prefix] [PrefixLength] [Pref (0-2)] [IsStable] [Flags]",             "Add a border router's information to the network data."},
        {cmd_RemoveBorderRouter,            false, "RemoveBorderRouter",      "[Prefix] [PrefixLength]",                                             "Remove a border router's information from the network data."},
        {cmd_AddExternalRoute,              false, "AddExternalRoute",        "[Prefix] [PrefixLength] [Pref (0-2)] [IsStable]",                     "Add an external route to the netweork data."},
        {cmd_RemoveExternalRoute,           false, "RemoveExternalRoute",     "[Prefix] [PrefixLength]",                                             "Remove an external route from the network data."},
        {cmd_RegisterServerData,            false, "RegisterServerData",      "",                                                                    "Register the pending network data with the network."},

        {cmd_Thread_UseDefaultInfo,         false, "UseDefaultInfo",          "",                                                                    "Populates the default network information."},

        {cmd_Thread_StartBorderAgent,       false, "StartBorderAgent",        "[wlan0/wlan1]",                                                       "Starts acting as a border agent on the specified interface (4020 only)."},
        {cmd_Thread_StopBorderAgent,        false, "StopBorderAgent",         "",                                                                    "Stops the border agent process (4020 only)."},

        {cmd_Thread_UpdatePSKc,             false, "UpdatePSKc",              "[Passphrase]",                                                        "Generates PSKc based on network info. Network must not be started."},
        {cmd_Thread_ClearPersist,           false, "ClearPersist",            "",                                                                    "Clears all persistent settings."},

        {cmd_Thread_BecomeRouter,           false, "BecomeRouter",            "",                                                                    "Attempts to upgrade to a router."},
        {cmd_Thread_BecomeLeader,           false, "BecomeLeader",            "",                                                                    "Attempts to become the Leader."},

        {cmd_Thread_SetIPStackInteg,        false, "SetIPStackInteg",         "[Enable 0/1]",                                                        "Enables or disables the use of QAPI sockets with the Thread interface."},
        {cmd_Thread_AddUnicastAddress,      false, "AddUnicastAddress",       "[Address] [PrefixLength] [Preferred (0/1)]",                          "Adds a static IP to the Thread interface."},
        {cmd_Thread_RemoveUnicastAddress,   false, "RemoveUnicastAddress",    "[Address]",                                                           "Removes a static IP from the Thread interface."},
        {cmd_Thread_SubscribeMulticast,     false, "SubscribeMulticast",      "[Address]",                                                           "Subscribes to a multicast address."},
        {cmd_Thread_UnsubscribeMulticast,   false, "UnsubscribeMulticast",    "[Address]",                                                           "Unsubscribes from a multicast address."},
        {cmd_Thread_SetPingEnabled,         false, "SetPingEnabled",          "[Enable 0/1]",                                                        "Enables or disables ping response on the Thread interface"},
        {cmd_Thread_SetProvisioningUrl,     false, "SetProvisioningUrl",      "[URL]",                                                               "Sets the Commissioner Provisioning URL for Joiner filtering."},

#endif
#ifdef ALLOW_OPENTHREAD_DEBUG_LOGS
        {cmd_Thread_SetLogging,             false, "SetLogging",              "[Enable 0/1]",                                                        "Enables OpenThread log messages."},
#endif

};


const QCLI_Command_Group_t Thread_CMD_Group = {"Thread", sizeof(Thread_CMD_List) / sizeof(QCLI_Command_t), Thread_CMD_List};

/**
  @brief Starts thread device based on mode.

  @param :  Mode is a Thread device mode router/joiner
  @Param :  PassPhrase is a security key to be verified at the time of joining

  @return
  - SUCCESS indicates the command is executed successfully.
  - FAILURE indicates the command is failed to execute.
 */

qurt_mutex_t Json_buf_lock;
int32_t Update_json(char *JsonDocumentBuffer, uint32_t Max_size_aws_buf);
uint32_t Start_thread(char Mode, char* PassPhrase)
{

    int32_t Ret_Val = SUCCESS;

    LOG_INFO("PassPhrase length =%d\n",strlen(PassPhrase));
    LOG_INFO("Starting Thread\n");
    switch (Mode)
    {
        case 'b':
        case 'B':
            LOG_INFO("Calling Start_Thread_Boarder_Router\n");
            qurt_mutex_create(&Json_buf_lock);
            if(FAILURE == Start_Thread_Boarder_Router(PassPhrase))
            {
                LOG_ERROR("Read Start_Thread_Boarder_Router is failing\n");
                qurt_mutex_delete(&Json_buf_lock);
                Ret_Val = FAILURE;
            }
            break;

        case 'r':
        case 'R':
            LOG_INFO("Calling Start_Thread_Router\n");
            if(FAILURE == Start_Thread_Router(PassPhrase))
            {
                LOG_ERROR("Read Start_Thread_Router is failing\n");
                Ret_Val = FAILURE;
            }
            break;
        case 'j':
        case 'J':
            LOG_INFO("Calling Start_Thread_Joiner\n");
            if(FAILURE == Start_Thread_Joiner(PassPhrase))
            {
                LOG_ERROR("Read Start_Thread_Joiner is failing\n");
                Ret_Val = FAILURE;
            }
            break;
        default :
            LOG_INFO("\n\n wrong option sent for thread opeartating mode/\n\n");
            Ret_Val = FAILURE;
    }
    return Ret_Val;
}

/**
  @brief Gets Joiner confirmation status.

 */
int32_t Get_Joiner_Confirm_Status()
{
    LOG_INFO("\n\n In Joiner_Confirm_Status = %d\n\n",Joiner_Confirm_Status);
    if(Joiner_Confirm_Status)
        return SUCCESS;
    else
        return FAILURE;
}

/**
  @brief Sends PIR data to Border Router.

 */
uint32_t Thread_PIR_Data_Send()
{
    uint8_t board_name[BOARD_NAME_BUF_SIZE] = {0};
    uint32_t Ret_Val = SUCCESS;

    if (!Joiner_Connection_Status)
        Ret_Val = FAILURE;

    memset(PIR_Payload, 0, PIR_PAYLOAD_LENGTH);
    memset(board_name, 0, BOARD_NAME_BUF_SIZE);
    get_localdevice_name((char *)board_name, BOARD_NAME_BUF_SIZE);
    LOG_INFO("/******** BOARD_NAME :%s\n", board_name);

    snprintf((char *) PIR_Payload, 15, "{\"Breached\":{");
    LOG_INFO("/************ PIR_Payload 15 string length:%d ********************/\n", strlen( (char *) PIR_Payload));
    LOG_INFO("\n\n PIR_Payload:%s\n\n",PIR_Payload);

    memcpy((char *) PIR_Payload + strlen((char *)PIR_Payload), board_name, strlen((char *)board_name));
    LOG_INFO("/************ PIR_Payload  string length:%d ********************/\n", strlen( (char *) PIR_Payload));
    LOG_INFO("\n\n PIR_Payload:%s\n\n",PIR_Payload);

    snprintf((char *) PIR_Payload + strlen((const char *)PIR_Payload), 37, ":{\"message\":\"Motion detected\"}}}");

    if(strlen((const char *)PIR_Payload) >= PIR_PAYLOAD_LENGTH)
    {
        LOG_INFO("\n\n Custom_Payload is grater than 100 bytes. Ignoring data:\n\n");
        Ret_Val = FAILURE;
    }

    LOG_INFO("\n\n PIR_Payload:%s\n\n",PIR_Payload);
    Send_data_to_router((char *)PIR_Payload);
    return Ret_Val;
}

/**
  @brief Reads sensor data from joiner device.

 */
void Thread_read_sensors()
{
    if (!Joiner_Connection_Status)
    {
        return;
    }

    memset(Thread_Payload, 0, THREAD_PAYLOAD_LENGTH);
    Update_json((char *)Thread_Payload, THREAD_PAYLOAD_LENGTH);
    LOG_INFO("\n\n Context Data:%s\n\n", Thread_Payload);
    Send_data_to_router((char *)Thread_Payload);
}

void get_board_name(char* Thread_Payload, char* board_name)
{
    char *Board_Name_Ptr = NULL;

    Board_Name_Ptr = strstr(Thread_Payload, "QCA402");
    if(Board_Name_Ptr != NULL)
        snprintf((char *) board_name, 15, "%s", Board_Name_Ptr);
    LOG_INFO("Board_Name = %s\n", board_name);
}

/**  @brief shows the thread devices list */
void Thread_ShowDeviceList()
{
    uint32_t Index;

    LOG_INFO("\n\nDevice List:\n");
    LOG_INFO(" ID | Board Name     | IP Address \n");
    LOG_INFO("----+----------------+---------------------\n");

    for(Index = 0; Index <= THREAD_DEV_ID_LIST_SIZE; Index++)
    {
        if((Thread_Context.DevIDList[Index].InUse))
            LOG_INFO(" %d | %s | %s \n", Index, Thread_Context.DevIDList[Index].BoardName, Thread_Context.DevIDList[Index].IpAddr);
    }
}

/** Reading the thread devices data */
void Read_Thread_Devices_Data(char *jsonbuf, uint32_t max_size)
{
    int Index;
    LOG_INFO("\n-----Remote Device Data--------\n");
    for(Index = 1; Index <= THREAD_DEV_ID_LIST_SIZE; Index++)
    {
        if ((Thread_Context.DevIDList[Index].InUse == TRUE))
        {
            qurt_mutex_lock(&Json_buf_lock);
            LOG_INFO("Length of Remote data : %s\n", (char *)Thread_Context.DevIDList[Index].Buffer);
            if (strlen((const char *)Thread_Context.DevIDList[Index].Buffer) > max_size)
            {
                qurt_mutex_unlock(&Json_buf_lock);
                break;
            }
            if (strlen((const char *)Thread_Context.DevIDList[Index].BreachBuf))
            {
                memset(jsonbuf, 0, max_size);
                snprintf(jsonbuf, max_size, "%s", Thread_Context.DevIDList[Index].BreachBuf);
                memset(Thread_Context.DevIDList[Index].BreachBuf, 0, THREAD_PAYLOAD_LENGTH);
                Notify_breach_update_from_remote_device(jsonbuf + strlen(BREACHED));


            }
            if (strlen((const char *)Thread_Context.DevIDList[Index].Buffer))
            {
                memset(jsonbuf, 0, max_size);
                snprintf(jsonbuf, max_size, "%s", Thread_Context.DevIDList[Index].Buffer);
                memset(Thread_Context.DevIDList[Index].Buffer, 0, THREAD_PAYLOAD_LENGTH);
                Send_Remote_device_update_to_aws(jsonbuf);
            }
            qurt_mutex_unlock(&Json_buf_lock);
        }
    }
    LOG_INFO("\n----------------------------\n");
}

/**  Getting data from joiner */
void Getting_Data_From_Joiner(char* Thread_Payload, char* IpAddr)
{
    int32_t              Index;
    int32_t      board_name_record = 0;
    char* board_name[20] = {0};
    uint32_t breach_buf = 0;
    //    uint32_t str_len = strlen((const char *)Thread_Payload);
#ifdef AWS_IOT
    if (strstr((char *)Thread_Payload, BREACHED))
    {
        LOG_INFO("Breach value is set\n");
        breach_buf = 1;
    }
#endif

    memset(board_name, 0, sizeof(board_name));
    get_board_name((char*)Thread_Payload, (char*)board_name);
    LOG_INFO("After Extracting Board_Name = %s\n", board_name);

    /* Update entry if any for same Extendded Address */
    for(Index = 1; Index <= THREAD_DEV_ID_LIST_SIZE; Index++)
    {
        if((Thread_Context.DevIDList[Index].InUse))
        {
            qurt_mutex_lock(&Json_buf_lock);
            if(!strcmp((const char*)(Thread_Context.DevIDList[Index].BoardName),(const char*) board_name))
            {
                board_name_record = 1;
                if (!strcmp((const char*)Thread_Context.DevIDList[Index].IpAddr,(const char*) IpAddr))
                {
                    if(breach_buf)
                    {
                        memset(Thread_Context.DevIDList[Index].BreachBuf, 0, sizeof(Thread_Context.DevIDList[Index].BreachBuf));
                        strcpy((char*)Thread_Context.DevIDList[Index].BreachBuf,(const char*)Thread_Payload);
                        //snprintf(Thread_Context.DevIDList[Index].BreachBuf, str_len+1, "%s", Thread_Payload);
                        qurt_mutex_unlock(&Json_buf_lock);
                        break;
                    }
                    else
                    {
                        memset(Thread_Context.DevIDList[Index].Buffer, 0, sizeof(Thread_Context.DevIDList[Index].Buffer));
                        strcpy((char*)Thread_Context.DevIDList[Index].Buffer,(const char*)Thread_Payload);
                        //snprintf(Thread_Context.DevIDList[Index].Buffer, str_len+1, "%s", Thread_Payload);
                        qurt_mutex_unlock(&Json_buf_lock);
                        break;
                    }
                }
                else
                {
                    memset(Thread_Context.DevIDList[Index].IpAddr, 0, sizeof(Thread_Context.DevIDList[Index].IpAddr));
                    strcpy((char*)Thread_Context.DevIDList[Index].IpAddr, (const char*)IpAddr);
                    //snprintf(Thread_Context.DevIDList[Index].IpAddr, INET6_ADDRSTRLEN, "%s", IpAddr);
                    if(breach_buf)
                    {
                        memset(Thread_Context.DevIDList[Index].BreachBuf, 0, sizeof(Thread_Context.DevIDList[Index].BreachBuf));
                        strcpy((char*)Thread_Context.DevIDList[Index].BreachBuf,(const char*)Thread_Payload);
                        //snprintf(Thread_Context.DevIDList[Index].BreachBuf, str_len+1, "%s", Thread_Payload);
                        qurt_mutex_unlock(&Json_buf_lock);
                        break;
                    }
                    else
                    {
                        memset(Thread_Context.DevIDList[Index].Buffer, 0, sizeof(Thread_Context.DevIDList[Index].Buffer));
                        strcpy((char*)Thread_Context.DevIDList[Index].Buffer,(const char*)Thread_Payload);
                        //snprintf(Thread_Context.DevIDList[Index].Buffer, str_len+1, "%s", Thread_Payload);
                        qurt_mutex_unlock(&Json_buf_lock);
                        break;
                    }

                }
            }
            qurt_mutex_unlock(&Json_buf_lock);
        }
    }

    if (!board_name_record)
    {
        for(Index = 1; Index <= THREAD_DEV_ID_LIST_SIZE; Index++)
        {
            if(!(Thread_Context.DevIDList[Index].InUse))
            {
                /* Found a spare slot. */
                qurt_mutex_lock(&Json_buf_lock);
                memset(Thread_Context.DevIDList[Index].BoardName, 0, sizeof(Thread_Context.DevIDList[Index].BoardName));
                strcpy((char*)Thread_Context.DevIDList[Index].BoardName, (const char*)board_name);
                //snprintf(Thread_Context.DevIDList[Index].BoardName, BOARD_NAME_SIZE, "%s", board_name);
                memset(Thread_Context.DevIDList[Index].IpAddr, 0, sizeof(Thread_Context.DevIDList[Index].IpAddr));
                strcpy((char*)Thread_Context.DevIDList[Index].IpAddr, (const char*)IpAddr);
                LOG_INFO("Board_Name :%s\t Thread_Context.DevIDList[Index].IpAddr:%s\n",Thread_Context.DevIDList[Index].BoardName,Thread_Context.DevIDList[Index].IpAddr);
                //snprintf(Thread_Context.DevIDList[Index].IpAddr, INET6_ADDRSTRLEN, "%s", IpAddr);
                if(breach_buf)
                {
                    memset(Thread_Context.DevIDList[Index].BreachBuf, 0, sizeof(Thread_Context.DevIDList[Index].BreachBuf));
                    strcpy((char*)Thread_Context.DevIDList[Index].BreachBuf,(const char*)Thread_Payload);
                    //snprintf(Thread_Context.DevIDList[Index].BreachBuf, str_len+1, "%s", Thread_Payload);
                    Thread_Context.DevIDList[Index].InUse = true;
                    qurt_mutex_unlock(&Json_buf_lock);
                    LOG_INFO("Registered Device Record: %d\n", Index);
                    break;
                }
                else
                {
                    memset(Thread_Context.DevIDList[Index].Buffer, 0, sizeof(Thread_Context.DevIDList[Index].Buffer));
                    strcpy((char*)Thread_Context.DevIDList[Index].Buffer,(const char*)Thread_Payload);
                    //snprintf(Thread_Context.DevIDList[Index].Buffer, str_len+1, "%s", Thread_Payload);
                    Thread_Context.DevIDList[Index].InUse = true;
                    qurt_mutex_unlock(&Json_buf_lock);
                    LOG_INFO("Registered Device Record: %d\n", Index);
                    break;
                }
            }
        }
    }
}

/**
  @brief Helper function to print out a Network State Event.

  @param State  The Network State to print out.
 */
static void Print_Network_State_Event(qapi_TWN_Network_State_t State)
{
    const char *State_String;

    switch(State)
    {
        case QAPI_TWN_NETWORK_STATE_DISABLED_E:
            State_String = "Disabled";
            break;

        case QAPI_TWN_NETWORK_STATE_DETACHED_E:
            State_String = "Detached";
            break;

        case QAPI_TWN_NETWORK_STATE_CHILD_E:
            State_String = "Child";
            break;

        case QAPI_TWN_NETWORK_STATE_ROUTER_E:
            State_String = "Router";
            break;

        case QAPI_TWN_NETWORK_STATE_LEADER_E:
            State_String = "Leader";
            break;

        default:
            State_String = "Unknown";
            break;
    }

    LOG_INFO("Network State Changed: %s\n", State_String);
    QCLI_Display_Prompt();
}

/**
  @brief Helper function to print out a Joiner Result Event.

  @param: Result is the Joiner Result to print.
 */
static void Print_Joiner_Result_Event(qapi_TWN_Joiner_Result_t Result)
{
    const char *Result_String;

    switch(Result)
    {
        case QAPI_TWN_JOINER_RESULT_SUCCESS_E:
            Result_String = "Success";
            Joiner_Connection_Status = 1;
            break;

        case QAPI_TWN_JOINER_RESULT_NO_NETWORKS_E:
            Result_String = "No Networks Found";
            break;

        case QAPI_TWN_JOINER_RESULT_SECURITY_E:
            Result_String = "Security Error";
            break;

        case QAPI_TWN_JOINER_RESULT_TIMEOUT_E:
            Result_String = "Timeout";
            break;

        case QAPI_TWN_JOINER_RESULT_ERROR_E:
            Result_String = "Other Error";
            break;

        default:
            Result_String = "Unknown";
            break;
    }

    LOG_INFO("Joiner Result: %s\n", Result_String);

    /* Also display the acquired network information if it succeeded. */
    if(Result == QAPI_TWN_JOINER_RESULT_SUCCESS_E)
    {
        DisplayNetworkInfo();
    }

    QCLI_Display_Prompt();
}

/**
  @brief Helper function to retrieve and print out the current Thread Network
  Information structure. This function will instead display an error if
  the information cannot be retrieved for any reason.
 */
static void DisplayNetworkInfo(void)
{
    qapi_Status_t                    Result;
    qapi_TWN_Network_Configuration_t Network_Config;
    uint8_t                          Index;

    if(Thread_Demo_Context.TWN_Handle != QAPI_TWN_INVALID_HANDLE)
    {
        Result = qapi_TWN_Get_Network_Configuration(Thread_Demo_Context.TWN_Handle, &Network_Config);
        if(Result == QAPI_OK)
        {
            /* Display the network configuration. */

            LOG_INFO("Network Configuration:\n");
            LOG_INFO("   Channel:          %d\n", Network_Config.Channel);
            LOG_INFO("   PAN_ID:           %04X\n", Network_Config.PAN_ID);
            LOG_INFO("   Extended_PAN_ID:  %08X%08X\n", (uint32_t)((Network_Config.Extended_PAN_ID) >> 32), (uint32_t)(Network_Config.Extended_PAN_ID));
            LOG_INFO("   NetworkName:      %s\n", Network_Config.NetworkName);
            LOG_INFO("   MasterKey:        ");


            for(Index = 0; Index < QAPI_OPEN_THREAD_MASTER_KEY_SIZE; Index++)
            {
                LOG_INFO("%02X", Network_Config.MasterKey[Index]);
            }

            LOG_INFO("\n");
        }
        else
        {
            LOG_ERROR("FAILURE: qapi_TWN_Get_Network_Configuration\n");
        }
    }
    else
    {
        LOG_ERROR("FAILURE: TWN not initialized.\n");
    }
}

/**
  @brief Handles callbacks from the Thread Wireless Interface.

  @param TWN_Event is a structure which contains the information for the
  event.
  @param CB_Param  is the user specified parameter for the callback function.
 */
static void TWN_Event_CB(qapi_TWN_Handle_t TWN_Handle, const qapi_TWN_Event_t *TWN_Event, uint32_t CB_Param)
{
    switch(TWN_Event->Event_Type)
    {
        case QAPI_TWN_EVENT_TYPE_NETWORK_STATE_E:
            Print_Network_State_Event(TWN_Event->Event_Data.Network_State);
            break;

        case QAPI_TWN_EVENT_TYPE_JOINER_RESULT_E:
            Print_Joiner_Result_Event(TWN_Event->Event_Data.Joiner_Result);
            break;

        default:
            LOG_INFO("Unknown Event Received (%u).\n", (uint32_t)TWN_Event->Event_Type);
            break;
    }
}

/**
  @brief Registers Thread demo commands with QCLI.
 */
void Initialize_Thread_Demo(void)
{
    memset(&Thread_Demo_Context, 0, sizeof(Thread_Demo_Context_t));

    Thread_Demo_Context.QCLI_Handle = QCLI_Register_Command_Group(NULL, &Thread_CMD_Group);

    if(Thread_Demo_Context.QCLI_Handle != NULL)
    {
        LOG_INFO("SUCCESS: Thread Demo Initialized.\n");
    }
    else
    {
        LOG_ERROR("Failed to register Thread commands.\n");
    }
}

/**
  @brief Sets a maximum limit on the poll period for SEDs.

  @param: Poll_Periodis Maximum requested poll time, in milliseconds. To clear
  a previously set maximum, set this to 0.

  @return
  - SUCCESS indicates the command is executed successfully.
  - FAILURE indicates the command is failed to execute.
 */
uint32_t Thread_SetPollPeriod(uint32_t Poll_Period)
{
    uint32_t Ret_Val;
    uint32_t Result;

    /* Verify the TWN layer is initialized. */
    if(Thread_Demo_Context.TWN_Handle != QAPI_TWN_INVALID_HANDLE)
    {

        Result = qapi_TWN_Set_Max_Poll_Period(Thread_Demo_Context.TWN_Handle, Poll_Period);

        if(Result == SUCCESS)
        {
            LOG_INFO("SUCCESS: qapi_TWN_Set_Max_Poll_Period\n");
            LOG_INFO("Max_Poll_Period set is: %d\n",Poll_Period);

            Ret_Val = SUCCESS;
        }
        else
        {
            LOG_ERROR("FAILURE: qapi_TWN_Set_Max_Poll_Period\n");
            Ret_Val = FAILURE;
        }
    }
    else
    {
        LOG_ERROR("FAILURE: TWN not initialized.\n");
        Ret_Val = FAILURE;
    }

    return(Ret_Val);
}

/**
  @brief Get the IPV6 address

  @param Interface indicates the Thread network interface name
  @param Global_IP_Str to get the Global IPv6 address

  @return
  - SUCCESS indicates the command is executed successfully.
  - FAILURE indicates the command is failed to execute.
 */

uint32_t Thread_Get_IPV6_Addr(const char *Interface, char *Global_IP_Str)
{
    uint32_t  Ret_Val;

    Ret_Val = SUCCESS;

    ip6_addr v6Global, v6GlobalExtd, v6LinkLocal, v6DefGw;
    uint32_t LinkPrefix, GlobalPrefix, DefGwPrefix, GlobalExtdPrefix;

    /* IPv6 */
    memset(&v6LinkLocal,0, sizeof(ip6_addr));
    memset(&v6Global,0, sizeof(ip6_addr));
    memset(&v6DefGw,0, sizeof(ip6_addr));
    memset(&v6GlobalExtd,0, sizeof(ip6_addr));
    LinkPrefix = GlobalPrefix = DefGwPrefix = GlobalExtdPrefix = 0;

    if (qapi_Net_IPv6_Get_Address(Interface,
                (uint8_t *)&v6LinkLocal,
                (uint8_t *)&v6Global,
                (uint8_t *)&v6DefGw,
                (uint8_t *)&v6GlobalExtd,
                &LinkPrefix,
                &GlobalPrefix,
                &DefGwPrefix,
                &GlobalExtdPrefix) == 0)
    {
        char ip_str[48];
        if (v6LinkLocal.s_addr[0])
        {
            if (inet_ntop(AF_INET6, &v6LinkLocal, ip_str, sizeof(ip_str)) != NULL)
            {
                if (LinkPrefix)
                    LOG_INFO(" IPv6 Link-local Address ..... : %s/%d\n", ip_str, LinkPrefix);
                else
                    LOG_INFO(" IPv6 Link-local Address ..... : %s\n", ip_str);
            }

            if (inet_ntop(AF_INET6, &v6Global, ip_str, sizeof(ip_str)) != NULL)
            {
                memcpy(Global_IP_Str, ip_str, sizeof(ip_str));

                if (GlobalPrefix)
                    LOG_INFO(" IPv6 Global Address ......... : %s/%d\n", ip_str, GlobalPrefix);
                else
                    LOG_INFO(" IPv6 Global Address ......... : %s\n", ip_str);
            }

            if (inet_ntop(AF_INET6, &v6DefGw, ip_str, sizeof(ip_str)) != NULL)
            {
                if (DefGwPrefix)
                    LOG_INFO(" IPv6 Default Gateway  ....... : %s/%d\n", ip_str, DefGwPrefix);
                else
                    LOG_INFO(" IPv6 Default Gateway  ....... : %s\n", ip_str);
            }

            if (inet_ntop(AF_INET6, &v6GlobalExtd, ip_str, sizeof(ip_str)) != NULL)
            {
                if (GlobalExtdPrefix)
                    LOG_INFO(" IPv6 Global Address 2 ....... : %s/%d\n", ip_str, GlobalExtdPrefix);
                else
                    LOG_INFO(" IPv6 Global Address 2 ....... : %s\n", ip_str);
            }
        }
    }

    return(Ret_Val);
}

/**
  @brief Initializes the thread interface and sets the default device and
  network configurations.

  @Param Device_Config indicates if the device should
  operation as a router or sleepy end device.

  @return
  - SUCCESS indicates the command is executed successfully.
  - FAILURE indicates the command is failed to execute.
 */
uint32_t Thread_Initialize(uint32_t Device_Config)
{
    LOG_ERROR("%s\n",__func__);
    uint32_t  Ret_Val;
    uint32_t     Result;

    /* Verify the TWN layer is not already initialized. */
    if(Thread_Demo_Context.TWN_Handle == QAPI_TWN_INVALID_HANDLE)
    {
        Result = qapi_TWN_Initialize(&(Thread_Demo_Context.TWN_Handle), TWN_Event_CB, 0);

        if((Result == SUCCESS) && (Thread_Demo_Context.TWN_Handle != QAPI_TWN_INVALID_HANDLE))
        {
            /* Set the default device information. */
            Thread_Demo_Context.Device_Configuration.Child_Timeout            = DEFAULT_CHILD_TIMEOUT;
            Thread_Demo_Context.Device_Configuration.Use_Secure_Data_Requests = true;

            if(Device_Config == 0)
            {
                LOG_INFO(" In Device_Config == 0\n");
                /* Router configuration. */
                Thread_Demo_Context.Device_Configuration.Is_FFD               = true;
                Thread_Demo_Context.Device_Configuration.Rx_On_While_Idle     = true;
                Thread_Demo_Context.Device_Configuration.Require_Network_Data = true;
            }
            else
            {
                LOG_INFO(" In Device_Config == 1\n");
                /* Sleepy Device configuration. */
                Thread_Demo_Context.Device_Configuration.Is_FFD               = false;
                Thread_Demo_Context.Device_Configuration.Rx_On_While_Idle     = false;
                Thread_Demo_Context.Device_Configuration.Require_Network_Data = false;
            }

            Result = qapi_TWN_Set_Device_Configuration(Thread_Demo_Context.TWN_Handle, &(Thread_Demo_Context.Device_Configuration));

            if(Result == SUCCESS)
            {
                /* Get the network information configured by default. */
                Result = qapi_TWN_Get_Network_Configuration(Thread_Demo_Context.TWN_Handle, &(Thread_Demo_Context.Network_Configuration));

                if(Result == SUCCESS)
                {
                    LOG_INFO("Thread Initialized Successfully:\n");
                    DisplayNetworkInfo();

                    Ret_Val = SUCCESS;
                }
                else
                {
                    LOG_ERROR("FAILURE: qapi_TWN_Get_Network_Configuration");
                    Ret_Val = FAILURE;
                }
            }
            else
            {
                LOG_ERROR("FAILURE: qapi_TWN_Set_Device_Configuration");
                Ret_Val = FAILURE;
            }

            if(Ret_Val != QCLI_STATUS_SUCCESS_E)
            {
                /* Error with initialization, shutdown the MAC. */
                qapi_TWN_Shutdown(Thread_Demo_Context.TWN_Handle);

                Thread_Demo_Context.TWN_Handle = QAPI_TWN_INVALID_HANDLE;
            }
        }
        else
        {
            LOG_ERROR("FAILURE: qapi_TWN_Initialize");
            Ret_Val = FAILURE;
        }
    }
    else
    {
        LOG_INFO("TWN already initialized.\n");

        Ret_Val = FAILURE;
    }

    return(Ret_Val);
}

uint32_t RegisterServerData()
{
    uint32_t  Ret_Val;
    uint32_t      Result;

    /* Verify the TWN layer is initialized. */
    if(Thread_Demo_Context.TWN_Handle != QAPI_TWN_INVALID_HANDLE)
    {
        Result = qapi_TWN_Register_Server_Data(Thread_Demo_Context.TWN_Handle);
        if(Result == SUCCESS)
        {
            Display_Function_Success(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Register_Server_Data");

            Ret_Val = SUCCESS;
        }
        else
        {
            Display_Function_Error(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Register_Server_Data", Result);

            Ret_Val = FAILURE;
        }
    }
    else
    {
        QCLI_Printf(Thread_Demo_Context.QCLI_Handle, "TWN not initialized.\n");
        Ret_Val = FAILURE;
    }

    return(Ret_Val);
}


/**
  @brief Convenience function to set up the default Network Data for this
  demo application.

  @param: void

  @return
  - SUCCESS indicates the command is executed successfully.
  - FAILURE indicates the command is failed to execute.
 */
uint32_t Thread_UseDefaultInfo(void)
{
    uint32_t            Ret_Val;
    uint32_t            Result;

    LOG_INFO("%s\n",__func__);
    /* Verify the TWN layer is initialized. */
    if(Thread_Demo_Context.TWN_Handle != QAPI_TWN_INVALID_HANDLE)
    {
        /* Set the default network information. */
        memscpy(&(Thread_Demo_Context.Network_Configuration), sizeof(qapi_TWN_Network_Configuration_t), &Default_Network_Configuration, sizeof(qapi_TWN_Network_Configuration_t));

        /* Get the network information configured by default. */
        Result = qapi_TWN_Set_Network_Configuration(Thread_Demo_Context.TWN_Handle, &(Thread_Demo_Context.Network_Configuration));

        if(Result == SUCCESS)
        {
            Result = qapi_TWN_Set_PSKc(Thread_Demo_Context.TWN_Handle, Default_PSKc);
            if(Result != SUCCESS)
            {
                Display_Function_Error(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Set_PSKc", Result);
                LOG_ERROR("FAILURE: qapi_TWN_Set_PSKc");
            }

            DisplayNetworkInfo();

            Ret_Val = SUCCESS;
        }
        else
        {
            Display_Function_Error(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Set_Network_Configuration", Result);
            LOG_ERROR("FAILURE: qapi_TWN_Set_Network_Configuration");

            Ret_Val = FAILURE;
        }
    }
    else
    {
        QCLI_Printf(Thread_Demo_Context.QCLI_Handle, "TWN not initialized.\n");
        LOG_ERROR("FAILURE: TWN not initialized.\n");
        Ret_Val = FAILURE;
    }

    return(Ret_Val);
}


/**
  @brief Attempts to commission onto an existing Thread network.

  @param: PassPhrase is device credential that must be supplied by the
  joining device. This is an ASCII string.

  @return
  - SUCCESS indicates the command is executed successfully.
  - FAILURE indicates the command is failed to execute.

 */
uint32_t Thread_MeshCoP_JoinerStart(const char * PassPhrase)
{
    QCLI_Command_Status_t            Ret_Val;
    qapi_Status_t                    Result;
    qapi_TWN_Joiner_Info_t           Joiner_Info;

    /* Verify the TWN layer is initialized. */
    if(Thread_Demo_Context.TWN_Handle != QAPI_TWN_INVALID_HANDLE)
    {

        Joiner_Info.PSKd              = PassPhrase;
        Joiner_Info.Provisioning_URL  = NULL;
        Joiner_Info.Vendor_Name       = Vendor_Name;
        Joiner_Info.Vendor_Model      = Vendor_Model;
        Joiner_Info.Vendor_Sw_Version = Vendor_SwVer;
        Joiner_Info.Vendor_Data       = Vendor_Data;

        Result = qapi_TWN_Joiner_Start(Thread_Demo_Context.TWN_Handle, &Joiner_Info);
        if(Result == SUCCESS)
        {
            Ret_Val = SUCCESS;
            Display_Function_Success(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Joiner_Start");
        }
        else
        {
            Result = FAILURE;
            Ret_Val = FAILURE;
            Display_Function_Error(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Joiner_Start", Result);
        }
    }
    else
    {
        QCLI_Printf(Thread_Demo_Context.QCLI_Handle, "TWN not initialized.\n");
        Ret_Val = FAILURE;
    }

    return Ret_Val;
}


/**
  @brief Adds a unicast address (and prefix) to a Thread network interface.

  @param: IPv6_Addr  IPV6 Address
  @param: Prefix_Len Combined address and prefix length to add.
  @param: Preferred  Flag indicating if this address is the preferred IP for
  this Thread device.
  @return
  - SUCCESS indicates the command is executed successfully.
  - FAILURE indicates the command is failed to execute.
 */
uint32_t Thread_AddUnicastAddress(const char *IPv6_Addr,uint8_t Prefix_Len, uint32_t Preferred)
{
    QCLI_Command_Status_t     Ret_Val;
    qapi_TWN_IPv6_Prefix_t    Prefix;
    qapi_Status_t             Result;

    /* Verify the TWN layer is initialized. */
    if(Thread_Demo_Context.TWN_Handle != QAPI_TWN_INVALID_HANDLE)
    {
        /* Parse the IPv6 address. */
        if(inet_pton(AF_INET6, IPv6_Addr, &(Prefix.Address)) == 0)
        {
            Prefix.Length = Prefix_Len;
            Preferred = (qbool_t) Preferred;

            Result = qapi_TWN_IPv6_Add_Unicast_Address(Thread_Demo_Context.TWN_Handle, &Prefix, Preferred);
            if(Result == QAPI_OK)
            {
                LOG_INFO("SUCCESS: qapi_TWN_IPv6_Add_Unicast_Address\n");

                Ret_Val = QCLI_STATUS_SUCCESS_E;
            }
            else
            {
                LOG_ERROR("FAILURE: qapi_TWN_IPv6_Add_Unicast_Address\n");

                Ret_Val = QCLI_STATUS_ERROR_E;
            }
        }
        else
        {
            /* The IPv6 Address is not valid. */
            Ret_Val = QCLI_STATUS_USAGE_E;
            LOG_ERROR("FAILURE: IPv6 Addr is not valid\n");
        }
    }
    else
    {
        QCLI_Printf(Thread_Demo_Context.QCLI_Handle, "TWN not initialized.\n");
        Ret_Val = QCLI_STATUS_ERROR_E;
    }

    return(Ret_Val);
}


int32_t Send_data_to_joiner(char *board_name, char* buf)
{
    uint32_t  Index;
    uint32_t  rc;
    int32_t bytes;
    int32_t len;

    len = sizeof(client6_addr);
    LOG_INFO (":%s\n", __func__);
    LOG_INFO ("Board name : %s\n", board_name);
    for(Index = 0; Index <= THREAD_DEV_ID_LIST_SIZE; Index++)
    {
        if(Thread_Context.DevIDList[Index].InUse)
        {
            LOG_INFO("inside Index inuse\n");
            LOG_INFO("Board name saved : %s\n", Thread_Context.DevIDList[Index].BoardName);
            rc = strncmp(((const char*)Thread_Context.DevIDList[Index].BoardName), (const char*)board_name+1, (strlen(board_name)-2));
            LOG_INFO("RC = %d\n", rc);
            if(!strncmp(((const char*)Thread_Context.DevIDList[Index].BoardName),(const char*) board_name+1, (strlen(board_name)-2)))
            {
                LOG_INFO("inside strncmp\n");
                LOG_INFO("\n\n %d | %s | %s \n", Index, Thread_Context.DevIDList[Index].BoardName, Thread_Context.DevIDList[Index].IpAddr);
                inet_pton(AF_INET6, (const char *)Thread_Context.DevIDList[Index].IpAddr, &client6_addr.sin_addr.s_addr);
                LOG_INFO("Buf Data:%s\n", buf);
                bytes = qapi_sendto(thread_sockid, buf, strlen(buf), 0, (struct sockaddr *) &client6_addr, len);
                LOG_INFO("Sending Bytes:%d\t%s\n", bytes, buf);

            }
        }
    }
    return SUCCESS;
}

/**
  @brief Get the IPv6 address

  @param Interface indicates the Thread network interface name
  @param Global_IP_Str to get the Global IPv6 address

  @return
  - SUCCESS indicates the command is executed successfully.
  - FAILURE indicates the command is failed to execute.
 */
uint32_t Thread_Get_IPv6_Addr(const char *Interface, char *Global_IP_Str)
{

    QCLI_Command_Status_t            Ret_Val;

    Ret_Val = SUCCESS;

    ip6_addr v6Global, v6GlobalExtd, v6LinkLocal, v6DefGw;
    uint32_t LinkPrefix, GlobalPrefix, DefGwPrefix, GlobalExtdPrefix;

    /* IPv6 */
    memset(&v6LinkLocal,0, sizeof(ip6_addr));
    memset(&v6Global,0, sizeof(ip6_addr));
    memset(&v6DefGw,0, sizeof(ip6_addr));
    memset(&v6GlobalExtd,0, sizeof(ip6_addr));
    LinkPrefix = GlobalPrefix = DefGwPrefix = GlobalExtdPrefix = 0;

    if (qapi_Net_IPv6_Get_Address(Interface,
                (uint8_t *)&v6LinkLocal,
                (uint8_t *)&v6Global,
                (uint8_t *)&v6DefGw,
                (uint8_t *)&v6GlobalExtd,
                &LinkPrefix,
                &GlobalPrefix,
                &DefGwPrefix,
                &GlobalExtdPrefix) == 0)
    {
        char ip_str[48];
        if (v6LinkLocal.s_addr[0])
        {
            if (inet_ntop(AF_INET6, &v6LinkLocal, ip_str, sizeof(ip_str)) != NULL)
            {
                if (LinkPrefix)
                    LOG_INFO(" IPv6 Link-local Address ..... : %s/%d\n", ip_str, LinkPrefix);
                else
                    LOG_INFO(" IPv6 Link-local Address ..... : %s\n", ip_str);
            }

            if (inet_ntop(AF_INET6, &v6Global, ip_str, sizeof(ip_str)) != NULL)
            {
                memcpy(Global_IP_Str, ip_str, sizeof(ip_str));

                if (GlobalPrefix)
                    LOG_INFO(" IPv6 Global Address ......... : %s/%d\n", ip_str, GlobalPrefix);
                else
                    LOG_INFO(" IPv6 Global Address ......... : %s\n", ip_str);
            }

            if (inet_ntop(AF_INET6, &v6DefGw, ip_str, sizeof(ip_str)) != NULL)
            {
                if (DefGwPrefix)
                    LOG_INFO(" IPv6 Default Gateway  ....... : %s/%d\n", ip_str, DefGwPrefix);
                else
                    LOG_INFO(" IPv6 Default Gateway  ....... : %s\n", ip_str);
            }

            if (inet_ntop(AF_INET6, &v6GlobalExtd, ip_str, sizeof(ip_str)) != NULL)
            {
                if (GlobalExtdPrefix)
                    LOG_INFO(" IPv6 Global Address 2 ....... : %s/%d\n", ip_str, GlobalExtdPrefix);
                else
                    LOG_INFO(" IPv6 Global Address 2 ....... : %s\n", ip_str);
            }
        }
    }

    return(Ret_Val);
}


/**
  @brief Add a joining device information to the Steering Information.

  @param: PassPhrase is device credential that must be supplied by the
  joining device. This is an ASCII string.

  @param: ExtAddr is an Extended address of the joining device. Set to
  zero to ignore.
  @param: Timeout is Number of milliseconds to allow devices to join
  the network.

  @return
  - SUCCESS indicates the command is executed successfully.
  - FAILURE indicates the command is failed to execute.
 */
uint32_t Thread_MeshCoP_CommissionerAddJoiner(const char *PassPhrase, uint64_t ExtAddr, uint32_t TimeOut)
{
    uint32_t    Ret_Val;
    uint32_t    Result;
    uint32_t    Timeout;



    LOG_ERROR("%s\n",__func__);
    /* Verify the TWN layer is initialized. */
    if(Thread_Demo_Context.TWN_Handle != QAPI_TWN_INVALID_HANDLE)
    {
        Ret_Val = SUCCESS;
        LOG_INFO(" PassPhrase = %s\n",PassPhrase);
        LOG_INFO(" ExtAddr  = %08X%08X\n", ((uint32_t)(ExtAddr >>32)),(uint32_t)ExtAddr);
        LOG_INFO(" Timeout  = %d\n",TimeOut);
        if(TimeOut >=1 && TimeOut <= 65535)
        {
            Timeout = TimeOut;
        }
        else
            Timeout = 120;

        Result = qapi_TWN_Commissioner_Add_Joiner(Thread_Demo_Context.TWN_Handle, ExtAddr, PassPhrase, Timeout);
        if(Result == SUCCESS)
        {
            Ret_Val = SUCCESS;
            Display_Function_Success(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Commissioner_Add_Joiner");
            LOG_INFO("SUCCESS:qapi_TWN_Commissioner_Add_Joiner\n");
        }
        else
        {
            Result = FAILURE;
            Ret_Val = FAILURE;
            Display_Function_Error(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Commissioner_Add_Joiner", Result);
            LOG_ERROR("FAILED: qapi_TWN_Commissioner_Add_Joiner           Restarting the device\n");
            qapi_System_Reset();
        }
    }
    else
    {
        QCLI_Printf(Thread_Demo_Context.QCLI_Handle, "TWN not initialized.\n");
        LOG_ERROR("FAILED: TWN not initialized.\n");
        Ret_Val = FAILURE;
    }

    return Ret_Val;
}


/**
  @brief Brings up the Thread Wireless Network using the network information
  that has been specified via qapi_TWN_Set_Network_Configuration().

  @param: void

  @return
  - SUCCESS indicates the command is executed successfully.
  - FAILURE indicates the command is failed to execute.
 */

uint32_t Thread_Interface_Start()
{
    uint32_t Ret_Val;
    uint32_t      Result;
    LOG_ERROR("%s\n",__func__);
    /* Verify the TWN layer is initialized. */
    if(Thread_Demo_Context.TWN_Handle != QAPI_TWN_INVALID_HANDLE)
    {
        Result = qapi_TWN_Start(Thread_Demo_Context.TWN_Handle);

        if(Result == SUCCESS)
        {
            Display_Function_Success(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Start");
            LOG_INFO("SUCCESS: qapi_TWN_Start");

            Ret_Val = SUCCESS;
            Joiner_Confirm_Status = 1;
        }
        else
        {
            Display_Function_Error(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Start", Result);
            LOG_ERROR("FAILURE: qapi_TWN_Start");

            Ret_Val = FAILURE;
        }
    }
    else
    {
        QCLI_Printf(Thread_Demo_Context.QCLI_Handle, "TWN not initialized.\n");
        LOG_ERROR("FAILURE: TWN not initialized.\n");
        Ret_Val = FAILURE;
    }

    return(Ret_Val);
}


/**
  @brief Start the Commissioner role on this device.

  @param: void

  @return
  - SUCCESS indicates the command is executed successfully.
  - FAILURE indicates the command is failed to execute.
 */
uint32_t Thread_MeshCoP_CommissionerStart()
{
    uint32_t    Ret_Val;
    uint32_t     Result;

    LOG_ERROR("%s\n",__func__);
    /* Verify the TWN layer is initialized. */
    if(Thread_Demo_Context.TWN_Handle != QAPI_TWN_INVALID_HANDLE)
    {
        Result = qapi_TWN_Commissioner_Start(Thread_Demo_Context.TWN_Handle);
        if(Result == SUCCESS)
        {
            Ret_Val = SUCCESS;
            Display_Function_Success(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Commissioner_Start");
            LOG_INFO("SUCCESS: qapi_TWN_Commissioner_Start");
        }
        else
        {
            Ret_Val = FAILURE;
            Display_Function_Error(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Commissioner_Start", Result);
            LOG_ERROR("ERROR: qapi_TWN_Commissioner_Start");
        }
    }
    else
    {
        QCLI_Printf(Thread_Demo_Context.QCLI_Handle, "TWN not initialized.\n");
        LOG_ERROR("ERROR: TWN not initialized.\n");
        Ret_Val = FAILURE;
    }

    return Ret_Val;
}

/**
  @brief Shuts down the Thread interface.

  @return
 */
void Thread_Shutdown(void) 
{

    Socket_close();
    qurt_mutex_delete(&Json_buf_lock);
    /* Verify the TWN layer is initialized. */
    if(Thread_Demo_Context.TWN_Handle != QAPI_TWN_INVALID_HANDLE)
    {
        qapi_TWN_Shutdown(Thread_Demo_Context.TWN_Handle);

        Thread_Demo_Context.TWN_Handle = QAPI_TWN_INVALID_HANDLE;

        LOG_INFO("TWN shutdown.\n");
    }
    else
    {
        LOG_ERROR("TWN not initialized.\n");
    }

    return ;
}


#ifdef THREAD_COMMAND_LIST
/**
  @brief Shuts down the Thread interface.

  @param void

  @return void
 */
void Thread_Shutdown(void)
{

    /* Verify the TWN layer is initialized. */
    if(Thread_Demo_Context.TWN_Handle != QAPI_TWN_INVALID_HANDLE)
    {
        qapi_TWN_Shutdown(Thread_Demo_Context.TWN_Handle);

        Thread_Demo_Context.TWN_Handle = QAPI_TWN_INVALID_HANDLE;

        LOG_INFO("TWN shutdown.\n");
    }
    else
    {
        LOG_ERROR("TWN not initialized.\n");
    }

    return ;
}

/**
  @brief Registers any pending network data on this device with the Leader.

  @param: void

  @return
  - SUCCESS indicates the command is executed successfully.
  - FAILURE indicates the command is failed to execute
 */


/**
  @brief Starts acting as a Border Agent on a specified WLAN interface.
  Available only on the QCA4020 with active WLAN connection.

  @param: Wlan_Interface is the WLAN interface to use for MDNS (wlan0/wlan1)

  @return
  - SUCCESS indicates the command is executed successfully.
  - FAILURE indicates the command is failed to execute
 */
uint32_t Thread_StartBorderAgent(char * Wlan_Interface)
{
    uint32_t            Ret_Val;
    uint32_t            Result;

    /* Verify the TWN layer is initialized. */
    if(Thread_Demo_Context.TWN_Handle != QAPI_TWN_INVALID_HANDLE)
    {
        Result = qapi_TWN_Start_Border_Agent(Thread_Demo_Context.TWN_Handle, QAPI_TWN_AF_INET, "Quartz Border Router", "thread.local.", Wlan_Interface);

        if(Result == SUCCESS)
        {
            Display_Function_Success(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Start_Border_Agent");
            Ret_Val = SUCCESS;
        }
        else
        {
            Display_Function_Error(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Start_Border_Agent", Result);
            Ret_Val = FAILURE;
        }
    }
    else
    {
        QCLI_Printf(Thread_Demo_Context.QCLI_Handle, "TWN not initialized.\n");
        Ret_Val = FAILURE;
    }

    return(Ret_Val);
}

/**
  @brief Add a border router's information to the network data.

  @param: Ipv6PrefixAddr is IPv6 prefix address.
  @param: PrefixLen  is the length of Prefix
  @param: Preference is the Preference
  @param: Stability  is the Stability
  @param: BRConfigFlags is flags of BRConfiguration

  @return
  - SUCCESS indicates the command is executed successfully.
  - FAILURE indicates the command is failed to execute
 */
uint32_t Thread_AddBorderRouter(char *Ipv6PrefixAddr, uint8_t PrefixLen, uint32_t Preference, uint32_t Stability, uint32_t BRConfigFlags)
{
    uint32_t     Ret_Val;
    qapi_TWN_Border_Router_t Border_Router_Config;
    uint32_t                 Result;

    /* Verify the TWN layer is initialized. */
    if(Thread_Demo_Context.TWN_Handle != QAPI_TWN_INVALID_HANDLE)
    {
        if((PrefixLen >= 0 && PrefixLen <= 128) &&
                (Preference >=  0 && Preference <= 2) &&
                (Stability >= 0 && Stability <= 1) &&
                (BRConfigFlags > 0))
        {
            /* Parse the IPv6 address. */
            if(inet_pton(AF_INET6, (const char *)Ipv6PrefixAddr, &(Border_Router_Config.Prefix.Address)) == 0)
            {
                /* Set up the remaining fields of Border_Router_Config. */
                Border_Router_Config.Prefix.Length = PrefixLen;
                Border_Router_Config.Preference    = (qapi_TWN_Routing_Preference_t) Preference;
                Border_Router_Config.Is_Stable     = Stability;
                Border_Router_Config.Flags         = BRConfigFlags;

                Result = qapi_TWN_Add_Border_Router(Thread_Demo_Context.TWN_Handle, &Border_Router_Config);
                if(Result == SUCCESS)
                {
                    Display_Function_Success(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Add_Border_Router");

                    Ret_Val = SUCCESS;
                }
                else
                {
                    Display_Function_Error(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Add_Border_Router", Result);

                    Ret_Val = FAILURE;
                }
            }
            else
            {
                /* The IPv6 Address is not valid. */
                Ret_Val = FAILURE;
            }
        }
        else
        {
            Ret_Val = FAILURE;
        }
    }
    else
    {
        QCLI_Printf(Thread_Demo_Context.QCLI_Handle, "TWN not initialized.\n");
        Ret_Val = FAILURE;
    }

    return(Ret_Val);
}


/**
  @brief Stop the Thread Interface.

  @param Parameter_Count is number of elements in Parameter_List.
  @param Parameter_List  is the list of parsed arguments associated with
  this command.

  @return
  - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
  - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
  - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
  command.
 */
static QCLI_Command_Status_t cmd_Stop(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    QCLI_Command_Status_t Ret_Val;
    qapi_Status_t         Result;

    /* Verify the TWN layer is initialized. */
    if(Thread_Demo_Context.TWN_Handle != QAPI_TWN_INVALID_HANDLE)
    {
        Result = qapi_TWN_Stop(Thread_Demo_Context.TWN_Handle);

        if(Result == QAPI_OK)
        {
            Display_Function_Success(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Stop");

            Ret_Val = QCLI_STATUS_SUCCESS_E;
        }
        else
        {
            Display_Function_Error(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Stop", Result);

            Ret_Val = QCLI_STATUS_ERROR_E;
        }
    }
    else
    {
        QCLI_Printf(Thread_Demo_Context.QCLI_Handle, "TWN not initialized.\n");
        Ret_Val = QCLI_STATUS_ERROR_E;
    }

    return(Ret_Val);
}

/**
  @brief Get the device configuration information.

  @param Parameter_Count is number of elements in Parameter_List.
  @param Parameter_List  is the list of parsed arguments associated with
  this command.

  @return
  - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
  - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
  - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
  command.
 */
static QCLI_Command_Status_t cmd_GetDeviceConfiguration(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    QCLI_Command_Status_t           Ret_Val;
    qapi_TWN_Device_Configuration_t Device_Config;
    qapi_Status_t                   Result;

    /* Verify the TWN layer is initialized. */
    if(Thread_Demo_Context.TWN_Handle != QAPI_TWN_INVALID_HANDLE)
    {
        Result = qapi_TWN_Get_Device_Configuration(Thread_Demo_Context.TWN_Handle, &Device_Config);

        if(Result == QAPI_OK)
        {
            /* Display the device configuration. */
            QCLI_Printf(Thread_Demo_Context.QCLI_Handle, "Device Configuration:\n");
            QCLI_Printf(Thread_Demo_Context.QCLI_Handle, "   Extended Address:        %08X%08X\n", (uint32_t)((Device_Config.Extended_Address) >> 32), (uint32_t)(Device_Config.Extended_Address));
            QCLI_Printf(Thread_Demo_Context.QCLI_Handle, "   Child Timeout:           %d seconds\n", Device_Config.Child_Timeout);
            QCLI_Printf(Thread_Demo_Context.QCLI_Handle, "   Rx On While Idle:        %d\n", Device_Config.Rx_On_While_Idle);
            QCLI_Printf(Thread_Demo_Context.QCLI_Handle, "   Use Secure Data Request: %d\n", Device_Config.Use_Secure_Data_Requests);
            QCLI_Printf(Thread_Demo_Context.QCLI_Handle, "   Is FFD:                  %d\n", Device_Config.Is_FFD);
            QCLI_Printf(Thread_Demo_Context.QCLI_Handle, "   Require Network Data:    %d\n", Device_Config.Require_Network_Data);

            Ret_Val = QCLI_STATUS_SUCCESS_E;
        }
        else
        {
            Display_Function_Error(Thread_Demo_Context.QCLI_Handle, "qapi_HMI_Get_Device_Configuration", Result);

            Ret_Val = QCLI_STATUS_ERROR_E;
        }
    }
    else
    {
        QCLI_Printf(Thread_Demo_Context.QCLI_Handle, "TWN not initialized.\n");
        Ret_Val = QCLI_STATUS_ERROR_E;
    }

    return(Ret_Val);
}

/**
  @brief Get the network configuration information.

  @param Parameter_Count is number of elements in Parameter_List.
  @param Parameter_List  is the list of parsed arguments associated with
  this command.

  @return
  - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
  - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
  - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
  command.
 */
static QCLI_Command_Status_t cmd_GetNetworkConfiguration(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    QCLI_Command_Status_t            Ret_Val;
    qapi_TWN_Network_Configuration_t Network_Config;
    qapi_Status_t                    Result;
    uint8_t                          Index;

    /* Verify the TWN layer is initialized. */
    if(Thread_Demo_Context.TWN_Handle != QAPI_TWN_INVALID_HANDLE)
    {
        Result = qapi_TWN_Get_Network_Configuration(Thread_Demo_Context.TWN_Handle, &Network_Config);

        if(Result == QAPI_OK)
        {
            /* Display the device cofiguration. */
            QCLI_Printf(Thread_Demo_Context.QCLI_Handle, "Network Configuration:\n");
            QCLI_Printf(Thread_Demo_Context.QCLI_Handle, "   Channel:          %d\n", Network_Config.Channel);
            QCLI_Printf(Thread_Demo_Context.QCLI_Handle, "   PAN_ID:           %04X\n", Network_Config.PAN_ID);
            QCLI_Printf(Thread_Demo_Context.QCLI_Handle, "   Extended_PAN_ID:  %08X%08X\n", (uint32_t)((Network_Config.Extended_PAN_ID) >> 32), (uint32_t)(Network_Config.Extended_PAN_ID));
            QCLI_Printf(Thread_Demo_Context.QCLI_Handle, "   NetworkName:      %s\n", Network_Config.NetworkName);
            QCLI_Printf(Thread_Demo_Context.QCLI_Handle, "   MasterKey:        ");

            for(Index = 0; Index < QAPI_OPEN_THREAD_MASTER_KEY_SIZE; Index++)
            {
                QCLI_Printf(Thread_Demo_Context.QCLI_Handle, "%02X", Network_Config.MasterKey[Index]);
            }

            QCLI_Printf(Thread_Demo_Context.QCLI_Handle, "\n");

            Ret_Val = QCLI_STATUS_SUCCESS_E;
        }
        else
        {
            Display_Function_Error(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Get_Network_Configuration", Result);

            Ret_Val = QCLI_STATUS_ERROR_E;
        }
    }
    else
    {
        QCLI_Printf(Thread_Demo_Context.QCLI_Handle, "TWN not initialized.\n");
        Ret_Val = QCLI_STATUS_ERROR_E;
    }

    return(Ret_Val);
}

/**
  @brief Set the device's extended address in the Thread Device configuration.

  Parameter_List[0] 64-bit extended address of the device in hexadecimal
  format. The "0x" prefix is optional.

  @param Parameter_Count is number of elements in Parameter_List.
  @param Parameter_List  is the list of parsed arguments associated with
  this command.

  @return
  - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
  - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
  - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
  command.
 */
static QCLI_Command_Status_t cmd_SetExtendedAddress(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    QCLI_Command_Status_t           Ret_Val;
    qapi_TWN_Device_Configuration_t Device_Config;
    uint64_t                        Extended_Address;
    qapi_Status_t                   Result;

    /* Verify the TWN layer is initialized. */
    if(Thread_Demo_Context.TWN_Handle != QAPI_TWN_INVALID_HANDLE)
    {
        if((Parameter_Count >= 1) &&
                (Hex_String_To_ULL(Parameter_List[0].String_Value, &Extended_Address)))
        {
            /* Read back the current device configuration. */
            Result = qapi_TWN_Get_Device_Configuration(Thread_Demo_Context.TWN_Handle, &Device_Config);
            if(Result == QAPI_OK)
            {
                /* Set up the extended address feild in device configuration. */
                Device_Config.Extended_Address = Extended_Address;

                /* Write back the device configuration. */
                Result = qapi_TWN_Set_Device_Configuration(Thread_Demo_Context.TWN_Handle, &Device_Config);
                if(Result == QAPI_OK)
                {
                    Display_Function_Success(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Set_Device_Configuration");

                    Ret_Val = QCLI_STATUS_SUCCESS_E;
                }
                else
                {
                    Display_Function_Error(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Set_Device_Configuration", Result);

                    Ret_Val = QCLI_STATUS_ERROR_E;
                }
            }
            else
            {
                Display_Function_Error(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Get_Device_Configuration", Result);

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
        QCLI_Printf(Thread_Demo_Context.QCLI_Handle, "TWN not initialized.\n");
        Ret_Val = QCLI_STATUS_ERROR_E;
    }

    return(Ret_Val);
}

/**
  @brief Set the device's child timeout in seconds, in the Thread device
  configuration.

  Parameter_List[0] Timeout of the device as a child in seconds.

  @param Parameter_Count is number of elements in Parameter_List.
  @param Parameter_List  is the list of parsed arguments associated with
  this command.

  @return
  - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
  - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
  - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
  command.
 */
static QCLI_Command_Status_t cmd_SetChildTimeout(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    QCLI_Command_Status_t           Ret_Val;
    qapi_TWN_Device_Configuration_t Device_Config;
    qapi_Status_t                   Result;

    /* Verify the TWN layer is initialized. */
    if(Thread_Demo_Context.TWN_Handle != QAPI_TWN_INVALID_HANDLE)
    {
        if((Parameter_Count >= 1) &&
                (Parameter_List[0].Integer_Is_Valid))
        {
            /* Read back the current device configuration. */
            Result = qapi_TWN_Get_Device_Configuration(Thread_Demo_Context.TWN_Handle, &Device_Config);
            if(Result == QAPI_OK)
            {
                /* Set up the child timeout feild in device configuration. */
                Device_Config.Child_Timeout = (uint32_t)(Parameter_List[0].Integer_Value);

                /* Write back the device configuration. */
                Result = qapi_TWN_Set_Device_Configuration(Thread_Demo_Context.TWN_Handle, &Device_Config);
                if(Result == QAPI_OK)
                {
                    Display_Function_Success(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Set_Device_Configuration");

                    Ret_Val = QCLI_STATUS_SUCCESS_E;
                }
                else
                {
                    Display_Function_Error(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Set_Device_Configuration", Result);

                    Ret_Val = QCLI_STATUS_ERROR_E;
                }
            }
            else
            {
                Display_Function_Error(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Get_Device_Configuration", Result);

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
        QCLI_Printf(Thread_Demo_Context.QCLI_Handle, "TWN not initialized.\n");
        Ret_Val = QCLI_STATUS_ERROR_E;
    }

    return(Ret_Val);
}

/**
  @brief Set the link mode in the Thread device configuration.

  Parameter_List[0] (0-1) Flag indicating if the device's receiver is on when
  the device is idle.
  Parameter_List[1] (0-1) Flag indicating if the device uses secure data
  requests.
  Parameter_List[2] (0-1) Flag indicating if the device is an FFD.
  Parameter_List[3] (0-1) Flag indicating if the device requires full network
  data.

  @param Parameter_Count is number of elements in Parameter_List.
  @param Parameter_List  is the list of parsed arguments associated with
  this command.

  @return
  - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
  - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
  - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
  command.
 */
static QCLI_Command_Status_t cmd_SetLinkMode(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    QCLI_Command_Status_t           Ret_Val;
    qapi_TWN_Device_Configuration_t Device_Config;
    qapi_Status_t                   Result;

    /* Verify the TWN layer is initialized. */
    if(Thread_Demo_Context.TWN_Handle != QAPI_TWN_INVALID_HANDLE)
    {
        if((Parameter_Count >= 1) &&
                (Verify_Integer_Parameter(&(Parameter_List[0]), 0, 1)) &&
                (Verify_Integer_Parameter(&(Parameter_List[1]), 0, 1)) &&
                (Verify_Integer_Parameter(&(Parameter_List[2]), 0, 1)) &&
                (Verify_Integer_Parameter(&(Parameter_List[3]), 0, 1)))
        {
            /* Read back the current device configuration. */
            Result = qapi_TWN_Get_Device_Configuration(Thread_Demo_Context.TWN_Handle, &Device_Config);
            if(Result == QAPI_OK)
            {
                /* Set up the rx on while idle, use secure data request, FFD and
                   require network data fields in device configuration. */
                Device_Config.Rx_On_While_Idle         = (qbool_t)Parameter_List[0].Integer_Value;
                Device_Config.Use_Secure_Data_Requests = (qbool_t)Parameter_List[1].Integer_Value;
                Device_Config.Is_FFD                   = (qbool_t)Parameter_List[2].Integer_Value;
                Device_Config.Require_Network_Data     = (qbool_t)Parameter_List[3].Integer_Value;

                /* Write back the device configuration. */
                Result = qapi_TWN_Set_Device_Configuration(Thread_Demo_Context.TWN_Handle, &Device_Config);
                if(Result == QAPI_OK)
                {
                    Display_Function_Success(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Set_Device_Configuration");

                    Ret_Val = QCLI_STATUS_SUCCESS_E;
                }
                else
                {
                    Display_Function_Error(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Set_Device_Configuration", Result);

                    Ret_Val = QCLI_STATUS_ERROR_E;
                }
            }
            else
            {
                Display_Function_Error(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Get_Device_Configuration", Result);

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
        QCLI_Printf(Thread_Demo_Context.QCLI_Handle, "TWN not initialized.\n");
        Ret_Val = QCLI_STATUS_ERROR_E;
    }

    return(Ret_Val);
}

/**
  @brief Set the Thread network channel in the Thread Network information.

  Parameter_List[0] (11-26) Channel of the network.

  @param Parameter_Count is number of elements in Parameter_List.
  @param Parameter_List  is the list of parsed arguments associated with
  this command.

  @return
  - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
  - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
  - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
  command.
 */
static QCLI_Command_Status_t cmd_SetChannel(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    QCLI_Command_Status_t            Ret_Val;
    qapi_TWN_Network_Configuration_t Network_Config;
    qapi_Status_t                    Result;

    /* Verify the TWN layer is initialized. */
    if(Thread_Demo_Context.TWN_Handle != QAPI_TWN_INVALID_HANDLE)
    {
        if((Parameter_Count >= 1) &&
                (Verify_Integer_Parameter(&(Parameter_List[0]), 11, 26)))
        {
            /* Read back the current network configuration. */
            Result = qapi_TWN_Get_Network_Configuration(Thread_Demo_Context.TWN_Handle, &Network_Config);
            if(Result == QAPI_OK)
            {
                /* Set up the channel feild in network configuration. */
                Network_Config.Channel = (uint8_t)(Parameter_List[0].Integer_Value);

                /* Write back the network configuration. */
                Result = qapi_TWN_Set_Network_Configuration(Thread_Demo_Context.TWN_Handle, &Network_Config);
                if(Result == QAPI_OK)
                {
                    Display_Function_Success(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Set_Network_Configuration");

                    Ret_Val = QCLI_STATUS_SUCCESS_E;
                }
                else
                {
                    Display_Function_Error(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Set_Network_Configuration", Result);

                    Ret_Val = QCLI_STATUS_ERROR_E;
                }
            }
            else
            {
                Display_Function_Error(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Get_Network_Configuration", Result);

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
        QCLI_Printf(Thread_Demo_Context.QCLI_Handle, "TWN not initialized.\n");
        Ret_Val = QCLI_STATUS_ERROR_E;
    }

    return(Ret_Val);
}

/**
  @brief Set the Thread network PAN ID in the Thread Network configuration.

  Parameter_List[0] The 16-bit PAN ID of the network.

  @param Parameter_Count is number of elements in Parameter_List.
  @param Parameter_List  is the list of parsed arguments associated with
  this command.

  @return
  - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
  - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
  - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
  command.
 */
static QCLI_Command_Status_t cmd_SetPANID(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    QCLI_Command_Status_t            Ret_Val;
    qapi_TWN_Network_Configuration_t Network_Config;
    qapi_Status_t                    Result;

    /* Verify the TWN layer is initialized. */
    if(Thread_Demo_Context.TWN_Handle != QAPI_TWN_INVALID_HANDLE)
    {
        if((Parameter_Count >= 1) &&
                (Verify_Integer_Parameter(&(Parameter_List[0]), 0, 0xFFFD)))
        {
            /* Read back the current network configuration. */
            Result = qapi_TWN_Get_Network_Configuration(Thread_Demo_Context.TWN_Handle, &Network_Config);
            if(Result == QAPI_OK)
            {
                /* Set up the pan ID feild in network configuration. */
                Network_Config.PAN_ID = (uint16_t)(Parameter_List[0].Integer_Value);

                /* Write back the network configuration. */
                Result = qapi_TWN_Set_Network_Configuration(Thread_Demo_Context.TWN_Handle, &Network_Config);
                if(Result == QAPI_OK)
                {
                    Display_Function_Success(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Set_Network_Configuration");

                    Ret_Val = QCLI_STATUS_SUCCESS_E;
                }
                else
                {
                    Display_Function_Error(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Set_Network_Configuration", Result);

                    Ret_Val = QCLI_STATUS_ERROR_E;
                }
            }
            else
            {
                Display_Function_Error(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Get_Network_Configuration", Result);

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
        QCLI_Printf(Thread_Demo_Context.QCLI_Handle, "TWN not initialized.\n");
        Ret_Val = QCLI_STATUS_ERROR_E;
    }

    return(Ret_Val);
}

/**
  @brief Set the Thread network extended PAN ID in the Thread Network
  configuration.

  Parameter_List[0] The 64-bit PAN ID of the network.

  @param Parameter_Count is number of elements in Parameter_List.
  @param Parameter_List  is the list of parsed arguments associated with
  this command.

  @return
  - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
  - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
  - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
  command.
 */
static QCLI_Command_Status_t cmd_SetExtendedPANID(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    QCLI_Command_Status_t            Ret_Val;
    qapi_TWN_Network_Configuration_t Network_Config;
    uint64_t                         Extended_PAN_ID;
    qapi_Status_t                    Result;

    /* Verify the TWN layer is initialized. */
    if(Thread_Demo_Context.TWN_Handle != QAPI_TWN_INVALID_HANDLE)
    {
        if((Parameter_Count >= 1) &&
                (Hex_String_To_ULL(Parameter_List[0].String_Value, &Extended_PAN_ID)))
        {
            /* Read back the current network configuration. */
            Result = qapi_TWN_Get_Network_Configuration(Thread_Demo_Context.TWN_Handle, &Network_Config);
            if(Result == QAPI_OK)
            {
                /* Set up the extended pan ID feild in network configuration. */
                Network_Config.Extended_PAN_ID = Extended_PAN_ID;

                /* Write back the network configuration. */
                Result = qapi_TWN_Set_Network_Configuration(Thread_Demo_Context.TWN_Handle, &Network_Config);
                if(Result == QAPI_OK)
                {
                    Display_Function_Success(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Set_Network_Configuration");

                    Ret_Val = QCLI_STATUS_SUCCESS_E;
                }
                else
                {
                    Display_Function_Error(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Set_Network_Configuration", Result);

                    Ret_Val = QCLI_STATUS_ERROR_E;
                }
            }
            else
            {
                Display_Function_Error(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Get_Network_Configuration", Result);

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
        QCLI_Printf(Thread_Demo_Context.QCLI_Handle, "TWN not initialized.\n");
        Ret_Val = QCLI_STATUS_ERROR_E;
    }

    return(Ret_Val);
}

/**
  @brief Set the Thread network name in the Thread Network configuration.

  Parameter_List[0] String representing the networks name (16 characters max).

  @param Parameter_Count is number of elements in Parameter_List.
  @param Parameter_List  is the list of parsed arguments associated with
  this command.

  @return
  - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
  - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
  - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
  command.
 */
static QCLI_Command_Status_t cmd_SetNetworkName(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    QCLI_Command_Status_t            Ret_Val;
    qapi_TWN_Network_Configuration_t Network_Config;
    qapi_Status_t                    Result;

    /* Verify the TWN layer is initialized. */
    if(Thread_Demo_Context.TWN_Handle != QAPI_TWN_INVALID_HANDLE)
    {
        if(Parameter_Count >= 1)
        {
            /* Read back the current network configuration. */
            Result = qapi_TWN_Get_Network_Configuration(Thread_Demo_Context.TWN_Handle, &Network_Config);
            if(Result == QAPI_OK)
            {
                /* Set up the network name feild in network configuration. */
                memset(Network_Config.NetworkName, 0, sizeof(Network_Config.NetworkName));
                strlcpy((char *)(Network_Config.NetworkName), Parameter_List[0].String_Value, sizeof(Network_Config.NetworkName));

                /* Write back the network configuration. */
                Result = qapi_TWN_Set_Network_Configuration(Thread_Demo_Context.TWN_Handle, &Network_Config);
                if(Result == QAPI_OK)
                {
                    Display_Function_Success(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Set_Network_Configuration");

                    Ret_Val = QCLI_STATUS_SUCCESS_E;
                }
                else
                {
                    Display_Function_Error(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Set_Network_Configuration", Result);

                    Ret_Val = QCLI_STATUS_ERROR_E;
                }
            }
            else
            {
                Display_Function_Error(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Get_Network_Configuration", Result);

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
        QCLI_Printf(Thread_Demo_Context.QCLI_Handle, "TWN not initialized.\n");
        Ret_Val = QCLI_STATUS_ERROR_E;
    }

    return(Ret_Val);
}

/**
  @brief Set the Thread network master key in the Thread Network configuration.

  Parameter_List[0] the 128-bit master key in hexadecimal.

  @param Parameter_Count is number of elements in Parameter_List.
  @param Parameter_List  is the list of parsed arguments associated with
  this command.

  @return
  - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
  - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
  - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
  command.
 */
static QCLI_Command_Status_t cmd_SetMasterKey(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    QCLI_Command_Status_t            Ret_Val;
    qapi_TWN_Network_Configuration_t Network_Config;
    qapi_Status_t                    Result;
    uint32_t                         Length;

    /* Verify the TWN layer is initialized. */
    if(Thread_Demo_Context.TWN_Handle != QAPI_TWN_INVALID_HANDLE)
    {
        if(Parameter_Count >= 1)
        {
            /* Read back the current network configuration. */
            Result = qapi_TWN_Get_Network_Configuration(Thread_Demo_Context.TWN_Handle, &Network_Config);
            if(Result == QAPI_OK)
            {
                /* Set up the masterkey feild in network configuration. */
                Length = QAPI_OPEN_THREAD_MASTER_KEY_SIZE;
                if(Hex_String_To_Array(Parameter_List[0].String_Value, &Length, Network_Config.MasterKey))
                {
                    /* Write back the network configuration. */
                    Result = qapi_TWN_Set_Network_Configuration(Thread_Demo_Context.TWN_Handle, &Network_Config);
                    if(Result == QAPI_OK)
                    {
                        Display_Function_Success(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Set_Network_Configuration");

                        Ret_Val = QCLI_STATUS_SUCCESS_E;
                    }
                    else
                    {
                        Display_Function_Error(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Set_Network_Configuration", Result);

                        Ret_Val = QCLI_STATUS_ERROR_E;
                    }
                }
                else
                {
                    /* The master key string is not valid. */
                    Ret_Val = QCLI_STATUS_USAGE_E;
                }
            }
            else
            {
                Display_Function_Error(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Get_Network_Configuration", Result);

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
        QCLI_Printf(Thread_Demo_Context.QCLI_Handle, "TWN not initialized.\n");
        Ret_Val = QCLI_STATUS_ERROR_E;
    }

    return(Ret_Val);
}

/**
  @brief Stop the Commissioner role on this device.

  @param: Parameter_Count is number of elements in Parameter_List.
  @param: Parameter_List  is the list of parsed arguments associated with
  this command.

  @return
  - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
  - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
  - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
  command.
 */
static QCLI_Command_Status_t cmd_MeshCoP_CommissionerStop(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    QCLI_Command_Status_t            Ret_Val;
    qapi_Status_t                    Result;

    /* Verify the TWN layer is initialized. */
    if(Thread_Demo_Context.TWN_Handle != QAPI_TWN_INVALID_HANDLE)
    {
        Result = qapi_TWN_Commissioner_Stop(Thread_Demo_Context.TWN_Handle);
        if(Result == QAPI_OK)
        {
            Ret_Val = QCLI_STATUS_SUCCESS_E;
            Display_Function_Success(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Commissioner_Stop");
        }
        else
        {
            Result = QAPI_ERROR;
            Ret_Val = QCLI_STATUS_ERROR_E;
            Display_Function_Error(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Commissioner_Stop", Result);
        }
    }
    else
    {
        QCLI_Printf(Thread_Demo_Context.QCLI_Handle, "TWN not initialized.\n");
        Ret_Val = QCLI_STATUS_ERROR_E;
    }

    return Ret_Val;
}


/**
  @brief Delete joining device information from the Steering Information.

  Parameter_List[0] the EUI-64 or * previously given to AddJoiner.

  @param Parameter_Count is number of elements in Parameter_List.
  @param Parameter_List  is the list of parsed arguments associated with
  this command.

  @return
  - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
  - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
  - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
  command.
 */
static QCLI_Command_Status_t cmd_MeshCoP_CommissionerDelJoiner(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    QCLI_Command_Status_t Ret_Val;
    qapi_Status_t         Result;
    uint64_t              EUI64;

    /* Verify the TWN layer is initialized. */
    if(Thread_Demo_Context.TWN_Handle != QAPI_TWN_INVALID_HANDLE)
    {
        Ret_Val = QCLI_STATUS_SUCCESS_E;

        if(Parameter_Count >= 1)
        {
            if(!strcmp(Parameter_List[0].String_Value, "*"))
            {
                EUI64 = 0ULL;
            }
            else if(!Hex_String_To_ULL(Parameter_List[0].String_Value, &EUI64))
            {
                QCLI_Printf(Thread_Demo_Context.QCLI_Handle, "Error parsing EUI-64.\n");
                Ret_Val = QCLI_STATUS_USAGE_E;
            }
        }
        else
        {
            EUI64 = 0ULL;
        }

        if(Ret_Val == QCLI_STATUS_SUCCESS_E)
        {
            Result = qapi_TWN_Commissioner_Remove_Joiner(Thread_Demo_Context.TWN_Handle, EUI64);
            if(Result == QAPI_OK)
            {
                Ret_Val = QCLI_STATUS_SUCCESS_E;
                Display_Function_Success(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Commissioner_Remove_Joiner");
            }
            else
            {
                Ret_Val = QCLI_STATUS_ERROR_E;
                Display_Function_Error(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Commissioner_Remove_Joiner", Result);
            }
        }
    }
    else
    {
        QCLI_Printf(Thread_Demo_Context.QCLI_Handle, "TWN not initialized.\n");
        Ret_Val = QCLI_STATUS_ERROR_E;
    }

    return Ret_Val;
}

/**
  @brief Stops an active attempt to commission onto a Thread network.

  @param Parameter_Count is number of elements in Parameter_List.
  @param Parameter_List  is the list of parsed arguments associated with
  this command.

  @return
  - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
  - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
  - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
  command.
 */
static QCLI_Command_Status_t cmd_MeshCoP_JoinerStop(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    QCLI_Command_Status_t            Ret_Val;
    qapi_Status_t                    Result;

    /* Verify the TWN layer is initialized. */
    if(Thread_Demo_Context.TWN_Handle != QAPI_TWN_INVALID_HANDLE)
    {
        Result = qapi_TWN_Joiner_Stop(Thread_Demo_Context.TWN_Handle);
        if(Result == QAPI_OK)
        {
            Ret_Val = QCLI_STATUS_SUCCESS_E;
            Display_Function_Success(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Joiner_Stop");
        }
        else
        {
            Result = QAPI_ERROR;
            Ret_Val = QCLI_STATUS_ERROR_E;
            Display_Function_Error(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Joiner_Stop", Result);
        }
    }
    else
    {
        QCLI_Printf(Thread_Demo_Context.QCLI_Handle, "TWN not initialized.\n");
        Ret_Val = QCLI_STATUS_ERROR_E;
    }

    return Ret_Val;
}


/**
  @brief Remove a border router's information from the network data.

  Parameter_List[0] IPv6 prefix of the border router.
  Parameter_List[1] The CIDR bit length of the prefix.

  @param Parameter_Count is number of elements in Parameter_List.
  @param Parameter_List  is the list of parsed arguments associated with
  this command.

  @return
  - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
  - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
  - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
  command.
 */
static QCLI_Command_Status_t cmd_RemoveBorderRouter(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    QCLI_Command_Status_t  Ret_Val;
    qapi_TWN_IPv6_Prefix_t Prefix_Config;
    qapi_Status_t          Result;

    /* Verify the TWN layer is initialized. */
    if(Thread_Demo_Context.TWN_Handle != QAPI_TWN_INVALID_HANDLE)
    {
        if((Parameter_Count >= 2) &&
                (Verify_Integer_Parameter(&(Parameter_List[1]), 0, 128)))
        {
            /* Parse the IPv6 address. */
            if(inet_pton(AF_INET6, Parameter_List[0].String_Value, &(Prefix_Config.Address)) == 0)
            {
                /* Set up the remaining fields of Prefix_Config. */
                Prefix_Config.Length = (uint8_t)(Parameter_List[1].Integer_Value);

                Result = qapi_TWN_Remove_Border_Router(Thread_Demo_Context.TWN_Handle, &Prefix_Config);
                if(Result == QAPI_OK)
                {
                    Display_Function_Success(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Remove_Border_Router");

                    Ret_Val = QCLI_STATUS_SUCCESS_E;
                }
                else
                {
                    Display_Function_Error(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Remove_Border_Router", Result);

                    Ret_Val = QCLI_STATUS_ERROR_E;
                }
            }
            else
            {
                /* The IPv6 Address is not valid. */
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
        QCLI_Printf(Thread_Demo_Context.QCLI_Handle, "TWN not initialized.\n");
        Ret_Val = QCLI_STATUS_ERROR_E;
    }

    return(Ret_Val);
}

/**
  @brief Add an external route to the network data.

  Parameter_List[0] IPv6 prefix of the route.
  Parameter_List[1] The CIDR bit length of the prefix.
  Parameter_List[2] (0/1) Is_Stable to determine if this route is part of
  stable network data.

  @param Parameter_Count is number of elements in Parameter_List.
  @param Parameter_List  is the list of parsed arguments associated with
  this command.

  @return
  - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
  - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
  - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
  command.
 */
static QCLI_Command_Status_t cmd_AddExternalRoute(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    QCLI_Command_Status_t     Ret_Val;
    qapi_TWN_External_Route_t Route_Config;
    qapi_Status_t             Result;

    /* Verify the TWN layer is initialized. */
    if(Thread_Demo_Context.TWN_Handle != QAPI_TWN_INVALID_HANDLE)
    {
        if((Parameter_Count >= 4) &&
                (Verify_Integer_Parameter(&(Parameter_List[1]), 0, 128)) &&
                (Verify_Integer_Parameter(&(Parameter_List[2]), 0, 2)) &&
                (Verify_Integer_Parameter(&(Parameter_List[3]), 0, 1)))
        {
            /* Parse the IPv6 address. */
            if(inet_pton(AF_INET6, Parameter_List[0].String_Value, &(Route_Config.Prefix.Address)) == 0)
            {
                /* Set up the remaining fields of Route_Config. */
                Route_Config.Prefix.Length = (uint8_t)(Parameter_List[1].Integer_Value);
                Route_Config.Preference    = (qapi_TWN_Routing_Preference_t)(Parameter_List[2].Integer_Value);
                Route_Config.Is_Stable     = (qbool_t)(Parameter_List[3].Integer_Value != 0);

                Result = qapi_TWN_Add_External_Route(Thread_Demo_Context.TWN_Handle, &Route_Config);
                if(Result == QAPI_OK)
                {
                    Display_Function_Success(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Add_External_Route");

                    Ret_Val = QCLI_STATUS_SUCCESS_E;
                }
                else
                {
                    Display_Function_Error(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Add_External_Route", Result);

                    Ret_Val = QCLI_STATUS_ERROR_E;
                }
            }
            else
            {
                /* The IPv6 Address is not valid. */
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
        QCLI_Printf(Thread_Demo_Context.QCLI_Handle, "TWN not initialized.\n");
        Ret_Val = QCLI_STATUS_ERROR_E;
    }

    return(Ret_Val);
}

/**
  @brief Remove an external route from the network data.

  Parameter_List[0] IPv6 prefix of the route.
  Parameter_List[1] The CIDR bit length of the prefix.

  @param Parameter_Count is number of elements in Parameter_List.
  @param Parameter_List  is the list of parsed arguments associated with
  this command.

  @return
  - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
  - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
  - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
  command.
 */
static QCLI_Command_Status_t cmd_RemoveExternalRoute(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    QCLI_Command_Status_t  Ret_Val;
    qapi_TWN_IPv6_Prefix_t Prefix_Config;
    qapi_Status_t          Result;

    /* Verify the TWN layer is initialized. */
    if(Thread_Demo_Context.TWN_Handle != QAPI_TWN_INVALID_HANDLE)
    {
        if((Parameter_Count >= 2) &&
                (Verify_Integer_Parameter(&(Parameter_List[1]), 0, 128)))
        {
            /* Parse the IPv6 address. */
            if(inet_pton(AF_INET6, Parameter_List[0].String_Value, &(Prefix_Config.Address)) == 0)
            {
                /* Set up the remaining fields of Prefix_Config. */
                Prefix_Config.Length = (uint8_t)(Parameter_List[1].Integer_Value);

                Result = qapi_TWN_Remove_External_Route(Thread_Demo_Context.TWN_Handle, &Prefix_Config);
                if(Result == QAPI_OK)
                {
                    Display_Function_Success(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Remove_External_Route");

                    Ret_Val = QCLI_STATUS_SUCCESS_E;
                }
                else
                {
                    Display_Function_Error(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Remove_External_Route", Result);

                    Ret_Val = QCLI_STATUS_ERROR_E;
                }
            }
            else
            {
                /* The IPv6 Address is not valid. */
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
        QCLI_Printf(Thread_Demo_Context.QCLI_Handle, "TWN not initialized.\n");
        Ret_Val = QCLI_STATUS_ERROR_E;
    }

    return(Ret_Val);
}




/**
  @brief Stops acting as a border agent on a WLAN interface.
  Available only on the QCA4020 with active WLAN connection.

  @param Parameter_Count is number of elements in Parameter_List.
  @param Parameter_List  is the list of parsed arguments associated with
  this command.

  @return
  - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
  - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
  - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
  command.
 */
static QCLI_Command_Status_t cmd_Thread_StopBorderAgent(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    QCLI_Command_Status_t            Ret_Val;
    qapi_Status_t                    Result;

    /* Verify the TWN layer is initialized. */
    if(Thread_Demo_Context.TWN_Handle != QAPI_TWN_INVALID_HANDLE)
    {
        Result = qapi_TWN_Stop_Border_Agent(Thread_Demo_Context.TWN_Handle);

        if(Result == QAPI_OK)
        {
            Display_Function_Success(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Stop_Border_Agent");
            Ret_Val = QCLI_STATUS_SUCCESS_E;
        }
        else
        {
            Display_Function_Error(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Stop_Border_Agent", Result);

            Ret_Val = QCLI_STATUS_ERROR_E;
        }
    }
    else
    {
        QCLI_Printf(Thread_Demo_Context.QCLI_Handle, "TWN not initialized.\n");
        Ret_Val = QCLI_STATUS_ERROR_E;
    }

    return(Ret_Val);
}

/**
  @brief Generates a new PSKc based on current Network Info and passphrase.

  Parameter_List[0] The passphrase with which to generate the new PSKc.

  @param Parameter_Count is number of elements in Parameter_List.
  @param Parameter_List  is the list of parsed arguments associated with
  this command.

  @return
  - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
  - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
  - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
  command.
 */
static QCLI_Command_Status_t cmd_Thread_UpdatePSKc(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    QCLI_Command_Status_t            Ret_Val;
    qapi_Status_t                    Result;
    qapi_TWN_Network_Configuration_t Network_Config;
    uint8_t                          PSKc[QAPI_TWN_PSKC_SIZE];

    /* Verify the TWN layer is initialized. */
    if(Thread_Demo_Context.TWN_Handle != QAPI_TWN_INVALID_HANDLE)
    {
        if(Parameter_Count >= 1)
        {
            Ret_Val = QCLI_STATUS_SUCCESS_E;

            Result = qapi_TWN_Get_Network_Configuration(Thread_Demo_Context.TWN_Handle, &Network_Config);
            if(Result == QAPI_OK)
            {
                /* Print out something because this call takes a while. */
                QCLI_Printf(Thread_Demo_Context.QCLI_Handle, "Generating PSKc...\n");
                Result = qapi_TWN_Commissioner_Generate_PSKc(Thread_Demo_Context.TWN_Handle, Parameter_List[0].String_Value, Network_Config.NetworkName, Network_Config.Extended_PAN_ID, PSKc);
                if(Result == QAPI_OK)
                {
                    QCLI_Printf(Thread_Demo_Context.QCLI_Handle, "PSKc generated!\n");
                }
                else
                {
                    Display_Function_Error(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Commissioner_Generate_PSKc", Result);
                    Ret_Val = QCLI_STATUS_ERROR_E;
                }
            }
            else
            {
                Display_Function_Error(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Get_Network_Configuration", Result);
                Ret_Val = QCLI_STATUS_ERROR_E;
            }

            /* Only bother setting the PSKc if we successfully generated it. */
            if(Ret_Val == QCLI_STATUS_SUCCESS_E)
            {
                Result = qapi_TWN_Set_PSKc(Thread_Demo_Context.TWN_Handle, PSKc);
                if(Result == QAPI_OK)
                {
                    Display_Function_Success(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Set_PSKc");
                }
                else
                {
                    Display_Function_Error(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Set_PSKc", Result);
                    Ret_Val = QCLI_STATUS_ERROR_E;
                }
            }
        }
        else
        {
            Ret_Val = QCLI_STATUS_USAGE_E;
        }
    }
    else
    {
        QCLI_Printf(Thread_Demo_Context.QCLI_Handle, "TWN not initialized.\n");
        Ret_Val = QCLI_STATUS_ERROR_E;
    }

    return(Ret_Val);
}

/**
  @brief Clears any persistent Thread information stored in Flash.

  @param Parameter_Count is number of elements in Parameter_List.
  @param Parameter_List  is the list of parsed arguments associated with
  this command.

  @return
  - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
  - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
  - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
  command.
 */
static QCLI_Command_Status_t cmd_Thread_ClearPersist(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    QCLI_Command_Status_t            Ret_Val;
    qapi_Status_t                    Result;

    /* Verify the TWN layer is initialized. */
    if(Thread_Demo_Context.TWN_Handle != QAPI_TWN_INVALID_HANDLE)
    {
        Result = qapi_TWN_Clear_Persistent_Data(Thread_Demo_Context.TWN_Handle);

        if(Result == QAPI_OK)
        {
            Display_Function_Success(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Clear_Persistent_Data");

            DisplayNetworkInfo();

            Ret_Val = QCLI_STATUS_SUCCESS_E;
        }
        else
        {
            Display_Function_Error(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Clear_Persistent_Data", Result);

            Ret_Val = QCLI_STATUS_ERROR_E;
        }
    }
    else
    {
        QCLI_Printf(Thread_Demo_Context.QCLI_Handle, "TWN not initialized.\n");
        Ret_Val = QCLI_STATUS_ERROR_E;
    }

    return(Ret_Val);
}

/**
  @brief Attempts to upgrade from a REED to a Router.

  @param Parameter_Count is number of elements in Parameter_List.
  @param Parameter_List  is the list of parsed arguments associated with
  this command.

  @return
  - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
  - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
  - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
  command.
 */
static QCLI_Command_Status_t cmd_Thread_BecomeRouter(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    QCLI_Command_Status_t            Ret_Val;
    qapi_Status_t                    Result;

    /* Verify the TWN layer is initialized. */
    if(Thread_Demo_Context.TWN_Handle != QAPI_TWN_INVALID_HANDLE)
    {
        Result = qapi_TWN_Become_Router(Thread_Demo_Context.TWN_Handle);

        if(Result == QAPI_OK)
        {
            Display_Function_Success(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Become_Router");

            Ret_Val = QCLI_STATUS_SUCCESS_E;
        }
        else
        {
            Display_Function_Error(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Become_Router", Result);

            Ret_Val = QCLI_STATUS_ERROR_E;
        }
    }
    else
    {
        QCLI_Printf(Thread_Demo_Context.QCLI_Handle, "TWN not initialized.\n");
        Ret_Val = QCLI_STATUS_ERROR_E;
    }

    return(Ret_Val);
}

/**
  @brief Attempts to become the Leader by forcing generation of a new Thread
  Network Partition.

  @param Parameter_Count is number of elements in Parameter_List.
  @param Parameter_List  is the list of parsed arguments associated with
  this command.

  @return
  - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
  - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
  - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
  command.
 */
static QCLI_Command_Status_t cmd_Thread_BecomeLeader(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    QCLI_Command_Status_t            Ret_Val;
    qapi_Status_t                    Result;

    /* Verify the TWN layer is initialized. */
    if(Thread_Demo_Context.TWN_Handle != QAPI_TWN_INVALID_HANDLE)
    {
        Result = qapi_TWN_Become_Leader(Thread_Demo_Context.TWN_Handle);

        if(Result == QAPI_OK)
        {
            Display_Function_Success(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Become_Leader");

            Ret_Val = QCLI_STATUS_SUCCESS_E;
        }
        else
        {
            Display_Function_Error(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Become_Leader", Result);

            Ret_Val = QCLI_STATUS_ERROR_E;
        }
    }
    else
    {
        QCLI_Printf(Thread_Demo_Context.QCLI_Handle, "TWN not initialized.\n");
        Ret_Val = QCLI_STATUS_ERROR_E;
    }

    return(Ret_Val);
}



uint32_t Thread_Get_Interfaces()
{
    int i, num, e;
    qapi_Net_Ifnameindex_t *p, *pp;
    char * interface_name;
    uint32_t addr, mask, gw, *gw_addr = NULL;
    qbool_t if_is_up;

    LOG_INFO("In Thread_Get_Interfaces\n");

    num = qapi_Net_Get_All_Ifnames(NULL);
    if (num == 0)
    {
        /* If no interface, we want to display cmd syntax */
        LOG_ERROR("No interfaces\n");
        return QCLI_STATUS_USAGE_E;
    }

    p = pp = malloc(num * sizeof(qapi_Net_Ifnameindex_t));
    if (p)
    {
        qapi_Net_Get_All_Ifnames(p);
        for (i = 0; i < num; ++i)
        {
            display_interface_info(pp->interface_Name, pp->if_Is_Up);
            ++pp;
        }

        free(p);
    }
    else
    {
        return QCLI_STATUS_USAGE_E;
        LOG_ERROR("ERROR: usage error\n");
    }

}

/**
  @brief Enables the use of QAPI socket functions with Thread.

  Parameter_List[0] Enables or disables the integrated IP stack (0/1).

  @param Parameter_Count is number of elements in Parameter_List.
  @param Parameter_List  is the list of parsed arguments associated with
  this command.

  @return
  - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
  - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
  - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
  command.
 */

uint32_t Thread_SetIPStackInteg(uint32_t IP_Stack_Enable)
{
    QCLI_Command_Status_t            Ret_Val;
    qapi_Status_t                    Result;

    /* Verify the TWN layer is initialized. */
    if(Thread_Demo_Context.TWN_Handle != QAPI_TWN_INVALID_HANDLE)
    {
        Result = qapi_TWN_Set_IP_Stack_Integration(Thread_Demo_Context.TWN_Handle, IP_Stack_Enable);

        if(Result == QAPI_OK)
        {
            Display_Function_Success(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Set_IP_Stack_Integration");
            LOG_INFO("SUCCESS: qapi_TWN_Set_IP_Stack_Integration");

            Ret_Val = QCLI_STATUS_SUCCESS_E;
        }
        else
        {
            Display_Function_Error(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Set_IP_Stack_Integration", Result);
            LOG_ERROR("FAILURE: qapi_TWN_Set_IP_Stack_Integration");

            Ret_Val = QCLI_STATUS_ERROR_E;
        }
    }
    else
    {
        QCLI_Printf(Thread_Demo_Context.QCLI_Handle, "TWN not initialized.\n");
        LOG_ERROR("FAILURE: TWN not initialized.\n");
        Ret_Val = QCLI_STATUS_ERROR_E;
    }

    return(Ret_Val);
}



/**
  @brief Removes a static IP from the Thread Network interface.

  Parameter_List[0] The IPv6 address to remove from the interface.

  @param Parameter_Count is number of elements in Parameter_List.
  @param Parameter_List  is the list of parsed arguments associated with
  this command.

  @return
  - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
  - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
  - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
  command.
 */
static QCLI_Command_Status_t cmd_Thread_RemoveUnicastAddress(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    QCLI_Command_Status_t     Ret_Val;
    qapi_TWN_IPv6_Address_t   Address;
    qapi_Status_t             Result;

    /* Verify the TWN layer is initialized. */
    if(Thread_Demo_Context.TWN_Handle != QAPI_TWN_INVALID_HANDLE)
    {
        if(Parameter_Count >= 1)
        {
            /* Parse the IPv6 address. */
            if(inet_pton(AF_INET6, Parameter_List[0].String_Value, &Address) == 0)
            {
                Result = qapi_TWN_IPv6_Remove_Unicast_Address(Thread_Demo_Context.TWN_Handle, &Address);
                if(Result == QAPI_OK)
                {
                    Display_Function_Success(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_IPv6_Remove_Unicast_Address");

                    Ret_Val = QCLI_STATUS_SUCCESS_E;
                }
                else
                {
                    Display_Function_Error(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_IPv6_Remove_Unicast_Address", Result);

                    Ret_Val = QCLI_STATUS_ERROR_E;
                }
            }
            else
            {
                /* The IPv6 Address is not valid. */
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
        QCLI_Printf(Thread_Demo_Context.QCLI_Handle, "TWN not initialized.\n");
        Ret_Val = QCLI_STATUS_ERROR_E;
    }

    return(Ret_Val);
}

/**
  @brief Subscribes to a multicast address on the Thread Network interface.

  Parameter_List[0] The multicast IPv6 address to subscribe.

  @param Parameter_Count is number of elements in Parameter_List.
  @param Parameter_List  is the list of parsed arguments associated with
  this command.

  @return
  - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
  - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
  - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
  command.
 */
static QCLI_Command_Status_t cmd_Thread_SubscribeMulticast(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    QCLI_Command_Status_t     Ret_Val;
    qapi_TWN_IPv6_Address_t   Address;
    qapi_Status_t             Result;

    /* Verify the TWN layer is initialized. */
    if(Thread_Demo_Context.TWN_Handle != QAPI_TWN_INVALID_HANDLE)
    {
        if(Parameter_Count >= 1)
        {
            /* Parse the IPv6 address. */
            if(inet_pton(AF_INET6, Parameter_List[0].String_Value, &Address) == 0)
            {
                Result = qapi_TWN_IPv6_Subscribe_Multicast_Address(Thread_Demo_Context.TWN_Handle, &Address);
                if(Result == QAPI_OK)
                {
                    Display_Function_Success(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_IPv6_Subscribe_Multicast_Address");

                    Ret_Val = QCLI_STATUS_SUCCESS_E;
                }
                else
                {
                    Display_Function_Error(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_IPv6_Subscribe_Multicast_Address", Result);

                    Ret_Val = QCLI_STATUS_ERROR_E;
                }
            }
            else
            {
                /* The IPv6 Address is not valid. */
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
        QCLI_Printf(Thread_Demo_Context.QCLI_Handle, "TWN not initialized.\n");
        Ret_Val = QCLI_STATUS_ERROR_E;
    }

    return(Ret_Val);
}

/**
  @brief Unsubscribes from a multicast address on the Thread Network interface.

  Parameter_List[0] The multicast IPv6 address to unsubscribe.

  @param Parameter_Count is number of elements in Parameter_List.
  @param Parameter_List  is the list of parsed arguments associated with
  this command.

  @return
  - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
  - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
  - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
  command.
 */
static QCLI_Command_Status_t cmd_Thread_UnsubscribeMulticast(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    QCLI_Command_Status_t     Ret_Val;
    qapi_TWN_IPv6_Address_t   Address;
    qapi_Status_t             Result;

    /* Verify the TWN layer is initialized. */
    if(Thread_Demo_Context.TWN_Handle != QAPI_TWN_INVALID_HANDLE)
    {
        if(Parameter_Count >= 1)
        {
            /* Parse the IPv6 address. */
            if(inet_pton(AF_INET6, Parameter_List[0].String_Value, &Address) == 0)
            {
                Result = qapi_TWN_IPv6_Unsubscribe_Multicast_Address(Thread_Demo_Context.TWN_Handle, &Address);
                if(Result == QAPI_OK)
                {
                    Display_Function_Success(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_IPv6_Unsubscribe_Multicast_Address");

                    Ret_Val = QCLI_STATUS_SUCCESS_E;
                }
                else
                {
                    Display_Function_Error(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_IPv6_Unsubscribe_Multicast_Address", Result);

                    Ret_Val = QCLI_STATUS_ERROR_E;
                }
            }
            else
            {
                /* The IPv6 Address is not valid. */
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
        QCLI_Printf(Thread_Demo_Context.QCLI_Handle, "TWN not initialized.\n");
        Ret_Val = QCLI_STATUS_ERROR_E;
    }

    return(Ret_Val);
}

/**
  @brief Sets whether the Thread Interface will respond to ping requests
  received on its interface.

  Parameter_List[0] Enables or disables ping responses (0/1).

  @param Parameter_Count is number of elements in Parameter_List.
  @param Parameter_List  is the list of parsed arguments associated with
  this command.

  @return
  - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
  - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
  - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
  command.
 */
static QCLI_Command_Status_t cmd_Thread_SetPingEnabled(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    QCLI_Command_Status_t            Ret_Val;
    qapi_Status_t                    Result;

    /* Verify the TWN layer is initialized. */
    if(Thread_Demo_Context.TWN_Handle != QAPI_TWN_INVALID_HANDLE)
    {
        if(Parameter_Count >= 1 && Verify_Integer_Parameter(&Parameter_List[0], 0, 1))
        {
            Result = qapi_TWN_Set_Ping_Response_Enabled(Thread_Demo_Context.TWN_Handle, Parameter_List[0].Integer_Value);

            if(Result == QAPI_OK)
            {
                Display_Function_Success(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Set_Ping_Response_Enabled");

                Ret_Val = QCLI_STATUS_SUCCESS_E;
            }
            else
            {
                Display_Function_Error(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Set_Ping_Response_Enabled", Result);

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
        QCLI_Printf(Thread_Demo_Context.QCLI_Handle, "TWN not initialized.\n");
        Ret_Val = QCLI_STATUS_ERROR_E;
    }

    return(Ret_Val);
}

/**
  @brief Sets the Provisioning URL used by the Commissioner.

  Parameter_List[0] The Provisioning URL to use.

  @param Parameter_Count is number of elements in Parameter_List.
  @param Parameter_List  is the list of parsed arguments associated with
  this command.

  @return
  - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
  - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
  - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
  command.
 */
static QCLI_Command_Status_t cmd_Thread_SetProvisioningUrl(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    QCLI_Command_Status_t            Ret_Val;
    qapi_Status_t                    Result;

    /* Verify the TWN layer is initialized. */
    if(Thread_Demo_Context.TWN_Handle != QAPI_TWN_INVALID_HANDLE)
    {
        if(Parameter_Count >= 1)
        {
            Result = qapi_TWN_Commissioner_Set_Provisioning_URL(Thread_Demo_Context.TWN_Handle, Parameter_List[0].String_Value);

            if(Result == QAPI_OK)
            {
                Display_Function_Success(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Commissioner_Set_Provisioning_URL");

                Ret_Val = QCLI_STATUS_SUCCESS_E;
            }
            else
            {
                Display_Function_Error(Thread_Demo_Context.QCLI_Handle, "qapi_TWN_Commissioner_Set_Provisioning_URL", Result);

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
        QCLI_Printf(Thread_Demo_Context.QCLI_Handle, "TWN not initialized.\n");
        Ret_Val = QCLI_STATUS_ERROR_E;
    }

    return(Ret_Val);
}

#ifdef ALLOW_OPENTHREAD_DEBUG_LOGS

/**
  @brief Enables or disables OpenThread debug messages.

  Parameter_List[0] Enables or disables the debug logging (0/1).

  @param Parameter_Count is number of elements in Parameter_List.
  @param Parameter_List  is the list of parsed arguments associated with
  this command.

  @return
  - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
  - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
  - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
  command.
 */
static QCLI_Command_Status_t cmd_Thread_SetLogging(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    QCLI_Command_Status_t Ret_Val;

    if(Parameter_Count >= 1 && Verify_Integer_Parameter(&Parameter_List[0], 0, 1))
    {
        Thread_Demo_Context.EnableLogging = (qbool_t)Parameter_List[0].Integer_Value;
        QCLI_Printf(Thread_Demo_Context.QCLI_Handle, "Thread logging %s.\n", Thread_Demo_Context.EnableLogging ? "enabled" : "disabled");
        Ret_Val = QCLI_STATUS_SUCCESS_E;
    }
    else
    {
        Ret_Val = QCLI_STATUS_USAGE_E;
    }

    return(Ret_Val);
}

static char otLogBuf[112];

/**
  @brief Callback function called directly from OpenThread when issuing log
  messages to the console. Optional to include in a user application.
 */
void otPlatLog(int LogLevel, int LogRegion, const char *Format, ...)
{
    va_list args;

    if(Thread_Demo_Context.EnableLogging)
    {
        va_start(args, Format);
        vsnprintf(otLogBuf, sizeof(otLogBuf), Format, args);
        va_end(args);

        QCLI_Printf(Thread_Demo_Context.QCLI_Handle, "%s\n", otLogBuf);
    }
}

#endif
#endif
