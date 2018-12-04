package com.qualcomm.qti.qca40xx.Activity;

import android.content.BroadcastReceiver;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.os.Handler;
import android.support.design.widget.CoordinatorLayout;
import android.support.design.widget.TabLayout;
import android.support.v4.content.LocalBroadcastManager;
import android.support.v4.view.ViewPager;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.util.Log;
import android.view.MenuItem;

import com.qualcomm.qti.qca40xx.Adapter.PagerAdapter;
import com.qualcomm.qti.qca40xx.R;
import com.qualcomm.qti.qca40xx.Util.BleServiceCharecteristic;
import com.qualcomm.qti.qca40xx.Util.Constants;
import com.qualcomm.qti.qca40xx.Service.BLEService;
import com.qualcomm.qti.qca40xx.Util.Logger;
import com.qualcomm.qti.qca40xx.Util.MainApplication;
import com.qualcomm.qti.qca40xx.Util.Util;

import org.json.JSONException;
import org.json.JSONObject;

import java.util.Iterator;
import java.util.Timer;
import java.util.TimerTask;


/**
 * SensorThermostatActivity is an Activity class for to show the
 * details sensor and thermostat of perticular Subdevice.
 */

public class SensorThermostatActivity extends AppCompatActivity {

    public static Timer timer;
    private String TAG = getClass().getSimpleName();
    private TabLayout tabLayout;
    private MainApplication application;
    private BroadcastReceiver notificationReceiver;
    private CoordinatorLayout coordinatorLayout;
    private String subdevice_id, mainDeviceName;
    private String JsondataFrmPrv;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_subdevice_detail);
        initialize();
        final ViewPager viewPager = (ViewPager) findViewById(R.id.pager);
        final PagerAdapter adapter = new PagerAdapter(getSupportFragmentManager(), tabLayout.getTabCount());
        viewPager.setAdapter(adapter);
        viewPager.addOnPageChangeListener(new TabLayout.TabLayoutOnPageChangeListener(tabLayout));
        tabLayout.setOnTabSelectedListener(new TabLayout.OnTabSelectedListener() {
            @Override
            public void onTabSelected(TabLayout.Tab tab) {
                viewPager.setCurrentItem(tab.getPosition());
            }

            @Override
            public void onTabUnselected(TabLayout.Tab tab) {

            }

            @Override
            public void onTabReselected(TabLayout.Tab tab) {

            }
        });
    }

    private void initialize() {
        Log.i(TAG, "onCreate");
        application = (MainApplication) getApplicationContext();
        Intent intent = getIntent();
        subdevice_id = intent.getStringExtra(Constants.SUBDEVICE_ID);
        JsondataFrmPrv = intent.getStringExtra(Constants.SENSOR_JSON_STRING);
        mainDeviceName = getIntent().getStringExtra(Constants.DEVICE_NAME);
        tabLayout = (TabLayout) findViewById(R.id.tabLayout);
        coordinatorLayout = (CoordinatorLayout) findViewById(R.id.fragments_coordinatorLayout);

        Toolbar toolbar = (Toolbar) findViewById(R.id.bar);
        toolbar.setTitle(subdevice_id);
        setSupportActionBar(toolbar);
        getSupportActionBar().setHomeButtonEnabled(true);
        getSupportActionBar().setDisplayHomeAsUpEnabled(true);


        //check for thermostat value to show or hide thermostat TAB
        boolean check = false;
        try {
            JSONObject jsonObject = new JSONObject(JsondataFrmPrv);
            Iterator<String> keys = jsonObject.keys();
            while (keys.hasNext()) {
                String currentDynamicKey = keys.next();
                if (currentDynamicKey.contains(Constants.THERMOSTAT)) {
                    check = true;
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
        Logger.d(TAG, "check.." + check);
        if (check == false) {
            tabLayout.addTab(tabLayout.newTab().setText(R.string.sensor));
        } else {
            tabLayout.addTab(tabLayout.newTab().setText(R.string.sensor));
            tabLayout.addTab(tabLayout.newTab().setText(R.string.thermostat));
            tabLayout.setTabGravity(TabLayout.GRAVITY_FILL);
        }


    }

    public void getSensorPollingData()

    {
        Handler handler1 = new Handler();
        handler1.postDelayed(new Runnable() {
            public void run() {
                String status = BleServiceCharecteristic.getInstance().readOfflineData();

            }
        }, 3000);
        Handler handler3 = new Handler();
        handler3.postDelayed(new Runnable() {
            public void run() {
                String status = BleServiceCharecteristic.getInstance().readOfflineData();
                Logger.d(TAG, "SENSOR DATA : " + status);
                if (status != null && (!status.isEmpty())) {
                    try {
                        JSONObject jsonObject = new JSONObject(status);
                        JSONObject reportedObject = jsonObject.getJSONObject(Constants.REPORTED);
                        JSONObject currentDynamicValue = reportedObject.getJSONObject(subdevice_id);
                        String data = currentDynamicValue.toString();
                        if (!data.isEmpty() && data != null) {
                            Intent intent = new Intent(Constants.INTENT_FILTER_SENSOR_DATA);
                            intent.putExtra(Constants.SENSOR_JSON_STRING, data);//If you need extra, add: intent.putExtra("extra","something");
                            LocalBroadcastManager.getInstance(SensorThermostatActivity.this).sendBroadcast(intent);
                        } else {
                            Logger.d(TAG, "No SensorData : " + data);
                        }
                    } catch (JSONException e) {
                        Logger.d(TAG, "Exception : " + e);
                        e.printStackTrace();
                    }
                } else {
                    Logger.d(TAG, "No SensorData : " + status);
                    Intent intent = new Intent(Constants.INTENT_FILTER_SENSOR_DATA);
                    intent.putExtra(Constants.SENSOR_JSON_STRING, "");//If you need extra, add: intent.putExtra("extra","something");
                    LocalBroadcastManager.getInstance(SensorThermostatActivity.this).sendBroadcast(intent);
                }
            }
        }, 5000);

        Handler handler2 = new Handler();
        handler2.postDelayed(new Runnable() {
            public void run() {
                BleServiceCharecteristic.getInstance().writeOfflineData(Util.RequestSensorData(mainDeviceName, subdevice_id));
            }
        }, 2000);
    }

    private void pollingData() {
        timer = new Timer();
        timer.schedule(new TimerTask() {
            @Override
            public void run() {
                runOnUiThread(new Runnable() {
                    public void run() {
                        Logger.d(TAG, "sensor polling");
                        getSensorPollingData();
                    }
                });

            }
        }, 0, 5000);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        Util.clearLightSp(SensorThermostatActivity.this);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if (item.getItemId() == android.R.id.home) {
            if (timer != null)
                Util.destroyTimer(timer);
            finish();
        }
        return super.onOptionsItemSelected(item);
    }

    @Override
    public void onBackPressed() {
        if (timer != null)
            Util.destroyTimer(timer);
        finish();
    }

    @Override
    protected void onPause() {
        super.onPause();
        unregisterReceiver(notificationReceiver);
        if (timer != null)
            Util.destroyTimer(timer);
    }

    @Override
    protected void onResume() {
        super.onResume();
        final IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(BLEService.ACTION_DATA_AVAILABLE);
        notificationReceiver = application.breachNotification(coordinatorLayout);
        registerReceiver(notificationReceiver, intentFilter);
        pollingData();
    }

}
