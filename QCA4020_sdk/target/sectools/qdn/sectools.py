# ===============================================================================
#
# Copyright (c) 2013-2018 Qualcomm Technologies, Inc.
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

"""Provides a command line interface to the services provided by sectools

.. data:: SECTOOLS_TOOL_NAME

    Name of the tool

.. data:: SECTOOLS_TOOL_VERSION

    Version of the tool
"""


import os
import sys
import optparse
import traceback

from sectools import SECTOOLS_TOOL_NAME
from sectools import SECTOOLS_TOOL_VERSION
from sectools.common.utils.c_logging import logger


# List of features
FEATURES_LIST = []

sectools_features_package_name = "sectools.features"

sectools_features_path = os.path.join(
    os.path.realpath(os.path.dirname(__file__)),
    *(sectools_features_package_name.split(".")))

for module_name in os.listdir(sectools_features_path):

    def import_features(module_path):
        """  Import all features under the module path recursively.

        :param module_path:
            Python module path, such as sectools.features.isc.
            It is already imported, and all its attributes are visible.

            The __init__.py file of a module to be imported could have
            only either or both attributes, the singular __sectools_feature__,
            and the plural __sectools_features__. The former is a string
            referring to a potential feature Python module file, while the
            latter is a list of strings referring to their respective
            potential feature Python module files or packages. Again, they
            are candidates. The logic implemented here determines whether
            the feature candidate is actually imported as a sectools feature.

            If neither attributes is present, then no feature is imported.
        """
        if hasattr(sys.modules[module_path], "__sectools_features__"):
            for feature_package in getattr(
                    sys.modules[module_path], "__sectools_features__"):
                feature_module_path = ".".join([module_path, feature_package])
                try:
                    __import__(feature_module_path)
                except ImportError:
                    continue
                import_features(feature_module_path)

        if hasattr(sys.modules[module_path], "__sectools_feature__"):
            feature_name = getattr(sys.modules[module_path],
                                   "__sectools_feature__")
            __import__(".".join([module_path, feature_name]))
            feature = getattr(sys.modules[module_path], feature_name)
            FEATURES_LIST.append(feature)

    if not os.path.isdir(os.path.join(sectools_features_path, module_name)):
        continue

    if hasattr(sectools_features_package_name, module_name):
        continue

    module_path = ".".join([sectools_features_package_name, module_name])
    try:
        __import__(module_path)
    except ImportError:
        continue

    import_features(module_path)


if not FEATURES_LIST:
    raise RuntimeError('Sectools could not find any packaged features')

__version__ = SECTOOLS_TOOL_NAME + ' ' + SECTOOLS_TOOL_VERSION


class SectoolsParser(optparse.OptionParser):
    """Parser for command line arguments supported by Sectools."""

    def __init__(self):
        # Initialize the base parser
        optparse.OptionParser.__init__(self, usage=self.c_usage,
                                       description=self.c_description,
                                       version=self.c_version,
                                       epilog=self.c_epilog)
        self.c_add_options()
        self.opt_args, self.pos_args = self.parse_args(args=sys.argv[:2])

        if len(self.pos_args) == 1:
            self.print_help(sys.stdout)

    @property
    def c_usage(self):
        """(str) Returns the usage of the program.
        """
        return self.c_prog + ' [feature]'

    @property
    def c_prog(self):
        """(str) Returns the name of the program. By default this is the name
        of the python file being executed.
        """
        return os.path.basename(sys.argv[0])

    @property
    def c_description(self):
        """(str) Returns the description of the program."""
        return 'This program provides an interface to the sectools features'

    @property
    def c_version(self):
        """(str) Returns the version of the program."""
        return __version__

    @property
    def c_epilog(self):
        """(str) Returns the epilog for the program."""
        feature_command_names = sorted(
            [f.CMD_ARG_TOOL_NAME for f in FEATURES_LIST])
        features = "\n".join('    %d. %s' % (idx, feature_name) for
                             idx, feature_name in
                             enumerate(feature_command_names, start=1))
        example = self.c_prog + ' ' + feature_command_names[-1] + ' -h'
        return """
Features available for sectools are:
{features}

Example usage:
    {example}
""".format(features=features, example=example)

    def format_epilog(self, formatter):
        """This method is implemented to override the OptionParser's formatting
        of the epilog"""
        return self.epilog

    def c_add_options(self):
        """Adds the command line args supported by sectools."""
        pass


def main(args):
    """Parses the command line arguments, performs any basic operations based on
    the parsed arguments and starts processing using the isc module.
    """
    # Print the tool's launch command
    logger.debug2('\n\n    Sectools launched as: "' + ' '.join(sys.argv) + '"\n')

    if len(args) > 1:
        feature = args[1]
        for supported_feature in FEATURES_LIST:
            if feature == supported_feature.CMD_ARG_TOOL_NAME:
                supported_feature.main(supported_feature.parse_args(sys.argv[1:]))
                break
        else:
            raise RuntimeError(
                'Feature provided from command line: "' + feature + '" is invalid.' + '\n'
                '       ' + 'Please choose from : ' + str([f.CMD_ARG_TOOL_NAME for f in FEATURES_LIST]))


if __name__ == '__main__':

    try:
        # Check that the command line are valid and are normalized.
        args = SectoolsParser().pos_args

        main(args)

    except Exception:
        logger.error(traceback.format_exc())
        logger.error(sys.exc_info()[1])
        sys.exit(1)

    except KeyboardInterrupt:
        print
        logger.error('Keyboard Interrupt Received. Exiting!')
        sys.exit(1)

    sys.exit(0)
