/**
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **/


var params = new URLSearchParams(location.search.slice(1));
var thingName = params.get("thingName");

// Plotly configuration ...

var selectorOptions = {
    buttons: [{
        step: 'minute',
        stepmode: 'backward',
        count: 1,
		label: 'Minute'
    }, {
        step: 'hour',
        stepmode: 'backward',
        count: 1,
        label: 'Hour'
    }, {
        step: 'Day',
        stepmode: 'backward',
        count: 1,
        label: 'Day'
    }, {
		step: 'all',
		label : 'All'
	}]
};

// Sensor Units ...
var sensorUnit = {
    "temperature" : " °C",
    "humidity" : " rH",
    "pressure" : " hPa",
	"ambient" : " lx",
	"accelerometer" : " m/s<sup>2</sup>",
	"gyroscope": " dps",
	"compass" : " µT"	
}

function loadSensorData() {
	getCurrentStatus();
};

function loadCurrentData(){
	$.ajax({
		type: "GET",
		url: "/devices/get",
		data: {
			thingName: thingName
		},
		success: function(result) {
			if(result.state && result.state.reported) {
				$("#timeId").text(new Date(result.timestamp * 1000));						
		
				var sensorTypes = [
					"temperature", "pressure", "gyroscope", "humidity", "ambient", "light", 
					"thermostat", "accelerometer", "compass", "dimmer"
				];
				var oldActiveDeviceData = $("#deviceTabId").children(".active")["0"];
				var oldActiveDeviceId;

				if(oldActiveDeviceData) {
					oldActiveDeviceId = oldActiveDeviceData.childNodes["0"].innerText;
				}
		
				$("#tabContentId").empty();

				var reportedObj = result.state.reported;
				for(deviceId in reportedObj) {
					var deviceObj = reportedObj[deviceId];
					var anchorElement = $("<a></a>").attr({
						"data-toggle" : "tab",
						"href" : "." + deviceId
					}).text(deviceId);
					var tabPaneDiv = $("<div></div").addClass("tab-pane  " + deviceId + " fade");

					for(var index = 0; index < sensorTypes.length; index++) {
						var returnDiv = createCardForSensor(deviceObj, sensorTypes[index]);
						if(returnDiv) {
							tabPaneDiv.append(returnDiv);
						}
					}
			
					$("#tabContentId").append(tabPaneDiv);
				}

				var oldActiveDeviceInNewData = $("#deviceTabId li:contains(" + oldActiveDeviceId + ")");
				if(oldActiveDeviceId && oldActiveDeviceInNewData.length > 0) {
					oldActiveDeviceInNewData.addClass("active");
					$("#tabContentId").children("." + oldActiveDeviceId).addClass("in active");
				} else {				
					$("#tabContentId div").first().addClass("in active");
				}
								
			} else {
				$("#timeId").text("OOPS!! No devices found.");
			}
		},
		error: function(error) {
			console.log("[ERROR] : ", error);
		}
	});
}

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
			if(result.state && result.state.reported) {
				$("#timeId").text(new Date(result.timestamp * 1000));
				
				/** 
				 * Creating DOM elements for the cards for different sensors defined/attached in 
				 * QCA boards/AP 
				 */
				createCards(result.state.reported);
				
				// Creating DOM elements for filters of devices in AWS things 
				createDeviceFilter(Object.keys(result.state.reported));
			} else {
				$("#timeId").text("OOPS!! No devices found.");
			}
		},
		error: function(error) {
			console.log("[ERROR] : ", error);
		}
	});
}

function createCards(reportedObj) {
	var sensorTypes = [
		"temperature", "pressure", "gyroscope", "humidity", "ambient", "light", 
		"thermostat", "accelerometer", "compass", "dimmer"
	];
	var oldActiveDeviceData = $("#deviceTabId").children(".active")["0"];
	var oldActiveDeviceId;

	if(oldActiveDeviceData) {
		oldActiveDeviceId = oldActiveDeviceData.childNodes["0"].innerText;
	}
	$("#deviceTabId").empty();
	$("#tabContentId").empty();

	for(deviceId in reportedObj) {
		var deviceObj = reportedObj[deviceId];
		var anchorElement = $("<a></a>").attr({
			"data-toggle" : "tab",
			"href" : "." + deviceId
		}).text(deviceId);
		var tabPaneDiv = $("<div></div").addClass("tab-pane " + deviceId + " fade");
	
		for(var index = 0; index < sensorTypes.length; index++) {
			var returnDiv = createCardForSensor(deviceObj, sensorTypes[index]);
			if(returnDiv) {
				tabPaneDiv.append(returnDiv);
			}
		}

		$("#deviceTabId").append($("<li class='mdl-color--primary'></li>").append(anchorElement));
		$("#tabContentId").append(tabPaneDiv);
	}

	var oldActiveDeviceInNewData = $("#deviceTabId li:contains(" + oldActiveDeviceId + ")");
	if(oldActiveDeviceId && oldActiveDeviceInNewData.length > 0) {
		oldActiveDeviceInNewData.addClass("active");
		$("#tabContentId").children("." + oldActiveDeviceId).addClass("in active");
	} else {
		$("#deviceTabId li").first().addClass("active mdl");
		$("#tabContentId div").first().addClass("in active");
	}
}

function createCardForSensor(deviceObj, sensorTypeName) {
	var sensorTypeObj = deviceObj[sensorTypeName];

	if(sensorTypeObj) {
		var headerElement = $("<h6></h6>").addClass("mdl-color--blue").text(""+capitalizeString(sensorTypeName))
		var bodyDiv = $("<div class='container-fluid'></div>");

		for(sensorName in sensorTypeObj) {
			var dataString;

			switch(sensorTypeName) {
				case "temperature":
					dataString = sensorTypeObj[sensorName] + " °C";
					break;

				case "humidity":
					dataString = sensorTypeObj[sensorName] + " rH";
					break;

				case "ambient":
					dataString = sensorTypeObj[sensorName] + " lx";
					break;

				case "light":
					dataString = sensorTypeObj[sensorName] ? "ON" : "OFF";
					break;

				case "pressure":
					dataString = (sensorTypeObj[sensorName]).toFixed(2) + " hPa";
					break;

				case "thermostat":
					dataString = sensorTypeObj[sensorName]["op_mode"] + " mode, " + sensorTypeObj[sensorName]["actual"] + "°C";
					break;

				case "gyroscope":
					dataString = (sensorTypeObj[sensorName]['X']).toFixed(2) + ' dps, ' + (sensorTypeObj[sensorName]['Y']).toFixed(2) + ' dps, ' + (sensorTypeObj[sensorName]['Z']).toFixed(2) + ' dps' ;
					break;
				case "accelerometer":
					dataString = (sensorTypeObj[sensorName]['X']).toFixed(2) + " m/s<sup>2</sup>, " + (sensorTypeObj[sensorName]['Y']).toFixed(2) + " m/s<sup>2</sup>, " + (sensorTypeObj[sensorName]['Z']).toFixed(2) + " m/s<sup>2</sup>";
					break;
				case "compass":
					dataString = (sensorTypeObj[sensorName]['X']).toFixed(2) + ' µT, ' + (sensorTypeObj[sensorName]['Y']).toFixed(2) + ' µT, ' + (sensorTypeObj[sensorName]['Z']).toFixed(2) + ' µT' ;
					break;
				case "dimmer":
					dataString = sensorTypeObj[sensorName];
					break;
			}

			var sensorIcon = $('<li class="col-sm-1"><span class="sensorIcon" style="height=40px;width=40px;"><img src="../images/'+sensorTypeName+'.png"></span></li>');		
			var pElement = $('<li class="col-middle col-sm-4"></li>').html("<h4>" + sensorName + "</h4> ");
			var sensorvalue = $('<li class="col-last col-sm-7"></li>').html("<h4>" + dataString + "</h4>");
			var list = $("<ul></ul>").addClass("col-in col-sm-12").append(sensorIcon, pElement, sensorvalue);

			bodyDiv.append(list);
		}

		var cardDivElement = $("<div></div>").addClass("sensorCard mdl-cell mdl-cell--12-col mdl-color--white mdl-shadow--4dp").append(headerElement, bodyDiv);
		return cardDivElement;
	}

	return null;
}

function createIcon(iconName) {
	return $("<i></i>").addClass("material-icons").text(iconName);
}

function capitalizeString(string) {
	return string.charAt(0).toUpperCase() + string.slice(1);
}

function createDeviceFilter(deviceIdArray) {
	$("#deviceList").empty();

	for (var i = 0; i < deviceIdArray.length; i++) {
		var optionElement = $("<li></li>").addClass("mdl-menu__item").attr("data-val", deviceIdArray[i]).text(deviceIdArray[i]);
		$("#deviceList").append(optionElement);
	}
	
	$("#deviceList li").first().attr("data-selected", "true");
	getmdlSelect.init(".deviceSelect")
}

function loadGraph() {
	window.localStorage.clear();
	
	var sensorTypeName = $("#sensor").val().toLowerCase();

	if(sensorTypeName === "all"){
		sensorTypeName = "base";
	}
	var deviceId = $("#DeviceId").val();


	$.ajax({
		type: "GET",
		url: "/devices/getDataFromDB", //@TODO Opt for Restful method/route names.
		data: {
			thingName: thingName,
			deviceId: deviceId,
			sensorTypeName: sensorTypeName
		},
		beforeSend: function(){
			$("#loader").css("display", "block");
		},
		complete: function() {
			$("#loader").css("display", "none");
		},
		success: function(result) {      
			$("#chartsDiv").empty();
			$("#chartsDiv").addClass("mdl-color--grey-200")
			$("#chartsDiv").append($("<h5 class='mdl-cell mdl-cell--12-col'></h5>").css("text-align", "center").text(deviceId + " Info"));

			for(sensorType in result) {
				if(Object.keys(result[sensorType]).length > 0) {  					
					var chartBox = $("<div></div>").addClass("mdl-cell mdl-cell--12-col mdl-cards mdl-shadow--2dp graphspace").attr("id", sensorType + "ChartDiv");
					$("#chartsDiv").append(chartBox);
					loadSensorGraph(result[sensorType], sensorType + "ChartDiv", capitalizeString(sensorType));					
				} else{
					if(sensorType == sensorTypeName){
						$("#chartsDiv").append($("<h6></h6>").css("text-align", "center").text("OOP's !! No "+sensorType+" sensor found"));
					}				
				}
			}
		},
		error: function(error) {
			console.log(error);
		}
	});	
}

function loadSensorGraph(sensorTypeObj, sensorChartDiv, sensorType) {	
	var sensorDataForChart = getChartFormatData(sensorTypeObj, sensorType);

	Plotly.plot(sensorChartDiv,  sensorDataForChart, {
		title: sensorType + ' info',
		xaxis: {
			title: 'Time',
			rangeselector: selectorOptions,		
			rangeslider: {}
		},
		yaxis: {
			title : sensorType
		},
		hovermode : 'closest'
	}, {displayModeBar : false});	
}

function getChartFormatData(sensorTypeObj, sensorType) {
	var sensorDataForChart = [];

	switch(sensorType){
		case "Accelerometer":
		case "Gyroscope":
		case "Compass": 
			for(sensorId in sensorTypeObj) {
				var sensorObjx = { name: 'xAxis', x:[], y:[], text:[], line: { shape : 'spline'}};
				var sensorObjy = { name: 'yAxis', x:[], y:[], text:[], line: { shape : 'spline'}};
				var sensorObjz = { name: 'zAxis', x:[], y:[], text:[], line: { shape : 'spline'}};

				sensorTypeObj[sensorId].forEach(function(data, i){
					sensorObjx.x.push(new Date(data[0]));
					sensorObjy.x.push(new Date(data[0]));
					sensorObjz.x.push(new Date(data[0]));

					sensorObjx.y.push(data[1]["X"]);
					sensorObjy.y.push(data[1]["Y"]);
					sensorObjz.y.push(data[1]["Z"]);

					sensorObjx.text.push(data[1]["X"] + sensorUnit[sensorType.toLowerCase()]);
					sensorObjy.text.push(data[1]["Y"] + sensorUnit[sensorType.toLowerCase()]);
					sensorObjz.text.push(data[1]["Z"] + sensorUnit[sensorType.toLowerCase()]);
				})
		
				sensorDataForChart.push(sensorObjx, sensorObjy, sensorObjz);
			}
			
			break;
		default :
		  	console.log("SensorType", sensorType)
		  	for(sensorId in sensorTypeObj) {
				var sensorObj = { name: sensorId, x:[], y:[], text:[], line : { shape : 'spline'}};
	
				sensorTypeObj[sensorId].forEach(function(data, i){					
					sensorObj.x.push(new Date(data[0]));
					sensorObj.y.push(data[1]);
					sensorObj.text.push(data[1] + sensorUnit[sensorType.toLowerCase()]);
				})
						
				sensorDataForChart.push(sensorObj);
			}			
	}
	
	return sensorDataForChart;
}

function loadList() {
	window.location.href = "things";
}

function renderUpdatePage() {
	window.location.href = "devices/update?thingName=" + thingName;
}

function pollingCurrentData() {
	if($("#dataTab").hasClass('is-active')) {
		loadCurrentData();
	}
}

loadSensorData();
setInterval(pollingCurrentData, 2 * 1000);