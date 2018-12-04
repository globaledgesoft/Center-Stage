/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package com.qualcomm.qti.qca40xx.sensor.Adapters;

import android.app.Activity;
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
import com.amazonaws.services.iot.model.ThingAttribute;
import com.amazonaws.services.iotdata.AWSIotDataClient;
import com.amazonaws.services.iotdata.model.GetThingShadowRequest;
import com.amazonaws.services.iotdata.model.GetThingShadowResult;
import com.qualcomm.qti.qca40xx.sensor.Activity.DeviceActivity;
import com.qualcomm.qti.qca40xx.sensor.Utils.Constants;
import com.qualcomm.qti.qca40xx.sensor.Utils.Logger;
import com.qualcomm.qti.qca40xx.sensor.R;

import org.json.JSONObject;

import java.sql.Timestamp;
import java.util.ArrayList;
import java.util.Date;

/**
 * ThingsListAdapter hold the list of Things
 */

public class ThingsListAdapter extends RecyclerView.Adapter<ThingsListAdapter.ViewHolder> {
    private String TAG = getClass().getSimpleName();
    OnDataChangeListener mOnDataChangeListener;
    private ArrayList<ThingAttribute> thingList;
    private AWSIotDataClient iotDataClient;
    private BasicAWSCredentials sAWSCredentials;
    private ProgressDialog dialog;
    private Context context;
    private SharedPreferences sharedpreferences;
    private String accessKey, secreteKey, regionEndpoint, lastUpdated;

    public ThingsListAdapter(Context context, ArrayList<ThingAttribute> thingList) {
        this.context = context;
        this.thingList = thingList;
    }

    @Override
    public ThingsListAdapter.ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        // create a new view
        LayoutInflater inflater = LayoutInflater.from(parent.getContext());
        View v = inflater.inflate(R.layout.common_thing_list_layout, parent, false);
        ViewHolder viewHolder = new ViewHolder(v);
        return viewHolder;
    }

    private void putThingNameSharedPreference(String name) {
        SharedPreferences pref = context.getSharedPreferences(Constants.THING, 0); // 0 - for private mode
        SharedPreferences.Editor editor = pref.edit();
        editor.putString(Constants.THING_KEY, name);
        editor.apply();
    }

    @Override
    public void onBindViewHolder(ViewHolder holder, final int position) {

        final String name = thingList.get(position).getThingName();

        holder.txtHeader.setText(name);
        holder.setClickListener(new ItemClickListener() {
            @Override
            public void onItemClick(int position, View v) {

                ThingsListAdapter.GetThingDetailsTask detailsTask = new ThingsListAdapter.GetThingDetailsTask(name);
                putThingNameSharedPreference(name);
                detailsTask.execute();
            }
        });
    }

    private void GetKeyFromSharedPreference() {
        sharedpreferences = context.getSharedPreferences(Constants.KEY,
                Context.MODE_PRIVATE);
        accessKey = sharedpreferences.getString(Constants.ACCESS_KEY, "");
        secreteKey = sharedpreferences.getString(Constants.SECRET_KEY, "");
        regionEndpoint = sharedpreferences.getString(Constants.REGION, "");
        lastUpdated = sharedpreferences.getString(Constants.LAST_UPDATED, "");
    }

    public void setOnDataChangeListener(OnDataChangeListener onDataChangeListener) {
        mOnDataChangeListener = onDataChangeListener;
    }

    @Override
    public int getItemCount() {

        if (mOnDataChangeListener != null) {
            mOnDataChangeListener.onDataChanged(thingList.size());
        }

        if (thingList.size() <= 0) {

            ((Activity) context).runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    Toast.makeText(context, Constants.THINGS_NOT_AVAILABLE, Toast.LENGTH_SHORT).show();
                }
            });

        }
        return thingList.size();
    }

    public interface ItemClickListener {
        public void onItemClick(int position, View v);
    }

    //Interface to hide the view in activity if size is zero
    public interface OnDataChangeListener {
        public void onDataChanged(int size);
    }

    public class ViewHolder extends RecyclerView.ViewHolder implements View.OnClickListener {

        public TextView txtHeader;
        public View layout;
        private ItemClickListener clickListener;

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

        public void setClickListener(ThingsListAdapter.ItemClickListener itemClickListener) {
            this.clickListener = itemClickListener;
        }
    }

    private class GetThingDetailsTask extends AsyncTask<Void, Void, String> {
        String thingName;


        public GetThingDetailsTask(String name) {

            thingName = name;
            dialog = new ProgressDialog(context);
        }

        @Override
        protected void onPreExecute() {
            super.onPreExecute();
            try {
                dialog.setCancelable(false);
                dialog.setMessage(Constants.LOADING_THINGS);
                dialog.show();
            } catch (Exception e) {
                e.printStackTrace();
            }
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
                GetThingShadowRequest getThingShadowRequest = new GetThingShadowRequest().withThingName(thingName);
                GetThingShadowResult result = iotDataClient.getThingShadow(getThingShadowRequest);

                byte[] bytes = new byte[result.getPayload().remaining()];
                result.getPayload().get(bytes);
                String sensorResultString = new String(bytes);
                data = sensorResultString;

                JSONObject jsonObject = new JSONObject(data);
                long stateObject = jsonObject.getInt(Constants.TIMESTAMP);

                Timestamp ts = new Timestamp(stateObject * 1000);
                Date date = new Date(ts.getTime());

                SharedPreferences pref = context.getSharedPreferences(Constants.KEY, 0); // 0 - for private mode
                SharedPreferences.Editor editor = pref.edit();
                editor.putString(Constants.LAST_UPDATED, date.toString());
                editor.apply();


            } catch (Exception e) {
                Logger.d(TAG, "error = " + e.toString());

            }
            return data;
        }

        @Override
        protected void onPostExecute(String data) {
            super.onPostExecute(data);
            if (data != null) {
                Intent sensorDetail = new Intent(context, DeviceActivity.class);
                sensorDetail.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                sensorDetail.putExtra(Constants.NAME, thingName);
                sensorDetail.putExtra(Constants.SENSOR_DETAILS, data);
                context.startActivity(sensorDetail);
            }

            if (dialog != null) {
                dialog.dismiss();
            }
        }
    }

}
