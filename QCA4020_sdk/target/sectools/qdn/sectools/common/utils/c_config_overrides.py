# ===============================================================================
#
#  Copyright (c) 2013-2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#
# ===============================================================================

import os

from sectools.common.utils.datautils import DataHandler
from sectools.common.utils.datautils import get_cb_dict

MULTI_SERIAL_NUMBERS_TAG = "multi_serial_numbers"

 
class CfgOverrides(object):

    def __init__(self, structure):
        assert isinstance(structure, dict)
        self.structure = structure
        cb_dict = get_cb_dict(path_basepath=os.path.curdir)
        self.root = DataHandler(cb_dict).detail(self.structure)

    def get_properties(self, has_value=False):
        simple = True
        properties = {}
        for tag, detail in self.root.children.items():
            if simple and not detail.is_simple and tag != MULTI_SERIAL_NUMBERS_TAG:
                continue
            if has_value:
                if detail.value_set:
                    properties[tag] = detail
            else:
                properties[tag] = detail
        return properties
