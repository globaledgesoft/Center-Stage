/*
 * Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
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

 
 /*-------------------------------------------------------------------------
 * Include Files
 *-----------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "qapi_types.h"

#include "pal.h"

#include "qurt_error.h"
#include "qurt_thread.h"
#include "qurt_signal.h"
#include "qurt_timer.h"

/*-------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 *-----------------------------------------------------------------------*/
#define Sleep(msec)    do { \
                              qurt_time_t qtime;\
                              qtime = qurt_timer_convert_time_to_ticks(msec, QURT_TIME_MSEC);\
                              qurt_thread_sleep(qtime);\
                          } while (0)

#define THREAD_STACK_SIZE                           (3072)
#define THREAD_PRIORITY                             (10)

 
 /*-------------------------------------------------------------------------
 * Function Definitions
 *-----------------------------------------------------------------------*/
 
/**
   @brief This function represents the main thread of execution.
*/
static void HelloWorld_Thread(void *Param)
{
   /* Say Hello World */
   while(true)
   {
      PAL_CONSOLE_WRITE_STRING_LITERAL("Hello World\r\n");
      PAL_CONSOLE_WRITE_STRING_LITERAL(PAL_OUTPUT_END_OF_LINE_STRING);
      Sleep(1000); //sleep for 1 sec
   }
}

/**
   @brief This function is used to pre initialize resource 
          required before starting the demo this is called
 		  from app_init function in pal.c
 */
void Initialize_Demo(void)
{
   PAL_CONSOLE_WRITE_STRING_LITERAL("Hello World demo - Welcome to QCA IoE \r\n");
   PAL_CONSOLE_WRITE_STRING_LITERAL(PAL_OUTPUT_END_OF_LINE_STRING);
}

/**
   @brief This function is used to start the demo thread this
          is called from app_init function in pal.c
 */
void App_Start(qbool_t ColdBoot)
{
   qurt_thread_attr_t Thread_Attribte;
   qurt_thread_t      Thread_Handle;
   int                Result;

   /* Start the main demo app. */
   qurt_thread_attr_init(&Thread_Attribte);
   qurt_thread_attr_set_name(&Thread_Attribte, "HelloThread");
   qurt_thread_attr_set_priority(&Thread_Attribte, THREAD_PRIORITY);
   qurt_thread_attr_set_stack_size(&Thread_Attribte, THREAD_STACK_SIZE);
   Result = qurt_thread_create(&Thread_Handle, &Thread_Attribte, HelloWorld_Thread, NULL);
   if(Result != QURT_EOK)
   {
      PAL_CONSOLE_WRITE_STRING_LITERAL("Failed to start Hello World Main thread.");
      PAL_CONSOLE_WRITE_STRING_LITERAL(PAL_OUTPUT_END_OF_LINE_STRING);
      PAL_CONSOLE_WRITE_STRING_LITERAL(PAL_OUTPUT_END_OF_LINE_STRING);
   }
}

