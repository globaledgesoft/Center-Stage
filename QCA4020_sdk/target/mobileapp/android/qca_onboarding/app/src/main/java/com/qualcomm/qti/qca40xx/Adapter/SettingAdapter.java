package com.qualcomm.qti.qca40xx.Adapter;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import com.qualcomm.qti.qca40xx.Activity.ListOfCoordinatorActivity;
import com.qualcomm.qti.qca40xx.Interface.ItemClickListener;
import com.qualcomm.qti.qca40xx.R;

import java.util.ArrayList;

// This adapter class is for Settings screen

public class SettingAdapter extends RecyclerView.Adapter<SettingAdapter.ViewHolder> {
    ArrayList<String> SubjectValues;
    Context context;
    View view1;
    ViewHolder viewHolder1;
    TextView textView;
    Activity activity;

    public SettingAdapter(Context context1, ArrayList<String> SubjectValues1, Activity activity1){

        SubjectValues = SubjectValues1;
        context = context1;
        activity = activity1;
    }

    public static class ViewHolder extends RecyclerView.ViewHolder implements View.OnClickListener{

        public TextView textView;
        private ItemClickListener clickListener;
        public ViewHolder(View v){

            super(v);

            textView = (TextView)v.findViewById(R.id.setting_item_textview);
        }
        public void setClickListener(ItemClickListener itemClickListener) {
            this.clickListener = itemClickListener;
        }

        @Override
        public void onClick(View view) {
            if (clickListener != null)
                clickListener.onClick(view, getPosition(), true);
        }

    }

    @Override
    public SettingAdapter.ViewHolder onCreateViewHolder(ViewGroup parent, int viewType){

        view1 = LayoutInflater.from(context).inflate(R.layout.setting_item_layout,parent,false);

        viewHolder1 = new ViewHolder(view1);

        return viewHolder1;
    }

    @Override
    public void onBindViewHolder(ViewHolder holder, int position){

        holder.textView.setText(SubjectValues.get(position));
       // holder.setClickListener();
        holder.textView.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Intent coordinatorListIntent = new Intent(activity,ListOfCoordinatorActivity.class);
                activity.startActivity(coordinatorListIntent);
            }
        });
    }

    @Override
    public int getItemCount(){

        return SubjectValues.size();
    }
}

