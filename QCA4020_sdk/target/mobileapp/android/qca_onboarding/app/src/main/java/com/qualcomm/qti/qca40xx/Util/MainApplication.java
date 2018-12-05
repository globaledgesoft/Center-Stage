/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package com.qualcomm.qti.qca40xx.Util;

import android.app.Activity;
import android.app.Application;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.graphics.Color;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.support.design.widget.CoordinatorLayout;
import android.support.design.widget.Snackbar;
import android.view.View;
import android.widget.TextView;

import com.qualcomm.qti.qca40xx.Service.BLEService;

/**
 * Created by sneha on 3/1/18.
 */

public class MainApplication extends Application {

    public WifiManager wifiManager;
    private String TAG=getClass().getSimpleName();
   // public BroadcastReceiver notificationReceiver;

    @Override
    public void onCreate() {
        super.onCreate();


        registerActivityLifecycleCallbacks(new ActivityLifecycleCallbacks() {
            @Override
            public void onActivityCreated(Activity activity, Bundle bundle) {
                activity.setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
            }

            @Override
            public void onActivityStarted(Activity activity) {

            }

            @Override
            public void onActivityResumed(Activity activity) {

            }

            @Override
            public void onActivityPaused(Activity activity) {

            }

            @Override
            public void onActivityStopped(Activity activity) {

            }

            @Override
            public void onActivitySaveInstanceState(Activity activity, Bundle bundle) {

            }

            @Override
            public void onActivityDestroyed(Activity activity) {

            }
        });
    }

    public WifiManager getWifiManager() {
        wifiManager = (WifiManager) getSystemService(Context.WIFI_SERVICE);
        return wifiManager;
    }

    public BroadcastReceiver breachNotification(final CoordinatorLayout coordinatorLayout) {
         BroadcastReceiver notificationReceiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                final String action = intent.getAction();
                if (BLEService.ACTION_DATA_AVAILABLE.equals(action)) {
                    String data = intent.getStringExtra(BLEService.EXTRA_DATA);
                    Logger.d(TAG,"Notification data : "+data);
                    String message=Util.getNotificationMsg(data);
                    Logger.d(TAG,"message : "+message);
                    Snackbar snackbar = Snackbar.make(coordinatorLayout, message, Snackbar.LENGTH_LONG);
                    snackbar.setActionTextColor(Color.WHITE);
                    View snackbarView = snackbar.getView();
                    snackbarView.setBackgroundColor(Color.DKGRAY);
                    TextView textView = (TextView) snackbarView.findViewById(android.support.design.R.id.snackbar_text);
                    textView.setTextColor(Color.WHITE);
                    snackbar.setDuration(6000);
                    snackbar.show();
                }
            }

        };
        return notificationReceiver;
    }

}
