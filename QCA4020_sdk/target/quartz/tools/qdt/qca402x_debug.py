# !/usr/bin/python
#
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

##################################################################################################################################
# qca402x_debug.py: Tool to decode QDL diag byte stream , from live comport or from a saved session file.
# :params:
#  --session_file :  binary session file (which captured all raw byte stream from COM port)
#            (OR)
#  --port :  Port number for debug messages.

#  --out  :  (Optional) output file to store parsed text output. If not specified StdOUT will be used.

#  Following are the optional arguments to specify dictionary files.
#  --wlan : WLAN dictionary file location with respect to the directory containing this script.
#           (optional - default file is bin/wlan/wlan_fw_dictionary.msc)
#  --apps : Apps processor dictionary file location with respect to the directory containing this script.
#           (optional - default file is bin/cortex-m4/diag_msg_ARNFRI.strdb)
#  --cnss : CNSS processor dictionary file location with respect to the directory containing this script.
#           (optional - default file is bin/cortex-m0/threadx/diag_msg_BRNTRI.strdb)

# Example usage :
#     python qca402x_debug.py --port=COM57

#     For each live logging session this tool creates a session file of type MISF which can be used to parse the stored
#     packets to text at a later time. This filename will represent the data and time of logging.
#     It is advisable to share this misf file instead of text file whenever required as it uses less memory.
#     python qca402x_debug.py --session_file=09072017_185106.misf --out="sample.log"
#
#############################################################################################################################

import os
import sys
import argparse
import time
import serial
import datetime
import struct
from shutil import copyfileobj
from textwrap import TextWrapper
from qca402x_dictionary import qca402x_dictionary
from qca402x_parser import qca402x_parser
import threading

# MISF file header
QUALCOMM_SIGNATURE_LENGTH = 16
TIMESTAMP_LENGTH = 8
OFFSET_FIRST_PACKET = 256
OFFSET_LENGTH = 8
PADDING_BEFORE_DATA = 54

# Debug/DBGLOG/PKTLOG data
DELIMITER_SIZE = 1 #byte
VERSION_SIZE = 1 #byte
PACKET_LENGTH = 2 #bytes

class reader_thread(threading.Thread):
    """ byte reader Class , has methods to help nhdlc decoding , diag header decoding """
    def __init__(self,parent_thread,file_path=None,com_port=None,fout=None):

        threading.Thread.__init__(self)
        self.parent_thread = parent_thread
        self.fout=fout
        self.ser_buf=''
        self.soff   = 0xffffffff
        self.nh_d   = int('7e',16)     ##nhdlc delimiter  == 0x7E == 126
        self.nh_v   = int('01',16)     ##nhdlc version    == 0x1
        self.count  = 1
        self.t0_host = 0
        self.t0_target = 0
        self.qdict = qca402x_dictionary()
        self.logs = None
        self.misf = None


        if file_path is not None:
            if os.path.exists(file_path):
                self.misf = open(file_path,'rb')
                self.mode = 'file'
            else:
                print "Session file doesn't exist\n"
                sys.exit(0)

        elif com_port is not None:
            if com_port.isOpen():
                self.ser = com_port
                self.mode = 'com'
                self.ssn = datetime.datetime.fromtimestamp(time.time()).strftime('%m%d%Y_%H%M%S')
                self.misf = open(self.ssn + '_temp.misf', 'wb+')


    def run(self):
        if self.qdict.create_dictionary() == 0:
            self.fout.write('{:>20} {:<18} {:<14} {:<5} {:<6} {:<16} {:<}\n'.format('Log Index','Timestamp','Type','Proc','Level','Module','Summary'))
            self.process_data()


    def process_data(self):
        if self.mode == 'file':
            self.misf.seek(QUALCOMM_SIGNATURE_LENGTH,0) # 16 bytes signature 

            # Host and target base timestaps from misf file
            base_ts_host = self.misf.read(TIMESTAMP_LENGTH).encode('hex')
            base_ts_target = self.misf.read(TIMESTAMP_LENGTH).encode('hex')
            self.t0_host = float(int(''.join(base_ts_host[i:i+2] for i in range(14,-2,-2)),16))
            self.t0_target = float(int(''.join(base_ts_target[i:i+2] for i in range(14,-2,-2)),16))

            # Offset of first packet in misf file
            offset_str = self.misf.read(OFFSET_LENGTH).encode('hex')
            self.soff = int(''.join(offset_str[i:i+2] for i in range(14,-2,-2)),16)

        # Following call is common for both live port logging and session file mode
        while self.parent_thread.is_alive():
            self.get_next_nhdlc_packet()

        self.exit_gracefully()


    def reader(self,nbytes=None):
        """ returns a string of byte stream :
            * file mode : reads specified nbytes from file object
            * com  mode : reads given number of bytes from serial port
        """
        if (self.mode == 'file') and nbytes is not None:
            rd = self.misf.read(nbytes)
            if rd=='':
                self.exit_gracefully() ## EOF reached ##
            else:
                return rd

        elif self.mode == 'com':
            if len(self.ser_buf) < nbytes:
                remaining_bytes = nbytes
                while(self.parent_thread.is_alive() == True):
                    rd = self.ser.read(remaining_bytes)
                    if self.parent_thread.is_alive():
                        self.ser_buf = self.ser_buf + rd

                        # Input buffer does not have enough bytes to be read
                        if len(rd) < remaining_bytes:
                            remaining_bytes = remaining_bytes - len(rd)
                            continue

                        # Required number of bytes read (nbytes), continue processing.
                        rt_str = self.ser_buf[:nbytes]
                        self.ser_buf = self.ser_buf[nbytes:]
                        self.misf.write(rt_str)
                        self.misf.flush()
                        return (rt_str)
                    else:
                        break

                self.exit_gracefully()


    def get_next_nhdlc_packet(self):
        """ This method decodes the nhdlc header from byte stream returned by reader().
        Looks for nhdlc delimiter, followed by version , length , pay load and delimiter.
        Upon successful identification of nhdlc header and trailer , calls process_diag_packet(payload)
        """
        if self.mode == 'file':
            self.misf.seek(self.soff,0)

        header = int(self.reader(DELIMITER_SIZE).encode('hex'),16)
        while (header != self.nh_d):
            header = int(self.reader(DELIMITER_SIZE).encode('hex'),16)

        version = int(self.reader(VERSION_SIZE).encode('hex'),16)
        while (version != self.nh_v):
            version = int(self.reader(VERSION_SIZE).encode('hex'),16)

        ls     = self.reader(PACKET_LENGTH).encode('hex')
        payload_len      = int(''.join([ls[-2:],ls[:2]]),16)
        pkt_st =  self.reader(payload_len).encode('hex')
        pkt    =  [pkt_st[i:i+2] for i in range(0,payload_len*2,2)]

        delimiter = int(self.reader(DELIMITER_SIZE).encode('hex'),16)
        while (self.nh_d != delimiter):
            delimiter = int(self.reader(DELIMITER_SIZE).encode('hex'),16)

        # While processing first debug message and dbglog/pktlog, t0_host and to_target will be 0
        # respectively. After processing, these will be set to valid values.
        self.logs = None
        dp = qca402x_parser(pkt,self.mode,self.qdict,self.t0_host,self.t0_target)
        self.logs = dp.process_packet()
        self.soff = self.soff + 5 + payload_len
        self.new_packet = 1

        if self.mode == 'com':
            # Get valid values for host and target base timestamps
            if self.t0_host == 0:
                self.t0_host = dp.get_ref_ts_host()
            if self.t0_target == 0:
                self.t0_target = dp.get_ref_ts_target()


        if self.logs is not None:
            self.fout.write(self.print_to_formatted_string(self.logs))
            if self.fout is not sys.stdout:
                self.fout.flush()
                os.fsync(self.fout)


    def print_to_formatted_string(self,dtl):
        rt_str = ''
        for dt in dtl:
            current_str = '{:>20} {:<18} {:<14} {:<5} {:<6} {:<16} '.format(str(self.count),dt['ts'],dt['cmd_code'],
                                                                                dt['proc_id'],dt['log_level'],dt['module_id'])

            wrapper = TextWrapper(initial_indent=current_str, width=150,subsequent_indent=' '*len(current_str))
            msg = wrapper.fill(dt['msg'])
            rt_str = rt_str + msg
            if rt_str[-1] != '\n':
                rt_str = rt_str + '\n'
            self.count += 1
        return rt_str


    def exit_gracefully(self):
        if self.mode == 'com':
            # Ctrl+c received from CLI
            # Write signature, host-target base timestamps, offset of first packet along with data to misf file.
            # In 'file' mode this data is used to parse the same data from saved session.
            self.ser.flushInput()
            self.misf.close()
            temp = open(self.ssn + '_temp.misf', 'rb')
            temp.seek(0,0)
            data = temp.read()
            temp.close()
            os.remove(self.ssn + '_temp.misf')

            # During live port logging, session file is store for later use.
            # This file will have following data in given order:
            # *QUALCOMM__DIAG* signature - 16 bytes
            # Host base timestamp = 8 bytes.
            # Target base timestamp = 8 bytes.
            # Offset of first packet takes 8 bytes and is set to 256.		
            # Padding = 216 bytes = 54 4-byte-words
            # Write above details in session file, copy the collected data from temp file and delete the temp file.
            self.misf = open(self.ssn + '.misf', 'wb')
            self.misf.write('*QUALCOMM__DIAG*')
            self.misf.write(struct.pack('<Q',self.t0_host))
            self.misf.write(struct.pack('<Q',self.t0_target))
            self.misf.write(struct.pack('<Q',OFFSET_FIRST_PACKET)) # Offset of first packet

            for i in range(0,PADDING_BEFORE_DATA,1):
                self.misf.write(struct.pack('<I',0x0))

            self.misf.write(data)
            print "total time: ",time.time()-self.t0_host

        if self.fout is not sys.stdout:
            self.fout.close()
        self.misf.close()
        sys.exit(0)
        

def main(argv):
    parser = argparse.ArgumentParser(description='file_reader.py ')
    parser.add_argument('--session_file', help='Saved session file')
    parser.add_argument('--port', help='COM port; e.g. COM53')
    parser.add_argument('--out', help='Output file path to store text logs')
    parser.add_argument('--wlan', help='WLAN dictionary file')
    parser.add_argument('--apps', help='M4 dictionary file')
    parser.add_argument('--cnss', help='M0 dictionary file')
    parser.add_argument('--user_dictionary', help='User specific dictionary file, can be used for diag messages from application')

    args = vars(parser.parse_args(argv))
    br = None
    if (args['out']):
        fout=open(args['out'],'w')
    else:
        fout=sys.stdout

    # Parsed stored session file to get logs.
    if args['session_file'] and os.path.exists(args['session_file']):
        br = reader_thread(threading.currentThread(),file_path=args['session_file'],fout=fout)

    # Get logs from live port.
    elif args['port']:
        try:
            ser = serial.Serial(args['port'], 115200,timeout=1)
            ser.flushInput()
        except:
            print "Unable to open port "+args['port']
            sys.exit(1)

        br = reader_thread(threading.currentThread(),com_port=ser,fout=fout)
    else:
        parser.print_help()
        return

    if (args['apps']):
        br.qdict.update(0,args['apps'])
    if (args['cnss']):
        br.qdict.update(1,args['cnss'])
    if (args['wlan']):
        br.qdict.update(2,args['wlan'])

    br.start()
    while br.is_alive():
        try:
            time.sleep(1)
        except:
            break

    sys.exit(0)


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
#Done


