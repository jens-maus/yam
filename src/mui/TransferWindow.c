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
 Description: Window for displaying a mail transfer (POP3, SMTP, etc.)

***************************************************************************/

#include "TransferWindow_cl.h"

#include "YAM_addressbookEntry.h"
#include "YAM_mainFolder.h"
#include "YAM_utilities.h"

#include "MailList.h"
#include "MUIObjects.h"
#include "Requesters.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *GR_LIST;
  Object *GR_PAGE;
  Object *GR_SEL;
  Object *GR_PROC;
  Object *LV_MAILS;
  Object *BT_ALL;
  Object *BT_LOADDEL;
  Object *BT_LEAVE;
  Object *BT_NONE;
  Object *BT_LOADONLY;
  Object *BT_DELONLY;
  Object *BT_PAUSE;
  Object *BT_RESUME;
  Object *BT_QUIT;
  Object *BT_START;
  Object *TX_STATS;
  Object *TX_STATUS;
  Object *GA_COUNT;
  Object *GA_BYTES;
  Object *BT_ABORT;

  struct MinNode       *GMD_Mail;
  struct Folder *       ImportFolder;
  struct HashTable *    UIDLhashTable;      // for maintaining all UIDLs
  struct MailList *     downloadedMails;
  struct TransferNode * activeTransfer;     // the currently active Transfer

  long                  Abort;
  long                  Pause;
  long                  Start;
  int                   GMD_Line;
  enum ImportFormat     ImportFormat;
  enum TransferType     transferType;       // the type of the transfer (IMPORT/EXPORT, SEND/RECEIVE)
  BOOL                  SinglePOP;
  BOOL                  DuplicatesChecking; // true if we check for duplicate mail downloads
  BOOL                  UIDLhashIsDirty;
  struct DownloadResult Stats;
  struct TransStat      transferStat;       // transferStatistics

  char                  windowTitle[SIZE_DEFAULT];
  char                  ImportFile[SIZE_PATHFILE];

  char status_label[SIZE_DEFAULT];
  char size_gauge_label[SIZE_DEFAULT];
  char msg_gauge_label[SIZE_DEFAULT];
};
*/

/* Private Functions */

/* Hooks */
/// DisplayHook
//  Message listview display hook
HOOKPROTONH(DisplayFunc, long, char **array, struct MailTransferNode *entry)
{
  ENTER();

  if(entry != NULL)
  {
    static char dispfro[SIZE_DEFAULT];
    static char dispsta[SIZE_DEFAULT];
    static char dispsiz[SIZE_SMALL];
    static char dispdate[64];
    struct Mail *mail = entry->mail;
    struct Person *pe = &mail->From;

    array[0] = dispsta;

    // status icon display
    snprintf(dispsta, sizeof(dispsta), "%3d ", entry->index);
    if(hasTR_LOAD(entry))
      strlcat(dispsta, SI_STR(si_Download), sizeof(dispsta));
    if(hasTR_DELETE(entry))
      strlcat(dispsta, SI_STR(si_Delete), sizeof(dispsta));

    // size display
    array[1] = dispsiz;
    if(C->WarnSize > 0 && mail->Size >= (C->WarnSize*1024))
    {
      strlcpy(dispsiz, MUIX_PH, sizeof(dispsiz));
      FormatSize(mail->Size, dispsiz+strlen(dispsiz), sizeof(dispsiz)-strlen(dispsiz), SF_AUTO);
    }
    else
      FormatSize(mail->Size, dispsiz, sizeof(dispsiz), SF_AUTO);

    // from address display
    array[2] = dispfro;
    strlcpy(dispfro, AddrName((*pe)), sizeof(dispfro));

    // mail subject display
    array[3] = mail->Subject;

    // display date
    array[4] = dispdate;
    *dispdate = '\0';

    if(mail->Date.ds_Days != 0)
      DateStamp2String(dispdate, sizeof(dispdate), &mail->Date, (C->DSListFormat == DSS_DATEBEAT || C->DSListFormat == DSS_RELDATEBEAT) ? DSS_DATEBEAT : DSS_DATETIME, TZC_LOCAL);
  }
  else
  {
    array[0] = (STRPTR)tr(MSG_MA_TitleStatus);
    array[1] = (STRPTR)tr(MSG_Size);
    array[2] = (STRPTR)tr(MSG_From);
    array[3] = (STRPTR)tr(MSG_Subject);
    array[4] = (STRPTR)tr(MSG_Date);
  }

  LEAVE();
  return 0;
}
MakeStaticHook(DisplayHook, DisplayFunc);

///

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  struct Data *data;
  struct Data *tmpData;

  ENTER();

  // generate a temporarly struct Data to which we store our data and
  // copy it later on
  if((data = tmpData = calloc(1, sizeof(struct Data))) == NULL)
  {
    RETURN(0);
    return 0;
  }

  // prepare the initial text object contents
  FormatSize(0, data->transferStat.str_size_done, sizeof(data->transferStat.str_size_done), SF_MIXED);
  FormatSize(0, data->transferStat.str_size_tot, sizeof(data->transferStat.str_size_tot), SF_MIXED);
  FormatSize(0, data->transferStat.str_speed, sizeof(data->transferStat.str_speed), SF_MIXED);
  snprintf(data->status_label, sizeof(data->status_label), tr(MSG_TR_TRANSFERSTATUS),
                               data->transferStat.str_size_done, data->transferStat.str_size_tot, data->transferStat.str_speed, 0, 0, 0, 0);

  snprintf(data->msg_gauge_label, sizeof(data->msg_gauge_label), tr(MSG_TR_MESSAGEGAUGE), 0);

  FormatSize(0, data->transferStat.str_size_curr, sizeof(data->transferStat.str_size_curr), SF_AUTO);
  FormatSize(0, data->transferStat.str_size_curr_max, sizeof(data->transferStat.str_size_curr_max), SF_AUTO);
  snprintf(data->size_gauge_label, sizeof(data->size_gauge_label), tr(MSG_TR_TRANSFERSIZE),
                                          data->transferStat.str_size_curr, data->transferStat.str_size_curr_max);

  // create the mail list for storing the downloaded mails
  data->downloadedMails = CreateMailList();

  if(data->downloadedMails != NULL && (obj = DoSuperNew(cl, obj,

    MUIA_Window_Title,  "",
    MUIA_HelpNode,      "TR_W",
    MUIA_Window_ID,     MAKE_ID('T','R','A','0'),
    MUIA_Window_CloseGadget, FALSE,
    WindowContents, VGroup,

      Child, data->GR_LIST = VGroup,
        GroupFrameT(tr(MSG_TR_MsgOnServer)),
        Child, NListviewObject,
          MUIA_CycleChain,  TRUE,
          MUIA_ContextMenu, NULL,
          MUIA_NListview_NList,         data->LV_MAILS = TransferMailListObject,
            MUIA_ObjectID,              MAKE_ID('N','L','0','4'),
            MUIA_Font,                  C->FixedFontList ? MUIV_NList_Font_Fixed : MUIV_NList_Font,
            MUIA_NList_MultiSelect,     MUIV_NList_MultiSelect_Default,
            MUIA_NList_Format,          "W=-1 BAR,W=-1 MACW=9 P=\33r BAR,MICW=20 BAR,MICW=16 BAR,MICW=9 MACW=15",
            MUIA_NList_DisplayHook,     &DisplayHook,
            MUIA_NList_AutoVisible,     TRUE,
            MUIA_NList_Title,           TRUE,
            MUIA_NList_TitleSeparator,  TRUE,
            MUIA_NList_DoubleClick,     TRUE,
            MUIA_NList_MinColSortable,  0,
            MUIA_NList_Exports,         MUIV_NList_Exports_Cols,
            MUIA_NList_Imports,         MUIV_NList_Imports_Cols,
          End,
        End,
      End,

      Child, data->GR_PAGE = PageGroup,

        Child, data->GR_SEL = VGroup,
          GroupFrameT(tr(MSG_TR_Control)),
          Child, ColGroup(5),
            Child, data->BT_ALL = MakeButton(tr(MSG_TR_All)),
            Child, data->BT_LOADDEL = MakeButton(tr(MSG_TR_DownloadDelete)),
            Child, data->BT_LEAVE = MakeButton(tr(MSG_TR_Leave)),
            Child, HSpace(0),
            Child, data->BT_PAUSE = MakeButton(tr(MSG_TR_Pause)),
            Child, data->BT_NONE = MakeButton(tr(MSG_TR_Clear)),
            Child, data->BT_LOADONLY = MakeButton(tr(MSG_TR_DownloadOnly)),
            Child, data->BT_DELONLY = MakeButton(tr(MSG_TR_DeleteOnly)),
            Child, HSpace(0),
            Child, data->BT_RESUME = MakeButton(tr(MSG_TR_Resume)),
          End,
          Child, ColGroup(2),
            Child, data->BT_START = MakeButton(tr(MSG_TR_Start)),
            Child, data->BT_QUIT = MakeButton(tr(MSG_TR_Abort)),
          End,
        End,

        Child, data->GR_PROC = ColGroup(2),
          GroupFrameT(tr(MSG_TR_Status)),
          Child, data->TX_STATS = TextObject,
            MUIA_Text_Contents, data->status_label,
            MUIA_Background,    MUII_TextBack,
            MUIA_Frame,         MUIV_Frame_Text,
            MUIA_Text_PreParse, MUIX_C,
          End,
          Child, VGroup,
            Child, data->GA_COUNT = GaugeObject,
              GaugeFrame,
              MUIA_Gauge_Horiz,    TRUE,
              MUIA_Gauge_InfoText, data->msg_gauge_label,
            End,
            Child, data->GA_BYTES = GaugeObject,
              GaugeFrame,
              MUIA_Gauge_Horiz,    TRUE,
              MUIA_Gauge_InfoText, data->size_gauge_label,
            End,
          End,
          Child, data->TX_STATUS = TextObject,
            MUIA_Background,  MUII_TextBack,
            MUIA_Frame,       MUIV_Frame_Text,
          End,
          Child, data->BT_ABORT = MakeButton(tr(MSG_TR_Abort)),
        End,

      End,
    End,

    TAG_MORE, (ULONG)inittags(msg))) != NULL)
  {
    if((data = (struct Data *)INST_DATA(cl,obj)) == NULL)
    {
      RETURN(0);
      return 0;
    }

    // copy back the data stored in our temporarly struct Data
    memcpy(data, tmpData, sizeof(struct Data));

    // Add the window to the application object
    DoMethod(G->App, OM_ADDMEMBER, obj);

    // set some help text
    SetHelp(data->TX_STATUS, MSG_HELP_TR_TX_STATUS);
    SetHelp(data->BT_ABORT,  MSG_HELP_TR_BT_ABORT);

    set(obj, MUIA_Window_DefaultObject, data->LV_MAILS);
    set(data->BT_RESUME, MUIA_Disabled, TRUE);
    set(data->GR_PAGE, MUIA_Group_ActivePage, 1);

    // add some notifies
    DoMethod(data->BT_RESUME, MUIM_Notify, MUIA_Pressed, FALSE, obj, 2, MUIM_TransferWindow_PauseTransfer, FALSE);
    DoMethod(data->BT_PAUSE,  MUIM_Notify, MUIA_Pressed, FALSE, obj, 2, MUIM_TransferWindow_PauseTransfer, TRUE);
    DoMethod(data->BT_PAUSE,  MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_WriteLong, TRUE, &(data->Pause));
    DoMethod(data->LV_MAILS,  MUIM_Notify, MUIA_NList_DoubleClick, TRUE, obj, 1, MUIM_TransferWindow_GetMessageInfo);
    DoMethod(data->BT_DELONLY,MUIM_Notify, MUIA_Pressed, FALSE, obj, 2, MUIM_TransferWindow_ChangeTransferFlags, TRF_DELETE);
    DoMethod(data->BT_LOADDEL,MUIM_Notify, MUIA_Pressed, FALSE, obj, 2, MUIM_TransferWindow_ChangeTransferFlags, (TRF_LOAD|TRF_DELETE));
    DoMethod(data->BT_START,  MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_WriteLong, TRUE, &(data->Start));
    DoMethod(data->BT_QUIT,   MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_WriteLong, TRUE, &(data->Abort));

    DoMethod(data->BT_LOADONLY, MUIM_Notify, MUIA_Pressed, FALSE, obj, 2, MUIM_TransferWindow_ChangeTransferFlags, TRF_LOAD);
    DoMethod(data->BT_LEAVE,    MUIM_Notify, MUIA_Pressed, FALSE, obj, 2, MUIM_TransferWindow_ChangeTransferFlags, TRF_NONE);
    DoMethod(data->BT_ALL,      MUIM_Notify, MUIA_Pressed, FALSE, data->LV_MAILS, 4, MUIM_NList_Select, MUIV_NList_Select_All, MUIV_NList_Select_On, NULL);
    DoMethod(data->BT_NONE,     MUIM_Notify, MUIA_Pressed, FALSE, data->LV_MAILS, 4, MUIM_NList_Select, MUIV_NList_Select_All, MUIV_NList_Select_Off, NULL);

    DoMethod(data->BT_ABORT, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_WriteLong, TRUE, &(data->Abort));

    MA_ChangeTransfer(FALSE);
  }
  else
    obj = NULL;

  RETURN((IPTR)obj);
  return (IPTR)obj;
}

///
/// OVERLOAD(OM_DISPOSE)
OVERLOAD(OM_DISPOSE)
{
  GETDATA;
  ULONG result;
  ENTER();

  DeleteMailList(data->downloadedMails);
  data->downloadedMails = NULL;

  result = DoSuperMethodA(cl, obj, msg);

  RETURN(result);
  return result;
}

///
/// OVERLOAD(OM_GET)
OVERLOAD(OM_GET)
{
  GETDATA;
  IPTR *store = ((struct opGet *)msg)->opg_Storage;

  switch(((struct opGet *)msg)->opg_AttrID)
  {
#warning "TODO: should we use the global ImportFile/Folder/Format instead??"
    ATTR(ImportFile):       *store = (ULONG)data->ImportFile; return TRUE;
    ATTR(ImportFolder):     *store = (ULONG)data->ImportFolder; return TRUE;
    ATTR(ImportFormat):     *store = data->ImportFormat; return TRUE;
    ATTR(DownloadedMails):  *store = (ULONG)data->downloadedMails; return TRUE;
    ATTR(TransStat_MsgsTot):*store = data->transferStat.Msgs_Tot; return TRUE;
  }

  return DoSuperMethodA(cl, obj, msg);
}

///
/// OVERLOAD(OM_SET)
OVERLOAD(OM_SET)
{
  GETDATA;
  struct TagItem *tags = inittags(msg), *tag;

  while((tag = NextTagItem((APTR)&tags)))
  {
    switch(tag->ti_Tag)
    {
      ATTR(ImportFile):
      {
        strlcpy(data->ImportFile, (char *)tag->ti_Data, sizeof(data->ImportFile));

        // make the superMethod call ignore those tags
        tag->ti_Tag = TAG_IGNORE;
      }
      break;

      ATTR(ImportFolder):
      {
        data->ImportFolder = (struct Folder *)tag->ti_Data;

        // make the superMethod call ignore those tags
        tag->ti_Tag = TAG_IGNORE;
      }
      break;

      ATTR(ImportFormat):
      {
        data->ImportFormat = (enum ImportFormat)tag->ti_Data;

        // make the superMethod call ignore those tags
        tag->ti_Tag = TAG_IGNORE;
      }
      break;

      ATTR(Title):
      {
        // compose the window title
        if(data->transferType == TR_IMPORT || data->transferType == TR_GET_USER || data->transferType == TR_GET_AUTO)
          snprintf(data->windowTitle, sizeof(data->windowTitle), tr(MSG_TR_MailTransferFrom), tag->ti_Data);
        else
          snprintf(data->windowTitle, sizeof(data->windowTitle), tr(MSG_TR_MailTransferTo), tag->ti_Data);

        // set the window title
        set(obj, MUIA_Window_Title, data->windowTitle);

        // make the superMethod call ignore those tags
        tag->ti_Tag = TAG_IGNORE;
      }
      break;

      ATTR(StatusLabel):
      {
        set(data->TX_STATUS, MUIA_Text_Contents, tag->ti_Data);

        // make the superMethod call ignore those tags
        tag->ti_Tag = TAG_IGNORE;
      }
      break;
    }
  }

  return DoSuperMethodA(cl, obj, msg);
}

///

/* Public Methods */
/// DECLARE(Clear)
DECLARE(Clear)
{
  GETDATA;

  DoMethod(data->LV_MAILS, MUIM_NList_Clear);

  RETURN(0);
  return 0;
}

///
/// DECLARE(DisplayMailList)
DECLARE(DisplayMailList) // struct TransferNode *tfn, ULONG largeonly
{
  GETDATA;
  struct Node *curNode;
  int pos=0;

  ENTER();

  set(data->LV_MAILS, MUIA_NList_Quiet, TRUE);

  // search through our transferList
  IterateList(&msg->tfn->mailTransferList, curNode)
  {
    struct MailTransferNode *mtn = (struct MailTransferNode *)curNode;
    #if defined(DEBUG)
    struct Mail *mail = mtn->mail;
    #endif

    D(DBF_GUI, "checking mail with flags %08lx and subject '%s'", mtn->tflags, mail->Subject);

    // only display mails to be downloaded
    if(hasTR_LOAD(mtn) || hasTR_PRESELECT(mtn))
    {
      // add this mail to the transfer list in case we either
      // should show ALL mails or the mail size is >= the warning size
      if(msg->largeonly == FALSE || hasTR_PRESELECT(mtn))
      {
        mtn->position = pos++;

        DoMethod(data->LV_MAILS, MUIM_NList_InsertSingle, mtn, MUIV_NList_Insert_Bottom);
        D(DBF_GUI, "added mail with subject '%s' and size %ld to preselection list", mail->Subject, mail->Size);
      }
      else
        D(DBF_GUI, "skipped mail with subject '%s' and size %ld", mail->Subject, mail->Size);
    }
    else
      D(DBF_GUI, "skipped mail with subject '%s' and size %ld", mail->Subject, mail->Size);
  }

  xset(data->LV_MAILS, MUIA_NList_Active, MUIV_NList_Active_Top,
                       MUIA_NList_Quiet, FALSE);

  RETURN(0);
  return 0;
}

///
/// DECLARE(CompleteMailList)
DECLARE(CompleteMailList) // struct List *mailTransferList
{
  GETDATA;
  ENTER();

  // make sure the correct group page
  // is visible
  set(data->GR_PAGE, MUIA_Group_ActivePage, 0);

  // if a mail transfer list is supplied we go and start from the
  // head of it
  if(msg->mailTransferList != NULL)
  {
    data->GMD_Mail = (struct MinNode *)GetHead((struct List *)msg->mailTransferList);
    data->GMD_Line = 0;
  }

  // first we have to set the notifies to the default values.
  // this is needed so that if we get mail from more than one POP3 at a line this
  // abort stuff works out
  set(data->BT_PAUSE, MUIA_Disabled, FALSE);
  set(data->BT_RESUME, MUIA_Disabled, TRUE);
  DoMethod(data->BT_START, MUIM_KillNotify, MUIA_Pressed);
  DoMethod(data->BT_START, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_WriteLong, TRUE, &(data->Start));
  DoMethod(data->BT_QUIT , MUIM_KillNotify, MUIA_Pressed);
  DoMethod(data->BT_QUIT , MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_WriteLong, TRUE, &(data->Abort));

  if(C->PreSelection < PSM_ALWAYSLARGE)
  {
    struct MinNode *curNode = data->GMD_Mail;

    for(; curNode->mln_Succ != NULL && data->Abort == FALSE && G->Error == FALSE; curNode = curNode->mln_Succ)
    {
      struct MailTransferNode *mtn = (struct MailTransferNode *)curNode;

      if(data->Pause == TRUE)
        break;

      if(data->Start == TRUE)
      {
        CallHookPkt(&TR_ProcessGETHook, 0, 0);
        break;
      }

      if(C->PreSelection != PSM_LARGE || mtn->mail->Size >= C->WarnSize*1024)
      {
        TR_GetMessageDetails(mtn, data->GMD_Line++);

        // set the next mail as the active one for the display,
        // so that if the user pauses we can go on here
        data->GMD_Mail = curNode->mln_Succ;
      }
    }
  }

  set(data->BT_PAUSE, MUIA_Disabled, TRUE);
  DoMethod(data->BT_START, MUIM_KillNotify, MUIA_Pressed);
  DoMethod(data->BT_START, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 2, MUIM_CallHook, &TR_ProcessGETHook);
  DoMethod(data->BT_QUIT , MUIM_KillNotify, MUIA_Pressed);
  DoMethod(data->BT_QUIT , MUIM_Notify, MUIA_Pressed, FALSE, obj, 1, MUIM_TransferWindow_AbortTransfer);

  if(data->Abort == TRUE)
    DoMethod(obj, MUIM_TransferWindow_AbortTransfer);
  else
  {
    // start the timer which makes sure we send
    // a regular NOOP command to the POP3 server
    // so that it doesn't drop the connection
    RestartTimer(TIMER_POP3_KEEPALIVE, C->KeepAliveInterval, 0);
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(RedrawEntry)
DECLARE(RedrawEntry) // int line
{
  GETDATA;
  ULONG result;
  ENTER();

  result = DoMethod(data->LV_MAILS, MUIM_NList_Redraw, msg->line);

  RETURN(result);
  return result;
}

///
/// DECLARE(PauseTransfer)
//  Pauses or resumes message information transfer
DECLARE(PauseTransfer) // ULONG pause
{
  GETDATA;
  ENTER();

  set(data->BT_RESUME, MUIA_Disabled, msg->pause == FALSE);
  set(data->BT_PAUSE,  MUIA_Disabled, msg->pause == TRUE);

  if(msg->pause == TRUE)
  {
    // start the timer which makes sure we send
    // a regular NOOP command to the POP3 server
    // so that it doesn't drop the connection
    RestartTimer(TIMER_POP3_KEEPALIVE, C->KeepAliveInterval, 0);
  }
  else
  {
    data->Pause = FALSE;
    DoMethod(obj, MUIM_TransferWindow_CompleteMailList, NULL);
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(GetMessageInfo)
// Requests message header of a message selected by the user
DECLARE(GetMessageInfo)
{
  GETDATA;
  int line;
  struct MailTransferNode *mtn = NULL;

  ENTER();

  line = xget(data->LV_MAILS, MUIA_NList_Active);
  DoMethod(data->LV_MAILS, MUIM_NList_GetEntry, line, &mtn);
  TR_GetMessageDetails(mtn, line);

  RETURN(0);
  return 0;

}

///
/// DECLARE(AbortTransfer)
// Aborts a mail transfer
DECLARE(AbortTransfer)
{
  GETDATA;
  ENTER();

  // make sure the NOOP timer is definitly stopped
  StopTimer(TIMER_POP3_KEEPALIVE);

  // first set the Abort variable so that other can benefit from it
  data->Abort = TRUE;
  data->activeTransfer->abort = TRUE;
  G->activeTransfer->abort = TRUE;

  // we can easily abort the transfer by setting the POP_Nr to the
  // highest value possible and issue a GetMailFromNextPOP command.
  // with this solution YAM will also process the filters for mails that
  // were already downloaded even if the user aborted the transfer somehow
#warning "FIXME: check if valid?"
//  G->TR->POP_Nr = MAXP3;
//  TR_GetMailFromNextPOP(FALSE, MAXP3, 0);

  RETURN(0);
  return 0;
}

///
/// DECLARE(TransStat_Init)
// Initializes transfer statistics
DECLARE(TransStat_Init)
{
  GETDATA;
  struct Node *curNode;
  ENTER();

  data->transferStat.Msgs_Tot = 0;
  data->transferStat.Size_Tot = 0;

  if(data->GR_LIST != NULL)
  {
    set(data->GR_PAGE, MUIA_Group_ActivePage, 1);
    DoMethod(data->LV_MAILS, MUIM_NList_Select, MUIV_NList_Select_All, MUIV_NList_Select_Off, NULL);
  }

  // search through our transferList
  IterateList(&data->activeTransfer->mailTransferList, curNode)
  {
    struct MailTransferNode *mtn = (struct MailTransferNode *)curNode;

    data->transferStat.Msgs_Tot++;

    if(hasTR_LOAD(mtn))
      data->transferStat.Size_Tot += mtn->mail->Size;
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(TransStat_Start)
// Resets statistics display
DECLARE(TransStat_Start)
{
  GETDATA;
  ENTER();

  data->transferStat.Msgs_Done = 0;
  data->transferStat.Size_Done = 0;

  // get the actual time we started the TransferStatus
  GetSysTime(TIMEVAL(&data->transferStat.Clock_Last));
  data->transferStat.Clock_Start = data->transferStat.Clock_Last.Seconds;

  memset(&data->transferStat.Clock_Last, 0, sizeof(data->transferStat.Clock_Last));

  snprintf(data->msg_gauge_label, sizeof(data->msg_gauge_label), tr(MSG_TR_MESSAGEGAUGE), data->transferStat.Msgs_Tot);
  xset(data->GA_COUNT, MUIA_Gauge_InfoText, data->msg_gauge_label,
                       MUIA_Gauge_Max,      data->transferStat.Msgs_Tot,
                       MUIA_Gauge_Current,  0);

  RETURN(0);
  return 0;
}

///
/// DECLARE(TransStat_Finish)
// updates statistics display to represent the final state
DECLARE(TransStat_Finish)
{
  GETDATA;
  ENTER();

  // make sure we have valid strings to display
  FormatSize(data->transferStat.Size_Curr_Max, data->transferStat.str_size_curr_max,
             sizeof(data->transferStat.str_size_curr_max), SF_AUTO);

  // show the final statistics
  snprintf(data->msg_gauge_label, sizeof(data->msg_gauge_label), tr(MSG_TR_MESSAGEGAUGE), data->transferStat.Msgs_Tot);
  xset(data->GA_COUNT, MUIA_Gauge_InfoText, data->msg_gauge_label,
                       MUIA_Gauge_Max,      data->transferStat.Msgs_Tot,
                       MUIA_Gauge_Current,  data->transferStat.Msgs_Tot);

  snprintf(data->size_gauge_label, sizeof(data->size_gauge_label), tr(MSG_TR_TRANSFERSIZE),
                                   data->transferStat.str_size_curr_max, data->transferStat.str_size_curr_max);

  xset(data->GA_BYTES, MUIA_Gauge_InfoText, data->size_gauge_label,
                       MUIA_Gauge_Max,      100,
                       MUIA_Gauge_Current,  100);
  RETURN(0);
  return 0;
}

///
/// DECLARE(TransStat_NextMsg)
// Updates statistics display for next message
DECLARE(TransStat_NextMsg) // int index, int listpos, LONG size, const char *status
{
  GETDATA;
  ENTER();

  data->transferStat.Msgs_Curr = msg->index;
  data->transferStat.Msgs_ListPos = msg->listpos;
  data->transferStat.Msgs_Done++;
  data->transferStat.Size_Curr = 0;
  data->transferStat.Size_Curr_Max = msg->size;

  // format the current mail's size ahead of any refresh
  FormatSize(msg->size, data->transferStat.str_size_curr_max, sizeof(data->transferStat.str_size_curr_max), SF_AUTO);

  DoMethod(obj, MUIM_TransferWindow_TransStat_Update, 0, msg->status);

  RETURN(0);
  return 0;
}

///
/// DECLARE(TransStat_Update)
// Updates statistics display for next block of data
DECLARE(TransStat_Update) // int size_incr, const char *status
{
  GETDATA;
  ENTER();

  if(msg->size_incr > 0)
  {
    data->transferStat.Size_Done += msg->size_incr;
    data->transferStat.Size_Curr += msg->size_incr;
  }
  else if(msg->size_incr == TS_SETMAX)
  {
    // first update the total transferred size
    data->transferStat.Size_Done += data->transferStat.Size_Curr_Max - data->transferStat.Size_Curr;

    // we are done with this mail, so make sure the current size equals the final size
    data->transferStat.Size_Curr = data->transferStat.Size_Curr_Max;
  }

  // if the window isn't open we don't need to update it, do we?
  if(xget(obj, MUIA_Window_Open) == TRUE)
  {
    // update the stats at most 4 times per second
    if(TimeHasElapsed(&data->transferStat.Clock_Last, 250000) == TRUE)
    {
      ULONG deltatime = data->transferStat.Clock_Last.Seconds - data->transferStat.Clock_Start;
      ULONG speed = 0;
      LONG remclock = 0;
      ULONG max;
      ULONG current;

      // if we have a preselection window, update it.
      if(data->GR_LIST != NULL && data->transferStat.Msgs_ListPos >= 0)
        set(data->LV_MAILS, MUIA_NList_Active, data->transferStat.Msgs_ListPos);

      // first we calculate the speed in bytes/sec
      // to display to the user
      if(deltatime != 0)
        speed = data->transferStat.Size_Done / deltatime;

      // calculate the estimated remaining time
      if(speed != 0 && ((remclock = (data->transferStat.Size_Tot / speed) - deltatime) < 0))
        remclock = 0;

      // show the current status
      set(data->TX_STATUS, MUIA_Text_Contents, msg->status);

      // show the current message index
      set(data->GA_COUNT, MUIA_Gauge_Current, data->transferStat.Msgs_Curr);

      // format the size done and size total strings
      FormatSize(data->transferStat.Size_Done, data->transferStat.str_size_done, sizeof(data->transferStat.str_size_done), SF_MIXED);
      FormatSize(data->transferStat.Size_Tot, data->transferStat.str_size_tot, sizeof(data->transferStat.str_size_tot), SF_MIXED);
      FormatSize(data->transferStat.Size_Curr, data->transferStat.str_size_curr, sizeof(data->transferStat.str_size_curr), SF_AUTO);
      FormatSize(speed, data->transferStat.str_speed, sizeof(data->transferStat.str_speed), SF_MIXED);

      // now format the StatsLabel and update it
      snprintf(data->status_label, sizeof(data->status_label), tr(MSG_TR_TRANSFERSTATUS),
                                   data->transferStat.str_size_done, data->transferStat.str_size_tot, data->transferStat.str_speed,
                                   deltatime / 60, deltatime % 60,
                                   remclock / 60, remclock % 60);

      set(data->TX_STATS, MUIA_Text_Contents, data->status_label);

      // update the gauge
      snprintf(data->size_gauge_label, sizeof(data->size_gauge_label), tr(MSG_TR_TRANSFERSIZE),
                                       data->transferStat.str_size_curr, data->transferStat.str_size_curr_max);

      if(msg->size_incr == TS_SETMAX)
      {
        max = 100;
        current = 100;
      }
      else if(data->transferStat.Size_Curr_Max <= 65536)
      {
        max = data->transferStat.Size_Curr_Max;
        current = data->transferStat.Size_Curr;
      }
      else
      {
        max = data->transferStat.Size_Curr_Max / 1024;
        current = data->transferStat.Size_Curr / 1024;
      }
      xset(data->GA_BYTES, MUIA_Gauge_InfoText, data->size_gauge_label,
                           MUIA_Gauge_Max,      max,
                           MUIA_Gauge_Current,  current);

      // signal the application to update now
      DoMethod(G->App, MUIM_Application_InputBuffered);
    }
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(ChangeTransferFlags)
// Changes transfer flags of all selected messages
DECLARE(ChangeTransferFlags) // int flags
{
  GETDATA;
  int id = MUIV_NList_NextSelected_Start;

  ENTER();

  while(TRUE)
  {
    struct MailTransferNode *mtn = NULL;

    DoMethod(data->LV_MAILS, MUIM_NList_NextSelected, &id);
    if(id == MUIV_NList_NextSelected_End)
      break;

    DoMethod(data->LV_MAILS, MUIM_NList_GetEntry, id, &mtn);
    if(mtn != NULL)
    {
      //set the transferfalsg
      mtn->tflags = msg->flags;

      // redraw the nlist entry
      DoMethod(data->LV_MAILS, MUIM_NList_Redraw, id);
    }
    else
      E(DBF_GUI, "couldn't get entry %d from LV_MAILS", id);
  }

  RETURN(0);
  return 0;
}

///
