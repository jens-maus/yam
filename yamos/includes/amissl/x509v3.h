/* x509v3.h */
/* Written by Dr Stephen N Henson (shenson@bigfoot.com) for the OpenSSL
 * project 1999.
 */
/* ====================================================================
 * Copyright (c) 1999 The OpenSSL Project.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit. (http://www.OpenSSL.org/)"
 *
 * 4. The names "OpenSSL Toolkit" and "OpenSSL Project" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For written permission, please contact
 *    licensing@OpenSSL.org.
 *
 * 5. Products derived from this software may not be called "OpenSSL"
 *    nor may "OpenSSL" appear in their names without prior written
 *    permission of the OpenSSL Project.
 *
 * 6. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit (http://www.OpenSSL.org/)"
 *
 * THIS SOFTWARE IS PROVIDED BY THE OpenSSL PROJECT ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE OpenSSL PROJECT OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * ====================================================================
 *
 * This product includes cryptographic software written by Eric Young
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
 *
 */
#ifndef HEADER_X509V3_H
#define HEADER_X509V3_H

#ifdef __cplusplus
extern "C" {
#endif

#include <amissl/bio.h>
#include <amissl/x509.h>
#include <amissl/conf.h>

/* Forward reference */
struct v3_ext_method;
struct v3_ext_ctx;

/* Useful typedefs */

typedef void * (*X509V3_EXT_NEW)(void);
typedef void (*X509V3_EXT_FREE)(void *);
typedef void * (*X509V3_EXT_D2I)(void *, unsigned char ** , long);
typedef int (*X509V3_EXT_I2D)(void *, unsigned char **);
typedef STACK_OF(CONF_VALUE) * (*X509V3_EXT_I2V)(struct v3_ext_method *method, void *ext, STACK_OF(CONF_VALUE) *extlist);
typedef void * (*X509V3_EXT_V2I)(struct v3_ext_method *method, struct v3_ext_ctx *ctx, STACK_OF(CONF_VALUE) *values);
typedef char * (*X509V3_EXT_I2S)(struct v3_ext_method *method, void *ext);
typedef void * (*X509V3_EXT_S2I)(struct v3_ext_method *method, struct v3_ext_ctx *ctx, char *str);
typedef int (*X509V3_EXT_I2R)(struct v3_ext_method *method, void *ext, BIO *out, int indent);
typedef void * (*X509V3_EXT_R2I)(struct v3_ext_method *method, struct v3_ext_ctx *ctx, char *str);

/* V3 extension structure */

struct v3_ext_method {
int ext_nid;
int ext_flags;
X509V3_EXT_NEW ext_new;
X509V3_EXT_FREE ext_free;
X509V3_EXT_D2I d2i;
X509V3_EXT_I2D i2d;

/* The following pair is used for string extensions */
X509V3_EXT_I2S i2s;
X509V3_EXT_S2I s2i;

/* The following pair is used for multi-valued extensions */
X509V3_EXT_I2V i2v;
X509V3_EXT_V2I v2i;

/* The following are used for raw extensions */
X509V3_EXT_I2R i2r;
X509V3_EXT_R2I r2i;

void *usr_data;	/* Any extension specific data */
};

typedef struct X509V3_CONF_METHOD_st {
char * (*get_string)(void *db, char *section, char *value);
STACK_OF(CONF_VALUE) * (*get_section)(void *db, char *section);
void (*free_string)(void *db, char * string);
void (*free_section)(void *db, STACK_OF(CONF_VALUE) *section);
} X509V3_CONF_METHOD;

/* Context specific info */
struct v3_ext_ctx {
#define CTX_TEST 0x1
int flags;
X509 *issuer_cert;
X509 *subject_cert;
X509_REQ *subject_req;
X509_CRL *crl;
X509V3_CONF_METHOD *db_meth;
void *db;
/* Maybe more here */
};

typedef struct v3_ext_method X509V3_EXT_METHOD;
typedef struct v3_ext_ctx X509V3_CTX;

/* ext_flags values */
#define X509V3_EXT_DYNAMIC	0x1
#define X509V3_EXT_CTX_DEP	0x2
#define X509V3_EXT_MULTILINE	0x4

typedef struct BIT_STRING_BITNAME_st {
int bitnum;
const char *lname;
const char *sname;
} BIT_STRING_BITNAME;

typedef BIT_STRING_BITNAME ENUMERATED_NAMES;

typedef struct BASIC_CONSTRAINTS_st {
int ca;
ASN1_INTEGER *pathlen;
} BASIC_CONSTRAINTS;


typedef struct PKEY_USAGE_PERIOD_st {
ASN1_GENERALIZEDTIME *notBefore;
ASN1_GENERALIZEDTIME *notAfter;
} PKEY_USAGE_PERIOD;

typedef struct GENERAL_NAME_st {

#define GEN_OTHERNAME	(0|V_ASN1_CONTEXT_SPECIFIC)
#define GEN_EMAIL	(1|V_ASN1_CONTEXT_SPECIFIC)
#define GEN_DNS		(2|V_ASN1_CONTEXT_SPECIFIC)
#define GEN_X400	(3|V_ASN1_CONTEXT_SPECIFIC)
#define GEN_DIRNAME	(4|V_ASN1_CONTEXT_SPECIFIC)
#define GEN_EDIPARTY	(5|V_ASN1_CONTEXT_SPECIFIC)
#define GEN_URI		(6|V_ASN1_CONTEXT_SPECIFIC)
#define GEN_IPADD	(7|V_ASN1_CONTEXT_SPECIFIC)
#define GEN_RID		(8|V_ASN1_CONTEXT_SPECIFIC)

int type;
union {
	char *ptr;
	ASN1_IA5STRING *ia5;/* rfc822Name, dNSName, uniformResourceIdentifier */
	ASN1_OCTET_STRING *ip; /* iPAddress */
	X509_NAME *dirn;		/* dirn */
	ASN1_OBJECT *rid; /* registeredID */
	ASN1_TYPE *other; /* otherName, ediPartyName, x400Address */
} d;
} GENERAL_NAME;

DECLARE_STACK_OF(GENERAL_NAME)
DECLARE_ASN1_SET_OF(GENERAL_NAME)

typedef struct DIST_POINT_NAME_st {
/* NB: this is a CHOICE type and only one of these should be set */
STACK_OF(GENERAL_NAME) *fullname;
X509_NAME *relativename;
} DIST_POINT_NAME;

typedef struct DIST_POINT_st {
DIST_POINT_NAME	*distpoint;
ASN1_BIT_STRING *reasons;
STACK_OF(GENERAL_NAME) *CRLissuer;
} DIST_POINT;

DECLARE_STACK_OF(DIST_POINT)
DECLARE_ASN1_SET_OF(DIST_POINT)

typedef struct AUTHORITY_KEYID_st {
ASN1_OCTET_STRING *keyid;
STACK_OF(GENERAL_NAME) *issuer;
ASN1_INTEGER *serial;
} AUTHORITY_KEYID;

/* Strong extranet structures */

typedef struct SXNET_ID_st {
	ASN1_INTEGER *zone;
	ASN1_OCTET_STRING *user;
} SXNETID;

DECLARE_STACK_OF(SXNETID)
DECLARE_ASN1_SET_OF(SXNETID)

typedef struct SXNET_st {
	ASN1_INTEGER *version;
	STACK_OF(SXNETID) *ids;
} SXNET;

typedef struct NOTICEREF_st {
	ASN1_STRING *organization;
	STACK *noticenos;
} NOTICEREF;

typedef struct USERNOTICE_st {
	NOTICEREF *noticeref;
	ASN1_STRING *exptext;
} USERNOTICE;

typedef struct POLICYQUALINFO_st {
	ASN1_OBJECT *pqualid;
	union {
		ASN1_IA5STRING *cpsuri;
		USERNOTICE *usernotice;
		ASN1_TYPE *other;
	} d;
} POLICYQUALINFO;

DECLARE_STACK_OF(POLICYQUALINFO)
DECLARE_ASN1_SET_OF(POLICYQUALINFO)

typedef struct POLICYINFO_st {
	ASN1_OBJECT *policyid;
	STACK_OF(POLICYQUALINFO) *qualifiers;
} POLICYINFO;

DECLARE_STACK_OF(POLICYINFO)
DECLARE_ASN1_SET_OF(POLICYINFO)

#define X509V3_conf_err(val) ERR_add_error_data(6, "section:", val->section, \
",name:", val->name, ",value:", val->value);

#define X509V3_set_ctx_test(ctx) \
			X509V3_set_ctx(ctx, NULL, NULL, NULL, NULL, CTX_TEST)
#define X509V3_set_ctx_nodb(ctx) ctx->db = NULL;

#define EXT_BITSTRING(nid, table) { nid, 0, \
			(X509V3_EXT_NEW)asn1_bit_string_new, \
			(X509V3_EXT_FREE)ASN1_STRING_free, \
			(X509V3_EXT_D2I)d2i_ASN1_BIT_STRING, \
			(X509V3_EXT_I2D)i2d_ASN1_BIT_STRING, \
			NULL, NULL, \
			(X509V3_EXT_I2V)i2v_ASN1_BIT_STRING, \
			(X509V3_EXT_V2I)v2i_ASN1_BIT_STRING, \
			NULL, NULL, \
			(char *)table}

#define EXT_IA5STRING(nid) { nid, 0, \
			(X509V3_EXT_NEW)ia5string_new, \
			(X509V3_EXT_FREE)ASN1_STRING_free, \
			(X509V3_EXT_D2I)d2i_ASN1_IA5STRING, \
			(X509V3_EXT_I2D)i2d_ASN1_IA5STRING, \
			(X509V3_EXT_I2S)i2s_ASN1_IA5STRING, \
			(X509V3_EXT_S2I)s2i_ASN1_IA5STRING, \
			NULL, NULL, NULL, NULL, \
			NULL}

#define EXT_END { -1, 0, NULL, NULL, NULL, NULL, NULL, NULL, \
			 NULL, NULL, NULL, NULL, \
			 NULL}

/* BEGIN ERROR CODES */
/* The following lines are auto generated by the script mkerr.pl. Any changes
 * made after this point may be overwritten when the script is next run.
 */

/* Error codes for the X509V3 functions. */

/* Function codes. */
#define X509V3_F_COPY_EMAIL				 122
#define X509V3_F_COPY_ISSUER				 123
#define X509V3_F_DO_EXT_CONF				 124
#define X509V3_F_DO_EXT_I2D				 135
#define X509V3_F_HEX_TO_STRING				 111
#define X509V3_F_I2S_ASN1_ENUMERATED			 121
#define X509V3_F_I2S_ASN1_INTEGER			 120
#define X509V3_F_NOTICE_SECTION				 132
#define X509V3_F_NREF_NOS				 133
#define X509V3_F_POLICY_SECTION				 131
#define X509V3_F_R2I_CERTPOL				 130
#define X509V3_F_S2I_ASN1_IA5STRING			 100
#define X509V3_F_S2I_ASN1_INTEGER			 108
#define X509V3_F_S2I_ASN1_OCTET_STRING			 112
#define X509V3_F_S2I_ASN1_SKEY_ID			 114
#define X509V3_F_S2I_S2I_SKEY_ID			 115
#define X509V3_F_STRING_TO_HEX				 113
#define X509V3_F_SXNET_ADD_ASC				 125
#define X509V3_F_SXNET_ADD_ID_INTEGER			 126
#define X509V3_F_SXNET_ADD_ID_ULONG			 127
#define X509V3_F_SXNET_GET_ID_ASC			 128
#define X509V3_F_SXNET_GET_ID_ULONG			 129
#define X509V3_F_V2I_ASN1_BIT_STRING			 101
#define X509V3_F_V2I_AUTHORITY_KEYID			 119
#define X509V3_F_V2I_BASIC_CONSTRAINTS			 102
#define X509V3_F_V2I_CRLD				 134
#define X509V3_F_V2I_EXT_KU				 103
#define X509V3_F_V2I_GENERAL_NAME			 117
#define X509V3_F_V2I_GENERAL_NAMES			 118
#define X509V3_F_V3_GENERIC_EXTENSION			 116
#define X509V3_F_X509V3_ADD_VALUE			 105
#define X509V3_F_X509V3_EXT_ADD				 104
#define X509V3_F_X509V3_EXT_ADD_ALIAS			 106
#define X509V3_F_X509V3_EXT_CONF			 107
#define X509V3_F_X509V3_EXT_I2D				 136
#define X509V3_F_X509V3_GET_VALUE_BOOL			 110
#define X509V3_F_X509V3_PARSE_LIST			 109

/* Reason codes. */
#define X509V3_R_BAD_IP_ADDRESS				 118
#define X509V3_R_BAD_OBJECT				 119
#define X509V3_R_BN_DEC2BN_ERROR			 100
#define X509V3_R_BN_TO_ASN1_INTEGER_ERROR		 101
#define X509V3_R_DUPLICATE_ZONE_ID			 133
#define X509V3_R_ERROR_CONVERTING_ZONE			 131
#define X509V3_R_ERROR_IN_EXTENSION			 128
#define X509V3_R_EXPECTED_A_SECTION_NAME		 137
#define X509V3_R_EXTENSION_NAME_ERROR			 115
#define X509V3_R_EXTENSION_NOT_FOUND			 102
#define X509V3_R_EXTENSION_SETTING_NOT_SUPPORTED	 103
#define X509V3_R_EXTENSION_VALUE_ERROR			 116
#define X509V3_R_ILLEGAL_HEX_DIGIT			 113
#define X509V3_R_INVALID_BOOLEAN_STRING			 104
#define X509V3_R_INVALID_EXTENSION_STRING		 105
#define X509V3_R_INVALID_NAME				 106
#define X509V3_R_INVALID_NULL_ARGUMENT			 107
#define X509V3_R_INVALID_NULL_NAME			 108
#define X509V3_R_INVALID_NULL_VALUE			 109
#define X509V3_R_INVALID_NUMBER				 140
#define X509V3_R_INVALID_NUMBERS			 141
#define X509V3_R_INVALID_OBJECT_IDENTIFIER		 110
#define X509V3_R_INVALID_OPTION				 138
#define X509V3_R_INVALID_POLICY_IDENTIFIER		 134
#define X509V3_R_INVALID_SECTION			 135
#define X509V3_R_ISSUER_DECODE_ERROR			 126
#define X509V3_R_MISSING_VALUE				 124
#define X509V3_R_NEED_ORGANIZATION_AND_NUMBERS		 142
#define X509V3_R_NO_CONFIG_DATABASE			 136
#define X509V3_R_NO_ISSUER_CERTIFICATE			 121
#define X509V3_R_NO_ISSUER_DETAILS			 127
#define X509V3_R_NO_POLICY_IDENTIFIER			 139
#define X509V3_R_NO_PUBLIC_KEY				 114
#define X509V3_R_NO_SUBJECT_DETAILS			 125
#define X509V3_R_ODD_NUMBER_OF_DIGITS			 112
#define X509V3_R_UNABLE_TO_GET_ISSUER_DETAILS		 122
#define X509V3_R_UNABLE_TO_GET_ISSUER_KEYID		 123
#define X509V3_R_UNKNOWN_BIT_STRING_ARGUMENT		 111
#define X509V3_R_UNKNOWN_EXTENSION			 129
#define X509V3_R_UNKNOWN_EXTENSION_NAME			 130
#define X509V3_R_UNKNOWN_OPTION				 120
#define X509V3_R_UNSUPPORTED_OPTION			 117
#define X509V3_R_USER_TOO_LONG				 132

#ifdef  __cplusplus
}
#endif
#endif

