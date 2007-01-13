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
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 YAM Official Support Site :  http://www.yam.ch
 YAM OpenSource project    :  http://sourceforge.net/projects/yamos/

 $Id$

 Superclass:  MUIC_NListtree
 Description: NListtree class of the main folder list in the main window

***************************************************************************/

#include "MainFolderListtree_cl.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *context_menu;
  Object *folderImage[MAX_FOLDERIMG+1];
};
*/

enum { CMN_EDITF=10,
       CMN_DELETEF,
       CMN_INDEX,
       CMN_NEWF,
       CMN_NEWFG,
       CMN_SNAPS,
       CMN_RELOAD,
       CMN_EMPTYTRASH,
       CMN_EMPTYSPAM };

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  struct Data *data;
  int i;

  ENTER();

  if(!(obj = DoSuperNew(cl, obj,
    TAG_MORE, inittags(msg))))
  {
    RETURN(0);
    return 0;
  }

  data = (struct Data *)INST_DATA(cl,obj);

  // prepare the folder images
  data->folderImage[FICON_ID_FOLD]        = MakeImageObject("folder_fold");
  data->folderImage[FICON_ID_UNFOLD]      = MakeImageObject("folder_unfold");
  data->folderImage[FICON_ID_INCOMING]    = MakeImageObject("folder_incoming");
  data->folderImage[FICON_ID_INCOMING_NEW]= MakeImageObject("folder_incoming_new");
  data->folderImage[FICON_ID_OUTGOING]    = MakeImageObject("folder_outgoing");
  data->folderImage[FICON_ID_OUTGOING_NEW]= MakeImageObject("folder_outgoing_new");
  data->folderImage[FICON_ID_TRASH]       = MakeImageObject("folder_deleted");
  data->folderImage[FICON_ID_TRASH_NEW]   = MakeImageObject("folder_deleted_new");
  data->folderImage[FICON_ID_SENT]        = MakeImageObject("folder_sent");
  data->folderImage[FICON_ID_PROTECTED]   = MakeImageObject("status_crypt");
  data->folderImage[FICON_ID_SPAM]        = MakeImageObject("folder_spam");
  data->folderImage[FICON_ID_SPAM_NEW]    = MakeImageObject("folder_spam_new");
  for(i=0; i < MAX_FOLDERIMG+1; i++)
    DoMethod(obj, MUIM_NList_UseImage, data->folderImage[i], i, MUIF_NONE);

  RETURN((ULONG)obj);
  return (ULONG)obj;
}

///
/// OVERLOAD(OM_DISPOSE)
OVERLOAD(OM_DISPOSE)
{
  GETDATA;
  int i;

  // make sure that our context menus are also disposed
  if(data->context_menu)
    MUI_DisposeObject(data->context_menu);

  for(i=0; i < MAX_FOLDERIMG+1; i++)
  {
    DoMethod(obj, MUIM_NList_UseImage, NULL, i, MUIF_NONE);
    if(data->folderImage[i])
    {
      MUI_DisposeObject(data->folderImage[i]);
      data->folderImage[i] = NULL;
    }
  }

  return DoSuperMethodA(cl,obj,msg);
}

///
/// OVERLOAD(OM_GET)
// get some stuff of our instance data
OVERLOAD(OM_GET)
{
  GETDATA;
  ULONG *store = ((struct opGet *)msg)->opg_Storage;

  switch(((struct opGet *)msg)->opg_AttrID)
  {
    ATTR(ImageArray): *store = (ULONG)data->folderImage; return TRUE;
  }

  return DoSuperMethodA(cl, obj, msg);
}

///
/// OVERLOAD(MUIM_DragReport)
// we catch MUIM_DragReport because we want to restrict some
// dragging for some special objects
OVERLOAD(MUIM_DragReport)
{
  struct MUIP_DragReport *dr = (struct MUIP_DragReport *)msg;
  struct MUI_NListtree_TestPos_Result res;
  struct MUI_NListtree_TreeNode *tn;

  DoMethod(obj, MUIM_NListtree_TestPos, dr->x, dr->y, &res);

  if((tn = res.tpr_TreeNode))
  {
    struct Folder *folder = (struct Folder *)tn->tn_User;

    // If we drag a folder on a folder we reject it immediatly because only below or above
    // is allowed
    if(dr->obj == obj)
    {
      if(!isGroupFolder(folder) && res.tpr_Type == MUIV_NListtree_TestPos_Result_Onto)
      {
        return(MUIV_DragReport_Abort);
      }
    }
    else
    {
      // If we drag a mail onto a folder we allow only dragging on and not below or above
      if(isGroupFolder(folder) || res.tpr_Type != MUIV_NListtree_TestPos_Result_Onto)
      {
        return(MUIV_DragReport_Abort);
      }
    }

    // to rescue the dropping we call the SuperMethod now
    return(DoSuperMethodA(cl, obj, msg));
  }

  return(MUIV_DragReport_Abort);
}

///
/// OVERLOAD(MUIM_DragQuery)
OVERLOAD(MUIM_DragQuery)
{
  struct MUIP_DragQuery *dq = (struct MUIP_DragQuery *)msg;

  // check if the object that requests the drag operation
  // is a mail list object or not
  if(DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_MainMailListGroup_IsMailList, dq->obj) == TRUE)
    return MUIV_DragQuery_Accept;

  return DoSuperMethodA(cl,obj,msg);
}

///
/// OVERLOAD(MUIM_DragDrop)
OVERLOAD(MUIM_DragDrop)
{
  struct MUIP_DragDrop *dd = (struct MUIP_DragDrop *)msg;

  // if a folder is dragged on a folder we break here and the SuperClass should handle the msg
  if(dd->obj != obj)
  {
    struct Folder *srcfolder;
    struct Folder *dstfolder;
    struct MUI_NListtree_TreeNode *tn_src;
    struct MUI_NListtree_TreeNode *tn_dst;
    
    tn_dst = (struct MUI_NListtree_TreeNode *)xget(obj, MUIA_NListtree_DropTarget);
    if(!tn_dst)
      return 0;
    
    dstfolder = tn_dst->tn_User;

    tn_src = (struct MUI_NListtree_TreeNode *)xget(obj, MUIA_NListtree_Active);
    if(!tn_src)
      return 0;
    
    srcfolder = tn_src->tn_User;

    if(!isGroupFolder(dstfolder))
      MA_MoveCopy(NULL, srcfolder, dstfolder, FALSE, TRUE);
    
    return 0;
  }

  return DoSuperMethodA(cl,obj,msg);
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
  struct MA_GUIData *gui = &G->MA->GUI;
  Object *lastItem;
  BOOL disable_delete = FALSE;
  BOOL disable_edit   = FALSE;
  BOOL disable_update = FALSE;

  ENTER();

  // dispose the old context_menu if it still exists
  if(data->context_menu)
  {
    MUI_DisposeObject(data->context_menu);
    data->context_menu = NULL;
  }

  // if this was a RMB click on the titlebar we create our own special menu
  if(m->ontop)
  {
    data->context_menu = MenustripObject,
      Child, MenuObjectT(tr(MSG_MA_CTX_FOLDERLIST)),
        Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_Folder), MUIA_UserData, 1, MUIA_Menuitem_Enabled, FALSE, MUIA_Menuitem_Checked, isFlagSet(C->FolderCols, (1<<0)), MUIA_Menuitem_Checkit, TRUE, MUIA_Menuitem_Toggle, TRUE, End,
        Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_Total),  MUIA_UserData, 2, MUIA_Menuitem_Checked, isFlagSet(C->FolderCols, (1<<1)), MUIA_Menuitem_Checkit, TRUE, MUIA_Menuitem_Toggle, TRUE, End,
        Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_Unread), MUIA_UserData, 3, MUIA_Menuitem_Checked, isFlagSet(C->FolderCols, (1<<2)), MUIA_Menuitem_Checkit, TRUE, MUIA_Menuitem_Toggle, TRUE, End,
        Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_New),    MUIA_UserData, 4, MUIA_Menuitem_Checked, isFlagSet(C->FolderCols, (1<<3)), MUIA_Menuitem_Checkit, TRUE, MUIA_Menuitem_Toggle, TRUE, End,
        Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_Size),   MUIA_UserData, 5, MUIA_Menuitem_Checked, isFlagSet(C->FolderCols, (1<<4)), MUIA_Menuitem_Checkit, TRUE, MUIA_Menuitem_Toggle, TRUE, End,
        Child, MenuitemObject, MUIA_Menuitem_Title, NM_BARLABEL, End,
        Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_MA_CTX_DEFWIDTH_THIS), MUIA_UserData, MUIV_NList_Menu_DefWidth_This, End,
        Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_MA_CTX_DEFWIDTH_ALL),  MUIA_UserData, MUIV_NList_Menu_DefWidth_All,  End,
        Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_MA_CTX_DEFORDER_THIS), MUIA_UserData, MUIV_NList_Menu_DefOrder_This, End,
        Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_MA_CTX_DEFORDER_ALL),  MUIA_UserData, MUIV_NList_Menu_DefOrder_All,  End,
      End,
    End;

    RETURN((ULONG)data->context_menu);
    return (ULONG)data->context_menu;
  }

  // Now lets find out which entry is under the mouse pointer
  DoMethod(gui->NL_FOLDERS, MUIM_NListtree_TestPos, m->mx, m->my, &r);

  tn = r.tpr_TreeNode;

  if(!tn || !tn->tn_User)
  {
    disable_delete = TRUE;
    disable_edit   = TRUE;
    disable_update = TRUE;
  }
  else
  {
    folder = (struct Folder *)tn->tn_User;

    // Set this Treenode as activ
    if(tn != (struct MUI_NListtree_TreeNode *)xget(gui->NL_FOLDERS, MUIA_NListtree_Active))
      set(gui->NL_FOLDERS, MUIA_NListtree_Active, tn);

    // Now we have to set the disabled flag if this is not a custom folder
    if(isDefaultFolder(folder) && !isGroupFolder(folder))
      disable_delete = TRUE;

    if(isGroupFolder(folder))
      disable_update = TRUE;
  }

  // We create the ContextMenu now
  data->context_menu = MenustripObject,
    Child, MenuObjectT(folder ? FolderName(folder) : tr(MSG_FOLDER_NONSEL)),
      Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_FOLDER_EDIT),           MUIA_Menuitem_Enabled, !disable_edit,   MUIA_UserData, CMN_EDITF,   End,
      Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_FOLDER_DELETE),         MUIA_Menuitem_Enabled, !disable_delete, MUIA_UserData, CMN_DELETEF, End,
      Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_MA_UPDATEINDEX),        MUIA_Menuitem_Enabled, !disable_update, MUIA_UserData, CMN_INDEX,   End,
      Child, MenuitemObject, MUIA_Menuitem_Title, NM_BARLABEL, End,
      Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_FOLDER_NEWFOLDER),      MUIA_UserData, CMN_NEWF,   End,
      Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_FOLDER_NEWFOLDERGROUP), MUIA_UserData, CMN_NEWFG,  End,
      Child, MenuitemObject, MUIA_Menuitem_Title, NM_BARLABEL, End,
      Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_FOLDER_SNAPSHOT),       MUIA_UserData, CMN_SNAPS,  End,
      Child, lastItem = MenuitemObject, MUIA_Menuitem_Title, tr(MSG_FOLDER_RELOAD),         MUIA_UserData, CMN_RELOAD, End,
    End,
  End;

  // depending on the folder we have to append some additional
  // menu items or not.
  if(folder && (isTrashFolder(folder) || isSpamFolder(folder)))
  {
    Object *newItem;

    // insert a BARLABEL first
    if((newItem = MenuBarLabel) != NULL)
    {
      DoMethod(data->context_menu, MUIM_Family_Insert, newItem, lastItem);
      lastItem = newItem;
    }

    // check if this is the trash folder
    if(isTrashFolder(folder) &&
       (newItem = Menuitem(tr(MSG_MA_REMOVEDELETED), NULL, TRUE, FALSE, CMN_EMPTYTRASH)) != NULL)
    {
      DoMethod(data->context_menu, MUIM_Family_Insert, newItem, lastItem);
      lastItem = newItem;
    }

    // check if this is the SPAM folder
    if(isSpamFolder(folder) &&
       (newItem = Menuitem(tr(MSG_MA_REMOVESPAM), NULL, TRUE, FALSE, CMN_EMPTYSPAM)) != NULL)
    {
      DoMethod(data->context_menu, MUIM_Family_Insert, newItem, lastItem);
      lastItem = newItem;
    }
  }

  RETURN((ULONG)data->context_menu);
  return (ULONG)data->context_menu;
}

///
/// OVERLOAD(MUIM_ContextMenuChoice)
OVERLOAD(MUIM_ContextMenuChoice)
{
  struct MUIP_ContextMenuChoice *m = (struct MUIP_ContextMenuChoice *)msg;

  switch(xget(m->item, MUIA_UserData))
  {
    // if the user selected a TitleContextMenu item
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    {
      ULONG col = xget(m->item, MUIA_UserData)-1;

      if(isFlagSet(C->FolderCols, (1<<col))) CLEAR_FLAG(C->FolderCols, (1<<col));
      else                                   SET_FLAG(C->FolderCols, (1<<col));

      MA_MakeFOFormat(G->MA->GUI.NL_FOLDERS);
    }
    break;

    // or other item out of the FolderListContextMenu
    case CMN_EDITF:     { DoMethod(G->App, MUIM_CallHook, &FO_EditFolderHook);          } break;
    case CMN_DELETEF:   { DoMethod(G->App, MUIM_CallHook, &FO_DeleteFolderHook);        } break;
    case CMN_INDEX:     { DoMethod(G->App, MUIM_CallHook, &MA_RescanIndexHook);         } break;
    case CMN_NEWF:      { DoMethod(G->App, MUIM_CallHook, &FO_NewFolderHook);           } break;
    case CMN_NEWFG:     { DoMethod(G->App, MUIM_CallHook, &FO_NewFolderGroupHook);      } break;
    case CMN_SNAPS:     { DoMethod(G->App, MUIM_CallHook, &FO_SetOrderHook, SO_SAVE);   } break;
    case CMN_RELOAD:    { DoMethod(G->App, MUIM_CallHook, &FO_SetOrderHook, SO_RESET);  } break;
    case CMN_EMPTYTRASH:{ DoMethod(G->App, MUIM_CallHook, &MA_DeleteDeletedHook, FALSE);} break;
    case CMN_EMPTYSPAM: { DoMethod(G->App, MUIM_CallHook, &MA_DeleteSpamHook, FALSE);   } break;

    default:
    {
      return DoSuperMethodA(cl, obj, (Msg)msg);
    }
  }

  return 0;
}

///

/* Private Functions */

/* Public Methods */
