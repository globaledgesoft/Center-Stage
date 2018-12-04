# Copyright (c) 2016-2018 Qualcomm Technologies, Inc.
# All Rights Reserved.
# Confidential and Proprietary - Qualcomm Technologies, Inc.
# $QTI_LICENSE_QDN_SH$


define runm0
monitor jtagconf 0 0
monitor sleep 100

#turn on all JTAG TAPs
#set *((int*)0x50310000)=0x7

#Load IPT version of RAM symbols
symbol-file ../../../../../bin/cortex-m0/threadx/IOE_RAM_IPT_IMG_BRNTRI.elf


add-symbol-file ../../../../../bin/cortex-m0/threadx/IOE_ROM_IPT_IMG_BRNTRI_patched_syms.elf 0x0000C000 


b jettison_core

b platform_init

end



# ONE-TIME when starting gdb
target extended-remote localhost:2330
monitor speed auto
monitor endian little

define hookpost-stepi
x/1i $pc
end
set history filename output/gdb_history_m0.log
set history save on
set print pretty on
set print object on
set print vtbl on
set pagination off
set output-radix 16
set disassemble-next-line on
source v1/v1_substitute_path_m0.gdb
set $COMBINED_BIN=0
runm0
