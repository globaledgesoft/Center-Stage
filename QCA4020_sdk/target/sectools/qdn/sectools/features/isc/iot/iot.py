# ===============================================================================
#
#  Copyright (c) 2013-2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#
# ===============================================================================

import os
import sys
import traceback

from sectools.common.utils import c_path
from sectools.common.utils.c_base import dynamicToolStatus
from sectools.common.utils.c_base import CoreOptionParser
from sectools.common.utils.c_logging import logger
from sectools.common.utils.c_misc import TablePrinter
from sectools.common.utils.c_config_overrides import CfgOverrides

from sectools.features.isc.cfgparser import CfgParserNamedTuple

from sectools.features.isc.iot.cfgparser import defines
from sectools.features.isc.iot.cfgparser import auto_gen_obj_config
from sectools.features.isc.iot.cfgparser import auto_gen_xml_config
from sectools.features.isc.iot.cfgparser.rule import IoTConfigRulesManager

from .secimage_iot import SecImageIotCore

iot_cfgparser = CfgParserNamedTuple(
    "1.0", IoTConfigRulesManager(), defines, auto_gen_obj_config, auto_gen_xml_config)

# Tool name for command arg
CMD_ARG_TOOL_NAME = 'iot'

# Name & version of the tool
IoT_SECIMAGE_TOOL_NAME = 'IoT'
IoT_SECIMAGE_TOOL_VERSION = '1.0'

# Path definitions
DEF_IoTIMAGE_OUTPUT_DIR_NAME = CMD_ARG_TOOL_NAME + '_output'
DEF_IoTIMAGE_OUTPUT_DIR_PATH = c_path.join(dynamicToolStatus.toolDir, DEF_IoTIMAGE_OUTPUT_DIR_NAME)

__version__ = IoT_SECIMAGE_TOOL_NAME + ' ' + IoT_SECIMAGE_TOOL_VERSION

IoT_CONFIG_DIR = os.path.realpath(os.path.join(os.path.dirname(__file__), *([".."] * 4)))


class SecImageIotParser(CoreOptionParser):
    """Parser for command line arguments supported by SecImage."""

    _LIST_TAG = 'LIST'

    # The prefix used for config overrides
    override_prefix = 'cfg_'

    def __init__(self, argv=None):
        self.overrides = self.init_overrides
        super(SecImageIotParser, self).__init__(argv=argv)

    @property
    def c_description(self):
        """(str) Returns the description of the program."""
        return 'This program signs & validates IoT images'

    @property
    def c_version(self):
        """(str) Returns the version of the program."""
        return __version__

    @property
    def init_overrides(self):
        return CfgOverrides(
            defines.CONFIG_STRUCTURE["images_list"]["image"][0][
                "general_properties_overrides"])

    @property
    def c_epilog(self):
        """(str) Returns the epilog for the program."""
        prog = self.c_prog
        _list = self._LIST_TAG
        return ('\n'
                'Extended Usage:' + '\n'
                '1. To get a list of available chipsets: ' + '\n'
                '      ' + prog + ' -h --chipset ' + _list + '\n'
                '2. To get a list of available sign ids for a certain chipset: ' + '\n'
                '      ' + prog + ' -h --chipset <id> --sign_id ' + _list + '\n'
                )

    @property
    def c_epilog_from_params(self):
        """(str) Based on the arguments provided, return the extended epilog
        message.
        """
        args = self.parsed_args
        help_obj = SecImageIotCore(
            config_dir=IoT_CONFIG_DIR, parsegen_config=None,
            cfgparser=iot_cfgparser, debug=args.debug)
        epilog = ''

        if args.chipset == self._LIST_TAG:
            # Compile a list of available chipsets
            epilog += '\n' + 'Chipsets available: ' + '\n'
            for idx, chipset in enumerate(help_obj.available_chipsets, start=1):
                epilog += '%d. %s\n' % (idx, chipset)
        elif (args.sign_id == self._LIST_TAG and args.chipset != self._LIST_TAG):
            # Extended Feature 2
            if args.chipset:
                # Compile a list of available sign ids for a chipset
                help_obj.chipset = args.chipset
                epilog += '\n' + 'Sign-ids available for chipset "' + args.chipset + '": ' + '\n'
                for idx, sign_id in enumerate(help_obj.available_sign_ids, start=1):
                    epilog += '%d. %s\n' % (idx, sign_id)

        return epilog

    @staticmethod
    def real_path(cmdline_option, actual_option, value, parser):
        setattr(parser.values, cmdline_option.dest, os.path.realpath(value))

    def c_add_options(self):
        """ Command-line arguments supported by iot. """
        sign_img_group = self.add_option_group('Signing options')
        sign_img_group.add_option('-i', '--image_file', metavar='<file>',
                                  type="str", action="callback", callback=self.real_path,
                                  help='path to the image file to be signed')
        sign_img_group.add_option('-g', '--sign_id', metavar='<sign_id>',
                                  help='sign id corresponding to the image_file provided.')
        sign_img_group.add_option('-p', '--chipset', metavar='<chipset_id>',
                                  help='chipset ID corresponding to the image_file')
        sign_img_group.add_option('-c', '--root_cert', metavar='<cert>',
                                  type="str", action="callback", callback=self.real_path,
                                  help="file path to a root certificate in either DER or PEM format")
        sign_img_group.add_option('-k', '--root_key', metavar='<key>',
                                  type="str", action="callback", callback=self.real_path,
                                  help="file path to a root private key in either DER or PEM format")

        # Specifying the output location
        output_group = self.add_option_group('Output options')
        output_group.add_option('-o', '--output_dir', metavar='<dir>',
                                type="str", action="callback", callback=self.real_path,
                                help='directory to store output files. DEFAULT: "./%s"' %
                                     DEF_IoTIMAGE_OUTPUT_DIR_NAME,
                                default=DEF_IoTIMAGE_OUTPUT_DIR_PATH)

        # Specifying the operation
        operations_group = self.add_option_group('Image operation options')
        operations_group.add_option('-s', '--sign', action='store_true',
                                    default=False, help='sign the image.')
        operations_group.add_option('-a', '--validate', action='store_true',
                                    default=False, help='validate the image.')
        operations_group.add_option('-t', '--integrity_check', action='store_true',
                                    default=False, help='add hash table segment.')

        # Populate override options
        override_group = self.add_option_group('Config Override options')
        properties = self.overrides.get_properties()
        for tag in sorted(properties.keys()):
            override = properties[tag]
            metavar = '<' + override.type_str + '_value>'
            if override.is_choice:
                choices = override.choices
                if () in choices:
                    choices.remove(())
                metavar = "<%s_value: %s>" % (override.type_str, choices)
            override_group.add_option('--' + self.override_prefix + tag,
                                      metavar=metavar)

    def c_update_overrides(self):
        """ Update override options """
        for tag, override in self.overrides.root.children.items():
            value_to_set = getattr(self.parsed_args, self.override_prefix + tag)
            if value_to_set is not None:
                try:
                    override.set_string_value(value_to_set)
                except RuntimeError as e:
                    raise RuntimeError(tag + ' ' + str(e))

    def c_validate(self):
        """Validates the command line args provided by the user.

        :raises: RuntimeError if any error occurs.
        """
        args = self.parsed_args
        err = []

        # Check the input files
        if not args.image_file:
            err.append('Provide an image_file for processing.')

        if args.chipset is None:
            err.append('Provide chipset to process the image.')

        if args.integrity_check or args.sign or args.validate:
            # Validate the output image resulting from all operations
            args.validate = True
        else:
            err.append('Specify one or more operations to perform.')

        # Check root certificate and key
        if args.sign:
            if not args.root_cert:
                err.append('Provide a root certificate file.')
            if not args.root_key:
                err.append('Provide a root private key file.')

        # Check and sanitize any paths for read access
        for path in ['image_file', 'root_cert', 'root_key']:
            path_val = getattr(args, path, None)
            # the file path is normalized earlier as part of its callback
            if args.sign:
                if getattr(args, path) and not c_path.validate_file(path_val):
                    err.append('Cannot access %s at: %s' % (path, path_val))

        # Check and sanitize paths for write access
        for path in ['output-dir']:
            path_val = getattr(args, path.replace('-', '_'), None)
            try:
                c_path.create_dir(path_val)
            except Exception as e:
                err.append('Cannot write at: %s\n    Error: %s' % (path_val, e))

        # Raise error if any
        if err:
            if len(err) > 1:
                err = [('  ' + str(idx + 1) + '. ' + error) for idx, error in enumerate(err)]
                err = 'Please check the command line args:\n\n' + '\n'.join(err)
            else:
                err = err[0]
            raise RuntimeError(err)


def print_summary(args, image_info_list):
    """Prints the summary of the actions performed by SecImage"""

    if not image_info_list:
        return

    # Check which actions were performed
    actions = []

    if args.sign:
        actions.append('sign')
    if args.validate:
        actions.append('validate')
    if args.integrity_check:
        actions.append('integrity_check')

    if not actions:
        return

    # Figure out the output directory
    if args.output_dir:
        output_print = 'Output is saved at: %s\n' % args.output_dir
    else:
        output_print = '\n'

    # Log the actions and output directory
    actions_str = 'Following actions were performed: "%s"\n' % ', '.join(actions)
    logger.info('SUMMARY:' + '\n' + actions_str + output_print)

    # Table information
    summary_table = TablePrinter([1])
    (COLUMN_PARSE,
     COLUMN_INTEGRITY_CHECK,
     COLUMN_SIGN,
     COLUMN_VAL_PARSE,
     COLUMN_VAL_INTEGRITY_CHECK,
     COLUMN_VAL_SIGN) = range(6)

    # First row
    row = 0
    summary_table.insert_data(row, COLUMN_PARSE, 'Parse')
    summary_table.insert_data(row, COLUMN_INTEGRITY_CHECK, 'Integrity')
    summary_table.insert_data(row, COLUMN_SIGN, 'Sign')
    summary_table.insert_data(row, COLUMN_VAL_PARSE, 'Validate', column_end=COLUMN_VAL_SIGN)

    # Second row
    row += 1
    summary_table.insert_data(row, COLUMN_VAL_PARSE, 'Parse')
    summary_table.insert_data(row, COLUMN_VAL_INTEGRITY_CHECK, 'Integrity')
    summary_table.insert_data(row, COLUMN_VAL_SIGN, 'Sign')

    # Data rows
    for image in image_info_list:
        row += 1
        summary_table.insert_data(row, COLUMN_PARSE, image.status.parsegen.state)
        summary_table.insert_data(row, COLUMN_INTEGRITY_CHECK, image.status.integrity_check.state)
        summary_table.insert_data(row, COLUMN_SIGN, image.status.sign.state)
        summary_table.insert_data(row, COLUMN_VAL_PARSE, image.status.validate_parsegen.state)
        summary_table.insert_data(row, COLUMN_VAL_INTEGRITY_CHECK, image.status.validate_integrity_check.state)
        summary_table.insert_data(row, COLUMN_VAL_SIGN, image.status.validate_sign.state)

    logger.info(summary_table.get_data())


def main(args, return_isc=False):
    """Parses the command line arguments, performs any basic operations based on
    the parsed arguments and starts processing using the isc module.
    """
    # Log to file
    flids = logger.log_to_file(IoT_SECIMAGE_TOOL_NAME, args.output_dir)

    try:
        # Print the tool's launch command
        # logged_args = sys.argv
        logger.info('\n\n    IoT launched as: "' + ' '.join(sys.argv) + '"\n')

        # Initialize SecImageIotCore
        # The parsegen_config is set to None for now. It will be initialized
        # later before set_image_path.
        isc = SecImageIotCore(config_dir=IoT_CONFIG_DIR, parsegen_config=None,
                              cfgparser=iot_cfgparser, debug=args.debug)

        # Configure image signer
        if args.image_file:
            isc.set_chipset(args.chipset, args.overrides)

            crypto_params = {}
            if args.sign:
                crypto_params.update(
                    root_certificate_properties=dict(
                        certificate_path=args.root_cert,
                        private_key_path=args.root_key))

            # Set the input, it must be done after isc.set_chipset(...)
            #
            # All actual IDs are passed to the back-end, along with root cert
            # and root private key as crypto_params.
            isc.set_image_path(args.image_file, args.sign_id,
                               crypto_params=crypto_params)

        # Set the output
        if args.output_dir:
            isc.output_dir = args.output_dir

        # Process the images
        isc.process(sign=args.sign,
                    integrity_check=args.integrity_check,
                    val_sign=args.validate,
                    val_image=args.validate,
                    val_integrity_check=args.validate)

        # Print the summary
        print_summary(args, isc.image_info_list)

        if return_isc:
            return isc
        else:
            return isc.image_info_list

    finally:
        # Clear all log handlers
        logger.removeFileLogger(flids)


def parse_args(argv):
    parser = SecImageIotParser(argv)
    setattr(parser.parsed_args, 'overrides', parser.overrides)
    return parser.parsed_args


if __name__ == '__main__':

    try:
        # Call the main of the tool
        main(parse_args(sys.argv))

    except Exception:
        logger.debug(traceback.format_exc())
        logger.error(sys.exc_info()[1])
        sys.exit(1)

    except KeyboardInterrupt:
        print
        logger.error('Keyboard Interrupt Received. Exiting!')
        sys.exit(1)

    sys.exit(0)
