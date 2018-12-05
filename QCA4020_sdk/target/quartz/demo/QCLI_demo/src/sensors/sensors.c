/*
 * Copyright (c) 2015-2018 Qualcomm Technologies, Inc.
 * 2015-2016 Qualcomm Atheros, Inc.
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

#include    "qapi/qapi_status.h"

#include <qapi_i2c_master.h>
#include "qapi_tlmm.h"
#include "qapi_gpioint.h"
#include "sensors_demo.h"

#include  "sensors.h"

#define I2C_wait(msec)    do { \
                              qurt_time_t qtime;\
                              qtime = qurt_timer_convert_time_to_ticks(msec, QURT_TIME_MSEC);\
                              qurt_thread_sleep(qtime);\
                          } while (0)

#define  I2C_ACT_READ			1
#define  I2C_ACT_WRITE			0
#define PIR_THREAD_STACK_SIZE		(512)
#define PIR_THREAD_PRIORITY		(10)
#define PIR_PIN				27
#define PIR_THREAD_SIGNAL_INTR		(1<<0)
#define PIR_THREAD_STOP			(1<<1)


void *h1;

sensors_val_t	sensor_vals;
extern  QCLI_Group_Handle_t qcli_sensors_group;              /* Handle for our QCLI Command Group. */


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



void I2CM_Transfer_cb(
    const uint32_t status,
    void *CB_Parameter
)
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
	uint8_t reg_val;
	
	if (reg_addr != 0x0F && reg_addr != 0x10 && reg_addr != 0x20 && reg_addr != 0x21 && reg_addr != 0x22 && reg_addr != 0x27)
	{
		QCLI_Printf(qcli_sensors_group, "Humidity wrong Register Addr:%x\n", reg_addr);
		return;
	}
	reg_val = read_sensor_reg8(&config_humidity, reg_addr);
	
	QCLI_Printf(qcli_sensors_group, "Humidity Sensor Reg:0x%X  Val:0x%X\n", reg_addr, reg_val);
	return;	
}

void write_humidity_reg(uint32_t reg_addr, uint8_t reg_val)
{	
	if (reg_addr != 0x10 && reg_addr != 0x20 && reg_addr != 0x21 && reg_addr != 0x22)
	{
		QCLI_Printf(qcli_sensors_group, "Humidity wrong Register Addr:%x\n", reg_addr);
		return;
	}
	write_sensor_reg8(&config_humidity, reg_addr, reg_val);
	
	QCLI_Printf(qcli_sensors_group, "Humidity Sensor Reg:0x%X  Val:0x%X\n", reg_addr, reg_val);
	return;	
}

int32_t sensors_humidity_get_measured_value()
{
	uint16_t   T0_DegCx8, T1_DegCx8, T_current;
	uint8_t    T0_T1_MSB;
	int16_t    T0_OUT_val, T1_OUT_val, T_OUT_val;
	uint32_t   T0_DegCx8_f, T1_DegCx8_f, T_DegCx8_fx10;
	uint16_t   H0_rHx2, H1_rHx2, H_current;
	uint32_t   H_rHx2_fx10;
	int16_t    H0_T0_OUT_val, H1_T0_OUT_val, H_OUT_val;

	// read temperature
	QCLI_Printf(qcli_sensors_group, "  ------  Temperature ------\n");
	T_OUT_val = humidity_read_sensor_reg16(&config_humidity, HUMIDITY_I2C_REG_ADDR_T_OUT);
	
	T0_DegCx8 = read_sensor_reg8(&config_humidity, HUMIDITY_I2C_REG_ADDR_T0_DegCx8);
	T1_DegCx8 = read_sensor_reg8(&config_humidity, HUMIDITY_I2C_REG_ADDR_T1_DegCx8);
	T0_T1_MSB = read_sensor_reg8(&config_humidity, HUMIDITY_I2C_REG_ADDR_T0_T1_MSB);
	QCLI_Printf(qcli_sensors_group, "DegCx8 T0:0x%x  T1:0x%x  T0_T_MSB:%x\n", T0_DegCx8, T1_DegCx8, T0_T1_MSB);
	
	T0_DegCx8 |= ((uint16_t)(T0_T1_MSB & 3)) << 8;
	T1_DegCx8 |= ((uint16_t)((T0_T1_MSB >> 2) & 3)) << 8;
	QCLI_Printf(qcli_sensors_group, "DegCx8 with MSB T0:0x%x  T1:0x%x\n", T0_DegCx8, T1_DegCx8);
	
	T0_OUT_val = humidity_read_sensor_reg16(&config_humidity, HUMIDITY_I2C_REG_ADDR_T0_OUT);
	T1_OUT_val = humidity_read_sensor_reg16(&config_humidity, HUMIDITY_I2C_REG_ADDR_T1_OUT);
	QCLI_Printf(qcli_sensors_group, "register 2's comp T0:%d T1:%d T:%d\n", T0_OUT_val, T1_OUT_val, T_OUT_val);
	
	T0_DegCx8_f = T0_DegCx8;
	T1_DegCx8_f = T1_DegCx8;
	T_DegCx8_fx10 = (T1_DegCx8_f - T0_DegCx8_f)  * (T_OUT_val - T0_OUT_val) * 10 / (T1_OUT_val - T0_OUT_val) + T0_DegCx8_f * 10;
	
	QCLI_Printf(qcli_sensors_group, "DegCx8 T0:%d T1:%d Tx10:%d\n", T0_DegCx8_f, T1_DegCx8_f, T_DegCx8_fx10);
	T_current = T_DegCx8_fx10 / 8;
	QCLI_Printf(qcli_sensors_group, "Current Temperature:%d.%d\n", T_current / 10, T_current % 10);
	
	
	// read humidity
	QCLI_Printf(qcli_sensors_group, "\n------  Relative Humidity ------\n");
	H_OUT_val = humidity_read_sensor_reg16(&config_humidity, HUMIDITY_I2C_REG_ADDR_H_OUT);
	
	H0_rHx2 = read_sensor_reg8(&config_humidity, HUMIDITY_I2C_REG_ADDR_H0_rHx2);
	H1_rHx2 = read_sensor_reg8(&config_humidity, HUMIDITY_I2C_REG_ADDR_H1_rHx2);
	QCLI_Printf(qcli_sensors_group, "rHx2 H0:0x%x  H1:0x%x\n", H0_rHx2, H1_rHx2);
		
	H0_T0_OUT_val = humidity_read_sensor_reg16(&config_humidity, HUMIDITY_I2C_REG_ADDR_H0_T0_OUT);
	H1_T0_OUT_val = humidity_read_sensor_reg16(&config_humidity, HUMIDITY_I2C_REG_ADDR_H1_T0_OUT);
	QCLI_Printf(qcli_sensors_group, "2's comp H0_T0:%d H1_T0:%d H:%d\n", H0_T0_OUT_val, H1_T0_OUT_val, H_OUT_val);
	
	H_rHx2_fx10 = (H1_rHx2 - H0_rHx2)  * (H_OUT_val - H0_T0_OUT_val) * 10 / (H1_T0_OUT_val - H0_T0_OUT_val) + H0_rHx2 * 10;
	
	QCLI_Printf(qcli_sensors_group, "rHx2 Hx10:%d\n", H_rHx2_fx10);
	H_current = H_rHx2_fx10 / 2;
	QCLI_Printf(qcli_sensors_group, "Current rH:%d.%d%% rH\n", H_current / 10, H_current % 10);
	
	return 0;
}

int32_t sensors_humidity_driver_test(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	qapi_Status_t status;
	uint32_t select, reg_addr;
	uint32_t   val;
	uint8_t    reg_val;

#ifdef CONFIG_CDB_PLATFORM
	status = qapi_I2CM_Open(QAPI_I2CM_INSTANCE_002_E, &h1);
#else	
	status = qapi_I2CM_Open(QAPI_I2CM_INSTANCE_001_E, &h1);
#endif
    if (status != QAPI_OK)
		return -1;

    qurt_signal_init(&i2c_ready_signal);
	
	if (Parameter_Count == 0)
	{
		sensor_vals.humidity.who_am_i = read_sensor_reg8(&config_humidity, HUMIDITY_I2C_REG_ADDR_WHO_AM_I);	
		QCLI_Printf(qcli_sensors_group, "Humidity ID=0x%02x\n", sensor_vals.humidity.who_am_i);
	}
	else
	{
		
		select = Parameter_List->Integer_Value;
		Parameter_List++;
		
		switch (select)
		{
		case   0:		// activate the sensor
			val = Parameter_List->Integer_Value;
			Parameter_List++;
			reg_val = read_sensor_reg8(&config_humidity, HUMIDITY_I2C_REG_ADDR_CTRL_1);	
			if (val != 0)
			{           // active mode
				write_sensor_reg8(&config_humidity, HUMIDITY_I2C_REG_ADDR_CTRL_1, (reg_val | 0x80));		
			}
			else
			{
				write_sensor_reg8(&config_humidity, HUMIDITY_I2C_REG_ADDR_CTRL_1, (reg_val & 0x7F));
			}
			break;
		case   1:		// read register
			reg_addr = Parameter_List->Integer_Value;
			Parameter_List++;
			read_humidity_reg(reg_addr);
			break;
		case   2:		// write register
			reg_addr = Parameter_List->Integer_Value;
			Parameter_List++;
			reg_val = Parameter_List->Integer_Value;
			Parameter_List++;
			write_humidity_reg(reg_addr, reg_val);
			break;
		case   3:
			sensors_humidity_get_measured_value();	
			break;	
		}
	}

    qurt_signal_destroy(&i2c_ready_signal);
    status = qapi_I2CM_Close(h1);
	if (status != QAPI_OK)
		return -1;

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
	QCLI_Printf(qcli_sensors_group, "power mode=%d osrs p=%d osrs t=%d\n", pwr_mode, osrs_p, osrs_t);
	
	reg_val = read_sensor_reg8(&config_pressure, PRESSURE_I2C_REG_ADDR_CONF);	
	filter = (reg_val >> 2) & 0x07;
	t_sb =  (reg_val >> 5) & 0x07;
	QCLI_Printf(qcli_sensors_group, "filter=%d t_sb=%d\n", filter, t_sb);

	reg_val = read_sensor_reg8(&config_pressure, PRESSURE_I2C_REG_ADDR_STATUS);	
	QCLI_Printf(qcli_sensors_group, "status=%x\n", reg_val);
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
	int32_t  var1, var2, T;
	
	var1 = ((((ut >> 3) - ((int32_t)dig_T1 << 1))) * ((int32_t)dig_T2)) >> 11;
	var2 = (((((ut>>4) - ((int32_t)dig_T1)) * ((ut>>4) - ((int32_t)dig_T1))) >> 12) * ((int32_t)dig_T3)) >> 14;
	t_fine = var1 + var2;
	T = (t_fine + 5 + 128) >> 8;
	return T;
}

void sensors_pressure_get_measured_values()
{
	uint32_t  ut, up;
	union {
		uint32_t   val32;
		uint8_t    val8[4];
	} reg_val32;
	uint16_t   dig_T1;
	int16_t	   dig_T2, dig_T3;
	int32_t    T;
	
	reg_val32.val32 = sensors_pressure_read_sensor_bits24(&config_pressure, PRESSURE_I2C_REG_ADDR_PRESS);
	up = (((uint32_t)(reg_val32.val8[0])) << 12) | (((uint32_t)(reg_val32.val8[1])) << 4) | ((((uint32_t)(reg_val32.val8[2])) >> 4) & 0x0F);
	QCLI_Printf(qcli_sensors_group, "Pressure=0x%08X\n", up);
										
	reg_val32.val32 = sensors_pressure_read_sensor_bits24(&config_pressure, PRESSURE_I2C_REG_ADDR_TEMP);
	ut = (((uint32_t)(reg_val32.val8[0])) << 12) | (((uint32_t)(reg_val32.val8[1])) << 4) | ((((uint32_t)(reg_val32.val8[2])) >> 4) & 0x0F);
	QCLI_Printf(qcli_sensors_group, "Temp=0x%08X\n", ut);
	dig_T1 = read_sensor_reg16(&config_pressure, PRESSURE_I2C_REG_ADDR_dig_T1);
	dig_T2 = read_sensor_reg16(&config_pressure, PRESSURE_I2C_REG_ADDR_dig_T2);
	dig_T3 = read_sensor_reg16(&config_pressure, PRESSURE_I2C_REG_ADDR_dig_T3);
    T = bmp820_compensate_T(ut, dig_T1, dig_T2, dig_T3);
	QCLI_Printf(qcli_sensors_group, "Current Temp DegC=%d.%d\n", T/100, T%100);
}

int32_t sensors_pressure_driver_test( uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List )
{
	qapi_Status_t status;
	uint8_t      id, val;
	uint32_t 	 select;
	uint8_t      osrs_p=0xFF, osrs_t=0xFF, filter=0xFF, t_sb=0xFF;

#ifdef CONFIG_CDB_PLATFORM
	status = qapi_I2CM_Open(QAPI_I2CM_INSTANCE_002_E, &h1);
#else	
	status = qapi_I2CM_Open(QAPI_I2CM_INSTANCE_001_E, &h1);
#endif
	if (status != QAPI_OK)
		return -1;

    qurt_signal_init(&i2c_ready_signal);
	
	if (Parameter_Count == 0)
	{
		id = read_sensor_reg8(&config_pressure, PRESSURE_I2C_REG_ADDR_ID);	
		QCLI_Printf(qcli_sensors_group, "BMP280 ID=0x%02x\n", id);
	}
	else
	{
		select = Parameter_List->Integer_Value;
		Parameter_List++;
		
		switch (select)
		{
		case   0:		// set power mode
			val = Parameter_List->Integer_Value;
			Parameter_List++;
			if (val > 3)
				break;
			
			sensors_pressure_set_power_mode(val);
			break;
		case   1:		// get setting
			sensors_pressure_get_parameters();
			break;
			
		case   2:		// set resolution
			if (Parameter_Count >= 2)
			{
				osrs_p = Parameter_List->Integer_Value;
				Parameter_List++;			
			}
			if (Parameter_Count >= 3)
			{
				osrs_t = Parameter_List->Integer_Value;
				Parameter_List++;
			}
			
			if (Parameter_Count >= 4)
			{
				filter = Parameter_List->Integer_Value;
				Parameter_List++;			
			}
			
			if (Parameter_Count >= 5)
			{
				t_sb = Parameter_List->Integer_Value;
				Parameter_List++;			
			}
			sensors_pressure_set_parameters(osrs_p, osrs_t, filter, t_sb);
			
			break;
		case   3:
			sensors_pressure_get_measured_values();
		
			break;
			
		}
	}
	
    qurt_signal_delete(&i2c_ready_signal);

    status = qapi_I2CM_Close(h1);
	if (status != QAPI_OK)
		return -1;
    
    return 0;
}

#ifdef CONFIG_CDB_PLATFORM
/** 
 * activate_onboard_sensors functions Initializes all the 
 * onboard sensors and write the appropriate data to register to actuate the sensors.
 */
int32_t activate_onboard_sensors(void)
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
int32_t deactivate_onboard_sensors(void) 
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
#endif

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
	QCLI_Printf(qcli_sensors_group, "ST1:0x%02X   CNTL2:0x%02X\n", status1_val, control2_val);
}

void sensors_compass_get_measured_values()
{
	int16_t   hx, hy, hz;
		
    hx = read_sensor_reg16(&config_compass, COMPASS_I2C_REG_ADDR_HX);
    hy = read_sensor_reg8(&config_compass, COMPASS_I2C_REG_ADDR_HY);
    hz = read_sensor_reg8(&config_compass, COMPASS_I2C_REG_ADDR_HZ);

	QCLI_Printf(qcli_sensors_group, "HX:%d   HY:%d   HZ:%d\n", hx, hy, hz);	
}

int32_t sensors_compass_driver_test( uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List )
{
	qapi_Status_t status;
	uint8_t     company_id, device_id;
	uint32_t	op_mode = 1;
	uint32_t 	 select;

#ifdef CONFIG_CDB_PLATFORM
	status = qapi_I2CM_Open(QAPI_I2CM_INSTANCE_002_E, &h1);
#else	
	status = qapi_I2CM_Open(QAPI_I2CM_INSTANCE_001_E, &h1);
#endif
    if (status != QAPI_OK)
		return -1;

    qurt_signal_init(&i2c_ready_signal);
	
	if (Parameter_Count == 0)
	{
		company_id = read_sensor_reg8(&config_compass, COMPASS_I2C_REG_ADDR_WIA1);	
		device_id = read_sensor_reg8(&config_compass, COMPASS_I2C_REG_ADDR_WIA2);	
		QCLI_Printf(qcli_sensors_group, "AK09911 Company ID=0x%02x Device ID:0x%02x\n", company_id, device_id);
	}
	else
	{
		select = Parameter_List->Integer_Value;
		Parameter_List++;
		
		switch (select)
		{
		case   0:		// set operation mode
			if (Parameter_Count >= 2)
			{
				op_mode = Parameter_List->Integer_Value;
				Parameter_List++;			
			}
			
			sensors_compass_set_operation_mode(op_mode);
			break;
		case  1:
			sensors_compass_get_parameters();
			break;
		case  2:
			break;
		case  3:
			sensors_compass_get_measured_values();
			break;
		};
	}
    qurt_signal_delete(&i2c_ready_signal);

    status = qapi_I2CM_Close(h1);
	if (status != QAPI_OK)
		return -1;
    
    return 0;
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
	QCLI_Printf(qcli_sensors_group, "CTRL1_XL:0x%02X  CTRL2_G:0x%02X\n", CTRL1_XL_val, CTRL2_G_val);

	status = read_sensor_reg8(&config_gyroscope_LSM6DS3, LSM6DS3_I2C_REG_ADDR_STATUS);
	QCLI_Printf(qcli_sensors_group, "STATUS:0x%02X\n", status);

	FIFO_CTRL1_val = read_sensor_reg8(&config_gyroscope_LSM6DS3, LSM6DS3_I2C_REG_ADDR_FIFO_CTRL1);
	FIFO_CTRL2_val = read_sensor_reg8(&config_gyroscope_LSM6DS3, LSM6DS3_I2C_REG_ADDR_FIFO_CTRL2);
	FIFO_CTRL3_val = read_sensor_reg8(&config_gyroscope_LSM6DS3, LSM6DS3_I2C_REG_ADDR_FIFO_CTRL3);
	FIFO_CTRL4_val = read_sensor_reg8(&config_gyroscope_LSM6DS3, LSM6DS3_I2C_REG_ADDR_FIFO_CTRL4);
	FIFO_CTRL5_val = read_sensor_reg8(&config_gyroscope_LSM6DS3, LSM6DS3_I2C_REG_ADDR_FIFO_CTRL5);
	QCLI_Printf(qcli_sensors_group, "FIFO_CTRL1:0x%02X  FIFO_CTRL2:0x%02X  FIFO_CTRL3:0x%02X FIFO_CTRL4:0x%02X FIFO_CTRL5:0x%02X\n",
			FIFO_CTRL1_val, FIFO_CTRL2_val, FIFO_CTRL3_val, FIFO_CTRL4_val, FIFO_CTRL5_val);
}

void sensors_gyroscope_get_measured_values()
{
	int16_t   OUT_TEMP_val, OUTX_G_val, OUTY_G_val, OUTZ_G_val;
	int16_t   OUTX_XL_val, OUTY_XL_val, OUTZ_XL_val;
	
	OUT_TEMP_val = read_sensor_reg16(&config_gyroscope_LSM6DS3, LSM6DS3_I2C_REG_ADDR_OUT_TEMP);
	OUTX_G_val = read_sensor_reg16(&config_gyroscope_LSM6DS3, LSM6DS3_I2C_REG_ADDR_OUTX_G);
	OUTY_G_val = read_sensor_reg16(&config_gyroscope_LSM6DS3, LSM6DS3_I2C_REG_ADDR_OUTY_G);
	OUTZ_G_val = read_sensor_reg16(&config_gyroscope_LSM6DS3, LSM6DS3_I2C_REG_ADDR_OUTZ_G);
	QCLI_Printf(qcli_sensors_group, "TEMP:%d   X_G:%d   Y_G:%d   Z_G:%d\n", OUT_TEMP_val, OUTX_G_val, OUTY_G_val, OUTZ_G_val);	
	
	OUTX_XL_val = read_sensor_reg16(&config_gyroscope_LSM6DS3, LSM6DS3_I2C_REG_ADDR_OUTX_XL);
	OUTY_XL_val = read_sensor_reg16(&config_gyroscope_LSM6DS3, LSM6DS3_I2C_REG_ADDR_OUTY_XL);
	OUTZ_XL_val = read_sensor_reg16(&config_gyroscope_LSM6DS3, LSM6DS3_I2C_REG_ADDR_OUTZ_XL);

	QCLI_Printf(qcli_sensors_group, "X_XL:%d   Y_XL:%d   Z_XL:%d\n", OUTX_XL_val, OUTY_XL_val, OUTZ_XL_val);	
}
 
int32_t sensors_gyroscope_driver_test( uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List )
{
	qapi_Status_t status;
	uint8_t      id;
	uint32_t     select;
	uint8_t      accelerometer_mode = 1, gyroscope_mode = 1;

#ifdef CONFIG_CDB_PLATFORM    
	status = qapi_I2CM_Open(QAPI_I2CM_INSTANCE_002_E, &h1);
#else	
	status = qapi_I2CM_Open(QAPI_I2CM_INSTANCE_001_E, &h1);
#endif
    if (status != QAPI_OK)
		return -1;

    qurt_signal_init(&i2c_ready_signal);
	if (Parameter_Count == 0)
	{
		id = read_sensor_reg8(&config_gyroscope_LSM6DS3, LSM6DS3_I2C_REG_ADDR_WIA);	
		QCLI_Printf(qcli_sensors_group, "LSM6DS3 ID=0x%02x\n", id);
	}
	else
	{		
		select = Parameter_List->Integer_Value;
		Parameter_List++;
		
		switch (select)
		{
		case   0:		// set operation mode
			if (Parameter_Count >= 2)
			{
				accelerometer_mode = Parameter_List->Integer_Value;
				Parameter_List++;			
			}
			if (Parameter_Count >= 3)
			{
				gyroscope_mode = Parameter_List->Integer_Value;
				Parameter_List++;			
			}
			
			sensors_gyroscope_set_operation_mode(accelerometer_mode, gyroscope_mode);
			break;
		case  1:
			sensors_gyroscope_get_parameters();
			break;
		case  2:
			break;
		case  3:
			sensors_gyroscope_get_measured_values();
			break;
		};
	}
    qurt_signal_delete(&i2c_ready_signal);

    status = qapi_I2CM_Close(h1);
	if (status != QAPI_OK)
		return -1;

    return 0;
}

/*
 * Ambinet Light Sensor
 */ 
uint32_t    i2c_initial = 0;

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
	QCLI_Printf(qcli_sensors_group, "LTR303ALS CNTR=0x%02x MEAS RATE=0x%02X STATUS=0x%02X INTERRUPT=0x%02X\n", 
										ALS_CONTR_val, ALS_MEAS_RATE_val, ALS_STATUS_val, INTERRUPT_val);

	ALS_THRES_UP_val = read_sensor_reg16(&config_light_LTR303ALS, LTR303ALS_I2C_REG_ADDR_ALS_THRES_UP);
	ALS_THRES_LOW_val = read_sensor_reg16(&config_light_LTR303ALS, LTR303ALS_I2C_REG_ADDR_ALS_THRES_LOW);
	QCLI_Printf(qcli_sensors_group, "LTR303ALS THRES_UP=0x%04x THRES_LOW=0x%04X\n", ALS_THRES_UP_val, ALS_THRES_LOW_val);
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

void sensors_light_LTR303ALS_get_measured_values()
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

	QCLI_Printf(qcli_sensors_group, "CH 1:%d   CH 0:%d\n", ch1_0_val.ch_val16.ch1, ch1_0_val.ch_val16.ch0);
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
	QCLI_Printf(qcli_sensors_group, "Light ch0 Lux=%d\n", lux);
}

int32_t sensors_light_driver_test( uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List )
{
	qapi_Status_t status;
	uint8_t       part_id, manufactory_id;
	uint8_t       mode = 0, select;
	uint32_t 	  als_gain = 0, als_integration_time = 0, als_measure_rate = 3;

#ifdef CONFIG_CDB_PLATFORM
	status = qapi_I2CM_Open(QAPI_I2CM_INSTANCE_002_E, &h1);
#else	
	status = qapi_I2CM_Open(QAPI_I2CM_INSTANCE_001_E, &h1);
#endif    
	if (status != QAPI_OK)
		return -1;

    qurt_signal_init(&i2c_ready_signal);
	if (Parameter_Count == 0)
	{
		part_id = read_sensor_reg8(&config_light_LTR303ALS, LTR303ALS_I2C_REG_ADDR_PART_ID);
		manufactory_id = read_sensor_reg8(&config_light_LTR303ALS, LTR303ALS_I2C_REG_ADDR_MANUFAC_ID);
		QCLI_Printf(qcli_sensors_group, "LTR303ALS PART ID=0x%02x MANUFACTURER ID=0x%02X\n", part_id, manufactory_id);
	}
	else
	{
		select = Parameter_List->Integer_Value;
		Parameter_List++;
		
		switch (select)
		{
		case   0:		// set operation mode
			if (Parameter_Count >= 2)
			{
				mode = Parameter_List->Integer_Value;
				Parameter_List++;			
			}
			
			sensors_light_LTR303ALS_set_operation_mode(mode);
			break;
		case   1:
			sensors_light_LTR303ALS_get_parameters();
			break;
		case   2:
			if (Parameter_Count >= 2)
			{
				als_gain = Parameter_List->Integer_Value;
				Parameter_List++;			
			}
			if (Parameter_Count >= 3)
			{
				als_integration_time = Parameter_List->Integer_Value;
				Parameter_List++;			
			}
			if (Parameter_Count >= 4)
			{
				als_measure_rate = Parameter_List->Integer_Value;
				Parameter_List++;			
			}
			sensors_light_LTR303ALS_set_parameters(als_gain, als_integration_time, als_measure_rate);

			break;
		case   3:
			sensors_light_LTR303ALS_get_measured_values();
			break;
		}
	}
    qurt_signal_delete(&i2c_ready_signal);
	
    status = qapi_I2CM_Close(h1);
	if (status != QAPI_OK)
		return -1;
    
    return 0;
}

#ifdef CONFIG_CDB_PLATFORM

static qurt_signal_t pir_int_signal;
/**
 * Int_Callback function is to handle the PIR interrupts
 */
void pir_int_callback(qapi_GPIOINT_Callback_Data_t data)
{
	qurt_signal_set(&pir_int_signal, PIR_THREAD_SIGNAL_INTR);
	return;
}

/**
 * func(): PIR_Thread handles the interrupt genearted by
 * PIR sensors
 */
void pir_thread(void *param)
{
	int32_t gpio_pin = PIR_PIN;
	uint32_t sig;

	// Necessary Data Type declarations
	qapi_GPIO_ID_t  gpio_id;
	qapi_Instance_Handle_t pH1;
	qapi_Status_t status = QAPI_OK;
	qapi_TLMM_Config_t tlmm_config;

	tlmm_config.pin = gpio_pin;
	tlmm_config.func = 0;   // Using the functionality tied to pin mux value
	tlmm_config.dir =  QAPI_GPIO_INPUT_E;
	tlmm_config.pull = QAPI_GPIO_PULL_DOWN_E;
	tlmm_config.drive = QAPI_GPIO_2MA_E; // drive is for output pins

	status = qapi_TLMM_Get_Gpio_ID(&tlmm_config, &gpio_id);

	if (status == QAPI_OK)
	{
		status = qapi_TLMM_Config_Gpio(gpio_id, &tlmm_config);
		if (status != QAPI_OK)
		{
			QCLI_Printf(qcli_sensors_group, "Failed to config GPIO\n");
			goto release_gpio;
		}
	}

	if (qapi_GPIOINT_Register_Interrupt(&pH1, gpio_pin, (qapi_GPIOINT_CB_t )pir_int_callback,
		0, QAPI_GPIOINT_TRIGGER_EDGE_DUAL_E, QAPI_GPIOINT_PRIO_MEDIUM_E, false) != QAPI_OK)
	{
		QCLI_Printf(qcli_sensors_group, "Interrupt Registeration failed\n");
		goto release_gpio;
	}

	while(1)
	{
		sig = qurt_signal_wait(&pir_int_signal, (PIR_THREAD_STOP|PIR_THREAD_SIGNAL_INTR),
				QURT_SIGNAL_ATTR_WAIT_ANY | QURT_SIGNAL_ATTR_CLEAR_MASK);

		if (sig & PIR_THREAD_SIGNAL_INTR)
		{
			QCLI_Printf(qcli_sensors_group, "PIR interrupt signal received!\n");
		}
		if (sig & PIR_THREAD_STOP)
		{
			break;
		}
	}

	QCLI_Printf(qcli_sensors_group, "Signal received to disable PIR\n");
	// Deregister the GPIO Interrupt
	status = qapi_GPIOINT_Deregister_Interrupt(&pH1, gpio_pin);
	if (status != QAPI_OK)
		QCLI_Printf(qcli_sensors_group, "Deregistering the Interrupt failed\n");

release_gpio:
	if (qapi_TLMM_Release_Gpio_ID(&tlmm_config, gpio_id ) != QAPI_OK)
		QCLI_Printf(qcli_sensors_group, "GPIO pin %d release Failed\n", gpio_pin);

	qurt_signal_delete(&pir_int_signal);
	
	qurt_thread_stop();
	return;
}

/**
 * Start the PIR thread to handle signal.
 */
QCLI_Command_Status_t sensors_pir_driver_test(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	int ret;
	static int pir_enabled = 0;
	qurt_thread_attr_t thread_attribute;
	qurt_thread_t      thread_handle;

	if (Parameter_Count != 1)
	{
		QCLI_Printf(qcli_sensors_group, "USAGE: number\n  number = 0:disable pir | 1:enable pir");
		return 0;
	}

	if (pir_enabled == !!Parameter_List[0].Integer_Value)
	{
		QCLI_Printf(qcli_sensors_group, "PIR Interrupt is already %s\n", pir_enabled ? "enabled" : "disabled");
		return 0;
	}

	if (Parameter_List[0].Integer_Value)
	{
		if (0 != qurt_signal_init(&pir_int_signal))
		{
			QCLI_Printf(qcli_sensors_group, "Not able to initialize signal\n");
			return -1;
		}
		qurt_thread_attr_init(&thread_attribute);
		qurt_thread_attr_set_name(&thread_attribute, "pir_thread");
		qurt_thread_attr_set_priority(&thread_attribute, PIR_THREAD_PRIORITY);
		qurt_thread_attr_set_stack_size(&thread_attribute, PIR_THREAD_STACK_SIZE);

		ret = qurt_thread_create(&thread_handle, &thread_attribute, pir_thread, NULL);
		if (ret != QAPI_OK)
		{
			QCLI_Printf(qcli_sensors_group, "PIR thread creation failed\n");
			return -1;
		}
		pir_enabled = 1;
	}
	else
	{
		qurt_signal_set(&pir_int_signal, PIR_THREAD_STOP);
		qurt_thread_sleep(5);
		pir_enabled = 0;
	}
	return 0;
}

/**
 * func: sensors_read_all() reads all the sensors values 
 */
QCLI_Command_Status_t sensors_read_all(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
	qapi_Status_t status;

	status = qapi_I2CM_Open(QAPI_I2CM_INSTANCE_002_E, &h1);

	if (status != QAPI_OK)
	{
		QCLI_Printf(qcli_sensors_group, "i2c instance open failed\n");
		return -1;
	}

	qurt_signal_init(&i2c_ready_signal);

	activate_onboard_sensors();

	sensors_humidity_get_measured_value();
	QCLI_Printf(qcli_sensors_group, "\n  ------  Pressure Sensors ------\n");
	sensors_pressure_get_measured_values();
	QCLI_Printf(qcli_sensors_group, "\n  ------  Compass & Magenetometer Sensors ------\n");
	sensors_compass_get_measured_values();
	QCLI_Printf(qcli_sensors_group, "\n  ------  Gyro & Accelerometer Sensors ------\n");
	sensors_gyroscope_get_measured_values();
	QCLI_Printf(qcli_sensors_group, "\n  ------  Light Sensors ------\n");
	sensors_light_LTR303ALS_get_measured_values();

	deactivate_onboard_sensors();

	status = qapi_I2CM_Close(h1);
	if (status != QAPI_OK)
	{
		QCLI_Printf(qcli_sensors_group, "I2C close failed\n");
		return -1;
	}
	return 0;
}
#endif

