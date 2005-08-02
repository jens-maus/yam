/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2005 by YAM Open Source Team

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

#include "Debug.h"

/* local protos */
static ULONG MA_GetSortType(int);
static struct Mail *MA_MoveCopySingle(struct Mail*, int, struct Folder*, struct Folder*, BOOL);
static void MA_UpdateStatus(void);
static char *MA_AppendRcpt(char*, struct Person*, BOOL);
static int MA_CmpDate(struct Mail**, struct Mail**);
static void MA_InsertIntroText(FILE*, char*, struct ExpandTextData*);
static void MA_EditorNotification(int);
static void MA_SetupQuoteString(struct WR_ClassData*, struct ExpandTextData*, struct Mail*);
static int MA_CheckWriteWindow(int);
static struct Person *MA_GetAddressSelect(struct Mail*);
static char *MA_GetRealSubject(char*);
static int MA_MailCompare(struct Mail*, struct Mail*, LONG col);

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
   if (!fo) return;

   set(G->MA->GUI.NL_MAILS, MUIA_NList_SortType, MA_GetSortType(fo->Sort[0]));
   set(G->MA->GUI.NL_MAILS, MUIA_NList_SortType2, MA_GetSortType(fo->Sort[1]));
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
/// MA_ChangeSelectedFunc
//  User selected some message(s) in the message list
HOOKPROTONHNONP(MA_ChangeSelectedFunc, void)
{
   struct MA_GUIData *gui = &G->MA->GUI;
   struct Folder *fo = FO_GetCurrentFolder();
   int selected, type, i;
   BOOL active, hasattach = FALSE, beingedited = FALSE;
   struct Mail *mail;

   ENTER();

   if(!fo)
   {
     LEAVE();
     return;
   }

   // we make sure the an eventually running timer event for setting the mail
   // status of a previous mail to read is canceled beforehand
   if(C->StatusChangeDelayOn)
      TC_Stop(TIO_READSTATUSUPDATE);

   // make sure the mail is displayed in our readMailGroup of the main window
   // (if enabled) - but we do only issue a timer event here so the read pane
   // is only refreshed about 100 milliseconds after the last change in the listview
   // was recognized.
   if(C->EmbeddedReadPane)
   {
      ULONG numSelected;

      // but before we really issue a readpaneupdate we check wheter the user has
      // selected more than one mail at a time which then should clear the
      // readpane as it might have been disabled.
      DoMethod(gui->NL_MAILS, MUIM_NList_Select, MUIV_NList_Select_All, MUIV_NList_Select_Ask, &numSelected);

      if(numSelected == 1)
        TC_Restart(TIO_READPANEUPDATE, 0, C->EmbeddedMailDelay*1000);
      else
        DoMethod(gui->MN_EMBEDDEDREADPANE, MUIM_ReadMailGroup_Clear, FALSE);
   }

   type = fo->Type;
   DoMethod(gui->NL_MAILS, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &mail);
   fo->LastActive = xget(gui->NL_MAILS, MUIA_NList_Active);

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

   DoMethod(gui->NL_MAILS, MUIM_NList_Select, MUIV_NList_Select_All, MUIV_NList_Select_Ask, &selected);
   if (gui->TO_TOOLBAR)
   {
      DoMethod(gui->TO_TOOLBAR, MUIM_Toolbar_Set, 1, MUIV_Toolbar_Set_Ghosted, !active || !isOutgoingFolder(fo) || beingedited);
      DoMethod(gui->TO_TOOLBAR, MUIM_Toolbar_Set, 0, MUIV_Toolbar_Set_Ghosted, !active);
      DoMethod(gui->TO_TOOLBAR, MUIM_Toolbar_MultiSet, MUIV_Toolbar_Set_Ghosted, !active && !selected, 2,3,4,7,8, -1);
   }
   DoMethod(G->App, MUIM_MultiSet, MUIA_Menuitem_Enabled, active || selected,
      gui->MI_MOVE, gui->MI_DELETE, gui->MI_GETADDRESS, gui->MI_REPLY, gui->MI_FORWARD, gui->MI_STATUS,
      gui->MI_EXPMSG, gui->MI_COPY, gui->MI_PRINT, gui->MI_SAVE, gui->MI_CHSUBJ, NULL);
   DoMethod(G->App, MUIM_MultiSet, MUIA_Menuitem_Enabled, active, gui->MI_READ, gui->MI_BOUNCE, NULL);
   DoMethod(G->App, MUIM_MultiSet, MUIA_Menuitem_Enabled, active && isOutgoingFolder(fo) && !beingedited, gui->MI_EDIT, NULL);
   DoMethod(G->App, MUIM_MultiSet, MUIA_Menuitem_Enabled, type == FT_OUTGOING && (active || selected), gui->MI_SEND, gui->MI_TOHOLD, gui->MI_TOQUEUED, NULL);
   DoMethod(G->App, MUIM_MultiSet, MUIA_Menuitem_Enabled, !isOutgoingFolder(fo) && (active || selected) , gui->MI_TOREAD, gui->MI_TOUNREAD, gui->MI_ALLTOREAD, NULL);
   DoMethod(G->App, MUIM_MultiSet, MUIA_Menuitem_Enabled, hasattach && (active || selected), gui->MI_ATTACH, gui->MI_SAVEATT, gui->MI_REMATT, NULL);

   LEAVE();
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

      DateStamp2String(datstr, &mail->Date, C->SwatchBeat ? DSS_DATEBEAT : DSS_DATETIME, TZC_LOCAL);
      SPrintF(sh = buffer, GetStr(MSG_MA_MessageInfo), mail->From.RealName,
                                                       mail->From.Address,
                                                       mail->To.RealName,
                                                       mail->To.Address,
                                                       mail->Subject,
                                                       datstr,
                                                       mail->MailFile,
                                                       mail->Size);
   }

   set(G->MA->GUI.NL_MAILS, MUIA_ShortHelp, sh);
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
   active = xget(G->MA->GUI.NL_MAILS, MUIA_NList_Active);
   if (active != MUIV_NList_Active_Off) DoMethod(G->MA->GUI.NL_MAILS, MUIM_NList_GetEntry, active, &mail);
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
      struct MailInfo *mi;

      D(DBF_MAIL, "ChangeMailStatus: +%08lx -%08lx", addflags, clearflags);

      // set the new status
      mail->sflags = newstatus;

      // set the comment to the Mailfile
      MA_UpdateMailFile(mail);

      // flag the index as expired
      MA_ExpireIndex(mail->Folder);

      // if the mail is currently displayed in the listview we
      // have to redraw some stuff.
      mi = GetMailInfo(mail);
      if(mi->Display)
      {
        // lets redraw the entry if it is actually displayed, so that
        // the status icon gets updated.
        DoMethod(G->MA->GUI.NL_MAILS, MUIM_NList_Redraw, mi->Pos);
      }
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
  strcpy(oldFilePath, folderDir);
  AddPart((unsigned char *)oldFilePath, mail->MailFile, SIZE_PATHFILE);

  while(success == FALSE)
  {
    // generate a new filename with the data we have collected
    sprintf(newFileName, "%s.%03d,%s", dateFilePart, mcounter, statusFilePart);

    // now check if the filename has changed or not
    if(strcmp(newFileName, mail->MailFile) == 0)
    {
      success = TRUE;
      break;
    }

    // construct new full file path
    strcpy(newFilePath, folderDir);
    AddPart((unsigned char *)newFilePath, newFileName, SIZE_PATHFILE);

    // then rename it
    if(Rename(oldFilePath, newFilePath) != 0)
    {
      strcpy(mail->MailFile, newFileName);
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
            strcpy(rmData->readFile, newFilePath);
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
         // only if we want ALL or this is just a îew mail we add it to our list
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

   // we first have to check wheter this is a valid folder or not
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
            mail->Position = id;

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
      DoMethod(G->MA->GUI.NL_MAILS, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &mail);
      if(mail && (!onlyNew || hasStatusNew(mail)))
      {
        if ((mlist = calloc(3, sizeof(struct Mail *))))
        {
          id = xget(G->MA->GUI.NL_MAILS, MUIA_NList_Active);
          mail->Position = id;
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
   struct MailInfo *mi = GetMailInfo(mail);
   struct Folder *mailFolder = mail->Folder;

   if(C->RemoveAtOnce || mailFolder->Type == FT_DELETED || forceatonce)
   {
      AppendLogVerbose(21, GetStr(MSG_LOG_DeletingVerbose), AddrName(mail->From), mail->Subject, mailFolder->Name, "");
      DeleteFile(mi->FName);
      if(mi->Display)
        DoMethod(G->MA->GUI.NL_MAILS, MUIM_NList_Remove, mi->Pos);

      RemoveMailFromList(mail);

      // if we are allowed to make some noise we
      // update our Statistics
      if(!quiet)
        DisplayStatistics(mailFolder, TRUE);
   }
   else
   {
      struct Folder *delfolder = FO_GetFolderByType(FT_DELETED, NULL);

      MA_MoveCopySingle(mail, mi->Pos, mailFolder, delfolder, FALSE);

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
static struct Mail *MA_MoveCopySingle(struct Mail *mail, int pos, struct Folder *from, struct Folder *to, BOOL copyit)
{
   struct Mail cmail = *mail;
   char mfile[SIZE_MFILE];
   APTR lv;
   int result;

   strcpy(mfile, mail->MailFile);

   if((result = TransferMailFile(copyit, mail, to)) >= 0)
   {
      strcpy(cmail.MailFile, mail->MailFile);

      if(copyit)
      {
        AppendLogVerbose(25, GetStr(MSG_LOG_CopyingVerbose), AddrName(mail->From), mail->Subject, from->Name, to->Name);

        strcpy(mail->MailFile, mfile);
      }
      else
      {
        AppendLogVerbose(23, GetStr(MSG_LOG_MovingVerbose),  AddrName(mail->From), mail->Subject, from->Name, to->Name);

        if((lv = WhichLV(from)))
           DoMethod(lv, MUIM_NList_Remove, pos);

        RemoveMailFromList(mail);
      }

      mail = AddMailToList(&cmail, to);
      if((lv = WhichLV(to)))
        DoMethod(lv, MUIM_NList_InsertSingle, mail, MUIV_NList_Insert_Sorted);

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
   APTR lv;
   struct MailInfo *mi;
   struct Mail **mlist;
   int i, pos, selected = 0;

   if (frombox == tobox && !copyit) return;
   if (!(lv = WhichLV(frombox)) && !mail) return;
   if (mail)
   {
      selected = 1;
      mi = GetMailInfo(mail);
      MA_MoveCopySingle(mail, mi->Pos, frombox, tobox, copyit);
   }
   else if ((mlist = MA_CreateMarkedList(lv, FALSE)))
   {
      selected = (int)*mlist;
      set(lv, MUIA_NList_Quiet, TRUE);
      BusyGauge(GetStr(MSG_BusyMoving), itoa(selected), selected);
      for (i = 0; i < selected; i++)
      {
         mail = mlist[i+2];
         if (copyit) pos = mail->Position; else { mi = GetMailInfo(mail); pos = mi->Pos; }
         MA_MoveCopySingle(mail, pos, frombox, tobox, copyit);
         BusySet(i+1);
      }
      BusyEnd();
      set(lv, MUIA_NList_Quiet, FALSE);
      free(mlist);
   }
   if (copyit) AppendLogNormal(24, GetStr(MSG_LOG_Copying), (void *)selected, FolderName(frombox), FolderName(tobox), "");
          else AppendLogNormal(22, GetStr(MSG_LOG_Moving),  (void *)selected, FolderName(frombox), FolderName(tobox), "");
   if (!copyit) DisplayStatistics(frombox, FALSE);
   DisplayStatistics(tobox, TRUE);
   MA_ChangeSelectedFunc();
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
        if(rmData->readWindow &&
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
      strcpy(addr, pe->Address);
      strcat(addr, strchr(C->EmailAddress, '@'));
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
   if (*text)
   {
      char *sbuf;
      sbuf = ExpandText(text, etd);
      fprintf(fh, "%s\n", sbuf);
      FreeStrBuf(sbuf);
   }
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
   if (!etd) etd = &l_etd;
   etd->OS_Name      = mail ? (*(mail->From.RealName) ? mail->From.RealName : mail->From.Address) : "";
   etd->OS_Address   = mail ? mail->From.Address : "";
   etd->OM_Subject   = mail ? mail->Subject : "";
   etd->OM_Date      = mail ? &(mail->Date) : &(G->StartDate);
   etd->OM_TimeZone  = mail ? mail->tzone : C->TimeZone;
   etd->R_Name       = "";
   etd->R_Address    = "";

   sbuf = ExpandText(C->QuoteText, etd);
   stccpy(wr->QuoteText, TrimEnd(sbuf), SIZE_DEFAULT);
   FreeStrBuf(sbuf);
   stccpy(wr->AltQuoteText, C->AltQuoteText, SIZE_SMALL);
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
   for (i = 0; i < MAXWR; i++)
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
              // we check whether cmsg contains any text and if so we
              // have to take care that the leading newline isn't written to the
              // file as ReadInMessage() always puts a newline at the end of cmsg!
              if(cmsg[0] == '\0' || fwrite(cmsg, strlen(cmsg)-1, 1, out) == 1)
              {
                // free our temp text now
                free(cmsg);

                strcpy(wr->MsgID, email->IRTMsgID);
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

            MA_SetupQuoteString(wr, &etd, mail);
            etd.OM_TimeZone = email->Mail.tzone;
            etd.OM_MessageID = email->MsgID;
            etd.R_Name = *mail->To.RealName ? mail->To.RealName : mail->To.Address;
            etd.R_Address = mail->To.Address;

            if (*mail->Subject)
            {
               sprintf(buffer, "%s (fwd)", mail->Subject);
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

            MA_SetupQuoteString(wr, &etd, mail);
            etd.OM_TimeZone = email->Mail.tzone;
            etd.OM_MessageID = email->MsgID;

            // If this mail already have a subject we are going to add a "Re:" to it.
            if (*mail->Subject)
            {
               if (j) strcpy(buffer, mail->Subject);
               else sprintf(buffer, "Re: %s", MA_GetRealSubject(mail->Subject));

               if (!strstr(rsub, buffer))
               {
                  if (*rsub) rsub = StrBufCat(rsub, "; ");
                  rsub = StrBufCat(rsub, buffer);
               }
            }
            if (!multi) strcpy(wr->MsgID, email->MsgID);

            // Now we analyse the folder of the selected mail and if it
            // is a mailing list we have to do some operation
            if (folder)
            {
               char tofld[SIZE_LARGE], fromfld[SIZE_LARGE];

               strcpy(tofld, BuildAddrName2(&mail->To));
               strcpy(fromfld, BuildAddrName2(&mail->From));

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
                  sprintf(buffer, GetStr(MSG_MA_CompareReq), mail->From.Address, mail->ReplyTo.Address);
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

     sprintf(tfname, "%s.tmp", GetMailFile(fname, NULL, mail));

     if((cmsg = RE_ReadInMessage(rmData, RIM_QUIET)))
     {
       if((part = rmData->firstPart->Next) && part->Next)
       {
          FILE *out;

          if((out = fopen(tfname, "w")))
          {
             FILE *in;
   					 struct MailInfo *mi = GetMailInfo(mail);
             struct Folder *fo = mail->Folder;
             int f;

             if((in = fopen(rmData->firstPart->Filename, "r")))
             {
                BOOL infield = FALSE, inbody = FALSE;

                while(fgets(buf, SIZE_LINE, in))
                {
                  if(!ISpace(*buf))
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
	   				 DoMethod(G->MA->GUI.NL_MAILS, MUIM_NList_Redraw, mi->Pos);

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
   {
      return;
   }

   if ((mlist = MA_CreateMarkedList(G->MA->GUI.NL_MAILS, FALSE)))
   {
      int selected = (int)*mlist;
      BusyGauge(GetStr(MSG_BusyRemovingAtt), "", selected);
      for (i = 0; i < selected; i++)
      {
         MA_RemoveAttach(mlist[i+2], FALSE);
         BusySet(i+1);
      }
      DoMethod(G->MA->GUI.NL_MAILS, MUIM_NList_Redraw, MUIV_NList_Redraw_All);
      MA_ChangeSelectedFunc();
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

   if((mlist = MA_CreateMarkedList(G->MA->GUI.NL_MAILS, FALSE)))
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

   if((mlist = MA_CreateMarkedList(G->MA->GUI.NL_MAILS, FALSE)))
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
                    RE_Export(rmData, tf->Filename, "", "", 0, FALSE, FALSE, (char*)ContType[CT_TX_PLAIN]);

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
      case NEW_FORWARD: if ((mlist = MA_CreateMarkedList(G->MA->GUI.NL_MAILS, FALSE))) winnr = MA_NewForward(mlist, flags);
                        break;
      case NEW_REPLY:   if ((mlist = MA_CreateMarkedList(G->MA->GUI.NL_MAILS, FALSE))) winnr = MA_NewReply(mlist, flags);
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
   APTR lv = G->MA->GUI.NL_MAILS;
   char buffer[SIZE_DEFAULT];
   struct Folder *delfolder = FO_GetFolderByType(FT_DELETED, NULL), *folder = FO_GetCurrentFolder();
   BOOL ignoreall = FALSE;

   if(!folder || !delfolder) return;

   if (!(mlist = MA_CreateMarkedList(lv, FALSE))) return;
   selected = (int)*mlist;
   if (C->Confirm && selected >= C->ConfirmDelete && !force)
   {
      SPrintF(buffer, selected==1 ? GetStr(MSG_MA_1Selected) : GetStr(MSG_MA_xSelected), selected);
      strcat(buffer, GetStr(MSG_MA_ConfirmDel));
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
   MA_ChangeSelectedFunc();
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
      sprintf(buffer, GetStr(MSG_MA_CompareReq), mail->From.Address, mail->ReplyTo.Address);
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
   struct Mail **mlist = MA_CreateMarkedList(G->MA->GUI.NL_MAILS, FALSE);
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
   else ER_NewError(GetStr(MSG_ER_NoTCP));

   MA_StartMacro(MACRO_POSTSEND, NULL);
   return success;
}

///
/// MA_Send
//  Sends selected or all messages
BOOL MA_Send(enum SendMode sendpos)
{
   struct Mail **mlist;
   APTR lv = G->MA->GUI.NL_MAILS;
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
  Object *lv = G->MA->GUI.NL_MAILS;
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

    if(FO_GetCurrentFolder() == folder) DisplayMailList(folder, G->MA->GUI.NL_MAILS);

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
   set(G->MA->GUI.NL_MAILS, MUIA_Disabled, TRUE);

   // make sure we clear the NList previous to our index rescanning
   // or we risk that it still refers to some free data.
   DoMethod(G->MA->GUI.NL_MAILS, MUIM_NList_Clear);

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
   struct Mail **mlist;
   if (all) mlist = MA_CreateFullList(FO_GetCurrentFolder(), FALSE);
   else mlist = MA_CreateMarkedList(G->MA->GUI.NL_MAILS, FALSE);

   if (mlist)
   {
      if (!filename) if (ReqFile(ASL_IMPORT, G->MA->GUI.WI, GetStr(MSG_MA_ExportMessages), REQF_SAVEMODE, C->DetachDir, ""))
      {
         strmfp(filename = outname, G->ASLReq[ASL_IMPORT]->fr_Drawer, G->ASLReq[ASL_IMPORT]->fr_File);
         if (FileExists(filename))
         {
            char * a = GetStr(MSG_MA_ExportAppendOpts);
            char * b = GetStr(MSG_MA_ExportAppendReq);
            switch (MUI_Request(G->App, G->MA->GUI.WI, 0, GetStr(MSG_MA_ExportMessages), a, b))
            {
               case 1: append = FALSE; break;
               case 2: append = TRUE; break;
               case 0: filename = NULL;
            }
         }
      }
      if (filename) if ((G->TR = TR_New(TR_EXPORT)))
      {
         if (SafeOpenWindow(G->TR->GUI.WI)) success = TR_ProcessEXPORT(filename, mlist, append);

         if(success == FALSE)
         {
            MA_ChangeTransfer(TRUE);
            DisposeModulePush(&G->TR);
         }
      }
      free(mlist);
   }
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
   FILE *fh;

   if(!actfo) return FALSE;

   if ((fh = fopen(fname, "r")))
   {
      if ((G->TR = TR_New(TR_IMPORT)))
      {
         stccpy(G->TR->ImportFile, fname, SIZE_PATHFILE);
         TR_SetWinTitle(TRUE, (char *)FilePart(fname));
         G->TR->ImportBox = actfo;

         if(TR_GetMessageList_IMPORT(fh) && SafeOpenWindow(G->TR->GUI.WI))
         {
            result = TRUE;
         }
         else
         {
            MA_ChangeTransfer(TRUE);
            DisposeModulePush(&G->TR);
         }
      }
      fclose(fh);
   }
   return result;
}

///
/// MA_ImportMessagesFunc
HOOKPROTONHNONP(MA_ImportMessagesFunc, void)
{
   if (ReqFile(ASL_IMPORT, G->MA->GUI.WI, GetStr(MSG_MA_ImportMessages), REQF_NONE, C->DetachDir, ""))
   {
      char inname[SIZE_PATHFILE];
      strmfp(inname, G->ASLReq[ASL_IMPORT]->fr_Drawer, G->ASLReq[ASL_IMPORT]->fr_File);
      if(!MA_ImportMessages(inname))
      {
        ER_NewError(GetStr(MSG_ER_IMPORTMAIL), inname);
      }
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
            if (!ISpace(*buf))
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
      strncpy(mail->Subject, subj, SIZE_SUBJECT-1); // only copy a maximum of SIZE_SUBJECT or it will burn
      mail->Subject[SIZE_SUBJECT-1] = '\0';
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
   APTR lv = G->MA->GUI.NL_MAILS;
   char subj[SIZE_SUBJECT];

   if (!(mlist = MA_CreateMarkedList(lv, FALSE))) return;
   selected = (int)*mlist;
   for (i = 0; i < selected; i++)
   {
      mail = mlist[i+2];
      if (ask)
      {
         strcpy(subj, mail->Subject);
         switch (StringRequest(subj, SIZE_SUBJECT, GetStr(MSG_MA_ChangeSubj), GetStr(MSG_MA_ChangeSubjReq), GetStr(MSG_Okay), (i || selected == 1) ? NULL : GetStr(MSG_MA_All), GetStr(MSG_Cancel), FALSE, G->MA->GUI.WI))
         {
            case 0: free(mlist); return;
            case 2: ask = FALSE;
         }
      }
      MA_ChangeSubject(mail, subj);
   }
   free(mlist);
   DoMethod(lv, MUIM_NList_Redraw, MUIV_NList_Redraw_All);
   DisplayStatistics(NULL, TRUE);
}
MakeHook(MA_ChangeSubjectHook, MA_ChangeSubjectFunc);

///
/// MA_AboutMUIFunc
//  Displays 'About MUI' window
HOOKPROTONHNONP(MA_AboutMUIFunc, void)
{
   static APTR muiwin = NULL;

   if (!muiwin) muiwin = AboutmuiObject,
      MUIA_Window_RefWindow, G->MA->GUI.WI,
      MUIA_Aboutmui_Application, G->App,
   End;
   if (muiwin) SafeOpenWindow(muiwin); else DisplayBeep(0);
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
    DisplayBeep(0);
}
MakeStaticHook(MA_ShowAboutWindowHook, MA_ShowAboutWindowFunc);

///
/// MA_CheckVersionFunc
//  Checks YAM homepage for new program versions
HOOKPROTONHNONP(MA_CheckVersionFunc, void)
{
   struct TempFile *tf;
   int mon, day, year;
   long thisver, currver;
   char newver[SIZE_SMALL], buf[SIZE_LARGE];

   // first we check if we can start a connection or if the
   // tcp/ip stuff is busy right now so that we do not interrupt something
   if(SocketBase && G->TR_Socket != SMTP_NO_SOCKET) return;

   if (TR_OpenTCPIP())
   {
      sscanf(yamversiondate, "%d.%d.%d", &day, &mon, &year);
      thisver = (year<78 ? 1000000:0)+year*10000+mon*100+day;
      BusyText(GetStr(MSG_BusyGettingVerInfo), "");
      tf = OpenTempFile(NULL);
      if (TR_DownloadURL(C->SupportSite, "files/", "version", tf->Filename))
      {
         if ((tf->FP = fopen(tf->Filename,"r")))
         {
            fscanf(tf->FP, "%d.%d.%d", &day, &mon, &year);
            GetLine(tf->FP, newver, SIZE_SMALL);
            currver = (year<78 ? 1000000:0)+year*10000+mon*100+day;
            sprintf(buf, GetStr(MSG_MA_LatestVersion), &newver[1], day, mon, year < 78 ? 2000+year : 1900+year, yamversion, yamversiondate,
               currver > thisver ? GetStr(MSG_MA_NewVersion) : GetStr(MSG_MA_NoNewVersion));
            if (MUI_Request(G->App, G->MA->GUI.WI, 0, GetStr(MSG_MA_CheckVersion), GetStr(MSG_MA_VersionReqOpt), buf)) GotoURL(C->SupportSite);
         }
         else ER_NewError(GetStr(MSG_ER_CantOpenTempfile), tf->Filename);
      }
      CloseTempFile(tf);
      BusyEnd();
      TR_CloseTCPIP();
   }
   else ER_NewError(GetStr(MSG_ER_NoTCP));
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

   strcpy(command, C->RX[num].Script);
   if (!*command) return 0;
   if (param) { strcat(command, " "); strcat(command, param); }
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
/// MA_FindAddressHook()
HOOKPROTONHNO(MA_FindAddressFunc, LONG, struct MUIP_NListtree_FindUserDataMessage *msg)
{
  struct ABEntry *entry = (struct ABEntry *)msg->UserData;
  return Stricmp((unsigned char *)msg->User, (unsigned char *)entry->Address);
}
MakeStaticHook(MA_FindAddressHook, MA_FindAddressFunc);

///
/// MA_LV_DspFunc
/*** MA_LV_DspFunc - Message listview display hook ***/
//HOOKPROTO(MA_LV_DspFunc, long, char **array, struct Mail *entry)
HOOKPROTONH(MA_LV_DspFunc, LONG, Object *obj, struct NList_DisplayMessage *msg)
{
   struct Mail *entry;
   char **array;
   BOOL searchWinHook = FALSE;

   if(!msg) return 0;

   // now we set our local variables to the DisplayMessage structure ones
   entry = (struct Mail *)msg->entry;
   array = msg->strings;

   // now we check who is the parent of this DisplayHook
   if(G->FI && obj == G->FI->GUI.LV_MAILS)
   {
      searchWinHook = TRUE;
   }
   else if(!G->MA) return 0;

   if(entry)
   {
      if(entry->Folder)
      {
         static char dispsta[SIZE_DEFAULT];
         static char dispsiz[SIZE_SMALL];
         struct Person *pe;
         STRPTR addr;

         // prepare the status char buffer
         dispsta[0] = '\0';
         array[0] = dispsta;

         // first we check which main status this mail has
         // and put the leftmost mail icon accordingly.
         if(hasStatusError(entry) || isPartialMail(entry)) strcat(dispsta, SICON_ERROR);
         else if(hasStatusQueued(entry))  strcat(dispsta, SICON_WAITSEND);
         else if(hasStatusSent(entry))    strcat(dispsta, SICON_SENT);
         else if(hasStatusRead(entry))    strcat(dispsta, SICON_OLD);
         else                             strcat(dispsta, SICON_UNREAD);

         // then we add the 2. level if icons with the additional mail information
         // like importance, signed/crypted, report and attachment information
         if(getImportanceLevel(entry) == IMP_HIGH)  strcat(dispsta, SICON_URGENT);
         if(isMP_CryptedMail(entry))                strcat(dispsta, SICON_CRYPT);
         else if(isMP_SignedMail(entry))            strcat(dispsta, SICON_SIGNED);
         if(isMP_ReportMail(entry))                 strcat(dispsta, SICON_REPORT);
         if(isMP_MixedMail(entry))                  strcat(dispsta, SICON_ATTACH);

         // and as the 3rd level of icons we put information on the secondary status
         // like replied, forwarded, hold
         if(hasStatusNew(entry))        strcat(dispsta, SICON_NEW);
         else if(hasStatusHold(entry))  strcat(dispsta, SICON_HOLD);
         if(hasStatusMarked(entry))     strcat(dispsta, SICON_MARK);
         if(hasStatusReplied(entry))    strcat(dispsta, SICON_REPLY);
         if(hasStatusForwarded(entry))  strcat(dispsta, SICON_FORWARD);

         // now we generate the proper string for the mailaddress
         if(C->MessageCols & (1<<1) || searchWinHook)
         {
            static char dispfro[SIZE_DEFAULT];
            dispfro[0] = '\0';
            array[1] = dispfro;

            if(isMultiRCPTMail(entry))
              strcat(dispfro, SICON_GROUP);

            if(((entry->Folder->Type == FT_CUSTOMMIXED || entry->Folder->Type == FT_DELETED) &&
                (hasStatusSent(entry) || hasStatusQueued(entry) || hasStatusHold(entry) ||
                 hasStatusError(entry))) || (searchWinHook && isOutgoingFolder(entry->Folder)))
            {
              pe = &entry->To;
              strcat(dispfro, GetStr(MSG_MA_ToPrefix));
            }
            else
              pe = isOutgoingFolder(entry->Folder) ? &entry->To : &entry->From;

            #ifndef DISABLE_ADDRESSBOOK_LOOKUP
            {
              struct MUI_NListtree_TreeNode *tn;

              set(G->AB->GUI.LV_ADDRESSES, MUIA_NListtree_FindUserDataHook, &MA_FindAddressHook);

              if((tn = (struct MUI_NListtree_TreeNode *)DoMethod(G->AB->GUI.LV_ADDRESSES, MUIM_NListtree_FindUserData, MUIV_NListtree_FindUserData_ListNode_Root, &pe->Address[0], MUIF_NONE)))
              {
                addr = ((struct ABEntry *)tn->tn_User)->RealName[0] ? ((struct ABEntry *)tn->tn_User)->RealName : AddrName((*pe));
              }
              else
                addr = AddrName((*pe));
            }
            #else
            addr = AddrName((*pe));
            #endif

            // lets put the string together
            strncat(dispfro, addr, SIZE_DEFAULT-strlen(dispfro)-1);
         }

         // lets set all other fields now
         if(!searchWinHook && C->MessageCols & (1<<2))
           array[2] = AddrName((entry->ReplyTo));

         // then the Subject
         array[3] = entry->Subject;

         // we first copy the Date Received/sent because this would probably be not
         // set by all ppl and strcpy() is costy ;)
         if((C->MessageCols & (1<<7) && entry->transDate.tv_secs > 0) || searchWinHook)
         {
            static char datstr[64]; // we don`t use LEN_DATSTRING as OS3.1 anyway ignores it.
            TimeVal2String(datstr, &entry->transDate, C->SwatchBeat ? DSS_DATEBEAT : DSS_DATETIME, TZC_LOCAL);
            array[7] = datstr;
         }
         else array[7] = "";

         if(C->MessageCols & (1<<4) || searchWinHook)
         {
            static char datstr[64];
            DateStamp2String(datstr, &entry->Date, C->SwatchBeat ? DSS_DATEBEAT : DSS_DATETIME, TZC_LOCAL);
            array[4] = datstr;
         }

         if(C->MessageCols & (1<<5) || searchWinHook)
         {
            FormatSize(entry->Size, array[5] = dispsiz);
         }

         array[6] = entry->MailFile;
         array[8] = entry->Folder->Name;

         // depending on the mail status we set the font to bold or plain
         if(hasStatusUnread(entry) || hasStatusNew(entry))
           msg->preparses[1] = msg->preparses[2] = msg->preparses[3] = msg->preparses[4] = msg->preparses[5] = MUIX_B;
      }
   }
   else
   {
      struct Folder *folder = NULL;

      // first we have to make sure that the mail window has a valid folder
      if(!searchWinHook && !(folder = FO_GetCurrentFolder()))
        return 0;

      array[0] = GetStr(MSG_MA_TitleStatus);

      // depending on the current folder and the parent object we
      // display different titles for different columns
      if(!searchWinHook && isOutgoingFolder(folder))
      {
        array[1] = GetStr(MSG_To);
        array[7] = GetStr(MSG_DATE_SENT);
      }
      else if(searchWinHook || folder->Type == FT_CUSTOMMIXED || folder->Type == FT_DELETED)
      {
        array[1] = GetStr(MSG_FROMTO);
        array[7] = GetStr(MSG_DATE_SNTRCVD);
      }
      else
      {
        array[1] = GetStr(MSG_From);
        array[7] = GetStr(MSG_DATE_RECEIVED);
      }

      array[2] = GetStr(MSG_ReturnAddress);
      array[3] = GetStr(MSG_Subject);
      array[4] = GetStr(MSG_Date);
      array[5] = GetStr(MSG_Size);
      array[6] = GetStr(MSG_Filename);

      array[8] = GetStr(MSG_Folder); // The Folder is just a dummy entry to serve the SearchWindowDisplayHook
   }

   return 0;
}
MakeHook(MA_LV_DspFuncHook,MA_LV_DspFunc);

///
/// MA_GetRealSubject
//  Strips reply prefix / mailing list name from subject
static char *MA_GetRealSubject(char *sub)
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
      if(ISpace(p[1])) return MA_GetRealSubject(p+2);
      else return MA_GetRealSubject(p+1);
   }

   if (strchr(":[({", sub[2])) if ((p = strchr(sub, ':'))) return MA_GetRealSubject(TrimStart(++p));

   return sub;
}

///
/// MA_MailCompare
//  Compares two messages
static int MA_MailCompare(struct Mail *entry1, struct Mail *entry2, LONG column)
{
  switch (column)
  {
    case 0:
    {
      // lets calculate each value
      int status1 = 0;
      int status2 = 0;

      // We do not sort on other things than the real status and the Importance+Marked flag of
      // the message because this would be confusing if you use "Status" as a sorting
      // criteria within the folder config. Why should a MultiPart mail be sorted with
      // other multipart messages? It`s more important to sort just for New/Unread/Read aso
      // and then be able to sort as a second criteria for the date. Sorting the message
      // depending on other stuff than importance will make it impossible to sort for
      // status+date in the folder config. Perhaps we need to have a configuable way for
      // sorting by status later, but this is future stuff..
      status1 += hasStatusNew(entry1) ? 512 : 0;
      status2 += hasStatusNew(entry2) ? 512 : 0;
      status1 += !hasStatusRead(entry1) ? 256 : 0;
      status2 += !hasStatusRead(entry2) ? 256 : 0;
      status1 += !hasStatusError(entry1) ? 256 : 0;
      status2 += !hasStatusError(entry2) ? 256 : 0;
      status1 += hasStatusHold(entry1) ? 128 : 0;
      status2 += hasStatusHold(entry2) ? 128 : 0;
      status1 += hasStatusReplied(entry1) ? 64 : 0;
      status2 += hasStatusReplied(entry2) ? 64 : 0;
      status1 += hasStatusQueued(entry1) ? 64 : 0;
      status2 += hasStatusQueued(entry2) ? 64 : 0;
      status1 += hasStatusForwarded(entry1) ? 32 : 0;
      status2 += hasStatusForwarded(entry2) ? 32 : 0;
      status1 += hasStatusSent(entry1) ? 32 : 0;
      status2 += hasStatusSent(entry2) ? 32 : 0;
      status1 += hasStatusDeleted(entry1) ? 16 : 0;
      status2 += hasStatusDeleted(entry2) ? 16 : 0;
      status1 += hasStatusMarked(entry1) ? 8  : 0;
      status2 += hasStatusMarked(entry2) ? 8  : 0;
      status1 += (getImportanceLevel(entry1) == IMP_HIGH)  ? 16 : 0;
      status2 += (getImportanceLevel(entry2) == IMP_HIGH)  ? 16 : 0;

      return -(status1)+(status2);
    }
    break;

    case 1:
    {
      if(isOutgoingFolder(entry1->Folder))
      {
        return stricmp(*entry1->To.RealName ? entry1->To.RealName : entry1->To.Address,
                       *entry2->To.RealName ? entry2->To.RealName : entry2->To.Address);
      }
      else
      {
        return stricmp(*entry1->From.RealName ? entry1->From.RealName : entry1->From.Address,
                       *entry2->From.RealName ? entry2->From.RealName : entry2->From.Address);
      }
    }
    break;

    case 2:
    {
      return stricmp(*entry1->ReplyTo.RealName ? entry1->ReplyTo.RealName : entry1->ReplyTo.Address,
                     *entry2->ReplyTo.RealName ? entry2->ReplyTo.RealName : entry2->ReplyTo.Address);
    }
    break;

    case 3:
    {
      return stricmp(MA_GetRealSubject(entry1->Subject), MA_GetRealSubject(entry2->Subject));
    }
    break;

    case 4:
    {
      return CompareDates(&entry2->Date, &entry1->Date);
    }
    break;

    case 5:
    {
      return entry1->Size-entry2->Size;
    }
    break;

    case 6:
    {
      return strcmp(entry1->MailFile, entry2->MailFile);
    }
    break;

    case 7:
    {
      return CmpTime(&entry2->transDate, &entry1->transDate);
    }
    break;

    case 8:
    {
      return stricmp(entry1->Folder->Name, entry2->Folder->Name);
    }
    break;
  }

  return 0;
}

///
/// MA_LV_Cmp2Func
//  Message listview sort hook
HOOKPROTONHNO(MA_LV_Cmp2Func, LONG, struct NList_CompareMessage *ncm)
{
   struct Mail *entry1 = (struct Mail *)ncm->entry1;
   struct Mail *entry2 = (struct Mail *)ncm->entry2;
   LONG col1 = ncm->sort_type & MUIV_NList_TitleMark_ColMask;
   LONG col2 = ncm->sort_type2 & MUIV_NList_TitleMark2_ColMask;
   int cmp;

   if(ncm->sort_type == (LONG)MUIV_NList_SortType_None)
     return 0;

   if (ncm->sort_type & MUIV_NList_TitleMark_TypeMask) cmp = MA_MailCompare(entry2, entry1, col1);
   else                                                cmp = MA_MailCompare(entry1, entry2, col1);

   if (cmp || col1 == col2) return cmp;
   if (ncm->sort_type2 & MUIV_NList_TitleMark2_TypeMask) cmp = MA_MailCompare(entry2, entry1, col2);
   else                                                  cmp = MA_MailCompare(entry1, entry2, col2);
   return cmp;
}
MakeHook(MA_LV_Cmp2Hook, MA_LV_Cmp2Func);

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

  // we get the first entry and if it`s a LIST we have to get the next one
  // and so on, until we have a real entry for that key or we set nothing active
  for(i=0; i <= count; i++)
  {
    tn = (struct MUI_NListtree_TreeNode *)DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NListtree_GetEntry, MUIV_NListtree_GetEntry_ListNode_Root, i, MUIF_NONE);
    if(!tn) return;

    if(isFlagSet(tn->tn_Flags, TNF_LIST)) count++;
  }

  // Force that the list is open at this entry
  DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NListtree_Open, MUIV_NListtree_Open_ListNode_Parent, tn, MUIF_NONE);

  // Now set this treenode activ
  set(G->MA->GUI.NL_FOLDERS, MUIA_NListtree_Active, tn);
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
   static const char *shortcuts[10] = { "0","1","2","3","4","5","6","7","8","9" };
   int i;

   /* Scripts menu */

   if (G->MA->GUI.MN_REXX)
      DoMethod(G->MA->GUI.MS_MAIN, MUIM_Family_Remove, G->MA->GUI.MN_REXX);

   G->MA->GUI.MN_REXX = MenuObject,
      MUIA_Menu_Title, GetStr(MSG_MA_Scripts),
      MUIA_Family_Child, MenuitemObject,
         MUIA_Menuitem_Title, GetStr(MSG_MA_ExecuteScript),
         MUIA_Menuitem_Shortcut, ".", MUIA_UserData, MMEN_SCRIPT,
         End,
      MUIA_Family_Child, MenuitemObject,
         MUIA_Menuitem_Title, NM_BARLABEL,
         End,
      End;

   for (i = 0; i < 10; i++) if (C->RX[i].Script[0])
      DoMethod(G->MA->GUI.MN_REXX, MUIM_Family_AddTail, MenuitemObject,
         MUIA_Menuitem_Title, C->RX[i].Name,
         MUIA_Menuitem_Shortcut, shortcuts[i],
         MUIA_UserData, MMEN_MACRO+i, End);

   DoMethod(G->MA->GUI.MS_MAIN, MUIM_Family_AddTail, G->MA->GUI.MN_REXX);

   /* 'Folder/Check single account' menu */

   if (G->MA->GUI.MI_CSINGLE)
      DoMethod(G->MA->GUI.MN_FOLDER, MUIM_Family_Remove, G->MA->GUI.MI_CSINGLE);

   G->MA->GUI.MI_CSINGLE = MenuitemObject,
      MUIA_Menuitem_Title, GetStr(MSG_MA_CheckSingle),
      End;

   for (i = 0; i < MAXP3; i++) if (C->P3[i])
   {
      sprintf(C->P3[i]->Account, "%s@%s", C->P3[i]->User, C->P3[i]->Server);

      /* Warning: Small memory leak here, each time this function is called,
                  since the strdup()'ed string doesn't get free()'d anywhere,
                  before program exit. The Menuitem class does *not* have a
                  private buffer for the string!
      */

      DoMethod(G->MA->GUI.MI_CSINGLE, MUIM_Family_AddTail, MenuitemObject, MUIA_Menuitem_Title, strdup(C->P3[i]->Account), MUIA_UserData,MMEN_POPHOST+i, End, TAG_DONE);

   }
   DoMethod(G->MA->GUI.MN_FOLDER, MUIM_Family_AddTail, G->MA->GUI.MI_CSINGLE, TAG_DONE);
}

///
/// MA_SetupEmbeddedReadPane()
//  Updates/Setup the embedded read pane part in the main window
void MA_SetupEmbeddedReadPane(void)
{
  Object *mailViewGroup  = G->MA->GUI.GR_MAILVIEW;
  Object *mailBalanceObj = G->MA->GUI.BL_MAILVIEW;
  Object *readPaneObj    = G->MA->GUI.MN_EMBEDDEDREADPANE;

  // check wheter the embedded read pane object is already embeeded in our main
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
        G->MA->GUI.MN_EMBEDDEDREADPANE = readPaneObj = ReadMailGroupObject, End;
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
                                                    G->MA->GUI.BC_GROUP,
                                                    G->MA->GUI.GR_BOTTOM,
                                                    NULL);
    }
    break;

    case IB_POS_CENTER:
    {
      DoMethod(G->MA->GUI.GR_MAIN, MUIM_Group_Sort, G->MA->GUI.GR_TOP,
                                                    G->MA->GUI.BC_GROUP,
                                                    G->MA->GUI.IB_INFOBAR,
                                                    G->MA->GUI.GR_BOTTOM,
                                                    NULL);
    }
    break;

    case IB_POS_BOTTOM:
    {
      DoMethod(G->MA->GUI.GR_MAIN, MUIM_Group_Sort, G->MA->GUI.GR_TOP,
                                                    G->MA->GUI.BC_GROUP,
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
/// MA_MakeMAFormat
//  Creates format definition for message listview
void MA_MakeMAFormat(Object *lv)
{
   static const int defwidth[MACOLNUM] = { -1,-1,-1,-1,-1,-1,-1,-1 };
   char format[SIZE_LARGE];
   BOOL first = TRUE;
   int i;

   *format = 0;
   for(i = 0; i < MACOLNUM; i++)
   {
      if(C->MessageCols & (1<<i))
      {
          if(first)
            first = FALSE;
          else
            strcat(format, " BAR,");

          sprintf(&format[strlen(format)], "COL=%d W=%d", i, defwidth[i]);

          if(i == 5)
            strcat(format, " P=\033r");
      }
   }
   strcat(format, " BAR");

   set(lv, MUIA_NList_Format, format);
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

      sprintf(data->WinTitle, GetStr(MSG_MA_WinTitle), yamversionver, username);

      data->GUI.MS_MAIN = MenustripObject,
         MUIA_Family_Child, MenuObject, MUIA_Menu_Title, GetStr(MSG_MA_Project),
            MUIA_Family_Child, MakeMenuitem(GetStr(MSG_PROJECT_ABOUT), MMEN_ABOUT),
            MUIA_Family_Child, MakeMenuitem(GetStr(MSG_MA_AboutMUI), MMEN_ABOUTMUI),
            MUIA_Family_Child, MakeMenuitem(GetStr(MSG_PROJECT_CHECKVERSION), MMEN_VERSION),
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
            MUIA_Family_Child, MakeMenuitem(GetStr(MSG_MA_MFilter), MMEN_FILTER),
            MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title, NM_BARLABEL, End,
            MUIA_Family_Child, MakeMenuitem(GetStr(MSG_MA_RemoveDeleted), MMEN_DELDEL),
            MUIA_Family_Child, MakeMenuitem(GetStr(MSG_MA_UpdateIndex), MMEN_INDEX),
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
            MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title, GetStr(MSG_MA_Select),
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
            Child, data->GUI.BC_GROUP = HGroup,
               MUIA_ShowMe, FALSE,
               // Create the status flag image objects
               Child, data->GUI.BC_STAT[SICON_ID_UNREAD]   = MakeBCImage("status_unread"),
               Child, data->GUI.BC_STAT[SICON_ID_OLD]      = MakeBCImage("status_old"),
               Child, data->GUI.BC_STAT[SICON_ID_FORWARD]  = MakeBCImage("status_forward"),
               Child, data->GUI.BC_STAT[SICON_ID_REPLY]    = MakeBCImage("status_reply"),
               Child, data->GUI.BC_STAT[SICON_ID_WAITSEND] = MakeBCImage("status_waitsend"),
               Child, data->GUI.BC_STAT[SICON_ID_ERROR]    = MakeBCImage("status_error"),
               Child, data->GUI.BC_STAT[SICON_ID_HOLD]     = MakeBCImage("status_hold"),
               Child, data->GUI.BC_STAT[SICON_ID_SENT]     = MakeBCImage("status_sent"),
               Child, data->GUI.BC_STAT[SICON_ID_NEW]      = MakeBCImage("status_new"),
               Child, data->GUI.BC_STAT[SICON_ID_DELETE]   = MakeBCImage("status_delete"),
               Child, data->GUI.BC_STAT[SICON_ID_DOWNLOAD] = MakeBCImage("status_download"),
               Child, data->GUI.BC_STAT[SICON_ID_GROUP]    = MakeBCImage("status_group"),
               Child, data->GUI.BC_STAT[SICON_ID_URGENT]   = MakeBCImage("status_urgent"),
               Child, data->GUI.BC_STAT[SICON_ID_ATTACH]   = MakeBCImage("status_attach"),
               Child, data->GUI.BC_STAT[SICON_ID_REPORT]   = MakeBCImage("status_report"),
               Child, data->GUI.BC_STAT[SICON_ID_CRYPT]    = MakeBCImage("status_crypt"),
               Child, data->GUI.BC_STAT[SICON_ID_SIGNED]   = MakeBCImage("status_signed"),
               Child, data->GUI.BC_STAT[SICON_ID_MARK]     = MakeBCImage("status_mark"),
               // Create the default folder image objects
               Child, data->GUI.BC_FOLDER[0] = MakeBCImage("folder_fold"),
               Child, data->GUI.BC_FOLDER[1] = MakeBCImage("folder_unfold"),
               Child, data->GUI.BC_FOLDER[2] = MakeBCImage("folder_incoming"),
               Child, data->GUI.BC_FOLDER[3] = MakeBCImage("folder_incoming_new"),
               Child, data->GUI.BC_FOLDER[4] = MakeBCImage("folder_outgoing"),
               Child, data->GUI.BC_FOLDER[5] = MakeBCImage("folder_outgoing_new"),
               Child, data->GUI.BC_FOLDER[6] = MakeBCImage("folder_deleted"),
               Child, data->GUI.BC_FOLDER[7] = MakeBCImage("folder_deleted_new"),
               Child, data->GUI.BC_FOLDER[8] = MakeBCImage("folder_sent"),
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
                 Child, data->GUI.LV_MAILS = NListviewObject,
                    MUIA_HelpNode,   "MA01",
                    MUIA_VertWeight, 25,
                    MUIA_CycleChain, TRUE,
                    MUIA_NListview_NList, data->GUI.NL_MAILS = MainMailListObject,
                       MUIA_NList_MinColSortable, 0,
                       MUIA_NList_TitleClick          , TRUE,
                       MUIA_NList_TitleClick2         , TRUE,
                       MUIA_NList_DragType            , MUIV_NList_DragType_Default,
                       MUIA_NList_MultiSelect         , MUIV_NList_MultiSelect_Default,
                       MUIA_NList_CompareHook2        , &MA_LV_Cmp2Hook,
                       MUIA_NList_DisplayHook2        , &MA_LV_DspFuncHook,
                       MUIA_NList_AutoVisible         , TRUE,
                       MUIA_NList_Title               , TRUE,
                       MUIA_NList_TitleSeparator      , TRUE,
                       MUIA_NList_DefaultObjectOnClick, FALSE,
                       MUIA_ContextMenu               , C->MessageCntMenu ? MUIV_NList_ContextMenu_Always : 0,
                       MUIA_Font                      , C->FixedFontList ? MUIV_NList_Font_Fixed : MUIV_NList_Font,
                       MUIA_NList_Exports             , MUIV_NList_Exports_ColWidth|MUIV_NList_Exports_ColOrder,
                       MUIA_NList_Imports             , MUIV_NList_Imports_ColWidth|MUIV_NList_Imports_ColOrder,
                       MUIA_ObjectID                  , MAKE_ID('N','L','0','2'),
                    End,
                 End,
               End,
            End,
         End,
      End;

      if (data->GUI.WI)
      {
         MA_MakeFOFormat(data->GUI.NL_FOLDERS);
         MA_MakeMAFormat(data->GUI.NL_MAILS);
         DoMethod(G->App, OM_ADDMEMBER, data->GUI.WI);

         // define the StatusFlag images that should be used
         for(i = 0; i < MAXBCSTATUSIMG; i++)
           DoMethod(data->GUI.NL_MAILS, MUIM_NList_UseImage, data->GUI.BC_STAT[i], i, MUIF_NONE);

         // Define the Images the FolderListtree that can be used
         for(i = 0; i < MAXBCFOLDERIMG; i++)
           DoMethod(data->GUI.NL_FOLDERS, MUIM_NList_UseImage, data->GUI.BC_FOLDER[i], i, MUIF_NONE);

         // Now we need the XPK image also in the folder list
         DoMethod(data->GUI.NL_FOLDERS, MUIM_NList_UseImage, data->GUI.BC_STAT[15], MAXBCFOLDERIMG, MUIF_NONE);

         set(data->GUI.WI,MUIA_Window_DefaultObject,data->GUI.NL_MAILS);
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
         DoMethod(data->GUI.WI             ,MUIM_Notify,MUIA_Window_MenuAction   ,MMEN_SELALL    ,data->GUI.NL_MAILS,4,MUIM_NList_Select,MUIV_NList_Select_All,MUIV_NList_Select_On,NULL);
         DoMethod(data->GUI.WI             ,MUIM_Notify,MUIA_Window_MenuAction   ,MMEN_SELNONE   ,data->GUI.NL_MAILS,4,MUIM_NList_Select,MUIV_NList_Select_All,MUIV_NList_Select_Off,NULL);
         DoMethod(data->GUI.WI             ,MUIM_Notify,MUIA_Window_MenuAction   ,MMEN_SELTOGG   ,data->GUI.NL_MAILS,4,MUIM_NList_Select,MUIV_NList_Select_All,MUIV_NList_Select_Toggle,NULL);
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
         DoMethod(data->GUI.NL_MAILS       ,MUIM_Notify,MUIA_NList_DoubleClick   ,MUIV_EveryTime,MUIV_Notify_Application  ,3,MUIM_CallHook            ,&MA_ReadMessageHook,FALSE);
         DoMethod(data->GUI.NL_MAILS       ,MUIM_Notify,MUIA_NList_TitleClick    ,MUIV_EveryTime,MUIV_Notify_Self         ,4,MUIM_NList_Sort3         ,MUIV_TriggerValue,MUIV_NList_SortTypeAdd_2Values,MUIV_NList_Sort3_SortType_Both);
         DoMethod(data->GUI.NL_MAILS       ,MUIM_Notify,MUIA_NList_TitleClick2   ,MUIV_EveryTime,MUIV_Notify_Self         ,4,MUIM_NList_Sort3         ,MUIV_TriggerValue,MUIV_NList_SortTypeAdd_2Values,MUIV_NList_Sort3_SortType_2);
         DoMethod(data->GUI.NL_MAILS       ,MUIM_Notify,MUIA_NList_SortType      ,MUIV_EveryTime,MUIV_Notify_Self         ,3,MUIM_Set                 ,MUIA_NList_TitleMark,MUIV_TriggerValue);
         DoMethod(data->GUI.NL_MAILS       ,MUIM_Notify,MUIA_NList_SortType2     ,MUIV_EveryTime,MUIV_Notify_Self         ,3,MUIM_Set                 ,MUIA_NList_TitleMark2,MUIV_TriggerValue);
         DoMethod(data->GUI.NL_MAILS       ,MUIM_Notify,MUIA_NList_SelectChange  ,TRUE          ,MUIV_Notify_Application  ,2,MUIM_CallHook            ,&MA_ChangeSelectedHook);
         //DoMethod(data->GUI.NL_MAILS       ,MUIM_Notify,MUIA_NList_Active        ,MUIV_EveryTime,MUIV_Notify_Application  ,2,MUIM_CallHook            ,&MA_ChangeSelectedHook);
         DoMethod(data->GUI.NL_MAILS       ,MUIM_Notify,MUIA_NList_Active        ,MUIV_EveryTime,MUIV_Notify_Application  ,2,MUIM_CallHook            ,&MA_SetMessageInfoHook);
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
