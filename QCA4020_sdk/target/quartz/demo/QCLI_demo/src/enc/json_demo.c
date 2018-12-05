/*
 * Copyright (c) 2017 Qualcomm Technologies, Inc.
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
#include <qcli_api.h>
#include "util.h"
#include "json_demo.h"
#include "qapi_json.h"


QCLI_Group_Handle_t qcli_json_group;              /* Handle for our QCLI Command Group. */

QCLI_Command_Status_t CreateObject(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t GetJsonString(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t Query(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t InsertValueByIndex(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t InsertKeyValueByIndex(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t ReplaceValueByIndex(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t ReplaceValueByKey(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t DeleteEntryByIndex(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t DeleteEntryByKey(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t DeleteObject(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t GetHandleList(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t DeleteAllObjects(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);

const QCLI_Command_t json_cmd_list[] =
{
{ CreateObject,          false, "CreateObject",           "<json formatted string>",   "Convert string to json object." },
{ GetJsonString,         false, "GetJsonString",          "<object handle>",          "Convert JSON object to a string." },
{ Query,                 false, "Query",                  "<object handle> <key|index>",    "Print value of specified key or value at specified index." },
{ InsertValueByIndex,    false, "InsertValueByIndex",     "<object handle> <index> <new_item_string>", "Insert new item in specified array at specified index." },
{ InsertKeyValueByIndex, false, "InsertKeyValueByIndex",  "<object handle> <index> <key> <new_item_string>", "Insert new item in specified object at specified index." },
{ ReplaceValueByIndex,   false, "ReplaceValueByIndex",    "<object handle> <index> <new_value>",  "Replace the item at given index." },
{ ReplaceValueByKey,     false, "ReplaceValueByKey",      "<object handle> <key> <new_value>",  "Replace the value for specified key in specified object." },
{ DeleteEntryByIndex,    false, "DeleteEntryByIndex",     "<object handle> <index>",   "Delete item at given index." },
{ DeleteEntryByKey,      false, "DeleteEntryByKey",       "<object handle> <key>",  "Delete item with specified key." },
{ DeleteObject,          false, "DeleteTree",             "<object handle>",    "Free the specified JSON object tree." },
{ GetHandleList,         false, "GetHandleList",          "",	                  "Get list of all objects created." },
{ DeleteAllObjects,	     false, "DeleteAllObjects",       "",                   "Delete all created objects trees." },
};


const QCLI_Command_Group_t json_cmd_group =
{
    "JSON",
    (sizeof(json_cmd_list) / sizeof(json_cmd_list[0])),
    json_cmd_list
};

/* This function is used to register JSON Command Group with QCLI. */
void Initialize_JSON_Demo(void)
{
    /* Attempt to reqister the Command Groups with the qcli framework.*/
    qcli_json_group = QCLI_Register_Command_Group(NULL, &json_cmd_group);
    if(qcli_json_group)
    {
        QCLI_Printf(qcli_json_group, "JSON Demo Registered.\r\n");
    }
}

/* This function converts JSON formatted string into an object. */
QCLI_Command_Status_t CreateObject(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    uint32_t handle = 0;

    if(Parameter_Count != 1)
    {
        return QCLI_STATUS_USAGE_E;
    }

    if (0 != qapi_JSON_Decode((const char *)Parameter_List[0].String_Value, &handle))
    {
        QCLI_Printf(qcli_json_group, "Error in creating json opbject.\r\n");
        return QCLI_STATUS_ERROR_E; 
    }

    QCLI_Printf(qcli_json_group, "Json object handle: %d.\r\n", handle);
    return QCLI_STATUS_SUCCESS_E;
}

/* This function converts JSON object into a JSON formatted string. */
QCLI_Command_Status_t GetJsonString(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    char *jsonString = NULL;

    if((Parameter_Count != 1) || !Parameter_List[0].Integer_Is_Valid || (Parameter_List[0].Integer_Value < 0))
    {
        return QCLI_STATUS_USAGE_E;
    }

    if (0 != qapi_JSON_Encode(Parameter_List[0].Integer_Value, &jsonString))
    {
        QCLI_Printf(qcli_json_group, "Invalid JSON object handle.\r\n");
        return QCLI_STATUS_ERROR_E;
    }

    QCLI_Printf(qcli_json_group, "\n%s\r\n",jsonString);
    free(jsonString);
    return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t Query(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    char *jsonString = NULL;

    if(Parameter_Count != 2 || !Parameter_List[0].Integer_Is_Valid || (Parameter_List[0].Integer_Value < 0))
    {
        return QCLI_STATUS_USAGE_E;
    }

    if (Parameter_List[1].Integer_Is_Valid && (Parameter_List[1].Integer_Value >= 0))
	{
        if (0 != qapi_JSON_Query_By_Index(Parameter_List[0].Integer_Value,
                                          Parameter_List[1].Integer_Value,
                                          &jsonString))
        {
            QCLI_Printf(qcli_json_group, "Invalid inputs.\r\n");
            return QCLI_STATUS_ERROR_E;
        }
    }
	else {
        if (0 != qapi_JSON_Query_By_Key(Parameter_List[0].Integer_Value, Parameter_List[1].String_Value, &jsonString))
        {
            QCLI_Printf(qcli_json_group, "Query string not found.\r\n");
            return QCLI_STATUS_ERROR_E;
        }
    }

    QCLI_Printf(qcli_json_group, "Json query value: %s\r\n",jsonString);
    free(jsonString);
    return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t InsertValueByIndex(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if(Parameter_Count != 3 ||
       !Parameter_List[0].Integer_Is_Valid || (Parameter_List[0].Integer_Value < 0) ||
       !Parameter_List[1].Integer_Is_Valid || (Parameter_List[1].Integer_Value < 0))
    {
        return QCLI_STATUS_USAGE_E;
    }

    if (0 != qapi_JSON_Insert_Value_By_Index(Parameter_List[0].Integer_Value,
                                         Parameter_List[1].Integer_Value,
                                         (const char* )Parameter_List[2].String_Value))
    {
        QCLI_Printf(qcli_json_group, "Json operation failed.\r\n");
        return QCLI_STATUS_ERROR_E;
    }
    return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t InsertKeyValueByIndex(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if(Parameter_Count != 4 ||
       !Parameter_List[0].Integer_Is_Valid || (Parameter_List[0].Integer_Value < 0) ||
       !Parameter_List[1].Integer_Is_Valid || (Parameter_List[1].Integer_Value < 0))
    {
        return QCLI_STATUS_USAGE_E;
    }

    if (0 != qapi_JSON_Insert_KeyValue_By_Index(Parameter_List[0].Integer_Value,
                                          Parameter_List[1].Integer_Value,
                                          (const char*)Parameter_List[2].String_Value,
                                          (const char*)Parameter_List[3].String_Value))
    {
        QCLI_Printf(qcli_json_group, "Json operation failed.\r\n");
        return QCLI_STATUS_ERROR_E;
    }
    return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t ReplaceValueByIndex(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if(Parameter_Count != 3 ||
       !Parameter_List[0].Integer_Is_Valid || (Parameter_List[0].Integer_Value < 0) ||
       !Parameter_List[1].Integer_Is_Valid || (Parameter_List[1].Integer_Value < 0))
    {
        return QCLI_STATUS_USAGE_E;
    }

    if (0 != qapi_JSON_Replace_Value_By_Index(Parameter_List[0].Integer_Value,
                                          Parameter_List[1].Integer_Value,
                                          Parameter_List[2].String_Value))
    {
        QCLI_Printf(qcli_json_group, "Json operation failed.\r\n");
        return QCLI_STATUS_ERROR_E;
    }

    return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t ReplaceValueByKey(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if(Parameter_Count != 3 || !Parameter_List[0].Integer_Is_Valid || (Parameter_List[0].Integer_Value < 0))
    {
        return QCLI_STATUS_USAGE_E;
    }

    if (0 != qapi_JSON_Replace_Value_By_Key(Parameter_List[0].Integer_Value,
                                           Parameter_List[1].String_Value,
                                           Parameter_List[2].String_Value))
    {
        QCLI_Printf(qcli_json_group, "Json operation failed.\r\n");
        return QCLI_STATUS_ERROR_E;
    }

    return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t DeleteEntryByIndex(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if(Parameter_Count != 2 ||
       !Parameter_List[0].Integer_Is_Valid || (Parameter_List[0].Integer_Value < 0) ||
       !Parameter_List[1].Integer_Is_Valid || (Parameter_List[1].Integer_Value < 0))
    {
        return QCLI_STATUS_USAGE_E;
    }

    if (0 != qapi_JSON_Delete_Entry_By_Index(Parameter_List[0].Integer_Value,
                                         Parameter_List[1].Integer_Value))
    {
        QCLI_Printf(qcli_json_group, "Json operation failed.\r\n");
        return QCLI_STATUS_ERROR_E;
    }

    return QAPI_OK;
}

QCLI_Command_Status_t DeleteEntryByKey(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if(Parameter_Count != 2 || !Parameter_List[0].Integer_Is_Valid || (Parameter_List[0].Integer_Value < 0))
    {
        return QCLI_STATUS_USAGE_E;
    }

    if (0 != qapi_JSON_Delete_Entry_By_Key(Parameter_List[0].Integer_Value,
                                          Parameter_List[1].String_Value))
    {
        QCLI_Printf(qcli_json_group, "Json operation failed.\r\n");
        return QCLI_STATUS_ERROR_E;
    }

    return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t DeleteObject(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if(Parameter_Count != 1 || !Parameter_List[0].Integer_Is_Valid || (Parameter_List[0].Integer_Value < 0))
    {
        return QCLI_STATUS_USAGE_E;
    }

    if (0 != qapi_JSON_Delete_Object(Parameter_List[0].Integer_Value, 0))
    {
        QCLI_Printf(qcli_json_group, "JSON object does not exist.\r\n");
        return QCLI_STATUS_ERROR_E;
    }

    QCLI_Printf(qcli_json_group, "JSON object corresponding to handle %d freed.\r\n", Parameter_List[0].Integer_Value);
    return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t GetHandleList(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    uint32_t size = 0, i = 0, *list = NULL;
    if (0 != qapi_JSON_Get_Handle_List(&list, &size))
    {
        QCLI_Printf(qcli_json_group, "Operation failed.\r\n");
        return QCLI_STATUS_ERROR_E;
    }

    if (!size)
    {
        QCLI_Printf(qcli_json_group, "No objects created.\r\n");
        return QCLI_STATUS_SUCCESS_E;
    }

    QCLI_Printf(qcli_json_group, "Handles:\r\n");
    for (i = 0; i < size; i++)
   	{
        QCLI_Printf(qcli_json_group, "%d, ", list[i]);
    }

    free(list);
    list = NULL;
    size = 0;
    i = 0;
    return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t DeleteAllObjects(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    qapi_JSON_Delete_Object(0, 1);
    return QCLI_STATUS_SUCCESS_E;
}
