#ifndef __QAPI_JSON_BASE_H__
#define __QAPI_JSON_BASE_H__

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
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "qapi_status.h"

/**
@file qapi_json.h
This section provides APIs, enumerations and data structures for applications to encode, decode, search, modify and delete JSON items.
*/

/**
@ingroup qapi_json  
Converts a given JSON formatted string into a JSON object and returns a handle for the object.

This handle is to be used for further operations, such as query, addition, deletion, and
replacement of items.

Applications are responsible for deleting the created objects. A failure to do so causes 
a memory leak. Use the API qapi_JSON_Delete_Object() for this purpose.


@param[in] input_String    JSON formatted string.
@param[in] handle          Pointer to the object handle returned to the application.

@sa
qapi_JSON_Delete_Object()\n
qapi_JSON_Get_Handle_List()

@return 
QAPI_OK -- JSON object creation succeeded.\n
Nonzero value -- JSON object creation failed.
*/

qapi_Status_t qapi_JSON_Decode(const char* input_String, uint32_t* handle);

/**
@ingroup qapi_json  
Converts a given JSON object into a JSON formatted string.

Applications must pass in the handle for the object to be converted and a pointer to retrieve 
the converted string. Applications are responsible for freeing the memory allocated for a new 
string. Standard library "free" functions can be used for this purpose.

@param[in] handle          Handle for the JSON object.
@param[out] output_String   Pointer to where to retrieve the JSON formatted string.

@return 
QAPI_OK -- JSON object was sucessfully converted into a string. \n
Nonzero value -- JSON object could not be converted to a string.
*/
qapi_Status_t qapi_JSON_Encode(uint32_t handle, char** output_String);

/**
@ingroup qapi_json  
Queries the specified key in a specified object and returns the value.

In the case of duplicate keys, this API returns the first occurence.

The application must pass in a pointer to retrieve the value.

@param[in] handle          Handle for the JSON object.
@param[in] query_Key       Key to search for in the specified object.
@param[out] output_String   Item value for the given key, if found.

@return
QAPI_OK -- Specified key was found. \n
Nonzero value -- Key was not found.
*/
qapi_Status_t qapi_JSON_Query_By_Key(uint32_t handle, const char* query_Key, char** output_String);

/**
@ingroup qapi_json
Gets the value of an array item at a given index.\n
Applications must pass a pointer to retrieve the item's value.

@param[in] handle          Handle for the JSON object.
@param[in] index           Index at which the new item is to be inserted.
@param[out] output_String  Item value at the given index, if found.

@return
QAPI_OK -- Item was found at the given index. \n
Nonzero value -- Item was not found.
*/
qapi_Status_t qapi_JSON_Query_By_Index(uint32_t handle, uint32_t index, char** output_String);

/**
@ingroup qapi_json
Inserts a new item at the given index in the given array.

Applications must pass a JSON formatted string for the value.

@param[in] handle          Handle for the JSON object.
@param[in] index           Index at which the new item is to be inserted.
@param[in] value           Value for new item to be added.

@return
QAPI_OK -- Specified item was deleted.\n
Nonzero value -- Item with the specified key was not found.
*/
qapi_Status_t qapi_JSON_Insert_Value_By_Index(uint32_t handle, uint32_t index, const char* value);

/**
@ingroup qapi_json
Inserts a new item at a given index in the object.

Applications must pass a JSON formatted string for the value. The key does not need 
formatting.

@param[in] handle          Handle for the JSON object.
@param[in] index           Index at which the new item is to be inserted.
@param[in] key             Key for the new item to be added.
@param[in] value           Value for the new item to be added.

@return
QAPI_OK -- Specified item was inserted.\n
Nonzero value -- Item with the specified key was not found.
*/
qapi_Status_t qapi_JSON_Insert_KeyValue_By_Index(uint32_t handle, uint32_t index, const char* key, const char* value);

/**
@ingroup qapi_json
Replaces the item at a given index in the given array.

@param[in] handle          Handle for the JSON object.
@param[in] index           Index of the item to be replaced.
@param[in] new_Value       New value for the specified key.

@return
QAPI_OK -- Specified item was replaced.\n
Nonzero value -- Item at the given index was not found.
*/
qapi_Status_t qapi_JSON_Replace_Value_By_Index(uint32_t handle, uint32_t index, const char* new_Value);

/**
@ingroup qapi_json
Replaces the value of a specified key with the given new value in the specified JSON object.
In the case of duplicate keys, this API deletes the first item.

@param[in] handle          Handle for the JSON object.
@param[in] query_Key       Key to search for in the specified object.
@param[in] new_Value       New value for the specified key.

@return
QAPI_OK -- Specified item was replaced.\n
Nonzero value -- Item with the specified key was not found.
*/
qapi_Status_t qapi_JSON_Replace_Value_By_Key(uint32_t handle, const char* query_Key, const char* new_Value);

/**
@ingroup qapi_json
Deletes an item at a given index in an array.

@param[in] handle          Handle for the JSON object.
@param[in] index           Index of the item to be deleted.

@return
QAPI_OK -- Specified item was deleted.\n
Nonzero value -- Item at the given index was not found.
*/
qapi_Status_t qapi_JSON_Delete_Entry_By_Index(uint32_t handle, uint32_t index);

/**
@ingroup qapi_json
Deletes an item with a specified key from the JSON object tree.

In the case of duplicate keys, this API deletes the first item.

@param[in] handle          Handle for the JSON object.
@param[in] query_Key       Key to search for in the specified object.

@return
QAPI_OK -- Specified item was deleted.\n
Nonzero value -- Item with the specified key was not found.
*/
qapi_Status_t qapi_JSON_Delete_Entry_By_Key(uint32_t handle, const char* query_Key);

/**
@ingroup qapi_json
Deletes a complete json object tree.
 
Applications should use the API to delete every object tree created using qapi_JSON_Decode() to free the allocated memory.

@param[in] handle          Handle for the JSON object for deleting individual objects.
@param[in] all             If this parameter is set to 1, the API deletes all created object trees. Otherwise, if set to 0, only the tree with specified handle is deleted.

@return
QAPI_OK -- Object was deleted.\n
Nonzero value -- Object was not found.
*/
qapi_Status_t qapi_JSON_Delete_Object(uint32_t handle, uint32_t all);


/**
@ingroup qapi_json
Gets a list of all created objects.

@param[in] list    Pointer to where to retrieve the list of created objects..
@param[in] size    Pointer to the size of the list of created objects.

@return
QAPI_OK -- Success in getting the list of created objects.\n
Nonzero value -- Error in getting the list of created objects.
*/
qapi_Status_t qapi_JSON_Get_Handle_List(uint32_t **list, uint32_t *size);

#endif // __QAPI_JSON_H__
