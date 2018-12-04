/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package com.qualcomm.qti.qca40xx.Util;

import android.app.ProgressDialog;
import android.content.Context;
import android.os.AsyncTask;

import com.qualcomm.qti.qca40xx.Interface.IMessageRecieved;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.net.InetAddress;
import java.net.Socket;

/**
 * This class is for tcp communication purpose
 */

public class TCPComm {

    private static final String TAG = "TCPComm";
    Socket socket;
    int charsRead = 0;
    char[] buffer = new char[1024];
    private InetAddress serverAddr;
    private Context mContext;
    private IMessageRecieved iMessageRecieved;
    private BufferedWriter mBufferOutput;
    private BufferedReader mBufferReader;
    private boolean isCommunicating = false;
    private String msgReceived;
    private ProgressDialog dialog;
    private String sendingMessage;


    public TCPComm(InetAddress serverAddr, Context mContext, IMessageRecieved tcpMessage, String sendMsg) {
        this.serverAddr = serverAddr;
        this.mContext = mContext;
        this.iMessageRecieved = tcpMessage;
        this.sendingMessage = sendMsg;

        TCPCommTask tcpCommTask = new TCPCommTask();
        tcpCommTask.execute();
    }

    public TCPComm(Context mContext) {

        this.mContext = mContext;
    }


    public void sendMessagetoBoard(String message) {

        if (mBufferOutput != null) {
            try {
                mBufferOutput.write(message);
                mBufferOutput.flush();


            } catch (Exception e) {
                Logger.e(TAG, "BufferdWriter " + e.getMessage());
            }

        }
    }

    public void inititeCommunication() {

        isCommunicating = true;


        try {
            socket = new Socket(serverAddr, Constants.PORT);
            Logger.d(TAG, " socket = " + socket);
            socket.setSoTimeout(10000);
            try {
                OutputStream os = socket.getOutputStream();
                OutputStreamWriter osw = new OutputStreamWriter(os);
                mBufferOutput = new BufferedWriter(osw);

                sendMessagetoBoard(sendingMessage);

                Logger.d(TAG, " message sent " + sendingMessage);
                InputStream is = socket.getInputStream();
                InputStreamReader isr = new InputStreamReader(is);
                mBufferReader = new BufferedReader(isr);

                charsRead = mBufferReader.read(buffer);
                msgReceived = new String(buffer).substring(0, charsRead);

                Logger.d(TAG, " " + msgReceived);


            } catch (Exception e) {
                Logger.e(TAG, "Error " + e.getMessage());
            } finally {
                socket.close();
            }

        } catch (Exception e) {
            Logger.e(TAG, "Error " + e.getMessage());
            if (dialog.isShowing()) {
                dialog.dismiss();
            }
        }
    }

    public void stopConnection() {
        isCommunicating = false;
        if (mBufferOutput != null) {
            try {
                mBufferOutput.flush();
                mBufferOutput.close();
            } catch (Exception e) {
                Logger.e(TAG, "Error " + e.getMessage());
            }

        }
    }


    private class TCPCommTask extends AsyncTask<Void, Void, Void> {

        protected void onPreExecute() {
            super.onPreExecute();
            if (dialog != null) {
                if (dialog.isShowing()) {
                    dialog.dismiss();
                    dialog = null;
                }
            }else
            {
                Logger.d(TAG, "onPreExecute() dialog is null");
            }
            dialog = new ProgressDialog(mContext);
            dialog.setMessage("Receiving Data From board");
            dialog.setCancelable(false);
            dialog.show();
        }

        @Override
        protected Void doInBackground(Void... voids) {

            inititeCommunication();
            return null;
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            super.onPostExecute(aVoid);

            if (msgReceived != null && iMessageRecieved != null) {
                iMessageRecieved.tcpMessageRecieved(msgReceived);
                Logger.d(TAG, "Message Received " + msgReceived);
            } else {
                iMessageRecieved.tcpMessageRecieved("");
                Logger.d(TAG, "Message Not Received ");
            }
            stopConnection();
            if (dialog.isShowing()) {
                dialog.dismiss();
            }
        }
    }


}
