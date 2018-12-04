#!/usr/bin/python

#===============================================================================
# Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
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
# Start a debug server that listens on a specified TCP port for a connection
# from gdb. Once connected, it speaks a small subset of gdb's "remote debug"
# protocol. In particular, given an address it returns a value from a RamDump
# file.
#
# Command line options:
#   --port N
#       defaults to 2331
#
# TBD: objectify/classify/pythonic
# TBD: support multiple files/address segments;
#      currently supports 1 RAM @ 0x10000000 plus 1 XIP @ 0x01000000
#===============================================================================

import sys
import os
import logging
import argparse
import subprocess
import re
import string
import glob
import socket

dumpserver_description = \
'''
Server, started from gdbinit scripts, used by gdb to access RAMDUMP files.
'''

want_debug = False

DBGCMD_SZ = 4096
DEAD32 = "addeadde" # 0xdeaddead

ack_enabled = True
gp_register = 17 * [0] # 17 general purpose CPU registers

M4_file_sz = 0
M0_file_sz = 0
XIP_file_sz = 0

# Read from a command coming from gdb.
def read_dbg_command():
    global dbgconn

    dbgcmd = bytearray(DBGCMD_SZ)
    nbytes = dbgconn.recv_into(dbgcmd, DBGCMD_SZ)

    return nbytes, dbgcmd

def cksum(response):
    return sum(ord(c) for c in response) % 256

# Write a response to gdb.
def write_dbg_response(response):
    global dbgconn
    global ack_enabled

    # The remote protocol requires escape characters. Since we
    # only send numbers we should never get into this code;
    # include it just in case for future use.
    escaped_response = ""
    for c in response:
        if c in set('}#$*'):
            escaped_response = escaped_response + '}' + chr((ord(c) ^ 0x20))
        else:
            escaped_response = escaped_response + c

    if len(response) != len(escaped_response):
        logging.debug(">>>>>PRE-ESCAPE: " + repr(response))
        logging.debug(">>>>>PPOST-ESCAPE: " + repr(escaped_response))

    if ack_enabled:
        final_response_pkt = '+$' + escaped_response + '#' + format(cksum(escaped_response), '02x')
    else:
        final_response_pkt = '$' + escaped_response + '#ff'


    logging.debug("In write_dbg_response send " +
                  str(len(final_response_pkt)) + "-byte response: " +
                  repr(final_response_pkt))
    dbgconn.sendall(final_response_pkt)

    return

def unhandled():
    write_dbg_response("")
    return

def address_read_handler(cmdstr, nbytes):
    global M4_data
    global M0_data
    global XIP_data

    read_spec = re.search(r'm([^,]*),([^,]*)', cmdstr)
    if read_spec:
        addr, count = int(read_spec.group(1), 16), int(read_spec.group(2), 16)
        if (addr >= 0x10000000) and (addr < 0x10000000 + M4_file_sz):
            offset = addr - 0x10000000
            def mem_data(i):
                return M4_data[i]
        elif (addr >= 0x01000000) and (addr < 0x01000000 + XIP_file_sz):
            offset = addr - 0x01000000
            def mem_data(i):
                return XIP_data[i]
        elif (addr >= 0x05000000) and (addr < 0x05000000 + M0_file_sz):
            offset = addr - 0x05000000
            def mem_data(i):
                return M0_data[i]
        else:
            offset = -1
    logging.debug("In address_read_handler addr=0x" + str(addr) + " count=0x" + str(count))

    if offset >= 0:
        response = ""
        for i in range(0,count):
            response = response + "{0:02x}".format(ord(mem_data(offset+i)))
    else:
        response = DEAD32

    write_dbg_response(response)
    return

def cpureg_read_handler(cmdstr, nbytes):
    response=""
    for reg in gp_register:
        response = response + "{:08x}".format(reg)
    write_dbg_response(response)
    return

#
# This stub must allow gdb to WRITE gp registers so that in gdb we can set
# pc,lr,sp prior to performing a backtrace.
#
def cpureg_write_handler(cmdstr, nbytes):
    cmdstr = cmdstr[1:] # strip off leading 'G'
    for i in range(len(gp_register)):
        start_offset = 8*i
        gp_register[i] = int(cmdstr[start_offset:start_offset+8], 16)
    write_dbg_response("OK")
    return

def unknown_handler(cmdstr, nbytes):
    unhandled()
    return

def setup_handler(cmdstr, nbytes):
    global ack_enabled

    logging.debug("In setup_handler cmdstr=" + cmdstr)
    if re.search('QStartNoAckMode', cmdstr):
        logging.debug("NoAckMode")
        write_dbg_response("OK")
        ack_enabled = False
    else:
        logging.debug("unknown setup request")
        unhandled()
    return

def whystop_handler(cmdstr, nbytes):
    logging.debug("In whystop_handler cmdstr=" + cmdstr)
    write_dbg_response("S05") # Stop reason: "trap signal" (with no reason for trap)
    return

def start_dbg_server():
    global dbgport
    global dbgconn

    dbgsocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    dbgsocket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    dbgsocket.bind(('', dbgport))
    dbgsocket.listen(1)
    logging.debug("In start_dbg_server, waiting for connection on TCP port %d" % dbgport)
    dbgconn, address = dbgsocket.accept()
    logging.debug("In start_dbg_server, debug connection formed")

    #
    # Our goal is to support "read memory" commands ('m') from gdb.
    # In order to do this, we must support the gdb startup handshake.
    #
    # Notes regarding gdb 7.12.50:
    #     gdb/target.c::remote_start_remote is gdb's remote startup.
    #     If anything goes wrong, gdb ends up in gdb/target.c::target_close.
    #
    counter = 0
    while True:
        nbytes, cmdbuf = read_dbg_command()
        if not nbytes:
            break # Other end closed the socket. We are done.
        cmdstr = str(cmdbuf)[0:nbytes]
        logging.debug("read_dbg_command returns: " + cmdstr)
        cmd_spec = re.search('\$(.*)#[0-9A-Fa-f][0-9A-Fa-f]$', cmdstr)
        if cmd_spec != None:
            cmdstr = cmd_spec.group(1)
            cmd = cmdstr[0]
            logging.debug("Command: " + cmd)
        elif cmdstr is '+':
            logging.debug("received standalone ACK")
            continue
        else:
            logging.debug("Received junk: " + cmdstr)
            break

        if cmd == 'm':
            address_read_handler(cmdstr, nbytes)
        elif cmd == 'g':
            cpureg_read_handler(cmdstr, nbytes)
        elif cmd == 'G':
            cpureg_write_handler(cmdstr, nbytes)
        elif cmd == 'Q':
            setup_handler(cmdstr, nbytes)
        elif cmd == '?':
            whystop_handler(cmdstr, nbytes)
        elif cmd == 'D':
            break; # detach
        else:
            unknown_handler(cmdstr, nbytes)

    dbgsocket.close()

def map_data():
        global M4_data
        global M0_data
        global XIP_data
	global M4_file_sz
	global M0_file_sz
	global XIP_file_sz

        try:
            with open("SRAM.BIN", "rb") as M4_file:
                M4_data = M4_file.read()
                M4_file.seek(0,2)
                M4_file_sz = M4_file.tell()
                if M4_file_sz > 704*1024:
                    logging.info("M4 RAM image is unexpectedly large: " + str(M4_file_sz))
        except:
            logging.info("WARNING: No SRAM.BIN file");
            M4_file_sz = 0

        try:
            with open("M0_CLM.BIN", "rb") as M0_file:
                M0_data = M0_file.read()
                M0_file.seek(0,2)
                M0_file_sz = M0_file.tell()
                if M0_file_sz > 192*1024:
                    logging.info("M0 RAM image is unexpectedly large: " + str(M0_file_sz))
        except:
            logging.info("WARNING: No M0_CLM.BIN file");
            M0_file_sz = 0

        try:
            with open("XIP.BIN", "rb") as XIP_file:
                XIP_data = XIP_file.read()
                XIP_file.seek(0,2)
                XIP_file_sz = XIP_file.tell()
                # It's OK to have a larger XiP image, but unusual....and untested
                if XIP_file_sz > 2*1024*1024:
                    logging.info("XIP image is unexpectedly large: " + str(XIP_file_sz))
        except:
            logging.info("WARNING: No XIP.BIN file");
            XIP_file_sz = 0

def process_command_line():
    global dbgport
    global want_debug

    parser = argparse.ArgumentParser(description=dumpserver_description,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)

    parser.add_argument('--debug', '--verbose',
                        required=False,
                        help='Enable debug messages',
                        default=False,
                        action='store_true')

    parser.add_argument('--port',
                        type=int,
                        required=False,
                        default=2331,
                        help='Specify TCP port number that gdb should use to attach')

    args = parser.parse_args()

    want_debug = args.debug
    if os.getenv("DEBUG") or os.getenv("VERBOSE"):
        want_debug = True

    dbgport = args.port

    return
     

# main

process_command_line()

if want_debug:
    logging.basicConfig(stream=sys.stderr, level=logging.DEBUG)
else:
    logging.basicConfig(stream=sys.stderr, level=logging.INFO)

logging.debug("TCP port number is " + str(dbgport));

map_data()
start_dbg_server()
