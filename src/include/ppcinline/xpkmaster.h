/* Automatically generated header! Do not edit! */

#ifndef _PPCINLINE_XPKMASTER_H
#define _PPCINLINE_XPKMASTER_H

#ifndef __PPCINLINE_MACROS_H
#include <ppcinline/macros.h>
#endif /* !__PPCINLINE_MACROS_H */

#ifndef XPKMASTER_BASE_NAME
#define XPKMASTER_BASE_NAME XpkBase
#endif /* !XPKMASTER_BASE_NAME */

#define XpkOpen(__p0, __p1) \
  LP2(54, LONG , XpkOpen, \
    struct XpkFib **, __p0, a0, \
    struct TagItem *, __p1, a1, \
    , XPKMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XpkQuery(__p0) \
  LP1(84, LONG , XpkQuery, \
    struct TagItem *, __p0, a0, \
    , XPKMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XpkClose(__p0) \
  LP1(78, LONG , XpkClose, \
    struct XpkFib *, __p0, a0, \
    , XPKMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XpkFreeObject(__p0, __p1) \
  LP2NR(96, XpkFreeObject, \
    ULONG , __p0, d0, \
    APTR , __p1, a0, \
    , XPKMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XpkRead(__p0, __p1, __p2) \
  LP3(60, LONG , XpkRead, \
    struct XpkFib *, __p0, a0, \
    STRPTR , __p1, a1, \
    ULONG , __p2, d0, \
    , XPKMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XpkFault(__p0, __p1, __p2, __p3) \
  LP4(108, ULONG , XpkFault, \
    LONG , __p0, d0, \
    STRPTR , __p1, a0, \
    STRPTR , __p2, a1, \
    ULONG , __p3, d1, \
    , XPKMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XpkPack(__p0) \
  LP1(42, LONG , XpkPack, \
    struct TagItem *, __p0, a0, \
    , XPKMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XpkPassRequest(__p0) \
  LP1(114, LONG , XpkPassRequest, \
    struct TagItem *, __p0, a0, \
    , XPKMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XpkUnpack(__p0) \
  LP1(48, LONG , XpkUnpack, \
    struct TagItem *, __p0, a0, \
    , XPKMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XpkAllocObject(__p0, __p1) \
  LP2(90, APTR , XpkAllocObject, \
    ULONG , __p0, d0, \
    struct TagItem *, __p1, a0, \
    , XPKMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XpkExamine(__p0, __p1) \
  LP2(36, LONG , XpkExamine, \
    struct XpkFib *, __p0, a0, \
    struct TagItem *, __p1, a1, \
    , XPKMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XpkSeek(__p0, __p1, __p2) \
  LP3(72, LONG , XpkSeek, \
    struct XpkFib *, __p0, a0, \
    LONG , __p1, d0, \
    LONG , __p2, d1, \
    , XPKMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XpkPrintFault(__p0, __p1) \
  LP2(102, BOOL , XpkPrintFault, \
    LONG , __p0, d0, \
    STRPTR , __p1, a0, \
    , XPKMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define XpkWrite(__p0, __p1, __p2) \
  LP3(66, LONG , XpkWrite, \
    struct XpkFib *, __p0, a0, \
    STRPTR , __p1, a1, \
    LONG , __p2, d0, \
    , XPKMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#ifdef USE_INLINE_STDARG

#include <stdarg.h>

#define XpkPackTags(...) \
  ({ULONG _tags[] = { __VA_ARGS__ }; \
  XpkPack((struct TagItem *)_tags);})

#define XpkPassRequestTags(...) \
  ({ULONG _tags[] = { __VA_ARGS__ }; \
  XpkPassRequest((struct TagItem *)_tags);})

#define XpkUnpackTags(...) \
  ({ULONG _tags[] = { __VA_ARGS__ }; \
  XpkUnpack((struct TagItem *)_tags);})

#define XpkAllocObjectTags(__p0, ...) \
  ({ULONG _tags[] = { __VA_ARGS__ }; \
  XpkAllocObject(__p0, (struct TagItem *)_tags);})

#define XpkExamineTags(__p0, ...) \
  ({ULONG _tags[] = { __VA_ARGS__ }; \
  XpkExamine(__p0, (struct TagItem *)_tags);})

#define XpkOpenTags(__p0, ...) \
  ({ULONG _tags[] = { __VA_ARGS__ }; \
  XpkOpen(__p0, (struct TagItem *)_tags);})

#define XpkQueryTags(...) \
  ({ULONG _tags[] = { __VA_ARGS__ }; \
  XpkQuery((struct TagItem *)_tags);})

#endif

#endif /* !_PPCINLINE_XPKMASTER_H */
