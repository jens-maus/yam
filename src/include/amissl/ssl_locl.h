/* ssl/ssl_locl.h */
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

#ifndef HEADER_SSL_LOCL_H
#define HEADER_SSL_LOCL_H

#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <errno.h>

#include <amissl/e_os2.h>

#include <amissl/buffer.h>
#include <amissl/comp.h>
#include <amissl/bio.h>
#include <amissl/crypto.h>
#include <amissl/evp.h>
#include <amissl/stack.h>
#include <amissl/x509.h>
#include <amissl/err.h>
#include <amissl/ssl.h>

#define PKCS1_CHECK

#define c2l(c,l)	(l = ((unsigned long)(*((c)++)))     , \
			 l|=(((unsigned long)(*((c)++)))<< 8), \
			 l|=(((unsigned long)(*((c)++)))<<16), \
			 l|=(((unsigned long)(*((c)++)))<<24))

/* NOTE - c is not incremented as per c2l */
#define c2ln(c,l1,l2,n)	{ \
			c+=n; \
			l1=l2=0; \
			switch (n) { \
			case 8: l2 =((unsigned long)(*(--(c))))<<24; \
			case 7: l2|=((unsigned long)(*(--(c))))<<16; \
			case 6: l2|=((unsigned long)(*(--(c))))<< 8; \
			case 5: l2|=((unsigned long)(*(--(c))));     \
			case 4: l1 =((unsigned long)(*(--(c))))<<24; \
			case 3: l1|=((unsigned long)(*(--(c))))<<16; \
			case 2: l1|=((unsigned long)(*(--(c))))<< 8; \
			case 1: l1|=((unsigned long)(*(--(c))));     \
				} \
			}

#define l2c(l,c)	(*((c)++)=(unsigned char)(((l)    )&0xff), \
			 *((c)++)=(unsigned char)(((l)>> 8)&0xff), \
			 *((c)++)=(unsigned char)(((l)>>16)&0xff), \
			 *((c)++)=(unsigned char)(((l)>>24)&0xff))

#define n2l(c,l)	(l =((unsigned long)(*((c)++)))<<24, \
			 l|=((unsigned long)(*((c)++)))<<16, \
			 l|=((unsigned long)(*((c)++)))<< 8, \
			 l|=((unsigned long)(*((c)++))))

#define l2n(l,c)	(*((c)++)=(unsigned char)(((l)>>24)&0xff), \
			 *((c)++)=(unsigned char)(((l)>>16)&0xff), \
			 *((c)++)=(unsigned char)(((l)>> 8)&0xff), \
			 *((c)++)=(unsigned char)(((l)    )&0xff))

/* NOTE - c is not incremented as per l2c */
#define l2cn(l1,l2,c,n)	{ \
			c+=n; \
			switch (n) { \
			case 8: *(--(c))=(unsigned char)(((l2)>>24)&0xff); \
			case 7: *(--(c))=(unsigned char)(((l2)>>16)&0xff); \
			case 6: *(--(c))=(unsigned char)(((l2)>> 8)&0xff); \
			case 5: *(--(c))=(unsigned char)(((l2)    )&0xff); \
			case 4: *(--(c))=(unsigned char)(((l1)>>24)&0xff); \
			case 3: *(--(c))=(unsigned char)(((l1)>>16)&0xff); \
			case 2: *(--(c))=(unsigned char)(((l1)>> 8)&0xff); \
			case 1: *(--(c))=(unsigned char)(((l1)    )&0xff); \
				} \
			}

#define n2s(c,s)	((s=(((unsigned int)(c[0]))<< 8)| \
			    (((unsigned int)(c[1]))    )),c+=2)
#define s2n(s,c)	((c[0]=(unsigned char)(((s)>> 8)&0xff), \
			  c[1]=(unsigned char)(((s)    )&0xff)),c+=2)

#define n2l3(c,l)	((l =(((unsigned long)(c[0]))<<16)| \
			     (((unsigned long)(c[1]))<< 8)| \
			     (((unsigned long)(c[2]))    )),c+=3)

#define l2n3(l,c)	((c[0]=(unsigned char)(((l)>>16)&0xff), \
			  c[1]=(unsigned char)(((l)>> 8)&0xff), \
			  c[2]=(unsigned char)(((l)    )&0xff)),c+=3)

/* LOCAL STUFF */

#define SSL_DECRYPT	0
#define SSL_ENCRYPT	1

#define TWO_BYTE_BIT	0x80
#define SEC_ESC_BIT	0x40
#define TWO_BYTE_MASK	0x7fff
#define THREE_BYTE_MASK	0x3fff

#define INC32(a)	((a)=((a)+1)&0xffffffffL)
#define DEC32(a)	((a)=((a)-1)&0xffffffffL)
#define MAX_MAC_SIZE	20 /* up from 16 for SSLv3 */

#define SSL_MKEY_MASK		0x0000001FL
#define SSL_kRSA		0x00000001L /* RSA key exchange */
#define SSL_kDHr		0x00000002L /* DH cert RSA CA cert */
#define SSL_kDHd		0x00000004L /* DH cert DSA CA cert */
#define SSL_kFZA		0x00000008L
#define SSL_kEDH		0x00000010L /* tmp DH key no DH cert */
#define SSL_EDH			(SSL_kEDH|(SSL_AUTH_MASK^SSL_aNULL))

#define SSL_AUTH_MASK		0x000003e0L
#define SSL_aRSA		0x00000020L /* Authenticate with RSA */
#define SSL_aDSS 		0x00000040L /* Authenticate with DSS */
#define SSL_DSS 		SSL_aDSS
#define SSL_aFZA 		0x00000080L
#define SSL_aNULL 		0x00000100L /* no Authenticate, ADH */
#define SSL_aDH 		0x00000200L /* no Authenticate, ADH */

#define SSL_NULL		(SSL_eNULL)
#define SSL_ADH			(SSL_kEDH|SSL_aNULL)
#define SSL_RSA			(SSL_kRSA|SSL_aRSA)
#define SSL_DH			(SSL_kDHr|SSL_kDHd|SSL_kEDH)
#define SSL_FZA			(SSL_aFZA|SSL_kFZA|SSL_eFZA)

#define SSL_ENC_MASK		0x0001Fc00L
#define SSL_DES			0x00000400L
#define SSL_3DES		0x00000800L
#define SSL_RC4			0x00001000L
#define SSL_RC2			0x00002000L
#define SSL_IDEA		0x00004000L
#define SSL_eFZA		0x00008000L
#define SSL_eNULL		0x00010000L

#define SSL_MAC_MASK		0x00060000L
#define SSL_MD5			0x00020000L
#define SSL_SHA1		0x00040000L
#define SSL_SHA			(SSL_SHA1)

#define SSL_EXP_MASK		0x00300000L
#define SSL_EXP40		0x00100000L
#define SSL_NOT_EXP		0x00200000L
#define SSL_EXP56		0x00300000L
#define SSL_IS_EXPORT(a)	((a)&SSL_EXP40)
#define SSL_IS_EXPORT56(a)	(((a)&SSL_EXP_MASK) == SSL_EXP56)
#define SSL_IS_EXPORT40(a)	(((a)&SSL_EXP_MASK) == SSL_EXP40)
#define SSL_C_IS_EXPORT(c)	SSL_IS_EXPORT((c)->algorithms)
#define SSL_C_IS_EXPORT56(c)	SSL_IS_EXPORT56((c)->algorithms)
#define SSL_C_IS_EXPORT40(c)	SSL_IS_EXPORT40((c)->algorithms)
#define SSL_EXPORT_KEYLENGTH(a)	(SSL_IS_EXPORT40(a) ? 5 : \
				 ((a)&SSL_ENC_MASK) == SSL_DES ? 8 : 7)
#define SSL_EXPORT_PKEYLENGTH(a) (SSL_IS_EXPORT40(a) ? 512 : 1024)
#define SSL_C_EXPORT_KEYLENGTH(c)	SSL_EXPORT_KEYLENGTH((c)->algorithms)
#define SSL_C_EXPORT_PKEYLENGTH(c)	SSL_EXPORT_PKEYLENGTH((c)->algorithms)

#define SSL_SSL_MASK		0x00c00000L
#define SSL_SSLV2		0x00400000L
#define SSL_SSLV3		0x00800000L
#define SSL_TLSV1		SSL_SSLV3	/* for now */

#define SSL_STRONG_MASK		0x07000000L
#define SSL_LOW			0x01000000L
#define SSL_MEDIUM		0x02000000L
#define SSL_HIGH		0x04000000L

/* we have used 0fffffff - 4 bits left to go */
#define SSL_ALL			0xffffffffL
#define SSL_ALL_CIPHERS		(SSL_MKEY_MASK|SSL_AUTH_MASK|SSL_ENC_MASK|\
				SSL_MAC_MASK|SSL_EXP_MASK)

/* Mostly for SSLv3 */
#define SSL_PKEY_RSA_ENC	0
#define SSL_PKEY_RSA_SIGN	1
#define SSL_PKEY_DSA_SIGN	2
#define SSL_PKEY_DH_RSA		3
#define SSL_PKEY_DH_DSA		4
#define SSL_PKEY_NUM		5

/* SSL_kRSA <- RSA_ENC | (RSA_TMP & RSA_SIGN) |
 * 	    <- (EXPORT & (RSA_ENC | RSA_TMP) & RSA_SIGN)
 * SSL_kDH  <- DH_ENC & (RSA_ENC | RSA_SIGN | DSA_SIGN)
 * SSL_kEDH <- RSA_ENC | RSA_SIGN | DSA_SIGN
 * SSL_aRSA <- RSA_ENC | RSA_SIGN
 * SSL_aDSS <- DSA_SIGN
 */

/*
#define CERT_INVALID		0
#define CERT_PUBLIC_KEY		1
#define CERT_PRIVATE_KEY	2
*/

typedef struct cert_pkey_st
	{
	X509 *x509;
	EVP_PKEY *privatekey;
	} CERT_PKEY;

typedef struct cert_st
	{
	/* Current active set */
	CERT_PKEY *key; /* ALWAYS points to an element of the pkeys array
					 * Probably it would make more sense to store
					 * an index, not a pointer. */

	/* The following masks are for the key and auth
	 * algorithms that are supported by the certs below */
	int valid;
	unsigned long mask;
	unsigned long export_mask;
	RSA *rsa_tmp;
	RSA *(*rsa_tmp_cb)(SSL *ssl,int is_export,int keysize);
	DH *dh_tmp;
	DH *(*dh_tmp_cb)(SSL *ssl,int is_export,int keysize);

	CERT_PKEY pkeys[SSL_PKEY_NUM];

	int references; /* >1 only if SSL_copy_session_id is used */
	} CERT;


typedef struct sess_cert_st
	{
	STACK_OF(X509) *cert_chain; /* as received from peer (not for SSL2) */

	/* The 'peer_...' members are used only by clients. */
	int peer_cert_type;

	CERT_PKEY *peer_key; /* points to an element of peer_pkeys (never NULL!) */
	CERT_PKEY peer_pkeys[SSL_PKEY_NUM];
	/* Obviously we don't have the private keys of these,
	 * so maybe we shouldn't even use the CERT_PKEY type here. */

	RSA *peer_rsa_tmp; /* not used for SSL 2 */
	DH *peer_dh_tmp; /* not used for SSL 2 */

	int references; /* actually always 1 at the moment */
	} SESS_CERT;


#define FP_ICC  (int (*)(const void *,const void *))
#define ssl_put_cipher_by_char(ssl,ciph,ptr) \
		((ssl)->method->put_cipher_by_char((ciph),(ptr)))
#define ssl_get_cipher_by_char(ssl,ptr) \
		((ssl)->method->get_cipher_by_char(ptr))

/* This is for the SSLv3/TLSv1.0 differences in crypto/hash stuff
 * It is a bit of a mess of functions, but hell, think of it as
 * an opaque strucute :-) */
typedef struct ssl3_enc_method
	{
	int (*enc)();
	int (*mac)();
	int (*setup_key_block)();
	int (*generate_master_secret)();
	int (*change_cipher_state)();
	int (*final_finish_mac)();
	int finish_mac_length;
	int (*cert_verify_mac)();
	unsigned char client_finished[20];
	int client_finished_len;
	unsigned char server_finished[20];
	int server_finished_len;
	int (*alert_value)();
	} SSL3_ENC_METHOD;

/* Used for holding the relevant compression methods loaded into SSL_CTX */
typedef struct ssl3_comp_st
	{
	int comp_id;	/* The identifer byte for this compression type */
	char *name;	/* Text name used for the compression type */
	COMP_METHOD *method; /* The method :-) */
	} SSL3_COMP;

OPENSSL_EXTERN SSL3_ENC_METHOD ssl3_undef_enc_method;
OPENSSL_EXTERN SSL_CIPHER ssl2_ciphers[];
OPENSSL_EXTERN SSL_CIPHER ssl3_ciphers[];

#endif
