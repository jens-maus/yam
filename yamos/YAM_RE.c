/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 2000  Marcel Beck <mbeck@yam.ch>

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

***************************************************************************/

#include "YAM.h"

/* local protos */
LOCAL void RE_PrintFile(char*,struct Part*);
LOCAL char **Init_ISO8859_to_LaTeX_Tab(char*);
LOCAL char *ISO8859_to_LaTeX(char*);


/***************************************************************************
 Module: Read
***************************************************************************/

/// RE_GetQuestion
//  Finds previous message in a thread
struct Mail *RE_GetQuestion(long irtid)
{
   struct Folder **flist;
   struct Mail *mail;
   int b;
   
   if (irtid) if (flist = FO_CreateList())
   {
     for (b = 1; b <= (int)*flist; b++) if (MA_GetIndex(flist[b]))
        for (mail = flist[b]->Messages; mail; mail = mail->Next)
           if (mail->cMsgID) if (mail->cMsgID == irtid) { free(flist); return mail; }
     free(flist);
   }
   return NULL;
}
///
/// RE_GetAnswer
//  Find next message in a thread
struct Mail *RE_GetAnswer(long id)
{
   struct Folder **flist;
   struct Mail *mail;
   int b;
   
   if (id) if (flist = FO_CreateList())
   {
     for (b = 1; b <= (int)*flist; b++) if (MA_GetIndex(flist[b]))
         for (mail = flist[b]->Messages; mail; mail = mail->Next)
            if (mail->cIRTMsgID) if (mail->cIRTMsgID == id) { free(flist); return mail; }
     free(flist);
   }
   return NULL;
}
///
/// RE_Follow
//  Follows a thread in either direction
SAVEDS ASM void RE_Follow(REG(a1,int *arg))
{  
   int i, direction = arg[0], winnum = arg[1];
   struct Folder **flist;
   struct Mail *fmail = NULL;
   BOOL allloaded = TRUE;

   if (flist = FO_CreateList())
   {
      for (i = 1; i < (int)*flist; i++) if (flist[i]->LoadedMode != 2 && flist[i]->Type != FT_SEPARATOR) allloaded = FALSE;
      free(flist);
   }
   if (!allloaded) if (!MUI_Request(G->App, G->RE[winnum]->GUI.WI, 0, GetStr(MSG_MA_ConfirmReq), GetStr(MSG_YesNoReq), GetStr(MSG_RE_FollowThreadReq))) return;
   if (direction == -1) fmail = RE_GetQuestion(G->RE[winnum]->Mail.cIRTMsgID);
   if (direction ==  1) fmail = RE_GetAnswer  (G->RE[winnum]->Mail.cMsgID);
   if (fmail)
   {
      struct MailInfo *mi;
      int pos;
      FO_GetFolderByName(fmail->Folder->Name, &pos);
      set(G->MA->GUI.NL_FOLDERS, MUIA_NList_Active, pos);
      mi = GetMailInfo(fmail);
      set(G->MA->GUI.NL_MAILS, MUIA_NList_Active, mi->Pos);
      RE_ReadMessage(winnum, fmail);
   }
   else DisplayBeep(0);
}
MakeHook(RE_FollowHook, RE_Follow);
///
/// RE_SwitchMessage
//  Goes to next or previous (new) message in list
void RE_SwitchMessage(int winnum, int direction, BOOL onlynew)
{
   extern struct Hook RE_CloseHook;
   struct Mail *mail = G->RE[winnum]->MailPtr;
   struct MailInfo *mi = GetMailInfo(mail);
   int act = mi->Pos;
   struct Folder *CurrentFolder = mail->Folder;

   G->RE[winnum]->LastDirection = direction;
   MA_ChangeFolder(CurrentFolder);
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

   if (onlynew)
   {
      struct Folder **flist;

      // only look for following folders if moving forwards
      if (direction == -1)
      {
         DisplayBeep(NULL);
         return;
      }

      if (flist = FO_CreateList())
      {
         int i;

         // look for next folder after the current one
         for (i = 1; i <= (int)*flist; i++)
         {
            if (flist[i] == CurrentFolder)
            {
               i++;
               break;
            }
         }

         // look for first folder with at least one unread mail
         // and if found read that mail
         for ( ; i <= (int)*flist; i++)
         {
            if (flist[i]->Type != FT_SEPARATOR && flist[i]->Unread > 0)
            {
               if (!MUI_Request(G->App, G->RE[winnum]->GUI.WI, 0, GetStr(MSG_MA_ConfirmReq), GetStr(MSG_YesNoReq), GetStr(MSG_RE_MoveNextFolderReq), flist[i]->Name))
                  break;

               MA_ChangeFolder(flist[i]);
               DoMethod(G->MA->GUI.NL_MAILS, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &mail);
               if (!mail) break;
               RE_ReadMessage(winnum, mail);
               break;
            }
         }

         // beep if no folder with unread mails was found
         if (i > (int)*flist)
            DisplayBeep(NULL);

         free(flist);
      }
   }
   else DoMethod(G->App, MUIM_CallHook, &RE_CloseHook, winnum);
}
///
/// RE_PrevNext
//  Goes to next or previous (new) message in list
SAVEDS ASM void RE_PrevNext(REG(a1,int *arg))
{  
   BOOL onlynew = arg[1] & (IEQUALIFIER_LSHIFT|IEQUALIFIER_RSHIFT);
   if (arg[3]) return; // Toolbar qualifier bug work-around
   RE_SwitchMessage(arg[2], arg[0], onlynew);
}
MakeHook(RE_PrevNextHook, RE_PrevNext);
///
/// RE_PrevNextPageFunc
//  Flips one page back or forth
SAVEDS ASM void RE_PrevNextPageFunc(REG(a1,int *arg))
{
   int direct = arg[0], winnum = arg[1], visible;
   struct RE_GUIData *gui = &G->RE[winnum]->GUI;
   get(gui->SL_TEXT, MUIA_Prop_Visible, &visible);
   DoMethod(gui->SL_TEXT, MUIM_Numeric_Increase, visible*direct);
}
MakeHook(RE_PrevNextPageHook, RE_PrevNextPageFunc);
///
/// RE_UpdateDisplay
//  Updates message display after deleting/moving the current message
void RE_UpdateDisplay(int pos, int winnum)
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
void RE_UpdateStatusGroup(int winnum)
{
   struct RE_ClassData *re = G->RE[winnum];
   struct RE_GUIData *gui = &re->GUI;
   struct Mail *mail = re->MailPtr;

   set(gui->GR_STATUS[0], MUIA_Group_ActivePage, 1+mail->Status);
   set(gui->GR_STATUS[1], MUIA_Group_ActivePage, re->PGPEncrypted ? 1 : 0);
   set(gui->GR_STATUS[2], MUIA_Group_ActivePage, re->PGPSigned ? 1 : 0);
}
///
/// RE_SendMDN
//  Creates a message disposition notification
void RE_SendMDN(int MDNtype, struct Mail *mail, struct Person *recipient, BOOL sendnow)
{
   static char *MDNMessage[5] =
   {
      "The message written on %s to %s with subject \"%s\" has been displayed. This is no guarantee that the content has been read or understood.\n",
      "The message written on %s to %s with subject \"%s\" has been sent somewhere %s, without being displayed to the user. The user may or may not see the message later.\n",
      "The message written on %s to %s with subject \"%s\" has been processed %s, without being displayed to the user. The user may or may not see the message later.\n",
      "The message written on %s to %s with subject \"%s\" has been deleted %s. The recipient may or may not have seen the message. The recipient may \"undelete\" the message at a later time and read the message.\n",
      "%s doesn't wish to inform you about the disposition of your message written on %s with subject \"%s\".\n"
   };
   struct WritePart *p1 = NewPart(2), *p2, *p3;
   struct TempFile *tf1, *tf2, *tf3;
   char buf[SIZE_LINE], disp[SIZE_DEFAULT], *mode;
   struct Compose comp;

   if (tf1 = OpenTempFile("w"))
   {
      char *date = DateStamp2String(&mail->Date, DSS_DATETIME), *rcpt = BuildAddrName2(&mail->To), *subj = mail->Subject;
      p1->Filename = tf1->Filename;
      mode = (MDNtype&MDN_AUTOACT) ? "automatically" : "in response to a user command";
      strcpy(disp, (MDNtype&MDN_AUTOACT) ? "automatic-action/" : "manual-action/");
      strcat(disp, (MDNtype&MDN_AUTOSEND) ? "MDN-sent-automatically; " : "MDN-sent-manually; ");
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
      if (tf2 = OpenTempFile("w"))
      {
         char mfile[SIZE_MFILE];
         struct Folder *outfolder = FO_GetFolderByType(FT_OUTGOING, NULL);
         struct ExtendedMail *email = MA_ExamineMail(mail->Folder, mail->MailFile, "", TRUE);
         p2->ContentType = "message/disposition-notification";
         p2->Filename = tf2->Filename;
         sprintf(buf, "%s (YAM %s)", C->SMTP_Domain, __YAM_VERSION);
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
         if (tf3 = OpenTempFile("w"))
         {
            char fullfile[SIZE_PATHFILE];
            FILE *fh;
            p3->ContentType = "text/rfc822-headers";
            p3->Filename = tf3->Filename;
            if (StartUnpack(GetMailFile(NULL, mail->Folder, mail), fullfile, mail->Folder))
            {
               if (fh = fopen(fullfile, "r"))
               {
                  while (fgets(buf, SIZE_LINE, fh)) if (*buf == '\n') break; else fputs(buf, tf3->FP);
                  fclose(fh);
               }
               FinishUnpack(fullfile);
            }
            fclose(tf3->FP); tf3->FP = NULL;
            clear(&comp, sizeof(struct Compose));
            comp.MailTo = StrBufCpy(comp.MailTo, BuildAddrName2(recipient));
            comp.Subject = "Disposition Notification";
            comp.ReportType = 1;
            comp.FirstPart = p1;
            if (comp.FH = fopen(MA_NewMailFile(outfolder, mfile, 0), "w"))
            {
               struct Mail *mlist[3];
               mlist[0] = (struct Mail *)1; mlist[2] = NULL;
               WriteOutMessage(&comp);
               fclose(comp.FH);
               if (email = MA_ExamineMail(outfolder, mfile, Status[STATUS_WFS], TRUE))
               {
                  mlist[2] = AddMailToList((struct Mail *)email, outfolder);
                  MA_FreeEMailStruct(email);
               }
               if (sendnow && mlist[2] && !G->TR) MA_SendMList(mlist);
            }
            else ER_NewError(GetStr(MSG_ER_CreateMailError), NULL, NULL);
            FreeStrBuf(comp.MailTo);
            CloseTempFile(tf3);
         }
         MA_FreeEMailStruct(email);
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
      if (*email->ReceiptTo.Address)
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
            case 3: if (MDNtype != MDN_IGNORE) MDNtype |= MDN_AUTOSEND; break;
         }
         if (MDNtype != MDN_IGNORE) RE_SendMDN(MDNtype, mail, &email->ReceiptTo, sendnow);
      }
      MA_FreeEMailStruct(email);
   }
   return ignoreall;
}
///
/// RE_ReadMessage
//  Displays a message in the read window
void RE_ReadMessage(int winnum, struct Mail *mail)
{
   extern struct Hook RE_CheckSignatureHook;
   struct MailInfo *mi = GetMailInfo(mail);
   struct RE_ClassData *re = G->RE[winnum];
   struct RE_GUIData *gui = &re->GUI;
   int i;
   struct Folder **flist, *folder = mail->Folder;
   BOOL real = !Virtual(mail);
   BOOL out = real ? OUTGOING(folder->Type) : FALSE, allloaded = TRUE;

   re->Mail = *mail;
   re->MailPtr = mail;
   re->PGPKey = FALSE;
   re->PGPSigned = re->PGPEncrypted = 0;
   sprintf(re->WTitle, "%s %s %s: ", mail->MailFile, out ? GetStr(MSG_To) : GetStr(MSG_From), out ? AddrName(mail->To) : AddrName(mail->From));
   stccat(re->WTitle, mail->Subject, SIZE_DEFAULT);
   set(gui->WI, MUIA_Window_Title, re->WTitle);
   set(gui->MI_EDIT, MUIA_Menuitem_Enabled, out);
   DoMethod(gui->TE_TEXT, MUIM_TextEditor_ClearText);
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
      if (flist = FO_CreateList())
      {
         for (i = 1; i <= (int)*flist; i++) if (flist[i]->LoadedMode != 2 && flist[i]->Type != FT_SEPARATOR) allloaded = FALSE;
         free(flist);
      }
      if (allloaded && gui->TO_TOOLBAR)
      {
         DoMethod(gui->TO_TOOLBAR, MUIM_Toolbar_Set, 2, MUIV_Toolbar_Set_Ghosted, !RE_GetQuestion(mail->cIRTMsgID));
         DoMethod(gui->TO_TOOLBAR, MUIM_Toolbar_Set, 3, MUIV_Toolbar_Set_Ghosted, !RE_GetAnswer(mail->cMsgID));
      }
   }
   else if (gui->TO_TOOLBAR)
   {
      DoMethod(gui->TO_TOOLBAR, MUIM_Toolbar_Set, 2, MUIV_Toolbar_Set_Ghosted, TRUE);
      DoMethod(gui->TO_TOOLBAR, MUIM_Toolbar_Set, 3, MUIV_Toolbar_Set_Ghosted, TRUE);
   }
   GetMailFile(G->RE[winnum]->File, folder, mail);
   if (RE_LoadMessage(winnum, PM_ALL))
   {
      RE_DisplayMessage(winnum);
      set(gui->MI_EXTKEY, MUIA_Menuitem_Enabled, re->PGPKey);
      set(gui->MI_CHKSIG, MUIA_Menuitem_Enabled, re->PGPSigned > 0);
      set(gui->MI_SAVEDEC, MUIA_Menuitem_Enabled, real && (re->PGPEncrypted&PGPE_MIME) > 0);
      RE_UpdateStatusGroup(winnum);
      MA_StartMacro(MACRO_READ, itoa(winnum));
      if (real && (mail->Status == STATUS_NEW || mail->Status == STATUS_UNR))
      {
         MA_SetMailStatus(mail, STATUS_OLD);
         DisplayStatistics(folder);
         if (re->PGPSigned) DoMethod(G->App, MUIM_CallHook, &RE_CheckSignatureHook, FALSE, winnum);
         RE_DoMDN(MDN_READ, mail, FALSE);
      }
   }
   else
   {
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

   if (G->RE[winnum]->Header)
   {
      int i;
      fputs("\033[3m", fh);
      for (i = 0; ; i++)
      {
         DoMethod(G->RE[winnum]->GUI.LV_HEAD, MUIM_NList_GetEntry, i, &ptr);
         if (!ptr) break;
         if (!strcmp(ptr, MUIX_I)) ptr += strlen(MUIX_I);
         fputs(ptr, fh); fputc('\n', fh);
      }
      fputs("\033[23m\n", fh);
   }
   for (ptr = (char *)DoMethod(G->RE[winnum]->GUI.TE_TEXT, MUIM_TextEditor_ExportText); *ptr; ptr++)
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
///
/// RE_SuggestName
//  Suggests a file name based on the message subject
char *RE_SuggestName(struct Mail *mail)
{
   static char name[SIZE_FILE];
   char *ptr = mail->Subject;
   int i = 0;

   clear(name, SIZE_FILE);
   while (*ptr && i < 26)
   {
      if ((int)*ptr <= ' ') name[i++] = '_';
      else if (*ptr != ':' && *ptr != '/') name[i++] = *ptr;
      ptr++;
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
      else if (nr) sprintf(buffer2, "%s-%ld", G->RE[winnum]->Mail.MailFile, nr);
      else strcpy(buffer2, RE_SuggestName(&(G->RE[winnum]->Mail)));
      if (force) strmfp(dest = buffer, C->DetachDir, buffer2);
      else if (ReqFile(ASL_DETACH, win, GetStr(MSG_RE_SaveMessage), 1, C->DetachDir, buffer2))
         strmfp(dest = buffer, G->ASLReq[ASL_DETACH]->fr_Drawer, G->ASLReq[ASL_DETACH]->fr_File);
      else return FALSE;
   }
   if (FileExists(dest) && !overwrite)
   {
      sprintf(buffer2, GetStr(MSG_RE_Overwrite), FilePart(dest));
      if (!MUI_Request(G->App, win, 0, GetStr(MSG_MA_ConfirmReq), GetStr(MSG_OkayCancelReq), buffer2)) return FALSE;
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
SAVEDS ASM void RE_MoveFunc(REG(a1,int *arg))
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
MakeHook(RE_MoveHook, RE_MoveFunc);
///
/// RE_CopyFunc
//  Copies the current message to another folder
SAVEDS ASM void RE_CopyFunc(REG(a1,int *arg))
{
   int winnum = *arg;
   struct Folder *srcfolder = G->RE[winnum]->Mail.Folder;
   struct Mail *mail = G->RE[winnum]->MailPtr;
   if (MailExists(mail, srcfolder))
   {
      struct Folder *dstfolder = FolderRequest(GetStr(MSG_MA_CopyMsg), GetStr(MSG_MA_MoveMsgReq), GetStr(MSG_MA_CopyGad), GetStr(MSG_Cancel), NULL, G->RE[winnum]->GUI.WI);
      if (dstfolder)
         if (srcfolder)
         {
            MA_MoveCopy(mail, srcfolder, dstfolder, TRUE);
            AppendLogNormal(24, GetStr(MSG_LOG_Copying), (void *)1, srcfolder->Name, dstfolder->Name, "");
         }
         else if (RE_Export(winnum, G->RE[winnum]->File, MA_NewMailFile(dstfolder, mail->MailFile, 0), "", 0, FALSE, FALSE, ContType[CT_ME_EMAIL]))
         {
            APTR lv;
            struct Mail *newmail = AddMailToList(mail, dstfolder);
            if (lv = WhichLV(dstfolder)) DoMethod(lv, MUIM_NList_InsertSingle, newmail, MUIV_NList_Insert_Sorted);
            MA_SetMailStatus(newmail, STATUS_OLD);
         }
   }
}
MakeHook(RE_CopyHook, RE_CopyFunc);
///
/// RE_DeleteFunc
//  Deletes the current message
SAVEDS ASM void RE_DeleteFunc(REG(a1,int *arg))
{
   BOOL delatonce = arg[0] & (IEQUALIFIER_LSHIFT|IEQUALIFIER_RSHIFT);
   int pos, winnum = arg[1];
   struct Folder *folder = G->RE[winnum]->Mail.Folder, *delfolder = FO_GetFolderByType(FT_DELETED, NULL);
   struct Mail *mail = G->RE[winnum]->MailPtr;
   if (arg[2]) return; // Toolbar qualifier bug work-around
   if (MailExists(mail, folder)) if ((pos = SelectMessage(mail)) >= 0)
   {
      MA_DeleteSingle(G->RE[winnum]->MailPtr, delatonce);
      RE_UpdateDisplay(pos, winnum);
      if (delatonce) AppendLogNormal(20, GetStr(MSG_LOG_Deleting), (void *)1, folder->Name, "", "");
      else           AppendLogNormal(22, GetStr(MSG_LOG_Moving), (void *)1, folder->Name, delfolder->Name, "");
   }
}
MakeHook(RE_DeleteHook, RE_DeleteFunc);
///
/// RE_PrintFunc
//  Sends the current message or an attachment to the printer
SAVEDS ASM void RE_PrintFunc(REG(a1,int *arg))
{
   int winnum = *arg;
   struct Part *part;
   struct TempFile *prttmp;

   if (part = AttachRequest(GetStr(MSG_RE_PrintMsg), GetStr(MSG_RE_SelectPrintPart), GetStr(MSG_RE_PrintGad), GetStr(MSG_Cancel), winnum, ATTREQ_PRINT|ATTREQ_MULTI, G->RE[winnum]->GUI.WI))
	   {
      if (C->PrinterCheck) if (!CheckPrinter()) return;
      Busy(GetStr(MSG_BusyDecPrinting), "", 0, 0);
      for (; part; part = part->NextSelected) switch (part->Nr)
      {
         case -2: RE_PrintFile(G->RE[winnum]->File,part);
                  break;
         case -1: if (prttmp = OpenTempFile("w"))
                  {
                     RE_SaveDisplay(winnum, prttmp->FP);
							fclose(prttmp->FP);
							prttmp->FP = NULL;
							RE_PrintFile(prttmp->Filename,part);
                     CloseTempFile(prttmp);
                  }
                  break;
         default: RE_PrintFile(part->Filename,part);
      }
      BusyEnd;
   }
}
MakeHook(RE_PrintHook, RE_PrintFunc);

///
/// RE_PrintFile
//  Prints a file. Currently it is just dumped to PRT:
//  To do for LaTeX printing:
//  - remap characters in header to LaTeX notation
//  - add options for latex/dviprint call
//  - make header lines to print (and parts where to print headers) configurable
LOCAL void RE_PrintFile(char *filename, struct Part *part)
{
	if(1)
	{
		CopyFile("PRT:", 0, filename, 0);
	} else
	{
	struct TempFile *texfile;
	
		if((texfile = OpenTempFile("w")))
		{
			if(CopyFile(NULL,texfile->FP,"YAM:.texheader",NULL))
			{
			char *ts1,*ts2;

				if((ts1 = AllocStrBuf(SIZE_LINE)) && (ts2 = AllocStrBuf(SIZE_LINE)))
				{
					if(1 == part->Nr)
					{
					int i,j;
					char Attrib[SIZE_DEFAULT];
					char *p;

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
							} else KPrintF("RE_PrintFile(): strange header line %s\n",p);
						}
						fprintf(texfile->FP,"\n%s\n%s\n%s\n%s\n",
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
									"\\bigskip\n"
									"\\input{Texts:TeXdocs/Experimental/email.text}\n"
									"\\end{document}\n");
						fclose(texfile->FP);
						texfile->FP = NULL;
						system("latex");
						
					} else
					{
						KPrintF("RE_PrintFile(): no headers for this part\n");
					}
				}
				if(ts1) FreeStrBuf(ts1);
				if(ts2) FreeStrBuf(ts2);
			} else KPrintF("RE_PrintFile(): can't copy YAM:.texheader to temp TeX file\n");
			CloseTempFile(texfile);
		} else KPrintF("RE_PrintFile(): can't open temp TeX file\n");
	}
}

///
// ISO8859_to_LaTeX
// Takes a string in ISO-8859 charset and converts it to a equivalent
// string in LaTeX notation. Free the result with FreeStrBuf() after use
LOCAL char *ISO8859_to_LaTeX(char *s)
{
char *result=NULL;
char **CVTab;

	if(CVTab = Init_ISO8859_to_LaTeX_Tab("YAM:.latex-chartab"))
	{
	int ResLen;
	char *p;

		for(p=s,ResLen=0; *p; p++)  // pre-calculate resulting string's length
			ResLen += (CVTab[*p] == NULL ? 1 : strlen(CVTab[*p]));

		KPrintF("ISO8859_to_LaTeX(): source=%ld result=%ld\n",strlen(s),ResLen);

		if(result = AllocStrBuf(ResLen+1))
		{
		char *q = result;
			for(p=s,ResLen=0; *p; p++)	// map input string
			{
				if(CVTab[*p] == NULL)
					*q++ = *p;
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
// Init_ISO8859_to_LaTeX_Tab
// Takes a filename for a ISO->LaTeX mapping table and returns a table for
// mapping ISO/ASCII codes to strings
LOCAL char **Init_ISO8859_to_LaTeX_Tab(char *TabFileName)
{
int TabSize;
char **CVTab, *TabFile;
BOOL success=FALSE;

	if(-1 != (TabSize = FileSize(TabFileName)))
	{
	BPTR fh;
		if(fh = Open(TabFileName,MODE_OLDFILE))
		{
			if(CVTab = AllocVec(TabSize+1+256*sizeof(char*),MEMF_ANY | MEMF_CLEAR))
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
							if(tok[1]) KPrintF("Init_ISO8859_to_LaTeX_tab(): line format is %%c %%s\n");
							else c = tok[0];
						} else
						{
							CVTab[c] = tok;
							KPrintF("LaTeX mapping: '%c' -> '%s'\n",c,tok);
							c = '\0';
						}
					}
					success = TRUE;
				}
				if(!success)
				{
					FreeMem(CVTab,TabSize);
					CVTab = NULL;
				}
			}
			Close(fh);
		}
	}
	return CVTab;
}

///
/// RE_SaveFunc
//  Saves the current message or an attachment to disk
SAVEDS ASM void RE_SaveFunc(REG(a1,int *arg))
{
   int winnum = *arg;
   struct Part *part;
   struct TempFile *tf;

   if (part = AttachRequest(GetStr(MSG_RE_SaveMessage), GetStr(MSG_RE_SelectSavePart), GetStr(MSG_RE_SaveGad), GetStr(MSG_Cancel), winnum, ATTREQ_SAVE|ATTREQ_MULTI, G->RE[winnum]->GUI.WI))
   {
      Busy(GetStr(MSG_BusyDecSaving), "", 0, 0);
      for (; part; part = part->NextSelected) switch (part->Nr)
      {
         case -2: RE_Export(winnum, G->RE[winnum]->File, "", "", 0, FALSE, FALSE, ContType[CT_ME_EMAIL]);
                  break;
         case -1: if (tf = OpenTempFile("w"))
                  {
                     RE_SaveDisplay(winnum, tf->FP);
                     fclose(tf->FP); tf->FP = NULL;
                     RE_Export(winnum, tf->Filename, "", "", 0, FALSE, FALSE, ContType[CT_TX_PLAIN]);
                     CloseTempFile(tf);
                  }
                  break;
         default: RE_DecodePart(part);
                  RE_Export(winnum, part->Filename, "", part->Name, part->Nr, FALSE, FALSE, part->ContentType);
      }
      BusyEnd;
   }
}
MakeHook(RE_SaveHook, RE_SaveFunc);
///
/// RE_DisplayMIME
//  Displays a message part (attachment) using a MIME viewer
void RE_DisplayMIME(char *fname, char *ctype)
{
   static char command[SIZE_COMMAND+SIZE_PATHFILE];
   int i;
   struct MimeView *mv = NULL;

   for (i = 1; i < MAXMV; i++) if (C->MV[i])
      if (MatchNoCase(ctype, C->MV[i]->ContentType)) { mv = C->MV[i]; break; }
   if (!mv && !stricmp(ctype, "message/rfc822"))
   {
      int winnum;
      struct Mail *mail;
      struct ExtendedMail *email;
      struct TempFile *tf = OpenTempFile(NULL);
      CopyFile(tf->Filename, NULL, fname, NULL);
      if (email = MA_ExamineMail(NULL, FilePart(tf->Filename), "O", TRUE))
      {
         mail = malloc(sizeof(struct Mail));
         memcpy(mail, &email->Mail, sizeof(struct Mail));
         mail->Folder = NULL;
         mail->Flags |= MFLAG_NOFOLDER;
         MA_FreeEMailStruct(email);
         if ((winnum = RE_Open(-1, FALSE)) != -1)
         {
            G->RE[winnum]->TempFile = tf;
            if (SafeOpenWindow(G->RE[winnum]->GUI.WI)) RE_ReadMessage(winnum, mail);
            else DisposeModulePush(&G->RE[winnum]);
         }
      }
   }
   else
   {
      if (!mv)
      {
         if (C->IdentifyBin)
         {
            ctype = IdentifyFile(fname);
            for (i = 1; i < MAXMV; i++) if (C->MV[i])
               if (MatchNoCase(ctype, C->MV[i]->ContentType)) { mv = C->MV[i]; break; }
         }
         if (!mv) mv = C->MV[0];
      }
      sprintf(command, mv->Command, fname);
      ExecuteCommand(command, TRUE, OUT_NIL);
   }
}
///
/// RE_DisplayFunc
//  Shows message or attachments separately
SAVEDS ASM void RE_DisplayFunc(REG(a1,int *arg))
{
   int winnum = *arg;
   struct Part *part;

   if (part = AttachRequest(GetStr(MSG_RE_DisplayMsg), GetStr(MSG_RE_SelectDisplayPart), GetStr(MSG_RE_DisplayGad), GetStr(MSG_Cancel), winnum, ATTREQ_DISP|ATTREQ_MULTI, G->RE[winnum]->GUI.WI))
   {
      Busy(GetStr(MSG_BusyDecDisplaying), "", 0, 0);
      for (; part; part = part->NextSelected)
      {
         RE_DecodePart(part);
         if (part->Nr == -2) RE_DisplayMIME(G->RE[winnum]->File, "text/plain");
         else RE_DisplayMIME(part->Filename, part->ContentType);
      }
      BusyEnd;
   }
}
MakeHook(RE_DisplayHook, RE_DisplayFunc);
///
/// RE_SaveAll
//  Saves all attachments to disk
void RE_SaveAll(int winnum, char *path)
{
   struct Part *part;
   char dest[SIZE_PATHFILE], fname[SIZE_FILE];

   for (part = G->RE[winnum]->FirstPart->Next->Next; part; part = part->Next)
   {
      if (*part->Name) stccpy(fname, part->Name, SIZE_FILE);
      else sprintf(fname, "%s-%ld", G->RE[winnum]->Mail.MailFile, part->Nr);
      strmfp(dest, path, fname);
      RE_DecodePart(part);
      RE_Export(winnum, part->Filename, dest, part->Name, part->Nr, FALSE, FALSE, part->ContentType);
   }
}
///
/// RE_SaveAllFunc
//  Asks user for a directory and saves all attachments there
SAVEDS ASM void RE_SaveAllFunc(REG(a1,int *arg))
{
   struct Part *part = G->RE[*arg]->FirstPart->Next;
   if (part) if (part->Next) if (ReqFile(ASL_DETACH, G->RE[*arg]->GUI.WI, GetStr(MSG_RE_SaveMessage), 5, C->DetachDir, ""))
   {
      Busy(GetStr(MSG_BusyDecSaving), "", 0, 0);
      RE_SaveAll(*arg, G->ASLReq[ASL_DETACH]->fr_Drawer);
      BusyEnd;
   }
}
MakeHook(RE_SaveAllHook, RE_SaveAllFunc);
///
/// RE_RemoveAttachFunc
//  Removes attachments from the current message
SAVEDS ASM void RE_RemoveAttachFunc(REG(a1,int *arg))
{
   struct Mail *mail = G->RE[*arg]->MailPtr;
   struct MailInfo *mi;
   MA_RemoveAttach(mail);
   if ((mi = GetMailInfo(mail))->Pos >= 0)
   {
      DoMethod(G->MA->GUI.NL_MAILS, MUIM_NList_Redraw, mi->Pos);
      MA_ChangeSelectedFunc();
      DisplayStatistics(mail->Folder);
   }
   RE_ReadMessage(*arg, mail);
}
MakeHook(RE_RemoveAttachHook, RE_RemoveAttachFunc);
///
/// RE_NewFunc
//  Starts a new message based on the current one
SAVEDS ASM void RE_NewFunc(REG(a1,int *arg))
{
   int mode = arg[0], winnum = arg[2], flags = 0;
   ULONG qual = arg[1];
   struct Mail *mail = G->RE[winnum]->MailPtr, *mlist[3] = { (struct Mail *)1, NULL, NULL };
   if (arg[3]) return; // Toolbar qualifier bug work-around
   if (mode == NEW_FORWARD && qual & (IEQUALIFIER_LSHIFT|IEQUALIFIER_RSHIFT)) mode = NEW_BOUNCE;
   if (mode == NEW_FORWARD && qual & IEQUALIFIER_CONTROL) flags = NEWF_FWD_NOATTACH;
   if (mode == NEW_REPLY && qual & (IEQUALIFIER_LSHIFT|IEQUALIFIER_RSHIFT)) flags = NEWF_REP_PRIVATE;
   if (mode == NEW_REPLY && qual & (IEQUALIFIER_LALT|IEQUALIFIER_RALT)) flags = NEWF_REP_MLIST;
   if (mode == NEW_REPLY && qual & IEQUALIFIER_CONTROL) flags = NEWF_REP_NOQUOTE;
   mlist[2] = mail;
   if (MailExists(mail, NULL)) switch (mode)
   {
      case NEW_NEW:     MA_NewNew(mail, flags); break;
      case NEW_EDIT:    MA_NewEdit(mail, flags); break;
      case NEW_BOUNCE:  MA_NewBounce(mail, flags); break;
      case NEW_FORWARD: MA_NewForward(mlist, flags); break;
      case NEW_REPLY:   MA_NewReply(mlist, flags); break;
   }
}
MakeHook(RE_NewHook, RE_NewFunc);
///
/// RE_GetAddressFunc
//  Stores sender address of current message in the address book
SAVEDS ASM void RE_GetAddressFunc(REG(a1,int *arg))
{
   int winnum = *arg;
   struct Folder *folder = G->RE[winnum]->Mail.Folder;
   struct Mail *mail = G->RE[winnum]->MailPtr, *mlist[3] = { (struct Mail *)1, NULL, NULL };
   mlist[2] = mail;
   if (MailExists(mail, folder)) MA_GetAddress(mlist);
}
MakeHook(RE_GetAddressHook, RE_GetAddressFunc);
///
/// RE_SetUnreadFunc
//  Sets the status of the current mail to unread
SAVEDS ASM void RE_SetUnreadFunc(REG(a1,int *arg))
{
   int winnum = *arg;
   MA_SetMailStatus(G->RE[winnum]->MailPtr, STATUS_UNR);
   RE_UpdateStatusGroup(winnum);
   DisplayStatistics(NULL);
}
MakeHook(RE_SetUnreadHook, RE_SetUnreadFunc);
///
/// RE_ChangeSubjectFunc
//  Changes the subject of the current message
SAVEDS ASM void RE_ChangeSubjectFunc(REG(a1,int *arg))
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
            MA_ChangeSelectedFunc();
            DisplayStatistics(mail->Folder);
         }
         RE_ReadMessage(*arg, mail);
      }
   }
}
MakeHook(RE_ChangeSubjectHook, RE_ChangeSubjectFunc);
///
/// RE_ExtractKeyFunc
//  Extracts public PGP key from the current message
SAVEDS ASM void RE_ExtractKeyFunc(REG(a1,int *arg))
{
   char fullfile[SIZE_PATHFILE], options[SIZE_PATHFILE];
   struct Mail *mail = G->RE[*arg]->MailPtr;

   if (!StartUnpack(GetMailFile(NULL, NULL, mail), fullfile, mail->Folder)) return;
   sprintf(options, (G->PGPVersion == 5) ? "-a %s +batchmode=1 +force" : "-ka %s +bat +f", fullfile);
   PGPCommand((G->PGPVersion == 5) ? "pgpk" : "pgp", options, 0);
   FinishUnpack(fullfile);
}
MakeHook(RE_ExtractKeyHook, RE_ExtractKeyFunc);
///
/// RE_GetAddressFromLog
//  Finds e-mail address in PGP output
BOOL RE_GetAddressFromLog(char *buf, char *address)
{
   if (buf = strchr(buf, 34))
   {
      stccpy(address, ++buf, SIZE_ADDRESS);
      if (buf = strchr(address, 34)) *buf = 0;
      return TRUE;
   }
   return FALSE;
}
///
/// RE_GetSigFromLog
//  Interprets logfile created from the PGP signature check
void RE_GetSigFromLog(int winnum, char *decrFor)
{
   BOOL sigDone = FALSE, decrFail = FALSE;
   struct RE_ClassData *re = G->RE[winnum];
   FILE *fh;
   char buffer[SIZE_LARGE];

   if (fh = fopen(PGPLOGFILE, "r"))
   {
      while (GetLine(fh, buffer, SIZE_LARGE))
      {
         if (!decrFail && decrFor && G->PGPVersion == 5)
            if (!strnicmp(buffer, "cannot decrypt", 14))
            {
               *decrFor = 0;
               GetLine(fh, buffer, SIZE_LARGE); GetLine(fh, buffer, SIZE_LARGE);
               RE_GetAddressFromLog(buffer, decrFor);
               decrFail = TRUE;
            }
         if (!sigDone)
         {
            if (!strnicmp(buffer, "good signature", 14)) sigDone = TRUE;
            if (!strnicmp(buffer, "bad signature", 13)) { re->PGPSigned |= PGPS_BADSIG; sigDone = TRUE; }
            if (sigDone)
            {
               if (G->PGPVersion == 5) { GetLine(fh, buffer, SIZE_LARGE); GetLine(fh, buffer, SIZE_LARGE); }
               if (RE_GetAddressFromLog(buffer, re->Signature)) re->PGPSigned |= PGPS_ADDRESS;
               re->PGPSigned |= PGPS_CHECKED;
            }
         }
      }
      fclose(fh);
      DeleteFile(PGPLOGFILE);
   }
}
///
/// RE_CheckSignatureFunc
//  Checks validity of a PGP signed message
SAVEDS ASM void RE_CheckSignatureFunc(REG(a1,int *arg))
{
   struct RE_ClassData *re = G->RE[arg[1]];

   if ((re->PGPSigned & PGPS_OLD) && !(re->PGPSigned & PGPS_CHECKED))
   {
      int error;
      char fullfile[SIZE_PATHFILE], options[SIZE_LARGE];
      if (!StartUnpack(GetMailFile(NULL, NULL, re->MailPtr), fullfile, re->MailPtr->Folder)) return;
      sprintf(options, (G->PGPVersion == 5) ? "%s -o %s +batchmode=1 +force +language=us" : "%s -o %s +bat +f", fullfile, "T:PGP.tmp");
      error = PGPCommand((G->PGPVersion == 5) ? "pgpv": "pgp", options, NOERRORS|KEEPLOG);
      FinishUnpack(fullfile);
      DeleteFile("T:PGP.tmp");
      if (error > 0) re->PGPSigned |= PGPS_BADSIG;
      if (error >= 0) RE_GetSigFromLog(arg[1], NULL); else return;
   }
   if ((re->PGPSigned & PGPS_BADSIG) || arg[0])
   {
      char buffer[SIZE_LARGE];
      strcpy(buffer, (re->PGPSigned & PGPS_BADSIG) ? GetStr(MSG_RE_BadSig) : GetStr(MSG_RE_GoodSig));
      if (re->PGPSigned & PGPS_ADDRESS) { strcat(buffer, GetStr(MSG_RE_SigFrom)); strcat(buffer, re->Signature); }
      MUI_Request(G->App, re->GUI.WI, 0, GetStr(MSG_RE_SigCheck), GetStr(MSG_Okay), buffer);
   }
}
MakeHook(RE_CheckSignatureHook, RE_CheckSignatureFunc);
///
/// RE_SaveDecryptedFunc
//  Saves decrypted version of a PGP message
SAVEDS ASM void RE_SaveDecryptedFunc(REG(a1,int *arg))
{
   struct RE_ClassData *re = G->RE[*arg];
   struct WritePart *p1;
   struct Compose comp;
   int choice;
   struct Folder *folder = re->MailPtr->Folder;
   char mfile[SIZE_MFILE];

   if (!(choice = MUI_Request(G->App, re->GUI.WI, 0, GetStr(MSG_RE_SaveDecrypted), GetStr(MSG_RE_SaveDecGads), GetStr(MSG_RE_SaveDecReq)))) return;
   clear(&comp, sizeof(struct Compose));
   if (comp.FH = fopen(MA_NewMailFile(folder, mfile, 0), "w"))
   {
      struct ExtendedMail *email;
      struct Mail *new;
      comp.Mode = NEW_SAVEDEC;
      comp.OrigMail = re->MailPtr;
      comp.FirstPart = p1 = NewPart(2);
      p1->Filename = re->FirstPart->Next->Filename;
      WriteOutMessage(&comp);
      FreePartsList(p1);
      fclose(comp.FH);
      if (email = MA_ExamineMail(folder, mfile, Status[re->MailPtr->Status], TRUE))
      {
         new = AddMailToList((struct Mail *)email, folder);
         if (FO_GetCurrentFolder() == folder) DoMethod(G->MA->GUI.NL_MAILS, MUIM_NList_InsertSingle, new, MUIV_NList_Insert_Sorted);
         MA_FreeEMailStruct(email);
         if (choice == 2)
         {
            MA_DeleteSingle(re->MailPtr, FALSE);
            RE_ReadMessage(*arg, new);
         }
      }
      else ER_NewError(GetStr(MSG_ER_CreateMailError), NULL, NULL);
   }
}
MakeHook(RE_SaveDecryptedHook, RE_SaveDecryptedFunc);
///

/*** MIME ***/
/// StripTrailingSpace
//  Strips trailing spaces from a string
void StripTrailingSpace(char *s)
{
   char *t = &s[strlen(s)-1];
   while (ISpace(*t) && t >= s) *t-- = 0;
}
///
/// ParamEnd
//  Finds next parameter in header field
char *ParamEnd(char *s)
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
char *Cleanse(char *s)
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
char *UnquoteString(char *s, BOOL new)
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
/// RE_CharIn
//  Converts character using translation table
int RE_CharIn(char c, struct TranslationTable *tt)
{
   if (tt) if (tt->Header) return (int)tt->Table[(UBYTE)c];
   return (int)c;
}
///
/// RE_ProcessHeader (rec)
//  Processes MIME encoded message headers (RFC-2047)
STACKEXT void RE_ProcessHeader(char *prevcharset, char *s, BOOL ShowLeadingWhitespace, char *ptr)
{
   char *charset, *encoding, *txt, *txtend, *t;
   int ecode = ENC_NONE, CorrectedCharset = 0;
   struct TranslationTable *tt = NULL;

   if (MatchTT(prevcharset, G->TTin, TRUE)) tt = G->TTin;
   while (*s && (*s != '='))
   {
      if (*s == ' ' || *s == '\t') { if (ShowLeadingWhitespace) *ptr++ = ' '; }
      else 
      {
         if (!CorrectedCharset) { CorrectedCharset = TRUE; strcpy(prevcharset, "us-ascii"); }
         *ptr++ = (char)RE_CharIn(*s, tt);
      }
      if (!ShowLeadingWhitespace) ShowLeadingWhitespace = TRUE;
      ++s;
   }
   if (!*s) return;
   if (*(s+1) != '?') 
   {
      *ptr++ = '=';
      RE_ProcessHeader(prevcharset, ++s, TRUE, ptr);
      return;
   }
   charset = s+2;
   encoding = strchr(charset, '?');
   if (!encoding) { *ptr++ = '='; RE_ProcessHeader(prevcharset, ++s, TRUE, ptr); return; }
   txt = strchr(encoding+1, '?');
   if (!txt) { *ptr++ = '='; RE_ProcessHeader(prevcharset, ++s, TRUE, ptr); return; }
   txtend = txt;
   do { txtend = strchr(txtend+1, '?'); } while(txtend && (*(txtend+1) != '='));
   if (!txtend) {
      *ptr++ = '=';
      RE_ProcessHeader(prevcharset, ++s, TRUE, ptr);
      return;
   }
   *encoding = 0;
   *txt = 0;
   *txtend = 0;
   if (tolower((int)*(encoding+1)) == 'q') ecode = ENC_QP;
   else if (tolower((int)*(encoding+1)) == 'b') ecode = ENC_B64;
   else ER_NewError(GetStr(MSG_ER_UnknownHeaderEnc), encoding+1, NULL);
   if (stricmp(charset, prevcharset))
   {
      char *s2;
      strcpy(prevcharset, charset);
      for (s2 = prevcharset; *s2; ++s2)
         if (isupper((int)*s2)) *s2 = tolower((int)*s2);
      if (MatchTT(prevcharset, G->TTin, TRUE)) tt = G->TTin;
   }
   if (ecode == ENC_NONE) for (t = txt+1; *t; ++t) *ptr++ = RE_CharIn(*t, tt);
   else 
   {
      for (t = txt+1; *t; ++t) if (*t == '_') *t = ' ';
      if (ecode == ENC_B64) from64txt(txt+1, ptr, tt);
      else if (ecode == ENC_QP) fromqptxt(txt+1, ptr, tt);
      while (*ptr) ptr++;
   }
   *encoding = '?'; *txt = '?'; *txtend = '?';
   RE_ProcessHeader(prevcharset, txtend+2, TRUE, ptr);
}
///
/// RE_ParseContentParameters
//  Parses parameters of Content-Type header field
void RE_ParseContentParameters(struct Part *rp)
{
   char *s, *t, *eq, *ct = rp->ContentType;

   s = strchr(ct, ';');
   if (!s) return;
   *s++ = 0;
   do {
      if (t = ParamEnd(s)) *t++ = 0;
      if (!(eq = strchr(s, '='))) rp->JunkParameter = Cleanse(s);
      else 
      {
         *eq++ = 0;
         s = Cleanse(s); eq = stpblk(eq);
         StripTrailingSpace(eq);
         UnquoteString(eq, FALSE);
         if (!stricmp(s, "name")) rp->CParName = eq;
         if (!stricmp(s, "description")) rp->CParDesc = eq;
         if (!stricmp(s, "boundary")) rp->CParBndr = eq;
         if (!stricmp(s, "protocol")) rp->CParProt = eq;
         if (!stricmp(s, "report-type")) rp->CParRType = eq;
         if (!stricmp(s, "charset")) rp->CParCSet = eq;
      }
      s = t;
   } while (t);
}
///
/// RE_ScanHeader
//  Parses the header of the message or of a message part
BOOL RE_ScanHeader(struct Part *rp, FILE *in, FILE *out, int mode)
{
   int i;
   char *p;

   if (!MA_ReadHeader(in))
   {
      if (mode == 0) ER_NewError(GetStr(MSG_ER_MIMEError), NULL, NULL);
      else if (mode == 1) ER_NewError(GetStr(MSG_ER_MultipartEOF), NULL, NULL);
      return FALSE;
   }
   rp->HasHeaders = TRUE;
   for (i = 0; i < Header.Used; i++)
   {
      char *s = Header.Data[i];
      int ls = strlen(s);
      rp->MaxHeaderLen = MAX(ls, rp->MaxHeaderLen);
      if (out) { fputs(s, out); fputc('\n', out); }
      if (!strnicmp(s, "content-type:", 13))
      {
         rp->ContentType = StrBufCpy(rp->ContentType, p = stpblk(&s[13]));
         while (TRUE) 
         {
            if (!(p = strchr(rp->ContentType, '/'))) break;
            if (ISpace(*(p-1)))    for (--p; *p; ++p) *p = *(p+1);
            else if (ISpace(*++p)) for ( ; *p; ++p) *p = *(p+1);
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
         if      (!stricmp(p, "base64"))           rp->EncodingCode = ENC_B64;
         else if (!stricmp(p, "quoted-printable")) rp->EncodingCode = ENC_QP;
         else if (!strnicmp(p, "x-uue", 5))        rp->EncodingCode = ENC_UUE;
         else if (stricmp(p, "none") && !stricmp(p, "8bit") && !stricmp(p, "7bit"))
            ER_NewError(GetStr(MSG_ER_UnknownEnc), p, NULL);
      } 
      else if (!strnicmp(s, "content-description:", 20))
      {
         stccpy(rp->Description, stpblk(&s[20]), SIZE_DEFAULT);
      } 
   }
   for (p = rp->ContentType; *p; ++p) if (isupper((int)*p)) *p = tolower((int)*p);
   return TRUE;
}
///
/// RE_ConsumeRestOfPart
//  Processes body of a message part
BOOL RE_ConsumeRestOfPart(FILE *in, FILE *out, struct TranslationTable *tt, struct Part *rp)
{
   char *ptr, c = 0, buf[SIZE_LINE];
   UBYTE *p;
   int blen = 0;

   if (rp) blen = strlen(rp->Boundary);
   while (fgets(buf, SIZE_LINE, in))
   {
      if (rp) if (!strncmp(buf, rp->Boundary, blen))
      {
         if (buf[blen] == '\n') return FALSE;
         if (buf[blen] == '-' && buf[blen+1] == '-' && buf[blen+2] == '\n') return TRUE;
      }
      if (out)
      {
         if (c == '\n') fputc(c, out);
         ptr = &buf[strlen(buf)-1];
         if ((c = *ptr) == '\n') *ptr = 0;
         if (tt) for (p = buf; *p; ++p) *p = tt->Table[*p];
         fputs(buf, out);
      }
   }
   if (out && c == '\n') fputc(c, out);
   return TRUE;
}
///
/// RE_DecodeStream
//  Decodes contents of a part
void RE_DecodeStream(struct Part *rp, FILE *in, FILE *out)
{
   struct TranslationTable *tt = NULL;
   if (rp->Nr == C->LetterPart && rp->Printable)
      if (!rp->CParCSet)
      {
         if (MatchTT(C->LocalCharset, G->TTin, TRUE) || MatchTT("us-ascii", G->TTin, TRUE)) tt = G->TTin;
      }
      else if (MatchTT(rp->CParCSet, G->TTin, TRUE)) tt = G->TTin;
   switch (rp->EncodingCode)
   {
      case ENC_B64:  from64  (in, out, tt, DoesNeedPortableNewlines(rp->ContentType)); break;
      case ENC_QP:   fromqp  (in, out, tt); break;
      case ENC_FORM: fromform(in, out, tt); break;
      case ENC_UUE:  fromuue (in, out); RE_ConsumeRestOfPart(in, NULL, NULL, NULL); break;
      default:       RE_ConsumeRestOfPart(in, out, tt, NULL);
   }
}
///
/// RE_OpenNewPart
//  Adds a new entry to the message part list
FILE *RE_OpenNewPart(int winnum, struct Part **new, struct Part *prev, struct Part *first)
{
   FILE *fp;
   if ((*new) = calloc(1,sizeof(struct Part)))
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
      sprintf(file, "YAMraw-w%ldp%ld.txt", winnum, (*new)->Nr);
      strmfp((*new)->Filename, C->TempDir, file);
      if (fp = fopen((*new)->Filename, "w")) return fp;
      free(*new);
   }
   return NULL;
}
///
/// RE_UndoPart
//  Removes an entry from the message part list
void RE_UndoPart(struct Part *rp)
{
   DeleteFile(rp->Filename);
   if (rp->Prev) rp->Prev->Next = rp->Next;
   if (rp->Next) rp->Next->Prev = rp->Prev;
   if (rp->ContentType) FreeStrBuf(rp->ContentType);
   free(rp);
}
///
/// RE_RequiresSpecialHandling
//  Checks if part is PGP signed/encrypted or a MDN
int RE_RequiresSpecialHandling(struct Part *hrp)
{
   if (!stricmp(hrp->ContentType, "multipart/report") && !stricmp(hrp->CParRType, "disposition-notification")) return 1;
   if (!stricmp(hrp->ContentType, "multipart/signed") && !stricmp(hrp->CParProt, "application/pgp-signature")) return 2;
   if (!stricmp(hrp->ContentType, "multipart/encrypted") && !stricmp(hrp->CParProt, "application/pgp-encrypted")) return 3;
   return 0;
}
///
/// RE_IsURLencoded
//  Checks if part contains encoded form data
BOOL RE_IsURLencoded(struct Part *rp)
{
   return (BOOL)(!stricmp(rp->ContentType, "application/x-www-form-urlencoded") ||
                 !stricmp(rp->ContentType, "application/x-url-encoded"));
}
///
/// RE_SaveThisPart
//  Decides if the part should be kept in memory
BOOL RE_SaveThisPart(struct Part *rp)
{
   int pm = G->RE[rp->Win]->ParseMode;
   switch (pm)
   {
      case PM_ALL:   return TRUE;
      case PM_NONE:  return FALSE;
      case PM_TEXTS: return (BOOL)(!strnicmp(rp->ContentType, "text", 4) || RE_IsURLencoded(rp));
   }
}
///
/// RE_SetPartInfo
//  Determines size and other information of a message part
void RE_SetPartInfo(struct Part *rp)
{
   int size = rp->Size = FileSize(rp->Filename);
   if (!rp->Decoded && rp->Nr) switch (rp->EncodingCode)
   {
      case ENC_UUE: case ENC_B64: rp->Size = (100*size)/136; break;
      case ENC_QP:                rp->Size = (100*size)/106; break;
   }
   if (!*rp->Name && rp->CParName) { stccpy(rp->Name, rp->CParName, SIZE_FILE); UnquoteString(rp->Name, FALSE); }
   switch (rp->Nr)
   {
      case 0:  SetComment(rp->Filename, GetStr(MSG_RE_Header)); break;
      case 1:  SetComment(rp->Filename, GetStr(MSG_RE_Letter)); break;
      default: SetComment(rp->Filename, *rp->Description ? rp->Description : rp->Name); break;
   }
   rp->Printable = !strnicmp(rp->ContentType, "text", 4) || rp->Nr == 0;
}
///
/// RE_ParseMessage
//  Parses a complete message
struct Part *RE_ParseMessage(int winnum, FILE *in, char *fname, struct Part *hrp)
{
   if (fname) in = fopen(fname, "r");
   if (in)
   {
      FILE *out;
      struct Part *rp;
      char *boundary;
      if (!hrp) if (out = RE_OpenNewPart(winnum, &hrp, NULL, NULL))
      {
         BOOL parse_ok = RE_ScanHeader(hrp, in, out, 0);
         fclose(out);
         if (parse_ok) RE_SetPartInfo(hrp);
      }
      else ER_NewError(GetStr(MSG_ER_CantCreateTempfile), NULL, NULL);
      if (hrp)
      {
         if (!(boundary = hrp->CParBndr)) boundary = hrp->JunkParameter;
         if (!strnicmp(hrp->ContentType, "multipart", 9))
         {
            if (!boundary) ER_NewError(GetStr(MSG_ER_MissingBoundary), NULL, NULL);
            else
            {
               BOOL done;
               if (*boundary == '"') boundary = UnquoteString(boundary, TRUE);
               sprintf(hrp->Boundary, "--%s", boundary);
               done = RE_ConsumeRestOfPart(in, NULL, NULL, hrp);
               rp = hrp;
               while (!done)
               {
                  struct Part *prev = rp, *newrp;
                  out = RE_OpenNewPart(winnum, &rp, prev, hrp);
                  if (!RE_ScanHeader(rp, in, out, 1)) break;
                  if (!strnicmp(rp->ContentType, "multipart", 9))
                  {
                     fclose(out);
                     if (newrp = RE_ParseMessage(winnum, in, NULL, rp))
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
         else if (out = RE_OpenNewPart(winnum, &rp, hrp, hrp))
         {
            if (RE_SaveThisPart(rp) || RE_RequiresSpecialHandling(hrp) == 3)
            {
               RE_ConsumeRestOfPart(in, out, NULL, NULL); fclose(out);
               RE_SetPartInfo(rp);
            }
            else
            {
               fclose(out); RE_UndoPart(rp);
               RE_ConsumeRestOfPart(in, NULL, NULL, NULL);
            }
         }
      }
      if (fname) fclose(in);
   } 
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
      if (in = fopen(rp->Filename, "r"))
      {
         if (rp->HasHeaders) while (GetLine(in, buf, SIZE_LINE)) if (!*buf) break;
         stcgfe(ext, rp->Name);
         if (strlen(ext) > 10) *ext = 0;
         sprintf(file, "YAMmsg-w%ldp%ld.%s", rp->Win, rp->Nr, *ext ? ext : "tmp");
         strmfp(buf, C->TempDir, file);
         if (out = fopen(buf, "w"))
         {
            RE_DecodeStream(rp, in, out);
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
      if (part->ContentType) FreeStrBuf(part->ContentType);
      free(part);
   }
   re->FirstPart     = NULL;
   re->FirstReadDone = FALSE;
   FinishUnpack(re->File);

}
///
/// RE_HandleMDNReport
//  Translates a message disposition notification to readable text
void RE_HandleMDNReport(struct Part *frp)
{
   struct Part *rp[3];
   char file[SIZE_FILE], buf[SIZE_PATHFILE], MDNtype[SIZE_DEFAULT];
   char *msgdesc, *mode = "", *type;
   int i, j;
   FILE *out, *fh;

   if (rp[0] = frp->Next) if (rp[1] = rp[0]->Next)
   {
      rp[2] = rp[1]->Next;
      msgdesc = AllocStrBuf(80);
      strcpy(MDNtype, "");
      for (j = 1; j < (rp[2] ? 3 : 2); j++)
      {
         RE_DecodePart(rp[j]);
         if (fh = fopen(rp[j]->Filename, "r"))
         {
            MA_ReadHeader(fh);
            fclose(fh);
            for (i = 0; i < Header.Used; i++)
            {
               char *value, *field = Header.Data[i];
               if (value = strchr(field, ':'))
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
      if (type = strchr(MDNtype, ';')) type = Trim(++type); else type = MDNtype;
      sprintf(file, "YAMmsg-w%ldp%ld.txt", rp[0]->Win, rp[0]->Nr);
      strmfp(buf, C->TempDir, file);
      if (out = fopen(buf, "w"))
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
void RE_HandleSignedMessage(struct Part *frp)
{
   struct Part *rp[2];

   if (rp[0] = frp->Next)
   {
      if (*C->PGPCmdPath && (rp[1] = rp[0]->Next))
      {
         int error;
         struct TempFile *tf = OpenTempFile(NULL);
         char options[SIZE_LARGE];
         G->RE[frp->Win]->PGPSigned |= PGPS_MIME;
         ConvertCRLF(rp[0]->Filename, tf->Filename, TRUE);
         sprintf(options, (G->PGPVersion == 5) ? "%s -o %s +batchmode=1 +force +language=us" : "%s %s +bat +f", rp[1]->Filename, tf->Filename);
         error = PGPCommand((G->PGPVersion == 5) ? "pgpv": "pgp", options, NOERRORS|KEEPLOG);
         if (error > 0) G->RE[frp->Win]->PGPSigned |= PGPS_BADSIG;
         if (error >= 0) RE_GetSigFromLog(frp->Win, NULL);
         tf->FP = NULL; CloseTempFile(tf);
      }
      RE_DecodePart(rp[0]);
   }
}
///
/// RE_DecryptPGP
//  Decrypts a PGP encrypted file
int RE_DecryptPGP(int winnum, char *src)
{
   FILE *fh;
   int error;
   char options[SIZE_LARGE], orcpt[SIZE_ADDRESS];

   *orcpt = 0;
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
      sprintf(options, "%s +bat +f", src);
      error = PGPCommand("pgp", options, KEEPLOG|NOERRORS);
      RE_GetSigFromLog(winnum, NULL);
   }
   PGPClearPassPhrase(error < 0 || error > 1);
   if (error < 0 || error > 1) if (fh = fopen(src, "w"))
   {
      fprintf(fh, GetStr(MSG_RE_PGPNotAllowed));
      if (G->PGPVersion == 5 && *orcpt) fprintf(fh, GetStr(MSG_RE_MsgReadOnly), orcpt);
      fclose(fh);
   }
   return error;
}
///
/// RE_HandleEncryptedMessage
//  Handles a PGP encryped message
void RE_HandleEncryptedMessage(struct Part *frp)
{
   struct Part *rp[2];
   FILE *in;
   if (rp[0] = frp->Next) if (rp[1] = rp[0]->Next)
   {
      if (!RE_DecryptPGP(frp->Win, rp[1]->Filename))
      {
         G->RE[frp->Win]->PGPSigned |= PGPS_OLD;
      }
      G->RE[frp->Win]->PGPEncrypted |= PGPE_MIME;
      if (ConvertCRLF(rp[1]->Filename, rp[0]->Filename, FALSE)) if (in = fopen(rp[0]->Filename, "r"))
      {
         rp[0]->ContentType = StrBufCpy(rp[0]->ContentType, "text/plain");
         rp[0]->Printable = TRUE; rp[0]->EncodingCode = ENC_NONE;
         *rp[0]->Description = 0;
         RE_ScanHeader(rp[0], in, NULL, 2);
         fclose(in);
         rp[0]->Decoded = FALSE; RE_DecodePart(rp[0]);
         RE_UndoPart(rp[1]);
      }
   }
}
///
/// RE_LoadMessagePart
//  Decodes a single message part
void RE_LoadMessagePart(int winnum, struct Part *part)
{
   struct Part *rp, *next;
   int rsh = RE_RequiresSpecialHandling(part);

   switch (rsh)
   {
      case 1:  RE_HandleMDNReport(part); break;
      case 2:  RE_HandleSignedMessage(part); break;
      case 3:  RE_HandleEncryptedMessage(part); break;
      default:
      for (rp = part->Next; rp; rp = next)
      {
         next = rp->Next;
         if (RE_IsURLencoded(rp))
         {
            rp->ContentType = StrBufCpy(rp->ContentType, "text/plain");
            rp->EncodingCode = ENC_FORM;
            RE_DecodePart(rp);
         }
         else if (!stricmp(rp->ContentType, "application/pgp-keys"))
            G->RE[winnum]->PGPKey = TRUE;
         else if (rp->Nr < 2 || (rp->Printable && C->DisplayAllTexts)) RE_DecodePart(rp);
      }
   }
}
///
/// RE_LoadMessage
//  Prepares a message for displaying
BOOL RE_LoadMessage(int winnum, int parsemode)
{
   char newfile[SIZE_PATHFILE], file[SIZE_FILE];
   struct Part *rp;
   int i;
   Busy(GetStr(MSG_BusyReading), "", 0, 0);
   RE_CleanupMessage(winnum);
   if (!StartUnpack(G->RE[winnum]->File, newfile, G->RE[winnum]->MailPtr->Folder)) return FALSE;
   strcpy(G->RE[winnum]->File, newfile);
   G->RE[winnum]->ParseMode = parsemode;
   if (rp = G->RE[winnum]->FirstPart = RE_ParseMessage(winnum, NULL, G->RE[winnum]->File, NULL))
   {
      RE_LoadMessagePart(winnum, rp);
      for (i = 0; rp; i++, rp = rp->Next) if (rp->Nr != i)
      {
         rp->Nr = i;
         sprintf(file, "YAMmsg-w%ldp%ld%s", winnum, i, strchr(rp->Filename,'.'));
         strmfp(newfile, C->TempDir, file);
         RenameFile(rp->Filename, newfile);
         strcpy(rp->Filename, newfile);
      }
   }
   BusyEnd;
   return TRUE;
}
///
/// RE_GetPart
//  Gets a message part by its index number
struct Part *RE_GetPart(int winnum, int partnr)
{
   struct Part *part;
   for (part = G->RE[winnum]->FirstPart; part; part = part->Next) if (part->Nr == partnr) break;
   return part;
}
///
/// RE_InitPrivateRC
//  Allocates resources for background message parsing
void RE_InitPrivateRC(struct Mail *mail, int parsemode)
{
   G->RE[4] = calloc(1,sizeof(struct RE_ClassData));
   G->RE[4]->Mail = *mail;
   G->RE[4]->MailPtr = mail;
   GetMailFile(G->RE[4]->File, mail->Folder, mail);
   RE_LoadMessage(4, parsemode);
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
/// AppendToBuffer
//  Appends a string to a dynamic-length buffer
char *AppendToBuffer(char *buf, int *wptr, int *len, char *add)
{
   int nlen = *len, npos = (*wptr)+strlen(add);
   while (npos >= nlen-1) nlen = (nlen*3)/2;
   if (nlen != *len) buf = realloc(buf, *len = nlen);
   while (*add) buf[(*wptr)++] = *add++;
   return buf;
}
///
/// RE_ExtractURL
//  Extracts URL from a message line
BOOL RE_ExtractURL(char *line, char *url, char **urlptr, char **rest)
{
   char *protocols[7] = { "mailto:", "http://", "https://", "ftp://", "gopher://", "telnet://", "news:" };
   char *legalchars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz@_?+-,.~/%&=:*#";
   char *foundurl = NULL, *p;
   int i;
   if (p = strchr(line, ':')) for (i = 0; i < 7; i++) if (foundurl = stristr(line, protocols[i])) break;
   if (!foundurl) return FALSE;
   for (i = 0; foundurl[i] && strchr(legalchars, foundurl[i]) && i < SIZE_URL-1; i++) url[i] = foundurl[i];
   if (strchr(".?!", url[i-1])) --i;
   url[i] = 0;
   if (urlptr) *urlptr = foundurl;
   if (rest) *rest = &foundurl[i];
   return TRUE;
}
///
/// RE_ReadInMessage
//  Reads a message into the display buffer
char *RE_ReadInMessage(int winnum, int mode)
{
   struct RE_ClassData *re = G->RE[winnum];
   struct Part *part, *uup = NULL, *last, *first = re->FirstPart;
   char buffer[SIZE_LARGE], *msg, *cmsg, *ptr, *rptr, *eolptr, url[SIZE_URL], *urlptr, *tsb, *sb, *bo, *pl;
   int totsize, len, wptr;
   FILE *fh;
   
   if (re->NoTextstyles) { tsb = "\033c\033[s:18]\033l"; sb = "\033[s:2]"; bo = "\033b"; pl = "\033n"; }
   else { tsb = "<tsb>"; sb = "<sb>"; bo = "*"; pl = "*"; }
   for (totsize = 1000, part = first; part; part = part->Next)
   {
      if (mode != RIM_READ && part->Nr && part->Nr != C->LetterPart) continue;
      if (part->Decoded || !part->Nr) totsize += part->Size; else totsize += 200;
   }
   if (cmsg = calloc(len=(totsize*3)/2,1))
   {
      if (mode != RIM_QUIET) Busy(GetStr(MSG_BusyDisplaying), "", 0, 0);
      wptr = 0;
      if (mode == RIM_READ)
         if (fh = fopen(first->Filename, "r"))
         {
            int buflen = re->FirstPart->MaxHeaderLen+4;
            char *linebuf = malloc(buflen);
            while (fgets(linebuf, buflen, fh))
               cmsg = AppendToBuffer(cmsg, &wptr, &len, linebuf);
            free(linebuf);
            fclose(fh);
            cmsg = AppendToBuffer(cmsg, &wptr, &len, "\n");
         }
      for (part = first->Next; part; part = part->Next)
      {
         BOOL dodisp = (part->Printable && part->Decoded);
         if (mode != RIM_READ && part->Nr > 1) break;
         if (mode == RIM_READ && (part->Nr > 1 || !dodisp))
         {
            *buffer = 0; sprintf(buffer, "%s%ld: %s\n%s%s:%s %s   %s%s:%s %ld %s\n", tsb, part->Nr, part->Name, bo, GetStr(MSG_RE_ContentType), pl, DescribeCT(part->ContentType), bo, GetStr(MSG_Size), pl, part->Size, GetStr(MSG_Bytes));
            if (*buffer) cmsg = AppendToBuffer(cmsg, &wptr, &len, buffer);
            *buffer = 0; if (*part->Description) sprintf(&buffer[strlen(buffer)], "%s%s:%s %s\n", bo, GetStr(MSG_RE_Description), pl, part->Description);
            if (dodisp) { strcat(buffer, sb); strcat(buffer, "\n"); }
            if (*buffer) cmsg = AppendToBuffer(cmsg, &wptr, &len, buffer);
         }
         if (dodisp)
         {
            if (fh = fopen(part->Filename, "r"))
            {       
               if (msg = calloc(part->Size+3,1))
               {
               char *sigptr;

                  *msg = '\n';
                  fread(msg+1, 1, part->Size, fh);
                  rptr = msg+1;

                  // find signature first if it should be stripped
                  if (mode == RIM_QUOTE && C->StripSignature)
                  {
                  int lines=21;

                     sigptr = msg + part->Size;
                     while(sigptr > msg)
                     {
                        sigptr--;
                        while((sigptr > msg) && (*sigptr != '\n')) sigptr--;  // step back to previous line
                        if((!--lines) || (sigptr <= msg+1))                   // abort after 20 lines or if at msg start
                        {
                           sigptr = NULL;
                           break;
                        }
                        if(strncmp(sigptr+1,"-- ",3) == 0)                    // check for sig separator
                        {
                           sigptr++;
                           break;
                        }
                     }
                  }

                  while (*rptr)
                  {
                     for (eolptr = rptr; *eolptr && *eolptr != '\n'; eolptr++); *eolptr = 0;
/* UUencoded */      if (!strncmp(rptr, "begin ", 6) && isdigit((int)rptr[6]))
                     {
                        if (!re->FirstReadDone)
                        {
                           FILE *ufh;
                           ptr = &rptr[6];
                           while (!ISpace(*ptr)) ptr++;
                           ptr = stpblk(ptr);
                           for (last = first; last->Next; last = last->Next);
                           if (ufh = RE_OpenNewPart(winnum, &uup, last, first))
                           {
                              uup->ContentType = StrBufCpy(uup->ContentType, "application/octet-stream");
                              strcpy(uup->Description, GetStr(MSG_RE_UUencodedFile));
                              stccpy(uup->Name, ptr, SIZE_FILE);
                              fromuuetxt(&rptr, ufh);
                              fclose(ufh);
                              uup->Decoded = TRUE;
                              RE_SetPartInfo(uup);
                              eolptr = rptr-1; ptr = rptr;
                           }
                           else ER_NewError(GetStr(MSG_ER_CantCreateTempfile), NULL, NULL);
                        }
                        else
                        {
                           for (ptr=eolptr+1; *ptr; ptr++)
                           {
                              if (!strncmp(ptr, "end", 3)) break;
                              while (*ptr && *ptr != '\n') ptr++;
                           }
                           while (*ptr && *ptr != '\n') ptr++; eolptr = ptr++;
                        }
                        if (!strncmp(ptr, "size", 4))
                        {
                           if (!re->FirstReadDone)
                           {
                              int expsize = atoi(&ptr[5]);
                              if (uup->Size != expsize) ER_NewError(GetStr(MSG_ER_UUSize), (char *)uup->Size, (char *)expsize);
                           }
                           for (eolptr = ptr; *eolptr && *eolptr!='\n'; eolptr++); *eolptr = 0;
                        }
                        goto rim_cont;
                     }
/* PGP message */    if (!strncmp(rptr, "-----BEGIN PGP MESSAGE", 21))
                     {
                        struct TempFile *tf;
                        if (tf = OpenTempFile("w"))
                        {
                           *eolptr = '\n';
                           for (ptr=eolptr+1; *ptr; ptr++)
                           {
                              if (!strncmp(ptr, "-----END PGP MESSAGE", 19)) break;
                              while (*ptr && *ptr != '\n') ptr++;
                           }
                           while (*ptr && *ptr != '\n') ptr++; eolptr = ptr++;
                           fwrite(rptr, 1, ptr-rptr, tf->FP);
                           fclose(tf->FP); tf->FP = NULL;
                           if (!RE_DecryptPGP(winnum, tf->Filename)) re->PGPSigned |= PGPS_OLD;
                           if (tf->FP = fopen(tf->Filename, "r"))
                           {
                              char buf2[SIZE_LARGE];
                              while (fgets(buf2, SIZE_LARGE, tf->FP))
                              {
                                 rptr = buf2;
                                 cmsg = AppendToBuffer(cmsg, &wptr, &len, buf2);
                              }
                           }
                           CloseTempFile(tf);
                        }
                        re->PGPEncrypted |= PGPE_OLD;
                        goto rim_cont;
                     }
/* signature */      if (!strcmp(rptr, "-- "))
                     {
                        if (mode == RIM_QUOTE && C->StripSignature && (rptr == sigptr)) break;
                        else if (mode == RIM_READ)
                        {
                           if (C->SigSepLine == 1) cmsg = AppendToBuffer(cmsg, &wptr, &len, rptr);
                           if (C->SigSepLine == 2) cmsg = AppendToBuffer(cmsg, &wptr, &len, sb);
                           if (C->SigSepLine == 3) break;
                           cmsg = AppendToBuffer(cmsg, &wptr, &len, "\n");
                           goto rim_cont;
                        }
                     }
/* URL */            if (!re->NoTextstyles && mode == RIM_READ) if (RE_ExtractURL(rptr, url, &urlptr, &ptr))
                     {
                        char *buf2, *p;
                        if (buf2 = calloc(SIZE_DEFAULT+(strlen(rptr)*3)/2,1))
                        {
                           p = buf2;
                           do
                           {
                              while (rptr < urlptr) *p++ = *rptr++;
                              sprintf(p, "\033p[7]%s\033p[0]", url);
                              p = &buf2[strlen(buf2)]; rptr = ptr;
                           } while (RE_ExtractURL(rptr, url, &urlptr, &ptr));
                           strcpy(p, rptr); strcat(p, "\n");
                           cmsg = AppendToBuffer(cmsg, &wptr, &len, buf2);
                           free(buf2);
                           goto rim_cont;
                        }
                     }
                     if (!strncmp(rptr, "-----BEGIN PGP PUBLIC KEY BLOCK", 31)) re->PGPKey = TRUE;
                     if (!strncmp(rptr, "-----BEGIN PGP SIGNED MESSAGE", 29)) re->PGPSigned |= PGPS_OLD;
                     cmsg = AppendToBuffer(cmsg, &wptr, &len, rptr);
                     cmsg = AppendToBuffer(cmsg, &wptr, &len, "\n");
rim_cont:
                     rptr = eolptr+1;
                     if (mode == RIM_QUIET) DoMethod(G->App,MUIM_Application_InputBuffered);
                  }
                  free(msg);
               }
               fclose(fh);
            }
         }
      }
      re->FirstReadDone = TRUE;
      if (mode != RIM_QUIET) BusyEnd;
   }
   return cmsg;
}
///
/// RE_AddExtraHeader
//  Adds additional headers to the header listview
void RE_AddExtraHeader(APTR lv, char *header, char *value)
{
   char buffer[SIZE_LARGE];
   if (!*value) return;
   sprintf(buffer, MUIX_I"%s: %s", StripUnderscore(header), value);
   DoMethod(lv, MUIM_NList_InsertSingle, buffer, MUIV_NList_Insert_Bottom);
}
///
/// RE_GetSenderInfo
//  Parses X-SenderInfo header field
void RE_GetSenderInfo(struct Mail *mail, struct ABEntry *ab)
{
   char *s, *t, *eq;
   struct ExtendedMail *email;

   clear(ab, sizeof(struct ABEntry));
   stccpy(ab->Address, mail->From.Address, SIZE_ADDRESS);
   stccpy(ab->RealName, mail->From.RealName, SIZE_REALNAME);
   if (mail->Flags & MFLAG_SENDERINFO)
   {
      email = MA_ExamineMail(mail->Folder, mail->MailFile, NULL, TRUE);
      if (s = strchr(email->SenderInfo, ';'))
      {
         *s++ = 0;
         do {
            if (t = ParamEnd(s)) *t++ = 0;
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
         } while (t);
      }
      MA_FreeEMailStruct(email);
   }
}
///
/// RE_UpdateSenderInfo
//  Updates address book entry of sender
void RE_UpdateSenderInfo(struct ABEntry *old, struct ABEntry *new)
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
   if (changed) AB_SaveABookFunc();
}
///
/// RE_AddSenderInfo
//  Displays sender information to header listview
void RE_AddSenderInfo(int winnum, struct ABEntry *ab)
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
struct ABEntry *RE_AddToAddrbook(APTR win, struct ABEntry *templ)
{
   struct ABEntry new;
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
      struct MUIS_Listtree_TreeNode *tn = MUIV_Listtree_Insert_ListNode_Root;
      int hits = 0;

      if (*C->NewAddrGroup) if (!AB_SearchEntry(MUIV_Listtree_GetEntry_ListNode_Root, C->NewAddrGroup, ASM_ALIAS|ASM_GROUP, &hits, &tn))
      {
         clear(&new, sizeof(struct ABEntry));
         stccpy(new.Alias, C->NewAddrGroup, SIZE_NAME);
         stccpy(new.Comment, GetStr(MSG_RE_NewGroupTitle), SIZE_DEFAULT);
         new.Type = AET_GROUP;
         tn = (struct MUIS_Listtree_TreeNode *)DoMethod(G->AB->GUI.LV_ADRESSES, MUIM_Listtree_Insert, new.Alias, &new, MUIV_Listtree_Insert_ListNode_Root, MUIV_Listtree_Insert_PrevNode_Sorted, TNF_LIST);
      }
      clear(&new, sizeof(struct ABEntry));
      new.Type = AET_USER;
      RE_UpdateSenderInfo(&new, templ);
      EA_SetDefaultAlias(&new);
      tn = (struct MUIS_Listtree_TreeNode *)DoMethod(G->AB->GUI.LV_ADRESSES, MUIM_Listtree_Insert, new.Alias, &new, tn, MUIV_Listtree_Insert_PrevNode_Sorted, 0);
      if (tn)
      {
         AB_SaveABookFunc();
         return tn->tn_User;
      }
   }
   return NULL;
}
///
/// RE_FindPhotoOnDisk
//  Searches portrait of sender in the gallery directory
BOOL RE_FindPhotoOnDisk(struct ABEntry *ab, char *photo)
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
BOOL RE_DownloadPhoto(APTR win, char *url, struct ABEntry *ab)
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
      for (i = 2; PFExists(C->GalleryDir, fname); i++) sprintf(fname, "%s%ld.%s", name, i, ext);
      strmfp(picfname, C->GalleryDir, fname);
      if (TR_OpenTCPIP())
      {
         Busy(GetStr(MSG_BusyDownloadingPic), name, 0, 0);
         CreateDirectory(C->GalleryDir);
         if (TR_DownloadURL(url, NULL, NULL, picfname))
         {
            strcpy(ab->Photo, picfname);
            AB_SaveABookFunc();
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
void RE_DisplayMessage(int winnum)
{
   char *cmsg, *body;
   BOOL dispheader;
   struct RE_GUIData *gui = &(G->RE[winnum]->GUI);
   struct Person *from = &G->RE[winnum]->Mail.From;
   struct MUIS_Listtree_TreeNode *tn;
   struct ABEntry *ab = NULL, abtmpl;
   int hits = 0;

   if (cmsg = RE_ReadInMessage(winnum, RIM_READ))
   {
      dispheader = G->RE[winnum]->Header != 0;
      set(gui->GR_HEAD, MUIA_ShowMe, dispheader);
      set(gui->BO_BALANCE, MUIA_ShowMe, dispheader);
      DoMethod(gui->LV_HEAD, MUIM_NList_Clear);
      set(gui->LV_HEAD, MUIA_NList_Quiet, TRUE);
      body = cmsg;
      while (*body)
      {
         if (*body == '\n') { body++; break; }
         dispheader = G->RE[winnum]->Header == 2;
         if (G->RE[winnum]->Header == 1)
         {
            char header[SIZE_DEFAULT];
            int i;
            for (i = 0; !strchr("\n :", body[i]) && i < SIZE_DEFAULT-1; i++) header[i] = body[i];
            header[i] = 0;
            dispheader = MatchNoCase(header, C->ShortHeaders);
         }
         if (dispheader) DoMethod(gui->LV_HEAD, MUIM_NList_InsertSingleWrap, body, MUIV_NList_Insert_Bottom, G->RE[winnum]->WrapHeader ? WRAPCOL1 : NOWRAP, ALIGN_LEFT);
         while (*body && *body != '\n') body++;
         if (*body) body++;
      }
      if (!AB_SearchEntry(MUIV_Listtree_GetEntry_ListNode_Root, from->Address, ASM_ADDRESS|ASM_USER, &hits, &tn) && *from->RealName)
           AB_SearchEntry(MUIV_Listtree_GetEntry_ListNode_Root, from->RealName, ASM_REALNAME|ASM_USER, &hits, &tn);
      if (hits) ab = tn->tn_User;
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
            if (!*ab->Photo && *abtmpl.Photo && *C->GalleryDir) RE_DownloadPhoto(gui->WI, abtmpl.Photo, ab);
         }
         else
         {
            if (ab = RE_AddToAddrbook(gui->WI, &abtmpl))
            {
               if (*abtmpl.Photo && *C->GalleryDir) RE_DownloadPhoto(gui->WI, abtmpl.Photo, ab);
            }
            else { ab = &abtmpl; *ab->Photo = 0; }
         }
      }
      if (G->RE[winnum]->SenderInfo)
      {
         if (hits || ab->Type == 1) RE_AddSenderInfo(winnum, ab);
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
      set(gui->TE_TEXT, MUIA_TextEditor_ImportHook, G->RE[winnum]->NoTextstyles ? MUIV_TextEditor_ImportHook_Plain : MUIV_TextEditor_ImportHook_EMail);
      set(gui->TE_TEXT, MUIA_TextEditor_FixedFont, G->RE[winnum]->FixedFont);
      set(gui->TE_TEXT, MUIA_TextEditor_Contents, body);
      free(cmsg);
   }
}
///
/// RE_ClickedOnMessage
//  User clicked on a e-mail address
void RE_ClickedOnMessage(char *address)
{
   struct MUIS_Listtree_TreeNode *tn;
   struct ABEntry *ab = NULL;
   int l, win, hits = 0;
   char *p, *gads, buf[SIZE_LARGE], *body = NULL, *subject = NULL;
   if (l = strlen(address)) if (strchr(".?!", address[--l])) address[l] = 0;
   for (p = strchr(address, '&'); p; p = strchr(p, '&'))
   {
      *p++ = 0;
      if (!strnicmp(p, "body=", 5)) body = &p[5];
      if (!strnicmp(p, "subject=", 8)) subject = &p[8];
   }
   if (AB_SearchEntry(MUIV_Listtree_GetEntry_ListNode_Root, address, ASM_ADDRESS|ASM_USER|ASM_LIST, &hits, &tn)) ab = tn->tn_User;
   sprintf(buf, GetStr(MSG_RE_SelectAddressReq), address);
   gads = GetStr(hits ? MSG_RE_SelectAddressEdit : MSG_RE_SelectAddressAdd);
   switch (MUI_Request(G->App, G->MA->GUI.WI, 0, NULL, gads, buf))
   {
      case 1: if ((win = MA_NewNew(NULL, 0)) >= 0)
              {
                 struct WR_GUIData *gui = &G->WR[win]->GUI;
                 setstring(gui->ST_TO, hits ? BuildAddrName(address, ab->RealName) : address);
                 if (subject) setstring(gui->ST_SUBJECT, subject);
                 if (body) set(gui->TE_EDIT, MUIA_TextEditor_Contents, body);
                 set(gui->WI, MUIA_Window_ActiveObject, gui->ST_SUBJECT);
              }
              break;
      case 2: DoMethod(G->App, MUIM_CallHook, &AB_OpenHook, ABM_EDIT);
              if (hits)
              {
                 if ((win = EA_Init(ab->Type, tn)) >= 0) EA_Setup(win, ab);
              }
              else
              {
                 if ((win = EA_Init(AET_USER, NULL)) >= 0) setstring(G->EA[win]->GUI.ST_ADDRESS, address);
              }
              break;
   }
}
///
/// RE_DoubleClickFunc
//  Handles double-clicks on an URL
SAVEDS ASM BOOL RE_DoubleClickFunc(REG(a1,struct ClickMessage *clickmsg), REG(a2,APTR obj))
{
   int pos = clickmsg->ClickPosition;
   char *line = clickmsg->LineContents, *p, *surl;
   static char url[SIZE_URL];

   DoMethod(G->App, MUIM_Application_InputBuffered);
   while (pos && !ISpace(line[pos-1]) && line[pos-1] != '<') pos--;
   surl = &line[pos];
   for (p = url; !ISpace(line[pos]) && line[pos] != '>' && line[pos] != '\n' && line[pos] && p-url < SIZE_URL; pos++) *p++ = line[pos];
   *p = 0;
   if (RE_ExtractURL(surl, url, NULL, NULL))
      if (!strnicmp(url, "mailto:", 7)) RE_ClickedOnMessage(&url[7]);
      else GotoURL(url);
   else if (strchr(url, '@')) RE_ClickedOnMessage(url);
   else if (isdigit(line[0]) && (line[1] == ':' || line[2] == ':'))
   {
      int pnr = atoi(line), winnum;
      struct Part *part;
      get(_win(obj), MUIA_UserData, &winnum);
      part = RE_GetPart(winnum, pnr);
      RE_DecodePart(part);
      RE_DisplayMIME(part->Filename, part->ContentType);
   }
   else return FALSE;
   return TRUE;
}
MakeHook(RE_DoubleClickHook, RE_DoubleClickFunc);
///
/// RE_ShowEnvFunc
//  Changes display options (header, textstyles, sender info)
SAVEDS ASM void RE_ShowEnvFunc(REG(a1,int *arg))
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
   RE_DisplayMessage(winnum);
   set(re->GUI.SL_TEXT, MUIA_Prop_First, lev);
}
MakeHook(RE_ShowEnvHook, RE_ShowEnvFunc);
///

/*** GUI ***/
/// RE_LV_AttachDspFunc
//  Attachment listview display hook
SAVEDS ASM long RE_LV_AttachDspFunc(REG(a2,char **array), REG(a1,struct Part *entry))
{
   if (entry)
   {
      static char dispnu[SIZE_SMALL], dispna[SIZE_CTYPE], dispsz[SIZE_SMALL];
      array[0] = array[2] = "";
      if (entry->Nr > 0) sprintf(array[0] = dispnu, "%ld", entry->Nr);
      sprintf(array[1] = dispna, *entry->Name ? entry->Name : DescribeCT(entry->ContentType));
      if (entry->Size) sprintf(array[2] = dispsz, "%s%ld", entry->Decoded ? "" : "~", entry->Size);
   }
   return 0;
}
MakeHook(RE_LV_AttachDspFuncHook,RE_LV_AttachDspFunc);
///
/// RE_CloseFunc
//  Closes a read window
SAVEDS ASM void RE_CloseFunc(REG(a1,int *arg))
{
   int winnum = *arg;
   struct RE_ClassData *re = G->RE[winnum];

   RE_CleanupMessage(winnum);
   if (Virtual(re->MailPtr))
   {
      free(re->MailPtr);
      CloseTempFile(re->TempFile);
   }
   G->Weights[2] = GetMUI(re->GUI.GR_HEAD, MUIA_VertWeight);
   G->Weights[3] = GetMUI(re->GUI.GR_BODY, MUIA_VertWeight);
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
SAVEDS ASM long RE_LV_HDspFunc(REG(a2,char **array), REG(a1,char *entry))
{
   static char hfield[40];
   char *cont = entry;
   int i = 0;

   clear(hfield, 40);
   while (*cont != ':' && *cont && i < 38) hfield[i++] = *cont++;
   array[0] = hfield;
   array[1] = stpblk(++cont);
   return 0;
}
MakeHook(RE_LV_HDspHook,RE_LV_HDspFunc);
///
/// RE_New
//  Creates a read window
enum {   RMEN_EDIT=501,RMEN_MOVE,RMEN_COPY,RMEN_DELETE,RMEN_PRINT,RMEN_SAVE,RMEN_DISPLAY,RMEN_DETACH,RMEN_CROP,RMEN_NEW,RMEN_REPLY,RMEN_FORWARD,RMEN_BOUNCE,RMEN_SAVEADDR,RMEN_SETUNREAD,RMEN_CHSUBJ,
         RMEN_PREV,RMEN_NEXT,RMEN_URPREV,RMEN_URNEXT,RMEN_PREVTH,RMEN_NEXTTH,
         RMEN_EXTKEY,RMEN_CHKSIG,RMEN_SAVEDEC,
         RMEN_HNONE,RMEN_HSHORT,RMEN_HFULL,RMEN_SNONE,RMEN_SDATA,RMEN_SFULL,RMEN_WRAPH,RMEN_TSTYLE,RMEN_FFONT };

APTR RE_LEDGroup(char *filename)
{
   return PageGroup, Child, HSpace(0), Child, MakeStatusFlag(filename), End;
}
struct RE_ClassData *RE_New(int winnum, BOOL real)
{
   struct RE_ClassData *data;

   if (data = calloc(1,sizeof(struct RE_ClassData)))
   {
      APTR tb_butt[14] = { MSG_RE_TBPrev,MSG_RE_TBNext,MSG_RE_TBPrevTh,MSG_RE_TBNextTh,MSG_Space,
                           MSG_RE_TBDisplay,MSG_RE_TBSave,MSG_RE_TBPrint,MSG_Space,
                           MSG_RE_TBDelete,MSG_RE_TBMove,MSG_RE_TBReply,MSG_RE_TBForward,NULL };
      APTR tb_help[14] = { MSG_HELP_RE_BT_PREVIOUS,MSG_HELP_RE_BT_NEXT,MSG_HELP_RE_BT_QUESTION,MSG_HELP_RE_BT_ANSWER,NULL,
                           MSG_HELP_RE_BT_DISPLAY,MSG_HELP_RE_BT_EXPORT,MSG_HELP_RE_BT_PRINT,NULL,
                           MSG_HELP_RE_BT_DELETE,MSG_HELP_RE_BT_MOVE,MSG_HELP_RE_BT_REPLY,MSG_HELP_RE_BT_FORWARD,NULL };
      int i;
      for (i = 0; i < 14; i++) SetupToolbar(&(data->GUI.TB_TOOLBAR[i]), tb_butt[i]?(tb_butt[i]==MSG_Space?"":GetStr(tb_butt[i])):NULL, tb_help[i]?GetStr(tb_help[i]):NULL, 0);
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
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_MA_ChangeSubj), MUIA_Menuitem_Enabled,real, MUIA_UserData,RMEN_CHSUBJ, End,
            End,
            MUIA_Family_Child, MenuObject, MUIA_Menu_Title, GetStr(MSG_RE_Navigation),
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_RE_MNext),  MUIA_Menuitem_Shortcut, "right", MUIA_Menuitem_Enabled,real, MUIA_Menuitem_CommandString,TRUE, MUIA_UserData,RMEN_NEXT, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_RE_MPrev),  MUIA_Menuitem_Shortcut, "left", MUIA_Menuitem_Enabled,real, MUIA_Menuitem_CommandString,TRUE, MUIA_UserData,RMEN_PREV, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_RE_MURNext),MUIA_Menuitem_Shortcut, "shift right", MUIA_Menuitem_Enabled,real, MUIA_Menuitem_CommandString,TRUE, MUIA_UserData,RMEN_URNEXT, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_RE_MURPrev),MUIA_Menuitem_Shortcut, "shift right", MUIA_Menuitem_Enabled,real, MUIA_Menuitem_CommandString,TRUE, MUIA_UserData,RMEN_URPREV, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_RE_MNextTh),MUIA_Menuitem_Shortcut, ">", MUIA_Menuitem_Enabled,real, MUIA_UserData,RMEN_NEXTTH, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_RE_MPrevTh),MUIA_Menuitem_Shortcut, "<", MUIA_Menuitem_Enabled,real, MUIA_UserData,RMEN_PREVTH, End,
            End,
            MUIA_Family_Child, MenuObject, MUIA_Menu_Title, "PGP",
               MUIA_Family_Child, data->GUI.MI_EXTKEY = MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_RE_ExtractKey), MUIA_Menuitem_Shortcut,"X", MUIA_UserData,RMEN_EXTKEY, End,
               MUIA_Family_Child, data->GUI.MI_CHKSIG = MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_RE_SigCheck), MUIA_Menuitem_Shortcut,"K", MUIA_UserData,RMEN_CHKSIG, End,
               MUIA_Family_Child, data->GUI.MI_SAVEDEC = MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_RE_SaveDecrypted), MUIA_Menuitem_Shortcut,"V", MUIA_UserData,RMEN_SAVEDEC, End,
            End,
            MUIA_Family_Child, MenuObject, MUIA_Menu_Title, GetStr(MSG_MA_Settings),
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_RE_NoHeaders),  MUIA_Menuitem_Shortcut,"0", MUIA_Menuitem_Checkit,TRUE, MUIA_Menuitem_Checked,data->Header==0, MUIA_Menuitem_Exclude,0x06, MUIA_UserData,RMEN_HNONE, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_RE_ShortHeaders), MUIA_Menuitem_Shortcut,"1", MUIA_Menuitem_Checkit,TRUE, MUIA_Menuitem_Checked,data->Header==1, MUIA_Menuitem_Exclude,0x05, MUIA_UserData,RMEN_HSHORT, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_RE_FullHeaders),  MUIA_Menuitem_Shortcut,"2", MUIA_Menuitem_Checkit,TRUE, MUIA_Menuitem_Checked,data->Header==2, MUIA_Menuitem_Exclude,0x03, MUIA_UserData,RMEN_HFULL, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,NM_BARLABEL, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_RE_NoSInfo), MUIA_Menuitem_Shortcut,"3", MUIA_Menuitem_Checkit,TRUE, MUIA_Menuitem_Checked,data->SenderInfo==0, MUIA_Menuitem_Exclude,0x60, MUIA_UserData,RMEN_SNONE, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_RE_SInfo), MUIA_Menuitem_Shortcut,"4", MUIA_Menuitem_Checkit,TRUE, MUIA_Menuitem_Checked,data->SenderInfo==1, MUIA_Menuitem_Exclude,0x50, MUIA_UserData,RMEN_SDATA, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_RE_SInfoImage), MUIA_Menuitem_Shortcut,"5", MUIA_Menuitem_Checkit,TRUE, MUIA_Menuitem_Checked,data->SenderInfo==2, MUIA_Menuitem_Exclude,0x30, MUIA_UserData,RMEN_SFULL, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,NM_BARLABEL, End,
               MUIA_Family_Child, data->GUI.MI_WRAPH = MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_RE_WrapHeader), MUIA_Menuitem_Shortcut,"H", MUIA_Menuitem_Checkit,TRUE, MUIA_Menuitem_Checked,data->WrapHeader, MUIA_Menuitem_Toggle,TRUE, MUIA_UserData,RMEN_WRAPH, End,
               MUIA_Family_Child, data->GUI.MI_TSTYLE = MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_RE_Textstyles), MUIA_Menuitem_Shortcut,"T", MUIA_Menuitem_Checkit,TRUE, MUIA_Menuitem_Checked,!data->NoTextstyles, MUIA_Menuitem_Toggle,TRUE, MUIA_UserData,RMEN_TSTYLE, End,
               MUIA_Family_Child, data->GUI.MI_FFONT = MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_RE_FixedFont), MUIA_Menuitem_Shortcut,"F", MUIA_Menuitem_Checkit,TRUE, MUIA_Menuitem_Checked,data->FixedFont, MUIA_Menuitem_Toggle,TRUE, MUIA_UserData,RMEN_FFONT, End,
            End,
         End,
         WindowContents, VGroup,
            Child, (C->HideGUIElements & HIDE_TBAR) ?
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
                        Child, data->GUI.GR_STATUS[1] = RE_LEDGroup("status_crypt"),
                        Child, data->GUI.GR_STATUS[2] = RE_LEDGroup("status_signed"),
                     End,
                     Child, VSpace(0),
                  End,
               End),
            Child, VGroup,
               Child, data->GUI.GR_HEAD = HGroup, GroupSpacing(0),
                  MUIA_ShowMe, data->Header > 0,
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
                  MUIA_ShowMe, data->Header > 0,
               End,
               Child, data->GUI.GR_BODY = HGroup,
                  MUIA_VertWeight, G->Weights[3],
                  MUIA_Group_Spacing, 0,
                  Child, data->GUI.TE_TEXT = NewObject(CL_TextEditor->mcc_Class,NULL,
                     InputListFrame,
                     MUIA_TextEditor_Slider, data->GUI.SL_TEXT,
                     MUIA_TextEditor_FixedFont, data->FixedFont,
                     MUIA_TextEditor_DoubleClickHook, &RE_DoubleClickHook,
                     MUIA_TextEditor_ImportHook, MUIV_TextEditor_ImportHook_EMail,
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
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,RMEN_EDIT            ,MUIV_Notify_Application,6,MUIM_CallHook,&RE_NewHook,NEW_EDIT,0,winnum,FALSE);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,RMEN_MOVE            ,MUIV_Notify_Application,3,MUIM_CallHook,&RE_MoveHook,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,RMEN_COPY            ,MUIV_Notify_Application,3,MUIM_CallHook,&RE_CopyHook,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,RMEN_DELETE          ,MUIV_Notify_Application,5,MUIM_CallHook,&RE_DeleteHook,0,winnum,FALSE);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,RMEN_PRINT           ,MUIV_Notify_Application,3,MUIM_CallHook,&RE_PrintHook,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,RMEN_SAVE            ,MUIV_Notify_Application,3,MUIM_CallHook,&RE_SaveHook,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,RMEN_DISPLAY         ,MUIV_Notify_Application,3,MUIM_CallHook,&RE_DisplayHook,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,RMEN_DETACH          ,MUIV_Notify_Application,3,MUIM_CallHook,&RE_SaveAllHook,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,RMEN_CROP            ,MUIV_Notify_Application,3,MUIM_CallHook,&RE_RemoveAttachHook,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,RMEN_NEW             ,MUIV_Notify_Application,6,MUIM_CallHook,&RE_NewHook,NEW_NEW,0,winnum,FALSE);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,RMEN_REPLY           ,MUIV_Notify_Application,6,MUIM_CallHook,&RE_NewHook,NEW_REPLY,0,winnum,FALSE);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,RMEN_FORWARD         ,MUIV_Notify_Application,6,MUIM_CallHook,&RE_NewHook,NEW_FORWARD,0,winnum,FALSE);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,RMEN_BOUNCE          ,MUIV_Notify_Application,6,MUIM_CallHook,&RE_NewHook,NEW_BOUNCE,0,winnum,FALSE);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,RMEN_SAVEADDR        ,MUIV_Notify_Application,3,MUIM_CallHook,&RE_GetAddressHook,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,RMEN_SETUNREAD       ,MUIV_Notify_Application,3,MUIM_CallHook,&RE_SetUnreadHook,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,RMEN_CHSUBJ          ,MUIV_Notify_Application,3,MUIM_CallHook,&RE_ChangeSubjectHook,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,RMEN_PREV            ,MUIV_Notify_Application,6,MUIM_CallHook,&RE_PrevNextHook,-1,0,winnum,FALSE);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,RMEN_NEXT            ,MUIV_Notify_Application,6,MUIM_CallHook,&RE_PrevNextHook,1,0,winnum,FALSE);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,RMEN_URPREV          ,MUIV_Notify_Application,6,MUIM_CallHook,&RE_PrevNextHook,-1,IEQUALIFIER_LSHIFT,winnum,FALSE);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,RMEN_URNEXT          ,MUIV_Notify_Application,6,MUIM_CallHook,&RE_PrevNextHook,1,IEQUALIFIER_LSHIFT,winnum,FALSE);
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
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,RMEN_FFONT           ,MUIV_Notify_Application,4,MUIM_CallHook,&RE_ShowEnvHook,winnum,8);
         DoMethod(data->GUI.TO_TOOLBAR ,MUIM_Toolbar_Notify, 0, MUIV_Toolbar_Notify_Pressed,MUIV_EveryTime, MUIV_Notify_Application,6,MUIM_CallHook,&RE_PrevNextHook,-1,MUIV_Toolbar_Qualifier,winnum,MUIV_TriggerValue);
         DoMethod(data->GUI.TO_TOOLBAR ,MUIM_Toolbar_Notify, 1, MUIV_Toolbar_Notify_Pressed,MUIV_EveryTime, MUIV_Notify_Application,6,MUIM_CallHook,&RE_PrevNextHook,1,MUIV_Toolbar_Qualifier,winnum,MUIV_TriggerValue);
         DoMethod(data->GUI.TO_TOOLBAR ,MUIM_Toolbar_Notify, 2, MUIV_Toolbar_Notify_Pressed,FALSE, MUIV_Notify_Application,4,MUIM_CallHook,&RE_FollowHook,-1,winnum);
         DoMethod(data->GUI.TO_TOOLBAR ,MUIM_Toolbar_Notify, 3, MUIV_Toolbar_Notify_Pressed,FALSE, MUIV_Notify_Application,4,MUIM_CallHook,&RE_FollowHook,1,winnum);
         DoMethod(data->GUI.TO_TOOLBAR ,MUIM_Toolbar_Notify, 5, MUIV_Toolbar_Notify_Pressed,FALSE, MUIV_Notify_Application,3,MUIM_CallHook,&RE_DisplayHook,winnum);
         DoMethod(data->GUI.TO_TOOLBAR ,MUIM_Toolbar_Notify, 6, MUIV_Toolbar_Notify_Pressed,FALSE, MUIV_Notify_Application,3,MUIM_CallHook,&RE_SaveHook,winnum);
         DoMethod(data->GUI.TO_TOOLBAR ,MUIM_Toolbar_Notify, 7, MUIV_Toolbar_Notify_Pressed,FALSE, MUIV_Notify_Application,3,MUIM_CallHook,&RE_PrintHook,winnum);
         DoMethod(data->GUI.TO_TOOLBAR ,MUIM_Toolbar_Notify, 9, MUIV_Toolbar_Notify_Pressed,MUIV_EveryTime, MUIV_Notify_Application,5,MUIM_CallHook,&RE_DeleteHook,MUIV_Toolbar_Qualifier,winnum,MUIV_TriggerValue);
         DoMethod(data->GUI.TO_TOOLBAR ,MUIM_Toolbar_Notify,10, MUIV_Toolbar_Notify_Pressed,FALSE, MUIV_Notify_Application,3,MUIM_CallHook,&RE_MoveHook,winnum);
         DoMethod(data->GUI.TO_TOOLBAR ,MUIM_Toolbar_Notify,11, MUIV_Toolbar_Notify_Pressed,MUIV_EveryTime, MUIV_Notify_Application,6,MUIM_CallHook,&RE_NewHook,NEW_REPLY,MUIV_Toolbar_Qualifier,winnum,MUIV_TriggerValue);
         DoMethod(data->GUI.TO_TOOLBAR ,MUIM_Toolbar_Notify,12, MUIV_Toolbar_Notify_Pressed,MUIV_EveryTime, MUIV_Notify_Application,6,MUIM_CallHook,&RE_NewHook,NEW_FORWARD,MUIV_Toolbar_Qualifier,winnum,MUIV_TriggerValue);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_CloseRequest ,TRUE                 ,MUIV_Notify_Application,3,MUIM_CallHook,&RE_CloseHook,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_InputEvent   ,"-repeat del"        ,MUIV_Notify_Application,5,MUIM_CallHook,&RE_DeleteHook,0,winnum,FALSE);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_InputEvent   ,"-repeat shift del"  ,MUIV_Notify_Application,5,MUIM_CallHook,&RE_DeleteHook,IEQUALIFIER_LSHIFT,winnum,FALSE);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_InputEvent   ,"-repeat space"      ,data->GUI.TE_TEXT      ,2,MUIM_TextEditor_ARexxCmd,"Next Page");
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_InputEvent   ,"-repeat backspace"  ,data->GUI.TE_TEXT      ,2,MUIM_TextEditor_ARexxCmd,"Previous Page");
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_InputEvent   ,"-repeat left"       ,MUIV_Notify_Application,5,MUIM_CallHook,&RE_PrevNextHook,-1,False,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_InputEvent   ,"-repeat right"      ,MUIV_Notify_Application,5,MUIM_CallHook,&RE_PrevNextHook,1,False,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_InputEvent   ,"-repeat shift left" ,MUIV_Notify_Application,5,MUIM_CallHook,&RE_PrevNextHook,-1,True,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_InputEvent   ,"-repeat shift right",MUIV_Notify_Application,5,MUIM_CallHook,&RE_PrevNextHook,1,True,winnum);
         return data;
      }
      free(data);
   }
   return NULL;
}
///
