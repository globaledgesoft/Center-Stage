/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package com.qualcomm.qti.qca40xx.sensor.Adapters;

import android.app.ProgressDialog;
import android.content.Context;
import android.content.SharedPreferences;
import android.os.AsyncTask;
import android.support.v7.widget.RecyclerView;
import android.support.v7.widget.SwitchCompat;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.SeekBar;
import android.widget.TextView;

import com.amazonaws.auth.BasicAWSCredentials;
import com.amazonaws.regions.Region;
import com.amazonaws.regions.Regions;
import com.amazonaws.services.iotdata.AWSIotDataClient;
import com.amazonaws.services.iotdata.model.UpdateThingShadowRequest;
import com.amazonaws.services.iotdata.model.UpdateThingShadowResult;
import com.qualcomm.qti.qca40xx.sensor.Interfaces.IUpdationSensorData;
import com.qualcomm.qti.qca40xx.sensor.Model.SensorDetails;
import com.qualcomm.qti.qca40xx.sensor.Utils.Constants;
import com.qualcomm.qti.qca40xx.sensor.Utils.Logger;
import com.qualcomm.qti.qca40xx.sensor.R;
import com.qualcomm.qti.qca40xx.sensor.Utils.Util;

import org.json.JSONObject;

import java.nio.ByteBuffer;
import java.util.ArrayList;


/**
 * SensorDetailsAdapter holds the sensor details
 */

public class SensorDetailsAdapter extends RecyclerView.Adapter<SensorDetailsAdapter.MyViewHolder> {

    private String TAG = getClass().getSimpleName();
    private String deviceName;
    private Context context;
    private SharedPreferences sharedpreferences;
    private String accessKey, secreteKey, regionEndpoint;
    private AWSIotDataClient iotDataClient;
    private BasicAWSCredentials sAWSCredentials;
    private String thing;
    private ArrayList<SensorDetails> mSensorDetailsArray = new ArrayList<>();
    private String jsondata = null;
    private IUpdationSensorData IUpdationSensorData;

    public SensorDetailsAdapter(Context context, IUpdationSensorData IUpdationSensorData, String mdeviceName, ArrayList<SensorDetails> sensorDetailsArray, String jsondata) {
        deviceName = mdeviceName;
        this.context = context;
        this.IUpdationSensorData = IUpdationSensorData;
        this.mSensorDetailsArray = sensorDetailsArray;
        this.jsondata = jsondata;
    }

    public void setData(Context context, String mdeviceName, ArrayList<SensorDetails> sensorDetailsArray, String jsondata) {
        deviceName = mdeviceName;
        this.context = context;
        this.mSensorDetailsArray = sensorDetailsArray;
        this.jsondata = jsondata;
    }

    @Override
    public MyViewHolder onCreateViewHolder(ViewGroup parent,
                                           int viewType) {
        View view = LayoutInflater.from(parent.getContext())
                .inflate(R.layout.detail_sensor_activity_layout, parent, false);

        MyViewHolder myViewHolder = new MyViewHolder(view);
        return myViewHolder;
    }

    @Override
    public void onBindViewHolder(final MyViewHolder holder, final int listPosition) {


        TextView textViewName = holder.textViewName;
        TextView textViewValue = holder.textViewSensorValue;
        ImageView imageView = holder.imageViewIcon;
        final SwitchCompat switchCompat = holder.switchCompat;
        final SeekBar seekBar = holder.seekBar;
        final TextView seekbarValue = holder.seekBarValue;
        seekBar.setMax(254);
        final String keyData = mSensorDetailsArray.get(listPosition).getSensorKey();
        String valueData = mSensorDetailsArray.get(listPosition).getSensorValue();
        textViewName.setText(keyData);


        String parentSensorType = mSensorDetailsArray.get(listPosition).getParentType();
        switchCompat.setVisibility(View.GONE);
        seekBar.setVisibility(View.GONE);
        seekbarValue.setVisibility(View.GONE);
        textViewValue.setVisibility(View.VISIBLE);

        if (parentSensorType.equalsIgnoreCase(Constants.TEMPERATURE)) {
            textViewValue.setText(valueData + "" + (char) 0x00B0 + "C");
        } else {
            textViewValue.setText(valueData);
        }

        switch (parentSensorType) {

            case Constants.LIGHT:
                String light_delta = Util.getSensorDeltaValue(jsondata, deviceName, Constants.LIGHT, keyData);
                String desiredValue = Util.getSensorDesiredValue(jsondata, valueData, deviceName, Constants.LIGHT, keyData);
                switchCompat.setVisibility(View.VISIBLE);
                textViewValue.setVisibility(View.GONE);
                imageView.setImageResource(R.drawable.light);
                switchCompat.setEnabled(false);

                Logger.d(TAG, "light value " + valueData);
                Logger.d(TAG, "light_delta  : " + keyData + " :" + light_delta);
                Logger.d(TAG, "desiredValue  : " + keyData + " :" + desiredValue);


                if (light_delta != null) {
                    switchCompat.setEnabled(false);
                    if (light_delta.equals(Constants.ONE)) {
                        switchCompat.setChecked(true);
                    } else {
                        switchCompat.setChecked(false);
                    }
                } else {
                    if (desiredValue != null) {
                        if (valueData.equals(desiredValue)) {
                            switchCompat.setEnabled(true);
                            if (valueData.equals(Constants.ONE)) {
                                switchCompat.setChecked(true);
                            } else {
                                switchCompat.setChecked(false);
                            }
                        } else {
                            switchCompat.setEnabled(false);
                            if (desiredValue.equals(Constants.ONE)) {
                                switchCompat.setChecked(true);
                            } else {
                                switchCompat.setChecked(false);
                            }
                        }
                    } else {
                        switchCompat.setEnabled(true);
                        if (valueData.equals(Constants.ONE)) {
                            switchCompat.setChecked(true);
                        } else {
                            switchCompat.setChecked(false);
                        }
                    }
                }
                switchCompat.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        String toggleValue;
                        IUpdationSensorData.setUpdationTriggerd(true);
                        Logger.d(TAG, "clicked" + " " + switchCompat.isChecked());
                        if (switchCompat.isChecked()) {
                            switchCompat.setChecked(true);
                            toggleValue = Constants.ONE;
                        } else {
                            switchCompat.setChecked(false);
                            toggleValue = Constants.ZERO;
                        }
                        switchCompat.setEnabled(false);
                        updateLightToggleValue(toggleValue, keyData);
                    }
                });

                switchCompat.setOnTouchListener(new View.OnTouchListener() {
                    @Override
                    public boolean onTouch(View v, MotionEvent event) {
                        return event.getActionMasked() == MotionEvent.ACTION_MOVE;
                    }
                });
                break;
            case Constants.DIMMER:
                String dimmer_delta = Util.getSensorDeltaValue(jsondata, deviceName, Constants.DIMMER, keyData);
                String desireDimmerValue = Util.getSensorDesiredValue(jsondata, valueData, deviceName, Constants.DIMMER, keyData);

                Logger.d(TAG, "dimmer_light : " + keyData + " :" + dimmer_delta);
                Logger.d(TAG, "desireDimmerValue  : " + keyData + " :" + desireDimmerValue);

                imageView.setImageResource(R.drawable.dimmer_light);
                textViewValue.setVisibility(View.GONE);
                seekBar.setVisibility(View.VISIBLE);
                seekbarValue.setVisibility(View.VISIBLE);

                int value = Integer.parseInt(valueData);

                if (dimmer_delta != null) {
                    seekBar.setEnabled(false);
                    seekbarValue.setText(dimmer_delta + "");
                    seekBar.setProgress(Integer.parseInt(dimmer_delta));
                } else {
                    if (desireDimmerValue != null) {
                        if (valueData.equals(desireDimmerValue)) {
                            seekBar.setProgress(value);
                            seekbarValue.setText(value + "");
                            seekBar.setEnabled(true);
                        } else {
                            seekBar.setProgress(Integer.parseInt(desireDimmerValue));
                            seekbarValue.setText(desireDimmerValue);
                            seekBar.setEnabled(false);
                        }
                    } else {
                        seekBar.setEnabled(true);
                        seekbarValue.setText(value + "");
                        seekBar.setProgress(value);
                    }

                }

                seekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
                    int progress = 0;

                    @Override
                    public void onProgressChanged(SeekBar seekBar, int i, boolean b) {
                        seekbarValue.setText(i + "");
                        progress = i;
                    }

                    @Override
                    public void onStartTrackingTouch(SeekBar seekBar) {

                    }

                    @Override
                    public void onStopTrackingTouch(SeekBar seekBar) {
                        IUpdationSensorData.setUpdationTriggerd(true);
                        seekBar.setProgress(progress);
                        seekBar.setEnabled(false);
                        seekbarValue.setText(progress + "");
                        updateDimmerData(progress, keyData);

                    }
                });
                break;
            case Constants.COMPASS:
                imageView.setImageResource(R.drawable.compass);
                valueData = valueData.replace("{", "").replace("}", "").replace("\"", "").replace("x:", "").replace("y:", "").replace("z:", "").replace("X:", "").replace("Y:", "").replace("Z:", "");
                textViewValue.setText(valueData + " " + "\u00B5" + "T");
                break;
            case Constants.ACCELEROMETER:
                imageView.setImageResource(R.drawable.accelerometer);
                valueData = valueData.replace("{", "").replace("}", "").replace("\"", "").replace("x:", "").replace("y:", "").replace("z:", "").replace("X:", "").replace("Y:", "").replace("Z:", "");
                String[] accelerometerArray = valueData.split(",");
                String accelerometerData = "";
                String accUnit = " m/s" + "\u00B2" + ",";
                StringBuilder accelerometerBuilder = new StringBuilder();
                for (int i = 0; i < accelerometerArray.length; i++) {
                    accelerometerBuilder.append(accelerometerArray[i]);
                    accelerometerBuilder.append(accUnit);

                }
                Logger.d(TAG, "e value " + accUnit);
                accelerometerData = accelerometerBuilder.toString();
                textViewValue.setText(accelerometerData.substring(0, accelerometerData.length() - 1));

                break;
            case Constants.TEMPERATURE:
                Logger.d(TAG, "temperature value " + valueData);
                imageView.setImageResource(R.drawable.temperature);
                textViewValue.setText(valueData + "" + (char) 0x00B0 + "C");
                break;
            case Constants.HUMIDITY:
                switchCompat.setVisibility(View.GONE);
                textViewValue.setVisibility(View.VISIBLE);
                imageView.setImageResource(R.drawable.humidity);
                textViewValue.setText(valueData + " " + "rH");
                break;
            case Constants.AMBIENT:
                imageView.setImageResource(R.drawable.ambient);
                textViewValue.setText(valueData + " " + "lx");
                break;
            case Constants.PRESSURE:
                imageView.setImageResource(R.drawable.pressure);
                textViewValue.setText(valueData + " " + "hPa");
                break;
            case Constants.GYROSCOPE:
                imageView.setImageResource(R.drawable.tilt);
                valueData = valueData.replace("{", "").replace("}", "").replace("\"", "").replace("x:", "").replace("y:", "").replace("z:", "").replace("X:", "").replace("Y:", "").replace("Z:", "");
                String[] gyroArray = valueData.split(",");
                String gyroData = "";
                StringBuilder builder = new StringBuilder();
                for (int i = 0; i < gyroArray.length; i++) {
                    builder.append(gyroArray[i] + " dps,");
                }
                gyroData = builder.toString();
                textViewValue.setText(gyroData.substring(0, gyroData.length() - 1));
                break;
        }
    }

    private void updateLightToggleValue(String sensorToggle, String sensorName) {

        try {
            Logger.d(TAG, "Light Value" + sensorToggle);
            //Update int value for light
            int sensorIntValue = Integer.parseInt(sensorToggle);
            JSONObject itemObject = new JSONObject();
            itemObject.put(sensorName, sensorIntValue);
            JSONObject sensorObject = new JSONObject();
            sensorObject.putOpt(Constants.LIGHT, itemObject);
            JSONObject deviceObject = new JSONObject();
            deviceObject.putOpt(deviceName, sensorObject);
            JSONObject desiredObject = new JSONObject();
            desiredObject.putOpt(Constants.DESIRED, deviceObject);
            JSONObject stateObject = new JSONObject();

            stateObject.putOpt(Constants.STATE, desiredObject);

            Logger.d(TAG, "after update here" + desiredObject);

            updateSensorDetails updateSensorDetails = new updateSensorDetails(stateObject + "");
            updateSensorDetails.execute();
        } catch (Exception e) {
            Logger.d(TAG, "Exception in updateLightToggleValue : " + e);
            e.printStackTrace();
        }
    }

    private void updateDimmerData(int val, String sensorName) {

        try {

            Logger.d(TAG, "dimmer Value" + val);
            //Update int value for dimmer
            JSONObject itemObject = new JSONObject();
            itemObject.put(sensorName, val);
            JSONObject sensorObject = new JSONObject();
            sensorObject.putOpt(Constants.DIMMER, itemObject);
            JSONObject deviceObject = new JSONObject();
            deviceObject.putOpt(deviceName, sensorObject);
            JSONObject desiredObject = new JSONObject();
            desiredObject.putOpt(Constants.DESIRED, deviceObject);
            JSONObject stateObject = new JSONObject();
            stateObject.putOpt(Constants.STATE, desiredObject);
            Logger.d(TAG, "after update here" + desiredObject);

            updateSensorDetails updateSensorDetails = new updateSensorDetails(stateObject + "");
            updateSensorDetails.execute();
        } catch (Exception e) {
            Logger.d(TAG, "Exception in updateDimmerData : " + e);
            e.printStackTrace();
        }
    }


    private void GetKeyFromSharedPreference() {
        try {
            sharedpreferences = context.getSharedPreferences(Constants.KEY,
                    Context.MODE_PRIVATE);
            accessKey = sharedpreferences.getString(Constants.ACCESS_KEY, "");
            secreteKey = sharedpreferences.getString(Constants.SECRET_KEY, "");
            regionEndpoint = sharedpreferences.getString(Constants.REGION, "");
        } catch (Exception e) {
            Logger.d(TAG, "Exception in GetKeyFromSharedPreference : " + e);
        }

    }

    private void GetThingNameSharedPreference() {
        try {
            sharedpreferences = context.getSharedPreferences(Constants.THING,
                    Context.MODE_PRIVATE);
            thing = sharedpreferences.getString(Constants.THING_KEY, "");
        } catch (Exception e) {
            Logger.d(TAG, "Exception in GetThingNameSharedPreference : " + e);
        }
    }

    @Override
    public int getItemCount() {
        return mSensorDetailsArray.size();
    }

    public static class MyViewHolder extends RecyclerView.ViewHolder {

        TextView textViewName, textViewSensorValue, seekBarValue;
        ImageView imageViewIcon;
        SwitchCompat switchCompat;
        SeekBar seekBar;

        public MyViewHolder(View itemView) {
            super(itemView);
            this.textViewName = (TextView) itemView.findViewById(R.id.textViewSensor);
            this.imageViewIcon = (ImageView) itemView.findViewById(R.id.imageViewSensor);
            this.textViewSensorValue = (TextView) itemView.findViewById(R.id.textViewSensorValue);
            this.switchCompat = (SwitchCompat) itemView.findViewById(R.id.toggBtn);
            this.seekBar = (SeekBar) itemView.findViewById(R.id.dimSeekbar);
            this.seekBarValue = (TextView) itemView.findViewById(R.id.seekBarTextValue);
        }
    }

    private class updateSensorDetails extends AsyncTask<Void, Void, String> {

        String updateState;


        public updateSensorDetails(String state) {
            updateState = state;
            ProgressDialog dialog = new ProgressDialog(context);
        }

        @Override
        protected void onPreExecute() {
            super.onPreExecute();
        }


        @Override
        protected String doInBackground(Void... voids) {
            String data = "";
            try {
                GetKeyFromSharedPreference();
                GetThingNameSharedPreference();
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
                String resultString = new String(bytes);
                data = resultString;


            } catch (Exception e) {
                Logger.d(TAG, "Exception in updateSensorDetails : " + e);
                e.printStackTrace();
            }
            return data;
        }

        @Override
        protected void onPostExecute(String result) {
            super.onPreExecute();
            IUpdationSensorData.setUpdationTriggerd(false);
        }
    }

}