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

#ifndef QOSA_UTIL_H
#define QOSA_UTIL_H

#define LOG_LVL_AT       (1<<0)
#define LOG_LVL_ERR      (1<<1)
#define LOG_LVL_WARN     (1<<2)
#define LOG_LVL_INFO     (1<<3)
#define LOG_LVL_DEBUG    (1<<4)

#define LOG_LVL_DEFAULT LOG_LVL_AT
#define MAX_LOG_LEVEL  (LOG_LVL_AT | LOG_LVL_ERR | LOG_LVL_WARN | LOG_LVL_INFO | LOG_LVL_DEBUG)

void QCLI_Printf(const char *format, ...);
extern uint32_t LogLevel;

#define LOG_AT(fmt, ...)    \
	do { \
		if (LogLevel & LOG_LVL_AT) \
	      	{ \
			QCLI_Printf(fmt, ##__VA_ARGS__); \
	       	} \
	} while(0)

#define LOG_AT_OK()         \
	do { \
		{ QCLI_Printf("\r\nOK\r\n"); } \
	} while(0)

#define LOG_AT_ERROR()      \
	do { \
		{ QCLI_Printf("\r\nERROR\r\n"); } \
	} while(0)

#define LOG_AT_EVT(fmt, ...)    \
    do { \
      	{ QCLI_Printf(fmt, ##__VA_ARGS__); }\
    } while(0)

#ifdef ENABLE_LOG
#define LOG_ERR(fmt, ...)   \
	do { \
	       	if (LogLevel & LOG_LVL_ERR) \
	      	{ \
		       	QCLI_Printf("(E)"fmt, ##__VA_ARGS__); \
	       	} \
       	} while(0)

#define LOG_WARN(fmt, ...)  \
	do { \
	       	if (LogLevel & LOG_LVL_WARN) \
	       	{ \
		       QCLI_Printf("(W)"fmt, ##__VA_ARGS__); \
	       	} \
	} while(0)

#define LOG_INFO(fmt, ...)  \
	do { \
	       	if (LogLevel & LOG_LVL_INFO) \
		{ \
			QCLI_Printf("(I)"fmt, ##__VA_ARGS__); \
		} \
	} while(0)

#define LOG_DEBUG(fmt, ...) \
	do { \
		if (LogLevel & LOG_LVL_DEBUG) \
		{ \
			QCLI_Printf("(D)"fmt, ##__VA_ARGS__); \
		} \
	} while(0)
#else
#define LOG_ERR(fmt, ...)    do { } while(0)
#define LOG_WARN(fmt, ...)   do { } while(0)
#define LOG_INFO(fmt, ...)   do { } while(0)
#define LOG_DEBUG(fmt, ...)  do { } while(0)
#endif

void qc_api_SetLogLevel(int32_t Level);
int32_t qc_api_GetLogLevel(void);

#endif
