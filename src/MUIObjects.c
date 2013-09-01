/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2013 YAM Open Source Team

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 YAM Official Support Site :  http://www.yam.ch
 YAM OpenSource project    :  http://sourceforge.net/projects/yamos/

 $Id$

***************************************************************************/

#include <stdlib.h>
#include <string.h>

#include <clib/alib_protos.h>
#include <mui/BetterString_mcc.h>
#include <mui/NList_mcc.h>
#include <mui/NListtree_mcc.h>
#include <mui/NListview_mcc.h>
#include <proto/asl.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/muimaster.h>
#include <proto/utility.h>

#include "extrasrc.h"

#include "SDI_hook.h"

#include "YAM.h"
#include "YAM_addressbook.h"
#include "YAM_error.h"
#include "YAM_global.h"
#include "YAM_read.h"

#include "mui/ClassesExtra.h"
#include "mui/PlaceholderPopupList.h"
#include "mui/RecipientString.h"

#include "Config.h"
#include "FolderList.h"
#include "Locale.h"
#include "MimeTypes.h"
#include "MUIObjects.h"

#include "Debug.h"

/// MakeCycle
//  Creates a MUI cycle object
Object *MakeCycle(const char *const *labels, const char *label)
{
  return CycleObject,
           MUIA_CycleChain,    TRUE,
           MUIA_Font,          MUIV_Font_Button,
           MUIA_Cycle_Entries, labels,
           MUIA_ControlChar,   ShortCut(label),
         End;
}

///
/// MakeButton
//  Creates a MUI button
Object *MakeButton(const char *txt)
{
  Object *obj;

  if((obj = MUI_MakeObject(MUIO_Button,(IPTR)txt)) != NULL)
    set(obj, MUIA_CycleChain, TRUE);

  return obj;
}

///
/// MakeCheck
//  Creates a MUI checkmark object
Object *MakeCheck(const char *label)
{
  return ImageObject,
           ImageButtonFrame,
           MUIA_InputMode   , MUIV_InputMode_Toggle,
           MUIA_Image_Spec  , MUII_CheckMark,
           MUIA_Background  , MUII_ButtonBack,
           MUIA_ShowSelState, FALSE,
           MUIA_ControlChar , ShortCut(label),
           MUIA_CycleChain  , TRUE,
         End;
}

///
/// MakeCheckGroup
//  Creates a labelled MUI checkmark object
Object *MakeCheckGroup(Object **check, const char *label)
{
  return HGroup,
           Child, *check = MakeCheck(label),
           Child, Label1(label),
           Child, HSpace(0),
         End;
}

///
/// MakeString
//  Creates a MUI string object
Object *MakeString(int maxlen, const char *label)
{
  return BetterStringObject,
           StringFrame,
           MUIA_String_MaxLen,      maxlen,
           MUIA_String_AdvanceOnCR, TRUE,
           MUIA_ControlChar,        ShortCut(label),
           MUIA_CycleChain,         TRUE,
         End;
}

///
/// MakePassString
//  Creates a MUI string object with hidden text
Object *MakePassString(const char *label)
{
  return BetterStringObject,
           StringFrame,
           MUIA_String_MaxLen,       SIZE_PASSWORD,
           MUIA_String_Secret,       TRUE,
           MUIA_String_AdvanceOnCR,  TRUE,
           MUIA_ControlChar,         ShortCut(label),
           MUIA_CycleChain,          TRUE,
         End;
}

///
/// MakeInteger
//  Creates a MUI string object for numeric input
Object *MakeInteger(int maxlen, const char *label)
{
  return BetterStringObject,
           StringFrame,
           MUIA_String_MaxLen,       maxlen+1,
           MUIA_String_AdvanceOnCR,  TRUE,
           MUIA_ControlChar,         ShortCut(label),
           MUIA_CycleChain,          TRUE,
           MUIA_String_Integer,      0,
           MUIA_String_Accept,       "0123456789",
         End;
}

///
/// MakeAddressField
//  Creates a recipient field
Object *MakeAddressField(Object **string, const char *label, const void *help, int abmode, int winnr, ULONG flags)
{
  Object *obj;
  Object *bt_adr;

  ENTER();

  if((obj = HGroup,

    GroupSpacing(1),
    Child, *string = RecipientStringObject,
      MUIA_CycleChain,                          TRUE,
      MUIA_String_AdvanceOnCR,                  TRUE,
      MUIA_RecipientString_ResolveOnCR,         TRUE,
      MUIA_RecipientString_MultipleRecipients,  isFlagSet(flags, AFF_ALLOW_MULTI),
      MUIA_RecipientString_NoFullName,          isFlagSet(flags, AFF_NOFULLNAME),
      MUIA_RecipientString_NoCache,             isFlagSet(flags, AFF_NOCACHE),
      MUIA_RecipientString_NoValid,             isFlagSet(flags, AFF_NOVALID),
      MUIA_RecipientString_ResolveOnInactive,   isFlagSet(flags, AFF_RESOLVEINACTIVE),
      MUIA_BetterString_NoShortcuts,            isFlagSet(flags, AFF_EXTERNAL_SHORTCUTS),
      MUIA_ControlChar,                         ShortCut(label),
    End,
    Child, bt_adr = PopButton(MUII_PopUp),

  End))
  {
    if(help != NULL)
      SetHelp(*string, help);
    SetHelp(bt_adr, MSG_HELP_WR_BT_ADR);

    if(abmode == ABM_CONFIG)
    {
      DoMethod(bt_adr, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 4, MUIM_CallHook, &AB_OpenHook, abmode, *string);
      DoMethod(*string, MUIM_Notify, MUIA_RecipientString_Popup, TRUE, MUIV_Notify_Application, 4, MUIM_CallHook, &AB_OpenHook, abmode, *string);
    }
    else
    {
      DoMethod(bt_adr, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 4, MUIM_CallHook, &AB_OpenHook, abmode, winnr);
      DoMethod(*string, MUIM_Notify, MUIA_RecipientString_Popup, TRUE, MUIV_Notify_Application, 4, MUIM_CallHook, &AB_OpenHook, abmode, winnr);
    }

    DoMethod(*string, MUIM_Notify, MUIA_Disabled, MUIV_EveryTime,  bt_adr, 3, MUIM_Set, MUIA_Disabled, MUIV_TriggerValue);
  }

  RETURN(obj);
  return obj;
}

///
/// MakeNumeric
//  Creates a MUI numeric slider
Object *MakeNumeric(int min, int max, BOOL percent)
{
  return NumericbuttonObject,
           MUIA_Numeric_Min, min,
           MUIA_Numeric_Max, max,
           MUIA_Numeric_Format, percent ? "%ld%%" : "%ld",
           MUIA_CycleChain, TRUE,
         End;
}

///
/// ShortCut
//  Finds keyboard shortcut in text label
char ShortCut(const char *label)
{
  char scut = '\0';
  char *ptr;

  ENTER();

  if(label != NULL && (ptr = strchr(label, '_')) != NULL)
    scut = (char)ToLower((ULONG)(*++ptr));

  RETURN(scut);
  return scut;
}

///
/// FilereqStartFunc
//  Will be executed as soon as the user wants to popup a file requester
//  for selecting files
HOOKPROTONO(FilereqStartFunc, BOOL, struct TagItem *tags)
{
  Object *strObj = (Object *)hook->h_Data;
  char *str;

  ENTER();

  str = (char *)xget(strObj, MUIA_String_Contents);
  if(IsStrEmpty(str) == FALSE)
  {
    int i=0;
    static char buf[SIZE_PATHFILE];
    char *p;

    // make sure the string is unquoted.
    strlcpy(buf, str, sizeof(buf));
    UnquoteString(buf, FALSE);

    if((p = PathPart(buf)))
    {
      static char drawer[SIZE_PATHFILE];

      strlcpy(drawer, buf, MIN(sizeof(drawer), (unsigned int)(p - buf + 1)));

      tags[i].ti_Tag = ASLFR_InitialDrawer;
      tags[i].ti_Data= (ULONG)drawer;
      i++;
    }

    tags[i].ti_Tag = ASLFR_InitialFile;
    tags[i].ti_Data = (ULONG)FilePart(buf);
    i++;

    tags[i].ti_Tag = TAG_DONE;
  }

  RETURN(TRUE);
  return TRUE;
}
MakeHook(FilereqStartHook, FilereqStartFunc);

///
/// FilereqStopFunc
//  Will be executed as soon as the user selected a file
HOOKPROTONO(FilereqStopFunc, void, struct FileRequester *fileReq)
{
  Object *strObj = (Object *)hook->h_Data;

  ENTER();

  // check if a file was selected or not
  if(IsStrEmpty(fileReq->fr_File) == FALSE)
  {
    char buf[SIZE_PATHFILE];

    AddPath(buf, fileReq->fr_Drawer, fileReq->fr_File, sizeof(buf));

    // check if there is any space in the path
    if(strchr(buf, ' ') != NULL)
    {
      int len = strlen(buf);

      memmove(&buf[1], buf, len+1);
      buf[0] = '"';
      buf[len+1] = '"';
      buf[len+2] = '\0';
    }

    set(strObj, MUIA_String_Contents, buf);
  }

  LEAVE();
}
MakeHook(FilereqStopHook, FilereqStopFunc);

///
/// PO_Window
// Window hook for popup objects
HOOKPROTONH(PO_Window, void, Object *pop, Object *win)
{
  ENTER();

  set(win, MUIA_Window_DefaultObject, pop);

  LEAVE();
}
MakeHook(PO_WindowHook, PO_Window);

///
/// GetMUIString
// copy the contents of a MUI string object to a string
void GetMUIString(char *s, Object *o, size_t len)
{
  char *c;

  ENTER();

  if((c = (char *)xget(o, MUIA_String_Contents)) != NULL)
    strlcpy(s, c, len);
  else
  {
    E(DBF_GUI, "NULL string contents of object %08lx", o);
    s[0] = '\0';
  }

  LEAVE();
}

///
/// GetMUIText
// copy the contents of a MUI text object to a string
void GetMUIText(char *s, Object *o, size_t len)
{
  char *c;

  ENTER();

  if((c = (char *)xget(o, MUIA_Text_Contents)) != NULL)
    strlcpy(s, c, len);
  else
  {
    E(DBF_GUI, "NULL text contents of object %08lx", o);
    s[0] = '\0';
  }

  LEAVE();
}

///
/// isChildOfList
// check if the supplied object is a member of the list
static BOOL isChildOfList(struct List *list, Object *child)
{
  BOOL isChild = FALSE;

  ENTER();

  if(list != NULL)
  {
    Object *curchild;
    Object *cstate;

    // here we check whether the child is part of the supplied group
    cstate = (Object *)list->lh_Head;
    while((curchild = NextObject(&cstate)) != NULL)
    {
      if(curchild == child)
      {
        isChild = TRUE;
        break;
      }
    }
  }

  RETURN(isChild);
  return isChild;
}

///
/// isChildOfGroup
// return TRUE if the supplied child object is part of the supplied group
BOOL isChildOfGroup(Object *group, Object *child)
{
  BOOL isChild;

  ENTER();

  // get the child list of the group object
  isChild = isChildOfList((struct List *)xget(group, MUIA_Group_ChildList), child);

  RETURN(isChild);
  return isChild;
}

///
/// isChildOfFamily
// return TRUE if the supplied child object is part of the supplied family/menu object
BOOL isChildOfFamily(Object *family, Object *child)
{
  BOOL isChild = FALSE;

  ENTER();

  // get the child list of the family object
  isChild = isChildOfList((struct List *)xget(family, MUIA_Family_List), child);

  RETURN(isChild);
  return isChild;
}

///
/// CreateScreenTitle
// constructs a screen title that always starts with "YAM - XXXXXX" where XXXXXX
// corresponds to the text supplied
const char *CreateScreenTitle(char *dst, size_t dstlen, const char *text)
{
  char *result = NULL;
  ENTER();

  if(dst != NULL && dstlen > 0)
  {
    if(text != NULL)
      snprintf(dst, dstlen, "YAM %s - %s", yamver, text);
    else
      snprintf(dst, dstlen, "YAM %s", yamver);

    result = dst;
  }

  RETURN(result);
  return result;
}

///
