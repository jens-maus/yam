#ifndef _PPCINLINE_PM_H
#define _PPCINLINE_PM_H

#ifndef __PPCINLINE_MACROS_H
#include <ppcinline/macros.h>
#endif

#ifndef PM_BASE_NAME
#define PM_BASE_NAME PopupMenuBase
#endif

#define PM_MakeMenuA(last) \
	LP1(0x1e, struct PopupMenu *, PM_MakeMenuA, struct TagItem *, last, a1, \
	, PM_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define PM_MakeMenu(tags...) \
	({ULONG _tags[] = {tags}; PM_MakeMenuA((struct TagItem *) _tags);})
#endif

#define PM_MakeItemA(last) \
	LP1(0x24, struct PopupMenu *, PM_MakeItemA, struct TagItem *, last, a1, \
	, PM_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define PM_MakeItem(tags...) \
	({ULONG _tags[] = {tags}; PM_MakeItemA((struct TagItem *) _tags);})
#endif

#define PM_FreePopupMenu(last) \
	LP1NR(0x2a, PM_FreePopupMenu, struct PopupMenu *, last, a1, \
	, PM_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PM_OpenPopupMenuA(par1, last) \
	LP2(0x30, ULONG, PM_OpenPopupMenuA, struct Window *, par1, a1, struct TagItem *, last, a2, \
	, PM_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define PM_OpenPopupMenu(par1, tags...) \
	({ULONG _tags[] = {tags}; PM_OpenPopupMenuA((par1), (struct TagItem *) _tags);})
#endif

#define PM_MakeIDListA(last) \
	LP1(0x36, struct PM_IDLst *, PM_MakeIDListA, struct TagItem *, last, a1, \
	, PM_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define PM_MakeIDList(tags...) \
	({ULONG _tags[] = {tags}; PM_MakeIDListA((struct TagItem *) _tags);})
#endif

#define PM_ItemChecked(par1, last) \
	LP2(0x3c, BOOL, PM_ItemChecked, struct PopupMenu *, par1, a1, ULONG, last, d1, \
	, PM_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PM_GetItemAttrsA(par1, last) \
	LP2(0x42, LONG, PM_GetItemAttrsA, struct PopupMenu *, par1, a2, struct TagItem *, last, a1, \
	, PM_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define PM_GetItemAttrs(par1, tags...) \
	({ULONG _tags[] = {tags}; PM_GetItemAttrsA((par1), (struct TagItem *) _tags);})
#endif

#define PM_SetItemAttrsA(par1, last) \
	LP2(0x48, LONG, PM_SetItemAttrsA, struct PopupMenu *, par1, a2, struct TagItem *, last, a1, \
	, PM_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define PM_SetItemAttrs(par1, tags...) \
	({ULONG _tags[] = {tags}; PM_SetItemAttrsA((par1), (struct TagItem *) _tags);})
#endif

#define PM_FindItem(par1, last) \
	LP2(0x4e, struct PopupMenu *, PM_FindItem, struct PopupMenu *, par1, a1, ULONG, last, d1, \
	, PM_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PM_AlterState(par1, par2, last) \
	LP3NR(0x54, PM_AlterState, struct PopupMenu *, par1, a1, struct PM_IDLst *, par2, a2, UWORD, last, d1, \
	, PM_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PM_FilterIMsgA(par1, par2, par3, last) \
	LP4A5(0x5a, APTR, PM_FilterIMsgA, struct Window *, par1, a1, struct PopupMenu *, par2, a2, struct IntuiMessage *, par3, a3, struct TagItem *, last, d7, \
	, PM_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define PM_FilterIMsg(par1, par2, par3, tags...) \
	({ULONG _tags[] = {tags}; PM_FilterIMsgA((par1), (par2), (par3), (struct TagItem *) _tags);})
#endif

#define PM_ExLstA(last) \
	LP1(0x60, struct PM_IDLst *, PM_ExLstA, ULONG *, last, a1, \
	, PM_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define PM_ExLst(tags...) \
	({ULONG _tags[] = {tags}; PM_ExLstA((ULONG *) _tags);})
#endif

#define PM_InsertMenuItemA(par1, last) \
	LP2(0x6c, LONG, PM_InsertMenuItemA, struct PopupMenu *, par1, a0, struct TagItem *, last, a1, \
	, PM_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define PM_InsertMenuItem(par1, tags...) \
	({ULONG _tags[] = {tags}; PM_InsertMenuItemA((par1), (struct TagItem *) _tags);})
#endif

#define PM_RemoveMenuItem(par1, last) \
	LP2(0x72, struct PopupMenu *, PM_RemoveMenuItem, struct PopupMenu *, par1, a0, struct PopupMenu *, last, a1, \
	, PM_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PM_AbortHook(last) \
	LP1(0x78, BOOL, PM_AbortHook, APTR, last, a0, \
	, PM_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PM_GetVersion() \
	LP0(0x7e, STRPTR, PM_GetVersion, \
	, PM_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PM_ReloadPrefs() \
	LP0NR(0x84, PM_ReloadPrefs, \
	, PM_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PM_LayoutMenuA(par1, par2, last) \
	LP3(0x8a, LONG, PM_LayoutMenuA, struct Window *, par1, a0, struct PopupMenu *, par2, a1, struct TagItem *, last, a2, \
	, PM_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define PM_LayoutMenu(par1, par2, tags...) \
	({ULONG _tags[] = {tags}; PM_LayoutMenuA((par1), (par2), (struct TagItem *) _tags);})
#endif

#define PM_FreeIDList(last) \
	LP1NR(0x96, PM_FreeIDList, struct PM_IDLst *, last, a0, \
	, PM_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#endif /*  _PPCINLINE_PM_H  */
