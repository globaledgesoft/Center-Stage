/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package com.qualcomm.qti.qca40xx.Interface;

import android.bluetooth.le.ScanResult;

import java.util.List;

/**
 * Interface to get the list of BLE devices
 */

public interface IBleDeviceList {
    public void onBleDeviceList(List<ScanResult> data);
}
