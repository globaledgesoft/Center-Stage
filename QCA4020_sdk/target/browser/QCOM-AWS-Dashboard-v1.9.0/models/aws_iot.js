
/**
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **/


// Import ...
var AWS = require('aws-sdk');

// Initialize Configuration ...
var aws_iot_config = require('config/global.js').aws_iot;
AWS.config.loadFromPath('config/aws.json');

var iot =new AWS.Iot(); // IoT Object ...

var aws_iot = function(){}; // Module ...

aws_iot.prototype.getThingList = function(callback) {
	var params = {
		thingTypeName: aws_iot_config.thing_type_name,
		maxResults: 250
	}; // This is filter for getting thing list (Thing type name) ...

	// Function that will take thing list from the AWS IoT ...
	iot.listThings(params, function(err, data) {
		var list = [];
		if(err) {	// an error occurred
			console.error("[AWS IoT]\t: Host is unreachable. Please check your internet connection.\n", err.message);
			callback(err, undefined);
		} else {
			for(var i in data.things) {
				list.push({'name': data.things[i].thingName})
			}
			callback(undefined, list);		// successful response
		}
		return list;
	});
};

module.exports = aws_iot;
