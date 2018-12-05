/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package com.qualcomm.qti.qca40xx.Util;

import android.app.ProgressDialog;
import android.content.Context;
import android.content.SharedPreferences;
import android.net.DhcpInfo;
import android.net.wifi.ScanResult;
import android.net.wifi.WifiConfiguration;
import android.net.wifi.WifiInfo;
import android.os.AsyncTask;
import android.os.Handler;
import android.widget.Toast;

import com.qualcomm.qti.qca40xx.Activity.WifiOnboardingViaWifiActivity;
import com.qualcomm.qti.qca40xx.Interface.IQRScan;

import java.math.BigInteger;
import java.net.InetAddress;
import java.nio.ByteOrder;

/**
 * This class is for connecting a Wi-Fi Board
 */

public class ConnectAP {
    private Context mContext;
    private int wifiState;
    private SharedPreferences sharedPreferences;
    private String wifiName, wifiAddress;
    private int netId;
    private IQRScan iqrScan;
    private MainApplication application;
    private WifiConfiguration wc;
    private String mCallingActivity;

    public ConnectAP(Context mContext, IQRScan qrScan, String callingActivity) {
        wc = new WifiConfiguration();
        this.mContext = mContext;
        this.iqrScan = qrScan;
        this.mCallingActivity = callingActivity;
        application = (MainApplication) mContext.getApplicationContext();

    }


    public int connectToWifi(ScanResult scanResult, String ssid, String password) {
        try {
            String networkSSID = ssid;
            String networkPass = password;
            if (scanResult.capabilities.toUpperCase().contains("WEP")) {

                wc.wepKeys[0] = "\"" + networkPass + "\"";
                wc.BSSID = scanResult.BSSID;
                wc.status = WifiConfiguration.Status.ENABLED;
                wc.wepTxKeyIndex = 0;
                wc.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.WEP40);


                netId = application.getWifiManager().addNetwork(wc);
                application.getWifiManager().disconnect();
                application.getWifiManager().enableNetwork(netId, true);
                application.getWifiManager().reconnect();
                wifiState = application.getWifiManager().getWifiState();
                NetworkUtilTask networkUtilTask = new NetworkUtilTask(mContext, scanResult);
                networkUtilTask.execute();

            } else if (scanResult.capabilities.toUpperCase().contains("WPA")) {

                wc.SSID = "\"" + networkSSID + "\"";
                wc.preSharedKey = "\"" + networkPass + "\"";
                wc.BSSID = scanResult.BSSID;
                wc.status = WifiConfiguration.Status.ENABLED;
                wc.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.TKIP);
                wc.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.CCMP);
                wc.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.WPA_PSK);
                wc.allowedPairwiseCiphers.set(WifiConfiguration.PairwiseCipher.TKIP);
                wc.allowedPairwiseCiphers.set(WifiConfiguration.PairwiseCipher.CCMP);
                wc.allowedProtocols.set(WifiConfiguration.Protocol.RSN);

                netId = application.getWifiManager().addNetwork(wc);
                application.getWifiManager().disconnect();
                application.getWifiManager().enableNetwork(netId, true);
                application.getWifiManager().reconnect();
                wifiState = application.getWifiManager().getWifiState();
                NetworkUtilTask networkUtilTask = new NetworkUtilTask(mContext, scanResult);
                networkUtilTask.execute();

            } else {
                wc.SSID = "\"" + networkSSID + "\"";
                wc.hiddenSSID = true;
                wc.priority = 0xBADBAD;
                wc.BSSID = scanResult.BSSID;
                wc.status = WifiConfiguration.Status.ENABLED;
                wc.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.NONE);

                netId = application.getWifiManager().addNetwork(wc);
                application.getWifiManager().disconnect();
                application.getWifiManager().enableNetwork(netId, true);
                application.getWifiManager().reconnect();
                wifiState = application.getWifiManager().getWifiState();
                NetworkUtilTask networkUtilTask = new NetworkUtilTask(mContext, scanResult);
                networkUtilTask.execute();
            }


        } catch (Exception e) {
            e.printStackTrace();
        }
        return wifiState;
    }

    public void saveIPAddress(String ip) {
        sharedPreferences = mContext.getSharedPreferences(Constants.IP_ADDRESS, Context.MODE_PRIVATE);
        SharedPreferences.Editor editor = sharedPreferences.edit();
        editor.putString(Constants.IP, ip);
        editor.putString(Constants.DEVICE_NAME, wifiName);
        editor.putString(Constants.MAC_ADDRESS, wifiAddress);
        if (netId != -1) {
            editor.putInt(Constants.NETWORK_ID, netId);
        }
        editor.commit();
    }

    private class NetworkUtilTask extends AsyncTask<Void, Void, Boolean> {
        Context context;
        ScanResult wifiDevice;
        private ProgressDialog dialog;

        public NetworkUtilTask(Context context, ScanResult wifiDevice) {
            this.context = context;
            this.wifiDevice = wifiDevice;
        }

        @Override
        protected void onPreExecute() {
            super.onPreExecute();
            dialog = new ProgressDialog(mContext);
            dialog.setMessage("Connecting to " + wifiDevice.SSID + " AP");
            dialog.setCancelable(false);
            dialog.show();

        }

        protected Boolean doInBackground(Void... params) {
            return true;
        }

        protected void onPostExecute(Boolean hasActiveConnection) {

            Handler handler2 = new Handler();
            handler2.postDelayed(new Runnable() {
                public void run() {

                    if (dialog.isShowing()) {
                        dialog.dismiss();
                    }
                    WifiInfo info = application.getWifiManager().getConnectionInfo();
                    DhcpInfo d = application.getWifiManager().getDhcpInfo();
                    int gatway = d.gateway;
                    wifiName = info.getSSID().replace("\"", "");
                    wifiAddress = info.getBSSID();

                    try {

                        gatway = (ByteOrder.nativeOrder().equals(ByteOrder.LITTLE_ENDIAN)) ? Integer.reverseBytes(gatway) : gatway;
                        byte[] bytes = BigInteger.valueOf(gatway).toByteArray();
                        InetAddress address = InetAddress.getByAddress(bytes);
                        saveIPAddress(address.toString().substring(1));

                    } catch (Exception e) {
                        e.printStackTrace();
                    }

                    if (wifiDevice.BSSID.equals(info.getBSSID())) {
                        Toast.makeText(mContext, "Connected to " + wifiDevice.SSID, Toast.LENGTH_SHORT).show();
                        Logger.d("log", wifiDevice.BSSID.equals(info.getBSSID()) + "");
                        Util.gotoScreen(mContext, WifiOnboardingViaWifiActivity.class, mCallingActivity);
                    } else {
                        Logger.d("log", wifiDevice.BSSID.equals(info.getBSSID()) + "");
                        Toast.makeText(mContext, "Could not connect to AP. Please try again!", Toast.LENGTH_SHORT).show();
                        Util.goToHomeScreen(mContext, mCallingActivity);

                    }
                }
            }, 10000);
        }
    }
}


