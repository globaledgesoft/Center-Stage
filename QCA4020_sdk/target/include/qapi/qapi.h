/* 
 * Copyright (c) 2011-2018 Qualcomm Technologies, Inc.
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

#ifndef __QAPI_H__ // [
#define __QAPI_H__

/**
 * @file qapi.h
 *
 * @brief QAPI interface definition
 *
 * @details This file provides the base type definitions used by the QAPI.
 *          This includes the basic integer types (based on stdint.h and
 *          stddef.h) and a basic boolean type.
 */

/*-------------------------------------------------------------------------
 * Include Files
 * ----------------------------------------------------------------------*/
#include "qapi_types.h"
#include "qapi_ver.h"
#include "qapi_status.h"

#ifdef QAPI_USE_LPW // [
  #include "qapi_lpw.h"
#endif // ] ifdef QAPI_USE_LPW

#ifdef QAPI_USE_BLE // [
  #include "qapi_ble.h"
#endif // ] ifdef QAPI_USE_BLE

#ifdef QAPI_USE_WLAN // [
  #include "qapi_wlan.h"
#endif // ] ifdef QAPI_USE_WLAN

#ifdef QAPI_USE_LPW // [
  #include "qapi_lpw.h"
#endif // ] ifdef QAPI_USE_LPW

#ifdef QAPI_USE_I2C_MASTER // [
  #include "qapi_i2c_master.h"
#endif // ] ifdef QAPI_USE_I2C_MASTER

#ifdef QAPI_USE_SPI_MASTER // [
  #include "qapi_spi_master.h"
#endif // ] ifdef QAPI_USE_SPI_MASTER

#ifdef QAPI_USE_PWM // [
  #include "qapi_pwm.h"
#endif // ] ifdef QAPI_USE_PWM

#ifdef QAPI_USE_TLMM // [
  #include "qapi_tlmm.h"
#endif // ] ifdef QAPI_USE_TLMM

#ifdef QAPI_USE_GPIOINT // [
  #include "qapi_gpioint.h"
#endif // ] ifdef QAPI_USE_GPIOINT

#ifdef QAPI_USE_ERR_SERVICES // [
  #include "qapi_fatal_err.h"
#endif // ] ifdef QAPI_USE_ERR_SERVICES

#ifdef QAPI_USE_DIAG_CLI // [
  #include "qapi_diag_cli.h"
#endif // ] ifdef QAPI_USE_DIAG_CLI

#ifdef QAPI_USE_DIAG_MSG // [
  #include "qapi_diag_msg.h"
#endif // ] ifdef QAPI_USE_DIAG_MSG

#ifdef QAPI_USE_TSENS // [
  #include "qapi_tsens.h"
#endif // ] ifdef QAPI_USE_TSENS

#ifdef QAPI_USE_ADC // [
  #include "qapi_adc.h"
#endif // ] ifdef QAPI_USE_ADC

#ifdef QAPI_USE_UART // [
  #include "qapi_uart.h"
#endif // ] ifdef QAPI_USE_UART

#ifdef QAPI_USE_KPD // [
  #include "qapi_kpd.h"
#endif // ] ifdef QAPI_USE_KPD

//#define QAPI_USE_PMU
#ifdef QAPI_USE_PMU // [
  #include "qapi_pmu.h"
#endif // ] ifdef QAPI_USE_PMU

//#define QAPI_USE_CLK
#ifdef QAPI_USE_CLK // [
  #include "qapi_clk.h"
#endif // ] ifdef QAPI_USE_CLK

#ifdef QAPI_USE_OMSM
  #include "qapi_om_smem.h"
#endif

#ifdef QAPI_USE_OMTM // [
  #include "qapi_omtm.h"
#endif // ] ifdef QAPI_USE_OMTM

#ifdef QAPI_USE_SPM // [
  #include "qapi_spm.h"
#endif // ] ifdef QAPI_USE_SPM

#ifdef QAPI_USE_SLP // [
  #include "qapi_slp.h"
#endif // ] ifdef QAPI_USE_SLP

#ifdef QAPI_USE_FILESYSTEM // [
  #include "qapi_fs.h"
#endif // ] ifdef QAPI_USE_FILESYSTEM

#ifdef QAPI_USE_HEAP // [
  #include "qapi_heap.h"
#endif // ] ifdef QAPI_USE_HEAP

#ifdef QAPI_USE_MOM // [
  #include "qapi_mom.h"
#endif // ] ifdef QAPI_USE_MOM

#ifdef QAPI_USE_TIMER // [
  #include "qapi_timer.h"  
#endif // ] ifdef QAPI_USE_TIMER

/*-------------------------------------------------------------------------
 * Function Declarations and Documentation
 * ----------------------------------------------------------------------*/

/* Function Definitions to be added later */

#endif // ] #ifndef __QAPI_H__
