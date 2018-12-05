
/**
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **/


// Import ...
var AWSIoTData = require('aws-iot-device-sdk');

// Initialize Configuration  ...
var AWSConfiguration = require('config/global.js').aws_iot;
var KeyConfiguration = require('config/aws.json')

// Notification topic ...
var breachSubscribeTopic = AWSConfiguration.breach_topic_name;

// Initialize clientId ...
var clientId = "ThresholdNotifier"

var breach_notifier = function(socket){

    // Initialize configuration for IoT device object ....
    var notifier= AWSIoTData.device({
        clientId: clientId,
        protocol: 'wss',
        region: KeyConfiguration.region,
        accessKeyId: KeyConfiguration.accessKeyId, 
        secretKey: KeyConfiguration.secretAccessKey,
        host: AWSConfiguration.host
    });

    /**
     * AWS IoT connect(), message(), reconnect() event handlers ....
     **/
    notifier.on('connect', function() {
        console.info(' [Notifier]\t: Connected to notification listener.');
        notifier.subscribe(breachSubscribeTopic);   
    });

    notifier.on('close', function() {
        console.warn(' [Notifier]\t: Notification connection closed.');
    });
    
    notifier.on('reconnect', function() {
        console.log('[Notifier]\t: Reconnecting for breach notification.');
    });
    
    notifier.on('offline', function() {
        console.warn(' [Notifier]\t: Now offline.');
    });
    
    notifier.on('error', function(error) {
        console.error('[Notifier]\t: Please! Make sure you are connected to internet.');
    });
    

    notifier.on('message', function(topic, payload) {

        try {
            JSON.parse(payload);
            where_is_breach(JSON.parse(payload), function(breachMessage){
                socket.emit('notify',breachMessage);
            });
        } catch (e) {
            var breachMessage = [];
            breachMessage.push("Wrong breach notification JSON format");
            console.log("Notification is not in proper JSON format");
            socket.emit('notify',breachMessage);
        }        
    });

    function where_is_breach(breachData, callback) {
        var breachMessage= [];
        for(thing in breachData) {
            for(device in breachData[thing]) {
                breachMessage.push("Thing: " + thing + "\nDevice: " + device + "\nMessage: " + breachData[thing][device]["message"]);
            }           
        }
        callback(breachMessage)
    }
};

module.exports = breach_notifier;