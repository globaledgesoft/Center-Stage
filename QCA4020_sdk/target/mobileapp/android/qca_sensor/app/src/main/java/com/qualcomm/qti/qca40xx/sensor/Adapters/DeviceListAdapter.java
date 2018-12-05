/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package com.qualcomm.qti.qca40xx.sensor.Adapters;

import android.app.ProgressDialog;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.AsyncTask;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;
import android.widget.Toast;

import com.amazonaws.auth.BasicAWSCredentials;
import com.amazonaws.regions.Region;
import com.amazonaws.regions.Regions;
import com.amazonaws.services.iotdata.AWSIotDataClient;
import com.amazonaws.services.iotdata.model.GetThingShadowRequest;
import com.amazonaws.services.iotdata.model.GetThingShadowResult;
import com.qualcomm.qti.qca40xx.sensor.Activity.SensorThermostatDeviceActivity;
import com.qualcomm.qti.qca40xx.sensor.Utils.Constants;
import com.qualcomm.qti.qca40xx.sensor.R;
import com.qualcomm.qti.qca40xx.sensor.Utils.Logger;
import com.qualcomm.qti.qca40xx.sensor.Utils.ShadowResults;
import com.qualcomm.qti.qca40xx.sensor.Utils.Util;

import org.json.JSONException;
import org.json.JSONObject;

import java.sql.Timestamp;
import java.util.ArrayList;
import java.util.Date;
import java.util.Iterator;

import static android.content.ContentValues.TAG;


/**
 * DeviceListAdapter is an Adapter class which hold the list of the sensor and thermostat
 */


public class DeviceListAdapter extends RecyclerView.Adapter<DeviceListAdapter.ViewHolder> {


    private ArrayList<String> deviceList;
    private Context context;
    private String deviceName;
    private String jsonData;
    private ProgressDialog dialog;
    private AWSIotDataClient iotDataClient;
    private String accessKey, secreteKey, regionEndpoint;
    private BasicAWSCredentials sAWSCredentials;
    private String thingsName;


    public DeviceListAdapter(Context context, ArrayList<String> deviceList, String jsonData) {
        this.context = context;
        this.deviceList = deviceList;
        this.jsonData = jsonData;
    }

    @Override
    public DeviceListAdapter.ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        // create a new view
        LayoutInflater inflater = LayoutInflater.from(parent.getContext());
        View v = inflater.inflate(R.layout.common_thing_list_layout, parent, false);
        DeviceListAdapter.ViewHolder viewHolder = new DeviceListAdapter.ViewHolder(v);
        return viewHolder;
    }

    @Override
    public void onBindViewHolder(DeviceListAdapter.ViewHolder holder, int position) {

        final String name = deviceList.get(position);
        holder.txtHeader.setText(name);
        holder.setClickListener(new ItemClickListener() {
            @Override
            public void onItemClick(int position, View v) {
                try {
                    deviceName = deviceList.get(position);
                    SharedPreferences sharedpreferences = context.getSharedPreferences(Constants.KEY,
                            Context.MODE_PRIVATE);
                    accessKey = sharedpreferences.getString(Constants.ACCESS_KEY, "");
                    secreteKey = sharedpreferences.getString(Constants.SECRET_KEY, "");
                    regionEndpoint = sharedpreferences.getString(Constants.REGION, "");
                    if (sAWSCredentials == null) {
                        sAWSCredentials = new BasicAWSCredentials(accessKey, secreteKey);

                    }
                    iotDataClient = new AWSIotDataClient(sAWSCredentials);

                    GetDynamicSensorList getDynamicSensorList = new GetDynamicSensorList(deviceName);
                    getDynamicSensorList.execute();
                } catch (Exception e) {
                    Logger.d(TAG, "Exception in onBindViewHolder : " + e);
                }
            }
        });

    }

    @Override
    public int getItemCount() {
        return deviceList.size();
    }

    public interface ItemClickListener {
        public void onItemClick(int position, View v);
    }

    public class ViewHolder extends RecyclerView.ViewHolder implements View.OnClickListener {
        public TextView txtHeader;
        public View layout;
        private DeviceListAdapter.ItemClickListener clickListener;

        public ViewHolder(View v) {
            super(v);
            layout = v;
            txtHeader = (TextView) v.findViewById(R.id.textViewThingName);

            v.setOnClickListener(this);

        }

        @Override
        public void onClick(View view) {
            clickListener.onItemClick(getPosition(), view);
        }

        public void setClickListener(DeviceListAdapter.ItemClickListener itemClickListener) {
            this.clickListener = itemClickListener;
        }
    }

    private class GetDynamicSensorList extends AsyncTask<Void, Void, String> {


        private String mDeviceName;
        String reportedData, sensorResultString;

        public GetDynamicSensorList(String deviceName) {

            mDeviceName = deviceName;
            dialog = new ProgressDialog(context);
        }

        @Override
        protected void onPreExecute() {
            super.onPreExecute();
            try {
                dialog.setCancelable(false);
                dialog.setMessage(Constants.FETCHING_SENSOR);
                dialog.show();
            } catch (Exception e) {
                Logger.d(TAG, "Exception in onPreExecute : " + e);
                e.printStackTrace();
            }
        }

        @Override
        protected String doInBackground(Void... voids) {

            try {
                SharedPreferences sharedpreferences = context.getSharedPreferences(Constants.THING,
                        Context.MODE_PRIVATE);
                thingsName = sharedpreferences.getString(Constants.THING_KEY, "");

                GetThingShadowRequest getThingShadowRequest = new GetThingShadowRequest().withThingName(thingsName);
                iotDataClient.setRegion(Region.getRegion(Regions.fromName(regionEndpoint)));
                GetThingShadowResult result = iotDataClient.getThingShadow(getThingShadowRequest);
                byte[] bytes = new byte[result.getPayload().remaining()];
                result.getPayload().get(bytes);
                sensorResultString = new String(bytes);

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
                JSONObject currentDynamicValues = reportedObject.getJSONObject(deviceName);
                reportedData = currentDynamicValues.toString();


                JSONObject sensorsJson = new JSONObject(reportedData);
                Iterator<String> keys = sensorsJson.keys();

            } catch (Exception e) {
                Logger.d(TAG, "Exception in doInBackground : " + e);
                e.printStackTrace();
            }
            return reportedData;
        }

        @Override
        protected void onPostExecute(String data) {
            super.onPostExecute(data);
            if (data != null) {
                Intent sensorDetail = new Intent(context, SensorThermostatDeviceActivity.class);
                sensorDetail.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                sensorDetail.putExtra(Constants.DEVICE_NAME, mDeviceName);
                sensorDetail.putExtra(Constants.SENSOR_JSON, data);
                sensorDetail.putExtra(Constants.SENSOR_DETAILS, sensorResultString);
                context.startActivity(sensorDetail);
            } else {
                Toast.makeText(context, Constants.CONTENTS_NOT, Toast.LENGTH_LONG).show();
            }

            if (dialog != null) {
                dialog.dismiss();
            }
        }
    }

}
