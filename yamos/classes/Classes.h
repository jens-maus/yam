#ifndef CLASSES_CLASSES_H
#define CLASSES_CLASSES_H

/*****************************************************************************
**
** rootclass
** +--Searchwindow
**
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

#include "YAM.h"
#include "YAM_hook.h"

#define inittags(msg) (((struct opSet *)msg)->ops_AttrList)
#define GETDATA struct Data *data = (struct Data *)INST_DATA(cl,obj)

#define NumberOfClasses 1

extern struct MUI_CustomClass *YAMClasses[NumberOfClasses];
Object *YAM_NewObject (STRPTR class, ULONG tag, ...);

int YAM_SetupClasses (VOID);
VOID YAM_CleanupClasses (VOID);

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
ULONG m_Searchwindow_OM_NEW(struct IClass *cl, Object *obj, Msg msg);
ULONG m_Searchwindow_Open(struct IClass *cl, Object *obj, struct MUIP_Searchwindow_Open *msg);
ULONG m_Searchwindow_Close(struct IClass *cl, Object *obj, struct MUIP_Searchwindow_Close *msg);
ULONG m_Searchwindow_Search(struct IClass *cl, Object *obj, struct MUIP_Searchwindow_Search *msg);
ULONG m_Searchwindow_Next(struct IClass *cl, Object *obj, struct MUIP_Searchwindow_Next *msg);

#endif
