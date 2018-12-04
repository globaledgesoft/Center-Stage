/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package com.qualcomm.qti.qca40xx.Activity;

import android.app.Activity;
import android.app.ProgressDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.os.Handler;
import android.support.design.widget.CoordinatorLayout;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.support.v7.widget.Toolbar;
import android.view.MenuItem;
import android.view.View;
import android.widget.TextView;

import com.qualcomm.qti.qca40xx.Adapter.SubDeviceListAdapter;
import com.qualcomm.qti.qca40xx.R;
import com.qualcomm.qti.qca40xx.Service.BLEService;
import com.qualcomm.qti.qca40xx.Util.BleServiceCharecteristic;
import com.qualcomm.qti.qca40xx.Util.Constants;
import com.qualcomm.qti.qca40xx.Util.Logger;

import com.qualcomm.qti.qca40xx.Util.MainApplication;
import com.qualcomm.qti.qca40xx.Util.Util;


import java.util.ArrayList;
import java.util.List;
import java.util.Timer;
import java.util.TimerTask;

import static android.view.View.GONE;

/**
 * OfflineDeviceListActivity is an Activity class for to show the
 * List of SubDevices attached to main devices.
 */
public class OfflineDeviceListActivity extends AppCompatActivity  {

    private String TAG = getClass().getSimpleName();
    private boolean mScanning;
    private RecyclerView recyclerView;
    private TextView emptyLV;
    private ProgressDialog dialog;
    private Toolbar toolbar;
    private SubDeviceListAdapter adapter;
    private List<String> subDeviceList;
    private String mainDeviceName;
    public static Timer listTimer;
    private MainApplication application;
    private BroadcastReceiver notificationReceiver;
    private CoordinatorLayout coordinatorLayout;
    private Activity mActivity;
    private int deviceListpolling=0;
    private String mDeviceName;
    private String mDeviceAddress;
    private SharedPreferences sp;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_subdevicelist);
        mActivity=OfflineDeviceListActivity.this;
        initialize();
        getBleDataFromSP();
    }


    public void initialize() {

        Intent intent = getIntent();
        mainDeviceName = intent.getStringExtra(Constants.MAIN_DEVICE_NAME);
        toolbar = (Toolbar) findViewById(R.id.tool_bar);
        toolbar.setTitle(mainDeviceName);
        setSupportActionBar(toolbar);
        getSupportActionBar().setHomeButtonEnabled(true);
        getSupportActionBar().setDisplayHomeAsUpEnabled(true);
        emptyLV = (TextView) findViewById(R.id.emptyDataTextView);
        recyclerView = (RecyclerView) findViewById(R.id.device_list);
        recyclerView.setHasFixedSize(true);
        RecyclerView.LayoutManager layoutManager = new LinearLayoutManager(OfflineDeviceListActivity.this);
        recyclerView.setLayoutManager(layoutManager);

        subDeviceList=new ArrayList<String>();

        coordinatorLayout = (CoordinatorLayout)findViewById(R.id.devicelist_coordinatorLayout);
        application = (MainApplication) getApplicationContext();
        sp = OfflineDeviceListActivity.this.getSharedPreferences(Constants.BLUETOOTH_DEVICE, Context.MODE_PRIVATE);

    }
    private void getBleDataFromSP() {
        try {
            mDeviceName = sp.getString(Constants.BLE_DEVICE_NAME, null);
            mDeviceAddress = sp.getString(Constants.BLE_DEVICE_ADDRESS, null);
        } catch (Exception e) {
            Logger.d(TAG, " Exception in getBleDataFromSP() -> " + e);
        }
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if (item.getItemId() == android.R.id.home) {
            Util.checkForBleDeviceToUnpair(mDeviceAddress);
            Util.bleDisconnection(OfflineDeviceListActivity.this, Constants.MANUAL_SCANNING);
        }
        return super.onOptionsItemSelected(item);
    }

    @Override
    public void onBackPressed() {
        Util.checkForBleDeviceToUnpair(mDeviceAddress);
        Util.bleDisconnection(OfflineDeviceListActivity.this, Constants.MANUAL_SCANNING);
    }

    @Override
    protected void onResume() {
        super.onResume();
        deviceListpolling=0;
        dialog = new ProgressDialog(OfflineDeviceListActivity.this);
        dialog.setMessage(Constants.FETCHING_DEVICE_LIST);
        dialog.setCancelable(false);
        dialog.show();
        final IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(BLEService.ACTION_DATA_AVAILABLE);
        notificationReceiver = application.breachNotification(coordinatorLayout);
        registerReceiver(notificationReceiver, intentFilter);
        pollingData();

    }

    @Override
    protected void onStop() {
        super.onStop();
        if (dialog != null) {
            dialog.dismiss();
            dialog = null;
        }
    }


    private void pollingData() {
            listTimer = new Timer();
            listTimer.schedule(new TimerTask() {
                @Override
                public void run() {
                    runOnUiThread(new Runnable() {
                        public void run() {
                            getDevicePollingData();
                        }
                    });


                }
            }, 0, 5000);

    }


    private void getDevicePollingData() {

        Handler handler1 = new Handler();
        handler1.postDelayed(new Runnable() {
            public void run() {
                String status = BleServiceCharecteristic.getInstance().readOfflineData();

            }
        }, 3000);
        Handler handler3 = new Handler();
        handler3.postDelayed(new Runnable() {
            public void run() {
                String list = BleServiceCharecteristic.getInstance().readOfflineData();
                Logger.d(TAG,"SubDeviceList polling : "+list);
                    if (!list.isEmpty() && list!=null) {
                        emptyLV.setVisibility(GONE);
                        recyclerView.setVisibility(View.VISIBLE);
                        subDeviceList=new ArrayList<>();
                        subDeviceList = Util.getSubDeviceList(list);
                        if (subDeviceList.size()>0) {
                            adapter = new SubDeviceListAdapter(OfflineDeviceListActivity.this, subDeviceList, mainDeviceName);
                            recyclerView.setAdapter(adapter);
                            deviceListpolling=0;
                            if (dialog!=null) {
                                dialog.dismiss();
                            }
                        }
                    } else {
                        deviceListpolling =deviceListpolling+1;
                        if (deviceListpolling > Constants.POLLING_FAIL_COUNT) {
                            recyclerView.setVisibility(GONE);
                            emptyLV.setVisibility(View.VISIBLE);
                            if (dialog != null) {
                                dialog.dismiss();
                            }
                        }
                    }
            }
        }, 5000);
        Handler handler2 = new Handler();
        handler2.postDelayed(new Runnable() {
            public void run() {
                BleServiceCharecteristic.getInstance().writeOfflineData(Util.RequestDeviceList(mainDeviceName));
            }
        }, 2000);
    }

    public static void destroyTimer() {
        if (listTimer != null) {
            listTimer.cancel();
            listTimer = null;
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        destroyTimer();
        unregisterReceiver(notificationReceiver);

    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        destroyTimer();
    }

}