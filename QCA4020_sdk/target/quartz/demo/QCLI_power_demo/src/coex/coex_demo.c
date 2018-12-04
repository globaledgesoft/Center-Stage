/*
 * Copyright (c) 2011-2018 Qualcomm Technologies, Inc.
 * 2011-2016 Qualcomm Atheros, Inc.
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

 
#include "string.h"
#include "stringl.h"
#include "malloc.h"

#include "qcli_api.h"
#include "qcli_util.h"
#include "pal.h"

#include "qapi_coex.h"
#include "qapi_tlmm.h"
#include "qapi_om_smem.h"

#if !defined(V1) && !defined(V2)
#error Either V1 or V2 must be defined.
#endif

/* Defines the number of priority values to set. */
#ifdef V1
#define CONFIG_PRIORITY_LENGTH                  (14)
#else
#define CONFIG_PRIORITY_LENGTH                  (15)
#endif

/* Defines coex statistics parameters. */
#define COEX_STATISTICS_MASK                    (0xFFFFFFFF)
#define COEX_STATISTICS_LENGTH                  32

/* This structure represents the contextual information for the coex demo
   application. */
typedef struct Coex_Demo_Context_s
{
   QCLI_Group_Handle_t         QCLI_Handle;
   qapi_COEX_Priority_Config_t Priorities[CONFIG_PRIORITY_LENGTH];
} Coex_Demo_Context_t;

typedef struct Coex_Demo_Transition_Data_s
{
   qapi_COEX_Priority_Config_t Priorities[CONFIG_PRIORITY_LENGTH];
} Coex_Demo_Transition_Data_t;

#define COEX_DEMO_TRANSITION_DATA_SIZE          (sizeof(Coex_Demo_Transition_Data_t))

static Coex_Demo_Context_t  Coex_Demo_Context;

   /* Define the GPIO for Coex. */
static qapi_TLMM_Config_t BT_Active_Config;
static qapi_TLMM_Config_t BT_Pri_0_Config;
static qapi_TLMM_Config_t BT_Pri_1_Config;
static qapi_TLMM_Config_t WLAN_Active_Config;
static qapi_TLMM_Config_t EPTA_Pin_1_Config;
static qapi_TLMM_Config_t EPTA_Pin_2_Config;
static qapi_TLMM_Config_t EPTA_Pin_3_Config;
static qapi_GPIO_ID_t     BT_Active_ID;
static qapi_GPIO_ID_t     BT_Pri_0_ID;
static qapi_GPIO_ID_t     BT_Pri_1_ID;
static qapi_GPIO_ID_t     WLAN_Active_ID;
static boolean            WLAN_Initialized;
static qapi_GPIO_ID_t     EPTA_Pin_1_ID;
static qapi_GPIO_ID_t     EPTA_Pin_2_ID;
static qapi_GPIO_ID_t     EPTA_Pin_3_ID;
static boolean            EPTA_Initialized;
static boolean            WLAN_Initialized;

static QCLI_Command_Status_t cmd_Configure(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t cmd_ConfigureAdvanced(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t cmd_SetBLEPriority(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t cmd_SetI15P4Priority(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t cmd_SetEXTPriority(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t cmd_StatisticsEnable(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t cmd_StatisticsGet(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t cmd_WLANIFEnable(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t cmd_EPTAIFEnable(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);

/* The following is the complete command list for the coexistence demo. */
const QCLI_Command_t Coex_CMD_List[] =
{
    // cmd_function                 thread cmd_string                  usage_string                                                                                                                                                                                                                                                                                                                description
    {cmd_Configure,                 false, "Configure",                "[Config Flags] [Priority 1] [Priority 2] [Priority 3] [Priority 4] ",                                                                                                                                                                                                                                                      "Configures coexistence."},
    {cmd_ConfigureAdvanced,         false, "ConfigureAdvanced",        "[Config Flags] [Concurrency Flags] ",                                                                                                                                                                                                                                                                                      "Configures coexistence, advanced."},
    {cmd_SetBLEPriority,            false, "SetBLEPriority",           "[Adv] [Scan] [DataRequest] [DataActive]",                                                                                                                                                                                                                                                                                  "Sets BLE priorities for advanced configuration."},
#ifdef V1
    {cmd_SetI15P4Priority,          false, "SetI15P4Priority",         "[RxRequest] [TxRequest] [RxActive] [TxActive] [Ack] [EDscan]",                                                                                                                                                                                                                                                          "Sets 802.15.4 priorities for advanced configuration."},
#else
    {cmd_SetI15P4Priority,          false, "SetI15P4Priority",         "[RxRequest] [TxRequest] [RxActive] [TxActive] [Ack] [EDScan] [HoldRequest]",                                                                                                                                                                                                                                            "Sets 802.15.4 priorities for advanced configuration."},
#endif
   {cmd_SetEXTPriority,            false, "SetEXTPriority",           "[LowRequest] [HighRequest] [LowActive] [HighActive]",                                                                                                                                                                                                                                                                       "Sets EXT priorities for advanced configuration."},
    {cmd_StatisticsEnable,          false, "StatisticsEnable",         "[Enable (0=Disable, 1=Enable)]",                                                                                                                                                                                                                                                                                           "Enables coexistence statistics."},
    {cmd_StatisticsGet,             false, "StatisticsGet",            "[Reset (0=FALSE, 1 = TRUE)]",                                                                                                                                                                                                                                                                                              "Retrieves coexistence statistics."},
    {cmd_WLANIFEnable,              false, "WLANIFEnable",        "[Mode (0=Disable, 1=Enable)]",                                                                                                                        "Enables the WLAN interface."},
    {cmd_EPTAIFEnable,              false, "EPTAIFEnable",        "[Mode (0=Disable, 1=Slave (External WiFi), 2=Master (External Bluetooth))]",                                                                          "Enables the EPTA interface."},
};

const QCLI_Command_Group_t Coex_CMD_Group = {"Coex", sizeof(Coex_CMD_List) / sizeof(QCLI_Command_t), Coex_CMD_List};

/**
   @brief Executes the "Configure" command to configure the coexistence module.

   Parameter_List[0] Configuration flags.
   Parameter_List[1] Priority 1 value - 1 = BLE, 2 = I15P4, 3 = EXT, 4 = WLAN.
   Parameter_List[2] Priority 2 value - 1 = BLE, 2 = I15P4, 3 = EXT, 4 = WLAN.
   Parameter_List[3] Priority 3 value - 1 = BLE, 2 = I15P4, 3 = EXT, 4 = WLAN.
   Parameter_List[4] Priority 4 value - 1 = BLE, 2 = I15P4, 3 = EXT, 4 = WLAN.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
static QCLI_Command_Status_t cmd_Configure(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   qapi_Status_t           Result;
   QCLI_Command_Status_t   RetVal;
   qapi_COEX_Config_Data_t ConfigData;

   /* Verify the input parameters. */
   if(Parameter_Count >= 5)
   {
      /* Set the configuration data. */
      ConfigData.config_Flags = Parameter_List[0].Integer_Value;
      ConfigData.priority_1   = Parameter_List[1].Integer_Value;
      ConfigData.priority_2   = Parameter_List[2].Integer_Value;
      ConfigData.priority_3   = Parameter_List[3].Integer_Value;
      ConfigData.priority_4   = Parameter_List[4].Integer_Value;

      /* Call the configuration API. */
      Result = qapi_COEX_Configure(&ConfigData);

      /* Print function success/failure. */
      if(Result == QAPI_OK)
      {
         Display_Function_Success(Coex_Demo_Context.QCLI_Handle, "qapi_COEX_Configure");
         RetVal = QCLI_STATUS_SUCCESS_E;
      }
      else
      {
         Display_Function_Error(Coex_Demo_Context.QCLI_Handle, "qapi_COEX_Configure", Result);
         RetVal = QCLI_STATUS_ERROR_E;
      }
   }
   else
   {
      RetVal = QCLI_STATUS_USAGE_E;
   }

   return RetVal;
}

/**
   @brief Executes the "Configure" command to configure the coexistence module.

   Parameter_List[0] Configuration flags.
   Parameter_List[1] Priority 1 value - 1 = BLE, 2 = I15P4, 3 = EXT, 4 = WLAN.
   Parameter_List[2] Priority 2 value - 1 = BLE, 2 = I15P4, 3 = EXT, 4 = WLAN.
   Parameter_List[3] Priority 3 value - 1 = BLE, 2 = I15P4, 3 = EXT, 4 = WLAN.
   Parameter_List[4] Priority 4 value - 1 = BLE, 2 = I15P4, 3 = EXT, 4 = WLAN.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
static QCLI_Command_Status_t cmd_ConfigureAdvanced(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   qapi_Status_t                    Result;
   QCLI_Command_Status_t            RetVal;
   qapi_COEX_Advanced_Config_Data_t ConfigData;

   /* Verify the input parameters. */
   if(Parameter_Count >= 2)
   {
      /* Set the configuration values. */
      memset(&ConfigData, 0, sizeof(qapi_COEX_Advanced_Config_Data_t));

      ConfigData.priority_Config_Data   = Coex_Demo_Context.Priorities;
      ConfigData.priority_Config_Length = CONFIG_PRIORITY_LENGTH;
      ConfigData.config_Flags           = Parameter_List[0].Integer_Value;
      ConfigData.concurrency_Flags      = Parameter_List[1].Integer_Value;

      /* Call the configuration API. */
      Result = qapi_COEX_Configure_Advanced(&ConfigData);

      /* Print function success/failure. */
      if(Result == QAPI_OK)
      {
         Display_Function_Success(Coex_Demo_Context.QCLI_Handle, "qapi_COEX_Configure_Advanced");
         RetVal = QCLI_STATUS_SUCCESS_E;
      }
      else
      {
         Display_Function_Error(Coex_Demo_Context.QCLI_Handle, "qapi_COEX_Configure_Advanced", Result);
         RetVal = QCLI_STATUS_ERROR_E;
      }
   }
   else
   {
      RetVal = QCLI_STATUS_USAGE_E;
   }

   return RetVal;
}

/**
   @brief Sets the BLE priority values.

   Parameter_List[0] Advertise value.
   Parameter_List[1] Scan value.
   Parameter_List[2] Data request value.
   Parameter_List[3] Data active value.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
static QCLI_Command_Status_t cmd_SetBLEPriority(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t   RetVal;

   /* Verify the input parameters. */
   if(Parameter_Count >= 4)
   {
      /* Set the values. */
      Coex_Demo_Context.Priorities[0].priority_Value = Parameter_List[0].Integer_Value;
      Coex_Demo_Context.Priorities[1].priority_Value = Parameter_List[1].Integer_Value;
      Coex_Demo_Context.Priorities[2].priority_Value = Parameter_List[2].Integer_Value;
      Coex_Demo_Context.Priorities[3].priority_Value = Parameter_List[3].Integer_Value;

      RetVal = QCLI_STATUS_SUCCESS_E;
   }
   else
   {
      RetVal = QCLI_STATUS_USAGE_E;
   }

   return RetVal;
}

/**
   @brief Sets the I15P4 priority values.

   Parameter_List[0] Rx request value.
   Parameter_List[1] Tx request value.
   Parameter_List[2] Rx active value.
   Parameter_List[3] Tx active value.
   Parameter_List[4] ACK value.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
static QCLI_Command_Status_t cmd_SetI15P4Priority(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t   RetVal;

   /* Verify the input parameters. */
#ifdef V1
   if(Parameter_Count >= 6)
#else
   if(Parameter_Count >= 7)
#endif
   {
      /* Set the values. */
      Coex_Demo_Context.Priorities[ 8].priority_Value = Parameter_List[0].Integer_Value;
      Coex_Demo_Context.Priorities[ 9].priority_Value = Parameter_List[1].Integer_Value;
      Coex_Demo_Context.Priorities[10].priority_Value = Parameter_List[2].Integer_Value;
      Coex_Demo_Context.Priorities[11].priority_Value = Parameter_List[3].Integer_Value;
      Coex_Demo_Context.Priorities[12].priority_Value = Parameter_List[4].Integer_Value;
      Coex_Demo_Context.Priorities[13].priority_Value = Parameter_List[5].Integer_Value;
#ifdef V2
      Coex_Demo_Context.Priorities[14].priority_Value = Parameter_List[6].Integer_Value;
#endif

      RetVal = QCLI_STATUS_SUCCESS_E;
   }
   else
   {
      RetVal = QCLI_STATUS_USAGE_E;
   }

   return RetVal;
}

/**
   @brief Sets the EXT priority values.

   Parameter_List[0] Low request value.
   Parameter_List[1] High request value.
   Parameter_List[2] Low active value.
   Parameter_List[3] High active value.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
static QCLI_Command_Status_t cmd_SetEXTPriority(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t   RetVal;

   /* Verify the input parameters. */
   if(Parameter_Count >= 4)
   {
      /* Set the values. */
      Coex_Demo_Context.Priorities[4].priority_Value = Parameter_List[0].Integer_Value;
      Coex_Demo_Context.Priorities[5].priority_Value = Parameter_List[1].Integer_Value;
      Coex_Demo_Context.Priorities[6].priority_Value = Parameter_List[2].Integer_Value;
      Coex_Demo_Context.Priorities[7].priority_Value = Parameter_List[3].Integer_Value;

      RetVal = QCLI_STATUS_SUCCESS_E;
   }
   else
   {
      RetVal = QCLI_STATUS_USAGE_E;
   }

   return RetVal;
}

/**
   @brief Executes the "StatisticsEnable" command to enable coexistence statistics.

   Parameter_List[0] Enable/disable statistics.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
static QCLI_Command_Status_t cmd_StatisticsEnable(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   qapi_Status_t           Result;
   QCLI_Command_Status_t   RetVal;
   boolean                 Enable;

   /* Verify the input parameters. */
   if((Parameter_Count >= 1) && (Parameter_List[0].Integer_Value <= 1))
   {
      if(Parameter_List[0].Integer_Value == 0)
         Enable = FALSE;
      else
         Enable = TRUE;

      /* Call the enable statistics API. */
      Result = qapi_COEX_Statistics_Enable(Enable, COEX_STATISTICS_MASK, 0);

      /* Print function success/failure. */
      if(Result == QAPI_OK)
      {
         if(Enable)
         {
            Display_Function_Success(Coex_Demo_Context.QCLI_Handle, "qapi_COEX_Statistics_Enable (Enabled)");
         }
         else
         {
            Display_Function_Success(Coex_Demo_Context.QCLI_Handle, "qapi_COEX_Statistics_Enable (Disabled)");
         }

         RetVal = QCLI_STATUS_SUCCESS_E;
      }
      else
      {
         Display_Function_Error(Coex_Demo_Context.QCLI_Handle, "qapi_COEX_Statistics_Enable", Result);
         RetVal = QCLI_STATUS_ERROR_E;
      }
   }
   else
   {
      RetVal = QCLI_STATUS_USAGE_E;
   }

   return RetVal;
}

/**
   @brief Executes the "StatisticsGet" command to get coexistence statistics.

   Parameter_List[0] Length of the statistics buffer to use.
   Parameter_List[1] Statistics mask.
   Parameter_List[2] Reset statistics (0 = FALSE, 1 = TRUE).

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List is list of parsed arguments associate with this
          command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
static QCLI_Command_Status_t cmd_StatisticsGet(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   qapi_Status_t                Result;
   QCLI_Command_Status_t        RetVal;
   boolean                      Reset;
   qapi_COEX_Statistics_Data_t *StatisticsData;
   uint8                        StatisticsDataLength;
   uint8                        Index;

   /* Verify the input parameters. */
   if((Parameter_Count >= 1) && (Parameter_List[0].Integer_Value <= 1))
   {
      /* Get the parameters. */
      if(Parameter_List[0].Integer_Value == 0)
         Reset = FALSE;
      else
         Reset = TRUE;

      StatisticsDataLength = COEX_STATISTICS_LENGTH;

      /* Allocate memory for the buffer. */
      StatisticsData = (qapi_COEX_Statistics_Data_t *)malloc(sizeof(qapi_COEX_Statistics_Data_t) * StatisticsDataLength);

      /* Make sure the memory was allocated. */
      if(StatisticsData)
      {
         /* Call the get statistics API. */
         Result = qapi_COEX_Statistics_Get(StatisticsData, &StatisticsDataLength, COEX_STATISTICS_MASK, Reset);

         /* Print function success/failure. */
         if(Result == QAPI_OK)
         {
            Display_Function_Success(Coex_Demo_Context.QCLI_Handle, "qapi_COEX_Statistics_Get");
            RetVal = QCLI_STATUS_SUCCESS_E;

            /* Loop through each item and print out the data. */
            for(Index = 0; Index < StatisticsDataLength; Index++)
            {
               switch(StatisticsData[Index].packet_Status_Type)
               {
                  case QAPI_COEX_PACKET_STATUS_TYPE_BLE_SCAN_COMPLETE:
                     QCLI_Printf(Coex_Demo_Context.QCLI_Handle, "QAPI_COEX_PACKET_STATUS_TYPE_BLE_SCAN_COMPLETE count:          %d.\r\n", StatisticsData[Index].packet_Status_Count);
                     break;
                  case QAPI_COEX_PACKET_STATUS_TYPE_BLE_SCAN_STOMP:
                     QCLI_Printf(Coex_Demo_Context.QCLI_Handle, "QAPI_COEX_PACKET_STATUS_TYPE_BLE_SCAN_STOMP count:             %d.\r\n", StatisticsData[Index].packet_Status_Count);
                     break;
                  case QAPI_COEX_PACKET_STATUS_TYPE_BLE_SCAN_CONNECT_COMPLETE:
                     QCLI_Printf(Coex_Demo_Context.QCLI_Handle, "QAPI_COEX_PACKET_STATUS_TYPE_BLE_SCAN_CONNECT_COMPLETE count:  %d.\r\n", StatisticsData[Index].packet_Status_Count);
                     break;
                  case QAPI_COEX_PACKET_STATUS_TYPE_BLE_SCAN_CONNECT_STOMP:
                     QCLI_Printf(Coex_Demo_Context.QCLI_Handle, "QAPI_COEX_PACKET_STATUS_TYPE_BLE_SCAN_CONNECT_STOMP count:     %d.\r\n", StatisticsData[Index].packet_Status_Count);
                     break;
                  case QAPI_COEX_PACKET_STATUS_TYPE_BLE_ADVERTISE_COMPLETE:
                     QCLI_Printf(Coex_Demo_Context.QCLI_Handle, "QAPI_COEX_PACKET_STATUS_TYPE_BLE_ADVERTISE_COMPLETE count:     %d.\r\n", StatisticsData[Index].packet_Status_Count);
                     break;
                  case QAPI_COEX_PACKET_STATUS_TYPE_BLE_ADVERTISE_STOMP:
                     QCLI_Printf(Coex_Demo_Context.QCLI_Handle, "QAPI_COEX_PACKET_STATUS_TYPE_BLE_ADVERTISE_STOMP count:        %d.\r\n", StatisticsData[Index].packet_Status_Count);
                     break;
                  case QAPI_COEX_PACKET_STATUS_TYPE_BLE_DATA_RX_COMPLETE:
                     QCLI_Printf(Coex_Demo_Context.QCLI_Handle, "QAPI_COEX_PACKET_STATUS_TYPE_BLE_DATA_RX_COMPLETE count:       %d.\r\n", StatisticsData[Index].packet_Status_Count);
                     break;
                  case QAPI_COEX_PACKET_STATUS_TYPE_BLE_DATA_RX_STOMP:
                     QCLI_Printf(Coex_Demo_Context.QCLI_Handle, "QAPI_COEX_PACKET_STATUS_TYPE_BLE_DATA_RX_STOMP count:          %d.\r\n", StatisticsData[Index].packet_Status_Count);
                     break;
                  case QAPI_COEX_PACKET_STATUS_TYPE_BLE_DATA_TX_COMPLETE:
                     QCLI_Printf(Coex_Demo_Context.QCLI_Handle, "QAPI_COEX_PACKET_STATUS_TYPE_BLE_DATA_TX_COMPLETE count:       %d.\r\n", StatisticsData[Index].packet_Status_Count);
                     break;
                  case QAPI_COEX_PACKET_STATUS_TYPE_BLE_DATA_TX_STOMP:
                     QCLI_Printf(Coex_Demo_Context.QCLI_Handle, "QAPI_COEX_PACKET_STATUS_TYPE_BLE_DATA_TX_STOMP count:          %d.\r\n", StatisticsData[Index].packet_Status_Count);
                     break;
                  case QAPI_COEX_PACKET_STATUS_TYPE_BLE_ISOC_RX_COMPLETE:
                     QCLI_Printf(Coex_Demo_Context.QCLI_Handle, "QAPI_COEX_PACKET_STATUS_TYPE_BLE_ISOC_RX_COMPLETE count:       %d.\r\n", StatisticsData[Index].packet_Status_Count);
                     break;
                  case QAPI_COEX_PACKET_STATUS_TYPE_BLE_ISOC_RX_STOMP:
                     QCLI_Printf(Coex_Demo_Context.QCLI_Handle, "QAPI_COEX_PACKET_STATUS_TYPE_BLE_ISOC_RX_STOMP count:          %d.\r\n", StatisticsData[Index].packet_Status_Count);
                     break;
                  case QAPI_COEX_PACKET_STATUS_TYPE_BLE_ISOC_TX_COMPLETE:
                     QCLI_Printf(Coex_Demo_Context.QCLI_Handle, "QAPI_COEX_PACKET_STATUS_TYPE_BLE_ISOC_TX_COMPLETE count:       %d.\r\n", StatisticsData[Index].packet_Status_Count);
                     break;
                  case QAPI_COEX_PACKET_STATUS_TYPE_BLE_ISOC_TX_STOMP:
                     QCLI_Printf(Coex_Demo_Context.QCLI_Handle, "QAPI_COEX_PACKET_STATUS_TYPE_BLE_ISOC_TX_STOMP count:          %d.\r\n", StatisticsData[Index].packet_Status_Count);
                     break;
                  case QAPI_COEX_PACKET_STATUS_TYPE_I15P4_ED_SCAN_COMPLETE:
                     QCLI_Printf(Coex_Demo_Context.QCLI_Handle, "QAPI_COEX_PACKET_STATUS_TYPE_I15P4_ED_SCAN_COMPLETE count:     %d.\r\n", StatisticsData[Index].packet_Status_Count);
                     break;
                  case QAPI_COEX_PACKET_STATUS_TYPE_I15P4_ED_SCAN_STOMP:
                     QCLI_Printf(Coex_Demo_Context.QCLI_Handle, "QAPI_COEX_PACKET_STATUS_TYPE_I15P4_ED_SCAN_STOMP count:        %d.\r\n", StatisticsData[Index].packet_Status_Count);
                     break;
                  case QAPI_COEX_PACKET_STATUS_TYPE_I15P4_BEACON_SCAN_COMPLETE:
                     QCLI_Printf(Coex_Demo_Context.QCLI_Handle, "QAPI_COEX_PACKET_STATUS_TYPE_I15P4_BEACON_SCAN_COMPLETE count: %d.\r\n", StatisticsData[Index].packet_Status_Count);
                     break;
                  case QAPI_COEX_PACKET_STATUS_TYPE_I15P4_BEACON_SCAN_STOMP:
                     QCLI_Printf(Coex_Demo_Context.QCLI_Handle, "QAPI_COEX_PACKET_STATUS_TYPE_I15P4_BEACON_SCAN_STOMP count:    %d.\r\n", StatisticsData[Index].packet_Status_Count);
                     break;
                  case QAPI_COEX_PACKET_STATUS_TYPE_I15P4_DATA_RX_COMPLETE:
                     QCLI_Printf(Coex_Demo_Context.QCLI_Handle, "QAPI_COEX_PACKET_STATUS_TYPE_I15P4_DATA_RX_COMPLETE count:     %d.\r\n", StatisticsData[Index].packet_Status_Count);
                     break;
                  case QAPI_COEX_PACKET_STATUS_TYPE_I15P4_DATA_RX_STOMP:
                     QCLI_Printf(Coex_Demo_Context.QCLI_Handle, "QAPI_COEX_PACKET_STATUS_TYPE_I15P4_DATA_RX_STOMP count:        %d.\r\n", StatisticsData[Index].packet_Status_Count);
                     break;
                  case QAPI_COEX_PACKET_STATUS_TYPE_I15P4_DATA_TX_COMPLETE:
                     QCLI_Printf(Coex_Demo_Context.QCLI_Handle, "QAPI_COEX_PACKET_STATUS_TYPE_I15P4_DATA_TX_COMPLETE count:     %d.\r\n", StatisticsData[Index].packet_Status_Count);
                     break;
                  case QAPI_COEX_PACKET_STATUS_TYPE_I15P4_DATA_TX_STOMP:
                     QCLI_Printf(Coex_Demo_Context.QCLI_Handle, "QAPI_COEX_PACKET_STATUS_TYPE_I15P4_DATA_TX_STOMP count:        %d.\r\n", StatisticsData[Index].packet_Status_Count);
                     break;
                  case QAPI_COEX_PACKET_STATUS_TYPE_I15P4_ACK_RX_COMPLETE:
                     QCLI_Printf(Coex_Demo_Context.QCLI_Handle, "QAPI_COEX_PACKET_STATUS_TYPE_I15P4_ACK_RX_COMPLETE count:      %d.\r\n", StatisticsData[Index].packet_Status_Count);
                     break;
                  case QAPI_COEX_PACKET_STATUS_TYPE_I15P4_ACK_RX_STOMP:
                     QCLI_Printf(Coex_Demo_Context.QCLI_Handle, "QAPI_COEX_PACKET_STATUS_TYPE_I15P4_ACK_RX_STOMP count:         %d.\r\n", StatisticsData[Index].packet_Status_Count);
                     break;
                  case QAPI_COEX_PACKET_STATUS_TYPE_I15P4_ACK_TX_COMPLETE:
                     QCLI_Printf(Coex_Demo_Context.QCLI_Handle, "QAPI_COEX_PACKET_STATUS_TYPE_I15P4_ACK_TX_COMPLETE count:      %d.\r\n", StatisticsData[Index].packet_Status_Count);
                     break;
                  case QAPI_COEX_PACKET_STATUS_TYPE_I15P4_ACK_TX_STOMP:
                     QCLI_Printf(Coex_Demo_Context.QCLI_Handle, "QAPI_COEX_PACKET_STATUS_TYPE_I15P4_ACK_TX_STOMP count:         %d.\r\n", StatisticsData[Index].packet_Status_Count);
                     break;
               }
            }
         }
         else
         {
            Display_Function_Error(Coex_Demo_Context.QCLI_Handle, "qapi_COEX_Statistics_Get", Result);
            RetVal = QCLI_STATUS_ERROR_E;
         }

         /* Free the data buffer. */
         free(StatisticsData);
      }
      else
      {
         RetVal = QCLI_STATUS_ERROR_E;
      }
   }
   else
   {
      RetVal = QCLI_STATUS_USAGE_E;
   }

   return RetVal;
}


static QCLI_Command_Status_t cmd_WLANIFEnable(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t RetVal;
   uint32_t              Mode;

   /* Verify the input parameters. */
   if((Parameter_Count >= 1) && (Parameter_List[0].Integer_Value >= 0) && (Parameter_List[0].Integer_Value <= 1))
   {
      /* Get the parameters. */
      Mode = Parameter_List[0].Integer_Value;

      /* Set the GPIO parameters. */
      BT_Active_Config.pin         = 0;
      BT_Pri_0_Config.pin          = 1;
      BT_Pri_1_Config.pin          = 2;
      WLAN_Active_Config.pin       = 3;

      /* Get the GPIO IDs if not already retrieved. */
      if(!WLAN_Initialized)
      {
         if((qapi_TLMM_Get_Gpio_ID(&BT_Active_Config,   &BT_Active_ID)   == QAPI_OK) && \
            (qapi_TLMM_Get_Gpio_ID(&BT_Pri_0_Config,    &BT_Pri_0_ID)    == QAPI_OK) && \
            (qapi_TLMM_Get_Gpio_ID(&BT_Pri_1_Config,    &BT_Pri_1_ID)    == QAPI_OK) && \
            (qapi_TLMM_Get_Gpio_ID(&WLAN_Active_Config, &WLAN_Active_ID) == QAPI_OK))
         {
            /* Flag that the WLAN GPIO IDs have been retrieved. */
            WLAN_Initialized = true;
         }
      }

      /* Configure the GPIO. */
      if(WLAN_Initialized)
      {
         /* Handle the disabled mode. */
         if(Mode == 0)
         {
            BT_Active_Config.func   = 0;
            BT_Pri_0_Config.func    = 0;
            BT_Pri_1_Config.func    = 0;
            WLAN_Active_Config.func = 0;
            BT_Active_Config.dir    = QAPI_GPIO_INPUT_E;
            BT_Pri_0_Config.dir     = QAPI_GPIO_INPUT_E;
            BT_Pri_1_Config.dir     = QAPI_GPIO_INPUT_E;
            WLAN_Active_Config.dir  = QAPI_GPIO_INPUT_E;
         }
            /* Handle the enabled mode. */
         else
         {
            /* Set the GPIO parameters. */
            BT_Active_Config.func   = 1;
            BT_Pri_0_Config.func    = 1;
            BT_Pri_1_Config.func    = 1;
            WLAN_Active_Config.func = 1;
            BT_Active_Config.dir    = QAPI_GPIO_OUTPUT_E;
            BT_Pri_0_Config.dir     = QAPI_GPIO_OUTPUT_E;
            BT_Pri_1_Config.dir     = QAPI_GPIO_OUTPUT_E;
            WLAN_Active_Config.dir  = QAPI_GPIO_INPUT_E;
         }

         /* Set the configuration. */
         if((qapi_TLMM_Config_Gpio(BT_Active_ID,   &BT_Active_Config)   == QAPI_OK) && \
            (qapi_TLMM_Config_Gpio(BT_Pri_0_ID,    &BT_Pri_0_Config)    == QAPI_OK) && \
            (qapi_TLMM_Config_Gpio(BT_Pri_1_ID,    &BT_Pri_1_Config)    == QAPI_OK) && \
            (qapi_TLMM_Config_Gpio(WLAN_Active_ID, &WLAN_Active_Config) == QAPI_OK))
         {
            RetVal = QCLI_STATUS_SUCCESS_E;

            /* Print the status. */
            if(Mode == 0)
            {
               Display_Function_Success(Coex_Demo_Context.QCLI_Handle, "WLAN Interface Disabled.");
            }
            else
            {
               Display_Function_Success(Coex_Demo_Context.QCLI_Handle, "WLAN Interface Enabled.");
            }
         }
         else
         {
            RetVal = QCLI_STATUS_ERROR_E;
            QCLI_Printf(Coex_Demo_Context.QCLI_Handle, "WLAN Interface Failure.\r\n");
         }
      }
      else
      {
         RetVal = QCLI_STATUS_ERROR_E;
         QCLI_Printf(Coex_Demo_Context.QCLI_Handle, "WLAN Interface Failure.\r\n");
      }
   }
   else
   {
      RetVal = QCLI_STATUS_USAGE_E;
   }

   return RetVal;
}

static QCLI_Command_Status_t cmd_EPTAIFEnable(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t RetVal;
   uint32_t              Mode;

   /* Verify the input parameters. */
   if((Parameter_Count >= 1) && (Parameter_List[0].Integer_Value >= 0) && (Parameter_List[0].Integer_Value <= 2))
   {
      /* Get the parameters. */
      Mode = Parameter_List[0].Integer_Value;

      /* Set the pin numbers. */
      EPTA_Pin_1_Config.pin  = 5;
      EPTA_Pin_2_Config.pin  = 6;
      EPTA_Pin_3_Config.pin  = 7;

      /* Get the GPIO IDs if not already retrieved. */
      if(!EPTA_Initialized)
      {
         if((qapi_TLMM_Get_Gpio_ID(&EPTA_Pin_1_Config, &EPTA_Pin_1_ID) == QAPI_OK) && \
            (qapi_TLMM_Get_Gpio_ID(&EPTA_Pin_2_Config, &EPTA_Pin_2_ID) == QAPI_OK) && \
            (qapi_TLMM_Get_Gpio_ID(&EPTA_Pin_3_Config, &EPTA_Pin_3_ID) == QAPI_OK))
         {
            /* Flag that the EPTA GPIO IDs have been retrieved. */
            EPTA_Initialized = true;
         }
      }

      /* Continue if the GPIO IDs were retrieved. */
      if(EPTA_Initialized)
      {
         /* Handle the disabled mode. */
         if(Mode == 0)
         {
            /* Set the GPIO parameters. */
            EPTA_Pin_1_Config.func = 0;
            EPTA_Pin_2_Config.func = 0;
            EPTA_Pin_3_Config.func = 0;
            EPTA_Pin_1_Config.dir  = QAPI_GPIO_INPUT_E;
            EPTA_Pin_2_Config.dir  = QAPI_GPIO_INPUT_E;
            EPTA_Pin_3_Config.dir  = QAPI_GPIO_INPUT_E;
         }
         else
         {
            EPTA_Pin_1_Config.func = 1;
            EPTA_Pin_2_Config.func = 1;
            EPTA_Pin_3_Config.func = 1;

            /* Handle the slave mode. */
            if(Mode == 1)
            {
               /* Set the GPIO parameters. */
               EPTA_Pin_1_Config.dir = QAPI_GPIO_OUTPUT_E; // BT_ACTIVE
               EPTA_Pin_2_Config.dir = QAPI_GPIO_INPUT_E;  // WLAN_ACTIVE
               EPTA_Pin_3_Config.dir = QAPI_GPIO_OUTPUT_E; // BT_PRIORITY
            }
            /* Handle the master mode. */
            else
            {
               /* Set the GPIO parameters. */
               EPTA_Pin_1_Config.dir = QAPI_GPIO_OUTPUT_E; // WLAN_ACTIVE
               EPTA_Pin_2_Config.dir = QAPI_GPIO_INPUT_E;  // BT_ACTIVE
               EPTA_Pin_3_Config.dir = QAPI_GPIO_INPUT_E;  // BT_PRIORITY
            }
         }

         /* Set the configuration. */
         if((qapi_TLMM_Config_Gpio(EPTA_Pin_1_ID, &EPTA_Pin_1_Config) == QAPI_OK) && \
            (qapi_TLMM_Config_Gpio(EPTA_Pin_2_ID, &EPTA_Pin_2_Config) == QAPI_OK) && \
            (qapi_TLMM_Config_Gpio(EPTA_Pin_3_ID, &EPTA_Pin_3_Config) == QAPI_OK))
         {
            RetVal = QCLI_STATUS_SUCCESS_E;

            /* Print the status. */
            if(Mode == 0)
            {
               Display_Function_Success(Coex_Demo_Context.QCLI_Handle, "EPTA Interface Disabled.");
            }
            else
            {
               if(Mode == 1)
               {
                  Display_Function_Success(Coex_Demo_Context.QCLI_Handle, "EPTA Interface Enabled (Slave).");
               }
               else
               {
                  Display_Function_Success(Coex_Demo_Context.QCLI_Handle, "EPTA Interface Enabled (Master).");
               }
            }
         }
         else
         {
            RetVal = QCLI_STATUS_ERROR_E;
            QCLI_Printf(Coex_Demo_Context.QCLI_Handle, "EPTA Interface Failure.\r\n");
         }
      }
      else
      {
         RetVal = QCLI_STATUS_ERROR_E;
         QCLI_Printf(Coex_Demo_Context.QCLI_Handle, "EPTA Interface Failure.\r\n");
      }
   }
   else
   {
      RetVal = QCLI_STATUS_USAGE_E;
   }

   return RetVal;
}

/**
   @brief Registers coex commands with QCLI and initializes the
          sample application.
*/
void Initialize_Coex_Demo(qbool_t IsColdBoot)
{
   qapi_Status_t                Result;
   uint16_t                     Data_Size;
   Coex_Demo_Transition_Data_t *Transition_Data;

   memset(&Coex_Demo_Context, 0, sizeof(Coex_Demo_Context_t));

   /* Restore settings from transition memory. */
   Result = qapi_OMSM_Retrieve(QAPI_OMSM_DEFAULT_AON_POOL, OMSM_CLIENT_ID_COEX_DEMO_E, &Data_Size, (void **)&Transition_Data);

   if((Result == QAPI_OK) && (Transition_Data))
   {
      if(Data_Size >= COEX_DEMO_TRANSITION_DATA_SIZE)
      {
         /* Restore backed up data. */
         memscpy(Coex_Demo_Context.Priorities, sizeof(Coex_Demo_Context.Priorities), Transition_Data->Priorities, sizeof(Transition_Data->Priorities));
      }

      qapi_OMSM_Free(QAPI_OMSM_DEFAULT_AON_POOL, OMSM_CLIENT_ID_COEX_DEMO_E);
   }
   else
   {
      /* Set the default priority types. */
      Coex_Demo_Context.Priorities[ 0].priority_Type  = QAPI_COEX_PRIORITY_BLE_ADVERTISE_E;
      Coex_Demo_Context.Priorities[ 0].priority_Value = 4;
      Coex_Demo_Context.Priorities[ 1].priority_Type  = QAPI_COEX_PRIORITY_BLE_SCAN_E;
      Coex_Demo_Context.Priorities[ 1].priority_Value = 3;
      Coex_Demo_Context.Priorities[ 2].priority_Type  = QAPI_COEX_PRIORITY_BLE_DATA_REQUEST_E;
      Coex_Demo_Context.Priorities[ 2].priority_Value = 44;
      Coex_Demo_Context.Priorities[ 3].priority_Type  = QAPI_COEX_PRIORITY_BLE_DATA_ACTIVE_E;
      Coex_Demo_Context.Priorities[ 3].priority_Value = 46;
      Coex_Demo_Context.Priorities[ 4].priority_Type  = QAPI_COEX_PRIORITY_EXT_LOW_REQUEST_E;
      Coex_Demo_Context.Priorities[ 4].priority_Value = 23;
      Coex_Demo_Context.Priorities[ 5].priority_Type  = QAPI_COEX_PRIORITY_EXT_HIGH_REQUEST_E;
      Coex_Demo_Context.Priorities[ 5].priority_Value = 59;
      Coex_Demo_Context.Priorities[ 6].priority_Type  = QAPI_COEX_PRIORITY_EXT_LOW_ACTIVE_E;
      Coex_Demo_Context.Priorities[ 6].priority_Value = 24;
      Coex_Demo_Context.Priorities[ 7].priority_Type  = QAPI_COEX_PRIORITY_EXT_HIGH_ACTIVE_E;
      Coex_Demo_Context.Priorities[ 7].priority_Value = 61;
      Coex_Demo_Context.Priorities[ 8].priority_Type  = QAPI_COEX_PRIORITY_I15P4_RX_REQUEST_E;
      Coex_Demo_Context.Priorities[ 8].priority_Value = 21;
      Coex_Demo_Context.Priorities[ 9].priority_Type  = QAPI_COEX_PRIORITY_I15P4_TX_REQUEST_E;
      Coex_Demo_Context.Priorities[ 9].priority_Value = 22;
      Coex_Demo_Context.Priorities[10].priority_Type  = QAPI_COEX_PRIORITY_I15P4_RX_ACTIVE_E;
      Coex_Demo_Context.Priorities[10].priority_Value = 41;
      Coex_Demo_Context.Priorities[11].priority_Type  = QAPI_COEX_PRIORITY_I15P4_TX_ACTIVE_E;
      Coex_Demo_Context.Priorities[11].priority_Value = 42;
      Coex_Demo_Context.Priorities[12].priority_Type  = QAPI_COEX_PRIORITY_I15P4_ACK_E;
      Coex_Demo_Context.Priorities[12].priority_Value = 62;
      Coex_Demo_Context.Priorities[13].priority_Type  = QAPI_COEX_PRIORITY_I15P4_ED_SCAN_E;
      Coex_Demo_Context.Priorities[13].priority_Value = 38;
      #ifdef V2
      Coex_Demo_Context.Priorities[14].priority_Type  = QAPI_COEX_PRIORITY_I15P4_HOLD_REQUEST_E;
      Coex_Demo_Context.Priorities[14].priority_Value = 60;
      #endif
   }

   Coex_Demo_Context.QCLI_Handle = QCLI_Register_Command_Group(Coex_Demo_Context.QCLI_Handle, &Coex_CMD_Group);

   if(IsColdBoot)
   {
      QCLI_Printf(Coex_Demo_Context.QCLI_Handle, "Coex Demo Initialized.\r\n");
   }
}

/**
   @brief Prepares the Coex demo for a mode change.

   @return true if the mode transition can proceed or false if it should be
           aborted.
*/
qbool_t Coex_Prepare_Mode_Switch(Operating_Mode_t Next_Mode)
{
   qbool_t                      Ret_Val;
   qapi_Status_t                Result;
   Coex_Demo_Transition_Data_t *Transition_Data;

   /* Make sure the demo has been initialized successfully. */
   if(Coex_Demo_Context.QCLI_Handle)
   {
      /* Allocate the transition memory. */
      Result = qapi_OMSM_Alloc(QAPI_OMSM_DEFAULT_AON_POOL, OMSM_CLIENT_ID_COEX_DEMO_E, COEX_DEMO_TRANSITION_DATA_SIZE, (void **)&Transition_Data);

      if((Result == QAPI_OK) && (Transition_Data))
      {
         memset(Transition_Data, 0, COEX_DEMO_TRANSITION_DATA_SIZE);

         Result = qapi_OMSM_Commit(QAPI_OMSM_DEFAULT_AON_POOL, OMSM_CLIENT_ID_COEX_DEMO_E);

         if(Result == QAPI_OK)
         {
            Ret_Val = true;
         }
         else
         {
            qapi_OMSM_Free(QAPI_OMSM_DEFAULT_AON_POOL, OMSM_CLIENT_ID_COEX_DEMO_E);
            Display_Function_Error(Coex_Demo_Context.QCLI_Handle, "qapi_OMSM_Commit", Result);
            Ret_Val = false;
         }
      }
      else
      {
         Display_Function_Error(Coex_Demo_Context.QCLI_Handle, "qapi_OMSM_Alloc", Result);
         Ret_Val = false;
      }
   }
   else
   {
      Ret_Val = true;
   }

   return(Ret_Val);
}

/**
   @brief Undoes preparation of the Coex demo for a mode change.
*/
void Coex_Cancel_Mode_Switch(void)
{
   if(Coex_Demo_Context.QCLI_Handle)
   {
      qapi_OMSM_Free(QAPI_OMSM_DEFAULT_AON_POOL, OMSM_CLIENT_ID_COEX_DEMO_E);
   }
}

/**
   @brief Finalizes the mode change for the Coex demo.
*/
void Coex_Exit_Mode(Operating_Mode_t Next_Mode)
{
   qapi_Status_t                Result;
   uint16_t                     Data_Size;
   Coex_Demo_Transition_Data_t *Transition_Data;

   if(Coex_Demo_Context.QCLI_Handle)
   {
      Result = qapi_OMSM_Retrieve(QAPI_OMSM_DEFAULT_AON_POOL, OMSM_CLIENT_ID_COEX_DEMO_E, &Data_Size, (void **)&Transition_Data);

      if((Result == QAPI_OK) && (Transition_Data))
      {
         /* Make sure the size matches what is expected. */
         if(Data_Size >= COEX_DEMO_TRANSITION_DATA_SIZE)
         {
            /* Backup necessary data. */
            memscpy(Transition_Data->Priorities, sizeof(Transition_Data->Priorities), Coex_Demo_Context.Priorities, sizeof(Coex_Demo_Context.Priorities));

            Result = qapi_OMSM_Commit(QAPI_OMSM_DEFAULT_AON_POOL, OMSM_CLIENT_ID_COEX_DEMO_E);

            if(Result != QAPI_OK)
            {
               /* Unable to commit memory so just free it. */
               qapi_OMSM_Free(QAPI_OMSM_DEFAULT_AON_POOL, OMSM_CLIENT_ID_COEX_DEMO_E);
            }
         }
         else
         {
            qapi_OMSM_Free(QAPI_OMSM_DEFAULT_AON_POOL, OMSM_CLIENT_ID_COEX_DEMO_E);
         }
      }
   }
}
