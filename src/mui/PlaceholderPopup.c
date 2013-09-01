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

 Superclass:  MUIC_Popobject
 Description: Popobject to select a placeholder

***************************************************************************/

#include "PlaceholderPopup_cl.h"

#include <stdlib.h>
#include <string.h>

#include <proto/muimaster.h>
#include <mui/BetterString_mcc.h>
#include <mui/NList_mcc.h>
#include <mui/NListview_mcc.h>

#include "SDI_hook.h"

#include "extrasrc.h"

#include "YAM_utilities.h"

#include "mui/PlaceholderPopupList.h"

#include "Locale.h"
#include "MUIObjects.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *ST_PHOLDER;
  Object *LV_PHOLDER;
  Object *BT_POPUP;
};
*/

/* Hooks */
/// HandleVarHook
//  Pastes an entry from variable listview into string gadget
HOOKPROTONH(HandleVarFunc, void, Object *listview, Object *string)
{
  Object *list;

  ENTER();

  if((list = (Object *)xget(listview, MUIA_NListview_NList)) != NULL)
  {
    char *var = NULL;

    DoMethod(list, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &var);
    if(var != NULL)
    {
      char addstr[3];
      char *str = (char *)xget(string, MUIA_String_Contents);
      LONG pos = xget(string, MUIA_String_BufferPos);

      strlcpy(addstr, var, sizeof(addstr));

      if(IsStrEmpty(str) == FALSE)
      {
        int len = strlen(str)+sizeof(addstr);
        char *buf;

        if((buf = calloc(1, len)) != NULL)
        {
          // append the addstr to the right position

          if(pos > 0)
            strlcpy(buf, str, MIN(len, pos + 1));

          strlcat(buf, addstr, len);

          if(pos >= 0)
            strlcat(buf, str + pos, len);

          set(string, MUIA_String_Contents, buf);

          free(buf);
        }
      }
      else
        set(string, MUIA_String_Contents, addstr);
    }
  }

  LEAVE();
}
MakeStaticHook(HandleVarHook, HandleVarFunc);

///

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  Object *ST_PHOLDER;
  Object *LV_PHOLDER;
  Object *BT_POPUP;
  ULONG controlChar;
  ULONG maxLen;
  ULONG mode;

  ENTER();

  controlChar = GetTagData(ATTR(ControlChar), 0, inittags(msg));
  maxLen = GetTagData(MUIA_String_MaxLen, 0, inittags(msg));
  mode = GetTagData(ATTR(Mode), 0, inittags(msg));

  if((obj = DoSuperNew(cl, obj,
    MUIA_Popstring_String, ST_PHOLDER = BetterStringObject,
      StringFrame,
      MUIA_String_MaxLen,      maxLen,
      MUIA_String_AdvanceOnCR, TRUE,
      MUIA_ControlChar,        controlChar,
      MUIA_CycleChain,         TRUE,
    End,
    MUIA_Popstring_Button, BT_POPUP = PopButton(MUII_PopUp),
    MUIA_Popobject_ObjStrHook, &HandleVarHook,
    MUIA_Popobject_WindowHook, &PO_WindowHook,
    MUIA_Popobject_Object, NListviewObject,
      MUIA_FixHeightTxt, "\n\n\n\n\n\n\n\n",
      MUIA_NListview_Horiz_ScrollBar, MUIV_NListview_HSB_None,
      MUIA_NListview_Vert_ScrollBar, MUIV_NListview_VSB_FullAuto,
      MUIA_NListview_NList, LV_PHOLDER = PlaceholderPopupListObject,
        MUIA_PlaceholderPopupList_Mode, mode,
      End,
    End,
    TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;

    data->ST_PHOLDER = ST_PHOLDER;
    data->LV_PHOLDER = LV_PHOLDER;
    data->BT_POPUP   = BT_POPUP;

    DoMethod(LV_PHOLDER, MUIM_Notify, MUIA_NList_DoubleClick, TRUE,           obj, 2, MUIM_Popstring_Close, TRUE);
    DoMethod(ST_PHOLDER, MUIM_Notify, MUIA_Disabled,          MUIV_EveryTime, obj, 3, MUIM_Set, MUIA_Disabled, MUIV_TriggerValue);
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
      case MUIA_String_Contents:
      {
        nnset(data->ST_PHOLDER, MUIA_String_Contents, tag->ti_Data);
        // make the superMethod call ignore those tags
        tag->ti_Tag = TAG_IGNORE;
      }
      break;

      case MUIA_Disabled:
      {
        // disabling the Popstring object completely doesn't work, because on reactivation the string
        // gadget is not redrawn correctly (bug in MUI?), hence we do it separately.
        nnset(data->ST_PHOLDER, MUIA_Disabled, tag->ti_Data);
        nnset(data->BT_POPUP, MUIA_Disabled, tag->ti_Data);
        // make the superMethod call ignore those tags
        tag->ti_Tag = TAG_IGNORE;
      }
      break;

      case ATTR(PopbuttonDisabled):
      {
        nnset(data->BT_POPUP, MUIA_Disabled, tag->ti_Data);
        // make the superMethod call ignore those tags
        tag->ti_Tag = TAG_IGNORE;
      }
      break;

      case ATTR(ScriptEntry):
      {
        nnset(data->LV_PHOLDER, MUIA_PlaceholderPopupList_ScriptEntry, tag->ti_Data);
        // make the superMethod call ignore those tags
        tag->ti_Tag = TAG_IGNORE;
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
    case MUIA_String_Contents:
    {
      *store = xget(data->ST_PHOLDER, MUIA_String_Contents);

      return TRUE;
    }
    break;
  }

  return DoSuperMethodA(cl, obj, msg);
}

///
