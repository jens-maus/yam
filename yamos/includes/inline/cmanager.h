/* Automatically generated header! Do not edit! */

#ifndef _INLINE_CMANAGER_H
#define _INLINE_CMANAGER_H

#ifndef __INLINE_MACROS_H
#include <inline/macros.h>
#endif /* !__INLINE_MACROS_H */

#ifndef CMANAGER_BASE_NAME
#define CMANAGER_BASE_NAME CManagerBase
#endif /* !CMANAGER_BASE_NAME */

#define CM_AddEntry(entry) \
	LP1(0x60, extern BOOL, CM_AddEntry, APTR, entry, a0, \
	, CMANAGER_BASE_NAME)

#define CM_AllocEntry(type) \
	LP1(0x3c, extern APTR, CM_AllocEntry, ULONG, type, d0, \
	, CMANAGER_BASE_NAME)

#define CM_CreateBitMap(width, height, depth, flags, friend) \
	LP5(0x54, extern struct BitMap   *, CM_CreateBitMap, ULONG, width, d0, ULONG, height, d1, ULONG, depth, d2, ULONG, flags, d3, struct BitMap *, friend, a0, \
	, CMANAGER_BASE_NAME)

#define CM_DeleteBitMap(bitmap) \
	LP1(0x5a, extern void, CM_DeleteBitMap, struct BitMap *, bitmap, a0, \
	, CMANAGER_BASE_NAME)

#define CM_FreeData(data) \
	LP1(0x7e, extern void, CM_FreeData, struct CMData *, data, a0, \
	, CMANAGER_BASE_NAME)

#define CM_FreeEntry(entry) \
	LP1(0x42, extern void, CM_FreeEntry, APTR, entry, a0, \
	, CMANAGER_BASE_NAME)

#define CM_FreeHandle(handle, close) \
	LP2(0x48, extern void, CM_FreeHandle, APTR, handle, a0, BOOL, close, d0, \
	, CMANAGER_BASE_NAME)

#define CM_FreeList(list) \
	LP1(0x66, extern void, CM_FreeList, struct MinList *, list, a0, \
	, CMANAGER_BASE_NAME)

#define CM_GetEntry(handle, flags) \
	LP2(0x4e, extern APTR, CM_GetEntry, APTR, handle, a0, ULONG, flags, d0, \
	, CMANAGER_BASE_NAME)

#define CM_GetParent(list, current) \
	LP2(0x36, extern struct CMGroup  *, CM_GetParent, struct CMGroup *, list, a0, struct CMGroup *, current, a1, \
	, CMANAGER_BASE_NAME)

#define CM_LoadData(prefs, data, user) \
	LP3(0x72, extern BOOL, CM_LoadData, STRPTR, prefs, a0, struct CMData *, data, a1, STRPTR, user, a2, \
	, CMANAGER_BASE_NAME)

#define CM_SaveData(prefs, data, user) \
	LP3(0x78, extern void, CM_SaveData, STRPTR, prefs, a0, struct CMData *, data, a1, STRPTR, user, a2, \
	, CMANAGER_BASE_NAME)

#define CM_StartManager(file, pubscreen) \
	LP2(0x1e, extern APTR, CM_StartManager, STRPTR, file, a0, STRPTR, pubscreen, a1, \
	, CMANAGER_BASE_NAME)

#endif /* !_INLINE_CMANAGER_H */
