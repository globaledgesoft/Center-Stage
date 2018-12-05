/*
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
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

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "qcli_api.h"
#include "qapi_status.h"
#include "qapi_fs.h"
#include "qapi_crypto.h"
#include "qapi_securefs.h"
#include "securefs_demo.h"



#ifndef MIN
   #define  MIN( x, y ) ( ((x) < (y)) ? (x) : (y) )
#endif

#ifndef off_t
   #define off_t int32_t
#endif

#define USER_PASSWORD_SIZE    16

typedef struct g_securefs_demo_test_s {
    off_t g_securefs_demo_off_t;
    size_t g_securefs_demo_size_t;
} g_securefs_demo_test_t;

g_securefs_demo_test_t g_securefs_demo_test;

/* 
 * This file contains the command handlers for secure storage management operations
 * on non-volatile memory like list, delete, read, write
 * 
 */

QCLI_Group_Handle_t qcli_securefs_handle; /* Handle for Fs Command Group. */


#define SECUREFS_DEMO_PRINTF(...) QCLI_Printf(qcli_securefs_handle, __VA_ARGS__)


#define hex_to_dec_nibble(hex_nibble) ( (hex_nibble >= 'a') ? (hex_nibble-'a'+10) : ((hex_nibble >= 'A') ? (hex_nibble-'A'+10) : (hex_nibble-'0')) )


void * g_securefs_demo_ctxt;


QCLI_Command_Status_t securefs_demo_ls(uint32_t parameters_count, QCLI_Parameter_t * parameters);
QCLI_Command_Status_t securefs_demo_rm(uint32_t parameters_count, QCLI_Parameter_t * parameters);
QCLI_Command_Status_t securefs_demo_open(uint32_t parameters_count, QCLI_Parameter_t * parameters);
QCLI_Command_Status_t securefs_demo_seek(uint32_t parameters_count, QCLI_Parameter_t * parameters);
QCLI_Command_Status_t securefs_demo_tell(uint32_t parameters_count, QCLI_Parameter_t * parameters);
QCLI_Command_Status_t securefs_demo_read(uint32_t parameters_count, QCLI_Parameter_t * parameters);
QCLI_Command_Status_t securefs_demo_write(uint32_t parameters_count, QCLI_Parameter_t * parameters);
QCLI_Command_Status_t securefs_demo_close(uint32_t parameters_count, QCLI_Parameter_t * parameters);
QCLI_Command_Status_t securefs_demo_run_unittests(uint32_t parameters_count, QCLI_Parameter_t * parameters);


const QCLI_Command_t securefs_cmd_list[] =
{
    {securefs_demo_ls, false, "ls", "password_in_hex\n", "lists valid securefs files encrypted with password_in_hex\n"},
    {securefs_demo_rm, false, "rm", "filename\n", "deletes filename\n"},
    {securefs_demo_open, false, "open", "filename password_in_hex oflags\n", "opens file using password and specified oflags\n"},
    {securefs_demo_seek, false, "seek", "offset whence\n", "change file offset for the opened file.\nWhence:\n\t0 - set to offset\n\t1 - set to offset + current position\n\t2 - set to offset + file_size\n"},
    {securefs_demo_tell, false, "tell", "\n", "prints current offset of the opened file\n"},
    {securefs_demo_read, false, "read", "length\n", "reads length bytes of data from the opened file and prints it as hex string\n"},
    {securefs_demo_write, false, "write", "hex_data\n", "writing hex_data into opened file\n"},
    {securefs_demo_close, false, "close", "\n", "closes opened file\n"},
    {securefs_demo_run_unittests, false, "run_unittests", "number_of_unittests_to_run\n", "Executes number_of_unittests_to_run random SecureFs unittests\n"},

};

const QCLI_Command_Group_t securefs_cmd_group =
{
    "SecureFs",              /* Group_String: will display cmd prompt as "SecureFs> " */
    sizeof(securefs_cmd_list)/sizeof(securefs_cmd_list[0]),   /* Command_Count */
    securefs_cmd_list        /* Command_List */
};


/*****************************************************************************
 * This function is used to register the Fs Command Group with QCLI.
 *****************************************************************************/
void Initialize_SecureFs_Demo(void)
{
    /* Attempt to reqister the Command Groups with the qcli framework.*/
    qcli_securefs_handle = QCLI_Register_Command_Group(NULL, &securefs_cmd_group);
    if (qcli_securefs_handle)
    {
        QCLI_Printf(qcli_securefs_handle, "SecureFs Registered\n");
    }

    return;
}


int convert_data_in_hex_to_byte_array(const char * data_in_hex, uint8_t * data_as_byte_array, uint32_t data_as_byte_array_size)
{
    const uint32_t data_in_hex_length = strlen(data_in_hex);
    if ( (2*data_as_byte_array_size) != data_in_hex_length ) {
        return -1;
    }
    int i;
    for ( i =0; i < data_as_byte_array_size; i++ ) {
        data_as_byte_array[i] = (hex_to_dec_nibble(data_in_hex[2*i]) << 4) | hex_to_dec_nibble(data_in_hex[2*i+1]);
    }

    return 0;
}

QCLI_Command_Status_t securefs_demo_ls(uint32_t parameters_count, QCLI_Parameter_t * parameters)
{
    uint8_t user_password[USER_PASSWORD_SIZE];

    if ( parameters_count < 1 ) {
        SECUREFS_DEMO_PRINTF("Invalid number of parameters\r\n");
        goto securefs_demo_ls_on_error;
    }


    char * password_in_hex = parameters[0].String_Value;
    int status_code = convert_data_in_hex_to_byte_array(password_in_hex, user_password, USER_PASSWORD_SIZE);
    if ( 0 != status_code ) {
        SECUREFS_DEMO_PRINTF("Invalid password_in_hex, must be exactly 32 hex chars\r\n");
        goto securefs_demo_ls_on_error;
    }


    qapi_fs_iter_handle_t iter_handle;
    qapi_Status_t status = qapi_Fs_Iter_Open("/spinor/", &iter_handle);
    if ( QAPI_OK != status ) {
        SECUREFS_DEMO_PRINTF("Failed on a call to qapi_Fs_Iter_Open()\r\n");
        goto securefs_demo_ls_on_error;
    }

    struct qapi_fs_iter_entry file_info;

    do {
        status = qapi_Fs_Iter_Next(iter_handle, &file_info);
        if ( QAPI_OK == status ) {
            void * securefs_ctxt = 0;
            int status = qapi_Securefs_Open(&securefs_ctxt, file_info.file_path, QAPI_FS_O_RDONLY, user_password, USER_PASSWORD_SIZE);
            if ( 0 == status ) {
                SECUREFS_DEMO_PRINTF("%10d %s\r\n", file_info.sbuf.st_size, file_info.file_path);
                qapi_Securefs_Close(securefs_ctxt);
            }
        }
    } while ( QAPI_OK == status );

    qapi_Fs_Iter_Close(iter_handle);

    return QCLI_STATUS_SUCCESS_E;

securefs_demo_ls_on_error:
    SECUREFS_DEMO_PRINTF("Usage: securefs ls password\r\n");
    return QCLI_STATUS_ERROR_E;
}


QCLI_Command_Status_t securefs_demo_rm(uint32_t parameters_count, QCLI_Parameter_t * parameters)
{
    if ( parameters_count < 1 ) {
        SECUREFS_DEMO_PRINTF("Invalid number of parameters\r\n");
        goto securefs_demo_rm_on_error;
    }

    qapi_Status_t status = qapi_Fs_Unlink(parameters[0].String_Value);
    if ( QAPI_OK != status ) {
        SECUREFS_DEMO_PRINTF("Failed on a call to qapi_Fs_Unlink()\r\n");
        goto securefs_demo_rm_on_error;
    }

    return QCLI_STATUS_SUCCESS_E;

securefs_demo_rm_on_error:
    SECUREFS_DEMO_PRINTF("Usage: securefs rm filename\r\n");
    return QCLI_STATUS_ERROR_E;
}


QCLI_Command_Status_t securefs_demo_open(uint32_t parameters_count, QCLI_Parameter_t * parameters)
{
    qapi_Status_t status = QAPI_OK;
    uint8_t user_password[USER_PASSWORD_SIZE];

    if( parameters_count != 3 )
    {
        SECUREFS_DEMO_PRINTF("Invalid number of parameters\r\n");
        status = QAPI_ERR_INVALID_PARAM;
        goto securefs_demo_open_on_error;
    }

    char * file_name = parameters[0].String_Value;

    char * password_in_hex = parameters[1].String_Value;
    int status_code = convert_data_in_hex_to_byte_array(password_in_hex, user_password, USER_PASSWORD_SIZE);
    if ( 0 != status_code ) {
        SECUREFS_DEMO_PRINTF("Invalid password_in_hex, must be exactly 32 hex chars\r\n");
        status = QAPI_ERR_INVALID_PARAM;
        goto securefs_demo_open_on_error;
    }

    if ( !parameters[2].Integer_Is_Valid ) {
        SECUREFS_DEMO_PRINTF("oflags is not a valid integer\r\n");
        status = QAPI_ERR_INVALID_PARAM;
        goto securefs_demo_open_on_error;
    }
    uint32_t oflags = parameters[2].Integer_Value;

    status = qapi_Securefs_Open(&g_securefs_demo_ctxt, file_name, oflags, user_password, USER_PASSWORD_SIZE);
    if ( 0 != status ) {
        SECUREFS_DEMO_PRINTF("Failed on a call to qapi_Securefs_Open()\r\n");
        status = QAPI_ERROR;
        goto securefs_demo_open_on_error;
    }

securefs_demo_open_on_error:
    if ( QAPI_OK != status ) {
        SECUREFS_DEMO_PRINTF("Usage: securefs open filename password_in_hex oflags\r\n");
        return QCLI_STATUS_ERROR_E;
    }
    return QCLI_STATUS_SUCCESS_E;
}


QCLI_Command_Status_t securefs_demo_seek(uint32_t parameters_count, QCLI_Parameter_t * parameters)
{
    qapi_Status_t status = QAPI_OK;

    if( parameters_count != 2 ) {
        SECUREFS_DEMO_PRINTF("Invalid number of parameters\r\n");
        status = QAPI_ERR_INVALID_PARAM;
        goto securefs_demo_seek_on_error;
    }

    if ( !parameters[0].Integer_Is_Valid ) {
        SECUREFS_DEMO_PRINTF("offset is not a valid integer\r\n");
        status = QAPI_ERR_INVALID_PARAM;
        goto securefs_demo_seek_on_error;
    }
    off_t offset = parameters[0].Integer_Value;

    if ( !parameters[1].Integer_Is_Valid ) {
        SECUREFS_DEMO_PRINTF("whence is not a valid integer\r\n");
        status = QAPI_ERR_INVALID_PARAM;
        goto securefs_demo_seek_on_error;
    }
    uint32_t whence = parameters[1].Integer_Value;

    off_t actual_offset = 0;
    status = qapi_Securefs_Lseek(g_securefs_demo_ctxt, offset, whence, &actual_offset);

securefs_demo_seek_on_error:
    if ( QAPI_OK != status ) {
        SECUREFS_DEMO_PRINTF("Usage: securefs seek offset whence\r\n");
        return QCLI_STATUS_ERROR_E;
    }
    return QCLI_STATUS_SUCCESS_E;
}


QCLI_Command_Status_t securefs_demo_tell(uint32_t parameters_count, QCLI_Parameter_t * parameters)
{
    qapi_Status_t status = QAPI_OK;

    off_t actual_offset = 0;
    status = qapi_Securefs_Tell(g_securefs_demo_ctxt, &actual_offset);
    if ( QAPI_OK != status ) {
        goto securefs_demo_tell_on_error;
    }
    SECUREFS_DEMO_PRINTF("offset: %d\r\n", actual_offset);

securefs_demo_tell_on_error:
    if ( QAPI_OK != status ) {
        SECUREFS_DEMO_PRINTF("Usage: securefs tell\r\n");
        return QCLI_STATUS_ERROR_E;
    }
    return QCLI_STATUS_SUCCESS_E;
}


QCLI_Command_Status_t securefs_demo_read(uint32_t parameters_count, QCLI_Parameter_t * parameters)
{
    qapi_Status_t status = QAPI_OK;

    if( parameters_count != 1 ) {
        SECUREFS_DEMO_PRINTF("Invalid number of parameters\r\n");
        status = QAPI_ERR_INVALID_PARAM;
        goto securefs_demo_read_on_error;
    }

    if ( !parameters[0].Integer_Is_Valid ) {
        SECUREFS_DEMO_PRINTF("length is not a valid integer\r\n");
        status = QAPI_ERR_INVALID_PARAM;
        goto securefs_demo_read_on_error;
    }
    uint32_t length = parameters[0].Integer_Value;

    uint8_t temp_buffer[16];

    int32_t total_bytes_remaining_to_read = length;

    while ( total_bytes_remaining_to_read > 0 )
    {
        size_t bytes_read;
        const size_t bytes_to_read = MIN(sizeof(temp_buffer), total_bytes_remaining_to_read);
        status = qapi_Securefs_Read(g_securefs_demo_ctxt, temp_buffer, bytes_to_read, &bytes_read);
        if ( 0 != status ) {
            SECUREFS_DEMO_PRINTF("Failed on a call to qapi_Securefs_Read()\r\n");
            status = QAPI_ERROR;
            goto securefs_demo_read_on_error;
        }
        int i;
        for ( i = 0; i < bytes_read; i++ ) {
            SECUREFS_DEMO_PRINTF("%02x", temp_buffer[i]);
            if ( i == (bytes_read-1) ) {
                SECUREFS_DEMO_PRINTF("\r\n");
            }
        }
        total_bytes_remaining_to_read -= bytes_read;
        if ( bytes_to_read != bytes_read ) {
            break;
        }
    }

securefs_demo_read_on_error:
    if ( QAPI_OK != status ) {
        SECUREFS_DEMO_PRINTF("Usage: securefs read length\r\n");
        return QCLI_STATUS_ERROR_E;
    }
    return QCLI_STATUS_SUCCESS_E;
}


QCLI_Command_Status_t securefs_demo_write(uint32_t parameters_count, QCLI_Parameter_t * parameters)
{
    qapi_Status_t status = QAPI_OK;

    // check parameters
    if ( parameters_count != 1 ) {
        SECUREFS_DEMO_PRINTF("Invalid number of parameters\r\n");
        status = QAPI_ERR_INVALID_PARAM;
        goto securefs_demo_write_on_error;
    }

    char * data_in_hex = parameters[0].String_Value;
    size_t data_in_hex_length = strnlen(data_in_hex, 256);
    if ( (data_in_hex_length < 2) || (data_in_hex_length & 0x01) ) {
        SECUREFS_DEMO_PRINTF("data_in_hex is invalid.  Either too short, or is not a multiple of 2 chars\r\n");
        status = QAPI_ERR_INVALID_PARAM;
        goto securefs_demo_write_on_error;
    }

    // convert hex string into array of bytes
    const size_t data_size = data_in_hex_length / 2;
    uint8_t * data = (uint8_t *) malloc(data_size);
    convert_data_in_hex_to_byte_array(data_in_hex, data, data_size);

    size_t bytes_written;
    status = qapi_Securefs_Write(g_securefs_demo_ctxt, data, data_size, &bytes_written);
    if ( (0 != status) || (bytes_written != data_size) ) {
        SECUREFS_DEMO_PRINTF("Failed on a call to qapi_Securefs_Lseek()\r\n");
        status = QAPI_ERROR;
        goto securefs_demo_write_cleanup;
    }

securefs_demo_write_cleanup:
    if ( data ) {
        free(data);
    }

securefs_demo_write_on_error:
    if ( QAPI_OK != status ) {
        SECUREFS_DEMO_PRINTF("Usage: securefs write data_in_hex\r\n");
        return QCLI_STATUS_ERROR_E;
    }

    return QCLI_STATUS_SUCCESS_E;
}


QCLI_Command_Status_t securefs_demo_close(uint32_t parameters_count, QCLI_Parameter_t * parameters)
{
    qapi_Status_t status = QAPI_OK;

    status = qapi_Securefs_Close(g_securefs_demo_ctxt);
    g_securefs_demo_ctxt = 0;

    if ( QAPI_OK != status ) {
        SECUREFS_DEMO_PRINTF("Usage: securefs close\r\n");
        return QCLI_STATUS_ERROR_E;
    }
    return QCLI_STATUS_SUCCESS_E;
}


#define MAXIMUM_NUMBER_OF_PATTERNS 6
#define MAXIMUM_FILE_SIZE (24*1024+1)
#define MAXIMUM_BLOCK_SIZE (6*1024)
static uint8_t securefs_unittest_patterns[MAXIMUM_NUMBER_OF_PATTERNS] =
{
    0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff
};

char securefs_unittest_filename[] = "/spinor/securefs_unittest.bin";
char securefs_unittest_password[] = "0123456789abcdef";


int securefs_unittest_run_random_test()
{
    size_t array_of_sizes[MAXIMUM_NUMBER_OF_PATTERNS];
    memset(array_of_sizes, 0, sizeof(array_of_sizes));

    uint8_t * buffer = 0;

    int status = 0;

    void * securefs_ctxt = 0;

    int patterns_count;
    status = qapi_Crypto_Random_Get(&patterns_count, sizeof(patterns_count));
    if ( 0 != status ) {
        return status;
    }

    uint32_t total = 0;
    uint32_t i;
    for ( i = 0; i < MAXIMUM_NUMBER_OF_PATTERNS; i++ ) {
        uint32_t remaining_bytes_to_write = MIN(MAXIMUM_FILE_SIZE - total, MAXIMUM_BLOCK_SIZE);
        uint32_t bytes_to_write;
        status = qapi_Crypto_Random_Get(&bytes_to_write, sizeof(bytes_to_write));
        if ( 0 != status ) {
            goto cleanup;
        }
        bytes_to_write = bytes_to_write % remaining_bytes_to_write;
        if ( 0 == bytes_to_write ) {
            array_of_sizes[i] = 1;
        }
        else {
            array_of_sizes[i] = bytes_to_write;
        }
        total = total + bytes_to_write;
    }

    SECUREFS_DEMO_PRINTF("Unittest Blocks: ");
    for ( i = 0; i < MAXIMUM_NUMBER_OF_PATTERNS; i++ ) {
        SECUREFS_DEMO_PRINTF("%d, ", array_of_sizes[i]);
    }
    SECUREFS_DEMO_PRINTF("\r\n");

    status = qapi_Securefs_Open(&securefs_ctxt, securefs_unittest_filename, QAPI_FS_O_CREAT | QAPI_FS_O_RDWR, (uint8_t*) securefs_unittest_password, (sizeof(securefs_unittest_password)-1));
    if ( 0 != status ) {
        SECUREFS_DEMO_PRINTF("Failed to create file\r\n");
        goto cleanup;
    }

    for ( i = 0; i < MAXIMUM_NUMBER_OF_PATTERNS; i++ ) {
        buffer = malloc(array_of_sizes[i]);
        if ( !buffer ) {
            SECUREFS_DEMO_PRINTF("Failed to allocate %d bytes write buffer\r\n", array_of_sizes[i]);
            status = -2;
            goto cleanup;
        }
        memset(buffer, securefs_unittest_patterns[i], array_of_sizes[i]);
        size_t bytes_written = 0;
        status = qapi_Securefs_Write(securefs_ctxt, buffer, array_of_sizes[i], &bytes_written);
        if ( (0 != status) || (bytes_written != array_of_sizes[i]) ) {
            SECUREFS_DEMO_PRINTF("Failed on a call to qapi_Securefs_Write, count=%d, bytes_written=%d, status=%d\r\n", array_of_sizes[i], bytes_written, status);
            status = -3;
            goto cleanup;
        }
        free(buffer);
        buffer = 0;
    }

    off_t actual_offset = 0;
    status = qapi_Securefs_Lseek(securefs_ctxt, 0, QAPI_FS_SEEK_SET, &actual_offset);
    if ( (0 != status) || (0 != actual_offset) ) {
        SECUREFS_DEMO_PRINTF("Failed on a call to qapi_Securefs_Lseek, offset=%d, actual_offset=%d, status=%d\r\n", 0, actual_offset, status);
        status = -4;
        goto cleanup;
    }


    buffer = malloc(MAXIMUM_BLOCK_SIZE);
    if ( !buffer ) {
        SECUREFS_DEMO_PRINTF("Failed to allocate %d bytes read buffer\r\n", MAXIMUM_BLOCK_SIZE);
        status = -5;
        goto cleanup;
    }
    for ( i = 0; i < MAXIMUM_NUMBER_OF_PATTERNS; i++ ) {
        memset(buffer, securefs_unittest_patterns[i], array_of_sizes[i]);
        size_t bytes_read = 0;
        status = qapi_Securefs_Read(securefs_ctxt, buffer, array_of_sizes[i], &bytes_read);
        if ( (0 != status) || (bytes_read != array_of_sizes[i]) ) {
            SECUREFS_DEMO_PRINTF("Failed on a call to qapi_Securefs_Read, count=%d, bytes_read=%d, status=%d\r\n", array_of_sizes[i], bytes_read, status);
            status = -6;
            goto cleanup;
        }
        status = 0;
        int j;
        for ( j = 0; j < array_of_sizes[i]; j++) {
            if ( buffer[j] != securefs_unittest_patterns[i] ) {
                SECUREFS_DEMO_PRINTF("Data does NOT match, i=%d, j=%d\r\n", i, j);
                status = -7;
                break;
            }
        }
        if ( 0 != status ) {
            goto cleanup;
        }
    }

cleanup:
    if ( buffer ) {
        free(buffer);
        buffer = 0;
    }
    if ( securefs_ctxt ) {
        qapi_Securefs_Close(securefs_ctxt);
        securefs_ctxt = 0;
    }

    qapi_Fs_Unlink(securefs_unittest_filename);

    return status;
}


QCLI_Command_Status_t securefs_demo_run_unittests(uint32_t parameters_count, QCLI_Parameter_t * parameters)
{
    qapi_Status_t status = QAPI_OK;

    // check parameters
    if(parameters_count < 1)
    {
        SECUREFS_DEMO_PRINTF("Invalid number of parameters\r\n");
        status = QAPI_ERR_INVALID_PARAM;
        goto securefs_demo_run_unittests_on_error;
    }

    if ( !parameters[0].Integer_Is_Valid ) {
        SECUREFS_DEMO_PRINTF("number_of_unittests_to_run is not valid integer\r\n");
        status = QAPI_ERR_INVALID_PARAM;
        goto securefs_demo_run_unittests_on_error;
    }

    const uint32_t number_of_unittests_to_run = parameters[0].Integer_Value;

    uint32_t i;
    for ( i = 0; i < number_of_unittests_to_run; i++ ) {
        int status = securefs_unittest_run_random_test();
        if ( 0 != status ) {
            SECUREFS_DEMO_PRINTF("FAILURE\r\n");
            break;
        }
        else {
            SECUREFS_DEMO_PRINTF("SUCCESS\r\n");
        }
    }

securefs_demo_run_unittests_on_error:
    if ( QAPI_OK != status ) {
        SECUREFS_DEMO_PRINTF("Usage: run_unittests number_of_unittests_to_run.\r\n");
        return QCLI_STATUS_ERROR_E;
    }

    return status;
}
