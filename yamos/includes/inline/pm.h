#ifndef _INLINE_POPUPMENU_H
#define _INLINE_POPUPMENU_H

#ifndef __INLINE_MACROS_H
#include <inline/macros.h>
#endif

#ifndef POPUPMENU_BASE_NAME
#define POPUPMENU_BASE_NAME PopupMenuBase
#endif

#define PM_MakeMenuA(tags) \
	LP1(0x1E, struct PopupMenu *, PM_MakeMenuA, struct TagItem *, tags, a1, \
	, POPUPMENU_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define PM_MakeMenu(tags...) \
	({ULONG _tags[] = {tags}; PM_MakeMenuA((struct TagItem *) _tags);})
#endif

#define PM_MakeItemA(tags) \
	LP1(0x24, struct PopupMenu *, PM_MakeItemA, struct TagItem *, tags, a1, \
	, POPUPMENU_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define PM_MakeItem(tags...) \
	({ULONG _tags[] = {tags}; PM_MakeItemA((struct TagItem *) _tags);})
#endif

#define PM_FreePopupMenu(menu) \
	LP1NR(0x2A, PM_FreePopupMenu, struct PopupMenu *, menu, a1, \
	, POPUPMENU_BASE_NAME)

#define PM_OpenPopupMenuA(wnd, tags) \
	LP2(0x30, ULONG, PM_OpenPopupMenuA, struct Window *, wnd, a1, struct TagItem *, tags, a2, \
	, POPUPMENU_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define PM_OpenPopupMenu(wnd, tags...) \
	({ULONG _tags[] = {tags}; PM_OpenPopupMenuA((wnd), (struct TagItem *) _tags);})
#endif

#define PM_MakeIDListA(tags) \
	LP1(0x36, struct PM_IDLst *, PM_MakeIDListA, struct TagItem *, tags, a1, \
	, POPUPMENU_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define PM_MakeIDList(tags...) \
	({ULONG _tags[] = {tags}; PM_MakeIDListA((struct TagItem *) _tags);})
#endif

#define PM_ItemChecked(pm, id) \
	LP2(0x3C, BOOL, PM_ItemChecked, struct PopupMenu *, pm, a1, ULONG, id, d1, \
	, POPUPMENU_BASE_NAME)

#define PM_GetItemAttrsA(item, tags) \
	LP2(0x42, LONG, PM_GetItemAttrsA, struct PopupMenu *, item, a2, struct TagItem *, tags, a1, \
	, POPUPMENU_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define PM_GetItemAttrs(item, tags...) \
	({ULONG _tags[] = {tags}; PM_GetItemAttrsA((item), (struct TagItem *) _tags);})
#endif

#define PM_SetItemAttrsA(item, tags) \
	LP2(0x48, LONG, PM_SetItemAttrsA, struct PopupMenu *, item, a2, struct TagItem *, tags, a1, \
	, POPUPMENU_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define PM_SetItemAttrs(item, tags...) \
	({ULONG _tags[] = {tags}; PM_SetItemAttrsA((item), (struct TagItem *) _tags);})
#endif

#define PM_FindItem(menu, id) \
	LP2(0x4E, struct PopupMenu *, PM_FindItem, struct PopupMenu *, menu, a1, ULONG, id, d1, \
	, POPUPMENU_BASE_NAME)

#define PM_AlterState(menu, idlst, action) \
	LP3NR(0x54, PM_AlterState, struct PopupMenu *, menu, a1, struct PM_IDLst *, idlst, a2, UWORD, action, d1, \
	, POPUPMENU_BASE_NAME)

#define PM_ExLstA(id) \
	LP1(0x60, struct PM_IDLst *, PM_ExLstA, ULONG *, id, a1, \
	, POPUPMENU_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define PM_ExLst(tags...) \
	({ULONG _tags[] = {tags}; PM_ExLstA((ULONG *) _tags);})
#endif

#define PM_FilterIMsgA(window, menu, imsg, tags) \
	LP4(0x66, APTR, PM_FilterIMsgA, struct Window *, window, a0, struct PopupMenu *, menu, a1, struct IntuiMessage *, imsg, a2, struct TagItem *, tags, a3, \
	, POPUPMENU_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define PM_FilterIMsg(window, menu, imsg, tags...) \
	({ULONG _tags[] = {tags}; PM_FilterIMsgA((window), (menu), (imsg), (struct TagItem *) _tags);})
#endif

#define PM_InsertMenuItemA(menu, tags) \
	LP2(0x6C, LONG, PM_InsertMenuItemA, struct PopupMenu *, menu, a0, struct TagItem *, tags, a1, \
	, POPUPMENU_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define PM_InsertMenuItem(menu, tags...) \
	({ULONG _tags[] = {tags}; PM_InsertMenuItemA((menu), (struct TagItem *) _tags);})
#endif

#define PM_RemoveMenuItem(menu, item) \
	LP2(0x72, struct PopupMenu *, PM_RemoveMenuItem, struct PopupMenu *, menu, a0, struct PopupMenu *, item, a1, \
	, POPUPMENU_BASE_NAME)

#define PM_AbortHook(handle) \
	LP1(0x78, BOOL, PM_AbortHook, APTR, handle, a0, \
	, POPUPMENU_BASE_NAME)

#define PM_GetVersion() \
	LP0(0x7E, STRPTR, PM_GetVersion, \
	, POPUPMENU_BASE_NAME)

#define PM_ReloadPrefs() \
	LP0NR(0x84, PM_ReloadPrefs, \
	, POPUPMENU_BASE_NAME)

#define PM_LayoutMenuA(window, menu, tags) \
	LP3(0x8A, LONG, PM_LayoutMenuA, struct Window *, window, a0, struct PopupMenu *, menu, a1, struct TagItem *, tags, a2, \
	, POPUPMENU_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define PM_LayoutMenu(window, menu, tags...) \
	({ULONG _tags[] = {tags}; PM_LayoutMenuA((window), (menu), (struct TagItem *) _tags);})
#endif

#define PM_FreeIDList(list) \
	LP1NR(0x96, PM_FreeIDList, struct PM_IDLst *, list, a0, \
	, POPUPMENU_BASE_NAME)

#endif /*  _INLINE_POPUPMENU_H  */
