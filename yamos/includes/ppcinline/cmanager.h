#ifndef _PPCINLINE_CMANAGER_H
#define _PPCINLINE_CMANAGER_H

#ifndef __PPCINLINE_MACROS_H
#include <ppcinline/macros.h>
#endif

#ifndef CMANAGER_BASE_NAME
#define CMANAGER_BASE_NAME CManagerBase
#endif

#define CM_StartManager(par1, last) \
	LP2(0x1e, APTR, CM_StartManager, STRPTR, par1, a0, STRPTR, last, a1, \
	, CMANAGER_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CM_LoadData(par1, par2, last) \
	LP3(0x24, BOOL, CM_LoadData, STRPTR, par1, a0, struct CMData *, par2, a1, STRPTR, last, a2, \
	, CMANAGER_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CM_SaveData(par1, par2, last) \
	LP3NR(0x2a, CM_SaveData, STRPTR, par1, a0, struct CMData *, par2, a1, STRPTR, last, a2, \
	, CMANAGER_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CM_FreeData(last) \
	LP1NR(0x30, CM_FreeData, struct CMData *, last, a0, \
	, CMANAGER_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CM_GetParent(par1, last) \
	LP2(0x36, struct CMGroup  *, CM_GetParent, struct CMGroup *, par1, a0, struct CMGroup *, last, a1, \
	, CMANAGER_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CM_AllocEntry(last) \
	LP1(0x3c, APTR, CM_AllocEntry, ULONG, last, d0, \
	, CMANAGER_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CM_FreeEntry(last) \
	LP1NR(0x42, CM_FreeEntry, APTR, last, a0, \
	, CMANAGER_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CM_FreeHandle(par1, last) \
	LP2NR(0x48, CM_FreeHandle, APTR, par1, a0, BOOL, last, d0, \
	, CMANAGER_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CM_GetEntry(par1, last) \
	LP2(0x4e, APTR, CM_GetEntry, APTR, par1, a0, ULONG, last, d0, \
	, CMANAGER_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CM_CreateBitMap(par1, par2, par3, par4, last) \
	LP5(0x54, struct BitMap   *, CM_CreateBitMap, ULONG, par1, d0, ULONG, par2, d1, ULONG, par3, d2, ULONG, par4, d3, struct BitMap *, last, a0, \
	, CMANAGER_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CM_DeleteBitMap(last) \
	LP1NR(0x5a, CM_DeleteBitMap, struct BitMap *, last, a0, \
	, CMANAGER_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CM_AddEntry(last) \
	LP1(0x60, BOOL, CM_AddEntry, APTR, last, a0, \
	, CMANAGER_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CM_FreeList(last) \
	LP1NR(0x66, CM_FreeList, struct MinList *, last, a0, \
	, CMANAGER_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#endif /*  _PPCINLINE_CMANAGER_H  */
