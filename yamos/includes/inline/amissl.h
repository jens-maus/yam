/* Automatically generated header! Do not edit! */

#ifndef _INLINE_AMISSL_H
#define _INLINE_AMISSL_H

#ifndef __INLINE_MACROS_H
#include <inline/macros.h>
#endif /* !__INLINE_MACROS_H */

#ifndef AMISSL_BASE_NAME
#define AMISSL_BASE_NAME AmiSSLBase
#endif /* !AMISSL_BASE_NAME */

#define CleanupAmiSSLA(tagList) \
	LP1(0x24, long, CleanupAmiSSLA, struct TagItem *, tagList, a0, \
	, AMISSL_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define CleanupAmiSSL(tags...) \
	({ULONG _tags[] = { tags }; CleanupAmiSSLA((struct TagItem *)_tags);})
#endif /* !NO_INLINE_STDARG */

#define InitAmiSSLA(tagList) \
	LP1(0x1e, long, InitAmiSSLA, struct TagItem *, tagList, a0, \
	, AMISSL_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define InitAmiSSL(tags...) \
	({ULONG _tags[] = { tags }; InitAmiSSLA((struct TagItem *)_tags);})
#endif /* !NO_INLINE_STDARG */

#define RAND_seed(buf, num) \
	LP2NR(0x10b6, RAND_seed, const void *, buf, a0, int, num, a1, \
	, AMISSL_BASE_NAME)

#define SSL_CTX_free(a) \
	LP1NR(0x11fa, SSL_CTX_free, SSL_CTX *, a, a0, \
	, AMISSL_BASE_NAME)

#define SSL_CTX_load_verify_locations(ctx, CAfile, CApath) \
	LP3(0x1554, int, SSL_CTX_load_verify_locations, SSL_CTX *, ctx, a0, const char *, CAfile, a1, const char *, CApath, a2, \
	, AMISSL_BASE_NAME)

#define SSL_CTX_new(meth) \
	LP1(0x11f4, SSL_CTX *, SSL_CTX_new, SSL_METHOD *, meth, a0, \
	, AMISSL_BASE_NAME)

#define SSL_CTX_set_cipher_list(a, str) \
	LP2(0x11ee, int, SSL_CTX_set_cipher_list, SSL_CTX *, a, a0, char *, str, a1, \
	, AMISSL_BASE_NAME)

#define SSL_CTX_set_default_verify_paths(ctx) \
	LP1(0x154e, int, SSL_CTX_set_default_verify_paths, SSL_CTX *, ctx, a0, \
	, AMISSL_BASE_NAME)

#define SSL_CTX_set_verify(ctx, mode, callback) \
	LP3NRFP(0x13aa, SSL_CTX_set_verify, SSL_CTX *, ctx, a0, int, mode, a1, __fpt, callback, a2, \
	, AMISSL_BASE_NAME, int (*__fpt)(int, X509_STORE_CTX *))

#define SSL_connect(ssl) \
	LP1(0x1410, int, SSL_connect, SSL *, ssl, a0, \
	, AMISSL_BASE_NAME)

#define SSL_free(ssl) \
	LP1NR(0x1404, SSL_free, SSL *, ssl, a0, \
	, AMISSL_BASE_NAME)

#define SSL_library_init() \
	LP0(0x1500, int, SSL_library_init, \
	, AMISSL_BASE_NAME)

#define SSL_load_error_strings() \
	LP0NR(0x1314, SSL_load_error_strings, \
	, AMISSL_BASE_NAME)

#define SSL_new(ctx) \
	LP1(0x13f8, SSL *, SSL_new, SSL_CTX *, ctx, a0, \
	, AMISSL_BASE_NAME)

#define SSL_read(ssl, buf, num) \
	LP3(0x1416, int, SSL_read, SSL *, ssl, a0, char *, buf, a1, int, num, a2, \
	, AMISSL_BASE_NAME)

#define SSL_set_fd(s, fd) \
	LP2(0x1260, int, SSL_set_fd, SSL *, s, a0, int, fd, a1, \
	, AMISSL_BASE_NAME)

#define SSL_shutdown(s) \
	LP1(0x14a0, int, SSL_shutdown, SSL *, s, a0, \
	, AMISSL_BASE_NAME)

#define SSL_write(ssl, buf, num) \
	LP3(0x1422, int, SSL_write, SSL *, ssl, a0, const char *, buf, a1, int, num, a2, \
	, AMISSL_BASE_NAME)

#define SSLv23_client_method() \
	LP0(0x1476, SSL_METHOD *, SSLv23_client_method, \
	, AMISSL_BASE_NAME)

#endif /* !_INLINE_AMISSL_H */
