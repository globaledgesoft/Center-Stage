/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
*/
// Copyright (c) 2018 Qualcomm Technologies, Inc.
// All rights reserved.
// Redistribution and use in source and binary forms, with or without modification, are permitted (subject to the limitations in the disclaimer below) 
// provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, 
// this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// Neither the name of Qualcomm Technologies, Inc. nor the names of its contributors may be used to endorse or promote products derived 
// from this software without specific prior written permission.
// NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE. 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, 
// BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, 
// OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, 
// EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <stdio.h>

#include "qurt_signal.h"

#include "qapi/qurt_thread.h"
#include "stdint.h"
#include <qcli.h>
#include <qcli_api.h>
#include <qurt_timer.h>

#include "qapi/qapi_status.h"

#include <qapi_i2c_master.h>
#include "sensors_demo.h"
#include "sensor_json.h"

#include  "sensors.h"
#include  "log_util.h"

#define I2C_wait(msec)    do { \
                              qurt_time_t qtime;\
                              qtime = qurt_timer_convert_time_to_ticks(msec, QURT_TIME_MSEC);\
                              qurt_thread_sleep(qtime);\
                          } while (0)

#define  I2C_ACT_READ       1
#define  I2C_ACT_WRITE      0

#define SENSOR_INFO         LOG_INFO
#define SENSOR_ERROR        LOG_ERROR
#define SENSOR_WARN         LOG_WARN
#define SENSOR_VERBOSE      LOG_VERBOSE

//undef LOG_VERBOSE
//#define LOG_VERBOSE(...)

void sensors_light_LTR303ALS_get_measured_values(sensor_info_t *sensor_data);
int32_t sensors_humidity_get_measured_value(sensor_info_t *sensor_data);
void sensors_pressure_get_measured_values(sensor_info_t *sensor_data);
void sensors_compass_get_measured_values(sensor_info_t *sensor_data);
void sensors_gyroscope_get_measured_values(sensor_info_t *sensor_data);
int add_sensor_entry(char *json_buf, sensor_info_t *sens_info);

void *h1; /**< I2C Handle */

sensors_val_t   sensor_vals;

extern  QCLI_Group_Handle_t qcli_onboard;              /* Handle for our QCLI Command Group. */

#define  I2CM_READY_SIG_MASK         0x10

qurt_signal_t   i2c_ready_signal;

qapi_I2CM_Config_t config_humidity = {
    100,            /**< I2C bus speed in kHz. */
    I2C_ADDR_HUMIDITY,                /**< 7-bit I2C slave address. */
    0,                   /**< SMBUS mode transfers. Set to TRUE for SMBUS mode. */
    0,   /**< Maximum slave clock stretch in us that a slave might perform. */
    0,          /**< Core Specific Configuration. Recommended 0. */
    0,          /**< Core Specific Configuration. Recommended 0. */
};

qapi_I2CM_Config_t config_pressure = {
    100,            /**< I2C bus speed in kHz. */
    I2C_ADDR_PRESSURE,                /**< 7-bit I2C slave address. */
    0,                   /**< SMBUS mode transfers. Set to TRUE for SMBUS mode. */
    0,   /**< Maximum slave clock stretch in us that a slave might perform. */
    0,          /**< Core Specific Configuration. Recommended 0. */
    0,          /**< Core Specific Configuration. Recommended 0. */
};

qapi_I2CM_Config_t config_compass = {
    100,            /**< I2C bus speed in kHz. */
    I2C_ADDR_COMPASS_AK09911C,                /**< 7-bit I2C slave address. */
    0,                   /**< SMBUS mode transfers. Set to TRUE for SMBUS mode. */
    0,   /**< Maximum slave clock stretch in us that a slave might perform. */
    0,          /**< Core Specific Configuration. Recommended 0. */
    0,          /**< Core Specific Configuration. Recommended 0. */
};

qapi_I2CM_Config_t config_gyroscope_LSM6DS3 = {
    100,            /**< I2C bus speed in kHz. */
    I2C_ADDR_GYROSCOPE_LSM6DS3,                /**< 7-bit I2C slave address. */
    0,                   /**< SMBUS mode transfers. Set to TRUE for SMBUS mode. */
    0,   /**< Maximum slave clock stretch in us that a slave might perform. */
    0,          /**< Core Specific Configuration. Recommended 0. */
    0,          /**< Core Specific Configuration. Recommended 0. */
};

qapi_I2CM_Config_t config_light_LTR303ALS = {
    100,            /**< I2C bus speed in kHz. */
    I2C_ADDR_LIGHT_LTR303ALS,                /**< 7-bit I2C slave address. */
    0,                   /**< SMBUS mode transfers. Set to TRUE for SMBUS mode. */
    0,   /**< Maximum slave clock stretch in us that a slave might perform. */
    0,          /**< Core Specific Configuration. Recommended 0. */
    0,          /**< Core Specific Configuration. Recommended 0. */
};

/*=========================================================================*/

/*=========================================================================
  CALIBRATION DATA
  -----------------------------------------------------------------------*/
typedef struct
{
	uint16_t dig_T1;
	int16_t  dig_T2;
	int16_t  dig_T3;

	uint16_t dig_P1;
	int16_t  dig_P2;
	int16_t  dig_P3;
	int16_t  dig_P4;
	int16_t  dig_P5;
	int16_t  dig_P6;
	int16_t  dig_P7;
	int16_t  dig_P8;
	int16_t  dig_P9;

	uint8_t  dig_H1;
	int16_t  dig_H2;
	uint8_t  dig_H3;
	int16_t  dig_H4;
	int16_t  dig_H5;
	int8_t   dig_H6;
} bmp280_calib_data;
/*=========================================================================*/



void I2CM_Transfer_cb(const uint32_t status, void *CB_Parameter)
{
    qurt_signal_set(&i2c_ready_signal, I2CM_READY_SIG_MASK);
}

uint8_t read_sensor_reg8(qapi_I2CM_Config_t *pI2C_config, uint8 reg_addr_val)
{
    qapi_Status_t status;
    uint8 reg_addr, reg_val;
    qapi_I2CM_Descriptor_t desc;

    desc.buffer = &reg_addr;
    desc.length = 1;
    desc.transferred = 0;
    desc.flags = QAPI_I2C_FLAG_START | QAPI_I2C_FLAG_STOP | QAPI_I2C_FLAG_WRITE;
    reg_addr = reg_addr_val;
    status = qapi_I2CM_Transfer(h1, pI2C_config, &desc, 1, I2CM_Transfer_cb, NULL);

    if (status != QAPI_OK)
    {
        return 0;
    }

    qurt_signal_wait(&i2c_ready_signal, I2CM_READY_SIG_MASK, QURT_SIGNAL_ATTR_CLEAR_MASK);

    desc.buffer = &reg_val;
    desc.length = 1;
    desc.transferred = 0;
    desc.flags = QAPI_I2C_FLAG_START | QAPI_I2C_FLAG_STOP | QAPI_I2C_FLAG_READ;
    status = qapi_I2CM_Transfer(h1, pI2C_config, &desc, 1, I2CM_Transfer_cb, NULL);
    qurt_signal_wait(&i2c_ready_signal, I2CM_READY_SIG_MASK, QURT_SIGNAL_ATTR_CLEAR_MASK);

    return reg_val;
}

uint16_t read_sensor_reg16(qapi_I2CM_Config_t *pI2C_config, uint8 reg_addr_val)
{
    qapi_Status_t status;
    uint8 reg_addr;
    uint16_t   reg_val;
    qapi_I2CM_Descriptor_t desc[2];

    desc[0].buffer = &reg_addr;
    desc[0].length = 1;
    desc[0].transferred = 0;
    desc[0].flags = QAPI_I2C_FLAG_START | QAPI_I2C_FLAG_WRITE;
    reg_addr = reg_addr_val;

    desc[1].buffer = (uint8_t *)&reg_val;
    desc[1].length = 2;
    desc[1].transferred = 0;
    desc[1].flags = QAPI_I2C_FLAG_START | QAPI_I2C_FLAG_STOP | QAPI_I2C_FLAG_READ;
    status = qapi_I2CM_Transfer(h1, pI2C_config, desc, 2, I2CM_Transfer_cb, NULL);
    if (status != QAPI_OK)
    {
        return 0;
    }

    qurt_signal_wait(&i2c_ready_signal, I2CM_READY_SIG_MASK, QURT_SIGNAL_ATTR_CLEAR_MASK);

    return reg_val;
}

uint8_t read_sensor_reg_multi8(qapi_I2CM_Config_t *pI2C_config, uint8 reg_addr_val, uint8 *buff, uint32_t len)
{
    qapi_Status_t status;
    uint8 reg_addr;
    qapi_I2CM_Descriptor_t desc[2];

    desc[0].buffer = &reg_addr;
    desc[0].length = 2;
    desc[0].transferred = 0;
    desc[0].flags = QAPI_I2C_FLAG_START | /*QAPI_I2C_FLAG_STOP |*/ QAPI_I2C_FLAG_WRITE;
    reg_addr = reg_addr_val;

    desc[1].buffer = buff;
    desc[1].length = len+1;
    desc[1].transferred = 0;
    desc[1].flags = QAPI_I2C_FLAG_START | QAPI_I2C_FLAG_STOP | QAPI_I2C_FLAG_READ;
    status = qapi_I2CM_Transfer(h1, pI2C_config, desc, 2, I2CM_Transfer_cb, NULL);
    if (status != QAPI_OK)
    {
        return 0;
    }

    qurt_signal_wait(&i2c_ready_signal, I2CM_READY_SIG_MASK, QURT_SIGNAL_ATTR_CLEAR_MASK);

    return 0;
}

uint8_t write_sensor_reg8(qapi_I2CM_Config_t *pI2C_config, uint8 reg_addr_val, uint8_t byte_val)
{
    qapi_Status_t status;
    qapi_I2CM_Descriptor_t desc;
    uint8 wr_buf[2];

    wr_buf[0] = reg_addr_val;
    wr_buf[1] = byte_val;

    desc.buffer = wr_buf;
    desc.length = 2;
    desc.transferred = 0;
    desc.flags = QAPI_I2C_FLAG_START | QAPI_I2C_FLAG_STOP | QAPI_I2C_FLAG_WRITE;
    status = qapi_I2CM_Transfer(h1, pI2C_config, &desc, 1, I2CM_Transfer_cb, NULL);
    if (status != QAPI_OK)
    {
        return 0;
    }

    qurt_signal_wait(&i2c_ready_signal, I2CM_READY_SIG_MASK, QURT_SIGNAL_ATTR_CLEAR_MASK);

    return 0;
}

uint32_t pressue_temp_read(qapi_I2CM_Config_t *pI2C_config, uint8 reg_addr_val)
{
    uint8_t  bytes[4];
    uint32_t  val32;

    read_sensor_reg_multi8(pI2C_config, reg_addr_val, bytes, 3);
    val32 = (((uint32_t)bytes[0]) << 16) | (((uint32_t)bytes[1]) << 8) | bytes[2];
    val32 >>= 4;

    return val32;
}

/*
 *  Humidity & Temperature
 */

uint16_t humidity_read_sensor_reg16(qapi_I2CM_Config_t *pI2C_config, uint8 reg_addr_val)
{
    qapi_Status_t status;
    uint8 reg_addr;
    uint16_t  reg_val;
    qapi_I2CM_Descriptor_t desc[2];

    reg_addr = reg_addr_val | 0x80;
    desc[0].buffer = &reg_addr;
    desc[0].length = 1;
    desc[0].transferred = 0;
    desc[0].flags = QAPI_I2C_FLAG_START | QAPI_I2C_FLAG_WRITE;

    desc[1].buffer = (uint8_t *)&reg_val;
    desc[1].length = 2;
    desc[1].transferred = 0;
    desc[1].flags = QAPI_I2C_FLAG_START | QAPI_I2C_FLAG_STOP | QAPI_I2C_FLAG_READ;
    status = qapi_I2CM_Transfer(h1, pI2C_config, desc, 2, I2CM_Transfer_cb, NULL);
    if (status != QAPI_OK)
    {
        return 0;
    }

    qurt_signal_wait(&i2c_ready_signal, I2CM_READY_SIG_MASK, QURT_SIGNAL_ATTR_CLEAR_MASK);

    return reg_val;
}

void read_humidity_reg(uint32_t reg_addr)
{
    uint32_t reg_val;

    if (reg_addr != 0x0F && reg_addr != 0x10 && reg_addr != 0x20 && reg_addr != 0x21 && reg_addr != 0x22 && reg_addr != 0x27)
    {
        SENSOR_VERBOSE( "Humidity wrong Register Addr:%x\n", reg_addr);
        return;
    }
    reg_val = read_sensor_reg8(&config_humidity, reg_addr);

    SENSOR_VERBOSE( "Humidity Sensor Reg:0x%X  Val:0x%X\n", reg_addr, reg_val);
    return;
}

void write_humidity_reg(uint32_t reg_addr, uint8_t reg_val)
{
    if (reg_addr != 0x10 && reg_addr != 0x20 && reg_addr != 0x21 && reg_addr != 0x22)
    {
        SENSOR_VERBOSE( "Humidity wrong Register Addr:%x\n", reg_addr);
        return;
    }
    write_sensor_reg8(&config_humidity, reg_addr, reg_val);

    SENSOR_VERBOSE( "Humidity Sensor Reg:0x%X  Val:0x%X\n", reg_addr, reg_val);
    return;
}

int32_t sensors_humidity_get_measured_value(sensor_info_t *sensor_data)
{
    uint16_t   T0_DegCx8, T1_DegCx8, T_current;
    uint8_t    T0_T1_MSB;
    int16_t    T0_OUT_val, T1_OUT_val, T_OUT_val;
    uint32_t   T0_DegCx8_f, T1_DegCx8_f, T_DegCx8_fx10;
    uint16_t   H0_rHx2, H1_rHx2, H_current;
    uint32_t   H_rHx2_fx10;
    int16_t    H0_T0_OUT_val, H1_T0_OUT_val, H_OUT_val;

    // read temperature
    SENSOR_VERBOSE( "  ------  Temperature ------\n");
    T_OUT_val = humidity_read_sensor_reg16(&config_humidity, HUMIDITY_I2C_REG_ADDR_T_OUT);

    T0_DegCx8 = read_sensor_reg8(&config_humidity, HUMIDITY_I2C_REG_ADDR_T0_DegCx8);
    T1_DegCx8 = read_sensor_reg8(&config_humidity, HUMIDITY_I2C_REG_ADDR_T1_DegCx8);
    T0_T1_MSB = read_sensor_reg8(&config_humidity, HUMIDITY_I2C_REG_ADDR_T0_T1_MSB);
    SENSOR_VERBOSE( "DegCx8 T0:0x%x  T1:0x%x  T0_T_MSB:%x\n", T0_DegCx8, T1_DegCx8, T0_T1_MSB);

    T0_DegCx8 |= ((uint16_t)(T0_T1_MSB & 3)) << 8;
    T1_DegCx8 |= ((uint16_t)((T0_T1_MSB >> 2) & 3)) << 8;
    SENSOR_VERBOSE( "DegCx8 with MSB T0:0x%x  T1:0x%x\n", T0_DegCx8, T1_DegCx8);

    T0_OUT_val = humidity_read_sensor_reg16(&config_humidity, HUMIDITY_I2C_REG_ADDR_T0_OUT);
    T1_OUT_val = humidity_read_sensor_reg16(&config_humidity, HUMIDITY_I2C_REG_ADDR_T1_OUT);
    SENSOR_VERBOSE( "register 2's comp T0:%d T1:%d T:%d\n", T0_OUT_val, T1_OUT_val, T_OUT_val);

    T0_DegCx8_f = T0_DegCx8;
    T1_DegCx8_f = T1_DegCx8;
    T_DegCx8_fx10 = (T1_DegCx8_f - T0_DegCx8_f)  * (T_OUT_val - T0_OUT_val) * 10 / (T1_OUT_val - T0_OUT_val) + T0_DegCx8_f * 10;

    SENSOR_VERBOSE( "DegCx8 T0:%d T1:%d Tx10:%d\n", T0_DegCx8_f, T1_DegCx8_f, T_DegCx8_fx10);
    T_current = T_DegCx8_fx10 / 8;
    SENSOR_VERBOSE( "Current Temperature:%d.%d\n", T_current / 10, T_current % 10);
    if (sensor_data->sensor_type == SENSOR_TEMPERATURE)
    {
        sensor_data->s.temp.mantissa = T_current / 10;
        sensor_data->s.temp.exponent = T_current % 10;
    }

    // read humidity
    SENSOR_VERBOSE( "\n------  Relative Humidity ------\n");
    H_OUT_val = humidity_read_sensor_reg16(&config_humidity, HUMIDITY_I2C_REG_ADDR_H_OUT);

    H0_rHx2 = read_sensor_reg8(&config_humidity, HUMIDITY_I2C_REG_ADDR_H0_rHx2);
    H1_rHx2 = read_sensor_reg8(&config_humidity, HUMIDITY_I2C_REG_ADDR_H1_rHx2);
    SENSOR_VERBOSE( "rHx2 H0:0x%x  H1:0x%x\n", H0_rHx2, H1_rHx2);

    H0_T0_OUT_val = humidity_read_sensor_reg16(&config_humidity, HUMIDITY_I2C_REG_ADDR_H0_T0_OUT);
    H1_T0_OUT_val = humidity_read_sensor_reg16(&config_humidity, HUMIDITY_I2C_REG_ADDR_H1_T0_OUT);
    SENSOR_VERBOSE( "2's comp H0_T0:%d H1_T0:%d H:%d\n", H0_T0_OUT_val, H1_T0_OUT_val, H_OUT_val);

    H_rHx2_fx10 = (H1_rHx2 - H0_rHx2)  * (H_OUT_val - H0_T0_OUT_val) * 10 / (H1_T0_OUT_val - H0_T0_OUT_val) + H0_rHx2 * 10;

    SENSOR_VERBOSE( "rHx2 Hx10:%d\n", H_rHx2_fx10);
    H_current = H_rHx2_fx10 / 2;
    SENSOR_VERBOSE( "Current rH:%d.%d%% rH\n", H_current / 10, H_current % 10);
    if (sensor_data->sensor_type == SENSOR_HUMIDITY)
    {
        sensor_data->s.hum.mantissa = H_current/10;
        sensor_data->s.hum.exponent = H_current%10;
    }

    return 0;
}

/*
 *  Pressure & Temperature
 */

uint32_t i2c_pressure_intial = 0;

uint32_t sensors_pressure_read_sensor_bits24(qapi_I2CM_Config_t *pI2C_config, uint8 reg_addr_val)
{
    qapi_Status_t status;
    uint8 reg_addr;
    uint32_t  reg_val;
    qapi_I2CM_Descriptor_t desc[2];

    reg_addr = reg_addr_val | 0x80;
    desc[0].buffer = &reg_addr;
    desc[0].length = 1;
    desc[0].transferred = 0;
    desc[0].flags = QAPI_I2C_FLAG_START | QAPI_I2C_FLAG_WRITE;

    desc[1].buffer = (uint8_t *)&reg_val;
    desc[1].length = 3;
    desc[1].transferred = 0;
    desc[1].flags = QAPI_I2C_FLAG_START | QAPI_I2C_FLAG_STOP | QAPI_I2C_FLAG_READ;
    status = qapi_I2CM_Transfer(h1, pI2C_config, desc, 2, I2CM_Transfer_cb, NULL);
    if (status != QAPI_OK)
    {
        return 0;
    }

    qurt_signal_wait(&i2c_ready_signal, I2CM_READY_SIG_MASK, QURT_SIGNAL_ATTR_CLEAR_MASK);

    return reg_val;
}

void sensors_pressure_set_power_mode(uint32_t mode)
{
    uint8_t  reg_val;

    reg_val = read_sensor_reg8(&config_pressure, PRESSURE_I2C_REG_ADDR_CTRL_MEAS);
    reg_val = (reg_val & ~0x03) | (mode & 0x03);
    write_sensor_reg8(&config_pressure, PRESSURE_I2C_REG_ADDR_CTRL_MEAS, reg_val);
}

void sensors_pressure_get_parameters()
{
    uint8_t pwr_mode, osrs_p, osrs_t;
    uint8_t filter, t_sb;
    uint8_t reg_val;

    reg_val = read_sensor_reg8(&config_pressure, PRESSURE_I2C_REG_ADDR_CTRL_MEAS);
    pwr_mode = reg_val & 0x03;
    osrs_p = (reg_val >> 2) & 0x07;
    osrs_t = (reg_val >> 5) & 0x07;
    SENSOR_VERBOSE( "power mode=%d osrs p=%d osrs t=%d\n", pwr_mode, osrs_p, osrs_t);

    reg_val = read_sensor_reg8(&config_pressure, PRESSURE_I2C_REG_ADDR_CONF);
    filter = (reg_val >> 2) & 0x07;
    t_sb =  (reg_val >> 5) & 0x07;
    SENSOR_VERBOSE( "filter=%d t_sb=%d\n", filter, t_sb);

    reg_val = read_sensor_reg8(&config_pressure, PRESSURE_I2C_REG_ADDR_STATUS);
    SENSOR_VERBOSE( "status=%x\n", reg_val);
}

void sensors_pressure_set_parameters(uint8_t osrs_p, uint8_t osrs_t, uint8_t filter, uint8_t t_sb)
{
    uint8_t reg_val;

    reg_val = read_sensor_reg8(&config_pressure, PRESSURE_I2C_REG_ADDR_CTRL_MEAS);
    if (osrs_p != 0xFF)
    {
        reg_val = (reg_val & ~(0x07 << 2)) | (osrs_p & 0x7) << 2;
    }
    if (osrs_t != 0xFF)
    {
        reg_val = (reg_val & ~(0x07 << 5)) | (osrs_t & 0x7) << 5;
    }
    write_sensor_reg8(&config_pressure, PRESSURE_I2C_REG_ADDR_CTRL_MEAS, reg_val);

    reg_val = read_sensor_reg8(&config_pressure, PRESSURE_I2C_REG_ADDR_CONF);
    if (filter != 0xFF)
    {
        reg_val = (reg_val & ~(0x07 << 2)) | (filter & 0x7) << 2;
    }
    if (t_sb != 0xFF)
    {
        reg_val = (reg_val & ~(0x07 << 5)) | (t_sb & 0x7) << 5;
    }
    write_sensor_reg8(&config_pressure, PRESSURE_I2C_REG_ADDR_CONF, reg_val);
}


int32_t  t_fine;
int32_t   bmp820_compensate_T(uint32_t ut, uint16_t dig_T1, int16_t dig_T2, int16_t dig_T3)
{
    int64_t  var1, var2, T;


    var1 = ((((ut >> 3) - ((int32_t)dig_T1 << 1))) * ((int32_t)dig_T2)) >> 11;
    var2 = (((((ut>>4) - ((int32_t)dig_T1)) * ((ut>>4) - ((int32_t)dig_T1))) >> 12) * ((int32_t)dig_T3)) >> 14;
    t_fine = var1 + var2;
    T = (t_fine + 5 + 128) >> 8;
    return T;
}

bmp280_calib_data pressure_sensor;

void readCoefficients_bmp820(void)
{
	pressure_sensor.dig_T1 = read_sensor_reg16(&config_pressure,PRESSURE_I2C_REG_ADDR_dig_T1);
	pressure_sensor.dig_T2 = read_sensor_reg16(&config_pressure,PRESSURE_I2C_REG_ADDR_dig_T2);
	pressure_sensor.dig_T3 = read_sensor_reg16(&config_pressure,PRESSURE_I2C_REG_ADDR_dig_T3);

	pressure_sensor.dig_P1 = read_sensor_reg16(&config_pressure,PRESSURE_I2C_REG_ADDR_dig_P1);	
	pressure_sensor.dig_P2 = read_sensor_reg16(&config_pressure,PRESSURE_I2C_REG_ADDR_dig_P2);	
	pressure_sensor.dig_P3 = read_sensor_reg16(&config_pressure,PRESSURE_I2C_REG_ADDR_dig_P3);
	pressure_sensor.dig_P4 = read_sensor_reg16(&config_pressure,PRESSURE_I2C_REG_ADDR_dig_P4);	
	pressure_sensor.dig_P5 = read_sensor_reg16(&config_pressure,PRESSURE_I2C_REG_ADDR_dig_P5);	
	pressure_sensor.dig_P6 = read_sensor_reg16(&config_pressure,PRESSURE_I2C_REG_ADDR_dig_P6);	
	pressure_sensor.dig_P7 = read_sensor_reg16(&config_pressure,PRESSURE_I2C_REG_ADDR_dig_P7);
	pressure_sensor.dig_P8 = read_sensor_reg16(&config_pressure,PRESSURE_I2C_REG_ADDR_dig_P8);	
	pressure_sensor.dig_P9 = read_sensor_reg16(&config_pressure,PRESSURE_I2C_REG_ADDR_dig_P9);

		
}

void sensors_pressure_get_measured_values(sensor_info_t *sensor_data)
{
	static int cocient_read = 1;
	int64_t var1, var2, p;
	uint32_t  ut;
	int32_t up;
	union {
		uint32_t   val32;
		uint8_t    val8[4];
	} reg_val32;
	uint16_t   dig_T1;
	int16_t   dig_T2, dig_T3;
	int32_t    T;

	if (cocient_read)
	{
		cocient_read = 0;
		readCoefficients_bmp820();
	}

	reg_val32.val32 = sensors_pressure_read_sensor_bits24(&config_pressure, PRESSURE_I2C_REG_ADDR_PRESS);

	 up = (((uint32_t)(reg_val32.val8[0])) << 12) | (((uint32_t)(reg_val32.val8[1])) << 4) | ((((uint32_t)(reg_val32.val8[2])) >> 4) & 0x0F);
	 SENSOR_VERBOSE( "-------------------------------------------Pressure=0x%08X\n", up);


	reg_val32.val32 = sensors_pressure_read_sensor_bits24(&config_pressure, PRESSURE_I2C_REG_ADDR_TEMP);
	ut = (((uint32_t)(reg_val32.val8[0])) << 12) | (((uint32_t)(reg_val32.val8[1])) << 4) | ((((uint32_t)(reg_val32.val8[2])) >> 4) & 0x0F);
	SENSOR_VERBOSE( "Temp=0x%08X\n", ut);
	dig_T1 = read_sensor_reg16(&config_pressure, PRESSURE_I2C_REG_ADDR_dig_T1);
	dig_T2 = read_sensor_reg16(&config_pressure, PRESSURE_I2C_REG_ADDR_dig_T2);
	dig_T3 = read_sensor_reg16(&config_pressure, PRESSURE_I2C_REG_ADDR_dig_T3);
	T = bmp820_compensate_T(ut, dig_T1, dig_T2, dig_T3);
	SENSOR_VERBOSE( "Current Temp DegC=%d.%d\n", T/100, T%100);


	var1 = ((int64_t)t_fine) - 128000;
	var2 = var1 * var1 * (int64_t)pressure_sensor.dig_P6;
	var2 = var2 + ((var1*(int64_t)pressure_sensor.dig_P5)<<17);
	var2 = var2 + (((int64_t)pressure_sensor.dig_P4)<<35);
	var1 = ((var1 * var1 * (int64_t)pressure_sensor.dig_P3)>>8) +
		((var1 * (int64_t)pressure_sensor.dig_P2)<<12);
	var1 = (((((int64_t)1)<<47)+var1))*((int64_t)pressure_sensor.dig_P1)>>33;
	if (var1 == 0) {
		return ;  // avoid exception caused by division by zero
	}

	
	p = 1048576 - up;
	p = (((p<<31) - var2)*3125) / var1;
	var1 = (((int64_t)pressure_sensor.dig_P9) * (p>>13) * (p>>13)) >> 25;
	var2 = (((int64_t)pressure_sensor.dig_P8) * p) >> 19;


	p = ((p + var1 + var2) >> 8) + (((int64_t)pressure_sensor.dig_P7)<<4);

	{
		sensor_data->s.pressure.val = (((float)p/256)/100);
	}

	SENSOR_VERBOSE("Pressure_sensor_calculated: %f\n", sensor_data->s.pressure.val);
}

/**
 * activate_onboard_sensors functions Initializes all the
 * onboard sensors and write the appropriate data to register to actuate the sensors.
 */
int32_t activate_onboard_sensors()
{
    // Activate Humidity Sensor
    write_sensor_reg8(&config_humidity, HUMIDITY_I2C_REG_ADDR_CTRL_1, 0x83);

    // Activate Pressure Sensor
    write_sensor_reg8(&config_pressure, PRESSURE_I2C_REG_ADDR_CTRL_MEAS, 0x6f);

    // Activate Compass Sensor
    write_sensor_reg8(&config_compass, COMPASS_I2C_REG_ADDR_CNTL2, 0x08);

    // Activate Gyro Sensor
    write_sensor_reg8(&config_gyroscope_LSM6DS3, LSM6DS3_I2C_REG_ADDR_CTRL1_XL , 0x50);
    write_sensor_reg8(&config_gyroscope_LSM6DS3, LSM6DS3_I2C_REG_ADDR_CTRL2_G, 0x50);

    // Activate Light Sensor
    write_sensor_reg8(&config_light_LTR303ALS, LTR303ALS_I2C_REG_ADDR_ALS_CONTR, 0x1D);

    return 0;
}

/**
 * deactivate_onboard_sensors functions Initializes all the
 * onboard sensors and write the appropriate data to register to actuate the sensors.
 */
int32_t deactivate_onboard_sensors()
{
    // Deactivate Humidity Sensor
    write_sensor_reg8(&config_humidity, HUMIDITY_I2C_REG_ADDR_CTRL_1, 0x00);

    // Deactivate Pressure Sensor
    write_sensor_reg8(&config_pressure, PRESSURE_I2C_REG_ADDR_CTRL_MEAS, 0x00);

    // Deactivate Compass Sensor
    write_sensor_reg8(&config_compass, COMPASS_I2C_REG_ADDR_CNTL2, 0x00);

    // Deactivate Gyro Sensor
    write_sensor_reg8(&config_gyroscope_LSM6DS3, LSM6DS3_I2C_REG_ADDR_CTRL1_XL , 0x00);
    write_sensor_reg8(&config_gyroscope_LSM6DS3, LSM6DS3_I2C_REG_ADDR_CTRL2_G, 0x00);

    // Deactivate Light Sensor
    write_sensor_reg8(&config_light_LTR303ALS, LTR303ALS_I2C_REG_ADDR_ALS_CONTR, 0x00);

    return 0;
}

/**
 * func: sensors_read_all() reads all the sensors values
 */
int32_t read_remote_sensors(char *json_buf, sensor_info_t *sensor_val)
{
    sensor_val->sensor_type = AMBIENT_LIGHT;
    sensor_val->s.light.val = 0;
    if(FAILURE == add_sensor_entry(json_buf, sensor_val))
    {
        SENSOR_ERROR("Sensor entry is failed\n");
        return FAILURE;
    }
    return SUCCESS;
}

extern int notify_thermo_breach;
//extern struct thermo_stat thermostat;
int32_t read_onboard_sensors(char *json_buf, sensor_info_t *sensor_val, uint32_t update_flag)
{
    static int lux = 0;
    static int flag = 0;
    SENSOR_INFO("Update_flag = %d\n", update_flag);
    //below functions reads all the onboard sensors
    //Temperature sensor reading
    sensor_val->sensor_type = SENSOR_TEMPERATURE;
    sensors_humidity_get_measured_value(sensor_val);
    if(FAILURE == add_sensor_entry(json_buf, sensor_val))
    {
        SENSOR_ERROR("Sensor entry is failed\n");
        return FAILURE;
    }

    //Humidity sensor reading
    sensor_val->sensor_type = SENSOR_HUMIDITY;
    sensors_humidity_get_measured_value(sensor_val);
    if(FAILURE == add_sensor_entry(json_buf, sensor_val))
    {
        SENSOR_ERROR("Sensor entry is failed\n");
        return FAILURE;
    }

    //Light_sensor reading
    sensor_val->sensor_type = SENSOR_LIGHT;
    sensors_light_LTR303ALS_get_measured_values(sensor_val);
    if(FAILURE == add_sensor_entry(json_buf, sensor_val))
    {
        SENSOR_ERROR("Sensor entry is failed\n");
        return FAILURE;
    }
    if ((update_flag >> 1) & 1)
    {
        if ((sensor_val->s.lux.val > (lux+200)))
        {
#if BOARD_SUPPORTS_WIFI
            notify_thermo_breach = 1;
#endif
#if OFFLINE
         notify_thermo_breach = 1;
#endif
            flag = 1;
            breach_thermo_stat_result(sensor_val);
        }
        lux = sensor_val->s.lux.val;

        if (!flag)
            Randomize_thermo_stat_result(sensor_val);
        else
        {
            flag++;
            if (flag == 5)
                flag = 0;
        }
#if BOARD_SUPPORTS_WIFI
        sensor_val->sensor_type = THERMO_STAT;
        if(FAILURE == add_sensor_entry(json_buf, sensor_val))
        {
            SENSOR_ERROR("Sensor entry is failed\n");
            return FAILURE;
        }
#elif OFFLINE
		sensor_val->sensor_type = THERMO_STAT;
        if(FAILURE == add_sensor_entry(json_buf, sensor_val))
        {
            SENSOR_ERROR("Sensor entry is failed\n");
            return FAILURE;
        }
#endif
    }

    //Pressure_sensor_reading
    sensor_val->sensor_type = SENSOR_PRESSURE;
    sensors_pressure_get_measured_values(sensor_val);
    if(FAILURE == add_sensor_entry(json_buf, sensor_val))
    {
        SENSOR_ERROR("Sensor entry is failed\n");
        return FAILURE;
    }

    //Compass sensor reading
    sensor_val->sensor_type = SENSOR_COMPASS;
    sensors_compass_get_measured_values(sensor_val);
    if(FAILURE == add_sensor_entry(json_buf, sensor_val))
    {
        SENSOR_ERROR("Sensor entry is failed\n");
        return FAILURE;
    }

    //Gyroscope sensor reading
    sensor_val->sensor_type = SENSOR_GYROSCOPE;
    sensors_gyroscope_get_measured_values(sensor_val);
    if(FAILURE == add_sensor_entry(json_buf, sensor_val))
    {
        SENSOR_ERROR("Sensor entry is failed\n");
        return FAILURE;
    }

    //ACCELROMETER sensor reading
    sensor_val->sensor_type = SENSOR_ACCELROMETER;
    sensors_gyroscope_get_measured_values(sensor_val);
    if(FAILURE == add_sensor_entry(json_buf, sensor_val))
    {
        SENSOR_ERROR("Sensor entry is failed\n");
        return FAILURE;
    }
    //AMBIENT LIGHT
    if (update_flag &1)
    {
        sensor_val->sensor_type = AMBIENT_LIGHT;
        if (FAILURE == add_sensor_entry(json_buf, sensor_val))
        {
            SENSOR_ERROR("Sensor entry is failed\n");
            return FAILURE;
        }
    }
    else
    {
#if ENABLE_DIMMER
        sensor_val->sensor_type = DIMMER_LIGHT;
        if(FAILURE == add_sensor_entry(json_buf, sensor_val))
        {
            SENSOR_ERROR("Sensor entry is failed\n");
            return FAILURE;
        }
#endif
    }
    return 0;
}

/*
 * 3-axis Compass
 */
void sensors_compass_set_operation_mode(uint32_t mode)
{
    uint8_t  reg_val;

    reg_val = read_sensor_reg8(&config_compass, COMPASS_I2C_REG_ADDR_CNTL2);
    reg_val = (reg_val & ~0x1F) | (mode & 0x1F);
    write_sensor_reg8(&config_compass, COMPASS_I2C_REG_ADDR_CNTL2, reg_val);
}

void sensors_compass_get_parameters()
{
    uint8_t  status1_val, control2_val;

    status1_val = read_sensor_reg8(&config_compass, COMPASS_I2C_REG_ADDR_ST1);
    control2_val = read_sensor_reg8(&config_compass, COMPASS_I2C_REG_ADDR_CNTL2);
    SENSOR_VERBOSE( "ST1:0x%02X   CNTL2:0x%02X\n", status1_val, control2_val);
}

void sensors_compass_get_measured_values(sensor_info_t *sensor_data)
{
    int16_t   hx, hy, hz;
    int32_t ret;
    ret = write_sensor_reg8(&config_compass, COMPASS_I2C_REG_ADDR_CNTL2, 0x02);

    SENSOR_VERBOSE("Compass return value: %d\n", ret);

    hx = read_sensor_reg16(&config_compass, COMPASS_I2C_REG_ADDR_HX);
    hy = read_sensor_reg8(&config_compass, COMPASS_I2C_REG_ADDR_HY);
    hz = read_sensor_reg8(&config_compass, COMPASS_I2C_REG_ADDR_HZ);

    SENSOR_VERBOSE( "HX:%d   HY:%d   HZ:%d\n", hx, hy, hz);

    sensor_data->s.compass.x = hx;
    sensor_data->s.compass.y = hy;
    sensor_data->s.compass.z = hz;

}

/*
 * Gyroscope
 */
void sensors_gyroscope_set_operation_mode(uint32_t accelerometer_mode, uint32_t gyroscope_mode)
{
    uint8_t  reg_val;

    reg_val = read_sensor_reg8(&config_gyroscope_LSM6DS3, LSM6DS3_I2C_REG_ADDR_CTRL1_XL);
    reg_val = (reg_val & 0x0F) | ((accelerometer_mode & 0x0F) << 4);
    write_sensor_reg8(&config_gyroscope_LSM6DS3, LSM6DS3_I2C_REG_ADDR_CTRL1_XL, reg_val);

    reg_val = read_sensor_reg8(&config_gyroscope_LSM6DS3, LSM6DS3_I2C_REG_ADDR_CTRL2_G);
    reg_val = (reg_val & 0x0F) | ((gyroscope_mode & 0x0F) << 4);
    write_sensor_reg8(&config_gyroscope_LSM6DS3, LSM6DS3_I2C_REG_ADDR_CTRL2_G, reg_val);
}

void sensors_gyroscope_get_parameters()
{
    uint8_t  CTRL1_XL_val, CTRL2_G_val, status;
    uint8_t  FIFO_CTRL1_val, FIFO_CTRL2_val, FIFO_CTRL3_val, FIFO_CTRL4_val, FIFO_CTRL5_val;

    CTRL1_XL_val = read_sensor_reg8(&config_gyroscope_LSM6DS3, LSM6DS3_I2C_REG_ADDR_CTRL1_XL);
    CTRL2_G_val = read_sensor_reg8(&config_gyroscope_LSM6DS3, LSM6DS3_I2C_REG_ADDR_CTRL2_G);
    SENSOR_VERBOSE( "CTRL1_XL:0x%02X  CTRL2_G:0x%02X\n", CTRL1_XL_val, CTRL2_G_val);

    status = read_sensor_reg8(&config_gyroscope_LSM6DS3, LSM6DS3_I2C_REG_ADDR_STATUS);
    SENSOR_VERBOSE( "STATUS:0x%02X\n", status);

    FIFO_CTRL1_val = read_sensor_reg8(&config_gyroscope_LSM6DS3, LSM6DS3_I2C_REG_ADDR_FIFO_CTRL1);
    FIFO_CTRL2_val = read_sensor_reg8(&config_gyroscope_LSM6DS3, LSM6DS3_I2C_REG_ADDR_FIFO_CTRL2);
    FIFO_CTRL3_val = read_sensor_reg8(&config_gyroscope_LSM6DS3, LSM6DS3_I2C_REG_ADDR_FIFO_CTRL3);
    FIFO_CTRL4_val = read_sensor_reg8(&config_gyroscope_LSM6DS3, LSM6DS3_I2C_REG_ADDR_FIFO_CTRL4);
    FIFO_CTRL5_val = read_sensor_reg8(&config_gyroscope_LSM6DS3, LSM6DS3_I2C_REG_ADDR_FIFO_CTRL5);
    SENSOR_VERBOSE( "FIFO_CTRL1:0x%02X  FIFO_CTRL2:0x%02X  FIFO_CTRL3:0x%02X FIFO_CTRL4:0x%02X FIFO_CTRL5:0x%02X\n",
            FIFO_CTRL1_val, FIFO_CTRL2_val, FIFO_CTRL3_val, FIFO_CTRL4_val, FIFO_CTRL5_val);
}

void sensors_gyroscope_get_measured_values(sensor_info_t *sensor_data)
{
    int16_t   OUT_TEMP_val, OUTX_G_val, OUTY_G_val, OUTZ_G_val;
    int16_t   OUTX_XL_val, OUTY_XL_val, OUTZ_XL_val;
    OUT_TEMP_val = read_sensor_reg16(&config_gyroscope_LSM6DS3, LSM6DS3_I2C_REG_ADDR_OUT_TEMP);
    OUTX_G_val = read_sensor_reg16(&config_gyroscope_LSM6DS3, LSM6DS3_I2C_REG_ADDR_OUTX_G);
    OUTY_G_val = read_sensor_reg16(&config_gyroscope_LSM6DS3, LSM6DS3_I2C_REG_ADDR_OUTY_G);
    OUTZ_G_val = read_sensor_reg16(&config_gyroscope_LSM6DS3, LSM6DS3_I2C_REG_ADDR_OUTZ_G);
    SENSOR_VERBOSE( "TEMP:%d   X_G:%d   Y_G:%d   Z_G:%d\n", OUT_TEMP_val, OUTX_G_val, OUTY_G_val, OUTZ_G_val);
    if (sensor_data->sensor_type == SENSOR_GYROSCOPE)
    {
		sensor_data->s.gyro_val.x_g = ((OUTX_G_val*4.375)/1000);
        sensor_data->s.gyro_val.y_g = ((OUTY_G_val*4.375)/1000);
        sensor_data->s.gyro_val.z_g = ((OUTZ_G_val*4.375)/1000);

    }

	SENSOR_VERBOSE( "X_G:%f   Y_G:%f   Z_G:%f\n", sensor_data->s.gyro_val.x_g, sensor_data->s.gyro_val.y_g, sensor_data->s.gyro_val.z_g);
    OUTX_XL_val = read_sensor_reg16(&config_gyroscope_LSM6DS3, LSM6DS3_I2C_REG_ADDR_OUTX_XL);
    OUTY_XL_val = read_sensor_reg16(&config_gyroscope_LSM6DS3, LSM6DS3_I2C_REG_ADDR_OUTY_XL);
    OUTZ_XL_val = read_sensor_reg16(&config_gyroscope_LSM6DS3, LSM6DS3_I2C_REG_ADDR_OUTZ_XL);

    SENSOR_VERBOSE( "X_XL:%d   Y_XL:%d   Z_XL:%d\n", OUTX_XL_val, OUTY_XL_val, OUTZ_XL_val);
    if (sensor_data->sensor_type == SENSOR_ACCELROMETER)
    {
        sensor_data->s.acc_val.x_xl = (((OUTX_XL_val*0.061)/1000)*9.8);
        sensor_data->s.acc_val.y_xl = (((OUTY_XL_val*0.061)/1000)*9.8);
        sensor_data->s.acc_val.z_xl = (((OUTZ_XL_val*0.061)/1000)*9.8);
    }
	SENSOR_VERBOSE( "X_XL:%f   Y_XL:%f   Z_XL:%f\n", sensor_data->s.acc_val.x_xl, sensor_data->s.acc_val.y_xl, sensor_data->s.acc_val.z_xl);
}

/*
 * Ambinet Light Sensor
 */

void sensors_light_LTR303ALS_set_operation_mode(uint8_t mode)
{
    uint8_t  reg_val;

    reg_val = read_sensor_reg8(&config_light_LTR303ALS, LTR303ALS_I2C_REG_ADDR_ALS_CONTR);

    if (mode != 0)
        reg_val |= 1;
    else
        reg_val &= ~1;

    write_sensor_reg8(&config_light_LTR303ALS, LTR303ALS_I2C_REG_ADDR_ALS_CONTR, reg_val);
}

void sensors_light_LTR303ALS_get_parameters()
{
    uint8_t   ALS_CONTR_val, ALS_MEAS_RATE_val, ALS_STATUS_val, INTERRUPT_val;
    uint16_t  ALS_THRES_UP_val, ALS_THRES_LOW_val;

    ALS_CONTR_val = read_sensor_reg8(&config_light_LTR303ALS, LTR303ALS_I2C_REG_ADDR_ALS_CONTR);
    ALS_MEAS_RATE_val = read_sensor_reg8(&config_light_LTR303ALS, LTR303ALS_I2C_REG_ADDR_ALS_MEAS_RATE);
    ALS_STATUS_val = read_sensor_reg8(&config_light_LTR303ALS, LTR303ALS_I2C_REG_ADDR_ALS_STATUS);
    INTERRUPT_val = read_sensor_reg8(&config_light_LTR303ALS, LTR303ALS_I2C_REG_ADDR_INTERRUPT);
    SENSOR_VERBOSE( "LTR303ALS CNTR=0x%02x MEAS RATE=0x%02X STATUS=0x%02X INTERRUPT=0x%02X\n",
            ALS_CONTR_val, ALS_MEAS_RATE_val, ALS_STATUS_val, INTERRUPT_val);

    ALS_THRES_UP_val = read_sensor_reg16(&config_light_LTR303ALS, LTR303ALS_I2C_REG_ADDR_ALS_THRES_UP);
    ALS_THRES_LOW_val = read_sensor_reg16(&config_light_LTR303ALS, LTR303ALS_I2C_REG_ADDR_ALS_THRES_LOW);
    SENSOR_VERBOSE( "LTR303ALS THRES_UP=0x%04x THRES_LOW=0x%04X\n", ALS_THRES_UP_val, ALS_THRES_LOW_val);
}

void sensors_light_LTR303ALS_set_parameters(uint32_t als_gain, uint32_t als_integration_time, uint32_t als_measure_rate)
{
    uint8_t   ALS_CONTR_val, ALS_MEAS_RATE_val;

    ALS_CONTR_val = read_sensor_reg8(&config_light_LTR303ALS, LTR303ALS_I2C_REG_ADDR_ALS_CONTR);
    ALS_CONTR_val = (ALS_CONTR_val & ~0x1C) | ((als_gain & 0x07) << 2);
    write_sensor_reg8(&config_light_LTR303ALS, LTR303ALS_I2C_REG_ADDR_ALS_CONTR, ALS_CONTR_val);

    ALS_MEAS_RATE_val = read_sensor_reg8(&config_light_LTR303ALS, LTR303ALS_I2C_REG_ADDR_ALS_MEAS_RATE);
    ALS_MEAS_RATE_val = (ALS_MEAS_RATE_val & ~0x3F) | ((als_integration_time & 0x07) << 3) | (als_measure_rate & 0x07);
    write_sensor_reg8(&config_light_LTR303ALS, LTR303ALS_I2C_REG_ADDR_ALS_MEAS_RATE, ALS_MEAS_RATE_val);
}

uint32_t sensors_light_LTR303ALS_read_sensor_bits32(qapi_I2CM_Config_t *pI2C_config, uint8 reg_addr_val)
{
    qapi_Status_t status;
    uint8 reg_addr;
    uint32_t  reg_val;
    qapi_I2CM_Descriptor_t desc[2];

    reg_addr = reg_addr_val;
    desc[0].buffer = &reg_addr;
    desc[0].length = 1;
    desc[0].transferred = 0;
    desc[0].flags = QAPI_I2C_FLAG_START | QAPI_I2C_FLAG_WRITE;
    desc[1].buffer = (uint8_t *)&reg_val;
    desc[1].length = 4;
    desc[1].transferred = 0;
    desc[1].flags = QAPI_I2C_FLAG_START | QAPI_I2C_FLAG_STOP | QAPI_I2C_FLAG_READ;
    status = qapi_I2CM_Transfer(h1, pI2C_config, desc, 2, I2CM_Transfer_cb, NULL);
    if (status != QAPI_OK)
    {
        return 0;
    }

    qurt_signal_wait(&i2c_ready_signal, I2CM_READY_SIG_MASK, QURT_SIGNAL_ATTR_CLEAR_MASK);

    return reg_val;
}

void sensors_light_LTR303ALS_get_measured_values(sensor_info_t *sensor_data)
{
    union  {
        uint32_t   ch_val32;
        struct  {
            uint16_t	ch1;
            uint16_t    ch0;
        } ch_val16;
    } ch1_0_val;
    uint8_t       ALS_CONTR_val, gain;
    uint32_t       lux=0;

    ch1_0_val.ch_val32 = sensors_light_LTR303ALS_read_sensor_bits32(&config_light_LTR303ALS, LTR303ALS_I2C_REG_ADDR_ALS_DATA_CH1_0);

    SENSOR_VERBOSE( "CH 1:%d   CH 0:%d\n", ch1_0_val.ch_val16.ch1, ch1_0_val.ch_val16.ch0);
    ALS_CONTR_val = read_sensor_reg8(&config_light_LTR303ALS, LTR303ALS_I2C_REG_ADDR_ALS_CONTR);
    gain = (ALS_CONTR_val >> 2) & 0x07;

    switch (gain)
    {
        case 0:
            lux = ch1_0_val.ch_val16.ch0;
            break;
        case 1:
            lux = 5 * ch1_0_val.ch_val16.ch0 / 10;
            break;
        case 2:
            lux = 25 * ch1_0_val.ch_val16.ch0 / 100;
            break;
        case 3:
            lux = 125 * ch1_0_val.ch_val16.ch0 / 1000;
            break;
        case 6:
            lux = 2 * ch1_0_val.ch_val16.ch0 / 100;
            break;
        case 7:
            lux = 1 * ch1_0_val.ch_val16.ch0 / 100;
            break;
    }
    SENSOR_VERBOSE("Light ch0 Lux=%d\n", lux);
    SENSOR_INFO("light sensor value : %d, gain:%d, ch0:%d\n", lux, gain, ch1_0_val.ch_val16.ch0);
    sensor_data->s.lux.val = lux;
}

int32_t  Initialize_sensors_handle(void)
{
    qapi_Status_t status;

#ifdef CONFIG_CDB_PLATFORM
    status = qapi_I2CM_Open(QAPI_I2CM_INSTANCE_002_E, &h1);
#else
    status = qapi_I2CM_Open(QAPI_I2CM_INSTANCE_001_E, &h1);
#endif

    if (status != QAPI_OK)
    {
        SENSOR_ERROR( "i2c instance open failed\n");
        return -1;
    }

    qurt_signal_init(&i2c_ready_signal);

    activate_onboard_sensors();
    return 0;
}

int32_t Deinitialize_sensors_handle(void)
{
    int32_t status;

    deactivate_onboard_sensors();

    qurt_signal_delete(&i2c_ready_signal);

    status = qapi_I2CM_Close(h1);
    if (status != QAPI_OK)
    {
        SENSOR_ERROR( "I2C close failed\n");
        return -1;
    }
    return 0;
}
