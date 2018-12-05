# Copyright (c) 2016-2018 Qualcomm Technologies, Inc.
# All Rights Reserved.
# Confidential and Proprietary - Qualcomm Technologies, Inc.
# $QTI_LICENSE_QDN_SH$

#
# Copyright (c) 2017 Qualcomm Technologies, Inc.
# All Rights Reserved.
# Confidential and Proprietary - Qualcomm Technologies, Inc.
#

# ONE-TIME when starting gdb

target remote localhost:2331
#target extended-remote ubuntu-dsl04:3333
monitor speed auto
monitor endian little

set history filename output/gdb_history.log
set history save on
set print pretty on
set print object on
set print vtbl on
set pagination off
set output-radix 16
monitor sleep 1000
#set *(int*)0x50742094=4

#Set PC
set $pc=0x44

b *0x3d70
command
#This cookie will force PBL into EDL mode
set *(int*)0x10000054=0x06D73E09
c
end

#At this address, USB is enumerated
b *0x3cbc
command
quit
end
c
