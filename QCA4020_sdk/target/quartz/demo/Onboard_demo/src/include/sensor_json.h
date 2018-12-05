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

#ifndef _SENSOR_H_
#define _SENSOR_H_

struct thermo_stat {
    uint8_t actual;
    uint8_t desired;
    uint8_t threshhold;
    uint8_t op_mode;
    uint8_t op_state;
};
enum {
    THERMO_STAT_OFF,
    THERMO_STAT_HEATER,
    THERMO_STAT_AC,
    THERMO_STAT_AUTO,
};


typedef enum {                                                                  
    SENSOR_TEMPERATURE = 1,                                                     
    SENSOR_HUMIDITY,                                                            
    SENSOR_PRESSURE,                                                            
    SENSOR_LIGHT,                                                               
    SENSOR_GYROSCOPE,                                                           
    SENSOR_ACCELROMETER,                                                        
    SENSOR_COMPASS,       
    AMBIENT_LIGHT,                                                     
    THERMO_STAT,
    DIMMER_LIGHT,
} SENSOR_TYPE;                                                                  

typedef struct temp_sense {                                                     
    uint32_t mantissa;
    uint32_t exponent;                                                                   
} temp_sensor_t;                                                                

typedef struct humidity_sense {
    uint32_t mantissa;
    uint32_t exponent;
} hum_sensor_t;

typedef struct light_sense {
    uint32_t val;
} light_sensor_t;

typedef struct ambient_light {
    uint32_t val;
} light_t;

typedef struct dimmer_light {
    uint32_t val;
} dimmer_t;

typedef struct pressure_sense {
    float val;
} pressure_sensor_t;

typedef struct compass_sense {
    int32_t x;
    int32_t y;
    int32_t z;
} compass_sensor_t;

typedef struct gyro_sense {
    float x_g;
    float y_g;
    float z_g;
} gyro_sensor_t;

typedef struct accelrometer {
    float x_xl;
    float y_xl;
    float z_xl;
    
} accelero_sensor_t;

typedef struct sensor_info {                                                    

    SENSOR_TYPE sensor_type;                                                     
    union {                                                                     
        temp_sensor_t       temp;                                                     
        hum_sensor_t        hum; 
        light_sensor_t      lux;
        pressure_sensor_t   pressure;
        compass_sensor_t    compass;
        gyro_sensor_t       gyro_val;
        accelero_sensor_t   acc_val;
        light_t             light;
        struct thermo_stat  thermostat;
        dimmer_t            dimmer;

    } s;                                                                        
} sensor_info_t; 

#define BREACHED "{\"Breached\""

int32_t Randomize_thermo_stat_result(sensor_info_t *);
int32_t breach_thermo_stat_result(sensor_info_t *);
int32_t Notify_breach_update_from_remote_device(char *);
int32_t fill_thermo_stat_result(sensor_info_t *);
int32_t Intial_sensor_values(void);
int32_t Get_thermostat_threshhold_values(char *, char *);
int32_t fill_breach_message(char *, char *, uint32_t);
int32_t process_light_localdevice(char *);
void pir_register_intr();
int32_t parse_recived_data(char *jsonbuf);
int32_t Send_data_to_joiner(char *Device_name, char *jsonbuf);
//int Process_light(char *board_name, char *val);
int32_t Update_remote_data_to_aws(char *json_buf);
int32_t Update_json(char *buf, uint32_t size);
void Update_dimmer_value(int32_t val);
void extract_device_name(char *json, char *device_name);
#endif 
