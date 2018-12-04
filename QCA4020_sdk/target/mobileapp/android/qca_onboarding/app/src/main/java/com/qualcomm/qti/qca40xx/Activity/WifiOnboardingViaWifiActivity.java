/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package com.qualcomm.qti.qca40xx.Activity;

import android.content.Context;
import android.content.SharedPreferences;
import android.net.wifi.ScanResult;
import android.net.wifi.WifiConfiguration;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.os.Bundle;
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

import com.qualcomm.qti.qca40xx.Interface.IMessageRecieved;
import com.qualcomm.qti.qca40xx.Interface.IWiFiDeviceList;
import com.qualcomm.qti.qca40xx.Manager.WiFiScanManager;
import com.qualcomm.qti.qca40xx.Util.Constants;
import com.qualcomm.qti.qca40xx.Util.Logger;
import com.qualcomm.qti.qca40xx.Util.MainApplication;
import com.qualcomm.qti.qca40xx.Util.TCPComm;
import com.qualcomm.qti.qca40xx.Util.Util;
import com.qualcomm.qti.qca40xx.R;

import org.json.JSONObject;

import java.net.InetAddress;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;

/**
 * WifiOnboardingViaWifiActivity is an Activity class for showing the Qualcomm Board details after connecting via SAP.
 */
public class WifiOnboardingViaWifiActivity extends AppCompatActivity implements IWiFiDeviceList, View.OnClickListener, IMessageRecieved {
    private static final String TAG = "WifiOnboardingDetailActivity";
    private static int val;
    private String jsonData;
    private TextView name, macAddr, onboardMode, powerMode, fwversion, chipset;
    private EditText password;
    private TextView passwordTextView;
    private Button onBoard, cancel, ok;
    private String ssidTxt, passwordTxt;
    private Toolbar toolbar;
    private String ip, deviceName, mac;
    private Spinner wifiScanListSpinner;
    private ArrayList<String> ssidList;
    private WiFiScanManager wiFiScanManager;
    private List<ScanResult> mWifiBleFilteredList;
    private LinearLayout onBoardLayout, buttonLayout, credentialsLayout;
    private int netId;
    private SharedPreferences settings;
    private JSONObject json;
    private TCPComm tcpComm;
    private Context mContext;
    private String mCallingActivity = "";
    private CheckBox chk_paswd;
    private String onBoardTxt;
    private String mode;
    private MainApplication application;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.wifi_onboarding_details_layout);

        initialize();
        getDeviceDataFromSP();
        Logger.d(TAG, "Ip Address " + ip);
        if (ip != null) {
            if (ip.equals("")) {
                Toast.makeText(this, R.string.toastTcpConnectionError, Toast.LENGTH_LONG).show();
                Util.disconnectWifiNetwork(mContext, netId);
                finish();
            } else {
                try {
                    JSONObject json = new JSONObject();
                    json.put(Constants.JSON_KEY_ACTION, Constants.TCP_FIRST_DATA);
                    int state = checkAPState();
                    if (state == 1) {
                        tcpComm = new TCPComm(InetAddress.getByName(ip), WifiOnboardingViaWifiActivity.this, this, json.toString());
                    } else {
                        onBackPressed();
                    }
                } catch (Exception e) {
                    e.printStackTrace();
                    Logger.d(TAG, " Exception in onCreate() -> " + e);
                }
            }

        } else {
            Toast.makeText(this, R.string.toastTcpConnectionError, Toast.LENGTH_LONG).show();
            Util.disconnectWifiNetwork(mContext, netId);
            finish();
        }

    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        //tool bar button click
        if (item.getItemId() == android.R.id.home) {
            Util.disconnectWifiNetwork(mContext, netId);
            onBackPressed();
        }
        return super.onOptionsItemSelected(item);
    }

    @Override
    protected void onResume() {
        super.onResume();
        try {
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
                                passwordTextView.setVisibility(View.VISIBLE);
                                chk_paswd.setVisibility(View.VISIBLE);
                            } else {
                                password.setVisibility(View.GONE);
                                passwordTextView.setVisibility(View.GONE);
                                chk_paswd.setVisibility(View.GONE);
                            }
                        }
                    }
                }

                public void onNothingSelected(AdapterView<?> parent) {
                }
            });

        } catch (Exception e) {
            e.printStackTrace();
            Logger.d(TAG, " Exception in onResume() -> " + e);
        }
    }

    private void initialize() {
        toolbar = (Toolbar) findViewById(R.id.tool_bar);
        mContext = this;
        toolbar.setTitle(Constants.TITLE_ONBOARDING_DETAILS);
        setSupportActionBar(toolbar);
        // toolbar navigation icon setup
        getSupportActionBar().setHomeButtonEnabled(true);
        getSupportActionBar().setDisplayHomeAsUpEnabled(true);
        application = (MainApplication) getApplicationContext();
        settings = WifiOnboardingViaWifiActivity.this.getSharedPreferences(Constants.IP_ADDRESS, Context.MODE_PRIVATE); //1

        Bundle bundle = this.getIntent().getExtras();
        if (bundle != null) {
            mCallingActivity = bundle.getString(Constants.CALLING_ACTIVITY);
        }

        name = (TextView) findViewById(R.id.textViewDeviceName);
        macAddr = (TextView) findViewById(R.id.textViewMacAddress);
        onboardMode = (TextView) findViewById(R.id.textViewOnboardingMode);
        powerMode = (TextView) findViewById(R.id.textViewBatteryMode);
        chipset = (TextView) findViewById(R.id.textViewChipset);
        fwversion = (TextView) findViewById(R.id.textViewFversion);
        chk_paswd = (CheckBox) findViewById(R.id.chk_paswd_wifi);
        wifiScanListSpinner = (Spinner) findViewById(R.id.spinnerWifiScanList);


        password = (EditText) findViewById(R.id.editTextPassword);
        onBoard = (Button) findViewById(R.id.btnOnboard);
        passwordTextView = (TextView) findViewById(R.id.textviewWifiPassword);
        cancel = (Button) findViewById(R.id.btnCancel);
        tcpComm = new TCPComm(this);

        ok = (Button) findViewById(R.id.ButtonOnBoardOk);
        onBoardLayout = (LinearLayout) findViewById(R.id.linearLayoutOnBoarded);
        credentialsLayout = (LinearLayout) findViewById(R.id.linearLayoutCredentials);
        buttonLayout = (LinearLayout) findViewById(R.id.linearLayoutButtons);
        json = new JSONObject();
        wiFiScanManager = new WiFiScanManager(WifiOnboardingViaWifiActivity.this);

        onBoard.setOnClickListener(this);
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

    private void enableViews(boolean flag) {
        onBoard.setEnabled(flag);
        password.setEnabled(flag);
        wifiScanListSpinner.setEnabled(flag);
        chk_paswd.setEnabled(flag);
    }

    private void parseJson(String data) {
        try {
            jsonData = data;
            JSONObject jsonObject = new JSONObject(data);
            String fVersionTxt = jsonObject.getString(Constants.JSON_FW_VER);
            String chipSetTxt = jsonObject.getString(Constants.JSON_CHIPSET);
            String batModeTxt = jsonObject.getString(Constants.JSON_BAT_MODE);

            String operationModeTxt = jsonObject.getString(Constants.JSON_OPERATION_MODE);
            mode = operationModeTxt;

            String onBoarded = jsonObject.getString(Constants.JSON_ONBOARDED);
            onBoardTxt = onBoarded.toLowerCase();
            name.setText(": " + deviceName);
            macAddr.setText(": " + mac);
            fwversion.setText(": " + fVersionTxt);
            chipset.setText(": " + chipSetTxt);
            powerMode.setText(": " + batModeTxt);

            if (operationModeTxt.contains(";")) {
                val = splitDataByDelimiter(operationModeTxt);
            } else if (operationModeTxt.toLowerCase().equalsIgnoreCase(Constants.ZIGBEE)) {
                Util.gotoScreenWithData(WifiOnboardingViaWifiActivity.this,
                        ZigbeeOnboardingViaWifiActivity.class, mCallingActivity, jsonData);
            } else if (operationModeTxt.toLowerCase().equalsIgnoreCase(Constants.THREAD)) {
                Util.gotoScreenWithData(WifiOnboardingViaWifiActivity.this,
                        ThreadOnboardingViaWifiActivity.class, mCallingActivity, jsonData);
            } else {
                val = Constants.WIFI_SINGLE_MODE_ONBOARDING;
            }

            if (val == Constants.WIFI_SINGLE_MODE_ONBOARDING) {
                if (onBoardTxt.equalsIgnoreCase(Constants.NONE)) {

                    onboardMode.setText(": " + onBoardTxt);
                    onBoardLayout.setVisibility(View.GONE);

                } else {
                    onboardMode.setText(": " + onBoardTxt);
                    onBoardLayout.setVisibility(View.VISIBLE);
                    credentialsLayout.setVisibility(View.GONE);
                    buttonLayout.setVisibility(View.GONE);
                    Util.disconnectWifiNetwork(mContext, netId);
                }
            } else if (val == Constants.WIFI_DOUBLE_MODE_ONBOARDING) {

                if (onBoardTxt.equalsIgnoreCase(Constants.NONE)) {
                    onboardMode.setText(": " + onBoardTxt);
                    onBoardLayout.setVisibility(View.GONE);
                } else if (onBoardTxt.equalsIgnoreCase(Constants.THREAD)) {
                    onboardMode.setText(": " + onBoardTxt);
                    onBoardLayout.setVisibility(View.GONE);
                } else if (onBoardTxt.equalsIgnoreCase(Constants.ZIGBEE)) {
                    onboardMode.setText(": " + onBoardTxt);
                    onBoardLayout.setVisibility(View.GONE);
                } else {
                    if (onBoardTxt.contains(Constants.SPLIT_STRING) || onBoardTxt.equalsIgnoreCase(Constants.WIFI)) {
                        String mode = operationModeTxt.toLowerCase();
                        if (mode.contains(Constants.THREAD)) {
                            Util.gotoScreenWithData(WifiOnboardingViaWifiActivity.this,
                                    ThreadOnboardingViaWifiActivity.class, mCallingActivity, jsonData);
                        } else {
                            Util.gotoScreenWithData(WifiOnboardingViaWifiActivity.this,
                                    ZigbeeOnboardingViaWifiActivity.class, mCallingActivity, jsonData);
                        }

                    }
                }
            } else if (val == Constants.WIFI_TRIPLE_MODE_ONBOARDING) {
                if (onBoardTxt.equals(Constants.WIFI)) {
                    onboardMode.setText(": " + onBoardTxt);
                    Util.gotoScreenWithData(WifiOnboardingViaWifiActivity.this,
                            ZigbeeOnboardingViaWifiActivity.class, mCallingActivity, jsonData);
                } else if (onBoardTxt.equals(Constants.NONE)) {
                    onboardMode.setText(": " + onBoardTxt);
                    onBoardLayout.setVisibility(View.GONE);
                } else if (onBoardTxt.equals(Constants.THREAD)) {
                    onboardMode.setText(": " + onBoardTxt);
                    onBoardLayout.setVisibility(View.GONE);
                } else if (onBoardTxt.equals(Constants.ZIGBEE)) {
                    onboardMode.setText(": " + onBoardTxt);
                    onBoardLayout.setVisibility(View.GONE);
                } else if (onBoardTxt.equals(Constants.WIFI_THREAD)) {
                    onboardMode.setText(": " + onBoardTxt);
                    Util.gotoScreenWithData(WifiOnboardingViaWifiActivity.this,
                            ThreadOnboardingViaWifiActivity.class, mCallingActivity, jsonData);
                } else if (onBoardTxt.equals(Constants.WIFI_ZIGBEE)) {
                    onboardMode.setText(": " + onBoardTxt);
                    Util.gotoScreenWithData(WifiOnboardingViaWifiActivity.this,
                            ZigbeeOnboardingViaWifiActivity.class, mCallingActivity, jsonData);
                }

            }

        } catch (Exception e) {
            e.printStackTrace();
            Logger.d(TAG, " Exception in onResume() -> " + e);
        }
    }

    private int splitDataByDelimiter(String result) {
        String[] data = result.split(Constants.SPLIT_STRING);
        if (data.length == Constants.WIFI_DOUBLE_MODE_ONBOARDING) {
            cancel.setText("SKIP");
            val = Constants.WIFI_DOUBLE_MODE_ONBOARDING;
        }
        if (data.length == Constants.WIFI_TRIPLE_MODE_ONBOARDING) {
            cancel.setText("SKIP");
            val = Constants.WIFI_TRIPLE_MODE_ONBOARDING;
        }
        return val;
    }


    @Override
    public void tcpMessageRecieved(String message) {
        if (message != null) {
            if (message.isEmpty()) {
                Toast.makeText(this, R.string.toastNoBoardInfo, Toast.LENGTH_SHORT).show();
                Util.disconnectWifiNetwork(mContext, netId);
                finish();
            } else {
                parseJson(message);
                enableViews(true);
            }
        } else {
            Toast.makeText(this, R.string.toastNoBoardInfo, Toast.LENGTH_SHORT).show();
            Util.disconnectWifiNetwork(mContext, netId);
            finish();
        }
    }

    private void getDeviceDataFromSP() {
        ip = settings.getString(Constants.IP, null);
        deviceName = settings.getString(Constants.DEVICE_NAME, null);
        mac = settings.getString(Constants.MAC_ADDRESS, null);
        netId = settings.getInt(Constants.NETWORK_ID, -1);
    }

    private int checkAPState() {
        int state = 0;
        WifiManager wifiManager = (WifiManager) mContext.getSystemService(Context.WIFI_SERVICE);
        WifiInfo info = wifiManager.getConnectionInfo();
        if (mac.equalsIgnoreCase(info.getBSSID())) {
            state = 1;
        } else {
            state = 0;
            Toast.makeText(WifiOnboardingViaWifiActivity.this, R.string.toastConnectionLoss, Toast.LENGTH_SHORT).show();
        }
        return state;
    }


    @Override
    public void onWifiDeviceList(List<ScanResult> data) {
        try {
            if (data.size() != 0) {
                System.out.println(data);
                for (int i = 0; i < data.size(); i++) {
                    if (!data.get(i).SSID.contains(Constants.WIFI_BLE_FILTER_KEYWORD)) {
                        mWifiBleFilteredList.add(data.get(i));
                    }
                }

                List<WifiConfiguration> configuredWifiList = application.getWifiManager().getConfiguredNetworks();
                ssidList = new ArrayList<String>();
                ssidList.add(0, getResources().getString(R.string.ssid_select));
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
                }
                ssidList.addAll(uniqueSsid);
                ArrayAdapter adapter = new ArrayAdapter(this, android.R.layout.simple_spinner_dropdown_item, ssidList);
                wifiScanListSpinner.setAdapter(adapter);

            } else {
                Toast.makeText(WifiOnboardingViaWifiActivity.this, getResources().getString(R.string.no_wifi_device), Toast.LENGTH_SHORT).show();
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.btnOnboard:
                String data;
                passwordTxt = password.getText().toString();
                if (ssidTxt.equalsIgnoreCase(getString(R.string.ssid_select))) {
                    Toast.makeText(WifiOnboardingViaWifiActivity.this, R.string.no_ssid, Toast.LENGTH_SHORT).show();
                } else {
                    if (passwordTxt.isEmpty()) {
                        try {
                            json.put("Action", "ConfigWifi");
                            json.put("SSID", ssidTxt);
                            json.put("Password", "");
                        } catch (Exception e) {
                            e.printStackTrace();
                        }
                        data = json.toString();
                    } else {
                        try {
                            json.put("Action", "ConfigWifi");
                            json.put("SSID", ssidTxt);
                            json.put("Password", passwordTxt);
                        } catch (Exception e) {
                            e.printStackTrace();
                        }
                        data = json.toString();
                    }
                    if (password.getVisibility() == View.VISIBLE) {
                        passwordTxt = password.getText().toString();
                        if (passwordTxt.length() >= Constants.VALIDATION_MIN_PASSWORD) {
                            if (passwordTxt.length() < Constants.VALIDATION_MAX_PASSWORD) {
                                try {
                                    int state = checkAPState();
                                    if (state == 1) {
                                        tcpComm = new TCPComm(InetAddress.getByName(ip), WifiOnboardingViaWifiActivity.this, this, data);
                                    } else {
                                        onBackPressed();
                                    }
                                } catch (Exception e) {
                                    e.printStackTrace();
                                }
                            } else {
                                Toast.makeText(WifiOnboardingViaWifiActivity.this, getResources().getString(R.string.passwd_is_big), Toast.LENGTH_SHORT).show();
                            }
                        } else {
                            Toast.makeText(WifiOnboardingViaWifiActivity.this, getResources().getString(R.string.passwd_min_char), Toast.LENGTH_SHORT).show();
                        }
                    } else {
                        try {
                            int state = checkAPState();
                            if (state == 1) {
                                tcpComm = new TCPComm(InetAddress.getByName(ip), WifiOnboardingViaWifiActivity.this, this, data);
                            } else {
                                onBackPressed();
                            }
                        } catch (Exception e) {
                            e.printStackTrace();
                        }
                    }
                }

                break;
            case R.id.btnCancel:
                if (val == Constants.WIFI_SINGLE_MODE_ONBOARDING) {
                    Util.disconnectWifiNetwork(mContext, netId);
                    onBackPressed();
                }
                if (val == Constants.WIFI_DOUBLE_MODE_ONBOARDING) {
                    String operationMode = mode.toLowerCase();
                    if (operationMode.contains(Constants.THREAD)) {
                        Util.gotoScreenWithData(WifiOnboardingViaWifiActivity.this,
                                ThreadOnboardingViaWifiActivity.class, mCallingActivity, jsonData);
                    } else {
                        Util.gotoScreenWithData(WifiOnboardingViaWifiActivity.this,
                                ZigbeeOnboardingViaWifiActivity.class, mCallingActivity, jsonData);
                    }
                } else if (onBoardTxt.equals(Constants.WIFI) || onBoardTxt.equals(Constants.NONE) && val > Constants.WIFI_SINGLE_MODE_ONBOARDING) {
                    Util.gotoScreenWithData(WifiOnboardingViaWifiActivity.this,
                            ZigbeeOnboardingViaWifiActivity.class, mCallingActivity, jsonData);
                } else if (onBoardTxt.equals(Constants.THREAD)) {
                    Util.gotoScreenWithData(WifiOnboardingViaWifiActivity.this,
                            ThreadOnboardingViaWifiActivity.class, mCallingActivity, jsonData);
                } else if (onBoardTxt.equals(Constants.ZIGBEE)) {
                    Util.gotoScreenWithData(WifiOnboardingViaWifiActivity.this,
                            ZigbeeOnboardingViaWifiActivity.class, mCallingActivity, jsonData);
                } else if (onBoardTxt.equals(Constants.WIFI_THREAD)) {
                    Util.gotoScreenWithData(WifiOnboardingViaWifiActivity.this,
                            ThreadOnboardingViaWifiActivity.class, mCallingActivity, jsonData);
                } else if (onBoardTxt.equals(Constants.WIFI_ZIGBEE)) {
                    Util.gotoScreenWithData(WifiOnboardingViaWifiActivity.this,
                            ZigbeeOnboardingViaWifiActivity.class, mCallingActivity, jsonData);
                }
                break;
            case R.id.ButtonOnBoardOk:
                Util.disconnectWifiNetwork(mContext, netId);
                onBackPressed();
                break;
        }
    }
}

