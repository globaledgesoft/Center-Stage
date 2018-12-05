/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package com.qualcomm.qti.qca40xx.Util;

import java.util.HashMap;
import java.util.UUID;

/**
 * This is a Constants class used in the application
 */

public class Constants {
    public static final String WIFI_BLE_FILTER_KEYWORD = "QCA";
    public static final String RANDOM_KEY = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890";
    public static final String BLE_DEVICE_NAME = "deviceName";
    public static final String BLE_DEVICE_ADDRESS = "deviceAddress";
    public static final String BLUETOOTH_DEVICE = "bluetoothDevice";
    public static final String IP_ADDRESS = "ipAddress";
    public static final String IP = "ip";
    public static final String TCP_FIRST_DATA = "Hello";
    public static final String DEVICE_NAME = "deviceName";
    public static final String MAC_ADDRESS = "macAddress";
    public static final String LOG_TAG = "log";
    public static final String NETWORK_ID = "networkId";
    public static final String READ_DATA = "data";
    public static final String JSON_KEY_ACTION = "Action";
    public static final String WIFI_SECURITY_WEP = "WEP";
    public static final String WIFI_SECURITY_WPA = "WPA";

    public static final int VALIDATION_SSID = 32;
    public static final int VALIDATION_MIN_PASSWORD = 8;
    public static final int VALIDATION_MAX_PASSWORD = 25;

    //Toolbar title
    public static final String TITLE_ONBOARDING_DETAILS = "QCAOnboard";
    public static final String TITLE_SETTING = "Settings";
    public static final String TITLE_LIST_COORDINATOR = "Coordinator List";


    //ble service & characteristics
    //SERVICE
    public static final UUID GATT_WIFI_SERVICE1 = UUID.fromString("00001118-0000-1000-8000-00805f9b34fb");
    public static final UUID GATT_ZIGBEE_SERVICE1 = UUID.fromString("00001123-0000-1000-8000-00805f9b34fb");
    public static final UUID GATT_THREAD_SERVICE1 = UUID.fromString("00001130-0000-1000-8000-00805f9b34fb");
    //WRITE
    public static final UUID GATT_WIFI_SSID_CHARECTERISTIC = UUID.fromString("00001120-0000-1000-8000-00805f9b34fb");
    public static final UUID GATT_WIFI_PASSWORD_CHARECTERISTIC = UUID.fromString("00001121-0000-1000-8000-00805f9b34fb");
    public static final UUID GATT_WIFI_TRIGGER_POINT_CHARECTERISTIC = UUID.fromString("00001122-0000-1000-8000-00805f9b34fb");

    public static final UUID GATT_ZIGBEE_LINKKEY_CHARECTERISTIC = UUID.fromString("00001127-0000-1000-8000-00805f9b34fb");
    public static final UUID GATT_ZIGBEE_OPERATION_MODE_CHARECTERISTIC = UUID.fromString("00001126-0000-1000-8000-00805f9b34fb");
    public static final UUID GATT_ZIGBEE_TRIGGER_POINT_CHARECTERISTIC = UUID.fromString("00001128-0000-1000-8000-00805f9b34fb");

    public static final UUID GATT_THREAD_LINKKEY_CHARECTERISTIC = UUID.fromString("00001134-0000-1000-8000-00805f9b34fb");
    public static final UUID GATT_THREAD_OPERATION_MODE_CHARECTERISTIC = UUID.fromString("00001133-0000-1000-8000-00805f9b34fb");
    public static final UUID GATT_THREAD_TRIGGER_POINT_CHARECTERISTIC = UUID.fromString("00001135-0000-1000-8000-00805f9b34fb");

    //READ
    public static final UUID GATT_WIFI_STATUS_CHARECTERISTIC = UUID.fromString("00001119-0000-1000-8000-00805f9b34fb");

    public static final UUID GATT_ZIGBEE_STATUS_CHARECTERISTIC = UUID.fromString("00001124-0000-1000-8000-00805f9b34fb");
    public static final UUID GATT_ZIGBEE_SUPPORT_MODE_CHARECTERISTIC = UUID.fromString("00001125-0000-1000-8000-00805f9b34fb");

    public static final UUID GATT_THREAD_STATUS_CHARECTERISTIC = UUID.fromString("00001131-0000-1000-8000-00805f9b34fb");
    public static final UUID GATT_THREAD_SUPPORT_MODE_CHARECTERISTIC = UUID.fromString("00001132-0000-1000-8000-00805f9b34fb");
    //offline
    public static final UUID GATT_OFFLINE_ADVERTISEMENT_CHARACTERISTIC = UUID.fromString("00001818-0000-1000-8000-00805f9b34fb");
    public static final UUID GATT_OFFLINE_ONBOARDED_CHARACTERISTIC = UUID.fromString("00001919-0000-1000-8000-00805f9b34fb");
    public static final UUID GATT_OFFLINE_ONBOARDED_COORDINATOR_CHARACTERISTIC = UUID.fromString("00002020-0000-1000-8000-00805f9b34fb");

    public static final UUID GATT_OFFLINE_SERVICE = UUID.fromString("00001136-0000-1000-8000-00805f9b34fb");
    public static final UUID GATT_OFFLINE_READ_WRITE_CHARACTERISTIC = UUID.fromString("00001137-0000-1000-8000-00805f9b34fb");
    public static final UUID GATT_OFFLINE_NOTIFY_CHARACTERISTIC = UUID.fromString("00001138-0000-1000-8000-00805f9b34fb");

    public static final int PORT = 6000;
    public static final int REQUEST_PERMISSIONS = 1;

    public static final boolean BLE_FLAG = true;
    public static final boolean WIFI_FLAG = true;

    //zigbee modes
    public static final String COORDINATOR = "Coordinator";
    public static final String ROUTER = "Router";
    public static final String ENDDEVICE = "End-Device";
    public static final String SUPPORT_MODE_BLE = "Ble";
    public static final String SUPPORT_MODE_WIFI = "Wifi";
    //thread modes
    public static final String BORDER_ROUTER = "Border-Router";
    public static final String JOINER_ROUTER = "Joiner-Router";
    public static final String JOINER = "Joiner";
    //Board JSON Keys
    public static final String JSON_FW_VER = "FwVer";
    public static final String JSON_CHIPSET = "Chipset";
    public static final String JSON_BAT_MODE = "BatMode";
    public static final String JSON_OPERATION_MODE = "OperationMode";
    public static final String JSON_ONBOARDED = "OnBoarded";
    public static final String JSON_JBMODE = "ZBModes";
    public static final String JSON_TMODE = "ThreadModes";
    //Dialog messages
    public static final String DEFAULT_DIALOG = "Loading...";
    public static final String PAIR_DIALOG = "Pairing...";
    public static final String QR_DIALOG = "Getting details from board...";
    public static final String ZIGBEE_MODE_DIALOG = "Fetching Zigbee details...";
    public static final String ZIGBEE_ONBOARD_DIALOG = "Zigbee Onboarding...";
    public static final String THREAD_MODE_DIALOG = "Fetching Thread details...";
    public static final String THREAD_ONBOARD_DIALOG = "Thread Onboarding...";
    public static final String STATUS_BOARD_DIALOG = "Verifying board status...";
    public static final String SCAN_DEVICE_DIALOG = "Scanning Device...";
    public static final String UPDATING_THERMOSTAT = "Updating Thermostat Data...";

    //onBoard status
    public static final String STATUS_ONBOARDED = "1";
    public static final String STATUS_NOT_ONBOARDED = "0";
    public static final String SPLIT_STRING = ";";

    public static final String QR_SCANNING = "QRSCAN";
    public static final String MANUAL_SCANNING = "MANUALSCAN";
    public static final String CALLING_ACTIVITY = "callingActivity";

    //paring ble statement
    public static final String REMOVE_BOND = "removeBond";
    public static final String CONVERT_PIN_TO_BYTES = "convertPinToBytes";
    public static final String SET_PIN = "setPin";
    public static final String CREATE_BOND = "createBond";
    //SCAN
    public static final String BLE = "ble";
    public static final String WIFI = "wifi";
    public static final String ZIGBEE = "zigbee";
    public static final String THREAD = "thread";
    public static final String NONE = "none";
    public static final String WIFI_THREAD = "wifi;thread";
    public static final String WIFI_ZIGBEE = "wifi;zigbee";


    //wifi mode constants
    public static final int WIFI_SINGLE_MODE_ONBOARDING = 1;
    public static final int WIFI_DOUBLE_MODE_ONBOARDING = 2;
    public static final int WIFI_TRIPLE_MODE_ONBOARDING = 3;
    //ble mode constants
    public static final int BLE_WIFI_ZIGBEE_MODE_ONBOARDING = 1;
    public static final int BLE_WIFI_THREAD_MODE_ONBOARDING = 2;
    public static final int BLE_ZIGBEE_THREAD_MODE_ONBOARDING = 4;
    public static final int BLE_TRIPLE_MODE_ONBOARDING = 3;

    public static final String MAIN_DEVICE_NAME = "maindeviceName";
    public static final String SUBDEVICE_ID ="subdeviceId" ;
    public static final String SENSOR_JSON_STRING = "sensor_data";
    public static final String INTENT_FILTER_SENSOR_DATA = "sensorDataFilter";


    public static final String LIGHT = "light";
    public static final String COMPASS = "comp";
    public static final String ACCELEROMETER = "accel";
    public static final String TEMPERATURE = "temp";
    public static final String HUMIDITY = "hum";
    public static final String AMBIENT = "amb";
    public static final String PRESSURE = "pres";
    public static final String GYROSCOPE = "gyro";
    public static final String THERMOSTAT = "thermo";
    public static final String DIMMER = "dimm";

    public static final String REPORTED = "reported";
    public static final String FETCHING_SENSOR = "Fetching Sensor List...";
    public static final String CONTENTS_NOT = "Content not available";
    public static final String SUBDEVICE_KEY ="dName" ;
    public static final String SENSOR_JSON_HEAD = "sensors";
    public static final String MAIN_DEVICE_KEY = "dName";
    public static final String GETLIST = "getList" ;
    public static final String DEVICE_LIST ="dList" ;
    public static final String UPDATION_THERMO = "Updation not successfull! please try again";
    public static final String UPDATION_THERMO_SUCCESS = "Updation successfull!";

    public static final CharSequence FETCHING_DEVICE_LIST ="Fetching device List.." ;

    public static final String ZERO = "0";
    public static final String ONE = "1";
    public static final String THERMO_POLLING = "thermo_polling" ;
    public static final String UPDATE = "thermo_update" ;

    public static boolean onToggleClicked = false;
    public static final String THERMO_OPR_STATE = "op_state";
    public static final String THERMO_CUR_STATE = "actual";
    public static final String THERMO_THRESHOLD = "threshold";
    public static final String DESIRED = "desired";
    public static final String THERMO_MODE = "op_mode";
    public static final long DEGREEVALUE = 0x00B0;
    public static final String DEGREEUNIT = "C";
    public static final String AC_ON = "AC_ON";
    public static final String HEATER_ON = "HEATER_ON";
    public static final String auto = "auto";
    public static final String OFF = "OFF";
    public static final String AUTO = "AUTO";
    public static final String AC = "AC";
    public static final String HEATER = "HEATER";
    public static final String STANDBY = "STANDBY";

    public static final int SEEKMAX = 35;
    public static final int POLLING_FAIL_COUNT = 4;
    public static final int THERMO_APPLY_FAIL_COUNT = 10;
    public static final int SEEKTHRESHOLD = 5;

    public static final String ONBOARD = "onboard";
    public static final String OFFLINE = "offline";
    public static final String FROM = " from the ";
    public static final String MESSAGE = "message";
    public static final String SENSORDATA_UNABLE_TO_FETCH = "Unable to fetch sensor details please try again";
    public static final String ONLINE_APP = "com.qualcomm.qti.qca40xx.sensor";
    public static final String YES = "yes";
    public static final String NO = "no";
    public static final String OFFLINE_ONBOARD_DEVICE = "offlineAndOnboard";
    public static final String ONLINE_ONBOARD_DEVICE = "onlineAndOnboard";


    public static HashMap<String, Integer> valueStored = new HashMap<>();


    public static String FIRST ="first";
    public static  String isCondition="";
    public static  int DIMMER_DATA =0 ;
    public static final String STATE = "state";



}
