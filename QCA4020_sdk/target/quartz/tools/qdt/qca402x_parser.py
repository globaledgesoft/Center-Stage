#!/usr/bin/python
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

import datetime
import time
from qca402x_dictionary import qca402x_dictionary

DEBUG_MESSAGE_CMD_CODE = 0x9c
LOG_CMD_CODE = 0x10
DBGLOG_CMD_CODE = 0x1923
PKTLOG_CMD_CODE = 0x1924
KF_DBGLOG_CMD_CODE = 0x2923

EFS_CMD_CODE = 0x4b
EFS_SUBSYS_CODE = 0x13

PKTLOG_MAX_TXCTL_WORDS = 13
PKTLOG_MAX_TXSTATUS_WORDS = 10
PKTLOG_MAX_RXSTATUS_WORDS = 12
MAX_TX_RATE_TBL = 32

proc_id = ['APPS','CNSS','WLAN']

dbglog_level = ['INFO','LOW','HIGH','ERR']
dbglog_module_id = ['BOOT','INF','WMI','MISC','PM','TXRX_MGMTBUF','TXRX_TXBUF','TXRX_RXBUF','WOW','WHAL','DC',
             'CO','RO','CM','MGMT','TMR','BTCOEX','BOOT','P2P','IEEE','MAC_CONC','WNM','RFKILL',
             'NET_AUTOIP','NET_DHCPV4','NET_PKT','NET_IFDRV','NET_PORT','NET_IPMAIN','NET_TCP','WPS',
             'DSET','BMI','MBOX','DFS','SECURITY','OTA','PAL','USB','WREG','OFFLOAD']

debug_message_loglevel = ['n/a','LOW','MED','HIGH']
debug_message_module_id = ['DIAG','WLAN_DRV','WLAN_FW','ADC','TSENS']

pktlog_id = ['TXCtl','TxStatus','Rx','RCFind','RCUpdate','Ani']

class dbglogframe:
    def __init__(self,file_path=None,com_port=None,fout=None):
        self.nargs = 0
        self.strlength = 0
        self.argtype = 0
        self.loglevel = 0
        self.ts = 0
        self.moduleID = 0
        self.args = list()

class pktlogframe:
    def __init__(self,file_path=None,com_port=None,fout=None):
        self.flags = 0
        self.type = 0
        self.size = 0
        self.ts = 0
        self.msg = None

# This script will generate dictionary objects from given files.
class qca402x_parser:
    def __init__(self,pkt,mode,qdict,t0_host,t0_target):
        self.pkt = pkt
        self.mode = mode
        self.qdict = qdict

        self.log_type = None
        self.p_id = None
        self.tempList = list()
        self.t0_host = t0_host
        self.t0_target = t0_target
        self.error = 0

    def get_ref_ts_host(self):
        return self.t0_host

    def get_ref_ts_target(self):
        return self.t0_target

    def process_packet(self):
        cmd_code = int(self.pkt.pop(0),16)
        if cmd_code == DEBUG_MESSAGE_CMD_CODE:
            return self.process_debug_message()
        elif cmd_code == EFS_CMD_CODE:
            return self.process_efs_packet()
        elif cmd_code == LOG_CMD_CODE:
            for n_i in range(0,5):
                unused = self.pkt.pop(0)

            # Extract log type
            log_t = int(''.join(self.pkt.pop(i) for i in range(1,-1,-1)),16)

            for n_i in range(0,8):
                unused = self.pkt.pop(0)

            if log_t == DBGLOG_CMD_CODE:
                self.log_type = 'DBGLOG'
                return self.process_dbglog(1)
            elif log_t == KF_DBGLOG_CMD_CODE:
                self.log_type = 'KF_DBGLOG'
                return self.process_dbglog(1)
            elif log_t == PKTLOG_CMD_CODE:
                self.log_type = 'PKTLOG'
                return self.process_pktlog(1)
            else:
                return self.process_unknown_packet()

        else:
            return self.process_unknown_packet()

    def module_str(self,module_id):
        if module_id > 4:
            return 'USER_SPECIFIC'
        else:
            return debug_message_module_id[module_id]

    def process_debug_message(self):
        """ This method decodes the diag headers and calls parse_diag_string to parse diag headers
        diag headers contain , sequentially :
            * cmd_code      : 1 byte
            * version       : 1 byte
            * Module ID     : 1 byte
            * Proc ID       : 3 bits
            * Message Level : 2 bits
            * Num_Args      : 3 bits
            * Timetick      : 8 bytes
            * Args          : 4 bytes * Num_Args
            * String ID     : 4 bytes

        :param pkt: a list of hex characters
        :return:
        """
        debugMessageFrameSize = 11 # Debug message header minus the cmd_id
        if len(self.pkt) < debugMessageFrameSize:
            print "Incomplete debug message header\n"
            for i in range(0,len(self.pkt),1):
                unused = self.pkt.pop(0)
            return None

        ret = list()
        version          = int(self.pkt.pop(0),16)
        module_id        = int(self.pkt.pop(0),16)

        # Extract proc_id, message level, number of arguments
        pmn = int(self.pkt.pop(0), 16)
        pmn_str          = bin(pmn)[2:].zfill(8)
        self.p_id = int(pmn_str[0:3],2)
        m_lvl = int(pmn_str[3:5],2)
        nargs = int(pmn_str[5:8],2)

        # Time ticks with respect to Quartz boot time.
        # Calculate the boot time from the first packet and store in t0_host.
		# This base timestamp will be used to get time for packets from saved session file.
        ticks = float(int(''.join(self.pkt.pop(i) for i in range(7, -1, -1)),16)) / 32768
        if self.mode == 'com':
            current_time = time.time()
            ts_str = datetime.datetime.fromtimestamp(current_time).strftime('%H:%M:%S-%m%d%Y')
            if self.t0_host == 0:
                self.t0_host = current_time - ticks
        else:
            ts_str = datetime.datetime.fromtimestamp(self.t0_host+ticks).strftime('%H:%M:%S-%m%d%Y')

        if len(self.pkt) < (4*nargs):
            print "Incomplete debug message arguments\n"
            for i in range(0,len(self.pkt),1):
                unused = self.pkt.pop(0)
            return None

        arg_list=list()
        for n_i in range(0,nargs):
            argi =  int(''.join(self.pkt.pop(i) for i in range(3,-1,-1)),16)
            arg_list.append((argi))

        # V2 uses compaction, 4 byte long message ID is used to find corresponding message string in dictionary.
        if (version == 2):
            if len(self.pkt) < 4:
                print "Incomplete debug message message ID\n"
                for i in range(0,len(self.pkt),1):
                    unused = self.pkt.pop(0)
                return None

            messageID = int(''.join(self.pkt.pop(i) for i in range(3,-1,-1)),16)
            arg_list.insert(0,messageID)
            msg = self.process_message_string(*arg_list)

        # V1 does not use compaction, message string is of variable length.
        elif (version == 1):
            msg = ''
            for str_i in range (0,len(self.pkt),1):
                msg = msg + chr(int(self.pkt.pop(0),16))
            if nargs > 0 and msg.find("%") >= 0:
                n_args = tuple([int(n,16) for n in arg_list])
                msg = msg % n_args

        # Add parsed packet data in list for printing 
        rt_dict = {
                   'ts': ts_str,
                   'cmd_code': 'Debug Message',
                   'proc_id': proc_id[self.p_id],
                   'log_level': debug_message_loglevel[m_lvl],
                   'module_id': self.module_str(module_id),
                   'msg': msg,
                   }

        ret.append(rt_dict)
        return ret


    def process_dbglog(self,first_packet):
        """ This method decodes the dbglog headers and calls parse_diag_string to parse diag headers
        diag headers contain , sequentially :
            * nargs         : 2 bits
            * strlen        : 8 bits
            * argtype       : 3 bits
            * logLevel      : 3 bits
            * timestamp     : 2 bytes
            * reserved      : 10 bits
            * moduleID      : 6 bits
            * messageID     : 2 bytes
            * Args          : 4 bytes * Num_Args
        """
        if len(self.pkt) < 8:
            print "Incomplete dbglog header/message header\n"
            self.error = -1
            for i in range(0,len(self.pkt),1):
                unused = self.pkt.pop(0)
            return None

        now = time.time()

        # DBGLOGs are bunched together and sent to Quartz as one big packet, hence the last packet in the bunched
        # gets current time stamp.
        # This function processes all these packets in sequence recursively, adds data for each packet into a list.
        # In the toplevel call, timestamps are calculated for each packet and then it returns complete list to the 
        # original caller for printing.
        frame = dbglogframe()
        param_str = bin(int(''.join(self.pkt.pop(j) for j in range(3,-1,-1)),16))[2:].zfill(32)
        frame.nargs = int(param_str[0:2],2)
        frame.strlen = int(param_str[2:10],2)
        frame.argtype = int(param_str[10:13],2)
        if (frame.argtype & (frame.argtype-1)) != 0:
            print "Only 1 string argument is allowed\n"
            self.error = -1
            for i in range(0,len(self.pkt),1):
                unused = self.pkt.pop(0)
            return None


        frame.loglevel = int(param_str[13:16],2)
        frame.ts = float(int(param_str[16:32],2))

        m_str = bin(int(''.join(self.pkt.pop(i) for i in range(3,-1,-1)),16))[2:].zfill(32)
        frame.moduleID = int(m_str[10:16],2)
        messageID = int(m_str[16:32],2)

        # DBGLOGs can have maximum 3 arguments, only 1 of which can be a string of variable length.
        # Argument type is 3 bit long, one for each argument. The bit correponding to string argument is set to 1.
        # Other bits are set to 0.
        for i in range (0,frame.nargs,1):
            mask = 0x1 << i
            if (mask & frame.argtype):# String argument
                arg_str = ''
                if len(self.pkt) < 4*frame.strlen:
                    print "Incomplete string argument\n"
                    self.error = -1
                    for i in range(0,len(self.pkt),1):
                        unused = self.pkt.pop(0)
                    return None
                # Convert hex bytes into ascii characters
                for str_i in range (0,(4*frame.strlen),1):
                    arg_str = arg_str + chr(int(self.pkt.pop(0),16))
                frame.args.append(arg_str)
            else:# Integer argument
                if len(self.pkt) < 4:
                    print "Incomplete dbglog integer argument\n"
                    self.error = -1
                    for i in range(0,len(self.pkt),1):
                        unused = self.pkt.pop(0)
                    return None
                arg_int =  int(''.join(self.pkt.pop(n) for n in range(3,-1,-1)),16)
                frame.args.append(arg_int)

        frame.args.insert(0,messageID)

        # Recursively process all DBGLOG packets in this bunch.
        self.tempList.append(frame)

        # Process next dbglog if this bunch contains more
        if len(self.pkt) > 0:
            self.process_dbglog(0)
            if self.error == -1:
                return None

        if (first_packet == 1):
            self.p_id = 2
            if (len(self.tempList) == 0):
                return None

            ret = list()
            for i in range(1,len(self.tempList),1):
                if (self.tempList[i].ts < self.tempList[i-1].ts):
                    self.tempList[i].ts = self.tempList[i].ts + 0xFFFF  #Timestamp rolled over

            # Timestamps in DBGLOGs are incremental,hence store the base timestamp calculated wrt last packet in current bunch.
            m4base = self.tempList[-1].ts
            if self.mode == 'com':
                if self.t0_target == 0:
                    self.t0_target = now - (m4base * 3125 / 100000000)

            for entry in self.tempList:
                if self.mode == 'com':
                    offset_Seconds = float(m4base - entry.ts) * 3125 / 100000000
                    secs = now - offset_Seconds
                else:
                    secs = self.t0_target + (entry.ts * 3125 / 100000000)

                msg = self.process_message_string(*(entry.args))
                # Add parsed packet data in list for printing 
                rt_dict = {
                           'ts': datetime.datetime.fromtimestamp(secs).strftime('%H:%M:%S-%m%d%Y'),
                           'cmd_code': self.log_type,
                           'proc_id': proc_id[self.p_id],
                           'log_level': dbglog_level[entry.loglevel],
                           'module_id': dbglog_module_id[entry.moduleID],
                           'msg': msg,
                           }
                ret.append(rt_dict)
            return ret
        else:
            return None


    def process_message_string(self,*args):
        if len(args) <= 0:
            return

        messageID = args[0]
        msg = self.qdict.find_message(messageID, self.p_id)
        if (msg == ''):
            msg = "Message ID %d not found\n" % messageID
            self.error = -1
        elif len(args)>=2:
            try:
                n_args = tuple(args[1:])
                msg = msg % n_args
            except:
                msg = msg + ':' + 'Invalid arguments\n'
                self.error = -1
        return msg


    def process_pktlog(self,first_packet):
        # PKTLOGs are bunched together and sent to Quartz as one big packet, hence the last packet in the bunched
        # gets current time stamp.
        # This function processes all these packets in sequence recursively, adds data for each packet into a list.
        # In the toplevel call, timestamps are calculated for each packet and then it returns complete list to the 
        # original caller for printing.

        if len(self.pkt) < 12:
            print "Incomplete pktlog header\n"
            self.error = -1
            for n_i in range(0,len(self.pkt),1):
                unused = self.pkt.pop(0)
            return None

        now = time.time()
        frame = pktlogframe()
        frame.flags = int(''.join(self.pkt.pop(i) for i in range(3,-1,-1)),16)
        frame.type = int(''.join(self.pkt.pop(i) for i in range(1,-1,-1)),16)
        frame.size = int(''.join(self.pkt.pop(i) for i in range(1,-1,-1)),16)
        frame.ts = int(''.join(self.pkt.pop(i) for i in range(3,-1,-1)),16)

        # Invalid pktlog type. Pop the data to make sure next packet aligns properly
        if frame.type > 5:
            print "Invalid pktlog type\n"
            self.error = -1
            for n_i in range(0,frame.size,1):
                unused = self.pkt.pop(0)
            return None

        # Incomplete pktlog. Pop the data to make sure next packet aligns properly
        if len(self.pkt) < frame.size:
            print "Incomplete pktlog data\n"
            self.error = -1
            for n_i in range(0,len(self.pkt),1):
                unused = self.pkt.pop(0)
            return None

        # Process the pktlog data
        frame.msg = self.pktlog_subtypes(self.pkt,frame)
        if self.error == -1:
            return None
        # Process remaining pktlogs in this bunch recursively
        self.tempList.append(frame)

        # Process next pktlog if this bunch contains more
        if len(self.pkt) > 0:
            self.process_pktlog(0)
            if self.error == -1:
                return None

        if (first_packet == 0):
            return None

        elif (first_packet == 1):
            if (len(self.tempList) == 0):
                return None

            ret = list()
            for i in range(1,len(self.tempList),1):
                if (self.tempList[i].ts < self.tempList[i-1].ts):
                    self.tempList[i].ts = self.tempList[i].ts + 0xFFFF  #Timestamp rolled over

            m4base = self.tempList[-1].ts
            if self.mode == 'com':
                if self.t0_target == 0:
                    self.t0_target = now - (m4base * 3125 / 100000000)

            for entry in self.tempList:
                if self.mode == 'com':
                    offset_Seconds = float(m4base - entry.ts) * 3125 / 100000000
                    secs = now - offset_Seconds
                else:
                    secs = self.t0_target + (entry.ts * 3125 / 100000000)

                rt_dict = {
                           'ts': datetime.datetime.fromtimestamp(secs).strftime('%H:%M:%S-%m%d%Y'),
                           'cmd_code': 'PKTLOG',
                           'proc_id': 'KF',
                           'log_level': 'n/a',
                           'module_id': pktlog_id[entry.type],
                           'msg': entry.msg,
                           }
                ret.append(rt_dict)
            return ret


    def pktlog_subtypes(self,pkt,frame):
        if frame.type == 0:
            return self.process_TXCtl(pkt,frame)
        elif frame.type == 1:
            return self.process_TxStatus(pkt,frame)
        elif frame.type == 2:
            return self.process_Rx(pkt,frame)
        elif frame.type == 3:
            return self.process_RCFind(pkt,frame)
        elif frame.type == 4:
            return self.process_RCUpdate(pkt,frame)
        elif frame.type == 5:
            return self.process_Ani(pkt,type)
        else:
            self.error = -1
            return "PKTLOG error\n"


    def process_TXCtl(self,pkt,frame):
        msg = 'Frame Ctrl:' + str(int(''.join(self.pkt.pop(i) for i in range(1,-1,-1)),16)) + ','
        msg = msg + 'msg Ctrl:' + str(int(''.join(self.pkt.pop(i) for i in range(1,-1,-1)),16)) + ','
        msg = msg + 'BSSID Tail:' + str(int(''.join(self.pkt.pop(i) for i in range(1,-1,-1)),16)) + ','
        msg = msg + 'SA Tail:' + str(int(''.join(self.pkt.pop(i) for i in range(1,-1,-1)),16)) + ','
        msg = msg + 'DA Tail:' + str(int(''.join(self.pkt.pop(i) for i in range(1,-1,-1)),16)) + ','
        msg = msg + 'Resvd:' + str(int(''.join(self.pkt.pop(i) for i in range(1,-1,-1)),16)) + ','

        for i in range (0,PKTLOG_MAX_TXCTL_WORDS,1):
            msg = msg + 'Txdesc_Ctl[' + str(i)+']:'
            msg = msg + str(int(''.join(self.pkt.pop(i) for i in range(3,-1,-1)),16)) + ','

        proto_hdr_len = int(''.join(self.pkt.pop(i) for i in range(3,-1,-1)),16)
        misc_info_len = int(''.join(self.pkt.pop(i) for i in range(3,-1,-1)),16)

        if proto_hdr_len > 0:
            msg = msg + 'Proto Hdr:'
            for i in range (0,proto_hdr_len,1):
                msg = msg + str(int(''.join(self.pkt.pop(i) for i in range(3,-1,-1)),16)) + ','

        if misc_info_len > 0:
            msg = msg + 'Misc:'
            for i in range (0,misc_info_len,1):
                msg = msg + str(int(''.join(self.pkt.pop(i) for i in range(3,-1,-1)),16)) + ','
        return msg


    def process_TxStatus(self,pkt,frame):
        for i in range (0,PKTLOG_MAX_TXSTATUS_WORDS,1):
            msg = msg + 'Txdesc_Status[' + str(i)+']:'
            msg = msg + str(int(''.join(self.pkt.pop(i) for i in range(3,-1,-1)),16)) + ','

        misc_info_len = int(''.join(self.pkt.pop(i) for i in range(3,-1,-1)),16)
        if misc_info_len > 0:
            msg = msg + 'Misc:'
            for i in range (0,misc_info_len,1):
                msg = msg + str(int(''.join(self.pkt.pop(i) for i in range(3,-1,-1)),16)) + ','
        return msg


    def process_Rx(self,pkt,frame):
        msg = 'Frame Ctrl:' + str(int(''.join(self.pkt.pop(i) for i in range(1,-1,-1)),16)) + ','
        msg = msg + 'msg Ctrl:' + str(int(''.join(self.pkt.pop(i) for i in range(1,-1,-1)),16)) + ','
        msg = msg + 'BSSID Tail:' + str(int(''.join(self.pkt.pop(i) for i in range(1,-1,-1)),16)) + ','
        msg = msg + 'SA Tail:' + str(int(''.join(self.pkt.pop(i) for i in range(1,-1,-1)),16)) + ','
        msg = msg + 'DA Tail:' + str(int(''.join(self.pkt.pop(i) for i in range(1,-1,-1)),16)) + ','
        msg = msg + 'Resvd:' + str(int(''.join(self.pkt.pop(i) for i in range(1,-1,-1)),16)) + ','

        for i in range (0,PKTLOG_MAX_TXCTL_WORDS,1):
            msg = msg + 'Rxdesc_Status[' + str(i)+']:'
            msg = msg + str(int(''.join(self.pkt.pop(i) for i in range(3,-1,-1)),16)) + ','

        proto_hdr_len = int(''.join(self.pkt.pop(i) for i in range(3,-1,-1)),16)
        misc_info_len = int(''.join(self.pkt.pop(i) for i in range(3,-1,-1)),16)

        if proto_hdr_len > 0:
            msg = msg + 'Proto Hdr:'
            for i in range (0,proto_hdr_len,1):
                msg = msg + str(int(''.join(self.pkt.pop(i) for i in range(3,-1,-1)),16)) + ','

        if misc_info_len > 0:
            msg = msg + 'Misc:'
            for i in range (0,misc_info_len,1):
                msg = msg + str(int(''.join(self.pkt.pop(i) for i in range(3,-1,-1)),16)) + ','
        return msg



    def process_RCFind(self,pkt,frame):
        msg = 'Rate:' + str(int(self.pkt.pop(0),16)) + ','
        msg = msg + 'RateCode:' + str(int(self.pkt.pop(0),16)) + ','
        msg = msg + 'RcRssiLast:' + str(int(self.pkt.pop(0),16)) + ','
        msg = msg + 'RcRssiLastPrev:' + str(int(self.pkt.pop(0),16)) + ','
        msg = msg + 'RcRssiLastPrev2:' + str(int(self.pkt.pop(0),16)) + ','
        msg = msg + 'RssiReduce:' + str(int(self.pkt.pop(0),16)) + ','
        msg = msg + 'RcProbeRate:' + str(int(self.pkt.pop(0),16)) + ','
        msg = msg + 'IsProbing:' + str(int(self.pkt.pop(0),16)) + ','
        msg = msg + 'PrimeInUse:' + str(int(self.pkt.pop(0),16)) + ','
        msg = msg + 'CurrentPrimeState:' + str(int(self.pkt.pop(0),16)) + ','
        msg = msg + 'RcRateTableSize:' + str(int(self.pkt.pop(0),16)) + ','
        msg = msg + 'RcRateMax:' + str(int(self.pkt.pop(0),16)) + ','
        msg = msg + 'AC:' + str(int(self.pkt.pop(0),16)) + ','

        misc_info_len = int(''.join(self.pkt.pop(i) for i in range(3,-1,-1)),16)
        if misc_info_len > 0:
            msg = msg + 'Misc:'
            for i in range (0,misc_info_len,1):
                msg = msg + str(int(''.join(self.pkt.pop(i) for i in range(3,-1,-1)),16)) + ','
        return msg


    def process_RCUpdate(self,pkt,frame):
        msg = 'Txrate:' + str(int(self.pkt.pop(0),16)) + ','
        msg = msg + 'RateCode:' + str(int(self.pkt.pop(0),16)) + ','
        msg = msg + 'RssiAck:' + str(int(self.pkt.pop(0),16)) + ','
        msg = msg + 'Xretries:' + str(int(self.pkt.pop(0),16)) + ','
        msg = msg + 'Retries:' + str(int(self.pkt.pop(0),16)) + ','
        msg = msg + 'RcRssiLast:' + str(int(self.pkt.pop(0),16)) + ','
        msg = msg + 'RcRssiLastLkup:' + str(int(self.pkt.pop(0),16)) + ','
        msg = msg + 'RcRssiLastPrev:' + str(int(self.pkt.pop(0),16)) + ','
        msg = msg + 'RcRssiLastPrev2:' + str(int(self.pkt.pop(0),16)) + ','
        msg = msg + 'RcProbeRate:' + str(int(self.pkt.pop(0),16)) + ','
        msg = msg + 'RcRateMax:' + str(int(self.pkt.pop(0),16)) + ','
        msg = msg + 'UseTurboPrime:' + str(int(self.pkt.pop(0),16)) + ','
        msg = msg + 'CurrentBoostState:' + str(int(self.pkt.pop(0),16)) + ','
        msg = msg + 'RcHwMaxRetryRate:' + str(int(self.pkt.pop(0),16)) + ','
        msg = msg + 'AC:' + str(int(self.pkt.pop(0),16)) + ','

        for i in range (0,2,1):
            msg = msg + 'Resvd[' + str(i)+']:'
            msg = msg + str(int(self.pkt.pop(0),16)) + ','

        for i in range (0,MAX_TX_RATE_TBL,1):
            msg = msg + 'RC RSSI Threshold[' + str(i)+']:'
            msg = msg + str(int(self.pkt.pop(0),16)) + ','

        for i in range (0,MAX_TX_RATE_TBL,1):
            msg = msg + 'RcPer[' + str(i)+']:'
            msg = msg + str(int(self.pkt.pop(0),16)) + ','

        for i in range (0,MAX_TX_RATE_TBL,1):
            msg = msg + 'RcMaxAggrSize[' + str(i)+']:'
            msg = msg + str(int(self.pkt.pop(0),16)) + ','

        msg = msg + 'HeadFail:' + str(int(self.pkt.pop(0),16)) + ','
        msg = msg + 'TailFail:' + str(int(self.pkt.pop(0),16)) + ','
        msg = msg + 'AggrSize:' + str(int(self.pkt.pop(0),16)) + ','
        msg = msg + 'AggrLimit:' + str(int(self.pkt.pop(0),16)) + ','
        msg = msg + 'LastRate:' + str(int(self.pkt.pop(0),16)) + ','

        misc_info_len = int(''.join(self.pkt.pop(i) for i in range(3,-1,-1)),16)
        if misc_info_len > 0:
            msg = msg + 'Misc:'
            for i in range (0,misc_info_len,1):
                msg = msg + str(int(''.join(self.pkt.pop(i) for i in range(3,-1,-1)),16)) + ','
        return msg



    def process_Ani(self,pkt,frame):
        msg = 'PhyStatsDisable:' + str(int(self.pkt.pop(0),16)) + ','
        msg = msg + 'NoiseImmunLvl:' + str(int(self.pkt.pop(0),16)) + ','
        msg = msg + 'SpurImmunLvl:' + str(int(self.pkt.pop(0),16)) + ','
        msg = msg + 'OfdmWeakDet:' + str(int(self.pkt.pop(0),16)) + ','
        msg = msg + 'CCKWeakThr:' + str(int(self.pkt.pop(0),16)) + ','
        msg = msg + 'RSSI:' + str(int(self.pkt.pop(0),16)) + ','

        msg = msg + 'FirLvl' + str(int(''.join(self.pkt.pop(i) for i in range(1,-1,-1)),16)) + ','
        msg = msg + 'ListenTime' + str(int(''.join(self.pkt.pop(i) for i in range(1,-1,-1)),16)) + ','
        msg = msg + 'Resvd' + str(int(''.join(self.pkt.pop(i) for i in range(1,-1,-1)),16)) + ','

        msg = msg + 'CycleCount' + str(int(''.join(self.pkt.pop(i) for i in range(3,-1,-1)),16)) + ','
        msg = msg + 'OfdmPhyErrCount' + str(int(''.join(self.pkt.pop(i) for i in range(3,-1,-1)),16)) + ','
        msg = msg + 'CCKPhyErrCount' + str(int(''.join(self.pkt.pop(i) for i in range(3,-1,-1)),16)) + ','

        misc_info_len = int(''.join(self.pkt.pop(i) for i in range(3,-1,-1)),16)
        if misc_info_len > 0:
            msg = msg + 'Misc:'
            for i in range (0,misc_info_len,1):
                msg = msg + str(int(''.join(self.pkt.pop(i) for i in range(3,-1,-1)),16)) + ','
        return msg


    def process_unknown_packet(self):
        for n_i in range(0,len(self.pkt),1):
            unused = self.pkt.pop(0)

        ret = list()
        rt_dict = {
                   'ts':'n/a',
                   'cmd_code':'n/a',
                   'proc_id':'n/a',
                   'log_level':'n/a',
                   'module_id':'n/a',
                   'msg':'Cannot parse message',
                   }

        ret.append(rt_dict)
        return ret


    def process_efs_packet(self):
        """ This method decodes the efs headers :
            * cmd_code    : 1 byte
            * ss_ID       : 1 byte
            * ss_cmd_ID   : 2 bytes
            * seq         : 2 bytes
            * error       : 2 bytes
            * iter_handle : 1 byte

        :param pkt: a list of hex characters
        :return: string
        """

        ret = list()
        for i in range(0,len(self.pkt),1):
            unused = self.pkt.pop(0)
        rt_dict = {
                   'ts': datetime.datetime.fromtimestamp(time.time()).strftime('%H:%M:%S-%m%d%Y'),
                   'cmd_code': 'EFS',
                   'proc_id': 'n/a',
                   'log_level': 'n/a',
                   'module_id': 'n/a',
                   'msg': 'n/a',
                   }

        ret.append(rt_dict)
        return ret
