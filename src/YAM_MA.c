/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2006 by YAM Open Source Team

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

#include <stdlib.h>
#include <string.h>

#include <clib/alib_protos.h>
#include <libraries/asl.h>
#include <libraries/iffparse.h>
#include <mui/NList_mcc.h>
#include <mui/NListview_mcc.h>
#include <mui/TextEditor_mcc.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/muimaster.h>
#include <proto/utility.h>
#include <proto/timer.h>
#include <rexx/storage.h>

#include "extra.h"
#include "SDI_hook.h"

#include "YAM.h"
#include "YAM_addressbook.h"
#include "YAM_addressbookEntry.h"
#include "YAM_config.h"
#include "YAM_error.h"
#include "YAM_find.h"
#include "YAM_folderconfig.h"
#include "YAM_global.h"
#include "YAM_locale.h"
#include "YAM_main.h"
#include "YAM_mainFolder.h"
#include "YAM_mime.h"
#include "YAM_read.h"
#include "YAM_rexx.h"
#include "YAM_userlist.h"
#include "YAM_utilities.h"
#include "YAM_write.h"

#include "classes/Classes.h"
#include "UpdateCheck.h"

#include "Debug.h"

/* local structures */
struct ExpandTextData
{
  char *            OS_Name;
  char *            OS_Address;
  char *            OM_Subject;
  struct DateStamp  OM_Date;
  int               OM_TimeZone;
  char *            OM_MessageID;
  char *            R_Name;
  char *            R_Address;
  char *            HeaderFile;
};

/* local protos */
static ULONG MA_GetSortType(int);
static struct Mail *MA_MoveCopySingle(struct Mail*, struct Folder*, struct Folder*, BOOL);
static void MA_UpdateStatus(void);
static char *MA_AppendRcpt(char*, struct Person*, BOOL);
static int MA_CmpDate(struct Mail**, struct Mail**);
static void MA_InsertIntroText(FILE*, char*, struct ExpandTextData*);
static void MA_EditorNotification(int);
static void MA_SetupQuoteString(struct WR_ClassData*, struct ExpandTextData*, struct Mail*);
static int MA_CheckWriteWindow(int);
static struct Person *MA_GetAddressSelect(struct Mail*);

/***************************************************************************
 Module: Main
***************************************************************************/

/*** Private functions ***/
/// MA_GetSortType
//  Calculates value for sort indicator
static ULONG MA_GetSortType(int sort)
{
   static const ULONG sort2col[8] = { 0,4,7,1,1,3,5,0 };
   if(sort > 0)
      return sort2col[sort];
   else
      return sort2col[-sort] | MUIV_NList_SortTypeAdd_2Values;
}

///
/// MA_SetSortFlag
//  Sets sort indicators in message listview header
void MA_SetSortFlag(void)
{
  struct Folder *fo = FO_GetCurrentFolder();
  if(!fo)
    return;

  SetAttrs(G->MA->GUI.PG_MAILLIST, MUIA_NList_SortType, MA_GetSortType(fo->Sort[0]),
                                   MUIA_NList_SortType2, MA_GetSortType(fo->Sort[1]),
                                   NULL);
}

///
/// MA_ChangeTransfer
//  Disables menus and toolbar buttons during transfer operations
void MA_ChangeTransfer(BOOL on)
{
   struct MA_GUIData *gui = &G->MA->GUI;
   if (gui->TO_TOOLBAR) DoMethod(gui->TO_TOOLBAR, MUIM_Toolbar_MultiSet, MUIV_Toolbar_Set_Ghosted, !on, 10,11, -1);
   DoMethod(G->App, MUIM_MultiSet, MUIA_Menuitem_Enabled, on, gui->MI_IMPORT, gui->MI_EXPORT, gui->MI_SENDALL, gui->MI_EXCHANGE, gui->MI_GETMAIL, gui->MI_CSINGLE, NULL);
}

///
/// MA_ChangeSelected()
// function which updates some mail information on the main
// window and triggers an update of the embeeded read pane if required.
void MA_ChangeSelected(BOOL forceUpdate)
{
  static struct Mail *lastMail = NULL;
  struct MA_GUIData *gui = &G->MA->GUI;
  struct Folder *fo = FO_GetCurrentFolder();
  int i;
  BOOL active;
  BOOL hasattach = FALSE;
  BOOL beingedited = FALSE;
  BOOL folderEnabled;
  ULONG numSelected = 0;
  struct Mail *mail;

  ENTER();

  if(!fo)
  {
    LEAVE();
    return;
  }

  // get the currently active mail entry.
  DoMethod(gui->PG_MAILLIST, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &mail);

  // now we check if the previously selected mail is the same one as
  // the currently active one, then we don't have to proceed.
  if(forceUpdate == FALSE && mail == lastMail)
  {
    LEAVE();
    return;
  }
  else
    lastMail = mail;

  // we make sure the an eventually running timer event for setting the mail
  // status of a previous mail to read is canceled beforehand
  if(C->StatusChangeDelayOn)
    TC_Stop(TIO_READSTATUSUPDATE);

  // ask the mail list how many entries as currently selected
  DoMethod(gui->PG_MAILLIST, MUIM_NList_Select, MUIV_NList_Select_All, MUIV_NList_Select_Ask, &numSelected);

  // make sure the mail is displayed in our readMailGroup of the main window
  // (if enabled) - but we do only issue a timer event here so the read pane
  // is only refreshed about 100 milliseconds after the last change in the listview
  // was recognized.
  if(C->EmbeddedReadPane)
  {
    // but before we really issue a readpaneupdate we check whether the user has
    // selected more than one mail at a time which then should clear the
    // readpane as it might have been disabled.
    if(numSelected == 1)
      TC_Restart(TIO_READPANEUPDATE, 0, C->EmbeddedMailDelay*1000);
    else
    {
      // make sure an already existing readpaneupdate timer is canceled in advance.
      TC_Stop(TIO_READPANEUPDATE);

      // clear the readmail group now
      DoMethod(gui->MN_EMBEDDEDREADPANE, MUIM_ReadMailGroup_Clear, FALSE);
      lastMail = NULL;
    }
  }

  // in case the currently active maillist is the mainmainlist we
  // have to save the lastactive mail ID
  if(xget(gui->PG_MAILLIST, MUIA_MainMailListGroup_ActiveList) == LT_MAIN)
    fo->LastActive = xget(gui->PG_MAILLIST, MUIA_NList_Active);

  if((active = (mail != NULL)) && isMultiPartMail(mail))
    hasattach = TRUE;

  for(i = 0; i < MAXWR; i++)
  {
    if(mail && G->WR[i] && G->WR[i]->Mail == mail)
    {
      beingedited = TRUE;
      break;
    }
  }

  // now we have to make sure that all toolbar and menu items are
  // enabled and disabled according to the folder/mail status
  folderEnabled = !(fo->Type == FT_GROUP);

  // deal with the toolbar
  if(gui->TO_TOOLBAR)
  {
    DoMethod(gui->TO_TOOLBAR, MUIM_Toolbar_Set, 0, MUIV_Toolbar_Set_Ghosted, !active);
    DoMethod(gui->TO_TOOLBAR, MUIM_Toolbar_Set, 1, MUIV_Toolbar_Set_Ghosted, !active || !isOutgoingFolder(fo) || numSelected > 1 || beingedited);
    DoMethod(gui->TO_TOOLBAR, MUIM_Toolbar_MultiSet, MUIV_Toolbar_Set_Ghosted, !active || numSelected == 0, 2,3,4,8, -1);
    DoMethod(gui->TO_TOOLBAR, MUIM_Toolbar_Set, 7, MUIV_Toolbar_Set_Ghosted, !active || numSelected == 0 || isOutgoingFolder(fo));
    DoMethod(gui->TO_TOOLBAR, MUIM_Toolbar_Set, 13, MUIV_Toolbar_Set_Ghosted, !folderEnabled);
  }

  // enable/disable menu items
  DoMethod(G->App, MUIM_MultiSet, MUIA_Menuitem_Enabled, (active || numSelected > 0) && folderEnabled,
                   gui->MI_MOVE, gui->MI_DELETE, gui->MI_GETADDRESS, gui->MI_FORWARD, gui->MI_STATUS,
                   gui->MI_EXPMSG, gui->MI_COPY, gui->MI_PRINT, gui->MI_SAVE, gui->MI_CHSUBJ, NULL);
  DoMethod(G->App, MUIM_MultiSet, MUIA_Menuitem_Enabled, folderEnabled, gui->MI_FILTER, gui->MI_UPDINDEX, gui->MI_IMPORT, gui->MI_EXPORT, gui->MI_SELECT, NULL);
  DoMethod(G->App, MUIM_MultiSet, MUIA_Menuitem_Enabled, active, gui->MI_READ, NULL);
  DoMethod(G->App, MUIM_MultiSet, MUIA_Menuitem_Enabled, active && isOutgoingFolder(fo) && !beingedited, gui->MI_EDIT, NULL);
  DoMethod(G->App, MUIM_MultiSet, MUIA_Menuitem_Enabled, fo->Type == FT_OUTGOING && (active || numSelected > 0), gui->MI_SEND, gui->MI_TOHOLD, gui->MI_TOQUEUED, NULL);
  DoMethod(G->App, MUIM_MultiSet, MUIA_Menuitem_Enabled, !isOutgoingFolder(fo) && (active || numSelected > 0) , gui->MI_TOREAD, gui->MI_TOUNREAD, gui->MI_ALLTOREAD, gui->MI_REPLY, gui->MI_BOUNCE, NULL);
  DoMethod(G->App, MUIM_MultiSet, MUIA_Menuitem_Enabled, hasattach && (active || numSelected > 0), gui->MI_ATTACH, gui->MI_SAVEATT, gui->MI_REMATT, NULL);

  LEAVE();
}

///
/// MA_ChangeSelectedFunc
//  User selected some message(s) in the message list
HOOKPROTONHNONP(MA_ChangeSelectedFunc, void)
{
  MA_ChangeSelected(FALSE);
}
MakeHook(MA_ChangeSelectedHook, MA_ChangeSelectedFunc);

///
/// MA_SetMessageInfoFunc
//  Builds help bubble for message list
HOOKPROTONHNONP(MA_SetMessageInfoFunc, void)
{
  static char buffer[SIZE_DEFAULT+SIZE_SUBJECT+2*SIZE_REALNAME+2*SIZE_ADDRESS+SIZE_MFILE];
  char *sh = NULL;
  struct Mail *mail = MA_GetActiveMail(NULL, NULL, NULL);

  if(mail)
  {
    char datstr[64];

    DateStamp2String(datstr, sizeof(datstr), &mail->Date, C->SwatchBeat ? DSS_DATEBEAT : DSS_DATETIME, TZC_LOCAL);
    SPrintF(sh = buffer, GetStr(MSG_MA_MessageInfo), mail->From.RealName,
                                                     mail->From.Address,
                                                     mail->To.RealName,
                                                     mail->To.Address,
                                                     mail->Subject,
                                                     datstr,
                                                     mail->MailFile,
                                                     mail->Size);
  }

  set(G->MA->GUI.PG_MAILLIST, MUIA_ShortHelp, sh);
}
MakeHook(MA_SetMessageInfoHook, MA_SetMessageInfoFunc);

///
/// MA_SetFolderInfoFunc
//  Builds help bubble for folder list
HOOKPROTONHNONP(MA_SetFolderInfoFunc, void)
{
   static char buffer[SIZE_DEFAULT+SIZE_NAME+SIZE_PATH];
   char *sh = NULL;
   struct Folder *fo = FO_GetCurrentFolder();

   if (fo && (fo->Type != FT_GROUP))
   {
      SPrintF(sh = buffer, GetStr(MSG_MA_FolderInfo), fo->Name, fo->Path, fo->Size, fo->Total, fo->New, fo->Unread);
   }

   set(G->MA->GUI.NL_FOLDERS, MUIA_ShortHelp, sh);
}
MakeHook(MA_SetFolderInfoHook, MA_SetFolderInfoFunc);

///
/// MA_GetActiveMail
//  Returns pointers to the active message and folder
struct Mail *MA_GetActiveMail(struct Folder *forcefolder, struct Folder **folderp, int *activep)
{
   struct Folder *folder;
   int active;
   struct Mail *mail = NULL;

   folder = forcefolder ? forcefolder : FO_GetCurrentFolder();

   if(!folder) return(NULL);

   MA_GetIndex(folder);
   active = xget(G->MA->GUI.PG_MAILLIST, MUIA_NList_Active);
   if (active != MUIV_NList_Active_Off) DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_NList_GetEntry, active, &mail);
   if (folderp) *folderp = folder;
   if (activep) *activep = active;

   return mail;
}

///
/// MA_ChangeMailStatus()
//  Sets the status of a message
void MA_ChangeMailStatus(struct Mail *mail, int addflags, int clearflags)
{
   unsigned int newstatus = (mail->sflags | addflags) & ~(clearflags);

   // check if the status is already set or not
   if(newstatus != mail->sflags)
   {
      D(DBF_MAIL, "ChangeMailStatus: +%08lx -%08lx", addflags, clearflags);

      // set the new status
      mail->sflags = newstatus;

      // set the comment to the Mailfile
      MA_UpdateMailFile(mail);

      // flag the index as expired
      MA_ExpireIndex(mail->Folder);

      // lets redraw the entry if it is actually displayed, so that
      // the status icon gets updated.
      DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_MainMailListGroup_RedrawMail, mail);
   }
}

///
/// MA_UpdateMailFile()
// Updates the mail filename by taking the supplied mail structure
// into account
BOOL MA_UpdateMailFile(struct Mail *mail)
{
  char dateFilePart[12+1];
  char statusFilePart[11+1];
  char newFileName[SIZE_MFILE];
  char newFilePath[SIZE_PATHFILE];
  char oldFilePath[SIZE_PATHFILE];
  char *folderDir = GetFolderDir(mail->Folder);
  char *ptr;
  BOOL success = FALSE;
  int mcounter;

  // modify the transferDate part
  base64encode(dateFilePart, (unsigned char *)&mail->transDate, sizeof(struct timeval));

  // for proper handling we have to remove an eventually existing "/" which
  // could be part of a base64 encoding
  ptr = dateFilePart;
  while((ptr = strchr(ptr, '/')))
    *ptr = '-';

  // get the counter from the current mailfile
  mcounter = atoi(&mail->MailFile[13]);
  if(mcounter < 1 || mcounter > 999)
    mcounter = 1;

  // now modify the status part
  ptr = statusFilePart;
  if(hasStatusRead(mail))       *ptr++ = SCHAR_READ;
  if(hasStatusReplied(mail))    *ptr++ = SCHAR_REPLIED;
  if(hasStatusForwarded(mail))  *ptr++ = SCHAR_FORWARDED;
  if(hasStatusNew(mail))        *ptr++ = SCHAR_NEW;
  if(hasStatusQueued(mail))     *ptr++ = SCHAR_QUEUED;
  if(hasStatusHold(mail))       *ptr++ = SCHAR_HOLD;
  if(hasStatusSent(mail))       *ptr++ = SCHAR_SENT;
  if(hasStatusDeleted(mail))    *ptr++ = SCHAR_DELETED;
  if(hasStatusMarked(mail))     *ptr++ = SCHAR_MARKED;
  if(hasStatusError(mail))      *ptr++ = SCHAR_ERROR;
  if(getPERValue(mail) > 0)     *ptr++ = '0'+getPERValue(mail);

  *ptr = '\0'; // NUL terminate it

  // construct the full old file path
  strlcpy(oldFilePath, folderDir, sizeof(oldFilePath));
  AddPart(oldFilePath, mail->MailFile, SIZE_PATHFILE);

  while(success == FALSE)
  {
    // generate a new filename with the data we have collected
    snprintf(newFileName, sizeof(newFileName), "%s.%03d,%s", dateFilePart, mcounter, statusFilePart);

    // now check if the filename has changed or not
    if(strcmp(newFileName, mail->MailFile) == 0)
    {
      success = TRUE;
      break;
    }

    // construct new full file path
    strlcpy(newFilePath, folderDir, sizeof(newFilePath));
    AddPart(newFilePath, newFileName, SIZE_PATHFILE);

    // then rename it
    if(Rename(oldFilePath, newFilePath) != 0)
    {
      strlcpy(mail->MailFile, newFileName, sizeof(mail->MailFile));
      success = TRUE;

      // before we exit we check through all our read windows if
      // they contain the mail we have changed the status, so
      // that we can update the filename in the read window structure
      // aswell
      if(IsMinListEmpty(&G->readMailDataList) == FALSE)
      {
        // search through our ReadDataList
        struct MinNode *curNode;
        for(curNode = G->readMailDataList.mlh_Head; curNode->mln_Succ; curNode = curNode->mln_Succ)
        {
          struct ReadMailData *rmData = (struct ReadMailData *)curNode;
          if(rmData->mail == mail)
            strlcpy(rmData->readFile, newFilePath, sizeof(rmData->readFile));
        }
      }
    }
    else
    {
      // if we end up here then a file with the newFileName
      // probably already exists, so lets increase the mail
      // counter.
      mcounter++;

      if(mcounter > 999)
        break;
    }
  }

  return success;
}

///
/// MA_CreateFullList
//  Builds a list containing all messages in a folder
struct Mail **MA_CreateFullList(struct Folder *fo, BOOL onlyNew)
{
   int selected;
   struct Mail *mail, **mlist = NULL;

   if(!fo) return(NULL);
   selected = onlyNew ? fo->New : fo->Total;

   if(selected && (mlist = calloc(selected+2, sizeof(struct Mail *))))
   {
      mlist[0] = (struct Mail *)selected;
      mlist[1] = (struct Mail *)2;
      for (selected = 2, mail = fo->Messages; mail; mail = mail->Next)
      {
         // only if we want ALL or this is just a �ew mail we add it to our list
         if(!onlyNew || hasStatusNew(mail))
         {
            mlist[selected] = mail;
            selected++;
         }
      }
   }

   return mlist;
}

///
/// MA_CreateMarkedList
//  Builds a linked list containing the selected messages
struct Mail **MA_CreateMarkedList(Object *lv, BOOL onlyNew)
{
   int id, selected;
   struct Mail *mail, **mlist = NULL;
   struct Folder *folder;

   // we first have to check whether this is a valid folder or not
   folder = FO_GetCurrentFolder();
   if(!folder || folder->Type == FT_GROUP) return NULL;

   DoMethod(lv, MUIM_NList_Select, MUIV_NList_Select_All, MUIV_NList_Select_Ask, &selected);
   if (selected)
   {
      if ((mlist = calloc(selected+2, sizeof(struct Mail *))))
      {
         mlist[0] = (struct Mail *)selected;
         mlist[1] = (struct Mail *)1;
         selected = 2;
         id = MUIV_NList_NextSelected_Start;

         while(1)
         {
            DoMethod(lv, MUIM_NList_NextSelected, &id);
            if (id == MUIV_NList_NextSelected_End) break;
            DoMethod(lv, MUIM_NList_GetEntry, id, &mail);
            mail->position = id;

            if(!onlyNew || hasStatusNew(mail))
            {
              mlist[selected] = mail;
              selected++;
            }
         }
      }
   }
   else
   {
      DoMethod(lv, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &mail);
      if(mail && (!onlyNew || hasStatusNew(mail)))
      {
        if ((mlist = calloc(3, sizeof(struct Mail *))))
        {
          id = xget(lv, MUIA_NList_Active);
          mail->position = id;
          mlist[0] = (struct Mail *)1;
          mlist[2] = mail;
        }
      }
   }

   return mlist;
}

///
/// MA_DeleteSingle
//  Deletes a single message
void MA_DeleteSingle(struct Mail *mail, BOOL forceatonce, BOOL quiet)
{
   struct Folder *mailFolder = mail->Folder;

   if(C->RemoveAtOnce || mailFolder->Type == FT_DELETED || forceatonce)
   {
      AppendLogVerbose(21, GetStr(MSG_LOG_DeletingVerbose), AddrName(mail->From), mail->Subject, mailFolder->Name, "");

      // make sure we delete the mailfile
      DeleteFile(GetMailFile(NULL, mailFolder, mail));

      // now remove the mail from its folder/mail list
      RemoveMailFromList(mail);

      // if we are allowed to make some noise we
      // update our Statistics
      if(!quiet)
        DisplayStatistics(mailFolder, TRUE);
   }
   else
   {
      struct Folder *delfolder = FO_GetFolderByType(FT_DELETED, NULL);

      MA_MoveCopySingle(mail, mailFolder, delfolder, FALSE);

      // if we are allowed to make some noise we
      // update our Statistics
      if(!quiet)
      {
        DisplayStatistics(delfolder, FALSE);  // don`t update the appicon
        DisplayStatistics(mailFolder, TRUE);  // but update it now.
      }
   }
}

///
/// MA_MoveCopySingle
//  Moves or copies a single message from one folder to another
static struct Mail *MA_MoveCopySingle(struct Mail *mail, struct Folder *from, struct Folder *to, BOOL copyit)
{
   struct Mail cmail = *mail;
   char mfile[SIZE_MFILE];
   int result;

   strlcpy(mfile, mail->MailFile, sizeof(mfile));

   if((result = TransferMailFile(copyit, mail, to)) >= 0)
   {
      strlcpy(cmail.MailFile, mail->MailFile, sizeof(cmail.MailFile));

      if(copyit)
      {
        AppendLogVerbose(25, GetStr(MSG_LOG_CopyingVerbose), AddrName(mail->From), mail->Subject, from->Name, to->Name);

        strlcpy(mail->MailFile, mfile, sizeof(mail->MailFile));
      }
      else
      {
        AppendLogVerbose(23, GetStr(MSG_LOG_MovingVerbose),  AddrName(mail->From), mail->Subject, from->Name, to->Name);

        // now remove the mail from its folder/mail list
        RemoveMailFromList(mail);
      }

      mail = AddMailToList(&cmail, to);
      if(to == FO_GetCurrentFolder())
        DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_NList_InsertSingle, mail, MUIV_NList_Insert_Sorted);

      // check the status flags and set the mail statues to queued if the mail was copied into
      // the outgoing folder
      if(to->Type == FT_OUTGOING && hasStatusSent(mail))
      {
        setStatusToQueued(mail);
      }

      return mail;
   }
   else
   {
      W(DBF_MAIL, "MA_MoveCopySingle error: %ld", result);

      switch(result)
      {
        case -2:
          ER_NewError(GetStr(MSG_ER_XPKUSAGE), mail->MailFile);
        break;

        default:
          ER_NewError(GetStr(MSG_ER_TRANSFERMAIL), mail->MailFile, to->Name);
      }
   }

   return NULL;
}

///
/// MA_MoveCopy
//  Moves or copies messages from one folder to another
void MA_MoveCopy(struct Mail *mail, struct Folder *frombox, struct Folder *tobox, BOOL copyit)
{
  struct Mail **mlist;
  int selected = 0;

  if(frombox == tobox && !copyit)
    return;

  if(!(frombox == FO_GetCurrentFolder()) && !mail)
    return;

  // if a specific mail should be moved we do it now.
  if(mail)
  {
    selected = 1;
    MA_MoveCopySingle(mail, frombox, tobox, copyit);
  }
  else if((mlist = MA_CreateMarkedList(G->MA->GUI.PG_MAILLIST, FALSE)))
  {
    int i;

    // get the list of the currently marked mails
    selected = (int)*mlist;
    set(G->MA->GUI.PG_MAILLIST, MUIA_NList_Quiet, TRUE);
    BusyGauge(GetStr(MSG_BusyMoving), itoa(selected), selected);
    for(i = 0; i < selected; i++)
    {
      mail = mlist[i+2];
      MA_MoveCopySingle(mail, frombox, tobox, copyit);
      BusySet(i+1);
    }
    BusyEnd();
    set(G->MA->GUI.PG_MAILLIST, MUIA_NList_Quiet, FALSE);
    free(mlist);
  }

  // write some log out
  if(copyit)
    AppendLogNormal(24, GetStr(MSG_LOG_Copying), (void *)selected, FolderName(frombox), FolderName(tobox), "");
  else
    AppendLogNormal(22, GetStr(MSG_LOG_Moving),  (void *)selected, FolderName(frombox), FolderName(tobox), "");

  // refresh the folder statistics if necessary
  if(!copyit) DisplayStatistics(frombox, FALSE);
  DisplayStatistics(tobox, TRUE);

  MA_ChangeSelected(FALSE);
}

///
/// MA_UpdateStatus
//  Changes status of all new messages to unread
static void MA_UpdateStatus(void)
{
   int i;
   struct Mail *mail;
   struct Folder **flist;

   if((flist = FO_CreateList()))
   {
      for(i = 1; i <= (int)*flist; i++)
      {
        if(!isOutgoingFolder(flist[i]) && flist[i]->LoadedMode == LM_VALID)
        {
          BOOL updated = FALSE;

          for (mail = flist[i]->Messages; mail; mail = mail->Next)
          {
            if(hasStatusNew(mail))
            {
              updated = TRUE;

              setStatusToUnread(mail);
            }
          }

          if(updated)
            DisplayStatistics(flist[i], TRUE);
        }
      }
      free(flist);
   }
}
///
/// MA_ToStatusHeader()
// Function that converts the current flags of a message
// to "Status:" headerline flags
char *MA_ToStatusHeader(struct Mail *mail)
{
  static char flags[3]; // should not be more than 3 bytes

  if(hasStatusRead(mail))
  {
    if(hasStatusNew(mail))
    {
      flags[0] = 'R';
      flags[1] = '\0';
    }
    else
    {
      flags[0] = 'R';
      flags[1] = 'O';
      flags[2] = '\0';
    }
  }
  else
  {
    if(hasStatusNew(mail))
    {
      flags[0] = '\0';
    }
    else
    {
      flags[0] = 'O';
      flags[1] = '\0';
    }
  }

  return flags;
}

///
/// MA_ToXStatusHeader()
// Function that converts the current flags of a message
// to "X-Status:" headerline flags
char *MA_ToXStatusHeader(struct Mail *mail)
{
  static char flags[6]; // should not be more than 5+1 bytes
  char *ptr = flags;

  if(hasStatusRead(mail))
    *ptr++ = 'R';

  if(hasStatusReplied(mail))
    *ptr++ = 'A';

  if(hasStatusMarked(mail))
    *ptr++ = 'F';

  if(hasStatusDeleted(mail))
    *ptr++ = 'D';

  if(hasStatusHold(mail))
    *ptr++ = 'T';

  // NUL terminate it
  *ptr = '\0';

  return flags;
}

///
/// MA_FromStatusHeader()
// Function that converts chars from the Status: headerline to a proper
// mail status flag value
unsigned int MA_FromStatusHeader(char *statusflags)
{
  unsigned int sflags = SFLAG_NEW;

  while(*statusflags != '\0')
  {
    switch(*statusflags)
    {
      case 'R':
        SET_FLAG(sflags, SFLAG_READ);
      break;

      case 'O':
        CLEAR_FLAG(sflags, SFLAG_NEW);
      break;
    }

    statusflags++;
  }

  return sflags;
}

///
/// MA_FromXStatusHeader()
// Function that converts chars from the X-Status: headerline to a
// proper mail status flag value
unsigned int MA_FromXStatusHeader(char *xstatusflags)
{
  unsigned int sflags = SFLAG_NEW;

  while(*xstatusflags != '\0')
  {
    switch(*xstatusflags)
    {
      case 'R':
        SET_FLAG(sflags, SFLAG_READ);
        CLEAR_FLAG(sflags, SFLAG_NEW);
      break;

      case 'A':
        SET_FLAG(sflags, SFLAG_REPLIED);
      break;

      case 'F':
        SET_FLAG(sflags, SFLAG_MARKED);
      break;

      case 'D':
        SET_FLAG(sflags, SFLAG_DELETED);
      break;

      case 'T':
        SET_FLAG(sflags, SFLAG_HOLD);
      break;
    }

    xstatusflags++;
  }

  return sflags;
}

///
/// ExpandText
//  Replaces variables with values
static char *ExpandText(char *src, struct ExpandTextData *etd)
{
  char buf[SIZE_ADDRESS];
  char *p;
  char *p2;
  char *dst = AllocStrBuf(SIZE_DEFAULT);

  ENTER();

  for(; *src; src++)
  {
    if(*src == '\\')
    {
      src++;
      switch (*src)
      {
        case '\\':
          dst = StrBufCat(dst, "\\");
        break;

        case 'n':
          dst = StrBufCat(dst, "\n");
        break;
      }
    }
    else if(*src == '%' && etd)
    {
      src++;
      switch(*src)
      {
        case 'n':
          dst = StrBufCat(dst, etd->OS_Name);
        break;

        case 'f':
        {
          strlcpy(buf, etd->OS_Name, sizeof(buf));

          if((p = strchr(buf, ',')))
            p = Trim(++p);
          else
          {
            for(p = buf; *p && *p != ' '; p++);

            *p = 0;
            p = buf;
          }
          dst = StrBufCat(dst, p);
        }
        break;

        case 's':
          dst = StrBufCat(dst, etd->OM_Subject);
        break;

        case 'e':
          dst = StrBufCat(dst, etd->OS_Address);
        break;

        case 'd':
        {
          char datstr[64];
          DateStamp2String(datstr, sizeof(datstr), &etd->OM_Date, DSS_DATE, TZC_NONE);
          dst = StrBufCat(dst, datstr);
        }
        break;

        case 't':
        {
          char datstr[64];
          DateStamp2String(datstr, sizeof(datstr), &etd->OM_Date, DSS_TIME, TZC_NONE);
          dst = StrBufCat(dst, datstr);
        }
        break;

        case 'z':
        {
          char tzone[6];
          int convertedTimeZone = (etd->OM_TimeZone/60)*100 + (etd->OM_TimeZone%60);
          snprintf(tzone, sizeof(tzone), "%+05d", convertedTimeZone);
          dst = StrBufCat(dst, tzone);
        }
        break;

        case 'w':
        {
          char datstr[64];
          DateStamp2String(datstr, sizeof(datstr), &etd->OM_Date, DSS_WEEKDAY, TZC_NONE);
          dst = StrBufCat(dst, datstr);
        }
        break;

        case 'c':
        {
          char datstr[64];
          DateStamp2RFCString(datstr, sizeof(datstr), &etd->OM_Date, etd->OM_TimeZone, FALSE);
          dst = StrBufCat(dst, datstr);
        }
        break;

        case 'm':
          dst = StrBufCat(dst, etd->OM_MessageID);
        break;

        case 'r':
          dst = StrBufCat(dst, etd->R_Name);
        break;

        case 'v':
        {
          strlcpy(buf, etd->R_Name, sizeof(buf));
          if((p = strchr(buf, ',')))
            p = Trim(++p);
          else
          {
            for(p = buf; *p && *p != ' '; p++);

            *p = '\0';
            p = buf;
          }
          dst = StrBufCat(dst, p);
        }
        break;

        case 'a':
          dst = StrBufCat(dst, etd->R_Address);
        break;

        case 'i':
        {
          strlcpy(buf, etd->OS_Name, sizeof(buf));

          for(p = p2 = &buf[1]; *p; p++)
          {
            if(*p == ' ' && p[1] && p[1] != ' ')
              *p2++ = *++p;
          }
          *p2 = '\0';
          dst = StrBufCat(dst, buf);
        }
        break;

        case 'j':
        {
          strlcpy(buf, etd->OS_Name, sizeof(buf));

          for(p2 = &buf[1], p = &buf[strlen(buf)-1]; p > p2; p--)
          {
            if(p[-1] == ' ')
            {
              *p2++ = *p;
              break;
            }
          }
          *p2 = '\0';
          dst = StrBufCat(dst, buf);
        }
        break;

        case 'h':
        {
          if((p = FileToBuffer(etd->HeaderFile)))
          {
            dst = StrBufCat(dst, p);
            free(p);
          }
        }
        break;
      }
    }
    else
    {
       static char chr[2] = { 0,0 };
       chr[0] = *src;
       dst = StrBufCat(dst, chr);
    }
  }

  RETURN(dst);
  return dst;
}
///

/*** Main button functions ***/
/// MA_ReadMessage
//  Loads active message into a read window
HOOKPROTONHNONP(MA_ReadMessage, void)
{
  struct Mail *mail;

  if((mail = MA_GetActiveMail(ANYBOX, NULL, NULL)))
  {
    struct ReadMailData *rmData;

    // Check if this mail is already in a readwindow
    if(IsMinListEmpty(&G->readMailDataList) == FALSE)
    {
      // search through our ReadDataList
      struct MinNode *curNode;
      for(curNode = G->readMailDataList.mlh_Head; curNode->mln_Succ; curNode = curNode->mln_Succ)
      {
        rmData = (struct ReadMailData *)curNode;

        // check if the active mail is already open in another read
        // window, and if so we just bring it to the front.
        if(rmData != G->ActiveRexxRMData &&
           rmData->readWindow &&
           rmData->mail == mail)
        {
          DoMethod(rmData->readWindow, MUIM_Window_ToFront);
          set(rmData->readWindow, MUIA_Window_Activate, TRUE);
          return;
        }
      }
    }

    // if not, then we create/reuse a new one
    if((rmData = CreateReadWindow(FALSE)))
    {
      // make sure it is opened correctly and then read in a mail
      if(SafeOpenWindow(rmData->readWindow) == FALSE ||
         DoMethod(rmData->readWindow, MUIM_ReadWindow_ReadMail, mail) == FALSE)
      {
        // on any error we make sure to delete the read window
        // immediatly again.
        CleanupReadMailData(rmData, TRUE);
      }
    }
  }
}
MakeHook(MA_ReadMessageHook, MA_ReadMessage);

///
/// MA_AppendRcpt
//  Appends a recipient address to a string
static char *MA_AppendRcpt(char *sbuf, struct Person *pe, BOOL excludeme)
{
   char *ins;

   if(!pe) return sbuf;

   if (strchr(pe->Address,'@'))
   {
      ins = BuildAddrName2(pe);
   }
   else
   {
      char addr[SIZE_ADDRESS];
      char *p = strchr(C->EmailAddress, '@');

      snprintf(addr, sizeof(addr), "%s%s", pe->Address, p ? p : "");
      ins = BuildAddrName(addr, pe->RealName);
   }
   if (excludeme) if (!stricmp(pe->Address, C->EmailAddress)) return sbuf;
   if (stristr(sbuf, ins)) return sbuf;
   if (*sbuf) sbuf = StrBufCat(sbuf, ", ");
   return StrBufCat(sbuf, ins);
}

///
/// MA_CmpDate
//  Compares two messages by date
static int MA_CmpDate(struct Mail **pentry1, struct Mail **pentry2)
{
   return CompareDates(&(pentry2[0]->Date), &(pentry1[0]->Date));
}

///
/// MA_InsertIntroText
//  Inserts a phrase into the message text
static void MA_InsertIntroText(FILE *fh, char *text, struct ExpandTextData *etd)
{
  ENTER();

  if(*text)
  {
    char *sbuf = ExpandText(text, etd);
    fprintf(fh, "%s\n", sbuf);
    FreeStrBuf(sbuf);
  }

  LEAVE();
}

///
/// MA_EditorNotification
//  Starts file notification for temporary message file
static void MA_EditorNotification(int winnum)
{
   FileToEditor(G->WR_Filename[winnum], G->WR[winnum]->GUI.TE_EDIT);
   StartNotify(&G->WR_NRequest[winnum]);
   set(G->WR[winnum]->GUI.TE_EDIT, MUIA_TextEditor_HasChanged, FALSE);
}

///
/// MA_SetupQuoteString
//  Creates quote string by replacing variables with values
static void MA_SetupQuoteString(struct WR_ClassData *wr, struct ExpandTextData *etd, struct Mail *mail)
{
  struct ExpandTextData l_etd;
  char *sbuf;

  ENTER();

  if(!etd)
    etd = &l_etd;

  etd->OS_Name     = mail ? (*(mail->From.RealName) ? mail->From.RealName : mail->From.Address) : "";
  etd->OS_Address  = mail ? mail->From.Address : "";
  etd->OM_Subject  = mail ? mail->Subject : "";
  etd->OM_TimeZone = mail ? mail->tzone : C->TimeZone;
  etd->R_Name      = "";
  etd->R_Address   = "";

  // we have to copy the datestamp and eventually convert it
  // according to the timezone
  if(mail)
  {
    // the mail time is in UTC, so we have to convert it to the
    // actual time of the mail as we don't do any conversion
    // later on
    memcpy(&etd->OM_Date, &mail->Date, sizeof(struct DateStamp));

    if(mail->tzone != 0)
    {
      struct DateStamp *date = &etd->OM_Date;

      date->ds_Minute += mail->tzone;

      // we need to check the datestamp variable that it is still in it`s borders
      // after adjustment
      while(date->ds_Minute < 0)     { date->ds_Minute += 1440; date->ds_Days--; }
      while(date->ds_Minute >= 1440) { date->ds_Minute -= 1440; date->ds_Days++; }
    }
  }
  else
    memcpy(&etd->OM_Date, &G->StartDate, sizeof(struct DateStamp));

  sbuf = ExpandText(C->QuoteText, etd);
  strlcpy(wr->QuoteText, TrimEnd(sbuf), sizeof(wr->QuoteText));
  FreeStrBuf(sbuf);
  strlcpy(wr->AltQuoteText, C->AltQuoteText, sizeof(wr->AltQuoteText));

  LEAVE();
}

///
/// MA_CheckWriteWindow
//  Opens a write window
static int MA_CheckWriteWindow(int winnum)
{
   if (SafeOpenWindow(G->WR[winnum]->GUI.WI)) return winnum;
   WR_Cleanup(winnum);
   DisposeModulePush(&G->WR[winnum]);
   return -1;
}

///
/// MA_NewNew
//  Creates a new, empty message
int MA_NewNew(struct Mail *mail, int flags)
{
   BOOL quiet = hasQuietFlag(flags);
   struct Folder *folder = FO_GetCurrentFolder();
   int winnum = -1;
   struct WR_ClassData *wr;
   FILE *out;

   if(!folder) return(-1);

   /* First check if the basic configuration is okay, then open write window */
   if (CO_IsValid()) if ((winnum = WR_Open(quiet ? 2 : -1, FALSE)) >= 0)
   {
      if ((out = fopen(G->WR_Filename[winnum], "w")))
      {
         wr = G->WR[winnum];
         wr->Mode = NEW_NEW;
         wr->Mail = mail;
         if (mail) setstring(wr->GUI.ST_TO, BuildAddrName2(GetReturnAddress(mail)));
         else if(folder->MLSupport)
         {
            if(folder->MLAddress[0]) setstring(wr->GUI.ST_TO, folder->MLAddress);
            if(folder->MLFromAddress[0]) setstring(wr->GUI.ST_FROM, folder->MLFromAddress);
            if(folder->MLReplyToAddress[0]) setstring(wr->GUI.ST_REPLYTO, folder->MLReplyToAddress);
         }

         MA_SetupQuoteString(wr, NULL, NULL);

         if(folder->WriteIntro[0]) MA_InsertIntroText(out, folder->WriteIntro, NULL);
         else MA_InsertIntroText(out, C->NewIntro, NULL);
         if(folder->WriteGreetings[0]) MA_InsertIntroText(out, folder->WriteGreetings, NULL);
         else MA_InsertIntroText(out, C->Greetings, NULL);
         fclose(out);

         // add a signature to the mail depending on the selected signature for this list
         WR_AddSignature(winnum, folder->MLSupport ? folder->MLSignature: -1);

         if (!quiet) set(wr->GUI.WI, MUIA_Window_Open, TRUE);
         MA_EditorNotification(winnum);
         set(wr->GUI.WI, MUIA_Window_ActiveObject, wr->GUI.ST_TO);
         if (C->LaunchAlways && !quiet) DoMethod(G->App, MUIM_CallHook, &WR_EditHook, winnum);
      }
      else
      {
        DisposeModulePush(&G->WR[winnum]);
      }
   }
   if (winnum >= 0 && !quiet) return MA_CheckWriteWindow(winnum);
   return winnum;
}

///
/// MA_NewEdit
//  Edits a message
int MA_NewEdit(struct Mail *mail, int flags, Object *readWindow)
{
   BOOL quiet = hasQuietFlag(flags);
   int i, winnum = -1;
   struct Folder *folder;
   struct WR_ClassData *wr;
   struct ExtendedMail *email;
   FILE *out;
   char *cmsg, *sbuf;

   // return if mail is already being written/edited
   for(i = 0; i < MAXWR; i++)
   {
     if(G->WR[i] && G->WR[i]->Mail == mail)
     {
       DoMethod(G->WR[i]->GUI.WI, MUIM_Window_ToFront);
       return -1;
     }
   }

   // check if necessary settings fror writing are OK and open new window
   if(CO_IsValid() && (winnum = WR_Open(quiet ? 2 : -1, FALSE)) >= 0)
   {
      if((out = fopen(G->WR_Filename[winnum], "w")))
      {
         struct ReadMailData *rmData;

         wr = G->WR[winnum];
         wr->Mode = NEW_EDIT;
         wr->Mail = mail;
         wr->readWindow = readWindow;
         folder = mail->Folder;

         if(!(email = MA_ExamineMail(folder, mail->MailFile, TRUE)))
         {
            ER_NewError(GetStr(MSG_ER_CantOpenFile), GetMailFile(NULL, folder, mail));
            fclose(out);
            DisposeModulePush(&G->WR[winnum]);
            return winnum;
         }

         MA_SetupQuoteString(wr, NULL, mail);

         if((rmData = AllocPrivateRMData(mail, PM_ALL)))
         {
            if((cmsg = RE_ReadInMessage(rmData, RIM_EDIT)))
            {
              int msglen = strlen(cmsg);

              // we check whether cmsg contains any text and if so we
              // write out the whole text to our temporary file.
              if(msglen == 0 || fwrite(cmsg, msglen, 1, out) == 1)
              {
                // free our temp text now
                free(cmsg);

                strlcpy(wr->MsgID, email->IRTMsgID, sizeof(wr->MsgID));
                setstring(wr->GUI.ST_SUBJECT, mail->Subject);
                setstring(wr->GUI.ST_FROM, BuildAddrName2(&mail->From));
                setstring(wr->GUI.ST_REPLYTO, BuildAddrName2(&mail->ReplyTo));
                sbuf = StrBufCpy(NULL, BuildAddrName2(&mail->To));

                if(isMultiRCPTMail(mail))
                {
                  *sbuf = '\0';
                  sbuf = MA_AppendRcpt(sbuf, &mail->To, FALSE);

                  for(i = 0; i < email->NoSTo; i++)
                    sbuf = MA_AppendRcpt(sbuf, &email->STo[i], FALSE);

                  setstring(wr->GUI.ST_TO, sbuf);
                  *sbuf = '\0';

                  for(i = 0; i < email->NoCC; i++)
                    sbuf = MA_AppendRcpt(sbuf, &email->CC[i], FALSE);

                  setstring(wr->GUI.ST_CC, sbuf);
                  *sbuf = '\0';

                  for(i = 0; i < email->NoBCC; i++)
                    sbuf = MA_AppendRcpt(sbuf, &email->BCC[i], FALSE);

                  setstring(wr->GUI.ST_BCC, sbuf);
                }
                else
                  setstring(wr->GUI.ST_TO, sbuf);

                FreeStrBuf(sbuf);
                if(email->extraHeaders)
                  setstring(wr->GUI.ST_EXTHEADER, email->extraHeaders);

                setcheckmark(wr->GUI.CH_DELSEND, email->DelSend);
                setcheckmark(wr->GUI.CH_RECEIPT, email->RetRcpt);
                setcheckmark(wr->GUI.CH_DISPNOTI, email->ReceiptType == RCPT_TYPE_ALL);
                setcheckmark(wr->GUI.CH_ADDINFO, isSenderInfoMail(mail));
                setcycle(wr->GUI.CY_IMPORTANCE, getImportanceLevel(mail) == IMP_HIGH ? 0 : getImportanceLevel(mail)+1);
                setmutex(wr->GUI.RA_SIGNATURE, email->Signature);
                setmutex(wr->GUI.RA_SECURITY, wr->OldSecurity = email->Security);

                if(folder->Type != FT_OUTGOING)
                  DoMethod(G->App, MUIM_MultiSet, MUIA_Disabled, TRUE, wr->GUI.BT_SEND, wr->GUI.BT_HOLD, NULL);

                WR_SetupOldMail(winnum, rmData);
              }
              else
              {
                E(DBF_MAIL, "Error while writing cmsg to out FH");

                // an error occurred while trying to write the text to out
                free(cmsg);
                FreePrivateRMData(rmData);
                fclose(out);
                MA_FreeEMailStruct(email);
                DisposeModulePush(&G->WR[winnum]);
                return winnum;
              }
            }

            FreePrivateRMData(rmData);
         }

         fclose(out);
         MA_FreeEMailStruct(email);

         if(!quiet)
           set(wr->GUI.WI, MUIA_Window_Open, TRUE);

         MA_EditorNotification(winnum);
         sbuf = (STRPTR)xget(wr->GUI.ST_TO, MUIA_String_Contents);
         set(wr->GUI.WI, MUIA_Window_ActiveObject, *sbuf ? wr->GUI.TE_EDIT : wr->GUI.ST_TO);

         if(C->LaunchAlways && !quiet)
           DoMethod(G->App, MUIM_CallHook, &WR_EditHook, winnum);
      }
      else
        DisposeModulePush(&G->WR[winnum]);
   }

   if(winnum >= 0 && !quiet)
     return MA_CheckWriteWindow(winnum);

   return winnum;
}

///
/// MA_NewBounce
//  Bounces a message
int MA_NewBounce(struct Mail *mail, int flags)
{
   BOOL quiet = hasQuietFlag(flags);
   int winnum = -1;
   struct WR_ClassData *wr;

   if (CO_IsValid()) if ((winnum = WR_Open(quiet ? 2 : -1, TRUE)) >= 0)
   {
      wr = G->WR[winnum];
      wr->Mode = NEW_BOUNCE;
      wr->Mail = mail;
      if (!quiet) set(wr->GUI.WI, MUIA_Window_Open, TRUE);
      set(wr->GUI.WI, MUIA_Window_ActiveObject, wr->GUI.ST_TO);
   }
   if (winnum >= 0 && !quiet) return MA_CheckWriteWindow(winnum);
   return winnum;
}

///
/// MA_NewForward
//  Forwards a list of messages
int MA_NewForward(struct Mail **mlist, int flags)
{
   BOOL quiet = hasQuietFlag(flags);
   char buffer[SIZE_LARGE];
   int i, winnum = -1, mlen = (2+(int)mlist[0])*sizeof(struct Mail *);
   struct WR_ClassData *wr;
   struct Mail *mail = NULL;
   struct ExtendedMail *email;
   struct ExpandTextData etd;
   FILE *out;
   char *cmsg, *rsub;

   if(CO_IsValid() && (winnum = WR_Open(quiet ? 2 : -1, FALSE)) >= 0)
   {
      if ((out = fopen(G->WR_Filename[winnum], "w")))
      {
         wr = G->WR[winnum];
         wr->Mode = NEW_FORWARD;
         wr->MList = memcpy(malloc(mlen), mlist, mlen);
         rsub = AllocStrBuf(SIZE_SUBJECT);
         qsort(&mlist[2], (int)mlist[0], sizeof(struct Mail *), (int (*)(const void *, const void *))MA_CmpDate);
         MA_InsertIntroText(out, C->NewIntro, NULL);
         for (i = 0; i < (int)mlist[0]; i++)
         {
            struct ReadMailData *rmData;

            mail = mlist[i+2];

            if(!(email = MA_ExamineMail(mail->Folder, mail->MailFile, TRUE)))
            {
              ER_NewError(GetStr(MSG_ER_CantOpenFile), GetMailFile(NULL, mail->Folder, mail));
              fclose(out);
              FreeStrBuf(rsub);
              DisposeModulePush(&G->WR[winnum]);
              return winnum;
            }

            MA_SetupQuoteString(wr, &etd, &email->Mail);
            etd.OM_MessageID = email->MsgID;
            etd.R_Name = *mail->To.RealName ? mail->To.RealName : mail->To.Address;
            etd.R_Address = mail->To.Address;

            if (*mail->Subject)
            {
               snprintf(buffer, sizeof(buffer), "%s (fwd)", mail->Subject);
               if (!strstr(rsub, buffer))
               {
                  if (*rsub) rsub = StrBufCat(rsub, "; ");
                  rsub = StrBufCat(rsub, buffer);
               }
            }

            if((rmData = AllocPrivateRMData(mail, PM_ALL)))
            {
              etd.HeaderFile = rmData->firstPart->Filename;
              MA_InsertIntroText(out, C->ForwardIntro, &etd);
              MA_FreeEMailStruct(email);

              if((cmsg = RE_ReadInMessage(rmData, RIM_EDIT)))
              {
                fputs(cmsg, out);
                free(cmsg);

                MA_InsertIntroText(out, C->ForwardFinish, &etd);

                if(!hasNoAttachFlag(flags))
                  WR_SetupOldMail(winnum, rmData);
              }

              FreePrivateRMData(rmData);
            }
         }
         MA_InsertIntroText(out, C->Greetings, NULL);
         fclose(out);

         // add a signature to the mail depending on the selected signature for this list
         WR_AddSignature(winnum, (mail->Folder && mail->Folder->MLSupport) ? mail->Folder->MLSignature: -1);

         setstring(wr->GUI.ST_SUBJECT, rsub);
         FreeStrBuf(rsub);
         if (!quiet) set(wr->GUI.WI, MUIA_Window_Open, TRUE);
         MA_EditorNotification(winnum);
         set(wr->GUI.WI, MUIA_Window_ActiveObject, wr->GUI.ST_TO);
         if (C->LaunchAlways && !quiet) DoMethod(G->App, MUIM_CallHook, &WR_EditHook, winnum);
      }
      else
        DisposeModulePush(&G->WR[winnum]);
   }
   if (winnum >= 0 && !quiet) return MA_CheckWriteWindow(winnum);
   return winnum;
}

///
/// MA_NewReply
//  Creates a reply to a list of messages
int MA_NewReply(struct Mail **mlist, int flags)
{
   int j, i, repmode = 1, winnum = -1, mlen = (2+(int)mlist[0])*sizeof(struct Mail *);
   BOOL doabort = FALSE, multi = (int)mlist[0] > 1, altpat = FALSE, quiet = hasQuietFlag(flags);
   struct WR_ClassData *wr;
   struct Mail *mail;
   struct ExtendedMail *email;
   struct ExpandTextData etd;
   struct Person *repto, rtml;
   struct Folder *folder = NULL;
   FILE *out;
   char *mlistad = NULL, buffer[SIZE_LARGE];
   char *cmsg, *rfrom = NULL, *rrepto = NULL, *rto = NULL, *rcc = NULL, *rsub = NULL;
   char *domain;

   if (CO_IsValid() && (winnum = WR_Open(quiet ? 2 : -1, FALSE)) >= 0)
   {
      if ((out = fopen(G->WR_Filename[winnum], "w")))
      {
         wr = G->WR[winnum];
         wr->Mode = NEW_REPLY;
         wr->MList = memcpy(malloc(mlen), mlist, mlen);
         rto = AllocStrBuf(SIZE_ADDRESS);
         rcc = AllocStrBuf(SIZE_ADDRESS);
         rsub = AllocStrBuf(SIZE_SUBJECT);
         qsort(&mlist[2], (int)mlist[0], sizeof(struct Mail *), (int (*)(const void *, const void *))MA_CmpDate);

         // Now we iterate through all selected mails
         for (j = 0; j < (int)mlist[0]; j++)
         {
            mail = mlist[j+2];
            folder = mail->Folder;

            if(!(email = MA_ExamineMail(folder, mail->MailFile, TRUE)))
            {
              ER_NewError(GetStr(MSG_ER_CantOpenFile), GetMailFile(NULL, folder, mail));
              fclose(out);
              DisposeModulePush(&G->WR[winnum]);
              FreeStrBuf(rto);
              FreeStrBuf(rcc);
              FreeStrBuf(rsub);
              return winnum;
            }

            MA_SetupQuoteString(wr, &etd, &email->Mail);
            etd.OM_MessageID = email->MsgID;

            // If this mail already have a subject we are going to add a "Re:" to it.
            if (*mail->Subject)
            {
               if(j)
                strlcpy(buffer, mail->Subject, sizeof(buffer));
               else
                snprintf(buffer, sizeof(buffer), "Re: %s", MA_GetRealSubject(mail->Subject));

               if (!strstr(rsub, buffer))
               {
                  if (*rsub) rsub = StrBufCat(rsub, "; ");
                  rsub = StrBufCat(rsub, buffer);
               }
            }

            if(!multi)
              strlcpy(wr->MsgID, email->MsgID, sizeof(wr->MsgID));

            // Now we analyse the folder of the selected mail and if it
            // is a mailing list we have to do some operation
            if (folder)
            {
               char tofld[SIZE_LARGE], fromfld[SIZE_LARGE];

               strlcpy(tofld, BuildAddrName2(&mail->To), sizeof(tofld));
               strlcpy(fromfld, BuildAddrName2(&mail->From), sizeof(fromfld));

               // if the mail we are going to reply resists in the incoming folder
               // we have to check all other folders first.
               if (folder->Type == FT_INCOMING)
               {
                  struct Folder **flist;

                  if ((flist = FO_CreateList()))
                  {
                     for (i = 1; i <= (int)*flist; i++)
                     {
                       if (flist[i]->MLSupport && flist[i]->MLPattern[0] && MatchNoCase(tofld, flist[i]->MLPattern))
                       {
                          mlistad = flist[i]->MLAddress[0] ? flist[i]->MLAddress : fromfld;
                          folder = flist[i];
                          if (flist[i]->MLFromAddress[0])    rfrom  = flist[i]->MLFromAddress;
                          if (flist[i]->MLReplyToAddress[0]) rrepto = flist[i]->MLReplyToAddress;
                          break;
                       }
                     }
                     free(flist);
                  }
               }
               else if (folder->MLSupport && folder->MLPattern[0] && MatchNoCase(tofld, folder->MLPattern))
               {
                  mlistad = folder->MLAddress[0] ? folder->MLAddress : fromfld;
                  if (folder->MLFromAddress[0])    rfrom  = folder->MLFromAddress;
                  if (folder->MLReplyToAddress[0]) rrepto = folder->MLReplyToAddress;
               }
            }

            if(mlistad && !hasPrivateFlag(flags) && !hasMListFlag(flags))
            {
               ExtractAddress(mlistad, repto = &rtml);
               if (!strstr(rto, mlistad))
               {
                  if (*rto) rto = StrBufCat(rto, ", ");
                  rto = StrBufCat(rto, mlistad);
               }
            }
            else repto = GetReturnAddress(mail);

            // If this mail is a standard (non-ML) mail and the user hasn`t pressed shift
            if(isMultiRCPTMail(mail) && !hasPrivateFlag(flags) && !hasMListFlag(flags))
            {
              if (!(repmode = MUI_Request(G->App, G->MA->GUI.WI, 0, NULL, GetStr(MSG_MA_ReplyReqOpt), GetStr(MSG_MA_ReplyReq))))
              {
                MA_FreeEMailStruct(email);
                fclose(out);
                DisposeModulePush(&G->WR[winnum]);
                FreeStrBuf(rto);
                FreeStrBuf(rcc);
                FreeStrBuf(rsub);
                return winnum;
              }
            }

            if (repmode == 1)
            {
               if(hasPrivateFlag(flags)) repto = &mail->From;
               else if(hasMListFlag(flags) || mlistad) ; // do nothing
               else if (C->CompareAddress && *mail->ReplyTo.Address && stricmp(mail->From.Address, mail->ReplyTo.Address))
               {
                  snprintf(buffer, sizeof(buffer), GetStr(MSG_MA_CompareReq), mail->From.Address, mail->ReplyTo.Address);
                  switch (MUI_Request(G->App, G->MA->GUI.WI, 0, NULL, GetStr(MSG_MA_Compare3ReqOpt), buffer))
                  {
                     case 3:
                     {
                        rcc = MA_AppendRcpt(rcc, &mail->From, FALSE);
                     }
                     // continue

                     case 2:
                     {
                        repto = &mail->ReplyTo;
                     }
                     break;

                     case 1:
                     {
                        repto = &mail->From;
                     }
                     break;

                     case 0:
                     {
                        MA_FreeEMailStruct(email);
                        fclose(out);
                        DisposeModulePush(&G->WR[winnum]);
                        FreeStrBuf(rto);
                        FreeStrBuf(rcc);
                        FreeStrBuf(rsub);
                        return winnum;
                     }
                  }
               }
               rto = MA_AppendRcpt(rto, repto, FALSE);
            }
            else
            {
               if (repmode == 2) rto = MA_AppendRcpt(rto, GetReturnAddress(mail), FALSE);
               rto = MA_AppendRcpt(rto, &mail->To, TRUE);
               for (i = 0; i < email->NoSTo; i++) rto = MA_AppendRcpt(rto, &email->STo[i], TRUE);
               for (i = 0; i < email->NoCC; i++) rcc = MA_AppendRcpt(rcc, &email->CC[i], TRUE);
            }

            etd.R_Name = repto->RealName;
            etd.R_Address = repto->Address;
            altpat = FALSE;
            if (!(domain = strchr(repto->Address,'@'))) domain = strchr(C->EmailAddress,'@');
            if (*C->AltReplyPattern) if (MatchNoCase(domain, C->AltReplyPattern)) altpat = TRUE;
            if (!j) MA_InsertIntroText(out, mlistad ? C->MLReplyHello : (altpat ? C->AltReplyHello : C->ReplyHello), &etd);
            if(C->QuoteMessage && !hasNoQuoteFlag(flags))
            {
               struct ReadMailData *rmData;

               if(j)
                 fputc('\n', out);

               if((rmData = AllocPrivateRMData(mail, PM_TEXTS)))
               {
                 etd.HeaderFile = rmData->firstPart->Filename;
                 MA_InsertIntroText(out, mlistad ? C->MLReplyIntro : (altpat ? C->AltReplyIntro : C->ReplyIntro), &etd);
                 if((cmsg = RE_ReadInMessage(rmData, RIM_QUOTE)))
                 {
                   Quote_Text(out, cmsg, strlen(cmsg), C->EdWrapMode ? C->EdWrapCol-strlen(wr->QuoteText)-1 : 1024, wr->QuoteText);
                   free(cmsg);
                 }

                 FreePrivateRMData(rmData);
               }
            }
            MA_FreeEMailStruct(email);
         }
         MA_InsertIntroText(out, mlistad ? C->MLReplyBye : (altpat ? C->AltReplyBye: C->ReplyBye), &etd);
         fclose(out);

         // now we add the configured signature to the reply
         WR_AddSignature(winnum, (folder && folder->MLSupport) ? folder->MLSignature: -1);

         /* If this is a reply to a mail belonging to a mailing list,
            set the "From:" and "Reply-To:" addresses accordingly */
         if (rfrom)  setstring(wr->GUI.ST_FROM,    rfrom);
         if (rrepto) setstring(wr->GUI.ST_REPLYTO, rrepto);

         setstring(wr->GUI.ST_TO, rto);
         setstring(*rto ? wr->GUI.ST_CC : wr->GUI.ST_TO, rcc);
         setstring(wr->GUI.ST_SUBJECT, rsub);
         if (!quiet) set(wr->GUI.WI, MUIA_Window_Open, TRUE);
         MA_EditorNotification(winnum);
         set(wr->GUI.WI, MUIA_Window_ActiveObject, wr->GUI.TE_EDIT);
         if (C->LaunchAlways && !quiet) DoMethod(G->App, MUIM_CallHook, &WR_EditHook, winnum);
      }
      else doabort = TRUE;
   }
   if (winnum >= 0 && !quiet && !doabort) return MA_CheckWriteWindow(winnum);

   if (doabort)
   {
    DisposeModulePush(&G->WR[winnum]);
   }
   FreeStrBuf(rto);
   FreeStrBuf(rcc);
   FreeStrBuf(rsub);
   return winnum;
}

///
/// MA_RemoveAttach
//  Removes attachments from a message
void MA_RemoveAttach(struct Mail *mail, BOOL warning)
{
   struct ReadMailData *rmData;

   // if we need to warn the user of this operation we put up a requester
   // before we go on
   if(warning &&
      MUI_Request(G->App, G->MA->GUI.WI, 0, NULL, GetStr(MSG_YesNoReq2), GetStr(MSG_MA_CROPREQUEST)) == 0)
   {
     return;
   }

   if((rmData = AllocPrivateRMData(mail, PM_ALL)))
   {
     struct Part *part;
     char *cmsg;
     char buf[SIZE_LINE];
     char fname[SIZE_PATHFILE];
     char tfname[SIZE_PATHFILE];

     snprintf(tfname, sizeof(tfname), "%s.tmp", GetMailFile(fname, NULL, mail));

     if((cmsg = RE_ReadInMessage(rmData, RIM_QUIET)))
     {
       if((part = rmData->firstPart->Next) && part->Next)
       {
          FILE *out;

          if((out = fopen(tfname, "w")))
          {
             FILE *in;
             struct Folder *fo = mail->Folder;
             int f;

             if((in = fopen(rmData->firstPart->Filename, "r")))
             {
                BOOL infield = FALSE, inbody = FALSE;

                while(fgets(buf, SIZE_LINE, in))
                {
                  if(!isspace(*buf))
                    infield = !strnicmp(buf, "content-transfer-encoding", 25) || !strnicmp(buf, "content-type", 12);

                  if(!infield || inbody)
                    fputs(buf, out);
                }
                fclose(in);
             }

             fputs("Content-Transfer-Encoding: 8bit\nContent-Type: text/plain; charset=iso-8859-1\n\n", out);
             fputs(cmsg, out);
             MA_ExpireIndex(fo);
             fputs(GetStr(MSG_MA_AttachRemoved), out);

             for(part = part->Next; part; part = part->Next)
             {
               fprintf(out, "%s (%ld %s, %s)\n", part->Name ? part->Name : GetStr(MSG_Unnamed),
                                                 part->Size,
                                                 GetStr(MSG_Bytes),
                                                 part->ContentType);
             }

             fclose(out);

             f = FileSize(tfname);
             fo->Size += f - mail->Size;
             mail->Size = f;

             CLEAR_FLAG(mail->mflags, MFLAG_MP_MIXED);
             SET_FLAG(rmData->mail->Folder->Flags, FOFL_MODIFY);  // flag folder as modified
              DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_MainMailListGroup_RedrawMail, mail);

             DeleteFile(fname);

             if(fo->Mode > FM_SIMPLE)
               DoPack(tfname, fname, fo);
             else
               RenameFile(tfname, fname);

             AppendLog(81, GetStr(MSG_LOG_CroppingAtt), mail->MailFile, fo->Name, "", "");
          }
       }

       free(cmsg);
     }

     FreePrivateRMData(rmData);
   }
}

///
/// MA_RemoveAttachFunc
//  Removes attachments from selected messages
HOOKPROTONHNONP(MA_RemoveAttachFunc, void)
{
   struct Mail **mlist;
   int i;

   // we need to warn the user of this operation we put up a requester
   // before we go on
   if(MUI_Request(G->App, G->MA->GUI.WI, 0, NULL, GetStr(MSG_YesNoReq2), GetStr(MSG_MA_CROPREQUEST)) == 0)
      return;

   if ((mlist = MA_CreateMarkedList(G->MA->GUI.PG_MAILLIST, FALSE)))
   {
      int selected = (int)*mlist;
      BusyGauge(GetStr(MSG_BusyRemovingAtt), "", selected);
      for (i = 0; i < selected; i++)
      {
         MA_RemoveAttach(mlist[i+2], FALSE);
         BusySet(i+1);
      }
      DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_NList_Redraw, MUIV_NList_Redraw_All);
      MA_ChangeSelected(TRUE);
      DisplayStatistics(NULL, TRUE);
      BusyEnd();
   }
}
MakeHook(MA_RemoveAttachHook, MA_RemoveAttachFunc);

///
/// MA_SaveAttachFunc
//  Saves all attachments of selected messages to disk
HOOKPROTONHNONP(MA_SaveAttachFunc, void)
{
   struct Mail **mlist;

   if((mlist = MA_CreateMarkedList(G->MA->GUI.PG_MAILLIST, FALSE)))
   {
      if(ReqFile(ASL_DETACH, G->MA->GUI.WI, GetStr(MSG_RE_SaveMessage), (REQF_SAVEMODE|REQF_DRAWERSONLY), C->DetachDir, ""))
      {
         int i;

         BusyText(GetStr(MSG_BusyDecSaving), "");

         for (i = 0; i < (int)*mlist; i++)
         {
            struct ReadMailData *rmData;

            if((rmData = AllocPrivateRMData(mlist[i+2], PM_ALL)))
            {
              char *cmsg;

              if((cmsg = RE_ReadInMessage(rmData, RIM_QUIET)))
              {
                struct Part *part;

                // free the message again as we don't need its content here.
                free(cmsg);

                if((part = rmData->firstPart->Next) && part->Next)
                {
                  RE_SaveAll(rmData, G->ASLReq[ASL_DETACH]->fr_Drawer);
                }
              }

              FreePrivateRMData(rmData);
            }
         }

         BusyEnd();
      }
      free(mlist);
   }
}
MakeHook(MA_SaveAttachHook, MA_SaveAttachFunc);

///
/// MA_SavePrintFunc
//  Prints selected messages
HOOKPROTONHNO(MA_SavePrintFunc, void, int *arg)
{
   BOOL doprint = (*arg != 0);
   struct Mail **mlist;

   if(doprint && C->PrinterCheck && !CheckPrinter())
     return;

   if((mlist = MA_CreateMarkedList(G->MA->GUI.PG_MAILLIST, FALSE)))
   {
      int i;

      for(i = 0; i < (int)*mlist; i++)
      {
         struct ReadMailData *rmData;

         if((rmData = AllocPrivateRMData(mlist[i+2], PM_TEXTS)))
         {
            char *cmsg;

            if((cmsg = RE_ReadInMessage(rmData, RIM_PRINT)))
            {
               struct TempFile *tf;

               if((tf = OpenTempFile("w")))
               {
                  fputs(cmsg, tf->FP);
                  fclose(tf->FP); tf->FP = NULL;

                  if(doprint)
                    CopyFile("PRT:", 0, tf->Filename, 0);
                  else
                    RE_Export(rmData, tf->Filename, "", "", 0, FALSE, FALSE, IntMimeTypeArray[MT_TX_PLAIN].ContentType);

                  CloseTempFile(tf);
               }

               free(cmsg);
            }

            FreePrivateRMData(rmData);
         }
      }

      free(mlist);
   }
}
MakeHook(MA_SavePrintHook, MA_SavePrintFunc);

///
/// MA_NewMessage
//  Starts a new message
int MA_NewMessage(int mode, int flags)
{
   struct Mail *mail, **mlist = NULL;
   int winnr = -1;
   switch (mode)
   {
      case NEW_NEW:     winnr = MA_NewNew(NULL, flags);
                        break;
      case NEW_EDIT:    if ((mail = MA_GetActiveMail(ANYBOX, NULL, NULL))) winnr = MA_NewEdit(mail, flags, NULL);
                        break;
      case NEW_BOUNCE:  if ((mail = MA_GetActiveMail(ANYBOX, NULL, NULL))) winnr = MA_NewBounce(mail, flags);
                        break;
      case NEW_FORWARD: if ((mlist = MA_CreateMarkedList(G->MA->GUI.PG_MAILLIST, FALSE))) winnr = MA_NewForward(mlist, flags);
                        break;
      case NEW_REPLY:   if ((mlist = MA_CreateMarkedList(G->MA->GUI.PG_MAILLIST, FALSE))) winnr = MA_NewReply(mlist, flags);
                        break;
   }
   if (mlist) free(mlist);
   return winnr;
}

///
/// MA_NewMessageFunc
HOOKPROTONHNO(MA_NewMessageFunc, void, int *arg)
{
   int mode = arg[0], flags = 0;
   ULONG qual = arg[1];

   if (mode == NEW_FORWARD &&   hasFlag(qual, (IEQUALIFIER_LSHIFT|IEQUALIFIER_RSHIFT))) mode = NEW_BOUNCE;
   if (mode == NEW_FORWARD && isFlagSet(qual, IEQUALIFIER_CONTROL))                     SET_FLAG(flags, NEWF_FWD_NOATTACH);
   if (mode == NEW_REPLY   &&   hasFlag(qual, (IEQUALIFIER_LSHIFT|IEQUALIFIER_RSHIFT))) SET_FLAG(flags, NEWF_REP_PRIVATE);
   if (mode == NEW_REPLY   &&   hasFlag(qual, (IEQUALIFIER_LALT|IEQUALIFIER_RALT)))     SET_FLAG(flags, NEWF_REP_MLIST);
   if (mode == NEW_REPLY   && isFlagSet(qual, IEQUALIFIER_CONTROL))                     SET_FLAG(flags, NEWF_REP_NOQUOTE);

   MA_NewMessage(mode, flags);
}
MakeHook(MA_NewMessageHook, MA_NewMessageFunc);

///
/// MA_DeleteMessage
//  Deletes selected messages
void MA_DeleteMessage(BOOL delatonce, BOOL force)
{
   struct Mail **mlist, *mail;
   int i, selected;
   APTR lv = G->MA->GUI.PG_MAILLIST;
   char buffer[SIZE_DEFAULT];
   struct Folder *delfolder = FO_GetFolderByType(FT_DELETED, NULL), *folder = FO_GetCurrentFolder();
   BOOL ignoreall = FALSE;

   if(!folder || !delfolder) return;

   if (!(mlist = MA_CreateMarkedList(lv, FALSE))) return;
   selected = (int)*mlist;
   if (C->Confirm && selected >= C->ConfirmDelete && !force)
   {
      SPrintF(buffer, selected==1 ? GetStr(MSG_MA_1Selected) : GetStr(MSG_MA_xSelected), selected);
      strlcat(buffer, GetStr(MSG_MA_ConfirmDel), sizeof(buffer));

      if (!MUI_Request(G->App, G->MA->GUI.WI, 0, GetStr(MSG_MA_ConfirmReq), GetStr(MSG_OkayCancelReq), buffer))
      {
         free(mlist);
         return;
      }
   }
   set(lv, MUIA_NList_Quiet, TRUE);

   BusyGauge(GetStr(MSG_BusyDeleting), itoa(selected), selected);
   for (i = 0; i < selected; i++)
   {
      mail = mlist[i+2];
      if(isSendMDNMail(mail) && !ignoreall &&
         (hasStatusNew(mail) || !hasStatusRead(mail)))
      {
        ignoreall = RE_DoMDN(MDN_DELE, mail, TRUE);
      }

      // call our subroutine with quiet option
      MA_DeleteSingle(mail, delatonce, TRUE);

      BusySet(i+1);
   }
   BusyEnd();
   set(lv, MUIA_NList_Quiet, FALSE);
   free(mlist);

   if (delatonce || C->RemoveAtOnce || folder == delfolder)
   {
      AppendLogNormal(20, GetStr(MSG_LOG_Deleting), (void *)selected, folder->Name, "", "");
   }
   else
   {
      AppendLogNormal(22, GetStr(MSG_LOG_Moving), (void *)selected, folder->Name, delfolder->Name, "");
      DisplayStatistics(delfolder, FALSE);
   }
   DisplayStatistics(NULL, TRUE);
   MA_ChangeSelected(FALSE);
}

///
/// MA_DeleteMessageFunc
HOOKPROTONHNO(MA_DeleteMessageFunc, void, int *arg)
{
   BOOL delatonce = hasFlag(arg[0], (IEQUALIFIER_LSHIFT|IEQUALIFIER_RSHIFT));
   MA_DeleteMessage(delatonce, FALSE);
}
MakeHook(MA_DeleteMessageHook, MA_DeleteMessageFunc);

///
/// MA_DelKey
//  User pressed DEL key
HOOKPROTONHNO(MA_DelKeyFunc, void, int *arg)
{
   Object *actobj = (Object *)xget(G->MA->GUI.WI, MUIA_Window_ActiveObject);

   if(!actobj || actobj == MUIV_Window_ActiveObject_None)
    actobj = (Object *)xget(G->MA->GUI.WI, MUIA_Window_DefaultObject);

   if(actobj == G->MA->GUI.LV_FOLDERS)
   {
      CallHookPkt(&FO_DeleteFolderHook, 0, 0);
   }
   else
   {
      MA_DeleteMessage(arg[0], FALSE);
   }
}
MakeStaticHook(MA_DelKeyHook, MA_DelKeyFunc);

///
/// MA_GetAddressSelect
//  Asks user which address (from/replyto) to store
static struct Person *MA_GetAddressSelect(struct Mail *mail)
{
   struct Person *pe = GetReturnAddress(mail);
   if (C->CompareAddress && *mail->ReplyTo.Address) if (stricmp(mail->From.Address, mail->ReplyTo.Address))
   {
      char buffer[SIZE_LARGE];
      snprintf(buffer, sizeof(buffer), GetStr(MSG_MA_CompareReq), mail->From.Address, mail->ReplyTo.Address);
      switch (MUI_Request(G->App, G->MA->GUI.WI, 0, NULL, GetStr(MSG_MA_Compare2ReqOpt), buffer))
      {
         case 2: pe = &mail->ReplyTo; break;
         case 1: pe = &mail->From; break;
         case 0: pe = NULL;
      }
   }
   return pe;
}

///
/// MA_GetAddress
//  Stores address from a list of messages to the address book
void MA_GetAddress(struct Mail **mlist)
{
   int i, j, mode, winnum, num = (int)mlist[0];
   struct Folder *folder = mlist[2]->Folder;
   BOOL outgoing = folder ? isOutgoingFolder(folder) : FALSE;
   struct ExtendedMail *email;
   struct Person *pe = NULL;

   if (num == 1 && !(outgoing && isMultiRCPTMail(mlist[2])))
   {
      if (outgoing)
      {
         pe = &mlist[2]->To;
      }
      else
      {
         pe = MA_GetAddressSelect(mlist[2]);
         if (!pe)
            return;
      }
      mode = AET_USER;
   }
   else
      mode = AET_LIST;

   DoMethod(G->App, MUIM_CallHook, &AB_OpenHook, ABM_EDIT);

   winnum = EA_Init(mode, NULL);
   if (winnum >= 0)
   {
      if (mode == AET_USER)
      {
        setstring(G->EA[winnum]->GUI.ST_REALNAME, pe->RealName);
        setstring(G->EA[winnum]->GUI.ST_ADDRESS, pe->Address);
      }
      else
      {
         for (i = 2; i < num+2; i++)
         {
            if (outgoing)
            {
               DoMethod(G->EA[winnum]->GUI.LV_MEMBER, MUIM_List_InsertSingle, BuildAddrName2(&mlist[i]->To), MUIV_List_Insert_Bottom);
               if(isMultiRCPTMail(mlist[i]) &&
                  (email = MA_ExamineMail(mlist[i]->Folder, mlist[i]->MailFile, TRUE)))
               {
                  for(j = 0; j < email->NoSTo; j++)
                    DoMethod(G->EA[winnum]->GUI.LV_MEMBER, MUIM_List_InsertSingle, BuildAddrName2(&email->STo[j]), MUIV_List_Insert_Bottom);

                  for(j = 0; j < email->NoCC; j++)
                    DoMethod(G->EA[winnum]->GUI.LV_MEMBER, MUIM_List_InsertSingle, BuildAddrName2(&email->CC[j]), MUIV_List_Insert_Bottom);

                  MA_FreeEMailStruct(email);
               }
            }
            else
            {
               struct Person *pe1 = GetReturnAddress(mlist[i]);
               DoMethod(G->EA[winnum]->GUI.LV_MEMBER, MUIM_List_InsertSingle, BuildAddrName2(pe1), MUIV_List_Insert_Bottom);
            }
         }
      }
   }
}

///
/// MA_GetAddressFunc
//  Stores addresses from selected messages to the address book
HOOKPROTONHNONP(MA_GetAddressFunc, void)
{
   struct Mail **mlist = MA_CreateMarkedList(G->MA->GUI.PG_MAILLIST, FALSE);
   if (mlist)
   {
      MA_GetAddress(mlist);
      free(mlist);
   }
}
MakeHook(MA_GetAddressHook, MA_GetAddressFunc);

///
/// MA_PopNow
//  Fetches new mail from POP3 account(s)
void MA_PopNow(int mode, int pop)
{
   if (G->TR) return; // Don't proceed if another transfer is in progress
   if (C->UpdateStatus) MA_UpdateStatus();
   MA_StartMacro(MACRO_PREGET, itoa(mode-POP_USER));
   TR_GetMailFromNextPOP(TRUE, pop, mode);
}

///
/// MA_PopNowFunc
HOOKPROTONHNO(MA_PopNowFunc, void, int *arg)
{
   ULONG qual = (ULONG)arg[2];
   if(hasFlag(qual, (IEQUALIFIER_LSHIFT|IEQUALIFIER_RSHIFT))) G->TR_Exchange = TRUE;
   MA_PopNow(arg[0],arg[1]);
}
MakeStaticHook(MA_PopNowHook, MA_PopNowFunc);
///

/*** Sub-button functions ***/
/// MA_SendMList
//  Sends a list of messages
BOOL MA_SendMList(struct Mail **mlist)
{
   BOOL success = FALSE;
   MA_StartMacro(MACRO_PRESEND, NULL);
   if (TR_OpenTCPIP())
   {
      if (CO_IsValid()) if ((G->TR = TR_New(TR_SEND)))
      {
         if (SafeOpenWindow(G->TR->GUI.WI)) success = TR_ProcessSEND(mlist);

         if(success == FALSE)
         {
            MA_ChangeTransfer(TRUE);
            DisposeModulePush(&G->TR);
         }
      }
      TR_CloseTCPIP();
   }
   else
     ER_NewError(GetStr(MSG_ER_OPENTCPIP));

   MA_StartMacro(MACRO_POSTSEND, NULL);
   return success;
}

///
/// MA_Send
//  Sends selected or all messages
BOOL MA_Send(enum SendMode sendpos)
{
   struct Mail **mlist;
   APTR lv = G->MA->GUI.PG_MAILLIST;
   BOOL success = FALSE;
   if (!G->TR)
   {
      MA_ChangeFolder(FO_GetFolderByType(FT_OUTGOING, NULL), TRUE);
      if (sendpos == SEND_ALL) DoMethod(lv, MUIM_NList_Select, MUIV_NList_Select_All, MUIV_NList_Select_On, NULL);
      if ((mlist = MA_CreateMarkedList(lv, FALSE)))
      {
         success = MA_SendMList(mlist);
         free(mlist);
      }
   }
   return success;
}

///
/// MA_SendFunc
HOOKPROTONHNO(MA_SendFunc, void, int *arg)
{
   MA_Send(arg[0]);
}
MakeHook(MA_SendHook, MA_SendFunc);
///

/*** Menu options ***/
/// MA_SetStatusTo
//  Sets status of selectes messages
void MA_SetStatusTo(int addflags, int clearflags, BOOL all)
{
  Object *lv = G->MA->GUI.PG_MAILLIST;
  struct Mail **mlist;

  // generate a mail list of either all or just the selected
  // (marked) mails.
  if(all)
    mlist = MA_CreateFullList(FO_GetCurrentFolder(), FALSE);
  else
    mlist = MA_CreateMarkedList(lv, FALSE);

  if(mlist)
  {
    int i;

    set(lv, MUIA_NList_Quiet, TRUE);
    for(i = 0; i < (int)*mlist; i++)
    {
      MA_ChangeMailStatus(mlist[i+2], addflags, clearflags);
    }
    set(lv, MUIA_NList_Quiet, FALSE);

    free(mlist);
    DisplayStatistics(NULL, TRUE);
  }
}

///
/// MA_SetStatusToFunc
HOOKPROTONHNO(MA_SetStatusToFunc, void, int *arg)
{
  MA_SetStatusTo(arg[0], arg[1], FALSE);
}
MakeHook(MA_SetStatusToHook, MA_SetStatusToFunc);

///
/// MA_SetAllStatusToFunc
HOOKPROTONHNO(MA_SetAllStatusToFunc, void, int *arg)
{
  MA_SetStatusTo(arg[0], arg[1], TRUE);
}
MakeHook(MA_SetAllStatusToHook, MA_SetAllStatusToFunc);

///
/// MA_DeleteOldFunc
//  Deletes old messages
HOOKPROTONHNONP(MA_DeleteOldFunc, void)
{
   struct Folder **flist;
   struct DateStamp today;
   long today_days;
   int f;
   struct Mail *mail, *next;

   DateStampUTC(&today);
   today_days = today.ds_Days;

   if ((flist = FO_CreateList()))
   {
      BusyGauge(GetStr(MSG_BusyDeletingOld), "", (int)*flist);

      for (f = 1; f <= (int)*flist; f++)
      {
        if (flist[f]->MaxAge)
        {
          if (MA_GetIndex(flist[f]))
          {
            for(mail = flist[f]->Messages; mail; mail = next)
            {
              next = mail->Next;
              today.ds_Days = today_days - flist[f]->MaxAge;
              if (CompareDates(&today, &(mail->Date)) < 0)
              {
                if(flist[f]->Type == FT_DELETED ||
                   (!hasStatusNew(mail) && hasStatusRead(mail)))
                {
                  MA_DeleteSingle(mail, C->RemoveOnQuit, TRUE);

                  DisplayStatistics(mail->Folder, FALSE);
                }
              }
            }
          }
        }

        BusySet(f);
      }
      free(flist);

      // and last but not least we update the appIcon also
      DisplayStatistics(NULL, TRUE);

      BusyEnd();
   }
}
MakeHook(MA_DeleteOldHook, MA_DeleteOldFunc);

///
/// MA_DeleteDeletedFunc
//  Removes messages from 'deleted' folder
HOOKPROTONHNO(MA_DeleteDeletedFunc, void, int *arg)
{
  BOOL quiet = *arg != 0;
  int i = 0;
  struct Mail *mail;
  struct Folder *folder = FO_GetFolderByType(FT_DELETED, NULL);

  if(!folder) return;

  BusyGauge(GetStr(MSG_BusyEmptyingTrash), "", folder->Total);

  for (mail = folder->Messages; mail; mail = mail->Next)
  {
    BusySet(++i);
    AppendLogVerbose(21, GetStr(MSG_LOG_DeletingVerbose), AddrName(mail->From), mail->Subject, folder->Name, "");
    DeleteFile(GetMailFile(NULL, NULL, mail));
  }

  // We only clear the folder if it wasn`t empty anyway..
  if(i > 0)
  {
    ClearMailList(folder, TRUE);

    MA_ExpireIndex(folder);

    if(FO_GetCurrentFolder() == folder) DisplayMailList(folder, G->MA->GUI.PG_MAILLIST);

    AppendLogNormal(20, GetStr(MSG_LOG_Deleting), (void *)i, folder->Name, "", "");

    if(quiet == FALSE) DisplayStatistics(folder, TRUE);
  }

  BusyEnd();
}
MakeHook(MA_DeleteDeletedHook, MA_DeleteDeletedFunc);

///
/// MA_RescanIndexFunc
//  Updates index of current folder
HOOKPROTONHNONP(MA_RescanIndexFunc, void)
{
   struct Folder *folder = FO_GetCurrentFolder();

   // on groups we don't allow any index rescanning operation
   if(!folder || folder->Type == FT_GROUP)
     return;

   // we make sure that the Listview is disabled before the
   // rescan takes place. This makes sure that the user can`t play around
   // with some strange data.. ;)
   set(G->MA->GUI.PG_MAILLIST, MUIA_Disabled, TRUE);

   // make sure we clear the NList previous to our index rescanning
   // or we risk that it still refers to some free data.
   DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_NList_Clear);

   // we start a rescan by expiring the current index and issueing
   // a new MA_GetIndex(). That will also cause the GUI to refresh!
   folder->LoadedMode = LM_UNLOAD;
   MA_ExpireIndex(folder);
   MA_GetIndex(folder);

   // if we are still in the folder we wanted to rescan,
   // we can refresh the list.
   if(folder == FO_GetCurrentFolder())
     MA_ChangeFolder(NULL, FALSE);
}
MakeHook(MA_RescanIndexHook, MA_RescanIndexFunc);

///
/// MA_ExportMessages
//  Saves messages to a MBOX mailbox file
BOOL MA_ExportMessages(BOOL all, char *filename, BOOL append)
{
  BOOL success = FALSE;
  char outname[SIZE_PATHFILE];
  struct Folder *actfo = FO_GetCurrentFolder();
  struct Mail **mlist;

  ENTER();

  // check that a real folder is active
  if(!actfo || actfo->Type == FT_GROUP)
  {
    RETURN(FALSE);
    return FALSE;
  }

  if(all)
    mlist = MA_CreateFullList(actfo, FALSE);
  else
    mlist = MA_CreateMarkedList(G->MA->GUI.PG_MAILLIST, FALSE);

  if(mlist)
  {
    if(!filename && ReqFile(ASL_EXPORT, G->MA->GUI.WI, GetStr(MSG_MA_MESSAGEEXPORT), REQF_SAVEMODE, C->DetachDir, ""))
    {
      strmfp(filename = outname, G->ASLReq[ASL_EXPORT]->fr_Drawer, G->ASLReq[ASL_EXPORT]->fr_File);

      if(FileExists(filename))
      {
        char * a = GetStr(MSG_MA_ExportAppendOpts);
        char * b = GetStr(MSG_MA_ExportAppendReq);

        switch(MUI_Request(G->App, G->MA->GUI.WI, 0, GetStr(MSG_MA_MESSAGEEXPORT), a, b))
        {
          case 1: append = FALSE; break;
          case 2: append = TRUE; break;
          case 0: filename = NULL;
        }
      }
    }

    if(filename && (G->TR = TR_New(TR_EXPORT)))
    {
      if(SafeOpenWindow(G->TR->GUI.WI))
        success = TR_ProcessEXPORT(filename, mlist, append);

      if(success == FALSE)
      {
        MA_ChangeTransfer(TRUE);
        DisposeModulePush(&G->TR);
      }
    }

    free(mlist);
  }

  RETURN(success);
  return success;
}

///
/// MA_ExportMessagesFunc
HOOKPROTONHNO(MA_ExportMessagesFunc, void, int *arg)
{
   MA_ExportMessages((BOOL)*arg, NULL, FALSE);
}
MakeHook(MA_ExportMessagesHook, MA_ExportMessagesFunc);

///
/// MA_ImportMessages
//  Imports messages from a MBOX mailbox file
BOOL MA_ImportMessages(char *fname)
{
  BOOL result = FALSE;
  struct Folder *actfo = FO_GetCurrentFolder();
  enum ImportFormat foundFormat = IMF_UNKNOWN;
  FILE *fh;

  ENTER();

  // check that a real folder is active
  if(!actfo || actfo->Type == FT_GROUP)
  {
    RETURN(FALSE);
    return FALSE;
  }

  // check if the file exists or not and if so, open
  // it immediately.
  if((fh = fopen(fname, "r")))
  {
    char buffer[SIZE_LINE];

    // what we do first is to try to find out which
    // file the user tries to import and if it is a valid
    // and supported one.

    // try to identify the file as an MBOX file
    D(DBF_IMPORT, "processing MBOX file identification");
    while(fgets(buffer, SIZE_LINE, fh) && foundFormat == IMF_UNKNOWN)
    {
      if(strncmp(buffer, "From ", 5) == 0)
        foundFormat = IMF_MBOX;
    }

    // if we still couldn't identify the file
    // we go and try to identify it as a dbx (Outlook Express)
    // message file
    // Please check http://oedbx.aroh.de/ for a recent description
    // of the format!
    if(foundFormat == IMF_UNKNOWN)
    {
      unsigned char *file_header;

      D(DBF_IMPORT, "processing DBX file identification");

      // seek the file pointer back
      fseek(fh, 0, SEEK_SET);

      // read the 9404 bytes long file header for properly identifying
      // an Outlook Express database file.
      if((file_header = (unsigned char *)malloc(0x24bc)))
      {
        if(fread(file_header, 1, 0x24bc, fh) == 0x24bc)
        {
          // try to identify the file as a CLSID_MessageDatabase file
          if((file_header[0] == 0xcf && file_header[1] == 0xad &&
              file_header[2] == 0x12 && file_header[3] == 0xfe) &&
             (file_header[4] == 0xc5 && file_header[5] == 0xfd &&
              file_header[6] == 0x74 && file_header[7] == 0x6f))
          {
            // the file seems to be indeed an Outlook Express
            // message database file (.dbx)
            foundFormat = IMF_DBX;
          }
        }

        free(file_header);
      }
    }

    // if we still haven't identified the file we try to find out
    // if it might be just a RAW mail file without a common "From "
    // phrase a MBOX compliant mail file normally contains.
    if(foundFormat == IMF_UNKNOWN || foundFormat == IMF_MBOX)
    {
      int foundTokens = 0;

      D(DBF_IMPORT, "processing PLAIN mail file identification");

      // seek the file pointer back
      fseek(fh, 0, SEEK_SET);

      while(fgets(buffer, SIZE_LINE, fh) && foundTokens < 2)
      {
        if(strnicmp(buffer, "From:", 5) == 0)
          foundTokens = 1;
        else if(strnicmp(buffer, "Subject:", 8) == 0)
          foundTokens = 2;
      }

      // if we found all tokens we can set the ImportFormat accordingly.
      if(foundTokens == 2)
        foundFormat = (foundFormat == IMF_UNKNOWN ? IMF_PLAIN : IMF_MBOX);
      else
        foundFormat = IMF_UNKNOWN;
    }

    fclose(fh);
  }

  SHOWVALUE(DBF_IMPORT, foundFormat);

  // if we found that the file contains a valid import format
  // we go and create a transfer window object and let the user
  // choose which mail he wants to actually import.
  if(foundFormat != IMF_UNKNOWN && (G->TR = TR_New(TR_IMPORT)))
  {
    TR_SetWinTitle(TRUE, (char *)FilePart(fname));

    // put some import relevant data into variables of our
    // transfer window object
    strlcpy(G->TR->ImportFile, fname, sizeof(G->TR->ImportFile));
    G->TR->ImportFolder = actfo;
    G->TR->ImportFormat = foundFormat;

    // call GetMessageList_IMPORT() to parse the file once again
    // and present the user with a selectable list of mails the file
    // contains.
    if(TR_GetMessageList_IMPORT() && SafeOpenWindow(G->TR->GUI.WI))
      result = TRUE;
    else
    {
      MA_ChangeTransfer(TRUE);
      DisposeModulePush(&G->TR);
    }
  }

  RETURN(result);
  return result;
}

///
/// MA_ImportMessagesFunc
HOOKPROTONHNONP(MA_ImportMessagesFunc, void)
{
  struct Folder *actfo = FO_GetCurrentFolder();

  if(!actfo || actfo->Type == FT_GROUP)
    return;

  // put up an Requester to query the user for the input file.
  if(ReqFile(ASL_IMPORT, G->MA->GUI.WI, GetStr(MSG_MA_MessageImport), REQF_NONE, C->DetachDir, ""))
  {
    char inname[SIZE_PATHFILE];
    strmfp(inname, G->ASLReq[ASL_IMPORT]->fr_Drawer, G->ASLReq[ASL_IMPORT]->fr_File);

    // now start the actual importing of the messages
    if(!MA_ImportMessages(inname))
      ER_NewError(GetStr(MSG_ER_MESSAGEIMPORT), inname);
  }
}
MakeStaticHook(MA_ImportMessagesHook, MA_ImportMessagesFunc);

///
/// MA_MoveMessageFunc
//  Moves selected messages to a user specified folder
HOOKPROTONHNONP(MA_MoveMessageFunc, void)
{
   struct Folder *src = FO_GetCurrentFolder(), *dst;

   if(!src) return;

   if ((dst = FolderRequest(GetStr(MSG_MA_MoveMsg), GetStr(MSG_MA_MoveMsgReq), GetStr(MSG_MA_MoveGad), GetStr(MSG_Cancel), src, G->MA->GUI.WI)))
      MA_MoveCopy(NULL, src, dst, FALSE);
}
MakeHook(MA_MoveMessageHook, MA_MoveMessageFunc);

///
/// MA_CopyMessageFunc
//  Copies selected messages to a user specified folder
HOOKPROTONHNONP(MA_CopyMessageFunc, void)
{
   struct Folder *src = FO_GetCurrentFolder(), *dst;

   if(!src) return;

   if ((dst = FolderRequest(GetStr(MSG_MA_CopyMsg), GetStr(MSG_MA_MoveMsgReq), GetStr(MSG_MA_CopyGad), GetStr(MSG_Cancel), NULL, G->MA->GUI.WI)))
      MA_MoveCopy(NULL, src, dst, TRUE);
}
MakeHook(MA_CopyMessageHook, MA_CopyMessageFunc);

///
/// MA_ChangeSubject
//  Changes subject of a message
void MA_ChangeSubject(struct Mail *mail, char *subj)
{
   struct Folder *fo = mail->Folder;
   int f;
   FILE *oldfh, *newfh;
   char *oldfile, newfile[SIZE_PATHFILE], fullfile[SIZE_PATHFILE], buf[SIZE_LINE];

   if (!strcmp(subj, mail->Subject)) return;
   if (!StartUnpack(oldfile = GetMailFile(NULL, NULL, mail), fullfile, fo)) return;
   strmfp(newfile, GetFolderDir(fo), "00000.tmp");
   if ((newfh = fopen(newfile, "w")))
   {
      if ((oldfh = fopen(fullfile, "r")))
      {
         BOOL infield = FALSE, inbody = FALSE, hasorigsubj = FALSE;
         while (fgets(buf, SIZE_LINE, oldfh))
         {
            if (*buf == '\n' && !inbody)
            {
               inbody = TRUE;
               if (!hasorigsubj) EmitHeader(newfh, "X-Original-Subject", mail->Subject);
               EmitHeader(newfh, "Subject", subj);
            }

            if(!isspace(*buf))
            {
               infield = !strnicmp(buf, "subject:", 8);
               if (!strnicmp(buf, "x-original-subject:", 19)) hasorigsubj = TRUE;
            }
            if (!infield || inbody) fputs(buf, newfh);
         }
         fclose(oldfh);
         DeleteFile(oldfile);
      }
      fclose(newfh);
      f = FileSize(newfile); fo->Size += f - mail->Size; mail->Size = f;
      AppendLog(82, GetStr(MSG_LOG_ChangingSubject), mail->Subject, mail->MailFile, fo->Name, subj);
      strlcpy(mail->Subject, subj, sizeof(mail->Subject));
      MA_ExpireIndex(fo);

      if(fo->Mode > FM_SIMPLE)
        DoPack(newfile, oldfile, fo);
      else
        RenameFile(newfile, oldfile);
   }
   FinishUnpack(fullfile);
}

///
/// MA_ChangeSubjectFunc
//  Changes subject of selected messages
HOOKPROTONHNONP(MA_ChangeSubjectFunc, void)
{
   struct Mail **mlist, *mail;
   int i, selected;
   BOOL ask = TRUE;
   char subj[SIZE_SUBJECT];

   if (!(mlist = MA_CreateMarkedList(G->MA->GUI.PG_MAILLIST, FALSE))) return;
   selected = (int)*mlist;

   for (i = 0; i < selected; i++)
   {
      mail = mlist[i+2];
      if (ask)
      {
         strlcpy(subj, mail->Subject, sizeof(subj));

         switch (StringRequest(subj, SIZE_SUBJECT, GetStr(MSG_MA_ChangeSubj), GetStr(MSG_MA_ChangeSubjReq), GetStr(MSG_Okay), (i || selected == 1) ? NULL : GetStr(MSG_MA_All), GetStr(MSG_Cancel), FALSE, G->MA->GUI.WI))
         {
            case 0: free(mlist); return;
            case 2: ask = FALSE;
         }
      }
      MA_ChangeSubject(mail, subj);
   }
   free(mlist);
   DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_NList_Redraw, MUIV_NList_Redraw_All);
   DisplayStatistics(NULL, TRUE);
}
MakeHook(MA_ChangeSubjectHook, MA_ChangeSubjectFunc);

///
/// MA_AboutMUIFunc
//  Displays 'About MUI' window
HOOKPROTONHNONP(MA_AboutMUIFunc, void)
{
  static Object *muiwin = NULL;

  if(!muiwin)
  {
    muiwin = AboutmuiObject,
      MUIA_Window_RefWindow,     G->MA->GUI.WI,
      MUIA_Aboutmui_Application, G->App,
    End;
  }

  if(muiwin)
    SafeOpenWindow(muiwin);
  else
    DisplayBeep(NULL);
}
MakeStaticHook(MA_AboutMUIHook, MA_AboutMUIFunc);

///
/// MA_ShowAboutWindowFunc
//  Displays 'About' window
HOOKPROTONHNONP(MA_ShowAboutWindowFunc, void)
{
  if(!G->AboutWinObject)
  {
    G->AboutWinObject = AboutwindowObject, End;
  }

  if(G->AboutWinObject)
    SafeOpenWindow(G->AboutWinObject);
  else
    DisplayBeep(NULL);
}
MakeStaticHook(MA_ShowAboutWindowHook, MA_ShowAboutWindowFunc);

///
/// MA_CheckVersionFunc
//  Checks YAM homepage for new program versions
HOOKPROTONHNONP(MA_CheckVersionFunc, void)
{
  // we rather call CheckForUpdates() directly, we better
  // issue the waiting timerequest with an interval of 1 micros so
  // that it gets fired immediately
  TC_Restart(TIO_UPDATECHECK, 0, 1);
}
MakeStaticHook(MA_CheckVersionHook, MA_CheckVersionFunc);

///
/// MA_ShowErrorsFunc
//  Opens error message window
HOOKPROTONHNONP(MA_ShowErrorsFunc, void)
{
   ER_NewError(NULL);
}
MakeStaticHook(MA_ShowErrorsHook, MA_ShowErrorsFunc);

///
/// MA_StartMacro
//  Launches user-defined ARexx script or AmigaDOS command
BOOL MA_StartMacro(enum Macro num, char *param)
{
   BPTR fh;
   char command[SIZE_LARGE], *wtitle = "CON:////YAM ARexx Window/AUTO";
   struct RexxMsg *sentrm;

   strlcpy(command, C->RX[num].Script, sizeof(command));
   if (!*command) return 0;

   if(param)
   {
     strlcat(command, " ", sizeof(command));
     strlcat(command, param, sizeof(command));
   }

   if (C->RX[num].IsAmigaDOS)
   {
      BusyText(GetStr(MSG_MA_EXECUTINGCMD), "");
      ExecuteCommand(command, !C->RX[num].WaitTerm, C->RX[num].UseConsole ? OUT_DOS : OUT_NIL);
      BusyEnd();
   }
   else if(G->RexxHost) // make sure that rexx it available
   {
      if (!(fh = Open(C->RX[num].UseConsole ? wtitle : "NIL:", MODE_NEWFILE)))
      {
         ER_NewError(GetStr(MSG_ER_ErrorConsole));
         return FALSE;
      }
      if (!(sentrm = SendRexxCommand(G->RexxHost, command, fh)))
      {
         Close(fh);
         ER_NewError(GetStr(MSG_ER_ErrorARexxScript), command);
         return FALSE;
      }
      if (C->RX[num].WaitTerm)
      {
         extern void DoRXCommand( struct RexxHost *, struct RexxMsg *);
         struct RexxMsg *rm;
         BOOL waiting = TRUE;
         BusyText(GetStr(MSG_MA_EXECUTINGCMD), "");
         do
         {
            WaitPort(G->RexxHost->port);
            while ((rm = (struct RexxMsg *)GetMsg(G->RexxHost->port)))
            {
               if ((rm->rm_Action & RXCODEMASK) != RXCOMM) ReplyMsg((struct Message *)rm);
               else if (rm->rm_Node.mn_Node.ln_Type == NT_REPLYMSG)
               {
                  struct RexxMsg *org = (struct RexxMsg *)rm->rm_Args[15];
                  if (org)
                  {
                     if (rm->rm_Result1) ReplyRexxCommand(org, 20, ERROR_NOT_IMPLEMENTED, NULL);
                     else ReplyRexxCommand(org, 0, 0, (char *)rm->rm_Result2);
                  }
                  if (rm == sentrm) waiting = FALSE;
                  FreeRexxCommand(rm);
                  --G->RexxHost->replies;
               }
               else if (rm->rm_Args[0]) DoRXCommand(G->RexxHost, rm);
               else ReplyMsg((struct Message *)rm);
            }
         }
         while (waiting);
         BusyEnd();
      }
   }
   else return FALSE;

   return TRUE;
}

///
/// MA_CallRexxFunc
//  Launches a script from the ARexx menu
HOOKPROTONHNO(MA_CallRexxFunc, void, int *arg)
{
   char scname[SIZE_COMMAND];
   int script = *arg;
   if (script >= 0)
   {
      MA_StartMacro(MACRO_MEN0+script, NULL);
   }
   else
   {
      strmfp(scname, G->ProgDir, "rexx");
      if (ReqFile(ASL_REXX, G->MA->GUI.WI, GetStr(MSG_MA_ExecuteScript), REQF_NONE, scname, ""))
      {
         strmfp(scname, G->ASLReq[ASL_REXX]->fr_Drawer, G->ASLReq[ASL_REXX]->fr_File);
         SendRexxCommand(G->RexxHost, scname, 0);
      }
   }
}
MakeStaticHook(MA_CallRexxHook, MA_CallRexxFunc);
///

/*** Hooks ***/
/// PO_Window
/*** PO_Window - Window hook for popup objects ***/
HOOKPROTONH(PO_Window, void, Object *pop, Object *win)
{
   set(win, MUIA_Window_DefaultObject, pop);
}
MakeHook(PO_WindowHook, PO_Window);

///
/// MA_LV_FConFunc
/*** MA_LV_FConFunc - Folder listview construction hook ***/
HOOKPROTONHNO(MA_LV_FConFunc, struct Folder *, struct MUIP_NListtree_ConstructMessage *msg)
{
   struct Folder *entry;

   if(!msg) return(NULL);

   entry = calloc(1, sizeof(struct Folder));
   memcpy(entry, msg->UserData, sizeof(struct Folder));

   return(entry);
}
MakeStaticHook(MA_LV_FConHook, MA_LV_FConFunc);

///
/// MA_LV_FDesFunc
/*** MA_LV_FDesFunc - Folder listtree destruction hook ***/
HOOKPROTONHNO(MA_LV_FDesFunc, LONG, struct MUIP_NListtree_DestructMessage *msg)
{
   if(!msg) return(-1);
   if(msg->UserData) FO_FreeFolder((struct Folder *)msg->UserData);
   return(0);
}
MakeStaticHook(MA_LV_FDesHook, MA_LV_FDesFunc);

///
/// MA_GetRealSubject
//  Strips reply prefix / mailing list name from subject
char *MA_GetRealSubject(char *sub)
{
   char *p;
   int sublen = strlen(sub);

   if (sublen < 3) return sub;
   if (sub[2] == ':' && !sub[3]) return "";

   // check if the subject contains some strings embedded in brackets like [test]
   // and return only the real subject after the last bracket.
   if(sub[0] == '[' && (p = strchr(sub, ']')) && p < (&sub[sublen])-3 && p < &sub[20])
   {
      // if the following char isn`t a whitespace we return the real
      // subject directly after the last bracket
      if(isspace(p[1]))
        return MA_GetRealSubject(p+2);
      else
        return MA_GetRealSubject(p+1);
   }

   if (strchr(":[({", sub[2])) if ((p = strchr(sub, ':'))) return MA_GetRealSubject(TrimStart(++p));

   return sub;
}

///
/// MA_LV_FCmp2Func
//  Folder listtree sort hook
/*
HOOKPROTONH(MA_LV_FCmp2Func, long, Object *obj, struct MUIP_NListtree_CompareMessage *ncm)
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
MakeStaticHook(MA_LV_FCmp2Hook, MA_LV_FCmp2Func);
*/
///
/// MA_FolderKeyFunc
//  If the user pressed 0-9 we jump to folder 1-10
HOOKPROTONHNO(MA_FolderKeyFunc, void, int *idx)
{
  struct MUI_NListtree_TreeNode *tn = NULL;
  int i, count = idx[0];

  // we make sure that the quicksearchbar is NOT active
  // or otherwise we steal it the focus while the user
  // tried to enter some numbers there
  if(xget(G->MA->GUI.GR_QUICKSEARCHBAR, MUIA_QuickSearchBar_SearchStringIsActive) == FALSE)
  {
    // we get the first entry and if it`s a LIST we have to get the next one
    // and so on, until we have a real entry for that key or we set nothing active
    for(i=0; i <= count; i++)
    {
      tn = (struct MUI_NListtree_TreeNode *)DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NListtree_GetEntry, MUIV_NListtree_GetEntry_ListNode_Root, i, MUIF_NONE);
      if(!tn)
        return;

      if(isFlagSet(tn->tn_Flags, TNF_LIST))
        count++;
    }

    // Force that the list is open at this entry
    DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NListtree_Open, MUIV_NListtree_Open_ListNode_Parent, tn, MUIF_NONE);

    // Now set this treenode activ
    set(G->MA->GUI.NL_FOLDERS, MUIA_NListtree_Active, tn);
  }
}
MakeHook(MA_FolderKeyHook, MA_FolderKeyFunc);

///
/// MA_FolderClickFunc
//  Handles double clicks on the folder listtree
HOOKPROTONHNONP(MA_FolderClickFunc, void)
{
  struct Folder *folder = FO_GetCurrentFolder();

  if(!folder || folder->Type == FT_GROUP) return;

  DoMethod(G->App, MUIM_CallHook, &FO_EditFolderHook);
}
MakeHook(MA_FolderClickHook, MA_FolderClickFunc);

///

/*** GUI ***/
/// MA_SetupDynamicMenus
//  Updates ARexx and POP3 account menu items
void MA_SetupDynamicMenus(void)
{
  ENTER();

  // generate the dynamic REXX Menu of the main window.
  // make sure we remove an old dynamic menu first
  if(G->MA->GUI.MN_REXX)
    DoMethod(G->MA->GUI.MS_MAIN, MUIM_Family_Remove, G->MA->GUI.MN_REXX);

  // now we generate a new one.
  G->MA->GUI.MN_REXX = MenuObject,
    MUIA_Menu_Title, GetStr(MSG_MA_Scripts),
    MUIA_Family_Child, MenuitemObject,
      MUIA_Menuitem_Title,    GetStr(MSG_MA_ExecuteScript),
      MUIA_Menuitem_Shortcut, "^",
      MUIA_UserData,          MMEN_SCRIPT,
    End,
    MUIA_Family_Child, MenuitemObject,
      MUIA_Menuitem_Title, NM_BARLABEL,
    End,
  End;

  if(G->MA->GUI.MN_REXX)
  {
    static const char *shortcuts[10] = { "0","1","2","3","4","5","6","7","8","9" };
    int i;

    // the first ten entries of our user definable
    // rexx script array is for defining rexx items
    // linked to the main menu.
    for(i=0; i < 10; i++)
    {
      if(C->RX[i].Script[0])
      {
        Object *newObj = MenuitemObject,
                           MUIA_Menuitem_Title,    C->RX[i].Name,
                           MUIA_Menuitem_Shortcut, shortcuts[i],
                           MUIA_UserData,          MMEN_MACRO+i,
                         End;

        if(newObj)
          DoMethod(G->MA->GUI.MN_REXX, MUIM_Family_AddTail, newObj);
      }
    }

    // add the new dynamic menu to our
    // main menu
    DoMethod(G->MA->GUI.MS_MAIN, MUIM_Family_AddTail, G->MA->GUI.MN_REXX);
  }


  // dynamic Folder/Check menu items
  if(G->MA->GUI.MI_CSINGLE)
    DoMethod(G->MA->GUI.MN_FOLDER, MUIM_Family_Remove, G->MA->GUI.MI_CSINGLE);

  G->MA->GUI.MI_CSINGLE = MenuitemObject,
    MUIA_Menuitem_Title, GetStr(MSG_MA_CheckSingle),
  End;

  if(G->MA->GUI.MI_CSINGLE)
  {
    int i;

    for(i=0; i < MAXP3; i++)
    {
      if(C->P3[i])
      {
        Object *newObj;

        snprintf(C->P3[i]->Account, sizeof(C->P3[i]->Account), "%s@%s", C->P3[i]->User, C->P3[i]->Server);

        newObj = MenuitemObject,
                   MUIA_Menuitem_Title, C->P3[i]->Account,
                   MUIA_UserData,       MMEN_POPHOST+i,
                 End;                   

        if(newObj)
          DoMethod(G->MA->GUI.MI_CSINGLE, MUIM_Family_AddTail, newObj);
      }
    }

    // add the new dynamic menu to our
    // main menu
    DoMethod(G->MA->GUI.MN_FOLDER, MUIM_Family_AddTail, G->MA->GUI.MI_CSINGLE);
  }

  LEAVE();
}

///
/// MA_SetupEmbeddedReadPane()
//  Updates/Setup the embedded read pane part in the main window
void MA_SetupEmbeddedReadPane(void)
{
  Object *mailViewGroup  = G->MA->GUI.GR_MAILVIEW;
  Object *mailBalanceObj = G->MA->GUI.BL_MAILVIEW;
  Object *readPaneObj    = G->MA->GUI.MN_EMBEDDEDREADPANE;

  // check whether the embedded read pane object is already embeeded in our main
  // window so that we know what to do now
  if(readPaneObj)
  {
    if(C->EmbeddedReadPane == FALSE)
    {
      // the user want to have the embedded read pane removed from the main
      // window, so lets do it now
      if(DoMethod(mailViewGroup, MUIM_Group_InitChange))
      {
        DoMethod(mailViewGroup, OM_REMMEMBER, readPaneObj);
        DoMethod(mailViewGroup, OM_REMMEMBER, mailBalanceObj);

        // dispose the objects now that we don't need them anymore
        MUI_DisposeObject(readPaneObj);
        MUI_DisposeObject(mailBalanceObj);

        // and nullify it to make it readdable again
        G->MA->GUI.MN_EMBEDDEDREADPANE = NULL;
        G->MA->GUI.BL_MAILVIEW = NULL;

        DoMethod(mailViewGroup, MUIM_Group_ExitChange);
      }
    }
  }
  else
  {
    if(C->EmbeddedReadPane == TRUE)
    {
      // the user want to have the embedded read pane added to the main
      // window, so lets do it now and create the object
      G->MA->GUI.BL_MAILVIEW = mailBalanceObj = BalanceObject, End;
      if(mailBalanceObj)
      {
        G->MA->GUI.MN_EMBEDDEDREADPANE = readPaneObj = ReadMailGroupObject,
                                                         MUIA_ContextMenu, TRUE,
                                                       End;

        if(readPaneObj)
        {
          if(DoMethod(mailViewGroup, MUIM_Group_InitChange))
          {
            DoMethod(mailViewGroup, OM_ADDMEMBER, mailBalanceObj);
            DoMethod(mailViewGroup, OM_ADDMEMBER, readPaneObj);

            DoMethod(mailViewGroup, MUIM_Group_ExitChange);

            // here everything worked fine so we can return immediately
            return;
          }

          MUI_DisposeObject(readPaneObj);
          G->MA->GUI.MN_EMBEDDEDREADPANE = NULL;
        }

        MUI_DisposeObject(mailBalanceObj);
        G->MA->GUI.BL_MAILVIEW = NULL;
      }
    }
  }
}
///
/// MA_SetupQuickSearchBar()
//  Updates/Setup the quicksearchbar part in the main window
void MA_SetupQuickSearchBar(void)
{
  ENTER();

  // if the quickSearchBar is enabled by the user we
  // make sure we show it
  DoMethod(G->MA->GUI.GR_QUICKSEARCHBAR, MUIM_QuickSearchBar_Clear);
  set(G->MA->GUI.GR_QUICKSEARCHBAR, MUIA_ShowMe, C->QuickSearchBar);

  LEAVE();
}

///
/// MA_SortWindow
//  Resorts the main window group accordingly to the InfoBar setting
BOOL MA_SortWindow(void)
{
  BOOL showbar = TRUE;

  DoMethod(G->MA->GUI.GR_MAIN, MUIM_Group_InitChange);

  switch(C->InfoBar)
  {
    case IB_POS_TOP:
    {
      DoMethod(G->MA->GUI.GR_MAIN, MUIM_Group_Sort, G->MA->GUI.IB_INFOBAR,
                                                    G->MA->GUI.GR_TOP,
                                                    G->MA->GUI.GR_HIDDEN,
                                                    G->MA->GUI.GR_BOTTOM,
                                                    NULL);
    }
    break;

    case IB_POS_CENTER:
    {
      DoMethod(G->MA->GUI.GR_MAIN, MUIM_Group_Sort, G->MA->GUI.GR_TOP,
                                                    G->MA->GUI.GR_HIDDEN,
                                                    G->MA->GUI.IB_INFOBAR,
                                                    G->MA->GUI.GR_BOTTOM,
                                                    NULL);
    }
    break;

    case IB_POS_BOTTOM:
    {
      DoMethod(G->MA->GUI.GR_MAIN, MUIM_Group_Sort, G->MA->GUI.GR_TOP,
                                                    G->MA->GUI.GR_HIDDEN,
                                                    G->MA->GUI.GR_BOTTOM,
                                                    G->MA->GUI.IB_INFOBAR,
                                                    NULL);
    }
    break;

    default:
    {
      showbar = FALSE;
    }
  }

  // Here we can do a MUIA_ShowMe, TRUE because ResortWindow is encapsulated
  // in a InitChange/ExitChange..
  set(G->MA->GUI.IB_INFOBAR, MUIA_ShowMe, showbar);

  DoMethod(G->MA->GUI.GR_MAIN, MUIM_Group_ExitChange);

  return TRUE;
}
///

/// MA_New
//  Creates main window
struct MA_ClassData *MA_New(void)
{
   struct MA_ClassData *data = calloc(1, sizeof(struct MA_ClassData));

   if(data)
   {
      static const struct NewToolbarEntry tb_butt[ARRAY_SIZE(data->GUI.TB_TOOLBAR)] = {
        { MSG_MA_TBRead,     MSG_HELP_MA_BT_READ       },
        { MSG_MA_TBEdit,     MSG_HELP_MA_BT_EDIT       },
        { MSG_MA_TBMove,     MSG_HELP_MA_BT_MOVE       },
        { MSG_MA_TBDelete,   MSG_HELP_MA_BT_DELETE     },
        { MSG_MA_TBGetAddr,  MSG_HELP_MA_BT_GETADDRESS },
        { MSG_Space,         NULL                      },
        { MSG_MA_TBWrite,    MSG_HELP_MA_BT_WRITE      },
        { MSG_MA_TBReply,    MSG_HELP_MA_BT_REPLY      },
        { MSG_MA_TBForward,  MSG_HELP_MA_BT_FORWARD    },
        { MSG_Space,         NULL                      },
        { MSG_MA_TBGetMail,  MSG_HELP_MA_BT_POPNOW     },
        { MSG_MA_TBSendAll,  MSG_HELP_MA_BT_SENDALL    },
        { MSG_Space,         NULL                      },
        { MSG_MA_TBFilter,   MSG_HELP_MA_BT_FILTER     },
        { MSG_MA_TBFind,     MSG_HELP_MA_BT_SEARCH     },
        { MSG_MA_TBAddrBook, MSG_HELP_MA_BT_ABOOK      },
        { MSG_MA_TBConfig,   MSG_HELP_MA_BT_CONFIG     },
        { NULL,              NULL                      }
      };
      char *username;
      struct User *user;
      ULONG i;

      for(i = 0; i < ARRAY_SIZE(data->GUI.TB_TOOLBAR); i++)
      {
        SetupToolbar(&(data->GUI.TB_TOOLBAR[i]), tb_butt[i].label?(tb_butt[i].label==MSG_Space?"":GetStr(tb_butt[i].label)):NULL, tb_butt[i].help?GetStr(tb_butt[i].help):NULL, 0);
      }

      if (username = C->RealName,(user = US_GetCurrentUser()))
        username = user->Name;

      snprintf(data->WinTitle, sizeof(data->WinTitle), GetStr(MSG_MA_WinTitle), yamversionver, username);

      data->GUI.MS_MAIN = MenustripObject,
         MUIA_Family_Child, MenuObject, MUIA_Menu_Title, GetStr(MSG_MA_Project),
            MUIA_Family_Child, MakeMenuitem(GetStr(MSG_PROJECT_ABOUT), MMEN_ABOUT),
            MUIA_Family_Child, MakeMenuitem(GetStr(MSG_MA_AboutMUI), MMEN_ABOUTMUI),
            MUIA_Family_Child, MakeMenuitem(GetStr(MSG_MA_UPDATECHECK), MMEN_VERSION),
            MUIA_Family_Child, data->GUI.MI_ERRORS = MenuitemObject, MUIA_Menuitem_Title, GetStr(MSG_MA_LastErrors), MUIA_Menuitem_Enabled, G->ER_NumErr > 0, MUIA_UserData, MMEN_ERRORS, End,
            MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title, NM_BARLABEL, End,
            MUIA_Family_Child, MakeMenuitem(GetStr(MSG_MA_Restart), MMEN_LOGIN),
            MUIA_Family_Child, MakeMenuitem(GetStr(MSG_MA_Hide), MMEN_HIDE),
            MUIA_Family_Child, MakeMenuitem(GetStr(MSG_MA_Quit), MMEN_QUIT),
         End,
         MUIA_Family_Child, data->GUI.MN_FOLDER = MenuObject, MUIA_Menu_Title, GetStr(MSG_Folder),
            MUIA_Family_Child, MakeMenuitem(GetStr(MSG_FOLDER_NEWFOLDER), MMEN_NEWF),
            MUIA_Family_Child, MakeMenuitem(GetStr(MSG_FOLDER_NEWFOLDERGROUP), MMEN_NEWFG),
            MUIA_Family_Child, MakeMenuitem(GetStr(MSG_FOLDER_EDIT), MMEN_EDITF),
            MUIA_Family_Child, MakeMenuitem(GetStr(MSG_FOLDER_DELETE), MMEN_DELETEF),
            MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title, GetStr(MSG_MA_SortOrder),
               MUIA_Family_Child, MakeMenuitem(GetStr(MSG_MA_OSave), MMEN_OSAVE),
               MUIA_Family_Child, MakeMenuitem(GetStr(MSG_MA_Reset), MMEN_ORESET),
            End,
            MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title, NM_BARLABEL, End,
            MUIA_Family_Child, MakeMenuitem(GetStr(MSG_MA_MSearch), MMEN_SEARCH),
            MUIA_Family_Child, data->GUI.MI_FILTER = MakeMenuitem(GetStr(MSG_MA_MFilter), MMEN_FILTER),
            MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title, NM_BARLABEL, End,
            MUIA_Family_Child, MakeMenuitem(GetStr(MSG_MA_RemoveDeleted), MMEN_DELDEL),
            MUIA_Family_Child, data->GUI.MI_UPDINDEX = MakeMenuitem(GetStr(MSG_MA_UpdateIndex), MMEN_INDEX),
            MUIA_Family_Child, MakeMenuitem(GetStr(MSG_MA_FlushIndices), MMEN_FLUSH),
            MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title, NM_BARLABEL, End,
            MUIA_Family_Child, data->GUI.MI_IMPORT = MakeMenuitem(GetStr(MSG_FOLDER_IMPORT), MMEN_IMPORT),
            MUIA_Family_Child, data->GUI.MI_EXPORT = MakeMenuitem(GetStr(MSG_FOLDER_EXPORT), MMEN_EXPORT),
            MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title, NM_BARLABEL, End,
            MUIA_Family_Child, data->GUI.MI_SENDALL = MakeMenuitem(GetStr(MSG_MA_MSendAll), MMEN_SENDMAIL),
            MUIA_Family_Child, data->GUI.MI_EXCHANGE = MakeMenuitem(GetStr(MSG_MA_Exchange), MMEN_EXMAIL),
            MUIA_Family_Child, data->GUI.MI_GETMAIL = MakeMenuitem(GetStr(MSG_MA_MGetMail), MMEN_GETMAIL),
        End,
         MUIA_Family_Child, MenuObject, MUIA_Menu_Title, GetStr(MSG_Message),
            MUIA_Family_Child, data->GUI.MI_READ = MakeMenuitem(GetStr(MSG_MA_MRead), MMEN_READ),
            MUIA_Family_Child, data->GUI.MI_EDIT = MakeMenuitem(GetStr(MSG_MESSAGE_EDIT), MMEN_EDIT),
            MUIA_Family_Child, data->GUI.MI_MOVE = MakeMenuitem(GetStr(MSG_MESSAGE_MOVE), MMEN_MOVE),
            MUIA_Family_Child, data->GUI.MI_COPY = MakeMenuitem(GetStr(MSG_MESSAGE_COPY), MMEN_COPY),
            MUIA_Family_Child, data->GUI.MI_DELETE = MenuitemObject, MUIA_Menuitem_Title, GetStr(MSG_MA_MDelete), MUIA_Menuitem_Shortcut, "Del", MUIA_Menuitem_CommandString,TRUE, MUIA_UserData, MMEN_DELETE, End,
            MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title, NM_BARLABEL, End,
            MUIA_Family_Child, data->GUI.MI_PRINT = MakeMenuitem(GetStr(MSG_MESSAGE_PRINT), MMEN_PRINT),
            MUIA_Family_Child, data->GUI.MI_SAVE = MakeMenuitem(GetStr(MSG_MESSAGE_SAVE), MMEN_SAVE),
            MUIA_Family_Child, data->GUI.MI_ATTACH = MenuitemObject, MUIA_Menuitem_Title, GetStr(MSG_Attachments),
               MUIA_Family_Child, data->GUI.MI_SAVEATT = MakeMenuitem(GetStr(MSG_MESSAGE_SAVEATT), MMEN_DETACH),
               MUIA_Family_Child, data->GUI.MI_REMATT = MakeMenuitem(GetStr(MSG_MESSAGE_CROP), MMEN_CROP),
            End,
            MUIA_Family_Child, data->GUI.MI_EXPMSG = MakeMenuitem(GetStr(MSG_MESSAGE_EXPORT), MMEN_EXPMSG),
            MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title, NM_BARLABEL, End,
            MUIA_Family_Child, MakeMenuitem(GetStr(MSG_MESSAGE_NEW), MMEN_NEW),
            MUIA_Family_Child, data->GUI.MI_REPLY = MakeMenuitem(GetStr(MSG_MESSAGE_REPLY), MMEN_REPLY),
            MUIA_Family_Child, data->GUI.MI_FORWARD = MakeMenuitem(GetStr(MSG_MESSAGE_FORWARD), MMEN_FORWARD),
            MUIA_Family_Child, data->GUI.MI_BOUNCE = MakeMenuitem(GetStr(MSG_MESSAGE_BOUNCE), MMEN_BOUNCE),
            MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title, NM_BARLABEL, End,
            MUIA_Family_Child, data->GUI.MI_GETADDRESS = MakeMenuitem(GetStr(MSG_MESSAGE_GETADDRESS), MMEN_SAVEADDR),
            MUIA_Family_Child, data->GUI.MI_SELECT = MenuitemObject, MUIA_Menuitem_Title, GetStr(MSG_MA_Select),
               MUIA_Family_Child, MakeMenuitem(GetStr(MSG_MA_SelectAll), MMEN_SELALL),
               MUIA_Family_Child, MakeMenuitem(GetStr(MSG_MA_SelectNone), MMEN_SELNONE),
               MUIA_Family_Child, MakeMenuitem(GetStr(MSG_MA_SelectToggle), MMEN_SELTOGG),
            End,
            MUIA_Family_Child, data->GUI.MI_STATUS = MenuitemObject, MUIA_Menuitem_Title, GetStr(MSG_MA_SetStatus),
               MUIA_Family_Child, data->GUI.MI_TOMARKED = MakeMenuitem(GetStr(MSG_MA_TOMARKED), MMEN_TOMARKED),
               MUIA_Family_Child, data->GUI.MI_TOUNMARKED = MakeMenuitem(GetStr(MSG_MA_TOUNMARKED), MMEN_TOUNMARKED),
               MUIA_Family_Child, data->GUI.MI_TOUNREAD = MakeMenuitem(GetStr(MSG_MA_ToUnread), MMEN_TOUNREAD),
               MUIA_Family_Child, data->GUI.MI_TOREAD = MakeMenuitem(GetStr(MSG_MA_ToRead), MMEN_TOREAD),
               MUIA_Family_Child, data->GUI.MI_TOHOLD = MakeMenuitem(GetStr(MSG_MA_ToHold), MMEN_TOHOLD),
               MUIA_Family_Child, data->GUI.MI_TOQUEUED = MakeMenuitem(GetStr(MSG_MA_ToQueued), MMEN_TOQUEUED),
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title, NM_BARLABEL, End,
               MUIA_Family_Child, data->GUI.MI_ALLTOREAD = MakeMenuitem(GetStr(MSG_MA_ALLTOREAD), MMEN_ALLTOREAD),
            End,
            MUIA_Family_Child, data->GUI.MI_CHSUBJ = MakeMenuitem(GetStr(MSG_MA_ChangeSubj), MMEN_CHSUBJ),
            MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title, NM_BARLABEL, End,
            MUIA_Family_Child, data->GUI.MI_SEND = MakeMenuitem(GetStr(MSG_MA_MSend), MMEN_SEND),
         End,
         MUIA_Family_Child, MenuObject, MUIA_Menu_Title, GetStr(MSG_MA_Settings),
            MUIA_Family_Child, MakeMenuitem(GetStr(MSG_SETTINGS_ADDRESSBOOK), MMEN_ABOOK),
            MUIA_Family_Child, MakeMenuitem(GetStr(MSG_SETTINGS_CONFIG), MMEN_CONFIG),
            MUIA_Family_Child, MakeMenuitem(GetStr(MSG_SETTINGS_USERS), MMEN_USER),
            MUIA_Family_Child, MakeMenuitem(GetStr(MSG_SETTINGS_MUI), MMEN_MUI),
         End,
      End;

      data->GUI.WI = MainWindowObject,
         MUIA_Window_Title, data->WinTitle,
         MUIA_HelpNode, "MA_W",
         MUIA_Window_ID, MAKE_ID('M','A','I','N'),
         MUIA_Window_Menustrip, data->GUI.MS_MAIN,
         WindowContents, data->GUI.GR_MAIN = VGroup,
            Child, data->GUI.GR_TOP = hasHideToolBarFlag(C->HideGUIElements) ?
               VSpace(1) :
               (HGroupV,
                  MUIA_HelpNode, "MA02",
                  Child, data->GUI.TO_TOOLBAR = ToolbarObject,
                     MUIA_Toolbar_ImageType,      MUIV_Toolbar_ImageType_File,
                     MUIA_Toolbar_ImageNormal,    "PROGDIR:Icons/Main.toolbar",
                     MUIA_Toolbar_ImageGhost,     "PROGDIR:Icons/Main_G.toolbar",
                     MUIA_Toolbar_ImageSelect,    "PROGDIR:Icons/Main_S.toolbar",
                     MUIA_Toolbar_Description,    data->GUI.TB_TOOLBAR,
                     MUIA_Toolbar_ParseUnderscore,TRUE,
                     MUIA_Font,                   MUIV_Font_Tiny,
                     MUIA_ShortHelp, TRUE,
                  End,
                  Child, HSpace(0),
               End),
            Child, data->GUI.GR_HIDDEN = HGroup,
               MUIA_ShowMe, FALSE,
               Child, data->GUI.ST_LAYOUT = StringObject,
                  MUIA_ObjectID, MAKE_ID('S','T','L','A'),
                  MUIA_String_MaxLen, SIZE_DEFAULT,
               End,
            End,
            Child, data->GUI.IB_INFOBAR = InfoBarObject,
              MUIA_ShowMe,  !(C->InfoBar == IB_POS_OFF),
            End,
            Child, data->GUI.GR_BOTTOM = HGroup,
               GroupSpacing(1),
               Child, data->GUI.LV_FOLDERS = NListviewObject,
                  MUIA_HelpNode,    "MA00",
                  MUIA_CycleChain,  TRUE,
                  MUIA_HorizWeight, 30,
                  MUIA_Listview_DragType,  MUIV_Listview_DragType_Immediate,
                  MUIA_NListview_NList, data->GUI.NL_FOLDERS = MainFolderListtreeObject,
                     InputListFrame,
//                     MUIA_NList_MinColSortable      , 0,
//                     MUIA_NList_TitleClick          , TRUE,
                     MUIA_NList_DragType            , MUIV_NList_DragType_Immediate,
                     MUIA_NList_DragSortable        , TRUE,
//                     MUIA_NListtree_CompareHook     , &MA_LV_FCmp2Hook,
                     MUIA_NListtree_DisplayHook     , &MA_LV_FDspFuncHook,
                     MUIA_NListtree_ConstructHook   , &MA_LV_FConHook,
                     MUIA_NListtree_DestructHook    , &MA_LV_FDesHook,
                     MUIA_NListtree_DragDropSort    , TRUE,
                     MUIA_NListtree_Title           , TRUE,
                     MUIA_NListtree_DoubleClick     , MUIV_NListtree_DoubleClick_All,
                     MUIA_NList_DefaultObjectOnClick, FALSE,
                     MUIA_ContextMenu               , C->FolderCntMenu ? MUIV_NList_ContextMenu_Always : 0,
                     MUIA_Font                      , C->FixedFontList ? MUIV_NList_Font_Fixed : MUIV_NList_Font,
                     MUIA_Dropable                  , TRUE,
                     MUIA_NList_Exports             , MUIV_NList_Exports_ColWidth|MUIV_NList_Exports_ColOrder,
                     MUIA_NList_Imports             , MUIV_NList_Imports_ColWidth|MUIV_NList_Imports_ColOrder,
                     MUIA_ObjectID                  , MAKE_ID('N','L','0','1'),
                  End,
               End,
               Child, BalanceObject, End,
               Child, data->GUI.GR_MAILVIEW = VGroup,
                 GroupSpacing(1),
                 Child, data->GUI.GR_QUICKSEARCHBAR = QuickSearchBarObject,
                   MUIA_ShowMe, C->QuickSearchBar,
                 End,
                 Child, data->GUI.PG_MAILLIST = MainMailListGroupObject,
                   MUIA_VertWeight, 25,
                   MUIA_HelpNode,   "MA01",
                   MUIA_CycleChain, TRUE,
                 End,
               End,
            End,
         End,
      End;

      if (data->GUI.WI)
      {
         MA_MakeFOFormat(data->GUI.NL_FOLDERS);

         DoMethod(G->App, OM_ADDMEMBER, data->GUI.WI);

         // set the maillist group as the default object of that window
         set(data->GUI.WI, MUIA_Window_DefaultObject, xget(data->GUI.PG_MAILLIST, MUIA_MainMailListGroup_ActiveListObject));

         DoMethod(data->GUI.WI             ,MUIM_Notify,MUIA_Window_MenuAction   ,MMEN_ABOUT     ,MUIV_Notify_Application  ,2,MUIM_CallHook            ,&MA_ShowAboutWindowHook);
         DoMethod(data->GUI.WI             ,MUIM_Notify,MUIA_Window_MenuAction   ,MMEN_VERSION   ,MUIV_Notify_Application  ,2,MUIM_CallHook            ,&MA_CheckVersionHook);
         DoMethod(data->GUI.WI             ,MUIM_Notify,MUIA_Window_MenuAction   ,MMEN_ERRORS    ,MUIV_Notify_Application  ,2,MUIM_CallHook            ,&MA_ShowErrorsHook);
         DoMethod(data->GUI.WI             ,MUIM_Notify,MUIA_Window_MenuAction   ,MMEN_LOGIN     ,MUIV_Notify_Application  ,2,MUIM_Application_ReturnID,ID_RESTART);
         DoMethod(data->GUI.WI             ,MUIM_Notify,MUIA_Window_MenuAction   ,MMEN_HIDE      ,MUIV_Notify_Application  ,3,MUIM_Set                 ,MUIA_Application_Iconified,TRUE);
         DoMethod(data->GUI.WI             ,MUIM_Notify,MUIA_Window_MenuAction   ,MMEN_QUIT      ,MUIV_Notify_Application  ,2,MUIM_Application_ReturnID,MUIV_Application_ReturnID_Quit);
         DoMethod(data->GUI.WI             ,MUIM_Notify,MUIA_Window_MenuAction   ,MMEN_NEWF      ,MUIV_Notify_Application  ,2,MUIM_CallHook            ,&FO_NewFolderHook);
         DoMethod(data->GUI.WI             ,MUIM_Notify,MUIA_Window_MenuAction   ,MMEN_NEWFG     ,MUIV_Notify_Application  ,2,MUIM_CallHook            ,&FO_NewFolderGroupHook);
         DoMethod(data->GUI.WI             ,MUIM_Notify,MUIA_Window_MenuAction   ,MMEN_EDITF     ,MUIV_Notify_Application  ,2,MUIM_CallHook            ,&FO_EditFolderHook);
         DoMethod(data->GUI.WI             ,MUIM_Notify,MUIA_Window_MenuAction   ,MMEN_DELETEF   ,MUIV_Notify_Application  ,2,MUIM_CallHook            ,&FO_DeleteFolderHook);
         DoMethod(data->GUI.WI             ,MUIM_Notify,MUIA_Window_MenuAction   ,MMEN_OSAVE     ,MUIV_Notify_Application  ,3,MUIM_CallHook            ,&FO_SetOrderHook,SO_SAVE);
         DoMethod(data->GUI.WI             ,MUIM_Notify,MUIA_Window_MenuAction   ,MMEN_ORESET    ,MUIV_Notify_Application  ,3,MUIM_CallHook            ,&FO_SetOrderHook,SO_RESET);
         DoMethod(data->GUI.WI             ,MUIM_Notify,MUIA_Window_MenuAction   ,MMEN_SELALL    ,data->GUI.PG_MAILLIST,4,MUIM_NList_Select,MUIV_NList_Select_All,MUIV_NList_Select_On,NULL);
         DoMethod(data->GUI.WI             ,MUIM_Notify,MUIA_Window_MenuAction   ,MMEN_SELNONE   ,data->GUI.PG_MAILLIST,4,MUIM_NList_Select,MUIV_NList_Select_All,MUIV_NList_Select_Off,NULL);
         DoMethod(data->GUI.WI             ,MUIM_Notify,MUIA_Window_MenuAction   ,MMEN_SELTOGG   ,data->GUI.PG_MAILLIST,4,MUIM_NList_Select,MUIV_NList_Select_All,MUIV_NList_Select_Toggle,NULL);
         DoMethod(data->GUI.WI             ,MUIM_Notify,MUIA_Window_MenuAction   ,MMEN_SEARCH    ,MUIV_Notify_Application  ,2,MUIM_CallHook            ,&FI_OpenHook);
         DoMethod(data->GUI.WI             ,MUIM_Notify,MUIA_Window_MenuAction   ,MMEN_FILTER    ,MUIV_Notify_Application  ,4,MUIM_CallHook            ,&ApplyFiltersHook,APPLY_USER,0);
         DoMethod(data->GUI.WI             ,MUIM_Notify,MUIA_Window_MenuAction   ,MMEN_DELDEL    ,MUIV_Notify_Application  ,2,MUIM_CallHook            ,&MA_DeleteDeletedHook, FALSE);
         DoMethod(data->GUI.WI             ,MUIM_Notify,MUIA_Window_MenuAction   ,MMEN_INDEX     ,MUIV_Notify_Application  ,2,MUIM_CallHook            ,&MA_RescanIndexHook);
         DoMethod(data->GUI.WI             ,MUIM_Notify,MUIA_Window_MenuAction   ,MMEN_FLUSH     ,MUIV_Notify_Application  ,2,MUIM_CallHook            ,&MA_FlushIndexHook);
         DoMethod(data->GUI.WI             ,MUIM_Notify,MUIA_Window_MenuAction   ,MMEN_ABOOK     ,MUIV_Notify_Application  ,3,MUIM_CallHook            ,&AB_OpenHook,ABM_EDIT);
         DoMethod(data->GUI.WI             ,MUIM_Notify,MUIA_Window_MenuAction   ,MMEN_EXPORT    ,MUIV_Notify_Application  ,3,MUIM_CallHook            ,&MA_ExportMessagesHook,TRUE);
         DoMethod(data->GUI.WI             ,MUIM_Notify,MUIA_Window_MenuAction   ,MMEN_IMPORT    ,MUIV_Notify_Application  ,2,MUIM_CallHook            ,&MA_ImportMessagesHook);
         DoMethod(data->GUI.WI             ,MUIM_Notify,MUIA_Window_MenuAction   ,MMEN_GETMAIL   ,MUIV_Notify_Application  ,5,MUIM_CallHook            ,&MA_PopNowHook,POP_USER,-1,0);
         DoMethod(data->GUI.WI             ,MUIM_Notify,MUIA_Window_MenuAction   ,MMEN_SENDMAIL  ,MUIV_Notify_Application  ,3,MUIM_CallHook            ,&MA_SendHook,SEND_ALL);
         DoMethod(data->GUI.WI             ,MUIM_Notify,MUIA_Window_MenuAction   ,MMEN_EXMAIL    ,MUIV_Notify_Application  ,5,MUIM_CallHook            ,&MA_PopNowHook,POP_USER,-1,IEQUALIFIER_LSHIFT);
         DoMethod(data->GUI.WI             ,MUIM_Notify,MUIA_Window_MenuAction   ,MMEN_READ      ,MUIV_Notify_Application  ,2,MUIM_CallHook            ,&MA_ReadMessageHook);
         DoMethod(data->GUI.WI             ,MUIM_Notify,MUIA_Window_MenuAction   ,MMEN_EDIT      ,MUIV_Notify_Application  ,4,MUIM_CallHook            ,&MA_NewMessageHook,NEW_EDIT,0);
         DoMethod(data->GUI.WI             ,MUIM_Notify,MUIA_Window_MenuAction   ,MMEN_MOVE      ,MUIV_Notify_Application  ,2,MUIM_CallHook            ,&MA_MoveMessageHook);
         DoMethod(data->GUI.WI             ,MUIM_Notify,MUIA_Window_MenuAction   ,MMEN_COPY      ,MUIV_Notify_Application  ,2,MUIM_CallHook            ,&MA_CopyMessageHook);
         DoMethod(data->GUI.WI             ,MUIM_Notify,MUIA_Window_MenuAction   ,MMEN_DELETE    ,MUIV_Notify_Application  ,3,MUIM_CallHook            ,&MA_DeleteMessageHook,0);
         DoMethod(data->GUI.WI             ,MUIM_Notify,MUIA_Window_MenuAction   ,MMEN_PRINT     ,MUIV_Notify_Application  ,3,MUIM_CallHook            ,&MA_SavePrintHook,TRUE);
         DoMethod(data->GUI.WI             ,MUIM_Notify,MUIA_Window_MenuAction   ,MMEN_SAVE      ,MUIV_Notify_Application  ,3,MUIM_CallHook            ,&MA_SavePrintHook,FALSE);
         DoMethod(data->GUI.WI             ,MUIM_Notify,MUIA_Window_MenuAction   ,MMEN_DETACH    ,MUIV_Notify_Application  ,2,MUIM_CallHook            ,&MA_SaveAttachHook);
         DoMethod(data->GUI.WI             ,MUIM_Notify,MUIA_Window_MenuAction   ,MMEN_CROP      ,MUIV_Notify_Application  ,2,MUIM_CallHook            ,&MA_RemoveAttachHook);
         DoMethod(data->GUI.WI             ,MUIM_Notify,MUIA_Window_MenuAction   ,MMEN_EXPMSG    ,MUIV_Notify_Application  ,3,MUIM_CallHook            ,&MA_ExportMessagesHook,FALSE);
         DoMethod(data->GUI.WI             ,MUIM_Notify,MUIA_Window_MenuAction   ,MMEN_NEW       ,MUIV_Notify_Application  ,4,MUIM_CallHook            ,&MA_NewMessageHook,NEW_NEW,0);
         DoMethod(data->GUI.WI             ,MUIM_Notify,MUIA_Window_MenuAction   ,MMEN_REPLY     ,MUIV_Notify_Application  ,4,MUIM_CallHook            ,&MA_NewMessageHook,NEW_REPLY,0);
         DoMethod(data->GUI.WI             ,MUIM_Notify,MUIA_Window_MenuAction   ,MMEN_FORWARD   ,MUIV_Notify_Application  ,4,MUIM_CallHook            ,&MA_NewMessageHook,NEW_FORWARD,0);
         DoMethod(data->GUI.WI             ,MUIM_Notify,MUIA_Window_MenuAction   ,MMEN_BOUNCE    ,MUIV_Notify_Application  ,4,MUIM_CallHook            ,&MA_NewMessageHook,NEW_BOUNCE,0);
         DoMethod(data->GUI.WI             ,MUIM_Notify,MUIA_Window_MenuAction   ,MMEN_SAVEADDR  ,MUIV_Notify_Application  ,2,MUIM_CallHook            ,&MA_GetAddressHook);
         DoMethod(data->GUI.WI             ,MUIM_Notify,MUIA_Window_MenuAction   ,MMEN_CHSUBJ    ,MUIV_Notify_Application  ,2,MUIM_CallHook            ,&MA_ChangeSubjectHook);
         DoMethod(data->GUI.WI             ,MUIM_Notify,MUIA_Window_MenuAction   ,MMEN_SEND      ,MUIV_Notify_Application  ,3,MUIM_CallHook            ,&MA_SendHook,SEND_ACTIVE);
         DoMethod(data->GUI.WI             ,MUIM_Notify,MUIA_Window_MenuAction   ,MMEN_TOUNREAD  ,MUIV_Notify_Application  ,4,MUIM_CallHook            ,&MA_SetStatusToHook, SFLAG_NONE,              SFLAG_NEW|SFLAG_READ);
         DoMethod(data->GUI.WI             ,MUIM_Notify,MUIA_Window_MenuAction   ,MMEN_TOREAD    ,MUIV_Notify_Application  ,4,MUIM_CallHook            ,&MA_SetStatusToHook, SFLAG_READ,              SFLAG_NEW);
         DoMethod(data->GUI.WI             ,MUIM_Notify,MUIA_Window_MenuAction   ,MMEN_TOHOLD    ,MUIV_Notify_Application  ,4,MUIM_CallHook            ,&MA_SetStatusToHook, SFLAG_HOLD|SFLAG_READ,   SFLAG_QUEUED|SFLAG_ERROR);
         DoMethod(data->GUI.WI             ,MUIM_Notify,MUIA_Window_MenuAction   ,MMEN_TOQUEUED  ,MUIV_Notify_Application  ,4,MUIM_CallHook            ,&MA_SetStatusToHook, SFLAG_QUEUED|SFLAG_READ, SFLAG_SENT|SFLAG_HOLD|SFLAG_ERROR);
         DoMethod(data->GUI.WI             ,MUIM_Notify,MUIA_Window_MenuAction   ,MMEN_TOMARKED  ,MUIV_Notify_Application  ,4,MUIM_CallHook            ,&MA_SetStatusToHook, SFLAG_MARKED, SFLAG_NONE);
         DoMethod(data->GUI.WI             ,MUIM_Notify,MUIA_Window_MenuAction   ,MMEN_TOUNMARKED,MUIV_Notify_Application  ,4,MUIM_CallHook            ,&MA_SetStatusToHook, SFLAG_NONE,   SFLAG_MARKED);
         DoMethod(data->GUI.WI             ,MUIM_Notify,MUIA_Window_MenuAction   ,MMEN_ALLTOREAD ,MUIV_Notify_Application  ,4,MUIM_CallHook            ,&MA_SetAllStatusToHook, SFLAG_READ, SFLAG_NEW);
         DoMethod(data->GUI.WI             ,MUIM_Notify,MUIA_Window_MenuAction   ,MMEN_CONFIG    ,MUIV_Notify_Application  ,2,MUIM_CallHook            ,&CO_OpenHook);
         DoMethod(data->GUI.WI             ,MUIM_Notify,MUIA_Window_MenuAction   ,MMEN_USER      ,MUIV_Notify_Application  ,2,MUIM_CallHook            ,&US_OpenHook);
         DoMethod(data->GUI.WI             ,MUIM_Notify,MUIA_Window_MenuAction   ,MMEN_MUI       ,MUIV_Notify_Application  ,2,MUIM_Application_OpenConfigWindow,0);
         DoMethod(data->GUI.WI             ,MUIM_Notify,MUIA_Window_MenuAction   ,MMEN_ABOUTMUI  ,MUIV_Notify_Application  ,2,MUIM_CallHook            ,&MA_AboutMUIHook);
         DoMethod(data->GUI.WI             ,MUIM_Notify,MUIA_Window_MenuAction   ,MMEN_SCRIPT    ,MUIV_Notify_Application  ,3,MUIM_CallHook            ,&MA_CallRexxHook,-1);

         for(i = 0; i < 10; i++)
          DoMethod(data->GUI.WI,MUIM_Notify,MUIA_Window_MenuAction,MMEN_MACRO+i,MUIV_Notify_Application,3,MUIM_CallHook, &MA_CallRexxHook, i);

         for(i = 0; i < MAXP3; i++)
          DoMethod(data->GUI.WI,MUIM_Notify,MUIA_Window_MenuAction,MMEN_POPHOST+i,MUIV_Notify_Application,5,MUIM_CallHook, &MA_PopNowHook, POP_USER, i, 0);

         if (data->GUI.TO_TOOLBAR)
         {
            DoMethod(data->GUI.TO_TOOLBAR     ,MUIM_Toolbar_Notify, 0, MUIV_Toolbar_Notify_Pressed,FALSE,MUIV_Notify_Application,2,MUIM_CallHook,&MA_ReadMessageHook);
            DoMethod(data->GUI.TO_TOOLBAR     ,MUIM_Toolbar_Notify, 1, MUIV_Toolbar_Notify_Pressed,FALSE,MUIV_Notify_Application,4,MUIM_CallHook,&MA_NewMessageHook,NEW_EDIT,MUIV_Toolbar_Qualifier);
            DoMethod(data->GUI.TO_TOOLBAR     ,MUIM_Toolbar_Notify, 2, MUIV_Toolbar_Notify_Pressed,FALSE,MUIV_Notify_Application,2,MUIM_CallHook,&MA_MoveMessageHook);
            DoMethod(data->GUI.TO_TOOLBAR     ,MUIM_Toolbar_Notify, 3, MUIV_Toolbar_Notify_Pressed,FALSE,MUIV_Notify_Application,3,MUIM_CallHook,&MA_DeleteMessageHook,MUIV_Toolbar_Qualifier);
            DoMethod(data->GUI.TO_TOOLBAR     ,MUIM_Toolbar_Notify, 4, MUIV_Toolbar_Notify_Pressed,FALSE,MUIV_Notify_Application,2,MUIM_CallHook,&MA_GetAddressHook);
            DoMethod(data->GUI.TO_TOOLBAR     ,MUIM_Toolbar_Notify, 6, MUIV_Toolbar_Notify_Pressed,FALSE,MUIV_Notify_Application,4,MUIM_CallHook,&MA_NewMessageHook,NEW_NEW,MUIV_Toolbar_Qualifier);
            DoMethod(data->GUI.TO_TOOLBAR     ,MUIM_Toolbar_Notify, 7, MUIV_Toolbar_Notify_Pressed,FALSE,MUIV_Notify_Application,4,MUIM_CallHook,&MA_NewMessageHook,NEW_REPLY,MUIV_Toolbar_Qualifier);
            DoMethod(data->GUI.TO_TOOLBAR     ,MUIM_Toolbar_Notify, 8, MUIV_Toolbar_Notify_Pressed,FALSE,MUIV_Notify_Application,4,MUIM_CallHook,&MA_NewMessageHook,NEW_FORWARD,MUIV_Toolbar_Qualifier);
            DoMethod(data->GUI.TO_TOOLBAR     ,MUIM_Toolbar_Notify,10, MUIV_Toolbar_Notify_Pressed,FALSE,MUIV_Notify_Application,9,MUIM_Application_PushMethod,G->App,5,MUIM_CallHook,&MA_PopNowHook,POP_USER,-1,MUIV_Toolbar_Qualifier);
            DoMethod(data->GUI.TO_TOOLBAR     ,MUIM_Toolbar_Notify,11, MUIV_Toolbar_Notify_Pressed,FALSE,MUIV_Notify_Application,7,MUIM_Application_PushMethod,G->App,3,MUIM_CallHook,&MA_SendHook,SEND_ALL);
            DoMethod(data->GUI.TO_TOOLBAR     ,MUIM_Toolbar_Notify,13, MUIV_Toolbar_Notify_Pressed,FALSE,MUIV_Notify_Application,4,MUIM_CallHook,&ApplyFiltersHook,APPLY_USER,MUIV_Toolbar_Qualifier);
            DoMethod(data->GUI.TO_TOOLBAR     ,MUIM_Toolbar_Notify,14, MUIV_Toolbar_Notify_Pressed,FALSE,MUIV_Notify_Application,2,MUIM_CallHook,&FI_OpenHook);
            DoMethod(data->GUI.TO_TOOLBAR     ,MUIM_Toolbar_Notify,15, MUIV_Toolbar_Notify_Pressed,FALSE,MUIV_Notify_Application,3,MUIM_CallHook,&AB_OpenHook,ABM_EDIT);
            DoMethod(data->GUI.TO_TOOLBAR     ,MUIM_Toolbar_Notify,16, MUIV_Toolbar_Notify_Pressed,FALSE,MUIV_Notify_Application,2,MUIM_CallHook,&CO_OpenHook);
         }

         DoMethod(data->GUI.NL_FOLDERS     ,MUIM_Notify,MUIA_NList_DoubleClick   ,MUIV_EveryTime,MUIV_Notify_Application  ,2,MUIM_CallHook            ,&MA_FolderClickHook);
//         DoMethod(data->GUI.NL_FOLDERS     ,MUIM_Notify,MUIA_NList_TitleClick    ,MUIV_EveryTime,MUIV_Notify_Self         ,3,MUIM_NList_Sort2         ,MUIV_TriggerValue,MUIV_NList_SortTypeAdd_2Values);
//         DoMethod(data->GUI.NL_FOLDERS     ,MUIM_Notify,MUIA_NList_SortType      ,MUIV_EveryTime,MUIV_Notify_Self         ,3,MUIM_Set                 ,MUIA_NList_TitleMark,MUIV_TriggerValue);
         DoMethod(data->GUI.NL_FOLDERS     ,MUIM_Notify,MUIA_NListtree_Active    ,MUIV_EveryTime,MUIV_Notify_Application  ,2,MUIM_CallHook            ,&MA_ChangeFolderHook);
         DoMethod(data->GUI.NL_FOLDERS     ,MUIM_Notify,MUIA_NListtree_Active    ,MUIV_EveryTime,MUIV_Notify_Application  ,2,MUIM_CallHook            ,&MA_SetFolderInfoHook);
         DoMethod(data->GUI.WI             ,MUIM_Notify,MUIA_Window_CloseRequest ,TRUE          ,MUIV_Notify_Application  ,2,MUIM_Application_ReturnID,ID_CLOSEALL);
         DoMethod(data->GUI.WI             ,MUIM_Notify,MUIA_Window_InputEvent   ,"-capslock del",       MUIV_Notify_Application, 3, MUIM_CallHook, &MA_DelKeyHook, FALSE);
         DoMethod(data->GUI.WI             ,MUIM_Notify,MUIA_Window_InputEvent   ,"-capslock shift del", MUIV_Notify_Application, 3, MUIM_CallHook, &MA_DelKeyHook, TRUE);
//       DoMethod(G->App                   ,MUIM_Notify,MUIA_Application_Iconified,FALSE        ,data->GUI.WI             ,3,MUIM_Set                 ,MUIA_Window_Open,TRUE);

         // Define Notifies for ShortcutFolderKeys
         for(i = 0; i < 10; i++)
         {
            static char key[] = "-repeat 0";
            key[8] = '0'+i;
            DoMethod(data->GUI.WI, MUIM_Notify, MUIA_Window_InputEvent, key, MUIV_Notify_Application, 3, MUIM_CallHook, &MA_FolderKeyHook, i);
         }

         return data;
      }

      free(data);
   }

   return NULL;
}
///
