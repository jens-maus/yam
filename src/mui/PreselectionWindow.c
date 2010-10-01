/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2010 by YAM Open Source Team

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

#include "Locale.h"
#include "MailImport.h"
#include "MUIObjects.h"
#include "Threads.h"

#include "YAM_mainFolder.h"
#include "YAM_transfer.h"

#include "tcp/Connection.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *transferMailList;
  Object *allButton;
  Object *noneButton;
  Object *leaveButton;
  Object *downloadDeleteButton;
  Object *downloadOnlyButton;
  Object *deleteOnlyButton;
  Object *pauseButton;
  Object *resumeButton;
  Object *startButton;
  Object *quitButton;

  ULONG mailCount;

  APTR thread;
  struct MinList *preselectList;

  ULONG accept;
};
*/

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  struct TagItem *tag;
  const char *titleText = "YAM";
  Object *transferMailList;
  Object *allButton;
  Object *noneButton;
  Object *leaveButton;
  Object *downloadDeleteButton;
  Object *downloadOnlyButton;
  Object *deleteOnlyButton;
  Object *pauseButton;
  Object *resumeButton;
  Object *startButton;
  Object *quitButton;

  ENTER();

  if((tag = FindTagItem(MUIA_Window_Title, inittags(msg))) != NULL)
  {
    if((const char *)tag->ti_Data != NULL)
    {
      titleText = (const char *)tag->ti_Data;
    }
    tag->ti_Tag = TAG_IGNORE;
  }

  if((obj = DoSuperNew(cl, obj,

    MUIA_Window_Title,       titleText,
    MUIA_Window_ID,          MAKE_ID('P','R','E','S'),
    MUIA_Window_CloseGadget, FALSE,
    WindowContents, VGroup,
      Child, NListviewObject,
        MUIA_CycleChain, TRUE,
        MUIA_NListview_NList, transferMailList = TransferMailListObject,
        End,
      End,
      Child, VGroup, GroupFrameT(tr(MSG_TR_Control)),
        Child, ColGroup(5),
          Child, allButton = MakeButton(tr(MSG_TR_All)),
          Child, downloadDeleteButton = MakeButton(tr(MSG_TR_DownloadDelete)),
          Child, leaveButton = MakeButton(tr(MSG_TR_Leave)),
          Child, HSpace(0),
          Child, pauseButton = MakeButton(tr(MSG_TR_Pause)),
          Child, noneButton= MakeButton(tr(MSG_TR_Clear)),
          Child, downloadOnlyButton = MakeButton(tr(MSG_TR_DownloadOnly)),
          Child, deleteOnlyButton = MakeButton(tr(MSG_TR_DeleteOnly)),
          Child, HSpace(0),
          Child, resumeButton = MakeButton(tr(MSG_TR_Resume)),
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

    data->transferMailList = transferMailList;
    data->allButton = allButton;
    data->noneButton = noneButton;
    data->leaveButton = leaveButton;
    data->downloadDeleteButton = downloadDeleteButton;
    data->downloadOnlyButton = downloadOnlyButton;
    data->deleteOnlyButton = deleteOnlyButton;
    data->pauseButton = pauseButton;
    data->resumeButton = resumeButton;
    data->startButton = startButton;
    data->quitButton = quitButton;

    data->thread = (struct Thread *)GetTagData(ATTR(Thread), (IPTR)NULL, inittags(msg));
    data->preselectList = (struct MinList *)GetTagData(ATTR(Mails), (IPTR)NULL, inittags(msg));

    if(data->preselectList != NULL)
    {
      struct Node *node;
      ULONG position = 0;

      set(data->transferMailList, MUIA_NList_Quiet, TRUE);

      IterateList(data->preselectList, node)
      {
        struct MailTransferNode *mtn = (struct MailTransferNode *)node;

        mtn->position = position++;

        DoMethod(data->transferMailList, MUIM_NList_InsertSingle, mtn, MUIV_NList_Insert_Bottom);
      }

      xset(data->transferMailList, MUIA_NList_Active, MUIV_NList_Active_Top,
                                   MUIA_NList_Quiet, FALSE);
    }

    set(data->pauseButton, MUIA_Disabled, TRUE);
    set(data->resumeButton, MUIA_Disabled, TRUE);
    set(data->deleteOnlyButton, MUIA_Disabled, TRUE);
    set(data->downloadDeleteButton, MUIA_Disabled, TRUE);

    DoMethod(data->startButton, MUIM_Notify, MUIA_Pressed, FALSE, obj, 2, METHOD(Accept), TRUE);
    DoMethod(data->quitButton, MUIM_Notify, MUIA_Pressed, FALSE, obj, 2, METHOD(Accept), FALSE);
    DoMethod(data->resumeButton, MUIM_Notify, MUIA_Pressed, FALSE, obj, 1, METHOD(Resume));
    DoMethod(data->pauseButton, MUIM_Notify, MUIA_Pressed, FALSE, obj, 1, METHOD(Pause));
    DoMethod(data->transferMailList, MUIM_Notify, MUIA_NList_DoubleClick, TRUE, obj, 1, METHOD(GetMessageDetails));
    DoMethod(data->deleteOnlyButton, MUIM_Notify, MUIA_Pressed, FALSE, obj, 2, METHOD(ChangeFlags), TRF_DELETE);
    DoMethod(data->downloadDeleteButton, MUIM_Notify, MUIA_Pressed, FALSE, obj, 2, METHOD(ChangeFlags), TRF_TRANSFER|TRF_DELETE);
    DoMethod(data->downloadOnlyButton, MUIM_Notify, MUIA_Pressed, FALSE, obj, 2, METHOD(ChangeFlags), TRF_TRANSFER);
    DoMethod(data->leaveButton, MUIM_Notify, MUIA_Pressed, FALSE, obj, 2, METHOD(ChangeFlags), TRF_NONE);
    DoMethod(data->allButton, MUIM_Notify, MUIA_Pressed, FALSE, data->transferMailList, 4, MUIM_NList_Select, MUIV_NList_Select_All, MUIV_NList_Select_On, NULL);
    DoMethod(data->noneButton, MUIM_Notify, MUIA_Pressed, FALSE, data->transferMailList, 4, MUIM_NList_Select, MUIV_NList_Select_All, MUIV_NList_Select_Off, NULL);

    DoMethod(G->App, OM_ADDMEMBER, obj);
  }

  RETURN((IPTR)obj);
  return (IPTR)obj;
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

/* Private Functions */

/* Public Methods */
/// DECLARE(Accept)
DECLARE(Accept) // ULONG accept
{
  GETDATA;

  data->accept = msg->accept;
  WakeupThread(data->thread);

  return 0;
}

///
/// DECLARE(Resume)
DECLARE(Resume)
{
  return 0;
}

///
/// DECLARE(Pause)
DECLARE(Pause)
{
  return 0;
}

///
/// DECLARE(ChangeFlags)
DECLARE(ChangeFlags) // ULONG flags
{
  GETDATA;
  LONG id = MUIV_NList_NextSelected_Start;

  do
  {
    struct MailTransferNode *mtn;

    DoMethod(data->transferMailList, MUIM_NList_NextSelected, &id);
    if(id == MUIV_NList_NextSelected_End)
      break;

    DoMethod(data->transferMailList, MUIM_NList_GetEntry, id, &mtn);

    mtn->tflags = msg->flags;

    DoMethod(data->transferMailList, MUIM_NList_Redraw, id);
  }
  while(TRUE);

  return 0;
}

///
/// DECLARE(GetMessageDetails)
DECLARE(GetMessageDetails)
{
  return 0;
}

///
