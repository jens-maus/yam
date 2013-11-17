/* Automatically generated header (sfdc 1.4)! Do not edit! */

#ifndef _INLINE_XPKMASTER_H
#define _INLINE_XPKMASTER_H

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

#ifndef XPKMASTER_BASE_NAME
#define XPKMASTER_BASE_NAME XpkBase
#endif /* !XPKMASTER_BASE_NAME */

#define XpkExamine(___fib, ___tags) \
  AROS_LC2(LONG, XpkExamine, \
  AROS_LCA(struct XpkFib *, (___fib), A0), \
  AROS_LCA(struct TagItem *, (___tags), A1), \
  struct Library *, XPKMASTER_BASE_NAME, 6, Xpkmaster)

#ifndef NO_INLINE_STDARG
#define XpkExamineTags(___fib, ___tags, ...) \
  ({_sfdc_vararg _tags[] = { ___tags, __VA_ARGS__ }; XpkExamine((___fib), (struct TagItem *) _tags); })
#endif /* !NO_INLINE_STDARG */

#define XpkPack(___tags) \
  AROS_LC1(LONG, XpkPack, \
  AROS_LCA(struct TagItem *, (___tags), A0), \
  struct Library *, XPKMASTER_BASE_NAME, 7, Xpkmaster)

#ifndef NO_INLINE_STDARG
#define XpkPackTags(___tags, ...) \
  ({_sfdc_vararg _tags[] = { ___tags, __VA_ARGS__ }; XpkPack((struct TagItem *) _tags); })
#endif /* !NO_INLINE_STDARG */

#define XpkUnpack(___tags) \
  AROS_LC1(LONG, XpkUnpack, \
  AROS_LCA(struct TagItem *, (___tags), A0), \
  struct Library *, XPKMASTER_BASE_NAME, 8, Xpkmaster)

#ifndef NO_INLINE_STDARG
#define XpkUnpackTags(___tags, ...) \
  ({_sfdc_vararg _tags[] = { ___tags, __VA_ARGS__ }; XpkUnpack((struct TagItem *) _tags); })
#endif /* !NO_INLINE_STDARG */

#define XpkOpen(___xbuf, ___tags) \
  AROS_LC2(LONG, XpkOpen, \
  AROS_LCA(struct XpkFib **, (___xbuf), A0), \
  AROS_LCA(struct TagItem *, (___tags), A1), \
  struct Library *, XPKMASTER_BASE_NAME, 9, Xpkmaster)

#ifndef NO_INLINE_STDARG
#define XpkOpenTags(___xbuf, ___tags, ...) \
  ({_sfdc_vararg _tags[] = { ___tags, __VA_ARGS__ }; XpkOpen((___xbuf), (struct TagItem *) _tags); })
#endif /* !NO_INLINE_STDARG */

#define XpkRead(___xbuf, ___buf, ___len) \
  AROS_LC3(LONG, XpkRead, \
  AROS_LCA(struct XpkFib *, (___xbuf), A0), \
  AROS_LCA(STRPTR, (___buf), A1), \
  AROS_LCA(ULONG, (___len), D0), \
  struct Library *, XPKMASTER_BASE_NAME, 10, Xpkmaster)

#define XpkWrite(___xbuf, ___buf, ___len) \
  AROS_LC3(LONG, XpkWrite, \
  AROS_LCA(struct XpkFib *, (___xbuf), A0), \
  AROS_LCA(STRPTR, (___buf), A1), \
  AROS_LCA(LONG, (___len), D0), \
  struct Library *, XPKMASTER_BASE_NAME, 11, Xpkmaster)

#define XpkSeek(___xbuf, ___len, ___mode) \
  AROS_LC3(LONG, XpkSeek, \
  AROS_LCA(struct XpkFib *, (___xbuf), A0), \
  AROS_LCA(LONG, (___len), D0), \
  AROS_LCA(LONG, (___mode), D1), \
  struct Library *, XPKMASTER_BASE_NAME, 12, Xpkmaster)

#define XpkClose(___xbuf) \
  AROS_LC1(LONG, XpkClose, \
  AROS_LCA(struct XpkFib *, (___xbuf), A0), \
  struct Library *, XPKMASTER_BASE_NAME, 13, Xpkmaster)

#define XpkQuery(___tags) \
  AROS_LC1(LONG, XpkQuery, \
  AROS_LCA(struct TagItem *, (___tags), A0), \
  struct Library *, XPKMASTER_BASE_NAME, 14, Xpkmaster)

#ifndef NO_INLINE_STDARG
#define XpkQueryTags(___tags, ...) \
  ({_sfdc_vararg _tags[] = { ___tags, __VA_ARGS__ }; XpkQuery((struct TagItem *) _tags); })
#endif /* !NO_INLINE_STDARG */

#define XpkAllocObject(___type, ___tags) \
  AROS_LC2(APTR, XpkAllocObject, \
  AROS_LCA(ULONG, (___type), D0), \
  AROS_LCA(struct TagItem *, (___tags), A0), \
  struct Library *, XPKMASTER_BASE_NAME, 15, Xpkmaster)

#ifndef NO_INLINE_STDARG
#define XpkAllocObjectTags(___type, ___tags, ...) \
  ({_sfdc_vararg _tags[] = { ___tags, __VA_ARGS__ }; XpkAllocObject((___type), (struct TagItem *) _tags); })
#endif /* !NO_INLINE_STDARG */

#define XpkFreeObject(___type, ___object) \
  AROS_LC2(void, XpkFreeObject, \
  AROS_LCA(ULONG, (___type), D0), \
  AROS_LCA(APTR, (___object), A0), \
  struct Library *, XPKMASTER_BASE_NAME, 16, Xpkmaster)

#define XpkPrintFault(___code, ___header) \
  AROS_LC2(BOOL, XpkPrintFault, \
  AROS_LCA(LONG, (___code), D0), \
  AROS_LCA(STRPTR, (___header), A0), \
  struct Library *, XPKMASTER_BASE_NAME, 17, Xpkmaster)

#define XpkFault(___code, ___header, ___buffer, ___size) \
  AROS_LC4(ULONG, XpkFault, \
  AROS_LCA(LONG, (___code), D0), \
  AROS_LCA(STRPTR, (___header), A0), \
  AROS_LCA(STRPTR, (___buffer), A1), \
  AROS_LCA(ULONG, (___size), D1), \
  struct Library *, XPKMASTER_BASE_NAME, 18, Xpkmaster)

#define XpkPassRequest(___tags) \
  AROS_LC1(LONG, XpkPassRequest, \
  AROS_LCA(struct TagItem *, (___tags), A0), \
  struct Library *, XPKMASTER_BASE_NAME, 19, Xpkmaster)

#ifndef NO_INLINE_STDARG
#define XpkPassRequestTags(___tags, ...) \
  ({_sfdc_vararg _tags[] = { ___tags, __VA_ARGS__ }; XpkPassRequest((struct TagItem *) _tags); })
#endif /* !NO_INLINE_STDARG */

#endif /* !_INLINE_XPKMASTER_H */
