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

***************************************************************************/

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined(__AROS__)
#include <sys/types.h>
#else
#include <sys/filio.h>
#endif

#include <netinet/ip.h>
#include <netinet/tcp.h>

#include <clib/alib_protos.h>
#include <clib/macros.h>
#include <libraries/iffparse.h>
#include <libraries/genesis.h>
#include <mui/NList_mcc.h>
#include <mui/NListview_mcc.h>
#include <mui/NListtree_mcc.h>
#include <proto/amissl.h>
#if defined(__amigaos4__)
#include <proto/application.h>
#endif
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/muimaster.h>
#include <proto/timer.h>
#include <proto/utility.h>

#if !defined(__amigaos4__) && !defined(__AROS__)
#include <proto/miami.h>
#include <proto/genesis.h>
#endif

#include "extrasrc.h"

#include "SDI_hook.h"

#include "YAM.h"
#include "YAM_addressbookEntry.h"
#include "YAM_config.h"
#include "YAM_error.h"
#include "YAM_find.h"
#include "YAM_folderconfig.h"
#include "YAM_global.h"
#include "YAM_main.h"
#include "YAM_mainFolder.h"
#include "YAM_utilities.h"

#include "mui/Classes.h"
#include "mime/base64.h"
#include "mime/md5.h"

#include "AppIcon.h"
#include "HashTable.h"
#include "FileInfo.h"
#include "Locale.h"
#include "MailList.h"
#include "MailServers.h"
#include "MUIObjects.h"
#include "Requesters.h"
#include "Threads.h"
#include "UIDL.h"

#include "tcp/Connection.h"
#include "tcp/pop3.h"

#include "Debug.h"

/***************************************************************************
 Module: Transfer
***************************************************************************/

/*** General connecting/disconnecting & transfer ***/
/// TR_Disconnect
//  Terminates a connection
void TR_Disconnect(void)
{
  ENTER();

  D(DBF_NET, "disconnecting TCP/IP session...");

  DisconnectFromHost(G->TR->connection);

  LEAVE();
}
///
/// TR_SetWinTitle
//  Sets the title of the transfer window
void TR_SetWinTitle(BOOL from, const char *text)
{
  // compose the window title
  snprintf(G->TR->WTitle, sizeof(G->TR->WTitle), tr(from ? MSG_TR_MailTransferFrom : MSG_TR_MailTransferTo), text);

  // set the window title
  set(G->TR->GUI.WI, MUIA_Window_Title, G->TR->WTitle);
}
///
/// TR_ApplyRemoteFilters
//  Applies remote filters to a message
void TR_ApplyRemoteFilters(struct MailTransferNode *mtn)
{
  struct Node *curNode;

  ENTER();

  IterateList(G->TR->remoteFilters, curNode)
  {
    struct FilterNode *filter = (struct FilterNode *)curNode;

    if(DoFilterSearch(filter, mtn->mail) == TRUE)
    {
      if(hasExecuteAction(filter) && *filter->executeCmd)
         LaunchCommand(filter->executeCmd, FALSE, OUT_STDOUT);

      if(hasPlaySoundAction(filter) && *filter->playSound)
         PlaySound(filter->playSound);

      if(hasDeleteAction(filter))
         SET_FLAG(mtn->tflags, TRF_DELETE);
      else
         CLEAR_FLAG(mtn->tflags, TRF_DELETE);

      if(hasSkipMsgAction(filter))
         CLEAR_FLAG(mtn->tflags, TRF_TRANSFER);
      else
         SET_FLAG(mtn->tflags, TRF_TRANSFER);

      // get out of this loop after a successful search
      break;
    }
  }

  LEAVE();
}
///
/// TR_ChangeTransFlagsFunc
//  Changes transfer flags of all selected messages
HOOKPROTONHNO(TR_ChangeTransFlagsFunc, void, int *arg)
{
  int id = MUIV_NList_NextSelected_Start;

  do
  {
    struct MailTransferNode *mtn;

    DoMethod(G->TR->GUI.LV_MAILS, MUIM_NList_NextSelected, &id);
    if(id == MUIV_NList_NextSelected_End)
      break;

    DoMethod(G->TR->GUI.LV_MAILS, MUIM_NList_GetEntry, id, &mtn);
    mtn->tflags = *arg;

    DoMethod(G->TR->GUI.LV_MAILS, MUIM_NList_Redraw, id);
  }
  while(TRUE);
}
MakeStaticHook(TR_ChangeTransFlagsHook, TR_ChangeTransFlagsFunc);
///
/// TR_TransStat_Init
//  Initializes transfer statistics
void TR_TransStat_Init(void)
{
  struct Node *curNode;
  int numberOfMails = 0;
  ULONG totalSize = 0;

  ENTER();

  if(G->TR->GUI.GR_LIST != NULL)
  {
    set(G->TR->GUI.GR_PAGE, MUIA_Group_ActivePage, 1);
    DoMethod(G->TR->GUI.LV_MAILS, MUIM_NList_Select, MUIV_NList_Select_All, MUIV_NList_Select_Off, NULL);
  }

  // search through our transferList
  IterateList(&G->TR->transferList, curNode)
  {
    struct MailTransferNode *mtn = (struct MailTransferNode *)curNode;

    numberOfMails++;

    if(hasTR_TRANSFER(mtn))
      totalSize += mtn->mail->Size;
  }

  DoMethod(G->TR->GUI.GR_STATS, MUIM_TransferControlGroup_Start, numberOfMails, totalSize);

  LEAVE();
}
///
/// TR_Cleanup
//  Free temporary message and UIDL lists
void TR_Cleanup(void)
{
  struct Node *curNode;

  ENTER();

  if(G->TR->GUI.LV_MAILS != NULL)
    DoMethod(G->TR->GUI.LV_MAILS, MUIM_NList_Clear);

  while((curNode = RemHead((struct List *)&G->TR->transferList)) != NULL)
  {
    struct MailTransferNode *mtn = (struct MailTransferNode *)curNode;

    // free the mail pointer
    free(mtn->mail);

    // free the UIDL
    free(mtn->UIDL);

    // free the node itself
    free(mtn);
  }

  NewList((struct List *)&G->TR->transferList);

  LEAVE();
}
///
/// TR_AbortnClose
//  Aborts a transfer
void TR_AbortnClose(void)
{
  ENTER();

  set(G->TR->GUI.WI, MUIA_Window_Open, FALSE);
  TR_Cleanup();
  MA_ChangeTransfer(TRUE);

  DeleteMailList(G->TR->downloadedMails);
  G->TR->downloadedMails = NULL;

  DisposeModulePush(&G->TR);

  LEAVE();
}
///

/*** GET ***/
/// TR_AbortGETFunc
//  Aborts a POP3 download
HOOKPROTONHNONP(TR_AbortGETFunc, void)
{
  ENTER();

  // make sure the NOOP timer is definitly stopped
  StopTimer(TIMER_POP3_KEEPALIVE);

  // first set the Abort variable so that other can benefit from it
  set(G->TR->GUI.GR_STATS, MUIA_TransferControlGroup_Aborted, TRUE);

  // we can easily abort the transfer by setting the POP_Nr to -1
  // and issue a GetMailFromNextPOP command. With this solution YAM will
  // also process the filters for mails that were already downloaded
  // even if the user aborted the transfer somehow
  G->TR->POP_Nr = -1;
  TR_GetMailFromNextPOP(FALSE, -1, 0);

  LEAVE();
}
MakeStaticHook(TR_AbortGETHook, TR_AbortGETFunc);
///
/// TR_NewMailAlert
//  Notifies user when new mail is available
void TR_NewMailAlert(void)
{
  struct DownloadResult *stats = &G->TR->Stats;
  struct RuleResult *rr = &G->RuleResults;

  ENTER();

  SHOWVALUE(DBF_NET, stats->Downloaded);
  SHOWVALUE(DBF_NET, rr->Spam);

  // show the statistics only if we downloaded some mails at all,
  // and not all of them were spam mails
  if(stats->Downloaded > 0 && stats->Downloaded > rr->Spam)
  {
    if(hasRequesterNotify(C->NotifyType) && G->TR->GUIlevel != POP_REXX)
    {
      char buffer[SIZE_LARGE];

      // make sure the application isn't iconified
      if(xget(G->App, MUIA_Application_Iconified) == TRUE)
        PopUp();

      snprintf(buffer, sizeof(buffer), tr(MSG_TR_NewMailReq), stats->Downloaded, stats->OnServer-stats->Deleted, stats->DupSkipped);
      if(C->SpamFilterEnabled == TRUE)
      {
        // include the number of spam classified mails
        snprintf(&buffer[strlen(buffer)], sizeof(buffer)-strlen(buffer), tr(MSG_TR_FILTER_STATS_SPAM),
                                                                         rr->Checked,
                                                                         rr->Bounced,
                                                                         rr->Forwarded,
                                                                         rr->Replied,
                                                                         rr->Executed,
                                                                         rr->Moved,
                                                                         rr->Deleted,
                                                                         rr->Spam);
      }
      else
      {
        snprintf(&buffer[strlen(buffer)], sizeof(buffer)-strlen(buffer), tr(MSG_TR_FilterStats),
                                                                         rr->Checked,
                                                                         rr->Bounced,
                                                                         rr->Forwarded,
                                                                         rr->Replied,
                                                                         rr->Executed,
                                                                         rr->Moved,
                                                                         rr->Deleted);
      }

      // show the info window.
      InfoWindowObject,
        MUIA_Window_Title, tr(MSG_TR_NewMail),
        MUIA_Window_RefWindow, G->MA->GUI.WI,
        MUIA_Window_Activate, G->TR->GUIlevel == POP_USER,
        MUIA_InfoWindow_Body, buffer,
      End;
    }

    #if defined(__amigaos4__)
    if(hasOS41SystemNotify(C->NotifyType))
    {
      D(DBF_GUI, "appID is %ld, application.lib is V%ld.%ld (needed V%ld.%ld)", G->applicationID, ApplicationBase->lib_Version, ApplicationBase->lib_Revision, 53, 7);
      // Notify() is V53.2+, but 53.7 fixes some serious issues
      if(G->applicationID > 0 && LIB_VERSION_IS_AT_LEAST(ApplicationBase, 53, 7) == TRUE)
      {
        // 128 chars is the current maximum :(
        char imagePath[SIZE_PATHFILE];
        char message[128];
        int count = stats->Downloaded - rr->Spam;

        // distinguish between single and multiple mails
        if(count >= 2)
          snprintf(message, sizeof(message), tr(MSG_TR_NEW_MAIL_NOTIFY_MANY), count);
        else
          strlcpy(message, tr(MSG_TR_NEW_MAIL_NOTIFY_ONE), sizeof(message));

        AddPath(imagePath, G->ProgDir, "Themes/default/notify", sizeof(imagePath));

        // We require 53.7+. From this version on proper tag values are used, hence there
        // is no need to distinguish between v1 and v2 interfaces here as we have to do for
        // other application.lib functions.
        Notify(G->applicationID, APPNOTIFY_Title, (uint32)"YAM",
                                 APPNOTIFY_PubScreenName, (uint32)"FRONT",
                                 APPNOTIFY_Text, (uint32)message,
                                 APPNOTIFY_CloseOnDC, TRUE,
                                 APPNOTIFY_BackMsg, (uint32)"POPUP",
                                 APPNOTIFY_ImageFile, (uint32)imagePath,
                                 TAG_DONE);
      }
    }
    #endif // __amigaos4__

    if(hasCommandNotify(C->NotifyType))
      LaunchCommand(C->NotifyCommand, FALSE, OUT_STDOUT);

    if(hasSoundNotify(C->NotifyType))
      PlaySound(C->NotifySound);
  }

  LEAVE();
}

///
/// TR_ProcessGETFunc
/*** TR_ProcessGETFunc - Downloads messages from a POP3 server ***/
HOOKPROTONHNONP(TR_ProcessGETFunc, void)
{
  ENTER();

  // initialize the transfer statistics
  TR_TransStat_Init();

  // make sure the NOOP timer is definitly stopped
  StopTimer(TIMER_POP3_KEEPALIVE);

  if(xget(G->TR->GUI.GR_STATS, MUIA_TransferControlGroup_NumberOfMails) > 0)
  {
    struct Folder *infolder = FO_GetFolderByType(FT_INCOMING, NULL);
    struct Node *curNode;

    if(C->TransferWindow == TWM_SHOW && xget(G->TR->GUI.WI, MUIA_Window_Open) == FALSE)
      set(G->TR->GUI.WI, MUIA_Window_Open, TRUE);

    IterateList(&G->TR->transferList, curNode)
    {
      struct MailTransferNode *mtn = (struct MailTransferNode *)curNode;
      struct Mail *mail = mtn->mail;

      D(DBF_NET, "download flags %08lx=%s%s%s for mail with subject '%s' and size %ld",mtn->tflags, hasTR_TRANSFER(mtn) ? "TR_TRANSFER " : "" , hasTR_DELETE(mtn) ? "TR_DELETE " : "", hasTR_PRESELECT(mtn) ? "TR_PRESELECT " : "", mail->Subject, mail->Size);
      if(hasTR_TRANSFER(mtn))
      {
        D(DBF_NET, "downloading mail with subject '%s' and size %ld", mail->Subject, mail->Size);

        // update the transfer status
        DoMethod(G->TR->GUI.GR_STATS, MUIM_TransferControlGroup_Next, mtn->index, mtn->position, mail->Size, tr(MSG_TR_Downloading));

        if(TR_LoadMessage(infolder, mtn->index) == TRUE)
        {
          // redraw the folderentry in the listtree
          DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NListtree_Redraw, infolder->Treenode, MUIF_NONE);

          // put the transferStat for this mail to 100%
          DoMethod(G->TR->GUI.GR_STATS, MUIM_TransferControlGroup_Update, TCG_SETMAX, tr(MSG_TR_Downloading));

          G->TR->Stats.Downloaded++;

          // Remember the UIDL of this mail, no matter if it is going
          // to be deleted or not. Some servers don't delete a mail
          // right after the DELETE command, but only after a successful
          // QUIT command. Personal experience shows that pop.gmx.de is
          // one of these servers.
          if(C->AvoidDuplicates == TRUE)
          {
            D(DBF_NET, "adding mail with subject '%s' to UIDL hash", mail->Subject);
            // add the UIDL to the hash table or update an existing entry
            AddUIDLtoHash(G->TR->UIDLhashTable, mtn->UIDL, UIDLF_NEW);
          }

          if(hasTR_DELETE(mtn))
          {
            D(DBF_NET, "deleting mail with subject '%s' on server", mail->Subject);

            TR_DeleteMessage(mtn->index);
          }
          else
            D(DBF_NET, "leaving mail with subject '%s' and size %ld on server to be downloaded again", mail->Subject, mail->Size);
        }
      }
      else if(hasTR_DELETE(mtn))
      {
        D(DBF_NET, "deleting mail with subject '%s' on server", mail->Subject);

        // now we "know" that this mail had existed, don't forget this in case
        // the delete operation fails
        if(C->AvoidDuplicates == TRUE)
        {
          D(DBF_NET, "adding mail with subject '%s' to UIDL hash", mail->Subject);
          // add the UIDL to the hash table or update an existing entry
          AddUIDLtoHash(G->TR->UIDLhashTable, mtn->UIDL, UIDLF_NEW);
        }

        TR_DeleteMessage(mtn->index);
      }
      else
      {
        D(DBF_NET, "leaving mail with subject '%s' and size %ld on server to be downloaded again", mail->Subject, mail->Size);
        // Do not modify the UIDL hash here!
        // The mail was marked as "don't download", but here we don't know if that
        // is due to the duplicates checking or if the user did that himself.
      }

      if(xget(G->TR->GUI.GR_STATS, MUIA_TransferControlGroup_Aborted) == TRUE || G->TR->connection->error != CONNECTERR_NO_ERROR)
        break;
    }

    DoMethod(G->TR->GUI.GR_STATS, MUIM_TransferControlGroup_Finish);

    DisplayStatistics(infolder, TRUE);

    // in case the current folder after the download
    // is the incoming folder we have to update
    // the main toolbar and menu items
    if(FO_GetCurrentFolder() == infolder)
      MA_ChangeSelected(TRUE);
  }

  TR_GetMailFromNextPOP(FALSE, 0, 0);

  LEAVE();
}
MakeHook(TR_ProcessGETHook, TR_ProcessGETFunc);

///
/// TR_GetMessageInfoFunc
//  Requests message header of a message selected by the user
HOOKPROTONHNONP(TR_GetMessageInfoFunc, void)
{
  int line;
  struct MailTransferNode *mtn;

  ENTER();

  line = xget(G->TR->GUI.LV_MAILS, MUIA_NList_Active);
  DoMethod(G->TR->GUI.LV_MAILS, MUIM_NList_GetEntry, line, &mtn);
  TR_GetMessageDetails(mtn, line);

  LEAVE();
}
MakeStaticHook(TR_GetMessageInfoHook, TR_GetMessageInfoFunc);
///
/// TR_CompleteMsgList
//  Gets details for messages on server
void TR_CompleteMsgList(void)
{
  struct TR_ClassData *tr = G->TR;

  ENTER();

  // first we have to set the notifies to the default values.
  // this is needed so that if we get mail from more than one POP3 at a line this
  // abort stuff works out
  set(tr->GUI.BT_PAUSE, MUIA_Disabled, FALSE);
  set(tr->GUI.BT_RESUME, MUIA_Disabled, TRUE);
  DoMethod(tr->GUI.BT_START, MUIM_KillNotify, MUIA_Pressed);
  DoMethod(tr->GUI.BT_START, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_WriteLong, TRUE, &(tr->Start));
  DoMethod(tr->GUI.BT_QUIT , MUIM_KillNotify, MUIA_Pressed);
  DoMethod(tr->GUI.BT_QUIT , MUIM_Notify, MUIA_Pressed, FALSE, tr->GUI.GR_STATS, 3, MUIM_Set, MUIA_TransferControlGroup_Aborted, TRUE);

  if(C->PreSelection < PSM_ALWAYSLARGE)
  {
    struct Node *curNode = (struct Node *)tr->GMD_Mail;

    while(curNode != NULL && xget(tr->GUI.GR_STATS, MUIA_TransferControlGroup_Aborted) == FALSE && tr->connection->error == CONNECTERR_NO_ERROR)
    {
      struct MailTransferNode *mtn = (struct MailTransferNode *)curNode;

      if(tr->Pause == TRUE)
        break;

      if(tr->Start == TRUE)
      {
        TR_ProcessGETFunc();
        break;
      }

      if(C->PreSelection != PSM_LARGE || mtn->mail->Size >= C->WarnSize*1024)
      {
        TR_GetMessageDetails(mtn, tr->GMD_Line++);

        // set the next mail as the active one for the display,
        // so that if the user pauses we can go on here
        tr->GMD_Mail = (struct MinNode *)curNode;
      }

      curNode = GetSucc(curNode);
    }
  }

  set(tr->GUI.BT_PAUSE, MUIA_Disabled, TRUE);
  DoMethod(tr->GUI.BT_START, MUIM_KillNotify, MUIA_Pressed);
  DoMethod(tr->GUI.BT_START, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 2, MUIM_CallHook, &TR_ProcessGETHook);
  DoMethod(tr->GUI.BT_QUIT , MUIM_KillNotify, MUIA_Pressed);
  DoMethod(tr->GUI.BT_QUIT , MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 2, MUIM_CallHook, &TR_AbortGETHook);

  if(xget(G->TR->GUI.GR_STATS, MUIA_TransferControlGroup_Aborted) == TRUE)
    TR_AbortGETFunc();
  else
  {
    // start the timer which makes sure we send
    // a regular NOOP command to the POP3 server
    // so that it doesn't drop the connection
    RestartTimer(TIMER_POP3_KEEPALIVE, C->KeepAliveInterval, 0);
  }

  LEAVE();
}
///
/// TR_PauseFunc
//  Pauses or resumes message download
HOOKPROTONHNO(TR_PauseFunc, void, int *arg)
{
  BOOL pause = *arg;

  ENTER();

  set(G->TR->GUI.BT_RESUME, MUIA_Disabled, pause == FALSE);
  set(G->TR->GUI.BT_PAUSE,  MUIA_Disabled, pause == TRUE);

  if(pause == TRUE)
  {
    // start the timer which makes sure we send
    // a regular NOOP command to the POP3 server
    // so that it doesn't drop the connection
    RestartTimer(TIMER_POP3_KEEPALIVE, C->KeepAliveInterval, 0);
  }
  else
  {
    G->TR->Pause = FALSE;
    TR_CompleteMsgList();
  }

  LEAVE();
}
MakeStaticHook(TR_PauseHook, TR_PauseFunc);
///

/*** GUI ***/
/// TR_New
//  Creates transfer window
struct TR_ClassData *TR_New(enum TransferType TRmode)
{
  struct TR_ClassData *data;

  ENTER();

  if((data = calloc(1, sizeof(struct TR_ClassData))) != NULL)
  {
    if((data->downloadedMails = CreateMailList()) != NULL)
    {
      Object *bt_all = NULL, *bt_none = NULL, *bt_loadonly = NULL, *bt_loaddel = NULL, *bt_delonly = NULL, *bt_leave = NULL;
      Object *gr_sel, *gr_proc, *gr_win;
      BOOL fullwin = (TRmode == TR_GET_USER || TRmode == TR_GET_AUTO || TRmode == TR_IMPORT);

      NewList((struct List *)&data->transferList);

      gr_proc = TransferControlGroupObject, End;

      if(fullwin == TRUE)
      {
        data->GUI.GR_LIST = VGroup, GroupFrameT(tr(MSG_TR_MsgOnServer)),
           MUIA_ShowMe, C->PreSelection >= PSM_ALWAYS,
           Child, NListviewObject,
              MUIA_CycleChain, TRUE,
              MUIA_NListview_NList, data->GUI.LV_MAILS = TransferMailListObject,
              End,
           End,
        End;
        gr_sel = VGroup, GroupFrameT(tr(MSG_TR_Control)),
           Child, ColGroup(5),
              Child, bt_all = MakeButton(tr(MSG_TR_All)),
              Child, bt_loaddel = MakeButton(tr(MSG_TR_DownloadDelete)),
              Child, bt_leave = MakeButton(tr(MSG_TR_Leave)),
              Child, HSpace(0),
              Child, data->GUI.BT_PAUSE = MakeButton(tr(MSG_TR_Pause)),
              Child, bt_none = MakeButton(tr(MSG_TR_Clear)),
              Child, bt_loadonly = MakeButton(tr(MSG_TR_DownloadOnly)),
              Child, bt_delonly = MakeButton(tr(MSG_TR_DeleteOnly)),
              Child, HSpace(0),
              Child, data->GUI.BT_RESUME = MakeButton(tr(MSG_TR_Resume)),
           End,
           Child, ColGroup(2),
              Child, data->GUI.BT_START = MakeButton(tr(MSG_TR_Start)),
              Child, data->GUI.BT_QUIT = MakeButton(tr(MSG_TR_Abort)),
           End,
        End;
        gr_win = VGroup,
           Child, data->GUI.GR_LIST,
           Child, data->GUI.GR_PAGE = PageGroup,
              Child, gr_sel,
              Child, gr_proc,
           End,
        End;
      }
      else
      {
        gr_win = VGroup, MUIA_Frame, MUIV_Frame_None,
           Child, gr_proc,
        End;
      }

      data->GUI.WI = WindowObject,
         MUIA_Window_ID, MAKE_ID('T','R','A','0'+TRmode),
         MUIA_Window_CloseGadget, FALSE,
         MUIA_Window_Activate, (TRmode == TR_GET_USER || TRmode == TR_SEND_USER),
         MUIA_HelpNode, "TR_W",
         WindowContents, gr_win,
      End;

      if(data->GUI.WI != NULL)
      {
        data->GUI.GR_STATS = gr_proc;

        DoMethod(G->App, OM_ADDMEMBER, data->GUI.WI);
        set(gr_proc, MUIA_TransferControlGroup_PreselectionList, data->GUI.GR_LIST);

        if(fullwin == TRUE)
        {
          set(data->GUI.WI, MUIA_Window_DefaultObject, data->GUI.LV_MAILS);
          set(data->GUI.BT_RESUME, MUIA_Disabled, TRUE);

          if(TRmode != TR_IMPORT)
          {
            set(data->GUI.GR_PAGE, MUIA_Group_ActivePage, 1);
            DoMethod(data->GUI.BT_RESUME,MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_CallHook, &TR_PauseHook, FALSE);
            DoMethod(data->GUI.BT_PAUSE ,MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_CallHook, &TR_PauseHook, TRUE);
            DoMethod(data->GUI.BT_PAUSE, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_WriteLong, TRUE, &(data->Pause));
            DoMethod(data->GUI.LV_MAILS ,MUIM_Notify, MUIA_NList_DoubleClick,TRUE, MUIV_Notify_Application, 2, MUIM_CallHook, &TR_GetMessageInfoHook);
            DoMethod(bt_delonly,         MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_CallHook, &TR_ChangeTransFlagsHook, TRF_DELETE);
            DoMethod(bt_loaddel,         MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_CallHook, &TR_ChangeTransFlagsHook, (TRF_TRANSFER|TRF_DELETE));
            DoMethod(data->GUI.BT_START, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_WriteLong, TRUE, &(data->Start));
            DoMethod(data->GUI.BT_QUIT , MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_WriteLong, TRUE, &(data->Abort));
          }
          DoMethod(bt_loadonly,        MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_CallHook, &TR_ChangeTransFlagsHook, TRF_TRANSFER);
          DoMethod(bt_leave,           MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_CallHook, &TR_ChangeTransFlagsHook, TRF_NONE);
          DoMethod(bt_all,             MUIM_Notify, MUIA_Pressed, FALSE, data->GUI.LV_MAILS, 4, MUIM_NList_Select, MUIV_NList_Select_All, MUIV_NList_Select_On, NULL);
          DoMethod(bt_none,            MUIM_Notify, MUIA_Pressed, FALSE, data->GUI.LV_MAILS, 4, MUIM_NList_Select, MUIV_NList_Select_All, MUIV_NList_Select_Off, NULL);
        }
        MA_ChangeTransfer(FALSE);
      }
      else
      {
        DeleteMailList(data->downloadedMails);
        free(data);
        data = NULL;
      }
    }
    else
    {
      free(data);
      data = NULL;
    }
  }

  RETURN(data);
  return data;
}

///
