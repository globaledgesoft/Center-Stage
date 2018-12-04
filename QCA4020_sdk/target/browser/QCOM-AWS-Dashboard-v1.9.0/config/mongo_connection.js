
/**
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **/


var mongo_config = require("config/global.js").mongodb;
var mongo_connection_url = "mongodb://" + mongo_config.hostname + ":" + mongo_config.port + "/" + mongo_config.db;

var mongoose = require("mongoose");
var db_connection = mongoose.createConnection(mongo_connection_url);

db_connection.on('error', function(error){
	console.error("[Launch]\t: Please ! Verify mongod service is running.\n\tMongoDB connection error:", error.message)
	process.exit()
})
module.exports = {
	db_connection : db_connection
};
