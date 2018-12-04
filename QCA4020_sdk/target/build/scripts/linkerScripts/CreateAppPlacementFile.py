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
# Create a Application PlacementFile for Applicatio modules which can be used as input
# to MakeLinkerScript. This runs as part of the App build; it is
#
# Inputs:SysPlacementFile AppConfigFile AppPlacementFile
#   1) Name of "sys.placement" file, specified on the command line.
#      The sys.placement file is created by CreateSysPlacementFile.py. It includes
#      all sys placement info
#   2) Name of "app.config" file, specified on the command line.
#      The app.config file is defined at application. It includes all 
#      application placement info
#   3) Name of "app.placement" file, specified on the command line.
#      This is the output file name which store application placement info.
# Output:
#   1) Contents of application placement file. The file is used to generate ld file.
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
# generate application placement file 
#
def create_application_placement_file():
   global SysPlacementFileName
   global CustPlacementFileName   
   global AppConfigFileName
   global AppPlacementFileName

   logging.debug("Create Application Placement File")
   app_placement_file = open(AppPlacementFileName, 'w')
   
   sys_placement_file = open(SysPlacementFileName, 'r')
   lineno = 0
   # copy sys placement file to application placement file
   for line in sys_placement_file:
      lineno = lineno + 1
      logging.debug("Sys placement file line %d: %s", lineno, line)
      if len(line.split()) < 3:
         logging.debug("In SysPlacementFile skipping %s", line)
         continue
      app_placement_file.write(line)
   
   # close sys placement file 
   sys_placement_file.close()

   cust_placement_file = open(CustPlacementFileName, 'r')
   lineno = 0
   # copy cust placement file to application placement file
   for line in cust_placement_file:
      lineno = lineno + 1
      logging.debug("Cust placement file line %d: %s", lineno, line)
      if len(line.split()) < 3:
         logging.debug("In CustPlacementFile skipping %s", line)
         continue
      app_placement_file.write(line)
   
   # close sys placement file 
   cust_placement_file.close()

   lineno = 0   
   # append application config to application placement file 
   app_config_file = open(AppConfigFileName, 'r')
   for line in app_config_file:
      lineno = lineno + 1
      logging.debug("app config file line %d: %s", lineno, line)
      if len(line.split()) < 3:
         logging.debug("In AppConfigFile skipping %s", line)
         continue      
      app_placement_file.write(line)

   # close app config file 
   app_config_file.close()
   # close app placement file 
   app_placement_file.close()

      
#
# Parse and validate the command line
#
def process_command_line():
   global SysPlacementFileName
   global CustPlacementFileName
   global AppConfigFileName
   global AppPlacementFileName
   
   argv_num = len(sys.argv)
   if not (argv_num == 5 ):
      print "Usage: ", str(sys.argv[0]), "SysPlacementFile CustPlacementFile AppConfigFile AppPlacementFile"
      sys.exit(1)

   SysPlacementFileName = str(sys.argv[1])
   logging.debug("SysPlacementFile: %s", SysPlacementFileName)

   if not os.path.exists(SysPlacementFileName):
      logging.debug("SysPlacementFile doesn't exist....")
      sys.exit(1)

   CustPlacementFileName = str(sys.argv[2])
   if not os.path.exists(CustPlacementFileName):
      logging.debug("CustPlacementFile doesn't exist....")
      sys.exit(1)
      
   AppConfigFileName = str(sys.argv[3])
   logging.debug("AppConfigFile: %s", AppConfigFileName)

   if not os.path.exists(AppConfigFileName):
      logging.debug("AppConfigFile doesn't exist....")
      sys.exit(1)
      
   AppPlacementFileName = str(sys.argv[4])      
   logging.debug("AppPlacementFile: %s", AppPlacementFileName)
   
# main
if os.getenv("DEBUG") or os.getenv("VERBOSE"):
   logging.basicConfig(stream=sys.stderr, level=logging.DEBUG)
else:
   logging.basicConfig(stream=sys.stderr, level=logging.INFO)


logging.debug("Argument List: %s", str(sys.argv))
process_command_line()
create_application_placement_file()
