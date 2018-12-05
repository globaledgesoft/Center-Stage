/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package com.qualcomm.qti.qca40xx.sensor.Activity;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.graphics.Color;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.support.design.widget.CoordinatorLayout;
import android.support.design.widget.Snackbar;
import android.support.design.widget.TabLayout;
import android.support.v4.content.LocalBroadcastManager;
import android.support.v4.view.ViewPager;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.view.View;
import android.widget.TextView;
import android.widget.Toast;

import com.qualcomm.qti.qca40xx.sensor.Adapters.PagerAdapter;
import com.qualcomm.qti.qca40xx.sensor.Service.NotificationService;
import com.qualcomm.qti.qca40xx.sensor.Utils.Constants;
import com.qualcomm.qti.qca40xx.sensor.Utils.Util;
import com.qualcomm.qti.qca40xx.sensor.R;

import org.json.JSONObject;

import java.util.Iterator;

/**
 * SensorThermostatDeviceActivity is an Activity which has two fragments to display the Sensors and Thermostat
 */
public class SensorThermostatDeviceActivity extends AppCompatActivity {

    public static String toolbarName;
    private String TAG = getClass().getSimpleName();
    private SharedPreferences preferences;
    private SharedPreferences.Editor editor;
    private Toolbar toolbar;
    private String thresholdMessage;
    private CoordinatorLayout coordinatorLayout;
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
        setContentView(R.layout.activity_list_device);
        initialization();

        TabLayout tabLayout = (TabLayout) findViewById(R.id.tabLayout);
        String data = getIntent().getStringExtra(Constants.SENSOR_JSON);
        boolean check = false;
        try {
            JSONObject jsonObject = new JSONObject(data);
            Iterator<String> keys = jsonObject.keys();
            while (keys.hasNext()) {
                String currentDynamicKey = keys.next();
                if (currentDynamicKey.contains(Constants.THERMOSTAT)) {
                    check = true;
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
        //check for thermostat value
        if (check == false) {
            tabLayout.addTab(tabLayout.newTab().setText(R.string.sensor));
        } else {
            tabLayout.addTab(tabLayout.newTab().setText(R.string.sensor));
            tabLayout.addTab(tabLayout.newTab().setText(R.string.thermostat));
            tabLayout.setTabGravity(TabLayout.GRAVITY_FILL);
        }


        final ViewPager viewPager = (ViewPager) findViewById(R.id.pager);
        final PagerAdapter adapter = new PagerAdapter(getSupportFragmentManager(), tabLayout.getTabCount());
        viewPager.setAdapter(adapter);

        viewPager.addOnPageChangeListener(new TabLayout.TabLayoutOnPageChangeListener(tabLayout));
        tabLayout.setOnTabSelectedListener(new TabLayout.OnTabSelectedListener() {
            @Override
            public void onTabSelected(TabLayout.Tab tab) {

                viewPager.setCurrentItem(tab.getPosition());
            }

            @Override
            public void onTabUnselected(TabLayout.Tab tab) {

            }

            @Override
            public void onTabReselected(TabLayout.Tab tab) {

            }
        });
        //Call Inapp notification for threshold breach
        if (Util.isConnectedToInternet(SensorThermostatDeviceActivity.this)) {
            Intent i = new Intent(SensorThermostatDeviceActivity.this, NotificationService.class);
            i.putExtra(Constants.SERVICE_NAME, Constants.LIST_DEVICE);
            startService(i);
            IntentFilter filter = new IntentFilter(NotificationService.ACTION);
            LocalBroadcastManager.getInstance(SensorThermostatDeviceActivity.this).registerReceiver(DataSyncReceiver, filter);
        } else {
            Toast.makeText(SensorThermostatDeviceActivity.this, R.string.toastNoInternetConnection, Toast.LENGTH_LONG).show();
        }
    }

    private void initialization() {
        toolbar = (Toolbar) findViewById(R.id.bar);
        toolbarName = getIntent().getStringExtra(Constants.DEVICE_NAME).toString();
        toolbar.setTitle(toolbarName);
        toolbar.setNavigationIcon(R.drawable.backarrow);
        setSupportActionBar(toolbar);
        toolbar.setNavigationOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                onBackPressed();
            }
        });
        coordinatorLayout = (CoordinatorLayout) findViewById(R.id.coordinatorLayout);
        preferences = PreferenceManager.getDefaultSharedPreferences(this);
        editor = preferences.edit();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        LocalBroadcastManager.getInstance(SensorThermostatDeviceActivity.this).unregisterReceiver(DataSyncReceiver);
    }
}
