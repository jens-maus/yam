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

 Superclass:  MUIC_FolderListtree
 Description: NListtree class of the main folder list in the main window

***************************************************************************/

#include "MainFolderListtree_cl.h"

#include <string.h>
#include <proto/dos.h>
#include <proto/muimaster.h>
#include <libraries/gadtools.h>
#include <libraries/iffparse.h>
#include <mui/NList_mcc.h>
#include <mui/NListtree_mcc.h>

#include "YAM.h"
#include "YAM_find.h"
#include "YAM_mainFolder.h"

#include "mui/FolderEditWindow.h"
#include "mui/ImageArea.h"
#include "mui/MainMailListGroup.h"
#include "mui/YAMApplication.h"

#include "Config.h"
#include "FolderList.h"
#include "Locale.h"
#include "MUIObjects.h"
#include "Requesters.h"
#include "Themes.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *contextMenu;
  Object *folderEditWindow;
  struct Folder newFolder;
  BOOL draggingMails;
  BOOL reorderFolderList;
  struct MUI_EventHandlerNode eh;
};
*/

/* INCLUDE
#include "YAM_folderconfig.h"
*/

/* EXPORT
#define NUMBER_FOLDERTREE_COLUMNS 5
*/

enum
{
  CMN_EDITF=10,
  CMN_DELETEF,
  CMN_INDEX,
  CMN_NEWF,
  CMN_NEWFG,
  CMN_EMPTYTRASH,
  CMN_EMPTYSPAM,
  CMN_ALLTOREAD,
  CMN_SEARCH
};

/* Private Functions */

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  ENTER();

  if((obj = DoSuperNew(cl, obj,

    InputListFrame,
    MUIA_ObjectID,                    MAKE_ID('N','L','0','1'),
    MUIA_ContextMenu,                 C->FolderCntMenu ? MUIV_NList_ContextMenu_Always : 0,
    MUIA_Font,                        C->FixedFontList ? MUIV_NList_Font_Fixed : MUIV_NList_Font,
    MUIA_Dropable,                    TRUE,
    MUIA_NList_DragType,              MUIV_NList_DragType_Immediate,
    MUIA_NList_DragSortable,          TRUE,
    MUIA_NList_ActiveObjectOnClick,   TRUE,
    MUIA_NList_DefaultObjectOnClick,  FALSE,
    MUIA_NList_Exports,               MUIV_NList_Exports_ColWidth|MUIV_NList_Exports_ColOrder,
    MUIA_NList_Imports,               MUIV_NList_Imports_ColWidth|MUIV_NList_Imports_ColOrder,
    MUIA_NListtree_DragDropSort,      TRUE,
    MUIA_NListtree_Title,             TRUE,
    MUIA_NListtree_DoubleClick,       MUIV_NListtree_DoubleClick_All,

    TAG_MORE, inittags(msg))) != NULL)
  {
    DoMethod(obj, MUIM_Notify, MUIA_NList_DoubleClick, MUIV_EveryTime, MUIV_Notify_Self, 2, METHOD(EditFolder), TRUE);
    //DoMethod(obj, MUIM_Notify, MUIA_NList_TitleClick,    MUIV_EveryTime, MUIV_Notify_Self, 3, MUIM_NList_Sort2,          MUIV_TriggerValue,MUIV_NList_SortTypeAdd_2Values);
    //DoMethod(obj, MUIM_Notify, MUIA_NList_SortType,      MUIV_EveryTime, MUIV_Notify_Self, 3, MUIM_Set,                  MUIA_NList_TitleMark,MUIV_TriggerValue);
    DoMethod(obj, MUIM_Notify, MUIA_NListtree_Active, MUIV_EveryTime, MUIV_Notify_Self, 2, METHOD(ChangeFolder), MUIV_TriggerValue);

    DoMethod(obj, METHOD(MakeFormat));
  }

  RETURN((IPTR)obj);
  return (IPTR)obj;
}

///
/// OVERLOAD(OM_DISPOSE)
OVERLOAD(OM_DISPOSE)
{
  IPTR result;
  GETDATA;

  ENTER();

  // make sure that our context menus are also disposed
  if(data->contextMenu != NULL)
    MUI_DisposeObject(data->contextMenu);

  // dispose ourself
  result = DoSuperMethodA(cl, obj, msg);

  RETURN(result);
  return result;
}

///
/// OVERLOAD(OM_SET)
OVERLOAD(OM_SET)
{
  GETDATA;
  struct TagItem *tags = inittags(msg), *tag;

  while((tag = NextTagItem((APTR)&tags)) != NULL)
  {
    switch(tag->ti_Tag)
    {
      case ATTR(ReorderFolderList):
      {
        data->reorderFolderList = tag->ti_Data;
        // make the superMethod call ignore those tags
        tag->ti_Tag = TAG_IGNORE;
      }
      break;
    }
  }

  return DoSuperMethodA(cl, obj, msg);
}

///
/// OVERLOAD(OM_GET)
OVERLOAD(OM_GET)
{
  IPTR *store = ((struct opGet *)msg)->opg_Storage;

  switch(((struct opGet *)msg)->opg_AttrID)
  {
    case ATTR(TreeChanged): *store = TRUE; return TRUE;
  }

  return DoSuperMethodA(cl, obj, msg);
}

///
/// OVERLOAD(MUIM_Setup)
OVERLOAD(MUIM_Setup)
{
  GETDATA;
  IPTR result;

  ENTER();

  if((result = DoSuperMethodA(cl, obj, msg)))
  {
    data->eh.ehn_Class  = cl;
    data->eh.ehn_Object = obj;
    data->eh.ehn_Events = IDCMP_RAWKEY;
    data->eh.ehn_Flags  = MUI_EHF_GUIMODE;
    data->eh.ehn_Priority = -1;

    if(_win(obj) != NULL)
      DoMethod(_win(obj), MUIM_Window_AddEventHandler, &data->eh);
  }

  RETURN(result);
  return result;
}

///
/// OVERLOAD(MUIM_Cleanup)
OVERLOAD(MUIM_Cleanup)
{
  GETDATA;
  IPTR result;

  ENTER();

  if(_win(obj) != NULL)
    DoMethod(_win(obj), MUIM_Window_RemEventHandler, &data->eh);

  result = DoSuperMethodA(cl, obj, msg);

  RETURN(result);
  return result;
}

///
/// OVERLOAD(MUIM_HandleEvent)
OVERLOAD(MUIM_HandleEvent)
{
  struct MUIP_HandleEvent *mhe = (struct MUIP_HandleEvent *)msg;
  IPTR result = 0;

  ENTER();

  if(mhe->imsg->Class == IDCMP_RAWKEY)
  {
    if(mhe->imsg->Code >= 1 && mhe->imsg->Code <= 10)
    {
      struct MUI_NListtree_TreeNode *tn = NULL;
      int count = mhe->imsg->Code;
      int i;

      // we get the first entry and if it's a LIST we have to get the next one
      // and so on, until we have a real entry for that key or we set nothing active
      for(i=count; i <= count; i++)
      {
        tn = (struct MUI_NListtree_TreeNode *)DoMethod(obj, MUIM_NListtree_GetEntry, MUIV_NListtree_GetEntry_ListNode_Root, i-1, MUIF_NONE);
        if(tn == NULL)
          break;

        if(isFlagSet(tn->tn_Flags, TNF_LIST))
          count++;
      }

      if(tn != NULL)
      {
        // force that the list is open at this entry
        DoMethod(obj, MUIM_NListtree_Open, MUIV_NListtree_Open_ListNode_Parent, tn, MUIF_NONE);
        // now set this treenode active
        set(obj, MUIA_NListtree_Active, tn);
      }

      // eat the key press in any case
      result = MUI_EventHandlerRC_Eat;
    }
  }

  RETURN(result);
  return result;
}

///
/// OVERLOAD(MUIM_DragQuery)
OVERLOAD(MUIM_DragQuery)
{
  GETDATA;
  struct MUIP_DragQuery *dq = (struct MUIP_DragQuery *)msg;
  ULONG result;

  ENTER();

  // check if the object that requests the drag operation
  // is a mail list object or not
  if(DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_MainMailListGroup_IsMailList, dq->obj) == TRUE)
  {
    data->draggingMails = TRUE;
    result = MUIV_DragQuery_Accept;
  }
  else
  {
    data->draggingMails = FALSE;
    result = DoSuperMethodA(cl, obj, msg);
  }

  RETURN(result);
  return result;
}

///
/// OVERLOAD(MUIM_DragDrop)
OVERLOAD(MUIM_DragDrop)
{
  IPTR result;
  struct MUIP_DragDrop *dd = (struct MUIP_DragDrop *)msg;

  ENTER();

  if(dd->obj == obj)
  {
    // A folder was dragged onto another folder we break here and
    // let the super class do the dirty work. This will invoke the
    // MUIM_NListtree_Move method, which we also catch to move the
    // folder node within our folder list.
    result = DoSuperMethodA(cl, obj, msg);
  }
  else
  {
    struct MUI_NListtree_TreeNode *tn_dst;

    if((tn_dst = (struct MUI_NListtree_TreeNode *)xget(obj, MUIA_NListtree_DropTarget)) != NULL)
    {
      struct Folder *dstfolder = ((struct FolderNode *)tn_dst->tn_User)->folder;

      if(isGroupFolder(dstfolder) == FALSE)
        MA_MoveCopy(NULL, dstfolder, "manual drag", MVCPF_CLOSE_WINDOWS);
    }

    result = 0;
  }

  RETURN(result);
  return result;
}

///
/// OVERLOAD(MUIM_NListtree_DropType)
OVERLOAD(MUIM_NListtree_DropType)
{
  GETDATA;
  struct MUIP_NListtree_DropType *dt = (struct MUIP_NListtree_DropType *)msg;
  struct MUI_NListtree_TreeNode *tn;

  ENTER();

  // get the current drop target
  if((tn = (struct MUI_NListtree_TreeNode *)xget(obj, MUIA_NListtree_DropTarget)) != NULL)
  {
    struct Folder *folder;

    if((folder = ((struct FolderNode *)tn->tn_User)->folder) != NULL)
    {
      if(data->draggingMails == TRUE)
      {
        // if mails are being dragged the currently active folder and group folders must be excluded.
        // All other folders are valid drop targets.
        if(*dt->Pos == (LONG)xget(obj, MUIA_NListtree_Active) || isGroupFolder(folder))
        {
          if(isFlagSet(tn->tn_Flags, TNF_LIST) && isFlagClear(tn->tn_Flags, TNF_OPEN))
            *dt->Type = MUIV_NListtree_DropType_Onto;
          else
            *dt->Type = MUIV_NListtree_DropType_None;
        }
        else
          *dt->Type = MUIV_NListtree_DropType_Onto;
      }
      else
      {
        // if folders are being dragged only group folders are valid drop targets. Else we place the
        // folder being dragged above the current folder below the mouse pointer.
        if(*dt->Type == MUIV_NListtree_DropType_Onto && !isGroupFolder(folder))
          *dt->Type = MUIV_NListtree_DropType_Above;
      }
    }
    else
      *dt->Type = MUIV_NListtree_DropType_None;
  }
  else
    *dt->Type = MUIV_NListtree_DropType_None;

  RETURN(0);
  return 0;
}

///
/// OVERLOAD(MUIM_NListtree_Insert)
OVERLOAD(MUIM_NListtree_Insert)
{
  GETDATA;
  struct MUI_NListtree_TreeNode *thisTreeNode;

  ENTER();

  // first let the list tree class do the actual insertion of the tree nodes
  if((thisTreeNode = (struct MUI_NListtree_TreeNode *)DoSuperMethodA(cl, obj, msg)) != NULL)
  {
    // reorder the folder list only if we are explicitly told to do so
    if(data->reorderFolderList == TRUE)
    {
      struct MUI_NListtree_TreeNode *prevTreeNode;

      // now determine the previous node
      if((prevTreeNode = (struct MUI_NListtree_TreeNode *)DoMethod(obj, MUIM_NListtree_GetEntry, thisTreeNode, MUIV_NListtree_GetEntry_Position_Previous, MUIF_NONE)) != NULL)
      {
        struct FolderNode *thisFNode;
        struct FolderNode *prevFNode;
        struct MUI_NListtree_TreeNode *parentTreeNode;

        thisFNode = (struct FolderNode *)thisTreeNode->tn_User;
        prevFNode = (struct FolderNode *)prevTreeNode->tn_User;

        // if the folder is to be moved behind a group folder then we have
        // to get the bottom-most leaf of that group, otherwise the global
        // folder list will be screwed up. This must be iterated through all
        // nested groups.
        while(isGroupFolder(prevFNode->folder))
        {
          prevTreeNode = (struct MUI_NListtree_TreeNode *)DoMethod(obj, MUIM_NListtree_GetEntry, prevTreeNode, MUIV_NListtree_GetEntry_Position_Tail, MUIF_NONE);
          prevFNode = (struct FolderNode *)prevTreeNode->tn_User;
        }

        // set the parent treenode of the new folder
        parentTreeNode = (struct MUI_NListtree_TreeNode *)DoMethod(obj, MUIM_NListtree_GetEntry, thisTreeNode, MUIV_NListtree_GetEntry_Position_Parent, MUIF_NONE);
        thisFNode->folder->parent = (struct FolderNode *)parentTreeNode->tn_User;

        // finally move the folder node within the exclusively locked folder list
        D(DBF_FOLDER, "insert folder '%s' behind folder '%s'", thisFNode->folder->Name, prevFNode->folder->Name);
        LockFolderList(G->folders);
        MoveFolderNode(G->folders, thisFNode, prevFNode);
        UnlockFolderList(G->folders);
      }
    }

    // trigger a changed folder tree
    set(obj, ATTR(TreeChanged), TRUE);
  }

  RETURN(thisTreeNode);
  return (IPTR)thisTreeNode;
}

///
/// OVERLOAD(MUIM_NListtree_Move)
OVERLOAD(MUIM_NListtree_Move)
{
  IPTR result;
  struct MUIP_NListtree_Move *mv = (struct MUIP_NListtree_Move *)msg;
  struct MUI_NListtree_TreeNode *prevTreeNode;

  ENTER();

  // first let the list tree class do the actual movement of the tree nodes
  result = DoSuperMethodA(cl, obj, msg);

  // now determine the previous node
  if((prevTreeNode = (struct MUI_NListtree_TreeNode *)DoMethod(obj, MUIM_NListtree_GetEntry, mv->OldTreeNode, MUIV_NListtree_GetEntry_Position_Previous, MUIF_NONE)) != NULL)
  {
    struct FolderNode *thisFNode;
    struct FolderNode *prevFNode;

    thisFNode = (struct FolderNode *)mv->OldTreeNode->tn_User;
    prevFNode = (struct FolderNode *)prevTreeNode->tn_User;

    // if the folder is to be moved behind a group folder then we have
    // to get the bottom-most leaf of that group, otherwise the global
    // folder list will be screwed up. This must be iterated through all
    // nested groups.
    while(isGroupFolder(prevFNode->folder))
    {
      prevTreeNode = (struct MUI_NListtree_TreeNode *)DoMethod(obj, MUIM_NListtree_GetEntry, prevTreeNode, MUIV_NListtree_GetEntry_Position_Tail, MUIF_NONE);
      prevFNode = (struct FolderNode *)prevTreeNode->tn_User;
    }

    // finally move the folder node within the exclusively locked folder list
    D(DBF_FOLDER, "move folder '%s' behind folder '%s'", thisFNode->folder->Name, prevFNode->folder->Name);
    LockFolderList(G->folders);
    MoveFolderNode(G->folders, thisFNode, prevFNode);
    UnlockFolderList(G->folders);

    // trigger a changed folder tree
    set(obj, ATTR(TreeChanged), TRUE);
  }

  RETURN(result);
  return result;
}

///
/// OVERLOAD(MUIM_NList_ContextMenuBuild)
OVERLOAD(MUIM_NList_ContextMenuBuild)
{
  GETDATA;
  struct MUIP_NList_ContextMenuBuild *m = (struct MUIP_NList_ContextMenuBuild *)msg;
  struct MUI_NListtree_TestPos_Result r;
  struct MUI_NListtree_TreeNode *tn;
  struct Folder *folder = NULL;
  Object *lastItem;
  BOOL disable_delete = FALSE;
  BOOL disable_edit = FALSE;
  BOOL disable_update = FALSE;
  BOOL disable_alltoread = FALSE;
  BOOL disable_search = FALSE;

  ENTER();

  // dispose the old contextMenu if it still exists
  if(data->contextMenu)
  {
    MUI_DisposeObject(data->contextMenu);
    data->contextMenu = NULL;
  }

  // if this was a RMB click on the titlebar we create our own special menu
  if(m->ontop)
  {
    data->contextMenu = MenustripObject,
      Child, MenuObjectT(tr(MSG_MA_CTX_FOLDERLIST)),
        Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_Folder), MUIA_Menuitem_CopyStrings, FALSE, MUIA_UserData, 1, MUIA_Menuitem_Enabled, FALSE, MUIA_Menuitem_Checked, TRUE, MUIA_Menuitem_Checkit, TRUE, MUIA_Menuitem_Toggle, TRUE, End,
        Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_Total),  MUIA_Menuitem_CopyStrings, FALSE, MUIA_UserData, 2, MUIA_Menuitem_Checked, hasFColTotal(C->FolderCols), MUIA_Menuitem_Checkit, TRUE, MUIA_Menuitem_Toggle, TRUE, End,
        Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_Unread), MUIA_Menuitem_CopyStrings, FALSE, MUIA_UserData, 3, MUIA_Menuitem_Checked, hasFColUnread(C->FolderCols), MUIA_Menuitem_Checkit, TRUE, MUIA_Menuitem_Toggle, TRUE, End,
        Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_New),    MUIA_Menuitem_CopyStrings, FALSE, MUIA_UserData, 4, MUIA_Menuitem_Checked, hasFColNew(C->FolderCols), MUIA_Menuitem_Checkit, TRUE, MUIA_Menuitem_Toggle, TRUE, End,
        Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_Size),   MUIA_Menuitem_CopyStrings, FALSE, MUIA_UserData, 5, MUIA_Menuitem_Checked, hasFColSize(C->FolderCols), MUIA_Menuitem_Checkit, TRUE, MUIA_Menuitem_Toggle, TRUE, End,
        Child, MenuitemObject, MUIA_Menuitem_Title, NM_BARLABEL, End,
        Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_MA_CTX_DEFWIDTH_THIS), MUIA_Menuitem_CopyStrings, FALSE, MUIA_UserData, MUIV_NList_Menu_DefWidth_This, End,
        Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_MA_CTX_DEFWIDTH_ALL),  MUIA_Menuitem_CopyStrings, FALSE, MUIA_UserData, MUIV_NList_Menu_DefWidth_All,  End,
        Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_MA_CTX_DEFORDER_THIS), MUIA_Menuitem_CopyStrings, FALSE, MUIA_UserData, MUIV_NList_Menu_DefOrder_This, End,
        Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_MA_CTX_DEFORDER_ALL),  MUIA_Menuitem_CopyStrings, FALSE, MUIA_UserData, MUIV_NList_Menu_DefOrder_All,  End,
      End,
    End;

    RETURN((IPTR)data->contextMenu);
    return (IPTR)data->contextMenu;
  }

  // Now lets find out which entry is under the mouse pointer
  DoMethod(obj, MUIM_NListtree_TestPos, m->mx, m->my, &r);

  tn = r.tpr_TreeNode;

  if(tn == NULL || tn->tn_User == NULL)
  {
    disable_delete = TRUE;
    disable_edit   = TRUE;
    disable_update = TRUE;
    disable_alltoread = TRUE;
    disable_search = TRUE;
  }
  else
  {
    folder = ((struct FolderNode *)tn->tn_User)->folder;

    // Set this Treenode as activ
    if(tn != (struct MUI_NListtree_TreeNode *)xget(obj, MUIA_NListtree_Active))
      set(obj, MUIA_NListtree_Active, tn);

    // Now we have to set the disabled flag if this is not a custom folder
    if(isDefaultFolder(folder) && !isGroupFolder(folder))
      disable_delete = TRUE;

    if(isGroupFolder(folder))
    {
      disable_update = TRUE;
      disable_search = TRUE;
      disable_alltoread = TRUE;
    }

    if(isSentMailFolder(folder))
      disable_alltoread = TRUE;
  }

  // We create the ContextMenu now
  data->contextMenu = MenustripObject,
    Child, MenuObjectT(folder ? FolderName(folder) : tr(MSG_FOLDER_NONSEL)),
      Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_FOLDER_NEWFOLDER),            MUIA_Menuitem_CopyStrings, FALSE, MUIA_UserData, CMN_NEWF,   End,
      Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_FOLDER_NEWFOLDERGROUP),       MUIA_Menuitem_CopyStrings, FALSE, MUIA_UserData, CMN_NEWFG,  End,
      Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_FOLDER_EDIT),                 MUIA_Menuitem_CopyStrings, FALSE, MUIA_Menuitem_Enabled, !disable_edit,   MUIA_UserData, CMN_EDITF,   End,
      Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_FOLDER_DELETE),               MUIA_Menuitem_CopyStrings, FALSE, MUIA_Menuitem_Enabled, !disable_delete, MUIA_UserData, CMN_DELETEF, End,
      Child, MenuitemObject, MUIA_Menuitem_Title, NM_BARLABEL, End,
      Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_MA_MSEARCH),                  MUIA_Menuitem_CopyStrings, FALSE, MUIA_Menuitem_Enabled, !disable_search, MUIA_UserData, CMN_SEARCH,  End,
      Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_MA_UPDATEINDEX),              MUIA_Menuitem_CopyStrings, FALSE, MUIA_Menuitem_Enabled, !disable_update, MUIA_UserData, CMN_INDEX,   End,
      Child, MenuitemObject, MUIA_Menuitem_Title, NM_BARLABEL, End,
      Child, lastItem = MenuitemObject, MUIA_Menuitem_Title, tr(MSG_FOLDER_ALLTOREAD), MUIA_Menuitem_CopyStrings, FALSE, MUIA_Menuitem_Enabled, !disable_alltoread, MUIA_UserData, CMN_ALLTOREAD, End,
    End,
  End;

  // depending on the folder we have to append some additional
  // menu items or not.
  if(folder != NULL)
  {
    Object *newItem;

    // check if this is the trash folder
    if(isTrashFolder(folder) &&
       (newItem = Menuitem(tr(MSG_MA_REMOVEDELETED), NULL, TRUE, FALSE, CMN_EMPTYTRASH)) != NULL)
    {
      DoMethod(data->contextMenu, MUIM_Family_Insert, newItem, lastItem);
      lastItem = newItem;
    }

    // check if this is the SPAM folder
    if(C->SpamFilterEnabled &&
       (newItem = Menuitem(tr(MSG_MA_REMOVESPAM), NULL, !isGroupFolder(folder), FALSE, CMN_EMPTYSPAM)) != NULL)
    {
      DoMethod(data->contextMenu, MUIM_Family_Insert, newItem, lastItem);
      lastItem = newItem;
    }
  }

  RETURN((IPTR)data->contextMenu);
  return (IPTR)data->contextMenu;
}

///
/// OVERLOAD(MUIM_ContextMenuChoice)
OVERLOAD(MUIM_ContextMenuChoice)
{
  struct MUIP_ContextMenuChoice *m = (struct MUIP_ContextMenuChoice *)msg;
  ULONG result = 0;

  ENTER();

  switch(xget(m->item, MUIA_UserData))
  {
    // if the user selected a TitleContextMenu item
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    {
      ULONG flag = (1 << (xget(m->item, MUIA_UserData)-1));

      if(isFlagSet(C->FolderCols, flag))
        clearFlag(C->FolderCols, flag);
      else
        setFlag(C->FolderCols, flag);

      DoMethod(obj, METHOD(MakeFormat));
    }
    break;

    // or other item out of the FolderListContextMenu
    case CMN_EDITF:     { DoMethod(obj, METHOD(EditFolder), FALSE); } break;
    case CMN_DELETEF:   { DoMethod(obj, METHOD(DeleteFolder)); } break;
    case CMN_INDEX:     { DoMethod(_app(obj), MUIM_YAMApplication_RebuildFolderIndex); } break;
    case CMN_NEWF:      { DoMethod(obj, METHOD(NewFolder)); } break;
    case CMN_NEWFG:     { DoMethod(obj, METHOD(NewFolderGroup), NULL); } break;
    case CMN_EMPTYTRASH:{ DoMethod(_app(obj), MUIM_YAMApplication_EmptyTrashFolder, FALSE); } break;
    case CMN_EMPTYSPAM: { DoMethod(_app(obj), MUIM_YAMApplication_DeleteSpamMails, FALSE); } break;
    case CMN_ALLTOREAD: { DoMethod(_app(obj), MUIM_CallHook, &MA_SetAllStatusToHook, SFLAG_READ, SFLAG_NEW); } break;
    case CMN_SEARCH:    { DoMethod(_app(obj), MUIM_YAMApplication_OpenSearchMailWindow, GetCurrentFolder()); } break;

    default:
      result = DoSuperMethodA(cl, obj, (Msg)msg);
  }

  RETURN(result);
  return result;
}

///

/* Public Methods */
/// DECLARE(MakeFormat)
// creates format definition for folder listtree
DECLARE(MakeFormat)
{
  static const int defwidth[NUMBER_FOLDERTREE_COLUMNS] = { 100,0,0,0,0 };
  char format[SIZE_LARGE];
  BOOL first = TRUE;
  int i;

  ENTER();

  *format = '\0';

  for(i = 0; i < NUMBER_FOLDERTREE_COLUMNS; i++)
  {
    if(isFlagSet(C->FolderCols, (1<<i)))
    {
      int p;

      if(first)
        first = FALSE;
      else
        strlcat(format, " TBAR,", sizeof(format));

      p = strlen(format);
      snprintf(&format[p], sizeof(format)-p, "COL=%d W=%d", i, defwidth[i]);

      if(i > 0)
        strlcat(format, " P=\033r", sizeof(format));
      else
        strlcat(format, " PCS=C", sizeof(format));
    }
  }
  strlcat(format, " TBAR", sizeof(format));

  SHOWSTRING(DBF_GUI, format);

  // set the new NList_Format to our object
  set(obj, MUIA_NList_Format, format);

  RETURN(0);
  return 0;
}

///
/// DECLARE(ChangeFolder)
// set the clicked folder as the current one
DECLARE(ChangeFolder) // struct MUI_NListtree_TreeNode *treenode
{
  struct MUI_NListtree_TreeNode *tn = msg->treenode;

  ENTER();

  // check the treenode and its user data, this method may be invoked with a NULL treenode
  if(tn != NULL && tn->tn_User != NULL)
  {
    struct FolderNode *fnode = (struct FolderNode *)tn->tn_User;

    SetCurrentFolder(fnode->folder);
    MA_ChangeFolder(NULL, FALSE);
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(NewFolderGroup)
// creates a new folder group
DECLARE(NewFolderGroup) // char *name
{
  struct Folder folder;
  ULONG result = FALSE;

  ENTER();

  InitFolder(&folder, FT_GROUP);

  // either use the supplied name or prompt the user to enter one otherwise
  if(msg->name != NULL)
  {
    strlcpy(folder.Name, msg->name, sizeof(folder.Name));
  }
  else
  {
    if(StringRequest(folder.Name, sizeof(folder.Name), tr(MSG_FO_NEWFGROUP), tr(MSG_FO_NEWFGROUPREQ), tr(MSG_Okay), NULL, tr(MSG_Cancel), FALSE, _win(obj)) == 0)
      folder.Name[0] = '\0';
  }

  if(folder.Name[0] != '\0')
  {
    struct FolderNode *fnode;

    LockFolderList(G->folders);
    fnode = AddNewFolderNode(G->folders, memdup(&folder, sizeof(folder)));
    UnlockFolderList(G->folders);

    if(fnode != NULL)
    {
      // remember the backlink to our own folder node
      fnode->folder->self = fnode;

      // insert the new folder node and remember its treenode pointer
      if((fnode->folder->Treenode = (struct MUI_NListtree_TreeNode *)DoMethod(obj, MUIM_NListtree_Insert, folder.Name, fnode, MUIV_NListtree_Insert_ListNode_Root, MUIV_NListtree_Insert_PrevNode_Tail, TNF_LIST | TNF_OPEN)) != NULL)
        set(obj, MUIA_NListtree_Active, fnode->folder->Treenode);

      result = FO_SaveTree();
    }
  }

  RETURN(result);
  return result;
}

///
/// DECLARE(NewFolder)
// creates a new folder
DECLARE(NewFolder)
{
  GETDATA;
  int mode;
  BOOL openEditWindow = TRUE;

  ENTER();

  // call MUI_Request() first
  mode = MUI_Request(_app(obj), _win(obj), MUIF_NONE, tr(MSG_MA_NewFolder), tr(MSG_FO_NewFolderGads), tr(MSG_FO_NewFolderReq));

  // reset the folder struct and set some default values
  InitFolder(&data->newFolder, FT_CUSTOM);

  switch (mode)
  {
    case 1: break;
    case 2:
    {
      // as the user decided to use the settings from the current folder, we copy
      // the current one to our new one.
      memcpy(&data->newFolder, GetCurrentFolder(), sizeof(data->newFolder));

      if(isGroupFolder(&data->newFolder))
      {
        DoMethod(obj, METHOD(NewFolderGroup), NULL);
        openEditWindow = FALSE;
      }
      else
      {
        if(isIncomingFolder(&data->newFolder) || isTrashFolder(&data->newFolder) || isDraftsFolder(&data->newFolder) || isSpamFolder(&data->newFolder) || isArchiveFolder(&data->newFolder))
          data->newFolder.Type = FT_CUSTOM;
        else if(isOutgoingFolder(&data->newFolder) || isSentFolder(&data->newFolder))
          data->newFolder.Type = FT_CUSTOMSENT;

        // now that we have the correct folder type, we set some default values for the new
        // folder
        data->newFolder.Path[0] = '\0';
        data->newFolder.Name[0] = '\0';
        data->newFolder.imageObject = NULL;
        // erase the message list which might have been copied from the current folder
        data->newFolder.messages = NULL;
        // no image for the folder by default
        data->newFolder.ImageIndex = -1;
      }
    }
    break;

    case 3:
    {
      char foldersPath[SIZE_PATHFILE];
      struct FileReqCache *frc;

      CreateFilename("Folders", foldersPath, sizeof(foldersPath));
      if((frc = ReqFile(ASL_FOLDER, _win(obj), tr(MSG_FO_SelectDir), REQF_DRAWERSONLY, foldersPath, "")) != NULL)
      {
        strlcpy(data->newFolder.Path, frc->drawer, sizeof(data->newFolder.Path));

        FO_LoadConfig(&data->newFolder);
      }
      else
      {
        openEditWindow = FALSE;
      }
    }
    break;

    default:
    {
      openEditWindow = FALSE;
    }
  }

  if(openEditWindow == TRUE)
  {
    // there is no "old" folder which could be edited, just the new one
    DoMethod(obj, METHOD(OpenFolderEditWindow), NULL);
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(EditFolder)
//  Opens folder window to edit the settings of the active folder
DECLARE(EditFolder) // ULONG wasDoubleClick
{
  GETDATA;

  ENTER();

  // respect the configuration about editing on double click
  if(msg->wasDoubleClick == FALSE || C->FolderDoubleClick == TRUE)
  {
    struct Folder *folder = GetCurrentFolder();

    // copy the current folder as this will be used for editing
    memcpy(&data->newFolder, GetCurrentFolder(), sizeof(data->newFolder));
    if(isGroupFolder(folder))
    {
      // don't edit folder groups on double click
      // a double click is used to fold/unfold the group
      if(msg->wasDoubleClick == FALSE)
      {
        if(StringRequest(folder->Name, SIZE_NAME, tr(MSG_FO_EDIT_FGROUP), tr(MSG_FO_EDIT_FGROUPREQ), tr(MSG_Okay), NULL, tr(MSG_Cancel), FALSE, _win(obj)))
          DoMethod(obj, MUIM_NListtree_Redraw, MUIV_NListtree_Redraw_Active, MUIF_NONE);
      }
    }
    else
    {
      DoMethod(obj, METHOD(OpenFolderEditWindow), folder);
    }
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(OpenFolderEditWindow)
// create and open a new folder edit window if it does exist yet
DECLARE(OpenFolderEditWindow) // struct Folder *folder
{
  GETDATA;

  ENTER();

  if(data->folderEditWindow == NULL)
  {
    Object *folderEditWindow;

    if((folderEditWindow = FolderEditWindowObject, End) != NULL)
    {
      if(SafeOpenWindow(folderEditWindow) == TRUE)
      {
        data->folderEditWindow = folderEditWindow;
        // dispose the folder edit window whenever it asks for it
        DoMethod(folderEditWindow, MUIM_Notify, MUIA_FolderEditWindow_DisposeRequest, MUIV_EveryTime, obj, 2, METHOD(CloseFolderEditWindow), FALSE);
      }
      else
      {
        // the folder edit window adds itself to the application, hence it is not
        // enough to just dispose the object
        DoMethod(_app(obj), MUIM_YAMApplication_DisposeWindow, folderEditWindow);
      }
    }
  }

  if(data->folderEditWindow != NULL)
  {
    // set the folder to be edited, the old folder might be NULL in case of a new folder
    xset(data->folderEditWindow,
      MUIA_FolderEditWindow_OldFolder, msg->folder,
      MUIA_FolderEditWindow_EditFolder, &data->newFolder);
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(CloseFolderEditWindow)
// dispose the folder edit window
DECLARE(CloseFolderEditWindow) // ULONG immediately
{
  GETDATA;

  ENTER();

  if(data->folderEditWindow != NULL)
  {
    if(msg->immediately == TRUE)
    {
      DoMethod(_app(obj), MUIM_YAMApplication_DisposeWindow, data->folderEditWindow);
    }
    else
    {
      // don't dispose the window directly here, because this method is called
      // as a direct notification of the window's close request and disposing
      // it immediately would "pull the rug out" from under the window.
      DoMethod(_app(obj), MUIM_Application_PushMethod, _app(obj), 2, MUIM_YAMApplication_DisposeWindow, data->folderEditWindow);
    }
    data->folderEditWindow = NULL;
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(DeleteFolder)
// removes the active folder
DECLARE(DeleteFolder)
{
  struct Folder *folder;
  struct FolderNode *fnode;
  BOOL delete_folder = FALSE;

  ENTER();

  folder = GetCurrentFolder();
  fnode = (struct FolderNode *)folder->Treenode->tn_User;

  switch(folder->Type)
  {
    case FT_CUSTOM:
    case FT_CUSTOMSENT:
    case FT_CUSTOMMIXED:
    case FT_ARCHIVE:
    {
      if(MUI_Request(_app(obj), _win(obj), MUIF_NONE, NULL, tr(MSG_YesNoReq2), tr(MSG_CO_ConfirmDelete)) != 0)
      {
        // check if the folder that is about to be deleted is part
        // of an active filter and if so remove it from it
        if(FolderIsUsedByFilters(folder) == TRUE)
          RemoveFolderFromFilters(folder);

        // Here we dispose the folderimage Object because the destructor
        // of the Folder Listtree can't do this without throwing enforcer hits
        if(folder->imageObject != NULL)
        {
          // we make sure that the NList also doesn't use the image in future anymore
          DoMethod(obj, MUIM_NList_UseImage, NULL, folder->ImageIndex, MUIF_NONE);

          // and last, but not least we free the BC object here, so that this Object is also gone
          MUI_DisposeObject(folder->imageObject);
          folder->imageObject = NULL; // let's set it to NULL so that the destructor doesn't do the work again.
        }

        delete_folder = TRUE;
        DeleteMailDir(folder->Fullpath, FALSE);
      }
    }
    break;

    case FT_GROUP:
    {
      struct MUI_NListtree_TreeNode *tn_sub;
      struct MUI_NListtree_TreeNode *tn_group = folder->Treenode;

      // check if the active treenode is a list and if it is empty
      // we have to do this like the following because there is no other way to
      // get known if the active entry has subentries.
      if((tn_sub = (struct MUI_NListtree_TreeNode *)DoMethod(obj, MUIM_NListtree_GetEntry, tn_group, MUIV_NListtree_GetEntry_Position_Head, MUIF_NONE)) != NULL)
      {
        // Now we popup a requester and if this requester is confirmed we move the subentries to the parent node.
        if(MUI_Request(_app(obj), _win(obj), MUIF_NONE, NULL, tr(MSG_YesNoReq2), tr(MSG_FO_GROUP_CONFDEL)))
        {
          struct MUI_NListtree_TreeNode *tn_sub_next = tn_sub;

          delete_folder = TRUE;

          set(obj, MUIA_NListtree_Quiet, TRUE);

          while(tn_sub_next != NULL)
          {
            tn_sub_next = (struct MUI_NListtree_TreeNode *)DoMethod(obj, MUIM_NListtree_GetEntry, tn_sub, MUIV_NListtree_GetEntry_Position_Next, MUIV_NListtree_GetEntry_Flag_SameLevel);

            // move entry to the parent of the group
            DoMethod(obj, MUIM_NListtree_Move, tn_group, tn_sub, MUIV_NListtree_Move_NewListNode_Active, MUIV_NListtree_Move_NewTreeNode_Tail, MUIF_NONE);

            tn_sub = tn_sub_next;
          }

          set(obj, MUIA_NListtree_Quiet, FALSE);
        }
      }
      else
        delete_folder = TRUE;
    }
    break;

    default:
      DisplayBeep(_screen(obj));
    break;
  }

  if(delete_folder == TRUE)
  {
    D(DBF_FOLDER, "deleting folder '%s'", folder->Name);

    // remove the entry from the listtree now
    DoMethod(obj, MUIM_NListtree_Remove, MUIV_NListtree_Remove_ListNode_Root, MUIV_NListtree_Remove_TreeNode_Active, MUIF_NONE);

    // save the Tree to the folder config now
    FO_SaveTree();

    // remove the folder from the global folder list
    LockFolderList(G->folders);
    RemoveFolderNode(G->folders, fnode);
    UnlockFolderList(G->folders);

    // finally free all the memory
    DeleteFolderNode(fnode);
    FreeFolder(folder);

    // update the statistics in case the just deleted folder contained new or unread mail
    DisplayStatistics(NULL, TRUE);

    // trigger a changed folder tree
    set(obj, ATTR(TreeChanged), TRUE);
  }
  else
    D(DBF_FOLDER, "keeping folder '%s'", folder->Name);

  RETURN(0);
  return 0;
}

///
