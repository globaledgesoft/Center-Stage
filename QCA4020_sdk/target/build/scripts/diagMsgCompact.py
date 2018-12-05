#!/usr/bin/python

#============================================================================
#
# diagMsgCompact main script 
#
# GENERAL DESCRIPTION
#    Diag Debug Message Compaction script.
#
#Note: http://flint.cs.yale.edu/cs422/doc/ELF_Format.pdf has good ELF 
#documentation. 
#
#===============================================================================
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

#===============================================================================

#----------------------------------------------------------------------------
#
#  $Header: //components/rel/core.ioe/1.0/bsp/build/scripts/diagMsgCompact.py#4 $
#  $DateTime: 2018/05/30 03:44:40 $
#  $Author: pwbldsvc $
#   
#                      EDIT HISTORY FOR FILE
#                      
#  This section contains comments describing changes made to the module.
#  Notice that changes are listed in reverse chronological order.
#  
# YYYY-MM-DD   who     what, where, why
# ----------   ---     ---------------------------------------------------------
# 2017-01-13   mad     Remove CR character from string before generating ID 
#                      and adding to database
# 2016-08-03   mad     Created
#================================================================================

import elfManipulator.include.elfFileClass as elfFileClass  # this is the main ELF manipulator class used in this script
import elfManipulator.include.elfConstants as const
import optparse
import hashlib  # for MD5
import zlib     # for crc
from os.path import exists as pe
# if ROM, we need to create a pickle file.
# if RAM, we need to import from the ROM pickle
#since cpickle is much faster, try to import it if it exists, else use python native pickle
try:
   import cPickle as pickle
except:
   import pickle

diagStringDatabase = {} #Global database that contains ID(key) and string (value)

copyrightStr = "#Copyright (c) 2016 by Qualcomm Technologies Incorporated. All Rights Reserved. \n\n"
   
#============================================================================================
#Parse and modify source_file_elf and write target_elf and target_db.
#Two compactTypes: 
#Intermediate: used for ROM. Creates a pickle file as output.
#Final: used for RAM build. Takes an additional source_file_pkl (from ROM) and creates the 
#final string database that tool will use.
#
#Reads out ELF one time into an Elf object. 
#Modifies the ELF object and writes out all modifications in one go.
#
#This is called from the diagMsgCompact_builder to hook this into scons system
#=============================================================================================
   
def diag_msg_compact_modify_elf(target_elf, target_db, source_file_elf, source_file_pkl, compactType):
   global diagStringDatabase
   global sourceElfObject #global ELF object that will hold all the modifications until we write out the target_elf

   diagRegion = "ROM_DIAG_STRING_RO_REGION" # bogus segment that needs to be removed, if ROM build. This contains all the format strings

   if(compactType == 'Final'): #Final is for RAM. 'Intermediate' is for ROM
      importROMPickle(source_file_pkl) #Import ROM database from the pickle file
      diagRegion = "RAM_DIAG_STRING_RO_REGION" #if RAM build, this is the name of the bogus region
   
   sourceElfObject = elfFileClass.elfFile(source_file_elf) #parse the ELF file into the ELF object

   # NOTE: The symbol list must be unique and deferring in the Prefix sub string.
   symbolList = ['_diag_msg_compact','_qapi_Diag_Msg_Compact'] # this is the symbol we're looking for

   for i in range(len(symbolList)):
       for j in symbolList[i+1:]:
           if symbolList[i].startswith(j) or j.startswith(symbolList[i]):
               print "Symbol strings need to be unique and no string starts with other string in the list."
               exit(1)

   # In order to support both GCC and ARMCT, these symbols are searched as sub strings to
   # mask out name decoration/mangling for these ELF symbols. Especially in case of GCC,
   # its observed that these ELF symbols are name decorated/mangled.
   compactSymbolList = sourceElfObject.getSymbolStartWithName(symbolList) # get all symbols
   if compactSymbolList == const.RC_ERROR :
      print "diagMsgCompact: Failed to get symbols from Elf."
      ##TODO: In this case, parse the map file and get the symbols? check with Amit. TODO.
      removeDiagStringSectionAndWriteElf(target_elf, diagRegion) #even if we fail to get symbols, make sure ELF doesn't have bogus program segment
      exit(1)

   try:
      targetDBFileHandle = open(target_db, 'w+', 1) #create and always over-write
   except IOError:
      print("diagMsgCompact: Failed to open or create " + target_db)
      removeDiagStringSectionAndWriteElf(target_elf, diagRegion)#whatever the error, we need to make sure we remove the bogus segment and write out the good elf.
      exit(0)

   if compactSymbolList == []:
      print "diagMsgCompact:No Debug Message usage found!"
   #print "diagMsgCompact: Got symbol list"
   for compactSymbol in compactSymbolList:
      #print compactSymbol.st_nameStr + " at address: ", hex(compactSymbol.st_value)
      stringAddress = sourceElfObject.readDataByAddress(compactSymbol.st_value, 4) #the data will be the string address
      #print "format string address:", hex(stringAddress)
      formatString = sourceElfObject.getStringByAddress(stringAddress) #read the string from the address
      #print "found string at address:", hex(stringAddress), formatString
      id = generateStringID(formatString) # generate ID
      #print "Setting string ID:", hex(id)
      sourceElfObject.setDataByAddress(compactSymbol.st_value, 4, id) #now replace the stringaddress with the stringID
      # only for debugging. Just read id and make sure we're reading out what we wrote.
      # comment out on release version
      #id = sourceElfObject.readDataByAddress(compactSymbol.st_value, 4)
      #print "Reading out id from %d : id = %d", hex(compactSymbol.st_value),hex(id)

   writeDatabaseToFile(targetDBFileHandle, compactType)# write the database (or intermediate pickle)
   #print diagStringDatabase #comment out before release
   removeDiagStringSectionAndWriteElf(target_elf, diagRegion) #write final elf and we're done.


#==============================================================================
# Removes diagRegion: bogus program segment that contains the format strings.
# Writes out the ELF
#==============================================================================
def removeDiagStringSectionAndWriteElf(targetElf, diagRegion):
   global sourceElfObject
   print "diagMsgCompact: Removing Diag debug strings segment from elf:",diagRegion
   sourceElfObject.removeSectionByName(diagRegion)
   print "diagMsgCompact: Saving the elf",targetElf
   sourceElfObject.writeOutELF(targetElf)

#------------------------------------------------------------------------
#If final (RAM), writes preamble and writes out all mappings to final database file.
#If intermediate (ROM), just pickles the mappings into the intermediate pickle file.
#==========================================================================
def writeDatabaseToFile(targetFileHandle, compactType):
   global diagStringDatabase

   if(compactType == 'Final'):
      writePreamble(targetFileHandle)
      #write items into the final string database file
      for key in diagStringDatabase:
         writeStr = "{0}:{1}\n".format(key,diagStringDatabase[key])
         targetFileHandle.write(writeStr)
      return

   if(compactType == 'Intermediate'):
      pickle.dump(diagStringDatabase, targetFileHandle)

   return


#=====================================================================
#string dictionary from ROM build is stored as an intermediate pickle file and
#has to be imported into the string database global variable
#=====================================================================
def importROMPickle(source_file_pkl):
   global diagStringDatabase
   
   try:
      pklFileHandle = open(source_file_pkl, 'r')
   except IOError:
      print("diagMsgCompact: Failed to open " + source_file_pkl)
      return
   #load the ROM pickle into the global database variable and close pickle file
   diagStringDatabase = pickle.load(pklFileHandle)
   print "diagMsgCompact: Imported ROM pickle:\n",source_file_pkl
   #print diagStringDatabase #comment out before release
   pklFileHandle.close()
   return

#------------------------------
#Writes the preamble.
#Copyright and a canned string
#------------------------------
def writePreamble(targetFileHandle):
   writeStr = "#<Diag String Database>\n\n"
   targetFileHandle.write(copyrightStr)
   targetFileHandle.write(writeStr)

#------------------------------------------------------------------------
#Generates string ID from string.
#Takes crc of 16-byte MD5 to generate a unique and consistent 4-byte value 
#corresponding to the string.
#consistency requirement : in different runs we need to generate same ID for same string.
#
#Checks dictionary for collisions. 
#collision defined as same Id for a different string.
# (If same string, it is not a collision)
#Collision resolution achieved by just incrementing the ID by 1 until it finds a unique ID.
#
#Makes sure ID is not 4-byte aligned, so that diag buffer-draining code 
#can distinguish between string addresses (that are always 4-byte aligned...uses linker 
#directives to make sure they are 4-byte aligned) and stringIDs.
#------------------------------------------------------------------------
def generateStringID(string):
   global diagStringDatabase
   
   string_strip = string.replace('\r','');
   stringMD5 = hashlib.md5(string_strip).digest() # md5 is 16 bytes
   crc = zlib.crc32(stringMD5) #take the 4-byte crc of the MD5 digest so we get a 4-byte unique, consistent value

   #un-align the hash and make it not divisible by 4
   id = unalignID(crc & 0xffffffff) # from documentation for zlib library, need to do the bit-wise AND to make crc same across different python versions

   #if id already present and not mapped to same string, keep incrementing until we find an un-used id
   while(id in diagStringDatabase and diagStringDatabase[id] != string_strip):
      id = unalignID(id+1)
      #print "diagMsgCompact: Collision, re-try"

   #store to database and return id
   diagStringDatabase[id]=string_strip
   return id   

#--------------------------------------------
#Unalign if id is 4-byte aligned
#--------------------------------------------
def unalignID(id):
   if id & 0x3 == 0:
      id += 1
   return id


#----------------------------------------------------------------------------------------------
#main function
#not used from scons builder, but can be used to debug the functionality independent from a build
#environment.
#----------------------------------------------------------------------------------------------------

def main():
    # Use optparse to set up usage
   use = "Usage: python %prog <Output ELF> <Output string database File> <Input ELF> <input pickle file> <CompactType: Intermediate or Final>"
   parser = optparse.OptionParser(usage = use, version="%prog 1.0")
   options, arguments = parser.parse_args()

    # Check arguments
   if len(arguments) != 5:
      parser.error("Unexpected argument length")
      print "Unexpected argument length"
      exit(1)

   baseELF = arguments[2]
   if not pe(baseELF):
      parser.error("Specified ELF file does not exist.")
      print "Specified ELF file does not exist."
      exit(1)

   #just call the function with all arguments
   diag_msg_compact_modify_elf(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4])
   #print out the database
   print diagStringDatabase
   exit(0)

#Needed so the main() will execute if run from a python debugger and not when it is imported
if __name__ == "__main__":
    main()