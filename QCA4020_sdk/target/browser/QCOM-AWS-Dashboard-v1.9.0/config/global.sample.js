
/**
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **/


var config = {
	mongodb : {
		hostname : "HOST NAME", //localhost
		port : "PORT NUMBER", //27017
		db : "DB NAME" //ruby-quartz
	},
	aws_iot : {
		host: "AWS HOST URL",
		thing_type_name: "AWS THING TYPE NAME FOR FILTER",
		breach_topic_name: "TOPIC NAME TO LISTEN FOR THRESHOLD BREACH"
	},
	app : {
		port : "PORT NUMBER FOR SERVER TO START LISTENING",
		polling_interval : "TIME INTERVAL FOR POLLING IN SECONDS"
	}
};

module.exports = config;
