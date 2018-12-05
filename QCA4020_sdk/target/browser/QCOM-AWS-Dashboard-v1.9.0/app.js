
/**
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **/

 
// Setting Project's root directory as root path for all files in this project
require('rootpath')();

// Importing Console Looging module 
require('console.logentries')();

// Imports 
var express = require("express");
var handler = require("express-handlebars");
var path = require("path");
var bodyParser = require("body-parser");
var fs = require("fs");

// Checking for configuration and credentials .....
if(!fs.existsSync("config/global.js") || !fs.existsSync("config/aws.json")){	
	console.error(" [Launch]\t: Please provide configuration and credential file",
				" \n\t Use sample from config/ to set your configuration and credentials.")
	process.exit()
}

var app_config = require("config/global.js").app;

// Custom imports
var DbHandler = require("models/mongodb_handler.js");
var AwsIotData = require("models/aws_iot_data.js");
var AwsIoT = require("models/aws_iot.js");

var awsIot = new AwsIoT();
var awsIotData = new AwsIotData();
var mongoDbModel = new DbHandler();

// Creating Express object, and making it accessible for all
var app = express();

// Setting up Views via HandleBar templetes
app.set("views", 'views');
app.engine("handlebars", handler({
	defaultLayout:'layout',
	partialsDir: [
		"views/partials/"
	]
}));
app.set('view engine', "handlebars");

// Setting up bodyParser to use parameters in request message
app.use(bodyParser.urlencoded({ extended: false }))

// Setting up folder paths to be loaded on it's own
app.use(express.static('public'));

// Create routes objects
app.use(require("routes"));

// This function is to create index on each collection ....
var createIndex = function () {
	awsIot.getThingList(function(err, list) {
		if(err) {
			console.error("[AWS IoT]\t: Error while getting list.", err.message);
		} else {
			for(var i in list) {
				awsIotData.getShadow(list[i].name, function(err, result) {
					if(err) {
						if(err.code === 'ResourceNotFoundException') {
							console.error("[AWS IoT]\t: Failed to get Shadow - Shadow is not yet created");
						} else {
							console.error("[AWS IoT]\t: Failed to get Shadow", err.code);
						}
					} else {
						mongoDbModel.createIndex(result.thingName, function(error, isSuccess) {
							if(error) {
								console.log('[MongoDB]\t:',error);
							}
						});
					}
				});
			}
		}
	});
}

createIndex()


// Function to get the data from AWS for all things and store into DB
var insertGetData = function () {
	awsIot.getThingList(function(err, list) {
		if(err) {
			console.error("[AWS IoT]\t: Error while getting list.", err.message);
		} else {
			for(var i in list) {
				awsIotData.getShadow(list[i].name, function(err, result) {
					if(err) {
						if(err.code === 'ResourceNotFoundException') {
							console.error("[AWS IoT]\t: Failed to get Shadow - Shadow is not yet created");
						} else {
							console.error("[AWS IoT]\t: Failed to get Shadow", err.code);
						}
					} else {
						
						if(result.data.metadata){
							delete result.data.metadata;
						}

						if(result.data.state.desired) {
							delete result.data.state.desired;
						}

						if(result.data.state.delta){
							delete result.data.state.delta;
						}
											
						// This below snippet will insert into DB if there is change in reported data from AWS and last stored reported data
						mongoDbModel.isDataChanged(result.data, result.thingName, function(error, data, thing_name) {
							if(error) {
								console.log('[MongoDB]\t:',error);
							} else {								
								mongoDbModel.insert(data, thing_name);
							}
						});

						/** Note : If the data needs to insert each time while polling, uncomment below line and comment
							* the above snippet.
							*/
						// mongoDbModel.insert(result.data, result.thingName);
					}
				});
			}
		}
	});
}

insertGetData();
/**
* NOTE : This line will start getting data from AWS and store into MongoDB at specified time interval
* Un-comment if you want to continuously poll the data of the Things + Devices + (onboard) Sensors and save it
* local MongoDB database. This is specifically for generating a good visualization of the sensors data on
* graphs
*/
setInterval(insertGetData, app_config.polling_interval * 1000);

// Starts listening on the port
var server = app.listen(app_config.port, function () {
	var host = server.address().address;
	var port = server.address().port;
	console.info(" [Launch]\t: AWS Dashboard application listening at http://%s:%s", host, port);
});

var socket = require('socket.io').listen(server);

socket.on('connection', function(){
	console.info(' [Socket]\t: Socket connected.')
});

var Notifier = require('models/breach_notifier');
var NotifierObject = new Notifier(socket);

module.exports = app;
