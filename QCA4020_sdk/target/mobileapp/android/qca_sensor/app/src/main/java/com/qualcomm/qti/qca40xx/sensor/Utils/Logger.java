/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package com.qualcomm.qti.qca40xx.sensor.Utils;

import android.util.Log;

import java.io.FileWriter;

/**
 * Custom logger class for Logs
 */

public class Logger {

    private static final String Log_TAG = "QCAOnboarding";
    private static boolean isLogEnable = true;
    private static Logger  logger = null;
    private static FileWriter writer;

    public static Logger getInstance() {
        if ( logger == null) {
             logger = new Logger();
        }
        return  logger;
    }

    public static void v(String TAG, String msg) {
        if (isLogEnable) {
            Log.v(Log_TAG, msg);

        }
    }

    public static void d(String TAG, String msg) {
        if (isLogEnable) {
            Log.d(Log_TAG, msg);

        }
    }

    public static void e(String TAG, String msg) {
        if (isLogEnable) {
            Log.e(Log_TAG, msg);

        }
    }


    public static void i(String TAG, String msg) {
        if (isLogEnable) {
            Log.i(Log_TAG, msg);

        }
    }


}
