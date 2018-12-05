package com.qualcomm.qti.qca40xx.Fragment;

import android.app.ProgressDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.IntentFilter;
import android.os.Bundle;
import android.os.Handler;
import android.support.annotation.Nullable;
import android.support.constraint.ConstraintLayout;
import android.support.v4.app.Fragment;
import android.support.v4.content.LocalBroadcastManager;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.SeekBar;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import com.qualcomm.qti.qca40xx.Model.ThermoUpdateModel;
import com.qualcomm.qti.qca40xx.R;
import com.qualcomm.qti.qca40xx.Util.BleServiceCharecteristic;
import com.qualcomm.qti.qca40xx.Util.Constants;
import com.qualcomm.qti.qca40xx.Util.Logger;
import com.qualcomm.qti.qca40xx.Util.Util;

import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import static com.qualcomm.qti.qca40xx.Activity.SensorThermostatActivity.timer;

// Thermostat fragment for thermostat data
public class ThermostatFragment extends Fragment {

    private String TAG = getClass().getSimpleName();
    private ProgressDialog dialog;
    private SeekBar seekBardesiredTemp, seekBarThreshold;
    private TextView desiredTempTextView, thresholdValue;
    private TextView currentTempTextView, textviewOperatingState, noThermoDetails;
    private Spinner spinnerData;
    private Button apply, cancel;
    private ArrayAdapter<String> adapter;
    private String operating_state;
    private String mOperatingMode;
    private int temp, desiredTemp;
    private int threshold;
    private String deviceName, subDeviceName, mainDeviceName;
    private ImageView imageView;
    private String selectedItemMode;
    private ArrayList<String> thermostatList;
    private int tempSelected, thresholdInt;
    private String currentDynamicKey;
    private ConstraintLayout thermolayt;
    private String JsondataFrmPrv;
    private ThermoUpdateModel desiredThermobj, reportedTheroBj;

    private int applyFailCount = 0; // maintaining count for compared desired and update obj equal or not
    private int thermoPollingcount = 0; // maintaining count for sensor data null or not
    private boolean isUpdating = false; // flag to maintain apply btn success or not.


    public final BroadcastReceiver myBroadcastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, android.content.Intent intent) {
            String status = intent.getStringExtra(Constants.SENSOR_JSON_STRING);
            Logger.d(TAG, "thermo rcvr : " + status);
            if (!status.isEmpty()) {
                noThermoDetails.setVisibility(View.GONE);
                thermolayt.setVisibility(View.VISIBLE);
                bindThermostatData(status, Constants.THERMO_POLLING);
            } else {
                thermoPollingcount = thermoPollingcount + 1;
                if (thermoPollingcount > Constants.POLLING_FAIL_COUNT) {
                    thermolayt.setVisibility(View.GONE);
                    noThermoDetails.setVisibility(View.VISIBLE);
                    Logger.d(TAG, "data is empty");
                }
            }
        }
    };

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        JsondataFrmPrv = getActivity().getIntent().getStringExtra(Constants.SENSOR_JSON_STRING);
    }

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        Logger.d(TAG, "onCreateView called");
        View view = inflater.inflate(R.layout.fragment_thermostat, container, false);
        initialization(view);
        subDeviceName = getActivity().getIntent().getStringExtra(Constants.SUBDEVICE_ID);
        mainDeviceName = getActivity().getIntent().getStringExtra(Constants.DEVICE_NAME);
        displayData();
        bindThermostatData(JsondataFrmPrv, Constants.FIRST);

        LocalBroadcastManager.getInstance(getActivity()).registerReceiver(myBroadcastReceiver,
                new IntentFilter(Constants.INTENT_FILTER_SENSOR_DATA));

        spinnerData.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> adapterView, View view, int i, long l) {
                selectedItemMode = adapterView.getItemAtPosition(i).toString();
            }

            @Override
            public void onNothingSelected(AdapterView<?> adapterView) {

            }
        });

        cancel.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Util.destroyTimer(timer);
                getActivity().finish();
            }
        });

        //Update Thermostat
        apply.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                //Util.destroyTimer(thermoTimer);
                apply.setClickable(false);
                apply.setText(R.string.APPLYING);
                isUpdating = true;
                String tempValue = desiredTempTextView.getText().toString();
                String splitDegree = (char) Constants.DEGREEVALUE + Constants.DEGREEUNIT;
                tempValue = tempValue.replace(splitDegree, "");
                tempSelected = Integer.parseInt(tempValue);
                thresholdInt = Integer.parseInt(thresholdValue.getText().toString());
                thermostatDataUpdateToBoard();
            }
        });

        seekBardesiredTemp.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {

            int progress = 0;

            @Override
            public void onProgressChanged(SeekBar seekBar, int progressValue, boolean b) {
                progress = progressValue;
                desiredTempTextView.setText("" + progressValue + (char) 0x00B0 + "C");
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });

        seekBarThreshold.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {

            int progress = 0;

            @Override
            public void onProgressChanged(SeekBar seekBar, int progressValue, boolean b) {
                progress = progressValue;
                thresholdValue.setText("" + progressValue);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });
        return view;
    }


    private void thermostatDataUpdateToBoard() {
        dialog.setMessage(Constants.UPDATING_THERMOSTAT);
        dialog.setCancelable(false);
        dialog.show();
        try {
            JSONObject updateValues = new JSONObject();
            updateValues.putOpt(Constants.THERMO_THRESHOLD, thresholdInt);
            updateValues.putOpt(Constants.DESIRED, tempSelected);
            updateValues.putOpt(Constants.THERMO_MODE, selectedItemMode);
            JSONObject termoIdObj = new JSONObject();
            termoIdObj.putOpt(deviceName, updateValues);

            JSONObject termostatOBJ = new JSONObject();
            termostatOBJ.putOpt(Constants.THERMOSTAT, termoIdObj);

            JSONObject deviceObj = new JSONObject();
            deviceObj.putOpt(subDeviceName, termostatOBJ);

            final JSONObject desiredObj = new JSONObject();
            desiredObj.putOpt(Constants.DESIRED, deviceObj);

            desiredThermobj = new ThermoUpdateModel
                    (selectedItemMode, tempSelected, thresholdInt);

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

                    //    BleServiceCharecteristic.getInstance().writeOfflineData(Util.RequestSensorData(mainDeviceName, subDeviceName));
                }
            }, 4000);

//            Handler handler4 = new Handler();
//            handler4.postDelayed(new Runnable() {
//                public void run() {
//                    BleServiceCharecteristic.getInstance().readOfflineData();
//                }
//            }, 6000);
//
//            Handler handler3 = new Handler();
//            handler3.postDelayed(new Runnable() {
//                public void run() {
//                    String status = BleServiceCharecteristic.getInstance().readOfflineData();
//                    Logger.d(TAG, "thermostat updated data : " + status);
//                }
//            }, 8000);


        } catch (Exception e) {
         Logger.d(TAG,"Exception in thermostatDataUpdateToBoard() : "+e);
        }
    }

    private void bindThermostatData(String data, String key) {
        JSONObject thermostatObject = null, thermoValues = null;
        try {
            JSONObject jsonObject = new JSONObject(data);
            if (!jsonObject.has(Constants.THERMOSTAT)) {
                noThermoDetails.setVisibility(View.VISIBLE);
                thermolayt.setVisibility(View.GONE);
                return;
            }
            thermostatObject = jsonObject.getJSONObject(Constants.THERMOSTAT);
            Iterator<String> keys = thermostatObject.keys();
            while (keys.hasNext()) {
                currentDynamicKey = keys.next();
                thermostatList.add(currentDynamicKey);
            }
            deviceName = thermostatList.get(0);
        } catch (JSONException e) {
            Logger.d(TAG, "Exception bindThermostatData() : " + e);
            e.printStackTrace();
        }
        if (thermostatObject != null) {
            try {
                thermoValues = thermostatObject.getJSONObject(deviceName);
                Logger.d(TAG, "thermoValues.." + thermostatObject);
                try {
                    thermoPollingcount = 0;
                    operating_state = thermoValues.getString(Constants.THERMO_OPR_STATE);
                    mOperatingMode = thermoValues.getString(Constants.THERMO_MODE);
                    temp = thermoValues.getInt(Constants.THERMO_CUR_STATE);
                    desiredTemp = thermoValues.getInt(Constants.DESIRED);
                    threshold = thermoValues.getInt(Constants.THERMO_THRESHOLD);
                    updateUIChanges(operating_state.toUpperCase());
                    currentTempTextView.setText(temp + "" + (char) Constants.DEGREEVALUE + Constants.DEGREEUNIT);

                    // Checking the reported object same as Desired obj :
                    if (isUpdating) {
                        reportedTheroBj = new ThermoUpdateModel(mOperatingMode, desiredTemp, threshold);
                        if (desiredThermobj.equals(reportedTheroBj)) {
                            isUpdating = false;
                            applyFailCount = 0;
                            Logger.d(TAG, "OBJ EQUAL");
                            key = Constants.UPDATE;
                            Toast.makeText(getActivity(), Constants.UPDATION_THERMO_SUCCESS, Toast.LENGTH_SHORT).show();
                        } else {
                            applyFailCount=applyFailCount+1;
                            if (applyFailCount > Constants.THERMO_APPLY_FAIL_COUNT){
                                isUpdating=false;
                                applyFailCount=0;
                                key = Constants.UPDATE;
                                Toast.makeText(getActivity(), Constants.UPDATION_THERMO, Toast.LENGTH_SHORT).show();
                            }
                        }
                    }

                    // update seekbar and its value UI only for first time data and updating.
                    if (key.equals(Constants.FIRST) || key.equals(Constants.UPDATE)) {
                        desiredTempTextView.setText(desiredTemp + "" + (char) Constants.DEGREEVALUE + Constants.DEGREEUNIT);
                        thresholdValue.setText(threshold + "");
                        seekBardesiredTemp.setProgress(desiredTemp);
                        seekBarThreshold.setProgress(threshold);
                        try {
                            String operatingmode = mOperatingMode.toUpperCase();
                            if (!operatingmode.equals(null)) {
                                int spinnerPosition = adapter.getPosition(operatingmode);
                                spinnerData.setSelection(spinnerPosition);
                            }
                            apply.setClickable(true);
                            apply.setText(R.string.APPLY);
                            dialog.dismiss();
                        } catch (Exception e) {
                            Logger.e(TAG, "e3.." + e);
                            e.printStackTrace();
                        }
                    }
                } catch (Exception e) {
                    Logger.e(TAG, "e2.." + e);
                    e.printStackTrace();
                }
            } catch (Exception e) {
                Logger.e(TAG, "e1.." + e);
                e.printStackTrace();
            }
        } else {
            Logger.d(TAG, "thermostat object null");
        }
    }


    private void initialization(View view) {
        thermostatList = new ArrayList<String>();
        cancel = (Button) view.findViewById(R.id.buttonCancel);
        apply = (Button) view.findViewById(R.id.buttonApply);
        noThermoDetails = (TextView) view.findViewById(R.id.noThermoDetails);
        thermolayt = (ConstraintLayout) view.findViewById(R.id.thermolayt);
        desiredTempTextView = (TextView) view.findViewById(R.id.textViewDesiredTemp);
        thresholdValue = (TextView) view.findViewById(R.id.textViewThreshold);
        currentTempTextView = (TextView) view.findViewById(R.id.textViewCurrentTemp);
        textviewOperatingState = (TextView) view.findViewById(R.id.textViewOperatingState);
        spinnerData = (Spinner) view.findViewById(R.id.spinnerTempModeList);
        seekBardesiredTemp = (SeekBar) view.findViewById(R.id.seekBar);
        seekBarThreshold = (SeekBar) view.findViewById(R.id.seekBarThreshold);
        imageView = (ImageView) view.findViewById(R.id.imageViewOperatingState);
        dialog = new ProgressDialog(getActivity());
        LocalBroadcastManager.getInstance(getActivity()).registerReceiver(myBroadcastReceiver,
                new IntentFilter(Constants.INTENT_FILTER_SENSOR_DATA));
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        LocalBroadcastManager.getInstance(getActivity()).unregisterReceiver(myBroadcastReceiver);
    }

    private void displayData() {
        seekBardesiredTemp.setMax(Constants.SEEKMAX);
        seekBarThreshold.setMax(Constants.SEEKTHRESHOLD);
        final List<String> mode = new ArrayList<String>();
        mode.add(Constants.AUTO);
        mode.add(Constants.AC);
        mode.add(Constants.HEATER);
        mode.add(Constants.OFF);
        adapter = new ArrayAdapter<String>(getActivity(), android.R.layout.simple_spinner_dropdown_item, mode);
        spinnerData.setAdapter(adapter);
    }

    private void updateUIChanges(String opStateValue) {
        if (opStateValue.equals(Constants.AC_ON)) {
            textviewOperatingState.setText(R.string.AC_ON);
            textviewOperatingState.setTextColor(getResources().getColor(R.color.acOn));
            imageView.setImageDrawable(getResources().getDrawable(R.drawable.acon));
        } else if (opStateValue.equals(Constants.HEATER_ON)) {
            textviewOperatingState.setText(R.string.HEATER_ON);
            textviewOperatingState.setTextColor(getResources().getColor(R.color.heater));
            imageView.setImageDrawable(getResources().getDrawable(R.drawable.heateron));
        } else if (opStateValue.equals(Constants.STANDBY)) {
            textviewOperatingState.setText(R.string.STAND_BY);
            textviewOperatingState.setTextColor(getResources().getColor(R.color.auto));
            imageView.setImageDrawable(getResources().getDrawable(R.drawable.standby));
        } else if (opStateValue.equals(Constants.OFF)) {
            textviewOperatingState.setText(R.string.OFF);
            textviewOperatingState.setTextColor(getResources().getColor(R.color.off));
            imageView.setImageDrawable(getResources().getDrawable(R.drawable.off));
        }

    }

}
