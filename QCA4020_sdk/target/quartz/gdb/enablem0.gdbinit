#===============================================================================
# Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
# All Rights Reserved.
# Confidential and Proprietary - Qualcomm Technologies, Inc.
# $QTI_LICENSE_QDN_SH$

# ONE-TIME when starting gdb

target remote localhost:2331
monitor speed auto
monitor endian little

set history filename output/gdb_history.log
set history save on
set print pretty on
set print object on
set print vtbl on
set pagination off
set output-radix 16
monitor sleep 2000

#Enable M0 JTAG
set *((int*)0x50310000)=0x7
monitor sleep 3000
q
