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
#include "fs_demo.h"


#ifndef MIN
   #define  MIN( x, y ) ( ((x) < (y)) ? (x) : (y) )
#endif

#define FS_DEMO_DEFAULT_MOUNT_POINT "/spinor/"

/*
 * This file contains the command handlers for file management operations
 * on non-volatile memory like list, delete, read, write
 *
 */

QCLI_Group_Handle_t qcli_fs_handle; /* Handle for Fs Command Group. */


#define FS_DEMO_PRINTF(...) QCLI_Printf(qcli_fs_handle, __VA_ARGS__)


#define hex_to_dec_nibble(hex_nibble) ( (hex_nibble >= 'a') ? (hex_nibble-'a'+10) : ((hex_nibble >= 'A') ? (hex_nibble-'A'+10) : (hex_nibble-'0')) )



QCLI_Command_Status_t fs_demo_ls(uint32_t parameters_count, QCLI_Parameter_t * parameters);
QCLI_Command_Status_t fs_demo_format(uint32_t parameters_count, QCLI_Parameter_t * parameters);
QCLI_Command_Status_t fs_demo_rm(uint32_t parameters_count, QCLI_Parameter_t * parameters);
QCLI_Command_Status_t fs_demo_read(uint32_t parameters_count, QCLI_Parameter_t * parameters);
QCLI_Command_Status_t fs_demo_write(uint32_t parameters_count, QCLI_Parameter_t * parameters);
QCLI_Command_Status_t fs_demo_run_unittests(uint32_t parameters_count, QCLI_Parameter_t * parameters);

const QCLI_Command_t fs_cmd_list[] =
{
    {fs_demo_ls, false, "ls", "\n", "lists all files in "FS_DEMO_DEFAULT_MOUNT_POINT"\n"},
    {fs_demo_format, false, "format", "\n", "removes all files in "FS_DEMO_DEFAULT_MOUNT_POINT"\n"},
    {fs_demo_rm, false, "rm", FS_DEMO_DEFAULT_MOUNT_POINT"<filename>\n", "removes "FS_DEMO_DEFAULT_MOUNT_POINT"filename\n"},
    {fs_demo_read, false, "read", FS_DEMO_DEFAULT_MOUNT_POINT"<filename> <offset> <length>\n", "reads and prints length bytes from filename starting at offset. Must specify "FS_DEMO_DEFAULT_MOUNT_POINT" prefix\n"},
    {fs_demo_write, false, "write", FS_DEMO_DEFAULT_MOUNT_POINT"<filename> <offset> <hex_data>\n", "writes data to filename starting at offset. Must specify "FS_DEMO_DEFAULT_MOUNT_POINT" prefix. The hex_data is converted into binary before being written into filename\n"},
    {fs_demo_run_unittests, false, "run_unittests", "number_of_unittests_to_run\n", "Executes number_of_unittests_to_run random FS_API unittests\n"},

};

const QCLI_Command_Group_t fs_cmd_group =
{
    "Fs",              /* Group_String: will display cmd prompt as "Fs> " */
    sizeof(fs_cmd_list)/sizeof(fs_cmd_list[0]),   /* Command_Count */
    fs_cmd_list        /* Command_List */
};


/*****************************************************************************
 * This function is used to register the Fs Command Group with QCLI.
 *****************************************************************************/
void Initialize_Fs_Demo(void)
{
    /* Attempt to reqister the Command Groups with the qcli framework.*/
    qcli_fs_handle = QCLI_Register_Command_Group(NULL, &fs_cmd_group);
    if (qcli_fs_handle)
    {
        QCLI_Printf(qcli_fs_handle, "Fs Registered\n");
    }

    return;
}

QCLI_Command_Status_t fs_demo_ls(uint32_t parameters_count, QCLI_Parameter_t * parameters)
{
    qapi_fs_iter_handle_t iter_handle;
    qapi_Status_t status = qapi_Fs_Iter_Open(FS_DEMO_DEFAULT_MOUNT_POINT, &iter_handle);
    if ( QAPI_OK != status ) {
        FS_DEMO_PRINTF("Failed on a call to qapi_Fs_Iter_Open()\r\n");
        goto fs_demo_ls_on_error;
    }

    struct qapi_fs_iter_entry file_info;

    do {
        status = qapi_Fs_Iter_Next(iter_handle, &file_info);
        if ( QAPI_OK == status ) {
            FS_DEMO_PRINTF("%10d %s\r\n", file_info.sbuf.st_size, file_info.file_path);
        }
    } while ( QAPI_OK == status );

    qapi_Fs_Iter_Close(iter_handle);

    return QCLI_STATUS_SUCCESS_E;

fs_demo_ls_on_error:
    FS_DEMO_PRINTF("Usage: ls\r\n");
    return QCLI_STATUS_ERROR_E;
}


QCLI_Command_Status_t fs_demo_format(uint32_t parameters_count, QCLI_Parameter_t * parameters)
{
    qapi_fs_iter_handle_t iter_handle;
    int is_done = 0;
    do
    {
        is_done = 1;
        qapi_Status_t status = qapi_Fs_Iter_Open(FS_DEMO_DEFAULT_MOUNT_POINT, &iter_handle);
        if ( QAPI_OK != status ) {
            FS_DEMO_PRINTF("Failed on a call to qapi_Fs_Iter_Open()\r\n");
            goto fs_demo_ls_on_error;
        }

        struct qapi_fs_iter_entry file_info;

        do {
            status = qapi_Fs_Iter_Next(iter_handle, &file_info);
            if ( QAPI_OK == status ) {
                qapi_Fs_Unlink(file_info.file_path);
                is_done = 0;
            }
        } while ( QAPI_OK == status );

        qapi_Fs_Iter_Close(iter_handle);
    } while ( !is_done );

    return QCLI_STATUS_SUCCESS_E;

fs_demo_ls_on_error:
    FS_DEMO_PRINTF("Usage: format\r\n");
    return QCLI_STATUS_ERROR_E;
}


QCLI_Command_Status_t fs_demo_rm(uint32_t parameters_count, QCLI_Parameter_t * parameters)
{
    const char * filename;
    qapi_Status_t status;

    if ( parameters_count < 1 ) {
        FS_DEMO_PRINTF("Invalid number of parameters\r\n");
        goto fs_demo_rm_on_error;
    }

    filename = (char *) parameters[0].String_Value;
    status = qapi_Fs_Unlink(filename);
    if ( QAPI_OK != status ) {
        FS_DEMO_PRINTF("Failed on a call to qapi_Fs_Unlink()\r\n");
        goto fs_demo_rm_on_error;
    }

    FS_DEMO_PRINTF("Successfully removed %s\r\n", filename);
    return QCLI_STATUS_SUCCESS_E;

fs_demo_rm_on_error:
    return QCLI_STATUS_USAGE_E;
}


QCLI_Command_Status_t fs_demo_read(uint32_t parameters_count, QCLI_Parameter_t * parameters)
{
    qapi_Status_t status = QAPI_OK;
    char * file_name;
    uint8_t temp_buffer[16];
    int fd;  
    uint32_t start_offset, length;
    int32_t actual_offset, total_bytes_remaining_to_read;

    if(parameters_count < 3)
    {
        FS_DEMO_PRINTF("Invalid number of parameters\r\n");
        status = QAPI_ERR_INVALID_PARAM;
        goto fs_demo_read_on_error;
    }

    file_name = (char *) parameters[0].String_Value;

    if ( !parameters[1].Integer_Is_Valid ) {
        FS_DEMO_PRINTF("start_offset is not a valid integer\r\n");
        status = QAPI_ERR_INVALID_PARAM;
        goto fs_demo_read_on_error;
    }

    start_offset = parameters[1].Integer_Value;

    if ( !parameters[2].Integer_Is_Valid ) {
        FS_DEMO_PRINTF("length is not a valid integer\r\n");
        status = QAPI_ERR_INVALID_PARAM;
        goto fs_demo_read_on_error;
    }

    length = parameters[2].Integer_Value;

    status = qapi_Fs_Open(file_name, QAPI_FS_O_RDONLY, &fd);
    if ( QAPI_OK != status ) {
        FS_DEMO_PRINTF("Failed on a call to qapi_Fs_Open()\r\n");
        goto fs_demo_read_on_error;
    }
    
    status = qapi_Fs_Lseek(fd, start_offset, QAPI_FS_SEEK_SET, &actual_offset);
    if ( (QAPI_OK != status) || (actual_offset != start_offset) ) {
        FS_DEMO_PRINTF("Failed on a call to qapi_Fs_Lseek()\r\n");
        status = QAPI_ERROR;
        goto fs_demo_read_cleanup;
    }

    total_bytes_remaining_to_read = length;

    while ( total_bytes_remaining_to_read > 0 )
    {
        uint32_t bytes_read;
        const uint32_t bytes_to_read = MIN(sizeof(temp_buffer), total_bytes_remaining_to_read);
        status = qapi_Fs_Read(fd, temp_buffer, bytes_to_read, &bytes_read);
        if ( QAPI_OK != status ) {
            FS_DEMO_PRINTF("Failed on a call to qapi_Fs_Read()\r\n");
            goto fs_demo_read_cleanup;
        }
        int i;
        for ( i = 0; i < bytes_read; i++ ) {
            FS_DEMO_PRINTF("%02x", temp_buffer[i]);
            if ( i == (bytes_read-1) ) {
                FS_DEMO_PRINTF("\r\n");
            }
        }
        total_bytes_remaining_to_read -= bytes_read;
        if ( bytes_to_read != bytes_read ) {
            break;
        }
    }

fs_demo_read_cleanup:
    qapi_Fs_Close(fd);

fs_demo_read_on_error:
    if ( QAPI_OK != status ) {
        return QCLI_STATUS_USAGE_E;
    }

    return QCLI_STATUS_SUCCESS_E;
}


QCLI_Command_Status_t fs_demo_write(uint32_t parameters_count, QCLI_Parameter_t * parameters)
{
    qapi_Status_t status = QAPI_OK;
    char * file_name;    
    char * data_in_hex;
    uint8_t * data;
    int fd, i, data_in_hex_length;
    int32_t actual_offset;
    uint32_t start_offset, data_size, bytes_written;    

    // check parameters
    if(parameters_count < 3)
    {
        FS_DEMO_PRINTF("Invalid number of parameters\r\n");
        status = QAPI_ERR_INVALID_PARAM;
        goto fs_demo_write_on_error;
    }

    file_name = (char *) parameters[0].String_Value;
    
    if ( !parameters[1].Integer_Is_Valid ) {
        FS_DEMO_PRINTF("start_offset is not a valid integer\r\n");
        status = QAPI_ERR_INVALID_PARAM;
        goto fs_demo_write_on_error;
    }

    start_offset = parameters[1].Integer_Value;

    data_in_hex = (char *) parameters[2].String_Value;
    data_in_hex_length = strnlen(data_in_hex, 256);
    if ( (data_in_hex_length < 2) || (data_in_hex_length & 0x01) ) {
        FS_DEMO_PRINTF("data_in_hex is invalid.  Either too short, or is not a multiple of 2 chars\r\n");
        status = QAPI_ERR_INVALID_PARAM;
        goto fs_demo_write_on_error;
    }

    // convert hex string into array of bytes
    data_size = data_in_hex_length / 2;
    data = (uint8_t *) malloc(data_size);
    for ( i =0; i < data_size; i++ ) {
        data[i] = (hex_to_dec_nibble(data_in_hex[2*i]) << 4) | hex_to_dec_nibble(data_in_hex[2*i+1]);
    }

    // write the array of bytes into the file
    fd = -1;
    status = qapi_Fs_Open(file_name, QAPI_FS_O_CREAT | QAPI_FS_O_WRONLY, &fd);
    if ( QAPI_OK != status ) {
        FS_DEMO_PRINTF("Failed on a call to qapi_Fs_Open()\r\n");
        goto fs_demo_write_cleanup;
    }

    status = qapi_Fs_Lseek(fd, start_offset, QAPI_FS_SEEK_SET, &actual_offset);
    if ( (QAPI_OK != status) || (actual_offset != start_offset) ) {
        FS_DEMO_PRINTF("Failed on a call to qapi_Fs_Lseek()\r\n");
        status = QAPI_ERROR;
        goto fs_demo_write_cleanup;
    }

    status = qapi_Fs_Write(fd, data, data_size, &bytes_written);
    if ( (QAPI_OK != status) || (bytes_written != data_size) ) {
        FS_DEMO_PRINTF("Failed on a call to qapi_Fs_Write()\r\n");
        status = QAPI_ERROR;
        goto fs_demo_write_cleanup;
    }

fs_demo_write_cleanup:
    if ( data ) {
        free(data);
    }
    if ( fd >= 0 ) {
        qapi_Fs_Close(fd);
    }

fs_demo_write_on_error:
    if ( QAPI_OK != status ) {
        return QCLI_STATUS_USAGE_E;
    }

    return QCLI_STATUS_SUCCESS_E;
}


#define MAXIMUM_NUMBER_OF_PATTERNS 6
#define MAXIMUM_FILE_SIZE (24*1024+1)
#define MAXIMUM_BLOCK_SIZE (6*1024)
static uint8_t fs_unittest_patterns[MAXIMUM_NUMBER_OF_PATTERNS] =
{
    0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff
};
char fs_unittest_filename[] = FS_DEMO_DEFAULT_MOUNT_POINT "fs_unittest.bin";

static int fs_unittest_run_random_test()
{
    int32_t array_of_sizes[MAXIMUM_NUMBER_OF_PATTERNS];
    uint8_t * buffer = 0;
    int status = 0;
    int fd = -1;
    int patterns_count;
    int32_t actual_offset;
    uint32_t i, total = 0;
    
    memset(array_of_sizes, 0, sizeof(array_of_sizes));
    
    status = qapi_Crypto_Random_Get(&patterns_count, sizeof(patterns_count));
    if ( 0 != status ) {
        return status;
    }

    for ( i = 0; i < MAXIMUM_NUMBER_OF_PATTERNS; i++ ) {
        uint32_t remaining_bytes_to_write = MIN(MAXIMUM_FILE_SIZE - total, MAXIMUM_BLOCK_SIZE);
        uint32_t bytes_to_write;
        status = qapi_Crypto_Random_Get(&bytes_to_write, sizeof(bytes_to_write));
        if ( 0 != status ) {
            goto cleanup;
        }
        bytes_to_write = bytes_to_write % remaining_bytes_to_write;
        array_of_sizes[i] = bytes_to_write;
        total = total + bytes_to_write;
    }

    FS_DEMO_PRINTF("Unittest Blocks: ");
    for ( i = 0; i < MAXIMUM_NUMBER_OF_PATTERNS; i++ ) {
        FS_DEMO_PRINTF("%d, ", array_of_sizes[i]);
    }
    FS_DEMO_PRINTF("\r\n");

    status = qapi_Fs_Open(fs_unittest_filename, QAPI_FS_O_CREAT | QAPI_FS_O_RDWR, &fd);
    if ( 0 != status ) {
        FS_DEMO_PRINTF("Failed to create file\r\n");
        goto cleanup;
    }

    for ( i = 0; i < MAXIMUM_NUMBER_OF_PATTERNS; i++ ) {
        buffer = malloc(array_of_sizes[i]);
        if ( !buffer ) {
            FS_DEMO_PRINTF("Failed to allocate %d bytes write buffer\r\n", array_of_sizes[i]);
            status = -2;
            goto cleanup;
        }
        memset(buffer, fs_unittest_patterns[i], array_of_sizes[i]);
        uint32_t bytes_written = 0;
        status = qapi_Fs_Write(fd, buffer, array_of_sizes[i], &bytes_written);
        if ( (0 != status) || (bytes_written != array_of_sizes[i]) ) {
            FS_DEMO_PRINTF("Failed on a call to qapi_Fs_Write, count=%d, bytes_written=%d, status=%d\r\n", array_of_sizes[i], bytes_written, status);
            status = -3;
            goto cleanup;
        }
        free(buffer);
        buffer = 0;
    }

    actual_offset = 0;
    status = qapi_Fs_Lseek(fd, 0, QAPI_FS_SEEK_SET, &actual_offset);
    if ( (0 != status) || (0 != actual_offset) ) {
        FS_DEMO_PRINTF("Failed on a call to qapi_Fs_Lseek, offset=%d, actual_offset=%d, status=%d\r\n", 0, actual_offset, status);
        status = -4;
        goto cleanup;
    }

    buffer = malloc(MAXIMUM_BLOCK_SIZE);
    if ( !buffer ) {
        FS_DEMO_PRINTF("Failed to allocate %d bytes read buffer\r\n", MAXIMUM_BLOCK_SIZE);
        status = -5;
        goto cleanup;
    }
    for ( i = 0; i < MAXIMUM_NUMBER_OF_PATTERNS; i++ ) {
        memset(buffer, fs_unittest_patterns[i], array_of_sizes[i]);
        uint32_t bytes_read = 0;
        status = qapi_Fs_Read(fd, buffer, array_of_sizes[i], &bytes_read);
        if ( (0 != status) || (bytes_read != array_of_sizes[i]) ) {
            FS_DEMO_PRINTF("Failed on a call to qapi_Fs_Read, count=%d, bytes_read=%d, status=%d\r\n", array_of_sizes[i], bytes_read, status);
            status = -6;
            goto cleanup;
        }
        status = 0;
        int j;
        for ( j = 0; j < array_of_sizes[i]; j++) {
            if ( buffer[j] != fs_unittest_patterns[i] ) {
                FS_DEMO_PRINTF("Data does NOT match, i=%d, j=%d\r\n", i, j);
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
    if ( fd >= 0 ) {
        qapi_Fs_Close(fd);
        fd = -1;
    }

    qapi_Fs_Unlink(fs_unittest_filename);

    return status;
}


QCLI_Command_Status_t fs_demo_run_unittests(uint32_t parameters_count, QCLI_Parameter_t * parameters)
{
    qapi_Status_t status = QAPI_OK;
    uint32_t i, number_of_unittests_to_run;
    
    // check parameters
    if(parameters_count < 1)
    {
        FS_DEMO_PRINTF("Invalid number of parameters\r\n");
        status = QAPI_ERR_INVALID_PARAM;
        goto fs_demo_run_unittests_on_error;
    }

    if ( !parameters[0].Integer_Is_Valid ) {
        FS_DEMO_PRINTF("number_of_unittests_to_run is not valid integer\r\n");
        status = QAPI_ERR_INVALID_PARAM;
        goto fs_demo_run_unittests_on_error;
    }

    number_of_unittests_to_run = parameters[0].Integer_Value;

    for ( i = 0; i < number_of_unittests_to_run; i++ ) {
        int status = fs_unittest_run_random_test();
        if ( 0 != status ) {
            FS_DEMO_PRINTF("FAILURE\r\n");
            break;
        }
        else {
            FS_DEMO_PRINTF("SUCCESS\r\n");
        }
    }

fs_demo_run_unittests_on_error:
    if ( QAPI_OK != status ) {
        FS_DEMO_PRINTF("Usage: run_unittests number_of_unittests_to_run.\r\n");
        return QCLI_STATUS_ERROR_E;
    }

    return status;
}
