# ===============================================================================
#
#  Copyright (c) 2013-2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#
# ===============================================================================

import os

from sectools.common.utils import c_path
from sectools.features.isc.cfgparser import ConfigParser
from sectools.features.isc.parsegen.config.parser import ParsegenCfgParser

from .base_stager import BaseStager


class ImagePathsStagerIot(BaseStager):

    def __init__(self, image_path, img_config_parser, parsegen_config, authority,
                 sign_id=None, crypto_params={}, imageinfo_class=None):

        assert isinstance(image_path, str)
        assert isinstance(img_config_parser, ConfigParser)
        assert isinstance(parsegen_config, ParsegenCfgParser)
        if sign_id is not None: assert isinstance(sign_id, str)

        # Initialize the BaseStager
        super(ImagePathsStagerIot, self).__init__(authority)

        # Validate that the image path exists
        image_path = c_path.normalize(image_path)
        if not c_path.validate_file(image_path):
            raise RuntimeError('No read access to the image path: ' + image_path)
        # Put the image info object into the list
        imageinfo = self._create_imageinfo(
            img_config_parser, parsegen_config, sign_id, image_path,
            crypto_params=crypto_params,
            imageinfo_class=imageinfo_class)
        self._image_info_list.append(imageinfo)

    @property
    def output_dir(self):
        """(str) Output directory to store the output data to."""
        return self._output_dir

    @output_dir.setter
    def output_dir(self, output_dir):
        assert isinstance(output_dir, str)
        output_dir = c_path.normalize(output_dir)
        if not c_path.validate_dir_write(output_dir):
            raise RuntimeError('No write access to output directory: ' + output_dir)

        # Update the output dir of each image in image_info_list
        for image_info in self.image_info_list:
            self.update_dest_image(image_info, output_dir)

        self._output_dir = output_dir

    def _create_imageinfo(self, img_config_parser, parsegen_config, sign_id, image_path,
                          src_image=None, dest_image=None,
                          crypto_params={}, imageinfo_class=None):

        sign_id = self._get_sign_id(img_config_parser, os.path.basename(image_path), sign_id)
        # Get the config block for the sign id
        img_config_block = img_config_parser.get_config_for_sign_id(sign_id)

        # Create the image info object
        image_info = imageinfo_class(image_path, sign_id, img_config_block, img_config_parser,
                                     parsegen_config, authority=self.authority,
                                     crypto_params=crypto_params)
        image_info.dest_image.image_name = image_info.src_image.image_name

        # Set src_image
        if src_image is not None:
            image_info.src_image = src_image
            image_info.image_under_operation = image_info.src_image.image_path

        # Set dest_image
        if dest_image is not None:
            image_info.dest_image = dest_image

        # Check if the dest image name should be overridden
        if img_config_block.output_file_name is not None:
            image_info.dest_image.image_name = img_config_block.output_file_name

        return image_info
