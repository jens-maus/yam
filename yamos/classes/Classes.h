#ifndef CLASSES_CLASSES_H
#define CLASSES_CLASSES_H

/*****************************************************************************
**
** rootclass
** +--Recipientstring
** +--Searchwindow
**
**
** Recipientstring -- Auto-completes email addresses etc.
**
**                    Implements:
**                    OM_NEW
**                    OM_DISPOSE
**                    MUIM_Setup
**                    MUIM_GoActive
**                    MUIM_GoInactive
**                    MUIM_HandleEvent
**                    ShowMatches
**                    MUIM_DragQuery
**                    MUIM_DragDrop
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

#include <clib/alib_protos.h>
#include <libraries/mui.h>
#include <proto/intuition.h>
#include <proto/muimaster.h>
#include <proto/utility.h>
#include <mui/BetterString_mcc.h>

#include "YAM.h"
#include "YAM_hook.h"

#ifndef MUIM_GoActive
#define MUIM_GoActive                0x8042491a
#endif
#ifndef MUIM_GoInactive
#define MUIM_GoInactive              0x80422c0c
#endif

#define inittags(msg) (((struct opSet *)msg)->ops_AttrList)
#define GETDATA struct Data *data = (struct Data *)INST_DATA(cl,obj)

#define NumberOfClasses 2

extern struct MUI_CustomClass *YAMClasses[NumberOfClasses];
Object *YAM_NewObject (STRPTR class, ULONG tag, ...);

int YAM_SetupClasses (VOID);
VOID YAM_CleanupClasses (VOID);

/* ----------------------- */
/* --- Recipientstring --- */
/* ----------------------- */


#define MUIC_Recipientstring "YAM_Recipientstring"

#define MUIM_Recipientstring_ShowMatches      0xAE006D64UL


struct MUIP_Recipientstring_ShowMatches
{
	ULONG methodID;
	STRPTR address;
};

#define RecipientstringObject YAM_NewObject(MUIC_Recipientstring

ULONG RecipientstringGetSize (VOID);
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
ULONG m_Recipientstring_OM_NEW(struct IClass *cl, Object *obj, Msg msg);
ULONG m_Recipientstring_OM_DISPOSE(struct IClass *cl, Object *obj, Msg msg);
ULONG m_Recipientstring_MUIM_Setup(struct IClass *cl, Object *obj, Msg msg);
ULONG m_Recipientstring_MUIM_GoActive(struct IClass *cl, Object *obj, Msg msg);
ULONG m_Recipientstring_MUIM_GoInactive(struct IClass *cl, Object *obj, Msg msg);
ULONG m_Recipientstring_MUIM_HandleEvent(struct IClass *cl, Object *obj, Msg msg);
ULONG m_Recipientstring_ShowMatches(struct IClass *cl, Object *obj, struct MUIP_Recipientstring_ShowMatches *msg);
ULONG m_Recipientstring_MUIM_DragQuery(struct IClass *cl, Object *obj, Msg msg);
ULONG m_Recipientstring_MUIM_DragDrop(struct IClass *cl, Object *obj, Msg msg);
ULONG m_Searchwindow_OM_NEW(struct IClass *cl, Object *obj, Msg msg);
ULONG m_Searchwindow_Open(struct IClass *cl, Object *obj, struct MUIP_Searchwindow_Open *msg);
ULONG m_Searchwindow_Close(struct IClass *cl, Object *obj, struct MUIP_Searchwindow_Close *msg);
ULONG m_Searchwindow_Search(struct IClass *cl, Object *obj, struct MUIP_Searchwindow_Search *msg);
ULONG m_Searchwindow_Next(struct IClass *cl, Object *obj, struct MUIP_Searchwindow_Next *msg);

#endif
