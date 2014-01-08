/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2014 YAM Open Source Team

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
 Description: Popobject to select a MIME type

***************************************************************************/

#include "MimeTypePopup_cl.h"

#include <proto/muimaster.h>
#include <mui/BetterString_mcc.h>

#include "SDI_hook.h"

#include "extrasrc.h"

#include "Config.h"
#include "Locale.h"
#include "MimeTypes.h"
#include "MUIObjects.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *ST_MIMETYPE;
  Object *BT_POPUP;
  Object *ST_EXTENSION;
  Object *ST_DESCRIPTION;

  struct Hook MimeTypeListOpenHook;
  struct Hook MimeTypeListCloseHook;
};
*/

/* Hooks */
/// MimeTypeListOpenHook
//  Sets the popup listview accordingly to the string gadget
HOOKPROTO(MimeTypeListOpenFunc, BOOL, Object *listview, Object *str)
{
  BOOL isConfigWindow = (BOOL)(hook->h_Data != NULL);
  char *s;
  Object *list;

  ENTER();

  if((s = (char *)xget(str, MUIA_String_Contents)) != NULL &&
     (list = (Object *)xget(listview, MUIA_Listview_List)) != NULL)
  {
    int i;

    // we build the list totally from ground up.
    DoMethod(list, MUIM_List_Clear);

    // populate the list with the user's own defined MIME types but only if the source
    // string isn't the one in the YAM config window.
    if(isConfigWindow == FALSE)
    {
      struct MimeTypeNode *mt;

      IterateList(&C->mimeTypeList, struct MimeTypeNode *, mt)
      {
        DoMethod(list, MUIM_List_InsertSingle, mt->ContentType, MUIV_List_Insert_Sorted);
      }
    }

    // populate the MUI list with our internal MIME types but check that
    // we don't add duplicate names
    for(i=0; IntMimeTypeArray[i].ContentType != NULL; i++)
    {
      BOOL duplicateFound = FALSE;

      if(isConfigWindow == FALSE)
      {
        struct MimeTypeNode *mt;

        IterateList(&C->mimeTypeList, struct MimeTypeNode *, mt)
        {
          if(strcasecmp(mt->ContentType, IntMimeTypeArray[i].ContentType) == 0)
          {
            duplicateFound = TRUE;
            break;
          }
        }
      }

      if(duplicateFound == FALSE)
        DoMethod(list, MUIM_List_InsertSingle, IntMimeTypeArray[i].ContentType, MUIV_List_Insert_Sorted);
    }

    // make sure to make the current entry active
    for(i=0;;i++)
    {
      char *c;

      DoMethod(list, MUIM_List_GetEntry, i, &c);
      if(c == NULL || s[0] == '\0')
      {
        set(list, MUIA_List_Active, MUIV_List_Active_Off);
        break;
      }
      else if(strcasecmp(c, s) == 0)
      {
        set(list, MUIA_List_Active, i);
        break;
      }
    }
  }

  RETURN(TRUE);
  return TRUE;
}
MakeStaticHook(MimeTypeListOpenHook, MimeTypeListOpenFunc);

///
/// MimeTypeListCloseHook
//  Pastes an entry from the popup listview into string gadget
HOOKPROTO(MimeTypeListCloseFunc, void, Object *listview, Object *str)
{
  struct Data *data = (struct Data *)hook->h_Data;
  Object *list;

  ENTER();

  if((list = (Object *)xget(listview, MUIA_Listview_List)) != NULL)
  {
    char *entry = NULL;

    DoMethod(list, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &entry);
    if(entry != NULL)
    {
      set(str, MUIA_String_Contents, entry);

      // in case that this close function is used with the
      // string gadget in the YAM config window we have to do a deeper search
      // as we also want to set the file extension and description gadgets
      if(data->ST_EXTENSION != NULL || data->ST_DESCRIPTION != NULL)
      {
        int i;

        for(i=0; IntMimeTypeArray[i].ContentType != NULL; i++)
        {
          struct IntMimeType *mt = (struct IntMimeType *)&IntMimeTypeArray[i];

          if(strcasecmp(mt->ContentType, entry) == 0)
          {
            // we also set the file extension
            if(mt->Extension != NULL && data->ST_EXTENSION != NULL)
              set(data->ST_EXTENSION, MUIA_String_Contents, mt->Extension);

            // we also set the mime description
            if(data->ST_DESCRIPTION != NULL)
              set(data->ST_DESCRIPTION, MUIA_String_Contents, tr(mt->Description));

            break;
          }
        }
      }
    }
  }

  LEAVE();
}
MakeStaticHook(MimeTypeListCloseHook, MimeTypeListCloseFunc);

///

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  Object *ST_MIMETYPE;
  Object *LV_MIMETYPE;
  Object *BT_POPUP;
  ULONG controlChar;

  ENTER();

  controlChar = GetTagData(ATTR(ControlChar), 0, inittags(msg));

  if((obj = DoSuperNew(cl, obj,
    MUIA_Popstring_String, ST_MIMETYPE = BetterStringObject,
      StringFrame,
      MUIA_String_Accept,      "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz+-/#?*",
      MUIA_String_MaxLen,      SIZE_CTYPE,
      MUIA_ControlChar,        controlChar,
      MUIA_String_AdvanceOnCR, TRUE,
      MUIA_CycleChain,         TRUE,
    End,
    MUIA_Popstring_Button, BT_POPUP = PopButton(MUII_PopUp),
    MUIA_Popobject_WindowHook, &PO_WindowHook,
    MUIA_Popobject_Object, LV_MIMETYPE = ListviewObject,
      MUIA_Listview_ScrollerPos, MUIV_Listview_ScrollerPos_Right,
      MUIA_Listview_List, ListObject,
        InputListFrame,
        MUIA_List_AutoVisible, TRUE,
        MUIA_List_ConstructHook, MUIV_List_ConstructHook_String,
        MUIA_List_DestructHook,  MUIV_List_DestructHook_String,
      End,
    End,
    TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;
    ULONG inConfigWindow = GetTagData(ATTR(InConfigWindow), FALSE, inittags(msg));

    data->ST_MIMETYPE = ST_MIMETYPE;
    data->BT_POPUP    = BT_POPUP;

    // hook->h_Data=TRUE tells the hook that the string object does belong to the config window
    // I know, this is a quite ugly hack, but unfortunately MUI does not
    // offer methods for this purpose which could do this stuff in a much
    // more sophisticated way :(
    InitHook(&data->MimeTypeListOpenHook, MimeTypeListOpenHook, inConfigWindow);
    InitHook(&data->MimeTypeListCloseHook, MimeTypeListCloseHook, data);
    xset(obj,
      MUIA_Popobject_StrObjHook, &data->MimeTypeListOpenHook,
      MUIA_Popobject_ObjStrHook, &data->MimeTypeListCloseHook);

    set(BT_POPUP, MUIA_CycleChain,TRUE);
    DoMethod(LV_MIMETYPE, MUIM_Notify, MUIA_Listview_DoubleClick, TRUE,           obj, 2, MUIM_Popstring_Close, TRUE);
    DoMethod(ST_MIMETYPE, MUIM_Notify, MUIA_Disabled,             MUIV_EveryTime, obj, 3, MUIM_Set, MUIA_Disabled, MUIV_TriggerValue);
    DoMethod(ST_MIMETYPE, MUIM_Notify, MUIA_String_Contents,      MUIV_EveryTime, obj, 3, MUIM_Set, ATTR(MimeTypeChanged), MUIV_TriggerValue);
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
      case ATTR(ExtensionObject):
      {
        data->ST_EXTENSION = (Object *)tag->ti_Data;
        // make the superMethod call ignore those tags
        tag->ti_Tag = TAG_IGNORE;
      }
      break;

      case ATTR(DescriptionObject):
      {
        data->ST_EXTENSION = (Object *)tag->ti_Data;
        // make the superMethod call ignore those tags
        tag->ti_Tag = TAG_IGNORE;
      }
      break;

      case ATTR(MimeType):
      {
        nnset(data->ST_MIMETYPE, MUIA_String_Contents, tag->ti_Data);
        // make the superMethod call ignore those tags
        tag->ti_Tag = TAG_IGNORE;
      }
      break;

      case MUIA_Disabled:
      {
        nnset(data->ST_MIMETYPE, MUIA_Disabled, tag->ti_Data);
        nnset(data->BT_POPUP, MUIA_Disabled, tag->ti_Data);
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
    case ATTR(MimeTypeChanged):
    {
      *store = TRUE;

      return TRUE;
    }
    break;

    case ATTR(MimeType):
    {
      *store = xget(data->ST_MIMETYPE, MUIA_String_Contents);

      return TRUE;
    }
    break;
  }

  return DoSuperMethodA(cl, obj, msg);
}

///
/// OVERLOAD(MUIM_GoActive)
OVERLOAD(MUIM_GoActive)
{
  GETDATA;

  ENTER();

  // forward the method to the string object
  DoMethodA(data->ST_MIMETYPE, msg);

  RETURN(0);
  return 0;
}

///
/// OVERLOAD(MUIM_BetterString_DoAction)
OVERLOAD(MUIM_BetterString_DoAction)
{
  GETDATA;

  ENTER();

  // forward the method to the string object
  DoMethodA(data->ST_MIMETYPE, msg);

  RETURN(0);
  return 0;
}

///
