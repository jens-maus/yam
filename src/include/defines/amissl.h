/* Automatically generated header (sfdc 1.4)! Do not edit! */

#ifndef _INLINE_AMISSL_H
#define _INLINE_AMISSL_H

#ifndef _SFDC_VARARG_DEFINED
#define _SFDC_VARARG_DEFINED
#ifdef __HAVE_IPTR_ATTR__
typedef APTR _sfdc_vararg __attribute__((iptr));
#else
typedef ULONG _sfdc_vararg;
#endif /* __HAVE_IPTR_ATTR__ */
#endif /* _SFDC_VARARG_DEFINED */

#ifndef AROS_LIBCALL_H
#include <aros/libcall.h>
#endif /* !AROS_LIBCALL_H */

#ifndef AMISSL_BASE_NAME
#define AMISSL_BASE_NAME AmiSSLBase
#endif /* !AMISSL_BASE_NAME */

#define InitAmiSSLA(___tagList) \
	AROS_LC1(long, InitAmiSSLA, \
	AROS_LCA(struct TagItem *, (___tagList), A0), \
	struct Library *, AMISSL_BASE_NAME, 6, Amissl)

#ifndef NO_INLINE_STDARG
#define InitAmiSSL(___tagList, ...) \
	({_sfdc_vararg _tags[] = { ___tagList, __VA_ARGS__ }; InitAmiSSLA((struct TagItem *) _tags); })
#endif /* !NO_INLINE_STDARG */

#define CleanupAmiSSLA(___tagList) \
	AROS_LC1(long, CleanupAmiSSLA, \
	AROS_LCA(struct TagItem *, (___tagList), A0), \
	struct Library *, AMISSL_BASE_NAME, 7, Amissl)

#ifndef NO_INLINE_STDARG
#define CleanupAmiSSL(___tagList, ...) \
	({_sfdc_vararg _tags[] = { ___tagList, __VA_ARGS__ }; CleanupAmiSSLA((struct TagItem *) _tags); })
#endif /* !NO_INLINE_STDARG */

#define IsCipherAvailable(___cipher) \
	AROS_LC1(long, IsCipherAvailable, \
	AROS_LCA(long, (___cipher), D0), \
	struct Library *, AMISSL_BASE_NAME, 8, Amissl)

#define ASN1_TYPE_new() \
	AROS_LC0(ASN1_TYPE *, ASN1_TYPE_new, \
	struct Library *, AMISSL_BASE_NAME, 17, Amissl)

#define ASN1_TYPE_free(___a) \
	AROS_LC1(void, ASN1_TYPE_free, \
	AROS_LCA(ASN1_TYPE *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 18, Amissl)

#define d2i_ASN1_TYPE(___a, ___in, ___len) \
	AROS_LC3(ASN1_TYPE *, d2i_ASN1_TYPE, \
	AROS_LCA(ASN1_TYPE **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 19, Amissl)

#define i2d_ASN1_TYPE(___a, ___out) \
	AROS_LC2(int, i2d_ASN1_TYPE, \
	AROS_LCA(ASN1_TYPE *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 20, Amissl)

#define ASN1_ANY_it() \
	AROS_LC0(const ASN1_ITEM *, ASN1_ANY_it, \
	struct Library *, AMISSL_BASE_NAME, 21, Amissl)

#define ASN1_TYPE_get(___a) \
	AROS_LC1(int, ASN1_TYPE_get, \
	AROS_LCA(ASN1_TYPE *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 22, Amissl)

#define ASN1_TYPE_set(___a, ___type, ___value) \
	AROS_LC3(void, ASN1_TYPE_set, \
	AROS_LCA(ASN1_TYPE *, (___a), A0), \
	AROS_LCA(int, (___type), D0), \
	AROS_LCA(void *, (___value), A1), \
	struct Library *, AMISSL_BASE_NAME, 23, Amissl)

#define ASN1_OBJECT_new() \
	AROS_LC0(ASN1_OBJECT *, ASN1_OBJECT_new, \
	struct Library *, AMISSL_BASE_NAME, 24, Amissl)

#define ASN1_OBJECT_free(___a) \
	AROS_LC1(void, ASN1_OBJECT_free, \
	AROS_LCA(ASN1_OBJECT *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 25, Amissl)

#define i2d_ASN1_OBJECT(___a, ___pp) \
	AROS_LC2(int, i2d_ASN1_OBJECT, \
	AROS_LCA(ASN1_OBJECT *, (___a), A0), \
	AROS_LCA(unsigned char **, (___pp), A1), \
	struct Library *, AMISSL_BASE_NAME, 26, Amissl)

#define c2i_ASN1_OBJECT(___a, ___pp, ___length) \
	AROS_LC3(ASN1_OBJECT *, c2i_ASN1_OBJECT, \
	AROS_LCA(ASN1_OBJECT **, (___a), A0), \
	AROS_LCA(unsigned char **, (___pp), A1), \
	AROS_LCA(long, (___length), D0), \
	struct Library *, AMISSL_BASE_NAME, 27, Amissl)

#define d2i_ASN1_OBJECT(___a, ___pp, ___length) \
	AROS_LC3(ASN1_OBJECT *, d2i_ASN1_OBJECT, \
	AROS_LCA(ASN1_OBJECT **, (___a), A0), \
	AROS_LCA(unsigned char **, (___pp), A1), \
	AROS_LCA(long, (___length), D0), \
	struct Library *, AMISSL_BASE_NAME, 28, Amissl)

#define ASN1_OBJECT_it() \
	AROS_LC0(const ASN1_ITEM *, ASN1_OBJECT_it, \
	struct Library *, AMISSL_BASE_NAME, 29, Amissl)

#define ASN1_STRING_new() \
	AROS_LC0(ASN1_STRING *, ASN1_STRING_new, \
	struct Library *, AMISSL_BASE_NAME, 30, Amissl)

#define ASN1_STRING_free(___a) \
	AROS_LC1(void, ASN1_STRING_free, \
	AROS_LCA(ASN1_STRING *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 31, Amissl)

#define ASN1_STRING_dup(___a) \
	AROS_LC1(ASN1_STRING *, ASN1_STRING_dup, \
	AROS_LCA(ASN1_STRING *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 32, Amissl)

#define ASN1_STRING_type_new(___type) \
	AROS_LC1(ASN1_STRING *, ASN1_STRING_type_new, \
	AROS_LCA(int, (___type), D0), \
	struct Library *, AMISSL_BASE_NAME, 33, Amissl)

#define ASN1_STRING_cmp(___a, ___b) \
	AROS_LC2(int, ASN1_STRING_cmp, \
	AROS_LCA(ASN1_STRING *, (___a), A0), \
	AROS_LCA(ASN1_STRING *, (___b), A1), \
	struct Library *, AMISSL_BASE_NAME, 34, Amissl)

#define ASN1_STRING_set(___str, ___data, ___len) \
	AROS_LC3(int, ASN1_STRING_set, \
	AROS_LCA(ASN1_STRING *, (___str), A0), \
	AROS_LCA(const void *, (___data), A1), \
	AROS_LCA(int, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 35, Amissl)

#define ASN1_STRING_length(___x) \
	AROS_LC1(int, ASN1_STRING_length, \
	AROS_LCA(ASN1_STRING *, (___x), A0), \
	struct Library *, AMISSL_BASE_NAME, 36, Amissl)

#define ASN1_STRING_length_set(___x, ___n) \
	AROS_LC2(void, ASN1_STRING_length_set, \
	AROS_LCA(ASN1_STRING *, (___x), A0), \
	AROS_LCA(int, (___n), D0), \
	struct Library *, AMISSL_BASE_NAME, 37, Amissl)

#define ASN1_STRING_type(___x) \
	AROS_LC1(int, ASN1_STRING_type, \
	AROS_LCA(ASN1_STRING *, (___x), A0), \
	struct Library *, AMISSL_BASE_NAME, 38, Amissl)

#define ASN1_STRING_data(___x) \
	AROS_LC1(unsigned char *, ASN1_STRING_data, \
	AROS_LCA(ASN1_STRING *, (___x), A0), \
	struct Library *, AMISSL_BASE_NAME, 39, Amissl)

#define ASN1_BIT_STRING_new() \
	AROS_LC0(ASN1_BIT_STRING *, ASN1_BIT_STRING_new, \
	struct Library *, AMISSL_BASE_NAME, 40, Amissl)

#define ASN1_BIT_STRING_free(___a) \
	AROS_LC1(void, ASN1_BIT_STRING_free, \
	AROS_LCA(ASN1_BIT_STRING *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 41, Amissl)

#define d2i_ASN1_BIT_STRING(___a, ___in, ___len) \
	AROS_LC3(ASN1_BIT_STRING *, d2i_ASN1_BIT_STRING, \
	AROS_LCA(ASN1_BIT_STRING **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 42, Amissl)

#define i2d_ASN1_BIT_STRING(___a, ___out) \
	AROS_LC2(int, i2d_ASN1_BIT_STRING, \
	AROS_LCA(ASN1_BIT_STRING *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 43, Amissl)

#define ASN1_BIT_STRING_it() \
	AROS_LC0(const ASN1_ITEM *, ASN1_BIT_STRING_it, \
	struct Library *, AMISSL_BASE_NAME, 44, Amissl)

#define i2c_ASN1_BIT_STRING(___a, ___pp) \
	AROS_LC2(int, i2c_ASN1_BIT_STRING, \
	AROS_LCA(ASN1_BIT_STRING *, (___a), A0), \
	AROS_LCA(unsigned char **, (___pp), A1), \
	struct Library *, AMISSL_BASE_NAME, 45, Amissl)

#define c2i_ASN1_BIT_STRING(___a, ___pp, ___length) \
	AROS_LC3(ASN1_BIT_STRING *, c2i_ASN1_BIT_STRING, \
	AROS_LCA(ASN1_BIT_STRING **, (___a), A0), \
	AROS_LCA(unsigned char **, (___pp), A1), \
	AROS_LCA(long, (___length), D0), \
	struct Library *, AMISSL_BASE_NAME, 46, Amissl)

#define ASN1_BIT_STRING_set(___a, ___d, ___length) \
	AROS_LC3(int, ASN1_BIT_STRING_set, \
	AROS_LCA(ASN1_BIT_STRING *, (___a), A0), \
	AROS_LCA(unsigned char *, (___d), A1), \
	AROS_LCA(int, (___length), D0), \
	struct Library *, AMISSL_BASE_NAME, 47, Amissl)

#define ASN1_BIT_STRING_set_bit(___a, ___n, ___value) \
	AROS_LC3(int, ASN1_BIT_STRING_set_bit, \
	AROS_LCA(ASN1_BIT_STRING *, (___a), A0), \
	AROS_LCA(int, (___n), D0), \
	AROS_LCA(int, (___value), D1), \
	struct Library *, AMISSL_BASE_NAME, 48, Amissl)

#define ASN1_BIT_STRING_get_bit(___a, ___n) \
	AROS_LC2(int, ASN1_BIT_STRING_get_bit, \
	AROS_LCA(ASN1_BIT_STRING *, (___a), A0), \
	AROS_LCA(int, (___n), D0), \
	struct Library *, AMISSL_BASE_NAME, 49, Amissl)

#define ASN1_BIT_STRING_name_print(___out, ___bs, ___tbl, ___indent) \
	AROS_LC4(int, ASN1_BIT_STRING_name_print, \
	AROS_LCA(BIO *, (___out), A0), \
	AROS_LCA(ASN1_BIT_STRING *, (___bs), A1), \
	AROS_LCA(BIT_STRING_BITNAME *, (___tbl), A2), \
	AROS_LCA(int, (___indent), D0), \
	struct Library *, AMISSL_BASE_NAME, 50, Amissl)

#define ASN1_BIT_STRING_num_asc(___name, ___tbl) \
	AROS_LC2(int, ASN1_BIT_STRING_num_asc, \
	AROS_LCA(char *, (___name), A0), \
	AROS_LCA(BIT_STRING_BITNAME *, (___tbl), A1), \
	struct Library *, AMISSL_BASE_NAME, 51, Amissl)

#define ASN1_BIT_STRING_set_asc(___bs, ___name, ___value, ___tbl) \
	AROS_LC4(int, ASN1_BIT_STRING_set_asc, \
	AROS_LCA(ASN1_BIT_STRING *, (___bs), A0), \
	AROS_LCA(char *, (___name), A1), \
	AROS_LCA(int, (___value), D0), \
	AROS_LCA(BIT_STRING_BITNAME *, (___tbl), A2), \
	struct Library *, AMISSL_BASE_NAME, 52, Amissl)

#define i2d_ASN1_BOOLEAN(___a, ___pp) \
	AROS_LC2(int, i2d_ASN1_BOOLEAN, \
	AROS_LCA(int, (___a), D0), \
	AROS_LCA(unsigned char **, (___pp), A0), \
	struct Library *, AMISSL_BASE_NAME, 53, Amissl)

#define d2i_ASN1_BOOLEAN(___a, ___pp, ___length) \
	AROS_LC3(int, d2i_ASN1_BOOLEAN, \
	AROS_LCA(int *, (___a), A0), \
	AROS_LCA(unsigned char **, (___pp), A1), \
	AROS_LCA(long, (___length), D0), \
	struct Library *, AMISSL_BASE_NAME, 54, Amissl)

#define ASN1_INTEGER_new() \
	AROS_LC0(ASN1_INTEGER *, ASN1_INTEGER_new, \
	struct Library *, AMISSL_BASE_NAME, 55, Amissl)

#define ASN1_INTEGER_free(___a) \
	AROS_LC1(void, ASN1_INTEGER_free, \
	AROS_LCA(ASN1_INTEGER *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 56, Amissl)

#define d2i_ASN1_INTEGER(___a, ___in, ___len) \
	AROS_LC3(ASN1_INTEGER *, d2i_ASN1_INTEGER, \
	AROS_LCA(ASN1_INTEGER **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 57, Amissl)

#define i2d_ASN1_INTEGER(___a, ___out) \
	AROS_LC2(int, i2d_ASN1_INTEGER, \
	AROS_LCA(ASN1_INTEGER *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 58, Amissl)

#define ASN1_INTEGER_it() \
	AROS_LC0(const ASN1_ITEM *, ASN1_INTEGER_it, \
	struct Library *, AMISSL_BASE_NAME, 59, Amissl)

#define i2c_ASN1_INTEGER(___a, ___pp) \
	AROS_LC2(int, i2c_ASN1_INTEGER, \
	AROS_LCA(ASN1_INTEGER *, (___a), A0), \
	AROS_LCA(unsigned char **, (___pp), A1), \
	struct Library *, AMISSL_BASE_NAME, 60, Amissl)

#define c2i_ASN1_INTEGER(___a, ___pp, ___length) \
	AROS_LC3(ASN1_INTEGER *, c2i_ASN1_INTEGER, \
	AROS_LCA(ASN1_INTEGER **, (___a), A0), \
	AROS_LCA(unsigned char **, (___pp), A1), \
	AROS_LCA(long, (___length), D0), \
	struct Library *, AMISSL_BASE_NAME, 61, Amissl)

#define d2i_ASN1_UINTEGER(___a, ___pp, ___length) \
	AROS_LC3(ASN1_INTEGER *, d2i_ASN1_UINTEGER, \
	AROS_LCA(ASN1_INTEGER **, (___a), A0), \
	AROS_LCA(unsigned char **, (___pp), A1), \
	AROS_LCA(long, (___length), D0), \
	struct Library *, AMISSL_BASE_NAME, 62, Amissl)

#define ASN1_INTEGER_dup(___x) \
	AROS_LC1(ASN1_INTEGER *, ASN1_INTEGER_dup, \
	AROS_LCA(ASN1_INTEGER *, (___x), A0), \
	struct Library *, AMISSL_BASE_NAME, 63, Amissl)

#define ASN1_INTEGER_cmp(___x, ___y) \
	AROS_LC2(int, ASN1_INTEGER_cmp, \
	AROS_LCA(ASN1_INTEGER *, (___x), A0), \
	AROS_LCA(ASN1_INTEGER *, (___y), A1), \
	struct Library *, AMISSL_BASE_NAME, 64, Amissl)

#define ASN1_ENUMERATED_new() \
	AROS_LC0(ASN1_ENUMERATED *, ASN1_ENUMERATED_new, \
	struct Library *, AMISSL_BASE_NAME, 65, Amissl)

#define ASN1_ENUMERATED_free(___a) \
	AROS_LC1(void, ASN1_ENUMERATED_free, \
	AROS_LCA(ASN1_ENUMERATED *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 66, Amissl)

#define d2i_ASN1_ENUMERATED(___a, ___in, ___len) \
	AROS_LC3(ASN1_ENUMERATED *, d2i_ASN1_ENUMERATED, \
	AROS_LCA(ASN1_ENUMERATED **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 67, Amissl)

#define i2d_ASN1_ENUMERATED(___a, ___out) \
	AROS_LC2(int, i2d_ASN1_ENUMERATED, \
	AROS_LCA(ASN1_ENUMERATED *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 68, Amissl)

#define ASN1_ENUMERATED_it() \
	AROS_LC0(const ASN1_ITEM *, ASN1_ENUMERATED_it, \
	struct Library *, AMISSL_BASE_NAME, 69, Amissl)

#define ASN1_UTCTIME_check(___a) \
	AROS_LC1(int, ASN1_UTCTIME_check, \
	AROS_LCA(ASN1_UTCTIME *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 70, Amissl)

#define ASN1_UTCTIME_set(___s, ___t) \
	AROS_LC2(ASN1_UTCTIME *, ASN1_UTCTIME_set, \
	AROS_LCA(ASN1_UTCTIME *, (___s), A0), \
	AROS_LCA(time_t, (___t), D0), \
	struct Library *, AMISSL_BASE_NAME, 71, Amissl)

#define ASN1_UTCTIME_set_string(___s, ___str) \
	AROS_LC2(int, ASN1_UTCTIME_set_string, \
	AROS_LCA(ASN1_UTCTIME *, (___s), A0), \
	AROS_LCA(char *, (___str), A1), \
	struct Library *, AMISSL_BASE_NAME, 72, Amissl)

#define ASN1_UTCTIME_cmp_time_t(___s, ___t) \
	AROS_LC2(int, ASN1_UTCTIME_cmp_time_t, \
	AROS_LCA(const ASN1_UTCTIME *, (___s), A0), \
	AROS_LCA(time_t, (___t), D0), \
	struct Library *, AMISSL_BASE_NAME, 73, Amissl)

#define ASN1_GENERALIZEDTIME_check(___a) \
	AROS_LC1(int, ASN1_GENERALIZEDTIME_check, \
	AROS_LCA(ASN1_GENERALIZEDTIME *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 74, Amissl)

#define ASN1_GENERALIZEDTIME_set(___s, ___t) \
	AROS_LC2(ASN1_GENERALIZEDTIME *, ASN1_GENERALIZEDTIME_set, \
	AROS_LCA(ASN1_GENERALIZEDTIME *, (___s), A0), \
	AROS_LCA(time_t, (___t), D0), \
	struct Library *, AMISSL_BASE_NAME, 75, Amissl)

#define ASN1_GENERALIZEDTIME_set_string(___s, ___str) \
	AROS_LC2(int, ASN1_GENERALIZEDTIME_set_string, \
	AROS_LCA(ASN1_GENERALIZEDTIME *, (___s), A0), \
	AROS_LCA(char *, (___str), A1), \
	struct Library *, AMISSL_BASE_NAME, 76, Amissl)

#define ASN1_OCTET_STRING_new() \
	AROS_LC0(ASN1_OCTET_STRING *, ASN1_OCTET_STRING_new, \
	struct Library *, AMISSL_BASE_NAME, 77, Amissl)

#define ASN1_OCTET_STRING_free(___a) \
	AROS_LC1(void, ASN1_OCTET_STRING_free, \
	AROS_LCA(ASN1_OCTET_STRING *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 78, Amissl)

#define d2i_ASN1_OCTET_STRING(___a, ___in, ___len) \
	AROS_LC3(ASN1_OCTET_STRING *, d2i_ASN1_OCTET_STRING, \
	AROS_LCA(ASN1_OCTET_STRING **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 79, Amissl)

#define i2d_ASN1_OCTET_STRING(___a, ___out) \
	AROS_LC2(int, i2d_ASN1_OCTET_STRING, \
	AROS_LCA(ASN1_OCTET_STRING *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 80, Amissl)

#define ASN1_OCTET_STRING_it() \
	AROS_LC0(const ASN1_ITEM *, ASN1_OCTET_STRING_it, \
	struct Library *, AMISSL_BASE_NAME, 81, Amissl)

#define ASN1_OCTET_STRING_dup(___a) \
	AROS_LC1(ASN1_OCTET_STRING *, ASN1_OCTET_STRING_dup, \
	AROS_LCA(ASN1_OCTET_STRING *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 82, Amissl)

#define ASN1_OCTET_STRING_cmp(___a, ___b) \
	AROS_LC2(int, ASN1_OCTET_STRING_cmp, \
	AROS_LCA(ASN1_OCTET_STRING *, (___a), A0), \
	AROS_LCA(ASN1_OCTET_STRING *, (___b), A1), \
	struct Library *, AMISSL_BASE_NAME, 83, Amissl)

#define ASN1_OCTET_STRING_set(___str, ___data, ___len) \
	AROS_LC3(int, ASN1_OCTET_STRING_set, \
	AROS_LCA(ASN1_OCTET_STRING *, (___str), A0), \
	AROS_LCA(unsigned char *, (___data), A1), \
	AROS_LCA(int, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 84, Amissl)

#define ASN1_VISIBLESTRING_new() \
	AROS_LC0(ASN1_VISIBLESTRING *, ASN1_VISIBLESTRING_new, \
	struct Library *, AMISSL_BASE_NAME, 85, Amissl)

#define ASN1_VISIBLESTRING_free(___a) \
	AROS_LC1(void, ASN1_VISIBLESTRING_free, \
	AROS_LCA(ASN1_VISIBLESTRING *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 86, Amissl)

#define d2i_ASN1_VISIBLESTRING(___a, ___in, ___len) \
	AROS_LC3(ASN1_VISIBLESTRING *, d2i_ASN1_VISIBLESTRING, \
	AROS_LCA(ASN1_VISIBLESTRING **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 87, Amissl)

#define i2d_ASN1_VISIBLESTRING(___a, ___out) \
	AROS_LC2(int, i2d_ASN1_VISIBLESTRING, \
	AROS_LCA(ASN1_VISIBLESTRING *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 88, Amissl)

#define ASN1_VISIBLESTRING_it() \
	AROS_LC0(const ASN1_ITEM *, ASN1_VISIBLESTRING_it, \
	struct Library *, AMISSL_BASE_NAME, 89, Amissl)

#define ASN1_UNIVERSALSTRING_new() \
	AROS_LC0(ASN1_UNIVERSALSTRING *, ASN1_UNIVERSALSTRING_new, \
	struct Library *, AMISSL_BASE_NAME, 90, Amissl)

#define ASN1_UNIVERSALSTRING_free(___a) \
	AROS_LC1(void, ASN1_UNIVERSALSTRING_free, \
	AROS_LCA(ASN1_UNIVERSALSTRING *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 91, Amissl)

#define d2i_ASN1_UNIVERSALSTRING(___a, ___in, ___len) \
	AROS_LC3(ASN1_UNIVERSALSTRING *, d2i_ASN1_UNIVERSALSTRING, \
	AROS_LCA(ASN1_UNIVERSALSTRING **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 92, Amissl)

#define i2d_ASN1_UNIVERSALSTRING(___a, ___out) \
	AROS_LC2(int, i2d_ASN1_UNIVERSALSTRING, \
	AROS_LCA(ASN1_UNIVERSALSTRING *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 93, Amissl)

#define ASN1_UNIVERSALSTRING_it() \
	AROS_LC0(const ASN1_ITEM *, ASN1_UNIVERSALSTRING_it, \
	struct Library *, AMISSL_BASE_NAME, 94, Amissl)

#define ASN1_UTF8STRING_new() \
	AROS_LC0(ASN1_UTF8STRING *, ASN1_UTF8STRING_new, \
	struct Library *, AMISSL_BASE_NAME, 95, Amissl)

#define ASN1_UTF8STRING_free(___a) \
	AROS_LC1(void, ASN1_UTF8STRING_free, \
	AROS_LCA(ASN1_UTF8STRING *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 96, Amissl)

#define d2i_ASN1_UTF8STRING(___a, ___in, ___len) \
	AROS_LC3(ASN1_UTF8STRING *, d2i_ASN1_UTF8STRING, \
	AROS_LCA(ASN1_UTF8STRING **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 97, Amissl)

#define i2d_ASN1_UTF8STRING(___a, ___out) \
	AROS_LC2(int, i2d_ASN1_UTF8STRING, \
	AROS_LCA(ASN1_UTF8STRING *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 98, Amissl)

#define ASN1_UTF8STRING_it() \
	AROS_LC0(const ASN1_ITEM *, ASN1_UTF8STRING_it, \
	struct Library *, AMISSL_BASE_NAME, 99, Amissl)

#define ASN1_NULL_new() \
	AROS_LC0(ASN1_NULL *, ASN1_NULL_new, \
	struct Library *, AMISSL_BASE_NAME, 100, Amissl)

#define ASN1_NULL_free(___a) \
	AROS_LC1(void, ASN1_NULL_free, \
	AROS_LCA(ASN1_NULL *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 101, Amissl)

#define d2i_ASN1_NULL(___a, ___in, ___len) \
	AROS_LC3(ASN1_NULL *, d2i_ASN1_NULL, \
	AROS_LCA(ASN1_NULL **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 102, Amissl)

#define i2d_ASN1_NULL(___a, ___out) \
	AROS_LC2(int, i2d_ASN1_NULL, \
	AROS_LCA(ASN1_NULL *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 103, Amissl)

#define ASN1_NULL_it() \
	AROS_LC0(const ASN1_ITEM *, ASN1_NULL_it, \
	struct Library *, AMISSL_BASE_NAME, 104, Amissl)

#define ASN1_BMPSTRING_new() \
	AROS_LC0(ASN1_BMPSTRING *, ASN1_BMPSTRING_new, \
	struct Library *, AMISSL_BASE_NAME, 105, Amissl)

#define ASN1_BMPSTRING_free(___a) \
	AROS_LC1(void, ASN1_BMPSTRING_free, \
	AROS_LCA(ASN1_BMPSTRING *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 106, Amissl)

#define d2i_ASN1_BMPSTRING(___a, ___in, ___len) \
	AROS_LC3(ASN1_BMPSTRING *, d2i_ASN1_BMPSTRING, \
	AROS_LCA(ASN1_BMPSTRING **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 107, Amissl)

#define i2d_ASN1_BMPSTRING(___a, ___out) \
	AROS_LC2(int, i2d_ASN1_BMPSTRING, \
	AROS_LCA(ASN1_BMPSTRING *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 108, Amissl)

#define ASN1_BMPSTRING_it() \
	AROS_LC0(const ASN1_ITEM *, ASN1_BMPSTRING_it, \
	struct Library *, AMISSL_BASE_NAME, 109, Amissl)

#define UTF8_getc(___str, ___len, ___val) \
	AROS_LC3(int, UTF8_getc, \
	AROS_LCA(const unsigned char *, (___str), A0), \
	AROS_LCA(int, (___len), D0), \
	AROS_LCA(unsigned long *, (___val), A1), \
	struct Library *, AMISSL_BASE_NAME, 110, Amissl)

#define UTF8_putc(___str, ___len, ___value) \
	AROS_LC3(int, UTF8_putc, \
	AROS_LCA(unsigned char *, (___str), A0), \
	AROS_LCA(int, (___len), D0), \
	AROS_LCA(unsigned long, (___value), D1), \
	struct Library *, AMISSL_BASE_NAME, 111, Amissl)

#define ASN1_PRINTABLE_new() \
	AROS_LC0(ASN1_STRING *, ASN1_PRINTABLE_new, \
	struct Library *, AMISSL_BASE_NAME, 112, Amissl)

#define ASN1_PRINTABLE_free(___a) \
	AROS_LC1(void, ASN1_PRINTABLE_free, \
	AROS_LCA(ASN1_STRING *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 113, Amissl)

#define d2i_ASN1_PRINTABLE(___a, ___in, ___len) \
	AROS_LC3(ASN1_STRING *, d2i_ASN1_PRINTABLE, \
	AROS_LCA(ASN1_STRING **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 114, Amissl)

#define i2d_ASN1_PRINTABLE(___a, ___out) \
	AROS_LC2(int, i2d_ASN1_PRINTABLE, \
	AROS_LCA(ASN1_STRING *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 115, Amissl)

#define ASN1_PRINTABLE_it() \
	AROS_LC0(const ASN1_ITEM *, ASN1_PRINTABLE_it, \
	struct Library *, AMISSL_BASE_NAME, 116, Amissl)

#define DIRECTORYSTRING_new() \
	AROS_LC0(ASN1_STRING *, DIRECTORYSTRING_new, \
	struct Library *, AMISSL_BASE_NAME, 117, Amissl)

#define DIRECTORYSTRING_free(___a) \
	AROS_LC1(void, DIRECTORYSTRING_free, \
	AROS_LCA(ASN1_STRING *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 118, Amissl)

#define d2i_DIRECTORYSTRING(___a, ___in, ___len) \
	AROS_LC3(ASN1_STRING *, d2i_DIRECTORYSTRING, \
	AROS_LCA(ASN1_STRING **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 119, Amissl)

#define i2d_DIRECTORYSTRING(___a, ___out) \
	AROS_LC2(int, i2d_DIRECTORYSTRING, \
	AROS_LCA(ASN1_STRING *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 120, Amissl)

#define DIRECTORYSTRING_it() \
	AROS_LC0(const ASN1_ITEM *, DIRECTORYSTRING_it, \
	struct Library *, AMISSL_BASE_NAME, 121, Amissl)

#define DISPLAYTEXT_new() \
	AROS_LC0(ASN1_STRING *, DISPLAYTEXT_new, \
	struct Library *, AMISSL_BASE_NAME, 122, Amissl)

#define DISPLAYTEXT_free(___a) \
	AROS_LC1(void, DISPLAYTEXT_free, \
	AROS_LCA(ASN1_STRING *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 123, Amissl)

#define d2i_DISPLAYTEXT(___a, ___in, ___len) \
	AROS_LC3(ASN1_STRING *, d2i_DISPLAYTEXT, \
	AROS_LCA(ASN1_STRING **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 124, Amissl)

#define i2d_DISPLAYTEXT(___a, ___out) \
	AROS_LC2(int, i2d_DISPLAYTEXT, \
	AROS_LCA(ASN1_STRING *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 125, Amissl)

#define DISPLAYTEXT_it() \
	AROS_LC0(const ASN1_ITEM *, DISPLAYTEXT_it, \
	struct Library *, AMISSL_BASE_NAME, 126, Amissl)

#define ASN1_PRINTABLESTRING_new() \
	AROS_LC0(ASN1_PRINTABLESTRING *, ASN1_PRINTABLESTRING_new, \
	struct Library *, AMISSL_BASE_NAME, 127, Amissl)

#define ASN1_PRINTABLESTRING_free(___a) \
	AROS_LC1(void, ASN1_PRINTABLESTRING_free, \
	AROS_LCA(ASN1_PRINTABLESTRING *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 128, Amissl)

#define d2i_ASN1_PRINTABLESTRING(___a, ___in, ___len) \
	AROS_LC3(ASN1_PRINTABLESTRING *, d2i_ASN1_PRINTABLESTRING, \
	AROS_LCA(ASN1_PRINTABLESTRING **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 129, Amissl)

#define i2d_ASN1_PRINTABLESTRING(___a, ___out) \
	AROS_LC2(int, i2d_ASN1_PRINTABLESTRING, \
	AROS_LCA(ASN1_PRINTABLESTRING *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 130, Amissl)

#define ASN1_PRINTABLESTRING_it() \
	AROS_LC0(const ASN1_ITEM *, ASN1_PRINTABLESTRING_it, \
	struct Library *, AMISSL_BASE_NAME, 131, Amissl)

#define ASN1_T61STRING_new() \
	AROS_LC0(ASN1_T61STRING *, ASN1_T61STRING_new, \
	struct Library *, AMISSL_BASE_NAME, 132, Amissl)

#define ASN1_T61STRING_free(___a) \
	AROS_LC1(void, ASN1_T61STRING_free, \
	AROS_LCA(ASN1_T61STRING *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 133, Amissl)

#define d2i_ASN1_T61STRING(___a, ___in, ___len) \
	AROS_LC3(ASN1_T61STRING *, d2i_ASN1_T61STRING, \
	AROS_LCA(ASN1_T61STRING **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 134, Amissl)

#define i2d_ASN1_T61STRING(___a, ___out) \
	AROS_LC2(int, i2d_ASN1_T61STRING, \
	AROS_LCA(ASN1_T61STRING *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 135, Amissl)

#define ASN1_T61STRING_it() \
	AROS_LC0(const ASN1_ITEM *, ASN1_T61STRING_it, \
	struct Library *, AMISSL_BASE_NAME, 136, Amissl)

#define ASN1_IA5STRING_new() \
	AROS_LC0(ASN1_IA5STRING *, ASN1_IA5STRING_new, \
	struct Library *, AMISSL_BASE_NAME, 137, Amissl)

#define ASN1_IA5STRING_free(___a) \
	AROS_LC1(void, ASN1_IA5STRING_free, \
	AROS_LCA(ASN1_IA5STRING *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 138, Amissl)

#define d2i_ASN1_IA5STRING(___a, ___in, ___len) \
	AROS_LC3(ASN1_IA5STRING *, d2i_ASN1_IA5STRING, \
	AROS_LCA(ASN1_IA5STRING **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 139, Amissl)

#define i2d_ASN1_IA5STRING(___a, ___out) \
	AROS_LC2(int, i2d_ASN1_IA5STRING, \
	AROS_LCA(ASN1_IA5STRING *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 140, Amissl)

#define ASN1_IA5STRING_it() \
	AROS_LC0(const ASN1_ITEM *, ASN1_IA5STRING_it, \
	struct Library *, AMISSL_BASE_NAME, 141, Amissl)

#define ASN1_GENERALSTRING_new() \
	AROS_LC0(ASN1_GENERALSTRING *, ASN1_GENERALSTRING_new, \
	struct Library *, AMISSL_BASE_NAME, 142, Amissl)

#define ASN1_GENERALSTRING_free(___a) \
	AROS_LC1(void, ASN1_GENERALSTRING_free, \
	AROS_LCA(ASN1_GENERALSTRING *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 143, Amissl)

#define d2i_ASN1_GENERALSTRING(___a, ___in, ___len) \
	AROS_LC3(ASN1_GENERALSTRING *, d2i_ASN1_GENERALSTRING, \
	AROS_LCA(ASN1_GENERALSTRING **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 144, Amissl)

#define i2d_ASN1_GENERALSTRING(___a, ___out) \
	AROS_LC2(int, i2d_ASN1_GENERALSTRING, \
	AROS_LCA(ASN1_GENERALSTRING *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 145, Amissl)

#define ASN1_GENERALSTRING_it() \
	AROS_LC0(const ASN1_ITEM *, ASN1_GENERALSTRING_it, \
	struct Library *, AMISSL_BASE_NAME, 146, Amissl)

#define ASN1_UTCTIME_new() \
	AROS_LC0(ASN1_UTCTIME *, ASN1_UTCTIME_new, \
	struct Library *, AMISSL_BASE_NAME, 147, Amissl)

#define ASN1_UTCTIME_free(___a) \
	AROS_LC1(void, ASN1_UTCTIME_free, \
	AROS_LCA(ASN1_UTCTIME *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 148, Amissl)

#define d2i_ASN1_UTCTIME(___a, ___in, ___len) \
	AROS_LC3(ASN1_UTCTIME *, d2i_ASN1_UTCTIME, \
	AROS_LCA(ASN1_UTCTIME **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 149, Amissl)

#define i2d_ASN1_UTCTIME(___a, ___out) \
	AROS_LC2(int, i2d_ASN1_UTCTIME, \
	AROS_LCA(ASN1_UTCTIME *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 150, Amissl)

#define ASN1_UTCTIME_it() \
	AROS_LC0(const ASN1_ITEM *, ASN1_UTCTIME_it, \
	struct Library *, AMISSL_BASE_NAME, 151, Amissl)

#define ASN1_GENERALIZEDTIME_new() \
	AROS_LC0(ASN1_GENERALIZEDTIME *, ASN1_GENERALIZEDTIME_new, \
	struct Library *, AMISSL_BASE_NAME, 152, Amissl)

#define ASN1_GENERALIZEDTIME_free(___a) \
	AROS_LC1(void, ASN1_GENERALIZEDTIME_free, \
	AROS_LCA(ASN1_GENERALIZEDTIME *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 153, Amissl)

#define d2i_ASN1_GENERALIZEDTIME(___a, ___in, ___len) \
	AROS_LC3(ASN1_GENERALIZEDTIME *, d2i_ASN1_GENERALIZEDTIME, \
	AROS_LCA(ASN1_GENERALIZEDTIME **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 154, Amissl)

#define i2d_ASN1_GENERALIZEDTIME(___a, ___out) \
	AROS_LC2(int, i2d_ASN1_GENERALIZEDTIME, \
	AROS_LCA(ASN1_GENERALIZEDTIME *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 155, Amissl)

#define ASN1_GENERALIZEDTIME_it() \
	AROS_LC0(const ASN1_ITEM *, ASN1_GENERALIZEDTIME_it, \
	struct Library *, AMISSL_BASE_NAME, 156, Amissl)

#define ASN1_TIME_new() \
	AROS_LC0(ASN1_TIME *, ASN1_TIME_new, \
	struct Library *, AMISSL_BASE_NAME, 157, Amissl)

#define ASN1_TIME_free(___a) \
	AROS_LC1(void, ASN1_TIME_free, \
	AROS_LCA(ASN1_TIME *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 158, Amissl)

#define d2i_ASN1_TIME(___a, ___in, ___len) \
	AROS_LC3(ASN1_TIME *, d2i_ASN1_TIME, \
	AROS_LCA(ASN1_TIME **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 159, Amissl)

#define i2d_ASN1_TIME(___a, ___out) \
	AROS_LC2(int, i2d_ASN1_TIME, \
	AROS_LCA(ASN1_TIME *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 160, Amissl)

#define ASN1_TIME_it() \
	AROS_LC0(const ASN1_ITEM *, ASN1_TIME_it, \
	struct Library *, AMISSL_BASE_NAME, 161, Amissl)

#define ASN1_TIME_set(___s, ___t) \
	AROS_LC2(ASN1_TIME *, ASN1_TIME_set, \
	AROS_LCA(ASN1_TIME *, (___s), A0), \
	AROS_LCA(time_t, (___t), D0), \
	struct Library *, AMISSL_BASE_NAME, 162, Amissl)

#define ASN1_TIME_check(___t) \
	AROS_LC1(int, ASN1_TIME_check, \
	AROS_LCA(ASN1_TIME *, (___t), A0), \
	struct Library *, AMISSL_BASE_NAME, 163, Amissl)

#define ASN1_TIME_to_generalizedtime(___t, ___out) \
	AROS_LC2(ASN1_GENERALIZEDTIME *, ASN1_TIME_to_generalizedtime, \
	AROS_LCA(ASN1_TIME *, (___t), A0), \
	AROS_LCA(ASN1_GENERALIZEDTIME **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 164, Amissl)

#define i2d_ASN1_SET(___a, ___pp, ___func, ___ex_tag, ___ex_class, ___is_set) \
	AROS_LC6(int, i2d_ASN1_SET, \
	AROS_LCA(STACK *, (___a), A0), \
	AROS_LCA(unsigned char **, (___pp), A1), \
	AROS_LCA(int (*)(), (___func), A2), \
	AROS_LCA(int, (___ex_tag), D0), \
	AROS_LCA(int, (___ex_class), D1), \
	AROS_LCA(int, (___is_set), D2), \
	struct Library *, AMISSL_BASE_NAME, 165, Amissl)

#define d2i_ASN1_SET(___a, ___pp, ___length, ___func, ___free_func, ___ex_tag, ___ex_class) \
	AROS_LC7(STACK *, d2i_ASN1_SET, \
	AROS_LCA(STACK **, (___a), A0), \
	AROS_LCA(unsigned char **, (___pp), A1), \
	AROS_LCA(long, (___length), D0), \
	AROS_LCA(char * (*)(), (___func), A2), \
	AROS_LCA(void (*)(void *), (___free_func), A3), \
	AROS_LCA(int, (___ex_tag), D1), \
	AROS_LCA(int, (___ex_class), D2), \
	struct Library *, AMISSL_BASE_NAME, 166, Amissl)

#define i2a_ASN1_INTEGER(___bp, ___a) \
	AROS_LC2(int, i2a_ASN1_INTEGER, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(ASN1_INTEGER *, (___a), A1), \
	struct Library *, AMISSL_BASE_NAME, 167, Amissl)

#define a2i_ASN1_INTEGER(___bp, ___bs, ___buf, ___size) \
	AROS_LC4(int, a2i_ASN1_INTEGER, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(ASN1_INTEGER *, (___bs), A1), \
	AROS_LCA(char *, (___buf), A2), \
	AROS_LCA(int, (___size), D0), \
	struct Library *, AMISSL_BASE_NAME, 168, Amissl)

#define i2a_ASN1_ENUMERATED(___bp, ___a) \
	AROS_LC2(int, i2a_ASN1_ENUMERATED, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(ASN1_ENUMERATED *, (___a), A1), \
	struct Library *, AMISSL_BASE_NAME, 169, Amissl)

#define a2i_ASN1_ENUMERATED(___bp, ___bs, ___buf, ___size) \
	AROS_LC4(int, a2i_ASN1_ENUMERATED, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(ASN1_ENUMERATED *, (___bs), A1), \
	AROS_LCA(char *, (___buf), A2), \
	AROS_LCA(int, (___size), D0), \
	struct Library *, AMISSL_BASE_NAME, 170, Amissl)

#define i2a_ASN1_OBJECT(___bp, ___a) \
	AROS_LC2(int, i2a_ASN1_OBJECT, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(ASN1_OBJECT *, (___a), A1), \
	struct Library *, AMISSL_BASE_NAME, 171, Amissl)

#define a2i_ASN1_STRING(___bp, ___bs, ___buf, ___size) \
	AROS_LC4(int, a2i_ASN1_STRING, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(ASN1_STRING *, (___bs), A1), \
	AROS_LCA(char *, (___buf), A2), \
	AROS_LCA(int, (___size), D0), \
	struct Library *, AMISSL_BASE_NAME, 172, Amissl)

#define i2a_ASN1_STRING(___bp, ___a, ___type) \
	AROS_LC3(int, i2a_ASN1_STRING, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(ASN1_STRING *, (___a), A1), \
	AROS_LCA(int, (___type), D0), \
	struct Library *, AMISSL_BASE_NAME, 173, Amissl)

#define i2t_ASN1_OBJECT(___buf, ___buf_len, ___a) \
	AROS_LC3(int, i2t_ASN1_OBJECT, \
	AROS_LCA(char *, (___buf), A0), \
	AROS_LCA(int, (___buf_len), D0), \
	AROS_LCA(ASN1_OBJECT *, (___a), A1), \
	struct Library *, AMISSL_BASE_NAME, 174, Amissl)

#define a2d_ASN1_OBJECT(___out, ___olen, ___buf, ___num) \
	AROS_LC4(int, a2d_ASN1_OBJECT, \
	AROS_LCA(unsigned char *, (___out), A0), \
	AROS_LCA(int, (___olen), D0), \
	AROS_LCA(const char *, (___buf), A1), \
	AROS_LCA(int, (___num), D1), \
	struct Library *, AMISSL_BASE_NAME, 175, Amissl)

#define ASN1_OBJECT_create(___nid, ___data, ___len, ___sn, ___ln) \
	AROS_LC5(ASN1_OBJECT *, ASN1_OBJECT_create, \
	AROS_LCA(int, (___nid), D0), \
	AROS_LCA(unsigned char *, (___data), A0), \
	AROS_LCA(int, (___len), D1), \
	AROS_LCA(const char *, (___sn), A1), \
	AROS_LCA(const char *, (___ln), A2), \
	struct Library *, AMISSL_BASE_NAME, 176, Amissl)

#define ASN1_INTEGER_set(___a, ___v) \
	AROS_LC2(int, ASN1_INTEGER_set, \
	AROS_LCA(ASN1_INTEGER *, (___a), A0), \
	AROS_LCA(long, (___v), D0), \
	struct Library *, AMISSL_BASE_NAME, 177, Amissl)

#define ASN1_INTEGER_get(___a) \
	AROS_LC1(long, ASN1_INTEGER_get, \
	AROS_LCA(ASN1_INTEGER *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 178, Amissl)

#define BN_to_ASN1_INTEGER(___bn, ___ai) \
	AROS_LC2(ASN1_INTEGER *, BN_to_ASN1_INTEGER, \
	AROS_LCA(BIGNUM *, (___bn), A0), \
	AROS_LCA(ASN1_INTEGER *, (___ai), A1), \
	struct Library *, AMISSL_BASE_NAME, 179, Amissl)

#define ASN1_INTEGER_to_BN(___ai, ___bn) \
	AROS_LC2(BIGNUM *, ASN1_INTEGER_to_BN, \
	AROS_LCA(ASN1_INTEGER *, (___ai), A0), \
	AROS_LCA(BIGNUM *, (___bn), A1), \
	struct Library *, AMISSL_BASE_NAME, 180, Amissl)

#define ASN1_ENUMERATED_set(___a, ___v) \
	AROS_LC2(int, ASN1_ENUMERATED_set, \
	AROS_LCA(ASN1_ENUMERATED *, (___a), A0), \
	AROS_LCA(long, (___v), D0), \
	struct Library *, AMISSL_BASE_NAME, 181, Amissl)

#define ASN1_ENUMERATED_get(___a) \
	AROS_LC1(long, ASN1_ENUMERATED_get, \
	AROS_LCA(ASN1_ENUMERATED *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 182, Amissl)

#define BN_to_ASN1_ENUMERATED(___bn, ___ai) \
	AROS_LC2(ASN1_ENUMERATED *, BN_to_ASN1_ENUMERATED, \
	AROS_LCA(BIGNUM *, (___bn), A0), \
	AROS_LCA(ASN1_ENUMERATED *, (___ai), A1), \
	struct Library *, AMISSL_BASE_NAME, 183, Amissl)

#define ASN1_ENUMERATED_to_BN(___ai, ___bn) \
	AROS_LC2(BIGNUM *, ASN1_ENUMERATED_to_BN, \
	AROS_LCA(ASN1_ENUMERATED *, (___ai), A0), \
	AROS_LCA(BIGNUM *, (___bn), A1), \
	struct Library *, AMISSL_BASE_NAME, 184, Amissl)

#define ASN1_PRINTABLE_type(___s, ___max) \
	AROS_LC2(int, ASN1_PRINTABLE_type, \
	AROS_LCA(const unsigned char *, (___s), A0), \
	AROS_LCA(int, (___max), D0), \
	struct Library *, AMISSL_BASE_NAME, 185, Amissl)

#define i2d_ASN1_bytes(___a, ___pp, ___t, ___xclass) \
	AROS_LC4(int, i2d_ASN1_bytes, \
	AROS_LCA(ASN1_STRING *, (___a), A0), \
	AROS_LCA(unsigned char **, (___pp), A1), \
	AROS_LCA(int, (___t), D0), \
	AROS_LCA(int, (___xclass), D1), \
	struct Library *, AMISSL_BASE_NAME, 186, Amissl)

#define d2i_ASN1_bytes(___a, ___pp, ___length, ___Ptag, ___Pclass) \
	AROS_LC5(ASN1_STRING *, d2i_ASN1_bytes, \
	AROS_LCA(ASN1_STRING **, (___a), A0), \
	AROS_LCA(unsigned char **, (___pp), A1), \
	AROS_LCA(long, (___length), D0), \
	AROS_LCA(int, (___Ptag), D1), \
	AROS_LCA(int, (___Pclass), D2), \
	struct Library *, AMISSL_BASE_NAME, 187, Amissl)

#define ASN1_tag2bit(___t) \
	AROS_LC1(unsigned long, ASN1_tag2bit, \
	AROS_LCA(int, (___t), D0), \
	struct Library *, AMISSL_BASE_NAME, 188, Amissl)

#define d2i_ASN1_type_bytes(___a, ___pp, ___length, ___type) \
	AROS_LC4(ASN1_STRING *, d2i_ASN1_type_bytes, \
	AROS_LCA(ASN1_STRING **, (___a), A0), \
	AROS_LCA(unsigned char **, (___pp), A1), \
	AROS_LCA(long, (___length), D0), \
	AROS_LCA(int, (___type), D1), \
	struct Library *, AMISSL_BASE_NAME, 189, Amissl)

#define asn1_Finish(___c) \
	AROS_LC1(int, asn1_Finish, \
	AROS_LCA(ASN1_CTX *, (___c), A0), \
	struct Library *, AMISSL_BASE_NAME, 190, Amissl)

#define ASN1_get_object(___pp, ___plength, ___ptag, ___pclass, ___omax) \
	AROS_LC5(int, ASN1_get_object, \
	AROS_LCA(unsigned char **, (___pp), A0), \
	AROS_LCA(long *, (___plength), A1), \
	AROS_LCA(int *, (___ptag), A2), \
	AROS_LCA(int *, (___pclass), A3), \
	AROS_LCA(long, (___omax), D0), \
	struct Library *, AMISSL_BASE_NAME, 191, Amissl)

#define ASN1_check_infinite_end(___p, ___len) \
	AROS_LC2(int, ASN1_check_infinite_end, \
	AROS_LCA(unsigned char **, (___p), A0), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 192, Amissl)

#define ASN1_put_object(___pp, ___constructed, ___length, ___t, ___xclass) \
	AROS_LC5(void, ASN1_put_object, \
	AROS_LCA(unsigned char **, (___pp), A0), \
	AROS_LCA(int, (___constructed), D0), \
	AROS_LCA(int, (___length), D1), \
	AROS_LCA(int, (___t), D2), \
	AROS_LCA(int, (___xclass), D3), \
	struct Library *, AMISSL_BASE_NAME, 193, Amissl)

#define ASN1_object_size(___constructed, ___length, ___t) \
	AROS_LC3(int, ASN1_object_size, \
	AROS_LCA(int, (___constructed), D0), \
	AROS_LCA(int, (___length), D1), \
	AROS_LCA(int, (___t), D2), \
	struct Library *, AMISSL_BASE_NAME, 194, Amissl)

#define ASN1_dup(___i2d, ___d2i, ___x) \
	AROS_LC3(char *, ASN1_dup, \
	AROS_LCA(int (*)(), (___i2d), A0), \
	AROS_LCA(char * (*)(), (___d2i), A1), \
	AROS_LCA(char *, (___x), A2), \
	struct Library *, AMISSL_BASE_NAME, 195, Amissl)

#define ASN1_item_dup(___it, ___x) \
	AROS_LC2(void *, ASN1_item_dup, \
	AROS_LCA(const ASN1_ITEM *, (___it), A0), \
	AROS_LCA(void *, (___x), A1), \
	struct Library *, AMISSL_BASE_NAME, 196, Amissl)

#define ASN1_STRING_to_UTF8(___out, ___in) \
	AROS_LC2(int, ASN1_STRING_to_UTF8, \
	AROS_LCA(unsigned char **, (___out), A0), \
	AROS_LCA(ASN1_STRING *, (___in), A1), \
	struct Library *, AMISSL_BASE_NAME, 197, Amissl)

#define ASN1_d2i_bio(___xnew, ___d2i, ___bp, ___x) \
	AROS_LC4(char *, ASN1_d2i_bio, \
	AROS_LCA(char * (*)(), (___xnew), A0), \
	AROS_LCA(char * (*)(), (___d2i), A1), \
	AROS_LCA(BIO *, (___bp), A2), \
	AROS_LCA(unsigned char **, (___x), A3), \
	struct Library *, AMISSL_BASE_NAME, 198, Amissl)

#define ASN1_item_d2i_bio(___it, ___in, ___x) \
	AROS_LC3(void *, ASN1_item_d2i_bio, \
	AROS_LCA(const ASN1_ITEM *, (___it), A0), \
	AROS_LCA(BIO *, (___in), A1), \
	AROS_LCA(void *, (___x), A2), \
	struct Library *, AMISSL_BASE_NAME, 199, Amissl)

#define ASN1_i2d_bio(___i2d, ___out, ___x) \
	AROS_LC3(int, ASN1_i2d_bio, \
	AROS_LCA(int (*)(), (___i2d), A0), \
	AROS_LCA(BIO *, (___out), A1), \
	AROS_LCA(unsigned char *, (___x), A2), \
	struct Library *, AMISSL_BASE_NAME, 200, Amissl)

#define ASN1_item_i2d_bio(___it, ___out, ___x) \
	AROS_LC3(int, ASN1_item_i2d_bio, \
	AROS_LCA(const ASN1_ITEM *, (___it), A0), \
	AROS_LCA(BIO *, (___out), A1), \
	AROS_LCA(void *, (___x), A2), \
	struct Library *, AMISSL_BASE_NAME, 201, Amissl)

#define ASN1_UTCTIME_print(___fp, ___a) \
	AROS_LC2(int, ASN1_UTCTIME_print, \
	AROS_LCA(BIO *, (___fp), A0), \
	AROS_LCA(ASN1_UTCTIME *, (___a), A1), \
	struct Library *, AMISSL_BASE_NAME, 202, Amissl)

#define ASN1_GENERALIZEDTIME_print(___fp, ___a) \
	AROS_LC2(int, ASN1_GENERALIZEDTIME_print, \
	AROS_LCA(BIO *, (___fp), A0), \
	AROS_LCA(ASN1_GENERALIZEDTIME *, (___a), A1), \
	struct Library *, AMISSL_BASE_NAME, 203, Amissl)

#define ASN1_TIME_print(___fp, ___a) \
	AROS_LC2(int, ASN1_TIME_print, \
	AROS_LCA(BIO *, (___fp), A0), \
	AROS_LCA(ASN1_TIME *, (___a), A1), \
	struct Library *, AMISSL_BASE_NAME, 204, Amissl)

#define ASN1_STRING_print(___bp, ___v) \
	AROS_LC2(int, ASN1_STRING_print, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(ASN1_STRING *, (___v), A1), \
	struct Library *, AMISSL_BASE_NAME, 205, Amissl)

#define ASN1_STRING_print_ex(___out, ___str, ___flags) \
	AROS_LC3(int, ASN1_STRING_print_ex, \
	AROS_LCA(BIO *, (___out), A0), \
	AROS_LCA(ASN1_STRING *, (___str), A1), \
	AROS_LCA(unsigned long, (___flags), D0), \
	struct Library *, AMISSL_BASE_NAME, 206, Amissl)

#define ASN1_parse(___bp, ___pp, ___len, ___indent) \
	AROS_LC4(int, ASN1_parse, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(unsigned char *, (___pp), A1), \
	AROS_LCA(long, (___len), D0), \
	AROS_LCA(int, (___indent), D1), \
	struct Library *, AMISSL_BASE_NAME, 207, Amissl)

#define ASN1_parse_dump(___bp, ___pp, ___len, ___indent, ___dump) \
	AROS_LC5(int, ASN1_parse_dump, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(unsigned char *, (___pp), A1), \
	AROS_LCA(long, (___len), D0), \
	AROS_LCA(int, (___indent), D1), \
	AROS_LCA(int, (___dump), D2), \
	struct Library *, AMISSL_BASE_NAME, 208, Amissl)

#define ASN1_tag2str(___t) \
	AROS_LC1(const char *, ASN1_tag2str, \
	AROS_LCA(int, (___t), D0), \
	struct Library *, AMISSL_BASE_NAME, 209, Amissl)

#define i2d_ASN1_HEADER(___a, ___pp) \
	AROS_LC2(int, i2d_ASN1_HEADER, \
	AROS_LCA(ASN1_HEADER *, (___a), A0), \
	AROS_LCA(unsigned char **, (___pp), A1), \
	struct Library *, AMISSL_BASE_NAME, 210, Amissl)

#define d2i_ASN1_HEADER(___a, ___pp, ___length) \
	AROS_LC3(ASN1_HEADER *, d2i_ASN1_HEADER, \
	AROS_LCA(ASN1_HEADER **, (___a), A0), \
	AROS_LCA(unsigned char **, (___pp), A1), \
	AROS_LCA(long, (___length), D0), \
	struct Library *, AMISSL_BASE_NAME, 211, Amissl)

#define ASN1_HEADER_new() \
	AROS_LC0(ASN1_HEADER *, ASN1_HEADER_new, \
	struct Library *, AMISSL_BASE_NAME, 212, Amissl)

#define ASN1_HEADER_free(___a) \
	AROS_LC1(void, ASN1_HEADER_free, \
	AROS_LCA(ASN1_HEADER *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 213, Amissl)

#define ASN1_UNIVERSALSTRING_to_string(___s) \
	AROS_LC1(int, ASN1_UNIVERSALSTRING_to_string, \
	AROS_LCA(ASN1_UNIVERSALSTRING *, (___s), A0), \
	struct Library *, AMISSL_BASE_NAME, 214, Amissl)

#define X509_asn1_meth() \
	AROS_LC0(ASN1_METHOD *, X509_asn1_meth, \
	struct Library *, AMISSL_BASE_NAME, 215, Amissl)

#define RSAPrivateKey_asn1_meth() \
	AROS_LC0(ASN1_METHOD *, RSAPrivateKey_asn1_meth, \
	struct Library *, AMISSL_BASE_NAME, 216, Amissl)

#define ASN1_IA5STRING_asn1_meth() \
	AROS_LC0(ASN1_METHOD *, ASN1_IA5STRING_asn1_meth, \
	struct Library *, AMISSL_BASE_NAME, 217, Amissl)

#define ASN1_BIT_STRING_asn1_meth() \
	AROS_LC0(ASN1_METHOD *, ASN1_BIT_STRING_asn1_meth, \
	struct Library *, AMISSL_BASE_NAME, 218, Amissl)

#define ASN1_TYPE_set_octetstring(___a, ___data, ___len) \
	AROS_LC3(int, ASN1_TYPE_set_octetstring, \
	AROS_LCA(ASN1_TYPE *, (___a), A0), \
	AROS_LCA(unsigned char *, (___data), A1), \
	AROS_LCA(int, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 219, Amissl)

#define ASN1_TYPE_get_octetstring(___a, ___data, ___max_len) \
	AROS_LC3(int, ASN1_TYPE_get_octetstring, \
	AROS_LCA(ASN1_TYPE *, (___a), A0), \
	AROS_LCA(unsigned char *, (___data), A1), \
	AROS_LCA(int, (___max_len), D0), \
	struct Library *, AMISSL_BASE_NAME, 220, Amissl)

#define ASN1_TYPE_set_int_octetstring(___a, ___num, ___data, ___len) \
	AROS_LC4(int, ASN1_TYPE_set_int_octetstring, \
	AROS_LCA(ASN1_TYPE *, (___a), A0), \
	AROS_LCA(long, (___num), D0), \
	AROS_LCA(unsigned char *, (___data), A1), \
	AROS_LCA(int, (___len), D1), \
	struct Library *, AMISSL_BASE_NAME, 221, Amissl)

#define ASN1_TYPE_get_int_octetstring(___a, ___num, ___data, ___max_len) \
	AROS_LC4(int, ASN1_TYPE_get_int_octetstring, \
	AROS_LCA(ASN1_TYPE *, (___a), A0), \
	AROS_LCA(long *, (___num), A1), \
	AROS_LCA(unsigned char *, (___data), A2), \
	AROS_LCA(int, (___max_len), D0), \
	struct Library *, AMISSL_BASE_NAME, 222, Amissl)

#define ASN1_seq_unpack(___buf, ___len, ___d2i, ___free_func) \
	AROS_LC4(STACK *, ASN1_seq_unpack, \
	AROS_LCA(unsigned char *, (___buf), A0), \
	AROS_LCA(int, (___len), D0), \
	AROS_LCA(char * (*)(), (___d2i), A1), \
	AROS_LCA(void (*)(void *), (___free_func), A2), \
	struct Library *, AMISSL_BASE_NAME, 223, Amissl)

#define ASN1_seq_pack(___safes, ___i2d, ___buf, ___len) \
	AROS_LC4(unsigned char *, ASN1_seq_pack, \
	AROS_LCA(STACK *, (___safes), A0), \
	AROS_LCA(int (*)(), (___i2d), A1), \
	AROS_LCA(unsigned char **, (___buf), A2), \
	AROS_LCA(int *, (___len), A3), \
	struct Library *, AMISSL_BASE_NAME, 224, Amissl)

#define ASN1_unpack_string(___oct, ___d2i) \
	AROS_LC2(void *, ASN1_unpack_string, \
	AROS_LCA(ASN1_STRING *, (___oct), A0), \
	AROS_LCA(char * (*)(), (___d2i), A1), \
	struct Library *, AMISSL_BASE_NAME, 225, Amissl)

#define ASN1_item_unpack(___oct, ___it) \
	AROS_LC2(void *, ASN1_item_unpack, \
	AROS_LCA(ASN1_STRING *, (___oct), A0), \
	AROS_LCA(const ASN1_ITEM *, (___it), A1), \
	struct Library *, AMISSL_BASE_NAME, 226, Amissl)

#define ASN1_pack_string(___obj, ___i2d, ___oct) \
	AROS_LC3(ASN1_STRING *, ASN1_pack_string, \
	AROS_LCA(void *, (___obj), A0), \
	AROS_LCA(int (*)(), (___i2d), A1), \
	AROS_LCA(ASN1_OCTET_STRING **, (___oct), A2), \
	struct Library *, AMISSL_BASE_NAME, 227, Amissl)

#define ASN1_item_pack(___obj, ___it, ___oct) \
	AROS_LC3(ASN1_STRING *, ASN1_item_pack, \
	AROS_LCA(void *, (___obj), A0), \
	AROS_LCA(const ASN1_ITEM *, (___it), A1), \
	AROS_LCA(ASN1_OCTET_STRING **, (___oct), A2), \
	struct Library *, AMISSL_BASE_NAME, 228, Amissl)

#define ASN1_STRING_set_default_mask(___mask) \
	AROS_LC1(void, ASN1_STRING_set_default_mask, \
	AROS_LCA(unsigned long, (___mask), D0), \
	struct Library *, AMISSL_BASE_NAME, 229, Amissl)

#define ASN1_STRING_set_default_mask_asc(___p) \
	AROS_LC1(int, ASN1_STRING_set_default_mask_asc, \
	AROS_LCA(char *, (___p), A0), \
	struct Library *, AMISSL_BASE_NAME, 230, Amissl)

#define ASN1_STRING_get_default_mask() \
	AROS_LC0(unsigned long, ASN1_STRING_get_default_mask, \
	struct Library *, AMISSL_BASE_NAME, 231, Amissl)

#define ASN1_mbstring_copy(___out, ___in, ___len, ___inform, ___mask) \
	AROS_LC5(int, ASN1_mbstring_copy, \
	AROS_LCA(ASN1_STRING **, (___out), A0), \
	AROS_LCA(const unsigned char *, (___in), A1), \
	AROS_LCA(int, (___len), D0), \
	AROS_LCA(int, (___inform), D1), \
	AROS_LCA(unsigned long, (___mask), D2), \
	struct Library *, AMISSL_BASE_NAME, 232, Amissl)

#define ASN1_mbstring_ncopy(___out, ___in, ___len, ___inform, ___mask, ___minsize, ___maxsize) \
	AROS_LC7(int, ASN1_mbstring_ncopy, \
	AROS_LCA(ASN1_STRING **, (___out), A0), \
	AROS_LCA(const unsigned char *, (___in), A1), \
	AROS_LCA(int, (___len), D0), \
	AROS_LCA(int, (___inform), D1), \
	AROS_LCA(unsigned long, (___mask), D2), \
	AROS_LCA(long, (___minsize), D3), \
	AROS_LCA(long, (___maxsize), D4), \
	struct Library *, AMISSL_BASE_NAME, 233, Amissl)

#define ASN1_STRING_set_by_NID(___out, ___in, ___inlen, ___inform, ___nid) \
	AROS_LC5(ASN1_STRING *, ASN1_STRING_set_by_NID, \
	AROS_LCA(ASN1_STRING **, (___out), A0), \
	AROS_LCA(const unsigned char *, (___in), A1), \
	AROS_LCA(int, (___inlen), D0), \
	AROS_LCA(int, (___inform), D1), \
	AROS_LCA(int, (___nid), D2), \
	struct Library *, AMISSL_BASE_NAME, 234, Amissl)

#define ASN1_STRING_TABLE_get(___nid) \
	AROS_LC1(ASN1_STRING_TABLE *, ASN1_STRING_TABLE_get, \
	AROS_LCA(int, (___nid), D0), \
	struct Library *, AMISSL_BASE_NAME, 235, Amissl)

#define ASN1_STRING_TABLE_add(___a, ___b, ___c, ___d, ___e) \
	AROS_LC5(int, ASN1_STRING_TABLE_add, \
	AROS_LCA(int, (___a), D0), \
	AROS_LCA(long, (___b), D1), \
	AROS_LCA(long, (___c), D2), \
	AROS_LCA(unsigned long, (___d), D3), \
	AROS_LCA(unsigned long, (___e), D4), \
	struct Library *, AMISSL_BASE_NAME, 236, Amissl)

#define ASN1_STRING_TABLE_cleanup() \
	AROS_LC0(void, ASN1_STRING_TABLE_cleanup, \
	struct Library *, AMISSL_BASE_NAME, 237, Amissl)

#define ASN1_item_new(___it) \
	AROS_LC1(ASN1_VALUE *, ASN1_item_new, \
	AROS_LCA(const ASN1_ITEM *, (___it), A0), \
	struct Library *, AMISSL_BASE_NAME, 238, Amissl)

#define ASN1_item_free(___val, ___it) \
	AROS_LC2(void, ASN1_item_free, \
	AROS_LCA(ASN1_VALUE *, (___val), A0), \
	AROS_LCA(const ASN1_ITEM *, (___it), A1), \
	struct Library *, AMISSL_BASE_NAME, 239, Amissl)

#define ASN1_item_d2i(___val, ___in, ___len, ___it) \
	AROS_LC4(ASN1_VALUE *, ASN1_item_d2i, \
	AROS_LCA(ASN1_VALUE **, (___val), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	AROS_LCA(const ASN1_ITEM *, (___it), A2), \
	struct Library *, AMISSL_BASE_NAME, 240, Amissl)

#define ASN1_item_i2d(___val, ___out, ___it) \
	AROS_LC3(int, ASN1_item_i2d, \
	AROS_LCA(ASN1_VALUE *, (___val), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	AROS_LCA(const ASN1_ITEM *, (___it), A2), \
	struct Library *, AMISSL_BASE_NAME, 241, Amissl)

#define ASN1_add_oid_module() \
	AROS_LC0(void, ASN1_add_oid_module, \
	struct Library *, AMISSL_BASE_NAME, 242, Amissl)

#define ERR_load_ASN1_strings() \
	AROS_LC0(void, ERR_load_ASN1_strings, \
	struct Library *, AMISSL_BASE_NAME, 243, Amissl)

#define asn1_GetSequence(___c, ___length) \
	AROS_LC2(int, asn1_GetSequence, \
	AROS_LCA(ASN1_CTX *, (___c), A0), \
	AROS_LCA(long *, (___length), A1), \
	struct Library *, AMISSL_BASE_NAME, 244, Amissl)

#define asn1_add_error(___address, ___offset) \
	AROS_LC2(void, asn1_add_error, \
	AROS_LCA(unsigned char *, (___address), A0), \
	AROS_LCA(int, (___offset), D0), \
	struct Library *, AMISSL_BASE_NAME, 245, Amissl)

#define ASN1_BOOLEAN_it() \
	AROS_LC0(const ASN1_ITEM *, ASN1_BOOLEAN_it, \
	struct Library *, AMISSL_BASE_NAME, 246, Amissl)

#define ASN1_TBOOLEAN_it() \
	AROS_LC0(const ASN1_ITEM *, ASN1_TBOOLEAN_it, \
	struct Library *, AMISSL_BASE_NAME, 247, Amissl)

#define ASN1_FBOOLEAN_it() \
	AROS_LC0(const ASN1_ITEM *, ASN1_FBOOLEAN_it, \
	struct Library *, AMISSL_BASE_NAME, 248, Amissl)

#define ASN1_SEQUENCE_it() \
	AROS_LC0(const ASN1_ITEM *, ASN1_SEQUENCE_it, \
	struct Library *, AMISSL_BASE_NAME, 249, Amissl)

#define CBIGNUM_it() \
	AROS_LC0(const ASN1_ITEM *, CBIGNUM_it, \
	struct Library *, AMISSL_BASE_NAME, 250, Amissl)

#define BIGNUM_it() \
	AROS_LC0(const ASN1_ITEM *, BIGNUM_it, \
	struct Library *, AMISSL_BASE_NAME, 251, Amissl)

#define LONG_it() \
	AROS_LC0(const ASN1_ITEM *, LONG_it, \
	struct Library *, AMISSL_BASE_NAME, 252, Amissl)

#define ZLONG_it() \
	AROS_LC0(const ASN1_ITEM *, ZLONG_it, \
	struct Library *, AMISSL_BASE_NAME, 253, Amissl)

#define ASN1_item_ex_new(___pval, ___it) \
	AROS_LC2(int, ASN1_item_ex_new, \
	AROS_LCA(ASN1_VALUE **, (___pval), A0), \
	AROS_LCA(const ASN1_ITEM *, (___it), A1), \
	struct Library *, AMISSL_BASE_NAME, 254, Amissl)

#define ASN1_item_ex_free(___pval, ___it) \
	AROS_LC2(void, ASN1_item_ex_free, \
	AROS_LCA(ASN1_VALUE **, (___pval), A0), \
	AROS_LCA(const ASN1_ITEM *, (___it), A1), \
	struct Library *, AMISSL_BASE_NAME, 255, Amissl)

#define ASN1_template_new(___pval, ___tt) \
	AROS_LC2(int, ASN1_template_new, \
	AROS_LCA(ASN1_VALUE **, (___pval), A0), \
	AROS_LCA(const ASN1_TEMPLATE *, (___tt), A1), \
	struct Library *, AMISSL_BASE_NAME, 256, Amissl)

#define ASN1_primitive_new(___pval, ___it) \
	AROS_LC2(int, ASN1_primitive_new, \
	AROS_LCA(ASN1_VALUE **, (___pval), A0), \
	AROS_LCA(const ASN1_ITEM *, (___it), A1), \
	struct Library *, AMISSL_BASE_NAME, 257, Amissl)

#define ASN1_template_free(___pval, ___tt) \
	AROS_LC2(void, ASN1_template_free, \
	AROS_LCA(ASN1_VALUE **, (___pval), A0), \
	AROS_LCA(const ASN1_TEMPLATE *, (___tt), A1), \
	struct Library *, AMISSL_BASE_NAME, 258, Amissl)

#define ASN1_template_d2i(___pval, ___in, ___len, ___tt) \
	AROS_LC4(int, ASN1_template_d2i, \
	AROS_LCA(ASN1_VALUE **, (___pval), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	AROS_LCA(const ASN1_TEMPLATE *, (___tt), A2), \
	struct Library *, AMISSL_BASE_NAME, 259, Amissl)

#define ASN1_item_ex_d2i(___pval, ___in, ___len, ___it, ___t, ___aclass, ___opt, ___ctx) \
	AROS_LC8(int, ASN1_item_ex_d2i, \
	AROS_LCA(ASN1_VALUE **, (___pval), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	AROS_LCA(const ASN1_ITEM *, (___it), A2), \
	AROS_LCA(int, (___t), D1), \
	AROS_LCA(int, (___aclass), D2), \
	AROS_LCA(char, (___opt), D3), \
	AROS_LCA(ASN1_TLC *, (___ctx), A3), \
	struct Library *, AMISSL_BASE_NAME, 260, Amissl)

#define ASN1_item_ex_i2d(___pval, ___out, ___it, ___t, ___aclass) \
	AROS_LC5(int, ASN1_item_ex_i2d, \
	AROS_LCA(ASN1_VALUE **, (___pval), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	AROS_LCA(const ASN1_ITEM *, (___it), A2), \
	AROS_LCA(int, (___t), D0), \
	AROS_LCA(int, (___aclass), D1), \
	struct Library *, AMISSL_BASE_NAME, 261, Amissl)

#define ASN1_template_i2d(___pval, ___out, ___tt) \
	AROS_LC3(int, ASN1_template_i2d, \
	AROS_LCA(ASN1_VALUE **, (___pval), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	AROS_LCA(const ASN1_TEMPLATE *, (___tt), A2), \
	struct Library *, AMISSL_BASE_NAME, 262, Amissl)

#define ASN1_primitive_free(___pval, ___it) \
	AROS_LC2(void, ASN1_primitive_free, \
	AROS_LCA(ASN1_VALUE **, (___pval), A0), \
	AROS_LCA(const ASN1_ITEM *, (___it), A1), \
	struct Library *, AMISSL_BASE_NAME, 263, Amissl)

#define asn1_ex_i2c(___pval, ___cont, ___putype, ___it) \
	AROS_LC4(int, asn1_ex_i2c, \
	AROS_LCA(ASN1_VALUE **, (___pval), A0), \
	AROS_LCA(unsigned char *, (___cont), A1), \
	AROS_LCA(int *, (___putype), A2), \
	AROS_LCA(const ASN1_ITEM *, (___it), A3), \
	struct Library *, AMISSL_BASE_NAME, 264, Amissl)

#define asn1_ex_c2i(___pval, ___cont, ___len, ___utype, ___free_cont, ___it) \
	AROS_LC6(int, asn1_ex_c2i, \
	AROS_LCA(ASN1_VALUE **, (___pval), A0), \
	AROS_LCA(unsigned char *, (___cont), A1), \
	AROS_LCA(int, (___len), D0), \
	AROS_LCA(int, (___utype), D1), \
	AROS_LCA(char *, (___free_cont), A2), \
	AROS_LCA(const ASN1_ITEM *, (___it), A3), \
	struct Library *, AMISSL_BASE_NAME, 265, Amissl)

#define asn1_get_choice_selector(___pval, ___it) \
	AROS_LC2(int, asn1_get_choice_selector, \
	AROS_LCA(ASN1_VALUE **, (___pval), A0), \
	AROS_LCA(const ASN1_ITEM *, (___it), A1), \
	struct Library *, AMISSL_BASE_NAME, 266, Amissl)

#define asn1_set_choice_selector(___pval, ___value, ___it) \
	AROS_LC3(int, asn1_set_choice_selector, \
	AROS_LCA(ASN1_VALUE **, (___pval), A0), \
	AROS_LCA(int, (___value), D0), \
	AROS_LCA(const ASN1_ITEM *, (___it), A1), \
	struct Library *, AMISSL_BASE_NAME, 267, Amissl)

#define asn1_get_field_ptr(___pval, ___tt) \
	AROS_LC2(ASN1_VALUE **, asn1_get_field_ptr, \
	AROS_LCA(ASN1_VALUE **, (___pval), A0), \
	AROS_LCA(const ASN1_TEMPLATE *, (___tt), A1), \
	struct Library *, AMISSL_BASE_NAME, 268, Amissl)

#define asn1_do_adb(___pval, ___tt, ___nullerr) \
	AROS_LC3(const ASN1_TEMPLATE *, asn1_do_adb, \
	AROS_LCA(ASN1_VALUE **, (___pval), A0), \
	AROS_LCA(const ASN1_TEMPLATE *, (___tt), A1), \
	AROS_LCA(int, (___nullerr), D0), \
	struct Library *, AMISSL_BASE_NAME, 269, Amissl)

#define asn1_do_lock(___pval, ___op, ___it) \
	AROS_LC3(int, asn1_do_lock, \
	AROS_LCA(ASN1_VALUE **, (___pval), A0), \
	AROS_LCA(int, (___op), D0), \
	AROS_LCA(const ASN1_ITEM *, (___it), A1), \
	struct Library *, AMISSL_BASE_NAME, 270, Amissl)

#define asn1_enc_init(___pval, ___it) \
	AROS_LC2(void, asn1_enc_init, \
	AROS_LCA(ASN1_VALUE **, (___pval), A0), \
	AROS_LCA(const ASN1_ITEM *, (___it), A1), \
	struct Library *, AMISSL_BASE_NAME, 271, Amissl)

#define asn1_enc_free(___pval, ___it) \
	AROS_LC2(void, asn1_enc_free, \
	AROS_LCA(ASN1_VALUE **, (___pval), A0), \
	AROS_LCA(const ASN1_ITEM *, (___it), A1), \
	struct Library *, AMISSL_BASE_NAME, 272, Amissl)

#define asn1_enc_restore(___len, ___out, ___pval, ___it) \
	AROS_LC4(int, asn1_enc_restore, \
	AROS_LCA(int *, (___len), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	AROS_LCA(ASN1_VALUE **, (___pval), A2), \
	AROS_LCA(const ASN1_ITEM *, (___it), A3), \
	struct Library *, AMISSL_BASE_NAME, 273, Amissl)

#define asn1_enc_save(___pval, ___in, ___inlen, ___it) \
	AROS_LC4(int, asn1_enc_save, \
	AROS_LCA(ASN1_VALUE **, (___pval), A0), \
	AROS_LCA(unsigned char *, (___in), A1), \
	AROS_LCA(int, (___inlen), D0), \
	AROS_LCA(const ASN1_ITEM *, (___it), A2), \
	struct Library *, AMISSL_BASE_NAME, 274, Amissl)

#define BIO_ctrl_pending(___b) \
	AROS_LC1(size_t, BIO_ctrl_pending, \
	AROS_LCA(BIO *, (___b), A0), \
	struct Library *, AMISSL_BASE_NAME, 275, Amissl)

#define BIO_ctrl_wpending(___b) \
	AROS_LC1(size_t, BIO_ctrl_wpending, \
	AROS_LCA(BIO *, (___b), A0), \
	struct Library *, AMISSL_BASE_NAME, 276, Amissl)

#define BIO_ctrl_get_write_guarantee(___b) \
	AROS_LC1(size_t, BIO_ctrl_get_write_guarantee, \
	AROS_LCA(BIO *, (___b), A0), \
	struct Library *, AMISSL_BASE_NAME, 277, Amissl)

#define BIO_ctrl_get_read_request(___b) \
	AROS_LC1(size_t, BIO_ctrl_get_read_request, \
	AROS_LCA(BIO *, (___b), A0), \
	struct Library *, AMISSL_BASE_NAME, 278, Amissl)

#define BIO_ctrl_reset_read_request(___b) \
	AROS_LC1(int, BIO_ctrl_reset_read_request, \
	AROS_LCA(BIO *, (___b), A0), \
	struct Library *, AMISSL_BASE_NAME, 279, Amissl)

#define BIO_set_ex_data(___bio, ___idx, ___data) \
	AROS_LC3(int, BIO_set_ex_data, \
	AROS_LCA(BIO *, (___bio), A0), \
	AROS_LCA(int, (___idx), D0), \
	AROS_LCA(void *, (___data), A1), \
	struct Library *, AMISSL_BASE_NAME, 280, Amissl)

#define BIO_get_ex_data(___bio, ___idx) \
	AROS_LC2(void *, BIO_get_ex_data, \
	AROS_LCA(BIO *, (___bio), A0), \
	AROS_LCA(int, (___idx), D0), \
	struct Library *, AMISSL_BASE_NAME, 281, Amissl)

#define BIO_get_ex_new_index(___argl, ___argp, ___new_func, ___dup_func, ___free_func) \
	AROS_LC5(int, BIO_get_ex_new_index, \
	AROS_LCA(long, (___argl), D0), \
	AROS_LCA(void *, (___argp), A0), \
	AROS_LCA(CRYPTO_EX_new * (*)(), (___new_func), A1), \
	AROS_LCA(CRYPTO_EX_dup * (*)(), (___dup_func), A2), \
	AROS_LCA(CRYPTO_EX_free * (*)(), (___free_func), A3), \
	struct Library *, AMISSL_BASE_NAME, 282, Amissl)

#define BIO_number_read(___bio) \
	AROS_LC1(unsigned long, BIO_number_read, \
	AROS_LCA(BIO *, (___bio), A0), \
	struct Library *, AMISSL_BASE_NAME, 283, Amissl)

#define BIO_number_written(___bio) \
	AROS_LC1(unsigned long, BIO_number_written, \
	AROS_LCA(BIO *, (___bio), A0), \
	struct Library *, AMISSL_BASE_NAME, 284, Amissl)

#define BIO_s_file() \
	AROS_LC0(BIO_METHOD *, BIO_s_file, \
	struct Library *, AMISSL_BASE_NAME, 285, Amissl)

#define BIO_new_file(___filename, ___mode) \
	AROS_LC2(BIO *, BIO_new_file, \
	AROS_LCA(const char *, (___filename), A0), \
	AROS_LCA(const char *, (___mode), A1), \
	struct Library *, AMISSL_BASE_NAME, 286, Amissl)

#define BIO_new_fp_amiga(___stream, ___close_flag) \
	AROS_LC2(BIO *, BIO_new_fp_amiga, \
	AROS_LCA(BPTR, (___stream), D0), \
	AROS_LCA(int, (___close_flag), D1), \
	struct Library *, AMISSL_BASE_NAME, 287, Amissl)

#define BIO_new(___type) \
	AROS_LC1(BIO *, BIO_new, \
	AROS_LCA(BIO_METHOD *, (___type), A0), \
	struct Library *, AMISSL_BASE_NAME, 288, Amissl)

#define BIO_set(___a, ___type) \
	AROS_LC2(int, BIO_set, \
	AROS_LCA(BIO *, (___a), A0), \
	AROS_LCA(BIO_METHOD *, (___type), A1), \
	struct Library *, AMISSL_BASE_NAME, 289, Amissl)

#define BIO_free(___a) \
	AROS_LC1(int, BIO_free, \
	AROS_LCA(BIO *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 290, Amissl)

#define BIO_vfree(___a) \
	AROS_LC1(void, BIO_vfree, \
	AROS_LCA(BIO *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 291, Amissl)

#define BIO_read(___b, ___data, ___len) \
	AROS_LC3(int, BIO_read, \
	AROS_LCA(BIO *, (___b), A0), \
	AROS_LCA(void *, (___data), A1), \
	AROS_LCA(int, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 292, Amissl)

#define BIO_gets(___bp, ___buf, ___size) \
	AROS_LC3(int, BIO_gets, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(char *, (___buf), A1), \
	AROS_LCA(int, (___size), D0), \
	struct Library *, AMISSL_BASE_NAME, 293, Amissl)

#define BIO_write(___b, ___data, ___len) \
	AROS_LC3(int, BIO_write, \
	AROS_LCA(BIO *, (___b), A0), \
	AROS_LCA(const void *, (___data), A1), \
	AROS_LCA(int, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 294, Amissl)

#define BIO_puts(___bp, ___buf) \
	AROS_LC2(int, BIO_puts, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(const char *, (___buf), A1), \
	struct Library *, AMISSL_BASE_NAME, 295, Amissl)

#define BIO_indent(___b, ___indent, ___max) \
	AROS_LC3(int, BIO_indent, \
	AROS_LCA(BIO *, (___b), A0), \
	AROS_LCA(int, (___indent), D0), \
	AROS_LCA(int, (___max), D1), \
	struct Library *, AMISSL_BASE_NAME, 296, Amissl)

#define BIO_ctrl(___bp, ___cmd, ___larg, ___parg) \
	AROS_LC4(long, BIO_ctrl, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(int, (___cmd), D0), \
	AROS_LCA(long, (___larg), D1), \
	AROS_LCA(void *, (___parg), A1), \
	struct Library *, AMISSL_BASE_NAME, 297, Amissl)

#define BIO_callback_ctrl(___b, ___cmd, ___fp) \
	AROS_LC3(long, BIO_callback_ctrl, \
	AROS_LCA(BIO *, (___b), A0), \
	AROS_LCA(int, (___cmd), D0), \
	AROS_LCA(void (*)(struct bio_st *,int,const char *,int,long,long), (___fp), A1), \
	struct Library *, AMISSL_BASE_NAME, 298, Amissl)

#define BIO_ptr_ctrl(___bp, ___cmd, ___larg) \
	AROS_LC3(char *, BIO_ptr_ctrl, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(int, (___cmd), D0), \
	AROS_LCA(long, (___larg), D1), \
	struct Library *, AMISSL_BASE_NAME, 299, Amissl)

#define BIO_int_ctrl(___bp, ___cmd, ___larg, ___iarg) \
	AROS_LC4(long, BIO_int_ctrl, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(int, (___cmd), D0), \
	AROS_LCA(long, (___larg), D1), \
	AROS_LCA(int, (___iarg), D2), \
	struct Library *, AMISSL_BASE_NAME, 300, Amissl)

#define BIO_push(___b, ___append) \
	AROS_LC2(BIO *, BIO_push, \
	AROS_LCA(BIO *, (___b), A0), \
	AROS_LCA(BIO *, (___append), A1), \
	struct Library *, AMISSL_BASE_NAME, 301, Amissl)

#define BIO_pop(___b) \
	AROS_LC1(BIO *, BIO_pop, \
	AROS_LCA(BIO *, (___b), A0), \
	struct Library *, AMISSL_BASE_NAME, 302, Amissl)

#define BIO_free_all(___a) \
	AROS_LC1(void, BIO_free_all, \
	AROS_LCA(BIO *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 303, Amissl)

#define BIO_find_type(___b, ___bio_type) \
	AROS_LC2(BIO *, BIO_find_type, \
	AROS_LCA(BIO *, (___b), A0), \
	AROS_LCA(int, (___bio_type), D0), \
	struct Library *, AMISSL_BASE_NAME, 304, Amissl)

#define BIO_next(___b) \
	AROS_LC1(BIO *, BIO_next, \
	AROS_LCA(BIO *, (___b), A0), \
	struct Library *, AMISSL_BASE_NAME, 305, Amissl)

#define BIO_get_retry_BIO(___bio, ___reason) \
	AROS_LC2(BIO *, BIO_get_retry_BIO, \
	AROS_LCA(BIO *, (___bio), A0), \
	AROS_LCA(int *, (___reason), A1), \
	struct Library *, AMISSL_BASE_NAME, 306, Amissl)

#define BIO_get_retry_reason(___bio) \
	AROS_LC1(int, BIO_get_retry_reason, \
	AROS_LCA(BIO *, (___bio), A0), \
	struct Library *, AMISSL_BASE_NAME, 307, Amissl)

#define BIO_dup_chain(___in) \
	AROS_LC1(BIO *, BIO_dup_chain, \
	AROS_LCA(BIO *, (___in), A0), \
	struct Library *, AMISSL_BASE_NAME, 308, Amissl)

#define BIO_nread0(___bio, ___buf) \
	AROS_LC2(int, BIO_nread0, \
	AROS_LCA(BIO *, (___bio), A0), \
	AROS_LCA(char **, (___buf), A1), \
	struct Library *, AMISSL_BASE_NAME, 309, Amissl)

#define BIO_nread(___bio, ___buf, ___num) \
	AROS_LC3(int, BIO_nread, \
	AROS_LCA(BIO *, (___bio), A0), \
	AROS_LCA(char **, (___buf), A1), \
	AROS_LCA(int, (___num), D0), \
	struct Library *, AMISSL_BASE_NAME, 310, Amissl)

#define BIO_nwrite0(___bio, ___buf) \
	AROS_LC2(int, BIO_nwrite0, \
	AROS_LCA(BIO *, (___bio), A0), \
	AROS_LCA(char **, (___buf), A1), \
	struct Library *, AMISSL_BASE_NAME, 311, Amissl)

#define BIO_nwrite(___bio, ___buf, ___num) \
	AROS_LC3(int, BIO_nwrite, \
	AROS_LCA(BIO *, (___bio), A0), \
	AROS_LCA(char **, (___buf), A1), \
	AROS_LCA(int, (___num), D0), \
	struct Library *, AMISSL_BASE_NAME, 312, Amissl)

#define BIO_debug_callback(___bio, ___cmd, ___argp, ___argi, ___argl, ___ret) \
	AROS_LC6(long, BIO_debug_callback, \
	AROS_LCA(BIO *, (___bio), A0), \
	AROS_LCA(int, (___cmd), D0), \
	AROS_LCA(const char *, (___argp), A1), \
	AROS_LCA(int, (___argi), D1), \
	AROS_LCA(long, (___argl), D2), \
	AROS_LCA(long, (___ret), D3), \
	struct Library *, AMISSL_BASE_NAME, 313, Amissl)

#define BIO_s_mem() \
	AROS_LC0(BIO_METHOD *, BIO_s_mem, \
	struct Library *, AMISSL_BASE_NAME, 314, Amissl)

#define BIO_new_mem_buf(___buf, ___len) \
	AROS_LC2(BIO *, BIO_new_mem_buf, \
	AROS_LCA(void *, (___buf), A0), \
	AROS_LCA(int, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 315, Amissl)

#define BIO_s_socket() \
	AROS_LC0(BIO_METHOD *, BIO_s_socket, \
	struct Library *, AMISSL_BASE_NAME, 316, Amissl)

#define BIO_s_connect() \
	AROS_LC0(BIO_METHOD *, BIO_s_connect, \
	struct Library *, AMISSL_BASE_NAME, 317, Amissl)

#define BIO_s_accept() \
	AROS_LC0(BIO_METHOD *, BIO_s_accept, \
	struct Library *, AMISSL_BASE_NAME, 318, Amissl)

#define BIO_s_fd() \
	AROS_LC0(BIO_METHOD *, BIO_s_fd, \
	struct Library *, AMISSL_BASE_NAME, 319, Amissl)

#define BIO_s_log() \
	AROS_LC0(BIO_METHOD *, BIO_s_log, \
	struct Library *, AMISSL_BASE_NAME, 320, Amissl)

#define BIO_s_bio() \
	AROS_LC0(BIO_METHOD *, BIO_s_bio, \
	struct Library *, AMISSL_BASE_NAME, 321, Amissl)

#define BIO_s_null() \
	AROS_LC0(BIO_METHOD *, BIO_s_null, \
	struct Library *, AMISSL_BASE_NAME, 322, Amissl)

#define BIO_f_null() \
	AROS_LC0(BIO_METHOD *, BIO_f_null, \
	struct Library *, AMISSL_BASE_NAME, 323, Amissl)

#define BIO_f_buffer() \
	AROS_LC0(BIO_METHOD *, BIO_f_buffer, \
	struct Library *, AMISSL_BASE_NAME, 324, Amissl)

#define BIO_f_nbio_test() \
	AROS_LC0(BIO_METHOD *, BIO_f_nbio_test, \
	struct Library *, AMISSL_BASE_NAME, 325, Amissl)

#define BIO_sock_should_retry(___i) \
	AROS_LC1(int, BIO_sock_should_retry, \
	AROS_LCA(int, (___i), D0), \
	struct Library *, AMISSL_BASE_NAME, 326, Amissl)

#define BIO_sock_non_fatal_error(___error) \
	AROS_LC1(int, BIO_sock_non_fatal_error, \
	AROS_LCA(int, (___error), D0), \
	struct Library *, AMISSL_BASE_NAME, 327, Amissl)

#define BIO_fd_should_retry(___i) \
	AROS_LC1(int, BIO_fd_should_retry, \
	AROS_LCA(int, (___i), D0), \
	struct Library *, AMISSL_BASE_NAME, 328, Amissl)

#define BIO_fd_non_fatal_error(___error) \
	AROS_LC1(int, BIO_fd_non_fatal_error, \
	AROS_LCA(int, (___error), D0), \
	struct Library *, AMISSL_BASE_NAME, 329, Amissl)

#define BIO_dump(___b, ___bytes, ___len) \
	AROS_LC3(int, BIO_dump, \
	AROS_LCA(BIO *, (___b), A0), \
	AROS_LCA(const char *, (___bytes), A1), \
	AROS_LCA(int, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 330, Amissl)

#define BIO_dump_indent(___b, ___bytes, ___len, ___indent) \
	AROS_LC4(int, BIO_dump_indent, \
	AROS_LCA(BIO *, (___b), A0), \
	AROS_LCA(const char *, (___bytes), A1), \
	AROS_LCA(int, (___len), D0), \
	AROS_LCA(int, (___indent), D1), \
	struct Library *, AMISSL_BASE_NAME, 331, Amissl)

#define BIO_gethostbyname(___name) \
	AROS_LC1(struct hostent *, BIO_gethostbyname, \
	AROS_LCA(const char *, (___name), A0), \
	struct Library *, AMISSL_BASE_NAME, 332, Amissl)

#define BIO_sock_error(___sock) \
	AROS_LC1(int, BIO_sock_error, \
	AROS_LCA(int, (___sock), D0), \
	struct Library *, AMISSL_BASE_NAME, 333, Amissl)

#define BIO_socket_ioctl(___fd, ___type, ___arg) \
	AROS_LC3(int, BIO_socket_ioctl, \
	AROS_LCA(int, (___fd), D0), \
	AROS_LCA(long, (___type), D1), \
	AROS_LCA(void *, (___arg), A0), \
	struct Library *, AMISSL_BASE_NAME, 334, Amissl)

#define BIO_socket_nbio(___fd, ___mode) \
	AROS_LC2(int, BIO_socket_nbio, \
	AROS_LCA(int, (___fd), D0), \
	AROS_LCA(int, (___mode), D1), \
	struct Library *, AMISSL_BASE_NAME, 335, Amissl)

#define BIO_get_port(___str, ___port_ptr) \
	AROS_LC2(int, BIO_get_port, \
	AROS_LCA(const char *, (___str), A0), \
	AROS_LCA(unsigned short *, (___port_ptr), A1), \
	struct Library *, AMISSL_BASE_NAME, 336, Amissl)

#define BIO_get_host_ip(___str, ___ip) \
	AROS_LC2(int, BIO_get_host_ip, \
	AROS_LCA(const char *, (___str), A0), \
	AROS_LCA(unsigned char *, (___ip), A1), \
	struct Library *, AMISSL_BASE_NAME, 337, Amissl)

#define BIO_get_accept_socket(___host_port, ___mode) \
	AROS_LC2(int, BIO_get_accept_socket, \
	AROS_LCA(char *, (___host_port), A0), \
	AROS_LCA(int, (___mode), D0), \
	struct Library *, AMISSL_BASE_NAME, 338, Amissl)

#define BIO_accept(___sock, ___ip_port) \
	AROS_LC2(int, BIO_accept, \
	AROS_LCA(int, (___sock), D0), \
	AROS_LCA(char **, (___ip_port), A0), \
	struct Library *, AMISSL_BASE_NAME, 339, Amissl)

#define BIO_sock_init() \
	AROS_LC0(int, BIO_sock_init, \
	struct Library *, AMISSL_BASE_NAME, 340, Amissl)

#define BIO_sock_cleanup() \
	AROS_LC0(void, BIO_sock_cleanup, \
	struct Library *, AMISSL_BASE_NAME, 341, Amissl)

#define BIO_set_tcp_ndelay(___sock, ___turn_on) \
	AROS_LC2(int, BIO_set_tcp_ndelay, \
	AROS_LCA(int, (___sock), D0), \
	AROS_LCA(int, (___turn_on), D1), \
	struct Library *, AMISSL_BASE_NAME, 342, Amissl)

#define BIO_new_socket(___sock, ___close_flag) \
	AROS_LC2(BIO *, BIO_new_socket, \
	AROS_LCA(int, (___sock), D0), \
	AROS_LCA(int, (___close_flag), D1), \
	struct Library *, AMISSL_BASE_NAME, 343, Amissl)

#define BIO_new_fd(___fd, ___close_flag) \
	AROS_LC2(BIO *, BIO_new_fd, \
	AROS_LCA(int, (___fd), D0), \
	AROS_LCA(int, (___close_flag), D1), \
	struct Library *, AMISSL_BASE_NAME, 344, Amissl)

#define BIO_new_connect(___host_port) \
	AROS_LC1(BIO *, BIO_new_connect, \
	AROS_LCA(char *, (___host_port), A0), \
	struct Library *, AMISSL_BASE_NAME, 345, Amissl)

#define BIO_new_accept(___host_port) \
	AROS_LC1(BIO *, BIO_new_accept, \
	AROS_LCA(char *, (___host_port), A0), \
	struct Library *, AMISSL_BASE_NAME, 346, Amissl)

#define BIO_new_bio_pair(___bio1, ___writebuf1, ___bio2, ___writebuf2) \
	AROS_LC4(int, BIO_new_bio_pair, \
	AROS_LCA(BIO **, (___bio1), A0), \
	AROS_LCA(size_t, (___writebuf1), D0), \
	AROS_LCA(BIO **, (___bio2), A1), \
	AROS_LCA(size_t, (___writebuf2), D1), \
	struct Library *, AMISSL_BASE_NAME, 347, Amissl)

#define BIO_copy_next_retry(___b) \
	AROS_LC1(void, BIO_copy_next_retry, \
	AROS_LCA(BIO *, (___b), A0), \
	struct Library *, AMISSL_BASE_NAME, 348, Amissl)

#define BIO_vprintf(___bio, ___format, ___args) \
	AROS_LC3(int, BIO_vprintf, \
	AROS_LCA(BIO *, (___bio), A0), \
	AROS_LCA(const char *, (___format), A1), \
	AROS_LCA(long *, (___args), D0), \
	struct Library *, AMISSL_BASE_NAME, 349, Amissl)

#ifndef NO_INLINE_VARARGS
#define BIO_printf(___bio, ___format, ___dummy, ...) \
	({_sfdc_vararg _message[] = { ___dummy, __VA_ARGS__ }; BIO_vprintf((___bio), (___format), (long *) _message); })
#endif /* !NO_INLINE_VARARGS */

#define BIO_vsnprintf(___buf, ___n, ___format, ___args) \
	AROS_LC4(int, BIO_vsnprintf, \
	AROS_LCA(char *, (___buf), A0), \
	AROS_LCA(size_t, (___n), D0), \
	AROS_LCA(const char *, (___format), A1), \
	AROS_LCA(long *, (___args), D1), \
	struct Library *, AMISSL_BASE_NAME, 350, Amissl)

#ifndef NO_INLINE_VARARGS
#define BIO_snprintf(___buf, ___n, ___format, ___dummy, ...) \
	({_sfdc_vararg _message[] = { ___dummy, __VA_ARGS__ }; BIO_vsnprintf((___buf), (___n), (___format), (long *) _message); })
#endif /* !NO_INLINE_VARARGS */

#define ERR_load_BIO_strings() \
	AROS_LC0(void, ERR_load_BIO_strings, \
	struct Library *, AMISSL_BASE_NAME, 351, Amissl)

#define BN_value_one() \
	AROS_LC0(const BIGNUM *, BN_value_one, \
	struct Library *, AMISSL_BASE_NAME, 352, Amissl)

#define BN_options() \
	AROS_LC0(char *, BN_options, \
	struct Library *, AMISSL_BASE_NAME, 353, Amissl)

#define BN_CTX_new() \
	AROS_LC0(BN_CTX *, BN_CTX_new, \
	struct Library *, AMISSL_BASE_NAME, 354, Amissl)

#define BN_CTX_init(___c) \
	AROS_LC1(void, BN_CTX_init, \
	AROS_LCA(BN_CTX *, (___c), A0), \
	struct Library *, AMISSL_BASE_NAME, 355, Amissl)

#define BN_CTX_free(___c) \
	AROS_LC1(void, BN_CTX_free, \
	AROS_LCA(BN_CTX *, (___c), A0), \
	struct Library *, AMISSL_BASE_NAME, 356, Amissl)

#define BN_CTX_start(___ctx) \
	AROS_LC1(void, BN_CTX_start, \
	AROS_LCA(BN_CTX *, (___ctx), A0), \
	struct Library *, AMISSL_BASE_NAME, 357, Amissl)

#define BN_CTX_get(___ctx) \
	AROS_LC1(BIGNUM *, BN_CTX_get, \
	AROS_LCA(BN_CTX *, (___ctx), A0), \
	struct Library *, AMISSL_BASE_NAME, 358, Amissl)

#define BN_CTX_end(___ctx) \
	AROS_LC1(void, BN_CTX_end, \
	AROS_LCA(BN_CTX *, (___ctx), A0), \
	struct Library *, AMISSL_BASE_NAME, 359, Amissl)

#define BN_rand(___rnd, ___bits, ___top, ___bottom) \
	AROS_LC4(int, BN_rand, \
	AROS_LCA(BIGNUM *, (___rnd), A0), \
	AROS_LCA(int, (___bits), D0), \
	AROS_LCA(int, (___top), D1), \
	AROS_LCA(int, (___bottom), D2), \
	struct Library *, AMISSL_BASE_NAME, 360, Amissl)

#define BN_pseudo_rand(___rnd, ___bits, ___top, ___bottom) \
	AROS_LC4(int, BN_pseudo_rand, \
	AROS_LCA(BIGNUM *, (___rnd), A0), \
	AROS_LCA(int, (___bits), D0), \
	AROS_LCA(int, (___top), D1), \
	AROS_LCA(int, (___bottom), D2), \
	struct Library *, AMISSL_BASE_NAME, 361, Amissl)

#define BN_rand_range(___rnd, ___range) \
	AROS_LC2(int, BN_rand_range, \
	AROS_LCA(BIGNUM *, (___rnd), A0), \
	AROS_LCA(BIGNUM *, (___range), A1), \
	struct Library *, AMISSL_BASE_NAME, 362, Amissl)

#define BN_pseudo_rand_range(___rnd, ___range) \
	AROS_LC2(int, BN_pseudo_rand_range, \
	AROS_LCA(BIGNUM *, (___rnd), A0), \
	AROS_LCA(BIGNUM *, (___range), A1), \
	struct Library *, AMISSL_BASE_NAME, 363, Amissl)

#define BN_num_bits(___a) \
	AROS_LC1(int, BN_num_bits, \
	AROS_LCA(const BIGNUM *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 364, Amissl)

#define BN_num_bits_word(___a) \
	AROS_LC1(int, BN_num_bits_word, \
	AROS_LCA(BN_ULONG, (___a), D0), \
	struct Library *, AMISSL_BASE_NAME, 365, Amissl)

#define BN_new() \
	AROS_LC0(BIGNUM *, BN_new, \
	struct Library *, AMISSL_BASE_NAME, 366, Amissl)

#define BN_init(___a) \
	AROS_LC1(void, BN_init, \
	AROS_LCA(BIGNUM *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 367, Amissl)

#define BN_clear_free(___a) \
	AROS_LC1(void, BN_clear_free, \
	AROS_LCA(BIGNUM *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 368, Amissl)

#define BN_copy(___a, ___b) \
	AROS_LC2(BIGNUM *, BN_copy, \
	AROS_LCA(BIGNUM *, (___a), A0), \
	AROS_LCA(const BIGNUM *, (___b), A1), \
	struct Library *, AMISSL_BASE_NAME, 369, Amissl)

#define BN_swap(___a, ___b) \
	AROS_LC2(void, BN_swap, \
	AROS_LCA(BIGNUM *, (___a), A0), \
	AROS_LCA(BIGNUM *, (___b), A1), \
	struct Library *, AMISSL_BASE_NAME, 370, Amissl)

#define BN_bin2bn(___s, ___len, ___ret) \
	AROS_LC3(BIGNUM *, BN_bin2bn, \
	AROS_LCA(const unsigned char *, (___s), A0), \
	AROS_LCA(int, (___len), D0), \
	AROS_LCA(BIGNUM *, (___ret), A1), \
	struct Library *, AMISSL_BASE_NAME, 371, Amissl)

#define BN_bn2bin(___a, ___to) \
	AROS_LC2(int, BN_bn2bin, \
	AROS_LCA(const BIGNUM *, (___a), A0), \
	AROS_LCA(unsigned char *, (___to), A1), \
	struct Library *, AMISSL_BASE_NAME, 372, Amissl)

#define BN_mpi2bn(___s, ___len, ___ret) \
	AROS_LC3(BIGNUM *, BN_mpi2bn, \
	AROS_LCA(const unsigned char *, (___s), A0), \
	AROS_LCA(int, (___len), D0), \
	AROS_LCA(BIGNUM *, (___ret), A1), \
	struct Library *, AMISSL_BASE_NAME, 373, Amissl)

#define BN_bn2mpi(___a, ___to) \
	AROS_LC2(int, BN_bn2mpi, \
	AROS_LCA(const BIGNUM *, (___a), A0), \
	AROS_LCA(unsigned char *, (___to), A1), \
	struct Library *, AMISSL_BASE_NAME, 374, Amissl)

#define BN_sub(___r, ___a, ___b) \
	AROS_LC3(int, BN_sub, \
	AROS_LCA(BIGNUM *, (___r), A0), \
	AROS_LCA(const BIGNUM *, (___a), A1), \
	AROS_LCA(const BIGNUM *, (___b), A2), \
	struct Library *, AMISSL_BASE_NAME, 375, Amissl)

#define BN_usub(___r, ___a, ___b) \
	AROS_LC3(int, BN_usub, \
	AROS_LCA(BIGNUM *, (___r), A0), \
	AROS_LCA(const BIGNUM *, (___a), A1), \
	AROS_LCA(const BIGNUM *, (___b), A2), \
	struct Library *, AMISSL_BASE_NAME, 376, Amissl)

#define BN_uadd(___r, ___a, ___b) \
	AROS_LC3(int, BN_uadd, \
	AROS_LCA(BIGNUM *, (___r), A0), \
	AROS_LCA(const BIGNUM *, (___a), A1), \
	AROS_LCA(const BIGNUM *, (___b), A2), \
	struct Library *, AMISSL_BASE_NAME, 377, Amissl)

#define BN_add(___r, ___a, ___b) \
	AROS_LC3(int, BN_add, \
	AROS_LCA(BIGNUM *, (___r), A0), \
	AROS_LCA(const BIGNUM *, (___a), A1), \
	AROS_LCA(const BIGNUM *, (___b), A2), \
	struct Library *, AMISSL_BASE_NAME, 378, Amissl)

#define BN_mul(___r, ___a, ___b, ___ctx) \
	AROS_LC4(int, BN_mul, \
	AROS_LCA(BIGNUM *, (___r), A0), \
	AROS_LCA(const BIGNUM *, (___a), A1), \
	AROS_LCA(const BIGNUM *, (___b), A2), \
	AROS_LCA(BN_CTX *, (___ctx), A3), \
	struct Library *, AMISSL_BASE_NAME, 379, Amissl)

#define BN_sqr(___r, ___a, ___ctx) \
	AROS_LC3(int, BN_sqr, \
	AROS_LCA(BIGNUM *, (___r), A0), \
	AROS_LCA(const BIGNUM *, (___a), A1), \
	AROS_LCA(BN_CTX *, (___ctx), A2), \
	struct Library *, AMISSL_BASE_NAME, 380, Amissl)

#define BN_div(___dv, ___rem, ___m, ___d, ___ctx) \
	AROS_LC5(int, BN_div, \
	AROS_LCA(BIGNUM *, (___dv), A0), \
	AROS_LCA(BIGNUM *, (___rem), A1), \
	AROS_LCA(const BIGNUM *, (___m), A2), \
	AROS_LCA(const BIGNUM *, (___d), A3), \
	AROS_LCA(BN_CTX *, (___ctx), D0), \
	struct Library *, AMISSL_BASE_NAME, 381, Amissl)

#define BN_nnmod(___r, ___m, ___d, ___ctx) \
	AROS_LC4(int, BN_nnmod, \
	AROS_LCA(BIGNUM *, (___r), A0), \
	AROS_LCA(const BIGNUM *, (___m), A1), \
	AROS_LCA(const BIGNUM *, (___d), A2), \
	AROS_LCA(BN_CTX *, (___ctx), A3), \
	struct Library *, AMISSL_BASE_NAME, 382, Amissl)

#define BN_mod_add(___r, ___a, ___b, ___m, ___ctx) \
	AROS_LC5(int, BN_mod_add, \
	AROS_LCA(BIGNUM *, (___r), A0), \
	AROS_LCA(const BIGNUM *, (___a), A1), \
	AROS_LCA(const BIGNUM *, (___b), A2), \
	AROS_LCA(const BIGNUM *, (___m), A3), \
	AROS_LCA(BN_CTX *, (___ctx), D0), \
	struct Library *, AMISSL_BASE_NAME, 383, Amissl)

#define BN_mod_add_quick(___r, ___a, ___b, ___m) \
	AROS_LC4(int, BN_mod_add_quick, \
	AROS_LCA(BIGNUM *, (___r), A0), \
	AROS_LCA(const BIGNUM *, (___a), A1), \
	AROS_LCA(const BIGNUM *, (___b), A2), \
	AROS_LCA(const BIGNUM *, (___m), A3), \
	struct Library *, AMISSL_BASE_NAME, 384, Amissl)

#define BN_mod_sub(___r, ___a, ___b, ___m, ___ctx) \
	AROS_LC5(int, BN_mod_sub, \
	AROS_LCA(BIGNUM *, (___r), A0), \
	AROS_LCA(const BIGNUM *, (___a), A1), \
	AROS_LCA(const BIGNUM *, (___b), A2), \
	AROS_LCA(const BIGNUM *, (___m), A3), \
	AROS_LCA(BN_CTX *, (___ctx), D0), \
	struct Library *, AMISSL_BASE_NAME, 385, Amissl)

#define BN_mod_sub_quick(___r, ___a, ___b, ___m) \
	AROS_LC4(int, BN_mod_sub_quick, \
	AROS_LCA(BIGNUM *, (___r), A0), \
	AROS_LCA(const BIGNUM *, (___a), A1), \
	AROS_LCA(const BIGNUM *, (___b), A2), \
	AROS_LCA(const BIGNUM *, (___m), A3), \
	struct Library *, AMISSL_BASE_NAME, 386, Amissl)

#define BN_mod_mul(___r, ___a, ___b, ___m, ___ctx) \
	AROS_LC5(int, BN_mod_mul, \
	AROS_LCA(BIGNUM *, (___r), A0), \
	AROS_LCA(const BIGNUM *, (___a), A1), \
	AROS_LCA(const BIGNUM *, (___b), A2), \
	AROS_LCA(const BIGNUM *, (___m), A3), \
	AROS_LCA(BN_CTX *, (___ctx), D0), \
	struct Library *, AMISSL_BASE_NAME, 387, Amissl)

#define BN_mod_sqr(___r, ___a, ___m, ___ctx) \
	AROS_LC4(int, BN_mod_sqr, \
	AROS_LCA(BIGNUM *, (___r), A0), \
	AROS_LCA(const BIGNUM *, (___a), A1), \
	AROS_LCA(const BIGNUM *, (___m), A2), \
	AROS_LCA(BN_CTX *, (___ctx), A3), \
	struct Library *, AMISSL_BASE_NAME, 388, Amissl)

#define BN_mod_lshift1(___r, ___a, ___m, ___ctx) \
	AROS_LC4(int, BN_mod_lshift1, \
	AROS_LCA(BIGNUM *, (___r), A0), \
	AROS_LCA(const BIGNUM *, (___a), A1), \
	AROS_LCA(const BIGNUM *, (___m), A2), \
	AROS_LCA(BN_CTX *, (___ctx), A3), \
	struct Library *, AMISSL_BASE_NAME, 389, Amissl)

#define BN_mod_lshift1_quick(___r, ___a, ___m) \
	AROS_LC3(int, BN_mod_lshift1_quick, \
	AROS_LCA(BIGNUM *, (___r), A0), \
	AROS_LCA(const BIGNUM *, (___a), A1), \
	AROS_LCA(const BIGNUM *, (___m), A2), \
	struct Library *, AMISSL_BASE_NAME, 390, Amissl)

#define BN_mod_lshift(___r, ___a, ___n, ___m, ___ctx) \
	AROS_LC5(int, BN_mod_lshift, \
	AROS_LCA(BIGNUM *, (___r), A0), \
	AROS_LCA(const BIGNUM *, (___a), A1), \
	AROS_LCA(int, (___n), D0), \
	AROS_LCA(const BIGNUM *, (___m), A2), \
	AROS_LCA(BN_CTX *, (___ctx), A3), \
	struct Library *, AMISSL_BASE_NAME, 391, Amissl)

#define BN_mod_lshift_quick(___r, ___a, ___n, ___m) \
	AROS_LC4(int, BN_mod_lshift_quick, \
	AROS_LCA(BIGNUM *, (___r), A0), \
	AROS_LCA(const BIGNUM *, (___a), A1), \
	AROS_LCA(int, (___n), D0), \
	AROS_LCA(const BIGNUM *, (___m), A2), \
	struct Library *, AMISSL_BASE_NAME, 392, Amissl)

#define BN_mod_word(___a, ___w) \
	AROS_LC2(BN_ULONG, BN_mod_word, \
	AROS_LCA(const BIGNUM *, (___a), A0), \
	AROS_LCA(BN_ULONG, (___w), D0), \
	struct Library *, AMISSL_BASE_NAME, 393, Amissl)

#define BN_div_word(___a, ___w) \
	AROS_LC2(BN_ULONG, BN_div_word, \
	AROS_LCA(BIGNUM *, (___a), A0), \
	AROS_LCA(BN_ULONG, (___w), D0), \
	struct Library *, AMISSL_BASE_NAME, 394, Amissl)

#define BN_mul_word(___a, ___w) \
	AROS_LC2(int, BN_mul_word, \
	AROS_LCA(BIGNUM *, (___a), A0), \
	AROS_LCA(BN_ULONG, (___w), D0), \
	struct Library *, AMISSL_BASE_NAME, 395, Amissl)

#define BN_add_word(___a, ___w) \
	AROS_LC2(int, BN_add_word, \
	AROS_LCA(BIGNUM *, (___a), A0), \
	AROS_LCA(BN_ULONG, (___w), D0), \
	struct Library *, AMISSL_BASE_NAME, 396, Amissl)

#define BN_sub_word(___a, ___w) \
	AROS_LC2(int, BN_sub_word, \
	AROS_LCA(BIGNUM *, (___a), A0), \
	AROS_LCA(BN_ULONG, (___w), D0), \
	struct Library *, AMISSL_BASE_NAME, 397, Amissl)

#define BN_set_word(___a, ___w) \
	AROS_LC2(int, BN_set_word, \
	AROS_LCA(BIGNUM *, (___a), A0), \
	AROS_LCA(BN_ULONG, (___w), D0), \
	struct Library *, AMISSL_BASE_NAME, 398, Amissl)

#define BN_get_word(___a) \
	AROS_LC1(BN_ULONG, BN_get_word, \
	AROS_LCA(const BIGNUM *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 399, Amissl)

#define BN_cmp(___a, ___b) \
	AROS_LC2(int, BN_cmp, \
	AROS_LCA(const BIGNUM *, (___a), A0), \
	AROS_LCA(const BIGNUM *, (___b), A1), \
	struct Library *, AMISSL_BASE_NAME, 400, Amissl)

#define BN_free(___a) \
	AROS_LC1(void, BN_free, \
	AROS_LCA(BIGNUM *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 401, Amissl)

#define BN_is_bit_set(___a, ___n) \
	AROS_LC2(int, BN_is_bit_set, \
	AROS_LCA(const BIGNUM *, (___a), A0), \
	AROS_LCA(int, (___n), D0), \
	struct Library *, AMISSL_BASE_NAME, 402, Amissl)

#define BN_lshift(___r, ___a, ___n) \
	AROS_LC3(int, BN_lshift, \
	AROS_LCA(BIGNUM *, (___r), A0), \
	AROS_LCA(const BIGNUM *, (___a), A1), \
	AROS_LCA(int, (___n), D0), \
	struct Library *, AMISSL_BASE_NAME, 403, Amissl)

#define BN_lshift1(___r, ___a) \
	AROS_LC2(int, BN_lshift1, \
	AROS_LCA(BIGNUM *, (___r), A0), \
	AROS_LCA(const BIGNUM *, (___a), A1), \
	struct Library *, AMISSL_BASE_NAME, 404, Amissl)

#define BN_exp(___r, ___a, ___p, ___ctx) \
	AROS_LC4(int, BN_exp, \
	AROS_LCA(BIGNUM *, (___r), A0), \
	AROS_LCA(const BIGNUM *, (___a), A1), \
	AROS_LCA(const BIGNUM *, (___p), A2), \
	AROS_LCA(BN_CTX *, (___ctx), A3), \
	struct Library *, AMISSL_BASE_NAME, 405, Amissl)

#define BN_mod_exp(___r, ___a, ___p, ___m, ___ctx) \
	AROS_LC5(int, BN_mod_exp, \
	AROS_LCA(BIGNUM *, (___r), A0), \
	AROS_LCA(const BIGNUM *, (___a), A1), \
	AROS_LCA(const BIGNUM *, (___p), A2), \
	AROS_LCA(const BIGNUM *, (___m), A3), \
	AROS_LCA(BN_CTX *, (___ctx), D0), \
	struct Library *, AMISSL_BASE_NAME, 406, Amissl)

#define BN_mod_exp_mont(___r, ___a, ___p, ___m, ___ctx, ___m_ctx) \
	AROS_LC6(int, BN_mod_exp_mont, \
	AROS_LCA(BIGNUM *, (___r), A0), \
	AROS_LCA(const BIGNUM *, (___a), A1), \
	AROS_LCA(const BIGNUM *, (___p), A2), \
	AROS_LCA(const BIGNUM *, (___m), A3), \
	AROS_LCA(BN_CTX *, (___ctx), D0), \
	AROS_LCA(BN_MONT_CTX *, (___m_ctx), D1), \
	struct Library *, AMISSL_BASE_NAME, 407, Amissl)

#define BN_mod_exp_mont_word(___r, ___a, ___p, ___m, ___ctx, ___m_ctx) \
	AROS_LC6(int, BN_mod_exp_mont_word, \
	AROS_LCA(BIGNUM *, (___r), A0), \
	AROS_LCA(BN_ULONG, (___a), D0), \
	AROS_LCA(const BIGNUM *, (___p), A1), \
	AROS_LCA(const BIGNUM *, (___m), A2), \
	AROS_LCA(BN_CTX *, (___ctx), A3), \
	AROS_LCA(BN_MONT_CTX *, (___m_ctx), D1), \
	struct Library *, AMISSL_BASE_NAME, 408, Amissl)

#define BN_mod_exp2_mont(___r, ___a1, ___p1, ___a2, ___p2, ___m, ___ctx, ___m_ctx) \
	AROS_LC8(int, BN_mod_exp2_mont, \
	AROS_LCA(BIGNUM *, (___r), A0), \
	AROS_LCA(const BIGNUM *, (___a1), A1), \
	AROS_LCA(const BIGNUM *, (___p1), A2), \
	AROS_LCA(const BIGNUM *, (___a2), A3), \
	AROS_LCA(const BIGNUM *, (___p2), D0), \
	AROS_LCA(const BIGNUM *, (___m), D1), \
	AROS_LCA(BN_CTX *, (___ctx), D2), \
	AROS_LCA(BN_MONT_CTX *, (___m_ctx), D3), \
	struct Library *, AMISSL_BASE_NAME, 409, Amissl)

#define BN_mod_exp_simple(___r, ___a, ___p, ___m, ___ctx) \
	AROS_LC5(int, BN_mod_exp_simple, \
	AROS_LCA(BIGNUM *, (___r), A0), \
	AROS_LCA(const BIGNUM *, (___a), A1), \
	AROS_LCA(const BIGNUM *, (___p), A2), \
	AROS_LCA(const BIGNUM *, (___m), A3), \
	AROS_LCA(BN_CTX *, (___ctx), D0), \
	struct Library *, AMISSL_BASE_NAME, 410, Amissl)

#define BN_mask_bits(___a, ___n) \
	AROS_LC2(int, BN_mask_bits, \
	AROS_LCA(BIGNUM *, (___a), A0), \
	AROS_LCA(int, (___n), D0), \
	struct Library *, AMISSL_BASE_NAME, 411, Amissl)

#define BN_print(___fp, ___a) \
	AROS_LC2(int, BN_print, \
	AROS_LCA(BIO *, (___fp), A0), \
	AROS_LCA(const BIGNUM *, (___a), A1), \
	struct Library *, AMISSL_BASE_NAME, 412, Amissl)

#define BN_reciprocal(___r, ___m, ___len, ___ctx) \
	AROS_LC4(int, BN_reciprocal, \
	AROS_LCA(BIGNUM *, (___r), A0), \
	AROS_LCA(const BIGNUM *, (___m), A1), \
	AROS_LCA(int, (___len), D0), \
	AROS_LCA(BN_CTX *, (___ctx), A2), \
	struct Library *, AMISSL_BASE_NAME, 413, Amissl)

#define BN_rshift(___r, ___a, ___n) \
	AROS_LC3(int, BN_rshift, \
	AROS_LCA(BIGNUM *, (___r), A0), \
	AROS_LCA(const BIGNUM *, (___a), A1), \
	AROS_LCA(int, (___n), D0), \
	struct Library *, AMISSL_BASE_NAME, 414, Amissl)

#define BN_rshift1(___r, ___a) \
	AROS_LC2(int, BN_rshift1, \
	AROS_LCA(BIGNUM *, (___r), A0), \
	AROS_LCA(const BIGNUM *, (___a), A1), \
	struct Library *, AMISSL_BASE_NAME, 415, Amissl)

#define BN_clear(___a) \
	AROS_LC1(void, BN_clear, \
	AROS_LCA(BIGNUM *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 416, Amissl)

#define BN_dup(___a) \
	AROS_LC1(BIGNUM *, BN_dup, \
	AROS_LCA(const BIGNUM *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 417, Amissl)

#define BN_ucmp(___a, ___b) \
	AROS_LC2(int, BN_ucmp, \
	AROS_LCA(const BIGNUM *, (___a), A0), \
	AROS_LCA(const BIGNUM *, (___b), A1), \
	struct Library *, AMISSL_BASE_NAME, 418, Amissl)

#define BN_set_bit(___a, ___n) \
	AROS_LC2(int, BN_set_bit, \
	AROS_LCA(BIGNUM *, (___a), A0), \
	AROS_LCA(int, (___n), D0), \
	struct Library *, AMISSL_BASE_NAME, 419, Amissl)

#define BN_clear_bit(___a, ___n) \
	AROS_LC2(int, BN_clear_bit, \
	AROS_LCA(BIGNUM *, (___a), A0), \
	AROS_LCA(int, (___n), D0), \
	struct Library *, AMISSL_BASE_NAME, 420, Amissl)

#define BN_bn2hex(___a) \
	AROS_LC1(char *, BN_bn2hex, \
	AROS_LCA(const BIGNUM *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 421, Amissl)

#define BN_bn2dec(___a) \
	AROS_LC1(char *, BN_bn2dec, \
	AROS_LCA(const BIGNUM *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 422, Amissl)

#define BN_hex2bn(___a, ___str) \
	AROS_LC2(int, BN_hex2bn, \
	AROS_LCA(BIGNUM **, (___a), A0), \
	AROS_LCA(const char *, (___str), A1), \
	struct Library *, AMISSL_BASE_NAME, 423, Amissl)

#define BN_dec2bn(___a, ___str) \
	AROS_LC2(int, BN_dec2bn, \
	AROS_LCA(BIGNUM **, (___a), A0), \
	AROS_LCA(const char *, (___str), A1), \
	struct Library *, AMISSL_BASE_NAME, 424, Amissl)

#define BN_gcd(___r, ___a, ___b, ___ctx) \
	AROS_LC4(int, BN_gcd, \
	AROS_LCA(BIGNUM *, (___r), A0), \
	AROS_LCA(const BIGNUM *, (___a), A1), \
	AROS_LCA(const BIGNUM *, (___b), A2), \
	AROS_LCA(BN_CTX *, (___ctx), A3), \
	struct Library *, AMISSL_BASE_NAME, 425, Amissl)

#define BN_kronecker(___a, ___b, ___ctx) \
	AROS_LC3(int, BN_kronecker, \
	AROS_LCA(const BIGNUM *, (___a), A0), \
	AROS_LCA(const BIGNUM *, (___b), A1), \
	AROS_LCA(BN_CTX *, (___ctx), A2), \
	struct Library *, AMISSL_BASE_NAME, 426, Amissl)

#define BN_mod_inverse(___ret, ___a, ___n, ___ctx) \
	AROS_LC4(BIGNUM *, BN_mod_inverse, \
	AROS_LCA(BIGNUM *, (___ret), A0), \
	AROS_LCA(const BIGNUM *, (___a), A1), \
	AROS_LCA(const BIGNUM *, (___n), A2), \
	AROS_LCA(BN_CTX *, (___ctx), A3), \
	struct Library *, AMISSL_BASE_NAME, 427, Amissl)

#define BN_mod_sqrt(___ret, ___a, ___n, ___ctx) \
	AROS_LC4(BIGNUM *, BN_mod_sqrt, \
	AROS_LCA(BIGNUM *, (___ret), A0), \
	AROS_LCA(const BIGNUM *, (___a), A1), \
	AROS_LCA(const BIGNUM *, (___n), A2), \
	AROS_LCA(BN_CTX *, (___ctx), A3), \
	struct Library *, AMISSL_BASE_NAME, 428, Amissl)

#define BN_generate_prime(___ret, ___bits, ___safe, ___add, ___rem, ___callback, ___cb_arg) \
	AROS_LC7(BIGNUM *, BN_generate_prime, \
	AROS_LCA(BIGNUM *, (___ret), A0), \
	AROS_LCA(int, (___bits), D0), \
	AROS_LCA(int, (___safe), D1), \
	AROS_LCA(const BIGNUM *, (___add), A1), \
	AROS_LCA(const BIGNUM *, (___rem), A2), \
	AROS_LCA(void (*)(int,int,void *), (___callback), A3), \
	AROS_LCA(void *, (___cb_arg), D2), \
	struct Library *, AMISSL_BASE_NAME, 429, Amissl)

#define BN_is_prime(___p, ___nchecks, ___callback, ___ctx, ___cb_arg) \
	AROS_LC5(int, BN_is_prime, \
	AROS_LCA(const BIGNUM *, (___p), A0), \
	AROS_LCA(int, (___nchecks), D0), \
	AROS_LCA(void (*)(int,int,void *), (___callback), A1), \
	AROS_LCA(BN_CTX *, (___ctx), A2), \
	AROS_LCA(void *, (___cb_arg), A3), \
	struct Library *, AMISSL_BASE_NAME, 430, Amissl)

#define BN_is_prime_fasttest(___p, ___nchecks, ___callback, ___ctx, ___cb_arg, ___do_trial_division) \
	AROS_LC6(int, BN_is_prime_fasttest, \
	AROS_LCA(const BIGNUM *, (___p), A0), \
	AROS_LCA(int, (___nchecks), D0), \
	AROS_LCA(void (*)(int,int,void *), (___callback), A1), \
	AROS_LCA(BN_CTX *, (___ctx), A2), \
	AROS_LCA(void *, (___cb_arg), A3), \
	AROS_LCA(int, (___do_trial_division), D1), \
	struct Library *, AMISSL_BASE_NAME, 431, Amissl)

#define BN_MONT_CTX_new() \
	AROS_LC0(BN_MONT_CTX *, BN_MONT_CTX_new, \
	struct Library *, AMISSL_BASE_NAME, 432, Amissl)

#define BN_MONT_CTX_init(___ctx) \
	AROS_LC1(void, BN_MONT_CTX_init, \
	AROS_LCA(BN_MONT_CTX *, (___ctx), A0), \
	struct Library *, AMISSL_BASE_NAME, 433, Amissl)

#define BN_mod_mul_montgomery(___r, ___a, ___b, ___mont, ___ctx) \
	AROS_LC5(int, BN_mod_mul_montgomery, \
	AROS_LCA(BIGNUM *, (___r), A0), \
	AROS_LCA(const BIGNUM *, (___a), A1), \
	AROS_LCA(const BIGNUM *, (___b), A2), \
	AROS_LCA(BN_MONT_CTX *, (___mont), A3), \
	AROS_LCA(BN_CTX *, (___ctx), D0), \
	struct Library *, AMISSL_BASE_NAME, 434, Amissl)

#define BN_from_montgomery(___r, ___a, ___mont, ___ctx) \
	AROS_LC4(int, BN_from_montgomery, \
	AROS_LCA(BIGNUM *, (___r), A0), \
	AROS_LCA(const BIGNUM *, (___a), A1), \
	AROS_LCA(BN_MONT_CTX *, (___mont), A2), \
	AROS_LCA(BN_CTX *, (___ctx), A3), \
	struct Library *, AMISSL_BASE_NAME, 435, Amissl)

#define BN_MONT_CTX_free(___mont) \
	AROS_LC1(void, BN_MONT_CTX_free, \
	AROS_LCA(BN_MONT_CTX *, (___mont), A0), \
	struct Library *, AMISSL_BASE_NAME, 436, Amissl)

#define BN_MONT_CTX_set(___mont, ___mod, ___ctx) \
	AROS_LC3(int, BN_MONT_CTX_set, \
	AROS_LCA(BN_MONT_CTX *, (___mont), A0), \
	AROS_LCA(const BIGNUM *, (___mod), A1), \
	AROS_LCA(BN_CTX *, (___ctx), A2), \
	struct Library *, AMISSL_BASE_NAME, 437, Amissl)

#define BN_MONT_CTX_copy(___to, ___from) \
	AROS_LC2(BN_MONT_CTX *, BN_MONT_CTX_copy, \
	AROS_LCA(BN_MONT_CTX *, (___to), A0), \
	AROS_LCA(BN_MONT_CTX *, (___from), A1), \
	struct Library *, AMISSL_BASE_NAME, 438, Amissl)

#define BN_BLINDING_new(___A, ___Ai, ___mod) \
	AROS_LC3(BN_BLINDING *, BN_BLINDING_new, \
	AROS_LCA(BIGNUM *, (___A), A0), \
	AROS_LCA(BIGNUM *, (___Ai), A1), \
	AROS_LCA(BIGNUM *, (___mod), A2), \
	struct Library *, AMISSL_BASE_NAME, 439, Amissl)

#define BN_BLINDING_free(___b) \
	AROS_LC1(void, BN_BLINDING_free, \
	AROS_LCA(BN_BLINDING *, (___b), A0), \
	struct Library *, AMISSL_BASE_NAME, 440, Amissl)

#define BN_BLINDING_update(___b, ___ctx) \
	AROS_LC2(int, BN_BLINDING_update, \
	AROS_LCA(BN_BLINDING *, (___b), A0), \
	AROS_LCA(BN_CTX *, (___ctx), A1), \
	struct Library *, AMISSL_BASE_NAME, 441, Amissl)

#define BN_BLINDING_convert(___n, ___r, ___ctx) \
	AROS_LC3(int, BN_BLINDING_convert, \
	AROS_LCA(BIGNUM *, (___n), A0), \
	AROS_LCA(BN_BLINDING *, (___r), A1), \
	AROS_LCA(BN_CTX *, (___ctx), A2), \
	struct Library *, AMISSL_BASE_NAME, 442, Amissl)

#define BN_BLINDING_invert(___n, ___b, ___ctx) \
	AROS_LC3(int, BN_BLINDING_invert, \
	AROS_LCA(BIGNUM *, (___n), A0), \
	AROS_LCA(BN_BLINDING *, (___b), A1), \
	AROS_LCA(BN_CTX *, (___ctx), A2), \
	struct Library *, AMISSL_BASE_NAME, 443, Amissl)

#define BN_set_params(___mul, ___high, ___low, ___mont) \
	AROS_LC4(void, BN_set_params, \
	AROS_LCA(int, (___mul), D0), \
	AROS_LCA(int, (___high), D1), \
	AROS_LCA(int, (___low), D2), \
	AROS_LCA(int, (___mont), D3), \
	struct Library *, AMISSL_BASE_NAME, 444, Amissl)

#define BN_get_params(___which) \
	AROS_LC1(int, BN_get_params, \
	AROS_LCA(int, (___which), D0), \
	struct Library *, AMISSL_BASE_NAME, 445, Amissl)

#define BN_RECP_CTX_init(___recp) \
	AROS_LC1(void, BN_RECP_CTX_init, \
	AROS_LCA(BN_RECP_CTX *, (___recp), A0), \
	struct Library *, AMISSL_BASE_NAME, 446, Amissl)

#define BN_RECP_CTX_new() \
	AROS_LC0(BN_RECP_CTX *, BN_RECP_CTX_new, \
	struct Library *, AMISSL_BASE_NAME, 447, Amissl)

#define BN_RECP_CTX_free(___recp) \
	AROS_LC1(void, BN_RECP_CTX_free, \
	AROS_LCA(BN_RECP_CTX *, (___recp), A0), \
	struct Library *, AMISSL_BASE_NAME, 448, Amissl)

#define BN_RECP_CTX_set(___recp, ___rdiv, ___ctx) \
	AROS_LC3(int, BN_RECP_CTX_set, \
	AROS_LCA(BN_RECP_CTX *, (___recp), A0), \
	AROS_LCA(const BIGNUM *, (___rdiv), A1), \
	AROS_LCA(BN_CTX *, (___ctx), A2), \
	struct Library *, AMISSL_BASE_NAME, 449, Amissl)

#define BN_mod_mul_reciprocal(___r, ___x, ___y, ___recp, ___ctx) \
	AROS_LC5(int, BN_mod_mul_reciprocal, \
	AROS_LCA(BIGNUM *, (___r), A0), \
	AROS_LCA(const BIGNUM *, (___x), A1), \
	AROS_LCA(const BIGNUM *, (___y), A2), \
	AROS_LCA(BN_RECP_CTX *, (___recp), A3), \
	AROS_LCA(BN_CTX *, (___ctx), D0), \
	struct Library *, AMISSL_BASE_NAME, 450, Amissl)

#define BN_mod_exp_recp(___r, ___a, ___p, ___m, ___ctx) \
	AROS_LC5(int, BN_mod_exp_recp, \
	AROS_LCA(BIGNUM *, (___r), A0), \
	AROS_LCA(const BIGNUM *, (___a), A1), \
	AROS_LCA(const BIGNUM *, (___p), A2), \
	AROS_LCA(const BIGNUM *, (___m), A3), \
	AROS_LCA(BN_CTX *, (___ctx), D0), \
	struct Library *, AMISSL_BASE_NAME, 451, Amissl)

#define BN_div_recp(___dv, ___rem, ___m, ___recp, ___ctx) \
	AROS_LC5(int, BN_div_recp, \
	AROS_LCA(BIGNUM *, (___dv), A0), \
	AROS_LCA(BIGNUM *, (___rem), A1), \
	AROS_LCA(const BIGNUM *, (___m), A2), \
	AROS_LCA(BN_RECP_CTX *, (___recp), A3), \
	AROS_LCA(BN_CTX *, (___ctx), D0), \
	struct Library *, AMISSL_BASE_NAME, 452, Amissl)

#define bn_expand2(___a, ___words) \
	AROS_LC2(BIGNUM *, bn_expand2, \
	AROS_LCA(BIGNUM *, (___a), A0), \
	AROS_LCA(int, (___words), D0), \
	struct Library *, AMISSL_BASE_NAME, 453, Amissl)

#define bn_dup_expand(___a, ___words) \
	AROS_LC2(BIGNUM *, bn_dup_expand, \
	AROS_LCA(const BIGNUM *, (___a), A0), \
	AROS_LCA(int, (___words), D0), \
	struct Library *, AMISSL_BASE_NAME, 454, Amissl)

#define bn_mul_add_words(___rp, ___ap, ___num, ___w) \
	AROS_LC4(BN_ULONG, bn_mul_add_words, \
	AROS_LCA(BN_ULONG *, (___rp), A0), \
	AROS_LCA(const BN_ULONG *, (___ap), A1), \
	AROS_LCA(int, (___num), D0), \
	AROS_LCA(BN_ULONG, (___w), D1), \
	struct Library *, AMISSL_BASE_NAME, 455, Amissl)

#define bn_mul_words(___rp, ___ap, ___num, ___w) \
	AROS_LC4(BN_ULONG, bn_mul_words, \
	AROS_LCA(BN_ULONG *, (___rp), A0), \
	AROS_LCA(const BN_ULONG *, (___ap), A1), \
	AROS_LCA(int, (___num), D0), \
	AROS_LCA(BN_ULONG, (___w), D1), \
	struct Library *, AMISSL_BASE_NAME, 456, Amissl)

#define bn_sqr_words(___rp, ___ap, ___num) \
	AROS_LC3(void, bn_sqr_words, \
	AROS_LCA(BN_ULONG *, (___rp), A0), \
	AROS_LCA(const BN_ULONG *, (___ap), A1), \
	AROS_LCA(int, (___num), D0), \
	struct Library *, AMISSL_BASE_NAME, 457, Amissl)

#define bn_div_words(___h, ___l, ___d) \
	AROS_LC3(BN_ULONG, bn_div_words, \
	AROS_LCA(BN_ULONG, (___h), D0), \
	AROS_LCA(BN_ULONG, (___l), D1), \
	AROS_LCA(BN_ULONG, (___d), D2), \
	struct Library *, AMISSL_BASE_NAME, 458, Amissl)

#define bn_add_words(___rp, ___ap, ___bp, ___num) \
	AROS_LC4(BN_ULONG, bn_add_words, \
	AROS_LCA(BN_ULONG *, (___rp), A0), \
	AROS_LCA(const BN_ULONG *, (___ap), A1), \
	AROS_LCA(const BN_ULONG *, (___bp), A2), \
	AROS_LCA(int, (___num), D0), \
	struct Library *, AMISSL_BASE_NAME, 459, Amissl)

#define bn_sub_words(___rp, ___ap, ___bp, ___num) \
	AROS_LC4(BN_ULONG, bn_sub_words, \
	AROS_LCA(BN_ULONG *, (___rp), A0), \
	AROS_LCA(const BN_ULONG *, (___ap), A1), \
	AROS_LCA(const BN_ULONG *, (___bp), A2), \
	AROS_LCA(int, (___num), D0), \
	struct Library *, AMISSL_BASE_NAME, 460, Amissl)

#define BN_bntest_rand(___rnd, ___bits, ___top, ___bottom) \
	AROS_LC4(int, BN_bntest_rand, \
	AROS_LCA(BIGNUM *, (___rnd), A0), \
	AROS_LCA(int, (___bits), D0), \
	AROS_LCA(int, (___top), D1), \
	AROS_LCA(int, (___bottom), D2), \
	struct Library *, AMISSL_BASE_NAME, 461, Amissl)

#define ERR_load_BN_strings() \
	AROS_LC0(void, ERR_load_BN_strings, \
	struct Library *, AMISSL_BASE_NAME, 462, Amissl)

#define BUF_MEM_new() \
	AROS_LC0(BUF_MEM *, BUF_MEM_new, \
	struct Library *, AMISSL_BASE_NAME, 463, Amissl)

#define BUF_MEM_free(___a) \
	AROS_LC1(void, BUF_MEM_free, \
	AROS_LCA(BUF_MEM *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 464, Amissl)

#define BUF_MEM_grow(___str, ___len) \
	AROS_LC2(int, BUF_MEM_grow, \
	AROS_LCA(BUF_MEM *, (___str), A0), \
	AROS_LCA(int, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 465, Amissl)

#define BUF_MEM_grow_clean(___str, ___len) \
	AROS_LC2(int, BUF_MEM_grow_clean, \
	AROS_LCA(BUF_MEM *, (___str), A0), \
	AROS_LCA(int, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 466, Amissl)

#define BUF_strdup(___str) \
	AROS_LC1(char *, BUF_strdup, \
	AROS_LCA(const char *, (___str), A0), \
	struct Library *, AMISSL_BASE_NAME, 467, Amissl)

#define BUF_strlcpy(___dst, ___src, ___siz) \
	AROS_LC3(size_t, BUF_strlcpy, \
	AROS_LCA(char *, (___dst), A0), \
	AROS_LCA(const char *, (___src), A1), \
	AROS_LCA(size_t, (___siz), D0), \
	struct Library *, AMISSL_BASE_NAME, 468, Amissl)

#define BUF_strlcat(___dst, ___src, ___siz) \
	AROS_LC3(size_t, BUF_strlcat, \
	AROS_LCA(char *, (___dst), A0), \
	AROS_LCA(const char *, (___src), A1), \
	AROS_LCA(size_t, (___siz), D0), \
	struct Library *, AMISSL_BASE_NAME, 469, Amissl)

#define ERR_load_BUF_strings() \
	AROS_LC0(void, ERR_load_BUF_strings, \
	struct Library *, AMISSL_BASE_NAME, 470, Amissl)

#define COMP_CTX_new(___meth) \
	AROS_LC1(COMP_CTX *, COMP_CTX_new, \
	AROS_LCA(COMP_METHOD *, (___meth), A0), \
	struct Library *, AMISSL_BASE_NAME, 471, Amissl)

#define COMP_CTX_free(___ctx) \
	AROS_LC1(void, COMP_CTX_free, \
	AROS_LCA(COMP_CTX *, (___ctx), A0), \
	struct Library *, AMISSL_BASE_NAME, 472, Amissl)

#define COMP_compress_block(___ctx, ___out, ___olen, ___in, ___ilen) \
	AROS_LC5(int, COMP_compress_block, \
	AROS_LCA(COMP_CTX *, (___ctx), A0), \
	AROS_LCA(unsigned char *, (___out), A1), \
	AROS_LCA(int, (___olen), D0), \
	AROS_LCA(unsigned char *, (___in), A2), \
	AROS_LCA(int, (___ilen), D1), \
	struct Library *, AMISSL_BASE_NAME, 473, Amissl)

#define COMP_expand_block(___ctx, ___out, ___olen, ___in, ___ilen) \
	AROS_LC5(int, COMP_expand_block, \
	AROS_LCA(COMP_CTX *, (___ctx), A0), \
	AROS_LCA(unsigned char *, (___out), A1), \
	AROS_LCA(int, (___olen), D0), \
	AROS_LCA(unsigned char *, (___in), A2), \
	AROS_LCA(int, (___ilen), D1), \
	struct Library *, AMISSL_BASE_NAME, 474, Amissl)

#define COMP_rle() \
	AROS_LC0(COMP_METHOD *, COMP_rle, \
	struct Library *, AMISSL_BASE_NAME, 475, Amissl)

#define COMP_zlib() \
	AROS_LC0(COMP_METHOD *, COMP_zlib, \
	struct Library *, AMISSL_BASE_NAME, 476, Amissl)

#define ERR_load_COMP_strings() \
	AROS_LC0(void, ERR_load_COMP_strings, \
	struct Library *, AMISSL_BASE_NAME, 477, Amissl)

#define CONF_set_default_method(___meth) \
	AROS_LC1(int, CONF_set_default_method, \
	AROS_LCA(CONF_METHOD *, (___meth), A0), \
	struct Library *, AMISSL_BASE_NAME, 478, Amissl)

#define CONF_set_nconf(___conf, ___hash) \
	AROS_LC2(void, CONF_set_nconf, \
	AROS_LCA(CONF *, (___conf), A0), \
	AROS_LCA(LHASH *, (___hash), A1), \
	struct Library *, AMISSL_BASE_NAME, 479, Amissl)

#define CONF_load(___conf, ___file, ___eline) \
	AROS_LC3(LHASH *, CONF_load, \
	AROS_LCA(LHASH *, (___conf), A0), \
	AROS_LCA(const char *, (___file), A1), \
	AROS_LCA(long *, (___eline), A2), \
	struct Library *, AMISSL_BASE_NAME, 480, Amissl)

#define CONF_load_bio(___conf, ___bp, ___eline) \
	AROS_LC3(LHASH *, CONF_load_bio, \
	AROS_LCA(LHASH *, (___conf), A0), \
	AROS_LCA(BIO *, (___bp), A1), \
	AROS_LCA(long *, (___eline), A2), \
	struct Library *, AMISSL_BASE_NAME, 481, Amissl)

#define CONF_get_section(___conf, ___section) \
	AROS_LC2(STACK_OF(CONF_VALUE) *, CONF_get_section, \
	AROS_LCA(LHASH *, (___conf), A0), \
	AROS_LCA(const char *, (___section), A1), \
	struct Library *, AMISSL_BASE_NAME, 482, Amissl)

#define CONF_get_string(___conf, ___group, ___name) \
	AROS_LC3(char *, CONF_get_string, \
	AROS_LCA(LHASH *, (___conf), A0), \
	AROS_LCA(const char *, (___group), A1), \
	AROS_LCA(const char *, (___name), A2), \
	struct Library *, AMISSL_BASE_NAME, 483, Amissl)

#define CONF_get_number(___conf, ___group, ___name) \
	AROS_LC3(long, CONF_get_number, \
	AROS_LCA(LHASH *, (___conf), A0), \
	AROS_LCA(const char *, (___group), A1), \
	AROS_LCA(const char *, (___name), A2), \
	struct Library *, AMISSL_BASE_NAME, 484, Amissl)

#define CONF_free(___conf) \
	AROS_LC1(void, CONF_free, \
	AROS_LCA(LHASH *, (___conf), A0), \
	struct Library *, AMISSL_BASE_NAME, 485, Amissl)

#define CONF_dump_bio(___conf, ___out) \
	AROS_LC2(int, CONF_dump_bio, \
	AROS_LCA(LHASH *, (___conf), A0), \
	AROS_LCA(BIO *, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 486, Amissl)

#define OPENSSL_config(___config_name) \
	AROS_LC1(void, OPENSSL_config, \
	AROS_LCA(const char *, (___config_name), A0), \
	struct Library *, AMISSL_BASE_NAME, 487, Amissl)

#define OPENSSL_no_config() \
	AROS_LC0(void, OPENSSL_no_config, \
	struct Library *, AMISSL_BASE_NAME, 488, Amissl)

#define NCONF_new(___meth) \
	AROS_LC1(CONF *, NCONF_new, \
	AROS_LCA(CONF_METHOD *, (___meth), A0), \
	struct Library *, AMISSL_BASE_NAME, 489, Amissl)

#define NCONF_default() \
	AROS_LC0(CONF_METHOD *, NCONF_default, \
	struct Library *, AMISSL_BASE_NAME, 490, Amissl)

#define NCONF_WIN32() \
	AROS_LC0(CONF_METHOD *, NCONF_WIN32, \
	struct Library *, AMISSL_BASE_NAME, 491, Amissl)

#define NCONF_free(___conf) \
	AROS_LC1(void, NCONF_free, \
	AROS_LCA(CONF *, (___conf), A0), \
	struct Library *, AMISSL_BASE_NAME, 492, Amissl)

#define NCONF_free_data(___conf) \
	AROS_LC1(void, NCONF_free_data, \
	AROS_LCA(CONF *, (___conf), A0), \
	struct Library *, AMISSL_BASE_NAME, 493, Amissl)

#define NCONF_load(___conf, ___file, ___eline) \
	AROS_LC3(int, NCONF_load, \
	AROS_LCA(CONF *, (___conf), A0), \
	AROS_LCA(const char *, (___file), A1), \
	AROS_LCA(long *, (___eline), A2), \
	struct Library *, AMISSL_BASE_NAME, 494, Amissl)

#define NCONF_load_bio(___conf, ___bp, ___eline) \
	AROS_LC3(int, NCONF_load_bio, \
	AROS_LCA(CONF *, (___conf), A0), \
	AROS_LCA(BIO *, (___bp), A1), \
	AROS_LCA(long *, (___eline), A2), \
	struct Library *, AMISSL_BASE_NAME, 495, Amissl)

#define NCONF_get_section(___conf, ___section) \
	AROS_LC2(STACK_OF(CONF_VALUE) *, NCONF_get_section, \
	AROS_LCA(const CONF *, (___conf), A0), \
	AROS_LCA(const char *, (___section), A1), \
	struct Library *, AMISSL_BASE_NAME, 496, Amissl)

#define NCONF_get_string(___conf, ___group, ___name) \
	AROS_LC3(char *, NCONF_get_string, \
	AROS_LCA(const CONF *, (___conf), A0), \
	AROS_LCA(const char *, (___group), A1), \
	AROS_LCA(const char *, (___name), A2), \
	struct Library *, AMISSL_BASE_NAME, 497, Amissl)

#define NCONF_get_number_e(___conf, ___group, ___name, ___result) \
	AROS_LC4(int, NCONF_get_number_e, \
	AROS_LCA(const CONF *, (___conf), A0), \
	AROS_LCA(const char *, (___group), A1), \
	AROS_LCA(const char *, (___name), A2), \
	AROS_LCA(long *, (___result), A3), \
	struct Library *, AMISSL_BASE_NAME, 498, Amissl)

#define NCONF_dump_bio(___conf, ___out) \
	AROS_LC2(int, NCONF_dump_bio, \
	AROS_LCA(const CONF *, (___conf), A0), \
	AROS_LCA(BIO *, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 499, Amissl)

#define CONF_modules_load(___cnf, ___appname, ___flags) \
	AROS_LC3(int, CONF_modules_load, \
	AROS_LCA(const CONF *, (___cnf), A0), \
	AROS_LCA(const char *, (___appname), A1), \
	AROS_LCA(unsigned long, (___flags), D0), \
	struct Library *, AMISSL_BASE_NAME, 500, Amissl)

#define CONF_modules_load_file(___filename, ___appname, ___flags) \
	AROS_LC3(int, CONF_modules_load_file, \
	AROS_LCA(const char *, (___filename), A0), \
	AROS_LCA(const char *, (___appname), A1), \
	AROS_LCA(unsigned long, (___flags), D0), \
	struct Library *, AMISSL_BASE_NAME, 501, Amissl)

#define CONF_modules_unload(___all) \
	AROS_LC1(void, CONF_modules_unload, \
	AROS_LCA(int, (___all), D0), \
	struct Library *, AMISSL_BASE_NAME, 502, Amissl)

#define CONF_modules_finish() \
	AROS_LC0(void, CONF_modules_finish, \
	struct Library *, AMISSL_BASE_NAME, 503, Amissl)

#define CONF_modules_free() \
	AROS_LC0(void, CONF_modules_free, \
	struct Library *, AMISSL_BASE_NAME, 504, Amissl)

#define CONF_module_add(___name, ___ifunc, ___ffunc) \
	AROS_LC3(int, CONF_module_add, \
	AROS_LCA(const char *, (___name), A0), \
	AROS_LCA(conf_init_func * (*)(), (___ifunc), A1), \
	AROS_LCA(conf_finish_func * (*)(), (___ffunc), A2), \
	struct Library *, AMISSL_BASE_NAME, 505, Amissl)

#define CONF_imodule_get_name(___md) \
	AROS_LC1(const char *, CONF_imodule_get_name, \
	AROS_LCA(const CONF_IMODULE *, (___md), A0), \
	struct Library *, AMISSL_BASE_NAME, 506, Amissl)

#define CONF_imodule_get_value(___md) \
	AROS_LC1(const char *, CONF_imodule_get_value, \
	AROS_LCA(const CONF_IMODULE *, (___md), A0), \
	struct Library *, AMISSL_BASE_NAME, 507, Amissl)

#define CONF_imodule_get_usr_data(___md) \
	AROS_LC1(void *, CONF_imodule_get_usr_data, \
	AROS_LCA(const CONF_IMODULE *, (___md), A0), \
	struct Library *, AMISSL_BASE_NAME, 508, Amissl)

#define CONF_imodule_set_usr_data(___md, ___usr_data) \
	AROS_LC2(void, CONF_imodule_set_usr_data, \
	AROS_LCA(CONF_IMODULE *, (___md), A0), \
	AROS_LCA(void *, (___usr_data), A1), \
	struct Library *, AMISSL_BASE_NAME, 509, Amissl)

#define CONF_imodule_get_module(___md) \
	AROS_LC1(CONF_MODULE *, CONF_imodule_get_module, \
	AROS_LCA(const CONF_IMODULE *, (___md), A0), \
	struct Library *, AMISSL_BASE_NAME, 510, Amissl)

#define CONF_imodule_get_flags(___md) \
	AROS_LC1(unsigned long, CONF_imodule_get_flags, \
	AROS_LCA(const CONF_IMODULE *, (___md), A0), \
	struct Library *, AMISSL_BASE_NAME, 511, Amissl)

#define CONF_imodule_set_flags(___md, ___flags) \
	AROS_LC2(void, CONF_imodule_set_flags, \
	AROS_LCA(CONF_IMODULE *, (___md), A0), \
	AROS_LCA(unsigned long, (___flags), D0), \
	struct Library *, AMISSL_BASE_NAME, 512, Amissl)

#define CONF_module_get_usr_data(___pmod) \
	AROS_LC1(void *, CONF_module_get_usr_data, \
	AROS_LCA(CONF_MODULE *, (___pmod), A0), \
	struct Library *, AMISSL_BASE_NAME, 513, Amissl)

#define CONF_module_set_usr_data(___pmod, ___usr_data) \
	AROS_LC2(void, CONF_module_set_usr_data, \
	AROS_LCA(CONF_MODULE *, (___pmod), A0), \
	AROS_LCA(void *, (___usr_data), A1), \
	struct Library *, AMISSL_BASE_NAME, 514, Amissl)

#define CONF_get1_default_config_file() \
	AROS_LC0(char *, CONF_get1_default_config_file, \
	struct Library *, AMISSL_BASE_NAME, 515, Amissl)

#define CONF_parse_list(___list, ___sep, ___nospc, ___list_cb, ___arg) \
	AROS_LC5(int, CONF_parse_list, \
	AROS_LCA(const char *, (___list), A0), \
	AROS_LCA(int, (___sep), D0), \
	AROS_LCA(int, (___nospc), D1), \
	AROS_LCA(int (*)(const char *elem,int len,void *usr), (___list_cb), A1), \
	AROS_LCA(void *, (___arg), A2), \
	struct Library *, AMISSL_BASE_NAME, 516, Amissl)

#define OPENSSL_load_builtin_modules() \
	AROS_LC0(void, OPENSSL_load_builtin_modules, \
	struct Library *, AMISSL_BASE_NAME, 517, Amissl)

#define ERR_load_CONF_strings() \
	AROS_LC0(void, ERR_load_CONF_strings, \
	struct Library *, AMISSL_BASE_NAME, 518, Amissl)

#define _CONF_new_section(___conf, ___section) \
	AROS_LC2(CONF_VALUE *, _CONF_new_section, \
	AROS_LCA(CONF *, (___conf), A0), \
	AROS_LCA(const char *, (___section), A1), \
	struct Library *, AMISSL_BASE_NAME, 519, Amissl)

#define _CONF_get_section(___conf, ___section) \
	AROS_LC2(CONF_VALUE *, _CONF_get_section, \
	AROS_LCA(const CONF *, (___conf), A0), \
	AROS_LCA(const char *, (___section), A1), \
	struct Library *, AMISSL_BASE_NAME, 520, Amissl)

#define _CONF_get_section_values(___conf, ___section) \
	AROS_LC2(STACK_OF(CONF_VALUE) *, _CONF_get_section_values, \
	AROS_LCA(const CONF *, (___conf), A0), \
	AROS_LCA(const char *, (___section), A1), \
	struct Library *, AMISSL_BASE_NAME, 521, Amissl)

#define _CONF_add_string(___conf, ___section, ___value) \
	AROS_LC3(int, _CONF_add_string, \
	AROS_LCA(CONF *, (___conf), A0), \
	AROS_LCA(CONF_VALUE *, (___section), A1), \
	AROS_LCA(CONF_VALUE *, (___value), A2), \
	struct Library *, AMISSL_BASE_NAME, 522, Amissl)

#define _CONF_get_string(___conf, ___section, ___name) \
	AROS_LC3(char *, _CONF_get_string, \
	AROS_LCA(const CONF *, (___conf), A0), \
	AROS_LCA(const char *, (___section), A1), \
	AROS_LCA(const char *, (___name), A2), \
	struct Library *, AMISSL_BASE_NAME, 523, Amissl)

#define _CONF_new_data(___conf) \
	AROS_LC1(int, _CONF_new_data, \
	AROS_LCA(CONF *, (___conf), A0), \
	struct Library *, AMISSL_BASE_NAME, 524, Amissl)

#define _CONF_free_data(___conf) \
	AROS_LC1(void, _CONF_free_data, \
	AROS_LCA(CONF *, (___conf), A0), \
	struct Library *, AMISSL_BASE_NAME, 525, Amissl)

#define CRYPTO_mem_ctrl(___mode) \
	AROS_LC1(int, CRYPTO_mem_ctrl, \
	AROS_LCA(int, (___mode), D0), \
	struct Library *, AMISSL_BASE_NAME, 526, Amissl)

#define CRYPTO_is_mem_check_on() \
	AROS_LC0(int, CRYPTO_is_mem_check_on, \
	struct Library *, AMISSL_BASE_NAME, 527, Amissl)

#define SSLeay_version(___type) \
	AROS_LC1(const char *, SSLeay_version, \
	AROS_LCA(int, (___type), D0), \
	struct Library *, AMISSL_BASE_NAME, 528, Amissl)

#define SSLeay() \
	AROS_LC0(unsigned long, SSLeay, \
	struct Library *, AMISSL_BASE_NAME, 529, Amissl)

#define OPENSSL_issetugid() \
	AROS_LC0(int, OPENSSL_issetugid, \
	struct Library *, AMISSL_BASE_NAME, 530, Amissl)

#define CRYPTO_get_ex_data_implementation() \
	AROS_LC0(const CRYPTO_EX_DATA_IMPL *, CRYPTO_get_ex_data_implementation, \
	struct Library *, AMISSL_BASE_NAME, 531, Amissl)

#define CRYPTO_set_ex_data_implementation(___i) \
	AROS_LC1(int, CRYPTO_set_ex_data_implementation, \
	AROS_LCA(const CRYPTO_EX_DATA_IMPL *, (___i), A0), \
	struct Library *, AMISSL_BASE_NAME, 532, Amissl)

#define CRYPTO_ex_data_new_class() \
	AROS_LC0(int, CRYPTO_ex_data_new_class, \
	struct Library *, AMISSL_BASE_NAME, 533, Amissl)

#define CRYPTO_get_ex_new_index(___class_index, ___argl, ___argp, ___new_func, ___dup_func, ___free_func) \
	AROS_LC6(int, CRYPTO_get_ex_new_index, \
	AROS_LCA(int, (___class_index), D0), \
	AROS_LCA(long, (___argl), D1), \
	AROS_LCA(void *, (___argp), A0), \
	AROS_LCA(CRYPTO_EX_new * (*)(), (___new_func), A1), \
	AROS_LCA(CRYPTO_EX_dup * (*)(), (___dup_func), A2), \
	AROS_LCA(CRYPTO_EX_free * (*)(), (___free_func), A3), \
	struct Library *, AMISSL_BASE_NAME, 534, Amissl)

#define CRYPTO_new_ex_data(___class_index, ___obj, ___ad) \
	AROS_LC3(int, CRYPTO_new_ex_data, \
	AROS_LCA(int, (___class_index), D0), \
	AROS_LCA(void *, (___obj), A0), \
	AROS_LCA(CRYPTO_EX_DATA *, (___ad), A1), \
	struct Library *, AMISSL_BASE_NAME, 535, Amissl)

#define CRYPTO_dup_ex_data(___class_index, ___to, ___from) \
	AROS_LC3(int, CRYPTO_dup_ex_data, \
	AROS_LCA(int, (___class_index), D0), \
	AROS_LCA(CRYPTO_EX_DATA *, (___to), A0), \
	AROS_LCA(CRYPTO_EX_DATA *, (___from), A1), \
	struct Library *, AMISSL_BASE_NAME, 536, Amissl)

#define CRYPTO_free_ex_data(___class_index, ___obj, ___ad) \
	AROS_LC3(void, CRYPTO_free_ex_data, \
	AROS_LCA(int, (___class_index), D0), \
	AROS_LCA(void *, (___obj), A0), \
	AROS_LCA(CRYPTO_EX_DATA *, (___ad), A1), \
	struct Library *, AMISSL_BASE_NAME, 537, Amissl)

#define CRYPTO_set_ex_data(___ad, ___idx, ___val) \
	AROS_LC3(int, CRYPTO_set_ex_data, \
	AROS_LCA(CRYPTO_EX_DATA *, (___ad), A0), \
	AROS_LCA(int, (___idx), D0), \
	AROS_LCA(void *, (___val), A1), \
	struct Library *, AMISSL_BASE_NAME, 538, Amissl)

#define CRYPTO_get_ex_data(___ad, ___idx) \
	AROS_LC2(void *, CRYPTO_get_ex_data, \
	AROS_LCA(const CRYPTO_EX_DATA *, (___ad), A0), \
	AROS_LCA(int, (___idx), D0), \
	struct Library *, AMISSL_BASE_NAME, 539, Amissl)

#define CRYPTO_cleanup_all_ex_data() \
	AROS_LC0(void, CRYPTO_cleanup_all_ex_data, \
	struct Library *, AMISSL_BASE_NAME, 540, Amissl)

#define CRYPTO_get_new_lockid(___name) \
	AROS_LC1(int, CRYPTO_get_new_lockid, \
	AROS_LCA(char *, (___name), A0), \
	struct Library *, AMISSL_BASE_NAME, 541, Amissl)

#define CRYPTO_num_locks() \
	AROS_LC0(int, CRYPTO_num_locks, \
	struct Library *, AMISSL_BASE_NAME, 542, Amissl)

#define CRYPTO_lock(___mode, ___type, ___file, ___line) \
	AROS_LC4(void, CRYPTO_lock, \
	AROS_LCA(int, (___mode), D0), \
	AROS_LCA(int, (___type), D1), \
	AROS_LCA(const char *, (___file), A0), \
	AROS_LCA(int, (___line), D2), \
	struct Library *, AMISSL_BASE_NAME, 543, Amissl)

#define CRYPTO_set_locking_callback(___func) \
	AROS_LC1(void, CRYPTO_set_locking_callback, \
	AROS_LCA(void (*)(int mode,int type,const char *file,int line), (___func), A0), \
	struct Library *, AMISSL_BASE_NAME, 544, Amissl)

#define CRYPTO_get_locking_callback() \
	AROS_LC0(void *, CRYPTO_get_locking_callback, \
	struct Library *, AMISSL_BASE_NAME, 545, Amissl)

#define CRYPTO_set_add_lock_callback(___func) \
	AROS_LC1(void, CRYPTO_set_add_lock_callback, \
	AROS_LCA(int (*)(int *num,int mount,int type,const char *file,int line), (___func), A0), \
	struct Library *, AMISSL_BASE_NAME, 546, Amissl)

#define CRYPTO_get_add_lock_callback() \
	AROS_LC0(int *, CRYPTO_get_add_lock_callback, \
	struct Library *, AMISSL_BASE_NAME, 547, Amissl)

#define CRYPTO_set_id_callback(___func) \
	AROS_LC1(void, CRYPTO_set_id_callback, \
	AROS_LCA(unsigned long (*)(void), (___func), A0), \
	struct Library *, AMISSL_BASE_NAME, 548, Amissl)

#define CRYPTO_get_id_callback() \
	AROS_LC0(unsigned long *, CRYPTO_get_id_callback, \
	struct Library *, AMISSL_BASE_NAME, 549, Amissl)

#define CRYPTO_thread_id() \
	AROS_LC0(unsigned long, CRYPTO_thread_id, \
	struct Library *, AMISSL_BASE_NAME, 550, Amissl)

#define CRYPTO_get_lock_name(___type) \
	AROS_LC1(const char *, CRYPTO_get_lock_name, \
	AROS_LCA(int, (___type), D0), \
	struct Library *, AMISSL_BASE_NAME, 551, Amissl)

#define CRYPTO_add_lock(___pointer, ___amount, ___type, ___file, ___line) \
	AROS_LC5(int, CRYPTO_add_lock, \
	AROS_LCA(int *, (___pointer), A0), \
	AROS_LCA(int, (___amount), D0), \
	AROS_LCA(int, (___type), D1), \
	AROS_LCA(const char *, (___file), A1), \
	AROS_LCA(int, (___line), D2), \
	struct Library *, AMISSL_BASE_NAME, 552, Amissl)

#define CRYPTO_get_new_dynlockid() \
	AROS_LC0(int, CRYPTO_get_new_dynlockid, \
	struct Library *, AMISSL_BASE_NAME, 553, Amissl)

#define CRYPTO_destroy_dynlockid(___i) \
	AROS_LC1(void, CRYPTO_destroy_dynlockid, \
	AROS_LCA(int, (___i), D0), \
	struct Library *, AMISSL_BASE_NAME, 554, Amissl)

#define CRYPTO_get_dynlock_value(___i) \
	AROS_LC1(struct CRYPTO_dynlock_value *, CRYPTO_get_dynlock_value, \
	AROS_LCA(int, (___i), D0), \
	struct Library *, AMISSL_BASE_NAME, 555, Amissl)

#define CRYPTO_set_dynlock_create_callback(___dyn_create_function) \
	AROS_LC1(void, CRYPTO_set_dynlock_create_callback, \
	AROS_LCA(struct CRYPTO_dynlock_value * (*)(const char *file,int line), (___dyn_create_function), A0), \
	struct Library *, AMISSL_BASE_NAME, 556, Amissl)

#define CRYPTO_set_dynlock_lock_callback(___dyn_lock_function) \
	AROS_LC1(void, CRYPTO_set_dynlock_lock_callback, \
	AROS_LCA(void (*)(int mode,struct CRYPTO_dynlock_value *l,const char *file,int line), (___dyn_lock_function), A0), \
	struct Library *, AMISSL_BASE_NAME, 557, Amissl)

#define CRYPTO_set_dynlock_destroy_callback(___dyn_destroy_function) \
	AROS_LC1(void, CRYPTO_set_dynlock_destroy_callback, \
	AROS_LCA(void (*)(struct CRYPTO_dynlock_value *l,const char *file,int line), (___dyn_destroy_function), A0), \
	struct Library *, AMISSL_BASE_NAME, 558, Amissl)

#define CRYPTO_get_dynlock_create_callback() \
	AROS_LC0(struct CRYPTO_dynlock_value **, CRYPTO_get_dynlock_create_callback, \
	struct Library *, AMISSL_BASE_NAME, 559, Amissl)

#define CRYPTO_get_dynlock_lock_callback() \
	AROS_LC0(void *, CRYPTO_get_dynlock_lock_callback, \
	struct Library *, AMISSL_BASE_NAME, 560, Amissl)

#define CRYPTO_get_dynlock_destroy_callback() \
	AROS_LC0(void *, CRYPTO_get_dynlock_destroy_callback, \
	struct Library *, AMISSL_BASE_NAME, 561, Amissl)

#define CRYPTO_set_mem_functions(___m, ___r, ___f) \
	AROS_LC3(int, CRYPTO_set_mem_functions, \
	AROS_LCA(void * (*)(size_t), (___m), A0), \
	AROS_LCA(void * (*)(void *,size_t), (___r), A1), \
	AROS_LCA(void (*)(void *), (___f), A2), \
	struct Library *, AMISSL_BASE_NAME, 562, Amissl)

#define CRYPTO_set_locked_mem_functions(___m, ___free_func) \
	AROS_LC2(int, CRYPTO_set_locked_mem_functions, \
	AROS_LCA(void * (*)(size_t), (___m), A0), \
	AROS_LCA(void (*)(void *), (___free_func), A1), \
	struct Library *, AMISSL_BASE_NAME, 563, Amissl)

#define CRYPTO_set_mem_ex_functions(___m, ___r, ___f) \
	AROS_LC3(int, CRYPTO_set_mem_ex_functions, \
	AROS_LCA(void * (*)(size_t,const char *,int), (___m), A0), \
	AROS_LCA(void * (*)(void *,size_t,const char *,int), (___r), A1), \
	AROS_LCA(void (*)(void *), (___f), A2), \
	struct Library *, AMISSL_BASE_NAME, 564, Amissl)

#define CRYPTO_set_locked_mem_ex_functions(___m, ___free_func) \
	AROS_LC2(int, CRYPTO_set_locked_mem_ex_functions, \
	AROS_LCA(void * (*)(size_t,const char *,int), (___m), A0), \
	AROS_LCA(void (*)(void *), (___free_func), A1), \
	struct Library *, AMISSL_BASE_NAME, 565, Amissl)

#define CRYPTO_set_mem_debug_functions(___m, ___r, ___f, ___so, ___go) \
	AROS_LC5(int, CRYPTO_set_mem_debug_functions, \
	AROS_LCA(void (*)(void *,int,const char *,int,int), (___m), A0), \
	AROS_LCA(void (*)(void *,void *,int,const char *,int,int), (___r), A1), \
	AROS_LCA(void (*)(void *,int), (___f), A2), \
	AROS_LCA(void (*)(long), (___so), A3), \
	AROS_LCA(long (*)(void), (___go), D0), \
	struct Library *, AMISSL_BASE_NAME, 566, Amissl)

#define CRYPTO_get_mem_functions(___m, ___r, ___f) \
	AROS_LC3(void, CRYPTO_get_mem_functions, \
	AROS_LCA(void * (*)(size_t), (___m), A0), \
	AROS_LCA(void * (*)(void *,size_t), (___r), A1), \
	AROS_LCA(void (*)(void *), (___f), A2), \
	struct Library *, AMISSL_BASE_NAME, 567, Amissl)

#define CRYPTO_get_locked_mem_functions(___m, ___f) \
	AROS_LC2(void, CRYPTO_get_locked_mem_functions, \
	AROS_LCA(void * (*)(size_t), (___m), A0), \
	AROS_LCA(void (*)(void *), (___f), A1), \
	struct Library *, AMISSL_BASE_NAME, 568, Amissl)

#define CRYPTO_get_mem_ex_functions(___m, ___r, ___f) \
	AROS_LC3(void, CRYPTO_get_mem_ex_functions, \
	AROS_LCA(void * (*)(size_t,const char *,int), (___m), A0), \
	AROS_LCA(void * (*)(void *,size_t,const char *,int), (___r), A1), \
	AROS_LCA(void (*)(void *), (___f), A2), \
	struct Library *, AMISSL_BASE_NAME, 569, Amissl)

#define CRYPTO_get_locked_mem_ex_functions(___m, ___f) \
	AROS_LC2(void, CRYPTO_get_locked_mem_ex_functions, \
	AROS_LCA(void * (*)(size_t,const char *,int), (___m), A0), \
	AROS_LCA(void (*)(void *), (___f), A1), \
	struct Library *, AMISSL_BASE_NAME, 570, Amissl)

#define CRYPTO_get_mem_debug_functions(___m, ___r, ___f, ___so, ___go) \
	AROS_LC5(void, CRYPTO_get_mem_debug_functions, \
	AROS_LCA(void (*)(void *,int,const char *,int,int), (___m), A0), \
	AROS_LCA(void (*)(void *,void *,int,const char *,int,int), (___r), A1), \
	AROS_LCA(void (*)(void *,int), (___f), A2), \
	AROS_LCA(void (*)(long), (___so), A3), \
	AROS_LCA(long (*)(void), (___go), D0), \
	struct Library *, AMISSL_BASE_NAME, 571, Amissl)

#define CRYPTO_malloc_locked(___num, ___file, ___line) \
	AROS_LC3(void *, CRYPTO_malloc_locked, \
	AROS_LCA(int, (___num), D0), \
	AROS_LCA(const char *, (___file), A0), \
	AROS_LCA(int, (___line), D1), \
	struct Library *, AMISSL_BASE_NAME, 572, Amissl)

#define CRYPTO_free_locked(___a) \
	AROS_LC1(void, CRYPTO_free_locked, \
	AROS_LCA(void *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 573, Amissl)

#define CRYPTO_malloc(___num, ___file, ___line) \
	AROS_LC3(void *, CRYPTO_malloc, \
	AROS_LCA(int, (___num), D0), \
	AROS_LCA(const char *, (___file), A0), \
	AROS_LCA(int, (___line), D1), \
	struct Library *, AMISSL_BASE_NAME, 574, Amissl)

#define CRYPTO_free(___a) \
	AROS_LC1(void, CRYPTO_free, \
	AROS_LCA(void *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 575, Amissl)

#define CRYPTO_realloc(___addr, ___num, ___file, ___line) \
	AROS_LC4(void *, CRYPTO_realloc, \
	AROS_LCA(void *, (___addr), A0), \
	AROS_LCA(int, (___num), D0), \
	AROS_LCA(const char *, (___file), A1), \
	AROS_LCA(int, (___line), D1), \
	struct Library *, AMISSL_BASE_NAME, 576, Amissl)

#define CRYPTO_realloc_clean(___addr, ___old_num, ___num, ___file, ___line) \
	AROS_LC5(void *, CRYPTO_realloc_clean, \
	AROS_LCA(void *, (___addr), A0), \
	AROS_LCA(int, (___old_num), D0), \
	AROS_LCA(int, (___num), D1), \
	AROS_LCA(const char *, (___file), A1), \
	AROS_LCA(int, (___line), D2), \
	struct Library *, AMISSL_BASE_NAME, 577, Amissl)

#define CRYPTO_remalloc(___addr, ___num, ___file, ___line) \
	AROS_LC4(void *, CRYPTO_remalloc, \
	AROS_LCA(void *, (___addr), A0), \
	AROS_LCA(int, (___num), D0), \
	AROS_LCA(const char *, (___file), A1), \
	AROS_LCA(int, (___line), D1), \
	struct Library *, AMISSL_BASE_NAME, 578, Amissl)

#define OPENSSL_cleanse(___ptr, ___len) \
	AROS_LC2(void, OPENSSL_cleanse, \
	AROS_LCA(void *, (___ptr), A0), \
	AROS_LCA(size_t, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 579, Amissl)

#define CRYPTO_set_mem_debug_options(___bits) \
	AROS_LC1(void, CRYPTO_set_mem_debug_options, \
	AROS_LCA(long, (___bits), D0), \
	struct Library *, AMISSL_BASE_NAME, 580, Amissl)

#define CRYPTO_get_mem_debug_options() \
	AROS_LC0(long, CRYPTO_get_mem_debug_options, \
	struct Library *, AMISSL_BASE_NAME, 581, Amissl)

#define CRYPTO_push_info_(___info, ___file, ___line) \
	AROS_LC3(int, CRYPTO_push_info_, \
	AROS_LCA(const char *, (___info), A0), \
	AROS_LCA(const char *, (___file), A1), \
	AROS_LCA(int, (___line), D0), \
	struct Library *, AMISSL_BASE_NAME, 582, Amissl)

#define CRYPTO_pop_info() \
	AROS_LC0(int, CRYPTO_pop_info, \
	struct Library *, AMISSL_BASE_NAME, 583, Amissl)

#define CRYPTO_remove_all_info() \
	AROS_LC0(int, CRYPTO_remove_all_info, \
	struct Library *, AMISSL_BASE_NAME, 584, Amissl)

#define CRYPTO_dbg_malloc(___addr, ___num, ___file, ___line, ___before_p) \
	AROS_LC5(void, CRYPTO_dbg_malloc, \
	AROS_LCA(void *, (___addr), A0), \
	AROS_LCA(int, (___num), D0), \
	AROS_LCA(const char *, (___file), A1), \
	AROS_LCA(int, (___line), D1), \
	AROS_LCA(int, (___before_p), D2), \
	struct Library *, AMISSL_BASE_NAME, 585, Amissl)

#define CRYPTO_dbg_realloc(___addr1, ___addr2, ___num, ___file, ___line, ___before_p) \
	AROS_LC6(void, CRYPTO_dbg_realloc, \
	AROS_LCA(void *, (___addr1), A0), \
	AROS_LCA(void *, (___addr2), A1), \
	AROS_LCA(int, (___num), D0), \
	AROS_LCA(const char *, (___file), A2), \
	AROS_LCA(int, (___line), D1), \
	AROS_LCA(int, (___before_p), D2), \
	struct Library *, AMISSL_BASE_NAME, 586, Amissl)

#define CRYPTO_dbg_free(___addr, ___before_p) \
	AROS_LC2(void, CRYPTO_dbg_free, \
	AROS_LCA(void *, (___addr), A0), \
	AROS_LCA(int, (___before_p), D0), \
	struct Library *, AMISSL_BASE_NAME, 587, Amissl)

#define CRYPTO_dbg_set_options(___bits) \
	AROS_LC1(void, CRYPTO_dbg_set_options, \
	AROS_LCA(long, (___bits), D0), \
	struct Library *, AMISSL_BASE_NAME, 588, Amissl)

#define CRYPTO_dbg_get_options() \
	AROS_LC0(long, CRYPTO_dbg_get_options, \
	struct Library *, AMISSL_BASE_NAME, 589, Amissl)

#define CRYPTO_mem_leaks(___bio) \
	AROS_LC1(void, CRYPTO_mem_leaks, \
	AROS_LCA(struct bio_st *, (___bio), A0), \
	struct Library *, AMISSL_BASE_NAME, 590, Amissl)

#define CRYPTO_mem_leaks_cb(___cb) \
	AROS_LC1(void, CRYPTO_mem_leaks_cb, \
	AROS_LCA(CRYPTO_MEM_LEAK_CB * (*)(unsigned long,const char *,int,int,void *), (___cb), A0), \
	struct Library *, AMISSL_BASE_NAME, 591, Amissl)

#define OpenSSLDie(___file, ___line, ___assertion) \
	AROS_LC3(void, OpenSSLDie, \
	AROS_LCA(const char *, (___file), A0), \
	AROS_LCA(int, (___line), D0), \
	AROS_LCA(const char *, (___assertion), A1), \
	struct Library *, AMISSL_BASE_NAME, 592, Amissl)

#define ERR_load_CRYPTO_strings() \
	AROS_LC0(void, ERR_load_CRYPTO_strings, \
	struct Library *, AMISSL_BASE_NAME, 593, Amissl)

#define DSO_new() \
	AROS_LC0(DSO *, DSO_new, \
	struct Library *, AMISSL_BASE_NAME, 594, Amissl)

#define DSO_new_method(___method) \
	AROS_LC1(DSO *, DSO_new_method, \
	AROS_LCA(DSO_METHOD *, (___method), A0), \
	struct Library *, AMISSL_BASE_NAME, 595, Amissl)

#define DSO_free(___dso) \
	AROS_LC1(int, DSO_free, \
	AROS_LCA(DSO *, (___dso), A0), \
	struct Library *, AMISSL_BASE_NAME, 596, Amissl)

#define DSO_flags(___dso) \
	AROS_LC1(int, DSO_flags, \
	AROS_LCA(DSO *, (___dso), A0), \
	struct Library *, AMISSL_BASE_NAME, 597, Amissl)

#define DSO_up_ref(___dso) \
	AROS_LC1(int, DSO_up_ref, \
	AROS_LCA(DSO *, (___dso), A0), \
	struct Library *, AMISSL_BASE_NAME, 598, Amissl)

#define DSO_ctrl(___dso, ___cmd, ___larg, ___parg) \
	AROS_LC4(long, DSO_ctrl, \
	AROS_LCA(DSO *, (___dso), A0), \
	AROS_LCA(int, (___cmd), D0), \
	AROS_LCA(long, (___larg), D1), \
	AROS_LCA(void *, (___parg), A1), \
	struct Library *, AMISSL_BASE_NAME, 599, Amissl)

#define DSO_set_name_converter(___dso, ___cb, ___oldcb) \
	AROS_LC3(int, DSO_set_name_converter, \
	AROS_LCA(DSO *, (___dso), A0), \
	AROS_LCA(DSO_NAME_CONVERTER_FUNC, (___cb), D0), \
	AROS_LCA(DSO_NAME_CONVERTER_FUNC *, (___oldcb), A1), \
	struct Library *, AMISSL_BASE_NAME, 600, Amissl)

#define DSO_get_filename(___dso) \
	AROS_LC1(const char *, DSO_get_filename, \
	AROS_LCA(DSO *, (___dso), A0), \
	struct Library *, AMISSL_BASE_NAME, 601, Amissl)

#define DSO_set_filename(___dso, ___filename) \
	AROS_LC2(int, DSO_set_filename, \
	AROS_LCA(DSO *, (___dso), A0), \
	AROS_LCA(const char *, (___filename), A1), \
	struct Library *, AMISSL_BASE_NAME, 602, Amissl)

#define DSO_convert_filename(___dso, ___filename) \
	AROS_LC2(char *, DSO_convert_filename, \
	AROS_LCA(DSO *, (___dso), A0), \
	AROS_LCA(const char *, (___filename), A1), \
	struct Library *, AMISSL_BASE_NAME, 603, Amissl)

#define DSO_get_loaded_filename(___dso) \
	AROS_LC1(const char *, DSO_get_loaded_filename, \
	AROS_LCA(DSO *, (___dso), A0), \
	struct Library *, AMISSL_BASE_NAME, 604, Amissl)

#define DSO_set_default_method(___meth) \
	AROS_LC1(void, DSO_set_default_method, \
	AROS_LCA(DSO_METHOD *, (___meth), A0), \
	struct Library *, AMISSL_BASE_NAME, 605, Amissl)

#define DSO_get_default_method() \
	AROS_LC0(DSO_METHOD *, DSO_get_default_method, \
	struct Library *, AMISSL_BASE_NAME, 606, Amissl)

#define DSO_get_method(___dso) \
	AROS_LC1(DSO_METHOD *, DSO_get_method, \
	AROS_LCA(DSO *, (___dso), A0), \
	struct Library *, AMISSL_BASE_NAME, 607, Amissl)

#define DSO_set_method(___dso, ___meth) \
	AROS_LC2(DSO_METHOD *, DSO_set_method, \
	AROS_LCA(DSO *, (___dso), A0), \
	AROS_LCA(DSO_METHOD *, (___meth), A1), \
	struct Library *, AMISSL_BASE_NAME, 608, Amissl)

#define DSO_load(___dso, ___filename, ___meth, ___flags) \
	AROS_LC4(DSO *, DSO_load, \
	AROS_LCA(DSO *, (___dso), A0), \
	AROS_LCA(const char *, (___filename), A1), \
	AROS_LCA(DSO_METHOD *, (___meth), A2), \
	AROS_LCA(int, (___flags), D0), \
	struct Library *, AMISSL_BASE_NAME, 609, Amissl)

#define DSO_bind_var(___dso, ___symname) \
	AROS_LC2(void *, DSO_bind_var, \
	AROS_LCA(DSO *, (___dso), A0), \
	AROS_LCA(const char *, (___symname), A1), \
	struct Library *, AMISSL_BASE_NAME, 610, Amissl)

#define DSO_bind_func(___dso, ___symname) \
	AROS_LC2(DSO_FUNC_TYPE, DSO_bind_func, \
	AROS_LCA(DSO *, (___dso), A0), \
	AROS_LCA(const char *, (___symname), A1), \
	struct Library *, AMISSL_BASE_NAME, 611, Amissl)

#define DSO_METHOD_openssl() \
	AROS_LC0(DSO_METHOD *, DSO_METHOD_openssl, \
	struct Library *, AMISSL_BASE_NAME, 612, Amissl)

#define DSO_METHOD_null() \
	AROS_LC0(DSO_METHOD *, DSO_METHOD_null, \
	struct Library *, AMISSL_BASE_NAME, 613, Amissl)

#define DSO_METHOD_dlfcn() \
	AROS_LC0(DSO_METHOD *, DSO_METHOD_dlfcn, \
	struct Library *, AMISSL_BASE_NAME, 614, Amissl)

#define DSO_METHOD_dl() \
	AROS_LC0(DSO_METHOD *, DSO_METHOD_dl, \
	struct Library *, AMISSL_BASE_NAME, 615, Amissl)

#define DSO_METHOD_win32() \
	AROS_LC0(DSO_METHOD *, DSO_METHOD_win32, \
	struct Library *, AMISSL_BASE_NAME, 616, Amissl)

#define DSO_METHOD_vms() \
	AROS_LC0(DSO_METHOD *, DSO_METHOD_vms, \
	struct Library *, AMISSL_BASE_NAME, 617, Amissl)

#define ERR_load_DSO_strings() \
	AROS_LC0(void, ERR_load_DSO_strings, \
	struct Library *, AMISSL_BASE_NAME, 618, Amissl)

#define EC_GFp_simple_method() \
	AROS_LC0(const EC_METHOD *, EC_GFp_simple_method, \
	struct Library *, AMISSL_BASE_NAME, 619, Amissl)

#define EC_GFp_mont_method() \
	AROS_LC0(const EC_METHOD *, EC_GFp_mont_method, \
	struct Library *, AMISSL_BASE_NAME, 620, Amissl)

#define EC_GROUP_new(___a) \
	AROS_LC1(EC_GROUP *, EC_GROUP_new, \
	AROS_LCA(const EC_METHOD *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 621, Amissl)

#define EC_GROUP_free(___a) \
	AROS_LC1(void, EC_GROUP_free, \
	AROS_LCA(EC_GROUP *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 622, Amissl)

#define EC_GROUP_clear_free(___a) \
	AROS_LC1(void, EC_GROUP_clear_free, \
	AROS_LCA(EC_GROUP *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 623, Amissl)

#define EC_GROUP_copy(___a, ___b) \
	AROS_LC2(int, EC_GROUP_copy, \
	AROS_LCA(EC_GROUP *, (___a), A0), \
	AROS_LCA(const EC_GROUP *, (___b), A1), \
	struct Library *, AMISSL_BASE_NAME, 624, Amissl)

#define EC_GROUP_method_of(___a) \
	AROS_LC1(const EC_METHOD *, EC_GROUP_method_of, \
	AROS_LCA(const EC_GROUP *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 625, Amissl)

#define EC_GROUP_set_curve_GFp(___a1, ___p, ___a, ___b, ___a2) \
	AROS_LC5(int, EC_GROUP_set_curve_GFp, \
	AROS_LCA(EC_GROUP *, (___a1), A0), \
	AROS_LCA(const BIGNUM *, (___p), A1), \
	AROS_LCA(const BIGNUM *, (___a), A2), \
	AROS_LCA(const BIGNUM *, (___b), A3), \
	AROS_LCA(BN_CTX *, (___a2), D0), \
	struct Library *, AMISSL_BASE_NAME, 626, Amissl)

#define EC_GROUP_get_curve_GFp(___a1, ___p, ___a, ___b, ___a2) \
	AROS_LC5(int, EC_GROUP_get_curve_GFp, \
	AROS_LCA(const EC_GROUP *, (___a1), A0), \
	AROS_LCA(BIGNUM *, (___p), A1), \
	AROS_LCA(BIGNUM *, (___a), A2), \
	AROS_LCA(BIGNUM *, (___b), A3), \
	AROS_LCA(BN_CTX *, (___a2), D0), \
	struct Library *, AMISSL_BASE_NAME, 627, Amissl)

#define EC_GROUP_new_curve_GFp(___p, ___a, ___b, ___a1) \
	AROS_LC4(EC_GROUP *, EC_GROUP_new_curve_GFp, \
	AROS_LCA(const BIGNUM *, (___p), A0), \
	AROS_LCA(const BIGNUM *, (___a), A1), \
	AROS_LCA(const BIGNUM *, (___b), A2), \
	AROS_LCA(BN_CTX *, (___a1), A3), \
	struct Library *, AMISSL_BASE_NAME, 628, Amissl)

#define EC_GROUP_set_generator(___a1, ___generator, ___order, ___cofactor) \
	AROS_LC4(int, EC_GROUP_set_generator, \
	AROS_LCA(EC_GROUP *, (___a1), A0), \
	AROS_LCA(const EC_POINT *, (___generator), A1), \
	AROS_LCA(const BIGNUM *, (___order), A2), \
	AROS_LCA(const BIGNUM *, (___cofactor), A3), \
	struct Library *, AMISSL_BASE_NAME, 629, Amissl)

#define EC_GROUP_get0_generator(___a1) \
	AROS_LC1(EC_POINT *, EC_GROUP_get0_generator, \
	AROS_LCA(const EC_GROUP *, (___a1), A0), \
	struct Library *, AMISSL_BASE_NAME, 630, Amissl)

#define EC_GROUP_get_order(___a1, ___order, ___a2) \
	AROS_LC3(int, EC_GROUP_get_order, \
	AROS_LCA(const EC_GROUP *, (___a1), A0), \
	AROS_LCA(BIGNUM *, (___order), A1), \
	AROS_LCA(BN_CTX *, (___a2), A2), \
	struct Library *, AMISSL_BASE_NAME, 631, Amissl)

#define EC_GROUP_get_cofactor(___a1, ___cofactor, ___a2) \
	AROS_LC3(int, EC_GROUP_get_cofactor, \
	AROS_LCA(const EC_GROUP *, (___a1), A0), \
	AROS_LCA(BIGNUM *, (___cofactor), A1), \
	AROS_LCA(BN_CTX *, (___a2), A2), \
	struct Library *, AMISSL_BASE_NAME, 632, Amissl)

#define EC_POINT_new(___a) \
	AROS_LC1(EC_POINT *, EC_POINT_new, \
	AROS_LCA(const EC_GROUP *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 633, Amissl)

#define EC_POINT_free(___a) \
	AROS_LC1(void, EC_POINT_free, \
	AROS_LCA(EC_POINT *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 634, Amissl)

#define EC_POINT_clear_free(___a) \
	AROS_LC1(void, EC_POINT_clear_free, \
	AROS_LCA(EC_POINT *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 635, Amissl)

#define EC_POINT_copy(___a, ___b) \
	AROS_LC2(int, EC_POINT_copy, \
	AROS_LCA(EC_POINT *, (___a), A0), \
	AROS_LCA(const EC_POINT *, (___b), A1), \
	struct Library *, AMISSL_BASE_NAME, 636, Amissl)

#define EC_POINT_method_of(___a) \
	AROS_LC1(const EC_METHOD *, EC_POINT_method_of, \
	AROS_LCA(const EC_POINT *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 637, Amissl)

#define EC_POINT_set_to_infinity(___a, ___b) \
	AROS_LC2(int, EC_POINT_set_to_infinity, \
	AROS_LCA(const EC_GROUP *, (___a), A0), \
	AROS_LCA(EC_POINT *, (___b), A1), \
	struct Library *, AMISSL_BASE_NAME, 638, Amissl)

#define EC_POINT_set_Jprojective_coordinates_GFp(___a1, ___a2, ___x, ___y, ___z, ___a3) \
	AROS_LC6(int, EC_POINT_set_Jprojective_coordinates_GFp, \
	AROS_LCA(const EC_GROUP *, (___a1), A0), \
	AROS_LCA(EC_POINT *, (___a2), A1), \
	AROS_LCA(const BIGNUM *, (___x), A2), \
	AROS_LCA(const BIGNUM *, (___y), A3), \
	AROS_LCA(const BIGNUM *, (___z), D0), \
	AROS_LCA(BN_CTX *, (___a3), D1), \
	struct Library *, AMISSL_BASE_NAME, 639, Amissl)

#define EC_POINT_get_Jprojective_coordinates_GFp(___a1, ___a2, ___x, ___y, ___z, ___a3) \
	AROS_LC6(int, EC_POINT_get_Jprojective_coordinates_GFp, \
	AROS_LCA(const EC_GROUP *, (___a1), A0), \
	AROS_LCA(const EC_POINT *, (___a2), A1), \
	AROS_LCA(BIGNUM *, (___x), A2), \
	AROS_LCA(BIGNUM *, (___y), A3), \
	AROS_LCA(BIGNUM *, (___z), D0), \
	AROS_LCA(BN_CTX *, (___a3), D1), \
	struct Library *, AMISSL_BASE_NAME, 640, Amissl)

#define EC_POINT_set_affine_coordinates_GFp(___a1, ___a2, ___x, ___y, ___a3) \
	AROS_LC5(int, EC_POINT_set_affine_coordinates_GFp, \
	AROS_LCA(const EC_GROUP *, (___a1), A0), \
	AROS_LCA(EC_POINT *, (___a2), A1), \
	AROS_LCA(const BIGNUM *, (___x), A2), \
	AROS_LCA(const BIGNUM *, (___y), A3), \
	AROS_LCA(BN_CTX *, (___a3), D0), \
	struct Library *, AMISSL_BASE_NAME, 641, Amissl)

#define EC_POINT_get_affine_coordinates_GFp(___a1, ___a2, ___x, ___y, ___a3) \
	AROS_LC5(int, EC_POINT_get_affine_coordinates_GFp, \
	AROS_LCA(const EC_GROUP *, (___a1), A0), \
	AROS_LCA(const EC_POINT *, (___a2), A1), \
	AROS_LCA(BIGNUM *, (___x), A2), \
	AROS_LCA(BIGNUM *, (___y), A3), \
	AROS_LCA(BN_CTX *, (___a3), D0), \
	struct Library *, AMISSL_BASE_NAME, 642, Amissl)

#define EC_POINT_set_compressed_coordinates_GFp(___a1, ___a2, ___x, ___y_bit, ___a3) \
	AROS_LC5(int, EC_POINT_set_compressed_coordinates_GFp, \
	AROS_LCA(const EC_GROUP *, (___a1), A0), \
	AROS_LCA(EC_POINT *, (___a2), A1), \
	AROS_LCA(const BIGNUM *, (___x), A2), \
	AROS_LCA(int, (___y_bit), D0), \
	AROS_LCA(BN_CTX *, (___a3), A3), \
	struct Library *, AMISSL_BASE_NAME, 643, Amissl)

#define EC_POINT_point2oct(___a1, ___a2, ___form, ___buf, ___len, ___a3) \
	AROS_LC6(size_t, EC_POINT_point2oct, \
	AROS_LCA(const EC_GROUP *, (___a1), A0), \
	AROS_LCA(const EC_POINT *, (___a2), A1), \
	AROS_LCA(point_conversion_form_t, (___form), D0), \
	AROS_LCA(unsigned char *, (___buf), A2), \
	AROS_LCA(size_t, (___len), D1), \
	AROS_LCA(BN_CTX *, (___a3), A3), \
	struct Library *, AMISSL_BASE_NAME, 644, Amissl)

#define EC_POINT_oct2point(___a1, ___a2, ___buf, ___len, ___a3) \
	AROS_LC5(int, EC_POINT_oct2point, \
	AROS_LCA(const EC_GROUP *, (___a1), A0), \
	AROS_LCA(EC_POINT *, (___a2), A1), \
	AROS_LCA(const unsigned char *, (___buf), A2), \
	AROS_LCA(size_t, (___len), D0), \
	AROS_LCA(BN_CTX *, (___a3), A3), \
	struct Library *, AMISSL_BASE_NAME, 645, Amissl)

#define EC_POINT_add(___a1, ___r, ___a, ___b, ___a2) \
	AROS_LC5(int, EC_POINT_add, \
	AROS_LCA(const EC_GROUP *, (___a1), A0), \
	AROS_LCA(EC_POINT *, (___r), A1), \
	AROS_LCA(const EC_POINT *, (___a), A2), \
	AROS_LCA(const EC_POINT *, (___b), A3), \
	AROS_LCA(BN_CTX *, (___a2), D0), \
	struct Library *, AMISSL_BASE_NAME, 646, Amissl)

#define EC_POINT_dbl(___a1, ___r, ___a, ___a2) \
	AROS_LC4(int, EC_POINT_dbl, \
	AROS_LCA(const EC_GROUP *, (___a1), A0), \
	AROS_LCA(EC_POINT *, (___r), A1), \
	AROS_LCA(const EC_POINT *, (___a), A2), \
	AROS_LCA(BN_CTX *, (___a2), A3), \
	struct Library *, AMISSL_BASE_NAME, 647, Amissl)

#define EC_POINT_invert(___a1, ___a2, ___a3) \
	AROS_LC3(int, EC_POINT_invert, \
	AROS_LCA(const EC_GROUP *, (___a1), A0), \
	AROS_LCA(EC_POINT *, (___a2), A1), \
	AROS_LCA(BN_CTX *, (___a3), A2), \
	struct Library *, AMISSL_BASE_NAME, 648, Amissl)

#define EC_POINT_is_at_infinity(___a, ___b) \
	AROS_LC2(int, EC_POINT_is_at_infinity, \
	AROS_LCA(const EC_GROUP *, (___a), A0), \
	AROS_LCA(const EC_POINT *, (___b), A1), \
	struct Library *, AMISSL_BASE_NAME, 649, Amissl)

#define EC_POINT_is_on_curve(___a, ___b, ___c) \
	AROS_LC3(int, EC_POINT_is_on_curve, \
	AROS_LCA(const EC_GROUP *, (___a), A0), \
	AROS_LCA(const EC_POINT *, (___b), A1), \
	AROS_LCA(BN_CTX *, (___c), A2), \
	struct Library *, AMISSL_BASE_NAME, 650, Amissl)

#define EC_POINT_cmp(___a1, ___a, ___b, ___a2) \
	AROS_LC4(int, EC_POINT_cmp, \
	AROS_LCA(const EC_GROUP *, (___a1), A0), \
	AROS_LCA(const EC_POINT *, (___a), A1), \
	AROS_LCA(const EC_POINT *, (___b), A2), \
	AROS_LCA(BN_CTX *, (___a2), A3), \
	struct Library *, AMISSL_BASE_NAME, 651, Amissl)

#define EC_POINT_make_affine(___a, ___b, ___c) \
	AROS_LC3(int, EC_POINT_make_affine, \
	AROS_LCA(const EC_GROUP *, (___a), A0), \
	AROS_LCA(EC_POINT *, (___b), A1), \
	AROS_LCA(BN_CTX *, (___c), A2), \
	struct Library *, AMISSL_BASE_NAME, 652, Amissl)

#define EC_POINTs_make_affine(___a1, ___num, ___a2, ___a3) \
	AROS_LC4(int, EC_POINTs_make_affine, \
	AROS_LCA(const EC_GROUP *, (___a1), A0), \
	AROS_LCA(size_t, (___num), D0), \
	AROS_LCA(EC_POINT **, (___a2), A1), \
	AROS_LCA(BN_CTX *, (___a3), A2), \
	struct Library *, AMISSL_BASE_NAME, 653, Amissl)

#define EC_POINTs_mul(___a1, ___r, ___a2, ___num, ___a3, ___a4, ___a5) \
	AROS_LC7(int, EC_POINTs_mul, \
	AROS_LCA(const EC_GROUP *, (___a1), A0), \
	AROS_LCA(EC_POINT *, (___r), A1), \
	AROS_LCA(const BIGNUM *, (___a2), A2), \
	AROS_LCA(size_t, (___num), D0), \
	AROS_LCA(const EC_POINT **, (___a3), A3), \
	AROS_LCA(const BIGNUM **, (___a4), D1), \
	AROS_LCA(BN_CTX *, (___a5), D2), \
	struct Library *, AMISSL_BASE_NAME, 654, Amissl)

#define EC_POINT_mul(___a1, ___r, ___a2, ___a3, ___a4, ___a5) \
	AROS_LC6(int, EC_POINT_mul, \
	AROS_LCA(const EC_GROUP *, (___a1), A0), \
	AROS_LCA(EC_POINT *, (___r), A1), \
	AROS_LCA(const BIGNUM *, (___a2), A2), \
	AROS_LCA(const EC_POINT *, (___a3), A3), \
	AROS_LCA(const BIGNUM *, (___a4), D0), \
	AROS_LCA(BN_CTX *, (___a5), D1), \
	struct Library *, AMISSL_BASE_NAME, 655, Amissl)

#define EC_GROUP_precompute_mult(___a, ___b) \
	AROS_LC2(int, EC_GROUP_precompute_mult, \
	AROS_LCA(EC_GROUP *, (___a), A0), \
	AROS_LCA(BN_CTX *, (___b), A1), \
	struct Library *, AMISSL_BASE_NAME, 656, Amissl)

#define ERR_load_EC_strings() \
	AROS_LC0(void, ERR_load_EC_strings, \
	struct Library *, AMISSL_BASE_NAME, 657, Amissl)

#define ERR_put_error(___lib, ___func, ___reason, ___file, ___line) \
	AROS_LC5(void, ERR_put_error, \
	AROS_LCA(int, (___lib), D0), \
	AROS_LCA(int, (___func), D1), \
	AROS_LCA(int, (___reason), D2), \
	AROS_LCA(const char *, (___file), A0), \
	AROS_LCA(int, (___line), D3), \
	struct Library *, AMISSL_BASE_NAME, 658, Amissl)

#define ERR_set_error_data(___data, ___flags) \
	AROS_LC2(void, ERR_set_error_data, \
	AROS_LCA(char *, (___data), A0), \
	AROS_LCA(int, (___flags), D0), \
	struct Library *, AMISSL_BASE_NAME, 659, Amissl)

#define ERR_get_error() \
	AROS_LC0(unsigned long, ERR_get_error, \
	struct Library *, AMISSL_BASE_NAME, 660, Amissl)

#define ERR_get_error_line(___file, ___line) \
	AROS_LC2(unsigned long, ERR_get_error_line, \
	AROS_LCA(const char **, (___file), A0), \
	AROS_LCA(int *, (___line), A1), \
	struct Library *, AMISSL_BASE_NAME, 661, Amissl)

#define ERR_get_error_line_data(___file, ___line, ___data, ___flags) \
	AROS_LC4(unsigned long, ERR_get_error_line_data, \
	AROS_LCA(const char **, (___file), A0), \
	AROS_LCA(int *, (___line), A1), \
	AROS_LCA(const char **, (___data), A2), \
	AROS_LCA(int *, (___flags), A3), \
	struct Library *, AMISSL_BASE_NAME, 662, Amissl)

#define ERR_peek_error() \
	AROS_LC0(unsigned long, ERR_peek_error, \
	struct Library *, AMISSL_BASE_NAME, 663, Amissl)

#define ERR_peek_error_line(___file, ___line) \
	AROS_LC2(unsigned long, ERR_peek_error_line, \
	AROS_LCA(const char **, (___file), A0), \
	AROS_LCA(int *, (___line), A1), \
	struct Library *, AMISSL_BASE_NAME, 664, Amissl)

#define ERR_peek_error_line_data(___file, ___line, ___data, ___flags) \
	AROS_LC4(unsigned long, ERR_peek_error_line_data, \
	AROS_LCA(const char **, (___file), A0), \
	AROS_LCA(int *, (___line), A1), \
	AROS_LCA(const char **, (___data), A2), \
	AROS_LCA(int *, (___flags), A3), \
	struct Library *, AMISSL_BASE_NAME, 665, Amissl)

#define ERR_peek_last_error() \
	AROS_LC0(unsigned long, ERR_peek_last_error, \
	struct Library *, AMISSL_BASE_NAME, 666, Amissl)

#define ERR_peek_last_error_line(___file, ___line) \
	AROS_LC2(unsigned long, ERR_peek_last_error_line, \
	AROS_LCA(const char **, (___file), A0), \
	AROS_LCA(int *, (___line), A1), \
	struct Library *, AMISSL_BASE_NAME, 667, Amissl)

#define ERR_peek_last_error_line_data(___file, ___line, ___data, ___flags) \
	AROS_LC4(unsigned long, ERR_peek_last_error_line_data, \
	AROS_LCA(const char **, (___file), A0), \
	AROS_LCA(int *, (___line), A1), \
	AROS_LCA(const char **, (___data), A2), \
	AROS_LCA(int *, (___flags), A3), \
	struct Library *, AMISSL_BASE_NAME, 668, Amissl)

#define ERR_clear_error() \
	AROS_LC0(void, ERR_clear_error, \
	struct Library *, AMISSL_BASE_NAME, 669, Amissl)

#define ERR_error_string(___e, ___buf) \
	AROS_LC2(char *, ERR_error_string, \
	AROS_LCA(unsigned long, (___e), D0), \
	AROS_LCA(char *, (___buf), A0), \
	struct Library *, AMISSL_BASE_NAME, 670, Amissl)

#define ERR_error_string_n(___e, ___buf, ___len) \
	AROS_LC3(void, ERR_error_string_n, \
	AROS_LCA(unsigned long, (___e), D0), \
	AROS_LCA(char *, (___buf), A0), \
	AROS_LCA(size_t, (___len), D1), \
	struct Library *, AMISSL_BASE_NAME, 671, Amissl)

#define ERR_lib_error_string(___e) \
	AROS_LC1(const char *, ERR_lib_error_string, \
	AROS_LCA(unsigned long, (___e), D0), \
	struct Library *, AMISSL_BASE_NAME, 672, Amissl)

#define ERR_func_error_string(___e) \
	AROS_LC1(const char *, ERR_func_error_string, \
	AROS_LCA(unsigned long, (___e), D0), \
	struct Library *, AMISSL_BASE_NAME, 673, Amissl)

#define ERR_reason_error_string(___e) \
	AROS_LC1(const char *, ERR_reason_error_string, \
	AROS_LCA(unsigned long, (___e), D0), \
	struct Library *, AMISSL_BASE_NAME, 674, Amissl)

#define ERR_print_errors_cb(___cb, ___u) \
	AROS_LC2(void, ERR_print_errors_cb, \
	AROS_LCA(int (*)(const char *str,size_t len,void *u), (___cb), A0), \
	AROS_LCA(void *, (___u), A1), \
	struct Library *, AMISSL_BASE_NAME, 675, Amissl)

#define ERR_print_errors(___bp) \
	AROS_LC1(void, ERR_print_errors, \
	AROS_LCA(BIO *, (___bp), A0), \
	struct Library *, AMISSL_BASE_NAME, 676, Amissl)

#define ERR_add_error_dataA(___num, ___args) \
	AROS_LC2(void, ERR_add_error_dataA, \
	AROS_LCA(int, (___num), D0), \
	AROS_LCA(long *, (___args), D1), \
	struct Library *, AMISSL_BASE_NAME, 677, Amissl)

#ifndef NO_INLINE_VARARGS
#define ERR_add_error_data(___num, ___dummy, ...) \
	({_sfdc_vararg _message[] = { ___dummy, __VA_ARGS__ }; ERR_add_error_dataA((___num), (long *) _message); })
#endif /* !NO_INLINE_VARARGS */

#define ERR_load_strings(___lib, ___str) \
	AROS_LC2(void, ERR_load_strings, \
	AROS_LCA(int, (___lib), D0), \
	AROS_LCA(ERR_STRING_DATA *, (___str), A0), \
	struct Library *, AMISSL_BASE_NAME, 678, Amissl)

#define ERR_unload_strings(___lib, ___str) \
	AROS_LC2(void, ERR_unload_strings, \
	AROS_LCA(int, (___lib), D0), \
	AROS_LCA(ERR_STRING_DATA *, (___str), A0), \
	struct Library *, AMISSL_BASE_NAME, 679, Amissl)

#define ERR_load_ERR_strings() \
	AROS_LC0(void, ERR_load_ERR_strings, \
	struct Library *, AMISSL_BASE_NAME, 680, Amissl)

#define ERR_load_crypto_strings() \
	AROS_LC0(void, ERR_load_crypto_strings, \
	struct Library *, AMISSL_BASE_NAME, 681, Amissl)

#define ERR_free_strings() \
	AROS_LC0(void, ERR_free_strings, \
	struct Library *, AMISSL_BASE_NAME, 682, Amissl)

#define ERR_remove_state(___pid) \
	AROS_LC1(void, ERR_remove_state, \
	AROS_LCA(unsigned long, (___pid), D0), \
	struct Library *, AMISSL_BASE_NAME, 683, Amissl)

#define ERR_get_state() \
	AROS_LC0(ERR_STATE *, ERR_get_state, \
	struct Library *, AMISSL_BASE_NAME, 684, Amissl)

#define ERR_get_string_table() \
	AROS_LC0(LHASH *, ERR_get_string_table, \
	struct Library *, AMISSL_BASE_NAME, 685, Amissl)

#define ERR_get_err_state_table() \
	AROS_LC0(LHASH *, ERR_get_err_state_table, \
	struct Library *, AMISSL_BASE_NAME, 686, Amissl)

#define ERR_release_err_state_table(___hash) \
	AROS_LC1(void, ERR_release_err_state_table, \
	AROS_LCA(LHASH **, (___hash), A0), \
	struct Library *, AMISSL_BASE_NAME, 687, Amissl)

#define ERR_get_next_error_library() \
	AROS_LC0(int, ERR_get_next_error_library, \
	struct Library *, AMISSL_BASE_NAME, 688, Amissl)

#define ERR_get_implementation() \
	AROS_LC0(const ERR_FNS *, ERR_get_implementation, \
	struct Library *, AMISSL_BASE_NAME, 689, Amissl)

#define ERR_set_implementation(___fns) \
	AROS_LC1(int, ERR_set_implementation, \
	AROS_LCA(const ERR_FNS *, (___fns), A0), \
	struct Library *, AMISSL_BASE_NAME, 690, Amissl)

#define EVP_MD_CTX_init(___ctx) \
	AROS_LC1(void, EVP_MD_CTX_init, \
	AROS_LCA(EVP_MD_CTX *, (___ctx), A0), \
	struct Library *, AMISSL_BASE_NAME, 691, Amissl)

#define EVP_MD_CTX_cleanup(___ctx) \
	AROS_LC1(int, EVP_MD_CTX_cleanup, \
	AROS_LCA(EVP_MD_CTX *, (___ctx), A0), \
	struct Library *, AMISSL_BASE_NAME, 692, Amissl)

#define EVP_MD_CTX_create() \
	AROS_LC0(EVP_MD_CTX *, EVP_MD_CTX_create, \
	struct Library *, AMISSL_BASE_NAME, 693, Amissl)

#define EVP_MD_CTX_destroy(___ctx) \
	AROS_LC1(void, EVP_MD_CTX_destroy, \
	AROS_LCA(EVP_MD_CTX *, (___ctx), A0), \
	struct Library *, AMISSL_BASE_NAME, 694, Amissl)

#define EVP_MD_CTX_copy_ex(___out, ___in) \
	AROS_LC2(int, EVP_MD_CTX_copy_ex, \
	AROS_LCA(EVP_MD_CTX *, (___out), A0), \
	AROS_LCA(const EVP_MD_CTX *, (___in), A1), \
	struct Library *, AMISSL_BASE_NAME, 695, Amissl)

#define EVP_DigestInit_ex(___ctx, ___type, ___impl) \
	AROS_LC3(int, EVP_DigestInit_ex, \
	AROS_LCA(EVP_MD_CTX *, (___ctx), A0), \
	AROS_LCA(const EVP_MD *, (___type), A1), \
	AROS_LCA(ENGINE *, (___impl), A2), \
	struct Library *, AMISSL_BASE_NAME, 696, Amissl)

#define EVP_DigestUpdate(___ctx, ___d, ___cnt) \
	AROS_LC3(int, EVP_DigestUpdate, \
	AROS_LCA(EVP_MD_CTX *, (___ctx), A0), \
	AROS_LCA(const void *, (___d), A1), \
	AROS_LCA(unsigned int, (___cnt), D0), \
	struct Library *, AMISSL_BASE_NAME, 697, Amissl)

#define EVP_DigestFinal_ex(___ctx, ___md, ___s) \
	AROS_LC3(int, EVP_DigestFinal_ex, \
	AROS_LCA(EVP_MD_CTX *, (___ctx), A0), \
	AROS_LCA(unsigned char *, (___md), A1), \
	AROS_LCA(unsigned int *, (___s), A2), \
	struct Library *, AMISSL_BASE_NAME, 698, Amissl)

#define EVP_Digest(___data, ___count, ___md, ___size, ___type, ___impl) \
	AROS_LC6(int, EVP_Digest, \
	AROS_LCA(void *, (___data), A0), \
	AROS_LCA(unsigned int, (___count), D0), \
	AROS_LCA(unsigned char *, (___md), A1), \
	AROS_LCA(unsigned int *, (___size), A2), \
	AROS_LCA(const EVP_MD *, (___type), A3), \
	AROS_LCA(ENGINE *, (___impl), D1), \
	struct Library *, AMISSL_BASE_NAME, 699, Amissl)

#define EVP_MD_CTX_copy(___out, ___in) \
	AROS_LC2(int, EVP_MD_CTX_copy, \
	AROS_LCA(EVP_MD_CTX *, (___out), A0), \
	AROS_LCA(const EVP_MD_CTX *, (___in), A1), \
	struct Library *, AMISSL_BASE_NAME, 700, Amissl)

#define EVP_DigestInit(___ctx, ___type) \
	AROS_LC2(int, EVP_DigestInit, \
	AROS_LCA(EVP_MD_CTX *, (___ctx), A0), \
	AROS_LCA(const EVP_MD *, (___type), A1), \
	struct Library *, AMISSL_BASE_NAME, 701, Amissl)

#define EVP_DigestFinal(___ctx, ___md, ___s) \
	AROS_LC3(int, EVP_DigestFinal, \
	AROS_LCA(EVP_MD_CTX *, (___ctx), A0), \
	AROS_LCA(unsigned char *, (___md), A1), \
	AROS_LCA(unsigned int *, (___s), A2), \
	struct Library *, AMISSL_BASE_NAME, 702, Amissl)

#define EVP_read_pw_string(___buf, ___length, ___prompt, ___verify) \
	AROS_LC4(int, EVP_read_pw_string, \
	AROS_LCA(char *, (___buf), A0), \
	AROS_LCA(int, (___length), D0), \
	AROS_LCA(const char *, (___prompt), A1), \
	AROS_LCA(int, (___verify), D1), \
	struct Library *, AMISSL_BASE_NAME, 703, Amissl)

#define EVP_set_pw_prompt(___prompt) \
	AROS_LC1(void, EVP_set_pw_prompt, \
	AROS_LCA(char *, (___prompt), A0), \
	struct Library *, AMISSL_BASE_NAME, 704, Amissl)

#define EVP_get_pw_prompt() \
	AROS_LC0(char *, EVP_get_pw_prompt, \
	struct Library *, AMISSL_BASE_NAME, 705, Amissl)

#define EVP_BytesToKey(___type, ___md, ___salt, ___data, ___datal, ___count, ___key, ___iv) \
	AROS_LC8(int, EVP_BytesToKey, \
	AROS_LCA(const EVP_CIPHER *, (___type), A0), \
	AROS_LCA(const EVP_MD *, (___md), A1), \
	AROS_LCA(const unsigned char *, (___salt), A2), \
	AROS_LCA(const unsigned char *, (___data), A3), \
	AROS_LCA(int, (___datal), D0), \
	AROS_LCA(int, (___count), D1), \
	AROS_LCA(unsigned char *, (___key), D2), \
	AROS_LCA(unsigned char *, (___iv), D3), \
	struct Library *, AMISSL_BASE_NAME, 706, Amissl)

#define EVP_EncryptInit(___ctx, ___cipher, ___key, ___iv) \
	AROS_LC4(int, EVP_EncryptInit, \
	AROS_LCA(EVP_CIPHER_CTX *, (___ctx), A0), \
	AROS_LCA(const EVP_CIPHER *, (___cipher), A1), \
	AROS_LCA(const unsigned char *, (___key), A2), \
	AROS_LCA(const unsigned char *, (___iv), A3), \
	struct Library *, AMISSL_BASE_NAME, 707, Amissl)

#define EVP_EncryptInit_ex(___ctx, ___cipher, ___impl, ___key, ___iv) \
	AROS_LC5(int, EVP_EncryptInit_ex, \
	AROS_LCA(EVP_CIPHER_CTX *, (___ctx), A0), \
	AROS_LCA(const EVP_CIPHER *, (___cipher), A1), \
	AROS_LCA(ENGINE *, (___impl), A2), \
	AROS_LCA(const unsigned char *, (___key), A3), \
	AROS_LCA(const unsigned char *, (___iv), D0), \
	struct Library *, AMISSL_BASE_NAME, 708, Amissl)

#define EVP_EncryptUpdate(___ctx, ___out, ___outl, ___in, ___inl) \
	AROS_LC5(int, EVP_EncryptUpdate, \
	AROS_LCA(EVP_CIPHER_CTX *, (___ctx), A0), \
	AROS_LCA(unsigned char *, (___out), A1), \
	AROS_LCA(int *, (___outl), A2), \
	AROS_LCA(const unsigned char *, (___in), A3), \
	AROS_LCA(int, (___inl), D0), \
	struct Library *, AMISSL_BASE_NAME, 709, Amissl)

#define EVP_EncryptFinal_ex(___ctx, ___out, ___outl) \
	AROS_LC3(int, EVP_EncryptFinal_ex, \
	AROS_LCA(EVP_CIPHER_CTX *, (___ctx), A0), \
	AROS_LCA(unsigned char *, (___out), A1), \
	AROS_LCA(int *, (___outl), A2), \
	struct Library *, AMISSL_BASE_NAME, 710, Amissl)

#define EVP_EncryptFinal(___ctx, ___out, ___outl) \
	AROS_LC3(int, EVP_EncryptFinal, \
	AROS_LCA(EVP_CIPHER_CTX *, (___ctx), A0), \
	AROS_LCA(unsigned char *, (___out), A1), \
	AROS_LCA(int *, (___outl), A2), \
	struct Library *, AMISSL_BASE_NAME, 711, Amissl)

#define EVP_DecryptInit(___ctx, ___cipher, ___key, ___iv) \
	AROS_LC4(int, EVP_DecryptInit, \
	AROS_LCA(EVP_CIPHER_CTX *, (___ctx), A0), \
	AROS_LCA(const EVP_CIPHER *, (___cipher), A1), \
	AROS_LCA(const unsigned char *, (___key), A2), \
	AROS_LCA(const unsigned char *, (___iv), A3), \
	struct Library *, AMISSL_BASE_NAME, 712, Amissl)

#define EVP_DecryptInit_ex(___ctx, ___cipher, ___impl, ___key, ___iv) \
	AROS_LC5(int, EVP_DecryptInit_ex, \
	AROS_LCA(EVP_CIPHER_CTX *, (___ctx), A0), \
	AROS_LCA(const EVP_CIPHER *, (___cipher), A1), \
	AROS_LCA(ENGINE *, (___impl), A2), \
	AROS_LCA(const unsigned char *, (___key), A3), \
	AROS_LCA(const unsigned char *, (___iv), D0), \
	struct Library *, AMISSL_BASE_NAME, 713, Amissl)

#define EVP_DecryptUpdate(___ctx, ___out, ___outl, ___in, ___inl) \
	AROS_LC5(int, EVP_DecryptUpdate, \
	AROS_LCA(EVP_CIPHER_CTX *, (___ctx), A0), \
	AROS_LCA(unsigned char *, (___out), A1), \
	AROS_LCA(int *, (___outl), A2), \
	AROS_LCA(const unsigned char *, (___in), A3), \
	AROS_LCA(int, (___inl), D0), \
	struct Library *, AMISSL_BASE_NAME, 714, Amissl)

#define EVP_DecryptFinal(___ctx, ___outm, ___outl) \
	AROS_LC3(int, EVP_DecryptFinal, \
	AROS_LCA(EVP_CIPHER_CTX *, (___ctx), A0), \
	AROS_LCA(unsigned char *, (___outm), A1), \
	AROS_LCA(int *, (___outl), A2), \
	struct Library *, AMISSL_BASE_NAME, 715, Amissl)

#define EVP_DecryptFinal_ex(___ctx, ___outm, ___outl) \
	AROS_LC3(int, EVP_DecryptFinal_ex, \
	AROS_LCA(EVP_CIPHER_CTX *, (___ctx), A0), \
	AROS_LCA(unsigned char *, (___outm), A1), \
	AROS_LCA(int *, (___outl), A2), \
	struct Library *, AMISSL_BASE_NAME, 716, Amissl)

#define EVP_CipherInit(___ctx, ___cipher, ___key, ___iv, ___enc) \
	AROS_LC5(int, EVP_CipherInit, \
	AROS_LCA(EVP_CIPHER_CTX *, (___ctx), A0), \
	AROS_LCA(const EVP_CIPHER *, (___cipher), A1), \
	AROS_LCA(const unsigned char *, (___key), A2), \
	AROS_LCA(const unsigned char *, (___iv), A3), \
	AROS_LCA(int, (___enc), D0), \
	struct Library *, AMISSL_BASE_NAME, 717, Amissl)

#define EVP_CipherInit_ex(___ctx, ___cipher, ___impl, ___key, ___iv, ___enc) \
	AROS_LC6(int, EVP_CipherInit_ex, \
	AROS_LCA(EVP_CIPHER_CTX *, (___ctx), A0), \
	AROS_LCA(const EVP_CIPHER *, (___cipher), A1), \
	AROS_LCA(ENGINE *, (___impl), A2), \
	AROS_LCA(const unsigned char *, (___key), A3), \
	AROS_LCA(const unsigned char *, (___iv), D0), \
	AROS_LCA(int, (___enc), D1), \
	struct Library *, AMISSL_BASE_NAME, 718, Amissl)

#define EVP_CipherUpdate(___ctx, ___out, ___outl, ___in, ___inl) \
	AROS_LC5(int, EVP_CipherUpdate, \
	AROS_LCA(EVP_CIPHER_CTX *, (___ctx), A0), \
	AROS_LCA(unsigned char *, (___out), A1), \
	AROS_LCA(int *, (___outl), A2), \
	AROS_LCA(const unsigned char *, (___in), A3), \
	AROS_LCA(int, (___inl), D0), \
	struct Library *, AMISSL_BASE_NAME, 719, Amissl)

#define EVP_CipherFinal(___ctx, ___outm, ___outl) \
	AROS_LC3(int, EVP_CipherFinal, \
	AROS_LCA(EVP_CIPHER_CTX *, (___ctx), A0), \
	AROS_LCA(unsigned char *, (___outm), A1), \
	AROS_LCA(int *, (___outl), A2), \
	struct Library *, AMISSL_BASE_NAME, 720, Amissl)

#define EVP_CipherFinal_ex(___ctx, ___outm, ___outl) \
	AROS_LC3(int, EVP_CipherFinal_ex, \
	AROS_LCA(EVP_CIPHER_CTX *, (___ctx), A0), \
	AROS_LCA(unsigned char *, (___outm), A1), \
	AROS_LCA(int *, (___outl), A2), \
	struct Library *, AMISSL_BASE_NAME, 721, Amissl)

#define EVP_SignFinal(___ctx, ___md, ___s, ___pkey) \
	AROS_LC4(int, EVP_SignFinal, \
	AROS_LCA(EVP_MD_CTX *, (___ctx), A0), \
	AROS_LCA(unsigned char *, (___md), A1), \
	AROS_LCA(unsigned int *, (___s), A2), \
	AROS_LCA(EVP_PKEY *, (___pkey), A3), \
	struct Library *, AMISSL_BASE_NAME, 722, Amissl)

#define EVP_VerifyFinal(___ctx, ___sigbuf, ___siglen, ___pkey) \
	AROS_LC4(int, EVP_VerifyFinal, \
	AROS_LCA(EVP_MD_CTX *, (___ctx), A0), \
	AROS_LCA(unsigned char *, (___sigbuf), A1), \
	AROS_LCA(unsigned int, (___siglen), D0), \
	AROS_LCA(EVP_PKEY *, (___pkey), A2), \
	struct Library *, AMISSL_BASE_NAME, 723, Amissl)

#define EVP_OpenInit(___ctx, ___type, ___ek, ___ekl, ___iv, ___priv) \
	AROS_LC6(int, EVP_OpenInit, \
	AROS_LCA(EVP_CIPHER_CTX *, (___ctx), A0), \
	AROS_LCA(const EVP_CIPHER *, (___type), A1), \
	AROS_LCA(unsigned char *, (___ek), A2), \
	AROS_LCA(int, (___ekl), D0), \
	AROS_LCA(unsigned char *, (___iv), A3), \
	AROS_LCA(EVP_PKEY *, (___priv), D1), \
	struct Library *, AMISSL_BASE_NAME, 724, Amissl)

#define EVP_OpenFinal(___ctx, ___out, ___outl) \
	AROS_LC3(int, EVP_OpenFinal, \
	AROS_LCA(EVP_CIPHER_CTX *, (___ctx), A0), \
	AROS_LCA(unsigned char *, (___out), A1), \
	AROS_LCA(int *, (___outl), A2), \
	struct Library *, AMISSL_BASE_NAME, 725, Amissl)

#define EVP_SealInit(___ctx, ___type, ___ek, ___ekl, ___iv, ___pubk, ___npubk) \
	AROS_LC7(int, EVP_SealInit, \
	AROS_LCA(EVP_CIPHER_CTX *, (___ctx), A0), \
	AROS_LCA(const EVP_CIPHER *, (___type), A1), \
	AROS_LCA(unsigned char **, (___ek), A2), \
	AROS_LCA(int *, (___ekl), A3), \
	AROS_LCA(unsigned char *, (___iv), D0), \
	AROS_LCA(EVP_PKEY **, (___pubk), D1), \
	AROS_LCA(int, (___npubk), D2), \
	struct Library *, AMISSL_BASE_NAME, 726, Amissl)

#define EVP_SealFinal(___ctx, ___out, ___outl) \
	AROS_LC3(int, EVP_SealFinal, \
	AROS_LCA(EVP_CIPHER_CTX *, (___ctx), A0), \
	AROS_LCA(unsigned char *, (___out), A1), \
	AROS_LCA(int *, (___outl), A2), \
	struct Library *, AMISSL_BASE_NAME, 727, Amissl)

#define EVP_EncodeInit(___ctx) \
	AROS_LC1(void, EVP_EncodeInit, \
	AROS_LCA(EVP_ENCODE_CTX *, (___ctx), A0), \
	struct Library *, AMISSL_BASE_NAME, 728, Amissl)

#define EVP_EncodeUpdate(___ctx, ___out, ___outl, ___in, ___inl) \
	AROS_LC5(void, EVP_EncodeUpdate, \
	AROS_LCA(EVP_ENCODE_CTX *, (___ctx), A0), \
	AROS_LCA(unsigned char *, (___out), A1), \
	AROS_LCA(int *, (___outl), A2), \
	AROS_LCA(unsigned char *, (___in), A3), \
	AROS_LCA(int, (___inl), D0), \
	struct Library *, AMISSL_BASE_NAME, 729, Amissl)

#define EVP_EncodeFinal(___ctx, ___out, ___outl) \
	AROS_LC3(void, EVP_EncodeFinal, \
	AROS_LCA(EVP_ENCODE_CTX *, (___ctx), A0), \
	AROS_LCA(unsigned char *, (___out), A1), \
	AROS_LCA(int *, (___outl), A2), \
	struct Library *, AMISSL_BASE_NAME, 730, Amissl)

#define EVP_EncodeBlock(___t, ___f, ___n) \
	AROS_LC3(int, EVP_EncodeBlock, \
	AROS_LCA(unsigned char *, (___t), A0), \
	AROS_LCA(const unsigned char *, (___f), A1), \
	AROS_LCA(int, (___n), D0), \
	struct Library *, AMISSL_BASE_NAME, 731, Amissl)

#define EVP_DecodeInit(___ctx) \
	AROS_LC1(void, EVP_DecodeInit, \
	AROS_LCA(EVP_ENCODE_CTX *, (___ctx), A0), \
	struct Library *, AMISSL_BASE_NAME, 732, Amissl)

#define EVP_DecodeUpdate(___ctx, ___out, ___outl, ___in, ___inl) \
	AROS_LC5(int, EVP_DecodeUpdate, \
	AROS_LCA(EVP_ENCODE_CTX *, (___ctx), A0), \
	AROS_LCA(unsigned char *, (___out), A1), \
	AROS_LCA(int *, (___outl), A2), \
	AROS_LCA(unsigned char *, (___in), A3), \
	AROS_LCA(int, (___inl), D0), \
	struct Library *, AMISSL_BASE_NAME, 733, Amissl)

#define EVP_DecodeFinal(___ctx, ___out, ___outl) \
	AROS_LC3(int, EVP_DecodeFinal, \
	AROS_LCA(EVP_ENCODE_CTX *, (___ctx), A0), \
	AROS_LCA(unsigned char *, (___out), A1), \
	AROS_LCA(int *, (___outl), A2), \
	struct Library *, AMISSL_BASE_NAME, 734, Amissl)

#define EVP_DecodeBlock(___t, ___f, ___n) \
	AROS_LC3(int, EVP_DecodeBlock, \
	AROS_LCA(unsigned char *, (___t), A0), \
	AROS_LCA(const unsigned char *, (___f), A1), \
	AROS_LCA(int, (___n), D0), \
	struct Library *, AMISSL_BASE_NAME, 735, Amissl)

#define EVP_CIPHER_CTX_init(___a) \
	AROS_LC1(void, EVP_CIPHER_CTX_init, \
	AROS_LCA(EVP_CIPHER_CTX *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 736, Amissl)

#define EVP_CIPHER_CTX_cleanup(___a) \
	AROS_LC1(int, EVP_CIPHER_CTX_cleanup, \
	AROS_LCA(EVP_CIPHER_CTX *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 737, Amissl)

#define EVP_CIPHER_CTX_set_key_length(___x, ___keylen) \
	AROS_LC2(int, EVP_CIPHER_CTX_set_key_length, \
	AROS_LCA(EVP_CIPHER_CTX *, (___x), A0), \
	AROS_LCA(int, (___keylen), D0), \
	struct Library *, AMISSL_BASE_NAME, 738, Amissl)

#define EVP_CIPHER_CTX_set_padding(___c, ___pad) \
	AROS_LC2(int, EVP_CIPHER_CTX_set_padding, \
	AROS_LCA(EVP_CIPHER_CTX *, (___c), A0), \
	AROS_LCA(int, (___pad), D0), \
	struct Library *, AMISSL_BASE_NAME, 739, Amissl)

#define EVP_CIPHER_CTX_ctrl(___ctx, ___type, ___arg, ___ptr) \
	AROS_LC4(int, EVP_CIPHER_CTX_ctrl, \
	AROS_LCA(EVP_CIPHER_CTX *, (___ctx), A0), \
	AROS_LCA(int, (___type), D0), \
	AROS_LCA(int, (___arg), D1), \
	AROS_LCA(void *, (___ptr), A1), \
	struct Library *, AMISSL_BASE_NAME, 740, Amissl)

#define BIO_f_md() \
	AROS_LC0(BIO_METHOD *, BIO_f_md, \
	struct Library *, AMISSL_BASE_NAME, 741, Amissl)

#define BIO_f_base64() \
	AROS_LC0(BIO_METHOD *, BIO_f_base64, \
	struct Library *, AMISSL_BASE_NAME, 742, Amissl)

#define BIO_f_cipher() \
	AROS_LC0(BIO_METHOD *, BIO_f_cipher, \
	struct Library *, AMISSL_BASE_NAME, 743, Amissl)

#define BIO_f_reliable() \
	AROS_LC0(BIO_METHOD *, BIO_f_reliable, \
	struct Library *, AMISSL_BASE_NAME, 744, Amissl)

#define BIO_set_cipher(___b, ___c, ___k, ___i, ___enc) \
	AROS_LC5(void, BIO_set_cipher, \
	AROS_LCA(BIO *, (___b), A0), \
	AROS_LCA(const EVP_CIPHER *, (___c), A1), \
	AROS_LCA(unsigned char *, (___k), A2), \
	AROS_LCA(unsigned char *, (___i), A3), \
	AROS_LCA(int, (___enc), D0), \
	struct Library *, AMISSL_BASE_NAME, 745, Amissl)

#define EVP_md_null() \
	AROS_LC0(const EVP_MD *, EVP_md_null, \
	struct Library *, AMISSL_BASE_NAME, 746, Amissl)

#define EVP_md2() \
	AROS_LC0(const EVP_MD *, EVP_md2, \
	struct Library *, AMISSL_BASE_NAME, 747, Amissl)

#define EVP_md4() \
	AROS_LC0(const EVP_MD *, EVP_md4, \
	struct Library *, AMISSL_BASE_NAME, 748, Amissl)

#define EVP_md5() \
	AROS_LC0(const EVP_MD *, EVP_md5, \
	struct Library *, AMISSL_BASE_NAME, 749, Amissl)

#define EVP_sha() \
	AROS_LC0(const EVP_MD *, EVP_sha, \
	struct Library *, AMISSL_BASE_NAME, 750, Amissl)

#define EVP_sha1() \
	AROS_LC0(const EVP_MD *, EVP_sha1, \
	struct Library *, AMISSL_BASE_NAME, 751, Amissl)

#define EVP_dss() \
	AROS_LC0(const EVP_MD *, EVP_dss, \
	struct Library *, AMISSL_BASE_NAME, 752, Amissl)

#define EVP_dss1() \
	AROS_LC0(const EVP_MD *, EVP_dss1, \
	struct Library *, AMISSL_BASE_NAME, 753, Amissl)

#define EVP_mdc2() \
	AROS_LC0(const EVP_MD *, EVP_mdc2, \
	struct Library *, AMISSL_BASE_NAME, 754, Amissl)

#define EVP_ripemd160() \
	AROS_LC0(const EVP_MD *, EVP_ripemd160, \
	struct Library *, AMISSL_BASE_NAME, 755, Amissl)

#define EVP_enc_null() \
	AROS_LC0(const EVP_CIPHER *, EVP_enc_null, \
	struct Library *, AMISSL_BASE_NAME, 756, Amissl)

#define EVP_des_ecb() \
	AROS_LC0(const EVP_CIPHER *, EVP_des_ecb, \
	struct Library *, AMISSL_BASE_NAME, 757, Amissl)

#define EVP_des_ede() \
	AROS_LC0(const EVP_CIPHER *, EVP_des_ede, \
	struct Library *, AMISSL_BASE_NAME, 758, Amissl)

#define EVP_des_ede3() \
	AROS_LC0(const EVP_CIPHER *, EVP_des_ede3, \
	struct Library *, AMISSL_BASE_NAME, 759, Amissl)

#define EVP_des_ede_ecb() \
	AROS_LC0(const EVP_CIPHER *, EVP_des_ede_ecb, \
	struct Library *, AMISSL_BASE_NAME, 760, Amissl)

#define EVP_des_ede3_ecb() \
	AROS_LC0(const EVP_CIPHER *, EVP_des_ede3_ecb, \
	struct Library *, AMISSL_BASE_NAME, 761, Amissl)

#define EVP_des_cfb64() \
	AROS_LC0(const EVP_CIPHER *, EVP_des_cfb64, \
	struct Library *, AMISSL_BASE_NAME, 762, Amissl)

#define EVP_des_cfb1() \
	AROS_LC0(const EVP_CIPHER *, EVP_des_cfb1, \
	struct Library *, AMISSL_BASE_NAME, 763, Amissl)

#define EVP_des_cfb8() \
	AROS_LC0(const EVP_CIPHER *, EVP_des_cfb8, \
	struct Library *, AMISSL_BASE_NAME, 764, Amissl)

#define EVP_des_ede_cfb64() \
	AROS_LC0(const EVP_CIPHER *, EVP_des_ede_cfb64, \
	struct Library *, AMISSL_BASE_NAME, 765, Amissl)

#define EVP_des_ede3_cfb64() \
	AROS_LC0(const EVP_CIPHER *, EVP_des_ede3_cfb64, \
	struct Library *, AMISSL_BASE_NAME, 766, Amissl)

#define EVP_des_ede3_cfb1() \
	AROS_LC0(const EVP_CIPHER *, EVP_des_ede3_cfb1, \
	struct Library *, AMISSL_BASE_NAME, 767, Amissl)

#define EVP_des_ede3_cfb8() \
	AROS_LC0(const EVP_CIPHER *, EVP_des_ede3_cfb8, \
	struct Library *, AMISSL_BASE_NAME, 768, Amissl)

#define EVP_des_ofb() \
	AROS_LC0(const EVP_CIPHER *, EVP_des_ofb, \
	struct Library *, AMISSL_BASE_NAME, 769, Amissl)

#define EVP_des_ede_ofb() \
	AROS_LC0(const EVP_CIPHER *, EVP_des_ede_ofb, \
	struct Library *, AMISSL_BASE_NAME, 770, Amissl)

#define EVP_des_ede3_ofb() \
	AROS_LC0(const EVP_CIPHER *, EVP_des_ede3_ofb, \
	struct Library *, AMISSL_BASE_NAME, 771, Amissl)

#define EVP_des_cbc() \
	AROS_LC0(const EVP_CIPHER *, EVP_des_cbc, \
	struct Library *, AMISSL_BASE_NAME, 772, Amissl)

#define EVP_des_ede_cbc() \
	AROS_LC0(const EVP_CIPHER *, EVP_des_ede_cbc, \
	struct Library *, AMISSL_BASE_NAME, 773, Amissl)

#define EVP_des_ede3_cbc() \
	AROS_LC0(const EVP_CIPHER *, EVP_des_ede3_cbc, \
	struct Library *, AMISSL_BASE_NAME, 774, Amissl)

#define EVP_desx_cbc() \
	AROS_LC0(const EVP_CIPHER *, EVP_desx_cbc, \
	struct Library *, AMISSL_BASE_NAME, 775, Amissl)

#define EVP_rc4() \
	AROS_LC0(const EVP_CIPHER *, EVP_rc4, \
	struct Library *, AMISSL_BASE_NAME, 776, Amissl)

#define EVP_rc4_40() \
	AROS_LC0(const EVP_CIPHER *, EVP_rc4_40, \
	struct Library *, AMISSL_BASE_NAME, 777, Amissl)

#define EVP_idea_ecb() \
	AROS_LC0(const EVP_CIPHER *, EVP_idea_ecb, \
	struct Library *, AMISSL_BASE_NAME, 778, Amissl)

#define EVP_idea_cfb64() \
	AROS_LC0(const EVP_CIPHER *, EVP_idea_cfb64, \
	struct Library *, AMISSL_BASE_NAME, 779, Amissl)

#define EVP_idea_ofb() \
	AROS_LC0(const EVP_CIPHER *, EVP_idea_ofb, \
	struct Library *, AMISSL_BASE_NAME, 780, Amissl)

#define EVP_idea_cbc() \
	AROS_LC0(const EVP_CIPHER *, EVP_idea_cbc, \
	struct Library *, AMISSL_BASE_NAME, 781, Amissl)

#define EVP_rc2_ecb() \
	AROS_LC0(const EVP_CIPHER *, EVP_rc2_ecb, \
	struct Library *, AMISSL_BASE_NAME, 782, Amissl)

#define EVP_rc2_cbc() \
	AROS_LC0(const EVP_CIPHER *, EVP_rc2_cbc, \
	struct Library *, AMISSL_BASE_NAME, 783, Amissl)

#define EVP_rc2_40_cbc() \
	AROS_LC0(const EVP_CIPHER *, EVP_rc2_40_cbc, \
	struct Library *, AMISSL_BASE_NAME, 784, Amissl)

#define EVP_rc2_64_cbc() \
	AROS_LC0(const EVP_CIPHER *, EVP_rc2_64_cbc, \
	struct Library *, AMISSL_BASE_NAME, 785, Amissl)

#define EVP_rc2_cfb64() \
	AROS_LC0(const EVP_CIPHER *, EVP_rc2_cfb64, \
	struct Library *, AMISSL_BASE_NAME, 786, Amissl)

#define EVP_rc2_ofb() \
	AROS_LC0(const EVP_CIPHER *, EVP_rc2_ofb, \
	struct Library *, AMISSL_BASE_NAME, 787, Amissl)

#define EVP_bf_ecb() \
	AROS_LC0(const EVP_CIPHER *, EVP_bf_ecb, \
	struct Library *, AMISSL_BASE_NAME, 788, Amissl)

#define EVP_bf_cbc() \
	AROS_LC0(const EVP_CIPHER *, EVP_bf_cbc, \
	struct Library *, AMISSL_BASE_NAME, 789, Amissl)

#define EVP_bf_cfb64() \
	AROS_LC0(const EVP_CIPHER *, EVP_bf_cfb64, \
	struct Library *, AMISSL_BASE_NAME, 790, Amissl)

#define EVP_bf_ofb() \
	AROS_LC0(const EVP_CIPHER *, EVP_bf_ofb, \
	struct Library *, AMISSL_BASE_NAME, 791, Amissl)

#define EVP_cast5_ecb() \
	AROS_LC0(const EVP_CIPHER *, EVP_cast5_ecb, \
	struct Library *, AMISSL_BASE_NAME, 792, Amissl)

#define EVP_cast5_cbc() \
	AROS_LC0(const EVP_CIPHER *, EVP_cast5_cbc, \
	struct Library *, AMISSL_BASE_NAME, 793, Amissl)

#define EVP_cast5_cfb64() \
	AROS_LC0(const EVP_CIPHER *, EVP_cast5_cfb64, \
	struct Library *, AMISSL_BASE_NAME, 794, Amissl)

#define EVP_cast5_ofb() \
	AROS_LC0(const EVP_CIPHER *, EVP_cast5_ofb, \
	struct Library *, AMISSL_BASE_NAME, 795, Amissl)

#define EVP_rc5_32_12_16_cbc() \
	AROS_LC0(const EVP_CIPHER *, EVP_rc5_32_12_16_cbc, \
	struct Library *, AMISSL_BASE_NAME, 796, Amissl)

#define EVP_rc5_32_12_16_ecb() \
	AROS_LC0(const EVP_CIPHER *, EVP_rc5_32_12_16_ecb, \
	struct Library *, AMISSL_BASE_NAME, 797, Amissl)

#define EVP_rc5_32_12_16_cfb64() \
	AROS_LC0(const EVP_CIPHER *, EVP_rc5_32_12_16_cfb64, \
	struct Library *, AMISSL_BASE_NAME, 798, Amissl)

#define EVP_rc5_32_12_16_ofb() \
	AROS_LC0(const EVP_CIPHER *, EVP_rc5_32_12_16_ofb, \
	struct Library *, AMISSL_BASE_NAME, 799, Amissl)

#define EVP_aes_128_ecb() \
	AROS_LC0(const EVP_CIPHER *, EVP_aes_128_ecb, \
	struct Library *, AMISSL_BASE_NAME, 800, Amissl)

#define EVP_aes_128_cbc() \
	AROS_LC0(const EVP_CIPHER *, EVP_aes_128_cbc, \
	struct Library *, AMISSL_BASE_NAME, 801, Amissl)

#define EVP_aes_128_cfb1() \
	AROS_LC0(const EVP_CIPHER *, EVP_aes_128_cfb1, \
	struct Library *, AMISSL_BASE_NAME, 802, Amissl)

#define EVP_aes_128_cfb8() \
	AROS_LC0(const EVP_CIPHER *, EVP_aes_128_cfb8, \
	struct Library *, AMISSL_BASE_NAME, 803, Amissl)

#define EVP_aes_128_cfb128() \
	AROS_LC0(const EVP_CIPHER *, EVP_aes_128_cfb128, \
	struct Library *, AMISSL_BASE_NAME, 804, Amissl)

#define EVP_aes_128_ofb() \
	AROS_LC0(const EVP_CIPHER *, EVP_aes_128_ofb, \
	struct Library *, AMISSL_BASE_NAME, 805, Amissl)

#define EVP_aes_192_ecb() \
	AROS_LC0(const EVP_CIPHER *, EVP_aes_192_ecb, \
	struct Library *, AMISSL_BASE_NAME, 806, Amissl)

#define EVP_aes_192_cbc() \
	AROS_LC0(const EVP_CIPHER *, EVP_aes_192_cbc, \
	struct Library *, AMISSL_BASE_NAME, 807, Amissl)

#define EVP_aes_192_cfb1() \
	AROS_LC0(const EVP_CIPHER *, EVP_aes_192_cfb1, \
	struct Library *, AMISSL_BASE_NAME, 808, Amissl)

#define EVP_aes_192_cfb8() \
	AROS_LC0(const EVP_CIPHER *, EVP_aes_192_cfb8, \
	struct Library *, AMISSL_BASE_NAME, 809, Amissl)

#define EVP_aes_192_cfb128() \
	AROS_LC0(const EVP_CIPHER *, EVP_aes_192_cfb128, \
	struct Library *, AMISSL_BASE_NAME, 810, Amissl)

#define EVP_aes_192_ofb() \
	AROS_LC0(const EVP_CIPHER *, EVP_aes_192_ofb, \
	struct Library *, AMISSL_BASE_NAME, 811, Amissl)

#define EVP_aes_256_ecb() \
	AROS_LC0(const EVP_CIPHER *, EVP_aes_256_ecb, \
	struct Library *, AMISSL_BASE_NAME, 812, Amissl)

#define EVP_aes_256_cbc() \
	AROS_LC0(const EVP_CIPHER *, EVP_aes_256_cbc, \
	struct Library *, AMISSL_BASE_NAME, 813, Amissl)

#define EVP_aes_256_cfb1() \
	AROS_LC0(const EVP_CIPHER *, EVP_aes_256_cfb1, \
	struct Library *, AMISSL_BASE_NAME, 814, Amissl)

#define EVP_aes_256_cfb8() \
	AROS_LC0(const EVP_CIPHER *, EVP_aes_256_cfb8, \
	struct Library *, AMISSL_BASE_NAME, 815, Amissl)

#define EVP_aes_256_cfb128() \
	AROS_LC0(const EVP_CIPHER *, EVP_aes_256_cfb128, \
	struct Library *, AMISSL_BASE_NAME, 816, Amissl)

#define EVP_aes_256_ofb() \
	AROS_LC0(const EVP_CIPHER *, EVP_aes_256_ofb, \
	struct Library *, AMISSL_BASE_NAME, 817, Amissl)

#define OPENSSL_add_all_algorithms_noconf() \
	AROS_LC0(void, OPENSSL_add_all_algorithms_noconf, \
	struct Library *, AMISSL_BASE_NAME, 818, Amissl)

#define OPENSSL_add_all_algorithms_conf() \
	AROS_LC0(void, OPENSSL_add_all_algorithms_conf, \
	struct Library *, AMISSL_BASE_NAME, 819, Amissl)

#define OpenSSL_add_all_ciphers() \
	AROS_LC0(void, OpenSSL_add_all_ciphers, \
	struct Library *, AMISSL_BASE_NAME, 820, Amissl)

#define OpenSSL_add_all_digests() \
	AROS_LC0(void, OpenSSL_add_all_digests, \
	struct Library *, AMISSL_BASE_NAME, 821, Amissl)

#define EVP_add_cipher(___cipher) \
	AROS_LC1(int, EVP_add_cipher, \
	AROS_LCA(const EVP_CIPHER *, (___cipher), A0), \
	struct Library *, AMISSL_BASE_NAME, 822, Amissl)

#define EVP_add_digest(___digest) \
	AROS_LC1(int, EVP_add_digest, \
	AROS_LCA(const EVP_MD *, (___digest), A0), \
	struct Library *, AMISSL_BASE_NAME, 823, Amissl)

#define EVP_get_cipherbyname(___name) \
	AROS_LC1(const EVP_CIPHER *, EVP_get_cipherbyname, \
	AROS_LCA(const char *, (___name), A0), \
	struct Library *, AMISSL_BASE_NAME, 824, Amissl)

#define EVP_get_digestbyname(___name) \
	AROS_LC1(const EVP_MD *, EVP_get_digestbyname, \
	AROS_LCA(const char *, (___name), A0), \
	struct Library *, AMISSL_BASE_NAME, 825, Amissl)

#define EVP_cleanup() \
	AROS_LC0(void, EVP_cleanup, \
	struct Library *, AMISSL_BASE_NAME, 826, Amissl)

#define EVP_PKEY_decrypt(___dec_key, ___enc_key, ___enc_key_len, ___private_key) \
	AROS_LC4(int, EVP_PKEY_decrypt, \
	AROS_LCA(unsigned char *, (___dec_key), A0), \
	AROS_LCA(unsigned char *, (___enc_key), A1), \
	AROS_LCA(int, (___enc_key_len), D0), \
	AROS_LCA(EVP_PKEY *, (___private_key), A2), \
	struct Library *, AMISSL_BASE_NAME, 827, Amissl)

#define EVP_PKEY_encrypt(___enc_key, ___key, ___key_len, ___pub_key) \
	AROS_LC4(int, EVP_PKEY_encrypt, \
	AROS_LCA(unsigned char *, (___enc_key), A0), \
	AROS_LCA(unsigned char *, (___key), A1), \
	AROS_LCA(int, (___key_len), D0), \
	AROS_LCA(EVP_PKEY *, (___pub_key), A2), \
	struct Library *, AMISSL_BASE_NAME, 828, Amissl)

#define EVP_PKEY_type(___type) \
	AROS_LC1(int, EVP_PKEY_type, \
	AROS_LCA(int, (___type), D0), \
	struct Library *, AMISSL_BASE_NAME, 829, Amissl)

#define EVP_PKEY_bits(___pkey) \
	AROS_LC1(int, EVP_PKEY_bits, \
	AROS_LCA(EVP_PKEY *, (___pkey), A0), \
	struct Library *, AMISSL_BASE_NAME, 830, Amissl)

#define EVP_PKEY_size(___pkey) \
	AROS_LC1(int, EVP_PKEY_size, \
	AROS_LCA(EVP_PKEY *, (___pkey), A0), \
	struct Library *, AMISSL_BASE_NAME, 831, Amissl)

#define EVP_PKEY_assign(___pkey, ___type, ___key) \
	AROS_LC3(int, EVP_PKEY_assign, \
	AROS_LCA(EVP_PKEY *, (___pkey), A0), \
	AROS_LCA(int, (___type), D0), \
	AROS_LCA(char *, (___key), A1), \
	struct Library *, AMISSL_BASE_NAME, 832, Amissl)

#define EVP_PKEY_set1_RSA(___pkey, ___key) \
	AROS_LC2(int, EVP_PKEY_set1_RSA, \
	AROS_LCA(EVP_PKEY *, (___pkey), A0), \
	AROS_LCA(struct rsa_st *, (___key), A1), \
	struct Library *, AMISSL_BASE_NAME, 833, Amissl)

#define EVP_PKEY_get1_RSA(___pkey) \
	AROS_LC1(struct rsa_st *, EVP_PKEY_get1_RSA, \
	AROS_LCA(EVP_PKEY *, (___pkey), A0), \
	struct Library *, AMISSL_BASE_NAME, 834, Amissl)

#define EVP_PKEY_set1_DSA(___pkey, ___key) \
	AROS_LC2(int, EVP_PKEY_set1_DSA, \
	AROS_LCA(EVP_PKEY *, (___pkey), A0), \
	AROS_LCA(struct dsa_st *, (___key), A1), \
	struct Library *, AMISSL_BASE_NAME, 835, Amissl)

#define EVP_PKEY_get1_DSA(___pkey) \
	AROS_LC1(struct dsa_st *, EVP_PKEY_get1_DSA, \
	AROS_LCA(EVP_PKEY *, (___pkey), A0), \
	struct Library *, AMISSL_BASE_NAME, 836, Amissl)

#define EVP_PKEY_set1_DH(___pkey, ___key) \
	AROS_LC2(int, EVP_PKEY_set1_DH, \
	AROS_LCA(EVP_PKEY *, (___pkey), A0), \
	AROS_LCA(struct dh_st *, (___key), A1), \
	struct Library *, AMISSL_BASE_NAME, 837, Amissl)

#define EVP_PKEY_get1_DH(___pkey) \
	AROS_LC1(struct dh_st *, EVP_PKEY_get1_DH, \
	AROS_LCA(EVP_PKEY *, (___pkey), A0), \
	struct Library *, AMISSL_BASE_NAME, 838, Amissl)

#define EVP_PKEY_new() \
	AROS_LC0(EVP_PKEY *, EVP_PKEY_new, \
	struct Library *, AMISSL_BASE_NAME, 839, Amissl)

#define EVP_PKEY_free(___pkey) \
	AROS_LC1(void, EVP_PKEY_free, \
	AROS_LCA(EVP_PKEY *, (___pkey), A0), \
	struct Library *, AMISSL_BASE_NAME, 840, Amissl)

#define d2i_PublicKey(___type, ___a, ___pp, ___length) \
	AROS_LC4(EVP_PKEY *, d2i_PublicKey, \
	AROS_LCA(int, (___type), D0), \
	AROS_LCA(EVP_PKEY **, (___a), A0), \
	AROS_LCA(unsigned char **, (___pp), A1), \
	AROS_LCA(long, (___length), D1), \
	struct Library *, AMISSL_BASE_NAME, 841, Amissl)

#define i2d_PublicKey(___a, ___pp) \
	AROS_LC2(int, i2d_PublicKey, \
	AROS_LCA(EVP_PKEY *, (___a), A0), \
	AROS_LCA(unsigned char **, (___pp), A1), \
	struct Library *, AMISSL_BASE_NAME, 842, Amissl)

#define d2i_PrivateKey(___type, ___a, ___pp, ___length) \
	AROS_LC4(EVP_PKEY *, d2i_PrivateKey, \
	AROS_LCA(int, (___type), D0), \
	AROS_LCA(EVP_PKEY **, (___a), A0), \
	AROS_LCA(unsigned char **, (___pp), A1), \
	AROS_LCA(long, (___length), D1), \
	struct Library *, AMISSL_BASE_NAME, 843, Amissl)

#define d2i_AutoPrivateKey(___a, ___pp, ___length) \
	AROS_LC3(EVP_PKEY *, d2i_AutoPrivateKey, \
	AROS_LCA(EVP_PKEY **, (___a), A0), \
	AROS_LCA(unsigned char **, (___pp), A1), \
	AROS_LCA(long, (___length), D0), \
	struct Library *, AMISSL_BASE_NAME, 844, Amissl)

#define i2d_PrivateKey(___a, ___pp) \
	AROS_LC2(int, i2d_PrivateKey, \
	AROS_LCA(EVP_PKEY *, (___a), A0), \
	AROS_LCA(unsigned char **, (___pp), A1), \
	struct Library *, AMISSL_BASE_NAME, 845, Amissl)

#define EVP_PKEY_copy_parameters(___to, ___from) \
	AROS_LC2(int, EVP_PKEY_copy_parameters, \
	AROS_LCA(EVP_PKEY *, (___to), A0), \
	AROS_LCA(EVP_PKEY *, (___from), A1), \
	struct Library *, AMISSL_BASE_NAME, 846, Amissl)

#define EVP_PKEY_missing_parameters(___pkey) \
	AROS_LC1(int, EVP_PKEY_missing_parameters, \
	AROS_LCA(EVP_PKEY *, (___pkey), A0), \
	struct Library *, AMISSL_BASE_NAME, 847, Amissl)

#define EVP_PKEY_save_parameters(___pkey, ___mode) \
	AROS_LC2(int, EVP_PKEY_save_parameters, \
	AROS_LCA(EVP_PKEY *, (___pkey), A0), \
	AROS_LCA(int, (___mode), D0), \
	struct Library *, AMISSL_BASE_NAME, 848, Amissl)

#define EVP_PKEY_cmp_parameters(___a, ___b) \
	AROS_LC2(int, EVP_PKEY_cmp_parameters, \
	AROS_LCA(EVP_PKEY *, (___a), A0), \
	AROS_LCA(EVP_PKEY *, (___b), A1), \
	struct Library *, AMISSL_BASE_NAME, 849, Amissl)

#define EVP_CIPHER_type(___ctx) \
	AROS_LC1(int, EVP_CIPHER_type, \
	AROS_LCA(const EVP_CIPHER *, (___ctx), A0), \
	struct Library *, AMISSL_BASE_NAME, 850, Amissl)

#define EVP_CIPHER_param_to_asn1(___c, ___type) \
	AROS_LC2(int, EVP_CIPHER_param_to_asn1, \
	AROS_LCA(EVP_CIPHER_CTX *, (___c), A0), \
	AROS_LCA(ASN1_TYPE *, (___type), A1), \
	struct Library *, AMISSL_BASE_NAME, 851, Amissl)

#define EVP_CIPHER_asn1_to_param(___c, ___type) \
	AROS_LC2(int, EVP_CIPHER_asn1_to_param, \
	AROS_LCA(EVP_CIPHER_CTX *, (___c), A0), \
	AROS_LCA(ASN1_TYPE *, (___type), A1), \
	struct Library *, AMISSL_BASE_NAME, 852, Amissl)

#define EVP_CIPHER_set_asn1_iv(___c, ___type) \
	AROS_LC2(int, EVP_CIPHER_set_asn1_iv, \
	AROS_LCA(EVP_CIPHER_CTX *, (___c), A0), \
	AROS_LCA(ASN1_TYPE *, (___type), A1), \
	struct Library *, AMISSL_BASE_NAME, 853, Amissl)

#define EVP_CIPHER_get_asn1_iv(___c, ___type) \
	AROS_LC2(int, EVP_CIPHER_get_asn1_iv, \
	AROS_LCA(EVP_CIPHER_CTX *, (___c), A0), \
	AROS_LCA(ASN1_TYPE *, (___type), A1), \
	struct Library *, AMISSL_BASE_NAME, 854, Amissl)

#define PKCS5_PBE_keyivgen(___ctx, ___pass, ___passlen, ___param, ___cipher, ___md, ___en_de) \
	AROS_LC7(int, PKCS5_PBE_keyivgen, \
	AROS_LCA(EVP_CIPHER_CTX *, (___ctx), A0), \
	AROS_LCA(const char *, (___pass), A1), \
	AROS_LCA(int, (___passlen), D0), \
	AROS_LCA(ASN1_TYPE *, (___param), A2), \
	AROS_LCA(const EVP_CIPHER *, (___cipher), A3), \
	AROS_LCA(const EVP_MD *, (___md), D1), \
	AROS_LCA(int, (___en_de), D2), \
	struct Library *, AMISSL_BASE_NAME, 855, Amissl)

#define PKCS5_PBKDF2_HMAC_SHA1(___pass, ___passlen, ___salt, ___saltlen, ___iter, ___keylen, ___out) \
	AROS_LC7(int, PKCS5_PBKDF2_HMAC_SHA1, \
	AROS_LCA(const char *, (___pass), A0), \
	AROS_LCA(int, (___passlen), D0), \
	AROS_LCA(unsigned char *, (___salt), A1), \
	AROS_LCA(int, (___saltlen), D1), \
	AROS_LCA(int, (___iter), D2), \
	AROS_LCA(int, (___keylen), D3), \
	AROS_LCA(unsigned char *, (___out), A2), \
	struct Library *, AMISSL_BASE_NAME, 856, Amissl)

#define PKCS5_v2_PBE_keyivgen(___ctx, ___pass, ___passlen, ___param, ___cipher, ___md, ___en_de) \
	AROS_LC7(int, PKCS5_v2_PBE_keyivgen, \
	AROS_LCA(EVP_CIPHER_CTX *, (___ctx), A0), \
	AROS_LCA(const char *, (___pass), A1), \
	AROS_LCA(int, (___passlen), D0), \
	AROS_LCA(ASN1_TYPE *, (___param), A2), \
	AROS_LCA(const EVP_CIPHER *, (___cipher), A3), \
	AROS_LCA(const EVP_MD *, (___md), D1), \
	AROS_LCA(int, (___en_de), D2), \
	struct Library *, AMISSL_BASE_NAME, 857, Amissl)

#define PKCS5_PBE_add() \
	AROS_LC0(void, PKCS5_PBE_add, \
	struct Library *, AMISSL_BASE_NAME, 858, Amissl)

#define EVP_PBE_CipherInit(___pbe_obj, ___pass, ___passlen, ___param, ___ctx, ___en_de) \
	AROS_LC6(int, EVP_PBE_CipherInit, \
	AROS_LCA(ASN1_OBJECT *, (___pbe_obj), A0), \
	AROS_LCA(const char *, (___pass), A1), \
	AROS_LCA(int, (___passlen), D0), \
	AROS_LCA(ASN1_TYPE *, (___param), A2), \
	AROS_LCA(EVP_CIPHER_CTX *, (___ctx), A3), \
	AROS_LCA(int, (___en_de), D1), \
	struct Library *, AMISSL_BASE_NAME, 859, Amissl)

#define EVP_PBE_alg_add(___nid, ___cipher, ___md, ___keygen) \
	AROS_LC4(int, EVP_PBE_alg_add, \
	AROS_LCA(int, (___nid), D0), \
	AROS_LCA(const EVP_CIPHER *, (___cipher), A0), \
	AROS_LCA(const EVP_MD *, (___md), A1), \
	AROS_LCA(EVP_PBE_KEYGEN * (*)(struct evp_cipher_ctx_st *ctx,const char *pass,int passlen,struct asn1_type_st *param,struct evp_cipher_st *cipher,struct env_md_st *md,int en_de), (___keygen), A2), \
	struct Library *, AMISSL_BASE_NAME, 860, Amissl)

#define EVP_PBE_cleanup() \
	AROS_LC0(void, EVP_PBE_cleanup, \
	struct Library *, AMISSL_BASE_NAME, 861, Amissl)

#define ERR_load_EVP_strings() \
	AROS_LC0(void, ERR_load_EVP_strings, \
	struct Library *, AMISSL_BASE_NAME, 862, Amissl)

#define HMAC_CTX_init(___ctx) \
	AROS_LC1(void, HMAC_CTX_init, \
	AROS_LCA(HMAC_CTX *, (___ctx), A0), \
	struct Library *, AMISSL_BASE_NAME, 863, Amissl)

#define HMAC_CTX_cleanup(___ctx) \
	AROS_LC1(void, HMAC_CTX_cleanup, \
	AROS_LCA(HMAC_CTX *, (___ctx), A0), \
	struct Library *, AMISSL_BASE_NAME, 864, Amissl)

#define HMAC_Init(___ctx, ___key, ___len, ___md) \
	AROS_LC4(void, HMAC_Init, \
	AROS_LCA(HMAC_CTX *, (___ctx), A0), \
	AROS_LCA(const void *, (___key), A1), \
	AROS_LCA(int, (___len), D0), \
	AROS_LCA(const EVP_MD *, (___md), A2), \
	struct Library *, AMISSL_BASE_NAME, 865, Amissl)

#define HMAC_Init_ex(___ctx, ___key, ___len, ___md, ___impl) \
	AROS_LC5(void, HMAC_Init_ex, \
	AROS_LCA(HMAC_CTX *, (___ctx), A0), \
	AROS_LCA(const void *, (___key), A1), \
	AROS_LCA(int, (___len), D0), \
	AROS_LCA(const EVP_MD *, (___md), A2), \
	AROS_LCA(ENGINE *, (___impl), A3), \
	struct Library *, AMISSL_BASE_NAME, 866, Amissl)

#define HMAC_Update(___ctx, ___data, ___len) \
	AROS_LC3(void, HMAC_Update, \
	AROS_LCA(HMAC_CTX *, (___ctx), A0), \
	AROS_LCA(const unsigned char *, (___data), A1), \
	AROS_LCA(int, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 867, Amissl)

#define HMAC_Final(___ctx, ___md, ___len) \
	AROS_LC3(void, HMAC_Final, \
	AROS_LCA(HMAC_CTX *, (___ctx), A0), \
	AROS_LCA(unsigned char *, (___md), A1), \
	AROS_LCA(unsigned int *, (___len), A2), \
	struct Library *, AMISSL_BASE_NAME, 868, Amissl)

#define HMAC(___evp_md, ___key, ___key_len, ___d, ___n, ___md, ___md_len) \
	AROS_LC7(unsigned char *, HMAC, \
	AROS_LCA(const EVP_MD *, (___evp_md), A0), \
	AROS_LCA(const void *, (___key), A1), \
	AROS_LCA(int, (___key_len), D0), \
	AROS_LCA(const unsigned char *, (___d), A2), \
	AROS_LCA(int, (___n), D1), \
	AROS_LCA(unsigned char *, (___md), A3), \
	AROS_LCA(unsigned int *, (___md_len), D2), \
	struct Library *, AMISSL_BASE_NAME, 869, Amissl)

#define KRB5_ENCDATA_new() \
	AROS_LC0(KRB5_ENCDATA *, KRB5_ENCDATA_new, \
	struct Library *, AMISSL_BASE_NAME, 870, Amissl)

#define KRB5_ENCDATA_free(___a) \
	AROS_LC1(void, KRB5_ENCDATA_free, \
	AROS_LCA(KRB5_ENCDATA *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 871, Amissl)

#define d2i_KRB5_ENCDATA(___a, ___in, ___len) \
	AROS_LC3(KRB5_ENCDATA *, d2i_KRB5_ENCDATA, \
	AROS_LCA(KRB5_ENCDATA **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 872, Amissl)

#define i2d_KRB5_ENCDATA(___a, ___out) \
	AROS_LC2(int, i2d_KRB5_ENCDATA, \
	AROS_LCA(KRB5_ENCDATA *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 873, Amissl)

#define KRB5_ENCDATA_it() \
	AROS_LC0(const ASN1_ITEM *, KRB5_ENCDATA_it, \
	struct Library *, AMISSL_BASE_NAME, 874, Amissl)

#define KRB5_PRINCNAME_new() \
	AROS_LC0(KRB5_PRINCNAME *, KRB5_PRINCNAME_new, \
	struct Library *, AMISSL_BASE_NAME, 875, Amissl)

#define KRB5_PRINCNAME_free(___a) \
	AROS_LC1(void, KRB5_PRINCNAME_free, \
	AROS_LCA(KRB5_PRINCNAME *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 876, Amissl)

#define d2i_KRB5_PRINCNAME(___a, ___in, ___len) \
	AROS_LC3(KRB5_PRINCNAME *, d2i_KRB5_PRINCNAME, \
	AROS_LCA(KRB5_PRINCNAME **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 877, Amissl)

#define i2d_KRB5_PRINCNAME(___a, ___out) \
	AROS_LC2(int, i2d_KRB5_PRINCNAME, \
	AROS_LCA(KRB5_PRINCNAME *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 878, Amissl)

#define KRB5_PRINCNAME_it() \
	AROS_LC0(const ASN1_ITEM *, KRB5_PRINCNAME_it, \
	struct Library *, AMISSL_BASE_NAME, 879, Amissl)

#define KRB5_TKTBODY_new() \
	AROS_LC0(KRB5_TKTBODY *, KRB5_TKTBODY_new, \
	struct Library *, AMISSL_BASE_NAME, 880, Amissl)

#define KRB5_TKTBODY_free(___a) \
	AROS_LC1(void, KRB5_TKTBODY_free, \
	AROS_LCA(KRB5_TKTBODY *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 881, Amissl)

#define d2i_KRB5_TKTBODY(___a, ___in, ___len) \
	AROS_LC3(KRB5_TKTBODY *, d2i_KRB5_TKTBODY, \
	AROS_LCA(KRB5_TKTBODY **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 882, Amissl)

#define i2d_KRB5_TKTBODY(___a, ___out) \
	AROS_LC2(int, i2d_KRB5_TKTBODY, \
	AROS_LCA(KRB5_TKTBODY *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 883, Amissl)

#define KRB5_TKTBODY_it() \
	AROS_LC0(const ASN1_ITEM *, KRB5_TKTBODY_it, \
	struct Library *, AMISSL_BASE_NAME, 884, Amissl)

#define KRB5_APREQBODY_new() \
	AROS_LC0(KRB5_APREQBODY *, KRB5_APREQBODY_new, \
	struct Library *, AMISSL_BASE_NAME, 885, Amissl)

#define KRB5_APREQBODY_free(___a) \
	AROS_LC1(void, KRB5_APREQBODY_free, \
	AROS_LCA(KRB5_APREQBODY *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 886, Amissl)

#define d2i_KRB5_APREQBODY(___a, ___in, ___len) \
	AROS_LC3(KRB5_APREQBODY *, d2i_KRB5_APREQBODY, \
	AROS_LCA(KRB5_APREQBODY **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 887, Amissl)

#define i2d_KRB5_APREQBODY(___a, ___out) \
	AROS_LC2(int, i2d_KRB5_APREQBODY, \
	AROS_LCA(KRB5_APREQBODY *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 888, Amissl)

#define KRB5_APREQBODY_it() \
	AROS_LC0(const ASN1_ITEM *, KRB5_APREQBODY_it, \
	struct Library *, AMISSL_BASE_NAME, 889, Amissl)

#define KRB5_TICKET_new() \
	AROS_LC0(KRB5_TICKET *, KRB5_TICKET_new, \
	struct Library *, AMISSL_BASE_NAME, 890, Amissl)

#define KRB5_TICKET_free(___a) \
	AROS_LC1(void, KRB5_TICKET_free, \
	AROS_LCA(KRB5_TICKET *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 891, Amissl)

#define d2i_KRB5_TICKET(___a, ___in, ___len) \
	AROS_LC3(KRB5_TICKET *, d2i_KRB5_TICKET, \
	AROS_LCA(KRB5_TICKET **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 892, Amissl)

#define i2d_KRB5_TICKET(___a, ___out) \
	AROS_LC2(int, i2d_KRB5_TICKET, \
	AROS_LCA(KRB5_TICKET *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 893, Amissl)

#define KRB5_TICKET_it() \
	AROS_LC0(const ASN1_ITEM *, KRB5_TICKET_it, \
	struct Library *, AMISSL_BASE_NAME, 894, Amissl)

#define KRB5_APREQ_new() \
	AROS_LC0(KRB5_APREQ *, KRB5_APREQ_new, \
	struct Library *, AMISSL_BASE_NAME, 895, Amissl)

#define KRB5_APREQ_free(___a) \
	AROS_LC1(void, KRB5_APREQ_free, \
	AROS_LCA(KRB5_APREQ *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 896, Amissl)

#define d2i_KRB5_APREQ(___a, ___in, ___len) \
	AROS_LC3(KRB5_APREQ *, d2i_KRB5_APREQ, \
	AROS_LCA(KRB5_APREQ **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 897, Amissl)

#define i2d_KRB5_APREQ(___a, ___out) \
	AROS_LC2(int, i2d_KRB5_APREQ, \
	AROS_LCA(KRB5_APREQ *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 898, Amissl)

#define KRB5_APREQ_it() \
	AROS_LC0(const ASN1_ITEM *, KRB5_APREQ_it, \
	struct Library *, AMISSL_BASE_NAME, 899, Amissl)

#define KRB5_CHECKSUM_new() \
	AROS_LC0(KRB5_CHECKSUM *, KRB5_CHECKSUM_new, \
	struct Library *, AMISSL_BASE_NAME, 900, Amissl)

#define KRB5_CHECKSUM_free(___a) \
	AROS_LC1(void, KRB5_CHECKSUM_free, \
	AROS_LCA(KRB5_CHECKSUM *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 901, Amissl)

#define d2i_KRB5_CHECKSUM(___a, ___in, ___len) \
	AROS_LC3(KRB5_CHECKSUM *, d2i_KRB5_CHECKSUM, \
	AROS_LCA(KRB5_CHECKSUM **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 902, Amissl)

#define i2d_KRB5_CHECKSUM(___a, ___out) \
	AROS_LC2(int, i2d_KRB5_CHECKSUM, \
	AROS_LCA(KRB5_CHECKSUM *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 903, Amissl)

#define KRB5_CHECKSUM_it() \
	AROS_LC0(const ASN1_ITEM *, KRB5_CHECKSUM_it, \
	struct Library *, AMISSL_BASE_NAME, 904, Amissl)

#define KRB5_ENCKEY_new() \
	AROS_LC0(KRB5_ENCKEY *, KRB5_ENCKEY_new, \
	struct Library *, AMISSL_BASE_NAME, 905, Amissl)

#define KRB5_ENCKEY_free(___a) \
	AROS_LC1(void, KRB5_ENCKEY_free, \
	AROS_LCA(KRB5_ENCKEY *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 906, Amissl)

#define d2i_KRB5_ENCKEY(___a, ___in, ___len) \
	AROS_LC3(KRB5_ENCKEY *, d2i_KRB5_ENCKEY, \
	AROS_LCA(KRB5_ENCKEY **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 907, Amissl)

#define i2d_KRB5_ENCKEY(___a, ___out) \
	AROS_LC2(int, i2d_KRB5_ENCKEY, \
	AROS_LCA(KRB5_ENCKEY *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 908, Amissl)

#define KRB5_ENCKEY_it() \
	AROS_LC0(const ASN1_ITEM *, KRB5_ENCKEY_it, \
	struct Library *, AMISSL_BASE_NAME, 909, Amissl)

#define KRB5_AUTHDATA_new() \
	AROS_LC0(KRB5_AUTHDATA *, KRB5_AUTHDATA_new, \
	struct Library *, AMISSL_BASE_NAME, 910, Amissl)

#define KRB5_AUTHDATA_free(___a) \
	AROS_LC1(void, KRB5_AUTHDATA_free, \
	AROS_LCA(KRB5_AUTHDATA *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 911, Amissl)

#define d2i_KRB5_AUTHDATA(___a, ___in, ___len) \
	AROS_LC3(KRB5_AUTHDATA *, d2i_KRB5_AUTHDATA, \
	AROS_LCA(KRB5_AUTHDATA **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 912, Amissl)

#define i2d_KRB5_AUTHDATA(___a, ___out) \
	AROS_LC2(int, i2d_KRB5_AUTHDATA, \
	AROS_LCA(KRB5_AUTHDATA *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 913, Amissl)

#define KRB5_AUTHDATA_it() \
	AROS_LC0(const ASN1_ITEM *, KRB5_AUTHDATA_it, \
	struct Library *, AMISSL_BASE_NAME, 914, Amissl)

#define KRB5_AUTHENTBODY_new() \
	AROS_LC0(KRB5_AUTHENTBODY *, KRB5_AUTHENTBODY_new, \
	struct Library *, AMISSL_BASE_NAME, 915, Amissl)

#define KRB5_AUTHENTBODY_free(___a) \
	AROS_LC1(void, KRB5_AUTHENTBODY_free, \
	AROS_LCA(KRB5_AUTHENTBODY *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 916, Amissl)

#define d2i_KRB5_AUTHENTBODY(___a, ___in, ___len) \
	AROS_LC3(KRB5_AUTHENTBODY *, d2i_KRB5_AUTHENTBODY, \
	AROS_LCA(KRB5_AUTHENTBODY **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 917, Amissl)

#define i2d_KRB5_AUTHENTBODY(___a, ___out) \
	AROS_LC2(int, i2d_KRB5_AUTHENTBODY, \
	AROS_LCA(KRB5_AUTHENTBODY *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 918, Amissl)

#define KRB5_AUTHENTBODY_it() \
	AROS_LC0(const ASN1_ITEM *, KRB5_AUTHENTBODY_it, \
	struct Library *, AMISSL_BASE_NAME, 919, Amissl)

#define KRB5_AUTHENT_new() \
	AROS_LC0(KRB5_AUTHENT *, KRB5_AUTHENT_new, \
	struct Library *, AMISSL_BASE_NAME, 920, Amissl)

#define KRB5_AUTHENT_free(___a) \
	AROS_LC1(void, KRB5_AUTHENT_free, \
	AROS_LCA(KRB5_AUTHENT *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 921, Amissl)

#define d2i_KRB5_AUTHENT(___a, ___in, ___len) \
	AROS_LC3(KRB5_AUTHENT *, d2i_KRB5_AUTHENT, \
	AROS_LCA(KRB5_AUTHENT **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 922, Amissl)

#define i2d_KRB5_AUTHENT(___a, ___out) \
	AROS_LC2(int, i2d_KRB5_AUTHENT, \
	AROS_LCA(KRB5_AUTHENT *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 923, Amissl)

#define KRB5_AUTHENT_it() \
	AROS_LC0(const ASN1_ITEM *, KRB5_AUTHENT_it, \
	struct Library *, AMISSL_BASE_NAME, 924, Amissl)

#define lh_new(___h, ___c) \
	AROS_LC2(LHASH *, lh_new, \
	AROS_LCA(LHASH_HASH_FN_TYPE, (___h), D0), \
	AROS_LCA(LHASH_COMP_FN_TYPE, (___c), D1), \
	struct Library *, AMISSL_BASE_NAME, 925, Amissl)

#define lh_free(___lh) \
	AROS_LC1(void, lh_free, \
	AROS_LCA(LHASH *, (___lh), A0), \
	struct Library *, AMISSL_BASE_NAME, 926, Amissl)

#define lh_insert(___lh, ___data) \
	AROS_LC2(void *, lh_insert, \
	AROS_LCA(LHASH *, (___lh), A0), \
	AROS_LCA(const void *, (___data), A1), \
	struct Library *, AMISSL_BASE_NAME, 927, Amissl)

#define lh_delete(___lh, ___data) \
	AROS_LC2(void *, lh_delete, \
	AROS_LCA(LHASH *, (___lh), A0), \
	AROS_LCA(const void *, (___data), A1), \
	struct Library *, AMISSL_BASE_NAME, 928, Amissl)

#define lh_retrieve(___lh, ___data) \
	AROS_LC2(void *, lh_retrieve, \
	AROS_LCA(LHASH *, (___lh), A0), \
	AROS_LCA(const void *, (___data), A1), \
	struct Library *, AMISSL_BASE_NAME, 929, Amissl)

#define lh_doall(___lh, ___func) \
	AROS_LC2(void, lh_doall, \
	AROS_LCA(LHASH *, (___lh), A0), \
	AROS_LCA(LHASH_DOALL_FN_TYPE, (___func), D0), \
	struct Library *, AMISSL_BASE_NAME, 930, Amissl)

#define lh_doall_arg(___lh, ___func, ___arg) \
	AROS_LC3(void, lh_doall_arg, \
	AROS_LCA(LHASH *, (___lh), A0), \
	AROS_LCA(LHASH_DOALL_ARG_FN_TYPE, (___func), D0), \
	AROS_LCA(void *, (___arg), A1), \
	struct Library *, AMISSL_BASE_NAME, 931, Amissl)

#define lh_strhash(___c) \
	AROS_LC1(unsigned long, lh_strhash, \
	AROS_LCA(const char *, (___c), A0), \
	struct Library *, AMISSL_BASE_NAME, 932, Amissl)

#define lh_num_items(___lh) \
	AROS_LC1(unsigned long, lh_num_items, \
	AROS_LCA(const LHASH *, (___lh), A0), \
	struct Library *, AMISSL_BASE_NAME, 933, Amissl)

#define lh_stats_bio(___lh, ___out) \
	AROS_LC2(void, lh_stats_bio, \
	AROS_LCA(const LHASH *, (___lh), A0), \
	AROS_LCA(BIO *, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 934, Amissl)

#define lh_node_stats_bio(___lh, ___out) \
	AROS_LC2(void, lh_node_stats_bio, \
	AROS_LCA(const LHASH *, (___lh), A0), \
	AROS_LCA(BIO *, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 935, Amissl)

#define lh_node_usage_stats_bio(___lh, ___out) \
	AROS_LC2(void, lh_node_usage_stats_bio, \
	AROS_LCA(const LHASH *, (___lh), A0), \
	AROS_LCA(BIO *, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 936, Amissl)

#define OBJ_NAME_init() \
	AROS_LC0(int, OBJ_NAME_init, \
	struct Library *, AMISSL_BASE_NAME, 937, Amissl)

#define OBJ_NAME_new_index(___hash_func, ___cmp_func, ___free_func) \
	AROS_LC3(int, OBJ_NAME_new_index, \
	AROS_LCA(unsigned long (*)(const char *), (___hash_func), A0), \
	AROS_LCA(int (*)(const char *,const char *), (___cmp_func), A1), \
	AROS_LCA(void (*)(const char *,int,const char *), (___free_func), A2), \
	struct Library *, AMISSL_BASE_NAME, 938, Amissl)

#define OBJ_NAME_get(___name, ___type) \
	AROS_LC2(const char *, OBJ_NAME_get, \
	AROS_LCA(const char *, (___name), A0), \
	AROS_LCA(int, (___type), D0), \
	struct Library *, AMISSL_BASE_NAME, 939, Amissl)

#define OBJ_NAME_add(___name, ___type, ___data) \
	AROS_LC3(int, OBJ_NAME_add, \
	AROS_LCA(const char *, (___name), A0), \
	AROS_LCA(int, (___type), D0), \
	AROS_LCA(const char *, (___data), A1), \
	struct Library *, AMISSL_BASE_NAME, 940, Amissl)

#define OBJ_NAME_remove(___name, ___type) \
	AROS_LC2(int, OBJ_NAME_remove, \
	AROS_LCA(const char *, (___name), A0), \
	AROS_LCA(int, (___type), D0), \
	struct Library *, AMISSL_BASE_NAME, 941, Amissl)

#define OBJ_NAME_cleanup(___type) \
	AROS_LC1(void, OBJ_NAME_cleanup, \
	AROS_LCA(int, (___type), D0), \
	struct Library *, AMISSL_BASE_NAME, 942, Amissl)

#define OBJ_NAME_do_all(___type, ___fn, ___arg) \
	AROS_LC3(void, OBJ_NAME_do_all, \
	AROS_LCA(int, (___type), D0), \
	AROS_LCA(void (*)(const OBJ_NAME *,void *arg), (___fn), A0), \
	AROS_LCA(void *, (___arg), A1), \
	struct Library *, AMISSL_BASE_NAME, 943, Amissl)

#define OBJ_NAME_do_all_sorted(___type, ___fn, ___arg) \
	AROS_LC3(void, OBJ_NAME_do_all_sorted, \
	AROS_LCA(int, (___type), D0), \
	AROS_LCA(void (*)(const OBJ_NAME *,void *arg), (___fn), A0), \
	AROS_LCA(void *, (___arg), A1), \
	struct Library *, AMISSL_BASE_NAME, 944, Amissl)

#define OBJ_dup(___o) \
	AROS_LC1(ASN1_OBJECT *, OBJ_dup, \
	AROS_LCA(const ASN1_OBJECT *, (___o), A0), \
	struct Library *, AMISSL_BASE_NAME, 945, Amissl)

#define OBJ_nid2obj(___n) \
	AROS_LC1(ASN1_OBJECT *, OBJ_nid2obj, \
	AROS_LCA(int, (___n), D0), \
	struct Library *, AMISSL_BASE_NAME, 946, Amissl)

#define OBJ_nid2ln(___n) \
	AROS_LC1(const char *, OBJ_nid2ln, \
	AROS_LCA(int, (___n), D0), \
	struct Library *, AMISSL_BASE_NAME, 947, Amissl)

#define OBJ_nid2sn(___n) \
	AROS_LC1(const char *, OBJ_nid2sn, \
	AROS_LCA(int, (___n), D0), \
	struct Library *, AMISSL_BASE_NAME, 948, Amissl)

#define OBJ_obj2nid(___o) \
	AROS_LC1(int, OBJ_obj2nid, \
	AROS_LCA(const ASN1_OBJECT *, (___o), A0), \
	struct Library *, AMISSL_BASE_NAME, 949, Amissl)

#define OBJ_txt2obj(___s, ___no_name) \
	AROS_LC2(ASN1_OBJECT *, OBJ_txt2obj, \
	AROS_LCA(const char *, (___s), A0), \
	AROS_LCA(int, (___no_name), D0), \
	struct Library *, AMISSL_BASE_NAME, 950, Amissl)

#define OBJ_obj2txt(___buf, ___buf_len, ___a, ___no_name) \
	AROS_LC4(int, OBJ_obj2txt, \
	AROS_LCA(char *, (___buf), A0), \
	AROS_LCA(int, (___buf_len), D0), \
	AROS_LCA(const ASN1_OBJECT *, (___a), A1), \
	AROS_LCA(int, (___no_name), D1), \
	struct Library *, AMISSL_BASE_NAME, 951, Amissl)

#define OBJ_txt2nid(___s) \
	AROS_LC1(int, OBJ_txt2nid, \
	AROS_LCA(const char *, (___s), A0), \
	struct Library *, AMISSL_BASE_NAME, 952, Amissl)

#define OBJ_ln2nid(___s) \
	AROS_LC1(int, OBJ_ln2nid, \
	AROS_LCA(const char *, (___s), A0), \
	struct Library *, AMISSL_BASE_NAME, 953, Amissl)

#define OBJ_sn2nid(___s) \
	AROS_LC1(int, OBJ_sn2nid, \
	AROS_LCA(const char *, (___s), A0), \
	struct Library *, AMISSL_BASE_NAME, 954, Amissl)

#define OBJ_cmp(___a, ___b) \
	AROS_LC2(int, OBJ_cmp, \
	AROS_LCA(const ASN1_OBJECT *, (___a), A0), \
	AROS_LCA(const ASN1_OBJECT *, (___b), A1), \
	struct Library *, AMISSL_BASE_NAME, 955, Amissl)

#define OBJ_bsearch(___key, ___base, ___num, ___size, ___cmp) \
	AROS_LC5(const char *, OBJ_bsearch, \
	AROS_LCA(const char *, (___key), A0), \
	AROS_LCA(const char *, (___base), A1), \
	AROS_LCA(int, (___num), D0), \
	AROS_LCA(int, (___size), D1), \
	AROS_LCA(int (*)(const void *,const void *), (___cmp), A2), \
	struct Library *, AMISSL_BASE_NAME, 956, Amissl)

#define OBJ_new_nid(___num) \
	AROS_LC1(int, OBJ_new_nid, \
	AROS_LCA(int, (___num), D0), \
	struct Library *, AMISSL_BASE_NAME, 957, Amissl)

#define OBJ_add_object(___obj) \
	AROS_LC1(int, OBJ_add_object, \
	AROS_LCA(const ASN1_OBJECT *, (___obj), A0), \
	struct Library *, AMISSL_BASE_NAME, 958, Amissl)

#define OBJ_create(___oid, ___sn, ___ln) \
	AROS_LC3(int, OBJ_create, \
	AROS_LCA(const char *, (___oid), A0), \
	AROS_LCA(const char *, (___sn), A1), \
	AROS_LCA(const char *, (___ln), A2), \
	struct Library *, AMISSL_BASE_NAME, 959, Amissl)

#define OBJ_cleanup() \
	AROS_LC0(void, OBJ_cleanup, \
	struct Library *, AMISSL_BASE_NAME, 960, Amissl)

#define OBJ_create_objects(___in) \
	AROS_LC1(int, OBJ_create_objects, \
	AROS_LCA(BIO *, (___in), A0), \
	struct Library *, AMISSL_BASE_NAME, 961, Amissl)

#define ERR_load_OBJ_strings() \
	AROS_LC0(void, ERR_load_OBJ_strings, \
	struct Library *, AMISSL_BASE_NAME, 962, Amissl)

#define OCSP_sendreq_bio(___b, ___path, ___req) \
	AROS_LC3(OCSP_RESPONSE *, OCSP_sendreq_bio, \
	AROS_LCA(BIO *, (___b), A0), \
	AROS_LCA(char *, (___path), A1), \
	AROS_LCA(OCSP_REQUEST *, (___req), A2), \
	struct Library *, AMISSL_BASE_NAME, 963, Amissl)

#define OCSP_cert_to_id(___dgst, ___subject, ___issuer) \
	AROS_LC3(OCSP_CERTID *, OCSP_cert_to_id, \
	AROS_LCA(const EVP_MD *, (___dgst), A0), \
	AROS_LCA(X509 *, (___subject), A1), \
	AROS_LCA(X509 *, (___issuer), A2), \
	struct Library *, AMISSL_BASE_NAME, 964, Amissl)

#define OCSP_cert_id_new(___dgst, ___issuerName, ___issuerKey, ___serialNumber) \
	AROS_LC4(OCSP_CERTID *, OCSP_cert_id_new, \
	AROS_LCA(const EVP_MD *, (___dgst), A0), \
	AROS_LCA(X509_NAME *, (___issuerName), A1), \
	AROS_LCA(ASN1_BIT_STRING*, (___issuerKey), A2), \
	AROS_LCA(ASN1_INTEGER *, (___serialNumber), A3), \
	struct Library *, AMISSL_BASE_NAME, 965, Amissl)

#define OCSP_request_add0_id(___req, ___cid) \
	AROS_LC2(OCSP_ONEREQ *, OCSP_request_add0_id, \
	AROS_LCA(OCSP_REQUEST *, (___req), A0), \
	AROS_LCA(OCSP_CERTID *, (___cid), A1), \
	struct Library *, AMISSL_BASE_NAME, 966, Amissl)

#define OCSP_request_add1_nonce(___req, ___val, ___len) \
	AROS_LC3(int, OCSP_request_add1_nonce, \
	AROS_LCA(OCSP_REQUEST *, (___req), A0), \
	AROS_LCA(unsigned char *, (___val), A1), \
	AROS_LCA(int, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 967, Amissl)

#define OCSP_basic_add1_nonce(___resp, ___val, ___len) \
	AROS_LC3(int, OCSP_basic_add1_nonce, \
	AROS_LCA(OCSP_BASICRESP *, (___resp), A0), \
	AROS_LCA(unsigned char *, (___val), A1), \
	AROS_LCA(int, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 968, Amissl)

#define OCSP_check_nonce(___req, ___bs) \
	AROS_LC2(int, OCSP_check_nonce, \
	AROS_LCA(OCSP_REQUEST *, (___req), A0), \
	AROS_LCA(OCSP_BASICRESP *, (___bs), A1), \
	struct Library *, AMISSL_BASE_NAME, 969, Amissl)

#define OCSP_copy_nonce(___resp, ___req) \
	AROS_LC2(int, OCSP_copy_nonce, \
	AROS_LCA(OCSP_BASICRESP *, (___resp), A0), \
	AROS_LCA(OCSP_REQUEST *, (___req), A1), \
	struct Library *, AMISSL_BASE_NAME, 970, Amissl)

#define OCSP_request_set1_name(___req, ___nm) \
	AROS_LC2(int, OCSP_request_set1_name, \
	AROS_LCA(OCSP_REQUEST *, (___req), A0), \
	AROS_LCA(X509_NAME *, (___nm), A1), \
	struct Library *, AMISSL_BASE_NAME, 971, Amissl)

#define OCSP_request_add1_cert(___req, ___cert) \
	AROS_LC2(int, OCSP_request_add1_cert, \
	AROS_LCA(OCSP_REQUEST *, (___req), A0), \
	AROS_LCA(X509 *, (___cert), A1), \
	struct Library *, AMISSL_BASE_NAME, 972, Amissl)

#define OCSP_request_sign(___req, ___signer, ___key, ___dgst, ___certs, ___flags) \
	AROS_LC6(int, OCSP_request_sign, \
	AROS_LCA(OCSP_REQUEST *, (___req), A0), \
	AROS_LCA(X509 *, (___signer), A1), \
	AROS_LCA(EVP_PKEY *, (___key), A2), \
	AROS_LCA(const EVP_MD *, (___dgst), A3), \
	AROS_LCA(STACK_OF(X509) *, (___certs), D0), \
	AROS_LCA(unsigned long, (___flags), D1), \
	struct Library *, AMISSL_BASE_NAME, 973, Amissl)

#define OCSP_response_status(___resp) \
	AROS_LC1(int, OCSP_response_status, \
	AROS_LCA(OCSP_RESPONSE *, (___resp), A0), \
	struct Library *, AMISSL_BASE_NAME, 974, Amissl)

#define OCSP_response_get1_basic(___resp) \
	AROS_LC1(OCSP_BASICRESP *, OCSP_response_get1_basic, \
	AROS_LCA(OCSP_RESPONSE *, (___resp), A0), \
	struct Library *, AMISSL_BASE_NAME, 975, Amissl)

#define OCSP_resp_count(___bs) \
	AROS_LC1(int, OCSP_resp_count, \
	AROS_LCA(OCSP_BASICRESP *, (___bs), A0), \
	struct Library *, AMISSL_BASE_NAME, 976, Amissl)

#define OCSP_resp_get0(___bs, ___idx) \
	AROS_LC2(OCSP_SINGLERESP *, OCSP_resp_get0, \
	AROS_LCA(OCSP_BASICRESP *, (___bs), A0), \
	AROS_LCA(int, (___idx), D0), \
	struct Library *, AMISSL_BASE_NAME, 977, Amissl)

#define OCSP_resp_find(___bs, ___id, ___last) \
	AROS_LC3(int, OCSP_resp_find, \
	AROS_LCA(OCSP_BASICRESP *, (___bs), A0), \
	AROS_LCA(OCSP_CERTID *, (___id), A1), \
	AROS_LCA(int, (___last), D0), \
	struct Library *, AMISSL_BASE_NAME, 978, Amissl)

#define OCSP_single_get0_status(___single, ___reason, ___revtime, ___thisupd, ___nextupd) \
	AROS_LC5(int, OCSP_single_get0_status, \
	AROS_LCA(OCSP_SINGLERESP *, (___single), A0), \
	AROS_LCA(int *, (___reason), A1), \
	AROS_LCA(ASN1_GENERALIZEDTIME **, (___revtime), A2), \
	AROS_LCA(ASN1_GENERALIZEDTIME **, (___thisupd), A3), \
	AROS_LCA(ASN1_GENERALIZEDTIME **, (___nextupd), D0), \
	struct Library *, AMISSL_BASE_NAME, 979, Amissl)

#define OCSP_resp_find_status(___bs, ___id, ___status, ___reason, ___revtime, ___thisupd, ___nextupd) \
	AROS_LC7(int, OCSP_resp_find_status, \
	AROS_LCA(OCSP_BASICRESP *, (___bs), A0), \
	AROS_LCA(OCSP_CERTID *, (___id), A1), \
	AROS_LCA(int *, (___status), A2), \
	AROS_LCA(int *, (___reason), A3), \
	AROS_LCA(ASN1_GENERALIZEDTIME **, (___revtime), D0), \
	AROS_LCA(ASN1_GENERALIZEDTIME **, (___thisupd), D1), \
	AROS_LCA(ASN1_GENERALIZEDTIME **, (___nextupd), D2), \
	struct Library *, AMISSL_BASE_NAME, 980, Amissl)

#define OCSP_check_validity(___thisupd, ___nextupd, ___sec, ___maxsec) \
	AROS_LC4(int, OCSP_check_validity, \
	AROS_LCA(ASN1_GENERALIZEDTIME *, (___thisupd), A0), \
	AROS_LCA(ASN1_GENERALIZEDTIME *, (___nextupd), A1), \
	AROS_LCA(long, (___sec), D0), \
	AROS_LCA(long, (___maxsec), D1), \
	struct Library *, AMISSL_BASE_NAME, 981, Amissl)

#define OCSP_request_verify(___req, ___certs, ___store, ___flags) \
	AROS_LC4(int, OCSP_request_verify, \
	AROS_LCA(OCSP_REQUEST *, (___req), A0), \
	AROS_LCA(STACK_OF(X509) *, (___certs), A1), \
	AROS_LCA(X509_STORE *, (___store), A2), \
	AROS_LCA(unsigned long, (___flags), D0), \
	struct Library *, AMISSL_BASE_NAME, 982, Amissl)

#define OCSP_parse_url(___url, ___phost, ___pport, ___ppath, ___pssl) \
	AROS_LC5(int, OCSP_parse_url, \
	AROS_LCA(char *, (___url), A0), \
	AROS_LCA(char **, (___phost), A1), \
	AROS_LCA(char **, (___pport), A2), \
	AROS_LCA(char **, (___ppath), A3), \
	AROS_LCA(int *, (___pssl), D0), \
	struct Library *, AMISSL_BASE_NAME, 983, Amissl)

#define OCSP_id_issuer_cmp(___a, ___b) \
	AROS_LC2(int, OCSP_id_issuer_cmp, \
	AROS_LCA(OCSP_CERTID *, (___a), A0), \
	AROS_LCA(OCSP_CERTID *, (___b), A1), \
	struct Library *, AMISSL_BASE_NAME, 984, Amissl)

#define OCSP_id_cmp(___a, ___b) \
	AROS_LC2(int, OCSP_id_cmp, \
	AROS_LCA(OCSP_CERTID *, (___a), A0), \
	AROS_LCA(OCSP_CERTID *, (___b), A1), \
	struct Library *, AMISSL_BASE_NAME, 985, Amissl)

#define OCSP_request_onereq_count(___req) \
	AROS_LC1(int, OCSP_request_onereq_count, \
	AROS_LCA(OCSP_REQUEST *, (___req), A0), \
	struct Library *, AMISSL_BASE_NAME, 986, Amissl)

#define OCSP_request_onereq_get0(___req, ___i) \
	AROS_LC2(OCSP_ONEREQ *, OCSP_request_onereq_get0, \
	AROS_LCA(OCSP_REQUEST *, (___req), A0), \
	AROS_LCA(int, (___i), D0), \
	struct Library *, AMISSL_BASE_NAME, 987, Amissl)

#define OCSP_onereq_get0_id(___one) \
	AROS_LC1(OCSP_CERTID *, OCSP_onereq_get0_id, \
	AROS_LCA(OCSP_ONEREQ *, (___one), A0), \
	struct Library *, AMISSL_BASE_NAME, 988, Amissl)

#define OCSP_id_get0_info(___piNameHash, ___pmd, ___pikeyHash, ___pserial, ___cid) \
	AROS_LC5(int, OCSP_id_get0_info, \
	AROS_LCA(ASN1_OCTET_STRING **, (___piNameHash), A0), \
	AROS_LCA(ASN1_OBJECT **, (___pmd), A1), \
	AROS_LCA(ASN1_OCTET_STRING **, (___pikeyHash), A2), \
	AROS_LCA(ASN1_INTEGER **, (___pserial), A3), \
	AROS_LCA(OCSP_CERTID *, (___cid), D0), \
	struct Library *, AMISSL_BASE_NAME, 989, Amissl)

#define OCSP_request_is_signed(___req) \
	AROS_LC1(int, OCSP_request_is_signed, \
	AROS_LCA(OCSP_REQUEST *, (___req), A0), \
	struct Library *, AMISSL_BASE_NAME, 990, Amissl)

#define OCSP_response_create(___status, ___bs) \
	AROS_LC2(OCSP_RESPONSE *, OCSP_response_create, \
	AROS_LCA(int, (___status), D0), \
	AROS_LCA(OCSP_BASICRESP *, (___bs), A0), \
	struct Library *, AMISSL_BASE_NAME, 991, Amissl)

#define OCSP_basic_add1_status(___rsp, ___cid, ___status, ___reason, ___revtime, ___thisupd, ___nextupd) \
	AROS_LC7(OCSP_SINGLERESP *, OCSP_basic_add1_status, \
	AROS_LCA(OCSP_BASICRESP *, (___rsp), A0), \
	AROS_LCA(OCSP_CERTID *, (___cid), A1), \
	AROS_LCA(int, (___status), D0), \
	AROS_LCA(int, (___reason), D1), \
	AROS_LCA(ASN1_TIME *, (___revtime), A2), \
	AROS_LCA(ASN1_TIME *, (___thisupd), A3), \
	AROS_LCA(ASN1_TIME *, (___nextupd), D2), \
	struct Library *, AMISSL_BASE_NAME, 992, Amissl)

#define OCSP_basic_add1_cert(___resp, ___cert) \
	AROS_LC2(int, OCSP_basic_add1_cert, \
	AROS_LCA(OCSP_BASICRESP *, (___resp), A0), \
	AROS_LCA(X509 *, (___cert), A1), \
	struct Library *, AMISSL_BASE_NAME, 993, Amissl)

#define OCSP_basic_sign(___brsp, ___signer, ___key, ___dgst, ___certs, ___flags) \
	AROS_LC6(int, OCSP_basic_sign, \
	AROS_LCA(OCSP_BASICRESP *, (___brsp), A0), \
	AROS_LCA(X509 *, (___signer), A1), \
	AROS_LCA(EVP_PKEY *, (___key), A2), \
	AROS_LCA(const EVP_MD *, (___dgst), A3), \
	AROS_LCA(STACK_OF(X509) *, (___certs), D0), \
	AROS_LCA(unsigned long, (___flags), D1), \
	struct Library *, AMISSL_BASE_NAME, 994, Amissl)

#define ASN1_STRING_encode(___s, ___i2d, ___data, ___sk) \
	AROS_LC4(ASN1_STRING *, ASN1_STRING_encode, \
	AROS_LCA(ASN1_STRING *, (___s), A0), \
	AROS_LCA(int (*)(), (___i2d), A1), \
	AROS_LCA(char *, (___data), A2), \
	AROS_LCA(STACK_OF(ASN1_OBJECT) *, (___sk), A3), \
	struct Library *, AMISSL_BASE_NAME, 995, Amissl)

#define OCSP_crlID_new(___url, ___n, ___tim) \
	AROS_LC3(X509_EXTENSION *, OCSP_crlID_new, \
	AROS_LCA(char *, (___url), A0), \
	AROS_LCA(long *, (___n), A1), \
	AROS_LCA(char *, (___tim), A2), \
	struct Library *, AMISSL_BASE_NAME, 996, Amissl)

#define OCSP_accept_responses_new(___oids) \
	AROS_LC1(X509_EXTENSION *, OCSP_accept_responses_new, \
	AROS_LCA(char **, (___oids), A0), \
	struct Library *, AMISSL_BASE_NAME, 997, Amissl)

#define OCSP_archive_cutoff_new(___tim) \
	AROS_LC1(X509_EXTENSION *, OCSP_archive_cutoff_new, \
	AROS_LCA(char*, (___tim), A0), \
	struct Library *, AMISSL_BASE_NAME, 998, Amissl)

#define OCSP_url_svcloc_new(___issuer, ___urls) \
	AROS_LC2(X509_EXTENSION *, OCSP_url_svcloc_new, \
	AROS_LCA(X509_NAME*, (___issuer), A0), \
	AROS_LCA(char **, (___urls), A1), \
	struct Library *, AMISSL_BASE_NAME, 999, Amissl)

#define OCSP_REQUEST_get_ext_count(___x) \
	AROS_LC1(int, OCSP_REQUEST_get_ext_count, \
	AROS_LCA(OCSP_REQUEST *, (___x), A0), \
	struct Library *, AMISSL_BASE_NAME, 1000, Amissl)

#define OCSP_REQUEST_get_ext_by_NID(___x, ___nid, ___lastpos) \
	AROS_LC3(int, OCSP_REQUEST_get_ext_by_NID, \
	AROS_LCA(OCSP_REQUEST *, (___x), A0), \
	AROS_LCA(int, (___nid), D0), \
	AROS_LCA(int, (___lastpos), D1), \
	struct Library *, AMISSL_BASE_NAME, 1001, Amissl)

#define OCSP_REQUEST_get_ext_by_OBJ(___x, ___obj, ___lastpos) \
	AROS_LC3(int, OCSP_REQUEST_get_ext_by_OBJ, \
	AROS_LCA(OCSP_REQUEST *, (___x), A0), \
	AROS_LCA(ASN1_OBJECT *, (___obj), A1), \
	AROS_LCA(int, (___lastpos), D0), \
	struct Library *, AMISSL_BASE_NAME, 1002, Amissl)

#define OCSP_REQUEST_get_ext_by_critical(___x, ___crit, ___lastpos) \
	AROS_LC3(int, OCSP_REQUEST_get_ext_by_critical, \
	AROS_LCA(OCSP_REQUEST *, (___x), A0), \
	AROS_LCA(int, (___crit), D0), \
	AROS_LCA(int, (___lastpos), D1), \
	struct Library *, AMISSL_BASE_NAME, 1003, Amissl)

#define OCSP_REQUEST_get_ext(___x, ___loc) \
	AROS_LC2(X509_EXTENSION *, OCSP_REQUEST_get_ext, \
	AROS_LCA(OCSP_REQUEST *, (___x), A0), \
	AROS_LCA(int, (___loc), D0), \
	struct Library *, AMISSL_BASE_NAME, 1004, Amissl)

#define OCSP_REQUEST_delete_ext(___x, ___loc) \
	AROS_LC2(X509_EXTENSION *, OCSP_REQUEST_delete_ext, \
	AROS_LCA(OCSP_REQUEST *, (___x), A0), \
	AROS_LCA(int, (___loc), D0), \
	struct Library *, AMISSL_BASE_NAME, 1005, Amissl)

#define OCSP_REQUEST_get1_ext_d2i(___x, ___nid, ___crit, ___idx) \
	AROS_LC4(void *, OCSP_REQUEST_get1_ext_d2i, \
	AROS_LCA(OCSP_REQUEST *, (___x), A0), \
	AROS_LCA(int, (___nid), D0), \
	AROS_LCA(int *, (___crit), A1), \
	AROS_LCA(int *, (___idx), A2), \
	struct Library *, AMISSL_BASE_NAME, 1006, Amissl)

#define OCSP_REQUEST_add1_ext_i2d(___x, ___nid, ___value, ___crit, ___flags) \
	AROS_LC5(int, OCSP_REQUEST_add1_ext_i2d, \
	AROS_LCA(OCSP_REQUEST *, (___x), A0), \
	AROS_LCA(int, (___nid), D0), \
	AROS_LCA(void *, (___value), A1), \
	AROS_LCA(int, (___crit), D1), \
	AROS_LCA(unsigned long, (___flags), D2), \
	struct Library *, AMISSL_BASE_NAME, 1007, Amissl)

#define OCSP_REQUEST_add_ext(___x, ___ex, ___loc) \
	AROS_LC3(int, OCSP_REQUEST_add_ext, \
	AROS_LCA(OCSP_REQUEST *, (___x), A0), \
	AROS_LCA(X509_EXTENSION *, (___ex), A1), \
	AROS_LCA(int, (___loc), D0), \
	struct Library *, AMISSL_BASE_NAME, 1008, Amissl)

#define OCSP_ONEREQ_get_ext_count(___x) \
	AROS_LC1(int, OCSP_ONEREQ_get_ext_count, \
	AROS_LCA(OCSP_ONEREQ *, (___x), A0), \
	struct Library *, AMISSL_BASE_NAME, 1009, Amissl)

#define OCSP_ONEREQ_get_ext_by_NID(___x, ___nid, ___lastpos) \
	AROS_LC3(int, OCSP_ONEREQ_get_ext_by_NID, \
	AROS_LCA(OCSP_ONEREQ *, (___x), A0), \
	AROS_LCA(int, (___nid), D0), \
	AROS_LCA(int, (___lastpos), D1), \
	struct Library *, AMISSL_BASE_NAME, 1010, Amissl)

#define OCSP_ONEREQ_get_ext_by_OBJ(___x, ___obj, ___lastpos) \
	AROS_LC3(int, OCSP_ONEREQ_get_ext_by_OBJ, \
	AROS_LCA(OCSP_ONEREQ *, (___x), A0), \
	AROS_LCA(ASN1_OBJECT *, (___obj), A1), \
	AROS_LCA(int, (___lastpos), D0), \
	struct Library *, AMISSL_BASE_NAME, 1011, Amissl)

#define OCSP_ONEREQ_get_ext_by_critical(___x, ___crit, ___lastpos) \
	AROS_LC3(int, OCSP_ONEREQ_get_ext_by_critical, \
	AROS_LCA(OCSP_ONEREQ *, (___x), A0), \
	AROS_LCA(int, (___crit), D0), \
	AROS_LCA(int, (___lastpos), D1), \
	struct Library *, AMISSL_BASE_NAME, 1012, Amissl)

#define OCSP_ONEREQ_get_ext(___x, ___loc) \
	AROS_LC2(X509_EXTENSION *, OCSP_ONEREQ_get_ext, \
	AROS_LCA(OCSP_ONEREQ *, (___x), A0), \
	AROS_LCA(int, (___loc), D0), \
	struct Library *, AMISSL_BASE_NAME, 1013, Amissl)

#define OCSP_ONEREQ_delete_ext(___x, ___loc) \
	AROS_LC2(X509_EXTENSION *, OCSP_ONEREQ_delete_ext, \
	AROS_LCA(OCSP_ONEREQ *, (___x), A0), \
	AROS_LCA(int, (___loc), D0), \
	struct Library *, AMISSL_BASE_NAME, 1014, Amissl)

#define OCSP_ONEREQ_get1_ext_d2i(___x, ___nid, ___crit, ___idx) \
	AROS_LC4(void *, OCSP_ONEREQ_get1_ext_d2i, \
	AROS_LCA(OCSP_ONEREQ *, (___x), A0), \
	AROS_LCA(int, (___nid), D0), \
	AROS_LCA(int *, (___crit), A1), \
	AROS_LCA(int *, (___idx), A2), \
	struct Library *, AMISSL_BASE_NAME, 1015, Amissl)

#define OCSP_ONEREQ_add1_ext_i2d(___x, ___nid, ___value, ___crit, ___flags) \
	AROS_LC5(int, OCSP_ONEREQ_add1_ext_i2d, \
	AROS_LCA(OCSP_ONEREQ *, (___x), A0), \
	AROS_LCA(int, (___nid), D0), \
	AROS_LCA(void *, (___value), A1), \
	AROS_LCA(int, (___crit), D1), \
	AROS_LCA(unsigned long, (___flags), D2), \
	struct Library *, AMISSL_BASE_NAME, 1016, Amissl)

#define OCSP_ONEREQ_add_ext(___x, ___ex, ___loc) \
	AROS_LC3(int, OCSP_ONEREQ_add_ext, \
	AROS_LCA(OCSP_ONEREQ *, (___x), A0), \
	AROS_LCA(X509_EXTENSION *, (___ex), A1), \
	AROS_LCA(int, (___loc), D0), \
	struct Library *, AMISSL_BASE_NAME, 1017, Amissl)

#define OCSP_BASICRESP_get_ext_count(___x) \
	AROS_LC1(int, OCSP_BASICRESP_get_ext_count, \
	AROS_LCA(OCSP_BASICRESP *, (___x), A0), \
	struct Library *, AMISSL_BASE_NAME, 1018, Amissl)

#define OCSP_BASICRESP_get_ext_by_NID(___x, ___nid, ___lastpos) \
	AROS_LC3(int, OCSP_BASICRESP_get_ext_by_NID, \
	AROS_LCA(OCSP_BASICRESP *, (___x), A0), \
	AROS_LCA(int, (___nid), D0), \
	AROS_LCA(int, (___lastpos), D1), \
	struct Library *, AMISSL_BASE_NAME, 1019, Amissl)

#define OCSP_BASICRESP_get_ext_by_OBJ(___x, ___obj, ___lastpos) \
	AROS_LC3(int, OCSP_BASICRESP_get_ext_by_OBJ, \
	AROS_LCA(OCSP_BASICRESP *, (___x), A0), \
	AROS_LCA(ASN1_OBJECT *, (___obj), A1), \
	AROS_LCA(int, (___lastpos), D0), \
	struct Library *, AMISSL_BASE_NAME, 1020, Amissl)

#define OCSP_BASICRESP_get_ext_by_critical(___x, ___crit, ___lastpos) \
	AROS_LC3(int, OCSP_BASICRESP_get_ext_by_critical, \
	AROS_LCA(OCSP_BASICRESP *, (___x), A0), \
	AROS_LCA(int, (___crit), D0), \
	AROS_LCA(int, (___lastpos), D1), \
	struct Library *, AMISSL_BASE_NAME, 1021, Amissl)

#define OCSP_BASICRESP_get_ext(___x, ___loc) \
	AROS_LC2(X509_EXTENSION *, OCSP_BASICRESP_get_ext, \
	AROS_LCA(OCSP_BASICRESP *, (___x), A0), \
	AROS_LCA(int, (___loc), D0), \
	struct Library *, AMISSL_BASE_NAME, 1022, Amissl)

#define OCSP_BASICRESP_delete_ext(___x, ___loc) \
	AROS_LC2(X509_EXTENSION *, OCSP_BASICRESP_delete_ext, \
	AROS_LCA(OCSP_BASICRESP *, (___x), A0), \
	AROS_LCA(int, (___loc), D0), \
	struct Library *, AMISSL_BASE_NAME, 1023, Amissl)

#define OCSP_BASICRESP_get1_ext_d2i(___x, ___nid, ___crit, ___idx) \
	AROS_LC4(void *, OCSP_BASICRESP_get1_ext_d2i, \
	AROS_LCA(OCSP_BASICRESP *, (___x), A0), \
	AROS_LCA(int, (___nid), D0), \
	AROS_LCA(int *, (___crit), A1), \
	AROS_LCA(int *, (___idx), A2), \
	struct Library *, AMISSL_BASE_NAME, 1024, Amissl)

#define OCSP_BASICRESP_add1_ext_i2d(___x, ___nid, ___value, ___crit, ___flags) \
	AROS_LC5(int, OCSP_BASICRESP_add1_ext_i2d, \
	AROS_LCA(OCSP_BASICRESP *, (___x), A0), \
	AROS_LCA(int, (___nid), D0), \
	AROS_LCA(void *, (___value), A1), \
	AROS_LCA(int, (___crit), D1), \
	AROS_LCA(unsigned long, (___flags), D2), \
	struct Library *, AMISSL_BASE_NAME, 1025, Amissl)

#define OCSP_BASICRESP_add_ext(___x, ___ex, ___loc) \
	AROS_LC3(int, OCSP_BASICRESP_add_ext, \
	AROS_LCA(OCSP_BASICRESP *, (___x), A0), \
	AROS_LCA(X509_EXTENSION *, (___ex), A1), \
	AROS_LCA(int, (___loc), D0), \
	struct Library *, AMISSL_BASE_NAME, 1026, Amissl)

#define OCSP_SINGLERESP_get_ext_count(___x) \
	AROS_LC1(int, OCSP_SINGLERESP_get_ext_count, \
	AROS_LCA(OCSP_SINGLERESP *, (___x), A0), \
	struct Library *, AMISSL_BASE_NAME, 1027, Amissl)

#define OCSP_SINGLERESP_get_ext_by_NID(___x, ___nid, ___lastpos) \
	AROS_LC3(int, OCSP_SINGLERESP_get_ext_by_NID, \
	AROS_LCA(OCSP_SINGLERESP *, (___x), A0), \
	AROS_LCA(int, (___nid), D0), \
	AROS_LCA(int, (___lastpos), D1), \
	struct Library *, AMISSL_BASE_NAME, 1028, Amissl)

#define OCSP_SINGLERESP_get_ext_by_OBJ(___x, ___obj, ___lastpos) \
	AROS_LC3(int, OCSP_SINGLERESP_get_ext_by_OBJ, \
	AROS_LCA(OCSP_SINGLERESP *, (___x), A0), \
	AROS_LCA(ASN1_OBJECT *, (___obj), A1), \
	AROS_LCA(int, (___lastpos), D0), \
	struct Library *, AMISSL_BASE_NAME, 1029, Amissl)

#define OCSP_SINGLERESP_get_ext_by_critical(___x, ___crit, ___lastpos) \
	AROS_LC3(int, OCSP_SINGLERESP_get_ext_by_critical, \
	AROS_LCA(OCSP_SINGLERESP *, (___x), A0), \
	AROS_LCA(int, (___crit), D0), \
	AROS_LCA(int, (___lastpos), D1), \
	struct Library *, AMISSL_BASE_NAME, 1030, Amissl)

#define OCSP_SINGLERESP_get_ext(___x, ___loc) \
	AROS_LC2(X509_EXTENSION *, OCSP_SINGLERESP_get_ext, \
	AROS_LCA(OCSP_SINGLERESP *, (___x), A0), \
	AROS_LCA(int, (___loc), D0), \
	struct Library *, AMISSL_BASE_NAME, 1031, Amissl)

#define OCSP_SINGLERESP_delete_ext(___x, ___loc) \
	AROS_LC2(X509_EXTENSION *, OCSP_SINGLERESP_delete_ext, \
	AROS_LCA(OCSP_SINGLERESP *, (___x), A0), \
	AROS_LCA(int, (___loc), D0), \
	struct Library *, AMISSL_BASE_NAME, 1032, Amissl)

#define OCSP_SINGLERESP_get1_ext_d2i(___x, ___nid, ___crit, ___idx) \
	AROS_LC4(void *, OCSP_SINGLERESP_get1_ext_d2i, \
	AROS_LCA(OCSP_SINGLERESP *, (___x), A0), \
	AROS_LCA(int, (___nid), D0), \
	AROS_LCA(int *, (___crit), A1), \
	AROS_LCA(int *, (___idx), A2), \
	struct Library *, AMISSL_BASE_NAME, 1033, Amissl)

#define OCSP_SINGLERESP_add1_ext_i2d(___x, ___nid, ___value, ___crit, ___flags) \
	AROS_LC5(int, OCSP_SINGLERESP_add1_ext_i2d, \
	AROS_LCA(OCSP_SINGLERESP *, (___x), A0), \
	AROS_LCA(int, (___nid), D0), \
	AROS_LCA(void *, (___value), A1), \
	AROS_LCA(int, (___crit), D1), \
	AROS_LCA(unsigned long, (___flags), D2), \
	struct Library *, AMISSL_BASE_NAME, 1034, Amissl)

#define OCSP_SINGLERESP_add_ext(___x, ___ex, ___loc) \
	AROS_LC3(int, OCSP_SINGLERESP_add_ext, \
	AROS_LCA(OCSP_SINGLERESP *, (___x), A0), \
	AROS_LCA(X509_EXTENSION *, (___ex), A1), \
	AROS_LCA(int, (___loc), D0), \
	struct Library *, AMISSL_BASE_NAME, 1035, Amissl)

#define OCSP_SINGLERESP_new() \
	AROS_LC0(OCSP_SINGLERESP *, OCSP_SINGLERESP_new, \
	struct Library *, AMISSL_BASE_NAME, 1036, Amissl)

#define OCSP_SINGLERESP_free(___a) \
	AROS_LC1(void, OCSP_SINGLERESP_free, \
	AROS_LCA(OCSP_SINGLERESP *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 1037, Amissl)

#define d2i_OCSP_SINGLERESP(___a, ___in, ___len) \
	AROS_LC3(OCSP_SINGLERESP *, d2i_OCSP_SINGLERESP, \
	AROS_LCA(OCSP_SINGLERESP **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 1038, Amissl)

#define i2d_OCSP_SINGLERESP(___a, ___out) \
	AROS_LC2(int, i2d_OCSP_SINGLERESP, \
	AROS_LCA(OCSP_SINGLERESP *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 1039, Amissl)

#define OCSP_SINGLERESP_it() \
	AROS_LC0(const ASN1_ITEM *, OCSP_SINGLERESP_it, \
	struct Library *, AMISSL_BASE_NAME, 1040, Amissl)

#define OCSP_CERTSTATUS_new() \
	AROS_LC0(OCSP_CERTSTATUS *, OCSP_CERTSTATUS_new, \
	struct Library *, AMISSL_BASE_NAME, 1041, Amissl)

#define OCSP_CERTSTATUS_free(___a) \
	AROS_LC1(void, OCSP_CERTSTATUS_free, \
	AROS_LCA(OCSP_CERTSTATUS *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 1042, Amissl)

#define d2i_OCSP_CERTSTATUS(___a, ___in, ___len) \
	AROS_LC3(OCSP_CERTSTATUS *, d2i_OCSP_CERTSTATUS, \
	AROS_LCA(OCSP_CERTSTATUS **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 1043, Amissl)

#define i2d_OCSP_CERTSTATUS(___a, ___out) \
	AROS_LC2(int, i2d_OCSP_CERTSTATUS, \
	AROS_LCA(OCSP_CERTSTATUS *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 1044, Amissl)

#define OCSP_CERTSTATUS_it() \
	AROS_LC0(const ASN1_ITEM *, OCSP_CERTSTATUS_it, \
	struct Library *, AMISSL_BASE_NAME, 1045, Amissl)

#define OCSP_REVOKEDINFO_new() \
	AROS_LC0(OCSP_REVOKEDINFO *, OCSP_REVOKEDINFO_new, \
	struct Library *, AMISSL_BASE_NAME, 1046, Amissl)

#define OCSP_REVOKEDINFO_free(___a) \
	AROS_LC1(void, OCSP_REVOKEDINFO_free, \
	AROS_LCA(OCSP_REVOKEDINFO *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 1047, Amissl)

#define d2i_OCSP_REVOKEDINFO(___a, ___in, ___len) \
	AROS_LC3(OCSP_REVOKEDINFO *, d2i_OCSP_REVOKEDINFO, \
	AROS_LCA(OCSP_REVOKEDINFO **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 1048, Amissl)

#define i2d_OCSP_REVOKEDINFO(___a, ___out) \
	AROS_LC2(int, i2d_OCSP_REVOKEDINFO, \
	AROS_LCA(OCSP_REVOKEDINFO *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 1049, Amissl)

#define OCSP_REVOKEDINFO_it() \
	AROS_LC0(const ASN1_ITEM *, OCSP_REVOKEDINFO_it, \
	struct Library *, AMISSL_BASE_NAME, 1050, Amissl)

#define OCSP_BASICRESP_new() \
	AROS_LC0(OCSP_BASICRESP *, OCSP_BASICRESP_new, \
	struct Library *, AMISSL_BASE_NAME, 1051, Amissl)

#define OCSP_BASICRESP_free(___a) \
	AROS_LC1(void, OCSP_BASICRESP_free, \
	AROS_LCA(OCSP_BASICRESP *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 1052, Amissl)

#define d2i_OCSP_BASICRESP(___a, ___in, ___len) \
	AROS_LC3(OCSP_BASICRESP *, d2i_OCSP_BASICRESP, \
	AROS_LCA(OCSP_BASICRESP **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 1053, Amissl)

#define i2d_OCSP_BASICRESP(___a, ___out) \
	AROS_LC2(int, i2d_OCSP_BASICRESP, \
	AROS_LCA(OCSP_BASICRESP *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 1054, Amissl)

#define OCSP_BASICRESP_it() \
	AROS_LC0(const ASN1_ITEM *, OCSP_BASICRESP_it, \
	struct Library *, AMISSL_BASE_NAME, 1055, Amissl)

#define OCSP_RESPDATA_new() \
	AROS_LC0(OCSP_RESPDATA *, OCSP_RESPDATA_new, \
	struct Library *, AMISSL_BASE_NAME, 1056, Amissl)

#define OCSP_RESPDATA_free(___a) \
	AROS_LC1(void, OCSP_RESPDATA_free, \
	AROS_LCA(OCSP_RESPDATA *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 1057, Amissl)

#define d2i_OCSP_RESPDATA(___a, ___in, ___len) \
	AROS_LC3(OCSP_RESPDATA *, d2i_OCSP_RESPDATA, \
	AROS_LCA(OCSP_RESPDATA **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 1058, Amissl)

#define i2d_OCSP_RESPDATA(___a, ___out) \
	AROS_LC2(int, i2d_OCSP_RESPDATA, \
	AROS_LCA(OCSP_RESPDATA *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 1059, Amissl)

#define OCSP_RESPDATA_it() \
	AROS_LC0(const ASN1_ITEM *, OCSP_RESPDATA_it, \
	struct Library *, AMISSL_BASE_NAME, 1060, Amissl)

#define OCSP_RESPID_new() \
	AROS_LC0(OCSP_RESPID *, OCSP_RESPID_new, \
	struct Library *, AMISSL_BASE_NAME, 1061, Amissl)

#define OCSP_RESPID_free(___a) \
	AROS_LC1(void, OCSP_RESPID_free, \
	AROS_LCA(OCSP_RESPID *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 1062, Amissl)

#define d2i_OCSP_RESPID(___a, ___in, ___len) \
	AROS_LC3(OCSP_RESPID *, d2i_OCSP_RESPID, \
	AROS_LCA(OCSP_RESPID **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 1063, Amissl)

#define i2d_OCSP_RESPID(___a, ___out) \
	AROS_LC2(int, i2d_OCSP_RESPID, \
	AROS_LCA(OCSP_RESPID *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 1064, Amissl)

#define OCSP_RESPID_it() \
	AROS_LC0(const ASN1_ITEM *, OCSP_RESPID_it, \
	struct Library *, AMISSL_BASE_NAME, 1065, Amissl)

#define OCSP_RESPONSE_new() \
	AROS_LC0(OCSP_RESPONSE *, OCSP_RESPONSE_new, \
	struct Library *, AMISSL_BASE_NAME, 1066, Amissl)

#define OCSP_RESPONSE_free(___a) \
	AROS_LC1(void, OCSP_RESPONSE_free, \
	AROS_LCA(OCSP_RESPONSE *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 1067, Amissl)

#define d2i_OCSP_RESPONSE(___a, ___in, ___len) \
	AROS_LC3(OCSP_RESPONSE *, d2i_OCSP_RESPONSE, \
	AROS_LCA(OCSP_RESPONSE **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 1068, Amissl)

#define i2d_OCSP_RESPONSE(___a, ___out) \
	AROS_LC2(int, i2d_OCSP_RESPONSE, \
	AROS_LCA(OCSP_RESPONSE *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 1069, Amissl)

#define OCSP_RESPONSE_it() \
	AROS_LC0(const ASN1_ITEM *, OCSP_RESPONSE_it, \
	struct Library *, AMISSL_BASE_NAME, 1070, Amissl)

#define OCSP_RESPBYTES_new() \
	AROS_LC0(OCSP_RESPBYTES *, OCSP_RESPBYTES_new, \
	struct Library *, AMISSL_BASE_NAME, 1071, Amissl)

#define OCSP_RESPBYTES_free(___a) \
	AROS_LC1(void, OCSP_RESPBYTES_free, \
	AROS_LCA(OCSP_RESPBYTES *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 1072, Amissl)

#define d2i_OCSP_RESPBYTES(___a, ___in, ___len) \
	AROS_LC3(OCSP_RESPBYTES *, d2i_OCSP_RESPBYTES, \
	AROS_LCA(OCSP_RESPBYTES **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 1073, Amissl)

#define i2d_OCSP_RESPBYTES(___a, ___out) \
	AROS_LC2(int, i2d_OCSP_RESPBYTES, \
	AROS_LCA(OCSP_RESPBYTES *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 1074, Amissl)

#define OCSP_RESPBYTES_it() \
	AROS_LC0(const ASN1_ITEM *, OCSP_RESPBYTES_it, \
	struct Library *, AMISSL_BASE_NAME, 1075, Amissl)

#define OCSP_ONEREQ_new() \
	AROS_LC0(OCSP_ONEREQ *, OCSP_ONEREQ_new, \
	struct Library *, AMISSL_BASE_NAME, 1076, Amissl)

#define OCSP_ONEREQ_free(___a) \
	AROS_LC1(void, OCSP_ONEREQ_free, \
	AROS_LCA(OCSP_ONEREQ *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 1077, Amissl)

#define d2i_OCSP_ONEREQ(___a, ___in, ___len) \
	AROS_LC3(OCSP_ONEREQ *, d2i_OCSP_ONEREQ, \
	AROS_LCA(OCSP_ONEREQ **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 1078, Amissl)

#define i2d_OCSP_ONEREQ(___a, ___out) \
	AROS_LC2(int, i2d_OCSP_ONEREQ, \
	AROS_LCA(OCSP_ONEREQ *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 1079, Amissl)

#define OCSP_ONEREQ_it() \
	AROS_LC0(const ASN1_ITEM *, OCSP_ONEREQ_it, \
	struct Library *, AMISSL_BASE_NAME, 1080, Amissl)

#define OCSP_CERTID_new() \
	AROS_LC0(OCSP_CERTID *, OCSP_CERTID_new, \
	struct Library *, AMISSL_BASE_NAME, 1081, Amissl)

#define OCSP_CERTID_free(___a) \
	AROS_LC1(void, OCSP_CERTID_free, \
	AROS_LCA(OCSP_CERTID *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 1082, Amissl)

#define d2i_OCSP_CERTID(___a, ___in, ___len) \
	AROS_LC3(OCSP_CERTID *, d2i_OCSP_CERTID, \
	AROS_LCA(OCSP_CERTID **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 1083, Amissl)

#define i2d_OCSP_CERTID(___a, ___out) \
	AROS_LC2(int, i2d_OCSP_CERTID, \
	AROS_LCA(OCSP_CERTID *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 1084, Amissl)

#define OCSP_CERTID_it() \
	AROS_LC0(const ASN1_ITEM *, OCSP_CERTID_it, \
	struct Library *, AMISSL_BASE_NAME, 1085, Amissl)

#define OCSP_REQUEST_new() \
	AROS_LC0(OCSP_REQUEST *, OCSP_REQUEST_new, \
	struct Library *, AMISSL_BASE_NAME, 1086, Amissl)

#define OCSP_REQUEST_free(___a) \
	AROS_LC1(void, OCSP_REQUEST_free, \
	AROS_LCA(OCSP_REQUEST *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 1087, Amissl)

#define d2i_OCSP_REQUEST(___a, ___in, ___len) \
	AROS_LC3(OCSP_REQUEST *, d2i_OCSP_REQUEST, \
	AROS_LCA(OCSP_REQUEST **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 1088, Amissl)

#define i2d_OCSP_REQUEST(___a, ___out) \
	AROS_LC2(int, i2d_OCSP_REQUEST, \
	AROS_LCA(OCSP_REQUEST *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 1089, Amissl)

#define OCSP_REQUEST_it() \
	AROS_LC0(const ASN1_ITEM *, OCSP_REQUEST_it, \
	struct Library *, AMISSL_BASE_NAME, 1090, Amissl)

#define OCSP_SIGNATURE_new() \
	AROS_LC0(OCSP_SIGNATURE *, OCSP_SIGNATURE_new, \
	struct Library *, AMISSL_BASE_NAME, 1091, Amissl)

#define OCSP_SIGNATURE_free(___a) \
	AROS_LC1(void, OCSP_SIGNATURE_free, \
	AROS_LCA(OCSP_SIGNATURE *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 1092, Amissl)

#define d2i_OCSP_SIGNATURE(___a, ___in, ___len) \
	AROS_LC3(OCSP_SIGNATURE *, d2i_OCSP_SIGNATURE, \
	AROS_LCA(OCSP_SIGNATURE **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 1093, Amissl)

#define i2d_OCSP_SIGNATURE(___a, ___out) \
	AROS_LC2(int, i2d_OCSP_SIGNATURE, \
	AROS_LCA(OCSP_SIGNATURE *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 1094, Amissl)

#define OCSP_SIGNATURE_it() \
	AROS_LC0(const ASN1_ITEM *, OCSP_SIGNATURE_it, \
	struct Library *, AMISSL_BASE_NAME, 1095, Amissl)

#define OCSP_REQINFO_new() \
	AROS_LC0(OCSP_REQINFO *, OCSP_REQINFO_new, \
	struct Library *, AMISSL_BASE_NAME, 1096, Amissl)

#define OCSP_REQINFO_free(___a) \
	AROS_LC1(void, OCSP_REQINFO_free, \
	AROS_LCA(OCSP_REQINFO *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 1097, Amissl)

#define d2i_OCSP_REQINFO(___a, ___in, ___len) \
	AROS_LC3(OCSP_REQINFO *, d2i_OCSP_REQINFO, \
	AROS_LCA(OCSP_REQINFO **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 1098, Amissl)

#define i2d_OCSP_REQINFO(___a, ___out) \
	AROS_LC2(int, i2d_OCSP_REQINFO, \
	AROS_LCA(OCSP_REQINFO *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 1099, Amissl)

#define OCSP_REQINFO_it() \
	AROS_LC0(const ASN1_ITEM *, OCSP_REQINFO_it, \
	struct Library *, AMISSL_BASE_NAME, 1100, Amissl)

#define OCSP_CRLID_new() \
	AROS_LC0(OCSP_CRLID *, OCSP_CRLID_new, \
	struct Library *, AMISSL_BASE_NAME, 1101, Amissl)

#define OCSP_CRLID_free(___a) \
	AROS_LC1(void, OCSP_CRLID_free, \
	AROS_LCA(OCSP_CRLID *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 1102, Amissl)

#define d2i_OCSP_CRLID(___a, ___in, ___len) \
	AROS_LC3(OCSP_CRLID *, d2i_OCSP_CRLID, \
	AROS_LCA(OCSP_CRLID **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 1103, Amissl)

#define i2d_OCSP_CRLID(___a, ___out) \
	AROS_LC2(int, i2d_OCSP_CRLID, \
	AROS_LCA(OCSP_CRLID *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 1104, Amissl)

#define OCSP_CRLID_it() \
	AROS_LC0(const ASN1_ITEM *, OCSP_CRLID_it, \
	struct Library *, AMISSL_BASE_NAME, 1105, Amissl)

#define OCSP_SERVICELOC_new() \
	AROS_LC0(OCSP_SERVICELOC *, OCSP_SERVICELOC_new, \
	struct Library *, AMISSL_BASE_NAME, 1106, Amissl)

#define OCSP_SERVICELOC_free(___a) \
	AROS_LC1(void, OCSP_SERVICELOC_free, \
	AROS_LCA(OCSP_SERVICELOC *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 1107, Amissl)

#define d2i_OCSP_SERVICELOC(___a, ___in, ___len) \
	AROS_LC3(OCSP_SERVICELOC *, d2i_OCSP_SERVICELOC, \
	AROS_LCA(OCSP_SERVICELOC **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 1108, Amissl)

#define i2d_OCSP_SERVICELOC(___a, ___out) \
	AROS_LC2(int, i2d_OCSP_SERVICELOC, \
	AROS_LCA(OCSP_SERVICELOC *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 1109, Amissl)

#define OCSP_SERVICELOC_it() \
	AROS_LC0(const ASN1_ITEM *, OCSP_SERVICELOC_it, \
	struct Library *, AMISSL_BASE_NAME, 1110, Amissl)

#define OCSP_response_status_str(___s) \
	AROS_LC1(char *, OCSP_response_status_str, \
	AROS_LCA(long, (___s), D0), \
	struct Library *, AMISSL_BASE_NAME, 1111, Amissl)

#define OCSP_cert_status_str(___s) \
	AROS_LC1(char *, OCSP_cert_status_str, \
	AROS_LCA(long, (___s), D0), \
	struct Library *, AMISSL_BASE_NAME, 1112, Amissl)

#define OCSP_crl_reason_str(___s) \
	AROS_LC1(char *, OCSP_crl_reason_str, \
	AROS_LCA(long, (___s), D0), \
	struct Library *, AMISSL_BASE_NAME, 1113, Amissl)

#define OCSP_REQUEST_print(___bp, ___a, ___flags) \
	AROS_LC3(int, OCSP_REQUEST_print, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(OCSP_REQUEST*, (___a), A1), \
	AROS_LCA(unsigned long, (___flags), D0), \
	struct Library *, AMISSL_BASE_NAME, 1114, Amissl)

#define OCSP_RESPONSE_print(___bp, ___o, ___flags) \
	AROS_LC3(int, OCSP_RESPONSE_print, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(OCSP_RESPONSE*, (___o), A1), \
	AROS_LCA(unsigned long, (___flags), D0), \
	struct Library *, AMISSL_BASE_NAME, 1115, Amissl)

#define OCSP_basic_verify(___bs, ___certs, ___st, ___flags) \
	AROS_LC4(int, OCSP_basic_verify, \
	AROS_LCA(OCSP_BASICRESP *, (___bs), A0), \
	AROS_LCA(STACK_OF(X509) *, (___certs), A1), \
	AROS_LCA(X509_STORE *, (___st), A2), \
	AROS_LCA(unsigned long, (___flags), D0), \
	struct Library *, AMISSL_BASE_NAME, 1116, Amissl)

#define ERR_load_OCSP_strings() \
	AROS_LC0(void, ERR_load_OCSP_strings, \
	struct Library *, AMISSL_BASE_NAME, 1117, Amissl)

#define PEM_get_EVP_CIPHER_INFO(___header, ___cipher) \
	AROS_LC2(int, PEM_get_EVP_CIPHER_INFO, \
	AROS_LCA(char *, (___header), A0), \
	AROS_LCA(EVP_CIPHER_INFO *, (___cipher), A1), \
	struct Library *, AMISSL_BASE_NAME, 1118, Amissl)

#define PEM_do_header(___cipher, ___data, ___len, ___callback, ___u) \
	AROS_LC5(int, PEM_do_header, \
	AROS_LCA(EVP_CIPHER_INFO *, (___cipher), A0), \
	AROS_LCA(unsigned char *, (___data), A1), \
	AROS_LCA(long *, (___len), A2), \
	AROS_LCA(pem_password_cb * (*)(char *buf,int size,int rwflag,void *userdata), (___callback), A3), \
	AROS_LCA(void *, (___u), D0), \
	struct Library *, AMISSL_BASE_NAME, 1119, Amissl)

#define PEM_read_bio(___bp, ___name, ___header, ___data, ___len) \
	AROS_LC5(int, PEM_read_bio, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(char **, (___name), A1), \
	AROS_LCA(char **, (___header), A2), \
	AROS_LCA(unsigned char **, (___data), A3), \
	AROS_LCA(long *, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 1120, Amissl)

#define PEM_write_bio(___bp, ___name, ___hdr, ___data, ___len) \
	AROS_LC5(int, PEM_write_bio, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(const char *, (___name), A1), \
	AROS_LCA(char *, (___hdr), A2), \
	AROS_LCA(unsigned char *, (___data), A3), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 1121, Amissl)

#define PEM_bytes_read_bio(___pdata, ___plen, ___pnm, ___name, ___bp, ___cb, ___u) \
	AROS_LC7(int, PEM_bytes_read_bio, \
	AROS_LCA(unsigned char **, (___pdata), A0), \
	AROS_LCA(long *, (___plen), A1), \
	AROS_LCA(char **, (___pnm), A2), \
	AROS_LCA(const char *, (___name), A3), \
	AROS_LCA(BIO *, (___bp), D0), \
	AROS_LCA(pem_password_cb * (*)(char *buf,int size,int rwflag,void *userdata), (___cb), D1), \
	AROS_LCA(void *, (___u), D2), \
	struct Library *, AMISSL_BASE_NAME, 1122, Amissl)

#define PEM_ASN1_read_bio(___d2i, ___name, ___bp, ___x, ___cb, ___u) \
	AROS_LC6(char *, PEM_ASN1_read_bio, \
	AROS_LCA(char * (*)(), (___d2i), A0), \
	AROS_LCA(const char *, (___name), A1), \
	AROS_LCA(BIO *, (___bp), A2), \
	AROS_LCA(char **, (___x), A3), \
	AROS_LCA(pem_password_cb * (*)(char *buf,int size,int rwflag,void *userdata), (___cb), D0), \
	AROS_LCA(void *, (___u), D1), \
	struct Library *, AMISSL_BASE_NAME, 1123, Amissl)

#define PEM_ASN1_write_bio(___i2d, ___name, ___bp, ___x, ___enc, ___kstr, ___klen, ___cb, ___u) \
	AROS_LC9(int, PEM_ASN1_write_bio, \
	AROS_LCA(int (*)(), (___i2d), A0), \
	AROS_LCA(const char *, (___name), A1), \
	AROS_LCA(BIO *, (___bp), A2), \
	AROS_LCA(char *, (___x), A3), \
	AROS_LCA(const EVP_CIPHER *, (___enc), D0), \
	AROS_LCA(unsigned char *, (___kstr), D1), \
	AROS_LCA(int, (___klen), D2), \
	AROS_LCA(pem_password_cb * (*)(char *buf,int size,int rwflag,void *userdata), (___cb), D3), \
	AROS_LCA(void *, (___u), D4), \
	struct Library *, AMISSL_BASE_NAME, 1124, Amissl)

#define PEM_X509_INFO_read_bio(___bp, ___sk, ___cb, ___u) \
	AROS_LC4(STACK_OF(X509_INFO) *, PEM_X509_INFO_read_bio, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(STACK_OF(X509_INFO) *, (___sk), A1), \
	AROS_LCA(pem_password_cb * (*)(char *buf,int size,int rwflag,void *userdata), (___cb), A2), \
	AROS_LCA(void *, (___u), A3), \
	struct Library *, AMISSL_BASE_NAME, 1125, Amissl)

#define PEM_X509_INFO_write_bio(___bp, ___xi, ___enc, ___kstr, ___klen, ___cd, ___u) \
	AROS_LC7(int, PEM_X509_INFO_write_bio, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(X509_INFO *, (___xi), A1), \
	AROS_LCA(EVP_CIPHER *, (___enc), A2), \
	AROS_LCA(unsigned char *, (___kstr), A3), \
	AROS_LCA(int, (___klen), D0), \
	AROS_LCA(pem_password_cb * (*)(char *buf,int size,int rwflag,void *userdata), (___cd), D1), \
	AROS_LCA(void *, (___u), D2), \
	struct Library *, AMISSL_BASE_NAME, 1126, Amissl)

#define PEM_SealInit(___ctx, ___type, ___md_type, ___ek, ___ekl, ___iv, ___pubk, ___npubk) \
	AROS_LC8(int, PEM_SealInit, \
	AROS_LCA(PEM_ENCODE_SEAL_CTX *, (___ctx), A0), \
	AROS_LCA(EVP_CIPHER *, (___type), A1), \
	AROS_LCA(EVP_MD *, (___md_type), A2), \
	AROS_LCA(unsigned char **, (___ek), A3), \
	AROS_LCA(int *, (___ekl), D0), \
	AROS_LCA(unsigned char *, (___iv), D1), \
	AROS_LCA(EVP_PKEY **, (___pubk), D2), \
	AROS_LCA(int, (___npubk), D3), \
	struct Library *, AMISSL_BASE_NAME, 1127, Amissl)

#define PEM_SealUpdate(___ctx, ___out, ___outl, ___in, ___inl) \
	AROS_LC5(void, PEM_SealUpdate, \
	AROS_LCA(PEM_ENCODE_SEAL_CTX *, (___ctx), A0), \
	AROS_LCA(unsigned char *, (___out), A1), \
	AROS_LCA(int *, (___outl), A2), \
	AROS_LCA(unsigned char *, (___in), A3), \
	AROS_LCA(int, (___inl), D0), \
	struct Library *, AMISSL_BASE_NAME, 1128, Amissl)

#define PEM_SealFinal(___ctx, ___sig, ___sigl, ___out, ___outl, ___priv) \
	AROS_LC6(int, PEM_SealFinal, \
	AROS_LCA(PEM_ENCODE_SEAL_CTX *, (___ctx), A0), \
	AROS_LCA(unsigned char *, (___sig), A1), \
	AROS_LCA(int *, (___sigl), A2), \
	AROS_LCA(unsigned char *, (___out), A3), \
	AROS_LCA(int *, (___outl), D0), \
	AROS_LCA(EVP_PKEY *, (___priv), D1), \
	struct Library *, AMISSL_BASE_NAME, 1129, Amissl)

#define PEM_SignInit(___ctx, ___type) \
	AROS_LC2(void, PEM_SignInit, \
	AROS_LCA(EVP_MD_CTX *, (___ctx), A0), \
	AROS_LCA(EVP_MD *, (___type), A1), \
	struct Library *, AMISSL_BASE_NAME, 1130, Amissl)

#define PEM_SignUpdate(___ctx, ___d, ___cnt) \
	AROS_LC3(void, PEM_SignUpdate, \
	AROS_LCA(EVP_MD_CTX *, (___ctx), A0), \
	AROS_LCA(unsigned char *, (___d), A1), \
	AROS_LCA(unsigned int, (___cnt), D0), \
	struct Library *, AMISSL_BASE_NAME, 1131, Amissl)

#define PEM_SignFinal(___ctx, ___sigret, ___siglen, ___pkey) \
	AROS_LC4(int, PEM_SignFinal, \
	AROS_LCA(EVP_MD_CTX *, (___ctx), A0), \
	AROS_LCA(unsigned char *, (___sigret), A1), \
	AROS_LCA(unsigned int *, (___siglen), A2), \
	AROS_LCA(EVP_PKEY *, (___pkey), A3), \
	struct Library *, AMISSL_BASE_NAME, 1132, Amissl)

#define PEM_def_callback(___buf, ___num, ___w, ___key) \
	AROS_LC4(int, PEM_def_callback, \
	AROS_LCA(char *, (___buf), A0), \
	AROS_LCA(int, (___num), D0), \
	AROS_LCA(int, (___w), D1), \
	AROS_LCA(void *, (___key), A1), \
	struct Library *, AMISSL_BASE_NAME, 1133, Amissl)

#define PEM_proc_type(___buf, ___type) \
	AROS_LC2(void, PEM_proc_type, \
	AROS_LCA(char *, (___buf), A0), \
	AROS_LCA(int, (___type), D0), \
	struct Library *, AMISSL_BASE_NAME, 1134, Amissl)

#define PEM_dek_info(___buf, ___type, ___len, ___str) \
	AROS_LC4(void, PEM_dek_info, \
	AROS_LCA(char *, (___buf), A0), \
	AROS_LCA(const char *, (___type), A1), \
	AROS_LCA(int, (___len), D0), \
	AROS_LCA(char *, (___str), A2), \
	struct Library *, AMISSL_BASE_NAME, 1135, Amissl)

#define PEM_read_bio_X509(___bp, ___x, ___cb, ___u) \
	AROS_LC4(X509 *, PEM_read_bio_X509, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(X509 **, (___x), A1), \
	AROS_LCA(pem_password_cb * (*)(char *buf,int size,int rwflag,void *userdata), (___cb), A2), \
	AROS_LCA(void *, (___u), A3), \
	struct Library *, AMISSL_BASE_NAME, 1136, Amissl)

#define PEM_write_bio_X509(___bp, ___x) \
	AROS_LC2(int, PEM_write_bio_X509, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(X509 *, (___x), A1), \
	struct Library *, AMISSL_BASE_NAME, 1137, Amissl)

#define PEM_read_bio_X509_AUX(___bp, ___x, ___cb, ___u) \
	AROS_LC4(X509 *, PEM_read_bio_X509_AUX, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(X509 **, (___x), A1), \
	AROS_LCA(pem_password_cb * (*)(char *buf,int size,int rwflag,void *userdata), (___cb), A2), \
	AROS_LCA(void *, (___u), A3), \
	struct Library *, AMISSL_BASE_NAME, 1138, Amissl)

#define PEM_write_bio_X509_AUX(___bp, ___x) \
	AROS_LC2(int, PEM_write_bio_X509_AUX, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(X509 *, (___x), A1), \
	struct Library *, AMISSL_BASE_NAME, 1139, Amissl)

#define PEM_read_bio_X509_REQ(___bp, ___x, ___cb, ___u) \
	AROS_LC4(X509_REQ *, PEM_read_bio_X509_REQ, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(X509_REQ **, (___x), A1), \
	AROS_LCA(pem_password_cb * (*)(char *buf,int size,int rwflag,void *userdata), (___cb), A2), \
	AROS_LCA(void *, (___u), A3), \
	struct Library *, AMISSL_BASE_NAME, 1140, Amissl)

#define PEM_write_bio_X509_REQ(___bp, ___x) \
	AROS_LC2(int, PEM_write_bio_X509_REQ, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(X509_REQ *, (___x), A1), \
	struct Library *, AMISSL_BASE_NAME, 1141, Amissl)

#define PEM_write_bio_X509_REQ_NEW(___bp, ___x) \
	AROS_LC2(int, PEM_write_bio_X509_REQ_NEW, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(X509_REQ *, (___x), A1), \
	struct Library *, AMISSL_BASE_NAME, 1142, Amissl)

#define PEM_read_bio_X509_CRL(___bp, ___x, ___cb, ___u) \
	AROS_LC4(X509_CRL *, PEM_read_bio_X509_CRL, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(X509_CRL **, (___x), A1), \
	AROS_LCA(pem_password_cb * (*)(char *buf,int size,int rwflag,void *userdata), (___cb), A2), \
	AROS_LCA(void *, (___u), A3), \
	struct Library *, AMISSL_BASE_NAME, 1143, Amissl)

#define PEM_write_bio_X509_CRL(___bp, ___x) \
	AROS_LC2(int, PEM_write_bio_X509_CRL, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(X509_CRL *, (___x), A1), \
	struct Library *, AMISSL_BASE_NAME, 1144, Amissl)

#define PEM_read_bio_PKCS7(___bp, ___x, ___cb, ___u) \
	AROS_LC4(PKCS7 *, PEM_read_bio_PKCS7, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(PKCS7 **, (___x), A1), \
	AROS_LCA(pem_password_cb * (*)(char *buf,int size,int rwflag,void *userdata), (___cb), A2), \
	AROS_LCA(void *, (___u), A3), \
	struct Library *, AMISSL_BASE_NAME, 1145, Amissl)

#define PEM_write_bio_PKCS7(___bp, ___x) \
	AROS_LC2(int, PEM_write_bio_PKCS7, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(PKCS7 *, (___x), A1), \
	struct Library *, AMISSL_BASE_NAME, 1146, Amissl)

#define PEM_read_bio_NETSCAPE_CERT_SEQUENCE(___bp, ___x, ___cb, ___u) \
	AROS_LC4(NETSCAPE_CERT_SEQUENCE *, PEM_read_bio_NETSCAPE_CERT_SEQUENCE, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(NETSCAPE_CERT_SEQUENCE **, (___x), A1), \
	AROS_LCA(pem_password_cb * (*)(char *buf,int size,int rwflag,void *userdata), (___cb), A2), \
	AROS_LCA(void *, (___u), A3), \
	struct Library *, AMISSL_BASE_NAME, 1147, Amissl)

#define PEM_write_bio_NETSCAPE_CERT_SEQUENCE(___bp, ___x) \
	AROS_LC2(int, PEM_write_bio_NETSCAPE_CERT_SEQUENCE, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(NETSCAPE_CERT_SEQUENCE *, (___x), A1), \
	struct Library *, AMISSL_BASE_NAME, 1148, Amissl)

#define PEM_read_bio_PKCS8(___bp, ___x, ___cb, ___u) \
	AROS_LC4(X509_SIG *, PEM_read_bio_PKCS8, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(X509_SIG **, (___x), A1), \
	AROS_LCA(pem_password_cb * (*)(char *buf,int size,int rwflag,void *userdata), (___cb), A2), \
	AROS_LCA(void *, (___u), A3), \
	struct Library *, AMISSL_BASE_NAME, 1149, Amissl)

#define PEM_write_bio_PKCS8(___bp, ___x) \
	AROS_LC2(int, PEM_write_bio_PKCS8, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(X509_SIG *, (___x), A1), \
	struct Library *, AMISSL_BASE_NAME, 1150, Amissl)

#define PEM_read_bio_PKCS8_PRIV_KEY_INFO(___bp, ___x, ___cb, ___u) \
	AROS_LC4(PKCS8_PRIV_KEY_INFO *, PEM_read_bio_PKCS8_PRIV_KEY_INFO, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(PKCS8_PRIV_KEY_INFO **, (___x), A1), \
	AROS_LCA(pem_password_cb * (*)(char *buf,int size,int rwflag,void *userdata), (___cb), A2), \
	AROS_LCA(void *, (___u), A3), \
	struct Library *, AMISSL_BASE_NAME, 1151, Amissl)

#define PEM_write_bio_PKCS8_PRIV_KEY_INFO(___bp, ___x) \
	AROS_LC2(int, PEM_write_bio_PKCS8_PRIV_KEY_INFO, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(PKCS8_PRIV_KEY_INFO *, (___x), A1), \
	struct Library *, AMISSL_BASE_NAME, 1152, Amissl)

#define PEM_read_bio_RSAPrivateKey(___bp, ___x, ___cb, ___u) \
	AROS_LC4(RSA *, PEM_read_bio_RSAPrivateKey, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(RSA **, (___x), A1), \
	AROS_LCA(pem_password_cb * (*)(char *buf,int size,int rwflag,void *userdata), (___cb), A2), \
	AROS_LCA(void *, (___u), A3), \
	struct Library *, AMISSL_BASE_NAME, 1153, Amissl)

#define PEM_write_bio_RSAPrivateKey(___bp, ___x, ___enc, ___kstr, ___klen, ___cb, ___u) \
	AROS_LC7(int, PEM_write_bio_RSAPrivateKey, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(RSA *, (___x), A1), \
	AROS_LCA(const EVP_CIPHER *, (___enc), A2), \
	AROS_LCA(unsigned char *, (___kstr), A3), \
	AROS_LCA(int, (___klen), D0), \
	AROS_LCA(pem_password_cb * (*)(char *buf,int size,int rwflag,void *userdata), (___cb), D1), \
	AROS_LCA(void *, (___u), D2), \
	struct Library *, AMISSL_BASE_NAME, 1154, Amissl)

#define PEM_read_bio_RSAPublicKey(___bp, ___x, ___cb, ___u) \
	AROS_LC4(RSA *, PEM_read_bio_RSAPublicKey, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(RSA **, (___x), A1), \
	AROS_LCA(pem_password_cb * (*)(char *buf,int size,int rwflag,void *userdata), (___cb), A2), \
	AROS_LCA(void *, (___u), A3), \
	struct Library *, AMISSL_BASE_NAME, 1155, Amissl)

#define PEM_write_bio_RSAPublicKey(___bp, ___x) \
	AROS_LC2(int, PEM_write_bio_RSAPublicKey, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(RSA *, (___x), A1), \
	struct Library *, AMISSL_BASE_NAME, 1156, Amissl)

#define PEM_read_bio_RSA_PUBKEY(___bp, ___x, ___cb, ___u) \
	AROS_LC4(RSA *, PEM_read_bio_RSA_PUBKEY, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(RSA **, (___x), A1), \
	AROS_LCA(pem_password_cb * (*)(char *buf,int size,int rwflag,void *userdata), (___cb), A2), \
	AROS_LCA(void *, (___u), A3), \
	struct Library *, AMISSL_BASE_NAME, 1157, Amissl)

#define PEM_write_bio_RSA_PUBKEY(___bp, ___x) \
	AROS_LC2(int, PEM_write_bio_RSA_PUBKEY, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(RSA *, (___x), A1), \
	struct Library *, AMISSL_BASE_NAME, 1158, Amissl)

#define PEM_read_bio_DSAPrivateKey(___bp, ___x, ___cb, ___u) \
	AROS_LC4(DSA *, PEM_read_bio_DSAPrivateKey, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(DSA **, (___x), A1), \
	AROS_LCA(pem_password_cb * (*)(char *buf,int size,int rwflag,void *userdata), (___cb), A2), \
	AROS_LCA(void *, (___u), A3), \
	struct Library *, AMISSL_BASE_NAME, 1159, Amissl)

#define PEM_write_bio_DSAPrivateKey(___bp, ___x, ___enc, ___kstr, ___klen, ___cb, ___u) \
	AROS_LC7(int, PEM_write_bio_DSAPrivateKey, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(DSA *, (___x), A1), \
	AROS_LCA(const EVP_CIPHER *, (___enc), A2), \
	AROS_LCA(unsigned char *, (___kstr), A3), \
	AROS_LCA(int, (___klen), D0), \
	AROS_LCA(pem_password_cb * (*)(char *buf,int size,int rwflag,void *userdata), (___cb), D1), \
	AROS_LCA(void *, (___u), D2), \
	struct Library *, AMISSL_BASE_NAME, 1160, Amissl)

#define PEM_read_bio_DSA_PUBKEY(___bp, ___x, ___cb, ___u) \
	AROS_LC4(DSA *, PEM_read_bio_DSA_PUBKEY, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(DSA **, (___x), A1), \
	AROS_LCA(pem_password_cb * (*)(char *buf,int size,int rwflag,void *userdata), (___cb), A2), \
	AROS_LCA(void *, (___u), A3), \
	struct Library *, AMISSL_BASE_NAME, 1161, Amissl)

#define PEM_write_bio_DSA_PUBKEY(___bp, ___x) \
	AROS_LC2(int, PEM_write_bio_DSA_PUBKEY, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(DSA *, (___x), A1), \
	struct Library *, AMISSL_BASE_NAME, 1162, Amissl)

#define PEM_read_bio_DSAparams(___bp, ___x, ___cb, ___u) \
	AROS_LC4(DSA *, PEM_read_bio_DSAparams, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(DSA **, (___x), A1), \
	AROS_LCA(pem_password_cb * (*)(char *buf,int size,int rwflag,void *userdata), (___cb), A2), \
	AROS_LCA(void *, (___u), A3), \
	struct Library *, AMISSL_BASE_NAME, 1163, Amissl)

#define PEM_write_bio_DSAparams(___bp, ___x) \
	AROS_LC2(int, PEM_write_bio_DSAparams, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(DSA *, (___x), A1), \
	struct Library *, AMISSL_BASE_NAME, 1164, Amissl)

#define PEM_read_bio_DHparams(___bp, ___x, ___cb, ___u) \
	AROS_LC4(DH *, PEM_read_bio_DHparams, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(DH **, (___x), A1), \
	AROS_LCA(pem_password_cb * (*)(char *buf,int size,int rwflag,void *userdata), (___cb), A2), \
	AROS_LCA(void *, (___u), A3), \
	struct Library *, AMISSL_BASE_NAME, 1165, Amissl)

#define PEM_write_bio_DHparams(___bp, ___x) \
	AROS_LC2(int, PEM_write_bio_DHparams, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(DH *, (___x), A1), \
	struct Library *, AMISSL_BASE_NAME, 1166, Amissl)

#define PEM_read_bio_PrivateKey(___bp, ___x, ___cb, ___u) \
	AROS_LC4(EVP_PKEY *, PEM_read_bio_PrivateKey, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(EVP_PKEY **, (___x), A1), \
	AROS_LCA(pem_password_cb * (*)(char *buf,int size,int rwflag,void *userdata), (___cb), A2), \
	AROS_LCA(void *, (___u), A3), \
	struct Library *, AMISSL_BASE_NAME, 1167, Amissl)

#define PEM_write_bio_PrivateKey(___bp, ___x, ___enc, ___kstr, ___klen, ___cb, ___u) \
	AROS_LC7(int, PEM_write_bio_PrivateKey, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(EVP_PKEY *, (___x), A1), \
	AROS_LCA(const EVP_CIPHER *, (___enc), A2), \
	AROS_LCA(unsigned char *, (___kstr), A3), \
	AROS_LCA(int, (___klen), D0), \
	AROS_LCA(pem_password_cb * (*)(char *buf,int size,int rwflag,void *userdata), (___cb), D1), \
	AROS_LCA(void *, (___u), D2), \
	struct Library *, AMISSL_BASE_NAME, 1168, Amissl)

#define PEM_read_bio_PUBKEY(___bp, ___x, ___cb, ___u) \
	AROS_LC4(EVP_PKEY *, PEM_read_bio_PUBKEY, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(EVP_PKEY **, (___x), A1), \
	AROS_LCA(pem_password_cb * (*)(char *buf,int size,int rwflag,void *userdata), (___cb), A2), \
	AROS_LCA(void *, (___u), A3), \
	struct Library *, AMISSL_BASE_NAME, 1169, Amissl)

#define PEM_write_bio_PUBKEY(___bp, ___x) \
	AROS_LC2(int, PEM_write_bio_PUBKEY, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(EVP_PKEY *, (___x), A1), \
	struct Library *, AMISSL_BASE_NAME, 1170, Amissl)

#define PEM_write_bio_PKCS8PrivateKey_nid(___bp, ___x, ___nid, ___kstr, ___klen, ___cb, ___u) \
	AROS_LC7(int, PEM_write_bio_PKCS8PrivateKey_nid, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(EVP_PKEY *, (___x), A1), \
	AROS_LCA(int, (___nid), D0), \
	AROS_LCA(char *, (___kstr), A2), \
	AROS_LCA(int, (___klen), D1), \
	AROS_LCA(pem_password_cb * (*)(char *buf,int size,int rwflag,void *userdata), (___cb), A3), \
	AROS_LCA(void *, (___u), D2), \
	struct Library *, AMISSL_BASE_NAME, 1171, Amissl)

#define PEM_write_bio_PKCS8PrivateKey(___a, ___b, ___c, ___d, ___a1, ___e, ___f) \
	AROS_LC7(int, PEM_write_bio_PKCS8PrivateKey, \
	AROS_LCA(BIO *, (___a), A0), \
	AROS_LCA(EVP_PKEY *, (___b), A1), \
	AROS_LCA(const EVP_CIPHER *, (___c), A2), \
	AROS_LCA(char *, (___d), A3), \
	AROS_LCA(int, (___a1), D0), \
	AROS_LCA(pem_password_cb * (*)(char *buf,int size,int rwflag,void *userdata), (___e), D1), \
	AROS_LCA(void *, (___f), D2), \
	struct Library *, AMISSL_BASE_NAME, 1172, Amissl)

#define i2d_PKCS8PrivateKey_bio(___bp, ___x, ___enc, ___kstr, ___klen, ___cb, ___u) \
	AROS_LC7(int, i2d_PKCS8PrivateKey_bio, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(EVP_PKEY *, (___x), A1), \
	AROS_LCA(const EVP_CIPHER *, (___enc), A2), \
	AROS_LCA(char *, (___kstr), A3), \
	AROS_LCA(int, (___klen), D0), \
	AROS_LCA(pem_password_cb * (*)(char *buf,int size,int rwflag,void *userdata), (___cb), D1), \
	AROS_LCA(void *, (___u), D2), \
	struct Library *, AMISSL_BASE_NAME, 1173, Amissl)

#define i2d_PKCS8PrivateKey_nid_bio(___bp, ___x, ___nid, ___kstr, ___klen, ___cb, ___u) \
	AROS_LC7(int, i2d_PKCS8PrivateKey_nid_bio, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(EVP_PKEY *, (___x), A1), \
	AROS_LCA(int, (___nid), D0), \
	AROS_LCA(char *, (___kstr), A2), \
	AROS_LCA(int, (___klen), D1), \
	AROS_LCA(pem_password_cb * (*)(char *buf,int size,int rwflag,void *userdata), (___cb), A3), \
	AROS_LCA(void *, (___u), D2), \
	struct Library *, AMISSL_BASE_NAME, 1174, Amissl)

#define d2i_PKCS8PrivateKey_bio(___bp, ___x, ___cb, ___u) \
	AROS_LC4(EVP_PKEY *, d2i_PKCS8PrivateKey_bio, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(EVP_PKEY **, (___x), A1), \
	AROS_LCA(pem_password_cb * (*)(char *buf,int size,int rwflag,void *userdata), (___cb), A2), \
	AROS_LCA(void *, (___u), A3), \
	struct Library *, AMISSL_BASE_NAME, 1175, Amissl)

#define ERR_load_PEM_strings() \
	AROS_LC0(void, ERR_load_PEM_strings, \
	struct Library *, AMISSL_BASE_NAME, 1176, Amissl)

#define PKCS12_x5092certbag(___x509) \
	AROS_LC1(PKCS12_SAFEBAG *, PKCS12_x5092certbag, \
	AROS_LCA(X509 *, (___x509), A0), \
	struct Library *, AMISSL_BASE_NAME, 1177, Amissl)

#define PKCS12_x509crl2certbag(___crl) \
	AROS_LC1(PKCS12_SAFEBAG *, PKCS12_x509crl2certbag, \
	AROS_LCA(X509_CRL *, (___crl), A0), \
	struct Library *, AMISSL_BASE_NAME, 1178, Amissl)

#define PKCS12_certbag2x509(___bag) \
	AROS_LC1(X509 *, PKCS12_certbag2x509, \
	AROS_LCA(PKCS12_SAFEBAG *, (___bag), A0), \
	struct Library *, AMISSL_BASE_NAME, 1179, Amissl)

#define PKCS12_certbag2x509crl(___bag) \
	AROS_LC1(X509_CRL *, PKCS12_certbag2x509crl, \
	AROS_LCA(PKCS12_SAFEBAG *, (___bag), A0), \
	struct Library *, AMISSL_BASE_NAME, 1180, Amissl)

#define PKCS12_item_pack_safebag(___obj, ___it, ___nid1, ___nid2) \
	AROS_LC4(PKCS12_SAFEBAG *, PKCS12_item_pack_safebag, \
	AROS_LCA(void *, (___obj), A0), \
	AROS_LCA(const ASN1_ITEM *, (___it), A1), \
	AROS_LCA(int, (___nid1), D0), \
	AROS_LCA(int, (___nid2), D1), \
	struct Library *, AMISSL_BASE_NAME, 1181, Amissl)

#define PKCS12_MAKE_KEYBAG(___p8) \
	AROS_LC1(PKCS12_SAFEBAG *, PKCS12_MAKE_KEYBAG, \
	AROS_LCA(PKCS8_PRIV_KEY_INFO *, (___p8), A0), \
	struct Library *, AMISSL_BASE_NAME, 1182, Amissl)

#define PKCS8_decrypt(___p8, ___pass, ___passlen) \
	AROS_LC3(PKCS8_PRIV_KEY_INFO *, PKCS8_decrypt, \
	AROS_LCA(X509_SIG *, (___p8), A0), \
	AROS_LCA(const char *, (___pass), A1), \
	AROS_LCA(int, (___passlen), D0), \
	struct Library *, AMISSL_BASE_NAME, 1183, Amissl)

#define PKCS12_decrypt_skey(___bag, ___pass, ___passlen) \
	AROS_LC3(PKCS8_PRIV_KEY_INFO *, PKCS12_decrypt_skey, \
	AROS_LCA(PKCS12_SAFEBAG *, (___bag), A0), \
	AROS_LCA(const char *, (___pass), A1), \
	AROS_LCA(int, (___passlen), D0), \
	struct Library *, AMISSL_BASE_NAME, 1184, Amissl)

#define PKCS8_encrypt(___pbe_nid, ___cipher, ___pass, ___passlen, ___salt, ___saltlen, ___iter, ___p8) \
	AROS_LC8(X509_SIG *, PKCS8_encrypt, \
	AROS_LCA(int, (___pbe_nid), D0), \
	AROS_LCA(const EVP_CIPHER *, (___cipher), A0), \
	AROS_LCA(const char *, (___pass), A1), \
	AROS_LCA(int, (___passlen), D1), \
	AROS_LCA(unsigned char *, (___salt), A2), \
	AROS_LCA(int, (___saltlen), D2), \
	AROS_LCA(int, (___iter), D3), \
	AROS_LCA(PKCS8_PRIV_KEY_INFO *, (___p8), A3), \
	struct Library *, AMISSL_BASE_NAME, 1185, Amissl)

#define PKCS12_MAKE_SHKEYBAG(___pbe_nid, ___pass, ___passlen, ___salt, ___saltlen, ___iter, ___p8) \
	AROS_LC7(PKCS12_SAFEBAG *, PKCS12_MAKE_SHKEYBAG, \
	AROS_LCA(int, (___pbe_nid), D0), \
	AROS_LCA(const char *, (___pass), A0), \
	AROS_LCA(int, (___passlen), D1), \
	AROS_LCA(unsigned char *, (___salt), A1), \
	AROS_LCA(int, (___saltlen), D2), \
	AROS_LCA(int, (___iter), D3), \
	AROS_LCA(PKCS8_PRIV_KEY_INFO *, (___p8), A2), \
	struct Library *, AMISSL_BASE_NAME, 1186, Amissl)

#define PKCS12_pack_p7data(___sk) \
	AROS_LC1(PKCS7 *, PKCS12_pack_p7data, \
	AROS_LCA(STACK_OF(PKCS12_SAFEBAG) *, (___sk), A0), \
	struct Library *, AMISSL_BASE_NAME, 1187, Amissl)

#define PKCS12_unpack_p7data(___p7) \
	AROS_LC1(STACK_OF(PKCS12_SAFEBAG) *, PKCS12_unpack_p7data, \
	AROS_LCA(PKCS7 *, (___p7), A0), \
	struct Library *, AMISSL_BASE_NAME, 1188, Amissl)

#define PKCS12_pack_p7encdata(___pbe_nid, ___pass, ___passlen, ___salt, ___saltlen, ___iter, ___bags) \
	AROS_LC7(PKCS7 *, PKCS12_pack_p7encdata, \
	AROS_LCA(int, (___pbe_nid), D0), \
	AROS_LCA(const char *, (___pass), A0), \
	AROS_LCA(int, (___passlen), D1), \
	AROS_LCA(unsigned char *, (___salt), A1), \
	AROS_LCA(int, (___saltlen), D2), \
	AROS_LCA(int, (___iter), D3), \
	AROS_LCA(STACK_OF(PKCS12_SAFEBAG) *, (___bags), A2), \
	struct Library *, AMISSL_BASE_NAME, 1189, Amissl)

#define PKCS12_unpack_p7encdata(___p7, ___pass, ___passlen) \
	AROS_LC3(STACK_OF(PKCS12_SAFEBAG) *, PKCS12_unpack_p7encdata, \
	AROS_LCA(PKCS7 *, (___p7), A0), \
	AROS_LCA(const char *, (___pass), A1), \
	AROS_LCA(int, (___passlen), D0), \
	struct Library *, AMISSL_BASE_NAME, 1190, Amissl)

#define PKCS12_pack_authsafes(___p12, ___safes) \
	AROS_LC2(int, PKCS12_pack_authsafes, \
	AROS_LCA(PKCS12 *, (___p12), A0), \
	AROS_LCA(STACK_OF(PKCS7) *, (___safes), A1), \
	struct Library *, AMISSL_BASE_NAME, 1191, Amissl)

#define PKCS12_unpack_authsafes(___p12) \
	AROS_LC1(STACK_OF(PKCS7) *, PKCS12_unpack_authsafes, \
	AROS_LCA(PKCS12 *, (___p12), A0), \
	struct Library *, AMISSL_BASE_NAME, 1192, Amissl)

#define PKCS12_add_localkeyid(___bag, ___name, ___namelen) \
	AROS_LC3(int, PKCS12_add_localkeyid, \
	AROS_LCA(PKCS12_SAFEBAG *, (___bag), A0), \
	AROS_LCA(unsigned char *, (___name), A1), \
	AROS_LCA(int, (___namelen), D0), \
	struct Library *, AMISSL_BASE_NAME, 1193, Amissl)

#define PKCS12_add_friendlyname_asc(___bag, ___name, ___namelen) \
	AROS_LC3(int, PKCS12_add_friendlyname_asc, \
	AROS_LCA(PKCS12_SAFEBAG *, (___bag), A0), \
	AROS_LCA(const char *, (___name), A1), \
	AROS_LCA(int, (___namelen), D0), \
	struct Library *, AMISSL_BASE_NAME, 1194, Amissl)

#define PKCS12_add_CSPName_asc(___bag, ___name, ___namelen) \
	AROS_LC3(int, PKCS12_add_CSPName_asc, \
	AROS_LCA(PKCS12_SAFEBAG *, (___bag), A0), \
	AROS_LCA(const char *, (___name), A1), \
	AROS_LCA(int, (___namelen), D0), \
	struct Library *, AMISSL_BASE_NAME, 1195, Amissl)

#define PKCS12_add_friendlyname_uni(___bag, ___name, ___namelen) \
	AROS_LC3(int, PKCS12_add_friendlyname_uni, \
	AROS_LCA(PKCS12_SAFEBAG *, (___bag), A0), \
	AROS_LCA(const unsigned char *, (___name), A1), \
	AROS_LCA(int, (___namelen), D0), \
	struct Library *, AMISSL_BASE_NAME, 1196, Amissl)

#define PKCS8_add_keyusage(___p8, ___usage) \
	AROS_LC2(int, PKCS8_add_keyusage, \
	AROS_LCA(PKCS8_PRIV_KEY_INFO *, (___p8), A0), \
	AROS_LCA(int, (___usage), D0), \
	struct Library *, AMISSL_BASE_NAME, 1197, Amissl)

#define PKCS12_get_attr_gen(___attrs, ___attr_nid) \
	AROS_LC2(ASN1_TYPE *, PKCS12_get_attr_gen, \
	AROS_LCA(STACK_OF(X509_ATTRIBUTE) *, (___attrs), A0), \
	AROS_LCA(int, (___attr_nid), D0), \
	struct Library *, AMISSL_BASE_NAME, 1198, Amissl)

#define PKCS12_get_friendlyname(___bag) \
	AROS_LC1(char *, PKCS12_get_friendlyname, \
	AROS_LCA(PKCS12_SAFEBAG *, (___bag), A0), \
	struct Library *, AMISSL_BASE_NAME, 1199, Amissl)

#define PKCS12_pbe_crypt(___algor, ___pass, ___passlen, ___in, ___inlen, ___data, ___datalen, ___en_de) \
	AROS_LC8(unsigned char *, PKCS12_pbe_crypt, \
	AROS_LCA(X509_ALGOR *, (___algor), A0), \
	AROS_LCA(const char *, (___pass), A1), \
	AROS_LCA(int, (___passlen), D0), \
	AROS_LCA(unsigned char *, (___in), A2), \
	AROS_LCA(int, (___inlen), D1), \
	AROS_LCA(unsigned char **, (___data), A3), \
	AROS_LCA(int *, (___datalen), D2), \
	AROS_LCA(int, (___en_de), D3), \
	struct Library *, AMISSL_BASE_NAME, 1200, Amissl)

#define PKCS12_item_decrypt_d2i(___algor, ___it, ___pass, ___passlen, ___oct, ___zbuf) \
	AROS_LC6(void *, PKCS12_item_decrypt_d2i, \
	AROS_LCA(X509_ALGOR *, (___algor), A0), \
	AROS_LCA(const ASN1_ITEM *, (___it), A1), \
	AROS_LCA(const char *, (___pass), A2), \
	AROS_LCA(int, (___passlen), D0), \
	AROS_LCA(ASN1_OCTET_STRING *, (___oct), A3), \
	AROS_LCA(int, (___zbuf), D1), \
	struct Library *, AMISSL_BASE_NAME, 1201, Amissl)

#define PKCS12_item_i2d_encrypt(___algor, ___it, ___pass, ___passlen, ___obj, ___zbuf) \
	AROS_LC6(ASN1_OCTET_STRING *, PKCS12_item_i2d_encrypt, \
	AROS_LCA(X509_ALGOR *, (___algor), A0), \
	AROS_LCA(const ASN1_ITEM *, (___it), A1), \
	AROS_LCA(const char *, (___pass), A2), \
	AROS_LCA(int, (___passlen), D0), \
	AROS_LCA(void *, (___obj), A3), \
	AROS_LCA(int, (___zbuf), D1), \
	struct Library *, AMISSL_BASE_NAME, 1202, Amissl)

#define PKCS12_init(___mode) \
	AROS_LC1(PKCS12 *, PKCS12_init, \
	AROS_LCA(int, (___mode), D0), \
	struct Library *, AMISSL_BASE_NAME, 1203, Amissl)

#define PKCS12_key_gen_asc(___pass, ___passlen, ___salt, ___saltlen, ___id, ___iter, ___n, ___out, ___md_type) \
	AROS_LC9(int, PKCS12_key_gen_asc, \
	AROS_LCA(const char *, (___pass), A0), \
	AROS_LCA(int, (___passlen), D0), \
	AROS_LCA(unsigned char *, (___salt), A1), \
	AROS_LCA(int, (___saltlen), D1), \
	AROS_LCA(int, (___id), D2), \
	AROS_LCA(int, (___iter), D3), \
	AROS_LCA(int, (___n), D4), \
	AROS_LCA(unsigned char *, (___out), A2), \
	AROS_LCA(const EVP_MD *, (___md_type), A3), \
	struct Library *, AMISSL_BASE_NAME, 1204, Amissl)

#define PKCS12_key_gen_uni(___pass, ___passlen, ___salt, ___saltlen, ___id, ___iter, ___n, ___out, ___md_type) \
	AROS_LC9(int, PKCS12_key_gen_uni, \
	AROS_LCA(unsigned char *, (___pass), A0), \
	AROS_LCA(int, (___passlen), D0), \
	AROS_LCA(unsigned char *, (___salt), A1), \
	AROS_LCA(int, (___saltlen), D1), \
	AROS_LCA(int, (___id), D2), \
	AROS_LCA(int, (___iter), D3), \
	AROS_LCA(int, (___n), D4), \
	AROS_LCA(unsigned char *, (___out), A2), \
	AROS_LCA(const EVP_MD *, (___md_type), A3), \
	struct Library *, AMISSL_BASE_NAME, 1205, Amissl)

#define PKCS12_PBE_keyivgen(___ctx, ___pass, ___passlen, ___param, ___cipher, ___md_type, ___en_de) \
	AROS_LC7(int, PKCS12_PBE_keyivgen, \
	AROS_LCA(EVP_CIPHER_CTX *, (___ctx), A0), \
	AROS_LCA(const char *, (___pass), A1), \
	AROS_LCA(int, (___passlen), D0), \
	AROS_LCA(ASN1_TYPE *, (___param), A2), \
	AROS_LCA(const EVP_CIPHER *, (___cipher), A3), \
	AROS_LCA(const EVP_MD *, (___md_type), D1), \
	AROS_LCA(int, (___en_de), D2), \
	struct Library *, AMISSL_BASE_NAME, 1206, Amissl)

#define PKCS12_gen_mac(___p12, ___pass, ___passlen, ___mac, ___maclen) \
	AROS_LC5(int, PKCS12_gen_mac, \
	AROS_LCA(PKCS12 *, (___p12), A0), \
	AROS_LCA(const char *, (___pass), A1), \
	AROS_LCA(int, (___passlen), D0), \
	AROS_LCA(unsigned char *, (___mac), A2), \
	AROS_LCA(unsigned int *, (___maclen), A3), \
	struct Library *, AMISSL_BASE_NAME, 1207, Amissl)

#define PKCS12_verify_mac(___p12, ___pass, ___passlen) \
	AROS_LC3(int, PKCS12_verify_mac, \
	AROS_LCA(PKCS12 *, (___p12), A0), \
	AROS_LCA(const char *, (___pass), A1), \
	AROS_LCA(int, (___passlen), D0), \
	struct Library *, AMISSL_BASE_NAME, 1208, Amissl)

#define PKCS12_set_mac(___p12, ___pass, ___passlen, ___salt, ___saltlen, ___iter, ___md_type) \
	AROS_LC7(int, PKCS12_set_mac, \
	AROS_LCA(PKCS12 *, (___p12), A0), \
	AROS_LCA(const char *, (___pass), A1), \
	AROS_LCA(int, (___passlen), D0), \
	AROS_LCA(unsigned char *, (___salt), A2), \
	AROS_LCA(int, (___saltlen), D1), \
	AROS_LCA(int, (___iter), D2), \
	AROS_LCA(const EVP_MD *, (___md_type), A3), \
	struct Library *, AMISSL_BASE_NAME, 1209, Amissl)

#define PKCS12_setup_mac(___p12, ___iter, ___salt, ___saltlen, ___md_type) \
	AROS_LC5(int, PKCS12_setup_mac, \
	AROS_LCA(PKCS12 *, (___p12), A0), \
	AROS_LCA(int, (___iter), D0), \
	AROS_LCA(unsigned char *, (___salt), A1), \
	AROS_LCA(int, (___saltlen), D1), \
	AROS_LCA(const EVP_MD *, (___md_type), A2), \
	struct Library *, AMISSL_BASE_NAME, 1210, Amissl)

#define asc2uni(___asc, ___asclen, ___uni, ___unilen) \
	AROS_LC4(unsigned char *, asc2uni, \
	AROS_LCA(const char *, (___asc), A0), \
	AROS_LCA(int, (___asclen), D0), \
	AROS_LCA(unsigned char **, (___uni), A1), \
	AROS_LCA(int *, (___unilen), A2), \
	struct Library *, AMISSL_BASE_NAME, 1211, Amissl)

#define uni2asc(___uni, ___unilen) \
	AROS_LC2(char *, uni2asc, \
	AROS_LCA(unsigned char *, (___uni), A0), \
	AROS_LCA(int, (___unilen), D0), \
	struct Library *, AMISSL_BASE_NAME, 1212, Amissl)

#define PKCS12_new() \
	AROS_LC0(PKCS12 *, PKCS12_new, \
	struct Library *, AMISSL_BASE_NAME, 1213, Amissl)

#define PKCS12_free(___a) \
	AROS_LC1(void, PKCS12_free, \
	AROS_LCA(PKCS12 *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 1214, Amissl)

#define d2i_PKCS12(___a, ___in, ___len) \
	AROS_LC3(PKCS12 *, d2i_PKCS12, \
	AROS_LCA(PKCS12 **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 1215, Amissl)

#define i2d_PKCS12(___a, ___out) \
	AROS_LC2(int, i2d_PKCS12, \
	AROS_LCA(PKCS12 *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 1216, Amissl)

#define PKCS12_it() \
	AROS_LC0(const ASN1_ITEM *, PKCS12_it, \
	struct Library *, AMISSL_BASE_NAME, 1217, Amissl)

#define PKCS12_MAC_DATA_new() \
	AROS_LC0(PKCS12_MAC_DATA *, PKCS12_MAC_DATA_new, \
	struct Library *, AMISSL_BASE_NAME, 1218, Amissl)

#define PKCS12_MAC_DATA_free(___a) \
	AROS_LC1(void, PKCS12_MAC_DATA_free, \
	AROS_LCA(PKCS12_MAC_DATA *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 1219, Amissl)

#define d2i_PKCS12_MAC_DATA(___a, ___in, ___len) \
	AROS_LC3(PKCS12_MAC_DATA *, d2i_PKCS12_MAC_DATA, \
	AROS_LCA(PKCS12_MAC_DATA **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 1220, Amissl)

#define i2d_PKCS12_MAC_DATA(___a, ___out) \
	AROS_LC2(int, i2d_PKCS12_MAC_DATA, \
	AROS_LCA(PKCS12_MAC_DATA *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 1221, Amissl)

#define PKCS12_MAC_DATA_it() \
	AROS_LC0(const ASN1_ITEM *, PKCS12_MAC_DATA_it, \
	struct Library *, AMISSL_BASE_NAME, 1222, Amissl)

#define PKCS12_SAFEBAG_new() \
	AROS_LC0(PKCS12_SAFEBAG *, PKCS12_SAFEBAG_new, \
	struct Library *, AMISSL_BASE_NAME, 1223, Amissl)

#define PKCS12_SAFEBAG_free(___a) \
	AROS_LC1(void, PKCS12_SAFEBAG_free, \
	AROS_LCA(PKCS12_SAFEBAG *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 1224, Amissl)

#define d2i_PKCS12_SAFEBAG(___a, ___in, ___len) \
	AROS_LC3(PKCS12_SAFEBAG *, d2i_PKCS12_SAFEBAG, \
	AROS_LCA(PKCS12_SAFEBAG **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 1225, Amissl)

#define i2d_PKCS12_SAFEBAG(___a, ___out) \
	AROS_LC2(int, i2d_PKCS12_SAFEBAG, \
	AROS_LCA(PKCS12_SAFEBAG *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 1226, Amissl)

#define PKCS12_SAFEBAG_it() \
	AROS_LC0(const ASN1_ITEM *, PKCS12_SAFEBAG_it, \
	struct Library *, AMISSL_BASE_NAME, 1227, Amissl)

#define PKCS12_BAGS_new() \
	AROS_LC0(PKCS12_BAGS *, PKCS12_BAGS_new, \
	struct Library *, AMISSL_BASE_NAME, 1228, Amissl)

#define PKCS12_BAGS_free(___a) \
	AROS_LC1(void, PKCS12_BAGS_free, \
	AROS_LCA(PKCS12_BAGS *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 1229, Amissl)

#define d2i_PKCS12_BAGS(___a, ___in, ___len) \
	AROS_LC3(PKCS12_BAGS *, d2i_PKCS12_BAGS, \
	AROS_LCA(PKCS12_BAGS **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 1230, Amissl)

#define i2d_PKCS12_BAGS(___a, ___out) \
	AROS_LC2(int, i2d_PKCS12_BAGS, \
	AROS_LCA(PKCS12_BAGS *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 1231, Amissl)

#define PKCS12_BAGS_it() \
	AROS_LC0(const ASN1_ITEM *, PKCS12_BAGS_it, \
	struct Library *, AMISSL_BASE_NAME, 1232, Amissl)

#define PKCS12_SAFEBAGS_it() \
	AROS_LC0(const ASN1_ITEM *, PKCS12_SAFEBAGS_it, \
	struct Library *, AMISSL_BASE_NAME, 1233, Amissl)

#define PKCS12_AUTHSAFES_it() \
	AROS_LC0(const ASN1_ITEM *, PKCS12_AUTHSAFES_it, \
	struct Library *, AMISSL_BASE_NAME, 1234, Amissl)

#define PKCS12_PBE_add() \
	AROS_LC0(void, PKCS12_PBE_add, \
	struct Library *, AMISSL_BASE_NAME, 1235, Amissl)

#define PKCS12_parse(___p12, ___pass, ___pkey, ___cert, ___ca) \
	AROS_LC5(int, PKCS12_parse, \
	AROS_LCA(PKCS12 *, (___p12), A0), \
	AROS_LCA(const char *, (___pass), A1), \
	AROS_LCA(EVP_PKEY **, (___pkey), A2), \
	AROS_LCA(X509 **, (___cert), A3), \
	AROS_LCA(STACK_OF(X509) **, (___ca), D0), \
	struct Library *, AMISSL_BASE_NAME, 1236, Amissl)

#define PKCS12_create(___pass, ___name, ___pkey, ___cert, ___ca, ___nid_key, ___nid_cert, ___iter, ___mac_iter, ___keytype) \
	AROS_LC10(PKCS12 *, PKCS12_create, \
	AROS_LCA(char *, (___pass), A0), \
	AROS_LCA(char *, (___name), A1), \
	AROS_LCA(EVP_PKEY *, (___pkey), A2), \
	AROS_LCA(X509 *, (___cert), A3), \
	AROS_LCA(STACK_OF(X509) *, (___ca), D0), \
	AROS_LCA(int, (___nid_key), D1), \
	AROS_LCA(int, (___nid_cert), D2), \
	AROS_LCA(int, (___iter), D3), \
	AROS_LCA(int, (___mac_iter), D4), \
	AROS_LCA(int, (___keytype), D5), \
	struct Library *, AMISSL_BASE_NAME, 1237, Amissl)

#define i2d_PKCS12_bio(___bp, ___p12) \
	AROS_LC2(int, i2d_PKCS12_bio, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(PKCS12 *, (___p12), A1), \
	struct Library *, AMISSL_BASE_NAME, 1238, Amissl)

#define d2i_PKCS12_bio(___bp, ___p12) \
	AROS_LC2(PKCS12 *, d2i_PKCS12_bio, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(PKCS12 **, (___p12), A1), \
	struct Library *, AMISSL_BASE_NAME, 1239, Amissl)

#define PKCS12_newpass(___p12, ___oldpass, ___newpass) \
	AROS_LC3(int, PKCS12_newpass, \
	AROS_LCA(PKCS12 *, (___p12), A0), \
	AROS_LCA(char *, (___oldpass), A1), \
	AROS_LCA(char *, (___newpass), A2), \
	struct Library *, AMISSL_BASE_NAME, 1240, Amissl)

#define ERR_load_PKCS12_strings() \
	AROS_LC0(void, ERR_load_PKCS12_strings, \
	struct Library *, AMISSL_BASE_NAME, 1241, Amissl)

#define PKCS7_ISSUER_AND_SERIAL_digest(___data, ___type, ___md, ___len) \
	AROS_LC4(int, PKCS7_ISSUER_AND_SERIAL_digest, \
	AROS_LCA(PKCS7_ISSUER_AND_SERIAL *, (___data), A0), \
	AROS_LCA(const EVP_MD *, (___type), A1), \
	AROS_LCA(unsigned char *, (___md), A2), \
	AROS_LCA(unsigned int *, (___len), A3), \
	struct Library *, AMISSL_BASE_NAME, 1242, Amissl)

#define PKCS7_dup(___p7) \
	AROS_LC1(PKCS7 *, PKCS7_dup, \
	AROS_LCA(PKCS7 *, (___p7), A0), \
	struct Library *, AMISSL_BASE_NAME, 1243, Amissl)

#define d2i_PKCS7_bio(___bp, ___p7) \
	AROS_LC2(PKCS7 *, d2i_PKCS7_bio, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(PKCS7 **, (___p7), A1), \
	struct Library *, AMISSL_BASE_NAME, 1244, Amissl)

#define i2d_PKCS7_bio(___bp, ___p7) \
	AROS_LC2(int, i2d_PKCS7_bio, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(PKCS7 *, (___p7), A1), \
	struct Library *, AMISSL_BASE_NAME, 1245, Amissl)

#define PKCS7_ISSUER_AND_SERIAL_new() \
	AROS_LC0(PKCS7_ISSUER_AND_SERIAL *, PKCS7_ISSUER_AND_SERIAL_new, \
	struct Library *, AMISSL_BASE_NAME, 1246, Amissl)

#define PKCS7_ISSUER_AND_SERIAL_free(___a) \
	AROS_LC1(void, PKCS7_ISSUER_AND_SERIAL_free, \
	AROS_LCA(PKCS7_ISSUER_AND_SERIAL *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 1247, Amissl)

#define d2i_PKCS7_ISSUER_AND_SERIAL(___a, ___in, ___len) \
	AROS_LC3(PKCS7_ISSUER_AND_SERIAL *, d2i_PKCS7_ISSUER_AND_SERIAL, \
	AROS_LCA(PKCS7_ISSUER_AND_SERIAL **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 1248, Amissl)

#define i2d_PKCS7_ISSUER_AND_SERIAL(___a, ___out) \
	AROS_LC2(int, i2d_PKCS7_ISSUER_AND_SERIAL, \
	AROS_LCA(PKCS7_ISSUER_AND_SERIAL *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 1249, Amissl)

#define PKCS7_ISSUER_AND_SERIAL_it() \
	AROS_LC0(const ASN1_ITEM *, PKCS7_ISSUER_AND_SERIAL_it, \
	struct Library *, AMISSL_BASE_NAME, 1250, Amissl)

#define PKCS7_SIGNER_INFO_new() \
	AROS_LC0(PKCS7_SIGNER_INFO *, PKCS7_SIGNER_INFO_new, \
	struct Library *, AMISSL_BASE_NAME, 1251, Amissl)

#define PKCS7_SIGNER_INFO_free(___a) \
	AROS_LC1(void, PKCS7_SIGNER_INFO_free, \
	AROS_LCA(PKCS7_SIGNER_INFO *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 1252, Amissl)

#define d2i_PKCS7_SIGNER_INFO(___a, ___in, ___len) \
	AROS_LC3(PKCS7_SIGNER_INFO *, d2i_PKCS7_SIGNER_INFO, \
	AROS_LCA(PKCS7_SIGNER_INFO **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 1253, Amissl)

#define i2d_PKCS7_SIGNER_INFO(___a, ___out) \
	AROS_LC2(int, i2d_PKCS7_SIGNER_INFO, \
	AROS_LCA(PKCS7_SIGNER_INFO *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 1254, Amissl)

#define PKCS7_SIGNER_INFO_it() \
	AROS_LC0(const ASN1_ITEM *, PKCS7_SIGNER_INFO_it, \
	struct Library *, AMISSL_BASE_NAME, 1255, Amissl)

#define PKCS7_RECIP_INFO_new() \
	AROS_LC0(PKCS7_RECIP_INFO *, PKCS7_RECIP_INFO_new, \
	struct Library *, AMISSL_BASE_NAME, 1256, Amissl)

#define PKCS7_RECIP_INFO_free(___a) \
	AROS_LC1(void, PKCS7_RECIP_INFO_free, \
	AROS_LCA(PKCS7_RECIP_INFO *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 1257, Amissl)

#define d2i_PKCS7_RECIP_INFO(___a, ___in, ___len) \
	AROS_LC3(PKCS7_RECIP_INFO *, d2i_PKCS7_RECIP_INFO, \
	AROS_LCA(PKCS7_RECIP_INFO **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 1258, Amissl)

#define i2d_PKCS7_RECIP_INFO(___a, ___out) \
	AROS_LC2(int, i2d_PKCS7_RECIP_INFO, \
	AROS_LCA(PKCS7_RECIP_INFO *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 1259, Amissl)

#define PKCS7_RECIP_INFO_it() \
	AROS_LC0(const ASN1_ITEM *, PKCS7_RECIP_INFO_it, \
	struct Library *, AMISSL_BASE_NAME, 1260, Amissl)

#define PKCS7_SIGNED_new() \
	AROS_LC0(PKCS7_SIGNED *, PKCS7_SIGNED_new, \
	struct Library *, AMISSL_BASE_NAME, 1261, Amissl)

#define PKCS7_SIGNED_free(___a) \
	AROS_LC1(void, PKCS7_SIGNED_free, \
	AROS_LCA(PKCS7_SIGNED *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 1262, Amissl)

#define d2i_PKCS7_SIGNED(___a, ___in, ___len) \
	AROS_LC3(PKCS7_SIGNED *, d2i_PKCS7_SIGNED, \
	AROS_LCA(PKCS7_SIGNED **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 1263, Amissl)

#define i2d_PKCS7_SIGNED(___a, ___out) \
	AROS_LC2(int, i2d_PKCS7_SIGNED, \
	AROS_LCA(PKCS7_SIGNED *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 1264, Amissl)

#define PKCS7_SIGNED_it() \
	AROS_LC0(const ASN1_ITEM *, PKCS7_SIGNED_it, \
	struct Library *, AMISSL_BASE_NAME, 1265, Amissl)

#define PKCS7_ENC_CONTENT_new() \
	AROS_LC0(PKCS7_ENC_CONTENT *, PKCS7_ENC_CONTENT_new, \
	struct Library *, AMISSL_BASE_NAME, 1266, Amissl)

#define PKCS7_ENC_CONTENT_free(___a) \
	AROS_LC1(void, PKCS7_ENC_CONTENT_free, \
	AROS_LCA(PKCS7_ENC_CONTENT *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 1267, Amissl)

#define d2i_PKCS7_ENC_CONTENT(___a, ___in, ___len) \
	AROS_LC3(PKCS7_ENC_CONTENT *, d2i_PKCS7_ENC_CONTENT, \
	AROS_LCA(PKCS7_ENC_CONTENT **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 1268, Amissl)

#define i2d_PKCS7_ENC_CONTENT(___a, ___out) \
	AROS_LC2(int, i2d_PKCS7_ENC_CONTENT, \
	AROS_LCA(PKCS7_ENC_CONTENT *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 1269, Amissl)

#define PKCS7_ENC_CONTENT_it() \
	AROS_LC0(const ASN1_ITEM *, PKCS7_ENC_CONTENT_it, \
	struct Library *, AMISSL_BASE_NAME, 1270, Amissl)

#define PKCS7_ENVELOPE_new() \
	AROS_LC0(PKCS7_ENVELOPE *, PKCS7_ENVELOPE_new, \
	struct Library *, AMISSL_BASE_NAME, 1271, Amissl)

#define PKCS7_ENVELOPE_free(___a) \
	AROS_LC1(void, PKCS7_ENVELOPE_free, \
	AROS_LCA(PKCS7_ENVELOPE *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 1272, Amissl)

#define d2i_PKCS7_ENVELOPE(___a, ___in, ___len) \
	AROS_LC3(PKCS7_ENVELOPE *, d2i_PKCS7_ENVELOPE, \
	AROS_LCA(PKCS7_ENVELOPE **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 1273, Amissl)

#define i2d_PKCS7_ENVELOPE(___a, ___out) \
	AROS_LC2(int, i2d_PKCS7_ENVELOPE, \
	AROS_LCA(PKCS7_ENVELOPE *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 1274, Amissl)

#define PKCS7_ENVELOPE_it() \
	AROS_LC0(const ASN1_ITEM *, PKCS7_ENVELOPE_it, \
	struct Library *, AMISSL_BASE_NAME, 1275, Amissl)

#define PKCS7_SIGN_ENVELOPE_new() \
	AROS_LC0(PKCS7_SIGN_ENVELOPE *, PKCS7_SIGN_ENVELOPE_new, \
	struct Library *, AMISSL_BASE_NAME, 1276, Amissl)

#define PKCS7_SIGN_ENVELOPE_free(___a) \
	AROS_LC1(void, PKCS7_SIGN_ENVELOPE_free, \
	AROS_LCA(PKCS7_SIGN_ENVELOPE *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 1277, Amissl)

#define d2i_PKCS7_SIGN_ENVELOPE(___a, ___in, ___len) \
	AROS_LC3(PKCS7_SIGN_ENVELOPE *, d2i_PKCS7_SIGN_ENVELOPE, \
	AROS_LCA(PKCS7_SIGN_ENVELOPE **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 1278, Amissl)

#define i2d_PKCS7_SIGN_ENVELOPE(___a, ___out) \
	AROS_LC2(int, i2d_PKCS7_SIGN_ENVELOPE, \
	AROS_LCA(PKCS7_SIGN_ENVELOPE *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 1279, Amissl)

#define PKCS7_SIGN_ENVELOPE_it() \
	AROS_LC0(const ASN1_ITEM *, PKCS7_SIGN_ENVELOPE_it, \
	struct Library *, AMISSL_BASE_NAME, 1280, Amissl)

#define PKCS7_DIGEST_new() \
	AROS_LC0(PKCS7_DIGEST *, PKCS7_DIGEST_new, \
	struct Library *, AMISSL_BASE_NAME, 1281, Amissl)

#define PKCS7_DIGEST_free(___a) \
	AROS_LC1(void, PKCS7_DIGEST_free, \
	AROS_LCA(PKCS7_DIGEST *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 1282, Amissl)

#define d2i_PKCS7_DIGEST(___a, ___in, ___len) \
	AROS_LC3(PKCS7_DIGEST *, d2i_PKCS7_DIGEST, \
	AROS_LCA(PKCS7_DIGEST **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 1283, Amissl)

#define i2d_PKCS7_DIGEST(___a, ___out) \
	AROS_LC2(int, i2d_PKCS7_DIGEST, \
	AROS_LCA(PKCS7_DIGEST *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 1284, Amissl)

#define PKCS7_DIGEST_it() \
	AROS_LC0(const ASN1_ITEM *, PKCS7_DIGEST_it, \
	struct Library *, AMISSL_BASE_NAME, 1285, Amissl)

#define PKCS7_ENCRYPT_new() \
	AROS_LC0(PKCS7_ENCRYPT *, PKCS7_ENCRYPT_new, \
	struct Library *, AMISSL_BASE_NAME, 1286, Amissl)

#define PKCS7_ENCRYPT_free(___a) \
	AROS_LC1(void, PKCS7_ENCRYPT_free, \
	AROS_LCA(PKCS7_ENCRYPT *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 1287, Amissl)

#define d2i_PKCS7_ENCRYPT(___a, ___in, ___len) \
	AROS_LC3(PKCS7_ENCRYPT *, d2i_PKCS7_ENCRYPT, \
	AROS_LCA(PKCS7_ENCRYPT **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 1288, Amissl)

#define i2d_PKCS7_ENCRYPT(___a, ___out) \
	AROS_LC2(int, i2d_PKCS7_ENCRYPT, \
	AROS_LCA(PKCS7_ENCRYPT *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 1289, Amissl)

#define PKCS7_ENCRYPT_it() \
	AROS_LC0(const ASN1_ITEM *, PKCS7_ENCRYPT_it, \
	struct Library *, AMISSL_BASE_NAME, 1290, Amissl)

#define PKCS7_new() \
	AROS_LC0(PKCS7 *, PKCS7_new, \
	struct Library *, AMISSL_BASE_NAME, 1291, Amissl)

#define PKCS7_free(___a) \
	AROS_LC1(void, PKCS7_free, \
	AROS_LCA(PKCS7 *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 1292, Amissl)

#define d2i_PKCS7(___a, ___in, ___len) \
	AROS_LC3(PKCS7 *, d2i_PKCS7, \
	AROS_LCA(PKCS7 **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 1293, Amissl)

#define i2d_PKCS7(___a, ___out) \
	AROS_LC2(int, i2d_PKCS7, \
	AROS_LCA(PKCS7 *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 1294, Amissl)

#define PKCS7_it() \
	AROS_LC0(const ASN1_ITEM *, PKCS7_it, \
	struct Library *, AMISSL_BASE_NAME, 1295, Amissl)

#define PKCS7_ATTR_SIGN_it() \
	AROS_LC0(const ASN1_ITEM *, PKCS7_ATTR_SIGN_it, \
	struct Library *, AMISSL_BASE_NAME, 1296, Amissl)

#define PKCS7_ATTR_VERIFY_it() \
	AROS_LC0(const ASN1_ITEM *, PKCS7_ATTR_VERIFY_it, \
	struct Library *, AMISSL_BASE_NAME, 1297, Amissl)

#define PKCS7_ctrl(___p7, ___cmd, ___larg, ___parg) \
	AROS_LC4(long, PKCS7_ctrl, \
	AROS_LCA(PKCS7 *, (___p7), A0), \
	AROS_LCA(int, (___cmd), D0), \
	AROS_LCA(long, (___larg), D1), \
	AROS_LCA(char *, (___parg), A1), \
	struct Library *, AMISSL_BASE_NAME, 1298, Amissl)

#define PKCS7_set_type(___p7, ___type) \
	AROS_LC2(int, PKCS7_set_type, \
	AROS_LCA(PKCS7 *, (___p7), A0), \
	AROS_LCA(int, (___type), D0), \
	struct Library *, AMISSL_BASE_NAME, 1299, Amissl)

#define PKCS7_set_content(___p7, ___p7_data) \
	AROS_LC2(int, PKCS7_set_content, \
	AROS_LCA(PKCS7 *, (___p7), A0), \
	AROS_LCA(PKCS7 *, (___p7_data), A1), \
	struct Library *, AMISSL_BASE_NAME, 1300, Amissl)

#define PKCS7_SIGNER_INFO_set(___p7i, ___x509, ___pkey, ___dgst) \
	AROS_LC4(int, PKCS7_SIGNER_INFO_set, \
	AROS_LCA(PKCS7_SIGNER_INFO *, (___p7i), A0), \
	AROS_LCA(X509 *, (___x509), A1), \
	AROS_LCA(EVP_PKEY *, (___pkey), A2), \
	AROS_LCA(const EVP_MD *, (___dgst), A3), \
	struct Library *, AMISSL_BASE_NAME, 1301, Amissl)

#define PKCS7_add_signer(___p7, ___p7i) \
	AROS_LC2(int, PKCS7_add_signer, \
	AROS_LCA(PKCS7 *, (___p7), A0), \
	AROS_LCA(PKCS7_SIGNER_INFO *, (___p7i), A1), \
	struct Library *, AMISSL_BASE_NAME, 1302, Amissl)

#define PKCS7_add_certificate(___p7, ___x509) \
	AROS_LC2(int, PKCS7_add_certificate, \
	AROS_LCA(PKCS7 *, (___p7), A0), \
	AROS_LCA(X509 *, (___x509), A1), \
	struct Library *, AMISSL_BASE_NAME, 1303, Amissl)

#define PKCS7_add_crl(___p7, ___x509) \
	AROS_LC2(int, PKCS7_add_crl, \
	AROS_LCA(PKCS7 *, (___p7), A0), \
	AROS_LCA(X509_CRL *, (___x509), A1), \
	struct Library *, AMISSL_BASE_NAME, 1304, Amissl)

#define PKCS7_content_new(___p7, ___nid) \
	AROS_LC2(int, PKCS7_content_new, \
	AROS_LCA(PKCS7 *, (___p7), A0), \
	AROS_LCA(int, (___nid), D0), \
	struct Library *, AMISSL_BASE_NAME, 1305, Amissl)

#define PKCS7_dataVerify(___cert_store, ___ctx, ___bio, ___p7, ___si) \
	AROS_LC5(int, PKCS7_dataVerify, \
	AROS_LCA(X509_STORE *, (___cert_store), A0), \
	AROS_LCA(X509_STORE_CTX *, (___ctx), A1), \
	AROS_LCA(BIO *, (___bio), A2), \
	AROS_LCA(PKCS7 *, (___p7), A3), \
	AROS_LCA(PKCS7_SIGNER_INFO *, (___si), D0), \
	struct Library *, AMISSL_BASE_NAME, 1306, Amissl)

#define PKCS7_signatureVerify(___bio, ___p7, ___si, ___x509) \
	AROS_LC4(int, PKCS7_signatureVerify, \
	AROS_LCA(BIO *, (___bio), A0), \
	AROS_LCA(PKCS7 *, (___p7), A1), \
	AROS_LCA(PKCS7_SIGNER_INFO *, (___si), A2), \
	AROS_LCA(X509 *, (___x509), A3), \
	struct Library *, AMISSL_BASE_NAME, 1307, Amissl)

#define PKCS7_dataInit(___p7, ___bio) \
	AROS_LC2(BIO *, PKCS7_dataInit, \
	AROS_LCA(PKCS7 *, (___p7), A0), \
	AROS_LCA(BIO *, (___bio), A1), \
	struct Library *, AMISSL_BASE_NAME, 1308, Amissl)

#define PKCS7_dataFinal(___p7, ___bio) \
	AROS_LC2(int, PKCS7_dataFinal, \
	AROS_LCA(PKCS7 *, (___p7), A0), \
	AROS_LCA(BIO *, (___bio), A1), \
	struct Library *, AMISSL_BASE_NAME, 1309, Amissl)

#define PKCS7_dataDecode(___p7, ___pkey, ___in_bio, ___pcert) \
	AROS_LC4(BIO *, PKCS7_dataDecode, \
	AROS_LCA(PKCS7 *, (___p7), A0), \
	AROS_LCA(EVP_PKEY *, (___pkey), A1), \
	AROS_LCA(BIO *, (___in_bio), A2), \
	AROS_LCA(X509 *, (___pcert), A3), \
	struct Library *, AMISSL_BASE_NAME, 1310, Amissl)

#define PKCS7_add_signature(___p7, ___x509, ___pkey, ___dgst) \
	AROS_LC4(PKCS7_SIGNER_INFO *, PKCS7_add_signature, \
	AROS_LCA(PKCS7 *, (___p7), A0), \
	AROS_LCA(X509 *, (___x509), A1), \
	AROS_LCA(EVP_PKEY *, (___pkey), A2), \
	AROS_LCA(const EVP_MD *, (___dgst), A3), \
	struct Library *, AMISSL_BASE_NAME, 1311, Amissl)

#define PKCS7_cert_from_signer_info(___p7, ___si) \
	AROS_LC2(X509 *, PKCS7_cert_from_signer_info, \
	AROS_LCA(PKCS7 *, (___p7), A0), \
	AROS_LCA(PKCS7_SIGNER_INFO *, (___si), A1), \
	struct Library *, AMISSL_BASE_NAME, 1312, Amissl)

#define PKCS7_get_signer_info(___p7) \
	AROS_LC1(STACK_OF(PKCS7_SIGNER_INFO) *, PKCS7_get_signer_info, \
	AROS_LCA(PKCS7 *, (___p7), A0), \
	struct Library *, AMISSL_BASE_NAME, 1313, Amissl)

#define PKCS7_add_recipient(___p7, ___x509) \
	AROS_LC2(PKCS7_RECIP_INFO *, PKCS7_add_recipient, \
	AROS_LCA(PKCS7 *, (___p7), A0), \
	AROS_LCA(X509 *, (___x509), A1), \
	struct Library *, AMISSL_BASE_NAME, 1314, Amissl)

#define PKCS7_add_recipient_info(___p7, ___ri) \
	AROS_LC2(int, PKCS7_add_recipient_info, \
	AROS_LCA(PKCS7 *, (___p7), A0), \
	AROS_LCA(PKCS7_RECIP_INFO *, (___ri), A1), \
	struct Library *, AMISSL_BASE_NAME, 1315, Amissl)

#define PKCS7_RECIP_INFO_set(___p7i, ___x509) \
	AROS_LC2(int, PKCS7_RECIP_INFO_set, \
	AROS_LCA(PKCS7_RECIP_INFO *, (___p7i), A0), \
	AROS_LCA(X509 *, (___x509), A1), \
	struct Library *, AMISSL_BASE_NAME, 1316, Amissl)

#define PKCS7_set_cipher(___p7, ___cipher) \
	AROS_LC2(int, PKCS7_set_cipher, \
	AROS_LCA(PKCS7 *, (___p7), A0), \
	AROS_LCA(const EVP_CIPHER *, (___cipher), A1), \
	struct Library *, AMISSL_BASE_NAME, 1317, Amissl)

#define PKCS7_get_issuer_and_serial(___p7, ___idx) \
	AROS_LC2(PKCS7_ISSUER_AND_SERIAL *, PKCS7_get_issuer_and_serial, \
	AROS_LCA(PKCS7 *, (___p7), A0), \
	AROS_LCA(int, (___idx), D0), \
	struct Library *, AMISSL_BASE_NAME, 1318, Amissl)

#define PKCS7_digest_from_attributes(___sk) \
	AROS_LC1(ASN1_OCTET_STRING *, PKCS7_digest_from_attributes, \
	AROS_LCA(STACK_OF(X509_ATTRIBUTE) *, (___sk), A0), \
	struct Library *, AMISSL_BASE_NAME, 1319, Amissl)

#define PKCS7_add_signed_attribute(___p7si, ___nid, ___type, ___data) \
	AROS_LC4(int, PKCS7_add_signed_attribute, \
	AROS_LCA(PKCS7_SIGNER_INFO *, (___p7si), A0), \
	AROS_LCA(int, (___nid), D0), \
	AROS_LCA(int, (___type), D1), \
	AROS_LCA(void *, (___data), A1), \
	struct Library *, AMISSL_BASE_NAME, 1320, Amissl)

#define PKCS7_add_attribute(___p7si, ___nid, ___atrtype, ___value) \
	AROS_LC4(int, PKCS7_add_attribute, \
	AROS_LCA(PKCS7_SIGNER_INFO *, (___p7si), A0), \
	AROS_LCA(int, (___nid), D0), \
	AROS_LCA(int, (___atrtype), D1), \
	AROS_LCA(void *, (___value), A1), \
	struct Library *, AMISSL_BASE_NAME, 1321, Amissl)

#define PKCS7_get_attribute(___si, ___nid) \
	AROS_LC2(ASN1_TYPE *, PKCS7_get_attribute, \
	AROS_LCA(PKCS7_SIGNER_INFO *, (___si), A0), \
	AROS_LCA(int, (___nid), D0), \
	struct Library *, AMISSL_BASE_NAME, 1322, Amissl)

#define PKCS7_get_signed_attribute(___si, ___nid) \
	AROS_LC2(ASN1_TYPE *, PKCS7_get_signed_attribute, \
	AROS_LCA(PKCS7_SIGNER_INFO *, (___si), A0), \
	AROS_LCA(int, (___nid), D0), \
	struct Library *, AMISSL_BASE_NAME, 1323, Amissl)

#define PKCS7_set_signed_attributes(___p7si, ___sk) \
	AROS_LC2(int, PKCS7_set_signed_attributes, \
	AROS_LCA(PKCS7_SIGNER_INFO *, (___p7si), A0), \
	AROS_LCA(STACK_OF(X509_ATTRIBUTE) *, (___sk), A1), \
	struct Library *, AMISSL_BASE_NAME, 1324, Amissl)

#define PKCS7_set_attributes(___p7si, ___sk) \
	AROS_LC2(int, PKCS7_set_attributes, \
	AROS_LCA(PKCS7_SIGNER_INFO *, (___p7si), A0), \
	AROS_LCA(STACK_OF(X509_ATTRIBUTE) *, (___sk), A1), \
	struct Library *, AMISSL_BASE_NAME, 1325, Amissl)

#define PKCS7_sign(___signcert, ___pkey, ___certs, ___data, ___flags) \
	AROS_LC5(PKCS7 *, PKCS7_sign, \
	AROS_LCA(X509 *, (___signcert), A0), \
	AROS_LCA(EVP_PKEY *, (___pkey), A1), \
	AROS_LCA(STACK_OF(X509) *, (___certs), A2), \
	AROS_LCA(BIO *, (___data), A3), \
	AROS_LCA(int, (___flags), D0), \
	struct Library *, AMISSL_BASE_NAME, 1326, Amissl)

#define PKCS7_verify(___p7, ___certs, ___store, ___indata, ___out, ___flags) \
	AROS_LC6(int, PKCS7_verify, \
	AROS_LCA(PKCS7 *, (___p7), A0), \
	AROS_LCA(STACK_OF(X509) *, (___certs), A1), \
	AROS_LCA(X509_STORE *, (___store), A2), \
	AROS_LCA(BIO *, (___indata), A3), \
	AROS_LCA(BIO *, (___out), D0), \
	AROS_LCA(int, (___flags), D1), \
	struct Library *, AMISSL_BASE_NAME, 1327, Amissl)

#define PKCS7_get0_signers(___p7, ___certs, ___flags) \
	AROS_LC3(STACK_OF(X509) *, PKCS7_get0_signers, \
	AROS_LCA(PKCS7 *, (___p7), A0), \
	AROS_LCA(STACK_OF(X509) *, (___certs), A1), \
	AROS_LCA(int, (___flags), D0), \
	struct Library *, AMISSL_BASE_NAME, 1328, Amissl)

#define PKCS7_encrypt(___certs, ___in, ___cipher, ___flags) \
	AROS_LC4(PKCS7 *, PKCS7_encrypt, \
	AROS_LCA(STACK_OF(X509) *, (___certs), A0), \
	AROS_LCA(BIO *, (___in), A1), \
	AROS_LCA(const EVP_CIPHER *, (___cipher), A2), \
	AROS_LCA(int, (___flags), D0), \
	struct Library *, AMISSL_BASE_NAME, 1329, Amissl)

#define PKCS7_decrypt(___p7, ___pkey, ___cert, ___data, ___flags) \
	AROS_LC5(int, PKCS7_decrypt, \
	AROS_LCA(PKCS7 *, (___p7), A0), \
	AROS_LCA(EVP_PKEY *, (___pkey), A1), \
	AROS_LCA(X509 *, (___cert), A2), \
	AROS_LCA(BIO *, (___data), A3), \
	AROS_LCA(int, (___flags), D0), \
	struct Library *, AMISSL_BASE_NAME, 1330, Amissl)

#define PKCS7_add_attrib_smimecap(___si, ___cap) \
	AROS_LC2(int, PKCS7_add_attrib_smimecap, \
	AROS_LCA(PKCS7_SIGNER_INFO *, (___si), A0), \
	AROS_LCA(STACK_OF(X509_ALGOR) *, (___cap), A1), \
	struct Library *, AMISSL_BASE_NAME, 1331, Amissl)

#define PKCS7_get_smimecap(___si) \
	AROS_LC1(STACK_OF(X509_ALGOR) *, PKCS7_get_smimecap, \
	AROS_LCA(PKCS7_SIGNER_INFO *, (___si), A0), \
	struct Library *, AMISSL_BASE_NAME, 1332, Amissl)

#define PKCS7_simple_smimecap(___sk, ___nid, ___arg) \
	AROS_LC3(int, PKCS7_simple_smimecap, \
	AROS_LCA(STACK_OF(X509_ALGOR) *, (___sk), A0), \
	AROS_LCA(int, (___nid), D0), \
	AROS_LCA(int, (___arg), D1), \
	struct Library *, AMISSL_BASE_NAME, 1333, Amissl)

#define SMIME_write_PKCS7(___bio, ___p7, ___data, ___flags) \
	AROS_LC4(int, SMIME_write_PKCS7, \
	AROS_LCA(BIO *, (___bio), A0), \
	AROS_LCA(PKCS7 *, (___p7), A1), \
	AROS_LCA(BIO *, (___data), A2), \
	AROS_LCA(int, (___flags), D0), \
	struct Library *, AMISSL_BASE_NAME, 1334, Amissl)

#define SMIME_read_PKCS7(___bio, ___bcont) \
	AROS_LC2(PKCS7 *, SMIME_read_PKCS7, \
	AROS_LCA(BIO *, (___bio), A0), \
	AROS_LCA(BIO **, (___bcont), A1), \
	struct Library *, AMISSL_BASE_NAME, 1335, Amissl)

#define SMIME_crlf_copy(___in, ___out, ___flags) \
	AROS_LC3(int, SMIME_crlf_copy, \
	AROS_LCA(BIO *, (___in), A0), \
	AROS_LCA(BIO *, (___out), A1), \
	AROS_LCA(int, (___flags), D0), \
	struct Library *, AMISSL_BASE_NAME, 1336, Amissl)

#define SMIME_text(___in, ___out) \
	AROS_LC2(int, SMIME_text, \
	AROS_LCA(BIO *, (___in), A0), \
	AROS_LCA(BIO *, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 1337, Amissl)

#define ERR_load_PKCS7_strings() \
	AROS_LC0(void, ERR_load_PKCS7_strings, \
	struct Library *, AMISSL_BASE_NAME, 1338, Amissl)

#define RAND_set_rand_method(___meth) \
	AROS_LC1(int, RAND_set_rand_method, \
	AROS_LCA(const RAND_METHOD *, (___meth), A0), \
	struct Library *, AMISSL_BASE_NAME, 1339, Amissl)

#define RAND_get_rand_method() \
	AROS_LC0(const RAND_METHOD *, RAND_get_rand_method, \
	struct Library *, AMISSL_BASE_NAME, 1340, Amissl)

#define RAND_SSLeay() \
	AROS_LC0(RAND_METHOD *, RAND_SSLeay, \
	struct Library *, AMISSL_BASE_NAME, 1341, Amissl)

#define RAND_cleanup() \
	AROS_LC0(void, RAND_cleanup, \
	struct Library *, AMISSL_BASE_NAME, 1342, Amissl)

#define RAND_bytes(___buf, ___num) \
	AROS_LC2(int, RAND_bytes, \
	AROS_LCA(unsigned char *, (___buf), A0), \
	AROS_LCA(int, (___num), D0), \
	struct Library *, AMISSL_BASE_NAME, 1343, Amissl)

#define RAND_pseudo_bytes(___buf, ___num) \
	AROS_LC2(int, RAND_pseudo_bytes, \
	AROS_LCA(unsigned char *, (___buf), A0), \
	AROS_LCA(int, (___num), D0), \
	struct Library *, AMISSL_BASE_NAME, 1344, Amissl)

#define RAND_seed(___buf, ___num) \
	AROS_LC2(void, RAND_seed, \
	AROS_LCA(const void *, (___buf), A0), \
	AROS_LCA(int, (___num), D0), \
	struct Library *, AMISSL_BASE_NAME, 1345, Amissl)

#define RAND_add(___buf, ___num, ___entropy) \
	AROS_LC3(void, RAND_add, \
	AROS_LCA(const void *, (___buf), A0), \
	AROS_LCA(int, (___num), D0), \
	AROS_LCA(float, (___entropy), D1), \
	struct Library *, AMISSL_BASE_NAME, 1346, Amissl)

#define RAND_load_file(___file, ___max_bytes) \
	AROS_LC2(int, RAND_load_file, \
	AROS_LCA(const char *, (___file), A0), \
	AROS_LCA(long, (___max_bytes), D0), \
	struct Library *, AMISSL_BASE_NAME, 1347, Amissl)

#define RAND_write_file(___file) \
	AROS_LC1(int, RAND_write_file, \
	AROS_LCA(const char *, (___file), A0), \
	struct Library *, AMISSL_BASE_NAME, 1348, Amissl)

#define RAND_file_name(___file, ___num) \
	AROS_LC2(const char *, RAND_file_name, \
	AROS_LCA(char *, (___file), A0), \
	AROS_LCA(size_t, (___num), D0), \
	struct Library *, AMISSL_BASE_NAME, 1349, Amissl)

#define RAND_status() \
	AROS_LC0(int, RAND_status, \
	struct Library *, AMISSL_BASE_NAME, 1350, Amissl)

#define RAND_query_egd_bytes(___path, ___buf, ___bytes) \
	AROS_LC3(int, RAND_query_egd_bytes, \
	AROS_LCA(const char *, (___path), A0), \
	AROS_LCA(unsigned char *, (___buf), A1), \
	AROS_LCA(int, (___bytes), D0), \
	struct Library *, AMISSL_BASE_NAME, 1351, Amissl)

#define RAND_egd(___path) \
	AROS_LC1(int, RAND_egd, \
	AROS_LCA(const char *, (___path), A0), \
	struct Library *, AMISSL_BASE_NAME, 1352, Amissl)

#define RAND_egd_bytes(___path, ___bytes) \
	AROS_LC2(int, RAND_egd_bytes, \
	AROS_LCA(const char *, (___path), A0), \
	AROS_LCA(int, (___bytes), D0), \
	struct Library *, AMISSL_BASE_NAME, 1353, Amissl)

#define RAND_poll() \
	AROS_LC0(int, RAND_poll, \
	struct Library *, AMISSL_BASE_NAME, 1354, Amissl)

#define ERR_load_RAND_strings() \
	AROS_LC0(void, ERR_load_RAND_strings, \
	struct Library *, AMISSL_BASE_NAME, 1355, Amissl)

#define SSL_CTX_set_msg_callback(___ctx, ___cb) \
	AROS_LC2(void, SSL_CTX_set_msg_callback, \
	AROS_LCA(SSL_CTX *, (___ctx), A0), \
	AROS_LCA(void (*)(int write_p,int version,int content_type,const void *buf,size_t len,SSL *ssl,void *arg), (___cb), A1), \
	struct Library *, AMISSL_BASE_NAME, 1356, Amissl)

#define SSL_set_msg_callback(___ssl, ___cb) \
	AROS_LC2(void, SSL_set_msg_callback, \
	AROS_LCA(SSL *, (___ssl), A0), \
	AROS_LCA(void (*)(int write_p,int version,int content_type,const void *buf,size_t len,SSL *ssl,void *arg), (___cb), A1), \
	struct Library *, AMISSL_BASE_NAME, 1357, Amissl)

#define SSL_CTX_sessions(___ctx) \
	AROS_LC1(struct lhash_st *, SSL_CTX_sessions, \
	AROS_LCA(SSL_CTX *, (___ctx), A0), \
	struct Library *, AMISSL_BASE_NAME, 1358, Amissl)

#define SSL_get_finished(___s, ___buf, ___count) \
	AROS_LC3(size_t, SSL_get_finished, \
	AROS_LCA(const SSL *, (___s), A0), \
	AROS_LCA(void *, (___buf), A1), \
	AROS_LCA(size_t, (___count), D0), \
	struct Library *, AMISSL_BASE_NAME, 1359, Amissl)

#define SSL_get_peer_finished(___s, ___buf, ___count) \
	AROS_LC3(size_t, SSL_get_peer_finished, \
	AROS_LCA(const SSL *, (___s), A0), \
	AROS_LCA(void *, (___buf), A1), \
	AROS_LCA(size_t, (___count), D0), \
	struct Library *, AMISSL_BASE_NAME, 1360, Amissl)

#define BIO_f_ssl() \
	AROS_LC0(BIO_METHOD *, BIO_f_ssl, \
	struct Library *, AMISSL_BASE_NAME, 1361, Amissl)

#define BIO_new_ssl(___ctx, ___client) \
	AROS_LC2(BIO *, BIO_new_ssl, \
	AROS_LCA(SSL_CTX *, (___ctx), A0), \
	AROS_LCA(int, (___client), D0), \
	struct Library *, AMISSL_BASE_NAME, 1362, Amissl)

#define BIO_new_ssl_connect(___ctx) \
	AROS_LC1(BIO *, BIO_new_ssl_connect, \
	AROS_LCA(SSL_CTX *, (___ctx), A0), \
	struct Library *, AMISSL_BASE_NAME, 1363, Amissl)

#define BIO_new_buffer_ssl_connect(___ctx) \
	AROS_LC1(BIO *, BIO_new_buffer_ssl_connect, \
	AROS_LCA(SSL_CTX *, (___ctx), A0), \
	struct Library *, AMISSL_BASE_NAME, 1364, Amissl)

#define BIO_ssl_copy_session_id(___to, ___from) \
	AROS_LC2(int, BIO_ssl_copy_session_id, \
	AROS_LCA(BIO *, (___to), A0), \
	AROS_LCA(BIO *, (___from), A1), \
	struct Library *, AMISSL_BASE_NAME, 1365, Amissl)

#define BIO_ssl_shutdown(___ssl_bio) \
	AROS_LC1(void, BIO_ssl_shutdown, \
	AROS_LCA(BIO *, (___ssl_bio), A0), \
	struct Library *, AMISSL_BASE_NAME, 1366, Amissl)

#define SSL_CTX_set_cipher_list(___a1, ___str) \
	AROS_LC2(int, SSL_CTX_set_cipher_list, \
	AROS_LCA(SSL_CTX *, (___a1), A0), \
	AROS_LCA(const char *, (___str), A1), \
	struct Library *, AMISSL_BASE_NAME, 1367, Amissl)

#define SSL_CTX_new(___meth) \
	AROS_LC1(SSL_CTX *, SSL_CTX_new, \
	AROS_LCA(SSL_METHOD *, (___meth), A0), \
	struct Library *, AMISSL_BASE_NAME, 1368, Amissl)

#define SSL_CTX_free(___a) \
	AROS_LC1(void, SSL_CTX_free, \
	AROS_LCA(SSL_CTX *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 1369, Amissl)

#define SSL_CTX_set_timeout(___ctx, ___t) \
	AROS_LC2(long, SSL_CTX_set_timeout, \
	AROS_LCA(SSL_CTX *, (___ctx), A0), \
	AROS_LCA(long, (___t), D0), \
	struct Library *, AMISSL_BASE_NAME, 1370, Amissl)

#define SSL_CTX_get_timeout(___ctx) \
	AROS_LC1(long, SSL_CTX_get_timeout, \
	AROS_LCA(const SSL_CTX *, (___ctx), A0), \
	struct Library *, AMISSL_BASE_NAME, 1371, Amissl)

#define SSL_CTX_get_cert_store(___a) \
	AROS_LC1(X509_STORE *, SSL_CTX_get_cert_store, \
	AROS_LCA(const SSL_CTX *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 1372, Amissl)

#define SSL_CTX_set_cert_store(___a, ___b) \
	AROS_LC2(void, SSL_CTX_set_cert_store, \
	AROS_LCA(SSL_CTX *, (___a), A0), \
	AROS_LCA(X509_STORE *, (___b), A1), \
	struct Library *, AMISSL_BASE_NAME, 1373, Amissl)

#define SSL_want(___s) \
	AROS_LC1(int, SSL_want, \
	AROS_LCA(const SSL *, (___s), A0), \
	struct Library *, AMISSL_BASE_NAME, 1374, Amissl)

#define SSL_clear(___s) \
	AROS_LC1(int, SSL_clear, \
	AROS_LCA(SSL *, (___s), A0), \
	struct Library *, AMISSL_BASE_NAME, 1375, Amissl)

#define SSL_CTX_flush_sessions(___ctx, ___tm) \
	AROS_LC2(void, SSL_CTX_flush_sessions, \
	AROS_LCA(SSL_CTX *, (___ctx), A0), \
	AROS_LCA(long, (___tm), D0), \
	struct Library *, AMISSL_BASE_NAME, 1376, Amissl)

#define SSL_get_current_cipher(___s) \
	AROS_LC1(SSL_CIPHER *, SSL_get_current_cipher, \
	AROS_LCA(const SSL *, (___s), A0), \
	struct Library *, AMISSL_BASE_NAME, 1377, Amissl)

#define SSL_CIPHER_get_bits(___c, ___alg_bits) \
	AROS_LC2(int, SSL_CIPHER_get_bits, \
	AROS_LCA(const SSL_CIPHER *, (___c), A0), \
	AROS_LCA(int *, (___alg_bits), A1), \
	struct Library *, AMISSL_BASE_NAME, 1378, Amissl)

#define SSL_CIPHER_get_version(___c) \
	AROS_LC1(char *, SSL_CIPHER_get_version, \
	AROS_LCA(const SSL_CIPHER *, (___c), A0), \
	struct Library *, AMISSL_BASE_NAME, 1379, Amissl)

#define SSL_CIPHER_get_name(___c) \
	AROS_LC1(const char *, SSL_CIPHER_get_name, \
	AROS_LCA(const SSL_CIPHER *, (___c), A0), \
	struct Library *, AMISSL_BASE_NAME, 1380, Amissl)

#define SSL_CIPHER_get_mac(___cipher) \
	AROS_LC1(const char *, SSL_CIPHER_get_mac, \
	AROS_LCA(SSL_CIPHER *, (___cipher), A0), \
	struct Library *, AMISSL_BASE_NAME, 1381, Amissl)

#define SSL_CIPHER_get_encryption(___cipher) \
	AROS_LC1(const char *, SSL_CIPHER_get_encryption, \
	AROS_LCA(SSL_CIPHER *, (___cipher), A0), \
	struct Library *, AMISSL_BASE_NAME, 1382, Amissl)

#define SSL_CIPHER_get_authentication(___cipher) \
	AROS_LC1(const char *, SSL_CIPHER_get_authentication, \
	AROS_LCA(SSL_CIPHER *, (___cipher), A0), \
	struct Library *, AMISSL_BASE_NAME, 1383, Amissl)

#define SSL_CIPHER_get_key_exchange(___cipher) \
	AROS_LC1(const char *, SSL_CIPHER_get_key_exchange, \
	AROS_LCA(SSL_CIPHER *, (___cipher), A0), \
	struct Library *, AMISSL_BASE_NAME, 1384, Amissl)

#define SSL_CIPHER_get_export(___cipher) \
	AROS_LC1(const char *, SSL_CIPHER_get_export, \
	AROS_LCA(SSL_CIPHER *, (___cipher), A0), \
	struct Library *, AMISSL_BASE_NAME, 1385, Amissl)

#define SSL_get_fd(___s) \
	AROS_LC1(int, SSL_get_fd, \
	AROS_LCA(const SSL *, (___s), A0), \
	struct Library *, AMISSL_BASE_NAME, 1386, Amissl)

#define SSL_get_rfd(___s) \
	AROS_LC1(int, SSL_get_rfd, \
	AROS_LCA(const SSL *, (___s), A0), \
	struct Library *, AMISSL_BASE_NAME, 1387, Amissl)

#define SSL_get_wfd(___s) \
	AROS_LC1(int, SSL_get_wfd, \
	AROS_LCA(const SSL *, (___s), A0), \
	struct Library *, AMISSL_BASE_NAME, 1388, Amissl)

#define SSL_get_cipher_list(___s, ___n) \
	AROS_LC2(const char *, SSL_get_cipher_list, \
	AROS_LCA(const SSL *, (___s), A0), \
	AROS_LCA(int, (___n), D0), \
	struct Library *, AMISSL_BASE_NAME, 1389, Amissl)

#define SSL_get_shared_ciphers(___s, ___buf, ___len) \
	AROS_LC3(char *, SSL_get_shared_ciphers, \
	AROS_LCA(const SSL *, (___s), A0), \
	AROS_LCA(char *, (___buf), A1), \
	AROS_LCA(int, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 1390, Amissl)

#define SSL_get_read_ahead(___s) \
	AROS_LC1(int, SSL_get_read_ahead, \
	AROS_LCA(const SSL *, (___s), A0), \
	struct Library *, AMISSL_BASE_NAME, 1391, Amissl)

#define SSL_pending(___s) \
	AROS_LC1(int, SSL_pending, \
	AROS_LCA(const SSL *, (___s), A0), \
	struct Library *, AMISSL_BASE_NAME, 1392, Amissl)

#define SSL_set_fd(___s, ___fd) \
	AROS_LC2(int, SSL_set_fd, \
	AROS_LCA(SSL *, (___s), A0), \
	AROS_LCA(int, (___fd), D0), \
	struct Library *, AMISSL_BASE_NAME, 1393, Amissl)

#define SSL_set_rfd(___s, ___fd) \
	AROS_LC2(int, SSL_set_rfd, \
	AROS_LCA(SSL *, (___s), A0), \
	AROS_LCA(int, (___fd), D0), \
	struct Library *, AMISSL_BASE_NAME, 1394, Amissl)

#define SSL_set_wfd(___s, ___fd) \
	AROS_LC2(int, SSL_set_wfd, \
	AROS_LCA(SSL *, (___s), A0), \
	AROS_LCA(int, (___fd), D0), \
	struct Library *, AMISSL_BASE_NAME, 1395, Amissl)

#define SSL_set_bio(___s, ___rbio, ___wbio) \
	AROS_LC3(void, SSL_set_bio, \
	AROS_LCA(SSL *, (___s), A0), \
	AROS_LCA(BIO *, (___rbio), A1), \
	AROS_LCA(BIO *, (___wbio), A2), \
	struct Library *, AMISSL_BASE_NAME, 1396, Amissl)

#define SSL_get_rbio(___s) \
	AROS_LC1(BIO *, SSL_get_rbio, \
	AROS_LCA(const SSL *, (___s), A0), \
	struct Library *, AMISSL_BASE_NAME, 1397, Amissl)

#define SSL_get_wbio(___s) \
	AROS_LC1(BIO *, SSL_get_wbio, \
	AROS_LCA(const SSL *, (___s), A0), \
	struct Library *, AMISSL_BASE_NAME, 1398, Amissl)

#define SSL_set_cipher_list(___s, ___str) \
	AROS_LC2(int, SSL_set_cipher_list, \
	AROS_LCA(SSL *, (___s), A0), \
	AROS_LCA(const char *, (___str), A1), \
	struct Library *, AMISSL_BASE_NAME, 1399, Amissl)

#define SSL_set_read_ahead(___s, ___yes) \
	AROS_LC2(void, SSL_set_read_ahead, \
	AROS_LCA(SSL *, (___s), A0), \
	AROS_LCA(int, (___yes), D0), \
	struct Library *, AMISSL_BASE_NAME, 1400, Amissl)

#define SSL_get_verify_mode(___s) \
	AROS_LC1(int, SSL_get_verify_mode, \
	AROS_LCA(const SSL *, (___s), A0), \
	struct Library *, AMISSL_BASE_NAME, 1401, Amissl)

#define SSL_get_verify_depth(___s) \
	AROS_LC1(int, SSL_get_verify_depth, \
	AROS_LCA(const SSL *, (___s), A0), \
	struct Library *, AMISSL_BASE_NAME, 1402, Amissl)

#define SSL_get_verify_callback(___s) \
	AROS_LC1(int *, SSL_get_verify_callback, \
	AROS_LCA(const SSL *, (___s), A0), \
	struct Library *, AMISSL_BASE_NAME, 1403, Amissl)

#define SSL_set_verify(___s, ___mode, ___callback) \
	AROS_LC3(void, SSL_set_verify, \
	AROS_LCA(SSL *, (___s), A0), \
	AROS_LCA(int, (___mode), D0), \
	AROS_LCA(int (*)(int ok,X509_STORE_CTX *ctx), (___callback), A1), \
	struct Library *, AMISSL_BASE_NAME, 1404, Amissl)

#define SSL_set_verify_depth(___s, ___depth) \
	AROS_LC2(void, SSL_set_verify_depth, \
	AROS_LCA(SSL *, (___s), A0), \
	AROS_LCA(int, (___depth), D0), \
	struct Library *, AMISSL_BASE_NAME, 1405, Amissl)

#define SSL_use_RSAPrivateKey(___ssl, ___rsa) \
	AROS_LC2(int, SSL_use_RSAPrivateKey, \
	AROS_LCA(SSL *, (___ssl), A0), \
	AROS_LCA(RSA *, (___rsa), A1), \
	struct Library *, AMISSL_BASE_NAME, 1406, Amissl)

#define SSL_use_RSAPrivateKey_ASN1(___ssl, ___d, ___len) \
	AROS_LC3(int, SSL_use_RSAPrivateKey_ASN1, \
	AROS_LCA(SSL *, (___ssl), A0), \
	AROS_LCA(unsigned char *, (___d), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 1407, Amissl)

#define SSL_use_PrivateKey(___ssl, ___pkey) \
	AROS_LC2(int, SSL_use_PrivateKey, \
	AROS_LCA(SSL *, (___ssl), A0), \
	AROS_LCA(EVP_PKEY *, (___pkey), A1), \
	struct Library *, AMISSL_BASE_NAME, 1408, Amissl)

#define SSL_use_PrivateKey_ASN1(___pk, ___ssl, ___d, ___len) \
	AROS_LC4(int, SSL_use_PrivateKey_ASN1, \
	AROS_LCA(int, (___pk), D0), \
	AROS_LCA(SSL *, (___ssl), A0), \
	AROS_LCA(unsigned char *, (___d), A1), \
	AROS_LCA(long, (___len), D1), \
	struct Library *, AMISSL_BASE_NAME, 1409, Amissl)

#define SSL_use_certificate(___ssl, ___x) \
	AROS_LC2(int, SSL_use_certificate, \
	AROS_LCA(SSL *, (___ssl), A0), \
	AROS_LCA(X509 *, (___x), A1), \
	struct Library *, AMISSL_BASE_NAME, 1410, Amissl)

#define SSL_use_certificate_ASN1(___ssl, ___d, ___len) \
	AROS_LC3(int, SSL_use_certificate_ASN1, \
	AROS_LCA(SSL *, (___ssl), A0), \
	AROS_LCA(unsigned char *, (___d), A1), \
	AROS_LCA(int, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 1411, Amissl)

#define SSL_use_RSAPrivateKey_file(___ssl, ___file, ___type) \
	AROS_LC3(int, SSL_use_RSAPrivateKey_file, \
	AROS_LCA(SSL *, (___ssl), A0), \
	AROS_LCA(const char *, (___file), A1), \
	AROS_LCA(int, (___type), D0), \
	struct Library *, AMISSL_BASE_NAME, 1412, Amissl)

#define SSL_use_PrivateKey_file(___ssl, ___file, ___type) \
	AROS_LC3(int, SSL_use_PrivateKey_file, \
	AROS_LCA(SSL *, (___ssl), A0), \
	AROS_LCA(const char *, (___file), A1), \
	AROS_LCA(int, (___type), D0), \
	struct Library *, AMISSL_BASE_NAME, 1413, Amissl)

#define SSL_use_certificate_file(___ssl, ___file, ___type) \
	AROS_LC3(int, SSL_use_certificate_file, \
	AROS_LCA(SSL *, (___ssl), A0), \
	AROS_LCA(const char *, (___file), A1), \
	AROS_LCA(int, (___type), D0), \
	struct Library *, AMISSL_BASE_NAME, 1414, Amissl)

#define SSL_CTX_use_RSAPrivateKey_file(___ctx, ___file, ___type) \
	AROS_LC3(int, SSL_CTX_use_RSAPrivateKey_file, \
	AROS_LCA(SSL_CTX *, (___ctx), A0), \
	AROS_LCA(const char *, (___file), A1), \
	AROS_LCA(int, (___type), D0), \
	struct Library *, AMISSL_BASE_NAME, 1415, Amissl)

#define SSL_CTX_use_PrivateKey_file(___ctx, ___file, ___type) \
	AROS_LC3(int, SSL_CTX_use_PrivateKey_file, \
	AROS_LCA(SSL_CTX *, (___ctx), A0), \
	AROS_LCA(const char *, (___file), A1), \
	AROS_LCA(int, (___type), D0), \
	struct Library *, AMISSL_BASE_NAME, 1416, Amissl)

#define SSL_CTX_use_certificate_file(___ctx, ___file, ___type) \
	AROS_LC3(int, SSL_CTX_use_certificate_file, \
	AROS_LCA(SSL_CTX *, (___ctx), A0), \
	AROS_LCA(const char *, (___file), A1), \
	AROS_LCA(int, (___type), D0), \
	struct Library *, AMISSL_BASE_NAME, 1417, Amissl)

#define SSL_CTX_use_certificate_chain_file(___ctx, ___file) \
	AROS_LC2(int, SSL_CTX_use_certificate_chain_file, \
	AROS_LCA(SSL_CTX *, (___ctx), A0), \
	AROS_LCA(const char *, (___file), A1), \
	struct Library *, AMISSL_BASE_NAME, 1418, Amissl)

#define SSL_load_client_CA_file(___file) \
	AROS_LC1(STACK_OF(X509_NAME) *, SSL_load_client_CA_file, \
	AROS_LCA(const char *, (___file), A0), \
	struct Library *, AMISSL_BASE_NAME, 1419, Amissl)

#define SSL_add_file_cert_subjects_to_stack(___stackCAs, ___file) \
	AROS_LC2(int, SSL_add_file_cert_subjects_to_stack, \
	AROS_LCA(STACK_OF(X509_NAME) *, (___stackCAs), A0), \
	AROS_LCA(const char *, (___file), A1), \
	struct Library *, AMISSL_BASE_NAME, 1420, Amissl)

#define SSL_add_dir_cert_subjects_to_stack(___stackCAs, ___dir) \
	AROS_LC2(int, SSL_add_dir_cert_subjects_to_stack, \
	AROS_LCA(STACK_OF(X509_NAME) *, (___stackCAs), A0), \
	AROS_LCA(const char *, (___dir), A1), \
	struct Library *, AMISSL_BASE_NAME, 1421, Amissl)

#define SSL_load_error_strings() \
	AROS_LC0(void, SSL_load_error_strings, \
	struct Library *, AMISSL_BASE_NAME, 1422, Amissl)

#define SSL_state_string(___s) \
	AROS_LC1(const char *, SSL_state_string, \
	AROS_LCA(const SSL *, (___s), A0), \
	struct Library *, AMISSL_BASE_NAME, 1423, Amissl)

#define SSL_rstate_string(___s) \
	AROS_LC1(const char *, SSL_rstate_string, \
	AROS_LCA(const SSL *, (___s), A0), \
	struct Library *, AMISSL_BASE_NAME, 1424, Amissl)

#define SSL_state_string_long(___s) \
	AROS_LC1(const char *, SSL_state_string_long, \
	AROS_LCA(const SSL *, (___s), A0), \
	struct Library *, AMISSL_BASE_NAME, 1425, Amissl)

#define SSL_rstate_string_long(___s) \
	AROS_LC1(const char *, SSL_rstate_string_long, \
	AROS_LCA(const SSL *, (___s), A0), \
	struct Library *, AMISSL_BASE_NAME, 1426, Amissl)

#define SSL_SESSION_get_time(___s) \
	AROS_LC1(long, SSL_SESSION_get_time, \
	AROS_LCA(const SSL_SESSION *, (___s), A0), \
	struct Library *, AMISSL_BASE_NAME, 1427, Amissl)

#define SSL_SESSION_set_time(___s, ___t) \
	AROS_LC2(long, SSL_SESSION_set_time, \
	AROS_LCA(SSL_SESSION *, (___s), A0), \
	AROS_LCA(long, (___t), D0), \
	struct Library *, AMISSL_BASE_NAME, 1428, Amissl)

#define SSL_SESSION_get_timeout(___s) \
	AROS_LC1(long, SSL_SESSION_get_timeout, \
	AROS_LCA(const SSL_SESSION *, (___s), A0), \
	struct Library *, AMISSL_BASE_NAME, 1429, Amissl)

#define SSL_SESSION_set_timeout(___s, ___t) \
	AROS_LC2(long, SSL_SESSION_set_timeout, \
	AROS_LCA(SSL_SESSION *, (___s), A0), \
	AROS_LCA(long, (___t), D0), \
	struct Library *, AMISSL_BASE_NAME, 1430, Amissl)

#define SSL_copy_session_id(___to, ___from) \
	AROS_LC2(void, SSL_copy_session_id, \
	AROS_LCA(SSL *, (___to), A0), \
	AROS_LCA(const SSL *, (___from), A1), \
	struct Library *, AMISSL_BASE_NAME, 1431, Amissl)

#define SSL_SESSION_new() \
	AROS_LC0(SSL_SESSION *, SSL_SESSION_new, \
	struct Library *, AMISSL_BASE_NAME, 1432, Amissl)

#define SSL_SESSION_hash(___a) \
	AROS_LC1(unsigned long, SSL_SESSION_hash, \
	AROS_LCA(const SSL_SESSION *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 1433, Amissl)

#define SSL_SESSION_cmp(___a, ___b) \
	AROS_LC2(int, SSL_SESSION_cmp, \
	AROS_LCA(const SSL_SESSION *, (___a), A0), \
	AROS_LCA(const SSL_SESSION *, (___b), A1), \
	struct Library *, AMISSL_BASE_NAME, 1434, Amissl)

#define SSL_SESSION_print(___fp, ___ses) \
	AROS_LC2(int, SSL_SESSION_print, \
	AROS_LCA(BIO *, (___fp), A0), \
	AROS_LCA(const SSL_SESSION *, (___ses), A1), \
	struct Library *, AMISSL_BASE_NAME, 1435, Amissl)

#define SSL_SESSION_free(___ses) \
	AROS_LC1(void, SSL_SESSION_free, \
	AROS_LCA(SSL_SESSION *, (___ses), A0), \
	struct Library *, AMISSL_BASE_NAME, 1436, Amissl)

#define i2d_SSL_SESSION(___in, ___pp) \
	AROS_LC2(int, i2d_SSL_SESSION, \
	AROS_LCA(SSL_SESSION *, (___in), A0), \
	AROS_LCA(unsigned char **, (___pp), A1), \
	struct Library *, AMISSL_BASE_NAME, 1437, Amissl)

#define SSL_set_session(___to, ___session) \
	AROS_LC2(int, SSL_set_session, \
	AROS_LCA(SSL *, (___to), A0), \
	AROS_LCA(SSL_SESSION *, (___session), A1), \
	struct Library *, AMISSL_BASE_NAME, 1438, Amissl)

#define SSL_CTX_add_session(___s, ___c) \
	AROS_LC2(int, SSL_CTX_add_session, \
	AROS_LCA(SSL_CTX *, (___s), A0), \
	AROS_LCA(SSL_SESSION *, (___c), A1), \
	struct Library *, AMISSL_BASE_NAME, 1439, Amissl)

#define SSL_CTX_remove_session(___a1, ___c) \
	AROS_LC2(int, SSL_CTX_remove_session, \
	AROS_LCA(SSL_CTX *, (___a1), A0), \
	AROS_LCA(SSL_SESSION *, (___c), A1), \
	struct Library *, AMISSL_BASE_NAME, 1440, Amissl)

#define SSL_CTX_set_generate_session_id(___a, ___b) \
	AROS_LC2(int, SSL_CTX_set_generate_session_id, \
	AROS_LCA(SSL_CTX *, (___a), A0), \
	AROS_LCA(GEN_SESSION_CB, (___b), D0), \
	struct Library *, AMISSL_BASE_NAME, 1441, Amissl)

#define SSL_set_generate_session_id(___a, ___b) \
	AROS_LC2(int, SSL_set_generate_session_id, \
	AROS_LCA(SSL *, (___a), A0), \
	AROS_LCA(GEN_SESSION_CB, (___b), D0), \
	struct Library *, AMISSL_BASE_NAME, 1442, Amissl)

#define SSL_has_matching_session_id(___ssl, ___id, ___id_len) \
	AROS_LC3(int, SSL_has_matching_session_id, \
	AROS_LCA(const SSL *, (___ssl), A0), \
	AROS_LCA(const unsigned char *, (___id), A1), \
	AROS_LCA(unsigned int, (___id_len), D0), \
	struct Library *, AMISSL_BASE_NAME, 1443, Amissl)

#define d2i_SSL_SESSION(___a, ___pp, ___length) \
	AROS_LC3(SSL_SESSION *, d2i_SSL_SESSION, \
	AROS_LCA(SSL_SESSION **, (___a), A0), \
	AROS_LCA(const unsigned char *const *, (___pp), A1), \
	AROS_LCA(long, (___length), D0), \
	struct Library *, AMISSL_BASE_NAME, 1444, Amissl)

#define SSL_get_peer_certificate(___s) \
	AROS_LC1(X509 *, SSL_get_peer_certificate, \
	AROS_LCA(const SSL *, (___s), A0), \
	struct Library *, AMISSL_BASE_NAME, 1445, Amissl)

#define SSL_get_peer_cert_chain(___s) \
	AROS_LC1(STACK_OF(X509) *, SSL_get_peer_cert_chain, \
	AROS_LCA(const SSL *, (___s), A0), \
	struct Library *, AMISSL_BASE_NAME, 1446, Amissl)

#define SSL_CTX_get_verify_mode(___ctx) \
	AROS_LC1(int, SSL_CTX_get_verify_mode, \
	AROS_LCA(const SSL_CTX *, (___ctx), A0), \
	struct Library *, AMISSL_BASE_NAME, 1447, Amissl)

#define SSL_CTX_get_verify_depth(___ctx) \
	AROS_LC1(int, SSL_CTX_get_verify_depth, \
	AROS_LCA(const SSL_CTX *, (___ctx), A0), \
	struct Library *, AMISSL_BASE_NAME, 1448, Amissl)

#define SSL_CTX_get_verify_callback(___ctx) \
	AROS_LC1(int *, SSL_CTX_get_verify_callback, \
	AROS_LCA(const SSL_CTX *, (___ctx), A0), \
	struct Library *, AMISSL_BASE_NAME, 1449, Amissl)

#define SSL_CTX_set_verify(___ctx, ___mode, ___callback) \
	AROS_LC3(void, SSL_CTX_set_verify, \
	AROS_LCA(SSL_CTX *, (___ctx), A0), \
	AROS_LCA(int, (___mode), D0), \
	AROS_LCA(int (*)(int,X509_STORE_CTX *), (___callback), A1), \
	struct Library *, AMISSL_BASE_NAME, 1450, Amissl)

#define SSL_CTX_set_verify_depth(___ctx, ___depth) \
	AROS_LC2(void, SSL_CTX_set_verify_depth, \
	AROS_LCA(SSL_CTX *, (___ctx), A0), \
	AROS_LCA(int, (___depth), D0), \
	struct Library *, AMISSL_BASE_NAME, 1451, Amissl)

#define SSL_CTX_set_cert_verify_callback(___ctx, ___cb, ___arg) \
	AROS_LC3(void, SSL_CTX_set_cert_verify_callback, \
	AROS_LCA(SSL_CTX *, (___ctx), A0), \
	AROS_LCA(int (*)(X509_STORE_CTX *,void *), (___cb), A1), \
	AROS_LCA(void *, (___arg), A2), \
	struct Library *, AMISSL_BASE_NAME, 1452, Amissl)

#define SSL_CTX_use_RSAPrivateKey(___ctx, ___rsa) \
	AROS_LC2(int, SSL_CTX_use_RSAPrivateKey, \
	AROS_LCA(SSL_CTX *, (___ctx), A0), \
	AROS_LCA(RSA *, (___rsa), A1), \
	struct Library *, AMISSL_BASE_NAME, 1453, Amissl)

#define SSL_CTX_use_RSAPrivateKey_ASN1(___ctx, ___d, ___len) \
	AROS_LC3(int, SSL_CTX_use_RSAPrivateKey_ASN1, \
	AROS_LCA(SSL_CTX *, (___ctx), A0), \
	AROS_LCA(unsigned char *, (___d), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 1454, Amissl)

#define SSL_CTX_use_PrivateKey(___ctx, ___pkey) \
	AROS_LC2(int, SSL_CTX_use_PrivateKey, \
	AROS_LCA(SSL_CTX *, (___ctx), A0), \
	AROS_LCA(EVP_PKEY *, (___pkey), A1), \
	struct Library *, AMISSL_BASE_NAME, 1455, Amissl)

#define SSL_CTX_use_PrivateKey_ASN1(___pk, ___ctx, ___d, ___len) \
	AROS_LC4(int, SSL_CTX_use_PrivateKey_ASN1, \
	AROS_LCA(int, (___pk), D0), \
	AROS_LCA(SSL_CTX *, (___ctx), A0), \
	AROS_LCA(unsigned char *, (___d), A1), \
	AROS_LCA(long, (___len), D1), \
	struct Library *, AMISSL_BASE_NAME, 1456, Amissl)

#define SSL_CTX_use_certificate(___ctx, ___x) \
	AROS_LC2(int, SSL_CTX_use_certificate, \
	AROS_LCA(SSL_CTX *, (___ctx), A0), \
	AROS_LCA(X509 *, (___x), A1), \
	struct Library *, AMISSL_BASE_NAME, 1457, Amissl)

#define SSL_CTX_use_certificate_ASN1(___ctx, ___len, ___d) \
	AROS_LC3(int, SSL_CTX_use_certificate_ASN1, \
	AROS_LCA(SSL_CTX *, (___ctx), A0), \
	AROS_LCA(int, (___len), D0), \
	AROS_LCA(unsigned char *, (___d), A1), \
	struct Library *, AMISSL_BASE_NAME, 1458, Amissl)

#define SSL_CTX_set_default_passwd_cb(___ctx, ___cb) \
	AROS_LC2(void, SSL_CTX_set_default_passwd_cb, \
	AROS_LCA(SSL_CTX *, (___ctx), A0), \
	AROS_LCA(pem_password_cb * (*)(char *buf,int size,int rwflag,void *userdata), (___cb), A1), \
	struct Library *, AMISSL_BASE_NAME, 1459, Amissl)

#define SSL_CTX_set_default_passwd_cb_userdata(___ctx, ___u) \
	AROS_LC2(void, SSL_CTX_set_default_passwd_cb_userdata, \
	AROS_LCA(SSL_CTX *, (___ctx), A0), \
	AROS_LCA(void *, (___u), A1), \
	struct Library *, AMISSL_BASE_NAME, 1460, Amissl)

#define SSL_CTX_check_private_key(___ctx) \
	AROS_LC1(int, SSL_CTX_check_private_key, \
	AROS_LCA(const SSL_CTX *, (___ctx), A0), \
	struct Library *, AMISSL_BASE_NAME, 1461, Amissl)

#define SSL_check_private_key(___ctx) \
	AROS_LC1(int, SSL_check_private_key, \
	AROS_LCA(const SSL *, (___ctx), A0), \
	struct Library *, AMISSL_BASE_NAME, 1462, Amissl)

#define SSL_CTX_set_session_id_context(___ctx, ___sid_ctx, ___sid_ctx_len) \
	AROS_LC3(int, SSL_CTX_set_session_id_context, \
	AROS_LCA(SSL_CTX *, (___ctx), A0), \
	AROS_LCA(const unsigned char *, (___sid_ctx), A1), \
	AROS_LCA(unsigned int, (___sid_ctx_len), D0), \
	struct Library *, AMISSL_BASE_NAME, 1463, Amissl)

#define SSL_new(___ctx) \
	AROS_LC1(SSL *, SSL_new, \
	AROS_LCA(SSL_CTX *, (___ctx), A0), \
	struct Library *, AMISSL_BASE_NAME, 1464, Amissl)

#define SSL_set_session_id_context(___ssl, ___sid_ctx, ___sid_ctx_len) \
	AROS_LC3(int, SSL_set_session_id_context, \
	AROS_LCA(SSL *, (___ssl), A0), \
	AROS_LCA(const unsigned char *, (___sid_ctx), A1), \
	AROS_LCA(unsigned int, (___sid_ctx_len), D0), \
	struct Library *, AMISSL_BASE_NAME, 1465, Amissl)

#define SSL_CTX_set_purpose(___s, ___purpose) \
	AROS_LC2(int, SSL_CTX_set_purpose, \
	AROS_LCA(SSL_CTX *, (___s), A0), \
	AROS_LCA(int, (___purpose), D0), \
	struct Library *, AMISSL_BASE_NAME, 1466, Amissl)

#define SSL_set_purpose(___s, ___purpose) \
	AROS_LC2(int, SSL_set_purpose, \
	AROS_LCA(SSL *, (___s), A0), \
	AROS_LCA(int, (___purpose), D0), \
	struct Library *, AMISSL_BASE_NAME, 1467, Amissl)

#define SSL_CTX_set_trust(___s, ___trust) \
	AROS_LC2(int, SSL_CTX_set_trust, \
	AROS_LCA(SSL_CTX *, (___s), A0), \
	AROS_LCA(int, (___trust), D0), \
	struct Library *, AMISSL_BASE_NAME, 1468, Amissl)

#define SSL_set_trust(___s, ___trust) \
	AROS_LC2(int, SSL_set_trust, \
	AROS_LCA(SSL *, (___s), A0), \
	AROS_LCA(int, (___trust), D0), \
	struct Library *, AMISSL_BASE_NAME, 1469, Amissl)

#define SSL_free(___ssl) \
	AROS_LC1(void, SSL_free, \
	AROS_LCA(SSL *, (___ssl), A0), \
	struct Library *, AMISSL_BASE_NAME, 1470, Amissl)

#define SSL_accept(___ssl) \
	AROS_LC1(int, SSL_accept, \
	AROS_LCA(SSL *, (___ssl), A0), \
	struct Library *, AMISSL_BASE_NAME, 1471, Amissl)

#define SSL_connect(___ssl) \
	AROS_LC1(int, SSL_connect, \
	AROS_LCA(SSL *, (___ssl), A0), \
	struct Library *, AMISSL_BASE_NAME, 1472, Amissl)

#define SSL_read(___ssl, ___buf, ___num) \
	AROS_LC3(int, SSL_read, \
	AROS_LCA(SSL *, (___ssl), A0), \
	AROS_LCA(void *, (___buf), A1), \
	AROS_LCA(int, (___num), D0), \
	struct Library *, AMISSL_BASE_NAME, 1473, Amissl)

#define SSL_peek(___ssl, ___buf, ___num) \
	AROS_LC3(int, SSL_peek, \
	AROS_LCA(SSL *, (___ssl), A0), \
	AROS_LCA(void *, (___buf), A1), \
	AROS_LCA(int, (___num), D0), \
	struct Library *, AMISSL_BASE_NAME, 1474, Amissl)

#define SSL_write(___ssl, ___buf, ___num) \
	AROS_LC3(int, SSL_write, \
	AROS_LCA(SSL *, (___ssl), A0), \
	AROS_LCA(const void *, (___buf), A1), \
	AROS_LCA(int, (___num), D0), \
	struct Library *, AMISSL_BASE_NAME, 1475, Amissl)

#define SSL_ctrl(___ssl, ___cmd, ___larg, ___parg) \
	AROS_LC4(long, SSL_ctrl, \
	AROS_LCA(SSL *, (___ssl), A0), \
	AROS_LCA(int, (___cmd), D0), \
	AROS_LCA(long, (___larg), D1), \
	AROS_LCA(void *, (___parg), A1), \
	struct Library *, AMISSL_BASE_NAME, 1476, Amissl)

#define SSL_callback_ctrl(___a, ___a1, ___b) \
	AROS_LC3(long, SSL_callback_ctrl, \
	AROS_LCA(SSL *, (___a), A0), \
	AROS_LCA(int, (___a1), D0), \
	AROS_LCA(void (*)(), (___b), A1), \
	struct Library *, AMISSL_BASE_NAME, 1477, Amissl)

#define SSL_CTX_ctrl(___ctx, ___cmd, ___larg, ___parg) \
	AROS_LC4(long, SSL_CTX_ctrl, \
	AROS_LCA(SSL_CTX *, (___ctx), A0), \
	AROS_LCA(int, (___cmd), D0), \
	AROS_LCA(long, (___larg), D1), \
	AROS_LCA(void *, (___parg), A1), \
	struct Library *, AMISSL_BASE_NAME, 1478, Amissl)

#define SSL_CTX_callback_ctrl(___a1, ___a2, ___a3) \
	AROS_LC3(long, SSL_CTX_callback_ctrl, \
	AROS_LCA(SSL_CTX *, (___a1), A0), \
	AROS_LCA(int, (___a2), D0), \
	AROS_LCA(void (*)(), (___a3), A1), \
	struct Library *, AMISSL_BASE_NAME, 1479, Amissl)

#define SSL_get_error(___s, ___ret_code) \
	AROS_LC2(int, SSL_get_error, \
	AROS_LCA(const SSL *, (___s), A0), \
	AROS_LCA(int, (___ret_code), D0), \
	struct Library *, AMISSL_BASE_NAME, 1480, Amissl)

#define SSL_get_version(___s) \
	AROS_LC1(const char *, SSL_get_version, \
	AROS_LCA(const SSL *, (___s), A0), \
	struct Library *, AMISSL_BASE_NAME, 1481, Amissl)

#define SSL_CTX_set_ssl_version(___ctx, ___meth) \
	AROS_LC2(int, SSL_CTX_set_ssl_version, \
	AROS_LCA(SSL_CTX *, (___ctx), A0), \
	AROS_LCA(SSL_METHOD *, (___meth), A1), \
	struct Library *, AMISSL_BASE_NAME, 1482, Amissl)

#define SSLv2_method() \
	AROS_LC0(SSL_METHOD *, SSLv2_method, \
	struct Library *, AMISSL_BASE_NAME, 1483, Amissl)

#define SSLv2_server_method() \
	AROS_LC0(SSL_METHOD *, SSLv2_server_method, \
	struct Library *, AMISSL_BASE_NAME, 1484, Amissl)

#define SSLv2_client_method() \
	AROS_LC0(SSL_METHOD *, SSLv2_client_method, \
	struct Library *, AMISSL_BASE_NAME, 1485, Amissl)

#define SSLv3_method() \
	AROS_LC0(SSL_METHOD *, SSLv3_method, \
	struct Library *, AMISSL_BASE_NAME, 1486, Amissl)

#define SSLv3_server_method() \
	AROS_LC0(SSL_METHOD *, SSLv3_server_method, \
	struct Library *, AMISSL_BASE_NAME, 1487, Amissl)

#define SSLv3_client_method() \
	AROS_LC0(SSL_METHOD *, SSLv3_client_method, \
	struct Library *, AMISSL_BASE_NAME, 1488, Amissl)

#define SSLv23_method() \
	AROS_LC0(SSL_METHOD *, SSLv23_method, \
	struct Library *, AMISSL_BASE_NAME, 1489, Amissl)

#define SSLv23_server_method() \
	AROS_LC0(SSL_METHOD *, SSLv23_server_method, \
	struct Library *, AMISSL_BASE_NAME, 1490, Amissl)

#define SSLv23_client_method() \
	AROS_LC0(SSL_METHOD *, SSLv23_client_method, \
	struct Library *, AMISSL_BASE_NAME, 1491, Amissl)

#define TLSv1_method() \
	AROS_LC0(SSL_METHOD *, TLSv1_method, \
	struct Library *, AMISSL_BASE_NAME, 1492, Amissl)

#define TLSv1_server_method() \
	AROS_LC0(SSL_METHOD *, TLSv1_server_method, \
	struct Library *, AMISSL_BASE_NAME, 1493, Amissl)

#define TLSv1_client_method() \
	AROS_LC0(SSL_METHOD *, TLSv1_client_method, \
	struct Library *, AMISSL_BASE_NAME, 1494, Amissl)

#define SSL_get_ciphers(___s) \
	AROS_LC1(STACK_OF(SSL_CIPHER) *, SSL_get_ciphers, \
	AROS_LCA(const SSL *, (___s), A0), \
	struct Library *, AMISSL_BASE_NAME, 1495, Amissl)

#define SSL_do_handshake(___s) \
	AROS_LC1(int, SSL_do_handshake, \
	AROS_LCA(SSL *, (___s), A0), \
	struct Library *, AMISSL_BASE_NAME, 1496, Amissl)

#define SSL_renegotiate(___s) \
	AROS_LC1(int, SSL_renegotiate, \
	AROS_LCA(SSL *, (___s), A0), \
	struct Library *, AMISSL_BASE_NAME, 1497, Amissl)

#define SSL_renegotiate_pending(___s) \
	AROS_LC1(int, SSL_renegotiate_pending, \
	AROS_LCA(SSL *, (___s), A0), \
	struct Library *, AMISSL_BASE_NAME, 1498, Amissl)

#define SSL_shutdown(___s) \
	AROS_LC1(int, SSL_shutdown, \
	AROS_LCA(SSL *, (___s), A0), \
	struct Library *, AMISSL_BASE_NAME, 1499, Amissl)

#define SSL_get_ssl_method(___s) \
	AROS_LC1(SSL_METHOD *, SSL_get_ssl_method, \
	AROS_LCA(SSL *, (___s), A0), \
	struct Library *, AMISSL_BASE_NAME, 1500, Amissl)

#define SSL_set_ssl_method(___s, ___method) \
	AROS_LC2(int, SSL_set_ssl_method, \
	AROS_LCA(SSL *, (___s), A0), \
	AROS_LCA(SSL_METHOD *, (___method), A1), \
	struct Library *, AMISSL_BASE_NAME, 1501, Amissl)

#define SSL_alert_type_string_long(___value) \
	AROS_LC1(const char *, SSL_alert_type_string_long, \
	AROS_LCA(int, (___value), D0), \
	struct Library *, AMISSL_BASE_NAME, 1502, Amissl)

#define SSL_alert_type_string(___value) \
	AROS_LC1(const char *, SSL_alert_type_string, \
	AROS_LCA(int, (___value), D0), \
	struct Library *, AMISSL_BASE_NAME, 1503, Amissl)

#define SSL_alert_desc_string_long(___value) \
	AROS_LC1(const char *, SSL_alert_desc_string_long, \
	AROS_LCA(int, (___value), D0), \
	struct Library *, AMISSL_BASE_NAME, 1504, Amissl)

#define SSL_alert_desc_string(___value) \
	AROS_LC1(const char *, SSL_alert_desc_string, \
	AROS_LCA(int, (___value), D0), \
	struct Library *, AMISSL_BASE_NAME, 1505, Amissl)

#define SSL_set_client_CA_list(___s, ___name_list) \
	AROS_LC2(void, SSL_set_client_CA_list, \
	AROS_LCA(SSL *, (___s), A0), \
	AROS_LCA(STACK_OF(X509_NAME) *, (___name_list), A1), \
	struct Library *, AMISSL_BASE_NAME, 1506, Amissl)

#define SSL_CTX_set_client_CA_list(___ctx, ___name_list) \
	AROS_LC2(void, SSL_CTX_set_client_CA_list, \
	AROS_LCA(SSL_CTX *, (___ctx), A0), \
	AROS_LCA(STACK_OF(X509_NAME) *, (___name_list), A1), \
	struct Library *, AMISSL_BASE_NAME, 1507, Amissl)

#define SSL_get_client_CA_list(___s) \
	AROS_LC1(STACK_OF(X509_NAME) *, SSL_get_client_CA_list, \
	AROS_LCA(const SSL *, (___s), A0), \
	struct Library *, AMISSL_BASE_NAME, 1508, Amissl)

#define SSL_CTX_get_client_CA_list(___s) \
	AROS_LC1(STACK_OF(X509_NAME) *, SSL_CTX_get_client_CA_list, \
	AROS_LCA(const SSL_CTX *, (___s), A0), \
	struct Library *, AMISSL_BASE_NAME, 1509, Amissl)

#define SSL_add_client_CA(___ssl, ___x) \
	AROS_LC2(int, SSL_add_client_CA, \
	AROS_LCA(SSL *, (___ssl), A0), \
	AROS_LCA(X509 *, (___x), A1), \
	struct Library *, AMISSL_BASE_NAME, 1510, Amissl)

#define SSL_CTX_add_client_CA(___ctx, ___x) \
	AROS_LC2(int, SSL_CTX_add_client_CA, \
	AROS_LCA(SSL_CTX *, (___ctx), A0), \
	AROS_LCA(X509 *, (___x), A1), \
	struct Library *, AMISSL_BASE_NAME, 1511, Amissl)

#define SSL_set_connect_state(___s) \
	AROS_LC1(void, SSL_set_connect_state, \
	AROS_LCA(SSL *, (___s), A0), \
	struct Library *, AMISSL_BASE_NAME, 1512, Amissl)

#define SSL_set_accept_state(___s) \
	AROS_LC1(void, SSL_set_accept_state, \
	AROS_LCA(SSL *, (___s), A0), \
	struct Library *, AMISSL_BASE_NAME, 1513, Amissl)

#define SSL_get_default_timeout(___s) \
	AROS_LC1(long, SSL_get_default_timeout, \
	AROS_LCA(const SSL *, (___s), A0), \
	struct Library *, AMISSL_BASE_NAME, 1514, Amissl)

#define SSL_library_init() \
	AROS_LC0(int, SSL_library_init, \
	struct Library *, AMISSL_BASE_NAME, 1515, Amissl)

#define SSL_CIPHER_description(___a1, ___buf, ___size) \
	AROS_LC3(char *, SSL_CIPHER_description, \
	AROS_LCA(SSL_CIPHER *, (___a1), A0), \
	AROS_LCA(char *, (___buf), A1), \
	AROS_LCA(int, (___size), D0), \
	struct Library *, AMISSL_BASE_NAME, 1516, Amissl)

#define SSL_dup_CA_list(___sk) \
	AROS_LC1(STACK_OF(X509_NAME) *, SSL_dup_CA_list, \
	AROS_LCA(STACK_OF(X509_NAME) *, (___sk), A0), \
	struct Library *, AMISSL_BASE_NAME, 1517, Amissl)

#define SSL_dup(___ssl) \
	AROS_LC1(SSL *, SSL_dup, \
	AROS_LCA(SSL *, (___ssl), A0), \
	struct Library *, AMISSL_BASE_NAME, 1518, Amissl)

#define SSL_get_certificate(___ssl) \
	AROS_LC1(X509 *, SSL_get_certificate, \
	AROS_LCA(const SSL *, (___ssl), A0), \
	struct Library *, AMISSL_BASE_NAME, 1519, Amissl)

#define SSL_get_privatekey(___ssl) \
	AROS_LC1(struct evp_pkey_st *, SSL_get_privatekey, \
	AROS_LCA(SSL *, (___ssl), A0), \
	struct Library *, AMISSL_BASE_NAME, 1520, Amissl)

#define SSL_CTX_set_quiet_shutdown(___ctx, ___mode) \
	AROS_LC2(void, SSL_CTX_set_quiet_shutdown, \
	AROS_LCA(SSL_CTX *, (___ctx), A0), \
	AROS_LCA(int, (___mode), D0), \
	struct Library *, AMISSL_BASE_NAME, 1521, Amissl)

#define SSL_CTX_get_quiet_shutdown(___ctx) \
	AROS_LC1(int, SSL_CTX_get_quiet_shutdown, \
	AROS_LCA(const SSL_CTX *, (___ctx), A0), \
	struct Library *, AMISSL_BASE_NAME, 1522, Amissl)

#define SSL_set_quiet_shutdown(___ssl, ___mode) \
	AROS_LC2(void, SSL_set_quiet_shutdown, \
	AROS_LCA(SSL *, (___ssl), A0), \
	AROS_LCA(int, (___mode), D0), \
	struct Library *, AMISSL_BASE_NAME, 1523, Amissl)

#define SSL_get_quiet_shutdown(___ssl) \
	AROS_LC1(int, SSL_get_quiet_shutdown, \
	AROS_LCA(const SSL *, (___ssl), A0), \
	struct Library *, AMISSL_BASE_NAME, 1524, Amissl)

#define SSL_set_shutdown(___ssl, ___mode) \
	AROS_LC2(void, SSL_set_shutdown, \
	AROS_LCA(SSL *, (___ssl), A0), \
	AROS_LCA(int, (___mode), D0), \
	struct Library *, AMISSL_BASE_NAME, 1525, Amissl)

#define SSL_get_shutdown(___ssl) \
	AROS_LC1(int, SSL_get_shutdown, \
	AROS_LCA(const SSL *, (___ssl), A0), \
	struct Library *, AMISSL_BASE_NAME, 1526, Amissl)

#define SSL_version(___ssl) \
	AROS_LC1(int, SSL_version, \
	AROS_LCA(const SSL *, (___ssl), A0), \
	struct Library *, AMISSL_BASE_NAME, 1527, Amissl)

#define SSL_CTX_set_default_verify_paths(___ctx) \
	AROS_LC1(int, SSL_CTX_set_default_verify_paths, \
	AROS_LCA(SSL_CTX *, (___ctx), A0), \
	struct Library *, AMISSL_BASE_NAME, 1528, Amissl)

#define SSL_CTX_load_verify_locations(___ctx, ___CAfile, ___CApath) \
	AROS_LC3(int, SSL_CTX_load_verify_locations, \
	AROS_LCA(SSL_CTX *, (___ctx), A0), \
	AROS_LCA(const char *, (___CAfile), A1), \
	AROS_LCA(const char *, (___CApath), A2), \
	struct Library *, AMISSL_BASE_NAME, 1529, Amissl)

#define SSL_get_session(___ssl) \
	AROS_LC1(SSL_SESSION *, SSL_get_session, \
	AROS_LCA(const SSL *, (___ssl), A0), \
	struct Library *, AMISSL_BASE_NAME, 1530, Amissl)

#define SSL_get1_session(___ssl) \
	AROS_LC1(SSL_SESSION *, SSL_get1_session, \
	AROS_LCA(SSL *, (___ssl), A0), \
	struct Library *, AMISSL_BASE_NAME, 1531, Amissl)

#define SSL_get_SSL_CTX(___ssl) \
	AROS_LC1(SSL_CTX *, SSL_get_SSL_CTX, \
	AROS_LCA(const SSL *, (___ssl), A0), \
	struct Library *, AMISSL_BASE_NAME, 1532, Amissl)

#define SSL_set_info_callback(___ssl, ___cb) \
	AROS_LC2(void, SSL_set_info_callback, \
	AROS_LCA(SSL *, (___ssl), A0), \
	AROS_LCA(void (*)(const SSL *ssl,int type,int val), (___cb), A1), \
	struct Library *, AMISSL_BASE_NAME, 1533, Amissl)

#define SSL_get_info_callback(___ssl) \
	AROS_LC1(void *, SSL_get_info_callback, \
	AROS_LCA(const SSL *, (___ssl), A0), \
	struct Library *, AMISSL_BASE_NAME, 1534, Amissl)

#define SSL_state(___ssl) \
	AROS_LC1(int, SSL_state, \
	AROS_LCA(const SSL *, (___ssl), A0), \
	struct Library *, AMISSL_BASE_NAME, 1535, Amissl)

#define SSL_set_verify_result(___ssl, ___v) \
	AROS_LC2(void, SSL_set_verify_result, \
	AROS_LCA(SSL *, (___ssl), A0), \
	AROS_LCA(long, (___v), D0), \
	struct Library *, AMISSL_BASE_NAME, 1536, Amissl)

#define SSL_get_verify_result(___ssl) \
	AROS_LC1(long, SSL_get_verify_result, \
	AROS_LCA(const SSL *, (___ssl), A0), \
	struct Library *, AMISSL_BASE_NAME, 1537, Amissl)

#define SSL_set_ex_data(___ssl, ___idx, ___data) \
	AROS_LC3(int, SSL_set_ex_data, \
	AROS_LCA(SSL *, (___ssl), A0), \
	AROS_LCA(int, (___idx), D0), \
	AROS_LCA(void *, (___data), A1), \
	struct Library *, AMISSL_BASE_NAME, 1538, Amissl)

#define SSL_get_ex_data(___ssl, ___idx) \
	AROS_LC2(void *, SSL_get_ex_data, \
	AROS_LCA(const SSL *, (___ssl), A0), \
	AROS_LCA(int, (___idx), D0), \
	struct Library *, AMISSL_BASE_NAME, 1539, Amissl)

#define SSL_get_ex_new_index(___argl, ___argp, ___new_func, ___dup_func, ___free_func) \
	AROS_LC5(int, SSL_get_ex_new_index, \
	AROS_LCA(long, (___argl), D0), \
	AROS_LCA(void *, (___argp), A0), \
	AROS_LCA(CRYPTO_EX_new * (*)(), (___new_func), A1), \
	AROS_LCA(CRYPTO_EX_dup * (*)(), (___dup_func), A2), \
	AROS_LCA(CRYPTO_EX_free * (*)(), (___free_func), A3), \
	struct Library *, AMISSL_BASE_NAME, 1540, Amissl)

#define SSL_SESSION_set_ex_data(___ss, ___idx, ___data) \
	AROS_LC3(int, SSL_SESSION_set_ex_data, \
	AROS_LCA(SSL_SESSION *, (___ss), A0), \
	AROS_LCA(int, (___idx), D0), \
	AROS_LCA(void *, (___data), A1), \
	struct Library *, AMISSL_BASE_NAME, 1541, Amissl)

#define SSL_SESSION_get_ex_data(___ss, ___idx) \
	AROS_LC2(void *, SSL_SESSION_get_ex_data, \
	AROS_LCA(const SSL_SESSION *, (___ss), A0), \
	AROS_LCA(int, (___idx), D0), \
	struct Library *, AMISSL_BASE_NAME, 1542, Amissl)

#define SSL_SESSION_get_ex_new_index(___argl, ___argp, ___new_func, ___dup_func, ___free_func) \
	AROS_LC5(int, SSL_SESSION_get_ex_new_index, \
	AROS_LCA(long, (___argl), D0), \
	AROS_LCA(void *, (___argp), A0), \
	AROS_LCA(CRYPTO_EX_new * (*)(), (___new_func), A1), \
	AROS_LCA(CRYPTO_EX_dup * (*)(), (___dup_func), A2), \
	AROS_LCA(CRYPTO_EX_free * (*)(), (___free_func), A3), \
	struct Library *, AMISSL_BASE_NAME, 1543, Amissl)

#define SSL_CTX_set_ex_data(___ssl, ___idx, ___data) \
	AROS_LC3(int, SSL_CTX_set_ex_data, \
	AROS_LCA(SSL_CTX *, (___ssl), A0), \
	AROS_LCA(int, (___idx), D0), \
	AROS_LCA(void *, (___data), A1), \
	struct Library *, AMISSL_BASE_NAME, 1544, Amissl)

#define SSL_CTX_get_ex_data(___ssl, ___idx) \
	AROS_LC2(void *, SSL_CTX_get_ex_data, \
	AROS_LCA(const SSL_CTX *, (___ssl), A0), \
	AROS_LCA(int, (___idx), D0), \
	struct Library *, AMISSL_BASE_NAME, 1545, Amissl)

#define SSL_CTX_get_ex_new_index(___argl, ___argp, ___new_func, ___dup_func, ___free_func) \
	AROS_LC5(int, SSL_CTX_get_ex_new_index, \
	AROS_LCA(long, (___argl), D0), \
	AROS_LCA(void *, (___argp), A0), \
	AROS_LCA(CRYPTO_EX_new * (*)(), (___new_func), A1), \
	AROS_LCA(CRYPTO_EX_dup * (*)(), (___dup_func), A2), \
	AROS_LCA(CRYPTO_EX_free * (*)(), (___free_func), A3), \
	struct Library *, AMISSL_BASE_NAME, 1546, Amissl)

#define SSL_get_ex_data_X509_STORE_CTX_idx() \
	AROS_LC0(int, SSL_get_ex_data_X509_STORE_CTX_idx, \
	struct Library *, AMISSL_BASE_NAME, 1547, Amissl)

#define SSL_CTX_set_tmp_rsa_callback(___ctx, ___cb) \
	AROS_LC2(void, SSL_CTX_set_tmp_rsa_callback, \
	AROS_LCA(SSL_CTX *, (___ctx), A0), \
	AROS_LCA(RSA * (*)(SSL *ssl,int is_export,int keylength), (___cb), A1), \
	struct Library *, AMISSL_BASE_NAME, 1548, Amissl)

#define SSL_set_tmp_rsa_callback(___ssl, ___cb) \
	AROS_LC2(void, SSL_set_tmp_rsa_callback, \
	AROS_LCA(SSL *, (___ssl), A0), \
	AROS_LCA(RSA * (*)(SSL *ssl,int is_export,int keylength), (___cb), A1), \
	struct Library *, AMISSL_BASE_NAME, 1549, Amissl)

#define SSL_CTX_set_tmp_dh_callback(___ctx, ___dh) \
	AROS_LC2(void, SSL_CTX_set_tmp_dh_callback, \
	AROS_LCA(SSL_CTX *, (___ctx), A0), \
	AROS_LCA(DH * (*)(SSL *ssl,int is_export,int keylength), (___dh), A1), \
	struct Library *, AMISSL_BASE_NAME, 1550, Amissl)

#define SSL_set_tmp_dh_callback(___ssl, ___dh) \
	AROS_LC2(void, SSL_set_tmp_dh_callback, \
	AROS_LCA(SSL *, (___ssl), A0), \
	AROS_LCA(DH * (*)(SSL *ssl,int is_export,int keylength), (___dh), A1), \
	struct Library *, AMISSL_BASE_NAME, 1551, Amissl)

#define SSL_COMP_add_compression_method(___id, ___cm) \
	AROS_LC2(int, SSL_COMP_add_compression_method, \
	AROS_LCA(int, (___id), D0), \
	AROS_LCA(COMP_METHOD *, (___cm), A0), \
	struct Library *, AMISSL_BASE_NAME, 1552, Amissl)

#define ERR_load_SSL_strings() \
	AROS_LC0(void, ERR_load_SSL_strings, \
	struct Library *, AMISSL_BASE_NAME, 1553, Amissl)

#define sk_num(___a) \
	AROS_LC1(int, sk_num, \
	AROS_LCA(const STACK *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 1554, Amissl)

#define sk_value(___a, ___b) \
	AROS_LC2(char *, sk_value, \
	AROS_LCA(const STACK *, (___a), A0), \
	AROS_LCA(int, (___b), D0), \
	struct Library *, AMISSL_BASE_NAME, 1555, Amissl)

#define sk_set(___a, ___b, ___c) \
	AROS_LC3(char *, sk_set, \
	AROS_LCA(STACK *, (___a), A0), \
	AROS_LCA(int, (___b), D0), \
	AROS_LCA(char *, (___c), A1), \
	struct Library *, AMISSL_BASE_NAME, 1556, Amissl)

#define sk_new(___cmp) \
	AROS_LC1(STACK *, sk_new, \
	AROS_LCA(int (*)(const char *const *,const char *const *), (___cmp), A0), \
	struct Library *, AMISSL_BASE_NAME, 1557, Amissl)

#define sk_new_null() \
	AROS_LC0(STACK *, sk_new_null, \
	struct Library *, AMISSL_BASE_NAME, 1558, Amissl)

#define sk_free(___a) \
	AROS_LC1(void, sk_free, \
	AROS_LCA(STACK *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 1559, Amissl)

#define sk_pop_free(___st, ___func) \
	AROS_LC2(void, sk_pop_free, \
	AROS_LCA(STACK *, (___st), A0), \
	AROS_LCA(void (*)(void *), (___func), A1), \
	struct Library *, AMISSL_BASE_NAME, 1560, Amissl)

#define sk_insert(___sk, ___data, ___where) \
	AROS_LC3(int, sk_insert, \
	AROS_LCA(STACK *, (___sk), A0), \
	AROS_LCA(char *, (___data), A1), \
	AROS_LCA(int, (___where), D0), \
	struct Library *, AMISSL_BASE_NAME, 1561, Amissl)

#define sk_delete(___st, ___loc) \
	AROS_LC2(char *, sk_delete, \
	AROS_LCA(STACK *, (___st), A0), \
	AROS_LCA(int, (___loc), D0), \
	struct Library *, AMISSL_BASE_NAME, 1562, Amissl)

#define sk_delete_ptr(___st, ___p) \
	AROS_LC2(char *, sk_delete_ptr, \
	AROS_LCA(STACK *, (___st), A0), \
	AROS_LCA(char *, (___p), A1), \
	struct Library *, AMISSL_BASE_NAME, 1563, Amissl)

#define sk_find(___st, ___data) \
	AROS_LC2(int, sk_find, \
	AROS_LCA(STACK *, (___st), A0), \
	AROS_LCA(char *, (___data), A1), \
	struct Library *, AMISSL_BASE_NAME, 1564, Amissl)

#define sk_push(___st, ___data) \
	AROS_LC2(int, sk_push, \
	AROS_LCA(STACK *, (___st), A0), \
	AROS_LCA(char *, (___data), A1), \
	struct Library *, AMISSL_BASE_NAME, 1565, Amissl)

#define sk_unshift(___st, ___data) \
	AROS_LC2(int, sk_unshift, \
	AROS_LCA(STACK *, (___st), A0), \
	AROS_LCA(char *, (___data), A1), \
	struct Library *, AMISSL_BASE_NAME, 1566, Amissl)

#define sk_shift(___st) \
	AROS_LC1(char *, sk_shift, \
	AROS_LCA(STACK *, (___st), A0), \
	struct Library *, AMISSL_BASE_NAME, 1567, Amissl)

#define sk_pop(___st) \
	AROS_LC1(char *, sk_pop, \
	AROS_LCA(STACK *, (___st), A0), \
	struct Library *, AMISSL_BASE_NAME, 1568, Amissl)

#define sk_zero(___st) \
	AROS_LC1(void, sk_zero, \
	AROS_LCA(STACK *, (___st), A0), \
	struct Library *, AMISSL_BASE_NAME, 1569, Amissl)

#define sk_set_cmp_func(___sk, ___c) \
	AROS_LC2(int *, sk_set_cmp_func, \
	AROS_LCA(STACK *, (___sk), A0), \
	AROS_LCA(int (*)(const char *const *,const char *const *), (___c), A1), \
	struct Library *, AMISSL_BASE_NAME, 1570, Amissl)

#define sk_dup(___st) \
	AROS_LC1(STACK *, sk_dup, \
	AROS_LCA(STACK *, (___st), A0), \
	struct Library *, AMISSL_BASE_NAME, 1571, Amissl)

#define sk_sort(___st) \
	AROS_LC1(void, sk_sort, \
	AROS_LCA(STACK *, (___st), A0), \
	struct Library *, AMISSL_BASE_NAME, 1572, Amissl)

#define sk_is_sorted(___st) \
	AROS_LC1(int, sk_is_sorted, \
	AROS_LCA(const STACK *, (___st), A0), \
	struct Library *, AMISSL_BASE_NAME, 1573, Amissl)

#define ms_time_new() \
	AROS_LC0(char *, ms_time_new, \
	struct Library *, AMISSL_BASE_NAME, 1574, Amissl)

#define ms_time_free(___a) \
	AROS_LC1(void, ms_time_free, \
	AROS_LCA(char *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 1575, Amissl)

#define ms_time_get(___a) \
	AROS_LC1(void, ms_time_get, \
	AROS_LCA(char *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 1576, Amissl)

#define ms_time_diff(___start, ___end) \
	AROS_LC2(double, ms_time_diff, \
	AROS_LCA(char *, (___start), A0), \
	AROS_LCA(char *, (___end), A1), \
	struct Library *, AMISSL_BASE_NAME, 1577, Amissl)

#define ms_time_cmp(___ap, ___bp) \
	AROS_LC2(int, ms_time_cmp, \
	AROS_LCA(char *, (___ap), A0), \
	AROS_LCA(char *, (___bp), A1), \
	struct Library *, AMISSL_BASE_NAME, 1578, Amissl)

#define TXT_DB_read(___in, ___num) \
	AROS_LC2(TXT_DB *, TXT_DB_read, \
	AROS_LCA(BIO *, (___in), A0), \
	AROS_LCA(int, (___num), D0), \
	struct Library *, AMISSL_BASE_NAME, 1579, Amissl)

#define TXT_DB_write(___out, ___db) \
	AROS_LC2(long, TXT_DB_write, \
	AROS_LCA(BIO *, (___out), A0), \
	AROS_LCA(TXT_DB *, (___db), A1), \
	struct Library *, AMISSL_BASE_NAME, 1580, Amissl)

#define TXT_DB_create_index(___db, ___field, ___qual, ___hash, ___cmp) \
	AROS_LC5(int, TXT_DB_create_index, \
	AROS_LCA(TXT_DB *, (___db), A0), \
	AROS_LCA(int, (___field), D0), \
	AROS_LCA(int (*)(), (___qual), A1), \
	AROS_LCA(LHASH_HASH_FN_TYPE, (___hash), D1), \
	AROS_LCA(LHASH_COMP_FN_TYPE, (___cmp), D2), \
	struct Library *, AMISSL_BASE_NAME, 1581, Amissl)

#define TXT_DB_free(___db) \
	AROS_LC1(void, TXT_DB_free, \
	AROS_LCA(TXT_DB *, (___db), A0), \
	struct Library *, AMISSL_BASE_NAME, 1582, Amissl)

#define TXT_DB_get_by_index(___db, ___idx, ___value) \
	AROS_LC3(char **, TXT_DB_get_by_index, \
	AROS_LCA(TXT_DB *, (___db), A0), \
	AROS_LCA(int, (___idx), D0), \
	AROS_LCA(char **, (___value), A1), \
	struct Library *, AMISSL_BASE_NAME, 1583, Amissl)

#define TXT_DB_insert(___db, ___value) \
	AROS_LC2(int, TXT_DB_insert, \
	AROS_LCA(TXT_DB *, (___db), A0), \
	AROS_LCA(char **, (___value), A1), \
	struct Library *, AMISSL_BASE_NAME, 1584, Amissl)

#define UI_new() \
	AROS_LC0(UI *, UI_new, \
	struct Library *, AMISSL_BASE_NAME, 1585, Amissl)

#define UI_new_method(___method) \
	AROS_LC1(UI *, UI_new_method, \
	AROS_LCA(const UI_METHOD *, (___method), A0), \
	struct Library *, AMISSL_BASE_NAME, 1586, Amissl)

#define UI_free(___ui) \
	AROS_LC1(void, UI_free, \
	AROS_LCA(UI *, (___ui), A0), \
	struct Library *, AMISSL_BASE_NAME, 1587, Amissl)

#define UI_add_input_string(___ui, ___prompt, ___flags, ___result_buf, ___minsize, ___maxsize) \
	AROS_LC6(int, UI_add_input_string, \
	AROS_LCA(UI *, (___ui), A0), \
	AROS_LCA(const char *, (___prompt), A1), \
	AROS_LCA(int, (___flags), D0), \
	AROS_LCA(char *, (___result_buf), A2), \
	AROS_LCA(int, (___minsize), D1), \
	AROS_LCA(int, (___maxsize), D2), \
	struct Library *, AMISSL_BASE_NAME, 1588, Amissl)

#define UI_dup_input_string(___ui, ___prompt, ___flags, ___result_buf, ___minsize, ___maxsize) \
	AROS_LC6(int, UI_dup_input_string, \
	AROS_LCA(UI *, (___ui), A0), \
	AROS_LCA(const char *, (___prompt), A1), \
	AROS_LCA(int, (___flags), D0), \
	AROS_LCA(char *, (___result_buf), A2), \
	AROS_LCA(int, (___minsize), D1), \
	AROS_LCA(int, (___maxsize), D2), \
	struct Library *, AMISSL_BASE_NAME, 1589, Amissl)

#define UI_add_verify_string(___ui, ___prompt, ___flags, ___result_buf, ___minsize, ___maxsize, ___test_buf) \
	AROS_LC7(int, UI_add_verify_string, \
	AROS_LCA(UI *, (___ui), A0), \
	AROS_LCA(const char *, (___prompt), A1), \
	AROS_LCA(int, (___flags), D0), \
	AROS_LCA(char *, (___result_buf), A2), \
	AROS_LCA(int, (___minsize), D1), \
	AROS_LCA(int, (___maxsize), D2), \
	AROS_LCA(const char *, (___test_buf), A3), \
	struct Library *, AMISSL_BASE_NAME, 1590, Amissl)

#define UI_dup_verify_string(___ui, ___prompt, ___flags, ___result_buf, ___minsize, ___maxsize, ___test_buf) \
	AROS_LC7(int, UI_dup_verify_string, \
	AROS_LCA(UI *, (___ui), A0), \
	AROS_LCA(const char *, (___prompt), A1), \
	AROS_LCA(int, (___flags), D0), \
	AROS_LCA(char *, (___result_buf), A2), \
	AROS_LCA(int, (___minsize), D1), \
	AROS_LCA(int, (___maxsize), D2), \
	AROS_LCA(const char *, (___test_buf), A3), \
	struct Library *, AMISSL_BASE_NAME, 1591, Amissl)

#define UI_add_input_boolean(___ui, ___prompt, ___action_desc, ___ok_chars, ___cancel_chars, ___flags, ___result_buf) \
	AROS_LC7(int, UI_add_input_boolean, \
	AROS_LCA(UI *, (___ui), A0), \
	AROS_LCA(const char *, (___prompt), A1), \
	AROS_LCA(const char *, (___action_desc), A2), \
	AROS_LCA(const char *, (___ok_chars), A3), \
	AROS_LCA(const char *, (___cancel_chars), D0), \
	AROS_LCA(int, (___flags), D1), \
	AROS_LCA(char *, (___result_buf), D2), \
	struct Library *, AMISSL_BASE_NAME, 1592, Amissl)

#define UI_dup_input_boolean(___ui, ___prompt, ___action_desc, ___ok_chars, ___cancel_chars, ___flags, ___result_buf) \
	AROS_LC7(int, UI_dup_input_boolean, \
	AROS_LCA(UI *, (___ui), A0), \
	AROS_LCA(const char *, (___prompt), A1), \
	AROS_LCA(const char *, (___action_desc), A2), \
	AROS_LCA(const char *, (___ok_chars), A3), \
	AROS_LCA(const char *, (___cancel_chars), D0), \
	AROS_LCA(int, (___flags), D1), \
	AROS_LCA(char *, (___result_buf), D2), \
	struct Library *, AMISSL_BASE_NAME, 1593, Amissl)

#define UI_add_info_string(___ui, ___text) \
	AROS_LC2(int, UI_add_info_string, \
	AROS_LCA(UI *, (___ui), A0), \
	AROS_LCA(const char *, (___text), A1), \
	struct Library *, AMISSL_BASE_NAME, 1594, Amissl)

#define UI_dup_info_string(___ui, ___text) \
	AROS_LC2(int, UI_dup_info_string, \
	AROS_LCA(UI *, (___ui), A0), \
	AROS_LCA(const char *, (___text), A1), \
	struct Library *, AMISSL_BASE_NAME, 1595, Amissl)

#define UI_add_error_string(___ui, ___text) \
	AROS_LC2(int, UI_add_error_string, \
	AROS_LCA(UI *, (___ui), A0), \
	AROS_LCA(const char *, (___text), A1), \
	struct Library *, AMISSL_BASE_NAME, 1596, Amissl)

#define UI_dup_error_string(___ui, ___text) \
	AROS_LC2(int, UI_dup_error_string, \
	AROS_LCA(UI *, (___ui), A0), \
	AROS_LCA(const char *, (___text), A1), \
	struct Library *, AMISSL_BASE_NAME, 1597, Amissl)

#define UI_construct_prompt(___ui_method, ___object_desc, ___object_name) \
	AROS_LC3(char *, UI_construct_prompt, \
	AROS_LCA(UI *, (___ui_method), A0), \
	AROS_LCA(const char *, (___object_desc), A1), \
	AROS_LCA(const char *, (___object_name), A2), \
	struct Library *, AMISSL_BASE_NAME, 1598, Amissl)

#define UI_add_user_data(___ui, ___user_data) \
	AROS_LC2(void *, UI_add_user_data, \
	AROS_LCA(UI *, (___ui), A0), \
	AROS_LCA(void *, (___user_data), A1), \
	struct Library *, AMISSL_BASE_NAME, 1599, Amissl)

#define UI_get0_user_data(___ui) \
	AROS_LC1(void *, UI_get0_user_data, \
	AROS_LCA(UI *, (___ui), A0), \
	struct Library *, AMISSL_BASE_NAME, 1600, Amissl)

#define UI_get0_result(___ui, ___i) \
	AROS_LC2(const char *, UI_get0_result, \
	AROS_LCA(UI *, (___ui), A0), \
	AROS_LCA(int, (___i), D0), \
	struct Library *, AMISSL_BASE_NAME, 1601, Amissl)

#define UI_process(___ui) \
	AROS_LC1(int, UI_process, \
	AROS_LCA(UI *, (___ui), A0), \
	struct Library *, AMISSL_BASE_NAME, 1602, Amissl)

#define UI_ctrl(___ui, ___cmd, ___i, ___p, ___f) \
	AROS_LC5(int, UI_ctrl, \
	AROS_LCA(UI *, (___ui), A0), \
	AROS_LCA(int, (___cmd), D0), \
	AROS_LCA(long, (___i), D1), \
	AROS_LCA(void *, (___p), A1), \
	AROS_LCA(void (*)(), (___f), A2), \
	struct Library *, AMISSL_BASE_NAME, 1603, Amissl)

#define UI_get_ex_new_index(___argl, ___argp, ___new_func, ___dup_func, ___free_func) \
	AROS_LC5(int, UI_get_ex_new_index, \
	AROS_LCA(long, (___argl), D0), \
	AROS_LCA(void *, (___argp), A0), \
	AROS_LCA(CRYPTO_EX_new * (*)(), (___new_func), A1), \
	AROS_LCA(CRYPTO_EX_dup * (*)(), (___dup_func), A2), \
	AROS_LCA(CRYPTO_EX_free * (*)(), (___free_func), A3), \
	struct Library *, AMISSL_BASE_NAME, 1604, Amissl)

#define UI_set_ex_data(___r, ___idx, ___arg) \
	AROS_LC3(int, UI_set_ex_data, \
	AROS_LCA(UI *, (___r), A0), \
	AROS_LCA(int, (___idx), D0), \
	AROS_LCA(void *, (___arg), A1), \
	struct Library *, AMISSL_BASE_NAME, 1605, Amissl)

#define UI_get_ex_data(___r, ___idx) \
	AROS_LC2(void *, UI_get_ex_data, \
	AROS_LCA(UI *, (___r), A0), \
	AROS_LCA(int, (___idx), D0), \
	struct Library *, AMISSL_BASE_NAME, 1606, Amissl)

#define UI_set_default_method(___meth) \
	AROS_LC1(void, UI_set_default_method, \
	AROS_LCA(const UI_METHOD *, (___meth), A0), \
	struct Library *, AMISSL_BASE_NAME, 1607, Amissl)

#define UI_get_default_method() \
	AROS_LC0(const UI_METHOD *, UI_get_default_method, \
	struct Library *, AMISSL_BASE_NAME, 1608, Amissl)

#define UI_get_method(___ui) \
	AROS_LC1(const UI_METHOD *, UI_get_method, \
	AROS_LCA(UI *, (___ui), A0), \
	struct Library *, AMISSL_BASE_NAME, 1609, Amissl)

#define UI_set_method(___ui, ___meth) \
	AROS_LC2(const UI_METHOD *, UI_set_method, \
	AROS_LCA(UI *, (___ui), A0), \
	AROS_LCA(const UI_METHOD *, (___meth), A1), \
	struct Library *, AMISSL_BASE_NAME, 1610, Amissl)

#define UI_OpenSSL() \
	AROS_LC0(UI_METHOD *, UI_OpenSSL, \
	struct Library *, AMISSL_BASE_NAME, 1611, Amissl)

#define UI_create_method(___name) \
	AROS_LC1(UI_METHOD *, UI_create_method, \
	AROS_LCA(char *, (___name), A0), \
	struct Library *, AMISSL_BASE_NAME, 1612, Amissl)

#define UI_destroy_method(___ui_method) \
	AROS_LC1(void, UI_destroy_method, \
	AROS_LCA(UI_METHOD *, (___ui_method), A0), \
	struct Library *, AMISSL_BASE_NAME, 1613, Amissl)

#define UI_method_set_opener(___method, ___opener) \
	AROS_LC2(int, UI_method_set_opener, \
	AROS_LCA(UI_METHOD *, (___method), A0), \
	AROS_LCA(int (*)(UI *ui), (___opener), A1), \
	struct Library *, AMISSL_BASE_NAME, 1614, Amissl)

#define UI_method_set_writer(___method, ___writer) \
	AROS_LC2(int, UI_method_set_writer, \
	AROS_LCA(UI_METHOD *, (___method), A0), \
	AROS_LCA(int (*)(UI *ui,UI_STRING *uis), (___writer), A1), \
	struct Library *, AMISSL_BASE_NAME, 1615, Amissl)

#define UI_method_set_flusher(___method, ___flusher) \
	AROS_LC2(int, UI_method_set_flusher, \
	AROS_LCA(UI_METHOD *, (___method), A0), \
	AROS_LCA(int (*)(UI *ui), (___flusher), A1), \
	struct Library *, AMISSL_BASE_NAME, 1616, Amissl)

#define UI_method_set_reader(___method, ___reader) \
	AROS_LC2(int, UI_method_set_reader, \
	AROS_LCA(UI_METHOD *, (___method), A0), \
	AROS_LCA(int (*)(UI *ui,UI_STRING *uis), (___reader), A1), \
	struct Library *, AMISSL_BASE_NAME, 1617, Amissl)

#define UI_method_set_closer(___method, ___closer) \
	AROS_LC2(int, UI_method_set_closer, \
	AROS_LCA(UI_METHOD *, (___method), A0), \
	AROS_LCA(int (*)(UI *ui), (___closer), A1), \
	struct Library *, AMISSL_BASE_NAME, 1618, Amissl)

#define UI_method_get_opener(___method) \
	AROS_LC1(int *, UI_method_get_opener, \
	AROS_LCA(UI_METHOD *, (___method), A0), \
	struct Library *, AMISSL_BASE_NAME, 1619, Amissl)

#define UI_method_get_writer(___method) \
	AROS_LC1(int *, UI_method_get_writer, \
	AROS_LCA(UI_METHOD *, (___method), A0), \
	struct Library *, AMISSL_BASE_NAME, 1620, Amissl)

#define UI_method_get_flusher(___method) \
	AROS_LC1(int *, UI_method_get_flusher, \
	AROS_LCA(UI_METHOD *, (___method), A0), \
	struct Library *, AMISSL_BASE_NAME, 1621, Amissl)

#define UI_method_get_reader(___method) \
	AROS_LC1(int *, UI_method_get_reader, \
	AROS_LCA(UI_METHOD *, (___method), A0), \
	struct Library *, AMISSL_BASE_NAME, 1622, Amissl)

#define UI_method_get_closer(___method) \
	AROS_LC1(int *, UI_method_get_closer, \
	AROS_LCA(UI_METHOD *, (___method), A0), \
	struct Library *, AMISSL_BASE_NAME, 1623, Amissl)

#define UI_get_string_type(___uis) \
	AROS_LC1(enum UI_string_types, UI_get_string_type, \
	AROS_LCA(UI_STRING *, (___uis), A0), \
	struct Library *, AMISSL_BASE_NAME, 1624, Amissl)

#define UI_get_input_flags(___uis) \
	AROS_LC1(int, UI_get_input_flags, \
	AROS_LCA(UI_STRING *, (___uis), A0), \
	struct Library *, AMISSL_BASE_NAME, 1625, Amissl)

#define UI_get0_output_string(___uis) \
	AROS_LC1(const char *, UI_get0_output_string, \
	AROS_LCA(UI_STRING *, (___uis), A0), \
	struct Library *, AMISSL_BASE_NAME, 1626, Amissl)

#define UI_get0_action_string(___uis) \
	AROS_LC1(const char *, UI_get0_action_string, \
	AROS_LCA(UI_STRING *, (___uis), A0), \
	struct Library *, AMISSL_BASE_NAME, 1627, Amissl)

#define UI_get0_result_string(___uis) \
	AROS_LC1(const char *, UI_get0_result_string, \
	AROS_LCA(UI_STRING *, (___uis), A0), \
	struct Library *, AMISSL_BASE_NAME, 1628, Amissl)

#define UI_get0_test_string(___uis) \
	AROS_LC1(const char *, UI_get0_test_string, \
	AROS_LCA(UI_STRING *, (___uis), A0), \
	struct Library *, AMISSL_BASE_NAME, 1629, Amissl)

#define UI_get_result_minsize(___uis) \
	AROS_LC1(int, UI_get_result_minsize, \
	AROS_LCA(UI_STRING *, (___uis), A0), \
	struct Library *, AMISSL_BASE_NAME, 1630, Amissl)

#define UI_get_result_maxsize(___uis) \
	AROS_LC1(int, UI_get_result_maxsize, \
	AROS_LCA(UI_STRING *, (___uis), A0), \
	struct Library *, AMISSL_BASE_NAME, 1631, Amissl)

#define UI_set_result(___ui, ___uis, ___result) \
	AROS_LC3(int, UI_set_result, \
	AROS_LCA(UI *, (___ui), A0), \
	AROS_LCA(UI_STRING *, (___uis), A1), \
	AROS_LCA(const char *, (___result), A2), \
	struct Library *, AMISSL_BASE_NAME, 1632, Amissl)

#define UI_UTIL_read_pw_string(___buf, ___length, ___prompt, ___verify) \
	AROS_LC4(int, UI_UTIL_read_pw_string, \
	AROS_LCA(char *, (___buf), A0), \
	AROS_LCA(int, (___length), D0), \
	AROS_LCA(const char *, (___prompt), A1), \
	AROS_LCA(int, (___verify), D1), \
	struct Library *, AMISSL_BASE_NAME, 1633, Amissl)

#define UI_UTIL_read_pw(___buf, ___buff, ___size, ___prompt, ___verify) \
	AROS_LC5(int, UI_UTIL_read_pw, \
	AROS_LCA(char *, (___buf), A0), \
	AROS_LCA(char *, (___buff), A1), \
	AROS_LCA(int, (___size), D0), \
	AROS_LCA(const char *, (___prompt), A2), \
	AROS_LCA(int, (___verify), D1), \
	struct Library *, AMISSL_BASE_NAME, 1634, Amissl)

#define ERR_load_UI_strings() \
	AROS_LC0(void, ERR_load_UI_strings, \
	struct Library *, AMISSL_BASE_NAME, 1635, Amissl)

#define _ossl_old_des_read_pw_string(___buf, ___length, ___prompt, ___verify) \
	AROS_LC4(int, _ossl_old_des_read_pw_string, \
	AROS_LCA(char *, (___buf), A0), \
	AROS_LCA(int, (___length), D0), \
	AROS_LCA(const char *, (___prompt), A1), \
	AROS_LCA(int, (___verify), D1), \
	struct Library *, AMISSL_BASE_NAME, 1636, Amissl)

#define _ossl_old_des_read_pw(___buf, ___buff, ___size, ___prompt, ___verify) \
	AROS_LC5(int, _ossl_old_des_read_pw, \
	AROS_LCA(char *, (___buf), A0), \
	AROS_LCA(char *, (___buff), A1), \
	AROS_LCA(int, (___size), D0), \
	AROS_LCA(const char *, (___prompt), A2), \
	AROS_LCA(int, (___verify), D1), \
	struct Library *, AMISSL_BASE_NAME, 1637, Amissl)

#define X509_verify_cert_error_string(___n) \
	AROS_LC1(const char *, X509_verify_cert_error_string, \
	AROS_LCA(long, (___n), D0), \
	struct Library *, AMISSL_BASE_NAME, 1638, Amissl)

#define X509_verify(___a, ___r) \
	AROS_LC2(int, X509_verify, \
	AROS_LCA(X509 *, (___a), A0), \
	AROS_LCA(EVP_PKEY *, (___r), A1), \
	struct Library *, AMISSL_BASE_NAME, 1639, Amissl)

#define X509_REQ_verify(___a, ___r) \
	AROS_LC2(int, X509_REQ_verify, \
	AROS_LCA(X509_REQ *, (___a), A0), \
	AROS_LCA(EVP_PKEY *, (___r), A1), \
	struct Library *, AMISSL_BASE_NAME, 1640, Amissl)

#define X509_CRL_verify(___a, ___r) \
	AROS_LC2(int, X509_CRL_verify, \
	AROS_LCA(X509_CRL *, (___a), A0), \
	AROS_LCA(EVP_PKEY *, (___r), A1), \
	struct Library *, AMISSL_BASE_NAME, 1641, Amissl)

#define NETSCAPE_SPKI_verify(___a, ___r) \
	AROS_LC2(int, NETSCAPE_SPKI_verify, \
	AROS_LCA(NETSCAPE_SPKI *, (___a), A0), \
	AROS_LCA(EVP_PKEY *, (___r), A1), \
	struct Library *, AMISSL_BASE_NAME, 1642, Amissl)

#define NETSCAPE_SPKI_b64_decode(___str, ___len) \
	AROS_LC2(NETSCAPE_SPKI *, NETSCAPE_SPKI_b64_decode, \
	AROS_LCA(const char *, (___str), A0), \
	AROS_LCA(int, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 1643, Amissl)

#define NETSCAPE_SPKI_b64_encode(___x) \
	AROS_LC1(char *, NETSCAPE_SPKI_b64_encode, \
	AROS_LCA(NETSCAPE_SPKI *, (___x), A0), \
	struct Library *, AMISSL_BASE_NAME, 1644, Amissl)

#define NETSCAPE_SPKI_get_pubkey(___x) \
	AROS_LC1(EVP_PKEY *, NETSCAPE_SPKI_get_pubkey, \
	AROS_LCA(NETSCAPE_SPKI *, (___x), A0), \
	struct Library *, AMISSL_BASE_NAME, 1645, Amissl)

#define NETSCAPE_SPKI_set_pubkey(___x, ___pkey) \
	AROS_LC2(int, NETSCAPE_SPKI_set_pubkey, \
	AROS_LCA(NETSCAPE_SPKI *, (___x), A0), \
	AROS_LCA(EVP_PKEY *, (___pkey), A1), \
	struct Library *, AMISSL_BASE_NAME, 1646, Amissl)

#define NETSCAPE_SPKI_print(___out, ___spki) \
	AROS_LC2(int, NETSCAPE_SPKI_print, \
	AROS_LCA(BIO *, (___out), A0), \
	AROS_LCA(NETSCAPE_SPKI *, (___spki), A1), \
	struct Library *, AMISSL_BASE_NAME, 1647, Amissl)

#define X509_signature_print(___bp, ___alg, ___sig) \
	AROS_LC3(int, X509_signature_print, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(X509_ALGOR *, (___alg), A1), \
	AROS_LCA(ASN1_STRING *, (___sig), A2), \
	struct Library *, AMISSL_BASE_NAME, 1648, Amissl)

#define X509_sign(___x, ___pkey, ___md) \
	AROS_LC3(int, X509_sign, \
	AROS_LCA(X509 *, (___x), A0), \
	AROS_LCA(EVP_PKEY *, (___pkey), A1), \
	AROS_LCA(const EVP_MD *, (___md), A2), \
	struct Library *, AMISSL_BASE_NAME, 1649, Amissl)

#define X509_REQ_sign(___x, ___pkey, ___md) \
	AROS_LC3(int, X509_REQ_sign, \
	AROS_LCA(X509_REQ *, (___x), A0), \
	AROS_LCA(EVP_PKEY *, (___pkey), A1), \
	AROS_LCA(const EVP_MD *, (___md), A2), \
	struct Library *, AMISSL_BASE_NAME, 1650, Amissl)

#define X509_CRL_sign(___x, ___pkey, ___md) \
	AROS_LC3(int, X509_CRL_sign, \
	AROS_LCA(X509_CRL *, (___x), A0), \
	AROS_LCA(EVP_PKEY *, (___pkey), A1), \
	AROS_LCA(const EVP_MD *, (___md), A2), \
	struct Library *, AMISSL_BASE_NAME, 1651, Amissl)

#define NETSCAPE_SPKI_sign(___x, ___pkey, ___md) \
	AROS_LC3(int, NETSCAPE_SPKI_sign, \
	AROS_LCA(NETSCAPE_SPKI *, (___x), A0), \
	AROS_LCA(EVP_PKEY *, (___pkey), A1), \
	AROS_LCA(const EVP_MD *, (___md), A2), \
	struct Library *, AMISSL_BASE_NAME, 1652, Amissl)

#define X509_pubkey_digest(___data, ___type, ___md, ___len) \
	AROS_LC4(int, X509_pubkey_digest, \
	AROS_LCA(const X509 *, (___data), A0), \
	AROS_LCA(const EVP_MD *, (___type), A1), \
	AROS_LCA(unsigned char *, (___md), A2), \
	AROS_LCA(unsigned int *, (___len), A3), \
	struct Library *, AMISSL_BASE_NAME, 1653, Amissl)

#define X509_digest(___data, ___type, ___md, ___len) \
	AROS_LC4(int, X509_digest, \
	AROS_LCA(const X509 *, (___data), A0), \
	AROS_LCA(const EVP_MD *, (___type), A1), \
	AROS_LCA(unsigned char *, (___md), A2), \
	AROS_LCA(unsigned int *, (___len), A3), \
	struct Library *, AMISSL_BASE_NAME, 1654, Amissl)

#define X509_CRL_digest(___data, ___type, ___md, ___len) \
	AROS_LC4(int, X509_CRL_digest, \
	AROS_LCA(const X509_CRL *, (___data), A0), \
	AROS_LCA(const EVP_MD *, (___type), A1), \
	AROS_LCA(unsigned char *, (___md), A2), \
	AROS_LCA(unsigned int *, (___len), A3), \
	struct Library *, AMISSL_BASE_NAME, 1655, Amissl)

#define X509_REQ_digest(___data, ___type, ___md, ___len) \
	AROS_LC4(int, X509_REQ_digest, \
	AROS_LCA(const X509_REQ *, (___data), A0), \
	AROS_LCA(const EVP_MD *, (___type), A1), \
	AROS_LCA(unsigned char *, (___md), A2), \
	AROS_LCA(unsigned int *, (___len), A3), \
	struct Library *, AMISSL_BASE_NAME, 1656, Amissl)

#define X509_NAME_digest(___data, ___type, ___md, ___len) \
	AROS_LC4(int, X509_NAME_digest, \
	AROS_LCA(const X509_NAME *, (___data), A0), \
	AROS_LCA(const EVP_MD *, (___type), A1), \
	AROS_LCA(unsigned char *, (___md), A2), \
	AROS_LCA(unsigned int *, (___len), A3), \
	struct Library *, AMISSL_BASE_NAME, 1657, Amissl)

#define d2i_X509_bio(___bp, ___x509) \
	AROS_LC2(X509 *, d2i_X509_bio, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(X509 **, (___x509), A1), \
	struct Library *, AMISSL_BASE_NAME, 1658, Amissl)

#define i2d_X509_bio(___bp, ___x509) \
	AROS_LC2(int, i2d_X509_bio, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(X509 *, (___x509), A1), \
	struct Library *, AMISSL_BASE_NAME, 1659, Amissl)

#define d2i_X509_CRL_bio(___bp, ___crl) \
	AROS_LC2(X509_CRL *, d2i_X509_CRL_bio, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(X509_CRL **, (___crl), A1), \
	struct Library *, AMISSL_BASE_NAME, 1660, Amissl)

#define i2d_X509_CRL_bio(___bp, ___crl) \
	AROS_LC2(int, i2d_X509_CRL_bio, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(X509_CRL *, (___crl), A1), \
	struct Library *, AMISSL_BASE_NAME, 1661, Amissl)

#define d2i_X509_REQ_bio(___bp, ___req) \
	AROS_LC2(X509_REQ *, d2i_X509_REQ_bio, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(X509_REQ **, (___req), A1), \
	struct Library *, AMISSL_BASE_NAME, 1662, Amissl)

#define i2d_X509_REQ_bio(___bp, ___req) \
	AROS_LC2(int, i2d_X509_REQ_bio, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(X509_REQ *, (___req), A1), \
	struct Library *, AMISSL_BASE_NAME, 1663, Amissl)

#define d2i_RSAPrivateKey_bio(___bp, ___rsa) \
	AROS_LC2(RSA *, d2i_RSAPrivateKey_bio, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(RSA **, (___rsa), A1), \
	struct Library *, AMISSL_BASE_NAME, 1664, Amissl)

#define i2d_RSAPrivateKey_bio(___bp, ___rsa) \
	AROS_LC2(int, i2d_RSAPrivateKey_bio, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(RSA *, (___rsa), A1), \
	struct Library *, AMISSL_BASE_NAME, 1665, Amissl)

#define d2i_RSAPublicKey_bio(___bp, ___rsa) \
	AROS_LC2(RSA *, d2i_RSAPublicKey_bio, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(RSA **, (___rsa), A1), \
	struct Library *, AMISSL_BASE_NAME, 1666, Amissl)

#define i2d_RSAPublicKey_bio(___bp, ___rsa) \
	AROS_LC2(int, i2d_RSAPublicKey_bio, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(RSA *, (___rsa), A1), \
	struct Library *, AMISSL_BASE_NAME, 1667, Amissl)

#define d2i_RSA_PUBKEY_bio(___bp, ___rsa) \
	AROS_LC2(RSA *, d2i_RSA_PUBKEY_bio, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(RSA **, (___rsa), A1), \
	struct Library *, AMISSL_BASE_NAME, 1668, Amissl)

#define i2d_RSA_PUBKEY_bio(___bp, ___rsa) \
	AROS_LC2(int, i2d_RSA_PUBKEY_bio, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(RSA *, (___rsa), A1), \
	struct Library *, AMISSL_BASE_NAME, 1669, Amissl)

#define d2i_DSA_PUBKEY_bio(___bp, ___dsa) \
	AROS_LC2(DSA *, d2i_DSA_PUBKEY_bio, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(DSA **, (___dsa), A1), \
	struct Library *, AMISSL_BASE_NAME, 1670, Amissl)

#define i2d_DSA_PUBKEY_bio(___bp, ___dsa) \
	AROS_LC2(int, i2d_DSA_PUBKEY_bio, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(DSA *, (___dsa), A1), \
	struct Library *, AMISSL_BASE_NAME, 1671, Amissl)

#define d2i_DSAPrivateKey_bio(___bp, ___dsa) \
	AROS_LC2(DSA *, d2i_DSAPrivateKey_bio, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(DSA **, (___dsa), A1), \
	struct Library *, AMISSL_BASE_NAME, 1672, Amissl)

#define i2d_DSAPrivateKey_bio(___bp, ___dsa) \
	AROS_LC2(int, i2d_DSAPrivateKey_bio, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(DSA *, (___dsa), A1), \
	struct Library *, AMISSL_BASE_NAME, 1673, Amissl)

#define d2i_PKCS8_bio(___bp, ___p8) \
	AROS_LC2(X509_SIG *, d2i_PKCS8_bio, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(X509_SIG **, (___p8), A1), \
	struct Library *, AMISSL_BASE_NAME, 1674, Amissl)

#define i2d_PKCS8_bio(___bp, ___p8) \
	AROS_LC2(int, i2d_PKCS8_bio, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(X509_SIG *, (___p8), A1), \
	struct Library *, AMISSL_BASE_NAME, 1675, Amissl)

#define d2i_PKCS8_PRIV_KEY_INFO_bio(___bp, ___p8inf) \
	AROS_LC2(PKCS8_PRIV_KEY_INFO *, d2i_PKCS8_PRIV_KEY_INFO_bio, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(PKCS8_PRIV_KEY_INFO **, (___p8inf), A1), \
	struct Library *, AMISSL_BASE_NAME, 1676, Amissl)

#define i2d_PKCS8_PRIV_KEY_INFO_bio(___bp, ___p8inf) \
	AROS_LC2(int, i2d_PKCS8_PRIV_KEY_INFO_bio, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(PKCS8_PRIV_KEY_INFO *, (___p8inf), A1), \
	struct Library *, AMISSL_BASE_NAME, 1677, Amissl)

#define i2d_PKCS8PrivateKeyInfo_bio(___bp, ___key) \
	AROS_LC2(int, i2d_PKCS8PrivateKeyInfo_bio, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(EVP_PKEY *, (___key), A1), \
	struct Library *, AMISSL_BASE_NAME, 1678, Amissl)

#define i2d_PrivateKey_bio(___bp, ___pkey) \
	AROS_LC2(int, i2d_PrivateKey_bio, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(EVP_PKEY *, (___pkey), A1), \
	struct Library *, AMISSL_BASE_NAME, 1679, Amissl)

#define d2i_PrivateKey_bio(___bp, ___a) \
	AROS_LC2(EVP_PKEY *, d2i_PrivateKey_bio, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(EVP_PKEY **, (___a), A1), \
	struct Library *, AMISSL_BASE_NAME, 1680, Amissl)

#define i2d_PUBKEY_bio(___bp, ___pkey) \
	AROS_LC2(int, i2d_PUBKEY_bio, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(EVP_PKEY *, (___pkey), A1), \
	struct Library *, AMISSL_BASE_NAME, 1681, Amissl)

#define d2i_PUBKEY_bio(___bp, ___a) \
	AROS_LC2(EVP_PKEY *, d2i_PUBKEY_bio, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(EVP_PKEY **, (___a), A1), \
	struct Library *, AMISSL_BASE_NAME, 1682, Amissl)

#define X509_dup(___x509) \
	AROS_LC1(X509 *, X509_dup, \
	AROS_LCA(X509 *, (___x509), A0), \
	struct Library *, AMISSL_BASE_NAME, 1683, Amissl)

#define X509_ATTRIBUTE_dup(___xa) \
	AROS_LC1(X509_ATTRIBUTE *, X509_ATTRIBUTE_dup, \
	AROS_LCA(X509_ATTRIBUTE *, (___xa), A0), \
	struct Library *, AMISSL_BASE_NAME, 1684, Amissl)

#define X509_EXTENSION_dup(___ex) \
	AROS_LC1(X509_EXTENSION *, X509_EXTENSION_dup, \
	AROS_LCA(X509_EXTENSION *, (___ex), A0), \
	struct Library *, AMISSL_BASE_NAME, 1685, Amissl)

#define X509_CRL_dup(___crl) \
	AROS_LC1(X509_CRL *, X509_CRL_dup, \
	AROS_LCA(X509_CRL *, (___crl), A0), \
	struct Library *, AMISSL_BASE_NAME, 1686, Amissl)

#define X509_REQ_dup(___req) \
	AROS_LC1(X509_REQ *, X509_REQ_dup, \
	AROS_LCA(X509_REQ *, (___req), A0), \
	struct Library *, AMISSL_BASE_NAME, 1687, Amissl)

#define X509_ALGOR_dup(___xn) \
	AROS_LC1(X509_ALGOR *, X509_ALGOR_dup, \
	AROS_LCA(X509_ALGOR *, (___xn), A0), \
	struct Library *, AMISSL_BASE_NAME, 1688, Amissl)

#define X509_NAME_dup(___xn) \
	AROS_LC1(X509_NAME *, X509_NAME_dup, \
	AROS_LCA(X509_NAME *, (___xn), A0), \
	struct Library *, AMISSL_BASE_NAME, 1689, Amissl)

#define X509_NAME_ENTRY_dup(___ne) \
	AROS_LC1(X509_NAME_ENTRY *, X509_NAME_ENTRY_dup, \
	AROS_LCA(X509_NAME_ENTRY *, (___ne), A0), \
	struct Library *, AMISSL_BASE_NAME, 1690, Amissl)

#define X509_cmp_time(___s, ___t) \
	AROS_LC2(int, X509_cmp_time, \
	AROS_LCA(ASN1_TIME *, (___s), A0), \
	AROS_LCA(time_t *, (___t), A1), \
	struct Library *, AMISSL_BASE_NAME, 1691, Amissl)

#define X509_cmp_current_time(___s) \
	AROS_LC1(int, X509_cmp_current_time, \
	AROS_LCA(ASN1_TIME *, (___s), A0), \
	struct Library *, AMISSL_BASE_NAME, 1692, Amissl)

#define X509_time_adj(___s, ___adj, ___t) \
	AROS_LC3(ASN1_TIME *, X509_time_adj, \
	AROS_LCA(ASN1_TIME *, (___s), A0), \
	AROS_LCA(long, (___adj), D0), \
	AROS_LCA(time_t *, (___t), A1), \
	struct Library *, AMISSL_BASE_NAME, 1693, Amissl)

#define X509_gmtime_adj(___s, ___adj) \
	AROS_LC2(ASN1_TIME *, X509_gmtime_adj, \
	AROS_LCA(ASN1_TIME *, (___s), A0), \
	AROS_LCA(long, (___adj), D0), \
	struct Library *, AMISSL_BASE_NAME, 1694, Amissl)

#define X509_get_default_cert_area() \
	AROS_LC0(const char *, X509_get_default_cert_area, \
	struct Library *, AMISSL_BASE_NAME, 1695, Amissl)

#define X509_get_default_cert_dir() \
	AROS_LC0(const char *, X509_get_default_cert_dir, \
	struct Library *, AMISSL_BASE_NAME, 1696, Amissl)

#define X509_get_default_cert_file() \
	AROS_LC0(const char *, X509_get_default_cert_file, \
	struct Library *, AMISSL_BASE_NAME, 1697, Amissl)

#define X509_get_default_cert_dir_env() \
	AROS_LC0(const char *, X509_get_default_cert_dir_env, \
	struct Library *, AMISSL_BASE_NAME, 1698, Amissl)

#define X509_get_default_cert_file_env() \
	AROS_LC0(const char *, X509_get_default_cert_file_env, \
	struct Library *, AMISSL_BASE_NAME, 1699, Amissl)

#define X509_get_default_private_dir() \
	AROS_LC0(const char *, X509_get_default_private_dir, \
	struct Library *, AMISSL_BASE_NAME, 1700, Amissl)

#define X509_to_X509_REQ(___x, ___pkey, ___md) \
	AROS_LC3(X509_REQ *, X509_to_X509_REQ, \
	AROS_LCA(X509 *, (___x), A0), \
	AROS_LCA(EVP_PKEY *, (___pkey), A1), \
	AROS_LCA(const EVP_MD *, (___md), A2), \
	struct Library *, AMISSL_BASE_NAME, 1701, Amissl)

#define X509_REQ_to_X509(___r, ___days, ___pkey) \
	AROS_LC3(X509 *, X509_REQ_to_X509, \
	AROS_LCA(X509_REQ *, (___r), A0), \
	AROS_LCA(int, (___days), D0), \
	AROS_LCA(EVP_PKEY *, (___pkey), A1), \
	struct Library *, AMISSL_BASE_NAME, 1702, Amissl)

#define X509_ALGOR_new() \
	AROS_LC0(X509_ALGOR *, X509_ALGOR_new, \
	struct Library *, AMISSL_BASE_NAME, 1703, Amissl)

#define X509_ALGOR_free(___a) \
	AROS_LC1(void, X509_ALGOR_free, \
	AROS_LCA(X509_ALGOR *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 1704, Amissl)

#define d2i_X509_ALGOR(___a, ___in, ___len) \
	AROS_LC3(X509_ALGOR *, d2i_X509_ALGOR, \
	AROS_LCA(X509_ALGOR **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 1705, Amissl)

#define i2d_X509_ALGOR(___a, ___out) \
	AROS_LC2(int, i2d_X509_ALGOR, \
	AROS_LCA(X509_ALGOR *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 1706, Amissl)

#define X509_ALGOR_it() \
	AROS_LC0(const ASN1_ITEM *, X509_ALGOR_it, \
	struct Library *, AMISSL_BASE_NAME, 1707, Amissl)

#define X509_VAL_new() \
	AROS_LC0(X509_VAL *, X509_VAL_new, \
	struct Library *, AMISSL_BASE_NAME, 1708, Amissl)

#define X509_VAL_free(___a) \
	AROS_LC1(void, X509_VAL_free, \
	AROS_LCA(X509_VAL *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 1709, Amissl)

#define d2i_X509_VAL(___a, ___in, ___len) \
	AROS_LC3(X509_VAL *, d2i_X509_VAL, \
	AROS_LCA(X509_VAL **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 1710, Amissl)

#define i2d_X509_VAL(___a, ___out) \
	AROS_LC2(int, i2d_X509_VAL, \
	AROS_LCA(X509_VAL *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 1711, Amissl)

#define X509_VAL_it() \
	AROS_LC0(const ASN1_ITEM *, X509_VAL_it, \
	struct Library *, AMISSL_BASE_NAME, 1712, Amissl)

#define X509_PUBKEY_new() \
	AROS_LC0(X509_PUBKEY *, X509_PUBKEY_new, \
	struct Library *, AMISSL_BASE_NAME, 1713, Amissl)

#define X509_PUBKEY_free(___a) \
	AROS_LC1(void, X509_PUBKEY_free, \
	AROS_LCA(X509_PUBKEY *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 1714, Amissl)

#define d2i_X509_PUBKEY(___a, ___in, ___len) \
	AROS_LC3(X509_PUBKEY *, d2i_X509_PUBKEY, \
	AROS_LCA(X509_PUBKEY **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 1715, Amissl)

#define i2d_X509_PUBKEY(___a, ___out) \
	AROS_LC2(int, i2d_X509_PUBKEY, \
	AROS_LCA(X509_PUBKEY *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 1716, Amissl)

#define X509_PUBKEY_it() \
	AROS_LC0(const ASN1_ITEM *, X509_PUBKEY_it, \
	struct Library *, AMISSL_BASE_NAME, 1717, Amissl)

#define X509_PUBKEY_set(___x, ___pkey) \
	AROS_LC2(int, X509_PUBKEY_set, \
	AROS_LCA(X509_PUBKEY **, (___x), A0), \
	AROS_LCA(EVP_PKEY *, (___pkey), A1), \
	struct Library *, AMISSL_BASE_NAME, 1718, Amissl)

#define X509_PUBKEY_get(___key) \
	AROS_LC1(EVP_PKEY *, X509_PUBKEY_get, \
	AROS_LCA(X509_PUBKEY *, (___key), A0), \
	struct Library *, AMISSL_BASE_NAME, 1719, Amissl)

#define X509_get_pubkey_parameters(___pkey, ___chain) \
	AROS_LC2(int, X509_get_pubkey_parameters, \
	AROS_LCA(EVP_PKEY *, (___pkey), A0), \
	AROS_LCA(STACK_OF(X509) *, (___chain), A1), \
	struct Library *, AMISSL_BASE_NAME, 1720, Amissl)

#define i2d_PUBKEY(___a, ___pp) \
	AROS_LC2(int, i2d_PUBKEY, \
	AROS_LCA(EVP_PKEY *, (___a), A0), \
	AROS_LCA(unsigned char **, (___pp), A1), \
	struct Library *, AMISSL_BASE_NAME, 1721, Amissl)

#define d2i_PUBKEY(___a, ___pp, ___length) \
	AROS_LC3(EVP_PKEY *, d2i_PUBKEY, \
	AROS_LCA(EVP_PKEY **, (___a), A0), \
	AROS_LCA(unsigned char **, (___pp), A1), \
	AROS_LCA(long, (___length), D0), \
	struct Library *, AMISSL_BASE_NAME, 1722, Amissl)

#define i2d_RSA_PUBKEY(___a, ___pp) \
	AROS_LC2(int, i2d_RSA_PUBKEY, \
	AROS_LCA(RSA *, (___a), A0), \
	AROS_LCA(unsigned char **, (___pp), A1), \
	struct Library *, AMISSL_BASE_NAME, 1723, Amissl)

#define d2i_RSA_PUBKEY(___a, ___pp, ___length) \
	AROS_LC3(RSA *, d2i_RSA_PUBKEY, \
	AROS_LCA(RSA **, (___a), A0), \
	AROS_LCA(unsigned char **, (___pp), A1), \
	AROS_LCA(long, (___length), D0), \
	struct Library *, AMISSL_BASE_NAME, 1724, Amissl)

#define i2d_DSA_PUBKEY(___a, ___pp) \
	AROS_LC2(int, i2d_DSA_PUBKEY, \
	AROS_LCA(DSA *, (___a), A0), \
	AROS_LCA(unsigned char **, (___pp), A1), \
	struct Library *, AMISSL_BASE_NAME, 1725, Amissl)

#define d2i_DSA_PUBKEY(___a, ___pp, ___length) \
	AROS_LC3(DSA *, d2i_DSA_PUBKEY, \
	AROS_LCA(DSA **, (___a), A0), \
	AROS_LCA(unsigned char **, (___pp), A1), \
	AROS_LCA(long, (___length), D0), \
	struct Library *, AMISSL_BASE_NAME, 1726, Amissl)

#define X509_SIG_new() \
	AROS_LC0(X509_SIG *, X509_SIG_new, \
	struct Library *, AMISSL_BASE_NAME, 1727, Amissl)

#define X509_SIG_free(___a) \
	AROS_LC1(void, X509_SIG_free, \
	AROS_LCA(X509_SIG *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 1728, Amissl)

#define d2i_X509_SIG(___a, ___in, ___len) \
	AROS_LC3(X509_SIG *, d2i_X509_SIG, \
	AROS_LCA(X509_SIG **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 1729, Amissl)

#define i2d_X509_SIG(___a, ___out) \
	AROS_LC2(int, i2d_X509_SIG, \
	AROS_LCA(X509_SIG *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 1730, Amissl)

#define X509_SIG_it() \
	AROS_LC0(const ASN1_ITEM *, X509_SIG_it, \
	struct Library *, AMISSL_BASE_NAME, 1731, Amissl)

#define X509_REQ_INFO_new() \
	AROS_LC0(X509_REQ_INFO *, X509_REQ_INFO_new, \
	struct Library *, AMISSL_BASE_NAME, 1732, Amissl)

#define X509_REQ_INFO_free(___a) \
	AROS_LC1(void, X509_REQ_INFO_free, \
	AROS_LCA(X509_REQ_INFO *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 1733, Amissl)

#define d2i_X509_REQ_INFO(___a, ___in, ___len) \
	AROS_LC3(X509_REQ_INFO *, d2i_X509_REQ_INFO, \
	AROS_LCA(X509_REQ_INFO **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 1734, Amissl)

#define i2d_X509_REQ_INFO(___a, ___out) \
	AROS_LC2(int, i2d_X509_REQ_INFO, \
	AROS_LCA(X509_REQ_INFO *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 1735, Amissl)

#define X509_REQ_INFO_it() \
	AROS_LC0(const ASN1_ITEM *, X509_REQ_INFO_it, \
	struct Library *, AMISSL_BASE_NAME, 1736, Amissl)

#define X509_REQ_new() \
	AROS_LC0(X509_REQ *, X509_REQ_new, \
	struct Library *, AMISSL_BASE_NAME, 1737, Amissl)

#define X509_REQ_free(___a) \
	AROS_LC1(void, X509_REQ_free, \
	AROS_LCA(X509_REQ *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 1738, Amissl)

#define d2i_X509_REQ(___a, ___in, ___len) \
	AROS_LC3(X509_REQ *, d2i_X509_REQ, \
	AROS_LCA(X509_REQ **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 1739, Amissl)

#define i2d_X509_REQ(___a, ___out) \
	AROS_LC2(int, i2d_X509_REQ, \
	AROS_LCA(X509_REQ *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 1740, Amissl)

#define X509_REQ_it() \
	AROS_LC0(const ASN1_ITEM *, X509_REQ_it, \
	struct Library *, AMISSL_BASE_NAME, 1741, Amissl)

#define X509_ATTRIBUTE_new() \
	AROS_LC0(X509_ATTRIBUTE *, X509_ATTRIBUTE_new, \
	struct Library *, AMISSL_BASE_NAME, 1742, Amissl)

#define X509_ATTRIBUTE_free(___a) \
	AROS_LC1(void, X509_ATTRIBUTE_free, \
	AROS_LCA(X509_ATTRIBUTE *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 1743, Amissl)

#define d2i_X509_ATTRIBUTE(___a, ___in, ___len) \
	AROS_LC3(X509_ATTRIBUTE *, d2i_X509_ATTRIBUTE, \
	AROS_LCA(X509_ATTRIBUTE **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 1744, Amissl)

#define i2d_X509_ATTRIBUTE(___a, ___out) \
	AROS_LC2(int, i2d_X509_ATTRIBUTE, \
	AROS_LCA(X509_ATTRIBUTE *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 1745, Amissl)

#define X509_ATTRIBUTE_it() \
	AROS_LC0(const ASN1_ITEM *, X509_ATTRIBUTE_it, \
	struct Library *, AMISSL_BASE_NAME, 1746, Amissl)

#define X509_ATTRIBUTE_create(___nid, ___atrtype, ___value) \
	AROS_LC3(X509_ATTRIBUTE *, X509_ATTRIBUTE_create, \
	AROS_LCA(int, (___nid), D0), \
	AROS_LCA(int, (___atrtype), D1), \
	AROS_LCA(void *, (___value), A0), \
	struct Library *, AMISSL_BASE_NAME, 1747, Amissl)

#define X509_EXTENSION_new() \
	AROS_LC0(X509_EXTENSION *, X509_EXTENSION_new, \
	struct Library *, AMISSL_BASE_NAME, 1748, Amissl)

#define X509_EXTENSION_free(___a) \
	AROS_LC1(void, X509_EXTENSION_free, \
	AROS_LCA(X509_EXTENSION *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 1749, Amissl)

#define d2i_X509_EXTENSION(___a, ___in, ___len) \
	AROS_LC3(X509_EXTENSION *, d2i_X509_EXTENSION, \
	AROS_LCA(X509_EXTENSION **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 1750, Amissl)

#define i2d_X509_EXTENSION(___a, ___out) \
	AROS_LC2(int, i2d_X509_EXTENSION, \
	AROS_LCA(X509_EXTENSION *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 1751, Amissl)

#define X509_EXTENSION_it() \
	AROS_LC0(const ASN1_ITEM *, X509_EXTENSION_it, \
	struct Library *, AMISSL_BASE_NAME, 1752, Amissl)

#define X509_NAME_ENTRY_new() \
	AROS_LC0(X509_NAME_ENTRY *, X509_NAME_ENTRY_new, \
	struct Library *, AMISSL_BASE_NAME, 1753, Amissl)

#define X509_NAME_ENTRY_free(___a) \
	AROS_LC1(void, X509_NAME_ENTRY_free, \
	AROS_LCA(X509_NAME_ENTRY *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 1754, Amissl)

#define d2i_X509_NAME_ENTRY(___a, ___in, ___len) \
	AROS_LC3(X509_NAME_ENTRY *, d2i_X509_NAME_ENTRY, \
	AROS_LCA(X509_NAME_ENTRY **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 1755, Amissl)

#define i2d_X509_NAME_ENTRY(___a, ___out) \
	AROS_LC2(int, i2d_X509_NAME_ENTRY, \
	AROS_LCA(X509_NAME_ENTRY *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 1756, Amissl)

#define X509_NAME_ENTRY_it() \
	AROS_LC0(const ASN1_ITEM *, X509_NAME_ENTRY_it, \
	struct Library *, AMISSL_BASE_NAME, 1757, Amissl)

#define X509_NAME_new() \
	AROS_LC0(X509_NAME *, X509_NAME_new, \
	struct Library *, AMISSL_BASE_NAME, 1758, Amissl)

#define X509_NAME_free(___a) \
	AROS_LC1(void, X509_NAME_free, \
	AROS_LCA(X509_NAME *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 1759, Amissl)

#define d2i_X509_NAME(___a, ___in, ___len) \
	AROS_LC3(X509_NAME *, d2i_X509_NAME, \
	AROS_LCA(X509_NAME **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 1760, Amissl)

#define i2d_X509_NAME(___a, ___out) \
	AROS_LC2(int, i2d_X509_NAME, \
	AROS_LCA(X509_NAME *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 1761, Amissl)

#define X509_NAME_it() \
	AROS_LC0(const ASN1_ITEM *, X509_NAME_it, \
	struct Library *, AMISSL_BASE_NAME, 1762, Amissl)

#define X509_NAME_set(___xn, ___name) \
	AROS_LC2(int, X509_NAME_set, \
	AROS_LCA(X509_NAME **, (___xn), A0), \
	AROS_LCA(X509_NAME *, (___name), A1), \
	struct Library *, AMISSL_BASE_NAME, 1763, Amissl)

#define X509_CINF_new() \
	AROS_LC0(X509_CINF *, X509_CINF_new, \
	struct Library *, AMISSL_BASE_NAME, 1764, Amissl)

#define X509_CINF_free(___a) \
	AROS_LC1(void, X509_CINF_free, \
	AROS_LCA(X509_CINF *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 1765, Amissl)

#define d2i_X509_CINF(___a, ___in, ___len) \
	AROS_LC3(X509_CINF *, d2i_X509_CINF, \
	AROS_LCA(X509_CINF **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 1766, Amissl)

#define i2d_X509_CINF(___a, ___out) \
	AROS_LC2(int, i2d_X509_CINF, \
	AROS_LCA(X509_CINF *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 1767, Amissl)

#define X509_CINF_it() \
	AROS_LC0(const ASN1_ITEM *, X509_CINF_it, \
	struct Library *, AMISSL_BASE_NAME, 1768, Amissl)

#define X509_new() \
	AROS_LC0(X509 *, X509_new, \
	struct Library *, AMISSL_BASE_NAME, 1769, Amissl)

#define X509_free(___a) \
	AROS_LC1(void, X509_free, \
	AROS_LCA(X509 *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 1770, Amissl)

#define d2i_X509(___a, ___in, ___len) \
	AROS_LC3(X509 *, d2i_X509, \
	AROS_LCA(X509 **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 1771, Amissl)

#define i2d_X509(___a, ___out) \
	AROS_LC2(int, i2d_X509, \
	AROS_LCA(X509 *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 1772, Amissl)

#define X509_it() \
	AROS_LC0(const ASN1_ITEM *, X509_it, \
	struct Library *, AMISSL_BASE_NAME, 1773, Amissl)

#define X509_CERT_AUX_new() \
	AROS_LC0(X509_CERT_AUX *, X509_CERT_AUX_new, \
	struct Library *, AMISSL_BASE_NAME, 1774, Amissl)

#define X509_CERT_AUX_free(___a) \
	AROS_LC1(void, X509_CERT_AUX_free, \
	AROS_LCA(X509_CERT_AUX *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 1775, Amissl)

#define d2i_X509_CERT_AUX(___a, ___in, ___len) \
	AROS_LC3(X509_CERT_AUX *, d2i_X509_CERT_AUX, \
	AROS_LCA(X509_CERT_AUX **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 1776, Amissl)

#define i2d_X509_CERT_AUX(___a, ___out) \
	AROS_LC2(int, i2d_X509_CERT_AUX, \
	AROS_LCA(X509_CERT_AUX *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 1777, Amissl)

#define X509_CERT_AUX_it() \
	AROS_LC0(const ASN1_ITEM *, X509_CERT_AUX_it, \
	struct Library *, AMISSL_BASE_NAME, 1778, Amissl)

#define X509_get_ex_new_index(___argl, ___argp, ___new_func, ___dup_func, ___free_func) \
	AROS_LC5(int, X509_get_ex_new_index, \
	AROS_LCA(long, (___argl), D0), \
	AROS_LCA(void *, (___argp), A0), \
	AROS_LCA(CRYPTO_EX_new * (*)(), (___new_func), A1), \
	AROS_LCA(CRYPTO_EX_dup * (*)(), (___dup_func), A2), \
	AROS_LCA(CRYPTO_EX_free * (*)(), (___free_func), A3), \
	struct Library *, AMISSL_BASE_NAME, 1779, Amissl)

#define X509_set_ex_data(___r, ___idx, ___arg) \
	AROS_LC3(int, X509_set_ex_data, \
	AROS_LCA(X509 *, (___r), A0), \
	AROS_LCA(int, (___idx), D0), \
	AROS_LCA(void *, (___arg), A1), \
	struct Library *, AMISSL_BASE_NAME, 1780, Amissl)

#define X509_get_ex_data(___r, ___idx) \
	AROS_LC2(void *, X509_get_ex_data, \
	AROS_LCA(X509 *, (___r), A0), \
	AROS_LCA(int, (___idx), D0), \
	struct Library *, AMISSL_BASE_NAME, 1781, Amissl)

#define i2d_X509_AUX(___a, ___pp) \
	AROS_LC2(int, i2d_X509_AUX, \
	AROS_LCA(X509 *, (___a), A0), \
	AROS_LCA(unsigned char **, (___pp), A1), \
	struct Library *, AMISSL_BASE_NAME, 1782, Amissl)

#define d2i_X509_AUX(___a, ___pp, ___length) \
	AROS_LC3(X509 *, d2i_X509_AUX, \
	AROS_LCA(X509 **, (___a), A0), \
	AROS_LCA(unsigned char **, (___pp), A1), \
	AROS_LCA(long, (___length), D0), \
	struct Library *, AMISSL_BASE_NAME, 1783, Amissl)

#define X509_alias_set1(___x, ___name, ___len) \
	AROS_LC3(int, X509_alias_set1, \
	AROS_LCA(X509 *, (___x), A0), \
	AROS_LCA(unsigned char *, (___name), A1), \
	AROS_LCA(int, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 1784, Amissl)

#define X509_keyid_set1(___x, ___id, ___len) \
	AROS_LC3(int, X509_keyid_set1, \
	AROS_LCA(X509 *, (___x), A0), \
	AROS_LCA(unsigned char *, (___id), A1), \
	AROS_LCA(int, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 1785, Amissl)

#define X509_alias_get0(___x, ___len) \
	AROS_LC2(unsigned char *, X509_alias_get0, \
	AROS_LCA(X509 *, (___x), A0), \
	AROS_LCA(int *, (___len), A1), \
	struct Library *, AMISSL_BASE_NAME, 1786, Amissl)

#define t(___trust) \
	AROS_LC1(in, t, \
	AROS_LCA(*)(int) X509_TRUST_set_default(int (*)(int,X509 *,int), (___trust), A0), \
	struct Library *, AMISSL_BASE_NAME, 1787, Amissl)

#define X509_TRUST_set(___t, ___trust) \
	AROS_LC2(int, X509_TRUST_set, \
	AROS_LCA(int *, (___t), A0), \
	AROS_LCA(int, (___trust), D0), \
	struct Library *, AMISSL_BASE_NAME, 1788, Amissl)

#define X509_add1_trust_object(___x, ___obj) \
	AROS_LC2(int, X509_add1_trust_object, \
	AROS_LCA(X509 *, (___x), A0), \
	AROS_LCA(ASN1_OBJECT *, (___obj), A1), \
	struct Library *, AMISSL_BASE_NAME, 1789, Amissl)

#define X509_add1_reject_object(___x, ___obj) \
	AROS_LC2(int, X509_add1_reject_object, \
	AROS_LCA(X509 *, (___x), A0), \
	AROS_LCA(ASN1_OBJECT *, (___obj), A1), \
	struct Library *, AMISSL_BASE_NAME, 1790, Amissl)

#define X509_trust_clear(___x) \
	AROS_LC1(void, X509_trust_clear, \
	AROS_LCA(X509 *, (___x), A0), \
	struct Library *, AMISSL_BASE_NAME, 1791, Amissl)

#define X509_reject_clear(___x) \
	AROS_LC1(void, X509_reject_clear, \
	AROS_LCA(X509 *, (___x), A0), \
	struct Library *, AMISSL_BASE_NAME, 1792, Amissl)

#define X509_REVOKED_new() \
	AROS_LC0(X509_REVOKED *, X509_REVOKED_new, \
	struct Library *, AMISSL_BASE_NAME, 1793, Amissl)

#define X509_REVOKED_free(___a) \
	AROS_LC1(void, X509_REVOKED_free, \
	AROS_LCA(X509_REVOKED *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 1794, Amissl)

#define d2i_X509_REVOKED(___a, ___in, ___len) \
	AROS_LC3(X509_REVOKED *, d2i_X509_REVOKED, \
	AROS_LCA(X509_REVOKED **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 1795, Amissl)

#define i2d_X509_REVOKED(___a, ___out) \
	AROS_LC2(int, i2d_X509_REVOKED, \
	AROS_LCA(X509_REVOKED *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 1796, Amissl)

#define X509_REVOKED_it() \
	AROS_LC0(const ASN1_ITEM *, X509_REVOKED_it, \
	struct Library *, AMISSL_BASE_NAME, 1797, Amissl)

#define X509_CRL_INFO_new() \
	AROS_LC0(X509_CRL_INFO *, X509_CRL_INFO_new, \
	struct Library *, AMISSL_BASE_NAME, 1798, Amissl)

#define X509_CRL_INFO_free(___a) \
	AROS_LC1(void, X509_CRL_INFO_free, \
	AROS_LCA(X509_CRL_INFO *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 1799, Amissl)

#define d2i_X509_CRL_INFO(___a, ___in, ___len) \
	AROS_LC3(X509_CRL_INFO *, d2i_X509_CRL_INFO, \
	AROS_LCA(X509_CRL_INFO **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 1800, Amissl)

#define i2d_X509_CRL_INFO(___a, ___out) \
	AROS_LC2(int, i2d_X509_CRL_INFO, \
	AROS_LCA(X509_CRL_INFO *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 1801, Amissl)

#define X509_CRL_INFO_it() \
	AROS_LC0(const ASN1_ITEM *, X509_CRL_INFO_it, \
	struct Library *, AMISSL_BASE_NAME, 1802, Amissl)

#define X509_CRL_new() \
	AROS_LC0(X509_CRL *, X509_CRL_new, \
	struct Library *, AMISSL_BASE_NAME, 1803, Amissl)

#define X509_CRL_free(___a) \
	AROS_LC1(void, X509_CRL_free, \
	AROS_LCA(X509_CRL *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 1804, Amissl)

#define d2i_X509_CRL(___a, ___in, ___len) \
	AROS_LC3(X509_CRL *, d2i_X509_CRL, \
	AROS_LCA(X509_CRL **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 1805, Amissl)

#define i2d_X509_CRL(___a, ___out) \
	AROS_LC2(int, i2d_X509_CRL, \
	AROS_LCA(X509_CRL *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 1806, Amissl)

#define X509_CRL_it() \
	AROS_LC0(const ASN1_ITEM *, X509_CRL_it, \
	struct Library *, AMISSL_BASE_NAME, 1807, Amissl)

#define X509_CRL_add0_revoked(___crl, ___rev) \
	AROS_LC2(int, X509_CRL_add0_revoked, \
	AROS_LCA(X509_CRL *, (___crl), A0), \
	AROS_LCA(X509_REVOKED *, (___rev), A1), \
	struct Library *, AMISSL_BASE_NAME, 1808, Amissl)

#define X509_PKEY_new() \
	AROS_LC0(X509_PKEY *, X509_PKEY_new, \
	struct Library *, AMISSL_BASE_NAME, 1809, Amissl)

#define X509_PKEY_free(___a) \
	AROS_LC1(void, X509_PKEY_free, \
	AROS_LCA(X509_PKEY *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 1810, Amissl)

#define i2d_X509_PKEY(___a, ___pp) \
	AROS_LC2(int, i2d_X509_PKEY, \
	AROS_LCA(X509_PKEY *, (___a), A0), \
	AROS_LCA(unsigned char **, (___pp), A1), \
	struct Library *, AMISSL_BASE_NAME, 1811, Amissl)

#define d2i_X509_PKEY(___a, ___pp, ___length) \
	AROS_LC3(X509_PKEY *, d2i_X509_PKEY, \
	AROS_LCA(X509_PKEY **, (___a), A0), \
	AROS_LCA(unsigned char **, (___pp), A1), \
	AROS_LCA(long, (___length), D0), \
	struct Library *, AMISSL_BASE_NAME, 1812, Amissl)

#define NETSCAPE_SPKI_new() \
	AROS_LC0(NETSCAPE_SPKI *, NETSCAPE_SPKI_new, \
	struct Library *, AMISSL_BASE_NAME, 1813, Amissl)

#define NETSCAPE_SPKI_free(___a) \
	AROS_LC1(void, NETSCAPE_SPKI_free, \
	AROS_LCA(NETSCAPE_SPKI *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 1814, Amissl)

#define d2i_NETSCAPE_SPKI(___a, ___in, ___len) \
	AROS_LC3(NETSCAPE_SPKI *, d2i_NETSCAPE_SPKI, \
	AROS_LCA(NETSCAPE_SPKI **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 1815, Amissl)

#define i2d_NETSCAPE_SPKI(___a, ___out) \
	AROS_LC2(int, i2d_NETSCAPE_SPKI, \
	AROS_LCA(NETSCAPE_SPKI *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 1816, Amissl)

#define NETSCAPE_SPKI_it() \
	AROS_LC0(const ASN1_ITEM *, NETSCAPE_SPKI_it, \
	struct Library *, AMISSL_BASE_NAME, 1817, Amissl)

#define NETSCAPE_SPKAC_new() \
	AROS_LC0(NETSCAPE_SPKAC *, NETSCAPE_SPKAC_new, \
	struct Library *, AMISSL_BASE_NAME, 1818, Amissl)

#define NETSCAPE_SPKAC_free(___a) \
	AROS_LC1(void, NETSCAPE_SPKAC_free, \
	AROS_LCA(NETSCAPE_SPKAC *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 1819, Amissl)

#define d2i_NETSCAPE_SPKAC(___a, ___in, ___len) \
	AROS_LC3(NETSCAPE_SPKAC *, d2i_NETSCAPE_SPKAC, \
	AROS_LCA(NETSCAPE_SPKAC **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 1820, Amissl)

#define i2d_NETSCAPE_SPKAC(___a, ___out) \
	AROS_LC2(int, i2d_NETSCAPE_SPKAC, \
	AROS_LCA(NETSCAPE_SPKAC *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 1821, Amissl)

#define NETSCAPE_SPKAC_it() \
	AROS_LC0(const ASN1_ITEM *, NETSCAPE_SPKAC_it, \
	struct Library *, AMISSL_BASE_NAME, 1822, Amissl)

#define NETSCAPE_CERT_SEQUENCE_new() \
	AROS_LC0(NETSCAPE_CERT_SEQUENCE *, NETSCAPE_CERT_SEQUENCE_new, \
	struct Library *, AMISSL_BASE_NAME, 1823, Amissl)

#define NETSCAPE_CERT_SEQUENCE_free(___a) \
	AROS_LC1(void, NETSCAPE_CERT_SEQUENCE_free, \
	AROS_LCA(NETSCAPE_CERT_SEQUENCE *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 1824, Amissl)

#define d2i_NETSCAPE_CERT_SEQUENCE(___a, ___in, ___len) \
	AROS_LC3(NETSCAPE_CERT_SEQUENCE *, d2i_NETSCAPE_CERT_SEQUENCE, \
	AROS_LCA(NETSCAPE_CERT_SEQUENCE **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 1825, Amissl)

#define i2d_NETSCAPE_CERT_SEQUENCE(___a, ___out) \
	AROS_LC2(int, i2d_NETSCAPE_CERT_SEQUENCE, \
	AROS_LCA(NETSCAPE_CERT_SEQUENCE *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 1826, Amissl)

#define NETSCAPE_CERT_SEQUENCE_it() \
	AROS_LC0(const ASN1_ITEM *, NETSCAPE_CERT_SEQUENCE_it, \
	struct Library *, AMISSL_BASE_NAME, 1827, Amissl)

#define X509_INFO_new() \
	AROS_LC0(X509_INFO *, X509_INFO_new, \
	struct Library *, AMISSL_BASE_NAME, 1828, Amissl)

#define X509_INFO_free(___a) \
	AROS_LC1(void, X509_INFO_free, \
	AROS_LCA(X509_INFO *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 1829, Amissl)

#define X509_NAME_oneline(___a, ___buf, ___size) \
	AROS_LC3(char *, X509_NAME_oneline, \
	AROS_LCA(X509_NAME *, (___a), A0), \
	AROS_LCA(char *, (___buf), A1), \
	AROS_LCA(int, (___size), D0), \
	struct Library *, AMISSL_BASE_NAME, 1830, Amissl)

#define ASN1_verify(___i2d, ___algor1, ___signature, ___data, ___pkey) \
	AROS_LC5(int, ASN1_verify, \
	AROS_LCA(int (*)(), (___i2d), A0), \
	AROS_LCA(X509_ALGOR *, (___algor1), A1), \
	AROS_LCA(ASN1_BIT_STRING *, (___signature), A2), \
	AROS_LCA(char *, (___data), A3), \
	AROS_LCA(EVP_PKEY *, (___pkey), D0), \
	struct Library *, AMISSL_BASE_NAME, 1831, Amissl)

#define ASN1_digest(___i2d, ___type, ___data, ___md, ___len) \
	AROS_LC5(int, ASN1_digest, \
	AROS_LCA(int (*)(), (___i2d), A0), \
	AROS_LCA(const EVP_MD *, (___type), A1), \
	AROS_LCA(char *, (___data), A2), \
	AROS_LCA(unsigned char *, (___md), A3), \
	AROS_LCA(unsigned int *, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 1832, Amissl)

#define ASN1_sign(___i2d, ___algor1, ___algor2, ___signature, ___data, ___pkey, ___type) \
	AROS_LC7(int, ASN1_sign, \
	AROS_LCA(int (*)(), (___i2d), A0), \
	AROS_LCA(X509_ALGOR *, (___algor1), A1), \
	AROS_LCA(X509_ALGOR *, (___algor2), A2), \
	AROS_LCA(ASN1_BIT_STRING *, (___signature), A3), \
	AROS_LCA(char *, (___data), D0), \
	AROS_LCA(EVP_PKEY *, (___pkey), D1), \
	AROS_LCA(const EVP_MD *, (___type), D2), \
	struct Library *, AMISSL_BASE_NAME, 1833, Amissl)

#define ASN1_item_digest(___it, ___type, ___data, ___md, ___len) \
	AROS_LC5(int, ASN1_item_digest, \
	AROS_LCA(const ASN1_ITEM *, (___it), A0), \
	AROS_LCA(const EVP_MD *, (___type), A1), \
	AROS_LCA(void *, (___data), A2), \
	AROS_LCA(unsigned char *, (___md), A3), \
	AROS_LCA(unsigned int *, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 1834, Amissl)

#define ASN1_item_verify(___it, ___algor1, ___signature, ___data, ___pkey) \
	AROS_LC5(int, ASN1_item_verify, \
	AROS_LCA(const ASN1_ITEM *, (___it), A0), \
	AROS_LCA(X509_ALGOR *, (___algor1), A1), \
	AROS_LCA(ASN1_BIT_STRING *, (___signature), A2), \
	AROS_LCA(void *, (___data), A3), \
	AROS_LCA(EVP_PKEY *, (___pkey), D0), \
	struct Library *, AMISSL_BASE_NAME, 1835, Amissl)

#define ASN1_item_sign(___it, ___algor1, ___algor2, ___signature, ___data, ___pkey, ___type) \
	AROS_LC7(int, ASN1_item_sign, \
	AROS_LCA(const ASN1_ITEM *, (___it), A0), \
	AROS_LCA(X509_ALGOR *, (___algor1), A1), \
	AROS_LCA(X509_ALGOR *, (___algor2), A2), \
	AROS_LCA(ASN1_BIT_STRING *, (___signature), A3), \
	AROS_LCA(void *, (___data), D0), \
	AROS_LCA(EVP_PKEY *, (___pkey), D1), \
	AROS_LCA(const EVP_MD *, (___type), D2), \
	struct Library *, AMISSL_BASE_NAME, 1836, Amissl)

#define X509_set_version(___x, ___version) \
	AROS_LC2(int, X509_set_version, \
	AROS_LCA(X509 *, (___x), A0), \
	AROS_LCA(long, (___version), D0), \
	struct Library *, AMISSL_BASE_NAME, 1837, Amissl)

#define X509_set_serialNumber(___x, ___serial) \
	AROS_LC2(int, X509_set_serialNumber, \
	AROS_LCA(X509 *, (___x), A0), \
	AROS_LCA(ASN1_INTEGER *, (___serial), A1), \
	struct Library *, AMISSL_BASE_NAME, 1838, Amissl)

#define X509_get_serialNumber(___x) \
	AROS_LC1(ASN1_INTEGER *, X509_get_serialNumber, \
	AROS_LCA(X509 *, (___x), A0), \
	struct Library *, AMISSL_BASE_NAME, 1839, Amissl)

#define X509_set_issuer_name(___x, ___name) \
	AROS_LC2(int, X509_set_issuer_name, \
	AROS_LCA(X509 *, (___x), A0), \
	AROS_LCA(X509_NAME *, (___name), A1), \
	struct Library *, AMISSL_BASE_NAME, 1840, Amissl)

#define X509_get_issuer_name(___a) \
	AROS_LC1(X509_NAME *, X509_get_issuer_name, \
	AROS_LCA(X509 *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 1841, Amissl)

#define X509_set_subject_name(___x, ___name) \
	AROS_LC2(int, X509_set_subject_name, \
	AROS_LCA(X509 *, (___x), A0), \
	AROS_LCA(X509_NAME *, (___name), A1), \
	struct Library *, AMISSL_BASE_NAME, 1842, Amissl)

#define X509_get_subject_name(___a) \
	AROS_LC1(X509_NAME *, X509_get_subject_name, \
	AROS_LCA(X509 *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 1843, Amissl)

#define X509_set_notBefore(___x, ___tm) \
	AROS_LC2(int, X509_set_notBefore, \
	AROS_LCA(X509 *, (___x), A0), \
	AROS_LCA(ASN1_TIME *, (___tm), A1), \
	struct Library *, AMISSL_BASE_NAME, 1844, Amissl)

#define X509_set_notAfter(___x, ___tm) \
	AROS_LC2(int, X509_set_notAfter, \
	AROS_LCA(X509 *, (___x), A0), \
	AROS_LCA(ASN1_TIME *, (___tm), A1), \
	struct Library *, AMISSL_BASE_NAME, 1845, Amissl)

#define X509_set_pubkey(___x, ___pkey) \
	AROS_LC2(int, X509_set_pubkey, \
	AROS_LCA(X509 *, (___x), A0), \
	AROS_LCA(EVP_PKEY *, (___pkey), A1), \
	struct Library *, AMISSL_BASE_NAME, 1846, Amissl)

#define X509_get_pubkey(___x) \
	AROS_LC1(EVP_PKEY *, X509_get_pubkey, \
	AROS_LCA(X509 *, (___x), A0), \
	struct Library *, AMISSL_BASE_NAME, 1847, Amissl)

#define X509_get0_pubkey_bitstr(___x) \
	AROS_LC1(ASN1_BIT_STRING *, X509_get0_pubkey_bitstr, \
	AROS_LCA(const X509 *, (___x), A0), \
	struct Library *, AMISSL_BASE_NAME, 1848, Amissl)

#define X509_certificate_type(___x, ___pubkey) \
	AROS_LC2(int, X509_certificate_type, \
	AROS_LCA(X509 *, (___x), A0), \
	AROS_LCA(EVP_PKEY *, (___pubkey), A1), \
	struct Library *, AMISSL_BASE_NAME, 1849, Amissl)

#define X509_REQ_set_version(___x, ___version) \
	AROS_LC2(int, X509_REQ_set_version, \
	AROS_LCA(X509_REQ *, (___x), A0), \
	AROS_LCA(long, (___version), D0), \
	struct Library *, AMISSL_BASE_NAME, 1850, Amissl)

#define X509_REQ_set_subject_name(___req, ___name) \
	AROS_LC2(int, X509_REQ_set_subject_name, \
	AROS_LCA(X509_REQ *, (___req), A0), \
	AROS_LCA(X509_NAME *, (___name), A1), \
	struct Library *, AMISSL_BASE_NAME, 1851, Amissl)

#define X509_REQ_set_pubkey(___x, ___pkey) \
	AROS_LC2(int, X509_REQ_set_pubkey, \
	AROS_LCA(X509_REQ *, (___x), A0), \
	AROS_LCA(EVP_PKEY *, (___pkey), A1), \
	struct Library *, AMISSL_BASE_NAME, 1852, Amissl)

#define X509_REQ_get_pubkey(___req) \
	AROS_LC1(EVP_PKEY *, X509_REQ_get_pubkey, \
	AROS_LCA(X509_REQ *, (___req), A0), \
	struct Library *, AMISSL_BASE_NAME, 1853, Amissl)

#define X509_REQ_extension_nid(___nid) \
	AROS_LC1(int, X509_REQ_extension_nid, \
	AROS_LCA(int, (___nid), D0), \
	struct Library *, AMISSL_BASE_NAME, 1854, Amissl)

#define X509_REQ_get_extension_nids() \
	AROS_LC0(int *, X509_REQ_get_extension_nids, \
	struct Library *, AMISSL_BASE_NAME, 1855, Amissl)

#define X509_REQ_set_extension_nids(___nids) \
	AROS_LC1(void, X509_REQ_set_extension_nids, \
	AROS_LCA(int *, (___nids), A0), \
	struct Library *, AMISSL_BASE_NAME, 1856, Amissl)

#define X509_REQ_get_extensions(___req) \
	AROS_LC1(STACK_OF(X509_EXTENSION) *, X509_REQ_get_extensions, \
	AROS_LCA(X509_REQ *, (___req), A0), \
	struct Library *, AMISSL_BASE_NAME, 1857, Amissl)

#define X509_REQ_add_extensions_nid(___req, ___exts, ___nid) \
	AROS_LC3(int, X509_REQ_add_extensions_nid, \
	AROS_LCA(X509_REQ *, (___req), A0), \
	AROS_LCA(STACK_OF(X509_EXTENSION) *, (___exts), A1), \
	AROS_LCA(int, (___nid), D0), \
	struct Library *, AMISSL_BASE_NAME, 1858, Amissl)

#define X509_REQ_add_extensions(___req, ___exts) \
	AROS_LC2(int, X509_REQ_add_extensions, \
	AROS_LCA(X509_REQ *, (___req), A0), \
	AROS_LCA(STACK_OF(X509_EXTENSION) *, (___exts), A1), \
	struct Library *, AMISSL_BASE_NAME, 1859, Amissl)

#define X509_REQ_get_attr_count(___req) \
	AROS_LC1(int, X509_REQ_get_attr_count, \
	AROS_LCA(const X509_REQ *, (___req), A0), \
	struct Library *, AMISSL_BASE_NAME, 1860, Amissl)

#define X509_REQ_get_attr_by_NID(___req, ___nid, ___lastpos) \
	AROS_LC3(int, X509_REQ_get_attr_by_NID, \
	AROS_LCA(const X509_REQ *, (___req), A0), \
	AROS_LCA(int, (___nid), D0), \
	AROS_LCA(int, (___lastpos), D1), \
	struct Library *, AMISSL_BASE_NAME, 1861, Amissl)

#define X509_REQ_get_attr_by_OBJ(___req, ___obj, ___lastpos) \
	AROS_LC3(int, X509_REQ_get_attr_by_OBJ, \
	AROS_LCA(const X509_REQ *, (___req), A0), \
	AROS_LCA(ASN1_OBJECT *, (___obj), A1), \
	AROS_LCA(int, (___lastpos), D0), \
	struct Library *, AMISSL_BASE_NAME, 1862, Amissl)

#define X509_REQ_get_attr(___req, ___loc) \
	AROS_LC2(X509_ATTRIBUTE *, X509_REQ_get_attr, \
	AROS_LCA(const X509_REQ *, (___req), A0), \
	AROS_LCA(int, (___loc), D0), \
	struct Library *, AMISSL_BASE_NAME, 1863, Amissl)

#define X509_REQ_delete_attr(___req, ___loc) \
	AROS_LC2(X509_ATTRIBUTE *, X509_REQ_delete_attr, \
	AROS_LCA(X509_REQ *, (___req), A0), \
	AROS_LCA(int, (___loc), D0), \
	struct Library *, AMISSL_BASE_NAME, 1864, Amissl)

#define X509_REQ_add1_attr(___req, ___attr) \
	AROS_LC2(int, X509_REQ_add1_attr, \
	AROS_LCA(X509_REQ *, (___req), A0), \
	AROS_LCA(X509_ATTRIBUTE *, (___attr), A1), \
	struct Library *, AMISSL_BASE_NAME, 1865, Amissl)

#define X509_REQ_add1_attr_by_OBJ(___req, ___obj, ___type, ___bytes, ___len) \
	AROS_LC5(int, X509_REQ_add1_attr_by_OBJ, \
	AROS_LCA(X509_REQ *, (___req), A0), \
	AROS_LCA(const ASN1_OBJECT *, (___obj), A1), \
	AROS_LCA(int, (___type), D0), \
	AROS_LCA(const unsigned char *, (___bytes), A2), \
	AROS_LCA(int, (___len), D1), \
	struct Library *, AMISSL_BASE_NAME, 1866, Amissl)

#define X509_REQ_add1_attr_by_NID(___req, ___nid, ___type, ___bytes, ___len) \
	AROS_LC5(int, X509_REQ_add1_attr_by_NID, \
	AROS_LCA(X509_REQ *, (___req), A0), \
	AROS_LCA(int, (___nid), D0), \
	AROS_LCA(int, (___type), D1), \
	AROS_LCA(const unsigned char *, (___bytes), A1), \
	AROS_LCA(int, (___len), D2), \
	struct Library *, AMISSL_BASE_NAME, 1867, Amissl)

#define X509_REQ_add1_attr_by_txt(___req, ___attrname, ___type, ___bytes, ___len) \
	AROS_LC5(int, X509_REQ_add1_attr_by_txt, \
	AROS_LCA(X509_REQ *, (___req), A0), \
	AROS_LCA(const char *, (___attrname), A1), \
	AROS_LCA(int, (___type), D0), \
	AROS_LCA(const unsigned char *, (___bytes), A2), \
	AROS_LCA(int, (___len), D1), \
	struct Library *, AMISSL_BASE_NAME, 1868, Amissl)

#define X509_CRL_set_version(___x, ___version) \
	AROS_LC2(int, X509_CRL_set_version, \
	AROS_LCA(X509_CRL *, (___x), A0), \
	AROS_LCA(long, (___version), D0), \
	struct Library *, AMISSL_BASE_NAME, 1869, Amissl)

#define X509_CRL_set_issuer_name(___x, ___name) \
	AROS_LC2(int, X509_CRL_set_issuer_name, \
	AROS_LCA(X509_CRL *, (___x), A0), \
	AROS_LCA(X509_NAME *, (___name), A1), \
	struct Library *, AMISSL_BASE_NAME, 1870, Amissl)

#define X509_CRL_set_lastUpdate(___x, ___tm) \
	AROS_LC2(int, X509_CRL_set_lastUpdate, \
	AROS_LCA(X509_CRL *, (___x), A0), \
	AROS_LCA(ASN1_TIME *, (___tm), A1), \
	struct Library *, AMISSL_BASE_NAME, 1871, Amissl)

#define X509_CRL_set_nextUpdate(___x, ___tm) \
	AROS_LC2(int, X509_CRL_set_nextUpdate, \
	AROS_LCA(X509_CRL *, (___x), A0), \
	AROS_LCA(ASN1_TIME *, (___tm), A1), \
	struct Library *, AMISSL_BASE_NAME, 1872, Amissl)

#define X509_CRL_sort(___crl) \
	AROS_LC1(int, X509_CRL_sort, \
	AROS_LCA(X509_CRL *, (___crl), A0), \
	struct Library *, AMISSL_BASE_NAME, 1873, Amissl)

#define X509_REVOKED_set_serialNumber(___x, ___serial) \
	AROS_LC2(int, X509_REVOKED_set_serialNumber, \
	AROS_LCA(X509_REVOKED *, (___x), A0), \
	AROS_LCA(ASN1_INTEGER *, (___serial), A1), \
	struct Library *, AMISSL_BASE_NAME, 1874, Amissl)

#define X509_REVOKED_set_revocationDate(___r, ___tm) \
	AROS_LC2(int, X509_REVOKED_set_revocationDate, \
	AROS_LCA(X509_REVOKED *, (___r), A0), \
	AROS_LCA(ASN1_TIME *, (___tm), A1), \
	struct Library *, AMISSL_BASE_NAME, 1875, Amissl)

#define X509_check_private_key(___x509, ___pkey) \
	AROS_LC2(int, X509_check_private_key, \
	AROS_LCA(X509 *, (___x509), A0), \
	AROS_LCA(EVP_PKEY *, (___pkey), A1), \
	struct Library *, AMISSL_BASE_NAME, 1876, Amissl)

#define X509_issuer_and_serial_cmp(___a, ___b) \
	AROS_LC2(int, X509_issuer_and_serial_cmp, \
	AROS_LCA(const X509 *, (___a), A0), \
	AROS_LCA(const X509 *, (___b), A1), \
	struct Library *, AMISSL_BASE_NAME, 1877, Amissl)

#define X509_issuer_and_serial_hash(___a) \
	AROS_LC1(unsigned long, X509_issuer_and_serial_hash, \
	AROS_LCA(X509 *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 1878, Amissl)

#define X509_issuer_name_cmp(___a, ___b) \
	AROS_LC2(int, X509_issuer_name_cmp, \
	AROS_LCA(const X509 *, (___a), A0), \
	AROS_LCA(const X509 *, (___b), A1), \
	struct Library *, AMISSL_BASE_NAME, 1879, Amissl)

#define X509_issuer_name_hash(___a) \
	AROS_LC1(unsigned long, X509_issuer_name_hash, \
	AROS_LCA(X509 *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 1880, Amissl)

#define X509_subject_name_cmp(___a, ___b) \
	AROS_LC2(int, X509_subject_name_cmp, \
	AROS_LCA(const X509 *, (___a), A0), \
	AROS_LCA(const X509 *, (___b), A1), \
	struct Library *, AMISSL_BASE_NAME, 1881, Amissl)

#define X509_subject_name_hash(___x) \
	AROS_LC1(unsigned long, X509_subject_name_hash, \
	AROS_LCA(X509 *, (___x), A0), \
	struct Library *, AMISSL_BASE_NAME, 1882, Amissl)

#define X509_cmp(___a, ___b) \
	AROS_LC2(int, X509_cmp, \
	AROS_LCA(const X509 *, (___a), A0), \
	AROS_LCA(const X509 *, (___b), A1), \
	struct Library *, AMISSL_BASE_NAME, 1883, Amissl)

#define X509_NAME_cmp(___a, ___b) \
	AROS_LC2(int, X509_NAME_cmp, \
	AROS_LCA(const X509_NAME *, (___a), A0), \
	AROS_LCA(const X509_NAME *, (___b), A1), \
	struct Library *, AMISSL_BASE_NAME, 1884, Amissl)

#define X509_NAME_hash(___x) \
	AROS_LC1(unsigned long, X509_NAME_hash, \
	AROS_LCA(X509_NAME *, (___x), A0), \
	struct Library *, AMISSL_BASE_NAME, 1885, Amissl)

#define X509_CRL_cmp(___a, ___b) \
	AROS_LC2(int, X509_CRL_cmp, \
	AROS_LCA(const X509_CRL *, (___a), A0), \
	AROS_LCA(const X509_CRL *, (___b), A1), \
	struct Library *, AMISSL_BASE_NAME, 1886, Amissl)

#define X509_NAME_print(___bp, ___name, ___obase) \
	AROS_LC3(int, X509_NAME_print, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(X509_NAME *, (___name), A1), \
	AROS_LCA(int, (___obase), D0), \
	struct Library *, AMISSL_BASE_NAME, 1887, Amissl)

#define X509_NAME_print_ex(___out, ___nm, ___indent, ___flags) \
	AROS_LC4(int, X509_NAME_print_ex, \
	AROS_LCA(BIO *, (___out), A0), \
	AROS_LCA(X509_NAME *, (___nm), A1), \
	AROS_LCA(int, (___indent), D0), \
	AROS_LCA(unsigned long, (___flags), D1), \
	struct Library *, AMISSL_BASE_NAME, 1888, Amissl)

#define X509_print_ex(___bp, ___x, ___nmflag, ___cflag) \
	AROS_LC4(int, X509_print_ex, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(X509 *, (___x), A1), \
	AROS_LCA(unsigned long, (___nmflag), D0), \
	AROS_LCA(unsigned long, (___cflag), D1), \
	struct Library *, AMISSL_BASE_NAME, 1889, Amissl)

#define X509_print(___bp, ___x) \
	AROS_LC2(int, X509_print, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(X509 *, (___x), A1), \
	struct Library *, AMISSL_BASE_NAME, 1890, Amissl)

#define X509_ocspid_print(___bp, ___x) \
	AROS_LC2(int, X509_ocspid_print, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(X509 *, (___x), A1), \
	struct Library *, AMISSL_BASE_NAME, 1891, Amissl)

#define X509_CERT_AUX_print(___bp, ___x, ___indent) \
	AROS_LC3(int, X509_CERT_AUX_print, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(X509_CERT_AUX *, (___x), A1), \
	AROS_LCA(int, (___indent), D0), \
	struct Library *, AMISSL_BASE_NAME, 1892, Amissl)

#define X509_CRL_print(___bp, ___x) \
	AROS_LC2(int, X509_CRL_print, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(X509_CRL *, (___x), A1), \
	struct Library *, AMISSL_BASE_NAME, 1893, Amissl)

#define X509_REQ_print_ex(___bp, ___x, ___nmflag, ___cflag) \
	AROS_LC4(int, X509_REQ_print_ex, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(X509_REQ *, (___x), A1), \
	AROS_LCA(unsigned long, (___nmflag), D0), \
	AROS_LCA(unsigned long, (___cflag), D1), \
	struct Library *, AMISSL_BASE_NAME, 1894, Amissl)

#define X509_REQ_print(___bp, ___req) \
	AROS_LC2(int, X509_REQ_print, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(X509_REQ *, (___req), A1), \
	struct Library *, AMISSL_BASE_NAME, 1895, Amissl)

#define X509_NAME_entry_count(___name) \
	AROS_LC1(int, X509_NAME_entry_count, \
	AROS_LCA(X509_NAME *, (___name), A0), \
	struct Library *, AMISSL_BASE_NAME, 1896, Amissl)

#define X509_NAME_get_text_by_NID(___name, ___nid, ___buf, ___len) \
	AROS_LC4(int, X509_NAME_get_text_by_NID, \
	AROS_LCA(X509_NAME *, (___name), A0), \
	AROS_LCA(int, (___nid), D0), \
	AROS_LCA(char *, (___buf), A1), \
	AROS_LCA(int, (___len), D1), \
	struct Library *, AMISSL_BASE_NAME, 1897, Amissl)

#define X509_NAME_get_text_by_OBJ(___name, ___obj, ___buf, ___len) \
	AROS_LC4(int, X509_NAME_get_text_by_OBJ, \
	AROS_LCA(X509_NAME *, (___name), A0), \
	AROS_LCA(ASN1_OBJECT *, (___obj), A1), \
	AROS_LCA(char *, (___buf), A2), \
	AROS_LCA(int, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 1898, Amissl)

#define X509_NAME_get_index_by_NID(___name, ___nid, ___lastpos) \
	AROS_LC3(int, X509_NAME_get_index_by_NID, \
	AROS_LCA(X509_NAME *, (___name), A0), \
	AROS_LCA(int, (___nid), D0), \
	AROS_LCA(int, (___lastpos), D1), \
	struct Library *, AMISSL_BASE_NAME, 1899, Amissl)

#define X509_NAME_get_index_by_OBJ(___name, ___obj, ___lastpos) \
	AROS_LC3(int, X509_NAME_get_index_by_OBJ, \
	AROS_LCA(X509_NAME *, (___name), A0), \
	AROS_LCA(ASN1_OBJECT *, (___obj), A1), \
	AROS_LCA(int, (___lastpos), D0), \
	struct Library *, AMISSL_BASE_NAME, 1900, Amissl)

#define X509_NAME_get_entry(___name, ___loc) \
	AROS_LC2(X509_NAME_ENTRY *, X509_NAME_get_entry, \
	AROS_LCA(X509_NAME *, (___name), A0), \
	AROS_LCA(int, (___loc), D0), \
	struct Library *, AMISSL_BASE_NAME, 1901, Amissl)

#define X509_NAME_delete_entry(___name, ___loc) \
	AROS_LC2(X509_NAME_ENTRY *, X509_NAME_delete_entry, \
	AROS_LCA(X509_NAME *, (___name), A0), \
	AROS_LCA(int, (___loc), D0), \
	struct Library *, AMISSL_BASE_NAME, 1902, Amissl)

#define X509_NAME_add_entry(___name, ___ne, ___loc, ___set) \
	AROS_LC4(int, X509_NAME_add_entry, \
	AROS_LCA(X509_NAME *, (___name), A0), \
	AROS_LCA(X509_NAME_ENTRY *, (___ne), A1), \
	AROS_LCA(int, (___loc), D0), \
	AROS_LCA(int, (___set), D1), \
	struct Library *, AMISSL_BASE_NAME, 1903, Amissl)

#define X509_NAME_add_entry_by_OBJ(___name, ___obj, ___type, ___bytes, ___len, ___loc, ___set) \
	AROS_LC7(int, X509_NAME_add_entry_by_OBJ, \
	AROS_LCA(X509_NAME *, (___name), A0), \
	AROS_LCA(ASN1_OBJECT *, (___obj), A1), \
	AROS_LCA(int, (___type), D0), \
	AROS_LCA(unsigned char *, (___bytes), A2), \
	AROS_LCA(int, (___len), D1), \
	AROS_LCA(int, (___loc), D2), \
	AROS_LCA(int, (___set), D3), \
	struct Library *, AMISSL_BASE_NAME, 1904, Amissl)

#define X509_NAME_add_entry_by_NID(___name, ___nid, ___type, ___bytes, ___len, ___loc, ___set) \
	AROS_LC7(int, X509_NAME_add_entry_by_NID, \
	AROS_LCA(X509_NAME *, (___name), A0), \
	AROS_LCA(int, (___nid), D0), \
	AROS_LCA(int, (___type), D1), \
	AROS_LCA(unsigned char *, (___bytes), A1), \
	AROS_LCA(int, (___len), D2), \
	AROS_LCA(int, (___loc), D3), \
	AROS_LCA(int, (___set), D4), \
	struct Library *, AMISSL_BASE_NAME, 1905, Amissl)

#define X509_NAME_ENTRY_create_by_txt(___ne, ___field, ___type, ___bytes, ___len) \
	AROS_LC5(X509_NAME_ENTRY *, X509_NAME_ENTRY_create_by_txt, \
	AROS_LCA(X509_NAME_ENTRY **, (___ne), A0), \
	AROS_LCA(const char *, (___field), A1), \
	AROS_LCA(int, (___type), D0), \
	AROS_LCA(const unsigned char *, (___bytes), A2), \
	AROS_LCA(int, (___len), D1), \
	struct Library *, AMISSL_BASE_NAME, 1906, Amissl)

#define X509_NAME_ENTRY_create_by_NID(___ne, ___nid, ___type, ___bytes, ___len) \
	AROS_LC5(X509_NAME_ENTRY *, X509_NAME_ENTRY_create_by_NID, \
	AROS_LCA(X509_NAME_ENTRY **, (___ne), A0), \
	AROS_LCA(int, (___nid), D0), \
	AROS_LCA(int, (___type), D1), \
	AROS_LCA(unsigned char *, (___bytes), A1), \
	AROS_LCA(int, (___len), D2), \
	struct Library *, AMISSL_BASE_NAME, 1907, Amissl)

#define X509_NAME_add_entry_by_txt(___name, ___field, ___type, ___bytes, ___len, ___loc, ___set) \
	AROS_LC7(int, X509_NAME_add_entry_by_txt, \
	AROS_LCA(X509_NAME *, (___name), A0), \
	AROS_LCA(const char *, (___field), A1), \
	AROS_LCA(int, (___type), D0), \
	AROS_LCA(const unsigned char *, (___bytes), A2), \
	AROS_LCA(int, (___len), D1), \
	AROS_LCA(int, (___loc), D2), \
	AROS_LCA(int, (___set), D3), \
	struct Library *, AMISSL_BASE_NAME, 1908, Amissl)

#define X509_NAME_ENTRY_create_by_OBJ(___ne, ___obj, ___type, ___bytes, ___len) \
	AROS_LC5(X509_NAME_ENTRY *, X509_NAME_ENTRY_create_by_OBJ, \
	AROS_LCA(X509_NAME_ENTRY **, (___ne), A0), \
	AROS_LCA(ASN1_OBJECT *, (___obj), A1), \
	AROS_LCA(int, (___type), D0), \
	AROS_LCA(const unsigned char *, (___bytes), A2), \
	AROS_LCA(int, (___len), D1), \
	struct Library *, AMISSL_BASE_NAME, 1909, Amissl)

#define X509_NAME_ENTRY_set_object(___ne, ___obj) \
	AROS_LC2(int, X509_NAME_ENTRY_set_object, \
	AROS_LCA(X509_NAME_ENTRY *, (___ne), A0), \
	AROS_LCA(ASN1_OBJECT *, (___obj), A1), \
	struct Library *, AMISSL_BASE_NAME, 1910, Amissl)

#define X509_NAME_ENTRY_set_data(___ne, ___type, ___bytes, ___len) \
	AROS_LC4(int, X509_NAME_ENTRY_set_data, \
	AROS_LCA(X509_NAME_ENTRY *, (___ne), A0), \
	AROS_LCA(int, (___type), D0), \
	AROS_LCA(const unsigned char *, (___bytes), A1), \
	AROS_LCA(int, (___len), D1), \
	struct Library *, AMISSL_BASE_NAME, 1911, Amissl)

#define X509_NAME_ENTRY_get_object(___ne) \
	AROS_LC1(ASN1_OBJECT *, X509_NAME_ENTRY_get_object, \
	AROS_LCA(X509_NAME_ENTRY *, (___ne), A0), \
	struct Library *, AMISSL_BASE_NAME, 1912, Amissl)

#define X509_NAME_ENTRY_get_data(___ne) \
	AROS_LC1(ASN1_STRING *, X509_NAME_ENTRY_get_data, \
	AROS_LCA(X509_NAME_ENTRY *, (___ne), A0), \
	struct Library *, AMISSL_BASE_NAME, 1913, Amissl)

#define X509v3_get_ext_count(___x) \
	AROS_LC1(int, X509v3_get_ext_count, \
	AROS_LCA(const STACK_OF(X509_EXTENSION) *, (___x), A0), \
	struct Library *, AMISSL_BASE_NAME, 1914, Amissl)

#define X509v3_get_ext_by_NID(___x, ___nid, ___lastpos) \
	AROS_LC3(int, X509v3_get_ext_by_NID, \
	AROS_LCA(const STACK_OF(X509_EXTENSION) *, (___x), A0), \
	AROS_LCA(int, (___nid), D0), \
	AROS_LCA(int, (___lastpos), D1), \
	struct Library *, AMISSL_BASE_NAME, 1915, Amissl)

#define X509v3_get_ext_by_OBJ(___x, ___obj, ___lastpos) \
	AROS_LC3(int, X509v3_get_ext_by_OBJ, \
	AROS_LCA(const STACK_OF(X509_EXTENSION) *, (___x), A0), \
	AROS_LCA(ASN1_OBJECT *, (___obj), A1), \
	AROS_LCA(int, (___lastpos), D0), \
	struct Library *, AMISSL_BASE_NAME, 1916, Amissl)

#define X509v3_get_ext_by_critical(___x, ___crit, ___lastpos) \
	AROS_LC3(int, X509v3_get_ext_by_critical, \
	AROS_LCA(const STACK_OF(X509_EXTENSION) *, (___x), A0), \
	AROS_LCA(int, (___crit), D0), \
	AROS_LCA(int, (___lastpos), D1), \
	struct Library *, AMISSL_BASE_NAME, 1917, Amissl)

#define X509v3_get_ext(___x, ___loc) \
	AROS_LC2(X509_EXTENSION *, X509v3_get_ext, \
	AROS_LCA(const STACK_OF(X509_EXTENSION) *, (___x), A0), \
	AROS_LCA(int, (___loc), D0), \
	struct Library *, AMISSL_BASE_NAME, 1918, Amissl)

#define X509v3_delete_ext(___x, ___loc) \
	AROS_LC2(X509_EXTENSION *, X509v3_delete_ext, \
	AROS_LCA(STACK_OF(X509_EXTENSION) *, (___x), A0), \
	AROS_LCA(int, (___loc), D0), \
	struct Library *, AMISSL_BASE_NAME, 1919, Amissl)

#define X509v3_add_ext(___x, ___ex, ___loc) \
	AROS_LC3(STACK_OF(X509_EXTENSION) *, X509v3_add_ext, \
	AROS_LCA(STACK_OF(X509_EXTENSION) **, (___x), A0), \
	AROS_LCA(X509_EXTENSION *, (___ex), A1), \
	AROS_LCA(int, (___loc), D0), \
	struct Library *, AMISSL_BASE_NAME, 1920, Amissl)

#define X509_get_ext_count(___x) \
	AROS_LC1(int, X509_get_ext_count, \
	AROS_LCA(X509 *, (___x), A0), \
	struct Library *, AMISSL_BASE_NAME, 1921, Amissl)

#define X509_get_ext_by_NID(___x, ___nid, ___lastpos) \
	AROS_LC3(int, X509_get_ext_by_NID, \
	AROS_LCA(X509 *, (___x), A0), \
	AROS_LCA(int, (___nid), D0), \
	AROS_LCA(int, (___lastpos), D1), \
	struct Library *, AMISSL_BASE_NAME, 1922, Amissl)

#define X509_get_ext_by_OBJ(___x, ___obj, ___lastpos) \
	AROS_LC3(int, X509_get_ext_by_OBJ, \
	AROS_LCA(X509 *, (___x), A0), \
	AROS_LCA(ASN1_OBJECT *, (___obj), A1), \
	AROS_LCA(int, (___lastpos), D0), \
	struct Library *, AMISSL_BASE_NAME, 1923, Amissl)

#define X509_get_ext_by_critical(___x, ___crit, ___lastpos) \
	AROS_LC3(int, X509_get_ext_by_critical, \
	AROS_LCA(X509 *, (___x), A0), \
	AROS_LCA(int, (___crit), D0), \
	AROS_LCA(int, (___lastpos), D1), \
	struct Library *, AMISSL_BASE_NAME, 1924, Amissl)

#define X509_get_ext(___x, ___loc) \
	AROS_LC2(X509_EXTENSION *, X509_get_ext, \
	AROS_LCA(X509 *, (___x), A0), \
	AROS_LCA(int, (___loc), D0), \
	struct Library *, AMISSL_BASE_NAME, 1925, Amissl)

#define X509_delete_ext(___x, ___loc) \
	AROS_LC2(X509_EXTENSION *, X509_delete_ext, \
	AROS_LCA(X509 *, (___x), A0), \
	AROS_LCA(int, (___loc), D0), \
	struct Library *, AMISSL_BASE_NAME, 1926, Amissl)

#define X509_add_ext(___x, ___ex, ___loc) \
	AROS_LC3(int, X509_add_ext, \
	AROS_LCA(X509 *, (___x), A0), \
	AROS_LCA(X509_EXTENSION *, (___ex), A1), \
	AROS_LCA(int, (___loc), D0), \
	struct Library *, AMISSL_BASE_NAME, 1927, Amissl)

#define X509_get_ext_d2i(___x, ___nid, ___crit, ___idx) \
	AROS_LC4(void *, X509_get_ext_d2i, \
	AROS_LCA(X509 *, (___x), A0), \
	AROS_LCA(int, (___nid), D0), \
	AROS_LCA(int *, (___crit), A1), \
	AROS_LCA(int *, (___idx), A2), \
	struct Library *, AMISSL_BASE_NAME, 1928, Amissl)

#define X509_add1_ext_i2d(___x, ___nid, ___value, ___crit, ___flags) \
	AROS_LC5(int, X509_add1_ext_i2d, \
	AROS_LCA(X509 *, (___x), A0), \
	AROS_LCA(int, (___nid), D0), \
	AROS_LCA(void *, (___value), A1), \
	AROS_LCA(int, (___crit), D1), \
	AROS_LCA(unsigned long, (___flags), D2), \
	struct Library *, AMISSL_BASE_NAME, 1929, Amissl)

#define X509_CRL_get_ext_count(___x) \
	AROS_LC1(int, X509_CRL_get_ext_count, \
	AROS_LCA(X509_CRL *, (___x), A0), \
	struct Library *, AMISSL_BASE_NAME, 1930, Amissl)

#define X509_CRL_get_ext_by_NID(___x, ___nid, ___lastpos) \
	AROS_LC3(int, X509_CRL_get_ext_by_NID, \
	AROS_LCA(X509_CRL *, (___x), A0), \
	AROS_LCA(int, (___nid), D0), \
	AROS_LCA(int, (___lastpos), D1), \
	struct Library *, AMISSL_BASE_NAME, 1931, Amissl)

#define X509_CRL_get_ext_by_OBJ(___x, ___obj, ___lastpos) \
	AROS_LC3(int, X509_CRL_get_ext_by_OBJ, \
	AROS_LCA(X509_CRL *, (___x), A0), \
	AROS_LCA(ASN1_OBJECT *, (___obj), A1), \
	AROS_LCA(int, (___lastpos), D0), \
	struct Library *, AMISSL_BASE_NAME, 1932, Amissl)

#define X509_CRL_get_ext_by_critical(___x, ___crit, ___lastpos) \
	AROS_LC3(int, X509_CRL_get_ext_by_critical, \
	AROS_LCA(X509_CRL *, (___x), A0), \
	AROS_LCA(int, (___crit), D0), \
	AROS_LCA(int, (___lastpos), D1), \
	struct Library *, AMISSL_BASE_NAME, 1933, Amissl)

#define X509_CRL_get_ext(___x, ___loc) \
	AROS_LC2(X509_EXTENSION *, X509_CRL_get_ext, \
	AROS_LCA(X509_CRL *, (___x), A0), \
	AROS_LCA(int, (___loc), D0), \
	struct Library *, AMISSL_BASE_NAME, 1934, Amissl)

#define X509_CRL_delete_ext(___x, ___loc) \
	AROS_LC2(X509_EXTENSION *, X509_CRL_delete_ext, \
	AROS_LCA(X509_CRL *, (___x), A0), \
	AROS_LCA(int, (___loc), D0), \
	struct Library *, AMISSL_BASE_NAME, 1935, Amissl)

#define X509_CRL_add_ext(___x, ___ex, ___loc) \
	AROS_LC3(int, X509_CRL_add_ext, \
	AROS_LCA(X509_CRL *, (___x), A0), \
	AROS_LCA(X509_EXTENSION *, (___ex), A1), \
	AROS_LCA(int, (___loc), D0), \
	struct Library *, AMISSL_BASE_NAME, 1936, Amissl)

#define X509_CRL_get_ext_d2i(___x, ___nid, ___crit, ___idx) \
	AROS_LC4(void *, X509_CRL_get_ext_d2i, \
	AROS_LCA(X509_CRL *, (___x), A0), \
	AROS_LCA(int, (___nid), D0), \
	AROS_LCA(int *, (___crit), A1), \
	AROS_LCA(int *, (___idx), A2), \
	struct Library *, AMISSL_BASE_NAME, 1937, Amissl)

#define X509_CRL_add1_ext_i2d(___x, ___nid, ___value, ___crit, ___flags) \
	AROS_LC5(int, X509_CRL_add1_ext_i2d, \
	AROS_LCA(X509_CRL *, (___x), A0), \
	AROS_LCA(int, (___nid), D0), \
	AROS_LCA(void *, (___value), A1), \
	AROS_LCA(int, (___crit), D1), \
	AROS_LCA(unsigned long, (___flags), D2), \
	struct Library *, AMISSL_BASE_NAME, 1938, Amissl)

#define X509_REVOKED_get_ext_count(___x) \
	AROS_LC1(int, X509_REVOKED_get_ext_count, \
	AROS_LCA(X509_REVOKED *, (___x), A0), \
	struct Library *, AMISSL_BASE_NAME, 1939, Amissl)

#define X509_REVOKED_get_ext_by_NID(___x, ___nid, ___lastpos) \
	AROS_LC3(int, X509_REVOKED_get_ext_by_NID, \
	AROS_LCA(X509_REVOKED *, (___x), A0), \
	AROS_LCA(int, (___nid), D0), \
	AROS_LCA(int, (___lastpos), D1), \
	struct Library *, AMISSL_BASE_NAME, 1940, Amissl)

#define X509_REVOKED_get_ext_by_OBJ(___x, ___obj, ___lastpos) \
	AROS_LC3(int, X509_REVOKED_get_ext_by_OBJ, \
	AROS_LCA(X509_REVOKED *, (___x), A0), \
	AROS_LCA(ASN1_OBJECT *, (___obj), A1), \
	AROS_LCA(int, (___lastpos), D0), \
	struct Library *, AMISSL_BASE_NAME, 1941, Amissl)

#define X509_REVOKED_get_ext_by_critical(___x, ___crit, ___lastpos) \
	AROS_LC3(int, X509_REVOKED_get_ext_by_critical, \
	AROS_LCA(X509_REVOKED *, (___x), A0), \
	AROS_LCA(int, (___crit), D0), \
	AROS_LCA(int, (___lastpos), D1), \
	struct Library *, AMISSL_BASE_NAME, 1942, Amissl)

#define X509_REVOKED_get_ext(___x, ___loc) \
	AROS_LC2(X509_EXTENSION *, X509_REVOKED_get_ext, \
	AROS_LCA(X509_REVOKED *, (___x), A0), \
	AROS_LCA(int, (___loc), D0), \
	struct Library *, AMISSL_BASE_NAME, 1943, Amissl)

#define X509_REVOKED_delete_ext(___x, ___loc) \
	AROS_LC2(X509_EXTENSION *, X509_REVOKED_delete_ext, \
	AROS_LCA(X509_REVOKED *, (___x), A0), \
	AROS_LCA(int, (___loc), D0), \
	struct Library *, AMISSL_BASE_NAME, 1944, Amissl)

#define X509_REVOKED_add_ext(___x, ___ex, ___loc) \
	AROS_LC3(int, X509_REVOKED_add_ext, \
	AROS_LCA(X509_REVOKED *, (___x), A0), \
	AROS_LCA(X509_EXTENSION *, (___ex), A1), \
	AROS_LCA(int, (___loc), D0), \
	struct Library *, AMISSL_BASE_NAME, 1945, Amissl)

#define X509_REVOKED_get_ext_d2i(___x, ___nid, ___crit, ___idx) \
	AROS_LC4(void *, X509_REVOKED_get_ext_d2i, \
	AROS_LCA(X509_REVOKED *, (___x), A0), \
	AROS_LCA(int, (___nid), D0), \
	AROS_LCA(int *, (___crit), A1), \
	AROS_LCA(int *, (___idx), A2), \
	struct Library *, AMISSL_BASE_NAME, 1946, Amissl)

#define X509_REVOKED_add1_ext_i2d(___x, ___nid, ___value, ___crit, ___flags) \
	AROS_LC5(int, X509_REVOKED_add1_ext_i2d, \
	AROS_LCA(X509_REVOKED *, (___x), A0), \
	AROS_LCA(int, (___nid), D0), \
	AROS_LCA(void *, (___value), A1), \
	AROS_LCA(int, (___crit), D1), \
	AROS_LCA(unsigned long, (___flags), D2), \
	struct Library *, AMISSL_BASE_NAME, 1947, Amissl)

#define X509_EXTENSION_create_by_NID(___ex, ___nid, ___crit, ___data) \
	AROS_LC4(X509_EXTENSION *, X509_EXTENSION_create_by_NID, \
	AROS_LCA(X509_EXTENSION **, (___ex), A0), \
	AROS_LCA(int, (___nid), D0), \
	AROS_LCA(int, (___crit), D1), \
	AROS_LCA(ASN1_OCTET_STRING *, (___data), A1), \
	struct Library *, AMISSL_BASE_NAME, 1948, Amissl)

#define X509_EXTENSION_create_by_OBJ(___ex, ___obj, ___crit, ___data) \
	AROS_LC4(X509_EXTENSION *, X509_EXTENSION_create_by_OBJ, \
	AROS_LCA(X509_EXTENSION **, (___ex), A0), \
	AROS_LCA(ASN1_OBJECT *, (___obj), A1), \
	AROS_LCA(int, (___crit), D0), \
	AROS_LCA(ASN1_OCTET_STRING *, (___data), A2), \
	struct Library *, AMISSL_BASE_NAME, 1949, Amissl)

#define X509_EXTENSION_set_object(___ex, ___obj) \
	AROS_LC2(int, X509_EXTENSION_set_object, \
	AROS_LCA(X509_EXTENSION *, (___ex), A0), \
	AROS_LCA(ASN1_OBJECT *, (___obj), A1), \
	struct Library *, AMISSL_BASE_NAME, 1950, Amissl)

#define X509_EXTENSION_set_critical(___ex, ___crit) \
	AROS_LC2(int, X509_EXTENSION_set_critical, \
	AROS_LCA(X509_EXTENSION *, (___ex), A0), \
	AROS_LCA(int, (___crit), D0), \
	struct Library *, AMISSL_BASE_NAME, 1951, Amissl)

#define X509_EXTENSION_set_data(___ex, ___data) \
	AROS_LC2(int, X509_EXTENSION_set_data, \
	AROS_LCA(X509_EXTENSION *, (___ex), A0), \
	AROS_LCA(ASN1_OCTET_STRING *, (___data), A1), \
	struct Library *, AMISSL_BASE_NAME, 1952, Amissl)

#define X509_EXTENSION_get_object(___ex) \
	AROS_LC1(ASN1_OBJECT *, X509_EXTENSION_get_object, \
	AROS_LCA(X509_EXTENSION *, (___ex), A0), \
	struct Library *, AMISSL_BASE_NAME, 1953, Amissl)

#define X509_EXTENSION_get_data(___ne) \
	AROS_LC1(ASN1_OCTET_STRING *, X509_EXTENSION_get_data, \
	AROS_LCA(X509_EXTENSION *, (___ne), A0), \
	struct Library *, AMISSL_BASE_NAME, 1954, Amissl)

#define X509_EXTENSION_get_critical(___ex) \
	AROS_LC1(int, X509_EXTENSION_get_critical, \
	AROS_LCA(X509_EXTENSION *, (___ex), A0), \
	struct Library *, AMISSL_BASE_NAME, 1955, Amissl)

#define X509at_get_attr_count(___x) \
	AROS_LC1(int, X509at_get_attr_count, \
	AROS_LCA(const STACK_OF(X509_ATTRIBUTE) *, (___x), A0), \
	struct Library *, AMISSL_BASE_NAME, 1956, Amissl)

#define X509at_get_attr_by_NID(___x, ___nid, ___lastpos) \
	AROS_LC3(int, X509at_get_attr_by_NID, \
	AROS_LCA(const STACK_OF(X509_ATTRIBUTE) *, (___x), A0), \
	AROS_LCA(int, (___nid), D0), \
	AROS_LCA(int, (___lastpos), D1), \
	struct Library *, AMISSL_BASE_NAME, 1957, Amissl)

#define X509at_get_attr_by_OBJ(___sk, ___obj, ___lastpos) \
	AROS_LC3(int, X509at_get_attr_by_OBJ, \
	AROS_LCA(const STACK_OF(X509_ATTRIBUTE) *, (___sk), A0), \
	AROS_LCA(ASN1_OBJECT *, (___obj), A1), \
	AROS_LCA(int, (___lastpos), D0), \
	struct Library *, AMISSL_BASE_NAME, 1958, Amissl)

#define X509at_get_attr(___x, ___loc) \
	AROS_LC2(X509_ATTRIBUTE *, X509at_get_attr, \
	AROS_LCA(const STACK_OF(X509_ATTRIBUTE) *, (___x), A0), \
	AROS_LCA(int, (___loc), D0), \
	struct Library *, AMISSL_BASE_NAME, 1959, Amissl)

#define X509at_delete_attr(___x, ___loc) \
	AROS_LC2(X509_ATTRIBUTE *, X509at_delete_attr, \
	AROS_LCA(STACK_OF(X509_ATTRIBUTE) *, (___x), A0), \
	AROS_LCA(int, (___loc), D0), \
	struct Library *, AMISSL_BASE_NAME, 1960, Amissl)

#define X509at_add1_attr(___x, ___attr) \
	AROS_LC2(STACK_OF(X509_ATTRIBUTE) *, X509at_add1_attr, \
	AROS_LCA(STACK_OF(X509_ATTRIBUTE) **, (___x), A0), \
	AROS_LCA(X509_ATTRIBUTE *, (___attr), A1), \
	struct Library *, AMISSL_BASE_NAME, 1961, Amissl)

#define X509at_add1_attr_by_OBJ(___x, ___obj, ___type, ___bytes, ___len) \
	AROS_LC5(STACK_OF(X509_ATTRIBUTE) *, X509at_add1_attr_by_OBJ, \
	AROS_LCA(STACK_OF(X509_ATTRIBUTE) **, (___x), A0), \
	AROS_LCA(const ASN1_OBJECT *, (___obj), A1), \
	AROS_LCA(int, (___type), D0), \
	AROS_LCA(const unsigned char *, (___bytes), A2), \
	AROS_LCA(int, (___len), D1), \
	struct Library *, AMISSL_BASE_NAME, 1962, Amissl)

#define X509at_add1_attr_by_NID(___x, ___nid, ___type, ___bytes, ___len) \
	AROS_LC5(STACK_OF(X509_ATTRIBUTE) *, X509at_add1_attr_by_NID, \
	AROS_LCA(STACK_OF(X509_ATTRIBUTE) **, (___x), A0), \
	AROS_LCA(int, (___nid), D0), \
	AROS_LCA(int, (___type), D1), \
	AROS_LCA(const unsigned char *, (___bytes), A1), \
	AROS_LCA(int, (___len), D2), \
	struct Library *, AMISSL_BASE_NAME, 1963, Amissl)

#define X509at_add1_attr_by_txt(___x, ___attrname, ___type, ___bytes, ___len) \
	AROS_LC5(STACK_OF(X509_ATTRIBUTE) *, X509at_add1_attr_by_txt, \
	AROS_LCA(STACK_OF(X509_ATTRIBUTE) **, (___x), A0), \
	AROS_LCA(const char *, (___attrname), A1), \
	AROS_LCA(int, (___type), D0), \
	AROS_LCA(const unsigned char *, (___bytes), A2), \
	AROS_LCA(int, (___len), D1), \
	struct Library *, AMISSL_BASE_NAME, 1964, Amissl)

#define X509_ATTRIBUTE_create_by_NID(___attr, ___nid, ___atrtype, ___data, ___len) \
	AROS_LC5(X509_ATTRIBUTE *, X509_ATTRIBUTE_create_by_NID, \
	AROS_LCA(X509_ATTRIBUTE **, (___attr), A0), \
	AROS_LCA(int, (___nid), D0), \
	AROS_LCA(int, (___atrtype), D1), \
	AROS_LCA(const void *, (___data), A1), \
	AROS_LCA(int, (___len), D2), \
	struct Library *, AMISSL_BASE_NAME, 1965, Amissl)

#define X509_ATTRIBUTE_create_by_OBJ(___attr, ___obj, ___atrtype, ___data, ___len) \
	AROS_LC5(X509_ATTRIBUTE *, X509_ATTRIBUTE_create_by_OBJ, \
	AROS_LCA(X509_ATTRIBUTE **, (___attr), A0), \
	AROS_LCA(const ASN1_OBJECT *, (___obj), A1), \
	AROS_LCA(int, (___atrtype), D0), \
	AROS_LCA(const void *, (___data), A2), \
	AROS_LCA(int, (___len), D1), \
	struct Library *, AMISSL_BASE_NAME, 1966, Amissl)

#define X509_ATTRIBUTE_create_by_txt(___attr, ___atrname, ___type, ___bytes, ___len) \
	AROS_LC5(X509_ATTRIBUTE *, X509_ATTRIBUTE_create_by_txt, \
	AROS_LCA(X509_ATTRIBUTE **, (___attr), A0), \
	AROS_LCA(const char *, (___atrname), A1), \
	AROS_LCA(int, (___type), D0), \
	AROS_LCA(const unsigned char *, (___bytes), A2), \
	AROS_LCA(int, (___len), D1), \
	struct Library *, AMISSL_BASE_NAME, 1967, Amissl)

#define X509_ATTRIBUTE_set1_object(___attr, ___obj) \
	AROS_LC2(int, X509_ATTRIBUTE_set1_object, \
	AROS_LCA(X509_ATTRIBUTE *, (___attr), A0), \
	AROS_LCA(const ASN1_OBJECT *, (___obj), A1), \
	struct Library *, AMISSL_BASE_NAME, 1968, Amissl)

#define X509_ATTRIBUTE_set1_data(___attr, ___attrtype, ___data, ___len) \
	AROS_LC4(int, X509_ATTRIBUTE_set1_data, \
	AROS_LCA(X509_ATTRIBUTE *, (___attr), A0), \
	AROS_LCA(int, (___attrtype), D0), \
	AROS_LCA(const void *, (___data), A1), \
	AROS_LCA(int, (___len), D1), \
	struct Library *, AMISSL_BASE_NAME, 1969, Amissl)

#define X509_ATTRIBUTE_get0_data(___attr, ___idx, ___atrtype, ___data) \
	AROS_LC4(void *, X509_ATTRIBUTE_get0_data, \
	AROS_LCA(X509_ATTRIBUTE *, (___attr), A0), \
	AROS_LCA(int, (___idx), D0), \
	AROS_LCA(int, (___atrtype), D1), \
	AROS_LCA(void *, (___data), A1), \
	struct Library *, AMISSL_BASE_NAME, 1970, Amissl)

#define X509_ATTRIBUTE_count(___attr) \
	AROS_LC1(int, X509_ATTRIBUTE_count, \
	AROS_LCA(X509_ATTRIBUTE *, (___attr), A0), \
	struct Library *, AMISSL_BASE_NAME, 1971, Amissl)

#define X509_ATTRIBUTE_get0_object(___attr) \
	AROS_LC1(ASN1_OBJECT *, X509_ATTRIBUTE_get0_object, \
	AROS_LCA(X509_ATTRIBUTE *, (___attr), A0), \
	struct Library *, AMISSL_BASE_NAME, 1972, Amissl)

#define X509_ATTRIBUTE_get0_type(___attr, ___idx) \
	AROS_LC2(ASN1_TYPE *, X509_ATTRIBUTE_get0_type, \
	AROS_LCA(X509_ATTRIBUTE *, (___attr), A0), \
	AROS_LCA(int, (___idx), D0), \
	struct Library *, AMISSL_BASE_NAME, 1973, Amissl)

#define X509_verify_cert(___ctx) \
	AROS_LC1(int, X509_verify_cert, \
	AROS_LCA(X509_STORE_CTX *, (___ctx), A0), \
	struct Library *, AMISSL_BASE_NAME, 1974, Amissl)

#define X509_find_by_issuer_and_serial(___sk, ___name, ___serial) \
	AROS_LC3(X509 *, X509_find_by_issuer_and_serial, \
	AROS_LCA(STACK_OF(X509) *, (___sk), A0), \
	AROS_LCA(X509_NAME *, (___name), A1), \
	AROS_LCA(ASN1_INTEGER *, (___serial), A2), \
	struct Library *, AMISSL_BASE_NAME, 1975, Amissl)

#define X509_find_by_subject(___sk, ___name) \
	AROS_LC2(X509 *, X509_find_by_subject, \
	AROS_LCA(STACK_OF(X509) *, (___sk), A0), \
	AROS_LCA(X509_NAME *, (___name), A1), \
	struct Library *, AMISSL_BASE_NAME, 1976, Amissl)

#define PBEPARAM_new() \
	AROS_LC0(PBEPARAM *, PBEPARAM_new, \
	struct Library *, AMISSL_BASE_NAME, 1977, Amissl)

#define PBEPARAM_free(___a) \
	AROS_LC1(void, PBEPARAM_free, \
	AROS_LCA(PBEPARAM *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 1978, Amissl)

#define d2i_PBEPARAM(___a, ___in, ___len) \
	AROS_LC3(PBEPARAM *, d2i_PBEPARAM, \
	AROS_LCA(PBEPARAM **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 1979, Amissl)

#define i2d_PBEPARAM(___a, ___out) \
	AROS_LC2(int, i2d_PBEPARAM, \
	AROS_LCA(PBEPARAM *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 1980, Amissl)

#define PBEPARAM_it() \
	AROS_LC0(const ASN1_ITEM *, PBEPARAM_it, \
	struct Library *, AMISSL_BASE_NAME, 1981, Amissl)

#define PBE2PARAM_new() \
	AROS_LC0(PBE2PARAM *, PBE2PARAM_new, \
	struct Library *, AMISSL_BASE_NAME, 1982, Amissl)

#define PBE2PARAM_free(___a) \
	AROS_LC1(void, PBE2PARAM_free, \
	AROS_LCA(PBE2PARAM *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 1983, Amissl)

#define d2i_PBE2PARAM(___a, ___in, ___len) \
	AROS_LC3(PBE2PARAM *, d2i_PBE2PARAM, \
	AROS_LCA(PBE2PARAM **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 1984, Amissl)

#define i2d_PBE2PARAM(___a, ___out) \
	AROS_LC2(int, i2d_PBE2PARAM, \
	AROS_LCA(PBE2PARAM *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 1985, Amissl)

#define PBE2PARAM_it() \
	AROS_LC0(const ASN1_ITEM *, PBE2PARAM_it, \
	struct Library *, AMISSL_BASE_NAME, 1986, Amissl)

#define PBKDF2PARAM_new() \
	AROS_LC0(PBKDF2PARAM *, PBKDF2PARAM_new, \
	struct Library *, AMISSL_BASE_NAME, 1987, Amissl)

#define PBKDF2PARAM_free(___a) \
	AROS_LC1(void, PBKDF2PARAM_free, \
	AROS_LCA(PBKDF2PARAM *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 1988, Amissl)

#define d2i_PBKDF2PARAM(___a, ___in, ___len) \
	AROS_LC3(PBKDF2PARAM *, d2i_PBKDF2PARAM, \
	AROS_LCA(PBKDF2PARAM **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 1989, Amissl)

#define i2d_PBKDF2PARAM(___a, ___out) \
	AROS_LC2(int, i2d_PBKDF2PARAM, \
	AROS_LCA(PBKDF2PARAM *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 1990, Amissl)

#define PBKDF2PARAM_it() \
	AROS_LC0(const ASN1_ITEM *, PBKDF2PARAM_it, \
	struct Library *, AMISSL_BASE_NAME, 1991, Amissl)

#define PKCS5_pbe_set(___alg, ___iter, ___salt, ___saltlen) \
	AROS_LC4(X509_ALGOR *, PKCS5_pbe_set, \
	AROS_LCA(int, (___alg), D0), \
	AROS_LCA(int, (___iter), D1), \
	AROS_LCA(unsigned char *, (___salt), A0), \
	AROS_LCA(int, (___saltlen), D2), \
	struct Library *, AMISSL_BASE_NAME, 1992, Amissl)

#define PKCS5_pbe2_set(___cipher, ___iter, ___salt, ___saltlen) \
	AROS_LC4(X509_ALGOR *, PKCS5_pbe2_set, \
	AROS_LCA(const EVP_CIPHER *, (___cipher), A0), \
	AROS_LCA(int, (___iter), D0), \
	AROS_LCA(unsigned char *, (___salt), A1), \
	AROS_LCA(int, (___saltlen), D1), \
	struct Library *, AMISSL_BASE_NAME, 1993, Amissl)

#define PKCS8_PRIV_KEY_INFO_new() \
	AROS_LC0(PKCS8_PRIV_KEY_INFO *, PKCS8_PRIV_KEY_INFO_new, \
	struct Library *, AMISSL_BASE_NAME, 1994, Amissl)

#define PKCS8_PRIV_KEY_INFO_free(___a) \
	AROS_LC1(void, PKCS8_PRIV_KEY_INFO_free, \
	AROS_LCA(PKCS8_PRIV_KEY_INFO *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 1995, Amissl)

#define d2i_PKCS8_PRIV_KEY_INFO(___a, ___in, ___len) \
	AROS_LC3(PKCS8_PRIV_KEY_INFO *, d2i_PKCS8_PRIV_KEY_INFO, \
	AROS_LCA(PKCS8_PRIV_KEY_INFO **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 1996, Amissl)

#define i2d_PKCS8_PRIV_KEY_INFO(___a, ___out) \
	AROS_LC2(int, i2d_PKCS8_PRIV_KEY_INFO, \
	AROS_LCA(PKCS8_PRIV_KEY_INFO *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 1997, Amissl)

#define PKCS8_PRIV_KEY_INFO_it() \
	AROS_LC0(const ASN1_ITEM *, PKCS8_PRIV_KEY_INFO_it, \
	struct Library *, AMISSL_BASE_NAME, 1998, Amissl)

#define EVP_PKCS82PKEY(___p8) \
	AROS_LC1(EVP_PKEY *, EVP_PKCS82PKEY, \
	AROS_LCA(PKCS8_PRIV_KEY_INFO *, (___p8), A0), \
	struct Library *, AMISSL_BASE_NAME, 1999, Amissl)

#define EVP_PKEY2PKCS8(___pkey) \
	AROS_LC1(PKCS8_PRIV_KEY_INFO *, EVP_PKEY2PKCS8, \
	AROS_LCA(EVP_PKEY *, (___pkey), A0), \
	struct Library *, AMISSL_BASE_NAME, 2000, Amissl)

#define EVP_PKEY2PKCS8_broken(___pkey, ___broken) \
	AROS_LC2(PKCS8_PRIV_KEY_INFO *, EVP_PKEY2PKCS8_broken, \
	AROS_LCA(EVP_PKEY *, (___pkey), A0), \
	AROS_LCA(int, (___broken), D0), \
	struct Library *, AMISSL_BASE_NAME, 2001, Amissl)

#define PKCS8_set_broken(___p8, ___broken) \
	AROS_LC2(PKCS8_PRIV_KEY_INFO *, PKCS8_set_broken, \
	AROS_LCA(PKCS8_PRIV_KEY_INFO *, (___p8), A0), \
	AROS_LCA(int, (___broken), D0), \
	struct Library *, AMISSL_BASE_NAME, 2002, Amissl)

#define X509_check_trust(___x, ___id, ___flags) \
	AROS_LC3(int, X509_check_trust, \
	AROS_LCA(X509 *, (___x), A0), \
	AROS_LCA(int, (___id), D0), \
	AROS_LCA(int, (___flags), D1), \
	struct Library *, AMISSL_BASE_NAME, 2003, Amissl)

#define X509_TRUST_get_count() \
	AROS_LC0(int, X509_TRUST_get_count, \
	struct Library *, AMISSL_BASE_NAME, 2004, Amissl)

#define X509_TRUST_get0(___idx) \
	AROS_LC1(X509_TRUST *, X509_TRUST_get0, \
	AROS_LCA(int, (___idx), D0), \
	struct Library *, AMISSL_BASE_NAME, 2005, Amissl)

#define X509_TRUST_get_by_id(___id) \
	AROS_LC1(int, X509_TRUST_get_by_id, \
	AROS_LCA(int, (___id), D0), \
	struct Library *, AMISSL_BASE_NAME, 2006, Amissl)

#define X509_TRUST_add(___id, ___flags, ___ck, ___name, ___arg1, ___arg2) \
	AROS_LC6(int, X509_TRUST_add, \
	AROS_LCA(int, (___id), D0), \
	AROS_LCA(int, (___flags), D1), \
	AROS_LCA(int (*)(X509_TRUST *,X509 *,int), (___ck), A0), \
	AROS_LCA(char *, (___name), A1), \
	AROS_LCA(int, (___arg1), D2), \
	AROS_LCA(void *, (___arg2), A2), \
	struct Library *, AMISSL_BASE_NAME, 2007, Amissl)

#define X509_TRUST_cleanup() \
	AROS_LC0(void, X509_TRUST_cleanup, \
	struct Library *, AMISSL_BASE_NAME, 2008, Amissl)

#define X509_TRUST_get_flags(___xp) \
	AROS_LC1(int, X509_TRUST_get_flags, \
	AROS_LCA(X509_TRUST *, (___xp), A0), \
	struct Library *, AMISSL_BASE_NAME, 2009, Amissl)

#define X509_TRUST_get0_name(___xp) \
	AROS_LC1(char *, X509_TRUST_get0_name, \
	AROS_LCA(X509_TRUST *, (___xp), A0), \
	struct Library *, AMISSL_BASE_NAME, 2010, Amissl)

#define X509_TRUST_get_trust(___xp) \
	AROS_LC1(int, X509_TRUST_get_trust, \
	AROS_LCA(X509_TRUST *, (___xp), A0), \
	struct Library *, AMISSL_BASE_NAME, 2011, Amissl)

#define ERR_load_X509_strings() \
	AROS_LC0(void, ERR_load_X509_strings, \
	struct Library *, AMISSL_BASE_NAME, 2012, Amissl)

#define X509_OBJECT_idx_by_subject(___h, ___type, ___name) \
	AROS_LC3(int, X509_OBJECT_idx_by_subject, \
	AROS_LCA(STACK_OF(X509_OBJECT) *, (___h), A0), \
	AROS_LCA(int, (___type), D0), \
	AROS_LCA(X509_NAME *, (___name), A1), \
	struct Library *, AMISSL_BASE_NAME, 2013, Amissl)

#define X509_OBJECT_retrieve_by_subject(___h, ___type, ___name) \
	AROS_LC3(X509_OBJECT *, X509_OBJECT_retrieve_by_subject, \
	AROS_LCA(STACK_OF(X509_OBJECT) *, (___h), A0), \
	AROS_LCA(int, (___type), D0), \
	AROS_LCA(X509_NAME *, (___name), A1), \
	struct Library *, AMISSL_BASE_NAME, 2014, Amissl)

#define X509_OBJECT_retrieve_match(___h, ___x) \
	AROS_LC2(X509_OBJECT *, X509_OBJECT_retrieve_match, \
	AROS_LCA(STACK_OF(X509_OBJECT) *, (___h), A0), \
	AROS_LCA(X509_OBJECT *, (___x), A1), \
	struct Library *, AMISSL_BASE_NAME, 2015, Amissl)

#define X509_OBJECT_up_ref_count(___a) \
	AROS_LC1(void, X509_OBJECT_up_ref_count, \
	AROS_LCA(X509_OBJECT *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 2016, Amissl)

#define X509_OBJECT_free_contents(___a) \
	AROS_LC1(void, X509_OBJECT_free_contents, \
	AROS_LCA(X509_OBJECT *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 2017, Amissl)

#define X509_STORE_new() \
	AROS_LC0(X509_STORE *, X509_STORE_new, \
	struct Library *, AMISSL_BASE_NAME, 2018, Amissl)

#define X509_STORE_free(___v) \
	AROS_LC1(void, X509_STORE_free, \
	AROS_LCA(X509_STORE *, (___v), A0), \
	struct Library *, AMISSL_BASE_NAME, 2019, Amissl)

#define X509_STORE_set_flags(___ctx, ___flags) \
	AROS_LC2(void, X509_STORE_set_flags, \
	AROS_LCA(X509_STORE *, (___ctx), A0), \
	AROS_LCA(long, (___flags), D0), \
	struct Library *, AMISSL_BASE_NAME, 2020, Amissl)

#define X509_STORE_set_purpose(___ctx, ___purpose) \
	AROS_LC2(int, X509_STORE_set_purpose, \
	AROS_LCA(X509_STORE *, (___ctx), A0), \
	AROS_LCA(int, (___purpose), D0), \
	struct Library *, AMISSL_BASE_NAME, 2021, Amissl)

#define X509_STORE_set_trust(___ctx, ___trust) \
	AROS_LC2(int, X509_STORE_set_trust, \
	AROS_LCA(X509_STORE *, (___ctx), A0), \
	AROS_LCA(int, (___trust), D0), \
	struct Library *, AMISSL_BASE_NAME, 2022, Amissl)

#define X509_STORE_CTX_new() \
	AROS_LC0(X509_STORE_CTX *, X509_STORE_CTX_new, \
	struct Library *, AMISSL_BASE_NAME, 2023, Amissl)

#define X509_STORE_CTX_get1_issuer(___issuer, ___ctx, ___x) \
	AROS_LC3(int, X509_STORE_CTX_get1_issuer, \
	AROS_LCA(X509 **, (___issuer), A0), \
	AROS_LCA(X509_STORE_CTX *, (___ctx), A1), \
	AROS_LCA(X509 *, (___x), A2), \
	struct Library *, AMISSL_BASE_NAME, 2024, Amissl)

#define X509_STORE_CTX_free(___ctx) \
	AROS_LC1(void, X509_STORE_CTX_free, \
	AROS_LCA(X509_STORE_CTX *, (___ctx), A0), \
	struct Library *, AMISSL_BASE_NAME, 2025, Amissl)

#define X509_STORE_CTX_init(___ctx, ___store, ___x509, ___chain) \
	AROS_LC4(int, X509_STORE_CTX_init, \
	AROS_LCA(X509_STORE_CTX *, (___ctx), A0), \
	AROS_LCA(X509_STORE *, (___store), A1), \
	AROS_LCA(X509 *, (___x509), A2), \
	AROS_LCA(STACK_OF(X509) *, (___chain), A3), \
	struct Library *, AMISSL_BASE_NAME, 2026, Amissl)

#define X509_STORE_CTX_trusted_stack(___ctx, ___sk) \
	AROS_LC2(void, X509_STORE_CTX_trusted_stack, \
	AROS_LCA(X509_STORE_CTX *, (___ctx), A0), \
	AROS_LCA(STACK_OF(X509) *, (___sk), A1), \
	struct Library *, AMISSL_BASE_NAME, 2027, Amissl)

#define X509_STORE_CTX_cleanup(___ctx) \
	AROS_LC1(void, X509_STORE_CTX_cleanup, \
	AROS_LCA(X509_STORE_CTX *, (___ctx), A0), \
	struct Library *, AMISSL_BASE_NAME, 2028, Amissl)

#define X509_STORE_add_lookup(___v, ___m) \
	AROS_LC2(X509_LOOKUP *, X509_STORE_add_lookup, \
	AROS_LCA(X509_STORE *, (___v), A0), \
	AROS_LCA(X509_LOOKUP_METHOD *, (___m), A1), \
	struct Library *, AMISSL_BASE_NAME, 2029, Amissl)

#define X509_LOOKUP_hash_dir() \
	AROS_LC0(X509_LOOKUP_METHOD *, X509_LOOKUP_hash_dir, \
	struct Library *, AMISSL_BASE_NAME, 2030, Amissl)

#define X509_LOOKUP_file() \
	AROS_LC0(X509_LOOKUP_METHOD *, X509_LOOKUP_file, \
	struct Library *, AMISSL_BASE_NAME, 2031, Amissl)

#define X509_STORE_add_cert(___ctx, ___x) \
	AROS_LC2(int, X509_STORE_add_cert, \
	AROS_LCA(X509_STORE *, (___ctx), A0), \
	AROS_LCA(X509 *, (___x), A1), \
	struct Library *, AMISSL_BASE_NAME, 2032, Amissl)

#define X509_STORE_add_crl(___ctx, ___x) \
	AROS_LC2(int, X509_STORE_add_crl, \
	AROS_LCA(X509_STORE *, (___ctx), A0), \
	AROS_LCA(X509_CRL *, (___x), A1), \
	struct Library *, AMISSL_BASE_NAME, 2033, Amissl)

#define X509_STORE_get_by_subject(___vs, ___type, ___name, ___ret) \
	AROS_LC4(int, X509_STORE_get_by_subject, \
	AROS_LCA(X509_STORE_CTX *, (___vs), A0), \
	AROS_LCA(int, (___type), D0), \
	AROS_LCA(X509_NAME *, (___name), A1), \
	AROS_LCA(X509_OBJECT *, (___ret), A2), \
	struct Library *, AMISSL_BASE_NAME, 2034, Amissl)

#define X509_LOOKUP_ctrl(___ctx, ___cmd, ___argc, ___argl, ___ret) \
	AROS_LC5(int, X509_LOOKUP_ctrl, \
	AROS_LCA(X509_LOOKUP *, (___ctx), A0), \
	AROS_LCA(int, (___cmd), D0), \
	AROS_LCA(const char *, (___argc), A1), \
	AROS_LCA(long, (___argl), D1), \
	AROS_LCA(char **, (___ret), A2), \
	struct Library *, AMISSL_BASE_NAME, 2035, Amissl)

#define X509_load_cert_file(___ctx, ___file, ___type) \
	AROS_LC3(int, X509_load_cert_file, \
	AROS_LCA(X509_LOOKUP *, (___ctx), A0), \
	AROS_LCA(const char *, (___file), A1), \
	AROS_LCA(int, (___type), D0), \
	struct Library *, AMISSL_BASE_NAME, 2036, Amissl)

#define X509_load_crl_file(___ctx, ___file, ___type) \
	AROS_LC3(int, X509_load_crl_file, \
	AROS_LCA(X509_LOOKUP *, (___ctx), A0), \
	AROS_LCA(const char *, (___file), A1), \
	AROS_LCA(int, (___type), D0), \
	struct Library *, AMISSL_BASE_NAME, 2037, Amissl)

#define X509_load_cert_crl_file(___ctx, ___file, ___type) \
	AROS_LC3(int, X509_load_cert_crl_file, \
	AROS_LCA(X509_LOOKUP *, (___ctx), A0), \
	AROS_LCA(const char *, (___file), A1), \
	AROS_LCA(int, (___type), D0), \
	struct Library *, AMISSL_BASE_NAME, 2038, Amissl)

#define X509_LOOKUP_new(___method) \
	AROS_LC1(X509_LOOKUP *, X509_LOOKUP_new, \
	AROS_LCA(X509_LOOKUP_METHOD *, (___method), A0), \
	struct Library *, AMISSL_BASE_NAME, 2039, Amissl)

#define X509_LOOKUP_free(___ctx) \
	AROS_LC1(void, X509_LOOKUP_free, \
	AROS_LCA(X509_LOOKUP *, (___ctx), A0), \
	struct Library *, AMISSL_BASE_NAME, 2040, Amissl)

#define X509_LOOKUP_init(___ctx) \
	AROS_LC1(int, X509_LOOKUP_init, \
	AROS_LCA(X509_LOOKUP *, (___ctx), A0), \
	struct Library *, AMISSL_BASE_NAME, 2041, Amissl)

#define X509_LOOKUP_by_subject(___ctx, ___type, ___name, ___ret) \
	AROS_LC4(int, X509_LOOKUP_by_subject, \
	AROS_LCA(X509_LOOKUP *, (___ctx), A0), \
	AROS_LCA(int, (___type), D0), \
	AROS_LCA(X509_NAME *, (___name), A1), \
	AROS_LCA(X509_OBJECT *, (___ret), A2), \
	struct Library *, AMISSL_BASE_NAME, 2042, Amissl)

#define X509_LOOKUP_by_issuer_serial(___ctx, ___type, ___name, ___serial, ___ret) \
	AROS_LC5(int, X509_LOOKUP_by_issuer_serial, \
	AROS_LCA(X509_LOOKUP *, (___ctx), A0), \
	AROS_LCA(int, (___type), D0), \
	AROS_LCA(X509_NAME *, (___name), A1), \
	AROS_LCA(ASN1_INTEGER *, (___serial), A2), \
	AROS_LCA(X509_OBJECT *, (___ret), A3), \
	struct Library *, AMISSL_BASE_NAME, 2043, Amissl)

#define X509_LOOKUP_by_fingerprint(___ctx, ___type, ___bytes, ___len, ___ret) \
	AROS_LC5(int, X509_LOOKUP_by_fingerprint, \
	AROS_LCA(X509_LOOKUP *, (___ctx), A0), \
	AROS_LCA(int, (___type), D0), \
	AROS_LCA(unsigned char *, (___bytes), A1), \
	AROS_LCA(int, (___len), D1), \
	AROS_LCA(X509_OBJECT *, (___ret), A2), \
	struct Library *, AMISSL_BASE_NAME, 2044, Amissl)

#define X509_LOOKUP_by_alias(___ctx, ___type, ___str, ___len, ___ret) \
	AROS_LC5(int, X509_LOOKUP_by_alias, \
	AROS_LCA(X509_LOOKUP *, (___ctx), A0), \
	AROS_LCA(int, (___type), D0), \
	AROS_LCA(char *, (___str), A1), \
	AROS_LCA(int, (___len), D1), \
	AROS_LCA(X509_OBJECT *, (___ret), A2), \
	struct Library *, AMISSL_BASE_NAME, 2045, Amissl)

#define X509_LOOKUP_shutdown(___ctx) \
	AROS_LC1(int, X509_LOOKUP_shutdown, \
	AROS_LCA(X509_LOOKUP *, (___ctx), A0), \
	struct Library *, AMISSL_BASE_NAME, 2046, Amissl)

#define X509_STORE_load_locations(___ctx, ___file, ___dir) \
	AROS_LC3(int, X509_STORE_load_locations, \
	AROS_LCA(X509_STORE *, (___ctx), A0), \
	AROS_LCA(const char *, (___file), A1), \
	AROS_LCA(const char *, (___dir), A2), \
	struct Library *, AMISSL_BASE_NAME, 2047, Amissl)

#define X509_STORE_set_default_paths(___ctx) \
	AROS_LC1(int, X509_STORE_set_default_paths, \
	AROS_LCA(X509_STORE *, (___ctx), A0), \
	struct Library *, AMISSL_BASE_NAME, 2048, Amissl)

#define X509_STORE_CTX_get_ex_new_index(___argl, ___argp, ___new_func, ___dup_func, ___free_func) \
	AROS_LC5(int, X509_STORE_CTX_get_ex_new_index, \
	AROS_LCA(long, (___argl), D0), \
	AROS_LCA(void *, (___argp), A0), \
	AROS_LCA(CRYPTO_EX_new * (*)(), (___new_func), A1), \
	AROS_LCA(CRYPTO_EX_dup * (*)(), (___dup_func), A2), \
	AROS_LCA(CRYPTO_EX_free * (*)(), (___free_func), A3), \
	struct Library *, AMISSL_BASE_NAME, 2049, Amissl)

#define X509_STORE_CTX_set_ex_data(___ctx, ___idx, ___data) \
	AROS_LC3(int, X509_STORE_CTX_set_ex_data, \
	AROS_LCA(X509_STORE_CTX *, (___ctx), A0), \
	AROS_LCA(int, (___idx), D0), \
	AROS_LCA(void *, (___data), A1), \
	struct Library *, AMISSL_BASE_NAME, 2050, Amissl)

#define X509_STORE_CTX_get_ex_data(___ctx, ___idx) \
	AROS_LC2(void *, X509_STORE_CTX_get_ex_data, \
	AROS_LCA(X509_STORE_CTX *, (___ctx), A0), \
	AROS_LCA(int, (___idx), D0), \
	struct Library *, AMISSL_BASE_NAME, 2051, Amissl)

#define X509_STORE_CTX_get_error(___ctx) \
	AROS_LC1(int, X509_STORE_CTX_get_error, \
	AROS_LCA(X509_STORE_CTX *, (___ctx), A0), \
	struct Library *, AMISSL_BASE_NAME, 2052, Amissl)

#define X509_STORE_CTX_set_error(___ctx, ___s) \
	AROS_LC2(void, X509_STORE_CTX_set_error, \
	AROS_LCA(X509_STORE_CTX *, (___ctx), A0), \
	AROS_LCA(int, (___s), D0), \
	struct Library *, AMISSL_BASE_NAME, 2053, Amissl)

#define X509_STORE_CTX_get_error_depth(___ctx) \
	AROS_LC1(int, X509_STORE_CTX_get_error_depth, \
	AROS_LCA(X509_STORE_CTX *, (___ctx), A0), \
	struct Library *, AMISSL_BASE_NAME, 2054, Amissl)

#define X509_STORE_CTX_get_current_cert(___ctx) \
	AROS_LC1(X509 *, X509_STORE_CTX_get_current_cert, \
	AROS_LCA(X509_STORE_CTX *, (___ctx), A0), \
	struct Library *, AMISSL_BASE_NAME, 2055, Amissl)

#define X509_STORE_CTX_get_chain(___ctx) \
	AROS_LC1(STACK_OF(X509) *, X509_STORE_CTX_get_chain, \
	AROS_LCA(X509_STORE_CTX *, (___ctx), A0), \
	struct Library *, AMISSL_BASE_NAME, 2056, Amissl)

#define X509_STORE_CTX_get1_chain(___ctx) \
	AROS_LC1(STACK_OF(X509) *, X509_STORE_CTX_get1_chain, \
	AROS_LCA(X509_STORE_CTX *, (___ctx), A0), \
	struct Library *, AMISSL_BASE_NAME, 2057, Amissl)

#define X509_STORE_CTX_set_cert(___c, ___x) \
	AROS_LC2(void, X509_STORE_CTX_set_cert, \
	AROS_LCA(X509_STORE_CTX *, (___c), A0), \
	AROS_LCA(X509 *, (___x), A1), \
	struct Library *, AMISSL_BASE_NAME, 2058, Amissl)

#define X509_STORE_CTX_set_chain(___c, ___sk) \
	AROS_LC2(void, X509_STORE_CTX_set_chain, \
	AROS_LCA(X509_STORE_CTX *, (___c), A0), \
	AROS_LCA(STACK_OF(X509) *, (___sk), A1), \
	struct Library *, AMISSL_BASE_NAME, 2059, Amissl)

#define X509_STORE_CTX_set_purpose(___ctx, ___purpose) \
	AROS_LC2(int, X509_STORE_CTX_set_purpose, \
	AROS_LCA(X509_STORE_CTX *, (___ctx), A0), \
	AROS_LCA(int, (___purpose), D0), \
	struct Library *, AMISSL_BASE_NAME, 2060, Amissl)

#define X509_STORE_CTX_set_trust(___ctx, ___trust) \
	AROS_LC2(int, X509_STORE_CTX_set_trust, \
	AROS_LCA(X509_STORE_CTX *, (___ctx), A0), \
	AROS_LCA(int, (___trust), D0), \
	struct Library *, AMISSL_BASE_NAME, 2061, Amissl)

#define X509_STORE_CTX_purpose_inherit(___ctx, ___def_purpose, ___purpose, ___trust) \
	AROS_LC4(int, X509_STORE_CTX_purpose_inherit, \
	AROS_LCA(X509_STORE_CTX *, (___ctx), A0), \
	AROS_LCA(int, (___def_purpose), D0), \
	AROS_LCA(int, (___purpose), D1), \
	AROS_LCA(int, (___trust), D2), \
	struct Library *, AMISSL_BASE_NAME, 2062, Amissl)

#define X509_STORE_CTX_set_flags(___ctx, ___flags) \
	AROS_LC2(void, X509_STORE_CTX_set_flags, \
	AROS_LCA(X509_STORE_CTX *, (___ctx), A0), \
	AROS_LCA(long, (___flags), D0), \
	struct Library *, AMISSL_BASE_NAME, 2063, Amissl)

#define X509_STORE_CTX_set_time(___ctx, ___flags, ___t) \
	AROS_LC3(void, X509_STORE_CTX_set_time, \
	AROS_LCA(X509_STORE_CTX *, (___ctx), A0), \
	AROS_LCA(long, (___flags), D0), \
	AROS_LCA(time_t, (___t), D1), \
	struct Library *, AMISSL_BASE_NAME, 2064, Amissl)

#define X509_STORE_CTX_set_verify_cb(___ctx, ___verify_cb) \
	AROS_LC2(void, X509_STORE_CTX_set_verify_cb, \
	AROS_LCA(X509_STORE_CTX *, (___ctx), A0), \
	AROS_LCA(int (*)(int,X509_STORE_CTX *), (___verify_cb), A1), \
	struct Library *, AMISSL_BASE_NAME, 2065, Amissl)

#define BASIC_CONSTRAINTS_new() \
	AROS_LC0(BASIC_CONSTRAINTS *, BASIC_CONSTRAINTS_new, \
	struct Library *, AMISSL_BASE_NAME, 2066, Amissl)

#define BASIC_CONSTRAINTS_free(___a) \
	AROS_LC1(void, BASIC_CONSTRAINTS_free, \
	AROS_LCA(BASIC_CONSTRAINTS *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 2067, Amissl)

#define d2i_BASIC_CONSTRAINTS(___a, ___in, ___len) \
	AROS_LC3(BASIC_CONSTRAINTS *, d2i_BASIC_CONSTRAINTS, \
	AROS_LCA(BASIC_CONSTRAINTS **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 2068, Amissl)

#define i2d_BASIC_CONSTRAINTS(___a, ___out) \
	AROS_LC2(int, i2d_BASIC_CONSTRAINTS, \
	AROS_LCA(BASIC_CONSTRAINTS *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 2069, Amissl)

#define BASIC_CONSTRAINTS_it() \
	AROS_LC0(const ASN1_ITEM *, BASIC_CONSTRAINTS_it, \
	struct Library *, AMISSL_BASE_NAME, 2070, Amissl)

#define SXNET_new() \
	AROS_LC0(SXNET *, SXNET_new, \
	struct Library *, AMISSL_BASE_NAME, 2071, Amissl)

#define SXNET_free(___a) \
	AROS_LC1(void, SXNET_free, \
	AROS_LCA(SXNET *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 2072, Amissl)

#define d2i_SXNET(___a, ___in, ___len) \
	AROS_LC3(SXNET *, d2i_SXNET, \
	AROS_LCA(SXNET **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 2073, Amissl)

#define i2d_SXNET(___a, ___out) \
	AROS_LC2(int, i2d_SXNET, \
	AROS_LCA(SXNET *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 2074, Amissl)

#define SXNET_it() \
	AROS_LC0(const ASN1_ITEM *, SXNET_it, \
	struct Library *, AMISSL_BASE_NAME, 2075, Amissl)

#define SXNETID_new() \
	AROS_LC0(SXNETID *, SXNETID_new, \
	struct Library *, AMISSL_BASE_NAME, 2076, Amissl)

#define SXNETID_free(___a) \
	AROS_LC1(void, SXNETID_free, \
	AROS_LCA(SXNETID *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 2077, Amissl)

#define d2i_SXNETID(___a, ___in, ___len) \
	AROS_LC3(SXNETID *, d2i_SXNETID, \
	AROS_LCA(SXNETID **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 2078, Amissl)

#define i2d_SXNETID(___a, ___out) \
	AROS_LC2(int, i2d_SXNETID, \
	AROS_LCA(SXNETID *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 2079, Amissl)

#define SXNETID_it() \
	AROS_LC0(const ASN1_ITEM *, SXNETID_it, \
	struct Library *, AMISSL_BASE_NAME, 2080, Amissl)

#define SXNET_add_id_asc(___psx, ___zone, ___user, ___userlen) \
	AROS_LC4(int, SXNET_add_id_asc, \
	AROS_LCA(SXNET **, (___psx), A0), \
	AROS_LCA(char *, (___zone), A1), \
	AROS_LCA(char *, (___user), A2), \
	AROS_LCA(int, (___userlen), D0), \
	struct Library *, AMISSL_BASE_NAME, 2081, Amissl)

#define SXNET_add_id_ulong(___psx, ___lzone, ___user, ___userlen) \
	AROS_LC4(int, SXNET_add_id_ulong, \
	AROS_LCA(SXNET **, (___psx), A0), \
	AROS_LCA(unsigned long, (___lzone), D0), \
	AROS_LCA(char *, (___user), A1), \
	AROS_LCA(int, (___userlen), D1), \
	struct Library *, AMISSL_BASE_NAME, 2082, Amissl)

#define SXNET_add_id_INTEGER(___psx, ___izone, ___user, ___userlen) \
	AROS_LC4(int, SXNET_add_id_INTEGER, \
	AROS_LCA(SXNET **, (___psx), A0), \
	AROS_LCA(ASN1_INTEGER *, (___izone), A1), \
	AROS_LCA(char *, (___user), A2), \
	AROS_LCA(int, (___userlen), D0), \
	struct Library *, AMISSL_BASE_NAME, 2083, Amissl)

#define SXNET_get_id_asc(___sx, ___zone) \
	AROS_LC2(ASN1_OCTET_STRING *, SXNET_get_id_asc, \
	AROS_LCA(SXNET *, (___sx), A0), \
	AROS_LCA(char *, (___zone), A1), \
	struct Library *, AMISSL_BASE_NAME, 2084, Amissl)

#define SXNET_get_id_ulong(___sx, ___lzone) \
	AROS_LC2(ASN1_OCTET_STRING *, SXNET_get_id_ulong, \
	AROS_LCA(SXNET *, (___sx), A0), \
	AROS_LCA(unsigned long, (___lzone), D0), \
	struct Library *, AMISSL_BASE_NAME, 2085, Amissl)

#define SXNET_get_id_INTEGER(___sx, ___zone) \
	AROS_LC2(ASN1_OCTET_STRING *, SXNET_get_id_INTEGER, \
	AROS_LCA(SXNET *, (___sx), A0), \
	AROS_LCA(ASN1_INTEGER *, (___zone), A1), \
	struct Library *, AMISSL_BASE_NAME, 2086, Amissl)

#define AUTHORITY_KEYID_new() \
	AROS_LC0(AUTHORITY_KEYID *, AUTHORITY_KEYID_new, \
	struct Library *, AMISSL_BASE_NAME, 2087, Amissl)

#define AUTHORITY_KEYID_free(___a) \
	AROS_LC1(void, AUTHORITY_KEYID_free, \
	AROS_LCA(AUTHORITY_KEYID *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 2088, Amissl)

#define d2i_AUTHORITY_KEYID(___a, ___in, ___len) \
	AROS_LC3(AUTHORITY_KEYID *, d2i_AUTHORITY_KEYID, \
	AROS_LCA(AUTHORITY_KEYID **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 2089, Amissl)

#define i2d_AUTHORITY_KEYID(___a, ___out) \
	AROS_LC2(int, i2d_AUTHORITY_KEYID, \
	AROS_LCA(AUTHORITY_KEYID *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 2090, Amissl)

#define AUTHORITY_KEYID_it() \
	AROS_LC0(const ASN1_ITEM *, AUTHORITY_KEYID_it, \
	struct Library *, AMISSL_BASE_NAME, 2091, Amissl)

#define PKEY_USAGE_PERIOD_new() \
	AROS_LC0(PKEY_USAGE_PERIOD *, PKEY_USAGE_PERIOD_new, \
	struct Library *, AMISSL_BASE_NAME, 2092, Amissl)

#define PKEY_USAGE_PERIOD_free(___a) \
	AROS_LC1(void, PKEY_USAGE_PERIOD_free, \
	AROS_LCA(PKEY_USAGE_PERIOD *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 2093, Amissl)

#define d2i_PKEY_USAGE_PERIOD(___a, ___in, ___len) \
	AROS_LC3(PKEY_USAGE_PERIOD *, d2i_PKEY_USAGE_PERIOD, \
	AROS_LCA(PKEY_USAGE_PERIOD **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 2094, Amissl)

#define i2d_PKEY_USAGE_PERIOD(___a, ___out) \
	AROS_LC2(int, i2d_PKEY_USAGE_PERIOD, \
	AROS_LCA(PKEY_USAGE_PERIOD *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 2095, Amissl)

#define PKEY_USAGE_PERIOD_it() \
	AROS_LC0(const ASN1_ITEM *, PKEY_USAGE_PERIOD_it, \
	struct Library *, AMISSL_BASE_NAME, 2096, Amissl)

#define GENERAL_NAME_new() \
	AROS_LC0(GENERAL_NAME *, GENERAL_NAME_new, \
	struct Library *, AMISSL_BASE_NAME, 2097, Amissl)

#define GENERAL_NAME_free(___a) \
	AROS_LC1(void, GENERAL_NAME_free, \
	AROS_LCA(GENERAL_NAME *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 2098, Amissl)

#define d2i_GENERAL_NAME(___a, ___in, ___len) \
	AROS_LC3(GENERAL_NAME *, d2i_GENERAL_NAME, \
	AROS_LCA(GENERAL_NAME **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 2099, Amissl)

#define i2d_GENERAL_NAME(___a, ___out) \
	AROS_LC2(int, i2d_GENERAL_NAME, \
	AROS_LCA(GENERAL_NAME *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 2100, Amissl)

#define GENERAL_NAME_it() \
	AROS_LC0(const ASN1_ITEM *, GENERAL_NAME_it, \
	struct Library *, AMISSL_BASE_NAME, 2101, Amissl)

#define i2v_GENERAL_NAME(___method, ___gen, ___ret) \
	AROS_LC3(STACK_OF(CONF_VALUE) *, i2v_GENERAL_NAME, \
	AROS_LCA(X509V3_EXT_METHOD *, (___method), A0), \
	AROS_LCA(GENERAL_NAME *, (___gen), A1), \
	AROS_LCA(STACK_OF(CONF_VALUE) *, (___ret), A2), \
	struct Library *, AMISSL_BASE_NAME, 2102, Amissl)

#define GENERAL_NAME_print(___out, ___gen) \
	AROS_LC2(int, GENERAL_NAME_print, \
	AROS_LCA(BIO *, (___out), A0), \
	AROS_LCA(GENERAL_NAME *, (___gen), A1), \
	struct Library *, AMISSL_BASE_NAME, 2103, Amissl)

#define GENERAL_NAMES_new() \
	AROS_LC0(GENERAL_NAMES *, GENERAL_NAMES_new, \
	struct Library *, AMISSL_BASE_NAME, 2104, Amissl)

#define GENERAL_NAMES_free(___a) \
	AROS_LC1(void, GENERAL_NAMES_free, \
	AROS_LCA(GENERAL_NAMES *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 2105, Amissl)

#define d2i_GENERAL_NAMES(___a, ___in, ___len) \
	AROS_LC3(GENERAL_NAMES *, d2i_GENERAL_NAMES, \
	AROS_LCA(GENERAL_NAMES **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 2106, Amissl)

#define i2d_GENERAL_NAMES(___a, ___out) \
	AROS_LC2(int, i2d_GENERAL_NAMES, \
	AROS_LCA(GENERAL_NAMES *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 2107, Amissl)

#define GENERAL_NAMES_it() \
	AROS_LC0(const ASN1_ITEM *, GENERAL_NAMES_it, \
	struct Library *, AMISSL_BASE_NAME, 2108, Amissl)

#define i2v_GENERAL_NAMES(___method, ___gen, ___extlist) \
	AROS_LC3(STACK_OF(CONF_VALUE) *, i2v_GENERAL_NAMES, \
	AROS_LCA(X509V3_EXT_METHOD *, (___method), A0), \
	AROS_LCA(GENERAL_NAMES *, (___gen), A1), \
	AROS_LCA(STACK_OF(CONF_VALUE) *, (___extlist), A2), \
	struct Library *, AMISSL_BASE_NAME, 2109, Amissl)

#define v2i_GENERAL_NAMES(___method, ___ctx, ___nval) \
	AROS_LC3(GENERAL_NAMES *, v2i_GENERAL_NAMES, \
	AROS_LCA(X509V3_EXT_METHOD *, (___method), A0), \
	AROS_LCA(X509V3_CTX *, (___ctx), A1), \
	AROS_LCA(STACK_OF(CONF_VALUE) *, (___nval), A2), \
	struct Library *, AMISSL_BASE_NAME, 2110, Amissl)

#define OTHERNAME_new() \
	AROS_LC0(OTHERNAME *, OTHERNAME_new, \
	struct Library *, AMISSL_BASE_NAME, 2111, Amissl)

#define OTHERNAME_free(___a) \
	AROS_LC1(void, OTHERNAME_free, \
	AROS_LCA(OTHERNAME *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 2112, Amissl)

#define d2i_OTHERNAME(___a, ___in, ___len) \
	AROS_LC3(OTHERNAME *, d2i_OTHERNAME, \
	AROS_LCA(OTHERNAME **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 2113, Amissl)

#define i2d_OTHERNAME(___a, ___out) \
	AROS_LC2(int, i2d_OTHERNAME, \
	AROS_LCA(OTHERNAME *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 2114, Amissl)

#define OTHERNAME_it() \
	AROS_LC0(const ASN1_ITEM *, OTHERNAME_it, \
	struct Library *, AMISSL_BASE_NAME, 2115, Amissl)

#define EDIPARTYNAME_new() \
	AROS_LC0(EDIPARTYNAME *, EDIPARTYNAME_new, \
	struct Library *, AMISSL_BASE_NAME, 2116, Amissl)

#define EDIPARTYNAME_free(___a) \
	AROS_LC1(void, EDIPARTYNAME_free, \
	AROS_LCA(EDIPARTYNAME *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 2117, Amissl)

#define d2i_EDIPARTYNAME(___a, ___in, ___len) \
	AROS_LC3(EDIPARTYNAME *, d2i_EDIPARTYNAME, \
	AROS_LCA(EDIPARTYNAME **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 2118, Amissl)

#define i2d_EDIPARTYNAME(___a, ___out) \
	AROS_LC2(int, i2d_EDIPARTYNAME, \
	AROS_LCA(EDIPARTYNAME *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 2119, Amissl)

#define EDIPARTYNAME_it() \
	AROS_LC0(const ASN1_ITEM *, EDIPARTYNAME_it, \
	struct Library *, AMISSL_BASE_NAME, 2120, Amissl)

#define i2s_ASN1_OCTET_STRING(___method, ___ia5) \
	AROS_LC2(char *, i2s_ASN1_OCTET_STRING, \
	AROS_LCA(X509V3_EXT_METHOD *, (___method), A0), \
	AROS_LCA(ASN1_OCTET_STRING *, (___ia5), A1), \
	struct Library *, AMISSL_BASE_NAME, 2121, Amissl)

#define s2i_ASN1_OCTET_STRING(___method, ___ctx, ___str) \
	AROS_LC3(ASN1_OCTET_STRING *, s2i_ASN1_OCTET_STRING, \
	AROS_LCA(X509V3_EXT_METHOD *, (___method), A0), \
	AROS_LCA(X509V3_CTX *, (___ctx), A1), \
	AROS_LCA(char *, (___str), A2), \
	struct Library *, AMISSL_BASE_NAME, 2122, Amissl)

#define EXTENDED_KEY_USAGE_new() \
	AROS_LC0(EXTENDED_KEY_USAGE *, EXTENDED_KEY_USAGE_new, \
	struct Library *, AMISSL_BASE_NAME, 2123, Amissl)

#define EXTENDED_KEY_USAGE_free(___a) \
	AROS_LC1(void, EXTENDED_KEY_USAGE_free, \
	AROS_LCA(EXTENDED_KEY_USAGE *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 2124, Amissl)

#define d2i_EXTENDED_KEY_USAGE(___a, ___in, ___len) \
	AROS_LC3(EXTENDED_KEY_USAGE *, d2i_EXTENDED_KEY_USAGE, \
	AROS_LCA(EXTENDED_KEY_USAGE **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 2125, Amissl)

#define i2d_EXTENDED_KEY_USAGE(___a, ___out) \
	AROS_LC2(int, i2d_EXTENDED_KEY_USAGE, \
	AROS_LCA(EXTENDED_KEY_USAGE *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 2126, Amissl)

#define EXTENDED_KEY_USAGE_it() \
	AROS_LC0(const ASN1_ITEM *, EXTENDED_KEY_USAGE_it, \
	struct Library *, AMISSL_BASE_NAME, 2127, Amissl)

#define i2a_ACCESS_DESCRIPTION(___bp, ___a) \
	AROS_LC2(int, i2a_ACCESS_DESCRIPTION, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(ACCESS_DESCRIPTION*, (___a), A1), \
	struct Library *, AMISSL_BASE_NAME, 2128, Amissl)

#define CERTIFICATEPOLICIES_new() \
	AROS_LC0(CERTIFICATEPOLICIES *, CERTIFICATEPOLICIES_new, \
	struct Library *, AMISSL_BASE_NAME, 2129, Amissl)

#define CERTIFICATEPOLICIES_free(___a) \
	AROS_LC1(void, CERTIFICATEPOLICIES_free, \
	AROS_LCA(CERTIFICATEPOLICIES *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 2130, Amissl)

#define d2i_CERTIFICATEPOLICIES(___a, ___in, ___len) \
	AROS_LC3(CERTIFICATEPOLICIES *, d2i_CERTIFICATEPOLICIES, \
	AROS_LCA(CERTIFICATEPOLICIES **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 2131, Amissl)

#define i2d_CERTIFICATEPOLICIES(___a, ___out) \
	AROS_LC2(int, i2d_CERTIFICATEPOLICIES, \
	AROS_LCA(CERTIFICATEPOLICIES *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 2132, Amissl)

#define CERTIFICATEPOLICIES_it() \
	AROS_LC0(const ASN1_ITEM *, CERTIFICATEPOLICIES_it, \
	struct Library *, AMISSL_BASE_NAME, 2133, Amissl)

#define POLICYINFO_new() \
	AROS_LC0(POLICYINFO *, POLICYINFO_new, \
	struct Library *, AMISSL_BASE_NAME, 2134, Amissl)

#define POLICYINFO_free(___a) \
	AROS_LC1(void, POLICYINFO_free, \
	AROS_LCA(POLICYINFO *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 2135, Amissl)

#define d2i_POLICYINFO(___a, ___in, ___len) \
	AROS_LC3(POLICYINFO *, d2i_POLICYINFO, \
	AROS_LCA(POLICYINFO **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 2136, Amissl)

#define i2d_POLICYINFO(___a, ___out) \
	AROS_LC2(int, i2d_POLICYINFO, \
	AROS_LCA(POLICYINFO *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 2137, Amissl)

#define POLICYINFO_it() \
	AROS_LC0(const ASN1_ITEM *, POLICYINFO_it, \
	struct Library *, AMISSL_BASE_NAME, 2138, Amissl)

#define POLICYQUALINFO_new() \
	AROS_LC0(POLICYQUALINFO *, POLICYQUALINFO_new, \
	struct Library *, AMISSL_BASE_NAME, 2139, Amissl)

#define POLICYQUALINFO_free(___a) \
	AROS_LC1(void, POLICYQUALINFO_free, \
	AROS_LCA(POLICYQUALINFO *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 2140, Amissl)

#define d2i_POLICYQUALINFO(___a, ___in, ___len) \
	AROS_LC3(POLICYQUALINFO *, d2i_POLICYQUALINFO, \
	AROS_LCA(POLICYQUALINFO **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 2141, Amissl)

#define i2d_POLICYQUALINFO(___a, ___out) \
	AROS_LC2(int, i2d_POLICYQUALINFO, \
	AROS_LCA(POLICYQUALINFO *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 2142, Amissl)

#define POLICYQUALINFO_it() \
	AROS_LC0(const ASN1_ITEM *, POLICYQUALINFO_it, \
	struct Library *, AMISSL_BASE_NAME, 2143, Amissl)

#define USERNOTICE_new() \
	AROS_LC0(USERNOTICE *, USERNOTICE_new, \
	struct Library *, AMISSL_BASE_NAME, 2144, Amissl)

#define USERNOTICE_free(___a) \
	AROS_LC1(void, USERNOTICE_free, \
	AROS_LCA(USERNOTICE *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 2145, Amissl)

#define d2i_USERNOTICE(___a, ___in, ___len) \
	AROS_LC3(USERNOTICE *, d2i_USERNOTICE, \
	AROS_LCA(USERNOTICE **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 2146, Amissl)

#define i2d_USERNOTICE(___a, ___out) \
	AROS_LC2(int, i2d_USERNOTICE, \
	AROS_LCA(USERNOTICE *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 2147, Amissl)

#define USERNOTICE_it() \
	AROS_LC0(const ASN1_ITEM *, USERNOTICE_it, \
	struct Library *, AMISSL_BASE_NAME, 2148, Amissl)

#define NOTICEREF_new() \
	AROS_LC0(NOTICEREF *, NOTICEREF_new, \
	struct Library *, AMISSL_BASE_NAME, 2149, Amissl)

#define NOTICEREF_free(___a) \
	AROS_LC1(void, NOTICEREF_free, \
	AROS_LCA(NOTICEREF *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 2150, Amissl)

#define d2i_NOTICEREF(___a, ___in, ___len) \
	AROS_LC3(NOTICEREF *, d2i_NOTICEREF, \
	AROS_LCA(NOTICEREF **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 2151, Amissl)

#define i2d_NOTICEREF(___a, ___out) \
	AROS_LC2(int, i2d_NOTICEREF, \
	AROS_LCA(NOTICEREF *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 2152, Amissl)

#define NOTICEREF_it() \
	AROS_LC0(const ASN1_ITEM *, NOTICEREF_it, \
	struct Library *, AMISSL_BASE_NAME, 2153, Amissl)

#define CRL_DIST_POINTS_new() \
	AROS_LC0(CRL_DIST_POINTS *, CRL_DIST_POINTS_new, \
	struct Library *, AMISSL_BASE_NAME, 2154, Amissl)

#define CRL_DIST_POINTS_free(___a) \
	AROS_LC1(void, CRL_DIST_POINTS_free, \
	AROS_LCA(CRL_DIST_POINTS *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 2155, Amissl)

#define d2i_CRL_DIST_POINTS(___a, ___in, ___len) \
	AROS_LC3(CRL_DIST_POINTS *, d2i_CRL_DIST_POINTS, \
	AROS_LCA(CRL_DIST_POINTS **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 2156, Amissl)

#define i2d_CRL_DIST_POINTS(___a, ___out) \
	AROS_LC2(int, i2d_CRL_DIST_POINTS, \
	AROS_LCA(CRL_DIST_POINTS *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 2157, Amissl)

#define CRL_DIST_POINTS_it() \
	AROS_LC0(const ASN1_ITEM *, CRL_DIST_POINTS_it, \
	struct Library *, AMISSL_BASE_NAME, 2158, Amissl)

#define DIST_POINT_new() \
	AROS_LC0(DIST_POINT *, DIST_POINT_new, \
	struct Library *, AMISSL_BASE_NAME, 2159, Amissl)

#define DIST_POINT_free(___a) \
	AROS_LC1(void, DIST_POINT_free, \
	AROS_LCA(DIST_POINT *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 2160, Amissl)

#define d2i_DIST_POINT(___a, ___in, ___len) \
	AROS_LC3(DIST_POINT *, d2i_DIST_POINT, \
	AROS_LCA(DIST_POINT **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 2161, Amissl)

#define i2d_DIST_POINT(___a, ___out) \
	AROS_LC2(int, i2d_DIST_POINT, \
	AROS_LCA(DIST_POINT *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 2162, Amissl)

#define DIST_POINT_it() \
	AROS_LC0(const ASN1_ITEM *, DIST_POINT_it, \
	struct Library *, AMISSL_BASE_NAME, 2163, Amissl)

#define DIST_POINT_NAME_new() \
	AROS_LC0(DIST_POINT_NAME *, DIST_POINT_NAME_new, \
	struct Library *, AMISSL_BASE_NAME, 2164, Amissl)

#define DIST_POINT_NAME_free(___a) \
	AROS_LC1(void, DIST_POINT_NAME_free, \
	AROS_LCA(DIST_POINT_NAME *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 2165, Amissl)

#define d2i_DIST_POINT_NAME(___a, ___in, ___len) \
	AROS_LC3(DIST_POINT_NAME *, d2i_DIST_POINT_NAME, \
	AROS_LCA(DIST_POINT_NAME **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 2166, Amissl)

#define i2d_DIST_POINT_NAME(___a, ___out) \
	AROS_LC2(int, i2d_DIST_POINT_NAME, \
	AROS_LCA(DIST_POINT_NAME *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 2167, Amissl)

#define DIST_POINT_NAME_it() \
	AROS_LC0(const ASN1_ITEM *, DIST_POINT_NAME_it, \
	struct Library *, AMISSL_BASE_NAME, 2168, Amissl)

#define ACCESS_DESCRIPTION_new() \
	AROS_LC0(ACCESS_DESCRIPTION *, ACCESS_DESCRIPTION_new, \
	struct Library *, AMISSL_BASE_NAME, 2169, Amissl)

#define ACCESS_DESCRIPTION_free(___a) \
	AROS_LC1(void, ACCESS_DESCRIPTION_free, \
	AROS_LCA(ACCESS_DESCRIPTION *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 2170, Amissl)

#define d2i_ACCESS_DESCRIPTION(___a, ___in, ___len) \
	AROS_LC3(ACCESS_DESCRIPTION *, d2i_ACCESS_DESCRIPTION, \
	AROS_LCA(ACCESS_DESCRIPTION **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 2171, Amissl)

#define i2d_ACCESS_DESCRIPTION(___a, ___out) \
	AROS_LC2(int, i2d_ACCESS_DESCRIPTION, \
	AROS_LCA(ACCESS_DESCRIPTION *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 2172, Amissl)

#define ACCESS_DESCRIPTION_it() \
	AROS_LC0(const ASN1_ITEM *, ACCESS_DESCRIPTION_it, \
	struct Library *, AMISSL_BASE_NAME, 2173, Amissl)

#define AUTHORITY_INFO_ACCESS_new() \
	AROS_LC0(AUTHORITY_INFO_ACCESS *, AUTHORITY_INFO_ACCESS_new, \
	struct Library *, AMISSL_BASE_NAME, 2174, Amissl)

#define AUTHORITY_INFO_ACCESS_free(___a) \
	AROS_LC1(void, AUTHORITY_INFO_ACCESS_free, \
	AROS_LCA(AUTHORITY_INFO_ACCESS *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 2175, Amissl)

#define d2i_AUTHORITY_INFO_ACCESS(___a, ___in, ___len) \
	AROS_LC3(AUTHORITY_INFO_ACCESS *, d2i_AUTHORITY_INFO_ACCESS, \
	AROS_LCA(AUTHORITY_INFO_ACCESS **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 2176, Amissl)

#define i2d_AUTHORITY_INFO_ACCESS(___a, ___out) \
	AROS_LC2(int, i2d_AUTHORITY_INFO_ACCESS, \
	AROS_LCA(AUTHORITY_INFO_ACCESS *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 2177, Amissl)

#define AUTHORITY_INFO_ACCESS_it() \
	AROS_LC0(const ASN1_ITEM *, AUTHORITY_INFO_ACCESS_it, \
	struct Library *, AMISSL_BASE_NAME, 2178, Amissl)

#define v2i_GENERAL_NAME(___method, ___ctx, ___cnf) \
	AROS_LC3(GENERAL_NAME *, v2i_GENERAL_NAME, \
	AROS_LCA(X509V3_EXT_METHOD *, (___method), A0), \
	AROS_LCA(X509V3_CTX *, (___ctx), A1), \
	AROS_LCA(CONF_VALUE *, (___cnf), A2), \
	struct Library *, AMISSL_BASE_NAME, 2179, Amissl)

#define X509V3_conf_free(___val) \
	AROS_LC1(void, X509V3_conf_free, \
	AROS_LCA(CONF_VALUE *, (___val), A0), \
	struct Library *, AMISSL_BASE_NAME, 2180, Amissl)

#define X509V3_EXT_nconf_nid(___conf, ___ctx, ___ext_nid, ___value) \
	AROS_LC4(X509_EXTENSION *, X509V3_EXT_nconf_nid, \
	AROS_LCA(CONF *, (___conf), A0), \
	AROS_LCA(X509V3_CTX *, (___ctx), A1), \
	AROS_LCA(int, (___ext_nid), D0), \
	AROS_LCA(char *, (___value), A2), \
	struct Library *, AMISSL_BASE_NAME, 2181, Amissl)

#define X509V3_EXT_nconf(___conf, ___ctx, ___name, ___value) \
	AROS_LC4(X509_EXTENSION *, X509V3_EXT_nconf, \
	AROS_LCA(CONF *, (___conf), A0), \
	AROS_LCA(X509V3_CTX *, (___ctx), A1), \
	AROS_LCA(char *, (___name), A2), \
	AROS_LCA(char *, (___value), A3), \
	struct Library *, AMISSL_BASE_NAME, 2182, Amissl)

#define X509V3_EXT_add_nconf_sk(___conf, ___ctx, ___section, ___sk) \
	AROS_LC4(int, X509V3_EXT_add_nconf_sk, \
	AROS_LCA(CONF *, (___conf), A0), \
	AROS_LCA(X509V3_CTX *, (___ctx), A1), \
	AROS_LCA(char *, (___section), A2), \
	AROS_LCA(STACK_OF(X509_EXTENSION) **, (___sk), A3), \
	struct Library *, AMISSL_BASE_NAME, 2183, Amissl)

#define X509V3_EXT_add_nconf(___conf, ___ctx, ___section, ___cert) \
	AROS_LC4(int, X509V3_EXT_add_nconf, \
	AROS_LCA(CONF *, (___conf), A0), \
	AROS_LCA(X509V3_CTX *, (___ctx), A1), \
	AROS_LCA(char *, (___section), A2), \
	AROS_LCA(X509 *, (___cert), A3), \
	struct Library *, AMISSL_BASE_NAME, 2184, Amissl)

#define X509V3_EXT_REQ_add_nconf(___conf, ___ctx, ___section, ___req) \
	AROS_LC4(int, X509V3_EXT_REQ_add_nconf, \
	AROS_LCA(CONF *, (___conf), A0), \
	AROS_LCA(X509V3_CTX *, (___ctx), A1), \
	AROS_LCA(char *, (___section), A2), \
	AROS_LCA(X509_REQ *, (___req), A3), \
	struct Library *, AMISSL_BASE_NAME, 2185, Amissl)

#define X509V3_EXT_CRL_add_nconf(___conf, ___ctx, ___section, ___crl) \
	AROS_LC4(int, X509V3_EXT_CRL_add_nconf, \
	AROS_LCA(CONF *, (___conf), A0), \
	AROS_LCA(X509V3_CTX *, (___ctx), A1), \
	AROS_LCA(char *, (___section), A2), \
	AROS_LCA(X509_CRL *, (___crl), A3), \
	struct Library *, AMISSL_BASE_NAME, 2186, Amissl)

#define X509V3_EXT_conf_nid(___conf, ___ctx, ___ext_nid, ___value) \
	AROS_LC4(X509_EXTENSION *, X509V3_EXT_conf_nid, \
	AROS_LCA(LHASH *, (___conf), A0), \
	AROS_LCA(X509V3_CTX *, (___ctx), A1), \
	AROS_LCA(int, (___ext_nid), D0), \
	AROS_LCA(char *, (___value), A2), \
	struct Library *, AMISSL_BASE_NAME, 2187, Amissl)

#define X509V3_EXT_conf(___conf, ___ctx, ___name, ___value) \
	AROS_LC4(X509_EXTENSION *, X509V3_EXT_conf, \
	AROS_LCA(LHASH *, (___conf), A0), \
	AROS_LCA(X509V3_CTX *, (___ctx), A1), \
	AROS_LCA(char *, (___name), A2), \
	AROS_LCA(char *, (___value), A3), \
	struct Library *, AMISSL_BASE_NAME, 2188, Amissl)

#define X509V3_EXT_add_conf(___conf, ___ctx, ___section, ___cert) \
	AROS_LC4(int, X509V3_EXT_add_conf, \
	AROS_LCA(LHASH *, (___conf), A0), \
	AROS_LCA(X509V3_CTX *, (___ctx), A1), \
	AROS_LCA(char *, (___section), A2), \
	AROS_LCA(X509 *, (___cert), A3), \
	struct Library *, AMISSL_BASE_NAME, 2189, Amissl)

#define X509V3_EXT_REQ_add_conf(___conf, ___ctx, ___section, ___req) \
	AROS_LC4(int, X509V3_EXT_REQ_add_conf, \
	AROS_LCA(LHASH *, (___conf), A0), \
	AROS_LCA(X509V3_CTX *, (___ctx), A1), \
	AROS_LCA(char *, (___section), A2), \
	AROS_LCA(X509_REQ *, (___req), A3), \
	struct Library *, AMISSL_BASE_NAME, 2190, Amissl)

#define X509V3_EXT_CRL_add_conf(___conf, ___ctx, ___section, ___crl) \
	AROS_LC4(int, X509V3_EXT_CRL_add_conf, \
	AROS_LCA(LHASH *, (___conf), A0), \
	AROS_LCA(X509V3_CTX *, (___ctx), A1), \
	AROS_LCA(char *, (___section), A2), \
	AROS_LCA(X509_CRL *, (___crl), A3), \
	struct Library *, AMISSL_BASE_NAME, 2191, Amissl)

#define X509V3_add_value_bool_nf(___name, ___asn1_bool, ___extlist) \
	AROS_LC3(int, X509V3_add_value_bool_nf, \
	AROS_LCA(char *, (___name), A0), \
	AROS_LCA(int, (___asn1_bool), D0), \
	AROS_LCA(STACK_OF(CONF_VALUE) **, (___extlist), A1), \
	struct Library *, AMISSL_BASE_NAME, 2192, Amissl)

#define X509V3_get_value_bool(___value, ___asn1_bool) \
	AROS_LC2(int, X509V3_get_value_bool, \
	AROS_LCA(CONF_VALUE *, (___value), A0), \
	AROS_LCA(int *, (___asn1_bool), A1), \
	struct Library *, AMISSL_BASE_NAME, 2193, Amissl)

#define X509V3_get_value_int(___value, ___aint) \
	AROS_LC2(int, X509V3_get_value_int, \
	AROS_LCA(CONF_VALUE *, (___value), A0), \
	AROS_LCA(ASN1_INTEGER **, (___aint), A1), \
	struct Library *, AMISSL_BASE_NAME, 2194, Amissl)

#define X509V3_set_nconf(___ctx, ___conf) \
	AROS_LC2(void, X509V3_set_nconf, \
	AROS_LCA(X509V3_CTX *, (___ctx), A0), \
	AROS_LCA(CONF *, (___conf), A1), \
	struct Library *, AMISSL_BASE_NAME, 2195, Amissl)

#define X509V3_set_conf_lhash(___ctx, ___lhash) \
	AROS_LC2(void, X509V3_set_conf_lhash, \
	AROS_LCA(X509V3_CTX *, (___ctx), A0), \
	AROS_LCA(LHASH *, (___lhash), A1), \
	struct Library *, AMISSL_BASE_NAME, 2196, Amissl)

#define X509V3_get_string(___ctx, ___name, ___section) \
	AROS_LC3(char *, X509V3_get_string, \
	AROS_LCA(X509V3_CTX *, (___ctx), A0), \
	AROS_LCA(char *, (___name), A1), \
	AROS_LCA(char *, (___section), A2), \
	struct Library *, AMISSL_BASE_NAME, 2197, Amissl)

#define X509V3_get_section(___ctx, ___section) \
	AROS_LC2(STACK_OF(CONF_VALUE) *, X509V3_get_section, \
	AROS_LCA(X509V3_CTX *, (___ctx), A0), \
	AROS_LCA(char *, (___section), A1), \
	struct Library *, AMISSL_BASE_NAME, 2198, Amissl)

#define X509V3_string_free(___ctx, ___str) \
	AROS_LC2(void, X509V3_string_free, \
	AROS_LCA(X509V3_CTX *, (___ctx), A0), \
	AROS_LCA(char *, (___str), A1), \
	struct Library *, AMISSL_BASE_NAME, 2199, Amissl)

#define X509V3_section_free(___ctx, ___section) \
	AROS_LC2(void, X509V3_section_free, \
	AROS_LCA(X509V3_CTX *, (___ctx), A0), \
	AROS_LCA(STACK_OF(CONF_VALUE) *, (___section), A1), \
	struct Library *, AMISSL_BASE_NAME, 2200, Amissl)

#define X509V3_set_ctx(___ctx, ___issuer, ___subject, ___req, ___crl, ___flags) \
	AROS_LC6(void, X509V3_set_ctx, \
	AROS_LCA(X509V3_CTX *, (___ctx), A0), \
	AROS_LCA(X509 *, (___issuer), A1), \
	AROS_LCA(X509 *, (___subject), A2), \
	AROS_LCA(X509_REQ *, (___req), A3), \
	AROS_LCA(X509_CRL *, (___crl), D0), \
	AROS_LCA(int, (___flags), D1), \
	struct Library *, AMISSL_BASE_NAME, 2201, Amissl)

#define X509V3_add_value(___name, ___value, ___extlist) \
	AROS_LC3(int, X509V3_add_value, \
	AROS_LCA(const char *, (___name), A0), \
	AROS_LCA(const char *, (___value), A1), \
	AROS_LCA(STACK_OF(CONF_VALUE) **, (___extlist), A2), \
	struct Library *, AMISSL_BASE_NAME, 2202, Amissl)

#define X509V3_add_value_uchar(___name, ___value, ___extlist) \
	AROS_LC3(int, X509V3_add_value_uchar, \
	AROS_LCA(const char *, (___name), A0), \
	AROS_LCA(const unsigned char *, (___value), A1), \
	AROS_LCA(STACK_OF(CONF_VALUE) **, (___extlist), A2), \
	struct Library *, AMISSL_BASE_NAME, 2203, Amissl)

#define X509V3_add_value_bool(___name, ___asn1_bool, ___extlist) \
	AROS_LC3(int, X509V3_add_value_bool, \
	AROS_LCA(const char *, (___name), A0), \
	AROS_LCA(int, (___asn1_bool), D0), \
	AROS_LCA(STACK_OF(CONF_VALUE) **, (___extlist), A1), \
	struct Library *, AMISSL_BASE_NAME, 2204, Amissl)

#define X509V3_add_value_int(___name, ___aint, ___extlist) \
	AROS_LC3(int, X509V3_add_value_int, \
	AROS_LCA(const char *, (___name), A0), \
	AROS_LCA(ASN1_INTEGER *, (___aint), A1), \
	AROS_LCA(STACK_OF(CONF_VALUE) **, (___extlist), A2), \
	struct Library *, AMISSL_BASE_NAME, 2205, Amissl)

#define i2s_ASN1_INTEGER(___meth, ___aint) \
	AROS_LC2(char *, i2s_ASN1_INTEGER, \
	AROS_LCA(X509V3_EXT_METHOD *, (___meth), A0), \
	AROS_LCA(ASN1_INTEGER *, (___aint), A1), \
	struct Library *, AMISSL_BASE_NAME, 2206, Amissl)

#define s2i_ASN1_INTEGER(___meth, ___value) \
	AROS_LC2(ASN1_INTEGER *, s2i_ASN1_INTEGER, \
	AROS_LCA(X509V3_EXT_METHOD *, (___meth), A0), \
	AROS_LCA(char *, (___value), A1), \
	struct Library *, AMISSL_BASE_NAME, 2207, Amissl)

#define i2s_ASN1_ENUMERATED(___meth, ___aint) \
	AROS_LC2(char *, i2s_ASN1_ENUMERATED, \
	AROS_LCA(X509V3_EXT_METHOD *, (___meth), A0), \
	AROS_LCA(ASN1_ENUMERATED *, (___aint), A1), \
	struct Library *, AMISSL_BASE_NAME, 2208, Amissl)

#define i2s_ASN1_ENUMERATED_TABLE(___meth, ___aint) \
	AROS_LC2(char *, i2s_ASN1_ENUMERATED_TABLE, \
	AROS_LCA(X509V3_EXT_METHOD *, (___meth), A0), \
	AROS_LCA(ASN1_ENUMERATED *, (___aint), A1), \
	struct Library *, AMISSL_BASE_NAME, 2209, Amissl)

#define X509V3_EXT_add(___ext) \
	AROS_LC1(int, X509V3_EXT_add, \
	AROS_LCA(X509V3_EXT_METHOD *, (___ext), A0), \
	struct Library *, AMISSL_BASE_NAME, 2210, Amissl)

#define X509V3_EXT_add_list(___extlist) \
	AROS_LC1(int, X509V3_EXT_add_list, \
	AROS_LCA(X509V3_EXT_METHOD *, (___extlist), A0), \
	struct Library *, AMISSL_BASE_NAME, 2211, Amissl)

#define X509V3_EXT_add_alias(___nid_to, ___nid_from) \
	AROS_LC2(int, X509V3_EXT_add_alias, \
	AROS_LCA(int, (___nid_to), D0), \
	AROS_LCA(int, (___nid_from), D1), \
	struct Library *, AMISSL_BASE_NAME, 2212, Amissl)

#define X509V3_EXT_cleanup() \
	AROS_LC0(void, X509V3_EXT_cleanup, \
	struct Library *, AMISSL_BASE_NAME, 2213, Amissl)

#define X509V3_EXT_get(___ext) \
	AROS_LC1(X509V3_EXT_METHOD *, X509V3_EXT_get, \
	AROS_LCA(X509_EXTENSION *, (___ext), A0), \
	struct Library *, AMISSL_BASE_NAME, 2214, Amissl)

#define X509V3_EXT_get_nid(___nid) \
	AROS_LC1(X509V3_EXT_METHOD *, X509V3_EXT_get_nid, \
	AROS_LCA(int, (___nid), D0), \
	struct Library *, AMISSL_BASE_NAME, 2215, Amissl)

#define X509V3_add_standard_extensions() \
	AROS_LC0(int, X509V3_add_standard_extensions, \
	struct Library *, AMISSL_BASE_NAME, 2216, Amissl)

#define X509V3_parse_list(___line) \
	AROS_LC1(STACK_OF(CONF_VALUE) *, X509V3_parse_list, \
	AROS_LCA(const char *, (___line), A0), \
	struct Library *, AMISSL_BASE_NAME, 2217, Amissl)

#define X509V3_EXT_d2i(___ext) \
	AROS_LC1(void *, X509V3_EXT_d2i, \
	AROS_LCA(X509_EXTENSION *, (___ext), A0), \
	struct Library *, AMISSL_BASE_NAME, 2218, Amissl)

#define X509V3_get_d2i(___x, ___nid, ___crit, ___idx) \
	AROS_LC4(void *, X509V3_get_d2i, \
	AROS_LCA(STACK_OF(X509_EXTENSION) *, (___x), A0), \
	AROS_LCA(int, (___nid), D0), \
	AROS_LCA(int *, (___crit), A1), \
	AROS_LCA(int *, (___idx), A2), \
	struct Library *, AMISSL_BASE_NAME, 2219, Amissl)

#define X509V3_EXT_i2d(___ext_nid, ___crit, ___ext_struc) \
	AROS_LC3(X509_EXTENSION *, X509V3_EXT_i2d, \
	AROS_LCA(int, (___ext_nid), D0), \
	AROS_LCA(int, (___crit), D1), \
	AROS_LCA(void *, (___ext_struc), A0), \
	struct Library *, AMISSL_BASE_NAME, 2220, Amissl)

#define X509V3_add1_i2d(___x, ___nid, ___value, ___crit, ___flags) \
	AROS_LC5(int, X509V3_add1_i2d, \
	AROS_LCA(STACK_OF(X509_EXTENSION) **, (___x), A0), \
	AROS_LCA(int, (___nid), D0), \
	AROS_LCA(void *, (___value), A1), \
	AROS_LCA(int, (___crit), D1), \
	AROS_LCA(unsigned long, (___flags), D2), \
	struct Library *, AMISSL_BASE_NAME, 2221, Amissl)

#define hex_to_string(___buffer, ___len) \
	AROS_LC2(char *, hex_to_string, \
	AROS_LCA(unsigned char *, (___buffer), A0), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 2222, Amissl)

#define string_to_hex(___str, ___len) \
	AROS_LC2(unsigned char *, string_to_hex, \
	AROS_LCA(char *, (___str), A0), \
	AROS_LCA(long *, (___len), A1), \
	struct Library *, AMISSL_BASE_NAME, 2223, Amissl)

#define name_cmp(___name, ___cmp) \
	AROS_LC2(int, name_cmp, \
	AROS_LCA(const char *, (___name), A0), \
	AROS_LCA(const char *, (___cmp), A1), \
	struct Library *, AMISSL_BASE_NAME, 2224, Amissl)

#define X509V3_EXT_val_prn(___out, ___val, ___indent, ___ml) \
	AROS_LC4(void, X509V3_EXT_val_prn, \
	AROS_LCA(BIO *, (___out), A0), \
	AROS_LCA(STACK_OF(CONF_VALUE) *, (___val), A1), \
	AROS_LCA(int, (___indent), D0), \
	AROS_LCA(int, (___ml), D1), \
	struct Library *, AMISSL_BASE_NAME, 2225, Amissl)

#define X509V3_EXT_print(___out, ___ext, ___flag, ___indent) \
	AROS_LC4(int, X509V3_EXT_print, \
	AROS_LCA(BIO *, (___out), A0), \
	AROS_LCA(X509_EXTENSION *, (___ext), A1), \
	AROS_LCA(unsigned long, (___flag), D0), \
	AROS_LCA(int, (___indent), D1), \
	struct Library *, AMISSL_BASE_NAME, 2226, Amissl)

#define X509V3_extensions_print(___out, ___title, ___exts, ___flag, ___indent) \
	AROS_LC5(int, X509V3_extensions_print, \
	AROS_LCA(BIO *, (___out), A0), \
	AROS_LCA(char *, (___title), A1), \
	AROS_LCA(STACK_OF(X509_EXTENSION) *, (___exts), A2), \
	AROS_LCA(unsigned long, (___flag), D0), \
	AROS_LCA(int, (___indent), D1), \
	struct Library *, AMISSL_BASE_NAME, 2227, Amissl)

#define X509_check_purpose(___x, ___id, ___ca) \
	AROS_LC3(int, X509_check_purpose, \
	AROS_LCA(X509 *, (___x), A0), \
	AROS_LCA(int, (___id), D0), \
	AROS_LCA(int, (___ca), D1), \
	struct Library *, AMISSL_BASE_NAME, 2228, Amissl)

#define X509_supported_extension(___ex) \
	AROS_LC1(int, X509_supported_extension, \
	AROS_LCA(X509_EXTENSION *, (___ex), A0), \
	struct Library *, AMISSL_BASE_NAME, 2229, Amissl)

#define X509_PURPOSE_set(___p, ___purpose) \
	AROS_LC2(int, X509_PURPOSE_set, \
	AROS_LCA(int *, (___p), A0), \
	AROS_LCA(int, (___purpose), D0), \
	struct Library *, AMISSL_BASE_NAME, 2230, Amissl)

#define X509_check_issued(___issuer, ___subject) \
	AROS_LC2(int, X509_check_issued, \
	AROS_LCA(X509 *, (___issuer), A0), \
	AROS_LCA(X509 *, (___subject), A1), \
	struct Library *, AMISSL_BASE_NAME, 2231, Amissl)

#define X509_PURPOSE_get_count() \
	AROS_LC0(int, X509_PURPOSE_get_count, \
	struct Library *, AMISSL_BASE_NAME, 2232, Amissl)

#define X509_PURPOSE_get0(___idx) \
	AROS_LC1(X509_PURPOSE *, X509_PURPOSE_get0, \
	AROS_LCA(int, (___idx), D0), \
	struct Library *, AMISSL_BASE_NAME, 2233, Amissl)

#define X509_PURPOSE_get_by_sname(___sname) \
	AROS_LC1(int, X509_PURPOSE_get_by_sname, \
	AROS_LCA(char *, (___sname), A0), \
	struct Library *, AMISSL_BASE_NAME, 2234, Amissl)

#define X509_PURPOSE_get_by_id(___id) \
	AROS_LC1(int, X509_PURPOSE_get_by_id, \
	AROS_LCA(int, (___id), D0), \
	struct Library *, AMISSL_BASE_NAME, 2235, Amissl)

#define X509_PURPOSE_add(___id, ___trust, ___flags, ___ck, ___name, ___sname, ___arg) \
	AROS_LC7(int, X509_PURPOSE_add, \
	AROS_LCA(int, (___id), D0), \
	AROS_LCA(int, (___trust), D1), \
	AROS_LCA(int, (___flags), D2), \
	AROS_LCA(int (*)(const X509_PURPOSE *,const X509 *,int), (___ck), A0), \
	AROS_LCA(char *, (___name), A1), \
	AROS_LCA(char *, (___sname), A2), \
	AROS_LCA(void *, (___arg), A3), \
	struct Library *, AMISSL_BASE_NAME, 2236, Amissl)

#define X509_PURPOSE_get0_name(___xp) \
	AROS_LC1(char *, X509_PURPOSE_get0_name, \
	AROS_LCA(X509_PURPOSE *, (___xp), A0), \
	struct Library *, AMISSL_BASE_NAME, 2237, Amissl)

#define X509_PURPOSE_get0_sname(___xp) \
	AROS_LC1(char *, X509_PURPOSE_get0_sname, \
	AROS_LCA(X509_PURPOSE *, (___xp), A0), \
	struct Library *, AMISSL_BASE_NAME, 2238, Amissl)

#define X509_PURPOSE_get_trust(___xp) \
	AROS_LC1(int, X509_PURPOSE_get_trust, \
	AROS_LCA(X509_PURPOSE *, (___xp), A0), \
	struct Library *, AMISSL_BASE_NAME, 2239, Amissl)

#define X509_PURPOSE_cleanup() \
	AROS_LC0(void, X509_PURPOSE_cleanup, \
	struct Library *, AMISSL_BASE_NAME, 2240, Amissl)

#define X509_PURPOSE_get_id(___a) \
	AROS_LC1(int, X509_PURPOSE_get_id, \
	AROS_LCA(X509_PURPOSE *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 2241, Amissl)

#define X509_get1_email(___x) \
	AROS_LC1(STACK *, X509_get1_email, \
	AROS_LCA(X509 *, (___x), A0), \
	struct Library *, AMISSL_BASE_NAME, 2242, Amissl)

#define X509_REQ_get1_email(___x) \
	AROS_LC1(STACK *, X509_REQ_get1_email, \
	AROS_LCA(X509_REQ *, (___x), A0), \
	struct Library *, AMISSL_BASE_NAME, 2243, Amissl)

#define X509_email_free(___sk) \
	AROS_LC1(void, X509_email_free, \
	AROS_LCA(STACK *, (___sk), A0), \
	struct Library *, AMISSL_BASE_NAME, 2244, Amissl)

#define ERR_load_X509V3_strings() \
	AROS_LC0(void, ERR_load_X509V3_strings, \
	struct Library *, AMISSL_BASE_NAME, 2245, Amissl)

#define AES_options() \
	AROS_LC0(const char *, AES_options, \
	struct Library *, AMISSL_BASE_NAME, 2246, Amissl)

#define AES_set_encrypt_key(___userKey, ___bits, ___key) \
	AROS_LC3(int, AES_set_encrypt_key, \
	AROS_LCA(const unsigned char *, (___userKey), A0), \
	AROS_LCA(const int, (___bits), D0), \
	AROS_LCA(AES_KEY *, (___key), A1), \
	struct Library *, AMISSL_BASE_NAME, 2247, Amissl)

#define AES_set_decrypt_key(___userKey, ___bits, ___key) \
	AROS_LC3(int, AES_set_decrypt_key, \
	AROS_LCA(const unsigned char *, (___userKey), A0), \
	AROS_LCA(const int, (___bits), D0), \
	AROS_LCA(AES_KEY *, (___key), A1), \
	struct Library *, AMISSL_BASE_NAME, 2248, Amissl)

#define AES_encrypt(___in, ___out, ___key) \
	AROS_LC3(void, AES_encrypt, \
	AROS_LCA(const unsigned char *, (___in), A0), \
	AROS_LCA(unsigned char *, (___out), A1), \
	AROS_LCA(const AES_KEY *, (___key), A2), \
	struct Library *, AMISSL_BASE_NAME, 2249, Amissl)

#define AES_decrypt(___in, ___out, ___key) \
	AROS_LC3(void, AES_decrypt, \
	AROS_LCA(const unsigned char *, (___in), A0), \
	AROS_LCA(unsigned char *, (___out), A1), \
	AROS_LCA(const AES_KEY *, (___key), A2), \
	struct Library *, AMISSL_BASE_NAME, 2250, Amissl)

#define AES_ecb_encrypt(___in, ___out, ___key, ___enc) \
	AROS_LC4(void, AES_ecb_encrypt, \
	AROS_LCA(const unsigned char *, (___in), A0), \
	AROS_LCA(unsigned char *, (___out), A1), \
	AROS_LCA(const AES_KEY *, (___key), A2), \
	AROS_LCA(const int, (___enc), D0), \
	struct Library *, AMISSL_BASE_NAME, 2251, Amissl)

#define AES_cbc_encrypt(___in, ___out, ___length, ___key, ___ivec, ___enc) \
	AROS_LC6(void, AES_cbc_encrypt, \
	AROS_LCA(const unsigned char *, (___in), A0), \
	AROS_LCA(unsigned char *, (___out), A1), \
	AROS_LCA(const unsigned long, (___length), D0), \
	AROS_LCA(const AES_KEY *, (___key), A2), \
	AROS_LCA(unsigned char *, (___ivec), A3), \
	AROS_LCA(const int, (___enc), D1), \
	struct Library *, AMISSL_BASE_NAME, 2252, Amissl)

#define AES_cfb128_encrypt(___in, ___out, ___length, ___key, ___ivec, ___num, ___enc) \
	AROS_LC7(void, AES_cfb128_encrypt, \
	AROS_LCA(const unsigned char *, (___in), A0), \
	AROS_LCA(unsigned char *, (___out), A1), \
	AROS_LCA(const unsigned long, (___length), D0), \
	AROS_LCA(const AES_KEY *, (___key), A2), \
	AROS_LCA(unsigned char *, (___ivec), A3), \
	AROS_LCA(int *, (___num), D1), \
	AROS_LCA(const int, (___enc), D2), \
	struct Library *, AMISSL_BASE_NAME, 2253, Amissl)

#define AES_cfb1_encrypt(___in, ___out, ___length, ___key, ___ivec, ___num, ___enc) \
	AROS_LC7(void, AES_cfb1_encrypt, \
	AROS_LCA(const unsigned char *, (___in), A0), \
	AROS_LCA(unsigned char *, (___out), A1), \
	AROS_LCA(const unsigned long, (___length), D0), \
	AROS_LCA(const AES_KEY *, (___key), A2), \
	AROS_LCA(unsigned char *, (___ivec), A3), \
	AROS_LCA(int *, (___num), D1), \
	AROS_LCA(const int, (___enc), D2), \
	struct Library *, AMISSL_BASE_NAME, 2254, Amissl)

#define AES_cfb8_encrypt(___in, ___out, ___length, ___key, ___ivec, ___num, ___enc) \
	AROS_LC7(void, AES_cfb8_encrypt, \
	AROS_LCA(const unsigned char *, (___in), A0), \
	AROS_LCA(unsigned char *, (___out), A1), \
	AROS_LCA(const unsigned long, (___length), D0), \
	AROS_LCA(const AES_KEY *, (___key), A2), \
	AROS_LCA(unsigned char *, (___ivec), A3), \
	AROS_LCA(int *, (___num), D1), \
	AROS_LCA(const int, (___enc), D2), \
	struct Library *, AMISSL_BASE_NAME, 2255, Amissl)

#define AES_cfbr_encrypt_block(___in, ___out, ___nbits, ___key, ___ivec, ___enc) \
	AROS_LC6(void, AES_cfbr_encrypt_block, \
	AROS_LCA(const unsigned char *, (___in), A0), \
	AROS_LCA(unsigned char *, (___out), A1), \
	AROS_LCA(const int, (___nbits), D0), \
	AROS_LCA(const AES_KEY *, (___key), A2), \
	AROS_LCA(unsigned char *, (___ivec), A3), \
	AROS_LCA(const int, (___enc), D1), \
	struct Library *, AMISSL_BASE_NAME, 2256, Amissl)

#define AES_ofb128_encrypt(___in, ___out, ___length, ___key, ___ivec, ___num) \
	AROS_LC6(void, AES_ofb128_encrypt, \
	AROS_LCA(const unsigned char *, (___in), A0), \
	AROS_LCA(unsigned char *, (___out), A1), \
	AROS_LCA(const unsigned long, (___length), D0), \
	AROS_LCA(const AES_KEY *, (___key), A2), \
	AROS_LCA(unsigned char *, (___ivec), A3), \
	AROS_LCA(int *, (___num), D1), \
	struct Library *, AMISSL_BASE_NAME, 2257, Amissl)

#define AES_ctr128_encrypt(___in, ___out, ___length, ___key, ___ivec, ___ecount_buf, ___num) \
	AROS_LC7(void, AES_ctr128_encrypt, \
	AROS_LCA(const unsigned char *, (___in), A0), \
	AROS_LCA(unsigned char *, (___out), A1), \
	AROS_LCA(const unsigned long, (___length), D0), \
	AROS_LCA(const AES_KEY *, (___key), A2), \
	AROS_LCA(unsigned char, (___ivec), D1), \
	AROS_LCA(unsigned char, (___ecount_buf), D2), \
	AROS_LCA(unsigned int *, (___num), A3), \
	struct Library *, AMISSL_BASE_NAME, 2258, Amissl)

#define BF_set_key(___key, ___len, ___data) \
	AROS_LC3(void, BF_set_key, \
	AROS_LCA(BF_KEY *, (___key), A0), \
	AROS_LCA(int, (___len), D0), \
	AROS_LCA(const unsigned char *, (___data), A1), \
	struct Library *, AMISSL_BASE_NAME, 2259, Amissl)

#define BF_encrypt(___data, ___key) \
	AROS_LC2(void, BF_encrypt, \
	AROS_LCA(BF_LONG *, (___data), A0), \
	AROS_LCA(const BF_KEY *, (___key), A1), \
	struct Library *, AMISSL_BASE_NAME, 2260, Amissl)

#define BF_decrypt(___data, ___key) \
	AROS_LC2(void, BF_decrypt, \
	AROS_LCA(BF_LONG *, (___data), A0), \
	AROS_LCA(const BF_KEY *, (___key), A1), \
	struct Library *, AMISSL_BASE_NAME, 2261, Amissl)

#define BF_ecb_encrypt(___in, ___out, ___key, ___enc) \
	AROS_LC4(void, BF_ecb_encrypt, \
	AROS_LCA(const unsigned char *, (___in), A0), \
	AROS_LCA(unsigned char *, (___out), A1), \
	AROS_LCA(const BF_KEY *, (___key), A2), \
	AROS_LCA(int, (___enc), D0), \
	struct Library *, AMISSL_BASE_NAME, 2262, Amissl)

#define BF_cbc_encrypt(___in, ___out, ___length, ___schedule, ___ivec, ___enc) \
	AROS_LC6(void, BF_cbc_encrypt, \
	AROS_LCA(const unsigned char *, (___in), A0), \
	AROS_LCA(unsigned char *, (___out), A1), \
	AROS_LCA(long, (___length), D0), \
	AROS_LCA(const BF_KEY *, (___schedule), A2), \
	AROS_LCA(unsigned char *, (___ivec), A3), \
	AROS_LCA(int, (___enc), D1), \
	struct Library *, AMISSL_BASE_NAME, 2263, Amissl)

#define BF_cfb64_encrypt(___in, ___out, ___length, ___schedule, ___ivec, ___num, ___enc) \
	AROS_LC7(void, BF_cfb64_encrypt, \
	AROS_LCA(const unsigned char *, (___in), A0), \
	AROS_LCA(unsigned char *, (___out), A1), \
	AROS_LCA(long, (___length), D0), \
	AROS_LCA(const BF_KEY *, (___schedule), A2), \
	AROS_LCA(unsigned char *, (___ivec), A3), \
	AROS_LCA(int *, (___num), D1), \
	AROS_LCA(int, (___enc), D2), \
	struct Library *, AMISSL_BASE_NAME, 2264, Amissl)

#define BF_ofb64_encrypt(___in, ___out, ___length, ___schedule, ___ivec, ___num) \
	AROS_LC6(void, BF_ofb64_encrypt, \
	AROS_LCA(const unsigned char *, (___in), A0), \
	AROS_LCA(unsigned char *, (___out), A1), \
	AROS_LCA(long, (___length), D0), \
	AROS_LCA(const BF_KEY *, (___schedule), A2), \
	AROS_LCA(unsigned char *, (___ivec), A3), \
	AROS_LCA(int *, (___num), D1), \
	struct Library *, AMISSL_BASE_NAME, 2265, Amissl)

#define BF_options() \
	AROS_LC0(const char *, BF_options, \
	struct Library *, AMISSL_BASE_NAME, 2266, Amissl)

#define CAST_set_key(___key, ___len, ___data) \
	AROS_LC3(void, CAST_set_key, \
	AROS_LCA(CAST_KEY *, (___key), A0), \
	AROS_LCA(int, (___len), D0), \
	AROS_LCA(const unsigned char *, (___data), A1), \
	struct Library *, AMISSL_BASE_NAME, 2267, Amissl)

#define CAST_ecb_encrypt(___in, ___out, ___key, ___enc) \
	AROS_LC4(void, CAST_ecb_encrypt, \
	AROS_LCA(const unsigned char *, (___in), A0), \
	AROS_LCA(unsigned char *, (___out), A1), \
	AROS_LCA(CAST_KEY *, (___key), A2), \
	AROS_LCA(int, (___enc), D0), \
	struct Library *, AMISSL_BASE_NAME, 2268, Amissl)

#define CAST_encrypt(___data, ___key) \
	AROS_LC2(void, CAST_encrypt, \
	AROS_LCA(CAST_LONG *, (___data), A0), \
	AROS_LCA(CAST_KEY *, (___key), A1), \
	struct Library *, AMISSL_BASE_NAME, 2269, Amissl)

#define CAST_decrypt(___data, ___key) \
	AROS_LC2(void, CAST_decrypt, \
	AROS_LCA(CAST_LONG *, (___data), A0), \
	AROS_LCA(CAST_KEY *, (___key), A1), \
	struct Library *, AMISSL_BASE_NAME, 2270, Amissl)

#define CAST_cbc_encrypt(___in, ___out, ___length, ___ks, ___iv, ___enc) \
	AROS_LC6(void, CAST_cbc_encrypt, \
	AROS_LCA(const unsigned char *, (___in), A0), \
	AROS_LCA(unsigned char *, (___out), A1), \
	AROS_LCA(long, (___length), D0), \
	AROS_LCA(CAST_KEY *, (___ks), A2), \
	AROS_LCA(unsigned char *, (___iv), A3), \
	AROS_LCA(int, (___enc), D1), \
	struct Library *, AMISSL_BASE_NAME, 2271, Amissl)

#define CAST_cfb64_encrypt(___in, ___out, ___length, ___schedule, ___ivec, ___num, ___enc) \
	AROS_LC7(void, CAST_cfb64_encrypt, \
	AROS_LCA(const unsigned char *, (___in), A0), \
	AROS_LCA(unsigned char *, (___out), A1), \
	AROS_LCA(long, (___length), D0), \
	AROS_LCA(CAST_KEY *, (___schedule), A2), \
	AROS_LCA(unsigned char *, (___ivec), A3), \
	AROS_LCA(int *, (___num), D1), \
	AROS_LCA(int, (___enc), D2), \
	struct Library *, AMISSL_BASE_NAME, 2272, Amissl)

#define CAST_ofb64_encrypt(___in, ___out, ___length, ___schedule, ___ivec, ___num) \
	AROS_LC6(void, CAST_ofb64_encrypt, \
	AROS_LCA(const unsigned char *, (___in), A0), \
	AROS_LCA(unsigned char *, (___out), A1), \
	AROS_LCA(long, (___length), D0), \
	AROS_LCA(CAST_KEY *, (___schedule), A2), \
	AROS_LCA(unsigned char *, (___ivec), A3), \
	AROS_LCA(int *, (___num), D1), \
	struct Library *, AMISSL_BASE_NAME, 2273, Amissl)

#define _shadow_DES_check_key() \
	AROS_LC0(int *, _shadow_DES_check_key, \
	struct Library *, AMISSL_BASE_NAME, 2274, Amissl)

#define _shadow_DES_rw_mode() \
	AROS_LC0(int *, _shadow_DES_rw_mode, \
	struct Library *, AMISSL_BASE_NAME, 2275, Amissl)

#define DES_options() \
	AROS_LC0(const char *, DES_options, \
	struct Library *, AMISSL_BASE_NAME, 2276, Amissl)

#define DES_ecb3_encrypt(___input, ___output, ___ks1, ___ks2, ___ks3, ___enc) \
	AROS_LC6(void, DES_ecb3_encrypt, \
	AROS_LCA(const unsigned char *, (___input), A0), \
	AROS_LCA(unsigned char *, (___output), A1), \
	AROS_LCA(DES_key_schedule *, (___ks1), A2), \
	AROS_LCA(DES_key_schedule *, (___ks2), A3), \
	AROS_LCA(DES_key_schedule *, (___ks3), D0), \
	AROS_LCA(int, (___enc), D1), \
	struct Library *, AMISSL_BASE_NAME, 2277, Amissl)

#define DES_cbc_cksum(___input, ___output, ___length, ___schedule, ___ivec) \
	AROS_LC5(DES_LONG, DES_cbc_cksum, \
	AROS_LCA(const unsigned char *, (___input), A0), \
	AROS_LCA(DES_cblock *, (___output), A1), \
	AROS_LCA(long, (___length), D0), \
	AROS_LCA(DES_key_schedule *, (___schedule), A2), \
	AROS_LCA(const_DES_cblock *, (___ivec), A3), \
	struct Library *, AMISSL_BASE_NAME, 2278, Amissl)

#define DES_cbc_encrypt(___input, ___output, ___length, ___schedule, ___ivec, ___enc) \
	AROS_LC6(void, DES_cbc_encrypt, \
	AROS_LCA(const unsigned char *, (___input), A0), \
	AROS_LCA(unsigned char *, (___output), A1), \
	AROS_LCA(long, (___length), D0), \
	AROS_LCA(DES_key_schedule *, (___schedule), A2), \
	AROS_LCA(DES_cblock *, (___ivec), A3), \
	AROS_LCA(int, (___enc), D1), \
	struct Library *, AMISSL_BASE_NAME, 2279, Amissl)

#define DES_ncbc_encrypt(___input, ___output, ___length, ___schedule, ___ivec, ___enc) \
	AROS_LC6(void, DES_ncbc_encrypt, \
	AROS_LCA(const unsigned char *, (___input), A0), \
	AROS_LCA(unsigned char *, (___output), A1), \
	AROS_LCA(long, (___length), D0), \
	AROS_LCA(DES_key_schedule *, (___schedule), A2), \
	AROS_LCA(DES_cblock *, (___ivec), A3), \
	AROS_LCA(int, (___enc), D1), \
	struct Library *, AMISSL_BASE_NAME, 2280, Amissl)

#define DES_xcbc_encrypt(___input, ___output, ___length, ___schedule, ___ivec, ___inw, ___outw, ___enc) \
	AROS_LC8(void, DES_xcbc_encrypt, \
	AROS_LCA(const unsigned char *, (___input), A0), \
	AROS_LCA(unsigned char *, (___output), A1), \
	AROS_LCA(long, (___length), D0), \
	AROS_LCA(DES_key_schedule *, (___schedule), A2), \
	AROS_LCA(DES_cblock *, (___ivec), A3), \
	AROS_LCA(const_DES_cblock *, (___inw), D1), \
	AROS_LCA(const_DES_cblock *, (___outw), D2), \
	AROS_LCA(int, (___enc), D3), \
	struct Library *, AMISSL_BASE_NAME, 2281, Amissl)

#define DES_cfb_encrypt(___in, ___out, ___numbits, ___length, ___schedule, ___ivec, ___enc) \
	AROS_LC7(void, DES_cfb_encrypt, \
	AROS_LCA(const unsigned char *, (___in), A0), \
	AROS_LCA(unsigned char *, (___out), A1), \
	AROS_LCA(int, (___numbits), D0), \
	AROS_LCA(long, (___length), D1), \
	AROS_LCA(DES_key_schedule *, (___schedule), A2), \
	AROS_LCA(DES_cblock *, (___ivec), A3), \
	AROS_LCA(int, (___enc), D2), \
	struct Library *, AMISSL_BASE_NAME, 2282, Amissl)

#define DES_ecb_encrypt(___input, ___output, ___ks, ___enc) \
	AROS_LC4(void, DES_ecb_encrypt, \
	AROS_LCA(const_DES_cblock *, (___input), A0), \
	AROS_LCA(DES_cblock *, (___output), A1), \
	AROS_LCA(DES_key_schedule *, (___ks), A2), \
	AROS_LCA(int, (___enc), D0), \
	struct Library *, AMISSL_BASE_NAME, 2283, Amissl)

#define DES_encrypt1(___data, ___ks, ___enc) \
	AROS_LC3(void, DES_encrypt1, \
	AROS_LCA(DES_LONG *, (___data), A0), \
	AROS_LCA(DES_key_schedule *, (___ks), A1), \
	AROS_LCA(int, (___enc), D0), \
	struct Library *, AMISSL_BASE_NAME, 2284, Amissl)

#define DES_encrypt2(___data, ___ks, ___enc) \
	AROS_LC3(void, DES_encrypt2, \
	AROS_LCA(DES_LONG *, (___data), A0), \
	AROS_LCA(DES_key_schedule *, (___ks), A1), \
	AROS_LCA(int, (___enc), D0), \
	struct Library *, AMISSL_BASE_NAME, 2285, Amissl)

#define DES_encrypt3(___data, ___ks1, ___ks2, ___ks3) \
	AROS_LC4(void, DES_encrypt3, \
	AROS_LCA(DES_LONG *, (___data), A0), \
	AROS_LCA(DES_key_schedule *, (___ks1), A1), \
	AROS_LCA(DES_key_schedule *, (___ks2), A2), \
	AROS_LCA(DES_key_schedule *, (___ks3), A3), \
	struct Library *, AMISSL_BASE_NAME, 2286, Amissl)

#define DES_decrypt3(___data, ___ks1, ___ks2, ___ks3) \
	AROS_LC4(void, DES_decrypt3, \
	AROS_LCA(DES_LONG *, (___data), A0), \
	AROS_LCA(DES_key_schedule *, (___ks1), A1), \
	AROS_LCA(DES_key_schedule *, (___ks2), A2), \
	AROS_LCA(DES_key_schedule *, (___ks3), A3), \
	struct Library *, AMISSL_BASE_NAME, 2287, Amissl)

#define DES_ede3_cbc_encrypt(___input, ___output, ___length, ___ks1, ___ks2, ___ks3, ___ivec, ___enc) \
	AROS_LC8(void, DES_ede3_cbc_encrypt, \
	AROS_LCA(const unsigned char *, (___input), A0), \
	AROS_LCA(unsigned char *, (___output), A1), \
	AROS_LCA(long, (___length), D0), \
	AROS_LCA(DES_key_schedule *, (___ks1), A2), \
	AROS_LCA(DES_key_schedule *, (___ks2), A3), \
	AROS_LCA(DES_key_schedule *, (___ks3), D1), \
	AROS_LCA(DES_cblock *, (___ivec), D2), \
	AROS_LCA(int, (___enc), D3), \
	struct Library *, AMISSL_BASE_NAME, 2288, Amissl)

#define DES_ede3_cbcm_encrypt(___in, ___out, ___length, ___ks1, ___ks2, ___ks3, ___ivec1, ___ivec2, ___enc) \
	AROS_LC9(void, DES_ede3_cbcm_encrypt, \
	AROS_LCA(const unsigned char *, (___in), A0), \
	AROS_LCA(unsigned char *, (___out), A1), \
	AROS_LCA(long, (___length), D0), \
	AROS_LCA(DES_key_schedule *, (___ks1), A2), \
	AROS_LCA(DES_key_schedule *, (___ks2), A3), \
	AROS_LCA(DES_key_schedule *, (___ks3), D1), \
	AROS_LCA(DES_cblock *, (___ivec1), D2), \
	AROS_LCA(DES_cblock *, (___ivec2), D3), \
	AROS_LCA(int, (___enc), D4), \
	struct Library *, AMISSL_BASE_NAME, 2289, Amissl)

#define DES_ede3_cfb64_encrypt(___in, ___out, ___length, ___ks1, ___ks2, ___ks3, ___ivec, ___num, ___enc) \
	AROS_LC9(void, DES_ede3_cfb64_encrypt, \
	AROS_LCA(const unsigned char *, (___in), A0), \
	AROS_LCA(unsigned char *, (___out), A1), \
	AROS_LCA(long, (___length), D0), \
	AROS_LCA(DES_key_schedule *, (___ks1), A2), \
	AROS_LCA(DES_key_schedule *, (___ks2), A3), \
	AROS_LCA(DES_key_schedule *, (___ks3), D1), \
	AROS_LCA(DES_cblock *, (___ivec), D2), \
	AROS_LCA(int *, (___num), D3), \
	AROS_LCA(int, (___enc), D4), \
	struct Library *, AMISSL_BASE_NAME, 2290, Amissl)

#define DES_ede3_cfb_encrypt(___in, ___out, ___numbits, ___length, ___ks1, ___ks2, ___ks3, ___ivec, ___enc) \
	AROS_LC9(void, DES_ede3_cfb_encrypt, \
	AROS_LCA(const unsigned char *, (___in), A0), \
	AROS_LCA(unsigned char *, (___out), A1), \
	AROS_LCA(int, (___numbits), D0), \
	AROS_LCA(long, (___length), D1), \
	AROS_LCA(DES_key_schedule *, (___ks1), A2), \
	AROS_LCA(DES_key_schedule *, (___ks2), A3), \
	AROS_LCA(DES_key_schedule *, (___ks3), D2), \
	AROS_LCA(DES_cblock *, (___ivec), D3), \
	AROS_LCA(int, (___enc), D4), \
	struct Library *, AMISSL_BASE_NAME, 2291, Amissl)

#define DES_ede3_ofb64_encrypt(___in, ___out, ___length, ___ks1, ___ks2, ___ks3, ___ivec, ___num) \
	AROS_LC8(void, DES_ede3_ofb64_encrypt, \
	AROS_LCA(const unsigned char *, (___in), A0), \
	AROS_LCA(unsigned char *, (___out), A1), \
	AROS_LCA(long, (___length), D0), \
	AROS_LCA(DES_key_schedule *, (___ks1), A2), \
	AROS_LCA(DES_key_schedule *, (___ks2), A3), \
	AROS_LCA(DES_key_schedule *, (___ks3), D1), \
	AROS_LCA(DES_cblock *, (___ivec), D2), \
	AROS_LCA(int *, (___num), D3), \
	struct Library *, AMISSL_BASE_NAME, 2292, Amissl)

#define DES_xwhite_in2out(___DES_key, ___in_white, ___out_white) \
	AROS_LC3(void, DES_xwhite_in2out, \
	AROS_LCA(const_DES_cblock *, (___DES_key), A0), \
	AROS_LCA(const_DES_cblock *, (___in_white), A1), \
	AROS_LCA(DES_cblock *, (___out_white), A2), \
	struct Library *, AMISSL_BASE_NAME, 2293, Amissl)

#define DES_enc_read(___fd, ___buf, ___len, ___sched, ___iv) \
	AROS_LC5(int, DES_enc_read, \
	AROS_LCA(int, (___fd), D0), \
	AROS_LCA(void *, (___buf), A0), \
	AROS_LCA(int, (___len), D1), \
	AROS_LCA(DES_key_schedule *, (___sched), A1), \
	AROS_LCA(DES_cblock *, (___iv), A2), \
	struct Library *, AMISSL_BASE_NAME, 2294, Amissl)

#define DES_enc_write(___fd, ___buf, ___len, ___sched, ___iv) \
	AROS_LC5(int, DES_enc_write, \
	AROS_LCA(int, (___fd), D0), \
	AROS_LCA(const void *, (___buf), A0), \
	AROS_LCA(int, (___len), D1), \
	AROS_LCA(DES_key_schedule *, (___sched), A1), \
	AROS_LCA(DES_cblock *, (___iv), A2), \
	struct Library *, AMISSL_BASE_NAME, 2295, Amissl)

#define DES_fcrypt(___buf, ___salt, ___ret) \
	AROS_LC3(char *, DES_fcrypt, \
	AROS_LCA(const char *, (___buf), A0), \
	AROS_LCA(const char *, (___salt), A1), \
	AROS_LCA(char *, (___ret), A2), \
	struct Library *, AMISSL_BASE_NAME, 2296, Amissl)

#define DES_crypt(___buf, ___salt) \
	AROS_LC2(char *, DES_crypt, \
	AROS_LCA(const char *, (___buf), A0), \
	AROS_LCA(const char *, (___salt), A1), \
	struct Library *, AMISSL_BASE_NAME, 2297, Amissl)

#define DES_ofb_encrypt(___in, ___out, ___numbits, ___length, ___schedule, ___ivec) \
	AROS_LC6(void, DES_ofb_encrypt, \
	AROS_LCA(const unsigned char *, (___in), A0), \
	AROS_LCA(unsigned char *, (___out), A1), \
	AROS_LCA(int, (___numbits), D0), \
	AROS_LCA(long, (___length), D1), \
	AROS_LCA(DES_key_schedule *, (___schedule), A2), \
	AROS_LCA(DES_cblock *, (___ivec), A3), \
	struct Library *, AMISSL_BASE_NAME, 2298, Amissl)

#define DES_pcbc_encrypt(___input, ___output, ___length, ___schedule, ___ivec, ___enc) \
	AROS_LC6(void, DES_pcbc_encrypt, \
	AROS_LCA(const unsigned char *, (___input), A0), \
	AROS_LCA(unsigned char *, (___output), A1), \
	AROS_LCA(long, (___length), D0), \
	AROS_LCA(DES_key_schedule *, (___schedule), A2), \
	AROS_LCA(DES_cblock *, (___ivec), A3), \
	AROS_LCA(int, (___enc), D1), \
	struct Library *, AMISSL_BASE_NAME, 2299, Amissl)

#define DES_quad_cksum(___input, ___output, ___length, ___out_count, ___seed) \
	AROS_LC5(DES_LONG, DES_quad_cksum, \
	AROS_LCA(const unsigned char *, (___input), A0), \
	AROS_LCA(DES_cblock *, (___output), A1), \
	AROS_LCA(long, (___length), D0), \
	AROS_LCA(int, (___out_count), D1), \
	AROS_LCA(DES_cblock *, (___seed), A2), \
	struct Library *, AMISSL_BASE_NAME, 2300, Amissl)

#define DES_random_key(___ret) \
	AROS_LC1(int, DES_random_key, \
	AROS_LCA(DES_cblock *, (___ret), A0), \
	struct Library *, AMISSL_BASE_NAME, 2301, Amissl)

#define DES_set_odd_parity(___key) \
	AROS_LC1(void, DES_set_odd_parity, \
	AROS_LCA(DES_cblock *, (___key), A0), \
	struct Library *, AMISSL_BASE_NAME, 2302, Amissl)

#define DES_check_key_parity(___key) \
	AROS_LC1(int, DES_check_key_parity, \
	AROS_LCA(const_DES_cblock *, (___key), A0), \
	struct Library *, AMISSL_BASE_NAME, 2303, Amissl)

#define DES_is_weak_key(___key) \
	AROS_LC1(int, DES_is_weak_key, \
	AROS_LCA(const_DES_cblock *, (___key), A0), \
	struct Library *, AMISSL_BASE_NAME, 2304, Amissl)

#define DES_set_key(___key, ___schedule) \
	AROS_LC2(int, DES_set_key, \
	AROS_LCA(const_DES_cblock *, (___key), A0), \
	AROS_LCA(DES_key_schedule *, (___schedule), A1), \
	struct Library *, AMISSL_BASE_NAME, 2305, Amissl)

#define DES_key_sched(___key, ___schedule) \
	AROS_LC2(int, DES_key_sched, \
	AROS_LCA(const_DES_cblock *, (___key), A0), \
	AROS_LCA(DES_key_schedule *, (___schedule), A1), \
	struct Library *, AMISSL_BASE_NAME, 2306, Amissl)

#define DES_set_key_checked(___key, ___schedule) \
	AROS_LC2(int, DES_set_key_checked, \
	AROS_LCA(const_DES_cblock *, (___key), A0), \
	AROS_LCA(DES_key_schedule *, (___schedule), A1), \
	struct Library *, AMISSL_BASE_NAME, 2307, Amissl)

#define DES_set_key_unchecked(___key, ___schedule) \
	AROS_LC2(void, DES_set_key_unchecked, \
	AROS_LCA(const_DES_cblock *, (___key), A0), \
	AROS_LCA(DES_key_schedule *, (___schedule), A1), \
	struct Library *, AMISSL_BASE_NAME, 2308, Amissl)

#define DES_string_to_key(___str, ___key) \
	AROS_LC2(void, DES_string_to_key, \
	AROS_LCA(const char *, (___str), A0), \
	AROS_LCA(DES_cblock *, (___key), A1), \
	struct Library *, AMISSL_BASE_NAME, 2309, Amissl)

#define DES_string_to_2keys(___str, ___key1, ___key2) \
	AROS_LC3(void, DES_string_to_2keys, \
	AROS_LCA(const char *, (___str), A0), \
	AROS_LCA(DES_cblock *, (___key1), A1), \
	AROS_LCA(DES_cblock *, (___key2), A2), \
	struct Library *, AMISSL_BASE_NAME, 2310, Amissl)

#define DES_cfb64_encrypt(___in, ___out, ___length, ___schedule, ___ivec, ___num, ___enc) \
	AROS_LC7(void, DES_cfb64_encrypt, \
	AROS_LCA(const unsigned char *, (___in), A0), \
	AROS_LCA(unsigned char *, (___out), A1), \
	AROS_LCA(long, (___length), D0), \
	AROS_LCA(DES_key_schedule *, (___schedule), A2), \
	AROS_LCA(DES_cblock *, (___ivec), A3), \
	AROS_LCA(int *, (___num), D1), \
	AROS_LCA(int, (___enc), D2), \
	struct Library *, AMISSL_BASE_NAME, 2311, Amissl)

#define DES_ofb64_encrypt(___in, ___out, ___length, ___schedule, ___ivec, ___num) \
	AROS_LC6(void, DES_ofb64_encrypt, \
	AROS_LCA(const unsigned char *, (___in), A0), \
	AROS_LCA(unsigned char *, (___out), A1), \
	AROS_LCA(long, (___length), D0), \
	AROS_LCA(DES_key_schedule *, (___schedule), A2), \
	AROS_LCA(DES_cblock *, (___ivec), A3), \
	AROS_LCA(int *, (___num), D1), \
	struct Library *, AMISSL_BASE_NAME, 2312, Amissl)

#define DES_read_password(___key, ___prompt, ___verify) \
	AROS_LC3(int, DES_read_password, \
	AROS_LCA(DES_cblock *, (___key), A0), \
	AROS_LCA(const char *, (___prompt), A1), \
	AROS_LCA(int, (___verify), D0), \
	struct Library *, AMISSL_BASE_NAME, 2313, Amissl)

#define DES_read_2passwords(___key1, ___key2, ___prompt, ___verify) \
	AROS_LC4(int, DES_read_2passwords, \
	AROS_LCA(DES_cblock *, (___key1), A0), \
	AROS_LCA(DES_cblock *, (___key2), A1), \
	AROS_LCA(const char *, (___prompt), A2), \
	AROS_LCA(int, (___verify), D0), \
	struct Library *, AMISSL_BASE_NAME, 2314, Amissl)

#define _ossl_old_des_options() \
	AROS_LC0(const char *, _ossl_old_des_options, \
	struct Library *, AMISSL_BASE_NAME, 2315, Amissl)

#define _ossl_old_des_ecb3_encrypt(___input, ___output, ___ks1, ___ks2, ___ks3, ___enc) \
	AROS_LC6(void, _ossl_old_des_ecb3_encrypt, \
	AROS_LCA(_ossl_old_des_cblock *, (___input), A0), \
	AROS_LCA(_ossl_old_des_cblock *, (___output), A1), \
	AROS_LCA(_ossl_old_des_key_schedule, (___ks1), D0), \
	AROS_LCA(_ossl_old_des_key_schedule, (___ks2), D1), \
	AROS_LCA(_ossl_old_des_key_schedule, (___ks3), D2), \
	AROS_LCA(int, (___enc), D3), \
	struct Library *, AMISSL_BASE_NAME, 2316, Amissl)

#define _ossl_old_des_cbc_cksum(___input, ___output, ___length, ___schedule, ___ivec) \
	AROS_LC5(DES_LONG, _ossl_old_des_cbc_cksum, \
	AROS_LCA(_ossl_old_des_cblock *, (___input), A0), \
	AROS_LCA(_ossl_old_des_cblock *, (___output), A1), \
	AROS_LCA(long, (___length), D0), \
	AROS_LCA(_ossl_old_des_key_schedule, (___schedule), D1), \
	AROS_LCA(_ossl_old_des_cblock *, (___ivec), A2), \
	struct Library *, AMISSL_BASE_NAME, 2317, Amissl)

#define _ossl_old_des_cbc_encrypt(___input, ___output, ___length, ___schedule, ___ivec, ___enc) \
	AROS_LC6(void, _ossl_old_des_cbc_encrypt, \
	AROS_LCA(_ossl_old_des_cblock *, (___input), A0), \
	AROS_LCA(_ossl_old_des_cblock *, (___output), A1), \
	AROS_LCA(long, (___length), D0), \
	AROS_LCA(_ossl_old_des_key_schedule, (___schedule), D1), \
	AROS_LCA(_ossl_old_des_cblock *, (___ivec), A2), \
	AROS_LCA(int, (___enc), D2), \
	struct Library *, AMISSL_BASE_NAME, 2318, Amissl)

#define _ossl_old_des_ncbc_encrypt(___input, ___output, ___length, ___schedule, ___ivec, ___enc) \
	AROS_LC6(void, _ossl_old_des_ncbc_encrypt, \
	AROS_LCA(_ossl_old_des_cblock *, (___input), A0), \
	AROS_LCA(_ossl_old_des_cblock *, (___output), A1), \
	AROS_LCA(long, (___length), D0), \
	AROS_LCA(_ossl_old_des_key_schedule, (___schedule), D1), \
	AROS_LCA(_ossl_old_des_cblock *, (___ivec), A2), \
	AROS_LCA(int, (___enc), D2), \
	struct Library *, AMISSL_BASE_NAME, 2319, Amissl)

#define _ossl_old_des_xcbc_encrypt(___input, ___output, ___length, ___schedule, ___ivec, ___inw, ___outw, ___enc) \
	AROS_LC8(void, _ossl_old_des_xcbc_encrypt, \
	AROS_LCA(_ossl_old_des_cblock *, (___input), A0), \
	AROS_LCA(_ossl_old_des_cblock *, (___output), A1), \
	AROS_LCA(long, (___length), D0), \
	AROS_LCA(_ossl_old_des_key_schedule, (___schedule), D1), \
	AROS_LCA(_ossl_old_des_cblock *, (___ivec), A2), \
	AROS_LCA(_ossl_old_des_cblock *, (___inw), A3), \
	AROS_LCA(_ossl_old_des_cblock *, (___outw), D2), \
	AROS_LCA(int, (___enc), D3), \
	struct Library *, AMISSL_BASE_NAME, 2320, Amissl)

#define _ossl_old_des_cfb_encrypt(___in, ___out, ___numbits, ___length, ___schedule, ___ivec, ___enc) \
	AROS_LC7(void, _ossl_old_des_cfb_encrypt, \
	AROS_LCA(unsigned char *, (___in), A0), \
	AROS_LCA(unsigned char *, (___out), A1), \
	AROS_LCA(int, (___numbits), D0), \
	AROS_LCA(long, (___length), D1), \
	AROS_LCA(_ossl_old_des_key_schedule, (___schedule), D2), \
	AROS_LCA(_ossl_old_des_cblock *, (___ivec), A2), \
	AROS_LCA(int, (___enc), D3), \
	struct Library *, AMISSL_BASE_NAME, 2321, Amissl)

#define _ossl_old_des_ecb_encrypt(___input, ___output, ___ks, ___enc) \
	AROS_LC4(void, _ossl_old_des_ecb_encrypt, \
	AROS_LCA(_ossl_old_des_cblock *, (___input), A0), \
	AROS_LCA(_ossl_old_des_cblock *, (___output), A1), \
	AROS_LCA(_ossl_old_des_key_schedule, (___ks), D0), \
	AROS_LCA(int, (___enc), D1), \
	struct Library *, AMISSL_BASE_NAME, 2322, Amissl)

#define _ossl_old_des_encrypt(___data, ___ks, ___enc) \
	AROS_LC3(void, _ossl_old_des_encrypt, \
	AROS_LCA(DES_LONG *, (___data), A0), \
	AROS_LCA(_ossl_old_des_key_schedule, (___ks), D0), \
	AROS_LCA(int, (___enc), D1), \
	struct Library *, AMISSL_BASE_NAME, 2323, Amissl)

#define _ossl_old_des_encrypt2(___data, ___ks, ___enc) \
	AROS_LC3(void, _ossl_old_des_encrypt2, \
	AROS_LCA(DES_LONG *, (___data), A0), \
	AROS_LCA(_ossl_old_des_key_schedule, (___ks), D0), \
	AROS_LCA(int, (___enc), D1), \
	struct Library *, AMISSL_BASE_NAME, 2324, Amissl)

#define _ossl_old_des_encrypt3(___data, ___ks1, ___ks2, ___ks3) \
	AROS_LC4(void, _ossl_old_des_encrypt3, \
	AROS_LCA(DES_LONG *, (___data), A0), \
	AROS_LCA(_ossl_old_des_key_schedule, (___ks1), D0), \
	AROS_LCA(_ossl_old_des_key_schedule, (___ks2), D1), \
	AROS_LCA(_ossl_old_des_key_schedule, (___ks3), D2), \
	struct Library *, AMISSL_BASE_NAME, 2325, Amissl)

#define _ossl_old_des_decrypt3(___data, ___ks1, ___ks2, ___ks3) \
	AROS_LC4(void, _ossl_old_des_decrypt3, \
	AROS_LCA(DES_LONG *, (___data), A0), \
	AROS_LCA(_ossl_old_des_key_schedule, (___ks1), D0), \
	AROS_LCA(_ossl_old_des_key_schedule, (___ks2), D1), \
	AROS_LCA(_ossl_old_des_key_schedule, (___ks3), D2), \
	struct Library *, AMISSL_BASE_NAME, 2326, Amissl)

#define _ossl_old_des_ede3_cbc_encrypt(___input, ___output, ___length, ___ks1, ___ks2, ___ks3, ___ivec, ___enc) \
	AROS_LC8(void, _ossl_old_des_ede3_cbc_encrypt, \
	AROS_LCA(_ossl_old_des_cblock *, (___input), A0), \
	AROS_LCA(_ossl_old_des_cblock *, (___output), A1), \
	AROS_LCA(long, (___length), D0), \
	AROS_LCA(_ossl_old_des_key_schedule, (___ks1), D1), \
	AROS_LCA(_ossl_old_des_key_schedule, (___ks2), D2), \
	AROS_LCA(_ossl_old_des_key_schedule, (___ks3), D3), \
	AROS_LCA(_ossl_old_des_cblock *, (___ivec), A2), \
	AROS_LCA(int, (___enc), D4), \
	struct Library *, AMISSL_BASE_NAME, 2327, Amissl)

#define _ossl_old_des_ede3_cfb64_encrypt(___in, ___out, ___length, ___ks1, ___ks2, ___ks3, ___ivec, ___num, ___enc) \
	AROS_LC9(void, _ossl_old_des_ede3_cfb64_encrypt, \
	AROS_LCA(unsigned char *, (___in), A0), \
	AROS_LCA(unsigned char *, (___out), A1), \
	AROS_LCA(long, (___length), D0), \
	AROS_LCA(_ossl_old_des_key_schedule, (___ks1), D1), \
	AROS_LCA(_ossl_old_des_key_schedule, (___ks2), D2), \
	AROS_LCA(_ossl_old_des_key_schedule, (___ks3), D3), \
	AROS_LCA(_ossl_old_des_cblock *, (___ivec), A2), \
	AROS_LCA(int *, (___num), A3), \
	AROS_LCA(int, (___enc), D4), \
	struct Library *, AMISSL_BASE_NAME, 2328, Amissl)

#define _ossl_old_des_ede3_ofb64_encrypt(___in, ___out, ___length, ___ks1, ___ks2, ___ks3, ___ivec, ___num) \
	AROS_LC8(void, _ossl_old_des_ede3_ofb64_encrypt, \
	AROS_LCA(unsigned char *, (___in), A0), \
	AROS_LCA(unsigned char *, (___out), A1), \
	AROS_LCA(long, (___length), D0), \
	AROS_LCA(_ossl_old_des_key_schedule, (___ks1), D1), \
	AROS_LCA(_ossl_old_des_key_schedule, (___ks2), D2), \
	AROS_LCA(_ossl_old_des_key_schedule, (___ks3), D3), \
	AROS_LCA(_ossl_old_des_cblock *, (___ivec), A2), \
	AROS_LCA(int *, (___num), A3), \
	struct Library *, AMISSL_BASE_NAME, 2329, Amissl)

#define _ossl_old_des_xwhite_in2out(___des_key, ___in_white, ___out_white) \
	AROS_LC3(void, _ossl_old_des_xwhite_in2out, \
	AROS_LCA(_ossl_old_des_cblock *, (___des_key), A0), \
	AROS_LCA(_ossl_old_des_cblock *, (___in_white), A1), \
	AROS_LCA(_ossl_old_des_cblock *, (___out_white), A2), \
	struct Library *, AMISSL_BASE_NAME, 2330, Amissl)

#define _ossl_old_des_enc_read(___fd, ___buf, ___len, ___sched, ___iv) \
	AROS_LC5(int, _ossl_old_des_enc_read, \
	AROS_LCA(int, (___fd), D0), \
	AROS_LCA(char *, (___buf), A0), \
	AROS_LCA(int, (___len), D1), \
	AROS_LCA(_ossl_old_des_key_schedule, (___sched), D2), \
	AROS_LCA(_ossl_old_des_cblock *, (___iv), A1), \
	struct Library *, AMISSL_BASE_NAME, 2331, Amissl)

#define _ossl_old_des_enc_write(___fd, ___buf, ___len, ___sched, ___iv) \
	AROS_LC5(int, _ossl_old_des_enc_write, \
	AROS_LCA(int, (___fd), D0), \
	AROS_LCA(char *, (___buf), A0), \
	AROS_LCA(int, (___len), D1), \
	AROS_LCA(_ossl_old_des_key_schedule, (___sched), D2), \
	AROS_LCA(_ossl_old_des_cblock *, (___iv), A1), \
	struct Library *, AMISSL_BASE_NAME, 2332, Amissl)

#define _ossl_old_des_fcrypt(___buf, ___salt, ___ret) \
	AROS_LC3(char *, _ossl_old_des_fcrypt, \
	AROS_LCA(const char *, (___buf), A0), \
	AROS_LCA(const char *, (___salt), A1), \
	AROS_LCA(char *, (___ret), A2), \
	struct Library *, AMISSL_BASE_NAME, 2333, Amissl)

#define _ossl_old_des_crypt(___buf, ___salt) \
	AROS_LC2(char *, _ossl_old_des_crypt, \
	AROS_LCA(const char *, (___buf), A0), \
	AROS_LCA(const char *, (___salt), A1), \
	struct Library *, AMISSL_BASE_NAME, 2334, Amissl)

#define _ossl_old_crypt(___buf, ___salt) \
	AROS_LC2(char *, _ossl_old_crypt, \
	AROS_LCA(const char *, (___buf), A0), \
	AROS_LCA(const char *, (___salt), A1), \
	struct Library *, AMISSL_BASE_NAME, 2335, Amissl)

#define _ossl_old_des_ofb_encrypt(___in, ___out, ___numbits, ___length, ___schedule, ___ivec) \
	AROS_LC6(void, _ossl_old_des_ofb_encrypt, \
	AROS_LCA(unsigned char *, (___in), A0), \
	AROS_LCA(unsigned char *, (___out), A1), \
	AROS_LCA(int, (___numbits), D0), \
	AROS_LCA(long, (___length), D1), \
	AROS_LCA(_ossl_old_des_key_schedule, (___schedule), D2), \
	AROS_LCA(_ossl_old_des_cblock *, (___ivec), A2), \
	struct Library *, AMISSL_BASE_NAME, 2336, Amissl)

#define _ossl_old_des_pcbc_encrypt(___input, ___output, ___length, ___schedule, ___ivec, ___enc) \
	AROS_LC6(void, _ossl_old_des_pcbc_encrypt, \
	AROS_LCA(_ossl_old_des_cblock *, (___input), A0), \
	AROS_LCA(_ossl_old_des_cblock *, (___output), A1), \
	AROS_LCA(long, (___length), D0), \
	AROS_LCA(_ossl_old_des_key_schedule, (___schedule), D1), \
	AROS_LCA(_ossl_old_des_cblock *, (___ivec), A2), \
	AROS_LCA(int, (___enc), D2), \
	struct Library *, AMISSL_BASE_NAME, 2337, Amissl)

#define _ossl_old_des_quad_cksum(___input, ___output, ___length, ___out_count, ___seed) \
	AROS_LC5(DES_LONG, _ossl_old_des_quad_cksum, \
	AROS_LCA(_ossl_old_des_cblock *, (___input), A0), \
	AROS_LCA(_ossl_old_des_cblock *, (___output), A1), \
	AROS_LCA(long, (___length), D0), \
	AROS_LCA(int, (___out_count), D1), \
	AROS_LCA(_ossl_old_des_cblock *, (___seed), A2), \
	struct Library *, AMISSL_BASE_NAME, 2338, Amissl)

#define _ossl_old_des_random_seed(___key) \
	AROS_LC1(void, _ossl_old_des_random_seed, \
	AROS_LCA(_ossl_old_des_cblock, (___key), D0), \
	struct Library *, AMISSL_BASE_NAME, 2339, Amissl)

#define _ossl_old_des_random_key(___ret) \
	AROS_LC1(void, _ossl_old_des_random_key, \
	AROS_LCA(_ossl_old_des_cblock, (___ret), D0), \
	struct Library *, AMISSL_BASE_NAME, 2340, Amissl)

#define _ossl_old_des_read_password(___key, ___prompt, ___verify) \
	AROS_LC3(int, _ossl_old_des_read_password, \
	AROS_LCA(_ossl_old_des_cblock *, (___key), A0), \
	AROS_LCA(const char *, (___prompt), A1), \
	AROS_LCA(int, (___verify), D0), \
	struct Library *, AMISSL_BASE_NAME, 2341, Amissl)

#define _ossl_old_des_read_2passwords(___key1, ___key2, ___prompt, ___verify) \
	AROS_LC4(int, _ossl_old_des_read_2passwords, \
	AROS_LCA(_ossl_old_des_cblock *, (___key1), A0), \
	AROS_LCA(_ossl_old_des_cblock *, (___key2), A1), \
	AROS_LCA(const char *, (___prompt), A2), \
	AROS_LCA(int, (___verify), D0), \
	struct Library *, AMISSL_BASE_NAME, 2342, Amissl)

#define _ossl_old_des_set_odd_parity(___key) \
	AROS_LC1(void, _ossl_old_des_set_odd_parity, \
	AROS_LCA(_ossl_old_des_cblock *, (___key), A0), \
	struct Library *, AMISSL_BASE_NAME, 2343, Amissl)

#define _ossl_old_des_is_weak_key(___key) \
	AROS_LC1(int, _ossl_old_des_is_weak_key, \
	AROS_LCA(_ossl_old_des_cblock *, (___key), A0), \
	struct Library *, AMISSL_BASE_NAME, 2344, Amissl)

#define _ossl_old_des_set_key(___key, ___schedule) \
	AROS_LC2(int, _ossl_old_des_set_key, \
	AROS_LCA(_ossl_old_des_cblock *, (___key), A0), \
	AROS_LCA(_ossl_old_des_key_schedule, (___schedule), D0), \
	struct Library *, AMISSL_BASE_NAME, 2345, Amissl)

#define _ossl_old_des_key_sched(___key, ___schedule) \
	AROS_LC2(int, _ossl_old_des_key_sched, \
	AROS_LCA(_ossl_old_des_cblock *, (___key), A0), \
	AROS_LCA(_ossl_old_des_key_schedule, (___schedule), D0), \
	struct Library *, AMISSL_BASE_NAME, 2346, Amissl)

#define _ossl_old_des_string_to_key(___str, ___key) \
	AROS_LC2(void, _ossl_old_des_string_to_key, \
	AROS_LCA(char *, (___str), A0), \
	AROS_LCA(_ossl_old_des_cblock *, (___key), A1), \
	struct Library *, AMISSL_BASE_NAME, 2347, Amissl)

#define _ossl_old_des_string_to_2keys(___str, ___key1, ___key2) \
	AROS_LC3(void, _ossl_old_des_string_to_2keys, \
	AROS_LCA(char *, (___str), A0), \
	AROS_LCA(_ossl_old_des_cblock *, (___key1), A1), \
	AROS_LCA(_ossl_old_des_cblock *, (___key2), A2), \
	struct Library *, AMISSL_BASE_NAME, 2348, Amissl)

#define _ossl_old_des_cfb64_encrypt(___in, ___out, ___length, ___schedule, ___ivec, ___num, ___enc) \
	AROS_LC7(void, _ossl_old_des_cfb64_encrypt, \
	AROS_LCA(unsigned char *, (___in), A0), \
	AROS_LCA(unsigned char *, (___out), A1), \
	AROS_LCA(long, (___length), D0), \
	AROS_LCA(_ossl_old_des_key_schedule, (___schedule), D1), \
	AROS_LCA(_ossl_old_des_cblock *, (___ivec), A2), \
	AROS_LCA(int *, (___num), A3), \
	AROS_LCA(int, (___enc), D2), \
	struct Library *, AMISSL_BASE_NAME, 2349, Amissl)

#define _ossl_old_des_ofb64_encrypt(___in, ___out, ___length, ___schedule, ___ivec, ___num) \
	AROS_LC6(void, _ossl_old_des_ofb64_encrypt, \
	AROS_LCA(unsigned char *, (___in), A0), \
	AROS_LCA(unsigned char *, (___out), A1), \
	AROS_LCA(long, (___length), D0), \
	AROS_LCA(_ossl_old_des_key_schedule, (___schedule), D1), \
	AROS_LCA(_ossl_old_des_cblock *, (___ivec), A2), \
	AROS_LCA(int *, (___num), A3), \
	struct Library *, AMISSL_BASE_NAME, 2350, Amissl)

#define _ossl_096_des_random_seed(___key) \
	AROS_LC1(void, _ossl_096_des_random_seed, \
	AROS_LCA(des_cblock *, (___key), A0), \
	struct Library *, AMISSL_BASE_NAME, 2351, Amissl)

#define DH_OpenSSL() \
	AROS_LC0(const DH_METHOD *, DH_OpenSSL, \
	struct Library *, AMISSL_BASE_NAME, 2352, Amissl)

#define DH_set_default_method(___meth) \
	AROS_LC1(void, DH_set_default_method, \
	AROS_LCA(const DH_METHOD *, (___meth), A0), \
	struct Library *, AMISSL_BASE_NAME, 2353, Amissl)

#define DH_get_default_method() \
	AROS_LC0(const DH_METHOD *, DH_get_default_method, \
	struct Library *, AMISSL_BASE_NAME, 2354, Amissl)

#define DH_set_method(___dh, ___meth) \
	AROS_LC2(int, DH_set_method, \
	AROS_LCA(DH *, (___dh), A0), \
	AROS_LCA(const DH_METHOD *, (___meth), A1), \
	struct Library *, AMISSL_BASE_NAME, 2355, Amissl)

#define DH_new_method(___engine) \
	AROS_LC1(DH *, DH_new_method, \
	AROS_LCA(ENGINE *, (___engine), A0), \
	struct Library *, AMISSL_BASE_NAME, 2356, Amissl)

#define DH_new() \
	AROS_LC0(DH *, DH_new, \
	struct Library *, AMISSL_BASE_NAME, 2357, Amissl)

#define DH_free(___dh) \
	AROS_LC1(void, DH_free, \
	AROS_LCA(DH *, (___dh), A0), \
	struct Library *, AMISSL_BASE_NAME, 2358, Amissl)

#define DH_up_ref(___dh) \
	AROS_LC1(int, DH_up_ref, \
	AROS_LCA(DH *, (___dh), A0), \
	struct Library *, AMISSL_BASE_NAME, 2359, Amissl)

#define DH_size(___dh) \
	AROS_LC1(int, DH_size, \
	AROS_LCA(const DH *, (___dh), A0), \
	struct Library *, AMISSL_BASE_NAME, 2360, Amissl)

#define DH_get_ex_new_index(___argl, ___argp, ___new_func, ___dup_func, ___free_func) \
	AROS_LC5(int, DH_get_ex_new_index, \
	AROS_LCA(long, (___argl), D0), \
	AROS_LCA(void *, (___argp), A0), \
	AROS_LCA(CRYPTO_EX_new * (*)(), (___new_func), A1), \
	AROS_LCA(CRYPTO_EX_dup * (*)(), (___dup_func), A2), \
	AROS_LCA(CRYPTO_EX_free * (*)(), (___free_func), A3), \
	struct Library *, AMISSL_BASE_NAME, 2361, Amissl)

#define DH_set_ex_data(___d, ___idx, ___arg) \
	AROS_LC3(int, DH_set_ex_data, \
	AROS_LCA(DH *, (___d), A0), \
	AROS_LCA(int, (___idx), D0), \
	AROS_LCA(void *, (___arg), A1), \
	struct Library *, AMISSL_BASE_NAME, 2362, Amissl)

#define DH_get_ex_data(___d, ___idx) \
	AROS_LC2(void *, DH_get_ex_data, \
	AROS_LCA(DH *, (___d), A0), \
	AROS_LCA(int, (___idx), D0), \
	struct Library *, AMISSL_BASE_NAME, 2363, Amissl)

#define DH_generate_parameters(___prime_len, ___generator, ___callback, ___cb_arg) \
	AROS_LC4(DH *, DH_generate_parameters, \
	AROS_LCA(int, (___prime_len), D0), \
	AROS_LCA(int, (___generator), D1), \
	AROS_LCA(void (*)(int,int,void *), (___callback), A0), \
	AROS_LCA(void *, (___cb_arg), A1), \
	struct Library *, AMISSL_BASE_NAME, 2364, Amissl)

#define DH_check(___dh, ___codes) \
	AROS_LC2(int, DH_check, \
	AROS_LCA(const DH *, (___dh), A0), \
	AROS_LCA(int *, (___codes), A1), \
	struct Library *, AMISSL_BASE_NAME, 2365, Amissl)

#define DH_generate_key(___dh) \
	AROS_LC1(int, DH_generate_key, \
	AROS_LCA(DH *, (___dh), A0), \
	struct Library *, AMISSL_BASE_NAME, 2366, Amissl)

#define DH_compute_key(___key, ___pub_key, ___dh) \
	AROS_LC3(int, DH_compute_key, \
	AROS_LCA(unsigned char *, (___key), A0), \
	AROS_LCA(const BIGNUM *, (___pub_key), A1), \
	AROS_LCA(DH *, (___dh), A2), \
	struct Library *, AMISSL_BASE_NAME, 2367, Amissl)

#define d2i_DHparams(___a, ___pp, ___length) \
	AROS_LC3(DH *, d2i_DHparams, \
	AROS_LCA(DH **, (___a), A0), \
	AROS_LCA(const unsigned char **, (___pp), A1), \
	AROS_LCA(long, (___length), D0), \
	struct Library *, AMISSL_BASE_NAME, 2368, Amissl)

#define i2d_DHparams(___a, ___pp) \
	AROS_LC2(int, i2d_DHparams, \
	AROS_LCA(const DH *, (___a), A0), \
	AROS_LCA(unsigned char **, (___pp), A1), \
	struct Library *, AMISSL_BASE_NAME, 2369, Amissl)

#define DHparams_print(___bp, ___x) \
	AROS_LC2(int, DHparams_print, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(const DH *, (___x), A1), \
	struct Library *, AMISSL_BASE_NAME, 2370, Amissl)

#define ERR_load_DH_strings() \
	AROS_LC0(void, ERR_load_DH_strings, \
	struct Library *, AMISSL_BASE_NAME, 2371, Amissl)

#define DSA_SIG_new() \
	AROS_LC0(DSA_SIG *, DSA_SIG_new, \
	struct Library *, AMISSL_BASE_NAME, 2372, Amissl)

#define DSA_SIG_free(___a) \
	AROS_LC1(void, DSA_SIG_free, \
	AROS_LCA(DSA_SIG *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 2373, Amissl)

#define i2d_DSA_SIG(___a, ___pp) \
	AROS_LC2(int, i2d_DSA_SIG, \
	AROS_LCA(const DSA_SIG *, (___a), A0), \
	AROS_LCA(unsigned char **, (___pp), A1), \
	struct Library *, AMISSL_BASE_NAME, 2374, Amissl)

#define d2i_DSA_SIG(___v, ___pp, ___length) \
	AROS_LC3(DSA_SIG *, d2i_DSA_SIG, \
	AROS_LCA(DSA_SIG **, (___v), A0), \
	AROS_LCA(const unsigned char **, (___pp), A1), \
	AROS_LCA(long, (___length), D0), \
	struct Library *, AMISSL_BASE_NAME, 2375, Amissl)

#define DSA_do_sign(___dgst, ___dlen, ___dsa) \
	AROS_LC3(DSA_SIG *, DSA_do_sign, \
	AROS_LCA(const unsigned char *, (___dgst), A0), \
	AROS_LCA(int, (___dlen), D0), \
	AROS_LCA(DSA *, (___dsa), A1), \
	struct Library *, AMISSL_BASE_NAME, 2376, Amissl)

#define DSA_do_verify(___dgst, ___dgst_len, ___sig, ___dsa) \
	AROS_LC4(int, DSA_do_verify, \
	AROS_LCA(const unsigned char *, (___dgst), A0), \
	AROS_LCA(int, (___dgst_len), D0), \
	AROS_LCA(DSA_SIG *, (___sig), A1), \
	AROS_LCA(DSA *, (___dsa), A2), \
	struct Library *, AMISSL_BASE_NAME, 2377, Amissl)

#define DSA_OpenSSL() \
	AROS_LC0(const DSA_METHOD *, DSA_OpenSSL, \
	struct Library *, AMISSL_BASE_NAME, 2378, Amissl)

#define DSA_set_default_method(___a) \
	AROS_LC1(void, DSA_set_default_method, \
	AROS_LCA(const DSA_METHOD *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 2379, Amissl)

#define DSA_get_default_method() \
	AROS_LC0(const DSA_METHOD *, DSA_get_default_method, \
	struct Library *, AMISSL_BASE_NAME, 2380, Amissl)

#define DSA_set_method(___dsa, ___a) \
	AROS_LC2(int, DSA_set_method, \
	AROS_LCA(DSA *, (___dsa), A0), \
	AROS_LCA(const DSA_METHOD *, (___a), A1), \
	struct Library *, AMISSL_BASE_NAME, 2381, Amissl)

#define DSA_new() \
	AROS_LC0(DSA *, DSA_new, \
	struct Library *, AMISSL_BASE_NAME, 2382, Amissl)

#define DSA_new_method(___engine) \
	AROS_LC1(DSA *, DSA_new_method, \
	AROS_LCA(ENGINE *, (___engine), A0), \
	struct Library *, AMISSL_BASE_NAME, 2383, Amissl)

#define DSA_free(___r) \
	AROS_LC1(void, DSA_free, \
	AROS_LCA(DSA *, (___r), A0), \
	struct Library *, AMISSL_BASE_NAME, 2384, Amissl)

#define DSA_up_ref(___r) \
	AROS_LC1(int, DSA_up_ref, \
	AROS_LCA(DSA *, (___r), A0), \
	struct Library *, AMISSL_BASE_NAME, 2385, Amissl)

#define DSA_size(___a) \
	AROS_LC1(int, DSA_size, \
	AROS_LCA(const DSA *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 2386, Amissl)

#define DSA_sign_setup(___dsa, ___ctx_in, ___kinvp, ___rp) \
	AROS_LC4(int, DSA_sign_setup, \
	AROS_LCA(DSA *, (___dsa), A0), \
	AROS_LCA(BN_CTX *, (___ctx_in), A1), \
	AROS_LCA(BIGNUM **, (___kinvp), A2), \
	AROS_LCA(BIGNUM **, (___rp), A3), \
	struct Library *, AMISSL_BASE_NAME, 2387, Amissl)

#define DSA_sign(___type, ___dgst, ___dlen, ___sig, ___siglen, ___dsa) \
	AROS_LC6(int, DSA_sign, \
	AROS_LCA(int, (___type), D0), \
	AROS_LCA(const unsigned char *, (___dgst), A0), \
	AROS_LCA(int, (___dlen), D1), \
	AROS_LCA(unsigned char *, (___sig), A1), \
	AROS_LCA(unsigned int *, (___siglen), A2), \
	AROS_LCA(DSA *, (___dsa), A3), \
	struct Library *, AMISSL_BASE_NAME, 2388, Amissl)

#define DSA_verify(___type, ___dgst, ___dgst_len, ___sigbuf, ___siglen, ___dsa) \
	AROS_LC6(int, DSA_verify, \
	AROS_LCA(int, (___type), D0), \
	AROS_LCA(const unsigned char *, (___dgst), A0), \
	AROS_LCA(int, (___dgst_len), D1), \
	AROS_LCA(const unsigned char *, (___sigbuf), A1), \
	AROS_LCA(int, (___siglen), D2), \
	AROS_LCA(DSA *, (___dsa), A2), \
	struct Library *, AMISSL_BASE_NAME, 2389, Amissl)

#define DSA_get_ex_new_index(___argl, ___argp, ___new_func, ___dup_func, ___free_func) \
	AROS_LC5(int, DSA_get_ex_new_index, \
	AROS_LCA(long, (___argl), D0), \
	AROS_LCA(void *, (___argp), A0), \
	AROS_LCA(CRYPTO_EX_new * (*)(), (___new_func), A1), \
	AROS_LCA(CRYPTO_EX_dup * (*)(), (___dup_func), A2), \
	AROS_LCA(CRYPTO_EX_free * (*)(), (___free_func), A3), \
	struct Library *, AMISSL_BASE_NAME, 2390, Amissl)

#define DSA_set_ex_data(___d, ___idx, ___arg) \
	AROS_LC3(int, DSA_set_ex_data, \
	AROS_LCA(DSA *, (___d), A0), \
	AROS_LCA(int, (___idx), D0), \
	AROS_LCA(void *, (___arg), A1), \
	struct Library *, AMISSL_BASE_NAME, 2391, Amissl)

#define DSA_get_ex_data(___d, ___idx) \
	AROS_LC2(void *, DSA_get_ex_data, \
	AROS_LCA(DSA *, (___d), A0), \
	AROS_LCA(int, (___idx), D0), \
	struct Library *, AMISSL_BASE_NAME, 2392, Amissl)

#define d2i_DSAPublicKey(___a, ___pp, ___length) \
	AROS_LC3(DSA *, d2i_DSAPublicKey, \
	AROS_LCA(DSA **, (___a), A0), \
	AROS_LCA(const unsigned char **, (___pp), A1), \
	AROS_LCA(long, (___length), D0), \
	struct Library *, AMISSL_BASE_NAME, 2393, Amissl)

#define d2i_DSAPrivateKey(___a, ___pp, ___length) \
	AROS_LC3(DSA *, d2i_DSAPrivateKey, \
	AROS_LCA(DSA **, (___a), A0), \
	AROS_LCA(const unsigned char **, (___pp), A1), \
	AROS_LCA(long, (___length), D0), \
	struct Library *, AMISSL_BASE_NAME, 2394, Amissl)

#define d2i_DSAparams(___a, ___pp, ___length) \
	AROS_LC3(DSA *, d2i_DSAparams, \
	AROS_LCA(DSA **, (___a), A0), \
	AROS_LCA(const unsigned char **, (___pp), A1), \
	AROS_LCA(long, (___length), D0), \
	struct Library *, AMISSL_BASE_NAME, 2395, Amissl)

#define DSA_generate_parameters(___bits, ___seed, ___seed_len, ___counter_ret, ___h_ret, ___callback, ___cb_arg) \
	AROS_LC7(DSA *, DSA_generate_parameters, \
	AROS_LCA(int, (___bits), D0), \
	AROS_LCA(unsigned char *, (___seed), A0), \
	AROS_LCA(int, (___seed_len), D1), \
	AROS_LCA(int *, (___counter_ret), A1), \
	AROS_LCA(unsigned long *, (___h_ret), A2), \
	AROS_LCA(void (*)(int,int,void *), (___callback), A3), \
	AROS_LCA(void *, (___cb_arg), D2), \
	struct Library *, AMISSL_BASE_NAME, 2396, Amissl)

#define DSA_generate_key(___a) \
	AROS_LC1(int, DSA_generate_key, \
	AROS_LCA(DSA *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 2397, Amissl)

#define i2d_DSAPublicKey(___a, ___pp) \
	AROS_LC2(int, i2d_DSAPublicKey, \
	AROS_LCA(const DSA *, (___a), A0), \
	AROS_LCA(unsigned char **, (___pp), A1), \
	struct Library *, AMISSL_BASE_NAME, 2398, Amissl)

#define i2d_DSAPrivateKey(___a, ___pp) \
	AROS_LC2(int, i2d_DSAPrivateKey, \
	AROS_LCA(const DSA *, (___a), A0), \
	AROS_LCA(unsigned char **, (___pp), A1), \
	struct Library *, AMISSL_BASE_NAME, 2399, Amissl)

#define i2d_DSAparams(___a, ___pp) \
	AROS_LC2(int, i2d_DSAparams, \
	AROS_LCA(const DSA *, (___a), A0), \
	AROS_LCA(unsigned char **, (___pp), A1), \
	struct Library *, AMISSL_BASE_NAME, 2400, Amissl)

#define DSAparams_print(___bp, ___x) \
	AROS_LC2(int, DSAparams_print, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(const DSA *, (___x), A1), \
	struct Library *, AMISSL_BASE_NAME, 2401, Amissl)

#define DSA_print(___bp, ___x, ___off) \
	AROS_LC3(int, DSA_print, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(const DSA *, (___x), A1), \
	AROS_LCA(int, (___off), D0), \
	struct Library *, AMISSL_BASE_NAME, 2402, Amissl)

#define DSA_dup_DH(___r) \
	AROS_LC1(DH *, DSA_dup_DH, \
	AROS_LCA(const DSA *, (___r), A0), \
	struct Library *, AMISSL_BASE_NAME, 2403, Amissl)

#define ERR_load_DSA_strings() \
	AROS_LC0(void, ERR_load_DSA_strings, \
	struct Library *, AMISSL_BASE_NAME, 2404, Amissl)

#define idea_options() \
	AROS_LC0(const char *, idea_options, \
	struct Library *, AMISSL_BASE_NAME, 2405, Amissl)

#define idea_ecb_encrypt(___in, ___out, ___ks) \
	AROS_LC3(void, idea_ecb_encrypt, \
	AROS_LCA(const unsigned char *, (___in), A0), \
	AROS_LCA(unsigned char *, (___out), A1), \
	AROS_LCA(IDEA_KEY_SCHEDULE *, (___ks), A2), \
	struct Library *, AMISSL_BASE_NAME, 2406, Amissl)

#define idea_set_encrypt_key(___key, ___ks) \
	AROS_LC2(void, idea_set_encrypt_key, \
	AROS_LCA(const unsigned char *, (___key), A0), \
	AROS_LCA(IDEA_KEY_SCHEDULE *, (___ks), A1), \
	struct Library *, AMISSL_BASE_NAME, 2407, Amissl)

#define idea_set_decrypt_key(___ek, ___dk) \
	AROS_LC2(void, idea_set_decrypt_key, \
	AROS_LCA(IDEA_KEY_SCHEDULE *, (___ek), A0), \
	AROS_LCA(IDEA_KEY_SCHEDULE *, (___dk), A1), \
	struct Library *, AMISSL_BASE_NAME, 2408, Amissl)

#define idea_cbc_encrypt(___in, ___out, ___length, ___ks, ___iv, ___enc) \
	AROS_LC6(void, idea_cbc_encrypt, \
	AROS_LCA(const unsigned char *, (___in), A0), \
	AROS_LCA(unsigned char *, (___out), A1), \
	AROS_LCA(long, (___length), D0), \
	AROS_LCA(IDEA_KEY_SCHEDULE *, (___ks), A2), \
	AROS_LCA(unsigned char *, (___iv), A3), \
	AROS_LCA(int, (___enc), D1), \
	struct Library *, AMISSL_BASE_NAME, 2409, Amissl)

#define idea_cfb64_encrypt(___in, ___out, ___length, ___ks, ___iv, ___num, ___enc) \
	AROS_LC7(void, idea_cfb64_encrypt, \
	AROS_LCA(const unsigned char *, (___in), A0), \
	AROS_LCA(unsigned char *, (___out), A1), \
	AROS_LCA(long, (___length), D0), \
	AROS_LCA(IDEA_KEY_SCHEDULE *, (___ks), A2), \
	AROS_LCA(unsigned char *, (___iv), A3), \
	AROS_LCA(int *, (___num), D1), \
	AROS_LCA(int, (___enc), D2), \
	struct Library *, AMISSL_BASE_NAME, 2410, Amissl)

#define idea_ofb64_encrypt(___in, ___out, ___length, ___ks, ___iv, ___num) \
	AROS_LC6(void, idea_ofb64_encrypt, \
	AROS_LCA(const unsigned char *, (___in), A0), \
	AROS_LCA(unsigned char *, (___out), A1), \
	AROS_LCA(long, (___length), D0), \
	AROS_LCA(IDEA_KEY_SCHEDULE *, (___ks), A2), \
	AROS_LCA(unsigned char *, (___iv), A3), \
	AROS_LCA(int *, (___num), D1), \
	struct Library *, AMISSL_BASE_NAME, 2411, Amissl)

#define idea_encrypt(___in, ___ks) \
	AROS_LC2(void, idea_encrypt, \
	AROS_LCA(unsigned long *, (___in), A0), \
	AROS_LCA(IDEA_KEY_SCHEDULE *, (___ks), A1), \
	struct Library *, AMISSL_BASE_NAME, 2412, Amissl)

#define MD2_options() \
	AROS_LC0(const char *, MD2_options, \
	struct Library *, AMISSL_BASE_NAME, 2413, Amissl)

#define MD2_Init(___c) \
	AROS_LC1(int, MD2_Init, \
	AROS_LCA(MD2_CTX *, (___c), A0), \
	struct Library *, AMISSL_BASE_NAME, 2414, Amissl)

#define MD2_Update(___c, ___data, ___len) \
	AROS_LC3(int, MD2_Update, \
	AROS_LCA(MD2_CTX *, (___c), A0), \
	AROS_LCA(const unsigned char *, (___data), A1), \
	AROS_LCA(unsigned long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 2415, Amissl)

#define MD2_Final(___md, ___c) \
	AROS_LC2(int, MD2_Final, \
	AROS_LCA(unsigned char *, (___md), A0), \
	AROS_LCA(MD2_CTX *, (___c), A1), \
	struct Library *, AMISSL_BASE_NAME, 2416, Amissl)

#define MD2(___d, ___n, ___md) \
	AROS_LC3(unsigned char *, MD2, \
	AROS_LCA(const unsigned char *, (___d), A0), \
	AROS_LCA(unsigned long, (___n), D0), \
	AROS_LCA(unsigned char *, (___md), A1), \
	struct Library *, AMISSL_BASE_NAME, 2417, Amissl)

#define MD4_Init(___c) \
	AROS_LC1(int, MD4_Init, \
	AROS_LCA(MD4_CTX *, (___c), A0), \
	struct Library *, AMISSL_BASE_NAME, 2418, Amissl)

#define MD4_Update(___c, ___data, ___len) \
	AROS_LC3(int, MD4_Update, \
	AROS_LCA(MD4_CTX *, (___c), A0), \
	AROS_LCA(const void *, (___data), A1), \
	AROS_LCA(unsigned long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 2419, Amissl)

#define MD4_Final(___md, ___c) \
	AROS_LC2(int, MD4_Final, \
	AROS_LCA(unsigned char *, (___md), A0), \
	AROS_LCA(MD4_CTX *, (___c), A1), \
	struct Library *, AMISSL_BASE_NAME, 2420, Amissl)

#define MD4(___d, ___n, ___md) \
	AROS_LC3(unsigned char *, MD4, \
	AROS_LCA(const unsigned char *, (___d), A0), \
	AROS_LCA(unsigned long, (___n), D0), \
	AROS_LCA(unsigned char *, (___md), A1), \
	struct Library *, AMISSL_BASE_NAME, 2421, Amissl)

#define MD4_Transform(___c, ___b) \
	AROS_LC2(void, MD4_Transform, \
	AROS_LCA(MD4_CTX *, (___c), A0), \
	AROS_LCA(const unsigned char *, (___b), A1), \
	struct Library *, AMISSL_BASE_NAME, 2422, Amissl)

#define MD5_Init(___c) \
	AROS_LC1(int, MD5_Init, \
	AROS_LCA(MD5_CTX *, (___c), A0), \
	struct Library *, AMISSL_BASE_NAME, 2423, Amissl)

#define MD5_Update(___c, ___data, ___len) \
	AROS_LC3(int, MD5_Update, \
	AROS_LCA(MD5_CTX *, (___c), A0), \
	AROS_LCA(const void *, (___data), A1), \
	AROS_LCA(unsigned long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 2424, Amissl)

#define MD5_Final(___md, ___c) \
	AROS_LC2(int, MD5_Final, \
	AROS_LCA(unsigned char *, (___md), A0), \
	AROS_LCA(MD5_CTX *, (___c), A1), \
	struct Library *, AMISSL_BASE_NAME, 2425, Amissl)

#define MD5(___d, ___n, ___md) \
	AROS_LC3(unsigned char *, MD5, \
	AROS_LCA(const unsigned char *, (___d), A0), \
	AROS_LCA(unsigned long, (___n), D0), \
	AROS_LCA(unsigned char *, (___md), A1), \
	struct Library *, AMISSL_BASE_NAME, 2426, Amissl)

#define MD5_Transform(___c, ___b) \
	AROS_LC2(void, MD5_Transform, \
	AROS_LCA(MD5_CTX *, (___c), A0), \
	AROS_LCA(const unsigned char *, (___b), A1), \
	struct Library *, AMISSL_BASE_NAME, 2427, Amissl)

#define MDC2_Init(___c) \
	AROS_LC1(int, MDC2_Init, \
	AROS_LCA(MDC2_CTX *, (___c), A0), \
	struct Library *, AMISSL_BASE_NAME, 2428, Amissl)

#define MDC2_Update(___c, ___data, ___len) \
	AROS_LC3(int, MDC2_Update, \
	AROS_LCA(MDC2_CTX *, (___c), A0), \
	AROS_LCA(const unsigned char *, (___data), A1), \
	AROS_LCA(unsigned long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 2429, Amissl)

#define MDC2_Final(___md, ___c) \
	AROS_LC2(int, MDC2_Final, \
	AROS_LCA(unsigned char *, (___md), A0), \
	AROS_LCA(MDC2_CTX *, (___c), A1), \
	struct Library *, AMISSL_BASE_NAME, 2430, Amissl)

#define MDC2(___d, ___n, ___md) \
	AROS_LC3(unsigned char *, MDC2, \
	AROS_LCA(const unsigned char *, (___d), A0), \
	AROS_LCA(unsigned long, (___n), D0), \
	AROS_LCA(unsigned char *, (___md), A1), \
	struct Library *, AMISSL_BASE_NAME, 2431, Amissl)

#define RC2_set_key(___key, ___len, ___data, ___bits) \
	AROS_LC4(void, RC2_set_key, \
	AROS_LCA(RC2_KEY *, (___key), A0), \
	AROS_LCA(int, (___len), D0), \
	AROS_LCA(const unsigned char *, (___data), A1), \
	AROS_LCA(int, (___bits), D1), \
	struct Library *, AMISSL_BASE_NAME, 2432, Amissl)

#define RC2_ecb_encrypt(___in, ___out, ___key, ___enc) \
	AROS_LC4(void, RC2_ecb_encrypt, \
	AROS_LCA(const unsigned char *, (___in), A0), \
	AROS_LCA(unsigned char *, (___out), A1), \
	AROS_LCA(RC2_KEY *, (___key), A2), \
	AROS_LCA(int, (___enc), D0), \
	struct Library *, AMISSL_BASE_NAME, 2433, Amissl)

#define RC2_encrypt(___data, ___key) \
	AROS_LC2(void, RC2_encrypt, \
	AROS_LCA(unsigned long *, (___data), A0), \
	AROS_LCA(RC2_KEY *, (___key), A1), \
	struct Library *, AMISSL_BASE_NAME, 2434, Amissl)

#define RC2_decrypt(___data, ___key) \
	AROS_LC2(void, RC2_decrypt, \
	AROS_LCA(unsigned long *, (___data), A0), \
	AROS_LCA(RC2_KEY *, (___key), A1), \
	struct Library *, AMISSL_BASE_NAME, 2435, Amissl)

#define RC2_cbc_encrypt(___in, ___out, ___length, ___ks, ___iv, ___enc) \
	AROS_LC6(void, RC2_cbc_encrypt, \
	AROS_LCA(const unsigned char *, (___in), A0), \
	AROS_LCA(unsigned char *, (___out), A1), \
	AROS_LCA(long, (___length), D0), \
	AROS_LCA(RC2_KEY *, (___ks), A2), \
	AROS_LCA(unsigned char *, (___iv), A3), \
	AROS_LCA(int, (___enc), D1), \
	struct Library *, AMISSL_BASE_NAME, 2436, Amissl)

#define RC2_cfb64_encrypt(___in, ___out, ___length, ___schedule, ___ivec, ___num, ___enc) \
	AROS_LC7(void, RC2_cfb64_encrypt, \
	AROS_LCA(const unsigned char *, (___in), A0), \
	AROS_LCA(unsigned char *, (___out), A1), \
	AROS_LCA(long, (___length), D0), \
	AROS_LCA(RC2_KEY *, (___schedule), A2), \
	AROS_LCA(unsigned char *, (___ivec), A3), \
	AROS_LCA(int *, (___num), D1), \
	AROS_LCA(int, (___enc), D2), \
	struct Library *, AMISSL_BASE_NAME, 2437, Amissl)

#define RC2_ofb64_encrypt(___in, ___out, ___length, ___schedule, ___ivec, ___num) \
	AROS_LC6(void, RC2_ofb64_encrypt, \
	AROS_LCA(const unsigned char *, (___in), A0), \
	AROS_LCA(unsigned char *, (___out), A1), \
	AROS_LCA(long, (___length), D0), \
	AROS_LCA(RC2_KEY *, (___schedule), A2), \
	AROS_LCA(unsigned char *, (___ivec), A3), \
	AROS_LCA(int *, (___num), D1), \
	struct Library *, AMISSL_BASE_NAME, 2438, Amissl)

#define RC4_options() \
	AROS_LC0(const char *, RC4_options, \
	struct Library *, AMISSL_BASE_NAME, 2439, Amissl)

#define RC4_set_key(___key, ___len, ___data) \
	AROS_LC3(void, RC4_set_key, \
	AROS_LCA(RC4_KEY *, (___key), A0), \
	AROS_LCA(int, (___len), D0), \
	AROS_LCA(const unsigned char *, (___data), A1), \
	struct Library *, AMISSL_BASE_NAME, 2440, Amissl)

#define RC4(___key, ___len, ___indata, ___outdata) \
	AROS_LC4(void, RC4, \
	AROS_LCA(RC4_KEY *, (___key), A0), \
	AROS_LCA(unsigned long, (___len), D0), \
	AROS_LCA(const unsigned char *, (___indata), A1), \
	AROS_LCA(unsigned char *, (___outdata), A2), \
	struct Library *, AMISSL_BASE_NAME, 2441, Amissl)

#define RC5_32_set_key(___key, ___len, ___data, ___rounds) \
	AROS_LC4(void, RC5_32_set_key, \
	AROS_LCA(RC5_32_KEY *, (___key), A0), \
	AROS_LCA(int, (___len), D0), \
	AROS_LCA(const unsigned char *, (___data), A1), \
	AROS_LCA(int, (___rounds), D1), \
	struct Library *, AMISSL_BASE_NAME, 2442, Amissl)

#define RC5_32_ecb_encrypt(___in, ___out, ___key, ___enc) \
	AROS_LC4(void, RC5_32_ecb_encrypt, \
	AROS_LCA(const unsigned char *, (___in), A0), \
	AROS_LCA(unsigned char *, (___out), A1), \
	AROS_LCA(RC5_32_KEY *, (___key), A2), \
	AROS_LCA(int, (___enc), D0), \
	struct Library *, AMISSL_BASE_NAME, 2443, Amissl)

#define RC5_32_encrypt(___data, ___key) \
	AROS_LC2(void, RC5_32_encrypt, \
	AROS_LCA(unsigned long *, (___data), A0), \
	AROS_LCA(RC5_32_KEY *, (___key), A1), \
	struct Library *, AMISSL_BASE_NAME, 2444, Amissl)

#define RC5_32_decrypt(___data, ___key) \
	AROS_LC2(void, RC5_32_decrypt, \
	AROS_LCA(unsigned long *, (___data), A0), \
	AROS_LCA(RC5_32_KEY *, (___key), A1), \
	struct Library *, AMISSL_BASE_NAME, 2445, Amissl)

#define RC5_32_cbc_encrypt(___in, ___out, ___length, ___ks, ___iv, ___enc) \
	AROS_LC6(void, RC5_32_cbc_encrypt, \
	AROS_LCA(const unsigned char *, (___in), A0), \
	AROS_LCA(unsigned char *, (___out), A1), \
	AROS_LCA(long, (___length), D0), \
	AROS_LCA(RC5_32_KEY *, (___ks), A2), \
	AROS_LCA(unsigned char *, (___iv), A3), \
	AROS_LCA(int, (___enc), D1), \
	struct Library *, AMISSL_BASE_NAME, 2446, Amissl)

#define RC5_32_cfb64_encrypt(___in, ___out, ___length, ___schedule, ___ivec, ___num, ___enc) \
	AROS_LC7(void, RC5_32_cfb64_encrypt, \
	AROS_LCA(const unsigned char *, (___in), A0), \
	AROS_LCA(unsigned char *, (___out), A1), \
	AROS_LCA(long, (___length), D0), \
	AROS_LCA(RC5_32_KEY *, (___schedule), A2), \
	AROS_LCA(unsigned char *, (___ivec), A3), \
	AROS_LCA(int *, (___num), D1), \
	AROS_LCA(int, (___enc), D2), \
	struct Library *, AMISSL_BASE_NAME, 2447, Amissl)

#define RC5_32_ofb64_encrypt(___in, ___out, ___length, ___schedule, ___ivec, ___num) \
	AROS_LC6(void, RC5_32_ofb64_encrypt, \
	AROS_LCA(const unsigned char *, (___in), A0), \
	AROS_LCA(unsigned char *, (___out), A1), \
	AROS_LCA(long, (___length), D0), \
	AROS_LCA(RC5_32_KEY *, (___schedule), A2), \
	AROS_LCA(unsigned char *, (___ivec), A3), \
	AROS_LCA(int *, (___num), D1), \
	struct Library *, AMISSL_BASE_NAME, 2448, Amissl)

#define RIPEMD160_Init(___c) \
	AROS_LC1(int, RIPEMD160_Init, \
	AROS_LCA(RIPEMD160_CTX *, (___c), A0), \
	struct Library *, AMISSL_BASE_NAME, 2449, Amissl)

#define RIPEMD160_Update(___c, ___data, ___len) \
	AROS_LC3(int, RIPEMD160_Update, \
	AROS_LCA(RIPEMD160_CTX *, (___c), A0), \
	AROS_LCA(const void *, (___data), A1), \
	AROS_LCA(unsigned long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 2450, Amissl)

#define RIPEMD160_Final(___md, ___c) \
	AROS_LC2(int, RIPEMD160_Final, \
	AROS_LCA(unsigned char *, (___md), A0), \
	AROS_LCA(RIPEMD160_CTX *, (___c), A1), \
	struct Library *, AMISSL_BASE_NAME, 2451, Amissl)

#define RIPEMD160(___d, ___n, ___md) \
	AROS_LC3(unsigned char *, RIPEMD160, \
	AROS_LCA(const unsigned char *, (___d), A0), \
	AROS_LCA(unsigned long, (___n), D0), \
	AROS_LCA(unsigned char *, (___md), A1), \
	struct Library *, AMISSL_BASE_NAME, 2452, Amissl)

#define RIPEMD160_Transform(___c, ___b) \
	AROS_LC2(void, RIPEMD160_Transform, \
	AROS_LCA(RIPEMD160_CTX *, (___c), A0), \
	AROS_LCA(const unsigned char *, (___b), A1), \
	struct Library *, AMISSL_BASE_NAME, 2453, Amissl)

#define RSA_new() \
	AROS_LC0(RSA *, RSA_new, \
	struct Library *, AMISSL_BASE_NAME, 2454, Amissl)

#define RSA_new_method(___engine) \
	AROS_LC1(RSA *, RSA_new_method, \
	AROS_LCA(ENGINE *, (___engine), A0), \
	struct Library *, AMISSL_BASE_NAME, 2455, Amissl)

#define RSA_size(___a) \
	AROS_LC1(int, RSA_size, \
	AROS_LCA(const RSA *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 2456, Amissl)

#define RSA_generate_key(___bits, ___e, ___callback, ___cb_arg) \
	AROS_LC4(RSA *, RSA_generate_key, \
	AROS_LCA(int, (___bits), D0), \
	AROS_LCA(unsigned long, (___e), D1), \
	AROS_LCA(void (*)(int,int,void *), (___callback), A0), \
	AROS_LCA(void *, (___cb_arg), A1), \
	struct Library *, AMISSL_BASE_NAME, 2457, Amissl)

#define RSA_check_key(___a) \
	AROS_LC1(int, RSA_check_key, \
	AROS_LCA(const RSA *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 2458, Amissl)

#define RSA_public_encrypt(___flen, ___from, ___to, ___rsa, ___padding) \
	AROS_LC5(int, RSA_public_encrypt, \
	AROS_LCA(int, (___flen), D0), \
	AROS_LCA(const unsigned char *, (___from), A0), \
	AROS_LCA(unsigned char *, (___to), A1), \
	AROS_LCA(RSA *, (___rsa), A2), \
	AROS_LCA(int, (___padding), D1), \
	struct Library *, AMISSL_BASE_NAME, 2459, Amissl)

#define RSA_private_encrypt(___flen, ___from, ___to, ___rsa, ___padding) \
	AROS_LC5(int, RSA_private_encrypt, \
	AROS_LCA(int, (___flen), D0), \
	AROS_LCA(const unsigned char *, (___from), A0), \
	AROS_LCA(unsigned char *, (___to), A1), \
	AROS_LCA(RSA *, (___rsa), A2), \
	AROS_LCA(int, (___padding), D1), \
	struct Library *, AMISSL_BASE_NAME, 2460, Amissl)

#define RSA_public_decrypt(___flen, ___from, ___to, ___rsa, ___padding) \
	AROS_LC5(int, RSA_public_decrypt, \
	AROS_LCA(int, (___flen), D0), \
	AROS_LCA(const unsigned char *, (___from), A0), \
	AROS_LCA(unsigned char *, (___to), A1), \
	AROS_LCA(RSA *, (___rsa), A2), \
	AROS_LCA(int, (___padding), D1), \
	struct Library *, AMISSL_BASE_NAME, 2461, Amissl)

#define RSA_private_decrypt(___flen, ___from, ___to, ___rsa, ___padding) \
	AROS_LC5(int, RSA_private_decrypt, \
	AROS_LCA(int, (___flen), D0), \
	AROS_LCA(const unsigned char *, (___from), A0), \
	AROS_LCA(unsigned char *, (___to), A1), \
	AROS_LCA(RSA *, (___rsa), A2), \
	AROS_LCA(int, (___padding), D1), \
	struct Library *, AMISSL_BASE_NAME, 2462, Amissl)

#define RSA_free(___r) \
	AROS_LC1(void, RSA_free, \
	AROS_LCA(RSA *, (___r), A0), \
	struct Library *, AMISSL_BASE_NAME, 2463, Amissl)

#define RSA_up_ref(___r) \
	AROS_LC1(int, RSA_up_ref, \
	AROS_LCA(RSA *, (___r), A0), \
	struct Library *, AMISSL_BASE_NAME, 2464, Amissl)

#define RSA_flags(___r) \
	AROS_LC1(int, RSA_flags, \
	AROS_LCA(const RSA *, (___r), A0), \
	struct Library *, AMISSL_BASE_NAME, 2465, Amissl)

#define RSA_set_default_method(___meth) \
	AROS_LC1(void, RSA_set_default_method, \
	AROS_LCA(const RSA_METHOD *, (___meth), A0), \
	struct Library *, AMISSL_BASE_NAME, 2466, Amissl)

#define RSA_get_default_method() \
	AROS_LC0(const RSA_METHOD *, RSA_get_default_method, \
	struct Library *, AMISSL_BASE_NAME, 2467, Amissl)

#define RSA_get_method(___rsa) \
	AROS_LC1(const RSA_METHOD *, RSA_get_method, \
	AROS_LCA(const RSA *, (___rsa), A0), \
	struct Library *, AMISSL_BASE_NAME, 2468, Amissl)

#define RSA_set_method(___rsa, ___meth) \
	AROS_LC2(int, RSA_set_method, \
	AROS_LCA(RSA *, (___rsa), A0), \
	AROS_LCA(const RSA_METHOD *, (___meth), A1), \
	struct Library *, AMISSL_BASE_NAME, 2469, Amissl)

#define RSA_memory_lock(___r) \
	AROS_LC1(int, RSA_memory_lock, \
	AROS_LCA(RSA *, (___r), A0), \
	struct Library *, AMISSL_BASE_NAME, 2470, Amissl)

#define RSA_PKCS1_SSLeay() \
	AROS_LC0(const RSA_METHOD *, RSA_PKCS1_SSLeay, \
	struct Library *, AMISSL_BASE_NAME, 2471, Amissl)

#define RSA_null_method() \
	AROS_LC0(const RSA_METHOD *, RSA_null_method, \
	struct Library *, AMISSL_BASE_NAME, 2472, Amissl)

#define d2i_RSAPublicKey(___a, ___in, ___len) \
	AROS_LC3(RSA *, d2i_RSAPublicKey, \
	AROS_LCA(RSA **, (___a), A0), \
	AROS_LCA(const unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 2473, Amissl)

#define i2d_RSAPublicKey(___a, ___out) \
	AROS_LC2(int, i2d_RSAPublicKey, \
	AROS_LCA(const RSA *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 2474, Amissl)

#define RSAPublicKey_it() \
	AROS_LC0(const ASN1_ITEM *, RSAPublicKey_it, \
	struct Library *, AMISSL_BASE_NAME, 2475, Amissl)

#define d2i_RSAPrivateKey(___a, ___in, ___len) \
	AROS_LC3(RSA *, d2i_RSAPrivateKey, \
	AROS_LCA(RSA **, (___a), A0), \
	AROS_LCA(const unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 2476, Amissl)

#define i2d_RSAPrivateKey(___a, ___out) \
	AROS_LC2(int, i2d_RSAPrivateKey, \
	AROS_LCA(const RSA *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 2477, Amissl)

#define RSAPrivateKey_it() \
	AROS_LC0(const ASN1_ITEM *, RSAPrivateKey_it, \
	struct Library *, AMISSL_BASE_NAME, 2478, Amissl)

#define RSA_print(___bp, ___r, ___offset) \
	AROS_LC3(int, RSA_print, \
	AROS_LCA(BIO *, (___bp), A0), \
	AROS_LCA(const RSA *, (___r), A1), \
	AROS_LCA(int, (___offset), D0), \
	struct Library *, AMISSL_BASE_NAME, 2479, Amissl)

#define i2d_RSA_NET(___a, ___pp, ___cb, ___sgckey) \
	AROS_LC4(int, i2d_RSA_NET, \
	AROS_LCA(const RSA *, (___a), A0), \
	AROS_LCA(unsigned char **, (___pp), A1), \
	AROS_LCA(int (*)(), (___cb), A2), \
	AROS_LCA(int, (___sgckey), D0), \
	struct Library *, AMISSL_BASE_NAME, 2480, Amissl)

#define d2i_RSA_NET(___a, ___pp, ___length, ___cb, ___sgckey) \
	AROS_LC5(RSA *, d2i_RSA_NET, \
	AROS_LCA(RSA **, (___a), A0), \
	AROS_LCA(const unsigned char **, (___pp), A1), \
	AROS_LCA(long, (___length), D0), \
	AROS_LCA(int (*)(), (___cb), A2), \
	AROS_LCA(int, (___sgckey), D1), \
	struct Library *, AMISSL_BASE_NAME, 2481, Amissl)

#define i2d_Netscape_RSA(___a, ___pp, ___cb) \
	AROS_LC3(int, i2d_Netscape_RSA, \
	AROS_LCA(const RSA *, (___a), A0), \
	AROS_LCA(unsigned char **, (___pp), A1), \
	AROS_LCA(int (*)(), (___cb), A2), \
	struct Library *, AMISSL_BASE_NAME, 2482, Amissl)

#define d2i_Netscape_RSA(___a, ___pp, ___length, ___cb) \
	AROS_LC4(RSA *, d2i_Netscape_RSA, \
	AROS_LCA(RSA **, (___a), A0), \
	AROS_LCA(const unsigned char **, (___pp), A1), \
	AROS_LCA(long, (___length), D0), \
	AROS_LCA(int (*)(), (___cb), A2), \
	struct Library *, AMISSL_BASE_NAME, 2483, Amissl)

#define RSA_sign(___type, ___m, ___m_length, ___sigret, ___siglen, ___rsa) \
	AROS_LC6(int, RSA_sign, \
	AROS_LCA(int, (___type), D0), \
	AROS_LCA(const unsigned char *, (___m), A0), \
	AROS_LCA(unsigned int, (___m_length), D1), \
	AROS_LCA(unsigned char *, (___sigret), A1), \
	AROS_LCA(unsigned int *, (___siglen), A2), \
	AROS_LCA(RSA *, (___rsa), A3), \
	struct Library *, AMISSL_BASE_NAME, 2484, Amissl)

#define RSA_verify(___type, ___m, ___m_length, ___sigbuf, ___siglen, ___rsa) \
	AROS_LC6(int, RSA_verify, \
	AROS_LCA(int, (___type), D0), \
	AROS_LCA(const unsigned char *, (___m), A0), \
	AROS_LCA(unsigned int, (___m_length), D1), \
	AROS_LCA(unsigned char *, (___sigbuf), A1), \
	AROS_LCA(unsigned int, (___siglen), D2), \
	AROS_LCA(RSA *, (___rsa), A2), \
	struct Library *, AMISSL_BASE_NAME, 2485, Amissl)

#define RSA_sign_ASN1_OCTET_STRING(___type, ___m, ___m_length, ___sigret, ___siglen, ___rsa) \
	AROS_LC6(int, RSA_sign_ASN1_OCTET_STRING, \
	AROS_LCA(int, (___type), D0), \
	AROS_LCA(const unsigned char *, (___m), A0), \
	AROS_LCA(unsigned int, (___m_length), D1), \
	AROS_LCA(unsigned char *, (___sigret), A1), \
	AROS_LCA(unsigned int *, (___siglen), A2), \
	AROS_LCA(RSA *, (___rsa), A3), \
	struct Library *, AMISSL_BASE_NAME, 2486, Amissl)

#define RSA_verify_ASN1_OCTET_STRING(___type, ___m, ___m_length, ___sigbuf, ___siglen, ___rsa) \
	AROS_LC6(int, RSA_verify_ASN1_OCTET_STRING, \
	AROS_LCA(int, (___type), D0), \
	AROS_LCA(const unsigned char *, (___m), A0), \
	AROS_LCA(unsigned int, (___m_length), D1), \
	AROS_LCA(unsigned char *, (___sigbuf), A1), \
	AROS_LCA(unsigned int, (___siglen), D2), \
	AROS_LCA(RSA *, (___rsa), A2), \
	struct Library *, AMISSL_BASE_NAME, 2487, Amissl)

#define RSA_blinding_on(___rsa, ___ctx) \
	AROS_LC2(int, RSA_blinding_on, \
	AROS_LCA(RSA *, (___rsa), A0), \
	AROS_LCA(BN_CTX *, (___ctx), A1), \
	struct Library *, AMISSL_BASE_NAME, 2488, Amissl)

#define RSA_blinding_off(___rsa) \
	AROS_LC1(void, RSA_blinding_off, \
	AROS_LCA(RSA *, (___rsa), A0), \
	struct Library *, AMISSL_BASE_NAME, 2489, Amissl)

#define RSA_padding_add_PKCS1_type_1(___to, ___tlen, ___f, ___fl) \
	AROS_LC4(int, RSA_padding_add_PKCS1_type_1, \
	AROS_LCA(unsigned char *, (___to), A0), \
	AROS_LCA(int, (___tlen), D0), \
	AROS_LCA(const unsigned char *, (___f), A1), \
	AROS_LCA(int, (___fl), D1), \
	struct Library *, AMISSL_BASE_NAME, 2490, Amissl)

#define RSA_padding_check_PKCS1_type_1(___to, ___tlen, ___f, ___fl, ___rsa_len) \
	AROS_LC5(int, RSA_padding_check_PKCS1_type_1, \
	AROS_LCA(unsigned char *, (___to), A0), \
	AROS_LCA(int, (___tlen), D0), \
	AROS_LCA(const unsigned char *, (___f), A1), \
	AROS_LCA(int, (___fl), D1), \
	AROS_LCA(int, (___rsa_len), D2), \
	struct Library *, AMISSL_BASE_NAME, 2491, Amissl)

#define RSA_padding_add_PKCS1_type_2(___to, ___tlen, ___f, ___fl) \
	AROS_LC4(int, RSA_padding_add_PKCS1_type_2, \
	AROS_LCA(unsigned char *, (___to), A0), \
	AROS_LCA(int, (___tlen), D0), \
	AROS_LCA(const unsigned char *, (___f), A1), \
	AROS_LCA(int, (___fl), D1), \
	struct Library *, AMISSL_BASE_NAME, 2492, Amissl)

#define RSA_padding_check_PKCS1_type_2(___to, ___tlen, ___f, ___fl, ___rsa_len) \
	AROS_LC5(int, RSA_padding_check_PKCS1_type_2, \
	AROS_LCA(unsigned char *, (___to), A0), \
	AROS_LCA(int, (___tlen), D0), \
	AROS_LCA(const unsigned char *, (___f), A1), \
	AROS_LCA(int, (___fl), D1), \
	AROS_LCA(int, (___rsa_len), D2), \
	struct Library *, AMISSL_BASE_NAME, 2493, Amissl)

#define RSA_padding_add_PKCS1_OAEP(___to, ___tlen, ___f, ___fl, ___p, ___pl) \
	AROS_LC6(int, RSA_padding_add_PKCS1_OAEP, \
	AROS_LCA(unsigned char *, (___to), A0), \
	AROS_LCA(int, (___tlen), D0), \
	AROS_LCA(const unsigned char *, (___f), A1), \
	AROS_LCA(int, (___fl), D1), \
	AROS_LCA(const unsigned char *, (___p), A2), \
	AROS_LCA(int, (___pl), D2), \
	struct Library *, AMISSL_BASE_NAME, 2494, Amissl)

#define RSA_padding_check_PKCS1_OAEP(___to, ___tlen, ___f, ___fl, ___rsa_len, ___p, ___pl) \
	AROS_LC7(int, RSA_padding_check_PKCS1_OAEP, \
	AROS_LCA(unsigned char *, (___to), A0), \
	AROS_LCA(int, (___tlen), D0), \
	AROS_LCA(const unsigned char *, (___f), A1), \
	AROS_LCA(int, (___fl), D1), \
	AROS_LCA(int, (___rsa_len), D2), \
	AROS_LCA(const unsigned char *, (___p), A2), \
	AROS_LCA(int, (___pl), D3), \
	struct Library *, AMISSL_BASE_NAME, 2495, Amissl)

#define RSA_padding_add_SSLv23(___to, ___tlen, ___f, ___fl) \
	AROS_LC4(int, RSA_padding_add_SSLv23, \
	AROS_LCA(unsigned char *, (___to), A0), \
	AROS_LCA(int, (___tlen), D0), \
	AROS_LCA(const unsigned char *, (___f), A1), \
	AROS_LCA(int, (___fl), D1), \
	struct Library *, AMISSL_BASE_NAME, 2496, Amissl)

#define RSA_padding_check_SSLv23(___to, ___tlen, ___f, ___fl, ___rsa_len) \
	AROS_LC5(int, RSA_padding_check_SSLv23, \
	AROS_LCA(unsigned char *, (___to), A0), \
	AROS_LCA(int, (___tlen), D0), \
	AROS_LCA(const unsigned char *, (___f), A1), \
	AROS_LCA(int, (___fl), D1), \
	AROS_LCA(int, (___rsa_len), D2), \
	struct Library *, AMISSL_BASE_NAME, 2497, Amissl)

#define RSA_padding_add_none(___to, ___tlen, ___f, ___fl) \
	AROS_LC4(int, RSA_padding_add_none, \
	AROS_LCA(unsigned char *, (___to), A0), \
	AROS_LCA(int, (___tlen), D0), \
	AROS_LCA(const unsigned char *, (___f), A1), \
	AROS_LCA(int, (___fl), D1), \
	struct Library *, AMISSL_BASE_NAME, 2498, Amissl)

#define RSA_padding_check_none(___to, ___tlen, ___f, ___fl, ___rsa_len) \
	AROS_LC5(int, RSA_padding_check_none, \
	AROS_LCA(unsigned char *, (___to), A0), \
	AROS_LCA(int, (___tlen), D0), \
	AROS_LCA(const unsigned char *, (___f), A1), \
	AROS_LCA(int, (___fl), D1), \
	AROS_LCA(int, (___rsa_len), D2), \
	struct Library *, AMISSL_BASE_NAME, 2499, Amissl)

#define RSA_get_ex_new_index(___argl, ___argp, ___new_func, ___dup_func, ___free_func) \
	AROS_LC5(int, RSA_get_ex_new_index, \
	AROS_LCA(long, (___argl), D0), \
	AROS_LCA(void *, (___argp), A0), \
	AROS_LCA(CRYPTO_EX_new * (*)(), (___new_func), A1), \
	AROS_LCA(CRYPTO_EX_dup * (*)(), (___dup_func), A2), \
	AROS_LCA(CRYPTO_EX_free * (*)(), (___free_func), A3), \
	struct Library *, AMISSL_BASE_NAME, 2500, Amissl)

#define RSA_set_ex_data(___r, ___idx, ___arg) \
	AROS_LC3(int, RSA_set_ex_data, \
	AROS_LCA(RSA *, (___r), A0), \
	AROS_LCA(int, (___idx), D0), \
	AROS_LCA(void *, (___arg), A1), \
	struct Library *, AMISSL_BASE_NAME, 2501, Amissl)

#define RSA_get_ex_data(___r, ___idx) \
	AROS_LC2(void *, RSA_get_ex_data, \
	AROS_LCA(const RSA *, (___r), A0), \
	AROS_LCA(int, (___idx), D0), \
	struct Library *, AMISSL_BASE_NAME, 2502, Amissl)

#define RSAPublicKey_dup(___rsa) \
	AROS_LC1(RSA *, RSAPublicKey_dup, \
	AROS_LCA(RSA *, (___rsa), A0), \
	struct Library *, AMISSL_BASE_NAME, 2503, Amissl)

#define RSAPrivateKey_dup(___rsa) \
	AROS_LC1(RSA *, RSAPrivateKey_dup, \
	AROS_LCA(RSA *, (___rsa), A0), \
	struct Library *, AMISSL_BASE_NAME, 2504, Amissl)

#define ERR_load_RSA_strings() \
	AROS_LC0(void, ERR_load_RSA_strings, \
	struct Library *, AMISSL_BASE_NAME, 2505, Amissl)

#define SHA_Init(___c) \
	AROS_LC1(int, SHA_Init, \
	AROS_LCA(SHA_CTX *, (___c), A0), \
	struct Library *, AMISSL_BASE_NAME, 2506, Amissl)

#define SHA_Update(___c, ___data, ___len) \
	AROS_LC3(int, SHA_Update, \
	AROS_LCA(SHA_CTX *, (___c), A0), \
	AROS_LCA(const void *, (___data), A1), \
	AROS_LCA(unsigned long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 2507, Amissl)

#define SHA_Final(___md, ___c) \
	AROS_LC2(int, SHA_Final, \
	AROS_LCA(unsigned char *, (___md), A0), \
	AROS_LCA(SHA_CTX *, (___c), A1), \
	struct Library *, AMISSL_BASE_NAME, 2508, Amissl)

#define SHA(___d, ___n, ___md) \
	AROS_LC3(unsigned char *, SHA, \
	AROS_LCA(const unsigned char *, (___d), A0), \
	AROS_LCA(unsigned long, (___n), D0), \
	AROS_LCA(unsigned char *, (___md), A1), \
	struct Library *, AMISSL_BASE_NAME, 2509, Amissl)

#define SHA_Transform(___c, ___data) \
	AROS_LC2(void, SHA_Transform, \
	AROS_LCA(SHA_CTX *, (___c), A0), \
	AROS_LCA(const unsigned char *, (___data), A1), \
	struct Library *, AMISSL_BASE_NAME, 2510, Amissl)

#define SHA1_Init(___c) \
	AROS_LC1(int, SHA1_Init, \
	AROS_LCA(SHA_CTX *, (___c), A0), \
	struct Library *, AMISSL_BASE_NAME, 2511, Amissl)

#define SHA1_Update(___c, ___data, ___len) \
	AROS_LC3(int, SHA1_Update, \
	AROS_LCA(SHA_CTX *, (___c), A0), \
	AROS_LCA(const void *, (___data), A1), \
	AROS_LCA(unsigned long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 2512, Amissl)

#define SHA1_Final(___md, ___c) \
	AROS_LC2(int, SHA1_Final, \
	AROS_LCA(unsigned char *, (___md), A0), \
	AROS_LCA(SHA_CTX *, (___c), A1), \
	struct Library *, AMISSL_BASE_NAME, 2513, Amissl)

#define SHA1(___d, ___n, ___md) \
	AROS_LC3(unsigned char *, SHA1, \
	AROS_LCA(const unsigned char *, (___d), A0), \
	AROS_LCA(unsigned long, (___n), D0), \
	AROS_LCA(unsigned char *, (___md), A1), \
	struct Library *, AMISSL_BASE_NAME, 2514, Amissl)

#define SHA1_Transform(___c, ___data) \
	AROS_LC2(void, SHA1_Transform, \
	AROS_LCA(SHA_CTX *, (___c), A0), \
	AROS_LCA(const unsigned char *, (___data), A1), \
	struct Library *, AMISSL_BASE_NAME, 2515, Amissl)

#define HMAC_CTX_set_flags(___ctx, ___flags) \
	AROS_LC2(void, HMAC_CTX_set_flags, \
	AROS_LCA(HMAC_CTX *, (___ctx), A0), \
	AROS_LCA(unsigned long, (___flags), D0), \
	struct Library *, AMISSL_BASE_NAME, 2518, Amissl)

#define X509_check_ca(___x) \
	AROS_LC1(int, X509_check_ca, \
	AROS_LCA(X509 *, (___x), A0), \
	struct Library *, AMISSL_BASE_NAME, 2519, Amissl)

#define PROXY_POLICY_new() \
	AROS_LC0(PROXY_POLICY *, PROXY_POLICY_new, \
	struct Library *, AMISSL_BASE_NAME, 2520, Amissl)

#define PROXY_POLICY_free(___a) \
	AROS_LC1(void, PROXY_POLICY_free, \
	AROS_LCA(PROXY_POLICY *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 2521, Amissl)

#define d2i_PROXY_POLICY(___a, ___in, ___len) \
	AROS_LC3(PROXY_POLICY *, d2i_PROXY_POLICY, \
	AROS_LCA(PROXY_POLICY **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 2522, Amissl)

#define i2d_PROXY_POLICY(___a, ___out) \
	AROS_LC2(int, i2d_PROXY_POLICY, \
	AROS_LCA(PROXY_POLICY *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 2523, Amissl)

#define PROXY_POLICY_it() \
	AROS_LC0(const ASN1_ITEM *, PROXY_POLICY_it, \
	struct Library *, AMISSL_BASE_NAME, 2524, Amissl)

#define PROXY_CERT_INFO_EXTENSION_new() \
	AROS_LC0(PROXY_CERT_INFO_EXTENSION *, PROXY_CERT_INFO_EXTENSION_new, \
	struct Library *, AMISSL_BASE_NAME, 2525, Amissl)

#define PROXY_CERT_INFO_EXTENSION_free(___a) \
	AROS_LC1(void, PROXY_CERT_INFO_EXTENSION_free, \
	AROS_LCA(PROXY_CERT_INFO_EXTENSION *, (___a), A0), \
	struct Library *, AMISSL_BASE_NAME, 2526, Amissl)

#define d2i_PROXY_CERT_INFO_EXTENSION(___a, ___in, ___len) \
	AROS_LC3(PROXY_CERT_INFO_EXTENSION *, d2i_PROXY_CERT_INFO_EXTENSION, \
	AROS_LCA(PROXY_CERT_INFO_EXTENSION **, (___a), A0), \
	AROS_LCA(unsigned char **, (___in), A1), \
	AROS_LCA(long, (___len), D0), \
	struct Library *, AMISSL_BASE_NAME, 2527, Amissl)

#define i2d_PROXY_CERT_INFO_EXTENSION(___a, ___out) \
	AROS_LC2(int, i2d_PROXY_CERT_INFO_EXTENSION, \
	AROS_LCA(PROXY_CERT_INFO_EXTENSION *, (___a), A0), \
	AROS_LCA(unsigned char **, (___out), A1), \
	struct Library *, AMISSL_BASE_NAME, 2528, Amissl)

#define PROXY_CERT_INFO_EXTENSION_it() \
	AROS_LC0(const ASN1_ITEM *, PROXY_CERT_INFO_EXTENSION_it, \
	struct Library *, AMISSL_BASE_NAME, 2529, Amissl)

#endif /* !_INLINE_AMISSL_H */
