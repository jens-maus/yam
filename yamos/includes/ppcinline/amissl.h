#ifndef _PPCINLINE_AMISSL_H
#define _PPCINLINE_AMISSL_H

#ifndef __PPCINLINE_MACROS_H
#include <ppcinline/macros.h>
#endif

#ifndef AMISSL_BASE_NAME
#define AMISSL_BASE_NAME AmiSSLBase
#endif

#define InitAmiSSLA(tagList) \
	LP1(0x1e, long, InitAmiSSLA, struct TagItem *, tagList, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define InitAmiSSL(tags...) \
	({ULONG _tags[] = {tags}; InitAmiSSLA((struct TagItem *) _tags);})
#endif

#define CleanupAmiSSLA(tagList) \
	LP1(0x24, long, CleanupAmiSSLA, struct TagItem *, tagList, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define CleanupAmiSSL(tags...) \
	({ULONG _tags[] = {tags}; CleanupAmiSSLA((struct TagItem *) _tags);})
#endif

#define AmiSSLMalloc(num) \
	LP1(0x2a, void *, AmiSSLMalloc, long, num, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define AmiSSLRealloc(block, num) \
	LP2(0x30, void *, AmiSSLRealloc, void *, block, a0, long, num, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define AmiSSLFree(block) \
	LP1NR(0x36, AmiSSLFree, void *, block, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_BF_IsReal() \
	LP0(0x3c, long, EVP_BF_IsReal, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_CAST_IsReal() \
	LP0(0x42, long, EVP_CAST_IsReal, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_des_IsReal() \
	LP0(0x48, long, EVP_des_IsReal, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_idea_IsReal() \
	LP0(0x4e, long, EVP_idea_IsReal, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_MD2_IsReal() \
	LP0(0x54, long, EVP_MD2_IsReal, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_MD5_IsReal() \
	LP0(0x5a, long, EVP_MD5_IsReal, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_MDC2_IsReal() \
	LP0(0x60, long, EVP_MDC2_IsReal, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_RC2_IsReal() \
	LP0(0x66, long, EVP_RC2_IsReal, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_RC4_IsReal() \
	LP0(0x6c, long, EVP_RC4_IsReal, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_RC5_IsReal() \
	LP0(0x72, long, EVP_RC5_IsReal, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_RIPEMD160_IsReal() \
	LP0(0x78, long, EVP_RIPEMD160_IsReal, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_SHA_IsReal() \
	LP0(0x7e, long, EVP_SHA_IsReal, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ASN1_TYPE_new() \
	LP0(0x84, ASN1_TYPE *, ASN1_TYPE_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ASN1_TYPE_free(a) \
	LP1NR(0x8a, ASN1_TYPE_free, ASN1_TYPE *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_ASN1_TYPE(a, pp) \
	LP2(0x90, int, i2d_ASN1_TYPE, ASN1_TYPE *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_ASN1_TYPE(a, pp, length) \
	LP3(0x96, ASN1_TYPE *, d2i_ASN1_TYPE, ASN1_TYPE **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ASN1_TYPE_get(a) \
	LP1(0x9c, int, ASN1_TYPE_get, ASN1_TYPE *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ASN1_TYPE_set(a, type, value) \
	LP3NR(0xa2, ASN1_TYPE_set, ASN1_TYPE *, a, a0, int, type, a1, void *, value, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ASN1_OBJECT_new() \
	LP0(0xa8, ASN1_OBJECT *, ASN1_OBJECT_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ASN1_OBJECT_free(a) \
	LP1NR(0xae, ASN1_OBJECT_free, ASN1_OBJECT *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_ASN1_OBJECT(a, pp) \
	LP2(0xb4, int, i2d_ASN1_OBJECT, ASN1_OBJECT *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_ASN1_OBJECT(a, pp, length) \
	LP3(0xba, ASN1_OBJECT *, d2i_ASN1_OBJECT, ASN1_OBJECT **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ASN1_STRING_new() \
	LP0(0xc0, ASN1_STRING *, ASN1_STRING_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ASN1_STRING_free(a) \
	LP1NR(0xc6, ASN1_STRING_free, ASN1_STRING *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ASN1_STRING_dup(a) \
	LP1(0xcc, ASN1_STRING *, ASN1_STRING_dup, ASN1_STRING *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ASN1_STRING_type_new(type) \
	LP1(0xd2, ASN1_STRING *, ASN1_STRING_type_new, int, type, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ASN1_STRING_cmp(a, b) \
	LP2(0xd8, int, ASN1_STRING_cmp, ASN1_STRING *, a, a0, ASN1_STRING *, b, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ASN1_STRING_set(str, data, len) \
	LP3(0xde, int, ASN1_STRING_set, ASN1_STRING *, str, a0, const void *, data, a1, int, len, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_ASN1_BIT_STRING(a, pp) \
	LP2(0xe4, int, i2d_ASN1_BIT_STRING, ASN1_BIT_STRING *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_ASN1_BIT_STRING(a, pp, length) \
	LP3(0xea, ASN1_BIT_STRING *, d2i_ASN1_BIT_STRING, ASN1_BIT_STRING **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ASN1_BIT_STRING_set_bit(a, n, value) \
	LP3(0xf0, int, ASN1_BIT_STRING_set_bit, ASN1_BIT_STRING *, a, a0, int, n, a1, int, value, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ASN1_BIT_STRING_get_bit(a, n) \
	LP2(0xf6, int, ASN1_BIT_STRING_get_bit, ASN1_BIT_STRING *, a, a0, int, n, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_ASN1_BOOLEAN(a, pp) \
	LP2(0xfc, int, i2d_ASN1_BOOLEAN, int, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_ASN1_BOOLEAN(a, pp, length) \
	LP3(0x102, int, d2i_ASN1_BOOLEAN, int *, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_ASN1_INTEGER(a, pp) \
	LP2(0x108, int, i2d_ASN1_INTEGER, ASN1_INTEGER *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_ASN1_INTEGER(a, pp, length) \
	LP3(0x10e, ASN1_INTEGER *, d2i_ASN1_INTEGER, ASN1_INTEGER **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_ASN1_UINTEGER(a, pp, length) \
	LP3(0x114, ASN1_INTEGER *, d2i_ASN1_UINTEGER, ASN1_INTEGER **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_ASN1_ENUMERATED(a, pp) \
	LP2(0x11a, int, i2d_ASN1_ENUMERATED, ASN1_ENUMERATED *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_ASN1_ENUMERATED(a, pp, length) \
	LP3(0x120, ASN1_ENUMERATED *, d2i_ASN1_ENUMERATED, ASN1_ENUMERATED **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ASN1_UTCTIME_check(a) \
	LP1(0x126, int, ASN1_UTCTIME_check, ASN1_UTCTIME *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ASN1_UTCTIME_set(s, t) \
	LP2(0x12c, ASN1_UTCTIME *, ASN1_UTCTIME_set, ASN1_UTCTIME *, s, a0, time_t, t, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ASN1_UTCTIME_set_string(s, str) \
	LP2(0x132, int, ASN1_UTCTIME_set_string, ASN1_UTCTIME *, s, a0, char *, str, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ASN1_GENERALIZEDTIME_check(a) \
	LP1(0x138, int, ASN1_GENERALIZEDTIME_check, ASN1_GENERALIZEDTIME *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ASN1_GENERALIZEDTIME_set(s, t) \
	LP2(0x13e, ASN1_GENERALIZEDTIME *, ASN1_GENERALIZEDTIME_set, ASN1_GENERALIZEDTIME *, s, a0, time_t, t, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ASN1_GENERALIZEDTIME_set_string(s, str) \
	LP2(0x144, int, ASN1_GENERALIZEDTIME_set_string, ASN1_GENERALIZEDTIME *, s, a0, char *, str, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_ASN1_OCTET_STRING(a, pp) \
	LP2(0x14a, int, i2d_ASN1_OCTET_STRING, ASN1_OCTET_STRING *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_ASN1_OCTET_STRING(a, pp, length) \
	LP3(0x150, ASN1_OCTET_STRING *, d2i_ASN1_OCTET_STRING, ASN1_OCTET_STRING **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_ASN1_VISIBLESTRING(a, pp) \
	LP2(0x156, int, i2d_ASN1_VISIBLESTRING, ASN1_VISIBLESTRING *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_ASN1_VISIBLESTRING(a, pp, length) \
	LP3(0x15c, ASN1_VISIBLESTRING *, d2i_ASN1_VISIBLESTRING, ASN1_VISIBLESTRING **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_ASN1_UTF8STRING(a, pp) \
	LP2(0x162, int, i2d_ASN1_UTF8STRING, ASN1_UTF8STRING *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_ASN1_UTF8STRING(a, pp, length) \
	LP3(0x168, ASN1_UTF8STRING *, d2i_ASN1_UTF8STRING, ASN1_UTF8STRING **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_ASN1_BMPSTRING(a, pp) \
	LP2(0x16e, int, i2d_ASN1_BMPSTRING, ASN1_BMPSTRING *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_ASN1_BMPSTRING(a, pp, length) \
	LP3(0x174, ASN1_BMPSTRING *, d2i_ASN1_BMPSTRING, ASN1_BMPSTRING **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_ASN1_PRINTABLE(a, pp) \
	LP2(0x17a, int, i2d_ASN1_PRINTABLE, ASN1_STRING *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_ASN1_PRINTABLE(a, pp, l) \
	LP3(0x180, ASN1_STRING *, d2i_ASN1_PRINTABLE, ASN1_STRING **, a, a0, unsigned char **, pp, a1, long, l, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_ASN1_PRINTABLESTRING(a, pp, l) \
	LP3(0x186, ASN1_PRINTABLESTRING *, d2i_ASN1_PRINTABLESTRING, ASN1_PRINTABLESTRING **, a, a0, unsigned char **, pp, a1, long, l, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_DIRECTORYSTRING(a, pp) \
	LP2(0x18c, int, i2d_DIRECTORYSTRING, ASN1_STRING *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_DIRECTORYSTRING(a, pp, length) \
	LP3(0x192, ASN1_STRING *, d2i_DIRECTORYSTRING, ASN1_STRING **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_DISPLAYTEXT(a, pp) \
	LP2(0x198, int, i2d_DISPLAYTEXT, ASN1_STRING *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_DISPLAYTEXT(a, pp, length) \
	LP3(0x19e, ASN1_STRING *, d2i_DISPLAYTEXT, ASN1_STRING **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_ASN1_T61STRING(a, pp, l) \
	LP3(0x1a4, ASN1_T61STRING *, d2i_ASN1_T61STRING, ASN1_T61STRING **, a, a0, unsigned char **, pp, a1, long, l, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_ASN1_IA5STRING(a, pp) \
	LP2(0x1aa, int, i2d_ASN1_IA5STRING, ASN1_IA5STRING *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_ASN1_IA5STRING(a, pp, l) \
	LP3(0x1b0, ASN1_IA5STRING *, d2i_ASN1_IA5STRING, ASN1_IA5STRING **, a, a0, unsigned char **, pp, a1, long, l, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_ASN1_UTCTIME(a, pp) \
	LP2(0x1b6, int, i2d_ASN1_UTCTIME, ASN1_UTCTIME *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_ASN1_UTCTIME(a, pp, length) \
	LP3(0x1bc, ASN1_UTCTIME *, d2i_ASN1_UTCTIME, ASN1_UTCTIME **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_ASN1_GENERALIZEDTIME(a, pp) \
	LP2(0x1c2, int, i2d_ASN1_GENERALIZEDTIME, ASN1_GENERALIZEDTIME *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_ASN1_GENERALIZEDTIME(a, pp, length) \
	LP3(0x1c8, ASN1_GENERALIZEDTIME *, d2i_ASN1_GENERALIZEDTIME, ASN1_GENERALIZEDTIME **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_ASN1_TIME(a, pp) \
	LP2(0x1ce, int, i2d_ASN1_TIME, ASN1_TIME *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_ASN1_TIME(a, pp, length) \
	LP3(0x1d4, ASN1_TIME *, d2i_ASN1_TIME, ASN1_TIME **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ASN1_TIME_set(s, t) \
	LP2(0x1da, ASN1_TIME *, ASN1_TIME_set, ASN1_TIME *, s, a0, time_t, t, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_ASN1_SET(a, pp, func, ex_tag, ex_class, is_set) \
	LP6FP(0x1e0, int, i2d_ASN1_SET, STACK *, a, a0, unsigned char **, pp, a1, __fpt, func, a2, int, ex_tag, a3, int, ex_class, d0, int, is_set, d1, \
	, AMISSL_BASE_NAME, int (*__fpt)(), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_ASN1_SET(a, pp, length, func, free_func, ex_tag, ex_class) \
	LP7FP(0x1e6, *, d2i_ASN1_SET, STACK **, a, a0, unsigned char **, pp, a1, long, length, a2, __fpt, func, a3, void *, free_func, d0, int, ex_tag, d1, int, ex_class, d2, \
	, AMISSL_BASE_NAME, char * (*__fpt)(), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2a_ASN1_INTEGER(bp, a) \
	LP2(0x1ec, int, i2a_ASN1_INTEGER, BIO *, bp, a0, ASN1_INTEGER *, a, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define a2i_ASN1_INTEGER(bp, bs, buf, size) \
	LP4(0x1f2, int, a2i_ASN1_INTEGER, BIO *, bp, a0, ASN1_INTEGER *, bs, a1, char *, buf, a2, int, size, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2a_ASN1_ENUMERATED(bp, a) \
	LP2(0x1f8, int, i2a_ASN1_ENUMERATED, BIO *, bp, a0, ASN1_ENUMERATED *, a, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define a2i_ASN1_ENUMERATED(bp, bs, buf, size) \
	LP4(0x1fe, int, a2i_ASN1_ENUMERATED, BIO *, bp, a0, ASN1_ENUMERATED *, bs, a1, char *, buf, a2, int, size, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2a_ASN1_OBJECT(bp, a) \
	LP2(0x204, int, i2a_ASN1_OBJECT, BIO *, bp, a0, ASN1_OBJECT *, a, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define a2i_ASN1_STRING(bp, bs, buf, size) \
	LP4(0x20a, int, a2i_ASN1_STRING, BIO *, bp, a0, ASN1_STRING *, bs, a1, char *, buf, a2, int, size, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2a_ASN1_STRING(bp, a, type) \
	LP3(0x210, int, i2a_ASN1_STRING, BIO *, bp, a0, ASN1_STRING *, a, a1, int, type, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2t_ASN1_OBJECT(buf, buf_len, a) \
	LP3(0x216, int, i2t_ASN1_OBJECT, char *, buf, a0, int, buf_len, a1, ASN1_OBJECT *, a, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define a2d_ASN1_OBJECT(out, olen, buf, num) \
	LP4(0x21c, int, a2d_ASN1_OBJECT, unsigned char *, out, a0, int, olen, a1, const char *, buf, a2, int, num, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ASN1_OBJECT_create(nid, data, len, sn, ln) \
	LP5(0x222, ASN1_OBJECT *, ASN1_OBJECT_create, int, nid, a0, unsigned char *, data, a1, int, len, a2, char *, sn, a3, char *, ln, d0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ASN1_INTEGER_set(a, v) \
	LP2(0x228, int, ASN1_INTEGER_set, ASN1_INTEGER *, a, a0, long, v, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ASN1_INTEGER_get(a) \
	LP1(0x22e, long, ASN1_INTEGER_get, ASN1_INTEGER *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_to_ASN1_INTEGER(bn, ai) \
	LP2(0x234, ASN1_INTEGER *, BN_to_ASN1_INTEGER, BIGNUM *, bn, a0, ASN1_INTEGER *, ai, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ASN1_INTEGER_to_BN(ai, bn) \
	LP2(0x23a, BIGNUM *, ASN1_INTEGER_to_BN, ASN1_INTEGER *, ai, a0, BIGNUM *, bn, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ASN1_ENUMERATED_set(a, v) \
	LP2(0x240, int, ASN1_ENUMERATED_set, ASN1_ENUMERATED *, a, a0, long, v, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ASN1_ENUMERATED_get(a) \
	LP1(0x246, long, ASN1_ENUMERATED_get, ASN1_ENUMERATED *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_to_ASN1_ENUMERATED(bn, ai) \
	LP2(0x24c, ASN1_ENUMERATED *, BN_to_ASN1_ENUMERATED, BIGNUM *, bn, a0, ASN1_ENUMERATED *, ai, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ASN1_ENUMERATED_to_BN(ai, bn) \
	LP2(0x252, BIGNUM *, ASN1_ENUMERATED_to_BN, ASN1_ENUMERATED *, ai, a0, BIGNUM *, bn, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ASN1_PRINTABLE_type(s, max) \
	LP2(0x258, int, ASN1_PRINTABLE_type, unsigned char *, s, a0, int, max, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_ASN1_bytes(a, pp, tag, xclass) \
	LP4(0x25e, int, i2d_ASN1_bytes, ASN1_STRING *, a, a0, unsigned char **, pp, a1, int, tag, a2, int, xclass, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_ASN1_bytes(a, pp, length, Ptag, Pclass) \
	LP5(0x264, ASN1_STRING *, d2i_ASN1_bytes, ASN1_STRING **, a, a0, unsigned char **, pp, a1, long, length, a2, int, Ptag, a3, int, Pclass, d0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_ASN1_type_bytes(a, pp, length, type) \
	LP4(0x26a, ASN1_STRING *, d2i_ASN1_type_bytes, ASN1_STRING **, a, a0, unsigned char **, pp, a1, long, length, a2, int, type, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define asn1_Finish(c) \
	LP1(0x270, int, asn1_Finish, ASN1_CTX *, c, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ASN1_get_object(pp, plength, ptag, pclass, omax) \
	LP5(0x276, int, ASN1_get_object, unsigned char **, pp, a0, long *, plength, a1, int *, ptag, a2, int *, pclass, a3, long, omax, d0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ASN1_check_infinite_end(p, len) \
	LP2(0x27c, int, ASN1_check_infinite_end, unsigned char **, p, a0, long, len, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ASN1_put_object(pp, constructed, length, tag, xclass) \
	LP5NR(0x282, ASN1_put_object, unsigned char **, pp, a0, int, constructed, a1, int, length, a2, int, tag, a3, int, xclass, d0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ASN1_object_size(constructed, length, tag) \
	LP3(0x288, int, ASN1_object_size, int, constructed, a0, int, length, a1, int, tag, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ASN1_dup(i2d, d2i, x) \
	LP3FP(0x28e, char *, ASN1_dup, __fpt, i2d, a0, void *, d2i, a1, char *, x, a2, \
	, AMISSL_BASE_NAME, int (*__fpt)(), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ASN1_d2i_bio(xnew, d2i, bp, x) \
	LP4FP(0x294, char *, ASN1_d2i_bio, __fpt, xnew, a0, void *, d2i, a1, BIO *, bp, a2, unsigned char **, x, a3, \
	, AMISSL_BASE_NAME, char * (*__fpt)(), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ASN1_i2d_bio(i2d, out, x) \
	LP3FP(0x29a, int, ASN1_i2d_bio, __fpt, i2d, a0, BIO *, out, a1, unsigned char *, x, a2, \
	, AMISSL_BASE_NAME, int (*__fpt)(), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ASN1_UTCTIME_print(fp, a) \
	LP2(0x2a0, int, ASN1_UTCTIME_print, BIO *, fp, a0, ASN1_UTCTIME *, a, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ASN1_GENERALIZEDTIME_print(fp, a) \
	LP2(0x2a6, int, ASN1_GENERALIZEDTIME_print, BIO *, fp, a0, ASN1_GENERALIZEDTIME *, a, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ASN1_TIME_print(fp, a) \
	LP2(0x2ac, int, ASN1_TIME_print, BIO *, fp, a0, ASN1_TIME *, a, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ASN1_STRING_print(bp, v) \
	LP2(0x2b2, int, ASN1_STRING_print, BIO *, bp, a0, ASN1_STRING *, v, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ASN1_parse(bp, pp, len, indent) \
	LP4(0x2b8, int, ASN1_parse, BIO *, bp, a0, unsigned char *, pp, a1, long, len, a2, int, indent, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_ASN1_HEADER(a, pp) \
	LP2(0x2be, int, i2d_ASN1_HEADER, ASN1_HEADER *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_ASN1_HEADER(a, pp, length) \
	LP3(0x2c4, ASN1_HEADER *, d2i_ASN1_HEADER, ASN1_HEADER **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ASN1_HEADER_new() \
	LP0(0x2ca, ASN1_HEADER *, ASN1_HEADER_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ASN1_HEADER_free(a) \
	LP1NR(0x2d0, ASN1_HEADER_free, ASN1_HEADER *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ASN1_UNIVERSALSTRING_to_string(s) \
	LP1(0x2d6, int, ASN1_UNIVERSALSTRING_to_string, ASN1_UNIVERSALSTRING *, s, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ERR_load_ASN1_strings() \
	LP0NR(0x2dc, ERR_load_ASN1_strings, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_asn1_meth() \
	LP0(0x2e2, ASN1_METHOD *, X509_asn1_meth, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define RSAPrivateKey_asn1_meth() \
	LP0(0x2e8, ASN1_METHOD *, RSAPrivateKey_asn1_meth, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ASN1_IA5STRING_asn1_meth() \
	LP0(0x2ee, ASN1_METHOD *, ASN1_IA5STRING_asn1_meth, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ASN1_BIT_STRING_asn1_meth() \
	LP0(0x2f4, ASN1_METHOD *, ASN1_BIT_STRING_asn1_meth, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ASN1_TYPE_set_octetstring(a, data, len) \
	LP3(0x2fa, int, ASN1_TYPE_set_octetstring, ASN1_TYPE *, a, a0, unsigned char *, data, a1, int, len, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ASN1_TYPE_get_octetstring(a, data, max_len) \
	LP3(0x300, int, ASN1_TYPE_get_octetstring, ASN1_TYPE *, a, a0, unsigned char *, data, a1, int, max_len, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ASN1_TYPE_set_int_octetstring(a, num, data, len) \
	LP4(0x306, int, ASN1_TYPE_set_int_octetstring, ASN1_TYPE *, a, a0, long, num, a1, unsigned char *, data, a2, int, len, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ASN1_TYPE_get_int_octetstring(a, num, data, max_len) \
	LP4(0x30c, int, ASN1_TYPE_get_int_octetstring, ASN1_TYPE *, a, a0, long *, num, a1, unsigned char *, data, a2, int, max_len, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ASN1_seq_unpack(buf, len, d2i, free_func) \
	LP4FP(0x312, *, ASN1_seq_unpack, unsigned char *, buf, a0, int, len, a1, __fpt, d2i, a2, void *, free_func, a3, \
	, AMISSL_BASE_NAME, char * (*__fpt)(), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ASN1_seq_pack(safes, i2d, buf, len) \
	LP4FP(0x318, unsigned char *, ASN1_seq_pack, STACK *, safes, a0, __fpt, i2d, a1, unsigned char **, buf, a2, int *, len, a3, \
	, AMISSL_BASE_NAME, int (*__fpt)(), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ASN1_unpack_string(oct, d2i) \
	LP2FP(0x31e, void *, ASN1_unpack_string, ASN1_STRING *, oct, a0, __fpt, d2i, a1, \
	, AMISSL_BASE_NAME, char * (*__fpt)(), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ASN1_pack_string(obj, i2d, oct) \
	LP3FP(0x324, ASN1_STRING *, ASN1_pack_string, void *, obj, a0, __fpt, i2d, a1, ASN1_OCTET_STRING **, oct, a2, \
	, AMISSL_BASE_NAME, int (*__fpt)(), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define asn1_GetSequence(c, length) \
	LP2(0x32a, int, asn1_GetSequence, ASN1_CTX *, c, a0, long *, length, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define asn1_add_error(address, offset) \
	LP2NR(0x330, asn1_add_error, unsigned char *, address, a0, int, offset, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_set_ex_data(bio, idx, data) \
	LP3(0x336, int, BIO_set_ex_data, BIO *, bio, a0, int, idx, a1, char *, data, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_get_ex_data(bio, idx) \
	LP2(0x33c, char *, BIO_get_ex_data, BIO *, bio, a0, int, idx, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_get_ex_new_index(argl, argp, new_func, dup_func, free_func) \
	LP5FP(0x342, int, BIO_get_ex_new_index, long, argl, a0, char *, argp, a1, __fpt, new_func, a2, void *, dup_func, a3, void *, free_func, d0, \
	, AMISSL_BASE_NAME, int (*__fpt)(), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_s_file() \
	LP0(0x348, BIO_METHOD *, BIO_s_file, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_new_file(filename, mode) \
	LP2(0x34e, BIO *, BIO_new_file, const char *, filename, a0, const char *, mode, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_new_fp_amiga(stream, close_flag) \
	LP2(0x354, BIO *, BIO_new_fp_amiga, BPTR, stream, a0, int, close_flag, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_new(type) \
	LP1(0x35a, BIO *, BIO_new, BIO_METHOD *, type, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_set(a, type) \
	LP2(0x360, int, BIO_set, BIO *, a, a0, BIO_METHOD *, type, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_free(a) \
	LP1(0x366, int, BIO_free, BIO *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_read(b, data, len) \
	LP3(0x36c, int, BIO_read, BIO *, b, a0, void *, data, a1, int, len, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_gets(bp, buf, size) \
	LP3(0x372, int, BIO_gets, BIO *, bp, a0, char *, buf, a1, int, size, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_write(b, data, len) \
	LP3(0x378, int, BIO_write, BIO *, b, a0, const char *, data, a1, int, len, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_puts(bp, buf) \
	LP2(0x37e, int, BIO_puts, BIO *, bp, a0, const char *, buf, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_ctrl(bp, cmd, larg, parg) \
	LP4(0x384, long, BIO_ctrl, BIO *, bp, a0, int, cmd, a1, long, larg, a2, void *, parg, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_ptr_ctrl(bp, cmd, larg) \
	LP3(0x38a, char *, BIO_ptr_ctrl, BIO *, bp, a0, int, cmd, a1, long, larg, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_int_ctrl(bp, cmd, larg, iarg) \
	LP4(0x390, long, BIO_int_ctrl, BIO *, bp, a0, int, cmd, a1, long, larg, a2, int, iarg, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_push(b, append) \
	LP2(0x396, BIO *, BIO_push, BIO *, b, a0, BIO *, append, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_pop(b) \
	LP1(0x39c, BIO *, BIO_pop, BIO *, b, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_free_all(a) \
	LP1NR(0x3a2, BIO_free_all, BIO *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_find_type(b, bio_type) \
	LP2(0x3a8, BIO *, BIO_find_type, BIO *, b, a0, int, bio_type, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_get_retry_BIO(bio, reason) \
	LP2(0x3ae, BIO *, BIO_get_retry_BIO, BIO *, bio, a0, int *, reason, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_get_retry_reason(bio) \
	LP1(0x3b4, int, BIO_get_retry_reason, BIO *, bio, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_dup_chain(in) \
	LP1(0x3ba, BIO *, BIO_dup_chain, BIO *, in, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_debug_callback(bio, cmd, argp, argi, argl, ret) \
	LP6(0x3c0, long, BIO_debug_callback, BIO *, bio, a0, int, cmd, a1, const char *, argp, a2, int, argi, a3, long, argl, d0, long, ret, d1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_s_mem() \
	LP0(0x3c6, BIO_METHOD *, BIO_s_mem, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_s_socket() \
	LP0(0x3cc, BIO_METHOD *, BIO_s_socket, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_s_connect() \
	LP0(0x3d2, BIO_METHOD *, BIO_s_connect, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_s_accept() \
	LP0(0x3d8, BIO_METHOD *, BIO_s_accept, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_s_fd() \
	LP0(0x3de, BIO_METHOD *, BIO_s_fd, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_s_log() \
	LP0(0x3e4, BIO_METHOD *, BIO_s_log, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_s_null() \
	LP0(0x3ea, BIO_METHOD *, BIO_s_null, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_f_null() \
	LP0(0x3f0, BIO_METHOD *, BIO_f_null, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_f_buffer() \
	LP0(0x3f6, BIO_METHOD *, BIO_f_buffer, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_f_nbio_test() \
	LP0(0x3fc, BIO_METHOD *, BIO_f_nbio_test, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_sock_should_retry(i) \
	LP1(0x402, int, BIO_sock_should_retry, int, i, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_sock_non_fatal_error(error) \
	LP1(0x408, int, BIO_sock_non_fatal_error, int, error, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_fd_should_retry(i) \
	LP1(0x40e, int, BIO_fd_should_retry, int, i, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_fd_non_fatal_error(error) \
	LP1(0x414, int, BIO_fd_non_fatal_error, int, error, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_dump(b, bytes, len) \
	LP3(0x41a, int, BIO_dump, BIO *, b, a0, const char *, bytes, a1, int, len, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_gethostbyname(name) \
	LP1(0x420, struct hostent *, BIO_gethostbyname, const char *, name, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_sock_error(sock) \
	LP1(0x426, int, BIO_sock_error, int, sock, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_socket_ioctl(fd, type, arg) \
	LP3(0x42c, int, BIO_socket_ioctl, int, fd, a0, long, type, a1, unsigned long *, arg, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_socket_nbio(fd, mode) \
	LP2(0x432, int, BIO_socket_nbio, int, fd, a0, int, mode, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_get_port(str, port_ptr) \
	LP2(0x438, int, BIO_get_port, const char *, str, a0, unsigned short *, port_ptr, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_get_host_ip(str, ip) \
	LP2(0x43e, int, BIO_get_host_ip, const char *, str, a0, unsigned char *, ip, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_get_accept_socket(host_port, mode) \
	LP2(0x444, int, BIO_get_accept_socket, char *, host_port, a0, int, mode, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_accept(sock, ip_port) \
	LP2(0x44a, int, BIO_accept, int, sock, a0, char **, ip_port, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_sock_init() \
	LP0(0x450, int, BIO_sock_init, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_sock_cleanup() \
	LP0NR(0x456, BIO_sock_cleanup, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_set_tcp_ndelay(sock, turn_on) \
	LP2(0x45c, int, BIO_set_tcp_ndelay, int, sock, a0, int, turn_on, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ERR_load_BIO_strings() \
	LP0NR(0x462, ERR_load_BIO_strings, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_new_socket(sock, close_flag) \
	LP2(0x468, BIO *, BIO_new_socket, int, sock, a0, int, close_flag, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_new_fd(fd, close_flag) \
	LP2(0x46e, BIO *, BIO_new_fd, int, fd, a0, int, close_flag, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_new_connect(host_port) \
	LP1(0x474, BIO *, BIO_new_connect, char *, host_port, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_new_accept(host_port) \
	LP1(0x47a, BIO *, BIO_new_accept, char *, host_port, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_copy_next_retry(b) \
	LP1NR(0x480, BIO_copy_next_retry, BIO *, b, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_ghbn_ctrl(cmd, iarg, parg) \
	LP3(0x486, long, BIO_ghbn_ctrl, int, cmd, a0, int, iarg, a1, char *, parg, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_printfA(bio, args) \
	LP2(0x48c, int, BIO_printfA, BIO *, bio, a0, void *, args, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define BIO_printf(bio, tags...) \
	({ULONG _tags[] = {tags}; BIO_printfA((bio), (void *) _tags);})
#endif

#define BN_value_one() \
	LP0(0x492, BIGNUM *, BN_value_one, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_options() \
	LP0(0x498, char *, BN_options, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_CTX_new() \
	LP0(0x49e, BN_CTX *, BN_CTX_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_CTX_init(c) \
	LP1NR(0x4a4, BN_CTX_init, BN_CTX *, c, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_CTX_free(c) \
	LP1NR(0x4aa, BN_CTX_free, BN_CTX *, c, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_rand(rnd, bits, top, bottom) \
	LP4(0x4b0, int, BN_rand, BIGNUM *, rnd, a0, int, bits, a1, int, top, a2, int, bottom, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_num_bits(a) \
	LP1(0x4b6, int, BN_num_bits, const BIGNUM *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_num_bits_word(a) \
	LP1(0x4bc, int, BN_num_bits_word, BN_ULONG, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_new() \
	LP0(0x4c2, BIGNUM *, BN_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_init(a) \
	LP1NR(0x4c8, BN_init, BIGNUM *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_clear_free(a) \
	LP1NR(0x4ce, BN_clear_free, BIGNUM *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_copy(a, b) \
	LP2(0x4d4, BIGNUM *, BN_copy, BIGNUM *, a, a0, const BIGNUM *, b, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_bin2bn(s, len, ret) \
	LP3(0x4da, BIGNUM *, BN_bin2bn, const unsigned char *, s, a0, int, len, a1, BIGNUM *, ret, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_bn2bin(a, to) \
	LP2(0x4e0, int, BN_bn2bin, const BIGNUM *, a, a0, unsigned char *, to, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_mpi2bn(s, len, ret) \
	LP3(0x4e6, BIGNUM *, BN_mpi2bn, unsigned char *, s, a0, int, len, a1, BIGNUM *, ret, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_bn2mpi(a, to) \
	LP2(0x4ec, int, BN_bn2mpi, const BIGNUM *, a, a0, unsigned char *, to, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_sub(r, a, b) \
	LP3(0x4f2, int, BN_sub, BIGNUM *, r, a0, const BIGNUM *, a, a1, const BIGNUM *, b, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_usub(r, a, b) \
	LP3(0x4f8, int, BN_usub, BIGNUM *, r, a0, const BIGNUM *, a, a1, const BIGNUM *, b, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_uadd(r, a, b) \
	LP3(0x4fe, int, BN_uadd, BIGNUM *, r, a0, const BIGNUM *, a, a1, const BIGNUM *, b, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_add(r, a, b) \
	LP3(0x504, int, BN_add, BIGNUM *, r, a0, BIGNUM *, a, a1, BIGNUM *, b, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_mod(rem, m, d, ctx) \
	LP4(0x50a, int, BN_mod, BIGNUM *, rem, a0, const BIGNUM *, m, a1, const BIGNUM *, d, a2, BN_CTX *, ctx, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_div(dv, rem, m, d, ctx) \
	LP5(0x510, int, BN_div, BIGNUM *, dv, a0, BIGNUM *, rem, a1, const BIGNUM *, m, a2, const BIGNUM *, d, a3, BN_CTX *, ctx, d0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_mul(r, a, b, ctx) \
	LP4(0x516, int, BN_mul, BIGNUM *, r, a0, BIGNUM *, a, a1, BIGNUM *, b, a2, BN_CTX *, ctx, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_sqr(r, a, ctx) \
	LP3(0x51c, int, BN_sqr, BIGNUM *, r, a0, BIGNUM *, a, a1, BN_CTX *, ctx, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_mod_word(a, w) \
	LP2(0x522, BN_ULONG, BN_mod_word, BIGNUM *, a, a0, BN_ULONG, w, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_div_word(a, w) \
	LP2(0x528, BN_ULONG, BN_div_word, BIGNUM *, a, a0, BN_ULONG, w, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_mul_word(a, w) \
	LP2(0x52e, int, BN_mul_word, BIGNUM *, a, a0, BN_ULONG, w, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_add_word(a, w) \
	LP2(0x534, int, BN_add_word, BIGNUM *, a, a0, BN_ULONG, w, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_sub_word(a, w) \
	LP2(0x53a, int, BN_sub_word, BIGNUM *, a, a0, BN_ULONG, w, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_set_word(a, w) \
	LP2(0x540, int, BN_set_word, BIGNUM *, a, a0, BN_ULONG, w, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_get_word(a) \
	LP1(0x546, BN_ULONG, BN_get_word, BIGNUM *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_cmp(a, b) \
	LP2(0x54c, int, BN_cmp, const BIGNUM *, a, a0, const BIGNUM *, b, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_free(a) \
	LP1NR(0x552, BN_free, BIGNUM *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_is_bit_set(a, n) \
	LP2(0x558, int, BN_is_bit_set, const BIGNUM *, a, a0, int, n, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_lshift(r, a, n) \
	LP3(0x55e, int, BN_lshift, BIGNUM *, r, a0, const BIGNUM *, a, a1, int, n, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_lshift1(r, a) \
	LP2(0x564, int, BN_lshift1, BIGNUM *, r, a0, BIGNUM *, a, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_exp(r, a, p, ctx) \
	LP4(0x56a, int, BN_exp, BIGNUM *, r, a0, BIGNUM *, a, a1, BIGNUM *, p, a2, BN_CTX *, ctx, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_mod_exp(r, a, p, m, ctx) \
	LP5(0x570, int, BN_mod_exp, BIGNUM *, r, a0, BIGNUM *, a, a1, const BIGNUM *, p, a2, const BIGNUM *, m, a3, BN_CTX *, ctx, d0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_mod_exp_mont(r, a, p, m, ctx, m_ctx) \
	LP6(0x576, int, BN_mod_exp_mont, BIGNUM *, r, a0, BIGNUM *, a, a1, const BIGNUM *, p, a2, const BIGNUM *, m, a3, BN_CTX *, ctx, d0, BN_MONT_CTX *, m_ctx, d1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_mod_exp2_mont(r, a1, p1, a2, p2, m, ctx, m_ctx) \
	LP8(0x57c, int, BN_mod_exp2_mont, BIGNUM *, r, a0, BIGNUM *, a1, a1, BIGNUM *, p1, a2, BIGNUM *, a2, a3, BIGNUM *, p2, d0, BIGNUM *, m, d1, BN_CTX *, ctx, d2, BN_MONT_CTX *, m_ctx, d3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_mod_exp_simple(r, a, p, m, ctx) \
	LP5(0x582, int, BN_mod_exp_simple, BIGNUM *, r, a0, BIGNUM *, a, a1, BIGNUM *, p, a2, BIGNUM *, m, a3, BN_CTX *, ctx, d0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_mask_bits(a, n) \
	LP2(0x588, int, BN_mask_bits, BIGNUM *, a, a0, int, n, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_mod_mul(ret, a, b, m, ctx) \
	LP5(0x58e, int, BN_mod_mul, BIGNUM *, ret, a0, BIGNUM *, a, a1, BIGNUM *, b, a2, const BIGNUM *, m, a3, BN_CTX *, ctx, d0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_print(fp, a) \
	LP2(0x594, int, BN_print, BIO *, fp, a0, const BIGNUM *, a, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_reciprocal(r, m, len, ctx) \
	LP4(0x59a, int, BN_reciprocal, BIGNUM *, r, a0, BIGNUM *, m, a1, int, len, a2, BN_CTX *, ctx, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_rshift(r, a, n) \
	LP3(0x5a0, int, BN_rshift, BIGNUM *, r, a0, BIGNUM *, a, a1, int, n, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_rshift1(r, a) \
	LP2(0x5a6, int, BN_rshift1, BIGNUM *, r, a0, BIGNUM *, a, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_clear(a) \
	LP1NR(0x5ac, BN_clear, BIGNUM *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define bn_expand2(b, bits) \
	LP2(0x5b2, BIGNUM *, bn_expand2, BIGNUM *, b, a0, int, bits, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_dup(a) \
	LP1(0x5b8, BIGNUM *, BN_dup, const BIGNUM *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_ucmp(a, b) \
	LP2(0x5be, int, BN_ucmp, const BIGNUM *, a, a0, const BIGNUM *, b, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_set_bit(a, n) \
	LP2(0x5c4, int, BN_set_bit, BIGNUM *, a, a0, int, n, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_clear_bit(a, n) \
	LP2(0x5ca, int, BN_clear_bit, BIGNUM *, a, a0, int, n, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_bn2hex(a) \
	LP1(0x5d0, char *, BN_bn2hex, const BIGNUM *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_bn2dec(a) \
	LP1(0x5d6, char *, BN_bn2dec, const BIGNUM *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_hex2bn(a, str) \
	LP2(0x5dc, int, BN_hex2bn, BIGNUM **, a, a0, const char *, str, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_dec2bn(a, str) \
	LP2(0x5e2, int, BN_dec2bn, BIGNUM **, a, a0, const char *, str, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_gcd(r, in_a, in_b, ctx) \
	LP4(0x5e8, int, BN_gcd, BIGNUM *, r, a0, BIGNUM *, in_a, a1, BIGNUM *, in_b, a2, BN_CTX *, ctx, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_mod_inverse(ret, a, n, ctx) \
	LP4(0x5ee, BIGNUM *, BN_mod_inverse, BIGNUM *, ret, a0, BIGNUM *, a, a1, const BIGNUM *, n, a2, BN_CTX *, ctx, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_generate_prime(ret, bits, strong, add, rem, callback, cb_arg) \
	LP7FP(0x5f4, BIGNUM *, BN_generate_prime, BIGNUM *, ret, a0, int, bits, a1, int, strong, a2, BIGNUM *, add, a3, BIGNUM *, rem, d0, __fpt, callback, d1, void *, cb_arg, d2, \
	, AMISSL_BASE_NAME, void (*__fpt)(int, int, void *), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_is_prime(p, nchecks, callback, ctx, cb_arg) \
	LP5FP(0x5fa, int, BN_is_prime, BIGNUM *, p, a0, int, nchecks, a1, __fpt, callback, a2, BN_CTX *, ctx, a3, void *, cb_arg, d0, \
	, AMISSL_BASE_NAME, void (*__fpt)(int, int, void *), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ERR_load_BN_strings() \
	LP0NR(0x600, ERR_load_BN_strings, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define bn_mul_add_words(rp, ap, num, w) \
	LP4(0x606, BN_ULONG, bn_mul_add_words, BN_ULONG *, rp, a0, BN_ULONG *, ap, a1, int, num, a2, BN_ULONG, w, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define bn_mul_words(rp, ap, num, w) \
	LP4(0x60c, BN_ULONG, bn_mul_words, BN_ULONG *, rp, a0, BN_ULONG *, ap, a1, int, num, a2, BN_ULONG, w, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define bn_sqr_words(rp, ap, num) \
	LP3NR(0x612, bn_sqr_words, BN_ULONG *, rp, a0, BN_ULONG *, ap, a1, int, num, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define bn_div_words(h, l, d) \
	LP3(0x618, BN_ULONG, bn_div_words, BN_ULONG, h, a0, BN_ULONG, l, a1, BN_ULONG, d, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define bn_add_words(rp, ap, bp, num) \
	LP4(0x61e, BN_ULONG, bn_add_words, BN_ULONG *, rp, a0, BN_ULONG *, ap, a1, BN_ULONG *, bp, a2, int, num, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define bn_sub_words(rp, ap, bp, num) \
	LP4(0x624, BN_ULONG, bn_sub_words, BN_ULONG *, rp, a0, BN_ULONG *, ap, a1, BN_ULONG *, bp, a2, int, num, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_MONT_CTX_new() \
	LP0(0x62a, BN_MONT_CTX *, BN_MONT_CTX_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_MONT_CTX_init(ctx) \
	LP1NR(0x630, BN_MONT_CTX_init, BN_MONT_CTX *, ctx, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_mod_mul_montgomery(r, a, b, mont, ctx) \
	LP5(0x636, int, BN_mod_mul_montgomery, BIGNUM *, r, a0, BIGNUM *, a, a1, BIGNUM *, b, a2, BN_MONT_CTX *, mont, a3, BN_CTX *, ctx, d0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_from_montgomery(r, a, mont, ctx) \
	LP4(0x63c, int, BN_from_montgomery, BIGNUM *, r, a0, BIGNUM *, a, a1, BN_MONT_CTX *, mont, a2, BN_CTX *, ctx, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_MONT_CTX_free(mont) \
	LP1NR(0x642, BN_MONT_CTX_free, BN_MONT_CTX *, mont, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_MONT_CTX_set(mont, modulus, ctx) \
	LP3(0x648, int, BN_MONT_CTX_set, BN_MONT_CTX *, mont, a0, const BIGNUM *, modulus, a1, BN_CTX *, ctx, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_MONT_CTX_copy(to, from) \
	LP2(0x64e, BN_MONT_CTX *, BN_MONT_CTX_copy, BN_MONT_CTX *, to, a0, BN_MONT_CTX *, from, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_BLINDING_new(A, Ai, mod) \
	LP3(0x654, BN_BLINDING *, BN_BLINDING_new, BIGNUM *, A, a0, BIGNUM *, Ai, a1, BIGNUM *, mod, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_BLINDING_free(b) \
	LP1NR(0x65a, BN_BLINDING_free, BN_BLINDING *, b, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_BLINDING_update(b, ctx) \
	LP2(0x660, int, BN_BLINDING_update, BN_BLINDING *, b, a0, BN_CTX *, ctx, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_BLINDING_convert(n, r, ctx) \
	LP3(0x666, int, BN_BLINDING_convert, BIGNUM *, n, a0, BN_BLINDING *, r, a1, BN_CTX *, ctx, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_BLINDING_invert(n, b, ctx) \
	LP3(0x66c, int, BN_BLINDING_invert, BIGNUM *, n, a0, BN_BLINDING *, b, a1, BN_CTX *, ctx, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_set_params(mul, high, low, mont) \
	LP4NR(0x672, BN_set_params, int, mul, a0, int, high, a1, int, low, a2, int, mont, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_get_params(which) \
	LP1(0x678, int, BN_get_params, int, which, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_RECP_CTX_init(recp) \
	LP1NR(0x67e, BN_RECP_CTX_init, BN_RECP_CTX *, recp, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_RECP_CTX_new() \
	LP0(0x684, BN_RECP_CTX *, BN_RECP_CTX_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_RECP_CTX_free(recp) \
	LP1NR(0x68a, BN_RECP_CTX_free, BN_RECP_CTX *, recp, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_RECP_CTX_set(recp, rdiv, ctx) \
	LP3(0x690, int, BN_RECP_CTX_set, BN_RECP_CTX *, recp, a0, const BIGNUM *, rdiv, a1, BN_CTX *, ctx, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_mod_mul_reciprocal(r, x, y, recp, ctx) \
	LP5(0x696, int, BN_mod_mul_reciprocal, BIGNUM *, r, a0, BIGNUM *, x, a1, BIGNUM *, y, a2, BN_RECP_CTX *, recp, a3, BN_CTX *, ctx, d0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_mod_exp_recp(r, a, p, m, ctx) \
	LP5(0x69c, int, BN_mod_exp_recp, BIGNUM *, r, a0, const BIGNUM *, a, a1, const BIGNUM *, p, a2, const BIGNUM *, m, a3, BN_CTX *, ctx, d0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BN_div_recp(dv, rem, m, recp, ctx) \
	LP5(0x6a2, int, BN_div_recp, BIGNUM *, dv, a0, BIGNUM *, rem, a1, BIGNUM *, m, a2, BN_RECP_CTX *, recp, a3, BN_CTX *, ctx, d0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BUF_MEM_new() \
	LP0(0x6a8, BUF_MEM *, BUF_MEM_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BUF_MEM_free(a) \
	LP1NR(0x6ae, BUF_MEM_free, BUF_MEM *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BUF_MEM_grow(str, len) \
	LP2(0x6b4, int, BUF_MEM_grow, BUF_MEM *, str, a0, int, len, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BUF_strdup(str) \
	LP1(0x6ba, char *, BUF_strdup, const char *, str, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ERR_load_BUF_strings() \
	LP0NR(0x6c0, ERR_load_BUF_strings, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define COMP_CTX_new(meth) \
	LP1(0x6c6, COMP_CTX *, COMP_CTX_new, COMP_METHOD *, meth, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define COMP_CTX_free(ctx) \
	LP1NR(0x6cc, COMP_CTX_free, COMP_CTX *, ctx, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define COMP_compress_block(ctx, out, olen, in, ilen) \
	LP5(0x6d2, int, COMP_compress_block, COMP_CTX *, ctx, a0, unsigned char *, out, a1, int, olen, a2, unsigned char *, in, a3, int, ilen, d0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define COMP_expand_block(ctx, out, olen, in, ilen) \
	LP5(0x6d8, int, COMP_expand_block, COMP_CTX *, ctx, a0, unsigned char *, out, a1, int, olen, a2, unsigned char *, in, a3, int, ilen, d0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define COMP_rle() \
	LP0(0x6de, COMP_METHOD *, COMP_rle, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CONF_load(conf, file, eline) \
	LP3(0x6e4, LHASH *, CONF_load, LHASH *, conf, a0, const char *, file, a1, long *, eline, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CONF_get_section(conf, section) \
	LP2(0x6ea, *, CONF_get_section, LHASH *, conf, a0, char *, section, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CONF_get_string(conf, group, name) \
	LP3(0x6f0, char *, CONF_get_string, LHASH *, conf, a0, char *, group, a1, char *, name, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CONF_get_number(conf, group, name) \
	LP3(0x6f6, long, CONF_get_number, LHASH *, conf, a0, char *, group, a1, char *, name, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CONF_free(conf) \
	LP1NR(0x6fc, CONF_free, LHASH *, conf, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ERR_load_CONF_strings() \
	LP0NR(0x702, ERR_load_CONF_strings, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSLeay_version(type) \
	LP1(0x708, const char *, SSLeay_version, int, type, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSLeay() \
	LP0(0x70e, unsigned long, SSLeay, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CRYPTO_get_ex_new_index(idx, sk, argl, argp, new_func, dup_func, free_func) \
	LP7FP(0x714, int, CRYPTO_get_ex_new_index, int, idx, a0, STACK **, sk, a1, long, argl, a2, char *, argp, a3, __fpt, new_func, d0, void *, dup_func, d1, void *, free_func, d2, \
	, AMISSL_BASE_NAME, int (*__fpt)(), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CRYPTO_set_ex_data(ad, idx, val) \
	LP3(0x71a, int, CRYPTO_set_ex_data, CRYPTO_EX_DATA *, ad, a0, int, idx, a1, char *, val, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CRYPTO_get_ex_data(ad, idx) \
	LP2(0x720, char *, CRYPTO_get_ex_data, CRYPTO_EX_DATA *, ad, a0, int, idx, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CRYPTO_dup_ex_data(meth, from, to) \
	LP3(0x726, int, CRYPTO_dup_ex_data, STACK *, meth, a0, CRYPTO_EX_DATA *, from, a1, CRYPTO_EX_DATA *, to, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CRYPTO_free_ex_data(meth, obj, ad) \
	LP3NR(0x72c, CRYPTO_free_ex_data, STACK *, meth, a0, char *, obj, a1, CRYPTO_EX_DATA *, ad, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CRYPTO_new_ex_data(meth, obj, ad) \
	LP3NR(0x732, CRYPTO_new_ex_data, STACK *, meth, a0, char *, obj, a1, CRYPTO_EX_DATA *, ad, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CRYPTO_mem_ctrl(mode) \
	LP1(0x738, int, CRYPTO_mem_ctrl, int, mode, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CRYPTO_get_new_lockid(name) \
	LP1(0x73e, int, CRYPTO_get_new_lockid, char *, name, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CRYPTO_lock(mode, type, file, line) \
	LP4NR(0x744, CRYPTO_lock, int, mode, a0, int, type, a1, const char *, file, a2, int, line, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CRYPTO_set_locking_callback(func) \
	LP1NRFP(0x74a, CRYPTO_set_locking_callback, __fpt, func, a0, \
	, AMISSL_BASE_NAME, void (*__fpt)(int mode, int type, const char *file, int line), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CRYPTO_set_add_lock_callback(func) \
	LP1NRFP(0x756, CRYPTO_set_add_lock_callback, __fpt, func, a0, \
	, AMISSL_BASE_NAME, int (*__fpt)(int *num, int mount, int type, const char *file, int line), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CRYPTO_set_id_callback(func) \
	LP1NRFP(0x762, CRYPTO_set_id_callback, __fpt, func, a0, \
	, AMISSL_BASE_NAME, unsigned long (*__fpt)(void), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CRYPTO_thread_id() \
	LP0(0x76e, unsigned long, CRYPTO_thread_id, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CRYPTO_get_lock_name(type) \
	LP1(0x774, const char *, CRYPTO_get_lock_name, int, type, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CRYPTO_add_lock(pointer, amount, type, file, line) \
	LP5(0x77a, int, CRYPTO_add_lock, int *, pointer, a0, int, amount, a1, int, type, a2, const char *, file, a3, int, line, d0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CRYPTO_set_mem_functions(m, r, free_func) \
	LP3NRFP(0x780, CRYPTO_set_mem_functions, __fpt, m, a0, void *, r, a1, void *, free_func, a2, \
	, AMISSL_BASE_NAME, char * (*__fpt)(), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CRYPTO_get_mem_functions(m, r, f) \
	LP3NRFP(0x786, CRYPTO_get_mem_functions, __fpt, m, a0, void *, r, a1, void *, f, a2, \
	, AMISSL_BASE_NAME, char * (**__fpt)(), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CRYPTO_set_locked_mem_functions(m, free_func) \
	LP2NRFP(0x78c, CRYPTO_set_locked_mem_functions, __fpt, m, a0, void *, free_func, a1, \
	, AMISSL_BASE_NAME, char * (*__fpt)(), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CRYPTO_get_locked_mem_functions(m, f) \
	LP2NRFP(0x792, CRYPTO_get_locked_mem_functions, __fpt, m, a0, void *, f, a1, \
	, AMISSL_BASE_NAME, char * (**__fpt)(), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CRYPTO_malloc_locked(num) \
	LP1(0x798, void *, CRYPTO_malloc_locked, int, num, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CRYPTO_free_locked(a) \
	LP1NR(0x79e, CRYPTO_free_locked, void *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CRYPTO_malloc(num) \
	LP1(0x7a4, void *, CRYPTO_malloc, int, num, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CRYPTO_free(a) \
	LP1NR(0x7aa, CRYPTO_free, void *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CRYPTO_realloc(addr, num) \
	LP2(0x7b0, void *, CRYPTO_realloc, void *, addr, a0, int, num, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CRYPTO_remalloc(addr, num) \
	LP2(0x7b6, void *, CRYPTO_remalloc, void *, addr, a0, int, num, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CRYPTO_dbg_malloc(num, file, line) \
	LP3(0x7bc, void *, CRYPTO_dbg_malloc, int, num, a0, const char *, file, a1, int, line, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CRYPTO_dbg_realloc(addr, num, file, line) \
	LP4(0x7c2, void *, CRYPTO_dbg_realloc, void *, addr, a0, int, num, a1, const char *, file, a2, int, line, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CRYPTO_dbg_free(a) \
	LP1NR(0x7c8, CRYPTO_dbg_free, void *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CRYPTO_dbg_remalloc(addr, num, file, line) \
	LP4(0x7ce, void *, CRYPTO_dbg_remalloc, void *, addr, a0, int, num, a1, const char *, file, a2, int, line, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CRYPTO_mem_leaks(bio) \
	LP1NR(0x7d4, CRYPTO_mem_leaks, struct bio_st *, bio, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CRYPTO_mem_leaks_cb(cb) \
	LP1NRFP(0x7da, CRYPTO_mem_leaks_cb, __fpt, cb, a0, \
	, AMISSL_BASE_NAME, void (*__fpt)(), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ERR_load_CRYPTO_strings() \
	LP0NR(0x7e0, ERR_load_CRYPTO_strings, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define DH_new() \
	LP0(0x7e6, DH *, DH_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define DH_free(dh) \
	LP1NR(0x7ec, DH_free, DH *, dh, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define DH_size(dh) \
	LP1(0x7f2, int, DH_size, DH *, dh, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define DH_generate_parameters(prime_len, generator, callback, cb_arg) \
	LP4FP(0x7f8, DH *, DH_generate_parameters, int, prime_len, a0, int, generator, a1, __fpt, callback, a2, void *, cb_arg, a3, \
	, AMISSL_BASE_NAME, void (*__fpt)(int, int, void *), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define DH_check(dh, codes) \
	LP2(0x7fe, int, DH_check, DH *, dh, a0, int *, codes, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define DH_generate_key(dh) \
	LP1(0x804, int, DH_generate_key, DH *, dh, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define DH_compute_key(key, pub_key, dh) \
	LP3(0x80a, int, DH_compute_key, unsigned char *, key, a0, BIGNUM *, pub_key, a1, DH *, dh, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_DHparams(a, pp, length) \
	LP3(0x810, DH *, d2i_DHparams, DH **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_DHparams(a, pp) \
	LP2(0x816, int, i2d_DHparams, DH *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define DHparams_print(bp, x) \
	LP2(0x81c, int, DHparams_print, BIO *, bp, a0, DH *, x, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ERR_load_DH_strings() \
	LP0NR(0x822, ERR_load_DH_strings, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define DSA_SIG_new() \
	LP0(0x828, DSA_SIG *, DSA_SIG_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define DSA_SIG_free(a) \
	LP1NR(0x82e, DSA_SIG_free, DSA_SIG *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_DSA_SIG(a, pp) \
	LP2(0x834, int, i2d_DSA_SIG, DSA_SIG *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_DSA_SIG(v, pp, length) \
	LP3(0x83a, DSA_SIG *, d2i_DSA_SIG, DSA_SIG **, v, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define DSA_do_sign(dgst, dlen, dsa) \
	LP3(0x840, DSA_SIG *, DSA_do_sign, const unsigned char *, dgst, a0, int, dlen, a1, DSA *, dsa, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define DSA_do_verify(dgst, dgst_len, sig, dsa) \
	LP4(0x846, int, DSA_do_verify, const unsigned char *, dgst, a0, int, dgst_len, a1, DSA_SIG *, sig, a2, DSA *, dsa, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define DSA_new() \
	LP0(0x84c, DSA *, DSA_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define DSA_size(a) \
	LP1(0x852, int, DSA_size, DSA *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define DSA_sign_setup(dsa, ctx_in, kinvp, rp) \
	LP4(0x858, int, DSA_sign_setup, DSA *, dsa, a0, BN_CTX *, ctx_in, a1, BIGNUM **, kinvp, a2, BIGNUM **, rp, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define DSA_sign(type, dgst, dlen, sig, siglen, dsa) \
	LP6(0x85e, int, DSA_sign, int, type, a0, const unsigned char *, dgst, a1, int, dlen, a2, unsigned char *, sig, a3, unsigned int *, siglen, d0, DSA *, dsa, d1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define DSA_verify(type, dgst, dgst_len, sigbuf, siglen, dsa) \
	LP6(0x864, int, DSA_verify, int, type, a0, const unsigned char *, dgst, a1, int, dgst_len, a2, unsigned char *, sigbuf, a3, int, siglen, d0, DSA *, dsa, d1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define DSA_free(r) \
	LP1NR(0x86a, DSA_free, DSA *, r, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ERR_load_DSA_strings() \
	LP0NR(0x870, ERR_load_DSA_strings, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_DSAPublicKey(a, pp, length) \
	LP3(0x876, DSA *, d2i_DSAPublicKey, DSA **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_DSAPrivateKey(a, pp, length) \
	LP3(0x87c, DSA *, d2i_DSAPrivateKey, DSA **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_DSAparams(a, pp, length) \
	LP3(0x882, DSA *, d2i_DSAparams, DSA **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define DSA_generate_parameters(bits, seed, seed_len, counter_ret, h_ret, callback, cb_arg) \
	LP7FP(0x888, DSA *, DSA_generate_parameters, int, bits, a0, unsigned char *, seed, a1, int, seed_len, a2, int *, counter_ret, a3, unsigned long *, h_ret, d0, __fpt, callback, d1, char *, cb_arg, d2, \
	, AMISSL_BASE_NAME, void (*__fpt)(), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define DSA_generate_key(a) \
	LP1(0x88e, int, DSA_generate_key, DSA *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_DSAPublicKey(a, pp) \
	LP2(0x894, int, i2d_DSAPublicKey, DSA *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_DSAPrivateKey(a, pp) \
	LP2(0x89a, int, i2d_DSAPrivateKey, DSA *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_DSAparams(a, pp) \
	LP2(0x8a0, int, i2d_DSAparams, DSA *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define DSAparams_print(bp, x) \
	LP2(0x8a6, int, DSAparams_print, BIO *, bp, a0, DSA *, x, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define DSA_print(bp, x, off) \
	LP3(0x8ac, int, DSA_print, BIO *, bp, a0, DSA *, x, a1, int, off, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define DSA_is_prime(q, callback, cb_arg) \
	LP3FP(0x8b2, int, DSA_is_prime, BIGNUM *, q, a0, __fpt, callback, a1, char *, cb_arg, a2, \
	, AMISSL_BASE_NAME, void (*__fpt)(), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ERR_put_error(lib, func, reason, file, line) \
	LP5NR(0x8b8, ERR_put_error, int, lib, a0, int, func, a1, int, reason, a2, const char *, file, a3, int, line, d0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ERR_set_error_data(data, flags) \
	LP2NR(0x8be, ERR_set_error_data, char *, data, a0, int, flags, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ERR_get_error() \
	LP0(0x8c4, unsigned long, ERR_get_error, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ERR_get_error_line(file, line) \
	LP2(0x8ca, unsigned long, ERR_get_error_line, const char **, file, a0, int *, line, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ERR_get_error_line_data(file, line, data, flags) \
	LP4(0x8d0, unsigned long, ERR_get_error_line_data, const char **, file, a0, int *, line, a1, const char **, data, a2, int *, flags, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ERR_peek_error() \
	LP0(0x8d6, unsigned long, ERR_peek_error, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ERR_peek_error_line(file, line) \
	LP2(0x8dc, unsigned long, ERR_peek_error_line, const char **, file, a0, int *, line, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ERR_peek_error_line_data(file, line, data, flags) \
	LP4(0x8e2, unsigned long, ERR_peek_error_line_data, const char **, file, a0, int *, line, a1, const char **, data, a2, int *, flags, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ERR_clear_error() \
	LP0NR(0x8e8, ERR_clear_error, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ERR_error_string(e, buf) \
	LP2(0x8ee, char *, ERR_error_string, unsigned long, e, a0, char *, buf, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ERR_lib_error_string(e) \
	LP1(0x8f4, const char *, ERR_lib_error_string, unsigned long, e, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ERR_func_error_string(e) \
	LP1(0x8fa, const char *, ERR_func_error_string, unsigned long, e, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ERR_reason_error_string(e) \
	LP1(0x900, const char *, ERR_reason_error_string, unsigned long, e, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ERR_print_errors(bp) \
	LP1NR(0x906, ERR_print_errors, BIO *, bp, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ERR_add_error_dataA(num, data) \
	LP2NR(0x90c, ERR_add_error_dataA, int, num, a0, void *, data, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define ERR_add_error_data(num, tags...) \
	({ULONG _tags[] = {tags}; ERR_add_error_dataA((num), (void *) _tags);})
#endif

#define ERR_load_strings(lib, str) \
	LP2NR(0x912, ERR_load_strings, int, lib, a0, ERR_STRING_DATA *, str, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ERR_load_ERR_strings() \
	LP0NR(0x918, ERR_load_ERR_strings, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ERR_load_crypto_strings() \
	LP0NR(0x91e, ERR_load_crypto_strings, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ERR_free_strings() \
	LP0NR(0x924, ERR_free_strings, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ERR_remove_state(pid) \
	LP1NR(0x92a, ERR_remove_state, unsigned long, pid, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ERR_get_state() \
	LP0(0x930, ERR_STATE *, ERR_get_state, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ERR_get_string_table() \
	LP0(0x936, LHASH *, ERR_get_string_table, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ERR_get_err_state_table() \
	LP0(0x93c, LHASH *, ERR_get_err_state_table, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ERR_get_next_error_library() \
	LP0(0x942, int, ERR_get_next_error_library, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_MD_CTX_copy(out, in) \
	LP2(0x948, int, EVP_MD_CTX_copy, EVP_MD_CTX *, out, a0, EVP_MD_CTX *, in, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_DigestInit(ctx, type) \
	LP2NR(0x94e, EVP_DigestInit, EVP_MD_CTX *, ctx, a0, const EVP_MD *, type, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_DigestUpdate(ctx, d, cnt) \
	LP3NR(0x954, EVP_DigestUpdate, EVP_MD_CTX *, ctx, a0, const unsigned char *, d, a1, unsigned int, cnt, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_DigestFinal(ctx, md, s) \
	LP3NR(0x95a, EVP_DigestFinal, EVP_MD_CTX *, ctx, a0, unsigned char *, md, a1, unsigned int *, s, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_read_pw_string(buf, length, prompt, verify) \
	LP4(0x960, int, EVP_read_pw_string, char *, buf, a0, int, length, a1, const char *, prompt, a2, int, verify, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_set_pw_prompt(prompt) \
	LP1NR(0x966, EVP_set_pw_prompt, char *, prompt, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_get_pw_prompt() \
	LP0(0x96c, char *, EVP_get_pw_prompt, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_BytesToKey(type, md, salt, data, datal, count, key, iv) \
	LP8(0x972, int, EVP_BytesToKey, const EVP_CIPHER *, type, a0, EVP_MD *, md, a1, unsigned char *, salt, a2, unsigned char *, data, a3, int, datal, d0, int, count, d1, unsigned char *, key, d2, unsigned char *, iv, d3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_EncryptInit(ctx, type, key, iv) \
	LP4NR(0x978, EVP_EncryptInit, EVP_CIPHER_CTX *, ctx, a0, const EVP_CIPHER *, type, a1, unsigned char *, key, a2, unsigned char *, iv, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_EncryptUpdate(ctx, out, outl, in, inl) \
	LP5NR(0x97e, EVP_EncryptUpdate, EVP_CIPHER_CTX *, ctx, a0, unsigned char *, out, a1, int *, outl, a2, unsigned char *, in, a3, int, inl, d0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_EncryptFinal(ctx, out, outl) \
	LP3NR(0x984, EVP_EncryptFinal, EVP_CIPHER_CTX *, ctx, a0, unsigned char *, out, a1, int *, outl, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_DecryptInit(ctx, type, key, iv) \
	LP4NR(0x98a, EVP_DecryptInit, EVP_CIPHER_CTX *, ctx, a0, const EVP_CIPHER *, type, a1, unsigned char *, key, a2, unsigned char *, iv, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_DecryptUpdate(ctx, out, outl, in, inl) \
	LP5NR(0x990, EVP_DecryptUpdate, EVP_CIPHER_CTX *, ctx, a0, unsigned char *, out, a1, int *, outl, a2, unsigned char *, in, a3, int, inl, d0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_DecryptFinal(ctx, outm, outl) \
	LP3(0x996, int, EVP_DecryptFinal, EVP_CIPHER_CTX *, ctx, a0, unsigned char *, outm, a1, int *, outl, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_CipherInit(ctx, type, key, iv, enc) \
	LP5NR(0x99c, EVP_CipherInit, EVP_CIPHER_CTX *, ctx, a0, const EVP_CIPHER *, type, a1, unsigned char *, key, a2, unsigned char *, iv, a3, int, enc, d0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_CipherUpdate(ctx, out, outl, in, inl) \
	LP5NR(0x9a2, EVP_CipherUpdate, EVP_CIPHER_CTX *, ctx, a0, unsigned char *, out, a1, int *, outl, a2, unsigned char *, in, a3, int, inl, d0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_CipherFinal(ctx, outm, outl) \
	LP3(0x9a8, int, EVP_CipherFinal, EVP_CIPHER_CTX *, ctx, a0, unsigned char *, outm, a1, int *, outl, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_SignFinal(ctx, md, s, pkey) \
	LP4(0x9ae, int, EVP_SignFinal, EVP_MD_CTX *, ctx, a0, unsigned char *, md, a1, unsigned int *, s, a2, EVP_PKEY *, pkey, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_VerifyFinal(ctx, sigbuf, siglen, pkey) \
	LP4(0x9b4, int, EVP_VerifyFinal, EVP_MD_CTX *, ctx, a0, unsigned char *, sigbuf, a1, unsigned int, siglen, a2, EVP_PKEY *, pkey, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_OpenInit(ctx, type, ek, ekl, iv, priv) \
	LP6(0x9ba, int, EVP_OpenInit, EVP_CIPHER_CTX *, ctx, a0, EVP_CIPHER *, type, a1, unsigned char *, ek, a2, int, ekl, a3, unsigned char *, iv, d0, EVP_PKEY *, priv, d1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_OpenFinal(ctx, out, outl) \
	LP3(0x9c0, int, EVP_OpenFinal, EVP_CIPHER_CTX *, ctx, a0, unsigned char *, out, a1, int *, outl, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_SealInit(ctx, type, ek, ekl, iv, pubk, npubk) \
	LP7(0x9c6, int, EVP_SealInit, EVP_CIPHER_CTX *, ctx, a0, EVP_CIPHER *, type, a1, unsigned char **, ek, a2, int *, ekl, a3, unsigned char *, iv, d0, EVP_PKEY **, pubk, d1, int, npubk, d2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_SealFinal(ctx, out, outl) \
	LP3NR(0x9cc, EVP_SealFinal, EVP_CIPHER_CTX *, ctx, a0, unsigned char *, out, a1, int *, outl, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_EncodeInit(ctx) \
	LP1NR(0x9d2, EVP_EncodeInit, EVP_ENCODE_CTX *, ctx, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_EncodeUpdate(ctx, out, outl, in, inl) \
	LP5NR(0x9d8, EVP_EncodeUpdate, EVP_ENCODE_CTX *, ctx, a0, unsigned char *, out, a1, int *, outl, a2, unsigned char *, in, a3, int, inl, d0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_EncodeFinal(ctx, out, outl) \
	LP3NR(0x9de, EVP_EncodeFinal, EVP_ENCODE_CTX *, ctx, a0, unsigned char *, out, a1, int *, outl, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_EncodeBlock(t, f, n) \
	LP3(0x9e4, int, EVP_EncodeBlock, unsigned char *, t, a0, unsigned char *, f, a1, int, n, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_DecodeInit(ctx) \
	LP1NR(0x9ea, EVP_DecodeInit, EVP_ENCODE_CTX *, ctx, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_DecodeUpdate(ctx, out, outl, in, inl) \
	LP5(0x9f0, int, EVP_DecodeUpdate, EVP_ENCODE_CTX *, ctx, a0, unsigned char *, out, a1, int *, outl, a2, unsigned char *, in, a3, int, inl, d0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_DecodeFinal(ctx, out, outl) \
	LP3(0x9f6, int, EVP_DecodeFinal, EVP_ENCODE_CTX *, ctx, a0, unsigned char *, out, a1, int *, outl, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_DecodeBlock(t, f, n) \
	LP3(0x9fc, int, EVP_DecodeBlock, unsigned char *, t, a0, unsigned char *, f, a1, int, n, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ERR_load_EVP_strings() \
	LP0NR(0xa02, ERR_load_EVP_strings, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_CIPHER_CTX_init(a) \
	LP1NR(0xa08, EVP_CIPHER_CTX_init, EVP_CIPHER_CTX *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_CIPHER_CTX_cleanup(a) \
	LP1NR(0xa0e, EVP_CIPHER_CTX_cleanup, EVP_CIPHER_CTX *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_f_md() \
	LP0(0xa14, BIO_METHOD *, BIO_f_md, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_f_base64() \
	LP0(0xa1a, BIO_METHOD *, BIO_f_base64, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_f_cipher() \
	LP0(0xa20, BIO_METHOD *, BIO_f_cipher, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_f_reliable() \
	LP0(0xa26, BIO_METHOD *, BIO_f_reliable, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_set_cipher(b, c, k, i, enc) \
	LP5NR(0xa2c, BIO_set_cipher, BIO *, b, a0, const EVP_CIPHER *, c, a1, unsigned char *, k, a2, unsigned char *, i, a3, int, enc, d0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_md_null() \
	LP0(0xa32, EVP_MD *, EVP_md_null, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_md2() \
	LP0(0xa38, EVP_MD *, EVP_md2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_md5() \
	LP0(0xa3e, EVP_MD *, EVP_md5, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_sha() \
	LP0(0xa44, EVP_MD *, EVP_sha, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_sha1() \
	LP0(0xa4a, EVP_MD *, EVP_sha1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_dss() \
	LP0(0xa50, EVP_MD *, EVP_dss, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_dss1() \
	LP0(0xa56, EVP_MD *, EVP_dss1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_mdc2() \
	LP0(0xa5c, EVP_MD *, EVP_mdc2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_ripemd160() \
	LP0(0xa62, EVP_MD *, EVP_ripemd160, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_enc_null() \
	LP0(0xa68, EVP_CIPHER *, EVP_enc_null, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_des_ecb() \
	LP0(0xa6e, EVP_CIPHER *, EVP_des_ecb, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_des_ede() \
	LP0(0xa74, EVP_CIPHER *, EVP_des_ede, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_des_ede3() \
	LP0(0xa7a, EVP_CIPHER *, EVP_des_ede3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_des_cfb() \
	LP0(0xa80, EVP_CIPHER *, EVP_des_cfb, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_des_ede_cfb() \
	LP0(0xa86, EVP_CIPHER *, EVP_des_ede_cfb, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_des_ede3_cfb() \
	LP0(0xa8c, EVP_CIPHER *, EVP_des_ede3_cfb, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_des_ofb() \
	LP0(0xa92, EVP_CIPHER *, EVP_des_ofb, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_des_ede_ofb() \
	LP0(0xa98, EVP_CIPHER *, EVP_des_ede_ofb, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_des_ede3_ofb() \
	LP0(0xa9e, EVP_CIPHER *, EVP_des_ede3_ofb, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_des_cbc() \
	LP0(0xaa4, EVP_CIPHER *, EVP_des_cbc, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_des_ede_cbc() \
	LP0(0xaaa, EVP_CIPHER *, EVP_des_ede_cbc, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_des_ede3_cbc() \
	LP0(0xab0, EVP_CIPHER *, EVP_des_ede3_cbc, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_desx_cbc() \
	LP0(0xab6, EVP_CIPHER *, EVP_desx_cbc, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_rc4() \
	LP0(0xabc, EVP_CIPHER *, EVP_rc4, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_rc4_40() \
	LP0(0xac2, EVP_CIPHER *, EVP_rc4_40, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_idea_ecb() \
	LP0(0xac8, EVP_CIPHER *, EVP_idea_ecb, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_idea_cfb() \
	LP0(0xace, EVP_CIPHER *, EVP_idea_cfb, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_idea_ofb() \
	LP0(0xad4, EVP_CIPHER *, EVP_idea_ofb, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_idea_cbc() \
	LP0(0xada, EVP_CIPHER *, EVP_idea_cbc, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_rc2_ecb() \
	LP0(0xae0, EVP_CIPHER *, EVP_rc2_ecb, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_rc2_cbc() \
	LP0(0xae6, EVP_CIPHER *, EVP_rc2_cbc, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_rc2_40_cbc() \
	LP0(0xaec, EVP_CIPHER *, EVP_rc2_40_cbc, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_rc2_64_cbc() \
	LP0(0xaf2, EVP_CIPHER *, EVP_rc2_64_cbc, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_rc2_cfb() \
	LP0(0xaf8, EVP_CIPHER *, EVP_rc2_cfb, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_rc2_ofb() \
	LP0(0xafe, EVP_CIPHER *, EVP_rc2_ofb, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_bf_ecb() \
	LP0(0xb04, EVP_CIPHER *, EVP_bf_ecb, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_bf_cbc() \
	LP0(0xb0a, EVP_CIPHER *, EVP_bf_cbc, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_bf_cfb() \
	LP0(0xb10, EVP_CIPHER *, EVP_bf_cfb, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_bf_ofb() \
	LP0(0xb16, EVP_CIPHER *, EVP_bf_ofb, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_cast5_ecb() \
	LP0(0xb1c, EVP_CIPHER *, EVP_cast5_ecb, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_cast5_cbc() \
	LP0(0xb22, EVP_CIPHER *, EVP_cast5_cbc, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_cast5_cfb() \
	LP0(0xb28, EVP_CIPHER *, EVP_cast5_cfb, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_cast5_ofb() \
	LP0(0xb2e, EVP_CIPHER *, EVP_cast5_ofb, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_rc5_32_12_16_cbc() \
	LP0(0xb34, EVP_CIPHER *, EVP_rc5_32_12_16_cbc, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_rc5_32_12_16_ecb() \
	LP0(0xb3a, EVP_CIPHER *, EVP_rc5_32_12_16_ecb, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_rc5_32_12_16_cfb() \
	LP0(0xb40, EVP_CIPHER *, EVP_rc5_32_12_16_cfb, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_rc5_32_12_16_ofb() \
	LP0(0xb46, EVP_CIPHER *, EVP_rc5_32_12_16_ofb, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSLeay_add_all_algorithms() \
	LP0NR(0xb4c, SSLeay_add_all_algorithms, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSLeay_add_all_ciphers() \
	LP0NR(0xb52, SSLeay_add_all_ciphers, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSLeay_add_all_digests() \
	LP0NR(0xb58, SSLeay_add_all_digests, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_add_cipher(cipher) \
	LP1(0xb5e, int, EVP_add_cipher, EVP_CIPHER *, cipher, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_add_digest(digest) \
	LP1(0xb64, int, EVP_add_digest, EVP_MD *, digest, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_get_cipherbyname(name) \
	LP1(0xb6a, const EVP_CIPHER *, EVP_get_cipherbyname, const char *, name, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_get_digestbyname(name) \
	LP1(0xb70, const EVP_MD *, EVP_get_digestbyname, const char *, name, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_cleanup() \
	LP0NR(0xb76, EVP_cleanup, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_PKEY_decrypt(dec_key, enc_key, enc_key_len, private_key) \
	LP4(0xb7c, int, EVP_PKEY_decrypt, unsigned char *, dec_key, a0, unsigned char *, enc_key, a1, int, enc_key_len, a2, EVP_PKEY *, private_key, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_PKEY_encrypt(enc_key, key, key_len, pub_key) \
	LP4(0xb82, int, EVP_PKEY_encrypt, unsigned char *, enc_key, a0, unsigned char *, key, a1, int, key_len, a2, EVP_PKEY *, pub_key, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_PKEY_type(type) \
	LP1(0xb88, int, EVP_PKEY_type, int, type, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_PKEY_bits(pkey) \
	LP1(0xb8e, int, EVP_PKEY_bits, EVP_PKEY *, pkey, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_PKEY_size(pkey) \
	LP1(0xb94, int, EVP_PKEY_size, EVP_PKEY *, pkey, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_PKEY_assign(pkey, type, key) \
	LP3(0xb9a, int, EVP_PKEY_assign, EVP_PKEY *, pkey, a0, int, type, a1, char *, key, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_PKEY_new() \
	LP0(0xba0, EVP_PKEY *, EVP_PKEY_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_PKEY_free(pkey) \
	LP1NR(0xba6, EVP_PKEY_free, EVP_PKEY *, pkey, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_PublicKey(type, a, pp, length) \
	LP4(0xbac, EVP_PKEY *, d2i_PublicKey, int, type, a0, EVP_PKEY **, a, a1, unsigned char **, pp, a2, long, length, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_PublicKey(a, pp) \
	LP2(0xbb2, int, i2d_PublicKey, EVP_PKEY *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_PrivateKey(type, a, pp, length) \
	LP4(0xbb8, EVP_PKEY *, d2i_PrivateKey, int, type, a0, EVP_PKEY **, a, a1, unsigned char **, pp, a2, long, length, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_PrivateKey(a, pp) \
	LP2(0xbbe, int, i2d_PrivateKey, EVP_PKEY *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_PKEY_copy_parameters(to, from) \
	LP2(0xbc4, int, EVP_PKEY_copy_parameters, EVP_PKEY *, to, a0, EVP_PKEY *, from, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_PKEY_missing_parameters(pkey) \
	LP1(0xbca, int, EVP_PKEY_missing_parameters, EVP_PKEY *, pkey, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_PKEY_save_parameters(pkey, mode) \
	LP2(0xbd0, int, EVP_PKEY_save_parameters, EVP_PKEY *, pkey, a0, int, mode, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_PKEY_cmp_parameters(a, b) \
	LP2(0xbd6, int, EVP_PKEY_cmp_parameters, EVP_PKEY *, a, a0, EVP_PKEY *, b, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_CIPHER_type(ctx) \
	LP1(0xbdc, int, EVP_CIPHER_type, const EVP_CIPHER *, ctx, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_CIPHER_param_to_asn1(c, type) \
	LP2(0xbe2, int, EVP_CIPHER_param_to_asn1, EVP_CIPHER_CTX *, c, a0, ASN1_TYPE *, type, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_CIPHER_asn1_to_param(c, type) \
	LP2(0xbe8, int, EVP_CIPHER_asn1_to_param, EVP_CIPHER_CTX *, c, a0, ASN1_TYPE *, type, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_CIPHER_set_asn1_iv(c, type) \
	LP2(0xbee, int, EVP_CIPHER_set_asn1_iv, EVP_CIPHER_CTX *, c, a0, ASN1_TYPE *, type, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_CIPHER_get_asn1_iv(c, type) \
	LP2(0xbf4, int, EVP_CIPHER_get_asn1_iv, EVP_CIPHER_CTX *, c, a0, ASN1_TYPE *, type, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define HMAC_Init(ctx, key, len, md) \
	LP4NR(0xbfa, HMAC_Init, HMAC_CTX *, ctx, a0, const void *, key, a1, int, len, a2, const EVP_MD *, md, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define HMAC_Update(ctx, key, len) \
	LP3NR(0xc00, HMAC_Update, HMAC_CTX *, ctx, a0, unsigned char *, key, a1, int, len, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define HMAC_Final(ctx, md, len) \
	LP3NR(0xc06, HMAC_Final, HMAC_CTX *, ctx, a0, unsigned char *, md, a1, unsigned int *, len, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define HMAC_cleanup(ctx) \
	LP1NR(0xc0c, HMAC_cleanup, HMAC_CTX *, ctx, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define HMAC(evp_md, key, key_len, d, n, md, md_len) \
	LP7(0xc12, unsigned char *, HMAC, const EVP_MD *, evp_md, a0, const void *, key, a1, int, key_len, a2, unsigned char *, d, a3, int, n, d0, unsigned char *, md, d1, unsigned int *, md_len, d2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define lh_new(h, c) \
	LP2FP(0xc18, LHASH *, lh_new, __fpt, h, a0, void *, c, a1, \
	, AMISSL_BASE_NAME, unsigned long (*__fpt)(), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define lh_free(lh) \
	LP1NR(0xc1e, lh_free, LHASH *, lh, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define lh_insert(lh, data) \
	LP2(0xc24, char *, lh_insert, LHASH *, lh, a0, char *, data, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define lh_delete(lh, data) \
	LP2(0xc2a, char *, lh_delete, LHASH *, lh, a0, char *, data, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define lh_retrieve(lh, data) \
	LP2(0xc30, char *, lh_retrieve, LHASH *, lh, a0, char *, data, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define lh_doall(lh, func) \
	LP2NRFP(0xc36, lh_doall, LHASH *, lh, a0, __fpt, func, a1, \
	, AMISSL_BASE_NAME, void (*__fpt)(), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define lh_doall_arg(lh, func, arg) \
	LP3NRFP(0xc3c, lh_doall_arg, LHASH *, lh, a0, __fpt, func, a1, char *, arg, a2, \
	, AMISSL_BASE_NAME, void (*__fpt)(), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define lh_strhash(c) \
	LP1(0xc42, unsigned long, lh_strhash, const char *, c, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define lh_stats_bio(lh, out) \
	LP2NR(0xc48, lh_stats_bio, LHASH *, lh, a0, BIO *, out, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define lh_node_stats_bio(lh, out) \
	LP2NR(0xc4e, lh_node_stats_bio, LHASH *, lh, a0, BIO *, out, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define lh_node_usage_stats_bio(lh, out) \
	LP2NR(0xc54, lh_node_usage_stats_bio, LHASH *, lh, a0, BIO *, out, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define OBJ_NAME_init() \
	LP0(0xc5a, int, OBJ_NAME_init, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define OBJ_NAME_new_index(hash_func, cmp_func, free_func) \
	LP3FP(0xc60, int, OBJ_NAME_new_index, __fpt, hash_func, a0, void *, cmp_func, a1, void *, free_func, a2, \
	, AMISSL_BASE_NAME, unsigned long (*__fpt)(), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define OBJ_NAME_get(name, type) \
	LP2(0xc66, const char *, OBJ_NAME_get, const char *, name, a0, int, type, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define OBJ_NAME_add(name, type, data) \
	LP3(0xc6c, int, OBJ_NAME_add, const char *, name, a0, int, type, a1, const char *, data, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define OBJ_NAME_remove(name, type) \
	LP2(0xc72, int, OBJ_NAME_remove, const char *, name, a0, int, type, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define OBJ_NAME_cleanup(type) \
	LP1NR(0xc78, OBJ_NAME_cleanup, int, type, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define OBJ_dup(o) \
	LP1(0xc7e, ASN1_OBJECT *, OBJ_dup, ASN1_OBJECT *, o, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define OBJ_nid2obj(n) \
	LP1(0xc84, ASN1_OBJECT *, OBJ_nid2obj, int, n, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define OBJ_nid2ln(n) \
	LP1(0xc8a, const char *, OBJ_nid2ln, int, n, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define OBJ_nid2sn(n) \
	LP1(0xc90, const char *, OBJ_nid2sn, int, n, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define OBJ_obj2nid(o) \
	LP1(0xc96, int, OBJ_obj2nid, ASN1_OBJECT *, o, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define OBJ_txt2obj(s, no_name) \
	LP2(0xc9c, ASN1_OBJECT *, OBJ_txt2obj, const char *, s, a0, int, no_name, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define OBJ_txt2nid(s) \
	LP1(0xca2, int, OBJ_txt2nid, char *, s, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define OBJ_ln2nid(s) \
	LP1(0xca8, int, OBJ_ln2nid, const char *, s, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define OBJ_sn2nid(s) \
	LP1(0xcae, int, OBJ_sn2nid, const char *, s, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define OBJ_cmp(a, b) \
	LP2(0xcb4, int, OBJ_cmp, ASN1_OBJECT *, a, a0, ASN1_OBJECT *, b, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define OBJ_bsearch(key, base, num, size, cmp) \
	LP5FP(0xcba, char *, OBJ_bsearch, char *, key, a0, char *, base, a1, int, num, a2, int, size, a3, __fpt, cmp, d0, \
	, AMISSL_BASE_NAME, int (*__fpt)(), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ERR_load_OBJ_strings() \
	LP0NR(0xcc0, ERR_load_OBJ_strings, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define OBJ_new_nid(num) \
	LP1(0xcc6, int, OBJ_new_nid, int, num, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define OBJ_add_object(obj) \
	LP1(0xccc, int, OBJ_add_object, ASN1_OBJECT *, obj, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define OBJ_create(oid, sn, ln) \
	LP3(0xcd2, int, OBJ_create, char *, oid, a0, char *, sn, a1, char *, ln, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define OBJ_cleanup() \
	LP0NR(0xcd8, OBJ_cleanup, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define OBJ_create_objects(in) \
	LP1(0xcde, int, OBJ_create_objects, BIO *, in, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PEM_get_EVP_CIPHER_INFO(header, cipher) \
	LP2(0xce4, int, PEM_get_EVP_CIPHER_INFO, char *, header, a0, EVP_CIPHER_INFO *, cipher, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PEM_do_header(cipher, data, len, callback, u) \
	LP5(0xcea, int, PEM_do_header, EVP_CIPHER_INFO *, cipher, a0, unsigned char *, data, a1, long *, len, a2, pem_password_cb *, callback, a3, void *, u, d0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PEM_read_bio(bp, name, header, data, len) \
	LP5(0xcf0, int, PEM_read_bio, BIO *, bp, a0, char **, name, a1, char **, header, a2, unsigned char **, data, a3, long *, len, d0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PEM_write_bio(bp, name, hdr, data, len) \
	LP5(0xcf6, int, PEM_write_bio, BIO *, bp, a0, const char *, name, a1, char *, hdr, a2, unsigned char *, data, a3, long, len, d0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PEM_ASN1_read_bio(d2i, name, bp, x, cb, u) \
	LP6FP(0xcfc, char *, PEM_ASN1_read_bio, __fpt, d2i, a0, const char *, name, a1, BIO *, bp, a2, char **, x, a3, pem_password_cb *, cb, d0, void *, u, d1, \
	, AMISSL_BASE_NAME, char * (*__fpt)(), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PEM_ASN1_write_bio(i2d, name, bp, x, enc, kstr, klen, cb, u) \
	LP9FP(0xd02, int, PEM_ASN1_write_bio, __fpt, i2d, a0, const char *, name, a1, BIO *, bp, a2, char *, x, a3, const EVP_CIPHER *, enc, d0, unsigned char *, kstr, d1, int, klen, d2, pem_password_cb *, cb, d3, void *, u, d4, \
	, AMISSL_BASE_NAME, int (*__fpt)(), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PEM_X509_INFO_read_bio(bp, sk, cb, u) \
	LP4(0xd08, *, PEM_X509_INFO_read_bio, BIO *, bp, a0, STACK *, sk, a1, pem_password_cb *, cb, a2, void *, u, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PEM_X509_INFO_write_bio(bp, xi, enc, kstr, klen, cb, u) \
	LP7(0xd0e, int, PEM_X509_INFO_write_bio, BIO *, bp, a0, X509_INFO *, xi, a1, EVP_CIPHER *, enc, a2, unsigned char *, kstr, a3, int, klen, d0, pem_password_cb *, cb, d1, void *, u, d2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PEM_SealInit(ctx, type, md_type, ek, ekl, iv, pubk, npubk) \
	LP8(0xd14, int, PEM_SealInit, PEM_ENCODE_SEAL_CTX *, ctx, a0, EVP_CIPHER *, type, a1, EVP_MD *, md_type, a2, unsigned char **, ek, a3, int *, ekl, d0, unsigned char *, iv, d1, EVP_PKEY **, pubk, d2, int, npubk, d3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PEM_SealUpdate(ctx, out, outl, in, inl) \
	LP5NR(0xd1a, PEM_SealUpdate, PEM_ENCODE_SEAL_CTX *, ctx, a0, unsigned char *, out, a1, int *, outl, a2, unsigned char *, in, a3, int, inl, d0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PEM_SealFinal(ctx, sig, sigl, out, outl, priv) \
	LP6(0xd20, int, PEM_SealFinal, PEM_ENCODE_SEAL_CTX *, ctx, a0, unsigned char *, sig, a1, int *, sigl, a2, unsigned char *, out, a3, int *, outl, d0, EVP_PKEY *, priv, d1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PEM_SignInit(ctx, type) \
	LP2NR(0xd26, PEM_SignInit, EVP_MD_CTX *, ctx, a0, EVP_MD *, type, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PEM_SignUpdate(ctx, d, cnt) \
	LP3NR(0xd2c, PEM_SignUpdate, EVP_MD_CTX *, ctx, a0, unsigned char *, d, a1, unsigned int, cnt, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PEM_SignFinal(ctx, sigret, siglen, pkey) \
	LP4(0xd32, int, PEM_SignFinal, EVP_MD_CTX *, ctx, a0, unsigned char *, sigret, a1, unsigned int *, siglen, a2, EVP_PKEY *, pkey, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ERR_load_PEM_strings() \
	LP0NR(0xd38, ERR_load_PEM_strings, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PEM_proc_type(buf, type) \
	LP2NR(0xd3e, PEM_proc_type, char *, buf, a0, int, type, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PEM_dek_info(buf, type, len, str) \
	LP4NR(0xd44, PEM_dek_info, char *, buf, a0, const char *, type, a1, int, len, a2, char *, str, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PEM_read_bio_X509(bp, x, cb, u) \
	LP4(0xd4a, X509 *, PEM_read_bio_X509, BIO *, bp, a0, X509 **, x, a1, pem_password_cb *, cb, a2, void *, u, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PEM_read_bio_X509_REQ(bp, x, cb, u) \
	LP4(0xd50, X509_REQ *, PEM_read_bio_X509_REQ, BIO *, bp, a0, X509_REQ **, x, a1, pem_password_cb *, cb, a2, void *, u, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PEM_read_bio_X509_CRL(bp, x, cb, u) \
	LP4(0xd56, X509_CRL *, PEM_read_bio_X509_CRL, BIO *, bp, a0, X509_CRL **, x, a1, pem_password_cb *, cb, a2, void *, u, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PEM_read_bio_RSAPrivateKey(bp, x, cb, u) \
	LP4(0xd5c, RSA *, PEM_read_bio_RSAPrivateKey, BIO *, bp, a0, RSA **, x, a1, pem_password_cb *, cb, a2, void *, u, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PEM_read_bio_RSAPublicKey(bp, x, cb, u) \
	LP4(0xd62, RSA *, PEM_read_bio_RSAPublicKey, BIO *, bp, a0, RSA **, x, a1, pem_password_cb *, cb, a2, void *, u, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PEM_read_bio_DSAPrivateKey(bp, x, cb, u) \
	LP4(0xd68, DSA *, PEM_read_bio_DSAPrivateKey, BIO *, bp, a0, DSA **, x, a1, pem_password_cb *, cb, a2, void *, u, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PEM_read_bio_PrivateKey(bp, x, cb, u) \
	LP4(0xd6e, EVP_PKEY *, PEM_read_bio_PrivateKey, BIO *, bp, a0, EVP_PKEY **, x, a1, pem_password_cb *, cb, a2, void *, u, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PEM_read_bio_PKCS7(bp, x, cb, u) \
	LP4(0xd74, PKCS7 *, PEM_read_bio_PKCS7, BIO *, bp, a0, PKCS7 **, x, a1, pem_password_cb *, cb, a2, void *, u, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PEM_read_bio_DHparams(bp, x, cb, u) \
	LP4(0xd7a, DH *, PEM_read_bio_DHparams, BIO *, bp, a0, DH **, x, a1, pem_password_cb *, cb, a2, void *, u, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PEM_read_bio_NETSCAPE_CERT_SEQUENCE(bp, x, cb, u) \
	LP4(0xd80, NETSCAPE_CERT_SEQUENCE *, PEM_read_bio_NETSCAPE_CERT_SEQUENCE, BIO *, bp, a0, NETSCAPE_CERT_SEQUENCE **, x, a1, pem_password_cb *, cb, a2, void *, u, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PEM_read_bio_DSAparams(bp, x, cb, u) \
	LP4(0xd86, DSA *, PEM_read_bio_DSAparams, BIO *, bp, a0, DSA **, x, a1, pem_password_cb *, cb, a2, void *, u, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PEM_write_bio_X509(bp, x) \
	LP2(0xd8c, int, PEM_write_bio_X509, BIO *, bp, a0, X509 *, x, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PEM_write_bio_X509_REQ(bp, x) \
	LP2(0xd92, int, PEM_write_bio_X509_REQ, BIO *, bp, a0, X509_REQ *, x, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PEM_write_bio_X509_CRL(bp, x) \
	LP2(0xd98, int, PEM_write_bio_X509_CRL, BIO *, bp, a0, X509_CRL *, x, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PEM_write_bio_RSAPrivateKey(fp, x, enc, kstr, klen, cb, u) \
	LP7(0xd9e, int, PEM_write_bio_RSAPrivateKey, BIO *, fp, a0, RSA *, x, a1, const EVP_CIPHER *, enc, a2, unsigned char *, kstr, a3, int, klen, d0, pem_password_cb *, cb, d1, void *, u, d2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PEM_write_bio_RSAPublicKey(fp, x) \
	LP2(0xda4, int, PEM_write_bio_RSAPublicKey, BIO *, fp, a0, RSA *, x, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PEM_write_bio_DSAPrivateKey(fp, x, enc, kstr, klen, cb, u) \
	LP7(0xdaa, int, PEM_write_bio_DSAPrivateKey, BIO *, fp, a0, DSA *, x, a1, const EVP_CIPHER *, enc, a2, unsigned char *, kstr, a3, int, klen, d0, pem_password_cb *, cb, d1, void *, u, d2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PEM_write_bio_PrivateKey(fp, x, enc, kstr, klen, cb, u) \
	LP7(0xdb0, int, PEM_write_bio_PrivateKey, BIO *, fp, a0, EVP_PKEY *, x, a1, EVP_CIPHER *, enc, a2, unsigned char *, kstr, a3, int, klen, d0, pem_password_cb *, cb, d1, void *, u, d2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PEM_write_bio_PKCS7(bp, x) \
	LP2(0xdb6, int, PEM_write_bio_PKCS7, BIO *, bp, a0, PKCS7 *, x, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PEM_write_bio_DHparams(bp, x) \
	LP2(0xdbc, int, PEM_write_bio_DHparams, BIO *, bp, a0, DH *, x, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PEM_write_bio_DSAparams(bp, x) \
	LP2(0xdc2, int, PEM_write_bio_DSAparams, BIO *, bp, a0, DSA *, x, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PEM_write_bio_NETSCAPE_CERT_SEQUENCE(bp, x) \
	LP2(0xdc8, int, PEM_write_bio_NETSCAPE_CERT_SEQUENCE, BIO *, bp, a0, NETSCAPE_CERT_SEQUENCE *, x, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS12_pack_safebag(obj, i2d, nid1, nid2) \
	LP4FP(0xdce, PKCS12_SAFEBAG *, PKCS12_pack_safebag, char *, obj, a0, __fpt, i2d, a1, int, nid1, a2, int, nid2, a3, \
	, AMISSL_BASE_NAME, int (*__fpt)(), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS12_MAKE_KEYBAG(p8) \
	LP1(0xdd4, PKCS12_SAFEBAG *, PKCS12_MAKE_KEYBAG, PKCS8_PRIV_KEY_INFO *, p8, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS8_encrypt(pbe_nid, cipher, pass, passlen, salt, saltlen, iter, p8) \
	LP8(0xdda, X509_SIG *, PKCS8_encrypt, int, pbe_nid, a0, const EVP_CIPHER *, cipher, a1, const char *, pass, a2, int, passlen, a3, unsigned char *, salt, d0, int, saltlen, d1, int, iter, d2, PKCS8_PRIV_KEY_INFO *, p8, d3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS12_MAKE_SHKEYBAG(pbe_nid, pass, passlen, salt, saltlen, iter, p8) \
	LP7(0xde0, PKCS12_SAFEBAG *, PKCS12_MAKE_SHKEYBAG, int, pbe_nid, a0, const char *, pass, a1, int, passlen, a2, unsigned char *, salt, a3, int, saltlen, d0, int, iter, d1, PKCS8_PRIV_KEY_INFO *, p8, d2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS12_pack_p7data(sk) \
	LP1(0xde6, PKCS7 *, PKCS12_pack_p7data, STACK *, sk, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS12_pack_p7encdata(pbe_nid, pass, passlen, salt, saltlen, iter, bags) \
	LP7(0xdec, PKCS7 *, PKCS12_pack_p7encdata, int, pbe_nid, a0, const char *, pass, a1, int, passlen, a2, unsigned char *, salt, a3, int, saltlen, d0, int, iter, d1, STACK *, bags, d2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS12_add_localkeyid(bag, name, namelen) \
	LP3(0xdf2, int, PKCS12_add_localkeyid, PKCS12_SAFEBAG *, bag, a0, unsigned char *, name, a1, int, namelen, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS12_add_friendlyname_asc(bag, name, namelen) \
	LP3(0xdf8, int, PKCS12_add_friendlyname_asc, PKCS12_SAFEBAG *, bag, a0, const char *, name, a1, int, namelen, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS12_add_friendlyname_uni(bag, name, namelen) \
	LP3(0xdfe, int, PKCS12_add_friendlyname_uni, PKCS12_SAFEBAG *, bag, a0, const unsigned char *, name, a1, int, namelen, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS8_add_keyusage(p8, usage) \
	LP2(0xe04, int, PKCS8_add_keyusage, PKCS8_PRIV_KEY_INFO *, p8, a0, int, usage, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS12_get_attr_gen(attrs, attr_nid) \
	LP2(0xe0a, ASN1_TYPE *, PKCS12_get_attr_gen, STACK *, attrs, a0, int, attr_nid, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS12_get_friendlyname(bag) \
	LP1(0xe10, char *, PKCS12_get_friendlyname, PKCS12_SAFEBAG *, bag, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS12_pbe_crypt(algor, pass, passlen, in, inlen, data, datalen, en_de) \
	LP8(0xe16, unsigned char *, PKCS12_pbe_crypt, X509_ALGOR *, algor, a0, const char *, pass, a1, int, passlen, a2, unsigned char *, in, a3, int, inlen, d0, unsigned char **, data, d1, int *, datalen, d2, int, en_de, d3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS12_decrypt_d2i(algor, d2i, free_func, pass, passlen, oct, seq) \
	LP7FP(0xe1c, char *, PKCS12_decrypt_d2i, X509_ALGOR *, algor, a0, __fpt, d2i, a1, void *, free_func, a2, const char *, pass, a3, int, passlen, d0, ASN1_STRING *, oct, d1, int, seq, d2, \
	, AMISSL_BASE_NAME, char * (*__fpt)(), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS12_i2d_encrypt(algor, i2d, pass, passlen, obj, seq) \
	LP6FP(0xe22, ASN1_STRING *, PKCS12_i2d_encrypt, X509_ALGOR *, algor, a0, __fpt, i2d, a1, const char *, pass, a2, int, passlen, a3, char *, obj, d0, int, seq, d1, \
	, AMISSL_BASE_NAME, int (*__fpt)(), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS12_init(mode) \
	LP1(0xe28, PKCS12 *, PKCS12_init, int, mode, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS12_key_gen_asc(pass, passlen, salt, saltlen, id, iter, n, out, md_type) \
	LP9(0xe2e, int, PKCS12_key_gen_asc, const char *, pass, a0, int, passlen, a1, unsigned char *, salt, a2, int, saltlen, a3, int, id, d0, int, iter, d1, int, n, d2, unsigned char *, out, d3, const EVP_MD *, md_type, d4, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS12_key_gen_uni(pass, passlen, salt, saltlen, id, iter, n, out, md_type) \
	LP9(0xe34, int, PKCS12_key_gen_uni, unsigned char *, pass, a0, int, passlen, a1, unsigned char *, salt, a2, int, saltlen, a3, int, id, d0, int, iter, d1, int, n, d2, unsigned char *, out, d3, const EVP_MD *, md_type, d4, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS12_gen_mac(p12, pass, passlen, mac, maclen) \
	LP5(0xe3a, int, PKCS12_gen_mac, PKCS12 *, p12, a0, const char *, pass, a1, int, passlen, a2, unsigned char *, mac, a3, unsigned int *, maclen, d0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS12_verify_mac(p12, pass, passlen) \
	LP3(0xe40, int, PKCS12_verify_mac, PKCS12 *, p12, a0, const char *, pass, a1, int, passlen, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS12_set_mac(p12, pass, passlen, salt, saltlen, iter, md_type) \
	LP7(0xe46, int, PKCS12_set_mac, PKCS12 *, p12, a0, const char *, pass, a1, int, passlen, a2, unsigned char *, salt, a3, int, saltlen, d0, int, iter, d1, EVP_MD *, md_type, d2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS12_setup_mac(p12, iter, salt, saltlen, md_type) \
	LP5(0xe4c, int, PKCS12_setup_mac, PKCS12 *, p12, a0, int, iter, a1, unsigned char *, salt, a2, int, saltlen, a3, EVP_MD *, md_type, d0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define asc2uni(asc, uni, unilen) \
	LP3(0xe52, unsigned char *, asc2uni, const char *, asc, a0, unsigned char **, uni, a1, int *, unilen, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define uni2asc(uni, unilen) \
	LP2(0xe58, char *, uni2asc, unsigned char *, uni, a0, int, unilen, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_PKCS12_BAGS(a, pp) \
	LP2(0xe5e, int, i2d_PKCS12_BAGS, PKCS12_BAGS *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS12_BAGS_new() \
	LP0(0xe64, PKCS12_BAGS *, PKCS12_BAGS_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_PKCS12_BAGS(a, pp, length) \
	LP3(0xe6a, PKCS12_BAGS *, d2i_PKCS12_BAGS, PKCS12_BAGS **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS12_BAGS_free(a) \
	LP1NR(0xe70, PKCS12_BAGS_free, PKCS12_BAGS *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_PKCS12(a, pp) \
	LP2(0xe76, int, i2d_PKCS12, PKCS12 *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_PKCS12(a, pp, length) \
	LP3(0xe7c, PKCS12 *, d2i_PKCS12, PKCS12 **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS12_new() \
	LP0(0xe82, PKCS12 *, PKCS12_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS12_free(a) \
	LP1NR(0xe88, PKCS12_free, PKCS12 *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_PKCS12_MAC_DATA(a, pp) \
	LP2(0xe8e, int, i2d_PKCS12_MAC_DATA, PKCS12_MAC_DATA *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define i2d_PKCS12_MAC_DAT(a, tags...) \
	({ULONG _tags[] = {tags}; i2d_PKCS12_MAC_DATA((a), (unsigned char **) _tags);})
#endif

#define PKCS12_MAC_DATA_new() \
	LP0(0xe94, PKCS12_MAC_DATA *, PKCS12_MAC_DATA_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_PKCS12_MAC_DATA(a, pp, length) \
	LP3(0xe9a, PKCS12_MAC_DATA *, d2i_PKCS12_MAC_DATA, PKCS12_MAC_DATA **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define d2i_PKCS12_MAC_DAT(a, pp, tags...) \
	({ULONG _tags[] = {tags}; d2i_PKCS12_MAC_DATA((a), (pp), (long) _tags);})
#endif

#define PKCS12_MAC_DATA_free(a) \
	LP1NR(0xea0, PKCS12_MAC_DATA_free, PKCS12_MAC_DATA *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_PKCS12_SAFEBAG(a, pp) \
	LP2(0xea6, int, i2d_PKCS12_SAFEBAG, PKCS12_SAFEBAG *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS12_SAFEBAG_new() \
	LP0(0xeac, PKCS12_SAFEBAG *, PKCS12_SAFEBAG_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_PKCS12_SAFEBAG(a, pp, length) \
	LP3(0xeb2, PKCS12_SAFEBAG *, d2i_PKCS12_SAFEBAG, PKCS12_SAFEBAG **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS12_SAFEBAG_free(a) \
	LP1NR(0xeb8, PKCS12_SAFEBAG_free, PKCS12_SAFEBAG *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ERR_load_PKCS12_strings() \
	LP0NR(0xebe, ERR_load_PKCS12_strings, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS12_PBE_add() \
	LP0NR(0xec4, PKCS12_PBE_add, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS12_parse(p12, pass, pkey, cert, ca) \
	LP5(0xeca, int, PKCS12_parse, PKCS12 *, p12, a0, const char *, pass, a1, EVP_PKEY **, pkey, a2, X509 **, cert, a3, STACK **, ca, d0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS12_create(pass, name, pkey, cert, ca, nid_key, nid_cert, iter, mac_iter, keytype) \
	LP10(0xed0, PKCS12 *, PKCS12_create, char *, pass, a0, char *, name, a1, EVP_PKEY *, pkey, a2, X509 *, cert, a3, STACK *, ca, d0, int, nid_key, d1, int, nid_cert, d2, int, iter, d3, int, mac_iter, d4, int, keytype, d5, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_PKCS12_bio(bp, p12) \
	LP2(0xed6, int, i2d_PKCS12_bio, BIO *, bp, a0, PKCS12 *, p12, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_PKCS12_bio(bp, p12) \
	LP2(0xedc, PKCS12 *, d2i_PKCS12_bio, BIO *, bp, a0, PKCS12 **, p12, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS7_ISSUER_AND_SERIAL_new() \
	LP0(0xee2, PKCS7_ISSUER_AND_SERIAL *, PKCS7_ISSUER_AND_SERIAL_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS7_ISSUER_AND_SERIAL_free(a) \
	LP1NR(0xee8, PKCS7_ISSUER_AND_SERIAL_free, PKCS7_ISSUER_AND_SERIAL *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_PKCS7_ISSUER_AND_SERIAL(a, pp) \
	LP2(0xeee, int, i2d_PKCS7_ISSUER_AND_SERIAL, PKCS7_ISSUER_AND_SERIAL *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_PKCS7_ISSUER_AND_SERIAL(a, pp, length) \
	LP3(0xef4, PKCS7_ISSUER_AND_SERIAL *, d2i_PKCS7_ISSUER_AND_SERIAL, PKCS7_ISSUER_AND_SERIAL **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS7_ISSUER_AND_SERIAL_digest(data, type, md, len) \
	LP4(0xefa, int, PKCS7_ISSUER_AND_SERIAL_digest, PKCS7_ISSUER_AND_SERIAL *, data, a0, EVP_MD *, type, a1, unsigned char *, md, a2, unsigned int *, len, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS7_dup(p7) \
	LP1(0xf00, PKCS7 *, PKCS7_dup, PKCS7 *, p7, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_PKCS7_bio(bp, p7) \
	LP2(0xf06, PKCS7 *, d2i_PKCS7_bio, BIO *, bp, a0, PKCS7 **, p7, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_PKCS7_bio(bp, p7) \
	LP2(0xf0c, int, i2d_PKCS7_bio, BIO *, bp, a0, PKCS7 *, p7, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS7_SIGNER_INFO_new() \
	LP0(0xf12, PKCS7_SIGNER_INFO *, PKCS7_SIGNER_INFO_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS7_SIGNER_INFO_free(a) \
	LP1NR(0xf18, PKCS7_SIGNER_INFO_free, PKCS7_SIGNER_INFO *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_PKCS7_SIGNER_INFO(a, pp) \
	LP2(0xf1e, int, i2d_PKCS7_SIGNER_INFO, PKCS7_SIGNER_INFO *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_PKCS7_SIGNER_INFO(a, pp, length) \
	LP3(0xf24, PKCS7_SIGNER_INFO *, d2i_PKCS7_SIGNER_INFO, PKCS7_SIGNER_INFO **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS7_RECIP_INFO_new() \
	LP0(0xf2a, PKCS7_RECIP_INFO *, PKCS7_RECIP_INFO_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS7_RECIP_INFO_free(a) \
	LP1NR(0xf30, PKCS7_RECIP_INFO_free, PKCS7_RECIP_INFO *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_PKCS7_RECIP_INFO(a, pp) \
	LP2(0xf36, int, i2d_PKCS7_RECIP_INFO, PKCS7_RECIP_INFO *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_PKCS7_RECIP_INFO(a, pp, length) \
	LP3(0xf3c, PKCS7_RECIP_INFO *, d2i_PKCS7_RECIP_INFO, PKCS7_RECIP_INFO **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS7_SIGNED_new() \
	LP0(0xf42, PKCS7_SIGNED *, PKCS7_SIGNED_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS7_SIGNED_free(a) \
	LP1NR(0xf48, PKCS7_SIGNED_free, PKCS7_SIGNED *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_PKCS7_SIGNED(a, pp) \
	LP2(0xf4e, int, i2d_PKCS7_SIGNED, PKCS7_SIGNED *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_PKCS7_SIGNED(a, pp, length) \
	LP3(0xf54, PKCS7_SIGNED *, d2i_PKCS7_SIGNED, PKCS7_SIGNED **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS7_ENC_CONTENT_new() \
	LP0(0xf5a, PKCS7_ENC_CONTENT *, PKCS7_ENC_CONTENT_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS7_ENC_CONTENT_free(a) \
	LP1NR(0xf60, PKCS7_ENC_CONTENT_free, PKCS7_ENC_CONTENT *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_PKCS7_ENC_CONTENT(a, pp) \
	LP2(0xf66, int, i2d_PKCS7_ENC_CONTENT, PKCS7_ENC_CONTENT *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_PKCS7_ENC_CONTENT(a, pp, length) \
	LP3(0xf6c, PKCS7_ENC_CONTENT *, d2i_PKCS7_ENC_CONTENT, PKCS7_ENC_CONTENT **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS7_ENVELOPE_new() \
	LP0(0xf72, PKCS7_ENVELOPE *, PKCS7_ENVELOPE_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS7_ENVELOPE_free(a) \
	LP1NR(0xf78, PKCS7_ENVELOPE_free, PKCS7_ENVELOPE *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_PKCS7_ENVELOPE(a, pp) \
	LP2(0xf7e, int, i2d_PKCS7_ENVELOPE, PKCS7_ENVELOPE *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_PKCS7_ENVELOPE(a, pp, length) \
	LP3(0xf84, PKCS7_ENVELOPE *, d2i_PKCS7_ENVELOPE, PKCS7_ENVELOPE **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS7_SIGN_ENVELOPE_new() \
	LP0(0xf8a, PKCS7_SIGN_ENVELOPE *, PKCS7_SIGN_ENVELOPE_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS7_SIGN_ENVELOPE_free(a) \
	LP1NR(0xf90, PKCS7_SIGN_ENVELOPE_free, PKCS7_SIGN_ENVELOPE *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_PKCS7_SIGN_ENVELOPE(a, pp) \
	LP2(0xf96, int, i2d_PKCS7_SIGN_ENVELOPE, PKCS7_SIGN_ENVELOPE *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_PKCS7_SIGN_ENVELOPE(a, pp, length) \
	LP3(0xf9c, PKCS7_SIGN_ENVELOPE *, d2i_PKCS7_SIGN_ENVELOPE, PKCS7_SIGN_ENVELOPE **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS7_DIGEST_new() \
	LP0(0xfa2, PKCS7_DIGEST *, PKCS7_DIGEST_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS7_DIGEST_free(a) \
	LP1NR(0xfa8, PKCS7_DIGEST_free, PKCS7_DIGEST *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_PKCS7_DIGEST(a, pp) \
	LP2(0xfae, int, i2d_PKCS7_DIGEST, PKCS7_DIGEST *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_PKCS7_DIGEST(a, pp, length) \
	LP3(0xfb4, PKCS7_DIGEST *, d2i_PKCS7_DIGEST, PKCS7_DIGEST **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS7_ENCRYPT_new() \
	LP0(0xfba, PKCS7_ENCRYPT *, PKCS7_ENCRYPT_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS7_ENCRYPT_free(a) \
	LP1NR(0xfc0, PKCS7_ENCRYPT_free, PKCS7_ENCRYPT *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_PKCS7_ENCRYPT(a, pp) \
	LP2(0xfc6, int, i2d_PKCS7_ENCRYPT, PKCS7_ENCRYPT *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_PKCS7_ENCRYPT(a, pp, length) \
	LP3(0xfcc, PKCS7_ENCRYPT *, d2i_PKCS7_ENCRYPT, PKCS7_ENCRYPT **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS7_new() \
	LP0(0xfd2, PKCS7 *, PKCS7_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS7_free(a) \
	LP1NR(0xfd8, PKCS7_free, PKCS7 *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS7_content_free(a) \
	LP1NR(0xfde, PKCS7_content_free, PKCS7 *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_PKCS7(a, pp) \
	LP2(0xfe4, int, i2d_PKCS7, PKCS7 *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_PKCS7(a, pp, length) \
	LP3(0xfea, PKCS7 *, d2i_PKCS7, PKCS7 **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ERR_load_PKCS7_strings() \
	LP0NR(0xff0, ERR_load_PKCS7_strings, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS7_ctrl(p7, cmd, larg, parg) \
	LP4(0xff6, long, PKCS7_ctrl, PKCS7 *, p7, a0, int, cmd, a1, long, larg, a2, char *, parg, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS7_set_type(p7, type) \
	LP2(0xffc, int, PKCS7_set_type, PKCS7 *, p7, a0, int, type, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS7_set_content(p7, p7_data) \
	LP2(0x1002, int, PKCS7_set_content, PKCS7 *, p7, a0, PKCS7 *, p7_data, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS7_SIGNER_INFO_set(p7i, x509, pkey, dgst) \
	LP4(0x1008, int, PKCS7_SIGNER_INFO_set, PKCS7_SIGNER_INFO *, p7i, a0, X509 *, x509, a1, EVP_PKEY *, pkey, a2, EVP_MD *, dgst, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS7_add_signer(p7, p7i) \
	LP2(0x100e, int, PKCS7_add_signer, PKCS7 *, p7, a0, PKCS7_SIGNER_INFO *, p7i, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS7_add_certificate(p7, x509) \
	LP2(0x1014, int, PKCS7_add_certificate, PKCS7 *, p7, a0, X509 *, x509, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS7_add_crl(p7, x509) \
	LP2(0x101a, int, PKCS7_add_crl, PKCS7 *, p7, a0, X509_CRL *, x509, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS7_content_new(p7, nid) \
	LP2(0x1020, int, PKCS7_content_new, PKCS7 *, p7, a0, int, nid, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS7_dataVerify(cert_store, ctx, bio, p7, si) \
	LP5(0x1026, int, PKCS7_dataVerify, X509_STORE *, cert_store, a0, X509_STORE_CTX *, ctx, a1, BIO *, bio, a2, PKCS7 *, p7, a3, PKCS7_SIGNER_INFO *, si, d0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS7_dataInit(p7, bio) \
	LP2(0x102c, BIO *, PKCS7_dataInit, PKCS7 *, p7, a0, BIO *, bio, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS7_dataFinal(p7, bio) \
	LP2(0x1032, int, PKCS7_dataFinal, PKCS7 *, p7, a0, BIO *, bio, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS7_dataDecode(p7, pkey, in_bio, pcert) \
	LP4(0x1038, BIO *, PKCS7_dataDecode, PKCS7 *, p7, a0, EVP_PKEY *, pkey, a1, BIO *, in_bio, a2, X509 *, pcert, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS7_add_signature(p7, x509, pkey, dgst) \
	LP4(0x103e, PKCS7_SIGNER_INFO *, PKCS7_add_signature, PKCS7 *, p7, a0, X509 *, x509, a1, EVP_PKEY *, pkey, a2, EVP_MD *, dgst, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS7_cert_from_signer_info(p7, si) \
	LP2(0x1044, X509 *, PKCS7_cert_from_signer_info, PKCS7 *, p7, a0, PKCS7_SIGNER_INFO *, si, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS7_get_signer_info(p7) \
	LP1(0x104a, *, PKCS7_get_signer_info, PKCS7 *, p7, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS7_add_recipient(p7, x509) \
	LP2(0x1050, PKCS7_RECIP_INFO *, PKCS7_add_recipient, PKCS7 *, p7, a0, X509 *, x509, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS7_add_recipient_info(p7, ri) \
	LP2(0x1056, int, PKCS7_add_recipient_info, PKCS7 *, p7, a0, PKCS7_RECIP_INFO *, ri, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS7_RECIP_INFO_set(p7i, x509) \
	LP2(0x105c, int, PKCS7_RECIP_INFO_set, PKCS7_RECIP_INFO *, p7i, a0, X509 *, x509, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS7_set_cipher(p7, cipher) \
	LP2(0x1062, int, PKCS7_set_cipher, PKCS7 *, p7, a0, const EVP_CIPHER *, cipher, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS7_get_issuer_and_serial(p7, idx) \
	LP2(0x1068, PKCS7_ISSUER_AND_SERIAL *, PKCS7_get_issuer_and_serial, PKCS7 *, p7, a0, int, idx, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS7_digest_from_attributes(sk) \
	LP1(0x106e, ASN1_OCTET_STRING *, PKCS7_digest_from_attributes, STACK *, sk, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS7_add_signed_attribute(p7si, nid, type, data) \
	LP4(0x1074, int, PKCS7_add_signed_attribute, PKCS7_SIGNER_INFO *, p7si, a0, int, nid, a1, int, type, a2, void *, data, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS7_add_attribute(p7si, nid, atrtype, value) \
	LP4(0x107a, int, PKCS7_add_attribute, PKCS7_SIGNER_INFO *, p7si, a0, int, nid, a1, int, atrtype, a2, void *, value, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS7_get_attribute(si, nid) \
	LP2(0x1080, ASN1_TYPE *, PKCS7_get_attribute, PKCS7_SIGNER_INFO *, si, a0, int, nid, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS7_get_signed_attribute(si, nid) \
	LP2(0x1086, ASN1_TYPE *, PKCS7_get_signed_attribute, PKCS7_SIGNER_INFO *, si, a0, int, nid, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS7_set_signed_attributes(p7si, sk) \
	LP2(0x108c, int, PKCS7_set_signed_attributes, PKCS7_SIGNER_INFO *, p7si, a0, STACK *, sk, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS7_set_attributes(p7si, sk) \
	LP2(0x1092, int, PKCS7_set_attributes, PKCS7_SIGNER_INFO *, p7si, a0, STACK *, sk, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define RAND_set_rand_method(meth) \
	LP1NR(0x1098, RAND_set_rand_method, RAND_METHOD *, meth, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define RAND_get_rand_method() \
	LP0(0x109e, RAND_METHOD *, RAND_get_rand_method, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define RAND_SSLeay() \
	LP0(0x10a4, RAND_METHOD *, RAND_SSLeay, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define RAND_cleanup() \
	LP0NR(0x10aa, RAND_cleanup, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define RAND_bytes(buf, num) \
	LP2NR(0x10b0, RAND_bytes, unsigned char *, buf, a0, int, num, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define RAND_seed(buf, num) \
	LP2NR(0x10b6, RAND_seed, const void *, buf, a0, int, num, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define RAND_load_file(file, max_bytes) \
	LP2(0x10bc, int, RAND_load_file, const char *, file, a0, long, max_bytes, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define RAND_write_file(file) \
	LP1(0x10c2, int, RAND_write_file, const char *, file, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define RAND_file_name(file, num) \
	LP2(0x10c8, char *, RAND_file_name, char *, file, a0, int, num, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ERR_load_RSAREF_strings() \
	LP0NR(0x10ce, ERR_load_RSAREF_strings, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define RSA_new() \
	LP0(0x10d4, RSA *, RSA_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define RSA_new_method(method) \
	LP1(0x10da, RSA *, RSA_new_method, RSA_METHOD *, method, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define RSA_size(a) \
	LP1(0x10e0, int, RSA_size, RSA *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define RSA_generate_key(bits, e, callback, cb_arg) \
	LP4FP(0x10e6, RSA *, RSA_generate_key, int, bits, a0, unsigned long, e, a1, __fpt, callback, a2, void *, cb_arg, a3, \
	, AMISSL_BASE_NAME, void (*__fpt)(int, int, void *), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define RSA_public_encrypt(flen, from, to, rsa, padding) \
	LP5(0x10ec, int, RSA_public_encrypt, int, flen, a0, unsigned char *, from, a1, unsigned char *, to, a2, RSA *, rsa, a3, int, padding, d0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define RSA_private_encrypt(flen, from, to, rsa, padding) \
	LP5(0x10f2, int, RSA_private_encrypt, int, flen, a0, unsigned char *, from, a1, unsigned char *, to, a2, RSA *, rsa, a3, int, padding, d0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define RSA_public_decrypt(flen, from, to, rsa, padding) \
	LP5(0x10f8, int, RSA_public_decrypt, int, flen, a0, unsigned char *, from, a1, unsigned char *, to, a2, RSA *, rsa, a3, int, padding, d0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define RSA_private_decrypt(flen, from, to, rsa, padding) \
	LP5(0x10fe, int, RSA_private_decrypt, int, flen, a0, unsigned char *, from, a1, unsigned char *, to, a2, RSA *, rsa, a3, int, padding, d0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define RSA_free(r) \
	LP1NR(0x1104, RSA_free, RSA *, r, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define RSA_flags(r) \
	LP1(0x110a, int, RSA_flags, RSA *, r, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define RSA_set_default_method(meth) \
	LP1NR(0x1110, RSA_set_default_method, RSA_METHOD *, meth, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define RSA_memory_lock(r) \
	LP1(0x1116, int, RSA_memory_lock, RSA *, r, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define RSA_PKCS1_Default() \
	LP0(0x111c, RSA_METHOD *, RSA_PKCS1_Default, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ERR_load_RSA_strings() \
	LP0NR(0x1122, ERR_load_RSA_strings, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_RSAPublicKey(a, pp, length) \
	LP3(0x1128, RSA *, d2i_RSAPublicKey, RSA **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_RSAPublicKey(a, pp) \
	LP2(0x112e, int, i2d_RSAPublicKey, RSA *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_RSAPrivateKey(a, pp, length) \
	LP3(0x1134, RSA *, d2i_RSAPrivateKey, RSA **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_RSAPrivateKey(a, pp) \
	LP2(0x113a, int, i2d_RSAPrivateKey, RSA *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define RSA_print(bp, r, offset) \
	LP3(0x1140, int, RSA_print, BIO *, bp, a0, RSA *, r, a1, int, offset, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_Netscape_RSA(a, pp, cb) \
	LP3FP(0x1146, int, i2d_Netscape_RSA, RSA *, a, a0, unsigned char **, pp, a1, __fpt, cb, a2, \
	, AMISSL_BASE_NAME, int (*__fpt)(), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define i2d_Netscape_RS(a, pp, tags...) \
	({ULONG _tags[] = {tags}; i2d_Netscape_RSA((a), (pp), (int (*)()) _tags);})
#endif

#define d2i_Netscape_RSA(a, pp, length, cb) \
	LP4FP(0x114c, RSA *, d2i_Netscape_RSA, RSA **, a, a0, unsigned char **, pp, a1, long, length, a2, __fpt, cb, a3, \
	, AMISSL_BASE_NAME, int (*__fpt)(), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define d2i_Netscape_RS(a, pp, length, tags...) \
	({ULONG _tags[] = {tags}; d2i_Netscape_RSA((a), (pp), (length), (int (*)()) _tags);})
#endif

#define d2i_Netscape_RSA_2(a, pp, length, cb) \
	LP4FP(0x1152, RSA *, d2i_Netscape_RSA_2, RSA **, a, a0, unsigned char **, pp, a1, long, length, a2, __fpt, cb, a3, \
	, AMISSL_BASE_NAME, int (*__fpt)(), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define RSA_sign(type, m, m_len, sigret, siglen, rsa) \
	LP6(0x1158, int, RSA_sign, int, type, a0, unsigned char *, m, a1, unsigned int, m_len, a2, unsigned char *, sigret, a3, unsigned int *, siglen, d0, RSA *, rsa, d1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define RSA_verify(type, m, m_len, sigbuf, siglen, rsa) \
	LP6(0x115e, int, RSA_verify, int, type, a0, unsigned char *, m, a1, unsigned int, m_len, a2, unsigned char *, sigbuf, a3, unsigned int, siglen, d0, RSA *, rsa, d1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define RSA_sign_ASN1_OCTET_STRING(type, m, m_len, sigret, siglen, rsa) \
	LP6(0x1164, int, RSA_sign_ASN1_OCTET_STRING, int, type, a0, unsigned char *, m, a1, unsigned int, m_len, a2, unsigned char *, sigret, a3, unsigned int *, siglen, d0, RSA *, rsa, d1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define RSA_verify_ASN1_OCTET_STRING(type, m, m_len, sigbuf, siglen, rsa) \
	LP6(0x116a, int, RSA_verify_ASN1_OCTET_STRING, int, type, a0, unsigned char *, m, a1, unsigned int, m_len, a2, unsigned char *, sigbuf, a3, unsigned int, siglen, d0, RSA *, rsa, d1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define RSA_blinding_on(rsa, ctx) \
	LP2(0x1170, int, RSA_blinding_on, RSA *, rsa, a0, BN_CTX *, ctx, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define RSA_blinding_off(rsa) \
	LP1NR(0x1176, RSA_blinding_off, RSA *, rsa, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define RSA_padding_add_PKCS1_type_1(to, tlen, f, fl) \
	LP4(0x117c, int, RSA_padding_add_PKCS1_type_1, unsigned char *, to, a0, int, tlen, a1, unsigned char *, f, a2, int, fl, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define RSA_padding_check_PKCS1_type_1(to, tlen, f, fl, rsa_len) \
	LP5(0x1182, int, RSA_padding_check_PKCS1_type_1, unsigned char *, to, a0, int, tlen, a1, unsigned char *, f, a2, int, fl, a3, int, rsa_len, d0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define RSA_padding_add_PKCS1_type_2(to, tlen, f, fl) \
	LP4(0x1188, int, RSA_padding_add_PKCS1_type_2, unsigned char *, to, a0, int, tlen, a1, unsigned char *, f, a2, int, fl, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define RSA_padding_check_PKCS1_type_2(to, tlen, f, fl, rsa_len) \
	LP5(0x118e, int, RSA_padding_check_PKCS1_type_2, unsigned char *, to, a0, int, tlen, a1, unsigned char *, f, a2, int, fl, a3, int, rsa_len, d0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define RSA_padding_add_PKCS1_OAEP(to, tlen, f, fl, p, pl) \
	LP6(0x1194, int, RSA_padding_add_PKCS1_OAEP, unsigned char *, to, a0, int, tlen, a1, unsigned char *, f, a2, int, fl, a3, unsigned char *, p, d0, int, pl, d1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define RSA_padding_check_PKCS1_OAEP(to, tlen, f, fl, rsa_len, p, pl) \
	LP7(0x119a, int, RSA_padding_check_PKCS1_OAEP, unsigned char *, to, a0, int, tlen, a1, unsigned char *, f, a2, int, fl, a3, int, rsa_len, d0, unsigned char *, p, d1, int, pl, d2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define RSA_padding_add_SSLv23(to, tlen, f, fl) \
	LP4(0x11a0, int, RSA_padding_add_SSLv23, unsigned char *, to, a0, int, tlen, a1, unsigned char *, f, a2, int, fl, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define RSA_padding_check_SSLv23(to, tlen, f, fl, rsa_len) \
	LP5(0x11a6, int, RSA_padding_check_SSLv23, unsigned char *, to, a0, int, tlen, a1, unsigned char *, f, a2, int, fl, a3, int, rsa_len, d0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define RSA_padding_add_none(to, tlen, f, fl) \
	LP4(0x11ac, int, RSA_padding_add_none, unsigned char *, to, a0, int, tlen, a1, unsigned char *, f, a2, int, fl, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define RSA_padding_check_none(to, tlen, f, fl, rsa_len) \
	LP5(0x11b2, int, RSA_padding_check_none, unsigned char *, to, a0, int, tlen, a1, unsigned char *, f, a2, int, fl, a3, int, rsa_len, d0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define RSA_get_ex_new_index(argl, argp, new_func, dup_func, free_func) \
	LP5FP(0x11b8, int, RSA_get_ex_new_index, long, argl, a0, char *, argp, a1, __fpt, new_func, a2, void *, dup_func, a3, void *, free_func, d0, \
	, AMISSL_BASE_NAME, int (*__fpt)(), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define RSA_set_ex_data(r, idx, arg) \
	LP3(0x11be, int, RSA_set_ex_data, RSA *, r, a0, int, idx, a1, char *, arg, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define RSA_get_ex_data(r, idx) \
	LP2(0x11c4, char *, RSA_get_ex_data, RSA *, r, a0, int, idx, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_f_ssl() \
	LP0(0x11ca, BIO_METHOD *, BIO_f_ssl, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_new_ssl(ctx, client) \
	LP2(0x11d0, BIO *, BIO_new_ssl, SSL_CTX *, ctx, a0, int, client, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_new_ssl_connect(ctx) \
	LP1(0x11d6, BIO *, BIO_new_ssl_connect, SSL_CTX *, ctx, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_new_buffer_ssl_connect(ctx) \
	LP1(0x11dc, BIO *, BIO_new_buffer_ssl_connect, SSL_CTX *, ctx, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_ssl_copy_session_id(to, from) \
	LP2(0x11e2, int, BIO_ssl_copy_session_id, BIO *, to, a0, BIO *, from, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_ssl_shutdown(ssl_bio) \
	LP1NR(0x11e8, BIO_ssl_shutdown, BIO *, ssl_bio, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_CTX_set_cipher_list(a, str) \
	LP2(0x11ee, int, SSL_CTX_set_cipher_list, SSL_CTX *, a, a0, char *, str, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_CTX_new(meth) \
	LP1(0x11f4, SSL_CTX *, SSL_CTX_new, SSL_METHOD *, meth, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_CTX_free(a) \
	LP1NR(0x11fa, SSL_CTX_free, SSL_CTX *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_CTX_set_timeout(ctx, t) \
	LP2(0x1200, long, SSL_CTX_set_timeout, SSL_CTX *, ctx, a0, long, t, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_CTX_get_timeout(ctx) \
	LP1(0x1206, long, SSL_CTX_get_timeout, SSL_CTX *, ctx, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_CTX_get_cert_store(a) \
	LP1(0x120c, X509_STORE *, SSL_CTX_get_cert_store, SSL_CTX *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_CTX_set_cert_store(a, b) \
	LP2NR(0x1212, SSL_CTX_set_cert_store, SSL_CTX *, a, a0, X509_STORE *, b, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_want(s) \
	LP1(0x1218, int, SSL_want, SSL *, s, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_clear(s) \
	LP1(0x121e, int, SSL_clear, SSL *, s, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_CTX_flush_sessions(ctx, tm) \
	LP2NR(0x1224, SSL_CTX_flush_sessions, SSL_CTX *, ctx, a0, long, tm, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_get_current_cipher(s) \
	LP1(0x122a, SSL_CIPHER *, SSL_get_current_cipher, SSL *, s, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_CIPHER_get_bits(c, alg_bits) \
	LP2(0x1230, int, SSL_CIPHER_get_bits, SSL_CIPHER *, c, a0, int *, alg_bits, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_CIPHER_get_version(c) \
	LP1(0x1236, char *, SSL_CIPHER_get_version, SSL_CIPHER *, c, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_CIPHER_get_name(c) \
	LP1(0x123c, const char *, SSL_CIPHER_get_name, SSL_CIPHER *, c, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_get_fd(s) \
	LP1(0x1242, int, SSL_get_fd, SSL *, s, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_get_cipher_list(s, n) \
	LP2(0x1248, const char *, SSL_get_cipher_list, SSL *, s, a0, int, n, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_get_shared_ciphers(s, buf, len) \
	LP3(0x124e, char *, SSL_get_shared_ciphers, SSL *, s, a0, char *, buf, a1, int, len, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_get_read_ahead(s) \
	LP1(0x1254, int, SSL_get_read_ahead, SSL *, s, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_pending(s) \
	LP1(0x125a, int, SSL_pending, SSL *, s, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_set_fd(s, fd) \
	LP2(0x1260, int, SSL_set_fd, SSL *, s, a0, int, fd, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_set_rfd(s, fd) \
	LP2(0x1266, int, SSL_set_rfd, SSL *, s, a0, int, fd, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_set_wfd(s, fd) \
	LP2(0x126c, int, SSL_set_wfd, SSL *, s, a0, int, fd, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_set_bio(s, rbio, wbio) \
	LP3NR(0x1272, SSL_set_bio, SSL *, s, a0, BIO *, rbio, a1, BIO *, wbio, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_get_rbio(s) \
	LP1(0x1278, BIO *, SSL_get_rbio, SSL *, s, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_get_wbio(s) \
	LP1(0x127e, BIO *, SSL_get_wbio, SSL *, s, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_set_cipher_list(s, str) \
	LP2(0x1284, int, SSL_set_cipher_list, SSL *, s, a0, char *, str, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_set_read_ahead(s, yes) \
	LP2NR(0x128a, SSL_set_read_ahead, SSL *, s, a0, int, yes, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_get_verify_mode(s) \
	LP1(0x1290, int, SSL_get_verify_mode, SSL *, s, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_get_verify_depth(s) \
	LP1(0x1296, int, SSL_get_verify_depth, SSL *, s, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_set_verify(s, mode, callback) \
	LP3NRFP(0x12a2, SSL_set_verify, SSL *, s, a0, int, mode, a1, __fpt, callback, a2, \
	, AMISSL_BASE_NAME, int (*__fpt)(int ok, X509_STORE_CTX *ctx), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_set_verify_depth(s, depth) \
	LP2NR(0x12a8, SSL_set_verify_depth, SSL *, s, a0, int, depth, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_use_RSAPrivateKey(ssl, rsa) \
	LP2(0x12ae, int, SSL_use_RSAPrivateKey, SSL *, ssl, a0, RSA *, rsa, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_use_RSAPrivateKey_ASN1(ssl, d, len) \
	LP3(0x12b4, int, SSL_use_RSAPrivateKey_ASN1, SSL *, ssl, a0, unsigned char *, d, a1, long, len, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_use_PrivateKey(ssl, pkey) \
	LP2(0x12ba, int, SSL_use_PrivateKey, SSL *, ssl, a0, EVP_PKEY *, pkey, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_use_PrivateKey_ASN1(pk, ssl, d, len) \
	LP4(0x12c0, int, SSL_use_PrivateKey_ASN1, int, pk, a0, SSL *, ssl, a1, unsigned char *, d, a2, long, len, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_use_certificate(ssl, x) \
	LP2(0x12c6, int, SSL_use_certificate, SSL *, ssl, a0, X509 *, x, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_use_certificate_ASN1(ssl, d, len) \
	LP3(0x12cc, int, SSL_use_certificate_ASN1, SSL *, ssl, a0, unsigned char *, d, a1, int, len, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_use_RSAPrivateKey_file(ssl, file, type) \
	LP3(0x12d2, int, SSL_use_RSAPrivateKey_file, SSL *, ssl, a0, const char *, file, a1, int, type, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_use_PrivateKey_file(ssl, file, type) \
	LP3(0x12d8, int, SSL_use_PrivateKey_file, SSL *, ssl, a0, const char *, file, a1, int, type, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_use_certificate_file(ssl, file, type) \
	LP3(0x12de, int, SSL_use_certificate_file, SSL *, ssl, a0, const char *, file, a1, int, type, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_CTX_use_RSAPrivateKey_file(ctx, file, type) \
	LP3(0x12e4, int, SSL_CTX_use_RSAPrivateKey_file, SSL_CTX *, ctx, a0, const char *, file, a1, int, type, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_CTX_use_PrivateKey_file(ctx, file, type) \
	LP3(0x12ea, int, SSL_CTX_use_PrivateKey_file, SSL_CTX *, ctx, a0, const char *, file, a1, int, type, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_CTX_use_certificate_file(ctx, file, type) \
	LP3(0x12f0, int, SSL_CTX_use_certificate_file, SSL_CTX *, ctx, a0, const char *, file, a1, int, type, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_CTX_use_certificate_chain_file(ctx, file) \
	LP2(0x12f6, int, SSL_CTX_use_certificate_chain_file, SSL_CTX *, ctx, a0, const char *, file, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_load_client_CA_file(file) \
	LP1(0x12fc, *, SSL_load_client_CA_file, const char *, file, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_add_file_cert_subjects_to_stack(stackCAs, file) \
	LP2(0x1302, int, SSL_add_file_cert_subjects_to_stack, STACK *, stackCAs, a0, const char *, file, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_add_dir_cert_subjects_to_stack(stackCAs, dir) \
	LP2(0x1308, int, SSL_add_dir_cert_subjects_to_stack, STACK *, stackCAs, a0, const char *, dir, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ERR_load_SSL_strings() \
	LP0NR(0x130e, ERR_load_SSL_strings, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_load_error_strings() \
	LP0NR(0x1314, SSL_load_error_strings, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_state_string(s) \
	LP1(0x131a, char *, SSL_state_string, SSL *, s, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_rstate_string(s) \
	LP1(0x1320, char *, SSL_rstate_string, SSL *, s, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_state_string_long(s) \
	LP1(0x1326, char *, SSL_state_string_long, SSL *, s, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_rstate_string_long(s) \
	LP1(0x132c, char *, SSL_rstate_string_long, SSL *, s, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_SESSION_get_time(s) \
	LP1(0x1332, long, SSL_SESSION_get_time, SSL_SESSION *, s, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_SESSION_set_time(s, t) \
	LP2(0x1338, long, SSL_SESSION_set_time, SSL_SESSION *, s, a0, long, t, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_SESSION_get_timeout(s) \
	LP1(0x133e, long, SSL_SESSION_get_timeout, SSL_SESSION *, s, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_SESSION_set_timeout(s, t) \
	LP2(0x1344, long, SSL_SESSION_set_timeout, SSL_SESSION *, s, a0, long, t, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_copy_session_id(to, from) \
	LP2NR(0x134a, SSL_copy_session_id, SSL *, to, a0, SSL *, from, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_SESSION_new() \
	LP0(0x1350, SSL_SESSION *, SSL_SESSION_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_SESSION_hash(a) \
	LP1(0x1356, unsigned long, SSL_SESSION_hash, SSL_SESSION *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_SESSION_cmp(a, b) \
	LP2(0x135c, int, SSL_SESSION_cmp, SSL_SESSION *, a, a0, SSL_SESSION *, b, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_SESSION_print(fp, ses) \
	LP2(0x1362, int, SSL_SESSION_print, BIO *, fp, a0, SSL_SESSION *, ses, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_SESSION_free(ses) \
	LP1NR(0x1368, SSL_SESSION_free, SSL_SESSION *, ses, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_SSL_SESSION(in, pp) \
	LP2(0x136e, int, i2d_SSL_SESSION, SSL_SESSION *, in, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_set_session(to, session) \
	LP2(0x1374, int, SSL_set_session, SSL *, to, a0, SSL_SESSION *, session, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_CTX_add_session(s, c) \
	LP2(0x137a, int, SSL_CTX_add_session, SSL_CTX *, s, a0, SSL_SESSION *, c, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_CTX_remove_session(a, c) \
	LP2(0x1380, int, SSL_CTX_remove_session, SSL_CTX *, a, a0, SSL_SESSION *, c, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_SSL_SESSION(a, pp, length) \
	LP3(0x1386, SSL_SESSION *, d2i_SSL_SESSION, SSL_SESSION **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_get_peer_certificate(s) \
	LP1(0x138c, X509 *, SSL_get_peer_certificate, SSL *, s, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_get_peer_cert_chain(s) \
	LP1(0x1392, *, SSL_get_peer_cert_chain, SSL *, s, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_CTX_get_verify_mode(ctx) \
	LP1(0x1398, int, SSL_CTX_get_verify_mode, SSL_CTX *, ctx, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_CTX_get_verify_depth(ctx) \
	LP1(0x139e, int, SSL_CTX_get_verify_depth, SSL_CTX *, ctx, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_CTX_set_verify(ctx, mode, callback) \
	LP3NRFP(0x13aa, SSL_CTX_set_verify, SSL_CTX *, ctx, a0, int, mode, a1, __fpt, callback, a2, \
	, AMISSL_BASE_NAME, int (*__fpt)(int, X509_STORE_CTX *), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_CTX_set_verify_depth(ctx, depth) \
	LP2NR(0x13b0, SSL_CTX_set_verify_depth, SSL_CTX *, ctx, a0, int, depth, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_CTX_set_cert_verify_callback(ctx, cb, arg) \
	LP3NRFP(0x13b6, SSL_CTX_set_cert_verify_callback, SSL_CTX *, ctx, a0, __fpt, cb, a1, char *, arg, a2, \
	, AMISSL_BASE_NAME, int (*__fpt)(), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_CTX_use_RSAPrivateKey(ctx, rsa) \
	LP2(0x13bc, int, SSL_CTX_use_RSAPrivateKey, SSL_CTX *, ctx, a0, RSA *, rsa, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_CTX_use_RSAPrivateKey_ASN1(ctx, d, len) \
	LP3(0x13c2, int, SSL_CTX_use_RSAPrivateKey_ASN1, SSL_CTX *, ctx, a0, unsigned char *, d, a1, long, len, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_CTX_use_PrivateKey(ctx, pkey) \
	LP2(0x13c8, int, SSL_CTX_use_PrivateKey, SSL_CTX *, ctx, a0, EVP_PKEY *, pkey, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_CTX_use_PrivateKey_ASN1(pk, ctx, d, len) \
	LP4(0x13ce, int, SSL_CTX_use_PrivateKey_ASN1, int, pk, a0, SSL_CTX *, ctx, a1, unsigned char *, d, a2, long, len, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_CTX_use_certificate(ctx, x) \
	LP2(0x13d4, int, SSL_CTX_use_certificate, SSL_CTX *, ctx, a0, X509 *, x, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_CTX_use_certificate_ASN1(ctx, len, d) \
	LP3(0x13da, int, SSL_CTX_use_certificate_ASN1, SSL_CTX *, ctx, a0, int, len, a1, unsigned char *, d, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_CTX_set_default_passwd_cb(ctx, cb) \
	LP2NR(0x13e0, SSL_CTX_set_default_passwd_cb, SSL_CTX *, ctx, a0, pem_password_cb *, cb, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_CTX_check_private_key(ctx) \
	LP1(0x13e6, int, SSL_CTX_check_private_key, SSL_CTX *, ctx, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_check_private_key(ctx) \
	LP1(0x13ec, int, SSL_check_private_key, SSL *, ctx, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_CTX_set_session_id_context(ctx, sid_ctx, sid_ctx_len) \
	LP3(0x13f2, int, SSL_CTX_set_session_id_context, SSL_CTX *, ctx, a0, const unsigned char *, sid_ctx, a1, unsigned int, sid_ctx_len, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_new(ctx) \
	LP1(0x13f8, SSL *, SSL_new, SSL_CTX *, ctx, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_set_session_id_context(ssl, sid_ctx, sid_ctx_len) \
	LP3(0x13fe, int, SSL_set_session_id_context, SSL *, ssl, a0, const unsigned char *, sid_ctx, a1, unsigned int, sid_ctx_len, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_free(ssl) \
	LP1NR(0x1404, SSL_free, SSL *, ssl, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_accept(ssl) \
	LP1(0x140a, int, SSL_accept, SSL *, ssl, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_connect(ssl) \
	LP1(0x1410, int, SSL_connect, SSL *, ssl, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_read(ssl, buf, num) \
	LP3(0x1416, int, SSL_read, SSL *, ssl, a0, char *, buf, a1, int, num, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_peek(ssl, buf, num) \
	LP3(0x141c, int, SSL_peek, SSL *, ssl, a0, char *, buf, a1, int, num, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_write(ssl, buf, num) \
	LP3(0x1422, int, SSL_write, SSL *, ssl, a0, const char *, buf, a1, int, num, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_ctrl(ssl, cmd, larg, parg) \
	LP4(0x1428, long, SSL_ctrl, SSL *, ssl, a0, int, cmd, a1, long, larg, a2, char *, parg, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_CTX_ctrl(ctx, cmd, larg, parg) \
	LP4(0x142e, long, SSL_CTX_ctrl, SSL_CTX *, ctx, a0, int, cmd, a1, long, larg, a2, char *, parg, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_get_error(s, ret_code) \
	LP2(0x1434, int, SSL_get_error, SSL *, s, a0, int, ret_code, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_get_version(s) \
	LP1(0x143a, char *, SSL_get_version, SSL *, s, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_CTX_set_ssl_version(ctx, meth) \
	LP2(0x1440, int, SSL_CTX_set_ssl_version, SSL_CTX *, ctx, a0, SSL_METHOD *, meth, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSLv2_method() \
	LP0(0x1446, SSL_METHOD *, SSLv2_method, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSLv2_server_method() \
	LP0(0x144c, SSL_METHOD *, SSLv2_server_method, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSLv2_client_method() \
	LP0(0x1452, SSL_METHOD *, SSLv2_client_method, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSLv3_method() \
	LP0(0x1458, SSL_METHOD *, SSLv3_method, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSLv3_server_method() \
	LP0(0x145e, SSL_METHOD *, SSLv3_server_method, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSLv3_client_method() \
	LP0(0x1464, SSL_METHOD *, SSLv3_client_method, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSLv23_method() \
	LP0(0x146a, SSL_METHOD *, SSLv23_method, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSLv23_server_method() \
	LP0(0x1470, SSL_METHOD *, SSLv23_server_method, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSLv23_client_method() \
	LP0(0x1476, SSL_METHOD *, SSLv23_client_method, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define TLSv1_method() \
	LP0(0x147c, SSL_METHOD *, TLSv1_method, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define TLSv1_server_method() \
	LP0(0x1482, SSL_METHOD *, TLSv1_server_method, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define TLSv1_client_method() \
	LP0(0x1488, SSL_METHOD *, TLSv1_client_method, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_get_ciphers(s) \
	LP1(0x148e, *, SSL_get_ciphers, SSL *, s, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_do_handshake(s) \
	LP1(0x1494, int, SSL_do_handshake, SSL *, s, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_renegotiate(s) \
	LP1(0x149a, int, SSL_renegotiate, SSL *, s, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_shutdown(s) \
	LP1(0x14a0, int, SSL_shutdown, SSL *, s, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_get_ssl_method(s) \
	LP1(0x14a6, SSL_METHOD *, SSL_get_ssl_method, SSL *, s, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_set_ssl_method(s, method) \
	LP2(0x14ac, int, SSL_set_ssl_method, SSL *, s, a0, SSL_METHOD *, method, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_alert_type_string_long(value) \
	LP1(0x14b2, char *, SSL_alert_type_string_long, int, value, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_alert_type_string(value) \
	LP1(0x14b8, char *, SSL_alert_type_string, int, value, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_alert_desc_string_long(value) \
	LP1(0x14be, char *, SSL_alert_desc_string_long, int, value, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_alert_desc_string(value) \
	LP1(0x14c4, char *, SSL_alert_desc_string, int, value, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_set_client_CA_list(s, list) \
	LP2NR(0x14ca, SSL_set_client_CA_list, SSL *, s, a0, STACK *, list, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_CTX_set_client_CA_list(ctx, list) \
	LP2NR(0x14d0, SSL_CTX_set_client_CA_list, SSL_CTX *, ctx, a0, STACK *, list, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_get_client_CA_list(s) \
	LP1(0x14d6, *, SSL_get_client_CA_list, SSL *, s, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_CTX_get_client_CA_list(s) \
	LP1(0x14dc, *, SSL_CTX_get_client_CA_list, SSL_CTX *, s, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_add_client_CA(ssl, x) \
	LP2(0x14e2, int, SSL_add_client_CA, SSL *, ssl, a0, X509 *, x, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define SSL_add_client_C(ssl, tags...) \
	({ULONG _tags[] = {tags}; SSL_add_client_CA((ssl), (X509 *) _tags);})
#endif

#define SSL_CTX_add_client_CA(ctx, x) \
	LP2(0x14e8, int, SSL_CTX_add_client_CA, SSL_CTX *, ctx, a0, X509 *, x, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define SSL_CTX_add_client_C(ctx, tags...) \
	({ULONG _tags[] = {tags}; SSL_CTX_add_client_CA((ctx), (X509 *) _tags);})
#endif

#define SSL_set_connect_state(s) \
	LP1NR(0x14ee, SSL_set_connect_state, SSL *, s, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_set_accept_state(s) \
	LP1NR(0x14f4, SSL_set_accept_state, SSL *, s, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_get_default_timeout(s) \
	LP1(0x14fa, long, SSL_get_default_timeout, SSL *, s, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_library_init() \
	LP0(0x1500, int, SSL_library_init, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_CIPHER_description(a, buf, size) \
	LP3(0x1506, char *, SSL_CIPHER_description, SSL_CIPHER *, a, a0, char *, buf, a1, int, size, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_dup_CA_list(sk) \
	LP1(0x150c, *, SSL_dup_CA_list, STACK *, sk, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_dup(ssl) \
	LP1(0x1512, SSL *, SSL_dup, SSL *, ssl, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_get_certificate(ssl) \
	LP1(0x1518, X509 *, SSL_get_certificate, SSL *, ssl, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_get_privatekey(ssl) \
	LP1(0x151e, struct evp_pkey_st *, SSL_get_privatekey, SSL *, ssl, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_CTX_set_quiet_shutdown(ctx, mode) \
	LP2NR(0x1524, SSL_CTX_set_quiet_shutdown, SSL_CTX *, ctx, a0, int, mode, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_CTX_get_quiet_shutdown(ctx) \
	LP1(0x152a, int, SSL_CTX_get_quiet_shutdown, SSL_CTX *, ctx, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_set_quiet_shutdown(ssl, mode) \
	LP2NR(0x1530, SSL_set_quiet_shutdown, SSL *, ssl, a0, int, mode, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_get_quiet_shutdown(ssl) \
	LP1(0x1536, int, SSL_get_quiet_shutdown, SSL *, ssl, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_set_shutdown(ssl, mode) \
	LP2NR(0x153c, SSL_set_shutdown, SSL *, ssl, a0, int, mode, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_get_shutdown(ssl) \
	LP1(0x1542, int, SSL_get_shutdown, SSL *, ssl, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_version(ssl) \
	LP1(0x1548, int, SSL_version, SSL *, ssl, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_CTX_set_default_verify_paths(ctx) \
	LP1(0x154e, int, SSL_CTX_set_default_verify_paths, SSL_CTX *, ctx, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_CTX_load_verify_locations(ctx, CAfile, CApath) \
	LP3(0x1554, int, SSL_CTX_load_verify_locations, SSL_CTX *, ctx, a0, const char *, CAfile, a1, const char *, CApath, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_get_session(ssl) \
	LP1(0x155a, SSL_SESSION *, SSL_get_session, SSL *, ssl, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_get_SSL_CTX(ssl) \
	LP1(0x1560, SSL_CTX *, SSL_get_SSL_CTX, SSL *, ssl, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_set_info_callback(ssl, cb) \
	LP2NRFP(0x1566, SSL_set_info_callback, SSL *, ssl, a0, __fpt, cb, a1, \
	, AMISSL_BASE_NAME, void (*__fpt)(), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_state(ssl) \
	LP1(0x1572, int, SSL_state, SSL *, ssl, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_set_verify_result(ssl, v) \
	LP2NR(0x1578, SSL_set_verify_result, SSL *, ssl, a0, long, v, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_get_verify_result(ssl) \
	LP1(0x157e, long, SSL_get_verify_result, SSL *, ssl, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_set_ex_data(ssl, idx, data) \
	LP3(0x1584, int, SSL_set_ex_data, SSL *, ssl, a0, int, idx, a1, void *, data, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_get_ex_data(ssl, idx) \
	LP2(0x158a, void *, SSL_get_ex_data, SSL *, ssl, a0, int, idx, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_get_ex_new_index(argl, argp, new_func, dup_func, free_func) \
	LP5FP(0x1590, int, SSL_get_ex_new_index, long, argl, a0, char *, argp, a1, __fpt, new_func, a2, void *, dup_func, a3, void *, free_func, d0, \
	, AMISSL_BASE_NAME, int (*__fpt)(), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_SESSION_set_ex_data(ss, idx, data) \
	LP3(0x1596, int, SSL_SESSION_set_ex_data, SSL_SESSION *, ss, a0, int, idx, a1, void *, data, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_SESSION_get_ex_data(ss, idx) \
	LP2(0x159c, void *, SSL_SESSION_get_ex_data, SSL_SESSION *, ss, a0, int, idx, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_SESSION_get_ex_new_index(argl, argp, new_func, dup_func, free_func) \
	LP5FP(0x15a2, int, SSL_SESSION_get_ex_new_index, long, argl, a0, char *, argp, a1, __fpt, new_func, a2, void *, dup_func, a3, void *, free_func, d0, \
	, AMISSL_BASE_NAME, int (*__fpt)(), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_CTX_set_ex_data(ssl, idx, data) \
	LP3(0x15a8, int, SSL_CTX_set_ex_data, SSL_CTX *, ssl, a0, int, idx, a1, void *, data, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_CTX_get_ex_data(ssl, idx) \
	LP2(0x15ae, void *, SSL_CTX_get_ex_data, SSL_CTX *, ssl, a0, int, idx, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_CTX_get_ex_new_index(argl, argp, new_func, dup_func, free_func) \
	LP5FP(0x15b4, int, SSL_CTX_get_ex_new_index, long, argl, a0, char *, argp, a1, __fpt, new_func, a2, void *, dup_func, a3, void *, free_func, d0, \
	, AMISSL_BASE_NAME, int (*__fpt)(), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_get_ex_data_X509_STORE_CTX_idx() \
	LP0(0x15ba, int, SSL_get_ex_data_X509_STORE_CTX_idx, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_CTX_set_tmp_rsa_callback(ctx, cb) \
	LP2NRFP(0x15c0, SSL_CTX_set_tmp_rsa_callback, SSL_CTX *, ctx, a0, __fpt, cb, a1, \
	, AMISSL_BASE_NAME, RSA * (*__fpt)(SSL *ssl, int is_export, int keylength), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_set_tmp_rsa_callback(ssl, cb) \
	LP2NRFP(0x15c6, SSL_set_tmp_rsa_callback, SSL *, ssl, a0, __fpt, cb, a1, \
	, AMISSL_BASE_NAME, RSA * (*__fpt)(SSL *ssl, int is_export, int keylength), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_CTX_set_tmp_dh_callback(ctx, dh) \
	LP2NRFP(0x15cc, SSL_CTX_set_tmp_dh_callback, SSL_CTX *, ctx, a0, __fpt, dh, a1, \
	, AMISSL_BASE_NAME, DH * (*__fpt)(SSL *ssl, int is_export, int keylength), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_set_tmp_dh_callback(ssl, dh) \
	LP2NRFP(0x15d2, SSL_set_tmp_dh_callback, SSL *, ssl, a0, __fpt, dh, a1, \
	, AMISSL_BASE_NAME, DH * (*__fpt)(SSL *ssl, int is_export, int keylength), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_COMP_add_compression_method(id, cm) \
	LP2(0x15d8, int, SSL_COMP_add_compression_method, int, id, a0, COMP_METHOD *, cm, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define sk_num(a) \
	LP1(0x15de, int, sk_num, STACK *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define sk_value(a, b) \
	LP2(0x15e4, char *, sk_value, STACK *, a, a0, int, b, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define sk_set(a, b, c) \
	LP3(0x15ea, char *, sk_set, STACK *, a, a0, int, b, a1, char *, c, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define sk_new(cmp) \
	LP1FP(0x15f0, *, sk_new, __fpt, cmp, a0, \
	, AMISSL_BASE_NAME, int (*__fpt)(), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define sk_free(a) \
	LP1NR(0x15f6, sk_free, STACK *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define sk_pop_free(st, func) \
	LP2NRFP(0x15fc, sk_pop_free, STACK *, st, a0, __fpt, func, a1, \
	, AMISSL_BASE_NAME, void (*__fpt)(), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define sk_insert(sk, data, where) \
	LP3(0x1602, int, sk_insert, STACK *, sk, a0, char *, data, a1, int, where, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define sk_delete(st, loc) \
	LP2(0x1608, char *, sk_delete, STACK *, st, a0, int, loc, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define sk_delete_ptr(st, p) \
	LP2(0x160e, char *, sk_delete_ptr, STACK *, st, a0, char *, p, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define sk_find(st, data) \
	LP2(0x1614, int, sk_find, STACK *, st, a0, char *, data, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define sk_push(st, data) \
	LP2(0x161a, int, sk_push, STACK *, st, a0, char *, data, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define sk_unshift(st, data) \
	LP2(0x1620, int, sk_unshift, STACK *, st, a0, char *, data, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define sk_shift(st) \
	LP1(0x1626, char *, sk_shift, STACK *, st, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define sk_pop(st) \
	LP1(0x162c, char *, sk_pop, STACK *, st, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define sk_zero(st) \
	LP1NR(0x1632, sk_zero, STACK *, st, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define sk_dup(st) \
	LP1(0x163e, *, sk_dup, STACK *, st, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ms_time_new() \
	LP0(0x1644, char *, ms_time_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ms_time_free(a) \
	LP1NR(0x164a, ms_time_free, char *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ms_time_get(a) \
	LP1NR(0x1650, ms_time_get, char *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ms_time_diff(start, end) \
	LP2(0x1656, double, ms_time_diff, char *, start, a0, char *, end, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ms_time_cmp(ap, bp) \
	LP2(0x165c, int, ms_time_cmp, char *, ap, a0, char *, bp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define TXT_DB_read(in, num) \
	LP2(0x1662, TXT_DB *, TXT_DB_read, BIO *, in, a0, int, num, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define TXT_DB_write(out, db) \
	LP2(0x1668, long, TXT_DB_write, BIO *, out, a0, TXT_DB *, db, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define TXT_DB_create_index(db, field, qual, hash, cmp) \
	LP5FP(0x166e, int, TXT_DB_create_index, TXT_DB *, db, a0, int, field, a1, __fpt, qual, a2, void *, hash, a3, void *, cmp, d0, \
	, AMISSL_BASE_NAME, int (*__fpt)(), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define TXT_DB_free(db) \
	LP1NR(0x1674, TXT_DB_free, TXT_DB *, db, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define TXT_DB_get_by_index(db, idx, value) \
	LP3(0x167a, char **, TXT_DB_get_by_index, TXT_DB *, db, a0, int, idx, a1, char **, value, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define TXT_DB_insert(db, value) \
	LP2(0x1680, int, TXT_DB_insert, TXT_DB *, db, a0, char **, value, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ERR_load_X509V3_strings() \
	LP0NR(0x1686, ERR_load_X509V3_strings, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_BASIC_CONSTRAINTS(a, pp) \
	LP2(0x168c, int, i2d_BASIC_CONSTRAINTS, BASIC_CONSTRAINTS *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_BASIC_CONSTRAINTS(a, pp, length) \
	LP3(0x1692, BASIC_CONSTRAINTS *, d2i_BASIC_CONSTRAINTS, BASIC_CONSTRAINTS **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BASIC_CONSTRAINTS_new() \
	LP0(0x1698, BASIC_CONSTRAINTS *, BASIC_CONSTRAINTS_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BASIC_CONSTRAINTS_free(a) \
	LP1NR(0x169e, BASIC_CONSTRAINTS_free, BASIC_CONSTRAINTS *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_GENERAL_NAME(a, pp) \
	LP2(0x16a4, int, i2d_GENERAL_NAME, GENERAL_NAME *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_GENERAL_NAME(a, pp, length) \
	LP3(0x16aa, GENERAL_NAME *, d2i_GENERAL_NAME, GENERAL_NAME **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define GENERAL_NAME_new() \
	LP0(0x16b0, GENERAL_NAME *, GENERAL_NAME_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define GENERAL_NAME_free(a) \
	LP1NR(0x16b6, GENERAL_NAME_free, GENERAL_NAME *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2v_GENERAL_NAME(method, gen, ret) \
	LP3(0x16bc, *, i2v_GENERAL_NAME, X509V3_EXT_METHOD *, method, a0, GENERAL_NAME *, gen, a1, STACK *, ret, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_SXNET(a, pp) \
	LP2(0x16c2, int, i2d_SXNET, SXNET *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_SXNET(a, pp, length) \
	LP3(0x16c8, SXNET *, d2i_SXNET, SXNET **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SXNET_new() \
	LP0(0x16ce, SXNET *, SXNET_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SXNET_free(a) \
	LP1NR(0x16d4, SXNET_free, SXNET *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_SXNETID(a, pp) \
	LP2(0x16da, int, i2d_SXNETID, SXNETID *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_SXNETID(a, pp, length) \
	LP3(0x16e0, SXNETID *, d2i_SXNETID, SXNETID **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SXNETID_new() \
	LP0(0x16e6, SXNETID *, SXNETID_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SXNETID_free(a) \
	LP1NR(0x16ec, SXNETID_free, SXNETID *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SXNET_add_id_asc(psx, zone, user, userlen) \
	LP4(0x16f2, int, SXNET_add_id_asc, SXNET **, psx, a0, char *, zone, a1, char *, user, a2, int, userlen, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SXNET_add_id_ulong(psx, lzone, user, userlen) \
	LP4(0x16f8, int, SXNET_add_id_ulong, SXNET **, psx, a0, unsigned long, lzone, a1, char *, user, a2, int, userlen, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SXNET_add_id_INTEGER(psx, izone, user, userlen) \
	LP4(0x16fe, int, SXNET_add_id_INTEGER, SXNET **, psx, a0, ASN1_INTEGER *, izone, a1, char *, user, a2, int, userlen, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SXNET_get_id_asc(sx, zone) \
	LP2(0x1704, ASN1_OCTET_STRING *, SXNET_get_id_asc, SXNET *, sx, a0, char *, zone, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SXNET_get_id_ulong(sx, lzone) \
	LP2(0x170a, ASN1_OCTET_STRING *, SXNET_get_id_ulong, SXNET *, sx, a0, unsigned long, lzone, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SXNET_get_id_INTEGER(sx, zone) \
	LP2(0x1710, ASN1_OCTET_STRING *, SXNET_get_id_INTEGER, SXNET *, sx, a0, ASN1_INTEGER *, zone, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_AUTHORITY_KEYID(a, pp) \
	LP2(0x1716, int, i2d_AUTHORITY_KEYID, AUTHORITY_KEYID *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_AUTHORITY_KEYID(a, pp, length) \
	LP3(0x171c, AUTHORITY_KEYID *, d2i_AUTHORITY_KEYID, AUTHORITY_KEYID **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define AUTHORITY_KEYID_new() \
	LP0(0x1722, AUTHORITY_KEYID *, AUTHORITY_KEYID_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define AUTHORITY_KEYID_free(a) \
	LP1NR(0x1728, AUTHORITY_KEYID_free, AUTHORITY_KEYID *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_PKEY_USAGE_PERIOD(a, pp) \
	LP2(0x172e, int, i2d_PKEY_USAGE_PERIOD, PKEY_USAGE_PERIOD *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_PKEY_USAGE_PERIOD(a, pp, length) \
	LP3(0x1734, PKEY_USAGE_PERIOD *, d2i_PKEY_USAGE_PERIOD, PKEY_USAGE_PERIOD **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKEY_USAGE_PERIOD_new() \
	LP0(0x173a, PKEY_USAGE_PERIOD *, PKEY_USAGE_PERIOD_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKEY_USAGE_PERIOD_free(a) \
	LP1NR(0x1740, PKEY_USAGE_PERIOD_free, PKEY_USAGE_PERIOD *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define GENERAL_NAMES_new() \
	LP0(0x1746, *, GENERAL_NAMES_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define GENERAL_NAMES_free(a) \
	LP1NR(0x174c, GENERAL_NAMES_free, STACK *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_GENERAL_NAMES(a, pp, length) \
	LP3(0x1752, *, d2i_GENERAL_NAMES, STACK **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_GENERAL_NAMES(a, pp) \
	LP2(0x1758, int, i2d_GENERAL_NAMES, STACK *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2v_GENERAL_NAMES(method, gen, extlist) \
	LP3(0x175e, *, i2v_GENERAL_NAMES, X509V3_EXT_METHOD *, method, a0, STACK *, gen, a1, STACK *, extlist, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define v2i_GENERAL_NAMES(method, ctx, nval) \
	LP3(0x1764, *, v2i_GENERAL_NAMES, X509V3_EXT_METHOD *, method, a0, X509V3_CTX *, ctx, a1, STACK *, nval, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2s_ASN1_OCTET_STRING(method, ia5) \
	LP2(0x176a, char *, i2s_ASN1_OCTET_STRING, X509V3_EXT_METHOD *, method, a0, ASN1_OCTET_STRING *, ia5, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define s2i_ASN1_OCTET_STRING(method, ctx, str) \
	LP3(0x1770, ASN1_OCTET_STRING *, s2i_ASN1_OCTET_STRING, X509V3_EXT_METHOD *, method, a0, X509V3_CTX *, ctx, a1, char *, str, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_ext_ku(a, pp) \
	LP2(0x1776, int, i2d_ext_ku, STACK *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_ext_ku(a, pp, length) \
	LP3(0x177c, *, d2i_ext_ku, STACK **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ext_ku_free(a) \
	LP1NR(0x1782, ext_ku_free, STACK *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ext_ku_new() \
	LP0(0x1788, *, ext_ku_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_CERTIFICATEPOLICIES(a, pp) \
	LP2(0x178e, int, i2d_CERTIFICATEPOLICIES, STACK *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CERTIFICATEPOLICIES_new() \
	LP0(0x1794, *, CERTIFICATEPOLICIES_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CERTIFICATEPOLICIES_free(a) \
	LP1NR(0x179a, CERTIFICATEPOLICIES_free, STACK *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_CERTIFICATEPOLICIES(a, pp, length) \
	LP3(0x17a0, *, d2i_CERTIFICATEPOLICIES, STACK **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_POLICYINFO(a, pp) \
	LP2(0x17a6, int, i2d_POLICYINFO, POLICYINFO *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define POLICYINFO_new() \
	LP0(0x17ac, POLICYINFO *, POLICYINFO_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_POLICYINFO(a, pp, length) \
	LP3(0x17b2, POLICYINFO *, d2i_POLICYINFO, POLICYINFO **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define POLICYINFO_free(a) \
	LP1NR(0x17b8, POLICYINFO_free, POLICYINFO *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_POLICYQUALINFO(a, pp) \
	LP2(0x17be, int, i2d_POLICYQUALINFO, POLICYQUALINFO *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define POLICYQUALINFO_new() \
	LP0(0x17c4, POLICYQUALINFO *, POLICYQUALINFO_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_POLICYQUALINFO(a, pp, length) \
	LP3(0x17ca, POLICYQUALINFO *, d2i_POLICYQUALINFO, POLICYQUALINFO **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define POLICYQUALINFO_free(a) \
	LP1NR(0x17d0, POLICYQUALINFO_free, POLICYQUALINFO *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_USERNOTICE(a, pp) \
	LP2(0x17d6, int, i2d_USERNOTICE, USERNOTICE *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define USERNOTICE_new() \
	LP0(0x17dc, USERNOTICE *, USERNOTICE_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_USERNOTICE(a, pp, length) \
	LP3(0x17e2, USERNOTICE *, d2i_USERNOTICE, USERNOTICE **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define USERNOTICE_free(a) \
	LP1NR(0x17e8, USERNOTICE_free, USERNOTICE *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_NOTICEREF(a, pp) \
	LP2(0x17ee, int, i2d_NOTICEREF, NOTICEREF *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define NOTICEREF_new() \
	LP0(0x17f4, NOTICEREF *, NOTICEREF_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_NOTICEREF(a, pp, length) \
	LP3(0x17fa, NOTICEREF *, d2i_NOTICEREF, NOTICEREF **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define NOTICEREF_free(a) \
	LP1NR(0x1800, NOTICEREF_free, NOTICEREF *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_CRL_DIST_POINTS(a, pp) \
	LP2(0x1806, int, i2d_CRL_DIST_POINTS, STACK *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CRL_DIST_POINTS_new() \
	LP0(0x180c, *, CRL_DIST_POINTS_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CRL_DIST_POINTS_free(a) \
	LP1NR(0x1812, CRL_DIST_POINTS_free, STACK *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_CRL_DIST_POINTS(a, pp, length) \
	LP3(0x1818, *, d2i_CRL_DIST_POINTS, STACK **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_DIST_POINT(a, pp) \
	LP2(0x181e, int, i2d_DIST_POINT, DIST_POINT *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define DIST_POINT_new() \
	LP0(0x1824, DIST_POINT *, DIST_POINT_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_DIST_POINT(a, pp, length) \
	LP3(0x182a, DIST_POINT *, d2i_DIST_POINT, DIST_POINT **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define DIST_POINT_free(a) \
	LP1NR(0x1830, DIST_POINT_free, DIST_POINT *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_DIST_POINT_NAME(a, pp) \
	LP2(0x1836, int, i2d_DIST_POINT_NAME, DIST_POINT_NAME *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define DIST_POINT_NAME_new() \
	LP0(0x183c, DIST_POINT_NAME *, DIST_POINT_NAME_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define DIST_POINT_NAME_free(a) \
	LP1NR(0x1842, DIST_POINT_NAME_free, DIST_POINT_NAME *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_DIST_POINT_NAME(a, pp, length) \
	LP3(0x1848, DIST_POINT_NAME *, d2i_DIST_POINT_NAME, DIST_POINT_NAME **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define v2i_GENERAL_NAME(method, ctx, cnf) \
	LP3(0x184e, GENERAL_NAME *, v2i_GENERAL_NAME, X509V3_EXT_METHOD *, method, a0, X509V3_CTX *, ctx, a1, CONF_VALUE *, cnf, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509V3_conf_free(val) \
	LP1NR(0x1854, X509V3_conf_free, CONF_VALUE *, val, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509V3_EXT_conf_nid(conf, ctx, ext_nid, value) \
	LP4(0x185a, X509_EXTENSION *, X509V3_EXT_conf_nid, LHASH *, conf, a0, X509V3_CTX *, ctx, a1, int, ext_nid, a2, char *, value, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509V3_EXT_conf(conf, ctx, name, value) \
	LP4(0x1860, X509_EXTENSION *, X509V3_EXT_conf, LHASH *, conf, a0, X509V3_CTX *, ctx, a1, char *, name, a2, char *, value, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509V3_EXT_add_conf(conf, ctx, section, cert) \
	LP4(0x1866, int, X509V3_EXT_add_conf, LHASH *, conf, a0, X509V3_CTX *, ctx, a1, char *, section, a2, X509 *, cert, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509V3_EXT_CRL_add_conf(conf, ctx, section, crl) \
	LP4(0x186c, int, X509V3_EXT_CRL_add_conf, LHASH *, conf, a0, X509V3_CTX *, ctx, a1, char *, section, a2, X509_CRL *, crl, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509V3_add_value_bool_nf(name, asn1_bool, extlist) \
	LP3(0x1872, int, X509V3_add_value_bool_nf, char *, name, a0, int, asn1_bool, a1, STACK **, extlist, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509V3_get_value_bool(value, asn1_bool) \
	LP2(0x1878, int, X509V3_get_value_bool, CONF_VALUE *, value, a0, int *, asn1_bool, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509V3_get_value_int(value, aint) \
	LP2(0x187e, int, X509V3_get_value_int, CONF_VALUE *, value, a0, ASN1_INTEGER **, aint, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509V3_set_conf_lhash(ctx, lhash) \
	LP2NR(0x1884, X509V3_set_conf_lhash, X509V3_CTX *, ctx, a0, LHASH *, lhash, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509V3_get_string(ctx, name, section) \
	LP3(0x188a, char *, X509V3_get_string, X509V3_CTX *, ctx, a0, char *, name, a1, char *, section, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509V3_get_section(ctx, section) \
	LP2(0x1890, *, X509V3_get_section, X509V3_CTX *, ctx, a0, char *, section, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509V3_string_free(ctx, str) \
	LP2NR(0x1896, X509V3_string_free, X509V3_CTX *, ctx, a0, char *, str, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509V3_section_free(ctx, section) \
	LP2NR(0x189c, X509V3_section_free, X509V3_CTX *, ctx, a0, STACK *, section, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509V3_set_ctx(ctx, issuer, subject, req, crl, flags) \
	LP6NR(0x18a2, X509V3_set_ctx, X509V3_CTX *, ctx, a0, X509 *, issuer, a1, X509 *, subject, a2, X509_REQ *, req, a3, X509_CRL *, crl, d0, int, flags, d1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509V3_add_value(name, value, extlist) \
	LP3(0x18a8, int, X509V3_add_value, const char *, name, a0, const char *, value, a1, STACK **, extlist, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509V3_add_value_uchar(name, value, extlist) \
	LP3(0x18ae, int, X509V3_add_value_uchar, const char *, name, a0, const unsigned char *, value, a1, STACK **, extlist, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509V3_add_value_bool(name, asn1_bool, extlist) \
	LP3(0x18b4, int, X509V3_add_value_bool, const char *, name, a0, int, asn1_bool, a1, STACK **, extlist, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509V3_add_value_int(name, aint, extlist) \
	LP3(0x18ba, int, X509V3_add_value_int, const char *, name, a0, ASN1_INTEGER *, aint, a1, STACK **, extlist, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2s_ASN1_INTEGER(meth, aint) \
	LP2(0x18c0, char *, i2s_ASN1_INTEGER, X509V3_EXT_METHOD *, meth, a0, ASN1_INTEGER *, aint, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define s2i_ASN1_INTEGER(meth, value) \
	LP2(0x18c6, ASN1_INTEGER *, s2i_ASN1_INTEGER, X509V3_EXT_METHOD *, meth, a0, char *, value, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2s_ASN1_ENUMERATED(meth, aint) \
	LP2(0x18cc, char *, i2s_ASN1_ENUMERATED, X509V3_EXT_METHOD *, meth, a0, ASN1_ENUMERATED *, aint, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2s_ASN1_ENUMERATED_TABLE(meth, aint) \
	LP2(0x18d2, char *, i2s_ASN1_ENUMERATED_TABLE, X509V3_EXT_METHOD *, meth, a0, ASN1_ENUMERATED *, aint, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509V3_EXT_add(ext) \
	LP1(0x18d8, int, X509V3_EXT_add, X509V3_EXT_METHOD *, ext, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509V3_EXT_add_list(extlist) \
	LP1(0x18de, int, X509V3_EXT_add_list, X509V3_EXT_METHOD *, extlist, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509V3_EXT_add_alias(nid_to, nid_from) \
	LP2(0x18e4, int, X509V3_EXT_add_alias, int, nid_to, a0, int, nid_from, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509V3_EXT_cleanup() \
	LP0NR(0x18ea, X509V3_EXT_cleanup, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509V3_EXT_get(ext) \
	LP1(0x18f0, X509V3_EXT_METHOD *, X509V3_EXT_get, X509_EXTENSION *, ext, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509V3_EXT_get_nid(nid) \
	LP1(0x18f6, X509V3_EXT_METHOD *, X509V3_EXT_get_nid, int, nid, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509V3_add_standard_extensions() \
	LP0(0x18fc, int, X509V3_add_standard_extensions, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509V3_parse_list(line) \
	LP1(0x1902, *, X509V3_parse_list, char *, line, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509V3_EXT_d2i(ext) \
	LP1(0x1908, void *, X509V3_EXT_d2i, X509_EXTENSION *, ext, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509V3_EXT_i2d(ext_nid, crit, ext_struc) \
	LP3(0x190e, X509_EXTENSION *, X509V3_EXT_i2d, int, ext_nid, a0, int, crit, a1, void *, ext_struc, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define hex_to_string(buffer, len) \
	LP2(0x1914, char *, hex_to_string, unsigned char *, buffer, a0, long, len, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define string_to_hex(str, len) \
	LP2(0x191a, unsigned char *, string_to_hex, char *, str, a0, long *, len, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define name_cmp(name, cmp) \
	LP2(0x1920, int, name_cmp, const char *, name, a0, const char *, cmp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509V3_EXT_val_prn(out, val, indent, ml) \
	LP4NR(0x1926, X509V3_EXT_val_prn, BIO *, out, a0, STACK *, val, a1, int, indent, a2, int, ml, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509V3_EXT_print(out, ext, flag, indent) \
	LP4(0x192c, int, X509V3_EXT_print, BIO *, out, a0, X509_EXTENSION *, ext, a1, int, flag, a2, int, indent, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_verify(a, r) \
	LP2(0x1932, int, X509_verify, X509 *, a, a0, EVP_PKEY *, r, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_verify_cert_error_string(n) \
	LP1(0x1938, const char *, X509_verify_cert_error_string, long, n, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_REQ_verify(a, r) \
	LP2(0x193e, int, X509_REQ_verify, X509_REQ *, a, a0, EVP_PKEY *, r, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_CRL_verify(a, r) \
	LP2(0x1944, int, X509_CRL_verify, X509_CRL *, a, a0, EVP_PKEY *, r, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define NETSCAPE_SPKI_verify(a, r) \
	LP2(0x194a, int, NETSCAPE_SPKI_verify, NETSCAPE_SPKI *, a, a0, EVP_PKEY *, r, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_sign(x, pkey, md) \
	LP3(0x1950, int, X509_sign, X509 *, x, a0, EVP_PKEY *, pkey, a1, const EVP_MD *, md, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_REQ_sign(x, pkey, md) \
	LP3(0x1956, int, X509_REQ_sign, X509_REQ *, x, a0, EVP_PKEY *, pkey, a1, const EVP_MD *, md, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_CRL_sign(x, pkey, md) \
	LP3(0x195c, int, X509_CRL_sign, X509_CRL *, x, a0, EVP_PKEY *, pkey, a1, const EVP_MD *, md, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define NETSCAPE_SPKI_sign(x, pkey, md) \
	LP3(0x1962, int, NETSCAPE_SPKI_sign, NETSCAPE_SPKI *, x, a0, EVP_PKEY *, pkey, a1, const EVP_MD *, md, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_digest(data, type, md, len) \
	LP4(0x1968, int, X509_digest, X509 *, data, a0, EVP_MD *, type, a1, unsigned char *, md, a2, unsigned int *, len, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_NAME_digest(data, type, md, len) \
	LP4(0x196e, int, X509_NAME_digest, X509_NAME *, data, a0, EVP_MD *, type, a1, unsigned char *, md, a2, unsigned int *, len, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_X509_bio(bp, x509) \
	LP2(0x1974, X509 *, d2i_X509_bio, BIO *, bp, a0, X509 **, x509, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_X509_bio(bp, x509) \
	LP2(0x197a, int, i2d_X509_bio, BIO *, bp, a0, X509 *, x509, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_X509_CRL_bio(bp, crl) \
	LP2(0x1980, X509_CRL *, d2i_X509_CRL_bio, BIO *, bp, a0, X509_CRL **, crl, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_X509_CRL_bio(bp, crl) \
	LP2(0x1986, int, i2d_X509_CRL_bio, BIO *, bp, a0, X509_CRL *, crl, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_X509_REQ_bio(bp, req) \
	LP2(0x198c, X509_REQ *, d2i_X509_REQ_bio, BIO *, bp, a0, X509_REQ **, req, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_X509_REQ_bio(bp, req) \
	LP2(0x1992, int, i2d_X509_REQ_bio, BIO *, bp, a0, X509_REQ *, req, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_RSAPrivateKey_bio(bp, rsa) \
	LP2(0x1998, RSA *, d2i_RSAPrivateKey_bio, BIO *, bp, a0, RSA **, rsa, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_RSAPrivateKey_bio(bp, rsa) \
	LP2(0x199e, int, i2d_RSAPrivateKey_bio, BIO *, bp, a0, RSA *, rsa, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_RSAPublicKey_bio(bp, rsa) \
	LP2(0x19a4, RSA *, d2i_RSAPublicKey_bio, BIO *, bp, a0, RSA **, rsa, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_RSAPublicKey_bio(bp, rsa) \
	LP2(0x19aa, int, i2d_RSAPublicKey_bio, BIO *, bp, a0, RSA *, rsa, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_DSAPrivateKey_bio(bp, dsa) \
	LP2(0x19b0, DSA *, d2i_DSAPrivateKey_bio, BIO *, bp, a0, DSA **, dsa, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_DSAPrivateKey_bio(bp, dsa) \
	LP2(0x19b6, int, i2d_DSAPrivateKey_bio, BIO *, bp, a0, DSA *, dsa, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_dup(x509) \
	LP1(0x19bc, X509 *, X509_dup, X509 *, x509, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_ATTRIBUTE_dup(xa) \
	LP1(0x19c2, X509_ATTRIBUTE *, X509_ATTRIBUTE_dup, X509_ATTRIBUTE *, xa, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_EXTENSION_dup(ex) \
	LP1(0x19c8, X509_EXTENSION *, X509_EXTENSION_dup, X509_EXTENSION *, ex, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_CRL_dup(crl) \
	LP1(0x19ce, X509_CRL *, X509_CRL_dup, X509_CRL *, crl, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_REQ_dup(req) \
	LP1(0x19d4, X509_REQ *, X509_REQ_dup, X509_REQ *, req, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_ALGOR_dup(xn) \
	LP1(0x19da, X509_ALGOR *, X509_ALGOR_dup, X509_ALGOR *, xn, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_NAME_dup(xn) \
	LP1(0x19e0, X509_NAME *, X509_NAME_dup, X509_NAME *, xn, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_NAME_ENTRY_dup(ne) \
	LP1(0x19e6, X509_NAME_ENTRY *, X509_NAME_ENTRY_dup, X509_NAME_ENTRY *, ne, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define RSAPublicKey_dup(rsa) \
	LP1(0x19ec, RSA *, RSAPublicKey_dup, RSA *, rsa, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define RSAPrivateKey_dup(rsa) \
	LP1(0x19f2, RSA *, RSAPrivateKey_dup, RSA *, rsa, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_cmp_current_time(s) \
	LP1(0x19f8, int, X509_cmp_current_time, ASN1_UTCTIME *, s, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_gmtime_adj(s, adj) \
	LP2(0x19fe, ASN1_UTCTIME *, X509_gmtime_adj, ASN1_UTCTIME *, s, a0, long, adj, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_get_default_cert_area() \
	LP0(0x1a04, const char *, X509_get_default_cert_area, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_get_default_cert_dir() \
	LP0(0x1a0a, const char *, X509_get_default_cert_dir, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_get_default_cert_file() \
	LP0(0x1a10, const char *, X509_get_default_cert_file, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_get_default_cert_dir_env() \
	LP0(0x1a16, const char *, X509_get_default_cert_dir_env, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_get_default_cert_file_env() \
	LP0(0x1a1c, const char *, X509_get_default_cert_file_env, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_get_default_private_dir() \
	LP0(0x1a22, const char *, X509_get_default_private_dir, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_to_X509_REQ(x, pkey, md) \
	LP3(0x1a28, X509_REQ *, X509_to_X509_REQ, X509 *, x, a0, EVP_PKEY *, pkey, a1, EVP_MD *, md, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_REQ_to_X509(r, days, pkey) \
	LP3(0x1a2e, X509 *, X509_REQ_to_X509, X509_REQ *, r, a0, int, days, a1, EVP_PKEY *, pkey, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ERR_load_X509_strings() \
	LP0NR(0x1a34, ERR_load_X509_strings, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_ALGOR_new() \
	LP0(0x1a3a, X509_ALGOR *, X509_ALGOR_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_ALGOR_free(a) \
	LP1NR(0x1a40, X509_ALGOR_free, X509_ALGOR *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_X509_ALGOR(a, pp) \
	LP2(0x1a46, int, i2d_X509_ALGOR, X509_ALGOR *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_X509_ALGOR(a, pp, length) \
	LP3(0x1a4c, X509_ALGOR *, d2i_X509_ALGOR, X509_ALGOR **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_VAL_new() \
	LP0(0x1a52, X509_VAL *, X509_VAL_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_VAL_free(a) \
	LP1NR(0x1a58, X509_VAL_free, X509_VAL *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_X509_VAL(a, pp) \
	LP2(0x1a5e, int, i2d_X509_VAL, X509_VAL *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_X509_VAL(a, pp, length) \
	LP3(0x1a64, X509_VAL *, d2i_X509_VAL, X509_VAL **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_PUBKEY_new() \
	LP0(0x1a6a, X509_PUBKEY *, X509_PUBKEY_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_PUBKEY_free(a) \
	LP1NR(0x1a70, X509_PUBKEY_free, X509_PUBKEY *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_X509_PUBKEY(a, pp) \
	LP2(0x1a76, int, i2d_X509_PUBKEY, X509_PUBKEY *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_X509_PUBKEY(a, pp, length) \
	LP3(0x1a7c, X509_PUBKEY *, d2i_X509_PUBKEY, X509_PUBKEY **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_PUBKEY_set(x, pkey) \
	LP2(0x1a82, int, X509_PUBKEY_set, X509_PUBKEY **, x, a0, EVP_PKEY *, pkey, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_PUBKEY_get(key) \
	LP1(0x1a88, EVP_PKEY *, X509_PUBKEY_get, X509_PUBKEY *, key, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_get_pubkey_parameters(pkey, chain) \
	LP2(0x1a8e, int, X509_get_pubkey_parameters, EVP_PKEY *, pkey, a0, STACK *, chain, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_SIG_new() \
	LP0(0x1a94, X509_SIG *, X509_SIG_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_SIG_free(a) \
	LP1NR(0x1a9a, X509_SIG_free, X509_SIG *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_X509_SIG(a, pp) \
	LP2(0x1aa0, int, i2d_X509_SIG, X509_SIG *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_X509_SIG(a, pp, length) \
	LP3(0x1aa6, X509_SIG *, d2i_X509_SIG, X509_SIG **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_REQ_INFO_new() \
	LP0(0x1aac, X509_REQ_INFO *, X509_REQ_INFO_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_REQ_INFO_free(a) \
	LP1NR(0x1ab2, X509_REQ_INFO_free, X509_REQ_INFO *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_X509_REQ_INFO(a, pp) \
	LP2(0x1ab8, int, i2d_X509_REQ_INFO, X509_REQ_INFO *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_X509_REQ_INFO(a, pp, length) \
	LP3(0x1abe, X509_REQ_INFO *, d2i_X509_REQ_INFO, X509_REQ_INFO **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_REQ_new() \
	LP0(0x1ac4, X509_REQ *, X509_REQ_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_REQ_free(a) \
	LP1NR(0x1aca, X509_REQ_free, X509_REQ *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_X509_REQ(a, pp) \
	LP2(0x1ad0, int, i2d_X509_REQ, X509_REQ *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_X509_REQ(a, pp, length) \
	LP3(0x1ad6, X509_REQ *, d2i_X509_REQ, X509_REQ **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_ATTRIBUTE_new() \
	LP0(0x1adc, X509_ATTRIBUTE *, X509_ATTRIBUTE_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_ATTRIBUTE_free(a) \
	LP1NR(0x1ae2, X509_ATTRIBUTE_free, X509_ATTRIBUTE *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_X509_ATTRIBUTE(a, pp) \
	LP2(0x1ae8, int, i2d_X509_ATTRIBUTE, X509_ATTRIBUTE *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_X509_ATTRIBUTE(a, pp, length) \
	LP3(0x1aee, X509_ATTRIBUTE *, d2i_X509_ATTRIBUTE, X509_ATTRIBUTE **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_ATTRIBUTE_create(nid, atrtype, value) \
	LP3(0x1af4, X509_ATTRIBUTE *, X509_ATTRIBUTE_create, int, nid, a0, int, atrtype, a1, void *, value, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_EXTENSION_new() \
	LP0(0x1afa, X509_EXTENSION *, X509_EXTENSION_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_EXTENSION_free(a) \
	LP1NR(0x1b00, X509_EXTENSION_free, X509_EXTENSION *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_X509_EXTENSION(a, pp) \
	LP2(0x1b06, int, i2d_X509_EXTENSION, X509_EXTENSION *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_X509_EXTENSION(a, pp, length) \
	LP3(0x1b0c, X509_EXTENSION *, d2i_X509_EXTENSION, X509_EXTENSION **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_NAME_ENTRY_new() \
	LP0(0x1b12, X509_NAME_ENTRY *, X509_NAME_ENTRY_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_NAME_ENTRY_free(a) \
	LP1NR(0x1b18, X509_NAME_ENTRY_free, X509_NAME_ENTRY *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_X509_NAME_ENTRY(a, pp) \
	LP2(0x1b1e, int, i2d_X509_NAME_ENTRY, X509_NAME_ENTRY *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_X509_NAME_ENTRY(a, pp, length) \
	LP3(0x1b24, X509_NAME_ENTRY *, d2i_X509_NAME_ENTRY, X509_NAME_ENTRY **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_NAME_new() \
	LP0(0x1b2a, X509_NAME *, X509_NAME_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_NAME_free(a) \
	LP1NR(0x1b30, X509_NAME_free, X509_NAME *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_X509_NAME(a, pp) \
	LP2(0x1b36, int, i2d_X509_NAME, X509_NAME *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_X509_NAME(a, pp, length) \
	LP3(0x1b3c, X509_NAME *, d2i_X509_NAME, X509_NAME **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_NAME_set(xn, name) \
	LP2(0x1b42, int, X509_NAME_set, X509_NAME **, xn, a0, X509_NAME *, name, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_CINF_new() \
	LP0(0x1b48, X509_CINF *, X509_CINF_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_CINF_free(a) \
	LP1NR(0x1b4e, X509_CINF_free, X509_CINF *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_X509_CINF(a, pp) \
	LP2(0x1b54, int, i2d_X509_CINF, X509_CINF *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_X509_CINF(a, pp, length) \
	LP3(0x1b5a, X509_CINF *, d2i_X509_CINF, X509_CINF **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_new() \
	LP0(0x1b60, X509 *, X509_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_free(a) \
	LP1NR(0x1b66, X509_free, X509 *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_X509(a, pp) \
	LP2(0x1b6c, int, i2d_X509, X509 *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_X509(a, pp, length) \
	LP3(0x1b72, X509 *, d2i_X509, X509 **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_REVOKED_new() \
	LP0(0x1b78, X509_REVOKED *, X509_REVOKED_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_REVOKED_free(a) \
	LP1NR(0x1b7e, X509_REVOKED_free, X509_REVOKED *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_X509_REVOKED(a, pp) \
	LP2(0x1b84, int, i2d_X509_REVOKED, X509_REVOKED *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_X509_REVOKED(a, pp, length) \
	LP3(0x1b8a, X509_REVOKED *, d2i_X509_REVOKED, X509_REVOKED **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_CRL_INFO_new() \
	LP0(0x1b90, X509_CRL_INFO *, X509_CRL_INFO_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_CRL_INFO_free(a) \
	LP1NR(0x1b96, X509_CRL_INFO_free, X509_CRL_INFO *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_X509_CRL_INFO(a, pp) \
	LP2(0x1b9c, int, i2d_X509_CRL_INFO, X509_CRL_INFO *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_X509_CRL_INFO(a, pp, length) \
	LP3(0x1ba2, X509_CRL_INFO *, d2i_X509_CRL_INFO, X509_CRL_INFO **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_CRL_new() \
	LP0(0x1ba8, X509_CRL *, X509_CRL_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_CRL_free(a) \
	LP1NR(0x1bae, X509_CRL_free, X509_CRL *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_X509_CRL(a, pp) \
	LP2(0x1bb4, int, i2d_X509_CRL, X509_CRL *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_X509_CRL(a, pp, length) \
	LP3(0x1bba, X509_CRL *, d2i_X509_CRL, X509_CRL **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_PKEY_new() \
	LP0(0x1bc0, X509_PKEY *, X509_PKEY_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_PKEY_free(a) \
	LP1NR(0x1bc6, X509_PKEY_free, X509_PKEY *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_X509_PKEY(a, pp) \
	LP2(0x1bcc, int, i2d_X509_PKEY, X509_PKEY *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_X509_PKEY(a, pp, length) \
	LP3(0x1bd2, X509_PKEY *, d2i_X509_PKEY, X509_PKEY **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define NETSCAPE_SPKI_new() \
	LP0(0x1bd8, NETSCAPE_SPKI *, NETSCAPE_SPKI_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define NETSCAPE_SPKI_free(a) \
	LP1NR(0x1bde, NETSCAPE_SPKI_free, NETSCAPE_SPKI *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_NETSCAPE_SPKI(a, pp) \
	LP2(0x1be4, int, i2d_NETSCAPE_SPKI, NETSCAPE_SPKI *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_NETSCAPE_SPKI(a, pp, length) \
	LP3(0x1bea, NETSCAPE_SPKI *, d2i_NETSCAPE_SPKI, NETSCAPE_SPKI **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define NETSCAPE_SPKAC_new() \
	LP0(0x1bf0, NETSCAPE_SPKAC *, NETSCAPE_SPKAC_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define NETSCAPE_SPKAC_free(a) \
	LP1NR(0x1bf6, NETSCAPE_SPKAC_free, NETSCAPE_SPKAC *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_NETSCAPE_SPKAC(a, pp) \
	LP2(0x1bfc, int, i2d_NETSCAPE_SPKAC, NETSCAPE_SPKAC *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_NETSCAPE_SPKAC(a, pp, length) \
	LP3(0x1c02, NETSCAPE_SPKAC *, d2i_NETSCAPE_SPKAC, NETSCAPE_SPKAC **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_NETSCAPE_CERT_SEQUENCE(a, pp) \
	LP2(0x1c08, int, i2d_NETSCAPE_CERT_SEQUENCE, NETSCAPE_CERT_SEQUENCE *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define NETSCAPE_CERT_SEQUENCE_new() \
	LP0(0x1c0e, NETSCAPE_CERT_SEQUENCE *, NETSCAPE_CERT_SEQUENCE_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_NETSCAPE_CERT_SEQUENCE(a, pp, length) \
	LP3(0x1c14, NETSCAPE_CERT_SEQUENCE *, d2i_NETSCAPE_CERT_SEQUENCE, NETSCAPE_CERT_SEQUENCE **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define NETSCAPE_CERT_SEQUENCE_free(a) \
	LP1NR(0x1c1a, NETSCAPE_CERT_SEQUENCE_free, NETSCAPE_CERT_SEQUENCE *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_INFO_new() \
	LP0(0x1c20, X509_INFO *, X509_INFO_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_INFO_free(a) \
	LP1NR(0x1c26, X509_INFO_free, X509_INFO *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_NAME_oneline(a, buf, size) \
	LP3(0x1c2c, char *, X509_NAME_oneline, X509_NAME *, a, a0, char *, buf, a1, int, size, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ASN1_verify(i2d, algor1, signature, data, pkey) \
	LP5FP(0x1c32, int, ASN1_verify, __fpt, i2d, a0, X509_ALGOR *, algor1, a1, ASN1_BIT_STRING *, signature, a2, char *, data, a3, EVP_PKEY *, pkey, d0, \
	, AMISSL_BASE_NAME, int (*__fpt)(), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ASN1_digest(i2d, type, data, md, len) \
	LP5FP(0x1c38, int, ASN1_digest, __fpt, i2d, a0, EVP_MD *, type, a1, char *, data, a2, unsigned char *, md, a3, unsigned int *, len, d0, \
	, AMISSL_BASE_NAME, int (*__fpt)(), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ASN1_sign(i2d, algor1, algor2, signature, data, pkey, type) \
	LP7FP(0x1c3e, int, ASN1_sign, __fpt, i2d, a0, X509_ALGOR *, algor1, a1, X509_ALGOR *, algor2, a2, ASN1_BIT_STRING *, signature, a3, char *, data, d0, EVP_PKEY *, pkey, d1, const EVP_MD *, type, d2, \
	, AMISSL_BASE_NAME, int (*__fpt)(), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_set_version(x, version) \
	LP2(0x1c44, int, X509_set_version, X509 *, x, a0, long, version, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_set_serialNumber(x, serial) \
	LP2(0x1c4a, int, X509_set_serialNumber, X509 *, x, a0, ASN1_INTEGER *, serial, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_get_serialNumber(x) \
	LP1(0x1c50, ASN1_INTEGER *, X509_get_serialNumber, X509 *, x, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_set_issuer_name(x, name) \
	LP2(0x1c56, int, X509_set_issuer_name, X509 *, x, a0, X509_NAME *, name, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_get_issuer_name(a) \
	LP1(0x1c5c, X509_NAME *, X509_get_issuer_name, X509 *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_set_subject_name(x, name) \
	LP2(0x1c62, int, X509_set_subject_name, X509 *, x, a0, X509_NAME *, name, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_get_subject_name(a) \
	LP1(0x1c68, X509_NAME *, X509_get_subject_name, X509 *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_set_notBefore(x, tm) \
	LP2(0x1c6e, int, X509_set_notBefore, X509 *, x, a0, ASN1_UTCTIME *, tm, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_set_notAfter(x, tm) \
	LP2(0x1c74, int, X509_set_notAfter, X509 *, x, a0, ASN1_UTCTIME *, tm, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_set_pubkey(x, pkey) \
	LP2(0x1c7a, int, X509_set_pubkey, X509 *, x, a0, EVP_PKEY *, pkey, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_get_pubkey(x) \
	LP1(0x1c80, EVP_PKEY *, X509_get_pubkey, X509 *, x, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_certificate_type(x, pubkey) \
	LP2(0x1c86, int, X509_certificate_type, X509 *, x, a0, EVP_PKEY *, pubkey, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_REQ_set_version(x, version) \
	LP2(0x1c8c, int, X509_REQ_set_version, X509_REQ *, x, a0, long, version, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_REQ_set_subject_name(req, name) \
	LP2(0x1c92, int, X509_REQ_set_subject_name, X509_REQ *, req, a0, X509_NAME *, name, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_REQ_set_pubkey(x, pkey) \
	LP2(0x1c98, int, X509_REQ_set_pubkey, X509_REQ *, x, a0, EVP_PKEY *, pkey, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_REQ_get_pubkey(req) \
	LP1(0x1c9e, EVP_PKEY *, X509_REQ_get_pubkey, X509_REQ *, req, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_check_private_key(x509, pkey) \
	LP2(0x1ca4, int, X509_check_private_key, X509 *, x509, a0, EVP_PKEY *, pkey, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_issuer_and_serial_cmp(a, b) \
	LP2(0x1caa, int, X509_issuer_and_serial_cmp, X509 *, a, a0, X509 *, b, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_issuer_and_serial_hash(a) \
	LP1(0x1cb0, unsigned long, X509_issuer_and_serial_hash, X509 *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_issuer_name_cmp(a, b) \
	LP2(0x1cb6, int, X509_issuer_name_cmp, X509 *, a, a0, X509 *, b, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_issuer_name_hash(a) \
	LP1(0x1cbc, unsigned long, X509_issuer_name_hash, X509 *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_subject_name_cmp(a, b) \
	LP2(0x1cc2, int, X509_subject_name_cmp, X509 *, a, a0, X509 *, b, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_subject_name_hash(x) \
	LP1(0x1cc8, unsigned long, X509_subject_name_hash, X509 *, x, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_NAME_cmp(a, b) \
	LP2(0x1cce, int, X509_NAME_cmp, X509_NAME *, a, a0, X509_NAME *, b, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_NAME_hash(x) \
	LP1(0x1cd4, unsigned long, X509_NAME_hash, X509_NAME *, x, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_CRL_cmp(a, b) \
	LP2(0x1cda, int, X509_CRL_cmp, X509_CRL *, a, a0, X509_CRL *, b, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_NAME_print(bp, name, obase) \
	LP3(0x1ce0, int, X509_NAME_print, BIO *, bp, a0, X509_NAME *, name, a1, int, obase, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_print(bp, x) \
	LP2(0x1ce6, int, X509_print, BIO *, bp, a0, X509 *, x, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_CRL_print(bp, x) \
	LP2(0x1cec, int, X509_CRL_print, BIO *, bp, a0, X509_CRL *, x, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_REQ_print(bp, req) \
	LP2(0x1cf2, int, X509_REQ_print, BIO *, bp, a0, X509_REQ *, req, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_NAME_entry_count(name) \
	LP1(0x1cf8, int, X509_NAME_entry_count, X509_NAME *, name, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_NAME_get_text_by_NID(name, nid, buf, len) \
	LP4(0x1cfe, int, X509_NAME_get_text_by_NID, X509_NAME *, name, a0, int, nid, a1, char *, buf, a2, int, len, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_NAME_get_text_by_OBJ(name, obj, buf, len) \
	LP4(0x1d04, int, X509_NAME_get_text_by_OBJ, X509_NAME *, name, a0, ASN1_OBJECT *, obj, a1, char *, buf, a2, int, len, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_NAME_get_index_by_NID(name, nid, lastpos) \
	LP3(0x1d0a, int, X509_NAME_get_index_by_NID, X509_NAME *, name, a0, int, nid, a1, int, lastpos, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_NAME_get_index_by_OBJ(name, obj, lastpos) \
	LP3(0x1d10, int, X509_NAME_get_index_by_OBJ, X509_NAME *, name, a0, ASN1_OBJECT *, obj, a1, int, lastpos, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_NAME_get_entry(name, loc) \
	LP2(0x1d16, X509_NAME_ENTRY *, X509_NAME_get_entry, X509_NAME *, name, a0, int, loc, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_NAME_delete_entry(name, loc) \
	LP2(0x1d1c, X509_NAME_ENTRY *, X509_NAME_delete_entry, X509_NAME *, name, a0, int, loc, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_NAME_add_entry(name, ne, loc, set) \
	LP4(0x1d22, int, X509_NAME_add_entry, X509_NAME *, name, a0, X509_NAME_ENTRY *, ne, a1, int, loc, a2, int, set, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_NAME_ENTRY_create_by_NID(ne, nid, type, bytes, len) \
	LP5(0x1d28, X509_NAME_ENTRY *, X509_NAME_ENTRY_create_by_NID, X509_NAME_ENTRY **, ne, a0, int, nid, a1, int, type, a2, unsigned char *, bytes, a3, int, len, d0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_NAME_ENTRY_create_by_OBJ(ne, obj, type, bytes, len) \
	LP5(0x1d2e, X509_NAME_ENTRY *, X509_NAME_ENTRY_create_by_OBJ, X509_NAME_ENTRY **, ne, a0, ASN1_OBJECT *, obj, a1, int, type, a2, unsigned char *, bytes, a3, int, len, d0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_NAME_ENTRY_set_object(ne, obj) \
	LP2(0x1d34, int, X509_NAME_ENTRY_set_object, X509_NAME_ENTRY *, ne, a0, ASN1_OBJECT *, obj, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_NAME_ENTRY_set_data(ne, type, bytes, len) \
	LP4(0x1d3a, int, X509_NAME_ENTRY_set_data, X509_NAME_ENTRY *, ne, a0, int, type, a1, unsigned char *, bytes, a2, int, len, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_NAME_ENTRY_get_object(ne) \
	LP1(0x1d40, ASN1_OBJECT *, X509_NAME_ENTRY_get_object, X509_NAME_ENTRY *, ne, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_NAME_ENTRY_get_data(ne) \
	LP1(0x1d46, ASN1_STRING *, X509_NAME_ENTRY_get_data, X509_NAME_ENTRY *, ne, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509v3_get_ext_count(x) \
	LP1(0x1d4c, int, X509v3_get_ext_count, const STACK *, x, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509v3_get_ext_by_NID(x, nid, lastpos) \
	LP3(0x1d52, int, X509v3_get_ext_by_NID, const STACK *, x, a0, int, nid, a1, int, lastpos, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509v3_get_ext_by_OBJ(x, obj, lastpos) \
	LP3(0x1d58, int, X509v3_get_ext_by_OBJ, const STACK *, x, a0, ASN1_OBJECT *, obj, a1, int, lastpos, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509v3_get_ext_by_critical(x, crit, lastpos) \
	LP3(0x1d5e, int, X509v3_get_ext_by_critical, const STACK *, x, a0, int, crit, a1, int, lastpos, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509v3_get_ext(x, loc) \
	LP2(0x1d64, X509_EXTENSION *, X509v3_get_ext, const STACK *, x, a0, int, loc, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509v3_delete_ext(x, loc) \
	LP2(0x1d6a, X509_EXTENSION *, X509v3_delete_ext, STACK *, x, a0, int, loc, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509v3_add_ext(x, ex, loc) \
	LP3(0x1d70, *, X509v3_add_ext, STACK **, x, a0, X509_EXTENSION *, ex, a1, int, loc, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_get_ext_count(x) \
	LP1(0x1d76, int, X509_get_ext_count, X509 *, x, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_get_ext_by_NID(x, nid, lastpos) \
	LP3(0x1d7c, int, X509_get_ext_by_NID, X509 *, x, a0, int, nid, a1, int, lastpos, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_get_ext_by_OBJ(x, obj, lastpos) \
	LP3(0x1d82, int, X509_get_ext_by_OBJ, X509 *, x, a0, ASN1_OBJECT *, obj, a1, int, lastpos, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_get_ext_by_critical(x, crit, lastpos) \
	LP3(0x1d88, int, X509_get_ext_by_critical, X509 *, x, a0, int, crit, a1, int, lastpos, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_get_ext(x, loc) \
	LP2(0x1d8e, X509_EXTENSION *, X509_get_ext, X509 *, x, a0, int, loc, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_delete_ext(x, loc) \
	LP2(0x1d94, X509_EXTENSION *, X509_delete_ext, X509 *, x, a0, int, loc, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_add_ext(x, ex, loc) \
	LP3(0x1d9a, int, X509_add_ext, X509 *, x, a0, X509_EXTENSION *, ex, a1, int, loc, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_CRL_get_ext_count(x) \
	LP1(0x1da0, int, X509_CRL_get_ext_count, X509_CRL *, x, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_CRL_get_ext_by_NID(x, nid, lastpos) \
	LP3(0x1da6, int, X509_CRL_get_ext_by_NID, X509_CRL *, x, a0, int, nid, a1, int, lastpos, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_CRL_get_ext_by_OBJ(x, obj, lastpos) \
	LP3(0x1dac, int, X509_CRL_get_ext_by_OBJ, X509_CRL *, x, a0, ASN1_OBJECT *, obj, a1, int, lastpos, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_CRL_get_ext_by_critical(x, crit, lastpos) \
	LP3(0x1db2, int, X509_CRL_get_ext_by_critical, X509_CRL *, x, a0, int, crit, a1, int, lastpos, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_CRL_get_ext(x, loc) \
	LP2(0x1db8, X509_EXTENSION *, X509_CRL_get_ext, X509_CRL *, x, a0, int, loc, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_CRL_delete_ext(x, loc) \
	LP2(0x1dbe, X509_EXTENSION *, X509_CRL_delete_ext, X509_CRL *, x, a0, int, loc, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_CRL_add_ext(x, ex, loc) \
	LP3(0x1dc4, int, X509_CRL_add_ext, X509_CRL *, x, a0, X509_EXTENSION *, ex, a1, int, loc, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_REVOKED_get_ext_count(x) \
	LP1(0x1dca, int, X509_REVOKED_get_ext_count, X509_REVOKED *, x, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_REVOKED_get_ext_by_NID(x, nid, lastpos) \
	LP3(0x1dd0, int, X509_REVOKED_get_ext_by_NID, X509_REVOKED *, x, a0, int, nid, a1, int, lastpos, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_REVOKED_get_ext_by_OBJ(x, obj, lastpos) \
	LP3(0x1dd6, int, X509_REVOKED_get_ext_by_OBJ, X509_REVOKED *, x, a0, ASN1_OBJECT *, obj, a1, int, lastpos, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_REVOKED_get_ext_by_critical(x, crit, lastpos) \
	LP3(0x1ddc, int, X509_REVOKED_get_ext_by_critical, X509_REVOKED *, x, a0, int, crit, a1, int, lastpos, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_REVOKED_get_ext(x, loc) \
	LP2(0x1de2, X509_EXTENSION *, X509_REVOKED_get_ext, X509_REVOKED *, x, a0, int, loc, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_REVOKED_delete_ext(x, loc) \
	LP2(0x1de8, X509_EXTENSION *, X509_REVOKED_delete_ext, X509_REVOKED *, x, a0, int, loc, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_REVOKED_add_ext(x, ex, loc) \
	LP3(0x1dee, int, X509_REVOKED_add_ext, X509_REVOKED *, x, a0, X509_EXTENSION *, ex, a1, int, loc, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_EXTENSION_create_by_NID(ex, nid, crit, data) \
	LP4(0x1df4, X509_EXTENSION *, X509_EXTENSION_create_by_NID, X509_EXTENSION **, ex, a0, int, nid, a1, int, crit, a2, ASN1_OCTET_STRING *, data, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_EXTENSION_create_by_OBJ(ex, obj, crit, data) \
	LP4(0x1dfa, X509_EXTENSION *, X509_EXTENSION_create_by_OBJ, X509_EXTENSION **, ex, a0, ASN1_OBJECT *, obj, a1, int, crit, a2, ASN1_OCTET_STRING *, data, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_EXTENSION_set_object(ex, obj) \
	LP2(0x1e00, int, X509_EXTENSION_set_object, X509_EXTENSION *, ex, a0, ASN1_OBJECT *, obj, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_EXTENSION_set_critical(ex, crit) \
	LP2(0x1e06, int, X509_EXTENSION_set_critical, X509_EXTENSION *, ex, a0, int, crit, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_EXTENSION_set_data(ex, data) \
	LP2(0x1e0c, int, X509_EXTENSION_set_data, X509_EXTENSION *, ex, a0, ASN1_OCTET_STRING *, data, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_EXTENSION_get_object(ex) \
	LP1(0x1e12, ASN1_OBJECT *, X509_EXTENSION_get_object, X509_EXTENSION *, ex, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_EXTENSION_get_data(ne) \
	LP1(0x1e18, ASN1_OCTET_STRING *, X509_EXTENSION_get_data, X509_EXTENSION *, ne, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_EXTENSION_get_critical(ex) \
	LP1(0x1e1e, int, X509_EXTENSION_get_critical, X509_EXTENSION *, ex, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_verify_cert(ctx) \
	LP1(0x1e24, int, X509_verify_cert, X509_STORE_CTX *, ctx, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_find_by_issuer_and_serial(sk, name, serial) \
	LP3(0x1e2a, X509 *, X509_find_by_issuer_and_serial, STACK *, sk, a0, X509_NAME *, name, a1, ASN1_INTEGER *, serial, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_find_by_subject(sk, name) \
	LP2(0x1e30, X509 *, X509_find_by_subject, STACK *, sk, a0, X509_NAME *, name, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_PBEPARAM(a, pp) \
	LP2(0x1e36, int, i2d_PBEPARAM, PBEPARAM *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PBEPARAM_new() \
	LP0(0x1e3c, PBEPARAM *, PBEPARAM_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_PBEPARAM(a, pp, length) \
	LP3(0x1e42, PBEPARAM *, d2i_PBEPARAM, PBEPARAM **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PBEPARAM_free(a) \
	LP1NR(0x1e48, PBEPARAM_free, PBEPARAM *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS5_pbe_set(alg, iter, salt, saltlen) \
	LP4(0x1e4e, X509_ALGOR *, PKCS5_pbe_set, int, alg, a0, int, iter, a1, unsigned char *, salt, a2, int, saltlen, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_PBKDF2PARAM(a, pp) \
	LP2(0x1e54, int, i2d_PBKDF2PARAM, PBKDF2PARAM *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PBKDF2PARAM_new() \
	LP0(0x1e5a, PBKDF2PARAM *, PBKDF2PARAM_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_PBKDF2PARAM(a, pp, length) \
	LP3(0x1e60, PBKDF2PARAM *, d2i_PBKDF2PARAM, PBKDF2PARAM **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PBKDF2PARAM_free(a) \
	LP1NR(0x1e66, PBKDF2PARAM_free, PBKDF2PARAM *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_PBE2PARAM(a, pp) \
	LP2(0x1e6c, int, i2d_PBE2PARAM, PBE2PARAM *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PBE2PARAM_new() \
	LP0(0x1e72, PBE2PARAM *, PBE2PARAM_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_PBE2PARAM(a, pp, length) \
	LP3(0x1e78, PBE2PARAM *, d2i_PBE2PARAM, PBE2PARAM **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PBE2PARAM_free(a) \
	LP1NR(0x1e7e, PBE2PARAM_free, PBE2PARAM *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_PKCS8_PRIV_KEY_INFO(a, pp) \
	LP2(0x1e84, int, i2d_PKCS8_PRIV_KEY_INFO, PKCS8_PRIV_KEY_INFO *, a, a0, unsigned char **, pp, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS8_PRIV_KEY_INFO_new() \
	LP0(0x1e8a, PKCS8_PRIV_KEY_INFO *, PKCS8_PRIV_KEY_INFO_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_PKCS8_PRIV_KEY_INFO(a, pp, length) \
	LP3(0x1e90, PKCS8_PRIV_KEY_INFO *, d2i_PKCS8_PRIV_KEY_INFO, PKCS8_PRIV_KEY_INFO **, a, a0, unsigned char **, pp, a1, long, length, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS8_PRIV_KEY_INFO_free(a) \
	LP1NR(0x1e96, PKCS8_PRIV_KEY_INFO_free, PKCS8_PRIV_KEY_INFO *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_PKCS82PKEY(p8) \
	LP1(0x1e9c, EVP_PKEY *, EVP_PKCS82PKEY, PKCS8_PRIV_KEY_INFO *, p8, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_PKEY2PKCS8(pkey) \
	LP1(0x1ea2, PKCS8_PRIV_KEY_INFO *, EVP_PKEY2PKCS8, EVP_PKEY *, pkey, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS8_set_broken(p8, broken) \
	LP2(0x1ea8, PKCS8_PRIV_KEY_INFO *, PKCS8_set_broken, PKCS8_PRIV_KEY_INFO *, p8, a0, int, broken, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_OBJECT_retrieve_by_subject(h, type, name) \
	LP3(0x1eae, X509_OBJECT *, X509_OBJECT_retrieve_by_subject, LHASH *, h, a0, int, type, a1, X509_NAME *, name, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_OBJECT_up_ref_count(a) \
	LP1NR(0x1eb4, X509_OBJECT_up_ref_count, X509_OBJECT *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_OBJECT_free_contents(a) \
	LP1NR(0x1eba, X509_OBJECT_free_contents, X509_OBJECT *, a, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_STORE_new() \
	LP0(0x1ec0, X509_STORE *, X509_STORE_new, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_STORE_free(v) \
	LP1NR(0x1ec6, X509_STORE_free, X509_STORE *, v, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_STORE_CTX_init(ctx, store, x509, chain) \
	LP4NR(0x1ecc, X509_STORE_CTX_init, X509_STORE_CTX *, ctx, a0, X509_STORE *, store, a1, X509 *, x509, a2, STACK *, chain, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_STORE_CTX_cleanup(ctx) \
	LP1NR(0x1ed2, X509_STORE_CTX_cleanup, X509_STORE_CTX *, ctx, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_STORE_add_lookup(v, m) \
	LP2(0x1ed8, X509_LOOKUP *, X509_STORE_add_lookup, X509_STORE *, v, a0, X509_LOOKUP_METHOD *, m, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_LOOKUP_hash_dir() \
	LP0(0x1ede, X509_LOOKUP_METHOD *, X509_LOOKUP_hash_dir, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_LOOKUP_file() \
	LP0(0x1ee4, X509_LOOKUP_METHOD *, X509_LOOKUP_file, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_STORE_add_cert(ctx, x) \
	LP2(0x1eea, int, X509_STORE_add_cert, X509_STORE *, ctx, a0, X509 *, x, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_STORE_add_crl(ctx, x) \
	LP2(0x1ef0, int, X509_STORE_add_crl, X509_STORE *, ctx, a0, X509_CRL *, x, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_STORE_get_by_subject(vs, type, name, ret) \
	LP4(0x1ef6, int, X509_STORE_get_by_subject, X509_STORE_CTX *, vs, a0, int, type, a1, X509_NAME *, name, a2, X509_OBJECT *, ret, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_LOOKUP_ctrl(ctx, cmd, argc, argl, ret) \
	LP5(0x1efc, int, X509_LOOKUP_ctrl, X509_LOOKUP *, ctx, a0, int, cmd, a1, const char *, argc, a2, long, argl, a3, char **, ret, d0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_load_cert_file(ctx, file, type) \
	LP3(0x1f02, int, X509_load_cert_file, X509_LOOKUP *, ctx, a0, const char *, file, a1, int, type, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_load_crl_file(ctx, file, type) \
	LP3(0x1f08, int, X509_load_crl_file, X509_LOOKUP *, ctx, a0, const char *, file, a1, int, type, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_LOOKUP_new(method) \
	LP1(0x1f0e, X509_LOOKUP *, X509_LOOKUP_new, X509_LOOKUP_METHOD *, method, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_LOOKUP_free(ctx) \
	LP1NR(0x1f14, X509_LOOKUP_free, X509_LOOKUP *, ctx, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_LOOKUP_init(ctx) \
	LP1(0x1f1a, int, X509_LOOKUP_init, X509_LOOKUP *, ctx, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_LOOKUP_by_subject(ctx, type, name, ret) \
	LP4(0x1f20, int, X509_LOOKUP_by_subject, X509_LOOKUP *, ctx, a0, int, type, a1, X509_NAME *, name, a2, X509_OBJECT *, ret, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_LOOKUP_by_issuer_serial(ctx, type, name, serial, ret) \
	LP5(0x1f26, int, X509_LOOKUP_by_issuer_serial, X509_LOOKUP *, ctx, a0, int, type, a1, X509_NAME *, name, a2, ASN1_INTEGER *, serial, a3, X509_OBJECT *, ret, d0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_LOOKUP_by_fingerprint(ctx, type, bytes, len, ret) \
	LP5(0x1f2c, int, X509_LOOKUP_by_fingerprint, X509_LOOKUP *, ctx, a0, int, type, a1, unsigned char *, bytes, a2, int, len, a3, X509_OBJECT *, ret, d0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_LOOKUP_by_alias(ctx, type, str, len, ret) \
	LP5(0x1f32, int, X509_LOOKUP_by_alias, X509_LOOKUP *, ctx, a0, int, type, a1, char *, str, a2, int, len, a3, X509_OBJECT *, ret, d0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_LOOKUP_shutdown(ctx) \
	LP1(0x1f38, int, X509_LOOKUP_shutdown, X509_LOOKUP *, ctx, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_STORE_load_locations(ctx, file, dir) \
	LP3(0x1f3e, int, X509_STORE_load_locations, X509_STORE *, ctx, a0, const char *, file, a1, const char *, dir, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_STORE_set_default_paths(ctx) \
	LP1(0x1f44, int, X509_STORE_set_default_paths, X509_STORE *, ctx, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_STORE_CTX_get_ex_new_index(argl, argp, new_func, dup_func, free_func) \
	LP5FP(0x1f4a, int, X509_STORE_CTX_get_ex_new_index, long, argl, a0, char *, argp, a1, __fpt, new_func, a2, void *, dup_func, a3, void *, free_func, d0, \
	, AMISSL_BASE_NAME, int (*__fpt)(), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_STORE_CTX_set_ex_data(ctx, idx, data) \
	LP3(0x1f50, int, X509_STORE_CTX_set_ex_data, X509_STORE_CTX *, ctx, a0, int, idx, a1, void *, data, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_STORE_CTX_get_ex_data(ctx, idx) \
	LP2(0x1f56, void *, X509_STORE_CTX_get_ex_data, X509_STORE_CTX *, ctx, a0, int, idx, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_STORE_CTX_get_error(ctx) \
	LP1(0x1f5c, int, X509_STORE_CTX_get_error, X509_STORE_CTX *, ctx, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_STORE_CTX_set_error(ctx, s) \
	LP2NR(0x1f62, X509_STORE_CTX_set_error, X509_STORE_CTX *, ctx, a0, int, s, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_STORE_CTX_get_error_depth(ctx) \
	LP1(0x1f68, int, X509_STORE_CTX_get_error_depth, X509_STORE_CTX *, ctx, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_STORE_CTX_get_current_cert(ctx) \
	LP1(0x1f6e, X509 *, X509_STORE_CTX_get_current_cert, X509_STORE_CTX *, ctx, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_STORE_CTX_get_chain(ctx) \
	LP1(0x1f74, *, X509_STORE_CTX_get_chain, X509_STORE_CTX *, ctx, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_STORE_CTX_set_cert(c, x) \
	LP2NR(0x1f7a, X509_STORE_CTX_set_cert, X509_STORE_CTX *, c, a0, X509 *, x, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define X509_STORE_CTX_set_chain(c, sk) \
	LP2NR(0x1f80, X509_STORE_CTX_set_chain, X509_STORE_CTX *, c, a0, STACK *, sk, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_ctrl_pending(b) \
	LP1(0x1f86, size_t, BIO_ctrl_pending, BIO *, b, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_ctrl_wpending(b) \
	LP1(0x1f8c, size_t, BIO_ctrl_wpending, BIO *, b, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_ctrl_get_write_guarantee(b) \
	LP1(0x1f92, size_t, BIO_ctrl_get_write_guarantee, BIO *, b, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_ctrl_get_read_request(b) \
	LP1(0x1f98, size_t, BIO_ctrl_get_read_request, BIO *, b, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_s_bio() \
	LP0(0x1f9e, BIO_METHOD *, BIO_s_bio, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define BIO_new_bio_pair(bio1, writebuf1, bio2, writebuf2) \
	LP4(0x1fa4, int, BIO_new_bio_pair, BIO **, bio1, a0, size_t, writebuf1, a1, BIO **, bio2, a2, size_t, writebuf2, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CRYPTO_num_locks() \
	LP0(0x1faa, int, CRYPTO_num_locks, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define DSA_dup_DH(r) \
	LP1(0x1fb0, DH *, DSA_dup_DH, DSA *, r, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS5_PBE_keyivgen(ctx, pass, passlen, param, cipher, md, en_de) \
	LP7(0x1fb6, int, PKCS5_PBE_keyivgen, EVP_CIPHER_CTX *, ctx, a0, const char *, pass, a1, int, passlen, a2, ASN1_TYPE *, param, a3, EVP_CIPHER *, cipher, d0, EVP_MD *, md, d1, int, en_de, d2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS5_PBKDF2_HMAC_SHA1(pass, passlen, salt, saltlen, iter, keylen, out) \
	LP7(0x1fbc, int, PKCS5_PBKDF2_HMAC_SHA1, const char *, pass, a0, int, passlen, a1, unsigned char *, salt, a2, int, saltlen, a3, int, iter, d0, int, keylen, d1, unsigned char *, out, d2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS5_v2_PBE_keyivgen(ctx, pass, passlen, param, cipher, md, en_de) \
	LP7(0x1fc2, int, PKCS5_v2_PBE_keyivgen, EVP_CIPHER_CTX *, ctx, a0, const char *, pass, a1, int, passlen, a2, ASN1_TYPE *, param, a3, EVP_CIPHER *, cipher, d0, EVP_MD *, md, d1, int, en_de, d2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS5_PBE_add() \
	LP0NR(0x1fc8, PKCS5_PBE_add, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_PBE_CipherInit(pbe_obj, pass, passlen, param, ctx, en_de) \
	LP6(0x1fce, int, EVP_PBE_CipherInit, ASN1_OBJECT *, pbe_obj, a0, const char *, pass, a1, int, passlen, a2, ASN1_TYPE *, param, a3, EVP_CIPHER_CTX *, ctx, d0, int, en_de, d1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_PBE_alg_add(nid, cipher, md, keygen) \
	LP4(0x1fd4, int, EVP_PBE_alg_add, int, nid, a0, EVP_CIPHER *, cipher, a1, EVP_MD *, md, a2, EVP_PBE_KEYGEN *, keygen, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define EVP_PBE_cleanup() \
	LP0NR(0x1fda, EVP_PBE_cleanup, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define OBJ_obj2txt(buf, buf_len, a, no_name) \
	LP4(0x1fe0, int, OBJ_obj2txt, char *, buf, a0, int, buf_len, a1, ASN1_OBJECT *, a, a2, int, no_name, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS7_signatureVerify(bio, p7, si, x509) \
	LP4(0x1fe6, int, PKCS7_signatureVerify, BIO *, bio, a0, PKCS7 *, p7, a1, PKCS7_SIGNER_INFO *, si, a2, X509 *, x509, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define RSA_check_key(rsa) \
	LP1(0x1fec, int, RSA_check_key, RSA *, rsa, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define RSA_get_default_method() \
	LP0(0x1ff2, RSA_METHOD *, RSA_get_default_method, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define RSA_get_method(rsa) \
	LP1(0x1ff8, RSA_METHOD *, RSA_get_method, RSA *, rsa, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define RSA_set_method(rsa, meth) \
	LP2(0x1ffe, RSA_METHOD *, RSA_set_method, RSA *, rsa, a0, RSA_METHOD *, meth, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define sk_sort(st) \
	LP1NR(0x2004, sk_sort, STACK *, st, a0, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SSL_CTX_set_default_passwd_cb_userdata(ctx, u) \
	LP2NR(0x200a, SSL_CTX_set_default_passwd_cb_userdata, SSL_CTX *, ctx, a0, void *, u, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_PKCS8_bio(bp, p8) \
	LP2(0x2010, X509_SIG *, d2i_PKCS8_bio, BIO *, bp, a0, X509_SIG **, p8, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_PKCS8_bio(bp, p8) \
	LP2(0x2016, int, i2d_PKCS8_bio, BIO *, bp, a0, X509_SIG *, p8, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define d2i_PKCS8_PRIV_KEY_INFO_bio(bp, p8inf) \
	LP2(0x201c, PKCS8_PRIV_KEY_INFO *, d2i_PKCS8_PRIV_KEY_INFO_bio, BIO *, bp, a0, PKCS8_PRIV_KEY_INFO **, p8inf, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define i2d_PKCS8_PRIV_KEY_INFO_bio(bp, p8inf) \
	LP2(0x2022, int, i2d_PKCS8_PRIV_KEY_INFO_bio, BIO *, bp, a0, PKCS8_PRIV_KEY_INFO *, p8inf, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS5_pbe2_set(cipher, iter, salt, saltlen) \
	LP4(0x2028, X509_ALGOR *, PKCS5_pbe2_set, const EVP_CIPHER *, cipher, a0, int, iter, a1, unsigned char *, salt, a2, int, saltlen, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PKCS12_PBE_keyivgen(ctx, pass, passlen, param, cipher, md_type, en_de) \
	LP7(0x202e, int, PKCS12_PBE_keyivgen, EVP_CIPHER_CTX *, ctx, a0, const char *, pass, a1, int, passlen, a2, ASN1_TYPE *, param, a3, EVP_CIPHER *, cipher, d0, EVP_MD *, md_type, d1, int, en_de, d2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PEM_read_bio_PKCS8(bp, x, cb, u) \
	LP4(0x2034, X509_SIG *, PEM_read_bio_PKCS8, BIO *, bp, a0, X509_SIG **, x, a1, pem_password_cb *, cb, a2, void *, u, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PEM_read_bio_PKCS8_PRIV_KEY_INFO(bp, x, cb, u) \
	LP4(0x203a, PKCS8_PRIV_KEY_INFO *, PEM_read_bio_PKCS8_PRIV_KEY_INFO, BIO *, bp, a0, PKCS8_PRIV_KEY_INFO **, x, a1, pem_password_cb *, cb, a2, void *, u, a3, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PEM_write_bio_PKCS8(bp, x) \
	LP2(0x2040, int, PEM_write_bio_PKCS8, BIO *, bp, a0, X509_SIG *, x, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PEM_write_bio_PKCS8_PRIV_KEY_INFO(bp, x) \
	LP2(0x2046, int, PEM_write_bio_PKCS8_PRIV_KEY_INFO, BIO *, bp, a0, PKCS8_PRIV_KEY_INFO *, x, a1, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PEM_write_bio_PKCS8PrivateKey(fp, x, enc, kstr, klen, cb, u) \
	LP7(0x204c, int, PEM_write_bio_PKCS8PrivateKey, BIO *, fp, a0, EVP_PKEY *, x, a1, const EVP_CIPHER *, enc, a2, char *, kstr, a3, int, klen, d0, pem_password_cb *, cb, d1, void *, u, d2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CONF_load_bio(conf, bp, eline) \
	LP3(0x2052, LHASH *, CONF_load_bio, LHASH *, conf, a0, BIO *, bp, a1, long *, eline, a2, \
	, AMISSL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#endif /*  _PPCINLINE_AMISSL_H  */
