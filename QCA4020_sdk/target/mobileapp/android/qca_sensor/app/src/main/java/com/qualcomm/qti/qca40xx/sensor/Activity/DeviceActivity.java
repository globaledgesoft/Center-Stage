/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package com.qualcomm.qti.qca40xx.sensor.Activity;

import android.app.ProgressDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.Color;
import android.os.AsyncTask;
import android.os.Bundle;
import android.support.design.widget.CoordinatorLayout;
import android.support.design.widget.Snackbar;
import android.support.v4.content.LocalBroadcastManager;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.DefaultItemAnimator;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.support.v7.widget.Toolbar;
import android.view.View;
import android.widget.TextView;
import android.widget.Toast;

import com.qualcomm.qti.qca40xx.sensor.Adapters.DeviceListAdapter;
import com.qualcomm.qti.qca40xx.sensor.Service.NotificationService;
import com.qualcomm.qti.qca40xx.sensor.Utils.Constants;
import com.qualcomm.qti.qca40xx.sensor.Utils.Util;
import com.qualcomm.qti.qca40xx.sensor.R;

import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.Iterator;

/**
 * DeviceActivity is an Activity displays the list of the devices under a AWS Thing
 */
public class DeviceActivity extends AppCompatActivity {

    private Toolbar toolbar;
    private ArrayList<String> deviceList;
    private RecyclerView recyclerView;
    private RecyclerView.LayoutManager layoutManager;
    private ProgressDialog dialog;
    private String currentDynamicKey;
    private String jsonData;
    private String thingName;
    private CoordinatorLayout coordinatorLayout;
    private String thresholdMessage;
    /**
     * Receiver to receive the Threshold breach and show In-app Notification
     */
    private BroadcastReceiver DataSyncReceiver = new BroadcastReceiver() {

        @Override
        public void onReceive(Context context, Intent intent) {

            thresholdMessage = intent.getStringExtra(Constants.THRESHOLD_MESSAGE);

            Snackbar snackbar = Snackbar
                    .make(coordinatorLayout, thresholdMessage, Snackbar.LENGTH_LONG);
            snackbar.setActionTextColor(Color.WHITE);
            View snackbarView = snackbar.getView();
            snackbarView.setBackgroundColor(Color.DKGRAY);
            TextView textView = (TextView) snackbarView.findViewById(android.support.design.R.id.snackbar_text);
            textView.setTextColor(Color.WHITE);
            snackbar.setDuration(Constants.SYNC_TIME);
            snackbar.show();

        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_device);
        initialization();

        toolbar.setNavigationOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                onBackPressed();
            }
        });

        if (Util.isConnectedToInternet(DeviceActivity.this)) {

            jsonData = getIntent().getStringExtra(Constants.SENSOR_DETAILS);
            GetDeviceList task = new GetDeviceList(jsonData);
            task.execute();

            Intent i = new Intent(DeviceActivity.this, NotificationService.class);
            i.putExtra(Constants.SERVICE_NAME, Constants.DEVICE_ACTIVITY);
            startService(i);

            IntentFilter filter = new IntentFilter(NotificationService.ACTION);
            LocalBroadcastManager.getInstance(DeviceActivity.this).registerReceiver(DataSyncReceiver, filter);
        } else {
            Toast.makeText(DeviceActivity.this, R.string.toastNoInternetConnection, Toast.LENGTH_LONG).show();
        }
    }

    private void initialization() {
        thingName = getIntent().getStringExtra(Constants.NAME);
        toolbar = (Toolbar) findViewById(R.id.bar);
        toolbar.setTitle(thingName);
        toolbar.setNavigationIcon(R.drawable.backarrow);
        setSupportActionBar(toolbar);
        dialog = new ProgressDialog(this);

        coordinatorLayout = (CoordinatorLayout) findViewById(R.id
                .coordinatorLayout);

        recyclerView = (RecyclerView) findViewById(R.id.recyclerViewDevices);
        layoutManager = new LinearLayoutManager(this);
        recyclerView.setLayoutManager(layoutManager);
        recyclerView.setItemAnimator(new DefaultItemAnimator());
        deviceList = new ArrayList<String>();
    }

    @Override
    protected void onResume() {
        super.onResume();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        LocalBroadcastManager.getInstance(DeviceActivity.this).unregisterReceiver(DataSyncReceiver);
    }

    /**
     * AsyncTask for fetching devices under a Thing
     */
    private class GetDeviceList extends AsyncTask<Void, Void, String> {

        String data;

        public GetDeviceList(String data) {

            this.data = data;
        }

        @Override
        protected void onPreExecute() {
            super.onPreExecute();

            dialog.setCancelable(false);
            dialog.setMessage(Constants.FETCHING_DEVICE_LIST);
            dialog.show();
        }


        @Override
        protected String doInBackground(Void... voids) {

            try {
                JSONObject jsonObject = new JSONObject(data);
                JSONObject stateObject = jsonObject.getJSONObject(Constants.STATE);
                JSONObject reportedObject = stateObject.getJSONObject(Constants.REPORTED);
                Iterator<String> keys = reportedObject.keys();
                while (keys.hasNext()) {
                    currentDynamicKey = keys.next();
                    deviceList.add(currentDynamicKey);
                    JSONObject currentDynamicValue = reportedObject.getJSONObject(currentDynamicKey);
                }

            } catch (JSONException e) {
                e.printStackTrace();
            }
            return currentDynamicKey;
        }

        @Override
        protected void onPostExecute(String currentDynamicKey) {
            super.onPostExecute(currentDynamicKey);
            if (deviceList.size() > 0) {
                DeviceListAdapter adapter = new DeviceListAdapter(DeviceActivity.this, deviceList, jsonData);
                recyclerView.setAdapter(adapter);
            } else {
                Toast.makeText(getApplicationContext(), Constants.THINGS_NOT, Toast.LENGTH_LONG).show();
            }
            if (dialog != null) {
                dialog.dismiss();

            }
        }
    }
}
