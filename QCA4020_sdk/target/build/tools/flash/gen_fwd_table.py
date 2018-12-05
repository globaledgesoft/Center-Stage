#!/usr/bin/python

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

'''
This script generates a Firwmare descriptor table used for some Internet of
 Things chips (starting from Quartz)

$Header: //components/rel/core.ioe/1.0/storage/tools/scripts/firmware_desc_table/gen_fwd_table.py#9 $
$DateTime: 2018/05/22 21:04:49 $
$Author: pwbldsvc $

when       who       what, where, why
---------- --------  -----------------------------------------------------
2016-11-16 wkimberl  Don't erase region before program, target does that.
2016-11-15 wkimberl  Use the start_block as the start of the partition.
2016-08-08 wkimberl  Don't erase partitions that have erase set to False.
2016-04-06 wkimberl  Erase all partitions, even those with no file to load
2016-04-05 wkimberl  Don't use full path in rawprogram.xml
2015-09-25 wkimberl  Increase the size of the Image ID field.
2015-09-02 wkimberl  Add output XML file for flashing using jtag.
2015-08-04 wkimberl  Create.

'''

import struct
import xml.etree.ElementTree as ET
import xml.dom.minidom as minidom
import math
import logging
import os
import sys
import ntpath

class Firmware_Descriptor_Entry:
    ''' Firmware Descriptor Entry, stores the data and translates it into
    binary

    Firmware Descriptor Entries
    Little endian         <
    uint32 Image ID       I
    uint32 start_block    I
    uint32 size_in_block  I
    uint32 reserved       I
    '''

    FWD_ENTRIES_FORMAT = '<IIII'
    # size of a single firmware descriptor entry.
    fde_size = 16
    fde_packed = struct.Struct(FWD_ENTRIES_FORMAT)

    reserved_default = 0

    def __init__ (self, block_size):
        ''' Create an empty Firware Partition Entry
        block_size is the size of the SPI-NOR Erase block in bytes.'''
        self.image_id = 0
        # if you ever wonder why filename vs file_name, it is because rawprogra.xml used in MSMs use filename, not file_name.
        self.filename = ""
        self.start_block = None
        self.size_in_kb = 0.0
        self.block_size = block_size
        self.erase = True # By default erase the partition.
        if block_size <= 0:
            raise AssertionError("The block size can't be 0 or below (%d)" % (block_size))
        self.reserved = self.reserved_default

    def update_block_size (self, block_size):
        ''' Update the block size used for the entry. Changing the block size
        modifies the size_in_block of the image '''
        # Check that the input is an integer
        if not isinstance(block_size, (int, long)):
            raise AssertionError ("The input block size isn't an integer")

        self.block_size = block_size

    def get_size_in_block (self):
        ''' Returns the size in blocks of the image. Useful to set up the
        size of the start of the next image. '''
        size_in_block = math.ceil(float(self.size_in_kb * 1024) / self.block_size)
        return int(size_in_block)

    def update_start_block (self, start_block):
        ''' Updates the start block of the image. '''
        # Check that the input is an integer
        if not isinstance(start_block, (int, long)):
            raise AssertionError ("The input start block isn't an integer")

        self.start_block = start_block
    def get_start_block(self):
        return self.start_block

    def to_binary (self):
        ''' Convert the firmware descriptor entry into a packed binary
        form '''
        size_in_block = self.get_size_in_block()
        data = self.fde_packed.pack(self.image_id,
                                    self.start_block,
                                    size_in_block,
                                    self.reserved)
        return data;

    def from_binary (self, data):
        ''' Convert the binary packed form of firmware descriptor into a
        easily manageable class. '''

        all_bytes =  self.fde_packed.unpack(data)
        self.image_id = all_bytes[0]
        self.start_block = all_bytes[1]
        size_in_block = all_bytes[2]
        self.reserved = all_bytes[3]

        self.size_in_kb = float(size_in_block) * float(self.block_size) / 1024

    def __str__ (self):
        ''' Convert the firmware descriptor entry into a nicely printable
        string '''
        data = ""
        data = data + "Image ID:     %d (0x%X)\n" % (self.image_id, self.image_id)
        data = data + "File Name:    %s\n" % (self.filename)
        data = data + "Start block:  %d (0x%X)\n" % (self.start_block, self.start_block)
        data = data + "Size in KB:   %d\n" % (self.size_in_kb)
        size_in_block = self.get_size_in_block()
        data = data + "Size in block:%d (0x%X)\n" % (size_in_block, size_in_block)
        data = data + "Erase:        %s\n" % (str(self.erase))
        data = data + "reserved:     0x%X\n" % (self.reserved)
        return data

    def __to_xml__ (self, xml_element):
        ''' Adds all the members of this class into the xml element.
        used by to_xml and to_program Methods. '''

        xml_element.set ('image_id', str(self.image_id))
        xml_element.set ('filename', str(self.filename))
        xml_element.set ('start_block', str(self.start_block))
        xml_element.set ('size_in_block', str(self.get_size_in_block()))
        xml_element.set ('size_in_kb', str(self.size_in_kb))
        xml_element.set ('erase', str(self.erase))

        if self.reserved != self.reserved_default:
            xml_element.set ('reserved', "0x%X" % (self.reserved))

    def to_xml (self):
        ''' Converts a single partition entry to an XML object.This object
        can be printed to XML by using ET.tostring(fde.to_xml) '''
        data = ET.Element('partition')
        self.__to_xml__(data)
        return data

    def from_xml (self, xml_root):
        ''' Parses the XML Root from an ElementTree, the XML data should
        look like:
        <partition image_id="4097" size_in_block="3" start_block="4"/>
        '''
        if xml_root.tag != 'partition':
            raise AssertionError("Trying to parse something that is not a partition." % (size))

        self.image_id = int(xml_root.attrib['image_id'], 0)

        self.filename = xml_root.attrib['filename']
        self.size_in_kb = float(xml_root.attrib['size_in_kb'])
        if 'start_block' in xml_root.attrib:
            self.start_block = int(xml_root.attrib['start_block'], 0)
        else:
            self.start_block = None

        #By default partitions don't have a file then they are erased, unless
        #the attribute says otherwise.
        self.erase = False
        if 'erase' in xml_root.attrib:
            attrib = xml_root.attrib['erase'].lower()
            if "false" in attrib:
                self.erase = False #XML Says not to erase
            elif "true" in attrib:
                self.erase = True #XML Says to erase
        elif len(self.filename) == 0:
            self.erase = True  #don't have a file to flash, erase by default.
        else:
            self.erase = False  #have a file to flash, Target will take care of erasing if not already erased.

        if 'reserved' in xml_root.attrib:
            self.reserved = int(xml_root.attrib['reserved'], 0)

        return


class Firmware_Descriptor_Table:
    ''' Definition of one File Descriptor Table

    FWD Table Header
    Little endian        <
    uint32 sig          I
    uint32 ver          I
    uint32 rank         I
    uint8  status       B
    uint8  num_images   B
    uint8  reserved[18] s*18
    '''
    FWD_TABLE_HEADER = '<IIIBB18s'
    # Size in bytes of the header specified above.
    header_size = 32
    # Number of reserved bytes
    reserve_size = 18

    FWD_TABLE_SIGNATURE = 0x54445746 #same as "FWDT"

    fdt_packed = struct.Struct(FWD_TABLE_HEADER)

    def __init__ (self, block_size):
        ''' Create the Firmware Descriptor Table structure with no entries
        in it. '''
        self.signature = self.FWD_TABLE_SIGNATURE
        self.version   = 0
        self.rank      = 0
        self.status    = 0
        self.num_images = 0

        if not isinstance(block_size, (int, long)):
            raise AssertionError ("The input start block isn't an integer")

        self.block_size = block_size

        #create a bytearray filled with 0xFF
        temp = [0x00] * self.reserve_size
        self.reserved = str(bytearray(temp))
        self.entries = []

    def add_entry (self, entry):
        self.entries.append(entry)
        self.num_images = self.num_images + 1

    def update (self, sig, ver, rank, status):
        self.signature = sig
        self.version   = ver
        self.rank      = rank
        self.status    = status

    def to_binary (self):
        ''' Convert the firmware descriptor entry into a packed binary
        form '''
        data = self.fdt_packed.pack(self.signature,
                                    self.version,
                                    self.rank,
                                    self.status,
                                    self.num_images,
                                    self.reserved
                                    )
        for entry in self.entries:
            data = data + entry.to_binary()

        return data;

    def from_binary (self, data):
        ''' Convert the binary packed form of firmware descriptor table
        header into a easily manageable class. '''
        offset = self.header_size

        all_bytes = self.fdt_packed.unpack(data[:offset])
        self.signature = all_bytes[0]
        self.version = all_bytes[1]
        self.rank = all_bytes[2]
        self.status = all_bytes[3]
        self.num_images = all_bytes[4]
        self.reserved = str(all_bytes[5])

        self.entries = []

        entry_size = Firmware_Descriptor_Entry.fde_size
        for count in xrange(self.num_images):
            entry = Firmware_Descriptor_Entry(self.block_size)
            entry.from_binary (data[offset:offset + entry_size])
            self.entries.append(entry)
            offset = offset + entry_size

    def __str__ (self):
        data = "HEADER:\n"
        data = data + ""
        data = data + "signature: 0x%X\n" % (self.signature)
        data = data + "Version:   0x%X\n" % (self.version)
        data = data + "Rank:      0x%X\n" % (self.rank)
        data = data + "Status:    0x%X\n" % (self.status)
        data = data + "Num_images:%d (0x%X)\n" % (self.num_images, self.num_images)
        data = data + "Reserved: 0x%s\n" % (self.reserved.encode('hex').upper())
        data = data + "\nENTRIES:\n"

        for entry in self.entries:
            data = data + str(entry)
            data = data + "\n"

        return data

    def to_xml(self):
        ''' Generates an XML Element that contains the Fimware Descriptor
        Table data. '''
        table = ET.Element('table')
        header = ET.SubElement(table, 'header')
        header.set('signature', hex(self.signature))
        header.set('version', str(self.version))
        header.set('rank', str(self.rank))
        header.set('status', str(self.status))
        header.set('reserved', "0x" + str(self.reserved.encode('hex').upper()))

        for entry in self.entries:
            data = entry.to_xml()
            table.append(data)

        return table

    def to_xml_str (self):
        # The XML Element Tree module does not do pretty print.
        # Convert it to another module and back to string to get nice
        # output.
        table = self.to_xml()
        xml_unformat = ET.tostring(table, 'utf-8')
        reparse = minidom.parseString(xml_unformat)
        return reparse.toprettyxml(indent="   ")

    def from_xml (self, xml_root):
        ''' Parses the XML Root from an ElementTree, the XML data should
        look like:
        <table>
        <header rank="0" signature="0x54445746" status="1" version="1"/>
        <partition image_id="4097" size_in_kb="3" start_block="4"/>
        <partition image_id="4098" size_in_kb="2" start_block="7"/>
        <partition image_id="4099" size_in_kb="1" start_block="9"/>
        <partition image_id="4100" size_in_kb="1" start_block="10"/>
        </table>
        '''
        if xml_root.tag != 'table':
            raise AssertionError("Trying to parse something that is not a table." % (size))

        #Clear any pre-existing data.
        self.__init__(self.block_size)

        for child in xml_root:
            if child.tag == 'header':
                self.rank = int(child.attrib['rank'], 0)
                self.status = int(child.attrib['status'], 0)
                self.version = int(child.attrib['version'], 0)

                if 'signature' in child.attrib:
                    self.signature = int(child.attrib['signature'], 0)

                if 'reserved' in child.attrib:
                    if child.attrib['reserved'][:2] == '0x':
                        reserved = child.attrib['reserved'][2:]
                    else:
                        reserved = child.attrib['reserved']
                    self.reserved = str(reserved.decode('hex'))

            elif child.tag == 'partition':
                entry = Firmware_Descriptor_Entry(self.block_size)
                entry.from_xml(child)
                self.add_entry (entry)

        return

    def __iter__(self):
        ''' Iterate through each entry in the partition table '''
        return self.entries.__iter__()


class Firmware_Descriptor:
    ''' Holds the different copies of a Firmware_Descriptor_Table.
    A firmware descriptor is typically formed of 3 tables: Active, Golden
    and Trial. This class holds all the different copies.
    '''

    TABLE_ALIGN_SIZE = 4096 # The tables should be aligned to this boundary

    def __init__ (self):
        ''' Initalizes the Firmware Descriptor'''
        self.table_align_size = self.TABLE_ALIGN_SIZE
        self.block_size = self.TABLE_ALIGN_SIZE
        self.tables = []

    def from_binary (self, data, table_align_size = TABLE_ALIGN_SIZE):
        ''' Populate the Firmware Descriptor from raw binary data read from
        the disk. The data field the stream of bytes from f.read(12288)'''

        # Clear any pre-existing data.
        self.__init__()

        align = self.table_align_size
        header_size = Firmware_Descriptor_Table.header_size

        while len(data) > header_size:
            # Extract each of the tables.
            fdt = Firmware_Descriptor_Table(self.block_size)
            fdt.from_binary(data)
            # Only add the tables that have a valid signature.
            if fdt.signature == fdt.FWD_TABLE_SIGNATURE:
                self.tables.append(fdt)
            data = data[self.table_align_size:]

    def tables_to_binary (self, table_list):
        ''' Convert a Firmware_Descriptor class into a binary data stream
        that can be saved directly into a file. '''

        data = bytearray()
        #create a bytearray filled with 0xFF
        temp = [0xff] * self.table_align_size
        pad = bytearray(temp)

        for table in table_list:
            t = table.to_binary()
            size = len(t)
            data.extend(t)
            data.extend(pad[size:])

        return data

    def to_xml (self):
        ''' Convert the data in the Firmware Descriptor to an XML tree
        '''

        fd_xml = ET.Element('firmwaredescriptor')
        instructions = ET.SubElement(fd_xml, 'instructions')
        instructions.set ('table_align_size_bytes', str(self.table_align_size))
        instructions.set ('erase_block_size_bytes', str(self.block_size))

        for table in self.tables:
            table_xml = table.to_xml()
            fd_xml.append(table_xml)

        return fd_xml

    def to_xml_str (self):
        ''' The XML Element Tree module does not do pretty print.
        Convert it to another module and back to string to get nice
        output. '''
        fd_xml = self.to_xml()

        xml_unformat = ET.tostring(fd_xml, 'utf-8')
        reparse = minidom.parseString(xml_unformat)
        return reparse.toprettyxml(indent="   ")

    def from_xml_file (self, xml_file):
        ''' Populate the Firmware Descriptor class with what is in an
        XML'''

        # Clear any pre-existing data.
        self.__init__()

        xml = ET.parse(xml_file)
        fw_descriptor = xml.getroot()
        if fw_descriptor.tag != 'firmwaredescriptor':
            raise ValueError("XML didn't start with the correct tag <firmwaredescriptor>")

        instructions = fw_descriptor.find('instructions')
        self.table_align_size = int(instructions.attrib['table_align_size_bytes'], 0)
        self.block_size = int(instructions.attrib['erase_block_size_bytes'], 0)

        for tab in fw_descriptor.iter('table'):
            fdt = Firmware_Descriptor_Table (self.block_size)
            fdt.from_xml(tab)
            self.tables.append(fdt)

    def get_disk_size (self):
        ''' Get the total size in bytes of the Disk according to the
        Firmware Descriptor'''

        last_block = 0
        size_in_blk = 0

        # Go through each table and find out the last used sector.
        for table in self.tables:
            for entry in table:
                curr_block = entry.get_start_block()
                if last_block < curr_block:
                    last_block = curr_block
                    size_in_blk = entry.get_size_in_block()


        # If the tables are empty, then the last block is the number of tables.
        if last_block == 0:
            last_block = len(self.tables * self.table_align_size / self.block_size)

        return (last_block + size_in_blk) * self.block_size

    def __check_overlap(self, pstart, pend):
        ''' Checks if the start and end overlap with another partition.
         if it is the same as an existing partition is not considered overlap. '''
        for table in self.tables:
            for entry in table:
                cstart = entry.get_start_block()
                cend = cstart + entry.get_size_in_block() - 1
                if cstart == pstart and cend == pend:
                    continue
                if (pstart <= cstart and cstart <= pend) or \
                        (pstart <= cend and cend <= pend):
                    logging.error ('Overlapping partition for file %s start_block %d end block %d'
                                   % (entry.filename, cstart, cend))
                    print entry
                    return -1
        return 0


    def __check_overlap_all(self):
        ''' Checks if there are overlapping partitions '''


        for table in self.tables:
            for entry in table:
                curr_start = entry.get_start_block()
                curr_end = curr_start + entry.get_size_in_block() - 1
                res = self.__check_overlap(curr_start, curr_end)
                if res != 0:
                    logging.error ('Overlapping partition for file %s start_block %d end block %d'
                                   % (entry.filename, curr_start, curr_end))
                    print entry
                    raise AssertionError("Overlapping partitions")

    def compute_start_block(self):
        ''' Adjust the start block of each partition to have them all packed
        toghether in disk'''

        num_fwt = len(self.tables) * self.table_align_size / self.block_size
        start_block = num_fwt

        highest_end = 0
        for table in self.tables:
            for entry in table:
                if entry.get_start_block() == None:
                    entry.update_start_block(start_block)
                else:
                    start_block = entry.get_start_block()

                curr_end = start_block + entry.get_size_in_block()
                if curr_end > highest_end:
                    highest_end = curr_end

                start_block = highest_end

        self.__check_overlap_all()



    def __str__(self):
        data = ""
        for table in self.tables:
            data = data + str(table)
        return data

    def gen_whole_disk_binary(self, filename):
        ''' Write to the given file name a copy of the binary blob contained
        in the partition table. Note: All files must be in current folder.'''

        out = open(filename, 'wb')

        # Fill the file with 0xFF
        temp = [0xff] * self.block_size
        pad = bytearray(temp)
        count = 0
        size = self.get_disk_size()
        logging.info('Will generate a %d byte image filled with 0xFF' % (size))
        while count < size:
            out.write(pad)
            count = count + self.block_size

        #file already pre-filled with 0xFF, add the tables
        out.seek(0)
        out.write(self.tables_to_binary(self.tables))

        count = 0
        for table in self.tables:
            logging.info('Checking for files to add to the image for Firmware Descriptor Table %d' % (count))
            count = count + 1
            for entry in table:
                if len(entry.filename) > 0:
                    logging.debug('Will try to open file %s' % (entry.filename))
                    try:
                        with open(entry.filename , 'rb') as f:
                            #read the data from the input file.

                            size = int(entry.size_in_kb * 1024)
                            data = f.read(size)
                            logging.debug('Read %d bytes from input, out of %d' % (len(data), size))

                            #place it on the output file in the right location
                            block = entry.get_start_block()
                            out.seek(block * self.block_size)
                            out.write(data)
                            f.close()
                    except IOError as e:
                        logging.exception("Unable to open the file '%s'\n" %
                                       (entry.filename))
                        raise AssertionError("Can't open file %s" % (entry.filename))
        logging.info('Done generating the whole disk image')


    def gen_xml_program (self, filename, table_filepath, table_index = None):
        ''' Generate a series of XML TAGS to program the device using JTAG.
        filename  Output XML file name to use.
        table_filepath   Filename for the binary file that holds the Firmware Table info.'''

        def add_basic_xml_attributes (xml_elem, operation, filename, start_sect, num_sect):
            ''' Add the filename, number of sectors and start sectors into a new element.
            For example, xml_elem, 'erase', 'fdt.bin', 0, 3 would generate:
            <erase filename="fdt.bin" num_partition_sectors="3", start_sector="0" /> '''

            element = ET.SubElement(xml_str, operation)
            element.set ('filename', filename)
            element.set ('start_sector', str(int(start_sect)))
            element.set ('num_partition_sectors', str(int(num_sect)))
            return element

        if table_index == None:
            table_list = self.tables
            start_block = 0
        else:
            if not isinstance(table_index, int):
                logging.error("Table index is not valid %s" % (str(table_index)))
                raise AssertionError("Invalid table index")
            if table_index >= len(self.tables) or table_index < 0:
                logging.error("Table index out of range %d >= %d" % (table_index, len(self.tables)))
                raise AssertionError("Invalid table index, too large or small.")
            table_list = []
            table_list.append(self.tables[table_index])
            start_block = table_index

        xml_str = ET.Element('jdata')
        xml_str.append(ET.Comment('This is an autogenerated file'))
        xml_str.append(ET.Comment('Sector size used is %d' % (self.block_size)))


        table_binary = self.tables_to_binary(table_list)
        table_len = len(table_binary)
        try:
            with open(table_filepath , 'wb') as f:
                f.seek(0)
                f.write(table_binary)
                f.close()
        except IOError as e:
            logging.exception("Unable to open the file '%s'\n" %
                              (table_filepath))
            raise AssertionError("Can't open file %s" % (table_filepath))

        table_len_blocks = table_len / self.block_size

        # Erase the sectors, then program them.

        table_filename = ntpath.basename(table_filepath)
        add_basic_xml_attributes (xml_str, 'program', table_filename, start_block, table_len_blocks)

        if table_index == None:
            count = 0
        else:
            count = table_index

        for table in table_list:
            logging.info('Checking for files to add to the image for Firmware Descriptor Table %d' % (count))
            count = count + 1
            for entry in table:
                block_start = entry.get_start_block()
                size_blocks = math.ceil (entry.size_in_kb * 1024.0 / float(self.block_size))
                # Erase the whole partition
                if entry.erase == True:
                    add_basic_xml_attributes (xml_str, 'erase', entry.filename, block_start, size_blocks)

                if len(entry.filename) > 0:
                    # Try to get the file size to only program the file data, if it can't open use the whole partition.
                    logging.debug('Will try to open file %s to check size' % (entry.filename))
                    try:
                        file_size = os.path.getsize(entry.filename)
                        file_size_blocks = math.ceil (float(file_size) / float(self.block_size))
                        if file_size_blocks > size_blocks:
                            logging.error ("The partition size (%d blocks) isn't large enough to hold the file %s (%d blocks)" % (size_blocks, entry.filename, file_size_blocks))
                    except:
                        logging.debug("Unable to access the file '%s', Can't check if it fits in the partition.\n" % (entry.filename))

                    # Now program only the file data.
                    add_basic_xml_attributes (xml_str, 'program', entry.filename, block_start, size_blocks)

        logging.debug('Completed generating the XML for programming.')
        xml_unformat = ET.tostring(xml_str, 'utf-8')
        reparse = minidom.parseString(xml_unformat)
        reparse.toprettyxml(indent="   ")

        try:
            with open(filename , 'wb') as f:
                f.seek(0)
                f.write(reparse.toprettyxml(indent="   "))
                f.close()
        except IOError as e:
            logging.exception("Unable to open the file '%s'\n" %
                              (filename))
            raise AssertionError("Can't open file %s" % (filename))

        logging.info('Done Generating XML file to program.')


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

    tool_verbose_description = """Tool to generate intermediate files for flashing a Firmware Descriptor Table and
related images into disk. There are two ways to generate the images to flash the
images into the target.

1. [Prefered] Generate a rawprogram.xml output and a firmware descriptor table
   binary file. Supply the rawprogram.xml and path to all images/files to
   flash_through_gdb.py.
2. Generate a whole disk (raw disk), flash this whole disk image to disk. This
   method requires all the images to exist in the current directory, is provided
   as a fallback.

Example Usage:
Generate rawprogram to feed to flash_through_gdb.py.
1. Update partition.xml (follow the example_partition.xml)
2. Run: python gen_fwd_table.py --partitionxml partition.xml --rawprogram rawprogram.xml --fdtbin firmware_table.bin
3. Flash with flash_through_gdb.py

"""

    parser = argparse.ArgumentParser(description=tool_verbose_description, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('-x', '--partitionxml', type=file, required=True, help='The partition.xml file for the Firmware Descriptor')
    parser.add_argument('--rawdisk', type=str, required=False, help='The output file where to store the whole disk')
    parser.add_argument('--rawprogram', type=str, required=False, help='Output XML File for programming on device')
    parser.add_argument('--fdtbin', type=str, required=False, help='Firmware Descriptor Table Binary output file used when --rawprogram is used. Default value is firmware_table.bin', default='firmware_table.bin')
    parser.add_argument('-v', '--verbose', type=int, choices=[0,1,2,3,4,5], help='Verbose levels. Higher numbers include lower. For example, 3 means 3,2,1 and 0. 0=Critcal. 1=Error, 2=Warning 3=Info[Default], 4=Debug, 5=Everything', default=3)
    parser.add_argument('--printtable', help='Print the partition table as a friendly string to the output.', default=False, action='store_true')
    parser.add_argument('--fwdindex', type=int, required=False, help='Generate the Firmware descriptor and rawprogram for the given firmware descriptor', default=None)

    args = parser.parse_args()

    log_level = [logging.CRITICAL, logging.ERROR, logging.WARNING, logging.INFO, logging.DEBUG, logging.NOTSET]
    logging.basicConfig(format='[%(asctime)-15s] %(levelname)s: %(message)s File:%(filename)s:%(lineno)d Function:%(funcName)s ', level=log_level[args.verbose])

    if args.rawdisk is None and args.rawprogram is None:
        logging.error ('No output specified, please use --rawdisk or --rawprogram to specify the output')
        return

    #read the intput XML file.
    fwd = Firmware_Descriptor()
    fwd.from_xml_file (args.partitionxml.name)

    #Compute the start block for each partition depending on the sizes given
    fwd.compute_start_block()

    #Generate the disk image.
    if args.rawdisk is not None:
        fwd.gen_whole_disk_binary(args.rawdisk)

#    import pdb; pdb.set_trace()
    if args.rawprogram is not None:
        fwd.gen_xml_program(args.rawprogram, args.fdtbin, table_index = args.fwdindex)

    if args.printtable:
        print str(fwd)

if __name__ == "__main__":
    main()
#    import pdb; pdb.set_trace()
