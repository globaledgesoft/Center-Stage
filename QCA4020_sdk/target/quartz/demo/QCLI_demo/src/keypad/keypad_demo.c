/*
 * Copyright (c) 2015-2018 Qualcomm Technologies, Inc.
 * 2015-2016 Qualcomm Atheros, Inc.
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <qcli.h>
#include <qcli_api.h>
#include <qapi_wlan.h>

#include "keypad_demo.h"

extern QCLI_Group_Handle_t qcli_peripherals_group;              /* Handle for our peripherals subgroup. */

QCLI_Group_Handle_t qcli_keybrd_group;              /* Handle for our QCLI Command Group. */

QCLI_Command_Status_t keybrd_qapi_test(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t keybrd_qapi_test_quit(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);

const QCLI_Command_t keybrd_cmd_list[] =
{
   { keybrd_qapi_test,         true,      "keypad",            "",               "keypad"   },
   { keybrd_qapi_test_quit,    false,     "Quit",              "",               "Quit"   },
};

const QCLI_Command_Group_t keybrd_cmd_group =
{
    "Keypad",
    (sizeof(keybrd_cmd_list) / sizeof(keybrd_cmd_list[0])),
    keybrd_cmd_list
};

   /* This function is used to register the keypad Command Group with    */
   /* QCLI.                                                             */
void Initialize_Keypad_Demo(void)
{
   /* Attempt to reqister the Command Groups with the qcli framework.*/
   qcli_keybrd_group = QCLI_Register_Command_Group(qcli_peripherals_group, &keybrd_cmd_group);
   if(qcli_keybrd_group)
   {
      QCLI_Printf(qcli_keybrd_group, "Keypad Registered \n");
   }
}

QCLI_Command_Status_t keybrd_qapi_test(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	int32_t keyboard_driver_qapi_test( uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List );
    int32_t  result;
    
    result = keyboard_driver_qapi_test(Parameter_Count, Parameter_List);
    if (result != 0)
    {
      	QCLI_Printf(qcli_keybrd_group, "Keypad fails\n");
        return QCLI_STATUS_ERROR_E;
    }
    return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t keybrd_qapi_test_quit(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	int32_t keypad_test_quit( uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List );
    int32_t  result;
    
    result = keypad_test_quit(Parameter_Count, Parameter_List);
    if (result != 0)
    {
      	QCLI_Printf(qcli_keybrd_group, "keypad fails\n");
        return QCLI_STATUS_ERROR_E;
    }
    return QCLI_STATUS_SUCCESS_E;
}



