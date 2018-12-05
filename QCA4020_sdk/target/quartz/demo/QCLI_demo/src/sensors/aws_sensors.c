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
#include "stdint.h"
#include <qcli.h>
#include <qcli_api.h>
#include <qurt_timer.h>

#include <qapi/qapi_status.h>
#include <qapi_i2c_master.h>

#include   "sensors.h"

extern  QCLI_Group_Handle_t qcli_sensors_group;              /* Handle for our QCLI Command Group. */
extern  qurt_signal_t i2c_ready_signal;
extern  void    *h1;                                         /* I2C client handle */


extern  qapi_I2CM_Config_t config_humidity;
extern  qapi_I2CM_Config_t config_light_LTR303ALS;

int  sensor_light_value;
char sensor_humidity_value_buf[20];
char sensor_temp_value_buf[20];

void sensor_humidity_update()
{
	uint8_t    reg_val;
	
	uint16_t   T0_DegCx8, T1_DegCx8, T_current;
	uint8_t    T0_T1_MSB;
	int16_t    T0_OUT_val, T1_OUT_val, T_OUT_val;
	uint32_t   T0_DegCx8_f, T1_DegCx8_f, T_DegCx8_fx10;
	
	int16_t    H0_T0_OUT_val, H1_T0_OUT_val, H_OUT_val;
	uint16_t   H0_rHx2, H1_rHx2, H_current;
	uint32_t   H_rHx2_fx10;

	reg_val = read_sensor_reg8(&config_humidity, HUMIDITY_I2C_REG_ADDR_CTRL_1);	

	// active mode
	write_sensor_reg8(&config_humidity, HUMIDITY_I2C_REG_ADDR_CTRL_1, (reg_val | 0x80));		

	// read temperature
	T_OUT_val = humidity_read_sensor_reg16(&config_humidity, HUMIDITY_I2C_REG_ADDR_T_OUT);
	
	T0_DegCx8 = read_sensor_reg8(&config_humidity, HUMIDITY_I2C_REG_ADDR_T0_DegCx8);
	T1_DegCx8 = read_sensor_reg8(&config_humidity, HUMIDITY_I2C_REG_ADDR_T1_DegCx8);
	T0_T1_MSB = read_sensor_reg8(&config_humidity, HUMIDITY_I2C_REG_ADDR_T0_T1_MSB);
	
	T0_DegCx8 |= ((uint16_t)(T0_T1_MSB & 3)) << 8;
	T1_DegCx8 |= ((uint16_t)((T0_T1_MSB >> 2) & 3)) << 8;
	
	T0_OUT_val = humidity_read_sensor_reg16(&config_humidity, HUMIDITY_I2C_REG_ADDR_T0_OUT);
	T1_OUT_val = humidity_read_sensor_reg16(&config_humidity, HUMIDITY_I2C_REG_ADDR_T1_OUT);
	
	T0_DegCx8_f = T0_DegCx8;
	T1_DegCx8_f = T1_DegCx8;
	T_DegCx8_fx10 = (T1_DegCx8_f - T0_DegCx8_f)  * (T_OUT_val - T0_OUT_val) * 10 / (T1_OUT_val - T0_OUT_val) + T0_DegCx8_f * 10;
	
	T_current = T_DegCx8_fx10 / 8;
	QCLI_Printf(qcli_sensors_group, "Current Temperature:%d.%d\n", T_current / 10, T_current % 10);
	snprintf(sensor_temp_value_buf, sizeof(sensor_temp_value_buf), "%d.%d", T_current / 10, T_current % 10);

	// read humidity
	H_OUT_val = humidity_read_sensor_reg16(&config_humidity, HUMIDITY_I2C_REG_ADDR_H_OUT);
	
	H0_rHx2 = read_sensor_reg8(&config_humidity, HUMIDITY_I2C_REG_ADDR_H0_rHx2);
	H1_rHx2 = read_sensor_reg8(&config_humidity, HUMIDITY_I2C_REG_ADDR_H1_rHx2);
		
	H0_T0_OUT_val = humidity_read_sensor_reg16(&config_humidity, HUMIDITY_I2C_REG_ADDR_H0_T0_OUT);
	H1_T0_OUT_val = humidity_read_sensor_reg16(&config_humidity, HUMIDITY_I2C_REG_ADDR_H1_T0_OUT);
	
	H_rHx2_fx10 = (H1_rHx2 - H0_rHx2)  * (H_OUT_val - H0_T0_OUT_val) * 10 / (H1_T0_OUT_val - H0_T0_OUT_val) + H0_rHx2 * 10;
	
	H_current = H_rHx2_fx10 / 2;
	QCLI_Printf(qcli_sensors_group, "Current rH:%d.%d%% rH\n", H_current / 10, H_current % 10);
	
	snprintf(sensor_humidity_value_buf, sizeof(sensor_humidity_value_buf), "%d.%d%%", H_current / 10, H_current % 10);
		
	// deactive mode
	reg_val = read_sensor_reg8(&config_humidity, HUMIDITY_I2C_REG_ADDR_CTRL_1);	
	write_sensor_reg8(&config_humidity, HUMIDITY_I2C_REG_ADDR_CTRL_1, (reg_val & 0x7F));		
}

void sensor_Light_update()
{
	uint8_t    reg_val;
	union  {
		uint32_t   ch_val32;
		struct  {
			uint16_t	ch1;
			uint16_t    ch0;
		} ch_val16;
	} ch1_0_val;
	uint8_t       ALS_CONTR_val, gain;
	uint32_t       lux =0;

	// active mode		
	reg_val = read_sensor_reg8(&config_light_LTR303ALS, LTR303ALS_I2C_REG_ADDR_ALS_CONTR);
	write_sensor_reg8(&config_light_LTR303ALS, LTR303ALS_I2C_REG_ADDR_ALS_CONTR, (reg_val | 1));

	// read measured value	
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
		
	sensor_light_value = lux;
	
	// deactive mode		
	reg_val = read_sensor_reg8(&config_light_LTR303ALS, LTR303ALS_I2C_REG_ADDR_ALS_CONTR);
	write_sensor_reg8(&config_light_LTR303ALS, LTR303ALS_I2C_REG_ADDR_ALS_CONTR, (reg_val & 0x7F));
}

void aws_sensors_update()
{
	qapi_Status_t status;

	status = qapi_I2CM_Open(QAPI_I2CM_INSTANCE_001_E, &h1);
	if (status != QAPI_OK)
		return;

    qurt_signal_init(&i2c_ready_signal);

	sensor_humidity_update();
	sensor_Light_update();
	
	qurt_signal_destroy(&i2c_ready_signal);
	qapi_I2CM_Close(h1);

	QCLI_Printf(qcli_sensors_group, "aws_sensors_update done !!!\n");	
}
