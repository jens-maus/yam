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

 Superclass:  MUIC_Window
 Description: Container window for a preselection list

***************************************************************************/

#include "PreselectionWindow_cl.h"

#include <string.h>
#include <proto/muimaster.h>
#include <libraries/iffparse.h>
#include <mui/NList_mcc.h>
#include <mui/NListview_mcc.h>

#include "YAM.h"
#include "YAM_mainFolder.h"

#include "Locale.h"
#include "MailImport.h"
#include "MailTransferList.h"
#include "MUIObjects.h"
#include "Threads.h"

#include "mui/Base64Dataspace.h"
#include "mui/TransferMailList.h"
#include "tcp/Connection.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *transferGroup;
  Object *transferMailList;
  Object *progressBar;
  Object *allButton;
  Object *noneButton;
  Object *leaveButton;
  Object *downloadDeleteButton;
  Object *downloadOnlyButton;
  Object *deleteOnlyButton;
  Object *startButton;
  Object *quitButton;

  ULONG mailCount;
  ULONG accept;

  APTR thread;
  struct MailTransferList *mailList;

  char progressText[SIZE_DEFAULT];
  char screenTitle[SIZE_DEFAULT];
};
*/

/* EXPORT
enum PreselectionWindowMode
{
  PRESELWINMODE_IMPORT=0,
  PRESELWINMODE_DOWNLOAD,
};
*/

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  struct TagItem *tags = inittags(msg), *tag;
  char *titleText = NULL;
  Object *transferGroup;
  Object *transferMailList;
  Object *allButton;
  Object *noneButton;
  Object *leaveButton;
  Object *downloadDeleteButton;
  Object *downloadOnlyButton;
  Object *deleteOnlyButton;
  Object *startButton;
  Object *quitButton;

  ENTER();

  while((tag = NextTagItem((APTR)&tags)) != NULL)
  {
    switch(tag->ti_Tag)
    {
      case MUIA_Window_Title:
      {
        titleText = (char *)tag->ti_Data;
        tag->ti_Tag = TAG_IGNORE;
      }
      break;
    }
  }

  if((obj = DoSuperNew(cl, obj,

    MUIA_Window_ID,          MAKE_ID('P','R','E','S'),
    MUIA_Window_CloseGadget, TRUE,
    WindowContents, VGroup,
      Child, transferGroup = VGroup,
        Child, NListviewObject,
          MUIA_CycleChain, TRUE,
          MUIA_NListview_NList, transferMailList = TransferMailListObject,
            MUIA_TransferMailList_SizeLimit, GetTagData(ATTR(SizeLimit), 0, inittags(msg)),
          End,
        End,
      End,
      Child, VGroup, GroupFrameT(tr(MSG_TR_Control)),
        Child, ColGroup(3),
          Child, allButton = MakeButton(tr(MSG_TR_All)),
          Child, downloadDeleteButton = MakeButton(tr(MSG_TR_DownloadDelete)),
          Child, leaveButton = MakeButton(tr(MSG_TR_Leave)),

          Child, noneButton = MakeButton(tr(MSG_TR_Clear)),
          Child, downloadOnlyButton = MakeButton(tr(MSG_TR_DownloadOnly)),
          Child, deleteOnlyButton = MakeButton(tr(MSG_TR_DeleteOnly)),
        End,
        Child, ColGroup(2),
          Child, startButton = MakeButton(tr(MSG_TR_Start)),
          Child, quitButton = MakeButton(tr(MSG_TR_Abort)),
        End,
      End,
    End,

    TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;
    Object *ds;
    enum PreselectionWindowMode mode;

    DoMethod(G->App, OM_ADDMEMBER, obj);

    data->transferGroup = transferGroup;
    data->transferMailList = transferMailList;
    data->allButton = allButton;
    data->noneButton = noneButton;
    data->leaveButton = leaveButton;
    data->downloadDeleteButton = downloadDeleteButton;
    data->downloadOnlyButton = downloadOnlyButton;
    data->deleteOnlyButton = deleteOnlyButton;
    data->startButton = startButton;
    data->quitButton = quitButton;

    data->thread = (APTR)GetTagData(ATTR(Thread), (IPTR)NULL, inittags(msg));
    data->mailList = (struct MailTransferList *)GetTagData(ATTR(Mails), (IPTR)NULL, inittags(msg));

    xset(obj, MUIA_Window_Title, titleText != NULL ? titleText : "YAM",
              MUIA_Window_ScreenTitle, CreateScreenTitle(data->screenTitle, sizeof(data->screenTitle), titleText));

    // try to restore any previously remembered NList layout
    if((ds = Base64DataspaceObject,
      MUIA_Base64Dataspace_Base64String, G->preselectionListLayout,
    End) != NULL)
    {
      DoMethod(data->transferMailList, MUIM_Import, ds);
      MUI_DisposeObject(ds);
    }

    if(data->mailList != NULL)
    {
      struct MailTransferNode *tnode;
      ULONG position = 0;

      set(data->transferMailList, MUIA_NList_Quiet, TRUE);

      LockMailTransferList(data->mailList);

      ForEachMailTransferNode(data->mailList, tnode)
      {
        tnode->position = position++;

        DoMethod(data->transferMailList, MUIM_NList_InsertSingle, tnode, MUIV_NList_Insert_Bottom);
      }

      UnlockMailTransferList(data->mailList);

      xset(data->transferMailList, MUIA_NList_Active, MUIV_NList_Active_Top,
                                   MUIA_NList_Quiet, FALSE);
    }

    mode = GetTagData(ATTR(Mode), PRESELWINMODE_IMPORT, inittags(msg));
    set(data->deleteOnlyButton, MUIA_Disabled, mode == PRESELWINMODE_IMPORT);
    set(data->downloadDeleteButton, MUIA_Disabled, mode == PRESELWINMODE_IMPORT);

    DoMethod(obj, MUIM_Notify, MUIA_Window_CloseRequest, TRUE, MUIV_Notify_Self, 2, METHOD(Accept), FALSE);
    DoMethod(data->startButton, MUIM_Notify, MUIA_Pressed, FALSE, obj, 2, METHOD(Accept), TRUE);
    DoMethod(data->quitButton, MUIM_Notify, MUIA_Pressed, FALSE, obj, 2, METHOD(Accept), FALSE);
    DoMethod(data->deleteOnlyButton, MUIM_Notify, MUIA_Pressed, FALSE, obj, 2, METHOD(ChangeFlags), TRF_DELETE);
    DoMethod(data->downloadDeleteButton, MUIM_Notify, MUIA_Pressed, FALSE, obj, 2, METHOD(ChangeFlags), TRF_TRANSFER|TRF_DELETE);
    DoMethod(data->downloadOnlyButton, MUIM_Notify, MUIA_Pressed, FALSE, obj, 2, METHOD(ChangeFlags), TRF_TRANSFER);
    DoMethod(data->leaveButton, MUIM_Notify, MUIA_Pressed, FALSE, obj, 2, METHOD(ChangeFlags), TRF_NONE);
    DoMethod(data->allButton, MUIM_Notify, MUIA_Pressed, FALSE, data->transferMailList, 4, MUIM_NList_Select, MUIV_NList_Select_All, MUIV_NList_Select_On, NULL);
    DoMethod(data->noneButton, MUIM_Notify, MUIA_Pressed, FALSE, data->transferMailList, 4, MUIM_NList_Select, MUIV_NList_Select_All, MUIV_NList_Select_Off, NULL);
  }

  RETURN((IPTR)obj);
  return (IPTR)obj;
}

///
/// OVERLOAD(OM_SET)
OVERLOAD(OM_SET)
{
  GETDATA;
  struct TagItem *tags = inittags(msg);
  struct TagItem *tag;

  while((tag = NextTagItem((APTR)&tags)) != NULL)
  {
    switch(tag->ti_Tag)
    {
      case ATTR(ActiveMail):
      {
        struct MailTransferNode *tnode = (struct MailTransferNode *)tag->ti_Data;

        if(tnode != NULL)
          DoMethod(data->transferMailList, MUIM_NList_SetActive, tnode->index-1, MUIV_NList_SetActive_Jump_Center);
      }
      break;

      case ATTR(Progress):
      {
        if(tag->ti_Data == 0)
        {
          // remove and dispose the progress bar
          if(data->progressBar != NULL)
          {
            if(DoMethod(data->transferGroup, MUIM_Group_InitChange))
            {
              DoMethod(data->transferGroup, OM_REMMEMBER, data->progressBar);
              DoMethod(data->transferGroup, MUIM_Group_ExitChange);
            }

            MUI_DisposeObject(data->progressBar);
            data->progressBar = NULL;
          }
        }
        else
        {
          // create the progress bar if it doesn't exist yet
          if(data->progressBar == NULL)
          {
            snprintf(data->progressText, sizeof(data->progressText), tr(MSG_PRESELECT_GETTING_DETAILS), data->mailList->count);
            if((data->progressBar = GaugeObject,
              GaugeFrame,
              MUIA_Gauge_InfoText, data->progressText,
              MUIA_Gauge_Horiz, TRUE,
              MUIA_Gauge_Max, data->mailList->count,
            End) != NULL)
            {
              if(DoMethod(data->transferGroup, MUIM_Group_InitChange))
              {
                DoMethod(data->transferGroup, OM_ADDMEMBER, data->progressBar);
                DoMethod(data->transferGroup, MUIM_Group_ExitChange);
              }
            }
          }

          if(data->progressBar != NULL)
            set(data->progressBar, MUIA_Gauge_Current, tag->ti_Data);
        }
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
  GETDATA;
  IPTR *store = ((struct opGet *)msg)->opg_Storage;

  switch(((struct opGet *)msg)->opg_AttrID)
  {
    case ATTR(Result):
    {
      *store = (IPTR)data->accept;

      return TRUE;
    }
    break;
  }

  return DoSuperMethodA(cl, obj, msg);
}

///
/// OVERLOAD(MUIM_Window_Snapshot)
OVERLOAD(MUIM_Window_Snapshot)
{
  GETDATA;
  struct MUIP_Window_Snapshot *snap = (struct MUIP_Window_Snapshot *)msg;
  BOOL setDefault = TRUE;

  // remember the columns for snapshot operations, but not for unsnapshot operations
  if(snap->flags != 0)
  {
    Object *ds;

    // remember the current NList layout whenever the window is closed
    if((ds = Base64DataspaceObject, End) != NULL)
    {
      char *b64;

      // let NList export its stuff to our subclassed Dataspace.mui object
      DoMethod(data->transferMailList, MUIM_Export, ds);
      // then get the base64 encoded string and remember it
      if((b64 = (char *)xget(ds, MUIA_Base64Dataspace_Base64String)) != NULL)
      {
        strlcpy(G->preselectionListLayout, b64, sizeof(G->preselectionListLayout));
        setDefault = FALSE;
      }

      MUI_DisposeObject(ds);
    }
  }
  else
  {
    // unsnapshot, set the defaults
  }

  if(setDefault == TRUE)
  {
    // set the default layout, which means that NList will calculate the column sizes automatically
    strlcpy(G->preselectionListLayout, EMPTY_B64DSPACE_STRING, sizeof(G->preselectionListLayout));
  }

  // make sure the layout is saved
  SaveLayout(TRUE);

  return DoSuperMethodA(cl, obj, msg);
}

///

/* Private Functions */

/* Public Methods */
/// DECLARE(Accept)
DECLARE(Accept) // ULONG accept
{
  GETDATA;

  data->accept = msg->accept;

  // wake up a possibly sleeping thread
  if(data->thread != NULL)
    WakeupThread(data->thread);

  return 0;
}

///
/// DECLARE(ChangeFlags)
DECLARE(ChangeFlags) // ULONG flags
{
  GETDATA;
  LONG id = MUIV_NList_NextSelected_Start;

  LockMailTransferList(data->mailList);

  do
  {
    struct MailTransferNode *tnode;

    DoMethod(data->transferMailList, MUIM_NList_NextSelected, &id);
    if(id == MUIV_NList_NextSelected_End)
      break;

    DoMethod(data->transferMailList, MUIM_NList_GetEntry, id, &tnode);

    tnode->tflags = msg->flags;

    DoMethod(data->transferMailList, MUIM_NList_Redraw, id);
  }
  while(TRUE);

  UnlockMailTransferList(data->mailList);

  return 0;
}

///
/// DECLARE(RefreshMail)
DECLARE(RefreshMail) // const LONG line
{
  GETDATA;

  DoMethod(data->transferMailList, MUIM_NList_Redraw, msg->line);

  return 0;
}

///
