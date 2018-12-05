/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package com.qualcomm.qti.qca40xx.sensor.Service;

import android.app.IntentService;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.AsyncTask;
import android.os.IBinder;
import android.support.annotation.Nullable;
import android.support.v4.content.LocalBroadcastManager;

import com.amazonaws.auth.AWSCredentials;
import com.amazonaws.auth.AWSCredentialsProvider;
import com.amazonaws.auth.BasicAWSCredentials;
import com.amazonaws.mobileconnectors.iot.AWSIotMqttClientStatusCallback;
import com.amazonaws.mobileconnectors.iot.AWSIotMqttManager;
import com.amazonaws.mobileconnectors.iot.AWSIotMqttNewMessageCallback;
import com.amazonaws.mobileconnectors.iot.AWSIotMqttQos;
import com.amazonaws.services.iot.AWSIotClient;
import com.qualcomm.qti.qca40xx.sensor.Utils.Constants;
import com.qualcomm.qti.qca40xx.sensor.Utils.Logger;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.UnsupportedEncodingException;
import java.util.Iterator;
import java.util.UUID;

/**
 * NotificationService is a service runs in background. This class will look for breach
 */

public class NotificationService extends IntentService implements AWSCredentialsProvider {

    public static final int STATUS_RUNNING = 0;
    public static final String ACTION = Constants.ACTION;
    private static final String TAG = "NotificationService";
    private final String topic = Constants.THRESHOLD_BREACHED;
    private AWSIotMqttManager mqttManager;
    private String clientId;
    private SharedPreferences sharedpreferences;
    private String accessKey, secreteKey, customer_endpoint;
    private BasicAWSCredentials sAWSCredentials;
    private AWSIotClient client;
    private AWSCredentials awsCredentials;
    private AWSCredentialsProvider awsCredentialsProvider;
    private String message;
    private boolean isTopicConnected = false;
    private String serviceName;
    private String mThingKey;
    private String mDeviceKey;
    private String mMessageKey;


    public NotificationService() {
        super(NotificationService.class.getName());
    }


    @Override
    protected void onHandleIntent(@Nullable Intent intent) {
        serviceName = intent.getStringExtra(Constants.SERVICE_NAME);
        initializeTopic();
    }


    private void publishResults(String serviceName, int result, String message) {
        Intent intent = new Intent(ACTION);
        // Put extras into the intent as usual
        intent.putExtra(Constants.SERVICE_NAME, serviceName);
        intent.putExtra(Constants.RESULT_VALUE, result);
        intent.putExtra(Constants.THRESHOLD_MESSAGE, message);
        // Fire the broadcast with intent packaged
        LocalBroadcastManager.getInstance(this).sendBroadcast(intent);
    }

    private void GetKeyFromSharedPreference() {

        sharedpreferences = this.getSharedPreferences(Constants.KEY,
                Context.MODE_PRIVATE);
        accessKey = sharedpreferences.getString(Constants.ACCESS_KEY, "");
        secreteKey = sharedpreferences.getString(Constants.SECRET_KEY, "");
        customer_endpoint = sharedpreferences.getString(Constants.CUSTOMER_ENDPOINT, "");

    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {

        //speedExceedMessageToActivity();
        return super.onStartCommand(intent, flags, startId);
    }

    @Override
    public void onCreate() {
        super.onCreate();

    }

    @Nullable
    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public AWSCredentials getCredentials() {
        return awsCredentials;
    }

    @Override
    public void refresh() {

    }

    private void initializeTopic() {

        clientId = UUID.randomUUID().toString();
        GetKeyFromSharedPreference();

        if (sAWSCredentials == null) {
            sAWSCredentials = new BasicAWSCredentials(accessKey, secreteKey);
        }

        try {
            client = new AWSIotClient(sAWSCredentials);

            awsCredentials = (AWSCredentials) sAWSCredentials;
            awsCredentialsProvider = NotificationService.this;

            AWSCredentials credentials = awsCredentialsProvider.getCredentials();

            // MQTT Client
            mqttManager = new AWSIotMqttManager(clientId, customer_endpoint);

            GetTopicConnection getTopicConnection = new GetTopicConnection();
            getTopicConnection.execute();

        } catch (Exception e) {
            e.printStackTrace();
        }

    }


    private void connectForTopic() {
        try {
            mqttManager.connect(awsCredentialsProvider, new AWSIotMqttClientStatusCallback() {
                @Override
                public void onStatusChanged(AWSIotMqttClientStatus status, Throwable throwable) {
                    Logger.d(TAG, "Status = " + String.valueOf(status));
                    if (String.valueOf(status).equals(Constants.CONNECTED)) {
                        isTopicConnected = true;
                        Logger.d(TAG, "isConnected" + isTopicConnected);
                    }
                }
            });
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private void subscripeTopic() {
        try {

            if (isTopicConnected) {
                Logger.d(TAG, "isTopicConnected " + isTopicConnected);
                mqttManager.subscribeToTopic(topic, AWSIotMqttQos.QOS0, new AWSIotMqttNewMessageCallback() {

                    @Override
                    public void onMessageArrived(String topic, byte[] data) {

                        try {

                            message = new String(data, "UTF-8");
                            Logger.d(TAG, "message " + message);
                            JSONObject jsonObject = new JSONObject(message);
                            Iterator<String> keys = jsonObject.keys();
                            while (keys.hasNext()) {
                                mThingKey = keys.next();
                                JSONObject jsonObject1 = jsonObject.getJSONObject(mThingKey);
                                Iterator<String> keys1 = jsonObject1.keys();
                                while (keys1.hasNext()) {
                                    mDeviceKey = keys1.next();
                                    JSONObject jsonObject2 = jsonObject1.getJSONObject(mDeviceKey);
                                    String msgBreach = jsonObject2.getString(Constants.MESSAGE);
                                    mMessageKey = msgBreach + Constants.FROM + mDeviceKey + Constants.OF + mThingKey;
                                }
                            }

                            publishResults(serviceName, STATUS_RUNNING, mMessageKey);
                        } catch (UnsupportedEncodingException e) {
                            e.printStackTrace();
                        } catch (JSONException e) {
                            e.printStackTrace();
                        }

                    }
                });
            } else {
                Logger.d(TAG, "Not  isTopicConnected " + isTopicConnected);
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private class GetTopicConnection extends AsyncTask<Void, Void, Void> {

        @Override
        protected Void doInBackground(Void... voids) {

            connectForTopic();
            try {
                Thread.sleep(3000);
            } catch (Exception e) {

            }

            return null;
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            super.onPostExecute(aVoid);
            subscripeTopic();
        }
    }


}
