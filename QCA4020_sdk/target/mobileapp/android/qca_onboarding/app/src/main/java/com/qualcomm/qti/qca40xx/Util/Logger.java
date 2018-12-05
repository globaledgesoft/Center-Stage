/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package com.qualcomm.qti.qca40xx.Util;

import android.os.Environment;
import android.util.Log;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Date;

/**
 * Created by ka.akshay on 10/14/2017.
 */

public class Logger {

    private static final String Log_TAG = "QCAOnboarding";
    private static boolean isLogEnable = true;
    private static Logger logger = null;
    private static FileWriter writer;

    public static Logger getInstance() {
        if (logger == null) {
            logger = new Logger();
            try {
                File root = new File(Environment.getExternalStorageDirectory(), "Notes");
                if (!root.exists()) {
                    root.mkdirs();
                }
                SimpleDateFormat dateFormat = new SimpleDateFormat(
                        "yyyy_MM_dd_HH_mm_ss");
                Date date = new Date();
                String filename = dateFormat.format(date) + ".txt";
                File gpxfile = new File(root, filename);
                writer = new FileWriter(gpxfile);
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        return logger;
    }

    public static void v(String TAG, String msg) {
        if (isLogEnable) {
            Log.v(TAG, msg);
            writeLogOnSD(TAG + " - \t" + msg + "\n");
        }
    }

    public static void d(String TAG, String msg) {
        if (isLogEnable) {
            Log.d(TAG, msg);
            writeLogOnSD(TAG + " - \t" + msg + "\n");
        }
    }

    public static void e(String TAG, String msg) {
        if (isLogEnable) {
            Log.e(TAG, msg);
            writeLogOnSD(TAG + " - \t" + msg + "\n");
        }
    }


    public static void i(String TAG, String msg) {
        if (isLogEnable) {
            Log.i(TAG, msg);
            writeLogOnSD(TAG + " - \t" + msg + "\n");
        }
    }

    private static void writeLogOnSD(String sBody) {
        try {
            if (writer != null) {
                writer.append(sBody);
                writer.flush();
            } else {
                Log.i(Log_TAG, "File Writer not created");
            }
            // writer.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
