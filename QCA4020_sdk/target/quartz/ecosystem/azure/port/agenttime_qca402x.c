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

// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

/* This is a template file used for porting */

/* TODO: If all the below adapters call the standard C functions, please simply use the agentime.c that already exists. */

#include <time.h>
#include "qurt_timer.h"
#include "azure_c_shared_utility/agenttime.h"
#include "qapi/qapi_status.h"
#include "qapi/qapi_rtc.h"

time_t get_time(time_t* p)
{
	uint64_t epoch_sec = 0;
	time_t t = (qurt_timer_convert_ticks_to_time(qurt_timer_get_ticks(), QURT_TIME_MSEC))/1000;
	
	qapi_Core_RTC_GPS_Epoch_Get(&epoch_sec);
	epoch_sec = epoch_sec/1000;
	
	if(p!=NULL)
		*p = t;

	return (t+epoch_sec);
}

struct tm* get_gmtime(time_t* currentTime)
{
	/* TODO: replace the gmtime call below with your own. Note that converting to struct tm needs to be done ... 
    return gmtime(currentTime);*/
	/*Not supported for now*/
	return NULL;
}

time_t get_mktime(struct tm* cal_time)
{
	/* TODO: replace the mktime call below with your own. Note that converting to time_t needs to be done ... 
	return mktime(cal_time);*/
	/*Not supported for now*/
	return (time_t)NULL;
}

char* get_ctime(time_t* timeToGet)
{
	/* TODO: replace the ctime call below with calls available on your platform.
    return ctime(timeToGet);*/
	/*Not supported for now*/
	return NULL;
}

double get_difftime(time_t stopTime, time_t startTime)
{
	return ((double)(stopTime - startTime));
}
