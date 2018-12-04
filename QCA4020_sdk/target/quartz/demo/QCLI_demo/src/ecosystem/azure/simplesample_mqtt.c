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

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

/* This sample uses the _LL APIs of iothub_client for example purposes.
That does not mean that MQTT only works with the _LL APIs.
Simply changing the using the convenience layer (functions not having _LL)
and removing calls to _DoWork will yield the same results. */

#ifdef ARDUINO
#include "AzureIoTHub.h"
#else
#include "azure_c_shared_utility/threadapi.h"
#include "azure_c_shared_utility/platform.h"
#include "serializer.h"
#include "iothub_client_ll.h"
#include "iothubtransportmqtt.h"
#endif
#include "certs.h"
#include "qcli_api.h"

extern QCLI_Group_Handle_t qcli_ecosystem_handle;

/*String containing Hostname, Device Id & Device Key in the format:             */
/*  "HostName=<host_name>;DeviceId=<device_id>;SharedAccessKey=<device_key>"    */

static const char* connectionString = 
"HostName=Quartz-IOT-Hub.azure-devices.net;DeviceId=qca402X_2;SharedAccessKey=z49SOy1ft5dNaW6Qukwzb1HqFY1ZN8mGFhrgBQnyoO4=";

//static const char* connectionString = "HostName=qca402x951e5.azure-devices.net;DeviceId=qcaiot509_2;x509=true";

/*Flag to start/stop the test*/
static int stop_simple_test = 0;

// Define the Model
BEGIN_NAMESPACE(WeatherStation);

DECLARE_MODEL(ContosoAnemometer,
WITH_DATA(ascii_char_ptr, DeviceId),
WITH_DATA(int, WindSpeed),
WITH_DATA(float, Temperature),
WITH_DATA(float, Humidity),
WITH_ACTION(TurnFanOn),
WITH_ACTION(TurnFanOff),
WITH_ACTION(SetAirResistance, int, Position)
);

END_NAMESPACE(WeatherStation);

static char propText[1024];



EXECUTE_COMMAND_RESULT TurnFanOn(ContosoAnemometer* device)
{
    (void)device;
    (void)QCLI_Printf(qcli_ecosystem_handle,"Turning fan on.\r\n");
    return EXECUTE_COMMAND_SUCCESS;
}

EXECUTE_COMMAND_RESULT TurnFanOff(ContosoAnemometer* device)
{
    (void)device;
    (void)QCLI_Printf(qcli_ecosystem_handle,"Turning fan off.\r\n");
    return EXECUTE_COMMAND_SUCCESS;
}

EXECUTE_COMMAND_RESULT SetAirResistance(ContosoAnemometer* device, int Position)
{
    (void)device;
    (void)QCLI_Printf(qcli_ecosystem_handle,"Setting Air Resistance Position to %d.\r\n", Position);
    return EXECUTE_COMMAND_SUCCESS;
}

void sendCallback(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void* userContextCallback)
{
    unsigned int messageTrackingId = (unsigned int)(uintptr_t)userContextCallback;

    (void)QCLI_Printf(qcli_ecosystem_handle,"Message Id: %u Received.\r\n", messageTrackingId);

    (void)QCLI_Printf(qcli_ecosystem_handle,"Result Call Back Called! Result is: %s \r\n", ENUM_TO_STRING(IOTHUB_CLIENT_CONFIRMATION_RESULT, result));
}

static void sendMessage(IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle, const unsigned char* buffer, size_t size, ContosoAnemometer *myWeather)
{
    static unsigned int messageTrackingId;
    IOTHUB_MESSAGE_HANDLE messageHandle = IoTHubMessage_CreateFromByteArray(buffer, size);
    if (messageHandle == NULL)
    {
        QCLI_Printf(qcli_ecosystem_handle,"unable to create a new IoTHubMessage\r\n");
    }
    else
    {
        MAP_HANDLE propMap = IoTHubMessage_Properties(messageHandle);
        (void)sprintf_s(propText, sizeof(propText), myWeather->Temperature > 28 ? "true" : "false");
        if (Map_AddOrUpdate(propMap, "temperatureAlert", propText) != MAP_OK)
        {
            (void)QCLI_Printf(qcli_ecosystem_handle,"ERROR: Map_AddOrUpdate Failed!\r\n");
        }

        if (IoTHubClient_LL_SendEventAsync(iotHubClientHandle, messageHandle, sendCallback, (void*)(uintptr_t)messageTrackingId) != IOTHUB_CLIENT_OK)
        {
            QCLI_Printf(qcli_ecosystem_handle,"failed to hand over the message to IoTHubClient");
        }
        else
        {
            QCLI_Printf(qcli_ecosystem_handle,"IoTHubClient accepted the message for delivery\r\n");
        }
        IoTHubMessage_Destroy(messageHandle);
    }
    messageTrackingId++;
}

/*this function "links" IoTHub to the serialization library*/
static IOTHUBMESSAGE_DISPOSITION_RESULT IoTHubMessage(IOTHUB_MESSAGE_HANDLE message, void* userContextCallback)
{
    IOTHUBMESSAGE_DISPOSITION_RESULT result;
    const unsigned char* buffer;
    size_t size;
    if (IoTHubMessage_GetByteArray(message, &buffer, &size) != IOTHUB_MESSAGE_OK)
    {
        QCLI_Printf(qcli_ecosystem_handle,"unable to IoTHubMessage_GetByteArray\r\n");
        result = IOTHUBMESSAGE_ABANDONED;
    }
    else
    {
        /*buffer is not zero terminated*/
        char* temp = malloc(size + 1);
        if (temp == NULL)
        {
            QCLI_Printf(qcli_ecosystem_handle,"failed to malloc\r\n");
            result = IOTHUBMESSAGE_ABANDONED;
        }
        else
        {
            (void)memcpy(temp, buffer, size);
            temp[size] = '\0';
            EXECUTE_COMMAND_RESULT executeCommandResult = EXECUTE_COMMAND(userContextCallback, temp);
            result =
                (executeCommandResult == EXECUTE_COMMAND_ERROR) ? IOTHUBMESSAGE_ABANDONED :
                (executeCommandResult == EXECUTE_COMMAND_SUCCESS) ? IOTHUBMESSAGE_ACCEPTED :
                IOTHUBMESSAGE_REJECTED;
            free(temp);
        }
    }
    return result;
}

void simplesample_mqtt_run(char* connString)
{
	stop_simple_test = 0;
    if (platform_init() != 0)
    {
        (void)QCLI_Printf(qcli_ecosystem_handle,"Failed to initialize platform.\r\n");
    }
    else
    {
        if (serializer_init(NULL) != SERIALIZER_OK)
        {
            (void)QCLI_Printf(qcli_ecosystem_handle,"Failed on serializer_init\r\n");
        }
        else
        {
            IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle = IoTHubClient_LL_CreateFromConnectionString(connString, MQTT_Protocol);
            srand((unsigned int)time(NULL));
            int avgWindSpeed = 10;
            float minTemperature = 20.0;
            float minHumidity = 60.0;

            if (iotHubClientHandle == NULL)
            {
                (void)QCLI_Printf(qcli_ecosystem_handle,"Failed on IoTHubClient_LL_Create\r\n");
            }
            else
            {

                // For mbed add the certificate information
                if (IoTHubClient_LL_SetOption(iotHubClientHandle, "TrustedCerts", certificates) != IOTHUB_CLIENT_OK)
                {
                    (void)QCLI_Printf(qcli_ecosystem_handle,"failure to set option \"TrustedCerts\"\r\n");
                }
                

                ContosoAnemometer* myWeather = CREATE_MODEL_INSTANCE(WeatherStation, ContosoAnemometer);
                if (myWeather == NULL)
                {
                    (void)QCLI_Printf(qcli_ecosystem_handle,"Failed on CREATE_MODEL_INSTANCE\r\n");
                }
                else
                {
                    if (IoTHubClient_LL_SetMessageCallback(iotHubClientHandle, IoTHubMessage, myWeather) != IOTHUB_CLIENT_OK)
                    {
                        QCLI_Printf(qcli_ecosystem_handle,"unable to IoTHubClient_SetMessageCallback\r\n");
                    }
                    else
                    {
                        myWeather->DeviceId = "myFirstDevice";
                        myWeather->WindSpeed = avgWindSpeed + (rand() % 4 + 2);
                        myWeather->Temperature = minTemperature + (rand() % 10);
                        myWeather->Humidity = minHumidity + (rand() % 20);
                        {
                            unsigned char* destination;
                            size_t destinationSize;
                            if (SERIALIZE(&destination, &destinationSize, myWeather->DeviceId, myWeather->WindSpeed, myWeather->Temperature, myWeather->Humidity) != CODEFIRST_OK)
                            {
                                (void)QCLI_Printf(qcli_ecosystem_handle,"Failed to serialize\r\n");
                            }
                            else
                            {
                                sendMessage(iotHubClientHandle, destination, destinationSize, myWeather);
                                free(destination);
                            }
                        }

                        /* wait for commands */
                        while (stop_simple_test == 0)
                        {
                            IoTHubClient_LL_DoWork(iotHubClientHandle);
                            ThreadAPI_Sleep(100);
                        }
                    }

                    DESTROY_MODEL_INSTANCE(myWeather);
                }
                IoTHubClient_LL_Destroy(iotHubClientHandle);
            }
            serializer_deinit();
        }
        platform_deinit();
    }
}

QCLI_Command_Status_t azure_simple_init(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	if(Parameter_Count > 0){
		if(Parameter_List[0].String_Value != NULL) {
			simplesample_mqtt_run(Parameter_List[0].String_Value);
		}else{
			QCLI_Printf(qcli_ecosystem_handle,"ERROR: Connection String not provided\n");
		}
	} else {
		QCLI_Printf(qcli_ecosystem_handle, "Missing Parameters\n");
	}
    
	return QCLI_STATUS_SUCCESS_E;
}

void simplesample_mqtt_stop(void)
{

	stop_simple_test = 1;
}