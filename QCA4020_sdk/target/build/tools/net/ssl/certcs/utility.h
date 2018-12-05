/*
 * Copyright (c) 2014-2018 Qualcomm Technologies, Inc.
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

#ifndef UTILITY_H
#define UTILITY_H

typedef struct {
    /** log level:
     *  0 = notice:  logNote()
     *  1 = normal:  logNorm()
     *  2 = verbose: logVerb()
     *  3 = debug:   logDebug()
     */
    char level;
    /** True to used syslog instead of console */
    char useSyslog;
    /** Bit mask of items to log (according to LogMask) */
    int mask;
    char *timeStampFormat;
    /** Name of error file where all ERROR messages are written to */
    char *errorFilename;
    /** Name used for syslog */
    char *syslogName;
} logConfig_t;

void logInit(const char *appName, char useSyslog, char level, int mask);

void logGetConfig(logConfig_t *logConfig);
void logSetConfig(const logConfig_t *logConfig);

int logGetMask(void);

void logSysError(const char *format, ...);
void logError(const char *format, ...);
void logWarn(const char *format, ...);

void logMasked(int flag, const char *format, ...);

void logNote(const char *format, ...);
void logNorm(const char *format, ...);
void logVerb(const char *format, ...);
void logDebug(const char *format, ...);
void logTrace(const char *format, ...);

void logHexStr(int level, const char *message, const void *buf, unsigned int len);

void hexdump(const void *addr, unsigned int len, unsigned char columns);

int toHexString(char *buf, size_t size, const void *addr, size_t len, char insertSpace);
int fromHexString(void *outBuf, size_t size, const char *hexstr);

void dieSysError(const char *format, ...);
void dieError(const char *format, ...);

void logToErrorFile(const char *msg);
int logTruncateErrorfile(int minNumLines, int maxNumLines);

void strToUpper(char *str);
void strToLower(char *str);

#endif // UTILITY_H
