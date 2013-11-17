/* Automatically generated header (sfdc 1.4)! Do not edit! */

#ifndef _INLINE_XADMASTER_H
#define _INLINE_XADMASTER_H

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

#ifndef XADMASTER_BASE_NAME
#define XADMASTER_BASE_NAME xadMasterBase
#endif /* !XADMASTER_BASE_NAME */

#define xadAllocObjectA(___type, ___tags) \
  AROS_LC2(xadPTR, xadAllocObjectA, \
  AROS_LCA(xadUINT32, (___type), D0), \
  AROS_LCA(const struct TagItem *, (___tags), A0), \
  struct xadMasterBase *, XADMASTER_BASE_NAME, 5, Xadmaster)

#ifndef NO_INLINE_STDARG
#define xadAllocObject(___type, ___tags, ...) \
  ({_sfdc_vararg _tags[] = { ___tags, __VA_ARGS__ }; xadAllocObjectA((___type), (const struct TagItem *) _tags); })
#endif /* !NO_INLINE_STDARG */

#define xadFreeObjectA(___object, ___tags) \
  AROS_LC2(void, xadFreeObjectA, \
  AROS_LCA(xadPTR, (___object), A0), \
  AROS_LCA(const struct TagItem *, (___tags), A1), \
  struct xadMasterBase *, XADMASTER_BASE_NAME, 6, Xadmaster)

#ifndef NO_INLINE_STDARG
#define xadFreeObject(___object, ___tags, ...) \
  ({_sfdc_vararg _tags[] = { ___tags, __VA_ARGS__ }; xadFreeObjectA((___object), (const struct TagItem *) _tags); })
#endif /* !NO_INLINE_STDARG */

#define xadRecogFileA(___size, ___memory, ___tags) \
  AROS_LC3(struct xadClient *, xadRecogFileA, \
  AROS_LCA(xadSize, (___size), D0), \
  AROS_LCA(const void *, (___memory), A0), \
  AROS_LCA(const struct TagItem *, (___tags), A1), \
  struct xadMasterBase *, XADMASTER_BASE_NAME, 7, Xadmaster)

#ifndef NO_INLINE_STDARG
#define xadRecogFile(___size, ___memory, ___tags, ...) \
  ({_sfdc_vararg _tags[] = { ___tags, __VA_ARGS__ }; xadRecogFileA((___size), (___memory), (const struct TagItem *) _tags); })
#endif /* !NO_INLINE_STDARG */

#define xadGetInfoA(___ai, ___tags) \
  AROS_LC2(xadERROR, xadGetInfoA, \
  AROS_LCA(struct xadArchiveInfo *, (___ai), A0), \
  AROS_LCA(const struct TagItem *, (___tags), A1), \
  struct xadMasterBase *, XADMASTER_BASE_NAME, 8, Xadmaster)

#ifndef NO_INLINE_STDARG
#define xadGetInfo(___ai, ___tags, ...) \
  ({_sfdc_vararg _tags[] = { ___tags, __VA_ARGS__ }; xadGetInfoA((___ai), (const struct TagItem *) _tags); })
#endif /* !NO_INLINE_STDARG */

#define xadFreeInfo(___ai) \
  AROS_LC1(void, xadFreeInfo, \
  AROS_LCA(struct xadArchiveInfo *, (___ai), A0), \
  struct xadMasterBase *, XADMASTER_BASE_NAME, 9, Xadmaster)

#define xadFileUnArcA(___ai, ___tags) \
  AROS_LC2(xadERROR, xadFileUnArcA, \
  AROS_LCA(struct xadArchiveInfo *, (___ai), A0), \
  AROS_LCA(const struct TagItem *, (___tags), A1), \
  struct xadMasterBase *, XADMASTER_BASE_NAME, 10, Xadmaster)

#ifndef NO_INLINE_STDARG
#define xadFileUnArc(___ai, ___tags, ...) \
  ({_sfdc_vararg _tags[] = { ___tags, __VA_ARGS__ }; xadFileUnArcA((___ai), (const struct TagItem *) _tags); })
#endif /* !NO_INLINE_STDARG */

#define xadDiskUnArcA(___ai, ___tags) \
  AROS_LC2(xadERROR, xadDiskUnArcA, \
  AROS_LCA(struct xadArchiveInfo *, (___ai), A0), \
  AROS_LCA(const struct TagItem *, (___tags), A1), \
  struct xadMasterBase *, XADMASTER_BASE_NAME, 11, Xadmaster)

#ifndef NO_INLINE_STDARG
#define xadDiskUnArc(___ai, ___tags, ...) \
  ({_sfdc_vararg _tags[] = { ___tags, __VA_ARGS__ }; xadDiskUnArcA((___ai), (const struct TagItem *) _tags); })
#endif /* !NO_INLINE_STDARG */

#define xadGetErrorText(___errnum) \
  AROS_LC1(xadSTRPTR, xadGetErrorText, \
  AROS_LCA(xadERROR, (___errnum), D0), \
  struct xadMasterBase *, XADMASTER_BASE_NAME, 12, Xadmaster)

#define xadGetClientInfo() \
  AROS_LC0(struct xadClient *, xadGetClientInfo, \
  struct xadMasterBase *, XADMASTER_BASE_NAME, 13, Xadmaster)

#define xadHookAccess(___command, ___data, ___buffer, ___ai) \
  AROS_LC4(xadERROR, xadHookAccess, \
  AROS_LCA(xadUINT32, (___command), D0), \
  AROS_LCA(xadSignSize, (___data), D1), \
  AROS_LCA(xadPTR, (___buffer), A0), \
  AROS_LCA(struct xadArchiveInfo *, (___ai), A1), \
  struct xadMasterBase *, XADMASTER_BASE_NAME, 14, Xadmaster)

#define xadConvertDatesA(___tags) \
  AROS_LC1(xadERROR, xadConvertDatesA, \
  AROS_LCA(const struct TagItem *, (___tags), A0), \
  struct xadMasterBase *, XADMASTER_BASE_NAME, 15, Xadmaster)

#ifndef NO_INLINE_STDARG
#define xadConvertDates(___tags, ...) \
  ({_sfdc_vararg _tags[] = { ___tags, __VA_ARGS__ }; xadConvertDatesA((const struct TagItem *) _tags); })
#endif /* !NO_INLINE_STDARG */

#define xadCalcCRC16(___id, ___init, ___size, ___buffer) \
  AROS_LC4(xadUINT16, xadCalcCRC16, \
  AROS_LCA(xadUINT16, (___id), D0), \
  AROS_LCA(xadUINT16, (___init), D1), \
  AROS_LCA(xadSize, (___size), D2), \
  AROS_LCA(const xadUINT8 *, (___buffer), A0), \
  struct xadMasterBase *, XADMASTER_BASE_NAME, 16, Xadmaster)

#define xadCalcCRC32(___id, ___init, ___size, ___buffer) \
  AROS_LC4(xadUINT32, xadCalcCRC32, \
  AROS_LCA(xadUINT32, (___id), D0), \
  AROS_LCA(xadUINT32, (___init), D1), \
  AROS_LCA(xadSize, (___size), D2), \
  AROS_LCA(const xadUINT8 *, (___buffer), A0), \
  struct xadMasterBase *, XADMASTER_BASE_NAME, 17, Xadmaster)

#define xadAllocVec(___size, ___flags) \
  AROS_LC2(xadPTR, xadAllocVec, \
  AROS_LCA(xadSize, (___size), D0), \
  AROS_LCA(xadUINT32, (___flags), D1), \
  struct xadMasterBase *, XADMASTER_BASE_NAME, 18, Xadmaster)

#define xadCopyMem(___src, ___dest, ___size) \
  AROS_LC3(void, xadCopyMem, \
  AROS_LCA(const void *, (___src), A0), \
  AROS_LCA(xadPTR, (___dest), A1), \
  AROS_LCA(xadSize, (___size), D0), \
  struct xadMasterBase *, XADMASTER_BASE_NAME, 19, Xadmaster)

#define xadHookTagAccessA(___command, ___data, ___buffer, ___ai, ___tags) \
  AROS_LC5(xadERROR, xadHookTagAccessA, \
  AROS_LCA(xadUINT32, (___command), D0), \
  AROS_LCA(xadSignSize, (___data), D1), \
  AROS_LCA(xadPTR, (___buffer), A0), \
  AROS_LCA(struct xadArchiveInfo *, (___ai), A1), \
  AROS_LCA(const struct TagItem *, (___tags), A2), \
  struct xadMasterBase *, XADMASTER_BASE_NAME, 20, Xadmaster)

#ifndef NO_INLINE_STDARG
#define xadHookTagAccess(___command, ___data, ___buffer, ___ai, ___tags, ...) \
  ({_sfdc_vararg _tags[] = { ___tags, __VA_ARGS__ }; xadHookTagAccessA((___command), (___data), (___buffer), (___ai), (const struct TagItem *) _tags); })
#endif /* !NO_INLINE_STDARG */

#define xadConvertProtectionA(___tags) \
  AROS_LC1(xadERROR, xadConvertProtectionA, \
  AROS_LCA(const struct TagItem *, (___tags), A0), \
  struct xadMasterBase *, XADMASTER_BASE_NAME, 21, Xadmaster)

#ifndef NO_INLINE_STDARG
#define xadConvertProtection(___tags, ...) \
  ({_sfdc_vararg _tags[] = { ___tags, __VA_ARGS__ }; xadConvertProtectionA((const struct TagItem *) _tags); })
#endif /* !NO_INLINE_STDARG */

#define xadGetDiskInfoA(___ai, ___tags) \
  AROS_LC2(xadERROR, xadGetDiskInfoA, \
  AROS_LCA(struct xadArchiveInfo *, (___ai), A0), \
  AROS_LCA(const struct TagItem *, (___tags), A1), \
  struct xadMasterBase *, XADMASTER_BASE_NAME, 22, Xadmaster)

#ifndef NO_INLINE_STDARG
#define xadGetDiskInfo(___ai, ___tags, ...) \
  ({_sfdc_vararg _tags[] = { ___tags, __VA_ARGS__ }; xadGetDiskInfoA((___ai), (const struct TagItem *) _tags); })
#endif /* !NO_INLINE_STDARG */

#define xadGetHookAccessA(___ai, ___tags) \
  AROS_LC2(xadERROR, xadGetHookAccessA, \
  AROS_LCA(struct xadArchiveInfo *, (___ai), A0), \
  AROS_LCA(const struct TagItem *, (___tags), A1), \
  struct xadMasterBase *, XADMASTER_BASE_NAME, 24, Xadmaster)

#ifndef NO_INLINE_STDARG
#define xadGetHookAccess(___ai, ___tags, ...) \
  ({_sfdc_vararg _tags[] = { ___tags, __VA_ARGS__ }; xadGetHookAccessA((___ai), (const struct TagItem *) _tags); })
#endif /* !NO_INLINE_STDARG */

#define xadFreeHookAccessA(___ai, ___tags) \
  AROS_LC2(void, xadFreeHookAccessA, \
  AROS_LCA(struct xadArchiveInfo *, (___ai), A0), \
  AROS_LCA(const struct TagItem *, (___tags), A1), \
  struct xadMasterBase *, XADMASTER_BASE_NAME, 25, Xadmaster)

#ifndef NO_INLINE_STDARG
#define xadFreeHookAccess(___ai, ___tags, ...) \
  ({_sfdc_vararg _tags[] = { ___tags, __VA_ARGS__ }; xadFreeHookAccessA((___ai), (const struct TagItem *) _tags); })
#endif /* !NO_INLINE_STDARG */

#define xadAddFileEntryA(___fi, ___ai, ___tags) \
  AROS_LC3(xadERROR, xadAddFileEntryA, \
  AROS_LCA(struct xadFileInfo *, (___fi), A0), \
  AROS_LCA(struct xadArchiveInfo *, (___ai), A1), \
  AROS_LCA(const struct TagItem *, (___tags), A2), \
  struct xadMasterBase *, XADMASTER_BASE_NAME, 26, Xadmaster)

#ifndef NO_INLINE_STDARG
#define xadAddFileEntry(___fi, ___ai, ___tags, ...) \
  ({_sfdc_vararg _tags[] = { ___tags, __VA_ARGS__ }; xadAddFileEntryA((___fi), (___ai), (const struct TagItem *) _tags); })
#endif /* !NO_INLINE_STDARG */

#define xadAddDiskEntryA(___di, ___ai, ___tags) \
  AROS_LC3(xadERROR, xadAddDiskEntryA, \
  AROS_LCA(struct xadDiskInfo *, (___di), A0), \
  AROS_LCA(struct xadArchiveInfo *, (___ai), A1), \
  AROS_LCA(const struct TagItem *, (___tags), A2), \
  struct xadMasterBase *, XADMASTER_BASE_NAME, 27, Xadmaster)

#ifndef NO_INLINE_STDARG
#define xadAddDiskEntry(___di, ___ai, ___tags, ...) \
  ({_sfdc_vararg _tags[] = { ___tags, __VA_ARGS__ }; xadAddDiskEntryA((___di), (___ai), (const struct TagItem *) _tags); })
#endif /* !NO_INLINE_STDARG */

#define xadGetFilenameA(___buffersize, ___buffer, ___path, ___name, ___tags) \
  AROS_LC5(xadERROR, xadGetFilenameA, \
  AROS_LCA(xadUINT32, (___buffersize), D0), \
  AROS_LCA(xadSTRPTR, (___buffer), A0), \
  AROS_LCA(const xadSTRING *, (___path), A1), \
  AROS_LCA(const xadSTRING *, (___name), A2), \
  AROS_LCA(const struct TagItem *, (___tags), A3), \
  struct xadMasterBase *, XADMASTER_BASE_NAME, 28, Xadmaster)

#ifndef NO_INLINE_STDARG
#define xadGetFilename(___buffersize, ___buffer, ___path, ___name, ___tags, ...) \
  ({_sfdc_vararg _tags[] = { ___tags, __VA_ARGS__ }; xadGetFilenameA((___buffersize), (___buffer), (___path), (___name), (const struct TagItem *) _tags); })
#endif /* !NO_INLINE_STDARG */

#define xadConvertNameA(___charset, ___tags) \
  AROS_LC2(xadSTRPTR, xadConvertNameA, \
  AROS_LCA(xadUINT32, (___charset), D0), \
  AROS_LCA(const struct TagItem *, (___tags), A0), \
  struct xadMasterBase *, XADMASTER_BASE_NAME, 29, Xadmaster)

#ifndef NO_INLINE_STDARG
#define xadConvertName(___charset, ___tags, ...) \
  ({_sfdc_vararg _tags[] = { ___tags, __VA_ARGS__ }; xadConvertNameA((___charset), (const struct TagItem *) _tags); })
#endif /* !NO_INLINE_STDARG */

#define xadGetDefaultNameA(___tags) \
  AROS_LC1(xadSTRPTR, xadGetDefaultNameA, \
  AROS_LCA(const struct TagItem *, (___tags), A0), \
  struct xadMasterBase *, XADMASTER_BASE_NAME, 30, Xadmaster)

#ifndef NO_INLINE_STDARG
#define xadGetDefaultName(___tags, ...) \
  ({_sfdc_vararg _tags[] = { ___tags, __VA_ARGS__ }; xadGetDefaultNameA((const struct TagItem *) _tags); })
#endif /* !NO_INLINE_STDARG */

#define xadGetSystemInfo() \
  AROS_LC0(const struct xadSystemInfo *, xadGetSystemInfo, \
  struct xadMasterBase *, XADMASTER_BASE_NAME, 31, Xadmaster)

#endif /* !_INLINE_XADMASTER_H */
