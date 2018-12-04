package com.qualcomm.qti.qca40xx.Adapter;

import android.content.Context;
import android.os.Handler;
import android.support.v7.widget.RecyclerView;
import android.support.v7.widget.SwitchCompat;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.SeekBar;
import android.widget.TextView;

import com.qualcomm.qti.qca40xx.Model.SensorDetails;
import com.qualcomm.qti.qca40xx.R;
import com.qualcomm.qti.qca40xx.Util.BleServiceCharecteristic;
import com.qualcomm.qti.qca40xx.Util.Constants;
import com.qualcomm.qti.qca40xx.Util.Logger;
import com.qualcomm.qti.qca40xx.Util.Util;

import org.json.JSONObject;

import java.util.ArrayList;


/**
 * SensorDetailAdapter is an Adapter class which hold the list of sensor data for a perticular subdevice.
 */

public class SensorDetailAdapter extends RecyclerView.Adapter<SensorDetailAdapter.MyViewHolder> {
    private String TAG = getClass().getSimpleName();

    private String mainDeviceName;
    private String subDeviceName;
    private ArrayList<SensorDetails> mSensorDetailsArray = new ArrayList<>();
    private Context mContext;

    public SensorDetailAdapter(Context context, String mdeviceName, String subDeviceName, ArrayList<SensorDetails> sensorDetailsArray) {
        mainDeviceName = mdeviceName;
        this.mContext = context;
        this.subDeviceName = subDeviceName;
        this.mSensorDetailsArray = sensorDetailsArray;

    }


    @Override
    public MyViewHolder onCreateViewHolder(ViewGroup parent,
                                           int viewType) {
        View view = LayoutInflater.from(parent.getContext())
                .inflate(R.layout.item_sensor, parent, false);
        MyViewHolder myViewHolder = new MyViewHolder(view);
        return myViewHolder;
    }

    @Override
    public int getItemCount() {
        return mSensorDetailsArray.size();
    }

    public static class MyViewHolder extends RecyclerView.ViewHolder {

        TextView textViewName, textViewSensorValue, seekBarValue;
        ImageView imageViewIcon;
        SwitchCompat switchCompat;
        SeekBar seekBar;

        public MyViewHolder(View itemView) {
            super(itemView);
            this.textViewName = (TextView) itemView.findViewById(R.id.textViewSensor);
            this.imageViewIcon = (ImageView) itemView.findViewById(R.id.imageViewSensor);
            this.textViewSensorValue = (TextView) itemView.findViewById(R.id.textViewSensorValue);
            this.switchCompat = (SwitchCompat) itemView.findViewById(R.id.toggBtn);
            this.seekBar = (SeekBar) itemView.findViewById(R.id.dimSeekbar);
            this.seekBarValue = (TextView) itemView.findViewById(R.id.seekBarTextValue);
        }
    }

    @Override
    public void onBindViewHolder(final MyViewHolder holder, final int listPosition) {
        TextView textViewName = holder.textViewName;
        TextView textViewValue = holder.textViewSensorValue;
        ImageView imageView = holder.imageViewIcon;
        final SwitchCompat switchCompat = holder.switchCompat;

        final SeekBar seekBar = holder.seekBar;
        final TextView seekbarValue = holder.seekBarValue;
        seekBar.setMax(254);
        switchCompat.setVisibility(View.GONE);
        textViewValue.setVisibility(View.VISIBLE);

        final String keyData = mSensorDetailsArray.get(listPosition).getSensorKey();
        String valueData = mSensorDetailsArray.get(listPosition).getSensorValue();
        textViewName.setText(keyData);
        String parentSensorType = mSensorDetailsArray.get(listPosition).getParentType();

        if (parentSensorType.equalsIgnoreCase(Constants.TEMPERATURE)) {
            textViewValue.setText(valueData + "" + (char) 0x00B0 + "C");
        } else {
            textViewValue.setText(valueData);
        }

        switch (parentSensorType) {
            case Constants.LIGHT:
                String light_value = Util.getlightValue(keyData);
                if (light_value.equals("2")) {
                    if (valueData.equals(Constants.ONE)) {
                        switchCompat.setChecked(true);
                    } else {
                        switchCompat.setChecked(false);
                    }
                } else if (light_value.equals(valueData)) {
                    switchCompat.setClickable(true);
                    if (valueData.equals(Constants.ONE)) {
                        switchCompat.setChecked(true);
                    } else {
                        switchCompat.setChecked(false);
                    }
                } else {
                    if (light_value.equals(Constants.ONE)) {
                        switchCompat.setChecked(true);
                    } else {
                        switchCompat.setChecked(false);
                    }
                }
                imageView.setImageResource(R.drawable.light);
                switchCompat.setVisibility(View.VISIBLE);
                textViewValue.setVisibility(View.GONE);

                switchCompat.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        if (switchCompat.isChecked()) {
                            Util.saveLightValue(mContext, keyData, Constants.ONE);
                            switchCompat.setClickable(false);
                            sensorDataUpdateToBoard(1, keyData, switchCompat);
                        } else {
                            Util.saveLightValue(mContext, keyData, Constants.ZERO);
                            switchCompat.setClickable(false);
                            sensorDataUpdateToBoard(0, keyData, switchCompat);
                        }
                    }
                });
                switchCompat.setOnTouchListener(new View.OnTouchListener() {
                    @Override
                    public boolean onTouch(View v, MotionEvent event) {
                        return event.getActionMasked() == MotionEvent.ACTION_MOVE;
                    }
                });
                break;
            case Constants.DIMMER:
                imageView.setImageResource(R.drawable.dimmer_light);
                textViewValue.setVisibility(View.GONE);
                seekBar.setVisibility(View.VISIBLE);
                seekbarValue.setVisibility(View.VISIBLE);

                int value = Integer.parseInt(valueData);

                if (Constants.isCondition.equals(Constants.FIRST)) {
                    seekbarValue.setText(value + "");
                    seekBar.setProgress(value);
                } else if (Constants.isCondition.equals(Constants.UPDATE)) {
                    if (value == Constants.DIMMER_DATA) {
                        seekBar.setProgress(value);
                        seekbarValue.setText(value + "");
                        seekBar.setEnabled(true);
                    } else {
                        seekBar.setProgress(Constants.DIMMER_DATA);
                        seekbarValue.setText(Constants.DIMMER_DATA + "");
                        seekBar.setEnabled(false);
                    }
                }
                seekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
                    int progress = 0;

                    @Override
                    public void onProgressChanged(SeekBar seekBar, int i, boolean b) {
                        seekbarValue.setText(i + "");
                        progress = i;
                    }

                    @Override
                    public void onStartTrackingTouch(SeekBar seekBar) {

                    }

                    @Override
                    public void onStopTrackingTouch(SeekBar seekBar) {
                        seekbarValue.setText(progress + "");
                        Constants.isCondition = Constants.UPDATE;
                        seekBar.setProgress(progress);
                        updateDimmerData(progress, keyData);
                        Constants.DIMMER_DATA = progress;
                        seekBar.setEnabled(false);
                    }
                });
                break;

            case Constants.COMPASS:
                imageView.setImageResource(R.drawable.compass);
                valueData = valueData.replace("{", "").replace("}", "").replace("\"", "").replace("x:", "").replace("y:", "").replace("z:", "").replace("X:", "").replace("Y:", "").replace("Z:", "");
                textViewValue.setText(valueData + " " + "\u00B5" + "T");
                break;
            case Constants.ACCELEROMETER:
                imageView.setImageResource(R.drawable.accelerometer);
                valueData = valueData.replace("{", "").replace("}", "").replace("\"", "").replace("x:", "").replace("y:", "").replace("z:", "").replace("X:", "").replace("Y:", "").replace("Z:", "");
                String[] accelerometerArray = valueData.split(",");
                String accelerometerData = "";
                String accUnit = " m/s" + "\u00B2" + ",";
                StringBuilder accelerometerBuilder = new StringBuilder();
                for (int i = 0; i < accelerometerArray.length; i++) {
                    accelerometerBuilder.append(accelerometerArray[i]);
                    accelerometerBuilder.append(accUnit);

                }
                accelerometerData = accelerometerBuilder.toString();
                textViewValue.setText(accelerometerData.substring(0, accelerometerData.length() - 1));
                break;
            case Constants.TEMPERATURE:
                imageView.setImageResource(R.drawable.temperature);
                textViewValue.setText(valueData + "" + (char) 0x00B0 + "C");
                break;
            case Constants.HUMIDITY:
                switchCompat.setVisibility(View.GONE);
                textViewValue.setVisibility(View.VISIBLE);
                imageView.setImageResource(R.drawable.humidity);
                textViewValue.setText(valueData + " " + "rH");
                break;
            case Constants.AMBIENT:
                imageView.setImageResource(R.drawable.ambient);
                textViewValue.setText(valueData + " " + "lx");
                break;
            case Constants.PRESSURE:
                imageView.setImageResource(R.drawable.pressure);
                textViewValue.setText(valueData + " " + "hPa");
                break;
            case Constants.GYROSCOPE:
                imageView.setImageResource(R.drawable.tilt);
                valueData = valueData.replace("{", "").replace("}", "").replace("\"", "").replace("x:", "").replace("y:", "").replace("z:", "").replace("X:", "").replace("Y:", "").replace("Z:", "");
                String[] gyroArray = valueData.split(",");
                String gyroData = "";
                StringBuilder builder = new StringBuilder();
                for (int i = 0; i < gyroArray.length; i++) {
                    builder.append(gyroArray[i] + " dps,");
                }
                gyroData = builder.toString();
                textViewValue.setText(gyroData.substring(0, gyroData.length() - 1));
                break;


        }
    }

    private void sensorDataUpdateToBoard(int lightVal, String lightId, final SwitchCompat switchCompat) {
        try {

            JSONObject updateValues = new JSONObject();
            updateValues.putOpt(lightId, lightVal);

            JSONObject lightOBJ = new JSONObject();
            lightOBJ.putOpt(Constants.LIGHT, updateValues);

            JSONObject deviceObj = new JSONObject();
            deviceObj.putOpt(subDeviceName, lightOBJ);

            final JSONObject desiredObj = new JSONObject();
            desiredObj.putOpt(Constants.DESIRED, deviceObj);

            Logger.d(TAG, "light desired obj" + desiredObj);


            Handler handler2 = new Handler();
            handler2.postDelayed(new Runnable() {
                public void run() {

                    BleServiceCharecteristic.getInstance().writeOfflineData(desiredObj.toString());
                }
            }, 2000);

            Handler handler1 = new Handler();
            handler1.postDelayed(new Runnable() {
                public void run() {
                    BleServiceCharecteristic.getInstance().writeOfflineData(desiredObj.toString());
                }
            }, 4000);

        } catch (Exception e) {
            Logger.d(TAG, "Exception in sensorDataUpdateToBoard() " + e);
        }

    }

    private void updateDimmerData(int val, String sensorName) {

        try {
            Logger.d(TAG, "dimmer Value" + val);
            //Update int value for dimmer
            JSONObject itemObject = new JSONObject();
            itemObject.put(sensorName, val);

            JSONObject sensorObject = new JSONObject();
            sensorObject.putOpt(Constants.DIMMER, itemObject);

            JSONObject deviceObject = new JSONObject();
            deviceObject.putOpt(subDeviceName, sensorObject);

            final JSONObject desiredObject = new JSONObject();
            desiredObject.putOpt(Constants.DESIRED, deviceObject);


            Logger.d(TAG, "dimmer desired obj" + desiredObject);

            Handler handler2 = new Handler();
            handler2.postDelayed(new Runnable() {
                public void run() {
                    BleServiceCharecteristic.getInstance().writeOfflineData(desiredObject.toString());
                }
            }, 2000);

            Handler handler1 = new Handler();
            handler1.postDelayed(new Runnable() {
                public void run() {
                    BleServiceCharecteristic.getInstance().writeOfflineData(desiredObject.toString());
                }
            }, 4000);

        } catch (Exception e) {
            e.printStackTrace();
            Logger.d(TAG, "Exception in updateDimmerData() " + e);
        }
    }


}
