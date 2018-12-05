
/**
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **/

 
var params = new URLSearchParams(location.search.slice(1));
var thingName = params.get("thingName");

var liveChart, intervalFunction, previousTimestamp;
var ChartpollingTime = 1000		// This is timing for live graph 'get' method interval

// Plotly.js configuration ....

var layout = {
	title: '',
	xaxis: {
		title: 'Time'
	},
	yaxis: {
		title : '',
		autorange: true
	},
	hovermode : 'closest'
};

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
				createDeviceFilter(Object.keys(result.state.reported));
			} else {
				$("#timeId").text("OOPS!! No devices found.");
			}
		},
		error: function(error) {
			console.log(error);
		}
	});
}


function createDeviceFilter(deviceIdArray) {
	$("#deviceList").empty();

	for(var i = 0; i < deviceIdArray.length; i++) {
		var optionElement = $("<li></li>").addClass("mdl-menu__item")
													.attr("data-val", deviceIdArray[i]).text(deviceIdArray[i]);
		$("#deviceList").append(optionElement);
	}
	
	$("#deviceList li").first().attr("data-selected", "true");
	getmdlSelect.init(".deviceSelect")
}

function requestData(sensorTypeName, deviceId){
	
	$.ajax({
		type: "GET",
		url: "/devices/get",
		data: {
			thingName: thingName		
		},		
		success: function(result) {
			var reportedObj = result.state.reported;
			var sensorTypeArray = ["temperature", "ambient", "pressure", "humidity", "accelerometer", "compass", "gyroscope"]
			var sensorDataForChart = [];

			if(result.state && reportedObj) {
				var deviceData = reportedObj[deviceId]
				if(deviceData) {						
					switch(sensorTypeName){
						case "accelerometer":
						case "gyroscope":
						case "compass":
							for(sensorData in deviceData[sensorTypeName]){										

								var time = new Date(result.metadata.reported[deviceId][sensorTypeName][sensorData]["X"].timestamp * 1000 );
							
								var olderTime = time.setMinutes(time.getMinutes() - 1);
								var futureTime = time.setMinutes(time.getMinutes() + 1);

								var update = {
									x: [[time], [time], [time]],
									y: [[deviceData[sensorTypeName][sensorData]["X"]], [deviceData[sensorTypeName][sensorData]["Y"]], [deviceData[sensorTypeName][sensorData]["Z"]]],
									text: [[deviceData[sensorTypeName][sensorData]["X"] + sensorUnit[sensorTypeName]], [deviceData[sensorTypeName][sensorData]["Y"] + sensorUnit[sensorTypeName]], [deviceData[sensorTypeName][sensorData]["Z"] + sensorUnit[sensorTypeName]]],								}
								var minuteView = {
									xaxis : {
										title : 'Time',
										type : 'date',
										range : [olderTime, futureTime]
									}
								};

								Plotly.relayout('liveChart', minuteView);
								Plotly.extendTraces('liveChart', update, [0,1,2]);								
							}  
							break;
						default:
							for(sensorData in deviceData[sensorTypeName]){						
								var time = new Date(result.metadata.reported[deviceId][sensorTypeName][sensorData].timestamp * 1000);

								var update = {
									x : [[time]],
									y : [[deviceData[sensorTypeName][sensorData]]],
									text: [[deviceData[sensorTypeName][sensorData] + sensorUnit[sensorTypeName]]]
								}

								var olderTime = time.setMinutes(time.getMinutes() - 1);
								var futureTime = time.setMinutes(time.getMinutes() + 1);

								var minuteView = {
									xaxis : {
										title: 'Time',
										type : 'date',
										range : [olderTime, futureTime]
									}
								};

								Plotly.relayout('liveChart', minuteView);
								Plotly.extendTraces('liveChart', update, [0]);						
							}  
					}
					      
				}
			} else {
				alert("No data found")
			}			
		},
		error: function(error) {
			console.log(error);
		},
		cache: false
	});
}

function liveGraph() {
	window.localStorage.clear();

	var sensorTypeName = $("#sensor").val().toLowerCase();
	var deviceId = $("#DeviceId").val();

	
	$.ajax({
		type: "GET",
		url: "/devices/get",
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
			var reportedObj = result.state.reported;
      		var sensorTypeArray = ["temperature", "ambient", "pressure", "humidity","accelerometer", "gyroscope", "compass"]
                   
			$("#chartsDiv").empty();
	  		$("#chartsDiv").addClass("mdl-color--grey-200")
			$("#chartsDiv").append($("<h5 class='mdl-cell mdl-cell--12-col'></h5>")
											.css("text-align", "center").text(deviceId + " Info"));

			if(intervalFunction){
				clearInterval(intervalFunction)
			}

			if(result.state && reportedObj) {			
				var deviceData = reportedObj[deviceId]
				if(deviceData) {
					if(!deviceData[sensorTypeName]){
						console.log("Sensor is not present....")
						$("#chartsDiv").append($("<h6></h6>").css("text-align", "center")
														.text("OOP's !! No "+sensorTypeName+" sensor found"));
					} else {						
						var chartBox = $("<div></div>")
									.addClass("mdl-cell mdl-cell--12-col mdl-cards mdl-shadow--2dp graphspace")
									.attr("id", "liveChart");

						$("#chartsDiv").append(chartBox);    

						if(Object.keys(deviceData[sensorTypeName]).length == 1) {
							
							var sensorDataForChart = [];
							switch(sensorTypeName) {
								case "accelerometer":
								case "gyroscope":
								case "compass": 
									for(sensorData in deviceData[sensorTypeName]){
										var sensorObjx = { name: 'xAxis', x:[], y:[], text:[], line: { shape : 'spline'}};
										var sensorObjy = { name: 'yAxis', x:[], y:[], text:[], line: { shape : 'spline'}};
										var sensorObjz = { name: 'zAxis', x:[], y:[], text:[], line: { shape : 'spline'}};
							
										sensorObjx.x.push(new Date(result.metadata.reported[deviceId][sensorTypeName][sensorData]["X"].timestamp));
										sensorObjy.x.push(new Date(result.metadata.reported[deviceId][sensorTypeName][sensorData]["X"].timestamp));
										sensorObjz.x.push(new Date(result.metadata.reported[deviceId][sensorTypeName][sensorData]["X"].timestamp));
					
										sensorObjx.y.push(deviceData[sensorTypeName][sensorData]["X"]);
										sensorObjy.y.push(deviceData[sensorTypeName][sensorData]["Y"]);
										sensorObjz.y.push(deviceData[sensorTypeName][sensorData]["Z"]);
										
										sensorDataForChart.push(sensorObjx, sensorObjy, sensorObjz);																	
									}
									break;
								default:
									for(sensorData in deviceData[sensorTypeName]){	
										var sensorObj = { name: sensorData, x:[], y: [], text:[], line : { shape : 'spline'}};
											
										sensorObj.x.push(new Date( result.metadata.reported[deviceId][sensorTypeName][sensorData].timestamp * 1000));
										sensorObj.y.push(deviceData[sensorTypeName][sensorData])
										sensorObj.text.push(deviceData[sensorTypeName][sensorData] + sensorUnit[sensorTypeName]);
													
										sensorDataForChart.push(sensorObj);
									}						
							}

							layout.title = $('#sensor').val() + ' Info';
							layout.yaxis.title = $("#sensor").val();
							Plotly.plot('liveChart', sensorDataForChart, layout, {displayModeBar: false});

							intervalFunction = setInterval(function(){
								requestData(sensorTypeName,deviceId);
							}, ChartpollingTime);

						} else {
							$("#chartsDiv").append($("<h6></h6>").css("text-align", "center")
							.text("OOP's !! Too many sensors"));
						}						
					}
				}
			} else {
				alert("No data found")
			}	
		},
		error: function(error) {
			console.log(error);
		}
	});	
}

loadSensorData();
