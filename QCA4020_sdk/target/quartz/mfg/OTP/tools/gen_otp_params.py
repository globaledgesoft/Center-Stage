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
import base64
import logging
import os
import sys
import binascii
from binascii import hexlify
from collections import defaultdict
   
class OTP_Operator:

    def __init__ (self):
        ''' Initalizes the OTP Operaror'''
        self.firmware_region_write_disable = 0
        self.firmware_region_write_disable_set = 0
        
        self.model_id = 0
        self.model_id_set = 0

        self.pk_hash = 0
        self.pk_hash_set = 0

        self.enc_key = 0
        self.enc_key_set = 0
        
        self.otp_profile = ""
        self.otp_profile_set = 0
        
        return
        
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
            
        otp_descriptor = xml.getroot()
        if otp_descriptor.tag != 'otp_descriptor':
            print "XML doesn't start with the correct tag <otp_descriptor>"
            return 0
            
        for child in otp_descriptor:
            # print '    ', child.tag, child.text
            if child.tag == 'secure_boot_enable':
                self.secure_boot_enable = int(child.text, 0)
                self.secure_boot_enable_set = 1           
            if child.tag == 'firmware_region_write_disable':
                self.firmware_region_write_disable = int(child.text, 0)
                self.firmware_region_write_disable_set = 1          
            if child.tag == 'model_id':
                self.model_id = int(child.text, 0)
                self.model_id_set = 1
            if child.tag == 'pk_hash':
                self.pk_hash = base64.b16decode(child.text, True)
                self.pk_hash_set = 1            
            if child.tag == 'otp_encryp_key':
                self.enc_key = base64.b16decode(child.text, True)
                self.enc_key_set = 1
            if child.tag == 'otp_profile':
                self.otp_profile = child.text
                self.otp_profile_set = 1
        return 1
            
    def check_input(self):
        if self.firmware_region_write_disable_set != 1:
            print "no entry for <firmware_region_write_disable>"
            return 0                       
        if self.model_id_set != 1:
            print "no entry for <model_id>"
            return 0                
        if self.otp_profile_set != 1:
            print "no entry for <otp_profile>"
            return 0    
        if self.otp_profile != "production" and self.otp_profile != "production_test" and self.otp_profile != "development":
           print "otp_profile setting is not correct"
           return 0 
        if self.pk_hash_set == 1  and len(self.pk_hash) != 32:
           print "The length of pk hash is not correct"
           return 0
        if self.enc_key_set == 1 and len(self.enc_key) != 16:
           print "The length if enc key is not correct"
           return 0
        if self.enc_key_set == 0:
           self.enc_key = str(bytearray(16))
        if self.pk_hash_set == 0:
           self.pk_hash = str(bytearray(32))
           
        return 1
        
    def gen_output(self, filename):
        output_file = open(filename, "w+")
        
        output_file.write("#include <stdio.h>\n")
        output_file.write("#include <stdarg.h>\n")

        if self.enc_key_set == 0:
            output_file.write("uint32_t kdf_enable = 0;\n")
        else:
            output_file.write("uint32_t kdf_enable = 1;\n")

        if self.pk_hash_set == 0:
            output_file.write("uint32_t secure_boot_enable = 0;\n")
        else:
            output_file.write("uint32_t secure_boot_enable = 1;\n")

        if self.firmware_region_write_disable == 0:
            output_file.write("uint32_t firmware_region_write_disable = 0;\n")
        else:
            output_file.write("uint32_t firmware_region_write_disable = 1;\n")
        
        output_file.write("uint32_t model_id = " + hex(self.model_id) + ";\n")

        output_file.write("uint8_t pk_hash[] = { ")
        for d in self.pk_hash:
            txt = '0x' + hexlify(d).decode('ascii') + ','
            output_file.write(txt)
        output_file.seek(-1, os.SEEK_CUR)
        output_file.write(" };\n")

        output_file.write("uint32_t pk_hash_size = " + str(len(self.pk_hash)) + ";\n")

        output_file.write("uint8_t enc_key[] = { ")
        for d in self.enc_key:
            txt = '0x' + hexlify(d).decode('ascii') + ','
            output_file.write(txt)
        output_file.seek(-1, os.SEEK_CUR)
        output_file.write(" };\n")

        output_file.write("uint32_t enc_key_size = " + str(len(self.enc_key)) + ";\n")        
        
        if self.otp_profile == "production":
            output_file.write("uint32_t otp_profile = 0;\n")
        elif  self.otp_profile == "production_test":
            output_file.write("uint32_t otp_profile = 1;\n")
        else:
            output_file.write("uint32_t otp_profile = 2;\n")
            
        output_file.close()
        print "OTP params file is generated"
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

    tool_verbose_description = """Tool to convert XML to otp params file. 

Example Usage:
Run: python gen_otp_params.py --xml otp_conf.xml  --output otp_params.c

"""

    parser = argparse.ArgumentParser(description=tool_verbose_description, formatter_class=argparse.RawDescriptionHelpFormatter)
    xml_file= parser.add_argument('--xml', type=file, required=True, help='The xml file for the C header file converter')
    parser.add_argument('--output', type=str, required=True, help='The output file where to store the C header for OTP')
    parser.add_argument('-v', '--verbose', type=int, choices=[0,1,2,3,4,5], help='Verbose levels. Higher numbers include lower. For example, 3 means 3,2,1 and 0. 0=Critcal. 1=Error, 2=Warning 3=Info[Default], 4=Debug, 5=Everything', default=0)	
    try: 
        args = parser.parse_args()
    except IOError as e:
        print "The xml file does NOT exist...."
        exit(1)
    
    log_level = [logging.CRITICAL, logging.ERROR, logging.WARNING, logging.INFO, logging.DEBUG, logging.NOTSET]
    logging.basicConfig(format='[%(asctime)-15s] %(levelname)s: %(message)s File:%(filename)s:%(lineno)d Function:%(funcName)s ', level=log_level[args.verbose])

    #read the intput XML file.
    otp = OTP_Operator()
    if otp.from_xml_file(args.xml.name) != 1:
        logging.error ('entry setting at XML are not correct')
        print 'Failed to generate otp params file'
        exit(1)   
    
    if otp.check_input() != 1:
        logging.error ('entry setting at XML are not correct')
        print 'Failed to generate otp params file'
        exit(1)  
        
    if otp.gen_output(args.output) != 1:
        #failed here
        logging.info('Failed to generate output file')
        print 'Failed to generate otp params file'
        exit(1)        
    return
    
if __name__ == "__main__":
    main()
#    import pdb; pdb.set_trace()

            
            
