/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package com.qualcomm.qti.qca40xx.Manager;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothManager;
import android.bluetooth.le.BluetoothLeScanner;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanResult;
import android.content.Context;
import android.os.AsyncTask;
import android.os.Handler;
import android.support.v4.app.Fragment;
import android.widget.Toast;

import com.qualcomm.qti.qca40xx.Interface.IBleDeviceList;

import java.util.ArrayList;
import java.util.List;

/**
 * BLEScanManager call for initiating BLE device scanning
 */

public class BLEScanManager {
    private BluetoothLeScanner btScanner;
    private BluetoothManager btManager;
    private BluetoothAdapter btAdapter;
    private Context mContext;
    private IBleDeviceList mBleDeviceList;
    private Fragment fragment;
    private List<ScanResult> bleData;
    private ScanCallback leScanCallback = new ScanCallback() {
        @Override
        public void onScanResult(int callbackType, ScanResult result) {
            bleData.add(result);
        }


        @Override
        public void onScanFailed(int errorCode) {
            super.onScanFailed(errorCode);
            if (errorCode == 2) {
                Toast.makeText(mContext, "error " + errorCode, Toast.LENGTH_LONG).show();
                final BluetoothAdapter mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
                if (mBluetoothAdapter.isEnabled()) {
                    mBluetoothAdapter.disable();
                    Handler handler = new Handler();
                    handler.postDelayed(new Runnable() {
                        @Override
                        public void run() {
                            mBluetoothAdapter.enable();
                        }
                    }, 1000);
                }
            }
        }
    };

    public BLEScanManager(Context context) {
        this.mContext = context;
        btManager = (BluetoothManager) mContext.getSystemService(Context.BLUETOOTH_SERVICE);
        btAdapter = btManager.getAdapter();
        btScanner = btAdapter.getBluetoothLeScanner();
        mBleDeviceList = (IBleDeviceList) (mContext);
        bleData = new ArrayList<ScanResult>();


    }

    public BLEScanManager(Fragment mfragment,Context context) {
        this.fragment=mfragment;
        this.mContext = context;
        btManager = (BluetoothManager) mContext.getSystemService(Context.BLUETOOTH_SERVICE);
        btAdapter = btManager.getAdapter();
        btScanner = btAdapter.getBluetoothLeScanner();
        mBleDeviceList = (IBleDeviceList) (fragment);
        bleData = new ArrayList<ScanResult>();


    }

    public void startScanning() {
        Handler handler = new Handler();
        handler.postDelayed(new Runnable() {
            public void run() {
                mBleDeviceList.onBleDeviceList(bleData);
                btScanner.stopScan(leScanCallback);
            }
        }, 5000);
        btScanner.startScan(leScanCallback);
    }

    public void stopScanning() {
        AsyncTask.execute(new Runnable() {
            @Override
            public void run() {
                btScanner.stopScan(leScanCallback);
            }
        });
    }

}
