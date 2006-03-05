/* Automatically generated header! Do not edit! */

#ifndef _PPCINLINE_AMISSL_H
#define _PPCINLINE_AMISSL_H

#ifndef __PPCINLINE_MACROS_H
#include <ppcinline/macros.h>
#endif /* !__PPCINLINE_MACROS_H */

#ifndef AMISSL_BASE_NAME
#define AMISSL_BASE_NAME AmiSSLBase
#endif /* !AMISSL_BASE_NAME */

#define EVP_read_pw_string(__p0, __p1, __p2, __p3) \
	LP4(2400, int , EVP_read_pw_string, \
		char *, __p0, a0, \
		int , __p1, a1, \
		const char *, __p2, a2, \
		int , __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_CRL_INFO_free(__p0) \
	LP1NR(7062, X509_CRL_INFO_free, \
		X509_CRL_INFO *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_EXTENSION_set_critical(__p0, __p1) \
	LP2(7686, int , X509_EXTENSION_set_critical, \
		X509_EXTENSION *, __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define RSA_public_encrypt(__p0, __p1, __p2, __p3, __p4) \
	LP5(4332, int , RSA_public_encrypt, \
		int , __p0, a0, \
		unsigned char *, __p1, a1, \
		unsigned char *, __p2, a2, \
		RSA *, __p3, a3, \
		int , __p4, d0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_set_read_ahead(__p0, __p1) \
	LP2NR(4746, SSL_set_read_ahead, \
		SSL *, __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_get_current_cipher(__p0) \
	LP1(4650, SSL_CIPHER *, SSL_get_current_cipher, \
		SSL *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSLv3_server_method() \
	LP0(5214, SSL_METHOD *, SSLv3_server_method, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ASN1_STRING_dup(__p0) \
	LP1(204, ASN1_STRING *, ASN1_STRING_dup, \
		ASN1_STRING *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509V3_get_section(__p0, __p1) \
	LP2(6288, STACK *, X509V3_get_section, \
		X509V3_CTX *, __p0, a0, \
		char *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PEM_read_bio_X509_REQ(__p0, __p1, __p2, __p3) \
	LP4(3408, X509_REQ *, PEM_read_bio_X509_REQ, \
		BIO *, __p0, a0, \
		X509_REQ **, __p1, a1, \
		pem_password_cb *, __p2, a2, \
		void *, __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_PKCS7_SIGN_ENVELOPE(__p0, __p1) \
	LP2(3990, int , i2d_PKCS7_SIGN_ENVELOPE, \
		PKCS7_SIGN_ENVELOPE *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_free_all(__p0) \
	LP1NR(930, BIO_free_all, \
		BIO *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ASN1_STRING_type_new(__p0) \
	LP1(210, ASN1_STRING *, ASN1_STRING_type_new, \
		int , __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_get_version(__p0) \
	LP1(5178, char *, SSL_get_version, \
		SSL *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_generate_prime(__p0, __p1, __p2, __p3, __p4, __p5, __p6) \
	LP7(1524, BIGNUM *, BN_generate_prime, \
		BIGNUM *, __p0, a0, \
		int , __p1, a1, \
		int , __p2, a2, \
		BIGNUM *, __p3, a3, \
		BIGNUM *, __p4, d0, \
		APTR , __p5, d1, \
		void *, __p6, d2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_BLINDING_update(__p0, __p1) \
	LP2(1632, int , BN_BLINDING_update, \
		BN_BLINDING *, __p0, a0, \
		BN_CTX *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ASN1_UTCTIME_print(__p0, __p1) \
	LP2(672, int , ASN1_UTCTIME_print, \
		BIO *, __p0, a0, \
		ASN1_UTCTIME *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define DSA_free(__p0) \
	LP1NR(2154, DSA_free, \
		DSA *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define RSA_private_decrypt(__p0, __p1, __p2, __p3, __p4) \
	LP5(4350, int , RSA_private_decrypt, \
		int , __p0, a0, \
		unsigned char *, __p1, a1, \
		unsigned char *, __p2, a2, \
		RSA *, __p3, a3, \
		int , __p4, d0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define DSA_SIG_free(__p0) \
	LP1NR(2094, DSA_SIG_free, \
		DSA_SIG *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_add_client_CA(__p0, __p1) \
	LP2(5346, int , SSL_add_client_CA, \
		SSL *, __p0, a0, \
		X509 *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ASN1_TYPE_new() \
	LP0(132, ASN1_TYPE *, ASN1_TYPE_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define InitAmiSSLA(__p0) \
	LP1(30, long , InitAmiSSLA, \
		struct TagItem *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS12_key_gen_uni(__p0, __p1, __p2, __p3, __p4, __p5, __p6, __p7, __p8) \
	LP9(3636, int , PKCS12_key_gen_uni, \
		unsigned char *, __p0, a0, \
		int , __p1, a1, \
		unsigned char *, __p2, a2, \
		int , __p3, a3, \
		int , __p4, d0, \
		int , __p5, d1, \
		int , __p6, d2, \
		unsigned char *, __p7, d3, \
		const EVP_MD *, __p8, d4, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_VAL_new() \
	LP0(6738, X509_VAL *, X509_VAL_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define v2i_GENERAL_NAMES(__p0, __p1, __p2) \
	LP3(5988, STACK *, v2i_GENERAL_NAMES, \
		X509V3_EXT_METHOD *, __p0, a0, \
		X509V3_CTX *, __p1, a1, \
		STACK *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_X509_REVOKED(__p0, __p1) \
	LP2(7044, int , i2d_X509_REVOKED, \
		X509_REVOKED *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2a_ASN1_OBJECT(__p0, __p1) \
	LP2(516, int , i2a_ASN1_OBJECT, \
		BIO *, __p0, a0, \
		ASN1_OBJECT *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS12_create(__p0, __p1, __p2, __p3, __p4, __p5, __p6, __p7, __p8, __p9) \
	LP10(3792, PKCS12 *, PKCS12_create, \
		char *, __p0, a0, \
		char *, __p1, a1, \
		EVP_PKEY *, __p2, a2, \
		X509 *, __p3, a3, \
		STACK *, __p4, d0, \
		int , __p5, d1, \
		int , __p6, d2, \
		int , __p7, d3, \
		int , __p8, d4, \
		int , __p9, d5, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CRYPTO_malloc_locked(__p0) \
	LP1(1944, void *, CRYPTO_malloc_locked, \
		int , __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_ctrl_get_read_request(__p0) \
	LP1(8088, size_t , BIO_ctrl_get_read_request, \
		BIO *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_div(__p0, __p1, __p2, __p3, __p4) \
	LP5(1296, int , BN_div, \
		BIGNUM *, __p0, a0, \
		BIGNUM *, __p1, a1, \
		const BIGNUM *, __p2, a2, \
		const BIGNUM *, __p3, a3, \
		BN_CTX *, __p4, d0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_ASN1_TYPE(__p0, __p1) \
	LP2(144, int , i2d_ASN1_TYPE, \
		ASN1_TYPE *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SXNET_add_id_INTEGER(__p0, __p1, __p2, __p3) \
	LP4(5886, int , SXNET_add_id_INTEGER, \
		SXNET **, __p0, a0, \
		ASN1_INTEGER *, __p1, a1, \
		char *, __p2, a2, \
		int , __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_free(__p0) \
	LP1NR(5124, SSL_free, \
		SSL *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS7_ISSUER_AND_SERIAL_free(__p0) \
	LP1NR(3816, PKCS7_ISSUER_AND_SERIAL_free, \
		PKCS7_ISSUER_AND_SERIAL *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509V3_EXT_conf(__p0, __p1, __p2, __p3) \
	LP4(6240, X509_EXTENSION *, X509V3_EXT_conf, \
		LHASH *, __p0, a0, \
		X509V3_CTX *, __p1, a1, \
		char *, __p2, a2, \
		char *, __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_load_crl_file(__p0, __p1, __p2) \
	LP3(7944, int , X509_load_crl_file, \
		X509_LOOKUP *, __p0, a0, \
		const char *, __p1, a1, \
		int , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_BLINDING_convert(__p0, __p1, __p2) \
	LP3(1638, int , BN_BLINDING_convert, \
		BIGNUM *, __p0, a0, \
		BN_BLINDING *, __p1, a1, \
		BN_CTX *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSLv2_client_method() \
	LP0(5202, SSL_METHOD *, SSLv2_client_method, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_enc_null() \
	LP0(2664, EVP_CIPHER *, EVP_enc_null, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define RSA_get_ex_data(__p0, __p1) \
	LP2(4548, char *, RSA_get_ex_data, \
		RSA *, __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_DIST_POINT_NAME(__p0, __p1, __p2) \
	LP3(6216, DIST_POINT_NAME *, d2i_DIST_POINT_NAME, \
		DIST_POINT_NAME **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define DIST_POINT_free(__p0) \
	LP1NR(6192, DIST_POINT_free, \
		DIST_POINT *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_NAME_get_index_by_OBJ(__p0, __p1, __p2) \
	LP3(7440, int , X509_NAME_get_index_by_OBJ, \
		X509_NAME *, __p0, a0, \
		ASN1_OBJECT *, __p1, a1, \
		int , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define OBJ_ln2nid(__p0) \
	LP1(3240, int , OBJ_ln2nid, \
		const char *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_X509_PUBKEY(__p0, __p1, __p2) \
	LP3(6780, X509_PUBKEY *, d2i_X509_PUBKEY, \
		X509_PUBKEY **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_sock_non_fatal_error(__p0) \
	LP1(1032, int , BIO_sock_non_fatal_error, \
		int , __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_SignFinal(__p0, __p1, __p2, __p3) \
	LP4(2478, int , EVP_SignFinal, \
		EVP_MD_CTX *, __p0, a0, \
		unsigned char *, __p1, a1, \
		unsigned int *, __p2, a2, \
		EVP_PKEY *, __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_set_fd(__p0, __p1) \
	LP2(4704, int , SSL_set_fd, \
		SSL *, __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_REVOKED_get_ext_by_NID(__p0, __p1, __p2) \
	LP3(7632, int , X509_REVOKED_get_ext_by_NID, \
		X509_REVOKED *, __p0, a0, \
		int , __p1, a1, \
		int , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_NAME_ENTRY_dup(__p0) \
	LP1(6630, X509_NAME_ENTRY *, X509_NAME_ENTRY_dup, \
		X509_NAME_ENTRY *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_set_session_id_context(__p0, __p1, __p2) \
	LP3(5118, int , SSL_set_session_id_context, \
		SSL *, __p0, a0, \
		const unsigned char *, __p1, a1, \
		unsigned int , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SXNET_free(__p0) \
	LP1NR(5844, SXNET_free, \
		SXNET *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_rc4() \
	LP0(2748, EVP_CIPHER *, EVP_rc4, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CONF_get_section(__p0, __p1) \
	LP2(1770, STACK *, CONF_get_section, \
		LHASH *, __p0, a0, \
		char *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define DHparams_print(__p0, __p1) \
	LP2(2076, int , DHparams_print, \
		BIO *, __p0, a0, \
		DH *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_set_params(__p0, __p1, __p2, __p3) \
	LP4NR(1650, BN_set_params, \
		int , __p0, a0, \
		int , __p1, a1, \
		int , __p2, a2, \
		int , __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_EncryptInit(__p0, __p1, __p2, __p3) \
	LP4NR(2424, EVP_EncryptInit, \
		EVP_CIPHER_CTX *, __p0, a0, \
		const EVP_CIPHER *, __p1, a1, \
		unsigned char *, __p2, a2, \
		unsigned char *, __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_init(__p0) \
	LP1NR(1224, BN_init, \
		BIGNUM *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_copy_next_retry(__p0) \
	LP1NR(1152, BIO_copy_next_retry, \
		BIO *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_CRL_INFO_new() \
	LP0(7056, X509_CRL_INFO *, X509_CRL_INFO_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_bin2bn(__p0, __p1, __p2) \
	LP3(1242, BIGNUM *, BN_bin2bn, \
		const unsigned char *, __p0, a0, \
		int , __p1, a1, \
		BIGNUM *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_ASN1_UTCTIME(__p0, __p1) \
	LP2(438, int , i2d_ASN1_UTCTIME, \
		ASN1_UTCTIME *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define RSA_set_method(__p0, __p1) \
	LP2(8190, RSA_METHOD *, RSA_set_method, \
		RSA *, __p0, a0, \
		RSA_METHOD *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_CTX_set_tmp_dh_callback(__p0, __p1) \
	LP2NR(5580, SSL_CTX_set_tmp_dh_callback, \
		SSL_CTX *, __p0, a0, \
		APTR , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_ASN1_UINTEGER(__p0, __p1, __p2) \
	LP3(276, ASN1_INTEGER *, d2i_ASN1_UINTEGER, \
		ASN1_INTEGER **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_is_bit_set(__p0, __p1) \
	LP2(1368, int , BN_is_bit_set, \
		const BIGNUM *, __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define DH_new() \
	LP0(2022, DH *, DH_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define RSA_generate_key(__p0, __p1, __p2, __p3) \
	LP4(4326, RSA *, RSA_generate_key, \
		int , __p0, a0, \
		unsigned long , __p1, a1, \
		APTR , __p2, a2, \
		void *, __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_CTX_set_cert_verify_callback(__p0, __p1, __p2) \
	LP3NR(5046, SSL_CTX_set_cert_verify_callback, \
		SSL_CTX *, __p0, a0, \
		APTR , __p1, a1, \
		char *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS12_new() \
	LP0(3714, PKCS12 *, PKCS12_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_CRL_new() \
	LP0(7080, X509_CRL *, X509_CRL_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_NAME_ENTRY_get_data(__p0) \
	LP1(7494, ASN1_STRING *, X509_NAME_ENTRY_get_data, \
		X509_NAME_ENTRY *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CRYPTO_get_lock_name(__p0) \
	LP1(1908, const char *, CRYPTO_get_lock_name, \
		int , __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_get_shutdown(__p0) \
	LP1(5442, int , SSL_get_shutdown, \
		SSL *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define HMAC_Init(__p0, __p1, __p2, __p3) \
	LP4NR(3066, HMAC_Init, \
		HMAC_CTX *, __p0, a0, \
		const void *, __p1, a1, \
		int , __p2, a2, \
		const EVP_MD *, __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CRYPTO_dbg_free(__p0) \
	LP1NR(1992, CRYPTO_dbg_free, \
		void *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ERR_load_X509V3_strings() \
	LP0NR(5766, ERR_load_X509V3_strings, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ASN1_STRING_new() \
	LP0(192, ASN1_STRING *, ASN1_STRING_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_EXTENSION_set_data(__p0, __p1) \
	LP2(7692, int , X509_EXTENSION_set_data, \
		X509_EXTENSION *, __p0, a0, \
		ASN1_OCTET_STRING *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_Netscape_RSA(__p0, __p1, __p2) \
	LP3(4422, int , i2d_Netscape_RSA, \
		RSA *, __p0, a0, \
		unsigned char **, __p1, a1, \
		APTR , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS7_dataVerify(__p0, __p1, __p2, __p3, __p4) \
	LP5(4134, int , PKCS7_dataVerify, \
		X509_STORE *, __p0, a0, \
		X509_STORE_CTX *, __p1, a1, \
		BIO *, __p2, a2, \
		PKCS7 *, __p3, a3, \
		PKCS7_SIGNER_INFO *, __p4, d0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509v3_add_ext(__p0, __p1, __p2) \
	LP3(7536, STACK *, X509v3_add_ext, \
		STACK **, __p0, a0, \
		X509_EXTENSION *, __p1, a1, \
		int , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_clear_free(__p0) \
	LP1NR(1230, BN_clear_free, \
		BIGNUM *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_NAME_delete_entry(__p0, __p1) \
	LP2(7452, X509_NAME_ENTRY *, X509_NAME_delete_entry, \
		X509_NAME *, __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_EncryptFinal(__p0, __p1, __p2) \
	LP3NR(2436, EVP_EncryptFinal, \
		EVP_CIPHER_CTX *, __p0, a0, \
		unsigned char *, __p1, a1, \
		int *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define sk_value(__p0, __p1) \
	LP2(5604, char *, sk_value, \
		STACK *, __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_AUTHORITY_KEYID(__p0, __p1, __p2) \
	LP3(5916, AUTHORITY_KEYID *, d2i_AUTHORITY_KEYID, \
		AUTHORITY_KEYID **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CRYPTO_get_add_lock_callback() \
	LP0(1884, APTR , CRYPTO_get_add_lock_callback, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_PKEY2PKCS8(__p0) \
	LP1(7842, PKCS8_PRIV_KEY_INFO *, EVP_PKEY2PKCS8, \
		EVP_PKEY *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_CTX_free(__p0) \
	LP1NR(1194, BN_CTX_free, \
		BN_CTX *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define lh_node_stats_bio(__p0, __p1) \
	LP2NR(3150, lh_node_stats_bio, \
		LHASH *, __p0, a0, \
		BIO *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CRYPTO_set_ex_data(__p0, __p1, __p2) \
	LP3(1818, int , CRYPTO_set_ex_data, \
		CRYPTO_EX_DATA *, __p0, a0, \
		int , __p1, a1, \
		char *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS12_MAKE_KEYBAG(__p0) \
	LP1(3540, PKCS12_SAFEBAG *, PKCS12_MAKE_KEYBAG, \
		PKCS8_PRIV_KEY_INFO *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS5_PBE_add() \
	LP0NR(8136, PKCS5_PBE_add, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_PUBKEY_get(__p0) \
	LP1(6792, EVP_PKEY *, X509_PUBKEY_get, \
		X509_PUBKEY *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ERR_load_PKCS12_strings() \
	LP0NR(3774, ERR_load_PKCS12_strings, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_X509_NAME_ENTRY(__p0, __p1, __p2) \
	LP3(6948, X509_NAME_ENTRY *, d2i_X509_NAME_ENTRY, \
		X509_NAME_ENTRY **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_use_RSAPrivateKey_file(__p0, __p1, __p2) \
	LP3(4818, int , SSL_use_RSAPrivateKey_file, \
		SSL *, __p0, a0, \
		const char *, __p1, a1, \
		int , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_ASN1_BMPSTRING(__p0, __p1, __p2) \
	LP3(372, ASN1_BMPSTRING *, d2i_ASN1_BMPSTRING, \
		ASN1_BMPSTRING **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ASN1_TYPE_get_octetstring(__p0, __p1, __p2) \
	LP3(768, int , ASN1_TYPE_get_octetstring, \
		ASN1_TYPE *, __p0, a0, \
		unsigned char *, __p1, a1, \
		int , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_REQ_print(__p0, __p1) \
	LP2(7410, int , X509_REQ_print, \
		BIO *, __p0, a0, \
		X509_REQ *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_verify(__p0, __p1) \
	LP2(6450, int , X509_verify, \
		X509 *, __p0, a0, \
		EVP_PKEY *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_X509_CRL_bio(__p0, __p1) \
	LP2(6534, int , i2d_X509_CRL_bio, \
		BIO *, __p0, a0, \
		X509_CRL *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_ctrl_pending(__p0) \
	LP1(8070, size_t , BIO_ctrl_pending, \
		BIO *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_X509(__p0, __p1, __p2) \
	LP3(7026, X509 *, d2i_X509, \
		X509 **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define GENERAL_NAMES_new() \
	LP0(5958, STACK *, GENERAL_NAMES_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_get_fd(__p0) \
	LP1(4674, int , SSL_get_fd, \
		SSL *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define DSA_do_sign(__p0, __p1, __p2) \
	LP3(2112, DSA_SIG *, DSA_do_sign, \
		const unsigned char *, __p0, a0, \
		int , __p1, a1, \
		DSA *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_X509_REQ(__p0, __p1, __p2) \
	LP3(6870, X509_REQ *, d2i_X509_REQ, \
		X509_REQ **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_NAME_set(__p0, __p1) \
	LP2(6978, int , X509_NAME_set, \
		X509_NAME **, __p0, a0, \
		X509_NAME *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509V3_get_value_int(__p0, __p1) \
	LP2(6270, int , X509V3_get_value_int, \
		CONF_VALUE *, __p0, a0, \
		ASN1_INTEGER **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_cmp_current_time(__p0) \
	LP1(6648, int , X509_cmp_current_time, \
		ASN1_UTCTIME *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ASN1_verify(__p0, __p1, __p2, __p3, __p4) \
	LP5(7218, int , ASN1_verify, \
		APTR , __p0, a0, \
		X509_ALGOR *, __p1, a1, \
		ASN1_BIT_STRING *, __p2, a2, \
		char *, __p3, a3, \
		EVP_PKEY *, __p4, d0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_use_RSAPrivateKey(__p0, __p1) \
	LP2(4782, int , SSL_use_RSAPrivateKey, \
		SSL *, __p0, a0, \
		RSA *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_PKEY_encrypt(__p0, __p1, __p2, __p3) \
	LP4(2946, int , EVP_PKEY_encrypt, \
		unsigned char *, __p0, a0, \
		unsigned char *, __p1, a1, \
		int , __p2, a2, \
		EVP_PKEY *, __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_get_info_callback(__p0) \
	LP1(5484, APTR , SSL_get_info_callback, \
		SSL *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_RSAPrivateKey(__p0, __p1, __p2) \
	LP3(4404, RSA *, d2i_RSAPrivateKey, \
		RSA **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_SESSION_get_ex_data(__p0, __p1) \
	LP2(5532, void *, SSL_SESSION_get_ex_data, \
		SSL_SESSION *, __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_s_mem() \
	LP0(966, BIO_METHOD *, BIO_s_mem, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ERR_load_OBJ_strings() \
	LP0NR(3264, ERR_load_OBJ_strings, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define AmiSSLRealloc(__p0, __p1) \
	LP2(48, void *, AmiSSLRealloc, \
		void *, __p0, a0, \
		long , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PEM_read_bio_NETSCAPE_CERT_SEQUENCE(__p0, __p1, __p2, __p3) \
	LP4(3456, NETSCAPE_CERT_SEQUENCE *, PEM_read_bio_NETSCAPE_CERT_SEQUENCE, \
		BIO *, __p0, a0, \
		NETSCAPE_CERT_SEQUENCE **, __p1, a1, \
		pem_password_cb *, __p2, a2, \
		void *, __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_LOOKUP_file() \
	LP0(7908, X509_LOOKUP_METHOD *, X509_LOOKUP_file, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CRYPTO_mem_leaks(__p0) \
	LP1NR(2004, CRYPTO_mem_leaks, \
		struct bio_st *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_CIPHER_description(__p0, __p1, __p2) \
	LP3(5382, char *, SSL_CIPHER_description, \
		SSL_CIPHER *, __p0, a0, \
		char *, __p1, a1, \
		int , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_LOOKUP_hash_dir() \
	LP0(7902, X509_LOOKUP_METHOD *, X509_LOOKUP_hash_dir, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_NAME_ENTRY_new() \
	LP0(6930, X509_NAME_ENTRY *, X509_NAME_ENTRY_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define sk_delete(__p0, __p1) \
	LP2(5640, char *, sk_delete, \
		STACK *, __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_CTX_set_timeout(__p0, __p1) \
	LP2(4608, long , SSL_CTX_set_timeout, \
		SSL_CTX *, __p0, a0, \
		long , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define sk_set_cmp_func(__p0, __p1) \
	LP2(5688, int *, sk_set_cmp_func, \
		STACK *, __p0, a0, \
		APTR , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_set_info_callback(__p0, __p1) \
	LP2NR(5478, SSL_set_info_callback, \
		SSL *, __p0, a0, \
		APTR , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_LOOKUP_new(__p0) \
	LP1(7950, X509_LOOKUP *, X509_LOOKUP_new, \
		X509_LOOKUP_METHOD *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ERR_load_EVP_strings() \
	LP0NR(2562, ERR_load_EVP_strings, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define sk_zero(__p0) \
	LP1NR(5682, sk_zero, \
		STACK *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define name_cmp(__p0, __p1) \
	LP2(6432, int , name_cmp, \
		const char *, __p0, a0, \
		const char *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2v_GENERAL_NAMES(__p0, __p1, __p2) \
	LP3(5982, STACK *, i2v_GENERAL_NAMES, \
		X509V3_EXT_METHOD *, __p0, a0, \
		STACK *, __p1, a1, \
		STACK *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_RSAPrivateKey_bio(__p0, __p1) \
	LP2(6558, int , i2d_RSAPrivateKey_bio, \
		BIO *, __p0, a0, \
		RSA *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_new(__p0) \
	LP1(858, BIO *, BIO_new, \
		BIO_METHOD *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define NETSCAPE_SPKAC_free(__p0) \
	LP1NR(7158, NETSCAPE_SPKAC_free, \
		NETSCAPE_SPKAC *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_rc5_32_12_16_cbc() \
	LP0(2868, EVP_CIPHER *, EVP_rc5_32_12_16_cbc, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define OBJ_txt2obj(__p0, __p1) \
	LP2(3228, ASN1_OBJECT *, OBJ_txt2obj, \
		const char *, __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_STORE_CTX_set_cert(__p0, __p1) \
	LP2NR(8058, X509_STORE_CTX_set_cert, \
		X509_STORE_CTX *, __p0, a0, \
		X509 *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_socket_nbio(__p0, __p1) \
	LP2(1074, int , BIO_socket_nbio, \
		int , __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PEM_SealUpdate(__p0, __p1, __p2, __p3, __p4) \
	LP5NR(3354, PEM_SealUpdate, \
		PEM_ENCODE_SEAL_CTX *, __p0, a0, \
		unsigned char *, __p1, a1, \
		int *, __p2, a2, \
		unsigned char *, __p3, a3, \
		int , __p4, d0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PEM_write_bio_PKCS7(__p0, __p1) \
	LP2(3510, int , PEM_write_bio_PKCS7, \
		BIO *, __p0, a0, \
		PKCS7 *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define NETSCAPE_SPKAC_new() \
	LP0(7152, NETSCAPE_SPKAC *, NETSCAPE_SPKAC_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PEM_write_bio_PKCS8(__p0, __p1) \
	LP2(8256, int , PEM_write_bio_PKCS8, \
		BIO *, __p0, a0, \
		X509_SIG *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_get_ex_data(__p0, __p1) \
	LP2(828, char *, BIO_get_ex_data, \
		BIO *, __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define RSA_print(__p0, __p1, __p2) \
	LP3(4416, int , RSA_print, \
		BIO *, __p0, a0, \
		RSA *, __p1, a1, \
		int , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define lh_node_usage_stats_bio(__p0, __p1) \
	LP2NR(3156, lh_node_usage_stats_bio, \
		LHASH *, __p0, a0, \
		BIO *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define RAND_set_rand_method(__p0) \
	LP1NR(4248, RAND_set_rand_method, \
		RAND_METHOD *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_check_private_key(__p0, __p1) \
	LP2(7332, int , X509_check_private_key, \
		X509 *, __p0, a0, \
		EVP_PKEY *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_bn2mpi(__p0, __p1) \
	LP2(1260, int , BN_bn2mpi, \
		const BIGNUM *, __p0, a0, \
		unsigned char *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_DecryptInit(__p0, __p1, __p2, __p3) \
	LP4NR(2442, EVP_DecryptInit, \
		EVP_CIPHER_CTX *, __p0, a0, \
		const EVP_CIPHER *, __p1, a1, \
		unsigned char *, __p2, a2, \
		unsigned char *, __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_DSAPrivateKey(__p0, __p1) \
	LP2(2202, int , i2d_DSAPrivateKey, \
		DSA *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PEM_write_bio_X509(__p0, __p1) \
	LP2(3468, int , PEM_write_bio_X509, \
		BIO *, __p0, a0, \
		X509 *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_OBJECT_retrieve_by_subject(__p0, __p1, __p2) \
	LP3(7854, X509_OBJECT *, X509_OBJECT_retrieve_by_subject, \
		LHASH *, __p0, a0, \
		int , __p1, a1, \
		X509_NAME *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define OBJ_NAME_new_index(__p0, __p1, __p2) \
	LP3(3168, int , OBJ_NAME_new_index, \
		APTR , __p0, a0, \
		APTR , __p1, a1, \
		APTR , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BUF_MEM_free(__p0) \
	LP1NR(1710, BUF_MEM_free, \
		BUF_MEM *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_set_pw_prompt(__p0) \
	LP1NR(2406, EVP_set_pw_prompt, \
		char *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_ASN1_INTEGER(__p0, __p1) \
	LP2(264, int , i2d_ASN1_INTEGER, \
		ASN1_INTEGER *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_add_dir_cert_subjects_to_stack(__p0, __p1) \
	LP2(4872, int , SSL_add_dir_cert_subjects_to_stack, \
		STACK *, __p0, a0, \
		const char *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_X509_CRL_INFO(__p0, __p1) \
	LP2(7068, int , i2d_X509_CRL_INFO, \
		X509_CRL_INFO *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_ripemd160() \
	LP0(2658, EVP_MD *, EVP_ripemd160, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ERR_get_next_error_library() \
	LP0(2370, int , ERR_get_next_error_library, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_PKCS7_DIGEST(__p0, __p1) \
	LP2(4014, int , i2d_PKCS7_DIGEST, \
		PKCS7_DIGEST *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_sqr(__p0, __p1, __p2) \
	LP3(1308, int , BN_sqr, \
		BIGNUM *, __p0, a0, \
		BIGNUM *, __p1, a1, \
		BN_CTX *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ERR_load_X509_strings() \
	LP0NR(6708, ERR_load_X509_strings, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_ssl_shutdown(__p0) \
	LP1NR(4584, BIO_ssl_shutdown, \
		BIO *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PEM_SignInit(__p0, __p1) \
	LP2NR(3366, PEM_SignInit, \
		EVP_MD_CTX *, __p0, a0, \
		EVP_MD *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define bn_add_words(__p0, __p1, __p2, __p3) \
	LP4(1566, BN_ULONG , bn_add_words, \
		BN_ULONG *, __p0, a0, \
		BN_ULONG *, __p1, a1, \
		BN_ULONG *, __p2, a2, \
		int , __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define RSA_padding_check_none(__p0, __p1, __p2, __p3, __p4) \
	LP5(4530, int , RSA_padding_check_none, \
		unsigned char *, __p0, a0, \
		int , __p1, a1, \
		unsigned char *, __p2, a2, \
		int , __p3, a3, \
		int , __p4, d0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_set_serialNumber(__p0, __p1) \
	LP2(7242, int , X509_set_serialNumber, \
		X509 *, __p0, a0, \
		ASN1_INTEGER *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ASN1_d2i_bio(__p0, __p1, __p2, __p3) \
	LP4(660, char *, ASN1_d2i_bio, \
		APTR , __p0, a0, \
		APTR , __p1, a1, \
		BIO *, __p2, a2, \
		unsigned char **, __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define AmiSSLFree(__p0) \
	LP1NR(54, AmiSSLFree, \
		void *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_shutdown(__p0) \
	LP1(5280, int , SSL_shutdown, \
		SSL *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS7_set_cipher(__p0, __p1) \
	LP2(4194, int , PKCS7_set_cipher, \
		PKCS7 *, __p0, a0, \
		const EVP_CIPHER *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_issuer_name_cmp(__p0, __p1) \
	LP2(7350, int , X509_issuer_name_cmp, \
		X509 *, __p0, a0, \
		X509 *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS12_pack_p7data(__p0) \
	LP1(3558, PKCS7 *, PKCS12_pack_p7data, \
		STACK *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define asn1_Finish(__p0) \
	LP1(624, int , asn1_Finish, \
		ASN1_CTX *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_OBJECT_up_ref_count(__p0) \
	LP1NR(7860, X509_OBJECT_up_ref_count, \
		X509_OBJECT *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_rand(__p0, __p1, __p2, __p3) \
	LP4(1200, int , BN_rand, \
		BIGNUM *, __p0, a0, \
		int , __p1, a1, \
		int , __p2, a2, \
		int , __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_set_bio(__p0, __p1, __p2) \
	LP3NR(4722, SSL_set_bio, \
		SSL *, __p0, a0, \
		BIO *, __p1, a1, \
		BIO *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_new_buffer_ssl_connect(__p0) \
	LP1(4572, BIO *, BIO_new_buffer_ssl_connect, \
		SSL_CTX *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_verify_cert(__p0) \
	LP1(7716, int , X509_verify_cert, \
		X509_STORE_CTX *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_set_shutdown(__p0, __p1) \
	LP2NR(5436, SSL_set_shutdown, \
		SSL *, __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ERR_clear_error() \
	LP0NR(2280, ERR_clear_error, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS7_set_attributes(__p0, __p1) \
	LP2(4242, int , PKCS7_set_attributes, \
		PKCS7_SIGNER_INFO *, __p0, a0, \
		STACK *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_ASN1_HEADER(__p0, __p1, __p2) \
	LP3(708, ASN1_HEADER *, d2i_ASN1_HEADER, \
		ASN1_HEADER **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_PKCS12_SAFEBAG(__p0, __p1) \
	LP2(3750, int , i2d_PKCS12_SAFEBAG, \
		PKCS12_SAFEBAG *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ERR_load_DH_strings() \
	LP0NR(2082, ERR_load_DH_strings, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_NOTICEREF(__p0, __p1, __p2) \
	LP3(6138, NOTICEREF *, d2i_NOTICEREF, \
		NOTICEREF **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509V3_EXT_val_prn(__p0, __p1, __p2, __p3) \
	LP4NR(6438, X509V3_EXT_val_prn, \
		BIO *, __p0, a0, \
		STACK *, __p1, a1, \
		int , __p2, a2, \
		int , __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509v3_get_ext_by_OBJ(__p0, __p1, __p2) \
	LP3(7512, int , X509v3_get_ext_by_OBJ, \
		const STACK *, __p0, a0, \
		ASN1_OBJECT *, __p1, a1, \
		int , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_issuer_and_serial_hash(__p0) \
	LP1(7344, unsigned long , X509_issuer_and_serial_hash, \
		X509 *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_set_notBefore(__p0, __p1) \
	LP2(7278, int , X509_set_notBefore, \
		X509 *, __p0, a0, \
		ASN1_UTCTIME *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2a_ASN1_STRING(__p0, __p1, __p2) \
	LP3(528, int , i2a_ASN1_STRING, \
		BIO *, __p0, a0, \
		ASN1_STRING *, __p1, a1, \
		int , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define DIST_POINT_new() \
	LP0(6180, DIST_POINT *, DIST_POINT_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_MONT_CTX_set(__p0, __p1, __p2) \
	LP3(1608, int , BN_MONT_CTX_set, \
		BN_MONT_CTX *, __p0, a0, \
		const BIGNUM *, __p1, a1, \
		BN_CTX *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ERR_load_DSA_strings() \
	LP0NR(2160, ERR_load_DSA_strings, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_STORE_CTX_get_ex_new_index(__p0, __p1, __p2, __p3, __p4) \
	LP5(8010, int , X509_STORE_CTX_get_ex_new_index, \
		long , __p0, a0, \
		char *, __p1, a1, \
		APTR , __p2, a2, \
		APTR , __p3, a3, \
		APTR , __p4, d0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_PKCS12_bio(__p0, __p1) \
	LP2(3798, int , i2d_PKCS12_bio, \
		BIO *, __p0, a0, \
		PKCS12 *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ERR_load_CRYPTO_strings() \
	LP0NR(2016, ERR_load_CRYPTO_strings, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509V3_EXT_add_list(__p0) \
	LP1(6366, int , X509V3_EXT_add_list, \
		X509V3_EXT_METHOD *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_X509_VAL(__p0, __p1, __p2) \
	LP3(6756, X509_VAL *, d2i_X509_VAL, \
		X509_VAL **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_REQ_set_pubkey(__p0, __p1) \
	LP2(7320, int , X509_REQ_set_pubkey, \
		X509_REQ *, __p0, a0, \
		EVP_PKEY *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_CERTIFICATEPOLICIES(__p0, __p1, __p2) \
	LP3(6048, STACK *, d2i_CERTIFICATEPOLICIES, \
		STACK **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_POLICYINFO(__p0, __p1, __p2) \
	LP3(6066, POLICYINFO *, d2i_POLICYINFO, \
		POLICYINFO **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PEM_SealInit(__p0, __p1, __p2, __p3, __p4, __p5, __p6, __p7) \
	LP8(3348, int , PEM_SealInit, \
		PEM_ENCODE_SEAL_CTX *, __p0, a0, \
		EVP_CIPHER *, __p1, a1, \
		EVP_MD *, __p2, a2, \
		unsigned char **, __p3, a3, \
		int *, __p4, d0, \
		unsigned char *, __p5, d1, \
		EVP_PKEY **, __p6, d2, \
		int , __p7, d3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_get_rbio(__p0) \
	LP1(4728, BIO *, SSL_get_rbio, \
		SSL *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define NETSCAPE_SPKI_verify(__p0, __p1) \
	LP2(6474, int , NETSCAPE_SPKI_verify, \
		NETSCAPE_SPKI *, __p0, a0, \
		EVP_PKEY *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define sk_push(__p0, __p1) \
	LP2(5658, int , sk_push, \
		STACK *, __p0, a0, \
		char *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS12_SAFEBAG_free(__p0) \
	LP1NR(3768, PKCS12_SAFEBAG_free, \
		PKCS12_SAFEBAG *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_bn2dec(__p0) \
	LP1(1494, char *, BN_bn2dec, \
		const BIGNUM *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_REQ_free(__p0) \
	LP1NR(6858, X509_REQ_free, \
		X509_REQ *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_CRL_print(__p0, __p1) \
	LP2(7404, int , X509_CRL_print, \
		BIO *, __p0, a0, \
		X509_CRL *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_subject_name_cmp(__p0, __p1) \
	LP2(7362, int , X509_subject_name_cmp, \
		X509 *, __p0, a0, \
		X509 *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define lh_insert(__p0, __p1) \
	LP2(3108, char *, lh_insert, \
		LHASH *, __p0, a0, \
		char *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_ASN1_GENERALIZEDTIME(__p0, __p1) \
	LP2(450, int , i2d_ASN1_GENERALIZEDTIME, \
		ASN1_GENERALIZEDTIME *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_bn2bin(__p0, __p1) \
	LP2(1248, int , BN_bn2bin, \
		const BIGNUM *, __p0, a0, \
		unsigned char *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_REVOKED_new() \
	LP0(7032, X509_REVOKED *, X509_REVOKED_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_ext_ku(__p0, __p1, __p2) \
	LP3(6012, STACK *, d2i_ext_ku, \
		STACK **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_GENERAL_NAMES(__p0, __p1) \
	LP2(5976, int , i2d_GENERAL_NAMES, \
		STACK *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS7_set_signed_attributes(__p0, __p1) \
	LP2(4236, int , PKCS7_set_signed_attributes, \
		PKCS7_SIGNER_INFO *, __p0, a0, \
		STACK *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_PKCS12(__p0, __p1) \
	LP2(3702, int , i2d_PKCS12, \
		PKCS12 *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_bf_cbc() \
	LP0(2826, EVP_CIPHER *, EVP_bf_cbc, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PBE2PARAM_free(__p0) \
	LP1NR(7806, PBE2PARAM_free, \
		PBE2PARAM *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CRYPTO_new_ex_data(__p0, __p1, __p2) \
	LP3NR(1842, CRYPTO_new_ex_data, \
		STACK *, __p0, a0, \
		char *, __p1, a1, \
		CRYPTO_EX_DATA *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_set_session(__p0, __p1) \
	LP2(4980, int , SSL_set_session, \
		SSL *, __p0, a0, \
		SSL_SESSION *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SXNET_get_id_ulong(__p0, __p1) \
	LP2(5898, ASN1_OCTET_STRING *, SXNET_get_id_ulong, \
		SXNET *, __p0, a0, \
		unsigned long , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_SESSION_get_time(__p0) \
	LP1(4914, long , SSL_SESSION_get_time, \
		SSL_SESSION *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define DSA_SIG_new() \
	LP0(2088, DSA_SIG *, DSA_SIG_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CONF_load(__p0, __p1, __p2) \
	LP3(1764, LHASH *, CONF_load, \
		LHASH *, __p0, a0, \
		const char *, __p1, a1, \
		long *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CRYPTO_get_locked_mem_functions(__p0, __p1) \
	LP2NR(1938, CRYPTO_get_locked_mem_functions, \
		APTR *, __p0, a0, \
		APTR *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_CTX_new() \
	LP0(1182, BN_CTX *, BN_CTX_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define RSA_blinding_on(__p0, __p1) \
	LP2(4464, int , RSA_blinding_on, \
		RSA *, __p0, a0, \
		BN_CTX *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_ASN1_PRINTABLESTRING(__p0, __p1, __p2) \
	LP3(390, ASN1_PRINTABLESTRING *, d2i_ASN1_PRINTABLESTRING, \
		ASN1_PRINTABLESTRING **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_rc5_32_12_16_cfb() \
	LP0(2880, EVP_CIPHER *, EVP_rc5_32_12_16_cfb, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ERR_load_BIO_strings() \
	LP0NR(1122, ERR_load_BIO_strings, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define sk_pop(__p0) \
	LP1(5676, char *, sk_pop, \
		STACK *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS12_decrypt_d2i(__p0, __p1, __p2, __p3, __p4, __p5, __p6) \
	LP7(3612, char *, PKCS12_decrypt_d2i, \
		X509_ALGOR *, __p0, a0, \
		char , __p1, a1, \
		APTR , __p2, a2, \
		const char *, __p3, a3, \
		int , __p4, d0, \
		ASN1_STRING *, __p5, d1, \
		int , __p6, d2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2s_ASN1_INTEGER(__p0, __p1) \
	LP2(6336, char *, i2s_ASN1_INTEGER, \
		X509V3_EXT_METHOD *, __p0, a0, \
		ASN1_INTEGER *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define DH_free(__p0) \
	LP1NR(2028, DH_free, \
		DH *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_NAME_ENTRY_create_by_OBJ(__p0, __p1, __p2, __p3, __p4) \
	LP5(7470, X509_NAME_ENTRY *, X509_NAME_ENTRY_create_by_OBJ, \
		X509_NAME_ENTRY **, __p0, a0, \
		ASN1_OBJECT *, __p1, a1, \
		int , __p2, a2, \
		unsigned char *, __p3, a3, \
		int , __p4, d0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS7_RECIP_INFO_new() \
	LP0(3882, PKCS7_RECIP_INFO *, PKCS7_RECIP_INFO_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define RSA_new_method(__p0) \
	LP1(4314, RSA *, RSA_new_method, \
		RSA_METHOD *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PEM_write_bio_PKCS8PrivateKey(__p0, __p1, __p2, __p3, __p4, __p5, __p6) \
	LP7(8268, int , PEM_write_bio_PKCS8PrivateKey, \
		BIO *, __p0, a0, \
		EVP_PKEY *, __p1, a1, \
		const EVP_CIPHER *, __p2, a2, \
		char *, __p3, a3, \
		int , __p4, d0, \
		pem_password_cb *, __p5, d1, \
		void *, __p6, d2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PEM_read_bio_DSAPrivateKey(__p0, __p1, __p2, __p3) \
	LP4(3432, DSA *, PEM_read_bio_DSAPrivateKey, \
		BIO *, __p0, a0, \
		DSA **, __p1, a1, \
		pem_password_cb *, __p2, a2, \
		void *, __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ERR_get_error_line_data(__p0, __p1, __p2, __p3) \
	LP4(2256, unsigned long , ERR_get_error_line_data, \
		const char **, __p0, a0, \
		int *, __p1, a1, \
		const char **, __p2, a2, \
		int *, __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define RSAPrivateKey_dup(__p0) \
	LP1(6642, RSA *, RSAPrivateKey_dup, \
		RSA *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_ALGOR_dup(__p0) \
	LP1(6618, X509_ALGOR *, X509_ALGOR_dup, \
		X509_ALGOR *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_get_certificate(__p0) \
	LP1(5400, X509 *, SSL_get_certificate, \
		SSL *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CRYPTO_free_locked(__p0) \
	LP1NR(1950, CRYPTO_free_locked, \
		void *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PEM_write_bio_DHparams(__p0, __p1) \
	LP2(3516, int , PEM_write_bio_DHparams, \
		BIO *, __p0, a0, \
		DH *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_CTX_use_RSAPrivateKey(__p0, __p1) \
	LP2(5052, int , SSL_CTX_use_RSAPrivateKey, \
		SSL_CTX *, __p0, a0, \
		RSA *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ASN1_GENERALIZEDTIME_print(__p0, __p1) \
	LP2(678, int , ASN1_GENERALIZEDTIME_print, \
		BIO *, __p0, a0, \
		ASN1_GENERALIZEDTIME *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_sub(__p0, __p1, __p2) \
	LP3(1266, int , BN_sub, \
		BIGNUM *, __p0, a0, \
		const BIGNUM *, __p1, a1, \
		const BIGNUM *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_print(__p0, __p1) \
	LP2(1428, int , BN_print, \
		BIO *, __p0, a0, \
		const BIGNUM *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_GENERAL_NAME(__p0, __p1) \
	LP2(5796, int , i2d_GENERAL_NAME, \
		GENERAL_NAME *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ASN1_TYPE_set_int_octetstring(__p0, __p1, __p2, __p3) \
	LP4(774, int , ASN1_TYPE_set_int_octetstring, \
		ASN1_TYPE *, __p0, a0, \
		long , __p1, a1, \
		unsigned char *, __p2, a2, \
		int , __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_dup_chain(__p0) \
	LP1(954, BIO *, BIO_dup_chain, \
		BIO *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define asn1_add_error(__p0, __p1) \
	LP2NR(816, asn1_add_error, \
		unsigned char *, __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_PKEY_type(__p0) \
	LP1(2952, int , EVP_PKEY_type, \
		int , __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS7_RECIP_INFO_free(__p0) \
	LP1NR(3888, PKCS7_RECIP_INFO_free, \
		PKCS7_RECIP_INFO *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_X509_ALGOR(__p0, __p1, __p2) \
	LP3(6732, X509_ALGOR *, d2i_X509_ALGOR, \
		X509_ALGOR **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_rc5_32_12_16_ecb() \
	LP0(2874, EVP_CIPHER *, EVP_rc5_32_12_16_ecb, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SXNET_get_id_INTEGER(__p0, __p1) \
	LP2(5904, ASN1_OCTET_STRING *, SXNET_get_id_INTEGER, \
		SXNET *, __p0, a0, \
		ASN1_INTEGER *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_INFO_new() \
	LP0(7200, X509_INFO *, X509_INFO_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_printfA(__p0, __p1) \
	LP2(1164, int , BIO_printfA, \
		BIO *, __p0, a0, \
		void *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_X509_bio(__p0, __p1) \
	LP2(6516, X509 *, d2i_X509_bio, \
		BIO *, __p0, a0, \
		X509 **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ERR_func_error_string(__p0) \
	LP1(2298, const char *, ERR_func_error_string, \
		unsigned long , __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_to_X509_REQ(__p0, __p1, __p2) \
	LP3(6696, X509_REQ *, X509_to_X509_REQ, \
		X509 *, __p0, a0, \
		EVP_PKEY *, __p1, a1, \
		EVP_MD *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_s_connect() \
	LP0(978, BIO_METHOD *, BIO_s_connect, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CRYPTO_get_ex_data(__p0, __p1) \
	LP2(1824, char *, CRYPTO_get_ex_data, \
		CRYPTO_EX_DATA *, __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS7_get_signer_info(__p0) \
	LP1(4170, STACK *, PKCS7_get_signer_info, \
		PKCS7 *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CONF_get_string(__p0, __p1, __p2) \
	LP3(1776, char *, CONF_get_string, \
		LHASH *, __p0, a0, \
		char *, __p1, a1, \
		char *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define POLICYINFO_free(__p0) \
	LP1NR(6072, POLICYINFO_free, \
		POLICYINFO *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_PKCS7_SIGNED(__p0, __p1, __p2) \
	LP3(3924, PKCS7_SIGNED *, d2i_PKCS7_SIGNED, \
		PKCS7_SIGNED **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define DSA_new() \
	LP0(2124, DSA *, DSA_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS7_SIGNED_free(__p0) \
	LP1NR(3912, PKCS7_SIGNED_free, \
		PKCS7_SIGNED *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_ASN1_BIT_STRING(__p0, __p1, __p2) \
	LP3(234, ASN1_BIT_STRING *, d2i_ASN1_BIT_STRING, \
		ASN1_BIT_STRING **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_REQ_dup(__p0) \
	LP1(6612, X509_REQ *, X509_REQ_dup, \
		X509_REQ *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define RSA_memory_lock(__p0) \
	LP1(4374, int , RSA_memory_lock, \
		RSA *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_PBEPARAM(__p0, __p1) \
	LP2(7734, int , i2d_PBEPARAM, \
		PBEPARAM *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_is_prime(__p0, __p1, __p2, __p3, __p4) \
	LP5(1530, int , BN_is_prime, \
		BIGNUM *, __p0, a0, \
		int , __p1, a1, \
		APTR , __p2, a2, \
		BN_CTX *, __p3, a3, \
		void *, __p4, d0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ASN1_pack_string(__p0, __p1, __p2) \
	LP3(804, ASN1_STRING *, ASN1_pack_string, \
		void *, __p0, a0, \
		APTR , __p1, a1, \
		ASN1_OCTET_STRING **, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define RSAPrivateKey_asn1_meth() \
	LP0(744, ASN1_METHOD *, RSAPrivateKey_asn1_meth, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ASN1_TIME_print(__p0, __p1) \
	LP2(684, int , ASN1_TIME_print, \
		BIO *, __p0, a0, \
		ASN1_TIME *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define sk_num(__p0) \
	LP1(5598, int , sk_num, \
		STACK *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_PKEY_USAGE_PERIOD(__p0, __p1, __p2) \
	LP3(5940, PKEY_USAGE_PERIOD *, d2i_PKEY_USAGE_PERIOD, \
		PKEY_USAGE_PERIOD **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_ASN1_UTF8STRING(__p0, __p1) \
	LP2(354, int , i2d_ASN1_UTF8STRING, \
		ASN1_UTF8STRING *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_STORE_CTX_set_ex_data(__p0, __p1, __p2) \
	LP3(8016, int , X509_STORE_CTX_set_ex_data, \
		X509_STORE_CTX *, __p0, a0, \
		int , __p1, a1, \
		void *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ERR_add_error_dataA(__p0, __p1) \
	LP2NR(2316, ERR_add_error_dataA, \
		int , __p0, a0, \
		void *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_PKCS7_ENC_CONTENT(__p0, __p1) \
	LP2(3942, int , i2d_PKCS7_ENC_CONTENT, \
		PKCS7_ENC_CONTENT *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_NAME_digest(__p0, __p1, __p2, __p3) \
	LP4(6510, int , X509_NAME_digest, \
		X509_NAME *, __p0, a0, \
		EVP_MD *, __p1, a1, \
		unsigned char *, __p2, a2, \
		unsigned int *, __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ERR_remove_state(__p0) \
	LP1NR(2346, ERR_remove_state, \
		unsigned long , __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PEM_read_bio(__p0, __p1, __p2, __p3, __p4) \
	LP5(3312, int , PEM_read_bio, \
		BIO *, __p0, a0, \
		char **, __p1, a1, \
		char **, __p2, a2, \
		unsigned char **, __p3, a3, \
		long *, __p4, d0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_dup(__p0) \
	LP1(1464, BIGNUM *, BN_dup, \
		const BIGNUM *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_DSA_SIG(__p0, __p1, __p2) \
	LP3(2106, DSA_SIG *, d2i_DSA_SIG, \
		DSA_SIG **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509V3_set_ctx(__p0, __p1, __p2, __p3, __p4, __p5) \
	LP6NR(6306, X509V3_set_ctx, \
		X509V3_CTX *, __p0, a0, \
		X509 *, __p1, a1, \
		X509 *, __p2, a2, \
		X509_REQ *, __p3, a3, \
		X509_CRL *, __p4, d0, \
		int , __p5, d1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_EncodeUpdate(__p0, __p1, __p2, __p3, __p4) \
	LP5NR(2520, EVP_EncodeUpdate, \
		EVP_ENCODE_CTX *, __p0, a0, \
		unsigned char *, __p1, a1, \
		int *, __p2, a2, \
		unsigned char *, __p3, a3, \
		int , __p4, d0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define lh_new(__p0, __p1) \
	LP2(3096, LHASH *, lh_new, \
		APTR , __p0, a0, \
		APTR , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_STORE_CTX_set_chain(__p0, __p1) \
	LP2NR(8064, X509_STORE_CTX_set_chain, \
		X509_STORE_CTX *, __p0, a0, \
		STACK *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define sk_find(__p0, __p1) \
	LP2(5652, int , sk_find, \
		STACK *, __p0, a0, \
		char *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_BLINDING_new(__p0, __p1, __p2) \
	LP3(1620, BN_BLINDING *, BN_BLINDING_new, \
		BIGNUM *, __p0, a0, \
		BIGNUM *, __p1, a1, \
		BIGNUM *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_PKCS7_bio(__p0, __p1) \
	LP2(3846, PKCS7 *, d2i_PKCS7_bio, \
		BIO *, __p0, a0, \
		PKCS7 **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define RSA_size(__p0) \
	LP1(4320, int , RSA_size, \
		RSA *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_dss1() \
	LP0(2646, EVP_MD *, EVP_dss1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_CTX_get_timeout(__p0) \
	LP1(4614, long , SSL_CTX_get_timeout, \
		SSL_CTX *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_des_ede3() \
	LP0(2682, EVP_CIPHER *, EVP_des_ede3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS5_pbe_set(__p0, __p1, __p2, __p3) \
	LP4(7758, X509_ALGOR *, PKCS5_pbe_set, \
		int , __p0, a0, \
		int , __p1, a1, \
		unsigned char *, __p2, a2, \
		int , __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_state_string(__p0) \
	LP1(4890, char *, SSL_state_string, \
		SSL *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_CTX_add_client_CA(__p0, __p1) \
	LP2(5352, int , SSL_CTX_add_client_CA, \
		SSL_CTX *, __p0, a0, \
		X509 *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_PKCS7_ENCRYPT(__p0, __p1) \
	LP2(4038, int , i2d_PKCS7_ENCRYPT, \
		PKCS7_ENCRYPT *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SXNET_add_id_ulong(__p0, __p1, __p2, __p3) \
	LP4(5880, int , SXNET_add_id_ulong, \
		SXNET **, __p0, a0, \
		unsigned long , __p1, a1, \
		char *, __p2, a2, \
		int , __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define asn1_GetSequence(__p0, __p1) \
	LP2(810, int , asn1_GetSequence, \
		ASN1_CTX *, __p0, a0, \
		long *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_STORE_CTX_get_error_depth(__p0) \
	LP1(8040, int , X509_STORE_CTX_get_error_depth, \
		X509_STORE_CTX *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_NAME_get_text_by_NID(__p0, __p1, __p2, __p3) \
	LP4(7422, int , X509_NAME_get_text_by_NID, \
		X509_NAME *, __p0, a0, \
		int , __p1, a1, \
		char *, __p2, a2, \
		int , __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS12_set_mac(__p0, __p1, __p2, __p3, __p4, __p5, __p6) \
	LP7(3654, int , PKCS12_set_mac, \
		PKCS12 *, __p0, a0, \
		const char *, __p1, a1, \
		int , __p2, a2, \
		unsigned char *, __p3, a3, \
		int , __p4, d0, \
		int , __p5, d1, \
		EVP_MD *, __p6, d2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_NAME_entry_count(__p0) \
	LP1(7416, int , X509_NAME_entry_count, \
		X509_NAME *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_RECP_CTX_new() \
	LP0(1668, BN_RECP_CTX *, BN_RECP_CTX_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PEM_read_bio_PKCS8_PRIV_KEY_INFO(__p0, __p1, __p2, __p3) \
	LP4(8250, PKCS8_PRIV_KEY_INFO *, PEM_read_bio_PKCS8_PRIV_KEY_INFO, \
		BIO *, __p0, a0, \
		PKCS8_PRIV_KEY_INFO **, __p1, a1, \
		pem_password_cb *, __p2, a2, \
		void *, __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS7_add_signer(__p0, __p1) \
	LP2(4110, int , PKCS7_add_signer, \
		PKCS7 *, __p0, a0, \
		PKCS7_SIGNER_INFO *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_PKEY_new() \
	LP0(7104, X509_PKEY *, X509_PKEY_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS12_BAGS_new() \
	LP0(3684, PKCS12_BAGS *, PKCS12_BAGS_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_certificate_type(__p0, __p1) \
	LP2(7302, int , X509_certificate_type, \
		X509 *, __p0, a0, \
		EVP_PKEY *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS7_get_attribute(__p0, __p1) \
	LP2(4224, ASN1_TYPE *, PKCS7_get_attribute, \
		PKCS7_SIGNER_INFO *, __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BASIC_CONSTRAINTS_free(__p0) \
	LP1NR(5790, BASIC_CONSTRAINTS_free, \
		BASIC_CONSTRAINTS *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_rc2_40_cbc() \
	LP0(2796, EVP_CIPHER *, EVP_rc2_40_cbc, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_bf_cfb() \
	LP0(2832, EVP_CIPHER *, EVP_bf_cfb, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_ATTRIBUTE_create(__p0, __p1, __p2) \
	LP3(6900, X509_ATTRIBUTE *, X509_ATTRIBUTE_create, \
		int , __p0, a0, \
		int , __p1, a1, \
		void *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_DSAPublicKey(__p0, __p1) \
	LP2(2196, int , i2d_DSAPublicKey, \
		DSA *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ASN1_INTEGER_get(__p0) \
	LP1(558, long , ASN1_INTEGER_get, \
		ASN1_INTEGER *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_f_nbio_test() \
	LP0(1020, BIO_METHOD *, BIO_f_nbio_test, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS7_SIGNER_INFO_set(__p0, __p1, __p2, __p3) \
	LP4(4104, int , PKCS7_SIGNER_INFO_set, \
		PKCS7_SIGNER_INFO *, __p0, a0, \
		X509 *, __p1, a1, \
		EVP_PKEY *, __p2, a2, \
		EVP_MD *, __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define DH_check(__p0, __p1) \
	LP2(2046, int , DH_check, \
		DH *, __p0, a0, \
		int *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_VerifyFinal(__p0, __p1, __p2, __p3) \
	LP4(2484, int , EVP_VerifyFinal, \
		EVP_MD_CTX *, __p0, a0, \
		unsigned char *, __p1, a1, \
		unsigned int , __p2, a2, \
		EVP_PKEY *, __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PEM_read_bio_X509_CRL(__p0, __p1, __p2, __p3) \
	LP4(3414, X509_CRL *, PEM_read_bio_X509_CRL, \
		BIO *, __p0, a0, \
		X509_CRL **, __p1, a1, \
		pem_password_cb *, __p2, a2, \
		void *, __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS7_set_content(__p0, __p1) \
	LP2(4098, int , PKCS7_set_content, \
		PKCS7 *, __p0, a0, \
		PKCS7 *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CRYPTO_remalloc(__p0, __p1) \
	LP2(1974, void *, CRYPTO_remalloc, \
		void *, __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define TLSv1_server_method() \
	LP0(5250, SSL_METHOD *, TLSv1_server_method, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ERR_load_strings(__p0, __p1) \
	LP2NR(2322, ERR_load_strings, \
		int , __p0, a0, \
		ERR_STRING_DATA *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_ALGOR_new() \
	LP0(6714, X509_ALGOR *, X509_ALGOR_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_ssl_copy_session_id(__p0, __p1) \
	LP2(4578, int , BIO_ssl_copy_session_id, \
		BIO *, __p0, a0, \
		BIO *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2s_ASN1_OCTET_STRING(__p0, __p1) \
	LP2(5994, char *, i2s_ASN1_OCTET_STRING, \
		X509V3_EXT_METHOD *, __p0, a0, \
		ASN1_OCTET_STRING *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define RSA_padding_add_PKCS1_type_1(__p0, __p1, __p2, __p3) \
	LP4(4476, int , RSA_padding_add_PKCS1_type_1, \
		unsigned char *, __p0, a0, \
		int , __p1, a1, \
		unsigned char *, __p2, a2, \
		int , __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2a_ASN1_ENUMERATED(__p0, __p1) \
	LP2(504, int , i2a_ASN1_ENUMERATED, \
		BIO *, __p0, a0, \
		ASN1_ENUMERATED *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define RSA_padding_add_PKCS1_type_2(__p0, __p1, __p2, __p3) \
	LP4(4488, int , RSA_padding_add_PKCS1_type_2, \
		unsigned char *, __p0, a0, \
		int , __p1, a1, \
		unsigned char *, __p2, a2, \
		int , __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_push(__p0, __p1) \
	LP2(918, BIO *, BIO_push, \
		BIO *, __p0, a0, \
		BIO *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define OBJ_cmp(__p0, __p1) \
	LP2(3252, int , OBJ_cmp, \
		ASN1_OBJECT *, __p0, a0, \
		ASN1_OBJECT *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS7_cert_from_signer_info(__p0, __p1) \
	LP2(4164, X509 *, PKCS7_cert_from_signer_info, \
		PKCS7 *, __p0, a0, \
		PKCS7_SIGNER_INFO *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_bf_ecb() \
	LP0(2820, EVP_CIPHER *, EVP_bf_ecb, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_set_wfd(__p0, __p1) \
	LP2(4716, int , SSL_set_wfd, \
		SSL *, __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS12_MAC_DATA_free(__p0) \
	LP1NR(3744, PKCS12_MAC_DATA_free, \
		PKCS12_MAC_DATA *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define bn_div_words(__p0, __p1, __p2) \
	LP3(1560, BN_ULONG , bn_div_words, \
		BN_ULONG , __p0, a0, \
		BN_ULONG , __p1, a1, \
		BN_ULONG , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS5_PBKDF2_HMAC_SHA1(__p0, __p1, __p2, __p3, __p4, __p5, __p6) \
	LP7(8124, int , PKCS5_PBKDF2_HMAC_SHA1, \
		const char *, __p0, a0, \
		int , __p1, a1, \
		unsigned char *, __p2, a2, \
		int , __p3, a3, \
		int , __p4, d0, \
		int , __p5, d1, \
		unsigned char *, __p6, d2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define OBJ_NAME_cleanup(__p0) \
	LP1NR(3192, OBJ_NAME_cleanup, \
		int , __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_PKCS7_RECIP_INFO(__p0, __p1, __p2) \
	LP3(3900, PKCS7_RECIP_INFO *, d2i_PKCS7_RECIP_INFO, \
		PKCS7_RECIP_INFO **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_RSAPublicKey_bio(__p0, __p1) \
	LP2(6570, int , i2d_RSAPublicKey_bio, \
		BIO *, __p0, a0, \
		RSA *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_ASN1_IA5STRING(__p0, __p1) \
	LP2(426, int , i2d_ASN1_IA5STRING, \
		ASN1_IA5STRING *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_get_word(__p0) \
	LP1(1350, BN_ULONG , BN_get_word, \
		BIGNUM *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ms_time_cmp(__p0, __p1) \
	LP2(5724, int , ms_time_cmp, \
		char *, __p0, a0, \
		char *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ms_time_get(__p0) \
	LP1NR(5712, ms_time_get, \
		char *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_CTX_use_PrivateKey(__p0, __p1) \
	LP2(5064, int , SSL_CTX_use_PrivateKey, \
		SSL_CTX *, __p0, a0, \
		EVP_PKEY *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define DSA_is_prime(__p0, __p1, __p2) \
	LP3(2226, int , DSA_is_prime, \
		BIGNUM *, __p0, a0, \
		APTR , __p1, a1, \
		char *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_use_certificate_ASN1(__p0, __p1, __p2) \
	LP3(4812, int , SSL_use_certificate_ASN1, \
		SSL *, __p0, a0, \
		unsigned char *, __p1, a1, \
		int , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ASN1_STRING_print(__p0, __p1) \
	LP2(690, int , ASN1_STRING_print, \
		BIO *, __p0, a0, \
		ASN1_STRING *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_md_null() \
	LP0(2610, EVP_MD *, EVP_md_null, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS12_i2d_encrypt(__p0, __p1, __p2, __p3, __p4, __p5) \
	LP6(3618, ASN1_STRING *, PKCS12_i2d_encrypt, \
		X509_ALGOR *, __p0, a0, \
		APTR , __p1, a1, \
		const char *, __p2, a2, \
		int , __p3, a3, \
		char *, __p4, d0, \
		int , __p5, d1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CRYPTO_get_mem_functions(__p0, __p1, __p2) \
	LP3NR(1926, CRYPTO_get_mem_functions, \
		APTR *, __p0, a0, \
		APTR *, __p1, a1, \
		APTR *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_ASN1_bytes(__p0, __p1, __p2, __p3, __p4) \
	LP5(612, ASN1_STRING *, d2i_ASN1_bytes, \
		ASN1_STRING **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		int , __p3, a3, \
		int , __p4, d0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_SESSION_hash(__p0) \
	LP1(4950, unsigned long , SSL_SESSION_hash, \
		SSL_SESSION *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_DSAparams(__p0, __p1, __p2) \
	LP3(2178, DSA *, d2i_DSAparams, \
		DSA **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define a2i_ASN1_INTEGER(__p0, __p1, __p2, __p3) \
	LP4(498, int , a2i_ASN1_INTEGER, \
		BIO *, __p0, a0, \
		ASN1_INTEGER *, __p1, a1, \
		char *, __p2, a2, \
		int , __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ASN1_check_infinite_end(__p0, __p1) \
	LP2(636, int , ASN1_check_infinite_end, \
		unsigned char **, __p0, a0, \
		long , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CRYPTO_set_mem_functions(__p0, __p1, __p2) \
	LP3NR(1920, CRYPTO_set_mem_functions, \
		APTR , __p0, a0, \
		APTR , __p1, a1, \
		APTR , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BUF_MEM_new() \
	LP0(1704, BUF_MEM *, BUF_MEM_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CRYPTO_set_id_callback(__p0) \
	LP1NR(1890, CRYPTO_set_id_callback, \
		APTR , __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PEM_X509_INFO_write_bio(__p0, __p1, __p2, __p3, __p4, __p5, __p6) \
	LP7(3342, int , PEM_X509_INFO_write_bio, \
		BIO *, __p0, a0, \
		X509_INFO *, __p1, a1, \
		EVP_CIPHER *, __p2, a2, \
		unsigned char *, __p3, a3, \
		int , __p4, d0, \
		pem_password_cb *, __p5, d1, \
		void *, __p6, d2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_puts(__p0, __p1) \
	LP2(894, int , BIO_puts, \
		BIO *, __p0, a0, \
		const char *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_REQ_new() \
	LP0(6852, X509_REQ *, X509_REQ_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS12_get_attr_gen(__p0, __p1) \
	LP2(3594, ASN1_TYPE *, PKCS12_get_attr_gen, \
		STACK *, __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_PublicKey(__p0, __p1, __p2, __p3) \
	LP4(2988, EVP_PKEY *, d2i_PublicKey, \
		int , __p0, a0, \
		EVP_PKEY **, __p1, a1, \
		unsigned char **, __p2, a2, \
		long , __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CONF_get_number(__p0, __p1, __p2) \
	LP3(1782, long , CONF_get_number, \
		LHASH *, __p0, a0, \
		char *, __p1, a1, \
		char *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define lh_strhash(__p0) \
	LP1(3138, unsigned long , lh_strhash, \
		const char *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_PKCS7_ISSUER_AND_SERIAL(__p0, __p1, __p2) \
	LP3(3828, PKCS7_ISSUER_AND_SERIAL *, d2i_PKCS7_ISSUER_AND_SERIAL, \
		PKCS7_ISSUER_AND_SERIAL **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_ASN1_PRINTABLE(__p0, __p1, __p2) \
	LP3(384, ASN1_STRING *, d2i_ASN1_PRINTABLE, \
		ASN1_STRING **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_CRL_get_ext_by_OBJ(__p0, __p1, __p2) \
	LP3(7596, int , X509_CRL_get_ext_by_OBJ, \
		X509_CRL *, __p0, a0, \
		ASN1_OBJECT *, __p1, a1, \
		int , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_new() \
	LP0(1218, BIGNUM *, BN_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_PKCS8_bio(__p0, __p1) \
	LP2(8208, X509_SIG *, d2i_PKCS8_bio, \
		BIO *, __p0, a0, \
		X509_SIG **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_use_certificate(__p0, __p1) \
	LP2(4806, int , SSL_use_certificate, \
		SSL *, __p0, a0, \
		X509 *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_SESSION_print(__p0, __p1) \
	LP2(4962, int , SSL_SESSION_print, \
		BIO *, __p0, a0, \
		SSL_SESSION *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_new_file(__p0, __p1) \
	LP2(846, BIO *, BIO_new_file, \
		const char *, __p0, a0, \
		const char *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_PrivateKey(__p0, __p1, __p2, __p3) \
	LP4(3000, EVP_PKEY *, d2i_PrivateKey, \
		int , __p0, a0, \
		EVP_PKEY **, __p1, a1, \
		unsigned char **, __p2, a2, \
		long , __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ASN1_HEADER_new() \
	LP0(714, ASN1_HEADER *, ASN1_HEADER_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_set_verify_depth(__p0, __p1) \
	LP2NR(4776, SSL_set_verify_depth, \
		SSL *, __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_set_version(__p0, __p1) \
	LP2(7236, int , X509_set_version, \
		X509 *, __p0, a0, \
		long , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_X509_REQ_INFO(__p0, __p1, __p2) \
	LP3(6846, X509_REQ_INFO *, d2i_X509_REQ_INFO, \
		X509_REQ_INFO **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509V3_add_standard_extensions() \
	LP0(6396, int , X509V3_add_standard_extensions, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_get_cipher_list(__p0, __p1) \
	LP2(4680, const char *, SSL_get_cipher_list, \
		SSL *, __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_PBKDF2PARAM(__p0, __p1, __p2) \
	LP3(7776, PBKDF2PARAM *, d2i_PBKDF2PARAM, \
		PBKDF2PARAM **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define RSA_check_key(__p0) \
	LP1(8172, int , RSA_check_key, \
		RSA *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define DSA_dup_DH(__p0) \
	LP1(8112, DH *, DSA_dup_DH, \
		DSA *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_get_ex_new_index(__p0, __p1, __p2, __p3, __p4) \
	LP5(5520, int , SSL_get_ex_new_index, \
		long , __p0, a0, \
		char *, __p1, a1, \
		APTR , __p2, a2, \
		APTR , __p3, a3, \
		APTR , __p4, d0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSLv2_server_method() \
	LP0(5196, SSL_METHOD *, SSLv2_server_method, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define GENERAL_NAMES_free(__p0) \
	LP1NR(5964, GENERAL_NAMES_free, \
		STACK *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ASN1_TYPE_get(__p0) \
	LP1(156, int , ASN1_TYPE_get, \
		ASN1_TYPE *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_X509_REQ_bio(__p0, __p1) \
	LP2(6546, int , i2d_X509_REQ_bio, \
		BIO *, __p0, a0, \
		X509_REQ *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_get_session(__p0) \
	LP1(5466, SSL_SESSION *, SSL_get_session, \
		SSL *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_MONT_CTX_copy(__p0, __p1) \
	LP2(1614, BN_MONT_CTX *, BN_MONT_CTX_copy, \
		BN_MONT_CTX *, __p0, a0, \
		BN_MONT_CTX *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define RSAPublicKey_dup(__p0) \
	LP1(6636, RSA *, RSAPublicKey_dup, \
		RSA *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_bn2hex(__p0) \
	LP1(1488, char *, BN_bn2hex, \
		const BIGNUM *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ERR_peek_error_line_data(__p0, __p1, __p2, __p3) \
	LP4(2274, unsigned long , ERR_peek_error_line_data, \
		const char **, __p0, a0, \
		int *, __p1, a1, \
		const char **, __p2, a2, \
		int *, __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_sha1() \
	LP0(2634, EVP_MD *, EVP_sha1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS12_pack_safebag(__p0, __p1, __p2, __p3) \
	LP4(3534, PKCS12_SAFEBAG *, PKCS12_pack_safebag, \
		char *, __p0, a0, \
		APTR , __p1, a1, \
		int , __p2, a2, \
		int , __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ASN1_dup(__p0, __p1, __p2) \
	LP3(654, char *, ASN1_dup, \
		APTR , __p0, a0, \
		APTR , __p1, a1, \
		char *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CRYPTO_dbg_remalloc(__p0, __p1, __p2, __p3) \
	LP4(1998, void *, CRYPTO_dbg_remalloc, \
		void *, __p0, a0, \
		int , __p1, a1, \
		const char *, __p2, a2, \
		int , __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS7_ISSUER_AND_SERIAL_new() \
	LP0(3810, PKCS7_ISSUER_AND_SERIAL *, PKCS7_ISSUER_AND_SERIAL_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_SXNETID(__p0, __p1) \
	LP2(5850, int , i2d_SXNETID, \
		SXNETID *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_PKEY_size(__p0) \
	LP1(2964, int , EVP_PKEY_size, \
		EVP_PKEY *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define COMP_CTX_free(__p0) \
	LP1NR(1740, COMP_CTX_free, \
		COMP_CTX *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_CTX_get_verify_callback(__p0) \
	LP1(5028, APTR , SSL_CTX_get_verify_callback, \
		SSL_CTX *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PEM_write_bio_PKCS8_PRIV_KEY_INFO(__p0, __p1) \
	LP2(8262, int , PEM_write_bio_PKCS8_PRIV_KEY_INFO, \
		BIO *, __p0, a0, \
		PKCS8_PRIV_KEY_INFO *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define bn_expand2(__p0, __p1) \
	LP2(1458, BIGNUM *, bn_expand2, \
		BIGNUM *, __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_EXTENSION_get_object(__p0) \
	LP1(7698, ASN1_OBJECT *, X509_EXTENSION_get_object, \
		X509_EXTENSION *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_CTX_use_RSAPrivateKey_ASN1(__p0, __p1, __p2) \
	LP3(5058, int , SSL_CTX_use_RSAPrivateKey_ASN1, \
		SSL_CTX *, __p0, a0, \
		unsigned char *, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_des_ede_ofb() \
	LP0(2712, EVP_CIPHER *, EVP_des_ede_ofb, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_DIRECTORYSTRING(__p0, __p1) \
	LP2(396, int , i2d_DIRECTORYSTRING, \
		ASN1_STRING *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509V3_EXT_d2i(__p0) \
	LP1(6408, void *, X509V3_EXT_d2i, \
		X509_EXTENSION *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_des_cbc() \
	LP0(2724, EVP_CIPHER *, EVP_des_cbc, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_REQ_INFO_new() \
	LP0(6828, X509_REQ_INFO *, X509_REQ_INFO_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define COMP_rle() \
	LP0(1758, COMP_METHOD *, COMP_rle, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_idea_ofb() \
	LP0(2772, EVP_CIPHER *, EVP_idea_ofb, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ASN1_PRINTABLE_type(__p0, __p1) \
	LP2(600, int , ASN1_PRINTABLE_type, \
		unsigned char *, __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS12_free(__p0) \
	LP1NR(3720, PKCS12_free, \
		PKCS12 *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_NETSCAPE_CERT_SEQUENCE(__p0, __p1) \
	LP2(7176, int , i2d_NETSCAPE_CERT_SEQUENCE, \
		NETSCAPE_CERT_SEQUENCE *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_PBE2PARAM(__p0, __p1, __p2) \
	LP3(7800, PBE2PARAM *, d2i_PBE2PARAM, \
		PBE2PARAM **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_PKCS7_SIGNER_INFO(__p0, __p1, __p2) \
	LP3(3876, PKCS7_SIGNER_INFO *, d2i_PKCS7_SIGNER_INFO, \
		PKCS7_SIGNER_INFO **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_EXTENSION_create_by_NID(__p0, __p1, __p2, __p3) \
	LP4(7668, X509_EXTENSION *, X509_EXTENSION_create_by_NID, \
		X509_EXTENSION **, __p0, a0, \
		int , __p1, a1, \
		int , __p2, a2, \
		ASN1_OCTET_STRING *, __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS12_pbe_crypt(__p0, __p1, __p2, __p3, __p4, __p5, __p6, __p7) \
	LP8(3606, unsigned char *, PKCS12_pbe_crypt, \
		X509_ALGOR *, __p0, a0, \
		const char *, __p1, a1, \
		int , __p2, a2, \
		unsigned char *, __p3, a3, \
		int , __p4, d0, \
		unsigned char **, __p5, d1, \
		int *, __p6, d2, \
		int , __p7, d3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_write(__p0, __p1, __p2) \
	LP3(5154, int , SSL_write, \
		SSL *, __p0, a0, \
		const char *, __p1, a1, \
		int , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_des_ede3_ofb() \
	LP0(2718, EVP_CIPHER *, EVP_des_ede3_ofb, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_get_serialNumber(__p0) \
	LP1(7248, ASN1_INTEGER *, X509_get_serialNumber, \
		X509 *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ERR_peek_error_line(__p0, __p1) \
	LP2(2268, unsigned long , ERR_peek_error_line, \
		const char **, __p0, a0, \
		int *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_BytesToKey(__p0, __p1, __p2, __p3, __p4, __p5, __p6, __p7) \
	LP8(2418, int , EVP_BytesToKey, \
		const EVP_CIPHER *, __p0, a0, \
		EVP_MD *, __p1, a1, \
		unsigned char *, __p2, a2, \
		unsigned char *, __p3, a3, \
		int , __p4, d0, \
		int , __p5, d1, \
		unsigned char *, __p6, d2, \
		unsigned char *, __p7, d3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define DIST_POINT_NAME_free(__p0) \
	LP1NR(6210, DIST_POINT_NAME_free, \
		DIST_POINT_NAME *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_NAME_oneline(__p0, __p1, __p2) \
	LP3(7212, char *, X509_NAME_oneline, \
		X509_NAME *, __p0, a0, \
		char *, __p1, a1, \
		int , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509V3_EXT_add_alias(__p0, __p1) \
	LP2(6372, int , X509V3_EXT_add_alias, \
		int , __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_NETSCAPE_SPKAC(__p0, __p1, __p2) \
	LP3(7170, NETSCAPE_SPKAC *, d2i_NETSCAPE_SPKAC, \
		NETSCAPE_SPKAC **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CRYPTO_realloc(__p0, __p1) \
	LP2(1968, void *, CRYPTO_realloc, \
		void *, __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_EncodeBlock(__p0, __p1, __p2) \
	LP3(2532, int , EVP_EncodeBlock, \
		unsigned char *, __p0, a0, \
		unsigned char *, __p1, a1, \
		int , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define RSA_private_encrypt(__p0, __p1, __p2, __p3, __p4) \
	LP5(4338, int , RSA_private_encrypt, \
		int , __p0, a0, \
		unsigned char *, __p1, a1, \
		unsigned char *, __p2, a2, \
		RSA *, __p3, a3, \
		int , __p4, d0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_s_bio() \
	LP0(8094, BIO_METHOD *, BIO_s_bio, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS8_PRIV_KEY_INFO_new() \
	LP0(7818, PKCS8_PRIV_KEY_INFO *, PKCS8_PRIV_KEY_INFO_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS7_free(__p0) \
	LP1NR(4056, PKCS7_free, \
		PKCS7 *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_get_issuer_name(__p0) \
	LP1(7260, X509_NAME *, X509_get_issuer_name, \
		X509 *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_CipherFinal(__p0, __p1, __p2) \
	LP3(2472, int , EVP_CipherFinal, \
		EVP_CIPHER_CTX *, __p0, a0, \
		unsigned char *, __p1, a1, \
		int *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_set_pubkey(__p0, __p1) \
	LP2(7290, int , X509_set_pubkey, \
		X509 *, __p0, a0, \
		EVP_PKEY *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ERR_load_RSAREF_strings() \
	LP0NR(4302, ERR_load_RSAREF_strings, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_X509_CRL(__p0, __p1, __p2) \
	LP3(7098, X509_CRL *, d2i_X509_CRL, \
		X509_CRL **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_X509_NAME(__p0, __p1) \
	LP2(6966, int , i2d_X509_NAME, \
		X509_NAME *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define asc2uni(__p0, __p1, __p2) \
	LP3(3666, unsigned char *, asc2uni, \
		const char *, __p0, a0, \
		unsigned char **, __p1, a1, \
		int *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_PKEY_cmp_parameters(__p0, __p1) \
	LP2(3030, int , EVP_PKEY_cmp_parameters, \
		EVP_PKEY *, __p0, a0, \
		EVP_PKEY *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_REQ_set_subject_name(__p0, __p1) \
	LP2(7314, int , X509_REQ_set_subject_name, \
		X509_REQ *, __p0, a0, \
		X509_NAME *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define POLICYQUALINFO_new() \
	LP0(6084, POLICYQUALINFO *, POLICYQUALINFO_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_f_null() \
	LP0(1008, BIO_METHOD *, BIO_f_null, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SXNET_new() \
	LP0(5838, SXNET *, SXNET_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509V3_EXT_add_conf(__p0, __p1, __p2, __p3) \
	LP4(6246, int , X509V3_EXT_add_conf, \
		LHASH *, __p0, a0, \
		X509V3_CTX *, __p1, a1, \
		char *, __p2, a2, \
		X509 *, __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS7_add_recipient_info(__p0, __p1) \
	LP2(4182, int , PKCS7_add_recipient_info, \
		PKCS7 *, __p0, a0, \
		PKCS7_RECIP_INFO *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_fd_non_fatal_error(__p0) \
	LP1(1044, int , BIO_fd_non_fatal_error, \
		int , __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SXNETID_free(__p0) \
	LP1NR(5868, SXNETID_free, \
		SXNETID *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_use_PrivateKey(__p0, __p1) \
	LP2(4794, int , SSL_use_PrivateKey, \
		SSL *, __p0, a0, \
		EVP_PKEY *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PBE2PARAM_new() \
	LP0(7794, PBE2PARAM *, PBE2PARAM_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_ptr_ctrl(__p0, __p1, __p2) \
	LP3(906, char *, BIO_ptr_ctrl, \
		BIO *, __p0, a0, \
		int , __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_STORE_CTX_get_ex_data(__p0, __p1) \
	LP2(8022, void *, X509_STORE_CTX_get_ex_data, \
		X509_STORE_CTX *, __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define COMP_compress_block(__p0, __p1, __p2, __p3, __p4) \
	LP5(1746, int , COMP_compress_block, \
		COMP_CTX *, __p0, a0, \
		unsigned char *, __p1, a1, \
		int , __p2, a2, \
		unsigned char *, __p3, a3, \
		int , __p4, d0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define POLICYINFO_new() \
	LP0(6060, POLICYINFO *, POLICYINFO_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_X509_ATTRIBUTE(__p0, __p1) \
	LP2(6888, int , i2d_X509_ATTRIBUTE, \
		X509_ATTRIBUTE *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ASN1_ENUMERATED_set(__p0, __p1) \
	LP2(576, int , ASN1_ENUMERATED_set, \
		ASN1_ENUMERATED *, __p0, a0, \
		long , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ASN1_get_object(__p0, __p1, __p2, __p3, __p4) \
	LP5(630, int , ASN1_get_object, \
		unsigned char **, __p0, a0, \
		long *, __p1, a1, \
		int *, __p2, a2, \
		int *, __p3, a3, \
		long , __p4, d0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_pop(__p0) \
	LP1(924, BIO *, BIO_pop, \
		BIO *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_ALGOR_free(__p0) \
	LP1NR(6720, X509_ALGOR_free, \
		X509_ALGOR *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_add_ext(__p0, __p1, __p2) \
	LP3(7578, int , X509_add_ext, \
		X509 *, __p0, a0, \
		X509_EXTENSION *, __p1, a1, \
		int , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_PKEY_missing_parameters(__p0) \
	LP1(3018, int , EVP_PKEY_missing_parameters, \
		EVP_PKEY *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_set_subject_name(__p0, __p1) \
	LP2(7266, int , X509_set_subject_name, \
		X509 *, __p0, a0, \
		X509_NAME *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_LOOKUP_by_alias(__p0, __p1, __p2, __p3, __p4) \
	LP5(7986, int , X509_LOOKUP_by_alias, \
		X509_LOOKUP *, __p0, a0, \
		int , __p1, a1, \
		char *, __p2, a2, \
		int , __p3, a3, \
		X509_OBJECT *, __p4, d0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS7_get_issuer_and_serial(__p0, __p1) \
	LP2(4200, PKCS7_ISSUER_AND_SERIAL *, PKCS7_get_issuer_and_serial, \
		PKCS7 *, __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CRYPTO_get_locking_callback() \
	LP0(1872, APTR , CRYPTO_get_locking_callback, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_PKCS8_PRIV_KEY_INFO_bio(__p0, __p1) \
	LP2(8220, PKCS8_PRIV_KEY_INFO *, d2i_PKCS8_PRIV_KEY_INFO_bio, \
		BIO *, __p0, a0, \
		PKCS8_PRIV_KEY_INFO **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_RC5_IsReal() \
	LP0(114, long , EVP_RC5_IsReal, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS8_set_broken(__p0, __p1) \
	LP2(7848, PKCS8_PRIV_KEY_INFO *, PKCS8_set_broken, \
		PKCS8_PRIV_KEY_INFO *, __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_CRL_cmp(__p0, __p1) \
	LP2(7386, int , X509_CRL_cmp, \
		X509_CRL *, __p0, a0, \
		X509_CRL *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_X509_SIG(__p0, __p1) \
	LP2(6816, int , i2d_X509_SIG, \
		X509_SIG *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ERR_load_RSA_strings() \
	LP0NR(4386, ERR_load_RSA_strings, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ERR_peek_error() \
	LP0(2262, unsigned long , ERR_peek_error, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_CRL_free(__p0) \
	LP1NR(7086, X509_CRL_free, \
		X509_CRL *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_CTX_get_verify_mode(__p0) \
	LP1(5016, int , SSL_CTX_get_verify_mode, \
		SSL_CTX *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_clear(__p0) \
	LP1(4638, int , SSL_clear, \
		SSL *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_add(__p0, __p1, __p2) \
	LP3(1284, int , BN_add, \
		BIGNUM *, __p0, a0, \
		BIGNUM *, __p1, a1, \
		BIGNUM *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_ASN1_ENUMERATED(__p0, __p1, __p2) \
	LP3(288, ASN1_ENUMERATED *, d2i_ASN1_ENUMERATED, \
		ASN1_ENUMERATED **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_MONT_CTX_free(__p0) \
	LP1NR(1602, BN_MONT_CTX_free, \
		BN_MONT_CTX *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CONF_load_bio(__p0, __p1, __p2) \
	LP3(8274, LHASH *, CONF_load_bio, \
		LHASH *, __p0, a0, \
		BIO *, __p1, a1, \
		long *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_use_PrivateKey_file(__p0, __p1, __p2) \
	LP3(4824, int , SSL_use_PrivateKey_file, \
		SSL *, __p0, a0, \
		const char *, __p1, a1, \
		int , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PEM_write_bio_DSAPrivateKey(__p0, __p1, __p2, __p3, __p4, __p5, __p6) \
	LP7(3498, int , PEM_write_bio_DSAPrivateKey, \
		BIO *, __p0, a0, \
		DSA *, __p1, a1, \
		const EVP_CIPHER *, __p2, a2, \
		unsigned char *, __p3, a3, \
		int , __p4, d0, \
		pem_password_cb *, __p5, d1, \
		void *, __p6, d2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PEM_read_bio_RSAPublicKey(__p0, __p1, __p2, __p3) \
	LP4(3426, RSA *, PEM_read_bio_RSAPublicKey, \
		BIO *, __p0, a0, \
		RSA **, __p1, a1, \
		pem_password_cb *, __p2, a2, \
		void *, __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_SealInit(__p0, __p1, __p2, __p3, __p4, __p5, __p6) \
	LP7(2502, int , EVP_SealInit, \
		EVP_CIPHER_CTX *, __p0, a0, \
		EVP_CIPHER *, __p1, a1, \
		unsigned char **, __p2, a2, \
		int *, __p3, a3, \
		unsigned char *, __p4, d0, \
		EVP_PKEY **, __p5, d1, \
		int , __p6, d2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_CRL_get_ext(__p0, __p1) \
	LP2(7608, X509_EXTENSION *, X509_CRL_get_ext, \
		X509_CRL *, __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_gmtime_adj(__p0, __p1) \
	LP2(6654, ASN1_UTCTIME *, X509_gmtime_adj, \
		ASN1_UTCTIME *, __p0, a0, \
		long , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_set_connect_state(__p0) \
	LP1NR(5358, SSL_set_connect_state, \
		SSL *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_s_null() \
	LP0(1002, BIO_METHOD *, BIO_s_null, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ASN1_STRING_cmp(__p0, __p1) \
	LP2(216, int , ASN1_STRING_cmp, \
		ASN1_STRING *, __p0, a0, \
		ASN1_STRING *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_get_peer_cert_chain(__p0) \
	LP1(5010, STACK *, SSL_get_peer_cert_chain, \
		SSL *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ASN1_GENERALIZEDTIME_set_string(__p0, __p1) \
	LP2(324, int , ASN1_GENERALIZEDTIME_set_string, \
		ASN1_GENERALIZEDTIME *, __p0, a0, \
		char *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_to_ASN1_ENUMERATED(__p0, __p1) \
	LP2(588, ASN1_ENUMERATED *, BN_to_ASN1_ENUMERATED, \
		BIGNUM *, __p0, a0, \
		ASN1_ENUMERATED *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS7_add_crl(__p0, __p1) \
	LP2(4122, int , PKCS7_add_crl, \
		PKCS7 *, __p0, a0, \
		X509_CRL *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_BF_IsReal() \
	LP0(60, long , EVP_BF_IsReal, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_check_private_key(__p0) \
	LP1(5100, int , SSL_check_private_key, \
		SSL *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_get_params(__p0) \
	LP1(1656, int , BN_get_params, \
		int , __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_PKCS7_SIGN_ENVELOPE(__p0, __p1, __p2) \
	LP3(3996, PKCS7_SIGN_ENVELOPE *, d2i_PKCS7_SIGN_ENVELOPE, \
		PKCS7_SIGN_ENVELOPE **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_new_ssl(__p0, __p1) \
	LP2(4560, BIO *, BIO_new_ssl, \
		SSL_CTX *, __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509V3_EXT_cleanup() \
	LP0NR(6378, X509V3_EXT_cleanup, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_BASIC_CONSTRAINTS(__p0, __p1) \
	LP2(5772, int , i2d_BASIC_CONSTRAINTS, \
		BASIC_CONSTRAINTS *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ASN1_UTCTIME_set(__p0, __p1) \
	LP2(300, ASN1_UTCTIME *, ASN1_UTCTIME_set, \
		ASN1_UTCTIME *, __p0, a0, \
		time_t , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_ASN1_type_bytes(__p0, __p1, __p2, __p3) \
	LP4(618, ASN1_STRING *, d2i_ASN1_type_bytes, \
		ASN1_STRING **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		int , __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_SIG_free(__p0) \
	LP1NR(6810, X509_SIG_free, \
		X509_SIG *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_RECP_CTX_init(__p0) \
	LP1NR(1662, BN_RECP_CTX_init, \
		BN_RECP_CTX *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSLv2_method() \
	LP0(5190, SSL_METHOD *, SSLv2_method, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define lh_stats_bio(__p0, __p1) \
	LP2NR(3144, lh_stats_bio, \
		LHASH *, __p0, a0, \
		BIO *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_PBE_alg_add(__p0, __p1, __p2, __p3) \
	LP4(8148, int , EVP_PBE_alg_add, \
		int , __p0, a0, \
		EVP_CIPHER *, __p1, a1, \
		EVP_MD *, __p2, a2, \
		EVP_PBE_KEYGEN *, __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define TXT_DB_read(__p0, __p1) \
	LP2(5730, TXT_DB *, TXT_DB_read, \
		BIO *, __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_EncodeFinal(__p0, __p1, __p2) \
	LP3NR(2526, EVP_EncodeFinal, \
		EVP_ENCODE_CTX *, __p0, a0, \
		unsigned char *, __p1, a1, \
		int *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define OBJ_nid2obj(__p0) \
	LP1(3204, ASN1_OBJECT *, OBJ_nid2obj, \
		int , __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define OBJ_obj2nid(__p0) \
	LP1(3222, int , OBJ_obj2nid, \
		ASN1_OBJECT *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_LOOKUP_init(__p0) \
	LP1(7962, int , X509_LOOKUP_init, \
		X509_LOOKUP *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_X509_REVOKED(__p0, __p1, __p2) \
	LP3(7050, X509_REVOKED *, d2i_X509_REVOKED, \
		X509_REVOKED **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define DSA_print(__p0, __p1, __p2) \
	LP3(2220, int , DSA_print, \
		BIO *, __p0, a0, \
		DSA *, __p1, a1, \
		int , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define RSA_get_method(__p0) \
	LP1(8184, RSA_METHOD *, RSA_get_method, \
		RSA *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS12_MAC_DATA_new() \
	LP0(3732, PKCS12_MAC_DATA *, PKCS12_MAC_DATA_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_REQ_INFO_free(__p0) \
	LP1NR(6834, X509_REQ_INFO_free, \
		X509_REQ_INFO *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_ctrl_wpending(__p0) \
	LP1(8076, size_t , BIO_ctrl_wpending, \
		BIO *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_ATTRIBUTE_free(__p0) \
	LP1NR(6882, X509_ATTRIBUTE_free, \
		X509_ATTRIBUTE *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSLv23_method() \
	LP0(5226, SSL_METHOD *, SSLv23_method, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS12_add_friendlyname_asc(__p0, __p1, __p2) \
	LP3(3576, int , PKCS12_add_friendlyname_asc, \
		PKCS12_SAFEBAG *, __p0, a0, \
		const char *, __p1, a1, \
		int , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_des_cfb() \
	LP0(2688, EVP_CIPHER *, EVP_des_cfb, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_ASN1_TYPE(__p0, __p1, __p2) \
	LP3(150, ASN1_TYPE *, d2i_ASN1_TYPE, \
		ASN1_TYPE **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define RSA_padding_add_none(__p0, __p1, __p2, __p3) \
	LP4(4524, int , RSA_padding_add_none, \
		unsigned char *, __p0, a0, \
		int , __p1, a1, \
		unsigned char *, __p2, a2, \
		int , __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PBKDF2PARAM_new() \
	LP0(7770, PBKDF2PARAM *, PBKDF2PARAM_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_rc2_cbc() \
	LP0(2790, EVP_CIPHER *, EVP_rc2_cbc, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CRL_DIST_POINTS_new() \
	LP0(6156, STACK *, CRL_DIST_POINTS_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_EXTENSION_dup(__p0) \
	LP1(6600, X509_EXTENSION *, X509_EXTENSION_dup, \
		X509_EXTENSION *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_CTX_get_client_CA_list(__p0) \
	LP1(5340, STACK *, SSL_CTX_get_client_CA_list, \
		SSL_CTX *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PEM_write_bio_NETSCAPE_CERT_SEQUENCE(__p0, __p1) \
	LP2(3528, int , PEM_write_bio_NETSCAPE_CERT_SEQUENCE, \
		BIO *, __p0, a0, \
		NETSCAPE_CERT_SEQUENCE *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS7_SIGN_ENVELOPE_new() \
	LP0(3978, PKCS7_SIGN_ENVELOPE *, PKCS7_SIGN_ENVELOPE_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_NAME_dup(__p0) \
	LP1(6624, X509_NAME *, X509_NAME_dup, \
		X509_NAME *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ERR_load_SSL_strings() \
	LP0NR(4878, ERR_load_SSL_strings, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_PKCS7_ENVELOPE(__p0, __p1) \
	LP2(3966, int , i2d_PKCS7_ENVELOPE, \
		PKCS7_ENVELOPE *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_set_notAfter(__p0, __p1) \
	LP2(7284, int , X509_set_notAfter, \
		X509 *, __p0, a0, \
		ASN1_UTCTIME *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CRYPTO_dbg_malloc(__p0, __p1, __p2) \
	LP3(1980, void *, CRYPTO_dbg_malloc, \
		int , __p0, a0, \
		const char *, __p1, a1, \
		int , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ERR_get_string_table() \
	LP0(2358, LHASH *, ERR_get_string_table, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_CIPHER_CTX_init(__p0) \
	LP1NR(2568, EVP_CIPHER_CTX_init, \
		EVP_CIPHER_CTX *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define DSA_sign(__p0, __p1, __p2, __p3, __p4, __p5) \
	LP6(2142, int , DSA_sign, \
		int , __p0, a0, \
		const unsigned char *, __p1, a1, \
		int , __p2, a2, \
		unsigned char *, __p3, a3, \
		unsigned int *, __p4, d0, \
		DSA *, __p5, d1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_NAME_ENTRY_set_data(__p0, __p1, __p2, __p3) \
	LP4(7482, int , X509_NAME_ENTRY_set_data, \
		X509_NAME_ENTRY *, __p0, a0, \
		int , __p1, a1, \
		unsigned char *, __p2, a2, \
		int , __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_set_ssl_method(__p0, __p1) \
	LP2(5292, int , SSL_set_ssl_method, \
		SSL *, __p0, a0, \
		SSL_METHOD *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_des_ecb() \
	LP0(2670, EVP_CIPHER *, EVP_des_ecb, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_cast5_cbc() \
	LP0(2850, EVP_CIPHER *, EVP_cast5_cbc, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS12_PBE_add() \
	LP0NR(3780, PKCS12_PBE_add, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_gethostbyname(__p0) \
	LP1(1056, struct hostent *, BIO_gethostbyname, \
		const char *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_REVOKED_get_ext(__p0, __p1) \
	LP2(7650, X509_EXTENSION *, X509_REVOKED_get_ext, \
		X509_REVOKED *, __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PEM_X509_INFO_read_bio(__p0, __p1, __p2, __p3) \
	LP4(3336, STACK *, PEM_X509_INFO_read_bio, \
		BIO *, __p0, a0, \
		STACK *, __p1, a1, \
		pem_password_cb *, __p2, a2, \
		void *, __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_DecodeBlock(__p0, __p1, __p2) \
	LP3(2556, int , EVP_DecodeBlock, \
		unsigned char *, __p0, a0, \
		unsigned char *, __p1, a1, \
		int , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ASN1_OBJECT_create(__p0, __p1, __p2, __p3, __p4) \
	LP5(546, ASN1_OBJECT *, ASN1_OBJECT_create, \
		int , __p0, a0, \
		unsigned char *, __p1, a1, \
		int , __p2, a2, \
		char *, __p3, a3, \
		char *, __p4, d0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_RIPEMD160_IsReal() \
	LP0(120, long , EVP_RIPEMD160_IsReal, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define OBJ_nid2ln(__p0) \
	LP1(3210, const char *, OBJ_nid2ln, \
		int , __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define lh_delete(__p0, __p1) \
	LP2(3114, char *, lh_delete, \
		LHASH *, __p0, a0, \
		char *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_rstate_string_long(__p0) \
	LP1(4908, char *, SSL_rstate_string_long, \
		SSL *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ERR_put_error(__p0, __p1, __p2, __p3, __p4) \
	LP5NR(2232, ERR_put_error, \
		int , __p0, a0, \
		int , __p1, a1, \
		int , __p2, a2, \
		const char *, __p3, a3, \
		int , __p4, d0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_ASN1_UTCTIME(__p0, __p1, __p2) \
	LP3(444, ASN1_UTCTIME *, d2i_ASN1_UTCTIME, \
		ASN1_UTCTIME **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_md2() \
	LP0(2616, EVP_MD *, EVP_md2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_md5() \
	LP0(2622, EVP_MD *, EVP_md5, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS7_DIGEST_new() \
	LP0(4002, PKCS7_DIGEST *, PKCS7_DIGEST_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_CIPHER_get_version(__p0) \
	LP1(4662, char *, SSL_CIPHER_get_version, \
		SSL_CIPHER *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_des_ede() \
	LP0(2676, EVP_CIPHER *, EVP_des_ede, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_CTX_ctrl(__p0, __p1, __p2, __p3) \
	LP4(5166, long , SSL_CTX_ctrl, \
		SSL_CTX *, __p0, a0, \
		int , __p1, a1, \
		long , __p2, a2, \
		char *, __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PEM_proc_type(__p0, __p1) \
	LP2NR(3390, PEM_proc_type, \
		char *, __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CERTIFICATEPOLICIES_free(__p0) \
	LP1NR(6042, CERTIFICATEPOLICIES_free, \
		STACK *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_STORE_CTX_get_current_cert(__p0) \
	LP1(8046, X509 *, X509_STORE_CTX_get_current_cert, \
		X509_STORE_CTX *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_CTX_use_PrivateKey_ASN1(__p0, __p1, __p2, __p3) \
	LP4(5070, int , SSL_CTX_use_PrivateKey_ASN1, \
		int , __p0, a0, \
		SSL_CTX *, __p1, a1, \
		unsigned char *, __p2, a2, \
		long , __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_f_base64() \
	LP0(2586, BIO_METHOD *, BIO_f_base64, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_LOOKUP_ctrl(__p0, __p1, __p2, __p3, __p4) \
	LP5(7932, int , X509_LOOKUP_ctrl, \
		X509_LOOKUP *, __p0, a0, \
		int , __p1, a1, \
		const char *, __p2, a2, \
		long , __p3, a3, \
		char **, __p4, d0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ASN1_BIT_STRING_asn1_meth() \
	LP0(756, ASN1_METHOD *, ASN1_BIT_STRING_asn1_meth, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ERR_error_string(__p0, __p1) \
	LP2(2286, char *, ERR_error_string, \
		unsigned long , __p0, a0, \
		char *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ASN1_UTCTIME_check(__p0) \
	LP1(294, int , ASN1_UTCTIME_check, \
		ASN1_UTCTIME *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_copy(__p0, __p1) \
	LP2(1236, BIGNUM *, BN_copy, \
		BIGNUM *, __p0, a0, \
		const BIGNUM *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_lshift1(__p0, __p1) \
	LP2(1380, int , BN_lshift1, \
		BIGNUM *, __p0, a0, \
		BIGNUM *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ASN1_parse(__p0, __p1, __p2, __p3) \
	LP4(696, int , ASN1_parse, \
		BIO *, __p0, a0, \
		unsigned char *, __p1, a1, \
		long , __p2, a2, \
		int , __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_get_default_cert_file_env() \
	LP0(6684, const char *, X509_get_default_cert_file_env, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_X509_EXTENSION(__p0, __p1) \
	LP2(6918, int , i2d_X509_EXTENSION, \
		X509_EXTENSION *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PBEPARAM_new() \
	LP0(7740, PBEPARAM *, PBEPARAM_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define OBJ_bsearch(__p0, __p1, __p2, __p3, __p4) \
	LP5(3258, char *, OBJ_bsearch, \
		char *, __p0, a0, \
		char *, __p1, a1, \
		int , __p2, a2, \
		int , __p3, a3, \
		APTR , __p4, d0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define RAND_SSLeay() \
	LP0(4260, RAND_METHOD *, RAND_SSLeay, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_Netscape_RSA(__p0, __p1, __p2, __p3) \
	LP4(4428, RSA *, d2i_Netscape_RSA, \
		RSA **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		APTR , __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ERR_load_PKCS7_strings() \
	LP0NR(4080, ERR_load_PKCS7_strings, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PEM_read_bio_PrivateKey(__p0, __p1, __p2, __p3) \
	LP4(3438, EVP_PKEY *, PEM_read_bio_PrivateKey, \
		BIO *, __p0, a0, \
		EVP_PKEY **, __p1, a1, \
		pem_password_cb *, __p2, a2, \
		void *, __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_get_ext_by_OBJ(__p0, __p1, __p2) \
	LP3(7554, int , X509_get_ext_by_OBJ, \
		X509 *, __p0, a0, \
		ASN1_OBJECT *, __p1, a1, \
		int , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_rstate_string(__p0) \
	LP1(4896, char *, SSL_rstate_string, \
		SSL *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS7_SIGNER_INFO_free(__p0) \
	LP1NR(3864, PKCS7_SIGNER_INFO_free, \
		PKCS7_SIGNER_INFO *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_DecodeInit(__p0) \
	LP1NR(2538, EVP_DecodeInit, \
		EVP_ENCODE_CTX *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_SHA_IsReal() \
	LP0(126, long , EVP_SHA_IsReal, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_CIPHER_type(__p0) \
	LP1(3036, int , EVP_CIPHER_type, \
		const EVP_CIPHER *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define DIST_POINT_NAME_new() \
	LP0(6204, DIST_POINT_NAME *, DIST_POINT_NAME_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ASN1_IA5STRING_asn1_meth() \
	LP0(750, ASN1_METHOD *, ASN1_IA5STRING_asn1_meth, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CRYPTO_lock(__p0, __p1, __p2, __p3) \
	LP4NR(1860, CRYPTO_lock, \
		int , __p0, a0, \
		int , __p1, a1, \
		const char *, __p2, a2, \
		int , __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_STORE_get_by_subject(__p0, __p1, __p2, __p3) \
	LP4(7926, int , X509_STORE_get_by_subject, \
		X509_STORE_CTX *, __p0, a0, \
		int , __p1, a1, \
		X509_NAME *, __p2, a2, \
		X509_OBJECT *, __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_X509_CRL_bio(__p0, __p1) \
	LP2(6528, X509_CRL *, d2i_X509_CRL_bio, \
		BIO *, __p0, a0, \
		X509_CRL **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_DHparams(__p0, __p1) \
	LP2(2070, int , i2d_DHparams, \
		DH *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_PKCS12_BAGS(__p0, __p1) \
	LP2(3678, int , i2d_PKCS12_BAGS, \
		PKCS12_BAGS *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define RAND_write_file(__p0) \
	LP1(4290, int , RAND_write_file, \
		const char *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_ASN1_BOOLEAN(__p0, __p1) \
	LP2(252, int , i2d_ASN1_BOOLEAN, \
		int , __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_DecodeFinal(__p0, __p1, __p2) \
	LP3(2550, int , EVP_DecodeFinal, \
		EVP_ENCODE_CTX *, __p0, a0, \
		unsigned char *, __p1, a1, \
		int *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS7_ENC_CONTENT_free(__p0) \
	LP1NR(3936, PKCS7_ENC_CONTENT_free, \
		PKCS7_ENC_CONTENT *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define HMAC(__p0, __p1, __p2, __p3, __p4, __p5, __p6) \
	LP7(3090, unsigned char *, HMAC, \
		const EVP_MD *, __p0, a0, \
		const void *, __p1, a1, \
		int , __p2, a2, \
		unsigned char *, __p3, a3, \
		int , __p4, d0, \
		unsigned char *, __p5, d1, \
		unsigned int *, __p6, d2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_COMP_add_compression_method(__p0, __p1) \
	LP2(5592, int , SSL_COMP_add_compression_method, \
		int , __p0, a0, \
		COMP_METHOD *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_rc2_cfb() \
	LP0(2808, EVP_CIPHER *, EVP_rc2_cfb, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_options() \
	LP0(1176, char *, BN_options, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_EXTENSION_new() \
	LP0(6906, X509_EXTENSION *, X509_EXTENSION_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define DH_generate_key(__p0) \
	LP1(2052, int , DH_generate_key, \
		DH *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_CTX_check_private_key(__p0) \
	LP1(5094, int , SSL_CTX_check_private_key, \
		SSL_CTX *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_NAME_new() \
	LP0(6954, X509_NAME *, X509_NAME_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS7_DIGEST_free(__p0) \
	LP1NR(4008, PKCS7_DIGEST_free, \
		PKCS7_DIGEST *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_want(__p0) \
	LP1(4632, int , SSL_want, \
		SSL *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_CRL_DIST_POINTS(__p0, __p1) \
	LP2(6150, int , i2d_CRL_DIST_POINTS, \
		STACK *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CRYPTO_mem_ctrl(__p0) \
	LP1(1848, int , CRYPTO_mem_ctrl, \
		int , __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SXNET_get_id_asc(__p0, __p1) \
	LP2(5892, ASN1_OCTET_STRING *, SXNET_get_id_asc, \
		SXNET *, __p0, a0, \
		char *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_cast5_cfb() \
	LP0(2856, EVP_CIPHER *, EVP_cast5_cfb, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_STORE_CTX_get_error(__p0) \
	LP1(8028, int , X509_STORE_CTX_get_error, \
		X509_STORE_CTX *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_CipherInit(__p0, __p1, __p2, __p3, __p4) \
	LP5NR(2460, EVP_CipherInit, \
		EVP_CIPHER_CTX *, __p0, a0, \
		const EVP_CIPHER *, __p1, a1, \
		unsigned char *, __p2, a2, \
		unsigned char *, __p3, a3, \
		int , __p4, d0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_rc2_ecb() \
	LP0(2784, EVP_CIPHER *, EVP_rc2_ecb, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define OBJ_new_nid(__p0) \
	LP1(3270, int , OBJ_new_nid, \
		int , __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_set_cipher(__p0, __p1, __p2, __p3, __p4) \
	LP5NR(2604, BIO_set_cipher, \
		BIO *, __p0, a0, \
		const EVP_CIPHER *, __p1, a1, \
		unsigned char *, __p2, a2, \
		unsigned char *, __p3, a3, \
		int , __p4, d0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_RSAPrivateKey_bio(__p0, __p1) \
	LP2(6552, RSA *, d2i_RSAPrivateKey_bio, \
		BIO *, __p0, a0, \
		RSA **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ASN1_sign(__p0, __p1, __p2, __p3, __p4, __p5, __p6) \
	LP7(7230, int , ASN1_sign, \
		APTR , __p0, a0, \
		X509_ALGOR *, __p1, a1, \
		X509_ALGOR *, __p2, a2, \
		ASN1_BIT_STRING *, __p3, a3, \
		char *, __p4, d0, \
		EVP_PKEY *, __p5, d1, \
		const EVP_MD *, __p6, d2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_NETSCAPE_SPKI(__p0, __p1) \
	LP2(7140, int , i2d_NETSCAPE_SPKI, \
		NETSCAPE_SPKI *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_new_bio_pair(__p0, __p1, __p2, __p3) \
	LP4(8100, int , BIO_new_bio_pair, \
		BIO **, __p0, a0, \
		size_t , __p1, a1, \
		BIO **, __p2, a2, \
		size_t , __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_NAME_ENTRY_set_object(__p0, __p1) \
	LP2(7476, int , X509_NAME_ENTRY_set_object, \
		X509_NAME_ENTRY *, __p0, a0, \
		ASN1_OBJECT *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_REVOKED_get_ext_by_OBJ(__p0, __p1, __p2) \
	LP3(7638, int , X509_REVOKED_get_ext_by_OBJ, \
		X509_REVOKED *, __p0, a0, \
		ASN1_OBJECT *, __p1, a1, \
		int , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define NETSCAPE_SPKI_free(__p0) \
	LP1NR(7134, NETSCAPE_SPKI_free, \
		NETSCAPE_SPKI *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS12_key_gen_asc(__p0, __p1, __p2, __p3, __p4, __p5, __p6, __p7, __p8) \
	LP9(3630, int , PKCS12_key_gen_asc, \
		const char *, __p0, a0, \
		int , __p1, a1, \
		unsigned char *, __p2, a2, \
		int , __p3, a3, \
		int , __p4, d0, \
		int , __p5, d1, \
		int , __p6, d2, \
		unsigned char *, __p7, d3, \
		const EVP_MD *, __p8, d4, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_REQ_get_pubkey(__p0) \
	LP1(7326, EVP_PKEY *, X509_REQ_get_pubkey, \
		X509_REQ *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PBKDF2PARAM_free(__p0) \
	LP1NR(7782, PBKDF2PARAM_free, \
		PBKDF2PARAM *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_new_fd(__p0, __p1) \
	LP2(1134, BIO *, BIO_new_fd, \
		int , __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_get_ext_count(__p0) \
	LP1(7542, int , X509_get_ext_count, \
		X509 *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_usub(__p0, __p1, __p2) \
	LP3(1272, int , BN_usub, \
		BIGNUM *, __p0, a0, \
		const BIGNUM *, __p1, a1, \
		const BIGNUM *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKEY_USAGE_PERIOD_free(__p0) \
	LP1NR(5952, PKEY_USAGE_PERIOD_free, \
		PKEY_USAGE_PERIOD *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_PUBKEY_set(__p0, __p1) \
	LP2(6786, int , X509_PUBKEY_set, \
		X509_PUBKEY **, __p0, a0, \
		EVP_PKEY *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_free(__p0) \
	LP1NR(1362, BN_free, \
		BIGNUM *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_cast5_ecb() \
	LP0(2844, EVP_CIPHER *, EVP_cast5_ecb, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_get_privatekey(__p0) \
	LP1(5406, struct evp_pkey_st *, SSL_get_privatekey, \
		SSL *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define lh_free(__p0) \
	LP1NR(3102, lh_free, \
		LHASH *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PEM_SignUpdate(__p0, __p1, __p2) \
	LP3NR(3372, PEM_SignUpdate, \
		EVP_MD_CTX *, __p0, a0, \
		unsigned char *, __p1, a1, \
		unsigned int , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_get_verify_depth(__p0) \
	LP1(4758, int , SSL_get_verify_depth, \
		SSL *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ext_ku_new() \
	LP0(6024, STACK *, ext_ku_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_DSAPrivateKey(__p0, __p1, __p2) \
	LP3(2172, DSA *, d2i_DSAPrivateKey, \
		DSA **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_CIPHER_get_bits(__p0, __p1) \
	LP2(4656, int , SSL_CIPHER_get_bits, \
		SSL_CIPHER *, __p0, a0, \
		int *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define string_to_hex(__p0, __p1) \
	LP2(6426, unsigned char *, string_to_hex, \
		char *, __p0, a0, \
		long *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_ASN1_INTEGER(__p0, __p1, __p2) \
	LP3(270, ASN1_INTEGER *, d2i_ASN1_INTEGER, \
		ASN1_INTEGER **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define sk_unshift(__p0, __p1) \
	LP2(5664, int , sk_unshift, \
		STACK *, __p0, a0, \
		char *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_mod_mul_reciprocal(__p0, __p1, __p2, __p3, __p4) \
	LP5(1686, int , BN_mod_mul_reciprocal, \
		BIGNUM *, __p0, a0, \
		BIGNUM *, __p1, a1, \
		BIGNUM *, __p2, a2, \
		BN_RECP_CTX *, __p3, a3, \
		BN_CTX *, __p4, d0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_ASN1_TIME(__p0, __p1) \
	LP2(462, int , i2d_ASN1_TIME, \
		ASN1_TIME *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_X509_CRL_INFO(__p0, __p1, __p2) \
	LP3(7074, X509_CRL_INFO *, d2i_X509_CRL_INFO, \
		X509_CRL_INFO **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_CTX_set_ex_data(__p0, __p1, __p2) \
	LP3(5544, int , SSL_CTX_set_ex_data, \
		SSL_CTX *, __p0, a0, \
		int , __p1, a1, \
		void *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_PKCS7_DIGEST(__p0, __p1, __p2) \
	LP3(4020, PKCS7_DIGEST *, d2i_PKCS7_DIGEST, \
		PKCS7_DIGEST **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define NOTICEREF_new() \
	LP0(6132, NOTICEREF *, NOTICEREF_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define USERNOTICE_new() \
	LP0(6108, USERNOTICE *, USERNOTICE_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_new_connect(__p0) \
	LP1(1140, BIO *, BIO_new_connect, \
		char *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_REQ_verify(__p0, __p1) \
	LP2(6462, int , X509_REQ_verify, \
		X509_REQ *, __p0, a0, \
		EVP_PKEY *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_sock_error(__p0) \
	LP1(1062, int , BIO_sock_error, \
		int , __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_free(__p0) \
	LP1NR(7014, X509_free, \
		X509 *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_set_bit(__p0, __p1) \
	LP2(1476, int , BN_set_bit, \
		BIGNUM *, __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_PKCS12_SAFEBAG(__p0, __p1, __p2) \
	LP3(3762, PKCS12_SAFEBAG *, d2i_PKCS12_SAFEBAG, \
		PKCS12_SAFEBAG **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_OBJECT_free_contents(__p0) \
	LP1NR(7866, X509_OBJECT_free_contents, \
		X509_OBJECT *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_MONT_CTX_new() \
	LP0(1578, BN_MONT_CTX *, BN_MONT_CTX_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define RAND_get_rand_method() \
	LP0(4254, RAND_METHOD *, RAND_get_rand_method, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_renegotiate(__p0) \
	LP1(5274, int , SSL_renegotiate, \
		SSL *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_PKCS12_bio(__p0, __p1) \
	LP2(3804, PKCS12 *, d2i_PKCS12_bio, \
		BIO *, __p0, a0, \
		PKCS12 **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_POLICYQUALINFO(__p0, __p1) \
	LP2(6078, int , i2d_POLICYQUALINFO, \
		POLICYQUALINFO *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define s2i_ASN1_INTEGER(__p0, __p1) \
	LP2(6342, ASN1_INTEGER *, s2i_ASN1_INTEGER, \
		X509V3_EXT_METHOD *, __p0, a0, \
		char *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_SESSION_set_time(__p0, __p1) \
	LP2(4920, long , SSL_SESSION_set_time, \
		SSL_SESSION *, __p0, a0, \
		long , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_DecryptFinal(__p0, __p1, __p2) \
	LP3(2454, int , EVP_DecryptFinal, \
		EVP_CIPHER_CTX *, __p0, a0, \
		unsigned char *, __p1, a1, \
		int *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CRYPTO_set_locked_mem_functions(__p0, __p1) \
	LP2NR(1932, CRYPTO_set_locked_mem_functions, \
		APTR , __p0, a0, \
		APTR , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_DigestFinal(__p0, __p1, __p2) \
	LP3NR(2394, EVP_DigestFinal, \
		EVP_MD_CTX *, __p0, a0, \
		unsigned char *, __p1, a1, \
		unsigned int *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ERR_set_error_data(__p0, __p1) \
	LP2NR(2238, ERR_set_error_data, \
		char *, __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_new_fp_amiga(__p0, __p1) \
	LP2(852, BIO *, BIO_new_fp_amiga, \
		BPTR , __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_BLINDING_free(__p0) \
	LP1NR(1626, BN_BLINDING_free, \
		BN_BLINDING *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_lshift(__p0, __p1, __p2) \
	LP3(1374, int , BN_lshift, \
		BIGNUM *, __p0, a0, \
		const BIGNUM *, __p1, a1, \
		int , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define lh_retrieve(__p0, __p1) \
	LP2(3120, char *, lh_retrieve, \
		LHASH *, __p0, a0, \
		char *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define TXT_DB_insert(__p0, __p1) \
	LP2(5760, int , TXT_DB_insert, \
		TXT_DB *, __p0, a0, \
		char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_socket_ioctl(__p0, __p1, __p2) \
	LP3(1068, int , BIO_socket_ioctl, \
		int , __p0, a0, \
		long , __p1, a1, \
		unsigned long *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_get_default_cert_area() \
	LP0(6660, const char *, X509_get_default_cert_area, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_CTX_use_certificate_ASN1(__p0, __p1, __p2) \
	LP3(5082, int , SSL_CTX_use_certificate_ASN1, \
		SSL_CTX *, __p0, a0, \
		int , __p1, a1, \
		unsigned char *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PEM_write_bio_X509_REQ(__p0, __p1) \
	LP2(3474, int , PEM_write_bio_X509_REQ, \
		BIO *, __p0, a0, \
		X509_REQ *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSLeay_add_all_digests() \
	LP0NR(2904, SSLeay_add_all_digests, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define OBJ_nid2sn(__p0) \
	LP1(3216, const char *, OBJ_nid2sn, \
		int , __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_ASN1_GENERALIZEDTIME(__p0, __p1, __p2) \
	LP3(456, ASN1_GENERALIZEDTIME *, d2i_ASN1_GENERALIZEDTIME, \
		ASN1_GENERALIZEDTIME **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define RAND_cleanup() \
	LP0NR(4266, RAND_cleanup, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509V3_EXT_conf_nid(__p0, __p1, __p2, __p3) \
	LP4(6234, X509_EXTENSION *, X509V3_EXT_conf_nid, \
		LHASH *, __p0, a0, \
		X509V3_CTX *, __p1, a1, \
		int , __p2, a2, \
		char *, __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_GENERAL_NAMES(__p0, __p1, __p2) \
	LP3(5970, STACK *, d2i_GENERAL_NAMES, \
		STACK **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_RC2_IsReal() \
	LP0(102, long , EVP_RC2_IsReal, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define DH_compute_key(__p0, __p1, __p2) \
	LP3(2058, int , DH_compute_key, \
		unsigned char *, __p0, a0, \
		BIGNUM *, __p1, a1, \
		DH *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ERR_get_error() \
	LP0(2244, unsigned long , ERR_get_error, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_new_accept(__p0) \
	LP1(1146, BIO *, BIO_new_accept, \
		char *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_PKCS12(__p0, __p1, __p2) \
	LP3(3708, PKCS12 *, d2i_PKCS12, \
		PKCS12 **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_CAST_IsReal() \
	LP0(66, long , EVP_CAST_IsReal, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_pending(__p0) \
	LP1(4698, int , SSL_pending, \
		SSL *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509V3_EXT_add(__p0) \
	LP1(6360, int , X509V3_EXT_add, \
		X509V3_EXT_METHOD *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_PKCS8_PRIV_KEY_INFO(__p0, __p1) \
	LP2(7812, int , i2d_PKCS8_PRIV_KEY_INFO, \
		PKCS8_PRIV_KEY_INFO *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2a_ASN1_INTEGER(__p0, __p1) \
	LP2(492, int , i2a_ASN1_INTEGER, \
		BIO *, __p0, a0, \
		ASN1_INTEGER *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_find_type(__p0, __p1) \
	LP2(936, BIO *, BIO_find_type, \
		BIO *, __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_copy_session_id(__p0, __p1) \
	LP2NR(4938, SSL_copy_session_id, \
		SSL *, __p0, a0, \
		SSL *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_gcd(__p0, __p1, __p2, __p3) \
	LP4(1512, int , BN_gcd, \
		BIGNUM *, __p0, a0, \
		BIGNUM *, __p1, a1, \
		BIGNUM *, __p2, a2, \
		BN_CTX *, __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CRYPTO_get_new_lockid(__p0) \
	LP1(1854, int , CRYPTO_get_new_lockid, \
		char *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_CTX_load_verify_locations(__p0, __p1, __p2) \
	LP3(5460, int , SSL_CTX_load_verify_locations, \
		SSL_CTX *, __p0, a0, \
		const char *, __p1, a1, \
		const char *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_STORE_free(__p0) \
	LP1NR(7878, X509_STORE_free, \
		X509_STORE *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_get_subject_name(__p0) \
	LP1(7272, X509_NAME *, X509_get_subject_name, \
		X509 *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_ASN1_OCTET_STRING(__p0, __p1) \
	LP2(330, int , i2d_ASN1_OCTET_STRING, \
		ASN1_OCTET_STRING *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ERR_get_state() \
	LP0(2352, ERR_STATE *, ERR_get_state, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ASN1_OBJECT_free(__p0) \
	LP1NR(174, ASN1_OBJECT_free, \
		ASN1_OBJECT *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_do_handshake(__p0) \
	LP1(5268, int , SSL_do_handshake, \
		SSL *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CRYPTO_num_locks() \
	LP0(8106, int , CRYPTO_num_locks, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_REQ_sign(__p0, __p1, __p2) \
	LP3(6486, int , X509_REQ_sign, \
		X509_REQ *, __p0, a0, \
		EVP_PKEY *, __p1, a1, \
		const EVP_MD *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ASN1_ENUMERATED_to_BN(__p0, __p1) \
	LP2(594, BIGNUM *, ASN1_ENUMERATED_to_BN, \
		ASN1_ENUMERATED *, __p0, a0, \
		BIGNUM *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define RSA_new() \
	LP0(4308, RSA *, RSA_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_CRL_delete_ext(__p0, __p1) \
	LP2(7614, X509_EXTENSION *, X509_CRL_delete_ext, \
		X509_CRL *, __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_mod_exp(__p0, __p1, __p2, __p3, __p4) \
	LP5(1392, int , BN_mod_exp, \
		BIGNUM *, __p0, a0, \
		BIGNUM *, __p1, a1, \
		const BIGNUM *, __p2, a2, \
		const BIGNUM *, __p3, a3, \
		BN_CTX *, __p4, d0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_GENERAL_NAME(__p0, __p1, __p2) \
	LP3(5802, GENERAL_NAME *, d2i_GENERAL_NAME, \
		GENERAL_NAME **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_CTX_set_ssl_version(__p0, __p1) \
	LP2(5184, int , SSL_CTX_set_ssl_version, \
		SSL_CTX *, __p0, a0, \
		SSL_METHOD *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_NAME_print(__p0, __p1, __p2) \
	LP3(7392, int , X509_NAME_print, \
		BIO *, __p0, a0, \
		X509_NAME *, __p1, a1, \
		int , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS5_v2_PBE_keyivgen(__p0, __p1, __p2, __p3, __p4, __p5, __p6) \
	LP7(8130, int , PKCS5_v2_PBE_keyivgen, \
		EVP_CIPHER_CTX *, __p0, a0, \
		const char *, __p1, a1, \
		int , __p2, a2, \
		ASN1_TYPE *, __p3, a3, \
		EVP_CIPHER *, __p4, d0, \
		EVP_MD *, __p5, d1, \
		int , __p6, d2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_set_tmp_dh_callback(__p0, __p1) \
	LP2NR(5586, SSL_set_tmp_dh_callback, \
		SSL *, __p0, a0, \
		APTR , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_get_digestbyname(__p0) \
	LP1(2928, const EVP_MD *, EVP_get_digestbyname, \
		const char *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define AUTHORITY_KEYID_new() \
	LP0(5922, AUTHORITY_KEYID *, AUTHORITY_KEYID_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_STORE_load_locations(__p0, __p1, __p2) \
	LP3(7998, int , X509_STORE_load_locations, \
		X509_STORE *, __p0, a0, \
		const char *, __p1, a1, \
		const char *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_set_accept_state(__p0) \
	LP1NR(5364, SSL_set_accept_state, \
		SSL *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define OBJ_NAME_init() \
	LP0(3162, int , OBJ_NAME_init, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PEM_SealFinal(__p0, __p1, __p2, __p3, __p4, __p5) \
	LP6(3360, int , PEM_SealFinal, \
		PEM_ENCODE_SEAL_CTX *, __p0, a0, \
		unsigned char *, __p1, a1, \
		int *, __p2, a2, \
		unsigned char *, __p3, a3, \
		int *, __p4, d0, \
		EVP_PKEY *, __p5, d1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PEM_read_bio_DSAparams(__p0, __p1, __p2, __p3) \
	LP4(3462, DSA *, PEM_read_bio_DSAparams, \
		BIO *, __p0, a0, \
		DSA **, __p1, a1, \
		pem_password_cb *, __p2, a2, \
		void *, __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_add_digest(__p0) \
	LP1(2916, int , EVP_add_digest, \
		EVP_MD *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_read(__p0, __p1, __p2) \
	LP3(876, int , BIO_read, \
		BIO *, __p0, a0, \
		void *, __p1, a1, \
		int , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509v3_get_ext_count(__p0) \
	LP1(7500, int , X509v3_get_ext_count, \
		const STACK *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_STORE_CTX_cleanup(__p0) \
	LP1NR(7890, X509_STORE_CTX_cleanup, \
		X509_STORE_CTX *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PEM_ASN1_write_bio(__p0, __p1, __p2, __p3, __p4, __p5, __p6, __p7, __p8) \
	LP9(3330, int , PEM_ASN1_write_bio, \
		APTR , __p0, a0, \
		const char *, __p1, a1, \
		BIO *, __p2, a2, \
		char *, __p3, a3, \
		const EVP_CIPHER *, __p4, d0, \
		unsigned char *, __p5, d1, \
		int , __p6, d2, \
		pem_password_cb *, __p7, d3, \
		void *, __p8, d4, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_CTX_get_cert_store(__p0) \
	LP1(4620, X509_STORE *, SSL_CTX_get_cert_store, \
		SSL_CTX *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_DISPLAYTEXT(__p0, __p1) \
	LP2(408, int , i2d_DISPLAYTEXT, \
		ASN1_STRING *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_get_port(__p0, __p1) \
	LP2(1080, int , BIO_get_port, \
		const char *, __p0, a0, \
		unsigned short *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_DigestInit(__p0, __p1) \
	LP2NR(2382, EVP_DigestInit, \
		EVP_MD_CTX *, __p0, a0, \
		const EVP_MD *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_find_by_subject(__p0, __p1) \
	LP2(7728, X509 *, X509_find_by_subject, \
		STACK *, __p0, a0, \
		X509_NAME *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_SIG_new() \
	LP0(6804, X509_SIG *, X509_SIG_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_accept(__p0) \
	LP1(5130, int , SSL_accept, \
		SSL *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CERTIFICATEPOLICIES_new() \
	LP0(6036, STACK *, CERTIFICATEPOLICIES_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_PBEPARAM(__p0, __p1, __p2) \
	LP3(7746, PBEPARAM *, d2i_PBEPARAM, \
		PBEPARAM **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CONF_free(__p0) \
	LP1NR(1788, CONF_free, \
		LHASH *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define RSA_free(__p0) \
	LP1NR(4356, RSA_free, \
		RSA *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_DecryptUpdate(__p0, __p1, __p2, __p3, __p4) \
	LP5NR(2448, EVP_DecryptUpdate, \
		EVP_CIPHER_CTX *, __p0, a0, \
		unsigned char *, __p1, a1, \
		int *, __p2, a2, \
		unsigned char *, __p3, a3, \
		int , __p4, d0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_STORE_new() \
	LP0(7872, X509_STORE *, X509_STORE_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_value_one() \
	LP0(1170, BIGNUM *, BN_value_one, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_ASN1_UTF8STRING(__p0, __p1, __p2) \
	LP3(360, ASN1_UTF8STRING *, d2i_ASN1_UTF8STRING, \
		ASN1_UTF8STRING **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_PBE_CipherInit(__p0, __p1, __p2, __p3, __p4, __p5) \
	LP6(8142, int , EVP_PBE_CipherInit, \
		ASN1_OBJECT *, __p0, a0, \
		const char *, __p1, a1, \
		int , __p2, a2, \
		ASN1_TYPE *, __p3, a3, \
		EVP_CIPHER_CTX *, __p4, d0, \
		int , __p5, d1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_cmp(__p0, __p1) \
	LP2(1356, int , BN_cmp, \
		const BIGNUM *, __p0, a0, \
		const BIGNUM *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CRL_DIST_POINTS_free(__p0) \
	LP1NR(6162, CRL_DIST_POINTS_free, \
		STACK *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ERR_load_PEM_strings() \
	LP0NR(3384, ERR_load_PEM_strings, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define bn_mul_add_words(__p0, __p1, __p2, __p3) \
	LP4(1542, BN_ULONG , bn_mul_add_words, \
		BN_ULONG *, __p0, a0, \
		BN_ULONG *, __p1, a1, \
		int , __p2, a2, \
		BN_ULONG , __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS7_get_signed_attribute(__p0, __p1) \
	LP2(4230, ASN1_TYPE *, PKCS7_get_signed_attribute, \
		PKCS7_SIGNER_INFO *, __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define s2i_ASN1_OCTET_STRING(__p0, __p1, __p2) \
	LP3(6000, ASN1_OCTET_STRING *, s2i_ASN1_OCTET_STRING, \
		X509V3_EXT_METHOD *, __p0, a0, \
		X509V3_CTX *, __p1, a1, \
		char *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_PKCS7_ENC_CONTENT(__p0, __p1, __p2) \
	LP3(3948, PKCS7_ENC_CONTENT *, d2i_PKCS7_ENC_CONTENT, \
		PKCS7_ENC_CONTENT **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_rc5_32_12_16_ofb() \
	LP0(2886, EVP_CIPHER *, EVP_rc5_32_12_16_ofb, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_version(__p0) \
	LP1(5448, int , SSL_version, \
		SSL *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_DIST_POINT(__p0, __p1) \
	LP2(6174, int , i2d_DIST_POINT, \
		DIST_POINT *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_get_read_ahead(__p0) \
	LP1(4692, int , SSL_get_read_ahead, \
		SSL *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define lh_doall(__p0, __p1) \
	LP2NR(3126, lh_doall, \
		LHASH *, __p0, a0, \
		APTR , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_CTX_set_cipher_list(__p0, __p1) \
	LP2(4590, int , SSL_CTX_set_cipher_list, \
		SSL_CTX *, __p0, a0, \
		char *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define RSA_get_ex_new_index(__p0, __p1, __p2, __p3, __p4) \
	LP5(4536, int , RSA_get_ex_new_index, \
		long , __p0, a0, \
		char *, __p1, a1, \
		APTR , __p2, a2, \
		APTR , __p3, a3, \
		APTR , __p4, d0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_get_default_cert_file() \
	LP0(6672, const char *, X509_get_default_cert_file, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define bn_sub_words(__p0, __p1, __p2, __p3) \
	LP4(1572, BN_ULONG , bn_sub_words, \
		BN_ULONG *, __p0, a0, \
		BN_ULONG *, __p1, a1, \
		BN_ULONG *, __p2, a2, \
		int , __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_CTX_set_client_CA_list(__p0, __p1) \
	LP2NR(5328, SSL_CTX_set_client_CA_list, \
		SSL_CTX *, __p0, a0, \
		STACK *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_get_ext_by_critical(__p0, __p1, __p2) \
	LP3(7560, int , X509_get_ext_by_critical, \
		X509 *, __p0, a0, \
		int , __p1, a1, \
		int , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_PKCS7_ENCRYPT(__p0, __p1, __p2) \
	LP3(4044, PKCS7_ENCRYPT *, d2i_PKCS7_ENCRYPT, \
		PKCS7_ENCRYPT **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PEM_read_bio_X509(__p0, __p1, __p2, __p3) \
	LP4(3402, X509 *, PEM_read_bio_X509, \
		BIO *, __p0, a0, \
		X509 **, __p1, a1, \
		pem_password_cb *, __p2, a2, \
		void *, __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_get_client_CA_list(__p0) \
	LP1(5334, STACK *, SSL_get_client_CA_list, \
		SSL *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_X509_CINF(__p0, __p1) \
	LP2(6996, int , i2d_X509_CINF, \
		X509_CINF *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS7_set_type(__p0, __p1) \
	LP2(4092, int , PKCS7_set_type, \
		PKCS7 *, __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_set_word(__p0, __p1) \
	LP2(1344, int , BN_set_word, \
		BIGNUM *, __p0, a0, \
		BN_ULONG , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ASN1_seq_unpack(__p0, __p1, __p2, __p3) \
	LP4(786, STACK *, ASN1_seq_unpack, \
		unsigned char *, __p0, a0, \
		int , __p1, a1, \
		APTR , __p2, a2, \
		APTR , __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_DSAPublicKey(__p0, __p1, __p2) \
	LP3(2166, DSA *, d2i_DSAPublicKey, \
		DSA **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define RSA_verify_ASN1_OCTET_STRING(__p0, __p1, __p2, __p3, __p4, __p5) \
	LP6(4458, int , RSA_verify_ASN1_OCTET_STRING, \
		int , __p0, a0, \
		unsigned char *, __p1, a1, \
		unsigned int , __p2, a2, \
		unsigned char *, __p3, a3, \
		unsigned int , __p4, d0, \
		RSA *, __p5, d1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS7_SIGNER_INFO_new() \
	LP0(3858, PKCS7_SIGNER_INFO *, PKCS7_SIGNER_INFO_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define sk_delete_ptr(__p0, __p1) \
	LP2(5646, char *, sk_delete_ptr, \
		STACK *, __p0, a0, \
		char *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_ASN1_OBJECT(__p0, __p1) \
	LP2(180, int , i2d_ASN1_OBJECT, \
		ASN1_OBJECT *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ASN1_GENERALIZEDTIME_check(__p0) \
	LP1(312, int , ASN1_GENERALIZEDTIME_check, \
		ASN1_GENERALIZEDTIME *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define OBJ_sn2nid(__p0) \
	LP1(3246, int , OBJ_sn2nid, \
		const char *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_s_file() \
	LP0(840, BIO_METHOD *, BIO_s_file, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_ctrl(__p0, __p1, __p2, __p3) \
	LP4(900, long , BIO_ctrl, \
		BIO *, __p0, a0, \
		int , __p1, a1, \
		long , __p2, a2, \
		void *, __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS8_add_keyusage(__p0, __p1) \
	LP2(3588, int , PKCS8_add_keyusage, \
		PKCS8_PRIV_KEY_INFO *, __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS7_ENVELOPE_new() \
	LP0(3954, PKCS7_ENVELOPE *, PKCS7_ENVELOPE_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_dump(__p0, __p1, __p2) \
	LP3(1050, int , BIO_dump, \
		BIO *, __p0, a0, \
		const char *, __p1, a1, \
		int , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_SXNET(__p0, __p1) \
	LP2(5826, int , i2d_SXNET, \
		SXNET *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_to_ASN1_INTEGER(__p0, __p1) \
	LP2(564, ASN1_INTEGER *, BN_to_ASN1_INTEGER, \
		BIGNUM *, __p0, a0, \
		ASN1_INTEGER *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509V3_parse_list(__p0) \
	LP1(6402, STACK *, X509V3_parse_list, \
		char *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_CTX_get_ex_data(__p0, __p1) \
	LP2(5550, void *, SSL_CTX_get_ex_data, \
		SSL_CTX *, __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ASN1_GENERALIZEDTIME_set(__p0, __p1) \
	LP2(318, ASN1_GENERALIZEDTIME *, ASN1_GENERALIZEDTIME_set, \
		ASN1_GENERALIZEDTIME *, __p0, a0, \
		time_t , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_peek(__p0, __p1, __p2) \
	LP3(5148, int , SSL_peek, \
		SSL *, __p0, a0, \
		char *, __p1, a1, \
		int , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2t_ASN1_OBJECT(__p0, __p1, __p2) \
	LP3(534, int , i2t_ASN1_OBJECT, \
		char *, __p0, a0, \
		int , __p1, a1, \
		ASN1_OBJECT *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ERR_get_error_line(__p0, __p1) \
	LP2(2250, unsigned long , ERR_get_error_line, \
		const char **, __p0, a0, \
		int *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_set_rfd(__p0, __p1) \
	LP2(4710, int , SSL_set_rfd, \
		SSL *, __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define OBJ_add_object(__p0) \
	LP1(3276, int , OBJ_add_object, \
		ASN1_OBJECT *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_s_accept() \
	LP0(984, BIO_METHOD *, BIO_s_accept, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define HMAC_Update(__p0, __p1, __p2) \
	LP3NR(3072, HMAC_Update, \
		HMAC_CTX *, __p0, a0, \
		unsigned char *, __p1, a1, \
		int , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_idea_IsReal() \
	LP0(78, long , EVP_idea_IsReal, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PEM_do_header(__p0, __p1, __p2, __p3, __p4) \
	LP5(3306, int , PEM_do_header, \
		EVP_CIPHER_INFO *, __p0, a0, \
		unsigned char *, __p1, a1, \
		long *, __p2, a2, \
		pem_password_cb *, __p3, a3, \
		void *, __p4, d0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_CTX_get_quiet_shutdown(__p0) \
	LP1(5418, int , SSL_CTX_get_quiet_shutdown, \
		SSL_CTX *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ERR_load_ASN1_strings() \
	LP0NR(732, ERR_load_ASN1_strings, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_mod_exp_simple(__p0, __p1, __p2, __p3, __p4) \
	LP5(1410, int , BN_mod_exp_simple, \
		BIGNUM *, __p0, a0, \
		BIGNUM *, __p1, a1, \
		BIGNUM *, __p2, a2, \
		BIGNUM *, __p3, a3, \
		BN_CTX *, __p4, d0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define sk_sort(__p0) \
	LP1NR(8196, sk_sort, \
		STACK *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_RSAPublicKey_bio(__p0, __p1) \
	LP2(6564, RSA *, d2i_RSAPublicKey_bio, \
		BIO *, __p0, a0, \
		RSA **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PEM_write_bio(__p0, __p1, __p2, __p3, __p4) \
	LP5(3318, int , PEM_write_bio, \
		BIO *, __p0, a0, \
		const char *, __p1, a1, \
		char *, __p2, a2, \
		unsigned char *, __p3, a3, \
		long , __p4, d0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS7_dataDecode(__p0, __p1, __p2, __p3) \
	LP4(4152, BIO *, PKCS7_dataDecode, \
		PKCS7 *, __p0, a0, \
		EVP_PKEY *, __p1, a1, \
		BIO *, __p2, a2, \
		X509 *, __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_ASN1_IA5STRING(__p0, __p1, __p2) \
	LP3(432, ASN1_IA5STRING *, d2i_ASN1_IA5STRING, \
		ASN1_IA5STRING **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS7_ENVELOPE_free(__p0) \
	LP1NR(3960, PKCS7_ENVELOPE_free, \
		PKCS7_ENVELOPE *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define OBJ_NAME_remove(__p0, __p1) \
	LP2(3186, int , OBJ_NAME_remove, \
		const char *, __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_mod_word(__p0, __p1) \
	LP2(1314, BN_ULONG , BN_mod_word, \
		BIGNUM *, __p0, a0, \
		BN_ULONG , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_DSAPrivateKey_bio(__p0, __p1) \
	LP2(6582, int , i2d_DSAPrivateKey_bio, \
		BIO *, __p0, a0, \
		DSA *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_SSL_SESSION(__p0, __p1) \
	LP2(4974, int , i2d_SSL_SESSION, \
		SSL_SESSION *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSLeay_version(__p0) \
	LP1(1800, const char *, SSLeay_version, \
		int , __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_CTX_flush_sessions(__p0, __p1) \
	LP2NR(4644, SSL_CTX_flush_sessions, \
		SSL_CTX *, __p0, a0, \
		long , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS12_add_localkeyid(__p0, __p1, __p2) \
	LP3(3570, int , PKCS12_add_localkeyid, \
		PKCS12_SAFEBAG *, __p0, a0, \
		unsigned char *, __p1, a1, \
		int , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ERR_lib_error_string(__p0) \
	LP1(2292, const char *, ERR_lib_error_string, \
		unsigned long , __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_NAME_get_index_by_NID(__p0, __p1, __p2) \
	LP3(7434, int , X509_NAME_get_index_by_NID, \
		X509_NAME *, __p0, a0, \
		int , __p1, a1, \
		int , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ERR_load_BN_strings() \
	LP0NR(1536, ERR_load_BN_strings, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_load_cert_file(__p0, __p1, __p2) \
	LP3(7938, int , X509_load_cert_file, \
		X509_LOOKUP *, __p0, a0, \
		const char *, __p1, a1, \
		int , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ASN1_put_object(__p0, __p1, __p2, __p3, __p4) \
	LP5NR(642, ASN1_put_object, \
		unsigned char **, __p0, a0, \
		int , __p1, a1, \
		int , __p2, a2, \
		int , __p3, a3, \
		int , __p4, d0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define RAND_load_file(__p0, __p1) \
	LP2(4284, int , RAND_load_file, \
		const char *, __p0, a0, \
		long , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509V3_conf_free(__p0) \
	LP1NR(6228, X509V3_conf_free, \
		CONF_VALUE *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_OpenFinal(__p0, __p1, __p2) \
	LP3(2496, int , EVP_OpenFinal, \
		EVP_CIPHER_CTX *, __p0, a0, \
		unsigned char *, __p1, a1, \
		int *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_SESSION_set_timeout(__p0, __p1) \
	LP2(4932, long , SSL_SESSION_set_timeout, \
		SSL_SESSION *, __p0, a0, \
		long , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_bf_ofb() \
	LP0(2838, EVP_CIPHER *, EVP_bf_ofb, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define sk_set(__p0, __p1, __p2) \
	LP3(5610, char *, sk_set, \
		STACK *, __p0, a0, \
		int , __p1, a1, \
		char *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ASN1_INTEGER_set(__p0, __p1) \
	LP2(552, int , ASN1_INTEGER_set, \
		ASN1_INTEGER *, __p0, a0, \
		long , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_num_bits_word(__p0) \
	LP1(1212, int , BN_num_bits_word, \
		BN_ULONG , __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSLv23_client_method() \
	LP0(5238, SSL_METHOD *, SSLv23_client_method, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_X509_PKEY(__p0, __p1) \
	LP2(7116, int , i2d_X509_PKEY, \
		X509_PKEY *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_mod_mul_montgomery(__p0, __p1, __p2, __p3, __p4) \
	LP5(1590, int , BN_mod_mul_montgomery, \
		BIGNUM *, __p0, a0, \
		BIGNUM *, __p1, a1, \
		BIGNUM *, __p2, a2, \
		BN_MONT_CTX *, __p3, a3, \
		BN_CTX *, __p4, d0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_uadd(__p0, __p1, __p2) \
	LP3(1278, int , BN_uadd, \
		BIGNUM *, __p0, a0, \
		const BIGNUM *, __p1, a1, \
		const BIGNUM *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_read(__p0, __p1, __p2) \
	LP3(5142, int , SSL_read, \
		SSL *, __p0, a0, \
		char *, __p1, a1, \
		int , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_PKEY_copy_parameters(__p0, __p1) \
	LP2(3012, int , EVP_PKEY_copy_parameters, \
		EVP_PKEY *, __p0, a0, \
		EVP_PKEY *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_mpi2bn(__p0, __p1, __p2) \
	LP3(1254, BIGNUM *, BN_mpi2bn, \
		unsigned char *, __p0, a0, \
		int , __p1, a1, \
		BIGNUM *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CRYPTO_free_ex_data(__p0, __p1, __p2) \
	LP3NR(1836, CRYPTO_free_ex_data, \
		STACK *, __p0, a0, \
		char *, __p1, a1, \
		CRYPTO_EX_DATA *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_use_RSAPrivateKey_ASN1(__p0, __p1, __p2) \
	LP3(4788, int , SSL_use_RSAPrivateKey_ASN1, \
		SSL *, __p0, a0, \
		unsigned char *, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_PKCS12_MAC_DATA(__p0, __p1) \
	LP2(3726, int , i2d_PKCS12_MAC_DATA, \
		PKCS12_MAC_DATA *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_CINF_new() \
	LP0(6984, X509_CINF *, X509_CINF_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_f_reliable() \
	LP0(2598, BIO_METHOD *, BIO_f_reliable, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS7_digest_from_attributes(__p0) \
	LP1(4206, ASN1_OCTET_STRING *, PKCS7_digest_from_attributes, \
		STACK *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define OBJ_cleanup() \
	LP0NR(3288, OBJ_cleanup, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_get_pubkey(__p0) \
	LP1(7296, EVP_PKEY *, X509_get_pubkey, \
		X509 *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_PKEY_free(__p0) \
	LP1NR(2982, EVP_PKEY_free, \
		EVP_PKEY *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define RSA_padding_check_PKCS1_type_1(__p0, __p1, __p2, __p3, __p4) \
	LP5(4482, int , RSA_padding_check_PKCS1_type_1, \
		unsigned char *, __p0, a0, \
		int , __p1, a1, \
		unsigned char *, __p2, a2, \
		int , __p3, a3, \
		int , __p4, d0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define RSA_padding_check_PKCS1_type_2(__p0, __p1, __p2, __p3, __p4) \
	LP5(4494, int , RSA_padding_check_PKCS1_type_2, \
		unsigned char *, __p0, a0, \
		int , __p1, a1, \
		unsigned char *, __p2, a2, \
		int , __p3, a3, \
		int , __p4, d0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_DigestUpdate(__p0, __p1, __p2) \
	LP3NR(2388, EVP_DigestUpdate, \
		EVP_MD_CTX *, __p0, a0, \
		const unsigned char *, __p1, a1, \
		unsigned int , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ms_time_free(__p0) \
	LP1NR(5706, ms_time_free, \
		char *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_DecodeUpdate(__p0, __p1, __p2, __p3, __p4) \
	LP5(2544, int , EVP_DecodeUpdate, \
		EVP_ENCODE_CTX *, __p0, a0, \
		unsigned char *, __p1, a1, \
		int *, __p2, a2, \
		unsigned char *, __p3, a3, \
		int , __p4, d0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_cleanup() \
	LP0NR(2934, EVP_cleanup, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define bn_sqr_words(__p0, __p1, __p2) \
	LP3NR(1554, bn_sqr_words, \
		BN_ULONG *, __p0, a0, \
		BN_ULONG *, __p1, a1, \
		int , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_REQ_to_X509(__p0, __p1, __p2) \
	LP3(6702, X509 *, X509_REQ_to_X509, \
		X509_REQ *, __p0, a0, \
		int , __p1, a1, \
		EVP_PKEY *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_X509_REQ_bio(__p0, __p1) \
	LP2(6540, X509_REQ *, d2i_X509_REQ_bio, \
		BIO *, __p0, a0, \
		X509_REQ **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_get_retry_reason(__p0) \
	LP1(948, int , BIO_get_retry_reason, \
		BIO *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define uni2asc(__p0, __p1) \
	LP2(3672, char *, uni2asc, \
		unsigned char *, __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS7_SIGN_ENVELOPE_free(__p0) \
	LP1NR(3984, PKCS7_SIGN_ENVELOPE_free, \
		PKCS7_SIGN_ENVELOPE *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ERR_get_err_state_table() \
	LP0(2364, LHASH *, ERR_get_err_state_table, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define OBJ_create_objects(__p0) \
	LP1(3294, int , OBJ_create_objects, \
		BIO *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_mod_exp_mont(__p0, __p1, __p2, __p3, __p4, __p5) \
	LP6(1398, int , BN_mod_exp_mont, \
		BIGNUM *, __p0, a0, \
		BIGNUM *, __p1, a1, \
		const BIGNUM *, __p2, a2, \
		const BIGNUM *, __p3, a3, \
		BN_CTX *, __p4, d0, \
		BN_MONT_CTX *, __p5, d1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CRYPTO_get_id_callback() \
	LP0(1896, APTR , CRYPTO_get_id_callback, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ASN1_seq_pack(__p0, __p1, __p2, __p3) \
	LP4(792, unsigned char *, ASN1_seq_pack, \
		STACK *, __p0, a0, \
		APTR , __p1, a1, \
		unsigned char **, __p2, a2, \
		int *, __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ASN1_HEADER_free(__p0) \
	LP1NR(720, ASN1_HEADER_free, \
		ASN1_HEADER *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_alert_desc_string(__p0) \
	LP1(5316, char *, SSL_alert_desc_string, \
		int , __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_SXNETID(__p0, __p1, __p2) \
	LP3(5856, SXNETID *, d2i_SXNETID, \
		SXNETID **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_dup(__p0) \
	LP1(6588, X509 *, X509_dup, \
		X509 *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSLeay_add_all_ciphers() \
	LP0NR(2898, SSLeay_add_all_ciphers, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_USERNOTICE(__p0, __p1) \
	LP2(6102, int , i2d_USERNOTICE, \
		USERNOTICE *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_CIPHER_set_asn1_iv(__p0, __p1) \
	LP2(3054, int , EVP_CIPHER_set_asn1_iv, \
		EVP_CIPHER_CTX *, __p0, a0, \
		ASN1_TYPE *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS7_ENC_CONTENT_new() \
	LP0(3930, PKCS7_ENC_CONTENT *, PKCS7_ENC_CONTENT_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_rc2_64_cbc() \
	LP0(2802, EVP_CIPHER *, EVP_rc2_64_cbc, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_INFO_free(__p0) \
	LP1NR(7206, X509_INFO_free, \
		X509_INFO *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_PKCS7(__p0, __p1) \
	LP2(4068, int , i2d_PKCS7, \
		PKCS7 *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_NAME_ENTRY_free(__p0) \
	LP1NR(6936, X509_NAME_ENTRY_free, \
		X509_NAME_ENTRY *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_DIRECTORYSTRING(__p0, __p1, __p2) \
	LP3(402, ASN1_STRING *, d2i_DIRECTORYSTRING, \
		ASN1_STRING **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ASN1_STRING_free(__p0) \
	LP1NR(198, ASN1_STRING_free, \
		ASN1_STRING *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_PKEY_new() \
	LP0(2976, EVP_PKEY *, EVP_PKEY_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_RSAPublicKey(__p0, __p1) \
	LP2(4398, int , i2d_RSAPublicKey, \
		RSA *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_NAME_get_text_by_OBJ(__p0, __p1, __p2, __p3) \
	LP4(7428, int , X509_NAME_get_text_by_OBJ, \
		X509_NAME *, __p0, a0, \
		ASN1_OBJECT *, __p1, a1, \
		char *, __p2, a2, \
		int , __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS8_encrypt(__p0, __p1, __p2, __p3, __p4, __p5, __p6, __p7) \
	LP8(3546, X509_SIG *, PKCS8_encrypt, \
		int , __p0, a0, \
		const EVP_CIPHER *, __p1, a1, \
		const char *, __p2, a2, \
		int , __p3, a3, \
		unsigned char *, __p4, d0, \
		int , __p5, d1, \
		int , __p6, d2, \
		PKCS8_PRIV_KEY_INFO *, __p7, d3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS12_parse(__p0, __p1, __p2, __p3, __p4) \
	LP5(3786, int , PKCS12_parse, \
		PKCS12 *, __p0, a0, \
		const char *, __p1, a1, \
		EVP_PKEY **, __p2, a2, \
		X509 **, __p3, a3, \
		STACK **, __p4, d0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ext_ku_free(__p0) \
	LP1NR(6018, ext_ku_free, \
		STACK *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_EXTENSION_free(__p0) \
	LP1NR(6912, X509_EXTENSION_free, \
		X509_EXTENSION *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_CTX_set_verify_depth(__p0, __p1) \
	LP2NR(5040, SSL_CTX_set_verify_depth, \
		SSL_CTX *, __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_ASN1_VISIBLESTRING(__p0, __p1) \
	LP2(342, int , i2d_ASN1_VISIBLESTRING, \
		ASN1_VISIBLESTRING *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_NETSCAPE_CERT_SEQUENCE(__p0, __p1, __p2) \
	LP3(7188, NETSCAPE_CERT_SEQUENCE *, d2i_NETSCAPE_CERT_SEQUENCE, \
		NETSCAPE_CERT_SEQUENCE **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_int_ctrl(__p0, __p1, __p2, __p3) \
	LP4(912, long , BIO_int_ctrl, \
		BIO *, __p0, a0, \
		int , __p1, a1, \
		long , __p2, a2, \
		int , __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define RSA_verify(__p0, __p1, __p2, __p3, __p4, __p5) \
	LP6(4446, int , RSA_verify, \
		int , __p0, a0, \
		unsigned char *, __p1, a1, \
		unsigned int , __p2, a2, \
		unsigned char *, __p3, a3, \
		unsigned int , __p4, d0, \
		RSA *, __p5, d1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_CTX_get_ex_new_index(__p0, __p1, __p2, __p3, __p4) \
	LP5(5556, int , SSL_CTX_get_ex_new_index, \
		long , __p0, a0, \
		char *, __p1, a1, \
		APTR , __p2, a2, \
		APTR , __p3, a3, \
		APTR , __p4, d0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_PKCS82PKEY(__p0) \
	LP1(7836, EVP_PKEY *, EVP_PKCS82PKEY, \
		PKCS8_PRIV_KEY_INFO *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509V3_add_value_bool_nf(__p0, __p1, __p2) \
	LP3(6258, int , X509V3_add_value_bool_nf, \
		char *, __p0, a0, \
		int , __p1, a1, \
		STACK **, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_LOOKUP_by_issuer_serial(__p0, __p1, __p2, __p3, __p4) \
	LP5(7974, int , X509_LOOKUP_by_issuer_serial, \
		X509_LOOKUP *, __p0, a0, \
		int , __p1, a1, \
		X509_NAME *, __p2, a2, \
		ASN1_INTEGER *, __p3, a3, \
		X509_OBJECT *, __p4, d0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define OBJ_NAME_add(__p0, __p1, __p2) \
	LP3(3180, int , OBJ_NAME_add, \
		const char *, __p0, a0, \
		int , __p1, a1, \
		const char *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_ctrl(__p0, __p1, __p2, __p3) \
	LP4(5160, long , SSL_ctrl, \
		SSL *, __p0, a0, \
		int , __p1, a1, \
		long , __p2, a2, \
		char *, __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_Netscape_RSA_2(__p0, __p1, __p2, __p3) \
	LP4(4434, RSA *, d2i_Netscape_RSA_2, \
		RSA **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		APTR , __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS12_pack_p7encdata(__p0, __p1, __p2, __p3, __p4, __p5, __p6) \
	LP7(3564, PKCS7 *, PKCS12_pack_p7encdata, \
		int , __p0, a0, \
		const char *, __p1, a1, \
		int , __p2, a2, \
		unsigned char *, __p3, a3, \
		int , __p4, d0, \
		int , __p5, d1, \
		STACK *, __p6, d2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_CIPHER_get_name(__p0) \
	LP1(4668, const char *, SSL_CIPHER_get_name, \
		SSL_CIPHER *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS7_signatureVerify(__p0, __p1, __p2, __p3) \
	LP4(8166, int , PKCS7_signatureVerify, \
		BIO *, __p0, a0, \
		PKCS7 *, __p1, a1, \
		PKCS7_SIGNER_INFO *, __p2, a2, \
		X509 *, __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509V3_add_value_bool(__p0, __p1, __p2) \
	LP3(6324, int , X509V3_add_value_bool, \
		const char *, __p0, a0, \
		int , __p1, a1, \
		STACK **, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PEM_get_EVP_CIPHER_INFO(__p0, __p1) \
	LP2(3300, int , PEM_get_EVP_CIPHER_INFO, \
		char *, __p0, a0, \
		EVP_CIPHER_INFO *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_CTX_init(__p0) \
	LP1NR(1188, BN_CTX_init, \
		BN_CTX *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ERR_print_errors(__p0) \
	LP1NR(2310, ERR_print_errors, \
		BIO *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_set_verify(__p0, __p1, __p2) \
	LP3NR(4770, SSL_set_verify, \
		SSL *, __p0, a0, \
		int , __p1, a1, \
		APTR , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_X509_NAME(__p0, __p1, __p2) \
	LP3(6972, X509_NAME *, d2i_X509_NAME, \
		X509_NAME **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_sub_word(__p0, __p1) \
	LP2(1338, int , BN_sub_word, \
		BIGNUM *, __p0, a0, \
		BN_ULONG , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509V3_add_value_uchar(__p0, __p1, __p2) \
	LP3(6318, int , X509V3_add_value_uchar, \
		const char *, __p0, a0, \
		const unsigned char *, __p1, a1, \
		STACK **, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_STORE_add_cert(__p0, __p1) \
	LP2(7914, int , X509_STORE_add_cert, \
		X509_STORE *, __p0, a0, \
		X509 *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_delete_ext(__p0, __p1) \
	LP2(7572, X509_EXTENSION *, X509_delete_ext, \
		X509 *, __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_div_recp(__p0, __p1, __p2, __p3, __p4) \
	LP5(1698, int , BN_div_recp, \
		BIGNUM *, __p0, a0, \
		BIGNUM *, __p1, a1, \
		BIGNUM *, __p2, a2, \
		BN_RECP_CTX *, __p3, a3, \
		BN_CTX *, __p4, d0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ASN1_TYPE_set(__p0, __p1, __p2) \
	LP3NR(162, ASN1_TYPE_set, \
		ASN1_TYPE *, __p0, a0, \
		int , __p1, a1, \
		void *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_CTX_set_default_verify_paths(__p0) \
	LP1(5454, int , SSL_CTX_set_default_verify_paths, \
		SSL_CTX *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_REVOKED_free(__p0) \
	LP1NR(7038, X509_REVOKED_free, \
		X509_REVOKED *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_sha() \
	LP0(2628, EVP_MD *, EVP_sha, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_X509_ATTRIBUTE(__p0, __p1, __p2) \
	LP3(6894, X509_ATTRIBUTE *, d2i_X509_ATTRIBUTE, \
		X509_ATTRIBUTE **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_subject_name_hash(__p0) \
	LP1(7368, unsigned long , X509_subject_name_hash, \
		X509 *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define DSA_verify(__p0, __p1, __p2, __p3, __p4, __p5) \
	LP6(2148, int , DSA_verify, \
		int , __p0, a0, \
		const unsigned char *, __p1, a1, \
		int , __p2, a2, \
		unsigned char *, __p3, a3, \
		int , __p4, d0, \
		DSA *, __p5, d1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CRYPTO_dbg_realloc(__p0, __p1, __p2, __p3) \
	LP4(1986, void *, CRYPTO_dbg_realloc, \
		void *, __p0, a0, \
		int , __p1, a1, \
		const char *, __p2, a2, \
		int , __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ERR_reason_error_string(__p0) \
	LP1(2304, const char *, ERR_reason_error_string, \
		unsigned long , __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSLeay() \
	LP0(1806, unsigned long , SSLeay, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define DSA_size(__p0) \
	LP1(2130, int , DSA_size, \
		DSA *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS7_dataInit(__p0, __p1) \
	LP2(4140, BIO *, PKCS7_dataInit, \
		PKCS7 *, __p0, a0, \
		BIO *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_X509_SIG(__p0, __p1, __p2) \
	LP3(6822, X509_SIG *, d2i_X509_SIG, \
		X509_SIG **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_DIST_POINT_NAME(__p0, __p1) \
	LP2(6198, int , i2d_DIST_POINT_NAME, \
		DIST_POINT_NAME *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS8_PRIV_KEY_INFO_free(__p0) \
	LP1NR(7830, PKCS8_PRIV_KEY_INFO_free, \
		PKCS8_PRIV_KEY_INFO *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_issuer_and_serial_cmp(__p0, __p1) \
	LP2(7338, int , X509_issuer_and_serial_cmp, \
		X509 *, __p0, a0, \
		X509 *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_X509_PUBKEY(__p0, __p1) \
	LP2(6774, int , i2d_X509_PUBKEY, \
		X509_PUBKEY *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PEM_read_bio_DHparams(__p0, __p1, __p2, __p3) \
	LP4(3450, DH *, PEM_read_bio_DHparams, \
		BIO *, __p0, a0, \
		DH **, __p1, a1, \
		pem_password_cb *, __p2, a2, \
		void *, __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define OBJ_txt2nid(__p0) \
	LP1(3234, int , OBJ_txt2nid, \
		char *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define DSAparams_print(__p0, __p1) \
	LP2(2214, int , DSAparams_print, \
		BIO *, __p0, a0, \
		DSA *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_get_verify_mode(__p0) \
	LP1(4752, int , SSL_get_verify_mode, \
		SSL *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_LOOKUP_shutdown(__p0) \
	LP1(7992, int , X509_LOOKUP_shutdown, \
		X509_LOOKUP *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_new_socket(__p0, __p1) \
	LP2(1128, BIO *, BIO_new_socket, \
		int , __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_EncodeInit(__p0) \
	LP1NR(2514, EVP_EncodeInit, \
		EVP_ENCODE_CTX *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_CRL_add_ext(__p0, __p1, __p2) \
	LP3(7620, int , X509_CRL_add_ext, \
		X509_CRL *, __p0, a0, \
		X509_EXTENSION *, __p1, a1, \
		int , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS7_dup(__p0) \
	LP1(3840, PKCS7 *, PKCS7_dup, \
		PKCS7 *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_f_ssl() \
	LP0(4554, BIO_METHOD *, BIO_f_ssl, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CleanupAmiSSLA(__p0) \
	LP1(36, long , CleanupAmiSSLA, \
		struct TagItem *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_set_ex_data(__p0, __p1, __p2) \
	LP3(5508, int , SSL_set_ex_data, \
		SSL *, __p0, a0, \
		int , __p1, a1, \
		void *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_BASIC_CONSTRAINTS(__p0, __p1, __p2) \
	LP3(5778, BASIC_CONSTRAINTS *, d2i_BASIC_CONSTRAINTS, \
		BASIC_CONSTRAINTS **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CRYPTO_malloc(__p0) \
	LP1(1956, void *, CRYPTO_malloc, \
		int , __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_dup_CA_list(__p0) \
	LP1(5388, STACK *, SSL_dup_CA_list, \
		STACK *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PEM_SignFinal(__p0, __p1, __p2, __p3) \
	LP4(3378, int , PEM_SignFinal, \
		EVP_MD_CTX *, __p0, a0, \
		unsigned char *, __p1, a1, \
		unsigned int *, __p2, a2, \
		EVP_PKEY *, __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define RSA_public_decrypt(__p0, __p1, __p2, __p3, __p4) \
	LP5(4344, int , RSA_public_decrypt, \
		int , __p0, a0, \
		unsigned char *, __p1, a1, \
		unsigned char *, __p2, a2, \
		RSA *, __p3, a3, \
		int , __p4, d0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_new() \
	LP0(7008, X509 *, X509_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define HMAC_cleanup(__p0) \
	LP1NR(3084, HMAC_cleanup, \
		HMAC_CTX *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_CRL_sign(__p0, __p1, __p2) \
	LP3(6492, int , X509_CRL_sign, \
		X509_CRL *, __p0, a0, \
		EVP_PKEY *, __p1, a1, \
		const EVP_MD *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_PBE_cleanup() \
	LP0NR(8154, EVP_PBE_cleanup, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_sock_cleanup() \
	LP0NR(1110, BIO_sock_cleanup, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS12_get_friendlyname(__p0) \
	LP1(3600, char *, PKCS12_get_friendlyname, \
		PKCS12_SAFEBAG *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_STORE_CTX_get_chain(__p0) \
	LP1(8052, STACK *, X509_STORE_CTX_get_chain, \
		X509_STORE_CTX *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_from_montgomery(__p0, __p1, __p2, __p3) \
	LP4(1596, int , BN_from_montgomery, \
		BIGNUM *, __p0, a0, \
		BIGNUM *, __p1, a1, \
		BN_MONT_CTX *, __p2, a2, \
		BN_CTX *, __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ASN1_TYPE_set_octetstring(__p0, __p1, __p2) \
	LP3(762, int , ASN1_TYPE_set_octetstring, \
		ASN1_TYPE *, __p0, a0, \
		unsigned char *, __p1, a1, \
		int , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_NAME_cmp(__p0, __p1) \
	LP2(7374, int , X509_NAME_cmp, \
		X509_NAME *, __p0, a0, \
		X509_NAME *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_get_ex_new_index(__p0, __p1, __p2, __p3, __p4) \
	LP5(834, int , BIO_get_ex_new_index, \
		long , __p0, a0, \
		char *, __p1, a1, \
		APTR , __p2, a2, \
		APTR , __p3, a3, \
		APTR , __p4, d0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_NAME_free(__p0) \
	LP1NR(6960, X509_NAME_free, \
		X509_NAME *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_CIPHER_CTX_cleanup(__p0) \
	LP1NR(2574, EVP_CIPHER_CTX_cleanup, \
		EVP_CIPHER_CTX *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define RSA_sign_ASN1_OCTET_STRING(__p0, __p1, __p2, __p3, __p4, __p5) \
	LP6(4452, int , RSA_sign_ASN1_OCTET_STRING, \
		int , __p0, a0, \
		unsigned char *, __p1, a1, \
		unsigned int , __p2, a2, \
		unsigned char *, __p3, a3, \
		unsigned int *, __p4, d0, \
		RSA *, __p5, d1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SXNET_add_id_asc(__p0, __p1, __p2, __p3) \
	LP4(5874, int , SXNET_add_id_asc, \
		SXNET **, __p0, a0, \
		char *, __p1, a1, \
		char *, __p2, a2, \
		int , __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS12_gen_mac(__p0, __p1, __p2, __p3, __p4) \
	LP5(3642, int , PKCS12_gen_mac, \
		PKCS12 *, __p0, a0, \
		const char *, __p1, a1, \
		int , __p2, a2, \
		unsigned char *, __p3, a3, \
		unsigned int *, __p4, d0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_SESSION_new() \
	LP0(4944, SSL_SESSION *, SSL_SESSION_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_CTX_set_session_id_context(__p0, __p1, __p2) \
	LP3(5106, int , SSL_CTX_set_session_id_context, \
		SSL_CTX *, __p0, a0, \
		const unsigned char *, __p1, a1, \
		unsigned int , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_use_certificate_file(__p0, __p1, __p2) \
	LP3(4830, int , SSL_use_certificate_file, \
		SSL *, __p0, a0, \
		const char *, __p1, a1, \
		int , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_CTX_set_verify(__p0, __p1, __p2) \
	LP3NR(5034, SSL_CTX_set_verify, \
		SSL_CTX *, __p0, a0, \
		int , __p1, a1, \
		APTR , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509v3_get_ext_by_NID(__p0, __p1, __p2) \
	LP3(7506, int , X509v3_get_ext_by_NID, \
		const STACK *, __p0, a0, \
		int , __p1, a1, \
		int , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_PKCS7_ENVELOPE(__p0, __p1, __p2) \
	LP3(3972, PKCS7_ENVELOPE *, d2i_PKCS7_ENVELOPE, \
		PKCS7_ENVELOPE **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define RSA_blinding_off(__p0) \
	LP1NR(4470, RSA_blinding_off, \
		RSA *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_accept(__p0, __p1) \
	LP2(1098, int , BIO_accept, \
		int , __p0, a0, \
		char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ASN1_unpack_string(__p0, __p1) \
	LP2(798, void *, ASN1_unpack_string, \
		ASN1_STRING *, __p0, a0, \
		APTR , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_REQ_set_version(__p0, __p1) \
	LP2(7308, int , X509_REQ_set_version, \
		X509_REQ *, __p0, a0, \
		long , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_s_log() \
	LP0(996, BIO_METHOD *, BIO_s_log, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define sk_pop_free(__p0, __p1) \
	LP2NR(5628, sk_pop_free, \
		STACK *, __p0, a0, \
		APTR , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509v3_get_ext(__p0, __p1) \
	LP2(7524, X509_EXTENSION *, X509v3_get_ext, \
		const STACK *, __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509V3_EXT_get(__p0) \
	LP1(6384, X509V3_EXT_METHOD *, X509V3_EXT_get, \
		X509_EXTENSION *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_ATTRIBUTE_dup(__p0) \
	LP1(6594, X509_ATTRIBUTE *, X509_ATTRIBUTE_dup, \
		X509_ATTRIBUTE *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_load_client_CA_file(__p0) \
	LP1(4860, STACK *, SSL_load_client_CA_file, \
		const char *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_RC4_IsReal() \
	LP0(108, long , EVP_RC4_IsReal, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS7_add_certificate(__p0, __p1) \
	LP2(4116, int , PKCS7_add_certificate, \
		PKCS7 *, __p0, a0, \
		X509 *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ASN1_STRING_set(__p0, __p1, __p2) \
	LP3(222, int , ASN1_STRING_set, \
		ASN1_STRING *, __p0, a0, \
		const void *, __p1, a1, \
		int , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_EXTENSION_create_by_OBJ(__p0, __p1, __p2, __p3) \
	LP4(7674, X509_EXTENSION *, X509_EXTENSION_create_by_OBJ, \
		X509_EXTENSION **, __p0, a0, \
		ASN1_OBJECT *, __p1, a1, \
		int , __p2, a2, \
		ASN1_OCTET_STRING *, __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_f_cipher() \
	LP0(2592, BIO_METHOD *, BIO_f_cipher, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PEM_write_bio_X509_CRL(__p0, __p1) \
	LP2(3480, int , PEM_write_bio_X509_CRL, \
		BIO *, __p0, a0, \
		X509_CRL *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_SESSION_get_timeout(__p0) \
	LP1(4926, long , SSL_SESSION_get_timeout, \
		SSL_SESSION *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_mod_mul(__p0, __p1, __p2, __p3, __p4) \
	LP5(1422, int , BN_mod_mul, \
		BIGNUM *, __p0, a0, \
		BIGNUM *, __p1, a1, \
		BIGNUM *, __p2, a2, \
		const BIGNUM *, __p3, a3, \
		BN_CTX *, __p4, d0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS7_add_signed_attribute(__p0, __p1, __p2, __p3) \
	LP4(4212, int , PKCS7_add_signed_attribute, \
		PKCS7_SIGNER_INFO *, __p0, a0, \
		int , __p1, a1, \
		int , __p2, a2, \
		void *, __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_AUTHORITY_KEYID(__p0, __p1) \
	LP2(5910, int , i2d_AUTHORITY_KEYID, \
		AUTHORITY_KEYID *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_REVOKED_add_ext(__p0, __p1, __p2) \
	LP3(7662, int , X509_REVOKED_add_ext, \
		X509_REVOKED *, __p0, a0, \
		X509_EXTENSION *, __p1, a1, \
		int , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_set_tmp_rsa_callback(__p0, __p1) \
	LP2NR(5574, SSL_set_tmp_rsa_callback, \
		SSL *, __p0, a0, \
		APTR , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define HMAC_Final(__p0, __p1, __p2) \
	LP3NR(3078, HMAC_Final, \
		HMAC_CTX *, __p0, a0, \
		unsigned char *, __p1, a1, \
		unsigned int *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_STORE_add_lookup(__p0, __p1) \
	LP2(7896, X509_LOOKUP *, X509_STORE_add_lookup, \
		X509_STORE *, __p0, a0, \
		X509_LOOKUP_METHOD *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ASN1_BIT_STRING_get_bit(__p0, __p1) \
	LP2(246, int , ASN1_BIT_STRING_get_bit, \
		ASN1_BIT_STRING *, __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_X509_NAME_ENTRY(__p0, __p1) \
	LP2(6942, int , i2d_X509_NAME_ENTRY, \
		X509_NAME_ENTRY *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_ASN1_BMPSTRING(__p0, __p1) \
	LP2(366, int , i2d_ASN1_BMPSTRING, \
		ASN1_BMPSTRING *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_get_default_cert_dir_env() \
	LP0(6678, const char *, X509_get_default_cert_dir_env, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ERR_load_BUF_strings() \
	LP0NR(1728, ERR_load_BUF_strings, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ERR_load_crypto_strings() \
	LP0NR(2334, ERR_load_crypto_strings, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_add_word(__p0, __p1) \
	LP2(1332, int , BN_add_word, \
		BIGNUM *, __p0, a0, \
		BN_ULONG , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_X509(__p0, __p1) \
	LP2(7020, int , i2d_X509, \
		X509 *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509V3_string_free(__p0, __p1) \
	LP2NR(6294, X509V3_string_free, \
		X509V3_CTX *, __p0, a0, \
		char *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_hex2bn(__p0, __p1) \
	LP2(1500, int , BN_hex2bn, \
		BIGNUM **, __p0, a0, \
		const char *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_clear(__p0) \
	LP1NR(1452, BN_clear, \
		BIGNUM *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define DSA_sign_setup(__p0, __p1, __p2, __p3) \
	LP4(2136, int , DSA_sign_setup, \
		DSA *, __p0, a0, \
		BN_CTX *, __p1, a1, \
		BIGNUM **, __p2, a2, \
		BIGNUM **, __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSLv3_client_method() \
	LP0(5220, SSL_METHOD *, SSLv3_client_method, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_dup(__p0) \
	LP1(5394, SSL *, SSL_dup, \
		SSL *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_X509_REQ(__p0, __p1) \
	LP2(6864, int , i2d_X509_REQ, \
		X509_REQ *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_connect(__p0) \
	LP1(5136, int , SSL_connect, \
		SSL *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_des_ofb() \
	LP0(2706, EVP_CIPHER *, EVP_des_ofb, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509V3_EXT_CRL_add_conf(__p0, __p1, __p2, __p3) \
	LP4(6252, int , X509V3_EXT_CRL_add_conf, \
		LHASH *, __p0, a0, \
		X509V3_CTX *, __p1, a1, \
		char *, __p2, a2, \
		X509_CRL *, __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_get_peer_certificate(__p0) \
	LP1(5004, X509 *, SSL_get_peer_certificate, \
		SSL *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS7_add_signature(__p0, __p1, __p2, __p3) \
	LP4(4158, PKCS7_SIGNER_INFO *, PKCS7_add_signature, \
		PKCS7 *, __p0, a0, \
		X509 *, __p1, a1, \
		EVP_PKEY *, __p2, a2, \
		EVP_MD *, __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_get_verify_result(__p0) \
	LP1(5502, long , SSL_get_verify_result, \
		SSL *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define RSA_padding_check_SSLv23(__p0, __p1, __p2, __p3, __p4) \
	LP5(4518, int , RSA_padding_check_SSLv23, \
		unsigned char *, __p0, a0, \
		int , __p1, a1, \
		unsigned char *, __p2, a2, \
		int , __p3, a3, \
		int , __p4, d0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_RSAPrivateKey(__p0, __p1) \
	LP2(4410, int , i2d_RSAPrivateKey, \
		RSA *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2s_ASN1_ENUMERATED(__p0, __p1) \
	LP2(6348, char *, i2s_ASN1_ENUMERATED, \
		X509V3_EXT_METHOD *, __p0, a0, \
		ASN1_ENUMERATED *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define RSA_get_default_method() \
	LP0(8178, RSA_METHOD *, RSA_get_default_method, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS7_SIGNED_new() \
	LP0(3906, PKCS7_SIGNED *, PKCS7_SIGNED_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_EXTENSION_get_data(__p0) \
	LP1(7704, ASN1_OCTET_STRING *, X509_EXTENSION_get_data, \
		X509_EXTENSION *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define bn_mul_words(__p0, __p1, __p2, __p3) \
	LP4(1548, BN_ULONG , bn_mul_words, \
		BN_ULONG *, __p0, a0, \
		BN_ULONG *, __p1, a1, \
		int , __p2, a2, \
		BN_ULONG , __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_ghbn_ctrl(__p0, __p1, __p2) \
	LP3(1158, long , BIO_ghbn_ctrl, \
		int , __p0, a0, \
		int , __p1, a1, \
		char *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_add_cipher(__p0) \
	LP1(2910, int , EVP_add_cipher, \
		EVP_CIPHER *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define OBJ_create(__p0, __p1, __p2) \
	LP3(3282, int , OBJ_create, \
		char *, __p0, a0, \
		char *, __p1, a1, \
		char *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define v2i_GENERAL_NAME(__p0, __p1, __p2) \
	LP3(6222, GENERAL_NAME *, v2i_GENERAL_NAME, \
		X509V3_EXT_METHOD *, __p0, a0, \
		X509V3_CTX *, __p1, a1, \
		CONF_VALUE *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_NAME_ENTRY_create_by_NID(__p0, __p1, __p2, __p3, __p4) \
	LP5(7464, X509_NAME_ENTRY *, X509_NAME_ENTRY_create_by_NID, \
		X509_NAME_ENTRY **, __p0, a0, \
		int , __p1, a1, \
		int , __p2, a2, \
		unsigned char *, __p3, a3, \
		int , __p4, d0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_set_verify_result(__p0, __p1) \
	LP2NR(5496, SSL_set_verify_result, \
		SSL *, __p0, a0, \
		long , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_alert_type_string_long(__p0) \
	LP1(5298, char *, SSL_alert_type_string_long, \
		int , __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_MD5_IsReal() \
	LP0(90, long , EVP_MD5_IsReal, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_MDC2_IsReal() \
	LP0(96, long , EVP_MDC2_IsReal, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_X509_EXTENSION(__p0, __p1, __p2) \
	LP3(6924, X509_EXTENSION *, d2i_X509_EXTENSION, \
		X509_EXTENSION **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS7_new() \
	LP0(4050, PKCS7 *, PKCS7_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_CTX_use_RSAPrivateKey_file(__p0, __p1, __p2) \
	LP3(4836, int , SSL_CTX_use_RSAPrivateKey_file, \
		SSL_CTX *, __p0, a0, \
		const char *, __p1, a1, \
		int , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_CIPHER_get_asn1_iv(__p0, __p1) \
	LP2(3060, int , EVP_CIPHER_get_asn1_iv, \
		EVP_CIPHER_CTX *, __p0, a0, \
		ASN1_TYPE *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ASN1_TYPE_free(__p0) \
	LP1NR(138, ASN1_TYPE_free, \
		ASN1_TYPE *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_get_ex_data_X509_STORE_CTX_idx() \
	LP0(5562, int , SSL_get_ex_data_X509_STORE_CTX_idx, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_set_client_CA_list(__p0, __p1) \
	LP2NR(5322, SSL_set_client_CA_list, \
		SSL *, __p0, a0, \
		STACK *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define NETSCAPE_CERT_SEQUENCE_free(__p0) \
	LP1NR(7194, NETSCAPE_CERT_SEQUENCE_free, \
		NETSCAPE_CERT_SEQUENCE *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_NAME_ENTRY_get_object(__p0) \
	LP1(7488, ASN1_OBJECT *, X509_NAME_ENTRY_get_object, \
		X509_NAME_ENTRY *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_REVOKED_get_ext_count(__p0) \
	LP1(7626, int , X509_REVOKED_get_ext_count, \
		X509_REVOKED *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PEM_write_bio_DSAparams(__p0, __p1) \
	LP2(3522, int , PEM_write_bio_DSAparams, \
		BIO *, __p0, a0, \
		DSA *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_CTX_free(__p0) \
	LP1NR(4602, SSL_CTX_free, \
		SSL_CTX *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CRYPTO_free(__p0) \
	LP1NR(1962, CRYPTO_free, \
		void *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_set(__p0, __p1) \
	LP2(864, int , BIO_set, \
		BIO *, __p0, a0, \
		BIO_METHOD *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define TXT_DB_free(__p0) \
	LP1NR(5748, TXT_DB_free, \
		TXT_DB *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ms_time_diff(__p0, __p1) \
	LP2(5718, double , ms_time_diff, \
		char *, __p0, a0, \
		char *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_REVOKED_delete_ext(__p0, __p1) \
	LP2(7656, X509_EXTENSION *, X509_REVOKED_delete_ext, \
		X509_REVOKED *, __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_RECP_CTX_free(__p0) \
	LP1NR(1674, BN_RECP_CTX_free, \
		BN_RECP_CTX *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define RSA_padding_add_PKCS1_OAEP(__p0, __p1, __p2, __p3, __p4, __p5) \
	LP6(4500, int , RSA_padding_add_PKCS1_OAEP, \
		unsigned char *, __p0, a0, \
		int , __p1, a1, \
		unsigned char *, __p2, a2, \
		int , __p3, a3, \
		unsigned char *, __p4, d0, \
		int , __p5, d1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define DSA_generate_key(__p0) \
	LP1(2190, int , DSA_generate_key, \
		DSA *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_STORE_add_crl(__p0, __p1) \
	LP2(7920, int , X509_STORE_add_crl, \
		X509_STORE *, __p0, a0, \
		X509_CRL *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define RSA_PKCS1_Default() \
	LP0(4380, RSA_METHOD *, RSA_PKCS1_Default, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_PKCS12_BAGS(__p0, __p1, __p2) \
	LP3(3690, PKCS12_BAGS *, d2i_PKCS12_BAGS, \
		PKCS12_BAGS **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_DHparams(__p0, __p1, __p2) \
	LP3(2064, DH *, d2i_DHparams, \
		DH **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_CTX_remove_session(__p0, __p1) \
	LP2(4992, int , SSL_CTX_remove_session, \
		SSL_CTX *, __p0, a0, \
		SSL_SESSION *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_ASN1_BOOLEAN(__p0, __p1, __p2) \
	LP3(258, int , d2i_ASN1_BOOLEAN, \
		int *, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_LOOKUP_free(__p0) \
	LP1NR(7956, X509_LOOKUP_free, \
		X509_LOOKUP *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_get_verify_callback(__p0) \
	LP1(4764, APTR , SSL_get_verify_callback, \
		SSL *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS7_add_recipient(__p0, __p1) \
	LP2(4176, PKCS7_RECIP_INFO *, PKCS7_add_recipient, \
		PKCS7 *, __p0, a0, \
		X509 *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PEM_write_bio_RSAPublicKey(__p0, __p1) \
	LP2(3492, int , PEM_write_bio_RSAPublicKey, \
		BIO *, __p0, a0, \
		RSA *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_reciprocal(__p0, __p1, __p2, __p3) \
	LP4(1434, int , BN_reciprocal, \
		BIGNUM *, __p0, a0, \
		BIGNUM *, __p1, a1, \
		int , __p2, a2, \
		BN_CTX *, __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_s_socket() \
	LP0(972, BIO_METHOD *, BIO_s_socket, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509V3_get_string(__p0, __p1, __p2) \
	LP3(6282, char *, X509V3_get_string, \
		X509V3_CTX *, __p0, a0, \
		char *, __p1, a1, \
		char *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_REVOKED_get_ext_by_critical(__p0, __p1, __p2) \
	LP3(7644, int , X509_REVOKED_get_ext_by_critical, \
		X509_REVOKED *, __p0, a0, \
		int , __p1, a1, \
		int , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define hex_to_string(__p0, __p1) \
	LP2(6420, char *, hex_to_string, \
		unsigned char *, __p0, a0, \
		long , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_get_cipherbyname(__p0) \
	LP1(2922, const EVP_CIPHER *, EVP_get_cipherbyname, \
		const char *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ASN1_object_size(__p0, __p1, __p2) \
	LP3(648, int , ASN1_object_size, \
		int , __p0, a0, \
		int , __p1, a1, \
		int , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS12_SAFEBAG_new() \
	LP0(3756, PKCS12_SAFEBAG *, PKCS12_SAFEBAG_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ASN1_i2d_bio(__p0, __p1, __p2) \
	LP3(666, int , ASN1_i2d_bio, \
		APTR , __p0, a0, \
		BIO *, __p1, a1, \
		unsigned char *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS12_PBE_keyivgen(__p0, __p1, __p2, __p3, __p4, __p5, __p6) \
	LP7(8238, int , PKCS12_PBE_keyivgen, \
		EVP_CIPHER_CTX *, __p0, a0, \
		const char *, __p1, a1, \
		int , __p2, a2, \
		ASN1_TYPE *, __p3, a3, \
		EVP_CIPHER *, __p4, d0, \
		EVP_MD *, __p5, d1, \
		int , __p6, d2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_CTX_set_quiet_shutdown(__p0, __p1) \
	LP2NR(5412, SSL_CTX_set_quiet_shutdown, \
		SSL_CTX *, __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509V3_add_value_int(__p0, __p1, __p2) \
	LP3(6330, int , X509V3_add_value_int, \
		const char *, __p0, a0, \
		ASN1_INTEGER *, __p1, a1, \
		STACK **, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_CRL_DIST_POINTS(__p0, __p1, __p2) \
	LP3(6168, STACK *, d2i_CRL_DIST_POINTS, \
		STACK **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2s_ASN1_ENUMERATED_TABLE(__p0, __p1) \
	LP2(6354, char *, i2s_ASN1_ENUMERATED_TABLE, \
		X509V3_EXT_METHOD *, __p0, a0, \
		ASN1_ENUMERATED *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_STORE_set_default_paths(__p0) \
	LP1(8004, int , X509_STORE_set_default_paths, \
		X509_STORE *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_EncryptUpdate(__p0, __p1, __p2, __p3, __p4) \
	LP5NR(2430, EVP_EncryptUpdate, \
		EVP_CIPHER_CTX *, __p0, a0, \
		unsigned char *, __p1, a1, \
		int *, __p2, a2, \
		unsigned char *, __p3, a3, \
		int , __p4, d0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_ATTRIBUTE_new() \
	LP0(6876, X509_ATTRIBUTE *, X509_ATTRIBUTE_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_get_quiet_shutdown(__p0) \
	LP1(5430, int , SSL_get_quiet_shutdown, \
		SSL *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_asn1_meth() \
	LP0(738, ASN1_METHOD *, X509_asn1_meth, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_state_string_long(__p0) \
	LP1(4902, char *, SSL_state_string_long, \
		SSL *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKEY_USAGE_PERIOD_new() \
	LP0(5946, PKEY_USAGE_PERIOD *, PKEY_USAGE_PERIOD_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_mul_word(__p0, __p1) \
	LP2(1326, int , BN_mul_word, \
		BIGNUM *, __p0, a0, \
		BN_ULONG , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_ASN1_HEADER(__p0, __p1) \
	LP2(702, int , i2d_ASN1_HEADER, \
		ASN1_HEADER *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ERR_load_ERR_strings() \
	LP0NR(2328, ERR_load_ERR_strings, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_exp(__p0, __p1, __p2, __p3) \
	LP4(1386, int , BN_exp, \
		BIGNUM *, __p0, a0, \
		BIGNUM *, __p1, a1, \
		BIGNUM *, __p2, a2, \
		BN_CTX *, __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_NOTICEREF(__p0, __p1) \
	LP2(6126, int , i2d_NOTICEREF, \
		NOTICEREF *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_PKEY_decrypt(__p0, __p1, __p2, __p3) \
	LP4(2940, int , EVP_PKEY_decrypt, \
		unsigned char *, __p0, a0, \
		unsigned char *, __p1, a1, \
		int , __p2, a2, \
		EVP_PKEY *, __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define GENERAL_NAME_free(__p0) \
	LP1NR(5814, GENERAL_NAME_free, \
		GENERAL_NAME *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_print(__p0, __p1) \
	LP2(7398, int , X509_print, \
		BIO *, __p0, a0, \
		X509 *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS7_content_free(__p0) \
	LP1NR(4062, PKCS7_content_free, \
		PKCS7 *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_NAME_get_entry(__p0, __p1) \
	LP2(7446, X509_NAME_ENTRY *, X509_NAME_get_entry, \
		X509_NAME *, __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_NETSCAPE_SPKI(__p0, __p1, __p2) \
	LP3(7146, NETSCAPE_SPKI *, d2i_NETSCAPE_SPKI, \
		NETSCAPE_SPKI **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define OBJ_obj2txt(__p0, __p1, __p2, __p3) \
	LP4(8160, int , OBJ_obj2txt, \
		char *, __p0, a0, \
		int , __p1, a1, \
		ASN1_OBJECT *, __p2, a2, \
		int , __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_PUBKEY_new() \
	LP0(6762, X509_PUBKEY *, X509_PUBKEY_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_SESSION_get_ex_new_index(__p0, __p1, __p2, __p3, __p4) \
	LP5(5538, int , SSL_SESSION_get_ex_new_index, \
		long , __p0, a0, \
		char *, __p1, a1, \
		APTR , __p2, a2, \
		APTR , __p3, a3, \
		APTR , __p4, d0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_get_SSL_CTX(__p0) \
	LP1(5472, SSL_CTX *, SSL_get_SSL_CTX, \
		SSL *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ASN1_TYPE_get_int_octetstring(__p0, __p1, __p2, __p3) \
	LP4(780, int , ASN1_TYPE_get_int_octetstring, \
		ASN1_TYPE *, __p0, a0, \
		long *, __p1, a1, \
		unsigned char *, __p2, a2, \
		int , __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_ctrl_get_write_guarantee(__p0) \
	LP1(8082, size_t , BIO_ctrl_get_write_guarantee, \
		BIO *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_rc4_40() \
	LP0(2754, EVP_CIPHER *, EVP_rc4_40, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_X509_VAL(__p0, __p1) \
	LP2(6750, int , i2d_X509_VAL, \
		X509_VAL *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_CERTIFICATEPOLICIES(__p0, __p1) \
	LP2(6030, int , i2d_CERTIFICATEPOLICIES, \
		STACK *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS5_pbe2_set(__p0, __p1, __p2, __p3) \
	LP4(8232, X509_ALGOR *, PKCS5_pbe2_set, \
		const EVP_CIPHER *, __p0, a0, \
		int , __p1, a1, \
		unsigned char *, __p2, a2, \
		int , __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_POLICYINFO(__p0, __p1) \
	LP2(6054, int , i2d_POLICYINFO, \
		POLICYINFO *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define TLSv1_method() \
	LP0(5244, SSL_METHOD *, TLSv1_method, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_new(__p0) \
	LP1(5112, SSL *, SSL_new, \
		SSL_CTX *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_get_ciphers(__p0) \
	LP1(5262, STACK *, SSL_get_ciphers, \
		SSL *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PEM_read_bio_RSAPrivateKey(__p0, __p1, __p2, __p3) \
	LP4(3420, RSA *, PEM_read_bio_RSAPrivateKey, \
		BIO *, __p0, a0, \
		RSA **, __p1, a1, \
		pem_password_cb *, __p2, a2, \
		void *, __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_ASN1_TIME(__p0, __p1, __p2) \
	LP3(468, ASN1_TIME *, d2i_ASN1_TIME, \
		ASN1_TIME **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_alert_type_string(__p0) \
	LP1(5304, char *, SSL_alert_type_string, \
		int , __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_get_ex_data(__p0, __p1) \
	LP2(5514, void *, SSL_get_ex_data, \
		SSL *, __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_PUBKEY_free(__p0) \
	LP1NR(6768, X509_PUBKEY_free, \
		X509_PUBKEY *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_rc2_ofb() \
	LP0(2814, EVP_CIPHER *, EVP_rc2_ofb, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define DH_generate_parameters(__p0, __p1, __p2, __p3) \
	LP4(2040, DH *, DH_generate_parameters, \
		int , __p0, a0, \
		int , __p1, a1, \
		APTR , __p2, a2, \
		void *, __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_CRL_get_ext_count(__p0) \
	LP1(7584, int , X509_CRL_get_ext_count, \
		X509_CRL *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_ext_ku(__p0, __p1) \
	LP2(6006, int , i2d_ext_ku, \
		STACK *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_get_accept_socket(__p0, __p1) \
	LP2(1092, int , BIO_get_accept_socket, \
		char *, __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_get_default_private_dir() \
	LP0(6690, const char *, X509_get_default_private_dir, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_issuer_name_hash(__p0) \
	LP1(7356, unsigned long , X509_issuer_name_hash, \
		X509 *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define NETSCAPE_SPKI_sign(__p0, __p1, __p2) \
	LP3(6498, int , NETSCAPE_SPKI_sign, \
		NETSCAPE_SPKI *, __p0, a0, \
		EVP_PKEY *, __p1, a1, \
		const EVP_MD *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_VAL_free(__p0) \
	LP1NR(6744, X509_VAL_free, \
		X509_VAL *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509V3_EXT_print(__p0, __p1, __p2, __p3) \
	LP4(6444, int , X509V3_EXT_print, \
		BIO *, __p0, a0, \
		X509_EXTENSION *, __p1, a1, \
		int , __p2, a2, \
		int , __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PEM_read_bio_PKCS7(__p0, __p1, __p2, __p3) \
	LP4(3444, PKCS7 *, PEM_read_bio_PKCS7, \
		BIO *, __p0, a0, \
		PKCS7 **, __p1, a1, \
		pem_password_cb *, __p2, a2, \
		void *, __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PEM_read_bio_PKCS8(__p0, __p1, __p2, __p3) \
	LP4(8244, X509_SIG *, PEM_read_bio_PKCS8, \
		BIO *, __p0, a0, \
		X509_SIG **, __p1, a1, \
		pem_password_cb *, __p2, a2, \
		void *, __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define a2d_ASN1_OBJECT(__p0, __p1, __p2, __p3) \
	LP4(540, int , a2d_ASN1_OBJECT, \
		unsigned char *, __p0, a0, \
		int , __p1, a1, \
		const char *, __p2, a2, \
		int , __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_cast5_ofb() \
	LP0(2862, EVP_CIPHER *, EVP_cast5_ofb, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509v3_delete_ext(__p0, __p1) \
	LP2(7530, X509_EXTENSION *, X509v3_delete_ext, \
		STACK *, __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_ucmp(__p0, __p1) \
	LP2(1470, int , BN_ucmp, \
		const BIGNUM *, __p0, a0, \
		const BIGNUM *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509V3_get_value_bool(__p0, __p1) \
	LP2(6264, int , X509V3_get_value_bool, \
		CONF_VALUE *, __p0, a0, \
		int *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_CTX_get_verify_depth(__p0) \
	LP1(5022, int , SSL_CTX_get_verify_depth, \
		SSL_CTX *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CRYPTO_set_add_lock_callback(__p0) \
	LP1NR(1878, CRYPTO_set_add_lock_callback, \
		APTR , __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509V3_EXT_get_nid(__p0) \
	LP1(6390, X509V3_EXT_METHOD *, X509V3_EXT_get_nid, \
		int , __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_CTX_new(__p0) \
	LP1(4596, SSL_CTX *, SSL_CTX_new, \
		SSL_METHOD *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS7_content_new(__p0, __p1) \
	LP2(4128, int , PKCS7_content_new, \
		PKCS7 *, __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_gets(__p0, __p1, __p2) \
	LP3(882, int , BIO_gets, \
		BIO *, __p0, a0, \
		char *, __p1, a1, \
		int , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_POLICYQUALINFO(__p0, __p1, __p2) \
	LP3(6090, POLICYQUALINFO *, d2i_POLICYQUALINFO, \
		POLICYQUALINFO **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ASN1_UTCTIME_set_string(__p0, __p1) \
	LP2(306, int , ASN1_UTCTIME_set_string, \
		ASN1_UTCTIME *, __p0, a0, \
		char *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define RSA_padding_add_SSLv23(__p0, __p1, __p2, __p3) \
	LP4(4512, int , RSA_padding_add_SSLv23, \
		unsigned char *, __p0, a0, \
		int , __p1, a1, \
		unsigned char *, __p2, a2, \
		int , __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS7_RECIP_INFO_set(__p0, __p1) \
	LP2(4188, int , PKCS7_RECIP_INFO_set, \
		PKCS7_RECIP_INFO *, __p0, a0, \
		X509 *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_num_bits(__p0) \
	LP1(1206, int , BN_num_bits, \
		const BIGNUM *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define RSA_flags(__p0) \
	LP1(4362, int , RSA_flags, \
		RSA *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509V3_section_free(__p0, __p1) \
	LP2NR(6300, X509V3_section_free, \
		X509V3_CTX *, __p0, a0, \
		STACK *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define TXT_DB_get_by_index(__p0, __p1, __p2) \
	LP3(5754, char **, TXT_DB_get_by_index, \
		TXT_DB *, __p0, a0, \
		int , __p1, a1, \
		char **, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS7_ENCRYPT_new() \
	LP0(4026, PKCS7_ENCRYPT *, PKCS7_ENCRYPT_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_verify_cert_error_string(__p0) \
	LP1(6456, const char *, X509_verify_cert_error_string, \
		long , __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_library_init() \
	LP0(5376, int , SSL_library_init, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BUF_MEM_grow(__p0, __p1) \
	LP2(1716, int , BUF_MEM_grow, \
		BUF_MEM *, __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_sock_init() \
	LP0(1104, int , BIO_sock_init, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CRYPTO_get_ex_new_index(__p0, __p1, __p2, __p3, __p4, __p5, __p6) \
	LP7(1812, int , CRYPTO_get_ex_new_index, \
		int , __p0, a0, \
		STACK **, __p1, a1, \
		long , __p2, a2, \
		char *, __p3, a3, \
		APTR , __p4, d0, \
		APTR , __p5, d1, \
		APTR , __p6, d2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_new_ssl_connect(__p0) \
	LP1(4566, BIO *, BIO_new_ssl_connect, \
		SSL_CTX *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define AmiSSLMalloc(__p0) \
	LP1(42, void *, AmiSSLMalloc, \
		long , __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_STORE_CTX_init(__p0, __p1, __p2, __p3) \
	LP4NR(7884, X509_STORE_CTX_init, \
		X509_STORE_CTX *, __p0, a0, \
		X509_STORE *, __p1, a1, \
		X509 *, __p2, a2, \
		STACK *, __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define DH_size(__p0) \
	LP1(2034, int , DH_size, \
		DH *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_sign(__p0, __p1, __p2) \
	LP3(6480, int , X509_sign, \
		X509 *, __p0, a0, \
		EVP_PKEY *, __p1, a1, \
		const EVP_MD *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_set_tcp_ndelay(__p0, __p1) \
	LP2(1116, int , BIO_set_tcp_ndelay, \
		int , __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_X509_ALGOR(__p0, __p1) \
	LP2(6726, int , i2d_X509_ALGOR, \
		X509_ALGOR *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_alert_desc_string_long(__p0) \
	LP1(5310, char *, SSL_alert_desc_string_long, \
		int , __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_X509_bio(__p0, __p1) \
	LP2(6522, int , i2d_X509_bio, \
		BIO *, __p0, a0, \
		X509 *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2v_GENERAL_NAME(__p0, __p1, __p2) \
	LP3(5820, STACK *, i2v_GENERAL_NAME, \
		X509V3_EXT_METHOD *, __p0, a0, \
		GENERAL_NAME *, __p1, a1, \
		STACK *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define TXT_DB_write(__p0, __p1) \
	LP2(5736, long , TXT_DB_write, \
		BIO *, __p0, a0, \
		TXT_DB *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CRYPTO_add_lock(__p0, __p1, __p2, __p3, __p4) \
	LP5(1914, int , CRYPTO_add_lock, \
		int *, __p0, a0, \
		int , __p1, a1, \
		int , __p2, a2, \
		const char *, __p3, a3, \
		int , __p4, d0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_get_shared_ciphers(__p0, __p1, __p2) \
	LP3(4686, char *, SSL_get_shared_ciphers, \
		SSL *, __p0, a0, \
		char *, __p1, a1, \
		int , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_CRL_get_ext_by_NID(__p0, __p1, __p2) \
	LP3(7590, int , X509_CRL_get_ext_by_NID, \
		X509_CRL *, __p0, a0, \
		int , __p1, a1, \
		int , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ASN1_UNIVERSALSTRING_to_string(__p0) \
	LP1(726, int , ASN1_UNIVERSALSTRING_to_string, \
		ASN1_UNIVERSALSTRING *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_des_ede_cbc() \
	LP0(2730, EVP_CIPHER *, EVP_des_ede_cbc, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_PKCS8_PRIV_KEY_INFO(__p0, __p1, __p2) \
	LP3(7824, PKCS8_PRIV_KEY_INFO *, d2i_PKCS8_PRIV_KEY_INFO, \
		PKCS8_PRIV_KEY_INFO **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_find_by_issuer_and_serial(__p0, __p1, __p2) \
	LP3(7722, X509 *, X509_find_by_issuer_and_serial, \
		STACK *, __p0, a0, \
		X509_NAME *, __p1, a1, \
		ASN1_INTEGER *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define sk_free(__p0) \
	LP1NR(5622, sk_free, \
		STACK *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_PKCS7_SIGNED(__p0, __p1) \
	LP2(3918, int , i2d_PKCS7_SIGNED, \
		PKCS7_SIGNED *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_write(__p0, __p1, __p2) \
	LP3(888, int , BIO_write, \
		BIO *, __p0, a0, \
		const char *, __p1, a1, \
		int , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_idea_cbc() \
	LP0(2778, EVP_CIPHER *, EVP_idea_cbc, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_ASN1_BIT_STRING(__p0, __p1) \
	LP2(228, int , i2d_ASN1_BIT_STRING, \
		ASN1_BIT_STRING *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_ASN1_OCTET_STRING(__p0, __p1, __p2) \
	LP3(336, ASN1_OCTET_STRING *, d2i_ASN1_OCTET_STRING, \
		ASN1_OCTET_STRING **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_PKEY_USAGE_PERIOD(__p0, __p1) \
	LP2(5934, int , i2d_PKEY_USAGE_PERIOD, \
		PKEY_USAGE_PERIOD *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_des_ede3_cbc() \
	LP0(2736, EVP_CIPHER *, EVP_des_ede3_cbc, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_CTX_use_PrivateKey_file(__p0, __p1, __p2) \
	LP3(4842, int , SSL_CTX_use_PrivateKey_file, \
		SSL_CTX *, __p0, a0, \
		const char *, __p1, a1, \
		int , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ASN1_BIT_STRING_set_bit(__p0, __p1, __p2) \
	LP3(240, int , ASN1_BIT_STRING_set_bit, \
		ASN1_BIT_STRING *, __p0, a0, \
		int , __p1, a1, \
		int , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_get_pw_prompt() \
	LP0(2412, char *, EVP_get_pw_prompt, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define sk_insert(__p0, __p1, __p2) \
	LP3(5634, int , sk_insert, \
		STACK *, __p0, a0, \
		char *, __p1, a1, \
		int , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ERR_free_strings() \
	LP0NR(2340, ERR_free_strings, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_get_pubkey_parameters(__p0, __p1) \
	LP2(6798, int , X509_get_pubkey_parameters, \
		EVP_PKEY *, __p0, a0, \
		STACK *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_f_buffer() \
	LP0(1014, BIO_METHOD *, BIO_f_buffer, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_DSA_SIG(__p0, __p1) \
	LP2(2100, int , i2d_DSA_SIG, \
		DSA_SIG *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_SealFinal(__p0, __p1, __p2) \
	LP3NR(2508, EVP_SealFinal, \
		EVP_CIPHER_CTX *, __p0, a0, \
		unsigned char *, __p1, a1, \
		int *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_get_error(__p0, __p1) \
	LP2(5172, int , SSL_get_error, \
		SSL *, __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define OBJ_NAME_get(__p0, __p1) \
	LP2(3174, const char *, OBJ_NAME_get, \
		const char *, __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_PKCS7_bio(__p0, __p1) \
	LP2(3852, int , i2d_PKCS7_bio, \
		BIO *, __p0, a0, \
		PKCS7 *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSLv23_server_method() \
	LP0(5232, SSL_METHOD *, SSLv23_server_method, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_mod_exp_recp(__p0, __p1, __p2, __p3, __p4) \
	LP5(1692, int , BN_mod_exp_recp, \
		BIGNUM *, __p0, a0, \
		const BIGNUM *, __p1, a1, \
		const BIGNUM *, __p2, a2, \
		const BIGNUM *, __p3, a3, \
		BN_CTX *, __p4, d0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PEM_write_bio_PrivateKey(__p0, __p1, __p2, __p3, __p4, __p5, __p6) \
	LP7(3504, int , PEM_write_bio_PrivateKey, \
		BIO *, __p0, a0, \
		EVP_PKEY *, __p1, a1, \
		EVP_CIPHER *, __p2, a2, \
		unsigned char *, __p3, a3, \
		int , __p4, d0, \
		pem_password_cb *, __p5, d1, \
		void *, __p6, d2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PEM_ASN1_read_bio(__p0, __p1, __p2, __p3, __p4, __p5) \
	LP6(3324, char *, PEM_ASN1_read_bio, \
		APTR , __p0, a0, \
		const char *, __p1, a1, \
		BIO *, __p2, a2, \
		char **, __p3, a3, \
		pem_password_cb *, __p4, d0, \
		void *, __p5, d1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_CipherUpdate(__p0, __p1, __p2, __p3, __p4) \
	LP5NR(2466, EVP_CipherUpdate, \
		EVP_CIPHER_CTX *, __p0, a0, \
		unsigned char *, __p1, a1, \
		int *, __p2, a2, \
		unsigned char *, __p3, a3, \
		int , __p4, d0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_CTX_use_certificate_chain_file(__p0, __p1) \
	LP2(4854, int , SSL_CTX_use_certificate_chain_file, \
		SSL_CTX *, __p0, a0, \
		const char *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_PKEY_free(__p0) \
	LP1NR(7110, X509_PKEY_free, \
		X509_PKEY *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_OpenInit(__p0, __p1, __p2, __p3, __p4, __p5) \
	LP6(2490, int , EVP_OpenInit, \
		EVP_CIPHER_CTX *, __p0, a0, \
		EVP_CIPHER *, __p1, a1, \
		unsigned char *, __p2, a2, \
		int , __p3, a3, \
		unsigned char *, __p4, d0, \
		EVP_PKEY *, __p5, d1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_RECP_CTX_set(__p0, __p1, __p2) \
	LP3(1680, int , BN_RECP_CTX_set, \
		BN_RECP_CTX *, __p0, a0, \
		const BIGNUM *, __p1, a1, \
		BN_CTX *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define NETSCAPE_SPKI_new() \
	LP0(7128, NETSCAPE_SPKI *, NETSCAPE_SPKI_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_use_PrivateKey_ASN1(__p0, __p1, __p2, __p3) \
	LP4(4800, int , SSL_use_PrivateKey_ASN1, \
		int , __p0, a0, \
		SSL *, __p1, a1, \
		unsigned char *, __p2, a2, \
		long , __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_dss() \
	LP0(2640, EVP_MD *, EVP_dss, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_CINF_free(__p0) \
	LP1NR(6990, X509_CINF_free, \
		X509_CINF *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_DISPLAYTEXT(__p0, __p1, __p2) \
	LP3(414, ASN1_STRING *, d2i_DISPLAYTEXT, \
		ASN1_STRING **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define sk_shift(__p0) \
	LP1(5670, char *, sk_shift, \
		STACK *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS5_PBE_keyivgen(__p0, __p1, __p2, __p3, __p4, __p5, __p6) \
	LP7(8118, int , PKCS5_PBE_keyivgen, \
		EVP_CIPHER_CTX *, __p0, a0, \
		const char *, __p1, a1, \
		int , __p2, a2, \
		ASN1_TYPE *, __p3, a3, \
		EVP_CIPHER *, __p4, d0, \
		EVP_MD *, __p5, d1, \
		int , __p6, d2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define NOTICEREF_free(__p0) \
	LP1NR(6144, NOTICEREF_free, \
		NOTICEREF *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_get_wbio(__p0) \
	LP1(4734, BIO *, SSL_get_wbio, \
		SSL *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_ASN1_T61STRING(__p0, __p1, __p2) \
	LP3(420, ASN1_T61STRING *, d2i_ASN1_T61STRING, \
		ASN1_T61STRING **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_MD_CTX_copy(__p0, __p1) \
	LP2(2376, int , EVP_MD_CTX_copy, \
		EVP_MD_CTX *, __p0, a0, \
		EVP_MD_CTX *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define a2i_ASN1_STRING(__p0, __p1, __p2, __p3) \
	LP4(522, int , a2i_ASN1_STRING, \
		BIO *, __p0, a0, \
		ASN1_STRING *, __p1, a1, \
		char *, __p2, a2, \
		int , __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define sk_dup(__p0) \
	LP1(5694, STACK *, sk_dup, \
		STACK *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define GENERAL_NAME_new() \
	LP0(5808, GENERAL_NAME *, GENERAL_NAME_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS12_init(__p0) \
	LP1(3624, PKCS12 *, PKCS12_init, \
		int , __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_sock_should_retry(__p0) \
	LP1(1026, int , BIO_sock_should_retry, \
		int , __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_BLINDING_invert(__p0, __p1, __p2) \
	LP3(1644, int , BN_BLINDING_invert, \
		BIGNUM *, __p0, a0, \
		BN_BLINDING *, __p1, a1, \
		BN_CTX *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_mod(__p0, __p1, __p2, __p3) \
	LP4(1290, int , BN_mod, \
		BIGNUM *, __p0, a0, \
		const BIGNUM *, __p1, a1, \
		const BIGNUM *, __p2, a2, \
		BN_CTX *, __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_state(__p0) \
	LP1(5490, int , SSL_state, \
		SSL *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_PKEY_bits(__p0) \
	LP1(2958, int , EVP_PKEY_bits, \
		EVP_PKEY *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_get_default_timeout(__p0) \
	LP1(5370, long , SSL_get_default_timeout, \
		SSL *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_DIST_POINT(__p0, __p1, __p2) \
	LP3(6186, DIST_POINT *, d2i_DIST_POINT, \
		DIST_POINT **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CRYPTO_set_locking_callback(__p0) \
	LP1NR(1866, CRYPTO_set_locking_callback, \
		APTR , __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509V3_set_conf_lhash(__p0, __p1) \
	LP2NR(6276, X509V3_set_conf_lhash, \
		X509V3_CTX *, __p0, a0, \
		LHASH *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_add_file_cert_subjects_to_stack(__p0, __p1) \
	LP2(4866, int , SSL_add_file_cert_subjects_to_stack, \
		STACK *, __p0, a0, \
		const char *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BASIC_CONSTRAINTS_new() \
	LP0(5784, BASIC_CONSTRAINTS *, BASIC_CONSTRAINTS_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_PKCS7_RECIP_INFO(__p0, __p1) \
	LP2(3894, int , i2d_PKCS7_RECIP_INFO, \
		PKCS7_RECIP_INFO *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define OBJ_dup(__p0) \
	LP1(3198, ASN1_OBJECT *, OBJ_dup, \
		ASN1_OBJECT *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CRYPTO_thread_id() \
	LP0(1902, unsigned long , CRYPTO_thread_id, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define RSA_set_ex_data(__p0, __p1, __p2) \
	LP3(4542, int , RSA_set_ex_data, \
		RSA *, __p0, a0, \
		int , __p1, a1, \
		char *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define COMP_CTX_new(__p0) \
	LP1(1734, COMP_CTX *, COMP_CTX_new, \
		COMP_METHOD *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SXNETID_new() \
	LP0(5862, SXNETID *, SXNETID_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define TXT_DB_create_index(__p0, __p1, __p2, __p3, __p4) \
	LP5(5742, int , TXT_DB_create_index, \
		TXT_DB *, __p0, a0, \
		int , __p1, a1, \
		APTR , __p2, a2, \
		APTR , __p3, a3, \
		APTR , __p4, d0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_mdc2() \
	LP0(2652, EVP_MD *, EVP_mdc2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS12_BAGS_free(__p0) \
	LP1NR(3696, PKCS12_BAGS_free, \
		PKCS12_BAGS *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_set_cipher_list(__p0, __p1) \
	LP2(4740, int , SSL_set_cipher_list, \
		SSL *, __p0, a0, \
		char *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CRYPTO_mem_leaks_cb(__p0) \
	LP1NR(2010, CRYPTO_mem_leaks_cb, \
		APTR , __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_X509_CINF(__p0, __p1, __p2) \
	LP3(7002, X509_CINF *, d2i_X509_CINF, \
		X509_CINF **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define RSA_sign(__p0, __p1, __p2, __p3, __p4, __p5) \
	LP6(4440, int , RSA_sign, \
		int , __p0, a0, \
		unsigned char *, __p1, a1, \
		unsigned int , __p2, a2, \
		unsigned char *, __p3, a3, \
		unsigned int *, __p4, d0, \
		RSA *, __p5, d1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_ASN1_bytes(__p0, __p1, __p2, __p3) \
	LP4(606, int , i2d_ASN1_bytes, \
		ASN1_STRING *, __p0, a0, \
		unsigned char **, __p1, a1, \
		int , __p2, a2, \
		int , __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS12_verify_mac(__p0, __p1, __p2) \
	LP3(3648, int , PKCS12_verify_mac, \
		PKCS12 *, __p0, a0, \
		const char *, __p1, a1, \
		int , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_DSAparams(__p0, __p1) \
	LP2(2208, int , i2d_DSAparams, \
		DSA *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_des_ede_cfb() \
	LP0(2694, EVP_CIPHER *, EVP_des_ede_cfb, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define USERNOTICE_free(__p0) \
	LP1NR(6120, USERNOTICE_free, \
		USERNOTICE *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS7_add_attribute(__p0, __p1, __p2, __p3) \
	LP4(4218, int , PKCS7_add_attribute, \
		PKCS7_SIGNER_INFO *, __p0, a0, \
		int , __p1, a1, \
		int , __p2, a2, \
		void *, __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_CTX_set_default_passwd_cb(__p0, __p1) \
	LP2NR(5088, SSL_CTX_set_default_passwd_cb, \
		SSL_CTX *, __p0, a0, \
		pem_password_cb *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS12_add_friendlyname_uni(__p0, __p1, __p2) \
	LP3(3582, int , PKCS12_add_friendlyname_uni, \
		PKCS12_SAFEBAG *, __p0, a0, \
		const unsigned char *, __p1, a1, \
		int , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_PublicKey(__p0, __p1) \
	LP2(2994, int , i2d_PublicKey, \
		EVP_PKEY *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_get_retry_BIO(__p0, __p1) \
	LP2(942, BIO *, BIO_get_retry_BIO, \
		BIO *, __p0, a0, \
		int *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_ASN1_OBJECT(__p0, __p1, __p2) \
	LP3(186, ASN1_OBJECT *, d2i_ASN1_OBJECT, \
		ASN1_OBJECT **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_idea_cfb() \
	LP0(2766, EVP_CIPHER *, EVP_idea_cfb, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_PKEY_assign(__p0, __p1, __p2) \
	LP3(2970, int , EVP_PKEY_assign, \
		EVP_PKEY *, __p0, a0, \
		int , __p1, a1, \
		char *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_CIPHER_param_to_asn1(__p0, __p1) \
	LP2(3042, int , EVP_CIPHER_param_to_asn1, \
		EVP_CIPHER_CTX *, __p0, a0, \
		ASN1_TYPE *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_CRL_get_ext_by_critical(__p0, __p1, __p2) \
	LP3(7602, int , X509_CRL_get_ext_by_critical, \
		X509_CRL *, __p0, a0, \
		int , __p1, a1, \
		int , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSLeay_add_all_algorithms() \
	LP0NR(2892, SSLeay_add_all_algorithms, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ASN1_TIME_set(__p0, __p1) \
	LP2(474, ASN1_TIME *, ASN1_TIME_set, \
		ASN1_TIME *, __p0, a0, \
		time_t , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_MD2_IsReal() \
	LP0(84, long , EVP_MD2_IsReal, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_CTX_use_certificate(__p0, __p1) \
	LP2(5076, int , SSL_CTX_use_certificate, \
		SSL_CTX *, __p0, a0, \
		X509 *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_MONT_CTX_init(__p0) \
	LP1NR(1584, BN_MONT_CTX_init, \
		BN_MONT_CTX *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_NAME_add_entry(__p0, __p1, __p2, __p3) \
	LP4(7458, int , X509_NAME_add_entry, \
		X509_NAME *, __p0, a0, \
		X509_NAME_ENTRY *, __p1, a1, \
		int , __p2, a2, \
		int , __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_SXNET(__p0, __p1, __p2) \
	LP3(5832, SXNET *, d2i_SXNET, \
		SXNET **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_des_ede3_cfb() \
	LP0(2700, EVP_CIPHER *, EVP_des_ede3_cfb, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_LOOKUP_by_fingerprint(__p0, __p1, __p2, __p3, __p4) \
	LP5(7980, int , X509_LOOKUP_by_fingerprint, \
		X509_LOOKUP *, __p0, a0, \
		int , __p1, a1, \
		unsigned char *, __p2, a2, \
		int , __p3, a3, \
		X509_OBJECT *, __p4, d0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_PKCS7_ISSUER_AND_SERIAL(__p0, __p1) \
	LP2(3822, int , i2d_PKCS7_ISSUER_AND_SERIAL, \
		PKCS7_ISSUER_AND_SERIAL *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ASN1_INTEGER_to_BN(__p0, __p1) \
	LP2(570, BIGNUM *, ASN1_INTEGER_to_BN, \
		ASN1_INTEGER *, __p0, a0, \
		BIGNUM *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_ASN1_PRINTABLE(__p0, __p1) \
	LP2(378, int , i2d_ASN1_PRINTABLE, \
		ASN1_STRING *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS7_ISSUER_AND_SERIAL_digest(__p0, __p1, __p2, __p3) \
	LP4(3834, int , PKCS7_ISSUER_AND_SERIAL_digest, \
		PKCS7_ISSUER_AND_SERIAL *, __p0, a0, \
		EVP_MD *, __p1, a1, \
		unsigned char *, __p2, a2, \
		unsigned int *, __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_EXTENSION_get_critical(__p0) \
	LP1(7710, int , X509_EXTENSION_get_critical, \
		X509_EXTENSION *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_ASN1_SET(__p0, __p1, __p2, __p3, __p4, __p5) \
	LP6(480, int , i2d_ASN1_SET, \
		STACK *, __p0, a0, \
		unsigned char **, __p1, a1, \
		APTR , __p2, a2, \
		int , __p3, a3, \
		int , __p4, d0, \
		int , __p5, d1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_dec2bn(__p0, __p1) \
	LP2(1506, int , BN_dec2bn, \
		BIGNUM **, __p0, a0, \
		const char *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define AUTHORITY_KEYID_free(__p0) \
	LP1NR(5928, AUTHORITY_KEYID_free, \
		AUTHORITY_KEYID *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_PKCS8_bio(__p0, __p1) \
	LP2(8214, int , i2d_PKCS8_bio, \
		BIO *, __p0, a0, \
		X509_SIG *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_get_ssl_method(__p0) \
	LP1(5286, SSL_METHOD *, SSL_get_ssl_method, \
		SSL *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_PrivateKey(__p0, __p1) \
	LP2(3006, int , i2d_PrivateKey, \
		EVP_PKEY *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_load_error_strings() \
	LP0NR(4884, SSL_load_error_strings, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509V3_add_value(__p0, __p1, __p2) \
	LP3(6312, int , X509V3_add_value, \
		const char *, __p0, a0, \
		const char *, __p1, a1, \
		STACK **, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define RAND_seed(__p0, __p1) \
	LP2NR(4278, RAND_seed, \
		const void *, __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_idea_ecb() \
	LP0(2760, EVP_CIPHER *, EVP_idea_ecb, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_X509_REQ_INFO(__p0, __p1) \
	LP2(6840, int , i2d_X509_REQ_INFO, \
		X509_REQ_INFO *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_PBKDF2PARAM(__p0, __p1) \
	LP2(7764, int , i2d_PBKDF2PARAM, \
		PBKDF2PARAM *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_CRL_verify(__p0, __p1) \
	LP2(6468, int , X509_CRL_verify, \
		X509_CRL *, __p0, a0, \
		EVP_PKEY *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define RAND_bytes(__p0, __p1) \
	LP2NR(4272, RAND_bytes, \
		unsigned char *, __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_debug_callback(__p0, __p1, __p2, __p3, __p4, __p5) \
	LP6(960, long , BIO_debug_callback, \
		BIO *, __p0, a0, \
		int , __p1, a1, \
		const char *, __p2, a2, \
		int , __p3, a3, \
		long , __p4, d0, \
		long , __p5, d1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ASN1_OBJECT_new() \
	LP0(168, ASN1_OBJECT *, ASN1_OBJECT_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_SSL_SESSION(__p0, __p1, __p2) \
	LP3(4998, SSL_SESSION *, d2i_SSL_SESSION, \
		SSL_SESSION **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_DSAPrivateKey_bio(__p0, __p1) \
	LP2(6576, DSA *, d2i_DSAPrivateKey_bio, \
		BIO *, __p0, a0, \
		DSA **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS12_setup_mac(__p0, __p1, __p2, __p3, __p4) \
	LP5(3660, int , PKCS12_setup_mac, \
		PKCS12 *, __p0, a0, \
		int , __p1, a1, \
		unsigned char *, __p2, a2, \
		int , __p3, a3, \
		EVP_MD *, __p4, d0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define RSA_set_default_method(__p0) \
	LP1NR(4368, RSA_set_default_method, \
		RSA_METHOD *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ERR_load_CONF_strings() \
	LP0NR(1794, ERR_load_CONF_strings, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_free(__p0) \
	LP1(870, int , BIO_free, \
		BIO *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ASN1_ENUMERATED_get(__p0) \
	LP1(582, long , ASN1_ENUMERATED_get, \
		ASN1_ENUMERATED *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_set_issuer_name(__p0, __p1) \
	LP2(7254, int , X509_set_issuer_name, \
		X509 *, __p0, a0, \
		X509_NAME *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS7_ctrl(__p0, __p1, __p2, __p3) \
	LP4(4086, long , PKCS7_ctrl, \
		PKCS7 *, __p0, a0, \
		int , __p1, a1, \
		long , __p2, a2, \
		char *, __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_CTX_set_default_passwd_cb_userdata(__p0, __p1) \
	LP2NR(8202, SSL_CTX_set_default_passwd_cb_userdata, \
		SSL_CTX *, __p0, a0, \
		void *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define DSA_do_verify(__p0, __p1, __p2, __p3) \
	LP4(2118, int , DSA_do_verify, \
		const unsigned char *, __p0, a0, \
		int , __p1, a1, \
		DSA_SIG *, __p2, a2, \
		DSA *, __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PEM_dek_info(__p0, __p1, __p2, __p3) \
	LP4NR(3396, PEM_dek_info, \
		char *, __p0, a0, \
		const char *, __p1, a1, \
		int , __p2, a2, \
		char *, __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define NETSCAPE_CERT_SEQUENCE_new() \
	LP0(7182, NETSCAPE_CERT_SEQUENCE *, NETSCAPE_CERT_SEQUENCE_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define RAND_file_name(__p0, __p1) \
	LP2(4296, char *, RAND_file_name, \
		char *, __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_PKEY_save_parameters(__p0, __p1) \
	LP2(3024, int , EVP_PKEY_save_parameters, \
		EVP_PKEY *, __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_get_default_cert_dir() \
	LP0(6666, const char *, X509_get_default_cert_dir, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_STORE_CTX_set_error(__p0, __p1) \
	LP2NR(8034, X509_STORE_CTX_set_error, \
		X509_STORE_CTX *, __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_des_IsReal() \
	LP0(72, long , EVP_des_IsReal, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_desx_cbc() \
	LP0(2742, EVP_CIPHER *, EVP_desx_cbc, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define sk_new(__p0) \
	LP1(5616, STACK *, sk_new, \
		APTR , __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_X509_PKEY(__p0, __p1, __p2) \
	LP3(7122, X509_PKEY *, d2i_X509_PKEY, \
		X509_PKEY **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_CTX_use_certificate_file(__p0, __p1, __p2) \
	LP3(4848, int , SSL_CTX_use_certificate_file, \
		SSL_CTX *, __p0, a0, \
		const char *, __p1, a1, \
		int , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS7_ENCRYPT_free(__p0) \
	LP1NR(4032, PKCS7_ENCRYPT_free, \
		PKCS7_ENCRYPT *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_div_word(__p0, __p1) \
	LP2(1320, BN_ULONG , BN_div_word, \
		BIGNUM *, __p0, a0, \
		BN_ULONG , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509V3_EXT_i2d(__p0, __p1, __p2) \
	LP3(6414, X509_EXTENSION *, X509V3_EXT_i2d, \
		int , __p0, a0, \
		int , __p1, a1, \
		void *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define DSA_generate_parameters(__p0, __p1, __p2, __p3, __p4, __p5, __p6) \
	LP7(2184, DSA *, DSA_generate_parameters, \
		int , __p0, a0, \
		unsigned char *, __p1, a1, \
		int , __p2, a2, \
		int *, __p3, a3, \
		unsigned long *, __p4, d0, \
		APTR , __p5, d1, \
		char *, __p6, d2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_PKCS7_SIGNER_INFO(__p0, __p1) \
	LP2(3870, int , i2d_PKCS7_SIGNER_INFO, \
		PKCS7_SIGNER_INFO *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define lh_doall_arg(__p0, __p1, __p2) \
	LP3NR(3132, lh_doall_arg, \
		LHASH *, __p0, a0, \
		APTR , __p1, a1, \
		char *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_PBE2PARAM(__p0, __p1) \
	LP2(7788, int , i2d_PBE2PARAM, \
		PBE2PARAM *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSLv3_method() \
	LP0(5208, SSL_METHOD *, SSLv3_method, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_PKCS12_MAC_DATA(__p0, __p1, __p2) \
	LP3(3738, PKCS12_MAC_DATA *, d2i_PKCS12_MAC_DATA, \
		PKCS12_MAC_DATA **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_rshift(__p0, __p1, __p2) \
	LP3(1440, int , BN_rshift, \
		BIGNUM *, __p0, a0, \
		BIGNUM *, __p1, a1, \
		int , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_NETSCAPE_SPKAC(__p0, __p1) \
	LP2(7164, int , i2d_NETSCAPE_SPKAC, \
		NETSCAPE_SPKAC *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_fd_should_retry(__p0) \
	LP1(1038, int , BIO_fd_should_retry, \
		int , __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PBEPARAM_free(__p0) \
	LP1NR(7752, PBEPARAM_free, \
		PBEPARAM *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_CTX_add_session(__p0, __p1) \
	LP2(4986, int , SSL_CTX_add_session, \
		SSL_CTX *, __p0, a0, \
		SSL_SESSION *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_CTX_set_tmp_rsa_callback(__p0, __p1) \
	LP2NR(5568, SSL_CTX_set_tmp_rsa_callback, \
		SSL_CTX *, __p0, a0, \
		APTR , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_mask_bits(__p0, __p1) \
	LP2(1416, int , BN_mask_bits, \
		BIGNUM *, __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PEM_write_bio_RSAPrivateKey(__p0, __p1, __p2, __p3, __p4, __p5, __p6) \
	LP7(3486, int , PEM_write_bio_RSAPrivateKey, \
		BIO *, __p0, a0, \
		RSA *, __p1, a1, \
		const EVP_CIPHER *, __p2, a2, \
		unsigned char *, __p3, a3, \
		int , __p4, d0, \
		pem_password_cb *, __p5, d1, \
		void *, __p6, d2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_get_host_ip(__p0, __p1) \
	LP2(1086, int , BIO_get_host_ip, \
		const char *, __p0, a0, \
		unsigned char *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_f_md() \
	LP0(2580, BIO_METHOD *, BIO_f_md, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_rshift1(__p0, __p1) \
	LP2(1446, int , BN_rshift1, \
		BIGNUM *, __p0, a0, \
		BIGNUM *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_clear_bit(__p0, __p1) \
	LP2(1482, int , BN_clear_bit, \
		BIGNUM *, __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_X509_CRL(__p0, __p1) \
	LP2(7092, int , i2d_X509_CRL, \
		X509_CRL *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_SESSION_set_ex_data(__p0, __p1, __p2) \
	LP3(5526, int , SSL_SESSION_set_ex_data, \
		SSL_SESSION *, __p0, a0, \
		int , __p1, a1, \
		void *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_s_fd() \
	LP0(990, BIO_METHOD *, BIO_s_fd, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_digest(__p0, __p1, __p2, __p3) \
	LP4(6504, int , X509_digest, \
		X509 *, __p0, a0, \
		EVP_MD *, __p1, a1, \
		unsigned char *, __p2, a2, \
		unsigned int *, __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_SESSION_free(__p0) \
	LP1NR(4968, SSL_SESSION_free, \
		SSL_SESSION *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ms_time_new() \
	LP0(5700, char *, ms_time_new, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define TLSv1_client_method() \
	LP0(5256, SSL_METHOD *, TLSv1_client_method, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS12_MAKE_SHKEYBAG(__p0, __p1, __p2, __p3, __p4, __p5, __p6) \
	LP7(3552, PKCS12_SAFEBAG *, PKCS12_MAKE_SHKEYBAG, \
		int , __p0, a0, \
		const char *, __p1, a1, \
		int , __p2, a2, \
		unsigned char *, __p3, a3, \
		int , __p4, d0, \
		int , __p5, d1, \
		PKCS8_PRIV_KEY_INFO *, __p6, d2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BUF_strdup(__p0) \
	LP1(1722, char *, BUF_strdup, \
		const char *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CRYPTO_dup_ex_data(__p0, __p1, __p2) \
	LP3(1830, int , CRYPTO_dup_ex_data, \
		STACK *, __p0, a0, \
		CRYPTO_EX_DATA *, __p1, a1, \
		CRYPTO_EX_DATA *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_USERNOTICE(__p0, __p1, __p2) \
	LP3(6114, USERNOTICE *, d2i_USERNOTICE, \
		USERNOTICE **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_set_quiet_shutdown(__p0, __p1) \
	LP2NR(5424, SSL_set_quiet_shutdown, \
		SSL *, __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_CTX_set_cert_store(__p0, __p1) \
	LP2NR(4626, SSL_CTX_set_cert_store, \
		SSL_CTX *, __p0, a0, \
		X509_STORE *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_get_ext(__p0, __p1) \
	LP2(7566, X509_EXTENSION *, X509_get_ext, \
		X509 *, __p0, a0, \
		int , __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_PKCS7(__p0, __p1, __p2) \
	LP3(4074, PKCS7 *, d2i_PKCS7, \
		PKCS7 **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define EVP_CIPHER_asn1_to_param(__p0, __p1) \
	LP2(3048, int , EVP_CIPHER_asn1_to_param, \
		EVP_CIPHER_CTX *, __p0, a0, \
		ASN1_TYPE *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_mod_inverse(__p0, __p1, __p2, __p3) \
	LP4(1518, BIGNUM *, BN_mod_inverse, \
		BIGNUM *, __p0, a0, \
		BIGNUM *, __p1, a1, \
		const BIGNUM *, __p2, a2, \
		BN_CTX *, __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_mod_exp2_mont(__p0, __p1, __p2, __p3, __p4, __p5, __p6, __p7) \
	LP8(1404, int , BN_mod_exp2_mont, \
		BIGNUM *, __p0, a0, \
		BIGNUM *, __p1, a1, \
		BIGNUM *, __p2, a2, \
		BIGNUM *, __p3, a3, \
		BIGNUM *, __p4, d0, \
		BIGNUM *, __p5, d1, \
		BN_CTX *, __p6, d2, \
		BN_MONT_CTX *, __p7, d3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PKCS7_dataFinal(__p0, __p1) \
	LP2(4146, int , PKCS7_dataFinal, \
		PKCS7 *, __p0, a0, \
		BIO *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_RSAPublicKey(__p0, __p1, __p2) \
	LP3(4392, RSA *, d2i_RSAPublicKey, \
		RSA **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define POLICYQUALINFO_free(__p0) \
	LP1NR(6096, POLICYQUALINFO_free, \
		POLICYQUALINFO *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ASN1_digest(__p0, __p1, __p2, __p3, __p4) \
	LP5(7224, int , ASN1_digest, \
		APTR , __p0, a0, \
		EVP_MD *, __p1, a1, \
		char *, __p2, a2, \
		unsigned char *, __p3, a3, \
		unsigned int *, __p4, d0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509v3_get_ext_by_critical(__p0, __p1, __p2) \
	LP3(7518, int , X509v3_get_ext_by_critical, \
		const STACK *, __p0, a0, \
		int , __p1, a1, \
		int , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_EXTENSION_set_object(__p0, __p1) \
	LP2(7680, int , X509_EXTENSION_set_object, \
		X509_EXTENSION *, __p0, a0, \
		ASN1_OBJECT *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_PKCS8_PRIV_KEY_INFO_bio(__p0, __p1) \
	LP2(8226, int , i2d_PKCS8_PRIV_KEY_INFO_bio, \
		BIO *, __p0, a0, \
		PKCS8_PRIV_KEY_INFO *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SSL_SESSION_cmp(__p0, __p1) \
	LP2(4956, int , SSL_SESSION_cmp, \
		SSL_SESSION *, __p0, a0, \
		SSL_SESSION *, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define d2i_ASN1_VISIBLESTRING(__p0, __p1, __p2) \
	LP3(348, ASN1_VISIBLESTRING *, d2i_ASN1_VISIBLESTRING, \
		ASN1_VISIBLESTRING **, __p0, a0, \
		unsigned char **, __p1, a1, \
		long , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define a2i_ASN1_ENUMERATED(__p0, __p1, __p2, __p3) \
	LP4(510, int , a2i_ASN1_ENUMERATED, \
		BIO *, __p0, a0, \
		ASN1_ENUMERATED *, __p1, a1, \
		char *, __p2, a2, \
		int , __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define COMP_expand_block(__p0, __p1, __p2, __p3, __p4) \
	LP5(1752, int , COMP_expand_block, \
		COMP_CTX *, __p0, a0, \
		unsigned char *, __p1, a1, \
		int , __p2, a2, \
		unsigned char *, __p3, a3, \
		int , __p4, d0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_LOOKUP_by_subject(__p0, __p1, __p2, __p3) \
	LP4(7968, int , X509_LOOKUP_by_subject, \
		X509_LOOKUP *, __p0, a0, \
		int , __p1, a1, \
		X509_NAME *, __p2, a2, \
		X509_OBJECT *, __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_NAME_hash(__p0) \
	LP1(7380, unsigned long , X509_NAME_hash, \
		X509_NAME *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_get_ext_by_NID(__p0, __p1, __p2) \
	LP3(7548, int , X509_get_ext_by_NID, \
		X509 *, __p0, a0, \
		int , __p1, a1, \
		int , __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BIO_set_ex_data(__p0, __p1, __p2) \
	LP3(822, int , BIO_set_ex_data, \
		BIO *, __p0, a0, \
		int , __p1, a1, \
		char *, __p2, a2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define i2d_ASN1_ENUMERATED(__p0, __p1) \
	LP2(282, int , i2d_ASN1_ENUMERATED, \
		ASN1_ENUMERATED *, __p0, a0, \
		unsigned char **, __p1, a1, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BN_mul(__p0, __p1, __p2, __p3) \
	LP4(1302, int , BN_mul, \
		BIGNUM *, __p0, a0, \
		BIGNUM *, __p1, a1, \
		BIGNUM *, __p2, a2, \
		BN_CTX *, __p3, a3, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define RSA_padding_check_PKCS1_OAEP(__p0, __p1, __p2, __p3, __p4, __p5, __p6) \
	LP7(4506, int , RSA_padding_check_PKCS1_OAEP, \
		unsigned char *, __p0, a0, \
		int , __p1, a1, \
		unsigned char *, __p2, a2, \
		int , __p3, a3, \
		int , __p4, d0, \
		unsigned char *, __p5, d1, \
		int , __p6, d2, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define X509_CRL_dup(__p0) \
	LP1(6606, X509_CRL *, X509_CRL_dup, \
		X509_CRL *, __p0, a0, \
		, AMISSL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#ifdef USE_INLINE_STDARG

#include <stdarg.h>

#define InitAmiSSL(...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	InitAmiSSLA((struct TagItem *)_tags);})

#define ERR_add_error_data(__p0, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	ERR_add_error_dataA(__p0, (void *)_tags);})

#define CleanupAmiSSL(...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	CleanupAmiSSLA((struct TagItem *)_tags);})

#define BIO_printf(__p0, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	BIO_printfA(__p0, (void *)_tags);})

#endif

#endif /* !_PPCINLINE_AMISSL_H */
