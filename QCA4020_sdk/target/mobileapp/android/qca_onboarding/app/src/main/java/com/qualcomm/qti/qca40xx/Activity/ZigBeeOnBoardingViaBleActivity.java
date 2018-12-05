/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package com.qualcomm.qti.qca40xx.Activity;

import android.app.ProgressDialog;
import android.bluetooth.BluetoothGattService;
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
import com.qualcomm.qti.qca40xx.Util.ConnectBle;
import com.qualcomm.qti.qca40xx.Util.Constants;
import com.qualcomm.qti.qca40xx.Util.DBHelper;
import com.qualcomm.qti.qca40xx.Util.Logger;
import com.qualcomm.qti.qca40xx.Util.Util;
import com.qualcomm.qti.qca40xx.R;

import java.util.ArrayList;
import java.util.List;

import static com.qualcomm.qti.qca40xx.R.string.toastNoServiceFound;

/**
 * ZigBeeOnBoardingViaBleActivity is an Activity class for showing the user the details of the Qualcomm Board
 * via BLE for onboarding.
 */
public class ZigBeeOnBoardingViaBleActivity extends AppCompatActivity implements View.OnClickListener {

    private static final String TAG = "ZigBeeOnBoardingViaBleActivity";
    private Spinner capabilitySpinner, deviceNameListSpinner;
    private DBHelper dbHelper;
    private byte[] supportMode;
    private Toolbar toolbar;
    private SharedPreferences sp;
    private String mDeviceName, mDeviceAddress, onlineOrOfflineDevice;
    private TextView name, macAddr, nameList;
    private String mode;
    private ProgressDialog dialog;
    private Button onboard, cancel, ok;
    private LinearLayout onBoardLayout, buttonLayout, credentialsLayout;
    private List<String> coordinatorList = null;
    private String boardStatus;
    private String mCallingActivity = "";
    private List<BluetoothGattService> gattServices;
    private String threadStatus;
    private ArrayList<String> modeValue;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_zig_bee_on_boarding_details);
        initialize();

        getBleDataFromSP();
        getZigbeeDetails();
        if (ConnectBle.val == 4) {
            try {
                checkThreadStatus();
            } catch (Exception e) {
                e.printStackTrace();
            }

        } else {
            checkZigBeeStatus();
        }


    }


    private void checkZigBeeStatus() {
        try {
            dialog = new ProgressDialog(ZigBeeOnBoardingViaBleActivity.this);
            dialog.setMessage(Constants.STATUS_BOARD_DIALOG);
            dialog.setCancelable(false);
            dialog.show();
            BleServiceCharecteristic.getInstance().readZigbeeBoardStatus();
            Handler handler3 = new Handler();
            handler3.postDelayed(new Runnable() {
                public void run() {
                    String status = BleServiceCharecteristic.getInstance().readZigbeeBoardStatus();
                    Logger.d(TAG, " Zigbee -> status2 " + status);

                }
            }, 2000);

            Handler handler4 = new Handler();
            handler4.postDelayed(new Runnable() {
                public void run() {
                    String status = BleServiceCharecteristic.getInstance().readZigbeeBoardStatus();
                    Logger.d(TAG, " Zigbee -> status3 " + status);
                    if (status != null) {
                        if (status.equals("1")) {
                            Toast.makeText(ZigBeeOnBoardingViaBleActivity.this, "Zigbee On-Boarding successful!", Toast.LENGTH_SHORT).show();
                            switchingOnboardScreen();
                            dialog.dismiss();
                        } else {
                            dialog.dismiss();
                            getSupportmode();
                        }
                    } else {
                        dialog.dismiss();
                        Toast.makeText(ZigBeeOnBoardingViaBleActivity.this, R.string.toastZigbeeNoData, Toast.LENGTH_SHORT).show();
                        Util.bleDisconnection(ZigBeeOnBoardingViaBleActivity.this, mCallingActivity);
                    }
                }
            }, 4000);
        } catch (Exception e) {
            e.printStackTrace();
            Logger.d(TAG, "Exception in checkZigBeeStatus()= " + e);
            Toast.makeText(ZigBeeOnBoardingViaBleActivity.this, R.string.toastZigbeeNoData, Toast.LENGTH_SHORT).show();
            Util.bleDisconnection(ZigBeeOnBoardingViaBleActivity.this, mCallingActivity);
        }

    }

    private void getSupportmode() {
        try {
            dialog = new ProgressDialog(ZigBeeOnBoardingViaBleActivity.this);
            dialog.setMessage(Constants.ZIGBEE_MODE_DIALOG);
            dialog.setCancelable(false);
            dialog.show();

            Handler handler3 = new Handler();
            handler3.postDelayed(new Runnable() {
                public void run() {
                    try {
                        String statusData = BleServiceCharecteristic.getInstance().readZigbeeSupportMode();
                        Logger.d(TAG, "Support mode : = " + statusData);
                        if (statusData != null) {
                            int mode = Integer.parseInt(statusData);
                            modeValue = Util.checkSupportMode(mode, Constants.ZIGBEE);
                            if (onlineOrOfflineDevice.equals(Constants.OFFLINE_ONBOARD_DEVICE)) {
                                String name = dbHelper.offlineCoordinatorExistOrNot(Constants.YES);
                                ArrayList<String> list = new ArrayList<>();
                                if (!(name.equals("")) && !(name.equals(mDeviceName))) {
                                    list.add(Constants.ENDDEVICE);
                                    ArrayAdapter<String> adapter = new ArrayAdapter<String>(ZigBeeOnBoardingViaBleActivity.this, android.R.layout.simple_spinner_dropdown_item, list);
                                    capabilitySpinner.setAdapter(adapter);
                                    capabilitySpinner.setEnabled(false);
                                } else {
                                    list.add(Constants.COORDINATOR);
                                    ArrayAdapter<String> adapter = new ArrayAdapter<String>(ZigBeeOnBoardingViaBleActivity.this, android.R.layout.simple_spinner_dropdown_item, list);
                                    capabilitySpinner.setAdapter(adapter);
                                    capabilitySpinner.setEnabled(false);
                                }
                            } else {
                                if (modeValue.size() == 1) {
                                    ArrayAdapter<String> adapter = new ArrayAdapter<String>(ZigBeeOnBoardingViaBleActivity.this, android.R.layout.simple_spinner_dropdown_item, modeValue);
                                    capabilitySpinner.setAdapter(adapter);
                                    capabilitySpinner.setEnabled(false);
                                } else {
                                    ArrayAdapter<String> adapter = new ArrayAdapter<String>(ZigBeeOnBoardingViaBleActivity.this, android.R.layout.simple_spinner_dropdown_item, modeValue);
                                    capabilitySpinner.setAdapter(adapter);
                                    capabilitySpinner.setEnabled(true);
                                }
                            }
                            displayData();
                        } else {
                            Toast.makeText(ZigBeeOnBoardingViaBleActivity.this, R.string.toastZigbeeNoData, Toast.LENGTH_SHORT).show();
                            Util.bleDisconnection(ZigBeeOnBoardingViaBleActivity.this, mCallingActivity);
                        }
                        if (dialog.isShowing()) {
                            dialog.dismiss();
                        }
                    } catch (Exception e) {
                        Toast.makeText(ZigBeeOnBoardingViaBleActivity.this, R.string.toastZigbeeNoData, Toast.LENGTH_SHORT).show();
                        Util.bleDisconnection(ZigBeeOnBoardingViaBleActivity.this, mCallingActivity);
                        Logger.d(TAG, "Exception in getSupportmode()= " + e);
                    }

                }
            }, 8000);

            Handler handler2 = new Handler();
            handler2.postDelayed(new Runnable() {
                public void run() {
                    String statusData = BleServiceCharecteristic.getInstance().readZigbeeSupportMode();
                    Logger.d(TAG, "Support mode1 : = " + statusData);
                }
            }, 6000);
            Handler handler = new Handler();
            handler.postDelayed(new Runnable() {
                public void run() {
                    String data = BleServiceCharecteristic.getInstance().readZigbeeSupportMode();
                    Logger.d(TAG, "Support mode : = " + data);
                }
            }, 3000);

        } catch (Exception e) {
            Toast.makeText(ZigBeeOnBoardingViaBleActivity.this, R.string.toastZigbeeNoData, Toast.LENGTH_SHORT).show();
            Util.bleDisconnection(ZigBeeOnBoardingViaBleActivity.this, mCallingActivity);
            Logger.d(TAG, "Exception in getSupportmode()= " + e);
        }
    }

    private void initialize() {


        capabilitySpinner = (Spinner) findViewById(R.id.zigbeeSpinnerCapabilityList);
        deviceNameListSpinner = (Spinner) findViewById(R.id.zigbeeSpinnerDeviceNameList);
        name = (TextView) findViewById(R.id.zigbeeTextViewDeviceName);
        macAddr = (TextView) findViewById(R.id.zigbeeTextViewMacAddress);
        nameList = (TextView) findViewById(R.id.zigbeeTextviewDeviceNameList);
        toolbar = (Toolbar) findViewById(R.id.tool_bar);

        onboard = (Button) findViewById(R.id.zigbeeBtnOnboard);
        cancel = (Button) findViewById(R.id.zigbeeBtnCancel);
        ok = (Button) findViewById(R.id.zigbeeButtonOnBoardOk);

        onboard.setOnClickListener(this);
        cancel.setOnClickListener(this);
        ok.setOnClickListener(this);

        onBoardLayout = (LinearLayout) findViewById(R.id.zigbeeLinearLayoutOnBoarded);
        credentialsLayout = (LinearLayout) findViewById(R.id.zigbeeLinearLayoutCredentials);
        buttonLayout = (LinearLayout) findViewById(R.id.zigbeeLinearLayoutButtons);
        coordinatorList = new ArrayList<>();
        dbHelper = new DBHelper(this);
        sp = ZigBeeOnBoardingViaBleActivity.this.getSharedPreferences(Constants.BLUETOOTH_DEVICE, Context.MODE_PRIVATE);

        toolbar.setTitle(Constants.TITLE_ONBOARDING_DETAILS);
        setSupportActionBar(toolbar);

        getSupportActionBar().setHomeButtonEnabled(true);
        getSupportActionBar().setDisplayHomeAsUpEnabled(true);

        Bundle bundle = this.getIntent().getExtras();
        if (bundle != null) {
            mCallingActivity = bundle.getString(Constants.CALLING_ACTIVITY);
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
    }

    private void checkThreadStatus() {
        try {
            dialog = new ProgressDialog(ZigBeeOnBoardingViaBleActivity.this);
            dialog.setMessage(Constants.DEFAULT_DIALOG);
            dialog.setCancelable(false);
            dialog.show();

            BleServiceCharecteristic.getInstance().readThreadBoardStatus();
            Handler handler2 = new Handler();
            handler2.postDelayed(new Runnable() {
                public void run() {
                    BleServiceCharecteristic.getInstance().readThreadBoardStatus();
                }
            }, 1000);

            Handler handler4 = new Handler();
            handler4.postDelayed(new Runnable() {
                public void run() {
                    BleServiceCharecteristic.getInstance().readThreadBoardStatus();
                }
            }, 2000);


            Handler handler3 = new Handler();
            handler3.postDelayed(new Runnable() {
                public void run() {
                    threadStatus = BleServiceCharecteristic.getInstance().readThreadBoardStatus();
                    if (threadStatus != null) {
                        if (threadStatus.equals(Constants.STATUS_ONBOARDED)) {
                            dialog.dismiss();
                            Util.gotoScreen(ZigBeeOnBoardingViaBleActivity.this, ThreadOnboardingViaBleActivity.class, mCallingActivity);

                        } else {
                            dialog.dismiss();
                            checkZigBeeStatus();
                        }
                    }
                }
            }, 4000);
        } catch (Exception e) {
            e.printStackTrace();
            Logger.d(TAG, "Exception in checkThreadStatus()= " + e);
            Toast.makeText(ZigBeeOnBoardingViaBleActivity.this, R.string.toastZigbeeNoData, Toast.LENGTH_SHORT).show();
            Util.bleDisconnection(ZigBeeOnBoardingViaBleActivity.this, mCallingActivity);
        }

    }

    private void displayData() {
        gattServices = ConnectBle.gattServices;

        if (gattServices != null && gattServices.size() > 0) {
            String boardStatus = BleServiceCharecteristic.getInstance().readZigbeeBoardStatus();
            if (boardStatus != null) {

                if (boardStatus.equals(Constants.STATUS_ONBOARDED)) {
                    switchingOnboardScreen();
                }

            } else {
                Toast.makeText(ZigBeeOnBoardingViaBleActivity.this, R.string.toastNoBoardInfo, Toast.LENGTH_SHORT).show();

                Util.bleDisconnection(ZigBeeOnBoardingViaBleActivity.this, mCallingActivity);
            }
            updateConnectionState();
            capabilitySpinner.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
                @Override
                public void onItemSelected(AdapterView<?> arg0, View arg1,
                                           int arg2, long arg3) {

                    String capabilityItems = capabilitySpinner.getSelectedItem().toString();
                    if (capabilityItems.equals(Constants.ROUTER)) {
                        supportMode = "r".getBytes();
                        deviceNameListSpinner.setEnabled(true);
                        getCordinatorDataFromDB();
                    } else if (capabilityItems.equals(Constants.ENDDEVICE)) {
                        supportMode = "e".getBytes();
                        deviceNameListSpinner.setEnabled(true);
                        getCordinatorDataFromDB();
                    } else if (capabilityItems.equals(Constants.COORDINATOR)) {
                        supportMode = "c".getBytes();
                        nameList.setVisibility(View.GONE);
                        deviceNameListSpinner.setVisibility(View.GONE);
                    }
                }

                @Override
                public void onNothingSelected(AdapterView<?> parent) {
                    nameList.setText("No Coordinator Found");

                }
            });

        } else {
            Toast.makeText(ZigBeeOnBoardingViaBleActivity.this, toastNoServiceFound, Toast.LENGTH_SHORT).show();
            Util.bleDisconnection(ZigBeeOnBoardingViaBleActivity.this, mCallingActivity);
        }
    }

    private void getCordinatorDataFromDB() {
        if (onlineOrOfflineDevice.equals(Constants.OFFLINE_ONBOARD_DEVICE)) {
            String name = dbHelper.offlineCoordinatorExistOrNot(Constants.YES);

            if (name.equals("")) {
                nameList.setVisibility(View.VISIBLE);
                nameList.setText("No Coordinator Found");
                deviceNameListSpinner.setVisibility(View.GONE);
            } else {
                coordinatorList.add(name);
                onboard.setEnabled(true);
                nameList.setVisibility(View.VISIBLE);
                deviceNameListSpinner.setVisibility(View.VISIBLE);
                deviceNameListSpinner.setEnabled(false);
            }
        } else {
            coordinatorList = dbHelper.getDeviceList();
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
            } else {
                nameList.setVisibility(View.VISIBLE);
                nameList.setText("No Coordinator Found");
                deviceNameListSpinner.setVisibility(View.GONE);
            }
        }
        try {
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

        } catch (Exception e) {
            Toast.makeText(ZigBeeOnBoardingViaBleActivity.this, R.string.toastZigbeeNoData, Toast.LENGTH_SHORT).show();
            Util.bleDisconnection(ZigBeeOnBoardingViaBleActivity.this, mCallingActivity);

            Logger.d(TAG, "Exception in getCordinatorDataFromDB()= " + e);
        }

    }

    public void getBleDataFromSP() {

        mDeviceName = sp.getString(Constants.BLE_DEVICE_NAME, null);
        mDeviceAddress = sp.getString(Constants.BLE_DEVICE_ADDRESS, null);
        onlineOrOfflineDevice = sp.getString(Constants.OFFLINE, null);
    }

    private void getZigbeeDetails() {
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


    private void zigbeeOnboarding(byte[] linkKey) {
        try {
            String status = BleServiceCharecteristic.getInstance().readZigbeeBoardStatus();
            boardStatus = status;
            if (boardStatus.equals(Constants.STATUS_NOT_ONBOARDED)) {

                dialog = new ProgressDialog(ZigBeeOnBoardingViaBleActivity.this);
                dialog.setMessage(Constants.ZIGBEE_ONBOARD_DIALOG);
                dialog.setCancelable(false);
                dialog.show();


                Handler handler1 = new Handler();
                handler1.postDelayed(new Runnable() {
                    public void run() {
                        String status = BleServiceCharecteristic.getInstance().readZigbeeBoardStatus();
                        boardStatus = status;
                    }
                }, 10000);
                BleServiceCharecteristic.getInstance().zigbeeOnboardingViaBle(linkKey, supportMode);


                Handler handler2 = new Handler();
                handler2.postDelayed(new Runnable() {
                    public void run() {
                        BleServiceCharecteristic.getInstance().readZigbeeBoardStatus();

                    }
                }, 12000);

                Handler handler3 = new Handler();
                handler3.postDelayed(new Runnable() {
                    public void run() {
                        String status = BleServiceCharecteristic.getInstance().readZigbeeBoardStatus();

                        if (status.equals(Constants.STATUS_NOT_ONBOARDED)) {
                            Toast.makeText(ZigBeeOnBoardingViaBleActivity.this, R.string.toastZigbeeOnboardNotSuccessfull, Toast.LENGTH_SHORT).show();
                        } else
                            switchingOnboardScreen();
                        dialog.dismiss();

                    }
                }, 14000);
            } else if (boardStatus.equals(Constants.STATUS_ONBOARDED)) {
                switchingOnboardScreen();
            }
        } catch (Exception e) {
            Logger.d(TAG, "Exception in zigbeeOnboarding()= " + e);
            Toast.makeText(ZigBeeOnBoardingViaBleActivity.this, R.string.toastZigbeeNoData, Toast.LENGTH_SHORT).show();
            Util.bleDisconnection(ZigBeeOnBoardingViaBleActivity.this, mCallingActivity);
        }
    }


    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if (item.getItemId() == android.R.id.home) {
            Util.checkForBleDeviceToUnpair(mDeviceAddress);
            Util.bleDisconnection(ZigBeeOnBoardingViaBleActivity.this, mCallingActivity);
        }
        return super.onOptionsItemSelected(item);
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.zigbeeBtnOnboard:
                String capability = capabilitySpinner.getSelectedItem().toString();
                if (capability.equals(Constants.COORDINATOR)) {
                    if (onlineOrOfflineDevice.equals(Constants.OFFLINE_ONBOARD_DEVICE)) {
                        String name = dbHelper.offlineCoordinatorExistOrNot(Constants.YES);

                        if (name.equals("")) {
                            String key = Util.generatingRandomKey();
                            Logger.d(TAG, "offline generated key coordinator " + key);
                            dbHelper.insertDataIntoTable(mDeviceName, mDeviceAddress, key, Constants.SUPPORT_MODE_BLE, "C", Constants.YES);
                            byte[] linkKey = key.getBytes();
                            zigbeeOnboarding(linkKey);
                        } else {

                            String key = dbHelper.getMasterKey(mDeviceName);
                            Logger.d(TAG, "offline get db key of coordinator " + key);
                            byte[] linkKey = key.getBytes();
                            zigbeeOnboarding(linkKey);
                        }
                    } else {
                        try {
                            String key = Util.generatingRandomKey();
                            boolean isAvail = dbHelper.dataExistOrNot(mDeviceName);
                            if (isAvail) {
                                Logger.d(TAG, "update key coordinator " + key);
                                dbHelper.updateDataIntoTable(mDeviceName, key, "C");
                                byte[] linkKey = key.getBytes();
                                zigbeeOnboarding(linkKey);

                            } else {
                                Logger.d(TAG, "update key coordinator " + key);
                                dbHelper.insertDataIntoTable(mDeviceName, mDeviceAddress, key, Constants.SUPPORT_MODE_BLE, "C", Constants.NO);
                                byte[] linkKey = key.getBytes();
                                zigbeeOnboarding(linkKey);
                            }

                        } catch (Exception e) {
                            e.printStackTrace();
                        }
                    }
                }
                if (capability.equals(Constants.ROUTER)) {
                    try {
                        if (deviceNameListSpinner.getSelectedItem() != null) {
                            String coordinatorName = deviceNameListSpinner.getSelectedItem().toString();
                            String dName = Util.getDeviceName(coordinatorName);

                            if (!TextUtils.isEmpty(dName)) {
                                String key = dbHelper.getMasterKey(dName);
                                byte[] linkKey = key.getBytes();
                                boolean isAvail = dbHelper.dataExistOrNot(mDeviceName);
                                if (isAvail) {
                                    Logger.d(TAG, "update key router " + key);
                                    dbHelper.updateDataIntoTable(mDeviceName, key, "R");
                                    zigbeeOnboarding(linkKey);
                                } else {
                                    Logger.d(TAG, "update key router " + key);
                                    dbHelper.insertDataIntoTable(mDeviceName, mDeviceAddress, key, Constants.SUPPORT_MODE_BLE, "R", Constants.NO);
                                    zigbeeOnboarding(linkKey);
                                }
                            }

                        } else {
                            Toast.makeText(ZigBeeOnBoardingViaBleActivity.this, R.string.toastNoCoordinator, Toast.LENGTH_SHORT).show();

                            Logger.d(TAG, "No Coordinator selected");
                        }
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                }
                if (capability.equals(Constants.ENDDEVICE)) {
                    if (onlineOrOfflineDevice.equals(Constants.OFFLINE_ONBOARD_DEVICE)) {
                        String name = dbHelper.offlineCoordinatorExistOrNot(Constants.YES);
                        if (name.equals("")) {
                            Toast.makeText(ZigBeeOnBoardingViaBleActivity.this, R.string.toastNoCoordinator, Toast.LENGTH_SHORT).show();
                            Logger.d(TAG, "No Coordinator selected ");
                        } else {
                            String key = dbHelper.getMasterKey(name);
                            Logger.d(TAG, "get key endDevice " + key);
                            byte[] linkKey = key.getBytes();
                            zigbeeOnboarding(linkKey);
                        }
                    } else {
                        try {
                            if (deviceNameListSpinner.getSelectedItem() != null) {
                                String coordinatorName = deviceNameListSpinner.getSelectedItem().toString();
                                String dName = Util.getDeviceName(coordinatorName);

                                if (!TextUtils.isEmpty(dName)) {
                                    String key = dbHelper.getMasterKey(dName);
                                    Logger.d(TAG, "get key endDevice " + key);
                                    byte[] linkKey = key.getBytes();
                                    zigbeeOnboarding(linkKey);
                                }
                            } else {
                                Toast.makeText(ZigBeeOnBoardingViaBleActivity.this, R.string.toastNoCoordinator, Toast.LENGTH_SHORT).show();
                                Logger.d(TAG, "No Coordinator selected ");
                            }
                        } catch (Exception e) {
                            e.printStackTrace();
                        }
                    }
                }
                break;
            case R.id.zigbeeBtnCancel:
                try {
                    if (ConnectBle.val == Constants.BLE_TRIPLE_MODE_ONBOARDING ||
                            ConnectBle.val == Constants.BLE_ZIGBEE_THREAD_MODE_ONBOARDING) {
                        dialog = new ProgressDialog(ZigBeeOnBoardingViaBleActivity.this);
                        dialog.setMessage(Constants.DEFAULT_DIALOG);
                        dialog.setCancelable(false);
                        dialog.show();
                        BleServiceCharecteristic.getInstance().readZigbeeBoardStatus();
                        Handler handler2 = new Handler();
                        handler2.postDelayed(new Runnable() {
                            public void run() {
                                dialog.dismiss();
                                Util.gotoScreen(ZigBeeOnBoardingViaBleActivity.this, ThreadOnboardingViaBleActivity.class, mCallingActivity);
                            }
                        }, 2000);
                    } else {
                        Util.checkForBleDeviceToUnpair(mDeviceAddress);
                        Util.bleDisconnection(ZigBeeOnBoardingViaBleActivity.this, mCallingActivity);
                    }
                } catch (Exception e) {
                    Toast.makeText(ZigBeeOnBoardingViaBleActivity.this, R.string.toastZigbeeNoData, Toast.LENGTH_SHORT).show();
                    Util.bleDisconnection(ZigBeeOnBoardingViaBleActivity.this, mCallingActivity);
                    Logger.d(TAG, " Exception in cancelButtonClick() -> " + e);
                }
                break;
            case R.id.zigbeeButtonOnBoardOk:
                Util.checkForBleDeviceToUnpair(mDeviceAddress);
                Util.bleDisconnection(ZigBeeOnBoardingViaBleActivity.this, mCallingActivity);
                finish();
                break;
        }
    }

    @Override
    public void onBackPressed() {
        super.onBackPressed();
        Util.checkForBleDeviceToUnpair(mDeviceAddress);
        Util.bleDisconnection(ZigBeeOnBoardingViaBleActivity.this, mCallingActivity);
        finish();
    }

    private void updateConnectionState() {
        if (ConnectBle.val == Constants.BLE_TRIPLE_MODE_ONBOARDING || ConnectBle.val == Constants.BLE_ZIGBEE_THREAD_MODE_ONBOARDING) {
            name.setText(":  " + mDeviceName);
            macAddr.setText(":  " + mDeviceAddress);
            cancel.setText("SKIP");
        } else {
            name.setText(":  " + mDeviceName);
            macAddr.setText(":  " + mDeviceAddress);

        }
    }


}
