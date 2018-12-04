/*
 * Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 */ 
// Copyright (c) 2018 Qualcomm Technologies, Inc.
// All rights reserved.
// Redistribution and use in source and binary forms, with or without modification, are permitted (subject to the limitations in the disclaimer below) 
// provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, 
// this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// Neither the name of Qualcomm Technologies, Inc. nor the names of its contributors may be used to endorse or promote products derived 
// from this software without specific prior written permission.
// NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE. 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, 
// BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, 
// OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, 
// EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

###########################################################
##### Root of Trust Management Tool #######################
###########################################################

Purpose: QCA402X modules can be provisioned with four root certificates at a given time. The purpose of this tool is to revoke or activate specific root(s) using KDF functionality.

Dependencies: 
1. The tool works in conjunction with KDF Password Generator tool provided in the SDK.
2. Secure boot must be enabled.
3. The chip should be configured for Multiple-root-of-trust (via OTP fuses). 

Usage:  To use this tool, following steps should be followed-

1. Generate KDF password using the password generator. Edit the input xml file based on your requirements (e.g. activation/revocation vector values, Op-Code- 7 (Activation), 8 (Revocation)). A different password is generated for activation and revocation operations.

2. Edit the rot_params.h file under src/app. Plug in the newly generated passwords and user input values. The "activate_input[]" and "revoc_input[]" values must match the user input used in the xml input file to the password generator.

3. Build the ROT tool. Navigate to ROT/build/gcc and run build.bat. Make sure the image is signed with a root that is currently active. This is specified by the "mrc_index" field in 4020_secimage.xml. A value of 0 indicates 1st root is used to sign the image, a value of 3 means 4th root is used to sign the image.

4. The signed elf file (ROT.elf) will be generated in build/gcc/4020/m4 directory. This file can now be:
	a. Flashed via USB or JTAG
	b. Added to firmware upgrade image bundle and downloaded over the air.
	
5. If the image is added to Firmware Upgrade bundle, then it must be assigned an ID of 10, and should be placed before the M4 ELF image. This will allow the boot loader to load the ROT image before all other images. 

6. If the image succeeds in the ROT operation, it will "invalidate" itself and trigger a reset, which will then force the boot loader to boot the original M4 application image. If the image fails (e.g. due to misconfigured parameters), the entire trial image set will be invalidated, and boot loader will load the current image set.

Please refer to QCA402X Security guide for more details.