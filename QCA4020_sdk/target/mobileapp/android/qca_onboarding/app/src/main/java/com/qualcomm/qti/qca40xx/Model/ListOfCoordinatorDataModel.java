package com.qualcomm.qti.qca40xx.Model;

/**
 * Created by sneha on 28/2/18.
 */

public class ListOfCoordinatorDataModel {
    public String getDeviceName() {
        return deviceName;
    }

    public void setDeviceName(String deviceName) {
        this.deviceName = deviceName;
    }

    public String getDeviceMac() {
        return DeviceMac;
    }

    public void setDeviceMac(String deviceMac) {
        DeviceMac = deviceMac;
    }

    public String getDeviceState() {
        return DeviceState;
    }

    public void setDeviceState(String deviceState) {
        DeviceState = deviceState;
    }

    String deviceName, DeviceMac, DeviceState;
}
