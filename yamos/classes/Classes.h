#ifndef CLASSES_CLASSES_H
#define CLASSES_CLASSES_H

/*****************************************************************************
**
** rootclass
** +--Searchwindow
** +--Recipientstring
**
**
=======
** Searchwindow    -- Window where user inputs search string and options.
**
**                    Implements:
**                    OM_NEW
**                    Open
**                    Close
**                    Search
**                    Next
**
** Recipientstring -- Auto-completes email addresses etc.
**
**                    Implements:
**                    OM_NEW
**                    OM_DISPOSE
**                    OM_SET
**                    MUIM_Setup
**                    MUIM_GoActive
**                    MUIM_GoInactive
**                    MUIM_DragQuery
**                    MUIM_DragDrop
**                    OM_GET
**                    MUIM_Popstring_Open
**                    AddRecipient
**                    Resolve
**                    RecipientStart
**                    MUIM_HandleEvent
**                    ShowMatches
**
*****************************************************************************/

#include <clib/alib_protos.h>
#include <proto/intuition.h>
#include <proto/muimaster.h>
#include <proto/utility.h>
#include <libraries/iffparse.h>
#include <libraries/mui.h>
#include <mui/BetterString_mcc.h>
#include <mui/NList_mcc.h>
#include <mui/TextEditor_mcc.h>
#include <stdlib.h>
#include <string.h>

#include "YAM.h"
#include "YAM_addressbook.h"
#include "YAM_config.h"
#include "YAM_debug.h"
#include "YAM_folderconfig.h"
#include "YAM_hook.h"
#include "YAM_locale.h"
#include "YAM_locale.h"
#include "YAM_main.h"
#include "YAM_mainFolder.h"
#include "YAM_utilities.h"
#include "YAM_utilities.h"
#include "YAM_write.h"

#ifndef MUIM_GoActive
#define MUIM_GoActive                0x8042491a
#endif
#ifndef MUIM_GoInactive
#define MUIM_GoInactive              0x80422c0c
#endif
#ifndef MUIA_String_Popup
#define MUIA_String_Popup            0x80420d71
#endif

#define inittags(msg) (((struct opSet *)msg)->ops_AttrList)
#define GETDATA struct Data *data = (struct Data *)INST_DATA(cl,obj)

#define NumberOfClasses 2

extern struct MUI_CustomClass *YAMClasses[NumberOfClasses];
Object *YAM_NewObject (STRPTR class, ULONG tag, ...);

int YAM_SetupClasses (VOID);
VOID YAM_CleanupClasses (VOID);

/* -------------------- */
/* --- Searchwindow --- */
/* -------------------- */


#define MUIC_Searchwindow "YAM_Searchwindow"

#define MUIM_Searchwindow_Open                  0xAE00129AUL
#define MUIM_Searchwindow_Close                 0xAE006FF1UL
#define MUIM_Searchwindow_Search                0xAE008B93UL
#define MUIM_Searchwindow_Next                  0xAE00F7C1UL

struct MUIP_Searchwindow_Open
{
	ULONG methodID;
	Object *texteditor;
};

struct MUIP_Searchwindow_Close
{
	ULONG methodID;
   ULONG flags;
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
/* ----------------------- */
/* --- Recipientstring --- */
/* ----------------------- */


#define MUIC_Recipientstring "YAM_Recipientstring"

#define MUIM_Recipientstring_AddRecipient          0xAE0000CBUL
#define MUIM_Recipientstring_Resolve               0xAE00EB08UL
#define MUIM_Recipientstring_RecipientStart        0xAE004D2EUL
#define MUIM_Recipientstring_ShowMatches           0xAE006D64UL

#define MUIA_Recipientstring_MultipleRecipients    0xAE005A79UL
#define MUIA_Recipientstring_FromString            0xAE00D557UL
#define MUIA_Recipientstring_ReplyToString         0xAE006744UL
#define MUIA_Recipientstring_Popup                 0xAE003126UL

#define MUIV_Recipientstring_Resolve_NoRealName    1
#define MUIV_Recipientstring_Resolve_NoValids      2

struct MUIP_Recipientstring_AddRecipient
{
	ULONG methodID;
	STRPTR address;
};

struct MUIP_Recipientstring_Resolve
{
	ULONG methodID;
   ULONG flags;
};

struct MUIP_Recipientstring_RecipientStart
{
	ULONG methodID;
};

struct MUIP_Recipientstring_ShowMatches
{
	ULONG methodID;
};

#define RecipientstringObject YAM_NewObject(MUIC_Recipientstring

ULONG RecipientstringGetSize (VOID);
ULONG m_Searchwindow_OM_NEW(struct IClass *cl, Object *obj, Msg msg);
ULONG m_Searchwindow_Open(struct IClass *cl, Object *obj, struct MUIP_Searchwindow_Open *msg);
ULONG m_Searchwindow_Close(struct IClass *cl, Object *obj, struct MUIP_Searchwindow_Close *msg);
ULONG m_Searchwindow_Search(struct IClass *cl, Object *obj, struct MUIP_Searchwindow_Search *msg);
ULONG m_Searchwindow_Next(struct IClass *cl, Object *obj, struct MUIP_Searchwindow_Next *msg);
ULONG m_Recipientstring_OM_NEW(struct IClass *cl, Object *obj, Msg msg);
ULONG m_Recipientstring_OM_DISPOSE(struct IClass *cl, Object *obj, Msg msg);
ULONG m_Recipientstring_OM_SET(struct IClass *cl, Object *obj, Msg msg);
ULONG m_Recipientstring_MUIM_Setup(struct IClass *cl, Object *obj, Msg msg);
ULONG m_Recipientstring_MUIM_GoActive(struct IClass *cl, Object *obj, Msg msg);
ULONG m_Recipientstring_MUIM_GoInactive(struct IClass *cl, Object *obj, Msg msg);
ULONG m_Recipientstring_MUIM_DragQuery(struct IClass *cl, Object *obj, Msg msg);
ULONG m_Recipientstring_MUIM_DragDrop(struct IClass *cl, Object *obj, Msg msg);
ULONG m_Recipientstring_OM_GET(struct IClass *cl, Object *obj, Msg msg);
ULONG m_Recipientstring_MUIM_Popstring_Open(struct IClass *cl, Object *obj, Msg msg);
ULONG m_Recipientstring_AddRecipient(struct IClass *cl, Object *obj, struct MUIP_Recipientstring_AddRecipient *msg);
ULONG m_Recipientstring_Resolve(struct IClass *cl, Object *obj, struct MUIP_Recipientstring_Resolve *msg);
ULONG m_Recipientstring_RecipientStart(struct IClass *cl, Object *obj, struct MUIP_Recipientstring_RecipientStart *msg);
ULONG m_Recipientstring_MUIM_HandleEvent(struct IClass *cl, Object *obj, Msg msg);
ULONG m_Recipientstring_ShowMatches(struct IClass *cl, Object *obj, struct MUIP_Recipientstring_ShowMatches *msg);

#endif
