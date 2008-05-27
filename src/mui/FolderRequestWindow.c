/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2008 by YAM Open Source Team

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

 Superclass:  MUIC_Window
 Description: Puts up a string requester for entering a PGP passphrase

***************************************************************************/

#include "FolderRequestWindow_cl.h"

#include "YAM.h"

#include "FolderList.h"
#include "Locale.h"
#include "MUIObjects.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *listviewObj;
  ULONG result;
};
*/

///
/// DisplayHook
HOOKPROTONH(DisplayFunc, LONG, char **array, struct Folder *folder)
{
  if(folder != NULL)
  {
    array[0] = folder->Name;
  }

  return 0;
}
MakeStaticHook(DisplayHook, DisplayFunc);

///

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  struct TagItem *tags = inittags(msg), *tag;
  char *titleText = (char *)"YAM";
  char *bodyText = NULL;
  char *yesText = (char *)tr(MSG_Okay);
  char *noText = (char *)tr(MSG_Cancel);
  struct Folder *prevFolder = NULL;
  struct Folder *excludeFolder = NULL;
  Object *listviewObj = NULL;
  Object *yesButton = NULL;
  Object *noButton = NULL;

  ENTER();

  while((tag = NextTagItem(&tags)) != NULL)
  {
    switch(tag->ti_Tag)
    {
      case MUIA_Window_Title: titleText = (char *)tag->ti_Data; tag->ti_Tag = TAG_IGNORE; break;
      ATTR(Body):             bodyText = (char *)tag->ti_Data; break;
      ATTR(YesText):          yesText = (char *)tag->ti_Data; break;
      ATTR(NoText):           noText = (char *)tag->ti_Data; break;
      ATTR(Exclude):          excludeFolder = (struct Folder *)tag->ti_Data; break;
      ATTR(Folder):           prevFolder = (struct Folder *)tag->ti_Data; break;
    }
  }

  if((obj = DoSuperNew(cl, obj,

    MUIA_Window_Title,     titleText,
    MUIA_Window_LeftEdge,  MUIV_Window_LeftEdge_Centered,
    MUIA_Window_TopEdge,   MUIV_Window_TopEdge_Centered,
    MUIA_Window_Height,    MUIV_Window_Height_MinMax(30),
    MUIA_Window_ID,        MAKE_ID('F','R','E','Q'),
    WindowContents, VGroup,
      MUIA_Background, MUII_RequesterBack,
      Child, VGroup,
        GroupFrame,
        MUIA_Background, MUII_GroupBack,
        Child, LLabel(bodyText),
        Child, listviewObj = ListviewObject,
          MUIA_CycleChain, TRUE,
          MUIA_Listview_DoubleClick, TRUE,
          MUIA_Listview_List, ListObject,
            InputListFrame,
            MUIA_List_AutoVisible, TRUE,
            MUIA_List_DisplayHook, &DisplayHook,
          End,
        End,
      End,
      Child, ColGroup(3),
        Child, yesButton = MakeButton(yesText),
        Child, HSpace(0),
        Child, noButton = MakeButton(noText),
      End,
    End,

    TAG_MORE, (ULONG)inittags(msg))) != NULL)
  {
    GETDATA;

    data->listviewObj = listviewObj;
    data->result = 0;

    LockFolderListShared(G->folders);

    if(IsFolderListEmpty(G->folders) == FALSE)
    {
      struct FolderNode *fnode;
      ULONG pos = 0;

      ForEachFolderNode(G->folders, fnode)
      {
        if(!isGroupFolder(fnode->folder))
        {
          // check if the folder is to be excluded
          if(fnode->folder != excludeFolder)
          {
            DoMethod(listviewObj, MUIM_List_InsertSingle, fnode->folder, MUIV_List_Insert_Bottom);
            // mark the previously selected folder
            if(fnode->folder == prevFolder)
              set(listviewObj, MUIA_List_Active, pos);
          }
        }
        pos++;
      }
    }

    UnlockFolderList(G->folders);

    DoMethod(obj, MUIM_Notify, MUIA_Window_CloseRequest, TRUE, obj, 2, MUIM_FolderRequestWindow_FinishInput, 0);
    DoMethod(yesButton, MUIM_Notify, MUIA_Pressed, FALSE, obj, 2, MUIM_FolderRequestWindow_FinishInput, 1);
    DoMethod(noButton, MUIM_Notify, MUIA_Pressed, FALSE, obj, 2, MUIM_FolderRequestWindow_FinishInput, 0);
    DoMethod(listviewObj, MUIM_Notify, MUIA_Listview_DoubleClick, MUIV_EveryTime, 2, MUIM_FolderRequestWindow_FinishInput, 1);

    set(obj, MUIA_Window_ActiveObject, listviewObj);
  }

  RETURN(obj);
  return (ULONG)obj;
}

///
/// OVERLOAD(OM_GET)
OVERLOAD(OM_GET)
{
  GETDATA;
  ULONG *store = ((struct opGet *)msg)->opg_Storage;

  switch(((struct opGet *)msg)->opg_AttrID)
  {
    ATTR(Result):
    {
      *store = data->result;
      return TRUE;
    }

    ATTR(Folder):
    {
      DoMethod(data->listviewObj, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, store);
      return TRUE;
    }
  }

  return DoSuperMethodA(cl, obj, msg);
}

///

/* Private Functions */

/* Public Methods */
/// DECLARE(FinishInput)
//
DECLARE(FinishInput) // ULONG result
{
  GETDATA;

  ENTER();

  data->result = msg->result;

  // trigger possible notifications
  set(obj, MUIA_FolderRequestWindow_Result, msg->result);

  RETURN(0);
  return 0;
}

///
