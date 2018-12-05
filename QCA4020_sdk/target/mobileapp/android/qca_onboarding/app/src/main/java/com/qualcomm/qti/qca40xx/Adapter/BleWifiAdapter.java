/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package com.qualcomm.qti.qca40xx.Adapter;

import android.app.ProgressDialog;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.net.wifi.ScanResult;
import android.net.wifi.WifiConfiguration;
import android.net.wifi.WifiManager;
import android.support.v7.widget.PopupMenu;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import com.qualcomm.qti.qca40xx.Interface.IQRScan;
import com.qualcomm.qti.qca40xx.Interface.IWiFiDevice;
import com.qualcomm.qti.qca40xx.Interface.ItemClickListener;
import com.qualcomm.qti.qca40xx.Util.ConnectAP;
import com.qualcomm.qti.qca40xx.Util.ConnectBle;
import com.qualcomm.qti.qca40xx.Util.Constants;
import com.qualcomm.qti.qca40xx.Util.Logger;
import com.qualcomm.qti.qca40xx.Util.Util;
import com.qualcomm.qti.qca40xx.R;

import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.List;
import java.util.Set;

/**
 * BleWifiAdapter is an Adapter class which hold the list of the BLE and Wi-Fi Boards that are scanned
 */

public class BleWifiAdapter extends RecyclerView.Adapter<RecyclerView.ViewHolder> {
    private static final String TAG = "BleWifiAdapter";
    private final static int TYPE_WIFI = 1, TYPE_BLE = 2;
    String checkForOfflineDevice = "";
    private List<ScanResult> mWifiScanList;
    private Context mContext;
    private ProgressDialog mProgressDialog;
    private String wifiName, wifiAddress;
    private IntentFilter intentFilter;
    private String menuClick;
    BroadcastReceiver receiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();


            if (action.equals(BluetoothDevice.ACTION_BOND_STATE_CHANGED)) {
                int state = intent.getIntExtra(BluetoothDevice.EXTRA_BOND_STATE, -1);

                if (state < 0) {
                    Toast.makeText(mContext, "Please try again", Toast.LENGTH_LONG).show();
                    //we should never get here
                } else if (state == BluetoothDevice.BOND_BONDING) {
                    //bonding process is still working
                    //essentially this means that the Confirmation Dialog is still visible
                    showProgress(true);
                    Logger.d(TAG, "Start Pairing...");
                } else if (state == BluetoothDevice.BOND_BONDED) {
                    //bonding process was successful
                    //also means that the user pressed OK on the Dialog
                    Logger.d(TAG, "Paired");
                    showProgress(false);
                    mContext.unregisterReceiver(receiver);
                    new ConnectBle(mContext, Constants.MANUAL_SCANNING, menuClick);
                } else if (state == BluetoothDevice.BOND_NONE) {
                    Logger.d(TAG, "not Paired");
                    showProgress(false);
                    Toast.makeText(mContext, R.string.toastNotPairing, Toast.LENGTH_LONG).show();

                    //bonding process failed
                    //which also means that the user pressed CANCEL on the Dialog
                }
            }
        }
    };
    private List<Object> bleWifiList = new ArrayList();
    private IQRScan iqrScan;
    private IWiFiDevice wifiDeviceSelected;

    public void addWifiBle(List<Object> bleWifiList, Context mContext, IWiFiDevice wDevice, IQRScan qrScan) {
        this.bleWifiList = bleWifiList;
        this.mContext = mContext;
        this.wifiDeviceSelected = wDevice;
        this.iqrScan = qrScan;
    }


    // We need to override this as we need to differentiate
    // which type viewHolder to be attached
    // This is being called from onBindViewHolder() method
    @Override
    public int getItemViewType(int position) {
        if (bleWifiList.get(position) instanceof ScanResult) {
            return TYPE_WIFI;
        } else if (bleWifiList.get(position) instanceof android.bluetooth.le.ScanResult) {
            return TYPE_BLE;
        }
        return -1;
    }

    // Invoked by layout manager to create new views
    @Override
    public RecyclerView.ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        // Attach layout for single cell
        int layout = 0;
        RecyclerView.ViewHolder viewHolder;
        // Identify viewType returned by getItemViewType(...)
        // and return ViewHolder Accordingly
        switch (viewType) {
            case TYPE_WIFI:
                layout = R.layout.cardview_item_layout;
                View wifiView = LayoutInflater.from(parent.getContext()).inflate(layout, parent, false);
                viewHolder = new WifiViewHolder(wifiView);
                break;
            case TYPE_BLE:
                layout = R.layout.cardview_item_layout;
                View bleView = LayoutInflater.from(parent.getContext()).inflate(layout, parent, false);
                viewHolder = new BleViewHolder(bleView);
                break;
            default:
                viewHolder = null;
                break;
        }
        return viewHolder;
    }

    @Override
    public void onBindViewHolder(final RecyclerView.ViewHolder holder, int position) {

        int viewType = holder.getItemViewType();
        switch (viewType) {
            case TYPE_WIFI:
                final ScanResult wifiDevice = (ScanResult) bleWifiList.get(position);
                ((WifiViewHolder) holder).bindWifiData(wifiDevice);
                ((WifiViewHolder) holder).menuOptions.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View view) {
                        //creating a popup menu
                        PopupMenu popup = new PopupMenu(mContext, ((WifiViewHolder) holder).menuOptions);
                        //inflating menu from xml resource
                        popup.inflate(R.menu.wifi_main_menu);
                        //adding click listener
                        popup.setOnMenuItemClickListener(new PopupMenu.OnMenuItemClickListener() {
                            @Override
                            public boolean onMenuItemClick(MenuItem item) {
                                switch (item.getItemId()) {
                                    case R.id.wifi_onboard_menu_item:
                                        try {
                                            checkWifiConfiguration(wifiDevice);
                                            if (wifiDevice.capabilities.contains("WEP") || wifiDevice.capabilities.contains("WPA")) {
                                                wifiDeviceSelected.wifiDeviceSelected(wifiDevice);
                                            } else {
                                                new ConnectAP(mContext, iqrScan, Constants.MANUAL_SCANNING).connectToWifi(wifiDevice, wifiDevice.SSID, "");
                                            }
                                        } catch (Exception e) {
                                            e.printStackTrace();
                                        }

                                        break;
                                    case R.id.wifi_sensor_menu_item:

                                        Intent launchIntent = mContext.getPackageManager().getLaunchIntentForPackage(Constants.ONLINE_APP);
                                        if (launchIntent != null) {
                                            mContext.startActivity(launchIntent);//null pointer check in case package name was not found
                                        } else {
                                            Toast.makeText(mContext, "QCASensor Application not installed.", Toast.LENGTH_LONG).show();

                                        }
                                        break;
                                }
                                return false;
                            }
                        });
                        //displaying the popup
                        popup.show();
                    }
                });
                break;

            case TYPE_BLE:
                final android.bluetooth.le.ScanResult result = (android.bluetooth.le.ScanResult) bleWifiList.get(position);
                final BluetoothDevice bleDevice = (BluetoothDevice) result.getDevice();
                ((BleViewHolder) holder).bindBleData(result);
                ((BleViewHolder) holder).menuOptions.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View view) {
                        if (result.getScanRecord().getServiceUuids() != null) {
                            int offlineDeviceCount = result.getScanRecord().getServiceUuids().size();
                            //creating a popup menu
                            PopupMenu popup = new PopupMenu(mContext, ((BleViewHolder) holder).menuOptions);
                            boolean isOnboard = false;
                            boolean isOffline = false;
                            boolean isCoordinatorOnboarded = false;

                            for (int j = 0; j < offlineDeviceCount; j++) {
                                if (result.getScanRecord().getServiceUuids().get(j).getUuid().equals(Constants.GATT_OFFLINE_ONBOARDED_CHARACTERISTIC)) {
                                    isOnboard = true;
                                }
                                if (result.getScanRecord().getServiceUuids().get(j).getUuid().equals(Constants.GATT_OFFLINE_ADVERTISEMENT_CHARACTERISTIC)) {
                                    isOffline = true;
                                    checkForOfflineDevice = Constants.OFFLINE_ONBOARD_DEVICE;
                                }
                                if (result.getScanRecord().getServiceUuids().get(j).getUuid().equals(Constants.GATT_OFFLINE_ONBOARDED_COORDINATOR_CHARACTERISTIC)) {
                                    isCoordinatorOnboarded = true;
                                }
                            }

                            if (isOnboard && isOffline) {
                                //inflating menu from xml resource
                                popup.inflate(R.menu.onboard_dboard_menu);
                            } else {
                                //inflating menu from xml resource
                                if (isOffline && isCoordinatorOnboarded) {
                                    popup.inflate(R.menu.coordinator_menu);
                                } else {
                                    popup.inflate(R.menu.ble_main_menu);
                                }
                            }
                            //adding click listener
                            popup.setOnMenuItemClickListener(new PopupMenu.OnMenuItemClickListener() {
                                @Override
                                public boolean onMenuItemClick(MenuItem item) {
                                    switch (item.getItemId()) {
                                        case R.id.onboard_menu_item:
                                            if (bleDevice != null) {
                                                if (checkForOfflineDevice.equals(Constants.OFFLINE_ONBOARD_DEVICE))
                                                    bleConnection(bleDevice, Constants.ONBOARD, checkForOfflineDevice);
                                                else
                                                    bleConnection(bleDevice, Constants.ONBOARD, Constants.ONLINE_ONBOARD_DEVICE);
                                            }

                                            break;
                                        case R.id.sensor_menu_item:
                                            boolean isOffline = false;
                                            if (bleDevice != null) {
                                                if (result != null) {
                                                    int offlineDeviceCount = result.getScanRecord().getServiceUuids().size();
                                                    for (int j = 0; j < offlineDeviceCount; j++) {
                                                        if (result.getScanRecord().getServiceUuids().get(j).getUuid().equals(Constants.GATT_OFFLINE_ADVERTISEMENT_CHARACTERISTIC)) {
                                                            isOffline = true;
                                                        }
                                                    }
                                                    if (isOffline) {
                                                        bleConnection(bleDevice, Constants.OFFLINE, Constants.OFFLINE_ONBOARD_DEVICE);
                                                    } else {
                                                        Intent launchIntent = mContext.getPackageManager().getLaunchIntentForPackage(Constants.ONLINE_APP);
                                                        if (launchIntent != null) {
                                                            mContext.startActivity(launchIntent);//null pointer check in case package name was not found
                                                        } else {
                                                            Toast.makeText(mContext, "QCASensor Application not installed.", Toast.LENGTH_LONG).show();
                                                        }
                                                    }
                                                }
                                            }
                                            break;
                                        case R.id.coordinator_present_menu:
                                            boolean isOffline1 = false;
                                            if (bleDevice != null) {
                                                if (result != null) {
                                                    int offlineDeviceCount = result.getScanRecord().getServiceUuids().size();
                                                    for (int j = 0; j < offlineDeviceCount; j++) {
                                                        if (result.getScanRecord().getServiceUuids().get(j).getUuid().equals(Constants.GATT_OFFLINE_ADVERTISEMENT_CHARACTERISTIC)) {
                                                            isOffline1 = true;
                                                        }
                                                    }
                                                    if (isOffline1) {
                                                        bleConnection(bleDevice, Constants.OFFLINE, Constants.OFFLINE_ONBOARD_DEVICE);
                                                    } else {
                                                        Intent launchIntent = mContext.getPackageManager().getLaunchIntentForPackage(Constants.ONLINE_APP);
                                                        if (launchIntent != null) {
                                                            mContext.startActivity(launchIntent);//null pointer check in case package name was not found
                                                        } else {
                                                            Toast.makeText(mContext, "QCASensor Application not installed.", Toast.LENGTH_LONG).show();
                                                        }
                                                    }
                                                }
                                            }
                                            break;
                                    }
                                    return false;
                                }
                            });
                            //displaying the popup
                            popup.show();
                        } else {
                            Toast.makeText(mContext, R.string.toastNoBoardInfo, Toast.LENGTH_LONG).show();
                        }
                    }
                });
                break;

        }
    }

    //
    private void bleConnection(BluetoothDevice bleDevice, String menuClick, String isOfflineOnboard) {
        this.menuClick = menuClick;
        BluetoothAdapter mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        Set<BluetoothDevice> pairedDevices = mBluetoothAdapter.getBondedDevices();

        for (BluetoothDevice bt : pairedDevices) {
            if (bt.getAddress().contains(bleDevice.getAddress())) {
                Util.unParingDevice(bt);
            }
        }
        if (bleDevice != null)
            saveBleDevice(bleDevice, isOfflineOnboard);
        if (bleDevice.getBondState() == BluetoothDevice.BOND_BONDED) {
            new ConnectBle(mContext, Constants.MANUAL_SCANNING, menuClick);
        } else {
            pairDevice(bleDevice);
        }
    }

    //wifi forget network
    private void checkWifiConfiguration(ScanResult wifiDevice) {
        WifiManager wifiManager = (WifiManager) mContext.getSystemService(Context.WIFI_SERVICE);
        List<WifiConfiguration> list = wifiManager.getConfiguredNetworks();
        for (WifiConfiguration i : list) {
            String configuredDevice = i.SSID;
            String connectingDvice = wifiDevice.SSID;
            if (configuredDevice.contains("\"")) {
                configuredDevice = configuredDevice.replace("\"", "");
            }
            if (connectingDvice.contains("\"")) {
                connectingDvice = connectingDvice.replace("\"", "");
            }

            if (configuredDevice.equalsIgnoreCase(connectingDvice)) {
                int networkId = wifiManager.getConnectionInfo().getNetworkId();
                wifiManager.removeNetwork(networkId);
                wifiManager.saveConfiguration();
            }
        }
    }

    private void pairDevice(BluetoothDevice device) {
        try {
            IntentFilter intentFilter = new IntentFilter();
            intentFilter.addAction(BluetoothDevice.ACTION_BOND_STATE_CHANGED);
            mContext.registerReceiver(receiver, intentFilter);
            mProgressDialog = new ProgressDialog(mContext);
            mProgressDialog.setMessage(Constants.PAIR_DIALOG);
            Method m = device.getClass().getMethod(Constants.CREATE_BOND, (Class[]) null);
            m.invoke(device, (Object[]) null);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public void showProgress(boolean show) {

        if (show) {
            mProgressDialog.setCancelable(false);
            if (!mProgressDialog.isShowing())
                mProgressDialog.show();
        } else {
            Logger.i(TAG, " " + mProgressDialog.isShowing());
            mProgressDialog.setCancelable(true);
            if (mProgressDialog.isShowing()) {
                mProgressDialog.dismiss();
            }
        }
    }


    private void saveBleDevice(BluetoothDevice device, String isOffline) {
        String bleName = device.getName();
        SharedPreferences sharedPreferences = mContext.getSharedPreferences(Constants.BLUETOOTH_DEVICE, Context.MODE_PRIVATE);
        SharedPreferences.Editor editor = sharedPreferences.edit();
        if (device.getName().contains("\"")) {
            bleName = bleName.replace("\"", "");
        }
        editor.putString(Constants.BLE_DEVICE_NAME, bleName);
        editor.putString(Constants.BLE_DEVICE_ADDRESS, device.getAddress());
        if (isOffline.equals(Constants.OFFLINE_ONBOARD_DEVICE))
            editor.putString(Constants.OFFLINE, Constants.OFFLINE_ONBOARD_DEVICE);
        else
            editor.putString(Constants.OFFLINE, Constants.ONLINE_ONBOARD_DEVICE);
        editor.commit();
    }

    public void clear() {
        bleWifiList.clear();
    }


    @Override
    public int getItemCount() {
        return bleWifiList.size();
    }

    public class WifiViewHolder extends RecyclerView.ViewHolder implements View.OnClickListener {
        public TextView name, address, menuOptions, awsConfigure;
        ImageView wifiImage;
        private ItemClickListener clickListener;

        public WifiViewHolder(View view) {
            super(view);
            wifiImage = (ImageView) view.findViewById(R.id.cardview_imageview_ble_wifi);
            name = (TextView) view.findViewById(R.id.cardview_textview_ble_name);
            address = (TextView) view.findViewById(R.id.cardview_textview_ble_mac_addr);
            menuOptions = (TextView) view.findViewById(R.id.cardview_textView_menu_options);
            awsConfigure = (TextView) view.findViewById(R.id.cardview_textview_device_configuration);
            view.setTag(itemView);
            view.setOnClickListener(this);
        }


        public void bindWifiData(ScanResult device) {
            wifiName = device.SSID;
            if (device.SSID.contains("\"")) {
                wifiName = wifiName.replace("\"", "");
            }
            wifiAddress = device.BSSID;
            name.setText(wifiName);
            address.setText(wifiAddress);
            wifiImage.setImageResource(R.drawable.wifi);
            awsConfigure.setVisibility(View.VISIBLE);
        }

        public void setClickListener(ItemClickListener itemClickListener) {
            this.clickListener = itemClickListener;
        }

        @Override
        public void onClick(View view) {
            if (clickListener != null)
                clickListener.onClick(view, getPosition(), true);
        }
    }

    public class BleViewHolder extends RecyclerView.ViewHolder implements View.OnClickListener {
        public TextView name, address, menuOptions, awsConfigure;
        ImageView bleImage;
        private ItemClickListener clickListener;

        public BleViewHolder(View view) {
            super(view);
            bleImage = (ImageView) view.findViewById(R.id.cardview_imageview_ble_wifi);
            name = (TextView) view.findViewById(R.id.cardview_textview_ble_name);
            address = (TextView) view.findViewById(R.id.cardview_textview_ble_mac_addr);
            menuOptions = (TextView) view.findViewById(R.id.cardview_textView_menu_options);
            awsConfigure = (TextView) view.findViewById(R.id.cardview_textview_device_configuration);
            view.setTag(itemView);
            view.setOnClickListener(this);
        }

        public void bindBleData(android.bluetooth.le.ScanResult device) {
            // Attach values for each item
            String bleName = device.getDevice().getName();
            if (bleName.contains("\"")) {
                bleName = bleName.replace("\"", "");
            }
            String bleAddress = device.getDevice().getAddress();
            name.setText(bleName);
            address.setText(bleAddress);
            bleImage.setImageResource(R.drawable.ble);
            if (device.getScanRecord().getServiceUuids() != null) {
                int offlineDeviceCount = device.getScanRecord().getServiceUuids().size();
                for (int j = 0; j < offlineDeviceCount; j++) {
                    if (device.getScanRecord().getServiceUuids().get(j).getUuid().equals(Constants.GATT_OFFLINE_ADVERTISEMENT_CHARACTERISTIC)) {
                        awsConfigure.setVisibility(View.GONE);
                    }
                }
            }
        }

        public void setClickListener(ItemClickListener itemClickListener) {
            this.clickListener = itemClickListener;
        }

        @Override
        public void onClick(View view) {
            if (clickListener != null)
                clickListener.onClick(view, getPosition(), true);
        }
    }

}
