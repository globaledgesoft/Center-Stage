
/**
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **/

var express = require("express");
var router = express.Router();

var AWSIoTData = require("models/aws_iot_data.js");
var awsIoTData = new AWSIoTData();

var DbHandler = require("models/mongodb_handler.js");
var mongoDbModel = new DbHandler();

router.get("/", function(req, res) {
  res.render("device_data", {
  	crumbs: [
  		{name: "Home", class: "", href: "/things"},
  		{name: "Things", class: "", href: "/things"},
  		{name: "Devices", class: "active", href: "javascript:void(0);"},
  	]
  });
});

router.get("/update", function(req, res) {
	res.render("thing_update", {
		crumbs: [
			{name: "Home", class: "", href: "/things"},
			{name: "Things", class: "", href: "/things"},
			{name: "Update", class: "active", href: "javascript:void(0);"}
		]
	});
})

router.get("/get", function(request, response){
  awsIoTData.getShadow(request.query.thingName, function(err, result) {
		if(err) {
			response.send(err);
		} else {
			response.send(result.data);
		}
  })
});

router.post("/update", function(request, response){
  awsIoTData.updateShadow(request.body.payload, request.body.thingName, function(err, result) {
		if(err) {
			response.send(err);
		} else {
			response.send(result);
		}
	});
});

router.get("/getDataFromDB", function(request, response) {
  mongoDbModel.getThingsData(request.query.thingName, request.query.deviceId, request.query.sensorTypeName, function(err, data) {
    if(err) {
      response.send(err);
    } else {
      response.send(data);
    }
  })
});

router.get("/livegraph", function(request, response){
	response.render("live_graph", {
		crumbs:[
			{name:"Home", class:"", href:"/things"},
			{name: "Things", class: "", href: "/things"},
			{name : "Live Graph", class: "active", href: "javascript:void(0);"}
		]
	});
});

module.exports = router;
