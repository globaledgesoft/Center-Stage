/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package com.qualcomm.qti.qca40xx.Util;

import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattService;
import android.content.Context;
import android.os.Handler;
import android.widget.Toast;

import com.qualcomm.qti.qca40xx.R;
import com.qualcomm.qti.qca40xx.Service.BLEService;

import java.util.List;

/**
 * This class is to read and write Gatt Characterstics to the Qualcomm Board.
 */

public class BleServiceCharecteristic {
    static BluetoothGatt mBluetoothGatt;
    private static BLEService mBLEService;
    private static BleServiceCharecteristic mBleServiceCharecteristic = null;
    List<BluetoothGattService> gattServices;
    String ssidTxt;
    Context context;
    private String TAG = getClass().getSimpleName();


    public static BleServiceCharecteristic getInstance() {
        if (mBleServiceCharecteristic == null) {

            mBleServiceCharecteristic = new BleServiceCharecteristic();
        }
        if (BLEGattAttributes.getBLEService() != null) {
            mBLEService = BLEGattAttributes.getBLEService();
            if (mBLEService.getmBluetoothGatt() != null) {
                mBluetoothGatt = mBLEService.getmBluetoothGatt();
            }
        }
        return mBleServiceCharecteristic;
    }


    //wifi onboarding via ble
    public void wifiOnboardingViaBle(String ssid, final String password) {
        ssidTxt = ssid;
        Handler handler = new Handler();
        handler.postDelayed(new Runnable() {
            public void run() {
                writePassword(password);
            }
        }, 4000);

        Handler handler1 = new Handler();
        handler1.postDelayed(new Runnable() {
            public void run() {
                writeSSID(ssidTxt);
            }
        }, 2000);

        Handler handler2 = new Handler();
        handler2.postDelayed(new Runnable() {
            public void run() {
                writeWifiOnboardNotify();
            }
        }, 6000);
    }

    private void writeSSID(String ssid) {
        try {
            if (mBluetoothGatt != null) {
                BluetoothGattCharacteristic characteristicSsid = mBluetoothGatt.getService(Constants.GATT_WIFI_SERVICE1).getCharacteristic(Constants.GATT_WIFI_SSID_CHARECTERISTIC);
                characteristicSsid.setValue(ssid);
                mBLEService.writeCharacteristic(characteristicSsid);
            }
        }catch (Exception e)
        {
            Logger.d(TAG, "Exception in writeSSID() -> " + e);

        }
    }

    private void writePassword(String password) {
        try {
            if (mBluetoothGatt != null) {
                BluetoothGattCharacteristic characteristicSsid = mBluetoothGatt.getService(Constants.GATT_WIFI_SERVICE1).getCharacteristic(Constants.GATT_WIFI_PASSWORD_CHARECTERISTIC);
                characteristicSsid.setValue(password);
                mBLEService.writeCharacteristic(characteristicSsid);
            }
        }catch (Exception e)
        {
            Logger.d(TAG, "Exception in writePassword() -> " + e);

        }
    }

    private void writeWifiOnboardNotify() {
        try {
            if (mBluetoothGatt != null) {
                BluetoothGattCharacteristic characteristicWifiOnboard = mBluetoothGatt.getService(Constants.GATT_WIFI_SERVICE1).getCharacteristic(Constants.GATT_WIFI_TRIGGER_POINT_CHARECTERISTIC);
                characteristicWifiOnboard.setValue("wifiOnboard");
                mBLEService.writeCharacteristic(characteristicWifiOnboard);
            }
        }catch (Exception e)
        {
            Logger.d(TAG, "Exception in writeWifiOnboardNotify() -> " + e);

        }
    }

    //zigbee onboarding via ble
    public void zigbeeOnboardingViaBle(final byte[] linkKey, final byte[] mode) {
        Handler handler = new Handler();
        handler.postDelayed(new Runnable() {
            public void run() {
                writeZeegbeeMode(mode);
            }
        }, 4000);

        Handler handler1 = new Handler();
        handler1.postDelayed(new Runnable() {
            public void run() {
                writeZeegbeeLinkKey(linkKey);
            }
        }, 2000);

        Handler handler2 = new Handler();
        handler2.postDelayed(new Runnable() {
            public void run() {
                writeZigbeeOnboardNotify();
            }
        }, 6000);
    }

    private void writeZeegbeeMode(byte[] mode) {
        try {
            if (mBluetoothGatt != null) {
                BluetoothGattCharacteristic characteristicMode = mBluetoothGatt.getService(Constants.GATT_ZIGBEE_SERVICE1).getCharacteristic(Constants.GATT_ZIGBEE_OPERATION_MODE_CHARECTERISTIC);
                characteristicMode.setValue(mode);
                mBLEService.writeCharacteristic(characteristicMode);
            }
        }catch (Exception e)
        {
            Logger.d(TAG, "Exception in writeZeegbeeMode() -> " + e);

        }
    }

    private void writeZeegbeeLinkKey(byte[] linkKey) {
        try {
            if (mBluetoothGatt != null) {
                BluetoothGattCharacteristic characteristicLinkKey = mBluetoothGatt.getService(Constants.GATT_ZIGBEE_SERVICE1).getCharacteristic(Constants.GATT_ZIGBEE_LINKKEY_CHARECTERISTIC);
                characteristicLinkKey.setValue(linkKey);
                mBLEService.writeCharacteristic(characteristicLinkKey);
            }
        }catch (Exception e)
        {
            Logger.d(TAG, "Exception in writeZeegbeeLinkKey() -> " + e);
        }
    }

    private void writeZigbeeOnboardNotify() {
        try {
            if (mBluetoothGatt != null) {
                BluetoothGattCharacteristic characteristicZigbee = mBluetoothGatt.getService(Constants.GATT_ZIGBEE_SERVICE1).getCharacteristic(Constants.GATT_ZIGBEE_TRIGGER_POINT_CHARECTERISTIC);
                characteristicZigbee.setValue("zigbeeOnboard");
                mBLEService.writeCharacteristic(characteristicZigbee);
            }
        }catch (Exception e)
        {
            Logger.d(TAG, "Exception in writeZigbeeOnboardNotify() -> " + e);
        }
    }

    public String readZigbeeBoardStatus() {
        String status = "";
        try {
            if (mBluetoothGatt != null) {
                BluetoothGattCharacteristic characteristicOnboard = mBluetoothGatt.getService(Constants.GATT_ZIGBEE_SERVICE1).getCharacteristic(Constants.GATT_ZIGBEE_STATUS_CHARECTERISTIC);
                mBLEService.readCharacteristic(characteristicOnboard);
                byte[] value = characteristicOnboard.getValue();
                status = convert(value);
            }
        }catch (Exception e)
        {
            Logger.d(TAG, "Exception in readZigbeeBoardStatus() -> " + e);

        }
        return status;
    }

    public String readZigbeeSupportMode() {
        String status = "";
        try {
            if (mBluetoothGatt != null) {
                BluetoothGattCharacteristic characteristicOnboard = mBluetoothGatt.getService(Constants.GATT_ZIGBEE_SERVICE1).getCharacteristic(Constants.GATT_ZIGBEE_SUPPORT_MODE_CHARECTERISTIC);
                mBLEService.readCharacteristic(characteristicOnboard);
                byte[] value = characteristicOnboard.getValue();
                status = convert(value);
            }
        }catch (Exception e)
        {
            Logger.d(TAG, "Exception in readZigbeeSupportMode() -> " + e);

        }
        return status;
    }

    public String readBleBoardStatus() {
        String status = "";
        try {
            if (mBluetoothGatt != null) {
                BluetoothGattCharacteristic characteristicOnboard = mBluetoothGatt.getService(Constants.GATT_WIFI_SERVICE1).getCharacteristic(Constants.GATT_WIFI_STATUS_CHARECTERISTIC);
                BLEGattAttributes.getBLEService().readCharacteristic(characteristicOnboard);
                byte[] value = characteristicOnboard.getValue();
                status = convert(value);
            }
        }catch (Exception e)
        {
            Logger.d(TAG, "Exception in readBleBoardStatus() -> " + e);

        }
        return status;
    }

    //Thread on boarding via ble
    public void threadOnboardingViaBle(final byte[] linkKey, final byte[] mode) {
        Handler handler = new Handler();
        handler.postDelayed(new Runnable() {
            public void run() {
                writeThreadMode(mode);
            }
        }, 2000);

        Handler handler1 = new Handler();
        handler1.postDelayed(new Runnable() {
            public void run() {
                writeThreadLinkKey(linkKey);
            }
        }, 4000);

        Handler handler2 = new Handler();
        handler2.postDelayed(new Runnable() {
            public void run() {
                writeThreadOnboardNotify();
            }
        }, 6000);
    }

    private void writeThreadMode(byte[] mode) {
        try {
            if (mBluetoothGatt != null) {
                BluetoothGattCharacteristic characteristicMode = mBluetoothGatt.getService(Constants.GATT_THREAD_SERVICE1).getCharacteristic(Constants.GATT_THREAD_OPERATION_MODE_CHARECTERISTIC);
                characteristicMode.setValue(mode);
                mBLEService.writeCharacteristic(characteristicMode);
            }
        }catch (Exception e)
        {
            Logger.d(TAG, "Exception in writeThreadMode() -> " + e);

        }
    }

    private void writeThreadLinkKey(byte[] linkKey) {
        try {
            if (mBluetoothGatt != null) {
                BluetoothGattCharacteristic characteristicLinkKey = mBluetoothGatt.getService(Constants.GATT_THREAD_SERVICE1).getCharacteristic(Constants.GATT_THREAD_LINKKEY_CHARECTERISTIC);
                characteristicLinkKey.setValue(linkKey);
                mBLEService.writeCharacteristic(characteristicLinkKey);
            }
        }catch (Exception e)
        {
            Logger.d(TAG, "Exception in writeThreadLinkKey() -> " + e);

        }
    }

    private void writeThreadOnboardNotify() {
        try {
            if (mBluetoothGatt != null) {
                BluetoothGattCharacteristic characteristicZigbee = mBluetoothGatt.getService(Constants.GATT_THREAD_SERVICE1).getCharacteristic(Constants.GATT_THREAD_TRIGGER_POINT_CHARECTERISTIC);
                characteristicZigbee.setValue("zigbeeOnboard");
                mBLEService.writeCharacteristic(characteristicZigbee);
            }
        }catch (Exception e)
        {
            Logger.d(TAG, "Exception in writeThreadOnboardNotify() -> " + e);

        }
    }

    public String readThreadBoardStatus() {
        String status = "";
        try {
            if (mBluetoothGatt != null) {
                BluetoothGattCharacteristic characteristicOnboard = mBluetoothGatt.getService(Constants.GATT_THREAD_SERVICE1).getCharacteristic(Constants.GATT_THREAD_STATUS_CHARECTERISTIC);
                mBLEService.readCharacteristic(characteristicOnboard);
                byte[] value = characteristicOnboard.getValue();
                status = convert(value);
            }
        }catch (Exception e)
        {
            Logger.d(TAG, "Exception in readThreadBoardStatus() -> " + e);

        }
        return status;
    }

    public String readThreadSupportMode() {
        String status = "";
        try {
            if (mBluetoothGatt != null) {
                BluetoothGattCharacteristic characteristicOnboard = mBluetoothGatt.getService(Constants.GATT_THREAD_SERVICE1).getCharacteristic(Constants.GATT_THREAD_SUPPORT_MODE_CHARECTERISTIC);
                mBLEService.readCharacteristic(characteristicOnboard);
                byte[] value = characteristicOnboard.getValue();
                status = convert(value);
            }
        }catch (Exception e)
        {
            Logger.d(TAG, "Exception in readThreadSupportMode() -> " + e);

        }
        return status;
    }

    public String readOfflineData()
    {
        String status = "";
        try {
            if (mBluetoothGatt != null) {
                BluetoothGattCharacteristic characteristicOfflineRead = mBluetoothGatt.getService(Constants.GATT_OFFLINE_SERVICE).getCharacteristic(Constants.GATT_OFFLINE_READ_WRITE_CHARACTERISTIC);
                mBLEService.readCharacteristic(characteristicOfflineRead);
                byte[] value = characteristicOfflineRead.getValue();
                status = convert1(value);
            }
        }catch (Exception e)
        {
            Logger.d(TAG, "Exception in readOfflineData() -> " + e);

        }
        return status;
    }
    public void writeOfflineData(String json) {
        try {
            if (mBluetoothGatt != null) {
                BluetoothGattCharacteristic characteristicOfflineWrite = mBluetoothGatt.getService(Constants.GATT_OFFLINE_SERVICE).getCharacteristic(Constants.GATT_OFFLINE_READ_WRITE_CHARACTERISTIC);
                characteristicOfflineWrite.setValue(json);
                mBLEService.writeCharacteristic(characteristicOfflineWrite);
                Logger.d(TAG, " writeOfflineData() "+json);

            }
        }catch (Exception e)
        {
            Logger.d(TAG, "Exception in writeOfflineData() -> " + e);

        }
    }
    public void notifyBreachData() {
        try {
            if (mBluetoothGatt != null) {
                BluetoothGattCharacteristic characteristicOfflineWrite = mBluetoothGatt.getService(Constants.GATT_OFFLINE_SERVICE).getCharacteristic(Constants.GATT_OFFLINE_NOTIFY_CHARACTERISTIC);
                //characteristicOfflineWrite.setValue(json);
                mBLEService.setCharacteristicNotification(characteristicOfflineWrite, true);
            }
        }catch (Exception e)
        {
            Logger.d(TAG, "Exception in notifyBreachData() -> " + e);

        }
    }
    String convert1(byte[] data) {
        if (data == null) {
            return null;
        }

        StringBuilder sb = new StringBuilder(data.length);
        for (int i = 0; i < data.length; ++i) {
            if (data[i] < 0) throw new IllegalArgumentException();
//            {
//                if (data[i] != 0) {
                    sb.append((char) data[i]);
//                }
//            }
        }
        return sb.toString();
    }

    String convert(byte[] data) {
        if (data == null) {
            return null;
        }

        StringBuilder sb = new StringBuilder(data.length);
        for (int i = 0; i < 1; ++i) {
            if (data[i] < 0) throw new IllegalArgumentException();
            sb.append((char) data[i]);
        }
        return sb.toString();
    }

}
