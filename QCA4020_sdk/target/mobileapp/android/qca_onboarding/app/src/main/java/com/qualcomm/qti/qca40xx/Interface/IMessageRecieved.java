/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package com.qualcomm.qti.qca40xx.Interface;

/**
 * Interface to receive response from TCP communication
 */

public interface IMessageRecieved {

    public void tcpMessageRecieved(String message);
}
