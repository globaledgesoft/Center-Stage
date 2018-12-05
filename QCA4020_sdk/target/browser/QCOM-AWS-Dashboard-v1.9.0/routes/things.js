
/**
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **/


var express = require("express");
var router = express.Router();

var AWSIoT = require('models/aws_iot.js');
var aws_iot = new AWSIoT();

router.get("/", function(req, res) {
	var list = aws_iot.getThingList(function(err, list){
		if(err) {
			console.log("Error : ", err);
			res.render("404", err)
		} else {
			res.render("things_list", {
				list: list,
				crumbs: [{name: "Home", class: "", href: "/things"}, {name: "Things", class: "active", href: "javascript:void(0);"}]
			});
		}
	});
});

module.exports = router;
