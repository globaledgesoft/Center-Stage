/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package com.qualcomm.qti.qca40xx.sensor.Utils;

import android.content.Context;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;

import com.qualcomm.qti.qca40xx.sensor.Activity.SensorThermostatDeviceActivity;

import org.json.JSONObject;

import java.util.Iterator;

/**
 * This class has generic methods.
 */

public class Util {
    private static String TAG = "Util";

    public static boolean isConnectedToInternet(Context mContext) {
        ConnectivityManager connectivityManager
                = (ConnectivityManager) mContext.getSystemService(Context.CONNECTIVITY_SERVICE);
        NetworkInfo activeNetworkInfo = connectivityManager.getActiveNetworkInfo();
        return activeNetworkInfo != null && activeNetworkInfo.isConnected();
    }

    public static String getSensorDeltaValue(String Jsondata, String mDeviceName, String keyName, String childkey) {
        String childValue = null;
        Logger.d(TAG, "json in getSensorDeltaValue : " + Jsondata);
        try {
            JSONObject jsonObject = new JSONObject(Jsondata);
            JSONObject stateObject = jsonObject.getJSONObject(Constants.STATE);
            JSONObject deltaObject = stateObject.getJSONObject(Constants.DELTA);
            JSONObject deltaValues = deltaObject.getJSONObject(mDeviceName);
            JSONObject sensorsJson = new JSONObject(deltaValues.toString());
            Iterator<String> keys = sensorsJson.keys();
            while (keys.hasNext()) {
                String currentDynamicKey = keys.next();
                if ((currentDynamicKey.contains(keyName))) {
                    JSONObject jObj = sensorsJson.getJSONObject(currentDynamicKey);
                    Iterator<String> childKeys = jObj.keys();
                    while (childKeys.hasNext()) {
                        String childData = childKeys.next();
                        childValue = jObj.getString(childkey);
                    }
                }
            }
        } catch (Exception e) {
            Logger.d(TAG, "Exception in getSensorDeltaValue : " + e);
            e.printStackTrace();
        }
        return childValue;
    }


    public static String getSensorDesiredValue(String Jsondata, String reporteData, String mDeviceName, String keyName, String childKey) {
        String desiredChildValue = null;
        Logger.d(TAG, "json in getSensorDesiredValue : " + Jsondata);
        try {
            JSONObject jsonObject = new JSONObject(Jsondata);
            JSONObject stateObject = jsonObject.getJSONObject(Constants.STATE);
            JSONObject desiredObject = stateObject.getJSONObject(Constants.DESIRED);
            JSONObject desiredValues = desiredObject.getJSONObject(mDeviceName);
            try {
                JSONObject desiredJson = new JSONObject(desiredValues.toString());
                Iterator<String> keys1 = desiredJson.keys();
                while (keys1.hasNext()) {
                    String currentDynamicKey = keys1.next();
                    if ((currentDynamicKey.contains(keyName))) {
                        JSONObject jObj = desiredJson.getJSONObject(currentDynamicKey);
                        Iterator<String> childKeys = jObj.keys();
                        while (childKeys.hasNext()) {
                            String childData = childKeys.next();
                            desiredChildValue = jObj.getString(childKey);
                        }
                    }
                }
            } catch (Exception e) {
                Logger.d(TAG, "Exception 1 in getSensorDesiredValue " + e);
                desiredChildValue = null;
            }
            Logger.d(TAG, "compare : " + keyName + "" + reporteData + " and " + desiredChildValue);
        } catch (Exception e) {
            desiredChildValue = null;
            Logger.d(TAG, "Exception 2 in getSensorDesiredValue : " + e);
            e.printStackTrace();
        }
        return desiredChildValue;
    }


    public static String getThermoDeltaValue(String Jsondata, String mDeviceName, String keyName, String childkey) {
        String thermodelta = null;
        Logger.d(TAG, "json in getThermoDeltaValue : " + Jsondata);
        try {
            JSONObject jsonObject = new JSONObject(Jsondata);
            JSONObject stateObject = jsonObject.getJSONObject(Constants.STATE);
            JSONObject deltaObject = stateObject.getJSONObject(Constants.DELTA);
            JSONObject deltaValues = deltaObject.getJSONObject(mDeviceName);
            JSONObject thermostatValues = deltaValues.getJSONObject(keyName);
            JSONObject thermoValues = thermostatValues.getJSONObject(childkey);
            thermodelta = thermoValues.toString();
        } catch (Exception e) {
            thermodelta = null;
            Logger.d(TAG, "Exception in getThermoDeltaValue : " + e);
            e.printStackTrace();
        }

        return thermodelta;
    }


    public static String getThermoDesiredValue(String sensorResultString, String deviceName, String childkey) {
        String desiredThermos = null;
        try {
            JSONObject jsonObject = new JSONObject(sensorResultString);
            JSONObject stateObject = jsonObject.getJSONObject(Constants.STATE);
            JSONObject desiredObject = stateObject.getJSONObject(Constants.DESIRED);
            JSONObject currentDynamicValues1 = desiredObject.getJSONObject(deviceName);
            JSONObject thermostatValues1 = currentDynamicValues1.getJSONObject(Constants.THERMOSTAT);
            JSONObject thermoValues1 = thermostatValues1.getJSONObject(childkey);
            desiredThermos = thermoValues1.toString();
        } catch (Exception e) {
            desiredThermos = null;
            Logger.d(TAG, "Exception in getThermoDesiredValue : " + e);
        }
        return desiredThermos;
    }


}
