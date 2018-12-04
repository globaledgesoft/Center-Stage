/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package com.qualcomm.qti.qca40xx.Activity;

import android.app.ProgressDialog;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothManager;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.net.wifi.ScanResult;
import android.os.AsyncTask;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.support.v7.app.AlertDialog;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.support.v7.widget.Toolbar;
import android.text.InputType;
import android.text.TextUtils;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.inputmethod.InputMethodManager;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import com.qualcomm.qti.qca40xx.Adapter.BleWifiAdapter;
import com.qualcomm.qti.qca40xx.Interface.IBleDeviceList;
import com.qualcomm.qti.qca40xx.Interface.IQRScan;
import com.qualcomm.qti.qca40xx.Interface.IWiFiDevice;
import com.qualcomm.qti.qca40xx.Interface.IWiFiDeviceList;
import com.qualcomm.qti.qca40xx.Manager.BLEScanManager;
import com.qualcomm.qti.qca40xx.Manager.WiFiScanManager;
import com.qualcomm.qti.qca40xx.Util.ConnectAP;
import com.qualcomm.qti.qca40xx.Util.Constants;
import com.qualcomm.qti.qca40xx.Util.Logger;
import com.qualcomm.qti.qca40xx.Util.MainApplication;
import com.qualcomm.qti.qca40xx.Util.RuntimePermissionManager;
import com.qualcomm.qti.qca40xx.R;
import com.qualcomm.qti.qca40xx.Util.Util;

import java.util.ArrayList;
import java.util.List;
import java.util.Timer;
import java.util.TimerTask;

/**
 * BleWifiListActivity is an Activity class for Manual Scan for BLE and Wi-Fi devices.
 * This class displays list of the devices scanned
 */

public class BleWifiListActivity extends AppCompatActivity implements IBleDeviceList, IWiFiDeviceList, IQRScan, IWiFiDevice {
    private String TAG = getClass().getSimpleName();
    private boolean mScanning;
    private WiFiScanManager wiFiScanManager;
    private RecyclerView recyclerView;
    private BleWifiAdapter adapter;
    private TextView emptyLV;
    private ProgressDialog dialog;
    private Context mContext;
    private List<Object> mWifiBleFilteredList;
    private Toolbar toolbar;
    private List<BluetoothDevice> noBleDuplicate;
    private BluetoothAdapter mBluetoothAdapter;
    private BluetoothManager bluetoothManager;
    private MainApplication application;
    Timer timer;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.ble_wifi_list_layout);
        initialize();
        Logger.d(TAG, "onCreate()");

        if (application.getWifiManager().setWifiEnabled(true)) {
            RuntimePermissionManager permissions = RuntimePermissionManager.getInstance(this);

            if (Build.VERSION.SDK_INT >= 23) {
                // Marshmallow+
                try {
                    permissions.externalStorageCheck();
                    permissions.verifyLocationEnabled();
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
            Logger.getInstance();
            int currentapiVersion = Build.VERSION.SDK_INT;
            if (currentapiVersion >= Build.VERSION_CODES.M) {
                if (permissions.checkPermission()) {
                    try {
                        if (!mBluetoothAdapter.isEnabled()) {
                            Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
                            startActivityForResult(enableBtIntent, Constants.REQUEST_PERMISSIONS);
                        } else {
                            Logger.d(TAG, "Android version is >= Marshmallow");
                            mWifiBleFilteredList.clear();
                            noBleDuplicate.clear();
                            new ScanDeviceTask().executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR);
                        }
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                } else {
                    permissions.requestPermission();
                }
            } else {
                Logger.d(TAG, "Android version is below Marshmallow");
            }
        } else {
            Toast.makeText(getApplicationContext(), R.string.toastTurnOnWifi, Toast.LENGTH_LONG).show();
        }
    }

    public void initialize() {
        mContext = this.getApplicationContext();
        toolbar = (Toolbar) findViewById(R.id.tool_bar);
        toolbar.setTitle(Constants.TITLE_ONBOARDING_DETAILS);
        setSupportActionBar(toolbar);
        noBleDuplicate = new ArrayList<>();
        wiFiScanManager = new WiFiScanManager(BleWifiListActivity.this);
        adapter = new BleWifiAdapter();
        emptyLV = (TextView) findViewById(R.id.emptyDataTextView);
        recyclerView = (RecyclerView) findViewById(R.id.recycler_view_scan);
        recyclerView.setHasFixedSize(true);
        RecyclerView.LayoutManager layoutManager = new LinearLayoutManager(BleWifiListActivity.this);
        recyclerView.setLayoutManager(layoutManager);
        recyclerView.setLayoutManager(layoutManager);
        bluetoothManager =
                (BluetoothManager) getSystemService(Context.BLUETOOTH_SERVICE);
        mBluetoothAdapter = bluetoothManager.getAdapter();
        mWifiBleFilteredList = new ArrayList<Object>();
        application = (MainApplication) getApplicationContext();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (Constants.BLE_FLAG == true) {
            new BLEScanManager(BleWifiListActivity.this).stopScanning();
        }
        if (Constants.WIFI_FLAG == true) {
            wiFiScanManager.wifiStopScan();
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.scan_device_menu, menu);

        menu.clear();
        getMenuInflater().inflate(R.menu.scan_device_menu, menu);
        if (!mScanning) {
            menu.findItem(R.id.menu_stop).setVisible(false);
            menu.findItem(R.id.menu_scan).setVisible(true);
        } else {
            menu.findItem(R.id.menu_stop).setVisible(true);
            menu.findItem(R.id.menu_scan).setVisible(false);

        }
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case R.id.common_menu_settings:
                Intent settingIntent = new Intent(BleWifiListActivity.this, SettingActivity.class);
                startActivity(settingIntent);
                break;
            case R.id.menu_scan:
                try {
                    if (application.getWifiManager().setWifiEnabled(true)) {
                        if (!mBluetoothAdapter.isEnabled()) {
                            Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
                            startActivityForResult(enableBtIntent, Constants.REQUEST_PERMISSIONS);
                        } else {
                            mWifiBleFilteredList.clear();
                            noBleDuplicate.clear();
                            mScanning = true;
                            try {
                                new ScanDeviceTask().executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR);
                            } catch (Exception e) {
                                e.printStackTrace();
                            }
                        }
                    } else {
                        Toast.makeText(getApplicationContext(), R.string.toastTurnOnWifi, Toast.LENGTH_LONG).show();

                    }
                } catch (Exception e) {
                    Logger.d(TAG, "Exception in scan menu clicked() -> " + e);
                }
                break;

            case R.id.menu_stop:
                mScanning = false;
                if (Constants.BLE_FLAG == true) {
                    new BLEScanManager(BleWifiListActivity.this).stopScanning();
                }
                if (Constants.WIFI_FLAG == true) {
                    wiFiScanManager.wifiStopScan();
                }
                break;
            case R.id.menu_qr_read:
                mScanning = false;
                Intent intent = new Intent(mContext, QRCodeScanningActivity.class);
                intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TASK);
                mContext.startActivity(intent);
                break;
        }
        return mScanning;
    }

    @Override
    protected void onStop() {
        super.onStop();
        if (dialog != null) {
            dialog.dismiss();
            dialog = null;
        }
    }

    @Override
    public void onWifiDeviceList(List<ScanResult> data) {
        try {
            if (data.size() != 0) {
                for (int i = 0; i < data.size(); i++) {

                    if (data.get(i).SSID.contains(Constants.WIFI_BLE_FILTER_KEYWORD)) {
                        mWifiBleFilteredList.add(data.get(i));
                    }
                }
            }
        } catch (Exception e) {
            Logger.d(TAG, " Exception in onWifiDeviceList() -> " + e);
        }
    }

    @Override
    public void onBleDeviceList(List<android.bluetooth.le.ScanResult> data) {
        try {

            if (data.size() != 0) {
                for (int i = 0; i < data.size(); i++) {
                    if (data.get(i).getDevice().getName() != null) {
                        if (data.get(i).getDevice().getName().contains(Constants.WIFI_BLE_FILTER_KEYWORD)) {
                            if (!noBleDuplicate.contains(data.get(i).getDevice())) {
                                noBleDuplicate.add(data.get(i).getDevice());
                                mWifiBleFilteredList.add(data.get(i));
                            }
                        }
                    }
                }
                adapter.notifyDataSetChanged();
            }

            if (mWifiBleFilteredList.size() != 0) {
                adapter.addWifiBle(mWifiBleFilteredList, BleWifiListActivity.this, BleWifiListActivity.this, this);
                recyclerView.setVisibility(View.VISIBLE);
                recyclerView.setAdapter(adapter);
                emptyLV.setVisibility(View.GONE);
            } else {
                recyclerView.setVisibility(View.GONE);
                emptyLV.setVisibility(View.VISIBLE);
            }
            mScanning = false;
            invalidateOptionsMenu();
            if (dialog != null) {
                dialog.dismiss();
                dialog = null;
            } else {
                Intent intent = new Intent(BleWifiListActivity.this, BleWifiListActivity.class);
               // intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TASK);
                startActivity(intent);
                //finish();
            }

        } catch (Exception e) {
            finish();
            Logger.d(TAG, "Exception in onBleDeviceList() dialog not closed -> " + e);
        }

    }


    @Override
    public void callReScan() {
    }

    @Override
    public void wifiDeviceSelected(ScanResult wifiDeviceSelected) {
        inputDevicePasswordDialog(wifiDeviceSelected);
    }


    /**
     * Method to display a dialog for the password input from the user to connect to the Secured Qualcomm Board
     *
     * @param wifiDeviceSelected
     */
    private void inputDevicePasswordDialog(final ScanResult wifiDeviceSelected) {
        final AlertDialog.Builder dialogPassword = new AlertDialog.Builder(this);
        LayoutInflater inflater = this.getLayoutInflater();
        final View dialogView = inflater.inflate(R.layout.ap_password, null);
        dialogPassword.setView(dialogView);
        dialogPassword.setCancelable(false);
        final EditText editPassword = (EditText) dialogView.findViewById(R.id.edit_password);
        final CheckBox chkPassword = (CheckBox) dialogView.findViewById(R.id.chk_paswd_device);

        chkPassword.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean isChecked) {
                if (!isChecked) {
                    editPassword.setInputType(129);
                } else {
                    editPassword.setInputType(InputType.TYPE_TEXT_VARIATION_PASSWORD);
                }
            }
        });

        dialogPassword.setTitle(getResources().getString(R.string.ap_password_title) + wifiDeviceSelected.SSID);
        dialogPassword.setPositiveButton(getResources().getString(R.string.ap_connect), new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface paramDialogInterface, int paramInt) {
                String passwordText = editPassword.getText().toString();
                if (!TextUtils.isEmpty(passwordText)) {
                    if (passwordText.length() > 8 && passwordText.length() <= 64) {
                        try {
                            InputMethodManager inputManager = (InputMethodManager) getApplicationContext().getSystemService(Context.INPUT_METHOD_SERVICE);
                            inputManager.hideSoftInputFromWindow(editPassword.getWindowToken(), InputMethodManager.RESULT_UNCHANGED_SHOWN);
                        } catch (Exception e) {
                            Logger.d(TAG, "Keyboard hiding exception");
                        }
                        new ConnectAP(BleWifiListActivity.this, null, Constants.MANUAL_SCANNING).connectToWifi(wifiDeviceSelected, wifiDeviceSelected.SSID, passwordText);
                    } else {
                        Toast.makeText(getApplicationContext(), getString(R.string.ap_password_invalid), Toast.LENGTH_SHORT).show();
                    }
                } else {
                    Toast.makeText(getApplicationContext(), getString(R.string.ap_password_empty), Toast.LENGTH_SHORT).show();
                }
            }
        });
        dialogPassword.setNegativeButton(getString(R.string.buttonCancel), new DialogInterface.OnClickListener() {

            @Override
            public void onClick(DialogInterface paramDialogInterface, int paramInt) {
                paramDialogInterface.dismiss();
            }
        });
        dialogPassword.show();
    }


    /**
     * ScanDeviceTask is an AsyncTask for Wi-Fi devices scanning.
     */
    private class ScanDeviceTask extends AsyncTask<Void, Void, Void> {

        @Override
        protected void onPreExecute() {
            try {
                super.onPreExecute();
                adapter.clear();
                dialog = new ProgressDialog(BleWifiListActivity.this);
                dialog.setMessage(Constants.SCAN_DEVICE_DIALOG);
                dialog.setCancelable(false);
                dialog.show();

            } catch (RuntimeException e) {
                e.printStackTrace();
            } catch (Exception e) {
                e.printStackTrace();
                Logger.d(TAG, "Exception in onPreExecute() ->" + e);
            }
        }

        @Override
        protected Void doInBackground(Void... params) {
            try {
                if (Constants.WIFI_FLAG == true) {
                    wiFiScanManager.wifiScan();
                }
            }catch (Exception e)
            {
                e.printStackTrace();
                if(dialog != null)
                {
                    if(dialog.isShowing())
                        dialog.dismiss();
                }
                Logger.d(TAG, "Exception in doInBackground() ->" + e);
            }
            return null;
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            super.onPostExecute(aVoid);
            try {
                if (Constants.BLE_FLAG == true) {
                    new BLEScanManager(BleWifiListActivity.this).startScanning();
                }
            }catch (Exception e)
            {
                e.printStackTrace();
                if(dialog != null)
                {
                    if(dialog.isShowing())
                        dialog.dismiss();
                }
                Logger.d(TAG, "Exception in onPostExecute() ->" + e);
            }
        }
    }

}