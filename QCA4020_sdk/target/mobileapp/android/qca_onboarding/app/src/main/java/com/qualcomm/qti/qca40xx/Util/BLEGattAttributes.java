/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package com.qualcomm.qti.qca40xx.Util;

import com.qualcomm.qti.qca40xx.Service.BLEService;

/**
 * This class includes a small subset of standard GATT attributes for demonstration purposes.
 */
public class BLEGattAttributes {
    private static BLEService mBLEService;

    public static BLEService getBLEService() {
        return mBLEService;
    }

    public static void setBLEService(BLEService bleService) {
        mBLEService = bleService;
    }
}
