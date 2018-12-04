/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
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

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
void __aeabi_assert(const char *expr, const char *file, int line)
{
   for(;;)
   {
   }
}

typedef void (*ctor_fn)(void);

extern ctor_fn __init_array_begin;
extern ctor_fn __init_array_end;

void __cpp_initialize__aeabi_(void)
{
   ctor_fn *ctors;

   /* The .init_array section is populated by both RealView and GCC as a list of
      function pointers. All that needs to be done is to go through the array in
      order, and call each function pointer. This needs to happen before any of
      the application code runs (i.e. main()). */
   for(ctors = &__init_array_begin; ctors != &__init_array_end; ctors++)
   {
      (*ctors)();
   }
}

/* Provides one of RealView's optimized versions of __cxa_vec_ctor that is used
   when no destructor is provided and no exceptions are being used. */
void *__aeabi_vec_ctor_nocookie_nodtor(void *user_array, void *(*constructor)(void *), size_t element_size, size_t element_count)
{
   size_t   index;
   intptr_t obj;

   if(constructor != NULL)
   {
      obj = (intptr_t)user_array;

      for(index = 0; index < element_count; ++index)
      {
         (*constructor)((void *)obj);
         obj += element_size;
      }
   }

   /* AEABI specifies this must return its first argument. */
   return user_array;
}

void *_sbrk(int x)
{
   return(NULL);
}

