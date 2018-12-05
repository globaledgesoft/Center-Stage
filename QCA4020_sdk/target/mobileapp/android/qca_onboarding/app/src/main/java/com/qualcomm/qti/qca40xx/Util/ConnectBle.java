/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package com.qualcomm.qti.qca40xx.Util;

import android.app.ProgressDialog;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGattService;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.content.SharedPreferences;
import android.os.AsyncTask;
import android.os.IBinder;
import android.widget.Toast;

import com.qualcomm.qti.qca40xx.Activity.OfflineDeviceListActivity;
import com.qualcomm.qti.qca40xx.Activity.ThreadOnboardingViaBleActivity;
import com.qualcomm.qti.qca40xx.Activity.WifiOnboardingViaBleActivity;
import com.qualcomm.qti.qca40xx.Activity.ZigBeeOnBoardingViaBleActivity;
import com.qualcomm.qti.qca40xx.Service.BLEService;

import java.util.ArrayList;
import java.util.List;
import java.util.UUID;

/**
 * This class is for connecting to a BLE Board
 */

public class ConnectBle {

    private String TAG = getClass().getSimpleName();
    public static List<BluetoothGattService> gattServices;
    public static int val;
    boolean wifi, zigbee, thread;
    private BLEService mBLEService;
    private Context context;
    private String mDeviceName;
    private String mDeviceAddress;
    private String menuClick;
    private SharedPreferences sp;
    private boolean mConnected = false;
    private BluetoothDevice device;
    private String mCallingActivity;

    private final ServiceConnection mServiceConnection = new ServiceConnection() {

        @Override
        public void onServiceConnected(ComponentName componentName, IBinder service) {
            mBLEService = ((BLEService.LocalBinder) service).getService();
            BLEGattAttributes.setBLEService(mBLEService);
            if (!mBLEService.initialize()) {
                Logger.e(TAG, "Unable to initialize Bluetooth");
            }
            //  Automatically connects to the device upon successful start-up initialization.
            boolean isConnected = mBLEService.connect(mDeviceAddress);
            try {
                if (isConnected) {
                    new BleConnectionTask().execute();
                    Logger.d(TAG, "Connected");
                } else {
                    Logger.d(TAG, "Not Connected");
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
        }

        @Override
        public void onServiceDisconnected(ComponentName componentName) {
            mBLEService = null;
        }
    };

    private final BroadcastReceiver mGattUpdateReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            final String action = intent.getAction();

            if (BLEService.ACTION_GATT_CONNECTED.equals(action)) {
                mConnected = false;
            } else if (BLEService.ACTION_GATT_DISCONNECTED.equals(action)) {
                mConnected = true;
                gattServices.clear();
                if (mCallingActivity.equals(Constants.QR_SCANNING)) {
                    Util.goToQRScanningScreen(context);
                } else {
                    Util.goToManualScanningScreen(context);
                }

                Toast.makeText(context, "Disconnected", Toast.LENGTH_LONG).show();

            } else if (BLEService.ACTION_GATT_SERVICES_DISCOVERED.equals(action)) {
                availableGattServices();
            }

        }
    };


    public ConnectBle(Context context, String callingActivity , String menuClick) {
        this.context = context;
        this.mCallingActivity = callingActivity;
        this.menuClick = menuClick;
        gattServices = new ArrayList<>();
        sp = context.getSharedPreferences(Constants.BLUETOOTH_DEVICE, Context.MODE_PRIVATE);
        getBleDataFromSP();
        Intent gattServiceIntent = new Intent(context, BLEService.class);
        context.bindService(gattServiceIntent, mServiceConnection, context.BIND_AUTO_CREATE);
        context.registerReceiver(mGattUpdateReceiver, makeGattUpdateIntentFilter());

    }

    private static IntentFilter makeGattUpdateIntentFilter() {
        final IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(BLEService.ACTION_GATT_CONNECTED);
        intentFilter.addAction(BLEService.ACTION_GATT_DISCONNECTED);
        intentFilter.addAction(BLEService.ACTION_GATT_SERVICES_DISCOVERED);
        intentFilter.addAction(BLEService.ACTION_DATA_AVAILABLE);

        return intentFilter;
    }

    private void availableGattServices() {

        gattServices = mBLEService.getSupportedGattServices();
        if (gattServices != null) {
            if(menuClick.equals(Constants.ONBOARD))
            {
                checkForOnboarding(gattServices);
            }
            if(menuClick.equals(Constants.OFFLINE))
            {
                checkForOffline();
            }

        }else {
            Util.checkForBleDeviceToUnpair(mDeviceAddress);
            Util.bleDisconnection(context, mCallingActivity);

        }
    }

    private void checkForOnboarding(List<BluetoothGattService> gattServices)
    {
        int i;
        ArrayList<UUID> availableUuid = new ArrayList<UUID>();
        availableUuid.add(Constants.GATT_WIFI_SERVICE1);
        availableUuid.add(Constants.GATT_ZIGBEE_SERVICE1);
        availableUuid.add(Constants.GATT_THREAD_SERVICE1);
        availableUuid.add(Constants.GATT_OFFLINE_SERVICE);
        for (i = 0; i < gattServices.size(); i++) {
            if (availableUuid.get(0).equals(gattServices.get(i).getUuid())) {
                wifi = true;
            }
            if (availableUuid.get(1).equals(gattServices.get(i).getUuid())) {
                zigbee = true;
            }
            if (availableUuid.get(2).equals(gattServices.get(i).getUuid())) {
                thread = true;
            }
        }
            if (wifi == true && zigbee == false && thread == false) {
            BleServiceCharecteristic.getInstance().readBleBoardStatus();
            mConnected = true;
            Util.gotoScreen(context, WifiOnboardingViaBleActivity.class, mCallingActivity);
        } else if (wifi == false && zigbee == true && thread == false) {
            String status = BleServiceCharecteristic.getInstance().readZigbeeBoardStatus();
            Logger.d("ZigBeeOnBoardingViaBleActivity", " Zigbee -> status1 " + status);
            mConnected = true;
            Util.gotoScreen(context, ZigBeeOnBoardingViaBleActivity.class, mCallingActivity);
        } else if (wifi == false && zigbee == false && thread == true) {
            BleServiceCharecteristic.getInstance().readThreadBoardStatus();
            mConnected = true;
            Util.gotoScreen(context, ThreadOnboardingViaBleActivity.class, mCallingActivity);
        } else if (wifi == true && zigbee == true && thread == true) {
            BleServiceCharecteristic.getInstance().readBleBoardStatus();
            val = Constants.BLE_TRIPLE_MODE_ONBOARDING;
            mConnected = true;
            Util.gotoScreen(context, WifiOnboardingViaBleActivity.class, mCallingActivity);
        } else if (wifi == true && zigbee == true && thread == false) {
            BleServiceCharecteristic.getInstance().readBleBoardStatus();
            val = Constants.BLE_WIFI_ZIGBEE_MODE_ONBOARDING;
            mConnected = true;
            Util.gotoScreen(context, WifiOnboardingViaBleActivity.class, mCallingActivity);
        } else if (wifi == true && thread == true && zigbee == false) {
            BleServiceCharecteristic.getInstance().readBleBoardStatus();
            val = Constants.BLE_WIFI_THREAD_MODE_ONBOARDING;
            mConnected = true;
            Util.gotoScreen(context, WifiOnboardingViaBleActivity.class, mCallingActivity);
        } else if (thread == true && zigbee == true && wifi == false) {
            BleServiceCharecteristic.getInstance().readZigbeeBoardStatus();
            val = 4;
            mConnected = true;
            Util.gotoScreen(context, ZigBeeOnBoardingViaBleActivity.class, mCallingActivity);
        }
    }
    private void checkForOffline()
    {
        BleServiceCharecteristic.getInstance().notifyBreachData();
            Intent intent=new Intent(context.getApplicationContext(), OfflineDeviceListActivity.class);
            Logger.d(TAG,"main device name.."+mDeviceName);
            intent.putExtra(Constants.MAIN_DEVICE_NAME,mDeviceName);
            context.startActivity(intent);
    }

    private void getBleDataFromSP() {
        mDeviceName = sp.getString(Constants.BLE_DEVICE_NAME, null);
        mDeviceAddress = sp.getString(Constants.BLE_DEVICE_ADDRESS, null);
    }

    private class BleConnectionTask extends AsyncTask<Void, Void, Boolean> {
        private ProgressDialog dialog;


        @Override
        protected void onPreExecute() {
            super.onPreExecute();
          /*  dialog = new ProgressDialog(context);
            dialog.setMessage(Constants.DEFAULT_DIALOG);
            dialog.setCancelable(false);
            dialog.show();*/


        }

        protected Boolean doInBackground(Void... params) {
            if (mBLEService != null) {
                while (!mConnected) {
                    //Logger.d(TAG, "waiting for services");
                }

                Logger.d(TAG, "found service");

            }
            return true;
        }

        protected void onPostExecute(Boolean hasActiveConnection) {
          /*  if (dialog.isShowing()) {
                dialog.dismiss();
            }*/
        }
    }
}
