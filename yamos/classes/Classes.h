#ifndef CLASSES_CLASSES_H
#define CLASSES_CLASSES_H

/*****************************************************************************
**
** rootclass
** +--Addrmatchlist
** +--Recipientstring
** +--Searchwindow
**
**
** Addrmatchlist   -- Popup a list of addresses which match a given substring
**
**                    Implements:
**                    OM_NEW
**                    OM_SET
**                    ChangeWindow
**                    Event
**                    Open
**
** Recipientstring -- Auto-completes email addresses etc.
**
**                    Implements:
**                    OM_NEW
**                    OM_DISPOSE
**                    OM_GET
**                    OM_SET
**                    MUIM_Setup
**                    MUIM_Show
**                    MUIM_GoActive
**                    MUIM_GoInactive
**                    MUIM_DragQuery
**                    MUIM_DragDrop
**                    MUIM_Popstring_Open
**                    MUIM_HandleEvent
**                    Resolve
**                    AddRecipient
**                    RecipientStart
**                    CurrentRecipient
**
** Searchwindow    -- Window where user inputs search string and options.
**
**                    Implements:
**                    OM_NEW
**                    Open
**                    Close
**                    Search
**                    Next
**
*****************************************************************************/

#include "ClassesExtra.h"

#define NumberOfClasses 3

extern struct MUI_CustomClass *YAMClasses[NumberOfClasses];
Object *YAM_NewObject (STRPTR class, ULONG tag, ...);

int YAM_SetupClasses (VOID);
VOID YAM_CleanupClasses (VOID);

/* --------------------- */
/* --- Addrmatchlist --- */
/* --------------------- */


#define MUIC_Addrmatchlist "YAM_Addrmatchlist"

#define MUIM_Addrmatchlist_ChangeWindow       0xAE008DA7UL
#define MUIM_Addrmatchlist_Event              0xAE0087CCUL
#define MUIM_Addrmatchlist_Open               0xAE00DF3EUL

#define MUIA_Addrmatchlist_String             0xAE00576EUL

struct MUIP_Addrmatchlist_ChangeWindow
{
	ULONG methodID;
};

struct MUIP_Addrmatchlist_Event
{
	ULONG methodID;
	struct IntuiMessage *imsg;
};

struct MUIP_Addrmatchlist_Open
{
	ULONG methodID;
	STRPTR str;
};

#define AddrmatchlistObject YAM_NewObject(MUIC_Addrmatchlist

ULONG AddrmatchlistGetSize (VOID);
/* ----------------------- */
/* --- Recipientstring --- */
/* ----------------------- */


#define MUIC_Recipientstring "YAM_Recipientstring"

#define MUIM_Recipientstring_Resolve          0xAE00EB08UL
#define MUIM_Recipientstring_AddRecipient     0xAE0000CBUL
#define MUIM_Recipientstring_RecipientStart   0xAE004D2EUL
#define MUIM_Recipientstring_CurrentRecipient 0xAE0049FEUL

#define MUIA_Recipientstring_ResolveOnCR      0xAE00FC97UL
#define MUIA_Recipientstring_MultipleRecipients 0xAE005A79UL
#define MUIA_Recipientstring_FromString       0xAE00D557UL
#define MUIA_Recipientstring_ReplyToString    0xAE006744UL
#define MUIA_Recipientstring_Popup            0xAE003126UL

struct MUIP_Recipientstring_Resolve
{
	ULONG methodID;
	ULONG flags;
};

struct MUIP_Recipientstring_AddRecipient
{
	ULONG methodID;
	STRPTR address;
};

struct MUIP_Recipientstring_RecipientStart
{
	ULONG methodID;
};

struct MUIP_Recipientstring_CurrentRecipient
{
	ULONG methodID;
};

#define RecipientstringObject YAM_NewObject(MUIC_Recipientstring

ULONG RecipientstringGetSize (VOID);
// User data:

#define MUIF_Recipientstring_Resolve_NoFullName  (1 << 0) // do not resolve with fullname "Mister X <misterx@mister.com>"
#define MUIF_Recipientstring_Resolve_NoValid     (1 << 1) // do not resolve already valid string like "misterx@mister.com"

/* -------------------- */
/* --- Searchwindow --- */
/* -------------------- */


#define MUIC_Searchwindow "YAM_Searchwindow"

#define MUIM_Searchwindow_Open                0xAE00129AUL
#define MUIM_Searchwindow_Close               0xAE006FF1UL
#define MUIM_Searchwindow_Search              0xAE008B93UL
#define MUIM_Searchwindow_Next                0xAE00F7C1UL


struct MUIP_Searchwindow_Open
{
	ULONG methodID;
	Object *texteditor;
};

struct MUIP_Searchwindow_Close
{
	ULONG methodID;
};

struct MUIP_Searchwindow_Search
{
	ULONG methodID;
	ULONG top;
};

struct MUIP_Searchwindow_Next
{
	ULONG methodID;
};

#define SearchwindowObject YAM_NewObject(MUIC_Searchwindow

ULONG SearchwindowGetSize (VOID);
ULONG m_Addrmatchlist_OM_NEW(struct IClass *cl, Object *obj, Msg msg);
ULONG m_Addrmatchlist_OM_SET(struct IClass *cl, Object *obj, Msg msg);
ULONG m_Addrmatchlist_ChangeWindow(struct IClass *cl, Object *obj, struct MUIP_Addrmatchlist_ChangeWindow *msg);
ULONG m_Addrmatchlist_Event(struct IClass *cl, Object *obj, struct MUIP_Addrmatchlist_Event *msg);
ULONG m_Addrmatchlist_Open(struct IClass *cl, Object *obj, struct MUIP_Addrmatchlist_Open *msg);
ULONG m_Recipientstring_OM_NEW(struct IClass *cl, Object *obj, Msg msg);
ULONG m_Recipientstring_OM_DISPOSE(struct IClass *cl, Object *obj, Msg msg);
ULONG m_Recipientstring_OM_GET(struct IClass *cl, Object *obj, Msg msg);
ULONG m_Recipientstring_OM_SET(struct IClass *cl, Object *obj, Msg msg);
ULONG m_Recipientstring_MUIM_Setup(struct IClass *cl, Object *obj, Msg msg);
ULONG m_Recipientstring_MUIM_Show(struct IClass *cl, Object *obj, Msg msg);
ULONG m_Recipientstring_MUIM_GoActive(struct IClass *cl, Object *obj, Msg msg);
ULONG m_Recipientstring_MUIM_GoInactive(struct IClass *cl, Object *obj, Msg msg);
ULONG m_Recipientstring_MUIM_DragQuery(struct IClass *cl, Object *obj, Msg msg);
ULONG m_Recipientstring_MUIM_DragDrop(struct IClass *cl, Object *obj, Msg msg);
ULONG m_Recipientstring_MUIM_Popstring_Open(struct IClass *cl, Object *obj, Msg msg);
ULONG m_Recipientstring_MUIM_HandleEvent(struct IClass *cl, Object *obj, Msg msg);
ULONG m_Recipientstring_Resolve(struct IClass *cl, Object *obj, struct MUIP_Recipientstring_Resolve *msg);
ULONG m_Recipientstring_AddRecipient(struct IClass *cl, Object *obj, struct MUIP_Recipientstring_AddRecipient *msg);
ULONG m_Recipientstring_RecipientStart(struct IClass *cl, Object *obj, struct MUIP_Recipientstring_RecipientStart *msg);
ULONG m_Recipientstring_CurrentRecipient(struct IClass *cl, Object *obj, struct MUIP_Recipientstring_CurrentRecipient *msg);
ULONG m_Searchwindow_OM_NEW(struct IClass *cl, Object *obj, Msg msg);
ULONG m_Searchwindow_Open(struct IClass *cl, Object *obj, struct MUIP_Searchwindow_Open *msg);
ULONG m_Searchwindow_Close(struct IClass *cl, Object *obj, struct MUIP_Searchwindow_Close *msg);
ULONG m_Searchwindow_Search(struct IClass *cl, Object *obj, struct MUIP_Searchwindow_Search *msg);
ULONG m_Searchwindow_Next(struct IClass *cl, Object *obj, struct MUIP_Searchwindow_Next *msg);

#endif
