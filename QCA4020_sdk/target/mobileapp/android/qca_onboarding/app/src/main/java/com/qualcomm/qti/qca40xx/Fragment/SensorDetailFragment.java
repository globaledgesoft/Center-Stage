package com.qualcomm.qti.qca40xx.Fragment;

import android.app.ProgressDialog;
import android.content.BroadcastReceiver;
import android.content.Context;

import android.content.IntentFilter;

import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v4.app.Fragment;
import android.support.v4.content.LocalBroadcastManager;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import com.qualcomm.qti.qca40xx.Adapter.SensorDetailAdapter;
import com.qualcomm.qti.qca40xx.Model.SensorDetails;
import com.qualcomm.qti.qca40xx.R;
import com.qualcomm.qti.qca40xx.Util.Constants;

import com.qualcomm.qti.qca40xx.Util.Logger;
import com.qualcomm.qti.qca40xx.Util.Util;

import org.json.JSONObject;

import java.util.ArrayList;
import java.util.Iterator;

// SensorFragment fragment for Sensor data


public class SensorDetailFragment extends Fragment {

    private String TAG = getClass().getSimpleName();
    private RecyclerView recyclerView;
    private TextView noSensorDetails;
    private SensorDetailAdapter adapter;
    private ArrayList<SensorDetails> sensorList;
    private String mainDeviceName, subDeviceName;
    private Context mContext;
    private ProgressDialog dialog;
    private int sensorPollingcount =0;
    String JsondataFrmPrv;

    public final BroadcastReceiver myBroadcastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, android.content.Intent intent) {
            String status = intent.getStringExtra(Constants.SENSOR_JSON_STRING);
            Logger.d(TAG, "sensor receiver data : " + status);
            if (!status.isEmpty()) {
                recyclerView.setVisibility(View.VISIBLE);
                noSensorDetails.setVisibility(View.GONE);
                sensorList = new ArrayList<>();
                jsonParsing(status);
            } else {
                sensorPollingcount=sensorPollingcount+1;
                if (sensorPollingcount > Constants.POLLING_FAIL_COUNT) {
                    noSensorDetails.setVisibility(View.VISIBLE);
                    recyclerView.setVisibility(View.GONE);
                    if (dialog != null) {
                        dialog.dismiss();
                    }
                }
            }

        }
    };

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mContext = getContext();
        sensorList = new ArrayList<>();

        mainDeviceName = getActivity().getIntent().getStringExtra(Constants.DEVICE_NAME);
        subDeviceName = getActivity().getIntent().getStringExtra(Constants.SUBDEVICE_ID);
        JsondataFrmPrv = getActivity().getIntent().getStringExtra(Constants.SENSOR_JSON_STRING);
    }

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.fragment_sensordetail, container, false);
        recyclerView = (RecyclerView) view.findViewById(R.id.sensor_list);
        noSensorDetails = (TextView) view.findViewById(R.id.noDetails);
        recyclerView.setHasFixedSize(true);
        RecyclerView.LayoutManager layoutManager = new LinearLayoutManager(getContext());
        recyclerView.setLayoutManager(layoutManager);
        jsonParsing(JsondataFrmPrv);

        //Store light_id with "2" by default in sharedpreference.
        for (int i = 0; i < sensorList.size(); i++) {
            String keyData = sensorList.get(i).getSensorKey();
            if (keyData.startsWith(Constants.LIGHT))
                Util.saveLightValue(mContext,keyData, "2");
        }

        LocalBroadcastManager.getInstance(getActivity()).registerReceiver(myBroadcastReceiver,
                new IntentFilter(Constants.INTENT_FILTER_SENSOR_DATA));
        return view;
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        LocalBroadcastManager.getInstance(getActivity()).unregisterReceiver(myBroadcastReceiver);
    }

    private void jsonParsing(String data) {
        try {
            JSONObject jsonObject = new JSONObject(data);
            Iterator<String> keys = jsonObject.keys();
            while (keys.hasNext()) {
                String currentDynamicKey = keys.next();
                if (!(currentDynamicKey.contains(Constants.THERMOSTAT))) {
                    JSONObject jObj = jsonObject.getJSONObject(currentDynamicKey);
                    Iterator<String> childKeys = jObj.keys();
                    while (childKeys.hasNext()) {
                        String childData = childKeys.next();
                        String childValue = jObj.getString(childData);
                        switch (currentDynamicKey) {
                            case Constants.LIGHT:
                                updateSensorDetailObject(currentDynamicKey, childData, childValue);
                                break;
                            case Constants.DIMMER:
                                updateSensorDetailObject(currentDynamicKey, childData, childValue);
                                break;
                            case Constants.COMPASS:
                                updateSensorDetailObject(currentDynamicKey, childData, childValue);
                                break;
                            case Constants.ACCELEROMETER:
                                updateSensorDetailObject(currentDynamicKey, childData, childValue);
                                break;
                            case Constants.TEMPERATURE:
                                updateSensorDetailObject(currentDynamicKey, childData, childValue);
                                break;
                            case Constants.HUMIDITY:
                                updateSensorDetailObject(currentDynamicKey, childData, childValue);
                                break;
                            case Constants.AMBIENT:
                                updateSensorDetailObject(currentDynamicKey, childData, childValue);
                                break;
                            case Constants.PRESSURE:
                                updateSensorDetailObject(currentDynamicKey, childData, childValue);
                                break;
                            case Constants.GYROSCOPE:
                                updateSensorDetailObject(currentDynamicKey, childData, childValue);
                                break;
                        }
                    }
                }
            }
        } catch (Exception e) {
            Logger.d(TAG, "Exception : " + e);
            e.printStackTrace();
        }

        if (sensorList.size() > 0) {
            sensorPollingcount=0;
            adapter = new SensorDetailAdapter(getContext(), mainDeviceName, subDeviceName, sensorList);
            recyclerView.setAdapter(adapter);
            if (dialog != null) {
                dialog.dismiss();
            }
        } else {
            noSensorDetails.setVisibility(View.VISIBLE);
            noSensorDetails.setText(R.string.nosensordata);
            if (dialog != null) {
                dialog.dismiss();
            }
        }
    }

    private void updateSensorDetailObject(String currentDynamicKey, String childData, String childValue) {
        SensorDetails sensorDetails = new SensorDetails();
        sensorDetails.setParentType(currentDynamicKey);
        sensorDetails.setSensorKey(childData);
        sensorDetails.setSensorValue(childValue);
        sensorList.add(sensorDetails);
    }



}
