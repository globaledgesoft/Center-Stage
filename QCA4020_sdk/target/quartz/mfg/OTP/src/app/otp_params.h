/*
 * Copyright (c) 2017 Qualcomm Technologies, Inc.
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

#ifndef _OTP_PARAM_H
#define _OTP_PARAM_H

#define OTP_STATUS_SUCCESS               0
#define OTP_STATUS_ERR_OTP_OP_FAIL       1
#define OTP_STATUS_ERR_ENC_KEY_LEN       2
#define OTP_STATUS_ERR_PK_HASH_LEN       3
#define OTP_STATUS_ERR_INVLID_PROFILE    4
#define OTP_STATUS_ERR_LAST              5

/*********************************************************************/

/* This API will write KDF related setting to OTP */
uint32_t OTP_set_kdf(uint32_t kdf_enable, uint8_t *enc_key, uint32_t enc_key_len);

/* This API will write secure boot related setting to OTP */
uint32_t OTP_set_secure_boot(uint32_t secure_boot_enable, uint8_t *pk_hash, uint32_t  pk_hask_len);

/* This API will write firmware region write setting to OTP */
uint32_t OTP_set_firmware_region_write_disable(uint32_t disable);

/* This API will write model id to OTP */
uint32_t OTP_set_model_id(uint32_t model_id);

/* This API will write otp profile related setting to OTP */
uint32_t OTP_set_profile(uint32_t otp_profile);

/*********************************************************************/
#endif

