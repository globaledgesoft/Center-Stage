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
# Update the DefaultTemplateLinkerScript.ld file.
# The main purpose of this script is to update the definitions of MEMORY areas.
# This script is not shipped to OEMs as part of the standard SDK; but it is executed
# as part of a standard build of the SDK.
#
# Inputs:
#   1) Current DefaultTemplateLinkerScript.ld file
#   2) A .scl file, generated during an earlier phase of a formal build.
#      (e.g. build/bsp/ioe_ram_img/build/v2/ARNFRI/IOE_RAM_IMG_ARNFRI.scl)
#
# Output:
#   1) Contents of the updated DefaultTemplateLinkerScript.ld file
#   2) Debug output (if enabled)
#===============================================================================

import sys
import os
import logging
import re
import string
import glob
from collections import defaultdict


#
# Key is name of memory area (or name of region from .scl file).
# Value is origin/length list.
mem_area_dict = defaultdict(list)
mem_segment_dict = defaultdict(list)

#
# Replace each parenthesized expression with an equivalent simple value
def normalize_line(my_line):
   new_line=""
   paren_count = 0
   for i, c in enumerate(my_line):
      if paren_count == 0:
         if c != '(':
            new_line += c
         else:
            # Found the start of a parenthesized expression.
            start_paren = i
            paren_count = 1
      else:
         if c == '(':
            paren_count = paren_count + 1
         elif c == ')':
            paren_count = paren_count - 1
            if paren_count == 0:
               # Reached the end of a parenthesized expression.
               new_line += str(hex(eval(my_line[start_paren:i+1])))
               
   return new_line

def consume_SCL_file():
   scl_file = open(SCLName, 'r')
   lineno = 0

   for line in scl_file:
      lineno = lineno + 1

      region_specification = re.search(r'\s*([\w]+)_REGION(_[0-9])*\s', line)
      if region_specification != None:
         # Parse REGION lines from the SCL file
         name = region_specification.group(1)
         if None != region_specification.group(2):
             name = name + region_specification.group(2)
         tokens = str.split(normalize_line(line))
         tokens = [t for t in tokens[1:] if t[0:2] == "0x"]

         if len(tokens) > 0:
            origin = int(tokens[0], 0)
         else:
            origin = 0

         if len(tokens) > 1:
            length = int(tokens[1], 0)
         else:
            length = 0

         if origin == 0 and last_name in mem_area_dict.keys():
            origin = mem_area_dict[last_name][0] + mem_area_dict[last_name][1]
            
         logging.debug("Region: %s 0x%x 0x%x", name, origin, length)
         mem_area_dict[name] = [origin, length, last_segment_origin]
        
         last_name = name

         continue

      segment_specification = re.search(r'\s*([\w]+)_SEGMENT*\s', line)
      if segment_specification != None:
         # Parse SEGMENT lines from the SCL file
         name = segment_specification.group(1)
         tokens = str.split(normalize_line(line))
         tokens = [t for t in tokens[1:] if t[0:2] == "0x"]

         if len(tokens) > 0:
            origin = int(tokens[0], 0)
         else:
            origin = 0

         if len(tokens) > 1:
            length = int(tokens[1], 0)
         else:
            length = 0

         logging.debug("Segment: %s 0x%x 0x%x", name, origin, length)
         last_segment_origin = origin
         mem_segment_dict[name] = [origin, length]

      section_specification = re.search(r'\s*([\w]+)_SECTION*\s', line)
      if section_specification != None:
         # Parse SECTION lines from the SCL file -- only sections that are actually REGIONS
         name = section_specification.group(1)
         tokens = str.split(normalize_line(line))
         tokens = [t for t in tokens[1:] if (t[0:2] == "0x") or (t[0:3] == "+0x") or t.isdigit()]

         if len(tokens) > 0:
            origin = int(tokens[0], 0)
         else:
            origin = 0

         if len(tokens) > 1:
            length = int(tokens[1], 0)
         else:
            length = 0

         if origin == 0 and last_name in mem_area_dict.keys():
            origin = mem_area_dict[last_name][0] + mem_area_dict[last_name][1]
            
         logging.debug("Section: %s 0x%x 0x%x", name, origin, length)
         mem_area_dict[name] = [origin, length, last_segment_origin]
        
         last_name = name
         continue


def create_default_linker_script_file_gcc():
   dls_file = open(DefaultLinkerScriptName, 'r')
   lineno = 0
   for line in dls_file:
      lineno = lineno + 1

      mem_spec = re.search(r'(\s)*([^\s]*)_MEMORY(.*)ORIGIN = (.*), LENGTH = (.*).*', line)
      if mem_spec:
         name = mem_spec.group(2)
         if name in mem_area_dict.keys():
            origin = mem_area_dict[name][0]
            length = mem_area_dict[name][1]
            seg_origin = mem_area_dict[name][2]
            if origin == 0: # hack
               origin = seg_origin
            print("%sORIGIN = 0x%x, LENGTH = 0x%x" % (
               mem_spec.group(1) + name + "_MEMORY" + mem_spec.group(3),
               origin,
               length))
         elif name in mem_segment_dict.keys():
            origin = mem_segment_dict[name][0]
            length = mem_segment_dict[name][1]
            print("%sORIGIN = 0x%x, LENGTH = 0x%x" % (
               mem_spec.group(1) + name + "_MEMORY" + mem_spec.group(3),
               origin,
               length))
         else:        
            logging.error("line no: %d missing memory name %s", lineno, name)
            exit(1)

      else:
         print("%s" % line), # reflect input to output
      
def create_default_linker_script_file_iar():
   dls_file = open(DefaultLinkerScriptName, 'r')
   lineno = 0
   for line in dls_file:
      lineno = lineno + 1

      mem_spec = re.search(r'define\s*region\s*([^\s]*)_MEMORY(.*)mem\:\[from(.*) to (.*)\];.*', line)
      if mem_spec:
         name = mem_spec.group(1)
         if name in mem_area_dict.keys():
            origin = mem_area_dict[name][0]
            length = mem_area_dict[name][1]
            seg_origin = mem_area_dict[name][2]
            if origin == 0: # hack
               origin = seg_origin
            print("define region %smem:[from 0x%x to 0x%x];" % (
               name + "_MEMORY" + mem_spec.group(2),
               origin, 
               origin+length))
         elif name in mem_segment_dict.keys():
            origin = mem_segment_dict[name][0]
            length = mem_segment_dict[name][1]
            print("define region %smem:[from 0x%x to 0x%x];" % (
               name + "_MEMORY" + mem_spec.group(2),
               origin,
               origin+length))
         else:        
            logging.error("line no: %d missing memory name %s", lineno, name)
            exit(1)
      else:
         print("%s" % line), # reflect input to output

def create_default_linker_script_file():         
   if forIAR == 0:
      create_default_linker_script_file_gcc()
   else:
      create_default_linker_script_file_iar()

#
# Parse and validate the command line
#
def process_command_line():
   global DefaultLinkerScriptName
   global SCLName
   global forIAR

   if len(sys.argv) == 3 or (len(sys.argv) == 4 and sys.argv[3] == 'iar'):
      DefaultLinkerScriptName = str(sys.argv[1])
      logging.debug("DefaultLinkerScriptName: %s", DefaultLinkerScriptName)
      SCLName = str(sys.argv[2])
      logging.debug("SCLName: %s", SCLName)
      if len(sys.argv) == 3:
         logging.debug("Update GCC DTLS")
         forIAR = 0
      else:
         logging.debug("Update IAR DTLS")
         forIAR = 1	  
   else:
      print "Usage: ", str(sys.argv[0]), "DefaultLinkerScriptName sclName [iar]"
      sys.exit(1)


# main
if os.getenv("DEBUG") or os.getenv("VERBOSE"):
   logging.basicConfig(stream=sys.stderr, level=logging.DEBUG)
else:
   logging.basicConfig(stream=sys.stderr, level=logging.INFO)
  
logging.debug("Argument List: %s", str(sys.argv))
process_command_line()
consume_SCL_file()
create_default_linker_script_file()
