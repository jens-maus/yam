/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2007 by YAM Open Source Team

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.   See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 YAM Official Support Site : http://www.yam.ch
 YAM OpenSource project     : http://sourceforge.net/projects/yamos/

 $Id$

 Superclass:  MUIC_Group
 Description: Groups multiple 'MainMailList' objects into one PageGroup

***************************************************************************/

#include "MainMailListGroup_cl.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *mainListObjects[2];
  struct Folder *lastActiveFolder;
  ULONG lastActiveEntry;
  ULONG activeList;
};
*/

/* EXPORT
enum MainListType { LT_MAIN=0, LT_QUICKVIEW };
*/

/* Private Functions */

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  struct Data *data;
  Object *mainList;
  Object *quickviewList;
  int i;

  if(!(obj = DoSuperNew(cl, obj,

    MUIA_Group_PageMode, TRUE,

    Child, NListviewObject,
       MUIA_NListview_NList, mainList = MainMailListObject,
          MUIA_ObjectID,                   MAKE_ID('N','L','0','2'),
          MUIA_ContextMenu,                C->MessageCntMenu ? MUIV_NList_ContextMenu_Always : MUIV_NList_ContextMenu_Never,
          MUIA_NList_DragType,             MUIV_NList_DragType_Default,
          MUIA_NList_Exports,              MUIV_NList_Exports_ColWidth|MUIV_NList_Exports_ColOrder,
          MUIA_NList_Imports,              MUIV_NList_Imports_ColWidth|MUIV_NList_Imports_ColOrder,
       End,
    End,
    Child, NListviewObject,
       MUIA_NListview_NList, quickviewList = MainMailListObject,
          MUIA_ContextMenu,                C->MessageCntMenu ? MUIV_NList_ContextMenu_Always : MUIV_NList_ContextMenu_Never,
          MUIA_NList_DragType,             MUIV_NList_DragType_Default,
       End,
    End,

    TAG_MORE, inittags(msg))))
  {
    return 0;
  }

  data = (struct Data *)INST_DATA(cl,obj);

  data->mainListObjects[LT_MAIN] = mainList;
  data->mainListObjects[LT_QUICKVIEW] = quickviewList;

  // make sure we generate the NList_Format of both objects now
  for(i=LT_MAIN; i <= LT_QUICKVIEW; i++)
  {
    DoMethod(data->mainListObjects[i], MUIM_MainMailList_MakeFormat);
    DoMethod(data->mainListObjects[i], MUIM_Notify, MUIA_NList_SelectChange,TRUE,           MUIV_Notify_Application, 2, MUIM_CallHook, &MA_ChangeSelectedHook);
    DoMethod(data->mainListObjects[i], MUIM_Notify, MUIA_NList_Active,      MUIV_EveryTime, MUIV_Notify_Application, 2, MUIM_CallHook, &MA_SetMessageInfoHook);
    DoMethod(data->mainListObjects[i], MUIM_Notify, MUIA_NList_DoubleClick, MUIV_EveryTime, obj,                     2, MUIM_MainMailListGroup_DoubleClicked, MUIV_TriggerValue);
  }

  return (ULONG)obj;
}

///
/// OVERLOAD(OM_GET)
// this is to delegate the OM_GET to the correct NLists
OVERLOAD(OM_GET)
{
  GETDATA;
  ULONG *store = ((struct opGet *)msg)->opg_Storage;

  switch(((struct opGet *)msg)->opg_AttrID)
  {
    ATTR(ActiveList):       *store = data->activeList; return TRUE;
    ATTR(ActiveListObject): *store = (ULONG)data->mainListObjects[data->activeList]; return TRUE;
    ATTR(MainList):          *store = (ULONG)data->mainListObjects[LT_MAIN]; return TRUE;

    // we also return foreign attributes
    case MUIA_NList_Active:
    case MUIA_NList_Entries:
    {
      *store = xget(data->mainListObjects[data->activeList], ((struct opGet *)msg)->opg_AttrID);
      return TRUE;
    }
    break;
  }

  return DoSuperMethodA(cl, obj, msg);
}
///
/// OVERLOAD(OM_SET)
OVERLOAD(OM_SET)
{
  GETDATA;
  struct TagItem *tags = inittags(msg), *tag;

  while((tag = NextTagItem(&tags)))
  {
    switch(tag->ti_Tag)
    {
      case MUIA_Group_ActivePage:
      {
        if(data->activeList != tag->ti_Data)
        {
          // set the new mainlist as the default object of the window it belongs to
          // but only if not another one is yet active
          if((Object*)xget(_win(obj), MUIA_Window_DefaultObject) == data->mainListObjects[data->activeList])
            set(_win(obj), MUIA_Window_DefaultObject, data->mainListObjects[tag->ti_Data]);

          data->activeList = tag->ti_Data;
        }
      }
      break;

      case MUIA_NList_Active:
      case MUIA_NList_SelectChange:
      {
        set(data->mainListObjects[data->activeList], tag->ti_Tag, tag->ti_Data);

        // make the superMethod call ignore those tags
        tag->ti_Tag = TAG_IGNORE;
      }
      break;

      case MUIA_ContextMenu:
      case MUIA_NList_DisplayRecall:
      case MUIA_NList_Quiet:
      case MUIA_NList_SortType:
      case MUIA_NList_SortType2:
      {
        set(data->mainListObjects[LT_MAIN], tag->ti_Tag, tag->ti_Data);
        set(data->mainListObjects[LT_QUICKVIEW], tag->ti_Tag, tag->ti_Data);

        // make the superMethod call ignore those tags
        tag->ti_Tag = TAG_IGNORE;
      }
      break;
    }
  }

  return DoSuperMethodA(cl, obj, msg);
}
///
/// OVERLOAD(MUIM_NList_Clear)
OVERLOAD(MUIM_NList_Clear)
{
  GETDATA;
  ULONG result;

  // delegate this method to the currently active NList only
  result = DoMethodA(data->mainListObjects[data->activeList], msg);

  return result;
}

///
/// OVERLOAD(MUIM_NList_GetEntry)
OVERLOAD(MUIM_NList_GetEntry)
{
  GETDATA;
  ULONG result;

  // delegate this method to the currently active NList only
  result = DoMethodA(data->mainListObjects[data->activeList], msg);

  return result;
}

///
/// OVERLOAD(MUIM_NList_GetPos)
OVERLOAD(MUIM_NList_GetPos)
{
  GETDATA;
  ULONG result;

  // delegate this method to the currently active NList only
  result = DoMethodA(data->mainListObjects[data->activeList], msg);

  return result;
}

///
/// OVERLOAD(MUIM_NList_Insert)
OVERLOAD(MUIM_NList_Insert)
{
  GETDATA;
  ULONG result;

  // delegate this method to the currently active NList only
  result = DoMethodA(data->mainListObjects[data->activeList], msg);

  return result;
}

///
/// OVERLOAD(MUIM_NList_InsertSingle)
OVERLOAD(MUIM_NList_InsertSingle)
{
  GETDATA;
  ULONG result;

  // we always add the mail to the main list
  result = DoMethodA(data->mainListObjects[LT_MAIN], msg);

  // the InsertSignal method is used by all internal parties
  // to put a mail into a main listview. But in case we have a
  // quicksearchbar we have to check whether this also matches
  // any criteria of it and also put it in there
  if(data->activeList == LT_QUICKVIEW)
  {
    struct MUIP_NList_InsertSingle* m = (struct MUIP_NList_InsertSingle*)msg;

    // check if mail matches the search/view criteria of the quicksearchbar
    if(DoMethod(G->MA->GUI.GR_QUICKSEARCHBAR, MUIM_QuickSearchBar_MatchMail, m->entry) == TRUE)
    {
      result = DoMethodA(data->mainListObjects[LT_QUICKVIEW], msg);

      // make sure the statistics of the quicksearchbar are updated.
      DoMethod(G->MA->GUI.GR_QUICKSEARCHBAR, MUIM_QuickSearchBar_UpdateStats, FALSE);
    }
  }

  return result;
}

///
/// OVERLOAD(MUIM_NList_NextSelected)
OVERLOAD(MUIM_NList_NextSelected)
{
  GETDATA;
  ULONG result;

  // delegate this method to the currently active NList only
  result = DoMethodA(data->mainListObjects[data->activeList], msg);

  return result;
}

///
/// OVERLOAD(MUIM_NList_Redraw)
OVERLOAD(MUIM_NList_Redraw)
{
  GETDATA;
  ULONG result;

  // delegate this method to the currently active NList only
  result = DoMethodA(data->mainListObjects[data->activeList], msg);

  return result;
}

///
/// OVERLOAD(MUIM_NList_Select)
OVERLOAD(MUIM_NList_Select)
{
  GETDATA;
  ULONG result;

  // delegate this method to the currently active NList only
  result = DoMethodA(data->mainListObjects[data->activeList], msg);

  return result;
}

///
/// OVERLOAD(MUIM_NList_Sort)
OVERLOAD(MUIM_NList_Sort)
{
  GETDATA;

  // delegate this method call to all subNLists
  DoMethodA(data->mainListObjects[LT_MAIN], msg);
  DoMethodA(data->mainListObjects[LT_QUICKVIEW], msg);

  return 0;
}

///

/* Private Methods */
/// DECLARE(DoubleClicked)
// if the user double-clicked in the mail list we either
// have to open the message in a read window or if it is currently in
// the outgoing folder we open it for editing.
DECLARE(DoubleClicked) // LONG entryNum
{
  ENTER();

  if(msg->entryNum >= 0)
  {
    struct Folder *folder = FO_GetCurrentFolder();

    // A double click in the outgoing folder should popup a write
    // window instead.
    if(folder != NULL && isOutgoingFolder(folder))
    {
      // in case the folder is the "outgoing" folder
      // we edit the mail instead.
      DoMethod(G->App, MUIM_CallHook, &MA_NewMessageHook, NEW_EDIT, 0);
    }
    else
    {
      // if not, then we open a read window instead
      DoMethod(G->App, MUIM_CallHook, &MA_ReadMessageHook);
    }
  }

  RETURN(0);
  return 0;
}

///

/* Public Methods */
/// DECLARE(MakeFormat)
DECLARE(MakeFormat)
{
  GETDATA;

  // forward the MakeFormat message call to all our sublists as well
  DoMethod(data->mainListObjects[LT_MAIN],       MUIM_MainMailList_MakeFormat);
  DoMethod(data->mainListObjects[LT_QUICKVIEW], MUIM_MainMailList_MakeFormat);

  return 0;
}

///
/// DECLARE(SwitchToList)
DECLARE(SwitchToList) // enum MainListType type
{
  GETDATA;

  ENTER();

  // no matter what, we always clear the quickview list on a switch.
  if(data->activeList != msg->type)
  {
    int i;

    // before we switch the activePage of the group object we
    // have to set the individual column width of the two NLists as we only save
    // the width of one.
    for(i=0; i < MACOLNUM; i++)
    {
      LONG colWidth = DoMethod(data->mainListObjects[data->activeList], MUIM_NList_ColWidth, i, MUIV_NList_ColWidth_Get);

      // set the columnwidth of the LT_QUICKVIEW maillist also the same
      DoMethod(data->mainListObjects[msg->type], MUIM_NList_ColWidth, i, colWidth != -1 ? colWidth : MUIV_NList_ColWidth_Default);
    }

    // switch the page of the group now
    set(obj, MUIA_Group_ActivePage, msg->type);

    if(msg->type == LT_MAIN)
    {
      struct Folder *curFolder = FO_GetCurrentFolder();

      // in case we are switching from LT_QUICKVIEW->LT_MAIN we go and set the
      // last active mail as well.
      if(curFolder && curFolder->LastActive >= 0)
      {
        SetAttrs(data->mainListObjects[LT_MAIN], MUIA_NList_Active,       curFolder->LastActive,
                                                 MUIA_NList_SelectChange, TRUE,
                                                 TAG_DONE);
      }
    }
  }

  DoMethod(data->mainListObjects[LT_QUICKVIEW], MUIM_NList_Clear);

  RETURN(0);
  return 0;
}

///
/// DECLARE(AddMailToList)
// add a mail to a specific list
DECLARE(AddMailToList) // enum MainListType type, struct Mail* mail
{
  GETDATA;

  ASSERT(msg->type <= LT_QUICKVIEW);

  // we add the mail to a specific list of our group
  DoMethod(data->mainListObjects[msg->type], MUIM_NList_InsertSingle, msg->mail, MUIV_NList_Insert_Sorted);

  // update the searchbar statistics if necessary
  if(data->activeList == LT_QUICKVIEW)
    DoMethod(G->MA->GUI.GR_QUICKSEARCHBAR, MUIM_QuickSearchBar_UpdateStats, FALSE);

  return 0;
}

///
/// DECLARE(RemoveMail)
// properly removes a mail from both lists
DECLARE(RemoveMail) // struct Mail* mail
{
  GETDATA;
  ULONG result;

  ENTER();

  // first we check whether the active one was the quickview and if so we also remove
  // the mail from the main list
  if(data->activeList == LT_QUICKVIEW)
  {
    // forward the command to the listview itself
    DoMethod(data->mainListObjects[LT_MAIN], MUIM_MainMailList_RemoveMail, msg->mail);
  }

  // now also remove the mail from the currently active list
  result = DoMethod(data->mainListObjects[data->activeList], MUIM_MainMailList_RemoveMail, msg->mail);

  RETURN(result);
  return result;
}

///
/// DECLARE(RedrawMail)
// redraws the mail on our currently active listview
DECLARE(RedrawMail) // struct Mail* mail
{
  GETDATA;
  LONG pos = MUIV_NList_GetPos_Start;
  BOOL result = FALSE;
  ENTER();

  DoMethod(data->mainListObjects[data->activeList], MUIM_NList_GetPos, msg->mail, &pos);
  if(pos != MUIV_NList_GetPos_End)
  {
    DoMethod(data->mainListObjects[data->activeList], MUIM_NList_Redraw, pos);
    result = TRUE;
  }

  RETURN(result);
  return result;
}

///
/// DECLARE(IsMailList)
// checks if a passed object pointer is one of our maillists (for checking dragdrop requests)
DECLARE(IsMailList) // Object* list
{
  GETDATA;

  ASSERT(msg->list != NULL);

  return (ULONG)(msg->list == data->mainListObjects[LT_MAIN] ||
                 msg->list == data->mainListObjects[LT_QUICKVIEW]);
}

///
