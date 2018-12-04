/*
 * Copyright (c) 2016 Qualcomm Atheros, Inc.
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

#include "stdint.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "qcli_api.h"
#include "qapi_status.h"
#include "qapi_crypto.h"
#include "qapi_ed25519.h"
#include "crypto_demo.h"
#include "crypto_helper.h"
#include "persistent_obj_demo.h"
#include "qapi_attestation.h"
#include "qapi_ns_utils.h"

#define CRYPTO_DBG
#ifdef CRYPTO_DBG
#define CRYPTO_PRINTF(...)     QCLI_Printf(qcli_crypto_handle, __VA_ARGS__)
#else
#define CRYPTO_PRINTF(x, ...)
#endif

QCLI_Group_Handle_t qcli_crypto_handle; /* Handle for Crypto Command Group. */

#if defined (CONFIG_CRYPTO_UNIT_TEST_DEMO) || defined (CONFIG_CRYPTO_PERSISTENT_OBJECT_DEMO)
const uint8_t test_adata[20] =
    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
     0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
     0x10, 0x11, 0x12, 0x13};

const uint8_t test_generic_secret_key[64] =
    {0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
     0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
     0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
     0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
     0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
     0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
     0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
     0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f};

//since the limitation of lit_reg, all the test data use this one.
const uint8_t test_iv[32] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,  0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,  0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};

const uint8_t rsa_mod[] = {0xa7,0x34,0x23,0xef,0xe9,0xcf,0xc5,0x25,0xed,0x78,0xe6,0x98,0xe9,0x63,0x37,0xc3,0xf5,0x1c,0xaf,0x39,0xb3,0xb1,0xb8,0x81,0x98,0x4f,0xcf,0x2f,0x95,0x2f,0x45,0x77,0x31,0xd5,0x4b,0x8e,0x86,0xc7,0x24,0x87,0xe8,0x46,0x18,0xa4,0xca,0x87,0x36,0x0b,0xaa,0x48,0x0b,0xd8,0xbd,0x31,0x2e,0xaf,0xd0,0xa0,0x0e,0xd6,0xe7,0x46,0x08,0x3a,0x03,0x90,0xdb,0x87,0x1c,0xcb,0x12,0x53,0x4a,0xaf,0x9e,0x9b,0x98,0xb0,0x66,0x78,0x43,0x61,0x37,0x23,0x9b,0x20,0x12,0xa3,0x55,0x63,0x47,0xe5,0x88,0xd8,0xf1,0x9c,0x1f,0x06,0x7b,0x9b,0xf3,0xfb,0xdb,0x76,0x86,0x97,0x67,0x11,0x9d,0x1f,0x02,0xda,0x15,0x2b,0x49,0x49,0x97,0xe6,0x92,0xbe,0x74,0xbb,0x8d,0x50,0xaa,0xed,0x1d,0x33};

const uint8_t rsa_pub_exp[] = {0x01, 0x00, 0x01};

#endif

#ifdef CONFIG_CRYPTO_UNIT_TEST_DEMO

#if !defined(MIN)
    #define MIN(a, b) ( (a < b) ? a : b )
#endif


#define  CRYPTO_CONFIG_3DES 0
#define ENABLE_DH_DEMO 1
#define ENABLE_ED25519_DEMO 1
#define SRP_MOD_SIZE_1024 1

#define crypto_sign_ed25519_BYTES 64U
#define crypto_sign_ed25519_SEEDBYTES 32U
#define crypto_sign_ed25519_PUBLICKEYBYTES 32U
#define crypto_sign_ed25519_SECRETKEYBYTES (32U + 32U)

#define QAPI_CRYPTO_AE_AES_CCM_NONCEBYTES    12U
#define QAPI_CRYPTO_AE_AES_CCM_AADATA_MAXBYTES    14U
#define QAPI_CRYPTO_AE_AES_GCM_NONCEBYTES    12U

#if ENABLE_ED25519_DEMO
typedef struct TestData_ {
    unsigned char  sk[crypto_sign_ed25519_SEEDBYTES];
    unsigned char  pk[crypto_sign_ed25519_PUBLICKEYBYTES];
    unsigned char  sig[crypto_sign_ed25519_BYTES];
    char          *m;
} TestData;

static const TestData test_data[] = {
{{0x9d,0x61,0xb1,0x9d,0xef,0xfd,0x5a,0x60,0xba,0x84,0x4a,0xf4,0x92,0xec,0x2c,0xc4,0x44,0x49,0xc5,0x69,0x7b,0x32,0x69,0x19,0x70,0x3b,0xac,0x03,0x1c,0xae,0x7f,0x60,},{0xd7,0x5a,0x98,0x01,0x82,0xb1,0x0a,0xb7,0xd5,0x4b,0xfe,0xd3,0xc9,0x64,0x07,0x3a,0x0e,0xe1,0x72,0xf3,0xda,0xa6,0x23,0x25,0xaf,0x02,0x1a,0x68,0xf7,0x07,0x51,0x1a,},{0xe5,0x56,0x43,0x00,0xc3,0x60,0xac,0x72,0x90,0x86,0xe2,0xcc,0x80,0x6e,0x82,0x8a,0x84,0x87,0x7f,0x1e,0xb8,0xe5,0xd9,0x74,0xd8,0x73,0xe0,0x65,0x22,0x49,0x01,0x55,0x5f,0xb8,0x82,0x15,0x90,0xa3,0x3b,0xac,0xc6,0x1e,0x39,0x70,0x1c,0xf9,0xb4,0x6b,0xd2,0x5b,0xf5,0xf0,0x59,0x5b,0xbe,0x24,0x65,0x51,0x41,0x43,0x8e,0x7a,0x10,0x0b,},""},
{{0x4c,0xcd,0x08,0x9b,0x28,0xff,0x96,0xda,0x9d,0xb6,0xc3,0x46,0xec,0x11,0x4e,0x0f,0x5b,0x8a,0x31,0x9f,0x35,0xab,0xa6,0x24,0xda,0x8c,0xf6,0xed,0x4f,0xb8,0xa6,0xfb,},{0x3d,0x40,0x17,0xc3,0xe8,0x43,0x89,0x5a,0x92,0xb7,0x0a,0xa7,0x4d,0x1b,0x7e,0xbc,0x9c,0x98,0x2c,0xcf,0x2e,0xc4,0x96,0x8c,0xc0,0xcd,0x55,0xf1,0x2a,0xf4,0x66,0x0c,},{0x92,0xa0,0x09,0xa9,0xf0,0xd4,0xca,0xb8,0x72,0x0e,0x82,0x0b,0x5f,0x64,0x25,0x40,0xa2,0xb2,0x7b,0x54,0x16,0x50,0x3f,0x8f,0xb3,0x76,0x22,0x23,0xeb,0xdb,0x69,0xda,0x08,0x5a,0xc1,0xe4,0x3e,0x15,0x99,0x6e,0x45,0x8f,0x36,0x13,0xd0,0xf1,0x1d,0x8c,0x38,0x7b,0x2e,0xae,0xb4,0x30,0x2a,0xee,0xb0,0x0d,0x29,0x16,0x12,0xbb,0x0c,0x00,},"\x72"},
{{0xc5,0xaa,0x8d,0xf4,0x3f,0x9f,0x83,0x7b,0xed,0xb7,0x44,0x2f,0x31,0xdc,0xb7,0xb1,0x66,0xd3,0x85,0x35,0x07,0x6f,0x09,0x4b,0x85,0xce,0x3a,0x2e,0x0b,0x44,0x58,0xf7,},{0xfc,0x51,0xcd,0x8e,0x62,0x18,0xa1,0xa3,0x8d,0xa4,0x7e,0xd0,0x02,0x30,0xf0,0x58,0x08,0x16,0xed,0x13,0xba,0x33,0x03,0xac,0x5d,0xeb,0x91,0x15,0x48,0x90,0x80,0x25,},{0x62,0x91,0xd6,0x57,0xde,0xec,0x24,0x02,0x48,0x27,0xe6,0x9c,0x3a,0xbe,0x01,0xa3,0x0c,0xe5,0x48,0xa2,0x84,0x74,0x3a,0x44,0x5e,0x36,0x80,0xd7,0xdb,0x5a,0xc3,0xac,0x18,0xff,0x9b,0x53,0x8d,0x16,0xf2,0x90,0xae,0x67,0xf7,0x60,0x98,0x4d,0xc6,0x59,0x4a,0x7c,0x15,0xe9,0x71,0x6e,0xd2,0x8d,0xc0,0x27,0xbe,0xce,0xea,0x1e,0xc4,0x0a,},"\xaf\x82"},
{{0x0d,0x4a,0x05,0xb0,0x73,0x52,0xa5,0x43,0x6e,0x18,0x03,0x56,0xda,0x0a,0xe6,0xef,0xa0,0x34,0x5f,0xf7,0xfb,0x15,0x72,0x57,0x57,0x72,0xe8,0x00,0x5e,0xd9,0x78,0xe9,},{0xe6,0x1a,0x18,0x5b,0xce,0xf2,0x61,0x3a,0x6c,0x7c,0xb7,0x97,0x63,0xce,0x94,0x5d,0x3b,0x24,0x5d,0x76,0x11,0x4d,0xd4,0x40,0xbc,0xf5,0xf2,0xdc,0x1a,0xa5,0x70,0x57,},{0xd9,0x86,0x8d,0x52,0xc2,0xbe,0xbc,0xe5,0xf3,0xfa,0x5a,0x79,0x89,0x19,0x70,0xf3,0x09,0xcb,0x65,0x91,0xe3,0xe1,0x70,0x2a,0x70,0x27,0x6f,0xa9,0x7c,0x24,0xb3,0xa8,0xe5,0x86,0x06,0xc3,0x8c,0x97,0x58,0x52,0x9d,0xa5,0x0e,0xe3,0x1b,0x82,0x19,0xcb,0xa4,0x52,0x71,0xc6,0x89,0xaf,0xa6,0x0b,0x0e,0xa2,0x6c,0x99,0xdb,0x19,0xb0,0x0c,},"\xcb\xc7\x7b"},
    };
#endif

const uint8_t test_aes_128_key[16] = {0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f};

#endif

#if defined (CONFIG_CRYPTO_UNIT_TEST_DEMO) || defined (CONFIG_CRYPTO_PERSISTENT_OBJECT_DEMO)
void ref_attr_init(qapi_Crypto_Attrib_t* attr, uint32_t id,
        const void* buffer, uint32_t length)
{
    attr->attrib_id = id;
    attr->u.ref.len = length;
    attr->u.ref.buf = (void*) buffer;
}
#endif

#ifdef CONFIG_CRYPTO_UNIT_TEST_DEMO
void val_attr_init(qapi_Crypto_Attrib_t* attr, uint32_t id,
        uint32_t a, uint32_t b)
{
    attr->attrib_id = id;
    attr->u.val.a = a;
    attr->u.val.b = b;
}

#if ENABLE_DH_DEMO
/* DH Params */
#define PRIME_BYTES (128)
#define PRIME_BITS (PRIME_BYTES * 8)
static const uint8_t p_1024[PRIME_BYTES] =
{
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xC9, 0x0F, 0xDA, 0xA2, 0x21, 0x68, 0xC2, 0x34,
    0xC4, 0xC6, 0x62, 0x8B, 0x80, 0xDC, 0x1C, 0xD1,
    0x29, 0x02, 0x4E, 0x08, 0x8A, 0x67, 0xCC, 0x74,
    0x02, 0x0B, 0xBE, 0xA6, 0x3B, 0x13, 0x9B, 0x22,
    0x51, 0x4A, 0x08, 0x79, 0x8E, 0x34, 0x04, 0xDD,
    0xEF, 0x95, 0x19, 0xB3, 0xCD, 0x3A, 0x43, 0x1B,
    0x30, 0x2B, 0x0A, 0x6D, 0xF2, 0x5F, 0x14, 0x37,
    0x4F, 0xE1, 0x35, 0x6D, 0x6D, 0x51, 0xC2, 0x45,
    0xE4, 0x85, 0xB5, 0x76, 0x62, 0x5E, 0x7E, 0xC6,
    0xF4, 0x4C, 0x42, 0xE9, 0xA6, 0x37, 0xED, 0x6B,
    0x0B, 0xFF, 0x5C, 0xB6, 0xF4, 0x06, 0xB7, 0xED,
    0xEE, 0x38, 0x6B, 0xFB, 0x5A, 0x89, 0x9F, 0xA5,
    0xAE, 0x9F, 0x24, 0x11, 0x7C, 0x4B, 0x1F, 0xE6,
    0x49, 0x28, 0x66, 0x51, 0xEC, 0xE6, 0x53, 0x81,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

static const uint8_t g[4] = {0, 0, 0, 2};
/* End of DH Params */
#endif

#define ENABLE_RNG_DEMO
#ifdef ENABLE_RNG_DEMO
/* RNG Params */
#define crypto_rng_RANDOMBYTES 256
/* End of RNG Params */
#endif

const uint8_t rsa_pvt_exp[] = {0xa2,0x2e,0xc8,0x21,0x01,0x49,0x40,0x70,0xde,0x9c,0x64,0x08,0xb4,0x71,0x41,0xf9,0x38,0x7b,0x1f,0x58,0x37,0xfd,0xcd,0xfb,0x1e,0x93,0x63,0x63,0x43,0x30,0xbc,0x0f,0xb6,0xed,0xae,0xc1,0x4b,0xe8,0x44,0x7d,0xf3,0x86,0x72,0x86,0xfd,0xd0,0x13,0x53,0x53,0x53,0x58,0xf2,0x5d,0xe9,0x23,0xe0,0xf3,0xdb,0x1d,0x29,0xc4,0xe1,0x40,0x3d,0xfc,0x15,0x98,0x0d,0x14,0x7f,0xcc,0xb3,0x4d,0xb9,0xeb,0xf5,0xf1,0xc7,0x9f,0xcd,0x9f,0xae,0x3c,0x8c,0x3b,0xc5,0x06,0x0a,0x59,0x11,0xac,0x44,0x9b,0xad,0xb9,0x1e,0x45,0x52,0xca,0x63,0xc2,0xea,0x67,0x0d,0x92,0x5c,0xa1,0x09,0x9a,0x9b,0xf3,0x79,0xf5,0x8b,0xcd,0x0d,0x8e,0x4f,0x9b,0x44,0xc1,0x36,0xda,0x01,0xd9,0x94,0xc3,0x61};

const uint8_t rsa_prime1[] = {0xd4,0x03,0x3d,0x5d,0xa8,0xb4,0x42,0xb8,0x20,0x6a,0xb1,0x79,0x10,0xfc,0xfa,0xff,0xa6,0x5e,0xa3,0xa9,0xcd,0x7b,0x61,0x54,0x16,0x7d,0xdf,0xfb,0x90,0x1a,0xc6,0x6b,0x29,0x24,0x08,0xca,0x84,0xa6,0xc2,0xcf,0xf5,0xac,0xa8,0xdc,0xc9,0x10,0x7f,0xb7,0x84,0x12,0x2d,0xd7,0x90,0x1f,0xdb,0x27,0x01,0xfa,0x9f,0xed,0xf2,0xd8,0xcf,0xb1};

const uint8_t rsa_prime2[] = {0xc9,0xe4,0xee,0xec,0x50,0xc7,0xa1,0x00,0xfd,0x37,0x89,0x92,0xab,0x99,0xb0,0x2d,0x45,0xa2,0x5a,0xec,0xd2,0x12,0x76,0xf1,0x68,0xc9,0x40,0x3e,0xe5,0x51,0x97,0x1f,0x43,0x77,0x7f,0x56,0x11,0x1e,0x05,0x82,0x85,0x8d,0x37,0x2e,0x1b,0xc7,0x07,0xd0,0x19,0x6d,0xdc,0x0c,0x8f,0xc4,0x74,0xa2,0x9f,0x1f,0x3a,0xbe,0x67,0xfa,0x38,0x23};

const uint8_t rsa_exp1[] = {0xc8,0x23,0x0e,0xc8,0xdd,0x3a,0xdd,0x48,0xc7,0x81,0x30,0x6b,0xa2,0xf7,0xcd,0x51,0x8c,0x12,0x06,0xd9,0x82,0x5a,0x18,0x34,0xb2,0xce,0xbc,0xa3,0xd3,0x13,0x13,0x7f,0x91,0x64,0xac,0xcf,0xd0,0x8d,0x43,0x95,0xe0,0xca,0xce,0xd5,0x2d,0x10,0xe0,0x1f,0xb3,0x13,0x1e,0x27,0x41,0xac,0x70,0xca,0xcf,0xf9,0x71,0x03,0xc4,0x9d,0x9a,0xe1};

const uint8_t rsa_exp2[] = {0x0e,0x5a,0x43,0x0d,0xf3,0xb4,0x2d,0x62,0xf7,0x9d,0x62,0x1f,0x56,0x29,0xa7,0xd7,0xa0,0x12,0xa9,0xaa,0x1a,0x49,0x0b,0xc1,0x9f,0xb4,0x66,0xe7,0xd1,0xbf,0x9a,0x21,0xb3,0xd7,0x23,0xeb,0x47,0x6e,0x3d,0xf0,0x08,0x74,0x80,0x8e,0xbb,0x94,0xcb,0x9e,0x64,0xa0,0x65,0xbb,0x52,0xe1,0x21,0x75,0x8a,0x20,0x5b,0x39,0xbc,0x04,0x92,0xc7};

const uint8_t rsa_coeff[] = {0xb0,0xe0,0x2c,0x43,0x47,0x11,0x38,0x5e,0xb9,0x26,0x74,0x39,0x73,0x96,0xc4,0x87,0x10,0xe4,0xb8,0x5c,0x2d,0x32,0x17,0x9b,0x5f,0x88,0xf7,0xf9,0xbd,0x09,0xac,0x48,0xf2,0x0e,0x96,0x8a,0xbc,0x97,0xfc,0xc7,0xa3,0x51,0x6f,0x84,0xfb,0x8f,0x0c,0x14,0x6a,0x26,0xec,0x08,0xe9,0xb2,0x43,0xde,0x23,0xc4,0x89,0x13,0x19,0x0a,0x8c,0xc1};

#ifdef SRP_MOD_SIZE_1024
/* Modulus size 1024 bit */
#define MOD_SIZE (1024/8)
static const uint32_t srp_mod_tmp[] = {
0xEEAF0AB9 , 0xADB38DD6 ,0x9C33F80A ,0xFA8FC5E8 ,0x60726187 ,0x75FF3C0B ,0x9EA2314C,
0x9C256576 , 0xD674DF74 ,0x96EA81D3 ,0x383B4813 ,0xD692C6E0 ,0xE0D5D8E2 ,0x50B98BE4,
0x8E495C1D , 0x6089DAD1 ,0x5DC7D7B4 ,0x6154D6B6 ,0xCE8EF4AD ,0x69B15D49 ,0x82559B29,
0x7BCF1885 , 0xC529F566 ,0x660E57EC ,0x68EDBC3C ,0x05726CC0 ,0x2FD4CBF4 ,0x976EAA9A,
0xFD5138FE , 0x8376435B ,0x9FC61D2F ,0xC0EB06E3
};

static const uint8_t srp_gen_sample[] = {2};

static const uint32_t srp_ver_tmp[] = {
0x7E273DE8 ,0x696FFC4F , 0x4E337D05, 0xB4B375BE , 0xB0DDE156 , 0x9E8FA00A , 0x9886D812,
0x9BADA1F1 ,0x822223CA , 0x1A605B53, 0x0E379BA4 , 0x729FDC59 , 0xF105B478 , 0x7E5186F5,
0xC671085A ,0x1447B52A , 0x48CF1970, 0xB4FB6F84 , 0x00BBF4CE , 0xBFBB1681 , 0x52E08AB5,
0xEA53D15C ,0x1AFF87B2 , 0xB9DA6E04, 0xE058AD51 , 0xCC72BFC9 , 0x033B564E , 0x26480D78,
0xE955A5E2 ,0x9E7AB245 , 0xDB2BE315, 0xE2099AFB
};

#endif

#ifdef SRP_MOD_SIZE_3072
/* Modulus size 3072 bit */
#define MOD_SIZE (3072/8)
#define ALG_TYPE (QAPI_CRYPTO_ALG_SHA512_E)

static const uint32_t srp_mod_tmp[] = {
0xFFFFFFFF ,0xFFFFFFFF ,0xC90FDAA2 ,0x2168C234 ,0xC4C6628B ,0x80DC1CD1 ,0x29024E08,
0x8A67CC74 ,0x020BBEA6 ,0x3B139B22 ,0x514A0879 ,0x8E3404DD ,0xEF9519B3 ,0xCD3A431B,
0x302B0A6D ,0xF25F1437 ,0x4FE1356D ,0x6D51C245 ,0xE485B576 ,0x625E7EC6 ,0xF44C42E9,
0xA637ED6B ,0x0BFF5CB6 ,0xF406B7ED ,0xEE386BFB ,0x5A899FA5 ,0xAE9F2411 ,0x7C4B1FE6,
0x49286651 ,0xECE45B3D ,0xC2007CB8 ,0xA163BF05 ,0x98DA4836 ,0x1C55D39A ,0x69163FA8,
0xFD24CF5F ,0x83655D23 ,0xDCA3AD96 ,0x1C62F356 ,0x208552BB ,0x9ED52907 ,0x7096966D,
0x670C354E ,0x4ABC9804 ,0xF1746C08 ,0xCA18217C ,0x32905E46 ,0x2E36CE3B ,0xE39E772C,
0x180E8603 ,0x9B2783A2 ,0xEC07A28F ,0xB5C55DF0 ,0x6F4C52C9 ,0xDE2BCBF6 ,0x95581718,
0x3995497C ,0xEA956AE5 ,0x15D22618 ,0x98FA0510 ,0x15728E5A ,0x8AAAC42D ,0xAD33170D,
0x04507A33 ,0xA85521AB ,0xDF1CBA64 ,0xECFB8504 ,0x58DBEF0A ,0x8AEA7157 ,0x5D060C7D,
0xB3970F85 ,0xA6E1E4C7 ,0xABF5AE8C ,0xDB0933D7 ,0x1E8C94E0 ,0x4A25619D ,0xCEE3D226,
0x1AD2EE6B ,0xF12FFA06 ,0xD98A0864 ,0xD8760273 ,0x3EC86A64 ,0x521F2B18 ,0x177B200C,
0xBBE11757 ,0x7A615D6C ,0x770988C0 ,0xBAD946E2 ,0x08E24FA0 ,0x74E5AB31 ,0x43DB5BFC,
0xE0FD108E ,0x4B82D120 ,0xA93AD2CA ,0xFFFFFFFF ,0xFFFFFFFF};
uint8_t srp_mod_sample[MOD_SIZE]; /* Hold SRP modulus as an array of srp_mod_tmp in big endian */

static const uint8_t srp_gen_sample[] = {5};
static const uint32_t srp_ver_tmp[] =
   {0x9b5e0617, 0x01ea7aeb, 0X39CF6E35, 0X19655A85, 0X3CF94C75, 0XCAF2555E, 0XF1FAF759, 0XBB79CB47,
    0X7014E04A, 0X88D68FFC, 0X05323891, 0xD4C205B8, 0XDE81C2F2, 0X03D8FAD1, 0XB24D2C10, 0X9737F1BE,
    0xBBD71F91, 0x2447C4A0, 0x3C26B9FA, 0xD8EDB3E7, 0x80778e30, 0x2529ed1e, 0xe138ccfc, 0x36d4ba31,
    0x3CC48B14, 0xEA8C22A0, 0x186B222E, 0x655f2df5, 0x603fd75d, 0xf76b3b08, 0xff895006, 0x9add03a7,
    0x54EE4AE8, 0x8587CCE1, 0xBFDE3679, 0x4DBAE459, 0x2b7b904f, 0x442b041c, 0xb17aebad, 0x1e3aebe3, 
    0xCBE99DE6, 0x5F4BB1FA, 0x00B0E7AF, 0x06863DB5, 0x3B02254E, 0xC66E781e, 0x3b62a821, 0x2c86beb0,
    0xD50B5BA6, 0xD0B478D8, 0xC4E9BBCE, 0xC2176532, 0x6FBD1405, 0x8D2BBDE2, 0xC33045F0, 0x3873E539,
    0x48D78B79, 0x4F0790E4, 0x8C36AED6, 0xE880F557, 0x427B2FC0, 0x6DB5E1E2, 0xE1D7E661, 0xAC482D18,
    0xE528D729, 0x5EF74372, 0x95FF1A72, 0xD4027717, 0x13F16876, 0xDD050AE5, 0xB7AD53cc, 0xb90855c9, 
    0x39566483, 0x58ADFD96, 0x6422F524, 0x98732D68, 0xD1D7FBEF, 0x10D78034, 0xAB8DCB6F, 0x0FCF885C, 
    0xC2B2EA2C, 0x3E6AC866, 0x09EA058A, 0x9DA8CC63, 0x531DC915, 0x414df568, 0xb09482dd, 0xac1954de,
    0xC7EB714F, 0x6FF7D44C, 0xD5B86F6B, 0xD1158109, 0x30637C01, 0xD0F6013B, 0xC9740FA2, 0xC633BA89};
uint8_t srp_ver_sample[MOD_SIZE];
#endif

uint8_t srp_cli_pub_key[MOD_SIZE];
uint8_t srp_srv_pub_key[MOD_SIZE];
uint8_t srp_srv_shared_secret[MOD_SIZE];
uint8_t srp_cli_shared_secret[MOD_SIZE];
static const uint32_t salt_tmp[] = {0xBEB25379, 0xD1A8581E, 0xB5A72767, 0x3A2441EE};
#define SALT_LEN 16
uint8_t salt[SALT_LEN];
uint8_t uname[] = "alice";
uint8_t pwd[] = "password123";
uint8_t srp_mod_sample[MOD_SIZE]; /* Holds big endian array */
uint8_t srp_ver_sample[MOD_SIZE]; /* Holds big endian array */

#if 0
/* To check for test vectors given in RFC 5054, remove RNG_getRNG from srp.c and replace with pvt keys below */
static const uint8_t client_pvt_key_sample[] = {0x60, 0x97, 0x55, 0x27, 0x03, 0x5c, 0xf2, 0xad, 0x19, 0x89, 0x80, 0x6f, 0x04, 0x07, 0x21, 0x0b, 0xc8, 0x1e, 0xdc, 0x04, 0xe2, 0x76, 0x2a, 0x56, 0xaf, 0xd5, 0x29, 0xdd, 0xda, 0x2d, 0x43, 0x93};
static const uint8_t server_pvt_key_sample[] = {0xE4, 0x87, 0xCB, 0x59, 0xD3, 0x1A, 0xC5, 0x50, 0x47, 0x1E, 0x81, 0xF0, 0x0F, 0x69, 0x28, 0xE0, 0x1D, 0xDA, 0x08, 0xE9, 0x74, 0xA0, 0x04, 0xF4, 0x9E, 0x61, 0xF5, 0xD1, 0x05, 0x28, 0x4D, 0x20};
#endif

static const unsigned char ecjpake_test_password[] = {
    0x74, 0x68, 0x72, 0x65, 0x61, 0x64, 0x6a, 0x70, 0x61, 0x6b, 0x65, 0x74, 0x65, 0x73, 0x74
};
static const unsigned char ecjpake_test_wrong_password[] = {
    0x75, 0x68, 0x72, 0x65, 0x61, 0x64, 0x6a, 0x70, 0x61, 0x6b, 0x65, 0x74, 0x65, 0x73, 0x74
};
const uint8_t ecjpake_test_client_id[6] = {0x63, 0x6c, 0x69, 0x65, 0x6e, 0x74}; // "client"
const uint8_t ecjpake_test_server_id[6] = {0x73, 0x65, 0x72, 0x76, 0x65, 0x72}; // "server"

void memmove_endianess(uint8_t *d , const uint8_t *s , uint16_t len );

int certcs_download_file(
    char * hostname,
    int port,
    char * filename,
    uint8_t ** pp_buffer,
    uint32_t * p_buffer_size
    );

int crypto_demo_is_buffer_all_zeros(uint8_t *buffer, uint32_t buffer_size) {
    int is_all_zeros = 1;
    int byte_index = 0;
    for(byte_index = 0; byte_index < buffer_size; byte_index++) {
    	if(buffer[byte_index] != 0) {
    		is_all_zeros = 0;
    		break;
    	}
    }
    return is_all_zeros;
}

int32_t srp_demo()
{
    qapi_Crypto_Obj_Hdl_t cli_hdl, srv_hdl, srv_sec_hdl, cli_sec_hdl;
    qapi_Crypto_Op_Hdl_t srv_op_hdl, cli_op_hdl;
    qapi_Crypto_Attrib_t attr[5];
    uint32_t srp_key_size = MOD_SIZE * 8;

    if (qapi_Crypto_Transient_Obj_Alloc(QAPI_CRYPTO_OBJ_TYPE_SRP_KEYPAIR_E, srp_key_size, 
                &cli_hdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc transient obj\n");
        return -1;
    }

    if (qapi_Crypto_Transient_Obj_Alloc(QAPI_CRYPTO_OBJ_TYPE_SRP_KEYPAIR_E, srp_key_size, 
                &srv_hdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc transient obj\n");
        return -1;
    }

    /* Input need to be array of bin endian integers */
    memmove_endianess(srp_mod_sample, (uint8_t *)srp_mod_tmp, MOD_SIZE);
    memmove_endianess(srp_ver_sample, (uint8_t *)srp_ver_tmp, MOD_SIZE);
    memmove_endianess(salt, (uint8_t *)salt_tmp, SALT_LEN);

    ref_attr_init(&attr[0], QAPI_CRYPTO_ATTR_SRP_PRIME_E, srp_mod_sample, MOD_SIZE);
    ref_attr_init(&attr[1], QAPI_CRYPTO_ATTR_SRP_GEN_E, srp_gen_sample, 1);
    val_attr_init(&attr[2], QAPI_CRYPTO_ATTR_SRP_TYPE_E, QAPI_CRYPTO_SRP_CLIENT_E, 0);
    if (qapi_Crypto_Transient_Obj_Key_Gen(cli_hdl, srp_key_size, attr, QAPI_CRYPTO_OBJ_ATTRIB_COUNT_SRP_CLIENT) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to gen client key\n");
        return -1;
    }

    if (qapi_Crypto_Obj_Buf_Attrib_Get(cli_hdl, QAPI_CRYPTO_ATTR_SRP_PUBLIC_VALUE_E, srp_cli_pub_key, MOD_SIZE) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to get client public val\n");
        return -1;
    }

    ref_attr_init(&attr[0], QAPI_CRYPTO_ATTR_SRP_PRIME_E, srp_mod_sample, MOD_SIZE);
    ref_attr_init(&attr[1], QAPI_CRYPTO_ATTR_SRP_GEN_E, srp_gen_sample, 1);
    ref_attr_init(&attr[2], QAPI_CRYPTO_ATTR_SRP_VERIFIER_E, srp_ver_sample, MOD_SIZE);
    val_attr_init(&attr[3], QAPI_CRYPTO_ATTR_SRP_TYPE_E, QAPI_CRYPTO_SRP_SERVER_E, 0);
    val_attr_init(&attr[4], QAPI_CRYPTO_ATTR_SRP_HASH_E, QAPI_CRYPTO_ALG_SHA1_E, 0);
    if (qapi_Crypto_Transient_Obj_Key_Gen(srv_hdl, srp_key_size, attr, QAPI_CRYPTO_OBJ_ATTRIB_COUNT_SRP_SERVER + 1) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to gen server key\n");
        return -1;
    }

    if (qapi_Crypto_Obj_Buf_Attrib_Get(srv_hdl, QAPI_CRYPTO_ATTR_SRP_PUBLIC_VALUE_E, srp_srv_pub_key, MOD_SIZE) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to get server public val\n");
        return -1;
    }

    /* Derive server shared secret */
    if (qapi_Crypto_Transient_Obj_Alloc(QAPI_CRYPTO_OBJ_TYPE_GENERIC_SECRET_E, MOD_SIZE * 8, &srv_sec_hdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc gen secret\n");
        return -1;
    }

    if (qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_SRP_DERIVE_SHARED_SECRET_E, QAPI_CRYPTO_MODE_DERIVE_E, MOD_SIZE*8, &srv_op_hdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc op\n");
        return -1;
    }

    if (qapi_Crypto_Op_Key_Set(srv_op_hdl, srv_hdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to set pvt key\n");
        return -1;
    }

    ref_attr_init(&attr[0], QAPI_CRYPTO_ATTR_SRP_PUBLIC_VALUE_E, srp_cli_pub_key, MOD_SIZE);
    if (qapi_Crypto_Op_Key_Derive(srv_op_hdl, &attr[0], 1, srv_sec_hdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to derive key 1\n");
        return -1;
    }

    if (qapi_Crypto_Obj_Buf_Attrib_Get(srv_sec_hdl, QAPI_CRYPTO_ATTR_SECRET_VALUE_E, srp_srv_shared_secret, MOD_SIZE) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to get derived key val 1\n");
        return -1;
    }

    /* Derive client shared secret */
    if (qapi_Crypto_Transient_Obj_Alloc(QAPI_CRYPTO_OBJ_TYPE_GENERIC_SECRET_E, MOD_SIZE * 8, &cli_sec_hdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc gen secret\n");
        return -1;
    }

    if (qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_SRP_DERIVE_SHARED_SECRET_E, QAPI_CRYPTO_MODE_DERIVE_E, MOD_SIZE*8, &cli_op_hdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc op\n");
        return -1;
    }

    if (qapi_Crypto_Op_Key_Set(cli_op_hdl, cli_hdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to set pvt key\n");
        return -1;
    }

    ref_attr_init(&attr[0], QAPI_CRYPTO_ATTR_SRP_PUBLIC_VALUE_E, srp_srv_pub_key, MOD_SIZE);
    /* Note: Do not include NULL termination character in length of username and
     * pwd */
    ref_attr_init(&attr[1], QAPI_CRYPTO_ATTR_SRP_USERNAME_E, uname, sizeof(uname) - 1);
    ref_attr_init(&attr[2], QAPI_CRYPTO_ATTR_SRP_PASSWORD_E, pwd, sizeof(pwd) - 1);
    ref_attr_init(&attr[3], QAPI_CRYPTO_ATTR_SRP_SALT_E, salt, SALT_LEN);
    val_attr_init(&attr[4], QAPI_CRYPTO_ATTR_SRP_HASH_E, QAPI_CRYPTO_ALG_SHA1_E, 0);
    if (qapi_Crypto_Op_Key_Derive(cli_op_hdl, &attr[0], 5, cli_sec_hdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to derive key 2\n");
        return -1;
    }


    if (qapi_Crypto_Obj_Buf_Attrib_Get(cli_sec_hdl, QAPI_CRYPTO_ATTR_SECRET_VALUE_E, srp_cli_shared_secret, MOD_SIZE) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to get derived key val 2\n");
        return -1;
    }

    if (memcmp(srp_cli_shared_secret, srp_srv_shared_secret, MOD_SIZE) != 0) {
        CRYPTO_PRINTF("shared secret failure\n");
        return -1;
    }
    CRYPTO_PRINTF("SRP shared secret match!\n");

    qapi_Crypto_Transient_Obj_Free(cli_hdl); 
    qapi_Crypto_Transient_Obj_Free(srv_hdl); 
    qapi_Crypto_Transient_Obj_Free(cli_sec_hdl); 
    qapi_Crypto_Transient_Obj_Free(srv_sec_hdl); 
    qapi_Crypto_Op_Free(srv_op_hdl); 
    qapi_Crypto_Op_Free(cli_op_hdl); 
    
    return 0;
}

int32_t ecjpake_demo_init(qapi_Crypto_Op_Hdl_t *p_op_hdl)
{
	int status = 0;
    status = qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_ECJPAKE_E, QAPI_CRYPTO_MODE_DERIVE_E,
    		8 * QAPI_CRYPTO_ECJPAKE_PASSWORD_MAX_BYTES,
			p_op_hdl);
    if (status != QAPI_OK) {
        return QAPI_ERROR;
    }

    return QAPI_OK;
}

int32_t ecjpake_demo_generate_round1_output(qapi_Crypto_Op_Hdl_t op_hdl,
		qapi_Crypto_Obj_Hdl_t *p_round1_public_key_and_zkp_pair_hdl,
		const uint8_t *id,
		uint32_t id_size,
		const uint8_t *password,
		uint32_t password_size) {

	int status = QAPI_OK;
	qapi_Crypto_Obj_Hdl_t password_hdl = 0;
	qapi_Crypto_Attrib_t password_attr[QAPI_CRYPTO_OBJ_ATTRIB_COUNT_GENERIC_SECRET];
	qapi_Crypto_Attrib_t generate_round1_output_attr[QAPI_CRYPTO_OP_ATTRIB_COUNT_ECJPAKE_DERIVE_ROUND1_KEYS];

	ref_attr_init(&password_attr[0],
			QAPI_CRYPTO_ATTR_SECRET_VALUE_E,
			password, password_size);

	status = qapi_Crypto_Transient_Obj_Alloc(QAPI_CRYPTO_OBJ_TYPE_GENERIC_SECRET_E,
			8 * QAPI_CRYPTO_ECJPAKE_PASSWORD_MAX_BYTES,
			&password_hdl);
    if (status != QAPI_OK) {
    	goto ecjpake_demo_generate_round1_output_cleanup;
    }

	status = qapi_Crypto_Transient_Obj_Populate(password_hdl,
			&password_attr[0],
			QAPI_CRYPTO_OBJ_ATTRIB_COUNT_GENERIC_SECRET);
    if (status != QAPI_OK) {
    	goto ecjpake_demo_generate_round1_output_cleanup;
    }

    status = qapi_Crypto_Op_Key_Set(op_hdl, password_hdl);
    if (status != QAPI_OK) {
    	goto ecjpake_demo_generate_round1_output_cleanup;
    }

	ref_attr_init(&generate_round1_output_attr[0],
			QAPI_CRYPTO_ATTR_ECJPAKE_LOCAL_IDENTITY_E,
			id, id_size);

    val_attr_init(&generate_round1_output_attr[1],
    		QAPI_CRYPTO_ATTR_ECC_CURVE_E,
			QAPI_CRYPTO_ECC_CURVE_NIST_P256, 0);

	status = qapi_Crypto_Transient_Obj_Alloc(QAPI_CRYPTO_OBJ_TYPE_ECJPAKE_ROUND1_PUBLIC_KEY_AND_ZKP_PAIR_E,
    		QAPI_CRYPTO_ECC_P256_PUBLIC_KEY_BITS,
			p_round1_public_key_and_zkp_pair_hdl);
    if (status != QAPI_OK) {
    	goto ecjpake_demo_generate_round1_output_cleanup;
    }

    status = qapi_Crypto_Op_Intermediate_Key_Derive(op_hdl,
    		&generate_round1_output_attr[0],
			QAPI_CRYPTO_OP_ATTRIB_COUNT_ECJPAKE_DERIVE_ROUND1_KEYS,
        	*p_round1_public_key_and_zkp_pair_hdl);
    if (status != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to generate round1 output\n");
        goto ecjpake_demo_generate_round1_output_cleanup;
    }

ecjpake_demo_generate_round1_output_cleanup:
	qapi_Crypto_Transient_Obj_Free(password_hdl);
    return status;
}

int32_t ecjpake_demo_generate_round2_output(qapi_Crypto_Op_Hdl_t op_hdl,
		qapi_Crypto_Obj_Hdl_t *p_round2_public_key_and_zkp_hdl,
		qapi_Crypto_Obj_Hdl_t remote_round1_public_key_and_zkp_pair_hdl,
		const uint8_t *remote_id,
		uint32_t remote_id_size) {

	int status = QAPI_OK;
    uint8_t *remote_round1_public_key_1 = 0;
    uint8_t *remote_round1_zkp_ephemeral_public_key_1 = 0;
    uint8_t *remote_round1_zkp_signature_1 = 0;
    uint8_t *remote_round1_public_key_2 = 0;
    uint8_t *remote_round1_zkp_ephemeral_public_key_2 = 0;
    uint8_t *remote_round1_zkp_signature_2 = 0;
	qapi_Crypto_Attrib_t remote_round1_attr[QAPI_CRYPTO_OP_ATTRIB_COUNT_ECJPAKE_DERIVE_ROUND2_KEYS];

    remote_round1_public_key_1 = (uint8_t*)malloc(QAPI_CRYPTO_ECC_P256_PUBLIC_KEY_UNCOMPRESSED_FORMAT_BYTES);
    remote_round1_zkp_ephemeral_public_key_1 = (uint8_t*)malloc(QAPI_CRYPTO_ECC_P256_PUBLIC_KEY_UNCOMPRESSED_FORMAT_BYTES);
    remote_round1_zkp_signature_1 = (uint8_t*)malloc(QAPI_CRYPTO_ECC_P256_PUB_VAL_X_BYTES);
    remote_round1_public_key_2 = (uint8_t*)malloc(QAPI_CRYPTO_ECC_P256_PUBLIC_KEY_UNCOMPRESSED_FORMAT_BYTES);
    remote_round1_zkp_ephemeral_public_key_2 = (uint8_t*)malloc(QAPI_CRYPTO_ECC_P256_PUBLIC_KEY_UNCOMPRESSED_FORMAT_BYTES);
    remote_round1_zkp_signature_2 = (uint8_t*)malloc(QAPI_CRYPTO_ECC_P256_PUB_VAL_X_BYTES);
    if((remote_round1_public_key_1 == NULL) ||
       (remote_round1_zkp_ephemeral_public_key_1 == NULL) ||
	   (remote_round1_zkp_signature_1 == NULL) ||
	   (remote_round1_public_key_2 == NULL) ||
	   (remote_round1_zkp_ephemeral_public_key_2 == NULL) ||
	   (remote_round1_zkp_signature_2 == NULL))
    {
    	status = QAPI_ERR_NO_MEMORY;
    	goto ecjpake_demo_generate_round2_output_cleanup;
    }

    /*
    * Normally the remote round1 attributes would be read over the network, but in
    * the demo we directly read attributes from the round1 output that we generated
    * for the other party.
    */
    status = qapi_Crypto_Obj_Buf_Attrib_Get(remote_round1_public_key_and_zkp_pair_hdl,
    		QAPI_CRYPTO_ATTR_ECJPAKE_ROUND1_PUBLIC_KEY_1_E,
			remote_round1_public_key_1,
			QAPI_CRYPTO_ECC_P256_PUBLIC_KEY_UNCOMPRESSED_FORMAT_BYTES);
    if (status != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to get remote public key\n");
        goto ecjpake_demo_generate_round2_output_cleanup;
    }

    status = qapi_Crypto_Obj_Buf_Attrib_Get(remote_round1_public_key_and_zkp_pair_hdl,
    		QAPI_CRYPTO_ATTR_ECJPAKE_ROUND1_ZKP_EPHEMERAL_PUBLIC_KEY_1_E,
			remote_round1_zkp_ephemeral_public_key_1,
			QAPI_CRYPTO_ECC_P256_PUBLIC_KEY_UNCOMPRESSED_FORMAT_BYTES);
    if (status != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to get remote zkp ephemeral public key\n");
        goto ecjpake_demo_generate_round2_output_cleanup;
    }

    status = qapi_Crypto_Obj_Buf_Attrib_Get(remote_round1_public_key_and_zkp_pair_hdl,
    		QAPI_CRYPTO_ATTR_ECJPAKE_ROUND1_ZKP_SIGNATURE_1_E, remote_round1_zkp_signature_1,
			QAPI_CRYPTO_ECC_P256_PUB_VAL_X_BYTES);
    if (status != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to get remote zkp signature\n");
        goto ecjpake_demo_generate_round2_output_cleanup;
    }

    status = qapi_Crypto_Obj_Buf_Attrib_Get(remote_round1_public_key_and_zkp_pair_hdl,
    		QAPI_CRYPTO_ATTR_ECJPAKE_ROUND1_PUBLIC_KEY_2_E, remote_round1_public_key_2,
			QAPI_CRYPTO_ECC_P256_PUBLIC_KEY_UNCOMPRESSED_FORMAT_BYTES);
    if (status != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to get remote public key\n");
        goto ecjpake_demo_generate_round2_output_cleanup;
    }

    status = qapi_Crypto_Obj_Buf_Attrib_Get(remote_round1_public_key_and_zkp_pair_hdl,
    		QAPI_CRYPTO_ATTR_ECJPAKE_ROUND1_ZKP_EPHEMERAL_PUBLIC_KEY_2_E,
			remote_round1_zkp_ephemeral_public_key_2,
			QAPI_CRYPTO_ECC_P256_PUBLIC_KEY_UNCOMPRESSED_FORMAT_BYTES);
    if (status != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to get remote zkp ephemeral public key\n");
        goto ecjpake_demo_generate_round2_output_cleanup;
    }

    status = qapi_Crypto_Obj_Buf_Attrib_Get(remote_round1_public_key_and_zkp_pair_hdl,
    		QAPI_CRYPTO_ATTR_ECJPAKE_ROUND1_ZKP_SIGNATURE_2_E,
			remote_round1_zkp_signature_2,
			QAPI_CRYPTO_ECC_P256_PUB_VAL_X_BYTES);
    if (status != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to get remote zkp signature\n");
        goto ecjpake_demo_generate_round2_output_cleanup;
    }

    ref_attr_init(&remote_round1_attr[0],
    		QAPI_CRYPTO_ATTR_ECJPAKE_REMOTE_IDENTITY_E,
			remote_id, remote_id_size);

    ref_attr_init(&remote_round1_attr[1],
			QAPI_CRYPTO_ATTR_ECJPAKE_ROUND1_PUBLIC_KEY_1_E,
			remote_round1_public_key_1,
			QAPI_CRYPTO_ECC_P256_PUBLIC_KEY_UNCOMPRESSED_FORMAT_BYTES);

	ref_attr_init(&remote_round1_attr[2],
			QAPI_CRYPTO_ATTR_ECJPAKE_ROUND1_ZKP_EPHEMERAL_PUBLIC_KEY_1_E,
			remote_round1_zkp_ephemeral_public_key_1,
			QAPI_CRYPTO_ECC_P256_PUBLIC_KEY_UNCOMPRESSED_FORMAT_BYTES);

	ref_attr_init(&remote_round1_attr[3],
    		QAPI_CRYPTO_ATTR_ECJPAKE_ROUND1_ZKP_SIGNATURE_1_E,
			remote_round1_zkp_signature_1,
			QAPI_CRYPTO_ECC_P256_PUB_VAL_X_BYTES);

	ref_attr_init(&remote_round1_attr[4],
			QAPI_CRYPTO_ATTR_ECJPAKE_ROUND1_PUBLIC_KEY_2_E,
			remote_round1_public_key_2,
			QAPI_CRYPTO_ECC_P256_PUBLIC_KEY_UNCOMPRESSED_FORMAT_BYTES);

	ref_attr_init(&remote_round1_attr[5],
			QAPI_CRYPTO_ATTR_ECJPAKE_ROUND1_ZKP_EPHEMERAL_PUBLIC_KEY_2_E,
			remote_round1_zkp_ephemeral_public_key_2,
			QAPI_CRYPTO_ECC_P256_PUBLIC_KEY_UNCOMPRESSED_FORMAT_BYTES);

	ref_attr_init(&remote_round1_attr[6],
    		QAPI_CRYPTO_ATTR_ECJPAKE_ROUND1_ZKP_SIGNATURE_2_E,
			remote_round1_zkp_signature_2,
			QAPI_CRYPTO_ECC_P256_PUB_VAL_X_BYTES);

    status = qapi_Crypto_Transient_Obj_Alloc(QAPI_CRYPTO_OBJ_TYPE_ECJPAKE_ROUND2_PUBLIC_KEY_AND_ZKP_E,
    		QAPI_CRYPTO_ECC_P256_PUBLIC_KEY_BITS,
			p_round2_public_key_and_zkp_hdl);
    if (status != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc transient obj\n");
        goto ecjpake_demo_generate_round2_output_cleanup;
    }

    status = qapi_Crypto_Op_Intermediate_Key_Derive(op_hdl, &remote_round1_attr[0],
    		QAPI_CRYPTO_OP_ATTRIB_COUNT_ECJPAKE_DERIVE_ROUND2_KEYS,
        	*p_round2_public_key_and_zkp_hdl);
    if (status != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to generate round2 output\n");
        goto ecjpake_demo_generate_round2_output_cleanup;
    }

ecjpake_demo_generate_round2_output_cleanup:

	if(remote_round1_public_key_1 != 0) {
    	free(remote_round1_public_key_1);
    }
    if(remote_round1_zkp_ephemeral_public_key_1 != 0) {
    	free(remote_round1_zkp_ephemeral_public_key_1);
    }
    if(remote_round1_zkp_signature_1 != 0) {
    	free(remote_round1_zkp_signature_1);
    }
    if(remote_round1_public_key_2 != 0) {
    	free(remote_round1_public_key_2);
    }
    if(remote_round1_zkp_ephemeral_public_key_2 != 0) {
    	free(remote_round1_zkp_ephemeral_public_key_2);
    }
    if(remote_round1_zkp_signature_2 != 0) {
    	free(remote_round1_zkp_signature_2);
    }

	return status;
}

int32_t ecjpake_demo_generate_premaster_secret(qapi_Crypto_Op_Hdl_t op_hdl,
		uint8_t *premaster_secret,
		qapi_Crypto_Obj_Hdl_t remote_round2_public_key_and_zkp_hdl)
{

	int status = 0;
	qapi_Crypto_Attrib_t remote_round2_attr[QAPI_CRYPTO_OP_ATTRIB_COUNT_ECJPAKE_DERIVE_PREMASTER_KEY];
	qapi_Crypto_Obj_Hdl_t premaster_secret_hdl = 0;
	uint8_t *remote_round2_public_key = 0;
	uint8_t *remote_round2_zkp_ephemeral_public_key = 0;
	uint8_t *remote_round2_zkp_signature = 0;

    /*
    * Normally the remote round2 attributes would be read over the network, but in
    * the demo we directly read attributes from the round2 output that we generated
    * for the other party.
    */
	remote_round2_public_key = (uint8_t*)malloc(QAPI_CRYPTO_ECC_P256_PUBLIC_KEY_UNCOMPRESSED_FORMAT_BYTES);
    remote_round2_zkp_ephemeral_public_key = (uint8_t*)malloc(QAPI_CRYPTO_ECC_P256_PUBLIC_KEY_UNCOMPRESSED_FORMAT_BYTES);
    remote_round2_zkp_signature = (uint8_t*)malloc(QAPI_CRYPTO_ECC_P256_PUB_VAL_X_BYTES);
    if((remote_round2_public_key == NULL) ||
       (remote_round2_zkp_ephemeral_public_key == NULL) ||
	   (remote_round2_zkp_signature == NULL))
    {
    	status = QAPI_ERR_NO_MEMORY;
    	goto ecjpake_demo_generate_premaster_secret_cleanup;
    }

    status = qapi_Crypto_Obj_Buf_Attrib_Get(remote_round2_public_key_and_zkp_hdl,
    		QAPI_CRYPTO_ATTR_ECJPAKE_ROUND2_PUBLIC_KEY_E,
			remote_round2_public_key,
			QAPI_CRYPTO_ECC_P256_PUBLIC_KEY_UNCOMPRESSED_FORMAT_BYTES);
    if (status != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to get remote round2 public key\n");
        goto ecjpake_demo_generate_premaster_secret_cleanup;
    }

    status = qapi_Crypto_Obj_Buf_Attrib_Get(remote_round2_public_key_and_zkp_hdl,
    		QAPI_CRYPTO_ATTR_ECJPAKE_ROUND2_ZKP_EPHEMERAL_PUBLIC_KEY_E,
			remote_round2_zkp_ephemeral_public_key,
			QAPI_CRYPTO_ECC_P256_PUBLIC_KEY_UNCOMPRESSED_FORMAT_BYTES);
    if (status != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to get remote round2 zkp ephemeral public key \n");
        goto ecjpake_demo_generate_premaster_secret_cleanup;
    }

    status = qapi_Crypto_Obj_Buf_Attrib_Get(remote_round2_public_key_and_zkp_hdl,
    		QAPI_CRYPTO_ATTR_ECJPAKE_ROUND2_ZKP_SIGNATURE_E,
			remote_round2_zkp_signature,
			QAPI_CRYPTO_ECC_P256_PUB_VAL_X_BYTES);
    if (status != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to get remote round2 zkp signature val\n");
        goto ecjpake_demo_generate_premaster_secret_cleanup;
    }

	ref_attr_init(&remote_round2_attr[0],
			QAPI_CRYPTO_ATTR_ECJPAKE_ROUND2_PUBLIC_KEY_E,
			remote_round2_public_key,
			QAPI_CRYPTO_ECC_P256_PUBLIC_KEY_UNCOMPRESSED_FORMAT_BYTES);

	ref_attr_init(&remote_round2_attr[1],
			QAPI_CRYPTO_ATTR_ECJPAKE_ROUND2_ZKP_EPHEMERAL_PUBLIC_KEY_E,
			remote_round2_zkp_ephemeral_public_key,
			QAPI_CRYPTO_ECC_P256_PUBLIC_KEY_UNCOMPRESSED_FORMAT_BYTES);

	ref_attr_init(&remote_round2_attr[2],
    		QAPI_CRYPTO_ATTR_ECJPAKE_ROUND2_ZKP_SIGNATURE_E,
			remote_round2_zkp_signature,
			QAPI_CRYPTO_ECC_P256_PUB_VAL_X_BYTES);

    status = qapi_Crypto_Transient_Obj_Alloc(QAPI_CRYPTO_OBJ_TYPE_GENERIC_SECRET_E,
    		8 * QAPI_CRYPTO_ECJPAKE_PREMASTER_SECRET_BYTES,
			&premaster_secret_hdl);
    if (status != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc transient obj\n");
        goto ecjpake_demo_generate_premaster_secret_cleanup;
    }

    status = qapi_Crypto_Op_Key_Derive(op_hdl, &remote_round2_attr[0],
    		QAPI_CRYPTO_OP_ATTRIB_COUNT_ECJPAKE_DERIVE_PREMASTER_KEY,
        	premaster_secret_hdl);
    if (status != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to generate premaster secret\n");
        goto ecjpake_demo_generate_premaster_secret_cleanup;
    }

    status = qapi_Crypto_Obj_Buf_Attrib_Get(premaster_secret_hdl,
    		QAPI_CRYPTO_ATTR_SECRET_VALUE_E, premaster_secret,
			QAPI_CRYPTO_ECJPAKE_PREMASTER_SECRET_BYTES);
    if (status != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to read premaster secret\n");
        goto ecjpake_demo_generate_premaster_secret_cleanup;
    }

ecjpake_demo_generate_premaster_secret_cleanup:

	qapi_Crypto_Transient_Obj_Free(premaster_secret_hdl);
	if(remote_round2_public_key != 0) {
		free(remote_round2_public_key);
	}
	if(remote_round2_zkp_ephemeral_public_key != 0) {
		free(remote_round2_zkp_ephemeral_public_key);
	}
	if(remote_round2_zkp_signature != 0) {
		free(remote_round2_zkp_signature);
	}

	return status;
}

int32_t ecjpake_demo_run(const uint8_t *client_password, uint32_t client_password_size,
		const uint8_t *server_password, uint32_t server_password_size,
		uint8_t *premaster_secret_client, uint8_t *premaster_secret_server)
{
	qapi_Crypto_Op_Hdl_t client_op_hdl = 0;
	qapi_Crypto_Obj_Hdl_t client_round1_public_key_and_zkp_pair_hdl = 0;
	qapi_Crypto_Obj_Hdl_t client_round2_public_key_and_zkp_hdl = 0;

	qapi_Crypto_Op_Hdl_t server_op_hdl = 0;
	qapi_Crypto_Obj_Hdl_t server_round1_public_key_and_zkp_pair_hdl = 0;
	qapi_Crypto_Obj_Hdl_t server_round2_public_key_and_zkp_hdl = 0;

    int status = 0;

    /* Create the ECJPAKE operations. */
    status = ecjpake_demo_init(&client_op_hdl);
    if (status != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc client op\n");
        goto ecjpake_demo_run_cleanup;
    }

    status = ecjpake_demo_init(&server_op_hdl);
    if (status != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc server op\n");
        goto ecjpake_demo_run_cleanup;
    }

    /* Generate client round1 keys */
    status = ecjpake_demo_generate_round1_output(client_op_hdl,
    		&client_round1_public_key_and_zkp_pair_hdl,
    		ecjpake_test_client_id,
			sizeof(ecjpake_test_client_id),
			client_password,
			client_password_size);
    if (status != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to generate client round1 output \n");
        goto ecjpake_demo_run_cleanup;
    }

    /* Generate server round1 keys */
    status = ecjpake_demo_generate_round1_output(server_op_hdl,
    		&server_round1_public_key_and_zkp_pair_hdl,
    		ecjpake_test_server_id,
			sizeof(ecjpake_test_server_id),
			server_password,
			server_password_size);
    if (status != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to generate server round1 output \n");
        goto ecjpake_demo_run_cleanup;
    }

    /* Client sets server identity and round1 output and its generates round 2 output.*/
    status = ecjpake_demo_generate_round2_output(client_op_hdl,
    		&client_round2_public_key_and_zkp_hdl,
    		server_round1_public_key_and_zkp_pair_hdl,
    		ecjpake_test_server_id,
			sizeof(ecjpake_test_server_id));
    if (status != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to generate client round2 output \n");
        goto ecjpake_demo_run_cleanup;
    }

    /* Server sets client identity and round1 output and its generates round 2 output.*/
    status = ecjpake_demo_generate_round2_output(server_op_hdl,
    		&server_round2_public_key_and_zkp_hdl,
    		client_round1_public_key_and_zkp_pair_hdl,
    		ecjpake_test_client_id,
			sizeof(ecjpake_test_client_id));
    if (status != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to generate server round2 output \n");
        goto ecjpake_demo_run_cleanup;
    }

    /* Client sets server round2 parameters and generates the premaster secret. */
    status = ecjpake_demo_generate_premaster_secret(client_op_hdl,
    		premaster_secret_client,
    		server_round2_public_key_and_zkp_hdl);
    if (status != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to generate client premaster secret \n");
        goto ecjpake_demo_run_cleanup;
    }

    /* Server sets client round2 parameters and generates the premaster secret. */
    status = ecjpake_demo_generate_premaster_secret(server_op_hdl,
    		premaster_secret_server,
    		client_round2_public_key_and_zkp_hdl);
    if (status != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to generate server premaster secret \n");
        goto ecjpake_demo_run_cleanup;
    }

    if(crypto_demo_is_buffer_all_zeros(premaster_secret_client, QAPI_CRYPTO_ECJPAKE_PREMASTER_SECRET_BYTES)) {
    	CRYPTO_PRINTF("Error derived premaster key is all zeros\n");
    }

ecjpake_demo_run_cleanup:

    qapi_Crypto_Op_Free(client_op_hdl);
    qapi_Crypto_Transient_Obj_Free(client_round1_public_key_and_zkp_pair_hdl);
    qapi_Crypto_Transient_Obj_Free(client_round2_public_key_and_zkp_hdl);

	qapi_Crypto_Op_Free(server_op_hdl);
	qapi_Crypto_Transient_Obj_Free(server_round1_public_key_and_zkp_pair_hdl);
	qapi_Crypto_Transient_Obj_Free(server_round2_public_key_and_zkp_hdl);

	return status;
}

int32_t ecjpake_demo()
{
	uint8_t *premaster_secret_client = 0;
	uint8_t *premaster_secret_server = 0;
	int status = QAPI_OK;

    premaster_secret_client = (uint8_t*)malloc(QAPI_CRYPTO_ECJPAKE_PREMASTER_SECRET_BYTES);
    premaster_secret_server = (uint8_t*)malloc(QAPI_CRYPTO_ECJPAKE_PREMASTER_SECRET_BYTES);
    if((premaster_secret_client == NULL) || (premaster_secret_server == NULL))
    {
    	status = QAPI_ERR_NO_MEMORY;
    	goto ecjpake_demo_cleanup;
    }

	status = ecjpake_demo_run(ecjpake_test_password, sizeof(ecjpake_test_password),
			ecjpake_test_password, sizeof(ecjpake_test_password),
			premaster_secret_client, premaster_secret_server);

    if((status == QAPI_OK) &&
       (memcmp(premaster_secret_client, premaster_secret_server, QAPI_CRYPTO_ECJPAKE_PREMASTER_SECRET_BYTES) == 0)
	  )
    {
		CRYPTO_PRINTF("ECJPAKE positive test case  Passed\n");
	} else {
		CRYPTO_PRINTF("ECJPAKE positive test case  Failed\n");
	}

	qapi_Crypto_Secure_Memzero(premaster_secret_client, QAPI_CRYPTO_ECJPAKE_PREMASTER_SECRET_BYTES);
	qapi_Crypto_Secure_Memzero(premaster_secret_server, QAPI_CRYPTO_ECJPAKE_PREMASTER_SECRET_BYTES);

	status = ecjpake_demo_run(ecjpake_test_password, sizeof(ecjpake_test_password),
			ecjpake_test_wrong_password, sizeof(ecjpake_test_wrong_password),
			premaster_secret_client, premaster_secret_server);

	/* Negative test is when the two parties do not share the same secrect.
	 * In this case the algorithm should still run and the status should
	 * be QAPI_OK, but the parties would not derive the same key.
	 */
    if( (status == QAPI_OK) &&
    	(memcmp(premaster_secret_client, premaster_secret_server, QAPI_CRYPTO_ECJPAKE_PREMASTER_SECRET_BYTES) != 0)
	  )
    {
    	CRYPTO_PRINTF("ECJPAKE negative test case  Passed\n");
	} else {
		CRYPTO_PRINTF("ECJPAKE negative test case  Failed\n");
	}

ecjpake_demo_cleanup:
	qapi_Crypto_Secure_Memzero(premaster_secret_client, QAPI_CRYPTO_ECJPAKE_PREMASTER_SECRET_BYTES);
	if(premaster_secret_client != 0) {
		free(premaster_secret_client);
	}
	qapi_Crypto_Secure_Memzero(premaster_secret_server, QAPI_CRYPTO_ECJPAKE_PREMASTER_SECRET_BYTES);
	if(premaster_secret_server != 0) {
		free(premaster_secret_server);
	}
	return status;
}

int32_t ecdsa_demo()
{
        qapi_Crypto_Obj_Hdl_t obj_hdl;
        qapi_Crypto_Op_Hdl_t sign_hdl, verify_hdl;
        qapi_Crypto_Attrib_t attr[4];
        unsigned char sm[QAPI_CRYPTO_ECDSA_SIGNATURE_BUFFER_SIZE_BYTES(QAPI_CRYPTO_ECC_P256_KEYPAIR_BYTES)];
        uint32_t smlen = 0;
        int i;

        uint8_t pub_key_X[QAPI_CRYPTO_ECC_P256_PUB_VAL_X_BYTES];
        uint8_t pub_key_Y[QAPI_CRYPTO_ECC_P256_PUB_VAL_Y_BYTES];
        uint8_t pvt_key[QAPI_CRYPTO_ECC_P256_PRIVATE_KEY_BYTES];
#define DIGEST_LEN (256/8)
        uint8_t digest[DIGEST_LEN];
        for (i = 0; i < DIGEST_LEN; i++) {
            digest[i] = i;
        }


        {
            /* ECDSA demo 1 - sign/verify using key_gen */

            /* Allocate ECDSA keypair object */
            if (qapi_Crypto_Transient_Obj_Alloc(QAPI_CRYPTO_OBJ_TYPE_ECDSA_KEYPAIR_E, QAPI_CRYPTO_ECC_P256_KEYPAIR_BITS, 
                        &obj_hdl) != QAPI_OK) {
                CRYPTO_PRINTF("\nFailed to alloc transient obj\n");
                return -1;
            }

            /* Generate ECDSA keypair for a given curve id */
            val_attr_init(attr, QAPI_CRYPTO_ATTR_ECC_CURVE_E, QAPI_CRYPTO_ECC_CURVE_NIST_P256, 0);
            if (qapi_Crypto_Transient_Obj_Key_Gen(obj_hdl, QAPI_CRYPTO_ECC_P256_KEYPAIR_BITS, attr, 1) != QAPI_OK) {
                CRYPTO_PRINTF("\nFailed to gen key\n");
                return -1;
            }

            /* Allocate ECDSA sign operation */
            if (qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_ECDSA_P256_E, QAPI_CRYPTO_MODE_SIGN_E, QAPI_CRYPTO_ECC_P256_PRIVATE_KEY_BITS, 
                        &sign_hdl) != QAPI_OK) {
                CRYPTO_PRINTF("\nFailed to alloc op\n");
                return -1;
            }

            /* Copy private key from keypair object for sign operation */
            if (qapi_Crypto_Op_Key_Set(sign_hdl, obj_hdl) != QAPI_OK) {
                CRYPTO_PRINTF("\nFailed to set key\n");
                return -1;
            }

            /* Allocate ECDSA verify operation */
            if (qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_ECDSA_P256_E, QAPI_CRYPTO_MODE_VERIFY_E, QAPI_CRYPTO_ECC_P256_PUBLIC_KEY_BITS, &verify_hdl) != QAPI_OK) {
                CRYPTO_PRINTF("\nFailed to alloc verify op\n");
                return -1;
            }


            /* Copy public key from keypair object for verify operation */
            if (qapi_Crypto_Op_Key_Set(verify_hdl, obj_hdl) != QAPI_OK) {
                CRYPTO_PRINTF("\nFailed to set key\n");
                return -1;
            }

            /* Generate signature */
            smlen = sizeof(sm);
            if (qapi_Crypto_Op_Sign_Digest(sign_hdl, NULL, 0, digest, DIGEST_LEN, sm, &smlen) != QAPI_OK) {
                CRYPTO_PRINTF("\nFailed to sign\n");
                return -1;
            }


            /* Verify signature */
            if (qapi_Crypto_Op_Verify_Digest(verify_hdl, NULL, 0, digest, DIGEST_LEN, sm, smlen) != QAPI_OK) {
                CRYPTO_PRINTF("\nFailed verify\n");
                return -1;
            }

            CRYPTO_PRINTF("\nDemo 1 successful\n");

            qapi_Crypto_Transient_Obj_Free(obj_hdl);
            qapi_Crypto_Op_Free(sign_hdl);
            qapi_Crypto_Op_Free(verify_hdl);
        }

        {
            /* DEMO 2 - sign/verify using populate QAPI 
             *
             * In the real world, the public key would be received over the
             * air and private key may be provisioned to the device. In
             * those cases, the populate API should be used 
             *
             * Here, for demo purposes, keygen API is used to generate the key;
             * buf_attr_get API to extract the keys and populate API to copy the
             * keys into the object
             *
             * */

            qapi_Crypto_Obj_Hdl_t keygen_hdl, keypair_hdl, public_key_hdl;
            /* Allocate ECDSA keypair object */
            if (qapi_Crypto_Transient_Obj_Alloc(QAPI_CRYPTO_OBJ_TYPE_ECDSA_KEYPAIR_E, QAPI_CRYPTO_ECC_P256_KEYPAIR_BITS, &keygen_hdl) != QAPI_OK) {
                CRYPTO_PRINTF("\nFailed to alloc transient obj\n");
                return -1;
            }

            /* Generate ECDSA keypair for a given curve id */
            val_attr_init(attr, QAPI_CRYPTO_ATTR_ECC_CURVE_E, QAPI_CRYPTO_ECC_CURVE_NIST_P256, 0);
            if (qapi_Crypto_Transient_Obj_Key_Gen(keygen_hdl, QAPI_CRYPTO_ECC_P256_KEYPAIR_BITS, attr, 1) != QAPI_OK) {
                CRYPTO_PRINTF("\nFailed to gen key\n");
                return -1;
            }

            /* Get the attributes from the generated keypair object */
            if (qapi_Crypto_Obj_Buf_Attrib_Get(keygen_hdl, QAPI_CRYPTO_ATTR_ECC_PUBLIC_VALUE_X_E, pub_key_X, QAPI_CRYPTO_ECC_P256_PUB_VAL_X_BYTES) != QAPI_OK) {
                CRYPTO_PRINTF("\nFailed to get public val X\n");
                return -1;
            }
            if (qapi_Crypto_Obj_Buf_Attrib_Get(keygen_hdl, QAPI_CRYPTO_ATTR_ECC_PUBLIC_VALUE_Y_E, pub_key_Y, QAPI_CRYPTO_ECC_P256_PUB_VAL_Y_BYTES) != QAPI_OK) {
                CRYPTO_PRINTF("\nFailed to get public val Y\n");
                return -1;
            }

            if (qapi_Crypto_Obj_Buf_Attrib_Get(keygen_hdl, QAPI_CRYPTO_ATTR_ECC_PRIVATE_VALUE_E, pvt_key, QAPI_CRYPTO_ECC_P256_PRIVATE_KEY_BYTES) != QAPI_OK) {
                CRYPTO_PRINTF("\nFailed to get public val X\n");
                return -1;
            }

            /* Allocate the keypair object that will be populated using
             * the attributes obtained from above. This object will be
             * used for signing
             */
            if (qapi_Crypto_Transient_Obj_Alloc(QAPI_CRYPTO_OBJ_TYPE_ECDSA_KEYPAIR_E, QAPI_CRYPTO_ECC_P256_KEYPAIR_BITS, &keypair_hdl) != QAPI_OK) {
                CRYPTO_PRINTF("\nFailed to alloc transient obj\n");
                return -1;
            }

            /* Populate attributes */
            ref_attr_init(&attr[0], QAPI_CRYPTO_ATTR_ECC_PRIVATE_VALUE_E, pvt_key,QAPI_CRYPTO_ECC_P256_PRIVATE_KEY_BYTES);
            ref_attr_init(&attr[1], QAPI_CRYPTO_ATTR_ECC_PUBLIC_VALUE_X_E, pub_key_X, QAPI_CRYPTO_ECC_P256_PRIVATE_KEY_BYTES);
            ref_attr_init(&attr[2], QAPI_CRYPTO_ATTR_ECC_PUBLIC_VALUE_Y_E, pub_key_Y, QAPI_CRYPTO_ECC_P256_PRIVATE_KEY_BYTES);
            val_attr_init(&attr[3], QAPI_CRYPTO_ATTR_ECC_CURVE_E, QAPI_CRYPTO_ECC_CURVE_NIST_P256, 0);
            if (qapi_Crypto_Transient_Obj_Populate(keypair_hdl, attr, QAPI_CRYPTO_OBJ_ATTRIB_COUNT_ECC_KEYPAIR) != QAPI_OK) {
                CRYPTO_PRINTF("\nFailed to populate ecc keypair obj\n");
                return -1;
            }

            /* Allocate public key object for use in verify operation */
            if (qapi_Crypto_Transient_Obj_Alloc(QAPI_CRYPTO_OBJ_TYPE_ECDSA_PUBLIC_KEY_E, QAPI_CRYPTO_ECC_P256_PUBLIC_KEY_BITS, &public_key_hdl) != QAPI_OK) {
                CRYPTO_PRINTF("\nFailed to alloc transient obj\n");
                return -1;
            }

            /* Populate public key object */
            if (qapi_Crypto_Transient_Obj_Populate(public_key_hdl, &attr[1], QAPI_CRYPTO_OBJ_ATTRIB_COUNT_ECC_PUBLIC_KEY) != QAPI_OK) {
                CRYPTO_PRINTF("\nFailed to populate ecc public obj\n");
                return -1;
            }

            /* Allocate sign operation */
            if (qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_ECDSA_P256_E, QAPI_CRYPTO_MODE_SIGN_E, QAPI_CRYPTO_ECC_P256_PRIVATE_KEY_BITS, &sign_hdl) != QAPI_OK) {
                CRYPTO_PRINTF("\nFailed to alloc op\n");
                return -1;
            }

            /* Copy private key from populated keypair object */
            if (qapi_Crypto_Op_Key_Set(sign_hdl, keypair_hdl) != QAPI_OK) {
                CRYPTO_PRINTF("\nFailed to set key\n");
                return -1;
            }

            /* Allocate verify operation */
            if (qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_ECDSA_P256_E, QAPI_CRYPTO_MODE_VERIFY_E, QAPI_CRYPTO_ECC_P256_PUBLIC_KEY_BITS, &verify_hdl) != QAPI_OK) {
                CRYPTO_PRINTF("\nFailed to alloc verify op\n");
                return -1;
            }

            /* Copy public key from populated public key object */
            if (qapi_Crypto_Op_Key_Set(verify_hdl, public_key_hdl) != QAPI_OK) {
                CRYPTO_PRINTF("\nFailed to set key\n");
                return -1;
            }

            /* Generate signature */
            smlen = sizeof(sm);
            if (qapi_Crypto_Op_Sign_Digest(sign_hdl, NULL, 0, digest, DIGEST_LEN, sm, &smlen) != QAPI_OK) {
                CRYPTO_PRINTF("\nFailed to sign\n");
                return -1;
            }


            /* Verify signature */
            smlen = sizeof(sm);
            if (qapi_Crypto_Op_Verify_Digest(verify_hdl, NULL, 0, digest, DIGEST_LEN, sm, smlen) != QAPI_OK) {
                CRYPTO_PRINTF("\nFailed verify\n");
                return -1;
            }

            CRYPTO_PRINTF("\nDemo 2 successful\n");

            qapi_Crypto_Transient_Obj_Free(keygen_hdl);
            qapi_Crypto_Transient_Obj_Free(keypair_hdl);
            qapi_Crypto_Transient_Obj_Free(public_key_hdl);
            qapi_Crypto_Op_Free(sign_hdl);
            qapi_Crypto_Op_Free(verify_hdl);
        }
            
    return 0;        
} 
#if ENABLE_DH_DEMO
int32_t dh_demo()
{
    /* 1. Allocate two dh key pair objects 
     * 2. Generate keys
     * 3. Extract public key from each object
     * 4. Allocate two derived key operations
     * 4. Derive shared keys using secret key from obj1 and public key from
     * obj2 and vice versa
     * 5. Verify that shared keys match
     */
    qapi_Crypto_Obj_Hdl_t dh_hdl_1, dh_hdl_2, derived_key_hdl_1, derived_key_hdl_2;
    qapi_Crypto_Op_Hdl_t op_hdl_1, op_hdl_2;
    qapi_Crypto_Attrib_t attr[QAPI_CRYPTO_OBJ_ATTRIB_COUNT_DH_KEYGEN];
    uint8_t pub_key_1[PRIME_BYTES];
    uint8_t pub_key_2[PRIME_BYTES];
    uint8_t derived_key_1[PRIME_BYTES];
    uint8_t derived_key_2[PRIME_BYTES];


    if (qapi_Crypto_Transient_Obj_Alloc(QAPI_CRYPTO_OBJ_TYPE_DH_KEYPAIR_E, PRIME_BITS, &dh_hdl_1) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc transient obj\n");
        return -1;
    }

    ref_attr_init(&attr[0], QAPI_CRYPTO_ATTR_DH_PRIME_E, p_1024, PRIME_BYTES);
    ref_attr_init(&attr[1], QAPI_CRYPTO_ATTR_DH_BASE_E, g, 4);
    if (qapi_Crypto_Transient_Obj_Key_Gen(dh_hdl_1, PRIME_BITS, attr, QAPI_CRYPTO_OBJ_ATTRIB_COUNT_DH_KEYGEN) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to gen key\n");
        return -1;
    }

    if (qapi_Crypto_Obj_Buf_Attrib_Get(dh_hdl_1, QAPI_CRYPTO_ATTR_DH_PUBLIC_VALUE_E, pub_key_1, PRIME_BYTES) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to get public val\n");
        return -1;
    }

    if (qapi_Crypto_Transient_Obj_Alloc(QAPI_CRYPTO_OBJ_TYPE_DH_KEYPAIR_E, PRIME_BITS, &dh_hdl_2) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc transient obj\n");
        return -1;
    }

    if (qapi_Crypto_Transient_Obj_Key_Gen(dh_hdl_2, PRIME_BITS, attr, QAPI_CRYPTO_OBJ_ATTRIB_COUNT_DH_KEYGEN) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to gen key\n");
        return -1;
    }

    if (qapi_Crypto_Obj_Buf_Attrib_Get(dh_hdl_2, QAPI_CRYPTO_ATTR_DH_PUBLIC_VALUE_E, pub_key_2, PRIME_BYTES) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to get public val\n");
        return -1;
    }


    if (qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_DH_DERIVE_SHARED_SECRET_E, QAPI_CRYPTO_MODE_DERIVE_E, PRIME_BITS, &op_hdl_1) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc op\n");
        return -1;
    }

    if (qapi_Crypto_Op_Key_Set(op_hdl_1, dh_hdl_1) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to set key\n");
        return -1;
    }


    if (qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_DH_DERIVE_SHARED_SECRET_E, QAPI_CRYPTO_MODE_DERIVE_E, PRIME_BITS, &op_hdl_2) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc op\n");
        return -1;
    }

    if (qapi_Crypto_Op_Key_Set(op_hdl_2, dh_hdl_2) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to set key\n");
        return -1;
    }

    if (qapi_Crypto_Transient_Obj_Alloc(QAPI_CRYPTO_OBJ_TYPE_GENERIC_SECRET_E, PRIME_BITS, &derived_key_hdl_1) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc transient obj\n");
        return -1;
    }

    if (qapi_Crypto_Transient_Obj_Alloc(QAPI_CRYPTO_OBJ_TYPE_GENERIC_SECRET_E, PRIME_BITS, &derived_key_hdl_2) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc transient obj\n");
        return -1;
    }


    ref_attr_init(&attr[0], QAPI_CRYPTO_ATTR_DH_PUBLIC_VALUE_E, pub_key_2, PRIME_BYTES);
    if (qapi_Crypto_Op_Key_Derive(op_hdl_1, &attr[0], 1, derived_key_hdl_1) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to derive key 1\n");
        return -1;
    }

    ref_attr_init(&attr[0], QAPI_CRYPTO_ATTR_DH_PUBLIC_VALUE_E, pub_key_1, PRIME_BYTES);
    if (qapi_Crypto_Op_Key_Derive(op_hdl_2, &attr[0], 1, derived_key_hdl_2) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to derive key 2\n");
        return -1;
    }

    if (qapi_Crypto_Obj_Buf_Attrib_Get(derived_key_hdl_1, QAPI_CRYPTO_ATTR_SECRET_VALUE_E, derived_key_1, PRIME_BYTES) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to get derived key val 1\n");
        return -1;
    }

    if (qapi_Crypto_Obj_Buf_Attrib_Get(derived_key_hdl_2, QAPI_CRYPTO_ATTR_SECRET_VALUE_E, derived_key_2, PRIME_BYTES) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to get derived key val 2\n");
        return -1;
    }

    if (memcmp(derived_key_1, derived_key_2, QAPI_CRYPTO_ECC_P256_SHARED_SECRET_BYTES) == 0) {
        CRYPTO_PRINTF("\nDerived keys match!\n");

    }
    else {
        CRYPTO_PRINTF("\nDerived keys DONOT match!\n");
    }

    qapi_Crypto_Transient_Obj_Free(dh_hdl_1);
    qapi_Crypto_Transient_Obj_Free(dh_hdl_2);
    qapi_Crypto_Transient_Obj_Free(derived_key_hdl_2);
    qapi_Crypto_Transient_Obj_Free(derived_key_hdl_1);
    qapi_Crypto_Op_Free(op_hdl_1);
    qapi_Crypto_Op_Free(op_hdl_2);

    return 0;
}
#endif

int32_t ecdh_demo()
{
    /* 1. Allocate two ecdh key pair objects 
     * 2. Generate keys
     * 3. Extract public key from each object
     * 4. Allocate two derived key operations
     * 4. Derive shared keys using secret key from obj1 and public key from
     * obj2 and vice versa
     * 5. Verify that shared keys match
     */
    qapi_Crypto_Obj_Hdl_t ecc_hdl_1, ecc_hdl_2, derived_key_hdl_1, derived_key_hdl_2;
    qapi_Crypto_Op_Hdl_t op_hdl_1, op_hdl_2;
    qapi_Crypto_Attrib_t attr[2];
    uint8_t pub_key_X_1[QAPI_CRYPTO_ECC_P256_PUB_VAL_X_BYTES];
    uint8_t pub_key_X_2[QAPI_CRYPTO_ECC_P256_PUB_VAL_X_BYTES];
    uint8_t pub_key_Y_1[QAPI_CRYPTO_ECC_P256_PUB_VAL_Y_BYTES];
    uint8_t pub_key_Y_2[QAPI_CRYPTO_ECC_P256_PUB_VAL_Y_BYTES];
    uint8_t derived_key_1[QAPI_CRYPTO_ECC_P256_PUB_VAL_X_BYTES];
    uint8_t derived_key_2[QAPI_CRYPTO_ECC_P256_PUB_VAL_X_BYTES];


    if (qapi_Crypto_Transient_Obj_Alloc(QAPI_CRYPTO_OBJ_TYPE_ECDH_KEYPAIR_E, QAPI_CRYPTO_ECC_P256_KEYPAIR_BITS, &ecc_hdl_1) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc transient obj\n");
        return -1;
    }

    val_attr_init(attr, QAPI_CRYPTO_ATTR_ECC_CURVE_E, QAPI_CRYPTO_ECC_CURVE_NIST_P256, 0);
    if (qapi_Crypto_Transient_Obj_Key_Gen(ecc_hdl_1, QAPI_CRYPTO_ECC_P256_KEYPAIR_BITS, attr, 1) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to gen key\n");
        return -1;
    }

    if (qapi_Crypto_Obj_Buf_Attrib_Get(ecc_hdl_1, QAPI_CRYPTO_ATTR_ECC_PUBLIC_VALUE_X_E, pub_key_X_1, QAPI_CRYPTO_ECC_P256_PUB_VAL_X_BYTES) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to get public val X\n");
        return -1;
    }

    if (qapi_Crypto_Obj_Buf_Attrib_Get(ecc_hdl_1, QAPI_CRYPTO_ATTR_ECC_PUBLIC_VALUE_Y_E, pub_key_Y_1, QAPI_CRYPTO_ECC_P256_PUB_VAL_Y_BYTES) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to get public val X\n");
        return -1;
    }

    if (qapi_Crypto_Transient_Obj_Alloc(QAPI_CRYPTO_OBJ_TYPE_ECDH_KEYPAIR_E, QAPI_CRYPTO_ECC_P256_KEYPAIR_BITS, &ecc_hdl_2) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc transient obj\n");
        return -1;
    }

    val_attr_init(attr, QAPI_CRYPTO_ATTR_ECC_CURVE_E, QAPI_CRYPTO_ECC_CURVE_NIST_P256, 0);
    if (qapi_Crypto_Transient_Obj_Key_Gen(ecc_hdl_2, QAPI_CRYPTO_ECC_P256_KEYPAIR_BITS, attr, 1) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to gen key\n");
        return -1;
    }

    if (qapi_Crypto_Obj_Buf_Attrib_Get(ecc_hdl_2, QAPI_CRYPTO_ATTR_ECC_PUBLIC_VALUE_X_E, pub_key_X_2, QAPI_CRYPTO_ECC_P256_PUB_VAL_X_BYTES) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to get public val X\n");
        return -1;
    }

    if (qapi_Crypto_Obj_Buf_Attrib_Get(ecc_hdl_2, QAPI_CRYPTO_ATTR_ECC_PUBLIC_VALUE_Y_E, pub_key_Y_2, QAPI_CRYPTO_ECC_P256_PUB_VAL_Y_BYTES) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to get public val Y\n");
        return -1;
    }


    if (qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_ECDH_P256_E, QAPI_CRYPTO_MODE_DERIVE_E, QAPI_CRYPTO_ECC_P256_SHARED_SECRET_BITS, &op_hdl_1) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc op\n");
        return -1;
    }

    if (qapi_Crypto_Op_Key_Set(op_hdl_1, ecc_hdl_1) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to set key\n");
        return -1;
    }


    if (qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_ECDH_P256_E, QAPI_CRYPTO_MODE_DERIVE_E, QAPI_CRYPTO_ECC_P256_SHARED_SECRET_BITS, &op_hdl_2) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc op\n");
        return -1;
    }

    if (qapi_Crypto_Op_Key_Set(op_hdl_2, ecc_hdl_2) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to set key\n");
        return -1;
    }

    if (qapi_Crypto_Transient_Obj_Alloc(QAPI_CRYPTO_OBJ_TYPE_GENERIC_SECRET_E, QAPI_CRYPTO_ECC_P256_SHARED_SECRET_BITS, &derived_key_hdl_1) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc transient obj\n");
        return -1;
    }

    if (qapi_Crypto_Transient_Obj_Alloc(QAPI_CRYPTO_OBJ_TYPE_GENERIC_SECRET_E, QAPI_CRYPTO_ECC_P256_SHARED_SECRET_BITS, &derived_key_hdl_2) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc transient obj\n");
        return -1;
    }


    ref_attr_init(&attr[0], QAPI_CRYPTO_ATTR_ECC_PUBLIC_VALUE_X_E, pub_key_X_2, QAPI_CRYPTO_ECC_P256_PUB_VAL_X_BYTES);
    ref_attr_init(&attr[1], QAPI_CRYPTO_ATTR_ECC_PUBLIC_VALUE_Y_E, pub_key_Y_2, QAPI_CRYPTO_ECC_P256_PUB_VAL_Y_BYTES);
    if (qapi_Crypto_Op_Key_Derive(op_hdl_1, &attr[0], 2, derived_key_hdl_1) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to derive key 1\n");
        return -1;
    }

    ref_attr_init(&attr[0], QAPI_CRYPTO_ATTR_ECC_PUBLIC_VALUE_X_E, pub_key_X_1, QAPI_CRYPTO_ECC_P256_PUB_VAL_X_BYTES);
    ref_attr_init(&attr[1], QAPI_CRYPTO_ATTR_ECC_PUBLIC_VALUE_Y_E, pub_key_Y_1, QAPI_CRYPTO_ECC_P256_PUB_VAL_Y_BYTES);
    if (qapi_Crypto_Op_Key_Derive(op_hdl_2, &attr[0], 2, derived_key_hdl_2) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to derive key 2\n");
        return -1;
    }

    if (qapi_Crypto_Obj_Buf_Attrib_Get(derived_key_hdl_1, QAPI_CRYPTO_ATTR_SECRET_VALUE_E, derived_key_1, QAPI_CRYPTO_ECC_P256_SHARED_SECRET_BYTES) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to get derived key val 1\n");
        return -1;
    }

    if (qapi_Crypto_Obj_Buf_Attrib_Get(derived_key_hdl_2, QAPI_CRYPTO_ATTR_SECRET_VALUE_E, derived_key_2, QAPI_CRYPTO_ECC_P256_SHARED_SECRET_BYTES) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to get derived key val 2\n");
        return -1;
    }

    if (memcmp(derived_key_1, derived_key_2, QAPI_CRYPTO_ECC_P256_SHARED_SECRET_BYTES) == 0) {
        CRYPTO_PRINTF("\nDerived keys match!\n");

    }
    else {
        CRYPTO_PRINTF("\nDerived keys DONOT match!\n");
    }

    qapi_Crypto_Transient_Obj_Free(ecc_hdl_1);
    qapi_Crypto_Transient_Obj_Free(ecc_hdl_2);
    qapi_Crypto_Transient_Obj_Free(derived_key_hdl_2);
    qapi_Crypto_Transient_Obj_Free(derived_key_hdl_1);
    qapi_Crypto_Op_Free(op_hdl_1);
    qapi_Crypto_Op_Free(op_hdl_2);
    return 0;
}

int32_t curve_demo()
{
    /* 1. Allocate two curve25519 key pair objects 
     * 2. Generate keys
     * 3. Extract public key from each object
     * 4. Allocate two derived key operations
     * 4. Derive shared keys using secret key from obj1 and public key from
     * obj2 and vice versa
     * 5. Verify that shared keys match
     */
    qapi_Crypto_Obj_Hdl_t curve_hdl_1, curve_hdl_2, derived_key_hdl_1, derived_key_hdl_2;
    qapi_Crypto_Op_Hdl_t op_hdl_1, op_hdl_2;
    uint8_t pub_key_1[QAPI_CRYPTO_CURVE25519_PUBLIC_KEY_BYTES];
    uint8_t pub_key_2[QAPI_CRYPTO_CURVE25519_PUBLIC_KEY_BYTES];
    qapi_Crypto_Attrib_t attr[2];
    uint8_t derived_key_1[QAPI_CRYPTO_CURVE25519_SHARED_SECRET_BYTES];
    uint8_t derived_key_2[QAPI_CRYPTO_CURVE25519_SHARED_SECRET_BYTES];


    if (qapi_Crypto_Transient_Obj_Alloc(QAPI_CRYPTO_OBJ_TYPE_CURVE25519_KEYPAIR_E, QAPI_CRYPTO_CURVE25519_KEYPAIR_BITS, &curve_hdl_1) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc transient obj\n");
        return -1;
    }

    if (qapi_Crypto_Transient_Obj_Key_Gen(curve_hdl_1, QAPI_CRYPTO_CURVE25519_KEYPAIR_BITS, NULL, 0) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to gen key\n");
        return -1;
    }

    if (qapi_Crypto_Obj_Buf_Attrib_Get(curve_hdl_1, QAPI_CRYPTO_ATTR_CURVE25519_PUBLIC_VALUE_E, pub_key_1, QAPI_CRYPTO_CURVE25519_PUBLIC_KEY_BYTES) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to get public val X\n");
        return -1;
    }

    if (qapi_Crypto_Transient_Obj_Alloc(QAPI_CRYPTO_OBJ_TYPE_CURVE25519_KEYPAIR_E, QAPI_CRYPTO_CURVE25519_KEYPAIR_BITS, &curve_hdl_2) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc transient obj\n");
        return -1;
    }

    if (qapi_Crypto_Transient_Obj_Key_Gen(curve_hdl_2, QAPI_CRYPTO_CURVE25519_KEYPAIR_BITS, NULL, 0) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to gen key\n");
        return -1;
    }

    if (qapi_Crypto_Obj_Buf_Attrib_Get(curve_hdl_2, QAPI_CRYPTO_ATTR_CURVE25519_PUBLIC_VALUE_E, pub_key_2, QAPI_CRYPTO_CURVE25519_PUBLIC_KEY_BYTES) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to get public val X\n");
        return -1;
    }
    if (qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_CURVE25519_DERIVE_SHARED_SECRET_E, QAPI_CRYPTO_MODE_DERIVE_E, QAPI_CRYPTO_CURVE25519_SHARED_SECRET_BITS, &op_hdl_1) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc op\n");
        return -1;
    }

    if (qapi_Crypto_Op_Key_Set(op_hdl_1, curve_hdl_1) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to set key\n");
        return -1;
    }

    if (qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_CURVE25519_DERIVE_SHARED_SECRET_E, QAPI_CRYPTO_MODE_DERIVE_E, QAPI_CRYPTO_CURVE25519_SHARED_SECRET_BITS, &op_hdl_2) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc op\n");
        return -1;
    }

    if (qapi_Crypto_Op_Key_Set(op_hdl_2, curve_hdl_2) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to set key\n");
        return -1;
    }

    if (qapi_Crypto_Transient_Obj_Alloc(QAPI_CRYPTO_OBJ_TYPE_GENERIC_SECRET_E, QAPI_CRYPTO_CURVE25519_SHARED_SECRET_BITS, &derived_key_hdl_1) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc transient obj\n");
        return -1;
    }

    if (qapi_Crypto_Transient_Obj_Alloc(QAPI_CRYPTO_OBJ_TYPE_GENERIC_SECRET_E, QAPI_CRYPTO_CURVE25519_SHARED_SECRET_BITS, &derived_key_hdl_2) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc transient obj\n");
        return -1;
    }


    ref_attr_init(attr, QAPI_CRYPTO_ATTR_CURVE25519_PUBLIC_VALUE_E, pub_key_2, QAPI_CRYPTO_CURVE25519_PUBLIC_KEY_BYTES);
    if (qapi_Crypto_Op_Key_Derive(op_hdl_1, attr, 1, derived_key_hdl_1) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to derive key\n");
        return -1;
    }

    ref_attr_init(attr, QAPI_CRYPTO_ATTR_CURVE25519_PUBLIC_VALUE_E, pub_key_1, QAPI_CRYPTO_CURVE25519_PUBLIC_KEY_BYTES);
    if (qapi_Crypto_Op_Key_Derive(op_hdl_2, attr, 1, derived_key_hdl_2) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to derive key\n");
        return -1;
    }

    if (qapi_Crypto_Obj_Buf_Attrib_Get(derived_key_hdl_1, QAPI_CRYPTO_ATTR_SECRET_VALUE_E, derived_key_1, QAPI_CRYPTO_CURVE25519_SHARED_SECRET_BYTES) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to get derived key val\n");
        return -1;
    }

    if (qapi_Crypto_Obj_Buf_Attrib_Get(derived_key_hdl_2, QAPI_CRYPTO_ATTR_SECRET_VALUE_E, derived_key_2, QAPI_CRYPTO_CURVE25519_SHARED_SECRET_BYTES) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to get derived key val\n");
        return -1;
    }

    if (memcmp(derived_key_1, derived_key_2, QAPI_CRYPTO_CURVE25519_SHARED_SECRET_BYTES) == 0) {
        CRYPTO_PRINTF("\nDerived keys match!\n");

    }
    else {
        CRYPTO_PRINTF("\nDerived keys DONOT match!\n");
    }

    qapi_Crypto_Transient_Obj_Free(curve_hdl_1);
    qapi_Crypto_Transient_Obj_Free(curve_hdl_2);
    qapi_Crypto_Transient_Obj_Free(derived_key_hdl_2);
    qapi_Crypto_Transient_Obj_Free(derived_key_hdl_1);
    qapi_Crypto_Op_Free(op_hdl_1);
    qapi_Crypto_Op_Free(op_hdl_2);

    return 0;
}

int32_t ed25519_demo()
{
#if ENABLE_ED25519_DEMO
    uint32_t klen;
    unsigned char skpk[crypto_sign_ed25519_SECRETKEYBYTES];
    qapi_Crypto_Obj_Hdl_t obj_hdl;
    qapi_Crypto_Op_Hdl_t sign_hdl, verify_hdl;
    qapi_Crypto_Attrib_t attr[2];
    unsigned char sm[crypto_sign_ed25519_BYTES];
    uint32_t smlen = 0;
    uint32_t i;

    int8_t temp_m[8];

    for (i = 0; i < (sizeof test_data) / (sizeof test_data[0]); i++) {

        memset(temp_m, 0, sizeof(temp_m));
        memcpy(temp_m, test_data[i].m, MIN(strlen(test_data[i].m), sizeof(temp_m)-1));

        if (qapi_Crypto_Transient_Obj_Alloc(QAPI_CRYPTO_OBJ_TYPE_ED25519_KEYPAIR_E, QAPI_CRYPTO_ED25519_PRIVATE_KEY_BITS, &obj_hdl) != QAPI_OK) {
            CRYPTO_PRINTF("\nFailed to alloc transient obj\n");
            continue;
        }

#if 0
        if (qapi_Crypto_Transient_Obj_Key_Gen(hdl, 64*8, NULL, 0) != QAPI_OK) {
            printf("\nFailed to gen key\n");
        }
#endif
        klen = i;
        memset(sm, 0, sizeof sm);
        memcpy(skpk, test_data[i].sk, crypto_sign_ed25519_SEEDBYTES);
        memcpy(skpk + crypto_sign_ed25519_SEEDBYTES, test_data[i].pk,
                crypto_sign_ed25519_PUBLICKEYBYTES);
        ref_attr_init(&attr[0], QAPI_CRYPTO_ATTR_ED25519_PRIVATE_VALUE_E, skpk, QAPI_CRYPTO_ED25519_PRIVATE_KEY_BYTES);
        ref_attr_init(&attr[1], QAPI_CRYPTO_ATTR_ED25519_PUBLIC_VALUE_E, test_data[i].pk, QAPI_CRYPTO_ED25519_PUBLIC_KEY_BYTES);

        if (qapi_Crypto_Transient_Obj_Populate(obj_hdl, attr, 2) != QAPI_OK) {
            CRYPTO_PRINTF("\nFailed to populate obj\n");
            continue;
        }

        if (qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_ED25519_E, QAPI_CRYPTO_MODE_SIGN_E, QAPI_CRYPTO_ED25519_PRIVATE_KEY_BITS, &sign_hdl) != QAPI_OK) {
            CRYPTO_PRINTF("\nFailed to alloc op\n");
            continue;
        }

        if (qapi_Crypto_Op_Key_Set(sign_hdl, obj_hdl) != QAPI_OK) {
            CRYPTO_PRINTF("\nFailed to set key\n");
            continue;
        }

        if (qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_ED25519_E, QAPI_CRYPTO_MODE_VERIFY_E, QAPI_CRYPTO_ED25519_PUBLIC_KEY_BITS, &verify_hdl) != QAPI_OK) {
            CRYPTO_PRINTF("\nFailed to alloc op\n");
            continue;
        }

        if (qapi_Crypto_Op_Key_Set(verify_hdl, obj_hdl) != QAPI_OK) {
            CRYPTO_PRINTF("\nFailed to set key\n");
            continue;
        }

        smlen = sizeof(sm);
        if (qapi_Crypto_Op_Sign_Digest(sign_hdl, NULL, 0, temp_m, klen, sm, &smlen) != QAPI_OK) {
            CRYPTO_PRINTF("\nFailed to sign\n");
            continue;
        }

        if (memcmp(test_data[i].sig, sm, crypto_sign_ed25519_BYTES) != 0) {
            CRYPTO_PRINTF("signature failure: [%u]\n", i);
            continue;
        }

        if (qapi_Crypto_Op_Verify_Digest(verify_hdl, NULL, 0, temp_m, klen, sm, smlen) != QAPI_OK) {
            CRYPTO_PRINTF("crypto_sign_open() failure: [%u]\n", i);
            continue;
        }

        CRYPTO_PRINTF("ed25519 Case [%u] pass\n", i);
        qapi_Crypto_Transient_Obj_Free(obj_hdl);
        qapi_Crypto_Op_Free(sign_hdl);
        qapi_Crypto_Op_Free(verify_hdl);

    }
#endif
    return 0;
}

int32_t rsa_demo()
{
    qapi_Crypto_Obj_Hdl_t obj_hdl;
    qapi_Crypto_Op_Hdl_t sign_hdl, verify_hdl;
    qapi_Crypto_Attrib_t attr[QAPI_CRYPTO_OBJ_ATTRIB_COUNT_RSA_KEYPAIR];
    uint32_t rsa_key_size = 1024;
    uint8_t sm[QAPI_CRYPTO_RSA_SIGNATURE_BUFFER_SIZE_BYTES(1024/8)];
    uint32_t smlen = 0;
#define RSA_DIGEST_LEN (256/8)
    uint8_t digest[RSA_DIGEST_LEN] = {0};

#if 1
    if (qapi_Crypto_Random_Get(digest, RSA_DIGEST_LEN) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to get rng\n");
        return -1;
    }
#endif

    if (qapi_Crypto_Transient_Obj_Alloc(QAPI_CRYPTO_OBJ_TYPE_RSA_KEYPAIR_E, rsa_key_size, 
                &obj_hdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc transient obj\n");
        return -1;
    }

    ref_attr_init(&attr[0], QAPI_CRYPTO_ATTR_RSA_MODULUS_E, rsa_mod, sizeof(rsa_mod));
    ref_attr_init(&attr[1], QAPI_CRYPTO_ATTR_RSA_PUBLIC_EXPONENT_E, rsa_pub_exp, sizeof(rsa_pub_exp));
    ref_attr_init(&attr[2], QAPI_CRYPTO_ATTR_RSA_PRIVATE_EXPONENT_E, rsa_pvt_exp, sizeof(rsa_pvt_exp));
    ref_attr_init(&attr[3], QAPI_CRYPTO_ATTR_RSA_PRIME1_E, rsa_prime1, sizeof(rsa_prime1));
    ref_attr_init(&attr[4], QAPI_CRYPTO_ATTR_RSA_PRIME2_E, rsa_prime2, sizeof(rsa_prime2));
    ref_attr_init(&attr[5], QAPI_CRYPTO_ATTR_RSA_EXPONENT1_E, rsa_exp1, sizeof(rsa_exp1));
    ref_attr_init(&attr[6], QAPI_CRYPTO_ATTR_RSA_EXPONENT2_E, rsa_exp2, sizeof(rsa_exp2));
    ref_attr_init(&attr[7], QAPI_CRYPTO_ATTR_RSA_COEFFICIENT_E, rsa_coeff, sizeof(rsa_coeff));

    if (qapi_Crypto_Transient_Obj_Populate(obj_hdl, attr, QAPI_CRYPTO_OBJ_ATTRIB_COUNT_RSA_KEYPAIR) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to populate rsa keypair obj\n");
        return -1;
    }
    /* Allocate sign operation */
    if (qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_RSASSA_PKCS1_V1_5_SHA256_E, QAPI_CRYPTO_MODE_SIGN_E, rsa_key_size, 
                &sign_hdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc op\n");
        return -1;
    }

    /* Copy private key from keypair object for sign operation */
    if (qapi_Crypto_Op_Key_Set(sign_hdl, obj_hdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to set pvt key\n");
        return -1;
    }

    /* Allocate verify operation */
    if (qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_RSASSA_PKCS1_V1_5_SHA256_E, QAPI_CRYPTO_MODE_VERIFY_E, rsa_key_size, &verify_hdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc verify op\n");
        return -1;
    }

    /* Copy public key from keypair object for verify operation */
    if (qapi_Crypto_Op_Key_Set(verify_hdl, obj_hdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to set public key\n");
        return -1;
    }

    /* Generate signature */
    smlen = sizeof(sm);
    if (qapi_Crypto_Op_Sign_Digest(sign_hdl, NULL, 0, digest, RSA_DIGEST_LEN, sm, &smlen) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to sign\n");
        return -1;
    }

    smlen = sizeof(sm);
    if (qapi_Crypto_Op_Sign_Digest(sign_hdl, NULL, 0, digest, RSA_DIGEST_LEN, sm, &smlen) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to sign second time\n");
        return -1;
    }

    /* Verify signature */
    if (qapi_Crypto_Op_Verify_Digest(verify_hdl, NULL, 0, digest, RSA_DIGEST_LEN, sm, smlen) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed verify\n");
        return -1;
    }

    /* Verify signature */
    if (qapi_Crypto_Op_Verify_Digest(verify_hdl, NULL, 0, digest, RSA_DIGEST_LEN, sm, smlen) != QAPI_OK) {
        CRYPTO_PRINTF("\nSecond verify failed \n");
        return -1;
    }

    CRYPTO_PRINTF("\nSuccessful\n");

    qapi_Crypto_Transient_Obj_Free(obj_hdl);
    qapi_Crypto_Op_Free(sign_hdl);
    qapi_Crypto_Op_Free(verify_hdl);

    return 0;
}

int32_t rsa_encrypt_demo()
{
	qapi_Status_t status = QAPI_OK;
	qapi_Crypto_Obj_Hdl_t obj_hdl;
	qapi_Crypto_Attrib_t attr[QAPI_CRYPTO_OBJ_ATTRIB_COUNT_RSA_KEYPAIR];
	uint8_t* p_cipher_text = NULL;
	uint32_t cipher_text_len = 0;
	uint8_t *p_decrypted_plain_text = 0;
	uint32_t decrypted_plain_text_len = 0;
	uint8_t plain_text[16] = {0xc6, 0xa1, 0x3b, 0x37, 0x87, 0x8f, 0x5b, 0x82, 0x6f, 0x4f, 0x81, 0x62, 0xa1, 0xc8, 0xd8, 0x79};
	uint32_t rsa_key_size = 1024;
	uint32_t plain_text_len = sizeof(plain_text);

	status = qapi_Crypto_Transient_Obj_Alloc(QAPI_CRYPTO_OBJ_TYPE_RSA_KEYPAIR_E,
			rsa_key_size,
			&obj_hdl);
	if (status != QAPI_OK) {
		CRYPTO_PRINTF("\nFailed to alloc transient obj\n");
		return QAPI_ERROR;
	}

	ref_attr_init(&attr[0], QAPI_CRYPTO_ATTR_RSA_MODULUS_E, rsa_mod, sizeof(rsa_mod));
	ref_attr_init(&attr[1], QAPI_CRYPTO_ATTR_RSA_PUBLIC_EXPONENT_E, rsa_pub_exp, sizeof(rsa_pub_exp));
	ref_attr_init(&attr[2], QAPI_CRYPTO_ATTR_RSA_PRIVATE_EXPONENT_E, rsa_pvt_exp, sizeof(rsa_pvt_exp));
	ref_attr_init(&attr[3], QAPI_CRYPTO_ATTR_RSA_PRIME1_E, rsa_prime1, sizeof(rsa_prime1));
	ref_attr_init(&attr[4], QAPI_CRYPTO_ATTR_RSA_PRIME2_E, rsa_prime2, sizeof(rsa_prime2));
	ref_attr_init(&attr[5], QAPI_CRYPTO_ATTR_RSA_EXPONENT1_E, rsa_exp1, sizeof(rsa_exp1));
	ref_attr_init(&attr[6], QAPI_CRYPTO_ATTR_RSA_EXPONENT2_E, rsa_exp2, sizeof(rsa_exp2));
	ref_attr_init(&attr[7], QAPI_CRYPTO_ATTR_RSA_COEFFICIENT_E, rsa_coeff, sizeof(rsa_coeff));

    if (qapi_Crypto_Transient_Obj_Populate(obj_hdl, attr, QAPI_CRYPTO_OBJ_ATTRIB_COUNT_RSA_KEYPAIR) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to populate rsa keypair obj\n");
        status = QAPI_ERROR;
        goto cleanup;
    }

	/* Encrypt */
	status = crypto_helper_asym_encrypt(
			obj_hdl,
			plain_text,
			plain_text_len,
			&p_cipher_text,
			&cipher_text_len,
			QAPI_CRYPTO_ALG_RSAES_PKCS1_V1_5_E);
	if(status != QAPI_OK) {
		goto cleanup;
	}

	/* Decrypt and compare to original */
	status = crypto_helper_asym_decrypt (
			obj_hdl,
			p_cipher_text,
			cipher_text_len,
			QAPI_CRYPTO_ALG_RSAES_PKCS1_V1_5_E,
			&p_decrypted_plain_text,
			&decrypted_plain_text_len);

	if ((plain_text_len != decrypted_plain_text_len) ||
		(memcmp(plain_text, p_decrypted_plain_text, plain_text_len) != 0))
	{
		status = QAPI_ERROR;
	}

cleanup:

	qapi_Crypto_Transient_Obj_Free(obj_hdl);

	if(p_cipher_text) {
		free(p_cipher_text);
	}

	if(p_decrypted_plain_text) {
		free(p_decrypted_plain_text);
	}

	if ( status == QAPI_OK )
	{
		CRYPTO_PRINTF("RSA encrypt and decrypt test successful\n");
	} else {
		CRYPTO_PRINTF("RSA encrypt and decrypt test failed (status=%d)\n", status);
	}

	return status;
}

int32_t crypto_aescbc(int32_t argc, char* argv[])
{
    //"wmiconfig --cryptotest aescbc 128/256"
    char srcData[16] = {0};
    char encData[16] = {0};
    char expectedEncData_128[16] = {0xc6, 0xa1, 0x3b, 0x37, 0x87, 0x8f, 0x5b, 0x82, 0x6f, 0x4f, 0x81, 0x62, 0xa1, 0xc8, 0xd8, 0x79};
    char expectedEncData_256[16] = {0x7c, 0x6c, 0x25, 0x8c, 0xcc, 0x6a, 0x40, 0x0e, 0xfa, 0xcc, 0x63, 0x14, 0x52, 0xa7, 0x5a, 0x25};
    char destData[16] = {0};
    qapi_Crypto_Attrib_t attr[2];
    uint32_t keyLen = atoi(argv[3]);
    uint32_t encLen, destLen, srcLen;

    srcLen = 16;
    memcpy(srcData, test_iv, srcLen);

    qapi_Crypto_Obj_Hdl_t aes_objHdl;
    qapi_Crypto_Op_Hdl_t aes_encHdl, aes_decHdl;

    //printf("srcData len %d %s\n", srcLen, srcData);
    if (qapi_Crypto_Transient_Obj_Alloc(QAPI_CRYPTO_OBJ_TYPE_AES_E, QAPI_CRYPTO_AES256_KEY_BITS, &aes_objHdl) != QAPI_OK) {
        CRYPTO_PRINTF("\n aes Failed to alloc transient obj\n");
        return -1;
    }
#if 0
    if (qapi_Crypto_Transient_Obj_Key_Gen(aes_objHdl, keyLen, NULL, 0) != QAPI_OK) {
        CRYPTO_PRINTF("\naes Failed to gen key\n");
    }
#endif
    if (keyLen == QAPI_CRYPTO_AES128_KEY_BITS){
        ref_attr_init(&attr[0], QAPI_CRYPTO_ATTR_SECRET_VALUE_E, test_iv, QAPI_CRYPTO_AES128_KEY_BYTES);
    }
    else if (keyLen == QAPI_CRYPTO_AES256_KEY_BITS){
        ref_attr_init(&attr[0], QAPI_CRYPTO_ATTR_SECRET_VALUE_E, test_iv, QAPI_CRYPTO_AES256_KEY_BYTES);
    }
    else {
        return -1;
    }

    if (qapi_Crypto_Transient_Obj_Populate(aes_objHdl, attr, 1) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to populate obj\n");
        return -1;
    }

    if (qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_AES_CBC_NOPAD_E, QAPI_CRYPTO_MODE_ENCRYPT_E, QAPI_CRYPTO_AES256_KEY_BITS, &aes_encHdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc op\n");
        return -1;
    }

    if (qapi_Crypto_Op_Key_Set(aes_encHdl, aes_objHdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to set key\n");
        return -1;
    }
    if (qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_AES_CBC_NOPAD_E, QAPI_CRYPTO_MODE_DECRYPT_E, QAPI_CRYPTO_AES256_KEY_BITS, &aes_decHdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc op\n");
        return -1;
    }

    if (qapi_Crypto_Op_Key_Set(aes_decHdl, aes_objHdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to set key\n");
        return -1;
    }

    if (qapi_Crypto_Op_Cipher_Init(aes_encHdl, (void*)test_iv, 16) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to cipher init\n");
        return -1;
    }
#if 0
    /* Enable after adding support for update */
    encLen = sizeof(encData);
    if (qapi_Crypto_Op_Cipher_Update(aes_encHdl, (void*)srcData, srcLen/2, encData, &encLen) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to cipher update\n");
        return -1;
    }
#endif

    encLen = sizeof(encData);
    if (qapi_Crypto_Op_Cipher_Final(aes_encHdl, (void*)srcData, srcLen, encData, &encLen) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to cipher doFinal\n");
        return -1;
    }

    char *expected;
    if (keyLen == QAPI_CRYPTO_AES128_KEY_BITS){
        expected = expectedEncData_128;
    }
    else {
        expected = expectedEncData_256;
    }
    if (memcmp(encData, expected, encLen) !=0 ) {
        CRYPTO_PRINTF("\nDoes not match expected encrypted data\n");
        return -1;
    }

#if 0
    printf("encrypt encData len %d :\n", encLen);
    int i;
    for (i=0; i<encLen; i++)
        printf("%02x, ", encData[i]);
    printf("\n ");
#endif
    if (qapi_Crypto_Op_Cipher_Init(aes_decHdl, (void*)test_iv, 16) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to cipher init\n");
        return -1;
    }

#if 0
    /* Enable after adding support for update */
    if (qapi_Crypto_Op_Cipher_Update(aes_decHdl, (void*)encData, encLen, destData, &destLen) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to cipher update\n");
        return -1;
    }
#endif

    destLen = sizeof(destData);
    if (qapi_Crypto_Op_Cipher_Final(aes_decHdl, (void*)encData, encLen, destData, &destLen) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to cipher doFinal\n");
        return -1;
    }
    //printf("Decrypt destData len %d %s\n", destLen, destData);
    if (memcmp(destData, srcData, destLen)==0){
        CRYPTO_PRINTF("aescbc encryption and decryption success\n");
    }
    else {
        CRYPTO_PRINTF("aescbc encryption and decryption failed\n");
    }

    qapi_Crypto_Transient_Obj_Free(aes_objHdl);
    qapi_Crypto_Op_Free(aes_encHdl);
    qapi_Crypto_Op_Free(aes_decHdl);
    return 0;
}

int32_t crypto_aesccm()
{
    //"wmiconfig --cryptotest aesccm"
    qapi_Crypto_Attrib_t attr[2];
#if 1
    const uint8_t *key = (uint8_t[]){0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f};
    uint32_t keyLen = 128;
    const uint8_t *nonce = (uint8_t[]){0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b};
    uint32_t nonceLen = 12;
    const uint8_t *adata = (uint8_t[]){0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13};
    uint32_t adataLen = 20;
    uint8_t tag[16] = {0};
    uint32_t tagLen = 0;
    const uint8_t *srcData = (uint8_t[]){0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37};
    uint32_t srcLen = 24;
    uint8_t destData[128] = {0};
    uint32_t destLen = 0;
    uint8_t decData[128] = {0};
    uint32_t decLen = 0;

    uint8_t goldTag[] = {0x48, 0x43, 0x92, 0xfb, 0xc1, 0xb0, 0x99, 0x51};
    uint32_t goldTagLen = 64;
#else
    uint8_t key[] = {0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f};
    uint32_t keyLen = 128;
    uint8_t nonce[] = {0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17};
    uint32_t nonceLen = 8;
    uint8_t adata[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
    uint32_t adataLen = 16;
    uint8_t tag[16] = {};
    uint32_t tagLen = 0;
    uint8_t srcData[] = {0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f};
    uint32_t srcLen = 16;
    uint8_t destData[256] = {0};
    uint32_t destLen = 0;
    uint8_t decData[256] = {0};
    uint32_t decLen = 0;

    uint8_t goldTag[] = {0x1f, 0xc6, 0x4f, 0xbf,0xac, 0xcd};
    uint32_t goldTagLen = 48;
#endif
    qapi_Crypto_Obj_Hdl_t aes_objHdl;
    qapi_Crypto_Op_Hdl_t aes_encHdl, aes_decHdl;

    //printf("srcData len %d %s\n", srcLen, srcData);
    if (qapi_Crypto_Transient_Obj_Alloc(QAPI_CRYPTO_OBJ_TYPE_AES_E, QAPI_CRYPTO_AES256_KEY_BITS, &aes_objHdl) != QAPI_OK) {
        CRYPTO_PRINTF("\n aes Failed to alloc transient obj\n");
        return -1;
    }
#if 0
    if (qapi_Crypto_Transient_Obj_Key_Gen(aes_objHdl, keyLen, NULL, 0) != QAPI_OK) {
        CRYPTO_PRINTF("\naes Failed to gen key\n");
    }
#endif
    if (keyLen == QAPI_CRYPTO_AES128_KEY_BITS){
        ref_attr_init(&attr[0], QAPI_CRYPTO_ATTR_SECRET_VALUE_E, (void*)key, QAPI_CRYPTO_AES128_KEY_BYTES);
    }
    else if (keyLen == QAPI_CRYPTO_AES256_KEY_BITS){
        ref_attr_init(&attr[0], QAPI_CRYPTO_ATTR_SECRET_VALUE_E, test_iv, QAPI_CRYPTO_AES256_KEY_BYTES);
    }
    else {
        return -1;
    }

    if (qapi_Crypto_Transient_Obj_Populate(aes_objHdl, attr, 1) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to populate obj\n");
        return -1;
    }

    if (qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_AES_CCM_E, QAPI_CRYPTO_MODE_ENCRYPT_E, QAPI_CRYPTO_AES256_KEY_BITS, &aes_encHdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc op\n");
        return -1;
    }

    if (qapi_Crypto_Op_Key_Set(aes_encHdl, aes_objHdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to set key\n");
        return -1;
    }

    if (qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_AES_CCM_E, QAPI_CRYPTO_MODE_DECRYPT_E, QAPI_CRYPTO_AES256_KEY_BITS, &aes_decHdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc op\n");
        return -1;
    }

    if (qapi_Crypto_Op_Key_Set(aes_decHdl, aes_objHdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to set key\n");
        return -1;
    }

    if (qapi_Crypto_Op_AE_Init(aes_encHdl, (void*)nonce, nonceLen, goldTagLen, adataLen, srcLen) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to ae init\n");
        return -1;
    }

    if (qapi_Crypto_Op_AE_AAD_Update(aes_encHdl, (void*)adata, adataLen) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to ae update aad\n");
        return -1;
    }

    destLen = sizeof(destData);
    tagLen = 8 * sizeof(tag);
    if (qapi_Crypto_Op_AE_Encrypt_Final(aes_encHdl, (void*)srcData, srcLen, destData, &destLen, tag, &tagLen) != QAPI_OK){
        CRYPTO_PRINTF("\nFailed to ae encrypto final\n");
        return -1;
    }

#if 0
    printf("encrypt destData len %d tagLen %d:\n", destLen, tagLen);
    int i;
    for (i=0; i<destLen; i++)
        printf("%02x, ", destData[i]);
    printf("\n ");

    for (i=0; i<tagLen/8; i++)
        printf("%02x, ", tag[i]);
    printf("\n ");
#endif

    if (qapi_Crypto_Op_AE_Init(aes_decHdl, (void*)nonce, nonceLen, goldTagLen, adataLen, srcLen) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to ae init\n");
        return -1;
    }

    if (qapi_Crypto_Op_AE_AAD_Update(aes_decHdl, (void*)adata, adataLen) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to ae update aad\n");
        return -1;
    }

    decLen = sizeof(decData);
    if (qapi_Crypto_Op_AE_Decrypt_Final(aes_decHdl, destData, destLen, decData, &decLen, goldTag, goldTagLen) != QAPI_OK){
        CRYPTO_PRINTF("\nFailed to ae decrypto final\n");
        return -1;
    }
    else{
#if 0
        printf("decrypt destData len %d\n", decLen);
        for (i=0; i<decLen; i++)
            printf("%02x, ", decData[i]);
        printf("\n ");
#endif
        if ((decLen==srcLen) && (memcmp(decData, srcData, decLen)==0))
            CRYPTO_PRINTF("aesccm encryption and decryption success\n");
        else
            CRYPTO_PRINTF("aesccm encryption and decryption fail\n");
    }

    qapi_Crypto_Transient_Obj_Free(aes_objHdl);
    qapi_Crypto_Op_Free(aes_encHdl);
    qapi_Crypto_Op_Free(aes_decHdl);
    return 0;
}


int32_t crypto_aesccm_enc_dec_test(
    const uint8_t * key,
    uint32_t keyLen,
    const uint8_t * nonce,
    uint32_t nonceLen,
    const uint8_t * adata,
    uint32_t adataLen,
    const uint8_t * srcData,
    uint32_t srcLen,
    const uint8_t * goldEncryptedData,
    const uint32_t goldEncryptedDataLen,
    const uint8_t * goldTag,
    const uint32_t goldTagLen
    )
{
    qapi_Crypto_Attrib_t attr[2];

    uint8_t tag[16] = {0};
    uint32_t tagLen = 0;
    uint8_t destData[128] = {0};
    uint32_t destLen = 0;
    uint8_t decData[128] = {0};
    uint32_t decLen = 0;

    uint32_t keyLenInBits = keyLen * 8;
    uint32_t goldTagLenInBits = goldTagLen * 8;


    qapi_Crypto_Obj_Hdl_t aes_objHdl;
    qapi_Crypto_Op_Hdl_t aes_encHdl, aes_decHdl;

    if (qapi_Crypto_Transient_Obj_Alloc(QAPI_CRYPTO_OBJ_TYPE_AES_E, QAPI_CRYPTO_AES256_KEY_BITS, &aes_objHdl) != QAPI_OK) {
        CRYPTO_PRINTF("\n aes Failed to alloc transient obj\n");
        return -1;
    }

    if (keyLenInBits == QAPI_CRYPTO_AES128_KEY_BITS){
        ref_attr_init(&attr[0], QAPI_CRYPTO_ATTR_SECRET_VALUE_E, (void*)key, QAPI_CRYPTO_AES128_KEY_BYTES);
    }
    else if (keyLenInBits == QAPI_CRYPTO_AES256_KEY_BITS){
        ref_attr_init(&attr[0], QAPI_CRYPTO_ATTR_SECRET_VALUE_E, test_iv, QAPI_CRYPTO_AES256_KEY_BYTES);
    }
    else {
        return -1;
    }

    if (qapi_Crypto_Transient_Obj_Populate(aes_objHdl, attr, 1) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to populate obj\n");
        return -1;
    }

    if (qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_AES_CCM_E, QAPI_CRYPTO_MODE_ENCRYPT_E, QAPI_CRYPTO_AES256_KEY_BITS, &aes_encHdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc op\n");
        return -1;
    }

    if (qapi_Crypto_Op_Key_Set(aes_encHdl, aes_objHdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to set key\n");
        return -1;
    }

    if (qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_AES_CCM_E, QAPI_CRYPTO_MODE_DECRYPT_E, QAPI_CRYPTO_AES256_KEY_BITS, &aes_decHdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc op\n");
        return -1;
    }

    if (qapi_Crypto_Op_Key_Set(aes_decHdl, aes_objHdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to set key\n");
        return -1;
    }

    if (qapi_Crypto_Op_AE_Init(aes_encHdl, (void*)nonce, nonceLen, goldTagLenInBits, adataLen, srcLen) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to ae init\n");
        return -1;
    }

    if (qapi_Crypto_Op_AE_AAD_Update(aes_encHdl, (void*)adata, adataLen) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to ae update aad\n");
        return -1;
    }

    destLen = sizeof(destData);
    tagLen = sizeof(tag) * 8;
    if (qapi_Crypto_Op_AE_Encrypt_Final(aes_encHdl, (void*)srcData, srcLen, destData, &destLen, tag, &tagLen) != QAPI_OK){
        CRYPTO_PRINTF("\nFailed to ae encrypto final\n");
        return -1;
    }


    if ( goldTagLenInBits != tagLen ) {
        CRYPTO_PRINTF("\nOn return from qapi_Crypto_Op_AE_Encrypt_Final: incorrect tagLen, expected=%d, got=%d\r\n", goldTagLenInBits, tagLen);
        return -1;
    }
    int status = memcmp(goldTag, tag, goldTagLen);
    if ( 0 != status ) {
        CRYPTO_PRINTF("\nOn return from qapi_Crypto_Op_AE_Encrypt_Final: incorrect tag\r\n");
        return -1;
    }

    if ( goldEncryptedDataLen != destLen ) {
        CRYPTO_PRINTF("\nOn return from qapi_Crypto_Op_AE_Encrypt_Final: incorrect ecnrypted data length, expected=%d, got=%d\r\n", goldEncryptedDataLen, destLen);
        return -1;
    }
    status = memcmp(goldEncryptedData, destData, goldEncryptedDataLen);
    if ( 0 != status ) {
        CRYPTO_PRINTF("\nOn return from qapi_Crypto_Op_AE_Encrypt_Final: incorrect encrypted data\r\n");
        return -1;
    }


    if (qapi_Crypto_Op_AE_Init(aes_decHdl, (void*)nonce, nonceLen, goldTagLenInBits, adataLen, srcLen) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to ae init\n");
        return -1;
    }

    if (qapi_Crypto_Op_AE_AAD_Update(aes_decHdl, (void*)adata, adataLen) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to ae update aad\n");
        return -1;
    }

    decLen = sizeof(decData);
    if (qapi_Crypto_Op_AE_Decrypt_Final(aes_decHdl, destData, destLen, decData, &decLen, (void*) goldTag, goldTagLenInBits) != QAPI_OK){
        CRYPTO_PRINTF("\nFailed to ae decrypto final\n");
        return -1;
    }
    else{
        if ((decLen==srcLen) && (memcmp(decData, srcData, decLen)==0))
            CRYPTO_PRINTF("aesccm encryption and decryption success\n");
        else
            CRYPTO_PRINTF("aesccm decryption fail\n");
    }

    qapi_Crypto_Transient_Obj_Free(aes_objHdl);
    qapi_Crypto_Op_Free(aes_encHdl);
    qapi_Crypto_Op_Free(aes_decHdl);
    return 0;
}


int crypto_aesccm_test_vector1_from_rfc3610()
{
    uint8_t vector_1_key[16] =
    {
        0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF
    };
    uint8_t vector_1_nonce[13] =
    {
        0x00, 0x00, 0x00, 0x03, 0x02, 0x01, 0x00, 0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5
    };
    uint8_t vector_1_plaintext_data[23] =
    {
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E
    };
    uint8_t vector_1_aad_data[8] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07
    };
    uint8_t vector_1_expected_ciphertext_data[23] =
    {
        0x58, 0x8C, 0x97, 0x9A, 0x61, 0xC6, 0x63, 0xD2, 0xF0, 0x66, 0xD0, 0xC2, 0xC0, 0xF9, 0x89, 0x80,
        0x6D, 0x5F, 0x6B, 0x61, 0xDA, 0xC3, 0x84
    };
    uint8_t vector_1_expected_mac[8] =
    {
        0x17, 0xE8, 0xD1, 0x2C, 0xFD, 0xF9, 0x26, 0xE0
    };

    int status =
        crypto_aesccm_enc_dec_test(
            vector_1_key,
            sizeof(vector_1_key),
            vector_1_nonce,
            sizeof(vector_1_nonce),
            vector_1_aad_data,
            sizeof(vector_1_aad_data),
            vector_1_plaintext_data,
            sizeof(vector_1_plaintext_data),
            vector_1_expected_ciphertext_data,
            sizeof(vector_1_expected_ciphertext_data),
            vector_1_expected_mac,
            sizeof(vector_1_expected_mac)
            );
    return status;
}


int crypto_aesccm_test_vector23_from_rfc3610()
{
    uint8_t vector_23_key[16] =
    {
        0xD7, 0x82, 0x8D, 0x13, 0xB2, 0xB0, 0xBD, 0xC3, 0x25, 0xA7, 0x62, 0x36, 0xDF, 0x93, 0xCC, 0x6B
    };
    uint8_t vector_23_nonce[13] =
    {
        0x00, 0x3E, 0xBE, 0x94, 0x04, 0x4B, 0x9A, 0x3C, 0x96, 0x96, 0x76, 0x6C, 0xFA
    };
    uint8_t vector_23_plaintext_data[20] =
    {
        0xE2, 0xFC, 0xFB, 0xB8, 0x80, 0x44, 0x2C, 0x73, 0x1B, 0xF9, 0x51, 0x67, 0xC8, 0xFF, 0xD7, 0x89, 0x5E, 0x33, 0x70, 0x76
    };
    uint8_t vector_23_aad_data[12] = {
        0x47, 0xA6, 0x5A, 0xC7, 0x8B, 0x3D, 0x59, 0x42, 0x27, 0xE8, 0x5E, 0x71
    };
    uint8_t vector_23_expected_ciphertext_data[20] =
    {
        0xE8, 0x82, 0xF1, 0xDB, 0xD3, 0x8C, 0xE3, 0xED, 0xA7, 0xC2, 0x3F, 0x04, 0xDD, 0x65, 0x07, 0x1E, 0xB4, 0x13, 0x42, 0xAC
    };
    uint8_t vector_23_expected_mac[10] =
    {
        0xDF, 0x7E, 0x00, 0xDC, 0xCE, 0xC7, 0xAE, 0x52, 0x98, 0x7D
    };

    int status =
        crypto_aesccm_enc_dec_test(
            vector_23_key,
            sizeof(vector_23_key),
            vector_23_nonce,
            sizeof(vector_23_nonce),
            vector_23_aad_data,
            sizeof(vector_23_aad_data),
            vector_23_plaintext_data,
            sizeof(vector_23_plaintext_data),
            vector_23_expected_ciphertext_data,
            sizeof(vector_23_expected_ciphertext_data),
            vector_23_expected_mac,
            sizeof(vector_23_expected_mac)
            );
    return status;
}


int crypto_aesccm_test_vector24_from_rfc3610()
{
    uint8_t vector_24_key[16] =
    {
        0xD7, 0x82, 0x8D, 0x13, 0xB2, 0xB0, 0xBD, 0xC3, 0x25, 0xA7, 0x62, 0x36, 0xDF, 0x93, 0xCC, 0x6B
    };
    uint8_t vector_24_nonce[13] =
    {
        0x00, 0x8D, 0x49, 0x3B, 0x30, 0xAE, 0x8B, 0x3C, 0x96, 0x96, 0x76, 0x6C, 0xFA
    };
    uint8_t vector_24_plaintext_data[21] =
    {
        0xAB, 0xF2, 0x1C, 0x0B, 0x02, 0xFE, 0xB8, 0x8F, 0x85, 0x6D, 0xF4, 0xA3, 0x73, 0x81, 0xBC, 0xE3,
        0xCC, 0x12, 0x85, 0x17, 0xD4
    };
    uint8_t vector_24_aad_data[12] = {
        0x6E, 0x37, 0xA6, 0xEF, 0x54, 0x6D, 0x95, 0x5D, 0x34, 0xAB, 0x60, 0x59
    };
    uint8_t vector_24_expected_ciphertext_data[21] =
    {
        0xF3, 0x29, 0x05, 0xB8, 0x8A, 0x64, 0x1B, 0x04, 0xB9, 0xC9, 0xFF, 0xB5, 0x8C, 0xC3, 0x90, 0x90,
        0x0F, 0x3D, 0xA1, 0x2A, 0xB1
    };
    uint8_t vector_24_expected_mac[10] =
    {
        0x6D, 0xCE, 0x9E, 0x82, 0xEF, 0xA1, 0x6D, 0xA6, 0x20, 0x59
    };

    int status =
        crypto_aesccm_enc_dec_test(
            vector_24_key,
            sizeof(vector_24_key),
            vector_24_nonce,
            sizeof(vector_24_nonce),
            vector_24_aad_data,
            sizeof(vector_24_aad_data),
            vector_24_plaintext_data,
            sizeof(vector_24_plaintext_data),
            vector_24_expected_ciphertext_data,
            sizeof(vector_24_expected_ciphertext_data),
            vector_24_expected_mac,
            sizeof(vector_24_expected_mac)
            );
    return status;
}


int32_t crypto_aesccm_unittests()
{
    int status;

    status = crypto_aesccm_test_vector1_from_rfc3610();
    if ( 0 != status ) {
        QCLI_Printf(qcli_crypto_handle, "Failed on a call to crypto_aesccm_test_vector1_from_rfc3610(), status=%d\r\n", status);
        return -1;
    }

    status = crypto_aesccm_test_vector23_from_rfc3610();
    if ( 0 != status ) {
        QCLI_Printf(qcli_crypto_handle, "Failed on a call to crypto_aesccm_test_vector23_from_rfc3610(), status=%d\r\n", status);
        return -1;
    }

    status = crypto_aesccm_test_vector24_from_rfc3610();
    if ( 0 != status ) {
        QCLI_Printf(qcli_crypto_handle, "Failed on a call to crypto_aesccm_test_vector24_from_rfc3610(), status=%d\r\n", status);
        return -1;
    }

    return 0;
}


int32_t crypto_aesgcm(int32_t argc, char* argv[])
{
    //"wmiconfig --cryptotest aesgcm  <keyLen>  <xxx> "  keyLen: 128/256,  xxx:test data
    char srcData[128] = {0};
    char originalData[128] = {0};
    qapi_Crypto_Attrib_t attr[2];
    uint32_t nonceLen = QAPI_CRYPTO_AE_AES_GCM_NONCEBYTES; //fixed in SharkSSL.
    uint32_t adataLen=16;
    uint8_t tag[16] = {0};

    uint32_t keyLen = atoi(argv[3]);
    uint32_t tagLen = 128;
    uint32_t srcLen = strlen(argv[4]);
    uint32_t destLen = 0;

    if (srcLen>128){
        CRYPTO_PRINTF("\n The max length of data is 128\n");
        return -1;
    }

    memcpy(srcData, argv[4], srcLen);
    memcpy(originalData, argv[4], srcLen);

    qapi_Crypto_Obj_Hdl_t aes_objHdl;
    qapi_Crypto_Op_Hdl_t aes_encHdl, aes_decHdl;

    //printf("srcData len %d %s\n", srcLen, srcData);
    if (qapi_Crypto_Transient_Obj_Alloc(QAPI_CRYPTO_OBJ_TYPE_AES_E, QAPI_CRYPTO_AES256_KEY_BITS, &aes_objHdl) != QAPI_OK) {
        CRYPTO_PRINTF("\n aes Failed to alloc transient obj\n");
        return -1;
    }
#if 0
    if (qapi_Crypto_Transient_Obj_Key_Gen(aes_objHdl, keyLen, NULL, 0) != QAPI_OK) {
        CRYPTO_PRINTF("\naes Failed to gen key\n");
    }
#endif
    if (keyLen == QAPI_CRYPTO_AES128_KEY_BITS){
        ref_attr_init(&attr[0], QAPI_CRYPTO_ATTR_SECRET_VALUE_E, test_iv, QAPI_CRYPTO_AES128_KEY_BYTES);
    }
    else if (keyLen == QAPI_CRYPTO_AES256_KEY_BITS){
        ref_attr_init(&attr[0], QAPI_CRYPTO_ATTR_SECRET_VALUE_E, test_iv, QAPI_CRYPTO_AES256_KEY_BYTES);
    }
    else {
         CRYPTO_PRINTF("\nIncorrect key length entered. Permitted values are 128 or 256\n");
         return -1;
    }

    if (qapi_Crypto_Transient_Obj_Populate(aes_objHdl, attr, 1) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to populate obj\n");
        return -1;
    }

    if (qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_AES_GCM_E, QAPI_CRYPTO_MODE_ENCRYPT_E, QAPI_CRYPTO_AES256_KEY_BITS, &aes_encHdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc op\n");
        return -1;
    }

    if (qapi_Crypto_Op_Key_Set(aes_encHdl, aes_objHdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to set key\n");
        return -1;
    }

    if (qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_AES_GCM_E, QAPI_CRYPTO_MODE_DECRYPT_E, QAPI_CRYPTO_AES256_KEY_BITS, &aes_decHdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc op\n");
        return -1;
    }

    if (qapi_Crypto_Op_Key_Set(aes_decHdl, aes_objHdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to set key\n");
        return -1;
    }

    if (qapi_Crypto_Op_AE_Init(aes_encHdl, (void*)test_iv, nonceLen, 128, adataLen, srcLen) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to ae init\n");
        return -1;
    }

    if (qapi_Crypto_Op_AE_AAD_Update(aes_encHdl, (void*)test_iv, adataLen) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to ae update aad\n");
        return -1;
    }

    if (qapi_Crypto_Op_AE_Encrypt_Final(aes_encHdl, srcData, srcLen, srcData, &destLen, tag, &tagLen) != QAPI_OK){
        CRYPTO_PRINTF("\nFailed to ae encrypto final\n");
        return -1;
    }

#if 0
    printf("encrypt srcData len %d tagLen %d:\n", destLen, tagLen);
    int i;
    for (i=0; i<destLen; i++)
        printf("%02x, ", srcData[i]);
    printf("\n ");

    for (i=0; i<tagLen/8; i++)
        printf("%02x, ", tag[i]);
    printf("\n ");
#endif
    if (qapi_Crypto_Op_AE_Init(aes_decHdl, (void*)test_iv, nonceLen, tagLen, adataLen, srcLen) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to ae init\n");
        return -1;
    }

    if (qapi_Crypto_Op_AE_AAD_Update(aes_decHdl, (void*)test_iv, adataLen) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to ae update aad\n");
        return -1;
    }

    if (qapi_Crypto_Op_AE_Decrypt_Final(aes_decHdl, srcData, srcLen, srcData, &destLen, tag, tagLen) != QAPI_OK){
        CRYPTO_PRINTF("\nFailed to ae decrypto final\n");
        return -1;
    }
    else{
        //printf("Decrypt srcData len %d %s\n", destLen, srcData);
        if (memcmp(originalData, srcData, destLen)==0){
            CRYPTO_PRINTF("aesgcm encryption and decryption success\n");
        }
    }

    qapi_Crypto_Transient_Obj_Free(aes_objHdl);
    qapi_Crypto_Op_Free(aes_encHdl);
    qapi_Crypto_Op_Free(aes_decHdl);
    return 0;
}

#endif

#if defined (CONFIG_CRYPTO_UNIT_TEST_DEMO) || defined (CONFIG_CRYPTO_BASE64_DEMO)
unsigned int get_pseudo_random_number(unsigned int lower_bound, unsigned int upper_bound)
{
    if ( upper_bound < lower_bound ) {
        CRYPTO_PRINTF("\nget_pseudo_random_number: upper bound must be at least equal to lower bound\n");
        return lower_bound;
    }
    if ( upper_bound == lower_bound ) {
        return lower_bound;
    }
    unsigned int temp_number;
    if (qapi_Crypto_Random_Get((void *) &temp_number, sizeof(temp_number)) != QAPI_OK) {
        CRYPTO_PRINTF("\nget_pseudo_random_number: failed on a call to qapi_Crypto_Random_Get\n");
        return lower_bound;
    }
    return ( (temp_number % (upper_bound-lower_bound)) + lower_bound );
}
#endif

#if defined (CONFIG_CRYPTO_UNIT_TEST_DEMO)
#define QAPI_CRYPTO_CHACHA20_POLY1305_ENABLE_RANDOM_SIZE_UPDATES 1

#if QAPI_CRYPTO_CHACHA20_POLY1305_ENABLE_RANDOM_SIZE_UPDATES

    typedef struct _chunk_info {
        unsigned int start_index;
        unsigned int length;
    } chunk_info_t;

    void print_chunks_info(chunk_info_t * chunks, unsigned int NUM_CHUNKS)
    {
        int chunk_index;
        CRYPTO_PRINTF("Chunks: ");
        for ( chunk_index = 0; chunk_index < NUM_CHUNKS; chunk_index++ ) {
            CRYPTO_PRINTF("(%d, %d) ", chunks[chunk_index].start_index, chunks[chunk_index].length);
        }
        CRYPTO_PRINTF("\n");
    }

    void randomize_chunks(chunk_info_t * p_chunks, unsigned int chunks_count, unsigned int total_length)
    {
        if ( chunks_count == 0 ) {
            CRYPTO_PRINTF("\nrandomize_chunks(): chunks_count must be greater than 0\n");
            return;
        }
        int i;
        unsigned int start_index = 0;
        unsigned int remaining_length = total_length;
        for ( i = 0; i < chunks_count-1; i++ ) {
            unsigned int chunk_length = get_pseudo_random_number(0, remaining_length);
            p_chunks[i].start_index = start_index;
            p_chunks[i].length = chunk_length;
            remaining_length -= chunk_length;
            start_index += chunk_length;
        }
        p_chunks[chunks_count-1].start_index = start_index;
        p_chunks[chunks_count-1].length = remaining_length;
        start_index += remaining_length;
        if ( total_length != start_index ) {
            CRYPTO_PRINTF("\nrandomize_chunks(): internal error\n");
        }
    }
#else
    #define print_chunks_info(a, b)
#endif


#define A_COMPILE_TIME_ASSERT(assertion_name, predicate) 


int32_t crypto_chacha20_poly1305(int32_t argc, char* argv[])
{
    static unsigned char key[] = { 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f, 0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f };
    A_COMPILE_TIME_ASSERT(_key_length_check, (sizeof(key) == key_length));

    const unsigned int nonce_length = 12;
    static unsigned char nonce[] = { 0x07, 0x00, 0x00, 0x00, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47 };
    A_COMPILE_TIME_ASSERT(_nonce_length_check, (sizeof(nonce) == nonce_length));

    const unsigned int aad_length = 12;
    static unsigned char aad[] = { 0x50, 0x51, 0x52, 0x53, 0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7 };
    A_COMPILE_TIME_ASSERT(_aad_length_check, (sizeof(aad) == aad_length));

#define PLAINTEXT_LENGTH 114
    static unsigned char plaintext[] = "Ladies and Gentlemen of the class of '99: If I could offer you only one tip for the future, sunscreen would be it.";
    A_COMPILE_TIME_ASSERT(_PLAINTEXT_LENGTH_check, (sizeof(plaintext) == PLAINTEXT_LENGTH+1));

#define EXPECTED_CIPHERTEXT_LENGTH 114
    static unsigned char expected_ciphertext[] = { 0xd3, 0x1a, 0x8d, 0x34, 0x64, 0x8e, 0x60, 0xdb, 0x7b, 0x86, 0xaf, 0xbc, 0x53, 0xef, 0x7e, 0xc2, 0xa4, 0xad, 0xed, 0x51, 0x29, 0x6e, 0x08, 0xfe, 0xa9, 0xe2, 0xb5, 0xa7, 0x36, 0xee, 0x62, 0xd6, 0x3d, 0xbe, 0xa4, 0x5e, 0x8c, 0xa9, 0x67, 0x12, 0x82, 0xfa, 0xfb, 0x69, 0xda, 0x92, 0x72, 0x8b, 0x1a, 0x71, 0xde, 0x0a, 0x9e, 0x06, 0x0b, 0x29, 0x05, 0xd6, 0xa5, 0xb6, 0x7e, 0xcd, 0x3b, 0x36, 0x92, 0xdd, 0xbd, 0x7f, 0x2d, 0x77, 0x8b, 0x8c, 0x98, 0x03, 0xae, 0xe3, 0x28, 0x09, 0x1b, 0x58, 0xfa, 0xb3, 0x24, 0xe4, 0xfa, 0xd6, 0x75, 0x94, 0x55, 0x85, 0x80, 0x8b, 0x48, 0x31, 0xd7, 0xbc, 0x3f, 0xf4, 0xde, 0xf0, 0x8e, 0x4b, 0x7a, 0x9d, 0xe5, 0x76, 0xd2, 0x65, 0x86, 0xce, 0xc6, 0x4b, 0x61, 0x16 };
    A_COMPILE_TIME_ASSERT(_EXPECTED_CIPHERTEXT_LENGTH_check, (sizeof(expected_ciphertext) == EXPECTED_CIPHERTEXT_LENGTH));

#define EXPECTED_TAG_LENGTH 16
    static unsigned char expected_tag[] = { 0x1a, 0xe1, 0x0b, 0x59, 0x4f, 0x09, 0xe2, 0x6a, 0x7e, 0x90, 0x2e, 0xcb, 0xd0, 0x60, 0x06, 0x91 };
    A_COMPILE_TIME_ASSERT(_EXPECTED_TAG_LENGTH_check, (sizeof(expected_tag) == EXPECTED_TAG_LENGTH));

    unsigned char ciphertext[EXPECTED_CIPHERTEXT_LENGTH];
    unsigned int ciphertext_length = EXPECTED_CIPHERTEXT_LENGTH;


    unsigned char tag[EXPECTED_TAG_LENGTH];
    uint32_t tag_length = EXPECTED_TAG_LENGTH;

    qapi_Crypto_Attrib_t attr[2];
    qapi_Crypto_Obj_Hdl_t aes_objHdl;
    qapi_Crypto_Op_Hdl_t aes_encHdl, aes_decHdl;

    //printf("srcData len %d %s\n", srcLen, srcData);
    if (qapi_Crypto_Transient_Obj_Alloc(QAPI_CRYPTO_OBJ_TYPE_CHACHA20_E, QAPI_CRYPTO_CHACHA20_POLY1305_KEY_BITS, &aes_objHdl) != QAPI_OK) {
    //if (qapi_Crypto_Transient_Obj_Alloc(QAPI_CRYPTO_OBJ_TYPE_GENERIC_SECRET_E, QAPI_CRYPTO_CHACHA20_POLY1305_KEY_BITS, &aes_objHdl) != QAPI_OK) {
        CRYPTO_PRINTF("\n chacha20_poly1305 Failed to alloc transient obj\n");
        return -1;
    }

    ref_attr_init(&attr[0], QAPI_CRYPTO_ATTR_SECRET_VALUE_E, key, QAPI_CRYPTO_CHACHA20_POLY1305_KEY_BYTES);

    if (qapi_Crypto_Transient_Obj_Populate(aes_objHdl, attr, 1) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to populate obj\n");
        return -1;
    }

    if (qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_CHACHA20_POLY1305_E, QAPI_CRYPTO_MODE_ENCRYPT_E, QAPI_CRYPTO_CHACHA20_POLY1305_KEY_BITS, &aes_encHdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc op\n");
        return -1;
    }

    if (qapi_Crypto_Op_Key_Set(aes_encHdl, aes_objHdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to set key\n");
        return -1;
    }


    if (qapi_Crypto_Op_AE_Init(aes_encHdl, (void*)nonce, nonce_length, tag_length, aad_length, PLAINTEXT_LENGTH) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to ae init\n");
        return -1;
    }

    int status_code;

#if QAPI_CRYPTO_CHACHA20_POLY1305_ENABLE_RANDOM_SIZE_UPDATES
#define NUM_AAD_CHUNKS 4
    chunk_info_t aad_chunks[NUM_AAD_CHUNKS];
    randomize_chunks(aad_chunks, NUM_AAD_CHUNKS, aad_length);
    int aad_chunk_index;
    for ( aad_chunk_index = 0; aad_chunk_index < NUM_AAD_CHUNKS; aad_chunk_index++ ) {
        status_code =
            qapi_Crypto_Op_AE_AAD_Update(
                aes_encHdl,
                (void*) &aad[aad_chunks[aad_chunk_index].start_index],
                aad_chunks[aad_chunk_index].length
                );

        if ( status_code != QAPI_OK ) {
            CRYPTO_PRINTF("\ncrypto_poly1305: failed on aad_update()\n");
            print_chunks_info(aad_chunks, NUM_AAD_CHUNKS);
            return -1;
        }
    }

#else
    status_code = qapi_Crypto_Op_AE_AAD_Update(aes_encHdl, (void*)aad, aad_length);
#endif

    if ( status_code != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to ae update aad\n");
        return -1;
    }


#if QAPI_CRYPTO_CHACHA20_POLY1305_ENABLE_RANDOM_SIZE_UPDATES
#define NUM_CHUNKS 3
    chunk_info_t chunks[NUM_CHUNKS];
    randomize_chunks(chunks, NUM_CHUNKS, PLAINTEXT_LENGTH);
    int chunk_index;
    ciphertext_length = 0;
    unsigned char buffer[PLAINTEXT_LENGTH];
    uint32_t buffer_length = PLAINTEXT_LENGTH;
    for ( chunk_index = 0; chunk_index < NUM_CHUNKS-1; chunk_index++ ) {
        buffer_length = PLAINTEXT_LENGTH;
        status_code =
            qapi_Crypto_Op_AE_Update(
                aes_encHdl,
                &plaintext[chunks[chunk_index].start_index],
                chunks[chunk_index].length,
                buffer,
                &buffer_length
                );
        if ( status_code != QAPI_OK ) {
            CRYPTO_PRINTF("\ncrypto_poly1305: failed on update()\n");
            print_chunks_info(chunks, NUM_CHUNKS);
            return -1;
        }
        if ( buffer_length > 0 ) {
            memcpy(&ciphertext[ciphertext_length], buffer, buffer_length);
            ciphertext_length += buffer_length;
        }
    }

    buffer_length = PLAINTEXT_LENGTH;
    status_code =
        qapi_Crypto_Op_AE_Encrypt_Final(
            aes_encHdl,
            &plaintext[chunks[NUM_CHUNKS-1].start_index],
            chunks[NUM_CHUNKS-1].length,
            buffer,
            &buffer_length,
            tag,
            &tag_length
            );
    if ( buffer_length > 0 ) {
        memcpy(&ciphertext[ciphertext_length], buffer, buffer_length);
        ciphertext_length += buffer_length;
    }
#else
    ciphertext_length = PLAINTEXT_LENGTH;
    status_code = qapi_Crypto_Op_AE_Encrypt_Final(aes_encHdl, plaintext, PLAINTEXT_LENGTH, ciphertext, &ciphertext_length, tag, &tag_length);
#endif

    if ( status_code != QAPI_OK){
        CRYPTO_PRINTF("\nFailed to ae encrypt final\n");
        print_chunks_info(chunks, NUM_CHUNKS);
        return -1;
    }
    else
    {
        if ( 1 &&
            (tag_length == EXPECTED_TAG_LENGTH) &&
            (0 == memcmp(expected_tag, tag, tag_length))
           )
        {
            CRYPTO_PRINTF("\nEncryption tag is CORRECT\n");
        }
        else
        {
            CRYPTO_PRINTF("\nEncryption tag is WRONG\n");
            print_chunks_info(chunks, NUM_CHUNKS);
            return -1;
        }
        if ( 1 &&
            (ciphertext_length == EXPECTED_CIPHERTEXT_LENGTH) &&
            (0 == memcmp(expected_ciphertext, ciphertext, ciphertext_length))
           )
        {
            CRYPTO_PRINTF("\nEncryption ciphertext is CORRECT\n");
        }
        else
        {
            CRYPTO_PRINTF("\nEncryption ciphertext is WRONG\n");
            print_chunks_info(chunks, NUM_CHUNKS);
            return -1;
        }
    }

    unsigned char decrypted_plaintext[PLAINTEXT_LENGTH];
    unsigned int decrypted_PLAINTEXT_LENGTH = PLAINTEXT_LENGTH;

    // decryption
    if (qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_CHACHA20_POLY1305_E, QAPI_CRYPTO_MODE_DECRYPT_E, QAPI_CRYPTO_CHACHA20_POLY1305_KEY_BITS, &aes_decHdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc op\n");
        return -1;
    }

    if (qapi_Crypto_Op_Key_Set(aes_decHdl, aes_objHdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to set key\n");
        return -1;
    }


    if (qapi_Crypto_Op_AE_Init(aes_decHdl, (void*)nonce, nonce_length, tag_length, aad_length, ciphertext_length) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to ae init\n");
        return -1;
    }

    if (qapi_Crypto_Op_AE_AAD_Update(aes_decHdl, (void*)aad, aad_length) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to ae update aad\n");
        return -1;
    }

#if QAPI_CRYPTO_CHACHA20_POLY1305_ENABLE_RANDOM_SIZE_UPDATES
    randomize_chunks(chunks, NUM_CHUNKS, PLAINTEXT_LENGTH);
    decrypted_PLAINTEXT_LENGTH = 0;
    buffer_length = PLAINTEXT_LENGTH;
    for ( chunk_index = 0; chunk_index < NUM_CHUNKS-1; chunk_index++ ) {
        buffer_length = PLAINTEXT_LENGTH;
        status_code =
            qapi_Crypto_Op_AE_Update(
                aes_decHdl,
                &ciphertext[chunks[chunk_index].start_index],
                chunks[chunk_index].length,
                buffer,
                &buffer_length
                );
        if ( status_code != QAPI_OK ) {
            CRYPTO_PRINTF("\ncrypto_poly1305: decryption failed on update()\n");
            print_chunks_info(chunks, NUM_CHUNKS);
            return -1;
        }
        if ( buffer_length > 0 ) {
            memcpy(&decrypted_plaintext[decrypted_PLAINTEXT_LENGTH], buffer, buffer_length);
            decrypted_PLAINTEXT_LENGTH += buffer_length;
        }
    }

    buffer_length = PLAINTEXT_LENGTH;
    status_code =
        qapi_Crypto_Op_AE_Decrypt_Final(
            aes_decHdl,
            &ciphertext[chunks[NUM_CHUNKS-1].start_index],
            chunks[NUM_CHUNKS-1].length,
            buffer,
            &buffer_length,
            tag,
            tag_length
            );
    if ( buffer_length > 0 ) {
        memcpy(&decrypted_plaintext[decrypted_PLAINTEXT_LENGTH], buffer, buffer_length);
        decrypted_PLAINTEXT_LENGTH += buffer_length;
    }
#else
    status_code = qapi_Crypto_Op_AE_Decrypt_Final(aes_decHdl, ciphertext, ciphertext_length, decrypted_plaintext, &decrypted_PLAINTEXT_LENGTH, tag, tag_length);
#endif

    if ( status_code == QAPI_OK){
        if (1 &&
            (decrypted_PLAINTEXT_LENGTH == PLAINTEXT_LENGTH) &&
            (0 == memcmp(plaintext, decrypted_plaintext, PLAINTEXT_LENGTH))
           )
        {
            CRYPTO_PRINTF("\nDecryption SUCCEEDED\n");
        }
        else
        {
            CRYPTO_PRINTF("\nDecryption FAILED\n");
            print_chunks_info(chunks, NUM_CHUNKS);
        }
    }
    else {

        CRYPTO_PRINTF("\nFailed to ae decrypto final %d\n", status_code);
        print_chunks_info(chunks, NUM_CHUNKS);
    }

    qapi_Crypto_Transient_Obj_Free(aes_objHdl);
    qapi_Crypto_Op_Free(aes_encHdl);
    qapi_Crypto_Op_Free(aes_decHdl);

    return 0;
}


int32_t crypto_aesctr(int32_t argc, char* argv[])
{
    //"wmiconfig --cryptotest aesctr 128/256 "
    char srcData[64] = {0};
    char encData[64] = {0};
    char destData[64] = {0};

    qapi_Crypto_Attrib_t attr[2];

    uint32_t keyLen = atoi(argv[3]);
    uint32_t srcLen = 64;
    uint32_t destLen, encLen;

#if 0
    uint32_t t_enLen, t_destLen;
#endif


    memcpy(srcData, test_iv, 32);
    memcpy(&srcData[32], test_iv, 32);

    qapi_Crypto_Obj_Hdl_t aes_objHdl;
    qapi_Crypto_Op_Hdl_t aes_encHdl, aes_decHdl;

    //printf("srcData len %d %s\n", srcLen, srcData);
    if (qapi_Crypto_Transient_Obj_Alloc(QAPI_CRYPTO_OBJ_TYPE_AES_E, QAPI_CRYPTO_AES256_KEY_BITS, &aes_objHdl) != QAPI_OK) {
        CRYPTO_PRINTF("\n aes Failed to alloc transient obj\n");
        return -1;
    }
#if 0
    if (qapi_Crypto_Transient_Obj_Key_Gen(aes_objHdl, keyLen, NULL, 0) != QAPI_OK) {
        CRYPTO_PRINTF("\naes Failed to gen key\n");
    }
#endif
    if (keyLen == QAPI_CRYPTO_AES128_KEY_BITS){
        ref_attr_init(&attr[0], QAPI_CRYPTO_ATTR_SECRET_VALUE_E, test_iv, QAPI_CRYPTO_AES128_KEY_BYTES);
    }
    else if (keyLen == QAPI_CRYPTO_AES256_KEY_BITS){
        ref_attr_init(&attr[0], QAPI_CRYPTO_ATTR_SECRET_VALUE_E, test_iv, QAPI_CRYPTO_AES256_KEY_BYTES);
    }
    else {
        return -1;
    }

    if (qapi_Crypto_Transient_Obj_Populate(aes_objHdl, attr, 1) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to populate obj\n");
        return -1;
    }

    if (qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_AES_CTR_E, QAPI_CRYPTO_MODE_ENCRYPT_E, QAPI_CRYPTO_AES256_KEY_BITS, &aes_encHdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc op\n");
        return -1;
    }

    if (qapi_Crypto_Op_Key_Set(aes_encHdl, aes_objHdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to set key\n");
        return -1;
    }
    if (qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_AES_CTR_E, QAPI_CRYPTO_MODE_DECRYPT_E, QAPI_CRYPTO_AES256_KEY_BITS, &aes_decHdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc op\n");
        return -1;
    }

    if (qapi_Crypto_Op_Key_Set(aes_decHdl, aes_objHdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to set key\n");
        return -1;
    }

    if (qapi_Crypto_Op_Cipher_Init(aes_encHdl, (void*)test_iv, 16) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to cipher init\n");
        return -1;
    }

#if 0
    /* Enable after adding support for update */
    if (qapi_Crypto_Op_Cipher_Update(aes_encHdl, (void*)srcData, 20, encData, &encLen) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to cipher update\n");
        return -1;
    }
    t_enLen = encLen;
    if (qapi_Crypto_Op_Cipher_Update(aes_encHdl, (void*)&srcData[20], 20, &encData[16], &encLen) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to cipher update\n");
        return -1;
    }
    t_enLen+=encLen;
#endif
    encLen = sizeof(encData);
    if (qapi_Crypto_Op_Cipher_Final(aes_encHdl, (void*)srcData, srcLen, encData, &encLen) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to cipher doFinal\n");
        return -1;
    }
#if 0
    t_enLen+=encLen;
    printf("encrypt encData len %d :\n", t_enLen);
    int i;
    for (i=0; i<t_enLen; i++)
        printf("%02x, ", encData[i]);
    printf("\n ");
#endif
    if (qapi_Crypto_Op_Cipher_Init(aes_decHdl, (void*)test_iv, 16) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to cipher init\n");
        return -1;
    }

#if 0
    /* Enable after adding support for update */
    if (qapi_Crypto_Op_Cipher_Update(aes_decHdl, (void*)encData, 30, destData, &destLen) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to cipher update\n");
        return -1;
    }
    t_destLen = destLen;
    if (qapi_Crypto_Op_Cipher_Update(aes_decHdl, (void*)&encData[30], 30, &destData[16], &destLen) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to cipher update\n");
        return -1;
    }
    t_destLen += destLen;
#endif
    destLen = sizeof(destData);
    if (qapi_Crypto_Op_Cipher_Final(aes_decHdl, encData, encLen, destData, &destLen) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to cipher doFinal\n");
        return -1;
    }
#if 0
    t_destLen += destLen;
    printf("encrypt decData len %d :\n", t_destLen );
    for (i=0; i<t_destLen ; i++)
        printf("%02x, ", destData[i]);
    printf("\n ");
#endif
    if (memcmp(srcData, destData, srcLen)==0){
        CRYPTO_PRINTF("aesctr encryption and decryption success\n");
    }
    else {
        CRYPTO_PRINTF("aesctr encryption and decryption failed\n");
    }

    qapi_Crypto_Transient_Obj_Free(aes_objHdl);
    qapi_Crypto_Op_Free(aes_encHdl);
    qapi_Crypto_Op_Free(aes_decHdl);
    return 0;
}

#if CRYPTO_CONFIG_3DES
int32_t crypto_descbc(int32_t argc, char* argv[])
{
    //"wmiconfig --cryptotest des/3des"
    char srcData[16] = {0};
    char encData[16] = {0};
    char destData[16] = {0};

    qapi_Crypto_Attrib_t attr[2];
    uint32_t keyLen;
    uint32_t srcLen = 16;
    uint32_t encLen, destLen;

    if (strcmp(argv[2], "des") == 0)
        keyLen = QAPI_CRYPTO_DES_KEY_BITS;
    else if (strcmp(argv[2], "3des") == 0)
        keyLen = QAPI_CRYPTO_DES3_KEY_BITS;

    memcpy(srcData, test_iv, srcLen);

    qapi_Crypto_Obj_Hdl_t des_objHdl;
    qapi_Crypto_Op_Hdl_t des_encHdl, des_decHdl;

    //printf("srcData len %d %s\n", srcLen, srcData);
    if (keyLen == QAPI_CRYPTO_DES_KEY_BITS)
        qapi_Crypto_Transient_Obj_Alloc(QAPI_CRYPTO_OBJ_TYPE_DES, QAPI_CRYPTO_DES_KEY_BITS, &des_objHdl);
    else if (keyLen == QAPI_CRYPTO_DES3_KEY_BITS)
        qapi_Crypto_Transient_Obj_Alloc(QAPI_CRYPTO_OBJ_TYPE_DES3, QAPI_CRYPTO_DES3_KEY_BITS, &des_objHdl);

#if 0
    if (qapi_Crypto_Transient_Obj_Key_Gen(des_objHdl, keyLen, NULL, 0) != QAPI_OK) {
        CRYPTO_PRINTF("\naes Failed to gen key\n");
    }
#endif

    if (keyLen == QAPI_CRYPTO_DES_KEY_BITS){
        ref_attr_init(&attr[0], QAPI_CRYPTO_ATTR_SECRET_VALUE_E, test_iv, QAPI_CRYPTO_DES_KEY_BYTES);
    }
    else if (keyLen == QAPI_CRYPTO_DES3_KEY_BITS){
        ref_attr_init(&attr[0], QAPI_CRYPTO_ATTR_SECRET_VALUE_E, test_iv, QAPI_CRYPTO_DES3_KEY_BYTES);
    }else {
        return -1;
    }

    if (qapi_Crypto_Transient_Obj_Populate(des_objHdl, attr, 1) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to populate obj\n");
        return -1;
    }

    if (keyLen == QAPI_CRYPTO_DES_KEY_BITS)
        qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_DES_CBC_NOPAD, QAPI_CRYPTO_MODE_ENCRYPT_E, QAPI_CRYPTO_DES_KEY_BITS, &des_encHdl);
    else if (keyLen == QAPI_CRYPTO_DES3_KEY_BITS)
        qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_DES3_CBC_NOPAD, QAPI_CRYPTO_MODE_ENCRYPT_E, QAPI_CRYPTO_DES3_KEY_BITS, &des_encHdl);

    if (qapi_Crypto_Op_Key_Set(des_encHdl, des_objHdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to set key\n");
        return -1;
    }

    if (keyLen == QAPI_CRYPTO_DES_KEY_BITS)
        qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_DES_CBC_NOPAD, QAPI_CRYPTO_MODE_DECRYPT_E, keyLen, &des_decHdl);
    else if (keyLen == QAPI_CRYPTO_DES3_KEY_BITS)
        qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_DES3_CBC_NOPAD, QAPI_CRYPTO_MODE_DECRYPT_E, keyLen, &des_decHdl);

    if (qapi_Crypto_Op_Key_Set(des_decHdl, des_objHdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to set key\n");
        return -1;
    }

    if (qapi_Crypto_Op_Cipher_Init(des_encHdl, (void*)test_iv, 8) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to cipher init\n");
        return -1;
    }

    if (qapi_Crypto_Op_Cipher_Update(des_encHdl, (void*)srcData, srcLen/2, encData, &encLen) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to cipher update\n");
        return -1;
    }

    if (qapi_Crypto_Op_Cipher_Final(des_encHdl, (void*)&srcData[8], srcLen/2, &encData[8], &encLen) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to cipher doFinal\n");
        return -1;
    }

#if 0
    printf("encrypt encData len %d :\n", encLen);
    int i;
    for (i=0; i<encLen; i++)
        printf("%02x, ", encData[i]);
    printf("\n ");
#endif
    if (qapi_Crypto_Op_Cipher_Init(des_decHdl, (void*)test_iv, 8) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to cipher init\n");
        return -1;
    }

    if (qapi_Crypto_Op_Cipher_Update(des_decHdl, (void*)encData, encLen, destData, &destLen) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to cipher update\n");
        return -1;
    }

    if (qapi_Crypto_Op_Cipher_Final(des_decHdl, (void*)&encData[8], encLen, &destData[8], &destLen) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to cipher doFinal\n");
        return -1;
    }
    else{
        //printf("Decrypt destData len %d %s\n", destLen, destData);
        if (memcmp(destData, srcData, destLen)==0){
            CRYPTO_PRINTF("des/3des encryption and decryption success\n");
        }
        else {
            CRYPTO_PRINTF("des/3des encryption and decryption fail\n");
        }
    }

    qapi_Crypto_Transient_Obj_Free(des_objHdl);
    qapi_Crypto_Op_Free(des_encHdl);
    qapi_Crypto_Op_Free(des_decHdl);
    return 0;
}
#endif
int32_t crypto_hmacsha256()
{
    //"wmiconfig --cryptotest hmacsha256 "
    int ret = QAPI_OK;
#if 0
    const  uint8_t *msg = (uint8_t[]){0x54, 0x65, 0x73, 0x74, 0x20, 0x55, 0x73, 0x69, 0x6e, 0x67, 0x20, 0x4c, 0x61, 0x72, 0x67, 0x65, 0x72, 0x20, 0x54, 0x68, 0x61, 0x6e, 0x20, 0x42, 0x6c, 0x6f, 0x63, 0x6b, 0x2d, 0x53, 0x69, 0x7a, 0x65, 0x20, 0x4b, 0x65, 0x79, 0x20, 0x2d, 0x20, 0x48, 0x61, 0x73, 0x68, 0x20, 0x4b, 0x65, 0x79, 0x20, 0x46, 0x69, 0x72, 0x73, 0x74};
    const  uint8_t *key = (uint8_t[]){0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa};
    const  uint8_t *hmac = (uint8_t[]){0x17, 0xc7, 0x97, 0xd4, 0xdc, 0xa7, 0x71, 0xbb, 0x78, 0x27, 0x8b, 0x21, 0xef, 0x40, 0x67, 0x51, 0xb8, 0x93, 0x17, 0x9f, 0xdf, 0x3a, 0xeb, 0xb7, 0xdb, 0x0c, 0x04, 0x81, 0xfa, 0x6f, 0x2c, 0x37};
    uint32_t msgLen = 54;
    uint32_t keyLen = 36*8;

    const uint8_t *msg = (uint8_t[]){0x54, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20, 0x61, 0x20, 0x74, 0x65, 0x73, 0x74,\
        0x20, 0x75, 0x73, 0x69, 0x6e, 0x67, 0x20, 0x61, 0x20, 0x6c, 0x61, 0x72, 0x67, 0x65, 0x72,\
            0x20, 0x74, 0x68, 0x61, 0x6e, 0x20, 0x62, 0x6c, 0x6f, 0x63, 0x6b, 0x2d, 0x73, 0x69, 0x7a, 0x65, \
            0x20, 0x6b, 0x65, 0x79, 0x20, 0x61, 0x6e, 0x64, 0x20, 0x61, 0x20, 0x6c, 0x61, 0x72, 0x67, 0x65, 0x72, \
            0x20, 0x74, 0x68, 0x61, 0x6e, 0x20, 0x62, 0x6c, 0x6f, 0x63, 0x6b, 0x2d, 0x73, 0x69, 0x7a, 0x65, 0x20, 0x64, \
            0x61, 0x74, 0x61, 0x2e, 0x20, 0x54, 0x68, 0x65, 0x20, 0x6b, 0x65, 0x79, 0x20, 0x6e, 0x65, 0x65, 0x64, 0x73, \
            0x20, 0x74, 0x6f, 0x20, 0x62, 0x65, 0x20, 0x68, 0x61, 0x73, 0x68, 0x65, 0x64, 0x20, 0x62, 0x65, 0x66, 0x6f, \
            0x72, 0x65, 0x20, 0x62, 0x65, 0x69, 0x6e, 0x67, 0x20, 0x75, 0x73, 0x65, 0x64, 0x20, 0x62, 0x79, 0x20, 0x74,\
            0x68, 0x65, 0x20, 0x48, 0x4d, 0x41, 0x43, 0x20, 0x61, 0x6c, 0x67, 0x6f, 0x72, 0x69, 0x74, 0x68, 0x6d, 0x2e};

    const uint8_t *key = (uint8_t[]){0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, \
        0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,\
            0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,\
            0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, \
            0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,\
            0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, \
            0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa};
    const uint8_t *hmac = (uint8_t[]){0x18, 0xe6, 0xdf, 0x57, 0x25, 0xd4, 0x21, 0x7b, 0x30, 0xff, 0x15, 0x86, 0x0c, 0x1e, 0x32, 0xf4, 0x60, 0x1b, 0x03, 0xca, 0xe3, 0x65, 0x9e, 0x48, 0xef, 0x93, 0x36, 0x6f, 0xc9, 0x19, 0x00, 0x6c};
    // uint8_t hkey[] = {0x6a, 0x27, 0x5e, 0x92, 0xd3, 0x56, 0x58, 0x78, 0x21, 0xfd, 0x00, 0x00, 0x56, 0x50, 0x5f, 0x73, 0x33, 0x87, 0x2a, 0xc1, 0x37, 0x8d, 0x82, 0xc3, 0x3c, 0x49, 0xcd, 0x9b, 0x1f, 0x67, 0x77, 0x68};
    uint32_t msgLen = 152;
    uint32_t keyLen = 122*8;
#endif
    uint8_t* msg = (uint8_t *)malloc(384);
    uint8_t *key = msg;
    //uint8_t hmac[32] = (uint8_t[]){0x17, 0xc7, 0x97, 0xd4, 0xdc, 0xa7, 0x71, 0xbb, 0x78, 0x27, 0x8b, 0x21, 0xef, 0x40, 0x67, 0x51, 0xb8, 0x93, 0x17, 0x9f, 0xdf, 0x3a, 0xeb, 0xb7, 0xdb, 0x0c, 0x04, 0x81, 0xfa, 0x6f, 0x2c, 0x37};
    uint32_t keyLen = 0;
    char mac[32] = {0};
    uint32_t macLen = 0;
    qapi_Crypto_Attrib_t attr[1];

    uint8_t i;
    uint32_t testLen[4][5] = {{70, 90, 160, 60, 24*8},{30, 32, 1, 1,46*8},{120,72, 64, 128, 64*8}, {64,64, 64, 128,128*8}};
    for (i=0;  i<384/32; i++){
        memcpy(&msg[i*32], test_iv, 32);
    }

    qapi_Crypto_Obj_Hdl_t mac_objHdl;
    qapi_Crypto_Op_Hdl_t compute_opHdl;
    qapi_Crypto_Op_Hdl_t compare_opHdl;

    for (i=0; i<4; i++){
        keyLen = testLen[i][4];
        if (qapi_Crypto_Transient_Obj_Alloc(QAPI_CRYPTO_OBJ_TYPE_HMAC_SHA256_E, QAPI_CRYPTO_HMAC_SHA256_MAX_KEY_BITS, &mac_objHdl) != QAPI_OK) {
            CRYPTO_PRINTF("\n aes Failed to alloc transient obj\n");
        }
#if 0
        if (qapi_Crypto_Transient_Obj_Key_Gen(mac_objHdl, QAPI_CRYPTO_HMAC_SHA256_MAX_KEY_BITS, NULL, 0) != QAPI_OK) {
            CRYPTO_PRINTF("\naes Failed to gen key\n");
        }
#endif
        if (keyLen<QAPI_CRYPTO_HMAC_SHA256_MIN_KEY_BITS || keyLen>QAPI_CRYPTO_HMAC_SHA256_MAX_KEY_BITS){
            CRYPTO_PRINTF("\nNot valid key length\n");
        }
        ref_attr_init(&attr[0], QAPI_CRYPTO_ATTR_SECRET_VALUE_E, (void*)key, keyLen/8);

        if (qapi_Crypto_Transient_Obj_Populate(mac_objHdl, &attr[0], 1) != QAPI_OK) {
            CRYPTO_PRINTF("\nFailed to populate mac public obj\n");
        }

        if (qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_HMAC_SHA256_E, QAPI_CRYPTO_MODE_MAC_E, QAPI_CRYPTO_HMAC_SHA256_MAX_KEY_BITS, &compute_opHdl) != QAPI_OK) {
            CRYPTO_PRINTF("\nFailed to alloc op\n");
        }

        if (qapi_Crypto_Op_Key_Set(compute_opHdl, mac_objHdl) != QAPI_OK) {
            CRYPTO_PRINTF("\nFailed to set key\n");
        }

        if (qapi_Crypto_Op_Mac_Init(compute_opHdl, NULL, 0) != QAPI_OK) {
            CRYPTO_PRINTF("\nFailed to mac init\n");
        }

        int len = testLen[i][0]+testLen[i][1] +testLen[i][2] + testLen[i][3];
        if (i<2){
            if (qapi_Crypto_Op_Mac_Update(compute_opHdl, (void*)msg, testLen[i][0]) != QAPI_OK) {
                CRYPTO_PRINTF("\nFailed to mac compute final\n");
            }

            if (qapi_Crypto_Op_Mac_Update(compute_opHdl, (void*)&msg[testLen[i][0]], testLen[i][1]) != QAPI_OK) {
                CRYPTO_PRINTF("\nFailed to mac compute final\n");
            }

            if (qapi_Crypto_Op_Mac_Update(compute_opHdl, (void*)&msg[testLen[i][0]+testLen[i][1]], testLen[i][2]) != QAPI_OK) {
                CRYPTO_PRINTF("\nFailed to mac compute final\n");
            }

            macLen = sizeof(mac);
            if (qapi_Crypto_Op_Mac_Final_Compute(compute_opHdl, (void*)&msg[len - testLen[i][3]], testLen[i][3], mac, &macLen) != QAPI_OK) {
                CRYPTO_PRINTF("\nFailed to mac compute final\n");
            }
        }
        else {
            macLen = sizeof(mac);
            if (qapi_Crypto_Op_Mac_Final_Compute(compute_opHdl, (void*)msg, len, mac, &macLen) != QAPI_OK) {
                CRYPTO_PRINTF("\nFailed to mac compute final\n");
            }
        }
#if 0
        printf("hmacsha256 mac len %d :\n", macLen);
        int j;
        for (j=0; j<macLen; j++)
            printf("%02x, ", mac[j]);
        printf("\n ");
#endif
        if (qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_HMAC_SHA256_E, QAPI_CRYPTO_MODE_MAC_E, QAPI_CRYPTO_HMAC_SHA256_MAX_KEY_BITS, &compare_opHdl) != QAPI_OK) {
            CRYPTO_PRINTF("\nFailed to alloc op\n");
        }

        if (qapi_Crypto_Op_Key_Set(compare_opHdl, mac_objHdl) != QAPI_OK) {
            CRYPTO_PRINTF("\nFailed to set key\n");
        }

        if (qapi_Crypto_Op_Mac_Init(compare_opHdl, NULL, 0) != QAPI_OK) {
            CRYPTO_PRINTF("\nFailed to mac init\n");
        }
        if (i>=2){
            if (qapi_Crypto_Op_Mac_Update(compare_opHdl, (void*)msg, testLen[i][0]) != QAPI_OK) {
                CRYPTO_PRINTF("\nFailed to mac compute final\n");
            }

            if (qapi_Crypto_Op_Mac_Update(compare_opHdl, (void*)&msg[testLen[i][0]], testLen[i][1]) != QAPI_OK) {
                CRYPTO_PRINTF("\nFailed to mac compute final\n");
            }

            if (qapi_Crypto_Op_Mac_Update(compare_opHdl, (void*)&msg[testLen[i][0]+testLen[i][1]], testLen[i][2]) != QAPI_OK) {
                CRYPTO_PRINTF("\nFailed to mac compute final\n");
            }

            ret = qapi_Crypto_Op_Mac_Final_Compare(compare_opHdl, (void*)&msg[len - testLen[i][3]], testLen[i][3], mac, macLen);
        }
        else {

            ret = qapi_Crypto_Op_Mac_Final_Compare(compare_opHdl, (void*)msg, len, (void*)mac, macLen);
        }
        CRYPTO_PRINTF("Case %d: ", i);
        if (ret == QAPI_ERR_CRYPTO_INVALID_MAC){
            CRYPTO_PRINTF("\nInvalid mac to mac compare final\n");
        }
        else if (ret == QAPI_ERROR){
            CRYPTO_PRINTF("\nFailed to mac compare final\n");
        }
        else {
            CRYPTO_PRINTF("hmacsha256 compare success\n");
        }
        qapi_Crypto_Transient_Obj_Free(mac_objHdl);
        qapi_Crypto_Op_Free(compute_opHdl);
        qapi_Crypto_Op_Free(compare_opHdl);
    }
    free(msg);
    return 0;
}

int32_t crypto_hmacsha1(int32_t argc, char* argv[])
{
    //"wmiconfig --cryptotest hmacsha1 "
    int ret = QAPI_OK;
#if 0
    const uint8_t *msg = (uint8_t[]){0x53, 0x61, 0x6D, 0x70, 0x6C, 0x65 , 0x20, 0x6D, 0x65, 0x73, 0x73, 0x61, 0x67, 0x65, 0x20, 0x66, 0x6F, 0x72, 0x20, 0x6B, 0x65, 0x79, 0x6C, 0x65, 0x6E, 0x3C, 0x62, 0x6C, 0x6F, 0x63, 0x6B, 0x6C, 0x65, 0x6E, 0x2C, 0x20, 0x77, 0x69, 0x74, 0x68, 0x20, 0x74, 0x72, 0x75, 0x6E, 0x63, 0x61, 0x74, 0x65, 0x64, 0x20, 0x74, 0x61, 0x67};
    const uint8_t *key = (uint8_t[]){0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30};
    const uint8_t *hmac = (uint8_t[]){0xFE, 0x35, 0x29, 0x56, 0x5C, 0xD8, 0xE2, 0x8C, 0x5F, 0xA7, 0x9E, 0xAC, 0x9D, 0x80, 0x23, 0xB5, 0x3B, 0x28, 0x9D, 0x96};
    uint32_t msgLen = 54;
    uint32_t keyLen =49*8;

    const char *msg = "Test Using Larger Than Block-Size Key and Larger Than One Block-Size Data";
    uint8_t key[80] = {0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa};
    //const uint8_t *hmac = (uint8_t[]){0xe8, 0xe9, 0x9d, 0x0f, 0x45, 0x23, 0x7d, 0x78, 0x6d, 0x6b, 0xba, 0xa7, 0x96, 0x5c, 0x78, 0x08, 0xbb, 0xff, 0x1a, 0x91};
    uint32_t msgLen = 73;
    uint32_t keyLen =50*8;
#endif
    uint8_t* msg = malloc(384);

    uint8_t *key = msg;
    uint32_t keyLen = 0;
    char mac[32] = {0};
    uint32_t macLen = 0;
    qapi_Crypto_Attrib_t attr[1];

    uint8_t i;
    uint32_t testLen[4][5] = {{70, 90, 160, 60, 10*8},{30, 32, 1, 1, 30*8},{120,72, 64, 128, 48*8}, {64,64, 64, 128,64*8}};
    for (i=0;  i<384/32; i++){
        memcpy(&msg[i*32], test_iv, 32);
    }

    qapi_Crypto_Obj_Hdl_t mac_objHdl;
    qapi_Crypto_Op_Hdl_t compute_opHdl;
    qapi_Crypto_Op_Hdl_t compare_opHdl;

    for (i=0; i<4; i++){
        keyLen = testLen[i][4];
        if (qapi_Crypto_Transient_Obj_Alloc(QAPI_CRYPTO_OBJ_TYPE_HMAC_SHA1_E, QAPI_CRYPTO_HMAC_SHA1_MAX_KEY_BITS, &mac_objHdl) != QAPI_OK) {
            CRYPTO_PRINTF("\n aes Failed to alloc transient obj\n");
        }
#if 0
        if (qapi_Crypto_Transient_Obj_Key_Gen(mac_objHdl, QAPI_CRYPTO_HMAC_SHA256_MAX_KEY_BITS, NULL, 0) != QAPI_OK) {
            CRYPTO_PRINTF("\naes Failed to gen key\n");
        }
#endif
        if (keyLen<QAPI_CRYPTO_HMAC_SHA1_MIN_KEY_BITS || keyLen>QAPI_CRYPTO_HMAC_SHA1_MAX_KEY_BITS){
            CRYPTO_PRINTF("\nUnvalid key length\n");
        }
        ref_attr_init(&attr[0], QAPI_CRYPTO_ATTR_SECRET_VALUE_E, (void*)key, keyLen/8);

        if (qapi_Crypto_Transient_Obj_Populate(mac_objHdl, &attr[0], 1) != QAPI_OK) {
            CRYPTO_PRINTF("\nFailed to populate mac public obj\n");
        }

        if (qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_HMAC_SHA1_E, QAPI_CRYPTO_MODE_MAC_E, QAPI_CRYPTO_HMAC_SHA1_MAX_KEY_BITS, &compute_opHdl) != QAPI_OK) {
            CRYPTO_PRINTF("\nFailed to alloc op\n");
        }

        if (qapi_Crypto_Op_Key_Set(compute_opHdl, mac_objHdl) != QAPI_OK) {
            CRYPTO_PRINTF("\nFailed to set key\n");
        }

        if (qapi_Crypto_Op_Mac_Init(compute_opHdl, NULL, 0) != QAPI_OK) {
            CRYPTO_PRINTF("\nFailed to mac init\n");
        }

        if (qapi_Crypto_Op_Reset(compute_opHdl) != QAPI_OK) {
            CRYPTO_PRINTF("\nFailed to reset\n");
        }

        if (qapi_Crypto_Op_Mac_Init(compute_opHdl, NULL, 0) != QAPI_OK) {
            CRYPTO_PRINTF("\nFailed to mac init\n");
        }

        int len = testLen[i][0]+testLen[i][1] +testLen[i][2] + testLen[i][3];
        if (i<2){
            if (qapi_Crypto_Op_Mac_Update(compute_opHdl, (void*)msg, testLen[i][0]) != QAPI_OK) {
                CRYPTO_PRINTF("\nFailed to mac compute final\n");
            }

            if (qapi_Crypto_Op_Mac_Update(compute_opHdl, (void*)&msg[testLen[i][0]], testLen[i][1]) != QAPI_OK) {
                CRYPTO_PRINTF("\nFailed to mac compute final\n");
            }

            if (qapi_Crypto_Op_Mac_Update(compute_opHdl, (void*)&msg[testLen[i][0]+testLen[i][1]], testLen[i][2]) != QAPI_OK) {
                CRYPTO_PRINTF("\nFailed to mac compute final\n");
            }

            if (qapi_Crypto_Op_Mac_Final_Compute(compute_opHdl, (void*)&msg[len - testLen[i][3]], testLen[i][3], mac, &macLen) != QAPI_OK) {
                CRYPTO_PRINTF("\nFailed to mac compute final\n");
            }
        }
        else {
            if (qapi_Crypto_Op_Mac_Final_Compute(compute_opHdl, (void*)msg, len, mac, &macLen) != QAPI_OK) {
                CRYPTO_PRINTF("\nFailed to mac compute final\n");
            }
        }
#if 0
        printf("hmacsha1 mac len %d :\n", macLen);
        int j;
        for (j=0; j<macLen; j++)
            printf("%02x, ", mac[j]);
        printf("\n ");
#endif
        if (qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_HMAC_SHA1_E, QAPI_CRYPTO_MODE_MAC_E, QAPI_CRYPTO_HMAC_SHA1_MAX_KEY_BITS, &compare_opHdl) != QAPI_OK) {
            CRYPTO_PRINTF("\nFailed to alloc op\n");
        }

        if (qapi_Crypto_Op_Key_Set(compare_opHdl, mac_objHdl) != QAPI_OK) {
            CRYPTO_PRINTF("\nFailed to set key\n");
        }

        if (qapi_Crypto_Op_Mac_Init(compare_opHdl, NULL, 0) != QAPI_OK) {
            CRYPTO_PRINTF("\nFailed to mac init\n");
        }

        if (i>=2){
            if (qapi_Crypto_Op_Mac_Update(compare_opHdl, (void*)msg, testLen[i][0]) != QAPI_OK) {
                CRYPTO_PRINTF("\nFailed to mac compute final\n");
            }

            if (qapi_Crypto_Op_Mac_Update(compare_opHdl, (void*)&msg[testLen[i][0]], testLen[i][1]) != QAPI_OK) {
                CRYPTO_PRINTF("\nFailed to mac compute final\n");
            }

            if (qapi_Crypto_Op_Mac_Update(compare_opHdl, (void*)&msg[testLen[i][0]+testLen[i][1]], testLen[i][2]) != QAPI_OK) {
                CRYPTO_PRINTF("\nFailed to mac compute final\n");
            }

            ret = qapi_Crypto_Op_Mac_Final_Compare(compare_opHdl, (void*)&msg[len - testLen[i][3]], testLen[i][3], mac, macLen);
        }
        else {
            ret = qapi_Crypto_Op_Mac_Final_Compare(compare_opHdl, (void*)msg, len, (void*)mac, macLen);
        }
        CRYPTO_PRINTF("Case %d: ", i);
        if (ret == QAPI_ERR_CRYPTO_INVALID_MAC){
            CRYPTO_PRINTF("\nInvalid mac to mac compare final\n");
        }
        else if (ret == QAPI_ERROR){
            CRYPTO_PRINTF("\nFailed to mac compare final\n");
        }
        else {
            CRYPTO_PRINTF("hmacsha1 compare success\n");
        }

        qapi_Crypto_Transient_Obj_Free(mac_objHdl);
        qapi_Crypto_Op_Free(compute_opHdl);
        qapi_Crypto_Op_Free(compare_opHdl);
    }
    free(msg);
    return 0;
}

int32_t crypto_hmacsha512()
{
    //"wmiconfig --cryptotest hmacsha512 "
    char *str = "Sample message for keylen<blocklen";
    uint8_t msg[48] = {0};
    const  uint8_t *key = (uint8_t[]){0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13,\
        0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, \
            0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B,\
            0x3C, 0x3D, 0x3E, 0x3F};
    char hmac[] = {0xFD, 0x44, 0xC1, 0x8B, 0xDA, 0x0B, 0xB0, 0xA6, 0xCE, 0x0E, 0x82, 0xB0, 0x31, 0xBF, 0x28, 0x18, 0xF6, 0x53, 0x9B, 0xD5,
        0x6E, 0xC0, 0x0B, 0xDC, 0x10, 0xA8, 0xA2, 0xD7, 0x30, 0xB3, 0x63, 0x4D, 0xE2, 0x54, 0x5D, 0x63, 0x9B, 0x0F, 0x2C, 0xF7,
        0x10, 0xD0, 0x69, 0x2C, 0x72, 0xA1, 0x89, 0x6F, 0x1F, 0x21, 0x1C, 0x2B, 0x92, 0x2D, 0x1A, 0x96, 0xC3, 0x92, 0xE0, 0x7E,
        0x7E, 0xA9, 0xFE, 0xDC};
    uint32_t msgLen = 34;
    uint32_t keyLen =64*8;
    memcpy(msg, str,msgLen);
    char mac[64] = {0};
    uint32_t macLen = 0;
    qapi_Crypto_Attrib_t attr[1];

    //memcpy(msg, argv[4], msgLen);

    qapi_Crypto_Obj_Hdl_t mac_objHdl;
    qapi_Crypto_Op_Hdl_t compute_opHdl;
    qapi_Crypto_Op_Hdl_t compare_opHdl;

    //printf("srcData len %d %s\n", srcLen, srcData);
    if (qapi_Crypto_Transient_Obj_Alloc(QAPI_CRYPTO_OBJ_TYPE_HMAC_SHA512_E, QAPI_CRYPTO_HMAC_SHA512_MAX_KEY_BITS, &mac_objHdl) != QAPI_OK) {
        CRYPTO_PRINTF("\n aes Failed to alloc transient obj\n");
        return -1;
    }
#if 0
    if (qapi_Crypto_Transient_Obj_Key_Gen(mac_objHdl, QAPI_CRYPTO_HMAC_SHA512_MAX_KEY_BITS, NULL, 0) != QAPI_OK) {
        CRYPTO_PRINTF("\naes Failed to gen key\n");
    }
#endif
    if (keyLen<QAPI_CRYPTO_HMAC_SHA512_MIN_KEY_BITS || keyLen>QAPI_CRYPTO_HMAC_SHA512_MAX_KEY_BITS){
        CRYPTO_PRINTF("\nUnvalid key length\n");
        return -1;
    }
    ref_attr_init(&attr[0], QAPI_CRYPTO_ATTR_SECRET_VALUE_E, (void*)key, keyLen/8);

    if (qapi_Crypto_Transient_Obj_Populate(mac_objHdl, &attr[0], 1) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to populate mac public obj\n");
        return -1;
    }

    if (qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_HMAC_SHA512_E, QAPI_CRYPTO_MODE_MAC_E, QAPI_CRYPTO_HMAC_SHA512_MAX_KEY_BITS, &compute_opHdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc op\n");
        return -1;
    }

    if (qapi_Crypto_Op_Key_Set(compute_opHdl, mac_objHdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to set key\n");
        return -1;
    }

    if (qapi_Crypto_Op_Mac_Init(compute_opHdl, NULL, 0) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to mac init\n");
        return -1;
    }

    macLen = sizeof(mac);
    if (qapi_Crypto_Op_Mac_Final_Compute(compute_opHdl, (void*)msg, msgLen, mac, &macLen) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to mac compute final\n");
        return -1;
    }
#if 0
    printf("hmacsha512 mac len %d :\n", macLen);
    int i;
    for (i=0; i<macLen; i++)
        printf("%02x, ", mac[i]);
    printf("\n ");
#endif
    if (qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_HMAC_SHA512_E, QAPI_CRYPTO_MODE_MAC_E, QAPI_CRYPTO_HMAC_SHA512_MAX_KEY_BITS, &compare_opHdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc op\n");
        return -1;
    }

    if (qapi_Crypto_Op_Key_Set(compare_opHdl, mac_objHdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to set key\n");
        return -1;
    }

    if (qapi_Crypto_Op_Mac_Init(compare_opHdl, NULL, 0) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to mac init\n");
        return -1;
    }
/*
    if (qapi_Crypto_Op_Mac_Final_Compute(compare_opHdl, (void*)msg, msgLen, hmac, &macLen) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to mac compute final\n");
        return -1;
    }
*/
    if (qapi_Crypto_Op_Mac_Final_Compare(compare_opHdl, (void*)msg, msgLen, (void*)hmac, macLen) != QAPI_OK) {
    //if (memcmp(hmac, mac, macLen) != 0) {
        CRYPTO_PRINTF("\nFailed to mac compare final\n");
        return -1;
    }
    else {
        CRYPTO_PRINTF("hmacsha512 compare success\n");
    }

    qapi_Crypto_Transient_Obj_Free(mac_objHdl);
    qapi_Crypto_Op_Free(compute_opHdl);
    qapi_Crypto_Op_Free(compare_opHdl);
    return 0;
}

int32_t crypto_hmacsha384()
{
    //"wmiconfig --cryptotest hmacsha384 "
    char *str = "Sample message for keylen<blocklen";
    uint8_t msg[48] = {0};
    const uint8_t *key = (uint8_t[]){0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13,\
        0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, \
            0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F};
    const uint8_t *hmac = (uint8_t[]){0x6E, 0xB2, 0x42, 0xBD, 0xBB, 0x58, 0x2C, 0xA1, 0x7B, 0xEB, 0xFA, 0x48, 0x1B, 0x1E, 0x23, 0x21, 0x14, 0x64, 0xD2, 0xB7, 0xF8, 0xC2, 0x0B, 0x9F, 0xF2, 0x20, 0x16, 0x37, 0xB9, 0x36, 0x46, 0xAF, 0x5A, 0xE9, 0xAC, 0x31, 0x6E, 0x98, 0xDB, 0x45, 0xD9, 0xCA, 0xE7, 0x73, 0x67, 0x5E, 0xEE, 0xD0};
    uint32_t msgLen = 34;
    uint32_t keyLen =48*8;
    memcpy(msg, str,msgLen);
    char mac[64] = {0};
    uint32_t macLen = 0;
    qapi_Crypto_Attrib_t attr[1];

    //memcpy(msg, argv[4], msgLen);

    qapi_Crypto_Obj_Hdl_t mac_objHdl;
    qapi_Crypto_Op_Hdl_t compute_opHdl;
    qapi_Crypto_Op_Hdl_t compare_opHdl;

    //printf("srcData len %d %s\n", srcLen, srcData);
    if (qapi_Crypto_Transient_Obj_Alloc(QAPI_CRYPTO_OBJ_TYPE_HMAC_SHA384_E, QAPI_CRYPTO_HMAC_SHA384_MAX_KEY_BITS, &mac_objHdl) != QAPI_OK) {
        CRYPTO_PRINTF("\n aes Failed to alloc transient obj\n");
        return -1;
    }
#if 0
    if (qapi_Crypto_Transient_Obj_Key_Gen(mac_objHdl, QAPI_CRYPTO_HMAC_SHA384_MAX_KEY_BITS, NULL, 0) != QAPI_OK) {
        CRYPTO_PRINTF("\naes Failed to gen key\n");
    }
#endif
    if (keyLen<QAPI_CRYPTO_HMAC_SHA384_MIN_KEY_BITS || keyLen>QAPI_CRYPTO_HMAC_SHA384_MAX_KEY_BITS){
        CRYPTO_PRINTF("\nUnvalid key length\n");
        return -1;
    }
    ref_attr_init(&attr[0], QAPI_CRYPTO_ATTR_SECRET_VALUE_E, (void*)key, keyLen/8);

    if (qapi_Crypto_Transient_Obj_Populate(mac_objHdl, &attr[0], 1) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to populate mac public obj\n");
        return -1;
    }

    if (qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_HMAC_SHA384_E, QAPI_CRYPTO_MODE_MAC_E, QAPI_CRYPTO_HMAC_SHA384_MAX_KEY_BITS, &compute_opHdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc op\n");
        return -1;
    }

    if (qapi_Crypto_Op_Key_Set(compute_opHdl, mac_objHdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to set key\n");
        return -1;
    }

    if (qapi_Crypto_Op_Mac_Init(compute_opHdl, NULL, 0) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to mac init\n");
        return -1;
    }

    macLen = sizeof(mac);
    if (qapi_Crypto_Op_Mac_Final_Compute(compute_opHdl, (void*)msg, msgLen, mac, &macLen) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to mac compute final\n");
        return -1;
    }
#if 0
    printf("hmacsha384 mac len %d :\n", macLen);
    int i;
    for (i=0; i<macLen; i++)
        printf("%02x, ", mac[i]);
    printf("\n ");
#endif
    if (qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_HMAC_SHA384_E, QAPI_CRYPTO_MODE_MAC_E, QAPI_CRYPTO_HMAC_SHA384_MAX_KEY_BITS, &compare_opHdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc op\n");
        return -1;
    }

    if (qapi_Crypto_Op_Key_Set(compare_opHdl, mac_objHdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to set key\n");
        return -1;
    }

    if (qapi_Crypto_Op_Mac_Init(compare_opHdl, NULL, 0) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to mac init\n");
        return -1;
    }

    if (qapi_Crypto_Op_Mac_Final_Compare(compare_opHdl, (void*)msg, msgLen, (void*)hmac, macLen) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to mac compare final\n");
        return -1;
    }
    else {
        CRYPTO_PRINTF("hmacsha384 compare success\n");
    }

    qapi_Crypto_Transient_Obj_Free(mac_objHdl);
    qapi_Crypto_Op_Free(compute_opHdl);
    qapi_Crypto_Op_Free(compare_opHdl);
    return 0;
}

int32_t crypto_hmacmd5()
{
    //"wmiconfig --cryptotest hmacmd5 "
    uint8_t msg[256] = {0};
    char *str = "Hi There";
    const uint8_t *key = (uint8_t[]){0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b};
    const uint8_t *hmac = (uint8_t[]){0x92, 0x94, 0x72, 0x7a, 0x36, 0x38, 0xbb, 0x1c, 0x13, 0xf4, 0x8e, 0xf8, 0x15, 0x8b, 0xfc, 0x9d};
    uint32_t msgLen = 8;
    uint32_t keyLen =16*8;
    memcpy(msg, str,msgLen);
    char mac[16] = {0};
    uint32_t macLen = 0;
    qapi_Crypto_Attrib_t attr[1];

    //memcpy(msg, argv[4], msgLen);

    qapi_Crypto_Obj_Hdl_t mac_objHdl;
    qapi_Crypto_Op_Hdl_t compute_opHdl;
    qapi_Crypto_Op_Hdl_t compare_opHdl;

    //printf("srcData len %d %s\n", srcLen, srcData);
    if (qapi_Crypto_Transient_Obj_Alloc(QAPI_CRYPTO_OBJ_TYPE_HMAC_MD5_E, QAPI_CRYPTO_HMAC_MD5_MAX_KEY_BITS, &mac_objHdl) != QAPI_OK) {
        CRYPTO_PRINTF("\n aes Failed to alloc transient obj\n");
        return -1;
    }
#if 0
    if (qapi_Crypto_Transient_Obj_Key_Gen(mac_objHdl, QAPI_CRYPTO_HMAC_SHA384_MAX_KEY_BITS, NULL, 0) != QAPI_OK) {
        CRYPTO_PRINTF("\naes Failed to gen key\n");
    }
#endif
    if (keyLen<QAPI_CRYPTO_HMAC_MD5_MIN_KEY_BITS || keyLen>QAPI_CRYPTO_HMAC_MD5_MAX_KEY_BITS){
        CRYPTO_PRINTF("\nUnvalid key length\n");
        return -1;
    }

    ref_attr_init(&attr[0], QAPI_CRYPTO_ATTR_SECRET_VALUE_E, (void*)key, keyLen/8);
    if (qapi_Crypto_Transient_Obj_Populate(mac_objHdl, &attr[0], 1) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to populate mac public obj\n");
        return -1;
    }

    if (qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_HMAC_MD5_E, QAPI_CRYPTO_MODE_MAC_E, QAPI_CRYPTO_HMAC_MD5_MAX_KEY_BITS, &compute_opHdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc op\n");
        return -1;
    }

    if (qapi_Crypto_Op_Key_Set(compute_opHdl, mac_objHdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to set key\n");
        return -1;
    }

    if (qapi_Crypto_Op_Mac_Init(compute_opHdl, NULL, 0) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to mac init\n");
        return -1;
    }

    macLen = sizeof(mac);
    if (qapi_Crypto_Op_Mac_Final_Compute(compute_opHdl, (void*)msg, msgLen, mac, &macLen) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to mac compute final\n");
        return -1;
    }
#if 0
    printf("hmacMD5 mac len %d :\n", macLen);
    int i;
    for (i=0; i<macLen; i++)
        printf("%02x, ", mac[i]);
    printf("\n ");
#endif
    if (qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_HMAC_MD5_E, QAPI_CRYPTO_MODE_MAC_E, QAPI_CRYPTO_HMAC_MD5_MAX_KEY_BITS, &compare_opHdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc op\n");
        return -1;
    }

    if (qapi_Crypto_Op_Key_Set(compare_opHdl, mac_objHdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to set key\n");
        return -1;
    }

    if (qapi_Crypto_Op_Mac_Init(compare_opHdl, NULL, 0) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to mac init\n");
        return -1;
    }
    if (qapi_Crypto_Op_Mac_Final_Compare(compare_opHdl, (void*)msg, msgLen, (void*)hmac, macLen) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to mac compare final\n");
        return -1;
    }
    else {
        CRYPTO_PRINTF("hmacmd5 compare success\n");
    }

    qapi_Crypto_Transient_Obj_Free(mac_objHdl);
    qapi_Crypto_Op_Free(compute_opHdl);
    qapi_Crypto_Op_Free(compare_opHdl);
    return 0;
}

int32_t crypto_sha1()
{
    //"wmiconfig --cryptotest sha1 "
    uint8_t msg[128] = {0};
    const uint8_t *digest = (uint8_t[]){0x60, 0x53, 0xD7, 0x61, 0x08, 0x4E, 0x9E, 0xB4, 0xEC, 0x12, 0x81, 0x01, 0x10, 0xDE, 0x07, 0xE7, 0x32, 0x07, 0x87, 0xB6};
    uint32_t msgLen = 127;
    char hash[32] = {0};
    uint32_t hashLen = 0;

    qapi_Crypto_Op_Hdl_t opHdl;

    if (qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_SHA1_E, QAPI_CRYPTO_MODE_DIGEST_E, 0, &opHdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc op\n");
    }

    if (qapi_Crypto_Op_Digest_Update(opHdl, msg, 32) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to digest update\n");
    }
    if (qapi_Crypto_Op_Digest_Update(opHdl, msg, 64) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to digest update\n");
    }

    if (qapi_Crypto_Op_Reset(opHdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to reset\n");
    }

    if (qapi_Crypto_Op_Digest_Update(opHdl, msg, 32) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to digest update\n");
    }
    if (qapi_Crypto_Op_Digest_Update(opHdl, msg, 64) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to digest update\n");
    }

    hashLen = sizeof(hash);
    if (qapi_Crypto_Op_Digest_Final(opHdl, (void*)msg, msgLen - 32-64, hash, &hashLen) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to digest dofinal\n");
    }

#if 0
    printf("sha1 digest len %d :\n", hashLen);
    int i;
    for (i=0; i<hashLen; i++)
        printf("%02x, ", hash[i]);
    printf("\n ");
#endif
    if (memcmp(hash, digest, QAPI_CRYPTO_SHA1_DIGEST_BYTES) == 0)
        CRYPTO_PRINTF("sha1 op sucess\n");
    qapi_Crypto_Op_Free(opHdl);
    return 0;
}

int32_t crypto_opcopy_sha1()
{
    //"wmiconfig --cryptotest opcopy_sha1 "
    uint8_t msg[128] = {0};
    const uint8_t *digest = (uint8_t[]){0x60, 0x53, 0xD7, 0x61, 0x08, 0x4E, 0x9E, 0xB4, 0xEC, 0x12, 0x81, 0x01, 0x10, 0xDE, 0x07, 0xE7, 0x32, 0x07, 0x87, 0xB6};
    const uint8_t *digest2= (uint8_t[]){0xC8, 0xD7, 0xD0, 0xEF, 0x0E, 0xED, 0xFA, 0x82, 0xD2, 0xEA, 0x1A, 0xA5, 0x92, 0x84, 0x5B, 0x9A, 0x6D, 0x4B, 0x02, 0xB7};
    uint32_t msgLen = 127;
    char hash[32] = {0};
    uint32_t hashLen = 0;

    qapi_Crypto_Op_Hdl_t src_Hdl;
    qapi_Crypto_Op_Hdl_t dst_Hdl;

    //CASE1: the total data size of qapi_Crypto_Op_Digest_Update is not a multiple of block size.
    if (qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_SHA1_E, QAPI_CRYPTO_MODE_DIGEST_E, 0, &src_Hdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc op\n");
    }

    if (qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_SHA1_E, QAPI_CRYPTO_MODE_DIGEST_E, 0, &dst_Hdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc op\n");
    }

    if (qapi_Crypto_Op_Digest_Update(src_Hdl, msg, 32) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to digest update\n");
    }

    if (qapi_Crypto_Op_Digest_Update(src_Hdl, msg, 64) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to digest update\n");
    }

    if (qapi_Crypto_Op_Digest_Update(src_Hdl, msg, msgLen - 32-64) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to digest update\n");
    }

    if (qapi_Crypto_Op_Copy(dst_Hdl, src_Hdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to op copy\n");
    }

    hashLen = sizeof(hash);
    if (qapi_Crypto_Op_Digest_Final(dst_Hdl, NULL, 0, hash, &hashLen) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to digest dofinal\n");
    }
#if 0
    printf("sha1 digest len %d :\n", hashLen);
    int i;
    for (i=0; i<hashLen; i++)
        printf("%02x, ", hash[i]);
    printf("\n ");
#endif

    if (memcmp(hash, digest, QAPI_CRYPTO_SHA1_DIGEST_BYTES) == 0)
        CRYPTO_PRINTF("sha1 op copy case 1 sucess\n");

    qapi_Crypto_Op_Free(src_Hdl);
    qapi_Crypto_Op_Free(dst_Hdl);

    //CASE2:the total data size of qapi_Crypto_Op_Digest_Update is a multiple of block size.
    memset(hash, 0, 32);
    if (qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_SHA1_E, QAPI_CRYPTO_MODE_DIGEST_E, 0, &src_Hdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc op\n");
    }

    if (qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_SHA1_E, QAPI_CRYPTO_MODE_DIGEST_E, 0, &dst_Hdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc op\n");
    }

    if (qapi_Crypto_Op_Digest_Update(src_Hdl, msg, 64) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to digest update\n");
    }

    if (qapi_Crypto_Op_Copy(dst_Hdl, src_Hdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to op copy\n");
    }

    hashLen = sizeof(hash);
    if (qapi_Crypto_Op_Digest_Final(dst_Hdl, NULL, 0, hash, &hashLen) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to digest dofinal\n");
    }
#if 0
    printf("sha1 digest len %d :\n", hashLen);
    int i;
    for (i=0; i<hashLen; i++)
        printf("%02x, ", hash[i]);
    printf("\n ");
#endif

    if (memcmp(hash, digest2, QAPI_CRYPTO_SHA1_DIGEST_BYTES) == 0)
        CRYPTO_PRINTF("sha1 op copy case 2 sucess\n");

    qapi_Crypto_Op_Free(src_Hdl);
    qapi_Crypto_Op_Free(dst_Hdl);
    return 0;
}

int32_t crypto_sha256()
{
    //"wmiconfig --cryptotest sha256 "
    uint8_t msg[128] = {0};
    const uint8_t *digest = (uint8_t[]){0x15, 0xDA, 0xE5, 0x97, 0x90, 0x58, 0xBF, 0xBF, 0x4F, 0x91, 0x66, 0x02, 0x9B, 0x6E, 0x34, 0x0E, 0xA3, 0xCA, 0x37, 0x4F, 0xEF, 0x57, 0x8A, 0x11, 0xDC, 0x9E, 0x6E, 0x92, 0x38, 0x60, 0xD7, 0xAE};
    uint32_t msgLen = 127;
    char hash[32] = {0};
    uint32_t hashLen = 0;

    qapi_Crypto_Op_Hdl_t opHdl;

    if (qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_SHA256_E, QAPI_CRYPTO_MODE_DIGEST_E, 0, &opHdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc op\n");
    }

    if (qapi_Crypto_Op_Digest_Update(opHdl, msg, 32) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to digest update\n");
    }
    if (qapi_Crypto_Op_Digest_Update(opHdl, msg, 64) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to digest update\n");
    }

    hashLen = sizeof(hash);
    if (qapi_Crypto_Op_Digest_Final(opHdl, (void*)msg, msgLen-32-64, hash, &hashLen) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to digest dofinal\n");
    }

#if 0
    printf("sha256 digest len %d :\n", hashLen);
    int i;
    for (i=0; i<hashLen; i++)
        printf("%02x, ", hash[i]);
    printf("\n ");
#endif
    if (memcmp(hash, digest, QAPI_CRYPTO_SHA256_DIGEST_BYTES) == 0)
        CRYPTO_PRINTF("sha256 op sucess\n");

    qapi_Crypto_Op_Free(opHdl);
    return 0;
}

int32_t crypto_opcopy_sha256()
{
    //"wmiconfig --cryptotest opcopy_sha256 "
    uint8_t msg[128] = {0};
    const uint8_t *digest= (uint8_t[]){0x15, 0xDA, 0xE5, 0x97, 0x90, 0x58, 0xBF, 0xBF, 0x4F, 0x91, 0x66, 0x02, 0x9B, 0x6E, 0x34, 0x0E, 0xA3, 0xCA, 0x37, 0x4F, 0xEF, 0x57, 0x8A, 0x11, 0xDC, 0x9E, 0x6E, 0x92, 0x38, 0x60, 0xD7, 0xAE};
    const uint8_t *digest2= (uint8_t[]){0xF5, 0xA5, 0xFD, 0x42, 0xD1, 0x6A, 0x20, 0x30, 0x27, 0x98, 0xEF, 0x6E, 0xD3, 0x09, 0x97, 0x9B, 0x43, 0x00, 0x3D, 0x23, 0x20, 0xD9, 0xF0, 0xE8, 0xEA, 0x98, 0x31, 0xA9, 0x27, 0x59, 0xFB, 0x4B};
    uint32_t msgLen = 127;
    char hash[32] = {0};
    uint32_t hashLen = 0;

    qapi_Crypto_Op_Hdl_t src_Hdl;
    qapi_Crypto_Op_Hdl_t dst_Hdl;

    //CASE1:the total data size of qapi_Crypto_Op_Digest_Update is not a multiple of block size.
    if (qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_SHA256_E, QAPI_CRYPTO_MODE_DIGEST_E, 0, &src_Hdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc op\n");
    }

    if (qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_SHA256_E, QAPI_CRYPTO_MODE_DIGEST_E, 0, &dst_Hdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc op\n");
    }

    if (qapi_Crypto_Op_Digest_Update(src_Hdl, msg, 32) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to digest update\n");
    }

    if (qapi_Crypto_Op_Digest_Update(src_Hdl, msg, 64) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to digest update\n");
    }

    if (qapi_Crypto_Op_Digest_Update(src_Hdl, msg, msgLen - 32-64) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to digest update\n");
    }

    if (qapi_Crypto_Op_Copy(dst_Hdl, src_Hdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to op copy\n");
    }

    hashLen = sizeof(hash);
    if (qapi_Crypto_Op_Digest_Final(dst_Hdl, NULL, 0, hash, &hashLen) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to digest dofinal\n");
    }
#if 0
    printf("sha256 digest len %d :\n", hashLen);
    int i;
    for (i=0; i<hashLen; i++)
        printf("%02x, ", hash[i]);
    printf("\n ");
#endif

    if (memcmp(hash, digest, QAPI_CRYPTO_SHA256_DIGEST_BYTES) == 0)
        CRYPTO_PRINTF("sha256 op copy case 1 sucess\n");

    qapi_Crypto_Op_Free(src_Hdl);
    qapi_Crypto_Op_Free(dst_Hdl);
    //CASE2:the total data size of qapi_Crypto_Op_Digest_Update is a multiple of block size.
    memset(hash, 0, 32);
    if (qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_SHA256_E, QAPI_CRYPTO_MODE_DIGEST_E, 0, &src_Hdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc op\n");
    }

    if (qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_SHA256_E, QAPI_CRYPTO_MODE_DIGEST_E, 0, &dst_Hdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc op\n");
    }

    if (qapi_Crypto_Op_Digest_Update(src_Hdl, msg, 64) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to digest update\n");
    }

    if (qapi_Crypto_Op_Copy(dst_Hdl, src_Hdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to op copy\n");
    }

    if (qapi_Crypto_Op_Digest_Final(dst_Hdl, NULL, 0, hash, &hashLen) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to digest dofinal\n");
    }
#if 0
    printf("sha256 digest len %d :\n", hashLen);
    int i;
    for (i=0; i<hashLen; i++)
        printf("%02x, ", hash[i]);
    printf("\n ");
#endif

    if (memcmp(hash, digest2, QAPI_CRYPTO_SHA256_DIGEST_BYTES) == 0)
        CRYPTO_PRINTF("sha256 op copy case 2 sucess\n");

    qapi_Crypto_Op_Free(src_Hdl);
    qapi_Crypto_Op_Free(dst_Hdl);
    return 0;
}

int32_t crypto_sha384()
{
    //"wmiconfig --cryptotest sha384 "
    uint8_t msg[256] = {0};
#if 0
    uint8_t digest[48] = {0x48, 0x57, 0x09, 0xA9, 0x1F, 0x98, 0x46, 0xD3, 0xEB, 0x98, 0x24, 0x33, 0x51, 0x06, 0x53, 0x2F,
        0xDE, 0x8B, 0xFD, 0x5D, 0x77, 0x04, 0x08, 0xE0, 0x89, 0x9D, 0x99, 0xDB, 0x5B, 0xF8, 0x5B, 0x79,
        0xBA, 0x04, 0x63, 0x0D, 0x1E, 0x6D, 0x13, 0xF9, 0xF3, 0x36, 0x91, 0x5D, 0xAF, 0x7C, 0x1D, 0x1D};
#endif
    uint8_t hash1[48] = {0};
    uint8_t hash2[48] = {0};
    uint32_t hashLen1 = 0;
    uint32_t hashLen2 = 0;

    uint32_t msgLen = 256;
    memset(msg, 0xa,msgLen);


    qapi_Crypto_Op_Hdl_t opHdl1;
    qapi_Crypto_Op_Hdl_t opHdl2;

    if (qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_SHA384_E, QAPI_CRYPTO_MODE_DIGEST_E, 0, &opHdl1) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc op\n");
    }

    hashLen1 = sizeof(hash1);
    if (qapi_Crypto_Op_Digest_Final(opHdl1, (void*)&msg, msgLen, hash1, &hashLen1) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to digest dofinal\n");
    }

    if (qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_SHA384_E, QAPI_CRYPTO_MODE_DIGEST_E, 0, &opHdl2) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc op\n");
    }

    if (qapi_Crypto_Op_Digest_Update(opHdl2, msg, 32) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to digest update\n");
    }

    if (qapi_Crypto_Op_Digest_Update(opHdl2, msg, 64) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to digest update\n");
    }

    if (qapi_Crypto_Op_Digest_Update(opHdl2, msg, 64) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to digest update\n");
    }

    hashLen2 = sizeof(hash2);
    if (qapi_Crypto_Op_Digest_Final(opHdl2, (void*)msg, msgLen - 64*2 -32, hash2, &hashLen2) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to digest dofinal\n");
    }

#if 0
    printf("sha384 hash1 len %d :\n", hashLen1);
    int i;
    for (i=0; i<hashLen1; i++)
        printf("%02x, ", hash1[i]);
    printf("\n ");

    printf("sha384 hash2 len %d :\n", hashLen2);

    for (i=0; i<hashLen2; i++)
        printf("%02x, ", hash2[i]);
    printf("\n ");
#endif
    if (memcmp(hash1, hash2, QAPI_CRYPTO_SHA384_DIGEST_BYTES) == 0)
        CRYPTO_PRINTF("sha384 op sucess\n");

    qapi_Crypto_Op_Free(opHdl1);
    qapi_Crypto_Op_Free(opHdl2);
    return 0;
}

int32_t crypto_opcopy_sha384()
{
    //"wmiconfig --cryptotest opcopy_384 "
    uint8_t msg[256] = {0};
    uint8_t dst_hash[48] = {0};
    uint8_t tmp_hash[48] = {0};
    uint32_t dst_hashLen = 0;
    uint32_t tmp_hashLen = 0;

    uint32_t msgLen = 256;
    memset(msg, 0xa,msgLen);

    qapi_Crypto_Op_Hdl_t src_Hdl;
    qapi_Crypto_Op_Hdl_t dst_Hdl;
    qapi_Crypto_Op_Hdl_t tmp_Hdl;

    if (qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_SHA384_E, QAPI_CRYPTO_MODE_DIGEST_E, 0, &tmp_Hdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc op\n");
    }

    tmp_hashLen = sizeof(tmp_hash);
    if (qapi_Crypto_Op_Digest_Final(tmp_Hdl, msg, msgLen, tmp_hash, &tmp_hashLen) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to digest dofinal\n");
    }

    if (qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_SHA384_E, QAPI_CRYPTO_MODE_DIGEST_E, 0, &src_Hdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc op\n");
    }

    if (qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_SHA384_E, QAPI_CRYPTO_MODE_DIGEST_E, 0, &dst_Hdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc op\n");
    }

    if (qapi_Crypto_Op_Digest_Update(src_Hdl, msg, 32) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to digest update\n");
    }

    if (qapi_Crypto_Op_Digest_Update(src_Hdl, msg, 64) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to digest update\n");
    }

    if (qapi_Crypto_Op_Digest_Update(src_Hdl, msg, 64) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to digest update\n");
    }

    if (qapi_Crypto_Op_Digest_Update(src_Hdl, msg, msgLen - 64*2 -32) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to digest update\n");
    }

    if (qapi_Crypto_Op_Copy(dst_Hdl, src_Hdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to op copy\n");
    }

    dst_hashLen = sizeof(dst_hash);
    if (qapi_Crypto_Op_Digest_Final(dst_Hdl, NULL, 0, dst_hash, &dst_hashLen) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to digest dofinal\n");
    }
#if 0
    printf("sha384 digest len %d :\n", dst_hashLen);
    int i;
    for (i=0; i<dst_hashLen; i++)
        printf("%02x, ", dst_hash[i]);
    printf("\n ");
#endif

    if (memcmp(dst_hash, tmp_hash, QAPI_CRYPTO_SHA384_DIGEST_BYTES) == 0){
        CRYPTO_PRINTF("sha384 op copy case 1 sucess\n");
    }
    qapi_Crypto_Op_Free(src_Hdl);
    qapi_Crypto_Op_Free(dst_Hdl);
    qapi_Crypto_Op_Free(tmp_Hdl);
    return 0;
}

int32_t crypto_sha512()
{
    //"wmiconfig --cryptotest sha512 "
    uint8_t msg[256] = {0};
#if 0
    uint8_t digest[48] = {0x48, 0x57, 0x09, 0xA9, 0x1F, 0x98, 0x46, 0xD3, 0xEB, 0x98, 0x24, 0x33, 0x51, 0x06, 0x53, 0x2F,
        0xDE, 0x8B, 0xFD, 0x5D, 0x77, 0x04, 0x08, 0xE0, 0x89, 0x9D, 0x99, 0xDB, 0x5B, 0xF8, 0x5B, 0x79,
        0xBA, 0x04, 0x63, 0x0D, 0x1E, 0x6D, 0x13, 0xF9, 0xF3, 0x36, 0x91, 0x5D, 0xAF, 0x7C, 0x1D, 0x1D};
#endif
    uint8_t hash1[64] = {0};
    uint8_t hash2[64] = {0};
    uint32_t hashLen1 = 0;
    uint32_t hashLen2 = 0;

    uint32_t msgLen = 256;
    memset(msg, 0xa,msgLen);


    qapi_Crypto_Op_Hdl_t opHdl1;
    qapi_Crypto_Op_Hdl_t opHdl2;

    if (qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_SHA512_E, QAPI_CRYPTO_MODE_DIGEST_E, 0, &opHdl1) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc op\n");
    }

    hashLen1 = sizeof(hash1);
    if (qapi_Crypto_Op_Digest_Final(opHdl1, (void*)&msg, msgLen, hash1, &hashLen1) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to digest dofinal\n");
    }

    if (qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_SHA512_E, QAPI_CRYPTO_MODE_DIGEST_E, 0, &opHdl2) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc op\n");
    }

    if (qapi_Crypto_Op_Digest_Update(opHdl2, msg, 32) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to digest update\n");
    }

    if (qapi_Crypto_Op_Digest_Update(opHdl2, msg, 64) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to digest update\n");
    }

    if (qapi_Crypto_Op_Digest_Update(opHdl2, msg, 64) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to digest update\n");
    }

    hashLen2 = sizeof(hash2);
    if (qapi_Crypto_Op_Digest_Final(opHdl2, (void*)msg, msgLen - 64*2 -32, hash2, &hashLen2) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to digest dofinal\n");
    }

#if 0
    printf("sha512 hash1 len %d :\n", hashLen1);
    int i;
    for (i=0; i<hashLen1; i++)
        printf("%02x, ", hash1[i]);
    printf("\n ");

    printf("sha512 hash2 len %d :\n", hashLen2);
    for (i=0; i<hashLen2; i++)
        printf("%02x, ", hash2[i]);
    printf("\n ");
#endif
    if (memcmp(hash1, hash2, QAPI_CRYPTO_SHA512_DIGEST_BYTES) == 0)
        CRYPTO_PRINTF("sha512 op sucess\n");

    qapi_Crypto_Op_Free(opHdl1);
    qapi_Crypto_Op_Free(opHdl2);
    return 0;
}

int32_t crypto_opcopy_sha512()
{
    //"wmiconfig --cryptotest opcopy_sha512"
    uint8_t msg[256] = {0};
    uint8_t dst_hash[64] = {0};
    uint8_t tmp_hash[64] = {0};
    uint32_t dst_hashLen = 0;
    uint32_t tmp_hashLen = 0;

    uint32_t msgLen = 256;
    memset(msg, 0xa,msgLen);

    qapi_Crypto_Op_Hdl_t src_Hdl;
    qapi_Crypto_Op_Hdl_t dst_Hdl;
    qapi_Crypto_Op_Hdl_t tmp_Hdl;

    if (qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_SHA512_E, QAPI_CRYPTO_MODE_DIGEST_E, 0, &tmp_Hdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc op\n");
    }

    tmp_hashLen = sizeof(tmp_hash);
    if (qapi_Crypto_Op_Digest_Final(tmp_Hdl, msg, msgLen, tmp_hash, &tmp_hashLen) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to digest dofinal\n");
    }

    //CASE1:
    if (qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_SHA512_E, QAPI_CRYPTO_MODE_DIGEST_E, 0, &src_Hdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc op\n");
    }

    if (qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_SHA512_E, QAPI_CRYPTO_MODE_DIGEST_E, 0, &dst_Hdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc op\n");
    }

    if (qapi_Crypto_Op_Digest_Update(src_Hdl, msg, 32) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to digest update\n");
    }

    if (qapi_Crypto_Op_Digest_Update(src_Hdl, msg, 64) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to digest update\n");
    }

    if (qapi_Crypto_Op_Digest_Update(src_Hdl, msg, 64) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to digest update\n");
    }

    if (qapi_Crypto_Op_Digest_Update(src_Hdl, msg, msgLen - 64*2 -32) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to digest update\n");
    }

    if (qapi_Crypto_Op_Copy(dst_Hdl, src_Hdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to op copy\n");
    }

    dst_hashLen = sizeof(dst_hash);
    if (qapi_Crypto_Op_Digest_Final(dst_Hdl, NULL, 0, dst_hash, &dst_hashLen) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to digest dofinal\n");
    }
#if 0
    printf("sha512 digest len %d :\n", hashLen);
    int i;
    for (i=0; i<hashLen; i++)
        printf("%02x, ", hash[i]);
    printf("\n ");
#endif

    if (memcmp(dst_hash, tmp_hash, QAPI_CRYPTO_SHA512_DIGEST_BYTES) == 0){
        CRYPTO_PRINTF("sha512 op copy case 1 sucess\n");
    }
    qapi_Crypto_Op_Free(src_Hdl);
    qapi_Crypto_Op_Free(dst_Hdl);
    qapi_Crypto_Op_Free(tmp_Hdl);
    return 0;
}

int32_t crypto_md5()
{
    //"wmiconfig --cryptotest md5 "
    uint8_t msg[256] = {0};
    uint8_t hash1[16] = {0};
    uint8_t hash2[16] = {0};
    uint32_t hashLen1 = 0;
    uint32_t hashLen2 = 0;

    uint32_t msgLen = 256;
    memset(msg, 0xa,msgLen);


    qapi_Crypto_Op_Hdl_t opHdl1;
    qapi_Crypto_Op_Hdl_t opHdl2;

    if (qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_MD5_E, QAPI_CRYPTO_MODE_DIGEST_E, 0, &opHdl1) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc op\n");
    }

    hashLen1 = sizeof(hash1);
    if (qapi_Crypto_Op_Digest_Final(opHdl1, (void*)&msg, msgLen, hash1, &hashLen1) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to digest dofinal\n");
    }

    if (qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_MD5_E, QAPI_CRYPTO_MODE_DIGEST_E, 0, &opHdl2) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc op\n");
    }

    if (qapi_Crypto_Op_Digest_Update(opHdl2, msg, 32) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to digest update\n");
    }

    if (qapi_Crypto_Op_Digest_Update(opHdl2, msg, 64) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to digest update\n");
    }

    if (qapi_Crypto_Op_Digest_Update(opHdl2, msg, 64) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to digest update\n");
    }

    hashLen2 = sizeof(hash2);
    if (qapi_Crypto_Op_Digest_Final(opHdl2, (void*)msg, msgLen - 64*2 -32, hash2, &hashLen2) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to digest dofinal\n");
    }

#if 0
    printf("md5 hash1 len %d :\n", hashLen1);
    int i;
    for (i=0; i<hashLen1; i++)
        printf("%02x, ", hash1[i]);
    printf("\n ");

    printf("md5 hash2 len %d :\n", hashLen2);
    for (i=0; i<hashLen2; i++)
        printf("%02x, ", hash2[i]);
    printf("\n ");
#endif
    if (memcmp(hash1, hash2, QAPI_CRYPTO_MD5_DIGEST_BYTES) == 0)
        CRYPTO_PRINTF("md5 op sucess\n");

    qapi_Crypto_Op_Free(opHdl1);
    qapi_Crypto_Op_Free(opHdl2);
    return 0;
}

int32_t crypto_opcopy_md5()
{
    //"wmiconfig --cryptotest opcopy_md5 "
    uint8_t msg[256] = {0};
    uint8_t dst_hash[16] = {0};
    uint8_t tmp_hash[16] = {0};
    uint32_t dst_hashLen = 0;
    uint32_t tmp_hashLen = 0;

    uint32_t msgLen = 256;
    memset(msg, 0xa,msgLen);

    qapi_Crypto_Op_Hdl_t src_Hdl;
    qapi_Crypto_Op_Hdl_t dst_Hdl;
    qapi_Crypto_Op_Hdl_t tmp_Hdl;

    if (qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_MD5_E, QAPI_CRYPTO_MODE_DIGEST_E, 0, &tmp_Hdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc op\n");
    }

    tmp_hashLen = sizeof(tmp_hash);
    if (qapi_Crypto_Op_Digest_Final(tmp_Hdl, msg, msgLen, tmp_hash, &tmp_hashLen) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to digest dofinal\n");
    }

    //CASE1:
    if (qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_MD5_E, QAPI_CRYPTO_MODE_DIGEST_E, 0, &src_Hdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc op\n");
    }

    if (qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_MD5_E, QAPI_CRYPTO_MODE_DIGEST_E, 0, &dst_Hdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc op\n");
    }

    if (qapi_Crypto_Op_Digest_Update(src_Hdl, msg, 32) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to digest update\n");
    }

    if (qapi_Crypto_Op_Digest_Update(src_Hdl, msg, 64) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to digest update\n");
    }

    if (qapi_Crypto_Op_Digest_Update(src_Hdl, msg, 64) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to digest update\n");
    }

    if (qapi_Crypto_Op_Digest_Update(src_Hdl, msg, msgLen - 64*2 -32) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to digest update\n");
    }

    if (qapi_Crypto_Op_Copy(dst_Hdl, src_Hdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to op copy\n");
    }

    dst_hashLen = sizeof(dst_hash);
    if (qapi_Crypto_Op_Digest_Final(dst_Hdl, NULL, 0, dst_hash, &dst_hashLen) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to digest dofinal\n");
    }
#if 0
    printf("md5 digest len %d :\n", hashLen);
    int i;
    for (i=0; i<hashLen; i++)
        printf("%02x, ", hash[i]);
    printf("\n ");
#endif

    if (memcmp(dst_hash, tmp_hash, QAPI_CRYPTO_MD5_DIGEST_BYTES) == 0){
        CRYPTO_PRINTF("md5 op copy case 1 sucess\n");
    }
    qapi_Crypto_Op_Free(src_Hdl);
    qapi_Crypto_Op_Free(dst_Hdl);
    qapi_Crypto_Op_Free(tmp_Hdl);
    return 0;
}

int32_t crypto_rsav15(int32_t argc, char* argv[])
{
    //wmiconfig --cryptotest rsav15 xxx    xxx:input data
    qapi_Crypto_Obj_Hdl_t obj_hdl;
    qapi_Crypto_Op_Hdl_t enc_hdl, dec_hdl;
    qapi_Crypto_Attrib_t attr[QAPI_CRYPTO_OBJ_ATTRIB_COUNT_RSA_KEYPAIR];
    uint32_t rsa_key_size = 1024;
    uint8_t* encData;;
    uint8_t* decData;;
    uint8_t* originalData;;
    uint8_t* temp;;
    uint32_t originalLen, encLen, decLen;

    uint8_t* data = malloc(512*3 +256);
    if (data == NULL){
        CRYPTO_PRINTF("\nFailed to alloc memory\n");
        return -1;
    }
    encData = data;
    decData = encData +512;
    originalData = decData +512;
    temp = originalData +512;

    originalLen = strlen(argv[3]);
    memcpy(temp, argv[3], originalLen);
    memcpy(originalData, argv[3], originalLen);

    if (qapi_Crypto_Transient_Obj_Alloc(QAPI_CRYPTO_OBJ_TYPE_RSA_KEYPAIR_E, rsa_key_size,
                &obj_hdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc transient obj\n");
    }

    ref_attr_init(&attr[0], QAPI_CRYPTO_ATTR_RSA_MODULUS_E, rsa_mod, sizeof(rsa_mod));
    ref_attr_init(&attr[1], QAPI_CRYPTO_ATTR_RSA_PUBLIC_EXPONENT_E, rsa_pub_exp, sizeof(rsa_pub_exp));
    ref_attr_init(&attr[2], QAPI_CRYPTO_ATTR_RSA_PRIVATE_EXPONENT_E, rsa_pvt_exp, sizeof(rsa_pvt_exp));
    ref_attr_init(&attr[3], QAPI_CRYPTO_ATTR_RSA_PRIME1_E, rsa_prime1, sizeof(rsa_prime1));
    ref_attr_init(&attr[4], QAPI_CRYPTO_ATTR_RSA_PRIME2_E, rsa_prime2, sizeof(rsa_prime2));
    ref_attr_init(&attr[5], QAPI_CRYPTO_ATTR_RSA_EXPONENT1_E, rsa_exp1, sizeof(rsa_exp1));
    ref_attr_init(&attr[6], QAPI_CRYPTO_ATTR_RSA_EXPONENT2_E, rsa_exp2, sizeof(rsa_exp2));
    ref_attr_init(&attr[7], QAPI_CRYPTO_ATTR_RSA_COEFFICIENT_E, rsa_coeff, sizeof(rsa_coeff));

    if (qapi_Crypto_Transient_Obj_Populate(obj_hdl, attr, QAPI_CRYPTO_OBJ_ATTRIB_COUNT_RSA_KEYPAIR) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to populate ecc keypair obj\n");
    }
    /* Allocate encrypt operation */
    if (qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_RSAES_PKCS1_V1_5_E, QAPI_CRYPTO_MODE_ENCRYPT_E, rsa_key_size,
                &enc_hdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc encrypt op\n");
    }

    /* Copy public key from keypair object for encrypt operation */
    if (qapi_Crypto_Op_Key_Set(enc_hdl, obj_hdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to set key\n");
    }

    /* Allocate decrypt operation */
    if (qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_RSAES_PKCS1_V1_5_E, QAPI_CRYPTO_MODE_DECRYPT_E, rsa_key_size, &dec_hdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc decrypt op\n");
    }

    /* Copy private key from keypair object for decrypt operation */
    if (qapi_Crypto_Op_Key_Set(dec_hdl, obj_hdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to set key\n");
    }

    /*encrypt */
    if (qapi_Crypto_Op_Asym_Encrypt(enc_hdl, NULL, 0, originalData, originalLen, encData, &encLen) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to encrypt\n");
    }

    /* decrypt */
    if (qapi_Crypto_Op_Asym_Decrypt(dec_hdl, NULL, 0, encData, encLen, decData, &decLen) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed decrypt\n");
    }

    if ((originalLen == decLen) && (memcmp(temp, decData, decLen) == 0)){
        CRYPTO_PRINTF("\nQAPI_CRYPTO_ALG_RSAES_PKCS1_V1_5_E encrypt/decrypt Successful\n");
    }
    free(data);
    qapi_Crypto_Op_Free(enc_hdl);
    qapi_Crypto_Op_Free(dec_hdl);
    qapi_Crypto_Transient_Obj_Free(obj_hdl);
    return 0;
}

int32_t crypto_rsanopad()
{
    //wmiconfig --cryptotest rsanopad
    qapi_Crypto_Obj_Hdl_t obj_hdl;
    qapi_Crypto_Op_Hdl_t enc_hdl, dec_hdl;
    qapi_Crypto_Attrib_t attr[QAPI_CRYPTO_OBJ_ATTRIB_COUNT_RSA_KEYPAIR];
    uint32_t rsa_key_size = 1024;
    uint8_t* encData;
    uint8_t* decData;
    uint8_t* originalData;
    uint8_t* temp;
    uint32_t originalLen, encLen, decLen;

    uint8_t* data = malloc(512*3 +256);
    if (data == NULL){
        CRYPTO_PRINTF("\nFailed to alloc memory\n");
        return -1;
    }
    encData = data;
    decData = encData +512;
    originalData = decData +512;
    temp = originalData +512;

    originalLen = rsa_key_size/8;
    if (qapi_Crypto_Random_Get(temp, originalLen) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to get rng\n");
    }

    if (temp[0] >= 0xa7) { //the plain text must be less than rsa_mod, this change just for test.
        temp[0] = 0xa6;
    }

    memcpy(originalData, temp, originalLen);
    if (qapi_Crypto_Transient_Obj_Alloc(QAPI_CRYPTO_OBJ_TYPE_RSA_KEYPAIR_E, rsa_key_size,
                &obj_hdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc transient obj\n");
    }

    ref_attr_init(&attr[0], QAPI_CRYPTO_ATTR_RSA_MODULUS_E, rsa_mod, sizeof(rsa_mod));
    ref_attr_init(&attr[1], QAPI_CRYPTO_ATTR_RSA_PUBLIC_EXPONENT_E, rsa_pub_exp, sizeof(rsa_pub_exp));
    ref_attr_init(&attr[2], QAPI_CRYPTO_ATTR_RSA_PRIVATE_EXPONENT_E, rsa_pvt_exp, sizeof(rsa_pvt_exp));
    ref_attr_init(&attr[3], QAPI_CRYPTO_ATTR_RSA_PRIME1_E, rsa_prime1, sizeof(rsa_prime1));
    ref_attr_init(&attr[4], QAPI_CRYPTO_ATTR_RSA_PRIME2_E, rsa_prime2, sizeof(rsa_prime2));
    ref_attr_init(&attr[5], QAPI_CRYPTO_ATTR_RSA_EXPONENT1_E, rsa_exp1, sizeof(rsa_exp1));
    ref_attr_init(&attr[6], QAPI_CRYPTO_ATTR_RSA_EXPONENT2_E, rsa_exp2, sizeof(rsa_exp2));
    ref_attr_init(&attr[7], QAPI_CRYPTO_ATTR_RSA_COEFFICIENT_E, rsa_coeff, sizeof(rsa_coeff));

    if (qapi_Crypto_Transient_Obj_Populate(obj_hdl, attr, QAPI_CRYPTO_OBJ_ATTRIB_COUNT_RSA_KEYPAIR) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to populate ecc keypair obj\n");
    }
    /* Allocate encrypt operation */
    if (qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_RSA_NOPAD_E, QAPI_CRYPTO_MODE_ENCRYPT_E, rsa_key_size,
                &enc_hdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc encrypt op\n");
    }

    /* Copy public key from keypair object for encrypt operation */
    if (qapi_Crypto_Op_Key_Set(enc_hdl, obj_hdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to set key\n");
    }

    /* Allocate decrypt operation */
    if (qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_RSA_NOPAD_E, QAPI_CRYPTO_MODE_DECRYPT_E, rsa_key_size, &dec_hdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc decrypt op\n");
    }

    /* Copy private key from keypair object for decrypt operation */
    if (qapi_Crypto_Op_Key_Set(dec_hdl, obj_hdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to set key\n");
    }

    /*encrypt */
    if (qapi_Crypto_Op_Asym_Encrypt(enc_hdl, NULL, 0, originalData, originalLen, encData, &encLen) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to encrypt\n");
    }

    /* decrypt */
    if (qapi_Crypto_Op_Asym_Decrypt(dec_hdl, NULL, 0, encData, encLen, decData, &decLen) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed decrypt\n");
    }

    if ((originalLen == decLen) && (memcmp(temp, decData, decLen) == 0)){
        CRYPTO_PRINTF("\nQAPI_CRYPTO_ALG_RSA_NOPAD_E encrypt/decrypt Successful\n");
    }

    free(data);
    qapi_Crypto_Op_Free(enc_hdl);
    qapi_Crypto_Op_Free(dec_hdl);
    qapi_Crypto_Transient_Obj_Free(obj_hdl);
    return 0;
}

int32_t getrng_demo()
{
#ifdef ENABLE_RNG_DEMO
    uint32_t rng_length;
    unsigned char rng_buffer[crypto_rng_RANDOMBYTES];
    int i;

    rng_length = sizeof(rng_buffer);
    memset(rng_buffer, 0, rng_length);

    if (qapi_Crypto_Random_Get(rng_buffer, rng_length) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to retrieve %d random bytes\n", rng_length);
    }
    else {
        CRYPTO_PRINTF("Successfully retrieved %d random bytes\n", rng_length);
    }

    CRYPTO_PRINTF("[");
    for (i = 0; i < rng_length; i++) {
        if (i%10 == 0)
            CRYPTO_PRINTF("\n");
        CRYPTO_PRINTF("%d, ", rng_buffer[i]);
    }
    CRYPTO_PRINTF("]\n");
#endif //ENABLE_RNG_DEMO
    return 0;
}

int32_t cryptolib_unit_test(int32_t argc, char* argv[])
{
	if (argc < 3) {
        return -1;
	}

    if(strcmp(argv[2], "all") == 0) {
        curve_demo();
        ed25519_demo();
        rsa_demo();
        dh_demo();
        ecdh_demo();
        ecdsa_demo();
        crypto_sha1();
        crypto_sha256();
        crypto_sha384();
        crypto_sha512();
        crypto_md5();

        crypto_opcopy_sha1();
        crypto_opcopy_sha256();
        crypto_opcopy_sha384();
        crypto_opcopy_sha512();
        crypto_opcopy_md5();

        crypto_hmacsha256();
        crypto_hmacsha1(argc, argv);
        crypto_hmacsha512();
        crypto_hmacsha384();
        crypto_hmacmd5();

        srp_demo();
        crypto_aescbc(argc, argv);
        crypto_aesctr(argc, argv);
        crypto_aesccm();
        crypto_aesgcm(argc, argv);
        crypto_chacha20_poly1305(argc, argv);
        return 0;
    }

    if(strcmp(argv[2], "curve") == 0) {
      curve_demo();
    }

    else if(strcmp(argv[2], "ed") == 0) {
      ed25519_demo();
    }

    else if(strcmp(argv[2], "rsa") == 0) {
        rsa_demo();
        crypto_rsav15(argc, argv);
        crypto_rsanopad();
    }

#if ENABLE_DH_DEMO
    else if(strcmp(argv[2], "dh") == 0) {
      dh_demo();
    }
#endif

    else if(strcmp(argv[2], "ecdh") == 0) {
      ecdh_demo();
    }

    else if(strcmp(argv[2], "ecdsa") == 0) {
       ecdsa_demo();
    }

    else if (strcmp(argv[2], "digest")==0){
        crypto_sha1();
        crypto_sha256();
        crypto_sha384();
        crypto_sha512();
        crypto_md5();

        crypto_opcopy_sha1();
        crypto_opcopy_sha256();
        crypto_opcopy_sha384();
        crypto_opcopy_sha512();
        crypto_opcopy_md5();
    }
    else if (strcmp(argv[2], "sha1")==0){
        crypto_sha1();
    }
    else if (strcmp(argv[2], "sha256")==0){
        crypto_sha256();
    }
    else if (strcmp(argv[2], "sha384")==0){
        crypto_sha384();
    }
    else if (strcmp(argv[2], "sha512")==0){
        crypto_sha512();
    }
    else if (strcmp(argv[2], "md5")==0){
        crypto_md5();
    }
    else if (strcmp(argv[2], "hmac")==0){
        crypto_hmacsha256();
        crypto_hmacsha1(argc, argv);
        crypto_hmacsha512();
        crypto_hmacsha384();
        crypto_hmacmd5();
    }
    else if (strcmp(argv[2], "hmacsha256")==0){
        crypto_hmacsha256();
            }
    else if (strcmp(argv[2], "hmacsha1")==0){
        crypto_hmacsha1(argc, argv);
    }
    else if (strcmp(argv[2], "hmacsha512")==0){
        crypto_hmacsha512();
    }
    else if (strcmp(argv[2], "hmacsha384")==0){
        crypto_hmacsha384();
    }
    else if (strcmp(argv[2], "hmacmd5")==0){
        crypto_hmacmd5();
    }
    else if(strcmp(argv[2], "srp") == 0) {
        srp_demo();
    }
    else if(strcmp(argv[2], "aes") == 0) {
        crypto_aescbc(argc, argv);
        crypto_aesctr(argc, argv);
        crypto_aesccm();
        crypto_aesgcm(argc, argv);
        crypto_chacha20_poly1305(argc, argv);
    }
    else if(strcmp(argv[2], "aescbc") == 0) {
        crypto_aescbc(argc, argv);
    }
    else if(strcmp(argv[2], "aesctr") == 0){
        crypto_aesctr(argc, argv);
    }
    else if(strcmp(argv[2], "ccm") == 0) {
        crypto_aesccm();
    }
    else if (strcmp(argv[2], "gcm") == 0){
        crypto_aesgcm(argc, argv);
    }
    else if (strcmp(argv[2], "cha") == 0) {
        crypto_chacha20_poly1305(argc, argv);
    }
#if CRYPTO_CONFIG_3DES
    else if((strcmp(argv[2], "des") == 0) || (strcmp(argv[2], "3des") == 0)){
        crypto_descbc(argc, argv);
    }
#endif
    else if (strcmp(argv[2], "rsav15") == 0){
        crypto_rsav15(argc, argv);
    }
    else if (strcmp(argv[2], "rsanopad") == 0){
        crypto_rsanopad();
    }

#if 0
    if(strcmp(argv[2], "getrng") == 0) {
      getrng_demo();
    }

#endif
    return 0;

}


int crypto_qapi_ed25519_unittest()
{
    #define crypto_sign_ed25519_BYTES 64U
    #define crypto_sign_ed25519_SEEDBYTES 32U
    #define crypto_sign_ed25519_PUBLICKEYBYTES 32U
    #define crypto_sign_ed25519_SECRETKEYBYTES (32U + 32U)

    int status;

    const uint32_t key_id = 0;

    uint8_t public_key[QAPI_CRYPTO_ED25519_PUBLIC_KEY_BYTES];
    memset(public_key, 0, sizeof(public_key));

    size_t public_key_size = sizeof(public_key) - 1;
    status =  qapi_Ed25519_Generate_Key_Pair(key_id, public_key, &public_key_size);
    if ( 0 == status ) {
        QCLI_Printf(qcli_crypto_handle, "Specified short public key size, but qapi_Ed25519_Generate_Key_Pair returned success\r\n");
        return -1;
    }

    public_key_size = sizeof(public_key)+1;
    status =  qapi_Ed25519_Generate_Key_Pair(key_id, public_key, &public_key_size);
    if ( 0 != status ) {
        QCLI_Printf(qcli_crypto_handle, "Failed on a call to qapi_Ed25519_Generate_Key_Pair\r\n");
        return -2;
    }

    size_t message_size = 100;
    uint8_t * message = (uint8_t *) malloc(message_size);
    if ( 0 == message ) {
        QCLI_Printf(qcli_crypto_handle, "Failed to allocate message\r\n");
        return -3;
    }
    memset(message, 0, message_size);
    int i;
    for ( i = 0; i < message_size; i++ ) {
        message[i] = i;
    }


    uint8_t signature[crypto_sign_ed25519_BYTES];
    memset(signature, 0, sizeof(signature));

    size_t signature_size = sizeof(signature);
    status = qapi_Ed25519_Sign(key_id, message, message_size, signature, &signature_size);
    if ( 0 != status ) {
        QCLI_Printf(qcli_crypto_handle, "Failed on a call to qapi_Ed25519_Sign\r\n");
        goto crypto_qapi_ed25519_unittest_cleanup;
    }



    qapi_Crypto_Obj_Hdl_t obj_hdl;
    if (qapi_Crypto_Transient_Obj_Alloc(QAPI_CRYPTO_OBJ_TYPE_ED25519_PUBLIC_KEY_E, QAPI_CRYPTO_ED25519_PUBLIC_KEY_BITS, &obj_hdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc transient obj\n");
        goto crypto_qapi_ed25519_unittest_cleanup;
    }


    qapi_Crypto_Attrib_t attr;
    qapi_Crypto_Op_Hdl_t verify_hdl;

    ref_attr_init(&attr, QAPI_CRYPTO_ATTR_ED25519_PUBLIC_VALUE_E, public_key, QAPI_CRYPTO_ED25519_PUBLIC_KEY_BYTES);

    if (qapi_Crypto_Transient_Obj_Populate(obj_hdl, &attr, 1) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to populate obj\n");
        goto crypto_qapi_ed25519_unittest_cleanup;
    }

    if (qapi_Crypto_Op_Alloc(QAPI_CRYPTO_ALG_ED25519_E, QAPI_CRYPTO_MODE_VERIFY_E, QAPI_CRYPTO_ED25519_PUBLIC_KEY_BITS, &verify_hdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to alloc op\n");
        goto crypto_qapi_ed25519_unittest_cleanup;
    }

    if (qapi_Crypto_Op_Key_Set(verify_hdl, obj_hdl) != QAPI_OK) {
        CRYPTO_PRINTF("\nFailed to set key\n");
        goto crypto_qapi_ed25519_unittest_cleanup;
    }

    if (qapi_Crypto_Op_Verify_Digest(verify_hdl, NULL, 0, message, message_size, signature, signature_size) != QAPI_OK) {
        CRYPTO_PRINTF("qapi_Crypto_Op_Verify_Digest() failure\r\n");
        goto crypto_qapi_ed25519_unittest_cleanup;
    }

    qapi_Crypto_Transient_Obj_Free(obj_hdl);
    qapi_Crypto_Op_Free(verify_hdl);

    status =  qapi_Ed25519_Reset_Key(key_id);
    if ( 0 != status ) {
        CRYPTO_PRINTF("Failed on a call to qapi_Ed25519_Reset_Key\r\n");
        goto crypto_qapi_ed25519_unittest_cleanup;
    }

    CRYPTO_PRINTF("qapi_Ed25519: Success\r\n");

crypto_qapi_ed25519_unittest_cleanup:
    if ( message ) {
        free(message);
    }
    return status;
}
#endif

#ifdef CONFIG_CRYPTO_BASE64_DEMO
int crypto_demo_base64_random_unittest()
{
    int status = 0;

    size_t plaintext_buffer_size = 0;
    uint8_t * p_plaintext_buffer = NULL;

    size_t ciphertext_buffer_size = 0;
    char * p_ciphertext_buffer = NULL;

    size_t decoded_buffer_size = 0;
    uint8_t * p_decoded_buffer = NULL;

    size_t bytes_decoded = 0;


    plaintext_buffer_size = get_pseudo_random_number(1, 32);
    p_plaintext_buffer = (uint8_t *) malloc(plaintext_buffer_size);
    if ( !p_plaintext_buffer ) {
        status = -1;
        goto crypto_demo_base64_random_unittest_cleanup;
    }
    memset(p_plaintext_buffer, 0xaa, plaintext_buffer_size);

    status = qapi_Crypto_Random_Get(p_plaintext_buffer, plaintext_buffer_size);
    if ( 0 != status ) {
        status = -2;
        goto crypto_demo_base64_random_unittest_cleanup;
    }

    ciphertext_buffer_size = qapi_Net_Get_Base64_Encoded_Output_Size(plaintext_buffer_size);
    p_ciphertext_buffer = (char *) malloc(ciphertext_buffer_size);
    if ( !p_ciphertext_buffer ) {
        status = -3;
        goto crypto_demo_base64_random_unittest_cleanup;
    }
    memset(p_ciphertext_buffer, 0xbb, ciphertext_buffer_size);

    status = qapi_Net_Base64_Encode(p_plaintext_buffer, plaintext_buffer_size, p_ciphertext_buffer, ciphertext_buffer_size);
    if ( 0 != status ) {
        status = -4;
        goto crypto_demo_base64_random_unittest_cleanup;
    }

    decoded_buffer_size = qapi_Net_Get_Base64_Decoded_Output_Size(ciphertext_buffer_size);
    p_decoded_buffer = (uint8_t *) malloc(decoded_buffer_size);
    if ( !p_decoded_buffer ) {
        status = -5;
        goto crypto_demo_base64_random_unittest_cleanup;
    }
    memset(p_decoded_buffer, 0xcc, decoded_buffer_size);

    bytes_decoded = qapi_Net_Base64_Decode(p_ciphertext_buffer, ciphertext_buffer_size, p_decoded_buffer, decoded_buffer_size);
    if ( bytes_decoded != plaintext_buffer_size ) {
        QCLI_Printf(qcli_crypto_handle, "crypto_demo_base64_random_unittest: decoded %d bytes, but original text is %d bytes\r\n", bytes_decoded, plaintext_buffer_size);
        status = -6;
        goto crypto_demo_base64_random_unittest_cleanup;
    }

    status = memcmp(p_decoded_buffer, p_plaintext_buffer, plaintext_buffer_size);
    if ( 0 != status ) {
        QCLI_Printf(qcli_crypto_handle, "crypto_demo_base64_random_unittest: string decoded incorrectly\r\n");
        status = -7;
        goto crypto_demo_base64_random_unittest_cleanup;
    }

    QCLI_Printf(qcli_crypto_handle, "crypto_demo_base64_random_unittest: PASSED: plaintext_buffer_size=%d\r\n", plaintext_buffer_size);


crypto_demo_base64_random_unittest_cleanup:
    if ( p_plaintext_buffer ) {
        if ( 0 != status ) {
            int i;
            QCLI_Printf(qcli_crypto_handle, "p_plaintext_buffer (%d bytes long):\r\n", plaintext_buffer_size);
            for ( i = 0; i < plaintext_buffer_size; i++ ) {
                QCLI_Printf(qcli_crypto_handle, "%02x", p_plaintext_buffer[i]);
            }
            QCLI_Printf(qcli_crypto_handle, "\r\n");
        }
        free(p_plaintext_buffer);
        p_plaintext_buffer = 0;
    }
    if ( p_ciphertext_buffer ) {
        if ( 0 != status ) {
            int i;
            QCLI_Printf(qcli_crypto_handle, "p_ciphertext_buffer (%d bytes long):\r\n", ciphertext_buffer_size);
            for ( i = 0; i < ciphertext_buffer_size; i++ ) {
                QCLI_Printf(qcli_crypto_handle, "%02x", p_ciphertext_buffer[i]);
            }
            QCLI_Printf(qcli_crypto_handle, "\r\n");
        }
        free(p_ciphertext_buffer);
        p_ciphertext_buffer = 0;
    }
    if ( p_decoded_buffer ) {
        if ( 0 != status ) {
            int i;
            QCLI_Printf(qcli_crypto_handle, "p_decoded_buffer (%d bytes long):\r\n", bytes_decoded);
            for ( i = 0; i < bytes_decoded; i++ ) {
                QCLI_Printf(qcli_crypto_handle, "%02x", p_decoded_buffer[i]);
            }
            QCLI_Printf(qcli_crypto_handle, "\r\n");
        }
        free(p_decoded_buffer);
        p_decoded_buffer = 0;
    }

    return status;
}

QCLI_Command_Status_t crypto_base64_unittests(uint32_t parameters_count, QCLI_Parameter_t * parameters)
{
    size_t iterations_count = 1;
    if ( 1 &&
        (1 == parameters_count) &&
        parameters[0].Integer_Is_Valid &&
        (parameters[0].Integer_Value > 0)
       )
    {
        iterations_count = parameters[0].Integer_Value;
    }

    int i;
    for ( i = 0; i < iterations_count; i++ ) {
        crypto_demo_base64_random_unittest();
    }

    return QCLI_STATUS_SUCCESS_E;
}

#endif

#ifdef CONFIG_CRYPTO_UNIT_TEST_DEMO
QCLI_Command_Status_t crypto_demo_unittest(uint32_t parameters_count, QCLI_Parameter_t * parameters);
#endif
#ifdef CONFIG_CRYPTO_BASE64_DEMO
QCLI_Command_Status_t crypto_base64_unittests(uint32_t parameters_count, QCLI_Parameter_t * parameters);
#endif
#ifdef CONFIG_CRYPTO_PERSISTENT_OBJECT_DEMO
QCLI_Command_Status_t crypto_demo_persistent_objects(uint32_t parameters_count, QCLI_Parameter_t * parameters);
#endif
#ifdef CONFIG_CRYPTO_ATTESTATION_DEMO
QCLI_Command_Status_t crypto_demo_attestation(uint32_t parameters_count, QCLI_Parameter_t * parameters);
#endif

const QCLI_Command_t crypto_cmd_list[] =
{
#ifdef CONFIG_CRYPTO_UNIT_TEST_DEMO
    {crypto_demo_unittest, false, "unittest", "all\n", "run all crypto unit tests\n"},
#endif
#ifdef CONFIG_CRYPTO_BASE64_DEMO
    {crypto_base64_unittests, false, "base64", "number_of_iterations\n", "run base64 random unit tests\n"},
#endif
#ifdef CONFIG_CRYPTO_PERSISTENT_OBJECT_DEMO
	{crypto_demo_persistent_objects, false, "pobj", "persistent objects demo"},
#endif
#ifdef CONFIG_CRYPTO_ATTESTATION_DEMO
    {crypto_demo_attestation, false, "attestation", "input_hexstring (16 bytes)\n", "Generates attestation token given input_hexstring\n"}
#endif
};

const QCLI_Command_Group_t crypto_cmd_group =
{
    "Crypto",              /* Group_String: will display cmd prompt as "Crypto> " */
    sizeof(crypto_cmd_list)/sizeof(crypto_cmd_list[0]),   /* Command_Count */
    crypto_cmd_list        /* Command_List */
};


/*****************************************************************************
 * This function is used to register the Fs Command Group with QCLI.
 *****************************************************************************/
void Initialize_Crypto_Demo(void)
{
    /* Attempt to reqister the Command Groups with the qcli framework.*/
    qcli_crypto_handle = QCLI_Register_Command_Group(NULL, &crypto_cmd_group);
    if (qcli_crypto_handle)
    {
        QCLI_Printf(qcli_crypto_handle, "Crypto Registered\n");
    }

    return;
}

#ifdef CONFIG_CRYPTO_UNIT_TEST_DEMO
QCLI_Command_Status_t crypto_demo_unittest(uint32_t parameters_count, QCLI_Parameter_t * parameters)
{
    {
        QCLI_Printf(qcli_crypto_handle, "crypto_sha1: start\r\n");
        crypto_sha1();

        QCLI_Printf(qcli_crypto_handle, "crypto_sha256: start\r\n");
        crypto_sha256();

        QCLI_Printf(qcli_crypto_handle, "crypto_sha384: start\r\n");
        crypto_sha384();

        QCLI_Printf(qcli_crypto_handle, "crypto_sha512: start\r\n");
        crypto_sha512();

        QCLI_Printf(qcli_crypto_handle, "crypto_md5: start\r\n");
        crypto_md5();
    }

    {
        QCLI_Printf(qcli_crypto_handle, "crypto_opcopy_sha1: start\r\n");
        crypto_opcopy_sha1();

        QCLI_Printf(qcli_crypto_handle, "crypto_opcopy_sha256: start\r\n");
        crypto_opcopy_sha256();

        QCLI_Printf(qcli_crypto_handle, "crypto_opcopy_sha384: start\r\n");
        crypto_opcopy_sha384();

        QCLI_Printf(qcli_crypto_handle, "crypto_opcopy_sha512: start\r\n");
        crypto_opcopy_sha512();

        QCLI_Printf(qcli_crypto_handle, "crypto_opcopy_md5: start\r\n");
        crypto_opcopy_md5();
    }

    {
        QCLI_Printf(qcli_crypto_handle, "crypto_hmacsha256: start\r\n");
        crypto_hmacsha256();

        QCLI_Printf(qcli_crypto_handle, "crypto_hmacsha512: start\r\n");
        crypto_hmacsha512();

        QCLI_Printf(qcli_crypto_handle, "crypto_hmacsha384: start\r\n");
        crypto_hmacsha384();

        QCLI_Printf(qcli_crypto_handle, "crypto_hmacmd5: start\r\n");
        crypto_hmacmd5();
    }


    {
        char * argv[4];
        argv[3] = "128";
        int32_t argc = 4;

        QCLI_Printf(qcli_crypto_handle, "crypto_aescbc128: start\r\n");
        crypto_aescbc(argc, argv);

        QCLI_Printf(qcli_crypto_handle, "crypto_aesctr128: start\r\n");
        crypto_aesctr(argc, argv);

        QCLI_Printf(qcli_crypto_handle, "crypto_aesccm128: start\r\n");
        crypto_aesccm();
    }

    {
        char * argv[4];
        argv[3] = "256";
        int32_t argc = 4;

        QCLI_Printf(qcli_crypto_handle, "crypto_aescbc256: start\r\n");
        crypto_aescbc(argc, argv);

        QCLI_Printf(qcli_crypto_handle, "crypto_aesctr256: start\r\n");
        crypto_aesctr(argc, argv);

        QCLI_Printf(qcli_crypto_handle, "crypto_aesccm256: start\r\n");
        crypto_aesccm();
    }

    crypto_aesccm_unittests();
    crypto_qapi_ed25519_unittest();
    curve_demo();
    ed25519_demo();
    rsa_demo();
    rsa_encrypt_demo();
    dh_demo();
    ecdh_demo();
    ecdsa_demo();
    srp_demo();
    ecjpake_demo();

    return QCLI_STATUS_SUCCESS_E;
}
#endif

#ifdef CONFIG_CRYPTO_ATTESTATION_DEMO

int convert_data_in_hex_to_byte_array(const char * data_in_hex, uint8_t * data_as_byte_array, uint32_t data_as_byte_array_size);

QCLI_Command_Status_t crypto_demo_attestation(uint32_t parameters_count, QCLI_Parameter_t * parameters)
{
    if ( parameters_count != 1 ) {
        QCLI_Printf(qcli_crypto_handle, "Invalid number of arguments\r\n");
        goto crypto_demo_attestation_on_error;
    }

    char * input_in_hex = parameters[0].String_Value;
    if ( strlen(input_in_hex) != 2*QAPI_CRYPTO_ATTESTATION_INPUT_BUFFER_SIZE ) {
        QCLI_Printf(qcli_crypto_handle, "Invalid input data length, must be exactly 16 bytes (32 hex characters)\r\n");
        goto crypto_demo_attestation_on_error;
    }

    uint8_t input_data[QAPI_CRYPTO_ATTESTATION_INPUT_BUFFER_SIZE];
    memset(input_data, 0, sizeof(input_data));
    uint8_t attestation_token[QAPI_CRYPTO_ATTESTATION_OUTPUT_BUFFER_SIZE];
    memset(attestation_token, 0, sizeof(attestation_token));


    int status_code = convert_data_in_hex_to_byte_array(input_in_hex, input_data, QAPI_CRYPTO_ATTESTATION_INPUT_BUFFER_SIZE);
    if ( 0 != status_code ) {
        QCLI_Printf(qcli_crypto_handle, "Invalid password_in_hex, must be exactly 32 hex chars\r\n");
        goto crypto_demo_attestation_on_error;
    }

    qapi_Status_t status =
        qapi_Crypto_Attestation_Generate_Token(
            input_data,
            attestation_token
            );
    if ( QAPI_OK != status )
    {
        QCLI_Printf(qcli_crypto_handle, "Failed on a call to qapi_Crypto_Attestation_Generate_Token, status=%d\r\n", status);
        goto crypto_demo_attestation_on_error;
    }

    QCLI_Printf(qcli_crypto_handle, "Attestation token: ");
    QCLI_Printf(qcli_crypto_handle, "%02x %02x %02x %02x ", attestation_token[0], attestation_token[1], attestation_token[2], attestation_token[3]);
    QCLI_Printf(qcli_crypto_handle, "%02x %02x %02x %02x ", attestation_token[4], attestation_token[5], attestation_token[6], attestation_token[7]);
    QCLI_Printf(qcli_crypto_handle, "%02x %02x %02x %02x ", attestation_token[8], attestation_token[9], attestation_token[10], attestation_token[11]);
    QCLI_Printf(qcli_crypto_handle, "%02x %02x %02x %02x", attestation_token[12], attestation_token[13], attestation_token[14], attestation_token[15]);

    return QCLI_STATUS_SUCCESS_E;

crypto_demo_attestation_on_error:
    QCLI_Printf(qcli_crypto_handle, "Usage: attestation 16_bytes_input_string\r\n");
    return QCLI_STATUS_ERROR_E;
}
#endif
