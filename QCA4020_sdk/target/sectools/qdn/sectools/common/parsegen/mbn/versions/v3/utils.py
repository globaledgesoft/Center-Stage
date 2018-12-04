# ===============================================================================
#
#  Copyright (c) 2013-2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#
# ===============================================================================

from sectools.common.parsegen.mbn.utils import extract_segment


class V3Utils(object):

    @classmethod
    def update_mbn_header(cls, header, image_dest_ptr, mbn_version):
        header.flash_parti_ver = mbn_version
        header.image_src = 0
        header.image_dest_ptr = image_dest_ptr

    @classmethod
    def extract_sign(cls, header, data):
        offset = header.code_size
        size = header.sig_size
        data, seg = extract_segment(data, offset, size)
        return data, seg

    @classmethod
    def extract_cert_chain(cls, header, data):
        offset = header.code_size + header.sig_size
        size = header.cert_chain_size
        data, seg = extract_segment(data, offset, size)
        return data, seg

    @classmethod
    def mask_header_values(cls, header, authority):
        return header
