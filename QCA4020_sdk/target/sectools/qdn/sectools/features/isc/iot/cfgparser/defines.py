# ===============================================================================
#
#  Copyright (c) 2013-2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#
# ===============================================================================

"""Contains constants related to the cfgparser package.

Constants relate to:

#. Common constants
#. Auto-generated XML Parser - auto_gen_xml_config
#. Auto-generated OBJ - auto_gen_obj_config
#. Config Data Structure
"""
import os

from sectools.common.utils import c_path
from sectools.common.utils import c_config
from sectools.features.isc.defines import SECBOOT_VERSION_1_0

CONFIG_KEY_IMAGE_TYPE = 'image_type'

CONFIG_STRUCTURE_IMAGE_TYPE = {
    'id' : '',
    'file_type' : '',
    'mbn_properties': {
        c_config.DICT.DictHandlerKey_Attr: [c_config.DICT.DictHandlerAttr_Optional],
        'header_size' : 0,
    },
    'elf_properties': {
        'image_type' : 0,
        'max_elf_segments' : (0,),
        'hash_seg_placement' : (
            ('POST_PHDR_ALIGNED', 'POST_PHDR', 'POST_PHDR_LOAD_ALIGNED',
             'END_64B_ALIGNED', 'END_128B_ALIGNED', ()),),
        'validate_ph_addrs': (True,),
        'validate_vir_addrs': (False,),
    },
}

#------------------------------------------------------------------------------
# Common constants
#------------------------------------------------------------------------------
# Absolute path of the package
CFGPARSER_PATH = os.path.relpath(os.path.dirname(__file__))

# Name of the root node of any config related objects
ROOTNODE_NAME = 'iot'

# Name of the directory containing chip specific folders
CONFIG_DIR_BASENAME = 'config'

#------------------------------------------------------------------------------
# Auto-generated XML Parser related information
#------------------------------------------------------------------------------
# XML - Basic information
XML_PY_FILE_NAME = 'auto_gen_xml_config.py'
XML_PY_PATH = c_path.join(CFGPARSER_PATH, XML_PY_FILE_NAME)
XML_NAME_ENDING = '_' + ROOTNODE_NAME + '.xml'
XML_NAME_REGEX = '[^_]+_' + ROOTNODE_NAME + '\.xml'

# XML - lines at the beginning of the file
XML_PREPEND_LINES = '<?xml version="1.0" encoding="UTF-8"?>\n'

# XML - rootnode related constants
XML_ROOTNODE_NAMESPACE = 'tns:'
XML_ROOTNODE_NAME = ROOTNODE_NAME
XML_ROOTNODE = XML_ROOTNODE_NAMESPACE + XML_ROOTNODE_NAME

# XML - namespace related constants
XML_NAMESPACE_TNS = 'xmlns:tns="http://www.qualcomm.com/iot"'
XML_NAMESPACE_W3 = 'xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"'
XML_NAMESPACE_SCHEMA = 'xsi:schemaLocation="http://www.qualcomm.com/iot ../xsd/iot.xsd"'
XML_NAMESPACE = XML_NAMESPACE_TNS + '\n\t' + XML_NAMESPACE_W3 + '\n\t' + XML_NAMESPACE_SCHEMA

# XML - classname generator
XML_CLASSNAME_GEN = lambda x: 'complex_' + x


#------------------------------------------------------------------------------
# Auto-generated OBJ Parser related information
#------------------------------------------------------------------------------
# OBJ - Basic information
OBJ_PY_FILE_NAME = 'auto_gen_obj_config.py'
OBJ_PY_PATH = c_path.join(CFGPARSER_PATH, OBJ_PY_FILE_NAME)

# OBJ - lines at the beginning of the file
OBJ_PREPEND_LINES = '"""\nAuto generated classes\n"""\n\n'

# OBJ - classname generator
OBJ_CLASSNAME_GEN = lambda x: 'Cfg_' + x.replace('_', ' ').title().replace(' ', '_')

#------------------------------------------------------------------------------
# Config Data Structure
#------------------------------------------------------------------------------
# general properties block
CONFIG_KEY_GENERAL_PROPERTIES = 'general_properties'
CONFIG_STRUCTURE_GENERAL_PROPERTIES = {
    'selected_signer'               : ((''),),
    'selected_cert_config'          : '',
    'hash_algorithm'                : (('sha256', (),),),
    'segment_hash_algorithm'        : (('sha256', (),),),
    'max_cert_size'                 : ((2048, ()),),
    'key_size'                      : ((2048, ()),),
    'num_certs_in_certchain'        : ((2, ()),),
    'num_root_certs'                : ((1, ()),),
    'max_num_root_certs'            : ((1, ()),),
    'msm_part'                      : (c_config.HEX32,),
    'oem_id'                        : (c_config.HEX16,),
    'model_id'                      : (c_config.HEX16,),
    'sw_id'                         : (c_config.HEX64,),
    'soc_hw_version'                : (c_config.HEX32,),
    'in_use_soc_hw_version'         : ((0, 1, (),),),
    'use_serial_number_in_signing'  : ((0, (),),),
    'debug'                         : (c_config.HEX64,),
    'exponent'                      : ((257, ()),),
    'secboot_version'               : ((SECBOOT_VERSION_1_0, ()),),
    'oem_sign'                      : ((True, ()),),
    'rsa_padding'                   : (('pkcs', (),),),
    'hmac'                          : ((True, ()),),
    'dsa_type'                      : (('rsa', (),),),
}

# top-level block
CONFIG_KEY = ROOTNODE_NAME
CONFIG_STRUCTURE = {
    'metadata': {
        'chipset': '',
        'version': '',
    },

    CONFIG_KEY_GENERAL_PROPERTIES: CONFIG_STRUCTURE_GENERAL_PROPERTIES,

    'parsegen': {
        c_config.DICT.DictHandlerKey_Attr: [c_config.DICT.DictHandlerAttr_Optional],
        'image_types_list': {
            c_config.DICT.DictHandlerKey_Attr : [c_config.DICT.DictHandlerAttr_Optional],
            CONFIG_KEY_IMAGE_TYPE : [CONFIG_STRUCTURE_IMAGE_TYPE],
        }
    },

    'data_provisioning': {
        'base_path' : c_config.PATH,
    },

    'images_list': {
        'image': [
            {
                'sign_id': '',
                'name': ('',),
                'image_type': '',
                'cert_config': ('',),
                'general_properties_overrides': {
                    'msm_part'                      : (c_config.HEX32,),
                    'soc_hw_version'                : (c_config.HEX32,),
                    'in_use_soc_hw_version'         : ((0, 1, (),),),
                    'oem_id'                        : (c_config.HEX16,),
                    'model_id'                      : (c_config.HEX16,),
                    'sw_id'                         : (c_config.HEX64,),
                    'debug'                         : (c_config.HEX64,),
                },
            }
        ]
    }
}
