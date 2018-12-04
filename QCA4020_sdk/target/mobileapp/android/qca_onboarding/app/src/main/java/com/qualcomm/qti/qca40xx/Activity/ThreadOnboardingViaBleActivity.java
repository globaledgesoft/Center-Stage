/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package com.qualcomm.qti.qca40xx.Activity;

import android.app.ProgressDialog;
import android.content.Context;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.os.Handler;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.text.TextUtils;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import com.qualcomm.qti.qca40xx.Util.BleServiceCharecteristic;
import com.qualcomm.qti.qca40xx.Util.Constants;
import com.qualcomm.qti.qca40xx.Util.DBHelper;
import com.qualcomm.qti.qca40xx.Util.Logger;
import com.qualcomm.qti.qca40xx.Util.Util;
import com.qualcomm.qti.qca40xx.R;

import java.util.ArrayList;
import java.util.List;

public class ThreadOnboardingViaBleActivity extends AppCompatActivity implements View.OnClickListener {

    private static final String TAG = "ThreadOnBoardingDetailsActivity";
    private Spinner capabilitySpinner, deviceNameListSpinner;
    private DBHelper dbHelper;
    private byte[] supportMode;
    private Toolbar toolbar;
    private SharedPreferences sp;
    private String mDeviceName;
    private String mDeviceAddress;
    private TextView name, macAddr, nameList;
    private ProgressDialog dialog;
    private Button onboard, cancel, ok;
    private LinearLayout onBoardLayout, buttonLayout, credentialsLayout;
    private List<String> coordinatorList = null;
    private String boardStatus;
    private String mCallingActivity = "";
    private ArrayList<String> modeValue;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_thread_onboarding_via_ble);
        initialize();
        getBleDataFromSP();
        getThreadDetails();
        checkThreadStatus();
    }


    private void checkThreadStatus() {
        try {
            dialog = new ProgressDialog(ThreadOnboardingViaBleActivity.this);
            dialog.setMessage(Constants.STATUS_BOARD_DIALOG);
            dialog.setCancelable(false);
            dialog.show();
            String status = BleServiceCharecteristic.getInstance().readThreadBoardStatus();

            Handler handler3 = new Handler();
            handler3.postDelayed(new Runnable() {
                public void run() {
                    String status = BleServiceCharecteristic.getInstance().readThreadBoardStatus();
                    Logger.d(TAG, " Zigbee -> status2 " + status);

                }
            }, 2000);

            Logger.d(TAG, "Thread -> status1 " + status);
            Handler handler4 = new Handler();
            handler4.postDelayed(new Runnable() {
                public void run() {
                    String status = BleServiceCharecteristic.getInstance().readThreadBoardStatus();
                    Logger.d(TAG, " Thread -> status2 " + status);
                    if (status != null) {
                        if (status.equals("1")) {
                            Toast.makeText(ThreadOnboardingViaBleActivity.this, "Thread On-Boarding successful!", Toast.LENGTH_SHORT).show();
                            switchingOnboardScreen();
                            dialog.dismiss();
                        } else {
                            dialog.dismiss();
                            getSupportmode();
                        }
                    } else {
                        dialog.dismiss();
                        Toast.makeText(ThreadOnboardingViaBleActivity.this, R.string.toastThreadNoData, Toast.LENGTH_SHORT).show();
                        Util.bleDisconnection(ThreadOnboardingViaBleActivity.this, mCallingActivity);

                    }
                }
            }, 4000);
        } catch (Exception e) {
            Logger.d(TAG, "Exception in checkThreadStatus() = " + e);
        }
    }

    private void getSupportmode() {
        try {
            dialog = new ProgressDialog(ThreadOnboardingViaBleActivity.this);
            dialog.setMessage(Constants.THREAD_MODE_DIALOG);
            dialog.setCancelable(false);
            dialog.show();

            Handler handler3 = new Handler();
            handler3.postDelayed(new Runnable() {
                public void run() {
                    String statusData = BleServiceCharecteristic.getInstance().readThreadSupportMode();
                    Logger.d(TAG, "Support mode : = " + statusData);
                    if (statusData != null) {
                        int mode = Integer.parseInt(statusData);
                        modeValue = Util.checkSupportMode(mode, Constants.THREAD);
                        if (modeValue.size() == 1) {
                            ArrayAdapter<String> adapter = new ArrayAdapter<String>(ThreadOnboardingViaBleActivity.this, android.R.layout.simple_spinner_dropdown_item, modeValue);
                            capabilitySpinner.setAdapter(adapter);
                            capabilitySpinner.setEnabled(false);
                        } else {
                            ArrayAdapter<String> adapter = new ArrayAdapter<String>(ThreadOnboardingViaBleActivity.this, android.R.layout.simple_spinner_dropdown_item, modeValue);
                            capabilitySpinner.setAdapter(adapter);
                            capabilitySpinner.setEnabled(true);
                        }

                    } else {
                        Toast.makeText(ThreadOnboardingViaBleActivity.this, R.string.toastThreadNoData, Toast.LENGTH_SHORT).show();
                        Util.bleDisconnection(ThreadOnboardingViaBleActivity.this, mCallingActivity);
                    }
                    if (dialog.isShowing()) {
                        dialog.dismiss();
                    }

                }
            }, 8000);

            Handler handler2 = new Handler();
            handler2.postDelayed(new Runnable() {
                public void run() {
                    String statusData = BleServiceCharecteristic.getInstance().readThreadSupportMode();
                    Logger.d(TAG, "Support mode1 : = " + statusData);
                }
            }, 6000);
            Handler handler = new Handler();
            handler.postDelayed(new Runnable() {
                public void run() {
                    String data = BleServiceCharecteristic.getInstance().readThreadSupportMode();
                    Logger.d(TAG, "Support mode : = " + data);
                }
            }, 3000);

        } catch (Exception e) {
            Logger.d(TAG, "Exception in getSupportMode()= " + e);
        }
    }

    public void initialize() {

        Bundle bundle = this.getIntent().getExtras();
        if (bundle != null) {
            mCallingActivity = bundle.getString(Constants.CALLING_ACTIVITY);
        }

        capabilitySpinner = (Spinner) findViewById(R.id.threadSpinnerCapabilityList);
        deviceNameListSpinner = (Spinner) findViewById(R.id.threadSpinnerDeviceNameList);
        name = (TextView) findViewById(R.id.threadTextViewDeviceName);
        macAddr = (TextView) findViewById(R.id.threadTextViewMacAddress);
        nameList = (TextView) findViewById(R.id.threadTextviewDeviceNameList);
        toolbar = (Toolbar) findViewById(R.id.tool_bar);

        onboard = (Button) findViewById(R.id.threadBtnOnboard);
        cancel = (Button) findViewById(R.id.threadBtnCancel);
        ok = (Button) findViewById(R.id.threadButtonOnBoardOk);

        onboard.setOnClickListener(this);
        cancel.setOnClickListener(this);
        ok.setOnClickListener(this);

        onBoardLayout = (LinearLayout) findViewById(R.id.threadLinearLayoutOnBoarded);
        credentialsLayout = (LinearLayout) findViewById(R.id.threadLinearLayoutCredentials);
        buttonLayout = (LinearLayout) findViewById(R.id.threadLinearLayoutButtons);
        coordinatorList = new ArrayList<>();
        dbHelper = new DBHelper(this);
        sp = ThreadOnboardingViaBleActivity.this.getSharedPreferences(Constants.BLUETOOTH_DEVICE, Context.MODE_PRIVATE);
        toolbar.setTitle(Constants.TITLE_ONBOARDING_DETAILS);
        setSupportActionBar(toolbar);

        getSupportActionBar().setHomeButtonEnabled(true);
        getSupportActionBar().setDisplayHomeAsUpEnabled(true);
    }

    @Override
    protected void onResume() {
        super.onResume();

        capabilitySpinner.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> arg0, View arg1,
                                       int arg2, long arg3) {

                String capabilityItems = capabilitySpinner.getSelectedItem().toString();
                if (capabilityItems.equals(Constants.JOINER_ROUTER)) {
                    supportMode = "r".getBytes();
                    deviceNameListSpinner.setEnabled(true);
                    getCordinatorDataFromDB();
                } else if (capabilityItems.equals(Constants.JOINER)) {
                    supportMode = "j".getBytes();
                    deviceNameListSpinner.setEnabled(true);
                    getCordinatorDataFromDB();
                } else if (capabilityItems.equals(Constants.BORDER_ROUTER)) {
                    supportMode = "b".getBytes();
                    nameList.setVisibility(View.GONE);
                    deviceNameListSpinner.setVisibility(View.GONE);
                }
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {

            }


        });

    }

    private void getCordinatorDataFromDB() {
        try {
            coordinatorList = dbHelper.getThreadDeviceList();
            if (coordinatorList.size() > 0) {
                if (coordinatorList.size() == 1) {
                    onboard.setEnabled(true);
                    nameList.setVisibility(View.VISIBLE);
                    deviceNameListSpinner.setVisibility(View.VISIBLE);
                    deviceNameListSpinner.setEnabled(false);
                } else {
                    onboard.setEnabled(true);
                    nameList.setVisibility(View.VISIBLE);
                    deviceNameListSpinner.setVisibility(View.VISIBLE);
                }

                ArrayAdapter<String> arrayAdapter = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_dropdown_item, coordinatorList);
                deviceNameListSpinner.setAdapter(arrayAdapter);
                deviceNameListSpinner.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
                    @Override
                    public void onItemSelected(AdapterView<?> adapterView, View view, int position, long l) {
                        String selectedItem = adapterView.getItemAtPosition(position).toString();
                    }

                    @Override
                    public void onNothingSelected(AdapterView<?> adapterView) {

                    }
                });
            } else {
                nameList.setVisibility(View.VISIBLE);
                nameList.setText("No Coordinator Found");
                deviceNameListSpinner.setVisibility(View.GONE);
            }
        } catch (Exception e) {
            Logger.d(TAG, "Exception in getCordinatorDataFromDB()= " + e);
        }
    }

    public void getBleDataFromSP() {

        mDeviceName = sp.getString(Constants.BLE_DEVICE_NAME, null);
        mDeviceAddress = sp.getString(Constants.BLE_DEVICE_ADDRESS, null);
    }

    private void getThreadDetails() {
        name.setText(":  " + mDeviceName);
        macAddr.setText(":  " + mDeviceAddress);
    }

    public void switchingOnboardScreen() {
        name.setText(": " + mDeviceName);
        macAddr.setText(": " + mDeviceAddress);
        onBoardLayout.setVisibility(View.VISIBLE);
        credentialsLayout.setVisibility(View.GONE);
        buttonLayout.setVisibility(View.GONE);
    }


    private void initiateThreadOnboarding(byte[] passPhrase) {
        try {
            String status = BleServiceCharecteristic.getInstance().readThreadBoardStatus();
            boardStatus = status;
            if (boardStatus.equals(Constants.STATUS_NOT_ONBOARDED)) {

                dialog = new ProgressDialog(ThreadOnboardingViaBleActivity.this);
                dialog.setMessage(Constants.THREAD_ONBOARD_DIALOG);
                dialog.setCancelable(false);
                dialog.show();


                Handler handler1 = new Handler();
                handler1.postDelayed(new Runnable() {
                    public void run() {
                        String status = BleServiceCharecteristic.getInstance().readThreadBoardStatus();
                        boardStatus = status;
                    }
                }, 10000);
                BleServiceCharecteristic.getInstance().threadOnboardingViaBle(passPhrase, supportMode);


                Handler handler2 = new Handler();
                handler2.postDelayed(new Runnable() {
                    public void run() {
                        String status = BleServiceCharecteristic.getInstance().readThreadBoardStatus();

                    }
                }, 12000);

                Handler handler3 = new Handler();
                handler3.postDelayed(new Runnable() {
                    public void run() {
                        String status = BleServiceCharecteristic.getInstance().readThreadBoardStatus();
                        if (status != null) {
                            if (status.equals(Constants.STATUS_NOT_ONBOARDED)) {
                                Toast.makeText(ThreadOnboardingViaBleActivity.this, R.string.toastThreadOnboardNotSuccessfull, Toast.LENGTH_SHORT).show();
                            } else
                                switchingOnboardScreen();
                        }
                        dialog.dismiss();

                    }
                }, 14000);
            } else if (boardStatus.equals(Constants.STATUS_ONBOARDED)) {
                switchingOnboardScreen();
            }
        } catch (Exception e) {
            Logger.d(TAG, "Exception in initiateThreadOnboarding() = " + e);
        }
    }


    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if (item.getItemId() == android.R.id.home) {
            // onBackPressed();
            Util.checkForBleDeviceToUnpair(mDeviceAddress);
            Util.bleDisconnection(ThreadOnboardingViaBleActivity.this, mCallingActivity);
        }
        return super.onOptionsItemSelected(item);
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.threadBtnOnboard:
                String capability = capabilitySpinner.getSelectedItem().toString();
                if (capability.equals(Constants.BORDER_ROUTER)) {
                    try {
                        String key = Util.generatingRandomKey();
                        boolean isAvail = dbHelper.threadDataExistOrNot(mDeviceName);
                        if (isAvail) {
                            Logger.d(TAG, "update key coordinator " + key);
                            dbHelper.updateDataIntoThreadTable(mDeviceName, key, "B");
                            byte[] passPhrase = key.getBytes();
                            initiateThreadOnboarding(passPhrase);
                        } else {
                            Logger.d(TAG, "update key coordinator " + key);
                            dbHelper.insertDataIntoThreadTable(mDeviceName, mDeviceAddress, key, Constants.SUPPORT_MODE_BLE, "B");
                            byte[] passPhrase = key.getBytes();
                            initiateThreadOnboarding(passPhrase);
                        }
                    } catch (Exception e) {
                        e.printStackTrace();
                        Logger.d(TAG, "Exception in if (capability.equals(Constants.BORDER_ROUTER)) = " + e);
                    }
                }
                if (capability.equals(Constants.JOINER_ROUTER)) {
                    try {
                        if (deviceNameListSpinner.getSelectedItem() != null) {
                            String coordinatorName = deviceNameListSpinner.getSelectedItem().toString();
                            String dName = Util.getDeviceName(coordinatorName);

                            if (!TextUtils.isEmpty(dName)) {
                                String key = dbHelper.getThreadPassPhrase(dName);
                                byte[] passPhrase = key.getBytes();
                                boolean isAvail = dbHelper.threadDataExistOrNot(mDeviceName);
                                if (isAvail) {
                                    Logger.d(TAG, "update key router " + key);
                                    dbHelper.updateDataIntoThreadTable(mDeviceName, key, "R");
                                    initiateThreadOnboarding(passPhrase);
                                } else {
                                    Logger.d(TAG, "update key router " + key);
                                    dbHelper.insertDataIntoThreadTable(mDeviceName, mDeviceAddress, key, Constants.SUPPORT_MODE_BLE, "R");
                                    initiateThreadOnboarding(passPhrase);
                                }
                            }

                        } else {
                            Toast.makeText(ThreadOnboardingViaBleActivity.this, R.string.toastNoCoordinator, Toast.LENGTH_SHORT).show();
                            Logger.d(TAG, "No Border-Router selected");
                        }
                    } catch (Exception e) {
                        e.printStackTrace();
                        Logger.d(TAG, "Exception in capability.equals(Constants.JOINER_ROUTER) = " + e);
                    }
                }
                if (capability.equals(Constants.JOINER)) {
                    try {
                        if (deviceNameListSpinner.getSelectedItem() != null) {
                            String coordinatorName = deviceNameListSpinner.getSelectedItem().toString();
                            String dName = Util.getDeviceName(coordinatorName);

                            if (!TextUtils.isEmpty(dName)) {
                                String key = dbHelper.getThreadPassPhrase(dName);
                                Logger.d(TAG, "get key joiner " + key);
                                byte[] passPhrase = key.getBytes();
                                initiateThreadOnboarding(passPhrase);
                            }
                        } else {
                            Toast.makeText(ThreadOnboardingViaBleActivity.this, R.string.toastNoBorderRouter, Toast.LENGTH_SHORT).show();
                            Logger.d(TAG, "No Border-Router selected ");
                        }
                    } catch (Exception e) {
                        e.printStackTrace();
                        Logger.d(TAG, "Exception in capability.equals(Constants.JOINER) = " + e);
                    }
                }
                break;
            case R.id.threadBtnCancel:
                Util.checkForBleDeviceToUnpair(mDeviceAddress);
                Util.bleDisconnection(ThreadOnboardingViaBleActivity.this, mCallingActivity);
                break;
            case R.id.threadButtonOnBoardOk:
                Util.checkForBleDeviceToUnpair(mDeviceAddress);
                Util.bleDisconnection(ThreadOnboardingViaBleActivity.this, mCallingActivity);
                break;
        }
    }

    @Override
    public void onBackPressed() {
        super.onBackPressed();

    }


}
