/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package com.qualcomm.qti.qca40xx.Activity;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.net.wifi.WifiInfo;
import android.os.Bundle;
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

import com.qualcomm.qti.qca40xx.Interface.IMessageRecieved;
import com.qualcomm.qti.qca40xx.Util.Constants;
import com.qualcomm.qti.qca40xx.Util.DBHelper;
import com.qualcomm.qti.qca40xx.Util.Logger;
import com.qualcomm.qti.qca40xx.Util.MainApplication;
import com.qualcomm.qti.qca40xx.Util.TCPComm;
import com.qualcomm.qti.qca40xx.Util.Util;
import com.qualcomm.qti.qca40xx.R;

import org.json.JSONObject;

import java.net.InetAddress;
import java.util.ArrayList;
import java.util.List;


/**
 * ZigbeeOnboardingViaWifiActivity is an Activity class for showing the user the details of the Qualcomm Board
 * via SAP for onboarding.
 */
public class ZigbeeOnboardingViaWifiActivity extends AppCompatActivity implements View.OnClickListener, IMessageRecieved {
    private static final String TAG = "ZigbeeOnboardingViaWifiActivity";
    private static int val;
    private TextView name, macAddr, onboardMode, powerMode, fwversion, chipset, nameList;
    private String ip, deviceName, mac;
    private SharedPreferences settings;
    private LinearLayout onBoardLayout, buttonLayout, capabilityLayout;
    private Context mContext;
    private int netId;
    private String jsonData;
    private ArrayList<String> arraySpinner;
    private Spinner capabilitySpinner, deviceNameListSpinner;
    private String supportMode;
    private List<String> coordinatorList = null;
    private DBHelper dbHelper;
    private JSONObject json;
    private TCPComm tcpComm;
    private Button onboard, cancel, ok;
    private Toolbar toolbar;
    private String mCallingActivity = "";
    private MainApplication application;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_zigbee_onboarding_via_wifi);

        initialize();
        getDeviceDataFromSP();
        Intent intent = getIntent();
        jsonData = intent.getStringExtra(Constants.READ_DATA);
        mCallingActivity = intent.getStringExtra(Constants.CALLING_ACTIVITY);
        parseJson(jsonData);
    }

    private int checkAPState() {
        int state;
        WifiInfo info = application.getWifiManager().getConnectionInfo();
        if (mac.equals(info.getBSSID())) {
            state = 1;
        } else {
            state = 0;
            Toast.makeText(ZigbeeOnboardingViaWifiActivity.this, R.string.toastConnectionLoss, Toast.LENGTH_SHORT).show();

        }
        return state;
    }

    @Override
    protected void onResume() {
        super.onResume();

        capabilitySpinner.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> arg0, View arg1,
                                       int arg2, long arg3) {

                String capabilityItems = capabilitySpinner.getSelectedItem().toString();
                if (capabilityItems.equals(Constants.ROUTER)) {
                    supportMode = "r";
                    deviceNameListSpinner.setEnabled(true);
                    getCordinatorDataFromDB();
                } else if (capabilityItems.equals(Constants.ENDDEVICE)) {
                    supportMode = "e";
                    System.out.println("-----------------" + supportMode);
                    deviceNameListSpinner.setEnabled(true);
                    getCordinatorDataFromDB();
                } else if (capabilityItems.equals(Constants.COORDINATOR)) {
                    supportMode = "c";
                    nameList.setVisibility(View.GONE);
                    deviceNameListSpinner.setVisibility(View.GONE);
                }
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {

            }
        });
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if (item.getItemId() == android.R.id.home) {
            Util.goToHomeScreen(ZigbeeOnboardingViaWifiActivity.this, mCallingActivity);
        }
        return super.onOptionsItemSelected(item);
    }

    private void initialize() {
        mContext = this;
        settings = ZigbeeOnboardingViaWifiActivity.this.getSharedPreferences(Constants.IP_ADDRESS, Context.MODE_PRIVATE);
        application = (MainApplication) getApplicationContext();
        coordinatorList = new ArrayList<>();
        dbHelper = new DBHelper(this);
        json = new JSONObject();
        toolbar = (Toolbar) findViewById(R.id.tool_bar);

        //textview
        name = (TextView) findViewById(R.id.zigbeeWifiTextViewDeviceName);
        macAddr = (TextView) findViewById(R.id.zigbeeWifiTextViewMacAddress);
        onboardMode = (TextView) findViewById(R.id.zigbeeWifiTextViewOnboardingMode);
        powerMode = (TextView) findViewById(R.id.zigbeeWifiTextViewBatteryMode);
        fwversion = (TextView) findViewById(R.id.zigbeeWifiTextViewFversion);
        chipset = (TextView) findViewById(R.id.zigbeeWifiTextViewChipset);

        nameList = (TextView) findViewById(R.id.zigbeeWifiDeviceList);

        onBoardLayout = (LinearLayout) findViewById(R.id.zigbeeWifilinearLayoutOnBoarded);
        capabilityLayout = (LinearLayout) findViewById(R.id.ZigbeeWifilinearLayoutCapability);
        buttonLayout = (LinearLayout) findViewById(R.id.zigbeeWifilinearLayoutButtons);
        //spinner
        deviceNameListSpinner = (Spinner) findViewById(R.id.zigbeeWifiSpinnerDeviceList);
        capabilitySpinner = (Spinner) findViewById(R.id.zigbeeWifiSpinnerCapabilityList);

        onboard = (Button) findViewById(R.id.zigbeeWifibtnOnboard);
        cancel = (Button) findViewById(R.id.zigbeeWifibtnCancel);
        ok = (Button) findViewById(R.id.zigbeeWifiButtonOnBoardOk);

        onboard.setOnClickListener(this);
        cancel.setOnClickListener(this);
        ok.setOnClickListener(this);

        toolbar.setTitle(Constants.TITLE_ONBOARDING_DETAILS);
        setSupportActionBar(toolbar);

        getSupportActionBar().setHomeButtonEnabled(true);
        getSupportActionBar().setDisplayHomeAsUpEnabled(true);

    }

    public void getDeviceDataFromSP() {
        ip = settings.getString(Constants.IP, null);
        deviceName = settings.getString(Constants.DEVICE_NAME, null);
        mac = settings.getString(Constants.MAC_ADDRESS, null);
        netId = settings.getInt(Constants.NETWORK_ID, -1);
    }

    private void getCordinatorDataFromDB() {
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
            deviceNameListSpinner.setVisibility(View.GONE);
            nameList.setText("No Coordinator Found");
        }
    }


    public void parseJson(String data) {
        try {

            JSONObject jsonObject = new JSONObject(data);
            String fVersionTxt = jsonObject.getString(Constants.JSON_FW_VER);
            String chipSetTxt = jsonObject.getString(Constants.JSON_CHIPSET);
            String batModeTxt = jsonObject.getString(Constants.JSON_BAT_MODE);
            String operationModeTxt = jsonObject.getString(Constants.JSON_OPERATION_MODE);
            String JBModeTxt = jsonObject.getString(Constants.JSON_JBMODE);
            String onBoardTxt = jsonObject.getString(Constants.JSON_ONBOARDED);

            name.setText(": " + deviceName);
            macAddr.setText(": " + mac);
            fwversion.setText(": " + fVersionTxt);
            chipset.setText(": " + chipSetTxt);
            powerMode.setText(": " + batModeTxt);

            if (onBoardTxt.equalsIgnoreCase(Constants.NONE) || onBoardTxt.equalsIgnoreCase(Constants.WIFI)) {
                onboardMode.setText(": " + onBoardTxt);
                onBoardLayout.setVisibility(View.GONE);
                int mode = Integer.parseInt(JBModeTxt);
                arraySpinner = Util.checkSupportMode(mode, Constants.ZIGBEE);
                if (arraySpinner.size() == 1) {
                    ArrayAdapter<String> adapter = new ArrayAdapter<String>(ZigbeeOnboardingViaWifiActivity.this, android.R.layout.simple_spinner_dropdown_item, arraySpinner);
                    capabilitySpinner.setAdapter(adapter);
                    capabilitySpinner.setEnabled(false);
                } else {
                    ArrayAdapter<String> adapter = new ArrayAdapter<String>(ZigbeeOnboardingViaWifiActivity.this, android.R.layout.simple_spinner_dropdown_item, arraySpinner);
                    capabilitySpinner.setAdapter(adapter);
                    capabilitySpinner.setEnabled(true);
                }

            } else {
                onboardMode.setText(": " + onBoardTxt);
                onBoardLayout.setVisibility(View.VISIBLE);
                capabilityLayout.setVisibility(View.GONE);
                buttonLayout.setVisibility(View.GONE);
                Util.disconnectWifiNetwork(mContext, netId);
            }


            if (operationModeTxt.contains(";")) {
                val = splitDataByDelimiter(operationModeTxt);
            }


        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public int splitDataByDelimiter(String result) {
        String[] data = result.split(Constants.SPLIT_STRING);

        if (data.length == Constants.WIFI_TRIPLE_MODE_ONBOARDING) {
            cancel.setText("SKIP");
            val = Constants.WIFI_TRIPLE_MODE_ONBOARDING;
        }

        return val;
    }


    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.zigbeeWifibtnOnboard:
                String data;
                String linkkey = "";
                String capabilityItems = capabilitySpinner.getSelectedItem().toString();
                if (capabilityItems.equals(Constants.COORDINATOR)) {
                    linkkey = Util.generatingRandomKey();
                    boolean isAvail = dbHelper.dataExistOrNot(deviceName);
                    if (isAvail) {
                        dbHelper.updateDataIntoTable(deviceName, linkkey, "C");
                    } else {
                        dbHelper.insertDataIntoTable(deviceName, mac, linkkey, Constants.SUPPORT_MODE_WIFI, "C",Constants.NO);

                    }

                } else if (capabilityItems.equals(Constants.ROUTER)) {
                    if (deviceNameListSpinner.getSelectedItem() != null) {
                        String coordinatorName = deviceNameListSpinner.getSelectedItem().toString();
                        String dName = Util.getDeviceName(coordinatorName);


                        if (!TextUtils.isEmpty(dName)) {
                            linkkey = dbHelper.getMasterKey(dName);
                            boolean isAvail = dbHelper.dataExistOrNot(deviceName);
                            if (isAvail) {
                                dbHelper.updateDataIntoTable(deviceName, linkkey, "R");
                            } else
                                dbHelper.insertDataIntoTable(deviceName, mac, linkkey, Constants.SUPPORT_MODE_WIFI, "R",Constants.NO);
                        }
                    } else {
                        Toast.makeText(ZigbeeOnboardingViaWifiActivity.this, R.string.toastNoCoordinator, Toast.LENGTH_SHORT).show();
                        Logger.d(TAG, "coordinatorName " + deviceNameListSpinner.getSelectedItem());

                    }

                } else {
                    if (deviceNameListSpinner.getSelectedItem() != null) {
                        String coordinatorName = deviceNameListSpinner.getSelectedItem().toString();
                        String dName = Util.getDeviceName(coordinatorName);

                        if (!TextUtils.isEmpty(dName)) {
                            linkkey = dbHelper.getMasterKey(dName);
                        }
                    } else {
                        Toast.makeText(ZigbeeOnboardingViaWifiActivity.this, R.string.toastNoCoordinator, Toast.LENGTH_SHORT).show();

                        Logger.d(TAG, "coordinatorName " + deviceNameListSpinner.getSelectedItem());

                    }

                }

                if (!TextUtils.isEmpty(linkkey)) {
                    try {
                        json.put("Action", "ConfigZigbee");
                        json.put("Mode", supportMode);
                        json.put("Linkkey", linkkey);
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                    data = json.toString();
                    try {
                        int state = checkAPState();
                        if (state == 1) {
                            tcpComm = new TCPComm(InetAddress.getByName(ip), ZigbeeOnboardingViaWifiActivity.this, this, data);
                        } else {
                            Util.goToHomeScreen(ZigbeeOnboardingViaWifiActivity.this, mCallingActivity);
                        }
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                }

                break;
            case R.id.zigbeeWifibtnCancel:
                if (val == Constants.WIFI_TRIPLE_MODE_ONBOARDING) {
                    Util.gotoScreenWithData(ZigbeeOnboardingViaWifiActivity.this,
                            ThreadOnboardingViaWifiActivity.class, mCallingActivity, jsonData);
                } else {
                    Util.disconnectWifiNetwork(mContext, netId);
                    onBackPressed();
                }
                break;
            case R.id.zigbeeWifiButtonOnBoardOk:
                Util.disconnectWifiNetwork(mContext, netId);
                Util.goToHomeScreen(ZigbeeOnboardingViaWifiActivity.this, mCallingActivity);
                break;
        }
    }

    @Override
    public void onBackPressed() {
        super.onBackPressed();
        Util.disconnectWifiNetwork(mContext, netId);
        Util.goToHomeScreen(ZigbeeOnboardingViaWifiActivity.this, mCallingActivity);
    }


    @Override
    public void tcpMessageRecieved(String message) {
        if (message != null) {
            if (message.isEmpty()) {
                Toast.makeText(this, R.string.toastNoBoardInfo, Toast.LENGTH_SHORT).show();
                Util.disconnectWifiNetwork(mContext, netId);
                Util.goToHomeScreen(ZigbeeOnboardingViaWifiActivity.this, mCallingActivity);
            } else {
                parseJson(message);
            }
        } else {
            Toast.makeText(this, R.string.toastNoBoardInfo, Toast.LENGTH_SHORT).show();
            Util.disconnectWifiNetwork(mContext, netId);
            Util.goToHomeScreen(ZigbeeOnboardingViaWifiActivity.this, mCallingActivity);
        }
    }
}
