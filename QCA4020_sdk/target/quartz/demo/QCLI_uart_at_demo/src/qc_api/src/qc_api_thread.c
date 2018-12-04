/*
 *  Copyright (c) 2018 Qualcomm Technologies, Inc.
 *  All Rights Reserved.
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

/*-------------------------------------------------------------------------
 *  Include Files
 *-----------------------------------------------------------------------*/
#include "qc_api_thread.h"
#include "qc_api_main.h"
#include "qc_drv_thread.h"
#include "qosa_util.h"

/*-------------------------------------------------------------------------
 * Static & global Variable Declarations
 *-----------------------------------------------------------------------*/

/* The prefix used for the default EUI64 address for the 802.15.4 MAC. The
   actual default EUI64 address is determined when the Initialize command is
   called by appending the short address. */
#define DEFAULT_EUI64_PREFIX                 (0x000000FFFE000000ULL)

/* This value is the default timeout for this device as a child. */
#define DEFAULT_CHILD_TIMEOUT                (60)

/* Uncomment this to enable printing logs from OpenThread. */
//#define ALLOW_OPENTHREAD_DEBUG_LOGS

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

/**
   @brief Helper function to print out a Network State Event.

   @param State  The Network State to print out.
*/
void Print_Network_State_Event(qapi_TWN_Network_State_t State)
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

   LOG_AT_EVT("EVT_TH: Network State Changed: %s\n", State_String);
   QCLI_Display_Prompt();
}

/**
   @brief Helper function to print out a Joiner Result Event.

   @param Result  The Joiner Result to print.
*/
void Print_Joiner_Result_Event(qapi_TWN_Joiner_Result_t Result)
{
   const char *Result_String;

   switch(Result)
   {
      case QAPI_TWN_JOINER_RESULT_SUCCESS_E:
         Result_String = "Success";
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

   LOG_AT_EVT("EVT_TH: Joiner Result: %s\n", Result_String);

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
void DisplayNetworkInfo(void)
{
   qapi_Status_t                    Result;
   qapi_TWN_Network_Configuration_t Network_Config;
   uint8_t                          Index;

   if(Thread_Demo_Context.TWN_Handle != QAPI_TWN_INVALID_HANDLE)
   {
      Result = qc_drv_TWN_Get_Network_Configuration(qc_api_get_qc_drv_context(), Thread_Demo_Context.TWN_Handle, &Network_Config);
      if(Result == QAPI_OK)
      {
         /* Display the network configuration. */
         LOG_AT("Network Configuration:\n");
         LOG_AT("   Channel:          %d\n", Network_Config.Channel);
         LOG_AT("   PAN_ID:           %04X\n", Network_Config.PAN_ID);
         LOG_AT("   Extended_PAN_ID:  %08X%08X\n", (uint32_t)((Network_Config.Extended_PAN_ID) >> 32), (uint32_t)(Network_Config.Extended_PAN_ID));
         LOG_AT("   NetworkName:      %s\n", Network_Config.NetworkName);
         LOG_AT("   MasterKey:        ");

         for(Index = 0; Index < QAPI_OPEN_THREAD_MASTER_KEY_SIZE; Index++)
         {
            LOG_AT("%02X", Network_Config.MasterKey[Index]);
         }

         LOG_AT("\n");
      }
      else
      {
         Display_Function_Error("qapi_TWN_Get_Network_Configuration", Result);
      }
   }
   else
   {
      LOG_WARN("TWN not initialized.\n");
   }
}

/**
   @brief Handles callbacks from the Thread Wireless Interface.

   @param TWN_Event is a structure which contains the information for the
          event.
   @param CB_Param  is the user specified parameter for the callback function.
*/
void TWN_Event_CB(qapi_TWN_Handle_t TWN_Handle, const qapi_TWN_Event_t *TWN_Event, uint32_t CB_Param)
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
         LOG_AT_EVT("EVT_TH: Unknown Event Received (%u).\n", (uint32_t)TWN_Event->Event_Type);
         break;
   }
}

/**
   @brief Set the Poll Period.

   Parameter_List[0] Poll Period in Sec.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List  is the list of parsed arguments associated with
          this command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_api_th_SetPollPeriod(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t Ret_Val;
   uint32_t Period;
   qapi_Status_t Result;

   /* Verify the TWN layer is initialized. */
   if(Thread_Demo_Context.TWN_Handle != QAPI_TWN_INVALID_HANDLE)
   {
      if((Parameter_Count >= 1) && (Verify_Integer_Parameter(&(Parameter_List[0]), 0, 1000)))
      {
	    Period = (uint32_t)(Parameter_List[0].Integer_Value); 

            Result = qc_drv_TWN_Set_Max_Poll_Period(qc_api_get_qc_drv_context(), Thread_Demo_Context.TWN_Handle, Period);

            if(Result == QAPI_OK)
            {
               LOG_INFO("Max_Poll_Period set is: %d\n", Period);
               Display_Function_Success("qapi_TWN_Set_Max_Poll_Period");

               Ret_Val = QCLI_STATUS_SUCCESS_E;
            }
            else
            {
               Display_Function_Error("qapi_TWN_Set_Max_Poll_Period", Result);
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
      LOG_WARN("TWN not initialized.\n");
      Ret_Val = QCLI_STATUS_ERROR_E;
   }

   return(Ret_Val);
}

/**
   @brief Initializes the thread interface and sets the default device and
          network configurations.

   Parameter_List[0] (0 - 1) is a flag that indicates if the device should
                     operation as a router or sleepy end device.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List  is the list of parsed arguments associated with
          this command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_api_th_Initialize(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t Ret_Val;
   qapi_Status_t         Result;

   /* Verify the TWN layer is not already initialized. */
   if(Thread_Demo_Context.TWN_Handle == QAPI_TWN_INVALID_HANDLE)
   {
      /* Check the parameters. */
      if((Parameter_Count >= 1) && (Verify_Integer_Parameter(&(Parameter_List[0]), 0, 1)))
      {
         Result = qc_drv_TWN_Initialize(qc_api_get_qc_drv_context(), &(Thread_Demo_Context.TWN_Handle), TWN_Event_CB, 0);

         if((Result == QAPI_OK) && (Thread_Demo_Context.TWN_Handle != QAPI_TWN_INVALID_HANDLE))
         {
            /* Set the default device information. */
            Thread_Demo_Context.Device_Configuration.Child_Timeout            = DEFAULT_CHILD_TIMEOUT;
            Thread_Demo_Context.Device_Configuration.Use_Secure_Data_Requests = true;

            if(Parameter_List[0].Integer_Value == 0)
            {
               /* Router configuration. */
               Thread_Demo_Context.Device_Configuration.Is_FFD               = true;
               Thread_Demo_Context.Device_Configuration.Rx_On_While_Idle     = true;
               Thread_Demo_Context.Device_Configuration.Require_Network_Data = true;
            }
            else
            {
               /* Sleepy Device configuration. */
               Thread_Demo_Context.Device_Configuration.Is_FFD               = false;
               Thread_Demo_Context.Device_Configuration.Rx_On_While_Idle     = false;
               Thread_Demo_Context.Device_Configuration.Require_Network_Data = false;
            }

            Result = qc_drv_TWN_Set_Device_Configuration(qc_api_get_qc_drv_context(), Thread_Demo_Context.TWN_Handle, &(Thread_Demo_Context.Device_Configuration));

            if(Result == QAPI_OK)
            {
               /* Get the network information configured by default. */
               Result = qc_drv_TWN_Get_Network_Configuration(qc_api_get_qc_drv_context(), Thread_Demo_Context.TWN_Handle, &(Thread_Demo_Context.Network_Configuration));

               if(Result == QAPI_OK)
               {
                  LOG_DEBUG("Thread Initialized Successfully:\n");
                  Ret_Val = QCLI_STATUS_SUCCESS_E;
               }
               else
               {
                  Display_Function_Error("qapi_TWN_Get_Network_Configuration", Result);
                  Ret_Val = QCLI_STATUS_ERROR_E;
               }
            }
            else
            {
               Display_Function_Error("qapi_TWN_Set_Device_Configuration", Result);
               Ret_Val = QCLI_STATUS_ERROR_E;
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
            Display_Function_Error("qapi_TWN_Initialize", Result);

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
      LOG_WARN("TWN already initialized.\n");

      Ret_Val = QCLI_STATUS_ERROR_E;
   }

   return(Ret_Val);
}

/**
   @brief Shuts down the Thread interface.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List  is the list of parsed arguments associated with
          this command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_api_th_Shutdown(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t Ret_Val;

   /* Verify the TWN layer is initialized. */
   if(Thread_Demo_Context.TWN_Handle != QAPI_TWN_INVALID_HANDLE)
   {
      qc_drv_TWN_Shutdown(qc_api_get_qc_drv_context(), Thread_Demo_Context.TWN_Handle);

      Thread_Demo_Context.TWN_Handle = QAPI_TWN_INVALID_HANDLE;

      LOG_INFO("TWN shutdown.\n");

      Ret_Val = QCLI_STATUS_SUCCESS_E;
   }
   else
   {
      LOG_WARN("TWN not initialized.\n");
      Ret_Val = QCLI_STATUS_ERROR_E;
   }

   return(Ret_Val);
}

/**
   @brief Start the Thread Interface, connecting or starting a network.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List  is the list of parsed arguments associated with
          this command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_api_th_Start(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t Ret_Val;
   qapi_Status_t         Result;

   /* Verify the TWN layer is initialized. */
   if(Thread_Demo_Context.TWN_Handle != QAPI_TWN_INVALID_HANDLE)
   {
      Result = qc_drv_TWN_Start(qc_api_get_qc_drv_context(), Thread_Demo_Context.TWN_Handle);

      if(Result == QAPI_OK)
      {
         Display_Function_Success("qapi_TWN_Start");

         Ret_Val = QCLI_STATUS_SUCCESS_E;
      }
      else
      {
         Display_Function_Error("qapi_TWN_Start", Result);

         Ret_Val = QCLI_STATUS_ERROR_E;
      }
   }
   else
   {
      LOG_WARN("TWN not initialized.\n");
      Ret_Val = QCLI_STATUS_ERROR_E;
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
QCLI_Command_Status_t qc_api_th_Stop(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t Ret_Val;
   qapi_Status_t         Result;

   /* Verify the TWN layer is initialized. */
   if(Thread_Demo_Context.TWN_Handle != QAPI_TWN_INVALID_HANDLE)
   {
      Result = qc_drv_TWN_Stop(qc_api_get_qc_drv_context(), Thread_Demo_Context.TWN_Handle);

      if(Result == QAPI_OK)
      {
         Display_Function_Success("qapi_TWN_Stop");

         Ret_Val = QCLI_STATUS_SUCCESS_E;
      }
      else
      {
         Display_Function_Error("qapi_TWN_Stop", Result);

         Ret_Val = QCLI_STATUS_ERROR_E;
      }
   }
   else
   {
      LOG_WARN("TWN not initialized.\n");
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
QCLI_Command_Status_t qc_api_th_GetDeviceConfiguration(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t           Ret_Val;
   qapi_TWN_Device_Configuration_t Device_Config;
   qapi_Status_t                   Result;

   /* Verify the TWN layer is initialized. */
   if(Thread_Demo_Context.TWN_Handle != QAPI_TWN_INVALID_HANDLE)
   {
      Result = qc_drv_TWN_Get_Device_Configuration(qc_api_get_qc_drv_context(), Thread_Demo_Context.TWN_Handle, &Device_Config);

      if(Result == QAPI_OK)
      {
         /* Display the device configuration. */
         LOG_AT("Device Configuration:\n");
         LOG_AT("   Extended Address:        %08X%08X\n", (uint32_t)((Device_Config.Extended_Address) >> 32), (uint32_t)(Device_Config.Extended_Address));
         LOG_AT("   Child Timeout:           %d seconds\n", Device_Config.Child_Timeout);
         LOG_AT("   Rx On While Idle:        %d\n", Device_Config.Rx_On_While_Idle);
         LOG_AT("   Use Secure Data Request: %d\n", Device_Config.Use_Secure_Data_Requests);
         LOG_AT("   Is FFD:                  %d\n", Device_Config.Is_FFD);
         LOG_AT("   Require Network Data:    %d\n", Device_Config.Require_Network_Data);

         Ret_Val = QCLI_STATUS_SUCCESS_E;
      }
      else
      {
         Display_Function_Error("qapi_HMI_Get_Device_Configuration", Result);

         Ret_Val = QCLI_STATUS_ERROR_E;
      }
   }
   else
   {
      LOG_WARN("TWN not initialized.\n");
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
QCLI_Command_Status_t qc_api_th_GetNetworkConfiguration(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t            Ret_Val;
   qapi_TWN_Network_Configuration_t Network_Config;
   qapi_Status_t                    Result;
   uint8_t                          Index;

   /* Verify the TWN layer is initialized. */
   if(Thread_Demo_Context.TWN_Handle != QAPI_TWN_INVALID_HANDLE)
   {
      Result = qc_drv_TWN_Get_Network_Configuration(qc_api_get_qc_drv_context(), Thread_Demo_Context.TWN_Handle, &Network_Config);

      if(Result == QAPI_OK)
      {
         /* Display the device cofiguration. */
         LOG_AT("Network Configuration:\n");
         LOG_AT("   Channel:          %d\n", Network_Config.Channel);
         LOG_AT("   PAN_ID:           %04X\n", Network_Config.PAN_ID);
         LOG_AT("   Extended_PAN_ID:  %08X%08X\n", (uint32_t)((Network_Config.Extended_PAN_ID) >> 32), (uint32_t)(Network_Config.Extended_PAN_ID));
         LOG_AT("   NetworkName:      %s\n", Network_Config.NetworkName);
         LOG_AT("   MasterKey:        ");

         for(Index = 0; Index < QAPI_OPEN_THREAD_MASTER_KEY_SIZE; Index++)
         {
            LOG_AT("%02X", Network_Config.MasterKey[Index]);
         }

         LOG_AT("\n");

         Ret_Val = QCLI_STATUS_SUCCESS_E;
      }
      else
      {
         Display_Function_Error("qapi_TWN_Get_Network_Configuration", Result);

         Ret_Val = QCLI_STATUS_ERROR_E;
      }
   }
   else
   {
      LOG_WARN("TWN not initialized.\n");
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
QCLI_Command_Status_t qc_api_th_SetExtendedAddress(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
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
         Result = qc_drv_TWN_Get_Device_Configuration(qc_api_get_qc_drv_context(), Thread_Demo_Context.TWN_Handle, &Device_Config);
         if(Result == QAPI_OK)
         {
            /* Set up the extended address feild in device configuration. */
            Device_Config.Extended_Address = Extended_Address;

            /* Write back the device configuration. */
            Result = qc_drv_TWN_Set_Device_Configuration(qc_api_get_qc_drv_context(), Thread_Demo_Context.TWN_Handle, &Device_Config);
            if(Result == QAPI_OK)
            {
               Display_Function_Success("qapi_TWN_Set_Device_Configuration");

               Ret_Val = QCLI_STATUS_SUCCESS_E;
            }
            else
            {
               Display_Function_Error("qapi_TWN_Set_Device_Configuration", Result);

               Ret_Val = QCLI_STATUS_ERROR_E;
            }
         }
         else
         {
            Display_Function_Error("qapi_TWN_Get_Device_Configuration", Result);

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
      LOG_WARN("TWN not initialized.\n");
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
QCLI_Command_Status_t qc_api_th_SetChildTimeout(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
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
         Result = qc_drv_TWN_Get_Device_Configuration(qc_api_get_qc_drv_context(), Thread_Demo_Context.TWN_Handle, &Device_Config);
         if(Result == QAPI_OK)
         {
            /* Set up the child timeout feild in device configuration. */
            Device_Config.Child_Timeout = (uint32_t)(Parameter_List[0].Integer_Value);

            /* Write back the device configuration. */
            Result = qc_drv_TWN_Set_Device_Configuration(qc_api_get_qc_drv_context(), Thread_Demo_Context.TWN_Handle, &Device_Config);
            if(Result == QAPI_OK)
            {
               Display_Function_Success("qapi_TWN_Set_Device_Configuration");

               Ret_Val = QCLI_STATUS_SUCCESS_E;
            }
            else
            {
               Display_Function_Error("qapi_TWN_Set_Device_Configuration", Result);

               Ret_Val = QCLI_STATUS_ERROR_E;
            }
         }
         else
         {
            Display_Function_Error("qapi_TWN_Get_Device_Configuration", Result);

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
      LOG_WARN("TWN not initialized.\n");
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
QCLI_Command_Status_t qc_api_th_SetLinkMode(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
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
         Result = qc_drv_TWN_Get_Device_Configuration(qc_api_get_qc_drv_context(), Thread_Demo_Context.TWN_Handle, &Device_Config);
         if(Result == QAPI_OK)
         {
            /* Set up the rx on while idle, use secure data request, FFD and
               require network data fields in device configuration. */
            Device_Config.Rx_On_While_Idle         = (qbool_t)Parameter_List[0].Integer_Value;
            Device_Config.Use_Secure_Data_Requests = (qbool_t)Parameter_List[1].Integer_Value;
            Device_Config.Is_FFD                   = (qbool_t)Parameter_List[2].Integer_Value;
            Device_Config.Require_Network_Data     = (qbool_t)Parameter_List[3].Integer_Value;

            /* Write back the device configuration. */
            Result = qc_drv_TWN_Set_Device_Configuration(qc_api_get_qc_drv_context(), Thread_Demo_Context.TWN_Handle, &Device_Config);
            if(Result == QAPI_OK)
            {
               Display_Function_Success("qapi_TWN_Set_Device_Configuration");

               Ret_Val = QCLI_STATUS_SUCCESS_E;
            }
            else
            {
               Display_Function_Error("qapi_TWN_Set_Device_Configuration", Result);

               Ret_Val = QCLI_STATUS_ERROR_E;
            }
         }
         else
         {
            Display_Function_Error("qapi_TWN_Get_Device_Configuration", Result);

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
      LOG_WARN("TWN not initialized.\n");
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
QCLI_Command_Status_t qc_api_th_SetChannel(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
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
         Result = qc_drv_TWN_Get_Network_Configuration(qc_api_get_qc_drv_context(), Thread_Demo_Context.TWN_Handle, &Network_Config);
         if(Result == QAPI_OK)
         {
            /* Set up the channel feild in network configuration. */
            Network_Config.Channel = (uint8_t)(Parameter_List[0].Integer_Value);

            /* Write back the network configuration. */
            Result = qc_drv_TWN_Set_Network_Configuration(qc_api_get_qc_drv_context(), Thread_Demo_Context.TWN_Handle, &Network_Config);
            if(Result == QAPI_OK)
            {
               Display_Function_Success("qapi_TWN_Set_Network_Configuration");

               Ret_Val = QCLI_STATUS_SUCCESS_E;
            }
            else
            {
               Display_Function_Error("qapi_TWN_Set_Network_Configuration", Result);

               Ret_Val = QCLI_STATUS_ERROR_E;
            }
         }
         else
         {
            Display_Function_Error("qapi_TWN_Get_Network_Configuration", Result);

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
      LOG_WARN("TWN not initialized.\n");
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
QCLI_Command_Status_t qc_api_th_SetPANID(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
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
         Result = qc_drv_TWN_Get_Network_Configuration(qc_api_get_qc_drv_context(), Thread_Demo_Context.TWN_Handle, &Network_Config);
         if(Result == QAPI_OK)
         {
            /* Set up the pan ID feild in network configuration. */
            Network_Config.PAN_ID = (uint16_t)(Parameter_List[0].Integer_Value);

            /* Write back the network configuration. */
            Result = qc_drv_TWN_Set_Network_Configuration(qc_api_get_qc_drv_context(), Thread_Demo_Context.TWN_Handle, &Network_Config);
            if(Result == QAPI_OK)
            {
               Display_Function_Success("qapi_TWN_Set_Network_Configuration");

               Ret_Val = QCLI_STATUS_SUCCESS_E;
            }
            else
            {
               Display_Function_Error("qapi_TWN_Set_Network_Configuration", Result);

               Ret_Val = QCLI_STATUS_ERROR_E;
            }
         }
         else
         {
            Display_Function_Error("qapi_TWN_Get_Network_Configuration", Result);

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
      LOG_WARN("TWN not initialized.\n");
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
QCLI_Command_Status_t qc_api_th_SetExtendedPANID(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
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
         Result = qc_drv_TWN_Get_Network_Configuration(qc_api_get_qc_drv_context(), Thread_Demo_Context.TWN_Handle, &Network_Config);
         if(Result == QAPI_OK)
         {
            /* Set up the extended pan ID feild in network configuration. */
            Network_Config.Extended_PAN_ID = Extended_PAN_ID;

            /* Write back the network configuration. */
            Result = qc_drv_TWN_Set_Network_Configuration(qc_api_get_qc_drv_context(), Thread_Demo_Context.TWN_Handle, &Network_Config);
            if(Result == QAPI_OK)
            {
               Display_Function_Success("qapi_TWN_Set_Network_Configuration");

               Ret_Val = QCLI_STATUS_SUCCESS_E;
            }
            else
            {
               Display_Function_Error("qapi_TWN_Set_Network_Configuration", Result);

               Ret_Val = QCLI_STATUS_ERROR_E;
            }
         }
         else
         {
            Display_Function_Error("qapi_TWN_Get_Network_Configuration", Result);

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
      LOG_WARN("TWN not initialized.\n");
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
QCLI_Command_Status_t qc_api_th_SetNetworkName(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
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
         Result = qc_drv_TWN_Get_Network_Configuration(qc_api_get_qc_drv_context(), Thread_Demo_Context.TWN_Handle, &Network_Config);
         if(Result == QAPI_OK)
         {
            /* Set up the network name feild in network configuration. */
            memset(Network_Config.NetworkName, 0, sizeof(Network_Config.NetworkName));
            strlcpy((char *)(Network_Config.NetworkName), Parameter_List[0].String_Value, sizeof(Network_Config.NetworkName));

            /* Write back the network configuration. */
            Result = qc_drv_TWN_Set_Network_Configuration(qc_api_get_qc_drv_context(), Thread_Demo_Context.TWN_Handle, &Network_Config);
            if(Result == QAPI_OK)
            {
               Display_Function_Success("qapi_TWN_Set_Network_Configuration");

               Ret_Val = QCLI_STATUS_SUCCESS_E;
            }
            else
            {
               Display_Function_Error("qapi_TWN_Set_Network_Configuration", Result);

               Ret_Val = QCLI_STATUS_ERROR_E;
            }
         }
         else
         {
            Display_Function_Error("qapi_TWN_Get_Network_Configuration", Result);

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
      LOG_WARN("TWN not initialized.\n");
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
QCLI_Command_Status_t qc_api_th_SetMasterKey(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
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
         Result = qc_drv_TWN_Get_Network_Configuration(qc_api_get_qc_drv_context(), Thread_Demo_Context.TWN_Handle, &Network_Config);
         if(Result == QAPI_OK)
         {
            /* Set up the masterkey feild in network configuration. */
            Length = QAPI_OPEN_THREAD_MASTER_KEY_SIZE;
            if(Hex_String_To_Array(Parameter_List[0].String_Value, &Length, Network_Config.MasterKey))
            {
               /* Write back the network configuration. */
               Result = qc_drv_TWN_Set_Network_Configuration(qc_api_get_qc_drv_context(), Thread_Demo_Context.TWN_Handle, &Network_Config);
               if(Result == QAPI_OK)
               {
                  Display_Function_Success("qapi_TWN_Set_Network_Configuration");

                  Ret_Val = QCLI_STATUS_SUCCESS_E;
               }
               else
               {
                  Display_Function_Error("qapi_TWN_Set_Network_Configuration", Result);

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
            Display_Function_Error("qapi_TWN_Get_Network_Configuration", Result);

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
      LOG_WARN("TWN not initialized.\n");
      Ret_Val = QCLI_STATUS_ERROR_E;
   }

   return(Ret_Val);
}

/**
   @brief Start the Commissioner role on this device.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List  is the list of parsed arguments associated with
          this command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_api_th_MeshCoP_CommissionerStart(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t            Ret_Val;
   qapi_Status_t                    Result;

   /* Verify the TWN layer is initialized. */
   if(Thread_Demo_Context.TWN_Handle != QAPI_TWN_INVALID_HANDLE)
   {
      Result = qc_drv_TWN_Commissioner_Start(qc_api_get_qc_drv_context(), Thread_Demo_Context.TWN_Handle);
      if(Result == QAPI_OK)
      {
         Ret_Val = QCLI_STATUS_SUCCESS_E;
         Display_Function_Success("qapi_TWN_Commissioner_Start");
      }
      else
      {
         Result = QAPI_ERROR;
         Ret_Val = QCLI_STATUS_ERROR_E;
         Display_Function_Error("qapi_TWN_Commissioner_Start", Result);
      }
   }
   else
   {
      LOG_WARN("TWN not initialized.\n");
      Ret_Val = QCLI_STATUS_ERROR_E;
   }

   return Ret_Val;
}

/**
   @brief Stop the Commissioner role on this device.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List  is the list of parsed arguments associated with
          this command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_api_th_MeshCoP_CommissionerStop(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t            Ret_Val;
   qapi_Status_t                    Result;

   /* Verify the TWN layer is initialized. */
   if(Thread_Demo_Context.TWN_Handle != QAPI_TWN_INVALID_HANDLE)
   {
      Result = qc_drv_TWN_Commissioner_Stop(qc_api_get_qc_drv_context(), Thread_Demo_Context.TWN_Handle);
      if(Result == QAPI_OK)
      {
         Ret_Val = QCLI_STATUS_SUCCESS_E;
         Display_Function_Success("qapi_TWN_Commissioner_Stop");
      }
      else
      {
         Result = QAPI_ERROR;
         Ret_Val = QCLI_STATUS_ERROR_E;
         Display_Function_Error("qapi_TWN_Commissioner_Stop", Result);
      }
   }
   else
   {
      LOG_WARN("TWN not initialized.\n");
      Ret_Val = QCLI_STATUS_ERROR_E;
   }

   return Ret_Val;
}

/**
   @brief Add a joining device information to the Steering Information.

   Parameter_List[0] the ASCII string for the joining device's PSKd.
   Parameter_List[1] the EUI-64 of the joining device in hexadecimal, or
                     provide * to allow any device (optional, default=*).
   Parameter_List[2] the timeout in seconds for the joining device (optional,
                     default=120).

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List  is the list of parsed arguments associated with
          this command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_api_th_MeshCoP_CommissionerAddJoiner(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t  Ret_Val;
   qapi_Status_t          Result;
   uint64_t               EUI64;
   const char            *Passphrase;
   uint32_t               Timeout;

   /* Verify the TWN layer is initialized. */
   if(Thread_Demo_Context.TWN_Handle != QAPI_TWN_INVALID_HANDLE)
   {
      Ret_Val = QCLI_STATUS_SUCCESS_E;

      /* Ensure at least the passphrase is provided. */
      if(Parameter_Count >= 1)
      {
         Passphrase = Parameter_List[0].String_Value;

         /* Is the extended address provided? (default *, any) */
         if(Parameter_Count > 1)
         {
            /* Did the user enter "*" as the EUI-64? */
            if(!strcmp(Parameter_List[1].String_Value, "*"))
            {
               /* Allow any. */
               EUI64 = 0ULL;
            }
            else
            {
               if(!Hex_String_To_ULL(Parameter_List[1].String_Value, &EUI64))
               {
                  LOG_WARN("Error parsing EUI-64.\r\n");
                  Ret_Val = QCLI_STATUS_USAGE_E;
               }
            }
         }
         else
         {
            /* Allow any. */
            EUI64 = 0ULL;
         }

         /* Is the timeout provided? (default 120). */
         if(Parameter_Count > 2)
         {
            if(Verify_Integer_Parameter(&Parameter_List[2], 1, 65535))
            {
               Timeout = Parameter_List[2].Integer_Value;
            }
            else
            {
               LOG_WARN("Error parsing timeout [1-65535]");
               Ret_Val = QCLI_STATUS_USAGE_E;
            }
         }
         else
         {
            Timeout = 120;
         }
      }
      else
      {
         Ret_Val = QCLI_STATUS_USAGE_E;
      }

      /* Continue if the parameters parsed out successfully. */
      if(Ret_Val == QCLI_STATUS_SUCCESS_E)
      {
         Result = qc_drv_TWN_Commissioner_Add_Joiner(qc_api_get_qc_drv_context(), Thread_Demo_Context.TWN_Handle, EUI64, Passphrase, Timeout);
         if(Result == QAPI_OK)
         {
            Ret_Val = QCLI_STATUS_SUCCESS_E;
            Display_Function_Success("qapi_TWN_Commissioner_Add_Joiner");
         }
         else
         {
            Result = QAPI_ERROR;
            Ret_Val = QCLI_STATUS_ERROR_E;
            Display_Function_Error("qapi_TWN_Commissioner_Add_Joiner", Result);
         }
      }
   }
   else
   {
      LOG_WARN("TWN not initialized.\n");
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
QCLI_Command_Status_t qc_api_th_MeshCoP_CommissionerDelJoiner(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
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
            LOG_WARN("Error parsing EUI-64.\n");
            Ret_Val = QCLI_STATUS_USAGE_E;
         }
      }
      else
      {
         EUI64 = 0ULL;
      }

      if(Ret_Val == QCLI_STATUS_SUCCESS_E)
      {
         Result = qc_drv_TWN_Commissioner_Remove_Joiner(qc_api_get_qc_drv_context(), Thread_Demo_Context.TWN_Handle, EUI64);
         if(Result == QAPI_OK)
         {
            Ret_Val = QCLI_STATUS_SUCCESS_E;
            Display_Function_Success("qapi_TWN_Commissioner_Remove_Joiner");
         }
         else
         {
            Ret_Val = QCLI_STATUS_ERROR_E;
            Display_Function_Error("qapi_TWN_Commissioner_Remove_Joiner", Result);
         }
      }
   }
   else
   {
      LOG_WARN("TWN not initialized.\n");
      Ret_Val = QCLI_STATUS_ERROR_E;
   }

   return Ret_Val;
}

/**
   @brief Attempts to commission onto an existing Thread network.

   Parameter_List[0] the ASCII string for the joining device's PSKd.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List  is the list of parsed arguments associated with
          this command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_api_th_MeshCoP_JoinerStart(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t            Ret_Val;
   qapi_Status_t                    Result;
   const char                      *Passphrase;
   qapi_TWN_Joiner_Info_t           Joiner_Info;

   /* Verify the TWN layer is initialized. */
   if(Thread_Demo_Context.TWN_Handle != QAPI_TWN_INVALID_HANDLE)
   {
      if(Parameter_Count >= 1)
      {
         Passphrase = Parameter_List[0].String_Value;
         Ret_Val = QCLI_STATUS_SUCCESS_E;
      }
      else
      {
         Ret_Val = QCLI_STATUS_USAGE_E;
      }

      if(Ret_Val == QCLI_STATUS_SUCCESS_E)
      {
         Joiner_Info.PSKd              = Passphrase;
         Joiner_Info.Provisioning_URL  = NULL;
         Joiner_Info.Vendor_Name       = Vendor_Name;
         Joiner_Info.Vendor_Model      = Vendor_Model;
         Joiner_Info.Vendor_Sw_Version = Vendor_SwVer;
         Joiner_Info.Vendor_Data       = Vendor_Data;

         Result = qc_drv_TWN_Joiner_Start(qc_api_get_qc_drv_context(), Thread_Demo_Context.TWN_Handle, &Joiner_Info);
         if(Result == QAPI_OK)
         {
            Ret_Val = QCLI_STATUS_SUCCESS_E;
            Display_Function_Success("qapi_TWN_Joiner_Start");
         }
         else
         {
            Result = QAPI_ERROR;
            Ret_Val = QCLI_STATUS_ERROR_E;
            Display_Function_Error("qapi_TWN_Joiner_Start", Result);
         }
      }
   }
   else
   {
      LOG_WARN("TWN not initialized.\n");
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
QCLI_Command_Status_t qc_api_th_MeshCoP_JoinerStop(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t            Ret_Val;
   qapi_Status_t                    Result;

   /* Verify the TWN layer is initialized. */
   if(Thread_Demo_Context.TWN_Handle != QAPI_TWN_INVALID_HANDLE)
   {
      Result = qc_drv_TWN_Joiner_Stop(qc_api_get_qc_drv_context(), Thread_Demo_Context.TWN_Handle);
      if(Result == QAPI_OK)
      {
         Ret_Val = QCLI_STATUS_SUCCESS_E;
         Display_Function_Success("qapi_TWN_Joiner_Stop");
      }
      else
      {
         Result = QAPI_ERROR;
         Ret_Val = QCLI_STATUS_ERROR_E;
         Display_Function_Error("qapi_TWN_Joiner_Stop", Result);
      }
   }
   else
   {
      LOG_WARN("TWN not initialized.\n");
      Ret_Val = QCLI_STATUS_ERROR_E;
   }

   return Ret_Val;
}

/**
   @brief Add a border router's information to the network data.

   Parameter_List[0] IPv6 prefix of the border router.
   Parameter_List[1] The CIDR bit length of the prefix.
   Parameter_List[2] (0-2) Routing preference of the border router from 0 (low)
                     to 2 (high).
   Parameter_List[3] Is_Stable indicating if the border router information
                     is stable network data.
   Parameter_List[4] Flags with remaining border router configuration.
                     Bit3 indicates if the prefix is preferred for address
                          auto-configuration.
                     Bit4 indicates if the prefix is valid for address
                          auto-configuration.
                     Bit5 indicates if the border router supports DHCPv6 address
                          configuration.
                     Bit6 indicates if the border router supports other DHCPv6
                          configuration.
                     Bit7 indicates if the border router is a default route for
                          its prefix.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List  is the list of parsed arguments associated with
          this command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_api_th_AddBorderRouter(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t    Ret_Val;
   qapi_TWN_Border_Router_t Border_Router_Config;
   qapi_Status_t            Result;

   /* Verify the TWN layer is initialized. */
   if(Thread_Demo_Context.TWN_Handle != QAPI_TWN_INVALID_HANDLE)
   {
      if((Parameter_Count >= 5) &&
         (Verify_Integer_Parameter(&(Parameter_List[1]), 0, 128)) &&
         (Verify_Integer_Parameter(&(Parameter_List[2]), 0, 2)) &&
         (Verify_Integer_Parameter(&(Parameter_List[3]), 0, 1)) &&
         (Parameter_List[4].Integer_Is_Valid))
      {
         /* Parse the IPv6 address. */
         if(inet_pton(AF_INET6, Parameter_List[0].String_Value, &(Border_Router_Config.Prefix.Address)) == 0)
         {
            /* Set up the remaining fields of Border_Router_Config. */
            Border_Router_Config.Prefix.Length = (uint8_t)(Parameter_List[1].Integer_Value);
            Border_Router_Config.Preference    = (qapi_TWN_Routing_Preference_t)(Parameter_List[2].Integer_Value);
            Border_Router_Config.Is_Stable     = (qbool_t)(Parameter_List[3].Integer_Value != 0);
            Border_Router_Config.Flags         = (uint32_t)(Parameter_List[4].Integer_Value);

            Result = qc_drv_TWN_Add_Border_Router(qc_api_get_qc_drv_context(), Thread_Demo_Context.TWN_Handle, &Border_Router_Config);
            if(Result == QAPI_OK)
            {
               Display_Function_Success("qapi_TWN_Add_Border_Router");

               Ret_Val = QCLI_STATUS_SUCCESS_E;
            }
            else
            {
               Display_Function_Error("qapi_TWN_Add_Border_Router", Result);

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
      LOG_WARN("TWN not initialized.\n");
      Ret_Val = QCLI_STATUS_ERROR_E;
   }

   return(Ret_Val);
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
QCLI_Command_Status_t qc_api_th_RemoveBorderRouter(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
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

            Result = qc_drv_TWN_Remove_Border_Router(qc_api_get_qc_drv_context(), Thread_Demo_Context.TWN_Handle, &Prefix_Config);
            if(Result == QAPI_OK)
            {
               Display_Function_Success("qapi_TWN_Remove_Border_Router");

               Ret_Val = QCLI_STATUS_SUCCESS_E;
            }
            else
            {
               Display_Function_Error("qapi_TWN_Remove_Border_Router", Result);

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
      LOG_WARN("TWN not initialized.\n");
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
QCLI_Command_Status_t qc_api_th_AddExternalRoute(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
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

            Result = qc_drv_TWN_Add_External_Route(qc_api_get_qc_drv_context(), Thread_Demo_Context.TWN_Handle, &Route_Config);
            if(Result == QAPI_OK)
            {
               Display_Function_Success("qapi_TWN_Add_External_Route");

               Ret_Val = QCLI_STATUS_SUCCESS_E;
            }
            else
            {
               Display_Function_Error("qapi_TWN_Add_External_Route", Result);

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
      LOG_WARN("TWN not initialized.\n");
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
QCLI_Command_Status_t qc_api_th_RemoveExternalRoute(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
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

            Result = qc_drv_TWN_Remove_External_Route(qc_api_get_qc_drv_context(), Thread_Demo_Context.TWN_Handle, &Prefix_Config);
            if(Result == QAPI_OK)
            {
               Display_Function_Success("qapi_TWN_Remove_External_Route");

               Ret_Val = QCLI_STATUS_SUCCESS_E;
            }
            else
            {
               Display_Function_Error("qapi_TWN_Remove_External_Route", Result);

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
      LOG_WARN("TWN not initialized.\n");
      Ret_Val = QCLI_STATUS_ERROR_E;
   }

   return(Ret_Val);
}

/**
   @brief Registers any pending network data on this device with the Leader.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List  is the list of parsed arguments associated with
          this command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_api_th_RegisterServerData(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t Ret_Val;
   qapi_Status_t         Result;

   /* Verify the TWN layer is initialized. */
   if(Thread_Demo_Context.TWN_Handle != QAPI_TWN_INVALID_HANDLE)
   {
      Result = qc_drv_TWN_Register_Server_Data(qc_api_get_qc_drv_context(), Thread_Demo_Context.TWN_Handle);
      if(Result == QAPI_OK)
      {
         Display_Function_Success("qapi_TWN_Register_Server_Data");

         Ret_Val = QCLI_STATUS_SUCCESS_E;
      }
      else
      {
         Display_Function_Error("qapi_TWN_Register_Server_Data", Result);

         Ret_Val = QCLI_STATUS_ERROR_E;
      }
   }
   else
   {
      LOG_WARN("TWN not initialized.\n");
      Ret_Val = QCLI_STATUS_ERROR_E;
   }

   return(Ret_Val);
}

/**
   @brief Convenience function to set up the default Network Data for this
          demo application.

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List  is the list of parsed arguments associated with
          this command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_api_th_Thread_UseDefaultInfo(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t            Ret_Val;
   qapi_Status_t                    Result;

   /* Verify the TWN layer is initialized. */
   if(Thread_Demo_Context.TWN_Handle != QAPI_TWN_INVALID_HANDLE)
   {
      /* Set the default network information. */
      memscpy(&(Thread_Demo_Context.Network_Configuration), sizeof(qapi_TWN_Network_Configuration_t), &Default_Network_Configuration, sizeof(qapi_TWN_Network_Configuration_t));

      /* Get the network information configured by default. */
      Result = qc_drv_TWN_Set_Network_Configuration(qc_api_get_qc_drv_context(), Thread_Demo_Context.TWN_Handle, &(Thread_Demo_Context.Network_Configuration));

      if(Result == QAPI_OK)
      {
         Result = qc_drv_TWN_Set_PSKc(qc_api_get_qc_drv_context(), Thread_Demo_Context.TWN_Handle, Default_PSKc);
         if(Result != QAPI_OK)
         {
            Display_Function_Error("qapi_TWN_Set_PSKc", Result);
         }

         Ret_Val = QCLI_STATUS_SUCCESS_E;
      }
      else
      {
         Display_Function_Error("qapi_TWN_Set_Network_Configuration", Result);

         Ret_Val = QCLI_STATUS_ERROR_E;
      }
   }
   else
   {
      LOG_WARN("TWN not initialized.\n");
      Ret_Val = QCLI_STATUS_ERROR_E;
   }

   return(Ret_Val);
}

/**
   @brief Starts acting as a Border Agent on a specified WLAN interface.
          Available only on the QCA4020 with active WLAN connection.

   Parameter_List[0] The WLAN interface to use for MDNS (wlan0/wlan1)

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List  is the list of parsed arguments associated with
          this command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_api_th_Thread_StartBorderAgent(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t            Ret_Val;
   qapi_Status_t                    Result;

   /* Verify the TWN layer is initialized. */
   if(Thread_Demo_Context.TWN_Handle != QAPI_TWN_INVALID_HANDLE)
   {
      if(Parameter_Count >= 1)
      {
         Result = qc_drv_TWN_Start_Border_Agent(qc_api_get_qc_drv_context(), Thread_Demo_Context.TWN_Handle, QAPI_TWN_AF_INET, "Quartz Border Router", "thread.local.", Parameter_List[0].String_Value);

         if(Result == QAPI_OK)
         {
            Display_Function_Success("qapi_TWN_Start_Border_Agent");
            Ret_Val = QCLI_STATUS_SUCCESS_E;
         }
         else
         {
            Display_Function_Error("qapi_TWN_Start_Border_Agent", Result);

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
      LOG_WARN("TWN not initialized.\n");
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
QCLI_Command_Status_t qc_api_th_Thread_StopBorderAgent(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t            Ret_Val;
   qapi_Status_t                    Result;

   /* Verify the TWN layer is initialized. */
   if(Thread_Demo_Context.TWN_Handle != QAPI_TWN_INVALID_HANDLE)
   {
      Result = qc_drv_TWN_Stop_Border_Agent(qc_api_get_qc_drv_context(), Thread_Demo_Context.TWN_Handle);

      if(Result == QAPI_OK)
      {
         Display_Function_Success("qapi_TWN_Stop_Border_Agent");
         Ret_Val = QCLI_STATUS_SUCCESS_E;
      }
      else
      {
         Display_Function_Error("qapi_TWN_Stop_Border_Agent", Result);

         Ret_Val = QCLI_STATUS_ERROR_E;
      }
   }
   else
   {
      LOG_WARN("TWN not initialized.\n");
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
QCLI_Command_Status_t qc_api_th_Thread_UpdatePSKc(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
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

         Result = qc_drv_TWN_Get_Network_Configuration(qc_api_get_qc_drv_context(), Thread_Demo_Context.TWN_Handle, &Network_Config);
         if(Result == QAPI_OK)
         {
            /* Print out something because this call takes a while. */
            LOG_DEBUG("Generating PSKc...\r\n");
            Result = qc_drv_TWN_Commissioner_Generate_PSKc(qc_api_get_qc_drv_context(), Thread_Demo_Context.TWN_Handle, Parameter_List[0].String_Value, Network_Config.NetworkName, Network_Config.Extended_PAN_ID, PSKc);
            if(Result == QAPI_OK)
            {
               LOG_DEBUG("PSKc generated!\r\n");
            }
            else
            {
               Display_Function_Error("qapi_TWN_Commissioner_Generate_PSKc", Result);
               Ret_Val = QCLI_STATUS_ERROR_E;
            }
         }
         else
         {
            Display_Function_Error("qapi_TWN_Get_Network_Configuration", Result);
            Ret_Val = QCLI_STATUS_ERROR_E;
         }

         /* Only bother setting the PSKc if we successfully generated it. */
         if(Ret_Val == QCLI_STATUS_SUCCESS_E)
         {
            Result = qc_drv_TWN_Set_PSKc(qc_api_get_qc_drv_context(), Thread_Demo_Context.TWN_Handle, PSKc);
            if(Result == QAPI_OK)
            {
               Display_Function_Success("qapi_TWN_Set_PSKc");
            }
            else
            {
               Display_Function_Error("qapi_TWN_Set_PSKc", Result);
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
      LOG_WARN("TWN not initialized.\n");
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
QCLI_Command_Status_t qc_api_th_Thread_ClearPersist(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t            Ret_Val;
   qapi_Status_t                    Result;

   /* Verify the TWN layer is initialized. */
   if(Thread_Demo_Context.TWN_Handle != QAPI_TWN_INVALID_HANDLE)
   {
      Result = qc_drv_TWN_Clear_Persistent_Data(qc_api_get_qc_drv_context(), Thread_Demo_Context.TWN_Handle);

      if(Result == QAPI_OK)
      {
         Display_Function_Success("qapi_TWN_Clear_Persistent_Data");

         DisplayNetworkInfo();

         Ret_Val = QCLI_STATUS_SUCCESS_E;
      }
      else
      {
         Display_Function_Error("qapi_TWN_Clear_Persistent_Data", Result);

         Ret_Val = QCLI_STATUS_ERROR_E;
      }
   }
   else
   {
      LOG_WARN("TWN not initialized.\n");
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
QCLI_Command_Status_t qc_api_th_Thread_BecomeRouter(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t            Ret_Val;
   qapi_Status_t                    Result;

   /* Verify the TWN layer is initialized. */
   if(Thread_Demo_Context.TWN_Handle != QAPI_TWN_INVALID_HANDLE)
   {
      Result = qc_drv_TWN_Become_Router(qc_api_get_qc_drv_context(), Thread_Demo_Context.TWN_Handle);

      if(Result == QAPI_OK)
      {
         Display_Function_Success("qapi_TWN_Become_Router");

         Ret_Val = QCLI_STATUS_SUCCESS_E;
      }
      else
      {
         Display_Function_Error("qapi_TWN_Become_Router", Result);

         Ret_Val = QCLI_STATUS_ERROR_E;
      }
   }
   else
   {
      LOG_WARN("TWN not initialized.\n");
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
QCLI_Command_Status_t qc_api_th_Thread_BecomeLeader(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t            Ret_Val;
   qapi_Status_t                    Result;

   /* Verify the TWN layer is initialized. */
   if(Thread_Demo_Context.TWN_Handle != QAPI_TWN_INVALID_HANDLE)
   {
      Result = qc_drv_TWN_Become_Leader(qc_api_get_qc_drv_context(), Thread_Demo_Context.TWN_Handle);

      if(Result == QAPI_OK)
      {
         Display_Function_Success("qapi_TWN_Become_Leader");

         Ret_Val = QCLI_STATUS_SUCCESS_E;
      }
      else
      {
         Display_Function_Error("qapi_TWN_Become_Leader", Result);

         Ret_Val = QCLI_STATUS_ERROR_E;
      }
   }
   else
   {
      LOG_WARN("TWN not initialized.\n");
      Ret_Val = QCLI_STATUS_ERROR_E;
   }

   return(Ret_Val);
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
QCLI_Command_Status_t qc_api_th_Thread_SetIPStackInteg(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t            Ret_Val;
   qapi_Status_t                    Result;

   /* Verify the TWN layer is initialized. */
   if(Thread_Demo_Context.TWN_Handle != QAPI_TWN_INVALID_HANDLE)
   {
      if(Parameter_Count >= 1 && Verify_Integer_Parameter(&Parameter_List[0], 0, 1))
      {
         Result = qc_drv_TWN_Set_IP_Stack_Integration(qc_api_get_qc_drv_context(), Thread_Demo_Context.TWN_Handle, Parameter_List[0].Integer_Value);

         if(Result == QAPI_OK)
         {
            Display_Function_Success("qapi_TWN_Set_IP_Stack_Integration");

            Ret_Val = QCLI_STATUS_SUCCESS_E;
         }
         else
         {
            Display_Function_Error("qapi_TWN_Set_IP_Stack_Integration", Result);

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
      LOG_WARN("TWN not initialized.\n");
      Ret_Val = QCLI_STATUS_ERROR_E;
   }

   return(Ret_Val);
}

/**
   @brief Adds a IP to the Thread Network interface.

   Parameter_List[0] The IPv6 address to add to the interface.
   Parameter_List[1] The CIDR bit length of the prefix.
   Parameter_List[2] Whether this address is preferred or not (0/1).

   @param Parameter_Count is number of elements in Parameter_List.
   @param Parameter_List  is the list of parsed arguments associated with
          this command.

   @return
    - QCLI_STATUS_SUCCESS_E indicates the command is executed successfully.
    - QCLI_STATUS_ERROR_E indicates the command is failed to execute.
    - QCLI_STATUS_USAGE_E indicates there is usage error associated with this
      command.
*/
QCLI_Command_Status_t qc_api_th_Thread_AddUnicastAddress(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t     Ret_Val;
   qapi_TWN_IPv6_Prefix_t    Prefix;
   qapi_Status_t             Result;
   qbool_t                   Preferred;

   /* Verify the TWN layer is initialized. */
   if(Thread_Demo_Context.TWN_Handle != QAPI_TWN_INVALID_HANDLE)
   {
      if((Parameter_Count >= 3) &&
         Verify_Integer_Parameter(&(Parameter_List[1]), 0, 128) &&
         Verify_Integer_Parameter(&(Parameter_List[2]), 0, 1))
      {
         /* Parse the IPv6 address. */
         if(inet_pton(AF_INET6, Parameter_List[0].String_Value, &(Prefix.Address)) == 0)
         {
            Prefix.Length = (uint8_t)(Parameter_List[1].Integer_Value);
            Preferred = (qbool_t)Parameter_List[2].Integer_Value;

            Result = qc_drv_TWN_IPv6_Add_Unicast_Address(qc_api_get_qc_drv_context(), Thread_Demo_Context.TWN_Handle, &Prefix, Preferred);
            if(Result == QAPI_OK)
            {
               Display_Function_Success("qapi_TWN_IPv6_Add_Unicast_Address");

               Ret_Val = QCLI_STATUS_SUCCESS_E;
            }
            else
            {
               Display_Function_Error("qapi_TWN_IPv6_Add_Unicast_Address", Result);

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
      LOG_WARN("TWN not initialized.\n");
      Ret_Val = QCLI_STATUS_ERROR_E;
   }

   return(Ret_Val);
}

/**
   @brief Removes a IP from the Thread Network interface.

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
QCLI_Command_Status_t qc_api_th_Thread_RemoveUnicastAddress(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
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
            Result = qc_drv_TWN_IPv6_Remove_Unicast_Address(qc_api_get_qc_drv_context(), Thread_Demo_Context.TWN_Handle, &Address);
            if(Result == QAPI_OK)
            {
               Display_Function_Success("qapi_TWN_IPv6_Remove_Unicast_Address");

               Ret_Val = QCLI_STATUS_SUCCESS_E;
            }
            else
            {
               Display_Function_Error("qapi_TWN_IPv6_Remove_Unicast_Address", Result);

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
      LOG_WARN("TWN not initialized.\n");
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
QCLI_Command_Status_t qc_api_th_Thread_SubscribeMulticast(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
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
            Result = qc_drv_TWN_IPv6_Subscribe_Multicast_Address(qc_api_get_qc_drv_context(), Thread_Demo_Context.TWN_Handle, &Address);
            if(Result == QAPI_OK)
            {
               Display_Function_Success("qapi_TWN_IPv6_Subscribe_Multicast_Address");

               Ret_Val = QCLI_STATUS_SUCCESS_E;
            }
            else
            {
               Display_Function_Error("qapi_TWN_IPv6_Subscribe_Multicast_Address", Result);

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
      LOG_WARN("TWN not initialized.\n");
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
QCLI_Command_Status_t qc_api_th_Thread_UnsubscribeMulticast(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
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
            Result = qc_drv_TWN_IPv6_Unsubscribe_Multicast_Address(qc_api_get_qc_drv_context(), Thread_Demo_Context.TWN_Handle, &Address);
            if(Result == QAPI_OK)
            {
               Display_Function_Success("qapi_TWN_IPv6_Unsubscribe_Multicast_Address");

               Ret_Val = QCLI_STATUS_SUCCESS_E;
            }
            else
            {
               Display_Function_Error("qapi_TWN_IPv6_Unsubscribe_Multicast_Address", Result);

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
      LOG_WARN("TWN not initialized.\n");
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
QCLI_Command_Status_t qc_api_th_Thread_SetPingEnabled(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t            Ret_Val;
   qapi_Status_t                    Result;

   /* Verify the TWN layer is initialized. */
   if(Thread_Demo_Context.TWN_Handle != QAPI_TWN_INVALID_HANDLE)
   {
      if(Parameter_Count >= 1 && Verify_Integer_Parameter(&Parameter_List[0], 0, 1))
      {
         Result = qc_drv_TWN_Set_Ping_Response_Enabled(qc_api_get_qc_drv_context(), Thread_Demo_Context.TWN_Handle, Parameter_List[0].Integer_Value);

         if(Result == QAPI_OK)
         {
            Display_Function_Success("qapi_TWN_Set_Ping_Response_Enabled");

            Ret_Val = QCLI_STATUS_SUCCESS_E;
         }
         else
         {
            Display_Function_Error("qapi_TWN_Set_Ping_Response_Enabled", Result);

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
      LOG_WARN("TWN not initialized.\n");
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
QCLI_Command_Status_t qc_api_th_Thread_SetProvisioningUrl(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t            Ret_Val;
   qapi_Status_t                    Result;

   /* Verify the TWN layer is initialized. */
   if(Thread_Demo_Context.TWN_Handle != QAPI_TWN_INVALID_HANDLE)
   {
      if(Parameter_Count >= 1)
      {
         Result = qc_drv_TWN_Commissioner_Set_Provisioning_URL(qc_api_get_qc_drv_context(), Thread_Demo_Context.TWN_Handle, Parameter_List[0].String_Value);

         if(Result == QAPI_OK)
         {
            Display_Function_Success("qapi_TWN_Commissioner_Set_Provisioning_URL");

            Ret_Val = QCLI_STATUS_SUCCESS_E;
         }
         else
         {
            Display_Function_Error("qapi_TWN_Commissioner_Set_Provisioning_URL", Result);

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
      LOG_WARN("TWN not initialized.\n");
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
QCLI_Command_Status_t qc_api_th_Thread_SetLogging(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t Ret_Val;

   if(Parameter_Count >= 1 && Verify_Integer_Parameter(&Parameter_List[0], 0, 1))
   {
      Thread_Demo_Context.EnableLogging = (qbool_t)Parameter_List[0].Integer_Value;
      LOG_INFO("Thread logging %s.\n", Thread_Demo_Context.EnableLogging ? "enabled" : "disabled");
      Ret_Val = QCLI_STATUS_SUCCESS_E;
   }
   else
   {
      Ret_Val = QCLI_STATUS_USAGE_E;
   }

   return(Ret_Val);
}

char otLogBuf[112];

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

      LOG_INFO("%s\n", otLogBuf);
   }
}

#endif
