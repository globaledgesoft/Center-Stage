/*
 * Copyright (c) 2015-2018 Qualcomm Technologies, Inc.
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


/**********************************************************************
 * flash_init_config.c
 *
 * Implementation of passing RAM data table from other boot environments
 * to flash driver
 *
 **********************************************************************/
/*=======================================================================
                        Edit History


when        who       what, where, why
----------  -----     ---------------------------------------------------
2017-03-03  bn        Updated Macronix's MX25R6435F supported frequency
2017-02-02  bn        Updated Winbond's suspend/resume latency parameters
2017-01-06  bn        Updated Macronix's suspend/resume latency parameters
2016-07-26  bn        Make flash_init_config independent of internal code
2016-07-14  bn        Added xip_ref_count to keep track XIP active clients
2016-01-20  bn        Initial Release

======================================================================*/

/** Flash operation status */
#define FLASH_DEVICE_DONE                 0   /**< Operation passed */
typedef int FLASH_STATUS;                     /**< Error status values used in FLASH driver */

#define MAX_XIP_REGIONS 3

#ifndef _UINT32_DEFINED
typedef  unsigned long int  uint32;      /* Unsigned 32 bit value */
#define _UINT32_DEFINED
#endif

#ifndef _UINT16_DEFINED
typedef  unsigned short     uint16;      /* Unsigned 16 bit value */
#define _UINT16_DEFINED
#endif

#ifndef _UINT8_DEFINED
typedef  unsigned char      uint8;       /* Unsigned 8  bit value */
#define _UINT8_DEFINED
#endif

typedef struct __attribute__((__packed__))
{
  uint8  xip_state_flags;
  uint16 region_size[MAX_XIP_REGIONS];
  uint32 region_base_addr[MAX_XIP_REGIONS];
  uint32 region_enable_bits;
} flash_xip_config_data;

typedef struct __attribute__((__packed__))
{
  uint32  magic_number;                   /* Magic number used to indicate the structure is valid */
  uint8   ddr_support;                    /* Is DDR Supported */
  uint8   addr_bytes;                     /* 3 or 4 Bytes Addressing Mode */
  uint8   read_mode;                      /* Read Mode: Command-Address-Data */
                                          /* READ_SDR_1_1_1_MODE = 0. READ_SDR_1_1_4_MODE = 1. READ_SDR_1_4_4_MODE = 2 */
                                          /* READ_SDR_4_4_4_MODE = 3. READ_DDR_1_1_4_MODE = 4. READ_DDR_1_4_4_MODE = 5. READ_DDR_4_4_4_MODE = 6 */
  uint8   read_sdr_opcode;                /* Read SDR mode opcode */
  uint8   read_ddr_opcode;                /* Read DDR mode opcode. Set to 0 if DDR is not supported */
  uint8   read_sdr_wait_state;            /* Read SDR Mode. Wait_state is the total of dummy cycles to wait */
  uint8   read_ddr_wait_state;            /* Read DDR mode. Wait state is the total of dummy cycles to wait */
  uint8   write_mode;                     /* Write mode: Command-Address-Data Mode */
                                          /* WRITE_SDR_1_1_1_MODE = 0. WRITE_SDR_1_1_4_MODE = 1. WRITE_SDR_1_4_4_MODE = 2 */
                                          /* WRITE_SDR_4_4_4_MODE = 3. WRITE_DDR_1_1_4_MODE = 4. WRITE_DDR_1_4_4_MODE = 5. WRITE_DDR_4_4_4_MODE = 6 */
  uint8   write_sdr_opcode;               /* Opcode used for Write in SDR mode */
  uint8   write_ddr_opcode;               /* Opcode used for Write in DDR mode */
  uint8   erase_4kb_opcode;               /* Opcode used for 4KB Block erase */
  uint8   bulk_erase_size_in_4KB;         /* Bulk erase size. Unit is 4KB */
  uint8   bulk_erase_opcode;              /* The opcode for the supported bulk erase size (larger than 4KB block erase). If bulk erase is not supported, set to 0 */
  uint8   quad_enable_mode;               /* Quad Enable Requirements (QER) as defined in the JEDEC Standard No. 216A Document */
  uint8   suspend_erase_opcode;           /* Instruction to suspend an in-progress erase */
  uint8   suspend_program_opcode;         /* Instruction to suspend an in-pogress program */
  uint8   resume_erase_opcode;            /* Instruction to resume erase operation */
  uint8   resume_program_opcode;          /* Instruction to resume program operation */
  uint8   erase_err_bmsk;                 /* Status BIT(s) in the erase_err_status_reg Register indicating if there's erase error condition */
  uint8   erase_err_status_reg;           /* register address used for polling the erase status */
  uint8   write_err_bmsk;                 /* Status BIT(s) in the write_err_status_reg Register indicating if there's write error condition */
  uint8   write_err_status_reg;           /* register address used for polling the write status */
  uint8   erase_status_polling_ms;        /* erase_err_status_reg Register status polling intervals in ms unit */
  uint8   read_sleep_time_in_10us;        /* total sleep time between reads to yeild CPU to other threads. Unit is 10us */
  uint8   write_sleep_time_in_10us;       /* total sleep time between writes to yeild CPU to other threads. Uint is 10us */     
  uint8   high_performance_mode_bmask;    /* High Performance Bit(s). Non-Macronix parts set to 0. This field is used to */
                                          /* enable High Performance Mode supported on some Macronix parts */
  uint8   read_max_transfer_in_pages_xip_enabled;          /* When XIP is enabled, this field indicates the maximum Read transfer size in unit of page (256 bytes per page) */
                                                           /* Software will break a large read transfer into smaller transfers of this size in page unit */ 
  uint16  read_page_count_between_sleep;                   /* total pages read before yielding CPU to other threads */  
  uint16  write_page_count_between_sleep;                  /* total pages write before yielding CPU to other threads */  
  uint16  write_status_polling_usec;                       /* write_err_status_reg Register status polling intervals in micro second unit */
  uint16  suspend_in_progress_erase_max_latency_in_usec;   /* delay needed after suspend in-progress erase. See JEDEC Standard No. 216A for definition */
  uint16  suspend_in_progress_program_max_latency_in_usec; /* delay needed after suspend in-progress program. See JEDEC Standard No. 216A for definition */
  uint16  erase_resume_to_suspend_interval_in_usec;        /* delay needed after sending resume erase. See JEDEC Standard No. 216A for definition */
  uint16  program_resume_to_suspend_interval_in_usec;      /* delay needed after sending resume program. See JEDEC Standard No. 216A for definition */  
  uint32  freq_khz;                                        /* Max supported Frequency */                      
  uint32  read_max_transfer_in_pages;                      /* This field indicates the maximum Read transfer size in unit of page (256 bytes per page) */   
  uint32  density_in_blocks;                               /* Device density in unit of Blocks */
  uint32  device_id;                                       /* Device ID when query with Device Read ID command 0x9F */  
  flash_xip_config_data xip_config;                        /* NOT configurable by OEM.. Data Structure to keep XIP's configuration data. */
  uint8   flash_client_ref_count;                          /* Keep track of how many clients have called flash_init() */
  uint8   power_on_delay_in_100us;                         /* Power On Reset delay in 100us unit */
  uint16  reserved_2; 
#define sr_auto_clear_mask reserved_2                      /* WORKAROUND: Auto-clear Write Block Protection Mask */
  uint32  reserved_3; 
} flash_config_data;

static flash_config_data flash_nor_entries[] =
{
  { /* Micron N25Q128A11E40. 16MB */ 
    .magic_number = 0xFACEECAF,
    .ddr_support = 0,
    .addr_bytes = 3,
    .read_mode = 2,
    .read_sdr_opcode = 0xEB,
    .read_ddr_opcode = 0,
    .read_sdr_wait_state = 10,
    .read_ddr_wait_state = 0,
    .write_mode = 1,
    .write_sdr_opcode = 0x32,
    .write_ddr_opcode = 0x00,
    .erase_4kb_opcode = 0x20,
    .bulk_erase_size_in_4KB = 16,
    .bulk_erase_opcode = 0xD8,
    .quad_enable_mode = 0,
    .suspend_erase_opcode = 0x75,
    .suspend_program_opcode = 0x75,
    .resume_erase_opcode = 0x7A,
    .resume_program_opcode = 0x7A,
    .erase_err_bmsk = 0x7E,
    .erase_err_status_reg = 0x70,
    .write_err_bmsk = 0x7E,
    .write_err_status_reg = 0x70,
    .erase_status_polling_ms = 5,
    .read_sleep_time_in_10us = 40,
    .write_sleep_time_in_10us = 40,
    .high_performance_mode_bmask = 0,
    .read_max_transfer_in_pages_xip_enabled = 64,
    .read_page_count_between_sleep = 255,
    .write_page_count_between_sleep = 64,
    .write_status_polling_usec = 100,      
    .suspend_in_progress_erase_max_latency_in_usec = 100,   
    .suspend_in_progress_program_max_latency_in_usec = 100,
    .erase_resume_to_suspend_interval_in_usec  = 200, 
    .program_resume_to_suspend_interval_in_usec  = 100,
    .freq_khz = 96000,
    .read_max_transfer_in_pages = 0x10000,
    .density_in_blocks = 4096,
    .device_id = 0x0018BB20,
    .xip_config.xip_state_flags = 0,
    .xip_config.region_enable_bits = 0,
    .flash_client_ref_count = 0,
    .power_on_delay_in_100us = 8,
    .reserved_2 = 0,
    .reserved_3 = 0,
  },
  { /* Macronix MX25R6435FM2IL0 / MX25R6435FM2IH0 8MB */
    .magic_number = 0xFACEECAF,
    .ddr_support = 0,
    .addr_bytes = 3,
    .read_mode = 2,
    .read_sdr_opcode = 0xEB,
    .read_ddr_opcode = 0,
    .read_sdr_wait_state = 6,
    .read_ddr_wait_state = 0,
    .write_mode = 2,
    .write_sdr_opcode = 0x38,
    .write_ddr_opcode = 0x00,
    .erase_4kb_opcode = 0x20,
    .bulk_erase_size_in_4KB = 0,
    .bulk_erase_opcode = 0,
    .quad_enable_mode = 2,
    .suspend_erase_opcode = 0xB0,
    .suspend_program_opcode = 0xB0,
    .resume_erase_opcode = 0x30,
    .resume_program_opcode = 0x30,
    .erase_err_bmsk = 0x40,
    .erase_err_status_reg = 0x2B,
    .write_err_bmsk = 0x20,
    .write_err_status_reg = 0x2B,
    .erase_status_polling_ms = 5,
    .read_sleep_time_in_10us = 40,
    .write_sleep_time_in_10us = 40,
    .high_performance_mode_bmask = 0x02,    
    .read_max_transfer_in_pages_xip_enabled = 64,   
    .read_page_count_between_sleep = 255,
    .write_page_count_between_sleep = 64,
    .write_status_polling_usec = 100,      
    .suspend_in_progress_erase_max_latency_in_usec = 80,   
    .suspend_in_progress_program_max_latency_in_usec = 80,
    .erase_resume_to_suspend_interval_in_usec = 360,
    .program_resume_to_suspend_interval_in_usec = 128,
    .freq_khz = 32000,
    .read_max_transfer_in_pages = 0x8000,
    .density_in_blocks = 2048,
    .device_id = 0x001728C2,
    .xip_config.xip_state_flags = 0,
    .xip_config.region_enable_bits = 0,
    .flash_client_ref_count = 0,
    .power_on_delay_in_100us = 8,
    .sr_auto_clear_mask = 0x00BC, /* WORKAROUND: Auto-clear Write Block Protection Mask */
    .reserved_3 = 0,
  },
  { /* Winbond W25Q16FWSSIQ. 2MB */
    .magic_number = 0xFACEECAF,
    .ddr_support = 0,
    .addr_bytes = 3,
    .read_mode = 2,
    .read_sdr_opcode = 0xEB,
    .read_ddr_opcode = 0,
    .read_sdr_wait_state = 6,
    .read_ddr_wait_state = 0,
    .write_mode = 1,
    .write_sdr_opcode = 0x32,
    .write_ddr_opcode = 0x00,
    .erase_4kb_opcode = 0x20,
    .bulk_erase_size_in_4KB = 16,
    .bulk_erase_opcode = 0xD8,
    .quad_enable_mode = 1,
    .suspend_erase_opcode = 0x75,
    .suspend_program_opcode = 0x75,
    .resume_erase_opcode = 0x7A,
    .resume_program_opcode = 0x7A,
    .erase_err_bmsk = 0,
    .erase_err_status_reg = 0,
    .write_err_bmsk = 0,
    .write_err_status_reg = 0,
    .erase_status_polling_ms = 5,
    .read_sleep_time_in_10us = 40,
    .write_sleep_time_in_10us = 40,
    .high_performance_mode_bmask = 0,
    .read_max_transfer_in_pages_xip_enabled = 64,
    .read_page_count_between_sleep = 255,
    .write_page_count_between_sleep = 64,
    .write_status_polling_usec = 100,      
    .suspend_in_progress_erase_max_latency_in_usec = 80,   
    .suspend_in_progress_program_max_latency_in_usec = 80,
    .erase_resume_to_suspend_interval_in_usec  = 300, 
    .program_resume_to_suspend_interval_in_usec  = 100,
    .freq_khz = 64000,
    .read_max_transfer_in_pages = 0x2000,
    .density_in_blocks = 512,
    .device_id = 0x001560EF,
    .xip_config.xip_state_flags = 0,
    .xip_config.region_enable_bits = 0,
    .flash_client_ref_count = 0,
    .power_on_delay_in_100us = 50,
    .reserved_2 = 0,
    .reserved_3 = 0,
  },
  { /* Macronix MX25R1635FM2IH1. 2MB */
    .magic_number = 0xFACEECAF,
    .ddr_support = 0,
    .addr_bytes = 3,
    .read_mode = 2,
    .read_sdr_opcode = 0xEB,
    .read_ddr_opcode = 0,
    .read_sdr_wait_state = 6,
    .read_ddr_wait_state = 0,
    .write_mode = 2,
    .write_sdr_opcode = 0x38,
    .write_ddr_opcode = 0x00,
    .erase_4kb_opcode = 0x20,
    .bulk_erase_size_in_4KB = 0,
    .bulk_erase_opcode = 0,
    .quad_enable_mode = 2,
    .suspend_erase_opcode = 0xB0,
    .suspend_program_opcode = 0xB0,
    .resume_erase_opcode = 0x30,
    .resume_program_opcode = 0x30,
    .erase_err_bmsk = 0x40,
    .erase_err_status_reg = 0x2B,
    .write_err_bmsk = 0x20,
    .write_err_status_reg = 0x2B,
    .erase_status_polling_ms = 5,
    .read_sleep_time_in_10us = 40,
    .write_sleep_time_in_10us = 40,
    .high_performance_mode_bmask = 0x02,    
    .read_max_transfer_in_pages_xip_enabled = 64,   
    .read_page_count_between_sleep = 255,
    .write_page_count_between_sleep = 64,
    .write_status_polling_usec = 100,      
    .suspend_in_progress_erase_max_latency_in_usec = 80,   
    .suspend_in_progress_program_max_latency_in_usec = 80,
    .erase_resume_to_suspend_interval_in_usec = 360,
    .program_resume_to_suspend_interval_in_usec = 128,
    .freq_khz = 64000,
    .read_max_transfer_in_pages = 0x2000,
    .density_in_blocks = 512,
    .device_id = 0x001528C2,
    .xip_config.xip_state_flags = 0,
    .xip_config.region_enable_bits = 0,
    .flash_client_ref_count = 0,
    .power_on_delay_in_100us = 8,
    .sr_auto_clear_mask = 0x00BC, /* WORKAROUND: Auto-clear Write Block Protection Mask */
    .reserved_3 = 0,
  },
  { /*Spansion S25FL128S. 16MB */ 
    .magic_number = 0xFACEECAF,
    .ddr_support = 0,
    .addr_bytes = 3,
    .read_mode = 0,
    .read_sdr_opcode = 0x0B,
    .read_ddr_opcode = 0,
    .read_sdr_wait_state = 8,
    .read_ddr_wait_state = 0,
    .write_mode = 0,
    .write_sdr_opcode = 0x02,
    .write_ddr_opcode = 0x00,
    .erase_4kb_opcode = 0x20,
    .bulk_erase_size_in_4KB = 16,
    .bulk_erase_opcode = 0xD8,
    .quad_enable_mode = 1,
    .suspend_erase_opcode = 0x75,
    .suspend_program_opcode = 0x85,
    .resume_erase_opcode = 0x7A,
    .resume_program_opcode = 0x8A,
    .erase_err_bmsk = 0x60,
    .erase_err_status_reg = 0x05,
    .write_err_bmsk = 0x60,
    .write_err_status_reg = 0x05,
    .erase_status_polling_ms = 5,
    .read_sleep_time_in_10us = 40,
    .write_sleep_time_in_10us = 40,
    .high_performance_mode_bmask = 0,
    .read_max_transfer_in_pages_xip_enabled = 64,
    .read_page_count_between_sleep = 255,
    .write_page_count_between_sleep = 64,
    .write_status_polling_usec = 100,      
    .suspend_in_progress_erase_max_latency_in_usec = 200,   
    .suspend_in_progress_program_max_latency_in_usec = 200,
    .erase_resume_to_suspend_interval_in_usec  = 200, 
    .program_resume_to_suspend_interval_in_usec  = 200,
    .freq_khz = 96000,
    .read_max_transfer_in_pages = 0x10000,
    .density_in_blocks = 4096,
    .device_id = 0x00200201,
    .xip_config.xip_state_flags = 0,
    .xip_config.region_enable_bits = 0,
    .flash_client_ref_count = 0,
    .power_on_delay_in_100us = 8,
    .reserved_2 = 0,
    .reserved_3 = 0,
  },
  { /* Macronix MX25R3235FZBIH0. 32MB */
    .magic_number = 0xFACEECAF,
    .ddr_support = 0,
    .addr_bytes = 3,
    .read_mode = 2,
    .read_sdr_opcode = 0xEB,
    .read_ddr_opcode = 0,
    .read_sdr_wait_state = 6,
    .read_ddr_wait_state = 0,
    .write_mode = 2,
    .write_sdr_opcode = 0x38,
    .write_ddr_opcode = 0x00,
    .erase_4kb_opcode = 0x20,
    .bulk_erase_size_in_4KB = 0,
    .bulk_erase_opcode = 0,
    .quad_enable_mode = 2,
    .suspend_erase_opcode = 0xB0,
    .suspend_program_opcode = 0xB0,
    .resume_erase_opcode = 0x30,
    .resume_program_opcode = 0x30,
    .erase_err_bmsk = 0x40,
    .erase_err_status_reg = 0x2B,
    .write_err_bmsk = 0x20,
    .write_err_status_reg = 0x2B,
    .erase_status_polling_ms = 5,
    .read_sleep_time_in_10us = 40,
    .write_sleep_time_in_10us = 40,
    .high_performance_mode_bmask = 0x02,
    .read_max_transfer_in_pages_xip_enabled = 64,
    .read_page_count_between_sleep = 255,
    .write_page_count_between_sleep = 64,
    .write_status_polling_usec = 100,
    .suspend_in_progress_erase_max_latency_in_usec = 80,
    .suspend_in_progress_program_max_latency_in_usec = 80,
    .erase_resume_to_suspend_interval_in_usec = 360,
    .program_resume_to_suspend_interval_in_usec = 128,
    .freq_khz = 64000,
    .read_max_transfer_in_pages = 0x20000,
    .density_in_blocks = 1024,
    .device_id = 0x001628C2,
    .xip_config.xip_state_flags = 0,
    .xip_config.region_enable_bits = 0,
    .flash_client_ref_count = 0,
    .power_on_delay_in_100us = 8,
    .sr_auto_clear_mask = 0x00BC, /* WORKAROUND: Auto-clear Write Block Protection Mask */
    .reserved_3 = 0,
  },
  { /* Winbond W25Q128JVYIQ. 16MB */
    .magic_number = 0xFACEECAF,
    .ddr_support = 0,
    .addr_bytes = 3,
    .read_mode = 2,
    .read_sdr_opcode = 0xEB,
    .read_ddr_opcode = 0,
    .read_sdr_wait_state = 6,
    .read_ddr_wait_state = 0,
    .write_mode = 1,
    .write_sdr_opcode = 0x32,
    .write_ddr_opcode = 0x00,
    .erase_4kb_opcode = 0x20,
    .bulk_erase_size_in_4KB = 16,
    .bulk_erase_opcode = 0xD8,
    .quad_enable_mode = 1,
    .suspend_erase_opcode = 0x75,
    .suspend_program_opcode = 0x75,
    .resume_erase_opcode = 0x7A,
    .resume_program_opcode = 0x7A,
    .erase_err_bmsk = 0,
    .erase_err_status_reg = 0,
    .write_err_bmsk = 0,
    .write_err_status_reg = 0,
    .erase_status_polling_ms = 5,
    .read_sleep_time_in_10us = 40,
    .write_sleep_time_in_10us = 40,
    .high_performance_mode_bmask = 0,
    .read_max_transfer_in_pages_xip_enabled = 64,
    .read_page_count_between_sleep = 255,
    .write_page_count_between_sleep = 64,
    .write_status_polling_usec = 100,      
    .suspend_in_progress_erase_max_latency_in_usec = 80,   
    .suspend_in_progress_program_max_latency_in_usec = 80,
    .erase_resume_to_suspend_interval_in_usec  = 300, 
    .program_resume_to_suspend_interval_in_usec  = 100,
    .freq_khz = 64000,
    .read_max_transfer_in_pages = 0x10000,
    .density_in_blocks = 4096,
    .device_id = 0x001840EF,
    .xip_config.xip_state_flags = 0,
    .xip_config.region_enable_bits = 0,
    .flash_client_ref_count = 0,
    .power_on_delay_in_100us = 50,
    .reserved_2 = 0,
    .reserved_3 = 0,
  },

};

/**********************************************************
 * Get the pointer to the supported NOR devices and their parameters
 *   
 * @return void* [OUT]
 *   Pointer to the table that contains the Flash's NOR config parameters 
 *   needed for flash operations
 *
 **********************************************************/
void* flash_get_config_entries_struct(void)
{
  return (void *)&flash_nor_entries[0];
}

/**********************************************************
 * Get the number of entries in the table
 *
 * @param total_nor_entries [OUT]
 *   Total number of entries in the table
 *
 * @return int [OUT]
 *   Result of the operation.
 *
 **********************************************************/
FLASH_STATUS flash_get_config_entries_count(uint32 *total_nor_entries)
{
  uint32 total_entries;

  total_entries = (sizeof(flash_nor_entries) /
                   sizeof(flash_nor_entries[0]));

  *total_nor_entries = total_entries;

  return FLASH_DEVICE_DONE;
}
