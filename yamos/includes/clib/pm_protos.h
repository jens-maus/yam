#ifndef CLIB_POPUPMENU_PROTOS_H
#define CLIB_POPUPMENU_PROTOS_H

//
//	$VER: pm_protos.h 8.0 (07.09.98)
//
//	©1996-1997 Henrik Isaksson
//	All Rights Reserved.
//

#ifndef LIBRARIES_POPUPMENU_H
#include <libraries/pm.h>
#endif

/* Functions in V2 */
struct PopupMenu *PM_MakeItem(ULONG tag1, ...);
struct PopupMenu *PM_MakeMenu(ULONG tag1, ...);
struct PopupMenu *PM_MakeMenuA(struct TagItem *tags);
struct PopupMenu *PM_MakeItemA(struct TagItem *tags);
void PM_FreePopupMenu(struct PopupMenu *p);
ULONG PM_OpenPopupMenuA(struct Window *prevwnd, struct TagItem *tags);
ULONG PM_OpenPopupMenu(struct Window *prevwnd, ULONG tag1, ...);
struct PM_IDLst *PM_MakeIDList(ULONG tag1, ...);
struct PM_IDLst *PM_MakeIDListA(struct TagItem *tags);

/* New for V3 */
BOOL PM_ItemChecked(struct PopupMenu *pm, ULONG ID);
LONG PM_GetItemAttrsA(struct PopupMenu *p, struct TagItem *tags);
LONG PM_GetItemAttrs(struct PopupMenu *p, ULONG tag1, ...);
LONG PM_SetItemAttrsA(struct PopupMenu *p, struct TagItem *tags);
LONG PM_SetItemAttrs(struct PopupMenu *p, ULONG tag1, ...);
struct PopupMenu *PM_FindItem(struct PopupMenu *menu, ULONG ID);

/* New for V5 */
void PM_AlterState(struct PopupMenu *pm, struct PM_IDLst *l, UWORD action);

/* New for V6 */
APTR PM_FilterIMsgA(struct Window *w, struct PopupMenu *pm, struct IntuiMessage *im,  struct TagItem *tags);
APTR PM_FilterIMsg(struct Window *w, struct PopupMenu *pm, struct IntuiMessage *im,  ULONG tag1, ...);
struct PM_IDLst *PM_ExLstA(ULONG *id);
struct PM_IDLst *PM_ExLst(ULONG id, ...);

/* New for V8 */
LONG PM_InsertMenuItemA(struct PopupMenu *menu, struct TagItem *tags);
LONG PM_InsertMenuItem(struct PopupMenu *menu, ULONG tag1, ...);
struct PopupMenu *PM_RemoveMenuItem(struct PopupMenu *menu, struct PopupMenu *item);

/* New for V9 */
BOOL PM_AbortHook(APTR handle);
STRPTR PM_GetVersion(void);
void PM_ReloadPrefs(void);
LONG PM_LayoutMenuA(struct Window *window, struct PopupMenu *pm, struct TagItem *tags);
LONG PM_LayoutMenu(struct Window *window, struct PopupMenu *pm, ULONG tag1, ...);

/* New for V10 */
void PM_FreeIDList(struct PM_IDLst *);

#endif
