/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package com.qualcomm.qti.qca40xx.Util;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;

import com.qualcomm.qti.qca40xx.Activity.BleWifiListActivity;
import com.qualcomm.qti.qca40xx.Activity.QRCodeScanningActivity;
import com.qualcomm.qti.qca40xx.Service.BLEService;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.UnsupportedEncodingException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Random;
import java.util.Set;
import java.util.Timer;

/**
 * This class has generic methods.
 */

public class Util {


    private static String TAG = "Util";
    private static SharedPreferences sharedPreferences;

    public static String getDeviceName(String splitString) {
        String name = "";

        if (splitString.contains("(")) {
            String[] arr = splitString.split(" ");
            name = arr[0];
        }
        return name;
    }


    public static void goToQRScanningScreen(Context context) {
        Intent connectedIntent = new Intent(context, QRCodeScanningActivity.class);
        connectedIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TASK);
        context.startActivity(connectedIntent);
    }

    public static void goToManualScanningScreen(Context context) {
        Intent connectedIntent = new Intent(context, BleWifiListActivity.class);
        connectedIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TASK);
        context.startActivity(connectedIntent);
    }

    //ble unpairing device
    public static void unParingDevice(BluetoothDevice device) {
        try {
            Method m = device.getClass().getMethod(Constants.REMOVE_BOND, (Class[]) null);
            m.invoke(device, (Object[]) null);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    //wifi network disconnection()
    public static void disconnectWifiNetwork(Context context, int netId) {

        MainApplication application = (MainApplication) context.getApplicationContext();
        application.getWifiManager().removeNetwork(netId);
        application.getWifiManager().saveConfiguration();
    }


    //screen navigation
    public static void gotoScreen(Context fromActivity, Class toActivity, String callingActivity) {
        Intent connectedIntent = new Intent(fromActivity, toActivity);
        connectedIntent.putExtra(Constants.CALLING_ACTIVITY, callingActivity);
        fromActivity.startActivity(connectedIntent);
    }

    //Screen navigation with data
    public static void gotoScreenWithData(Context fromActivity, Class toActivity, String callingActivity, String jsonData) {
        Intent intent = new Intent(fromActivity, toActivity);
        intent.putExtra(Constants.CALLING_ACTIVITY, callingActivity);
        intent.putExtra(Constants.READ_DATA, jsonData);
        fromActivity.startActivity(intent);
    }

    //Screen navigation to home page
    public static void goToHomeScreen(Context fromActivity, String mCallingActivity) {
        if (mCallingActivity.equals(Constants.QR_SCANNING)) {
            Intent intent = new Intent(fromActivity, QRCodeScanningActivity.class);
            intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TASK);
            fromActivity.startActivity(intent);
        } else if (mCallingActivity.equals(Constants.MANUAL_SCANNING)) {
            Intent intent = new Intent(fromActivity, BleWifiListActivity.class);
            intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TASK);
            fromActivity.startActivity(intent);
        }
    }

    // Ble disconnection
    public static void bleDisconnection(Context fromActivity, String mCallingActivity) {
        BLEService.getInstance().disconnect();
        goToHomeScreen(fromActivity, mCallingActivity);
    }

    // Ble disconnection
    public static void checkForBleDeviceToUnpair(String macAddress) {
        BluetoothAdapter mBluetoothAdapter1 = BluetoothAdapter.getDefaultAdapter();
        Set<BluetoothDevice> pairedDevices1 = mBluetoothAdapter1.getBondedDevices();

        for (BluetoothDevice bt : pairedDevices1) {
            if (bt.getAddress().contains(macAddress)) {
                unParingDevice(bt);
            }
        }
    }

    public static String generatingRandomKey() {
        String SALTCHARS = Constants.RANDOM_KEY;
        StringBuilder random = new StringBuilder();
        Random rnd = new Random();
        while (random.length() < 16) { // length of the random string.
            int index = (int) (rnd.nextFloat() * SALTCHARS.length());
            random.append(SALTCHARS.charAt(index));
        }
        String randomArray = random.toString();
        return randomArray;
    }


    //check support mode in wifi and ble
    public static ArrayList<String> checkSupportMode(int value, String nameOfBoard) {
        ArrayList<String> modeValue = new ArrayList<>();

        int num = 2;
        if (nameOfBoard.equalsIgnoreCase(Constants.ZIGBEE)) {
            if (value < 8 && value >= 0) {
                int pos = 3;
                while (pos >= 1) {
                    int i = (value >> num) & 1;

                    if (pos == 3 && i == 1) {
                        modeValue.add(Constants.COORDINATOR);
                    } else if (pos == 2 && i == 1) {
                        modeValue.add(Constants.ROUTER);
                    } else if (pos == 1 && i == 1) {
                        modeValue.add(Constants.ENDDEVICE);
                    }

                    --num;
                    --pos;

                }


            }
        } else if (nameOfBoard.equalsIgnoreCase(Constants.THREAD)) {
            if (value < 8 && value >= 0) {
                int pos = 3;
                while (pos >= 1) {
                    int i = (value >> num) & 1;

                    if (pos == 3 && i == 1) {
                        modeValue.add(Constants.BORDER_ROUTER);
                    } else if (pos == 2 && i == 1) {
                        modeValue.add(Constants.JOINER_ROUTER);
                    } else if (pos == 1 && i == 1) {
                        modeValue.add(Constants.JOINER);
                    }

                    --num;
                    --pos;

                }


            }
        }


        return modeValue;
    }

    // Method to generate Jsonrequest format for sensor details.
    public static String RequestSensorData(String mainDeviceName, String SubDeviceName) {
        JSONObject DName = new JSONObject();
        try {
            DName.put(Constants.SUBDEVICE_KEY, SubDeviceName);
        } catch (JSONException e) {
            e.printStackTrace();
        }
        JSONObject deviceName = new JSONObject();
        try {
            deviceName.put(mainDeviceName, DName);
        } catch (JSONException e) {
            e.printStackTrace();
        }
        JSONObject sensor = new JSONObject();
        try {
            sensor.put(Constants.SENSOR_JSON_HEAD, deviceName);
        } catch (JSONException e) {
            e.printStackTrace();
        }

        return sensor.toString();
    }

    // Method to generate Jsonrequest format for deviceList.
    public static String RequestDeviceList(String mainDeviceName) {
        JSONObject dName = new JSONObject();
        try {
            dName.put(Constants.MAIN_DEVICE_KEY, mainDeviceName);
        } catch (JSONException e) {
            e.printStackTrace();
        }

        JSONObject GetList = new JSONObject();
        try {
            GetList.put(Constants.GETLIST, dName);
        } catch (JSONException e) {
            e.printStackTrace();
        }

        return GetList.toString();
    }

    // Method to get the List of devices.
    public static List<String> getSubDeviceList(String deviceListjsonString) {
        List<String> devicelist = new ArrayList<>();
        try {
            JSONObject deviceListJson = new JSONObject(deviceListjsonString);
            JSONArray deviceListArry = deviceListJson.getJSONArray(Constants.DEVICE_LIST);
            for (int i = 0; i < deviceListArry.length(); i++) {
                devicelist.add(deviceListArry.getJSONObject(i).get(Constants.MAIN_DEVICE_KEY).toString());
            }
            return devicelist;
        } catch (JSONException e) {
            e.printStackTrace();
            return devicelist;
        }
    }

    //Stop timer
    public static void destroyTimer(Timer timer) {
        if (timer != null) {
            timer.cancel();
            timer = null;
            Logger.d(TAG, "timer stopped " + timer);
        }
    }

    public static String getNotificationMsg(String msgJson) {
        JSONObject jsonObject = null;
        String msgBreach = "";
        try {
            jsonObject = new JSONObject(msgJson);
            Iterator<String> keys = jsonObject.keys();
            String mDeviceKey = keys.next();
            JSONObject jsonObject1 = jsonObject.getJSONObject(mDeviceKey);
            msgBreach = jsonObject1.getString(Constants.MESSAGE);
            String message = msgBreach + Constants.FROM + mDeviceKey;
            return message;

        } catch (JSONException e) {
            Logger.d(TAG, "Exception : " + e);
            e.printStackTrace();
        }
        return msgBreach;
    }


    public static void saveLightValue(Context mContext, String light_id, String light_val) {
        sharedPreferences= mContext.getSharedPreferences(Constants.LIGHT, Context.MODE_PRIVATE);
        SharedPreferences.Editor editor = sharedPreferences.edit();
        editor.putString(light_id, light_val);
        editor.commit();
    }

    public static String getlightValue(String light_id) {
        String light_value = sharedPreferences.getString(light_id, "2");
        return light_value;
    }

    public static void clearLightSp(Context mContext) {
        sharedPreferences = mContext.getSharedPreferences(Constants.LIGHT, Context.MODE_PRIVATE);
        sharedPreferences.edit().clear().commit();
    }
}

