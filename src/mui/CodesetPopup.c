/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2017 YAM Open Source Team

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

 Superclass:  MUIC_Popobject
 Description: Popobject to select a character set

***************************************************************************/

#include "CodesetPopup_cl.h"

#include <string.h>

#include <proto/codesets.h>
#include <proto/muimaster.h>
#include <mui/NList_mcc.h>
#include <mui/NListtree_mcc.h>
#include <mui/NListview_mcc.h>

#include "SDI_hook.h"

#include "extrasrc.h"

#include "Locale.h"
#include "MUIObjects.h"

#include "mui/CodesetPopupList.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *TX_CODESET;
};
*/

/* Hooks */
/// CodesetOpenHook
//  Sets the popup listview accordingly to the string gadget
HOOKPROTONH(CodesetOpenFunc, BOOL, Object *listview, Object *str)
{
  char *s;
  Object *list;

  ENTER();

  if((s = (char *)xget(str, MUIA_Text_Contents)) != NULL &&
     (list = (Object *)xget(listview, MUIA_NListview_NList)) != NULL)
  {
    int i;

    for(i=0;;i++)
    {
      char *x;

      DoMethod(list, MUIM_NList_GetEntry, i, &x);
      if(x == NULL)
      {
        set(list, MUIA_NList_Active, MUIV_NList_Active_Off);
        break;
      }
      else if(strcasecmp(x, s) == 0)
      {
        set(list, MUIA_NList_Active, i);
        break;
      }
    }
  }

  RETURN(TRUE);
  return TRUE;
}
MakeStaticHook(CodesetOpenHook, CodesetOpenFunc);

///
/// CodesetCloseHook
//  Pastes an entry from the popup listview into string gadget
HOOKPROTONH(CodesetCloseFunc, void, Object *listview, Object *txt)
{
  Object *list;

  ENTER();

  if((list = (Object *)xget(listview, MUIA_NListview_NList)) != NULL)
  {
    char *var = NULL;

    DoMethod(list, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &var);
    if(var != NULL)
      set(txt, MUIA_Text_Contents, var);
  }

  LEAVE();
}
MakeStaticHook(CodesetCloseHook, CodesetCloseFunc);

///

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  Object *TX_CODESET;
  Object *pop;
  Object *list;
  ULONG controlChar;
  BOOL allowMultibyte;

  ENTER();

  controlChar = GetTagData(ATTR(ControlChar), 0, inittags(msg));
  allowMultibyte = GetTagData(ATTR(AllowMultibyteCodesets), TRUE, inittags(msg));

  if((obj = DoSuperNew(cl, obj,
    MUIA_Popstring_String, TX_CODESET = TextObject,
      TextFrame,
      MUIA_Background,  MUII_TextBack,
    End,

    MUIA_Popstring_Button, pop = PopButton(MUII_PopUp),
    MUIA_Popobject_StrObjHook, &CodesetOpenHook,
    MUIA_Popobject_ObjStrHook, &CodesetCloseHook,
    MUIA_Popobject_WindowHook, &PO_WindowHook,
    MUIA_Popobject_Object, NListviewObject,
      MUIA_NListview_Horiz_ScrollBar, MUIV_NListview_HSB_None,
      MUIA_NListview_NList, list = CodesetPopupListObject,
        MUIA_CodesetPopupList_AllowMultibyteCodesets, allowMultibyte,
      End,
    End,
    TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;
    struct codeset *codeset;

    data->TX_CODESET = TX_CODESET;

    xset(pop,
      MUIA_CycleChain, TRUE,
      MUIA_ControlChar, controlChar);

    DoMethod(list, MUIM_Notify, MUIA_NList_DoubleClick, TRUE, obj, 2, MUIM_Popstring_Close, TRUE);
    DoMethod(TX_CODESET, MUIM_Notify, MUIA_Text_Contents, MUIV_EveryTime, obj, 3, MUIM_Set, ATTR(CodesetChanged), TRUE);
    DoMethod(TX_CODESET, MUIM_Notify, MUIA_Text_Contents, MUIV_EveryTime, obj, 3, MUIM_Set, ATTR(Codeset), MUIV_TriggerValue);

    // disable the popup button in case there are no codesets available
    if(xget(list, MUIA_NList_Entries) == 0)
      set(obj, MUIA_Disabled, TRUE);

    // Use the system's default codeset
    if((codeset = CodesetsFindA(NULL, NULL)) != NULL)
      set(TX_CODESET, MUIA_Text_Contents, codeset->name);
  }

  RETURN((IPTR)obj);
  return (IPTR)obj;
}

///
/// OVERLOAD(OM_SET)
OVERLOAD(OM_SET)
{
  GETDATA;
  struct TagItem *tags = inittags(msg), *tag;
  ULONG result;

  ENTER();

  while((tag = NextTagItem((APTR)&tags)) != NULL)
  {
    switch(tag->ti_Tag)
    {
      case ATTR(Codeset):
      {
        nnset(data->TX_CODESET, MUIA_Text_Contents, tag->ti_Data);
      }
      break;
    }
  }

  result = DoSuperMethodA(cl, obj, msg);

  RETURN(result);
  return result;
}

///
/// OVERLOAD(OM_GET)
OVERLOAD(OM_GET)
{
  GETDATA;
  IPTR *store = ((struct opGet *)msg)->opg_Storage;

  switch(((struct opGet *)msg)->opg_AttrID)
  {
    case ATTR(CodesetChanged):
    {
      *store = TRUE;

      return TRUE;
    }
    break;

    case ATTR(Codeset):
    {
      *store = xget(data->TX_CODESET, MUIA_Text_Contents);

      return TRUE;
    }
    break;
  }

  return DoSuperMethodA(cl, obj, msg);
}

///
