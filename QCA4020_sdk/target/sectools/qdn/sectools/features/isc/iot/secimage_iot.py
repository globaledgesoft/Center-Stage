# ===============================================================================
#
#  Copyright (c) 2013-2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#
# ===============================================================================

import sys
import traceback

from sectools.common import crypto
from sectools.common.utils import c_path
from sectools.common.utils.c_logging import logger
from sectools.common.utils.c_misc import PROGRESS_CB_PYPASS
from sectools.common.utils.c_misc import ProgressNotifier

from sectools.features.isc.secimage_base import SecImageCoreBase
from sectools.features.isc.imageinfo.imageinfo_base import DestImagePathBase
from sectools.features.isc.secpolicy import create_security_policy_list
from sectools.features.isc.imageinfo.imageinfo_iot import ImageInfoIot
from sectools.features.isc.imageinfo import StatusInfo
from sectools.features.isc.secimage_base import PROGRESS_TOTAL_STAGES
from sectools.features.isc.stager.image_paths_stager_iot import ImagePathsStagerIot
from sectools.features.isc.parsegen.config.parser import ParsegenCfgParser


class IotParsegenCfgParser(ParsegenCfgParser):
    """ IoT-specific Config ParseGen Class """
    @property
    def config_data(self):
        return self.root.parsegen

    def validate(self):
        pass


class SecImageIotCore(SecImageCoreBase):

    dest_image_path_class = DestImagePathBase

    def set_image_path(self, image_path, sign_id, crypto_params={},
                       imageinfo_class=ImageInfoIot):
        """Sets the image that will be processed.

        :param str image_path: Path to the image to be processed
        :param str sign_id: sign_id corresponding to the image_path. This must
            be one of the :meth:`available_sign_ids`
        :param dict crypto_params: cryptography parameters used for signing
        """
        if self._img_config_parser is None:
            raise RuntimeError('Set chipset before setting the image path.')

        # Unlike secimage, use the same IoT config for parsegen config.
        self._parsegen_config = IotParsegenCfgParser(
            self._img_config_parser.config_path,
            defines=self.cfgparser.defines,
            auto_gen_xml_config=self.cfgparser.auto_gen_xml_config,
            auto_gen_obj_config=self.cfgparser.auto_gen_obj_config)

        self._stager = ImagePathsStagerIot(
            image_path, self._img_config_parser, self._parsegen_config, self.authority,
            sign_id=sign_id, crypto_params=crypto_params,
            imageinfo_class=imageinfo_class)

    def process(self,
                integrity_check=False,
                sign=False,
                val_image=False,
                val_integrity_check=False,
                val_sign=False,
                root_cert_hash=None,
                progress_cb=PROGRESS_CB_PYPASS,
                ):
        """Performs the secure-image related operations specified from the params.

        :param bool integrity_check: Add integrity check to the image.
        :param bool sign: Sign the images. (Re-sign if image is already signed)
        :param bool val_image: Validate the integrity of the image against the config file.
        :param bool val_integrity_check: Validate the integrity check in the image.
        :param bool val_sign: Validate that the image is signed and validate the integrity of the signing related data.
        :param cb progress_cb: Callback method to get a status update during processing.

            Callback method should have this format:
            ::
                def progress_cb(status_string, progress_percent):
                    \"""
                    :param str status_string: the current status.
                    :param int progress_percent: the progress (in percent)
                    \"""
                    ...
        """
        # Print the openssl path
        version = ''
        path_info = ('is unavailable. Please run "which openssl" and "openssl version" to '
                     'check openssl version info, and upgrade to required version')
        try:
            version = 'v' + '.'.join([str(x) for x in crypto.openssl.OPENSSL_VERSION_MIN]) + ' or greater '
            path_info = 'is available at: "' + crypto.discovery_factory.get_impl(crypto.modules.MOD_OPENSSL) + '"'
        except Exception as e:
            pass
        logger.info('Openssl ' + version + path_info)

        # Start processing images
        assert len(self.image_info_list) == 1, "multiple images are found"
        progress = ProgressNotifier(1, progress_cb, PROGRESS_TOTAL_STAGES)

        parsegens = dict()

        def _process(idx, image, integrity_check, sign,
                     val_image, val_integrity_check, val_sign):
            assert isinstance(image, ImageInfoIot)

            logger.info('------------------------------------------------------')
            status_string = ('Processing: ' + image.image_under_operation)
            logger.info(status_string + '\n')

            # Send a progress notification to the toplevel
            progress.status = status_string
            progress.cur = idx
            progress.cur_stage = 0

            file_logger_id = None
            image.authority = self.authority

            try:
                # Create the required directory structure for this image
                image_output_dir = image.dest_image.image_dir
                try:
                    c_path.create_dir(image_output_dir)
                except Exception as e:
                    raise RuntimeError('Could not create output directory: ' + image_output_dir + '\n'
                                       '    ' + 'Error: ' + str(e))

                # Enable/Disable debug
                image.dest_image.debug_enable = self.debug
                c_path.create_debug_dir(image.dest_image.debug_dir)

                # Set the root cert hash
                image.validation_root_cert_hash = root_cert_hash

                # Enable file logging to the directory
                file_logger_id = logger.add_file_logger(c_path.join(image_output_dir, 'IoT_log.txt'),
                                                        logger.verbosity)

                # Create the security policies list for this image
                security_policy_list = create_security_policy_list(image)

                # Parsegen object
                parsegen = None

                # For secure operations
                if integrity_check or sign:
                    parsegen = self._process_secure_operation(
                        image, progress, security_policy_list, integrity_check, sign,
                        parsegens=parsegens.get(image.chipset, list()))

                # For validation
                if val_image or val_integrity_check or val_sign:
                    parsegen = self._process_validation(
                        image, progress, security_policy_list, val_image, val_integrity_check, val_sign)

                # Print the image data
                if parsegen is not None:
                    logger.info('\n' + str(parsegen))

                # Set overall processing to true
                if not ((val_image and image.status.validate_parsegen.state == StatusInfo.ERROR) or
                        (val_integrity_check and image.status.validate_integrity_check.state == StatusInfo.ERROR) or
                        (val_sign and image.status.validate_sign.state == StatusInfo.ERROR)):
                    image.status.overall.state = StatusInfo.SUCCESS

            except Exception:
                logger.error(traceback.format_exc())
                logger.error(sys.exc_info()[1])
            finally:
                if file_logger_id is not None:
                    logger.removeFileLogger(file_logger_id)

            logger.info('------------------------------------------------------\n')

        for idx, image in enumerate(self.image_info_list):
            _process(idx, image, integrity_check, sign,
                     val_image, val_integrity_check, val_sign)

        progress.complete()
