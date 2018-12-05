/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package com.qualcomm.qti.qca40xx.sensor.Activity;

import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.support.v7.app.AlertDialog;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.CardView;
import android.support.v7.widget.Toolbar;
import android.view.View;
import android.widget.Toast;

import com.amazonaws.regions.Regions;
import com.qualcomm.qti.qca40xx.sensor.Utils.Constants;
import com.qualcomm.qti.qca40xx.sensor.Utils.Logger;
import com.qualcomm.qti.qca40xx.sensor.R;


public class UpdateRegion extends AppCompatActivity {
    private static final String TAG = "UpdateRegion";

    private Toolbar toolbar;
    private CharSequence[] regionEndpoints = {};
    private CardView cardView, credentialsCardView, aboutCardView;
    private String regionKey;
    private Context mContext;
    private AlertDialog alert = null;
    private String version;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_update_region);
        initialization();

        try {
            PackageInfo pInfo = this.getPackageManager().getPackageInfo(getPackageName(), 0);
            version = pInfo.versionName;
            int verCode = pInfo.versionCode;
        } catch (PackageManager.NameNotFoundException e) {
            e.printStackTrace();
        }

        cardView.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                regionsDialog();
            }
        });

        credentialsCardView.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                clearCredential();
            }
        });

        aboutCardView.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                aboutApp();
            }
        });


    }

    private void initialization() {
        toolbar = (Toolbar) findViewById(R.id.bar);
        toolbar.setTitle(R.string.QualcommSensors);
        toolbar.setNavigationIcon(R.drawable.backarrow);
        setSupportActionBar(toolbar);
        toolbar.setNavigationOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                onBackPressed();
            }
        });
        cardView = (CardView) findViewById(R.id.card_view_update);
        credentialsCardView = (CardView) findViewById(R.id.card_view_credentials);
        aboutCardView = (CardView) findViewById(R.id.card_view_about);
        mContext = this;
    }

    private void aboutApp() {
        AlertDialog.Builder alertdialog = new AlertDialog.Builder(UpdateRegion.this);
        alertdialog.setTitle(R.string.QualcommSensors);
        alertdialog.setCancelable(false);
        alertdialog.setMessage("Version Number : " + version);
        alertdialog.setNegativeButton(Constants.OK, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int i) {
                dialog.dismiss();
            }
        });
        alertdialog.show();
    }


    private void clearCredential() {

        AlertDialog.Builder alertdialog = new AlertDialog.Builder(UpdateRegion.this);
        alertdialog.setTitle(R.string.clearCredentials);
        alertdialog.setCancelable(false);
        alertdialog.setMessage(R.string.clearAlertMessage)
                .setPositiveButton(Constants.OK, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int i) {
                        SharedPreferences preferences = getSharedPreferences(Constants.KEY, 0);
                        preferences.edit().clear().commit();
                        Toast.makeText(mContext, Constants.CREDENTIALS_CLEARED, Toast.LENGTH_LONG).show();
                        Intent intent = new Intent(UpdateRegion.this, LoginActivity.class);
                        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TASK);
                        startActivity(intent);
                    }
                })
                .setNegativeButton(Constants.CANCEL, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int i) {
                        dialog.dismiss();
                    }
                });
        alertdialog.show();
    }

    private void regionsDialog() {

        AlertDialog.Builder alt_bld = new AlertDialog.Builder(this);
        alt_bld.setCancelable(false);
        Logger.d(TAG, "regionsFound " + Regions.GovCloud.getName());
        regionEndpoints = new CharSequence[]{Regions.GovCloud.getName(), Regions.US_EAST_1.getName(), Regions.US_WEST_1.getName(), Regions.US_WEST_2.getName(), Regions.EU_WEST_1.getName(), Regions.EU_CENTRAL_1.getName(), Regions.AP_SOUTH_1.getName(), Regions.AP_SOUTHEAST_1.getName(), Regions.AP_SOUTHEAST_2.getName(), Regions.AP_NORTHEAST_1.getName(), Regions.AP_NORTHEAST_2.getName(), Regions.SA_EAST_1.getName(), Regions.CN_NORTH_1.getName()};
        alt_bld.setTitle(R.string.updateRegion);
        alt_bld.setCancelable(false);
        alt_bld.setSingleChoiceItems(regionEndpoints, -1, new DialogInterface
                .OnClickListener() {

            public void onClick(DialogInterface dialog, int item) {

                regionKey = (String) regionEndpoints[item];
                SharedPreferences pref = getSharedPreferences(Constants.KEY, 0); // 0 - for private mode
                SharedPreferences.Editor editor = pref.edit();
                editor.putString(Constants.REGION, regionKey);
                editor.apply();
                if (alert != null && alert.isShowing()) {
                    alert.dismiss();
                }
                Intent i = new Intent(getApplication(), ListThings.class);
                i.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TASK);
                startActivity(i);

            }
        });
        alt_bld.setNegativeButton(Constants.CANCEL, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int i) {
                dialog.dismiss();
            }
        });
        alert = alt_bld.create();
        alert.show();

    }
}
