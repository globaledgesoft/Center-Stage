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
#include <qapi.h>
#include <qapi/qapi_status.h>

#include "led_utils.h"

enum led_status
{
    LED_ERROR = -1,
    LED_SUCCESS = 0
};

typedef struct led_info {
    qapi_PWM_Handle_t hled;
    int32_t freq;
    int32_t duty_cycle;
} led_Info_t;

#if BOARD_SUPPORTS_WIFI
static led_Info_t wlan_led;
#endif
static led_Info_t ble_led;
static led_Info_t r15_4_led;
static led_Info_t red_led;
static led_Info_t blue_led;
static led_Info_t green_led;

/**
 * @func : led_open
 * @Desc : opens the channel and returns the
 *         handle to the respective channel
 */
static int32_t led_open(int32_t led_chan, qapi_PWM_Handle_t *handle)
{
    if (QAPI_OK != qapi_PWM_Channel_Open(led_chan, handle))
        return LED_ERROR;
    return LED_SUCCESS;
}

static int32_t config(qapi_PWM_Handle_t handle, int32_t freq, int32_t duty_cycle)
{
    qapi_PWM_Handle_t  handleArray[2] = {NULL, NULL};

    /* Default Configuring the PWM channel */
    qapi_PWM_Config_t pwm_config = {
        7500,
        2000,
        0,
        0,
        QAPI_PWM_SOURCE_CLK_ECON_MODE_E,
    };

    if (NULL == handle)
    {
        return LED_ERROR;
    }

    pwm_config.freq = freq;
    pwm_config.duty = duty_cycle;
    handleArray[0] = handle;

    if (freq == 0 || duty_cycle == 0)
    {
        if (QAPI_OK != qapi_PWM_Enable(handleArray, 1, 0))
            return LED_ERROR;
        return LED_SUCCESS;
    }

    if (QAPI_OK != qapi_PWM_Channel_Set(handle, &pwm_config))
        return LED_ERROR;

    if (QAPI_OK != qapi_PWM_Enable(handleArray, 1, 1))
        return LED_ERROR;

    return LED_SUCCESS;
}

static void sync_led_states(void)
{
#if BOARD_SUPPORTS_WIFI
    config(wlan_led.hled, 0, 0);
#endif
    config(ble_led.hled, 0, 0);
    config(r15_4_led.hled, 0, 0);
    config(red_led.hled, 0, 0);
    config(blue_led.hled, 0, 0);
    config(green_led.hled, 0, 0);

#if BOARD_SUPPORTS_WIFI
    config(wlan_led.hled, wlan_led.freq, wlan_led.duty_cycle);
#endif
    config(ble_led.hled, ble_led.freq, ble_led.duty_cycle);
    config(r15_4_led.hled, r15_4_led.freq, r15_4_led.duty_cycle);
    config(red_led.hled, red_led.freq, red_led.duty_cycle);
    config(blue_led.hled, blue_led.freq, blue_led.duty_cycle);
    config(green_led.hled, green_led.freq, green_led.duty_cycle);
}

/**
 * @func : led_config
 * @Desc : function configures the respective channel with
 *         frequency and duty cycle value to control the LED's
 */
int32_t led_config(int32_t led, int32_t freq_hz, int32_t duty_percent)
{
    led_Info_t *pled;

    switch (led)
    {
#if BOARD_SUPPORTS_WIFI
        case LED_WLAN:
            pled = &wlan_led;
            break;
#endif
        case LED_BLE:
            pled = &ble_led;
            break;
        case LED_R15_4:
            pled = &r15_4_led;
            break;
        case LED_RED:
            pled = &red_led;
            break;
        case LED_BLUE:
            pled = &blue_led;
            break;
        case LED_GREEN:
            pled = &green_led;
            break;
        default:
            return LED_ERROR;
    }

    pled->freq = freq_hz * 100;
    pled->duty_cycle = duty_percent * 100;

    if (duty_percent == 0 || duty_percent == 100)
    {
        return config(pled->hled, pled->freq, pled->duty_cycle);
    }

    sync_led_states();

    return LED_SUCCESS;
}

/**
 * @func : led_initialize
 * @Desc : opens the channel and returns the
 *         handle to the respective channel
 */
int32_t led_initialize(void)
{
#if BOARD_SUPPORTS_WIFI
    (void)led_open(LED_WLAN,  &wlan_led.hled);
#endif
    (void)led_open(LED_BLE,   &ble_led.hled);
    (void)led_open(LED_R15_4, &r15_4_led.hled);
    (void)led_open(LED_RED,   &red_led.hled);
    (void)led_open(LED_BLUE,  &blue_led.hled);
    (void)led_open(LED_GREEN, &green_led.hled);

    return LED_SUCCESS;
}


/**
 * @func : led_close
 * @Desc : closes the respective channel handle
 */
void led_deinitialize(void)
{
#if BOARD_SUPPORTS_WIFI
    config(wlan_led.hled, 0, 0);
#endif
    config(ble_led.hled, 0, 0);
    config(r15_4_led.hled, 0, 0);
    config(red_led.hled, 0, 0);
    config(blue_led.hled, 0, 0);
    config(green_led.hled, 0, 0);

#if BOARD_SUPPORTS_WIFI
    qapi_PWM_Channel_Close(wlan_led.hled);
#endif
    qapi_PWM_Channel_Close(ble_led.hled);
    qapi_PWM_Channel_Close(r15_4_led.hled);
    qapi_PWM_Channel_Close(red_led.hled);
    qapi_PWM_Channel_Close(blue_led.hled);
    qapi_PWM_Channel_Close(green_led.hled);

    return;
}
