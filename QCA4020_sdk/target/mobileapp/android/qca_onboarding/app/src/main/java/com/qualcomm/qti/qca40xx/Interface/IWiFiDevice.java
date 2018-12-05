/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package com.qualcomm.qti.qca40xx.Interface;

import android.net.wifi.ScanResult;

/**
 * Interface to get the list of Wi-Fi devices
 */

public interface IWiFiDevice {

    public void wifiDeviceSelected(ScanResult wifiDeviceSelected);
}
