/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2002 by YAM Open Source Team

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
#include <mui/TextEditor_mcc.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/locale.h>
#include <proto/utility.h>
#include <proto/pm.h>

#include "extra.h"
#include "YAM.h"
#include "YAM_config.h"
#include "YAM_error.h"
#include "YAM_folderconfig.h"
#include "YAM_global.h"
#include "YAM_locale.h"
#include "YAM_utilities.h"

/***************************************************************************
 Module: Configuration - Basic Get/Put routines
***************************************************************************/

/// Bool2Txt
//  Converts boolean value to text
const char *Bool2Txt(BOOL bool)
{
   return bool ? "Y" : "N";
}

///
/// Txt2Bool
//  Converts Y/N string to boolean value
BOOL Txt2Bool(const char *txt)
{
   return (BOOL)(toupper((int)*txt) == 'Y' || (int)*txt == '1');
}

///
/// CO_SaveConfig
//  Saves configuration to a file
void CO_SaveConfig(struct Config *co, char *fname)
{
   FILE *fh;
   int i;

   if (fh = fopen(fname, "w"))
   {
      fprintf(fh, "YCO3 - YAM Configuration\n");

      fprintf(fh, "\n[First steps]\n");
      fprintf(fh, "RealName         = %s\n", co->RealName);
      fprintf(fh, "EmailAddress     = %s\n", co->EmailAddress);
      fprintf(fh, "TimeZone         = %ld\n", co->TimeZone);
      fprintf(fh, "DaylightSaving   = %s\n", Bool2Txt(co->DaylightSaving));

      fprintf(fh, "\n[TCP/IP]\n");
      fprintf(fh, "SMTP-Server      = %s\n", co->SMTP_Server);
      fprintf(fh, "SMTP-Domain      = %s\n", co->SMTP_Domain);
      fprintf(fh, "Allow8bit        = %s\n", Bool2Txt(co->Allow8bit));
      fprintf(fh, "Use-SMTP-TLS     = %s\n", Bool2Txt(co->Use_SMTP_TLS));
      fprintf(fh, "Use-SMTP-AUTH    = %s\n", Bool2Txt(co->Use_SMTP_AUTH));
      fprintf(fh, "SMTP-AUTH-User   = %s\n", co->SMTP_AUTH_User);
      fprintf(fh, "SMTP-AUTH-Pass   = %s\n", Encrypt(co->SMTP_AUTH_Pass));
      for (i = 0; i < MAXP3; i++) if (co->P3[i])
      {
         struct POP3 *p3 = co->P3[i];
         fprintf(fh, "POP%02ld.Server     = %s\n", i, p3->Server);
         fprintf(fh, "POP%02ld.Port       = %ld\n", i, p3->Port);
         fprintf(fh, "POP%02ld.User       = %s\n", i, p3->User);
         fprintf(fh, "POP%02ld.Password   = %s\n", i, Encrypt(p3->Password));
         fprintf(fh, "POP%02ld.Enabled    = %s\n", i, Bool2Txt(p3->Enabled));
         fprintf(fh, "POP%02ld.SSLMode    = %ld\n", i, p3->SSLMode);
         fprintf(fh, "POP%02ld.UseAPOP    = %s\n", i, Bool2Txt(p3->UseAPOP));
         fprintf(fh, "POP%02ld.Delete     = %s\n", i, Bool2Txt(p3->DeleteOnServer));
      }

      fprintf(fh, "\n[New mail]\n");
      fprintf(fh, "AvoidDuplicates  = %s\n", Bool2Txt(co->AvoidDuplicates));
      fprintf(fh, "PreSelection     = %ld\n", co->PreSelection);
      fprintf(fh, "TransferWindow   = %ld\n", co->TransferWindow);
      fprintf(fh, "UpdateStatus     = %s\n", Bool2Txt(co->UpdateStatus));
      fprintf(fh, "WarnSize         = %ld\n", co->WarnSize);
      fprintf(fh, "CheckMailDelay   = %ld\n", co->CheckMailDelay);
      fprintf(fh, "DownloadLarge    = %s\n", Bool2Txt(co->DownloadLarge));
      fprintf(fh, "NotifyType       = %ld\n", co->NotifyType);
      fprintf(fh, "NotifySound      = %s\n", co->NotifySound);
      fprintf(fh, "NotifyCommand    = %s\n", co->NotifyCommand);

      fprintf(fh, "\n[Filters]\n");
      for (i = 0; i < MAXRU; i++) if (co->RU[i])
      {
         struct Rule *ru = co->RU[i];
         fprintf(fh, "FI%02ld.Name        = %s\n", i, ru->Name);
         fprintf(fh, "FI%02ld.Remote      = %s\n", i, Bool2Txt(ru->Remote));
         fprintf(fh, "FI%02ld.ApplyToNew  = %s\n", i, Bool2Txt(ru->ApplyToNew));
         fprintf(fh, "FI%02ld.ApplyToSent = %s\n", i, Bool2Txt(ru->ApplyToSent));
         fprintf(fh, "FI%02ld.ApplyOnReq  = %s\n", i, Bool2Txt(ru->ApplyOnReq));
         fprintf(fh, "FI%02ld.Field       = %ld\n", i, ru->Field[0]);
         fprintf(fh, "FI%02ld.SubField    = %ld\n", i, ru->SubField[0]);
         fprintf(fh, "FI%02ld.CustomField = %s\n", i, ru->CustomField[0]);
         fprintf(fh, "FI%02ld.Comparison  = %ld\n", i, ru->Comparison[0]);
         fprintf(fh, "FI%02ld.Match       = %s\n", i, ru->Match[0]);
         fprintf(fh, "FI%02ld.CaseSens    = %s\n", i, Bool2Txt(ru->CaseSens[0]));
         fprintf(fh, "FI%02ld.Substring   = %s\n", i, Bool2Txt(ru->Substring[0]));
         fprintf(fh, "FI%02ld.Combine     = %ld\n", i, ru->Combine);
         if (ru->Combine)
         {
            fprintf(fh, "FI%02ld.Field2      = %ld\n", i, ru->Field[1]);
            fprintf(fh, "FI%02ld.SubField2   = %ld\n", i, ru->SubField[1]);
            fprintf(fh, "FI%02ld.CustomField2= %s\n", i, ru->CustomField[1]);
            fprintf(fh, "FI%02ld.Comparison2 = %ld\n", i, ru->Comparison[1]);
            fprintf(fh, "FI%02ld.Match2      = %s\n", i, ru->Match[1]);
            fprintf(fh, "FI%02ld.CaseSens2   = %s\n", i, Bool2Txt(ru->CaseSens[1]));
            fprintf(fh, "FI%02ld.Substring2  = %s\n", i, Bool2Txt(ru->Substring[1]));
         }
         fprintf(fh, "FI%02ld.Actions     = %ld\n", i, ru->Actions);
         fprintf(fh, "FI%02ld.BounceTo    = %s\n", i, ru->BounceTo);
         fprintf(fh, "FI%02ld.ForwardTo   = %s\n", i, ru->ForwardTo);
         fprintf(fh, "FI%02ld.ReplyFile   = %s\n", i, ru->ReplyFile);
         fprintf(fh, "FI%02ld.ExecuteCmd  = %s\n", i, ru->ExecuteCmd);
         fprintf(fh, "FI%02ld.PlaySound   = %s\n", i, ru->PlaySound);
         fprintf(fh, "FI%02ld.MoveTo      = %s\n", i, ru->MoveTo);
      }

      fprintf(fh, "\n[Read]\n");
      fprintf(fh, "ShowHeader       = %ld\n", co->ShowHeader);
      fprintf(fh, "ShortHeaders     = %s\n", co->ShortHeaders);
      fprintf(fh, "ShowSenderInfo   = %ld\n", co->ShowSenderInfo);
      fprintf(fh, "WrapHeader       = %s\n", Bool2Txt(co->WrapHeader));
      fprintf(fh, "SigSepLine       = %ld\n", co->SigSepLine);
      fprintf(fh, "ColoredText      = %s\n", co->ColoredText.buf);
      fprintf(fh, "Color2ndLevel    = %s\n", co->Color2ndLevel.buf);
      fprintf(fh, "DisplayAllTexts  = %s\n", Bool2Txt(co->DisplayAllTexts));
      fprintf(fh, "FixedFontEdit    = %s\n", Bool2Txt(co->FixedFontEdit));
      fprintf(fh, "UseTextstyles    = %s\n", Bool2Txt(co->UseTextstyles));
      fprintf(fh, "MultipleWindows  = %s\n", Bool2Txt(co->MultipleWindows));
      fprintf(fh, "TranslationIn    = %s\n", co->TranslationIn);
      fprintf(fh, "AutoTranslationIn= %s\n", Bool2Txt(co->AutomaticTranslationIn));

      fprintf(fh, "\n[Write]\n");
      fprintf(fh, "ReplyTo          = %s\n", co->ReplyTo);
      fprintf(fh, "Organization     = %s\n", co->Organization);
      fprintf(fh, "ExtraHeaders     = %s\n", co->ExtraHeaders);
      fprintf(fh, "NewIntro         = %s\n", co->NewIntro);
      fprintf(fh, "Greetings        = %s\n", co->Greetings);
      fprintf(fh, "TranslationOut   = %s\n", co->TranslationOut);
      fprintf(fh, "WarnSubject      = %s\n", Bool2Txt(co->WarnSubject));
      fprintf(fh, "EdWrapCol        = %ld\n", co->EdWrapCol);
      fprintf(fh, "EdWrapMode       = %ld\n", co->EdWrapMode);
      fprintf(fh, "Editor           = %s\n", co->Editor);
      fprintf(fh, "LaunchAlways     = %s\n", Bool2Txt(co->LaunchAlways));
      fprintf(fh, "EmailCache       = %ld\n", co->EmailCache);

      fprintf(fh, "\n[Reply/Forward]\n");
      fprintf(fh, "ReplyHello       = %s\n", co->ReplyHello);
      fprintf(fh, "ReplyIntro       = %s\n", co->ReplyIntro);
      fprintf(fh, "ReplyBye         = %s\n", co->ReplyBye);
      fprintf(fh, "AltReplyHello    = %s\n", co->AltReplyHello);
      fprintf(fh, "AltReplyIntro    = %s\n", co->AltReplyIntro);
      fprintf(fh, "AltReplyBye      = %s\n", co->AltReplyBye);
      fprintf(fh, "AltReplyPattern  = %s\n", co->AltReplyPattern);
      fprintf(fh, "MLReplyHello     = %s\n", co->MLReplyHello);
      fprintf(fh, "MLReplyIntro     = %s\n", co->MLReplyIntro);
      fprintf(fh, "MLReplyBye       = %s\n", co->MLReplyBye);
      fprintf(fh, "ForwardIntro     = %s\n", co->ForwardIntro);
      fprintf(fh, "ForwardFinish    = %s\n", co->ForwardFinish);
      fprintf(fh, "QuoteMessage     = %s\n", Bool2Txt(co->QuoteMessage));
      fprintf(fh, "QuoteText        = %s\n", co->QuoteText);
      fprintf(fh, "AltQuoteText     = %s\n", co->AltQuoteText);
      fprintf(fh, "QuoteEmptyLines  = %s\n", Bool2Txt(co->QuoteEmptyLines));
      fprintf(fh, "CompareAddress   = %s\n", Bool2Txt(co->CompareAddress));
      fprintf(fh, "StripSignature   = %s\n", Bool2Txt(co->StripSignature));

      fprintf(fh, "\n[Signature]\n");
      fprintf(fh, "UseSignature     = %s\n", Bool2Txt(co->UseSignature));
      fprintf(fh, "TagsFile         = %s\n", co->TagsFile);
      fprintf(fh, "TagsSeparator    = %s\n", co->TagsSeparator);

      fprintf(fh, "\n[Lists]\n");
      fprintf(fh, "FolderCols       = %ld\n", co->FolderCols);
      fprintf(fh, "MessageCols      = %ld\n", co->MessageCols);
      fprintf(fh, "FixedFontList    = %s\n", Bool2Txt(co->FixedFontList));
      fprintf(fh, "SwatchBeat       = %s\n", Bool2Txt(co->SwatchBeat));
      fprintf(fh, "SizeFormat       = %ld\n", co->SizeFormat);
      fprintf(fh, "FolderCntMenu    = %s\n", Bool2Txt(co->FolderCntMenu));
      fprintf(fh, "MessageCntMenu   = %s\n", Bool2Txt(co->MessageCntMenu));
      fprintf(fh, "InfoBar          = %ld\n", co->InfoBar);
      fprintf(fh, "InfoBarText      = %s\n", co->InfoBarText);

      fprintf(fh, "\n[Security]\n");
      fprintf(fh, "PGPCmdPath       = %s\n", co->PGPCmdPath);
      fprintf(fh, "MyPGPID          = %s\n", co->MyPGPID);
      fprintf(fh, "EncryptToSelf    = %s\n", Bool2Txt(co->EncryptToSelf));
      fprintf(fh, "ReMailer         = %s\n", co->ReMailer);
      fprintf(fh, "RMCommands       = %s\n", co->RMCommands);
      fprintf(fh, "LogfilePath      = %s\n", co->LogfilePath);
      fprintf(fh, "LogfileMode      = %ld\n", co->LogfileMode);
      fprintf(fh, "SplitLogfile     = %s\n", Bool2Txt(co->SplitLogfile));
      fprintf(fh, "LogAllEvents     = %s\n", Bool2Txt(co->LogAllEvents));

      fprintf(fh, "\n[Start/Quit]\n");
      fprintf(fh, "GetOnStartup     = %s\n", Bool2Txt(co->GetOnStartup));
      fprintf(fh, "SendOnStartup    = %s\n", Bool2Txt(co->SendOnStartup));
      fprintf(fh, "CleanupOnStartup = %s\n", Bool2Txt(co->CleanupOnStartup));
      fprintf(fh, "RemoveOnStartup  = %s\n", Bool2Txt(co->RemoveOnStartup));
      fprintf(fh, "LoadAllFolders   = %s\n", Bool2Txt(co->LoadAllFolders));
      fprintf(fh, "UpdateNewMail    = %s\n", Bool2Txt(co->UpdateNewMail));
      fprintf(fh, "CheckBirthdates  = %s\n", Bool2Txt(co->CheckBirthdates));
      fprintf(fh, "SendOnQuit       = %s\n", Bool2Txt(co->SendOnQuit));
      fprintf(fh, "CleanupOnQuit    = %s\n", Bool2Txt(co->CleanupOnQuit));
      fprintf(fh, "RemoveOnQuit     = %s\n", Bool2Txt(co->RemoveOnQuit));

      fprintf(fh, "\n[MIME]\n");
      for (i = 0; i < MAXMV; i++) if (co->MV[i])
      {
         fprintf(fh, "MV%02ld.ContentType = %s\n", i, co->MV[i]->ContentType);
         fprintf(fh, "MV%02ld.Extension   = %s\n", i, co->MV[i]->Extension);
         fprintf(fh, "MV%02ld.Command     = %s\n", i, co->MV[i]->Command);
      }
      fprintf(fh, "IdentifyBin      = %s\n", Bool2Txt(co->IdentifyBin));
      fprintf(fh, "DetachDir        = %s\n", co->DetachDir);
      fprintf(fh, "AttachDir        = %s\n", co->AttachDir);

      fprintf(fh, "\n[Address book]\n");
      fprintf(fh, "GalleryDir       = %s\n", co->GalleryDir);
      fprintf(fh, "MyPictureURL     = %s\n", co->MyPictureURL);
      fprintf(fh, "ProxyServer      = %s\n", co->ProxyServer);
      fprintf(fh, "NewAddrGroup     = %s\n", co->NewAddrGroup);
      fprintf(fh, "AddToAddrbook    = %ld\n", co->AddToAddrbook);
      fprintf(fh, "AddMyInfo        = %s\n", Bool2Txt(co->AddMyInfo));
      fprintf(fh, "AddrbookCols     = %ld\n", co->AddrbookCols);

      fprintf(fh, "\n[Scripts]\n");
      for (i = 0; i < MAXRX; i++)
      {
         if (i < 10)
           fprintf(fh, "Rexx%02ld.Name      = %s\n", i, co->RX[i].Name);
         fprintf(fh, "Rexx%02ld.Script    = %s\n", i, co->RX[i].Script);
         fprintf(fh, "Rexx%02ld.IsAmigaDOS= %s\n", i, Bool2Txt(co->RX[i].IsAmigaDOS));
         fprintf(fh, "Rexx%02ld.UseConsole= %s\n", i, Bool2Txt(co->RX[i].UseConsole));
         fprintf(fh, "Rexx%02ld.WaitTerm  = %s\n", i, Bool2Txt(co->RX[i].WaitTerm));
      }

      fprintf(fh, "\n[Mixed]\n");
      fprintf(fh, "TempDir          = %s\n", co->TempDir);
      fprintf(fh, "IconPosition     = %ld;%ld\n", co->IconPositionX, co->IconPositionY);
      fprintf(fh, "IconifyOnQuit    = %s\n", Bool2Txt(co->IconifyOnQuit));
      fprintf(fh, "Confirm          = %s\n", Bool2Txt(co->Confirm));
      fprintf(fh, "ConfirmDelete    = %ld\n", co->ConfirmDelete);
      fprintf(fh, "RemoveAtOnce     = %s\n", Bool2Txt(co->RemoveAtOnce));
      fprintf(fh, "SaveSent         = %s\n", Bool2Txt(co->SaveSent));
      fprintf(fh, "MDN_Display      = %ld\n", co->MDN_Display);
      fprintf(fh, "MDN_Process      = %ld\n", co->MDN_Process);
      fprintf(fh, "MDN_Delete       = %ld\n", co->MDN_Delete);
      fprintf(fh, "MDN_Filter       = %ld\n", co->MDN_Filter);
      fprintf(fh, "SendMDNAtOnce    = %s\n", Bool2Txt(co->SendMDNAtOnce));
      fprintf(fh, "XPKPack          = %s;%ld\n", co->XPKPack, co->XPKPackEff);
      fprintf(fh, "XPKPackEncrypt   = %s;%ld\n", co->XPKPackEncrypt, co->XPKPackEncryptEff);
      fprintf(fh, "PackerCommand    = %s\n", co->PackerCommand);
      fprintf(fh, "AppIconText      = %s\n", co->AppIconText);

      fprintf(fh, "\n[Advanced]\n");
      fprintf(fh, "LetterPart       = %ld\n", co->LetterPart);
      fprintf(fh, "WriteIndexes     = %ld\n", co->WriteIndexes);
      fprintf(fh, "AutoSave         = %ld\n", co->AutoSave);
      fprintf(fh, "SupportSite      = %s\n", co->SupportSite);
      fprintf(fh, "JumpToNewMsg     = %s\n", Bool2Txt(co->JumpToNewMsg));
			fprintf(fh, "JumpToIncoming   = %s\n", Bool2Txt(co->JumpToIncoming));
      fprintf(fh, "AskJumpUnread    = %s\n", Bool2Txt(co->AskJumpUnread));
      fprintf(fh, "PrinterCheck     = %s\n", Bool2Txt(co->PrinterCheck));
      fprintf(fh, "IsOnlineCheck    = %s\n", Bool2Txt(co->IsOnlineCheck));
      fprintf(fh, "IOCInterface     = %s\n", co->IOCInterface);
      fprintf(fh, "ConfirmOnQuit    = %s\n", Bool2Txt(co->ConfirmOnQuit));
      fprintf(fh, "HideGUIElements  = %ld\n", co->HideGUIElements);
      fprintf(fh, "LocalCharset     = %s\n", co->LocalCharset);
      fprintf(fh, "StackSize        = %ld\n", co->StackSize);
      fprintf(fh, "PrintMethod      = %ld\n", co->PrintMethod);
      fclose(fh);
      AppendLogVerbose(60, GetStr(MSG_LOG_SavingConfig), fname, "", "", "");
   }
   else ER_NewError(GetStr(MSG_ER_CantCreateFile), fname, NULL);
}

///
/// CO_LoadConfig
//  Loads configuration from a file
BOOL CO_LoadConfig(struct Config *co, char *fname, struct Folder ***oldfolders)
{
   struct Folder **ofo = NULL;
   FILE *fh;
   char buffer[SIZE_LARGE];
   int version;
   
   if (fh = fopen(fname, "r"))
   {
      fgets(buffer, SIZE_LARGE, fh);
      if (!Strnicmp(buffer, "YCO", 3))
      {
         version = buffer[3]-'0';
         CO_SetDefaults(co, -1);
         while (fgets(buffer, SIZE_LARGE, fh))
         {
            char *p, *p2, *value, *value2 = "";
            if (value = strchr(buffer, '=')) for (value2 = (++value)+1; ISpace(*value); value++);
            if (p = strpbrk(buffer,"\r\n")) *p = 0;
            for (p = buffer; *p && *p != '=' && !ISpace(*p); p++); *p = 0;
            if (*buffer && value)
            {
               if (version == 2)
               {
                  if (!stricmp(buffer, "POP3-Server"))    stccpy(co->P3[0]->Server, value, SIZE_HOST);
                  if (!stricmp(buffer, "POP3-Password"))  stccpy(co->P3[0]->Password, Decrypt(value), SIZE_PASSWORD);
                  if (!stricmp(buffer, "POP3-User"))      stccpy(co->P3[0]->User, value, SIZE_USERID);
                  if (!stricmp(buffer, "DeleteOnServer")) co->P3[0]->DeleteOnServer = Txt2Bool(value);
                  if (!stricmp(buffer, "CheckMail"))      if (!Txt2Bool(value)) co->CheckMailDelay = 0;
                  if (!stricmp(buffer, "ConfirmSize")) switch (atoi(value))
                  {
                     case  0: co->PreSelection = 2; co->WarnSize = 0; break;
                     case 13: co->PreSelection = 0; co->WarnSize = 0; break;
                     default: co->PreSelection = 1; co->WarnSize = 1<<atoi(value); break;
                  }
                  if (!stricmp(buffer, "Verbosity"))      co->TransferWindow = atoi(value) > 0 ? 2 : 0;
                  if (!stricmp(buffer, "WordWrap"))       co->EdWrapCol = atoi(value);
                  if (!stricmp(buffer, "DeleteOnExit"))   co->RemoveAtOnce = !(co->RemoveOnQuit = Txt2Bool(value));
                  if (!strnicmp(buffer, "Folder", 6) && oldfolders)
                  {
                     static int sortconv[4] = { -1, 1, 3, 5 };
                     int j = atoi(&buffer[6]), type;
                     if (!ofo) ofo = *oldfolders = calloc(100, sizeof(struct Folder *));
                     if (j >= 3) for (j = 4; j < 100; j++) if (!ofo[j]) break;
                     type = (j == 0 ? FT_INCOMING : (j == 1 ? FT_OUTGOING : (j == 2 ? FT_SENT : FT_CUSTOM)));
                     p = strchr(&value[4], ';'); *p++ = 0;
                     if (!ofo[j]) if (ofo[j] = FO_NewFolder(type, &value[4], p)) ofo[j]->Sort[0] = sortconv[atoi(&value[2])];
                  }
                  if (!strnicmp(buffer, "Rule", 4))
                  {
                     int j = atoi(&buffer[4]);
                     struct Rule *ru = co->RU[j];
                     if (!ru) ru = co->RU[j] = CO_NewRule();
                     ru->ApplyToNew = ru->ApplyOnReq = Txt2Bool(value);
                     p = strchr(p2 = &value[2], ';'); *p++ = 0;
                     stccpy(ru->Name, p2, SIZE_NAME);
                     if ((ru->Field[0] = atoi(p)) == 2) ru->Field[0] = 4;
                     ru->CaseSens[0] = Txt2Bool(&p[2]);
                     ru->Comparison[0] = Txt2Bool(&p[4]) ? 1 : 0;
                     p = strchr(p2 = &p[6], ';'); *p++ = 0;
                     stccpy(ru->Match[0], p2, SIZE_PATTERN);
                     switch (atoi(p))
                     {
                        case 0: ru->Actions = 32; break;
                        case 1: ru->Actions = 64; break;
                        case 2: ru->Actions = 2; break;
                     }
                     p = strchr(p2 = &p[2], ';'); *p++ = 0;
                     stccpy(ru->MoveTo, p2, SIZE_NAME);
                     stccpy(ru->ForwardTo, p, SIZE_ADDRESS);
                     if (*ru->ForwardTo) ru->Actions |= 2;
                  }
                  if (!strnicmp(buffer, "MimeViewer", 10))
                  {
                     int j = atoi(&buffer[10]);
                     struct MimeView *mv = co->MV[j];
                     if (!mv) mv = co->MV[j] = CO_NewMimeView();
                     p = strchr(value, ';'); *p++ = 0;
                     stccpy(mv->ContentType, value, SIZE_CTYPE);
                     stccpy(mv->Command, p, SIZE_COMMAND);
                  }
                  if (!Strnicmp(buffer, "RexxMenu", 8))
                  {
                     int j = atoi(&buffer[8]);
                     stccpy(co->RX[j].Name, FilePart(value), SIZE_NAME);
                     stccpy(co->RX[j].Script, value, SIZE_PATHFILE);
                  }
               }
               if (!strnicmp(buffer, "FolderPath", 10) && oldfolders)
               {
                  int j = atoi(&buffer[10]);
                  if (!ofo) ofo = *oldfolders = calloc(100, sizeof(struct Folder *));
                  if (!ofo[j]) ofo[j] = FO_NewFolder(FT_CUSTOM, value, FilePart(value));
                  if (!FO_LoadConfig(ofo[j])) FO_SaveConfig(ofo[j]);
               }
/*0*/          if (!stricmp(buffer, "RealName"))       stccpy(co->RealName, value, SIZE_REALNAME);
               if (!stricmp(buffer, "EmailAddress"))   stccpy(co->EmailAddress, value, SIZE_ADDRESS);

               if (!stricmp(buffer, "TimeZone"))
               {
                 /* If Locale is present, don't use the timezone from the config */
                 if (G->Locale)
                 {
                    CloseLocale(G->Locale);
                    G->Locale = OpenLocale(NULL);
                    co->TimeZone = -G->Locale->loc_GMTOffset/60;
                 }else
                 {
                    co->TimeZone = atoi(value);
                 }
               }

               if (!stricmp(buffer, "DaylightSaving")) co->DaylightSaving = Txt2Bool(value);
/*1*/          if (!stricmp(buffer, "SMTP-Server"))    stccpy(co->SMTP_Server, value, SIZE_HOST);
               if (!stricmp(buffer, "SMTP-Port"))      co->SMTP_Port = atoi(value);
               if (!stricmp(buffer, "SMTP-Domain"))    stccpy(co->SMTP_Domain, value, SIZE_HOST);
               if (!stricmp(buffer, "Allow8bit"))      co->Allow8bit = Txt2Bool(value);
               if (!stricmp(buffer, "Use-SMTP-TLS"))   co->Use_SMTP_TLS = Txt2Bool(value);
               if (!stricmp(buffer, "Use-SMTP-AUTH"))  co->Use_SMTP_AUTH = Txt2Bool(value);
               if (!stricmp(buffer, "SMTP-AUTH-User")) stccpy(co->SMTP_AUTH_User, value, SIZE_USERID);
               if (!stricmp(buffer, "SMTP-AUTH-Pass")) stccpy(co->SMTP_AUTH_Pass, Decrypt(value), SIZE_USERID);
               if (!strnicmp(buffer, "POP", 3) && buffer[5] == '.')
               {
                  int j = atoi(&buffer[3]);
                  struct POP3 *p3 = co->P3[j];
                  p = &buffer[6];
                  if (!p3) p3 = co->P3[j] = CO_NewPOP3(co, FALSE);
                  if (!stricmp(p, "Server"))    stccpy(p3->Server, value, SIZE_HOST);
                  if (!stricmp(p, "Port"))      p3->Port = atoi(value);
                  if (!stricmp(p, "Password"))  stccpy(p3->Password, Decrypt(value), SIZE_PASSWORD);
                  if (!stricmp(p, "User"))      stccpy(p3->User, value, SIZE_USERID);
                  if (!stricmp(p, "Enabled"))   p3->Enabled = Txt2Bool(value);
                  if (!stricmp(p, "SSLMode"))   p3->SSLMode = atoi(value);
                  if (!stricmp(p, "UseAPOP"))   p3->UseAPOP = Txt2Bool(value);
                  if (!stricmp(p, "Delete"))    p3->DeleteOnServer = Txt2Bool(value);
               }
/*2*/          if (!stricmp(buffer, "AvoidDuplicates"))co->AvoidDuplicates = Txt2Bool(value);
               if (!stricmp(buffer, "PreSelection"))   co->PreSelection = atoi(value);
               if (!stricmp(buffer, "TransferWindow")) co->TransferWindow = atoi(value);
               if (!stricmp(buffer, "UpdateStatus"))   co->UpdateStatus = Txt2Bool(value);
               if (!stricmp(buffer, "WarnSize"))       co->WarnSize = atoi(value);
               if (!stricmp(buffer, "CheckMailDelay")) co->CheckMailDelay = atoi(value);
               if (!stricmp(buffer, "DownloadLarge"))  co->DownloadLarge = Txt2Bool(value);
               if (!stricmp(buffer, "NotifyType"))     co->NotifyType = atoi(value);
               if (!stricmp(buffer, "NotifySound"))    stccpy(co->NotifySound, value, SIZE_COMMAND);
               if (!stricmp(buffer, "NotifyCommand"))  stccpy(co->NotifyCommand, value, SIZE_COMMAND);
/*3*/          if (!strnicmp(buffer, "FI", 2) && buffer[4] == '.')
               {
                  int j = atoi(&buffer[2]);
                  struct Rule *ru = co->RU[j];
                  p = &buffer[5];
                  if (!ru) ru = co->RU[j] = CO_NewRule();
                  if (!stricmp(p, "Name"))       stccpy(ru->Name, value, SIZE_NAME);
                  if (!stricmp(p, "Remote"))     ru->Remote = Txt2Bool(value);
                  if (!stricmp(p, "ApplyToNew")) ru->ApplyToNew = Txt2Bool(value);
                  if (!stricmp(p, "ApplyToSent"))ru->ApplyToSent = Txt2Bool(value);
                  if (!stricmp(p, "ApplyOnReq")) ru->ApplyOnReq = Txt2Bool(value);
                  if (!strnicmp(p, "Field", 5))       ru->Field[p[5]=='2'] = atoi(value);
                  if (!strnicmp(p, "SubField", 8))    ru->SubField[p[8]=='2'] = atoi(value);
                  if (!strnicmp(p, "CustomField", 11)) stccpy(ru->CustomField[p[11]=='2'], value, SIZE_DEFAULT);
                  if (!strnicmp(p, "Comparison", 10)) ru->Comparison[p[10]=='2'] = atoi(value);
                  if (!strnicmp(p, "Match", 5))       stccpy(ru->Match[p[5]=='2'], value2, SIZE_PATTERN);
                  if (!strnicmp(p, "CaseSens", 8))    ru->CaseSens[p[8]=='2'] = Txt2Bool(value);
                  if (!strnicmp(p, "Substring", 9))   ru->Substring[p[9]=='2'] = Txt2Bool(value);
                  if (!stricmp(p, "Combine"))    ru->Combine = atoi(value);
                  if (!stricmp(p, "Actions"))    ru->Actions = atoi(value);
                  if (!stricmp(p, "BounceTo"))   stccpy(ru->BounceTo, value, SIZE_ADDRESS);
                  if (!stricmp(p, "ForwardTo"))  stccpy(ru->ForwardTo, value, SIZE_ADDRESS);
                  if (!stricmp(p, "ReplyFile"))  stccpy(ru->ReplyFile, value, SIZE_PATHFILE);
                  if (!stricmp(p, "ExecuteCmd")) stccpy(ru->ExecuteCmd, value, SIZE_COMMAND);
                  if (!stricmp(p, "PlaySound"))  stccpy(ru->PlaySound, value, SIZE_PATHFILE);
                  if (!stricmp(p, "MoveTo"))     stccpy(ru->MoveTo, value, SIZE_NAME);
               }
/*4*/          if (!stricmp(buffer, "ShowHeader"))     co->ShowHeader = atoi(value);
               if (!stricmp(buffer, "ShortHeaders"))   stccpy(co->ShortHeaders, value, SIZE_PATTERN);
               if (!stricmp(buffer, "ShowSenderInfo")) co->ShowSenderInfo = atoi(value);
               if (!stricmp(buffer, "WrapHeader"))     co->WrapHeader = Txt2Bool(value);
               if (!stricmp(buffer, "SigSepLine"))     co->SigSepLine = atoi(value);
               if (!stricmp(buffer, "ColoredText"))    stccpy(co->ColoredText.buf, value, 32);
               if (!stricmp(buffer, "Color2ndLevel"))  stccpy(co->Color2ndLevel.buf, value, 32);
               if (!stricmp(buffer, "DisplayAllTexts"))co->DisplayAllTexts = Txt2Bool(value);
               if (!stricmp(buffer, "FixedFontEdit"))  co->FixedFontEdit = Txt2Bool(value);
               if (!stricmp(buffer, "UseTextstyles"))  co->UseTextstyles = Txt2Bool(value);
               if (!stricmp(buffer, "MultipleWindows"))co->MultipleWindows = Txt2Bool(value);
               if (!stricmp(buffer, "TranslationIn"))  stccpy(co->TranslationIn, value, SIZE_PATHFILE);
               if (!stricmp(buffer, "AutoTranslationIn"))co->AutomaticTranslationIn = Txt2Bool(value);
/*5*/          if (!stricmp(buffer, "ReplyTo"))        stccpy(co->ReplyTo,  value, SIZE_ADDRESS);
               if (!stricmp(buffer, "Organization"))   stccpy(co->Organization, value, SIZE_DEFAULT);
               if (!stricmp(buffer, "ExtraHeaders"))   stccpy(co->ExtraHeaders, value, SIZE_LARGE);
               if (!stricmp(buffer, "NewIntro"))       stccpy(co->NewIntro, value2, SIZE_INTRO);
               if (!stricmp(buffer, "Greetings"))      stccpy(co->Greetings, value2, SIZE_INTRO);
               if (!stricmp(buffer, "TranslationOut")) stccpy(co->TranslationOut, value, SIZE_PATHFILE);
               if (!stricmp(buffer, "WarnSubject"))    co->WarnSubject = Txt2Bool(value);
               if (!stricmp(buffer, "EdWrapCol"))      co->EdWrapCol = atoi(value);
               if (!stricmp(buffer, "EdWrapMode"))     co->EdWrapMode = atoi(value);
               if (!stricmp(buffer, "Editor"))         stccpy(co->Editor, value, SIZE_PATHFILE);
               if (!stricmp(buffer, "LaunchAlways"))   co->LaunchAlways = Txt2Bool(value);
               if (!stricmp(buffer, "EmailCache"))     co->EmailCache = atoi(value);
/*6*/          if (!stricmp(buffer, "ReplyHello"))     stccpy(co->ReplyHello, value2, SIZE_INTRO);
               if (!stricmp(buffer, "ReplyIntro"))     stccpy(co->ReplyIntro, value2, SIZE_INTRO);
               if (!stricmp(buffer, "ReplyBye"))       stccpy(co->ReplyBye, value2, SIZE_INTRO);
               if (!stricmp(buffer, "AltReplyHello"))  stccpy(co->AltReplyHello, value2, SIZE_INTRO);
               if (!stricmp(buffer, "AltReplyIntro"))  stccpy(co->AltReplyIntro, value2, SIZE_INTRO);
               if (!stricmp(buffer, "AltReplyBye"))    stccpy(co->AltReplyBye, value2, SIZE_INTRO);
               if (!stricmp(buffer, "AltReplyPattern"))stccpy(co->AltReplyPattern, value2, SIZE_PATTERN);
               if (!stricmp(buffer, "MLReplyHello"))   stccpy(co->MLReplyHello, value2, SIZE_INTRO);
               if (!stricmp(buffer, "MLReplyIntro"))   stccpy(co->MLReplyIntro, value2, SIZE_INTRO);
               if (!stricmp(buffer, "MLReplyBye"))     stccpy(co->MLReplyBye, value2, SIZE_INTRO);
               if (!stricmp(buffer, "ForwardIntro"))   stccpy(co->ForwardIntro, value2, SIZE_INTRO);
               if (!stricmp(buffer, "ForwardFinish"))  stccpy(co->ForwardFinish, value2, SIZE_INTRO);
               if (!stricmp(buffer, "QuoteMessage"))   co->QuoteMessage = Txt2Bool(value);
               if (!stricmp(buffer, "QuoteText"))      stccpy(co->QuoteText, value2, SIZE_SMALL);
               if (!stricmp(buffer, "AltQuoteText"))   stccpy(co->AltQuoteText, value2, SIZE_SMALL);
               if (!stricmp(buffer, "QuoteEmptyLines"))co->QuoteEmptyLines = Txt2Bool(value);
               if (!stricmp(buffer, "CompareAddress")) co->CompareAddress = Txt2Bool(value);
               if (!stricmp(buffer, "StripSignature")) co->StripSignature = Txt2Bool(value);
/*7*/          if (!stricmp(buffer, "UseSignature"))   co->UseSignature = Txt2Bool(value);
               if (!stricmp(buffer, "TagsFile"))       stccpy(co->TagsFile, value, SIZE_PATHFILE);
               if (!stricmp(buffer, "TagsSeparator"))  stccpy(co->TagsSeparator, value2, SIZE_SMALL);
/*8*/          if (!stricmp(buffer, "FolderCols"))     co->FolderCols = atoi(value);
               if (!stricmp(buffer, "MessageCols"))    co->MessageCols = atoi(value);
               if (!stricmp(buffer, "FixedFontList"))  co->FixedFontList = Txt2Bool(value);
               if (!stricmp(buffer, "SwatchBeat"))     co->SwatchBeat = Txt2Bool(value);
               if (!stricmp(buffer, "SizeFormat"))     co->SizeFormat = atoi(value);
               if (!stricmp(buffer, "FolderCntMenu"))  co->FolderCntMenu = Txt2Bool(value);
               if (!stricmp(buffer, "MessageCntMenu")) co->MessageCntMenu = Txt2Bool(value);
               if (!stricmp(buffer, "InfoBar"))        co->InfoBar = atoi(value);
               if (!stricmp(buffer, "InfoBarText"))    stccpy(co->InfoBarText, value, SIZE_DEFAULT);
/*9*/          if (!stricmp(buffer, "PGPCmdPath"))     stccpy(co->PGPCmdPath, value, SIZE_PATH);
               if (!stricmp(buffer, "MyPGPID"))        stccpy(co->MyPGPID, value, SIZE_DEFAULT);
               if (!stricmp(buffer, "EncryptToSelf"))  co->EncryptToSelf = Txt2Bool(value);
               if (!stricmp(buffer, "ReMailer"))       stccpy(co->ReMailer, value, SIZE_ADDRESS);
               if (!stricmp(buffer, "RMCommands"))     stccpy(co->RMCommands, value2, SIZE_INTRO);
               if (!stricmp(buffer, "LogfilePath"))    stccpy(co->LogfilePath, value, SIZE_PATH);
               if (!stricmp(buffer, "LogfileMode"))    co->LogfileMode = atoi(value);
               if (!stricmp(buffer, "SplitLogfile"))   co->SplitLogfile = Txt2Bool(value);
               if (!stricmp(buffer, "LogAllEvents"))   co->LogAllEvents = Txt2Bool(value);
/*10*/         if (!stricmp(buffer, "GetOnStartup"))   co->GetOnStartup = Txt2Bool(value);
               if (!stricmp(buffer, "SendOnStartup"))  co->SendOnStartup = Txt2Bool(value);
               if (!stricmp(buffer, "CleanupOnStartup")) co->CleanupOnStartup = Txt2Bool(value);
               if (!stricmp(buffer, "RemoveOnStartup"))  co->RemoveOnStartup = Txt2Bool(value);
               if (!stricmp(buffer, "LoadAllFolders")) co->LoadAllFolders = Txt2Bool(value);
               if (!stricmp(buffer, "UpdateNewMail"))  co->UpdateNewMail = Txt2Bool(value);
               if (!stricmp(buffer, "CheckBirthdates"))co->CheckBirthdates = Txt2Bool(value);
               if (!stricmp(buffer, "SendOnQuit"))     co->SendOnQuit = Txt2Bool(value);
               if (!stricmp(buffer, "CleanupOnQuit"))  co->CleanupOnQuit = Txt2Bool(value);
               if (!stricmp(buffer, "RemoveOnQuit"))   co->RemoveOnQuit = Txt2Bool(value);
/*11*/         if (!strnicmp(buffer, "MV", 2) && buffer[4] == '.')
               {
                  int j = atoi(&buffer[2]);
                  struct MimeView *mv = co->MV[j];
                  p = &buffer[5];
                  if (!mv) mv = co->MV[j] = CO_NewMimeView();
                  if (!stricmp(p, "ContentType")) stccpy(mv->ContentType, value, SIZE_CTYPE);
                  if (!stricmp(p, "Extension"))   stccpy(mv->Extension, value, SIZE_NAME);
                  if (!stricmp(p, "Command"))     stccpy(mv->Command, value, SIZE_COMMAND);
               }
               if (!stricmp(buffer, "IdentifyBin"))    co->IdentifyBin = Txt2Bool(value);
               if (!stricmp(buffer, "DetachDir"))      stccpy(co->DetachDir, value, SIZE_PATH);
               if (!stricmp(buffer, "AttachDir"))      stccpy(co->AttachDir, value, SIZE_PATH);
/*12*/
               if (!stricmp(buffer, "GalleryDir"))     stccpy(co->GalleryDir, value, SIZE_PATH);
               if (!stricmp(buffer, "MyPictureURL"))   stccpy(co->MyPictureURL, value, SIZE_URL);
               if (!stricmp(buffer, "ProxyServer"))    stccpy(co->ProxyServer, value, SIZE_HOST);
               if (!stricmp(buffer, "NewAddrGroup"))   stccpy(co->NewAddrGroup, value, SIZE_NAME);
               if (!stricmp(buffer, "AddToAddrbook"))  co->AddToAddrbook = atoi(value);
               if (!stricmp(buffer, "AddMyInfo")    )  co->AddMyInfo= Txt2Bool(value);
               if (!stricmp(buffer, "AddrbookCols"))   co->AddrbookCols = atoi(value);
/*13*/         if (!strnicmp(buffer, "Rexx", 4) && buffer[6] == '.')
               {
                  int j = atoi(&buffer[4]);
                  if (j < MAXRX)
                  {
                     p = &buffer[7];
                     if (!stricmp(p, "Name"))       stccpy(co->RX[j].Name, value, SIZE_NAME);
                     if (!stricmp(p, "Script"))     stccpy(co->RX[j].Script, value, SIZE_PATHFILE);
                     if (!stricmp(p, "IsAmigaDOS")) co->RX[j].IsAmigaDOS = Txt2Bool(value);
                     if (!stricmp(p, "UseConsole")) co->RX[j].UseConsole = Txt2Bool(value);
                     if (!stricmp(p, "WaitTerm"))   co->RX[j].WaitTerm = Txt2Bool(value);
                  }
               }
/*14*/         if (!stricmp(buffer, "TempDir"))        stccpy(co->TempDir, value, SIZE_PATH);
               if (!stricmp(buffer, "IconPosition"))   sscanf(value, "%ld;%ld", &(co->IconPositionX), &(co->IconPositionY));
               if (!stricmp(buffer, "IconifyOnQuit"))  co->IconifyOnQuit = Txt2Bool(value);
               if (!stricmp(buffer, "Confirm"))        co->Confirm = Txt2Bool(value);
               if (!stricmp(buffer, "ConfirmDelete"))  co->ConfirmDelete = atoi(value);
               if (!stricmp(buffer, "RemoveAtOnce"))   co->RemoveAtOnce = Txt2Bool(value);
               if (!stricmp(buffer, "SaveSent"))       co->SaveSent = Txt2Bool(value);
               if (!stricmp(buffer, "MDN_Display"))    co->MDN_Display = atoi(value);
               if (!stricmp(buffer, "MDN_Process"))    co->MDN_Process = atoi(value);
               if (!stricmp(buffer, "MDN_Delete"))     co->MDN_Delete = atoi(value);
               if (!stricmp(buffer, "MDN_Filter"))     co->MDN_Filter = atoi(value);
               if (!stricmp(buffer, "SendMDNAtOnce"))  co->SendMDNAtOnce = Txt2Bool(value);
               if (!stricmp(buffer, "XPKPack"))        { stccpy(co->XPKPack, value, 5); co->XPKPackEff = atoi(&value[5]); }
               if (!stricmp(buffer, "XPKPackEncrypt")) { stccpy(co->XPKPackEncrypt, value, 5); co->XPKPackEncryptEff = atoi(&value[5]); }
               if (!stricmp(buffer, "PackerCommand"))  stccpy(co->PackerCommand, value, SIZE_COMMAND);
               if (!stricmp(buffer, "AppIconText"))    stccpy(co->AppIconText, value, SIZE_DEFAULT/2);
/*Hidden*/     if (!stricmp(buffer, "LetterPart"))     co->LetterPart = atoi(value);
               if (!stricmp(buffer, "WriteIndexes"))   co->WriteIndexes = atoi(value);
               if (!stricmp(buffer, "AutoSave"))       co->AutoSave = atoi(value);
               if (!stricmp(buffer, "SupportSite"))    stccpy(co->SupportSite, value, SIZE_HOST);
               if (!stricmp(buffer, "JumpToNewMsg"))   co->JumpToNewMsg = Txt2Bool(value);
							 if (!stricmp(buffer, "JumpToIncoming")) co->JumpToIncoming = Txt2Bool(value);
               if (!stricmp(buffer, "AskJumpUnread"))  co->AskJumpUnread = Txt2Bool(value);
               if (!stricmp(buffer, "PrinterCheck"))   co->PrinterCheck = Txt2Bool(value);
               if (!stricmp(buffer, "IsOnlineCheck"))  co->IsOnlineCheck = Txt2Bool(value);
               if (!stricmp(buffer, "IOCInterface"))   stccpy(co->IOCInterface, value, SIZE_SMALL);
               if (!stricmp(buffer, "ConfirmOnQuit"))  co->ConfirmOnQuit = Txt2Bool(value);
               if (!stricmp(buffer, "HideGUIElements")) co->HideGUIElements = atoi(value);
               if (!stricmp(buffer, "LocalCharset"))   stccpy(co->LocalCharset, value, SIZE_CTYPE);
               if (!stricmp(buffer, "StackSize"))      co->StackSize = atoi(value);
               if (!stricmp(buffer, "PrintMethod"))    co->PrintMethod = atoi(value);
            }
         }
         fclose(fh);
         return TRUE;
      }
      fclose(fh);
   }
   return FALSE;
}

///
/// CO_GetConfig
//  Fills form data of current section with data from configuration structure
void CO_GetConfig(void)
{
   struct CO_GUIData *gui = &G->CO->GUI;
   int i, modified;
   struct MUI_PenSpec *ps;

   switch (G->CO->VisiblePage)
   {
      case 0:
         GetMUIString(CE->RealName        ,gui->ST_REALNAME);
         GetMUIString(CE->EmailAddress    ,gui->ST_EMAIL);
         CE->TimeZone          = GetMUICycle  (gui->CY_TZONE)-12;
         CE->DaylightSaving    = GetMUICheck  (gui->CH_DLSAVING);
         break;
      case 1:
         GetMUIString(CE->SMTP_Server     ,gui->ST_SMTPHOST);
         CE->SMTP_Port = GetMUIInteger(gui->ST_SMTPPORT);
         GetMUIString(CE->SMTP_Domain     ,gui->ST_DOMAIN);
         CE->Allow8bit         = GetMUICheck  (gui->CH_SMTP8BIT);
         CE->Use_SMTP_TLS      = GetMUICheck  (gui->CH_SMTPTLS);
         CE->Use_SMTP_AUTH     = GetMUICheck  (gui->CH_USESMTPAUTH);
         GetMUIString(CE->SMTP_AUTH_User  ,gui->ST_SMTPAUTHUSER);
         GetMUIString(CE->SMTP_AUTH_Pass  ,gui->ST_SMTPAUTHPASS);
         break;
      case 2:
         CE->PreSelection      = GetMUICycle  (gui->CY_MSGSELECT);
         CE->TransferWindow    = GetMUICycle  (gui->CY_TRANSWIN);
         CE->AvoidDuplicates   = GetMUICheck  (gui->CH_AVOIDDUP);
         CE->UpdateStatus      = GetMUICheck  (gui->CH_UPDSTAT);
         CE->WarnSize          = GetMUIInteger(gui->ST_WARNSIZE);
         CE->CheckMailDelay    = GetMUINumer  (gui->NM_INTERVAL);
         CE->DownloadLarge     = GetMUICheck  (gui->CH_DLLARGE);
         CE->NotifyType        = (GetMUICheck(gui->CH_NOTIREQ) ? NOTI_REQ : 0)
                               + (GetMUICheck(gui->CH_NOTISOUND) ? NOTI_SOUND : 0)
                               + (GetMUICheck(gui->CH_NOTICMD) ? NOTI_CMD : 0);
         GetMUIString(CE->NotifySound         ,gui->ST_NOTISOUND);
         GetMUIString(CE->NotifyCommand       ,gui->ST_NOTICMD);
         break;
      case 3:
         for (i = 0; i < MAXRU; i++) DoMethod(gui->LV_RULES, MUIM_List_GetEntry, i, &(CE->RU[i]));
         break;
      case 4:
         CE->ShowHeader        = GetMUICycle  (gui->CY_HEADER);
         GetMUIString(CE->ShortHeaders        ,gui->ST_HEADERS);
         CE->ShowSenderInfo    = GetMUICycle  (gui->CY_SENDERINFO);
         CE->SigSepLine        = GetMUICycle  (gui->CY_SIGSEPLINE);
         get(gui->CA_COLTEXT, MUIA_Pendisplay_Spec, &ps); CE->ColoredText = *ps;
         get(gui->CA_COL2QUOT, MUIA_Pendisplay_Spec, &ps); CE->Color2ndLevel = *ps;
         CE->DisplayAllTexts   = GetMUICheck  (gui->CH_ALLTEXTS);
         CE->FixedFontEdit     = GetMUICheck  (gui->CH_FIXFEDIT);
         CE->WrapHeader        = GetMUICheck  (gui->CH_WRAPHEAD);
         CE->UseTextstyles     = GetMUICheck  (gui->CH_TEXTSTYLES);
         CE->MultipleWindows   = GetMUICheck  (gui->CH_MULTIWIN);
         GetMUIString(CE->TranslationIn       ,gui->ST_INTRANS);
         CE->AutomaticTranslationIn = GetMUICheck(gui->CH_AUTOTRANSLATEIN);
         break;
      case 5:
         GetMUIString(CE->ReplyTo             ,gui->ST_REPLYTO);
         GetMUIString(CE->Organization        ,gui->ST_ORGAN);
         GetMUIString(CE->ExtraHeaders        ,gui->ST_EXTHEADER);
         GetMUIString(CE->NewIntro            ,gui->ST_HELLOTEXT);
         GetMUIString(CE->Greetings           ,gui->ST_BYETEXT);
         GetMUIString(CE->TranslationOut      ,gui->ST_OUTTRANS);
         CE->WarnSubject       = GetMUICheck  (gui->CH_WARNSUBJECT);
         CE->EdWrapCol         = GetMUIInteger(gui->ST_EDWRAP);
         CE->EdWrapMode        = GetMUICycle  (gui->CY_EDWRAP);
         GetMUIString(CE->Editor              ,gui->ST_EDITOR);
         CE->LaunchAlways      = GetMUICheck  (gui->CH_LAUNCH);
         CE->EmailCache        = GetMUINumer  (gui->NB_EMAILCACHE);
         break;
      case 6:
         GetMUIString(CE->ReplyHello          ,gui->ST_REPLYHI);
         GetMUIString(CE->ReplyIntro          ,gui->ST_REPLYTEXT);
         GetMUIString(CE->ReplyBye            ,gui->ST_REPLYBYE);
         GetMUIString(CE->AltReplyHello       ,gui->ST_AREPLYHI);
         GetMUIString(CE->AltReplyIntro       ,gui->ST_AREPLYTEXT);
         GetMUIString(CE->AltReplyBye         ,gui->ST_AREPLYBYE);
         GetMUIString(CE->AltReplyPattern     ,gui->ST_AREPLYPAT);
         GetMUIString(CE->MLReplyHello        ,gui->ST_MREPLYHI);
         GetMUIString(CE->MLReplyIntro        ,gui->ST_MREPLYTEXT);
         GetMUIString(CE->MLReplyBye          ,gui->ST_MREPLYBYE);
         GetMUIString(CE->ForwardIntro        ,gui->ST_FWDSTART);
         GetMUIString(CE->ForwardFinish       ,gui->ST_FWDEND);
         CE->QuoteMessage      = GetMUICheck  (gui->CH_QUOTE);
         GetMUIString(CE->QuoteText           ,gui->ST_REPLYCHAR);
         GetMUIString(CE->AltQuoteText        ,gui->ST_ALTQUOTECHAR);
         CE->QuoteEmptyLines   = GetMUICheck  (gui->CH_QUOTEEMPTY);
         CE->CompareAddress    = GetMUICheck  (gui->CH_COMPADDR);
         CE->StripSignature    = GetMUICheck  (gui->CH_STRIPSIG);
         break;
      case 7:
         CE->UseSignature      = GetMUICheck  (gui->CH_USESIG);
         GetMUIString(CE->TagsFile            ,gui->ST_TAGFILE);
         GetMUIString(CE->TagsSeparator       ,gui->ST_TAGSEP);
         get(gui->TE_SIGEDIT, MUIA_TextEditor_HasChanged, &modified);
         if (modified) EditorToFile(gui->TE_SIGEDIT, CreateFilename(SigNames[G->CO->LastSig]), NULL);
         break;
      case 8:
         CE->FolderCols = 1; for (i = 1; i < FOCOLNUM; i++) if (GetMUICheck(gui->CH_FCOLS[i])) CE->FolderCols += (1<<i);
         CE->MessageCols = 1; for (i = 1; i < MACOLNUM; i++) if (GetMUICheck(gui->CH_MCOLS[i])) CE->MessageCols += (1<<i);
         CE->FixedFontList = GetMUICheck(gui->CH_FIXFLIST);
         CE->SwatchBeat    = GetMUICheck(gui->CH_BEAT);
         CE->SizeFormat    = GetMUICycle(gui->CY_SIZE);
         CE->FolderCntMenu = GetMUICheck(gui->CH_FCNTMENU);
         CE->MessageCntMenu= GetMUICheck(gui->CH_MCNTMENU);
         CE->InfoBar       = GetMUICycle(gui->CY_INFOBAR);
         GetMUIString(CE->InfoBarText, gui->ST_INFOBARTXT);
         break;
      case 9:
         GetMUIString(CE->PGPCmdPath          ,gui->ST_PGPCMD);
         GetMUIString(CE->MyPGPID             ,gui->ST_MYPGPID);
         CE->EncryptToSelf     = GetMUICheck  (gui->CH_ENCSELF);
         GetMUIString(CE->ReMailer            ,gui->ST_REMAILER);
         GetMUIString(CE->RMCommands          ,gui->ST_FIRSTLINE);
         GetMUIString(CE->LogfilePath         ,gui->ST_LOGFILE);
         CE->LogfileMode       = GetMUICycle  (gui->CY_LOGMODE);
         CE->SplitLogfile      = GetMUICheck  (gui->CH_SPLITLOG);
         CE->LogAllEvents      = GetMUICheck  (gui->CH_LOGALL);
         break;
      case 10:
         CE->GetOnStartup      = GetMUICheck  (gui->CH_POPSTART);
         CE->SendOnStartup     = GetMUICheck  (gui->CH_SENDSTART);
         CE->CleanupOnStartup  = GetMUICheck  (gui->CH_DELETESTART);
         CE->RemoveOnStartup   = GetMUICheck  (gui->CH_REMOVESTART);
         CE->LoadAllFolders    = GetMUICheck  (gui->CH_LOADALL);
         CE->UpdateNewMail     = GetMUICheck  (gui->CH_MARKNEW);
         CE->CheckBirthdates   = GetMUICheck  (gui->CH_CHECKBD);
         CE->SendOnQuit        = GetMUICheck  (gui->CH_SENDQUIT);
         CE->CleanupOnQuit     = GetMUICheck  (gui->CH_DELETEQUIT);
         CE->RemoveOnQuit      = GetMUICheck  (gui->CH_REMOVEQUIT);
         break;
      case 11:
         GetMUIString(CE->MV[0]->Command      ,gui->ST_DEFVIEWER);
         CE->IdentifyBin       = GetMUICheck  (gui->CH_IDENTBIN);
         GetMUIString(CE->DetachDir           ,gui->ST_DETACHDIR);
         GetMUIString(CE->AttachDir           ,gui->ST_ATTACHDIR);
         break;
      case 12:
         GetMUIString(CE->GalleryDir          ,gui->ST_GALLDIR);
         GetMUIString(CE->MyPictureURL        ,gui->ST_PHOTOURL);
         GetMUIString(CE->NewAddrGroup        ,gui->ST_NEWGROUP);
         GetMUIString(CE->ProxyServer         ,gui->ST_PROXY);
         CE->AddToAddrbook     = GetMUICycle  (gui->CY_ATAB);
         CE->AddMyInfo         = GetMUICheck  (gui->CH_ADDINFO);
         CE->AddrbookCols = 1; for (i = 1; i < ABCOLNUM; i++) if (GetMUICheck(gui->CH_ACOLS[i])) CE->AddrbookCols += (1<<i);
         break;
      case 14:
         GetMUIString(CE->TempDir             ,gui->ST_TEMPDIR);
         CE->IconPositionX     = GetMUIInteger(gui->ST_APPX);
         CE->IconPositionY     = GetMUIInteger(gui->ST_APPY);
         CE->IconifyOnQuit     = GetMUICheck  (gui->CH_CLGADGET);
         CE->Confirm           = GetMUICheck  (gui->CH_CONFIRM);
         CE->ConfirmDelete     = GetMUINumer  (gui->NB_CONFIRMDEL);
         CE->RemoveAtOnce      = GetMUICheck  (gui->CH_REMOVE);
         CE->SaveSent          = GetMUICheck  (gui->CH_SAVESENT);
         CE->MDN_Display       = GetMUIRadio  (gui->RA_MDN_DISP);
         CE->MDN_Process       = GetMUIRadio  (gui->RA_MDN_PROC);
         CE->MDN_Delete        = GetMUIRadio  (gui->RA_MDN_DELE);
         CE->MDN_Filter        = GetMUIRadio  (gui->RA_MDN_RULE);
         CE->SendMDNAtOnce     = GetMUICheck  (gui->CH_SEND_MDN);
         GetMUIText(CE->XPKPack               ,gui->TX_PACKER);
         GetMUIText(CE->XPKPackEncrypt        ,gui->TX_ENCPACK);
         CE->XPKPackEff        = GetMUINumer  (gui->NB_PACKER);
         CE->XPKPackEncryptEff = GetMUINumer  (gui->NB_ENCPACK);
         GetMUIString(CE->PackerCommand       ,gui->ST_ARCHIVER);
         GetMUIString(CE->AppIconText         ,gui->ST_APPICON);
         break;
   }
}

///
/// CO_SetConfig
//  Sets current section of configuration structure with data from GUI
void CO_SetConfig(void)
{
   struct CO_GUIData *gui = &G->CO->GUI;
   struct POP3 *pop3;
   int i;
   switch (G->CO->VisiblePage)
   {
      case 0:
         setstring(gui->ST_REALNAME  ,CE->RealName);
         setstring(gui->ST_EMAIL     ,CE->EmailAddress);
         setcycle(gui->CY_TZONE, CE->TimeZone+12);
         setcheckmark(gui->CH_DLSAVING  ,CE->DaylightSaving);
         nnset(gui->ST_POPHOST0, MUIA_String_Contents, CE->P3[0]->Server);
         nnset(gui->ST_PASSWD0,  MUIA_String_Contents, CE->P3[0]->Password);
         break;
      case 1:
         setstring   (gui->ST_SMTPHOST  ,CE->SMTP_Server);
         set(gui->ST_SMTPPORT, MUIA_String_Integer, CE->SMTP_Port);
         setstring   (gui->ST_DOMAIN    ,CE->SMTP_Domain);
         setcheckmark(gui->CH_SMTP8BIT  ,CE->Allow8bit);
         setcheckmark(gui->CH_SMTPTLS   ,CE->Use_SMTP_TLS);
         set(gui->CH_SMTPTLS, MUIA_Disabled, !G->TR_UseableTLS);
         setcheckmark(gui->CH_USESMTPAUTH,CE->Use_SMTP_AUTH);
         setstring   (gui->ST_SMTPAUTHUSER,CE->SMTP_AUTH_User);
         setstring   (gui->ST_SMTPAUTHPASS,CE->SMTP_AUTH_Pass);
         DoMethod(gui->LV_POP3, MUIM_List_Clear);
         for (i = 0; i < MAXP3; i++) if (pop3 = CE->P3[i])
         {
            sprintf(pop3->Account, "%s@%s", pop3->User, pop3->Server);
            DoMethod(gui->LV_POP3, MUIM_List_InsertSingle, pop3, MUIV_List_Insert_Bottom);
         }
         break;
      case 2:
         setcheckmark(gui->CH_AVOIDDUP  ,CE->AvoidDuplicates);
         setcycle    (gui->CY_MSGSELECT ,CE->PreSelection);
         setcycle    (gui->CY_TRANSWIN  ,CE->TransferWindow);
         setcheckmark(gui->CH_UPDSTAT   ,CE->UpdateStatus);
         set(gui->ST_WARNSIZE, MUIA_String_Integer, CE->WarnSize);
         set(gui->NM_INTERVAL, MUIA_Numeric_Value,  CE->CheckMailDelay);
         setcheckmark(gui->CH_DLLARGE   ,CE->DownloadLarge);
         setcheckmark(gui->CH_NOTIREQ   ,(CE->NotifyType&NOTI_REQ)!=0);
         setcheckmark(gui->CH_NOTISOUND ,(CE->NotifyType&NOTI_SOUND)!=0);
         setcheckmark(gui->CH_NOTICMD   ,(CE->NotifyType&NOTI_CMD)!=0);
         setstring   (gui->ST_NOTISOUND ,CE->NotifySound);
         setstring   (gui->ST_NOTICMD   ,CE->NotifyCommand);
         break;
      case 3:
         DoMethod(gui->LV_RULES, MUIM_List_Clear);
         for (i = 0; i < MAXRU; i++) if (CE->RU[i]) DoMethod(gui->LV_RULES, MUIM_List_InsertSingle, CE->RU[i], MUIV_List_Insert_Bottom);
         break;
      case 4:
         setcycle    (gui->CY_HEADER    ,CE->ShowHeader);
         setstring   (gui->ST_HEADERS   ,CE->ShortHeaders);
         setcycle    (gui->CY_SENDERINFO,CE->ShowSenderInfo);
         setcycle    (gui->CY_SIGSEPLINE,CE->SigSepLine);
         set(gui->CA_COLTEXT, MUIA_Pendisplay_Spec, &CE->ColoredText);
         set(gui->CA_COL2QUOT, MUIA_Pendisplay_Spec, &CE->Color2ndLevel);
         setcheckmark(gui->CH_ALLTEXTS  ,CE->DisplayAllTexts);
         setcheckmark(gui->CH_FIXFEDIT  ,CE->FixedFontEdit);
         setcheckmark(gui->CH_WRAPHEAD  ,CE->WrapHeader);
         setcheckmark(gui->CH_TEXTSTYLES,CE->UseTextstyles);
         setcheckmark(gui->CH_MULTIWIN  ,CE->MultipleWindows);
         setstring   (gui->ST_INTRANS   ,CE->TranslationIn);
         setcheckmark(gui->CH_AUTOTRANSLATEIN, CE->AutomaticTranslationIn);
         break;
      case 5:
         setstring   (gui->ST_REPLYTO   ,CE->ReplyTo);
         setstring   (gui->ST_ORGAN     ,CE->Organization);
         setstring   (gui->ST_EXTHEADER ,CE->ExtraHeaders);
         setstring   (gui->ST_HELLOTEXT ,CE->NewIntro);
         setstring   (gui->ST_BYETEXT   ,CE->Greetings);
         setstring   (gui->ST_OUTTRANS  ,CE->TranslationOut);
         setcheckmark(gui->CH_WARNSUBJECT,CE->WarnSubject);
         set(gui->ST_EDWRAP, MUIA_String_Integer, CE->EdWrapCol);
         setcycle    (gui->CY_EDWRAP    ,CE->EdWrapMode);
         setstring   (gui->ST_EDITOR    ,CE->Editor);
         setcheckmark(gui->CH_LAUNCH    ,CE->LaunchAlways);
         setslider   (gui->NB_EMAILCACHE,CE->EmailCache);
         break;
      case 6:
         setstring   (gui->ST_REPLYHI     ,CE->ReplyHello);
         setstring   (gui->ST_REPLYTEXT   ,CE->ReplyIntro);
         setstring   (gui->ST_REPLYBYE    ,CE->ReplyBye);
         setstring   (gui->ST_AREPLYHI    ,CE->AltReplyHello);
         setstring   (gui->ST_AREPLYTEXT  ,CE->AltReplyIntro);
         setstring   (gui->ST_AREPLYBYE   ,CE->AltReplyBye);
         setstring   (gui->ST_AREPLYPAT   ,CE->AltReplyPattern);
         setstring   (gui->ST_MREPLYHI    ,CE->MLReplyHello);
         setstring   (gui->ST_MREPLYTEXT  ,CE->MLReplyIntro);
         setstring   (gui->ST_MREPLYBYE   ,CE->MLReplyBye);
         setstring   (gui->ST_FWDSTART    ,CE->ForwardIntro);
         setstring   (gui->ST_FWDEND      ,CE->ForwardFinish);
         setcheckmark(gui->CH_QUOTE       ,CE->QuoteMessage);
         setstring   (gui->ST_REPLYCHAR   ,CE->QuoteText);
         setstring   (gui->ST_ALTQUOTECHAR,CE->AltQuoteText);
         setcheckmark(gui->CH_QUOTEEMPTY  ,CE->QuoteEmptyLines);
         setcheckmark(gui->CH_COMPADDR    ,CE->CompareAddress);
         setcheckmark(gui->CH_STRIPSIG    ,CE->StripSignature);
         break;
      case 7:
         setcheckmark(gui->CH_USESIG      ,CE->UseSignature);
         setstring   (gui->ST_TAGFILE     ,CE->TagsFile);
         setstring   (gui->ST_TAGSEP      ,CE->TagsSeparator);
         setcycle    (gui->CY_SIGNAT      ,G->CO->LastSig);
         FileToEditor(CreateFilename(SigNames[G->CO->LastSig]), gui->TE_SIGEDIT);
         break;
      case 8:
         for (i = 0; i < FOCOLNUM; i++) setcheckmark(gui->CH_FCOLS[i], (CE->FolderCols & (1<<i)) != 0);
         for (i = 0; i < MACOLNUM; i++) setcheckmark(gui->CH_MCOLS[i], (CE->MessageCols & (1<<i)) != 0);
         setcheckmark(gui->CH_FIXFLIST  ,CE->FixedFontList);
         setcheckmark(gui->CH_BEAT      ,CE->SwatchBeat);
         setcycle(gui->CY_SIZE, CE->SizeFormat);
         setcheckmark(gui->CH_FCNTMENU  ,CE->FolderCntMenu);
         setcheckmark(gui->CH_MCNTMENU  ,CE->MessageCntMenu);
         setcycle(gui->CY_INFOBAR,       CE->InfoBar);
         set(gui->CH_FCNTMENU, MUIA_Disabled, !PopupMenuBase);
         set(gui->CH_MCNTMENU, MUIA_Disabled, !PopupMenuBase);
         setstring(gui->ST_INFOBARTXT   ,CE->InfoBarText);
         break;
      case 9:
         setstring   (gui->ST_PGPCMD    ,CE->PGPCmdPath);
         setstring   (gui->ST_MYPGPID   ,CE->MyPGPID);
         setcheckmark(gui->CH_ENCSELF   ,CE->EncryptToSelf);
         setstring   (gui->ST_REMAILER  ,CE->ReMailer);
         setstring   (gui->ST_FIRSTLINE ,CE->RMCommands);
         setstring   (gui->ST_LOGFILE   ,CE->LogfilePath);
         setcycle    (gui->CY_LOGMODE   ,CE->LogfileMode);
         setcheckmark(gui->CH_SPLITLOG  ,CE->SplitLogfile);
         setcheckmark(gui->CH_LOGALL    ,CE->LogAllEvents);
         break;
      case 10:
         setcheckmark(gui->CH_POPSTART   ,CE->GetOnStartup);
         setcheckmark(gui->CH_SENDSTART  ,CE->SendOnStartup);
         setcheckmark(gui->CH_DELETESTART,CE->CleanupOnStartup);
         setcheckmark(gui->CH_REMOVESTART,CE->RemoveOnStartup);
         setcheckmark(gui->CH_LOADALL    ,CE->LoadAllFolders);
         setcheckmark(gui->CH_MARKNEW    ,CE->UpdateNewMail);
         setcheckmark(gui->CH_CHECKBD    ,CE->CheckBirthdates);
         setcheckmark(gui->CH_SENDQUIT   ,CE->SendOnQuit);
         setcheckmark(gui->CH_DELETEQUIT ,CE->CleanupOnQuit);
         setcheckmark(gui->CH_REMOVEQUIT ,CE->RemoveOnQuit);
         break;
      case 11:
         DoMethod(gui->LV_MIME, MUIM_List_Clear);
         for (i = 1; i < MAXMV; i++) if (CE->MV[i]) DoMethod(gui->LV_MIME, MUIM_List_InsertSingle, CE->MV[i], MUIV_List_Insert_Bottom);
         setstring   (gui->ST_DEFVIEWER ,CE->MV[0]->Command);
         setcheckmark(gui->CH_IDENTBIN  ,CE->IdentifyBin);
         setstring   (gui->ST_DETACHDIR ,CE->DetachDir);
         setstring   (gui->ST_ATTACHDIR ,CE->AttachDir);
         break;
      case 12:
         setstring   (gui->ST_GALLDIR   ,CE->GalleryDir);
         setstring   (gui->ST_PHOTOURL  ,CE->MyPictureURL);
         setstring   (gui->ST_NEWGROUP  ,CE->NewAddrGroup);
         setstring   (gui->ST_PROXY     ,CE->ProxyServer);
         setcycle    (gui->CY_ATAB      ,CE->AddToAddrbook);
         setcheckmark(gui->CH_ADDINFO   ,CE->AddMyInfo);
         for (i = 0; i < ABCOLNUM; i++) setcheckmark(gui->CH_ACOLS[i], (CE->AddrbookCols & (1<<i)) != 0);
         break;
      case 13:
         set(G->CO->GUI.LV_REXX, MUIA_List_Active, 0);
         break;
      case 14:
         setstring   (gui->ST_TEMPDIR   ,CE->TempDir);
         set(gui->ST_APPX, MUIA_String_Integer, CE->IconPositionX);
         set(gui->ST_APPY, MUIA_String_Integer, CE->IconPositionY);
         setcheckmark(gui->CH_CLGADGET  ,CE->IconifyOnQuit);
         setcheckmark(gui->CH_CONFIRM   ,CE->Confirm);
         setslider   (gui->NB_CONFIRMDEL,CE->ConfirmDelete);
         setcheckmark(gui->CH_REMOVE    ,CE->RemoveAtOnce);
         setcheckmark(gui->CH_SAVESENT  ,CE->SaveSent);
         setmutex    (gui->RA_MDN_DISP  ,CE->MDN_Display);
         setmutex    (gui->RA_MDN_PROC  ,CE->MDN_Process);
         setmutex    (gui->RA_MDN_DELE  ,CE->MDN_Delete);
         setmutex    (gui->RA_MDN_RULE  ,CE->MDN_Filter);
         setcheckmark(gui->CH_SEND_MDN  ,CE->SendMDNAtOnce);
         set(gui->TX_PACKER , MUIA_Text_Contents, CE->XPKPack);
         set(gui->TX_ENCPACK, MUIA_Text_Contents, CE->XPKPackEncrypt);
         setslider   (gui->NB_PACKER    ,CE->XPKPackEff);
         setslider   (gui->NB_ENCPACK   ,CE->XPKPackEncryptEff);
         setstring   (gui->ST_ARCHIVER  ,CE->PackerCommand);
         setstring   (gui->ST_APPICON   ,CE->AppIconText);
         break;
   }
}
///
