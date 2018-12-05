/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package com.qualcomm.qti.qca40xx.sensor.Fragments;

import android.app.Activity;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.SharedPreferences;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.support.annotation.Nullable;
import android.support.v4.app.Fragment;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.SeekBar;
import android.widget.Spinner;
import android.widget.TextView;

import com.amazonaws.auth.BasicAWSCredentials;
import com.amazonaws.regions.Region;
import com.amazonaws.regions.Regions;
import com.amazonaws.services.iotdata.AWSIotDataClient;
import com.amazonaws.services.iotdata.model.GetThingShadowRequest;
import com.amazonaws.services.iotdata.model.GetThingShadowResult;
import com.amazonaws.services.iotdata.model.UpdateThingShadowRequest;
import com.amazonaws.services.iotdata.model.UpdateThingShadowResult;
import com.qualcomm.qti.qca40xx.sensor.Activity.SensorThermostatDeviceActivity;
import com.qualcomm.qti.qca40xx.sensor.Interfaces.IRefreshThermostat;
import com.qualcomm.qti.qca40xx.sensor.Model.ThermoUpdateModel;
import com.qualcomm.qti.qca40xx.sensor.R;
import com.qualcomm.qti.qca40xx.sensor.Utils.Constants;
import com.qualcomm.qti.qca40xx.sensor.Utils.Logger;
import com.qualcomm.qti.qca40xx.sensor.Utils.ShadowResults;
import com.qualcomm.qti.qca40xx.sensor.Utils.Util;

import org.json.JSONException;
import org.json.JSONObject;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Timer;
import java.util.TimerTask;


public class ThermostatFragment extends Fragment implements IRefreshThermostat {


    private String TAG = getClass().getSimpleName();
    private SeekBar seekBar, seekBarThreshold;
    private TextView desiredTempTextView, thresholdValue;
    private TextView currentTempTextView, textviewOperatingState, txtLastUpdated;
    private Spinner spinnerData;
    private Button apply;
    private AWSIotDataClient iotDataClient;
    private BasicAWSCredentials sAWSCredentials;
    private ProgressDialog dialog;
    private ArrayAdapter<String> adapter;
    private SharedPreferences sharedpreferences;
    private String accessKey, secreteKey, regionEndpoint;
    private String deviceData, completeJson;
    private String operatingMode;
    private String mOperatingMode;
    private int temp, desiredTemp;
    private int threshold;
    private String thing;
    private String deviceName;
    private ImageView imageView;
    private String lastUpdated;
    private Timer repeatRefresh;
    private TimerTask task;
    private String selectedItemMode;
    private ArrayList<String> thermostatList;
    private int tempSelected, thresholdInt;
    private String currentDynamicKey;
    private Activity mActivity;

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mActivity = getActivity();
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        // Inflate the layout for this fragment
        View view = inflater.inflate(R.layout.fragment_thermostat_details, container, false);
        initialization(view);
        GetThingNameSharedPreference();
        GetKeyFromSharedPreference();
        deviceData = getActivity().getIntent().getStringExtra(Constants.SENSOR_JSON);
        completeJson = getActivity().getIntent().getStringExtra(Constants.SENSOR_DETAILS);
        try {
            JSONObject jsonObject = new JSONObject(deviceData);
            JSONObject thermostatObject = jsonObject.getJSONObject(Constants.THERMOSTAT);
            Iterator<String> keys = thermostatObject.keys();
            while (keys.hasNext()) {
                currentDynamicKey = keys.next();
                thermostatList.add(currentDynamicKey);
            }
        } catch (JSONException e) {
            Logger.d(TAG, "Exception in onCreateView : " + e);
            e.printStackTrace();
        }
        deviceName = thermostatList.get(0);
        if (deviceData != null) {
            GetThermostatData thermostatData = new GetThermostatData();
            thermostatData.execute();
            displayData();
            spinnerData.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
                @Override
                public void onItemSelected(AdapterView<?> adapterView, View view, int i, long l) {
                    selectedItemMode = adapterView.getItemAtPosition(i).toString();
                }

                @Override
                public void onNothingSelected(AdapterView<?> adapterView) {

                }
            });
            //Update Thermostat
            apply.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View view) {

                    try {
                        destroyTimer();
                        String tempValue = desiredTempTextView.getText().toString();
                        String splitDegree = (char) Constants.DEGREEVALUE + Constants.DEGREEUNIT;
                        tempValue = tempValue.replace(splitDegree, "");


                        tempSelected = Integer.parseInt(tempValue);
                        thresholdInt = Integer.parseInt(thresholdValue.getText().toString());


                        JSONObject updateValues = new JSONObject();
                        updateValues.putOpt(Constants.THERMO_THRESHOLD, thresholdInt);
                        updateValues.putOpt(Constants.THERMO_DESIRED, tempSelected);
                        updateValues.putOpt(Constants.THERMO_MODE, selectedItemMode);

                        JSONObject termoObj = new JSONObject();
                        termoObj.putOpt(deviceName, updateValues);

                        JSONObject termostat = new JSONObject();
                        termostat.putOpt(Constants.THERMOSTAT, termoObj);

                        JSONObject deviceIdData = new JSONObject();
                        deviceIdData.putOpt(SensorThermostatDeviceActivity.toolbarName, termostat);

                        JSONObject desiredJsonObject = new JSONObject();
                        desiredJsonObject.putOpt(Constants.DESIRED, deviceIdData);

                        JSONObject stateJsonObject = new JSONObject();
                        stateJsonObject.put(Constants.STATE, desiredJsonObject);
                        UpdateThermostatTask updateShadowTask = new UpdateThermostatTask(stateJsonObject + "");
                        updateShadowTask.execute();

                        apply.setClickable(false);
                        UpdateApplyBtnUI(false);
                        apply.setText(R.string.APPLYING);
                    } catch (JSONException e) {
                        Logger.d(TAG, "Exception in apply click : " + e);
                        e.printStackTrace();
                    }


                }
            });

            seekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {

                int progress = 0;

                @Override
                public void onProgressChanged(SeekBar seekBar, int progressValue, boolean b) {
                    progress = progressValue;
                    desiredTempTextView.setText("" + progressValue + (char) 0x00B0 + "C");
                }

                @Override
                public void onStartTrackingTouch(SeekBar seekBar) {

                }

                @Override
                public void onStopTrackingTouch(SeekBar seekBar) {

                }
            });

            seekBarThreshold.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {

                int progress = 0;

                @Override
                public void onProgressChanged(SeekBar seekBar, int progressValue, boolean b) {
                    progress = progressValue;
                    thresholdValue.setText("" + progressValue);
                }

                @Override
                public void onStartTrackingTouch(SeekBar seekBar) {

                }

                @Override
                public void onStopTrackingTouch(SeekBar seekBar) {

                }
            });
        }
        if (isAdded() && mActivity != null) {
            startTimer();
        }
        return view;
    }

    private void displayData() {
        try {
            seekBar.setMax(Constants.SEEKMAX);
            seekBarThreshold.setMax(Constants.SEEKTHRESHOLD);
            final List<String> mode = new ArrayList<String>();
            mode.add(Constants.AUTO);
            mode.add(Constants.AC);
            mode.add(Constants.HEATER);
            mode.add(Constants.OFF);
            adapter = new ArrayAdapter<String>(getActivity(), android.R.layout.simple_spinner_dropdown_item, mode);
            spinnerData.setAdapter(adapter);
        } catch (Exception e) {
            Logger.d(TAG, "Exception in displayData : " + e);
        }
    }


    private void initialization(View view) {
        thermostatList = new ArrayList<String>();
        apply = (Button) view.findViewById(R.id.buttonApply);
        desiredTempTextView = (TextView) view.findViewById(R.id.textViewDesiredTemp);
        thresholdValue = (TextView) view.findViewById(R.id.textViewThreshold);
        currentTempTextView = (TextView) view.findViewById(R.id.textViewCurrentTemp);
        textviewOperatingState = (TextView) view.findViewById(R.id.textViewOperatingState);
        spinnerData = (Spinner) view.findViewById(R.id.spinnerTempModeList);
        seekBar = (SeekBar) view.findViewById(R.id.seekBar);
        seekBarThreshold = (SeekBar) view.findViewById(R.id.seekBarThreshold);
        imageView = (ImageView) view.findViewById(R.id.imageViewOperatingState);
        txtLastUpdated = (TextView) view.findViewById(R.id.txtLastUpdated);
        dialog = new ProgressDialog(getActivity());
    }

    public void GetThingNameSharedPreference() {
        try {
            SharedPreferences sharedpreferences = getActivity().getSharedPreferences(Constants.THING,
                    Context.MODE_PRIVATE);
            thing = sharedpreferences.getString(Constants.THING_KEY, "");
        } catch (Exception e) {
            Logger.d(TAG, "Exception in GetThingNameSharedPreference : " + e);
        }
    }

    public void startTimer() {
        repeatRefresh = new Timer();
        task = new TimerTask() {
            @Override
            public void run() {
                if (getActivity() == null)
                    return;
                getActivity().runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        new ShadowResults(getActivity(), ThermostatFragment.this, SensorThermostatDeviceActivity.toolbarName, deviceName, thing, accessKey, secreteKey);
                    }
                });
            }
        };
        repeatRefresh.schedule(task, Constants.THREAD_TIME);
    }

    public void destroyTimer() {
        if (repeatRefresh != null) {
            repeatRefresh.cancel();
            repeatRefresh = null;
        }
    }

    @Override
    public void setThermoData(String dataRecieved, String sensorResultString) {
        Log.d(TAG, dataRecieved);
        if (isAdded()) {
            updateThermoPollingData(dataRecieved, sensorResultString);
        } else {
            startTimer();
        }
    }

    //Polling Update
    public void updateThermoPollingData(String result, String sensorResultString) {
        if (result != null) {
            try {
                JSONObject reportedThermo = new JSONObject(result);
                operatingMode = reportedThermo.getString(Constants.THERMO_OPR_STATE);
                temp = reportedThermo.getInt(Constants.THERMO_CUR_STATE);
                desiredTemp = reportedThermo.getInt(Constants.THERMO_DESIRED);
                threshold = reportedThermo.getInt(Constants.THERMO_THRESHOLD);
                mOperatingMode = reportedThermo.getString(Constants.THERMO_MODE);
                lastUpdated = sharedpreferences.getString(Constants.LAST_UPDATED, "");

                String thermoDelta = Util.getThermoDeltaValue(sensorResultString, SensorThermostatDeviceActivity.toolbarName, Constants.THERMOSTAT, deviceName);
                String desiredThermo = Util.getThermoDesiredValue(sensorResultString, SensorThermostatDeviceActivity.toolbarName, deviceName);

                Logger.d(TAG, "thermoDelta in updateThermoPollingData: " + thermoDelta);
                Logger.d(TAG, "desiredThermo in updateThermoPollingData: " + desiredThermo);
                Logger.d(TAG, "result updateThermoPollingData " + result);

                if (thermoDelta != null) {
                    new Handler(Looper.getMainLooper()).post(new Runnable() {
                        public void run() {
                            apply.setClickable(false);
                            UpdateApplyBtnUI(false);
                            txtLastUpdated.setText(Constants.LAST_UPDATED_AT + " " + lastUpdated);
                            apply.setText(R.string.APPLYING);
                        }
                    });
                } else {
                    if (desiredThermo != null) {
                        ThermoUpdateModel reportedmodel = new ThermoUpdateModel(mOperatingMode, desiredTemp, threshold);
                        JSONObject desiredThermoObj = new JSONObject(desiredThermo);
                        int desiredTemp = desiredThermoObj.getInt(Constants.THERMO_DESIRED);
                        int threshold = desiredThermoObj.getInt(Constants.THERMO_THRESHOLD);
                        String mOperatingMode = desiredThermoObj.getString(Constants.THERMO_MODE);
                        ThermoUpdateModel desiredmodel = new ThermoUpdateModel(mOperatingMode, desiredTemp, threshold);
                        if (reportedmodel.equals(desiredmodel)) {
                            Logger.d(TAG, "both equal in update");
                            new Handler(Looper.getMainLooper()).post(new Runnable() {
                                public void run() {
                                    updateUIChanges(operatingMode.toUpperCase());
                                    txtLastUpdated.setText(Constants.LAST_UPDATED_AT + " " + lastUpdated);
                                    currentTempTextView.setText(temp + "" + (char) Constants.DEGREEVALUE + Constants.DEGREEUNIT);
                                    apply.setClickable(true);
                                    UpdateApplyBtnUI(true);
                                    apply.setText(R.string.APPLY);
                                }
                            });
                        } else {
                            new Handler(Looper.getMainLooper()).post(new Runnable() {
                                public void run() {
                                    updateUIChanges(operatingMode.toUpperCase());
                                    txtLastUpdated.setText(Constants.LAST_UPDATED_AT + " " + lastUpdated);
                                    currentTempTextView.setText(temp + "" + (char) Constants.DEGREEVALUE + Constants.DEGREEUNIT);
                                    apply.setClickable(false);
                                    UpdateApplyBtnUI(false);
                                    apply.setText(R.string.APPLYING);
                                }
                            });
                        }
                    } else {
                        Logger.d(TAG, "No desired");
                        new Handler(Looper.getMainLooper()).post(new Runnable() {
                            public void run() {
                                updateUIChanges(operatingMode.toUpperCase());
                                txtLastUpdated.setText(Constants.LAST_UPDATED_AT + " " + lastUpdated);
                                currentTempTextView.setText(temp + "" + (char) Constants.DEGREEVALUE + Constants.DEGREEUNIT);
                                apply.setClickable(true);
                                UpdateApplyBtnUI(true);
                                apply.setText(R.string.APPLY);
                            }
                        });
                    }
                }

            } catch (Exception e) {
                Logger.d(TAG, "Exception in updateThermoPollingData : " + e);
                e.printStackTrace();
            }
            startTimer();
        }

    }

    @Override
    public void onAttach(Context context) {
        super.onAttach(context);
        mActivity = getActivity();
    }

    private void updateUIChanges(String operatingVal) {
        try {
            if (operatingVal.equals(Constants.AC_ON)) {
                textviewOperatingState.setText(R.string.AC_ON);
                textviewOperatingState.setTextColor(getResources().getColor(R.color.acOn));
                imageView.setImageDrawable(getResources().getDrawable(R.drawable.acon));
            } else if (operatingVal.equals(Constants.HEATER_ON)) {
                textviewOperatingState.setText(R.string.HEATER_ON);
                textviewOperatingState.setTextColor(getResources().getColor(R.color.heater));
                imageView.setImageDrawable(getResources().getDrawable(R.drawable.heateron));
            } else if (operatingVal.equals(Constants.STANDBY)) {
                textviewOperatingState.setText(R.string.STAND_BY);
                textviewOperatingState.setTextColor(getResources().getColor(R.color.auto));
                imageView.setImageDrawable(getResources().getDrawable(R.drawable.standby));
            } else if (operatingVal.equals(Constants.OFF)) {
                textviewOperatingState.setText(R.string.OFF);
                textviewOperatingState.setTextColor(getResources().getColor(R.color.off));
                imageView.setImageDrawable(getResources().getDrawable(R.drawable.off));
            }
        } catch (Exception e) {
            Logger.d(TAG, "Exception in updateUIChanges : " + e);
        }
    }

    public void UpdateApplyBtnUI(boolean isEnable) {
        try {
            if (isEnable) {
                apply.setBackgroundColor(getResources().getColor(R.color.irisBlue));
            } else {
                apply.setBackgroundColor(getResources().getColor(R.color.off));
            }
        } catch (Exception e) {
            Logger.d(TAG, "Exception in UpdateApplyBtnUI : " + e);
        }
    }

    private void GetKeyFromSharedPreference() {
        try {
            sharedpreferences = getActivity().getSharedPreferences(Constants.KEY,
                    Context.MODE_PRIVATE);
            accessKey = sharedpreferences.getString(Constants.ACCESS_KEY, "");
            secreteKey = sharedpreferences.getString(Constants.SECRET_KEY, "");
            regionEndpoint = sharedpreferences.getString(Constants.REGION, "");
            lastUpdated = sharedpreferences.getString(Constants.LAST_UPDATED, "");
        } catch (Exception e) {
            Logger.d(TAG, "Exception in GetKeyFromSharedPreference : " + e);
        }
    }

    private class GetThermostatData extends AsyncTask<Void, Void, String> {
        @Override
        protected void onPreExecute() {
            super.onPreExecute();
            dialog.setCancelable(false);
            dialog.setMessage(Constants.FETCHING_THERMOSTAT);
            dialog.show();
        }

        @Override
        protected String doInBackground(Void... voids) {
            String data = "";

            try {
                if (sAWSCredentials == null) {
                    sAWSCredentials = new BasicAWSCredentials(accessKey, secreteKey);
                }
                iotDataClient = new AWSIotDataClient(sAWSCredentials);
                GetThingShadowRequest getThingShadowRequest = new GetThingShadowRequest().withThingName(thing);
                iotDataClient.setRegion(Region.getRegion(Regions.fromName(regionEndpoint)));
                GetThingShadowResult result = iotDataClient.getThingShadow(getThingShadowRequest);
                byte[] bytes = new byte[result.getPayload().remaining()];
                result.getPayload().get(bytes);
                String sensorResultString = new String(bytes);

                JSONObject jsonObject = new JSONObject(sensorResultString);
                JSONObject stateObject = jsonObject.getJSONObject(Constants.STATE);
                JSONObject reportedObject = stateObject.getJSONObject(Constants.REPORTED);

                JSONObject currentDynamicValues = reportedObject.getJSONObject(SensorThermostatDeviceActivity.toolbarName);
                JSONObject thermostatValues = currentDynamicValues.getJSONObject(Constants.THERMOSTAT);
                JSONObject thermoValues = thermostatValues.getJSONObject(deviceName);
                data = thermoValues.toString();

            } catch (Exception e) {
                e.printStackTrace();
            }

            return data;
        }

        @Override
        protected void onPostExecute(String result) {
            if (result != null) {
                try {
                    JSONObject reportedThermo = new JSONObject(result);
                    operatingMode = reportedThermo.getString(Constants.THERMO_OPR_STATE);
                    temp = reportedThermo.getInt(Constants.THERMO_CUR_STATE);
                    desiredTemp = reportedThermo.getInt(Constants.THERMO_DESIRED);
                    threshold = reportedThermo.getInt(Constants.THERMO_THRESHOLD);
                    mOperatingMode = reportedThermo.getString(Constants.THERMO_MODE);

                    String thermoDelta = Util.getThermoDeltaValue(completeJson, SensorThermostatDeviceActivity.toolbarName, Constants.THERMOSTAT, deviceName);
                    String desiredThermo = Util.getThermoDesiredValue(completeJson, SensorThermostatDeviceActivity.toolbarName, deviceName);

                    if (thermoDelta != null) {
                        apply.setClickable(false);
                        UpdateApplyBtnUI(false);
                        apply.setText(R.string.APPLYING);
                    } else {
                        if (desiredThermo != null) {
                            ThermoUpdateModel reportedmodel = new ThermoUpdateModel(mOperatingMode, desiredTemp, threshold);
                            try {
                                JSONObject desiredThermoObj = new JSONObject(desiredThermo);
                                int desiredTemp = desiredThermoObj.getInt(Constants.THERMO_DESIRED);
                                int threshold = desiredThermoObj.getInt(Constants.THERMO_THRESHOLD);
                                String mOperatingMode = desiredThermoObj.getString(Constants.THERMO_MODE);
                                ThermoUpdateModel desiredmodel = new ThermoUpdateModel(mOperatingMode, desiredTemp, threshold);
                                if (reportedmodel.equals(desiredmodel)) {
                                    Logger.d(TAG, "both equal in update");
                                    apply.setClickable(true);
                                    UpdateApplyBtnUI(true);
                                    apply.setText(R.string.APPLY);
                                } else {
                                    apply.setClickable(false);
                                    UpdateApplyBtnUI(false);
                                    apply.setText(R.string.APPLYING);
                                }
                            } catch (Exception e) {
                                Logger.d(TAG, "Exception : " + e);
                            }
                        } else {
                            UpdateApplyBtnUI(true);
                            apply.setClickable(true);
                            apply.setText(R.string.APPLY);
                        }

                    }
                    updateUIChanges(operatingMode.toUpperCase());
                    txtLastUpdated.setText(Constants.LAST_UPDATED_AT + " " + lastUpdated);
                    currentTempTextView.setText(temp + "" + (char) Constants.DEGREEVALUE + Constants.DEGREEUNIT);
                    desiredTempTextView.setText(desiredTemp + "" + (char) Constants.DEGREEVALUE + Constants.DEGREEUNIT);
                    thresholdValue.setText(threshold + "");
                    seekBar.setProgress(desiredTemp);
                    seekBarThreshold.setProgress(threshold);

                    try {
                        String operatingmode = mOperatingMode.toUpperCase();
                        if (!operatingmode.equals(null)) {
                            int spinnerPosition = adapter.getPosition(operatingmode);
                            spinnerData.setSelection(spinnerPosition);
                        }

                    } catch (Exception e) {
                        e.printStackTrace();
                    }

                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
            if (dialog != null) {
                dialog.dismiss();
            }

        }
    }


    private class UpdateThermostatTask extends AsyncTask<Void, Void, String> {
        String updateState;

        public UpdateThermostatTask(String state) {
            updateState = state;
        }

        @Override
        protected void onPreExecute() {
            super.onPreExecute();
            dialog.setCancelable(false);
            dialog.setMessage(Constants.UPDATING_THERMOSTAT);
            dialog.show();
        }

        @Override
        protected String doInBackground(Void... voids) {
            String data = "";
            try {
                GetKeyFromSharedPreference();
                if (sAWSCredentials == null) {
                    sAWSCredentials = new BasicAWSCredentials(accessKey, secreteKey);

                }
                iotDataClient = new AWSIotDataClient(sAWSCredentials);
                iotDataClient.setRegion(Region.getRegion(Regions.fromName(regionEndpoint)));
                UpdateThingShadowRequest request = new UpdateThingShadowRequest();
                request.setThingName(thing);

                ByteBuffer payloadBuffer = ByteBuffer.wrap(updateState.getBytes());
                request.setPayload(payloadBuffer);

                UpdateThingShadowResult result = iotDataClient.updateThingShadow(request);

                byte[] bytes = new byte[result.getPayload().remaining()];
                result.getPayload().get(bytes);
                String sensorResultString = new String(bytes);
                completeJson = sensorResultString;

                JSONObject jsonObject = new JSONObject(sensorResultString);
                JSONObject stateObject = jsonObject.getJSONObject(Constants.STATE);
                JSONObject reportedObject = stateObject.getJSONObject(Constants.REPORTED);

                JSONObject currentDynamicValues = reportedObject.getJSONObject(SensorThermostatDeviceActivity.toolbarName);
                JSONObject thermostatValues = currentDynamicValues.getJSONObject(Constants.THERMOSTAT);
                JSONObject thermoValues = thermostatValues.getJSONObject(deviceName);
                data = thermoValues.toString();

            } catch (Exception e) {
                e.printStackTrace();
            }
            return data;
        }

        @Override
        protected void onPostExecute(String result) {
            if (result != null) {
                try {
                    JSONObject reportedThermo = new JSONObject(result);
                    operatingMode = reportedThermo.getString(Constants.THERMO_OPR_STATE);
                    temp = reportedThermo.getInt(Constants.THERMO_CUR_STATE);
                    desiredTemp = reportedThermo.getInt(Constants.THERMO_DESIRED);
                    threshold = reportedThermo.getInt(Constants.THERMO_THRESHOLD);
                    mOperatingMode = reportedThermo.getString(Constants.THERMO_MODE);


                    String thermoDelta = Util.getThermoDeltaValue(completeJson, SensorThermostatDeviceActivity.toolbarName, Constants.THERMOSTAT, deviceName);
                    String desiredThermo = Util.getThermoDesiredValue(completeJson, SensorThermostatDeviceActivity.toolbarName, deviceName);


                    if (thermoDelta != null) {
                        apply.setClickable(false);
                        UpdateApplyBtnUI(false);
                        apply.setText(R.string.APPLYING);
                    } else {
                        if (desiredThermo != null) {
                            ThermoUpdateModel reportedmodel = new ThermoUpdateModel(mOperatingMode, desiredTemp, threshold);
                            try {
                                JSONObject desiredThermoObj = new JSONObject(desiredThermo);
                                int desiredTemp = desiredThermoObj.getInt(Constants.THERMO_DESIRED);
                                int threshold = desiredThermoObj.getInt(Constants.THERMO_THRESHOLD);
                                String mOperatingMode = desiredThermoObj.getString(Constants.THERMO_MODE);
                                ThermoUpdateModel desiredmodel = new ThermoUpdateModel(mOperatingMode, desiredTemp, threshold);
                                if (reportedmodel.equals(desiredmodel)) {
                                    Logger.d(TAG, "desiredThermo : " + desiredThermo);
                                    Logger.d(TAG, "result : " + result);
                                    apply.setClickable(true);
                                    UpdateApplyBtnUI(true);
                                    apply.setText(R.string.APPLY);
                                } else {
                                    apply.setClickable(false);
                                    UpdateApplyBtnUI(false);
                                    apply.setText(R.string.APPLYING);
                                }
                            } catch (Exception e) {
                                Logger.d(TAG, "exception : " + e);
                            }
                        } else {
                            apply.setClickable(true);
                            UpdateApplyBtnUI(true);
                            apply.setText(R.string.APPLY);
                        }

                    }

                    updateUIChanges(operatingMode.toUpperCase());
                    txtLastUpdated.setText(Constants.LAST_UPDATED_AT + " " + lastUpdated);
                    currentTempTextView.setText(temp + "" + (char) Constants.DEGREEVALUE + Constants.DEGREEUNIT);
                    desiredTempTextView.setText(desiredTemp + "" + (char) Constants.DEGREEVALUE + Constants.DEGREEUNIT);
                    thresholdValue.setText(threshold + "");
                    seekBar.setProgress(desiredTemp);
                    seekBarThreshold.setProgress(threshold);

                    try {
                        String operatingmode = mOperatingMode.toUpperCase();
                        if (!operatingmode.equals(null)) {
                            int spinnerPosition = adapter.getPosition(operatingmode);
                            spinnerData.setSelection(spinnerPosition);
                        }

                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
            if (dialog != null) {
                dialog.dismiss();
            }
            startTimer();
        }
    }

}

