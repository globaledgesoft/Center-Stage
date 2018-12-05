/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package com.qualcomm.qti.qca40xx.Util;

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;

import com.qualcomm.qti.qca40xx.Model.ListOfCoordinatorDataModel;

import java.util.ArrayList;

/**
 * SQLite Database Helper class
 */

public class DBHelper extends SQLiteOpenHelper {
    //Database name
    private static final String DATABASE_NAME = "RUBY_QUARTZ_DB";
    //Database Version
    private static final int DATABASE_VERSION = 1;
    //Table Name
    private static final String TABLE_NAME = "zigbee_table";
    //column name of table
    private static final String NAME_COL1 = "zigbee_device_name";
    private static final String MAC_ADDRESS_COL2 = "zigbee_mac_address";
    private static final String MASTER_KEY_COL3 = "zigbee_master_key";
    private static final String SUPPORT_COL4 = "zigbee_support_mode";
    private static final String CAPABILITY_COL5 = "zigbee_capability";
    private static final String OFFLINE_COL6 = "offline";
    //Table Name
    private static final String THREAD_TABLE_NAME = "thread_table";
    //column name of table
    private static final String THREAD_DEVICE_NAME = "thread_device_name";
    private static final String THREAD_MAC_ADDRESS = "thread_mac_address";
    private static final String THREAD_PASS_PHRASE = "thread_pass_phrase";
    private static final String THREAD_SUPPORT = "thread_support_mode";
    private static final String THREAD_CAPABILITY = "thread_capability";
    private String TAG = getClass().getSimpleName();

    //constructor
    public DBHelper(Context context) {
        super(context, DATABASE_NAME, null, DATABASE_VERSION);
    }


    //Create Table
    @Override
    public void onCreate(SQLiteDatabase db) {
        String zeegbeeTableName = "CREATE TABLE " + TABLE_NAME + "(" + NAME_COL1 + " TEXT," + MAC_ADDRESS_COL2 + " TEXT," + MASTER_KEY_COL3 + " TEXT," + SUPPORT_COL4 + " TEXT," + CAPABILITY_COL5 + " TEXT," + OFFLINE_COL6 + " TEXT" + ")";
        db.execSQL(zeegbeeTableName);
        String threadTableName = "CREATE TABLE " + THREAD_TABLE_NAME + "(" + THREAD_DEVICE_NAME + " TEXT," + THREAD_MAC_ADDRESS + " TEXT," + THREAD_PASS_PHRASE + " TEXT," + THREAD_SUPPORT + " TEXT," + THREAD_CAPABILITY + " TEXT" + ")";
        db.execSQL(threadTableName);
    }

    //Upgrade Table
    @Override
    public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {


    }

    //Insert Zigbee data
    public void insertDataIntoTable(String name, String mac_addr, String passkey, String supportMode, String cap, String offline) {
        try {
            SQLiteDatabase db = this.getReadableDatabase();
            ContentValues cv = new ContentValues();

            cv.put(NAME_COL1, name);
            cv.put(MAC_ADDRESS_COL2, mac_addr);
            cv.put(MASTER_KEY_COL3, passkey);
            cv.put(SUPPORT_COL4, supportMode);
            cv.put(CAPABILITY_COL5, cap);
            cv.put(OFFLINE_COL6, offline);
            db.insert(TABLE_NAME, null, cv);
        } catch (Exception e) {
            Logger.d(TAG, "Exception in insertDataIntoTable() " + e);
        }
    }

    //update zigbee data
    public void updateDataIntoTable(String name, String linkKey, String cap) {
        try {
            SQLiteDatabase db = this.getReadableDatabase();
            ContentValues cv = new ContentValues();
            cv.put(MASTER_KEY_COL3, linkKey); //These Fields should be your String values of actual column names
            cv.put(CAPABILITY_COL5, cap);
            int i = db.update(TABLE_NAME, cv, NAME_COL1 + " = ? ", new String[]{name});

        } catch (Exception e) {
            Logger.d(TAG, "Exception in updateDataIntoTable() " + e);
        }
    }

    public boolean dataExistOrNot(String name) {
        String data = "";
        try {
            SQLiteDatabase db = this.getReadableDatabase();
            String query = "select * from " + TABLE_NAME + " where " + NAME_COL1 + " ='" + name + "'";

            Cursor cursor = db.rawQuery(query, null);

            if (cursor.getCount() <= 0) {
                cursor.close();
                return false;
            }
            cursor.close();
        } catch (Exception e) {
            Logger.d(TAG, "Exception in dataExistOrNot() " + e);
        }
        return true;
    }

    public String offlineCoordinatorExistOrNot(String isCoordinatorPresent) {
        String name = "";
        try {
            SQLiteDatabase db = this.getReadableDatabase();
            String query = "select * from " + TABLE_NAME + " where " + OFFLINE_COL6 + " ='" + isCoordinatorPresent + "'";

            Cursor cursor = db.rawQuery(query, null);
            if (cursor.getCount() == 1) {
                if (cursor.moveToFirst()) {
                    while (!cursor.isAfterLast()) {
                        name = cursor.getString(cursor.getColumnIndex(NAME_COL1));
                        cursor.moveToNext();
                    }
                }
            }
            cursor.close();
        } catch (Exception e) {
            Logger.d(TAG, "Exception in offlineCoordinatorExistOrNot() " + e);
        }
        return name;
    }

    // get zigbee device name
    public ArrayList<String> getDeviceList() {
        ArrayList<String> deviceList = new ArrayList<String>();
        try {
            SQLiteDatabase db = this.getReadableDatabase();
            Cursor cursor = db.rawQuery("select * from " + TABLE_NAME, null);

            if (cursor.moveToFirst()) {
                while (!cursor.isAfterLast()) {
                    String name = cursor.getString(cursor.getColumnIndex(NAME_COL1));
                    String capability = cursor.getString(cursor.getColumnIndex(CAPABILITY_COL5));
                    deviceList.add(name + " (" + capability + ")");
                    cursor.moveToNext();
                }
            }
            cursor.close();
        } catch (Exception e) {
            Logger.d(TAG, "Exception in getDeviceList() " + e);
        }
        return deviceList;
    }

    // get coordinator device details
    public ArrayList<ListOfCoordinatorDataModel> getCoordinatorList() {
        ArrayList<ListOfCoordinatorDataModel> coordinatorList = new ArrayList<ListOfCoordinatorDataModel>();
        try {
            SQLiteDatabase db = this.getReadableDatabase();
            Cursor cursor = db.rawQuery("select * from " + TABLE_NAME, null);

            if (cursor.moveToFirst()) {
                while (!cursor.isAfterLast()) {
                    ListOfCoordinatorDataModel model = new ListOfCoordinatorDataModel();
                    String name = cursor.getString(cursor.getColumnIndex(NAME_COL1));
                    model.setDeviceName(name);
                    String mac = cursor.getString(cursor.getColumnIndex(MAC_ADDRESS_COL2));
                    model.setDeviceMac(mac);
                    String isOffline = cursor.getString(cursor.getColumnIndex(OFFLINE_COL6));
                    if (isOffline.equals(Constants.NO)) {
                        model.setDeviceState("Configured to AWS");
                    } else {
                        model.setDeviceState("OFFLINE");
                    }
                    coordinatorList.add(model);
                    cursor.moveToNext();
                }
            }
            cursor.close();
        } catch (Exception e) {
            Logger.d(TAG, "Exception in getDeviceList() " + e);
        }
        return coordinatorList;
    }

    //delete coordinator
    public int deleteCoorinator(String name) {
        int val = 0;
        try {
            SQLiteDatabase db = this.getReadableDatabase();
            val = db.delete(TABLE_NAME, NAME_COL1 + "=?", new String[]{name});

            db.close();
        } catch (Exception e) {
            Logger.d(TAG, "Exception in deleteCoorinator() " + e);
        }
        return val;
    }

    // get zigbee passkey
    public String getMasterKey(String deviceName) {
        String key = "";
        try {
            SQLiteDatabase db = this.getReadableDatabase();

            String query = "select * from " + TABLE_NAME + " where " + NAME_COL1 + " ='" + deviceName + "'";
            Cursor cursor = db.rawQuery(query, null);

            if (cursor != null)
                cursor.moveToFirst();
            key = cursor.getString(cursor.getColumnIndex(MASTER_KEY_COL3));
            cursor.close();
        } catch (Exception e) {
            Logger.d(TAG, "Exception in getMasterKey() " + e);
        }
        return key;
    }

    //Insert thread data
    public void insertDataIntoThreadTable(String name, String mac_addr, String passkey, String supportMode, String cap) {
        SQLiteDatabase db = this.getReadableDatabase();
        ContentValues cv = new ContentValues();

        cv.put(THREAD_DEVICE_NAME, name);
        cv.put(THREAD_MAC_ADDRESS, mac_addr);
        cv.put(THREAD_PASS_PHRASE, passkey);
        cv.put(THREAD_SUPPORT, supportMode);
        cv.put(THREAD_CAPABILITY, cap);
        db.insert(THREAD_TABLE_NAME, null, cv);
    }

    //update thread data
    public void updateDataIntoThreadTable(String name, String linkKey, String cap) {
        SQLiteDatabase db = this.getReadableDatabase();
        ContentValues cv = new ContentValues();
        cv.put(THREAD_PASS_PHRASE, linkKey); //These Fields should be your String values of actual column names
        cv.put(THREAD_CAPABILITY, cap);
        int i = db.update(THREAD_TABLE_NAME, cv, THREAD_DEVICE_NAME + " = ? ", new String[]{name});
    }

    public boolean threadDataExistOrNot(String name) {
        String data = "";
        SQLiteDatabase db = this.getReadableDatabase();
        String query = "select * from " + THREAD_TABLE_NAME + " where " + THREAD_DEVICE_NAME + " ='" + name + "'";

        Cursor cursor = db.rawQuery(query, null);

        if (cursor.getCount() <= 0) {
            cursor.close();
            return false;
        }
        cursor.close();
        return true;
    }

    // get thread device name
    public ArrayList<String> getThreadDeviceList() {
        ArrayList<String> deviceList = new ArrayList<String>();

        SQLiteDatabase db = this.getReadableDatabase();
        Cursor cursor = db.rawQuery("select * from " + THREAD_TABLE_NAME, null);

        if (cursor.moveToFirst()) {
            while (!cursor.isAfterLast()) {
                String name = cursor.getString(cursor.getColumnIndex(THREAD_DEVICE_NAME));
                String capability = cursor.getString(cursor.getColumnIndex(THREAD_CAPABILITY));
                deviceList.add(name + " (" + capability + ")");
                cursor.moveToNext();
            }
        }
        cursor.close();
        return deviceList;
    }

    // get thread passPhrase
    public String getThreadPassPhrase(String deviceName) {

        String key = "";
        try {
            SQLiteDatabase db = this.getReadableDatabase();

            String query = "select * from " + THREAD_TABLE_NAME + " where " + THREAD_DEVICE_NAME + " ='" + deviceName + "'";
            Cursor cursor = db.rawQuery(query, null);

            if (cursor != null)
                cursor.moveToFirst();
            key = cursor.getString(cursor.getColumnIndex(THREAD_PASS_PHRASE));
            cursor.close();
        } catch (Exception e) {
            Logger.d(TAG, "Exception in getThreadPassPhrase() " + e);

        }
        return key;
    }
}
