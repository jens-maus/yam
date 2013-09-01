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
 Description: Popobject to select a folder

***************************************************************************/

#include "FolderRequestPopup_cl.h"

#include <proto/muimaster.h>
#include <mui/NList_mcc.h>
#include <mui/NListtree_mcc.h>
#include <mui/NListview_mcc.h>

#include "SDI_hook.h"

#include "Locale.h"
#include "FolderList.h"
#include "MUIObjects.h"

#include "mui/FolderRequestListtree.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *TX_FOLDER;
};
*/

/* Hooks */
/// Text2ListFunc
//  selects the folder as active which is currently in the 'str'
//  object
HOOKPROTONH(Text2ListFunc, BOOL, Object *listview, Object *str)
{
  char *s;

  ENTER();

  // get the currently set string
  s = (char *)xget(str, MUIA_Text_Contents);

  if(s != NULL && listview != NULL)
  {
    Object *list = (Object *)xget(listview, MUIA_NListview_NList);

    // now try to find the node and activate it right away
    DoMethod(list, MUIM_NListtree_FindName, MUIV_NListtree_FindName_ListNode_Root, s, MUIV_NListtree_FindName_Flag_Activate);
  }

  RETURN(TRUE);
  return TRUE;
}
MakeStaticHook(Text2ListHook, Text2ListFunc);

///
/// List2TextFunc
//  Copies listview selection to text gadget
HOOKPROTONH(List2TextFunc, void, Object *listview, Object *text)
{
  Object *list;

  ENTER();

  if((list = (Object *)xget(listview, MUIA_NListview_NList)) != NULL && text != NULL)
  {
    struct MUI_NListtree_TreeNode *tn = (struct MUI_NListtree_TreeNode *)xget(list, MUIA_NListtree_Active);

    if(tn != NULL && tn->tn_User != NULL)
    {
      struct FolderNode *fnode = (struct FolderNode *)tn->tn_User;
      set(text, MUIA_Text_Contents, fnode->folder->Name);
    }
  }

  LEAVE();
}
MakeStaticHook(List2TextHook, List2TextFunc);

///

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  Object *TX_FOLDER;
  Object *LV_FOLDER;
  Object *BT_FOLDER;

  ENTER();

  if((obj = DoSuperNew(cl, obj,
    MUIA_Popstring_String, TX_FOLDER = TextObject,
      TextFrame,
      MUIA_Text_Copy, FALSE,
    End,
    MUIA_Popstring_Button, BT_FOLDER = PopButton(MUII_PopUp),
    MUIA_Popobject_StrObjHook, &Text2ListHook,
    MUIA_Popobject_ObjStrHook, &List2TextHook,
    MUIA_Popobject_WindowHook, &PO_WindowHook,
    MUIA_Popobject_Object, NListviewObject,
      MUIA_NListview_NList, LV_FOLDER = FolderRequestListtreeObject,
        MUIA_NList_DoubleClick, TRUE,
      End,
    End,
    TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;

    data->TX_FOLDER = TX_FOLDER;

    set(BT_FOLDER, MUIA_CycleChain, TRUE);
    DoMethod(LV_FOLDER, MUIM_Notify, MUIA_NList_DoubleClick, TRUE, obj, 2, MUIM_Popstring_Close, TRUE);
    DoMethod(TX_FOLDER, MUIM_Notify, MUIA_Text_Contents, MUIV_EveryTime, obj, 3, MUIM_Set, ATTR(FolderChanged), TRUE);
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
      case ATTR(Folder):
      {
        nnset(data->TX_FOLDER, MUIA_Text_Contents, tag->ti_Data);
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
    case ATTR(FolderChanged):
    {
      *store = TRUE;

      return TRUE;
    }
    break;

    case ATTR(Folder):
    {
      *store = xget(data->TX_FOLDER, MUIA_Text_Contents);

      return TRUE;
    }
    break;
  }

  return DoSuperMethodA(cl, obj, msg);
}

///
