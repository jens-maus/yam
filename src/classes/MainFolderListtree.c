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

 Superclass:  MUIC_NListtree
 Description: NListtree class of the main folder list in the main window

***************************************************************************/

#include "MainFolderListtree_cl.h"

#include "YAM_find.h"
#include "YAM_mainFolder.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *context_menu;
  Object *folderImage[MAX_FOLDERIMG+1];

  BOOL draggingMails;
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
       CMN_EMPTYSPAM,
       CMN_ALLTOREAD,
       CMN_SEARCH };

/* Private Functions */
/// FormatFolderInfo
// puts all user defined folder information into a string
static void FormatFolderInfo(char *folderStr, const size_t maxLen, const struct Folder *folder)
{
  ENTER();

  // add the folder image first, if it exists
  if(folder->ImageIndex >= 0)
    snprintf(folderStr, maxLen, "\033o[%d] ", folder->ImageIndex);
  else
    strlcpy(folderStr, " ", maxLen);

  // include the folder name/path
  if(folder->Name[0] != '\0')
    strlcat(folderStr, folder->Name, maxLen);
  else
    snprintf(folderStr, maxLen, "%s[%s]", folderStr, FilePart(folder->Path));

  if(folder->LoadedMode != LM_UNLOAD && folder->LoadedMode != LM_REBUILD)
  {
    char dst[SIZE_SMALL];

    dst[0] = '\0';

    switch(C->FolderInfoMode)
    {
      case FIM_NAME_ONLY:
      {
        // nothing
      }
      break;

      case FIM_NAME_AND_NEW_MAILS:
      {
        if(folder->New != 0)
          snprintf(dst, sizeof(dst), " (%d)", folder->New);
      }
      break;

      case FIM_NAME_AND_UNREAD_MAILS:
      {
        if(folder->Unread != 0)
          snprintf(dst, sizeof(dst), " (%d)", folder->Unread);
      }
      break;

      case FIM_NAME_AND_NEW_UNREAD_MAILS:
      {
        if(folder->New != 0 || folder->Unread != 0)
          snprintf(dst, sizeof(dst), " (%d/%d)", folder->New, folder->Unread);
      }
      break;

      case FIM_NAME_AND_UNREAD_NEW_MAILS:
      {
        if(folder->New != 0 || folder->Unread != 0)
          snprintf(dst, sizeof(dst), " (%d/%d)", folder->Unread, folder->New);
      }
      break;
    }

    if(dst[0] != '\0')
      strlcat(folderStr, dst, maxLen);
  }

  LEAVE();
}
///

/* Hooks */
/// DisplayHook
// Folder listview display hook
HOOKPROTONHNO(DisplayFunc, ULONG, struct MUIP_NListtree_DisplayMessage *msg)
{
  ENTER();

  if(msg != NULL)
  {
    if(msg->TreeNode != NULL)
    {
      static char folderStr[SIZE_DEFAULT];
      static char totalStr[SIZE_SMALL];
      static char unreadStr[SIZE_SMALL];
      static char newStr[SIZE_SMALL];
      static char sizeStr[SIZE_SMALL];
      struct Folder *entry = (struct Folder *)msg->TreeNode->tn_User;

      folderStr[0] = '\0';
      totalStr[0] = '\0';
      unreadStr[0] = '\0';
      newStr[0] = '\0';
      sizeStr[0] = '\0';

      msg->Array[0] = folderStr;
      msg->Array[1] = totalStr;
      msg->Array[2] = unreadStr;
      msg->Array[3] = newStr;
      msg->Array[4] = sizeStr;

      switch(entry->Type)
      {
        case FT_GROUP:
        {
          snprintf(folderStr, sizeof(folderStr), "\033o[%d] %s", (isFlagSet(msg->TreeNode->tn_Flags, TNF_OPEN) ? FICON_ID_UNFOLD : FICON_ID_FOLD), entry->Name);
          msg->Preparse[0] = (entry->New != 0 || entry->Unread != 0) ? C->StyleFGroupUnread : C->StyleFGroupRead;
        }
        break;

        default:
        {
          FormatFolderInfo(folderStr, sizeof(folderStr), entry);

          if(entry->LoadedMode != LM_UNLOAD && entry->LoadedMode != LM_REBUILD)
          {
            if(entry->New != 0)
              msg->Preparse[0] = C->StyleFolderNew;
            else if(entry->Unread != 0)
              msg->Preparse[0] = C->StyleFolderUnread;
            else
              msg->Preparse[0] = C->StyleFolderRead;

            // if other folder columns are enabled lets fill the values in
            if(hasFColTotal(C->FolderCols))
              snprintf(totalStr, sizeof(totalStr), "%d", entry->Total);

            if(hasFColUnread(C->FolderCols) && entry->Unread != 0)
              snprintf(unreadStr, sizeof(unreadStr), "%d", entry->Unread);

            if(hasFColNew(C->FolderCols) && entry->New != 0)
              snprintf(newStr, sizeof(newStr), "%d", entry->New);

            if(hasFColSize(C->FolderCols) && entry->Size > 0)
              FormatSize(entry->Size, sizeStr, sizeof(sizeStr), SF_AUTO);
          }
          else
            msg->Preparse[0] = (char *)MUIX_I;

          if(isProtectedFolder(entry))
            snprintf(folderStr, sizeof(folderStr), "%s \033o[%d]", folderStr, FICON_ID_PROTECTED);
        }
      }
    }
    else
    {
      msg->Array[0] = (STRPTR)tr(MSG_Folder);
      msg->Array[1] = (STRPTR)tr(MSG_Total);
      msg->Array[2] = (STRPTR)tr(MSG_Unread);
      msg->Array[3] = (STRPTR)tr(MSG_New);
      msg->Array[4] = (STRPTR)tr(MSG_Size);
    }
  }

  LEAVE();
  return 0;
}
MakeStaticHook(DisplayHook, DisplayFunc);

///
/// ConstructHook
// Folder listview construction hook
HOOKPROTONHNO(ConstructFunc, struct Folder *, struct MUIP_NListtree_ConstructMessage *msg)
{
  struct Folder *entry;

  ENTER();

  entry = memdup(msg->UserData, sizeof(struct Folder));

  RETURN(entry);
  return entry;
}
MakeStaticHook(ConstructHook, ConstructFunc);

///
/// DestructHook
// Folder listtree destruction hook
HOOKPROTONHNO(DestructFunc, LONG, struct MUIP_NListtree_DestructMessage *msg)
{
  ENTER();

  FO_FreeFolder((struct Folder *)msg->UserData);

  RETURN(0);
  return 0;
}
MakeStaticHook(DestructHook, DestructFunc);

///
/// CompareHook
// Folder listtree compare hook
/*
HOOKPROTONH(CompareFunc, long, Object *obj, struct MUIP_NListtree_CompareMessage *ncm)
{
   struct Folder *entry1 = (struct Folder *)ncm->TreeNode1->tn_User;
   struct Folder *entry2 = (struct Folder *)ncm->TreeNode2->tn_User;
   int cmp = 0;

   if (ncm->SortType != MUIV_NList_SortType_None)
   {
      switch (ncm->SortType & MUIV_NList_TitleMark_ColMask)
      {
         case 0:  cmp = stricmp(entry1->Name, entry2->Name); break;
         case 1:  cmp = entry1->Total-entry2->Total; break;
         case 2:  cmp = entry1->Unread-entry2->Unread; break;
         case 3:  cmp = entry1->New-entry2->New; break;
         case 4:  cmp = entry1->Size-entry2->Size; break;
         case 10: return entry1->SortIndex-entry2->SortIndex;
      }
      if (ncm->SortType & MUIV_NList_TitleMark_TypeMask) cmp = -cmp;
   }

   return cmp;
}
MakeStaticHook(CompareHook, CompareFunc);
*/
///

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  struct Data *data;
  ULONG i;

  ENTER();

  if(!(obj = DoSuperNew(cl, obj,

    InputListFrame,
    MUIA_ObjectID,                    MAKE_ID('N','L','0','1'),
    MUIA_ContextMenu,                 C->FolderCntMenu ? MUIV_NList_ContextMenu_Always : 0,
    MUIA_Font,                        C->FixedFontList ? MUIV_NList_Font_Fixed : MUIV_NList_Font,
    MUIA_Dropable,                    TRUE,
    MUIA_NList_DragType,              MUIV_NList_DragType_Immediate,
    MUIA_NList_DragSortable,          TRUE,
    MUIA_NList_ActiveObjectOnClick,   TRUE,
    MUIA_NList_Exports,               MUIV_NList_Exports_ColWidth|MUIV_NList_Exports_ColOrder,
    MUIA_NList_Imports,               MUIV_NList_Imports_ColWidth|MUIV_NList_Imports_ColOrder,
    MUIA_NListtree_DisplayHook,       &DisplayHook,
    MUIA_NListtree_ConstructHook,     &ConstructHook,
    MUIA_NListtree_DestructHook,      &DestructHook,
    //MUIA_NListtree_CompareHook,       &MA_LV_FCmp2Hook,
    MUIA_NListtree_DragDropSort,      TRUE,
    MUIA_NListtree_Title,             TRUE,
    MUIA_NListtree_DoubleClick,       MUIV_NListtree_DoubleClick_All,

    TAG_MORE, inittags(msg))))
  {
    RETURN(0);
    return 0;
  }

  data = (struct Data *)INST_DATA(cl,obj);

  // prepare the folder images
  data->folderImage[FICON_ID_FOLD]        = MakeImageObject("folder_fold",         G->theme.folderImages[fi_Fold]);
  data->folderImage[FICON_ID_UNFOLD]      = MakeImageObject("folder_unfold",       G->theme.folderImages[fi_Unfold]);
  data->folderImage[FICON_ID_INCOMING]    = MakeImageObject("folder_incoming",     G->theme.folderImages[fi_Incoming]);
  data->folderImage[FICON_ID_INCOMING_NEW]= MakeImageObject("folder_incoming_new", G->theme.folderImages[fi_IncomingNew]);
  data->folderImage[FICON_ID_OUTGOING]    = MakeImageObject("folder_outgoing",     G->theme.folderImages[fi_Outgoing]);
  data->folderImage[FICON_ID_OUTGOING_NEW]= MakeImageObject("folder_outgoing_new", G->theme.folderImages[fi_OutgoingNew]);
  data->folderImage[FICON_ID_TRASH]       = MakeImageObject("folder_trash",        G->theme.folderImages[fi_Trash]);
  data->folderImage[FICON_ID_TRASH_NEW]   = MakeImageObject("folder_trash_new",    G->theme.folderImages[fi_TrashNew]);
  data->folderImage[FICON_ID_SENT]        = MakeImageObject("folder_sent",         G->theme.folderImages[fi_Sent]);
  data->folderImage[FICON_ID_PROTECTED]   = MakeImageObject("status_crypt",        G->theme.statusImages[si_Crypt]);
  data->folderImage[FICON_ID_SPAM]        = MakeImageObject("folder_spam",         G->theme.folderImages[fi_Spam]);
  data->folderImage[FICON_ID_SPAM_NEW]    = MakeImageObject("folder_spam_new",     G->theme.folderImages[fi_SpamNew]);
  for(i = 0; i < ARRAY_SIZE(data->folderImage); i++)
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
  } else {
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

  return DoSuperMethodA(cl, obj, msg);
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

    if((folder = (struct Folder *)tn->tn_User) != NULL)
    {
      if(data->draggingMails)
      {
        // if mails are being dragged the currently active folder and group folders must be excluded.
        // All other folders are valid drop targets.
        if(*dt->Pos == (LONG)xget(obj, MUIA_NListtree_Active) || isGroupFolder(folder))
          *dt->Type = MUIV_NListtree_DropType_None;
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
  BOOL disable_alltoread = FALSE;
  BOOL disable_search = FALSE;

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
        Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_Folder), MUIA_UserData, 1, MUIA_Menuitem_Enabled, FALSE, MUIA_Menuitem_Checked, TRUE, MUIA_Menuitem_Checkit, TRUE, MUIA_Menuitem_Toggle, TRUE, End,
        Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_Total),  MUIA_UserData, 2, MUIA_Menuitem_Checked, hasFColTotal(C->FolderCols), MUIA_Menuitem_Checkit, TRUE, MUIA_Menuitem_Toggle, TRUE, End,
        Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_Unread), MUIA_UserData, 3, MUIA_Menuitem_Checked, hasFColUnread(C->FolderCols), MUIA_Menuitem_Checkit, TRUE, MUIA_Menuitem_Toggle, TRUE, End,
        Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_New),    MUIA_UserData, 4, MUIA_Menuitem_Checked, hasFColNew(C->FolderCols), MUIA_Menuitem_Checkit, TRUE, MUIA_Menuitem_Toggle, TRUE, End,
        Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_Size),   MUIA_UserData, 5, MUIA_Menuitem_Checked, hasFColSize(C->FolderCols), MUIA_Menuitem_Checkit, TRUE, MUIA_Menuitem_Toggle, TRUE, End,
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
    disable_alltoread = TRUE;
    disable_search = TRUE;
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
    {
      disable_update = TRUE;
      disable_search = TRUE;
      disable_alltoread = TRUE;
    }

    if(isSentMailFolder(folder))
      disable_alltoread = TRUE;
  }

  // We create the ContextMenu now
  data->context_menu = MenustripObject,
    Child, MenuObjectT(folder ? FolderName(folder) : tr(MSG_FOLDER_NONSEL)),
      Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_FOLDER_NEWFOLDER),      MUIA_UserData, CMN_NEWF,   End,
      Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_FOLDER_NEWFOLDERGROUP), MUIA_UserData, CMN_NEWFG,  End,
      Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_FOLDER_EDIT),           MUIA_Menuitem_Enabled, !disable_edit,   MUIA_UserData, CMN_EDITF,   End,
      Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_FOLDER_DELETE),         MUIA_Menuitem_Enabled, !disable_delete, MUIA_UserData, CMN_DELETEF, End,
      Child, MenuitemObject, MUIA_Menuitem_Title, NM_BARLABEL, End,
      Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_MA_MSEARCH),            MUIA_Menuitem_Enabled, !disable_search, MUIA_UserData, CMN_SEARCH,  End,
      Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_MA_UPDATEINDEX),        MUIA_Menuitem_Enabled, !disable_update, MUIA_UserData, CMN_INDEX,   End,
      Child, MenuitemObject, MUIA_Menuitem_Title, NM_BARLABEL, End,
      Child, lastItem = MenuitemObject, MUIA_Menuitem_Title, tr(MSG_FOLDER_ALLTOREAD),      MUIA_Menuitem_Enabled, !disable_alltoread, MUIA_UserData, CMN_ALLTOREAD, End,
      Child, MenuitemObject, MUIA_Menuitem_Title, NM_BARLABEL, End,
      Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_FOLDER_SNAPSHOT_TREE),  MUIA_UserData, CMN_SNAPS,  End,
      Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_FOLDER_RELOAD_TREE), MUIA_UserData, CMN_RELOAD, End,
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
      DoMethod(data->context_menu, MUIM_Family_Insert, newItem, lastItem);
      lastItem = newItem;
    }

    // check if this is the SPAM folder
    if(C->SpamFilterEnabled &&
       (newItem = Menuitem(tr(MSG_MA_REMOVESPAM), NULL, !isGroupFolder(folder), FALSE, CMN_EMPTYSPAM)) != NULL)
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
      ULONG flag = (1 << (xget(m->item, MUIA_UserData)-1));

      if(isFlagSet(C->FolderCols, flag))
        CLEAR_FLAG(C->FolderCols, flag);
      else
        SET_FLAG(C->FolderCols, flag);

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
    case CMN_ALLTOREAD: { DoMethod(G->App, MUIM_CallHook, &MA_SetAllStatusToHook, SFLAG_READ, SFLAG_NEW); } break;
    case CMN_SEARCH:    { DoMethod(G->App, MUIM_CallHook, &FI_OpenHook); } break;

    default:
    {
      return DoSuperMethodA(cl, obj, (Msg)msg);
    }
  }

  return 0;
}

///

/* Public Methods */
