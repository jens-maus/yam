#ifndef _VBCCINLINE_PM_H
#define _VBCCINLINE_PM_H

struct PopupMenu * __PM_MakeMenuA(__reg("a1") struct TagItem * tags, __reg("a6") void *)="\tjsr\t-30(a6)";
#define PM_MakeMenuA(tags) __PM_MakeMenuA((tags), PopupMenuBase)

struct PopupMenu * __PM_MakeItemA(__reg("a1") struct TagItem * tags, __reg("a6") void *)="\tjsr\t-36(a6)";
#define PM_MakeItemA(tags) __PM_MakeItemA((tags), PopupMenuBase)

void __PM_FreePopupMenu(__reg("a1") struct PopupMenu * menu, __reg("a6") void *)="\tjsr\t-42(a6)";
#define PM_FreePopupMenu(menu) __PM_FreePopupMenu((menu), PopupMenuBase)

ULONG __PM_OpenPopupMenuA(__reg("a1") struct Window * wnd, __reg("a2") struct TagItem * tags, __reg("a6") void *)="\tjsr\t-48(a6)";
#define PM_OpenPopupMenuA(wnd, tags) __PM_OpenPopupMenuA((wnd), (tags), PopupMenuBase)

struct PM_IDLst * __PM_MakeIDListA(__reg("a1") struct TagItem * tags, __reg("a6") void *)="\tjsr\t-54(a6)";
#define PM_MakeIDListA(tags) __PM_MakeIDListA((tags), PopupMenuBase)

BOOL __PM_ItemChecked(__reg("a1") struct PopupMenu * pm, __reg("d1") ULONG id, __reg("a6") void *)="\tjsr\t-60(a6)";
#define PM_ItemChecked(pm, id) __PM_ItemChecked((pm), (id), PopupMenuBase)

LONG __PM_GetItemAttrsA(__reg("a2") struct PopupMenu * item, __reg("a1") struct TagItem * tags, __reg("a6") void *)="\tjsr\t-66(a6)";
#define PM_GetItemAttrsA(item, tags) __PM_GetItemAttrsA((item), (tags), PopupMenuBase)

LONG __PM_SetItemAttrsA(__reg("a2") struct PopupMenu * item, __reg("a1") struct TagItem * tags, __reg("a6") void *)="\tjsr\t-72(a6)";
#define PM_SetItemAttrsA(item, tags) __PM_SetItemAttrsA((item), (tags), PopupMenuBase)

struct PopupMenu * __PM_FindItem(__reg("a1") struct PopupMenu * menu, __reg("d1") ULONG id, __reg("a6") void *)="\tjsr\t-78(a6)";
#define PM_FindItem(menu, id) __PM_FindItem((menu), (id), PopupMenuBase)

void __PM_AlterState(__reg("a1") struct PopupMenu * menu, __reg("a2") struct PM_IDLst * idlst, __reg("d1") UWORD action, __reg("a6") void *)="\tjsr\t-84(a6)";
#define PM_AlterState(menu, idlst, action) __PM_AlterState((menu), (idlst), (action), PopupMenuBase)

struct PM_IDLst * __PM_ExLstA(__reg("a1") ULONG * id, __reg("a6") void *)="\tjsr\t-96(a6)";
#define PM_ExLstA(id) __PM_ExLstA((id), PopupMenuBase)

APTR __PM_FilterIMsgA(__reg("a0") struct Window * window, __reg("a1") struct PopupMenu * menu, __reg("a2") struct IntuiMessage * imsg, __reg("a3") struct TagItem * tags, __reg("a6") void *)="\tjsr\t-102(a6)";
#define PM_FilterIMsgA(window, menu, imsg, tags) __PM_FilterIMsgA((window), (menu), (imsg), (tags), PopupMenuBase)

LONG __PM_InsertMenuItemA(__reg("a0") struct PopupMenu * menu, __reg("a1") struct TagItem * tags, __reg("a6") void *)="\tjsr\t-108(a6)";
#define PM_InsertMenuItemA(menu, tags) __PM_InsertMenuItemA((menu), (tags), PopupMenuBase)

struct PopupMenu * __PM_RemoveMenuItem(__reg("a0") struct PopupMenu * menu, __reg("a1") struct PopupMenu * item, __reg("a6") void *)="\tjsr\t-114(a6)";
#define PM_RemoveMenuItem(menu, item) __PM_RemoveMenuItem((menu), (item), PopupMenuBase)

BOOL __PM_AbortHook(__reg("a0") APTR handle, __reg("a6") void *)="\tjsr\t-120(a6)";
#define PM_AbortHook(handle) __PM_AbortHook((handle), PopupMenuBase)

STRPTR __PM_GetVersion(__reg("a6") void *)="\tjsr\t-126(a6)";
#define PM_GetVersion() __PM_GetVersion(PopupMenuBase)

void __PM_ReloadPrefs(__reg("a6") void *)="\tjsr\t-132(a6)";
#define PM_ReloadPrefs() __PM_ReloadPrefs(PopupMenuBase)

LONG __PM_LayoutMenuA(__reg("a0") struct Window * window, __reg("a1") struct PopupMenu * menu, __reg("a2") struct TagItem * tags, __reg("a6") void *)="\tjsr\t-138(a6)";
#define PM_LayoutMenuA(window, menu, tags) __PM_LayoutMenuA((window), (menu), (tags), PopupMenuBase)

void __PM_FreeIDList(__reg("a0") struct PM_IDLst * list, __reg("a6") void *)="\tjsr\t-150(a6)";
#define PM_FreeIDList(list) __PM_FreeIDList((list), PopupMenuBase)

#endif /*  _VBCCINLINE_PM_H  */
