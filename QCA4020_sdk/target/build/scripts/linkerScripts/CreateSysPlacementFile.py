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
# Create a PlacementFile for SYSTEM modules which can be used as input
# to MakeLinkerScript. This runs as part of the SDK build; it is
# NOT part of the SDK that is shipped to OEMs.
#
# Inputs:
#   1) Path to system libraries (.lib) and object files (.o).
#   2) Name of "scatterInput.txt" file, specified on the command line.
#      The scatterInput.txt file is an artifact of an earlier phase of
#      a formal build of system software. CreateSysPlacementFile is
#      typically used during a subsequent phase of the build.
#
# Output:
#   1) Contents of system placement file. The system placement file is
#      distributed as part of the OEM SDK where it is used during the
#      OEM's build process to place system modues.
#   2) Debug output (if enabled)
#===============================================================================

import sys
import os
import logging
import subprocess
import re
import string
import glob
from collections import defaultdict

#
# Default attributes to be used for system libraries and objects
# that are not explicitly placed.
#
DEFAULT_ATTRIBUTES = "XIP FOM SYS NORMAL"

plc_dict = defaultdict(list)

#
# Convert a placement such as XIP_BSP_RO_REGION to a list of
# section attributes such as ["XIP", "BSP", "RO"].
#
def convert_placement_to_attributes(plc_attrs):
   filtered_attrs = ["SYS"]
   logging.debug("Convert %s", plc_attrs)

   for attr in plc_attrs:
      if attr == "XIP":
         filtered_attrs.append("XIP")
      elif attr == "RAM":
         filtered_attrs.append("RAM")
      elif attr == "FOM":
         filtered_attrs.append("FOM")
      elif attr == "SOM":
         filtered_attrs.append("SOM")
      elif attr == "AON":
         filtered_attrs.append("AON")
      elif attr == "MOM":
         filtered_attrs.append("AON")
      elif attr == "SBL":
         filtered_attrs.append("SBL")
      elif attr == "DEVCFG":
         filtered_attrs.append("DEVCFG")
      elif attr == "BSP":
         continue # ignore
      elif attr == "OEM":
         continue # ignore
      elif attr == "APPS":
         filtered_attrs.append("APPS")
         filtered_attrs.remove("SYS")
      elif attr == "PATCHROM":
         filtered_attrs.append("PATCH")
      elif attr == "PATCHTABLE":
         filtered_attrs.append("PATCH")
#        filtered_attrs.append("SYS") # ignore claim of APPS in system software -- broken
      elif attr == "RO":
         continue # handled by MakeLinkerScript
      elif attr == "+RO":
         continue # handled by MakeLinkerScript
      elif attr == "+RO,":
         continue # handled by MakeLinkerScript
      elif attr == "RW":
         continue # handled by MakeLinkerScript
      elif attr == "+RW":
         continue # handled by MakeLinkerScript
      elif attr == "ZI":
         continue # handled by MakeLinkerScript
      elif attr == "+ZI":
         continue # handled by MakeLinkerScript
      elif attr == "DATA":
         continue # ignore
      elif attr == "ROM":
         continue # ignore
      elif attr == "REGION":
         continue # ignore
      elif attr == "+FIRST":
         filtered_attrs.append("FIRST")
      elif attr == "+RO,+FIRST":
         filtered_attrs.append("FIRST")
      elif attr == "QUARTZ_SBL_ENTRY":
         continue # ignore
      elif 0 <= string.find(attr, "HASH"):
         continue # ignore
      elif 0 <= string.find(attr, "RAMDUMP"):
         filtered_attrs.append("RAMDUMP")
      else:
         logging.debug("Error: Unhandled placement directive in ScatterFile: %s", attr)
      
   return filtered_attrs


#
# Read each line from the scatter file and create a dictionary
# of section attribute keywords associated with each object.
#
def consume_scatter_file():
   scatter_file = open(ScatterFileName, 'r')
   lineno = 0
   for line in scatter_file:
      lineno = lineno + 1

      scatter_info = line.split()
      if len(scatter_info) < 3:
         logging.debug("In ScatterFile skipping %s", line)
         continue
      if re.search(r'^CORE_', scatter_info[0]):
         logging.debug("In ScatterFile skipping %s", line)
         continue
#      if re.search(r'_IPT', scatter_info[0]):
#         logging.debug("In ScatterFile skipping %s", line)
#         continue
      obj = scatter_info[1]
      if obj:
         logging.debug("In ScatterFile processing %s", line)

         # Add all appropriate placement keywords to the data for this object
         attrs = string.split(scatter_info[2], "_") + scatter_info[3:]
         if attrs[-2][-1] == ",":
            plc_dict[obj].append(convert_placement_to_attributes(attrs[0:-2]))

            # Add a section with special flags
            logging.debug("In ScatterFile processing, special line %s" % line)
            section = obj + "(" + attrs[-2][:-1] + ")"
            plc_dict[section].append(convert_placement_to_attributes(attrs[0:-2]))
            plc_dict[section].append(convert_placement_to_attributes([attrs[-1]]))
         else:
            plc_dict[obj].append(convert_placement_to_attributes(attrs))



#
# Set ObjdumpName to the name of the ARM objdump command
# Check environment variable $ARM_OBJDUMP. If that's not set, look for
# arm-objdump, arm-none-gnueabi-objdump and arm-linux-gnueabi-objdump.
#
def define_ObjdumpName():
   global ObjdumpName

   objdumps = [os.getenv("ARM_OBJDUMP"), "arm-objdump", "arm-none-gnueabi-objdump", "arm-linux-gnueabi-objdump"]
   if objdumps[0] == "None":
      objdumps.pop(0)

   for ObjdumpName in objdumps:
      try:
         objdump_output = subprocess.Popen([ObjdumpName,'--version'], stdout=subprocess.PIPE)
         return
      except:
         objdump_found = False

   logging.error("Cannot find ARM objdump command. Please set $ARM_OBJDUMP.")
   exit(1)



def defaults_for_lib(lib):
      define_ObjdumpName()
      objdump_output = subprocess.Popen([ObjdumpName,'-hww', lib], stdout=subprocess.PIPE)
      while True:
         line = objdump_output.stdout.readline()
         if not line:
            return

         # Test for start of execution object (.o)
         object = re.search(r'^(.+):\s+file format ', line)
         if not object:
            continue

         object_path, object_name = os.path.split(object.group(1))
         if object_name in plc_dict.keys():
            continue # already explicitly handled

         libname = os.path.split(lib)[1]
         lib_list_need_default.append(libname)
         logging.debug("DEFAULT placement for library %s due to %s", libname, object_name)

         # There may be additional objects in this library that require defaults;
         # but all it takes is one and the entire library is added to the output stream.
         return

   


def defaults_for_obj(obj):
   object_path, object_name = os.path.split(obj)
   if object_name in plc_dict.keys():
      return # already explicitly handled

   obj_list_need_default.append(object_name)
   logging.debug("DEFAULT placement for object %s", object_name)




#
# Provide DEFAULT attributes for every object in the QC system library
# that was not explicitly covered by the scatterInput file.
#
def provide_default_attributes():
   global lib_list_need_default
   global obj_list_need_default
   lib_list_need_default = []
   obj_list_need_default = []

   lib_list = glob.glob(os.path.join(SysLibPath, "*.lib"))
   for lib in lib_list:
      defaults_for_lib(lib)

   obj_list = glob.glob(os.path.join(SysLibPath, "*.o"))
   for obj in obj_list:
      defaults_for_obj(obj)



#
# Create the placement file, starting with libraries and then objects
# that use default attributes followed by explicit placements.
#
def create_system_placement_file():
   for lib in lib_list_need_default:
      print lib, DEFAULT_ATTRIBUTES

   for obj in obj_list_need_default:
      print obj, DEFAULT_ATTRIBUTES

   for obj in plc_dict:
      flattened_list = [ y for x in plc_dict[obj] for y in x]
      plc_dict[obj] = dict.fromkeys(flattened_list).keys() # elim dup attrs

      # If both XIP and RAM attributes are present, mark this object as XIP.
      # Writable sections are automatically placed RAM.
      if "XIP" in plc_dict[obj]:
         if "RAM" in plc_dict[obj]:
            plc_dict[obj].remove("RAM")

      print obj, " ".join(plc_dict[obj])

#
# Parse and validate the command line
#
def process_command_line():
   global SysLibPath
   global ScatterFileName
   
   argv_num = len(sys.argv)
   if not (argv_num == 3):
      print "Usage: ", str(sys.argv[0]), "PathToSystemLibs ScatterFile"
      sys.exit(1)
   else:
      SysLibPath = str(sys.argv[1])
      logging.debug("System Library Path: %s", SysLibPath)

      if os.path.split(sys.argv[2])[0] == "":
         # If just the last component of a filename was specified
         # then check if there is a local file with that name. If
         # not, then look for that filename in the same directory
         # as libraries.
         ScatterFileName = str(sys.argv[2])
         if not os.path.exists(ScatterFileName):
            ScatterFileName = os.path.join(SysLibPath, ScatterFileName)
      else:
         ScatterFileName = str(sys.argv[2])
   
      logging.debug("ScatterFile: %s", ScatterFileName)
      
# main
if os.getenv("DEBUG") or os.getenv("VERBOSE"):
   logging.basicConfig(stream=sys.stderr, level=logging.DEBUG)
else:
   logging.basicConfig(stream=sys.stderr, level=logging.INFO)


logging.debug("Argument List: %s", str(sys.argv))

process_command_line()
consume_scatter_file()
provide_default_attributes()
create_system_placement_file()
