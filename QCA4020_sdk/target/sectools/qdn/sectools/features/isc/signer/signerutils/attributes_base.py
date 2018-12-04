# ===============================================================================
#
#  Copyright (c) 2013-2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#
# ===============================================================================

import json

from sectools.features.isc.signer.signerutils import get_hmac_params_from_config
from sectools.features.isc.defines import SECBOOT_VERSION_1_0

from .attestation_cert_base import AttestationCertificateBase


class SigningAttributesBase(object):

    ATTR_STR_DICT = {
        'exponent': None,
        'hash_algorithm': None,
        'msm_part': None,
        'oem_id': None,
        'model_id': None,
        'hw_id': None,
        'sw_id': None,
        'debug': None,
        'in_use_soc_hw_version': None,
        'use_serial_number_in_signing': None,
    }

    SECBOOT_VERSIONS = (None, SECBOOT_VERSION_1_0)

    EXCLUDE_NON_ATTEST = ['msm_part']
    EXCLUDE_LIST = []
    OPTIONAL_BOOLS = ['in_use_soc_hw_version',
                      'use_serial_number_in_signing',
                      ]
    OPTIONAL_STRING = []

    attestation_certificate_class = AttestationCertificateBase

    def __init__(self):
        self.exponent = None
        self.hash_algorithm = None
        self.msm_part = None
        self.oem_id = None
        self.model_id = None
        self.hw_id = None
        self.sw_id = None
        self.debug = None
        self.in_use_soc_hw_version = None
        self.use_serial_number_in_signing = None
        self.cert_text = None
        self.from_hash_segment = False

    def update_from_attest_cert(self, cert_data):
        # Create the attestation cert object
        cert = self.attestation_certificate_class(cert_data)
        self.cert_text = cert.cert_text

        # Update from the certificate
        for attr in self.ATTR_STR_DICT:
            val = cert.subject_dict.get(attr, None)
            setattr(self, attr, val)

    def update_image_info_attrs(self, attrs):
        # Update the imageinfo
        for attr in self.ATTR_STR_DICT:
            val = getattr(self, attr)
            setattr(attrs, attr, val)

    def update_from_json(self, json_data):
        json_attrs = json.loads(json_data)
        for attr in self.ATTR_STR_DICT:
            json_attr = json_attrs.get(attr, None)
            if isinstance(json_attr, unicode):
                json_attr = str(json_attr)
            setattr(self, attr, json_attr)

    def get_dict(self, exclude=[]):
        retval = {}
        for attr in self.ATTR_STR_DICT:
            if attr in exclude:
                continue
            retval[attr] = getattr(self, attr)
        return retval

    def get_json(self, exclude=[]):
        return json.dumps(self.get_dict(exclude))

    def update_from_image_info_attrs(self, attrs):

        def sanitize(attr, val):
            if attr == 'hash_algorithm' and val is None:
                val = 'sha256'
            elif attr == 'hw_id':
                if attrs.secboot_version in self.SECBOOT_VERSIONS:
                    val = '0x' + get_hmac_params_from_config(attrs).msm_id_str
                else:
                    val = getattr(self, "msm_part")
            return val

        # Update from the imageinfo
        for attr in self.ATTR_STR_DICT:
            val = getattr(attrs, attr, None)
            val = sanitize(attr, val)
            setattr(self, attr, val)

    def update_from_hash_segment_and_attest_cert(self, hash_segment_data, cert_data):
        def sanitize(attr, val):
            if attr in ("sw_id", "hw_id", "oem_id", "model_id",
                        "debug") and not isinstance(val, basestring):
                val = hex(val).strip("L")
            return val

        # Update from the hash segment
        for attr in self.ATTR_STR_DICT:
            val = hash_segment_data.get(attr, None)
            val = sanitize(attr, val)
            setattr(self, attr, val)

        if cert_data:
            # Get exponent from cert
            cert = self.attestation_certificate_class(cert_data)
            self.cert_text = cert.cert_text
            attr = "exponent"
            exponent = cert.subject_dict.get(attr, None)
            setattr(self, attr, exponent)

            # Get signature algorithm from cert
            attr = "hash_algorithm"
            hash_algorithm = cert.subject_dict.get(attr, None)
            setattr(self, attr, hash_algorithm)

        self.from_hash_segment = True

    def compare(self, other, exclude=[]):
        mismatch = []
        for attr in self.ATTR_STR_DICT:
            if attr in exclude:
                continue

            value_self = getattr(self, attr)
            value_other = getattr(other, attr)

            def clean_value(attr, value):
                if isinstance(value, str):
                    value = value.lower()
                    if value.startswith('0x'):
                        value = str(hex(int(value, 16))).rstrip("L")
                if attr in self.OPTIONAL_BOOLS and value is None:
                    value = 0
                if attr in self.OPTIONAL_STRING and value is None:
                    value = '0x0'
                return value

            value_self = clean_value(attr, value_self)
            value_other = clean_value(attr, value_other)
            if value_self != value_other:
                mismatch.append((attr, value_self, value_other))

        return mismatch
