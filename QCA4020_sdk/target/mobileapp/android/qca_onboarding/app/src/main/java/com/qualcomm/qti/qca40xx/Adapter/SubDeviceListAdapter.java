package com.qualcomm.qti.qca40xx.Adapter;

import android.app.ProgressDialog;
import android.content.Context;
import android.content.Intent;
import android.os.Handler;

import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;
import android.widget.Toast;

import com.qualcomm.qti.qca40xx.Activity.SensorThermostatActivity;
import com.qualcomm.qti.qca40xx.Activity.OfflineDeviceListActivity;
import com.qualcomm.qti.qca40xx.Interface.ItemClickListener;
import com.qualcomm.qti.qca40xx.R;
import com.qualcomm.qti.qca40xx.Util.BleServiceCharecteristic;
import com.qualcomm.qti.qca40xx.Util.Constants;
import com.qualcomm.qti.qca40xx.Util.Logger;
import com.qualcomm.qti.qca40xx.Util.Util;


import org.json.JSONException;
import org.json.JSONObject;

import java.util.List;

import static com.qualcomm.qti.qca40xx.Activity.OfflineDeviceListActivity.listTimer;

/**
 * Created by sushma on 6/2/18.
 */

/**
 * SubDeviceListAdapter is an Adapter class which hold the list of the Sub hub for the perticular main hub
 */
public class SubDeviceListAdapter extends RecyclerView.Adapter<RecyclerView.ViewHolder> {

    private String TAG = getClass().getSimpleName();

    private Context mContext;
    private List<String> mSubDeviceList;
    private String mainDeviceName = "";
    private String subDeviceName = "";
    private ProgressDialog dialog;
    private Intent intent;
    private int sensorFailcount=0;

    public SubDeviceListAdapter(Context context, List<String> subDeviceList, String MainDeviceName) {
        this.mContext = context;
        this.mSubDeviceList = subDeviceList;
        this.mainDeviceName = MainDeviceName;
    }

    @Override
    public RecyclerView.ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        // Attach layout for single cell
        int layout = 0;
        RecyclerView.ViewHolder viewHolder;
        layout = R.layout.item_subdevice;
        View view = LayoutInflater.from(parent.getContext()).inflate(layout, parent, false);
        viewHolder = new SubDeviceListAdapter.DeviceHolder(view);
        return viewHolder;
    }

    @Override
    public void onBindViewHolder(RecyclerView.ViewHolder holder, int position) {
        ((SubDeviceListAdapter.DeviceHolder) holder).bindData(mSubDeviceList.get(position));

        ((SubDeviceListAdapter.DeviceHolder) holder).setClickListener(new ItemClickListener() {
            @Override
            public void onClick(View view, int position, boolean isItemClick) {
                OfflineDeviceListActivity.destroyTimer();
                sensorFailcount=0;
                Logger.d(TAG,"clicked : "+mSubDeviceList.get(position));
                subDeviceName=mSubDeviceList.get(position);
                dialog = new ProgressDialog(mContext);
                dialog.setMessage(Constants.FETCHING_SENSOR);
                dialog.setCancelable(false);
                dialog.show();
                if (listTimer==null) {
                    getSensorPollingData();
                } else {
                    Logger.d(TAG,"listTimer .."+listTimer);
                }
            }
        });
    }

    @Override
    public int getItemCount() {
        int size = 0;
        if(mSubDeviceList != null)
            size = mSubDeviceList.size();
        return size;
    }

    public class DeviceHolder extends RecyclerView.ViewHolder implements View.OnClickListener {
        public TextView device_name;
        private ItemClickListener clickListener;

        public DeviceHolder(View view) {
            super(view);
            device_name = (TextView) view.findViewById(R.id.device_name);
            view.setTag(itemView);
            view.setOnClickListener(this);
        }

        public void bindData(String devicename) {
            device_name.setText(devicename);
        }

        public void setClickListener(ItemClickListener itemClickListener) {
            this.clickListener = itemClickListener;
        }

        @Override
        public void onClick(View v) {
            clickListener.onClick(v, getPosition(), true);
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
                        JSONObject currentDynamicValue = reportedObject.getJSONObject(subDeviceName);
                        String data = currentDynamicValue.toString();
                        if (!data.isEmpty() && data != null) {
                            sensorFailcount=0;
                            if (dialog!=null) {
                                dialog.dismiss();
                            }
                                intent = new Intent(mContext.getApplicationContext(), SensorThermostatActivity.class);
                                intent.putExtra(Constants.SUBDEVICE_ID, subDeviceName);
                                intent.putExtra(Constants.DEVICE_NAME, mainDeviceName);
                                intent.putExtra(Constants.SENSOR_JSON_STRING,data);
                                Constants.isCondition = Constants.FIRST;
                                mContext.startActivity(intent);

                        } else {
                            sensorFailcount=sensorFailcount+1;
                            getSensorPollingData();
                            if (sensorFailcount>Constants.POLLING_FAIL_COUNT) {
                                if (dialog != null) {
                                    dialog.dismiss();
                                }
                                Toast.makeText(mContext, Constants.SENSORDATA_UNABLE_TO_FETCH, Toast.LENGTH_SHORT).show();
                            }
                            Logger.d(TAG, "No SensorData : " + data);
                        }
                    } catch (JSONException e) {
                        Logger.d(TAG, "Exception : " + e);
                        if (dialog!=null) {
                            dialog.dismiss();
                        }
                        Toast.makeText(mContext, Constants.SENSORDATA_UNABLE_TO_FETCH, Toast.LENGTH_SHORT).show();
                        e.printStackTrace();
                    }
                } else {
                    sensorFailcount=sensorFailcount+1;
                    getSensorPollingData();
                    if (sensorFailcount>Constants.POLLING_FAIL_COUNT) {
                        if (dialog != null) {
                            dialog.dismiss();
                        }
                        Toast.makeText(mContext, Constants.SENSORDATA_UNABLE_TO_FETCH, Toast.LENGTH_SHORT).show();
                    }
                    Logger.d(TAG, "No SensorData : " + status);
                }
            }
        }, 5000);

        Handler handler2 = new Handler();
        handler2.postDelayed(new Runnable() {
            public void run() {
                BleServiceCharecteristic.getInstance().writeOfflineData(Util.RequestSensorData(mainDeviceName, subDeviceName));
            }
        }, 2000);
    }

}
