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

 
#ifndef __CRYPTO_HELPER_H__
#define __CRYPTO_HELPER_H__

#include "qapi_status.h"
#include "qapi_crypto.h"

qapi_Status_t crypto_helper_generate_aes_key(
		qapi_Crypto_Obj_Hdl_t *p_obj_hdl,
		uint32_t key_size_in_bits);

qapi_Status_t crypto_helper_generate_ecdsa_key_pair(
		qapi_Crypto_Obj_Hdl_t *p_obj_hdl,
		uint32_t curve_id);

qapi_Status_t crypto_helper_asym_encrypt(
		qapi_Crypto_Obj_Hdl_t obj_hdl,
		const uint8_t *p_plain_text,
		uint32_t plain_text_len,
		uint8_t** pp_cipher_text,
		uint32_t *p_cipher_text_len,
		uint32_t algorithm_id);

qapi_Status_t crypto_helper_asym_decrypt (
		qapi_Crypto_Obj_Hdl_t obj_hdl,
		uint8_t* p_cipher_text,
		uint32_t cipher_text_len,
		uint32_t algorithm_id,
		uint8_t **pp_decrypted_plain_text,
		uint32_t *p_decrypted_plain_text_len);

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
		uint32_t *p_cipher_text_length);

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
		uint32_t *p_plain_text_length);

qapi_Status_t crypto_helper_cipher_encrypt(
		qapi_Crypto_Obj_Hdl_t obj_hdl,
		uint32_t cipher_algorithm_id,
		uint8_t *p_iv,
		uint32_t iv_length,
		uint8_t *p_plain_text,
		uint32_t plain_text_size,
		uint8_t **pp_cipher_text,
		uint32_t *p_cipher_text_len);

qapi_Status_t crypto_helper_cipher_decrypt(
		qapi_Crypto_Obj_Hdl_t obj_hdl,
		uint32_t cipher_algorithm_id,
		uint8_t *p_iv,
		uint32_t iv_length,
		uint8_t *p_cipher_text,
		uint32_t cipher_text_len,
		uint8_t **pp_plain_text,
		uint32_t *p_plain_text_size);

qapi_Status_t crypto_helper_compute_hmac(
		qapi_Crypto_Obj_Hdl_t obj_hdl,
		uint8_t *p_iv,
		uint32_t iv_length,
		uint8_t* p_message,
		uint32_t message_length,
		uint8_t **pp_hmac,
		uint32_t *p_hmac_length);

qapi_Status_t crypto_helper_compute_and_compare_hmac(
		qapi_Crypto_Obj_Hdl_t obj_hdl,
		uint8_t *p_iv,
		uint32_t iv_length,
		uint8_t* p_message,
		uint32_t message_length,
		uint8_t *p_hmac,
		uint32_t hmac_length);

qapi_Status_t crypto_helper_hash_and_sign(
		qapi_Crypto_Obj_Hdl_t obj_hdl,
		const uint8_t *p_input_buffer,
		uint32_t input_buffer_size,
		uint32_t hash_algorithm_id,
		uint32_t signature_algorithm_id,
		uint8_t **pp_digest,
		uint32_t *p_digest_len,
		uint8_t **pp_signature,
		uint32_t *p_signature_len);

qapi_Status_t crypto_helper_verify_signature(
		qapi_Crypto_Obj_Hdl_t obj_hdl,
		uint32_t signature_algorithm_id,
		uint8_t *p_digest,
		uint32_t digest_len,
		uint8_t *p_signature,
		uint32_t signature_len);

qapi_Status_t crypto_helper_hash_and_verify_signature(
		qapi_Crypto_Obj_Hdl_t obj_hdl,
		const uint8_t *p_message,
		uint32_t message_size,
		uint32_t hash_algorithm_id,
		uint32_t signature_algorithm_id,
		uint8_t *p_signature,
		uint32_t signature_length);

qapi_Status_t crypto_helper_ecdh(
		qapi_Crypto_Obj_Hdl_t ecc_key_pair_obj_hdl,
		const char *p_peer_public_key_pem,
		const char* p_peer_public_key_pem_pass_phrase,
		uint8_t **pp_secret,
		uint32_t *p_secret_length);

qapi_Status_t crypto_helper_persistent_obj_create_asymmetric_key_from_pem(
		uint32_t obj_type,
		uint32_t create_flags,
		uint8_t * p_obj_id,
		uint32_t obj_id_len,
		const char *p_pem,
		const char* p_pem_pass_phrase);

qapi_Status_t crypto_helper_persistent_obj_create_symmetric_key(
		uint32_t symmetric_key_obj_type,
		uint32_t create_flags,
		uint8_t *obj_id,
		uint32_t obj_id_length,
		uint8_t *key,
		uint32_t key_length_in_bits);

qapi_Status_t crypto_helper_persistent_obj_compute_hmac(
		uint8_t * obj_id,
		uint32_t obj_id_len,
		uint32_t flags,
		uint8_t *p_iv,
		uint32_t iv_length,
		uint8_t* p_message,
		uint32_t message_length,
		uint8_t **pp_hmac,
		uint32_t *p_hmac_length);

qapi_Status_t crypto_helper_persistent_obj_compute_and_compare_hmac(
		uint8_t * obj_id,
		uint32_t obj_id_len,
		uint32_t flags,
		uint8_t *p_iv,
		uint32_t iv_length,
		uint8_t* p_message,
		uint32_t message_length,
		uint8_t *p_hmac,
		uint32_t hmac_length);

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
		uint32_t *p_signature_len);

qapi_Status_t crypto_helper_persistent_obj_verify_signature(
		uint8_t * obj_id,
		uint32_t obj_id_len,
		uint32_t flags,
		uint32_t signature_algorithm_id,
		uint8_t *p_digest,
		uint32_t digest_len,
		uint8_t *p_signature,
		uint32_t signature_len);

qapi_Status_t crypto_helper_persistent_obj_hash_and_verify_signature(
		uint8_t * obj_id,
		uint32_t obj_id_len,
		uint32_t flags,
		const uint8_t *p_message,
		uint32_t message_size,
		uint32_t hash_algorithm_id,
		uint32_t signature_algorithm_id,
		uint8_t *p_signature,
		uint32_t signature_length);

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
		uint32_t *p_cipher_text_length);

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
		uint32_t *p_plain_text_length);

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
		uint32_t *p_cipher_text_len);

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
		uint32_t *p_plain_text_size);

qapi_Status_t crypto_helper_persistent_obj_asym_encrypt(
		uint8_t * p_obj_id,
		uint32_t obj_id_len,
		uint32_t flags,
		const uint8_t *p_plain_text,
		uint32_t plain_text_len,
		uint8_t** pp_cipher_text,
		uint32_t *p_cipher_text_len,
		uint32_t algorithm_id);

qapi_Status_t crypto_helper_persistent_obj_asym_decrypt(
		uint8_t * p_obj_id,
		uint32_t obj_id_len,
		uint32_t flags,
		uint8_t* p_cipher_text,
		uint32_t cipher_text_len,
		uint32_t algorithm_id,
		uint8_t **pp_decrypted_plain_text,
		uint32_t *p_decrypted_plain_text_len);

qapi_Status_t crypto_helper_persistent_obj_ecdh(
		uint8_t *p_ecc_key_pair_obj_id,
		uint32_t ecc_key_pair_obj_id_length,
		const char *p_peer_public_key_pem,
		const char* p_peer_public_key_pem_pass_phrase,
		uint8_t **pp_secret,
		uint32_t *p_secret_length);

qapi_Status_t crypto_helper_persistent_obj_rename (
		uint8_t * p_obj_id,
		uint32_t obj_id_len,
		uint8_t * p_new_obj_id,
		uint32_t new_obj_id_len,
		uint32_t flags);

qapi_Status_t crypto_helper_persistent_obj_delete (
		uint8_t * p_obj_id,
		uint32_t obj_id_len,
		uint32_t flags);

qapi_Status_t crypto_helper_persistent_obj_get_obj_info(
		uint8_t * p_obj_id,
		uint32_t obj_id_len,
		qapi_Crypto_Obj_Info_t *p_obj_info);

qapi_Status_t crypto_helper_persistent_obj_get_buffer_attibute(
		uint8_t * p_obj_id,
		uint32_t obj_id_len,
		uint32_t attribute_id,
		void *buffer,
		uint32_t buffer_size);

qapi_Status_t crypto_helper_persistent_obj_get_value_attibute(
		uint8_t * p_obj_id,
		uint32_t obj_id_len,
		uint32_t attribute_id,
		uint32_t *attribute_value_a,
		uint32_t *attribute_value_b);

qapi_Status_t crypto_helper_persistent_obj_clone(
		uint8_t *p_template_obj_id,
		uint32_t template_obj_id_len,
		uint8_t *p_obj_id,
		uint32_t obj_id_len);

#endif //__CRYPTO_HELPER_H__
