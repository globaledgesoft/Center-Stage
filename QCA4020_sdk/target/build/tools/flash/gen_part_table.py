######################################################################################
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

######################################################################################
#
# Create a XML description of a partition table for a Firmware Image Set.
# The Firmware Image Set is described by command line parameters.
#
# Example:
#     python gen_part_table.py --help
# Displays help information
#
# Example:
#     python gen_part_table.py --file=m4app_HASHED.elf --file=ioe_ram_m0_threadx_ipt.mbn --file=wlan_fw_img.bin
# Creates generated_partition_table.xml
#
# Typically, the output from this command, generated_partition_table.xml,
# is used as input to gen_fwd_table.py.
#
######################################################################################


import sys
import os
import logging
import getopt
import re
import subprocess
from collections import defaultdict

# Image ID constants; MUST MATCH qapi_firmware_image_id.h
FS1_IMG_ID    =   "5"
M4_IMG_ID     =  "10"
M0_IMG_ID     =  "11"
WLAN_IMG_ID   =  "13"
FS2_IMG_ID    = "128"
UNUSED_IMG_ID = "129"
FLOG_IMG_ID   = "100"

# Human-readable names, simply for documentation purposes in the XML output file.
M0_IMG_NAME     = "M0/CONSS Firmware"
M4_IMG_NAME     = "M4/APPSS Firmware"
WLAN_IMG_NAME   = "WLAN Firmware"
FS1_IMG_NAME    = "Primary Filesystem"
FS2_IMG_NAME    = "Secondary Filesystem"
UNUSED_IMG_NAME = "Unused"
FLOG_IMG_NAME   = "Flash Log Partitions"

default_output_file = "generated_partition_table.xml"
default_FS1_sz = "64KB"

using_cygwin = False;

part_name = ""
part_id = ""
part_start = ""
part_size = ""
part_file = ""

partition_list = []

image_version = ""

num_copies_of_table = 1

#
# Print a usage message for this command.
# Invoked when no arguments are supplied or when an argument cannot be parsed
#
def usage():
    print sys.argv[0], "usage:"
    print """
Uses command line options to generate a partition_table.xml file which
describes the various partitions within an ImageSet. If an incomplete
specification is provided for a partition, heuristics are used to infer
the missing values.

Options:
--output=<filename>
    Output file name. Defaults to generated_partition_table.xml

--name=<partition_name>
    Provides a human-readable name for a partition

--id=<image_id>
    Image ID# or a pre-defined image name:
        APPSS or M4  (Application or M4 image)
        CONSS or M0  (Connectivity or M0 image)
        FS or FS1    (primary file system)
        FS2          (secondary file system)
        WIFI or WLAN (802.11 image)

--size=<size_specification>
    Size of partition specified in bytes, kilobytes, megabytes or 4KB blocks.
    Examples: --size=32768 or --size=32KB or --size=8BLK or --size=1MB
    If size is not specified for a partition that comes from a file, the size
    of the file (rounded up to a block) is used as the size of the partition.

--start=<size_specification>
    Absolute start offset of the current partition.
    Example: --start=4KB causes a partition to be located at 4 kilobytes
    into flash. So, for instance, a pre-existing file system can be
    referenced.

--file=<filename_or_keyword>
    A file containing the desired image to be placed onto flash and
    used for a particular partition within an ImageSet. If there is
    no file specified for a partition then the size of the partition
    must be specified along with --file=ERASE (to erase the partition)
    or --file=KEEP (to keep the current flash contents). If --size is
    specified and --file is not specified, the default is ERASE.
    
--partition
    Specifies the start of a set of options that specify a new partition.
    It is safe to use --partition in front of each set of command line
    options that pertain to a partition. In the absence of an explicit
    --partition, this command uses heuristics to determine when a new
    partition specification starts. For instance, when scanning options
    if a file is known for a partition and a new file is specified then
    it is assumed a new partition specification has begun.

--version=<version_number>
    Selects a version number to be used for the ImageSet. The bootloader
    gives preference to a Trial ImageSet with a higher version number over
    a Trial ImageSet with a lower version number.
    
--begin=<size_specification>
    Absolute offset at which to begin this ImageSet.
    Example: --begin=1MB (starts the ImageSet at 1 megabyte into flash)

A simple example of options to create an ImageSet with an M4 image,
a 16KB "reserved area" an M0 image and an implicit primary file system:
    --file=$M4_IMAGE \\
    --file=ERASE --size=16KB --id=129 \\
    --file=$M0_IMAGE
Note that the size of the M0 and M4 images are inferred from the size
of the files that fill them. The IDs and human-readable partition names
are inferred from the names of the files. Partitions are placed in
default order, starting with the implicit primary file system and
follwed by the M4 image, the 16KB reserved space, and then the M0 image.
"""
    exit(-1)



#
# Well-known symbolic names map to numeric Image IDs
#
well_known_id = defaultdict()
def initialize_well_known_dict():
    well_known_id["FS"] = FS1_IMG_ID;
    well_known_id["FS1"] = FS1_IMG_ID;
    well_known_id["M4"] = M4_IMG_ID;
    well_known_id["APPSS"] = M4_IMG_ID;
    well_known_id["CONSS"] = M0_IMG_ID;
    well_known_id["M0"] = M0_IMG_ID;
    well_known_id["WLAN"] = WLAN_IMG_ID;
    well_known_id["WIFI"] = WLAN_IMG_ID;
    well_known_id["FS2"] = FS2_IMG_ID;
    well_known_id["UNUSED"] = UNUSED_IMG_ID;
    well_known_id["FLOG"] = FLOG_IMG_ID;    

#
# Start the definition of a new partition specification within this ImageSet.
#
def new_partition_spec():
    global part_name
    global part_id
    global part_start
    global part_size
    global part_file
    global partition_list

    if ((part_name != "") or (part_id != "") or (part_start != "") or (part_file != "")):
        partition_list = partition_list + [(part_name, part_id, part_start, part_size, part_file)]
    part_name = ""
    part_id = ""
    part_start = ""
    part_size = ""
    part_file = ""

#
# Convert a numeric string with an optional unit suffix into an integer number of bytes.
#
def convert_size_str_to_int(sz):
    match = re.match(r'(?i)^([1-9][0-9]*)\s*(MBS|MB|M|KBS|KB|K|B|BLKS|BLK)$', sz)

    if match:
        s_units = match.group(2)
        if ((s_units == "MBS") or (s_units == "MB") or (s_units == "M")):
            unit_multiplier = 1024*1024
        elif ((s_units == "KBS") or (s_units == "KB") or (s_units == "K")):
            unit_multiplier = 1024
        elif ((s_units == "BLKS") or (s_units =="BLK")):
            unit_multiplier = 4096
        elif (match.group(2) == "B"):
            unit_multiplier = 1

        nbytes = int(match.group(1)) * unit_multiplier
        return nbytes
    else:
        return int(sz)


#
# Use heuristics to determine defaults for unspecified values in the partition list.
# Perform validations, too. This is invoked once after all command line options have
# been parsed.
#
def complete_partition_list():
    global partition_list
    global begin_offset
    new_partition_list = []
    primary_file_system_found = False
    file_size = 0

    initialize_well_known_dict()

    for partition in partition_list:
        s_name, s_id, s_start, s_size, s_file = partition
        file_size = 0

        # If s_file is specified, validate that the file exists.
        if ((s_file != "") and (s_file.upper() != "KEEP") and (s_file.upper() != "ERASE")):
            # If a file is specified, check for existence and get its size.
            try:
                file_size = os.path.getsize(s_file)
            except Exception as err:
                logging.error(str(err))
                exit(-1)

            # Inferences based on filename....
            dirname, filename = os.path.split(s_file)

            # Infer partition name from the filename
            if (s_name == ""):
                if re.search('(?i)m0', filename):
                    s_name = M0_IMG_NAME
                elif re.search('(?i)m4', filename):
                    s_name = M4_IMG_NAME
                elif re.search('(?i)wlan', filename):
                    s_name = WLAN_IMG_NAME

            # Infer partition ID from the filename
            if (s_id == ""):
                # Infer s_id from the file name, if possible
                if re.search('(?i)m0', filename):
                    s_id = M0_IMG_ID
                elif re.search('(?i)wlan', filename):
                    s_id = WLAN_IMG_ID
                else:
                    s_id = M4_IMG_ID

            # Infer partition size from the size of the file.
            if (s_size == ""):
                s_size = str(file_size)

        # If s_id is symbolic, convert it to numeric string
        if s_id in well_known_id.keys():
            s_id = well_known_id[s_id]

        # Infer partition ID from partition name
        if ((s_name != "") and (s_id == "")):
            if (re.search('(?i)FS2', s_name)):
                s_id = FS2_IMG_ID
            elif (re.search('(?i)FS', s_name)):
                s_id = FS1_IMG_ID

        # Infer default partition size for primary file system
        if (s_size == "") and (s_id == FS1_IMG_ID):
            s_size = default_FS1_sz

        # Infer partition name from partition ID
        if ((s_name == "") and (s_id != "")):
            if (s_id == FS1_IMG_ID):
                s_name = FS1_IMG_NAME
            elif (s_id == FS2_IMG_ID):
                s_name = FS2_IMG_NAME
            elif (s_id == UNUSED_IMG_ID):
                s_name = UNUSED_IMG_NAME
            elif (s_id == FLOG_IMG_ID):
                s_name = FLOG_IMG_NAME
            else:
                s_name = s_id;

        # If no partition content is specified for a file system,
        # infer that the partition should be erased.
        if (s_file == ""):
            i_id = int(s_id)
            if ((i_id == int(FS1_IMG_ID)) or (i_id == int(FS2_IMG_ID)) or (i_id == int(UNUSED_IMG_ID)) or (i_id == int(FLOG_IMG_ID))):
                s_file = "ERASE"

        # If we are preserving contents of an unnamed partition
        # using a file system ID, infer the partition name
        if (s_file.upper() == "KEEP") and (s_name == ""):
            if (int(s_id) == int(FS1_IMG_ID)):
                s_name = FS1_IMG_NAME
            elif (int(s_id) == int(FS2_IMG_ID)):
                s_name = FS2_IMG_NAME

        if ((s_id == "") or (s_size == "") or (s_file == "")):
            if (s_name == ""):
                s_name="(none)"
            if (s_id == ""):
                s_id = "UNKNOWN"
            if (s_size == ""):
                s_size = "UNKNOWN"
            if (s_file == ""):
                s_file = "UNKNOWN"
            logging.error("Incomplete partition specification: name=%s id=%s size=%s file=%s",
                           s_name, s_id, s_size, s_file)
            exit(-1)

        if (int(s_id) == int(FS1_IMG_ID)):
            primary_file_system_found = True

        # Validate size specification against size of file
        i_size = convert_size_str_to_int(s_size)
        if ((file_size > 0) and (i_size < file_size)):
            logging.error("Specified size (%d) is too small for specified image (%d)", i_size, file_size)
            logging.error(partition)
            exit(-1)

        new_partition_list = new_partition_list + [(s_name, s_id, s_start, str(i_size), s_file)]


    # Every ImageSet must include a primary file system.
    # Default positioning for FS is at the start of the ImageSet.
    if (not primary_file_system_found):
        new_partition_list = [("PrimaryFS", FS1_IMG_ID, "", default_FS1_sz, "ERASE")] + new_partition_list

    partition_list = new_partition_list

    return

#
# Prepare the partition list to be easily digested by
# the current version of gen_fwd_table.py.
#
# The current implementation of gen_fwd_table.py requires partitions
# that are placed automatically to PRECEDE partitions with a defined
# start_block EXCEPT for the first partition, which defines the starting
# point on flash for the ImageSet.
#
def prepare_list_for_gen_fwd():
    global partition_list
    global begin_offset

    # Scan through the partition_list moving all partitions with defined
    # start blocks to the end.

    defined_start_list=[]
    undefined_start_list=[]
    for partition in partition_list:
        s_name, s_id, s_start, s_size, s_file = partition
        if (s_start == ""):
            undefined_start_list = undefined_start_list + [partition]
        else:
            defined_start_list = defined_start_list + [partition]

    # Default ImageSet start is just after the three FWDs
    if (begin_offset == 0):
        begin_offset = 3*4096 # 3 FWDs, 1 sector each

    # Now, if there is at least one partition without a pre-defined start block,
    # then update the start block of the first partition accordingly.
    if (undefined_start_list != []):
        s_name, s_id, s_start, s_size, s_file = undefined_start_list.pop(0)
        undefined_start_list = [(s_name, s_id, str(begin_offset), s_size, s_file)] + undefined_start_list

    partition_list = undefined_start_list + defined_start_list


#
# Create an XML output file which contains the specification of an ImageSet.
#
def generate_XML_output():
    global partition_list
    global output_file
    global image_version
    xml_file = open(output_file, 'w')

    # Header
    xml_file.write("<?xml version=\"1.0\" ?>\n")

    xml_file.write("<?ignore\n")
    xml_file.write("This XML file was auto-generated:\n")
    cmddir, cmdname = os.path.split(sys.argv[0])
    xml_file.write(cmdname)
    for arg in sys.argv[1:]:
        xml_file.write(" ")
        xml_file.write(arg)
    xml_file.write("\n?>\n")

    xml_file.write("<firmwaredescriptor>\n") # Start FWDT
    xml_file.write("   <instructions erase_block_size_bytes=\"4096\" table_align_size_bytes=\"4096\"/>\n")

    #
    # This is a hack intended to provide compatibility for an existing tool, gen_fwd_table.py.
    # We write SEVERAL COPIES of the ImageSet partition table in order to support
    # "gen_fwd_table.py --fwdindex=N". It would be preferable to improve gen_fwd_table.py.
    #
    for copy in range(num_copies_of_table):
        xml_file.write("   <table>\n") # Start partition list
        xml_file.write("      <header rank=\"1\" signature=\"0x54445746\" status=\"1\" version=\"")
        xml_file.write(image_version)
        xml_file.write("\"/>\n")

        # Write to the XML file a line for each image in the ImageSet.
        for partition in partition_list:
            s_name, s_id, s_start, s_size, s_file = partition

            if ((s_file.upper() == "ERASE") or (s_file.upper() == "KEEP")):
                # s_file is a reserved keyword, not an actual filename
                xml_file.write("      <partition filename=\"\"") # no actual file
            else:
                xml_file.write("      <partition")
                dirname, filename = os.path.split(os.path.abspath(s_file))
                if dirname:
                    if using_cygwin:
                        try:
                            dirname = subprocess.check_output("cygpath -w " + '"' + dirname + '"', shell=True).rstrip('\n')
                        except:
                            logging.error("Failed to convert Cygwin name to Windows name.")

                    xml_file.write(" dirname=\"")
                    xml_file.write(dirname)
                    xml_file.write("\"")
                xml_file.write(" filename=\"")
                xml_file.write(filename)
                xml_file.write("\"")

            xml_file.write(" image_id=\"")
            xml_file.write(s_id)
            xml_file.write("\"")

            if (s_start != ""):
                i_start = (convert_size_str_to_int(s_start) + 4095) / 4096 # expressed in BLKs
                xml_file.write(" start_block=\"")
                xml_file.write(str(i_start))
                xml_file.write("\"")

            i_size = convert_size_str_to_int(s_size)
            size_in_kb = (i_size + 1023) / 1024
            xml_file.write(" size_in_kb=\"")
            xml_file.write(str(size_in_kb))
            xml_file.write("\"")

            if (s_file.upper() == "KEEP"):
                xml_file.write(" erase=\"False\"")

            xml_file.write("/>")

            if (s_name != ""):
                xml_file.write(" <!--")
                xml_file.write(s_name)
                xml_file.write("-->")

            xml_file.write("\n")

        xml_file.write("   </table>\n")

    xml_file.write("</firmwaredescriptor>\n")
    xml_file.close()



def main():
    global part_name
    global part_id
    global part_start
    global part_size
    global part_file
    global partition_list
    global output_file
    global image_version
    global begin_offset
    global num_copies_of_table
    global using_cygwin

    if re.search("cygwin", sys.platform):
        using_cygwin = True
    else:
        using_cygwin = False

    output_file = ""
    begin_offset = 0

    try:
        opts, args = getopt.getopt(sys.argv[1:], "o:n:i:s:f:p", [
                     "output=",   # output file name
                     "name=",     # human-readable partition name
                     "id=",       # image ID number or name
                     "size=",     # partition size in bytes, KBs, MBs or 4KB BLKs
                     "start=",    # partition start (absolute offset from start of flash)
                     "file=",     # file containing contents OR keyword "KEEP" or "ERASE"
                     "partition", # indicates start of a new partition definition (optional)
                     "version=",  # which version to use for this ImageSet (defaults to 1)
                     "begin=",    # where to begin this ImageSet
                     "copies=",   # number of copies of partition table (defaults to 3)
        ])
    except getopt.GetoptError as err:
        logging.error(str(err))
        usage()

    if (len(opts) == 0):
        usage()

    if (len(args) > 0):
        logging.error("Unexpected option %s", args[0])
        usage()

    for myopt, myarg in opts:
        if myopt in ("-o", "--output"):
            if (output_file == ""):
                output_file = myarg
            else:
                logging.error("Specify output file at most once.")
                exit(-1)
        elif myopt in ("-n", "--name"):
            if (part_name != ""):
                new_partition_spec()
            part_name = myarg
        elif myopt in ("-i", "--id"):
            if (part_id != ""):
                new_partition_spec()
            part_id = myarg.upper()
        elif myopt in ("--start"):
            if (part_start != ""):
                new_partition_spec()
            part_start = myarg.upper()
        elif myopt in ("-s", "--size"):
            if (part_size != ""):
                new_partition_spec()
            part_size = myarg.upper()
        elif myopt in ("-f", "--file"):
            if (part_file != ""):
                new_partition_spec()
            part_file = myarg
        elif myopt in ("-p", "--partition"):
            new_partition_spec()
        elif myopt in ("--version"):
            image_version = myarg
        elif myopt in ("--begin"):
            if (begin_offset != 0):
                logging.error("Specify 'begin' at most once.")
                exit(-1)
            begin_offset = convert_size_str_to_int(myarg)

            # Verify space for three 4KB firmware descriptors at start of flash
            if (begin_offset < 3 * 4096):
                logging.error("Begin offset is too small; must leave 12KB for firmware descriptors");
                exit(-1)
        elif myopt in ("--copies"): # (undocumented option)
            num_copies_of_table = int(myarg)
            if ((num_copies_of_table < 1) or (num_copies_of_table > 3)):
                logging.error("Requested number of copies is inappropriate");
                exit(-1)
        else:
            usage()

    # Push final partition data to partition_list
    if ((part_name != "") or (part_id != "") or (part_start != "") or (part_size != "") or (part_file != "")):
        new_partition_spec()

    if (output_file == ""):
        output_file = default_output_file

    if (image_version == ""):
        image_version = "1"

    # print "DEBUG: partition list before completion:", partition_list
    complete_partition_list()
    # print "DEBUG: partition list after completion:", partition_list
    prepare_list_for_gen_fwd()
    # print "DEBUG: partition list after preparing for gen_fwd:", partition_list

    generate_XML_output()


if __name__ == "__main__":
    main()
