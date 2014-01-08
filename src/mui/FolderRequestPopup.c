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
 Description: Popobject to select a folder

***************************************************************************/

#include "FolderRequestPopup_cl.h"

#include <proto/muimaster.h>
#include <mui/NList_mcc.h>
#include <mui/NListtree_mcc.h>
#include <mui/NListview_mcc.h>

#include "SDI_hook.h"

#include "YAM.h"

#include "mui/FolderRequestListtree.h"

#include "DynamicString.h"
#include "Locale.h"
#include "FolderList.h"
#include "MUIObjects.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *self;
  Object *TX_FOLDER;
  Object *LT_FOLDER;

  int folderID;
  struct Folder *folder;
  char folderPath[SIZE_PATH];

  struct Hook Text2ListHook;
  struct Hook List2TextHook;
};
*/

/* Private Functions */
/// BuildFolderPathString
// recursively build a path string like "group1 / group2 / folder"
static void BuildFolderPathString(struct Data *data, const struct Folder *folder)
{
  ENTER();

  if(isGroupFolder(folder) == TRUE)
  {
    // prepend the group path
    char *childPath;

    // duplicate the path to avoid overwriting it
    if((childPath = strdup(data->folderPath)) != NULL)
    {
      // construct the new path
      snprintf(data->folderPath, sizeof(data->folderPath), "%s / %s", folder->Name, childPath);
      free(childPath);
    }
  }
  else
  {
    // use the plain folder name
    strlcpy(data->folderPath, folder->Name, sizeof(data->folderPath));
  }

  // prepend parent folder group names except the root group
  if(folder->parent != NULL)
    BuildFolderPathString(data, folder->parent->folder);

  LEAVE();
}

///

/* Hooks */
/// Text2ListFunc
// select the currently active folder
HOOKPROTONONP(Text2ListFunc, BOOL)
{
  struct Data *data = (struct Data *)hook->h_Data;

  ENTER();

  if(data->folder != NULL)
    DoMethod(data->LT_FOLDER, MUIM_NListtree_FindUserData, MUIV_NListtree_FindUserData_ListNode_Root, data->folder->self, MUIV_NListtree_FindUserData_Flag_Activate);
  else
    set(data->LT_FOLDER, MUIA_NListtree_Active, MUIV_NListtree_Active_Off);

  RETURN(TRUE);
  return TRUE;
}
MakeStaticHook(Text2ListHook, Text2ListFunc);

///
/// List2TextFunc
// update the text object with the newly selected folder
HOOKPROTONONP(List2TextFunc, void)
{
  struct Data *data = (struct Data *)hook->h_Data;
  struct MUI_NListtree_TreeNode *tn;

  ENTER();

  tn = (struct MUI_NListtree_TreeNode *)xget(data->LT_FOLDER, MUIA_NListtree_Active);
  if(tn != NULL && tn->tn_User != NULL)
  {
    struct FolderNode *fnode = (struct FolderNode *)tn->tn_User;
    struct Folder *folder = fnode->folder;

    data->folderID = folder->ID;
    data->folder = folder;
    BuildFolderPathString(data, folder);
    set(data->TX_FOLDER, MUIA_Text_Contents, data->folderPath);
    set(data->self, ATTR(FolderChanged), folder->ID);
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
  Object *LT_FOLDER;
  Object *BT_FOLDER;

  ENTER();

  if((obj = DoSuperNew(cl, obj,
    MUIA_Popstring_String, TX_FOLDER = TextObject,
      TextFrame,
      MUIA_Text_Copy, FALSE,
    End,
    MUIA_Popstring_Button, BT_FOLDER = PopButton(MUII_PopUp),
    MUIA_Popobject_WindowHook, &PO_WindowHook,
    MUIA_Popobject_Object, NListviewObject,
      MUIA_FixHeightTxt, "\n\n\n\n\n\n\n\n",
      MUIA_NListview_NList, LT_FOLDER = FolderRequestListtreeObject,
        MUIA_NList_DoubleClick, TRUE,
      End,
    End,
    TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;

    data->self = obj;
    data->TX_FOLDER = TX_FOLDER;
    data->LT_FOLDER = LT_FOLDER;

    InitHook(&data->Text2ListHook, Text2ListHook, data);
    InitHook(&data->List2TextHook, List2TextHook, data);
    xset(obj,
      MUIA_Popobject_StrObjHook, &data->Text2ListHook,
      MUIA_Popobject_ObjStrHook, &data->List2TextHook);

    set(BT_FOLDER, MUIA_CycleChain, TRUE);
    DoMethod(LT_FOLDER, MUIM_Notify, MUIA_NList_DoubleClick, TRUE, obj, 2, MUIM_Popstring_Close, TRUE);
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
      case ATTR(FolderID):
      {
        data->folderID = tag->ti_Data;
        if((data->folder = FindFolderByID(G->folders, data->folderID)) != NULL)
        {
          BuildFolderPathString(data, data->folder);
        }
        else
        {
          data->folderPath[0] = '\0';
          if(data->folderID != 0)
            W(DBF_FOLDER, "cannot resolve folder ID 0x%08lx", data->folderID);
        }

        set(data->TX_FOLDER, MUIA_Text_Contents, data->folderPath);

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

    case ATTR(FolderID):
    {
      *store = data->folderID;

      return TRUE;
    }
    break;

    case ATTR(FolderName):
    {
      *store = (IPTR)((data->folderID != 0 && data->folder != NULL) ? data->folder->Name : "");

      return TRUE;
    }
    break;
  }

  return DoSuperMethodA(cl, obj, msg);
}

///
