/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2022 YAM Open Source Team

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

 Superclass:  MUIC_NList
 Description: NList class of the write window's attachment list

***************************************************************************/

#include "WriteAttachmentList_cl.h"

#include <string.h>

#include <proto/muimaster.h>
#include <mui/NList_mcc.h>

#include "YAM.h"
#include "YAM_mainFolder.h"

#include "mui/AttachmentImage.h"
#include "mui/MainMailListGroup.h"
#include "mui/WriteWindow.h"

#include "Busy.h"
#include "Config.h"
#include "Locale.h"
#include "MUIObjects.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *contextMenu;
  Object *syncList;
  char sizeBuffer[SIZE_SMALL];
  BOOL tiny;
  BOOL syncing;
};
*/

enum
{
  CMN_ADD=10,
  CMN_ADDPACK,
  CMN_REMOVE,
  CMN_RENAME,
  CMN_DISPLAY,
};

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  BOOL tiny;

  ENTER();

  tiny = (BOOL)GetTagData(ATTR(Tiny), FALSE, inittags(msg));

  if((obj = DoSuperNew(cl, obj,

    InputListFrame,
    MUIA_NList_ActiveObjectOnClick,  TRUE,
    MUIA_NList_DefaultObjectOnClick, FALSE,
    MUIA_NList_DragType,             MUIV_NList_DragType_Immediate,
    MUIA_NList_DragSortable,         TRUE,
    MUIA_NList_Format,               (tiny == FALSE) ? "D=8 BAR,P=\033r D=8 BAR,D=8 BAR," : "PCS=R,P=\033r",
    MUIA_NList_Title,                TRUE,
    MUIA_ContextMenu,                MUIV_NList_ContextMenu_Always,

    TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;

    data->tiny = tiny;
  }

  RETURN((IPTR)obj);
  return (IPTR)obj;
}

///
/// OVERLOAD(OM_DISPOSE)
OVERLOAD(OM_DISPOSE)
{
  GETDATA;

  if(data->contextMenu != NULL)
    MUI_DisposeObject(data->contextMenu);

  return DoSuperMethodA(cl, obj, msg);
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
      case ATTR(SyncList):
      {
        data->syncList = (Object *)tag->ti_Data;
        tag->ti_Tag = TAG_IGNORE;
      }
      break;
    }
  }

  return DoSuperMethodA(cl, obj, msg);
}

///
/// OVERLOAD(MUIM_DragQuery)
OVERLOAD(MUIM_DragQuery)
{
  struct MUIP_DragQuery *d = (struct MUIP_DragQuery *)msg;
  IPTR result;

  ENTER();

  // the attachmentlist either accepts drag requests from the main mail list
  // or from a readmailgroup object
  if(DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_MainMailListGroup_IsMailList, d->obj) == TRUE)
    result = MUIV_DragQuery_Accept;
  else if(xget(d->obj, MUIA_AttachmentImage_MailPart) != 0)
    result = MUIV_DragQuery_Accept;
  else
    result = DoSuperMethodA(cl, obj, msg);

  RETURN(result);
  return(result);
}

///
/// OVERLOAD(MUIM_DragDrop)
OVERLOAD(MUIM_DragDrop)
{
  struct MUIP_DragDrop *d = (struct MUIP_DragDrop *)msg;
  IPTR result;

  ENTER();

  if(DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_MainMailListGroup_IsMailList, d->obj) == TRUE)
  {
    int id = MUIV_NList_NextSelected_Start;

    do
    {
      struct Mail *mail = NULL;

      DoMethod(d->obj, MUIM_NList_NextSelected, &id);
      if(id == MUIV_NList_NextSelected_End)
        break;

      DoMethod(d->obj, MUIM_NList_GetEntry, id, &mail);
      // add the attachment to our attachment list
      DoMethod(_win(obj), MUIM_WriteWindow_AddMailAttachment, mail);
    }
    while(TRUE);

    result = 0;
  }
  else if(xget(d->obj, MUIA_AttachmentImage_MailPart) != 0)
  {
    struct BusyNode *busy;
    char tempFile[SIZE_FILE];
    struct Attach attach;
    struct Part *mailPart = (struct Part *)xget(d->obj, MUIA_AttachmentImage_MailPart);

    busy = BusyBegin(BUSY_TEXT);
    BusyText(busy, tr(MSG_BusyDecSaving), "");

    // make sure the mail part is properly decoded before we add it
    RE_DecodePart(mailPart);

    // clear the attachment structure
    memset(&attach, 0, sizeof(struct Attach));

    // then we create a copy of our decoded file which we can use
    // independent from the object we got the mailPart from
    snprintf(tempFile, sizeof(tempFile), "YAMt%08x.tmp", (unsigned int)mailPart);
    AddPath(attach.FilePath, C->TempDir, tempFile, sizeof(attach.FilePath));

    // copy the file now
    if(CopyFile(attach.FilePath, NULL, mailPart->Filename, NULL))
    {
      strlcpy(attach.Description, mailPart->Description, sizeof(attach.Description));
      strlcpy(attach.ContentType, mailPart->ContentType, sizeof(attach.ContentType));
      attach.Size = mailPart->Size;
      attach.IsTemp = TRUE;

      if(mailPart->Name != NULL)
        strlcpy(attach.Name, mailPart->Name, sizeof(attach.Name));

      // add the attachment to our attachment list
      DoMethod(_win(obj), MUIM_WriteWindow_InsertAttachment, &attach);
    }
    else
      DisplayBeep(_screen(obj));

    BusyEnd(busy);
    result = 0;
  }
  else
  {
    // let the superclass decide
    result = DoSuperMethodA(cl, obj, msg);
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

  ENTER();

  // dispose the old context_menu if it still exists
  if(data->contextMenu)
  {
    MUI_DisposeObject(data->contextMenu);
    data->contextMenu = NULL;
  }

  if(!m->ontop)
  {
    struct Attach *attach = NULL;

    DoMethod(obj, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &attach);

    // We create the ContextMenu now
    data->contextMenu = MenustripObject,
      Child, MenuObjectT(tr(MSG_WR_CMENU_TITLE)),
        Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_WR_CMENU_ADD_ATTACHMENT),         MUIA_Menuitem_CopyStrings, FALSE, MUIA_UserData, CMN_ADD,     End,
        Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_WR_CMENU_ADD_ARCHIVE_ATTACHMENT), MUIA_Menuitem_CopyStrings, FALSE, MUIA_UserData, CMN_ADDPACK, End,
        Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_WR_CMENU_REMOVE_ATTACHMENT),      MUIA_Menuitem_CopyStrings, FALSE, MUIA_UserData, CMN_REMOVE,  MUIA_Menuitem_Enabled, attach != NULL, End,
        Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_WR_CMENU_RENAME_ATTACHMENT),      MUIA_Menuitem_CopyStrings, FALSE, MUIA_UserData, CMN_RENAME,  MUIA_Menuitem_Enabled, attach != NULL, End,
        Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_WR_CMENU_DISPLAY_ATTACHMENT),     MUIA_Menuitem_CopyStrings, FALSE, MUIA_UserData, CMN_DISPLAY, MUIA_Menuitem_Enabled, attach != NULL, End,
      End,
    End;
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
    case CMN_ADD:
      DoMethod(_win(obj), MUIM_WriteWindow_RequestAttachment, C->AttachDir);
    break;

    case CMN_ADDPACK:
      DoMethod(_win(obj), MUIM_WriteWindow_AddArchive);
    break;

    case CMN_REMOVE:
      DoMethod(_win(obj), MUIM_WriteWindow_RemoveAttachment);
    break;

    case CMN_RENAME:
      DoMethod(_win(obj), MUIM_WriteWindow_RenameAttachment);
    break;

    case CMN_DISPLAY:
      DoMethod(_win(obj), MUIM_WriteWindow_DisplayAttachment, obj);
    break;

    default:
      result = DoSuperMethodA(cl, obj, (Msg)msg);
    break;
  }

  RETURN(result);
  return result;
}

///
/// OVERLOAD(MUIM_NList_Construct)
OVERLOAD(MUIM_NList_Construct)
{
  struct MUIP_NList_Construct *ncm = (struct MUIP_NList_Construct *)msg;
  struct Attach *attach = ncm->entry;
  struct Attach *entry;

  ENTER();

  entry = memdup(attach, sizeof(*attach));

  RETURN((IPTR)entry);
  return (IPTR)entry;
}

///
/// OVERLOAD(MUIM_NList_Destruct)
OVERLOAD(MUIM_NList_Destruct)
{
  struct MUIP_NList_Destruct *ncm = (struct MUIP_NList_Destruct *)msg;
  struct Attach *attach = ncm->entry;

  ENTER();

  FinishUnpack(attach->FilePath);
  free(attach);

  RETURN(0);
  return 0;
}

///
/// OVERLOAD(MUIM_NList_Display)
OVERLOAD(MUIM_NList_Display)
{
  struct MUIP_NList_Display *ndm = (struct MUIP_NList_Display *)msg;
  struct Attach *entry = (struct Attach *)ndm->entry;
  GETDATA;

  ENTER();

  if(entry != NULL)
  {
    FormatSize(entry->Size, data->sizeBuffer, sizeof(data->sizeBuffer), SF_AUTO);
    ndm->strings[0] = entry->Name;
    ndm->strings[1] = data->sizeBuffer;
    if(data->tiny == FALSE)
    {
      ndm->strings[2] = (STRPTR)DescribeCT(entry->ContentType);
      ndm->strings[3] = entry->Description;
    }
  }
  else
  {
    if(data->tiny == TRUE)
    {
      ndm->strings[0] = (STRPTR)tr(MSG_WR_TITLEATTACHMENT);
      ndm->strings[1] = (STRPTR)tr(MSG_WR_TitleSize);
    }
    else
    {
      ndm->strings[0] = (STRPTR)tr(MSG_WR_TitleFile);
      ndm->strings[1] = (STRPTR)tr(MSG_WR_TitleSize);
      ndm->strings[2] = (STRPTR)tr(MSG_WR_TitleContents);
      ndm->strings[3] = (STRPTR)tr(MSG_WR_TitleDescription);
    }
  }

  RETURN(0);
  return 0;
}

///
/// OVERLOAD(MUIM_NList_Insert)
OVERLOAD(MUIM_NList_Insert)
{
  ULONG rc;
  GETDATA;

  ENTER();

  rc = DoSuperMethodA(cl, obj, msg);
  if(rc != (ULONG)NULL && data->syncing == FALSE && data->syncList != NULL)
    DoMethod(data->syncList, METHOD(Sync), obj);

  RETURN(rc);
  return rc;
}

///
/// OVERLOAD(MUIM_NList_InsertSingle)
OVERLOAD(MUIM_NList_InsertSingle)
{
  ULONG rc;
  GETDATA;

  ENTER();

  rc = DoSuperMethodA(cl, obj, msg);
  if(rc != (ULONG)NULL && data->syncing == FALSE && data->syncList != NULL)
    DoMethod(data->syncList, METHOD(Sync), obj);

  RETURN(rc);
  return rc;
}

///
/// OVERLOAD(MUIM_NList_Remove)
OVERLOAD(MUIM_NList_Remove)
{
  ULONG rc;
  GETDATA;

  ENTER();

  rc = DoSuperMethodA(cl, obj, msg);
  if(rc != FALSE && data->syncing == FALSE && data->syncList != NULL)
    DoMethod(data->syncList, METHOD(Sync), obj);

  RETURN(rc);
  return rc;
}

///
/// OVERLOAD(MUIM_NList_Move)
OVERLOAD(MUIM_NList_Move)
{
  ULONG rc;
  GETDATA;

  ENTER();

  rc = DoSuperMethodA(cl, obj, msg);
  if(rc != FALSE && data->syncing == FALSE && data->syncList != NULL)
    DoMethod(data->syncList, METHOD(Sync), obj);

  RETURN(rc);
  return rc;
}

///

/* Private Functions */

/* Public Methods */
/// DECLARE(Sync)
// get ourselves in sync with the sync list object
DECLARE(Sync) // Object *source
{
  GETDATA;
  ULONG i;
  struct Attach *attach;

  ENTER();

  data->syncing = TRUE;
  set(obj, MUIA_NList_Quiet, TRUE);

  DoMethod(obj, MUIM_NList_Clear);
  i = 0;
  do
  {
    if((attach = (struct Attach *)DoMethod(msg->source, MUIM_NList_GetEntry, i, NULL)) != NULL)
    {
      DoMethod(obj, MUIM_NList_InsertSingle, attach, MUIV_NList_Insert_Bottom);
      i++;
    }
  }
  while(attach != NULL);

  set(obj, MUIA_NList_Quiet, FALSE);
  data->syncing = FALSE;

  RETURN(0);
  return 0;
}

///
