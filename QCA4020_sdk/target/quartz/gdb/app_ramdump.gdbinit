# Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
# All Rights Reserved.
# Confidential and Proprietary - Qualcomm Technologies, Inc.
# $QTI_LICENSE_QDN_SH$

# gdb commands to help debug M4 using a Quartz RAM dump
#
# Note: Requires a version of gdb which
#    -is built for ARM CPUs
#    -includes python support
#    -includes "Component: ARM Compiler" in symtab.c::arm_idents
#
# Linux startup:
#    arm-none-eabi-gdb -x $SDK/quartz/gdb/app_ramdump.gdbinit Quartz.elf [or other app]
#
# Windows startup:
#    arm-none-eabi-gdb -x %SDK%/quartz/gdb/app_ramdump.gdbinit Quartz.elf [or other app]
#    Example SDK setting: set SDK=c:/foo/bar/
#    Note: You must start from a cmd.exe window; do not use cygwin!
#    Note: If the gdb package is installed somewhere other than c:\gdb,
#          add "--data-directory=/path/to/gdb"
#
# Currently, this script expects the caller to organize files (or links to files)
# as follows:
#    SRAM.BIN (M4 RAM - ALM dump) in the local directory
#    XIP.BIN  (XiP dump) in the local directory
#    Quartz.elf   [or other application running at the time of ramdump]
#    $SDK/bin/cortex-m4/IOE_ROM_IPT_IMG_ARNNRI.elf (ROM contents, if available)
#    $SDK/bin/cortex-m4/IOE_ROM_IPT_IMG_ARNNRI_patched_syms.elf (ROM symbols, if available)
#    $SDK/quartz/gdb/dumpserver.py (from quartz/gdb)
#    $SDK/quartz/gdb/gdb.threadx or other useful debug scripts (from quartz/gdb)
#
# This script automatically starts dumpserver.py, which is used to access
# contents of SRAM.BIN and XIP.BIN.
#

# TCP port for dumpserver to use.
# You may change this to any suitable unused TCP port.
set $dumpserver_port=2331

python
import os
import posixpath
import subprocess

SDK = os.getenv("SDK")
if not SDK:
  print "You must set the SDK environment variable"
  gdb.execute("quit")
end

set history filename gdb_history.app.log
set history save on
set print pretty on
set pagination off
set output-radix 16

# For ramdumps that are a result of a catastrophic exception:
define coreregs
  if (coredump != 0)
    # Show M4's General Purpose Registers at the time of dump
    echo \n\nM4 coredump area:\n
    p *coredump

    set $r0=coredump->arch->regs->name->exception_r0
    set $r1=coredump->arch->regs->name->exception_r1
    set $r2=coredump->arch->regs->name->exception_r2
    set $r3=coredump->arch->regs->name->exception_r3
    set $r4=coredump->arch->regs->name->regs[4]
    set $r5=coredump->arch->regs->name->regs[5]
    set $r6=coredump->arch->regs->name->regs[6]
    set $r7=coredump->arch->regs->name->regs[7]
    set $r8=coredump->arch->regs->name->regs[8]
    set $r9=coredump->arch->regs->name->regs[9]
    set $r10=coredump->arch->regs->name->regs[10]
    set $r11=coredump->arch->regs->name->regs[11]
    set $r12=coredump->arch->regs->name->exception_r12
    set $sp=coredump->arch->regs->name->psp+0x20
    set $lr=coredump->arch->regs->name->exception_lr
    set $pc=coredump->arch->regs->name->exception_pc
  end
end

# START dumpserver on port $dumpserver_port
# You may remove these lines and start dumpserver.py manually if you prefer.
python
dumpserver = posixpath.join(SDK, "quartz", "gdb", "dumpserver.py")
dmpport = str(int(gdb.parse_and_eval("$dumpserver_port")))
subprocess.Popen(["python", dumpserver, "--port", dmpport])
end

echo \nAttach gdb to dumpserver.\n
python gdb.execute("target remote localhost:" + str(dmpport))

# Load ROM content
python
rom_elf = posixpath.join(SDK, "bin", "cortex-m4", "IOE_ROM_IPT_IMG_ARNNRI.elf")
try:
  gdb.execute('exec-file ' + rom_elf)
except:
  print "WARNING: Cannot load AppSS ROM content from " + rom_elf
end
set trust-readonly-sections on

# Load patched ROM symbols
python
rom_patched_syms = posixpath.join(SDK, "bin", "cortex-m4", "IOE_ROM_IPT_IMG_ARNNRI_patched_syms.elf")
try:
  gdb.execute('add-symbol-file ' + rom_patched_syms + ' 0')
except:
  print "WARNING: Cannot load patched AppSS ROM symbols from " + rom_patched_syms
end

# Load ROM symbols
python
try:
  gdb.execute('add-symbol-file ' + rom_elf + ' 0')
except:
  print "WARNING: Cannot load AppSS ROM symbols from " + rom_elf
end

# Set registers according to coredump and display coredump info
coreregs

# Probe to see if we are using ThreadX or FreeRTOS or something else
set $rtos=0
python
if gdb.lookup_global_symbol("_tx_thread_current_ptr"):
  gdb.execute('set $rtos="threadx"')
if gdb.lookup_global_symbol("pxCurrentTCB"):
  gdb.execute('set $rtos="freertos"')
end

if ! $_streq($rtos, "") 
    echo Load RTOS-specific macros....\n
    python rtosmacros = "gdb." + str(gdb.parse_and_eval("$rtos")).strip('"')
    python rtosmacros = posixpath.join(SDK, "quartz", "gdb", rtosmacros)
    python gdb.execute("source " + rtosmacros)
end
