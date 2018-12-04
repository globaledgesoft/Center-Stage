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
#include "crypto_helper.h"

#if defined (CONFIG_CRYPTO_UNIT_TEST_DEMO) || defined (CONFIG_CRYPTO_PERSISTENT_OBJECT_DEMO)

void ref_attr_init(qapi_Crypto_Attrib_t* attr, uint32_t id,
        const void* buffer, uint32_t length);

void val_attr_init(qapi_Crypto_Attrib_t* attr, uint32_t id,
        uint32_t a, uint32_t b);

qapi_Status_t crypto_helper_generate_aes_key(qapi_Crypto_Obj_Hdl_t *p_obj_hdl, uint32_t key_size_in_bits)
{
	qapi_Status_t status = QAPI_OK;

	status = qapi_Crypto_Transient_Obj_Alloc(QAPI_CRYPTO_OBJ_TYPE_AES_E,
			key_size_in_bits,
			p_obj_hdl);
	if (status != QAPI_OK) {
		return status;
	}
	status = qapi_Crypto_Transient_Obj_Key_Gen(*p_obj_hdl, key_size_in_bits, NULL, 0);
	if (status != QAPI_OK) {
		qapi_Crypto_Transient_Obj_Free(*p_obj_hdl);
	}
	return status;
}

qapi_Status_t crypto_helper_generate_ecdsa_key_pair(
		qapi_Crypto_Obj_Hdl_t *p_obj_hdl,
		uint32_t curve_id)
{
	qapi_Status_t status = QAPI_OK;
	qapi_Crypto_Attrib_t attr[1];
	qapi_Crypto_Obj_Hdl_t obj_hdl;
	uint32_t key_pair_size_in_bits = 0;

	switch(curve_id) {
	case QAPI_CRYPTO_ECC_CURVE_NIST_P192:
		key_pair_size_in_bits = QAPI_CRYPTO_ECC_P192_KEYPAIR_BITS;
		break;
	case QAPI_CRYPTO_ECC_CURVE_NIST_P256:
		key_pair_size_in_bits = QAPI_CRYPTO_ECC_P256_KEYPAIR_BITS;
		break;
	case QAPI_CRYPTO_ECC_CURVE_NIST_P384:
		key_pair_size_in_bits = QAPI_CRYPTO_ECC_P384_KEYPAIR_BITS;
		break;
	case QAPI_CRYPTO_ECC_CURVE_NIST_P521:
		key_pair_size_in_bits = QAPI_CRYPTO_ECC_P521_KEYPAIR_BITS;
		break;
	default:
		return QAPI_ERR_INVALID_PARAM;
	};

	status = qapi_Crypto_Transient_Obj_Alloc(
			QAPI_CRYPTO_OBJ_TYPE_ECDSA_KEYPAIR_E,
			key_pair_size_in_bits,
			&obj_hdl);
	if (status != QAPI_OK) {
		return status;
	}

	val_attr_init(attr, QAPI_CRYPTO_ATTR_ECC_CURVE_E, curve_id, 0);
	status = qapi_Crypto_Transient_Obj_Key_Gen(obj_hdl, key_pair_size_in_bits, attr, 1);
	if (status != QAPI_OK) {
		qapi_Crypto_Transient_Obj_Free(obj_hdl);
	} else {
		*p_obj_hdl = obj_hdl;
	}

	return status;
}

qapi_Status_t crypto_helper_asym_encrypt(
		qapi_Crypto_Obj_Hdl_t obj_hdl,
		const uint8_t *p_plain_text,
		uint32_t plain_text_len,
		uint8_t** pp_cipher_text,
		uint32_t *p_cipher_text_len,
		uint32_t algorithm_id)
{
	qapi_Status_t status = QAPI_OK;
	qapi_Crypto_Op_Hdl_t enc_hdl = 0;
	uint8_t* p_cipher_text = NULL;
	uint8_t* p_plain_text_copy = NULL;
	qapi_Crypto_Obj_Info_t obj_info;

    status = qapi_Crypto_Obj_Info_Get(obj_hdl, &obj_info);
    if(status != QAPI_OK) {
    	return status;
    }

	p_plain_text_copy = (uint8_t*)malloc(plain_text_len);
	if (p_plain_text_copy == NULL) {
	    status = QAPI_ERR_NO_MEMORY;
	    goto cleanup;
	}
	memcpy(p_plain_text_copy, p_plain_text, plain_text_len);

	p_cipher_text = (uint8_t*)malloc(QAPI_CRYPTO_RSA_ENCRYPT_CIPHER_TEXT_BUFFER_SIZE_BYTES(obj_info.key_Size / 8));
	if (p_cipher_text == NULL){
	    status = QAPI_ERR_NO_MEMORY;
	    goto cleanup;
	}
	*p_cipher_text_len = obj_info.key_Size / 8;

	/* Allocate encrypt operation */
	status = qapi_Crypto_Op_Alloc(
			algorithm_id,
			QAPI_CRYPTO_MODE_ENCRYPT_E,
			obj_info.key_Size,
            &enc_hdl);
	if (status != QAPI_OK) {
	    goto cleanup;
	}

	/* Copy public key from keypair object for encrypt operation */
	status = qapi_Crypto_Op_Key_Set(enc_hdl, obj_hdl);
	if (status != QAPI_OK) {
		goto cleanup;
	}

	/* Encrypt */
	status = qapi_Crypto_Op_Asym_Encrypt(enc_hdl,
			NULL,
			0,
			p_plain_text_copy,
			plain_text_len,
			p_cipher_text,
			p_cipher_text_len);
	if (status != QAPI_OK) {
		goto cleanup;
	}

	*pp_cipher_text = p_cipher_text;

cleanup:

	if( (status != QAPI_OK) && p_cipher_text) {
		free(p_cipher_text);
	}

	if(p_plain_text_copy) {
		free(p_plain_text_copy);
	}

	if(enc_hdl) {
		qapi_Crypto_Op_Free(enc_hdl);
	}

	return status;
}

qapi_Status_t crypto_helper_asym_decrypt (
		qapi_Crypto_Obj_Hdl_t obj_hdl,
		uint8_t* p_cipher_text,
		uint32_t cipher_text_len,
		uint32_t algorithm_id,
		uint8_t **pp_decrypted_plain_text,
		uint32_t *p_decrypted_plain_text_len)
{
	qapi_Status_t status = QAPI_OK;
	uint8_t* p_decrypted_plain_text = NULL;
	uint32_t decrypted_plain_text_len = 0;
	qapi_Crypto_Op_Hdl_t dec_hdl = 0;
	qapi_Crypto_Obj_Info_t obj_info;

    status = qapi_Crypto_Obj_Info_Get(obj_hdl, &obj_info);
    if(status != QAPI_OK) {
    	return status;
    }

	p_decrypted_plain_text = (uint8_t*)malloc(QAPI_CRYPTO_RSA_DECRYPT_PLAIN_TEXT_BUFFER_SIZE_BYTES(obj_info.key_Size / 8));
	if (p_decrypted_plain_text == NULL){
    	return QAPI_ERR_NO_MEMORY;
	}
	decrypted_plain_text_len = obj_info.key_Size / 8;

	/* Allocate decrypt operation */
	status = qapi_Crypto_Op_Alloc(
			algorithm_id,
			QAPI_CRYPTO_MODE_DECRYPT_E,
			obj_info.key_Size,
			&dec_hdl);
	if (status != QAPI_OK) {
		goto cleanup;
	}

	/* Copy private key from keypair object for decrypt operation */
	status = qapi_Crypto_Op_Key_Set(dec_hdl, obj_hdl);
	if (status != QAPI_OK) {
		goto cleanup;
	}

	/* Decrypt */
	status = qapi_Crypto_Op_Asym_Decrypt(dec_hdl,
			NULL,
			0,
			p_cipher_text,
			cipher_text_len,
			p_decrypted_plain_text,
			&decrypted_plain_text_len);
	if (status != QAPI_OK) {
		goto cleanup;
	}

	*pp_decrypted_plain_text = p_decrypted_plain_text;
	*p_decrypted_plain_text_len = decrypted_plain_text_len;

cleanup:

	if( (status != QAPI_OK) && p_decrypted_plain_text) {
		free(p_decrypted_plain_text);
	}

	if(dec_hdl) {
		qapi_Crypto_Op_Free(dec_hdl);
	}

	return status;
}

qapi_Status_t crypto_helper_allocate_cipher_or_ae_output_buffer(
		qapi_Crypto_Obj_Hdl_t obj_hdl,
		uint32_t mode,
		uint32_t algorithm_id,
		uint32_t input_buffer_size,
		uint8_t **pp_output,
		uint32_t *p_output_buffer_size)
{
	uint32_t output_buffer_size = 0;

   if(mode == QAPI_CRYPTO_MODE_ENCRYPT_E) {
	   switch(algorithm_id) {
		case QAPI_CRYPTO_ALG_AES_CBC_NOPAD_E:
		case QAPI_CRYPTO_ALG_AES_CTR_E:
		case QAPI_CRYPTO_ALG_AES_CCM_E:
		case QAPI_CRYPTO_ALG_AES_GCM_E:
			output_buffer_size = QAPI_CRYPTO_AES_ENCRYPT_CIPHER_TEXT_BUFFER_SIZE_BYTES(input_buffer_size);
			break;
		case QAPI_CRYPTO_ALG_CHACHA20_POLY1305_E:
			output_buffer_size = QAPI_CRYPTO_CHACHA20_POLY1305_ENCRYPT_CIPHER_TEXT_BUFFER_SIZE_BYTES(input_buffer_size);
			break;
		default:
			return QAPI_ERR_INVALID_PARAM;
		};
   } else {
	   switch(algorithm_id) {
		case QAPI_CRYPTO_ALG_AES_CBC_NOPAD_E:
		case QAPI_CRYPTO_ALG_AES_CTR_E:
		case QAPI_CRYPTO_ALG_AES_CCM_E:
		case QAPI_CRYPTO_ALG_AES_GCM_E:
			output_buffer_size = QAPI_CRYPTO_AES_DECRYPT_PLAIN_TEXT_BUFFER_SIZE_BYTES(input_buffer_size);
			break;
		case QAPI_CRYPTO_ALG_CHACHA20_POLY1305_E:
			output_buffer_size = QAPI_CRYPTO_CHACHA20_POLY1305_DECRYPT_PLAIN_TEXT_BUFFER_SIZE_BYTES(input_buffer_size);
			break;
		default:
			return QAPI_ERR_INVALID_PARAM;
		};
   }

   *pp_output = (uint8_t*)malloc(output_buffer_size);
   if(*pp_output == NULL) {
       return QAPI_ERR_NO_MEMORY;
   }
   memset(*pp_output, 0, output_buffer_size);
   *p_output_buffer_size = output_buffer_size;

   return QAPI_OK;
}

qapi_Status_t crypto_helper_set_up_cipher_or_ae_op(
		qapi_Crypto_Obj_Hdl_t obj_hdl,
		uint32_t mode,
		uint32_t algorithm_id,
		qapi_Crypto_Op_Hdl_t *p_cipher_enc_hdl)
{
	qapi_Status_t status = QAPI_OK;
	qapi_Crypto_Op_Hdl_t cipher_enc_hdl = 0;
	qapi_Crypto_Obj_Info_t obj_info;

    status = qapi_Crypto_Obj_Info_Get(obj_hdl, &obj_info);
    if(status != QAPI_OK) {
    	return status;
    }

	status = qapi_Crypto_Op_Alloc(
			algorithm_id,
			mode,
			obj_info.key_Size,
			&cipher_enc_hdl);
    if (status != QAPI_OK) {
    	return status;
    }

    status = qapi_Crypto_Op_Key_Set(cipher_enc_hdl, obj_hdl);
    if (status != QAPI_OK) {
    	return status;
    }

    *p_cipher_enc_hdl = cipher_enc_hdl;

    return QAPI_OK;
}

qapi_Status_t crypto_helper_ae_encrypt(
		qapi_Crypto_Obj_Hdl_t obj_hdl,
		uint32_t ae_algorithm_id,
		uint8_t *p_nonce,
		uint32_t nonce_length,
		uint8_t *p_tag,
		uint32_t *p_tag_length,
		uint8_t *p_aad_data,
		uint32_t aad_length,
		uint8_t* p_plain_text,
		uint32_t plain_text_length,
		uint8_t** pp_cipher_text,
		uint32_t *p_cipher_text_length)
{
	qapi_Status_t status = QAPI_OK;
	qapi_Crypto_Op_Hdl_t ae_enc_hdl = 0;
	uint8_t *p_output = NULL;
	uint32_t output_length = 0;

	if( (p_aad_data == NULL) || (aad_length == 0) ) {
		return QAPI_ERR_INVALID_PARAM;
	}

	status = crypto_helper_set_up_cipher_or_ae_op(
			obj_hdl,
			QAPI_CRYPTO_MODE_ENCRYPT_E,
			ae_algorithm_id,
			&ae_enc_hdl);
    if (status != QAPI_OK) {
    	goto cleanup;
    }

    status = crypto_helper_allocate_cipher_or_ae_output_buffer(
    		obj_hdl,
			QAPI_CRYPTO_MODE_ENCRYPT_E,
			ae_algorithm_id,
			plain_text_length,
    		&p_output,
    		&output_length);
    if (status != QAPI_OK) {
    	goto cleanup;
    }

    status = qapi_Crypto_Op_AE_Init(
    		ae_enc_hdl,
			(void*)p_nonce,
			nonce_length,
			*p_tag_length,
			aad_length,
			plain_text_length);
    if (status != QAPI_OK) {
    	goto cleanup;
    }

	status = qapi_Crypto_Op_AE_AAD_Update(ae_enc_hdl,
			(void*)p_aad_data,
			aad_length);
    if (status != QAPI_OK) {
    	goto cleanup;
    }

	status = qapi_Crypto_Op_AE_Encrypt_Final(ae_enc_hdl,
			p_plain_text,
			plain_text_length,
			p_output,
			&output_length,
			p_tag,
			p_tag_length);
    if (status != QAPI_OK) {
    	goto cleanup;
    }

	*pp_cipher_text = p_output;
	*p_cipher_text_length = output_length;

cleanup:

	if( (status != QAPI_OK) && p_output ) {
		free(p_output);
	}

	if(ae_enc_hdl) {
		qapi_Crypto_Op_Free(ae_enc_hdl);
	}

	return status;
}

qapi_Status_t crypto_helper_ae_decrypt(
		qapi_Crypto_Obj_Hdl_t obj_hdl,
		uint32_t ae_algorithm_id,
		uint8_t *p_nonce,
		uint32_t nonce_length,
		uint8_t *p_tag,
		uint32_t tag_length,
		uint8_t *p_aad_data,
		uint32_t aad_length,
		uint8_t* p_cipher_text,
		uint32_t cipher_text_length,
		uint8_t** pp_plain_text,
		uint32_t *p_plain_text_length)
{
	qapi_Status_t status = QAPI_OK;
	qapi_Crypto_Op_Hdl_t ae_dec_hdl = 0;
	uint8_t *p_output = NULL;
	uint32_t output_length = 0;

	if( (p_aad_data == NULL) || (aad_length == 0) ) {
		return QAPI_ERR_INVALID_PARAM;
	}

	status = crypto_helper_set_up_cipher_or_ae_op(
			obj_hdl,
			QAPI_CRYPTO_MODE_DECRYPT_E,
			ae_algorithm_id,
			&ae_dec_hdl);
    if (status != QAPI_OK) {
    	goto cleanup;
    }

    status = crypto_helper_allocate_cipher_or_ae_output_buffer(
    		obj_hdl,
			QAPI_CRYPTO_MODE_DECRYPT_E,
			ae_algorithm_id,
			cipher_text_length,
    		&p_output,
    		&output_length);
    if (status != QAPI_OK) {
    	goto cleanup;
    }

    status = qapi_Crypto_Op_AE_Init(
    		ae_dec_hdl,
			(void*)p_nonce,
			nonce_length,
			tag_length,
			aad_length,
			cipher_text_length);
    if (status != QAPI_OK) {
    	goto cleanup;
    }

	status = qapi_Crypto_Op_AE_AAD_Update(ae_dec_hdl,
			(void*)p_aad_data,
			aad_length);
    if (status != QAPI_OK) {
    	goto cleanup;
    }

	status = qapi_Crypto_Op_AE_Decrypt_Final(ae_dec_hdl,
			p_cipher_text,
			cipher_text_length,
			p_output,
			&output_length,
			p_tag,
			tag_length);
    if (status != QAPI_OK) {
    	goto cleanup;
    }

	*pp_plain_text = p_output;
	*p_plain_text_length = output_length;

cleanup:

	if( (status != QAPI_OK) && p_output ) {
		free(p_output);
	}

	if(ae_dec_hdl) {
		qapi_Crypto_Op_Free(ae_dec_hdl);
	}

	return status;
}

qapi_Status_t crypto_helper_cipher_encrypt_or_decrypt(
		qapi_Crypto_Obj_Hdl_t obj_hdl,
		uint32_t mode,
		uint32_t cipher_algorithm_id,
		uint8_t *p_iv,
		uint32_t iv_length,
		uint8_t *p_input,
		uint32_t input_size,
		uint8_t **pp_output,
		uint32_t *p_output_length)
{
	qapi_Status_t status = QAPI_OK;
	qapi_Crypto_Op_Hdl_t cipher_op_hdl = 0;
	uint8_t *p_output = NULL;
	uint32_t output_length = 0;

	status = crypto_helper_set_up_cipher_or_ae_op(
			obj_hdl,
			mode,
			cipher_algorithm_id,
			&cipher_op_hdl);
    if (status != QAPI_OK) {
    	goto cleanup;
    }

    status = crypto_helper_allocate_cipher_or_ae_output_buffer(
     		obj_hdl,
 			mode,
			cipher_algorithm_id,
 			input_size,
     		&p_output,
     		&output_length);
     if (status != QAPI_OK) {
     	goto cleanup;
     }

    status = qapi_Crypto_Op_Cipher_Init(
    		cipher_op_hdl,
			(void*)p_iv,
			iv_length);
    if (status != QAPI_OK) {
    	goto cleanup;
    }

    status = qapi_Crypto_Op_Cipher_Final(
    		cipher_op_hdl,
			p_input,
			input_size,
			p_output,
			&output_length);
    if (status != QAPI_OK) {
    	goto cleanup;
    }

    *pp_output = p_output;
    *p_output_length = output_length;

cleanup:

	if( (status != QAPI_OK) && p_output){
		free(p_output);
	}

	if(cipher_op_hdl) {
		qapi_Crypto_Op_Free(cipher_op_hdl);
	}

	return status;
}

qapi_Status_t crypto_helper_cipher_encrypt(
		qapi_Crypto_Obj_Hdl_t obj_hdl,
		uint32_t cipher_algorithm_id,
		uint8_t *p_iv,
		uint32_t iv_length,
		uint8_t *p_plain_text,
		uint32_t plain_text_size,
		uint8_t **pp_cipher_text,
		uint32_t *p_cipher_text_len)
{
	return crypto_helper_cipher_encrypt_or_decrypt(
			obj_hdl,
			QAPI_CRYPTO_MODE_ENCRYPT_E,
			cipher_algorithm_id,
			p_iv,
			iv_length,
			p_plain_text,
			plain_text_size,
			pp_cipher_text,
			p_cipher_text_len);
}

qapi_Status_t crypto_helper_cipher_decrypt(
		qapi_Crypto_Obj_Hdl_t obj_hdl,
		uint32_t cipher_algorithm_id,
		uint8_t *p_iv,
		uint32_t iv_length,
		uint8_t *p_cipher_text,
		uint32_t cipher_text_len,
		uint8_t **pp_plain_text,
		uint32_t *p_plain_text_size)
{
	return crypto_helper_cipher_encrypt_or_decrypt(
			obj_hdl,
			QAPI_CRYPTO_MODE_DECRYPT_E,
			cipher_algorithm_id,
			p_iv,
			iv_length,
			p_cipher_text,
			cipher_text_len,
			pp_plain_text,
			p_plain_text_size);
}

qapi_Status_t crypto_helper_compute_hmac(
		qapi_Crypto_Obj_Hdl_t obj_hdl,
		uint8_t *p_iv,
		uint32_t iv_length,
		uint8_t* p_message,
		uint32_t message_length,
		uint8_t **pp_hmac,
		uint32_t *p_hmac_length)
{
	qapi_Status_t status = QAPI_OK;
	qapi_Crypto_Op_Hdl_t hmac_op_hdl = 0;
	uint32_t max_key_size = 0;
	uint32_t hmac_algorithm_id = 0;
	qapi_Crypto_Obj_Info_t obj_info = {0};
	uint8_t *p_hmac = NULL;
	uint32_t hmac_length = 0;

    status = qapi_Crypto_Obj_Info_Get(obj_hdl,
    		&obj_info);
    if(status != QAPI_OK) {
    	goto cleanup;
    }

	switch(obj_info.object_Type)
	{
	case QAPI_CRYPTO_OBJ_TYPE_HMAC_MD5_E:
		max_key_size = QAPI_CRYPTO_HMAC_MD5_MAX_KEY_BITS;
		hmac_algorithm_id = QAPI_CRYPTO_ALG_HMAC_MD5_E;
		hmac_length = QAPI_CRYPTO_HMAC_MD5_MAC_BYTES;
		break;
	case QAPI_CRYPTO_OBJ_TYPE_HMAC_SHA1_E:
		max_key_size = QAPI_CRYPTO_HMAC_SHA1_MAX_KEY_BITS;
		hmac_algorithm_id = QAPI_CRYPTO_ALG_HMAC_SHA1_E;
		hmac_length = QAPI_CRYPTO_HMAC_SHA1_MAC_BYTES;
		break;
	case QAPI_CRYPTO_OBJ_TYPE_HMAC_SHA256_E:
		max_key_size = QAPI_CRYPTO_HMAC_SHA256_MAX_KEY_BITS;
		hmac_algorithm_id = QAPI_CRYPTO_ALG_HMAC_SHA256_E;
		hmac_length = QAPI_CRYPTO_HMAC_SHA256_MAC_BYTES;
		break;
	case QAPI_CRYPTO_OBJ_TYPE_HMAC_SHA384_E:
		max_key_size = QAPI_CRYPTO_HMAC_SHA384_MAX_KEY_BITS;
		hmac_algorithm_id = QAPI_CRYPTO_ALG_HMAC_SHA384_E;
		hmac_length = QAPI_CRYPTO_HMAC_SHA384_MAC_BYTES;
		break;
	case QAPI_CRYPTO_OBJ_TYPE_HMAC_SHA512_E:
		max_key_size = QAPI_CRYPTO_HMAC_SHA512_MAX_KEY_BITS;
		hmac_algorithm_id = QAPI_CRYPTO_ALG_HMAC_SHA512_E;
		hmac_length = QAPI_CRYPTO_HMAC_SHA512_MAC_BYTES;
		break;
	default:
		return QAPI_ERR_INVALID_PARAM;
	};

	p_hmac = (uint8_t*)malloc(hmac_length);
	if(p_hmac == NULL) {
		status = QAPI_ERR_NO_MEMORY;
		goto cleanup;
	}

	status = qapi_Crypto_Op_Alloc(hmac_algorithm_id,
		QAPI_CRYPTO_MODE_MAC_E,
		max_key_size,
		&hmac_op_hdl);
	if (status != QAPI_OK) {
		goto cleanup;
	}

	status = qapi_Crypto_Op_Key_Set(hmac_op_hdl, obj_hdl);
	if (status != QAPI_OK) {
		goto cleanup;
	}

	status = qapi_Crypto_Op_Mac_Init(hmac_op_hdl,
		p_iv,
		iv_length);
	if (status != QAPI_OK) {
		goto cleanup;
	}

	status = qapi_Crypto_Op_Mac_Final_Compute(hmac_op_hdl,
		(void*)p_message,
		message_length,
		p_hmac,
		&hmac_length);
	if (status != QAPI_OK) {
		goto cleanup;
	}

	*pp_hmac = p_hmac;
	*p_hmac_length = hmac_length;

	status = QAPI_OK;

cleanup:

	if(hmac_op_hdl) {
		qapi_Crypto_Op_Free(hmac_op_hdl);
	}

	if( (status != QAPI_OK) && p_hmac )
	{
		free(p_hmac);
	}

	return status;
}

qapi_Status_t crypto_helper_compute_and_compare_hmac(
		qapi_Crypto_Obj_Hdl_t obj_hdl,
		uint8_t *p_iv,
		uint32_t iv_length,
		uint8_t* p_message,
		uint32_t message_length,
		uint8_t *p_hmac,
		uint32_t hmac_length)
{
	qapi_Status_t status = QAPI_OK;
	qapi_Crypto_Op_Hdl_t hmac_op_hdl = 0;
	uint32_t max_key_size = 0;
	uint32_t hmac_algorithm_id = 0;
	qapi_Crypto_Obj_Info_t obj_info = {0};

    status = qapi_Crypto_Obj_Info_Get(obj_hdl,
    		&obj_info);
    if(status != QAPI_OK) {
    	goto cleanup;
    }

	switch(obj_info.object_Type)
	{
	case QAPI_CRYPTO_OBJ_TYPE_HMAC_MD5_E:
		max_key_size = QAPI_CRYPTO_HMAC_MD5_MAX_KEY_BITS;
		hmac_algorithm_id = QAPI_CRYPTO_ALG_HMAC_MD5_E;
		break;
	case QAPI_CRYPTO_OBJ_TYPE_HMAC_SHA1_E:
		max_key_size = QAPI_CRYPTO_HMAC_SHA1_MAX_KEY_BITS;
		hmac_algorithm_id = QAPI_CRYPTO_ALG_HMAC_SHA1_E;
		break;
	case QAPI_CRYPTO_OBJ_TYPE_HMAC_SHA256_E:
		max_key_size = QAPI_CRYPTO_HMAC_SHA256_MAX_KEY_BITS;
		hmac_algorithm_id = QAPI_CRYPTO_ALG_HMAC_SHA256_E;
		break;
	case QAPI_CRYPTO_OBJ_TYPE_HMAC_SHA384_E:
		max_key_size = QAPI_CRYPTO_HMAC_SHA384_MAX_KEY_BITS;
		hmac_algorithm_id = QAPI_CRYPTO_ALG_HMAC_SHA384_E;
		break;
	case QAPI_CRYPTO_OBJ_TYPE_HMAC_SHA512_E:
		max_key_size = QAPI_CRYPTO_HMAC_SHA512_MAX_KEY_BITS;
		hmac_algorithm_id = QAPI_CRYPTO_ALG_HMAC_SHA512_E;
		break;
	default:
		return QAPI_ERR_INVALID_PARAM;
	};

	status = qapi_Crypto_Op_Alloc(hmac_algorithm_id,
		QAPI_CRYPTO_MODE_MAC_E,
		max_key_size,
		&hmac_op_hdl);
	if (status != QAPI_OK) {
		goto cleanup;
	}

	status = qapi_Crypto_Op_Key_Set(hmac_op_hdl, obj_hdl);
	if (status != QAPI_OK) {
		goto cleanup;
	}

	status = qapi_Crypto_Op_Mac_Init(hmac_op_hdl,
		p_iv,
		iv_length);
	if (status != QAPI_OK) {
		goto cleanup;
	}

	status = qapi_Crypto_Op_Mac_Final_Compare(hmac_op_hdl,
		(void*)p_message,
		message_length,
		p_hmac,
		hmac_length);
	if (status != QAPI_OK) {
		goto cleanup;
	}

cleanup:

	if(hmac_op_hdl) {
		qapi_Crypto_Op_Free(hmac_op_hdl);
	}

	return status;
}

qapi_Status_t crypto_helper_hash_and_sign(
		qapi_Crypto_Obj_Hdl_t obj_hdl,
		const uint8_t *p_input_buffer,
		uint32_t input_buffer_size,
		uint32_t hash_algorithm_id,
		uint32_t signature_algorithm_id,
		uint8_t **pp_digest,
		uint32_t *p_digest_len,
		uint8_t **pp_signature,
		uint32_t *p_signature_len)
{
	qapi_Status_t status = QAPI_OK;
	qapi_Crypto_Op_Hdl_t sign_hdl = 0;
    qapi_Crypto_Op_Hdl_t digest_op_hdl = 0;
    qapi_Crypto_Obj_Info_t persistent_obj_info;
    uint32_t sign_op_key_size = 0;

    if (0 ||
    	(obj_hdl == 0) ||
		!p_input_buffer ||
		(input_buffer_size == 0) ||
		!pp_digest ||
		!p_digest_len ||
		!pp_signature ||
		!p_signature_len
		)
    {
    	return QAPI_ERR_INVALID_PARAM;
    }

    /* Hash */

    // Allocate digest for largest possible value of 128 bytes for
    // sha512 hash.
    *pp_digest = (uint8_t*)malloc(QAPI_CRYPTO_SHA512_DIGEST_BYTES);
    if(*pp_digest == NULL) {
    	return QAPI_ERR_NO_MEMORY;
    }
    *p_digest_len = QAPI_CRYPTO_SHA512_DIGEST_BYTES;

	status = qapi_Crypto_Op_Alloc(
			hash_algorithm_id,
			QAPI_CRYPTO_MODE_DIGEST_E,
			0,
			&digest_op_hdl);
    if (status != QAPI_OK) {
        goto cleanup;
    }

    status = qapi_Crypto_Op_Digest_Final(
    		digest_op_hdl,
			(void*)p_input_buffer,
			input_buffer_size,
			*pp_digest,
			p_digest_len);
    if (status != QAPI_OK) {
        goto cleanup;
    }

    /* Sign */

    status = qapi_Crypto_Obj_Info_Get(obj_hdl,
    		&persistent_obj_info);
    if(status != QAPI_OK) {
    	goto cleanup;
    }

    switch(persistent_obj_info.object_Type) {
    case QAPI_CRYPTO_OBJ_TYPE_RSA_KEYPAIR_E:
		sign_op_key_size = persistent_obj_info.max_Key_Size;
    	break;
    case QAPI_CRYPTO_OBJ_TYPE_ECDSA_KEYPAIR_E:
		sign_op_key_size = persistent_obj_info.max_Key_Size / 2;
    	break;
    default:
    	status = QAPI_ERR_INVALID_PARAM;
        goto cleanup;
    }

    uint32_t signature_size = 0;

    if(persistent_obj_info.object_Type == QAPI_CRYPTO_OBJ_TYPE_ECDSA_KEYPAIR_E) {
    	signature_size = QAPI_CRYPTO_ECDSA_SIGNATURE_BUFFER_SIZE_BYTES(persistent_obj_info.max_Key_Size / 8);
    } else if(persistent_obj_info.object_Type == QAPI_CRYPTO_OBJ_TYPE_RSA_KEYPAIR_E) {
    	signature_size = QAPI_CRYPTO_RSA_SIGNATURE_BUFFER_SIZE_BYTES(persistent_obj_info.max_Key_Size / 8);
    } else {
    	status = QAPI_ERR_INVALID_PARAM;
        goto cleanup;
    }

	*pp_signature = (uint8_t*)malloc(signature_size);
    if(*pp_signature == NULL) {
    	status = QAPI_ERR_NO_MEMORY;
        goto cleanup;
    }
    *p_signature_len = signature_size;

    status = qapi_Crypto_Op_Alloc(
    			signature_algorithm_id,
    			QAPI_CRYPTO_MODE_SIGN_E,
				sign_op_key_size,
    			&sign_hdl);
    if (status != QAPI_OK) {
        goto cleanup;
    }

    status = qapi_Crypto_Op_Key_Set(
    		sign_hdl,
			obj_hdl );
    if(status != QAPI_OK) {
    	goto cleanup;
    }

    status = qapi_Crypto_Op_Sign_Digest(sign_hdl,
			NULL,
			0,
			*pp_digest,
			*p_digest_len,
			*pp_signature,
			p_signature_len);

cleanup:

	if(sign_hdl) {
		qapi_Crypto_Op_Free(sign_hdl);
	}

	if(digest_op_hdl) {
		qapi_Crypto_Op_Free(digest_op_hdl);
	}

	if(status != QAPI_OK) {
		if(*pp_digest) {
			free(*pp_digest);
			*pp_digest = NULL;
		}
		if(*pp_signature) {
			free(*pp_signature);
			*pp_signature = NULL;
		}
	}

	return status;
}

qapi_Status_t crypto_helper_verify_signature(
		qapi_Crypto_Obj_Hdl_t obj_hdl,
		uint32_t signature_algorithm_id,
		uint8_t *p_digest,
		uint32_t digest_len,
		uint8_t *p_signature,
		uint32_t signature_len)
{
	qapi_Status_t status = QAPI_OK;
	qapi_Crypto_Op_Hdl_t verify_hdl = 0;
	qapi_Crypto_Obj_Info_t persistent_obj_info;

    status = qapi_Crypto_Obj_Info_Get(obj_hdl,
    		&persistent_obj_info);
    if(status != QAPI_OK) {
    	goto cleanup;
    }

	status = qapi_Crypto_Op_Alloc(signature_algorithm_id,
				QAPI_CRYPTO_MODE_VERIFY_E,
				persistent_obj_info.max_Key_Size,
				&verify_hdl);
	if (status != QAPI_OK) {
		goto cleanup;
	}

	status = qapi_Crypto_Op_Key_Set(verify_hdl, obj_hdl);
	if (status != QAPI_OK) {
		goto cleanup;
	}

	status = qapi_Crypto_Op_Verify_Digest(verify_hdl,
			NULL,
			0,
			p_digest,
			digest_len,
			p_signature,
			signature_len);

cleanup:

	if(verify_hdl) {
		qapi_Crypto_Op_Free(verify_hdl);
	}

	return status;
}

qapi_Status_t crypto_helper_hash_and_verify_signature(
		qapi_Crypto_Obj_Hdl_t obj_hdl,
		const uint8_t *p_message,
		uint32_t message_size,
		uint32_t hash_algorithm_id,
		uint32_t signature_algorithm_id,
		uint8_t *p_signature,
		uint32_t signature_length)
{
	qapi_Status_t status = QAPI_OK;
    qapi_Crypto_Op_Hdl_t digest_op_hdl = 0;
    uint8_t *p_digest = NULL;
    uint32_t digest_length = 0;

    if (0 ||
    	(obj_hdl == 0) ||
		!p_message ||
		(message_size == 0) ||
		!p_signature ||
		(signature_length == 0)
		)
    {
    	return QAPI_ERR_INVALID_PARAM;
    }

    // Allocate digest for largest possible value of 128 bytes for
    // sha512 hash.
    p_digest = (uint8_t*)malloc(QAPI_CRYPTO_HMAC_SHA512_MAC_BYTES);
    if(p_digest == NULL) {
    	return QAPI_ERR_NO_MEMORY;
    }

	status = qapi_Crypto_Op_Alloc(
			hash_algorithm_id,
			QAPI_CRYPTO_MODE_DIGEST_E,
			0,
			&digest_op_hdl);
    if (status != QAPI_OK) {
        goto cleanup;
    }

    digest_length = QAPI_CRYPTO_HMAC_SHA512_MAC_BYTES;
    status = qapi_Crypto_Op_Digest_Final(
    		digest_op_hdl,
			(void*)p_message,
			message_size,
			p_digest,
			&digest_length);
    if (status != QAPI_OK) {
        goto cleanup;
    }

    status = crypto_helper_verify_signature(
    		obj_hdl,
    		signature_algorithm_id,
    		p_digest,
    		digest_length,
    		p_signature,
    		signature_length);

cleanup:

	if(digest_op_hdl) {
		qapi_Crypto_Op_Free(digest_op_hdl);
	}

	if(p_digest) {
		free(p_digest);
	}

    return status;
}

qapi_Status_t crypto_helper_ecdh(
		qapi_Crypto_Obj_Hdl_t ecc_key_pair_obj_hdl,
		const char *p_peer_public_key_pem,
		const char* p_peer_public_key_pem_pass_phrase,
		uint8_t **pp_secret,
		uint32_t *p_secret_length)
{
	qapi_Crypto_Obj_Hdl_t peer_public_key_hdl = 0;
	qapi_Crypto_Op_Hdl_t ecdh_op_hdl = 0;
	uint32_t public_key_coordinate_size = 0;
	uint8_t *p_peer_ecc_public_x = NULL;
	uint8_t *p_peer_ecc_public_y = NULL;
	uint32_t dummy = 0;
	uint32_t curve_oid = 0;
	uint32_t peer_curve_oid = 0;
	qapi_Status_t status = QAPI_OK;
	qapi_Crypto_Obj_Hdl_t derived_secret_obj_hdl = 0;
	uint8_t *p_secret = NULL;
	uint32_t secret_length = 0;
	uint32_t ecdh_algorithm_id = 0;
	qapi_Crypto_Attrib_t attr[QAPI_CRYPTO_OBJ_ATTRIB_COUNT_ECDH];

	// Read the peer's public key pem into a transient object so that we can
	// read out the public key values
	status = qapi_Crypto_Transient_Obj_Import_From_Pem(
			QAPI_CRYPTO_OBJ_TYPE_ECDH_PUBLIC_KEY_E,
			p_peer_public_key_pem,
			p_peer_public_key_pem_pass_phrase,
			&peer_public_key_hdl);
	if(status != QAPI_OK) {
		return status;
	}

	// Make sure our ecc key pair's curve oid matches the other's oid
	status = qapi_Crypto_Obj_Val_Attrib_Get(peer_public_key_hdl,
			QAPI_CRYPTO_ATTR_ECC_CURVE_E,
			&peer_curve_oid,
			&dummy);
	if(status != QAPI_OK) {
		goto cleanup;
	}

	status = qapi_Crypto_Obj_Val_Attrib_Get(ecc_key_pair_obj_hdl,
			QAPI_CRYPTO_ATTR_ECC_CURVE_E,
			&curve_oid,
			&dummy);
	if(status != QAPI_OK) {
		goto cleanup;
	}
	if( peer_curve_oid != curve_oid ) {
		status = QAPI_ERR_INVALID_PARAM;
		goto cleanup;
	}

	// Figure out the key and secret lengths from the curve oid
	switch(curve_oid) {
	case QAPI_CRYPTO_ECC_CURVE_NIST_P192:
		public_key_coordinate_size = QAPI_CRYPTO_ECC_P192_PUB_VAL_X_BYTES;
		secret_length = QAPI_CRYPTO_ECC_P192_SHARED_SECRET_BYTES;
		ecdh_algorithm_id = QAPI_CRYPTO_ALG_ECDH_P192_E;
		break;
	case QAPI_CRYPTO_ECC_CURVE_NIST_P256:
		public_key_coordinate_size = QAPI_CRYPTO_ECC_P256_PUB_VAL_X_BYTES;
		secret_length = QAPI_CRYPTO_ECC_P256_SHARED_SECRET_BYTES;
		ecdh_algorithm_id = QAPI_CRYPTO_ALG_ECDH_P256_E;
		break;
	case QAPI_CRYPTO_ECC_CURVE_NIST_P384:
		public_key_coordinate_size = QAPI_CRYPTO_ECC_P384_PUB_VAL_X_BYTES;
		secret_length = QAPI_CRYPTO_ECC_P384_SHARED_SECRET_BYTES;
		ecdh_algorithm_id = QAPI_CRYPTO_ALG_ECDH_P384_E;
		break;
	case QAPI_CRYPTO_ECC_CURVE_NIST_P521:
		public_key_coordinate_size = QAPI_CRYPTO_ECC_P521_PUB_VAL_X_BYTES;
		secret_length = QAPI_CRYPTO_ECC_P521_SHARED_SECRET_BYTES;
		ecdh_algorithm_id = QAPI_CRYPTO_ALG_ECDH_P521_E;
		break;
	default:
		status = QAPI_ERR_INVALID_PARAM;
		goto cleanup;
	};

	// Create an ECDH operation
	status = qapi_Crypto_Op_Alloc(ecdh_algorithm_id,
			QAPI_CRYPTO_MODE_DERIVE_E,
			secret_length * 8, // key size specified in bits
			&ecdh_op_hdl);
	if (status != QAPI_OK) {
		goto cleanup;
	}

	// Set the key material (out private/public key pair) for the ECDH
	// operation.
	status = qapi_Crypto_Op_Key_Set(ecdh_op_hdl, ecc_key_pair_obj_hdl);
	if (status != QAPI_OK) {
		goto cleanup;
	}

	// Copy the peer's public x and y value into an attribute array
	p_peer_ecc_public_x = (uint8_t*)malloc(public_key_coordinate_size);
	if( p_peer_ecc_public_x == NULL ) {
		status = QAPI_ERR_NO_MEMORY;
		goto cleanup;
	}
	status = qapi_Crypto_Obj_Buf_Attrib_Get(peer_public_key_hdl,
			QAPI_CRYPTO_ATTR_ECC_PUBLIC_VALUE_X_E,
			p_peer_ecc_public_x,
			public_key_coordinate_size);
	if(status != QAPI_OK) {
		goto cleanup;
	}

	p_peer_ecc_public_y = (uint8_t*)malloc(public_key_coordinate_size);
	if( p_peer_ecc_public_y == NULL ) {
		status = QAPI_ERR_NO_MEMORY;
		goto cleanup;
	}
	status = qapi_Crypto_Obj_Buf_Attrib_Get(peer_public_key_hdl,
			QAPI_CRYPTO_ATTR_ECC_PUBLIC_VALUE_Y_E,
			p_peer_ecc_public_y,
			public_key_coordinate_size);
	if(status != QAPI_OK) {
		goto cleanup;
	}
	ref_attr_init(&attr[0], QAPI_CRYPTO_ATTR_ECC_PUBLIC_VALUE_X_E, (void*)p_peer_ecc_public_x, public_key_coordinate_size);
	ref_attr_init(&attr[1], QAPI_CRYPTO_ATTR_ECC_PUBLIC_VALUE_Y_E, (void*)p_peer_ecc_public_y, public_key_coordinate_size);

	// Create a transient object to hold the derived secret
	status = qapi_Crypto_Transient_Obj_Alloc(
			QAPI_CRYPTO_OBJ_TYPE_GENERIC_SECRET_E,
			secret_length * 8, // key size specified in bits
			&derived_secret_obj_hdl);
	if (status != QAPI_OK) {
		goto cleanup;
	}

	// Run the ECDH operation
	status = qapi_Crypto_Op_Key_Derive(ecdh_op_hdl,
				(qapi_Crypto_Attrib_t*)attr,
				QAPI_CRYPTO_OBJ_ATTRIB_COUNT_ECDH,
				derived_secret_obj_hdl);
	if(status != QAPI_OK) {
		goto cleanup;
	}

	// Copy the secret from the transient object to a buffer
	p_secret = (uint8_t*)malloc(secret_length);
	if(p_secret == NULL) {
		status = QAPI_ERR_NO_MEMORY;
		goto cleanup;
	}

	status = qapi_Crypto_Obj_Buf_Attrib_Get(derived_secret_obj_hdl,
				QAPI_CRYPTO_ATTR_SECRET_VALUE_E,
				p_secret,
				secret_length);

	if( status != QAPI_OK ) {
		free(p_secret);
		*pp_secret = NULL;
		*p_secret_length = 0;
	} else {
		*pp_secret = p_secret;
		*p_secret_length = secret_length;
	}

cleanup:

	if( p_peer_ecc_public_x ) {
		free(p_peer_ecc_public_x);
	}

	if( p_peer_ecc_public_y ) {
		free(p_peer_ecc_public_y);
	}

	if( peer_public_key_hdl ) {
		qapi_Crypto_Transient_Obj_Free(peer_public_key_hdl);
	}

	if(derived_secret_obj_hdl)
	{
		qapi_Crypto_Transient_Obj_Free(derived_secret_obj_hdl);
	}

	if(ecdh_op_hdl)
	{
		qapi_Crypto_Op_Free(ecdh_op_hdl);
	}

	return status;
}

qapi_Status_t crypto_helper_persistent_obj_create_asymmetric_key_from_pem(
		uint32_t obj_type,
		uint32_t create_flags,
		uint8_t * p_obj_id,
		uint32_t obj_id_len,
		const char *p_pem,
		const char* p_pem_pass_phrase)
{
	qapi_Status_t status = QAPI_OK;
	qapi_Crypto_Obj_Hdl_t persistent_obj_hdl = 0;
	qapi_Crypto_Obj_Hdl_t transient_obj_hdl = 0;

	status = qapi_Crypto_Transient_Obj_Import_From_Pem(obj_type,
			p_pem,
			p_pem_pass_phrase,
			&transient_obj_hdl);
	if(status == QAPI_OK) {
		status = qapi_Crypto_Persistent_Obj_Create(
				QAPI_CRYPTO_PERSISTENT_OBJ_DATA_TEE_STORAGE_PRIVATE,
				p_obj_id,
				obj_id_len,
				create_flags,
				transient_obj_hdl,
				NULL,
				0,
				&persistent_obj_hdl);
		qapi_Crypto_Persistent_Obj_Close(persistent_obj_hdl);
	}

	qapi_Crypto_Persistent_Obj_Close(transient_obj_hdl);

	return status;
}

qapi_Status_t crypto_helper_persistent_obj_create_symmetric_key(
		uint32_t symmetric_key_obj_type,
		uint32_t create_flags,
		uint8_t *obj_id,
		uint32_t obj_id_length,
		uint8_t *key,
		uint32_t key_length_in_bits)
{
	qapi_Status_t status = QAPI_OK;
	qapi_Crypto_Attrib_t attr[QAPI_CRYPTO_OBJ_ATTRIB_COUNT_GENERIC_SECRET];
	qapi_Crypto_Obj_Hdl_t transient_obj_hdl;
	qapi_Crypto_Obj_Hdl_t persistent_obj_hdl;

	if ( 0 ||
		(obj_id == NULL) ||
		(obj_id_length == 0) ||
		(obj_id_length > QAPI_CRYPTO_PERSISTENT_OBJECT_ID_MAX_LEN)
		)
	{
		return QAPI_ERR_INVALID_PARAM;
	}

	ref_attr_init(&attr[0], QAPI_CRYPTO_ATTR_SECRET_VALUE_E, (void*)key, key_length_in_bits / 8);

	status = qapi_Crypto_Transient_Obj_Alloc(symmetric_key_obj_type,
			key_length_in_bits,
			&transient_obj_hdl);
	if (status != QAPI_OK) {
		return status;
	}

    status = qapi_Crypto_Transient_Obj_Populate(transient_obj_hdl, attr, 1);
    if (status != QAPI_OK) {
    	goto cleanup;
    }

	status = qapi_Crypto_Persistent_Obj_Create(
			QAPI_CRYPTO_PERSISTENT_OBJ_DATA_TEE_STORAGE_PRIVATE,
			obj_id,
			obj_id_length,
			create_flags,
			transient_obj_hdl,
			NULL,
			0,
			&persistent_obj_hdl);

	qapi_Crypto_Persistent_Obj_Close(persistent_obj_hdl);

cleanup:

	qapi_Crypto_Transient_Obj_Free(transient_obj_hdl);

	return status;
}

qapi_Status_t crypto_helper_persistent_obj_compute_hmac(
		uint8_t * obj_id,
		uint32_t obj_id_len,
		uint32_t flags,
		uint8_t *p_iv,
		uint32_t iv_length,
		uint8_t* p_message,
		uint32_t message_length,
		uint8_t **pp_hmac,
		uint32_t *p_hmac_length)
{
	qapi_Status_t status = QAPI_OK;
	qapi_Crypto_Obj_Hdl_t persistent_obj_hdl;

	status = qapi_Crypto_Persistent_Obj_Open(
			QAPI_CRYPTO_PERSISTENT_OBJ_DATA_TEE_STORAGE_PRIVATE,
			obj_id,
			obj_id_len,
			flags,
			&persistent_obj_hdl);
	if(status != QAPI_OK) {
		return status;
	}

	status = crypto_helper_compute_hmac(
			persistent_obj_hdl,
			p_iv,
			iv_length,
			p_message,
			message_length,
			pp_hmac,
			p_hmac_length);

	qapi_Crypto_Persistent_Obj_Close(persistent_obj_hdl);

	return status;
}

qapi_Status_t crypto_helper_persistent_obj_compute_and_compare_hmac(
		uint8_t * obj_id,
		uint32_t obj_id_len,
		uint32_t flags,
		uint8_t *p_iv,
		uint32_t iv_length,
		uint8_t* p_message,
		uint32_t message_length,
		uint8_t *p_hmac,
		uint32_t hmac_length)
{
	qapi_Status_t status = QAPI_OK;
	qapi_Crypto_Obj_Hdl_t persistent_obj_hdl;

	status = qapi_Crypto_Persistent_Obj_Open(
			QAPI_CRYPTO_PERSISTENT_OBJ_DATA_TEE_STORAGE_PRIVATE,
			obj_id,
			obj_id_len,
			flags,
			&persistent_obj_hdl);
	if(status != QAPI_OK) {
		return status;
	}

	status = crypto_helper_compute_and_compare_hmac(
			persistent_obj_hdl,
			p_iv,
			iv_length,
			p_message,
			message_length,
			p_hmac,
			hmac_length);

	qapi_Crypto_Persistent_Obj_Close(persistent_obj_hdl);

	return status;
}

qapi_Status_t crypto_helper_persistent_obj_hash_and_sign(
		uint8_t * obj_id,
		uint32_t obj_id_len,
		uint32_t flags,
		const uint8_t *p_input_buffer,
		uint32_t input_buffer_size,
		uint32_t hash_algorithm_id,
		uint32_t signature_algorithm_id,
		uint8_t **pp_digest,
		uint32_t *p_digest_len,
		uint8_t **pp_signature,
		uint32_t *p_signature_len)
{

	qapi_Status_t status = QAPI_OK;
	qapi_Crypto_Obj_Hdl_t persistent_obj_hdl;

	status = qapi_Crypto_Persistent_Obj_Open(
			QAPI_CRYPTO_PERSISTENT_OBJ_DATA_TEE_STORAGE_PRIVATE,
			obj_id,
			obj_id_len,
			flags,
			&persistent_obj_hdl);
	if(status != QAPI_OK) {
		return status;
	}

	status = crypto_helper_hash_and_sign(
			persistent_obj_hdl,
			p_input_buffer,
			input_buffer_size,
			hash_algorithm_id,
			signature_algorithm_id,
			pp_digest,
			p_digest_len,
			pp_signature,
			p_signature_len);

	qapi_Crypto_Persistent_Obj_Close(persistent_obj_hdl);

	return status;
}

qapi_Status_t crypto_helper_persistent_obj_verify_signature(
		uint8_t * obj_id,
		uint32_t obj_id_len,
		uint32_t flags,
		uint32_t signature_algorithm_id,
		uint8_t *p_digest,
		uint32_t digest_len,
		uint8_t *p_signature,
		uint32_t signature_len)
{
	qapi_Status_t status = QAPI_OK;
	qapi_Crypto_Obj_Hdl_t persistent_obj_hdl;

	status = qapi_Crypto_Persistent_Obj_Open(
			QAPI_CRYPTO_PERSISTENT_OBJ_DATA_TEE_STORAGE_PRIVATE,
			obj_id,
			obj_id_len,
			flags,
			&persistent_obj_hdl);
	if(status != QAPI_OK) {
		return status;
	}

	status = crypto_helper_verify_signature(
			persistent_obj_hdl,
			signature_algorithm_id,
			p_digest,
			digest_len,
			p_signature,
			signature_len);

	qapi_Crypto_Persistent_Obj_Close(persistent_obj_hdl);

	return status;
}

qapi_Status_t crypto_helper_persistent_obj_hash_and_verify_signature(
		uint8_t * obj_id,
		uint32_t obj_id_len,
		uint32_t flags,
		const uint8_t *p_message,
		uint32_t message_size,
		uint32_t hash_algorithm_id,
		uint32_t signature_algorithm_id,
		uint8_t *p_signature,
		uint32_t signature_length)
{
	qapi_Status_t status = QAPI_OK;
	qapi_Crypto_Obj_Hdl_t persistent_obj_hdl;

	status = qapi_Crypto_Persistent_Obj_Open(
			QAPI_CRYPTO_PERSISTENT_OBJ_DATA_TEE_STORAGE_PRIVATE,
			obj_id,
			obj_id_len,
			flags,
			&persistent_obj_hdl);
	if(status != QAPI_OK) {
		return status;
	}

	status = crypto_helper_hash_and_verify_signature(
			persistent_obj_hdl,
			p_message,
			message_size,
			hash_algorithm_id,
			signature_algorithm_id,
			p_signature,
			signature_length);

	qapi_Crypto_Persistent_Obj_Close(persistent_obj_hdl);

	return status;
}


qapi_Status_t crypto_helper_persistent_obj_ae_encrypt (
		uint8_t * obj_id,
		uint32_t obj_id_len,
		uint32_t flags,
		uint32_t ae_algorithm_id,
		uint8_t *p_nonce,
		uint32_t nonce_length,
		uint8_t *p_tag,
		uint32_t *p_tag_len,
		uint8_t *p_aad_data,
		uint32_t aad_length,
		uint8_t* p_plain_text,
		uint32_t plain_text_length,
		uint8_t** pp_cipher_text,
		uint32_t *p_cipher_text_length)
{
	qapi_Status_t status = QAPI_OK;
	qapi_Crypto_Obj_Hdl_t persistent_obj_hdl;

	status = qapi_Crypto_Persistent_Obj_Open(
			QAPI_CRYPTO_PERSISTENT_OBJ_DATA_TEE_STORAGE_PRIVATE,
			obj_id,
			obj_id_len,
			flags,
			&persistent_obj_hdl);
	if(status != QAPI_OK) {
		return status;
	}

	status = crypto_helper_ae_encrypt (
			persistent_obj_hdl,
			ae_algorithm_id,
			p_nonce,
			nonce_length,
			p_tag,
			p_tag_len,
			p_aad_data,
			aad_length,
			p_plain_text,
			plain_text_length,
			pp_cipher_text,
			p_cipher_text_length);

	qapi_Crypto_Persistent_Obj_Close(persistent_obj_hdl);

	return status;
}

qapi_Status_t crypto_helper_persistent_obj_ae_decrypt (
		uint8_t * p_obj_id,
		uint32_t obj_id_len,
		uint32_t flags,
		uint32_t ae_algorithm_id,
		uint8_t *p_nonce,
		uint32_t nonce_length,
		uint8_t *p_tag,
		uint32_t tag_len,
		uint8_t *p_aad_data,
		uint32_t aad_length,
		uint8_t* p_cipher_text,
		uint32_t cipher_text_length,
		uint8_t** pp_plain_text,
		uint32_t *p_plain_text_length)
{
	qapi_Status_t status = QAPI_OK;
	qapi_Crypto_Obj_Hdl_t persistent_obj_hdl;

	status = qapi_Crypto_Persistent_Obj_Open(
			QAPI_CRYPTO_PERSISTENT_OBJ_DATA_TEE_STORAGE_PRIVATE,
			p_obj_id,
			obj_id_len,
			flags,
			&persistent_obj_hdl);
	if(status != QAPI_OK) {
		return status;
	}

	status = crypto_helper_ae_decrypt (
			persistent_obj_hdl,
			ae_algorithm_id,
			p_nonce,
			nonce_length,
			p_tag,
			tag_len,
			p_aad_data,
			aad_length,
			p_cipher_text,
			cipher_text_length,
			pp_plain_text,
			p_plain_text_length);

	qapi_Crypto_Persistent_Obj_Close(persistent_obj_hdl);

	return status;
}

qapi_Status_t crypto_helper_persistent_obj_cipher_encrypt(
		uint8_t * p_obj_id,
		uint32_t obj_id_len,
		uint32_t flags,
		uint32_t cipher_algorithm_id,
		uint8_t *p_iv,
		uint32_t iv_length,
		uint8_t *p_plain_text,
		uint32_t plain_text_size,
		uint8_t **pp_cipher_text,
		uint32_t *p_cipher_text_len)
{
	qapi_Status_t status = QAPI_OK;
	qapi_Crypto_Obj_Hdl_t persistent_obj_hdl;

	status = qapi_Crypto_Persistent_Obj_Open(
			QAPI_CRYPTO_PERSISTENT_OBJ_DATA_TEE_STORAGE_PRIVATE,
			p_obj_id,
			obj_id_len,
			flags,
			&persistent_obj_hdl);
	if(status != QAPI_OK) {
		return status;
	}

	status = crypto_helper_cipher_encrypt(
			persistent_obj_hdl,
			cipher_algorithm_id,
			p_iv,
			iv_length,
			p_plain_text,
			plain_text_size,
			pp_cipher_text,
			p_cipher_text_len);

	qapi_Crypto_Persistent_Obj_Close(persistent_obj_hdl);

	return status;
}

qapi_Status_t crypto_helper_persistent_obj_cipher_decrypt (
		uint8_t * p_obj_id,
		uint32_t obj_id_len,
		uint32_t flags,
		uint32_t cipher_algorithm_id,
		uint8_t *p_iv,
		uint32_t iv_length,
		uint8_t *p_cipher_text,
		uint32_t cipher_text_len,
		uint8_t **pp_plain_text,
		uint32_t *p_plain_text_size)
{
	qapi_Status_t status = QAPI_OK;
	qapi_Crypto_Obj_Hdl_t persistent_obj_hdl;

	status = qapi_Crypto_Persistent_Obj_Open(
			QAPI_CRYPTO_PERSISTENT_OBJ_DATA_TEE_STORAGE_PRIVATE,
			p_obj_id,
			obj_id_len,
			flags,
			&persistent_obj_hdl);
	if(status != QAPI_OK) {
		return status;
	}

	status = crypto_helper_cipher_decrypt (
			persistent_obj_hdl,
			cipher_algorithm_id,
			p_iv,
			iv_length,
			p_cipher_text,
			cipher_text_len,
			pp_plain_text,
			p_plain_text_size);

	qapi_Crypto_Persistent_Obj_Close(persistent_obj_hdl);

	return status;
}

qapi_Status_t crypto_helper_persistent_obj_asym_encrypt(
		uint8_t * p_obj_id,
		uint32_t obj_id_len,
		uint32_t flags,
		const uint8_t *p_plain_text,
		uint32_t plain_text_len,
		uint8_t** pp_cipher_text,
		uint32_t *p_cipher_text_len,
		uint32_t algorithm_id)
{
	qapi_Status_t status = QAPI_OK;
	qapi_Crypto_Obj_Hdl_t persistent_obj_hdl;

	status = qapi_Crypto_Persistent_Obj_Open(
			QAPI_CRYPTO_PERSISTENT_OBJ_DATA_TEE_STORAGE_PRIVATE,
			p_obj_id,
			obj_id_len,
			flags,
			&persistent_obj_hdl);
	if(status != QAPI_OK) {
		return status;
	}

	status = crypto_helper_asym_encrypt(
			persistent_obj_hdl,
			p_plain_text,
			plain_text_len,
			pp_cipher_text,
			p_cipher_text_len,
			algorithm_id);

	qapi_Crypto_Persistent_Obj_Close(persistent_obj_hdl);

	return status;
}

qapi_Status_t crypto_helper_persistent_obj_asym_decrypt(
		uint8_t * p_obj_id,
		uint32_t obj_id_len,
		uint32_t flags,
		uint8_t* p_cipher_text,
		uint32_t cipher_text_len,
		uint32_t algorithm_id,
		uint8_t **pp_decrypted_plain_text,
		uint32_t *p_decrypted_plain_text_len)
{
	qapi_Status_t status = QAPI_OK;
	qapi_Crypto_Obj_Hdl_t persistent_obj_hdl;

	status = qapi_Crypto_Persistent_Obj_Open(
			QAPI_CRYPTO_PERSISTENT_OBJ_DATA_TEE_STORAGE_PRIVATE,
			p_obj_id,
			obj_id_len,
			flags,
			&persistent_obj_hdl);
	if(status != QAPI_OK) {
		return status;
	}

	status = crypto_helper_asym_decrypt (
			persistent_obj_hdl,
			p_cipher_text,
			cipher_text_len,
			algorithm_id,
			pp_decrypted_plain_text,
			p_decrypted_plain_text_len);

	qapi_Crypto_Persistent_Obj_Close(persistent_obj_hdl);

	return status;
}

qapi_Status_t crypto_helper_persistent_obj_ecdh(
		uint8_t *p_ecc_key_pair_obj_id,
		uint32_t ecc_key_pair_obj_id_length,
		const char *p_peer_public_key_pem,
		const char* p_peer_public_key_pem_pass_phrase,
		uint8_t **pp_secret,
		uint32_t *p_secret_length)
{
	qapi_Status_t status = QAPI_OK;
	qapi_Crypto_Obj_Hdl_t ecc_key_pair_persistent_obj_hdl;

	// Open the persistent object that contains the ECDH keypair
	status = qapi_Crypto_Persistent_Obj_Open(
			QAPI_CRYPTO_PERSISTENT_OBJ_DATA_TEE_STORAGE_PRIVATE,
			p_ecc_key_pair_obj_id,
			ecc_key_pair_obj_id_length,
			0,
			&ecc_key_pair_persistent_obj_hdl);
	if(status != QAPI_OK) {
		return status;
	}

	// Run ECDH
	status = crypto_helper_ecdh(
		ecc_key_pair_persistent_obj_hdl,
		p_peer_public_key_pem,
		p_peer_public_key_pem_pass_phrase,
		pp_secret,
		p_secret_length);

	qapi_Crypto_Persistent_Obj_Close(ecc_key_pair_persistent_obj_hdl);

	return status;
}

qapi_Status_t crypto_helper_persistent_obj_rename (
		uint8_t * p_obj_id,
		uint32_t obj_id_len,
		uint8_t * p_new_obj_id,
		uint32_t new_obj_id_len,
		uint32_t flags)
{
	qapi_Status_t status = QAPI_OK;
	qapi_Crypto_Obj_Hdl_t persistent_obj_hdl;

	status = qapi_Crypto_Persistent_Obj_Open(
			QAPI_CRYPTO_PERSISTENT_OBJ_DATA_TEE_STORAGE_PRIVATE,
			p_obj_id,
			obj_id_len,
			flags,
			&persistent_obj_hdl);
	if(status != QAPI_OK) {
		return status;
	}

	status = qapi_Crypto_Persistent_Obj_Rename(
			persistent_obj_hdl,
			p_new_obj_id,
			new_obj_id_len);

	qapi_Crypto_Persistent_Obj_Close(persistent_obj_hdl);

	return status;
}

qapi_Status_t crypto_helper_persistent_obj_delete (
		uint8_t * p_obj_id,
		uint32_t obj_id_len,
		uint32_t flags)
{
	qapi_Status_t status = QAPI_OK;
	qapi_Crypto_Obj_Hdl_t persistent_obj_hdl;

	status = qapi_Crypto_Persistent_Obj_Open(
			QAPI_CRYPTO_PERSISTENT_OBJ_DATA_TEE_STORAGE_PRIVATE,
			p_obj_id,
			obj_id_len,
			flags,
			&persistent_obj_hdl);
	if(status != QAPI_OK) {
		return status;
	}

	status = qapi_Crypto_Persistent_Obj_Close_and_Delete(persistent_obj_hdl);

	// if we fail to delete e.g. because we don't have the right access flags,
	// try to till close the handle that we opened
	if(status == QAPI_ERR_CRYPTO_ACCESS_CONFLICT) {
		qapi_Crypto_Persistent_Obj_Close(persistent_obj_hdl);
	}

	return status;
}

qapi_Status_t crypto_helper_persistent_obj_get_obj_info(
		uint8_t * p_obj_id,
		uint32_t obj_id_len,
		qapi_Crypto_Obj_Info_t *p_obj_info)
{
	qapi_Status_t status = QAPI_OK;
	qapi_Crypto_Obj_Hdl_t persistent_obj_hdl;

	status = qapi_Crypto_Persistent_Obj_Open(
			QAPI_CRYPTO_PERSISTENT_OBJ_DATA_TEE_STORAGE_PRIVATE,
			p_obj_id,
			obj_id_len,
			0,
			&persistent_obj_hdl);
	if(status != QAPI_OK) {
		return status;
	}

	status = qapi_Crypto_Obj_Info_Get(persistent_obj_hdl,
			p_obj_info);

	qapi_Crypto_Persistent_Obj_Close(persistent_obj_hdl);

	return status;
}

qapi_Status_t crypto_helper_persistent_obj_get_buffer_attibute(
		uint8_t * p_obj_id,
		uint32_t obj_id_len,
		uint32_t attribute_id,
		void *buffer,
		uint32_t buffer_size)
{
	qapi_Status_t status = QAPI_OK;
	qapi_Crypto_Obj_Hdl_t persistent_obj_hdl;

	status = qapi_Crypto_Persistent_Obj_Open(
			QAPI_CRYPTO_PERSISTENT_OBJ_DATA_TEE_STORAGE_PRIVATE,
			p_obj_id,
			obj_id_len,
			0,
			&persistent_obj_hdl);
	if(status != QAPI_OK) {
		return status;
	}

	status = qapi_Crypto_Obj_Buf_Attrib_Get(persistent_obj_hdl,
			attribute_id,
			buffer,
			buffer_size);

	qapi_Crypto_Persistent_Obj_Close(persistent_obj_hdl);

	return status;
}

qapi_Status_t crypto_helper_persistent_obj_get_value_attibute(
		uint8_t * p_obj_id,
		uint32_t obj_id_len,
		uint32_t attribute_id,
		uint32_t *attribute_value_a,
		uint32_t *attribute_value_b)
{
	qapi_Status_t status = QAPI_OK;
	qapi_Crypto_Obj_Hdl_t persistent_obj_hdl;

	status = qapi_Crypto_Persistent_Obj_Open(
			QAPI_CRYPTO_PERSISTENT_OBJ_DATA_TEE_STORAGE_PRIVATE,
			p_obj_id,
			obj_id_len,
			0,
			&persistent_obj_hdl);
	if(status != QAPI_OK) {
		return status;
	}

	status = qapi_Crypto_Obj_Val_Attrib_Get(persistent_obj_hdl,
			attribute_id,
			attribute_value_a,
			attribute_value_b);

	qapi_Crypto_Persistent_Obj_Close(persistent_obj_hdl);

	return status;
}

qapi_Status_t crypto_helper_persistent_obj_clone(
		uint8_t *p_template_obj_id,
		uint32_t template_obj_id_len,
		uint8_t *p_obj_id,
		uint32_t obj_id_len)
{
	qapi_Status_t status = QAPI_OK;
	qapi_Crypto_Obj_Hdl_t template_obj_hdl = 0;
	qapi_Crypto_Obj_Hdl_t cloned_obj_hdl = 0;

	status = qapi_Crypto_Persistent_Obj_Open(
		QAPI_CRYPTO_PERSISTENT_OBJ_DATA_TEE_STORAGE_PRIVATE,
		p_template_obj_id,
		template_obj_id_len,
		0,
		&template_obj_hdl);
	if(status != QAPI_OK) {
		return status;
	}

	status = qapi_Crypto_Persistent_Obj_Create(
			QAPI_CRYPTO_PERSISTENT_OBJ_DATA_TEE_STORAGE_PRIVATE,
			p_obj_id,
			obj_id_len,
			0,
			template_obj_hdl,
			NULL,
			0,
			&cloned_obj_hdl);

	qapi_Crypto_Persistent_Obj_Close(template_obj_hdl);
	qapi_Crypto_Persistent_Obj_Close(cloned_obj_hdl);

	return status;
}

#endif
