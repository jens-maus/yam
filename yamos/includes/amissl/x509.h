/* crypto/x509/x509.h */
/* Copyright (C) 1995-1998 Eric Young (eay@cryptsoft.com)
 * All rights reserved.
 *
 * This package is an SSL implementation written
 * by Eric Young (eay@cryptsoft.com).
 * The implementation was written so as to conform with Netscapes SSL.
 * 
 * This library is free for commercial and non-commercial use as long as
 * the following conditions are aheared to.  The following conditions
 * apply to all code found in this distribution, be it the RC4, RSA,
 * lhash, DES, etc., code; not just the SSL code.  The SSL documentation
 * included with this distribution is covered by the same copyright terms
 * except that the holder is Tim Hudson (tjh@cryptsoft.com).
 * 
 * Copyright remains Eric Young's, and as such any Copyright notices in
 * the code are not to be removed.
 * If this package is used in a product, Eric Young should be given attribution
 * as the author of the parts of the library used.
 * This can be in the form of a textual message at program startup or
 * in documentation (online or textual) provided with the package.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    "This product includes cryptographic software written by
 *     Eric Young (eay@cryptsoft.com)"
 *    The word 'cryptographic' can be left out if the rouines from the library
 *    being used are not cryptographic related :-).
 * 4. If you include any Windows specific code (or a derivative thereof) from 
 *    the apps directory (application code) you must include an acknowledgement:
 *    "This product includes software written by Tim Hudson (tjh@cryptsoft.com)"
 * 
 * THIS SOFTWARE IS PROVIDED BY ERIC YOUNG ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * The licence and distribution terms for any publically available version or
 * derivative of this code cannot be changed.  i.e. this code cannot simply be
 * copied and put under another distribution licence
 * [including the GNU Public Licence.]
 */

#ifndef HEADER_X509_H
#define HEADER_X509_H

#ifdef  __cplusplus
extern "C" {
#endif

#include <amissl/stack.h>
#include <amissl/asn1.h>
#include <amissl/safestack.h>

#include <amissl/buffer.h>
#include <amissl/evp.h>
#include <amissl/bn.h>

#include <amissl/rsa.h>
#include <amissl/dsa.h>
#include <amissl/dh.h>
#include <amissl/evp.h>

#define X509_FILETYPE_PEM	1
#define X509_FILETYPE_ASN1	2
#define X509_FILETYPE_DEFAULT	3

#define X509v3_KU_DIGITAL_SIGNATURE	0x0080
#define X509v3_KU_NON_REPUDIATION	0x0040
#define X509v3_KU_KEY_ENCIPHERMENT	0x0020
#define X509v3_KU_DATA_ENCIPHERMENT	0x0010
#define X509v3_KU_KEY_AGREEMENT		0x0008
#define X509v3_KU_KEY_CERT_SIGN		0x0004
#define X509v3_KU_CRL_SIGN		0x0002
#define X509v3_KU_ENCIPHER_ONLY		0x0001
#define X509v3_KU_DECIPHER_ONLY		0x8000
#define X509v3_KU_UNDEF			0xffff

typedef struct X509_objects_st
	{
	int nid;
	int (*a2i)();
	int (*i2a)();
	} X509_OBJECTS;

typedef struct X509_algor_st
	{
	ASN1_OBJECT *algorithm;
	ASN1_TYPE *parameter;
	} X509_ALGOR;

DECLARE_STACK_OF(X509_ALGOR)
DECLARE_ASN1_SET_OF(X509_ALGOR)

typedef struct X509_val_st
	{
	ASN1_UTCTIME *notBefore;
	ASN1_UTCTIME *notAfter;
	} X509_VAL;

typedef struct X509_pubkey_st
	{
	X509_ALGOR *algor;
	ASN1_BIT_STRING *public_key;
	EVP_PKEY *pkey;
	} X509_PUBKEY;

typedef struct X509_sig_st
	{
	X509_ALGOR *algor;
	ASN1_OCTET_STRING *digest;
	} X509_SIG;

typedef struct X509_name_entry_st
	{
	ASN1_OBJECT *object;
	ASN1_STRING *value;
	int set;
	int size; 	/* temp variable */
	} X509_NAME_ENTRY;

DECLARE_STACK_OF(X509_NAME_ENTRY)
DECLARE_ASN1_SET_OF(X509_NAME_ENTRY)

/* we always keep X509_NAMEs in 2 forms. */
typedef struct X509_name_st
	{
	STACK_OF(X509_NAME_ENTRY) *entries;
	int modified;	/* true if 'bytes' needs to be built */
	BUF_MEM *bytes;
	unsigned long hash; /* Keep the hash around for lookups */
	} X509_NAME;

DECLARE_STACK_OF(X509_NAME)

#define X509_EX_V_NETSCAPE_HACK		0x8000
#define X509_EX_V_INIT			0x0001
typedef struct X509_extension_st
	{
	ASN1_OBJECT *object;
	short critical;
	short netscape_hack;
	ASN1_OCTET_STRING *value;
	long argl;			/* used when decoding */
	char *argp;			/* used when decoding */
	void (*ex_free)();		/* clear argp stuff */
	} X509_EXTENSION;

DECLARE_STACK_OF(X509_EXTENSION)
DECLARE_ASN1_SET_OF(X509_EXTENSION)

/* a sequence of these are used */
typedef struct x509_attributes_st
	{
	ASN1_OBJECT *object;
	int set; /* 1 for a set, 0 for a single item (which is wrong) */
	union	{
		char		*ptr;
/* 1 */		STACK_OF(ASN1_TYPE) *set;
/* 0 */		ASN1_TYPE	*single;
		} value;
	} X509_ATTRIBUTE;

DECLARE_STACK_OF(X509_ATTRIBUTE)
DECLARE_ASN1_SET_OF(X509_ATTRIBUTE)

typedef struct X509_req_info_st
	{
	ASN1_INTEGER *version;
	X509_NAME *subject;
	X509_PUBKEY *pubkey;
	/*  d=2 hl=2 l=  0 cons: cont: 00 */
	STACK_OF(X509_ATTRIBUTE) *attributes; /* [ 0 ] */
	int req_kludge;
	} X509_REQ_INFO;

typedef struct X509_req_st
	{
	X509_REQ_INFO *req_info;
	X509_ALGOR *sig_alg;
	ASN1_BIT_STRING *signature;
	int references;
	} X509_REQ;

typedef struct x509_cinf_st
	{
	ASN1_INTEGER *version;		/* [ 0 ] default of v1 */
	ASN1_INTEGER *serialNumber;
	X509_ALGOR *signature;
	X509_NAME *issuer;
	X509_VAL *validity;
	X509_NAME *subject;
	X509_PUBKEY *key;
	ASN1_BIT_STRING *issuerUID;		/* [ 1 ] optional in v2 */
	ASN1_BIT_STRING *subjectUID;		/* [ 2 ] optional in v2 */
	STACK_OF(X509_EXTENSION) *extensions;	/* [ 3 ] optional in v3 */
	} X509_CINF;

typedef struct x509_st
	{
	X509_CINF *cert_info;
	X509_ALGOR *sig_alg;
	ASN1_BIT_STRING *signature;
	int valid;
	int references;
	char *name;
	} X509;

DECLARE_STACK_OF(X509)
DECLARE_ASN1_SET_OF(X509)

typedef struct X509_revoked_st
	{
	ASN1_INTEGER *serialNumber;
	ASN1_UTCTIME *revocationDate;
	STACK_OF(X509_EXTENSION) /* optional */ *extensions;
	int sequence; /* load sequence */
	} X509_REVOKED;

DECLARE_STACK_OF(X509_REVOKED)
DECLARE_ASN1_SET_OF(X509_REVOKED)

typedef struct X509_crl_info_st
	{
	ASN1_INTEGER *version;
	X509_ALGOR *sig_alg;
	X509_NAME *issuer;
	ASN1_UTCTIME *lastUpdate;
	ASN1_UTCTIME *nextUpdate;
	STACK_OF(X509_REVOKED) *revoked;
	STACK_OF(X509_EXTENSION) /* [0] */ *extensions;
	} X509_CRL_INFO;

typedef struct X509_crl_st
	{
	/* actual signature */
	X509_CRL_INFO *crl;
	X509_ALGOR *sig_alg;
	ASN1_BIT_STRING *signature;
	int references;
	} X509_CRL;

DECLARE_STACK_OF(X509_CRL)
DECLARE_ASN1_SET_OF(X509_CRL)

typedef struct private_key_st
	{
	int version;
	/* The PKCS#8 data types */
	X509_ALGOR *enc_algor;
	ASN1_OCTET_STRING *enc_pkey;	/* encrypted pub key */

	/* When decrypted, the following will not be NULL */
	EVP_PKEY *dec_pkey;

	/* used to encrypt and decrypt */
	int key_length;
	char *key_data;
	int key_free;	/* true if we should auto free key_data */

	/* expanded version of 'enc_algor' */
	EVP_CIPHER_INFO cipher;

	int references;
	} X509_PKEY;

typedef struct X509_info_st
	{
	X509 *x509;
	X509_CRL *crl;
	X509_PKEY *x_pkey;

	EVP_CIPHER_INFO enc_cipher;
	int enc_len;
	char *enc_data;

	int references;
	} X509_INFO;

DECLARE_STACK_OF(X509_INFO)

/* The next 2 structures and their 8 routines were sent to me by
 * Pat Richard <patr@x509.com> and are used to manipulate
 * Netscapes spki strucutres - usefull if you are writing a CA web page
 */
typedef struct Netscape_spkac_st
	{
	X509_PUBKEY *pubkey;
	ASN1_IA5STRING *challenge;	/* challenge sent in atlas >= PR2 */
	} NETSCAPE_SPKAC;

typedef struct Netscape_spki_st
	{
	NETSCAPE_SPKAC *spkac;	/* signed public key and challenge */
	X509_ALGOR *sig_algor;
	ASN1_BIT_STRING *signature;
	} NETSCAPE_SPKI;

/* Netscape certificate sequence structure */
typedef struct Netscape_certificate_sequence
	{
	ASN1_OBJECT *type;
	STACK_OF(X509) *certs;
	} NETSCAPE_CERT_SEQUENCE;

typedef struct CBCParameter_st
	{
	unsigned char iv[8];
	} CBC_PARAM;

/* Password based encryption structure */

typedef struct PBEPARAM_st {
ASN1_OCTET_STRING *salt;
ASN1_INTEGER *iter;
} PBEPARAM;

/* Password based encryption V2 structures */

typedef struct PBE2PARAM_st {
X509_ALGOR *keyfunc;
X509_ALGOR *encryption;
} PBE2PARAM;

typedef struct PBKDF2PARAM_st {
ASN1_TYPE *salt;	/* Usually OCTET STRING but could be anything */
ASN1_INTEGER *iter;
ASN1_INTEGER *keylength;
X509_ALGOR *prf;
} PBKDF2PARAM;


/* PKCS#8 private key info structure */

typedef struct pkcs8_priv_key_info_st
        {
        int broken;     /* Flag for various broken formats */
#define PKCS8_OK        0
#define PKCS8_NO_OCTET  1
        ASN1_INTEGER *version;
        X509_ALGOR *pkeyalg;
        ASN1_TYPE *pkey; /* Should be OCTET STRING but some are broken */
        STACK_OF(X509_ATTRIBUTE) *attributes;
        } PKCS8_PRIV_KEY_INFO;

#include <amissl/x509_vfy.h>
#include <amissl/pkcs7.h>

#define X509_EXT_PACK_UNKNOWN	1
#define X509_EXT_PACK_STRING	2

#define		X509_get_version(x) ASN1_INTEGER_get((x)->cert_info->version)
/* #define	X509_get_serialNumber(x) ((x)->cert_info->serialNumber) */
#define		X509_get_notBefore(x) ((x)->cert_info->validity->notBefore)
#define		X509_get_notAfter(x) ((x)->cert_info->validity->notAfter)
#define		X509_extract_key(x)	X509_get_pubkey(x) /*****/
#define		X509_REQ_get_version(x) ASN1_INTEGER_get((x)->req_info->version)
#define		X509_REQ_get_subject_name(x) ((x)->req_info->subject)
#define		X509_REQ_extract_key(a)	X509_REQ_get_pubkey(a)
#define		X509_name_cmp(a,b)	X509_NAME_cmp((a),(b))
#define		X509_get_signature_type(x) EVP_PKEY_type(OBJ_obj2nid((x)->sig_alg->algorithm))

#define		X509_CRL_get_version(x) ASN1_INTEGER_get((x)->crl->version)
#define 	X509_CRL_get_lastUpdate(x) ((x)->crl->lastUpdate)
#define 	X509_CRL_get_nextUpdate(x) ((x)->crl->nextUpdate)
#define		X509_CRL_get_issuer(x) ((x)->crl->issuer)
#define		X509_CRL_get_REVOKED(x) ((x)->crl->revoked)

/* This one is only used so that a binary form can output, as in
 * i2d_X509_NAME(X509_get_X509_PUBKEY(x),&buf) */
#define 	X509_get_X509_PUBKEY(x) ((x)->cert_info->key)

/* BEGIN ERROR CODES */
/* The following lines are auto generated by the script mkerr.pl. Any changes
 * made after this point may be overwritten when the script is next run.
 */

/* Error codes for the X509 functions. */

/* Function codes. */
#define X509_F_ADD_CERT_DIR				 100
#define X509_F_BY_FILE_CTRL				 101
#define X509_F_DIR_CTRL					 102
#define X509_F_GET_CERT_BY_SUBJECT			 103
#define X509_F_X509V3_ADD_EXT				 104
#define X509_F_X509_CHECK_PRIVATE_KEY			 128
#define X509_F_X509_EXTENSION_CREATE_BY_NID		 108
#define X509_F_X509_EXTENSION_CREATE_BY_OBJ		 109
#define X509_F_X509_GET_PUBKEY_PARAMETERS		 110
#define X509_F_X509_LOAD_CERT_FILE			 111
#define X509_F_X509_LOAD_CRL_FILE			 112
#define X509_F_X509_NAME_ADD_ENTRY			 113
#define X509_F_X509_NAME_ENTRY_CREATE_BY_NID		 114
#define X509_F_X509_NAME_ENTRY_SET_OBJECT		 115
#define X509_F_X509_NAME_ONELINE			 116
#define X509_F_X509_NAME_PRINT				 117
#define X509_F_X509_PRINT_FP				 118
#define X509_F_X509_PUBKEY_GET				 119
#define X509_F_X509_PUBKEY_SET				 120
#define X509_F_X509_REQ_PRINT				 121
#define X509_F_X509_REQ_PRINT_FP			 122
#define X509_F_X509_REQ_TO_X509				 123
#define X509_F_X509_STORE_ADD_CERT			 124
#define X509_F_X509_STORE_ADD_CRL			 125
#define X509_F_X509_TO_X509_REQ				 126
#define X509_F_X509_VERIFY_CERT				 127

/* Reason codes. */
#define X509_R_BAD_X509_FILETYPE			 100
#define X509_R_CANT_CHECK_DH_KEY			 114
#define X509_R_CERT_ALREADY_IN_HASH_TABLE		 101
#define X509_R_ERR_ASN1_LIB				 102
#define X509_R_INVALID_DIRECTORY			 113
#define X509_R_KEY_TYPE_MISMATCH			 115
#define X509_R_KEY_VALUES_MISMATCH			 116
#define X509_R_LOADING_CERT_DIR				 103
#define X509_R_LOADING_DEFAULTS				 104
#define X509_R_NO_CERT_SET_FOR_US_TO_VERIFY		 105
#define X509_R_SHOULD_RETRY				 106
#define X509_R_UNABLE_TO_FIND_PARAMETERS_IN_CHAIN	 107
#define X509_R_UNABLE_TO_GET_CERTS_PUBLIC_KEY		 108
#define X509_R_UNKNOWN_KEY_TYPE				 117
#define X509_R_UNKNOWN_NID				 109
#define X509_R_UNSUPPORTED_ALGORITHM			 111
#define X509_R_WRONG_LOOKUP_TYPE			 112

#ifdef  __cplusplus
}
#endif
#endif

