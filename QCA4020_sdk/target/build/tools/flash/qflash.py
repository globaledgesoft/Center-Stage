#!/usr/bin/python

######################################################################################
# Copyright (c) 2018 Qualcomm Technologies, Inc.
# All Rights Reserved.
# Copyright (c) 2018 Qualcomm Technologies, Inc.
# All rights reserved.
# Redistribution and use in source and binary forms, with or without modification, are permitted (subject to the limitations in the disclaimer below)
# provided that the following conditions are met:
# Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
# Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
# Neither the name of Qualcomm Technologies, Inc. nor the names of its contributors may be used to endorse or promote products derived
# from this software without specific prior written permission.
# NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE.
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
# BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
# OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
# EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

######################################################################################
# Quartz SPI NOR flash programmer
# This wrapper script uses various sub-tools in order to satisfy the most
# COMMON flash programming needs. It employs heuristics in order to simplify
# command-line options. For more complex needs that this tool does not address,
# users may directly invoke gen_part_table.py and/or gen_fwd_table.py,
# modifying the resulting table(s), if desired, and then use "qflash.py --nogen"
# (or directly use QSaharaServer and fh_loader) in order to program flash.
#
# The primary use of this script is
#    In a Windows command window AND
#    With Qualcomm USB drivers pre-installed AND
#    From a firmware application's output or 402x or build directory such as:
#        QCLI_demo\build\gcc\output or
#        QCLI_demo\build\gcc\output\4020 or
#        QCLI_demo\build\gcc or
#        m4agent\build\gcc\output or
#        m4agent\build\gcc\4020 or
#        m4agent\build\gcc
#    With a Quartz device in Emergency Download ("EDL") Mode
#        (so the QDLoader port appears in Device Manager).
#
# Command line options:
#
#    --comm xx or --comm_port xx
#        Use this to specify the COM port number for QDLoader device.
#        Get this number from Device Manager after you start your Quartz
#        board in Emergency Download (EDL) Mode.
#
#    --app path\to\ApplicationImage_HASHED.elf
#    --app path\to\SecureApplicationImage.elf
#        Specifies the paths and names of M4 application images to be programmed to flash.
#        [Typically, this is AUTOMATICALLY derived based on the current folder.]
#
#    --nogen
#        This suppresses generation of the Partition Table and FWD Table.
#            If BOTH files exist, then do not generate either table; use the existing FWD Table to program flash
#            If just the FWD Table exists then do not generate tables; use FWD Table
#            If just the Partition Table exists, then use it to generate the FWD Table.
#            If neither table exists then generate both tables and use the FWD Table to program flash
#        Useful if the tables were manually generated and/or modified.
#
#    --nodev
#        This suppresses interactions with the Quartz device. So it does NOT
#        program flash. Useful if you want to generate the Partition Table and
#        the Firmware Descriptor Table (i.e. so you can make modifications
#        and then re-run qflash.py with the --nogen option).
#
#    --erase	
#		This allows the entire flash to be erased.
#
#    --debug or --verbose
#        This prints extra information about progress and status.
#
# Environment variables:
#    $SDK selects the location where flash tools are installed.
#    If not set, this script attempts to use heuristics to determine SDK 
#
#    $COMM_PORT selects the COM port number (much like --comm=xx)
#
#    $FS1IMG is a file system image to use for the primary file system
#    or the keyword "KEEP", which retains the current flash contents of
#    the primary file system area, or the keyword "ERASE" which erases
#    the current contents of the primary file system area. The default
#    is ERASE.
#
#    $FS1SZ is the size, in KB, to use for the primary file system
#    rather than the default size.
#    Default: FS1SZ=64KB
#
#    $FS2IMG is similar to FS1IMG but for the secondary file system.
#    For FS2IMG, the additional keyword, NONE, indicates that the
#    secondary file system should be omitted.
#
#    $FS2SZ is the size, in KB, to use for the secondary file system
#    rather than the default size. If $FS2SZ is 0 then the secondary
#    file system is omitted. (Equivalent to setting FS2IMG=NONE.)
#    Default: FS2SZ=64KB
#
#    $FLOGSZ is the size, in KB, to reserve partitions for DIAG
#    logging onto Flash.
#    Default: FLOGSZ=0 i.e. no partitions reserved.
#
# Returns 0 on success; non-zero on failure
######################################################################################

import os
import sys
import glob
import re
import subprocess
import argparse
from xml.dom import minidom

# Secure Boot directory. Automatically set if we specify a secured M4 application.
secboot_dir = None

# Flag that tells whether or not this is a FTM (mfg) ImageSet
FTM_image_set = False

qflash_description = \
"""
Use Emergency Download (EDL) Mode to program flash over USB.

#############################################################################

Configurable Environment variables:
    $SDK selects the location where flash tools are installed.
    If not set, this script attempts to use heuristics to determine SDK 

    $COMM_PORT selects the COM port number (much like --comm=xx)

    $FS1IMG is a file system image to use for the primary file system
    or the keyword "KEEP", which retains the current flash contents of
    the primary file system area, or the keyword "ERASE" which erases
    the current contents of the primary file system area. The default
    is ERASE.

    $FS1SZ is the size, in KB, to use for the primary file system
    rather than the default size.
    Default: FS1SZ=64KB

    $FS2IMG is similar to FS1IMG but for the secondary file system.
    For FS2IMG, the additional keyword, NONE, indicates that the
    secondary file system should be omitted.

    $FS2SZ is the size, in KB, to use for the secondary file system
    rather than the default size. If $FS2SZ is 0 then the secondary
    file system is omitted. (Equivalent to setting FS2IMG=NONE.)
    Default: FS2SZ=64KB

    $FLOGSZ is the size, in KB, to reserve partitions for DIAG
    logging onto Flash.
    Default: FLOGSZ=0 i.e. no partitions reserved.

    NOTE: All sizes need to be mutliple of 4KB.

#############################################################################
"""

# Default sizes of primary and secondary file systems
FS1_sizeKB=64 # Application (e.g. QCLI_demo) Primary FS; m4agent Secondary FS
FS2_sizeKB=64 # Application (e.g. QCLI_demo) Secondary FS; m4agent Primary FS

# Default size of partitions reserved for DIAG logging onto Flash
FLOG_sizeKB=0

flash_tool_path = os.path.join("build", "tools", "flash")
script_name = os.path.basename(sys.argv[0])

def INFO(strarg):
    global script_name
    print script_name + " Info: " + strarg

def DEBUG(strarg):
    global want_debug
    if want_debug:
        INFO(strarg)

def FAIL(strarg):
    print script_name + " Error: " + strarg
    sys.exit(1)

def validate_file(path):
    if not os.path.isfile(path):
        FAIL ("Missing file: " + path)

if re.search("cygwin", sys.platform):
    using_cygwin = True
else:
    using_cygwin = False

if re.search("win32", sys.platform):
    using_windows = True
    need_shell = False # Avoid "line too long" bug in Windows when using double-quoted args
else:
    using_windows = False
    need_shell = True

# Turn path into a path that is useful for this script.
#
# What we start with depends on the execution environment --
# we may be using Windows or Cygwin or Linux (for table regeneration).
#
def fixpath(path):
    global using_cygwin

    # Normalize
    path = os.path.normpath(path)

    if using_cygwin:
        # Eliminate cygwin cruft
        srch = re.search(r'^/cygdrive/(.)/(.*)', path)
        if srch:
            path = srch.group(1) + ':/' + srch.group(2)

        # Try to convert Cygwin-style path to native Windows-style path
        try:
            path = subprocess.check_output("cygpath -w " + '"' + path + '"', shell=True).rstrip('\n')
        except:
            pass

        # Convert slashes to backslashes
        path = path.replace('/', '\\')

        # Change backslashes to double backslashes for use with shell
        path = path.replace('\\', '\\\\')

    return path

# path_to_app interpret an app specification which was specified on the
# command line using "--app xyz".
# 
# Interpretation of an app_spec employs these heuristics which are derived
# from SDK build rules.
# Case1) If app_spec is a file, that is the app. (Most precise specification.)
# Case2) If app_spec is a directory and if there is a single file, *_HASHED.elf,
#        in that directory, then that is the app.
# Case3) If app_spec is a directory and if there is a single file, *.elf,
#        in that directory, then that is the app.
# Case4) If app_spec is a directory and if there is a directory under that
#        which contains an m4 directory which contains a single file, *.elf,
#        then that is the app.
# Case5) If app_spec is a directory and there is no "402?" directory in that
#        directory but there is an "output" directory in that directory
#        which contains a single file, *_HASHED.elf, that is the app.
# If none of the above hold, then print an error message and EXIT qflash.py.
# 
# If any specified app appears to be a secure version (e.g. resides in a 402x/m4
# directory) then the secboot_dir is set. This causes the corresponding
# SECURED versions of M0 and KF images to be used.
#
def path_to_app(app_spec):
    global secboot_dir

    app_spec = os.path.abspath(app_spec)
    if os.path.isfile(app_spec):
        if os.path.basename(os.path.dirname(app_spec)) == "m4":
            secboot_dir = os.path.dirname(os.path.dirname(app_spec))
        return app_spec # Case1

    if os.path.isdir(app_spec):
        img_name = ''.join((glob.glob(os.path.join(app_spec, "*_HASHED.elf"))));
        if img_name and os.path.isfile(img_name):
            return img_name # Case2

        img_name = ''.join((glob.glob(os.path.join(app_spec, "*.elf"))));
        if img_name and os.path.isfile(img_name):
            if os.path.basename(os.path.dirname(img_name)) == "m4":
                secboot_dir = os.path.dirname(os.path.dirname(img_name))
            return img_name # Case3

        secure_dir = ''.join((glob.glob(os.path.join(app_spec, "m4"))));
        if (os.path.isdir(secure_dir)):
            img_name = ''.join((glob.glob(os.path.join(secure_dir, "*.elf"))));
            if img_name and os.path.isfile(img_name):
                secboot_dir = app_spec
                return img_name # Case4

        secure_dir = ''.join((glob.glob(os.path.join(app_spec, "402?"))))
        if not os.path.isdir(secure_dir):
            unsecured_dir = ''.join((glob.glob(os.path.join(app_spec, "output"))))
            if os.path.isdir(unsecured_dir):
                img_name = ''.join((glob.glob(os.path.join(unsecured_dir, "*_HASHED.elf"))));
                if img_name and os.path.isfile(img_name):
                    return img_name # Case5

    # Error case -- cannot decide what M4 app to program
    FAIL("Cannot determine application for " + str(app_spec) + "\nPlease check --app.")


# Parse command-line options
parser = argparse.ArgumentParser(description=qflash_description,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)

parser.add_argument('--comm', '--comm_port',
                    type=int,
                    required=False,
                    help='Specify QDLoader COM port number. Alternative: set COMM_PORT environment variable.')

parser.add_argument('--app',
                    type=str,
                    required=False,
                    action='append',
                    help='Specify the path and/or name of an Application Image to run on M4')

parser.add_argument('--nogen',
                    required=False,
                    help='Suppress generation of Partition and FWD tables',
                    default=False,
                    action='store_true')

parser.add_argument('--nodev',
                    required=False,
                    help='Do not access device; do not actually program flash',
                    default=False,
                    action='store_true')

parser.add_argument('--erase',
                    required=False,
                    help='erases the entire flash',
                    default=False,
                    action='store_true')

					
parser.add_argument('--debug', '--verbose',
                    required=False,
                    help='Enable debug messages',
                    default=False,
                    action='store_true')

args = parser.parse_args()

# Do this first thing so DEBUG works properly
want_debug=args.debug

want_gen=not args.nogen
DEBUG("want_gen is " + str(want_gen))

want_dev=not args.nodev
DEBUG("want_dev is " + str(want_dev))

do_erase=args.erase
DEBUG("erase is " + str(do_erase))
if do_erase:
	want_gen = False
	DEBUG("Erase want_gen is " + str(want_gen))
	DEBUG("Erase want_dev is " + str(want_dev))


if want_dev and ( not using_windows ) and (not using_cygwin ):
    FAIL("Cannot program flash on this platform. Use --nodev?")

# If there is an "output" folder in the current folder, use it.
if not os.path.isdir("output"):
    # If we are already in an "output" folder, then pop up one level
    curr_dir = os.getcwd()
    if os.path.basename(curr_dir) == "output":
        os.chdir("..")
    else:
        FAIL("Missing output folder. Something seems wrong.")

DEBUG("Executing in " + os.getcwd())

if args.comm is not None:
    comm_port=str(args.comm)
    DEBUG("comm_port is " + comm_port)
else:
    comm_port=""

DEBUG("script is " + script_name)
DEBUG("flash tool path is " + flash_tool_path)

# Determine location of Software Development Kit (SDK)
SDK = os.getenv("SDK")
if not SDK:
    # Walk up directories from the current working directory
    curr_dir = os.getcwd()
    DEBUG("Initial cwd is " + curr_dir)
    last_dir=""
    while curr_dir != last_dir:
        DEBUG("Check for SDK top-level: " + curr_dir)
        if os.path.isdir(os.path.join(curr_dir, flash_tool_path)):
            SDK = curr_dir
            break
        last_dir = curr_dir
        curr_dir = os.path.dirname(curr_dir)
    if not SDK:
        FAIL("Cannot locate Software Development Kit");

DEBUG("SDK is " + SDK)

SDK_flash_tools = os.path.join(SDK, flash_tool_path)
if (os.path.isdir(SDK_flash_tools)):
    DEBUG("Flash tools are at " + SDK_flash_tools)
else:
    FAIL("Cannot find flash tools.")

# Set want_gen_*_table
if want_gen:
    want_gen_partition_table = True
    want_gen_fwd_table = True
else:
    want_gen_partition_table = False
    want_gen_fwd_table = False

    if not do_erase:
        if not os.path.isfile((os.path.join("output", "generated_fwd_table.xml"))):
            want_gen_fwd_table = True

        if want_gen_fwd_table:
            if not os.path.isfile((os.path.join("output", "generated_partition_table.xml"))):
                want_gen_partition_table = True

if want_gen_partition_table:
    # Create a Partition Table
    validate_file(os.path.join(SDK_flash_tools, "gen_part_table.py"))

    # Set M4_img_list to a list of all M4 applications

    if args.app:
        # --app was specified at least once
        M4_img_list = []
        for app in args.app:
            M4_img_list.append(path_to_app(app))
    else:
        # --app was not specified so look in the current directory for an app
        M4_img_list = [path_to_app(os.getcwd())]

    # At this point, M4_img_list contains a list of one or more M4 applications
    # to be programmed to flash.
    
    if not M4_img_list:
        FAIL("Please check the value of --app.")
    DEBUG("Use M4 image(s): " + str(M4_img_list))

    
    INFO("Generate partition table...")
    M4_part = ""
    for M4_img_name in M4_img_list:
        DEBUG("M4_img_name is " + M4_img_name)
        validate_file(M4_img_name)
        M4_part = M4_part + " --partition --file=" + '"' + M4_img_name + '"'
        M4_name = os.path.basename(M4_img_name)
        DEBUG("M4_name is " + M4_name)
        if M4_name == "m4ftm_HASHED.elf":
            FTM_image_set = True
    
    if secboot_dir:
        M0_img_name = os.path.join(secboot_dir, "m0", "ioe_ram_m0_threadx_ipt.mbn")
    else:
        M0_img_name = os.path.join(SDK, "bin", "cortex-m0", "threadx", "ioe_ram_m0_threadx_ipt.mbn")
    validate_file(M0_img_name)
    M0_part = " --partition --file=" + '"' + M0_img_name + '"'
    
    KF_img_name=""
    if secboot_dir:
        wlan_dir = os.path.join(secboot_dir, "kf")
    else:
        wlan_dir = os.path.join(SDK, "bin", "wlan")
    DEBUG("wlan_dir is " + wlan_dir)

    FS1IMG = os.getenv("FS1IMG")
    FS2IMG = os.getenv("FS2IMG")

    # If FS2IMG environment variable is not set, default to NONE
    if str(FS2IMG) == "None":
        FS2IMG = "NONE"

    # Non-default Primary Filesystem size?
    FS1SZ = os.getenv("FS1SZ")
    if FS1SZ:
        FS1_sizeKB = int(FS1SZ)

    # Non-default Secondary Filesystem size?
    FS2SZ = os.getenv("FS2SZ")
    if FS2SZ:
        FS2_sizeKB = int(FS2SZ)
    if FS2_sizeKB == 0:
        FS2IMG = "NONE"

    # Non-default Flash logging partition
    FLOGSZ = os.getenv("FLOGSZ")
    if FLOGSZ:
        FLOG_sizeKB = int(FLOGSZ)

    if FTM_image_set:
        #
        # Special handling for FTM (manufacturing) firmware.
        # FTM uses a special "UTF" WLAN image.
        #
        if (os.path.isdir(wlan_dir)):
            KF_img_name = os.path.join(wlan_dir, "wlan_utf_img.bin") # mfg KF img
        FS_floating_part=" --partition --id=FS1 --size=" + str(FS1_sizeKB) + "KB"
        if FS1IMG:
            FS_floating_part = FS_floating_part + ' --file="' + FS1IMG + '"'

        # FTM always includes a secondary FS. Ignore "NONE".
        FS_fixed_part=" --partition --id=FS2 --start=12KB --size=" + str(FS2_sizeKB) + "KB"
        if FS2IMG and (FS2IMG.upper() != "NONE"):
            FS_fixed_part = FS_fixed_part + ' --file="' + FS2IMG + '"'
    else:
        #
        # Normal application; not FTM/manufacturing firmware.
        # By default, use a fixed-position FS1 and FS2. FS1 is the automounted file system
        # used during normal operation whereas FS2's purpose is to support concurrent
        # FW Upgrade (so the Upgrade engine copies files from FS1 to FS2).
        #
        # If a secondary file system is not desired, set FS2IMG="NONE".
        #
        if (os.path.isdir(wlan_dir)):
            KF_img_name = os.path.join(wlan_dir, "wlan_fw_img.bin") # std KF img
        FS_floating_part=""
        FS_fixed_part=" --partition --id=FS1 --start=12KB --size=" + str(FS1_sizeKB) + "KB"
        if FS1IMG:
            FS_fixed_part = FS_fixed_part + ' --file="' + FS1IMG + '"'

        if FS2IMG and (FS2IMG.upper() == "NONE"):
            DEBUG("FS2IMG IS NONE")
            FS2_sizeKB = 0
        else:
            FS_fixed_part = FS_fixed_part + " --partition --id=FS2 --start=" + str(FS1_sizeKB+12) + "KB --size=" + str(FS2_sizeKB) + "KB"
            if FS2IMG:
                DEBUG("FS2IMG IS " + str(FS2IMG))
                FS_fixed_part = FS_fixed_part + ' --file="' + FS2IMG + '"'
    
    if KF_img_name != "":
        KF_part=" --partition --file=" + '"' + KF_img_name + '"'
    else:
        KF_part=""

    # Regardless of whether normal application or FTM/manufacting firmware is used,
    # avoid reserving Flash partitions if logging to Flash mode is not used.
    if FLOG_sizeKB != 0:
        FLOG_part=" --partition --id=FLOG --size=" + str(FLOG_sizeKB) + "KB" + " --start=" + str(12 + FS1_sizeKB + FS2_sizeKB) + "KB"
    else:
        FLOG_part=""

    # Remove old version of partition_table, if any
    try:
        os.remove(os.path.join("output", "generated_partition_table.xml"))
    except:
        pass

    # Generate a new partition table
    # Always includes M4 firmware and M0 firmware.
    # Optionally includes KF WLAN firmware
    #     wlan image for normal application firmware
    #     utf image for FTM firmware
    # Includes one or two file systems.
    #     For normal application firmware
    #         a single fixed-position primary file system
    #     For FTM firmware
    #         a fixed-position secondary file system plus
    #         a floating-position primary file system
    #
    cmd_string = "python " + '"' + os.path.join(SDK_flash_tools, "gen_part_table.py") + '"' +\
        " --output=" + '"' + os.path.join("output", "generated_partition_table.xml") + '"' +\
        " --begin=" + str(12 + FS1_sizeKB + FS2_sizeKB + FLOG_sizeKB) + "KB" +\
        M4_part +\
        M0_part +\
        KF_part +\
        FS_floating_part +\
        FS_fixed_part +\
        " --partition --id=UNUSED --size=8KB --start=4KB" +\
        FLOG_part

    DEBUG("Execute: " + cmd_string);
    try:
        subprocess.check_output(cmd_string, shell=need_shell)

    except:
        FAIL("gen_part_table.py failed")
    
    validate_file(os.path.join("output", "generated_partition_table.xml"))
else:
    if args.app:
        FAIL("Application was specified (--app) even though partition table generation was suppressed (--nogen).")
    DEBUG("Partition table generation suppressed (--nogen).")

if want_gen_fwd_table:
    validate_file(os.path.join(SDK_flash_tools, "gen_fwd_table.py"))
    INFO("Generate FWD table...")
    # Create a FWD Table
    # Remove old version of fwd_table, if any
    try:
        os.remove(os.path.join("output", "generated_fwd_table.xml"))
    except:
        pass

    cmd_string = "python " + '"' + os.path.join(SDK_flash_tools, "gen_fwd_table.py") + '"' +\
        " -x " + '"' + os.path.join("output", "generated_partition_table.xml") + '"' +\
        " --rawprogram " + '"' + os.path.join("output", "generated_fwd_table.xml") + '"' +\
        " --fdtbin " + '"' + os.path.join("output", "firmware_table.bin") + '"'

    DEBUG("Execute: " + cmd_string);
    try:
        subprocess.check_output(cmd_string, shell=need_shell)

    except:
        FAIL("gen_fwd_table.py failed")
    
    validate_file(os.path.join("output", "generated_fwd_table.xml"))
else:
    DEBUG("FWD table generation suppressed (--nogen).")

if want_dev:
    # Load Device Programmer into Quartz RAM

    # Validate that comm_port is set
    if comm_port == "":
        # Try to get COMM_PORT from the environment
        comm_port = os.getenv("COMM_PORT")

    if comm_port:
        DEBUG("Using COMM_PORT " + comm_port)
    else:
        FAIL("Set COMM_PORT to the port number of QDLoader. See Device Manager.")

    validate_file(os.path.join(SDK_flash_tools, "fh_loader.exe"))
    validate_file(os.path.join(SDK_flash_tools, "QSaharaServer.exe"))
    validate_file(os.path.join(SDK_flash_tools, "prog_spinor_firehose_qca4020_lite_m4_threadx.mbn"))

    INFO("Download device programmer...")

    # Convert the name of the Device Programmer to backslash-separated components as required
    # by the native Windows QSaharaServer application. Note that this script may be executed
    # using Cygwin python (/) or native Windows python (\); but QSaharaServer requires Windows
    # paths.
    windows_dev_prog_name = fixpath(os.path.join(SDK_flash_tools, "prog_spinor_firehose_qca4020_lite_m4_threadx.mbn"))
    DEBUG("windows_dev_prog_name is " + windows_dev_prog_name)

    comm_port_string = 'COM' + comm_port

    # Confusing. In the call to check_output, we want
    # \\.\ on Windows and \\\\.\\ on cygwin
    # so that when QSaharaServer sees it, it looks like "\\.\".
    if os.sep == '\\':
        comm_port_string = '\\\\.\\' + comm_port_string
    else:
        comm_port_string = '\\\\\\\\.\\\\' + comm_port_string

    cmd_string = '"' + os.path.join(SDK_flash_tools, "QSaharaServer.exe") + '"' +\
                 " -p " + comm_port_string +\
                 " -s 13:" + '"' + windows_dev_prog_name + '"'

    DEBUG("Execute: " + cmd_string);
    QSaharaServer_good = True
    try:
        subprocess.check_output(cmd_string, shell=need_shell)

    except:
        QSaharaServer_good = False

    if QSaharaServer_good:
        INFO("Device Programmer loaded successfully")
		
        if do_erase:
			cmd_string = '"' + os.path.join(SDK_flash_tools, "fh_loader.exe") + '"' +\
				" --port=" + comm_port_string +\
				" --memoryname=spinor" +\
				" --porttracename=output\port_trace.txt" +\
				" --erase=0"
			DEBUG("Execute: " + cmd_string);
			try:
				subprocess.check_output(cmd_string, shell=need_shell)

			except:
				FAIL("fh_loader.exe failed to program flash. You may find messages in " + windows_port_trace)

			INFO("Flash Erase complete!")
			sys.exit(0) # success
			
        else:
			INFO("Skipping to program image")
    else:
        INFO("Check for QDLoader port in Device Manager.")
        INFO("Need to reset device?")
        FAIL("QSaharaServer failed to load Device Programmer.")

    fwd_table = os.path.join("output", "generated_fwd_table.xml")
    validate_file(fwd_table)
    windows_fwd_table=fixpath(fwd_table)
    
    # Use Firehose Loader to send flash directives to Device Programmer which now runs on Quartz
    INFO("Program flash. Please wait...")
    DEBUG("Using " + fwd_table + " to program flash")

    windows_m0_dir = fixpath(os.path.join(SDK, "bin", "cortex-m0", "threadx"))
    DEBUG("windows_m0_dir is " + windows_m0_dir)

    windows_port_trace = fixpath(os.path.join("output", "port_trace.txt"))
    DEBUG("windows_port_trace is " + windows_port_trace)

    windows_wlan_dir = fixpath(os.path.join(SDK, "bin", "wlan"))
    DEBUG("windows_wlan_dir is " + windows_wlan_dir)

    # Extract all dirnames from generated_partition_table.xml to form search_path
    search_path=""
    xmldoc = minidom.parse(os.path.join("output", "generated_partition_table.xml"))
    partlist = xmldoc.getElementsByTagName("partition")
    for part in partlist:
        if part.hasAttribute('dirname'):
            if len(search_path):
                search_path = search_path + ","
            search_path = search_path + '"' + part.attributes["dirname"].value + '"' 

    if len(search_path) == 0:
        # NO dirnames found? Unusual case.
        search_path = "output"
    else:
        # Need to find firmware_table.bin in cases where output isn't otherwise on the search path
        search_path = search_path + "," + "output"

    cmd_string = '"' + os.path.join(SDK_flash_tools, "fh_loader.exe") + '"' +\
                 " --port=" + comm_port_string +\
                 " --porttracename=" + '"' + windows_port_trace + '"' +\
                 " --memoryname=spinor" +\
                 " --sendxml=" + '"' + windows_fwd_table + '"' +\
                 " --search_path=" + search_path

    DEBUG("Execute: " + cmd_string);
    try:
        subprocess.check_output(cmd_string, shell=need_shell)

    except:
        FAIL("fh_loader.exe failed to program flash. You may find messages in " + windows_port_trace)

    INFO("Flash programming complete!")
else:
    DEBUG("Device access suppressed (--nodev).")

sys.exit(0) # success
