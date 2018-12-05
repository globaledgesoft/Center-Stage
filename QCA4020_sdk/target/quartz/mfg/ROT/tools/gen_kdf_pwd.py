#!/usr/bin/python
# Copyright (c) 2016-2018 Qualcomm Technologies, Inc.
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

import struct
import xml.etree.ElementTree as ET
import xml.dom.minidom as minidom
import math
import hashlib
import hmac
import base64
import logging
import os
import sys
import binascii
      
class KDF_Operator:
    KDF_HEADER = '<BB'
    kdf_packed = struct.Struct(KDF_HEADER)
	
    def __init__ (self):
        ''' Initalizes the KDF Operaror'''
        self.op_code = 0
        self.oem_id = 0       
        self.model_id = 0
        self.otp_encryp_key = bytearray([0x5A]*16)
        self.dbg_enable = 0
        self.sw_input = bytearray([0x5A]*16)
        self.oem_enc_wrap_key = bytearray([0x5A]*32)
        self.iteration = 1
        self.mid = 0x5a
        self.otp_hw_key = bytearray([0x5A]*32)
        self.otps_regs = bytearray([0x5A]*8)
        return
        
    def check_parameters(self):
        if (self.op_code != 7 ) and (self.op_code != 8) and (self.op_code != 10 )  :
            print 'Op code is invalid number:', self.op_code 
            return 0
        elif len(self.sw_input) != 16 :
            print 'sw_input must be 16 bytes long' 
            return 0
        elif len(self.oem_enc_wrap_key) != 32 :
            print 'enc wrap key must be 32 bytes long' 
            return 0
        return 1

    def from_xml_file (self, xml_file):
        ''' Parses the XML Root from an ElementTree
        '''
        # Clear any pre-existing data.
        self.__init__()

        try:
            xml = ET.parse(xml_file)
        except :
            print "XML format error, check XML file"
            return 0
            
        kdf_descriptor = xml.getroot()
        if kdf_descriptor.tag != 'kdf_descriptor':
            print "XML didn't start with the correct tag <kdf_descriptor>"
            return 0
        print 'Input from XML file:' 
        for child in kdf_descriptor:
            print '    ', child.tag, child.text
            if child.tag == 'op_code':
                self.op_code = int(child.text, 0)
            if child.tag == 'oem_id':
                self.oem_id = int(child.text, 0)
            if child.tag == 'model_id':
                self.model_id = int(child.text, 0)
            if child.tag == 'mid':
                self.mid = int(child.text, 0)
            if child.tag == 'otp_encryp_key':
                self.otp_encryp_key = base64.b16decode(child.text, True)
            if child.tag == 'dbg_enable':
                self.dbg_enable = int(child.text, 0)
            if child.tag == 'sw_input':
                self.sw_input = base64.b16decode(child.text, True)
        return 1

    def set_wrap_key(self, wrap_key):
        if self.dbg_enable == 1:
            key = bytearray([0x5A]*16)
            data = self.kdf_packed.pack( ( (self.iteration & 0x03) << 6 | 0x03), 0x5a)
            data = data + bytearray([0x5A]*48) + bytearray([(self.oem_id >> 8) & 0xff ,self.oem_id & 0xff]) + bytearray([0x5A]*6)
            signature = hmac.new(key, data, digestmod=hashlib.sha256)
            self.oem_enc_wrap_key = base64.b16decode(signature.hexdigest(), True) 
        else:
            if wrap_key is None:
                print 'Must specify enc wrap key on command line when dbg_enable is 0' 
                return 0
            else:
                self.oem_enc_wrap_key = base64.b16decode(wrap_key, True)
        return 1
        
    def set_otp_hw_key(self):
        if self.dbg_enable  == 1:
            self.otp_hw_key = bytearray([0x00]*16) + bytearray([0x5A]*16) 
        else:
            self.otp_hw_key = bytearray([0x00]*16) + self.otp_encryp_key  
            
    def do_kdf(self, wrap_key):
        #set wrap key based in input
        if self.set_wrap_key(wrap_key) != 1:
            logging.error ('Must specify enc wrap key on command line when dbg_enable is 0.')
            print 'Failed to generate KDF pwd due to enc wrap key'
            return 0
    
        #check input parameters
        if self.check_parameters() != 1:
            logging.error ('parameters at XML are not correct')
            print 'Failed to generate KDF pwd due to parameter error at XML file'
            return 0
                    
        self.iteration = 2
        self.set_otp_hw_key()
        
        self.otps_regs = bytearray([0x5A]*8)
        if self.op_code == 7:
            self.mid = 0x5a
            self.otps_regs[0] = (self.oem_id >> 8) & 0xff
            self.otps_regs[1] = self.oem_id & 0xff
            self.otps_regs[4] = (self.model_id >> 8) & 0xff
            self.otps_regs[5] = self.model_id & 0xff
        elif self.op_code == 8:
            self.mid = 0x5a
            self.otps_regs[0] = (self.oem_id >> 8) & 0xff
            self.otps_regs[1] = self.oem_id & 0xff
            self.otps_regs[4] = (self.model_id >> 8) & 0xff
            self.otps_regs[5] = self.model_id & 0xff
        elif self.op_code == 10:
            self.mid = self.mid
            self.otps_regs[4] = (self.model_id >> 8) & 0xff
            self.otps_regs[5] = self.model_id & 0xff
        else:
            print "Invalid Op Code %d", self.op_code
            return 0            
        
        data = self.kdf_packed.pack( ( (self.iteration & 0x03) << 6 | (self.op_code & 0x3f)), self.mid)
        data = data + self.otp_hw_key + self.sw_input + self.otps_regs
        #Calculate HMAC
        signature = hmac.new(self.oem_enc_wrap_key, data, digestmod=hashlib.sha256)
        self.pwd = base64.b16decode(signature.hexdigest(), True)
        self.pwd = self.pwd[32-8:32]
        print
        print 'Gen KDF password is done'
        print "Password: " + binascii.hexlify(self.pwd)
        return 1
       
    def gen_output(self, filename):
        ''' Saving password to output file'''
        output_file = open(filename, "w+")
        output_file.write("Password:" + binascii.hexlify(self.pwd))
        output_file.write("\n")
        output_file.close()
        print "Output file is generated"
        return 1
        
def main():

    # Give a version number to this script.
    script_version = '1.0'

    # Check the version, since some changes to argparse on Python 3 causes it to fail.
    if sys.version_info[0] >= 3:
        logging.critical ("This tool won't run on Python 3, please use Python 2.7")
        return

    if sys.version_info[0] <= 2 and sys.version_info[1] < 7:
        logging.warning ("This tool requires on Python 2.7. Most systems have python 2.7 installed, but it is not part of the PATH")
        return

    import argparse

    tool_verbose_description = """Tool to generate KDF password. 

Example Usage:
Run: python gen_kdf_key.py --xml kdf_conf.xml  --wrap_key wrap_key [--output kdf_pwd.txt]

"""

    parser = argparse.ArgumentParser(description=tool_verbose_description, formatter_class=argparse.RawDescriptionHelpFormatter)
    xml_file= parser.add_argument('--xml', type=file, required=True, help='The xml file for the KDF password generater')
    parser.add_argument('--wrap_key', type=str, required=False, help='The enc wrap key')
    parser.add_argument('--output', type=str, required=False, help='The output file where to store the KDF password')
    parser.add_argument('-v', '--verbose', type=int, choices=[0,1,2,3,4,5], help='Verbose levels. Higher numbers include lower. For example, 3 means 3,2,1 and 0. 0=Critcal. 1=Error, 2=Warning 3=Info[Default], 4=Debug, 5=Everything', default=0)	
    try: 
        args = parser.parse_args()
    except IOError as e:
        print "The xml file does NOT exist...."
        return
    
    log_level = [logging.CRITICAL, logging.ERROR, logging.WARNING, logging.INFO, logging.DEBUG, logging.NOTSET]
    logging.basicConfig(format='[%(asctime)-15s] %(levelname)s: %(message)s File:%(filename)s:%(lineno)d Function:%(funcName)s ', level=log_level[args.verbose])

    #read the intput XML file.
    kdf = KDF_Operator()
    if kdf.from_xml_file(args.xml.name) != 1:
        logging.error ('entry setting at XML are not correct')
        print 'Failed to generate the KDF key'
        return    
    
    #Generate the password
    if kdf.do_kdf(args.wrap_key) != 1:
        #failed here
        logging.info('Failed to generate kdf key')
        print 'Failed to generate kdf key'

    if args.output is not None:
        if kdf.gen_output(args.output) != 1:
            #failed here
            logging.info('Failed to generate output file')
            print 'Failed to generate output file'       
    return
    
if __name__ == "__main__":
    main()
#    import pdb; pdb.set_trace()
