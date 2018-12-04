/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package com.qualcomm.qti.qca40xx.sensor.Utils;

import java.util.HashMap;

/**
 * Constants used in the application
 */

public class Constants {

    public static final String ACCESS_KEY = "accessKey";
    public static final String SECRET_KEY = "secretKey";
    public static final String LAST_UPDATED = "lastUpdated";
    public static final String REGION = "region";
    public static final String ACCESSED = "accessed";
    public static final String CREDENTIALS = "credentials.csv";
    public static final String UPDATE_NOTSUCESS = "Something went wrong please try again";

    public static final String MESSAGE = "message";
    public static final String CUSTOMER_ENDPOINT = "customer_endpoint";
    public static final String THRESHOLD_BREACHED = "threshold_breached";
    public static final String ACTION = "com.qualcomm.qti.qca40xx.sensor.Service";

    public static final String KEY = "key";
    public static final String THING_FILTER = "qcathings";
    public static final String THING_KEY = "thingName";
    public static final String THING = "thing";
    public static final String STATE = "state";
    public static final String REPORTED = "reported";
    public static final String THERMOSTAT = "thermostat";
    public static final String DESIRED = "desired";
    public static final String TIMESTAMP = "timestamp";
    public static final String DELTA = "delta";

    //bundle
    public static final String NAME = "NAME";
    public static final String SENSOR_DETAILS = "sensorDetails";
    public static final String DEVICE_NAME = "deviceName";
    public static final String SENSOR_JSON = "sensorJson";

    public static final String THERMO_OPR_STATE = "op_state";
    public static final String THERMO_CUR_STATE = "actual";
    public static final String THERMO_THRESHOLD = "threshold";
    public static final String THERMO_DESIRED = "desired";
    public static final String THERMO_MODE = "op_mode";


    public static final String OK = "Ok";
    public static final String PERMISSSION = "Write External Storage permission allows us to do store images. Please allow this permission in App Settings.";
    public static final String CANCEL = "Cancel";
    public static final String ITEM = "No Item Found";
    public static final String CSV_FILES = "Looking for (credentials.csv) files...";
    public static final String CSV = ".csv";
    public static final String CREDENTIALS_CLEARED = "Credentials has been cleared";
    public static final String FETCHING_SENSOR = "Fetching Sensor List...";
    public static final String CONTENTS_NOT = "Content not available";
    public static final String THINGS_NOT_AVAILABLE = "Things not available for this region";
    public static final String LOADING_THINGS = "Loading Things Details...";
    public static final String FETCHING_THERMOSTAT = "Fetching Thermostat Data...";
    public static final String FETCHING_DEVICE_LIST = "Fetching Devices List...";
    public static final String THINGS_NOT = "Things not available";
    public static final String FETCHING_THINGS = "Fetching Things List...";
    public static final String FETCHING_ENDPOINT = "Fetching EndPoint ...";
    public static final String UPDATING_THERMOSTAT = "Updating Thermostat Details...";
    public static final String LAST_UPDATED_AT = "Last Updated At :";

    public static final String LIGHT = "light";
    public static final String COMPASS = "compass";
    public static final String ACCELEROMETER = "accelerometer";
    public static final String TEMPERATURE = "temperature";
    public static final String HUMIDITY = "humidity";
    public static final String AMBIENT = "ambient";
    public static final String PRESSURE = "pressure";
    public static final String GYROSCOPE = "gyroscope";
    public static final String DIMMER = "dimmer";


    public static final String SERVICE_NAME = "serviceName";
    public static final String RESULT_VALUE = "resultValue";
    public static final String THRESHOLD_MESSAGE = "thresholdMessage";
    public static final String CONNECTED = "Connected";
    public static final String FROM = " from the ";
    public static final String OF = " of ";
    public static final String DEVICE_ACTIVITY = "deviceActivity";
    public static final String LIST_DEVICE = "listDevice";
    public static final String LIST_THINGS = "listThings";


    public static final String ZERO = "0";
    public static final String ONE = "1";
    public static final String TRUE = "true";
    public static final String FALSE = "false";
    public static final String ON = "ON";

    public static final int SYNC_TIME = 6000;
    public static final int THREAD_TIME = 2000;

    public static final String ac_on = "ac_on";
    public static final String AC_ON = "AC_ON";
    public static final String heater_on = "heater_on";
    public static final String HEATER_ON = "HEATER_ON";
    public static final String auto = "auto";
    public static final String STAND_BY = "STAND_BY";
    public static final String off = "off";
    public static final String OFF = "OFF";
    public static final String AUTO = "AUTO";
    public static final String AC = "AC";
    public static final String HEATER = "HEATER";
    public static final String STANDBY = "STANDBY";

    public static final int SEEKMAX = 35;
    public static final int SEEKTHRESHOLD = 5;

    public static final long DEGREEVALUE = 0x00B0;
    public static final String DEGREEUNIT = "C";
    public static boolean isDeltaPresent = false;

}
