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

typedef struct _humidity_val {
	uint8_t   who_am_i;
	uint8_t   av_conf;
	uint8_t   ctrl_reg1;
	uint8_t   ctrl_reg2;
	uint8_t   ctrl_reg3;
	uint8_t   status_reg;
	
	uint16_t  humidity_T_OUT;
	uint16_t  humidity_T0_OUT;
	uint16_t  humidity_T1_OUT;
	uint16_t  humidity_T0_rHx2;
	uint16_t  humidity_T1_rHx2;

	uint16_t  temp_T_OUT;
	uint16_t  temp_T0_OUT;
	uint16_t  temp_T1_OUT;
	uint16_t  temp_T0_DegCx8;
	uint16_t  temp_T1_DegCx8;
} humidity_val_t;

typedef struct _pressure_val {
	uint8_t   id;
	uint8_t   status_reg;
	uint8_t   ctrl_meas;
	
	uint32_t  press;
	uint32_t  temp;
} pressure_val_t;

typedef struct _compass_val {
	uint8_t   company_id;
	uint8_t   device_id;
	uint8_t   status_reg;
	uint8_t   ctrl2;
	
	int16_t   hx;
	int16_t   hy;
	int16_t   hz;
} compass_val_t;

typedef struct _gyroscope_val {
	uint8_t   who_am_i;
	uint8_t   device_id;
	uint8_t   status_reg;
	
	uint32_t  press;
	uint32_t  temp;
} gyroscope_val_t;

typedef struct _light_val {
	uint8_t   part_id;
	uint8_t   manufactory_id;
	uint8_t   control_reg;
	uint8_t   status_reg;
	uint16_t  ch1_val, ch0_val;
} light_val_t;

typedef struct  sensors_val {
	humidity_val_t  humidity;
	pressure_val_t  pressure;
	compass_val_t   compass;
	gyroscope_val_t gyroscope;
	light_val_t     light;	
} sensors_val_t;

extern sensors_val_t	sensor_vals;

/**
   @brief This function registers the SENSORS demo commands with QCLI.
*/

void Initialize_Sensors_Demo(void);

