/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package com.qualcomm.qti.qca40xx.sensor.Utils;

import android.content.Context;
import android.content.SharedPreferences;
import android.os.AsyncTask;

import com.amazonaws.auth.BasicAWSCredentials;
import com.amazonaws.regions.Region;
import com.amazonaws.regions.Regions;
import com.amazonaws.services.iotdata.AWSIotDataClient;
import com.amazonaws.services.iotdata.model.GetThingShadowRequest;
import com.amazonaws.services.iotdata.model.GetThingShadowResult;
import com.qualcomm.qti.qca40xx.sensor.Interfaces.IRefreshData;
import com.qualcomm.qti.qca40xx.sensor.Interfaces.IRefreshThermostat;
import com.qualcomm.qti.qca40xx.sensor.Model.SensorDetails;

import org.json.JSONObject;

import java.sql.Timestamp;
import java.util.ArrayList;
import java.util.Date;
import java.util.Iterator;


/**
 * This class is used for polling data
 */

public class ShadowResults {
    private String TAG = getClass().getSimpleName();
    private Context context;
    private String mDeviceName;
    private String mThingName;
    private String itemName;
    private String thingName;
    private String deviceName;
    private String reportedData;
    private String jSensorData;
    private IRefreshData mInterfaceObject;
    private IRefreshThermostat mInterfaceThermostat;
    private BasicAWSCredentials sAWSCredentials;
    private String accessKey, secreteKey, regionEndpoint;
    private AWSIotDataClient iotDataClient;
    private ArrayList<SensorDetails> sensorDetailsArray = new ArrayList<>();
    private SharedPreferences sharedpreferences;


    public ShadowResults(Context context, IRefreshData interfaceObject, String mDeviceName, String mThingName, String mAccesskey, String mSecretkey) {
        this.context = context;
        this.mInterfaceObject = interfaceObject;
        this.mDeviceName = mDeviceName;
        this.mThingName = mThingName;
        this.accessKey = mAccesskey;
        this.secreteKey = mSecretkey;
        GetKeyFromSharedPreference();
        if (sAWSCredentials == null) {
            sAWSCredentials = new BasicAWSCredentials(accessKey, secreteKey);

        }
        iotDataClient = new AWSIotDataClient(sAWSCredentials);
        GetDynamicSensorList getDynamicSensorList = new GetDynamicSensorList();
        getDynamicSensorList.execute();
    }


    public ShadowResults(Context context, IRefreshThermostat interfaceObject, String deviceName, String itemName, String thingName, String accessKey, String secreteKey) {

        this.context = context;
        this.mInterfaceThermostat = interfaceObject;
        this.deviceName = deviceName;
        this.itemName = itemName;
        this.thingName = thingName;
        this.accessKey = accessKey;
        this.secreteKey = secreteKey;
        GetKeyFromSharedPreference();
        if (sAWSCredentials == null) {
            sAWSCredentials = new BasicAWSCredentials(accessKey, secreteKey);
        }
        iotDataClient = new AWSIotDataClient(sAWSCredentials);
        GetDynamicThermostat getDynamicThermostat = new GetDynamicThermostat();
        getDynamicThermostat.execute();
    }


    public void GetKeyFromSharedPreference() {
        try {
            sharedpreferences = context.getSharedPreferences(Constants.KEY,
                    Context.MODE_PRIVATE);
            regionEndpoint = sharedpreferences.getString(Constants.REGION, "");
        } catch (Exception e) {
            Logger.d(TAG, "Exception in GetKeyFromSharedPreference : " + e);
        }
    }

    private void updateSensorDetailObject(String currentDynamicKey, String childData, String childValue) {
        SensorDetails sensorDetails = new SensorDetails();
        sensorDetails.setParentType(currentDynamicKey);
        sensorDetails.setSensorKey(childData);
        sensorDetails.setSensorValue(childValue);
        sensorDetailsArray.add(sensorDetails);
    }

    private class GetDynamicSensorList extends AsyncTask<Void, Void, String> {

        @Override
        protected String doInBackground(Void... voids) {

            try {
                GetThingShadowRequest getThingShadowRequest = new GetThingShadowRequest().withThingName(mThingName);
                iotDataClient.setRegion(Region.getRegion(Regions.fromName(regionEndpoint)));
                GetThingShadowResult result = iotDataClient.getThingShadow(getThingShadowRequest);
                byte[] bytes = new byte[result.getPayload().remaining()];
                result.getPayload().get(bytes);
                String sensorResultString = new String(bytes);

                JSONObject timestampObject = new JSONObject(sensorResultString);
                long timeStamp = timestampObject.getInt(Constants.TIMESTAMP);
                Timestamp ts = new Timestamp(timeStamp * 1000);
                Date date = new Date(ts.getTime());

                SharedPreferences pref = context.getSharedPreferences(Constants.KEY, 0); // 0 - for private mode
                SharedPreferences.Editor editor = pref.edit();
                editor.putString(Constants.LAST_UPDATED, date.toString());
                editor.apply();

                JSONObject jsonObject = new JSONObject(sensorResultString);
                Logger.d(TAG, "sensorResultString : " + sensorResultString);
                JSONObject stateObject = jsonObject.getJSONObject(Constants.STATE);
                JSONObject reportedObject = stateObject.getJSONObject(Constants.REPORTED);
                JSONObject currentDynamicValues = reportedObject.getJSONObject(mDeviceName);
                reportedData = currentDynamicValues.toString();

                JSONObject sensorsJson = new JSONObject(reportedData);
                Iterator<String> keys = sensorsJson.keys();
                while (keys.hasNext()) {
                    String currentDynamicKey = keys.next();
                    if (!(currentDynamicKey.contains(Constants.THERMOSTAT))) {
                        JSONObject jObj = sensorsJson.getJSONObject(currentDynamicKey);
                        Iterator<String> childKeys = jObj.keys();
                        while (childKeys.hasNext()) {
                            String childData = childKeys.next();
                            String childValue = jObj.getString(childData);
                            Logger.d(TAG, "values  " + childData + " " + childValue);

                            switch (currentDynamicKey) {
                                case Constants.LIGHT:
                                    updateSensorDetailObject(currentDynamicKey, childData, childValue);
                                    break;
                                case Constants.COMPASS:
                                    updateSensorDetailObject(currentDynamicKey, childData, childValue);
                                    break;
                                case Constants.ACCELEROMETER:
                                    updateSensorDetailObject(currentDynamicKey, childData, childValue);
                                    break;
                                case Constants.TEMPERATURE:
                                    updateSensorDetailObject(currentDynamicKey, childData, childValue);
                                    break;
                                case Constants.HUMIDITY:
                                    updateSensorDetailObject(currentDynamicKey, childData, childValue);
                                    break;
                                case Constants.AMBIENT:
                                    updateSensorDetailObject(currentDynamicKey, childData, childValue);
                                    break;
                                case Constants.PRESSURE:
                                    updateSensorDetailObject(currentDynamicKey, childData, childValue);
                                    break;
                                case Constants.GYROSCOPE:
                                    updateSensorDetailObject(currentDynamicKey, childData, childValue);
                                    break;
                                case Constants.DIMMER:
                                    updateSensorDetailObject(currentDynamicKey, childData, childValue);
                                    break;
                            }
                        }
                    }
                }
                mInterfaceObject.setRefereshData(sensorDetailsArray, sensorResultString);
            } catch (Exception e) {
                Logger.d(TAG, "Exception in GetDynamicSensorList : " + e);
                e.printStackTrace();
            }

            return null;
        }
    }

    private class GetDynamicThermostat extends AsyncTask<Void, Void, String> {

        @Override
        protected String doInBackground(Void... voids) {

            try {
                GetThingShadowRequest getThingShadowRequest = new GetThingShadowRequest().withThingName(thingName);
                iotDataClient.setRegion(Region.getRegion(Regions.fromName(regionEndpoint)));
                GetThingShadowResult result = iotDataClient.getThingShadow(getThingShadowRequest);
                byte[] bytes = new byte[result.getPayload().remaining()];
                result.getPayload().get(bytes);
                String sensorResultString = new String(bytes);

                JSONObject timestampObject = new JSONObject(sensorResultString);
                long timeStamp = timestampObject.getInt(Constants.TIMESTAMP);
                Timestamp ts = new Timestamp(timeStamp * 1000);
                Date date = new Date(ts.getTime());

                SharedPreferences pref = context.getSharedPreferences(Constants.KEY, 0); // 0 - for private mode
                SharedPreferences.Editor editor = pref.edit();
                editor.putString(Constants.LAST_UPDATED, date.toString());
                editor.apply();

                JSONObject jsonObject = new JSONObject(sensorResultString);
                JSONObject stateObject = jsonObject.getJSONObject(Constants.STATE);
                JSONObject reportedObject = stateObject.getJSONObject(Constants.REPORTED);
                JSONObject currentDynamicValues = reportedObject.getJSONObject(deviceName);
                JSONObject thermostatValues = currentDynamicValues.getJSONObject(Constants.THERMOSTAT);
                JSONObject thermoValues = thermostatValues.getJSONObject(itemName);
                jSensorData = thermoValues.toString();
                mInterfaceThermostat.setThermoData(jSensorData, sensorResultString);
            } catch (Exception e) {
                Logger.d(TAG, "Exception in GetDynamicThermostat : " + e);
                e.printStackTrace();
            }
            return null;
        }
    }

}
