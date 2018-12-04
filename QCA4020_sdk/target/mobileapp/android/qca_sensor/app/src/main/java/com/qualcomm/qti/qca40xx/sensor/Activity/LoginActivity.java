/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package com.qualcomm.qti.qca40xx.sensor.Activity;

import android.app.ProgressDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.os.AsyncTask;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.preference.PreferenceManager;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.AlertDialog;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.widget.Toast;

import com.amazonaws.regions.Regions;
import com.qualcomm.qti.qca40xx.sensor.Utils.Constants;
import com.qualcomm.qti.qca40xx.sensor.Utils.Logger;
import com.qualcomm.qti.qca40xx.sensor.R;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;


public class LoginActivity extends AppCompatActivity {

    private static final String TAG = "LoginActivity";
    private static final int PERMISSION_REQUEST_CODE = 1;
    public ArrayList<File> fileList = new ArrayList<File>();
    private CharSequence[] csvFiles = {};
    private CharSequence[] regionEndpoints = {};
    private SharedPreferences preferences;
    private SharedPreferences.Editor editor;
    private String accessKey, secreteKey;
    private String regionKey;
    private File root;
    private Toolbar toolbar;
    private boolean isAccessed;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_login);

        toolbar = (Toolbar) findViewById(R.id.bar);
        toolbar.setTitle(R.string.QualcommSensors);
        setSupportActionBar(toolbar);

        if (Build.VERSION.SDK_INT >= 23) {
            if (checkPermission()) {
                getDataAfterPermission();
            } else {
                requestPermission();
                getDataAfterPermission();
            }
        } else {
            getDataAfterPermission();
        }
    }

    //Get Permissions for Storage
    private void getDataAfterPermission() {
        preferences = PreferenceManager.getDefaultSharedPreferences(LoginActivity.this);
        editor = preferences.edit();
        GetKeyFromSharedPreference();
        if ((!accessKey.equals("")) && (!secreteKey.equals(""))) {
            if (isAccessed) {
                Intent intent = new Intent(this, ListThings.class);
                intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TASK);
                startActivity(intent);
            } else {
                checkCredential();
            }
        } else {
            checkCredential();
        }

    }

    private void GetKeyFromSharedPreference() {

        SharedPreferences sharedpreferences = getSharedPreferences(Constants.KEY,
                Context.MODE_PRIVATE);
        accessKey = sharedpreferences.getString(Constants.ACCESS_KEY, "");
        secreteKey = sharedpreferences.getString(Constants.SECRET_KEY, "");
        isAccessed = sharedpreferences.getBoolean(Constants.ACCESSED, false);


    }

    private boolean checkPermission() {
        int result = ContextCompat.checkSelfPermission(LoginActivity.this, android.Manifest.permission.WRITE_EXTERNAL_STORAGE);
        if (result == PackageManager.PERMISSION_GRANTED) {
            return true;
        } else {
            return false;
        }
    }

    private void requestPermission() {

        if (ActivityCompat.shouldShowRequestPermissionRationale(LoginActivity.this, android.Manifest.permission.WRITE_EXTERNAL_STORAGE)) {
            Toast.makeText(LoginActivity.this, Constants.PERMISSSION, Toast.LENGTH_LONG).show();
        } else {
            ActivityCompat.requestPermissions(LoginActivity.this, new String[]{android.Manifest.permission.WRITE_EXTERNAL_STORAGE}, PERMISSION_REQUEST_CODE);
        }

    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String permissions[], int[] grantResults) {
        switch (requestCode) {
            case PERMISSION_REQUEST_CODE:
                if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                    Logger.e(TAG, "Permission Granted, Now you can use local drive .");
                } else {
                    Logger.e(TAG, "Permission Denied, You cannot use local drive .");
                }
                break;
        }
    }

    public void checkCredential() {

        AlertDialog.Builder alertdialog = new AlertDialog.Builder(LoginActivity.this);
        alertdialog.setTitle(R.string.dialogAwsTitle);
        alertdialog.setCancelable(false);
        alertdialog.setMessage(R.string.dialogAwsMessage)
                .setPositiveButton(Constants.OK, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int i) {
                        BackgroundTask task = new BackgroundTask(LoginActivity.this);
                        task.execute();
                    }
                })
                .setNegativeButton(Constants.CANCEL, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int i) {
                        finish();
                    }
                });
        alertdialog.show();
    }

    //CSV Credentials dialog
    public void dialog() {

        final AlertDialog.Builder alt_bld = new AlertDialog.Builder(this);
        alt_bld.setTitle(R.string.credentials);
        alt_bld.setCancelable(false);
        if (csvFiles.length == 0) {
            alt_bld.setMessage(Constants.ITEM);
        } else {
            alt_bld.setSingleChoiceItems(csvFiles, -1, new DialogInterface
                    .OnClickListener() {
                public void onClick(DialogInterface dialog, int item) {

                    regionsDialog();
                    dialog.dismiss();
                }
            });
        }
        alt_bld.setNegativeButton(Constants.CANCEL, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int i) {
                finish();
            }
        });
        AlertDialog alert = alt_bld.create();
        alert.show();

    }


    private void regionsDialog() {

        AlertDialog.Builder alt_bld = new AlertDialog.Builder(this);
        alt_bld.setCancelable(false);
        regionEndpoints = new CharSequence[]{Regions.GovCloud.getName(), Regions.US_EAST_1.getName(), Regions.US_WEST_1.getName(), Regions.US_WEST_2.getName(), Regions.EU_WEST_1.getName(), Regions.EU_CENTRAL_1.getName(), Regions.AP_SOUTH_1.getName(), Regions.AP_SOUTHEAST_1.getName(), Regions.AP_SOUTHEAST_2.getName(), Regions.AP_NORTHEAST_1.getName(), Regions.AP_NORTHEAST_2.getName(), Regions.SA_EAST_1.getName(), Regions.CN_NORTH_1.getName()};
        alt_bld.setTitle(R.string.regionEndpoint);
        alt_bld.setCancelable(false);
        alt_bld.setSingleChoiceItems(regionEndpoints, -1, new DialogInterface
                .OnClickListener() {

            public void onClick(DialogInterface dialog, int item) {

                Intent i = new Intent(LoginActivity.this, ListThings.class);
                i.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TASK);
                regionKey = (String) regionEndpoints[item];
                SharedPreferences pref = getSharedPreferences(Constants.KEY, 0); // 0 - for private mode
                SharedPreferences.Editor editor = pref.edit();
                isAccessed = true;
                editor.putString(Constants.REGION, regionKey);
                editor.putBoolean(Constants.ACCESSED, isAccessed);
                editor.apply();

                startActivity(i);

            }
        });
        alt_bld.setNegativeButton(Constants.CANCEL, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int i) {
                finish();
            }
        });
        AlertDialog alert = alt_bld.create();
        alert.show();

    }


    public ArrayList<File> getfile(File dir) {
        File listFile[] = dir.listFiles();
        File sdPathFile = Environment.getExternalStorageDirectory();
        if (listFile != null && listFile.length > 0) {
            for (int i = 0; i < listFile.length; i++) {
                if (listFile[i].getName().endsWith(Constants.CSV)) {
                    if (listFile[i].getName().equals(Constants.CREDENTIALS)) {
                        fileList.add(listFile[i]);
                        File yourFile = new File(dir, listFile[i].getName());
                        csvFiles = new CharSequence[]{yourFile.toString()};
                        try {
                            readFileData(yourFile.toString());

                        } catch (FileNotFoundException e) {
                            e.printStackTrace();
                        }
                    }

                }
            }
        }
        return fileList;
    }


    //Read Csv File and parse
    void readFileData(String path) throws FileNotFoundException {
        String[] data;
        File file = new File(path);
        if (file.exists()) {
            BufferedReader br = new BufferedReader(new FileReader(file));
            try {
                String csvLine;
                while ((csvLine = br.readLine()) != null) {
                    data = csvLine.split(",");
                    try {
                        putKeyInSharedPreference(data[2], data[3]);
                    } catch (Exception e) {
                        Logger.d(TAG, e.toString());
                    }
                }
            } catch (IOException ex) {
                throw new RuntimeException("Error in reading CSV file: " + ex);
            }
        } else {

        }
    }

    private void putKeyInSharedPreference(String accessKey, String secretKey) {


        SharedPreferences pref = getSharedPreferences(Constants.KEY, 0); // 0 - for private mode
        SharedPreferences.Editor editor = pref.edit();
        editor.putString(Constants.ACCESS_KEY, accessKey);
        editor.putString(Constants.SECRET_KEY, secretKey);
        editor.putString(Constants.CUSTOMER_ENDPOINT, "");
        editor.apply();

    }


    //Find the root of SD CARD and check for CSV
    private class BackgroundTask extends AsyncTask<Void, Void, Void> {
        private ProgressDialog dialog;

        public BackgroundTask(LoginActivity activity) {
            dialog = new ProgressDialog(activity);
        }

        @Override
        protected void onPreExecute() {
            dialog.setMessage(Constants.CSV_FILES);
            dialog.setCancelable(false);
            dialog.show();
        }

        @Override
        protected void onPostExecute(Void result) {
            if (dialog.isShowing()) {
                dialog.dismiss();
                dialog();

            }
        }

        @Override
        protected Void doInBackground(Void... params) {
            try {
                root = new File(Environment.getExternalStorageDirectory()
                        .getAbsolutePath());
                getfile(root);
                Thread.sleep(Constants.THREAD_TIME);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }

            return null;
        }

    }

}
