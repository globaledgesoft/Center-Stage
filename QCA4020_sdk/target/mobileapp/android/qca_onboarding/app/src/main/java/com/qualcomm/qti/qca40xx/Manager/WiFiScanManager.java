/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package com.qualcomm.qti.qca40xx.Manager;

import android.app.Activity;
import android.content.Context;
import android.content.IntentFilter;
import android.net.wifi.ScanResult;
import android.net.wifi.WifiManager;
import android.support.v4.app.Fragment;

import com.qualcomm.qti.qca40xx.Interface.IWiFiDeviceList;
import com.qualcomm.qti.qca40xx.Receiver.WiFiScanBroadcastReceiver;
import com.qualcomm.qti.qca40xx.Util.MainApplication;

import java.util.List;

/**
 * WiFiScanManager call for initiating Wi-Fi device scanning
 */

public class WiFiScanManager {

    WiFiScanBroadcastReceiver wiFiScanBroadcastReceiver;
    List<ScanResult> wifiScanList;
    Activity mContext;
    Context context;
    IWiFiDeviceList mWiFiDeviceList;
    private MainApplication application;
    private Fragment fragment;

    public WiFiScanManager(Activity mContext) {
        this.mContext = mContext;
        context = mContext;
        mWiFiDeviceList = (IWiFiDeviceList) (mContext);
        wiFiScanBroadcastReceiver = new WiFiScanBroadcastReceiver();
        application = (MainApplication) mContext.getApplicationContext();
        mContext.getApplicationContext().registerReceiver(wiFiScanBroadcastReceiver, new IntentFilter(WifiManager.SCAN_RESULTS_AVAILABLE_ACTION));
    }

    public void wifiScan() {

        application.getWifiManager().startScan();
        wifiScanList = application.getWifiManager().getScanResults();
        mWiFiDeviceList.onWifiDeviceList(wifiScanList);
    }

    public void wifiStopScan() {
        try {
            mContext.getApplicationContext().unregisterReceiver(wiFiScanBroadcastReceiver);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }


}
