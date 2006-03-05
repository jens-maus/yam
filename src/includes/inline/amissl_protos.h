#ifndef _VBCCINLINE_AMISSL_H
#define _VBCCINLINE_AMISSL_H

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

long __CleanupAmiSSLA(__reg("a0") struct TagItem * tagList, __reg("a6") void *)="\tjsr\t-36(a6)";
#define CleanupAmiSSLA(tagList) __CleanupAmiSSLA((tagList), AmiSSLBase)

int __EVP_PKEY_bits(__reg("a0") EVP_PKEY * pkey, __reg("a6") void *)="\tjsr\t-2958(a6)";
#define EVP_PKEY_bits(pkey) __EVP_PKEY_bits((pkey), AmiSSLBase)

long __InitAmiSSLA(__reg("a0") struct TagItem * tagList, __reg("a6") void *)="\tjsr\t-30(a6)";
#define InitAmiSSLA(tagList) __InitAmiSSLA((tagList), AmiSSLBase)

void __RAND_seed(__reg("a0") const void * buf, __reg("a1") void * num, __reg("a6") void *)="\tjsr\t-4278(a6)";
#define RAND_seed(buf, num) __RAND_seed((buf), (void *)(num), AmiSSLBase)

const char * __SSL_CIPHER_get_name(__reg("a0") SSL_CIPHER * c, __reg("a6") void *)="\tjsr\t-4668(a6)";
#define SSL_CIPHER_get_name(c) __SSL_CIPHER_get_name((c), AmiSSLBase)

char * __SSL_CIPHER_get_version(__reg("a0") SSL_CIPHER * c, __reg("a6") void *)="\tjsr\t-4662(a6)";
#define SSL_CIPHER_get_version(c) __SSL_CIPHER_get_version((c), AmiSSLBase)

void __SSL_CTX_free(__reg("a0") SSL_CTX * a, __reg("a6") void *)="\tjsr\t-4602(a6)";
#define SSL_CTX_free(a) __SSL_CTX_free((a), AmiSSLBase)

int __SSL_CTX_load_verify_locations(__reg("a0") SSL_CTX * ctx, __reg("a1") const char * CAfile, __reg("a2") const char * CApath, __reg("a6") void *)="\tjsr\t-5460(a6)";
#define SSL_CTX_load_verify_locations(ctx, CAfile, CApath) __SSL_CTX_load_verify_locations((ctx), (CAfile), (CApath), AmiSSLBase)

SSL_CTX * __SSL_CTX_new(__reg("a0") SSL_METHOD * meth, __reg("a6") void *)="\tjsr\t-4596(a6)";
#define SSL_CTX_new(meth) __SSL_CTX_new((meth), AmiSSLBase)

int __SSL_CTX_set_cipher_list(__reg("a0") SSL_CTX * a, __reg("a1") char * str, __reg("a6") void *)="\tjsr\t-4590(a6)";
#define SSL_CTX_set_cipher_list(a, str) __SSL_CTX_set_cipher_list((a), (str), AmiSSLBase)

int __SSL_CTX_set_default_verify_paths(__reg("a0") SSL_CTX * ctx, __reg("a6") void *)="\tjsr\t-5454(a6)";
#define SSL_CTX_set_default_verify_paths(ctx) __SSL_CTX_set_default_verify_paths((ctx), AmiSSLBase)

void __SSL_CTX_set_verify(__reg("a0") SSL_CTX * ctx, __reg("a1") void * mode, __reg("a2") int (*callback)(int, X509_STORE_CTX *), __reg("a6") void *)="\tjsr\t-5034(a6)";
#define SSL_CTX_set_verify(ctx, mode, callback) __SSL_CTX_set_verify((ctx), (void *)(mode), (callback), AmiSSLBase)

int __SSL_connect(__reg("a0") SSL * ssl, __reg("a6") void *)="\tjsr\t-5136(a6)";
#define SSL_connect(ssl) __SSL_connect((ssl), AmiSSLBase)

void __SSL_free(__reg("a0") SSL * ssl, __reg("a6") void *)="\tjsr\t-5124(a6)";
#define SSL_free(ssl) __SSL_free((ssl), AmiSSLBase)

SSL_CIPHER * __SSL_get_current_cipher(__reg("a0") SSL * s, __reg("a6") void *)="\tjsr\t-4650(a6)";
#define SSL_get_current_cipher(s) __SSL_get_current_cipher((s), AmiSSLBase)

X509 * __SSL_get_peer_certificate(__reg("a0") SSL * s, __reg("a6") void *)="\tjsr\t-5004(a6)";
#define SSL_get_peer_certificate(s) __SSL_get_peer_certificate((s), AmiSSLBase)

int __SSL_library_init(__reg("a6") void *)="\tjsr\t-5376(a6)";
#define SSL_library_init() __SSL_library_init(AmiSSLBase)

void __SSL_load_error_strings(__reg("a6") void *)="\tjsr\t-4884(a6)";
#define SSL_load_error_strings() __SSL_load_error_strings(AmiSSLBase)

SSL * __SSL_new(__reg("a0") SSL_CTX * ctx, __reg("a6") void *)="\tjsr\t-5112(a6)";
#define SSL_new(ctx) __SSL_new((ctx), AmiSSLBase)

int __SSL_read(__reg("a0") SSL * ssl, __reg("a1") char * buf, __reg("a2") void * num, __reg("a6") void *)="\tjsr\t-5142(a6)";
#define SSL_read(ssl, buf, num) __SSL_read((ssl), (buf), (void *)(num), AmiSSLBase)

int __SSL_set_fd(__reg("a0") SSL * s, __reg("a1") void * fd, __reg("a6") void *)="\tjsr\t-4704(a6)";
#define SSL_set_fd(s, fd) __SSL_set_fd((s), (void *)(fd), AmiSSLBase)

int __SSL_shutdown(__reg("a0") SSL * s, __reg("a6") void *)="\tjsr\t-5280(a6)";
#define SSL_shutdown(s) __SSL_shutdown((s), AmiSSLBase)

int __SSL_write(__reg("a0") SSL * ssl, __reg("a1") const char * buf, __reg("a2") void * num, __reg("a6") void *)="\tjsr\t-5154(a6)";
#define SSL_write(ssl, buf, num) __SSL_write((ssl), (buf), (void *)(num), AmiSSLBase)

SSL_METHOD * __SSLv23_client_method(__reg("a6") void *)="\tjsr\t-5238(a6)";
#define SSLv23_client_method() __SSLv23_client_method(AmiSSLBase)

char * __X509_NAME_oneline(__reg("a0") X509_NAME * a, __reg("a1") char * buf, __reg("a2") void * size, __reg("a6") void *)="\tjsr\t-7212(a6)";
#define X509_NAME_oneline(a, buf, size) __X509_NAME_oneline((a), (buf), (void *)(size), AmiSSLBase)

void __X509_free(__reg("a0") X509 * a, __reg("a6") void *)="\tjsr\t-7014(a6)";
#define X509_free(a) __X509_free((a), AmiSSLBase)

X509_NAME * __X509_get_issuer_name(__reg("a0") X509 * a, __reg("a6") void *)="\tjsr\t-7260(a6)";
#define X509_get_issuer_name(a) __X509_get_issuer_name((a), AmiSSLBase)

EVP_PKEY * __X509_get_pubkey(__reg("a0") X509 * x, __reg("a6") void *)="\tjsr\t-7296(a6)";
#define X509_get_pubkey(x) __X509_get_pubkey((x), AmiSSLBase)

X509_NAME * __X509_get_subject_name(__reg("a0") X509 * a, __reg("a6") void *)="\tjsr\t-7272(a6)";
#define X509_get_subject_name(a) __X509_get_subject_name((a), AmiSSLBase)

#endif /*  _VBCCINLINE_AMISSL_H  */
