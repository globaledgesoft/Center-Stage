#!/usr/bin/python

#===============================================================================
# Copyright (c) 2015-2018 Qualcomm Technologies, Inc.
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

#===============================================================================

'''
Script to flash images to storage using JLINK/GDB Interface. This scripts will read a XML file and use it to flash the images to the device.

$Header: //components/rel/core.ioe/1.0/storage/tools/scripts/flash_through_gdb/flash_through_gdb.py#16 $
$DateTime: 2018/05/22 21:04:49 $
$Author: pwbldsvc $

when        who       what, where, why
----------  --------  -----------------------------------------------------
2017-03-04  wkimberl  Exit with non zero value in case of error. 2y.
2016-08-08  wkimberl  Add option to read back the data and verify.
2016-01-25  wkimberl  Allow any host to be the GDB Server.
2015-09-04  wkimberl  Create. 6m.

'''

import xml.etree.ElementTree as ET
import logging
import struct
import os
import math
import sys
import subprocess
import datetime
import filecmp


def search_for_file (filename, folder_list):
    ''' Searches the list of "folder_list" until it finds a file with the given
    filename on that folder.

    filename      String, the file name to search for.
    folder_list  List of strings containing different folders to search.

    Returns the full path to where the file was found. None if the file was not
    found.'''

    if os.path.isabs (filename):
        return filename

    #Check that the input is a list of folders
    if not isinstance(folder_list, list):
        return None

    full_path = None

    for path in folder_list:
        try:
            candidate_path = path + filename
            if os.path.isfile (candidate_path):
                if full_path is None:
                    full_path = candidate_path
                else:
                    logging.error ("The file %s is found in two places %s and %s, using the first" % (filename, full_path, candidate_path))
        except:
            logging.debug('Error while searching for file %s in path %s' % (filename, path))

    return full_path

def search_path_cleanup (path_list):
    ''' Cleans up the Search path given as input from command line arguments.
    The paths are layed out in an array of arrays, depending on how the user
    typed the search path information. This function cleans up the
    list and generates a one dimentional list with all the search paths.
    The current directory './' is always appended to the end. '''
    folder_list = []
    for cmd_arg_spath in path_list:
        # each argument may be a list, get each member of the list.
        for spath in cmd_arg_spath:
            # If the command line argument is given as path1,path2,path3
            # Split the string and get each path by itself.
            paths = spath.split(',')
            for path in paths:
                # Check if it ends with /
                if path[-1] != '/' and path[-1] != '\\':
                    path = path + '/'
                folder_list.append(path)
    folder_list.append('./')

    return folder_list


class Flash_Gdb:
    ''' Class to interact with the device through GDB.
    It implements the basic read/write/erase calls to flash by using GDB
    command file. For example, if it needs to write data it will:
    1. Load the JTAG Programmer ELF
    2. Set the parameters to flash.
    3. Run until completion.
    4. Get the result.

    All these steps are specified on the command file given to GDB.
    The result is retrive by reading a different file. '''

    JTAG_CMD_PROGRAM              = 0x00000001
    JTAG_CMD_ERASE                = 0x00000002
    JTAG_CMD_CHIP_ERASE           = 0x00000005
    JTAG_CMD_READ_SECTOR          = 0x00000009
    JTAG_CMD_WRITE_SECTOR         = 0x0000000A
    JTAG_CMD_PRINT_INFO           = 0x00000020
    JTAG_CMD_SET_HW_PARTITION     = 0x00000031
    JTAG_CMD_SET_ACTIVE_BOOT      = 0x00000035
    JTAG_CMD_PROG_GPP             = 0x00000036
    JTAG_CMD_ERASE_GENERIC        = 0x00000040
    JTAG_CMD_COMPUTE_CRC          = 0x00000041
    JTAG_CMD_PROG_EXTRAS          = 0x00000042
    JTAG_CMD_ERASE_PARTITION      = 0x00000043

    block_size                    = 4096
    #Chunk size is the maximum size of the buffer on target.
    chunk_size                    = (32 * 1024)
    sect_per_chunk                = chunk_size / block_size

    def __init__ (self, jtag_prg_elf, gdb_server = ":2331", batch = True, outdir = './', gdb_path = 'arm-none-eabi-gdb', dev_type = 'MEM_DEVICE_SPINOR'):
        ''' Initalize the class to interact with the device for flashing trhough GDB.
        gdb_server The listening server:port of the GDB Server.
        gdb_path   Path for the GDB Executable that can run in the current architecture and the target is ARM.
        outdir     Location where th place the output files.

        '''
        self.gdb_server = gdb_server
        self.gdb_cmd_file_path = outdir + 'gdb_commands.txt'
        self.gdb_cmd_file_path_exec = outdir + 'gdb_commands_list.txt'
        self.gdb_path = gdb_path
        self.cmd_status_file = outdir + 'command_result.log'
        self.jtag_prg_log = outdir + 'jtagprogrammer.log'
        self.jtag_prg_elf_path = jtag_prg_elf
        self.dev_type = dev_type
        self.total_write = 0
        self.total_read = 0
        self.total_erase = 0
        self.need_cmd_shell = False
        self.check_cmd_shell = True


        # Remove the file used to append the output log of JTAG Programmer.
        try:
            os.remove (self.jtag_prg_log)
        except:
            pass
        try:
            os.remove (self.gdb_cmd_file_path_exec)
        except:
            pass

        self.cmd_file = self._gdb_cmd_file_open()

        # Defines "batch" mode.
        # batch = False  Each method is called individually (slower, loads ELF for each call)
        # batch = True   All methods are put together into a single GDB command file.
        if batch == True:
            self._add_target_init()

        self.batch = batch


    def __del__ (self):
        ''' Cleanup things created during Init '''
        self.cmd_file.close()

    def _add_target_init (self):

        common_commands = """target remote %s
load %s
file %s
set $pc = jstorage_init
set $sp = &jstorage_stack[sizeof(jstorage_stack)]
set {int}0xE000ED94 = 0
set {int}0x44000008 = 0
break main_c
continue
""" % (self.gdb_server, self.jtag_prg_elf_path, self.jtag_prg_elf_path)

        self.cmd_file.write(common_commands)

    def _gdb_cmd_file_open (self):
        ''' Populates the GDB Command file with the common set of GDB
        commands used for programming

        returns File handle to write the commands into'''

        try:
            f = open(self.gdb_cmd_file_path , 'w+')
        except IOError as e:
            logging.exception("Unable to open the file '%s'\n" %
                              (self.gdb_cmd_file_path))
            raise AssertionError("Can't open file %s" % (self.gdb_cmd_file_path))
        return f

    def _gdb_init_target (self):
        ''' Adds the init target code if required '''
        if self.batch == False:
            self._add_target_init()

    def _gdb_cmd_done (self):
        ''' Populates the GDB Command file with the common set of GDB
        commands used for getting data after programming.

        param f File handle to write the commands into'''

        # Get the output buffer
        # Get the command result.
        # Add one to the output buffer length in case it is zero, to avoid errors.
        cmd = "append binary memory %s output_buffer &output_buffer[output_buffer_length+1]\n" % (self.jtag_prg_log)
        cmd = cmd + "set output_buffer_length = 0\n"
        cmd = cmd + "dump binary value %s jtag_prog_param.cmd_status\n" % (self.cmd_status_file)

        self.cmd_file.write(cmd)
        if self.batch == False:
            self.run()

    def _check_status (self):
        ''' Checks if the command status file is zero. If the file does not
        exist or returns zero, then return error'''

        # If in batch mode return success.
        if self.batch == True:
            return 0

        cmd_result = struct.Struct('<Q')
        result = None

        try:
            f = open(self.cmd_status_file , 'rb')
            data = f.read(8)
            f.close()
            all_bytes = cmd_result.unpack(data)
            result = all_bytes[0]
        except IOError as e:
            logging.exception("Unable to open the file '%s'\n" %
                              (self.cmd_status_file))

        if result != 0:
            logging.warning("The last command return error\n")

        return result


    def _write_str (self, in_file, in_offset, start_sector, num_sectors):
        ''' Create a string to write the input file at a given offset into
        the device at start_sector with a length of num_sectors'''

        d = ''
        size_bytes = num_sectors * self.block_size
        if (size_bytes > self.chunk_size):
            logging.critical("Amount of data to write is too large %d > %d\n" % (size_bytes, self.chunk_size))
            raise AssertionError("Amount of data to write is too large %d > %d\n" % (size_bytes, self.chunk_size))

        self.total_write = self.total_write + size_bytes

        d = d + ('echo "WRITE %s, offset=%d start_sector=%d num_sectors=%d"\n' % (in_file.replace('\\', '\\\\'), in_offset, start_sector, num_sectors))
        d = d + ('set device_type = %s\n' % (self.dev_type))
        d = d + ('set jtag_prog_param.cmd_status = %d\n' % (self.JTAG_CMD_WRITE_SECTOR))
        d = d + ('set jtag_prog_param.addr = %d\n' % (start_sector))
        d = d + ('set jtag_prog_param.data_length = %d\n' % (num_sectors))
        # The restore command will add the offset to both the file and the
        # address. Substract the offset from the destination address so the
        # data falls into the correct location
        d = d + ('restore %s binary jtag_prog_param.data-%d %d %d\n' % (in_file, in_offset, in_offset, in_offset + size_bytes))

        d = d + ('continue\n')


        return d


    def write (self, in_file, start_sector, total_sectors):
        ''' Writes data from infile into the storage part '''

        offset = 0
        sect_offset = 0
        file_size = os.path.getsize(in_file)

        # Divide the writes in chunks of chunk_size.
        while offset < file_size and total_sectors > 0:

            # Check if the total sectors to write fit on the target.
            if total_sectors < self.sect_per_chunk:
                num_sectors = total_sectors
                num_sectors_in_file = math.ceil(float(file_size - offset) / self.block_size)
                if num_sectors_in_file > num_sectors:
                    logging.debug("The input file %s size (%d bytes, %d sectors) is larger than the total number of sectors %d"
                                  % (in_file, file_size, num_sectors_in_file, total_sectors))
                if num_sectors_in_file < num_sectors:
                    logging.debug("The input file %s size (%d bytes, %d sectors) is smaller than the total number of sectors %d"
                                  % (in_file, file_size, num_sectors_in_file, total_sectors))
            elif file_size - offset < self.chunk_size:
                # check if the data to write is less than the chunk size.
                num_sectors = int(math.ceil(float(file_size - offset) / self.block_size))
            else:
                num_sectors = self.sect_per_chunk

            self._gdb_init_target()
            self.cmd_file.write(self._write_str(in_file, offset, start_sector + sect_offset, num_sectors))
            self._gdb_cmd_done ()

            result = self._check_status()
            if result != 0:
                break

            offset = offset + num_sectors * self.block_size
            total_sectors = total_sectors - num_sectors
            sect_offset = sect_offset + num_sectors

        return result

    def _read_str (self, out_file, append, start_sector, num_sectors):
        ''' Create the Read string for a file through GDB.  '''

        size_bytes = num_sectors * self.block_size
        if (size_bytes > self.chunk_size):
            logging.critical("Amount of data to read is too large %d > %d\n" % (size_bytes, self.chunk_size))
            raise AssertionError("Amount of data to read is too large %d > %d\n" % (size_bytes, self.chunk_size))

        self.total_read = self.total_read + size_bytes

        d = ''
        d = d + ('echo "READ %s, append=%d start_sector=%d num_sectors=%d"\n' % (out_file.replace('\\', '\\\\'), append, start_sector, num_sectors))
        d = d + ('set device_type = %s\n' % (self.dev_type))
        d = d + ('set jtag_prog_param.cmd_status = %d\n' % (self.JTAG_CMD_READ_SECTOR))
        d = d + ('set jtag_prog_param.addr = %d\n' % (start_sector))
        d = d + ('set jtag_prog_param.data_length = %d\n' % (num_sectors))
        d = d + ('continue\n')

        if append == True:
            d = d + ('append binary memory %s jtag_prog_param.data &jtag_prog_param.data[%d]\n' % (out_file, size_bytes))
        else:
            d = d + ('dump binary memory %s jtag_prog_param.data &jtag_prog_param.data[%d]\n' % (out_file, size_bytes))

        return d

    def read (self, out_file, start_sector, total_sectors):

        sect_offset = 0
        # ON the first write don't append, all subsequent writes do append to output file.
        append = False

        # Divide the read in chunks of chunk_size.
        while total_sectors > 0:

            # Check if the total sectors to write fit on the target.
            if total_sectors < self.sect_per_chunk:
                num_sectors = total_sectors
            else:
                num_sectors = self.sect_per_chunk

            self._gdb_init_target()
            self.cmd_file.write (self._read_str(out_file, append, start_sector + sect_offset, num_sectors))
            self._gdb_cmd_done ()

            # All subsequent reads should be appended to file on Host (PC)
            append = True

            result = self._check_status()
            if result != 0:
                break

            total_sectors = total_sectors - num_sectors
            sect_offset = sect_offset + num_sectors

        return result

    def _erase_str (self, start_sector, num_sectors):
        ''' Create the Erase string for a file through GDB.  '''

        self.total_erase = self.total_erase + num_sectors * self.block_size

        d = ''
        d = d + ('print "ERASE start_sector=%d num_sectors=%d"\n' % (start_sector, num_sectors))
        d = d + ('set device_type = %s\n' % (self.dev_type))
        d = d + ('set jtag_prog_param.cmd_status = %d\n' % (self.JTAG_CMD_ERASE_PARTITION))
        d = d + ('set jtag_prog_param.addr = %d\n' % (start_sector))
        d = d + ('set jtag_prog_param.data_length = %d\n' % (num_sectors))
        d = d + ('continue\n')

        return d

    def erase (self, start_sector, num_sectors):

        self._gdb_init_target()
        self.cmd_file.write (self._erase_str(start_sector, num_sectors))
        self._gdb_cmd_done ()

        result = self._check_status()

        return result

    def _append_exec_commands(self):
        ''' Appends all the commnds executed so far to the outupt exec file
        '''
        self.cmd_file.seek(0)

        try:
            f = open(self.gdb_cmd_file_path_exec, 'a')
            data = self.cmd_file.read()
            f.write (data)
            f.close()
        except IOError as e:
            logging.exception("Unable to open the file '%s'\n" %
                              (self.gdb_cmd_file_path))
            raise AssertionError("Can't open file %s" % (self.gdb_cmd_file_path))

    def _check_need_shell(self):
        if self.check_cmd_shell == True:
            use_shell = False
            try:
                exec_str = '%s --version' % (self.gdb_path)
                subprocess.call (exec_str)
                use_shell = False
                logging.info("GDB can run without shell=true")
            except:
                logging.info("GDB may require shell=true, will try on shell mode")
                use_shell = True

            self.need_cmd_shell = use_shell
            self.check_cmd_shell = False

        return self.need_cmd_shell


    def run (self):
        self.cmd_file.flush()
        if self.cmd_file.tell() != 0:
            exec_str = '%s --command="%s" --batch' % (self.gdb_path, self.gdb_cmd_file_path)
            #exec_str = '"' + self.gdb_path + '"' + ' --command="' + self.gdb_cmd_file_path + '" --batch'
            logging.debug(exec_str)

            need_shell = self._check_need_shell()
            if need_shell == True:
                subprocess.call (exec_str, shell=True)
            else:
                subprocess.call (exec_str)

        #We just executed the command, clean up and create a copy.
        self._append_exec_commands()
        self.cmd_file.seek(0)
        self.cmd_file.truncate(0)

    def get_total_write_bytes(self):
        return self.total_write

    def get_total_read_bytes(self):
        return self.total_read

    def get_total_erase_bytes(self):
        return self.total_erase



def gdb_check_availability (gdb_path):
    ''' Check if the available gdb can be used to interact with ARM.
    gdb_path   Path to the GDB executable binary.

    returns    The same path on success, None if the GDB in the path in not valid '''

    if os.path.isfile (gdb_path) == False:
        gdb_path = None
        logging.error ('Path %s does not point to a file' % (gdb_path))
        return gdb_path

    out_str = ''

    try:
        out_str = subprocess.check_output([gdb_path, '--version'])
    except OSError as e:
        if e.errno == os.errno.ENOENT:
            logging.error ('Did not find GDB at %s' % (gdb_path))
            gdb_path = None
        else:
            logging.error ('GDB at %s not executable? see --help for more info' % (gdb_path))
            gdb_path = None
    except:
        logging.error ('GDB at %s not executable? see --help for more info' % (gdb_path))
        gdb_path = None

    if ' GDB ' not in out_str or ' gdb ' not in out_str:
        logging.error ('Please check %s, looks like it is not a GDB executable' % (gdb_path))

    if '--target=arm' not in out_str:
        logging.error ('The given GDB does not support ARM. Please install an ARM Compatible GDB (see --help for more info).')

    return gdb_path

def jtag_prg_elf_path_check (jtagprgelf):
    ''' Check if the JTAG Programmer ELF is valid
    jtagprgelf   Path to the JTAG Programmer elf.

    returns    The same path on success, None if the JTAG Programmer ELF is not valid'''

    jtag_elf = None
    if os.path.isfile (jtagprgelf) == False:
        logging.error ('There is no ELF file at %s' % (jtagprgelf))
        return jtag_elf

    try:
        f = open(jtagprgelf, 'rb')
        # Look in the file for a string/symbol known to exist in an ELF with symbols.
        for line in f:
            if 'jtag_prog_param' in line:
                # The string of a symbol is in the file, most likely an ELF with symbols.
                jtag_elf = jtagprgelf
                break
        f.close()
    except:
        logging.exception("Unable to access the file '%s'\n" %
                          (jtagprgelf))


    return jtag_elf

def cleanup_path (path):
    ''' Cleans up the path by changing all the \ to / '''
    path = path.replace('\\','/')
    path = path.encode('string-escape')
    path = path.replace('\\','/')
    return path

def main():

    # Give a version number to this script.
    script_version = '1.0.5'

    # Check the version, since some changes to argparse on Python 3 causes it to fail.
    if sys.version_info[0] >= 3:
        logging.critical ("This tool won't run on Python 3, please use Python 2.7")
        return

    if sys.version_info[0] <= 2 and sys.version_info[1] < 7:
        logging.warning ("This tool requires on Python 2.7. Most systems have python 2.7 installed, but it is not part of the PATH")
        return

    import argparse

    tool_verbose_description = """Flash images described in rawprogra XML to a target using GDB and JTAG Programmer ELF.

The rawprogram XML file describes where files should be placed on disk. This
tool will read that file and translate the information into a series of GDB
commands that will read each file and flash them to the device.

There are 3 output files generated with information of what was done:
gdb_commands_list.txt: List of GDB commands executed. Copy paste to GDB client to reproduce the steps.
command_result.log:    Value of the last result executed on target. Only useful if not using Batch mode.
jtagprogrammer.log:    Logs from the target.

example usage:

- Flash the set of images as described in rawprogram.xml using the GDB at the specified path and the JTAG Programmer elf at the given path.
python flash_through_gdb.py --rawprogram rawprogram.xml --gdbpath gcc-arm-none-eabi-4_9-2015q3/bin/arm-none-eabi-gdb --jtagprgelf "../../../../bsp/tools/jtagprogrammer/build/ARNFRI/JTAGPROGRAMMER_IMG_ARNFRI.elf"

- Flash the set of images as described in rawprogram.xml using the JTAG Programmer ELF in C:/temp. Use the defaults for Output directory, GDB, search paths and others.
python flash_through_gdb.py --rawprogram rawprogram.xml --jtagprgelf C:/temp/JTAGPROGRAMMER_IMG_ARNFRI.elf"

"""

    parser = argparse.ArgumentParser(description=tool_verbose_description, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('-x', '--rawprogram', type=file, required=True, help='The rawprogram.xml that describes how to write things to disk.')
    parser.add_argument('-v', '--verbose', type=int, choices=[0,1,2,3,4,5], help='Verbose levels. Higher numbers include lower. For example, 3 means 3,2,1 and 0. 0=Critcal. 1=Error, 2=Warning 3=Info[Default], 4=Debug, 5=Everything', default=3)
    parser.add_argument('--outputdir', type=str, required=False, help='Location where to put the log files and data read from the target. Defaults to "./"')
    parser.add_argument('--search_path', type=str, nargs='*', required=False, action='append', help='List of paths where to search for the files in rawprogramxml. The paths can be given with comma separated (--search_path=path1,path2,path3) or using multiple search paths (--search_path path1 --search_path path2). the current directory "./" is always added to the end of the list. If the path has spaces it will be treated as two different paths, put quotes around paths with spaces.')
    parser.add_argument('--gdbport', type=int, required=False, help='Local port to use to communicate with GDB Server, default is 2331. Ignored if --gdbserver is used.', default=2331)
    parser.add_argument('--gdbserver', type=str, required=False, help='server:port where the GDB Server is running, Default ":2331".')
    parser.add_argument('--gdbpath', type=str, required=False, help='Path to the local executable of ARM Compatible GDB. You can download an ARM compatible version of GDB client at https://launchpad.net/gcc-arm-embedded/+download . Defaults to "C:/Program Files (x86)/GNU Tools ARM Embedded/4.9 2015q3/bin/arm-none-eabi-gdb.exe" on windows. Needs to be specified for Linux.', default = "C:/Program Files (x86)/GNU Tools ARM Embedded/4.9 2015q3/bin/arm-none-eabi-gdb.exe")
    parser.add_argument('--jtagprgelf', type=str, required=True, help='Path to the JTAG_PROGRAMMER ELF Image compiled in this build. This ELF image must contain symbols. It is typically located inside the build at core/bsp/tools/jtagprogrammer/build/ARN*RI/JTAGPROGRAMMER_IMG_ARN*RI.elf')
    parser.add_argument('--verify', action='store_true', help='Read back the data to make sure it matches the data written, it forces ')

    args = parser.parse_args()

    log_level = [logging.CRITICAL, logging.ERROR, logging.WARNING, logging.INFO, logging.DEBUG, logging.NOTSET]
    logging.basicConfig(format='[%(asctime)-15s] %(levelname)s: %(message)s File:%(filename)s:%(lineno)d Function:%(funcName)s ', level=log_level[args.verbose])

    logging.info ('flash_through_gdb.py version %s' % (script_version))

    #Check if the GDB File path is valid
    gdbpath = args.gdbpath
    gdbpath = cleanup_path (gdbpath)
    gdbpath = gdb_check_availability (gdbpath)
    if gdbpath == None:
        logging.critical('The given GDB path is not valid')
        return

    #Check if jtag programmer elf is valid:
    jtag_elf_path = args.jtagprgelf
    jtag_elf_path = cleanup_path (jtag_elf_path)
    jtag_elf_path = jtag_prg_elf_path_check (jtag_elf_path)
    if jtag_elf_path == None:
        logging.critical('The JTAG Programmer ELF is not valid. Do not use the one in build/ms. --help for more info.')
        return

    if args.gdbserver is None:
        gdbserver = (':%d' % args.gdbport)
    else:
        gdbserver = args.gdbserver

    # Get the output directory
    if args.outputdir is None:
        output_dir = './'
    else:
        output_dir = args.outputdir
        # Append a / if not present at the end.
        if output_dir[-1] != '/' and output_dir[-1] != '\\':
            output_dir = output_dir + '/'

    logging.debug ("Output Directory is %s" % (output_dir))

    # Get the list of search paths.
    if args.search_path is None:
        folder_list = ['./']
    else:
        folder_list = search_path_cleanup(args.search_path)

    count = 0
    for folder in folder_list:
        # Start count with 1 because it is more intuitive for most users.
        count = count + 1
        logging.debug ("Search Path %d %s" % (count, folder))

    xml = ET.parse (args.rawprogram.name)
    jdata = xml.getroot()
    if jdata.tag != 'jdata':
        raise ValueError("rapwrogram XML didn't start with the correct tag jdata")

    batch_mode = True
    if args.verify == True:
        batch_mode = False

    start_time = datetime.datetime.now()

    gdb_flash = Flash_Gdb(jtag_prg_elf = jtag_elf_path, batch = batch_mode, outdir = output_dir, gdb_server = gdbserver, gdb_path = gdbpath)
    for command in jdata:
        if command.tag == 'erase':
            start_sector = int(command.attrib['start_sector'], 0)
            num_sectors = int(command.attrib['num_partition_sectors'], 0)
            logging.info ("Will erase sectors on range [%d, %d]" % (start_sector, start_sector + num_sectors - 1))
            result = gdb_flash.erase (start_sector, num_sectors)
            if result != 0:
                logging.error("Failed to erase start sector=%d, num sectors=%d" % (start_sector, num_sectors))
                return -1

        elif command.tag == 'program':
            start_sector = int(command.attrib['start_sector'], 0)
            num_sectors = int(command.attrib['num_partition_sectors'], 0)
            file_name = command.attrib['filename']

            file_path = search_for_file (file_name, folder_list)
            if file_path is None:
                logging.error ("Coudn't find file %s to flash in the list of paths given" % (file_name))
                #Bail early!
                return -1
            else:
                logging.info ("Will write file %s into sectors [%d, %d]" % (file_path, start_sector, start_sector + num_sectors - 1))
                result = gdb_flash.write (file_path, start_sector, num_sectors)
                if result != 0:
                    logging.error("Failed to write to start sector=%d, num sectors=%d" % (start_sector, num_sectors))
                    return -1
                if args.verify == True and batch_mode == False:
                    read_path = output_dir + os.path.basename(file_name) + "_readback"
                    result = gdb_flash.read(read_path, start_sector, num_sectors)
                    if result != 0:
                        logging.error("Failed to read back verify to start sector=%d, num sectors=%d" % (start_sector, num_sectors))
                        return -1
                    file_size = os.path.getsize(file_path)
                    readf = open(read_path, "r+")
                    readf.truncate(file_size)
                    readf.close()
                    same = filecmp.cmp(read_path, file_path)
                    if same == True:
                        logging.info("Data successfully flashed to disk for %s" % (file_path))
                    else:
                        logging.error("Data read back was different than flashed, check %s and %s" % (file_path, read_path))
                        return -1


        elif command.tag == 'read':
            start_sector = int(command.attrib['start_sector'], 0)
            num_sectors = int(command.attrib['num_partition_sectors'], 0)
            file_name = command.attrib['filename']

            if os.path.isabs(file_name):
                file_path = filename
            else:
                file_path = output_dir + file_name
            logging.info ("Will read sectors [%d, %d] into file %s" % (start_sector, start_sector + num_sectors - 1, file_path))
            result = gdb_flash.read (file_path, start_sector, num_sectors)
            if result != 0:
                logging.error("Failed to read to start sector=%d, num sectors=%d" % (start_sector, num_sectors))
                return -1

    gdb_flash.run()

    end_time = datetime.datetime.now()
    total_time = end_time - start_time

    logging.info ("Script finished processing %s" % (args.rawprogram.name))

    return 0

#    import pdb; pdb.set_trace()
# The blelow lines are for reference and debugging. Allows to erase 8 sectors,
# flash a file, read back that file, erase the data and read it back. The first
# read should be the same as the first write. The last read should be filled with
# zeros.
#    gdb_flash = Flash_Gdb(batch = True)
#    gdb_flash.erase(0, 8)
#    gdb_flash.write('seq.bin', 0, 8)
#    gdb_flash.read('read_seq.bin', 0, 8)
#    gdb_flash.erase(0, 8)
#    gdb_flash.read('read_zero.bin', 0, 8)
#    gdb_flash.run()


if __name__ == "__main__":
    result = main()
    if result != 0:
        exit(result)
