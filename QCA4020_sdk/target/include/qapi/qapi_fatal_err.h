#ifndef __QAPI_FATAL_ERR_H__ 
#define __QAPI_FATAL_ERR_H__

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

/*=================================================================================
 *
 *                            FATAL ERROR MANAGER
 *
 *===============================================================================*/
 /** @file qapi_fatal_err.h
 *
 * @addtogroup qapi_fatal_err
 * @{
 *
 * @brief Fatal Error Manager (FEM)
 *
 * @details Complex software systems often run into unrecoverable error
 *          scenarios. These fatal errors cause the system to
 *          abruptly abort execution, since there is no recovery path. By
 *          nature, fatal errors are difficult to debug because detailed
 *          information related to the error is not preserved. The fatal
 *          error manager (FEM) service provides its clients a way to handle
 *          unrecoverable errors in a graceful debug-friendly fashion. It
 *          exposes a macro which, when called after a catastrophic error,
 *          preserves pertinent information to aid in debug before resetting
 *          the system.
 *
 * @code {.c}
 *
 *    * The code snippet below demonstrates the use of this interface. The example
 *    * dynamically allocates a region of memory, failing in which it 
 *    * asserts the code. This macro populates the debug information in a global
 *    * variable 'coredump' with line number, file name, and user parameters.
 *    * It also dumps the contents of general purpose registers and invokes
 *    * various user callbacks before resetting the system. The header file 
 *    * qapi_fatal_err.h should be included before calling the macro.
 *
 *   char * c;
 *
 *   c = malloc(sizeof(char));
 *   if ( c == NULL )
 *   {
 *     QAPI_FATAL_ERR(0,0,0);
 *   }
 *
 * @endcode
 *
 * @}
 */

/*==================================================================================

                           EDIT HISTORY FOR FILE

This section contains comments describing changes made to this file.
Notice that changes are listed in reverse chronological order.

$Header: //components/rel/core.ioe/1.0/v2/rom/release/api/debugtools/err/qapi_fatal_err.h#9 $

when       who     what, where, why
--------   ---     -----------------------------------------------------------------
10/30/15   din     Updated documentation.
09/29/15   din     Initial version.
==================================================================================*/

/*==================================================================================

                               INCLUDE FILES

==================================================================================*/

#include "qapi_types.h"

/*==================================================================================

                                   MACROS

==================================================================================*/
#if !defined(_PACKED_START)
    #if (defined(__CC_ARM) || defined(__GNUC__))
        /* ARM and GCC compilers */
        #define _PACKED_START
    #elif (defined(__ICCARM__))
    /* IAR compiler */
        #define _PACKED_START       __packed
    #else
        #define _PACKED_START
    #endif
#endif

#if !defined(_PACKED_END)
    #if (defined(__CC_ARM) || defined(__GNUC__))
        /* ARM and GCC compilers */
        #define _PACKED_END  __attribute__ ((packed))
    #elif (defined(__ICCARM__))
        /* IAR compiler */
        #define _PACKED_END
    #else
        #define _PACKED_END
    #endif
#endif

/*==================================================================================

                               TYPE DEFINITIONS

==================================================================================*/

/** @addtogroup qapi_fatal_err
@{ */

/**
* Debug information structure.
*
* This structure is used to capture the module name and line number in the 
* source file where a fatal error was detected. Reference to an instance of
* this structure is passed as a parameter to the qapi_err_fatal_internal()
*          function. 
*/
typedef _PACKED_START struct
{
  uint16_t     line;
  /**< Line number in the source module. */

  const char  *fname;
  /**< Pointer to the source file name. */ 
} _PACKED_END qapi_Err_const_t;


/**
* Debug information structure.
*
* This type is used for post crash callbacks from the error handler. A reference to an 
* instance of this type is passed as a parameter to the qapi_err_crash_cb_reg()
* and qapi_err_crash_cb_dereg() functions. 
*/
typedef void (*qapi_err_cb_ptr)(void);

/** @} */ /* end_addtogroup qapi_fatal_err */

/*==================================================================================

                            FUNCTION DECLARATIONS

==================================================================================*/

/*==================================================================================
  FUNCTION      qapi_err_fatal_internal
==================================================================================*/

/** @addtogroup qapi_fatal_err
@{ */

/**
 * Fatal error handler.
 *
 * This function implements back-end functionality supported by macro
 * QAPI_FATAL_ERR. It preserves debug information at a well-known location
 * (typically a global variable "coredump"). Preserved information captures
 * the source module name and line number, user-provided values, and contents
 *          of general purpose registers for underlying CPU architecture. After
 *          invoking several notification callbacks, it resets the system.
 *
 * @param[in] err_const Reference to the structure record line number and module name.
 * @param[in] param1    Client-provided parameter saved with debug information.
 * @param[in] param2    Client-provided parameter saved with debug information.
 * @param[in] param3    Client-provided parameter saved with debug information.
 *
 * @note1hang This function does not return. It should only be used to gracefully
 *       handle unrecoverable errors and restart the system. Clients should 
 *       not call the function directly. Instead, they should use the macro
 *       QAPI_FATAL_ERR to access the functionality to ensure that all
 *       relevant debug information is carried forward.
 */
void qapi_err_fatal_internal
(
  const qapi_Err_const_t * err_const, 
  uint32_t                 param1, 
  uint32_t                 param2, 
  uint32_t                 param3 
);

/** @} */ /* end_addtogroup qapi_fatal_err */ 

/*==================================================================================
  MACRO         QAPI_FATAL_ERR
==================================================================================*/

/** @addtogroup qapi_fatal_err
@{ */

/**
 * Fatal error handler macro.
 *
 * This function allows for graceful handling of fatal errors. It
 * preserves information related to fatal crashes at a well-known location
 * (typically a global variable "coredump"). Preserved information captures
 * the source module name and line number, user-provided values, and contents
 * of general purpose registers used by the underlying CPU architecture.
 * After invoking several notification callbacks, it resets the system. @newpage
 *
 * @param[in] param1   User-provided parameter to be logged in coredump.
 * @param[in] param2   User-provided parameter to be logged in coredump.
 * @param[in] param3   User-provided parameter to be logged in coredump.
 *
 * @note1hang This macro does not return. It should only be used to gracefully
 *       handle unrecoverable errors and restart the system.
 @hideinitializer */
#define QAPI_FATAL_ERR(param1,param2,param3)                             \
do                                                                       \
{                                                                        \
   static const qapi_Err_const_t xx_err_const = {__LINE__, __FILENAME__};\
   qapi_err_fatal_internal(&xx_err_const, param1,param2,param3);         \
}while (0)

/** @} */ /* end_addtogroup qapi_fatal_err */

/*==================================================================================
  FUNCTION      qapi_err_crash_cb_reg
==================================================================================*/

/** @addtogroup qapi_fatal_err
@{ */

/**
 * Registers a callback with the fatal crash handler.
 *
 * This function can be used to 
 * save client-specific information in the ramdumps and carry out some clean up 
 * during fatal error handling.
 *
 * @note1hang The callback must not call ERR_FATAL under any circumstance. This callback should not 
 * rely on any messaging, task switching (or system calls that may invoke task switching), 
 * interrupts, or any blocking opertaions. 
 * 
 * @param[in] cb  Client callback to be invoked during fatal crash handling.
 *
 * @return TRUE if the callback is added, false otherwise.
 */
qbool_t qapi_err_crash_cb_reg
(
  qapi_err_cb_ptr          cb 
);


/*==================================================================================
  FUNCTION      qapi_err_crash_cb_dereg
==================================================================================*/

/**
 * Deregisters a callback with the fatal crash handler.
 *
 * @param[in] cb  Client callback to be deregistered.
 *
 * @return TRUE if callback is removed, false otherwise.
 *
 */
qbool_t qapi_err_crash_cb_dereg
(
  qapi_err_cb_ptr          cb 
);

/** @} */ /* end_addtogroup qapi_fatal_err */ 

#endif /* __QAPI_FATAL_ERR_H__ */
