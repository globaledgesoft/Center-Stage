# ===============================================================================
#
#  Copyright (c) 2013-2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#
# ===============================================================================

from sectools.common.utils.c_data import remove_duplicates
from sectools.common.utils.c_rule import CoreRuleBase

from sectools.features.isc.iot.cfgparser.auto_gen_xml_config import complex_metadata
from sectools.features.isc.iot.cfgparser.auto_gen_xml_config import complex_images_list
from sectools.features.isc.iot.cfgparser.auto_gen_xml_config import complex_general_properties

from . import defines


class IoTConfigRulesManager(CoreRuleBase):
    """
    This is the main SecImage config rules manager that runs the rules
    """
    def __init__(self):
        self.configs = {}
        self.configs['images_list'] = _ImageList()
        self.configs['general_properties'] = _GeneralProperties()
        self.configs['metadata'] = None
        self.configs['parsegen'] = None

    def validate(self, data, data_dict):
        retval = True
        error_str = ''

        # based on the dictionary structure, go through each section from root
        for name in data_dict:
            # check if there's a registered rule object for this config section
            if name in self.configs and self.configs[name] is not None:
                config = getattr(data.root, name)
                if name == 'images_list':
                    ret, error = self.configs[name].validate(
                        config, getattr(data.root, 'general_properties'),
                        getattr(data.root, 'metadata'))
                else:
                    ret, error = self.configs[name].validate(config, config)

                # accumulate the return values
                if ret == False:
                    retval &= ret
                    error_str += '\n\n<%s>%s' % (name, error)

        if retval == False:
            raise RuntimeError(
                '\nIoT config validation failed with following error(s): %s' %
                error_str)


class _Signing(object):
    """
    Defines the rules for signing default attributes
    """

    def __init__(self):
        pass

    def validate(self, signing, *args):
        retval = True
        error_str = ''
        return retval, error_str


class _GeneralProperties(object):
    """
    Defines the rules for general properties
    """

    def __init__(self):
        pass

    def validate(self, general_properties, *args):
        assert(isinstance(general_properties, complex_general_properties))

        retval = True
        error_str = ''

        return retval, error_str


class _ImageList(object):
    """
    Defines the rules for image parameters to be signed
    """

    def __init__(self):
        self.mask_warning = True
        self.version_warning = True

    def validate(self, images, *args):
        assert(isinstance(images, complex_images_list))

        image_list = images.get_image()
        retval = [True]
        errors = []

        def add_error(sign_id, error):
            retval[0] = False
            errors.append("\nsign_id={0}: ".format(sign_id) + error)

        # expect args[0] to be instance of signing
        assert(isinstance(args[0], complex_general_properties))
        assert(isinstance(args[1], complex_metadata))

        general_properties = args[0]

        # Not Overridable
        use_serial_number_in_signing =\
            general_properties.get_use_serial_number_in_signing()

        attributes = dict(
            hash_algorithm=general_properties.get_hash_algorithm(),
            hmac=general_properties.get_hmac(),
            rsa_padding=general_properties.get_rsa_padding(),
            num_root_certs=general_properties.get_num_root_certs(),
            secboot_version=general_properties.get_secboot_version(),
            dsa_type=general_properties.get_dsa_type(),
            key_size=general_properties.get_key_size(),
            exponent=general_properties.get_exponent())

        # Overridable
        default_debug = general_properties.get_debug()
        default_sw_id = general_properties.get_sw_id()
        default_oem_id = general_properties.get_oem_id()
        default_model_id = general_properties.get_model_id()
        default_msm_part = general_properties.get_msm_part()
        default_soc_hw_version = general_properties.get_soc_hw_version()
        default_in_use_soc_hw_version = general_properties.get_in_use_soc_hw_version()

        for image in image_list:
            sign_id = image.get_sign_id()
            overrides = image.get_general_properties_overrides()

            # Update all the overridable attributes
            for attr in defines.CONFIG_STRUCTURE["images_list"]["image"][0][
                                        "general_properties_overrides"].keys():
                attr_override = getattr(overrides, "get_" + attr)()
                if attr_override is None:
                    attributes[attr] = locals()["default_" + attr]
                else:
                    attributes[attr] = attr_override

        return retval[0], "".join(remove_duplicates(errors))
