package com.qualcomm.qti.qca40xx.Adapter;

import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentPagerAdapter;

import com.qualcomm.qti.qca40xx.Fragment.SensorDetailFragment;
import com.qualcomm.qti.qca40xx.Fragment.ThermostatFragment;

//Pager Adapter for tabs

public class PagerAdapter extends FragmentPagerAdapter {

    private int mNumOfTabs;

    public PagerAdapter(FragmentManager fm, int NumOfTabs) {
        super(fm);
        this.mNumOfTabs = NumOfTabs;
    }


    @Override
    public Fragment getItem(int position) {

        switch (position) {
            case 0:
                    return new SensorDetailFragment();
            case 1:
                    return new ThermostatFragment();
            default:
                return null;
        }
    }

    @Override
    public int getCount() {
        return mNumOfTabs;
    }
}
