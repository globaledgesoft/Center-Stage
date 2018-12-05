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
import android.content.SharedPreferences;
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
import android.util.Log;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.widget.TextView;
import android.widget.Toast;

import com.amazonaws.auth.BasicAWSCredentials;
import com.amazonaws.regions.Region;
import com.amazonaws.regions.Regions;
import com.amazonaws.services.iot.AWSIotClient;
import com.amazonaws.services.iot.model.DescribeEndpointRequest;
import com.amazonaws.services.iot.model.DescribeEndpointResult;
import com.amazonaws.services.iot.model.ListThingsRequest;
import com.amazonaws.services.iot.model.ListThingsResult;
import com.amazonaws.services.iot.model.ThingAttribute;
import com.qualcomm.qti.qca40xx.sensor.Adapters.ThingsListAdapter;
import com.qualcomm.qti.qca40xx.sensor.Service.NotificationService;
import com.qualcomm.qti.qca40xx.sensor.Utils.Constants;
import com.qualcomm.qti.qca40xx.sensor.Utils.Logger;
import com.qualcomm.qti.qca40xx.sensor.Utils.Util;
import com.qualcomm.qti.qca40xx.sensor.R;

import java.util.ArrayList;


/**
 * This activity fetches for Things and displays the list
 */
public class ListThings extends AppCompatActivity {

    private static Regions MY_REGION;
    private RecyclerView recyclerView;
    private RecyclerView.LayoutManager layoutManager;
    private BasicAWSCredentials sAWSCredentials;
    private ArrayList<ThingAttribute> thingsList;
    private ListThingsResult listThingsResult;
    private AWSIotClient client;
    private ProgressDialog dialog;
    private SharedPreferences sharedpreferences;
    private String accessKey, secreteKey, regionEndpoint, customer_endpoint;
    private Toolbar toolbar;
    private TextView thingsListName, emptyDataTextView;
    private Context mContext;
    private CoordinatorLayout coordinatorLayout;
    private ThingsListAdapter adapter;
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
        setContentView(R.layout.activity_list_things);
        initialization();

        //Check for internet connections
        if (Util.isConnectedToInternet(ListThings.this)) {
            GetKeyFromSharedPreference();
            if (sAWSCredentials == null) {
                sAWSCredentials = new BasicAWSCredentials(accessKey, secreteKey);
            }

            try {
                client = new AWSIotClient(sAWSCredentials);
                MY_REGION = Regions.fromName(regionEndpoint);
                client.setRegion(Region.getRegion(MY_REGION));
            } catch (Exception e) {
                e.printStackTrace();
            }

            GetThingListTask task = new GetThingListTask();
            task.execute();
            Intent i = new Intent(ListThings.this, NotificationService.class);
            i.putExtra(Constants.SERVICE_NAME, Constants.LIST_THINGS);
            startService(i);

            IntentFilter filter = new IntentFilter(NotificationService.ACTION);
            LocalBroadcastManager.getInstance(ListThings.this).registerReceiver(DataSyncReceiver, filter);


        } else {
            Toast.makeText(ListThings.this, R.string.toastNoInternetConnection, Toast.LENGTH_LONG).show();
        }


    }

    private void initialization() {
        sharedpreferences = this.getSharedPreferences(Constants.KEY,
                Context.MODE_PRIVATE);

        sharedpreferences = this.getSharedPreferences(Constants.KEY, Context.MODE_PRIVATE);

        toolbar = (Toolbar) findViewById(R.id.bar);
        toolbar.setTitle(R.string.QualcommSensors);
        setSupportActionBar(toolbar);

        coordinatorLayout = (CoordinatorLayout) findViewById(R.id
                .coordinatorLayout);

        thingsList = new ArrayList<ThingAttribute>();
        dialog = new ProgressDialog(this);

        recyclerView = (RecyclerView) findViewById(R.id.recyclerViewSensors);
        thingsListName = (TextView) findViewById(R.id.listText);
        emptyDataTextView = (TextView) findViewById(R.id.emptyDataTextView);
        layoutManager = new LinearLayoutManager(this);
        recyclerView.setLayoutManager(layoutManager);
        recyclerView.setItemAnimator(new DefaultItemAnimator());
    }

    @Override
    protected void onResume() {

        super.onResume();

    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        LocalBroadcastManager.getInstance(ListThings.this).unregisterReceiver(DataSyncReceiver);
    }

    public void GetKeyFromSharedPreference() {

        accessKey = sharedpreferences.getString(Constants.ACCESS_KEY, "");
        secreteKey = sharedpreferences.getString(Constants.SECRET_KEY, "");
        regionEndpoint = sharedpreferences.getString(Constants.REGION, "");
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.activity_drawer_options, menu);
        return true;
    }


    @Override
    public void onBackPressed() {
        super.onBackPressed();
        finish();
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {

            // action with ID action_settings was selected
            case R.id.action_settings:
                Intent intent = new Intent(this, UpdateRegion.class);
                startActivity(intent);
                break;
            default:
                break;
        }

        return true;
    }


    //Fetching Things List
    private class GetThingListTask extends AsyncTask<Void, Void, ListThingsResult> {

        @Override
        protected void onPreExecute() {
            super.onPreExecute();
            dialog.setCancelable(false);
            dialog.setMessage(Constants.FETCHING_THINGS);
            dialog.show();
        }

        @Override
        protected ListThingsResult doInBackground(Void... voids) {
            try {

                ListThingsRequest listThingsRequest = new ListThingsRequest();
                listThingsResult = client.listThings(listThingsRequest);
                if (listThingsResult != null) {

                    for (int i = 0; i < listThingsResult.getThings().size(); i++) {
                        String thingName = listThingsResult.getThings().get(i).getThingTypeName();
                        if (thingName != null) {
                            if (thingName.equals(Constants.THING_FILTER)) {
                                thingsList.add(listThingsResult.getThings().get(i));
                            }
                        }
                    }
                }

            } catch (Exception e) {
                Log.e("E", "getShadowTask", e);
            }
            return listThingsResult;
        }


        @Override
        protected void onPostExecute(ListThingsResult listThingsResult) {
            super.onPostExecute(listThingsResult);
            Log.e("deviceList", thingsList + " " + thingsList.size());
            adapter = new ThingsListAdapter(ListThings.this, thingsList);
            recyclerView.setAdapter(adapter);

            //Hide text if things list is null
            adapter.setOnDataChangeListener(new ThingsListAdapter.OnDataChangeListener() {
                public void onDataChanged(int size) {
                    if (size <= 0) {
                        thingsListName.setVisibility(View.INVISIBLE);
                        emptyDataTextView.setVisibility(View.VISIBLE);
                    }

                }
            });

            if (dialog != null) {
                dialog.dismiss();
                thingsListName.setVisibility(View.VISIBLE);
            }

            CustomerEndPointResult customerEndPointResult = new CustomerEndPointResult();
            customerEndPointResult.execute();
        }
    }

    /**
     * Get the Customer Specific Endpoint from AWS
     */
    class CustomerEndPointResult extends AsyncTask<String, Void, DescribeEndpointResult> {

        private Exception exception;

        @Override
        protected void onPreExecute() {
            super.onPreExecute();
            dialog.setCancelable(false);
            dialog.setMessage(Constants.FETCHING_ENDPOINT);
            dialog.show();
        }

        @Override
        protected DescribeEndpointResult doInBackground(String... urls) {
            try {
                DescribeEndpointRequest describeEndpointRequest = new DescribeEndpointRequest();
                describeEndpointRequest.setRequestCredentials(sAWSCredentials);
                DescribeEndpointResult describeEndpointResult = client.describeEndpoint(describeEndpointRequest);

                return describeEndpointResult;
            } catch (Exception e) {
                this.exception = e;
                Logger.d("EndPoint ", " " + exception.getMessage());
                return null;
            }
        }

        @Override
        protected void onPostExecute(DescribeEndpointResult result) {
            if (result != null) {
                try {
                    Logger.d("EndPoint ", " " + result.getEndpointAddress());
                    SharedPreferences.Editor editor = sharedpreferences.edit();
                    editor.putString(Constants.CUSTOMER_ENDPOINT, result.getEndpointAddress());
                    editor.apply();

                    customer_endpoint = sharedpreferences.getString(Constants.CUSTOMER_ENDPOINT, "");
                    Intent i = new Intent(ListThings.this, NotificationService.class);
                    i.putExtra(Constants.SERVICE_NAME, Constants.LIST_THINGS);
                    startService(i);

                    IntentFilter filter = new IntentFilter(NotificationService.ACTION);
                    LocalBroadcastManager.getInstance(ListThings.this).registerReceiver(DataSyncReceiver, filter);
                } catch (Exception e) {
                    e.printStackTrace();
                }


            } else {
                try {
                    Logger.d("EndPoint ", mContext.getResources().getString(R.string.endpoint_failure));
                } catch (Exception e) {
                    e.printStackTrace();
                }

            }

            if (dialog != null) {
                dialog.dismiss();
            }
        }
    }
}
