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

 
#include "string.h"
#include "stringl.h"

#include "qapi_types.h"
#include "qapi_status.h"
#include "qapi_firmware_upgrade.h"
#include "qapi_fs.h"
#include "qapi_pmu.h"
#include "qurt_mutex.h"

#include "malloc.h"
#include "qurt_error.h"
#include "qurt_mutex.h"
#include "qurt_signal.h"
#include "qurt_thread.h"
#include "qurt_types.h"
#include "string.h"
#include "pal.h"

#include "qcli.h"
#include "qcli_api.h"
#include "lp_demo.h"
#include "om_lp_test.h"

/*-------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 *-----------------------------------------------------------------------*/

#define LP_PRINTF_HANDLE  qcli_LP_group

/*-------------------------------------------------------------------------
 * Type Declarations
 *-----------------------------------------------------------------------*/
 
QCLI_Group_Handle_t qcli_LP_group;              /* Handle for our QCLI Command Group. */


/*-------------------------------------------------------------------------
 * Function Declarations
 *-----------------------------------------------------------------------*/
static QCLI_Command_Status_t Cmd_Reset_OM_Test_Vectors(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t Cmd_Start_OM_Transition_tests(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t Cmd_Deep_Sleep(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t Cmd_Uart(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);

/* The following is the complete command list for the LP demo. */
const QCLI_Command_t LP_Command_List[] =
{
   {Cmd_Deep_Sleep, false, "DeepSleep", "<1: enable| 0: disable>", "Enable/Disable Deep Sleep"},
   {Cmd_Reset_OM_Test_Vectors, false, "ResetOMTests", "", "Reset OM Test Vectors"},
   {Cmd_Start_OM_Transition_tests, false, "RunOMTests", "", "Start or continue OM transition tests\r\n"},
   {Cmd_Uart, false, "UartDisable", "", "Disable UART"},
}; 
 
const QCLI_Command_Group_t LP_Command_Group = 
{
    "LP", 
    sizeof(LP_Command_List) / sizeof(QCLI_Command_t), 
    LP_Command_List,
};

/*-------------------------------------------------------------------------
 * Function Definitions
 *-----------------------------------------------------------------------*/
 
/* This function is used to register the LP Command Group with QCLI   */
void Initialize_LP_Demo(void)
{
   /* Attempt to reqister the Command Groups with the qcli framework.
   */
   LP_PRINTF_HANDLE = QCLI_Register_Command_Group(NULL, &LP_Command_Group);
   if(LP_PRINTF_HANDLE)
   {
      QCLI_Printf("LP Registered \n");
   }

   /* Register the operating modes - specific for FOM
   */
   fom_register_operating_modes();
}

/*=================================================================================================*/


/**
   @brief This function processes the "FWD" command from the CLI.

*/

static QCLI_Command_Status_t Cmd_Reset_OM_Test_Vectors(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t Ret_Val = QCLI_STATUS_SUCCESS_E;

   QCLI_Printf("Resetting OM Test Vectors\r\n");

   if( reset_OM_Test_Vectors() == false )
   {
       Ret_Val = QCLI_STATUS_ERROR_E;
   }

   return(Ret_Val);
}


/**
   @brief This function processes the "Start OM tests" command 
          from the CLI.

*/
static QCLI_Command_Status_t Cmd_Start_OM_Transition_tests(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t Ret_Val;

   Ret_Val = QCLI_STATUS_SUCCESS_E;
   QCLI_Printf("Starting OM transition test\r\n");
   
   /*Set timer value for mode transition*/
   if(Parameter_Count > 0)
       timer_set_time[0] = timer_set_time[1] = Parameter_List->Integer_Value;
   
   om_transition_test();
   return Ret_Val;
}



static QCLI_Command_Status_t Cmd_Deep_Sleep(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   if(Parameter_Count < 1){
	   QCLI_Printf("Missing params [0 : Disable, 1 : Enable\r\n");
	   return QCLI_STATUS_ERROR_E;
   }
   	   
   QCLI_Printf("Setting Sleep %d\r\n",Parameter_List->Integer_Value);
   
   return(set_sleep(Parameter_List->Integer_Value));  
}


static QCLI_Command_Status_t Cmd_Uart(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   if(Parameter_Count < 1){
	   QCLI_Printf("Missing params [Duration in ms] \r\n");
	   return QCLI_STATUS_ERROR_E;
   }
   	   
   QCLI_Printf("UART disabled for %d ms\r\n",Parameter_List->Integer_Value);
   
   return(disable_uart(Parameter_List->Integer_Value));  
}

/****************************************************************************************/



