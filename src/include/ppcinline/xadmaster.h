/* Automatically generated header! Do not edit! */

#ifndef _PPCINLINE_XADMASTER_H
#define _PPCINLINE_XADMASTER_H

#ifndef __PPCINLINE_MACROS_H
#include <ppcinline/macros.h>
#endif /* !__PPCINLINE_MACROS_H */

#ifndef XADMASTER_BASE_NAME
#define XADMASTER_BASE_NAME xadMasterBase
#endif /* !XADMASTER_BASE_NAME */

#define xadFreeInfo(__p0) \
  LP1NR(54, xadFreeInfo, \
    struct xadArchiveInfo *, __p0, a0, \
    , XADMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define xadHookAccess(__p0, __p1, __p2, __p3) \
  LP4(84, xadERROR , xadHookAccess, \
    xadUINT32 , __p0, d0, \
    xadSignSize , __p1, d1, \
    xadPTR , __p2, a0, \
    struct xadArchiveInfo *, __p3, a1, \
    , XADMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define xadDiskUnArcA(__p0, __p1) \
  LP2(66, xadERROR , xadDiskUnArcA, \
    struct xadArchiveInfo *, __p0, a0, \
    const struct TagItem *, __p1, a1, \
    , XADMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define xadGetHookAccessA(__p0, __p1) \
  LP2(144, xadERROR , xadGetHookAccessA, \
    struct xadArchiveInfo *, __p0, a0, \
    const struct TagItem *, __p1, a1, \
    , XADMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define xadRecogFileA(__p0, __p1, __p2) \
  LP3(42, struct xadClient *, xadRecogFileA, \
    xadSize , __p0, d0, \
    const void *, __p1, a0, \
    const struct TagItem *, __p2, a1, \
    , XADMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define xadAllocObjectA(__p0, __p1) \
  LP2(30, xadPTR , xadAllocObjectA, \
    xadUINT32 , __p0, d0, \
    const struct TagItem *, __p1, a0, \
    , XADMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define xadCopyMem(__p0, __p1, __p2) \
  LP3NR(114, xadCopyMem, \
    const void *, __p0, a0, \
    xadPTR , __p1, a1, \
    xadSize , __p2, d0, \
    , XADMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define xadGetFilenameA(__p0, __p1, __p2, __p3, __p4) \
  LP5(168, xadERROR , xadGetFilenameA, \
    xadUINT32 , __p0, d0, \
    xadSTRPTR , __p1, a0, \
    const xadSTRING *, __p2, a1, \
    const xadSTRING *, __p3, a2, \
    const struct TagItem *, __p4, a3, \
    , XADMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define xadGetSystemInfo() \
  LP0(186, const struct xadSystemInfo *, xadGetSystemInfo, \
    , XADMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define xadFreeObjectA(__p0, __p1) \
  LP2NR(36, xadFreeObjectA, \
    xadPTR , __p0, a0, \
    const struct TagItem *, __p1, a1, \
    , XADMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define xadGetInfoA(__p0, __p1) \
  LP2(48, xadERROR , xadGetInfoA, \
    struct xadArchiveInfo *, __p0, a0, \
    const struct TagItem *, __p1, a1, \
    , XADMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define xadAllocVec(__p0, __p1) \
  LP2(108, xadPTR , xadAllocVec, \
    xadSize , __p0, d0, \
    xadUINT32 , __p1, d1, \
    , XADMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define xadGetErrorText(__p0) \
  LP1(72, xadSTRPTR , xadGetErrorText, \
    xadERROR , __p0, d0, \
    , XADMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define xadConvertProtectionA(__p0) \
  LP1(126, xadERROR , xadConvertProtectionA, \
    const struct TagItem *, __p0, a0, \
    , XADMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define xadAddFileEntryA(__p0, __p1, __p2) \
  LP3(156, xadERROR , xadAddFileEntryA, \
    struct xadFileInfo *, __p0, a0, \
    struct xadArchiveInfo *, __p1, a1, \
    const struct TagItem *, __p2, a2, \
    , XADMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define xadCalcCRC32(__p0, __p1, __p2, __p3) \
  LP4(102, xadUINT32 , xadCalcCRC32, \
    xadUINT32 , __p0, d0, \
    xadUINT32 , __p1, d1, \
    xadSize , __p2, d2, \
    const xadUINT8 *, __p3, a0, \
    , XADMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define xadConvertNameA(__p0, __p1) \
  LP2(174, xadSTRPTR , xadConvertNameA, \
    xadUINT32 , __p0, d0, \
    const struct TagItem *, __p1, a0, \
    , XADMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define xadCalcCRC16(__p0, __p1, __p2, __p3) \
  LP4(96, xadUINT16 , xadCalcCRC16, \
    xadUINT32 , __p0, d0, \
    xadUINT32 , __p1, d1, \
    xadSize , __p2, d2, \
    const xadUINT8 *, __p3, a0, \
    , XADMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define xadGetDiskInfoA(__p0, __p1) \
  LP2(132, xadERROR , xadGetDiskInfoA, \
    struct xadArchiveInfo *, __p0, a0, \
    const struct TagItem *, __p1, a1, \
    , XADMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define xadGetDefaultNameA(__p0) \
  LP1(180, xadSTRPTR , xadGetDefaultNameA, \
    const struct TagItem *, __p0, a0, \
    , XADMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define xadConvertDatesA(__p0) \
  LP1(90, xadERROR , xadConvertDatesA, \
    const struct TagItem *, __p0, a0, \
    , XADMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define xadFreeHookAccessA(__p0, __p1) \
  LP2NR(150, xadFreeHookAccessA, \
    struct xadArchiveInfo *, __p0, a0, \
    const struct TagItem *, __p1, a1, \
    , XADMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define xadGetClientInfo() \
  LP0(78, struct xadClient *, xadGetClientInfo, \
    , XADMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define xadAddDiskEntryA(__p0, __p1, __p2) \
  LP3(162, xadERROR , xadAddDiskEntryA, \
    struct xadDiskInfo *, __p0, a0, \
    struct xadArchiveInfo *, __p1, a1, \
    const struct TagItem *, __p2, a2, \
    , XADMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define xadHookTagAccessA(__p0, __p1, __p2, __p3, __p4) \
  LP5(120, xadERROR , xadHookTagAccessA, \
    xadUINT32 , __p0, d0, \
    xadSignSize , __p1, d1, \
    xadPTR , __p2, a0, \
    struct xadArchiveInfo *, __p3, a1, \
    const struct TagItem *, __p4, a2, \
    , XADMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define xadFileUnArcA(__p0, __p1) \
  LP2(60, xadERROR , xadFileUnArcA, \
    struct xadArchiveInfo *, __p0, a0, \
    const struct TagItem *, __p1, a1, \
    , XADMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#if defined(USE_INLINE_STDARG) && !defined(__STRICT_ANSI__)

#include <stdarg.h>

#define xadAllocObject(__p0, ...) \
  ({ULONG _tags[] = { __VA_ARGS__ }; \
  xadAllocObjectA(__p0, (const struct TagItem *)_tags);})

#define xadGetHookAccess(__p0, ...) \
  ({ULONG _tags[] = { __VA_ARGS__ }; \
  xadGetHookAccessA(__p0, (const struct TagItem *)_tags);})

#define xadFreeObject(__p0, ...) \
  ({ULONG _tags[] = { __VA_ARGS__ }; \
  xadFreeObjectA(__p0, (const struct TagItem *)_tags);})

#define xadConvertDates(...) \
  ({ULONG _tags[] = { __VA_ARGS__ }; \
  xadConvertDatesA((const struct TagItem *)_tags);})

#define xadAddDiskEntry(__p0, __p1, ...) \
  ({ULONG _tags[] = { __VA_ARGS__ }; \
  xadAddDiskEntryA(__p0, __p1, (const struct TagItem *)_tags);})

#define xadAddFileEntry(__p0, __p1, ...) \
  ({ULONG _tags[] = { __VA_ARGS__ }; \
  xadAddFileEntryA(__p0, __p1, (const struct TagItem *)_tags);})

#define xadGetFilename(__p0, __p1, __p2, __p3, ...) \
  ({ULONG _tags[] = { __VA_ARGS__ }; \
  xadGetFilenameA(__p0, __p1, __p2, __p3, (const struct TagItem *)_tags);})

#define xadConvertProtection(...) \
  ({ULONG _tags[] = { __VA_ARGS__ }; \
  xadConvertProtectionA((const struct TagItem *)_tags);})

#define xadConvertName(__p0, ...) \
  ({ULONG _tags[] = { __VA_ARGS__ }; \
  xadConvertNameA(__p0, (const struct TagItem *)_tags);})

#define xadRecogFile(__p0, __p1, ...) \
  ({ULONG _tags[] = { __VA_ARGS__ }; \
  xadRecogFileA(__p0, __p1, (const struct TagItem *)_tags);})

#define xadFreeHookAccess(__p0, ...) \
  ({ULONG _tags[] = { __VA_ARGS__ }; \
  xadFreeHookAccessA(__p0, (const struct TagItem *)_tags);})

#define xadFileUnArc(__p0, ...) \
  ({ULONG _tags[] = { __VA_ARGS__ }; \
  xadFileUnArcA(__p0, (const struct TagItem *)_tags);})

#define xadGetDiskInfo(__p0, ...) \
  ({ULONG _tags[] = { __VA_ARGS__ }; \
  xadGetDiskInfoA(__p0, (const struct TagItem *)_tags);})

#define xadDiskUnArc(__p0, ...) \
  ({ULONG _tags[] = { __VA_ARGS__ }; \
  xadDiskUnArcA(__p0, (const struct TagItem *)_tags);})

#define xadGetDefaultName(...) \
  ({ULONG _tags[] = { __VA_ARGS__ }; \
  xadGetDefaultNameA((const struct TagItem *)_tags);})

#define xadGetInfo(__p0, ...) \
  ({ULONG _tags[] = { __VA_ARGS__ }; \
  xadGetInfoA(__p0, (const struct TagItem *)_tags);})

#define xadHookTagAccess(__p0, __p1, __p2, __p3, ...) \
  ({ULONG _tags[] = { __VA_ARGS__ }; \
  xadHookTagAccessA(__p0, __p1, __p2, __p3, (const struct TagItem *)_tags);})

#endif

#endif /* !_PPCINLINE_XADMASTER_H */
