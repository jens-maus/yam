/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2012 YAM Open Source Team

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
 Description: NListtree class for displaying a folder listtree

***************************************************************************/

#include "FolderListtree_cl.h"

#include <string.h>
#include <proto/dos.h>
#include <proto/muimaster.h>
#include <libraries/gadtools.h>
#include <libraries/iffparse.h>
#include <mui/NList_mcc.h>
#include <mui/NListtree_mcc.h>

#include "YAM.h"
#include "YAM_config.h"
#include "YAM_find.h"
#include "YAM_mainFolder.h"

#include "FolderList.h"
#include "Locale.h"
#include "MUIObjects.h"
#include "Themes.h"

#include "mui/ImageArea.h"

#include "Debug.h"

/* INCLUDE
#include "Themes.h"
*/

/* CLASSDATA
struct Data
{
  Object *folderImage[FI_MAX];
  char folderStr[SIZE_DEFAULT];
  char totalStr[SIZE_SMALL];
  char unreadStr[SIZE_SMALL];
  char newStr[SIZE_SMALL];
  char sizeStr[SIZE_SMALL];
};
*/

/* Private Functions */
/// FormatFolderInfo
// puts all user defined folder information into a string
static void FormatFolderInfo(char *folderStr, const size_t maxLen,
                             const struct Folder *folder, const struct MUI_NListtree_TreeNode *treeNode)
{
  int imageIndex = -1;

  ENTER();

  // add the folder image
  if(folder->Type == FT_GROUP)
    imageIndex = isFlagSet(treeNode->tn_Flags, TNF_OPEN) ? FICON_ID_UNFOLD : FICON_ID_FOLD;
  else
    imageIndex = folder->ImageIndex >= 0 ? folder->ImageIndex : FICON_ID_FOLD;

  snprintf(folderStr, maxLen, "\033o[%d]", imageIndex);

  // include the folder name/path
  if(folder->Name[0] != '\0')
    strlcat(folderStr, folder->Name, maxLen);
  else
    snprintf(folderStr, maxLen, "%s[%s]", folderStr, FilePart(folder->Path));

  // append the numbers if this is an close folder group or a folder with a valid index
  if((folder->Type == FT_GROUP && isFlagClear(treeNode->tn_Flags, TNF_OPEN)) ||
     (folder->LoadedMode != LM_UNLOAD && folder->LoadedMode != LM_REBUILD))
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

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  ENTER();

  if((obj = DoSuperNew(cl, obj,

    InputListFrame,
    MUIA_ContextMenu,                 0,
    MUIA_ShortHelp,                   TRUE,
    MUIA_Font,                        C->FixedFontList ? MUIV_NList_Font_Fixed : MUIV_NList_Font,
    MUIA_Dropable,                    FALSE,
    MUIA_NList_ActiveObjectOnClick,   TRUE,
    MUIA_NList_DefaultObjectOnClick,  FALSE,
    MUIA_NList_Exports,               MUIV_NList_Exports_ColWidth|MUIV_NList_Exports_ColOrder,
    MUIA_NList_Imports,               MUIV_NList_Imports_ColWidth|MUIV_NList_Imports_ColOrder,
    MUIA_NListtree_DragDropSort,      FALSE,
    MUIA_NListtree_Title,             TRUE,
    MUIA_NListtree_DoubleClick,       MUIV_NListtree_DoubleClick_All,

    TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;
    ULONG i;

    // prepare the folder images
    data->folderImage[FICON_ID_FOLD]        = MakeImageObject("folder_fold",         G->theme.folderImages[FI_FOLD]);
    data->folderImage[FICON_ID_UNFOLD]      = MakeImageObject("folder_unfold",       G->theme.folderImages[FI_UNFOLD]);
    data->folderImage[FICON_ID_INCOMING]    = MakeImageObject("folder_incoming",     G->theme.folderImages[FI_INCOMING]);
    data->folderImage[FICON_ID_INCOMING_NEW]= MakeImageObject("folder_incoming_new", G->theme.folderImages[FI_INCOMINGNEW]);
    data->folderImage[FICON_ID_OUTGOING]    = MakeImageObject("folder_outgoing",     G->theme.folderImages[FI_OUTGOING]);
    data->folderImage[FICON_ID_OUTGOING_NEW]= MakeImageObject("folder_outgoing_new", G->theme.folderImages[FI_OUTGOINGNEW]);
    data->folderImage[FICON_ID_TRASH]       = MakeImageObject("folder_trash",        G->theme.folderImages[FI_TRASH]);
    data->folderImage[FICON_ID_TRASH_NEW]   = MakeImageObject("folder_trash_new",    G->theme.folderImages[FI_TRASHNEW]);
    data->folderImage[FICON_ID_SENT]        = MakeImageObject("folder_sent",         G->theme.folderImages[FI_SENT]);
    data->folderImage[FICON_ID_PROTECTED]   = MakeImageObject("status_crypt",        G->theme.statusImages[SI_CRYPT]);
    data->folderImage[FICON_ID_SPAM]        = MakeImageObject("folder_spam",         G->theme.folderImages[FI_SPAM]);
    data->folderImage[FICON_ID_SPAM_NEW]    = MakeImageObject("folder_spam_new",     G->theme.folderImages[FI_SPAMNEW]);
    data->folderImage[FICON_ID_DRAFTS]      = MakeImageObject("folder_drafts",       G->theme.folderImages[FI_DRAFTS]);
    data->folderImage[FICON_ID_DRAFTS_NEW]  = MakeImageObject("folder_drafts_new",   G->theme.folderImages[FI_DRAFTSNEW]);
    for(i = 0; i < ARRAY_SIZE(data->folderImage); i++)
      DoMethod(obj, MUIM_NList_UseImage, data->folderImage[i], i, MUIF_NONE);
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
  ULONG i;

  ENTER();

  for(i=0; i < ARRAY_SIZE(data->folderImage); i++)
  {
    DoMethod(obj, MUIM_NList_UseImage, NULL, i, MUIF_NONE);
    if(data->folderImage[i] != NULL)
    {
      MUI_DisposeObject(data->folderImage[i]);
      data->folderImage[i] = NULL;
    }
  }

  // dispose ourself
  result = DoSuperMethodA(cl, obj, msg);

  RETURN(result);
  return result;
}

///
/// OVERLOAD(OM_GET)
// get some stuff of our instance data
OVERLOAD(OM_GET)
{
  GETDATA;
  IPTR *store = ((struct opGet *)msg)->opg_Storage;

  switch(((struct opGet *)msg)->opg_AttrID)
  {
    case ATTR(ImageArray): *store = (ULONG)data->folderImage; return TRUE;
  }

  return DoSuperMethodA(cl, obj, msg);
}

///
/// OVERLOAD(MUIM_CreateShortHelp)
// set up a text for the bubble help
OVERLOAD(MUIM_CreateShortHelp)
{
  struct MUIP_CreateShortHelp *csh = (struct MUIP_CreateShortHelp *)msg;
  struct MUI_NListtree_TestPos_Result res;
  char *shortHelp = NULL;

  ENTER();

  DoMethod(obj, MUIM_NListtree_TestPos, csh->mx, csh->my, &res);
  if(res.tpr_TreeNode != NULL)
  {
    struct Folder *folder = ((struct FolderNode *)res.tpr_TreeNode->tn_User)->folder;

    if(folder != NULL && !isGroupFolder(folder))
    {
      char sizestr[SIZE_DEFAULT];

      FormatSize(folder->Size, sizestr, sizeof(sizestr), SF_AUTO);

      if(asprintf(&shortHelp, tr(MSG_FOLDERINFO_SHORTHELP), folder->Name,
                                                            folder->Path,
                                                            sizestr,
                                                            folder->Total,
                                                            folder->New,
                                                            folder->Unread) == -1)
      {
        shortHelp = NULL;
      }
    }
  }

  RETURN(shortHelp);
  return (IPTR)shortHelp;
}

///
/// OVERLOAD(MUIM_DeleteShortHelp)
// free the bubble help text
OVERLOAD(MUIM_DeleteShortHelp)
{
  struct MUIP_DeleteShortHelp *dsh = (struct MUIP_DeleteShortHelp *)msg;

  ENTER();

  free(dsh->help);

  LEAVE();
  return 0;
}

///
/// OVERLOAD(MUIM_NListtree_Display)
OVERLOAD(MUIM_NListtree_Display)
{
  struct MUIP_NListtree_Display *ndm = (struct MUIP_NListtree_Display *)msg;

  ENTER();

  if(ndm->TreeNode != NULL)
  {
    GETDATA;
    struct FolderNode *fnode = (struct FolderNode *)ndm->TreeNode->tn_User;
    struct Folder *entry = fnode->folder;

    data->folderStr[0] = '\0';
    data->totalStr[0] = '\0';
    data->unreadStr[0] = '\0';
    data->newStr[0] = '\0';
    data->sizeStr[0] = '\0';

    ndm->Array[0] = data->folderStr;
    ndm->Array[1] = data->totalStr;
    ndm->Array[2] = data->unreadStr;
    ndm->Array[3] = data->newStr;
    ndm->Array[4] = data->sizeStr;

    // create folderStr
    FormatFolderInfo(data->folderStr, sizeof(data->folderStr), entry, ndm->TreeNode);

    switch(entry->Type)
    {
      case FT_GROUP:
      {
        ndm->Preparse[0] = (entry->New != 0 || entry->Unread != 0) ? C->StyleFGroupUnread : C->StyleFGroupRead;

        // show group stats for closed nodes only
        if(isFlagClear(ndm->TreeNode->tn_Flags, TNF_OPEN))
        {
          // if other folder columns are enabled lets fill the values in
          if(hasFColTotal(C->FolderCols))
            snprintf(data->totalStr, sizeof(data->totalStr), "%d", entry->Total);

          if(hasFColUnread(C->FolderCols) && entry->Unread != 0)
            snprintf(data->unreadStr, sizeof(data->unreadStr), "%d", entry->Unread);

          if(hasFColNew(C->FolderCols) && entry->New != 0)
            snprintf(data->newStr, sizeof(data->newStr), "%d", entry->New);

          if(hasFColSize(C->FolderCols) && entry->Size > 0)
            FormatSize(entry->Size, data->sizeStr, sizeof(data->sizeStr), SF_AUTO);
        }
      }
      break;

      default:
      {
        if(entry->LoadedMode != LM_UNLOAD && entry->LoadedMode != LM_REBUILD)
        {
          if(entry->New != 0)
            ndm->Preparse[0] = C->StyleFolderNew;
          else if(entry->Unread != 0)
            ndm->Preparse[0] = C->StyleFolderUnread;
          else
            ndm->Preparse[0] = C->StyleFolderRead;

          // if other folder columns are enabled lets fill the values in
          if(hasFColTotal(C->FolderCols))
            snprintf(data->totalStr, sizeof(data->totalStr), "%d", entry->Total);

          if(hasFColUnread(C->FolderCols) && entry->Unread != 0)
            snprintf(data->unreadStr, sizeof(data->unreadStr), "%d", entry->Unread);

          if(hasFColNew(C->FolderCols) && entry->New != 0)
            snprintf(data->newStr, sizeof(data->newStr), "%d", entry->New);

          if(hasFColSize(C->FolderCols) && entry->Size > 0)
            FormatSize(entry->Size, data->sizeStr, sizeof(data->sizeStr), SF_AUTO);
        }
        else
          ndm->Preparse[0] = (char *)MUIX_I;

        if(isProtectedFolder(entry))
          snprintf(data->folderStr, sizeof(data->folderStr), "%s \033o[%d]", data->folderStr, FICON_ID_PROTECTED);
      }
    }
  }
  else
  {
    ndm->Array[0] = (STRPTR)tr(MSG_Folder);
    ndm->Array[1] = (STRPTR)tr(MSG_Total);
    ndm->Array[2] = (STRPTR)tr(MSG_Unread);
    ndm->Array[3] = (STRPTR)tr(MSG_New);
    ndm->Array[4] = (STRPTR)tr(MSG_Size);
  }

  LEAVE();
  return 0;
}

///

/* Public Methods */
