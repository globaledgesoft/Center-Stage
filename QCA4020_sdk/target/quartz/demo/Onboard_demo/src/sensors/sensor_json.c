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

/**
 * @file sensor_json.c 
 * @brief File contains the logic to build the sensor_json message
 * according to the json format 
 *
 */
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "qcli_api.h"
#include "qurt_types.h"
#include "aws_util.h"
#include "util.h"
#include <qapi_status.h>
#include <qapi_fs.h>
#include "log_util.h"
#include "led_utils.h"
#include "onboard.h"
#include "jsmn.h"
#include "sensor_json.h"

/*-------------------------------------------------------------------------
  - Preprocessor Definitions and Constants
  ------------------------------------------------------------------------*/
#ifndef OFFLINE
#define TEMP_SENSOR               "\"temperature\""
#define HUMIDITY_SENSOR            "\"humidity\""
#define LIGHT_SENSOR               "\"ambient\""
#define PRESSURE_SENSOR           "\"pressure\""
#define COMPASS_SENSOR            "\"compass\""
#define GYROSCOPE_SENSOR           "\"gyroscope\""
#define ACCELEROMETER_SENSOR       "\"accelerometer\""
#define LIGHT                      "\"light\""
#define THERMOSTAT                "\"thermostat\""
#define DIMMER                    "\"dimmer\""

#else
#define TEMP_SENSOR               "\"temp\""
#define HUMIDITY_SENSOR            "\"hum\""
#define LIGHT_SENSOR               "\"amb\""
#define PRESSURE_SENSOR           "\"pres\""
#define COMPASS_SENSOR            "\"comp\""
#define GYROSCOPE_SENSOR           "\"gyro\""
#define ACCELEROMETER_SENSOR       "\"accel\""
#define LIGHT                      "\"light\""
#define THERMOSTAT                 "\"thermo\""
#define DIMMER                     "\"dimm\""

#endif

#define THERMO_STAT_FILE        "/spinor/sensors/thermostat.txt"

#define VALIDATE_AND_RETURN(js_buf, ret_val)  \
{ \
    if (ret_val < 0) \
    { \
        IOT_ERROR("%s:%d JSON BUF error\n", __func__, __LINE__); \
        return FAILURE; \
    } \
    if (strlen(js_buf) >= MAX_LENGTH_OF_UPDATE_JSON_BUFFER) \
    { \
        IOT_ERROR("%s:%d JSON BUF overun\n", __func__, __LINE__); \
        return FAILURE; \
    } \
}


#define LIGHT_ID          "light_id_1"
#define THRMSTAT          "thermostat"
#define DIMMER_ID         "dimmer_id_1"

int32_t read_onboard_sensors(char *json_buffer, sensor_info_t *sensor_val, uint32_t flag);
int32_t Read_remote_devices_data(char *json_buf, uint32_t size);
int32_t Notify_sensors_update_from_remote_device(char *buf);
int32_t write_thermo_val(struct thermo_stat *sens);
int32_t read_thermo_val(struct thermo_stat *thermo);
int32_t parse_zigbee_data(char *device_name, char *jsonbuf);
struct thermo_stat thermostat;

enum {
    OFF,
    ON,
    POWER_SAVE,
    STANDBY

};
light_t light_state;
dimmer_t dim_val;

/*-------------------------------------------------------------------------
  - Functions
  ------------------------------------------------------------------------*/
/**
 * @func  : Update_dimmer_value 
 * @breif : updates the dimmer value 
 */
void Update_dimmer_value(int32_t val)
{
    dim_val.val = val;
}

/**
 * @func  : process_light_localdevice 
 * @breif : process the light state takes action on/off  
 */
int32_t process_light_localdevice(char *val)
{
    int32_t rc = 0;
    if (!strcmp(val,"1"))
    {

        rc = BLUE_LED_CONFIG(5,100); 
        if(rc == SUCCESS)
        {
            light_state.val = 1; 
        } 
    }
    else
    {
        rc = BLUE_LED_CONFIG(5,0); 
        if(rc == SUCCESS)
        {
            light_state.val = 0; 
        } 
    }
    return rc;
}

/**
 * @func  : Get_thermostat_threshhold_values 
 * @breif : Gets the thermostat threshhold values and sets the thremostat values  
 */
int32_t Get_thermostat_threshhold_values(char *key, char *val)
{
    static int token_count = 0;

    if (!strcmp(key, "desired"))
    {
        thermostat.desired = atoi(val);
        token_count++;
        write_thermo_val(&thermostat);
    }
    else if (!strcmp(key, "threshold"))
    {
        thermostat.threshhold = atoi(val);
        token_count++;
        write_thermo_val(&thermostat);
    }
    else if (!strcmp(key, "op_mode"))
    {
        if (!strcmp(val, "AUTO"))
            thermostat.op_mode = THERMO_STAT_AUTO;
        else if (!strcmp(val, "AC"))
            thermostat.op_mode = THERMO_STAT_AC;
        else if (!strcmp(val, "HEATER"))
            thermostat.op_mode = THERMO_STAT_HEATER;
        else if (!strcmp(val, "OFF"))
            thermostat.op_mode = THERMO_STAT_OFF;
        token_count++;
        write_thermo_val(&thermostat);
    }

    if (token_count == 3)
    {
        token_count = 0;
        return 3;
    }
    return 0;
}

/**
 * @func  : Send_Remote_Device_update_to_aws  
 * @breif : sends the remote devices data to the aws server 
 */
int32_t Send_Remote_Device_update_to_aws(char *buf)
{
    int32_t rc = 0;
    IOT_INFO("\n_-----------Remote_buffer-------\n");
    IOT_INFO("%s\n", buf);
    if (is_aws_running())
    {
        if (!strncmp(buf, BREACHED,strlen(BREACHED)))
        {
            rc = Notify_breach_update_from_remote_device(buf+strlen(BREACHED));
        }
        else
        {
           Notify_sensors_update_from_remote_device(buf);
        }
    }
    return rc;
}

/**
 * @func  : update_thermostat_states  
 * @breif : update the thermostat states to the json 
 */
int32_t update_thermostat_states(char **op_mode, char **state)
{

    if (thermostat.op_mode == THERMO_STAT_AUTO)
    {
        *op_mode = "\"AUTO\"";
        if (thermostat.actual < thermostat.desired)
            *state = "\"HEATER_ON\"";
        else if (thermostat.actual > thermostat.desired)
            *state = "\"AC_ON\"";
        else
            *state = "\"STANDBY\"";
    }
    else if ( thermostat.op_mode == THERMO_STAT_AC)
    {
        *op_mode = "\"AC\"";
        if (thermostat.actual == thermostat.desired)
            *state = "\"STANDBY\"";
        else
            *state = "\"AC_ON\"";

    }
    else if (thermostat.op_mode == THERMO_STAT_HEATER)
    {
        *op_mode = "\"HEATER\"";
        if (thermostat.actual < thermostat.desired)
            *state = "\"HEATER_ON\"";
        else if (thermostat.actual > thermostat.desired)
            *state = "\"HEATER_OFF\"";
        else
            *state = "\"POWER_SAVE\"";

    }
    else if (thermostat.op_mode == THERMO_STAT_OFF)
    {
        *op_mode = "\"OFF\"";
        *state = "\"OFF\"";
    }
    return 0;
}

/**
 * @func  : Update_json  
 * @breif : updates the constructed json from the device 
 */
int32_t Update_json(char *JsonDocumentBuffer, uint32_t Max_size_aws_buf)
{
    uint32_t rem_size;
    int32_t ret_val;
    sensor_info_t sensor_data;
    uint32_t flag = 2;

    if (is_zigbee_onboarded())
    {
        if (zigbee_mode() == 'c' || zigbee_mode() == 'C')
        {
#ifdef AWS_IOT
            flag |= 1;
#endif
        }        
        if (zigbee_mode() == 'e' || zigbee_mode() == 'E')
        {
#if OFFLINE
            flag = 0;
#endif

        }
    }
    if (is_thread_onboarded())
    {
        if (thread_mode() == 'b' || thread_mode() == 'B')
        {
#ifdef AWS_IOT
            flag |= 1;
#endif
        }
    }
    rem_size = Max_size_aws_buf - strlen(JsonDocumentBuffer);
#ifndef OFFLINE
    ret_val = snprintf(JsonDocumentBuffer, rem_size, "{\"state\":{\"reported\":{");
#else
    ret_val = snprintf(JsonDocumentBuffer, rem_size, "{\"reported\":{");
#endif
    VALIDATE_AND_RETURN(JsonDocumentBuffer, ret_val);

    rem_size = Max_size_aws_buf - strlen(JsonDocumentBuffer);

    if (SUCCESS != get_localdevice_name(JsonDocumentBuffer, rem_size))
    {
        IOT_WARN("Mac address is not appended to the Local device name !!!\n");
    }
    rem_size = Max_size_aws_buf - strlen(JsonDocumentBuffer);
    ret_val = snprintf((JsonDocumentBuffer + strlen(JsonDocumentBuffer)), rem_size, ":{");
    VALIDATE_AND_RETURN(JsonDocumentBuffer, ret_val);

    rem_size = Max_size_aws_buf - strlen(JsonDocumentBuffer);
    if (FAILURE == read_onboard_sensors(JsonDocumentBuffer, &sensor_data, flag))
    {
        return FAILURE;
    }


    rem_size = Max_size_aws_buf - strlen(JsonDocumentBuffer);
#ifndef OFFLINE
    ret_val = snprintf((JsonDocumentBuffer + strlen(JsonDocumentBuffer)-1), rem_size, "}}}}");
#else
    ret_val = snprintf((JsonDocumentBuffer + strlen(JsonDocumentBuffer)-1), rem_size, "}}}");
#endif
    VALIDATE_AND_RETURN(JsonDocumentBuffer, ret_val);
    return SUCCESS;
}

/**
 * @func  : add_sensor_entry  
 * @breif : adds the each sensor entry into the json 
 */
int add_sensor_entry(char *json_buf, sensor_info_t *sens_info)
{
    int ret_val;
    int remaining_size;
    char *therm_op = "\"AUTO\"";
    char *therm_state = "\"ON\"";
    remaining_size = MAX_LENGTH_OF_UPDATE_JSON_BUFFER - strlen(json_buf);
    switch (sens_info->sensor_type)
    {
        case SENSOR_TEMPERATURE:
            ret_val = snprintf(json_buf +strlen(json_buf), remaining_size, "%s:{\"temp_id1\":%u.%u},",
                    TEMP_SENSOR, (unsigned int)sens_info->s.temp.mantissa, (unsigned int)sens_info->s.temp.exponent);
            VALIDATE_AND_RETURN(json_buf, ret_val);
            break;

        case SENSOR_HUMIDITY:
            ret_val = snprintf(json_buf + strlen(json_buf), remaining_size,"%s:{\"humidity_id1\":%u.%u},",
                    HUMIDITY_SENSOR, (unsigned int)sens_info->s.hum.mantissa, (unsigned int)sens_info->s.hum.exponent);
            VALIDATE_AND_RETURN(json_buf, ret_val);
            break;
        case SENSOR_LIGHT:
            ret_val = snprintf(json_buf + strlen(json_buf), remaining_size, "%s:{\"light_senor_id1\":%u},",
                    LIGHT_SENSOR, (unsigned int)sens_info->s.lux.val);
            VALIDATE_AND_RETURN(json_buf, ret_val);
            break;
        case SENSOR_PRESSURE:
            ret_val = snprintf(json_buf + strlen(json_buf), remaining_size, "%s:{\"pressure_sensor_id1\":%.02f},",
                    PRESSURE_SENSOR, (float)sens_info->s.pressure.val);
            VALIDATE_AND_RETURN(json_buf, ret_val);
            break;
        case SENSOR_COMPASS:
            ret_val = snprintf(json_buf + strlen(json_buf), remaining_size, "%s:{\"compass_id1\":{\"X\":%d,\"Y\":%d,\"Z\":%d}},",
                    COMPASS_SENSOR, (int) sens_info->s.compass.x, (int) sens_info->s.compass.y, (int) sens_info->s.compass.z);
            VALIDATE_AND_RETURN(json_buf, ret_val);
            break;
        case SENSOR_GYROSCOPE:
            ret_val = snprintf(json_buf + strlen(json_buf), remaining_size, "%s:{\"gyro_id1\":{\"X\":%.02f,\"Y\":%.02f,\"Z\":%.02f}},",
                    GYROSCOPE_SENSOR, (float)sens_info->s.gyro_val.x_g, (float)sens_info->s.gyro_val.y_g, (float)sens_info->s.gyro_val.z_g);
            VALIDATE_AND_RETURN(json_buf, ret_val);
            break;
        case SENSOR_ACCELROMETER:
            ret_val = snprintf(json_buf + strlen(json_buf), remaining_size, "%s:{\"accelerometer_id1\":{\"X\":%.02f,\"Y\":%.02f,\"Z\":%0.02f}},",                      ACCELEROMETER_SENSOR, (float)sens_info->s.acc_val.x_xl, (float) sens_info->s.acc_val.y_xl, (float)sens_info->s.acc_val.z_xl);
            VALIDATE_AND_RETURN(json_buf, ret_val);
            break;
        case AMBIENT_LIGHT:
            sens_info->s.light.val = light_state.val;
            ret_val = snprintf(json_buf + strlen(json_buf), remaining_size, "%s:{\"light_id_1\":%d},",
                    LIGHT, (int) sens_info->s.light.val);
            VALIDATE_AND_RETURN(json_buf, ret_val);
            break;
        case THERMO_STAT:
            sens_info->s.thermostat = thermostat;
            update_thermostat_states(&therm_op, &therm_state);
            ret_val = snprintf(json_buf + strlen(json_buf), remaining_size, "%s:{\"thermostat_id1\":"
                    "{\"actual\":%d,\"op_mode\":%s,\"desired\":%d,\"op_state\":%s,\"threshold\":%d}},",
                    THERMOSTAT, (int) sens_info->s.thermostat.actual, therm_op, (int) sens_info->s.thermostat.desired, therm_state,
                         (int) sens_info->s.thermostat.threshhold);
            VALIDATE_AND_RETURN(json_buf, ret_val);
            break;
        case DIMMER_LIGHT:
            sens_info->s.dimmer.val = dim_val.val;
            ret_val = snprintf(json_buf + strlen(json_buf), remaining_size, "%s:{\"dimmer_id_1\":%d},",
                    DIMMER, (int) sens_info->s.dimmer.val);
            VALIDATE_AND_RETURN(json_buf, ret_val);

            break;
    }
    return 0;
}


/**
 * @func  : fill_breach_message  
 * @breif : fills the breach message 
 */
int32_t fill_breach_message(char *json_buf, char *msg, uint32_t size)
{
    uint32_t rem_size;
    int32_t ret_val;

    rem_size = size - strlen(json_buf);

    if (SUCCESS != get_localdevice_name(json_buf, rem_size))
    {
        IOT_WARN("Mac address is not appended to the Local device name !!!\n");
    }
    rem_size = size - strlen(json_buf);
    ret_val = snprintf((json_buf + strlen(json_buf)), rem_size,
             ":{\"message\": \"%s\"}}}", msg);
    VALIDATE_AND_RETURN(json_buf, ret_val);

    return 0;
}

/**
 * @func  : Intial_sensor_values  
 * @breif : Intiaites the Intial sensor values and get the values from the file 
 */
int32_t Intial_sensor_values(void)
{
    struct qapi_fs_stat_type fstat;

    if (QAPI_OK == qapi_Fs_Stat(THERMO_STAT_FILE, &fstat))
    {
        if (fstat.st_size != 0)
        {
            IOT_INFO("Thermo_stat_values already loaded");
            return read_thermo_val(&thermostat);
        }
    }


    thermostat.actual = 20;
    thermostat.desired = 22;
    thermostat.threshhold = 2;
    thermostat.op_mode = THERMO_STAT_AUTO;

    return write_thermo_val(&thermostat);
}

/**
 * @func  : breach_thermo_stat_result  
 * @breif : genration of thermostat value 
 */
int32_t breach_thermo_stat_result(sensor_info_t *sensor)
{
    int8_t val;

    val = rand() % (2*thermostat.threshhold+1);

    val += (thermostat.desired+thermostat.threshhold);

    thermostat.actual = val;

    return 0;
}

/**
 * @func  : Randomize_thermo_stat_result  
 * @breif : Randomly genrates the thermo stat values
 */
int32_t Randomize_thermo_stat_result(sensor_info_t *sensor)
{
    int8_t val;

    val = rand() % (2*thermostat.threshhold+1);

    val += (thermostat.desired - thermostat.threshhold);

    thermostat.actual = val;
    sensor->s.thermostat.actual = thermostat.actual;
    sensor->s.thermostat.desired = thermostat.desired;
    sensor->s.thermostat.threshhold = thermostat.threshhold;
    sensor->s.thermostat.op_mode    = thermostat.op_mode;

    return 0;

}

/**
 * @func  : write_thermo_val  
 * @breif : writes the thermostat values into file 
 */
int32_t write_thermo_val(struct thermo_stat *thermo)
{
    uint32_t bytes_written = 0;
    int fd;
    int32_t ret;

    ret = qapi_Fs_Open(THERMO_STAT_FILE, QAPI_FS_O_RDWR | QAPI_FS_O_CREAT, &fd);
    if (ret != QAPI_OK)
    {
        LOG_ERROR("File creation is failed !!!!\n");
        return FAILURE;
    }

    qapi_Fs_Write(fd, &(thermo->desired), 1, &bytes_written);

    LOG_VERBOSE("bytes_written num = %d\n", bytes_written);
    if (!bytes_written)
    {
        LOG_ERROR("Failed to store desired temperature\n");
        goto error;
    }
    bytes_written = 0;

    qapi_Fs_Write(fd, &(thermo->threshhold), 1, &bytes_written);
    if (!bytes_written)
    {
        LOG_ERROR("Failed to store desired temperature\n");
        goto error;
    }

    qapi_Fs_Write(fd, &(thermo->op_mode), 1, &bytes_written);
    if (!bytes_written)
    {
        LOG_ERROR("Failed to store desired temperature\n");
        goto error;
    }


    qapi_Fs_Close(fd);
    return 0;

error:
    qapi_Fs_Close(fd);
    return FAILURE;

}

/**
 * @func  : read_thermo_val  
 * @breif : reads the thermostat values into file 
 */
int32_t read_thermo_val(struct thermo_stat *thermo)
{
    uint32_t bytes_read = 0;
    int fd;
    int32_t ret;

    ret = qapi_Fs_Open(THERMO_STAT_FILE, QAPI_FS_O_RDWR | QAPI_FS_O_CREAT, &fd);
    if (ret != QAPI_OK)
    {
        LOG_ERROR("File creation is failed !!!!\n");
        return FAILURE;
    }

    qapi_Fs_Read(fd, &(thermo->desired), 1, &bytes_read);

    LOG_VERBOSE("bytes_read num = %d\n", bytes_read);
    if (!bytes_read)
    {
        LOG_ERROR("Failed to store desired temperature\n");
        goto error;
    }
    bytes_read = 0;

    qapi_Fs_Read(fd, &(thermo->threshhold), 1, &bytes_read);
    if (!bytes_read)
    {
        LOG_ERROR("Failed to store desired temperature\n");
        goto error;
    }


    qapi_Fs_Read(fd, &(thermo->op_mode), 1, &bytes_read);
    if (!bytes_read)
    {
        LOG_ERROR("Failed to store desired temperature\n");
        goto error;
    }

    qapi_Fs_Close(fd);
    return 0;

error:
    qapi_Fs_Close(fd);
    return FAILURE;
}

/**
 * @func  : parse_recived_data  
 * @breif : parse the recived data from co ordinator in thread case 
 */
#ifdef AWS_IOT
char token_name[128];
char sub_token[32];
int32_t parse_recived_data(char *jsonbuf)
{
    int i;
    int rc = -1;
    int tokens;
    jsmn_parser p;
    jsmntok_t t[20]; /* We expect no more than 128 tokens */
    uint32_t dimmer_val = 0;

    jsmn_init(&p);
    tokens = jsmn_parse(&p, jsonbuf, strlen(jsonbuf), t, sizeof(t)/sizeof(t[0]));

    if (tokens < 0)
    {
        LOG_ERROR("Failed to parse json\n");
        return FAILURE;
    }

    LOG_WARN("%s\t %d\n", __func__, __LINE__);
    for (i =1 ; i < tokens; i++)
    {
        if (t[i].parent == 0)
        {
            memset(token_name, 0, sizeof(token_name));
            memcpy(token_name, jsonbuf+t[i].start, t[i].end-t[i].start);
            LOG_INFO(" Device_name: %s\n", token_name);
        }

        if (t[i].parent == 2)
        {
            i = i +2;
            memset(token_name, 0, sizeof(token_name));
            memcpy(token_name, jsonbuf+t[i].start, t[i].end-t[i].start);

            memset(sub_token, 0, sizeof(sub_token));
            memcpy(sub_token, jsonbuf+t[i+1].start, t[i+1].end-t[i+1].start);

            rc = strncmp(DIMMER_ID, token_name, strlen(DIMMER_ID));

            LOG_INFO(" key : %s\t", token_name);
            LOG_INFO(" Val : %s\n", sub_token);
            if (!rc)
            {
                dimmer_val = ((atoi(sub_token))/5);
                LOG_INFO("dimmer_val : %d\n", dimmer_val);
                BLUE_LED_CONFIG(50, dimmer_val);
                Update_dimmer_value(atoi(sub_token));

            }
        }
        i++;
    }
    LOG_WARN("%s\t %d\n", __func__, __LINE__);
    return  SUCCESS;
}
#endif


