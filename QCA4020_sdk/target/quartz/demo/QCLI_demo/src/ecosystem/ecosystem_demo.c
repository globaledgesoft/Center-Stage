/*
 * Copyright (c) 2016-2017 Qualcomm Technologies, Inc.
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
#include "qapi_ssl.h"
#include "ecosystem_demo.h"

#ifdef AWS_IOT
#include "shadow_sample.h"
#endif
#ifdef OC_CLIENT
#include "simpleclient.h"
#endif
#ifdef OC_SERVER
#include "simpleserver.h"
#endif
#ifdef LIB_COAP
#include "coap_demo.h"
#endif
#ifdef LIBIOTA
extern QCLI_Command_Status_t qc_iota_app_main(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
#endif
#ifdef AZURE_IOT
extern QCLI_Command_Status_t azure_twin_init(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
extern QCLI_Command_Status_t azure_simple_init(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t azure_stop(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);

extern void device_twin_simple_sample_run(void);
extern void device_twin_simple_sample_stop(void);
extern void simplesample_mqtt_run(void);
extern void simplesample_mqtt_stop(void);
#endif

QCLI_Command_Status_t dummy_cmd(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);

/*
 * This file contains the command handlers for ecosystem
 *
 */

QCLI_Group_Handle_t qcli_ecosystem_handle; /* Handle for Ecosystem Command Group. */


const QCLI_Command_t ecosystem_cmd_list[] =
{
#ifdef AWS_IOT
    {shadow_sample_init, true, "aws_set_schema", " Enter 'shadow-sample' for help\n", "Initialize Shadow Sample application"},
    {shadow_sample_set_params, true, "aws_set_params", " Enter 'aws_quit' for help\n", "Set params for schema objects"},
    {shadow_sample_run, true, "aws_run", "Run aws shadow sample application", "run shadow_sample application"},
    {shadow_sample_destroy, true, "aws_quit", " Enter 'aws_quit' for help\n", "Quit AWS IOT shadow sample test"},
#endif
#ifdef LIB_COAP
    {coap_handler, true, "coap", " Enter 'coap' for help\n", "Run Coap test"},
#endif
#ifdef OC_CLIENT
    {simple_client_main, true, "iot-simpleclient", " Enter 'iot-simpleclient' for help\n", "Run IOTivity simpleclient test"},
#endif    
#ifdef OC_SERVER
    {simple_server_main, true, "iot-simpleserver", " Enter 'iot-simpleserver' for help\n", "Run IOTivity simpleserver test"},
#endif
#ifdef LIBIOTA
    {qc_iota_app_main, true, "weave", " Enter 'wave' for help\n", "Run weave test"},
#endif
#ifdef AZURE_IOT
    {azure_twin_init, true, "azure-twin", " Enter 'azure' for help\n", "Run azure test"},
    {azure_simple_init, true, "azure-simple", " Enter 'azure' for help\n", "Run azure test"},
	{azure_stop, false, "azure-stop", " Enter 'azure' for help\n", "Run azure test"},
#endif
     {dummy_cmd,false,"dummy",NULL,NULL},
};

const QCLI_Command_Group_t ecosystem_cmd_group =
{
    "Ecosystem",              /* Group_String: will display cmd prompt as "Ecosystem> " */
    sizeof(ecosystem_cmd_list)/sizeof(ecosystem_cmd_list[0]),   /* Command_Count */
    ecosystem_cmd_list        /* Command_List */
};


/*****************************************************************************
 * This function is used to register the Fs Command Group with QCLI.
 *****************************************************************************/
void Initialize_Ecosystem_Demo(void)
{
    /* Attempt to reqister the Command Groups with the qcli framework.*/
    qcli_ecosystem_handle = QCLI_Register_Command_Group(NULL, &ecosystem_cmd_group);
    if (qcli_ecosystem_handle)
    {
        QCLI_Printf(qcli_ecosystem_handle, "Ecosystem Registered\n");
    }

    return;
}


#ifdef AZURE_IOT




QCLI_Command_Status_t azure_stop(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
		QCLI_Printf(qcli_ecosystem_handle, "Stopping Azure Demo\n");
	simplesample_mqtt_stop();
	device_twin_simple_sample_stop();		
    
	return QCLI_STATUS_SUCCESS_E;
}





#ifdef TMP_REALLOC
//remove once realloc function is verified

typedef struct mem_block_header_struct {
  short        header_guard;

  unsigned char extra;        /**< Extra bytes at the end of a block. */
  char          free_flag:4;  /**< Flag to indicate if this memory block 
                                   is free. */
  char          last_flag:4;/**< Flag to indicate if this is the last block 
                                   in the allocated section. */
  unsigned int forw_offset; /**< Forward offset. The value of the offset 
                                   includes the size of the header and the 
                                   allocated block. */
#ifdef FEATURE_MEM_DEBUG
  void          *caller_ptr;
  uint32        tid;
#endif
								   
} mem_block_header_type;

int malloc_size(void* ptr)
{
	mem_block_header_type* tmp = (mem_block_header_type*)((char*)ptr - sizeof(mem_block_header_type));
	return (tmp->forw_offset -  tmp->extra -  sizeof(mem_block_header_type) );
}
void *
realloc(void * ptr, size_t size)
{
    void *new;
	int ptrsz;
    new = malloc(size);
    if (!new) { goto error; }
	
		
    if (!ptr) {
    //    new = malloc(size);
    //    if (!new) { goto error; }
    } else {

		 ptrsz= malloc_size(ptr);

            if (ptrsz > size)
				ptrsz=size;
            memcpy(new, ptr, ptrsz);

            free(ptr);

    }

    return new;
error:
    return NULL;
}
#endif
#endif

QCLI_Command_Status_t dummy_cmd(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	return QCLI_STATUS_SUCCESS_E;  
}