/*
 * Copyright (c) 2016-2018 Qualcomm Technologies, Inc.
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
#include "crypto_helper.h"
#include "persistent_obj_demo.h"
#include "qapi_ns_utils.h"
#include "qapi_fs.h"

#ifdef CONFIG_CRYPTO_PERSISTENT_OBJECT_DEMO
extern QCLI_Group_Handle_t qcli_crypto_handle;
#define CRYPTO_DBG
#ifdef CRYPTO_DBG
#define CRYPTO_PRINTF(...)     QCLI_Printf(qcli_crypto_handle, __VA_ARGS__)
#else
#define CRYPTO_PRINTF(x, ...)
#endif

char *g_persistent_obj_demo_root_dir = "/spinor/persistent_obj_demo/";

#define CRYPTO_DEMO_MAX_RSA_KEY_SIZE_SUPPORTED 4096
#define RSA_1024_TEST_KEY_PAIR_OBJECT_ID  "rsa_1024_test_key_pair"
#define RSA_1024_TEST_PUBLIC_KEY_OBJECT_ID  "rsa_1024_test_public_key"
#define ECDSA_P256_TEST_KEY_PAIR_OBJECT_ID  "ecdsa_p256_test_key_pair"
#define ECDSA_P256_TEST_PUBLIC_KEY_OBJECT_ID  "ecdsa_p256_test_public_key"
#define ECDH_P256_TEST_KEY_PAIR_OBJECT_ID  "ecdh_p256_test_key_pair"
#define ECDH_P256_TEST_PUBLIC_KEY_OBJECT_ID  "ecdh_p256_test_public_key"
#define RSA_1024_TEST_KEY_PAIR_RENAMED_OBJECT_ID  "rsa_1024_test_key_pair_renamed"
#define AES_128_TEST_KEY_OBJECT_ID "aes_128_test_key"
#define HMAC_SHA_256_TEST_KEY_OBJECT_ID "hmac_sha_256_test_key"

uint8_t persistent_object_test_input_str[50] =
		{0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a,
		 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32,
		 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a,
		 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0x40, 0x41, 0x42,
		 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a,
		 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50, 0x51, 0x52,
		 0x53, 0x54};

uint8_t expected_hmac_sha_256_hash[32] =
		{0xc6, 0xe9, 0x67, 0x5f, 0x56, 0xa7, 0x44, 0x1c,
		 0x18, 0xa3, 0x31, 0x46, 0x4b, 0xc0, 0x3b, 0x65,
		 0xa3, 0x96, 0x8c, 0xa9, 0xa6, 0xdf, 0xbd, 0xd9,
		 0x5a, 0xa5, 0xcf, 0xf1, 0xc2, 0xe,  0x98, 0xb5};

uint8_t expected_aes_cbc_ciphertext[48] =
	    {0x00, 0x63, 0x38, 0x68, 0xa4, 0xfb, 0x66, 0x91,
	     0xa9, 0x2e, 0x30, 0x67, 0x9e, 0x23, 0x38, 0xb7,
		 0x6e, 0xea, 0x40, 0x05, 0x8c, 0x78, 0x80, 0x2d,
		 0x7b, 0x07, 0xf1, 0x7d, 0xfc, 0x3e, 0xb9, 0x7e,
		 0xc5, 0x17, 0x2c, 0xc3, 0x76, 0x82, 0xf5, 0x82,
		 0xde, 0x20, 0x03, 0x91, 0x57, 0xf7, 0x3d, 0x41};

uint8_t expected_aes_ctr_ciphertext[50] =
	    {0x1e, 0x2b, 0x81, 0x9e, 0x72, 0xfa, 0x8c, 0x80,
	     0x62, 0x78, 0x95, 0x9b, 0xf0, 0x68, 0x1b, 0x08,
		 0x50, 0xaa, 0x53, 0x2d, 0xa9, 0x55, 0x60, 0x2b,
		 0xb0, 0x3c, 0xa8, 0xc2, 0xdb, 0x51, 0xa8, 0xd2,
		 0x76, 0x9b, 0xfc, 0xce, 0x4b, 0xd8, 0xae, 0x92,
		 0x2d, 0x5a, 0x0a, 0x0c, 0xb6, 0xbb, 0x05, 0xac,
	     0x62, 0x3e};

uint8_t expected_aes_ccm_ciphertext[48] =
		{0xd4, 0x92, 0xa6, 0x7a, 0xf8, 0x73, 0xa9, 0x14,
		 0x94, 0xee, 0xbc, 0xc0, 0xa6, 0x5e, 0xc7, 0x63,
		 0x3e, 0x33, 0x2c, 0x2e, 0x8c, 0xf0, 0x81, 0xc9,
		 0xaa, 0xd5, 0x5c, 0x29, 0xf4, 0xad, 0x30, 0x14,
		 0x6a, 0x8f, 0x64, 0x68, 0xfa, 0x2d, 0xef, 0x3f,
		 0xf5, 0xb9, 0xb7, 0xc6, 0x88, 0x19, 0x44, 0x1c};

uint8_t expected_aes_gcm_ciphertext[48] =
		{0x52, 0xd0, 0xa3, 0x65, 0xe6, 0x74, 0xbb, 0x4c,
		 0x22, 0x0a, 0x27, 0x51, 0xfa, 0x25, 0x43, 0xca,
		 0x0d, 0x3c, 0xf3, 0x22, 0x26, 0x36, 0x1d, 0x46,
		 0x6e, 0x4c, 0x7e, 0x82, 0xec, 0x6c, 0x90, 0x23,
		 0xdb, 0x2b, 0xbc, 0x0d, 0x09, 0xcc, 0x79, 0xf3,
		 0x96, 0x7d, 0xca, 0xa5, 0xe5, 0xa6, 0xdd, 0xa6};

uint8_t expected_ecdh_derived_secret[32] =
	    {0x3f, 0x65, 0x9a, 0x1d, 0xca, 0x2a, 0x4a, 0x72,
	     0xc3, 0x8d, 0xbe, 0x17, 0xeb, 0x1b, 0x6a, 0xfe,
		 0xab, 0xdb, 0xcc, 0x9d, 0x31, 0xf9, 0x91, 0xce,
		 0xbe, 0xa0, 0xbc, 0xa1, 0x15, 0x5f, 0x8c, 0xf9};

extern uint8_t rsa_pub_exp[3];
extern uint8_t rsa_mod[128];
extern uint8_t rsa_pvt_exp[128];
extern uint8_t rsa_prime1[64];
extern uint8_t rsa_prime2[64];
extern uint8_t rsa_exp1[64];
extern uint8_t rsa_exp2[64];
extern uint8_t rsa_coeff[64];
extern uint8_t test_iv[32];
extern uint8_t test_adata[20];
extern uint8_t test_generic_secret_key[64];

const char *p_rsa_key_pair_pem =
	"-----BEGIN RSA PRIVATE KEY-----"
	"MIICXgIBAAKBgQCnNCPv6c/FJe145pjpYzfD9RyvObOxuIGYT88vlS9FdzHVS46GxySH6EYYpMqH"
	"NguqSAvYvTEur9CgDtbnRgg6A5DbhxzLElNKr56bmLBmeENhNyObIBKjVWNH5YjY8ZwfBnub8/vb"
	"doaXZxGdHwLaFStJSZfmkr50u41Qqu0dMwIDAQABAoGBAKIuyCEBSUBw3pxkCLRxQfk4ex9YN/3N"
	"+x6TY2NDMLwPtu2uwUvoRH3zhnKG/dATU1NTWPJd6SPg89sdKcThQD38FZgNFH/Ms0256/Xxx5/N"
	"n648jDvFBgpZEaxEm625HkVSymPC6mcNklyhCZqb83n1i80Njk+bRME22gHZlMNhAkEA1AM9Xai0"
	"QrggarF5EPz6/6Zeo6nNe2FUFn3f+5AaxmspJAjKhKbCz/WsqNzJEH+3hBIt15Af2ycB+p/t8tjP"
	"sQJBAMnk7uxQx6EA/TeJkquZsC1Folrs0hJ28WjJQD7lUZcfQ3d/VhEeBYKFjTcuG8cH0Blt3AyP"
	"xHSinx86vmf6OCMCQQDIIw7I3TrdSMeBMGui981RjBIG2YJaGDSyzryj0xMTf5FkrM/QjUOV4MrO"
	"1S0Q4B+zEx4nQaxwys/5cQPEnZrhAkAOWkMN87QtYvedYh9WKafXoBKpqhpJC8GftGbn0b+aIbPX"
	"I+tHbj3wCHSAjruUy55koGW7UuEhdYogWzm8BJLHAkEAsOAsQ0cROF65JnQ5c5bEhxDkuFwtMheb"
	"X4j3+b0JrEjyDpaKvJf8x6NRb4T7jwwUaibsCOmyQ94jxIkTGQqMwQ=="
	"-----END RSA PRIVATE KEY-----";

const char *p_rsa_public_key_pem =
	"-----BEGIN PUBLIC KEY-----"
	"MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQCnNCPv6c/FJe145pjpYzfD9Ryv"
	"ObOxuIGYT88vlS9FdzHVS46GxySH6EYYpMqHNguqSAvYvTEur9CgDtbnRgg6A5Db"
	"hxzLElNKr56bmLBmeENhNyObIBKjVWNH5YjY8ZwfBnub8/vbdoaXZxGdHwLaFStJ"
	"SZfmkr50u41Qqu0dMwIDAQAB"
	"-----END PUBLIC KEY-----";

const char *p_ecc_key_pair_pem =
	"-----BEGIN EC PRIVATE KEY-----"
	"MHcCAQEEIPeR1tgh/jcQa/m590JoEPhY1nopF2Y4HFceyyQWCZZToAoGCCqGSM49"
	"AwEHoUQDQgAE9fLSX7IVG5rSg4Den9OLnHK7YOUm0WKLkYWb9R/QYdB2T0Wix0fH"
	"XMi92+np9fkuq1olTn5VWAZtpwatPByrbA=="
	"-----END EC PRIVATE KEY-----";

const char *p_ecc_public_key_pem =
	"-----BEGIN PUBLIC KEY-----"
	"MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAE9fLSX7IVG5rSg4Den9OLnHK7YOUm"
	"0WKLkYWb9R/QYdB2T0Wix0fHXMi92+np9fkuq1olTn5VWAZtpwatPByrbA=="
	"-----END PUBLIC KEY-----";

// Peer public key for ECDH
const char *p_ecc_peer_public_key_pem =
	"-----BEGIN PUBLIC KEY-----"
	"MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEdw2dT4HwYutsI7o9mEFvc2/XVvBc"
	"yZUMaWeOUGPAnbROFKlHw14yeFXmEm/QjP5acFKJ/XqhZflUISWnYsreDQ=="
	"-----END PUBLIC KEY-----";

void ref_attr_init(qapi_Crypto_Attrib_t* attr, uint32_t id,
        const void* buffer, uint32_t length);

char obj_id_to_print[QAPI_CRYPTO_PERSISTENT_OBJECT_ID_MAX_LEN];
void persistent_obj_demo_print_obj_info(
		uint8_t * obj_id,
		size_t obj_id_length,
		qapi_Crypto_Obj_Info_t *obj_info)
{
	char *object_type = NULL;

	switch(obj_info->object_Type) {
	case QAPI_CRYPTO_OBJ_TYPE_RSA_PUBLIC_KEY_E:
		object_type = "RSA Public key";
		break;
	case QAPI_CRYPTO_OBJ_TYPE_RSA_KEYPAIR_E:
        object_type = "RSA Key pair";
        break;
	case QAPI_CRYPTO_OBJ_TYPE_ECDSA_PUBLIC_KEY_E:
		object_type = "ECDSA public key";
		break;
	case QAPI_CRYPTO_OBJ_TYPE_ECDSA_KEYPAIR_E:
		object_type = "ECDSA key pair";
		break;
	case QAPI_CRYPTO_OBJ_TYPE_ECDH_KEYPAIR_E:
		object_type = "ECDH key pair";
		break;
	case QAPI_CRYPTO_OBJ_TYPE_AES_E:
		object_type = "AES";
		break;
	case QAPI_CRYPTO_OBJ_TYPE_HMAC_MD5_E:
		object_type = "HMAC Md5";
		break;
	case QAPI_CRYPTO_OBJ_TYPE_HMAC_SHA1_E:
		object_type = "HMAC SHA1";
		break;
	case QAPI_CRYPTO_OBJ_TYPE_HMAC_SHA224_E:
		object_type = "HMAC SHA224";
		break;
	case QAPI_CRYPTO_OBJ_TYPE_HMAC_SHA256_E:
		object_type = "HMAC SHA256";
		break;
	case QAPI_CRYPTO_OBJ_TYPE_HMAC_SHA384_E:
		object_type = "HMAC SHA384";
		break;
	case QAPI_CRYPTO_OBJ_TYPE_HMAC_SHA512_E:
		object_type = "HMAC SHA512";
		break;
	case QAPI_CRYPTO_OBJ_TYPE_CHACHA20_E:
		object_type = "CHACHA20";
		break;
	case QAPI_CRYPTO_OBJ_TYPE_GENERIC_SECRET_E:
		object_type = "Generic Secret";
		break;
	case 0:
		object_type = "Data Object";
		break;
	default:
        object_type = "Invalid";
	};

	if(obj_id_length > QAPI_CRYPTO_PERSISTENT_OBJECT_ID_MAX_LEN) {
		CRYPTO_PRINTF("\nInvalid Object Id Length\n");
	}

	memset(obj_id_to_print, 0, sizeof(obj_id_to_print));
	memcpy(obj_id_to_print, obj_id, obj_id_length);

	CRYPTO_PRINTF("\n%30s\t%20s\t%10d\n", obj_id_to_print, object_type, obj_info->key_Size);
}

qapi_Status_t persistent_obj_demo_list_keys()
{
	qapi_Crypto_Enumerator_Hdl_t hdl_enumerator;
	qapi_Status_t status = QAPI_OK;
	qapi_Crypto_Obj_Info_t obj_info;
	uint8_t * obj_id;
	uint32_t obj_id_length;

	obj_id = (uint8_t *)malloc(QAPI_CRYPTO_PERSISTENT_OBJECT_ID_MAX_LEN);
	if(obj_id == NULL) {
		return QAPI_ERR_NO_MEMORY;
	}

	status = qapi_Crypto_Persistent_Obj_Enumerator_Alloc(&hdl_enumerator);
	if(status != QAPI_OK) {
		return status;
	}

	status = qapi_Crypto_Persistent_Obj_Enumerator_Start(
			hdl_enumerator, 0);
	if(status != QAPI_OK) {
		return status;
	}

	CRYPTO_PRINTF("\n%30s\t%20s\t%10s\n", "id", "type", "key size");

	do
	{
		status = qapi_Crypto_Persistent_Obj_Enumerator_Get_Next(
				hdl_enumerator,
				&obj_info,
				obj_id,
				&obj_id_length);
		if ( !((status == QAPI_OK) || (status == QAPI_ERR_NO_ENTRY)) ) {
			goto cleanup;
		}

		if(status != QAPI_ERR_NO_ENTRY)
		{
			persistent_obj_demo_print_obj_info(
					obj_id,
					obj_id_length,
					&obj_info);
		}

	} while(status != QAPI_ERR_NO_ENTRY);

cleanup:

    if(hdl_enumerator) {
    	qapi_Crypto_Persistent_Obj_Enumerator_Free(hdl_enumerator);
    }

    if(obj_id) {
    	free(obj_id);
    }

    if(status == QAPI_ERR_NO_ENTRY) {
    	return QAPI_OK;
    }

    return status;
}

void persistent_obj_demo_print_test_result(uint32_t status,
		uint32_t expected_status,
		const char* p_test_name,
		const char* p_obj_id,
		uint32_t *p_test_number)
{
	if(p_obj_id == NULL) {
		if(status != expected_status) {
			CRYPTO_PRINTF("Test %3d: %100s Failed (status=%d, expected status=%d)\n", *p_test_number, p_test_name, status, expected_status);
		} else {
			CRYPTO_PRINTF("Test %3d: %100s Passed\n", *p_test_number, p_test_name);
		}
	} else {
		if(status != expected_status) {
			CRYPTO_PRINTF("Test %3d: %55s (object id = %30s) Failed (status=%d, expected status=%d)\n", *p_test_number, p_test_name, p_obj_id, status, expected_status);
		} else {
			CRYPTO_PRINTF("Test %3d: %55s (object id = %30s) Passed\n", *p_test_number, p_test_name, p_obj_id);
		}
	}

	*p_test_number = *p_test_number + 1;
}

void persistent_obj_demo_run_asym_encrypt_decrypt_test(
		uint32_t *p_test_number,
		char * obj_id,
		const uint8_t *p_plain_text,
		uint32_t plain_text_len,
		uint32_t algorithm_id)
{
	qapi_Status_t status = QAPI_OK;
	uint8_t* p_cipher_text = NULL;
	uint32_t cipher_text_len = 0;
	uint8_t *p_decrypted_plain_text = 0;
	uint32_t decrypted_plain_text_len = 0;

	/* Encrypt */
	status = crypto_helper_persistent_obj_asym_encrypt (
			(uint8_t*)obj_id,
			strlen(obj_id),
			0,
			p_plain_text,
			plain_text_len,
			&p_cipher_text,
			&cipher_text_len,
			algorithm_id);
	if(status != QAPI_OK) {
		goto cleanup;
	}

	/* Decrypt and compare to original */
	status = crypto_helper_persistent_obj_asym_decrypt(
			(uint8_t*)obj_id,
			strlen(obj_id),
			0,
			p_cipher_text,
			cipher_text_len,
			algorithm_id,
			&p_decrypted_plain_text,
			&decrypted_plain_text_len);
	if(status != QAPI_OK) {
		goto cleanup;
	}

	if ((plain_text_len != decrypted_plain_text_len) ||
		(memcmp(p_plain_text, p_decrypted_plain_text, plain_text_len) != 0))
	{
		status = QAPI_ERROR;
	}

cleanup:

	if(p_cipher_text) {
		free(p_cipher_text);
	}

	if(p_decrypted_plain_text) {
		free(p_decrypted_plain_text);
	}

	persistent_obj_demo_print_test_result(status,
			QAPI_OK,
			"Asymmetric encrypt and decrypt test",
			obj_id,
			p_test_number);
}

void persistent_obj_demo_run_ae_encrypt_decrypt_test(
		uint32_t *p_test_number,
		const char *test_name,
		char * p_obj_id,
		uint8_t *p_plain_text,
		uint32_t plain_text_length,
		uint8_t *p_expected_cipher_text,
		uint32_t expected_cipher_text_len,
		uint32_t algorithm_id)
{
	qapi_Status_t status = QAPI_OK;
	uint8_t* p_cipher_text = NULL;
	uint32_t cipher_text_length = 0;
	uint8_t *p_decrypted_plain_text = 0;
	uint32_t decrypted_plain_text_len = 0;
	uint32_t tag_length_in_bits = 128;
	uint8_t *p_tag_generated_from_encrypt = NULL;
	uint32_t memcmp_ret = 0;

	p_tag_generated_from_encrypt = (uint8_t*)malloc(tag_length_in_bits / 8);
	if(p_tag_generated_from_encrypt == NULL) {
		status = QAPI_ERR_NO_MEMORY;
		goto cleanup;
	}

	status = crypto_helper_persistent_obj_ae_encrypt (
			(uint8_t*) p_obj_id,
			strlen(p_obj_id),
			0,
			algorithm_id,
			&test_iv[0],
			12,
			p_tag_generated_from_encrypt,
			&tag_length_in_bits,
			&test_adata[0],
			sizeof(test_adata),
			p_plain_text,
			plain_text_length,
			&p_cipher_text,
			&cipher_text_length);
	if(status != QAPI_OK) {
		goto cleanup;
	}

	/* Compare cipher text to expected cipher text */
	if(cipher_text_length != expected_cipher_text_len) {
		status = QAPI_ERROR;
		goto cleanup;
	}
	memcmp_ret = memcmp(p_cipher_text, p_expected_cipher_text, cipher_text_length);
	if(memcmp_ret) {
		status = QAPI_ERROR;
		goto cleanup;
	}

	status = crypto_helper_persistent_obj_ae_decrypt (
			(uint8_t *)p_obj_id,
			strlen(p_obj_id),
			0,
			algorithm_id,
			&test_iv[0],
			12,
			p_tag_generated_from_encrypt,
			tag_length_in_bits,
			&test_adata[0],
			sizeof(test_adata),
			p_cipher_text,
			cipher_text_length,
			&p_decrypted_plain_text,
			&decrypted_plain_text_len);
	if(status != QAPI_OK) {
		goto cleanup;
	}

	if ((plain_text_length != decrypted_plain_text_len) ||
		(memcmp(p_plain_text, p_decrypted_plain_text, plain_text_length) != 0))
	{
		status = QAPI_ERROR;
	}

cleanup:

	if(p_tag_generated_from_encrypt) {
		free(p_tag_generated_from_encrypt);
	}

	if(p_cipher_text) {
		free(p_cipher_text);
	}

	if(p_decrypted_plain_text) {
		free(p_decrypted_plain_text);
	}

	persistent_obj_demo_print_test_result(status,
			QAPI_OK,
			test_name,
			p_obj_id,
			p_test_number);
}

void persistent_obj_demo_run_cipher_encrypt_decrypt_test(
		uint32_t *p_test_number,
		const char *test_name,
		char * p_obj_id,
		uint8_t *p_plain_text,
		uint32_t plain_text_len,
		uint8_t *p_expected_cipher_text,
		uint32_t expected_cipher_text_len,
		uint32_t algorithm_id)
{
	qapi_Status_t status = QAPI_OK;
	uint8_t* p_cipher_text = NULL;
	uint32_t cipher_text_len = 0;
	uint8_t *p_decrypted_plain_text = 0;
	uint32_t decrypted_plain_text_len = 0;
	uint32_t memcmp_ret = 0;

	/* Encrypt */
	status = crypto_helper_persistent_obj_cipher_encrypt (
			(uint8_t*)p_obj_id,
			strlen(p_obj_id),
			0,
			algorithm_id,
			&test_iv[0],
			QAPI_CRYPTO_AES_BLOCK_BYTES,
			p_plain_text,
			plain_text_len,
			&p_cipher_text,
			&cipher_text_len);
	if(status != QAPI_OK) {
		goto cleanup;
	}

	/* Compare cipher text to expected cipher text */
	if(cipher_text_len != expected_cipher_text_len) {
		status = QAPI_ERROR;
		goto cleanup;
	}
	memcmp_ret = memcmp(p_cipher_text, p_expected_cipher_text, cipher_text_len);
	if(memcmp_ret) {
		status = QAPI_ERROR;
		goto cleanup;
	}

	/* Decrypt and compare to original */
	status = crypto_helper_persistent_obj_cipher_decrypt(
			(uint8_t*)p_obj_id,
			strlen(p_obj_id),
			0,
			algorithm_id,
			&test_iv[0],
			QAPI_CRYPTO_AES_BLOCK_BYTES,
			p_cipher_text,
			cipher_text_len,
			&p_decrypted_plain_text,
			&decrypted_plain_text_len);
	if(status != QAPI_OK) {
		goto cleanup;
	}

	if ((plain_text_len != decrypted_plain_text_len) ||
		(memcmp(p_plain_text, p_decrypted_plain_text, plain_text_len) != 0))
	{
		status = QAPI_ERROR;
	}

cleanup:

	if(p_cipher_text) {
		free(p_cipher_text);
	}

	if(p_decrypted_plain_text) {
		free(p_decrypted_plain_text);
	}

	persistent_obj_demo_print_test_result(status,
			QAPI_OK,
			test_name,
			p_obj_id,
			p_test_number);
}

void persistent_obj_demo_run_ecdh_test(
		uint32_t *p_test_number,
		const char *key_pair_obj_id,
		const char * public_key_obj_id,
		const char *p_peer_public_key_pem)
{
	uint8_t *p_secret = NULL;
	uint32_t secret_length = 0;

	qapi_Status_t status = crypto_helper_persistent_obj_ecdh(
			(uint8_t *)key_pair_obj_id,
			 strlen(key_pair_obj_id),
			 p_peer_public_key_pem,
			 NULL,
			 &p_secret,
			 &secret_length);

	if(status == QAPI_OK) {
		// Make sure derived secret matched expected value
		if ((secret_length != sizeof(expected_ecdh_derived_secret)) ||
			(memcmp(p_secret, expected_ecdh_derived_secret, secret_length) != 0))
		{
			status = QAPI_ERROR;
		}
	}

	persistent_obj_demo_print_test_result(status,
			QAPI_OK,
			"ECDH test",
			key_pair_obj_id,
			p_test_number);

	if(p_secret) {
		qapi_Crypto_Secure_Memzero(p_secret, secret_length);
		free(p_secret);
	}
}

void persistent_obj_demo_run_sign_verify_test(
		uint32_t *p_test_number,
		const char *key_pair_obj_id,
		const char * public_key_obj_id,
		const uint8_t *p_message,
		uint32_t message_len,
		uint32_t hash_algorithm_id,
		uint32_t signature_algorithm_id)
{
	qapi_Status_t status = QAPI_OK;
	uint8_t *p_signature = NULL;
	uint32_t signature_len = 0;
	uint8_t *p_digest = NULL;
	uint32_t digest_len;

	status = crypto_helper_persistent_obj_hash_and_sign(
			(uint8_t *)key_pair_obj_id,
			strlen(key_pair_obj_id),
			0,
			p_message,
			message_len,
			hash_algorithm_id,
			signature_algorithm_id,
			&p_digest,
			&digest_len,
			&p_signature,
			&signature_len);
	if(status != QAPI_OK) {
		persistent_obj_demo_print_test_result(status,
				QAPI_OK,
				"Sign and verify test",
				key_pair_obj_id,
				p_test_number);
		goto cleanup;
	}

	status = crypto_helper_persistent_obj_verify_signature(
		(uint8_t *)key_pair_obj_id,
		strlen(key_pair_obj_id),
		0,
		signature_algorithm_id,
		p_digest,
		digest_len,
		p_signature,
		signature_len);
	persistent_obj_demo_print_test_result(status,
			QAPI_OK,
			"Sign and verify test",
			key_pair_obj_id,
			p_test_number);

	status = crypto_helper_persistent_obj_verify_signature(
			(uint8_t *)public_key_obj_id,
			strlen(public_key_obj_id),
			0,
			signature_algorithm_id,
			p_digest,
			digest_len,
			p_signature,
			signature_len);
	persistent_obj_demo_print_test_result(status,
			QAPI_OK,
			"Public key signature verify test",
			public_key_obj_id,
			p_test_number);

	// Corrupt signature
	p_signature[0] = p_signature[0] + 1;

	status = crypto_helper_persistent_obj_verify_signature(
		(uint8_t *)key_pair_obj_id,
		strlen(key_pair_obj_id),
		0,
		signature_algorithm_id,
		p_digest,
		digest_len,
		p_signature,
		signature_len);
	persistent_obj_demo_print_test_result(status,
			QAPI_ERROR,
			"Key Pair bad signature test",
			key_pair_obj_id,
			p_test_number);

	status = crypto_helper_persistent_obj_verify_signature(
			(uint8_t *)public_key_obj_id,
			strlen(public_key_obj_id),
			0,
			signature_algorithm_id,
			p_digest,
			digest_len,
			p_signature,
			signature_len);
	persistent_obj_demo_print_test_result(status,
			QAPI_ERROR,
			"Public key bad signature test",
			public_key_obj_id,
			p_test_number);

cleanup:

	if(p_digest) {
		free(p_digest);
	}

	if(p_signature) {
		free(p_signature);
	}
}

void persistent_obj_demo_run_hmac_test(
		uint32_t *p_test_number,
		const char *p_test_name,
		char * p_obj_id,
		uint8_t *p_message,
		uint32_t message_length)
{

	qapi_Status_t status = QAPI_OK;

	status = crypto_helper_persistent_obj_compute_and_compare_hmac(
			(uint8_t*)p_obj_id,
			strlen(p_obj_id),
			0,
			NULL,
			0,
			p_message,
			message_length,
			&expected_hmac_sha_256_hash[0],
			sizeof(expected_hmac_sha_256_hash));
	if(status != QAPI_OK) {
		goto finish;
	}

finish:

	persistent_obj_demo_print_test_result(status,
			QAPI_OK,
			p_test_name,
			p_obj_id,
			p_test_number);
}

void persistent_obj_demo_run_non_existent_object_test(uint32_t *p_test_number)
{
	qapi_Crypto_Obj_Hdl_t persistent_obj_hdl;
	qapi_Status_t status = qapi_Crypto_Persistent_Obj_Open(
		QAPI_CRYPTO_PERSISTENT_OBJ_DATA_TEE_STORAGE_PRIVATE,
		(uint8_t *)"nonexistant",
		strlen("nonexistant"),
		0,
		&persistent_obj_hdl);
	persistent_obj_demo_print_test_result(status,
			QAPI_ERR_NO_ENTRY,
			 "Non-existing key error handling",
			NULL,
			p_test_number);
}

void persistent_obj_demo_clone_test(uint32_t *p_test_number)
{
	qapi_Status_t status = QAPI_OK;
	uint8_t *p_rsa_modulus_buffer = NULL;
	uint32_t modulus_buffer_size = sizeof(rsa_mod);

	p_rsa_modulus_buffer = (uint8_t *)malloc(modulus_buffer_size);
	if(p_rsa_modulus_buffer == NULL) {
		status = QAPI_ERR_NO_MEMORY;
		goto finish;
	}
	memset(p_rsa_modulus_buffer, 0, modulus_buffer_size);

	status = crypto_helper_persistent_obj_clone(
			(uint8_t *)RSA_1024_TEST_KEY_PAIR_OBJECT_ID,
			strlen(RSA_1024_TEST_KEY_PAIR_OBJECT_ID),
			(uint8_t *)"cloned_object",
			strlen("cloned_object"));
	if(status != QAPI_OK) {
		goto finish;
	}

	status = crypto_helper_persistent_obj_get_buffer_attibute(
			(uint8_t *)"cloned_object",
			strlen("cloned_object"),
			QAPI_CRYPTO_ATTR_RSA_MODULUS_E,
			p_rsa_modulus_buffer,
			modulus_buffer_size);
	if(status == QAPI_OK ) {
		if(memcmp(p_rsa_modulus_buffer, rsa_mod, modulus_buffer_size) != 0) {
			status = QAPI_ERROR;
		}
	} else {
		status = QAPI_ERROR;
	}

	free(p_rsa_modulus_buffer);

	uint32_t delete_status = crypto_helper_persistent_obj_delete(
			(uint8_t *)"cloned_object",
			strlen("cloned_object"),
			QAPI_CRYPTO_DATA_ACCESS_WRITE_META);
	if(delete_status != QAPI_OK) {
		CRYPTO_PRINTF("Failed to delete cloned object. (status=%d)\n", delete_status);
	}

finish:

	persistent_obj_demo_print_test_result(status,
			QAPI_OK,
			"Object clone test",
			NULL,
			p_test_number);
}

void persistent_obj_demo_rename_test(uint32_t *p_test_number)
{
	qapi_Crypto_Obj_Info_t obj_info = {0};

    // Rename the object to a new id
	qapi_Status_t status = crypto_helper_persistent_obj_rename (
			(uint8_t*)RSA_1024_TEST_KEY_PAIR_OBJECT_ID,
			strlen(RSA_1024_TEST_KEY_PAIR_OBJECT_ID),
			(uint8_t*)RSA_1024_TEST_KEY_PAIR_RENAMED_OBJECT_ID,
			strlen(RSA_1024_TEST_KEY_PAIR_RENAMED_OBJECT_ID),
			QAPI_CRYPTO_DATA_ACCESS_WRITE_META);
	if(status != QAPI_OK) {
		goto finish;
	}

	// Check if we can still parse the object meta-data after
	// renaming the object
	status = crypto_helper_persistent_obj_get_obj_info(
			(uint8_t*)RSA_1024_TEST_KEY_PAIR_RENAMED_OBJECT_ID,
			strlen(RSA_1024_TEST_KEY_PAIR_RENAMED_OBJECT_ID),
			&obj_info);
	if(status != QAPI_OK) {
		goto finish;
	}
    if (obj_info.object_Type != QAPI_CRYPTO_OBJ_TYPE_RSA_KEYPAIR_E) {
    	status = QAPI_ERROR;
    	goto finish;
    }

    // Rename the object back to its original id
	status = crypto_helper_persistent_obj_rename (
			(uint8_t*)RSA_1024_TEST_KEY_PAIR_RENAMED_OBJECT_ID,
			strlen(RSA_1024_TEST_KEY_PAIR_RENAMED_OBJECT_ID),
			(uint8_t*)RSA_1024_TEST_KEY_PAIR_OBJECT_ID,
			strlen(RSA_1024_TEST_KEY_PAIR_OBJECT_ID),
			QAPI_CRYPTO_DATA_ACCESS_WRITE_META);
	if(status != QAPI_OK) {
		goto finish;
	}

finish:
	persistent_obj_demo_print_test_result(status,
			QAPI_OK,
			"Rename test",
			NULL,
			p_test_number);
}

QCLI_Command_Status_t persistent_obj_demo_create_test_objects()
{
	qapi_Status_t status = QAPI_OK;

	// Create RSA key pair persistent object
	status = crypto_helper_persistent_obj_create_asymmetric_key_from_pem(
			QAPI_CRYPTO_OBJ_TYPE_RSA_KEYPAIR_E,
			0,
			(uint8_t *)RSA_1024_TEST_KEY_PAIR_OBJECT_ID,
			strlen(RSA_1024_TEST_KEY_PAIR_OBJECT_ID),
			p_rsa_key_pair_pem,
			NULL);
	if(status != QAPI_OK) {
		CRYPTO_PRINTF("Create RSA 1024 key pair failed (status=%d)\n", status);
		return QCLI_STATUS_ERROR_E;
	} else {
		CRYPTO_PRINTF("Created RSA 1024 key pair.\n");
	}

	// Create RSA public key persistent object
	status = crypto_helper_persistent_obj_create_asymmetric_key_from_pem(
			QAPI_CRYPTO_OBJ_TYPE_RSA_PUBLIC_KEY_E,
			0,
			(uint8_t *)RSA_1024_TEST_PUBLIC_KEY_OBJECT_ID,
			strlen(RSA_1024_TEST_PUBLIC_KEY_OBJECT_ID),
			p_rsa_public_key_pem,
			NULL);
	if(status != QAPI_OK) {
		CRYPTO_PRINTF("Create RSA 1024 public key failed (status=%d)\n", status);
		return QCLI_STATUS_ERROR_E;
	} else {
		CRYPTO_PRINTF("Created RSA 1024 public key.\n");
	}

	// Create ECDSA key pair persistent object
	status = crypto_helper_persistent_obj_create_asymmetric_key_from_pem(
			QAPI_CRYPTO_OBJ_TYPE_ECDSA_KEYPAIR_E,
			0,
			(uint8_t *)ECDSA_P256_TEST_KEY_PAIR_OBJECT_ID,
			strlen(ECDSA_P256_TEST_KEY_PAIR_OBJECT_ID),
			p_ecc_key_pair_pem,
			NULL);
	if(status != QAPI_OK) {
		CRYPTO_PRINTF("Create ECDSA P256 key pair failed (status=%d)\n", status);
		return QCLI_STATUS_ERROR_E;
	} else {
		CRYPTO_PRINTF("Created EDCSA P256 key pair.\n");
	}

	// Create ECDH key pair persistent object
	status = crypto_helper_persistent_obj_create_asymmetric_key_from_pem(
			QAPI_CRYPTO_OBJ_TYPE_ECDH_KEYPAIR_E,
			0,
			(uint8_t *)ECDH_P256_TEST_KEY_PAIR_OBJECT_ID,
			strlen(ECDH_P256_TEST_KEY_PAIR_OBJECT_ID),
			p_ecc_key_pair_pem,
			NULL);
	if(status != QAPI_OK) {
		CRYPTO_PRINTF("Create ECDH P256 key pair failed (status=%d)\n", status);
		return QCLI_STATUS_ERROR_E;
	} else {
		CRYPTO_PRINTF("Created ECDH P256 key pair.\n");
	}

	// Create ECDSA public key persistent object
	status = crypto_helper_persistent_obj_create_asymmetric_key_from_pem(
			QAPI_CRYPTO_OBJ_TYPE_ECDSA_PUBLIC_KEY_E,
			0,
			(uint8_t *)ECDSA_P256_TEST_PUBLIC_KEY_OBJECT_ID,
			strlen(ECDSA_P256_TEST_PUBLIC_KEY_OBJECT_ID),
			p_ecc_public_key_pem,
			NULL);
	if(status != QAPI_OK) {
		CRYPTO_PRINTF("Create ECDSA P256 public key failed (status=%d)\n", status);
		return QCLI_STATUS_ERROR_E;
	} else {
		CRYPTO_PRINTF("Created EDCSA P256 public key.\n");
	}

	// Create AES 128 key persistent object
	status = crypto_helper_persistent_obj_create_symmetric_key(
			QAPI_CRYPTO_OBJ_TYPE_AES_E,
			0,
			(uint8_t*)AES_128_TEST_KEY_OBJECT_ID,
			strlen(AES_128_TEST_KEY_OBJECT_ID),
			test_generic_secret_key,
			8 * 16);
	if(status != QAPI_OK) {
		CRYPTO_PRINTF("Create AES 128 key failed (status=%d)\n", status);
		return QCLI_STATUS_ERROR_E;
	} else {
		CRYPTO_PRINTF("Created AES 128 key.\n");
	}

	// Create SHA-256 key persistent object
	status = crypto_helper_persistent_obj_create_symmetric_key(
			QAPI_CRYPTO_OBJ_TYPE_HMAC_SHA256_E,
			0,
			(uint8_t*)HMAC_SHA_256_TEST_KEY_OBJECT_ID,
			strlen(HMAC_SHA_256_TEST_KEY_OBJECT_ID),
			test_generic_secret_key,
			8 * 64);
	if(status != QAPI_OK) {
		CRYPTO_PRINTF("Create HMAC-SHA 256 key failed (status=%d)\n", status);
		return QCLI_STATUS_ERROR_E;
	} else {
		CRYPTO_PRINTF("Create HMAC-SHA 256 key.\n");
	}

	return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t persistent_obj_demo_delete_test_objects()
{
	qapi_Status_t status = QAPI_OK;

	// Delete RSA key pair persistent object
	crypto_helper_persistent_obj_delete(
			(uint8_t *)RSA_1024_TEST_KEY_PAIR_OBJECT_ID,
			strlen(RSA_1024_TEST_KEY_PAIR_OBJECT_ID),
			QAPI_CRYPTO_DATA_ACCESS_WRITE_META);
	if(status != QAPI_OK) {
		CRYPTO_PRINTF("Delete RSA 1024 key pair failed (status=%d)\n", status);
	} else {
		CRYPTO_PRINTF("Deleted RSA 1024 key pair.\n");
	}

	// Delete RSA public key persistent object
	status = crypto_helper_persistent_obj_delete(
			(uint8_t *)RSA_1024_TEST_PUBLIC_KEY_OBJECT_ID,
			strlen(RSA_1024_TEST_PUBLIC_KEY_OBJECT_ID),
			QAPI_CRYPTO_DATA_ACCESS_WRITE_META);
	if(status != QAPI_OK) {
		CRYPTO_PRINTF("Delete RSA 1024 public key failed (status=%d)\n", status);
	} else {
		CRYPTO_PRINTF("Deleted RSA 1024 public key.\n");
	}

	// Delete ECDSA key pair persistent object
	status = crypto_helper_persistent_obj_delete(
			(uint8_t *)ECDSA_P256_TEST_KEY_PAIR_OBJECT_ID,
			strlen(ECDSA_P256_TEST_KEY_PAIR_OBJECT_ID),
			QAPI_CRYPTO_DATA_ACCESS_WRITE_META);
	if(status != QAPI_OK) {
		CRYPTO_PRINTF("Delete ECDSA P256 key pair failed (status=%d)\n", status);
	} else {
		CRYPTO_PRINTF("Deleted EDCSA P256 key pair.\n");
	}

	// Delete ECDSA public key persistent object
	status = crypto_helper_persistent_obj_delete(
			(uint8_t *)ECDSA_P256_TEST_PUBLIC_KEY_OBJECT_ID,
			strlen(ECDSA_P256_TEST_PUBLIC_KEY_OBJECT_ID),
			QAPI_CRYPTO_DATA_ACCESS_WRITE_META);
	if(status != QAPI_OK) {
		CRYPTO_PRINTF("Delete ECDSA P256 public key failed (status=%d)\n", status);
	} else {
		CRYPTO_PRINTF("Deleted EDCSA P256 public key.\n");
	}

	// Delete ECDH key pair persistent object
	status = crypto_helper_persistent_obj_delete(
			(uint8_t *)ECDH_P256_TEST_KEY_PAIR_OBJECT_ID,
			strlen(ECDH_P256_TEST_KEY_PAIR_OBJECT_ID),
			QAPI_CRYPTO_DATA_ACCESS_WRITE_META);
	if(status != QAPI_OK) {
		CRYPTO_PRINTF("Delete ECDH P256 key pair failed (status=%d)\n", status);
	} else {
		CRYPTO_PRINTF("Deleted ECDH P256 key pair.\n");
	}

	// Delete AES persistent object key
	status = crypto_helper_persistent_obj_delete(
			(uint8_t *)AES_128_TEST_KEY_OBJECT_ID,
			strlen(AES_128_TEST_KEY_OBJECT_ID),
			QAPI_CRYPTO_DATA_ACCESS_WRITE_META);
	if(status != QAPI_OK) {
		CRYPTO_PRINTF("Delete AES 128 key failed (status=%d)\n", status);
	} else {
		CRYPTO_PRINTF("Deleted AES 128 key.\n");
	}

	// Delete HMAC-SHA-256 persistent object key
	status = crypto_helper_persistent_obj_delete(
			(uint8_t *)HMAC_SHA_256_TEST_KEY_OBJECT_ID,
			strlen(HMAC_SHA_256_TEST_KEY_OBJECT_ID),
			QAPI_CRYPTO_DATA_ACCESS_WRITE_META);
	if(status != QAPI_OK) {
		CRYPTO_PRINTF("Delete HMAC-SHA 256 key failed (status=%d)\n", status);
	} else {
		CRYPTO_PRINTF("Deleted HMAC-SHA 256 key.\n");
	}

	return QCLI_STATUS_SUCCESS_E;
}

void persistent_obj_demo_rsa_get_attribute_tests(uint32_t *p_test_number)
{
	uint8_t *p_rsa_modulus_buffer = NULL;
	uint8_t *p_rsa_public_exponent = NULL;
	uint32_t modulus_buffer_size = sizeof(rsa_mod);
	uint32_t rsa_public_exponent_size = sizeof(rsa_pub_exp);
	qapi_Status_t status = QAPI_OK;

	p_rsa_modulus_buffer = (uint8_t *)malloc(modulus_buffer_size);
	if(p_rsa_modulus_buffer == NULL) {
		CRYPTO_PRINTF("\nGet attributes test failed due to memory allocation error\n");
	}
	memset(p_rsa_modulus_buffer, 0, modulus_buffer_size);

	p_rsa_public_exponent = (uint8_t *)malloc(rsa_public_exponent_size);
	if(p_rsa_public_exponent == NULL) {
		CRYPTO_PRINTF("\nGet attributes test failed due to memory allocation error\n");
	}
	memset(p_rsa_public_exponent, 0, rsa_public_exponent_size);

	// get modulus from key pair
	status = crypto_helper_persistent_obj_get_buffer_attibute(
			(uint8_t*)RSA_1024_TEST_KEY_PAIR_OBJECT_ID,
			strlen(RSA_1024_TEST_KEY_PAIR_OBJECT_ID),
			QAPI_CRYPTO_ATTR_RSA_MODULUS_E,
			p_rsa_modulus_buffer,
			modulus_buffer_size);
	if(status == QAPI_OK) {
		if(memcmp(p_rsa_modulus_buffer, rsa_mod, modulus_buffer_size) != 0) {
			status = QAPI_ERROR;
		}
	}
	persistent_obj_demo_print_test_result(status,
			QAPI_OK,
			"Get RSA modulus test",
			RSA_1024_TEST_KEY_PAIR_OBJECT_ID,
			p_test_number);

	// get modulus from public key
	memset(p_rsa_modulus_buffer, 0, modulus_buffer_size);

	status = crypto_helper_persistent_obj_get_buffer_attibute(
			(uint8_t*)RSA_1024_TEST_PUBLIC_KEY_OBJECT_ID,
			strlen(RSA_1024_TEST_PUBLIC_KEY_OBJECT_ID),
			QAPI_CRYPTO_ATTR_RSA_MODULUS_E,
			p_rsa_modulus_buffer,
			modulus_buffer_size);
	if(status == QAPI_OK) {
		if(memcmp(p_rsa_modulus_buffer, rsa_mod, modulus_buffer_size) != 0) {
			status = QAPI_ERROR;
		}
	}
	persistent_obj_demo_print_test_result(status,
			QAPI_OK,
			"Get RSA modulus test",
			RSA_1024_TEST_PUBLIC_KEY_OBJECT_ID,
			p_test_number);

	// get public exponent from key pair
	status = crypto_helper_persistent_obj_get_buffer_attibute(
			(uint8_t*)RSA_1024_TEST_KEY_PAIR_OBJECT_ID,
			strlen(RSA_1024_TEST_KEY_PAIR_OBJECT_ID),
			QAPI_CRYPTO_ATTR_RSA_PUBLIC_EXPONENT_E,
			p_rsa_public_exponent,
			rsa_public_exponent_size);
	if(status == QAPI_OK) {
		if(memcmp(p_rsa_public_exponent, rsa_pub_exp, rsa_public_exponent_size) != 0) {
			status = QAPI_ERROR;
		}
	}
	persistent_obj_demo_print_test_result(status,
			QAPI_OK,
			"Get RSA public exponent test",
			RSA_1024_TEST_KEY_PAIR_OBJECT_ID,
			p_test_number);

	// get exponent from public key
	memset(p_rsa_modulus_buffer, 0, modulus_buffer_size);

	status = crypto_helper_persistent_obj_get_buffer_attibute(
			(uint8_t*)RSA_1024_TEST_PUBLIC_KEY_OBJECT_ID,
			strlen(RSA_1024_TEST_PUBLIC_KEY_OBJECT_ID),
			QAPI_CRYPTO_ATTR_RSA_PUBLIC_EXPONENT_E,
			p_rsa_public_exponent,
			rsa_public_exponent_size);
	if(status == QAPI_OK) {
		if(memcmp(p_rsa_public_exponent, rsa_pub_exp, rsa_public_exponent_size) != 0) {
			status = QAPI_ERROR;
		}
	}
	persistent_obj_demo_print_test_result(status,
			QAPI_OK,
			"Get RSA public exponent test",
			RSA_1024_TEST_PUBLIC_KEY_OBJECT_ID,
			p_test_number);

	uint32_t curve_id = 0;
	status = crypto_helper_persistent_obj_get_value_attibute(
			(uint8_t*)ECDSA_P256_TEST_KEY_PAIR_OBJECT_ID,
			strlen(ECDSA_P256_TEST_KEY_PAIR_OBJECT_ID),
			QAPI_CRYPTO_ATTR_ECC_CURVE_E,
			&curve_id,
			NULL);
	if(status == QAPI_OK) {
		if(curve_id != QAPI_CRYPTO_ECC_CURVE_NIST_P256) {
			status = QAPI_ERROR;
		}
	}
	persistent_obj_demo_print_test_result(status,
			QAPI_OK,
			"Get ECDSA key curve id value attribute test",
			ECDSA_P256_TEST_KEY_PAIR_OBJECT_ID,
			p_test_number);

	if(p_rsa_modulus_buffer) {
		free(p_rsa_modulus_buffer);
	}

	if(p_rsa_public_exponent) {
		free(p_rsa_public_exponent);
	}
}

void persistent_obj_demo_run_create_flags_test_suite(uint32_t *p_test_number)
{
	qapi_Status_t status = QAPI_OK;
	qapi_Crypto_Obj_Hdl_t transient_obj_hdl = 0;
	qapi_Crypto_Obj_Hdl_t persistent_obj_hdl = 0;

	/* Try to overwrite RSA public key with out exclusive flag set */
	status = qapi_Crypto_Transient_Obj_Import_From_Pem(QAPI_CRYPTO_OBJ_TYPE_RSA_PUBLIC_KEY_E, p_rsa_public_key_pem, NULL, &transient_obj_hdl);
	if(status == QAPI_OK) {
		status = qapi_Crypto_Persistent_Obj_Create(
				QAPI_CRYPTO_PERSISTENT_OBJ_DATA_TEE_STORAGE_PRIVATE,
				(uint8_t *)RSA_1024_TEST_PUBLIC_KEY_OBJECT_ID,
				strlen(RSA_1024_TEST_PUBLIC_KEY_OBJECT_ID),
				0,
				transient_obj_hdl,
				NULL,
				0,
				&persistent_obj_hdl);
	}
	persistent_obj_demo_print_test_result(status,
			QAPI_OK,
			"Overwrite without exclusive flag test",
			NULL,
			p_test_number);

	if(persistent_obj_hdl != 0) {
		qapi_Crypto_Persistent_Obj_Close(persistent_obj_hdl);
		persistent_obj_hdl = 0;
	}

	/* Try to overwrite RSA public key persistent object with
	 * exclusive flag set.  This should failed since this persistent
	 * object already exists. */
	status = qapi_Crypto_Persistent_Obj_Create(
			QAPI_CRYPTO_PERSISTENT_OBJ_DATA_TEE_STORAGE_PRIVATE,
			(uint8_t *)RSA_1024_TEST_PUBLIC_KEY_OBJECT_ID,
			strlen(RSA_1024_TEST_PUBLIC_KEY_OBJECT_ID),
			QAPI_CRYPTO_DATA_EXCLUSIVE,
			transient_obj_hdl,
			NULL,
			0,
			&persistent_obj_hdl);
	persistent_obj_demo_print_test_result(status,
			QAPI_ERR_CRYPTO_ACCESS_CONFLICT,
			"Overwrite with exclusive flag test",
			NULL,
			p_test_number);

	if(persistent_obj_hdl != 0) {
		qapi_Crypto_Persistent_Obj_Close(persistent_obj_hdl);
	}
	if(transient_obj_hdl != 0) {
		qapi_Crypto_Transient_Obj_Free(transient_obj_hdl);
	}
}

void persistent_obj_demo_run_write_meta_data_flags_test_suite(uint32_t *p_test_number)
{
	qapi_Status_t status = QAPI_OK;
	// Without the QAPI_CRYPTO_DATA_ACCESS_WRITE_META access flag this should fail
	status = crypto_helper_persistent_obj_delete(
			(uint8_t *)ECDSA_P256_TEST_PUBLIC_KEY_OBJECT_ID,
			strlen(ECDSA_P256_TEST_PUBLIC_KEY_OBJECT_ID),
			0);
	persistent_obj_demo_print_test_result(status,
			QAPI_ERR_CRYPTO_ACCESS_CONFLICT,
			"Delete without meta-data access flag test",
			NULL,
			p_test_number);
}

QCLI_Command_Status_t persistent_obj_demo_run_test_suite()
{
	QCLI_Command_Status_t status = QCLI_STATUS_SUCCESS_E;
	uint32_t test_number = 1;

	CRYPTO_PRINTF("\n===Creating test persistent objects===\n");
	status = persistent_obj_demo_create_test_objects();
	if(status != QCLI_STATUS_SUCCESS_E) {
		return status;
	}

	CRYPTO_PRINTF("\n===Enumerate test persistent objects===\n");

	status = persistent_obj_demo_list_keys();
	if(status != QAPI_OK) {
		CRYPTO_PRINTF("Failed to enumerate keys with (status=%d)\n", status);
	}

	CRYPTO_PRINTF("\n===Running tests===\n");

	persistent_obj_demo_rsa_get_attribute_tests(&test_number);

	persistent_obj_demo_run_non_existent_object_test(&test_number);

	persistent_obj_demo_run_write_meta_data_flags_test_suite(&test_number);

	persistent_obj_demo_rename_test(&test_number);

	persistent_obj_demo_clone_test(&test_number);

	persistent_obj_demo_run_create_flags_test_suite(&test_number);

	uint32_t input_size = sizeof(persistent_object_test_input_str);

	persistent_obj_demo_run_sign_verify_test(
			&test_number,
			RSA_1024_TEST_KEY_PAIR_OBJECT_ID,
			RSA_1024_TEST_PUBLIC_KEY_OBJECT_ID,
			persistent_object_test_input_str,
			input_size,
			QAPI_CRYPTO_ALG_SHA256_E,
			QAPI_CRYPTO_ALG_RSASSA_PKCS1_V1_5_SHA256_E);

	persistent_obj_demo_run_asym_encrypt_decrypt_test(
			&test_number,
			RSA_1024_TEST_KEY_PAIR_OBJECT_ID,
			persistent_object_test_input_str,
			input_size,
			QAPI_CRYPTO_ALG_RSAES_PKCS1_V1_5_E);

	persistent_obj_demo_run_sign_verify_test(
			&test_number,
			ECDSA_P256_TEST_KEY_PAIR_OBJECT_ID,
			ECDSA_P256_TEST_PUBLIC_KEY_OBJECT_ID,
			persistent_object_test_input_str,
			input_size,
			QAPI_CRYPTO_ALG_SHA256_E,
			QAPI_CRYPTO_ALG_ECDSA_P256_E);

	persistent_obj_demo_run_ecdh_test(
			&test_number,
			ECDH_P256_TEST_KEY_PAIR_OBJECT_ID,
			ECDH_P256_TEST_KEY_PAIR_OBJECT_ID,
			p_ecc_peer_public_key_pem);

	persistent_obj_demo_run_cipher_encrypt_decrypt_test(
			&test_number,
			"Cipher encrypt and decrypt (AES CTR) test",
			AES_128_TEST_KEY_OBJECT_ID,
			persistent_object_test_input_str,
			input_size,
			expected_aes_ctr_ciphertext,
			sizeof(expected_aes_ctr_ciphertext),
			QAPI_CRYPTO_ALG_AES_CTR_E);

	// the input must be a multiple of block size for AES CBC_NOPAD mode
	persistent_obj_demo_run_cipher_encrypt_decrypt_test(
			&test_number,
			"Cipher encrypt and decrypt (AES CBC no pad) test",
			AES_128_TEST_KEY_OBJECT_ID,
			persistent_object_test_input_str,
			QAPI_CRYPTO_AES_BLOCK_BYTES * (input_size / QAPI_CRYPTO_AES_BLOCK_BYTES),
			expected_aes_cbc_ciphertext,
			sizeof(expected_aes_cbc_ciphertext),
			QAPI_CRYPTO_ALG_AES_CBC_NOPAD_E);

	persistent_obj_demo_run_ae_encrypt_decrypt_test(
			&test_number,
			"Authenticated encrypt and decrypt (AES GCM) test",
			AES_128_TEST_KEY_OBJECT_ID,
			persistent_object_test_input_str,
			4 * (input_size / 4),
			expected_aes_gcm_ciphertext,
			sizeof(expected_aes_gcm_ciphertext),
			QAPI_CRYPTO_ALG_AES_GCM_E);

	persistent_obj_demo_run_ae_encrypt_decrypt_test(
			&test_number,
			"Authenticated encrypt and decrypt (AES CCM) test",
			AES_128_TEST_KEY_OBJECT_ID,
			persistent_object_test_input_str,
			4 * (input_size / 4),
			expected_aes_ccm_ciphertext,
			sizeof(expected_aes_ccm_ciphertext),
			QAPI_CRYPTO_ALG_AES_CCM_E);

	persistent_obj_demo_run_hmac_test(
			&test_number,
			"HMAC-SHA256 test",
			HMAC_SHA_256_TEST_KEY_OBJECT_ID,
			persistent_object_test_input_str,
			4 * (input_size / 4));

	CRYPTO_PRINTF("\n===Deleting keys===\n");

	persistent_obj_demo_delete_test_objects();

	return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t persistent_obj_demo_list_command()
{
	QCLI_Command_Status_t status = persistent_obj_demo_list_keys();
	if(status != QAPI_OK) {
		CRYPTO_PRINTF("Failed to enumerate keys with (status=%d)\n", status);
		return QCLI_STATUS_ERROR_E;
	}
	return QCLI_STATUS_SUCCESS_E;
}

void persistent_obj_demo_print_binary_buffer(uint8_t *p_buffer, uint32_t buffer_size)
{
	size_t b64encoded_buffer_size = qapi_Net_Get_Base64_Encoded_Output_Size(buffer_size);
	char *p_b64_output_buffer = (char*)malloc(b64encoded_buffer_size);
	if(p_b64_output_buffer == NULL) {
		CRYPTO_PRINTF("Error allocating memory to print base64 encoded buffer\n");
	}
	QCLI_Command_Status_t status = qapi_Net_Base64_Encode(
			p_buffer,
			buffer_size,
			p_b64_output_buffer,
			b64encoded_buffer_size);
	if(status != QAPI_OK) {
		CRYPTO_PRINTF("Error encoding buffer to base64\n");
	} else {
		int i;
		for(i = 0; i < b64encoded_buffer_size; i++) {
			if((i % 32) == 0) {
				CRYPTO_PRINTF("\n");
			}
			CRYPTO_PRINTF("%c", p_b64_output_buffer[i]);
		}
		CRYPTO_PRINTF("\n\n");
	}
	free(p_b64_output_buffer);
}

int certcs_download_file(
    char * hostname,
    int port,
    char * filename,
    uint8_t ** pp_buffer,
    uint32_t * p_buffer_size);

qapi_Status_t persistent_obj_demo_load_test_file(
		const char * filename,
	    uint8_t ** pp_buffer,
	    uint32_t * p_buffer_size)
{
	qapi_Status_t status = QAPI_ERROR;
	int fd = 0;
	uint8_t *p_buffer = NULL;

	uint32_t file_path_length = strlen(g_persistent_obj_demo_root_dir) + strlen(filename) + 1;

	// if no host is configured try to read the file
	// from the persistent object demo root directory
	char *file_path = malloc( file_path_length );
	if ( file_path == NULL ) {
		CRYPTO_PRINTF("ERROR: Unable to allocate memory\r\n");
		return QAPI_ERR_NO_MEMORY;
	}
	memset(file_path, 0, file_path_length);

	memcpy(file_path, g_persistent_obj_demo_root_dir, strlen(g_persistent_obj_demo_root_dir));
	uint32_t strlcat_ret = strlcat(file_path, filename, file_path_length);
	if(strlcat_ret >= file_path_length) {
		return QAPI_ERROR;
	}

	struct qapi_fs_stat_type stat;
	status = qapi_Fs_Stat(file_path, &stat);
    if ( QAPI_OK != status ) {
        CRYPTO_PRINTF("Failed on a call to qapi_Fs_Stat()\r\n");
        goto cleanup;
    }

	p_buffer = (uint8_t*)malloc(stat.st_size);
	if(p_buffer == NULL) {
        CRYPTO_PRINTF("Failed to allocate memory.\r\n");
		status = QAPI_ERR_NO_MEMORY;
        goto cleanup;
	}

    status = qapi_Fs_Open(file_path, QAPI_FS_O_RDONLY, &fd);
    if ( QAPI_OK != status ) {
        CRYPTO_PRINTF("Failed on a call to qapi_Fs_Open()\r\n");
        goto cleanup;
    }

	uint32_t bytes_read = 0;
	status = qapi_Fs_Read(fd, p_buffer, stat.st_size, &bytes_read);
	if( (QAPI_OK != status) || (bytes_read != stat.st_size) ) {
	    CRYPTO_PRINTF("Failed to read file (status=%d).\r\n", status);
		*pp_buffer = NULL;
		*p_buffer_size = 0;
	} else {
		*pp_buffer = p_buffer;
		*p_buffer_size = stat.st_size;
	}

	qapi_Fs_Close(fd);

cleanup:

	free(file_path);

	if( (status != QAPI_OK) && (p_buffer != NULL) ) {
		free(p_buffer);
	}

	return status;
}

qapi_Status_t persistent_obj_demo_load_file(
	const char *hostname,
	uint16_t port,
	const char *filename,
	uint8_t **pp_buffer,
	uint32_t *p_buffer_size)
{
	qapi_Status_t status = QAPI_ERROR;

	if ( hostname != NULL )
	{
#ifdef CONFIG_NET_SSL_DEMO
		status =
			certcs_download_file(
				(char *)hostname,
				port,
				(char *)filename,
				pp_buffer,
				p_buffer_size);
#endif
		if ( status != 0 ) {
			CRYPTO_PRINTF("ERROR: failed to download file, status=%d\r\n", status);
		} else {
			CRYPTO_PRINTF("INFO: downloaded file of size %d bytes\r\n", *p_buffer_size);
		}
	}
	else
	{
		status = persistent_obj_demo_load_test_file(
				filename,
			    pp_buffer,
			    p_buffer_size);
		if ( status != QAPI_OK ) {
			CRYPTO_PRINTF("ERROR: failed to load file, status=%d\r\n", status);
		} else {
			CRYPTO_PRINTF("INFO: loaded file of size %d bytes\r\n", *p_buffer_size);
		}
	}

	return status;
}

QCLI_Command_Status_t persistent_obj_demo_delete_command(uint32_t Parameter_Count, QCLI_Parameter_t * Parameter_List)
{
	uint8_t * obj_id = NULL;
	size_t object_id_length = 0;

	if(Parameter_Count != 1) {
		CRYPTO_PRINTF("ERROR: invalid number of arguments\r\n");
		return QCLI_STATUS_USAGE_E;
	}

	obj_id = (uint8_t *)Parameter_List[0].String_Value;
    object_id_length = strlen((char *)obj_id);

    qapi_Status_t status = crypto_helper_persistent_obj_delete(
    		obj_id,
			object_id_length,
			QAPI_CRYPTO_DATA_ACCESS_WRITE_META);
	if(status != QAPI_OK) {
		CRYPTO_PRINTF("Delete failed (status=%d)\n", status);
		return QCLI_STATUS_ERROR_E;
	} else {
		CRYPTO_PRINTF("Deleted object.\n");
		return QCLI_STATUS_SUCCESS_E;
	}
}

QCLI_Command_Status_t persistent_obj_demo_rename_command(uint32_t Parameter_Count, QCLI_Parameter_t * Parameter_List)
{
	uint8_t * p_obj_id = NULL;
	size_t object_id_length = 0;
	uint8_t * p_new_obj_id = NULL;
	size_t new_object_id_length = 0;

	if(Parameter_Count != 2) {
		CRYPTO_PRINTF("ERROR: invalid number of arguments\r\n");
		return QCLI_STATUS_USAGE_E;
	}

	p_obj_id = (uint8_t *)Parameter_List[0].String_Value;
    object_id_length = strlen((char *)p_obj_id);
	p_new_obj_id = (uint8_t *)Parameter_List[1].String_Value;
    new_object_id_length = strlen((char *)p_new_obj_id);

    qapi_Status_t status = crypto_helper_persistent_obj_rename(
    		p_obj_id,
			object_id_length,
			p_new_obj_id,
			new_object_id_length,
			QAPI_CRYPTO_DATA_ACCESS_WRITE_META);
	if(status != QAPI_OK) {
		CRYPTO_PRINTF("Rename failed (status=%d)\n", status);
		return QCLI_STATUS_ERROR_E;
	} else {
		CRYPTO_PRINTF("Renamed object.\n");
		return QCLI_STATUS_SUCCESS_E;
	}
}

QCLI_Command_Status_t persistent_obj_demo_clone_command(uint32_t Parameter_Count, QCLI_Parameter_t * Parameter_List)
{
	uint8_t * p_obj_id = NULL;
	size_t object_id_length = 0;
	uint8_t * p_new_obj_id = NULL;
	size_t new_object_id_length = 0;

	if(Parameter_Count != 2) {
		CRYPTO_PRINTF("ERROR: invalid number of arguments\r\n");
		return QCLI_STATUS_USAGE_E;
	}

	p_obj_id = (uint8_t *)Parameter_List[0].String_Value;
    object_id_length = strlen((char *)p_obj_id);
	p_new_obj_id = (uint8_t *)Parameter_List[1].String_Value;
    new_object_id_length = strlen((char *)p_new_obj_id);

    qapi_Status_t status = crypto_helper_persistent_obj_clone(
    		p_obj_id,
			object_id_length,
			p_new_obj_id,
			new_object_id_length);
	if(status != QAPI_OK) {
		CRYPTO_PRINTF("Clone failed (status=%d)\n", status);
		return QCLI_STATUS_ERROR_E;
	} else {
		CRYPTO_PRINTF("Clone object.\n");
		return QCLI_STATUS_SUCCESS_E;
	}
}

QCLI_Command_Status_t persistent_obj_demo_create_command(uint32_t Parameter_Count, QCLI_Parameter_t * Parameter_List)
{
	qapi_Status_t status = QAPI_OK;
	char * obj_id = NULL;
    uint8_t * p_key_buffer = 0;
    uint32_t key_buffer_size = 0;
    char * hostname = 0;
    char * filename = 0;
    char * file_type = 0;
    uint32_t obj_type = 0;
    int parameter_index = 0;
    uint16_t port = 1443;
	uint32_t flags = 0;

    for ( parameter_index = 0; parameter_index < Parameter_Count; parameter_index++ )
    {
        if ( 0 == strcmp((char*)Parameter_List[parameter_index].String_Value, "-h") )
        {
            parameter_index++;
            if ( parameter_index == Parameter_Count ) {
            	CRYPTO_PRINTF("ERROR: -h option is present, but no hostname is specified\r\n");
            	return QCLI_STATUS_USAGE_E;
            }
            hostname = (char*)Parameter_List[parameter_index].String_Value;
        }
        else if ( 0 == strcmp((char*)Parameter_List[parameter_index].String_Value, "-t") )
        {
            parameter_index++;
            if ( parameter_index == Parameter_Count ) {
            	CRYPTO_PRINTF("ERROR: -t option is present, but no type is specified\r\n");
            	return QCLI_STATUS_USAGE_E;
            }
            file_type = (char*)Parameter_List[parameter_index].String_Value;
        }
        else if ( 0 == strcmp((char*)Parameter_List[parameter_index].String_Value, "-f") )
        {
            parameter_index++;
            if ( parameter_index == Parameter_Count ) {
            	CRYPTO_PRINTF("ERROR: -f option is present, but no key filename is specified\r\n");
            	return QCLI_STATUS_USAGE_E;
            }
            filename = (char*)Parameter_List[parameter_index].String_Value;
        }
        else if ( 0 == strcmp((char*)Parameter_List[parameter_index].String_Value, "-i") )
        {
            parameter_index++;
            if ( parameter_index == Parameter_Count ) {
            	CRYPTO_PRINTF("ERROR: -i option is present, but no id is specified\r\n");
            	return QCLI_STATUS_USAGE_E;
            }
			obj_id = Parameter_List[parameter_index].String_Value;
        }
        else if ( 0 == strcmp((char*)Parameter_List[parameter_index].String_Value, "-o") )
        {
            parameter_index++;
            if ( parameter_index == Parameter_Count ) {
            	CRYPTO_PRINTF("ERROR: -o option is present, but no object type is specified\r\n");
            	return QCLI_STATUS_USAGE_E;
            }
            obj_type = strtoul(Parameter_List[parameter_index].String_Value, NULL, 16);
        }
		else if ( 0 == strcmp((char*)Parameter_List[parameter_index].String_Value, "-l") )
        {
            parameter_index++;
            if ( parameter_index == Parameter_Count ) {
            	CRYPTO_PRINTF("ERROR: -l option is present, but no flags is specified\r\n");
            	return QCLI_STATUS_USAGE_E;
            }
            flags = Parameter_List[parameter_index].Integer_Value;
        }
        else
        {
        	CRYPTO_PRINTF("ERROR: Unknown option (%s)\r\n", (char*)Parameter_List[parameter_index].String_Value);
        	return QCLI_STATUS_USAGE_E;
        }
    }

	if(obj_type == 0) {
		CRYPTO_PRINTF("ERROR: Must specify object type\n");
		return QCLI_STATUS_USAGE_E;
	}
	if(obj_id == NULL) {
		CRYPTO_PRINTF("ERROR: Must specify an object id\n");
		return QCLI_STATUS_USAGE_E;
	}
	if(filename == NULL) {
		CRYPTO_PRINTF("ERROR: Must specify a filename\n");
		return QCLI_STATUS_USAGE_E;
	}
	if(file_type == NULL) {
		CRYPTO_PRINTF("ERROR: Must specify a file type\n");
		return QCLI_STATUS_USAGE_E;
	}

	status = persistent_obj_demo_load_file(hostname,
			port,
			filename,
			&p_key_buffer,
			&key_buffer_size);
	if(status != QAPI_OK) {
	    goto cleanup;
	}

	if (0 ||
		(obj_type == QAPI_CRYPTO_OBJ_TYPE_RSA_KEYPAIR_E) ||
		(obj_type == QAPI_CRYPTO_OBJ_TYPE_RSA_PUBLIC_KEY_E) ||
		(obj_type == QAPI_CRYPTO_OBJ_TYPE_ECDSA_KEYPAIR_E) ||
		(obj_type == QAPI_CRYPTO_OBJ_TYPE_ECDSA_PUBLIC_KEY_E)
		)
	{
		if(strcmp(file_type, "PEM") != 0) {
			status = QAPI_ERROR;
			CRYPTO_PRINTF("ERROR: invalid file type for object type.\n");
			goto cleanup;
		}

		status = crypto_helper_persistent_obj_create_asymmetric_key_from_pem(
				obj_type,
				flags,
				(uint8_t*)obj_id,
				strlen(obj_id),
				(char*)p_key_buffer,
				NULL);
	}
	else if (0 ||
		(obj_type == QAPI_CRYPTO_OBJ_TYPE_AES_E) ||
		(obj_type == QAPI_CRYPTO_OBJ_TYPE_HMAC_MD5_E) ||
		(obj_type == QAPI_CRYPTO_OBJ_TYPE_HMAC_SHA1_E) ||
		(obj_type == QAPI_CRYPTO_OBJ_TYPE_HMAC_SHA224_E) ||
		(obj_type == QAPI_CRYPTO_OBJ_TYPE_HMAC_SHA256_E) ||
		(obj_type == QAPI_CRYPTO_OBJ_TYPE_HMAC_SHA384_E) ||
		(obj_type == QAPI_CRYPTO_OBJ_TYPE_HMAC_SHA512_E)
		)
	{
		if(strcmp(file_type, "RAW") != 0) {
			status = QAPI_ERROR;
			CRYPTO_PRINTF("ERROR: invalid file type for object type.\n");
			goto cleanup;
		}

		status = crypto_helper_persistent_obj_create_symmetric_key(
				obj_type,
				flags,
				(uint8_t*)obj_id,
				strlen(obj_id),
				p_key_buffer,
				8 * key_buffer_size);
	}
	else
	{
	   status = QAPI_ERR_INVALID_PARAM;
	   CRYPTO_PRINTF("ERROR: unsupported object type\r\n");
	}

cleanup:

	if(p_key_buffer) {
		qapi_Crypto_Secure_Memzero(p_key_buffer, key_buffer_size);
		free(p_key_buffer);
	}

	if ( status != QAPI_OK ) {
		CRYPTO_PRINTF("ERROR: create command failed, status=%d\r\n", status);
		return QCLI_STATUS_ERROR_E;
	}

	return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t persistent_obj_demo_import_command(uint32_t Parameter_Count, QCLI_Parameter_t * Parameter_List)
{
	qapi_Status_t status = QAPI_OK;
	char * certificate_name = NULL;
    uint32_t obj_type = 0;
    uint32_t obj_usage = 0;
    int parameter_index = 0;
	uint32_t flags = 0;
	qapi_Crypto_Obj_Hdl_t hdl;
	char * obj_id = NULL;

    for ( parameter_index = 0; parameter_index < Parameter_Count; parameter_index++ )
    {
        if ( 0 == strcmp((char*)Parameter_List[parameter_index].String_Value, "-c") )
        {
            parameter_index++;
            if ( parameter_index == Parameter_Count ) {
            	CRYPTO_PRINTF("ERROR: -c option is present, but no certificate name is specified\r\n");
            	return QCLI_STATUS_USAGE_E;
            }
            certificate_name = (char*)Parameter_List[parameter_index].String_Value;
        }
        else if ( 0 == strcmp((char*)Parameter_List[parameter_index].String_Value, "-i") )
        {
            parameter_index++;
            if ( parameter_index == Parameter_Count ) {
            	CRYPTO_PRINTF("ERROR: -i option is present, but no id is specified\r\n");
            	return QCLI_STATUS_USAGE_E;
            }
			obj_id = Parameter_List[parameter_index].String_Value;
        }
        else if ( 0 == strcmp((char*)Parameter_List[parameter_index].String_Value, "-u") )
        {
            parameter_index++;
            if ( parameter_index == Parameter_Count ) {
            	CRYPTO_PRINTF("ERROR: -u option is present, but no object usage is specified\r\n");
            	return QCLI_STATUS_USAGE_E;
            }
            obj_usage = strtoul(Parameter_List[parameter_index].String_Value, NULL, 16);
        }
        else if ( 0 == strcmp((char*)Parameter_List[parameter_index].String_Value, "-o") )
        {
            parameter_index++;
            if ( parameter_index == Parameter_Count ) {
            	CRYPTO_PRINTF("ERROR: -o option is present, but no object type is specified\r\n");
            	return QCLI_STATUS_USAGE_E;
            }
            obj_type = strtoul(Parameter_List[parameter_index].String_Value, NULL, 16);
        }
		else if ( 0 == strcmp((char*)Parameter_List[parameter_index].String_Value, "-l") )
        {
            parameter_index++;
            if ( parameter_index == Parameter_Count ) {
            	CRYPTO_PRINTF("ERROR: -l option is present, but no flags is specified\r\n");
            	return QCLI_STATUS_USAGE_E;
            }
            flags = Parameter_List[parameter_index].Integer_Value;
        }
        else
        {
        	CRYPTO_PRINTF("ERROR: Unknown option (%s)\r\n", (char*)Parameter_List[parameter_index].String_Value);
        	return QCLI_STATUS_USAGE_E;
        }
    }

	if(certificate_name == NULL) {
		CRYPTO_PRINTF("ERROR: Must specify a certificate name\n");
		return QCLI_STATUS_USAGE_E;
	}
	if(obj_id == NULL) {
		CRYPTO_PRINTF("ERROR: Must specify an object id\n");
		return QCLI_STATUS_USAGE_E;
	}

	if (0 ||
		(obj_type == 0) || // Object type can be left unspecified and instead inferred from the certificate
		(obj_type == QAPI_CRYPTO_OBJ_TYPE_RSA_KEYPAIR_E) ||
		(obj_type == QAPI_CRYPTO_OBJ_TYPE_RSA_PUBLIC_KEY_E) ||
		(obj_type == QAPI_CRYPTO_OBJ_TYPE_ECDSA_KEYPAIR_E) ||
		(obj_type == QAPI_CRYPTO_OBJ_TYPE_ECDSA_PUBLIC_KEY_E) ||
		(obj_type == QAPI_CRYPTO_OBJ_TYPE_ECDH_KEYPAIR_E)
		)
	{

		status =  qapi_Crypto_Persistent_Obj_Import_From_Cert_Store(
				QAPI_CRYPTO_PERSISTENT_OBJ_DATA_TEE_STORAGE_PRIVATE,
				(uint8_t*)obj_id,
				strlen(obj_id),
				certificate_name,
				flags,
				obj_type,
				obj_usage,
				&hdl);

		// Close the handle to object, the command creates the object but
		// does not use it
		if(status == QAPI_OK) {
			qapi_Crypto_Persistent_Obj_Close(hdl);
		}

		if ( status != QAPI_OK ) {
			CRYPTO_PRINTF("ERROR: import command failed, status=%d\r\n", status);
			return QCLI_STATUS_ERROR_E;
		}
	}
	else
	{
	   status = QAPI_ERR_INVALID_PARAM;
	   CRYPTO_PRINTF("ERROR: unsupported object type\r\n");
	}

	return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t persistent_obj_demo_encrypt_command(uint32_t Parameter_Count, QCLI_Parameter_t * Parameter_List)
{
	qapi_Status_t status = QAPI_OK;
	char * p_obj_id = NULL;
	size_t object_id_length = 0;
    uint8_t * p_plain_text = 0;
    uint32_t plain_text_length = 0;
    char * hostname = 0;
    char * filename = 0;
    uint32_t algorithm_id = 0;
    int parameter_index = 0;
    uint16_t port = 1443;
    uint8_t *p_cipher_text = NULL;
    uint32_t cipher_text_length = 0;
    const char *iv_filename = NULL;
    uint32_t iv_length = 0;
    uint8_t *p_iv = NULL;
    const char *nonce_filename = NULL;
    uint32_t nonce_length = 0;
    uint8_t *p_nonce = NULL;
    const char *aad_filename = NULL;
    uint32_t aad_length = 0;
    uint8_t *p_aad = NULL;
	uint8_t *p_tag_generated_from_encrypt = NULL;
	uint32_t tag_length_in_bits = 128;

    for ( parameter_index = 0; parameter_index < Parameter_Count; parameter_index++ )
    {
        if ( 0 == strcmp((char*)Parameter_List[parameter_index].String_Value, "-h") )
        {
            parameter_index++;
            if ( parameter_index == Parameter_Count ) {
            	CRYPTO_PRINTF("ERROR: -h option is present, but no host name is specified\r\n");
            	return QCLI_STATUS_USAGE_E;
            }
            hostname = (char*)Parameter_List[parameter_index].String_Value;
        }
        else if ( 0 == strcmp((char*)Parameter_List[parameter_index].String_Value, "-f") )
        {
            parameter_index++;
            if ( parameter_index == Parameter_Count ) {
            	CRYPTO_PRINTF("ERROR: -f option is present, but no filename is specified\r\n");
            	return QCLI_STATUS_USAGE_E;
            }
            filename = (char*)Parameter_List[parameter_index].String_Value;
        }
        else if ( 0 == strcmp((char*)Parameter_List[parameter_index].String_Value, "-i") )
        {
            parameter_index++;
            if ( parameter_index == Parameter_Count ) {
            	CRYPTO_PRINTF("ERROR: -i option is present, but no id is specified\r\n");
            	return QCLI_STATUS_USAGE_E;
            }
			p_obj_id = Parameter_List[parameter_index].String_Value;
            object_id_length = strlen(p_obj_id);
        }
        else if ( 0 == strcmp((char*)Parameter_List[parameter_index].String_Value, "-v") )
        {
            parameter_index++;
            if ( parameter_index == Parameter_Count ) {
            	CRYPTO_PRINTF("ERROR: -v option is present, but no iv file is specified\r\n");
            	return QCLI_STATUS_USAGE_E;
            }
            iv_filename = Parameter_List[parameter_index].String_Value;
        }
        else if ( 0 == strcmp((char*)Parameter_List[parameter_index].String_Value, "-n") )
        {
            parameter_index++;
            if ( parameter_index == Parameter_Count ) {
            	CRYPTO_PRINTF("ERROR: -n option is present, but no nonce file is specified\r\n");
            	return QCLI_STATUS_USAGE_E;
            }
            nonce_filename = Parameter_List[parameter_index].String_Value;
        }
        else if ( 0 == strcmp((char*)Parameter_List[parameter_index].String_Value, "-d") )
        {
            parameter_index++;
            if ( parameter_index == Parameter_Count ) {
            	CRYPTO_PRINTF("ERROR: -d option is present, but no aad file is specified\r\n");
            	return QCLI_STATUS_USAGE_E;
            }
            aad_filename = Parameter_List[parameter_index].String_Value;
        }
        else if ( 0 == strcmp((char*)Parameter_List[parameter_index].String_Value, "-a") )
        {
            parameter_index++;
            if ( parameter_index == Parameter_Count ) {
            	CRYPTO_PRINTF("ERROR: -a option is present, but no algorithm id is specified\r\n");
            	return QCLI_STATUS_USAGE_E;
            }
            algorithm_id = strtoul(Parameter_List[parameter_index].String_Value, NULL, 16);
        }
        else if ( 0 == strcmp((char*)Parameter_List[parameter_index].String_Value, "-l") )
        {
            parameter_index++;
            if ( parameter_index == Parameter_Count ) {
            	CRYPTO_PRINTF("ERROR: -l option is present, but no tag length is specified\r\n");
            	return QCLI_STATUS_USAGE_E;
            }
            tag_length_in_bits = Parameter_List[parameter_index].Integer_Value;
        }
        else
        {
        	CRYPTO_PRINTF("ERROR: Unknown option (%s)\r\n", (char*)Parameter_List[parameter_index].String_Value);
        	return QCLI_STATUS_USAGE_E;
        }
    }

	if(algorithm_id == 0) {
		CRYPTO_PRINTF("ERROR: Must specify algorithm id parameter\n");
		return QCLI_STATUS_USAGE_E;
	}
	if(p_obj_id == NULL || object_id_length == 0) {
		CRYPTO_PRINTF("ERROR: Must specify an object id\n");
		return QCLI_STATUS_USAGE_E;
	}
	if(filename == NULL) {
		CRYPTO_PRINTF("ERROR: Must specify a file\n");
		return QCLI_STATUS_USAGE_E;
	}

	status = persistent_obj_demo_load_file(hostname,
			port,
			filename,
			&p_plain_text,
			&plain_text_length);
	if(status != QAPI_OK) {
	    goto cleanup;
	}

	if(iv_filename) {
		status = persistent_obj_demo_load_file(hostname,
				port,
				iv_filename,
				&p_iv,
				&iv_length);
		if(status != QAPI_OK) {
		    goto cleanup;
		}
	}

	if(nonce_filename) {
		status = persistent_obj_demo_load_file(hostname,
				port,
				nonce_filename,
				&p_nonce,
				&nonce_length);
		if(status != QAPI_OK) {
		    goto cleanup;
		}
	}

	if(aad_filename) {
		status = persistent_obj_demo_load_file(hostname,
				port,
				aad_filename,
				&p_aad,
				&aad_length);
		if(status != QAPI_OK) {
		    goto cleanup;
		}
	}

	if (0 ||
		(algorithm_id == QAPI_CRYPTO_ALG_RSAES_PKCS1_V1_5_E) ||
		(algorithm_id == QAPI_CRYPTO_ALG_RSA_NOPAD_E)
		)
	{
		status = crypto_helper_persistent_obj_asym_encrypt (
				(uint8_t*)p_obj_id,
				strlen(p_obj_id),
				0,
				p_plain_text,
				plain_text_length,
				&p_cipher_text,
				&cipher_text_length,
				algorithm_id);
		if(status != QAPI_OK) {
			goto cleanup;
		}
	}
	else if (0 ||
		(algorithm_id == QAPI_CRYPTO_ALG_AES_CBC_NOPAD_E) ||
		(algorithm_id == QAPI_CRYPTO_ALG_AES_CTR_E)
		)
	{
		status = crypto_helper_persistent_obj_cipher_encrypt (
				(uint8_t*)p_obj_id,
				strlen(p_obj_id),
				0,
				algorithm_id,
				p_iv,
				iv_length,
				p_plain_text,
				plain_text_length,
				&p_cipher_text,
				&cipher_text_length);
		if(status != QAPI_OK) {
			goto cleanup;
		}
	}
	else if (0 ||
		(algorithm_id == QAPI_CRYPTO_ALG_AES_CCM_E) ||
		(algorithm_id == QAPI_CRYPTO_ALG_AES_GCM_E)
		)
	{
		if((algorithm_id == QAPI_CRYPTO_ALG_AES_GCM_E) &&
		   (tag_length_in_bits != 128))
		{
			CRYPTO_PRINTF("\r\nERROR: unsupported tag length\r\n");
		}
		else if(algorithm_id == QAPI_CRYPTO_ALG_AES_CCM_E)
		{
			if((tag_length_in_bits < 32) || (tag_length_in_bits > 128) || ((tag_length_in_bits & 0x7) != 0)) {
				CRYPTO_PRINTF("\r\nERROR: unsupported tag length\r\n");
			}
		}

		p_tag_generated_from_encrypt = (uint8_t*)malloc(tag_length_in_bits / 8);
		if(p_tag_generated_from_encrypt == NULL) {
			status = QAPI_ERR_NO_MEMORY;
			goto cleanup;
		}

		status = crypto_helper_persistent_obj_ae_encrypt (
				(uint8_t*) p_obj_id,
				strlen(p_obj_id),
				0,
				algorithm_id,
				p_nonce,
				nonce_length,
				p_tag_generated_from_encrypt,
				&tag_length_in_bits,
				p_aad,
				aad_length,
				p_plain_text,
				plain_text_length,
				&p_cipher_text,
				&cipher_text_length);
		if(status != QAPI_OK) {
			goto cleanup;
		}

		CRYPTO_PRINTF("Tag:\n");
		persistent_obj_demo_print_binary_buffer(p_tag_generated_from_encrypt, tag_length_in_bits / 8);
	}
	else
	{
	   status = QAPI_ERR_INVALID_PARAM;
	   CRYPTO_PRINTF("ERROR: unsupported object type\r\n");
	   goto cleanup;
	}

	CRYPTO_PRINTF("Ciphertext:\n");
	persistent_obj_demo_print_binary_buffer(p_cipher_text, cipher_text_length);

cleanup:

	if(p_plain_text) {
		qapi_Crypto_Secure_Memzero(p_plain_text, plain_text_length);
		free(p_plain_text);
	}

	if(p_tag_generated_from_encrypt) {
		free(p_tag_generated_from_encrypt);
	}

	if(p_nonce) {
		free(p_nonce);
	}

	if(p_aad) {
		free(p_aad);
	}

	if(p_iv) {
		free(p_iv);
	}

	if ( status != QAPI_OK ) {
		CRYPTO_PRINTF("ERROR: encrypt command failed, status=%d\r\n", status);
		return QCLI_STATUS_ERROR_E;
	}

	return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t persistent_obj_demo_decrypt_command(uint32_t Parameter_Count, QCLI_Parameter_t * Parameter_List)
{
	qapi_Status_t status = QAPI_OK;
	char * p_obj_id = NULL;
	size_t object_id_length = 0;
    uint8_t * p_plain_text = 0;
    uint32_t plain_text_length = 0;
    char * hostname = 0;
    char * filename = 0;
    uint32_t algorithm_id = 0;
    int parameter_index = 0;
    uint16_t port = 1443;
    uint8_t *p_cipher_text = NULL;
    uint32_t cipher_text_length = 0;
    const char *iv_filename = NULL;
    uint32_t iv_length = 0;
    uint8_t *p_iv = NULL;
    const char *nonce_filename = NULL;
    uint32_t nonce_length = 0;
    uint8_t *p_nonce = NULL;
    const char *aad_filename = NULL;
    uint32_t aad_length = 0;
    uint8_t *p_aad = NULL;
    const char *tag_filename = NULL;
    uint32_t tag_length = 0;
    uint8_t *p_tag = NULL;

    for ( parameter_index = 0; parameter_index < Parameter_Count; parameter_index++ )
    {
        if ( 0 == strcmp((char*)Parameter_List[parameter_index].String_Value, "-h") )
        {
            parameter_index++;
            if ( parameter_index == Parameter_Count ) {
            	CRYPTO_PRINTF("ERROR: -h option is present, but no hostname is specified\r\n");
            	return QCLI_STATUS_USAGE_E;
            }
            hostname = (char*)Parameter_List[parameter_index].String_Value;
        }
        else if ( 0 == strcmp((char*)Parameter_List[parameter_index].String_Value, "-f") )
        {
            parameter_index++;
            if ( parameter_index == Parameter_Count ) {
            	CRYPTO_PRINTF("ERROR: -f option is present, but no key filename is specified\r\n");
            	return QCLI_STATUS_USAGE_E;
            }
            filename = (char*)Parameter_List[parameter_index].String_Value;
        }
        else if ( 0 == strcmp((char*)Parameter_List[parameter_index].String_Value, "-i") )
        {
            parameter_index++;
            if ( parameter_index == Parameter_Count ) {
            	CRYPTO_PRINTF("ERROR: -i option is present, but no id is specified\r\n");
            	return QCLI_STATUS_USAGE_E;
            }
			p_obj_id = Parameter_List[parameter_index].String_Value;
            object_id_length = strlen(p_obj_id);
        }
        else if ( 0 == strcmp((char*)Parameter_List[parameter_index].String_Value, "-v") )
        {
            parameter_index++;
            if ( parameter_index == Parameter_Count ) {
            	CRYPTO_PRINTF("ERROR: -v option is present, but no iv file is specified\r\n");
            	return QCLI_STATUS_USAGE_E;
            }
            iv_filename = Parameter_List[parameter_index].String_Value;
        }
        else if ( 0 == strcmp((char*)Parameter_List[parameter_index].String_Value, "-n") )
        {
            parameter_index++;
            if ( parameter_index == Parameter_Count ) {
            	CRYPTO_PRINTF("ERROR: -n option is present, but no nonce file is specified\r\n");
            	return QCLI_STATUS_USAGE_E;
            }
            nonce_filename = Parameter_List[parameter_index].String_Value;
        }
        else if ( 0 == strcmp((char*)Parameter_List[parameter_index].String_Value, "-d") )
        {
            parameter_index++;
            if ( parameter_index == Parameter_Count ) {
            	CRYPTO_PRINTF("ERROR: -d option is present, but no aad file is specified\r\n");
            	return QCLI_STATUS_USAGE_E;
            }
            aad_filename = Parameter_List[parameter_index].String_Value;
        }
        else if ( 0 == strcmp((char*)Parameter_List[parameter_index].String_Value, "-a") )
        {
            parameter_index++;
            if ( parameter_index == Parameter_Count ) {
            	CRYPTO_PRINTF("ERROR: -a option is present, but no algorithm id is specified\r\n");
            	return QCLI_STATUS_USAGE_E;
            }
            algorithm_id = strtoul(Parameter_List[parameter_index].String_Value, NULL, 16);
        }
        else if ( 0 == strcmp((char*)Parameter_List[parameter_index].String_Value, "-t") )
        {
            parameter_index++;
            if ( parameter_index == Parameter_Count ) {
            	CRYPTO_PRINTF("ERROR: -t option is present, but no tag file is specified\r\n");
            	return QCLI_STATUS_USAGE_E;
            }
            tag_filename = Parameter_List[parameter_index].String_Value;
        }
        else if ( 0 == strcmp((char*)Parameter_List[parameter_index].String_Value, "-d") )
        {
            parameter_index++;
            if ( parameter_index == Parameter_Count ) {
            	CRYPTO_PRINTF("ERROR: -d option is present, but no aad file is specified\r\n");
            	return QCLI_STATUS_USAGE_E;
            }
            aad_filename = Parameter_List[parameter_index].String_Value;
        }
        else
        {
        	CRYPTO_PRINTF("ERROR: Unknown option (%s)\r\n", (char*)Parameter_List[parameter_index].String_Value);
        	return QCLI_STATUS_USAGE_E;
        }
    }

	if(algorithm_id == 0) {
		CRYPTO_PRINTF("ERROR: Must specify algorithm id\n");
		return QCLI_STATUS_USAGE_E;
	}
	if(p_obj_id == NULL || object_id_length == 0) {
		CRYPTO_PRINTF("ERROR: Must specify an object id\n");
		return QCLI_STATUS_USAGE_E;
	}
	if(filename == NULL) {
		CRYPTO_PRINTF("ERROR: Must specify a filename\n");
		return QCLI_STATUS_USAGE_E;
	}

	status = persistent_obj_demo_load_file(hostname,
			port,
			filename,
			&p_cipher_text,
			&cipher_text_length);
	if(status != QAPI_OK) {
	    goto cleanup;
	}

	if(iv_filename) {
		status = persistent_obj_demo_load_file(hostname,
				port,
				iv_filename,
				&p_iv,
				&iv_length);
		if(status != QAPI_OK) {
			goto cleanup;
		}
	}

	if(tag_filename) {
		status = persistent_obj_demo_load_file(hostname,
				port,
				tag_filename,
				&p_tag,
				&tag_length);
		if(status != QAPI_OK) {
		    goto cleanup;
		}
	}

	if(nonce_filename) {
		status = persistent_obj_demo_load_file(hostname,
				port,
				nonce_filename,
				&p_nonce,
				&nonce_length);
		if(status != QAPI_OK) {
		    goto cleanup;
		}
	}

	if(aad_filename) {
		status = persistent_obj_demo_load_file(hostname,
				port,
				aad_filename,
				&p_aad,
				&aad_length);
		if(status != QAPI_OK) {
		    goto cleanup;
		}
	}

	if (0 ||
		(algorithm_id == QAPI_CRYPTO_ALG_RSAES_PKCS1_V1_5_E) ||
		(algorithm_id == QAPI_CRYPTO_ALG_RSA_NOPAD_E)
		)
	{
		status = crypto_helper_persistent_obj_asym_decrypt (
				(uint8_t*)p_obj_id,
				strlen(p_obj_id),
				0,
				p_cipher_text,
				cipher_text_length,
				algorithm_id,
				&p_plain_text,
				&plain_text_length);
		if(status != QAPI_OK) {
			goto cleanup;
		}
	}
	else if (0 ||
		(algorithm_id == QAPI_CRYPTO_ALG_AES_CBC_NOPAD_E) ||
		(algorithm_id == QAPI_CRYPTO_ALG_AES_CTR_E)
		)
	{

		status = crypto_helper_persistent_obj_cipher_decrypt (
				(uint8_t*)p_obj_id,
				strlen(p_obj_id),
				0,
				algorithm_id,
				p_iv,
				iv_length,
				p_cipher_text,
				cipher_text_length,
				&p_plain_text,
				&plain_text_length);
		if(status != QAPI_OK) {
			goto cleanup;
		}
	}
	else if (0 ||
		(algorithm_id == QAPI_CRYPTO_ALG_AES_CCM_E) ||
		(algorithm_id == QAPI_CRYPTO_ALG_AES_GCM_E)
		)
	{
		status = crypto_helper_persistent_obj_ae_decrypt (
				(uint8_t*) p_obj_id,
				strlen(p_obj_id),
				0,
				algorithm_id,
				p_nonce,
				nonce_length,
				p_tag,
				tag_length * 8,
				p_aad,
				aad_length,
				p_cipher_text,
				cipher_text_length,
				&p_plain_text,
				&plain_text_length);
		if(status != QAPI_OK) {
			goto cleanup;
		}
	}
	else
	{
	   status = QAPI_ERR_INVALID_PARAM;
	   CRYPTO_PRINTF("ERROR: unsupported algorithm id: %d\r\n", algorithm_id);
	   goto cleanup;
	}

	CRYPTO_PRINTF("Plaintext:\n");
	persistent_obj_demo_print_binary_buffer(p_plain_text, plain_text_length);

cleanup:

	if(p_plain_text) {
		qapi_Crypto_Secure_Memzero(p_plain_text, plain_text_length);
		free(p_plain_text);
	}

	if(p_tag) {
		free(p_tag);
	}

	if(p_nonce) {
		free(p_nonce);
	}

	if(p_aad) {
		free(p_aad);
	}

	if(p_iv) {
		free(p_iv);
	}

	if ( status != QAPI_OK ) {
		CRYPTO_PRINTF("ERROR: decrypt command failed, status=%d\r\n", status);
		return QCLI_STATUS_ERROR_E;
	}

	return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t persistent_obj_demo_sign_command(uint32_t Parameter_Count, QCLI_Parameter_t * Parameter_List)
{
	qapi_Status_t status = QAPI_OK;
	char * p_obj_id = NULL;
    uint8_t * p_message = 0;
    uint32_t message_length = 0;
    uint8_t * p_digest = 0;
    uint32_t digest_length = 0;
    uint8_t * p_signature = 0;
    uint32_t signature_length = 0;
    char * hostname = 0;
    char * filename = 0;
    uint32_t signature_algorithm_id = 0;
    uint32_t hash_algorithm_id = 0;
    int parameter_index = 0;
    uint16_t port = 1443;

    for ( parameter_index = 0; parameter_index < Parameter_Count; parameter_index++ )
    {
        if ( 0 == strcmp((char*)Parameter_List[parameter_index].String_Value, "-h") )
        {
            parameter_index++;
            if ( parameter_index == Parameter_Count ) {
            	CRYPTO_PRINTF("ERROR: -h option is present, but no hostname is specified\r\n");
            	return QCLI_STATUS_USAGE_E;
            }
            hostname = (char*)Parameter_List[parameter_index].String_Value;
        }
        else if ( 0 == strcmp((char*)Parameter_List[parameter_index].String_Value, "-f") )
        {
            parameter_index++;
            if ( parameter_index == Parameter_Count ) {
            	CRYPTO_PRINTF("ERROR: -f option is present, but no key filename is specified\r\n");
            	return QCLI_STATUS_USAGE_E;
            }
            filename = (char*)Parameter_List[parameter_index].String_Value;
        }
        else if ( 0 == strcmp((char*)Parameter_List[parameter_index].String_Value, "-i") )
        {
            parameter_index++;
            if ( parameter_index == Parameter_Count ) {
            	CRYPTO_PRINTF("ERROR: -i option is present, but no id is specified\r\n");
            	return QCLI_STATUS_USAGE_E;
            }
			p_obj_id = Parameter_List[parameter_index].String_Value;
        }
        else if ( 0 == strcmp((char*)Parameter_List[parameter_index].String_Value, "-a") )
        {
            parameter_index++;
            if ( parameter_index == Parameter_Count ) {
            	CRYPTO_PRINTF("ERROR: -a option is present, but no signature algorithm id is specified\r\n");
            	return QCLI_STATUS_USAGE_E;
            }
            signature_algorithm_id = strtoul(Parameter_List[parameter_index].String_Value, NULL, 16);
        }
        else if ( 0 == strcmp((char*)Parameter_List[parameter_index].String_Value, "-d") )
        {
            parameter_index++;
            if ( parameter_index == Parameter_Count ) {
            	CRYPTO_PRINTF("ERROR: -d option is present, but no hash algorithm id is specified\r\n");
            	return QCLI_STATUS_USAGE_E;
            }
            hash_algorithm_id = strtoul(Parameter_List[parameter_index].String_Value, NULL, 16);
        }
        else if ( 0 == strcmp((char*)Parameter_List[parameter_index].String_Value, "-v") )
        {
            parameter_index++;
            if ( parameter_index == Parameter_Count ) {
            	CRYPTO_PRINTF("ERROR: -v option is present, but no iv file is specified\r\n");
            	return QCLI_STATUS_USAGE_E;
            }
        }
    	else
    	{
    	   CRYPTO_PRINTF("ERROR: unsupported parameter\r\n");
    	   return QCLI_STATUS_USAGE_E;
    	}
    }

	if(signature_algorithm_id == 0) {
		CRYPTO_PRINTF("ERROR: Must specify signature algorithm id\n");
		return QCLI_STATUS_USAGE_E;
	}
	if(p_obj_id == NULL) {
		CRYPTO_PRINTF("ERROR: Must specify an object id\n");
		return QCLI_STATUS_USAGE_E;
	}
	if(filename == NULL) {
		CRYPTO_PRINTF("ERROR: Must specify a filename\n");
		return QCLI_STATUS_USAGE_E;
	}

	status = persistent_obj_demo_load_file(hostname,
			port,
			filename,
			&p_message,
			&message_length);
	if(status != QAPI_OK) {
	    goto cleanup;
	}

	if (0 ||
		(signature_algorithm_id == QAPI_CRYPTO_ALG_RSASSA_PKCS1_V1_5_SHA1_E) ||
		(signature_algorithm_id == QAPI_CRYPTO_ALG_RSASSA_PKCS1_V1_5_SHA256_E) ||
		(signature_algorithm_id == QAPI_CRYPTO_ALG_RSASSA_PKCS1_V1_5_SHA384_E) ||
		(signature_algorithm_id == QAPI_CRYPTO_ALG_RSASSA_PKCS1_V1_5_SHA512_E) ||
		(signature_algorithm_id == QAPI_CRYPTO_ALG_ECDSA_P192_E) ||
		(signature_algorithm_id == QAPI_CRYPTO_ALG_ECDSA_P224_E) ||
		(signature_algorithm_id == QAPI_CRYPTO_ALG_ECDSA_P256_E) ||
		(signature_algorithm_id == QAPI_CRYPTO_ALG_ECDSA_P384_E) ||
		(signature_algorithm_id == QAPI_CRYPTO_ALG_ECDSA_P521_E)
		)
	{
		if(hash_algorithm_id == 0) {
			CRYPTO_PRINTF("ERROR: Must specify hash algorithm id\n");
			return QCLI_STATUS_USAGE_E;
		}

		status = crypto_helper_persistent_obj_hash_and_sign(
				(uint8_t *)p_obj_id,
				strlen(p_obj_id),
				0,
				p_message,
				message_length,
				hash_algorithm_id,
				signature_algorithm_id,
				&p_digest,
				&digest_length,
				&p_signature,
				&signature_length);
		if(status != QAPI_OK) {
			goto cleanup;
		}

		CRYPTO_PRINTF("Signature:\n");
		persistent_obj_demo_print_binary_buffer(p_signature, signature_length);
	}
	else if (0 ||
			 (signature_algorithm_id == QAPI_CRYPTO_ALG_HMAC_MD5_E) ||
			 (signature_algorithm_id == QAPI_CRYPTO_ALG_HMAC_SHA1_E) ||
			 (signature_algorithm_id == QAPI_CRYPTO_ALG_HMAC_SHA224_E) ||
			 (signature_algorithm_id == QAPI_CRYPTO_ALG_HMAC_SHA256_E) ||
			 (signature_algorithm_id == QAPI_CRYPTO_ALG_HMAC_SHA384_E) ||
			 (signature_algorithm_id == QAPI_CRYPTO_ALG_HMAC_SHA512_E)
			 )
	{
		status = crypto_helper_persistent_obj_compute_hmac(
				(uint8_t*)p_obj_id,
				strlen(p_obj_id),
				0,
				NULL,
				0,
				p_message,
				message_length,
				&p_signature,
				&signature_length);
		if(status != QAPI_OK) {
			goto cleanup;
		}

		CRYPTO_PRINTF("MAC:\n");
		persistent_obj_demo_print_binary_buffer(p_signature, signature_length);
	}

cleanup:

	if(p_signature) {
		free(p_signature);
	}

	if(p_message) {
		free(p_message);
	}

	if(p_digest) {
		free(p_digest);
	}

	if ( status != QAPI_OK ) {
		CRYPTO_PRINTF("ERROR: sign command failed, status=%d\r\n", status);
		return QCLI_STATUS_ERROR_E;
	}

	return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t persistent_obj_demo_verify_command(uint32_t Parameter_Count, QCLI_Parameter_t * Parameter_List)
{
	qapi_Status_t status = QAPI_OK;
	char * p_obj_id = NULL;
    uint8_t * p_message = 0;
    uint32_t message_length = 0;
    char *signature_filename = NULL;
    uint8_t * p_signature = 0;
    uint32_t signature_length = 0;
    uint32_t iv_length = 0;
    uint8_t *p_iv = NULL;
    char * hostname = NULL;
    char * message_filename = 0;
    uint32_t signature_algorithm_id = 0;
    uint32_t hash_algorithm_id = 0;
    int parameter_index = 0;
    uint16_t port = 1443;

    for ( parameter_index = 0; parameter_index < Parameter_Count; parameter_index++ )
    {
        if ( 0 == strcmp((char*)Parameter_List[parameter_index].String_Value, "-h") )
        {
            parameter_index++;
            if ( parameter_index == Parameter_Count ) {
            	CRYPTO_PRINTF("ERROR: -h option is present, but no hostname is specified\r\n");
            	return QCLI_STATUS_USAGE_E;
            }
            hostname = (char*)Parameter_List[parameter_index].String_Value;
        }
        else if ( 0 == strcmp((char*)Parameter_List[parameter_index].String_Value, "-f") )
        {
            parameter_index++;
            if ( parameter_index == Parameter_Count ) {
            	CRYPTO_PRINTF("ERROR: -f option is present, but no key filename is specified\r\n");
            	return QCLI_STATUS_USAGE_E;
            }
            message_filename = (char*)Parameter_List[parameter_index].String_Value;
        }
        else if ( 0 == strcmp((char*)Parameter_List[parameter_index].String_Value, "-s") )
        {
            parameter_index++;
            if ( parameter_index == Parameter_Count ) {
            	CRYPTO_PRINTF("ERROR: -s option is present, but no signature filename is specified\r\n");
            	return QCLI_STATUS_USAGE_E;
            }
            signature_filename = (char*)Parameter_List[parameter_index].String_Value;
        }
        else if ( 0 == strcmp((char*)Parameter_List[parameter_index].String_Value, "-i") )
        {
            parameter_index++;
            if ( parameter_index == Parameter_Count ) {
            	CRYPTO_PRINTF("ERROR: -i option is present, but no id is specified\r\n");
            	return QCLI_STATUS_USAGE_E;
            }
			p_obj_id = Parameter_List[parameter_index].String_Value;
        }
        else if ( 0 == strcmp((char*)Parameter_List[parameter_index].String_Value, "-a") )
        {
            parameter_index++;
            if ( parameter_index == Parameter_Count ) {
            	CRYPTO_PRINTF("ERROR: -a option is present, but no signature algorithm id is specified\r\n");
            	return QCLI_STATUS_USAGE_E;
            }
            signature_algorithm_id = strtoul(Parameter_List[parameter_index].String_Value, NULL, 16);
        }
        else if ( 0 == strcmp((char*)Parameter_List[parameter_index].String_Value, "-d") )
        {
            parameter_index++;
            if ( parameter_index == Parameter_Count ) {
            	CRYPTO_PRINTF("ERROR: -d option is present, but no hash algorithm id is specified\r\n");
            	return QCLI_STATUS_USAGE_E;
            }
            hash_algorithm_id = strtoul(Parameter_List[parameter_index].String_Value, NULL, 16);
        }
        else if ( 0 == strcmp((char*)Parameter_List[parameter_index].String_Value, "-v") )
        {
            parameter_index++;
            if ( parameter_index == Parameter_Count ) {
            	CRYPTO_PRINTF("ERROR: -v option is present, but no iv file is specified\r\n");
            	return QCLI_STATUS_USAGE_E;
            }
        }
    	else
    	{
    	   CRYPTO_PRINTF("ERROR: unsupported parameter\r\n");
    	   return QCLI_STATUS_USAGE_E;
    	}
    }

	if(signature_algorithm_id == 0) {
		CRYPTO_PRINTF("ERROR: Must specify signature algorithm id parameter\n");
		return QCLI_STATUS_USAGE_E;
	}
	if(p_obj_id == NULL) {
		CRYPTO_PRINTF("ERROR: Must specify an object id\n");
		return QCLI_STATUS_USAGE_E;
	}
	if(message_filename == NULL) {
		CRYPTO_PRINTF("ERROR: Must specify a message file\n");
		return QCLI_STATUS_USAGE_E;
	}
	if(signature_filename == NULL) {
		CRYPTO_PRINTF("ERROR: Must specify a signature file\n");
		return QCLI_STATUS_USAGE_E;
	}

	status = persistent_obj_demo_load_file(hostname,
			port,
			message_filename,
			&p_message,
			&message_length);
	if(status != QAPI_OK) {
	    goto cleanup;
	}

	status = persistent_obj_demo_load_file(hostname,
			port,
			signature_filename,
			&p_signature,
			&signature_length);
	if(status != QAPI_OK) {
	    goto cleanup;
	}

	if (0 ||
		(signature_algorithm_id == QAPI_CRYPTO_ALG_RSASSA_PKCS1_V1_5_SHA1_E) ||
		(signature_algorithm_id == QAPI_CRYPTO_ALG_RSASSA_PKCS1_V1_5_SHA256_E) ||
		(signature_algorithm_id == QAPI_CRYPTO_ALG_RSASSA_PKCS1_V1_5_SHA384_E) ||
		(signature_algorithm_id == QAPI_CRYPTO_ALG_RSASSA_PKCS1_V1_5_SHA512_E) ||
		(signature_algorithm_id == QAPI_CRYPTO_ALG_ECDSA_P192_E) ||
		(signature_algorithm_id == QAPI_CRYPTO_ALG_ECDSA_P224_E) ||
		(signature_algorithm_id == QAPI_CRYPTO_ALG_ECDSA_P256_E) ||
		(signature_algorithm_id == QAPI_CRYPTO_ALG_ECDSA_P384_E) ||
		(signature_algorithm_id == QAPI_CRYPTO_ALG_ECDSA_P521_E)
		)
	{
		if(hash_algorithm_id == 0) {
			CRYPTO_PRINTF("ERROR: Must specify hash algorithm id parameter\n");
			return QCLI_STATUS_USAGE_E;
		}

		status = crypto_helper_persistent_obj_hash_and_verify_signature(
				(uint8_t *)p_obj_id,
				strlen(p_obj_id),
				0,
				p_message,
				message_length,
				hash_algorithm_id,
				signature_algorithm_id,
				p_signature,
				signature_length);

		if(status == QAPI_OK) {
			CRYPTO_PRINTF("Signature verified.\n");
		} else if (status == QAPI_ERR_CRYPTO_INVALID_SIG) {
			CRYPTO_PRINTF("Bad signature.\n");
		}
	}
	else if (0 ||
			 (signature_algorithm_id == QAPI_CRYPTO_ALG_HMAC_MD5_E) ||
			 (signature_algorithm_id == QAPI_CRYPTO_ALG_HMAC_SHA1_E) ||
			 (signature_algorithm_id == QAPI_CRYPTO_ALG_HMAC_SHA224_E) ||
			 (signature_algorithm_id == QAPI_CRYPTO_ALG_HMAC_SHA256_E) ||
			 (signature_algorithm_id == QAPI_CRYPTO_ALG_HMAC_SHA384_E) ||
			 (signature_algorithm_id == QAPI_CRYPTO_ALG_HMAC_SHA512_E)
			 )
	{
		status = crypto_helper_persistent_obj_compute_and_compare_hmac(
				(uint8_t*)p_obj_id,
				strlen(p_obj_id),
				0,
				p_iv,
				iv_length,
				p_message,
				message_length,
				p_signature,
				signature_length);

		if(status == QAPI_OK) {
			CRYPTO_PRINTF("Mac verified.\n");
		} else if(status == QAPI_ERR_CRYPTO_INVALID_MAC){
			CRYPTO_PRINTF("Bad Mac.\n", status);
		}
	}

cleanup:

	if(p_signature) {
		free(p_signature);
	}

	if(p_message) {
		free(p_message);
	}

	if(p_iv) {
		free(p_iv);
	}

	if ( (status != QAPI_OK) &&
		 (status != QAPI_ERR_CRYPTO_INVALID_SIG) &&
		 (status != QAPI_ERR_CRYPTO_INVALID_MAC)
	   )
	{
		CRYPTO_PRINTF("ERROR: verify command failed, status=%d\r\n", status);
		return QCLI_STATUS_ERROR_E;
	}

	return QCLI_STATUS_SUCCESS_E;

}

QCLI_Command_Status_t persistent_obj_demo_get_attribute_command(uint32_t Parameter_Count, QCLI_Parameter_t * Parameter_List)
{
	qapi_Status_t status = QAPI_OK;
	char * p_obj_id = NULL;
    uint32_t attribute_id = 0;
    int parameter_index = 0;

    for ( parameter_index = 0; parameter_index < Parameter_Count; parameter_index++ )
    {
        if ( 0 == strcmp((char*)Parameter_List[parameter_index].String_Value, "-a") )
		{
			parameter_index++;
			if ( parameter_index == Parameter_Count ) {
				CRYPTO_PRINTF("ERROR: -a option is present, but no attribute id is specified\r\n");
				return QCLI_STATUS_USAGE_E;
			}
			attribute_id = strtoul(Parameter_List[parameter_index].String_Value, NULL, 16);
		}
        else if ( 0 == strcmp((char*)Parameter_List[parameter_index].String_Value, "-i") )
        {
            parameter_index++;
            if ( parameter_index == Parameter_Count ) {
            	CRYPTO_PRINTF("ERROR: -i option is present, but no id is specified\r\n");
            	return QCLI_STATUS_USAGE_E;
            }
			p_obj_id = Parameter_List[parameter_index].String_Value;
        }
    	else
    	{
    	   CRYPTO_PRINTF("ERROR: unsupported parameter\r\n");
    	   return QCLI_STATUS_USAGE_E;
    	}
    }

	if(p_obj_id == NULL) {
		CRYPTO_PRINTF("ERROR: Must specify an object id\n");
		return QCLI_STATUS_USAGE_E;
	}
	if(attribute_id == 0) {
		CRYPTO_PRINTF("ERROR: Must specify attribute id\n");
		return QCLI_STATUS_USAGE_E;
	}

	switch(attribute_id) {
	case QAPI_CRYPTO_ATTR_RSA_MODULUS_E:
	case QAPI_CRYPTO_ATTR_RSA_PUBLIC_EXPONENT_E:
	case QAPI_CRYPTO_ATTR_ECC_PUBLIC_VALUE_X_E:
	case QAPI_CRYPTO_ATTR_ECC_PUBLIC_VALUE_Y_E:
		{
			qapi_Crypto_Obj_Info_t p_obj_info;

			status = crypto_helper_persistent_obj_get_obj_info(
					(uint8_t*)p_obj_id,
					strlen(p_obj_id),
					&p_obj_info);
			if(status != QAPI_OK) {
				CRYPTO_PRINTF("Failed to get object info\n");
				return QCLI_STATUS_ERROR_E;
			}

			// some attributes may be less than key in which case
			// the value printed will be padded with zeros
		    int attribute_length = p_obj_info.key_Size / 8;
			uint8_t * p_attribute = NULL;
			p_attribute = (uint8_t*)malloc(attribute_length);
			if(p_attribute == NULL) {
				CRYPTO_PRINTF("Failed to allocate memory for attribute\n");
				return QCLI_STATUS_ERROR_E;
			}
			memset(p_attribute, 0, attribute_length);

			status = crypto_helper_persistent_obj_get_buffer_attibute(
					(uint8_t*)p_obj_id,
					strlen(p_obj_id),
					attribute_id,
					p_attribute,
					attribute_length);

			if(status == QAPI_OK) {
				CRYPTO_PRINTF("Buffer Attribute:\n");
				persistent_obj_demo_print_binary_buffer(p_attribute, attribute_length);
				free(p_attribute);
				return QCLI_STATUS_SUCCESS_E;
			} else {
				CRYPTO_PRINTF("Get buffer attribute failed (status=%d)\n", status);
				free(p_attribute);
				return QCLI_STATUS_ERROR_E;
			}
		}
	case QAPI_CRYPTO_ATTR_ECC_CURVE_E:
		{
			uint32_t attribute_value_a = 0;
			uint32_t attribute_value_b = 0;
			status = crypto_helper_persistent_obj_get_value_attibute(
					(uint8_t*)p_obj_id,
					strlen(p_obj_id),
					attribute_id,
					&attribute_value_a,
					&attribute_value_b);
			if(status == QAPI_OK) {
				CRYPTO_PRINTF("Value Attribute A: %d, Attribute B: %d\n", attribute_value_a, attribute_value_b);
				return QCLI_STATUS_SUCCESS_E;
			} else {
				CRYPTO_PRINTF("Get value attribute failed (status=%d)\n", status);
				return QCLI_STATUS_ERROR_E;
			}
		}
	default:
		CRYPTO_PRINTF("Attribute id is not supported\n", status);
		return QCLI_STATUS_ERROR_E;
	};
}

QCLI_Command_Status_t persistent_obj_demo_ecdh_command(uint32_t Parameter_Count, QCLI_Parameter_t * Parameter_List)
{
	qapi_Status_t status = QAPI_OK;
	char * obj_id = NULL;
    int parameter_index = 0;
    char * hostname = NULL;
    uint16_t port = 1443;
    char *peer_pem = NULL;
    uint32_t peem_pem_size = 0;
    char *peer_pem_filename = NULL;
    qapi_Crypto_Obj_Hdl_t derived_secret_obj_hdl = 0;
    uint8_t *p_secret = NULL;
	uint32_t secret_length = 0;

    for ( parameter_index = 0; parameter_index < Parameter_Count; parameter_index++ )
    {
        if ( 0 == strcmp((char*)Parameter_List[parameter_index].String_Value, "-h") )
        {
            parameter_index++;
            if ( parameter_index == Parameter_Count ) {
            	CRYPTO_PRINTF("ERROR: -h option is present, but no hostname is specified\r\n");
            	return QCLI_STATUS_USAGE_E;
            }
            hostname = (char*)Parameter_List[parameter_index].String_Value;
        }
        else if ( 0 == strcmp((char*)Parameter_List[parameter_index].String_Value, "-i") )
        {
            parameter_index++;
            if ( parameter_index == Parameter_Count ) {
            	CRYPTO_PRINTF("ERROR: -i option is present, but no id is specified\r\n");
            	return QCLI_STATUS_USAGE_E;
            }
			obj_id = Parameter_List[parameter_index].String_Value;
        }
    	else if ( 0 == strcmp((char*)Parameter_List[parameter_index].String_Value, "-f") )
		{
			parameter_index++;
			if ( parameter_index == Parameter_Count ) {
				CRYPTO_PRINTF("ERROR: -f option is present, but no peer public key pem file is specified\r\n");
				return QCLI_STATUS_USAGE_E;
			}
			peer_pem_filename = Parameter_List[parameter_index].String_Value;
		}
    	else
    	{
    	   CRYPTO_PRINTF("ERROR: unsupported parameter\r\n");
    	   return QCLI_STATUS_USAGE_E;
    	}
    }

	if(obj_id == NULL) {
		CRYPTO_PRINTF("ERROR: Must specify an object id\n");
		return QCLI_STATUS_USAGE_E;
	}

	if(peer_pem_filename == NULL) {
		CRYPTO_PRINTF("ERROR: Must specify peer PEM filename\n");
		return QCLI_STATUS_USAGE_E;
	}

	status = persistent_obj_demo_load_file(hostname,
			port,
			peer_pem_filename,
			(uint8_t**)&peer_pem,
			&peem_pem_size);
	if(status != QAPI_OK) {
		CRYPTO_PRINTF("ERROR: Unable to load PEM file.\n");
	    goto cleanup;
	}

	// Derive the shared secret and store it in a new transient object
	status = crypto_helper_persistent_obj_ecdh(
			(uint8_t*)obj_id,
			strlen(obj_id),
			peer_pem,
			NULL,
			&p_secret,
			&secret_length);
	if(status != QAPI_OK) {
		CRYPTO_PRINTF("ERROR: ECDH failed with status %d.\n", status);
	    goto cleanup;
	}

	CRYPTO_PRINTF("Derived Key:\n");
	persistent_obj_demo_print_binary_buffer(p_secret, secret_length);

cleanup:

	if(peer_pem) {
		free(peer_pem);
	}

	if(derived_secret_obj_hdl) {
		qapi_Crypto_Transient_Obj_Free(derived_secret_obj_hdl);
	}

	if(p_secret) {
		qapi_Crypto_Secure_Memzero(p_secret, secret_length);
		free(p_secret);
	}

	return status;

}

QCLI_Command_Status_t crypto_demo_persistent_objects(uint32_t Parameter_Count, QCLI_Parameter_t * Parameter_List)
{
	QCLI_Command_Status_t status = 0;

    if(Parameter_Count < 1) {
        status = QCLI_STATUS_USAGE_E;
    } else if (0 == strncmp(Parameter_List[0].String_Value, "run-tests", 9)) {
    	status = persistent_obj_demo_run_test_suite();
    } else if (0 == strncmp(Parameter_List[0].String_Value, "list", 6)) {
    	status = persistent_obj_demo_list_command();
    } else if (0 == strncmp(Parameter_List[0].String_Value, "create", 6)) {
    	status = persistent_obj_demo_create_command(Parameter_Count - 1, &Parameter_List[1]);
    } else if (0 == strncmp(Parameter_List[0].String_Value, "delete", 6)) {
    	status = persistent_obj_demo_delete_command(Parameter_Count - 1, &Parameter_List[1]);
    } else if (0 == strncmp(Parameter_List[0].String_Value, "rename", 6)) {
    	status = persistent_obj_demo_rename_command(Parameter_Count - 1, &Parameter_List[1]);
    } else if (0 == strncmp(Parameter_List[0].String_Value, "clone", 6)) {
    	status = persistent_obj_demo_clone_command(Parameter_Count - 1, &Parameter_List[1]);
    } else if (0 == strncmp(Parameter_List[0].String_Value, "encrypt", 7)) {
    	status = persistent_obj_demo_encrypt_command(Parameter_Count - 1, &Parameter_List[1]);
    } else if (0 == strncmp(Parameter_List[0].String_Value, "decrypt", 7)) {
    	status = persistent_obj_demo_decrypt_command(Parameter_Count - 1, &Parameter_List[1]);
    } else if (0 == strncmp(Parameter_List[0].String_Value, "sign", 4)) {
    	status = persistent_obj_demo_sign_command(Parameter_Count - 1, &Parameter_List[1]);
    } else if (0 == strncmp(Parameter_List[0].String_Value, "verify", 6)) {
    	status = persistent_obj_demo_verify_command(Parameter_Count - 1, &Parameter_List[1]);
    }  else if (0 == strncmp(Parameter_List[0].String_Value, "get-attribute", 13)) {
    	status = persistent_obj_demo_get_attribute_command(Parameter_Count - 1, &Parameter_List[1]);
    }  else if (0 == strncmp(Parameter_List[0].String_Value, "import", 6)) {
    	status = persistent_obj_demo_import_command(Parameter_Count - 1, &Parameter_List[1]);
    }  else if (0 == strncmp(Parameter_List[0].String_Value, "ecdh", 6)) {
    	status = persistent_obj_demo_ecdh_command(Parameter_Count - 1, &Parameter_List[1]);
    }  else {
    	status = QCLI_STATUS_USAGE_E;
    }

    if(status == QCLI_STATUS_USAGE_E) {
    	CRYPTO_PRINTF("pobj run-tests\n");
    	CRYPTO_PRINTF("     create [-h <server>] -t <file_type> -f <file_name> -i <obj_id> -o <obj_type> -l <flags>\n");
    	CRYPTO_PRINTF("     list\n");
    	CRYPTO_PRINTF("     sign [-h <server>] -f <file_name> -i <obj_id> -a <signature_algorithm> -d <hash_algorithm>\n");
    	CRYPTO_PRINTF("     verify [-h <server>] -f <file_name> -s <signature_file_name> -i <obj_id> -a <signature_algorithm> -d <hash_algorithm>\n");
    	CRYPTO_PRINTF("     encrypt [-h <server>] -f <file_name> -i <obj_id> -a <algorithm> [-v <iv_file_name>] [-n <nonce_file_name>] [-t <tag_file_name>] [-d <aad_file_name>] [-l <tag_length> (default = 128)] \n");
    	CRYPTO_PRINTF("     decrypt  [-h <server>] -f <file_name> -i <obj_id> -a <algorithm> [-v <iv_file_name>] [-n <nonce_file_name>] [-t <tag_file_name>] [-d <aad_file_name>]\n");
    	CRYPTO_PRINTF("     delete <obj_id>\n");
    	CRYPTO_PRINTF("     rename <obj_id> <new_obj_id>\n");
    	CRYPTO_PRINTF("     get-attribute <attribute-id> <obj_id>\n");
    	CRYPTO_PRINTF("     clone <obj_id> <obj_id_of_copy>\n");
    	CRYPTO_PRINTF("     import -c <certificate_name> -i <obj_id> [-o <obj_type>] [-u <obj_usage>] [-l <flags>]\n");
    	CRYPTO_PRINTF("     ecdh -i <key_pair_obj_id> -f <peer_public_key_pem_file> -d <derived_secret_obj_id> \n");
    }

    return status;
}

#endif
