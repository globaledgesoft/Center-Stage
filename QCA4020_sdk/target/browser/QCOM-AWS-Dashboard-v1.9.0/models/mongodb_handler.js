
/**
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **/


/**
 * Description: This class is used to handle CRUD operations on MongoDB
 */
var mongoConfObj = require("config/mongo_connection.js");
var db_connection = mongoConfObj.db_connection;
var jsonDiff = require("lodash.isequal");

var mongodbhandler = function() {

};

module.exports = mongodbhandler;



mongodbhandler.prototype.createIndex = function(collection_name, callback){
	db_connection.collection(collection_name).createIndex({"timestamp": -1}, function(err){
		if(err){
			callback(err)
		} else {
			callback(undefined, "Indexing Done")
		}
	})
}

/**
 *	@ Description: This function will check whether the given object and last stored value has same reported
 *	data or not.

 *	@ Param {JSON Object} data_obj: Json Object which contains the latest value of the thing data from AWS
 *	@ Param {String} collection_name: String containing collection name [thingName] in which data need to compare.
 *	@ Param {function} callback: A callback function to execute after execution of this function
 */
mongodbhandler.prototype.isDataChanged = function(data_obj, collection_name, callback) {
	db_connection.collection(collection_name).find({}).sort({"timestamp": -1}).limit(1).toArray(function(err, documents) {
		if(err) {
			callback(err, undefined, undefined);
		} else {
			var equal = false;
			if(documents[0]) {			
				equal = jsonDiff(data_obj.state.reported, documents[0].state.reported);
			}

			if(equal) {
				callback("Data is same hence not inserted", undefined, undefined);
			} else {
				callback(undefined, data_obj, collection_name);
			}
		}
	})
}

/**
 *	@ Description: This function will insert the passed object to mongodb

 *	@ Param {JSON Object} dataObj: Data in json format which is to insert into Database
 *	@ Param {String} collection_name: String containing collection name [thingName] in which data need to compare.
 */
mongodbhandler.prototype.insert = function(dataObj, collection_name) {
	db_connection.collection(collection_name).insertOne(dataObj, function(err, result) {
		if(err) {
			console.error("[MongoDB]\t: Error while inserting: ", err);
		} else {
			console.log("[MongoDB]\t: Inserted Successfuly");
		}
	});
};

/**
 *	@ Description : This prototype function is used to retrieve the documents from mongodb for a specific deviceId
 *	and sensortype inside the device over a period of time.

 *	@ Param {String} collectionName : Collection name in the mongodb to retrieve documents.
 *	@ Param {String} deviceId : String containing deviceid which user requested
 *	@ Param {String} sensorTypeName : String containing sensor type which user has requested.
 *	@ Param {function} callback : A callback function to execute after fetching the requested documents from mongodb
 */
mongodbhandler.prototype.getThingsData = function(collectionName, deviceId, sensorTypeName, callback) {
	var filterKey;

	// If the user requested for a sensortype add it to filter key else filter for device only
	if(sensorTypeName !== 'base') {
		filterKey = "state.reported." + deviceId + "." + sensorTypeName;
	} else {
		filterKey = "state.reported." + deviceId;
	}

	// sortObj is to sort the documents
	var sortObj = {
		"timestamp" : -1
	}

	// filterObj is to filter the documents and projectionObj is to project only some fields in the documents.
	var filterObj = {};
	var projectionObj = {};

	filterObj[filterKey] = {
		$exists: true
	};

	projectionObj[filterKey] = 1;
	projectionObj["timestamp"] = 1;
	projectionObj["_id"] = 0;

	db_connection.collection(collectionName).find(filterObj, projectionObj).sort(sortObj).limit(7200).toArray(function(err, documentsArray) {
		if(err) {
			console.error("[MongoDB]\t: Error while fetching data", err);
			callback(err, undefined);
		} else {
			console.log("[MongoDB]\t: Successfuly Fetched");
			callback(undefined, parseDocuments(documentsArray.reverse(), deviceId, sensorTypeName));
		}
	});
}

/**
 *	@ Description : This function will parse the Mongo documents for a specific device and sensortype
 *	Ex. User request data for all sensors in deviceid_1 Or Temperature sensor in deviceid_1

 *	@ Param {Array} documentsArray : An Array containing the documents retrieved from mongodb
 *	@ Param {String} deviceId : String containing deviceid which user has requested.
 *	@ Param {String} sensorTypeName : String containing sensor type which user has requested.

 *	@ Return {JSON Object} thingData : Returning JSON Object after adding sensortype data to it from all documents.
 */
function parseDocuments(documentsArray, deviceId, sensorTypeName) {
	var thingData = {};
	var sensorTypeNameArray = [];

	if(sensorTypeName !== "base") {	// If user requested for a specific sensortype
		thingData[sensorTypeName] = {};
		sensorTypeNameArray.push(sensorTypeName);
	} else { // If user requested for all sensors in the device
		thingData = {
			"temperature": {},
			"humidity": {},
			"pressure": {},
			"ambient": {},
			"accelerometer" : {},
			"compass" : {},
			"gyroscope" : {}
		};
		sensorTypeNameArray = sensorTypeNameArray.concat(["temperature", "pressure", "humidity", "ambient","accelerometer", "compass", "gyroscope"]);
	}

	for(var i = 0; i < documentsArray.length; i++) {	// Loop over multiple documents
		var deviceObj = documentsArray[i].state.reported[deviceId];

		for(var j = 0; j < sensorTypeNameArray.length; j++) {
			thingData = getSensorTypeData(
				deviceObj[sensorTypeNameArray[j]],
				sensorTypeNameArray[j],
				documentsArray[i].timestamp,
				thingData
			);
		}
	}

	return thingData;
}

/**
 *	@Description : This method will parse the JSON object to get a specific sensortype data.
 *	Ex. To parse the query result and get the Temperature sensortype data in a JSON Object

 *	@ Param {JSON Object} sensorTypeObj : A JSON Object which contains a single sensortype data.
 *	@ Param {String} sensorTypeName : The name of the sensortype. Ex. "Temperature", "Pressure"
 *	@ Param {Number} timestamp : The time of the data recieved from AWS in epoch time (in seconds)
 *	@ Param {JSON Object} thingData : A JSON object to add the sensortype data for every call for this method

 *	@ Return {JSON Object} thingData : Returning JSON Object after adding the sensortype data to it from a single document.
 */
function getSensorTypeData(sensorTypeObj, sensorTypeName, timestamp, thingData) {
	for(sensorId in sensorTypeObj) {	// Looping over multiple sensors inside a sensortype object
		var dataForOneTimestamp = [];
		var dataForMultipleTimestamp = [];

		if(thingData[sensorTypeName].hasOwnProperty(sensorId)) {
			// If the sensor is already present, then get the previous data to append the new data to it
			dataForMultipleTimestamp = thingData[sensorTypeName][sensorId];
		}

		dataForOneTimestamp.push(timestamp * 1000);
		dataForOneTimestamp.push(sensorTypeObj[sensorId]);

		dataForMultipleTimestamp.push(dataForOneTimestamp);

		thingData[sensorTypeName][sensorId] = dataForMultipleTimestamp;
	}

	return thingData;
}