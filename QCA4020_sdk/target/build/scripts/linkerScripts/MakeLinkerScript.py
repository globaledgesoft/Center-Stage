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
# Generate a GNU Linker Script (.ld file)
#
# This is used by OEMs in order to create a linker script which places every
# bit of system and application code and data appropriately.
# 
# Inputs:
#   1) List of libraries and object files to be included in linkage, specified on the cmd line
#   2) Template Linker Script including placeholder keywords (see DefaultTemplateLinkerScript.ld)
#   3) Placement File which directs where to place each object section (see PlacementFile.sys)
#
# Output:
#   1) Linker Script to be used to link an ELF executable
#   2) Debug output (if enabled in the environment: DEBUG=1 or VERBOSE=1)
#
# This script places every section of every .o (of every .lib) into an appropriate REGION
# which is in some MEMORY area. Each MEMORY area is in some SEGMENT.
#
# Example:
#   This script might place the .data section of demo.o into the RAM_FOM_APPS_RW_REGION
# which is in the RAM_FOM_APPS_RW_MEMORY area which is part of the RAM_FOM_APPS_DATA_SEGMENT.
# [The other memory area in the RAM_FOM_APPS_DATA_SEGMENT is the RAM_FOM_APPS_ZI_MEMORY area.]
#
# Invocation:
#   python MakeLinkerScript.py TemplateFile PlacementFile [list of .lib and .o files]
# If TemplateFile is "-" then a default Linker Script Template File is used. This
# default Template File is found in the same directory as the MakeLinkerScript.py
# command itself.
#
# Format of a TemplateFile
#   This is intended to be a a linker file for some toolchain (e.g. GNU)
#   with designated placeholders which are replaced in the output. A placeholder
#   consists of a number of Section Attribute flags (see below) bracketed with [].
#   For example, "[RAM FOM APP RW NORMAL]" is used for sections of object files
#   that belong to the RAM_FOM_APPS_RW REGION. Order of section flags in the
#   TemplateFile is important -- it must match the Section Attribute Group ordering.
#
# Format of PlacementFile:
#   This is a list of .lib's, .o's, and specific sections, one per line,
#   followed by space-separated Section Attributes in any order. If a
#   section appears multiple times in PlacementFile, later attributes
#   override earlier attributes.
#   Format of a .lib archive is simply "archive.lib".
#   Format of a .o is "object.o" or "archive.lib:object.o".
#   Format of a section is "object.o(.section)" or "archive.lib:object.o(.section)"
#   Note: Use NO spaces in these section names!
#
# Example PlacementFile:
#   Assume demo.o consists of standard sections .text, .data, .rodata and .bss.
#   This PlacementFile
#      demo.o RAM FOM APP
#      demo.o(.text) SOM
#   results in section placement (as reflected in the Linker Script output):
#      demo.o(.text)    RAM_SOM_APPS_RO
#      demo.o(.data)    RAM_FOM_APPS_RW
#      demo.o(.rodata)  RAM_FOM_APPS_RO
#      demo.o(.bss)     RAM_FOM_APPS_ZI
#
# Note that a single line in the PlacementFile can cause appropriate placement of every
# section of every .o within a library. A subsequent line in the PlacementFile may override
# placement of a particular .o within that library. A subsequent line in the PlacementFile
# may also override placement of a particular section (.text, .data, etc.) within a .o.
#===============================================================================

#===============================================================================
# Design and Operation:
#
# Phase I: Parse and validate command line
#
# Phase II: For each lib/obj specified, use objdump to list sections and extract
# allocatable section names. This allows us to create a complete list of all sections
# in all objects in all libraries that will be part of the linkage.
#
# For each section, parse the PlacementFile in order to determine these attributes:
#  XIP vs. RAM
#  FOM vs. SOM vs. SBL
#  SYS vs. APP
#  RO vs. RW vs. ZI
#  Special flag (see below)
#
# If a section attribute is not specified for a section, we may try to use an
# intelligent DEFAULT based on
#   ELF section flags from the object file
#   name of the section
#   name of the object
#   name of the library
#
# Then we may use heuristics on the library/object/section names to OVERRIDE
# likely problems. (These are somewhat risky since they are based on
# names of things.)
#
# Finally, sensibly RECONCILE conflicts in assigned section attributes.
# For instance, even if a library is marked as "XIP", read/write sections
# within that library are changed to "RAM".
#
# The result of Phase II is to populate a dictionary of canonical section
# name LISTS, keyed by section attributes. Names within each list are roughly of
# the form "archive.lib:obj.o(.section)" or "obj.o(.section)". So, for example, the
# list of names in the ["RAM", "FOM", "APP", "RW", NORMAL] element of the array
# includes a list of all section names that should be placed into the RAM_FOM_APPS_RW
# REGION which is in the RAM_FOM_APPS_RW_MEMORY area of the RAM_FOM_APPS_DATA SEGMENT.
#
# Phase III: The template linker script is parsed and output is produced. Most
# lines are simply reflected directly to the output. Placeholder lines are handled
# specially: A placeholder is replaced by the list of section names from the
# corresponding dictionary entry.
#
# Note that this script doesn't particularly care about the format of the 
# template linker script. It happily copies lines that it doesn't understand
# from the template to the output and it replaces the lines that it knows how
# to replace. This can be useful for debug and it also makes this tool
# toolchain-neutral: It can work with a GNU linker file template or any
# arbitrary file format with placeholders.
#===============================================================================

import sys
import os
import logging
import subprocess
import re
import glob
from collections import defaultdict
from shutil import rmtree

#===============================================================================
# Section Attribute groups
# (and corresponding Section Attributes)
#
# Note that these are Section Attributes that this script knows about. There is
# some overlap between these Section Attributes and the section flags in an
# ELF object file. These are different but related concepts.
#
# Exactly one attribute from each group is applied to every section in order to
# determine placement.
#===============================================================================

# Section Attribute Groups (SAG)
# (See initialize_sag_dict)
SAG_ADDRSPC = 0
SAG_OM      = 1
SAG_ORIGIN  = 2
SAG_PROT    = 3
SAG_SPECIAL = 4
NUM_SECT_ATTR_GRP = 5  # Number of Section Attribute Groups

# Section Attribute Group Dictionary
# Keys are section attributes.
# Values are SAG names.
sag_dict = defaultdict()

def initialize_sag_dict():
   sag_dict["XIP"] = SAG_ADDRSPC
   sag_dict["RAM"] = SAG_ADDRSPC

   sag_dict["FOM"] = SAG_OM
   sag_dict["SOM"] = SAG_OM
   sag_dict["SBL"] = SAG_OM
   sag_dict["AON"] = SAG_OM   

   sag_dict["SYS"] = SAG_ORIGIN
   sag_dict["APP"] = SAG_ORIGIN
   sag_dict["APPS"] = SAG_ORIGIN
   
   sag_dict["RO"]  = SAG_PROT
   sag_dict["RW"]  = SAG_PROT
   sag_dict["ZI"]  = SAG_PROT

   sag_dict["NORMAL"] = SAG_SPECIAL
   sag_dict["RAM_VECTOR_TABLE"] = SAG_SPECIAL
   sag_dict["PATCH"] = SAG_SPECIAL
   sag_dict["INDIR"] = SAG_SPECIAL
   sag_dict["DEVCFG"] = SAG_SPECIAL
   sag_dict["HEAP"] = SAG_SPECIAL
   sag_dict["STACK"] = SAG_SPECIAL
   sag_dict["ROM2RAM"] = SAG_SPECIAL
   sag_dict["ROM2RAM0"] = SAG_SPECIAL
   sag_dict["ROM2RAM1"] = SAG_SPECIAL
   sag_dict["LOGMETA"] = SAG_SPECIAL
   sag_dict["LOGBUF"] = SAG_SPECIAL
   sag_dict["VENEER"] = SAG_SPECIAL
   sag_dict["SBL_VECTOR_TABLE"] = SAG_SPECIAL
   sag_dict["RAMDUMP"] = SAG_SPECIAL
   sag_dict["FIRST"] = SAG_SPECIAL


# Section Dictionary
# Keys are items from each Section Attribute Group, in SAG order.
# Values are LISTS of canonical section names (see canonical_section_name).
section_dict = defaultdict(list)
section_first = "UNKNOWN"

#
# Check for a match between a name (library, object, section) being
# processed and a name (library, object section) in the placement file
#
def name_match(name, plc_name):
   if name == "None" or plc_name == "None":
      return 1

   if os.path.split(name)[1] == os.path.split(plc_name)[1]: # Assume unique library names!
      return 1

   return 0 # No, the plc_name does NOT match the name


#
# Process ELF section flags from one line of an objdump
# of the object file. Initialize the SAG_PROT attribute accordingly.
#
def process_objdump_flags_gcc(line, section_attr):
   is_loadable = re.search(r'^\s*[0-9]+\s+[\w_.]+\s+.* LOAD[\s,$]', line)
   is_code = re.search(r'^\s*[0-9]+\s+[\w_.]+\s+.* CODE[\s,$]', line)
   is_readonly = re.search(r'^\s*[0-9]+\s+[\w_.]+\s+.* READONLY[\s,$]', line)

   if not is_loadable:
      process_placement_flag("ZI", section_attr)
      return

   # is_loadable
   if is_code or is_readonly:
      process_placement_flag("RO", section_attr)
   else:
      process_placement_flag("RW", section_attr)

#
# Process ELF section flags from one line of an objdump
# of the object file. Initialize the SAG_PROT attribute accordingly.
#
def process_objdump_flags_iar(line, section_attr):
   is_nobits = re.search(r'^\s*[0-9]+:\s+[\w_.]+\s+nobits\s+.* WA*', line)
   is_writeable = re.search(r'^\s*[0-9]+:\s+[\w_.]+\s+pbits\s+.* WA*', line)   

   if is_nobits:
      process_placement_flag("ZI", section_attr)
      logging.debug("process_objdump_flags: ZI")
      return

   # is_loadable
   if is_writeable:
      process_placement_flag("RW", section_attr)
      logging.debug("process_objdump_flags: RW")
   else:
      process_placement_flag("RO", section_attr)
      logging.debug("process_objdump_flags: RO")

#
# Process Section Attributes found in a PlacementFile.
#
def process_placement_flag(flag, section_attr):
    if not flag in sag_dict.keys():
      logging.error("Unknown placement flag %s", flag)
      sys.exit(1)
    section_attr[sag_dict[flag]] = flag




#
# Form a fully-qualified "canonical section name" by combining
# the archive name (if specified), the object name and the section name
#
def canonical_section_name(archive_name, object_name, section_name):
   name = object_name + "(" + section_name + ")"
   if archive_name != "":
      name = "*/" + archive_name + ":" + name 
   else:
      name = "*/" + name
   logging.debug("Form canonical name |%s|%s|%s|--> %s", archive_name, object_name, section_name, name)
   return name

#
# Form a fully-qualified "canonical section name" by combining
# the archive name (if specified), the object name only fir IAR
#
def canonical_section_name_iar(archive_name, object_name, section_name, sag_prot_name):
   name = sag_prot_name.lower() + " object      " + object_name + ","
      
   logging.debug("Form canonical name |%s|%s|%s|--> %s", archive_name, object_name, sag_prot_name, name)
   return name
 
#
# Read and parse the Placement File and store its data internally
# as a list of rules in placement_list.
#
def internalize_placement_file():
   global placement_list
   # Parse the PlacementFile and store it as a list
   placement_list = []
   placement_file = open(PlacementFileName, 'r')
   lineno = 0
   for line in placement_file:
      lineno = lineno + 1

      # Process one line of PlacementFile
      # TBD: Permit comments and blank lines
      placement_info = line.split()
      if placement_info[0]:
         logging.debug("In PlacementFile processing line for %s", placement_info[0])

      # Try to parse this line in the PlacementFile
      for one_time in range(1):
         # Fully qualified lib and object and section: "mylib.lib:myobj.o(.mysection)"
         placed  = re.search(r'^\s*([^ ]+)\:([^ ]+)\(([^ ]+)\)', placement_info[0])
         if placed:
            plclib  = placed.group(1)
            plcobj  = placed.group(2)
            plcsect = placed.group(3)
            break;

         # object and section; implicit archive: "myobj.o(.mysection)"
         placed  = re.search(r'^\s*([^ ]+)\(([^ ]+)\)', placement_info[0])
         if placed:
            plclib  = "None"
            plcobj  = placed.group(1)
            plcsect = placed.group(2)
            break;

         # lib and object; all sections: "mylib.lib:mobj.o"
         placed  = re.search(r'^\s*([^ ]+)\:([^ ]+)', placement_info[0])
         if placed:
            plclib  = placed.group(1)
            plcobj  = placed.group(2)
            plcsect = "None"
            break;

         # simple object ending in .o; implicit lib; all sections: "myobj.o"
         placed  = re.search(r'^\s*([^ ]+\.o)', placement_info[0])
         if placed:
            plclib  = "None"
            plcobj  = placed.group(1)
            plcsect = "None"
            break;

         # simple lib ending in .lib; all objects; all sections: "mylib.lib"
         placed  = re.search(r'^\s*([\w_./\\-]+\.lib)', placement_info[0])
         if placed:
            plclib  = placed.group(1)
            plcobj = "None"
            plcsect = "None"
            break;
         # simple lib ending in .a; all objects; all sections: "mylib.a" (ever used?)
         placed  = re.search(r'^\s*([\w_./\\-]+\.a)', placement_info[0])
         if placed:
            plclib  = placed.group(1)
            plcobj = "None"
            plcsect = "None"
            break;
      else:
         logging.error("Unknown line #%d in Placement File %s: %s",  lineno, PlacementFileName, line)
         sys.exit(1)

      plcflags = placement_info[1:]

      placement_list = placement_list + [(plclib, plcobj, plcsect, plcflags)]

   placement_file.close()

#
# Process one section of an object file (possibly of a library)
#
def process_section(archive_name, object_name, section_name, section_attr):
   global placement_list

   archive_default_attr = []
   object_default_attr = []
   global section_first
   
   logging.debug("process_section: ARCHIVE = %s  OBJECT = %s  SECTION = %s",
      archive_name, object_name, section_name)

   # Initialize attributes for this section to UNKNOWN
   for i in range (0, NUM_SECT_ATTR_GRP):
      archive_default_attr.append("UNKNOWN")
      object_default_attr.append("UNKNOWN")

   for placement_rule in placement_list:
      plclib, plcobj, plcsect, plcflags = placement_rule
      # If the current placement rule is relevant to our section, update section attribute info.
      if not name_match(archive_name, plclib):
         continue
      if not name_match(object_name, plcobj):
         continue
      if not name_match(section_name, plcsect):
         continue

      logging.debug("Found placement rule match: plclib=%s plcobj=%s plcsect=%s", plclib, plcobj, plcsect)

      # Parse flags associated with this placement rule
      for pflag in plcflags:
         logging.debug("Placement flag info: %s", pflag)
         #process "FIRST" as special case here
         if pflag == "FIRST":
            section_first = "FIRST"
         else:
            process_placement_flag(pflag, section_attr)




#
# Iterate through all linked objects -- .o files and .lib files
# and call process_section to handle each section within each object.
# 
# The goal is to place every section of every object of every archive
# into the Section Dictionary.
#
def process_all_objects_gcc():
   objectlist_file = open(ObjectListFileName, 'r')

   for line in objectlist_file.readlines():  
      line = line.strip()
      objectlist = line.split(" ")
       
      for objectfile in objectlist:   # Scan through object file names
         objectfile = objectfile.strip('"')
         archive_name=""
         object_name=""
         objdump_output = subprocess.Popen([ObjdumpName,'-hww', objectfile], stdout=subprocess.PIPE)
         while True:
            line = objdump_output.stdout.readline()
            if not line:
               break # continue with next object file
            else:
               # Test for library/archive
               archive = re.search(r'^In archive (.*):.*', line)
               if archive:
                  archive_path, archive_name = os.path.split(archive.group(1))
                  continue
   
               # Test for start of execution object (.o)
               # NB: This line appears in the objdump output for both standalone .o
               # files AND for .o's inside of an archive.
               object = re.search(r'^(.+):\s+file format ', line)
               if object:
                  object_path, object_name = os.path.split(object.group(1))
                  continue
   
               # Test for an allocated section (text, data, bss, rodata, ...)
               alloc_section = re.search(r'^\s*[0-9]+\s+([.\w]+)\s+.*ALLOC', line)
               if alloc_section:
                  section_attr = []
                  global section_first
                  section_first = "UNKNOWN"
                  for i in range (0, NUM_SECT_ATTR_GRP):
                     section_attr.append("UNKNOWN")
                  process_objdump_flags_gcc(line, section_attr)
                  section_name = alloc_section.group(1)
                  logging.debug("Process objdump line for %s:%s: %s", archive_name, object_name, line)
                  process_section(archive_name, object_name, section_name, section_attr)

                  # 
                  # DEFAULT RULES
                  # We have considered section attributes based on the object file
                  # and based on the PlacementFile. If any attribute group has not
                  # been specified, supply an intelligent default based on attributes
                  # that were specified and possibly based on the name of the archive,
                  # object or section.
                  #
                  composite_name = archive_name + object_name + section_name
                  if section_attr[SAG_ADDRSPC] == "UNKNOWN":
                     if section_attr[SAG_PROT] == "RO":
                        process_placement_flag("XIP", section_attr)
                     else:
                        process_placement_flag("RAM", section_attr)
                     logging.debug("Default: ADDRSPC --> %s", section_attr[SAG_ADDRSPC])

                  if section_attr[SAG_OM] == "UNKNOWN":
                     process_placement_flag("FOM", section_attr)
                     logging.debug("Default: OM --> %s", section_attr[SAG_OM])

                  if section_attr[SAG_ORIGIN] == "UNKNOWN":
                     process_placement_flag("APP", section_attr)
                     logging.debug("Default: ORIGIN --> %s", section_attr[SAG_ORIGIN])

                  # SAG_PROT is always set
                  assert section_attr[SAG_PROT] != "UNKNOWN"

                  if section_attr[SAG_SPECIAL] == "UNKNOWN":
                     process_placement_flag("NORMAL", section_attr)
                     logging.debug("Default: SPECIAL --> %s", section_attr[SAG_SPECIAL])



                  #
                  # OVERRIDE RULES
                  # Override some settings based on component names.
                  #

                  # (No overrides)


                  #
                  # RECONCILE RULES
                  # Reconcile seemingly incompatible attributes for a section which
                  # may result from reasonable use of names and reasonable placement files.
                  #
                  if section_name == ".ARM.exidx" :
                     logging.debug("skip SECTION = .ARM.exidx")
                     continue
                  if re.search(r'.ARM.exidx*', section_name) :
                     continue   				  
                  if section_name == ".diagmsg.format" :
                     logging.debug("skip SECTION = .diagmsg.format")
                     continue

                  # Rule: Writeable sections always belong in RAM, not XIP
                  if section_attr[SAG_PROT] == "RW":
                     process_placement_flag("RAM", section_attr)
                     logging.debug("Reconcile: PROT=RW --> RAM");
                  if section_attr[SAG_PROT] == "ZI":
                     process_placement_flag("RAM", section_attr)
                     logging.debug("Reconcile: PROT=ZI --> RAM");


                  # At this point, placement is final. Form a canonical name for the
                  # section and append it to the Section Dictionary
                  canonical_name = canonical_section_name(archive_name, object_name, section_name)

                  if canonical_name in section_dict[
                     section_attr[SAG_ADDRSPC],
                     section_attr[SAG_OM],
                     section_attr[SAG_ORIGIN],
                     section_attr[SAG_PROT],
                     section_attr[SAG_SPECIAL]]:

                     logging.debug("Duplicate canonical_name, skip update database for %s: %s %s %s %s %s %s",
                        canonical_name,
                        section_attr[SAG_ADDRSPC],
                        section_attr[SAG_OM],
                        section_attr[SAG_ORIGIN],
                        section_attr[SAG_PROT],
                        section_attr[SAG_SPECIAL],
                        section_dict[
                           section_attr[SAG_ADDRSPC],
                           section_attr[SAG_OM],
                           section_attr[SAG_ORIGIN],
                           section_attr[SAG_PROT],
                           section_attr[SAG_SPECIAL]])

                  else:
                     if section_first == "FIRST":
                        section_dict[
                           section_attr[SAG_ADDRSPC],
                           section_attr[SAG_OM],
                           section_attr[SAG_ORIGIN],
                           section_attr[SAG_PROT],
                           section_attr[SAG_SPECIAL]].insert(0, canonical_name)
                        
                        logging.debug("Insert database for %s: %s %s %s %s %s %s",
                           canonical_name,
                           section_attr[SAG_ADDRSPC],
                           section_attr[SAG_OM],
                           section_attr[SAG_ORIGIN],
                           section_attr[SAG_PROT],
                           section_attr[SAG_SPECIAL],
                           section_dict[
                              section_attr[SAG_ADDRSPC],
                              section_attr[SAG_OM],
                              section_attr[SAG_ORIGIN],
                              section_attr[SAG_PROT],
                              section_attr[SAG_SPECIAL]])                        
                     else:                  
                        section_dict[
                           section_attr[SAG_ADDRSPC],
                           section_attr[SAG_OM],
                           section_attr[SAG_ORIGIN],
                           section_attr[SAG_PROT],
                           section_attr[SAG_SPECIAL]].append(canonical_name)

                        logging.debug("Append database for %s: %s %s %s %s %s %s",
                           canonical_name,
                           section_attr[SAG_ADDRSPC],
                           section_attr[SAG_OM],
                           section_attr[SAG_ORIGIN],
                           section_attr[SAG_PROT],
                           section_attr[SAG_SPECIAL],
                           section_dict[
                              section_attr[SAG_ADDRSPC],
                              section_attr[SAG_OM],
                              section_attr[SAG_ORIGIN],
                              section_attr[SAG_PROT],
                              section_attr[SAG_SPECIAL]])
   
               else:
                  logging.debug("process_all_objects_gccignores objdump line: %s", line)

def process_app_objects_iar():
   logging.debug("process_app_object_iar: ")
   app_config_file = open(AppConfigFileName, 'r')
   for line in app_config_file.readlines():
      line = line.rstrip()

      for i in range(1):      
         # object and section; implicit archive: "myobj.o(.mysection)"
         placed  = re.search(r'^\s*([^ ]+)\(([^ ]+)\)', line)
         if placed:
            archive_name  = "None"
            object_name  = placed.group(1)
            section_name = placed.group(2)
            section_tag = 1
            break;
         
         # simple object ending in .o; implicit lib; all sections: "myobj.o"
         placed  = re.search(r'^\s*([^ ]+\.o)', line)      
         if placed:
            archive_name  = "None"
            object_name  = placed.group(1)
            section_name = "None"
            section_tag = 0         
            break;

      if placed:
         if section_tag ==0 or (section_tag ==1 and section_name == ".bss"):
            section_first = "UNKNOWN"
            section_attr = []
            for i in range (0, NUM_SECT_ATTR_GRP):
               section_attr.append("UNKNOWN")
            
            process_placement_flag("ZI", section_attr)
            section_name = ".bss"
            process_one_section_iar(archive_name, object_name, section_name, section_attr)               

         if section_tag ==0 or (section_tag ==1 and section_name == ".data"):
            section_first = "UNKNOWN"
            section_attr = []
            for i in range (0, NUM_SECT_ATTR_GRP):
               section_attr.append("UNKNOWN")

            process_placement_flag("RW", section_attr)
            section_name = ".data"
            process_one_section_iar(archive_name, object_name, section_name, section_attr)               

         if section_tag ==0 or (section_tag ==1 and section_name == ".text"):
            section_first = "UNKNOWN"
            section_attr = []
            for i in range (0, NUM_SECT_ATTR_GRP):
               section_attr.append("UNKNOWN")
               
            process_placement_flag("RO", section_attr)
            section_name = ".text"
            process_one_section_iar(archive_name, object_name, section_name, section_attr)
         
         logging.debug("process_app_objects_iar process line: %s", line)
      else:
         logging.debug("process_app_objects_iar ignores line: %s", line)      
   app_config_file.close()
   
def process_one_section_iar(archive_name, object_name, section_name, section_attr):
   for one_time in range(1):
      #
      process_section(archive_name, object_name, section_name, section_attr)

      # 
      # DEFAULT RULES
      # We have considered section attributes based on the object file
      # and based on the PlacementFile. If any attribute group has not
      # been specified, supply an intelligent default based on attributes
      # that were specified and possibly based on the name of the archive,
      # object or section.
      #
      composite_name = archive_name + object_name + section_name
      if section_attr[SAG_ADDRSPC] == "UNKNOWN":
         if section_attr[SAG_PROT] == "RO":
            process_placement_flag("XIP", section_attr)
         else:
            process_placement_flag("RAM", section_attr)
         logging.debug("Default: ADDRSPC --> %s", section_attr[SAG_ADDRSPC])

      if section_attr[SAG_OM] == "UNKNOWN":
         process_placement_flag("FOM", section_attr)
         logging.debug("Default: OM --> %s", section_attr[SAG_OM])

      if section_attr[SAG_ORIGIN] == "UNKNOWN":
         process_placement_flag("APP", section_attr)
         logging.debug("Default: ORIGIN --> %s", section_attr[SAG_ORIGIN])

      # SAG_PROT is always set
      assert section_attr[SAG_PROT] != "UNKNOWN"

      if section_attr[SAG_SPECIAL] == "UNKNOWN":
         process_placement_flag("NORMAL", section_attr)
         logging.debug("Default: SPECIAL --> %s", section_attr[SAG_SPECIAL])

      #
      # OVERRIDE RULES
      # Override some settings based on component names.
      #

      # (No overrides)

      #
      # RECONCILE RULES
      # Reconcile seemingly incompatible attributes for a section which
      # may result from reasonable use of names and reasonable placement files.
      #
      if section_name == ".ARM.exidx" :
         logging.debug("skip SECTION = .ARM.exidx")
         break
      if re.search(r'.ARM.exidx*', section_name) :
         break
      if section_name == ".diagmsg.format" :
         logging.debug("skip SECTION = .diagmsg.format")
         break
      if section_name == "RAM_FOM_BSP_ROM2RAM_INDIRECTION_TABLE_RW_SECTION_0" :
         logging.debug("skip SECTION = RAM_FOM_BSP_ROM2RAM_INDIRECTION_TABLE_RW_SECTION_0")
         break
      if section_name == "RAM_FOM_BSP_ROM2RAM_INDIRECTION_TABLE_RW_SECTION_1" :
         logging.debug("skip SECTION = RAM_FOM_BSP_ROM2RAM_INDIRECTION_TABLE_RW_SECTION_1")
         break
      if section_name == "VECTOR_TABLE_RW_SECTION" :
         logging.debug("skip SECTION = VECTOR_TABLE_RW_SECTION")
         break
               
      # Rule: Writeable sections always belong in RAM, not XIP
      if section_attr[SAG_PROT] == "RW":
         process_placement_flag("RAM", section_attr)
         logging.debug("Reconcile: PROT=RW --> RAM");
      if section_attr[SAG_PROT] == "ZI":
         process_placement_flag("RAM", section_attr)
         logging.debug("Reconcile: PROT=ZI --> RAM");

      # At this point, placement is final. Form a canonical name for the
      # section and append it to the Section Dictionary
      canonical_name = canonical_section_name_iar(archive_name, object_name, section_name, section_attr[SAG_PROT])

      if canonical_name in section_dict[
         section_attr[SAG_ADDRSPC],
         section_attr[SAG_OM],
         section_attr[SAG_ORIGIN],
         section_attr[SAG_PROT],
         section_attr[SAG_SPECIAL]]:

         logging.debug("Duplicate canonical_name, skip update database for %s: %s %s %s %s %s %s",
            canonical_name,
            section_attr[SAG_ADDRSPC],
            section_attr[SAG_OM],
            section_attr[SAG_ORIGIN],
            section_attr[SAG_PROT],
            section_attr[SAG_SPECIAL],
            section_dict[
               section_attr[SAG_ADDRSPC],
               section_attr[SAG_OM],
               section_attr[SAG_ORIGIN],
               section_attr[SAG_PROT],
               section_attr[SAG_SPECIAL]])

      else:
         if section_first == "FIRST":
            section_dict[
               section_attr[SAG_ADDRSPC],
               section_attr[SAG_OM],
               section_attr[SAG_ORIGIN],
               section_attr[SAG_PROT],
               section_attr[SAG_SPECIAL]].insert(0, canonical_name)
                     
            # add canonical_name at first_list   
            first_list.append(canonical_name)   
                     
            logging.debug("Insert database for %s: %s %s %s %s %s %s",
               canonical_name,
               section_attr[SAG_ADDRSPC],
               section_attr[SAG_OM],
               section_attr[SAG_ORIGIN],
               section_attr[SAG_PROT],
               section_attr[SAG_SPECIAL],
               section_dict[
                  section_attr[SAG_ADDRSPC],
                  section_attr[SAG_OM],
                  section_attr[SAG_ORIGIN],
                  section_attr[SAG_PROT],
                  section_attr[SAG_SPECIAL]])            
         else:                  
            section_dict[
               section_attr[SAG_ADDRSPC],
               section_attr[SAG_OM],
               section_attr[SAG_ORIGIN],
               section_attr[SAG_PROT],
               section_attr[SAG_SPECIAL]].append(canonical_name)

            logging.debug("Append database for %s: %s %s %s %s %s %s",
               canonical_name,
               section_attr[SAG_ADDRSPC],
               section_attr[SAG_OM],
               section_attr[SAG_ORIGIN],
               section_attr[SAG_PROT],
               section_attr[SAG_SPECIAL],
               section_dict[
                  section_attr[SAG_ADDRSPC],
                  section_attr[SAG_OM],
                  section_attr[SAG_ORIGIN],
                  section_attr[SAG_PROT],
                  section_attr[SAG_SPECIAL]])


def process_one_object_iar(archive_name, objectfile):
   objdump_output = subprocess.Popen([ObjdumpName,objectfile], stdout=subprocess.PIPE)
   while True:
      line = objdump_output.stdout.readline()
      if not line:
         break # continue with next object file
      else:
         line = line.rstrip()
         # Test for start of execution object (.o)
         # NB: This line appears in the objdump output for both standalone .o
         # files AND for .o's inside of an archive.
         object = re.search(r'^#.*Input file.*', line)
         if object:
            line = objdump_output.stdout.readline()
            if not line:
               break # continue with next object file
            object = re.search(r'^#\s*(\S*)\s', line)
                  
            object_path, object_name = os.path.split(object.group(1))
            continue
   
         # Test for an allocated section (.text, .data, .bss, .rodata, ...)
         #   112: __DLIB_PERTHREAD    pbits     0x9980    0x0                   0x2  A
         alloc_section = re.search(r'^\s*[0-9]+:\s+([.\w]+)', line)
         if alloc_section:
            if len(alloc_section.group(1)) > 19:
               next_line = objdump_output.stdout.readline()
               line = line + next_line.rstrip()
               
            alloc_section = re.search(r'^\s*[0-9]+:\s+([.\w]+)\s+[\w]+\s+[\w]+\s+([\w]+)\s+.*A.*', line)
            if alloc_section and alloc_section.group(2) != '0x0':
               section_attr = []
               global section_first
               section_first = "UNKNOWN"
               for i in range (0, NUM_SECT_ATTR_GRP):
                  section_attr.append("UNKNOWN")
               
               # set SAG_PROT    
               process_objdump_flags_iar(line, section_attr)
               
               section_name = alloc_section.group(1)
               logging.debug("Process objdump line for %s:%s: %s", archive_name, object_name, line)
               process_one_section_iar(archive_name, object_name, section_name, section_attr)               
            else:
               logging.debug("process_one_objects_iar ignores objdump line: %s", line)
         else:
            logging.debug("process_one_objects_iar ignores objdump line: %s", line)


#
# Iterate through all linked objects -- .o files and .lib files
# and call process_section to handle each section within each object.
# 
# The goal is to place every section of every object of every archive
# into the Section Dictionary.
#
def process_all_objects_iar():
   objectlist_file = open(ObjectListFileName, 'r')

   for line in objectlist_file.readlines():  
      line = line.strip()
      objectlist = line.split(" ")
       
      for objectfile in objectlist:   # Scan through object file names
         objectfile = objectfile.strip('"')
         objectfile = os.path.abspath(objectfile)
         
         # test for library/archive 
         archive = re.search(r'.*\.LIB', objectfile.upper())
         if archive:
            logging.debug("process_all_objects_iar lib: %s", objectfile)         
            current_dir = os.getcwd()
            temp_dir = os.path.join(current_dir, "temp")
            archive_path, archive_name = os.path.split(objectfile)
            if os.path.isdir(temp_dir) == True:     
               rmtree(temp_dir)
            os.mkdir(temp_dir)
            os.chdir(temp_dir)
            archive_output = subprocess.call([ArchiveExactName, "-x", objectfile])  
            obj_list = glob.glob(os.path.join(temp_dir, "*.o"))
            for objfile in obj_list:   
               process_one_object_iar(archive_name, objfile)
            os.chdir(current_dir)
            rmtree(temp_dir)
         else:
            logging.debug("process_all_objects_iar obj: %s", objectfile)         
            archive_name=""
            process_one_object_iar(archive_name, objectfile)
   
   # process app objects which are not generated 
   process_app_objects_iar()
   
   
def process_all_objects():
   global first_list
   first_list = []
   
   if tagIAR == 1:
      process_all_objects_iar()
   else:
      process_all_objects_gcc()        

#
# Place a banner into the linker script output stream
#
def create_banner():
   print ""





#
# Read the linker template file, substituting a list from the Section Dictionary
# whenever a placeholder is encountered.
#
def create_linker_file_from_template():
   create_banner()

   template_file = open(TemplateFileName, 'r')
   lineno = 0
   first_section = 1

   for line in template_file:
      lineno = lineno + 1
      logging.debug("Linker template line %d: %s", lineno, line)

      # A bit ugly....works only if there are exactly FIVE Section Attribute Groups.
      assert NUM_SECT_ATTR_GRP == 5
      placeholder = re.search(r'^(\s*)\[(\w+)\s+(\w+)\s+(\w+)\s+(\w+)\s+(\w+)\s*\]', line)
      #                            1      2       3       4       5       6
      if placeholder:
         sect = section_dict[
            placeholder.group(2),
            placeholder.group(3),
            placeholder.group(4),
            placeholder.group(5),
            placeholder.group(6)]

         # Add canonical names to linker output stream
         for section in sect:
            if tagIAR == 1 and section in first_list:
#               print placeholder.group(1) + str("first ") + str(section)
               print placeholder.group(1) + str(section)
            else:
               print placeholder.group(1) + str(section)
            
            # Hack to place COMMON sections along with bss from same object
            is_bss = re.search(r'^(.*)\(.bss\)', str(section))
            if is_bss:
               print placeholder.group(1) + is_bss.group(1) + "(COMMON)"
         first_section = 0
         
      else:
         print("%s" % line), # reflect input to output
         first_section = 1

def define_ObjdumpName():
   if tagIAR == 0:
      define_ObjdumpName_gcc()
      logging.debug("Use objdump command %s", ObjdumpName)
   else:
      define_ObjdumpName_IAR()
      logging.debug("Use objdump command %s", ObjdumpName)

#
# Set ObjdumpName to the name of the ARM objdump command
# Check environment variable $ARM_OBJDUMP. If that's not set, look for
# arm-objdump, arm-none-gnueabi-objdump and arm-linux-gnueabi-objdump.
#
def define_ObjdumpName_gcc():
   global ObjdumpName
   
   objdumps = [os.getenv("ARM_OBJDUMP"), "arm-objdump", "arm-none-gnueabi-objdump", "arm-none-eabi-objdump", "arm-linux-gnueabi-objdump", "arm-linux-eabi-objdump"]
   if objdumps[0] == "None":
      objdumps.pop(0)

   for ObjdumpName in objdumps:
      try:
         objdump_output = subprocess.Popen([ObjdumpName,'--version'], stdout=subprocess.PIPE)
         return
      except:
         objdump_found = False

   logging.error("Cannot find ARM objdump command. Please set $ARM_OBJDUMP.")
   sys.exit(1)

#
# Set ArchiveExactName and ObjdumpName to the name of the IAR archive exact and objdump command
# Check environment variable $IARCHIVE and $IELFDUMPARM. 
# If that's not set, report error and exit
#
def define_ObjdumpName_IAR():
   global ArchiveExactName
   global ObjdumpName

   iarchives = [os.getenv("IARCHIVE"), "iarchive.exe"]
   if iarchives[0] == "None":
      iarchives.pop(0)

   for ArchiveExactName in iarchives:
      iarchive_found = True
      try:
         iarchives_output = subprocess.Popen([ArchiveExactName], stdout=subprocess.PIPE)
         break
      except:
         iarchive_found = False
         pass
   if iarchive_found == False:
      logging.error("Cannot find IAR iarchive command. Please set $IARCHIVE.")
      sys.exit(1)
      
   objdumps = [os.getenv("IELFDUMPARM"), "ielfdumparm.exe"]
   if objdumps[0] == "None":
      objdumps.pop(0)

   for ObjdumpName in objdumps:
      objdump_found = True
      try:
         objdump_output = subprocess.Popen([ObjdumpName], stdout=subprocess.PIPE)
         break
      except:
         objdump_found = False
         pass
   if objdump_found == False:
      logging.error("Cannot find IAR achive exact command. Please set $IELFDUMPARM.")
      sys.exit(1)
         
#
# Parse and validate the command line
#
def process_command_line():
   global TemplateFileName
   global PlacementFileName
   global ObjectListFileName
   global AppConfigFileName
   global tagIAR
   
   if len(sys.argv) < 4:
      print "Usage: ", str(sys.argv[0]), "TemplateFile PlacementFile ObjectListFile [iar] [AppConfigFile]"
      sys.exit(1)
   else:
      TemplateFileName = str(sys.argv[1])
      PlacementFileName = str(sys.argv[2])
      ObjectListFileName = str(sys.argv[3])
  
      if len(sys.argv) == 6 and str(sys.argv[4]) == "iar":
         tagIAR = 1
         AppConfigFileName = str(sys.argv[5])
      else:
         tagIAR = 0      

      if TemplateFileName == "-":
         TemplateFileName = os.path.join(os.path.split(sys.argv[0])[0], "DefaultTemplateLinkerScript.ld")
   
      if logging.getLogger().isEnabledFor(logging.DEBUG):
         logging.debug("TemplateFile: %s", TemplateFileName)
         logging.debug("PlacementFile: %s", PlacementFileName)
         logging.debug("ObjectListFile: %s", ObjectListFileName)


# main
if os.getenv("DEBUG") or os.getenv("VERBOSE"):
   logging.basicConfig(stream=sys.stderr, level=logging.DEBUG)
else:
   logging.basicConfig(stream=sys.stderr, level=logging.INFO)
   
logging.debug("Argument List: %s", str(sys.argv))

process_command_line()
define_ObjdumpName()
initialize_sag_dict()
internalize_placement_file()
process_all_objects()
create_linker_file_from_template()
