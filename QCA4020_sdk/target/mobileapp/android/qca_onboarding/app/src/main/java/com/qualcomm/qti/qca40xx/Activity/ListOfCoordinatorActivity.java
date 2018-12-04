package com.qualcomm.qti.qca40xx.Activity;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.support.v7.widget.Toolbar;
import android.view.MenuItem;
import android.view.View;
import android.widget.TextView;

import com.qualcomm.qti.qca40xx.Adapter.ListOfCoordinatorAdapter;
import com.qualcomm.qti.qca40xx.Model.ListOfCoordinatorDataModel;
import com.qualcomm.qti.qca40xx.R;
import com.qualcomm.qti.qca40xx.Util.Constants;
import com.qualcomm.qti.qca40xx.Util.DBHelper;

import java.util.ArrayList;


// ListOfCoordinatorActivity display the list of coordinator in Database
public class ListOfCoordinatorActivity extends AppCompatActivity {
    DBHelper dbHelper;
    ArrayList<ListOfCoordinatorDataModel> dataModels;
    ListOfCoordinatorAdapter adapter;
    RecyclerView recyclerView;
    TextView noData;
    Toolbar toolbar;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_list_of_coordinator);
        toolbar = (Toolbar) findViewById(R.id.tool_bar);
        toolbar.setTitle(Constants.TITLE_LIST_COORDINATOR);
        setSupportActionBar(toolbar);
        getSupportActionBar().setHomeButtonEnabled(true);
        getSupportActionBar().setDisplayHomeAsUpEnabled(true);

        dbHelper = new DBHelper(this);
        recyclerView = (RecyclerView) findViewById(R.id.recycler_view_db_data);
        noData = (TextView) findViewById(R.id.setting_no_data);

        recyclerView.setHasFixedSize(true);
        RecyclerView.LayoutManager layoutManager = new LinearLayoutManager(ListOfCoordinatorActivity.this);
        recyclerView.setLayoutManager(layoutManager);
        recyclerView.setLayoutManager(layoutManager);

        dataModels = dbHelper.getCoordinatorList();
        if (dataModels.size() > 0) {
            adapter = new ListOfCoordinatorAdapter(ListOfCoordinatorActivity.this, dataModels);
            recyclerView.setAdapter(adapter);
            recyclerView.invalidate();
        } else {
            recyclerView.setVisibility(View.GONE);
            noData.setVisibility(View.VISIBLE);
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if (item.getItemId() == android.R.id.home) {
            onBackPressed();
        }
        return super.onOptionsItemSelected(item);
    }
}
