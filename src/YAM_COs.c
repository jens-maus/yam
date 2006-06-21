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

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include <clib/alib_protos.h>
#include <mui/TextEditor_mcc.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/locale.h>
#include <proto/utility.h>

#include "extra.h"
#include "YAM.h"
#include "YAM_config.h"
#include "YAM_error.h"
#include "YAM_folderconfig.h"
#include "YAM_global.h"
#include "YAM_locale.h"
#include "YAM_main.h"
#include "YAM_mime.h"
#include "YAM_utilities.h"

#include "Debug.h"

/***************************************************************************
 Module: Configuration - Basic Get/Put routines
***************************************************************************/

/// MUIStyle2String()
// converts a MUI style string which contains common \033 sequences into a
// human-readable form which we can save to our configuration file.
char *MUIStyle2String(const char *style)
{
  static char buf[SIZE_SMALL];
  char *s = (char *)style;

  ENTER();

  // clear the string first
  buf[0] = '\0';

  // Now we have to identify each \033 sequence
  // in our source string
  while(*s)
  {
    char *e;

    if((e = strpbrk(s, "\033")))
    {
      if(e[1] == 'b') // MUIX_B
        strlcat(buf, "b:", sizeof(buf));
      else if(e[1] == 'i') // MUIX_I
        strlcat(buf, "i:", sizeof(buf));
      else if(e[1] == 'u') // MUIX_B
        strlcat(buf, "u:", sizeof(buf));
      else if(e[1] >= '2' || e[1] <= '9')
        snprintf(buf, sizeof(buf), "%s$%c:", buf, e[1]);

      s = ++e;
    }
    else
      break;
  }

  // strip the last ':' if it is there.
  if(strlen(buf) > 0 && buf[strlen(buf)-1] == ':')
    buf[strlen(buf)-1] = '\0';

  LEAVE();
  return buf;
}
///
/// String2MUIStyle()
// converts a string with style definitions into a MUI style
// conform string with \033 sequences
void String2MUIStyle(const char *string, char *muistr)
{
  char *s = (char *)string;

  ENTER();

  // clear the muistr first
  muistr[0] = '\0';

  // Now we have to identify each style tag one by one
  while(*s)
  {
    char *e;
    char c;

    if((e = strpbrk(s, ": |;,")) == NULL)
      e = s+strlen(s);

    c = tolower(*s);

    if(c == 'b') // MUIX_B
      strlcat(muistr, MUIX_B, SIZE_SMALL);
    else if(c == 'i') // MUIX_I
      strlcat(muistr, MUIX_I, SIZE_SMALL);
    else if(c == 'u') // MUIX_U
      strlcat(muistr, MUIX_U, SIZE_SMALL);
    else if(c == '$' && (s[1] >= '2' || s[1] <= '9')) // screen pen number (2..9)
      snprintf(muistr, SIZE_SMALL, "%s\033%c", muistr, s[1]);

    // set the next start to our last search
    if(*e)
      s = ++e;
    else
      break;
  }

  LEAVE();
}
///
/// MapTZ
//
static int MapTZ(int value, BOOL forward)
{
  static const struct
  {
    int CY_TZONE; // the number in the cycle gadget
    int GMTOffset;// GMT offset in minutes
  } tzmap[] =
  {
    {  0, -720 }, // GMT-12:00
    {  1, -660 }, // GMT-11:00
    {  2, -600 }, // GMT-10:00
    {  3, -540 }, // GMT-09:00
    {  4, -480 }, // GMT-08:00
    {  5, -420 }, // GMT-07:00
    {  6, -360 }, // GMT-06:00
    {  7, -300 }, // GMT-05:00
    {  8, -240 }, // GMT-04:00
    {  9, -210 }, // GMT-03:30
    { 10, -180 }, // GMT-03:00
    { 11, -120 }, // GMT-02:00
    { 12,  -60 }, // GMT-01:00
    { 13,    0 }, // GMT 00:00
    { 14,   60 }, // GMT+01:00
    { 15,  120 }, // GMT+02:00
    { 16,  180 }, // GMT+03:00
    { 17,  210 }, // GMT+03:30
    { 18,  240 }, // GMT+04:00
    { 19,  270 }, // GMT+04:30
    { 20,  300 }, // GMT+05:00
    { 21,  330 }, // GMT+05:30
    { 22,  345 }, // GMT+05:45
    { 23,  360 }, // GMT+06:00
    { 24,  390 }, // GMT+06:30
    { 25,  420 }, // GMT+07:00
    { 26,  480 }, // GMT+08:00
    { 27,  540 }, // GMT+09:00
    { 28,  570 }, // GMT+09:30
    { 29,  600 }, // GMT+10:00
    { 30,  660 }, // GMT+11:00
    { 31,  720 }, // GMT+12:00
    { 32,  780 }  // GMT+13:00
  };

  // depending on the mode we do a forward/backward mapping
  if(forward)
  {
    // we can do a direct mapping
    if(value >= 0 && value <= 32) return tzmap[value].GMTOffset;
  }
  else
  {
    // here we have to iterate through our array
    if(value == 0 || value >= 12 || value <= -12)
    {
      int i;
      for(i=0; i <= 32; i++)
      {
        if(tzmap[i].GMTOffset == value) return i;
      }
    }
    else
    {
      // if we end up here we have probably an old type TimeZone Mapping of YAM 2.3
      return value-12;
    }
  }

  // minus 1000 can NEVER be because there is no
  // index of minus 1000 or GMT value
  return -1000;
}
///
/// CO_SaveConfig
//  Saves configuration to a file
void CO_SaveConfig(struct Config *co, char *fname)
{
   FILE *fh;

   if((fh = fopen(fname, "w")))
   {
      int i;
      char buf[SIZE_LARGE];
      struct MinNode *curNode;

      fprintf(fh, "YCO3 - YAM Configuration\n");
      fprintf(fh, "# generated by '%s (%s)'\n", yamversion, yamversiondate);

      fprintf(fh, "\n[First steps]\n");
      fprintf(fh, "RealName         = %s\n", co->RealName);
      fprintf(fh, "EmailAddress     = %s\n", co->EmailAddress);
      fprintf(fh, "TimeZone         = %d\n", co->TimeZone);
      fprintf(fh, "DaylightSaving   = %s\n", Bool2Txt(co->DaylightSaving));
      fprintf(fh, "LocalCharset     = %s\n", co->LocalCharset);
      fprintf(fh, "DetectCyrillic   = %s\n", Bool2Txt(co->DetectCyrillic));

      fprintf(fh, "\n[TCP/IP]\n");
      fprintf(fh, "SMTP-Server      = %s\n", co->SMTP_Server);
      fprintf(fh, "SMTP-Port        = %d\n", co->SMTP_Port);
      fprintf(fh, "SMTP-Domain      = %s\n", co->SMTP_Domain);
      fprintf(fh, "SMTP-SecMethod   = %d\n", co->SMTP_SecureMethod);
      fprintf(fh, "Allow8bit        = %s\n", Bool2Txt(co->Allow8bit));
      fprintf(fh, "Use-SMTP-AUTH    = %s\n", Bool2Txt(co->Use_SMTP_AUTH));
      fprintf(fh, "SMTP-AUTH-User   = %s\n", co->SMTP_AUTH_User);
      fprintf(fh, "SMTP-AUTH-Pass   = %s\n", Encrypt(co->SMTP_AUTH_Pass));
      for (i = 0; i < MAXP3; i++) if (co->P3[i])
      {
         struct POP3 *p3 = co->P3[i];
         fprintf(fh, "POP%02d.Server     = %s\n", i, p3->Server);
         fprintf(fh, "POP%02d.Port       = %d\n", i, p3->Port);
         fprintf(fh, "POP%02d.User       = %s\n", i, p3->User);
         fprintf(fh, "POP%02d.Password   = %s\n", i, Encrypt(p3->Password));
         fprintf(fh, "POP%02d.Enabled    = %s\n", i, Bool2Txt(p3->Enabled));
         fprintf(fh, "POP%02d.SSLMode    = %d\n", i, p3->SSLMode);
         fprintf(fh, "POP%02d.UseAPOP    = %s\n", i, Bool2Txt(p3->UseAPOP));
         fprintf(fh, "POP%02d.Delete     = %s\n", i, Bool2Txt(p3->DeleteOnServer));
      }

      fprintf(fh, "\n[New mail]\n");
      fprintf(fh, "AvoidDuplicates  = %s\n", Bool2Txt(co->AvoidDuplicates));
      fprintf(fh, "PreSelection     = %d\n", co->PreSelection);
      fprintf(fh, "TransferWindow   = %d\n", co->TransferWindow);
      fprintf(fh, "UpdateStatus     = %s\n", Bool2Txt(co->UpdateStatus));
      fprintf(fh, "WarnSize         = %d\n", co->WarnSize);
      fprintf(fh, "CheckMailDelay   = %d\n", co->CheckMailDelay);
      fprintf(fh, "DownloadLarge    = %s\n", Bool2Txt(co->DownloadLarge));
      fprintf(fh, "NotifyType       = %d\n", co->NotifyType);
      fprintf(fh, "NotifySound      = %s\n", co->NotifySound);
      fprintf(fh, "NotifyCommand    = %s\n", co->NotifyCommand);

      fprintf(fh, "\n[Filters]\n");

      // we iterate through our filter list and save out the whole filter
      // configuration accordingly.
      for(i=0, curNode = co->filterList.mlh_Head; curNode->mln_Succ; curNode = curNode->mln_Succ, i++)
      {
        int j;
        struct FilterNode *filter = (struct FilterNode *)curNode;
        struct MinNode *curRuleNode;

        fprintf(fh, "FI%02d.Name        = %s\n", i, filter->name);
        fprintf(fh, "FI%02d.Remote      = %s\n", i, Bool2Txt(filter->remote));
        fprintf(fh, "FI%02d.ApplyToNew  = %s\n", i, Bool2Txt(filter->applyToNew));
        fprintf(fh, "FI%02d.ApplyToSent = %s\n", i, Bool2Txt(filter->applyToSent));
        fprintf(fh, "FI%02d.ApplyOnReq  = %s\n", i, Bool2Txt(filter->applyOnReq));

        // now we do have to iterate through our ruleList
        for(j=0, curRuleNode = filter->ruleList.mlh_Head; curRuleNode->mln_Succ; curRuleNode = curRuleNode->mln_Succ, j++)
        {
          struct RuleNode *rule = (struct RuleNode *)curRuleNode;

          // we treat the first rule a bit different for compatibility
          // reasons.
          if(j == 0)
          {
            fprintf(fh, "FI%02d.Field       = %d\n", i, rule->searchMode);
            fprintf(fh, "FI%02d.SubField    = %d\n", i, rule->subSearchMode);
            fprintf(fh, "FI%02d.CustomField = %s\n", i, rule->customField);
            fprintf(fh, "FI%02d.Comparison  = %d\n", i, rule->comparison);
            fprintf(fh, "FI%02d.Match       = %s\n", i, rule->matchPattern);
            fprintf(fh, "FI%02d.CaseSens    = %s\n", i, Bool2Txt(rule->caseSensitive));
            fprintf(fh, "FI%02d.Substring   = %s\n", i, Bool2Txt(rule->subString));
          }
          else
          {
            // we handle the combine string different as it relates
            // to the previous one
            if(j > 1)
              fprintf(fh, "FI%02d.Combine%d    = %d\n", i, j, rule->combine);
            else
              fprintf(fh, "FI%02d.Combine     = %d\n", i, rule->combine);

            fprintf(fh, "FI%02d.Field%d      = %d\n", i, j+1, rule->searchMode);
            fprintf(fh, "FI%02d.SubField%d   = %d\n", i, j+1, rule->subSearchMode);
            fprintf(fh, "FI%02d.CustomField%d= %s\n", i, j+1, rule->customField);
            fprintf(fh, "FI%02d.Comparison%d = %d\n", i, j+1, rule->comparison);
            fprintf(fh, "FI%02d.Match%d      = %s\n", i, j+1, rule->matchPattern);
            fprintf(fh, "FI%02d.CaseSens%d   = %s\n", i, j+1, Bool2Txt(rule->caseSensitive));
            fprintf(fh, "FI%02d.Substring%d  = %s\n", i, j+1, Bool2Txt(rule->subString));
          }
        }

        fprintf(fh, "FI%02d.Actions     = %d\n", i, filter->actions);
        fprintf(fh, "FI%02d.BounceTo    = %s\n", i, filter->bounceTo);
        fprintf(fh, "FI%02d.ForwardTo   = %s\n", i, filter->forwardTo);
        fprintf(fh, "FI%02d.ReplyFile   = %s\n", i, filter->replyFile);
        fprintf(fh, "FI%02d.ExecuteCmd  = %s\n", i, filter->executeCmd);
        fprintf(fh, "FI%02d.PlaySound   = %s\n", i, filter->playSound);
        fprintf(fh, "FI%02d.MoveTo      = %s\n", i, filter->moveTo);
      }

      fprintf(fh, "\n[Read]\n");
      fprintf(fh, "ShowHeader       = %d\n", co->ShowHeader);
      fprintf(fh, "ShortHeaders     = %s\n", co->ShortHeaders);
      fprintf(fh, "ShowSenderInfo   = %d\n", co->ShowSenderInfo);
      fprintf(fh, "WrapHeader       = %s\n", Bool2Txt(co->WrapHeader));
      fprintf(fh, "SigSepLine       = %d\n", co->SigSepLine);
      fprintf(fh, "ColoredText      = %s\n", co->ColoredText.buf);
      fprintf(fh, "Color1stLevel    = %s\n", co->Color1stLevel.buf);
      fprintf(fh, "Color2ndLevel    = %s\n", co->Color2ndLevel.buf);
      fprintf(fh, "Color3rdLevel    = %s\n", co->Color3rdLevel.buf);
      fprintf(fh, "Color4thLevel    = %s\n", co->Color4thLevel.buf);
      fprintf(fh, "ColorURL         = %s\n", co->ColorURL.buf);
      fprintf(fh, "DisplayAllTexts  = %s\n", Bool2Txt(co->DisplayAllTexts));
      fprintf(fh, "FixedFontEdit    = %s\n", Bool2Txt(co->FixedFontEdit));
      fprintf(fh, "UseTextstyles    = %s\n", Bool2Txt(co->UseTextstyles));
      fprintf(fh, "MultipleWindows  = %s\n", Bool2Txt(co->MultipleWindows));
      fprintf(fh, "EmbeddedReadPane = %s\n", Bool2Txt(co->EmbeddedReadPane));
      fprintf(fh, "StatusChangeDelay= %d\n", co->StatusChangeDelayOn ? co->StatusChangeDelay : -co->StatusChangeDelay);

      fprintf(fh, "\n[Write]\n");
      fprintf(fh, "ReplyTo          = %s\n", co->ReplyTo);
      fprintf(fh, "Organization     = %s\n", co->Organization);
      fprintf(fh, "ExtraHeaders     = %s\n", co->ExtraHeaders);
      fprintf(fh, "NewIntro         = %s\n", co->NewIntro);
      fprintf(fh, "Greetings        = %s\n", co->Greetings);
      fprintf(fh, "WarnSubject      = %s\n", Bool2Txt(co->WarnSubject));
      fprintf(fh, "EdWrapCol        = %d\n", co->EdWrapCol);
      fprintf(fh, "EdWrapMode       = %d\n", co->EdWrapMode);
      fprintf(fh, "Editor           = %s\n", co->Editor);
      fprintf(fh, "LaunchAlways     = %s\n", Bool2Txt(co->LaunchAlways));
      fprintf(fh, "EmailCache       = %d\n", co->EmailCache);
      fprintf(fh, "AutoSave         = %d\n", co->AutoSave);

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
      fprintf(fh, "FolderCols       = %d\n", co->FolderCols);
      fprintf(fh, "MessageCols      = %d\n", co->MessageCols);
      fprintf(fh, "FixedFontList    = %s\n", Bool2Txt(co->FixedFontList));
      fprintf(fh, "SwatchBeat       = %s\n", Bool2Txt(co->SwatchBeat));
      fprintf(fh, "SizeFormat       = %d\n", co->SizeFormat);
      fprintf(fh, "FolderCntMenu    = %s\n", Bool2Txt(co->FolderCntMenu));
      fprintf(fh, "MessageCntMenu   = %s\n", Bool2Txt(co->MessageCntMenu));
      fprintf(fh, "InfoBar          = %d\n", co->InfoBar);
      fprintf(fh, "InfoBarText      = %s\n", co->InfoBarText);
      fprintf(fh, "QuickSearchBar   = %s\n", Bool2Txt(co->QuickSearchBar));

      fprintf(fh, "\n[Security]\n");
      fprintf(fh, "PGPCmdPath       = %s\n", co->PGPCmdPath);
      fprintf(fh, "MyPGPID          = %s\n", co->MyPGPID);
      fprintf(fh, "EncryptToSelf    = %s\n", Bool2Txt(co->EncryptToSelf));
      fprintf(fh, "ReMailer         = %s\n", co->ReMailer);
      fprintf(fh, "RMCommands       = %s\n", co->RMCommands);
      fprintf(fh, "LogfilePath      = %s\n", co->LogfilePath);
      fprintf(fh, "LogfileMode      = %d\n", co->LogfileMode);
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
      fprintf(fh, "MV00.ContentType = Default\n");
      fprintf(fh, "MV00.Command     = %s\n", C->DefaultMimeViewer);

      for(i=1, curNode = C->mimeTypeList.mlh_Head; curNode->mln_Succ; curNode = curNode->mln_Succ, i++)
      {
        struct MimeTypeNode *mtNode = (struct MimeTypeNode *)curNode;

        fprintf(fh, "MV%02d.ContentType = %s\n", i, mtNode->ContentType);
        fprintf(fh, "MV%02d.Extension   = %s\n", i, mtNode->Extension);
        fprintf(fh, "MV%02d.Command     = %s\n", i, mtNode->Command);
        fprintf(fh, "MV%02d.Description = %s\n", i, mtNode->Description);
      }
      fprintf(fh, "DetachDir        = %s\n", co->DetachDir);
      fprintf(fh, "AttachDir        = %s\n", co->AttachDir);

      fprintf(fh, "\n[Address book]\n");
      fprintf(fh, "GalleryDir       = %s\n", co->GalleryDir);
      fprintf(fh, "MyPictureURL     = %s\n", co->MyPictureURL);
      fprintf(fh, "ProxyServer      = %s\n", co->ProxyServer);
      fprintf(fh, "NewAddrGroup     = %s\n", co->NewAddrGroup);
      fprintf(fh, "AddToAddrbook    = %d\n", co->AddToAddrbook);
      fprintf(fh, "AddMyInfo        = %s\n", Bool2Txt(co->AddMyInfo));
      fprintf(fh, "AddrbookCols     = %d\n", co->AddrbookCols);

      fprintf(fh, "\n[Scripts]\n");
      for (i = 0; i < MAXRX; i++)
      {
         if(i < 10)
         {
           fprintf(fh, "Rexx%02d.Name      = %s\n", i, co->RX[i].Name);
         }
         fprintf(fh, "Rexx%02d.Script    = %s\n", i, co->RX[i].Script);
         fprintf(fh, "Rexx%02d.IsAmigaDOS= %s\n", i, Bool2Txt(co->RX[i].IsAmigaDOS));
         fprintf(fh, "Rexx%02d.UseConsole= %s\n", i, Bool2Txt(co->RX[i].UseConsole));
         fprintf(fh, "Rexx%02d.WaitTerm  = %s\n", i, Bool2Txt(co->RX[i].WaitTerm));
      }

      fprintf(fh, "\n[Mixed]\n");
      fprintf(fh, "TempDir          = %s\n", co->TempDir);
      fprintf(fh, "WBAppIcon        = %s\n", Bool2Txt(co->WBAppIcon));
      fprintf(fh, "IconPosition     = %d;%d\n", co->IconPositionX, co->IconPositionY);
      fprintf(fh, "AppIconText      = %s\n", co->AppIconText);
      fprintf(fh, "DockyIcon        = %s\n", Bool2Txt(co->DockyIcon));
      fprintf(fh, "IconifyOnQuit    = %s\n", Bool2Txt(co->IconifyOnQuit));
      fprintf(fh, "Confirm          = %s\n", Bool2Txt(co->Confirm));
      fprintf(fh, "ConfirmDelete    = %d\n", co->ConfirmDelete);
      fprintf(fh, "RemoveAtOnce     = %s\n", Bool2Txt(co->RemoveAtOnce));
      fprintf(fh, "SaveSent         = %s\n", Bool2Txt(co->SaveSent));
      fprintf(fh, "MDN_Display      = %d\n", co->MDN_Display);
      fprintf(fh, "MDN_Process      = %d\n", co->MDN_Process);
      fprintf(fh, "MDN_Delete       = %d\n", co->MDN_Delete);
      fprintf(fh, "MDN_Filter       = %d\n", co->MDN_Filter);
      fprintf(fh, "SendMDNAtOnce    = %s\n", Bool2Txt(co->SendMDNAtOnce));
      fprintf(fh, "XPKPack          = %s;%d\n", co->XPKPack, co->XPKPackEff);
      fprintf(fh, "XPKPackEncrypt   = %s;%d\n", co->XPKPackEncrypt, co->XPKPackEncryptEff);
      fprintf(fh, "PackerCommand    = %s\n", co->PackerCommand);

      fprintf(fh, "\n[Update]\n");
      fprintf(fh, "UpdateInterval   = %d\n", co->UpdateInterval);
      fprintf(fh, "UpdateServer     = %s\n", co->UpdateServer);
      TimeVal2String(buf, sizeof(buf), &co->LastUpdateCheck, DSS_USDATETIME, TZC_NONE);
      fprintf(fh, "LastUpdateCheck  = %s\n", buf);
      fprintf(fh, "LastUpdateStatus = %d\n", co->LastUpdateStatus);

      fprintf(fh, "\n[Advanced]\n");
      fprintf(fh, "LetterPart       = %d\n", co->LetterPart);
      fprintf(fh, "WriteIndexes     = %d\n", co->WriteIndexes);
      fprintf(fh, "SupportSite      = %s\n", co->SupportSite);
      fprintf(fh, "JumpToNewMsg     = %s\n", Bool2Txt(co->JumpToNewMsg));
      fprintf(fh, "JumpToIncoming   = %s\n", Bool2Txt(co->JumpToIncoming));
      fprintf(fh, "AskJumpUnread    = %s\n", Bool2Txt(co->AskJumpUnread));
      fprintf(fh, "PrinterCheck     = %s\n", Bool2Txt(co->PrinterCheck));
      fprintf(fh, "IsOnlineCheck    = %s\n", Bool2Txt(co->IsOnlineCheck));
      fprintf(fh, "IOCInterface     = %s\n", co->IOCInterface);
      fprintf(fh, "ConfirmOnQuit    = %s\n", Bool2Txt(co->ConfirmOnQuit));
      fprintf(fh, "HideGUIElements  = %d\n", co->HideGUIElements);
      fprintf(fh, "SysCharsetCheck  = %s\n", Bool2Txt(co->SysCharsetCheck));
      fprintf(fh, "AmiSSLCheck      = %s\n", Bool2Txt(co->AmiSSLCheck));
      fprintf(fh, "TimeZoneCheck    = %s\n", Bool2Txt(co->TimeZoneCheck));
      fprintf(fh, "AutoDSTCheck     = %s\n", Bool2Txt(co->AutoDSTCheck));
      fprintf(fh, "StackSize        = %d\n", co->StackSize);
      fprintf(fh, "PrintMethod      = %d\n", co->PrintMethod);
      fprintf(fh, "AutoColumnResize = %s\n", Bool2Txt(co->AutoColumnResize));

      // prepare the socket option string
      buf[0] = '\0'; // clear it first
      if(co->SocketOptions.KeepAlive)
        strlcat(buf, " SO_KEEPALIVE", sizeof(buf));
      if(co->SocketOptions.NoDelay)
        strlcat(buf, " TCP_NODELAY", sizeof(buf));
      if(co->SocketOptions.LowDelay)
        strlcat(buf, " IPTOS_LOWDELAY", sizeof(buf));
      if(co->SocketOptions.SendBuffer > -1)
        snprintf(&buf[strlen(buf)], sizeof(buf)-strlen(buf), " SO_SNDBUF=%ld", co->SocketOptions.SendBuffer);
      if(co->SocketOptions.RecvBuffer > -1)
        snprintf(&buf[strlen(buf)], sizeof(buf)-strlen(buf), " SO_RCVBUF=%ld", co->SocketOptions.RecvBuffer);
      if(co->SocketOptions.SendLowAt > -1)
        snprintf(&buf[strlen(buf)], sizeof(buf)-strlen(buf), " SO_SNDLOWAT=%ld", co->SocketOptions.SendLowAt);
      if(co->SocketOptions.RecvLowAt > -1)
        snprintf(&buf[strlen(buf)], sizeof(buf)-strlen(buf), " SO_RCVLOWAT=%ld", co->SocketOptions.RecvLowAt);
      if(co->SocketOptions.SendTimeOut > -1)
        snprintf(&buf[strlen(buf)], sizeof(buf)-strlen(buf), " SO_SNDTIMEO=%ld", co->SocketOptions.SendTimeOut);
      if(co->SocketOptions.RecvTimeOut > -1)
        snprintf(&buf[strlen(buf)], sizeof(buf)-strlen(buf), " SO_RCVTIMEO=%ld", co->SocketOptions.RecvTimeOut);

      fprintf(fh, "SocketOptions    =%s\n", buf);
      fprintf(fh, "TRBufferSize     = %d\n", co->TRBufferSize);
      fprintf(fh, "EmbeddedMailDelay= %d\n", co->EmbeddedMailDelay);
      fprintf(fh, "KeepAliveInterval= %d\n", co->KeepAliveInterval);
      fprintf(fh, "StyleFGroupUnread= %s\n", MUIStyle2String(co->StyleFGroupUnread));
      fprintf(fh, "StyleFGroupRead  = %s\n", MUIStyle2String(co->StyleFGroupRead));
      fprintf(fh, "StyleFolderUnread= %s\n", MUIStyle2String(co->StyleFolderUnread));
      fprintf(fh, "StyleFolderRead  = %s\n", MUIStyle2String(co->StyleFolderRead));
      fprintf(fh, "StyleFolderNew   = %s\n", MUIStyle2String(co->StyleFolderNew));
      fprintf(fh, "StyleMailUnread  = %s\n", MUIStyle2String(co->StyleMailUnread));
      fprintf(fh, "StyleMailRead    = %s\n", MUIStyle2String(co->StyleMailRead));

      fclose(fh);
      AppendLogVerbose(60, GetStr(MSG_LOG_SavingConfig), fname, "", "", "");
   }
   else ER_NewError(GetStr(MSG_ER_CantCreateFile), fname);
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

   if ((fh = fopen(fname, "r")))
   {
      fgets(buffer, SIZE_LARGE, fh);
      if (!strnicmp(buffer, "YCO", 3))
      {
         version = buffer[3]-'0';
         CO_SetDefaults(co, -1);

         while (fgets(buffer, SIZE_LARGE, fh))
         {
            char *p, *value, *value2 = "";

            if ((value = strchr(buffer, '='))) for (value2 = (++value)+1; isspace(*value); value++);
            if ((p = strpbrk(buffer,"\r\n"))) *p = 0;
            for (p = buffer; *p && *p != '=' && !isspace(*p); p++); *p = 0;

            if (*buffer && value)
            {
               // check for an old config version and try to import its values
               if (version == 2)
               {
                  if (!stricmp(buffer, "POP3-Server"))         strlcpy(co->P3[0]->Server, value, sizeof(co->P3[0]->Server));
                  else if (!stricmp(buffer, "POP3-Password"))  strlcpy(co->P3[0]->Password, Decrypt(value), sizeof(co->P3[0]->Password));
                  else if (!stricmp(buffer, "POP3-User"))      strlcpy(co->P3[0]->User, value, sizeof(co->P3[0]->User));
                  else if (!stricmp(buffer, "DeleteOnServer")) co->P3[0]->DeleteOnServer = Txt2Bool(value);
                  else if (!stricmp(buffer, "CheckMail"))      { if (!Txt2Bool(value)) co->CheckMailDelay = 0; }
                  else if (!stricmp(buffer, "ConfirmSize"))
                  {
                    switch (atoi(value))
                    {
                      case  0: co->PreSelection = 2; co->WarnSize = 0; break;
                      case 13: co->PreSelection = 0; co->WarnSize = 0; break;
                      default: co->PreSelection = 1; co->WarnSize = 1<<atoi(value); break;
                    }
                  }
                  else if (!stricmp(buffer, "Verbosity"))      co->TransferWindow = atoi(value) > 0 ? 2 : 0;
                  else if (!stricmp(buffer, "WordWrap"))       co->EdWrapCol = atoi(value);
                  else if (!stricmp(buffer, "DeleteOnExit"))   co->RemoveAtOnce = !(co->RemoveOnQuit = Txt2Bool(value));
                  else if (!strnicmp(buffer, "Folder", 6) && oldfolders)
                  {
                     static const int sortconv[4] = { -1, 1, 3, 5 };
                     int j = atoi(&buffer[6]), type;
                     if (!ofo) ofo = *oldfolders = calloc(100, sizeof(struct Folder *));
                     if (j >= 3) for (j = 4; j < 100; j++) if (!ofo[j]) break;
                     type = (j == 0 ? FT_INCOMING : (j == 1 ? FT_OUTGOING : (j == 2 ? FT_SENT : FT_CUSTOM)));
                     p = strchr(&value[4], ';'); *p++ = 0;
                     if (!ofo[j]) if ((ofo[j] = FO_NewFolder(type, &value[4], p))) ofo[j]->Sort[0] = sortconv[atoi(&value[2])];
                  }
                  else if (!strnicmp(buffer, "Rule", 4))
                  {
                     struct FilterNode *filter = CreateNewFilter();

                     if(filter)
                     {
                        struct RuleNode *rule;
                        char *p2;

                        filter->applyToNew = filter->applyOnReq = Txt2Bool(value);
                        p = strchr(p2 = &value[2], ';');
                        *p++ = '\0';
                        strlcpy(filter->name, p2, sizeof(filter->name));

                        // get the first rule (always existent) and fill in data
                        rule = (struct RuleNode *)filter->ruleList.mlh_Head;
                        if((rule->searchMode = atoi(p)) == 2)
                          rule->searchMode = SM_SUBJECT;

                        rule->caseSensitive = Txt2Bool(&p[2]);
                        rule->comparison = Txt2Bool(&p[4]) ? CP_NOTEQUAL : CP_EQUAL;
                        p = strchr(p2 = &p[6], ';');
                        *p++ = '\0';
                        strlcpy(rule->matchPattern, p2, sizeof(rule->matchPattern));

                        switch(atoi(p))
                        {
                          case 0: filter->actions = FA_MOVE;    break;
                          case 1: filter->actions = FA_DELETE;  break;
                          case 2: filter->actions = FA_FORWARD; break;
                        }

                        p = strchr(p2 = &p[2], ';');
                        *p++ = '\0';
                        strlcpy(filter->moveTo, p2, sizeof(filter->moveTo));
                        strlcpy(filter->forwardTo, p, sizeof(filter->forwardTo));
                        if(*filter->forwardTo)
                          SET_FLAG(filter->actions, FA_FORWARD);

                        AddTail((struct List *)&co->filterList, (struct Node *)filter);
                     }
                  }
                  else if (!strnicmp(buffer, "MimeViewer", 10))
                  {
                     int j = atoi(&buffer[10]);
                     struct MimeTypeNode *mt;

                     if(j >= 0 && j < 100 && (mt = CreateNewMimeType()))
                     {
                       p = strchr(value, ';');
                       *p++ = '\0';

                       strlcpy(mt->ContentType, value, sizeof(mt->ContentType));
                       strlcpy(mt->Command, p, sizeof(mt->Command));

                       AddTail((struct List *)&(co->mimeTypeList), (struct Node *)mt);
                     }
                  }
                  else if (!strnicmp(buffer, "RexxMenu", 8))
                  {
                     int j = atoi(&buffer[8]);
                     strlcpy(co->RX[j].Name, (char *)FilePart(value), sizeof(co->RX[j].Name));
                     strlcpy(co->RX[j].Script, value, sizeof(co->RX[j].Script));
                  }
               }

               if (!strnicmp(buffer, "FolderPath", 10) && oldfolders)
               {
                  int j = atoi(&buffer[10]);
                  if (!ofo) ofo = *oldfolders = calloc(100, sizeof(struct Folder *));
                  if (!ofo[j]) ofo[j] = FO_NewFolder(FT_CUSTOM, value, (char *)FilePart(value));
                  if (!FO_LoadConfig(ofo[j])) FO_SaveConfig(ofo[j]);
               }
/*0*/          else if (!stricmp(buffer, "RealName"))       strlcpy(co->RealName, value, sizeof(co->RealName));
               else if (!stricmp(buffer, "EmailAddress"))   strlcpy(co->EmailAddress, value, sizeof(co->EmailAddress));
               else if (!stricmp(buffer, "TimeZone"))       co->TimeZone = atoi(value);
               else if (!stricmp(buffer, "DaylightSaving")) co->DaylightSaving = Txt2Bool(value);
               else if (!stricmp(buffer, "LocalCharset"))   strlcpy(co->LocalCharset, value, sizeof(co->LocalCharset));
               else if (!stricmp(buffer, "DetectCyrillic")) co->DetectCyrillic = Txt2Bool(value);
/*1*/          else if (!stricmp(buffer, "SMTP-Server"))    strlcpy(co->SMTP_Server, value, sizeof(co->SMTP_Server));
               else if (!stricmp(buffer, "SMTP-Port"))      co->SMTP_Port = atoi(value);
               else if (!stricmp(buffer, "SMTP-Domain"))    strlcpy(co->SMTP_Domain, value, sizeof(co->SMTP_Domain));
               else if (!stricmp(buffer, "SMTP-SecMethod")) co->SMTP_SecureMethod = atoi(value);
               else if (!stricmp(buffer, "Allow8bit"))      co->Allow8bit = Txt2Bool(value);
               else if (!stricmp(buffer, "Use-SMTP-TLS"))   co->SMTP_SecureMethod = (int)Txt2Bool(value);
               else if (!stricmp(buffer, "Use-SMTP-AUTH"))  co->Use_SMTP_AUTH = Txt2Bool(value);
               else if (!stricmp(buffer, "SMTP-AUTH-User")) strlcpy(co->SMTP_AUTH_User, value, sizeof(co->SMTP_AUTH_User));
               else if (!stricmp(buffer, "SMTP-AUTH-Pass")) strlcpy(co->SMTP_AUTH_Pass, Decrypt(value), sizeof(co->SMTP_AUTH_Pass));
               else if (!strnicmp(buffer, "POP", 3) && buffer[5] == '.')
               {
                  int j = atoi(&buffer[3]);
                  struct POP3 *p3 = co->P3[j];
                  p = &buffer[6];
                  if(!p3)
                    p3 = co->P3[j] = CO_NewPOP3(co, FALSE);

                  if(!stricmp(p, "Server"))          strlcpy(p3->Server, value, sizeof(p3->Server));
                  else if (!stricmp(p, "Port"))      p3->Port = atoi(value);
                  else if (!stricmp(p, "Password"))  strlcpy(p3->Password, Decrypt(value), sizeof(p3->Password));
                  else if (!stricmp(p, "User"))      strlcpy(p3->User, value, sizeof(p3->User));
                  else if (!stricmp(p, "Enabled"))   p3->Enabled = Txt2Bool(value);
                  else if (!stricmp(p, "SSLMode"))   p3->SSLMode = atoi(value);
                  else if (!stricmp(p, "UseAPOP"))   p3->UseAPOP = Txt2Bool(value);
                  else if (!stricmp(p, "Delete"))    p3->DeleteOnServer = Txt2Bool(value);
               }
/*2*/          else if (!stricmp(buffer, "AvoidDuplicates"))co->AvoidDuplicates = Txt2Bool(value);
               else if (!stricmp(buffer, "PreSelection"))   co->PreSelection = atoi(value);
               else if (!stricmp(buffer, "TransferWindow")) co->TransferWindow = atoi(value);
               else if (!stricmp(buffer, "UpdateStatus"))   co->UpdateStatus = Txt2Bool(value);
               else if (!stricmp(buffer, "WarnSize"))       co->WarnSize = atoi(value);
               else if (!stricmp(buffer, "CheckMailDelay")) co->CheckMailDelay = atoi(value);
               else if (!stricmp(buffer, "DownloadLarge"))  co->DownloadLarge = Txt2Bool(value);
               else if (!stricmp(buffer, "NotifyType"))     co->NotifyType = atoi(value);
               else if (!stricmp(buffer, "NotifySound"))    strlcpy(co->NotifySound, value, sizeof(co->NotifySound));
               else if (!stricmp(buffer, "NotifyCommand"))  strlcpy(co->NotifyCommand, value, sizeof(co->NotifyCommand));
/*3*/          else if (!strnicmp(buffer, "FI", 2) && isdigit(buffer[2]) && isdigit(buffer[3]) && strchr(buffer, '.'))
               {
                 static struct FilterNode *lastFilter = NULL;
                 static int lastFilterID = -1;
                 int curFilterID = atoi(&buffer[2]);
                 char *p = strchr(buffer, '.')+1;

                 if(lastFilter && lastFilterID != curFilterID)
                 {
                   int i;
                   struct MinNode *curNode;

                   // reset the lastFilter
                   lastFilter = NULL;
                   lastFilterID = -1;

                   // try to get the filter with that particular filter ID out of our
                   // filterList
                   for(i=0, curNode = co->filterList.mlh_Head; curNode->mln_Succ; curNode = curNode->mln_Succ, i++)
                   {
                     if(i == curFilterID)
                     {
                       lastFilter = (struct FilterNode *)curNode;
                       lastFilterID = i;
                       break;
                     }
                   }
                 }

                 if(lastFilter == NULL)
                 {
                   if((lastFilter = CreateNewFilter()))
                   {
                     AddTail((struct List *)&co->filterList, (struct Node *)lastFilter);
                     lastFilterID = curFilterID;
                   }
                   else
                     break;
                 }

                 // now find out which subtype this filter has
                 if(!stricmp(p, "Name"))                  strlcpy(lastFilter->name, value, sizeof(lastFilter->name));
                 else if(!stricmp(p, "Remote"))           lastFilter->remote = Txt2Bool(value);
                 else if(!stricmp(p, "ApplyToNew"))       lastFilter->applyToNew = Txt2Bool(value);
                 else if(!stricmp(p, "ApplyToSent"))      lastFilter->applyToSent = Txt2Bool(value);
                 else if(!stricmp(p, "ApplyOnReq"))       lastFilter->applyOnReq = Txt2Bool(value);
                 else if(!stricmp(p, "Actions"))          lastFilter->actions = atoi(value);
                 else if(!stricmp(p, "BounceTo"))         strlcpy(lastFilter->bounceTo, value, sizeof(lastFilter->bounceTo));
                 else if(!stricmp(p, "ForwardTo"))        strlcpy(lastFilter->forwardTo, value, sizeof(lastFilter->forwardTo));
                 else if(!stricmp(p, "ReplyFile"))        strlcpy(lastFilter->replyFile, value, sizeof(lastFilter->replyFile));
                 else if(!stricmp(p, "ExecuteCmd"))       strlcpy(lastFilter->executeCmd, value, sizeof(lastFilter->executeCmd));
                 else if(!stricmp(p, "PlaySound"))        strlcpy(lastFilter->playSound, value, sizeof(lastFilter->playSound));
                 else if(!stricmp(p, "MoveTo"))           strlcpy(lastFilter->moveTo, value, sizeof(lastFilter->moveTo));
                 else
                 {
                   struct RuleNode *rule;

                   // if nothing of the above string matched than the FI string
                   // is probably a rule definition so we check it here

                   if(!strnicmp(p, "Field", 5))
                   {
                     int n = atoi(p+5);

                     while(!(rule = GetFilterRule(lastFilter, n>0 ? n-1 : 0)))
                       CreateNewRule(lastFilter);

                     rule->searchMode = atoi(value);
                   }
                   else if(!strnicmp(p, "SubField", 8))
                   {
                     int n = atoi(p+8);

                     while(!(rule = GetFilterRule(lastFilter, n>0 ? n-1 : 0)))
                       CreateNewRule(lastFilter);

                     rule->subSearchMode = atoi(value);
                   }
                   else if(!strnicmp(p, "CustomField", 11))
                   {
                     int n = atoi(p+11);

                     while(!(rule = GetFilterRule(lastFilter, n>0 ? n-1 : 0)))
                       CreateNewRule(lastFilter);

                     strlcpy(rule->customField, value, sizeof(rule->customField));
                   }
                   else if(!strnicmp(p, "Comparison", 10))
                   {
                     int n = atoi(p+10);

                     while(!(rule = GetFilterRule(lastFilter, n>0 ? n-1 : 0)))
                       CreateNewRule(lastFilter);

                     rule->comparison = atoi(value);
                   }
                   else if(!strnicmp(p, "Match", 5))
                   {
                     int n = atoi(p+5);

                     while(!(rule = GetFilterRule(lastFilter, n>0 ? n-1 : 0)))
                       CreateNewRule(lastFilter);

                     strlcpy(rule->matchPattern, value2, sizeof(rule->matchPattern));
                   }
                   else if(!strnicmp(p, "CaseSens", 8))
                   {
                     int n = atoi(p+8);

                     while(!(rule = GetFilterRule(lastFilter, n>0 ? n-1 : 0)))
                       CreateNewRule(lastFilter);

                     rule->caseSensitive = Txt2Bool(value);
                   }
                   else if(!strnicmp(p, "Substring", 9))
                   {
                     int n = atoi(p+9);

                     while(!(rule = GetFilterRule(lastFilter, n>0 ? n-1 : 0)))
                       CreateNewRule(lastFilter);

                     rule->subString = Txt2Bool(value);
                   }
                   else if(!strnicmp(p, "Combine", 7) && atoi(value) > CB_NONE)
                   {
                     int n = atoi(p+7);

                     // here we use n and not n-1 on purpose because the combine line
                     // refers always to the next one.
                     while(!(rule = GetFilterRule(lastFilter, n>0 ? n : 1)))
                       CreateNewRule(lastFilter);

                     rule->combine = atoi(value);
                   }
                 }
               }
/*4*/          else if (!stricmp(buffer, "ShowHeader"))     co->ShowHeader = atoi(value);
               else if (!stricmp(buffer, "ShortHeaders"))   strlcpy(co->ShortHeaders, value, sizeof(co->ShortHeaders));
               else if (!stricmp(buffer, "ShowSenderInfo")) co->ShowSenderInfo = atoi(value);
               else if (!stricmp(buffer, "WrapHeader"))     co->WrapHeader = Txt2Bool(value);
               else if (!stricmp(buffer, "SigSepLine"))     co->SigSepLine = atoi(value);
               else if (!stricmp(buffer, "ColoredText"))    strlcpy(co->ColoredText.buf, value, sizeof(co->ColoredText.buf));
               else if (!stricmp(buffer, "Color1stLevel"))  strlcpy(co->Color1stLevel.buf, value, sizeof(co->Color1stLevel.buf));
               else if (!stricmp(buffer, "Color2ndLevel"))  strlcpy(co->Color2ndLevel.buf, value, sizeof(co->Color2ndLevel.buf));
               else if (!stricmp(buffer, "Color3rdLevel"))  strlcpy(co->Color3rdLevel.buf, value, sizeof(co->Color3rdLevel.buf));
               else if (!stricmp(buffer, "Color4thLevel"))  strlcpy(co->Color4thLevel.buf, value, sizeof(co->Color4thLevel.buf));
               else if (!stricmp(buffer, "ColorURL"))       strlcpy(co->ColorURL.buf, value, sizeof(co->ColorURL.buf));
               else if (!stricmp(buffer, "DisplayAllTexts"))co->DisplayAllTexts = Txt2Bool(value);
               else if (!stricmp(buffer, "FixedFontEdit"))  co->FixedFontEdit = Txt2Bool(value);
               else if (!stricmp(buffer, "UseTextstyles"))  co->UseTextstyles = Txt2Bool(value);
               else if (!stricmp(buffer, "MultipleWindows"))co->MultipleWindows = Txt2Bool(value);
               else if (!stricmp(buffer, "EmbeddedReadPane"))    co->EmbeddedReadPane = Txt2Bool(value);
               else if (!stricmp(buffer, "StatusChangeDelay"))
               {
                 int delay = atoi(value);

                 if(delay < 0)
                 {
                   co->StatusChangeDelay = -1*delay;
                   co->StatusChangeDelayOn = FALSE;
                 }
                 else
                 {
                   co->StatusChangeDelay = delay;
                   co->StatusChangeDelayOn = TRUE;
                 }
               }
/*5*/          else if (!stricmp(buffer, "ReplyTo"))        strlcpy(co->ReplyTo,  value, sizeof(co->ReplyTo));
               else if (!stricmp(buffer, "Organization"))   strlcpy(co->Organization, value, sizeof(co->Organization));
               else if (!stricmp(buffer, "ExtraHeaders"))   strlcpy(co->ExtraHeaders, value, sizeof(co->ExtraHeaders));
               else if (!stricmp(buffer, "NewIntro"))       strlcpy(co->NewIntro, value2, sizeof(co->NewIntro));
               else if (!stricmp(buffer, "Greetings"))      strlcpy(co->Greetings, value2, sizeof(co->Greetings));
               else if (!stricmp(buffer, "WarnSubject"))    co->WarnSubject = Txt2Bool(value);
               else if (!stricmp(buffer, "EdWrapCol"))      co->EdWrapCol = atoi(value);
               else if (!stricmp(buffer, "EdWrapMode"))     co->EdWrapMode = atoi(value);
               else if (!stricmp(buffer, "Editor"))         strlcpy(co->Editor, value, sizeof(co->Editor));
               else if (!stricmp(buffer, "LaunchAlways"))   co->LaunchAlways = Txt2Bool(value);
               else if (!stricmp(buffer, "EmailCache"))     co->EmailCache = atoi(value);
               else if (!stricmp(buffer, "AutoSave"))       co->AutoSave = atoi(value);
/*6*/          else if (!stricmp(buffer, "ReplyHello"))     strlcpy(co->ReplyHello, value2, sizeof(co->ReplyHello));
               else if (!stricmp(buffer, "ReplyIntro"))     strlcpy(co->ReplyIntro, value2, sizeof(co->ReplyIntro));
               else if (!stricmp(buffer, "ReplyBye"))       strlcpy(co->ReplyBye, value2, sizeof(co->ReplyBye));
               else if (!stricmp(buffer, "AltReplyHello"))  strlcpy(co->AltReplyHello, value2, sizeof(co->AltReplyHello));
               else if (!stricmp(buffer, "AltReplyIntro"))  strlcpy(co->AltReplyIntro, value2, sizeof(co->AltReplyIntro));
               else if (!stricmp(buffer, "AltReplyBye"))    strlcpy(co->AltReplyBye, value2, sizeof(co->AltReplyBye));
               else if (!stricmp(buffer, "AltReplyPattern"))strlcpy(co->AltReplyPattern, value2, sizeof(co->AltReplyPattern));
               else if (!stricmp(buffer, "MLReplyHello"))   strlcpy(co->MLReplyHello, value2, sizeof(co->MLReplyHello));
               else if (!stricmp(buffer, "MLReplyIntro"))   strlcpy(co->MLReplyIntro, value2, sizeof(co->MLReplyIntro));
               else if (!stricmp(buffer, "MLReplyBye"))     strlcpy(co->MLReplyBye, value2, sizeof(co->MLReplyBye));
               else if (!stricmp(buffer, "ForwardIntro"))   strlcpy(co->ForwardIntro, value2, sizeof(co->ForwardIntro));
               else if (!stricmp(buffer, "ForwardFinish"))  strlcpy(co->ForwardFinish, value2, sizeof(co->ForwardFinish));
               else if (!stricmp(buffer, "QuoteMessage"))   co->QuoteMessage = Txt2Bool(value);
               else if (!stricmp(buffer, "QuoteText"))      strlcpy(co->QuoteText, value2, sizeof(co->QuoteText));
               else if (!stricmp(buffer, "AltQuoteText"))   strlcpy(co->AltQuoteText, value2, sizeof(co->AltQuoteText));
               else if (!stricmp(buffer, "QuoteEmptyLines"))co->QuoteEmptyLines = Txt2Bool(value);
               else if (!stricmp(buffer, "CompareAddress")) co->CompareAddress = Txt2Bool(value);
               else if (!stricmp(buffer, "StripSignature")) co->StripSignature = Txt2Bool(value);
/*7*/          else if (!stricmp(buffer, "UseSignature"))   co->UseSignature = Txt2Bool(value);
               else if (!stricmp(buffer, "TagsFile"))       strlcpy(co->TagsFile, value, sizeof(co->TagsFile));
               else if (!stricmp(buffer, "TagsSeparator"))  strlcpy(co->TagsSeparator, value2, sizeof(co->TagsSeparator));
/*8*/          else if (!stricmp(buffer, "FolderCols"))     co->FolderCols = atoi(value);
               else if (!stricmp(buffer, "MessageCols"))    co->MessageCols = atoi(value);
               else if (!stricmp(buffer, "FixedFontList"))  co->FixedFontList = Txt2Bool(value);
               else if (!stricmp(buffer, "SwatchBeat"))     co->SwatchBeat = Txt2Bool(value);
               else if (!stricmp(buffer, "SizeFormat"))     co->SizeFormat = atoi(value);
               else if (!stricmp(buffer, "FolderCntMenu"))  co->FolderCntMenu = Txt2Bool(value);
               else if (!stricmp(buffer, "MessageCntMenu")) co->MessageCntMenu = Txt2Bool(value);
               else if (!stricmp(buffer, "InfoBar"))        co->InfoBar = atoi(value);
               else if (!stricmp(buffer, "InfoBarText"))    strlcpy(co->InfoBarText, value, sizeof(co->InfoBarText));
               else if (!stricmp(buffer, "QuickSearchBar")) co->QuickSearchBar = Txt2Bool(value);
/*9*/          else if (!stricmp(buffer, "PGPCmdPath"))     strlcpy(co->PGPCmdPath, value, sizeof(co->PGPCmdPath));
               else if (!stricmp(buffer, "MyPGPID"))        strlcpy(co->MyPGPID, value, sizeof(co->MyPGPID));
               else if (!stricmp(buffer, "EncryptToSelf"))  co->EncryptToSelf = Txt2Bool(value);
               else if (!stricmp(buffer, "ReMailer"))       strlcpy(co->ReMailer, value, sizeof(co->ReMailer));
               else if (!stricmp(buffer, "RMCommands"))     strlcpy(co->RMCommands, value2, sizeof(co->RMCommands));
               else if (!stricmp(buffer, "LogfilePath"))    strlcpy(co->LogfilePath, value, sizeof(co->LogfilePath));
               else if (!stricmp(buffer, "LogfileMode"))    co->LogfileMode = atoi(value);
               else if (!stricmp(buffer, "SplitLogfile"))   co->SplitLogfile = Txt2Bool(value);
               else if (!stricmp(buffer, "LogAllEvents"))   co->LogAllEvents = Txt2Bool(value);
/*10*/         else if (!stricmp(buffer, "GetOnStartup"))   co->GetOnStartup = Txt2Bool(value);
               else if (!stricmp(buffer, "SendOnStartup"))  co->SendOnStartup = Txt2Bool(value);
               else if (!stricmp(buffer, "CleanupOnStartup")) co->CleanupOnStartup = Txt2Bool(value);
               else if (!stricmp(buffer, "RemoveOnStartup"))  co->RemoveOnStartup = Txt2Bool(value);
               else if (!stricmp(buffer, "LoadAllFolders")) co->LoadAllFolders = Txt2Bool(value);
               else if (!stricmp(buffer, "UpdateNewMail"))  co->UpdateNewMail = Txt2Bool(value);
               else if (!stricmp(buffer, "CheckBirthdates"))co->CheckBirthdates = Txt2Bool(value);
               else if (!stricmp(buffer, "SendOnQuit"))     co->SendOnQuit = Txt2Bool(value);
               else if (!stricmp(buffer, "CleanupOnQuit"))  co->CleanupOnQuit = Txt2Bool(value);
               else if (!stricmp(buffer, "RemoveOnQuit"))   co->RemoveOnQuit = Txt2Bool(value);
/*11*/         else if (!strnicmp(buffer, "MV", 2) && isdigit(buffer[2]) && isdigit(buffer[3]) && strchr(buffer, '.'))
               {
                 static struct MimeTypeNode *lastType = NULL;
                 static int lastTypeID = -1;
                 int curTypeID = atoi(&buffer[2]);
                 char *p = strchr(buffer, '.')+1;

                 // we only get the correct mimetype node if the ID
                 // is greater than zero, because zero is reserved for the default
                 // mime viewer type.
                 if(curTypeID > 0)
                 {
                   if(lastType && lastTypeID != curTypeID)
                   {
                     int i;
                     struct MinNode *curNode;

                     // reset the lastType
                     lastType = NULL;
                     lastTypeID = -1;

                     // try to get the mimeType with that particular filter ID out of our
                     // filterList
                     for(i=0, curNode = co->mimeTypeList.mlh_Head; curNode->mln_Succ; curNode = curNode->mln_Succ, i++)
                     {
                       if(i == curTypeID)
                       {
                         lastType = (struct MimeTypeNode *)curNode;
                         lastTypeID = i;
                         break;
                       }
                     }
                   }

                   if(lastType == NULL)
                   {
                     if((lastType = CreateNewMimeType()))
                     {
                        AddTail((struct List *)&co->mimeTypeList, (struct Node *)lastType);
                        lastTypeID = curTypeID;
                     }
                     else
                      break;
                   }
                 }

                 // now we can fill the mimeType with data
                 if(curTypeID > 0)
                 {
                   if(!stricmp(p, "ContentType"))
                     strlcpy(lastType->ContentType, value, sizeof(lastType->ContentType));
                   else if(!stricmp(p, "Extension"))
                     strlcpy(lastType->Extension, value, sizeof(lastType->Extension));
                   else if(!stricmp(p, "Command"))
                     strlcpy(lastType->Command, value, sizeof(lastType->Command));
                   else if(!stricmp(p, "Description"))
                     strlcpy(lastType->Description, value, sizeof(lastType->Description));
                 }
                 else
                 {
                   if(!stricmp(p, "Command"))
                     strlcpy(C->DefaultMimeViewer, value, sizeof(C->DefaultMimeViewer));
                 }
               }
               else if (!stricmp(buffer, "DetachDir"))      strlcpy(co->DetachDir, value, sizeof(co->DetachDir));
               else if (!stricmp(buffer, "AttachDir"))      strlcpy(co->AttachDir, value, sizeof(co->AttachDir));
/*12*/         else if (!stricmp(buffer, "GalleryDir"))     strlcpy(co->GalleryDir, value, sizeof(co->GalleryDir));
               else if (!stricmp(buffer, "MyPictureURL"))   strlcpy(co->MyPictureURL, value, sizeof(co->MyPictureURL));
               else if (!stricmp(buffer, "ProxyServer"))    strlcpy(co->ProxyServer, value, sizeof(co->ProxyServer));
               else if (!stricmp(buffer, "NewAddrGroup"))   strlcpy(co->NewAddrGroup, value, sizeof(co->NewAddrGroup));
               else if (!stricmp(buffer, "AddToAddrbook"))  co->AddToAddrbook = atoi(value);
               else if (!stricmp(buffer, "AddMyInfo")    )  co->AddMyInfo= Txt2Bool(value);
               else if (!stricmp(buffer, "AddrbookCols"))   co->AddrbookCols = atoi(value);
/*13*/         else if (!strnicmp(buffer, "Rexx", 4) && buffer[6] == '.')
               {
                  int j = atoi(&buffer[4]);
                  if (j < MAXRX)
                  {
                     p = &buffer[7];
                     if (!stricmp(p, "Name"))            strlcpy(co->RX[j].Name, value, sizeof(co->RX[j].Name));
                     else if (!stricmp(p, "Script"))     strlcpy(co->RX[j].Script, value, sizeof(co->RX[j].Script));
                     else if (!stricmp(p, "IsAmigaDOS")) co->RX[j].IsAmigaDOS = Txt2Bool(value);
                     else if (!stricmp(p, "UseConsole")) co->RX[j].UseConsole = Txt2Bool(value);
                     else if (!stricmp(p, "WaitTerm"))   co->RX[j].WaitTerm = Txt2Bool(value);
                  }
               }
/*14*/         else if (!stricmp(buffer, "TempDir"))        strlcpy(co->TempDir, value, sizeof(co->TempDir));
               else if (!stricmp(buffer, "WBAppIcon"))      co->WBAppIcon = Txt2Bool(value);
               else if (!stricmp(buffer, "IconPosition"))   sscanf(value, "%d;%d", &(co->IconPositionX), &(co->IconPositionY));
               else if (!stricmp(buffer, "AppIconText"))    strlcpy(co->AppIconText, value, sizeof(co->AppIconText));
               else if (!stricmp(buffer, "DockyIcon"))      co->DockyIcon = Txt2Bool(value);
               else if (!stricmp(buffer, "IconifyOnQuit"))  co->IconifyOnQuit = Txt2Bool(value);
               else if (!stricmp(buffer, "Confirm"))        co->Confirm = Txt2Bool(value);
               else if (!stricmp(buffer, "ConfirmDelete"))  co->ConfirmDelete = atoi(value);
               else if (!stricmp(buffer, "RemoveAtOnce"))   co->RemoveAtOnce = Txt2Bool(value);
               else if (!stricmp(buffer, "SaveSent"))       co->SaveSent = Txt2Bool(value);
               else if (!stricmp(buffer, "MDN_Display"))    co->MDN_Display = atoi(value);
               else if (!stricmp(buffer, "MDN_Process"))    co->MDN_Process = atoi(value);
               else if (!stricmp(buffer, "MDN_Delete"))     co->MDN_Delete = atoi(value);
               else if (!stricmp(buffer, "MDN_Filter"))     co->MDN_Filter = atoi(value);
               else if (!stricmp(buffer, "SendMDNAtOnce"))  co->SendMDNAtOnce = Txt2Bool(value);
               else if (!stricmp(buffer, "XPKPack"))        { strlcpy(co->XPKPack, value, sizeof(co->XPKPack)); co->XPKPackEff = atoi(&value[5]); }
               else if (!stricmp(buffer, "XPKPackEncrypt")) { strlcpy(co->XPKPackEncrypt, value, sizeof(co->XPKPackEncrypt)); co->XPKPackEncryptEff = atoi(&value[5]); }
               else if (!stricmp(buffer, "PackerCommand"))  strlcpy(co->PackerCommand, value, sizeof(co->PackerCommand));
/*Update*/     else if (!stricmp(buffer, "UpdateInterval")) co->UpdateInterval = atoi(value);
               else if (!stricmp(buffer, "UpdateServer"))   strlcpy(co->UpdateServer, value, sizeof(co->UpdateServer));
               else if (!stricmp(buffer, "LastUpdateCheck"))String2TimeVal(&co->LastUpdateCheck, value, DSS_USDATETIME, TZC_NONE);
               else if (!stricmp(buffer, "LastUpdateStatus")) co->LastUpdateStatus = atoi(value);
/*Advanced*/   else if (!stricmp(buffer, "LetterPart"))     { co->LetterPart = atoi(value); if(co->LetterPart == 0) co->LetterPart=1; }
               else if (!stricmp(buffer, "WriteIndexes"))   co->WriteIndexes = atoi(value);
               else if (!stricmp(buffer, "SupportSite"))    strlcpy(co->SupportSite, value, sizeof(co->SupportSite));
               else if (!stricmp(buffer, "JumpToNewMsg"))   co->JumpToNewMsg = Txt2Bool(value);
               else if (!stricmp(buffer, "JumpToIncoming")) co->JumpToIncoming = Txt2Bool(value);
               else if (!stricmp(buffer, "AskJumpUnread"))  co->AskJumpUnread = Txt2Bool(value);
               else if (!stricmp(buffer, "PrinterCheck"))   co->PrinterCheck = Txt2Bool(value);
               else if (!stricmp(buffer, "IsOnlineCheck"))  co->IsOnlineCheck = Txt2Bool(value);
               else if (!stricmp(buffer, "IOCInterface"))   strlcpy(co->IOCInterface, value, sizeof(co->IOCInterface));
               else if (!stricmp(buffer, "ConfirmOnQuit"))  co->ConfirmOnQuit = Txt2Bool(value);
               else if (!stricmp(buffer, "HideGUIElements")) co->HideGUIElements = atoi(value);
               else if (!stricmp(buffer, "SysCharsetCheck"))co->SysCharsetCheck = Txt2Bool(value);
               else if (!stricmp(buffer, "AmiSSLCheck"))    co->AmiSSLCheck = Txt2Bool(value);
               else if (!stricmp(buffer, "TimeZoneCheck"))  co->TimeZoneCheck = Txt2Bool(value);
               else if (!stricmp(buffer, "AutoDSTCheck"))   co->AutoDSTCheck = Txt2Bool(value);
               else if (!stricmp(buffer, "StackSize"))      co->StackSize = atoi(value);
               else if (!stricmp(buffer, "PrintMethod"))    co->PrintMethod = atoi(value);
               else if (!stricmp(buffer, "AutoColumnResize")) co->AutoColumnResize = Txt2Bool(value);
               else if (!stricmp(buffer, "SocketOptions"))
               {
                  char *s = value;

                  // Now we have to identify the socket option line
                  // and we to that by tokenizing it
                  while(*s)
                  {
                    char *e;

                    if((e = strpbrk(s, " |;,")) == NULL)
                      e = s+strlen(s);

                    if(strnicmp(s, "SO_KEEPALIVE", 12) == 0)
                    {
                      co->SocketOptions.KeepAlive = TRUE;
                    }
                    else if(strnicmp(s, "TCP_NODELAY", 11) == 0)
                    {
                      co->SocketOptions.NoDelay = TRUE;
                    }
                    else if(strnicmp(s, "IPTOS_LOWDELAY", 14) == 0)
                    {
                      co->SocketOptions.LowDelay = TRUE;
                    }
                    else if(strnicmp(s, "SO_SNDBUF", 9) == 0)
                    {
                      char *p = strchr(s, '=');
                      if(p) co->SocketOptions.SendBuffer = atoi(p+1);
                    }
                    else if(strnicmp(s, "SO_RCVBUF", 9) == 0)
                    {
                      char *p = strchr(s, '=');
                      if(p) co->SocketOptions.RecvBuffer = atoi(p+1);
                    }
                    else if(strnicmp(s, "SO_SNDLOWAT", 11) == 0)
                    {
                      char *p = strchr(s, '=');
                      if(p) co->SocketOptions.SendLowAt = atoi(p+1);
                    }
                    else if(strnicmp(s, "SO_RCVLOWAT", 11) == 0)
                    {
                      char *p = strchr(s, '=');
                      if(p) co->SocketOptions.RecvLowAt = atoi(p+1);
                    }
                    else if(strnicmp(s, "SO_SNDTIMEO", 11) == 0)
                    {
                      char *p = strchr(s, '=');
                      if(p) co->SocketOptions.SendTimeOut = atoi(p+1);
                    }
                    else if(strnicmp(s, "SO_RCVTIMEO", 11) == 0)
                    {
                      char *p = strchr(s, '=');
                      if(p) co->SocketOptions.RecvTimeOut = atoi(p+1);
                    }

                    // set the next start to our last search
                    if(*e)
                      s = ++e;
                    else
                      break;
                  }
               }
               else if (!stricmp(buffer, "TRBufferSize")) co->TRBufferSize = atoi(value);
               else if (!stricmp(buffer, "EmbeddedMailDelay")) co->EmbeddedMailDelay = atoi(value);
               else if (!stricmp(buffer, "KeepAliveInterval")) co->KeepAliveInterval = atoi(value);
               else if (!stricmp(buffer, "StyleFGroupUnread")) String2MUIStyle(value, co->StyleFGroupUnread);
               else if (!stricmp(buffer, "StyleFGroupRead")) String2MUIStyle(value, co->StyleFGroupRead);
               else if (!stricmp(buffer, "StyleFolderUnread")) String2MUIStyle(value, co->StyleFolderUnread);
               else if (!stricmp(buffer, "StyleFolderRead")) String2MUIStyle(value, co->StyleFolderRead);
               else if (!stricmp(buffer, "StyleFolderNew")) String2MUIStyle(value, co->StyleFolderNew);
               else if (!stricmp(buffer, "StyleMailUnread")) String2MUIStyle(value, co->StyleMailUnread);
               else if (!stricmp(buffer, "StyleMailRead")) String2MUIStyle(value, co->StyleMailRead);
               else
                 W(DBF_CONFIG, "unknown config option: '%s' = '%s'", buffer, value);
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
   int i;
   struct CO_GUIData *gui = &G->CO->GUI;

   ENTER();

   switch (G->CO->VisiblePage)
   {
      case 0:
         GetMUIString(CE->RealName, gui->ST_REALNAME, sizeof(CE->RealName));
         GetMUIString(CE->EmailAddress, gui->ST_EMAIL, sizeof(CE->EmailAddress));
         CE->TimeZone          = MapTZ(GetMUICycle(gui->CY_TZONE), TRUE);
         CE->DaylightSaving    = GetMUICheck  (gui->CH_DLSAVING);
         GetMUIString(CE->LocalCharset, gui->ST_DEFAULTCHARSET, sizeof(CE->LocalCharset));
         CE->DetectCyrillic= GetMUICheck(gui->CH_DETECTCYRILLIC);
         break;
      case 1:
         GetMUIString(CE->SMTP_Server, gui->ST_SMTPHOST, sizeof(CE->SMTP_Server));
         CE->SMTP_Port = GetMUIInteger(gui->ST_SMTPPORT);
         GetMUIString(CE->SMTP_Domain, gui->ST_DOMAIN, sizeof(CE->SMTP_Domain));
         CE->SMTP_SecureMethod = GetMUIRadio  (gui->RA_SMTPSECURE);
         CE->Allow8bit         = GetMUICheck  (gui->CH_SMTP8BIT);
         CE->Use_SMTP_AUTH     = GetMUICheck  (gui->CH_USESMTPAUTH);
         GetMUIString(CE->SMTP_AUTH_User, gui->ST_SMTPAUTHUSER, sizeof(CE->SMTP_AUTH_User));
         GetMUIString(CE->SMTP_AUTH_Pass, gui->ST_SMTPAUTHPASS, sizeof(CE->SMTP_AUTH_Pass));
         break;
      case 2:
         CE->PreSelection      = GetMUICycle  (gui->CY_MSGSELECT);
         CE->TransferWindow    = GetMUICycle  (gui->CY_TRANSWIN);
         CE->AvoidDuplicates   = GetMUICheck  (gui->CH_AVOIDDUP);
         CE->UpdateStatus      = GetMUICheck  (gui->CH_UPDSTAT);
         CE->WarnSize          = GetMUIInteger(gui->ST_WARNSIZE);
         CE->CheckMailDelay    = GetMUINumer  (gui->NM_INTERVAL);
         CE->DownloadLarge     = GetMUICheck  (gui->CH_DLLARGE);
         CE->NotifyType        = (GetMUICheck(gui->CH_NOTIREQ)   ? NOTIFY_REQ   : 0)
                               + (GetMUICheck(gui->CH_NOTISOUND) ? NOTIFY_SOUND : 0)
                               + (GetMUICheck(gui->CH_NOTICMD)   ? NOTIFY_CMD   : 0);
         GetMUIString(CE->NotifySound, gui->ST_NOTISOUND, sizeof(CE->NotifySound));
         GetMUIString(CE->NotifyCommand, gui->ST_NOTICMD, sizeof(CE->NotifyCommand));
         break;

      case 3:
      {
        int i=0;

        // as the user may have changed the order of the filters
        // we have to make sure the order in the NList fits to the
        // exec list order of our filter list
        do
        {
          struct FilterNode *filter = NULL;

          DoMethod(gui->LV_RULES, MUIM_NList_GetEntry, i, &filter);
          if(filter)
          {
            // for resorting the filterlist we just have to remove that particular filter
            // and add it to the tail - all other operations like adding/removing should
            // have been done by others already - so this is just resorting
            Remove((struct Node *)filter);
            AddTail((struct List *)&CE->filterList, (struct Node *)filter);
          }
          else
            break;
        }
        while(++i);
      }
      break;

      case 4:
         CE->ShowHeader        = GetMUICycle  (gui->CY_HEADER);
         GetMUIString(CE->ShortHeaders, gui->ST_HEADERS, sizeof(CE->ShortHeaders));
         CE->ShowSenderInfo    = GetMUICycle  (gui->CY_SENDERINFO);
         CE->SigSepLine        = GetMUICycle  (gui->CY_SIGSEPLINE);
         CE->ColoredText       = *(struct MUI_PenSpec *)xget(gui->CA_COLTEXT,  MUIA_Pendisplay_Spec);
         CE->Color1stLevel     = *(struct MUI_PenSpec *)xget(gui->CA_COL1QUOT, MUIA_Pendisplay_Spec);
         CE->Color2ndLevel     = *(struct MUI_PenSpec *)xget(gui->CA_COL2QUOT, MUIA_Pendisplay_Spec);
         CE->Color3rdLevel     = *(struct MUI_PenSpec *)xget(gui->CA_COL3QUOT, MUIA_Pendisplay_Spec);
         CE->Color4thLevel     = *(struct MUI_PenSpec *)xget(gui->CA_COL4QUOT, MUIA_Pendisplay_Spec);
         CE->ColorURL          = *(struct MUI_PenSpec *)xget(gui->CA_COLURL,   MUIA_Pendisplay_Spec);
         CE->DisplayAllTexts   = GetMUICheck  (gui->CH_ALLTEXTS);
         CE->FixedFontEdit     = GetMUICheck  (gui->CH_FIXFEDIT);
         CE->WrapHeader        = GetMUICheck  (gui->CH_WRAPHEAD);
         CE->UseTextstyles     = GetMUICheck  (gui->CH_TEXTSTYLES);
         CE->MultipleWindows   = GetMUICheck  (gui->CH_MULTIWIN);
         CE->EmbeddedReadPane  = GetMUICheck  (gui->CH_EMBEDDEDREADPANE);
         CE->StatusChangeDelayOn  = GetMUICheck  (gui->CH_DELAYEDSTATUS);
         CE->StatusChangeDelay    = GetMUINumer  (gui->NB_DELAYEDSTATUS)*1000;
         break;
      case 5:
         GetMUIString(CE->ReplyTo, gui->ST_REPLYTO, sizeof(CE->ReplyTo));
         GetMUIString(CE->Organization, gui->ST_ORGAN, sizeof(CE->Organization));
         GetMUIString(CE->ExtraHeaders, gui->ST_EXTHEADER, sizeof(CE->ExtraHeaders));
         GetMUIString(CE->NewIntro, gui->ST_HELLOTEXT, sizeof(CE->NewIntro));
         GetMUIString(CE->Greetings, gui->ST_BYETEXT, sizeof(CE->Greetings));
         CE->WarnSubject       = GetMUICheck  (gui->CH_WARNSUBJECT);
         CE->EdWrapCol         = GetMUIInteger(gui->ST_EDWRAP);
         CE->EdWrapMode        = GetMUICycle  (gui->CY_EDWRAP);
         GetMUIString(CE->Editor, gui->ST_EDITOR, sizeof(CE->Editor));
         CE->LaunchAlways      = GetMUICheck  (gui->CH_LAUNCH);
         CE->EmailCache        = GetMUINumer  (gui->NB_EMAILCACHE);
         CE->AutoSave          = GetMUINumer  (gui->NB_AUTOSAVE)*60; // in seconds
         break;
      case 6:
         GetMUIString(CE->ReplyHello, gui->ST_REPLYHI, sizeof(CE->ReplyHello));
         GetMUIString(CE->ReplyIntro, gui->ST_REPLYTEXT, sizeof(CE->ReplyIntro));
         GetMUIString(CE->ReplyBye, gui->ST_REPLYBYE, sizeof(CE->ReplyBye));
         GetMUIString(CE->AltReplyHello, gui->ST_AREPLYHI, sizeof(CE->AltReplyHello));
         GetMUIString(CE->AltReplyIntro, gui->ST_AREPLYTEXT, sizeof(CE->AltReplyIntro));
         GetMUIString(CE->AltReplyBye, gui->ST_AREPLYBYE, sizeof(CE->AltReplyBye));
         GetMUIString(CE->AltReplyPattern, gui->ST_AREPLYPAT, sizeof(CE->AltReplyPattern));
         GetMUIString(CE->MLReplyHello, gui->ST_MREPLYHI, sizeof(CE->MLReplyHello));
         GetMUIString(CE->MLReplyIntro, gui->ST_MREPLYTEXT, sizeof(CE->MLReplyIntro));
         GetMUIString(CE->MLReplyBye, gui->ST_MREPLYBYE, sizeof(CE->MLReplyBye));
         GetMUIString(CE->ForwardIntro, gui->ST_FWDSTART, sizeof(CE->ForwardIntro));
         GetMUIString(CE->ForwardFinish, gui->ST_FWDEND, sizeof(CE->ForwardFinish));
         CE->QuoteMessage      = GetMUICheck  (gui->CH_QUOTE);
         GetMUIString(CE->QuoteText, gui->ST_REPLYCHAR, sizeof(CE->QuoteText));
         GetMUIString(CE->AltQuoteText, gui->ST_ALTQUOTECHAR, sizeof(CE->AltQuoteText));
         CE->QuoteEmptyLines   = GetMUICheck  (gui->CH_QUOTEEMPTY);
         CE->CompareAddress    = GetMUICheck  (gui->CH_COMPADDR);
         CE->StripSignature    = GetMUICheck  (gui->CH_STRIPSIG);
         break;
      case 7:
         CE->UseSignature      = GetMUICheck  (gui->CH_USESIG);
         GetMUIString(CE->TagsFile, gui->ST_TAGFILE, sizeof(CE->TagsFile));
         GetMUIString(CE->TagsSeparator, gui->ST_TAGSEP, sizeof(CE->TagsSeparator));
         if(xget(gui->TE_SIGEDIT, MUIA_TextEditor_HasChanged))
            EditorToFile(gui->TE_SIGEDIT, CreateFilename(SigNames[G->CO->LastSig]));
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
         GetMUIString(CE->InfoBarText, gui->ST_INFOBARTXT, sizeof(CE->InfoBarText));
         CE->QuickSearchBar= GetMUICheck(gui->CH_QUICKSEARCHBAR);
         break;
      case 9:
         GetMUIString(CE->PGPCmdPath, gui->ST_PGPCMD, sizeof(CE->PGPCmdPath));
         GetMUIString(CE->MyPGPID, gui->ST_MYPGPID, sizeof(CE->MyPGPID));
         CE->EncryptToSelf     = GetMUICheck  (gui->CH_ENCSELF);
         GetMUIString(CE->ReMailer, gui->ST_REMAILER, sizeof(CE->ReMailer));
         GetMUIString(CE->RMCommands, gui->ST_FIRSTLINE, sizeof(CE->RMCommands));
         GetMUIString(CE->LogfilePath, gui->ST_LOGFILE, sizeof(CE->LogfilePath));
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
         GetMUIString(CE->DefaultMimeViewer, gui->ST_DEFVIEWER, sizeof(CE->DefaultMimeViewer));
         GetMUIString(CE->DetachDir, gui->ST_DETACHDIR, sizeof(CE->DetachDir));
         GetMUIString(CE->AttachDir, gui->ST_ATTACHDIR, sizeof(CE->AttachDir));
         break;
      case 12:
         GetMUIString(CE->GalleryDir, gui->ST_GALLDIR, sizeof(CE->GalleryDir));
         GetMUIString(CE->MyPictureURL, gui->ST_PHOTOURL, sizeof(CE->MyPictureURL));
         GetMUIString(CE->NewAddrGroup, gui->ST_NEWGROUP, sizeof(CE->NewAddrGroup));
         GetMUIString(CE->ProxyServer, gui->ST_PROXY, sizeof(CE->ProxyServer));
         CE->AddToAddrbook     = GetMUICycle  (gui->CY_ATAB);
         CE->AddMyInfo         = GetMUICheck  (gui->CH_ADDINFO);
         CE->AddrbookCols = 1; for (i = 1; i < ABCOLNUM; i++) if (GetMUICheck(gui->CH_ACOLS[i])) CE->AddrbookCols += (1<<i);
         break;
      case 14:
         GetMUIString(CE->TempDir, gui->ST_TEMPDIR, sizeof(CE->TempDir));
         CE->WBAppIcon         = GetMUICheck  (gui->CH_WBAPPICON);
         CE->IconPositionX     = GetMUIInteger(gui->ST_APPX);
         CE->IconPositionY     = GetMUIInteger(gui->ST_APPY);
         GetMUIString(CE->AppIconText, gui->ST_APPICON, sizeof(CE->AppIconText));
         CE->DockyIcon         = GetMUICheck  (gui->CH_DOCKYICON);
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
         GetMUIText(CE->XPKPack, gui->TX_PACKER, sizeof(CE->XPKPack));
         GetMUIText(CE->XPKPackEncrypt, gui->TX_ENCPACK, sizeof(CE->XPKPackEncrypt));
         CE->XPKPackEff        = GetMUINumer  (gui->NB_PACKER);
         CE->XPKPackEncryptEff = GetMUINumer  (gui->NB_ENCPACK);
         GetMUIString(CE->PackerCommand, gui->ST_ARCHIVER, sizeof(CE->PackerCommand));
      break;

      // [Update]
      case 15:
      {
        if(GetMUICheck(gui->CH_UPDATECHECK) == TRUE)
        {
          int interval = GetMUICycle(gui->CY_UPDATEINTERVAL);

          switch(interval)
          {
            case 0:
              CE->UpdateInterval = 86400; // 1 day
            break;

            case 1:
              CE->UpdateInterval = 604800; // 1 week
            break;

            case 2:
              CE->UpdateInterval = 2419200; // 1 month
            break;

            default:
              CE->UpdateInterval = 0;
          }
        }
        else
          CE->UpdateInterval = 0; // disabled
      }
      break;
   }

   LEAVE();
}

///
/// CO_SetConfig
//  Sets current section of configuration structure with data from GUI
void CO_SetConfig(void)
{
   struct CO_GUIData *gui = &G->CO->GUI;
   int i;

   ENTER();

   switch (G->CO->VisiblePage)
   {
      case 0:
         setstring(gui->ST_REALNAME  ,CE->RealName);
         setstring(gui->ST_EMAIL     ,CE->EmailAddress);
         setcycle(gui->CY_TZONE, MapTZ(CE->TimeZone, FALSE));
         setcheckmark(gui->CH_DLSAVING  ,CE->DaylightSaving);
         nnset(gui->ST_POPHOST0, MUIA_String_Contents, CE->P3[0]->Server);
         nnset(gui->ST_PASSWD0,  MUIA_String_Contents, CE->P3[0]->Password);
         nnset(gui->ST_DEFAULTCHARSET,  MUIA_String_Contents, CE->LocalCharset);
         setcheckmark(gui->CH_DETECTCYRILLIC, CE->DetectCyrillic);
         break;

      case 1:
      {
         setstring(gui->ST_SMTPHOST, CE->SMTP_Server);
         set(gui->ST_SMTPPORT, MUIA_String_Integer, CE->SMTP_Port);
         setstring(gui->ST_DOMAIN, CE->SMTP_Domain);
         setmutex(gui->RA_SMTPSECURE, CE->SMTP_SecureMethod);
         nnset(gui->RA_SMTPSECURE, MUIA_Disabled, !G->TR_UseableTLS && CE->SMTP_SecureMethod == SMTPSEC_NONE);
         setcheckmark(gui->CH_SMTP8BIT  ,CE->Allow8bit);
         setcheckmark(gui->CH_USESMTPAUTH,CE->Use_SMTP_AUTH);
         setstring   (gui->ST_SMTPAUTHUSER,CE->SMTP_AUTH_User);
         setstring   (gui->ST_SMTPAUTHPASS,CE->SMTP_AUTH_Pass);

         // clear the list first
         DoMethod(gui->LV_POP3, MUIM_List_Clear);

         for(i=0; i < MAXP3; i++)
         {
           struct POP3 *pop3;

           if((pop3 = CE->P3[i]))
           {
             snprintf(pop3->Account, sizeof(pop3->Account), "%s@%s", pop3->User, pop3->Server);
             DoMethod(gui->LV_POP3, MUIM_List_InsertSingle, pop3, MUIV_List_Insert_Bottom);
           }
         }

         // make sure the first entry is selected per default
         set(gui->LV_POP3, MUIA_List_Active, MUIV_List_Active_Top);
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
         setcheckmark(gui->CH_NOTIREQ   , hasRequesterNotify(CE->NotifyType));
         setcheckmark(gui->CH_NOTISOUND , hasSoundNotify(CE->NotifyType));
         setcheckmark(gui->CH_NOTICMD   , hasCommandNotify(CE->NotifyType));
         setstring   (gui->ST_NOTISOUND ,CE->NotifySound);
         setstring   (gui->ST_NOTICMD   ,CE->NotifyCommand);
         break;

      case 3:
      {
         struct MinNode *curNode;

         // clear the filter list first
         DoMethod(gui->LV_RULES, MUIM_NList_Clear);

         // iterate through our filter list and add it to our
         // MUI List
         for(curNode = CE->filterList.mlh_Head; curNode->mln_Succ; curNode = curNode->mln_Succ)
         {
           DoMethod(gui->LV_RULES, MUIM_NList_InsertSingle, curNode, MUIV_NList_Insert_Bottom);
         }

         // make sure the first entry is selected per default
         set(gui->LV_RULES, MUIA_NList_Active, MUIV_NList_Active_Top);
      }
      break;

      case 4:
         setcycle    (gui->CY_HEADER    ,CE->ShowHeader);
         setstring   (gui->ST_HEADERS   ,CE->ShortHeaders);
         setcycle    (gui->CY_SENDERINFO,CE->ShowSenderInfo);
         setcycle    (gui->CY_SIGSEPLINE,CE->SigSepLine);
         set(gui->CA_COLTEXT,  MUIA_Pendisplay_Spec, &CE->ColoredText);
         set(gui->CA_COL1QUOT, MUIA_Pendisplay_Spec, &CE->Color1stLevel);
         set(gui->CA_COL2QUOT, MUIA_Pendisplay_Spec, &CE->Color2ndLevel);
         set(gui->CA_COL3QUOT, MUIA_Pendisplay_Spec, &CE->Color3rdLevel);
         set(gui->CA_COL4QUOT, MUIA_Pendisplay_Spec, &CE->Color4thLevel);
         set(gui->CA_COLURL,   MUIA_Pendisplay_Spec, &CE->ColorURL);
         setcheckmark(gui->CH_ALLTEXTS  ,CE->DisplayAllTexts);
         setcheckmark(gui->CH_FIXFEDIT  ,CE->FixedFontEdit);
         setcheckmark(gui->CH_WRAPHEAD  ,CE->WrapHeader);
         setcheckmark(gui->CH_TEXTSTYLES,CE->UseTextstyles);
         setcheckmark(gui->CH_MULTIWIN  ,CE->MultipleWindows);
         setcheckmark(gui->CH_EMBEDDEDREADPANE, CE->EmbeddedReadPane);
         setcheckmark(gui->CH_DELAYEDSTATUS, CE->StatusChangeDelayOn);
         set(gui->NB_DELAYEDSTATUS, MUIA_Numeric_Value, CE->StatusChangeDelay/1000);
         break;
      case 5:
         setstring   (gui->ST_REPLYTO   ,CE->ReplyTo);
         setstring   (gui->ST_ORGAN     ,CE->Organization);
         setstring   (gui->ST_EXTHEADER ,CE->ExtraHeaders);
         setstring   (gui->ST_HELLOTEXT ,CE->NewIntro);
         setstring   (gui->ST_BYETEXT   ,CE->Greetings);
         setcheckmark(gui->CH_WARNSUBJECT,CE->WarnSubject);
         set(gui->ST_EDWRAP, MUIA_String_Integer, CE->EdWrapCol);
         setcycle    (gui->CY_EDWRAP    ,CE->EdWrapMode);
         setstring   (gui->ST_EDITOR    ,CE->Editor);
         setcheckmark(gui->CH_LAUNCH    ,CE->LaunchAlways);
         setslider   (gui->NB_EMAILCACHE,CE->EmailCache);
         setslider   (gui->NB_AUTOSAVE,  CE->AutoSave/60);
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
         DoMethod(G->App, MUIM_CallHook, &CO_SwitchSignatHook, !CE->UseSignature);
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
         setstring(gui->ST_INFOBARTXT   ,CE->InfoBarText);
         setcheckmark(gui->CH_QUICKSEARCHBAR, CE->QuickSearchBar);
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
      {
         struct MinNode *curNode;

         // clear the filter list first
         DoMethod(gui->LV_MIME, MUIM_List_Clear);

         // iterate through our filter list and add it to our
         // MUI List
         for(curNode = CE->mimeTypeList.mlh_Head; curNode->mln_Succ; curNode = curNode->mln_Succ)
           DoMethod(gui->LV_MIME, MUIM_List_InsertSingle, curNode, MUIV_List_Insert_Bottom);

         // make sure the first entry is selected per default
         set(gui->LV_MIME, MUIA_List_Active, MUIV_List_Active_Top);

         setstring   (gui->ST_DEFVIEWER ,CE->DefaultMimeViewer);
         setstring   (gui->ST_DETACHDIR ,CE->DetachDir);
         setstring   (gui->ST_ATTACHDIR ,CE->AttachDir);
      }
      break;

      case 12:
         setstring   (gui->ST_GALLDIR   ,CE->GalleryDir);
         setstring   (gui->ST_PHOTOURL  ,CE->MyPictureURL);
         setstring   (gui->ST_NEWGROUP  ,CE->NewAddrGroup);
         set(gui->ST_NEWGROUP, MUIA_Disabled, CE->AddToAddrbook == 0);
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
         setcheckmark(gui->CH_WBAPPICON ,CE->WBAppIcon);
         set(gui->ST_APPX, MUIA_String_Integer, CE->IconPositionX);
         set(gui->ST_APPY, MUIA_String_Integer, CE->IconPositionY);
         setstring   (gui->ST_APPICON   ,CE->AppIconText);
         setcheckmark(gui->CH_DOCKYICON ,CE->DockyIcon);
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
         break;

      // [Update]
      case 15:
      {
        setcheckmark(gui->CH_UPDATECHECK, CE->UpdateInterval > 0);

        if(CE->UpdateInterval > 0)
        {
          if(CE->UpdateInterval <= 86400)
            setcycle(gui->CY_UPDATEINTERVAL, 0); // daily
          else if(CE->UpdateInterval <= 604800)
            setcycle(gui->CY_UPDATEINTERVAL, 1); // weekly
          else
            setcycle(gui->CY_UPDATEINTERVAL, 2); // monthly
        }
        else
          setcycle(gui->CY_UPDATEINTERVAL, 1);

        // now we set the information on the last update check
        switch(C->LastUpdateStatus)
        {
          case UST_NOCHECK:
            set(gui->TX_UPDATESTATUS, MUIA_Text_Contents, GetStr(MSG_CO_LASTSTATUS_NOCHECK));
          break;

          case UST_NOUPDATE:
            set(gui->TX_UPDATESTATUS, MUIA_Text_Contents, GetStr(MSG_CO_LASTSTATUS_NOUPDATE));
          break;

          case UST_NOQUERY:
            set(gui->TX_UPDATESTATUS, MUIA_Text_Contents, GetStr(MSG_CO_LASTSTATUS_NOQUERY));
          break;

          case UST_UPDATESUCCESS:
            set(gui->TX_UPDATESTATUS, MUIA_Text_Contents, GetStr(MSG_CO_LASTSTATUS_UPDATESUCCESS));
          break;
        }

        // set the lastUpdateCheckDate
        if(C->LastUpdateStatus != UST_NOCHECK && C->LastUpdateCheck.Seconds > 0)
        {
          char buf[SIZE_DEFAULT];

          TimeVal2String(buf, sizeof(buf), &C->LastUpdateCheck, DSS_DATETIME, TZC_NONE);
          set(gui->TX_UPDATEDATE, MUIA_Text_Contents, buf);
        }
        else
        {
          // no update check was yet performed, so we clear our status gadgets
          set(gui->TX_UPDATEDATE, MUIA_Text_Contents, "");
        }
      }
      break;

   }

   LEAVE();
}
///
