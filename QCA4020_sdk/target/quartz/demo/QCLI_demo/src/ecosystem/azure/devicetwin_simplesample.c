// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.
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
 
#include <stdio.h>
#include <inttypes.h>
#include "serializer_devicetwin.h"
#include "iothub_client.h"
#include "iothubtransportmqtt.h"
#include "azure_c_shared_utility/threadapi.h"
#include "parson.h"
#include "azure_c_shared_utility/platform.h"
#include "qcli_api.h"
#include "certs.h"

extern QCLI_Group_Handle_t qcli_ecosystem_handle;

/*String containing Hostname, Device Id & Device Key in the format:             */
/*  "HostName=<host_name>;DeviceId=<device_id>;SharedAccessKey=<device_key>"    */

static const char* connectionString = "HostName=Quartz-IOT-Hub.azure-devices.net;DeviceId=qca402X_1;SharedAccessKey=ajgqIgkW4VYs9yJrDX8+QWEiS/QeRC8yZpx4lAzvuN4=";
/*Flag to start/stop the test*/
static int stop_twin_test = 0;
static char inputString[64];
static int reported_state_interval;

// Define the Model - it is a car.
BEGIN_NAMESPACE(Contoso);

DECLARE_STRUCT(Maker,
    ascii_char_ptr, makerName, /*Fabrikam, Contoso ... */
    ascii_char_ptr, style, /*sedan, minivan ...*/
    int, year
);

DECLARE_STRUCT(Geo,
    double, longitude,
    double, latitude
);

DECLARE_MODEL(CarState,
    WITH_REPORTED_PROPERTY(int32_t, softwareVersion),
    WITH_REPORTED_PROPERTY(uint8_t, reported_maxSpeed),
    WITH_REPORTED_PROPERTY(ascii_char_ptr, vanityPlate),
	WITH_REPORTED_PROPERTY(float, oilLevelPercentage)
);

DECLARE_MODEL(CarSettings,
    WITH_DESIRED_PROPERTY(uint8_t, desired_maxSpeed, onDesiredMaxSpeed),
    WITH_DESIRED_PROPERTY(Geo, location)
);

DECLARE_DEVICETWIN_MODEL(Car,
    WITH_REPORTED_PROPERTY(ascii_char_ptr, lastOilChangeDate), /*this is a simple reported property*/
    WITH_DESIRED_PROPERTY(ascii_char_ptr, changeOilReminder),
    
    WITH_REPORTED_PROPERTY(Maker, maker), /*this is a structured reported property*/
    WITH_REPORTED_PROPERTY(CarState, state), /*this is a model in model*/
    WITH_DESIRED_PROPERTY(CarSettings, settings), /*this is a model in model*/
    WITH_METHOD(getCarVIN),
	WITH_METHOD(sendDevReportedState)
);

END_NAMESPACE(Contoso);

DEFINE_ENUM_STRINGS(DEVICE_TWIN_UPDATE_STATE, DEVICE_TWIN_UPDATE_STATE_VALUES);

Car* car = NULL;

METHODRETURN_HANDLE getCarVIN(Car* car)
{
    (void)(car);
	QCLI_Printf(qcli_ecosystem_handle,"Returning Car VIN \n");
    /*Car VINs are JSON strings, for example: 1HGCM82633A004352*/
    METHODRETURN_HANDLE result = MethodReturn_Create(201, "\"1HGCM82633A004352\"");
    return result;
}

void deviceTwinCallback(int status_code, void* userContextCallback)
{
    (void)(userContextCallback);
    QCLI_Printf(qcli_ecosystem_handle,"IoTHub: reported properties delivered with status_code = %u\n", status_code);
}

static void deviceTwinGetStateCallback(DEVICE_TWIN_UPDATE_STATE update_state, const unsigned char* payLoad, size_t size, void* userContextCallback)
{
    (void)userContextCallback;
    QCLI_Printf(qcli_ecosystem_handle,"Device Twin properties received: update=%s payload=%s, size=%zu\n", ENUM_TO_STRING(DEVICE_TWIN_UPDATE_STATE, update_state), payLoad, size);
}

void onDesiredMaxSpeed(void* argument)
{
    /*by convention 'argument' is of the type of the MODEL encompassing the desired property*/
    /*in this case, it is 'CarSettings'*/

    CarSettings* car = argument;
    QCLI_Printf(qcli_ecosystem_handle,"received a new desired_maxSpeed = %" PRIu8 "\n", car->desired_maxSpeed);

}

METHODRETURN_HANDLE sendDevReportedState(Car* car)
{
	METHODRETURN_HANDLE result;
	/*sending the values to IoTHub*/
	if ((IoTHubDeviceTwin_SendReportedStateCar(car, deviceTwinCallback, NULL)) != IOTHUB_CLIENT_OK)
	{
		QCLI_Printf(qcli_ecosystem_handle,"Failed sending serialized reported state\n");
	}
	else
	{
		QCLI_Printf(qcli_ecosystem_handle,"Reported state will be send to IoTHub\n");
	}
	return result;
}

#if 1


#include <sys/reent.h>
#include <stdlib.h>
void *_malloc_r(
  struct _reent *ignored __attribute__((unused)),
  size_t         size
)
{
  return malloc( size );
}

void *_malloc_trim_r(
  struct _reent *ignored __attribute__((unused)),
  size_t         size
)
{
  return 0;
}

void
_free_r (struct _reent *r, _PTR x)
{
  free (x);
}

_PTR 
_realloc_r (struct _reent *r, _PTR x, size_t sz)
{
  return realloc (x, sz);
}

_PTR 
_calloc_r (struct _reent *r, size_t a, size_t b)
{	
	return calloc(a,b);
}
#endif
void device_twin_simple_sample_run(char* connString, char* x509cert)
{
	static int iteration = 0;
	stop_twin_test = 0;

    /*prepare the platform*/
	
    if (platform_init() != 0)
    {
        QCLI_Printf(qcli_ecosystem_handle,"Failed to initialize the platform.\n");
    }
    else
    {
        if (SERIALIZER_REGISTER_NAMESPACE(Contoso) == NULL)
        {
            LogError("unable to SERIALIZER_REGISTER_NAMESPACE");
        }
        else
        {
            /*create an IoTHub client*/
            IOTHUB_CLIENT_HANDLE iotHubClientHandle = IoTHubClient_CreateFromConnectionString(connString, MQTT_Protocol);
            if (iotHubClientHandle == NULL)
            {
                QCLI_Printf(qcli_ecosystem_handle,"Failure creating IoTHubClient handle");
            }
            else
            {
				(void)IoTHubClient_SetOption(iotHubClientHandle, "TrustedCerts", certificates);
				
				/*Set x509 certificate if provided*/
				if(x509cert != NULL)
					IoTHubClient_SetOption(iotHubClientHandle, "x509", x509cert);
				
                // Turn on Log 
                bool trace = true;
                (void)IoTHubClient_SetOption(iotHubClientHandle, "logtrace", &trace);

                car = IoTHubDeviceTwin_CreateCar(iotHubClientHandle);
                if (car == NULL)
                {
                    QCLI_Printf(qcli_ecosystem_handle,"Failure in IoTHubDeviceTwin_CreateCar");
                }
                else
                {
                    /*setting values for reported properties*/
                    car->lastOilChangeDate = "2016";
                    car->maker.makerName = "Fabrikam";
                    car->maker.style = "sedan";
                    car->maker.year = 2014;
                    car->state.reported_maxSpeed = 100;
                    car->state.softwareVersion = 1;
                    car->state.vanityPlate = "1I1";
					car->state.oilLevelPercentage = 10.5;

                    /*sending the values to IoTHub*/
                    if (IoTHubDeviceTwin_SendReportedStateCar(car, deviceTwinCallback, NULL) != IOTHUB_CLIENT_OK)
                    {
                        (void)QCLI_Printf(qcli_ecosystem_handle,"Failed sending serialized reported state\n");
                    }
                    else
                    {
                        QCLI_Printf(qcli_ecosystem_handle,"Reported state will be send to IoTHub\n");
				/*		 if (IoTHubClient_SetDeviceTwinCallback(iotHubClientHandle, deviceTwinGetStateCallback, NULL) != IOTHUB_CLIENT_OK)
                        {
                            (void)printf("Failed subscribing for device twin properties\n");
                        }*/
                    }

					while(stop_twin_test == 0){
						ThreadAPI_Sleep(1000);
						if(reported_state_interval && (iteration++ % reported_state_interval) == 0){
							sendDevReportedState(car);
						}
					}
                }
                IoTHubDeviceTwin_DestroyCar(car);
            }
            IoTHubClient_Destroy(iotHubClientHandle);
        }
    }
    platform_deinit();
}

void device_twin_simple_sample_stop(void)
{
    stop_twin_test = 1;
    return;
}

void update_twin_reported_properties(uint8_t maxSpeed, int32_t version, char* vanity, char* oilLevelPercent)
{
	int len = strlen(vanity);
	if(len > 63)
		len = 63;
	
	if(car){
		strncpy(inputString, vanity, len+1);
		car->state.reported_maxSpeed = maxSpeed;
		car->state.softwareVersion = version;
		car->state.vanityPlate = inputString;
		car->state.oilLevelPercentage = atof(oilLevelPercent);
		
		QCLI_Printf(qcli_ecosystem_handle," Updated reported properties maxSpeed %d, version %d, vanity %s, oil level percent %f \n",car->state.reported_maxSpeed,car->state.softwareVersion, car->state.vanityPlate, car->state.oilLevelPercentage );
	}
}

QCLI_Command_Status_t azure_twin_init(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	if(Parameter_Count > 0){
		if((Parameter_List[0].Integer_Is_Valid == true) && (Parameter_List[0].Integer_Value == 1)) {
			/*Start device twin demo*/
			if(Parameter_List[1].String_Value != NULL) {
				if(Parameter_List[2].String_Value != NULL) {
					device_twin_simple_sample_run(Parameter_List[1].String_Value, Parameter_List[2].String_Value);	
				} else {
					device_twin_simple_sample_run(Parameter_List[1].String_Value, NULL);	
				}
			} else{
				QCLI_Printf(qcli_ecosystem_handle,"ERROR: Connection String not provided\n");
			}
		} else if((Parameter_List[0].Integer_Is_Valid == true) && (Parameter_List[0].Integer_Value == 2)) {	
			if (Parameter_Count > 4){
				update_twin_reported_properties(Parameter_List[1].Integer_Value, Parameter_List[2].Integer_Value, Parameter_List[3].String_Value, Parameter_List[4].String_Value);
			}else {
				QCLI_Printf(qcli_ecosystem_handle, "Missing Parameters\n");
			}
			
		} else if((Parameter_List[0].Integer_Is_Valid == true) && (Parameter_List[0].Integer_Value == 3)) {
			if (Parameter_Count > 1 && (Parameter_List[1].Integer_Is_Valid == true)){
				reported_state_interval = Parameter_List[1].Integer_Value;
			}else {
				QCLI_Printf(qcli_ecosystem_handle, "Missing Parameters\n");
			}
		} else {
				QCLI_Printf(qcli_ecosystem_handle, "Missing Parameters\n");
		}		
	} else {
		QCLI_Printf(qcli_ecosystem_handle, "Missing Parameters\n");
	}
    
	return QCLI_STATUS_SUCCESS_E;
}