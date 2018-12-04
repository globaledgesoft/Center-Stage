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
#include <qcli.h>
#include <qcli_api.h>
#include <qapi_wlan.h>
#include "flashlog_demo.h"
#include "qapi/qapi.h"
#include "qapi_diag_cli.h"
#include "qapi_diag_msg.h"
#include "qapi_firmware_upgrade.h"


#define DIAG_MODULE_DIAG  ((unsigned char)0)


QCLI_Command_Status_t write_diag_msg_sectnum(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t write_diag_msg_sectnum_partial(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t erase_flash_enable(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t flash_enable_disable(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t write_diag_msg_msgnum(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
void write_diag_msg(uint8_t fill_type,uint32_t sect_count, uint32_t arg_type);

qapi_Status_t qapi_Fw_Upgrade_init(void);
uint8_t qapi_Fw_Upgrade_Get_Active_FWD(uint32_t *, uint32_t *);
qapi_Status_t qapi_Fw_Upgrade_Find_Partition(uint8_t , uint32_t, qapi_Part_Hdl_t *);
qapi_Status_t qapi_Fw_Upgrade_Get_Partition_Size(qapi_Part_Hdl_t , uint32_t *);
qapi_Status_t qapi_Fw_Upgrade_Erase_Partition(qapi_Part_Hdl_t , uint32_t , uint32_t );
qapi_Status_t qapi_Fw_Upgrade_Close_Partition(qapi_Part_Hdl_t );


QCLI_Group_Handle_t qcli_flashlog_group;              /* Handle for our QCLI Command Group. */                                                   

/* Command list for the main Flashlog demo. */
const QCLI_Command_t flashlog_cmd_list[] =
{
   // cmd_function                 start_thread                 cmd_string                  usage_string                       description
   { write_diag_msg_sectnum_partial,           false,           "write_diag_msg_sectnum_partial",                 "",                      "write diag messages : number of sector argument  type,fill last sector in half and other sectors completely"   },
   { erase_flash_enable,               false,           "erase_flash_enable",                     "",                      "erase specific sectors in flash and enables flash logging"   },
   { flash_enable_disable,             false,           "flash_enable_diable",                    "",                      "enable or disable flash"   },
   { write_diag_msg_msgnum,            false,           "write_diag_msg_msgnum",                  "",                      "write diag messages : number of messages argument  type"   },
   { write_diag_msg_sectnum,   false,           "write_diag_msg_sectnum",         "",                      "write diag messages : number of sector argument  type"   },
};

const QCLI_Command_Group_t flashlog_cmd_group =
{
    "FLASHLOG",
    (sizeof(flashlog_cmd_list) / sizeof(flashlog_cmd_list[0])),
    flashlog_cmd_list
};

/* ============================================================================
     FUNCTION : Initialize_FLASHLOG_Demo
	 This function is used to register the flashlog Command Group with    
     QCLI. 
***========================================================================== */
                                                            
void Initialize_FLASHLOG_Demo(void)
{
   /* Attempt to reqister the Command Groups with the qcli framework.*/
   qcli_flashlog_group = QCLI_Register_Command_Group(NULL, &flashlog_cmd_group);
   
   if(qcli_flashlog_group)
   {
      QCLI_Printf(qcli_flashlog_group, "FLASHLOG Registered \n");
   }   
   
}
/* ============================================================================
     FUNCTION : write_diag_msg
   @brief  This function is used to write the diag messages.
	  
   @param fill_type is used to identify whether partial filling or complete filling is required and the unit of messages is sector or message packets.
      0 for partial filling and sector wise filling
      1 for complete filling and sector wise filling
      2 for messages wise filling.
   
   @param sect_count is used to indicate the number of sectors to be filled .
***========================================================================== */

void write_diag_msg(uint8_t fill_type,uint32_t sect_count, uint32_t arg_type)
{   
    uint32_t iter;
	uint32_t my_int1 = 0, my_int2 = 100, my_int3 = 200;
	uint32_t sect_size[3][4]={{98,82,71,62},{196,164,142,124},{1,1,1,1}};  /*Lookup table for the packet size of various type of diag messages , for varios fill types*/
/* number of iterations is the product of number of sectors and number of diag messages in one sector =size of one sector(4kb/size of each message(20+4*number of arguments+2(delimitter bytes))).*/

    if(arg_type==3)
	{
		for( iter = ((fill_type?sect_count:(2*sect_count-1)) * sect_size[fill_type][arg_type]); iter>0; iter--)
		{
			my_int1++;
			QAPI_DIAG_MSG_ARG3( QAPI_DIAG_MSG_SYSBUF_HDL, DIAG_MODULE_DIAG, QAPI_DIAG_MSG_LVL_HIGH, "sample test command with 3 arguments arg1:%d arg2:%d arg3:%d ",my_int1, my_int2, my_int3 );
			QCLI_Printf(qcli_flashlog_group, "SAMPLE PRINT for three arguments : %d %d %d  \n",my_int1,my_int2,my_int3);
		}
	}
    else if(arg_type==2)
	{
		for( iter = ((fill_type?sect_count:(2*sect_count-1)) * sect_size[fill_type][arg_type]); iter>0; iter--)
		{
			my_int1++;
			my_int2++;
			QAPI_DIAG_MSG_ARG2( QAPI_DIAG_MSG_SYSBUF_HDL, DIAG_MODULE_DIAG, QAPI_DIAG_MSG_LVL_HIGH, "sample test command with 2 arguments arg1:%d arg2:%d ",my_int1, my_int2 );
			QCLI_Printf(qcli_flashlog_group, "SAMPLE PRINT two arguments : %d %d \n",my_int1,my_int2);
		}
	}
	else if(arg_type==1)
	{
		for( iter = ((fill_type?sect_count:(2*sect_count-1)) * sect_size[fill_type][arg_type]); iter>0; iter--)
		{   my_int1++;
			QAPI_DIAG_MSG_ARG1( QAPI_DIAG_MSG_SYSBUF_HDL, DIAG_MODULE_DIAG, QAPI_DIAG_MSG_LVL_HIGH, "sample test command with 1 argument arg1:%d",my_int1);
			QCLI_Printf(qcli_flashlog_group, "SAMPLE PRINT for single argument %d \n",my_int1);
		}
	}
	else 
	{
		for( iter = ((fill_type?sect_count:(2*sect_count-1)) * sect_size[fill_type][0]); iter>0; iter--)
		{
			QAPI_DIAG_MSG_ARG0( QAPI_DIAG_MSG_SYSBUF_HDL, DIAG_MODULE_DIAG, QAPI_DIAG_MSG_LVL_MED, "sample test command with no argument" );
			QCLI_Printf(qcli_flashlog_group, "SAMPLE PRINT for no arguments \n");
		}
	}
	return;
      
}
/* ============================================================================
     FUNCTION : write_diag_msg_sectnum
	 @brief This function is used to write diag messages from qcli demo where the unit of diag messages is sector and complete filling is required ( number of sectors specified by sct_count are filled completely)
	 @Param Parameter_Count contains the number of arguments
	 @param Parameter list points to the list of parameters
***========================================================================== */
QCLI_Command_Status_t write_diag_msg_sectnum(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{   
   
    uint32_t sect_count,arg_type;
	uint8_t fill_type=1;
		
    if (Parameter_Count > 0 && Parameter_List && Parameter_List[0].Integer_Is_Valid)
       sect_count = (Parameter_List[0].Integer_Value);
    else
       sect_count = 1;
	
    if (Parameter_Count > 1 && Parameter_List && Parameter_List[1].Integer_Is_Valid)
       arg_type = (Parameter_List[1].Integer_Value);
    else
       arg_type = 1;
   
    write_diag_msg(fill_type,sect_count,arg_type);
	QCLI_Printf(qcli_flashlog_group, "SAMPLE PRINT FOR write diag msg fill sector COMMAND\n");
	
	return QCLI_STATUS_SUCCESS_E;
        
}
/* ============================================================================
          FUNCTION : write_diag_msg_sectnum_partial
	 @brief This function is used to write diag messages from qcli demo where the unit of diag messages is sector and partial filling is required ( last sector as specified by sct_count is filled half and the other sectors are filled completely)
	 @Param Parameter_Count contains the number of arguments
	 @param Parameter list points to the list of parameters
***========================================================================== */
QCLI_Command_Status_t write_diag_msg_sectnum_partial(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{   
   
    uint32_t sect_count,arg_type;
	uint8_t fill_type=0;
		
    if (Parameter_Count > 0 && Parameter_List && Parameter_List[0].Integer_Is_Valid)
       sect_count = (Parameter_List[0].Integer_Value);
    else
       sect_count = 1;
	
    if (Parameter_Count > 1 && Parameter_List && Parameter_List[1].Integer_Is_Valid)
       arg_type = (Parameter_List[1].Integer_Value);
    else
       arg_type = 1;
   
    write_diag_msg(fill_type,sect_count,arg_type);
	QCLI_Printf(qcli_flashlog_group, "SAMPLE PRINT FOR write diag msg fill sector COMMAND\n");
	
	return QCLI_STATUS_SUCCESS_E;
        
}

/* ============================================================================
     FUNCTION : write_diag_msg_msgnum 
	 @brief This function is used to write diag messages from qcli demo where the unit of diag messages is message packets
	 @Param Parameter_Count contains the number of arguments
	 @param Parameter list points to the list of parameters
***========================================================================== */
QCLI_Command_Status_t write_diag_msg_msgnum(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{   
   
    uint32_t sect_count,arg_type;
	uint8_t fill_type=2;
		
    if (Parameter_Count > 0 && Parameter_List && Parameter_List[0].Integer_Is_Valid)
       sect_count = (Parameter_List[0].Integer_Value);
    else
       sect_count = 1;
	
    if (Parameter_Count > 1 && Parameter_List && Parameter_List[1].Integer_Is_Valid)
       arg_type = (Parameter_List[1].Integer_Value);
    else
       arg_type = 1;
   
    write_diag_msg(fill_type,sect_count,arg_type);
    QCLI_Printf(qcli_flashlog_group, "SAMPLE PRINT for write diag msg fill message number COMMAND\n");
	
	return QCLI_STATUS_SUCCESS_E;
        
}
/* ============================================================================
     FUNCTION : erase_flash_enable
	  @brief This function is used to erase flash partitions and enable flashlogging optionally
	  @Param Parameter_Count contains the number of arguments
	  @param Parameter list points to the list of parameters
***========================================================================== */

QCLI_Command_Status_t erase_flash_enable(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{   
	uint32_t disk_size = 8192; /*default sector size for flash erase is 8 KB*/
	void *fw_upgrade_cxt;
	qbool_t isEnable=1;/*flag to call the method only for erasing the flash partitions.*/
	uint8_t FWD_idx = 0;
	qbool_t erase_only;
	
	if (Parameter_Count > 0 && Parameter_List && Parameter_List[0].Integer_Is_Valid)
       erase_only = (Parameter_List[0].Integer_Value);
    else
       erase_only = 0;
	
 
    if( qapi_Fw_Upgrade_init() != QAPI_OK )
	{
		QCLI_Printf(qcli_flashlog_group, "fw_upgrade_init failed");
		return QCLI_STATUS_ERROR_E;
	}
 
	
    FWD_idx = qapi_Fw_Upgrade_Get_Active_FWD(NULL, NULL);
	
    if( qapi_Fw_Upgrade_Find_Partition(FWD_idx, 100, &fw_upgrade_cxt) != QAPI_OK ) /*Get the flash handle*/
	{
		QCLI_Printf(qcli_flashlog_group, "Unable to get flash handle"); 
        return QCLI_STATUS_ERROR_E;		
    }

    qapi_Fw_Upgrade_Get_Partition_Size(fw_upgrade_cxt, &disk_size); /*give the partition size*/
                
    
    if( qapi_Fw_Upgrade_Erase_Partition(fw_upgrade_cxt, 0, disk_size) != QAPI_OK ) /*erase partition*/
	{
        QCLI_Printf(qcli_flashlog_group, "Flash erase failed");
		return QCLI_STATUS_ERROR_E;
    }                
    qapi_Fw_Upgrade_Close_Partition(fw_upgrade_cxt); /*close the partition handle*/

	QCLI_Printf(qcli_flashlog_group, "Flash erased successfully");
	
	if(!erase_only)
	{
	    if(_qapi_Diag_Enable_FlashLogs(isEnable) != 1 ) /*Get the flash handle*/
	    {
	
		    QCLI_Printf(qcli_flashlog_group, "Unable to enable FLASHLOG");    
			return QCLI_STATUS_ERROR_E;
        }
	}
	
	return QCLI_STATUS_SUCCESS_E;
        
}

/* ============================================================================
     FUNCTION : erase_flash_enable
	  @brief This function is used to enable and disable flash logging
	  @Param Parameter_Count contains the number of arguments
	  @param Parameter list points to the list of parameters
***========================================================================== */
QCLI_Command_Status_t flash_enable_disable(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{   
   
	qbool_t isEnable=1;
	
    if (Parameter_Count > 0 && Parameter_List && Parameter_List[0].Integer_Is_Valid)
       isEnable = (Parameter_List[0].Integer_Value);
    else
       isEnable = 1;/*enabled by default*/
	
	if(_qapi_Diag_Enable_FlashLogs(isEnable) != 1 ) /*Get the flash handle*/
	{
		QCLI_Printf(qcli_flashlog_group, "Unable to enable FLASHLOG");    
		return QCLI_STATUS_ERROR_E;
    }
	else if(isEnable)
		QCLI_Printf(qcli_flashlog_group, " enabled FLASHLOG");    
	else
		QCLI_Printf(qcli_flashlog_group, " disabled FLASHLOG");   
	return QCLI_STATUS_SUCCESS_E;
        
}


