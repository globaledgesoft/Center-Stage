
/**
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **/


// Imports ...
var AWS = require('aws-sdk');

// Initialize configuration ...
var AWSConfiguration = require('config/global.js').aws_iot;
AWS.config.loadFromPath('config/aws.json');

var iotData = new AWS.IotData({endpoint: AWSConfiguration.host,region: AWS.config.region });

//Creating module ...
var awsIoTData = function(){};


awsIoTData.prototype.getShadow = function(thingName, callback) {
	var params = {
		thingName: thingName
	};

	iotData.getThingShadow(params, function(err, data) {
		if(err) {	
			console.error("[AWS IoT]\t: Failed to get Shadow status.", err.message);
			callback(err, undefined);	// an error occurred
		} else {
			var result = {
				data : JSON.parse(data.payload),
				thingName : thingName
			}
			callback(undefined, result); 	// success response
		}
	});
}

awsIoTData.prototype.updateShadow = function(payload, thingName, callback){
	var params = {
		payload: payload,
		thingName: thingName
	}

	iotData.updateThingShadow(params, function(err, data) {
		if(err) {
			console.error("[AWS IoT]\t: Failed to update shadow.", err.message);
			callback(err, undefined);
		} else {		
			callback(undefined, "Update Success")
		}
	});
}

module.exports = awsIoTData;