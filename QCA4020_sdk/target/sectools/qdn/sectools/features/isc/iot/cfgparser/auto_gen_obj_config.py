"""
Auto generated classes
"""

class Cfg_Iot:

    def __init__(self):
        self.parsegen = Cfg_Parsegen()
        self.metadata = Cfg_Metadata()
        self.general_properties = Cfg_General_Properties()
        self.images_list = Cfg_Images_List()
        self.data_provisioning = Cfg_Data_Provisioning()

    def __str__(self):
        return 'Instance of ' + self.__class__.__name__

    def __repr__(self):
        retval = ''
        for attr in sorted(self.__dict__.keys()):
            value = self.__dict__[attr]
            retval += (str(attr) + ': ' + '\n'
                       '    ' + repr(value) + '\n')
        return (self.__class__.__name__ + ':' + '\n' +
                '\n'.join(['  ' + line for line in retval.split('\n')]))


class Cfg_Parsegen:

    def __init__(self):
        self.image_types_list = Cfg_Image_Types_List()

    def __str__(self):
        return 'Instance of ' + self.__class__.__name__

    def __repr__(self):
        retval = ''
        for attr in sorted(self.__dict__.keys()):
            value = self.__dict__[attr]
            retval += (str(attr) + ': ' + '\n'
                       '    ' + repr(value) + '\n')
        return (self.__class__.__name__ + ':' + '\n' +
                '\n'.join(['  ' + line for line in retval.split('\n')]))


class Cfg_Image_Types_List:

    def __init__(self):
        self.image_type = []

    def __str__(self):
        return 'Instance of ' + self.__class__.__name__

    def __repr__(self):
        retval = ''
        for attr in sorted(self.__dict__.keys()):
            value = self.__dict__[attr]
            retval += (str(attr) + ': ' + '\n'
                       '    ' + repr(value) + '\n')
        return (self.__class__.__name__ + ':' + '\n' +
                '\n'.join(['  ' + line for line in retval.split('\n')]))


class Cfg_Image_Type:

    def __init__(self):
        self.file_type = ''
        self.mbn_properties = Cfg_Mbn_Properties()
        self.id = ''
        self.elf_properties = Cfg_Elf_Properties()

    def __str__(self):
        return 'Instance of ' + self.__class__.__name__

    def __repr__(self):
        retval = ''
        for attr in sorted(self.__dict__.keys()):
            value = self.__dict__[attr]
            retval += (str(attr) + ': ' + '\n'
                       '    ' + repr(value) + '\n')
        return (self.__class__.__name__ + ':' + '\n' +
                '\n'.join(['  ' + line for line in retval.split('\n')]))


class Cfg_Mbn_Properties:

    def __init__(self):
        self.header_size = 0

    def __str__(self):
        return 'Instance of ' + self.__class__.__name__

    def __repr__(self):
        retval = ''
        for attr in sorted(self.__dict__.keys()):
            value = self.__dict__[attr]
            retval += (str(attr) + ': ' + '\n'
                       '    ' + repr(value) + '\n')
        return (self.__class__.__name__ + ':' + '\n' +
                '\n'.join(['  ' + line for line in retval.split('\n')]))


class Cfg_Elf_Properties:

    def __init__(self):
        self.image_type = 0
        self.max_elf_segments = 0
        self.validate_vir_addrs = False
        self.hash_seg_placement = ''
        self.validate_ph_addrs = True

    def __str__(self):
        return 'Instance of ' + self.__class__.__name__

    def __repr__(self):
        retval = ''
        for attr in sorted(self.__dict__.keys()):
            value = self.__dict__[attr]
            retval += (str(attr) + ': ' + '\n'
                       '    ' + repr(value) + '\n')
        return (self.__class__.__name__ + ':' + '\n' +
                '\n'.join(['  ' + line for line in retval.split('\n')]))


class Cfg_Metadata:

    def __init__(self):
        self.chipset = ''
        self.version = ''

    def __str__(self):
        return 'Instance of ' + self.__class__.__name__

    def __repr__(self):
        retval = ''
        for attr in sorted(self.__dict__.keys()):
            value = self.__dict__[attr]
            retval += (str(attr) + ': ' + '\n'
                       '    ' + repr(value) + '\n')
        return (self.__class__.__name__ + ':' + '\n' +
                '\n'.join(['  ' + line for line in retval.split('\n')]))


class Cfg_General_Properties:

    def __init__(self):
        self.msm_part = ''
        self.max_cert_size = 2048
        self.selected_cert_config = ''
        self.use_serial_number_in_signing = 0
        self.in_use_soc_hw_version = 0
        self.exponent = 257
        self.num_certs_in_certchain = 2
        self.dsa_type = ''
        self.key_size = 2048
        self.model_id = ''
        self.segment_hash_algorithm = ''
        self.num_root_certs = 1
        self.hash_algorithm = ''
        self.sw_id = ''
        self.secboot_version = ''
        self.max_num_root_certs = 1
        self.oem_sign = True
        self.soc_hw_version = ''
        self.rsa_padding = ''
        self.selected_signer = ''
        self.debug = ''
        self.oem_id = ''
        self.hmac = True

    def __str__(self):
        return 'Instance of ' + self.__class__.__name__

    def __repr__(self):
        retval = ''
        for attr in sorted(self.__dict__.keys()):
            value = self.__dict__[attr]
            retval += (str(attr) + ': ' + '\n'
                       '    ' + repr(value) + '\n')
        return (self.__class__.__name__ + ':' + '\n' +
                '\n'.join(['  ' + line for line in retval.split('\n')]))


class Cfg_Images_List:

    def __init__(self):
        self.image = []

    def __str__(self):
        return 'Instance of ' + self.__class__.__name__

    def __repr__(self):
        retval = ''
        for attr in sorted(self.__dict__.keys()):
            value = self.__dict__[attr]
            retval += (str(attr) + ': ' + '\n'
                       '    ' + repr(value) + '\n')
        return (self.__class__.__name__ + ':' + '\n' +
                '\n'.join(['  ' + line for line in retval.split('\n')]))


class Cfg_Image:

    def __init__(self):
        self.image_type = ''
        self.sign_id = ''
        self.cert_config = ''
        self.name = ''
        self.general_properties_overrides = Cfg_General_Properties_Overrides()

    def __str__(self):
        return 'Instance of ' + self.__class__.__name__

    def __repr__(self):
        retval = ''
        for attr in sorted(self.__dict__.keys()):
            value = self.__dict__[attr]
            retval += (str(attr) + ': ' + '\n'
                       '    ' + repr(value) + '\n')
        return (self.__class__.__name__ + ':' + '\n' +
                '\n'.join(['  ' + line for line in retval.split('\n')]))


class Cfg_General_Properties_Overrides:

    def __init__(self):
        self.model_id = ''
        self.in_use_soc_hw_version = 0
        self.soc_hw_version = ''
        self.msm_part = ''
        self.debug = ''
        self.oem_id = ''
        self.sw_id = ''

    def __str__(self):
        return 'Instance of ' + self.__class__.__name__

    def __repr__(self):
        retval = ''
        for attr in sorted(self.__dict__.keys()):
            value = self.__dict__[attr]
            retval += (str(attr) + ': ' + '\n'
                       '    ' + repr(value) + '\n')
        return (self.__class__.__name__ + ':' + '\n' +
                '\n'.join(['  ' + line for line in retval.split('\n')]))


class Cfg_Data_Provisioning:

    def __init__(self):
        self.base_path = ''

    def __str__(self):
        return 'Instance of ' + self.__class__.__name__

    def __repr__(self):
        retval = ''
        for attr in sorted(self.__dict__.keys()):
            value = self.__dict__[attr]
            retval += (str(attr) + ': ' + '\n'
                       '    ' + repr(value) + '\n')
        return (self.__class__.__name__ + ':' + '\n' +
                '\n'.join(['  ' + line for line in retval.split('\n')]))


