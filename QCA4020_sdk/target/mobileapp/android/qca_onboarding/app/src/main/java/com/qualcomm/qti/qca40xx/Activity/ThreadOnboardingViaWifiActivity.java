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

public class ThreadOnboardingViaWifiActivity extends AppCompatActivity implements View.OnClickListener, IMessageRecieved {
    private static final String TAG = "ThreadOnboardingViaWifiActivity";
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
        setContentView(R.layout.activity_thread_onboarding_via_wifi);
        mContext = this;
        initialize();

        getDeviceDataFromSP();
        Intent intent = getIntent();
        jsonData = intent.getStringExtra(Constants.READ_DATA);
        mCallingActivity = intent.getStringExtra(Constants.CALLING_ACTIVITY);
        parseJson(jsonData);
    }

    private int checkAPState() {
        int state = 0;
        try {
            WifiInfo info = application.getWifiManager().getConnectionInfo();
            if (mac.equals(info.getBSSID())) {
                state = 1;
            } else {
                state = 0;
                Toast.makeText(ThreadOnboardingViaWifiActivity.this, R.string.toastConnectionLoss, Toast.LENGTH_SHORT).show();
            }
        }catch (Exception e)
        {
            Logger.d(TAG, "Exception in checkAPState() ->" + e);
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
                if (capabilityItems.equals(Constants.JOINER_ROUTER)) {
                    supportMode = "r";
                    deviceNameListSpinner.setEnabled(true);
                    getCordinatorDataFromDB();
                } else if (capabilityItems.equals(Constants.JOINER)) {
                    supportMode = "j";
                    System.out.println("-----------------" + supportMode);
                    deviceNameListSpinner.setEnabled(true);
                    getCordinatorDataFromDB();
                } else if (capabilityItems.equals(Constants.BORDER_ROUTER)) {
                    supportMode = "b";
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
            Util.disconnectWifiNetwork(mContext, netId);
            Util.goToHomeScreen(ThreadOnboardingViaWifiActivity.this, mCallingActivity);
        }
        return super.onOptionsItemSelected(item);
    }

    public void initialize() {
        settings = ThreadOnboardingViaWifiActivity.this.getSharedPreferences(Constants.IP_ADDRESS, Context.MODE_PRIVATE);
        coordinatorList = new ArrayList<>();
        dbHelper = new DBHelper(this);
        json = new JSONObject();
        toolbar = (Toolbar) findViewById(R.id.tool_bar);
        application = (MainApplication) getApplicationContext();
        //textview
        name = (TextView) findViewById(R.id.ThreadWifiTextViewDeviceName);
        macAddr = (TextView) findViewById(R.id.ThreadWifiTextViewMacAddress);
        onboardMode = (TextView) findViewById(R.id.ThreadWifiTextViewOnboardingMode);
        powerMode = (TextView) findViewById(R.id.ThreadWifiTextViewBatteryMode);
        fwversion = (TextView) findViewById(R.id.ThreadWifiTextViewFversion);
        chipset = (TextView) findViewById(R.id.ThreadWifiTextViewChipset);
        nameList = (TextView) findViewById(R.id.ThreadWifiDeviceList);
        onBoardLayout = (LinearLayout) findViewById(R.id.ThreadWifilinearLayoutOnBoarded);
        capabilityLayout = (LinearLayout) findViewById(R.id.ThreadWifilinearLayoutCapability);
        buttonLayout = (LinearLayout) findViewById(R.id.ThreadWifilinearLayoutButtons);

        //spinner
        deviceNameListSpinner = (Spinner) findViewById(R.id.ThreadWifiSpinnerDeviceList);
        capabilitySpinner = (Spinner) findViewById(R.id.ThreadWifiSpinnerCapabilityList);

        onboard = (Button) findViewById(R.id.ThreadWifibtnOnboard);
        cancel = (Button) findViewById(R.id.ThreadWifibtnCancel);
        ok = (Button) findViewById(R.id.ThreadWifiButtonOnBoardOk);

        onboard.setOnClickListener(this);
        cancel.setOnClickListener(this);
        ok.setOnClickListener(this);

        toolbar.setTitle(Constants.TITLE_ONBOARDING_DETAILS);
        setSupportActionBar(toolbar);

        getSupportActionBar().setHomeButtonEnabled(true);
        getSupportActionBar().setDisplayHomeAsUpEnabled(true);

    }

    public void getDeviceDataFromSP() {
        try {
            ip = settings.getString(Constants.IP, null);
            deviceName = settings.getString(Constants.DEVICE_NAME, null);
            mac = settings.getString(Constants.MAC_ADDRESS, null);
            netId = settings.getInt(Constants.NETWORK_ID, -1);
        }catch (Exception e)
        {
            Logger.d(TAG, "Exception in getDeviceDataFromSP() ->" + e);
        }
    }

    private void getCordinatorDataFromDB() {
        try
        {
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
            deviceNameListSpinner.setVisibility(View.GONE);
            nameList.setText("No Coordinator Found");
        }
        }catch (Exception e)
        {
            Logger.d(TAG, "Exception in getCordinatorDataFromDB() ->" + e);
        }
    }


    public void parseJson(String data) {
        try {

            JSONObject jsonObject = new JSONObject(data);
            String fVersionTxt = jsonObject.getString(Constants.JSON_FW_VER);
            String chipSetTxt = jsonObject.getString(Constants.JSON_CHIPSET);
            String batModeTxt = jsonObject.getString(Constants.JSON_BAT_MODE);

            String operationModeTxt = jsonObject.getString(Constants.JSON_OPERATION_MODE);
            String ThreadModeTxt = jsonObject.getString(Constants.JSON_TMODE);

            String onBoardTxt = jsonObject.getString(Constants.JSON_ONBOARDED);
            name.setText(": " + deviceName);
            macAddr.setText(": " + mac);
            fwversion.setText(": " + fVersionTxt);
            chipset.setText(": " + chipSetTxt);
            powerMode.setText(": " + batModeTxt);

            if (onBoardTxt.equalsIgnoreCase(Constants.NONE) || onBoardTxt.equalsIgnoreCase(Constants.WIFI)) {
                onboardMode.setText(": " + onBoardTxt);
                onBoardLayout.setVisibility(View.GONE);
                int mode = Integer.parseInt(ThreadModeTxt);
                arraySpinner = Util.checkSupportMode(mode, Constants.THREAD);
                if (arraySpinner.size() == 1) {
                    ArrayAdapter<String> adapter = new ArrayAdapter<String>(ThreadOnboardingViaWifiActivity.this, android.R.layout.simple_spinner_dropdown_item, arraySpinner);
                    capabilitySpinner.setAdapter(adapter);
                    capabilitySpinner.setEnabled(false);
                } else {
                    ArrayAdapter<String> adapter = new ArrayAdapter<String>(ThreadOnboardingViaWifiActivity.this, android.R.layout.simple_spinner_dropdown_item, arraySpinner);
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
        } catch (Exception e) {
            e.printStackTrace();
            Logger.d(TAG, "Exception in parseJson() ->" + e);

        }
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.ThreadWifibtnOnboard:
                String data;
                String passPhrase = "";
                String capabilityItems = capabilitySpinner.getSelectedItem().toString();
                if (capabilityItems.equals(Constants.BORDER_ROUTER)) {
                    passPhrase = Util.generatingRandomKey();
                    Logger.d(TAG, "passPhrase generatingRandomKey()-----------" + passPhrase);

                    boolean isAvail = dbHelper.threadDataExistOrNot(deviceName);
                    if (isAvail) {
                        dbHelper.updateDataIntoThreadTable(deviceName, passPhrase, "b");
                    } else {
                        dbHelper.insertDataIntoThreadTable(deviceName, mac, passPhrase, Constants.SUPPORT_MODE_WIFI, "b");

                    }

                } else if (capabilityItems.equals(Constants.JOINER_ROUTER)) {
                    if (deviceNameListSpinner.getSelectedItem() != null) {
                        String coordinatorName = deviceNameListSpinner.getSelectedItem().toString();
                        String dName = Util.getDeviceName(coordinatorName);


                        if (!TextUtils.isEmpty(dName)) {
                            passPhrase = dbHelper.getThreadPassPhrase(dName);
                            Logger.d(TAG, "passPhrase dbHelper-----------" + passPhrase);

                            boolean isAvail = dbHelper.threadDataExistOrNot(deviceName);
                            if (isAvail) {
                                dbHelper.updateDataIntoThreadTable(deviceName, passPhrase, "r");
                            } else
                                dbHelper.insertDataIntoThreadTable(deviceName, mac, passPhrase, Constants.SUPPORT_MODE_WIFI, "R");
                        }
                    } else {
                        Toast.makeText(ThreadOnboardingViaWifiActivity.this, R.string.toastNoBorderRouter, Toast.LENGTH_SHORT).show();
                        Logger.d(TAG, "borderRouterName " + deviceNameListSpinner.getSelectedItem());

                    }

                } else {
                    if (deviceNameListSpinner.getSelectedItem() != null) {
                        String coordinatorName = deviceNameListSpinner.getSelectedItem().toString();
                        String dName = Util.getDeviceName(coordinatorName);

                        if (!TextUtils.isEmpty(dName)) {
                            passPhrase = dbHelper.getThreadPassPhrase(dName);
                            Logger.d(TAG, "passPhrase dbHelper-----------" + passPhrase);
                        }
                    } else {
                        Toast.makeText(ThreadOnboardingViaWifiActivity.this, R.string.toastNoBorderRouter, Toast.LENGTH_SHORT).show();

                        Logger.d(TAG, "borderRouterName " + deviceNameListSpinner.getSelectedItem());

                    }

                }

                if (!TextUtils.isEmpty(passPhrase)) {
                    try {
                        json.put("Action", "ConfigThread");
                        json.put("Mode", supportMode);
                        json.put("Passphrase", passPhrase);
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                    data = json.toString();
                    try {
                        int state = checkAPState();
                        if (state == 1) {
                            tcpComm = new TCPComm(InetAddress.getByName(ip), ThreadOnboardingViaWifiActivity.this, this, data);
                        } else {
                            Util.goToHomeScreen(ThreadOnboardingViaWifiActivity.this, mCallingActivity);
                        }
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                }

                break;
            case R.id.ThreadWifibtnCancel:
                Util.disconnectWifiNetwork(mContext, netId);
                Util.goToHomeScreen(ThreadOnboardingViaWifiActivity.this, mCallingActivity);
                break;
            case R.id.ThreadWifiButtonOnBoardOk:
                Util.disconnectWifiNetwork(mContext, netId);
                Util.goToHomeScreen(ThreadOnboardingViaWifiActivity.this, mCallingActivity);
                break;
        }
    }

    @Override
    public void onBackPressed() {
        super.onBackPressed();
        Util.disconnectWifiNetwork(mContext, netId);
        Util.goToHomeScreen(ThreadOnboardingViaWifiActivity.this, mCallingActivity);
    }


    @Override
    public void tcpMessageRecieved(String message) {
        if (message != null) {
            if (message.isEmpty()) {
                Toast.makeText(this, R.string.toastNoBoardInfo, Toast.LENGTH_SHORT).show();
                Util.disconnectWifiNetwork(mContext, netId);
                Util.goToHomeScreen(ThreadOnboardingViaWifiActivity.this, mCallingActivity);
            } else {
                parseJson(message);
            }
        } else {
            Toast.makeText(this, R.string.toastNoBoardInfo, Toast.LENGTH_SHORT).show();
            Util.disconnectWifiNetwork(mContext, netId);
            Util.goToHomeScreen(ThreadOnboardingViaWifiActivity.this, mCallingActivity);
        }
    }
}
