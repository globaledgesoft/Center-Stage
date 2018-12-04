package com.qualcomm.qti.qca40xx.Adapter;

import android.app.Activity;
import android.content.DialogInterface;
import android.support.v7.app.AlertDialog;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import com.qualcomm.qti.qca40xx.Model.ListOfCoordinatorDataModel;
import com.qualcomm.qti.qca40xx.R;
import com.qualcomm.qti.qca40xx.Util.DBHelper;

import java.util.ArrayList;

// Adapter for ListOfCoordinatorActivity
public class ListOfCoordinatorAdapter extends RecyclerView.Adapter<ListOfCoordinatorAdapter.SettingViewHolder> {
    Activity context;
    ArrayList<ListOfCoordinatorDataModel> settingDataList = new ArrayList<>();
    DBHelper dbHelper;

    public ListOfCoordinatorAdapter(Activity context, ArrayList<ListOfCoordinatorDataModel> settingDataList) {
        this.context = context;
        this.settingDataList = settingDataList;
        dbHelper = new DBHelper(context);
    }

    @Override
    public SettingViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        // Attach layout for single cell
        View view = LayoutInflater.from(parent.getContext())
                .inflate(R.layout.delete_db_data_layout, parent, false);
        SettingViewHolder myViewHolder = new SettingViewHolder(view);
        return myViewHolder;
    }

    @Override
    public void onBindViewHolder(final SettingViewHolder holder, final int listPosition) {
        TextView textViewName = holder.textViewName;
        TextView textViewMac = holder.textViewMac;
        TextView textViewState = holder.textViewIsOffline;
        ImageView imageViewDelete = holder.imageViewDelete;

        String name = settingDataList.get(listPosition).getDeviceName();
        String mac = settingDataList.get(listPosition).getDeviceMac();
        String state = settingDataList.get(listPosition).getDeviceState();
        textViewMac.setText(mac);
        textViewState.setText(state);
        textViewName.setText(name);

        imageViewDelete.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                AskOption(listPosition);

            }
        });
    }

    @Override
    public int getItemCount() {
        return settingDataList.size();
    }

    private void AskOption(final int position) {
        DialogInterface.OnClickListener dialogClickListener = new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                switch (which) {
                    case DialogInterface.BUTTON_POSITIVE:
                        //Do your Yes progress
                        int val = dbHelper.deleteCoorinator(settingDataList.get(position).getDeviceName());
                        if (val > 0) {
                            //delete the row from the records ArrayList
                            settingDataList.remove(position);
                            //notifyItemRemoved(position);
                            notifyDataSetChanged();
                        } else {
                            Toast.makeText(context, R.string.toastNotSuccessfulDelete, Toast.LENGTH_LONG).show();
                        }
                        break;

                    case DialogInterface.BUTTON_NEGATIVE:
                        //Do your No progress
                        break;
                }
            }
        };
        AlertDialog.Builder ab = new AlertDialog.Builder(context);
        ab.setMessage("Are you sure to delete?").setPositiveButton("Yes", dialogClickListener)
                .setNegativeButton("No", dialogClickListener).show();
    }

    public static class SettingViewHolder extends RecyclerView.ViewHolder {

        TextView textViewName, textViewMac, textViewIsOffline;
        ImageView imageViewDelete;


        public SettingViewHolder(View itemView) {
            super(itemView);
            this.textViewName = (TextView) itemView.findViewById(R.id.settings_cardview_textview_ble_name);
            this.imageViewDelete = (ImageView) itemView.findViewById(R.id.settings_cardview_imageview_delete);
            this.textViewMac = (TextView) itemView.findViewById(R.id.settings_cardview_textview_ble_mac_addr);
            this.textViewIsOffline = (TextView) itemView.findViewById(R.id.settings_cardview_textview_device_configuration);
        }
    }
}
