'''
/*
 * Copyright 2010-2017 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *  http://aws.amazon.com/apache2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */
 '''

from AWSIoTPythonSDK.MQTTLib import AWSIoTMQTTClient
import logging
import time
import json
import serial

# Custom MQTT message callback
def customCallback(client, userdata, message):
    print("Received a new message: ")
    print(message.payload)
    print("from topic: ")
    print(message.topic)
    print("--------------\n\n")

def aws_iot_pub():
    with open('../config/config.json') as f:
        config = json.load(f)
    # Read connection parameters from config file
    host = config["AWS_IOT_CLIENT"]["host"]
    rootCAPath = config["AWS_IOT_CLIENT"]["rootCAPath"]
    certificatePath = config["AWS_IOT_CLIENT"]["certificatePath"]
    privateKeyPath = config["AWS_IOT_CLIENT"]["privateKeyPath"]
    clientId = config["AWS_IOT_CLIENT"]["clientId"]
    topic = config["AWS_IOT_CLIENT"]["topic"]
    port = 8883
    mode = config["AWS_IOT_CLIENT"]["mode"]

    # Configure logging
    logger = logging.getLogger("AWSIoTPythonSDK.core")
    logger.setLevel(logging.DEBUG)
    streamHandler = logging.StreamHandler()
    formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
    streamHandler.setFormatter(formatter)
    logger.addHandler(streamHandler)

    # Init AWSIoTMQTTClient
    myAWSIoTMQTTClient = None
    myAWSIoTMQTTClient = AWSIoTMQTTClient(clientId)
    myAWSIoTMQTTClient.configureEndpoint(host, port)
    myAWSIoTMQTTClient.configureCredentials(rootCAPath, privateKeyPath, certificatePath)

    # AWSIoTMQTTClient connection configuration
    myAWSIoTMQTTClient.configureAutoReconnectBackoffTime(1, 32, 20)
    myAWSIoTMQTTClient.configureOfflinePublishQueueing(-1)  # Infinite offline Publish queueing
    myAWSIoTMQTTClient.configureDrainingFrequency(2)  # Draining: 2 Hz
    myAWSIoTMQTTClient.configureConnectDisconnectTimeout(10)  # 10 sec
    myAWSIoTMQTTClient.configureMQTTOperationTimeout(5)  # 5 sec

    # Connect to AWS IoT
    myAWSIoTMQTTClient.connect()
    time.sleep(2)

    # Publish to AWS IoT when PIR motion is detected
    loopCount = 0
    se = serial.Serial(config["SERIAL_COMM"]["serial_port"], 115200)

    while True:
        str = se.readline()
        print (str)
        if (str.find("MUSIC_FESTIVAL_DEMO: PIR sensor detected motion") != -1):
            message = {}
            message['message'] = time.time() * 1000
            message['sequence'] = loopCount
            messageJson = json.dumps(message)
            myAWSIoTMQTTClient.publish(topic, messageJson, 1)
            if mode == 'publish':
                print('Published topic %s: %s\n' % (topic, messageJson))
            loopCount += 1
        #time.sleep(1)

if __name__ == '__main__':
    aws_iot_pub()

