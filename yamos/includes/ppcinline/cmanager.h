/* Automatically generated header! Do not edit! */

#ifndef _PPCINLINE_CMANAGER_H
#define _PPCINLINE_CMANAGER_H

#ifndef __PPCINLINE_MACROS_H
#include <ppcinline/macros.h>
#endif /* !__PPCINLINE_MACROS_H */

#ifndef CMANAGER_BASE_NAME
#define CMANAGER_BASE_NAME CManagerBase
#endif /* !CMANAGER_BASE_NAME */

#define CM_CreateBitMap(__p0, __p1, __p2, __p3, __p4) \
	LP5(84, struct BitMap *, CM_CreateBitMap, \
		ULONG , __p0, d0, \
		ULONG , __p1, d1, \
		ULONG , __p2, d2, \
		ULONG , __p3, d3, \
		struct BitMap *, __p4, a0, \
		, CMANAGER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CM_GetEntry(__p0, __p1) \
	LP2(78, APTR , CM_GetEntry, \
		APTR , __p0, a0, \
		ULONG , __p1, d0, \
		, CMANAGER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CM_FreeEntry(__p0) \
	LP1NR(66, CM_FreeEntry, \
		APTR , __p0, a0, \
		, CMANAGER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CM_SaveData(__p0, __p1, __p2) \
	LP3NR(42, CM_SaveData, \
		STRPTR , __p0, a0, \
		struct CMData *, __p1, a1, \
		STRPTR , __p2, a2, \
		, CMANAGER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CM_StartManager(__p0, __p1) \
	LP2(30, APTR , CM_StartManager, \
		STRPTR , __p0, a0, \
		STRPTR , __p1, a1, \
		, CMANAGER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CM_GetParent(__p0, __p1) \
	LP2(54, struct CMGroup *, CM_GetParent, \
		struct CMGroup *, __p0, a0, \
		struct CMGroup *, __p1, a1, \
		, CMANAGER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CM_LoadData(__p0, __p1, __p2) \
	LP3(36, BOOL , CM_LoadData, \
		STRPTR , __p0, a0, \
		struct CMData *, __p1, a1, \
		STRPTR , __p2, a2, \
		, CMANAGER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CM_AddEntry(__p0) \
	LP1(96, BOOL , CM_AddEntry, \
		APTR , __p0, a0, \
		, CMANAGER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CM_FreeData(__p0) \
	LP1NR(48, CM_FreeData, \
		struct CMData *, __p0, a0, \
		, CMANAGER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CM_AllocEntry(__p0) \
	LP1(60, APTR , CM_AllocEntry, \
		ULONG , __p0, d0, \
		, CMANAGER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CM_FreeHandle(__p0, __p1) \
	LP2NR(72, CM_FreeHandle, \
		APTR , __p0, a0, \
		BOOL , __p1, d0, \
		, CMANAGER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CM_FreeList(__p0) \
	LP1NR(102, CM_FreeList, \
		struct MinList *, __p0, a0, \
		, CMANAGER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CM_DeleteBitMap(__p0) \
	LP1NR(90, CM_DeleteBitMap, \
		struct BitMap *, __p0, a0, \
		, CMANAGER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#endif /* !_PPCINLINE_CMANAGER_H */
