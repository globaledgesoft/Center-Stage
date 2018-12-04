
/**
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **/


var params = new URLSearchParams(location.search.slice(1));
var thingName = params.get("thingName");

var intervalFunction = null;
var sensorInDevice = {};
var dimmerIdDevice = {};
var timestamp;
var reportedJson = {};
var lightIdDevice = {};

$("#DeviceId").change(function() {

	var deviceId = $(this).val();

	if(intervalFunction){
		clearInterval(intervalFunction)			
		intervalFunction = null;
		$("#reportedDataBody, #timeId").empty();
		changeUpdateStatus($(this).val());
	}


	if(deviceId === 'base') {
		$("#reportedData, #updateButton").css("display", "none");
		$("#reportedDataBody, #timeId").empty();
		$("#selectSensorId").html("<li class='mdl-menu__item' data-val='base' data-selected='true'>Please Select deviceId</li>");
	} else {
		var values = sensorInDevice[deviceId];
		var dimmerValues = dimmerIdDevice[deviceId];
		var lightValues = lightIdDevice[deviceId];

		$("#selectSensorId").empty();
		$("#dimmerSelectId").empty();
		$("#lightId").empty();

		$.each(values, function(index, value) {
			var optionElement = $("<li></li>").addClass("mdl-menu__item").attr("data-val", value).text(value);
			$("#selectSensorId").append(optionElement);
		});

		$.each(dimmerValues, function(index, value) {
			var optionElement = $("<li></li>").addClass("mdl-menu__item").attr("data-val", value).text(value);
			$("#dimmerSelectId").append(optionElement);
		});

		var lightDiv = setLightStatus(reportedJson[deviceId], "light");
		$('#lightId').append(lightDiv);
		
		$("#selectSensorId li").first().attr("data-selected", "true");
		$("#dimmerSelectId li").first().attr("data-selected", "true");

		if(reportedJson[deviceId]["thermostat"]) {
			$("#thermostat-update").css("display", "block");	
		}else {		
			$("#thermostat-update").css("display", "none");		
		}

		if(reportedJson[deviceId]["dimmer"]) {		
			$("#dimmer-update").css("display", "block");
		} else {
			$("#dimmer-update").css("display", "none");
		}


		if(reportedJson[deviceId]["light"]) {		
			$("#light-update").css("display", "block");
		} else {
			$("#light-update").css("display", "none");
		}

		getmdlSelect.init(".sensorSelect");
		getmdlSelect.init(".dimmerSelect")

		createCard(deviceId);
		$("#updateButton").css("display", "block");
	}
});

function getCurrentStatus() {
	$.ajax({
		type: "GET",
		url: "/devices/get",
		data: {
			thingName: thingName
		},
		beforeSend: function(){
			$("#loader").css("display", "block");
		},
		complete: function() {
			$("#loader").css("display", "none");
		},
		success: function(result) {
			if(result.state) {
				reportedJson = result.state.reported;
			}
			timestamp = result.timestamp;

			$("#thingNameId").val(thingName);
			$("#thingNameId").attr("readOnly", "true");
			createDeviceList();

			showChangedValue(0, 'tempValueId')
			showChangedValue(0, 'tempThresholdId')
			showChangedValue(0, 'dimvalue')
		},
		error: function(error) {
			console.log(error);
		}
	});
};

function changeUpdateStatus(deviceId) {
	$.ajax({
		type: "GET",
		url: "/devices/get",
		data: {
			thingName: thingName
		},	
		success: function(result) {
			if(result.state) {
				reportedJson = result.state.reported;
			}
			timestamp = result.timestamp;

			$("#thingNameId").val(thingName);
			$("#thingNameId").attr("readOnly", "true");
		
			if(deviceId === $('#DeviceId').val()) {				
				createCard(deviceId);
			}
			
		},
		error: function(error) {
			console.log(error);
		}
	});
};

function createCard(deviceId) {

	$("#reportedDataBody").empty();

	$("#timeId").text(new Date(timestamp * 1000));
	$("#deviceName").text("Device ID : " + deviceId);

	var dataCard = createCardForDevice(reportedJson[deviceId], "thermostat");
	var dimmerCard = createDimmerCard(reportedJson[deviceId], "dimmer");

	$("#reportedDataBody").append(dataCard, dimmerCard);
	$("#reportedData").css("display", "block");
}

function setLightStatus(deviceObj, sensorTypeName) {
	var sensorTypeObject = deviceObj[sensorTypeName];
	var cardDivElement;
	var allcards = $("<div></div>").addClass("mdl-grid");
	
	for(sensorName in sensorTypeObject) {
		var dataString = sensorTypeObject[sensorName] ? "true" : "false";;
		
		var pElement =$("<div></div>").addClass(" mdl-cell mdl-cell--6-col ").append("<h4>" + sensorName + "</h4> ");

		if(dataString == "true"){
			var toggleClass = "btn btn-lg btn-toggle focus active";
		} else {
			var toggleClass = "btn btn-lg btn-toggle";
		}

		var toggleButton = $("<div></div>").addClass(" mdl-cell mdl-cell--6-col ").append(`<button type="button" id="switchStatus" class="`+toggleClass+`" data-toggle="button" data-sensorId="`+sensorName+`"  aria-pressed="`+dataString+`" autocomplete="off">
        												<div class="handle"></div>
													</button>`);
															
		var sensorvalue = toggleButton;	
	
		allcards.append(pElement, sensorvalue);
	} 

	return allcards;
}

function createCardForDevice(deviceObj, sensorTypeName) {

	var sensorTypeObject = deviceObj[sensorTypeName];
	var cardDivElement;
	var allcards = $("<div></div>"); //.addClass(" mdl-cell mdl-cell--12-col ");
	
	for(sensorName in sensorTypeObject) {
		var headerElement = $("<h6></h6>").addClass("mdl-color--blue").html("" + capitalizeString(sensorTypeName));
		var bodyDiv = $("<div></div>");

		var mode = "<h4> Mode : " + sensorTypeObject[sensorName]["op_mode"] +" "+ "</h4>";
		var temp = "<h4> Temperature : " + sensorTypeObject[sensorName]["actual"] + "°C " + "</h4>";
		var dataString = mode + temp;			
		
		console.log("CreateCardForDevice | Sensor type : ", sensorTypeName);

		var sensorIcon = $('<li class="' + sensorTypeName + '_icon"><span class="sensorIcon" style="height=40px;width=40px;"><img src="../images/'+sensorTypeName+'.png"></span></li>');
		var pElement = $('<li class="col-middle"></li>').html("<h4>" + sensorName + "</h4> ");
		var sensorvalue = $('<div class="thermodata"></div>').html( dataString );
		var list = $("<ul></ul>").addClass("col-in").append(sensorIcon, pElement);

		bodyDiv.append(list, sensorvalue);

		cardDivElement  = $("<div></div>").addClass("thermoCard mdl-cell mdl-cell--12-col mdl-color--white mdl-shadow--4dp").append(headerElement, bodyDiv);	
		allcards.append(cardDivElement);
	} 

	return allcards;
}

function createDimmerCard(deviceObj, sensorTypeName) {
	var sensorTypeObject = deviceObj[sensorTypeName];
	var cardDivElement;
	var allcards = $("<div></div>").addClass(" mdl-cell mdl-cell--12-col ");
	
	for(sensorName in sensorTypeObject) {
		var headerElement = $("<h6></h6>").addClass("mdl-color--blue").html("" + capitalizeString(sensorTypeName));
		var bodyDiv = $("<div></div>");

		var dataString = "<h4> Dim value : " + sensorTypeObject[sensorName] + " " + "</h4>";	
		
		console.log("createDimmerCard | Sensor type : ", sensorTypeName);

		var sensorIcon = $('<li class="' + sensorTypeName + '_icon"><span class="sensorIcon" style="height=40px;width=40px;"><img src="../images/'+sensorTypeName+'.png"></span></li>');
		var pElement = $('<li class="col-middle"></li>').html("<h4>" + sensorName + "</h4> ");
		var sensorvalue = $('<div class="thermodata"></div>').html( dataString );
		var list = $("<ul></ul>").addClass("col-in").append(sensorIcon, pElement);

		bodyDiv.append(list, sensorvalue);
		cardDivElement  = $("<div></div>").addClass("thermoCard mdl-cell mdl-cell--12-col mdl-color--white mdl-shadow--4dp").append(headerElement, bodyDiv);	
		allcards.append(cardDivElement);
	} 

	return allcards;
}

function createDeviceList() {
	var deviceIdArray = Object.keys(reportedJson);

	for(var i = 0; i < deviceIdArray.length; i++) {
		if(reportedJson[deviceIdArray[i]]["thermostat"]) {			
			sensorInDevice[deviceIdArray[i]] = Object.keys(reportedJson[deviceIdArray[i]]["thermostat"]);			
		} else {
			$("#thermostat-update").css("display", "none");		
		}

		if(reportedJson[deviceIdArray[i]]["dimmer"]) {
			dimmerIdDevice[deviceIdArray[i]] = Object.keys(reportedJson[deviceIdArray[i]]["dimmer"]);
		} else {
			$("#dimmer-update").css("display", "none");
		}

		 if(! reportedJson[deviceIdArray[i]]["light"]) {
			$("#light-update").css("display", "none");		 }

		var optionElement = $("<li class='mdl-menu__item'></li>").attr("data-value", deviceIdArray[i]).text(deviceIdArray[i]);
		$("#deviceInputId").append(optionElement);
	}	
	
	$("#deviceInputId li").first().attr("data-selected", "true");
	getmdlSelect.init(".deviceSelect")
}

function capitalizeString(string) {
	return string.charAt(0).toUpperCase() + string.slice(1);
}

function loadList() {
	window.location.href = "/things";
}

function showChangedValue(value, id) {
	document.getElementById(id).innerHTML = value;
}

function payloadData() {
	var thingName = $("#thingNameId").val();
	var deviceId = $("#DeviceId").val();
	var sensorId = $("#sensorId").val();
	var operatingModes = $("#selectOperatingModeId").val();
	var temperature = parseInt($("#temperatureId").val());
	var threshold = parseInt($("#thresholdId").val());
	var dimmer = parseInt($("#dimmerId").val());
	var dimmerId = $('#dimmerSelectId').val();

	var lightId = $('#switchStatus').attr('data-sensorId');
	var switchStatus = $('#switchStatus').attr('aria-pressed');
	
	if(switchStatus === 'true') {
		var lightStatus = 1;
	} else {
		var lightStatus = 0;
	}
	
	var payloadData = {
		state: {
			desired: {}
		}
	}

	payloadData["state"]["desired"][deviceId] = {};

	if(sensorId && temperature != 15 && threshold) {
		var thermostatData = {
			"op_mode": operatingModes,		
			"desired": temperature,
			"threshold" : threshold
		};

	
		payloadData["state"]["desired"][deviceId]["thermostat"] = {};
		payloadData["state"]["desired"][deviceId]["thermostat"][sensorId] = thermostatData;
	}
	
	if(dimmer) {
		if(dimmerId) {
			payloadData["state"]["desired"][deviceId]["dimmer"] = {};
			payloadData["state"]["desired"][deviceId]["dimmer"][dimmerId] = dimmer;
		}		
	}

	if(lightId) {
		payloadData["state"]["desired"][deviceId]["light"] = {};
		payloadData["state"]["desired"][deviceId]["light"][lightId] = lightStatus;
	}
	
	
	console.log(payloadData);

	var responseData = {
		payload: JSON.stringify(payloadData),
		thingName: thingName
	}	

	$.ajax({
		type: "POST",
		url: "/devices/update",
		data: responseData,
		beforeSend: function(){
			$("#loader").css("display", "block");
		},
		complete: function() {
			$("#loader").css("display", "none");
		},
		success: function(result) {

			var snackbarContainer = document.querySelector('#update-toast');
			var showToastButton = document.querySelector('#updateButton');

			'use strict';
			var data = {message: 'Shadow Update Successful.'};
			snackbarContainer.MaterialSnackbar.showSnackbar(data);	
			
			intervalFunction = setInterval(function(){						
				if(deviceId === $('#DeviceId').val()) {
					if(intervalFunction != null){
						changeUpdateStatus(deviceId);	
						console.log("interval ", intervalFunction);
					}				
				} 				
			}, 2000);
		},
		error: function(error) {
			console.log(error);

			var snackbarContainer = document.querySelector('#update-toast');
			var showToastButton = document.querySelector('#updateButton');

			'use strict';
			var data = {message: 'Shadow Update Failed.'};
			snackbarContainer.MaterialSnackbar.showSnackbar(data);			
		}
	});
}

getCurrentStatus();
