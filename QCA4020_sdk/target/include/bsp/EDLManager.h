/*
 * Copyright (c) 2016-2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __EDLMANAGER_H__
#define __EDLMANAGER_H__

#include <stdint.h>

   /* The following represent the error codes that can be returned from */
   /* the functions in this module.                                     */
#define EDL_MANAGER_ERROR_SUCCESS                 (0)
#define EDL_MANAGER_ERROR_INVALID_PARAMETER      (-1)
#define EDL_MANAGER_ERROR_FILE_NOT_FOUND         (-2)
#define EDL_MANAGER_ERROR_READ_ERROR             (-3)
#define EDL_MANAGER_ERROR_INSUFFICIENT_RESOURCES (-4)
#define EDL_MANAGER_ERROR_ALREADY_INITIALIZED    (-5)

   /* The list of allowed file IDs supported by this module.            */
#define EDL_FILE_ID_DEFAULT                     0
#define EDL_FILE_ID_USER                        1
#define EDL_FILE_ID_SYSTEM                      2

   /* The following structure holds all information that corresponds to */
   /* an EDL NVM binary object that is to be downloaded.                */
typedef struct EDL_File_Info_s
{
   void     *Data;
   uint32_t  DataLength;
   uint16_t  Flags;
} EDL_File_Info_t;

   /* Currently defined Flags for the EDL_File_Info_t Flags member.  All*/
   /* other fields are reserved.                                        */
#define EDL_FILE_INFO_FLAGS_ENCRYPTED        (0x0001)

/**
   @brief Initialize the EDL Manager.

   This function is responsible for initializing all data structures and allocating
   all resources required by the EDL Manager.

   @return
      0 upon success, a negative error code otherwise.
*/
int EDLManager_Initialize(void);

/**
   @brief De-initialize the EDL Manager.

   This function is responsible for freeing all resources that are currently
   being used by the EDL Manager.  After this function is called no other
   functions in this module can be called until the EDL Manager is initialized
   again via successful call to the EDLManager_Initialize() function.

   @return
      none
*/
void EDLManager_Cleanup(void);

/**
   @brief Reets the EDL Manager.

   This function is responsible for resetting the state of the EDL manager so
   that all EDL files can be downloaded to the M0. It is called when the M0 is
   transitioning to FMM wihtout a corresponding FOM transition.

   @return
      none
*/
void EDLManager_Reset(void);

/**
   @brief Loads an EDL file from the filesystem and returns its info.

   This function is used to load an NVM EDL file from the flash filesystem and
   populate an info structure so that it can be sent to the M0 for tag parsing.
   After a file is done being used, EDLManager_Free_EDL_File should be called.

   @param EDLFileID
      The ID of the file to load from the filesystem.

   @param[out] EDLFileInfo
      The file information structure to populate upon success.

   @return
      0 upon success, a negative error code otherwise.
*/
int EDLManager_Load_EDL_File(uint8_t EDLFileID, EDL_File_Info_t *EDLFileInfo);

/**
   @brief Frees a previously-loaded EDL file.

   This function is called after the EDL file data is no longer needed. Normally
   this is after the M0 returns a confirmation that all the data was processed.

   @param[in] EDLFileInfo
      The file information structure to be freed.

   @return
      0 upon success, a negative error code otherwise.
*/
int EDLManager_Free_EDL_File(EDL_File_Info_t *EDLFileInfo);

#endif
