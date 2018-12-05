package com.qualcomm.qti.qca40xx.Model;

/**
 * Created by sushma on 23/2/18.
 */

public class ThermoUpdateModel {

    private String op_mode;
    private int desired_temp;
    private int threshold;

    public ThermoUpdateModel(String op_mode, int desired_temp, int threshold) {
        this.op_mode = op_mode;
        this.desired_temp = desired_temp;
        this.threshold = threshold;
    }

    public String getOp_mode() {
        return op_mode;
    }

    public void setOp_mode(String op_mode) {
        this.op_mode = op_mode;
    }

    public int getDesired_temp() {
        return desired_temp;
    }

    public void setDesired_temp(int desired_temp) {
        this.desired_temp = desired_temp;
    }

    public int getThreshold() {
        return threshold;
    }

    public void setThreshold(int threshold) {
        this.threshold = threshold;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;

        ThermoUpdateModel that = (ThermoUpdateModel) o;

        if (desired_temp != that.desired_temp) return false;
        if (threshold != that.threshold) return false;
        return op_mode != null ? op_mode.equals(that.op_mode) : that.op_mode == null;
    }

    @Override
    public int hashCode() {
        int result = op_mode != null ? op_mode.hashCode() : 0;
        result = 31 * result + desired_temp;
        result = 31 * result + threshold;
        return result;
    }
}
