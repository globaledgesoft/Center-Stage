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
import android.net.wifi.ScanResult;
import android.net.wifi.WifiConfiguration;
import android.os.Bundle;
import android.os.Handler;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.text.InputType;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import com.qualcomm.qti.qca40xx.Interface.IWiFiDeviceList;
import com.qualcomm.qti.qca40xx.Manager.WiFiScanManager;
import com.qualcomm.qti.qca40xx.Util.BleServiceCharecteristic;
import com.qualcomm.qti.qca40xx.Util.ConnectBle;
import com.qualcomm.qti.qca40xx.Util.Constants;
import com.qualcomm.qti.qca40xx.Util.Logger;
import com.qualcomm.qti.qca40xx.Util.MainApplication;
import com.qualcomm.qti.qca40xx.Util.Util;
import com.qualcomm.qti.qca40xx.R;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;


/**
 * \
 * WifiOnboardingViaBleActivity is an Activity class for Showring the user with the details of
 * the Qualcomm Boards connected. This class provides user for SSID and Password input for onboarding
 * via BLE.
 */
public class WifiOnboardingViaBleActivity extends AppCompatActivity implements IWiFiDeviceList, View.OnClickListener {

    private static final String TAG = "WifiOnboardingViaBleActivity";
    private Toolbar toolbar;
    private String mDeviceName;
    private String mDeviceAddress;
    private TextView name, macAddr, onboardMode;
    private EditText password;
    private Button onboard, cancel, ok;
    private String ssidTxt, passwordTxt;
    private Spinner wifiScanListSpinner;
    private TextView textViewPassword;
    private ArrayList<String> ssidList;
    private List<ScanResult> mWifiBleFilteredList;
    private LinearLayout onBoardLayout, buttonLayout, credentialsLayout;
    private WiFiScanManager wiFiScanManager;
    private ProgressDialog dialog;
    private List<BluetoothGattService> gattServices;
    private SharedPreferences sp;
    private String mCallingActivity = "";
    private Context mContext;
    private CheckBox chk_paswd;
    private String zigbeeStatus, threadStatus, wifiStatus;
    private MainApplication application;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.ble_onboarding_detatils_layout);
        initialize();
    }

    private void initialize() {
        toolbar = (Toolbar) findViewById(R.id.tool_bar);
        toolbar.setTitle(Constants.TITLE_ONBOARDING_DETAILS);
        setSupportActionBar(toolbar);
        // toolbar navigation icon setup
        getSupportActionBar().setHomeButtonEnabled(true);
        getSupportActionBar().setDisplayHomeAsUpEnabled(true);
        mContext = this;
        application = (MainApplication) getApplicationContext();
        name = (TextView) findViewById(R.id.bleTextViewDeviceName);
        macAddr = (TextView) findViewById(R.id.bleTextViewMacAddress);
        onboardMode = (TextView) findViewById(R.id.bleTextViewOnboardingMode);
        wifiScanListSpinner = (Spinner) findViewById(R.id.bleSpinnerWifiScanList);
        textViewPassword = (TextView) findViewById(R.id.bleTextViewPassword);
        password = (EditText) findViewById(R.id.bleEditTextPassword);
        onboard = (Button) findViewById(R.id.bleBtnOnboard);
        cancel = (Button) findViewById(R.id.bleBtnCancel);
        ok = (Button) findViewById(R.id.bleButtonOnBoardOk);
        onBoardLayout = (LinearLayout) findViewById(R.id.bleLinearLayoutOnBoarded);
        credentialsLayout = (LinearLayout) findViewById(R.id.bleLinearLayoutCredentials);
        buttonLayout = (LinearLayout) findViewById(R.id.bleLinearLayoutButtons);
        chk_paswd = (CheckBox) findViewById(R.id.chk_paswd);
        wiFiScanManager = new WiFiScanManager(WifiOnboardingViaBleActivity.this);

        Bundle bundle = this.getIntent().getExtras();
        if (bundle != null) {
            mCallingActivity = bundle.getString(Constants.CALLING_ACTIVITY);
        }
        sp = WifiOnboardingViaBleActivity.this.getSharedPreferences(Constants.BLUETOOTH_DEVICE, Context.MODE_PRIVATE);
        onboard.setOnClickListener(this);
        cancel.setOnClickListener(this);
        ok.setOnClickListener(this);

        chk_paswd.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean isChecked) {
                if (!isChecked) {
                    password.setInputType(129);
                } else {
                    password.setInputType(InputType.TYPE_TEXT_VARIATION_PASSWORD);
                }
            }
        });
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        //tool bar button click
        if (item.getItemId() == android.R.id.home) {
            Util.checkForBleDeviceToUnpair(mDeviceAddress);
            Util.bleDisconnection(WifiOnboardingViaBleActivity.this, mCallingActivity);
        }
        return super.onOptionsItemSelected(item);
    }

    private void checkWifiStatus() {
        try {
            dialog = new ProgressDialog(WifiOnboardingViaBleActivity.this);
            dialog.setMessage(Constants.STATUS_BOARD_DIALOG);
            dialog.setCancelable(false);
            dialog.show();
            BleServiceCharecteristic.getInstance().readBleBoardStatus();
            Handler handler3 = new Handler();
            handler3.postDelayed(new Runnable() {
                public void run() {
                    String status = BleServiceCharecteristic.getInstance().readBleBoardStatus();
                    Logger.d(TAG, " wifi -> status2 " + status);

                }
            }, 2000);

            Handler handler4 = new Handler();
            handler4.postDelayed(new Runnable() {
                public void run() {
                    String status = BleServiceCharecteristic.getInstance().readBleBoardStatus();
                    Logger.d(TAG, " wifi -> status3 " + status);
                }
            }, 4000);

            Handler handler5 = new Handler();
            handler5.postDelayed(new Runnable() {
                public void run() {
                    wifiStatus = BleServiceCharecteristic.getInstance().readBleBoardStatus();
                    Logger.d(TAG, " wifi -> status3 " + wifiStatus);
                    if (ConnectBle.val == 3) {
                        checkZigbeeThreadStatus();
                    } else {
                        if (wifiStatus != null) {
                            if (wifiStatus.equals(Constants.STATUS_ONBOARDED)) {
                                dialog.dismiss();
                                switchingOnboardScreen();
                                onboardThreadOrZigbee();
                            } else {
                                dialog.dismiss();
                                onboardThreadOrZigbee();
                            }
                        } else {
                            Util.checkForBleDeviceToUnpair(mDeviceAddress);
                            Util.bleDisconnection(WifiOnboardingViaBleActivity.this, mCallingActivity);
                            Toast.makeText(WifiOnboardingViaBleActivity.this, R.string.toastNoBoardInfo, Toast.LENGTH_SHORT).show();
                            Logger.d("disconnection", "1");

                        }
                    }
                }
            }, 6000);
        }catch (Exception e)
        {
            Logger.d(TAG, "Exception in checkWifiStatus() ->" + e);
        }

    }

    @Override
    protected void onResume() {
        super.onResume();
        checkWifiStatus();
        displayData();
    }



    private void displayData() {
        try {
            getBleDataFromSP();
            updateConnectionState();
            mWifiBleFilteredList = new ArrayList<ScanResult>();
            wiFiScanManager.wifiScan();
            wifiScanListSpinner.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {

                public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                    ssidTxt = parent.getItemAtPosition(position).toString();
                    for (int i = 0; i < mWifiBleFilteredList.size(); i++) {
                        ScanResult device = mWifiBleFilteredList.get(i);
                        String configuredDevice = device.SSID;
                        if (configuredDevice.contains("\"")) {
                            configuredDevice = configuredDevice.replace("\"", "");
                        }

                        if (configuredDevice.equalsIgnoreCase(ssidTxt)) {
                            if (device.capabilities.contains(Constants.WIFI_SECURITY_WEP) || device.capabilities.contains(Constants.WIFI_SECURITY_WPA)) {
                                password.setVisibility(View.VISIBLE);
                                textViewPassword.setVisibility(View.VISIBLE);
                                chk_paswd.setVisibility(View.VISIBLE);
                            } else {
                                password.setVisibility(View.GONE);
                                textViewPassword.setVisibility(View.GONE);
                                chk_paswd.setVisibility(View.GONE);
                            }
                        }
                    }


                }

                public void onNothingSelected(AdapterView<?> parent) {
                }
            });
        }catch (Exception e)
        {
                Logger.d(TAG, "Exception in displayData() ->" + e);
        }

    }

    @Override
    protected void onPause() {
        super.onPause();

        if (dialog != null) {
            dialog.dismiss();
        }
        else
        {
            Logger.d(TAG, "Exception in onPause() -> dialog = null");
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
    }


    //get data from shared preference
    private void getBleDataFromSP() {
        try {
            mDeviceName = sp.getString(Constants.BLE_DEVICE_NAME, null);
            mDeviceAddress = sp.getString(Constants.BLE_DEVICE_ADDRESS, null);
        } catch (Exception e) {
            Logger.d(TAG, " Exception in getBleDataFromSP() -> " + e);
        }
    }

    //scanning AP list(wifi device near by)
    @Override
    public void onWifiDeviceList(List<ScanResult> data) {
        try {
            if (data.size() != 0) {
                for (int i = 0; i < data.size(); i++) {
                    if (!data.get(i).SSID.contains(Constants.WIFI_BLE_FILTER_KEYWORD)) {
                        mWifiBleFilteredList.add(data.get(i));
                    }
                }

                List<WifiConfiguration> configuredWifiList = application.getWifiManager().getConfiguredNetworks();
                ssidList = new ArrayList<String>();
                ssidList.add(0, getString(R.string.ssid_select));
                HashSet<String> uniqueSsid = new HashSet();
                if (configuredWifiList.size() != 0) {

                    for (int i = 0; i < configuredWifiList.size(); i++) {
                        WifiConfiguration device = configuredWifiList.get(i);
                        String name = device.SSID;
                        if (name.contains("\"")) {
                            name = name.replace("\"", "");
                        }
                        if (!(name.contains(Constants.WIFI_BLE_FILTER_KEYWORD))) {
                            if (!(name.equals(""))) {
                                if (name.length() < Constants.VALIDATION_SSID)
                                    for (int j = 0; j < mWifiBleFilteredList.size(); j++) {
                                        if (name.equals(mWifiBleFilteredList.get(j).SSID)) {
                                            uniqueSsid.add(name);
                                        }
                                    }
                            }
                        }
                    }
                    ssidList.addAll(uniqueSsid);
                    ArrayAdapter adapter = new ArrayAdapter(this, android.R.layout.simple_spinner_dropdown_item, ssidList);
                    wifiScanListSpinner.setAdapter(adapter);
                }
            } else {
                Toast.makeText(WifiOnboardingViaBleActivity.this, R.string.toastWifiDeviceNotFound, Toast.LENGTH_SHORT).show();
            }
        } catch (Exception e) {
            e.printStackTrace();
            Logger.d(TAG, " Exception in onWifiDeviceList() -> " + e);
        }
    }


    private void resetValue() {
        password.setText("");
        wifiScanListSpinner.setSelection(0);
    }

    /**
     * This method communicates the user SSID and Password selected from the screen to the Qualcomm Board
     *
     * @param ssid
     * @param password
     */
    private void writeToBoard(String ssid, String password) {
        try {
            //check board status
            // onboarded or not
            String boardStatus = BleServiceCharecteristic.getInstance().readBleBoardStatus();
            if (boardStatus != null) {
                //if not onboarded
                if (boardStatus.contains(Constants.STATUS_NOT_ONBOARDED)) {

                    dialog = new ProgressDialog(WifiOnboardingViaBleActivity.this);
                    dialog.setMessage("Connecting to " + ssidTxt + " AP");
                    dialog.setCancelable(false);
                    dialog.show();

                    Handler onboardHandler = new Handler();
                    onboardHandler.postDelayed(new Runnable() {
                        public void run() {
                            //after 12 sec it will check board status again
                            BleServiceCharecteristic.getInstance().readBleBoardStatus();
                        }
                    }, 12000);
                    //this method calls 1st
                    //this  method call is for onboarding SAP via BLE
                    BleServiceCharecteristic.getInstance().wifiOnboardingViaBle(ssid, password);

                    Handler handler2 = new Handler();
                    handler2.postDelayed(new Runnable() {
                        public void run() {
                            String boardStatus = BleServiceCharecteristic.getInstance().readBleBoardStatus();
                            if (boardStatus.contains(Constants.STATUS_NOT_ONBOARDED)) {
                                Toast.makeText(WifiOnboardingViaBleActivity.this, R.string.toastNotSuccessfullOnboard, Toast.LENGTH_SHORT).show();
                            } else {

                                // if it equals 0 it has to do only wifi onboarding
                                if (ConnectBle.val == 0) {
                                    dialog.dismiss();
                                    switchingOnboardScreen();
                                } else if (ConnectBle.val == 1) {
                                    BleServiceCharecteristic.getInstance().readZigbeeBoardStatus();
                                } else if (ConnectBle.val == 2) {
                                    BleServiceCharecteristic.getInstance().readThreadBoardStatus();
                                } else {
                                    BleServiceCharecteristic.getInstance().readZigbeeBoardStatus();
                                }
                            }
                        }
                    }, 14000);

                    Handler zigbeeHandler = new Handler();
                    zigbeeHandler.postDelayed(new Runnable() {
                        public void run() {
                            dialog.dismiss();
                            wifiStatus = BleServiceCharecteristic.getInstance().readBleBoardStatus();
                            onboardThreadOrZigbee();
                        }
                    }, 17000);

                } else if (boardStatus.contains(Constants.STATUS_ONBOARDED)) {
                    if (ConnectBle.val == 0) {
                        switchingOnboardScreen();
                    } else if (ConnectBle.val == 1) {
                        String status = BleServiceCharecteristic.getInstance().readZigbeeBoardStatus();
                        Logger.d(TAG, " Reading zigbee status4 " + status);
                        Handler handler2 = new Handler();
                        handler2.postDelayed(new Runnable() {
                            public void run() {
                                Util.gotoScreen(WifiOnboardingViaBleActivity.this, ZigBeeOnBoardingViaBleActivity.class, mCallingActivity);
                            }
                        }, 2000);
                    } else if (ConnectBle.val == 2) {
                        String status = BleServiceCharecteristic.getInstance().readThreadBoardStatus();
                        Logger.d(TAG, " Reading zigbee status4 " + status);
                        Handler handler2 = new Handler();
                        handler2.postDelayed(new Runnable() {
                            public void run() {
                                Util.gotoScreen(WifiOnboardingViaBleActivity.this, ThreadOnboardingViaBleActivity.class, mCallingActivity);
                            }
                        }, 2000);
                    }
                }
            } else {
                Util.bleDisconnection(WifiOnboardingViaBleActivity.this, mCallingActivity);
                Toast.makeText(WifiOnboardingViaBleActivity.this, R.string.toastNoBoardInfo, Toast.LENGTH_SHORT).show();
                Logger.d("disconnection", "2");

            }
        } catch (Exception e) {
            Logger.d(TAG, " Exception in writeToBoard() -> " + e);
        }
    }

    //succefull onboarding screen
    public void switchingOnboardScreen() {
        onboardMode.setText(": STA + SAP");
        name.setText(": " + mDeviceName);
        macAddr.setText(": " + mDeviceAddress);
        onBoardLayout.setVisibility(View.VISIBLE);
        credentialsLayout.setVisibility(View.GONE);
        buttonLayout.setVisibility(View.GONE);
    }

    //if zigbee present ConnectBle.val==1
    // insted of cancel button ,skip button required here
    private void updateConnectionState() {
        if (ConnectBle.val == 0) {
            name.setText(":  " + mDeviceName);
            macAddr.setText(":  " + mDeviceAddress);
            onboardMode.setText(":  BLE");
        } else {
            name.setText(":  " + mDeviceName);
            macAddr.setText(":  " + mDeviceAddress);
            onboardMode.setText(":  BLE");
            cancel.setText("SKIP");
        }


    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.bleBtnOnboard:
                try {
                    //on board button click
                    passwordTxt = password.getText().toString();

                    if (passwordTxt.length() >= Constants.VALIDATION_MIN_PASSWORD) {

                        if (!ssidTxt.equals(getString(R.string.ssid_select))) {
                            writeToBoard(ssidTxt, passwordTxt);
                        } else {
                            Toast.makeText(WifiOnboardingViaBleActivity.this, R.string.no_ssid, Toast.LENGTH_SHORT).show();
                        }

                    } else if (passwordTxt.length() == 0 && password.getVisibility() == View.GONE) {
                        writeToBoard(ssidTxt, "open");
                    } else {
                        Toast.makeText(WifiOnboardingViaBleActivity.this, R.string.toastPasswordValidation, Toast.LENGTH_SHORT).show();
                    }
                } catch (Exception e) {
                    Logger.d(TAG, " Exception in onBoardButtonClick() -> " + e);
                }

                break;
            case R.id.bleBtnCancel:
                //cancel button acts based on this ConnectBle.val
                //if it is "1" instead of cancel button there will be skip button
                //if it is "0" there will be cancel button
                try {

                    if (ConnectBle.val == 0) {
                        Util.checkForBleDeviceToUnpair(mDeviceAddress);
                        Util.bleDisconnection(WifiOnboardingViaBleActivity.this, mCallingActivity);
                    } else if (ConnectBle.val == 1) {
                        BleServiceCharecteristic.getInstance().readZigbeeBoardStatus();
                        Util.gotoScreen(WifiOnboardingViaBleActivity.this, ZigBeeOnBoardingViaBleActivity.class, mCallingActivity);
                    } else if (ConnectBle.val == 2) {
                        BleServiceCharecteristic.getInstance().readThreadBoardStatus();
                        Util.gotoScreen(WifiOnboardingViaBleActivity.this, ThreadOnboardingViaBleActivity.class, mCallingActivity);
                    } else {
                        String status = BleServiceCharecteristic.getInstance().readZigbeeBoardStatus();
                        if (status != null) {
                            if (zigbeeStatus.equals(Constants.STATUS_ONBOARDED)) {
                                Util.gotoScreen(WifiOnboardingViaBleActivity.this, ZigBeeOnBoardingViaBleActivity.class, mCallingActivity);
                            } else if (threadStatus.equals(Constants.STATUS_ONBOARDED)) {
                                Util.gotoScreen(WifiOnboardingViaBleActivity.this, ThreadOnboardingViaBleActivity.class, mCallingActivity);
                            } else {
                                Util.gotoScreen(WifiOnboardingViaBleActivity.this, ZigBeeOnBoardingViaBleActivity.class, mCallingActivity);
                            }
                        } else {
                            Util.checkForBleDeviceToUnpair(mDeviceAddress);
                            Util.bleDisconnection(WifiOnboardingViaBleActivity.this, mCallingActivity);
                            Toast.makeText(WifiOnboardingViaBleActivity.this, R.string.toastNoBoardInfo, Toast.LENGTH_SHORT).show();
                            Logger.d("disconnection", "3");

                        }
                    }


                } catch (Exception e) {
                    Logger.d(TAG, " Exception in cancelButtonClick() -> " + e);
                }
                break;
            case R.id.bleButtonOnBoardOk:
                try {
                    resetValue();
                    Util.checkForBleDeviceToUnpair(mDeviceAddress);
                    Util.bleDisconnection(WifiOnboardingViaBleActivity.this, mCallingActivity);
                } catch (Exception e) {
                    Logger.d(TAG, " Exception in OKbuttonClick() -> " + e);
                }
                break;
        }
    }

    private void checkZigbeeThreadStatus() {
        try {
            BleServiceCharecteristic.getInstance().readZigbeeBoardStatus();
            Handler handler2 = new Handler();
            handler2.postDelayed(new Runnable() {
                public void run() {
                    BleServiceCharecteristic.getInstance().readZigbeeBoardStatus();
                }
            }, 1000);

            Handler handler3 = new Handler();
            handler3.postDelayed(new Runnable() {
                public void run() {
                    BleServiceCharecteristic.getInstance().readThreadBoardStatus();
                }
            }, 2000);
            Handler handler4 = new Handler();
            handler4.postDelayed(new Runnable() {
                public void run() {
                    BleServiceCharecteristic.getInstance().readThreadBoardStatus();
                }
            }, 3000);

            Handler handler5 = new Handler();
            handler5.postDelayed(new Runnable() {
                public void run() {
                    BleServiceCharecteristic.getInstance().readZigbeeBoardStatus();
                }
            }, 4000);
            Handler handler6 = new Handler();
            handler6.postDelayed(new Runnable() {
                public void run() {
                    threadStatus = BleServiceCharecteristic.getInstance().readThreadBoardStatus();
                    Logger.d(TAG, " Reading Thread status4 " + threadStatus);
                }
            }, 5000);

            Handler handler7 = new Handler();
            handler7.postDelayed(new Runnable() {
                public void run() {
                    zigbeeStatus = BleServiceCharecteristic.getInstance().readZigbeeBoardStatus();
                    Logger.d(TAG, " Reading zigbee status3 " + zigbeeStatus);

                    if (wifiStatus != null) {
                        if (wifiStatus.equals(Constants.STATUS_ONBOARDED)) {
                            dialog.dismiss();
                            switchingOnboardScreen();
                            onboardThreadOrZigbee();
                        } else {
                            dialog.dismiss();
                            onboardThreadOrZigbee();
                        }
                    } else {
                        Util.checkForBleDeviceToUnpair(mDeviceAddress);
                        Util.bleDisconnection(WifiOnboardingViaBleActivity.this, mCallingActivity);
                        Toast.makeText(WifiOnboardingViaBleActivity.this, R.string.toastNoBoardInfo, Toast.LENGTH_SHORT).show();
                        Logger.d("disconnection", "1");

                    }

                }
            }, 6000);
        }catch (Exception e)
        {
            Logger.d(TAG, " Exception in checkZigbeeThreadStatus() -> " + e);
        }

    }


    private void onboardThreadOrZigbee() {

        switch (ConnectBle.val) {
            case 1:
                if (wifiStatus.equals(Constants.STATUS_ONBOARDED)) {
                    BleServiceCharecteristic.getInstance().readZigbeeBoardStatus();
                    Util.gotoScreen(WifiOnboardingViaBleActivity.this, ZigBeeOnBoardingViaBleActivity.class, mCallingActivity);
                    Toast.makeText(WifiOnboardingViaBleActivity.this, R.string.toastWifiSuccessfullOnboard, Toast.LENGTH_SHORT).show();
                }
                break;
            case 2:
                if (wifiStatus.equals(Constants.STATUS_ONBOARDED)) {
                    BleServiceCharecteristic.getInstance().readThreadBoardStatus();
                    Util.gotoScreen(WifiOnboardingViaBleActivity.this, ThreadOnboardingViaBleActivity.class, mCallingActivity);
                    Toast.makeText(WifiOnboardingViaBleActivity.this, R.string.toastWifiSuccessfullOnboard, Toast.LENGTH_SHORT).show();
                }
                break;
            case 3:
                if (wifiStatus.equals(Constants.STATUS_ONBOARDED)) {
                    Logger.d(TAG, " Reading status2 " + zigbeeStatus + "," + threadStatus);
                    if (zigbeeStatus != null && threadStatus != null) {
                        if (zigbeeStatus.equals(Constants.STATUS_ONBOARDED) || threadStatus.equals(Constants.STATUS_NOT_ONBOARDED)) {
                            BleServiceCharecteristic.getInstance().readZigbeeBoardStatus();
                            Util.gotoScreen(WifiOnboardingViaBleActivity.this, ZigBeeOnBoardingViaBleActivity.class, mCallingActivity);
                        } else if (threadStatus.equals(Constants.STATUS_ONBOARDED)) {
                            BleServiceCharecteristic.getInstance().readThreadBoardStatus();
                            Util.gotoScreen(WifiOnboardingViaBleActivity.this, ThreadOnboardingViaBleActivity.class, mCallingActivity);
                        }
                        Toast.makeText(WifiOnboardingViaBleActivity.this, R.string.toastWifiSuccessfullOnboard, Toast.LENGTH_SHORT).show();
                    } else {
                        Util.bleDisconnection(WifiOnboardingViaBleActivity.this, mCallingActivity);
                        Toast.makeText(WifiOnboardingViaBleActivity.this, R.string.toastNoBoardInfo, Toast.LENGTH_SHORT).show();
                        Logger.d("disconnection", "4");
                    }
                }
                break;
        }
    }



}