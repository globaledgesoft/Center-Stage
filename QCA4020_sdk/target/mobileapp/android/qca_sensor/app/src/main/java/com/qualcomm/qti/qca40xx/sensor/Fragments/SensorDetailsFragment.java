/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package com.qualcomm.qti.qca40xx.sensor.Fragments;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.support.annotation.Nullable;
import android.support.design.widget.CoordinatorLayout;
import android.support.v4.app.Fragment;
import android.support.v7.widget.DefaultItemAnimator;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;
import android.widget.Toast;

import com.qualcomm.qti.qca40xx.sensor.Adapters.SensorDetailsAdapter;
import com.qualcomm.qti.qca40xx.sensor.Interfaces.IRefreshData;
import com.qualcomm.qti.qca40xx.sensor.Interfaces.IUpdationSensorData;
import com.qualcomm.qti.qca40xx.sensor.Model.SensorDetails;
import com.qualcomm.qti.qca40xx.sensor.Service.NotificationService;
import com.qualcomm.qti.qca40xx.sensor.Utils.Constants;
import com.qualcomm.qti.qca40xx.sensor.Utils.Logger;
import com.qualcomm.qti.qca40xx.sensor.Utils.ShadowResults;
import com.qualcomm.qti.qca40xx.sensor.R;
import com.qualcomm.qti.qca40xx.sensor.Utils.Util;

import org.json.JSONObject;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.Timer;
import java.util.TimerTask;

/**
 * Sensor Tab which displays the details of the Sensors
 */

public class SensorDetailsFragment extends Fragment implements IRefreshData, IUpdationSensorData {

    private String TAG = getClass().getSimpleName();
    private static SensorDetailsAdapter adapter;
    private RecyclerView recyclerView;
    private RecyclerView.LayoutManager layoutManager;
    private String jsonData, complete_jsonData;
    private String deviceName;
    private ArrayList<SensorDetails> sensorDetailsArray = new ArrayList<>();
    public Timer repeatRefresh;
    private TimerTask task;
    private SharedPreferences sharedpreferences;
    private String accessKey, secreteKey;
    private String thingsName, lastUpdated;
    private TextView txtLastUpdated;
    private Activity mActivity;
    private boolean isUpdationTriggered = false;


    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        jsonData = getActivity().getIntent().getStringExtra(Constants.SENSOR_JSON);
        complete_jsonData = getActivity().getIntent().getStringExtra(Constants.SENSOR_DETAILS);
        deviceName = getActivity().getIntent().getStringExtra(Constants.DEVICE_NAME);
        GetKeyFromSharedPreference();
        GetThingNameSharedPreference();
        mActivity = getActivity();
    }


    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {

        View view = inflater.inflate(R.layout.fragment_sensor_list, container, false);
        mActivity = getActivity();
        isUpdationTriggered = false;
        recyclerView = (RecyclerView) view.findViewById(R.id.recyclerViewSensors);
        txtLastUpdated = (TextView) view.findViewById(R.id.txtLastUpdated);
        txtLastUpdated.setText(Constants.LAST_UPDATED_AT + " " + lastUpdated);
        layoutManager = new LinearLayoutManager(getActivity());
        recyclerView.setLayoutManager(layoutManager);
        recyclerView.setItemAnimator(new DefaultItemAnimator());
        jsonParsing(jsonData);
        adapter = new SensorDetailsAdapter(getActivity(), SensorDetailsFragment.this, deviceName, sensorDetailsArray, complete_jsonData);
        recyclerView.setAdapter(adapter);
        if (isAdded() && mActivity != null) {
            startTimer();
        }
        return view;
    }

    private void jsonParsing(String data) {
        try {
            JSONObject jsonObject = new JSONObject(data);
            Iterator<String> keys = jsonObject.keys();
            while (keys.hasNext()) {
                String currentDynamicKey = keys.next();
                if (!(currentDynamicKey.contains(Constants.THERMOSTAT))) {
                    JSONObject jObj = jsonObject.getJSONObject(currentDynamicKey);
                    Logger.d(TAG, "deviceId Value " + jObj);
                    Iterator<String> childKeys = jObj.keys();
                    while (childKeys.hasNext()) {
                        String childData = childKeys.next();
                        String childValue = jObj.getString(childData);

                        switch (currentDynamicKey) {
                            case Constants.LIGHT:
                                Logger.d(TAG, "deviceId Value1 " + jObj);
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
        } catch (Exception e) {
            Logger.d(TAG, "Exception in jsonParsing : " + e);
            e.printStackTrace();
        }
    }

    private void updateSensorDetailObject(String currentDynamicKey, String childData, String childValue) {
        try {
            SensorDetails sensorDetails = new SensorDetails();
            sensorDetails.setParentType(currentDynamicKey);
            sensorDetails.setSensorKey(childData);
            sensorDetails.setSensorValue(childValue);
            sensorDetailsArray.add(sensorDetails);
        } catch (Exception e) {
            Logger.d(TAG, "Exception in updateSensorDetailObject : " + e);
        }
    }

    private void GetKeyFromSharedPreference() {
        try {
            sharedpreferences = getActivity().getSharedPreferences(Constants.KEY,
                    Context.MODE_PRIVATE);
            accessKey = sharedpreferences.getString(Constants.ACCESS_KEY, "");
            secreteKey = sharedpreferences.getString(Constants.SECRET_KEY, "");
            lastUpdated = sharedpreferences.getString(Constants.LAST_UPDATED, "");
        } catch (Exception e) {
            Logger.d(TAG, "Exception in GetKeyFromSharedPreference : " + e);
        }
    }


    private void GetThingNameSharedPreference() {
        try {
            SharedPreferences sharedpreferences = getActivity().getSharedPreferences(Constants.THING,
                    Context.MODE_PRIVATE);
            thingsName = sharedpreferences.getString(Constants.THING_KEY, "");
        } catch (Exception e) {
            Logger.d(TAG, "Exception in GetThingNameSharedPreference : " + e);
        }
    }


    private void startTimer() {
        repeatRefresh = new Timer();
        task = new TimerTask() {
            @Override
            public void run() {
                if (getActivity() == null)
                    return;
                getActivity().runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        new ShadowResults(getActivity(), SensorDetailsFragment.this, deviceName, thingsName, accessKey, secreteKey);
                    }
                });
            }
        };
        repeatRefresh.schedule(task, Constants.THREAD_TIME);
    }

    @Override
    public void setRefereshData(ArrayList<SensorDetails> senDetailArray, String sensorResultString) {
        if (!isUpdationTriggered) {
            updateSensor(senDetailArray, sensorResultString);
        } else {
            startTimer();
        }
    }

    public void updateSensor(ArrayList<SensorDetails> senDetailArray, String sensorResultString) {
        adapter.setData(getActivity(), deviceName, senDetailArray, sensorResultString);
        new Handler(Looper.getMainLooper()).post(new Runnable() {
            public void run() {
                lastUpdated = sharedpreferences.getString(Constants.LAST_UPDATED, "");
                txtLastUpdated.setText(Constants.LAST_UPDATED_AT + " " + lastUpdated);
                adapter.notifyDataSetChanged();
            }
        });
        startTimer();
    }

    @Override
    public void setUpdationTriggerd(boolean value) {
        isUpdationTriggered = value;
    }

}