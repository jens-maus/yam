/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2003 by YAM Open Source Team

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

#include <clib/alib_protos.h>
#include <libraries/asl.h>
#include <libraries/iffparse.h>
#include <libraries/gadtools.h>
#include <mui/NList_mcc.h>
#include <mui/NListview_mcc.h>
#include <mui/TextEditor_mcc.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/muimaster.h>
#include <proto/utility.h>

#include "extra.h"
#include "SDI_hook.h"

#include "YAM.h"
#include "YAM_addressbook.h"
#include "YAM_addressbookEntry.h"
#include "YAM_classes.h"
#include "YAM_config.h"
#include "YAM_debug.h"
#include "YAM_error.h"
#include "YAM_folderconfig.h"
#include "YAM_global.h"
#include "YAM_locale.h"
#include "YAM_mail_lex.h"
#include "YAM_main.h"
#include "YAM_mainFolder.h"
#include "YAM_mime.h"
#include "YAM_read.h"
#include "YAM_write.h"
#include "YAM_utilities.h"
#include "classes/Classes.h"

// PGP flags & macros
#define PGPE_MIME     (1<<0)
#define PGPE_OLD      (1<<1)
#define hasPGPEMimeFlag(v)     (isFlagSet((v)->PGPEncrypted, PGPE_MIME))
#define hasPGPEOldFlag(v)      (isFlagSet((v)->PGPEncrypted, PGPE_OLD))

#define PGPS_MIME     (1<<0)
#define PGPS_OLD      (1<<1)
#define PGPS_BADSIG   (1<<2)
#define PGPS_ADDRESS  (1<<3)
#define PGPS_CHECKED  (1<<4)
#define hasPGPSMimeFlag(v)     (isFlagSet((v)->PGPSigned, PGPS_MIME))
#define hasPGPSOldFlag(v)      (isFlagSet((v)->PGPSigned, PGPS_OLD))
#define hasPGPSBadSigFlag(v)   (isFlagSet((v)->PGPSigned, PGPS_BADSIG))
#define hasPGPSAddressFlag(v)  (isFlagSet((v)->PGPSigned, PGPS_ADDRESS))
#define hasPGPSCheckedFlag(v)  (isFlagSet((v)->PGPSigned, PGPS_CHECKED))

/* local protos */
static BOOL RE_LoadMessage(int winnum, int parsemode);
static struct RE_ClassData *RE_New(int winnum, BOOL real);
static void RE_DisplayMessage(int winnum, BOOL update);
static void RE_PrintFile(char*,struct Part*);
static void RE_PrintLaTeX(char*,struct Part*);
static void RE_GetSigFromLog(int winnum, char *decrFor);
#ifdef UNUSED
static char **Init_ISO8859_to_LaTeX_Tab(char*);
static char *ISO8859_to_LaTeX(char*);
#endif

/***************************************************************************
 Module: Read
***************************************************************************/

/// RE_GetThread
//  Function that find the next/prev message in a thread and returns a pointer to it
static struct Mail *RE_GetThread(struct Mail *srcMail, BOOL nextThread, BOOL askLoadAllFolder, int winnum)
{
   struct Folder **flist;
   struct Mail *mail = NULL;
   BOOL found = FALSE;

   if(srcMail)
   {
      // first we take the folder of the srcMail as a priority in the
      // search of the next/prev thread so we have to check that we
      // have a valid index before we are going to go on.
      if(srcMail->Folder->LoadedMode != LM_VALID && MA_GetIndex(srcMail->Folder))
      {
        return NULL;
      }

      // ok the folder is valid and we can scan it now
      for(mail = srcMail->Folder->Messages; mail; mail = mail->Next)
      {
        if(nextThread) // find the answer to the srcMail
        {
          if(mail->cIRTMsgID && mail->cIRTMsgID == srcMail->cMsgID)
          {
            found = TRUE;
            break;
          }
        }
        else // else we have to find the question to the srcMail
        {
          if(mail->cMsgID && mail->cMsgID == srcMail->cIRTMsgID)
          {
            found = TRUE;
            break;
          }
        }
      }

      // if we still haven`t found the mail we have to scan the other folder aswell
      if(!found && (flist = FO_CreateList()))
      {
        int i;
        int autoloadindex = -1;

        for(i=1; i <= (int)*flist && !found; i++)
        {
          // we already scanned this folder so lets skip it.
          if(flist[i] != srcMail->Folder)
          {
            if(flist[i]->LoadedMode != LM_VALID)
            {
              if(autoloadindex == -1)
              {
                if(askLoadAllFolder)
                {
                  char *str = GetStr(MSG_RE_FollowThreadReq); // don`t remove, this is a SAS/C bug workaround !
                  if(MUI_Request(G->App, G->RE[winnum]->GUI.WI, 0, GetStr(MSG_MA_ConfirmReq), GetStr(MSG_YesNoReq), str))
                  {
                    autoloadindex = 1;
                  }
                  else autoloadindex = 0;
                }
                else autoloadindex = 0;
              }

              // we have to check again and perhaps load the index or continue
              if(autoloadindex == 1)
              {
                if(!MA_GetIndex(flist[i])) continue;
              }
              else continue;
            }

            // if we end up here we have a valid index and can scan the folder
            for(mail = flist[i]->Messages; mail; mail = mail->Next)
            {
              if(nextThread) // find the answer to the srcMail
              {
                if(mail->cIRTMsgID && mail->cIRTMsgID == srcMail->cMsgID)
                {
                  found = TRUE;
                  break;
                }
              }
              else // else we have to find the question to the srcMail
              {
                if(mail->cMsgID && mail->cMsgID == srcMail->cIRTMsgID)
                {
                  found = TRUE;
                  break;
                }
              }
            }
          }
        }
      }
   }

   return mail;
}

///
/// RE_Follow
//  Follows a thread in either direction
HOOKPROTONHNO(RE_Follow, void, int *arg)
{
   int direction = arg[0], winnum = arg[1];
   struct Mail *fmail;

   // depending on the direction we get the Question or Answer to the current Message
   fmail = RE_GetThread(&G->RE[winnum]->Mail, direction <= 0 ? FALSE : TRUE, TRUE, winnum);

   if(fmail)
   {
      struct MailInfo *mi;

      // we have to make sure that the folder where the message will be showed
      // from is active and ready to display the mail
      MA_ChangeFolder(fmail->Folder, TRUE);

      mi = GetMailInfo(fmail);
      if(mi->Display) set(G->MA->GUI.NL_MAILS, MUIA_NList_Active, mi->Pos);
      RE_ReadMessage(winnum, fmail);
   }
   else DisplayBeep(0);
}
MakeStaticHook(RE_FollowHook, RE_Follow);
///
/// RE_SwitchMessage
//  Goes to next or previous (new) message in list
static void RE_SwitchMessage(int winnum, int direction, BOOL onlynew)
{
   struct Mail *mail = G->RE[winnum]->MailPtr;
   struct MailInfo *mi;
   int act;
   struct Folder *CurrentFolder = mail->Folder;

   G->RE[winnum]->LastDirection = direction;

   // we have to make sure that the folder the next/prev mail will
   // be showed from is active, that`s why we call ChangeFolder with TRUE.
   MA_ChangeFolder(CurrentFolder, TRUE);

   // after changing the folder we have to get the MailInfo (Position etc.)
   mi = GetMailInfo(mail);
   act = mi->Pos;

   for (act += direction; act >= 0; act += direction)
   {
      DoMethod(G->MA->GUI.NL_MAILS, MUIM_NList_GetEntry, act, &mail);
      if (!mail) break;
      if (!onlynew || (mail->Status == STATUS_NEW || mail->Status == STATUS_UNR))
      {
         set(G->MA->GUI.NL_MAILS, MUIA_NList_Active, act);
         RE_ReadMessage(winnum, mail);
         return;
      }
   }

   // check if there are following/previous folders with unread
   // mails and change to there if the user wants
   if (onlynew)
   {
      if (C->AskJumpUnread)
      {
         struct Folder **flist;

         if ((flist = FO_CreateList()))
         {
            int i;

            // look for the current folder in the array
            for (i = 1; i <= (int)*flist; i++)
            {
               if (flist[i] == CurrentFolder)
                  break;
            }

            // look for first folder with at least one unread mail
            // and if found read that mail
            for (i += direction; i <= (int)*flist && i >= 1; i += direction)
            {
               if (flist[i]->Type != FT_GROUP && flist[i]->Unread > 0)
               {
                  if (!MUI_Request(G->App, G->RE[winnum]->GUI.WI, 0, GetStr(MSG_MA_ConfirmReq), GetStr(MSG_YesNoReq), GetStr(MSG_RE_MoveNextFolderReq), flist[i]->Name))
                     break;

                  MA_ChangeFolder(flist[i], TRUE);
                  DoMethod(G->MA->GUI.NL_MAILS, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &mail);
                  if (!mail) break;
                  RE_ReadMessage(winnum, mail);
                  break;
               }
            }

            // beep if no folder with unread mails was found
            if (i > (int)*flist || i < 1)
               DisplayBeep(NULL);

            free(flist);
         }
      }
      else DisplayBeep(NULL);
   }
   else DoMethod(G->App, MUIM_CallHook, &RE_CloseHook, winnum);
}
///
/// RE_PrevNext
//  Goes to next or previous (new) message in list
HOOKPROTONHNO(RE_PrevNext, void, int *arg)
{
   BOOL onlynew = hasFlag(arg[1], (IEQUALIFIER_LSHIFT|IEQUALIFIER_RSHIFT));
   RE_SwitchMessage(arg[2], arg[0], onlynew);
}
MakeStaticHook(RE_PrevNextHook, RE_PrevNext);
///
/// RE_UpdateDisplay
//  Updates message display after deleting/moving the current message
static void RE_UpdateDisplay(int pos, int winnum)
{
   struct Mail *mail = NULL;

   if (G->RE[winnum]->LastDirection == -1) --pos;
   if (pos >= 0)
   {
      DoMethod(G->MA->GUI.NL_MAILS, MUIM_NList_GetEntry, pos, &mail);
      if (mail)
      {
         set(G->MA->GUI.NL_MAILS, MUIA_NList_Active, pos);
         RE_ReadMessage(winnum, mail);
         return;
      }
   }
   DoMethod(G->App, MUIM_CallHook, &RE_CloseHook, winnum);
}
///
/// RE_UpdateStatusGroup
//  Updates status images (right side of the toolbar)
static void RE_UpdateStatusGroup(int winnum)
{
   struct RE_ClassData *re = G->RE[winnum];
   struct RE_GUIData *gui = &re->GUI;
   struct Mail *mail = re->MailPtr;
   int    status = 0;

   // set the correct page for the mail Status group
   set(gui->GR_STATUS[0], MUIA_Group_ActivePage, 1+mail->Status);

   // Now we check for the other statuses of the mail
   if(isCryptedMail(mail))        status = 1;
   else if(isSignedMail(mail))    status = 2;
   else if(isReportMail(mail))    status = 3;
   else if(isMultiPartMail(mail)) status = 4;
   set(gui->GR_STATUS[1], MUIA_Group_ActivePage, status);

   // set the correct page for the Importance flag
   set(gui->GR_STATUS[2], MUIA_Group_ActivePage, mail->Importance == 1 ? 1 : 0);

   // set the correct page for the Marked flag
   set(gui->GR_STATUS[3], MUIA_Group_ActivePage, isMarkedMail(mail) ? 1 : 0);
}
///
/// RE_SendMDN
//  Creates a message disposition notification
static void RE_SendMDN(int MDNtype, struct Mail *mail, struct Person *recipient, BOOL sendnow)
{
   static const char *MDNMessage[5] = {
      "The message written on %s (UTC) to %s with subject \"%s\" has been displayed. This is no guarantee that the content has been read or understood.\n",
      "The message written on %s (UTC) to %s with subject \"%s\" has been sent somewhere %s, without being displayed to the user. The user may or may not see the message later.\n",
      "The message written on %s (UTC) to %s with subject \"%s\" has been processed %s, without being displayed to the user. The user may or may not see the message later.\n",
      "The message written on %s (UTC) to %s with subject \"%s\" has been deleted %s. The recipient may or may not have seen the message. The recipient may \"undelete\" the message at a later time and read the message.\n",
      "%s doesn't wish to inform you about the disposition of your message written on %s with subject \"%s\".\n"
   };
   struct WritePart *p1 = NewPart(2), *p2, *p3;
   struct TempFile *tf1, *tf2, *tf3;
   char buf[SIZE_LINE], disp[SIZE_DEFAULT], *mode;
   struct Compose comp;

   if ((tf1 = OpenTempFile("w")))
   {
      char *date = DateStamp2String(&mail->Date, DSS_DATETIME, TZC_NONE), *rcpt = BuildAddrName2(&mail->To), *subj = mail->Subject;
      p1->Filename = tf1->Filename;
      mode = isAutoActMDN(MDNtype) ? "automatically" : "in response to a user command";
      strcpy(disp, isAutoActMDN(MDNtype) ? "automatic-action/" : "manual-action/");
      strcat(disp, isAutoSendMDN(MDNtype) ? "MDN-sent-automatically; " : "MDN-sent-manually; ");
      switch (MDNtype & MDN_TYPEMASK)
      {
         case MDN_READ: strcat(disp, "displayed");  fprintf(tf1->FP, MDNMessage[0], date, rcpt, subj); break;
         case MDN_DISP: strcat(disp, "dispatched"); fprintf(tf1->FP, MDNMessage[1], date, rcpt, subj, mode); break;
         case MDN_PROC: strcat(disp, "processed");  fprintf(tf1->FP, MDNMessage[2], date, rcpt, subj, mode); break;
         case MDN_DELE: strcat(disp, "deleted");    fprintf(tf1->FP, MDNMessage[3], date, rcpt, subj, mode); break;
         case MDN_DENY: strcat(disp, "denied");     fprintf(tf1->FP, MDNMessage[4], rcpt, date, subj); break;
      }
      fclose(tf1->FP); tf1->FP = NULL;
      SimpleWordWrap(tf1->Filename, 72);
      p2 = p1->Next = NewPart(2);
      if ((tf2 = OpenTempFile("w")))
      {
         char mfile[SIZE_MFILE];
         struct Folder *outfolder = FO_GetFolderByType(FT_OUTGOING, NULL);
         struct ExtendedMail *email = MA_ExamineMail(mail->Folder, mail->MailFile, NULL, TRUE);

         if(email)
         {
            p2->ContentType = "message/disposition-notification";
            p2->Filename = tf2->Filename;
            sprintf(buf, "%s (%s)", C->SMTP_Domain, yamversion);
            EmitHeader(tf2->FP, "Reporting-UA", buf);
            if (*email->OriginalRcpt.Address)
            {
              sprintf(buf, "rfc822;%s", BuildAddrName2(&email->OriginalRcpt));
              EmitHeader(tf2->FP, "Original-Recipient", buf);
            }
            sprintf(buf, "rfc822;%s", BuildAddrName(C->EmailAddress, C->RealName));
            EmitHeader(tf2->FP, "Final-Recipient", buf);
            EmitHeader(tf2->FP, "Original-Message-ID", email->MsgID);
            EmitHeader(tf2->FP, "Disposition", disp);
            fclose(tf2->FP);  tf2->FP = NULL;
            p3 = p2->Next = NewPart(2);

            MA_FreeEMailStruct(email);

            if ((tf3 = OpenTempFile("w")))
            {
              char fullfile[SIZE_PATHFILE];
              FILE *fh;
              p3->ContentType = "text/rfc822-headers";
              p3->Filename = tf3->Filename;
              if (StartUnpack(GetMailFile(NULL, mail->Folder, mail), fullfile, mail->Folder))
              {
                if ((fh = fopen(fullfile, "r")))
                {
                  while (fgets(buf, SIZE_LINE, fh)) if (*buf == '\n') break; else fputs(buf, tf3->FP);
                  fclose(fh);
                }
                FinishUnpack(fullfile);
              }
              fclose(tf3->FP); tf3->FP = NULL;
              memset(&comp, 0, sizeof(struct Compose));
              comp.MailTo = StrBufCpy(comp.MailTo, BuildAddrName2(recipient));
              comp.Subject = "Disposition Notification";
              comp.ReportType = 1;
              comp.FirstPart = p1;
              if ((comp.FH = fopen(MA_NewMailFile(outfolder, mfile, 0), "w")))
              {
                struct Mail *mlist[3];
                mlist[0] = (struct Mail *)1; mlist[2] = NULL;
                WriteOutMessage(&comp);
                fclose(comp.FH);
                if ((email = MA_ExamineMail(outfolder, mfile, NULL, TRUE)))
                {
                  email->Mail.Status = STATUS_WFS;  // Set "WaitForSend" status
                  mlist[2] = AddMailToList(&email->Mail, outfolder);
                  MA_FreeEMailStruct(email);
                }
                if (sendnow && mlist[2] && !G->TR) MA_SendMList(mlist);
              }
              else ER_NewError(GetStr(MSG_ER_CreateMailError), NULL, NULL);
              FreeStrBuf(comp.MailTo);
              CloseTempFile(tf3);
            }
         }
         CloseTempFile(tf2);
      }
      CloseTempFile(tf1);
   }
   FreePartsList(p1);
}
///
/// RE_DoMDN
//  Handles message disposition requests
BOOL RE_DoMDN(int MDNtype, struct Mail *mail, BOOL multi)
{
   BOOL ignoreall = FALSE;
   int MDNmode;
   switch (MDNtype)
   {
      case MDN_READ: MDNmode = C->MDN_Display; break;
      case MDN_PROC:
      case MDN_DISP: MDNmode = C->MDN_Process; break;
      case MDN_DELE: MDNmode = C->MDN_Delete; break;
      default:       MDNmode = C->MDN_Filter; break;
   }
   if (MDNmode)
   {
      struct ExtendedMail *email = MA_ExamineMail(mail->Folder, mail->MailFile, NULL, TRUE);
      if(email)
      {
        if(*email->ReceiptTo.Address)
        {
          char buttons[SIZE_DEFAULT*2];
          BOOL isonline = TR_IsOnline(), sendnow = C->SendMDNAtOnce && isonline;
          switch (MDNmode)
          {
            case 1: MDNtype = MDN_DENY|MDN_AUTOSEND|MDN_AUTOACT; break;
            case 2: sendnow = FALSE;
                    strcpy(buttons, GetStr(MSG_RE_MDNGads1));
                    if (isonline) strcat(buttons, GetStr(MSG_RE_MDNGads2));
                    strcat(buttons, GetStr(MSG_RE_MDNGads3));
                    if (multi) strcat(buttons, GetStr(MSG_RE_MDNGads4));
                    switch (MUI_Request(G->App, G->MA->GUI.WI, 0, GetStr(MSG_MA_ConfirmReq), buttons, GetStr(MSG_RE_MDNReq)))
                    {
                       case 0: ignoreall = TRUE;
                       case 5: MDNtype = MDN_IGNORE; break;
                       case 3: sendnow = TRUE;
                       case 1: break;
                       case 4: sendnow = TRUE;
                       case 2: MDNtype = MDN_DENY; break;
                    }
                    break;
            case 3: if (MDNtype != MDN_IGNORE) SET_FLAG(MDNtype, MDN_AUTOSEND); break;
          }
          if (MDNtype != MDN_IGNORE) RE_SendMDN(MDNtype, mail, &email->ReceiptTo, sendnow);
        }
        MA_FreeEMailStruct(email);
      }
   }
   return ignoreall;
}
///
/// RE_CheckSignatureFunc
//  Checks validity of a PGP signed message
HOOKPROTONHNO(RE_CheckSignatureFunc, void, int *arg)
{
   struct RE_ClassData *re = G->RE[arg[1]];

   // Don't try to use PGP if it's not installed
   if (G->PGPVersion == 0) return;

   if((hasPGPSOldFlag(re) || hasPGPSMimeFlag(re)) && !hasPGPSCheckedFlag(re))
   {
      int error;
      char fullfile[SIZE_PATHFILE], options[SIZE_LARGE];
      if (!StartUnpack(GetMailFile(NULL, NULL, re->MailPtr), fullfile, re->MailPtr->Folder)) return;
      sprintf(options, (G->PGPVersion == 5) ? "%s -o %s +batchmode=1 +force +language=us" : "%s -o %s +bat +f +lang=en", fullfile, "T:PGP.tmp");
      error = PGPCommand((G->PGPVersion == 5) ? "pgpv": "pgp", options, KEEPLOG);
      FinishUnpack(fullfile);
      DeleteFile("T:PGP.tmp");
      if(error > 0) SET_FLAG(re->PGPSigned, PGPS_BADSIG);
      if(error >= 0) RE_GetSigFromLog(arg[1], NULL);
      else return;
   }

   if(hasPGPSBadSigFlag(re) || arg[0])
   {
      char buffer[SIZE_LARGE];
      strcpy(buffer, hasPGPSBadSigFlag(re) ? GetStr(MSG_RE_BadSig) : GetStr(MSG_RE_GoodSig));
      if(hasPGPSAddressFlag(re)) { strcat(buffer, GetStr(MSG_RE_SigFrom)); strcat(buffer, re->Signature); }
      MUI_Request(G->App, re->GUI.WI, 0, GetStr(MSG_RE_SigCheck), GetStr(MSG_Okay), buffer);
   }
}
MakeStaticHook(RE_CheckSignatureHook, RE_CheckSignatureFunc);
///
/// RE_ReadMessage
//  Displays a message in the read window
void RE_ReadMessage(int winnum, struct Mail *mail)
{
   struct MailInfo *mi = GetMailInfo(mail);
   struct RE_ClassData *re = G->RE[winnum];
   struct RE_GUIData *gui;
   struct Folder *folder = mail->Folder;
   BOOL real = !isVirtualMail(mail);
   BOOL out = real ? isOutgoingFolder(folder) : FALSE;

   /* Check if the window is still open,
    * needed for the "update readwindow after writewindow close" feature
    */
   if (re == NULL)
   {
      return;
   }

   // lets clean the previous mail out of the window if exists to make the
   // window ready for the new message
   RE_CleanupMessage(winnum);

   gui = &re->GUI;
   re->Mail = *mail;
   re->MailPtr = mail;
   re->PGPKey = FALSE;
   re->PGPSigned = re->PGPEncrypted = 0;

   sprintf(re->WTitle, "%s %s %s: ", mail->MailFile, out ? GetStr(MSG_To) : GetStr(MSG_From), out ? AddrName(mail->To) : AddrName(mail->From));
   stccat(re->WTitle, mail->Subject, SIZE_DEFAULT);
   set(gui->WI, MUIA_Window_Title, re->WTitle);
   set(gui->MI_EDIT, MUIA_Menuitem_Enabled, out);
   if (gui->TO_TOOLBAR)
   {
      DoMethod(gui->TO_TOOLBAR, MUIM_Toolbar_Set, 0, MUIV_Toolbar_Set_Ghosted, real ? mi->Pos == 0 : TRUE);
      DoMethod(gui->TO_TOOLBAR, MUIM_Toolbar_Set, 1, MUIV_Toolbar_Set_Ghosted, real ? mi->Pos == folder->Total-1 : TRUE);
      DoMethod(gui->TO_TOOLBAR, MUIM_Toolbar_Set, 9, MUIV_Toolbar_Set_Ghosted, !real);
      DoMethod(gui->TO_TOOLBAR, MUIM_Toolbar_Set,10, MUIV_Toolbar_Set_Ghosted, !real);
      DoMethod(gui->TO_TOOLBAR, MUIM_Toolbar_Set,11, MUIV_Toolbar_Set_Ghosted, out);
   }
   if (real)
   {
      if (AllFolderLoaded() && gui->TO_TOOLBAR)
      {
         DoMethod(gui->TO_TOOLBAR, MUIM_Toolbar_Set, 2, MUIV_Toolbar_Set_Ghosted, !RE_GetThread(mail, FALSE, FALSE, -1));
         DoMethod(gui->TO_TOOLBAR, MUIM_Toolbar_Set, 3, MUIV_Toolbar_Set_Ghosted, !RE_GetThread(mail, TRUE, FALSE, -1));
      }
   }
   else if (gui->TO_TOOLBAR)
   {
      DoMethod(gui->TO_TOOLBAR, MUIM_Toolbar_Set, 2, MUIV_Toolbar_Set_Ghosted, TRUE);
      DoMethod(gui->TO_TOOLBAR, MUIM_Toolbar_Set, 3, MUIV_Toolbar_Set_Ghosted, TRUE);
   }

   GetMailFile(G->RE[winnum]->File, folder, mail);

   if(RE_LoadMessage(winnum, PM_ALL))
   {
      RE_DisplayMessage(winnum, FALSE);
      set(gui->MI_EXTKEY, MUIA_Menuitem_Enabled, re->PGPKey);
      set(gui->MI_CHKSIG, MUIA_Menuitem_Enabled, hasPGPSOldFlag(re) || hasPGPSMimeFlag(re));
      set(gui->MI_SAVEDEC, MUIA_Menuitem_Enabled, real && (hasPGPEMimeFlag(re) || hasPGPEOldFlag(re)));
      RE_UpdateStatusGroup(winnum);
      MA_StartMacro(MACRO_READ, itoa(winnum));
      if (real && (mail->Status == STATUS_NEW || mail->Status == STATUS_UNR))
      {
         MA_SetMailStatus(mail, STATUS_OLD);
         DisplayStatistics(folder, TRUE);
         if((hasPGPSOldFlag(re) || hasPGPSMimeFlag(re)) && !hasPGPSCheckedFlag(re))
         {
            DoMethod(G->App, MUIM_CallHook, &RE_CheckSignatureHook, FALSE, winnum);
         }
         RE_DoMDN(MDN_READ, mail, FALSE);
      }
   }
   else
   {
      // check first if the mail file exists and if not we have to exit with an error
      if(!FileExists(mail->MailFile))
      {
        ER_NewError(GetStr(MSG_ER_CantOpenFile), GetMailFile(NULL, folder, mail), NULL);
      }

      RE_CleanupMessage(winnum);
      DisposeModulePush(&G->RE[winnum]);
   }
}
///
/// RE_SaveDisplay
//  Saves current message as displayed
void RE_SaveDisplay(int winnum, FILE *fh)
{
   char *ptr;

   if (G->RE[winnum]->Header != HM_NOHEADER)
   {
      int i;
      struct MUI_NList_GetEntryInfo res;

      fputs("\033[3m", fh);
      for (i=0;;i++)
      {
         res.pos = MUIV_NList_GetEntryInfo_Line;
         res.line = i;
         DoMethod(G->RE[winnum]->GUI.LV_HEAD, MUIM_NList_GetEntryInfo, &res);
         if (!res.entry) break;

         ptr = (char *)res.entry;
         if (!strcmp(ptr, MUIX_I)) ptr += strlen(MUIX_I);
         fputs(ptr, fh);
         fputc('\n', fh);
      }
      fputs("\033[23m\n", fh);
   }

   for (ptr = (char *)DoMethod(G->RE[winnum]->GUI.TE_TEXT, MUIM_TextEditor_ExportText); *ptr; ptr++)
   {
      if (*ptr == '\033')
      {
         switch (*++ptr)
         {
            case 'u': fputs("\033[4m", fh); break;
            case 'b': fputs("\033[1m", fh); break;
            case 'i': fputs("\033[3m", fh); break;
            case 'n': fputs("\033[0m", fh); break;
            case 'h': break;
            case '[': if (!strncmp(ptr, "[s:18]", 6))     { fputs("===========================================================", fh); }
                      else if (!strncmp(ptr, "[s:2]", 5)) { fputs("-----------------------------------------------------------", fh); }
            case 'p': while (*ptr != ']' && *ptr && *ptr != '\n') ptr++; break;
         }
      }
      else fputc(*ptr, fh);
   }
}

///
/// RE_SuggestName
//  Suggests a file name based on the message subject
char *RE_SuggestName(struct Mail *mail)
{
   static char name[SIZE_FILE];
   char *ptr = mail->Subject;
   int i = 0;
   unsigned char tc;

   memset(name, 0, SIZE_FILE);
   while (*ptr && i < 26)
   {
      tc = *ptr++;

      if ((tc <= 32) || (tc > 0x80 && tc < 0xA0) || (tc == ':') || (tc == '/'))
      {
         name[i++] = '_';
      }
      else name[i++] = tc;
   }

   strcat(name, ".msg");
   return name;
}

///
/// RE_Export
//  Saves message or attachments to disk
BOOL RE_Export(int winnum, char *source, char *dest, char *name, int nr, BOOL force, BOOL overwrite, char *ctype)
{
   char buffer[SIZE_PATHFILE], buffer2[SIZE_FILE+SIZE_DEFAULT];
   APTR win = winnum==4 ? G->MA->GUI.WI : G->RE[winnum]->GUI.WI;
   struct Mail *mail = &G->RE[winnum]->Mail;

   if (!*dest)
   {
      if (*name) strcpy(buffer2, name);
      else if (nr) sprintf(buffer2, "%s-%d", G->RE[winnum]->Mail.MailFile, nr);
      else strcpy(buffer2, RE_SuggestName(&(G->RE[winnum]->Mail)));
      if (force) strmfp(dest = buffer, C->DetachDir, buffer2);
      else if (ReqFile(ASL_DETACH, win, GetStr(MSG_RE_SaveMessage), REQF_SAVEMODE, C->DetachDir, buffer2))
         strmfp(dest = buffer, G->ASLReq[ASL_DETACH]->fr_Drawer, G->ASLReq[ASL_DETACH]->fr_File);
      else return FALSE;
   }
   if (FileExists(dest) && !overwrite)
   {
      if (!MUI_Request(G->App, win, 0, GetStr(MSG_MA_ConfirmReq), GetStr(MSG_OkayCancelReq), GetStr(MSG_RE_Overwrite), FilePart(dest)))
        return FALSE;
   }
   if (!CopyFile(dest, 0, source, 0))
   {
      ER_NewError(GetStr(MSG_ER_CantCreateFile), dest, NULL);
      return FALSE;
   }
   SetComment(dest, BuildAddrName2(&mail->From));
   if (!stricmp(ctype, ContType[CT_AP_AEXE])) SetProtection(dest, 0);
   if (!stricmp(ctype, ContType[CT_AP_SCRIPT])) SetProtection(dest, FIBF_SCRIPT);
   AppendLogVerbose(80, GetStr(MSG_LOG_SavingAtt), dest, mail->MailFile, FolderName(mail->Folder), "");
   return TRUE;
}
///
/// RE_MoveFunc
//  Moves the current message to another folder
HOOKPROTONHNO(RE_MoveFunc, void, int *arg)
{
   int winnum = *arg;
   struct Folder *srcfolder = G->RE[winnum]->Mail.Folder;
   struct Mail *mail = G->RE[winnum]->MailPtr;
   if (MailExists(mail, srcfolder))
   {
      int pos;
      struct Folder *dstfolder = FolderRequest(GetStr(MSG_MA_MoveMsg), GetStr(MSG_MA_MoveMsgReq), GetStr(MSG_MA_MoveGad), GetStr(MSG_Cancel), srcfolder, G->RE[winnum]->GUI.WI);
      if (dstfolder) if ((pos = SelectMessage(mail)) >= 0)
      {
         MA_MoveCopy(mail, srcfolder, dstfolder, FALSE);
         RE_UpdateDisplay(pos, winnum);
         AppendLogNormal(22, GetStr(MSG_LOG_Moving), (void *)1, srcfolder->Name, dstfolder->Name, "");
      }
   }
}
MakeStaticHook(RE_MoveHook, RE_MoveFunc);
///
/// RE_CopyFunc
//  Copies the current message to another folder
HOOKPROTONHNO(RE_CopyFunc, void, int *arg)
{
   int winnum = *arg;
   struct Folder *srcfolder = G->RE[winnum]->Mail.Folder;
   struct Mail *mail = G->RE[winnum]->MailPtr;
   if (MailExists(mail, srcfolder))
   {
      struct Folder *dstfolder = FolderRequest(GetStr(MSG_MA_CopyMsg), GetStr(MSG_MA_MoveMsgReq), GetStr(MSG_MA_CopyGad), GetStr(MSG_Cancel), NULL, G->RE[winnum]->GUI.WI);
      if (dstfolder)
      {
         if (srcfolder)
         {
            MA_MoveCopy(mail, srcfolder, dstfolder, TRUE);
            AppendLogNormal(24, GetStr(MSG_LOG_Copying), (void *)1, srcfolder->Name, dstfolder->Name, "");
         }
         else if (RE_Export(winnum, G->RE[winnum]->File, MA_NewMailFile(dstfolder, mail->MailFile, 0), "", 0, FALSE, FALSE, ContType[CT_ME_EMAIL]))
         {
            APTR lv;
            struct Mail *newmail = AddMailToList(mail, dstfolder);
            if ((lv = WhichLV(dstfolder))) DoMethod(lv, MUIM_NList_InsertSingle, newmail, MUIV_NList_Insert_Sorted);
            MA_SetMailStatus(newmail, STATUS_OLD);
         }
      }
   }
}
MakeStaticHook(RE_CopyHook, RE_CopyFunc);
///
/// RE_DeleteFunc
//  Deletes the current message
HOOKPROTONHNO(RE_DeleteFunc, void, int *arg)
{
   BOOL delatonce = hasFlag(arg[0], (IEQUALIFIER_LSHIFT|IEQUALIFIER_RSHIFT));
   int pos, winnum = arg[1];
   struct Folder *folder = G->RE[winnum]->Mail.Folder, *delfolder = FO_GetFolderByType(FT_DELETED, NULL);
   struct Mail *mail = G->RE[winnum]->MailPtr;
   if (MailExists(mail, folder)) if ((pos = SelectMessage(mail)) >= 0)
   {
      MA_DeleteSingle(G->RE[winnum]->MailPtr, delatonce, FALSE);
      RE_UpdateDisplay(pos, winnum);
      if (delatonce) AppendLogNormal(20, GetStr(MSG_LOG_Deleting), (void *)1, folder->Name, "", "");
      else           AppendLogNormal(22, GetStr(MSG_LOG_Moving), (void *)1, folder->Name, delfolder->Name, "");
   }
}
MakeStaticHook(RE_DeleteHook, RE_DeleteFunc);
///
/// RE_PrintFunc
//  Sends the current message or an attachment to the printer
HOOKPROTONHNO(RE_PrintFunc, void, int *arg)
{
   int winnum = *arg;
   struct Part *part;
   struct TempFile *prttmp;

   if ((part = AttachRequest(GetStr(MSG_RE_PrintMsg), GetStr(MSG_RE_SelectPrintPart), GetStr(MSG_RE_PrintGad), GetStr(MSG_Cancel), winnum, ATTREQ_PRINT|ATTREQ_MULTI, G->RE[winnum]->GUI.WI)))
   {
      if (C->PrinterCheck && !CheckPrinter()) return;
      BusyText(GetStr(MSG_BusyDecPrinting), "");
      for (; part; part = part->NextSelected)
      {
        switch (part->Nr)
        {
          case PART_ORIGINAL:
          {
            RE_PrintFile(G->RE[winnum]->File,part);
          }
          break;

          case PART_ALLTEXT:
          {
            if ((prttmp = OpenTempFile("w")))
            {
              RE_SaveDisplay(winnum, prttmp->FP);
              fclose(prttmp->FP);
              prttmp->FP = NULL;
              RE_PrintFile(prttmp->Filename,part);
              CloseTempFile(prttmp);
            }
          }
          break;

          default:
          {
            RE_PrintFile(part->Filename,part);
          }
        }
      }
      BusyEnd;
   }
}
MakeStaticHook(RE_PrintHook, RE_PrintFunc);

///
/// RE_PrintFile
//  Prints a file. Currently it is just dumped to PRT:
//  To do for LaTeX printing:
//  - remap characters in header to LaTeX notation
//  - make header lines to print (and parts where to print headers) configurable
static void RE_PrintFile(char *filename, struct Part *part)
{
   switch(C->PrintMethod)
   {
      case PRINTMETHOD_DUMPRAW :
         CopyFile("PRT:", 0, filename, 0);
         break;
      case PRINTMETHOD_LATEX :
         RE_PrintLaTeX(filename,part);
         break;
      case PRINTMETHOD_POSTSCRIPT :
         // fall through
      default:
         MUI_Request(G->App, NULL, 0, "YAM Error", "OK",
                     "Printing method #%ld is not implemented!\n"
                     "Use 0 for raw printer dump, 1 for LaTeX",C->PrintMethod);
         break;
   }
}

static void RE_PrintLaTeX(char *filename, struct Part *part)
{
  struct TempFile *texfile;

  if((texfile = OpenTempFile("w")))
  {
    if(CopyFile(NULL,texfile->FP,"YAM:.texheader",NULL))
    {
      char *ts1,*ts2 = 0;

      if((ts1 = AllocStrBuf(SIZE_LINE)) && (ts2 = AllocStrBuf(SIZE_LINE)))
      {
        if(part->Nr == PART_LETTER)
        {
          int i,j;
          char Attrib[SIZE_DEFAULT];
          char *p,*printcmd;
          const char PrintScript[] = "YAM:Scripts/LaTeX-print";

          for(i=0; i<Header.Used; i++)
          {
            p = Header.Data[i];
            if(NULL != strchr(p,':'))
            {
              for(j=0; p[j] != ':' && j < sizeof(Attrib); j++) Attrib[j] = p[j];
              Attrib[j++] = ':';
              Attrib[j++] = '\0';
              ts1 = StrBufCat(ts1,"\\NewLabWidth{");
              ts1 = StrBufCat(ts1,Attrib);
              ts1 = StrBufCat(ts1,"}\n");
              ts2 = StrBufCat(ts2,Attrib);
              ts2 = StrBufCat(ts2," &");
              ts2 = StrBufCat(ts2,p+j-1);
              ts2 = StrBufCat(ts2,"\\\\\n");
            } DB( else kprintf("RE_PrintFile(): strange header line %s\n",p); )
          }

          fprintf(texfile->FP,"\n%s\n%s\n%s\n%s\n\\input{%s}\n\\end{document}\n",
                  ts1,
                  "\\begin{document}\n\n"
                  "\\setlength{\\tabcolsep}{3.0pt}\n"
                  "\\setlength{\\TabRestWidth}{\\linewidth}\n"
                  "\\addtolength{\\TabRestWidth}{-\\tabcolsep}\n"
                  "\\addtolength{\\TabRestWidth}{-\\LabelWidth}\n\n"
                  "\\begin{tabular}"
                  "{@{}>{\\PBS\\raggedleft\\hspace{0pt}\\bf}p{\\LabelWidth}"
                  ">{\\PBS\\raggedright\\hspace{0pt}}p{\\TabRestWidth}}\n\n",
                  ts2,
                  "\\end{tabular}\n"
                  "\\hrule\n"
                  "\\bigskip\n",
                  filename);

          fclose(texfile->FP);
          texfile->FP = NULL;

          if((printcmd = malloc(sizeof(PrintScript)+strlen(texfile->Filename)+1)))
          {
            strcpy(printcmd,PrintScript);
            strcat(printcmd," ");
            strcat(printcmd,texfile->Filename);
            system(printcmd);
            free(printcmd);
          } else DisplayBeep(NULL);
        } DB( else kprintf("RE_PrintFile(): no headers for this part\n"); )
      }
      FreeStrBuf(ts1);
      FreeStrBuf(ts2);
    } DB( else kprintf("RE_PrintFile(): can't copy YAM:.texheader to temp TeX file\n"); )

    CloseTempFile(texfile);

  } DB( else kprintf("RE_PrintFile(): can't open temp TeX file\n"); )
}


#ifdef UNUSED
///
/// ISO8859_to_LaTeX
// Takes a string in ISO-8859 charset and converts it to a equivalent
// string in LaTeX notation. Free the result with FreeStrBuf() after use
static char *ISO8859_to_LaTeX(char *s)
{
  char *result=NULL;
  char **CVTab;

  if(CVTab = Init_ISO8859_to_LaTeX_Tab("YAM:.latex-chartab"))
  {
    int ResLen;
    char *p;

    for(p=s,ResLen=0; *p; p++)  // pre-calculate resulting string's length
    {
      ResLen += (CVTab[*p] == NULL ? 1 : strlen(CVTab[*p]));
    }

    DB( kprintf("ISO8859_to_LaTeX(): source=%ld result=%ld\n",strlen(s),ResLen); )

    if(result = AllocStrBuf(ResLen+1))
    {
      char *q = result;

      for(p=s,ResLen=0; *p; p++)   // map input string
      {

        if(CVTab[*p] == NULL) *q++ = *p;
        else
        {
          strcpy(q,CVTab[*p]);
          while(*q++) ;
        }
      }
    }
  }
  return result;
}
///
/// Init_ISO8859_to_LaTeX_Tab
// Takes a filename for a ISO->LaTeX mapping table and returns a table for
// mapping ISO/ASCII codes to strings
static char **Init_ISO8859_to_LaTeX_Tab(char *TabFileName)
{
  int TabSize;
  char **CVTab, *TabFile;
  BOOL success=FALSE;

  if(-1 != (TabSize = FileSize(TabFileName)))
  {
    BPTR fh;

    if(fh = Open(TabFileName,MODE_OLDFILE))
    {
      if(CVTab = AllocVec(TabSize+1+256*sizeof(char*), MEMF_ANY|MEMF_CLEAR))
      {
        TabFile = (char*)(CVTab+256*sizeof(char*));

        if(Read(fh,TabFile,TabSize) == TabSize)
        {
          char *tok, c=0;

          TabFile[TabSize] = '\0';
          tok = strtok(TabFile," \t\n");

          while(NULL != tok)
          {
            if(!c)
            {
              if(tok[1]) { DB(kprintf("Init_ISO8859_to_LaTeX_tab(): line format is %%c %%s\n");) }
              else c = tok[0];
            }
            else
            {
              CVTab[c] = tok;
              DB(kprintf("LaTeX mapping: '%c' -> '%s'\n",c,tok);)
              c = '\0';
            }
          }
          success = TRUE;
        }

        if(!success)
        {
          FreeVec(CVTab);
          CVTab = NULL;
        }
      }
      Close(fh);
    }
  }
  return CVTab; // and who one will free this Vector ????
}
#endif
///
/// RE_SaveFunc
//  Saves the current message or an attachment to disk
HOOKPROTONHNO(RE_SaveFunc, void, int *arg)
{
  int winnum = *arg;
  struct Part *part;
  struct TempFile *tf;

  if ((part = AttachRequest(GetStr(MSG_RE_SaveMessage), GetStr(MSG_RE_SelectSavePart), GetStr(MSG_RE_SaveGad), GetStr(MSG_Cancel), winnum, ATTREQ_SAVE|ATTREQ_MULTI, G->RE[winnum]->GUI.WI)))
  {
    BusyText(GetStr(MSG_BusyDecSaving), "");
    for (; part; part = part->NextSelected)
    {
      switch (part->Nr)
      {
        case PART_ORIGINAL:
        {
          RE_Export(winnum, G->RE[winnum]->File, "", "", 0, FALSE, FALSE, ContType[CT_ME_EMAIL]);
        }
        break;

        case PART_ALLTEXT:
        {
          if ((tf = OpenTempFile("w")))
          {
            RE_SaveDisplay(winnum, tf->FP);
            fclose(tf->FP);
            tf->FP = NULL;
            RE_Export(winnum, tf->Filename, "", "", 0, FALSE, FALSE, ContType[CT_TX_PLAIN]);
            CloseTempFile(tf);
          }
        }
        break;

        default:
        {
          RE_DecodePart(part);
          RE_Export(winnum, part->Filename, "", part->CParFileName ? part->CParFileName : part->Name, part->Nr, FALSE, FALSE, part->ContentType);
        }
      }
    }
    BusyEnd;
  }
}
MakeStaticHook(RE_SaveHook, RE_SaveFunc);
///
/// RE_DisplayMIME
//  Displays a message part (attachment) using a MIME viewer
void RE_DisplayMIME(char *fname, char *ctype)
{
  int i;
  struct MimeView *mv = NULL;
  static char command[SIZE_COMMAND+SIZE_PATHFILE];
  char *fileptr;

  for (i = 1; i < MAXMV; i++)
  {
    if (C->MV[i] && MatchNoCase(ctype, C->MV[i]->ContentType))
    {
      mv = C->MV[i];
      break;
    }
  }

  if (!mv && !stricmp(ctype, "message/rfc822"))
  {
    int winnum;
    struct Mail *mail;
    struct ExtendedMail *email;
    struct TempFile *tf = OpenTempFile(NULL);
    CopyFile(tf->Filename, NULL, fname, NULL);

    if ((email = MA_ExamineMail(NULL, FilePart(tf->Filename), NULL, TRUE)))
    {
      mail = calloc(1, sizeof(struct Mail));
      if(!mail) return;

      memcpy(mail, &(email->Mail), sizeof(struct Mail));
      mail->Next      = NULL;
      mail->Reference = NULL;
      mail->Folder    = NULL;
      mail->UIDL      = NULL;
      mail->Status    = STATUS_OLD;
      SET_FLAG(mail->Flags, MFLAG_NOFOLDER);

      MA_FreeEMailStruct(email);

      if ((winnum = RE_Open(-1, FALSE)) != -1)
      {
        G->RE[winnum]->TempFile = tf;
        if(SafeOpenWindow(G->RE[winnum]->GUI.WI))
        {
          RE_ReadMessage(winnum, mail);
        }
        else
        {
          DisposeModulePush(&G->RE[winnum]);
          free(mail);
        }
      }
      else free(mail);
    }
  }
  else
  {
    if (!mv)
    {
      if (C->IdentifyBin)
      {
        ctype = IdentifyFile(fname);
        for (i = 1; i < MAXMV; i++)
        {
          if(C->MV[i] && MatchNoCase(ctype, C->MV[i]->ContentType))
          {
            mv = C->MV[i];
            break;
          }
        }
      }
      if (!mv) mv = C->MV[0];
    }

    // now we have to generate a real commandstring and make sure that
    // the %s is covered with "" or otherwise we will run into trouble with
    // the execute and pathes that have whitespaces.
    if((fileptr = strstr(mv->Command, "%s")))
    {
      char *startPtr = fileptr;
      char *endPtr = fileptr;

      // lets see if the %s is surrounded by some "" and find the beginning where
      // we should place the quotation marks

      // we first move back until we find whitespace or a quotation mark
      do
      {
        if(*startPtr == ' ' || startPtr == &mv->Command[0])
        {
          startPtr++;
          break;
        }
        else if(*startPtr == '"')
        {
          startPtr = NULL;
          break;
        }

      }while(startPtr--);

      if(startPtr)
      {
        // then we search for the end where we should put the closing quotation mark
        do
        {
          if(*endPtr == ' ' || *endPtr == '\0') break;
          else if(*endPtr == '"')
          {
            endPtr = NULL;
            break;
          }

        }while(endPtr++);
      }

      // if we found the start and endPtr we can place the quotation marks there
      // by copying the whole string in a buffer.
      if(startPtr && endPtr)
      {
        char realcmd[SIZE_COMMAND];
        int i,j;

        for(i=0,j=0; ;i++,j++)
        {
          // if this is the start or end we place the first quotation mark
          if(&mv->Command[j] == startPtr || &mv->Command[j] == endPtr)
          {
            realcmd[i++] = '"';
          }

          realcmd[i] = mv->Command[j];

          if(mv->Command[j] == '\0') break;
        }

        sprintf(command, realcmd, GetRealPath(fname));
      }
      else sprintf(command, mv->Command, GetRealPath(fname));

      ExecuteCommand(command, TRUE, OUT_NIL);
    }
    else ExecuteCommand(mv->Command, TRUE, OUT_NIL);
  }
}
///
/// RE_DisplayFunc
//  Shows message or attachments separately
HOOKPROTONHNO(RE_DisplayFunc, void, int *arg)
{
   int winnum = *arg;
   struct Part *part;

   if ((part = AttachRequest(GetStr(MSG_RE_DisplayMsg), GetStr(MSG_RE_SelectDisplayPart), GetStr(MSG_RE_DisplayGad), GetStr(MSG_Cancel), winnum, ATTREQ_DISP|ATTREQ_MULTI, G->RE[winnum]->GUI.WI)))
   {
      BusyText(GetStr(MSG_BusyDecDisplaying), "");

      for (; part; part = part->NextSelected)
      {
         RE_DecodePart(part);
         switch(part->Nr)
         {
            case PART_ORIGINAL:
            {
              RE_DisplayMIME(G->RE[winnum]->File, "text/plain");
            }
            break;

            default:
            {
              RE_DisplayMIME(part->Filename, part->ContentType);
            }
         }
      }
      BusyEnd;
   }
}
MakeStaticHook(RE_DisplayHook, RE_DisplayFunc);
///
/// RE_SaveAll
//  Saves all attachments to disk
void RE_SaveAll(int winnum, char *path)
{
   struct Part *part;
   char fname[SIZE_DEFAULT], *dest;

   if(!(dest = calloc(1, strlen(path)+SIZE_DEFAULT+1))) return;

   for (part = G->RE[winnum]->FirstPart->Next->Next; part; part = part->Next)
   {
      if (*part->Name) stccpy(fname, part->Name, SIZE_DEFAULT);
      else sprintf(fname, "%s-%d", G->RE[winnum]->Mail.MailFile, part->Nr);
      strmfp(dest, path, fname);

      RE_DecodePart(part);
      RE_Export(winnum, part->Filename, dest, part->Name, part->Nr, FALSE, FALSE, part->ContentType);
   }

   free(dest);
}
///
/// RE_SaveAllFunc
//  Asks user for a directory and saves all attachments there
HOOKPROTONHNO(RE_SaveAllFunc, void, int *arg)
{
   struct Part *part = G->RE[*arg]->FirstPart->Next;
   if (part) if (part->Next) if (ReqFile(ASL_DETACH, G->RE[*arg]->GUI.WI, GetStr(MSG_RE_SaveMessage), (REQF_SAVEMODE|REQF_DRAWERSONLY), C->DetachDir, ""))
   {
      BusyText(GetStr(MSG_BusyDecSaving), "");
      RE_SaveAll(*arg, G->ASLReq[ASL_DETACH]->fr_Drawer);
      BusyEnd;
   }
}
MakeStaticHook(RE_SaveAllHook, RE_SaveAllFunc);
///
/// RE_RemoveAttachFunc
//  Removes attachments from the current message
HOOKPROTONHNO(RE_RemoveAttachFunc, void, int *arg)
{
   struct Mail *mail = G->RE[*arg]->MailPtr;
   struct MailInfo *mi;
   MA_RemoveAttach(mail, TRUE);
   if ((mi = GetMailInfo(mail))->Pos >= 0)
   {
      DoMethod(G->MA->GUI.NL_MAILS, MUIM_NList_Redraw, mi->Pos);
      CallHookPkt(&MA_ChangeSelectedHook, 0, 0);
      DisplayStatistics(mail->Folder, TRUE);
   }
   RE_ReadMessage(*arg, mail);
}
MakeStaticHook(RE_RemoveAttachHook, RE_RemoveAttachFunc);
///
/// RE_NewFunc
//  Starts a new message based on the current one
HOOKPROTONHNO(RE_NewFunc, void, int *arg)
{
   int mode = arg[0], winnum = arg[2], flags = 0;
   ULONG qual = arg[1];
   struct Mail *mail = G->RE[winnum]->MailPtr, *mlist[3] = { (struct Mail *)1, NULL, NULL };
   if (mode == NEW_FORWARD &&   hasFlag(qual, (IEQUALIFIER_LSHIFT|IEQUALIFIER_RSHIFT))) mode = NEW_BOUNCE;
   if (mode == NEW_FORWARD && isFlagSet(qual, IEQUALIFIER_CONTROL))                     SET_FLAG(flags, NEWF_FWD_NOATTACH);
   if (mode == NEW_REPLY   &&   hasFlag(qual, (IEQUALIFIER_LSHIFT|IEQUALIFIER_RSHIFT))) SET_FLAG(flags, NEWF_REP_PRIVATE);
   if (mode == NEW_REPLY   &&   hasFlag(qual, (IEQUALIFIER_LALT|IEQUALIFIER_RALT)))     SET_FLAG(flags, NEWF_REP_MLIST);
   if (mode == NEW_REPLY   && isFlagSet(qual, IEQUALIFIER_CONTROL))                     SET_FLAG(flags, NEWF_REP_NOQUOTE);
   mlist[2] = mail;

   if (MailExists(mail, NULL)) switch (mode)
   {
      case NEW_NEW:     MA_NewNew(mail, flags); break;
      case NEW_EDIT:    MA_NewEdit(mail, flags, winnum); break;
      case NEW_BOUNCE:  MA_NewBounce(mail, flags); break;
      case NEW_FORWARD: MA_NewForward(mlist, flags); break;
      case NEW_REPLY:   MA_NewReply(mlist, flags); break;
   }
}
MakeStaticHook(RE_NewHook, RE_NewFunc);
///
/// RE_GetAddressFunc
//  Stores sender address of current message in the address book
HOOKPROTONHNO(RE_GetAddressFunc, void, int *arg)
{
   int winnum = *arg;
   struct Folder *folder = G->RE[winnum]->Mail.Folder;
   struct Mail *mail = G->RE[winnum]->MailPtr, *mlist[3] = { (struct Mail *)1, NULL, NULL };
   mlist[2] = mail;
   if (MailExists(mail, folder)) MA_GetAddress(mlist);
}
MakeStaticHook(RE_GetAddressHook, RE_GetAddressFunc);
///
/// RE_SetUnreadFunc
//  Sets the status of the current mail to unread
HOOKPROTONHNO(RE_SetUnreadFunc, void, int *arg)
{
   int winnum = *arg;
   MA_SetMailStatus(G->RE[winnum]->MailPtr, STATUS_UNR);
   RE_UpdateStatusGroup(winnum);
   DisplayStatistics(NULL, TRUE);
}
MakeStaticHook(RE_SetUnreadHook, RE_SetUnreadFunc);
///
/// RE_SetMarkedFunc
//  Sets the flags of the current mail to marked
HOOKPROTONHNO(RE_SetMarkedFunc, void, int *arg)
{
   int winnum = *arg;
   MA_SetMailFlag(G->RE[winnum]->MailPtr, MFLAG_MARK, FALSE);
   RE_UpdateStatusGroup(winnum);
   DisplayStatistics(NULL, TRUE);
}
MakeStaticHook(RE_SetMarkedHook, RE_SetMarkedFunc);
///
/// RE_ChangeSubjectFunc
//  Changes the subject of the current message
HOOKPROTONHNO(RE_ChangeSubjectFunc, void, int *arg)
{
   char subj[SIZE_SUBJECT];
   int winnum = *arg;
   struct Folder *folder = G->RE[winnum]->Mail.Folder;
   struct Mail *mail = G->RE[winnum]->MailPtr;
   struct MailInfo *mi;
   if (MailExists(mail, folder))
   {
      strcpy(subj, mail->Subject);
      if (StringRequest(subj, SIZE_SUBJECT, GetStr(MSG_MA_ChangeSubj), GetStr(MSG_MA_ChangeSubjReq), GetStr(MSG_Okay), NULL, GetStr(MSG_Cancel), FALSE, G->RE[*arg]->GUI.WI))
      {
         MA_ChangeSubject(mail, subj);
         if ((mi = GetMailInfo(mail))->Pos >= 0)
         {
            DoMethod(G->MA->GUI.NL_MAILS, MUIM_NList_Redraw, mi->Pos);
            CallHookPkt(&MA_ChangeSelectedHook, 0, 0);
            DisplayStatistics(mail->Folder, TRUE);
         }
         RE_ReadMessage(*arg, mail);
      }
   }
}
MakeStaticHook(RE_ChangeSubjectHook, RE_ChangeSubjectFunc);
///
/// RE_ExtractKeyFunc
//  Extracts public PGP key from the current message
HOOKPROTONHNO(RE_ExtractKeyFunc, void, int *arg)
{
   char fullfile[SIZE_PATHFILE], options[SIZE_PATHFILE];
   struct Mail *mail = G->RE[*arg]->MailPtr;

   if (!StartUnpack(GetMailFile(NULL, NULL, mail), fullfile, mail->Folder)) return;
   sprintf(options, (G->PGPVersion == 5) ? "-a %s +batchmode=1 +force" : "-ka %s +bat +f", fullfile);
   PGPCommand((G->PGPVersion == 5) ? "pgpk" : "pgp", options, 0);
   FinishUnpack(fullfile);
}
MakeStaticHook(RE_ExtractKeyHook, RE_ExtractKeyFunc);
///
/// RE_GetAddressFromLog
//  Finds e-mail address in PGP output
static BOOL RE_GetAddressFromLog(char *buf, char *address)
{
   if ((buf = strchr(buf, 34)))
   {
      stccpy(address, ++buf, SIZE_ADDRESS);
      if ((buf = strchr(address, 34))) *buf = 0;
      return TRUE;
   }
   return FALSE;
}
///
/// RE_GetSigFromLog
//  Interprets logfile created from the PGP signature check
static void RE_GetSigFromLog(int winnum, char *decrFor)
{
   BOOL sigDone = FALSE, decrFail = FALSE;
   struct RE_ClassData *re = G->RE[winnum];
   FILE *fh;
   char buffer[SIZE_LARGE];

   if ((fh = fopen(PGPLOGFILE, "r")))
   {
      while (GetLine(fh, buffer, SIZE_LARGE))
      {
         if(!decrFail && decrFor && G->PGPVersion == 5)
         {
            if(!strnicmp(buffer, "cannot decrypt", 14))
            {
               *decrFor = 0;
               GetLine(fh, buffer, SIZE_LARGE); GetLine(fh, buffer, SIZE_LARGE);
               RE_GetAddressFromLog(buffer, decrFor);
               decrFail = TRUE;
            }
         }

         if(!sigDone)
         {
            if(!strnicmp(buffer, "good signature", 14)) sigDone = TRUE;
            else if(!strnicmp(buffer, "bad signature", 13) || stristr(buffer, "unknown keyid"))
            {
              SET_FLAG(re->PGPSigned, PGPS_BADSIG);
              sigDone = TRUE;
            }

            if(sigDone)
            {
               if (G->PGPVersion == 5) { GetLine(fh, buffer, SIZE_LARGE); GetLine(fh, buffer, SIZE_LARGE); }
               if (RE_GetAddressFromLog(buffer, re->Signature)) SET_FLAG(re->PGPSigned, PGPS_ADDRESS);

               SET_FLAG(re->PGPSigned, PGPS_CHECKED);
               break;
            }
         }
      }
      fclose(fh);

      if(sigDone || (decrFor && !decrFail))
        DeleteFile(PGPLOGFILE);
   }
}
///
/// RE_SaveDecryptedFunc
//  Saves decrypted version of a PGP message
HOOKPROTONHNO(RE_SaveDecryptedFunc, void, int *arg)
{
   struct RE_ClassData *re = G->RE[*arg];
   struct WritePart *p1;
   struct Compose comp;
   int choice;
   struct Folder *folder = re->MailPtr->Folder;
   char mfile[SIZE_MFILE];

   if(!folder) return;

   if (!(choice = MUI_Request(G->App, re->GUI.WI, 0, GetStr(MSG_RE_SaveDecrypted), GetStr(MSG_RE_SaveDecGads), GetStr(MSG_RE_SaveDecReq)))) return;
   memset(&comp, 0, sizeof(struct Compose));

   if ((comp.FH = fopen(MA_NewMailFile(folder, mfile, 0), "w")))
   {
      struct ExtendedMail *email;

      comp.Mode = NEW_SAVEDEC;
      comp.OrigMail = re->MailPtr;
      comp.FirstPart = p1 = NewPart(2);
      p1->Filename = re->FirstPart->Next->Filename;
      WriteOutMessage(&comp);
      FreePartsList(p1);
      fclose(comp.FH);

      if ((email = MA_ExamineMail(folder, mfile, NULL, TRUE)))
      {
         struct Mail *newmail;

         // lets set some values depending on the original message
         email->Mail.Status = re->MailPtr->Status;
         memcpy(&email->Mail.transDate, &re->MailPtr->transDate, sizeof(struct timeval));

         // add the mail to the folder now
         newmail = AddMailToList(&email->Mail, folder);

         // if this was a compressed/encrypted folder we need to pack the mail now
         if(folder->XPKType != XPK_OFF) RepackMailFile(newmail, -1, NULL);

         if (FO_GetCurrentFolder() == folder) DoMethod(G->MA->GUI.NL_MAILS, MUIM_NList_InsertSingle, newmail, MUIV_NList_Insert_Sorted);
         MA_FreeEMailStruct(email);
         if (choice == 2)
         {
            MA_DeleteSingle(re->MailPtr, FALSE, FALSE);
            RE_ReadMessage(*arg, newmail);
         }
      }
      else ER_NewError(GetStr(MSG_ER_CreateMailError), NULL, NULL);
   }
}
MakeStaticHook(RE_SaveDecryptedHook, RE_SaveDecryptedFunc);
///

/*** MIME ***/
/// StripTrailingSpace
//  Strips trailing spaces from a string
static void StripTrailingSpace(char *s)
{
   char *t = &s[strlen(s)-1];
   while (ISpace(*t) && t >= s) *t-- = 0;
}
///
/// ParamEnd
//  Finds next parameter in header field
static char *ParamEnd(char *s)
{
   BOOL inquotes = FALSE;

   while (*s)
   {
      if (inquotes)
      {
         if (*s == '"') inquotes = FALSE; else if (*s == '\\') ++s;
      }
      else if (*s == ';') return(s);
      else if (*s == '"') inquotes = TRUE;
      ++s;
   }
   return NULL;
}
///
/// Cleanse
//  Removes trailing and leading spaces and converts string to lower case
static char *Cleanse(char *s)
{
   char *tmp, *news;

   news = s = stpblk(s);
   for (tmp=s; *tmp; ++tmp) if (isupper((int)*tmp)) *tmp = tolower((int)*tmp);
   while (tmp > news && *--tmp && ISpace(*tmp)) *tmp = 0;
   return news;
}
///
/// UnquoteString
//  Removes quotes from a string, skipping "escaped" quotes
static char *UnquoteString(char *s, BOOL new)
{
   char *ans, *t, *o = s;

   if (*s != '"') return s;
   ans = malloc(1+strlen(s));
   ++s;
   t = ans;
   while (*s)
   {
      if (*s == '\\') *t++ = *++s;
      else if (*s == '"') break;
      else *t++ = *s;
      ++s;
   }
   *t = 0;
   if (new) return ans;
   strcpy(o, ans);
   free(ans);
   return o;
}
///
/// RE_ParseContentParameters
//  Parses parameters of Content-Type header field
static void RE_ParseContentParameters(struct Part *rp)
{
   char *s, *t, *eq, *ct = rp->ContentType;

   s = strchr(ct, ';');
   if (!s) return;
   *s++ = 0;
   do {
      if ((t = ParamEnd(s))) *t++ = 0;
      if (!(eq = strchr(s, '='))) rp->JunkParameter = Cleanse(s);
      else
      {
         *eq++ = 0;
         s = Cleanse(s); eq = stpblk(eq);
         StripTrailingSpace(eq);
         UnquoteString(eq, FALSE);
         if (!stricmp(s, "name"))
         {
            SParse(eq);
            rp->CParName = eq;
         }
         else if (!stricmp(s, "description")) rp->CParDesc  = eq;
         else if (!stricmp(s, "boundary"))    rp->CParBndr  = eq;
         else if (!stricmp(s, "protocol"))    rp->CParProt  = eq;
         else if (!stricmp(s, "report-type")) rp->CParRType = eq;
         else if (!stricmp(s, "charset"))     rp->CParCSet  = eq;
      }
      s = t;
   } while (t);
}
///
/// RE_ParseContentDispositionParameters
//  Parses parameters of Content-Disposition header field
static void RE_ParseContentDispositionParameters(struct Part *rp)
{
   char *s, *t, *eq, *cd = rp->ContentDisposition;

   s = strchr(cd, ';');
   if (!s) return;
   *s++ = 0;
   do {
      if ((t = ParamEnd(s))) *t++ = 0;
      if (!(eq = strchr(s, '='))) rp->JunkParameter = Cleanse(s);
      else
      {
         *eq++ = 0;
         s = Cleanse(s); eq = stpblk(eq);
         StripTrailingSpace(eq);
         UnquoteString(eq, FALSE);
         if (!stricmp(s, "filename"))
         {
            SParse(eq);
            rp->CParFileName = eq;
         }
      }
      s = t;
   } while (t);
}
///
/// RE_ScanHeader
//  Parses the header of the message or of a message part
static BOOL RE_ScanHeader(struct Part *rp, FILE *in, FILE *out, int mode)
{
   int i;
   char *p;

   if (!MA_ReadHeader(in))
   {
      if (mode == 0) ER_NewError(GetStr(MSG_ER_MIMEError), NULL, NULL);
      else if (mode == 1) ER_NewError(GetStr(MSG_ER_MultipartEOF), NULL, NULL);

      rp->HasHeaders = FALSE;
      return FALSE;
   }

   rp->HasHeaders = TRUE;
   for (i = 0; i < Header.Used; i++)
   {
      char *s = Header.Data[i];
      int ls = strlen(s);
      if(ls > rp->MaxHeaderLen)
        rp->MaxHeaderLen = ls;
      if (out) { fputs(s, out); fputc('\n', out); }
      if (!strnicmp(s, "content-type:", 13))
      {
         rp->ContentType = StrBufCpy(rp->ContentType, p = stpblk(&s[13]));
         while (TRUE)
         {
            if (!(p = strchr(rp->ContentType, '/'))) break;
            if (ISpace(*(p-1)))     for (--p; *p; ++p) *p = *(p+1);
            else if(ISpace(*(p+1))) for (++p; *p; ++p) *p = *(p+1);
            else break;
         }
         StripTrailingSpace(rp->ContentType);
         RE_ParseContentParameters(rp);
      }
      else if (!strnicmp(s, "content-transfer-encoding:", 26))
      {
         char buf[SIZE_DEFAULT];
         stccpy(p = buf, stpblk(&s[26]), SIZE_DEFAULT);
         StripTrailingSpace(p);

         // As the content-transfer-encoding field is mostly used in
         // attachment MIME fields, we first check for common attachement encodings
         if     (!stricmp(p, "base64"))                        rp->EncodingCode = ENC_B64;
         else if(!stricmp(p, "quoted-printable"))              rp->EncodingCode = ENC_QP;
         else if(!strnicmp(p, "x-uue", 5))                     rp->EncodingCode = ENC_UUE;
         else if(!stricmp(p, "8bit") || !stricmp(p, "8-bit"))  rp->EncodingCode = ENC_8BIT;
         else if(!stricmp(p, "binary"))                        rp->EncodingCode = ENC_BIN;
         else if(!stricmp(p, "7bit") || !stricmp(p, "7-bit")
                 || !stricmp(p, "none"))                       rp->EncodingCode = ENC_NONE;
         else
         {
            ER_NewError(GetStr(MSG_ER_UnknownEnc), p, NULL);
         }
      }
      else if (!strnicmp(s, "content-description:", 20))
      {
         stccpy(rp->Description, stpblk(&s[20]), SIZE_DEFAULT);
      }
      else if (!strnicmp(s, "content-disposition:", 20))
      {
        // if we found a content-disposition field we have to parse it for
        // some information.
        rp->ContentDisposition = StrBufCpy(rp->ContentDisposition, p = stpblk(&s[20]));
        while (TRUE)
        {
          if (!(p = strchr(rp->ContentDisposition, '/'))) break;
          if (ISpace(*(p-1)))       for (--p; *p; ++p) *p = *(p+1);
          else if(ISpace(*(p+1)))   for (++p; *p; ++p) *p = *(p+1);
          else break;
        }
        StripTrailingSpace(rp->ContentDisposition);
        RE_ParseContentDispositionParameters(rp);
      }
   }
   for (p = rp->ContentType; *p; ++p) if (isupper((int)*p)) *p = tolower((int)*p);
   return TRUE;
}
///
/// RE_ConsumeRestOfPart
//  Processes body of a message part
static BOOL RE_ConsumeRestOfPart(FILE *in, FILE *out, struct TranslationTable *tt, struct Part *rp)
{
   char c = 0, buf[SIZE_LINE];
   int blen = 0;
   long cpos = 0;
   BOOL cempty = TRUE;

   if(!in) return FALSE;
   if(rp) blen = strlen(rp->Boundary);
   if(out) cpos = ftell(in);

   while(fgets(buf, SIZE_LINE, in))
   {
      // first we check if we reached the boundary yet.
      if(rp && !strncmp(buf, rp->Boundary, blen))
      {
         if (buf[blen] == '-' && buf[blen+1] == '-' && buf[blen+2] == '\n') return TRUE;
         else return FALSE;
      }

      if (out)
      {
         long size = ftell(in)-cpos; // get new position and size of read chars

         // if this function was invokes with a translation table, we have to change
         // all chars accordingly
         if(tt)
         {
            long t = size;
            unsigned char *p = buf;

            // iterate through the buffer and change the chars accordingly.
            for(;size; size--, p++) *p = tt->Table[*p];

            size = t;
         }

         // if there is some endchar in the c variable we write it first
         // out to the fh.
         if(!cempty) fputc(c, out);

         // lets save the last char of the buffer in a temp variable because
         // we need to skip the last byte of the stream later on
         c = buf[size-1];
         cempty = FALSE;

         // now write back exactly the same amount of bytes we read previously
         if(fwrite(buf, 1, (size_t)size-1, out) != size-1) return FALSE;

         // increase cpos for next iteration
         cpos += size;
      }
   }

   // if we end up here because of a EOF we have check
   // if there is still something in c and then write it into the out fh.
   if(feof(in))
   {
      if(out && !cempty) fputc(c, out);
      return TRUE;
   }

   return FALSE;
}
///
/// RE_DecodeStream
//  Decodes contents of a part
static BOOL RE_DecodeStream(struct Part *rp, FILE *in, FILE *out)
{
   struct TranslationTable *tt = NULL;
   BOOL decodeResult = FALSE;

   if(rp->Nr == PART_LETTER && rp->Printable &&
      G->TTin && G->TTin->SourceCharset
     )
   {
      char *srcCharset = G->TTin->SourceCharset;

      if(!rp->CParCSet)
      {
         if(MatchNoCase(C->LocalCharset, srcCharset) ||
            MatchNoCase("us-ascii", srcCharset))
         {
            tt = G->TTin;
         }
      }
      else if(MatchNoCase(rp->CParCSet, srcCharset))
      {
         tt = G->TTin;
      }
   }

   // lets check if we got some encoding here and
   // if so we have to decode it immediatly
   switch(rp->EncodingCode)
   {
      // process a base64 decoding.
      case ENC_B64:
      {
        long decoded = base64decode_file(in, out, tt, rp->Printable);
        DB(kprintf("base64 decoded %ld bytes of part %ld.\n", decoded, rp->Nr);)

        if(decoded > 0)
          decodeResult = TRUE;
      }
      break;

      // process a Quoted-Printable decoding
      case ENC_QP:
      {
        fromqp(in, out, tt);

        decodeResult = TRUE;
      }
      break;

      // process UU-Encoded decoding
      case ENC_UUE:
      {
        fromuue (in, out);
        if(RE_ConsumeRestOfPart(in, NULL, NULL, NULL))
          decodeResult = TRUE;
      }
      break;

      // process URL encoded decoding
      case ENC_FORM:
      {
        fromform(in, out, tt);

        decodeResult = TRUE;
      }
      break;

      default:
      {
        decodeResult = RE_ConsumeRestOfPart(in, out, tt, NULL);
      }
   }

   return decodeResult;
}
///
/// RE_OpenNewPart
//  Adds a new entry to the message part list
static FILE *RE_OpenNewPart(int winnum, struct Part **new, struct Part *prev, struct Part *first)
{
   FILE *fp;

   if (((*new) = calloc(1,sizeof(struct Part))))
   {
      char file[SIZE_FILE];
      if (prev)
      {
         (*new)->Prev = prev;
         prev->Next = *new;
         (*new)->Nr = prev->Nr+1;
      }
      (*new)->ContentType = StrBufCpy(NULL, "text/plain");
      (*new)->EncodingCode = ENC_NONE;
      if (first) if (strnicmp(first->ContentType, "multipart", 9))
      {
         (*new)->ContentType = StrBufCpy((*new)->ContentType, first->ContentType);
         (*new)->CParCSet = first->CParCSet;
         (*new)->EncodingCode = first->EncodingCode;
      }
      strcpy((*new)->Boundary, first ? first->Boundary : (prev ? prev->Boundary : ""));
      (*new)->Win = winnum;
      sprintf(file, "YAMr%08lx-w%dp%d.txt", (ULONG)G->RE[winnum]->MailPtr, winnum, (*new)->Nr);
      strmfp((*new)->Filename, C->TempDir, file);
      if ((fp = fopen((*new)->Filename, "w"))) return fp;
      free(*new);
   }
   return NULL;
}
///
/// RE_UndoPart
//  Removes an entry from the message part list
static void RE_UndoPart(struct Part *rp)
{
   struct Part *trp = rp;

   // lets delete the file first so that we can cleanly "undo" the part
   DeleteFile(rp->Filename);

   // we only iterate through our partlist if there is
   // a next item, if not we can simply relink it
   if(trp->Next)
   {
      // if we remove a part from the part list we have to take
      // care of the part index number aswell. So all following
      // parts have to be descreased somehow by one.
      //
      // p2->p3->p4->p5
      do
      {
        // use the next element as the current trp
        trp = trp->Next;

        // decrease the part number aswell
        trp->Nr--;

        // Now we also have to rename the temporary filename also
        Rename(trp->Filename, trp->Prev->Filename);

      } while(trp->Next);

      // now go from the end to the start again and copy
      // the filenames strings as we couldn`t do that in the previous
      // loop also
      //
      // p5->p4->p3->p2
      do
      {
        // iterate backwards
        trp = trp->Prev;

        // now copy the filename string
        strcpy(trp->Next->Filename, trp->Filename);

      } while(trp->Prev && trp != rp);
   }

   // relink the partlist
   if (rp->Prev) rp->Prev->Next = rp->Next;
   if (rp->Next) rp->Next->Prev = rp->Prev;

   // free some string buffers
   FreeStrBuf(rp->ContentType);
   FreeStrBuf(rp->ContentDisposition);

   // and last, but not least we free the part
   free(rp);
}
///
/// RE_RequiresSpecialHandling
//  Checks if part is PGP signed/encrypted or a MDN
static int RE_RequiresSpecialHandling(struct Part *hrp)
{
   if (!stricmp(hrp->ContentType, "multipart/report") && !stricmp(hrp->CParRType, "disposition-notification")) return 1;
   if (!stricmp(hrp->ContentType, "multipart/signed") && !stricmp(hrp->CParProt, "application/pgp-signature")) return 2;
   if (!stricmp(hrp->ContentType, "multipart/encrypted") && !stricmp(hrp->CParProt, "application/pgp-encrypted")) return 3;
   return 0;
}
///
/// RE_IsURLencoded
//  Checks if part contains encoded form data
static BOOL RE_IsURLencoded(struct Part *rp)
{
   return (BOOL)(!stricmp(rp->ContentType, "application/x-www-form-urlencoded") ||
                 !stricmp(rp->ContentType, "application/x-url-encoded"));
}
///
/// RE_SaveThisPart
//  Decides if the part should be kept in memory
static BOOL RE_SaveThisPart(struct Part *rp)
{
   int pm = G->RE[rp->Win]->ParseMode;
   switch (pm)
   {
      case PM_ALL:   return TRUE;
      case PM_NONE:  return FALSE;
      case PM_TEXTS: return (BOOL)(!strnicmp(rp->ContentType, "text", 4) || RE_IsURLencoded(rp));
   }

   return FALSE;
}
///
/// RE_SetPartInfo
//  Determines size and other information of a message part
static void RE_SetPartInfo(struct Part *rp)
{
   int size = rp->Size = FileSize(rp->Filename);

   // let`s calculate the partsize on a undecoded part, if not a common part
   if(!rp->Decoded && rp->Nr > 0)
   {
      switch (rp->EncodingCode)
      {
        case ENC_UUE: case ENC_B64: rp->Size = (100*size)/136; break;
        case ENC_QP:                rp->Size = (100*size)/106; break;
        default:
          // nothing
        break;
      }
   }

   // if this part hasn`t got any name, we place the CParName as the normal name
   if (!*rp->Name && (rp->CParName || rp->CParFileName))
   {
      stccpy(rp->Name, rp->CParName ? rp->CParName : rp->CParFileName, SIZE_DEFAULT);
      UnquoteString(rp->Name, FALSE);
   }

   // let`s set if this is a printable (readable part)
   rp->Printable = !strnicmp(rp->ContentType, "text", 4) || rp->Nr == PART_RAW;

   // lets set the comments of the partfiles
   if(rp->Nr == PART_RAW)                          SetComment(rp->Filename, GetStr(MSG_RE_Header));
   else if(rp->Nr == PART_LETTER && rp->Printable) SetComment(rp->Filename, GetStr(MSG_RE_Letter));
   else
   {
      // if this is not a printable LETTER part or a RAW part we
      // write another comment
      SetComment(rp->Filename, *rp->Description ? rp->Description : (*rp->Name ? rp->Name : rp->ContentType));
   }
}
///
/// RE_ParseMessage
//  Parses a complete message
static struct Part *RE_ParseMessage(int winnum, FILE *in, char *fname, struct Part *hrp)
{
  if(in == NULL && fname) in = fopen(fname, "r");

  if(in)
  {
    FILE *out;
    struct Part *rp;
    char *boundary;

    if(hrp == NULL)
    {
      if ((out = RE_OpenNewPart(winnum, &hrp, NULL, NULL)))
      {
        BOOL parse_ok = RE_ScanHeader(hrp, in, out, 0);

        fclose(out);
        if (parse_ok) RE_SetPartInfo(hrp);
      }
      else ER_NewError(GetStr(MSG_ER_CantCreateTempfile), NULL, NULL);
    }

    if(hrp)
    {
      if (!(boundary = hrp->CParBndr)) boundary = hrp->JunkParameter;

      if (!strnicmp(hrp->ContentType, "multipart", 9))
      {
        if(!boundary) ER_NewError(GetStr(MSG_ER_MissingBoundary), NULL, NULL);
        else
        {
          BOOL done;
          if (*boundary == '"') boundary = UnquoteString(boundary, TRUE);
          sprintf(hrp->Boundary, "--%s", boundary);
          done = RE_ConsumeRestOfPart(in, NULL, NULL, hrp);
          rp = hrp;

          while (!done)
          {
            struct Part *prev = rp;
            out = RE_OpenNewPart(winnum, &rp, prev, hrp);

            if(out == NULL) break;

            if(!RE_ScanHeader(rp, in, out, 1))
            {
              fclose(out);
              RE_UndoPart(rp);
              break;
            }

            if(!strnicmp(rp->ContentType, "multipart", 9))
            {
              fclose(out);

              if(RE_ParseMessage(winnum, in, NULL, rp))
              {
                RE_UndoPart(rp);
                done = RE_ConsumeRestOfPart(in, NULL, NULL, prev);
                for (rp = prev; rp->Next; rp = rp->Next);
              }
            }
            else if (RE_SaveThisPart(rp) || RE_RequiresSpecialHandling(hrp) == 3)
            {
              fputc('\n', out);
              done = RE_ConsumeRestOfPart(in, out, NULL, rp);
              fclose(out);
              RE_SetPartInfo(rp);
            }
            else
            {
              fclose(out);
              done = RE_ConsumeRestOfPart(in, NULL, NULL, rp);
              RE_UndoPart(rp);
              rp = prev;
            }
          }
        }
      }
      else if ((out = RE_OpenNewPart(winnum, &rp, hrp, hrp)))
      {
        if (RE_SaveThisPart(rp) || RE_RequiresSpecialHandling(hrp) == 3)
        {
          RE_ConsumeRestOfPart(in, out, NULL, NULL); fclose(out);
          RE_SetPartInfo(rp);
        }
        else
        {
          fclose(out);
          RE_UndoPart(rp);
          RE_ConsumeRestOfPart(in, NULL, NULL, NULL);
        }
      }
    }

    if (fname) fclose(in);
  }

#ifdef DEBUG
if(fname)
{
  struct Part *rp;

  kprintf("\nHeaderPart: [%lx]\n", hrp);

  for(rp = hrp; rp; rp = rp->Next)
  {
    kprintf("Part[%lx] - %ld\n", rp, rp->Nr);
    kprintf("  Name.......: [%s]\n", rp->Name);
    kprintf("  ContentType: [%s]\n", rp->ContentType);
    kprintf("  Encoding...: %ld\n",  rp->EncodingCode);
    kprintf("  Filename...: [%s]\n", rp->Filename);
    kprintf("  Size.......: %ld\n", rp->Size);
    kprintf("  Nextptr....: %lx\n", rp->Next);
    kprintf("  Prevptr....: %lx\n", rp->Prev);
  }
}
#endif

  return hrp;
}
///
/// RE_DecodePart
//  Decodes a single message part
BOOL RE_DecodePart(struct Part *rp)
{
   if (!rp->Decoded)
   {
      FILE *in, *out;
      char file[SIZE_FILE], buf[SIZE_LINE], ext[FNSIZE];

      if ((in = fopen(rp->Filename, "r")))
      {
         if (rp->HasHeaders) while (GetLine(in, buf, SIZE_LINE)) if (!*buf) break;
         stcgfe(ext, rp->Name);
         if (strlen(ext) > 10) *ext = 0;
         sprintf(file, "YAMm%08lx-w%dp%d.%s", (ULONG)G->RE[rp->Win]->MailPtr, rp->Win, rp->Nr, *ext ? ext : "tmp");
         strmfp(buf, C->TempDir, file);
         if ((out = fopen(buf, "w")))
         {
            BOOL decodeResult = RE_DecodeStream(rp, in, out);

            // close the streams first.
            fclose(out);
            fclose(in);

            // check if we were successfull in decoding the data.
            if(decodeResult)
            {
              DeleteFile(rp->Filename);
              strcpy(rp->Filename, buf);
              rp->Decoded = TRUE;
              RE_SetPartInfo(rp);
            }
         }
         else if((out = fopen(buf, "r")))
         {
            // if we couldn`t open that file for writing we check if it exists
            // and if so we use it because it is locked actually and already decoded
            fclose(out);
            fclose(in);
            DeleteFile(rp->Filename);
            strcpy(rp->Filename, buf);
            rp->Decoded = TRUE;
            RE_SetPartInfo(rp);
         }
         else fclose(in);
      }
   }
   return rp->Decoded;
}
///
/// RE_CleanupMessage
//  Cleanup memory and temporary files used to display the message
void RE_CleanupMessage(int winnum)
{
   struct RE_ClassData *re = G->RE[winnum];
   struct Part *part, *next;

   for (part = re->FirstPart; part; part = next)
   {
      next = part->Next;
      if (*part->Filename) DeleteFile(part->Filename);
      FreeStrBuf(part->ContentType);
      FreeStrBuf(part->ContentDisposition);
      free(part);
   }
   re->FirstPart     = NULL;
   re->FirstReadDone = FALSE;

   // now we have to check whether there is a .unp (unpack) file and delete
   // it acoordingly (we can`t use the FinishUnpack() function because the
   // window still refers to the file which will be prevent the deletion.
   if(strstr(re->File, ".unp")) DeleteFile(re->File);
}
///
/// RE_HandleMDNReport
//  Translates a message disposition notification to readable text
static void RE_HandleMDNReport(struct Part *frp)
{
   struct Part *rp[3];
   char file[SIZE_FILE], buf[SIZE_PATHFILE], MDNtype[SIZE_DEFAULT];
   char *msgdesc, *mode = "", *type;
   int i, j;
   FILE *out, *fh;

   if ((rp[0] = frp->Next)) if ((rp[1] = rp[0]->Next))
   {
      rp[2] = rp[1]->Next;
      msgdesc = AllocStrBuf(80);
      strcpy(MDNtype, "");
      for (j = 1; j < (rp[2] ? 3 : 2); j++)
      {
         RE_DecodePart(rp[j]);
         if ((fh = fopen(rp[j]->Filename, "r")))
         {
            MA_ReadHeader(fh);
            fclose(fh);
            for (i = 0; i < Header.Used; i++)
            {
               char *value, *field = Header.Data[i];
               if ((value = strchr(field, ':')))
               {
                  *value++ = 0;
                  if (!stricmp(field, "from")) msgdesc = StrBufCat(StrBufCat(msgdesc, GetStr(MSG_RE_MDNFrom)), value);
                  else if (!stricmp(field, "to")) msgdesc = StrBufCat(StrBufCat(msgdesc, GetStr(MSG_RE_MDNTo)), value);
                  else if (!stricmp(field, "subject")) msgdesc = StrBufCat(StrBufCat(msgdesc, GetStr(MSG_RE_MDNSubject)), value);
                  else if (!stricmp(field, "original-message-id")) msgdesc = StrBufCat(StrBufCat(msgdesc, GetStr(MSG_RE_MDNMessageID)), value);
                  else if (!stricmp(field, "date")) msgdesc = StrBufCat(StrBufCat(msgdesc, GetStr(MSG_RE_MDNDate)), value);
                  else if (!stricmp(field, "original-recipient")) msgdesc = StrBufCat(StrBufCat(msgdesc, GetStr(MSG_RE_MDNOrigRecpt)), value);
                  else if (!stricmp(field, "final-recipient")) msgdesc = StrBufCat(StrBufCat(msgdesc, GetStr(MSG_RE_MDNFinalRecpt)), value);
                  else if (!stricmp(field, "disposition")) stccpy(MDNtype, Trim(value), SIZE_DEFAULT);
               }
            }
            FreeData2D(&Header);
         }
      }
      msgdesc = StrBufCat(msgdesc, "\n");
      if (!strnicmp(MDNtype, "manual-action", 13)) mode = GetStr(MSG_RE_MDNmanual);
      if (!strnicmp(MDNtype, "automatic-action", 16)) mode = GetStr(MSG_RE_MDNauto);
      if ((type = strchr(MDNtype, ';'))) type = Trim(++type); else type = MDNtype;
      sprintf(file, "YAMm%08lx-w%dp%d.txt", (ULONG)G->RE[rp[0]->Win]->MailPtr, rp[0]->Win, rp[0]->Nr);
      strmfp(buf, C->TempDir, file);
      if ((out = fopen(buf, "w")))
      {
         if      (!stricmp(type, "displayed"))  fprintf(out, GetStr(MSG_RE_MDNdisplay), msgdesc);
         else if (!stricmp(type, "processed"))  fprintf(out, GetStr(MSG_RE_MDNprocessed), msgdesc, mode);
         else if (!stricmp(type, "dispatched")) fprintf(out, GetStr(MSG_RE_MDNdispatched), msgdesc, mode);
         else if (!stricmp(type, "deleted"))    fprintf(out, GetStr(MSG_RE_MDNdeleted), msgdesc, mode);
         else if (!stricmp(type, "denied"))     fprintf(out, GetStr(MSG_RE_MDNdenied), msgdesc);
         else fprintf(out, GetStr(MSG_RE_MDNunknown), msgdesc, type, mode);
         fclose(out);
         DeleteFile(rp[0]->Filename);
         strcpy(rp[0]->Filename, buf);
         rp[0]->Decoded = TRUE;
         RE_SetPartInfo(rp[0]);
         if (rp[2]) RE_UndoPart(rp[2]);
         RE_UndoPart(rp[1]);
      }
      FreeStrBuf(msgdesc);
   }
}
///
/// RE_HandleSignedMessage
//  Handles a PGP signed message, checks validity of signature
static void RE_HandleSignedMessage(struct Part *frp)
{
   struct Part *rp[2];

   if ((rp[0] = frp->Next))
   {
      if (G->PGPVersion && (rp[1] = rp[0]->Next))
      {
         int error;
         struct TempFile *tf = OpenTempFile(NULL);
         char options[SIZE_LARGE];

         // flag the mail as having a PGP signature within the MIME encoding
         SET_FLAG(G->RE[frp->Win]->PGPSigned, PGPS_MIME);

         ConvertCRLF(rp[0]->Filename, tf->Filename, TRUE);
         sprintf(options, (G->PGPVersion == 5) ? "%s -o %s +batchmode=1 +force +language=us" : "%s %s +bat +f +lang=en", rp[1]->Filename, tf->Filename);
         error = PGPCommand((G->PGPVersion == 5) ? "pgpv": "pgp", options, NOERRORS|KEEPLOG);
         if (error > 0) SET_FLAG(G->RE[frp->Win]->PGPSigned, PGPS_BADSIG);
         if (error >= 0) RE_GetSigFromLog(frp->Win, NULL);
         tf->FP = NULL;
         CloseTempFile(tf);
      }
      RE_DecodePart(rp[0]);
   }
}
///
/// RE_DecryptPGP
//  Decrypts a PGP encrypted file
static int RE_DecryptPGP(int winnum, char *src)
{
   FILE *fh;
   int error;
   char options[SIZE_LARGE], orcpt[SIZE_ADDRESS];

   *orcpt = 0;
   DB( kprintf("RE_DecryptPGP()\n"); )
   PGPGetPassPhrase();
   if (G->PGPVersion == 5)
   {
      char fname[SIZE_PATHFILE];
      sprintf(fname, "%s.asc", src); Rename(src, fname);
      sprintf(options, "%s +batchmode=1 +force +language=us", fname);
      error = PGPCommand("pgpv", options, KEEPLOG|NOERRORS);
      RE_GetSigFromLog(winnum, orcpt);
      if (*orcpt) error = 2;
      DeleteFile(fname);
   }
   else
   {
      sprintf(options, "%s +bat +f +lang=en", src);
      error = PGPCommand("pgp", options, KEEPLOG|NOERRORS);
      RE_GetSigFromLog(winnum, NULL);
   }
   PGPClearPassPhrase(error < 0 || error > 1);
   if (error < 0 || error > 1) if ((fh = fopen(src, "w")))
   {
      fputs(GetStr(MSG_RE_PGPNotAllowed), fh);
      if (G->PGPVersion == 5 && *orcpt) fprintf(fh, GetStr(MSG_RE_MsgReadOnly), orcpt);
      fclose(fh);
   }
   DB( kprintf("RE_DecryptPGP() ends: %ld\n", error); )
   return error;
}
///
/// RE_HandleEncryptedMessage
//  Handles a PGP encryped message
static void RE_HandleEncryptedMessage(struct Part *frp)
{
   struct Part *warnPart;
   struct Part *encrPart;

   // if we find a warning and a encryption part we start decrypting
   if((warnPart = frp->Next) && (encrPart = warnPart->Next))
   {
      int decryptResult;
      struct TempFile *tf = OpenTempFile("w");

      // first we copy our encrypted part because the DecryptPGP()
      // function will overwrite it
      if(!tf || !CopyFile(NULL, tf->FP, encrPart->Filename, NULL)) return;
      fclose(tf->FP);
      tf->FP = NULL;

      decryptResult = RE_DecryptPGP(frp->Win, tf->Filename);

      if(decryptResult == 1 || decryptResult == 0)
      {
         FILE *in;

         if(decryptResult == 0) SET_FLAG(G->RE[frp->Win]->PGPSigned, PGPS_MIME);
         SET_FLAG(G->RE[frp->Win]->PGPEncrypted, PGPE_MIME);

         // if DecryptPGP() returns with 0 everything worked perfectly and we can
         // convert & copy our decrypted file over the encrypted part
         if(ConvertCRLF(tf->Filename, warnPart->Filename, FALSE) && (in = fopen(warnPart->Filename, "r")))
         {
            warnPart->ContentType = StrBufCpy(warnPart->ContentType, "text/plain");
            warnPart->Printable = TRUE;
            warnPart->EncodingCode = ENC_NONE;
            *warnPart->Description = 0;
            RE_ScanHeader(warnPart, in, NULL, 2);
            fclose(in);
            warnPart->Decoded = FALSE;
            RE_DecodePart(warnPart);
            RE_UndoPart(encrPart); // undo the encrypted part because we have a decrypted now.
         }
      }
      else
      {
        // if we end up here the DecryptPGP returned an error an
        // we have to put this error in place were the nonlocalized version is right now.
        if(CopyFile(warnPart->Filename, NULL, tf->Filename, NULL))
        {
           warnPart->ContentType = StrBufCpy(warnPart->ContentType, "text/plain");
           warnPart->Printable = TRUE;
           warnPart->EncodingCode = ENC_NONE;
           *warnPart->Description = 0;
           warnPart->Decoded = TRUE;
           warnPart->HasHeaders = FALSE;
        }
      }

      CloseTempFile(tf);
   }
}
///
/// RE_LoadMessagePart
//  Decodes a single message part
static void RE_LoadMessagePart(int winnum, struct Part *part)
{
   int rsh = RE_RequiresSpecialHandling(part);

   switch (rsh)
   {
      case 1:
      {
        RE_HandleMDNReport(part);
      }
      break;

      case 2:
      {
        RE_HandleSignedMessage(part);
      }
      break;

      case 3:
      {
        RE_HandleEncryptedMessage(part);
      }
      break;

      default:
      {
        struct Part *rp;

        for(rp = part->Next; rp; rp = rp->Next)
        {
          if(RE_IsURLencoded(rp))
          {
            rp->ContentType = StrBufCpy(rp->ContentType, "text/plain");
            rp->EncodingCode = ENC_FORM;
            RE_DecodePart(rp);
          }
          else if(!stricmp(rp->ContentType, "application/pgp-keys"))
          {
            G->RE[winnum]->PGPKey = TRUE;
          }
          else if(rp->Nr < PART_LETTER || (rp->Printable && (rp->Nr == PART_LETTER || C->DisplayAllTexts)))
          {
            RE_DecodePart(rp);
          }
        }
      }
   }
}
///
/// RE_LoadMessage
//  Prepares a message for displaying
static BOOL RE_LoadMessage(int winnum, int parsemode)
{
   char newfile[SIZE_PATHFILE], file[SIZE_FILE];
   struct Part *rp;
   int i;

   BusyText(GetStr(MSG_BusyReading), "");

   if (!StartUnpack(G->RE[winnum]->File, newfile, G->RE[winnum]->MailPtr->Folder))
   {
      BusyEnd;
      return FALSE;
   }

   strcpy(G->RE[winnum]->File, newfile);
   G->RE[winnum]->ParseMode = parsemode;
   if ((rp = G->RE[winnum]->FirstPart = RE_ParseMessage(winnum, NULL, G->RE[winnum]->File, NULL)))
   {
      RE_LoadMessagePart(winnum, rp);
      for (i = 0; rp; i++, rp = rp->Next)
      {
        if (rp->Nr != i)
        {
          rp->Nr = i;
          sprintf(file, "YAMm%08lx-w%dp%d%s", (ULONG)G->RE[winnum]->MailPtr, winnum, i, strchr(rp->Filename,'.'));
          strmfp(newfile, C->TempDir, file);

          RenameFile(rp->Filename, newfile);
          strcpy(rp->Filename, newfile);
        }
      }
   }
   BusyEnd;
   return TRUE;
}
///
/// RE_GetPart
//  Gets a message part by its index number
static struct Part *RE_GetPart(int winnum, int partnr)
{
   struct Part *part;
   for (part = G->RE[winnum]->FirstPart; part; part = part->Next) if (part->Nr == partnr) break;
   return part;
}
///
/// RE_InitPrivateRC
//  Allocates resources for background message parsing
void RE_InitPrivateRC(struct Mail *mail, enum ParseMode parsemode)
{
   struct RE_ClassData *data = calloc(1, sizeof(struct RE_ClassData));
   if ((G->RE[4] = data))
   {
     data->Mail = *mail;
     data->MailPtr = mail;
     GetMailFile(data->File, mail->Folder, mail);
     RE_LoadMessage(4, parsemode);
   }
}
///
/// RE_FreePrivateRC
//  Frees resources used by background message parsing
void RE_FreePrivateRC(void)
{
   RE_CleanupMessage(4);
   free(G->RE[4]);
}
///
/// RE_ReadInMessage
//  Reads a message into a dynamic buffer and returns this buffer.
//  The text returned should *NOT* contain any MUI specific escape sequences, as
//  we will later parse the buffer again before we put it into the texteditor. So no deeep lexical analysis
//  are necessary here.
char *RE_ReadInMessage(int winnum, enum ReadInMode mode)
{
  struct RE_ClassData *re = G->RE[winnum];
  struct Part *first, *last, *part, *uup = NULL;
  char *cmsg;
  int totsize, len;

  DB(kprintf("RE_ReadInMessage\n");)

  // save exit conditions
  if(!re || !(first = re->FirstPart)) return NULL;

  // first we precalucalte the size of the final buffer where the message text will be put in
  for(totsize = 1000, part = first; part; part = part->Next)
  {
    if(mode != RIM_READ && part->Nr && part->Nr != PART_LETTER) continue;
    if(part->Decoded || !part->Nr) totsize += part->Size;
    else totsize += 200;
  }

  // then we generate our final buffer for the message
  if((cmsg = calloc(len=(totsize*3)/2, sizeof(char))))
  {
    int wptr=0, prewptr;

    // if this function wasn`t called with QUIET we place a BusyText into the Main Window
    if (mode != RIM_QUIET) BusyText(GetStr(MSG_BusyDisplaying), "");

    // then we copy the first part (which is the header of the mail
    // into our final buffer because we don`t need to preparse it
    // but only if we are in READ mode
    if (mode == RIM_READ)
    {
      FILE *fh;

      if((fh = fopen(first->Filename, "r")))
      {
        int buflen = first->MaxHeaderLen+4;
        char *linebuf = malloc(buflen);
        while(fgets(linebuf, buflen, fh))
        {
          SParse(linebuf);
          cmsg = AppendToBuffer(cmsg, &wptr, &len, linebuf);
        }
        free(linebuf);
        fclose(fh);
        cmsg = AppendToBuffer(cmsg, &wptr, &len, "\n");
      }
    }

    // Now we check every part of the message if it will be displayed in the
    // texteditor or not and if so we run the part through the lexer
    for(part = first->Next; part; part = part->Next)
    {
      BOOL dodisp = (part->Nr < PART_LETTER || (part->Printable && (part->Nr == PART_LETTER || (C->DisplayAllTexts && part->Decoded))));

      prewptr = wptr;

      if (mode != RIM_READ && part->Nr > PART_LETTER) break;

      // if we are in READ mode and other parts than the LETTER part
      // shouldn`t be displayed in the texteditor, we drop a simple separator bar with info.
      // This is used for attachments and here escape sequences are allowed as we don`t want them
      // to get stripped if the user selects "NoTextStyles"
      if (mode == RIM_READ && (part->Nr > PART_LETTER || !dodisp))
      {
        char buffer[SIZE_LARGE];

        // lets generate the separator bar.
        sprintf(buffer, "\033c\033[s:18]\033p[7]%d: %s\033p[0]\n\033l\033b%s:\033n %s   \033b%s:\033n %s%ld %s\n", part->Nr, part->Name, GetStr(MSG_RE_ContentType), DescribeCT(part->ContentType), GetStr(MSG_Size), part->Decoded ? "" : "~", part->Size, GetStr(MSG_Bytes));
        cmsg = AppendToBuffer(cmsg, &wptr, &len, buffer);

        *buffer = 0;
        if(*part->Description) sprintf(buffer, "\033b%s:\033n %s\n", GetStr(MSG_RE_Description), part->Description);
        if(dodisp)
        {
          strcat(buffer, "\033[s:2]\n");
        }
        if(*buffer) cmsg = AppendToBuffer(cmsg, &wptr, &len, buffer);
      }

      // only continue of this part should be displayed
      // and is greater than zero, or else we don`t have
      // to parse anything at all.
      if(dodisp && part->Size > 0)
      {
        FILE *fh;

        if((fh = fopen(part->Filename, "r")))
        {
          char *msg;

          if((msg = calloc((size_t)(part->Size+3), sizeof(char))))
          {
            char *ptr, *rptr, *eolptr, *sigptr = 0;
            int nread;

            *msg = '\n';
            nread = fread(msg+1, 1, (size_t)(part->Size), fh);

            // lets check if an error or short item count occurred
            if(nread == 0 || nread != part->Size)
            {
              DB(kprintf("Warning: EOF or short item count detected: feof()=%ld ferror()=%ld\n", feof(fh), ferror(fh));)
              // distinguish between EOF and error
              if(feof(fh) == 0 && ferror(fh) != 0)
              {
                // an error occurred, lets signal it by returning NULL
                DB(kprintf("ERROR occurred while reading at pos %ld of %s\n", ftell(fh), part->Filename);)

                // cleanup and return NULL
                free(msg);
                fclose(fh);
                if(mode != RIM_QUIET) BusyEnd;
                return NULL;
              }

              // if we end up here it is "just" an EOF so lets put out
              // a warning and continue.
              DB(kprintf("Warning: EOF detected at pos %ld of %s\n", ftell(fh), part->Filename);)
            }

            // nothing serious happened so lets continue...
            rptr = msg+1;

            // find signature first if it should be stripped
            if(mode == RIM_QUOTE && C->StripSignature)
            {
              sigptr = msg + nread;
              while(sigptr > msg)
              {
                sigptr--;
                while((sigptr > msg) && (*sigptr != '\n')) sigptr--;  // step back to previous line

                if((sigptr <= msg+1))
                {
                  sigptr = NULL;
                  break;
                }

                if(strncmp(sigptr+1, "-- \n", 4) == 0)                // check for sig separator
                {                                                     // per definition it is a "-- " on a single line
                  sigptr++;
                  break;
                }
              }
            }

            // parse the message string
            while(*rptr)
            {
              // lets get the first real line of the data and make sure to strip all
              // NUL bytes because otherwise we are not able to show the text.
              for(eolptr = rptr; *eolptr != '\n' && eolptr < msg+nread+1; eolptr++)
              {
                // strip null bytes that are in between the start and end of stream
                // here we simply exchange it by a space
                if(*eolptr == '\0') *eolptr = ' ';
              }
              *eolptr = '\0';

/* UUenc */   if(!strncmp(rptr, "begin ", 6) && isdigit((int)rptr[6]))
              {
                if(!re->FirstReadDone)
                {
                  FILE *ufh;
                  ptr = &rptr[6];

                  while (!ISpace(*ptr)) ptr++;
                  ptr = stpblk(ptr);
                  for (last = first; last->Next; last = last->Next);

                  if((ufh = RE_OpenNewPart(winnum, &uup, last, first)))
                  {
                    uup->ContentType = StrBufCpy(uup->ContentType, "application/octet-stream");
                    strcpy(uup->Description, GetStr(MSG_RE_UUencodedFile));
                    stccpy(uup->Name, ptr, SIZE_FILE);
                    fromuuetxt(&rptr, ufh);
                    fclose(ufh);

                    uup->Decoded = TRUE;
                    RE_SetPartInfo(uup);
                    eolptr = rptr-1;
                    ptr = rptr;
                  }
                  else ER_NewError(GetStr(MSG_ER_CantCreateTempfile), NULL, NULL);
                }
                else
                {
                  for(ptr=eolptr+1; *ptr; ptr++)
                  {
                    if(!strncmp(ptr, "end", 3)) break;

                    while(*ptr && *ptr != '\n') ptr++;
                  }

                  while(*ptr && *ptr != '\n') ptr++;

                  eolptr = ptr++;
                }

                if(!strncmp(ptr, "size", 4))
                {
                  if(!re->FirstReadDone)
                  {
                    int expsize = atoi(&ptr[5]);
                    if (uup->Size != expsize) ER_NewError(GetStr(MSG_ER_UUSize), (char *)uup->Size, (char *)expsize);
                  }

                  for (eolptr = ptr; *eolptr && *eolptr!='\n'; eolptr++);
                  *eolptr = 0;
                }
              }
/* PGP msg */ else if(!strncmp(rptr, "-----BEGIN PGP MESSAGE", 21))
              {
                struct TempFile *tf;
                DB( kprintf("RE_ReadInMessage(): encrypted message\n"); )

                if((tf = OpenTempFile("w")))
                {
                  *eolptr = '\n';
                  for(ptr=eolptr+1; *ptr; ptr++)
                  {
                    if(!strncmp(ptr, "-----END PGP MESSAGE", 19)) break;

                    while (*ptr && *ptr != '\n') ptr++;
                  }

                  while (*ptr && *ptr != '\n') ptr++;

                  eolptr = ptr++;
                  fwrite(rptr, 1, (size_t)(ptr-rptr), tf->FP);
                  fclose(tf->FP);
                  tf->FP = NULL;

                  DB( kprintf("RE_ReadInMessage(): decrypting\n"); )

                  if(RE_DecryptPGP(winnum, tf->Filename) == 0)
                  {
                    // flag the mail as having a inline PGP signature
                    SET_FLAG(re->PGPSigned, PGPS_OLD);

                    // make sure that the mail is flaged as signed
                    if(!isSignedMail(re->MailPtr))
                    {
                      SET_FLAG(re->MailPtr->Flags, MFLAG_SIGNED);
                      SET_FLAG(re->MailPtr->Folder->Flags, FOFL_MODIFY);  // flag folder as modified
                    }
                  }

                  if ((tf->FP = fopen(tf->Filename, "r")))
                  {
                    char buf2[SIZE_LARGE];
                    DB( kprintf("RE_ReadInMessage(): decrypted message follows\n"); )

                    while(fgets(buf2, SIZE_LARGE, tf->FP))
                    {
                      rptr = buf2;
                      DB( kprintf(buf2); )
                      cmsg = AppendToBuffer(cmsg, &wptr, &len, buf2);
                    }
                  }
                  CloseTempFile(tf);
                }

                // flag the mail as being inline PGP encrypted
                SET_FLAG(re->PGPEncrypted, PGPE_OLD);

                // make sure that mail is flagged as crypted
                if(!isCryptedMail(re->MailPtr))
                {
                  SET_FLAG(re->MailPtr->Flags, MFLAG_CRYPT);
                  SET_FLAG(re->MailPtr->Folder->Flags, FOFL_MODIFY);  // flag folder as modified
                }

                DB( kprintf("RE_ReadInMessage(): done with decryption\n"); )
              }
/* Signat. */ else if(!strcmp(rptr, "-- "))
              {
                if (mode == RIM_READ)
                {
                  if(C->SigSepLine == 1) cmsg = AppendToBuffer(cmsg, &wptr, &len, rptr);
                  else if(C->SigSepLine == 2) cmsg = AppendToBuffer(cmsg, &wptr, &len, "\033[s:2]");
                  else if(C->SigSepLine == 3) break;

                  cmsg = AppendToBuffer(cmsg, &wptr, &len, "\n");
                }
                else if(mode == RIM_QUOTE)
                {
                  if(C->StripSignature && (rptr == sigptr)) break;
                }
                else
                {
                  cmsg = AppendToBuffer(cmsg, &wptr, &len, "-- \n");
                }
              }
/* PGP sig */ else if (!strncmp(rptr, "-----BEGIN PGP PUBLIC KEY BLOCK", 31))
              {
                re->PGPKey = TRUE;

                cmsg = AppendToBuffer(cmsg, &wptr, &len, rptr);
                cmsg = AppendToBuffer(cmsg, &wptr, &len, "\n");
              }
              else if (!strncmp(rptr, "-----BEGIN PGP SIGNED MESSAGE", 29))
              {
                // flag the mail as having a inline PGP signature
                SET_FLAG(re->PGPSigned, PGPS_OLD);

                if(!isSignedMail(re->MailPtr))
                {
                  SET_FLAG(re->MailPtr->Flags, MFLAG_SIGNED);
                  SET_FLAG(re->MailPtr->Folder->Flags, FOFL_MODIFY);  // flag folder as modified
                }
              }
/* other */   else
              {
                cmsg = AppendToBuffer(cmsg, &wptr, &len, rptr);
                cmsg = AppendToBuffer(cmsg, &wptr, &len, "\n");
              }

              rptr = eolptr+1;
            }

            free(msg);
          }

          fclose(fh);
        }
      }

      SParse(cmsg + prewptr);
    }

    re->FirstReadDone = TRUE;
    if(mode != RIM_QUIET) BusyEnd;
  }

  return cmsg;
}
///
/// RE_AddExtraHeader
//  Adds additional headers to the header listview
static void RE_AddExtraHeader(APTR lv, char *header, char *value)
{
   char buffer[SIZE_LARGE];
   if (!*value) return;
   sprintf(buffer, MUIX_I"%s: %s", StripUnderscore(header), value);
   DoMethod(lv, MUIM_NList_InsertSingle, buffer, MUIV_NList_Insert_Bottom);
}
///
/// RE_GetSenderInfo
//  Parses X-SenderInfo header field
static void RE_GetSenderInfo(struct Mail *mail, struct ABEntry *ab)
{
   char *s, *t, *eq;
   struct ExtendedMail *email;

   memset(ab, 0, sizeof(struct ABEntry));
   stccpy(ab->Address, mail->From.Address, SIZE_ADDRESS);
   stccpy(ab->RealName, mail->From.RealName, SIZE_REALNAME);

   if(isSenderInfoMail(mail))
   {
      if((email = MA_ExamineMail(mail->Folder, mail->MailFile, NULL, TRUE)))
      {
        if ((s = strchr(email->SenderInfo, ';')))
        {
          *s++ = 0;
          do
          {
            if ((t = ParamEnd(s))) *t++ = 0;
            if (!(eq = strchr(s, '='))) Cleanse(s);
            else
            {
               *eq++ = 0;
               s = Cleanse(s); eq = stpblk(eq);
               StripTrailingSpace(eq);
               UnquoteString(eq, FALSE);
               if (!stricmp(s, "street")) stccpy(ab->Street, eq, SIZE_DEFAULT);
               if (!stricmp(s, "city")) stccpy(ab->City, eq, SIZE_DEFAULT);
               if (!stricmp(s, "country")) stccpy(ab->Country, eq, SIZE_DEFAULT);
               if (!stricmp(s, "phone")) stccpy(ab->Phone, eq, SIZE_DEFAULT);
               if (!stricmp(s, "homepage")) stccpy(ab->Homepage, eq, SIZE_URL);
               if (!stricmp(s, "dob")) ab->BirthDay = atol(eq);
               if (!stricmp(s, "picture")) stccpy(ab->Photo, eq, SIZE_PATHFILE);
               ab->Type = 1;
            }
            s = t;
         }while (t);
       }
       MA_FreeEMailStruct(email);
     }
   }
}
///
/// RE_UpdateSenderInfo
//  Updates address book entry of sender
static void RE_UpdateSenderInfo(struct ABEntry *old, struct ABEntry *new)
{
   BOOL changed = FALSE;

   if (!*old->RealName && *new->RealName) { strcpy(old->RealName, new->RealName); changed = TRUE; }
   if (!*old->Address  && *new->Address ) { strcpy(old->Address,  new->Address ); changed = TRUE; }
   if (!*old->Street   && *new->Street  ) { strcpy(old->Street,   new->Street  ); changed = TRUE; }
   if (!*old->Country  && *new->Country ) { strcpy(old->Country,  new->Country ); changed = TRUE; }
   if (!*old->City     && *new->City    ) { strcpy(old->City,     new->City    ); changed = TRUE; }
   if (!*old->Phone    && *new->Phone   ) { strcpy(old->Phone,    new->Phone   ); changed = TRUE; }
   if (!*old->Homepage && *new->Homepage) { strcpy(old->Homepage, new->Homepage); changed = TRUE; }
   if (!old->BirthDay  && new->BirthDay ) { old->BirthDay = new->BirthDay; changed = TRUE; }
   if (changed)
      CallHookPkt(&AB_SaveABookHook, 0, 0);
}
///
/// RE_AddSenderInfo
//  Displays sender information to header listview
static void RE_AddSenderInfo(int winnum, struct ABEntry *ab)
{
   APTR lv = G->RE[winnum]->GUI.LV_HEAD;
   RE_AddExtraHeader(lv, GetStr(MSG_EA_RealName), ab->RealName);
   RE_AddExtraHeader(lv, GetStr(MSG_EA_Street), ab->Street);
   RE_AddExtraHeader(lv, GetStr(MSG_EA_City), ab->City);
   RE_AddExtraHeader(lv, GetStr(MSG_EA_Country), ab->Country);
   RE_AddExtraHeader(lv, GetStr(MSG_EA_Phone), ab->Phone);
   RE_AddExtraHeader(lv, GetStr(MSG_EA_DOB), AB_ExpandBD(ab->BirthDay));
   RE_AddExtraHeader(lv, GetStr(MSG_EA_Description), ab->Comment);
   RE_AddExtraHeader(lv, GetStr(MSG_EA_Homepage), ab->Homepage);
}
///
/// RE_AddToAddrbook
//  Adds sender to the address book
static struct ABEntry *RE_AddToAddrbook(APTR win, struct ABEntry *templ)
{
   struct ABEntry ab_new;
   char buf[SIZE_LARGE];
   BOOL doit = FALSE;

   switch (C->AddToAddrbook)
   {
      case 1: if (!templ->Type) break;
      case 2: sprintf(buf, GetStr(MSG_RE_AddSender), BuildAddrName(templ->Address, templ->RealName));
              doit = MUI_Request(G->App, win, 0, NULL, GetStr(MSG_YesNoReq), buf);
              break;
      case 3: if (!templ->Type) break;
      case 4: doit = TRUE;
   }

   if (doit)
   {
      struct MUI_NListtree_TreeNode *tn = NULL;

      // first we check if the group for new entries already exists and if so
      // we add this address to this special group.
      if(C->NewAddrGroup[0])
      {
         tn = (struct MUI_NListtree_TreeNode *)DoMethod(G->AB->GUI.LV_ADDRESSES, MUIM_NListtree_FindName, MUIV_NListtree_FindName_ListNode_Root, C->NewAddrGroup, MUIF_NONE);

         // only if the group doesn`t exist yet
         if(!tn || ((struct ABEntry *)tn->tn_User)->Type != AET_GROUP)
         {
            memset(&ab_new, 0, sizeof(struct ABEntry));
            stccpy(ab_new.Alias, C->NewAddrGroup, SIZE_NAME);
            stccpy(ab_new.Comment, GetStr(MSG_RE_NewGroupTitle), SIZE_DEFAULT);
            ab_new.Type = AET_GROUP;
            tn = (struct MUI_NListtree_TreeNode *)DoMethod(G->AB->GUI.LV_ADDRESSES, MUIM_NListtree_Insert, ab_new.Alias, &ab_new, MUIV_NListtree_Insert_ListNode_Root, MUIV_NListtree_Insert_PrevNode_Sorted, TNF_LIST);
         }
      }

      // then lets add the entry to the group that was perhaps
      // created previously.
      memset(&ab_new, 0, sizeof(struct ABEntry));
      ab_new.Type = AET_USER;
      RE_UpdateSenderInfo(&ab_new, templ);
      EA_SetDefaultAlias(&ab_new);
      tn = (struct MUI_NListtree_TreeNode *)DoMethod(G->AB->GUI.LV_ADDRESSES, MUIM_NListtree_Insert, ab_new.Alias, &ab_new, tn ? tn : MUIV_NListtree_Insert_ListNode_Root, MUIV_NListtree_Insert_PrevNode_Sorted, MUIF_NONE);
      if (tn)
      {
         CallHookPkt(&AB_SaveABookHook, 0, 0);
         return tn->tn_User;
      }
   }
   return NULL;
}
///
/// RE_FindPhotoOnDisk
//  Searches portrait of sender in the gallery directory
static BOOL RE_FindPhotoOnDisk(struct ABEntry *ab, char *photo)
{
   *photo = 0;
   if (*ab->Photo) strcpy(photo, ab->Photo);
   else if (*C->GalleryDir)
   {
      char fname[SIZE_FILE];
      stccpy(fname, ab->RealName, SIZE_FILE);
      if (PFExists(C->GalleryDir, fname)) strmfp(photo, C->GalleryDir, fname);
      else
      {
         stccpy(fname, ab->Address, SIZE_FILE);
         if (PFExists(C->GalleryDir, fname)) strmfp(photo, C->GalleryDir, fname);
      }
   }
   if (!*photo) return FALSE;
   return (BOOL)(FileSize(photo) > 0);
}
///
/// RE_DownloadPhoto
//  Downloads portrait photograph of sender from the YAM homepage
static BOOL RE_DownloadPhoto(APTR win, char *url, struct ABEntry *ab)
{
   char fname[SIZE_FILE], picfname[SIZE_PATHFILE], ext[SIZE_SMALL];
   char *name = *ab->Alias ? ab->Alias : "pic";
   int i;
   BOOL success = FALSE, doit = FALSE;

   switch (C->AddToAddrbook)
   {
      case 1: case 2: doit = MUI_Request(G->App, win, 0, NULL, GetStr(MSG_OkayCancelReq), GetStr(MSG_RE_DownloadPhotoReq)); break;
      case 3: case 4: doit = TR_IsOnline();
   }
   if (doit)
   {
      if (!stcgfe(ext, url)) strcpy(ext, "iff");
      sprintf(fname, "%s.%s", name, ext);
      for (i = 2; PFExists(C->GalleryDir, fname); i++) sprintf(fname, "%s%d.%s", name, i, ext);
      strmfp(picfname, C->GalleryDir, fname);
      if (TR_OpenTCPIP())
      {
         BusyText(GetStr(MSG_BusyDownloadingPic), name);
         CreateDirectory(C->GalleryDir);
         if (TR_DownloadURL(url, NULL, NULL, picfname))
         {
            strcpy(ab->Photo, picfname);
            CallHookPkt(&AB_SaveABookHook, 0, 0);
            success = TRUE;
         }
         BusyEnd;
         TR_CloseTCPIP();
      }
      else ER_NewError(GetStr(MSG_ER_NoTCP), NULL, NULL);
   }
   return success;
}
///
/// RE_DisplayMessage
//  Shows message header and body in read window
static void RE_DisplayMessage(int winnum, BOOL update)
{
   char *cmsg, *body;
   BOOL dispheader;
   struct RE_GUIData *gui = &(G->RE[winnum]->GUI);
   struct Person *from = &G->RE[winnum]->Mail.From;
   struct ABEntry *ab = NULL, abtmpl;
   int hits;

   BusyText(GetStr(MSG_BusyDisplaying), "");

   if ((cmsg = RE_ReadInMessage(winnum, RIM_READ)))
   {
      char headername[SIZE_DEFAULT];
      int i;

      dispheader = (G->RE[winnum]->Header != HM_NOHEADER);
      set(gui->GR_HEAD, MUIA_ShowMe, dispheader);
      set(gui->BO_BALANCE, MUIA_ShowMe, dispheader);
      DoMethod(gui->LV_HEAD, MUIM_NList_Clear);
      set(gui->LV_HEAD, MUIA_NList_Quiet, TRUE);
      body = cmsg;

      // here we have to parse the header and place all header
      // information in the header listview.
      while (*body)
      {
         // if the first char in the line is a newline \n, then we found the start of
         // the body and break here.
         if (*body == '\n') { body++; break; }

         // we copy the headername from the mail as long as there is no space and
         // no interrupting character is found
         for(i = 0; body[i] != ':' && !isspace(body[i]) && body[i] != '\n' && body[i] != '\0' && i < SIZE_DEFAULT-1; i++)
         {
           headername[i] = body[i];
         }

         // if we end up here and body[i] isn`t a : then this wasn`t a proper headerline and we
         // can ignore it anyway because the RFC says that a headerline must have characters
         // without any space followed by a ":"
         if(body[i] == ':')
         {
           headername[i] = '\0'; // terminate with 0

           // Now we check if this is a header the user wants to be displayed if he has choosen
           // to display only shortheaders
           if(G->RE[winnum]->Header == HM_SHORTHEADER) dispheader = MatchNoCase(headername, C->ShortHeaders);
           else dispheader = (G->RE[winnum]->Header == HM_FULLHEADER);

           if(dispheader)
           {
              // we simply insert the whole thing from the actual body pointer
              // because the ConstructHook_String of NList will anyway just copy until a \n, \r or \0
              DoMethod(gui->LV_HEAD, MUIM_NList_InsertSingleWrap, body, MUIV_NList_Insert_Bottom, G->RE[winnum]->WrapHeader ? WRAPCOL1 : NOWRAP, ALIGN_LEFT);
           }
         }

         // then we move forward until the end of the line
         while (*body && *body != '\n') body++;
         if (*body) body++; // if the end of the line isn`t a \0 we have to move on
      }

      if ((hits = AB_SearchEntry(from->Address, ASM_ADDRESS|ASM_USER, &ab)) == 0 && *from->RealName)
      {
        hits = AB_SearchEntry(from->RealName, ASM_REALNAME|ASM_USER, &ab);
      }

      RE_GetSenderInfo(G->RE[winnum]->MailPtr, &abtmpl);

      if (!stricmp(from->Address, C->EmailAddress) || !stricmp(from->RealName, C->RealName))
      {
         if (!ab) { ab = &abtmpl; *ab->Photo = 0; }
      }
      else
      {
         if (ab)
         {
            RE_UpdateSenderInfo(ab, &abtmpl);
            if(!update && C->AddToAddrbook > 0 && !*ab->Photo && *abtmpl.Photo && *C->GalleryDir)
            {
              RE_DownloadPhoto(gui->WI, abtmpl.Photo, ab);
            }
         }
         else
         {
            if(!update && C->AddToAddrbook > 0 && (ab = RE_AddToAddrbook(gui->WI, &abtmpl)))
            {
               if (*abtmpl.Photo && *C->GalleryDir) RE_DownloadPhoto(gui->WI, abtmpl.Photo, ab);
            }
            else { ab = &abtmpl; *ab->Photo = 0; }
         }
      }
      if (G->RE[winnum]->SenderInfo)
      {
         if (hits == 1 || ab->Type == AET_LIST) RE_AddSenderInfo(winnum, ab);
         if (G->RE[winnum]->SenderInfo == 2) if (DoMethod(gui->GR_PHOTO, MUIM_Group_InitChange))
         {
            char photopath[SIZE_PATHFILE];
            if (gui->BC_PHOTO)
            {
               DoMethod(gui->GR_PHOTO, OM_REMMEMBER, gui->BC_PHOTO);
               MUI_DisposeObject(gui->BC_PHOTO);
            }
            gui->BC_PHOTO = NULL;
            if (RE_FindPhotoOnDisk(ab, photopath))
            {
               gui->BC_PHOTO = MakePicture(photopath);
               DoMethod(gui->GR_PHOTO, OM_ADDMEMBER, gui->BC_PHOTO);
            }
            DoMethod(gui->GR_PHOTO, MUIM_Group_ExitChange);
         }
      }
      set(gui->GR_INFO, MUIA_ShowMe, (G->RE[winnum]->SenderInfo == 2) && (gui->BC_PHOTO != NULL));
      set(gui->LV_HEAD, MUIA_NList_Quiet, FALSE);

      // before we can put the message body into the TextEditor, we have to preparse the text and
      // try to set some styles, as we don`t use the buggy ImportHooks of TextEditor anymore and are anyway
      // more powerful with that.
      if(!G->RE[winnum]->NoTextstyles) body = ParseEmailText(body);

      SetAttrs(gui->TE_TEXT, MUIA_TextEditor_FixedFont,  G->RE[winnum]->FixedFont,
                             MUIA_TextEditor_Contents,   body,
                             TAG_DONE);

      if(!G->RE[winnum]->NoTextstyles) free(body);
      free(cmsg);
   }

   BusyEnd;
}
///
/// RE_ClickedOnMessage
//  User clicked on a e-mail address
static void RE_ClickedOnMessage(char *address)
{
   struct ABEntry *ab = NULL;
   int l, win, hits;
   char *p, *gads, buf[SIZE_LARGE];
   char *body = NULL, *subject = NULL, *cc = NULL, *bcc = NULL;

   DB(kprintf("ClickedOnMessage: [%s]\n", address);)

   // just prevent something bad from happening.
   if(!address || !(l = strlen(address))) return;

   // now we check for additional options to the mailto: string (if it is one)
   if((p = strchr(address, '?'))) *p++ = '\0';

   while(p)
   {
      if(!strnicmp(p, "subject=", 8))    subject = &p[8];
      else if(!strnicmp(p, "body=", 5))  body = &p[5];
      else if(!strnicmp(p, "cc=", 3))    cc = &p[3];
      else if(!strnicmp(p, "bcc=", 4))   bcc = &p[4];

      if((p = strchr(p, '&'))) *p++ = '\0';

      // now we check if this "&" is because of a "&amp;" which is the HTML code
      // for a "&" - we only handle this code because handling ALL HTML code
      // would be too complicated right now. we will support that later anyway.
      if(p && !strnicmp(p, "amp;", 4)) p+=4;
   }

   // please note that afterwards we should normally transfer HTML specific codes
   // like &amp; %20 aso. because otherwise links like mailto:Bilbo%20Baggins%20&lt;bilbo@baggins.de&gt;
   // will not work... but this is stuff we can do in one of the next versions.

   // lets see if we have an entry for that in the Addressbook
   // and if so, we reuse it
   hits = AB_SearchEntry(address, ASM_ADDRESS|ASM_USER|ASM_LIST, &ab);

   sprintf(buf, GetStr(MSG_RE_SelectAddressReq), address);
   gads = GetStr(hits ? MSG_RE_SelectAddressEdit : MSG_RE_SelectAddressAdd);

   switch (MUI_Request(G->App, G->MA->GUI.WI, 0, NULL, gads, buf))
   {
      case 1:
      {
        if ((win = MA_NewNew(NULL, 0)) >= 0)
        {
          struct WR_GUIData *gui = &G->WR[win]->GUI;

          setstring(gui->ST_TO, hits ? BuildAddrName(address, ab->RealName) : address);
          if (subject) setstring(gui->ST_SUBJECT, subject);
          if (body) set(gui->TE_EDIT, MUIA_TextEditor_Contents, body);
          if (cc) setstring(gui->ST_CC, cc);
          if (bcc) setstring(gui->ST_BCC, bcc);
          set(gui->WI, MUIA_Window_ActiveObject, gui->ST_SUBJECT);
        }
      }
      break;

      case 2:
      {
        DoMethod(G->App, MUIM_CallHook, &AB_OpenHook, ABM_EDIT, TAG_DONE);
        if (hits)
        {
          if ((win = EA_Init(ab->Type, ab)) >= 0) EA_Setup(win, ab);
        }
        else
        {
          if ((win = EA_Init(AET_USER, NULL)) >= 0) setstring(G->EA[win]->GUI.ST_ADDRESS, address);
        }
      }
      break;
   }
}
///
/// RE_DoubleClickFunc
//  Handles double-clicks on an URL
HOOKPROTONH(RE_DoubleClickFunc, BOOL, APTR obj, struct ClickMessage *clickmsg)
{
   char *p;
   BOOL result = FALSE;

   DB(kprintf("DoubleClick: %ld - [%s]\n", clickmsg->ClickPosition, clickmsg->LineContents);)

   DoMethod(G->App, MUIM_Application_InputBuffered);

   // for safety reasons
   if(!clickmsg->LineContents) return FALSE;

   // if the user clicked on space we skip the following
   // analysis of a URL and just check if it was an attachment the user clicked at
   if(!ISpace(clickmsg->LineContents[clickmsg->ClickPosition]))
   {
      int pos = clickmsg->ClickPosition;
      char *line, *surl;
      static char url[SIZE_URL];
      enum tokenType type;

      // then we make a copy of the LineContents
      if(!(line = StrBufCpy(NULL, clickmsg->LineContents))) return FALSE;

      // find the beginning of the word we clicked at
      surl = &line[pos];
      while(surl != &line[0] && !ISpace(*(surl-1))) surl--;

      // now find the end of the word the user clicked at
      p = &line[pos];
      while(p+1 != &line[strlen(line)] && !ISpace(*(p+1))) p++;
      *(++p) = '\0';

      // now we start our quick lexical analysis to find a clickable element within
      // the doubleclick area
      if((type = ExtractURL(surl, url)))
      {
        switch(type)
        {
          case tEMAIL:
          {
            RE_ClickedOnMessage(url);
          }
          break;

          case tMAILTO:
          {
            RE_ClickedOnMessage(&url[7]);
          }
          break;

          case tHTTP:
          case tHTTPS:
          case tFTP:
          case tGOPHER:
          case tTELNET:
          case tNEWS:
          case tURL:
          {
            GotoURL(url);
          }
          break;

          default:
            // nothing
          break;
        }

        result = TRUE;
      }

      FreeStrBuf(line);
   }

   // if we still don`t have a result here we check if the user clicked on
   // a attachment.
   if(result == FALSE)
   {
      p = clickmsg->LineContents;
      if(isdigit(p[0]) && ((p[1] == ':' && p[2] == ' ') || (p[2] == ':' && p[3] == ' ')))
      {
        struct Part *part = RE_GetPart((int)xget(_win(obj), MUIA_UserData), atoi(p));

        if(part)
        {
          RE_DecodePart(part);
          RE_DisplayMIME(part->Filename, part->ContentType);

          result = TRUE;
        }
      }
   }

   return result;
}
MakeStaticHook(RE_DoubleClickHook, RE_DoubleClickFunc);
///
/// RE_ShowEnvFunc
//  Changes display options (header, textstyles, sender info)
HOOKPROTONHNO(RE_ShowEnvFunc, void, int *arg)
{
   int lev, winnum = arg[0], mode = arg[1];
   struct RE_ClassData *re = G->RE[winnum];
   long opt;

   get(re->GUI.SL_TEXT, MUIA_Prop_First, &lev);
   switch (mode)
   {
      case 0: case 1: case 2: re->Header = mode;
                              break;
      case 3: case 4: case 5: re->SenderInfo = mode-3;
                              break;
      case 6:  get(re->GUI.MI_WRAPH, MUIA_Menuitem_Checked, &opt);
               re->WrapHeader = opt; break;
      case 7:  get(re->GUI.MI_TSTYLE, MUIA_Menuitem_Checked, &opt);
               re->NoTextstyles = !opt; break;
      case 8:  get(re->GUI.MI_FFONT, MUIA_Menuitem_Checked, &opt);
               re->FixedFont = opt; break;
   }
   RE_DisplayMessage(winnum, TRUE);
   set(re->GUI.SL_TEXT, MUIA_Prop_First, lev);
}
MakeStaticHook(RE_ShowEnvHook, RE_ShowEnvFunc);
///

/*** GUI ***/
/// RE_LV_AttachDspFunc
//  Attachment listview display hook
HOOKPROTONH(RE_LV_AttachDspFunc, long, char **array, struct Part *entry)
{
   if (entry)
   {
      static char dispnu[SIZE_SMALL], dispsz[SIZE_SMALL];
      array[0] = array[2] = "";
      if (entry->Nr > PART_RAW) sprintf(array[0] = dispnu, "%d", entry->Nr);

      if(*entry->Name) array[1] = entry->Name;
      else             array[1] = DescribeCT(entry->ContentType);

      if (entry->Size)
      {
        sprintf(array[2] = dispsz, "%s", entry->Decoded ? "" : "~");
        FormatSize(entry->Size, dispsz);
      }
   }
   else
   {
      array[0] = GetStr(MSG_ATTACH_NO);
      array[1] = GetStr(MSG_ATTACH_PART);
      array[2] = GetStr(MSG_Size);
   }

   return 0;
}
MakeHook(RE_LV_AttachDspFuncHook,RE_LV_AttachDspFunc);
///
/// RE_CloseFunc
//  Closes a read window
HOOKPROTONHNO(RE_CloseFunc, void, int *arg)
{
   int winnum = *arg;
   struct RE_ClassData *re = G->RE[winnum];

   RE_CleanupMessage(winnum);
   if(isVirtualMail(re->MailPtr))
   {
      free(re->MailPtr);
      CloseTempFile(re->TempFile);
   }
   G->Weights[2] = xget(re->GUI.GR_HEAD, MUIA_VertWeight);
   G->Weights[3] = xget(re->GUI.GR_BODY, MUIA_VertWeight);
   DisposeModulePush(&G->RE[winnum]);
}
MakeHook(RE_CloseHook, RE_CloseFunc);
///
/// RE_Open
//  Opens a read window
int RE_Open(int winnum, BOOL real)
{
   if (winnum < 0) for (winnum = 0; winnum < 4; winnum++) if (!G->RE[winnum]) break;
   if (winnum > 3) return -1;
   if (!G->RE[winnum])
   {
      if (!(G->RE[winnum] = RE_New(winnum, real))) return -1;
      G->RE[winnum]->Header = C->ShowHeader;
   }
   return winnum;
}
///
/// RE_LV_HDspFunc
//  Header listview display hook
HOOKPROTONH(RE_LV_HDspFunc, long, char **array, char *entry)
{
   static char hfield[40];
   char *cont = entry;
   int i = 0;

   // copy the headername into the static hfield to display it.
   while (*cont != ':' && *cont && i < 38) hfield[i++] = *cont++;
   hfield[i] = '\0';

   // set the array now so that the NList shows the correct values.
   array[0] = hfield;
   array[1] = stpblk(++cont);

   return 0;
}
MakeStaticHook(RE_LV_HDspHook,RE_LV_HDspFunc);
///
/// RE_New
//  Creates a read window
static struct RE_ClassData *RE_New(int winnum, BOOL real)
{
   struct RE_ClassData *data = calloc(1, sizeof(struct RE_ClassData));
   if (data)
   {
      enum {
        RMEN_EDIT=501,RMEN_MOVE,RMEN_COPY,RMEN_DELETE,RMEN_PRINT,RMEN_SAVE,RMEN_DISPLAY,RMEN_DETACH,
        RMEN_CROP,RMEN_NEW,RMEN_REPLY,RMEN_FORWARD,RMEN_BOUNCE,RMEN_SAVEADDR,RMEN_SETUNREAD,RMEN_SETMARKED,
        RMEN_CHSUBJ,RMEN_PREV,RMEN_NEXT,RMEN_URPREV,RMEN_URNEXT,RMEN_PREVTH,RMEN_NEXTTH,
        RMEN_EXTKEY,RMEN_CHKSIG,RMEN_SAVEDEC,
        RMEN_HNONE,RMEN_HSHORT,RMEN_HFULL,RMEN_SNONE,RMEN_SDATA,RMEN_SFULL,RMEN_WRAPH,RMEN_TSTYLE,RMEN_FFONT
      };

      static const struct NewToolbarEntry tb_butt[ARRAY_SIZE(data->GUI.TB_TOOLBAR)] = {
        { MSG_RE_TBPrev,    MSG_HELP_RE_BT_PREVIOUS },
        { MSG_RE_TBNext,    MSG_HELP_RE_BT_NEXT     },
        { MSG_RE_TBPrevTh,  MSG_HELP_RE_BT_QUESTION },
        { MSG_RE_TBNextTh,  MSG_HELP_RE_BT_ANSWER   },
        { MSG_Space,        NULL                    },
        { MSG_RE_TBDisplay, MSG_HELP_RE_BT_DISPLAY  },
        { MSG_RE_TBSave,    MSG_HELP_RE_BT_EXPORT   },
        { MSG_RE_TBPrint,   MSG_HELP_RE_BT_PRINT    },
        { MSG_Space,        NULL                    },
        { MSG_RE_TBDelete,  MSG_HELP_RE_BT_DELETE   },
        { MSG_RE_TBMove,    MSG_HELP_RE_BT_MOVE     },
        { MSG_RE_TBReply,   MSG_HELP_RE_BT_REPLY    },
        { MSG_RE_TBForward, MSG_HELP_RE_BT_FORWARD  },
        { NULL,             NULL                    }
      };
      int i;

      for (i = 0; i < ARRAY_SIZE(data->GUI.TB_TOOLBAR); i++)
        SetupToolbar(&(data->GUI.TB_TOOLBAR[i]), tb_butt[i].label?(tb_butt[i].label==MSG_Space?"":GetStr(tb_butt[i].label)):NULL, tb_butt[i].help?GetStr(tb_butt[i].help):NULL, 0);

      data->GUI.SL_TEXT = ScrollbarObject, End;
      data->Header = C->ShowHeader;
      data->SenderInfo = C->ShowSenderInfo;
      data->WrapHeader = C->WrapHeader;
      data->NoTextstyles = !C->UseTextstyles;
      data->FixedFont = C->FixedFontEdit;
      data->GUI.WI = WindowObject,
         MUIA_Window_Title, "",
         MUIA_HelpNode, "RE_W",
         MUIA_Window_ID, MAKE_ID('R','E','A','D'),
         MUIA_UserData, winnum,
         MUIA_Window_Menustrip, MenustripObject,
            MUIA_Family_Child, MenuObject, MUIA_Menu_Title, GetStr(MSG_Message),
               MUIA_Family_Child, data->GUI.MI_EDIT = MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_MA_MEdit), MUIA_Menuitem_Shortcut,"E", MUIA_Menuitem_Enabled,real, MUIA_UserData,RMEN_EDIT, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_MA_MMove), MUIA_Menuitem_Shortcut, "M", MUIA_Menuitem_Enabled,real, MUIA_UserData,RMEN_MOVE, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_MA_MCopy),MUIA_Menuitem_Shortcut,"Y",  MUIA_UserData,RMEN_COPY, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_MA_MDelete), MUIA_Menuitem_Shortcut,"Del", MUIA_Menuitem_Enabled,real, MUIA_Menuitem_CommandString,TRUE, MUIA_UserData,RMEN_DELETE, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,NM_BARLABEL, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_Print), MUIA_Menuitem_Shortcut, "P", MUIA_UserData,RMEN_PRINT, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_MA_Save), MUIA_Menuitem_Shortcut, "S", MUIA_UserData,RMEN_SAVE, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_Attachments),
                  MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_RE_MDisplay), MUIA_Menuitem_Shortcut, "D", MUIA_UserData,RMEN_DISPLAY, End,
                  MUIA_Family_Child, data->GUI.MI_DETACH = MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_RE_SaveAll), MUIA_Menuitem_Shortcut,"A", MUIA_UserData,RMEN_DETACH, End,
                  MUIA_Family_Child, data->GUI.MI_CROP = MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_MA_Crop), MUIA_Menuitem_Shortcut,"O", MUIA_Menuitem_Enabled,real, MUIA_UserData,RMEN_CROP, End,
              End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,NM_BARLABEL, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_New), MUIA_Menuitem_Shortcut,"N", MUIA_UserData,RMEN_NEW, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_MA_MReply), MUIA_Menuitem_Shortcut,"R", MUIA_UserData,RMEN_REPLY, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_MA_MForward), MUIA_Menuitem_Shortcut,"W", MUIA_UserData,RMEN_FORWARD, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_MA_MBounce), MUIA_Menuitem_Shortcut,"B", MUIA_UserData,RMEN_BOUNCE, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,NM_BARLABEL, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_MA_MGetAddress), MUIA_Menuitem_Shortcut,"J", MUIA_UserData,RMEN_SAVEADDR, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_RE_SetUnread), MUIA_Menuitem_Shortcut,"U", MUIA_UserData,RMEN_SETUNREAD, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_RE_SETMARKED), MUIA_Menuitem_Shortcut,",", MUIA_UserData,RMEN_SETMARKED, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_MA_ChangeSubj), MUIA_Menuitem_Enabled,real, MUIA_UserData,RMEN_CHSUBJ, End,
            End,
            MUIA_Family_Child, MenuObject, MUIA_Menu_Title, GetStr(MSG_RE_Navigation),
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_RE_MNext),  MUIA_Menuitem_Shortcut, "right", MUIA_Menuitem_Enabled,real, MUIA_Menuitem_CommandString,TRUE, MUIA_UserData,RMEN_NEXT, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_RE_MPrev),  MUIA_Menuitem_Shortcut, "left", MUIA_Menuitem_Enabled,real, MUIA_Menuitem_CommandString,TRUE, MUIA_UserData,RMEN_PREV, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_RE_MURNext),MUIA_Menuitem_Shortcut, "shift right", MUIA_Menuitem_Enabled,real, MUIA_Menuitem_CommandString,TRUE, MUIA_UserData,RMEN_URNEXT, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_RE_MURPrev),MUIA_Menuitem_Shortcut, "shift left", MUIA_Menuitem_Enabled,real, MUIA_Menuitem_CommandString,TRUE, MUIA_UserData,RMEN_URPREV, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_RE_MNextTh),MUIA_Menuitem_Shortcut, ">", MUIA_Menuitem_Enabled,real, MUIA_UserData,RMEN_NEXTTH, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_RE_MPrevTh),MUIA_Menuitem_Shortcut, "<", MUIA_Menuitem_Enabled,real, MUIA_UserData,RMEN_PREVTH, End,
            End,
            MUIA_Family_Child, MenuObject, MUIA_Menu_Title, "PGP",
               MUIA_Family_Child, data->GUI.MI_EXTKEY = MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_RE_ExtractKey), MUIA_Menuitem_Shortcut,"X", MUIA_UserData,RMEN_EXTKEY, End,
               MUIA_Family_Child, data->GUI.MI_CHKSIG = MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_RE_SigCheck), MUIA_Menuitem_Shortcut,"K", MUIA_UserData,RMEN_CHKSIG, End,
               MUIA_Family_Child, data->GUI.MI_SAVEDEC = MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_RE_SaveDecrypted), MUIA_Menuitem_Shortcut,"V", MUIA_UserData,RMEN_SAVEDEC, End,
            End,
            MUIA_Family_Child, MenuObject, MUIA_Menu_Title, GetStr(MSG_MA_Settings),
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_RE_NoHeaders),  MUIA_Menuitem_Shortcut,"0", MUIA_Menuitem_Checkit,TRUE, MUIA_Menuitem_Checked,data->Header==HM_NOHEADER, MUIA_Menuitem_Exclude,0x06, MUIA_UserData,RMEN_HNONE, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_RE_ShortHeaders), MUIA_Menuitem_Shortcut,"1", MUIA_Menuitem_Checkit,TRUE, MUIA_Menuitem_Checked,data->Header==HM_SHORTHEADER, MUIA_Menuitem_Exclude,0x05, MUIA_UserData,RMEN_HSHORT, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_RE_FullHeaders),  MUIA_Menuitem_Shortcut,"2", MUIA_Menuitem_Checkit,TRUE, MUIA_Menuitem_Checked,data->Header==HM_FULLHEADER, MUIA_Menuitem_Exclude,0x03, MUIA_UserData,RMEN_HFULL, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,NM_BARLABEL, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_RE_NoSInfo), MUIA_Menuitem_Shortcut,"3", MUIA_Menuitem_Checkit,TRUE, MUIA_Menuitem_Checked,data->SenderInfo==0, MUIA_Menuitem_Exclude,0x60, MUIA_UserData,RMEN_SNONE, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_RE_SInfo), MUIA_Menuitem_Shortcut,"4", MUIA_Menuitem_Checkit,TRUE, MUIA_Menuitem_Checked,data->SenderInfo==1, MUIA_Menuitem_Exclude,0x50, MUIA_UserData,RMEN_SDATA, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_RE_SInfoImage), MUIA_Menuitem_Shortcut,"5", MUIA_Menuitem_Checkit,TRUE, MUIA_Menuitem_Checked,data->SenderInfo==2, MUIA_Menuitem_Exclude,0x30, MUIA_UserData,RMEN_SFULL, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,NM_BARLABEL, End,
               MUIA_Family_Child, data->GUI.MI_WRAPH = MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_RE_WrapHeader), MUIA_Menuitem_Shortcut,"H", MUIA_Menuitem_Checkit,TRUE, MUIA_Menuitem_Checked,data->WrapHeader, MUIA_Menuitem_Toggle,TRUE, MUIA_UserData,RMEN_WRAPH, End,
               MUIA_Family_Child, data->GUI.MI_TSTYLE = MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_RE_Textstyles), MUIA_Menuitem_Shortcut,"T", MUIA_Menuitem_Checkit,TRUE, MUIA_Menuitem_Checked,!data->NoTextstyles, MUIA_Menuitem_Toggle,TRUE, MUIA_UserData,RMEN_TSTYLE, End,
               //MUIA_Family_Child, data->GUI.MI_FFONT = MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_RE_FixedFont), MUIA_Menuitem_Shortcut,"F", MUIA_Menuitem_Checkit,TRUE, MUIA_Menuitem_Checked,data->FixedFont, MUIA_Menuitem_Toggle,TRUE, MUIA_UserData,RMEN_FFONT, End,
            End,
         End,
         WindowContents, VGroup,
            Child, hasHideToolBarFlag(C->HideGUIElements) ?
               (RectangleObject, MUIA_ShowMe, FALSE, End) :
               (HGroup, GroupSpacing(0),
                  Child, HGroupV,
                     Child, data->GUI.TO_TOOLBAR = ToolbarObject,
                        MUIA_HelpNode, "RE_B",
                        MUIA_Toolbar_ImageType,      MUIV_Toolbar_ImageType_File,
                        MUIA_Toolbar_ImageNormal,    "PROGDIR:Icons/Read.toolbar",
                        MUIA_Toolbar_ImageGhost,     "PROGDIR:Icons/Read_G.toolbar",
                        MUIA_Toolbar_ImageSelect,    "PROGDIR:Icons/Read_S.toolbar",
                        MUIA_Toolbar_Description,    data->GUI.TB_TOOLBAR,
                        MUIA_Toolbar_ParseUnderscore,TRUE,
                        MUIA_Font,                   MUIV_Font_Tiny,
                        MUIA_ShortHelp, TRUE,
                     End,
                     Child, HSpace(0),
                  End,
                  Child, HSpace(8),
                  Child, VGroup,
                     Child, VSpace(0),
                     Child, HGroup,
                        TextFrame,
                        MUIA_Group_Spacing, 1,
                        MUIA_Background, MUII_TextBack,
                        Child, data->GUI.GR_STATUS[0] = PageGroup,
                           Child, HSpace(0),
                           Child, MakeStatusFlag("status_unread"),
                           Child, MakeStatusFlag("status_old"),
                           Child, MakeStatusFlag("status_forward"),
                           Child, MakeStatusFlag("status_reply"),
                           Child, MakeStatusFlag("status_waitsend"),
                           Child, MakeStatusFlag("status_error"),
                           Child, MakeStatusFlag("status_hold"),
                           Child, MakeStatusFlag("status_sent"),
                           Child, MakeStatusFlag("status_new"),
                        End,
                        Child, data->GUI.GR_STATUS[1] = PageGroup,
                           Child, HSpace(0),
                           Child, MakeStatusFlag("status_crypt"),
                           Child, MakeStatusFlag("status_signed"),
                           Child, MakeStatusFlag("status_report"),
                           Child, MakeStatusFlag("status_attach"),
                        End,
                        Child, data->GUI.GR_STATUS[2] = PageGroup,
                           Child, HSpace(0),
                           Child, MakeStatusFlag("status_urgent"),
                        End,
                        Child, data->GUI.GR_STATUS[3] = PageGroup,
                           Child, HSpace(0),
                           Child, MakeStatusFlag("status_mark"),
                        End,
                     End,
                     Child, VSpace(0),
                  End,
               End),
            Child, VGroup,
               Child, data->GUI.GR_HEAD = HGroup, GroupSpacing(0),
                  MUIA_ShowMe, data->Header != HM_NOHEADER,
                  MUIA_VertWeight, G->Weights[2],
                  Child, NListviewObject,
                     MUIA_NListview_NList, data->GUI.LV_HEAD = NListObject,
                        InputListFrame,
                        MUIA_NList_ConstructHook, MUIV_NList_ConstructHook_String,
                        MUIA_NList_DestructHook, MUIV_NList_DestructHook_String,
                        MUIA_NList_DisplayHook, &RE_LV_HDspHook,
                        MUIA_NList_Format, "P=\033r\0338 W=-1 MIW=-1,",
                        MUIA_NList_Input, FALSE,
                        MUIA_NList_TypeSelect, MUIV_NList_TypeSelect_Char,
                        MUIA_NList_DefaultObjectOnClick, FALSE,
                        MUIA_ContextMenu, NULL,
                        MUIA_CycleChain, 1,
                     End,
                  End,
                  Child, data->GUI.GR_INFO = ScrollgroupObject,
                     MUIA_ShowMe, FALSE,
                     MUIA_Scrollgroup_FreeHoriz, FALSE,
                     MUIA_HorizWeight, 0,
                     MUIA_Scrollgroup_Contents, data->GUI.GR_PHOTO = VGroupV, GroupSpacing(0),
                        InputListFrame,
                        Child, HVSpace,
                     End,
                  End,
               End,
               Child, data->GUI.BO_BALANCE = BalanceObject,
                  MUIA_ShowMe, data->Header != HM_NOHEADER,
               End,
               Child, data->GUI.GR_BODY = HGroup,
                  MUIA_VertWeight, G->Weights[3],
                  MUIA_Group_Spacing, 0,
                  Child, data->GUI.TE_TEXT = NewObject(CL_TextEditor->mcc_Class,NULL,
                     InputListFrame,
                     MUIA_TextEditor_Slider, data->GUI.SL_TEXT,
                     MUIA_TextEditor_FixedFont, data->FixedFont,
                     MUIA_TextEditor_DoubleClickHook, &RE_DoubleClickHook,
//                     MUIA_TextEditor_ImportHook, MUIV_TextEditor_ImportHook_EMail,
                     MUIA_TextEditor_ImportHook, MUIV_TextEditor_ImportHook_Plain,
                     MUIA_TextEditor_ExportHook, MUIV_TextEditor_ExportHook_Plain,
                     MUIA_TextEditor_ReadOnly, TRUE,
                     MUIA_TextEditor_ColorMap, G->EdColMap,
                     MUIA_CycleChain, TRUE,
                  End,
                  Child, data->GUI.SL_TEXT,
               End,
            End,
         End,
      End;
      if (data->GUI.WI)
      {
         DoMethod(G->App, OM_ADDMEMBER, data->GUI.WI);
         set(data->GUI.WI,MUIA_Window_DefaultObject,data->GUI.TE_TEXT);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,RMEN_EDIT            ,MUIV_Notify_Application,5,MUIM_CallHook,&RE_NewHook,NEW_EDIT,0,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,RMEN_MOVE            ,MUIV_Notify_Application,3,MUIM_CallHook,&RE_MoveHook,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,RMEN_COPY            ,MUIV_Notify_Application,3,MUIM_CallHook,&RE_CopyHook,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,RMEN_DELETE          ,MUIV_Notify_Application,4,MUIM_CallHook,&RE_DeleteHook,0,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,RMEN_PRINT           ,MUIV_Notify_Application,3,MUIM_CallHook,&RE_PrintHook,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,RMEN_SAVE            ,MUIV_Notify_Application,3,MUIM_CallHook,&RE_SaveHook,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,RMEN_DISPLAY         ,MUIV_Notify_Application,3,MUIM_CallHook,&RE_DisplayHook,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,RMEN_DETACH          ,MUIV_Notify_Application,3,MUIM_CallHook,&RE_SaveAllHook,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,RMEN_CROP            ,MUIV_Notify_Application,3,MUIM_CallHook,&RE_RemoveAttachHook,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,RMEN_NEW             ,MUIV_Notify_Application,5,MUIM_CallHook,&RE_NewHook,NEW_NEW,0,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,RMEN_REPLY           ,MUIV_Notify_Application,5,MUIM_CallHook,&RE_NewHook,NEW_REPLY,0,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,RMEN_FORWARD         ,MUIV_Notify_Application,5,MUIM_CallHook,&RE_NewHook,NEW_FORWARD,0,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,RMEN_BOUNCE          ,MUIV_Notify_Application,5,MUIM_CallHook,&RE_NewHook,NEW_BOUNCE,0,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,RMEN_SAVEADDR        ,MUIV_Notify_Application,3,MUIM_CallHook,&RE_GetAddressHook,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,RMEN_SETUNREAD       ,MUIV_Notify_Application,3,MUIM_CallHook,&RE_SetUnreadHook,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,RMEN_SETMARKED       ,MUIV_Notify_Application,3,MUIM_CallHook,&RE_SetMarkedHook,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,RMEN_CHSUBJ          ,MUIV_Notify_Application,3,MUIM_CallHook,&RE_ChangeSubjectHook,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,RMEN_PREV            ,MUIV_Notify_Application,5,MUIM_CallHook,&RE_PrevNextHook,-1,0,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,RMEN_NEXT            ,MUIV_Notify_Application,5,MUIM_CallHook,&RE_PrevNextHook,1,0,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,RMEN_URPREV          ,MUIV_Notify_Application,5,MUIM_CallHook,&RE_PrevNextHook,-1,IEQUALIFIER_LSHIFT,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,RMEN_URNEXT          ,MUIV_Notify_Application,5,MUIM_CallHook,&RE_PrevNextHook,1,IEQUALIFIER_LSHIFT,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,RMEN_PREVTH          ,MUIV_Notify_Application,4,MUIM_CallHook,&RE_FollowHook,-1,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,RMEN_NEXTTH          ,MUIV_Notify_Application,4,MUIM_CallHook,&RE_FollowHook,1,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,RMEN_EXTKEY          ,MUIV_Notify_Application,3,MUIM_CallHook,&RE_ExtractKeyHook,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,RMEN_CHKSIG          ,MUIV_Notify_Application,4,MUIM_CallHook,&RE_CheckSignatureHook,TRUE,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,RMEN_SAVEDEC         ,MUIV_Notify_Application,3,MUIM_CallHook,&RE_SaveDecryptedHook,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,RMEN_HNONE           ,MUIV_Notify_Application,4,MUIM_CallHook,&RE_ShowEnvHook,winnum,0);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,RMEN_HSHORT          ,MUIV_Notify_Application,4,MUIM_CallHook,&RE_ShowEnvHook,winnum,1);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,RMEN_HFULL           ,MUIV_Notify_Application,4,MUIM_CallHook,&RE_ShowEnvHook,winnum,2);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,RMEN_SNONE           ,MUIV_Notify_Application,4,MUIM_CallHook,&RE_ShowEnvHook,winnum,3);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,RMEN_SDATA           ,MUIV_Notify_Application,4,MUIM_CallHook,&RE_ShowEnvHook,winnum,4);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,RMEN_SFULL           ,MUIV_Notify_Application,4,MUIM_CallHook,&RE_ShowEnvHook,winnum,5);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,RMEN_WRAPH           ,MUIV_Notify_Application,4,MUIM_CallHook,&RE_ShowEnvHook,winnum,6);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,RMEN_TSTYLE          ,MUIV_Notify_Application,4,MUIM_CallHook,&RE_ShowEnvHook,winnum,7);
//         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,RMEN_FFONT           ,MUIV_Notify_Application,4,MUIM_CallHook,&RE_ShowEnvHook,winnum,8);
         DoMethod(data->GUI.TO_TOOLBAR ,MUIM_Toolbar_Notify, 0, MUIV_Toolbar_Notify_Pressed,FALSE, MUIV_Notify_Application,5,MUIM_CallHook,&RE_PrevNextHook,-1,MUIV_Toolbar_Qualifier,winnum);
         DoMethod(data->GUI.TO_TOOLBAR ,MUIM_Toolbar_Notify, 1, MUIV_Toolbar_Notify_Pressed,FALSE, MUIV_Notify_Application,5,MUIM_CallHook,&RE_PrevNextHook,1,MUIV_Toolbar_Qualifier,winnum);
         DoMethod(data->GUI.TO_TOOLBAR ,MUIM_Toolbar_Notify, 2, MUIV_Toolbar_Notify_Pressed,FALSE, MUIV_Notify_Application,4,MUIM_CallHook,&RE_FollowHook,-1,winnum);
         DoMethod(data->GUI.TO_TOOLBAR ,MUIM_Toolbar_Notify, 3, MUIV_Toolbar_Notify_Pressed,FALSE, MUIV_Notify_Application,4,MUIM_CallHook,&RE_FollowHook,1,winnum);
         DoMethod(data->GUI.TO_TOOLBAR ,MUIM_Toolbar_Notify, 5, MUIV_Toolbar_Notify_Pressed,FALSE, MUIV_Notify_Application,3,MUIM_CallHook,&RE_DisplayHook,winnum);
         DoMethod(data->GUI.TO_TOOLBAR ,MUIM_Toolbar_Notify, 6, MUIV_Toolbar_Notify_Pressed,FALSE, MUIV_Notify_Application,3,MUIM_CallHook,&RE_SaveHook,winnum);
         DoMethod(data->GUI.TO_TOOLBAR ,MUIM_Toolbar_Notify, 7, MUIV_Toolbar_Notify_Pressed,FALSE, MUIV_Notify_Application,3,MUIM_CallHook,&RE_PrintHook,winnum);
         DoMethod(data->GUI.TO_TOOLBAR ,MUIM_Toolbar_Notify, 9, MUIV_Toolbar_Notify_Pressed,FALSE, MUIV_Notify_Application,4,MUIM_CallHook,&RE_DeleteHook,MUIV_Toolbar_Qualifier,winnum);
         DoMethod(data->GUI.TO_TOOLBAR ,MUIM_Toolbar_Notify,10, MUIV_Toolbar_Notify_Pressed,FALSE, MUIV_Notify_Application,3,MUIM_CallHook,&RE_MoveHook,winnum);
         DoMethod(data->GUI.TO_TOOLBAR ,MUIM_Toolbar_Notify,11, MUIV_Toolbar_Notify_Pressed,FALSE, MUIV_Notify_Application,5,MUIM_CallHook,&RE_NewHook,NEW_REPLY,MUIV_Toolbar_Qualifier,winnum);
         DoMethod(data->GUI.TO_TOOLBAR ,MUIM_Toolbar_Notify,12, MUIV_Toolbar_Notify_Pressed,FALSE, MUIV_Notify_Application,5,MUIM_CallHook,&RE_NewHook,NEW_FORWARD,MUIV_Toolbar_Qualifier,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_CloseRequest ,TRUE                 ,MUIV_Notify_Application,3,MUIM_CallHook,&RE_CloseHook,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_InputEvent   ,"-repeat -capslock del"        ,MUIV_Notify_Application,4,MUIM_CallHook,&RE_DeleteHook,0,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_InputEvent   ,"-repeat -capslock shift del"  ,MUIV_Notify_Application,4,MUIM_CallHook,&RE_DeleteHook,IEQUALIFIER_LSHIFT,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_InputEvent   ,"-repeat -capslock space"      ,data->GUI.TE_TEXT      ,2,MUIM_TextEditor_ARexxCmd,"Next Page");
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_InputEvent   ,"-repeat -capslock backspace"  ,data->GUI.TE_TEXT      ,2,MUIM_TextEditor_ARexxCmd,"Previous Page");
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_InputEvent   ,"-repeat -capslock left"       ,MUIV_Notify_Application,5,MUIM_CallHook,&RE_PrevNextHook,-1,FALSE,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_InputEvent   ,"-repeat -capslock right"      ,MUIV_Notify_Application,5,MUIM_CallHook,&RE_PrevNextHook,1,FALSE,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_InputEvent   ,"-repeat -capslock shift left" ,MUIV_Notify_Application,5,MUIM_CallHook,&RE_PrevNextHook,-1,TRUE,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_InputEvent   ,"-repeat -capslock shift right",MUIV_Notify_Application,5,MUIM_CallHook,&RE_PrevNextHook,1,TRUE,winnum);
         return data;
      }
      free(data);
   }
   return NULL;
}
///
