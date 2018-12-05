# ===============================================================================
#
#  Copyright (c) 2013-2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#
# ===============================================================================

import re

from sectools.common import crypto
from sectools.common.crypto.functions.cert import split_subject_string


class Certificate(object):

    @classmethod
    def get_text(cls, cert_data):
        return crypto.cert.get_text(cert_data)

    @classmethod
    def get_subject_list_from_text(cls, text):
        match = re.search('Subject: (.*?)\n', text, re.IGNORECASE)
        if match is None:
            raise RuntimeError('Could not get the SUBJECT field from the attestation certificate')
        return split_subject_string(match.group(1).lower())

    @classmethod
    def get_exponent_from_text(cls, text):
        match = re.search('Exponent: (\d+) \(.*?\)', text, re.IGNORECASE)
        """exponent is only required for RSA, not for ECDSA"""
        if match is not None:
            exponent = match.group(1)
            return int(exponent, 10)
        else:
            return None

    @classmethod
    def get_hash_algorithm(cls, subject_dict, text):
        # OU field takes precedence
        if subject_dict.get('hash_algorithm', None) is not None:
            return subject_dict['hash_algorithm']
        # OU field wasn't found so get hash algo from Signature Algorithm field
        else:
            matchs = [
                re.search('Signature Algorithm:.*\n.*Hash Algorithm: (sha[0-9]+)', text, re.IGNORECASE),
                re.search('Signature Algorithm:.*(sha[0-9]+)WithRSAEncryption', text, re.IGNORECASE),
                re.search('Signature Algorithm:.*ecdsa-with-(sha[0-9]+)', text, re.IGNORECASE)
            ]
            for match in matchs:
                if match is not None:
                    return match.group(1).lower()
            else:
                raise RuntimeError('Could not get the Hash Algorithm field from the attestation certificate')


class AttestationCertificateBase(Certificate):

    def __init__(self, cert_data):
        self.cert_data = cert_data
        self.cert_text = self.get_text(cert_data)
        self.subject_list = self.get_subject_list_from_text(self.cert_text)
        subject_dict, ou_dict = self.process_subject_list(self.subject_list)
        self.subject_dict = subject_dict
        self.subject_dict['exponent'] = self.get_exponent_from_text(self.cert_text)
        self.subject_dict['hash_algorithm'] = self.get_hash_algorithm(self.subject_dict, self.cert_text)
        self.ou_dict = ou_dict

    def process_ou_field(self, ou_str):
        match = re.match('([^ ]+) ([^ ]+) ([^ ]+)', ou_str)
        if match is None:
            raise RuntimeError('OU field OU=' + ou_str + ' is incorrectly formatted')
        ou_number = match.group(1)
        ou_value = match.group(2)
        ou_id = match.group(3)
        return ou_number, ou_value, ou_id

    def process_subject_list(self, subject_list):
        subject_dict = {}
        ou_dict = {}

        for k, v in subject_list:
            # Update OU fields
            if k == 'ou':

                # Get the OU decomposition
                ou_number, ou_value, ou_id = self.process_ou_field(v)

                # Update the OU dictionary
                ou_dict_value = {
                                    'number' : ou_number,
                                    'value'  : ou_value,
                                }
                ou_dict[ou_id] = ou_dict_value

                # Update the key and value
                k = ou_id
                v = ou_value

            # Update the sha value
            if k.startswith('sha'):
                sha_mapping = {
                                'sha1'  : '0000',
                                'sha256': '0001',
                                'sha384': '0002',
                              }
                if (k, v) not in sha_mapping.items():
                    raise RuntimeError('Invalid sha value: ' + k + ':' + v)
                v = k
                k = 'hash_algorithm'

            # Update the hex values
            elif k in ['oem_id', 'model_id', 'hw_id', 'sw_id', 'debug']:
                v = '0x' + v

            # Update the boolean values
            elif k in ['in_use_soc_hw_version',
                       'use_serial_number_in_signing',
                       ]:
                v = int(v)

            subject_dict[k] = v

        return subject_dict, ou_dict
