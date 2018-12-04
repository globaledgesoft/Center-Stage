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

#ifndef __LED_UTILS_H__
#define __LED_UTILS_H__

#include <qapi_pwm.h>

#ifdef BOARD_SUPPORTS_WIFI
#define LED_WLAN           0
#define LED_BLE            5 
#define LED_R15_4          7
#define LED_RED            6
#define LED_BLUE           1
#define LED_GREEN          2

#define WLAN_LED_CONFIG(f, d)       led_config(LED_WLAN, f, d)
#define BLE_LED_CONFIG(f, d)        led_config(LED_BLE, f, d)
#define R15_4_LED_CONFIG(f, d)      led_config(LED_R15_4, f, d)
#define RED_LED_CONFIG(f, d)        led_config(LED_RED, f, d)
#define BLUE_LED_CONFIG(f, d)       led_config(LED_BLUE, f, d)
#define GREEN_LED_CONFIG(f, d)      led_config(LED_GREEN, f, d)

#else

#define LED_BLE            5
#define LED_R15_4          0
#define LED_RED            6
#define LED_BLUE           1
#define LED_GREEN          2

#define WLAN_LED_CONFIG(f, d)       
#define BLE_LED_CONFIG(f, d)        led_config(LED_BLE, f, d)
#define R15_4_LED_CONFIG(f, d)      led_config(LED_R15_4, f, d)
#define RED_LED_CONFIG(f, d)        led_config(LED_RED, f, d)
#define BLUE_LED_CONFIG(f, d)       led_config(LED_BLUE, f, d)
#define GREEN_LED_CONFIG(f, d)      led_config(LED_GREEN, f, d)
#endif


int32_t led_initialize(void);
int32_t led_config(int32_t led_chan, int32_t freq_hz, int32_t duty_percent);
void led_deinitialize(void);

#endif
