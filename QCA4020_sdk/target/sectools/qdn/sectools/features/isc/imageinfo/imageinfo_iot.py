# ===============================================================================
#
#  Copyright (c) 2013-2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#
# ===============================================================================

from sectools.common.utils.c_base import ValPolicy

from .imageinfo_base import ImageInfoBase
from .imageinfo_base import DestImagePathBase

from sectools.features.isc.iot.cfgparser import defines as cfgdef
from sectools.features.isc.iot.cfgparser import auto_gen_obj_config as agoc

POLICY_OEM_ID_0 = ValPolicy(ValPolicy.WARN)


class ImageInfoIot(ImageInfoBase):

    def __init__(self, image_path, sign_id, img_config_block, img_config_parser,
                 parsegen_config, authority=None, crypto_params={}, **kwargs):
        assert isinstance(image_path, str)

        self.dest_image = DestImagePathBase()
        self.crypto_params = crypto_params

        super(ImageInfoIot, self).__init__(
            image_path, sign_id, img_config_block, img_config_parser,
            parsegen_config, authority=authority,
            agoc=agoc, pgagoc=agoc, cfgdef=cfgdef, crypto_params=crypto_params)

        self.image_under_operation = self.src_image.image_path

    #--------------------------------------------------------------------------
    # Helper functions
    #--------------------------------------------------------------------------
    def _sanitize_general_properties(self):
        pass

    def _validate_properties(self):
        self.errors = []
        if self.errors:
            raise RuntimeError("".join(self.errors))
