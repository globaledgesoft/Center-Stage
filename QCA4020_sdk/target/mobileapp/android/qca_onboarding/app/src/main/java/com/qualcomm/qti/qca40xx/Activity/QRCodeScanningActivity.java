/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package com.qualcomm.qti.qca40xx.Activity;

import android.app.ProgressDialog;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.net.wifi.ScanResult;
import android.net.wifi.WifiConfiguration;
import android.os.AsyncTask;
import android.os.Build;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.Toast;

import com.google.zxing.Result;
import com.qualcomm.qti.qca40xx.Interface.IBleDeviceList;
import com.qualcomm.qti.qca40xx.Interface.IQRScan;
import com.qualcomm.qti.qca40xx.Interface.IWiFiDeviceList;
import com.qualcomm.qti.qca40xx.Manager.BLEScanManager;
import com.qualcomm.qti.qca40xx.Manager.WiFiScanManager;
import com.qualcomm.qti.qca40xx.Util.ConnectAP;
import com.qualcomm.qti.qca40xx.Util.ConnectBle;
import com.qualcomm.qti.qca40xx.Util.Constants;
import com.qualcomm.qti.qca40xx.Util.Logger;
import com.qualcomm.qti.qca40xx.Util.MainApplication;
import com.qualcomm.qti.qca40xx.Util.RuntimePermissionManager;
import com.qualcomm.qti.qca40xx.Util.Util;
import com.qualcomm.qti.qca40xx.R;

import java.lang.reflect.Method;
import java.util.List;
import java.util.Set;

import me.dm7.barcodescanner.zxing.ZXingScannerView;


/**
 * QRCodeScanningActivity is an Activity class for QR code scanning to be read from the Board which is in the format
 * <p>
 * For BLE Board    - Ble;MacAddress:Passkey
 * For Wi-Fi Board  - Wifi;MacAddress;BoardName;Password
 */
public class QRCodeScanningActivity extends AppCompatActivity implements ZXingScannerView.ResultHandler, IBleDeviceList, IWiFiDeviceList, IQRScan {

    private String TAG = getClass().getSimpleName();
    private ZXingScannerView mScannerView;
    private ScanResult wifiDevice;
    private BLEScanManager bleScanManager;
    private WiFiScanManager wiFiScanManager;
    private String deviceName, macAddress, ssid, password, passkey;
    private ProgressDialog dialog;
    private BluetoothDevice bluetoothDevice;
    private ProgressDialog mProgressDialog;
    BroadcastReceiver receiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();

            if (BluetoothDevice.ACTION_PAIRING_REQUEST.equals(action)) {
                BluetoothDevice mBluetoothDevice = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
                if (mBluetoothDevice != null) {

                    try {
                        byte[] pin = (byte[]) BluetoothDevice.class.getMethod(Constants.CONVERT_PIN_TO_BYTES, String.class).invoke(BluetoothDevice.class, passkey);
                        Method m = mBluetoothDevice.getClass().getMethod(Constants.SET_PIN, byte[].class);
                        m.invoke(mBluetoothDevice, pin);
                    } catch (Exception e) {

                        e.printStackTrace();

                    }
                }
            }

            if (action.equals(BluetoothDevice.ACTION_BOND_STATE_CHANGED)) {
                int state = intent.getIntExtra(BluetoothDevice.EXTRA_BOND_STATE, -1);

                if (state < 0) {
                    Toast.makeText(QRCodeScanningActivity.this, "Please try again!", Toast.LENGTH_LONG).show();
                    //we should never get here
                } else if (state == BluetoothDevice.BOND_BONDING) {
                    //bonding process is still working
                    //essentially this means that the Confirmation Dialog is still visible
                    showProgress(true);

                    Logger.d(TAG, "Start Pairing...");


                } else if (state == BluetoothDevice.BOND_BONDED) {
                    //bonding process was successful
                    //also means that the user pressed OK on the Dialog

                    Logger.d(TAG, "Paired");
                    showProgress(false);
                    new ConnectBle(QRCodeScanningActivity.this, Constants.QR_SCANNING, Constants.ONBOARD);
                } else if (state == BluetoothDevice.BOND_NONE) {
                    Logger.d(TAG, "not Paired");
                    showProgress(false);
                    mScannerView.resumeCameraPreview(QRCodeScanningActivity.this);
                    //bonding process failed
                    //which also means that the user pressed CANCEL on the Dialog
                }
            }
        }
    };
    private Toolbar toolbar;
    private Context mContext;
    private MainApplication application;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_qrcode_scanning);
        initialize();
        RuntimePermissionManager permissions = RuntimePermissionManager.getInstance(this);
        if (Build.VERSION.SDK_INT >= 23) {
            // Marshmallow+
            if (!permissions.checkPermission()) {
                permissions.requestPermission();
            }
            permissions.verifyLocationEnabled();
        }

    }

    private void initialize() {
        toolbar = (Toolbar) findViewById(R.id.tool_bar);
        toolbar.setTitle(Constants.TITLE_ONBOARDING_DETAILS);
        setSupportActionBar(toolbar);
        Logger.getInstance();
        mContext = this;
        bleScanManager = new BLEScanManager(this);
        wiFiScanManager = new WiFiScanManager(this);
        dialog = new ProgressDialog(QRCodeScanningActivity.this);
        mScannerView = (ZXingScannerView) findViewById(R.id.zxingView);
        application = (MainApplication) getApplicationContext();
    }

    @Override
    public void onResume() {
        super.onResume();

        mScannerView.setResultHandler(this);
        mScannerView.startCamera();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.qr_activity_menu, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case R.id.menu_device_list:
                try {
                    Intent intent = new Intent(mContext, BleWifiListActivity.class);
                    intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TASK);
                    mContext.startActivity(intent);
                } catch (Exception e) {
                    e.printStackTrace();
                }
                break;
            case R.id.common_menu_settings:
                Intent settingIntent = new Intent(QRCodeScanningActivity.this, SettingActivity.class);
                startActivity(settingIntent);
                break;
        }
        return true;
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        mScannerView.stopCamera();
    }

    @Override
    public void onPointerCaptureChanged(boolean hasCapture) {

    }

    @Override
    public void handleResult(Result rawResult) {
        final String result = rawResult.getText();
        Logger.d(TAG, "QR Result : + " + result);
        splitDataByDelimiter(result);

    }

    public void splitDataByDelimiter(String result) {
        try {
            String[] data = result.split(Constants.SPLIT_STRING);
            if (data.length > 2) {

                if (data[0].equalsIgnoreCase(Constants.WIFI)) {
                    macAddress = data[1];
                    ssid = data[2];
                    password = data[3];
                    wiFiScanManager.wifiScan();
                    Logger.d(TAG, "Inside wifi: = " + macAddress + "," + ssid + "," + password);
                }
                if (data[0].equalsIgnoreCase(Constants.BLE)) {
                    macAddress = data[1];
                    passkey = data[2];
                    new ScanDeviceTask().execute();
                }
            } else {
                Toast.makeText(QRCodeScanningActivity.this, R.string.toastWrongQRFormat, Toast.LENGTH_SHORT).show();
                mScannerView.resumeCameraPreview(this);
            }
        } catch (Exception e) {
            Logger.d(TAG, " Exception in splitDataByDelimiter() -> " + e);
        }

    }

    @Override
    public void onWifiDeviceList(List<ScanResult> data) {
        boolean isDevicefound = false;
        if (data.size() != 0) {
            for (int i = 0; i < data.size(); i++) {

                if (data.get(i).BSSID.equalsIgnoreCase(macAddress)) {
                    wifiDevice = data.get(i);
                    deviceName = data.get(i).SSID;
                    checkWifiConfiguration(data.get(i));
                    isDevicefound = true;
                    Logger.d(TAG, "Selected wifi device = " + data.get(i));
                    new ConnectAP(QRCodeScanningActivity.this, this, Constants.QR_SCANNING).connectToWifi(wifiDevice, ssid, password);
                }
            }

            if (!isDevicefound) {
                Toast.makeText(QRCodeScanningActivity.this, R.string.toastWifiDeviceNotFound, Toast.LENGTH_SHORT).show();
                mScannerView.resumeCameraPreview(this);
            }
        }

    }

    @Override
    public void onBleDeviceList(List<android.bluetooth.le.ScanResult> data) {
        if (data.size() != 0) {

            for (int i = 0; i < data.size(); i++) {
                if (deviceName == null) {
                    if (data.get(i).getDevice().getAddress().equalsIgnoreCase(macAddress)) {
                        deviceName = data.get(i).getDevice().getName();
                        bluetoothDevice = data.get(i).getDevice();
                        break;
                    }

                }
            }
            if (dialog != null) {
                bleScanManager.stopScanning();
                dialog.dismiss();
            }
            if (bluetoothDevice != null) {

                BluetoothAdapter mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
                Set<BluetoothDevice> pairedDevices = mBluetoothAdapter.getBondedDevices();

                for (BluetoothDevice bt : pairedDevices) {
                    if (bt.getAddress().contains(bluetoothDevice.getAddress())) {
                        Util.unParingDevice(bt);
                    }
                }
                saveBleDevice(bluetoothDevice);
                if (bluetoothDevice.getBondState() == BluetoothDevice.BOND_BONDED) {
                    new ConnectBle(QRCodeScanningActivity.this, Constants.QR_SCANNING, Constants.ONBOARD);
                } else {
                    pairDevice(bluetoothDevice);
                }


            } else {
                Toast.makeText(QRCodeScanningActivity.this, R.string.toastDeviceNotFound, Toast.LENGTH_SHORT).show();
                finish();
                startActivity(getIntent());

            }

        } else {
            if (dialog != null) {
                bleScanManager.stopScanning();
                dialog.dismiss();
            }
            Toast.makeText(QRCodeScanningActivity.this, R.string.toastDeviceNotFound, Toast.LENGTH_LONG).show();
            finish();
            startActivity(getIntent());
        }

    }

    //wifi forget network
    private void checkWifiConfiguration(ScanResult wifiDevice) {
        List<WifiConfiguration> list = application.getWifiManager().getConfiguredNetworks();
        for (WifiConfiguration i : list) {
            String configuredDevice = i.SSID;
            String connectingDvice = wifiDevice.SSID;
            if (configuredDevice.contains("\"")) {
                configuredDevice = configuredDevice.replace("\"", "");
            }
            if (connectingDvice.contains("\"")) {
                connectingDvice = connectingDvice.replace("\"", "");
            }

            if (configuredDevice.equalsIgnoreCase(connectingDvice)) {
                application.getWifiManager().removeNetwork(i.networkId);
                application.getWifiManager().saveConfiguration();
            }
        }
    }

    @Override
    protected void onPause() {
        super.onPause();

    }

    private void pairDevice(BluetoothDevice device) {
        try {
            IntentFilter intentFilter = new IntentFilter();
            intentFilter.addAction(BluetoothDevice.ACTION_PAIRING_REQUEST);
            intentFilter.addAction(BluetoothDevice.ACTION_BOND_STATE_CHANGED);
            registerReceiver(receiver, intentFilter);
            mProgressDialog = new ProgressDialog(QRCodeScanningActivity.this);
            mProgressDialog.setMessage(Constants.PAIR_DIALOG);
            Method m = device.getClass().getMethod(Constants.CREATE_BOND, (Class[]) null);
            m.invoke(device, (Object[]) null);

        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private void showProgress(boolean show) {

        if (show) {
            mProgressDialog.setCancelable(false);
            if (!mProgressDialog.isShowing())
                mProgressDialog.show();
        } else {
            Logger.i(TAG, " " + mProgressDialog.isShowing());
            mProgressDialog.setCancelable(true);
            if (mProgressDialog.isShowing()) {
                mProgressDialog.dismiss();
            }
        }
    }


    private void saveBleDevice(BluetoothDevice device) {

        SharedPreferences sharedPreferences = getSharedPreferences(Constants.BLUETOOTH_DEVICE, Context.MODE_PRIVATE);
        SharedPreferences.Editor editor = sharedPreferences.edit();
        editor.putString(Constants.BLE_DEVICE_NAME, device.getName());
        editor.putString(Constants.BLE_DEVICE_ADDRESS, device.getAddress());
        editor.commit();

    }

    @Override
    public void callReScan() {
        mScannerView.resumeCameraPreview(this);
    }

    private class ScanDeviceTask extends AsyncTask<Void, Void, Void> {


        @Override
        protected void onPreExecute() {
            try {
                super.onPreExecute();
                dialog.setMessage(Constants.QR_DIALOG);
                dialog.setCancelable(false);
                dialog.show();
            } catch (Exception e) {
                e.printStackTrace();
            }

        }

        @Override
        protected Void doInBackground(Void... params) {
            return null;
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            super.onPostExecute(aVoid);

            if (Constants.BLE_FLAG == true) {
                new BLEScanManager(QRCodeScanningActivity.this).startScanning();

            }
        }
    }
}
