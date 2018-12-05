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

#ifndef __SENSORS__H__
#define __SENSORS__H__

#define	 I2C_ADDR_HUMIDITY		0x5F
#define  HUMIDITY_I2C_ADDR(act) ((I2C_ADDR_HUMIDITY << 1) | (act))

#define  HUMIDITY_I2C_REG_ADDR_WHO_AM_I		0x0F
#define  HUMIDITY_I2C_REG_ADDR_AV_CONF		0x10
#define  HUMIDITY_I2C_REG_ADDR_CTRL_1		0x20
#define  HUMIDITY_I2C_REG_ADDR_CTRL_2		0x21
#define  HUMIDITY_I2C_REG_ADDR_CTRL_3		0x22
#define  HUMIDITY_I2C_REG_ADDR_STATUS		0x27
#define  HUMIDITY_I2C_REG_ADDR_H_OUT		0x28
#define  HUMIDITY_I2C_REG_ADDR_T_OUT	    0x2A
#define  HUMIDITY_I2C_REG_ADDR_H0_rHx2  	0x30
#define  HUMIDITY_I2C_REG_ADDR_H1_rHx2  	0x31
#define  HUMIDITY_I2C_REG_ADDR_T0_DegCx8  	0x32
#define  HUMIDITY_I2C_REG_ADDR_T1_DegCx8  	0x33
#define  HUMIDITY_I2C_REG_ADDR_T0_T1_MSB  	0x35
#define  HUMIDITY_I2C_REG_ADDR_H0_T0_OUT  	0x36
#define  HUMIDITY_I2C_REG_ADDR_H1_T0_OUT  	0x3A
#define  HUMIDITY_I2C_REG_ADDR_T0_OUT  	    0x3C
#define  HUMIDITY_I2C_REG_ADDR_T1_OUT  	    0x3E

#define	 I2C_ADDR_PRESSURE					0x76

#define  PRESSURE_I2C_REG_ADDR_ID		    0xD0
#define  PRESSURE_I2C_REG_ADDR_RESET		0xE0
#define  PRESSURE_I2C_REG_ADDR_STATUS		0xF3
#define  PRESSURE_I2C_REG_ADDR_CTRL_MEAS	0xF4
#define  PRESSURE_I2C_REG_ADDR_CONF			0xF5

#define  PRESSURE_I2C_REG_ADDR_PRESS		0xF7
#define  PRESSURE_I2C_REG_ADDR_PRESS_MSB	0xF7
#define  PRESSURE_I2C_REG_ADDR_PRESS_LSB	0xF8
#define  PRESSURE_I2C_REG_ADDR_PRESS_XLSB	0xF9

#define  PRESSURE_I2C_REG_ADDR_TEMP			0xFA
#define  PRESSURE_I2C_REG_ADDR_TEMP_MSB		0xFA
#define  PRESSURE_I2C_REG_ADDR_TEMP_LSB		0xFB
#define  PRESSURE_I2C_REG_ADDR_TEMP_XLSB	0xFC

#define  PRESSURE_I2C_REG_ADDR_dig_T1		0x88
#define  PRESSURE_I2C_REG_ADDR_dig_T2		0x8A
#define  PRESSURE_I2C_REG_ADDR_dig_T3		0x8C

#define  PRESSURE_I2C_REG_ADDR_dig_P1		0x8E
#define  PRESSURE_I2C_REG_ADDR_dig_P2		0x90
#define  PRESSURE_I2C_REG_ADDR_dig_P3		0x92
#define  PRESSURE_I2C_REG_ADDR_dig_P4		0x94
#define  PRESSURE_I2C_REG_ADDR_dig_P5		0x96
#define  PRESSURE_I2C_REG_ADDR_dig_P6		0x98
#define  PRESSURE_I2C_REG_ADDR_dig_P7		0x9A
#define  PRESSURE_I2C_REG_ADDR_dig_P8		0x9C
#define  PRESSURE_I2C_REG_ADDR_dig_P9		0x9E


/*
 * compass and magnetometer
 */
 
#define	 I2C_ADDR_COMPASS_AK09911C			0x0C

#define  COMPASS_I2C_REG_ADDR_WIA1		    0x00
#define  COMPASS_I2C_REG_ADDR_WIA2		    0x01
#define  COMPASS_I2C_REG_ADDR_INFO		    0x02
#define  COMPASS_I2C_REG_ADDR_ST1		    0x10

#define  COMPASS_I2C_REG_ADDR_HX		    0x11
#define  COMPASS_I2C_REG_ADDR_HX_L		    0x11
#define  COMPASS_I2C_REG_ADDR_HX_H		    0x12

#define  COMPASS_I2C_REG_ADDR_HY		    0x13
#define  COMPASS_I2C_REG_ADDR_HY_L		    0x13
#define  COMPASS_I2C_REG_ADDR_HY_H		    0x14

#define  COMPASS_I2C_REG_ADDR_HZ		    0x15
#define  COMPASS_I2C_REG_ADDR_HZ_L		    0x15
#define  COMPASS_I2C_REG_ADDR_HZ_H		    0x16

#define  COMPASS_I2C_REG_ADDR_TMPS		    0x17
#define  COMPASS_I2C_REG_ADDR_ST2		    0x18

#define  COMPASS_I2C_REG_ADDR_CNTL1		    0x30
#define  COMPASS_I2C_REG_ADDR_CNTL2		    0x31
#define  COMPASS_I2C_REG_ADDR_CNTL3		    0x32
#define  COMPASS_I2C_REG_ADDR_TS1		    0x33

#define  COMPASS_I2C_REG_ADDR_ASAX		    0x60
#define  COMPASS_I2C_REG_ADDR_ASAY		    0x61
#define  COMPASS_I2C_REG_ADDR_ASAZ		    0x62

/*
 * 3D accelerometer and 3D gyroscope
 */
 
#define	 I2C_ADDR_GYROSCOPE_LSM6DS3			0x6A

#define  LSM6DS3_I2C_REG_ADDR_FIFO_CTRL1    0x06
#define  LSM6DS3_I2C_REG_ADDR_FIFO_CTRL2    0x07
#define  LSM6DS3_I2C_REG_ADDR_FIFO_CTRL3    0x08
#define  LSM6DS3_I2C_REG_ADDR_FIFO_CTRL4    0x09
#define  LSM6DS3_I2C_REG_ADDR_FIFO_CTRL5    0x0A

#define  LSM6DS3_I2C_REG_ADDR_WIA		    0x0F
#define  LSM6DS3_I2C_REG_ADDR_CTRL1_XL	    0x10
#define  LSM6DS3_I2C_REG_ADDR_CTRL2_G	    0x11
#define  LSM6DS3_I2C_REG_ADDR_CTRL3_C	    0x12
#define  LSM6DS3_I2C_REG_ADDR_CTRL4_C	    0x13
#define  LSM6DS3_I2C_REG_ADDR_CTRL5_C	    0x14
#define  LSM6DS3_I2C_REG_ADDR_CTRL6_C	    0x15
#define  LSM6DS3_I2C_REG_ADDR_CTRL7_G	    0x16
#define  LSM6DS3_I2C_REG_ADDR_CTRL8_XL	    0x17
#define  LSM6DS3_I2C_REG_ADDR_CTRL9_XL	    0x18
#define  LSM6DS3_I2C_REG_ADDR_CTRL10_C	    0x19

#define  LSM6DS3_I2C_REG_ADDR_STATUS	    0x1E

#define  LSM6DS3_I2C_REG_ADDR_OUT_TEMP	    0x20
#define  LSM6DS3_I2C_REG_ADDR_OUTX_G	    0x22
#define  LSM6DS3_I2C_REG_ADDR_OUTY_G	    0x24
#define  LSM6DS3_I2C_REG_ADDR_OUTZ_G	    0x26
#define  LSM6DS3_I2C_REG_ADDR_OUTX_XL	    0x28
#define  LSM6DS3_I2C_REG_ADDR_OUTY_XL	    0x2A
#define  LSM6DS3_I2C_REG_ADDR_OUTZ_XL	    0x2C

/*
 * light
 */
 
#define	 I2C_ADDR_LIGHT_LTR303ALS			0x29

#define  LTR303ALS_I2C_REG_ADDR_ALS_CONTR   0x80
#define  LTR303ALS_I2C_REG_ADDR_ALS_MEAS_RATE  0x85

#define  LTR303ALS_I2C_REG_ADDR_PART_ID	    0x86
#define  LTR303ALS_I2C_REG_ADDR_MANUFAC_ID	0x87

#define  LTR303ALS_I2C_REG_ADDR_ALS_DATA_CH1_0  0x88
#define  LTR303ALS_I2C_REG_ADDR_ALS_DATA_CH1_L  0x88
#define  LTR303ALS_I2C_REG_ADDR_ALS_DATA_CH1_H  0x89
#define  LTR303ALS_I2C_REG_ADDR_ALS_DATA_CH0_L	0x8A
#define  LTR303ALS_I2C_REG_ADDR_ALS_DATA_CH0_H	0x8B

#define  LTR303ALS_I2C_REG_ADDR_ALS_STATUS	0x8C
#define  LTR303ALS_I2C_REG_ADDR_INTERRUPT	0x8F

#define  LTR303ALS_I2C_REG_ADDR_ALS_THRES_UP	0x97
#define  LTR303ALS_I2C_REG_ADDR_ALS_THRES_UP_0	0x97
#define  LTR303ALS_I2C_REG_ADDR_ALS_THRES_UP_1	0x98

#define  LTR303ALS_I2C_REG_ADDR_ALS_THRES_LOW	0x99
#define  LTR303ALS_I2C_REG_ADDR_ALS_THRES_LOW_0	0x99
#define  LTR303ALS_I2C_REG_ADDR_ALS_THRES_LOW_1	0x9A

#define  LTR303ALS_I2C_REG_ADDR_INTERRUPT_PERSIST	0x9E

uint8_t read_sensor_reg8(qapi_I2CM_Config_t *pI2C_config, uint8 reg_addr_val);
uint8_t write_sensor_reg8(qapi_I2CM_Config_t *pI2C_config, uint8 reg_addr_val, uint8_t byte_val);

uint16_t humidity_read_sensor_reg16(qapi_I2CM_Config_t *pI2C_config, uint8 reg_addr_val);
uint32_t sensors_light_LTR303ALS_read_sensor_bits32(qapi_I2CM_Config_t *pI2C_config, uint8 reg_addr_val);

#endif
