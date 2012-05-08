/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2012 YAM Open Source Team

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
#include <workbench/workbench.h>
#include <mui/NList_mcc.h>
#include <mui/NListtree_mcc.h>
#include <mui/TextEditor_mcc.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/locale.h>
#include <proto/utility.h>

#include "extrasrc.h"

#include "timeval.h"

#include "YAM.h"
#include "YAM_config.h"
#include "YAM_error.h"
#include "YAM_find.h"
#include "YAM_folderconfig.h"
#include "YAM_global.h"
#include "YAM_main.h"
#include "YAM_utilities.h"

#include "mui/ClassesExtra.h"
#include "mui/MailTextEdit.h"
#include "mui/MainWindowToolbar.h"
#include "mui/ThemeListGroup.h"

#include "BayesFilter.h"
#include "FileInfo.h"
#include "FolderList.h"
#include "Locale.h"
#include "Logfile.h"
#include "MailList.h"
#include "MailServers.h"
#include "MimeTypes.h"
#include "MUIObjects.h"
#include "Requesters.h"
#include "UserIdentity.h"

#include "Debug.h"

/***************************************************************************
 Module: Configuration - Basic Get/Put routines
***************************************************************************/

/// MUIStyle2String
// converts a MUI style string which contains common \033 sequences into a
// human-readable form which we can save to our configuration file.
char *MUIStyle2String(const char *style)
{
  static char buf[SIZE_SMALL];
  const char *s = style;
  size_t buflen;

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
  buflen = strlen(buf);
  if(buflen > 0 && buf[buflen-1] == ':')
    buf[buflen-1] = '\0';

  LEAVE();
  return buf;
}
///
/// String2MUIStyle
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
BOOL CO_SaveConfig(struct Config *co, const char *fname)
{
  BOOL result = FALSE;
  FILE *fh;

  ENTER();

  D(DBF_CONFIG, "about to save configuration to '%s'", fname);

  // flag this configuration as not (properly) saved in
  // advanced, just in case something goes wrong.
  co->ConfigIsSaved = FALSE;

  if((fh = fopen(fname, "w")) != NULL)
  {
    int i;
    char buf[SIZE_LARGE];
    struct Node *curNode;

    setvbuf(fh, NULL, _IOFBF, SIZE_FILEBUF);

    fprintf(fh, "YCO4 - YAM Configuration\n");
    fprintf(fh, "# generated by '%s (%s)'\n", yamversion, yamversiondate);

    fprintf(fh, "\n[First steps]\n");
    fprintf(fh, "TimeZone         = %d\n", co->TimeZone);
    fprintf(fh, "DaylightSaving   = %s\n", Bool2Txt(co->DaylightSaving));

    fprintf(fh, "\n[TCP/IP]\n");

    // we iterate through our mail server list and ouput the SMTP servers in it
    i = 0;
    IterateList(&co->mailServerList, curNode)
    {
      struct MailServerNode *msn = (struct MailServerNode *)curNode;

      if(msn->type == MST_SMTP)
      {
        // for the first SMTP server we output in the old format
        if(i == 0)
        {
          fprintf(fh, "SMTP-ID                  = %08x\n", msn->id);
          fprintf(fh, "SMTP-Enabled             = %s\n", Bool2Txt(isServerActive(msn)));
          fprintf(fh, "SMTP-Description         = %s\n", msn->description);
          fprintf(fh, "SMTP-Server              = %s\n", msn->hostname);
          fprintf(fh, "SMTP-Port                = %d\n", msn->port);
          fprintf(fh, "SMTP-SecMethod           = %d\n", MSF2SMTPSecMethod(msn));
          fprintf(fh, "Allow8bit                = %s\n", Bool2Txt(hasServer8bit(msn)));
          fprintf(fh, "Use-SMTP-AUTH            = %s\n", Bool2Txt(hasServerAuth(msn)));
          fprintf(fh, "SMTP-AUTH-User           = %s\n", msn->username);
          fprintf(fh, "SMTP-AUTH-Pass           = %s\n", Encrypt(msn->password));
          fprintf(fh, "SMTP-AUTH-Method         = %d\n", MSF2SMTPAuthMethod(msn));
        }
        else
        {
          fprintf(fh, "SMTP%02d.ID                = %08x\n", i, msn->id);
          fprintf(fh, "SMTP%02d.Enabled           = %s\n", i, Bool2Txt(isServerActive(msn)));
          fprintf(fh, "SMTP%02d.Description       = %s\n", i, msn->description);
          fprintf(fh, "SMTP%02d.Server            = %s\n", i, msn->hostname);
          fprintf(fh, "SMTP%02d.Port              = %d\n", i, msn->port);
          fprintf(fh, "SMTP%02d.SecMethod         = %d\n", i, MSF2SMTPSecMethod(msn));
          fprintf(fh, "SMTP%02d.Allow8bit         = %s\n", i, Bool2Txt(hasServer8bit(msn)));
          fprintf(fh, "SMTP%02d.SMTP-AUTH         = %s\n", i, Bool2Txt(hasServerAuth(msn)));
          fprintf(fh, "SMTP%02d.AUTH-User         = %s\n", i, msn->username);
          fprintf(fh, "SMTP%02d.AUTH-Pass         = %s\n", i, Encrypt(msn->password));
          fprintf(fh, "SMTP%02d.AUTH-Method       = %d\n", i, MSF2SMTPAuthMethod(msn));
        }

        i++;
      }
    }

    // we iterate through our mail server list and ouput the POP3 servers in it
    i = 0;
    IterateList(&co->mailServerList, curNode)
    {
      struct MailServerNode *msn = (struct MailServerNode *)curNode;

      if(msn->type == MST_POP3)
      {
        fprintf(fh, "POP%02d.ID                 = %08x\n", i, msn->id);
        fprintf(fh, "POP%02d.Enabled            = %s\n", i, Bool2Txt(isServerActive(msn)));
        fprintf(fh, "POP%02d.Account            = %s\n", i, msn->description);
        fprintf(fh, "POP%02d.Server             = %s\n", i, msn->hostname);
        fprintf(fh, "POP%02d.Port               = %d\n", i, msn->port);
        fprintf(fh, "POP%02d.User               = %s\n", i, msn->username);
        fprintf(fh, "POP%02d.Password           = %s\n", i, Encrypt(msn->password));
        fprintf(fh, "POP%02d.SSLMode            = %d\n", i, MSF2POP3SecMethod(msn));
        fprintf(fh, "POP%02d.UseAPOP            = %s\n", i, Bool2Txt(hasServerAPOP(msn)));
        fprintf(fh, "POP%02d.Delete             = %s\n", i, Bool2Txt(hasServerPurge(msn)));
        fprintf(fh, "POP%02d.AvoidDuplicates    = %s\n", i, Bool2Txt(hasServerAvoidDuplicates(msn)));
        fprintf(fh, "POP%02d.ApplyRemoteFilters = %s\n", i, Bool2Txt(hasServerApplyRemoteFilters(msn)));
        fprintf(fh, "POP%02d.Preselection       = %d\n", i, msn->preselection);
        fprintf(fh, "POP%02d.DownloadOnStartup  = %s\n", i, Bool2Txt(hasServerDownloadOnStartup(msn)));

        i++;
      }
    }

    fprintf(fh, "\n[Identities]\n");

    // we iterate through our mail server list and ouput the POP3 servers in it
    i = 0;
    IterateList(&co->userIdentityList, curNode)
    {
      struct UserIdentityNode *uin = (struct UserIdentityNode *)curNode;

      fprintf(fh, "ID%02d.Enabled            = %s\n", i, Bool2Txt(uin->active));
      fprintf(fh, "ID%02d.Description        = %s\n", i, uin->description);
      fprintf(fh, "ID%02d.Realname           = %s\n", i, uin->realname);
      fprintf(fh, "ID%02d.Address            = %s\n", i, uin->address);
      fprintf(fh, "ID%02d.Organization       = %s\n", i, uin->organization);
      fprintf(fh, "ID%02d.MailServerID       = %08x\n", i, uin->mailServer != NULL ? uin->mailServer->id : 0);
      fprintf(fh, "ID%02d.Signature          = %d\n", i, uin->signature);
      fprintf(fh, "ID%02d.MailCC             = %s\n", i, uin->mailCC);
      fprintf(fh, "ID%02d.MailBCC            = %s\n", i, uin->mailBCC);
      fprintf(fh, "ID%02d.MailReplyTo        = %s\n", i, uin->mailReplyTo);
      fprintf(fh, "ID%02d.ExtraHeaders       = %s\n", i, uin->extraHeaders);
      fprintf(fh, "ID%02d.PhotoURL           = %s\n", i, uin->photoURL);
      fprintf(fh, "ID%02d.SentFolder         = %s\n", i, uin->sentFolder);
      fprintf(fh, "ID%02d.SaveSentMail       = %s\n", i, Bool2Txt(uin->saveSentMail));
      fprintf(fh, "ID%02d.QuoteMails         = %s\n", i, Bool2Txt(uin->quoteMails));
      fprintf(fh, "ID%02d.QuotePosition      = %d\n", i, uin->quotePosition);
      fprintf(fh, "ID%02d.SignaturePosition  = %d\n", i, uin->signaturePosition);
      fprintf(fh, "ID%02d.SignatureReply     = %s\n", i, Bool2Txt(uin->sigReply));
      fprintf(fh, "ID%02d.SignatureForward   = %s\n", i, Bool2Txt(uin->sigForwarding));
      fprintf(fh, "ID%02d.AddPersonalInfo    = %s\n", i, Bool2Txt(uin->addPersonalInfo));
      fprintf(fh, "ID%02d.RequestMDN         = %s\n", i, Bool2Txt(uin->requestMDN));
      fprintf(fh, "ID%02d.UsePGP             = %s\n", i, Bool2Txt(uin->usePGP));
      fprintf(fh, "ID%02d.PGPKeyID           = %s\n", i, uin->pgpKeyID);
      fprintf(fh, "ID%02d.PGPKeyURL          = %s\n", i, uin->pgpKeyURL);
      fprintf(fh, "ID%02d.PGPSignUnencrypted = %s\n", i, Bool2Txt(uin->pgpSignUnencrypted));
      fprintf(fh, "ID%02d.PGPSignEncrypted   = %s\n", i, Bool2Txt(uin->pgpSignEncrypted));
      fprintf(fh, "ID%02d.PGPEncryptAll      = %s\n", i, Bool2Txt(uin->pgpEncryptAll));
      fprintf(fh, "ID%02d.PGPSelfEncrypt     = %s\n", i, Bool2Txt(uin->pgpSelfEncrypt));

      i++;
    }

    fprintf(fh, "\n[New mail]\n");
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
    i = 0;
    IterateList(&co->filterList, curNode)
    {
      int j;
      struct FilterNode *filter = (struct FilterNode *)curNode;
      struct Node *curRuleNode;

      fprintf(fh, "FI%02d.Name        = %s\n", i, filter->name);
      fprintf(fh, "FI%02d.Remote      = %s\n", i, Bool2Txt(filter->remote));
      fprintf(fh, "FI%02d.ApplyToNew  = %s\n", i, Bool2Txt(filter->applyToNew));
      fprintf(fh, "FI%02d.ApplyToSent = %s\n", i, Bool2Txt(filter->applyToSent));
      fprintf(fh, "FI%02d.ApplyOnReq  = %s\n", i, Bool2Txt(filter->applyOnReq));

      // now we do have to iterate through our ruleList
      j = 0;
      IterateList(&filter->ruleList, curRuleNode)
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
          fprintf(fh, "FI%02d.CaseSens    = %s\n", i, Bool2Txt(isFlagSet(rule->flags, SEARCHF_CASE_SENSITIVE)));
          fprintf(fh, "FI%02d.Substring   = %s\n", i, Bool2Txt(isFlagSet(rule->flags, SEARCHF_SUBSTRING)));
          fprintf(fh, "FI%02d.DOSPattern  = %s\n", i, Bool2Txt(isFlagSet(rule->flags, SEARCHF_DOS_PATTERN)));
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
          fprintf(fh, "FI%02d.CaseSens%d   = %s\n", i, j+1, Bool2Txt(isFlagSet(rule->flags, SEARCHF_CASE_SENSITIVE)));
          fprintf(fh, "FI%02d.Substring%d  = %s\n", i, j+1, Bool2Txt(isFlagSet(rule->flags, SEARCHF_SUBSTRING)));
          fprintf(fh, "FI%02d.DOSPattern%d = %s\n", i, j+1, Bool2Txt(isFlagSet(rule->flags, SEARCHF_DOS_PATTERN)));
        }

        j++;
      }

      fprintf(fh, "FI%02d.Actions     = %d\n", i, filter->actions);
      fprintf(fh, "FI%02d.BounceTo    = %s\n", i, filter->bounceTo);
      fprintf(fh, "FI%02d.ForwardTo   = %s\n", i, filter->forwardTo);
      fprintf(fh, "FI%02d.ReplyFile   = %s\n", i, filter->replyFile);
      fprintf(fh, "FI%02d.ExecuteCmd  = %s\n", i, filter->executeCmd);
      fprintf(fh, "FI%02d.PlaySound   = %s\n", i, filter->playSound);
      fprintf(fh, "FI%02d.MoveTo      = %s\n", i, filter->moveTo);

      i++;
    }

    fprintf(fh, "\n[Spam filter]\n");
    fprintf(fh, "SpamFilterEnabled= %s\n", Bool2Txt(co->SpamFilterEnabled));
    fprintf(fh, "SpamFilterForNew = %s\n", Bool2Txt(co->SpamFilterForNewMail));
    fprintf(fh, "SpamMarkOnMove   = %s\n", Bool2Txt(co->SpamMarkOnMove));
    fprintf(fh, "SpamMarkAsRead   = %s\n", Bool2Txt(co->SpamMarkAsRead));
    fprintf(fh, "SpamABookIsWhite = %s\n", Bool2Txt(co->SpamAddressBookIsWhiteList));
    fprintf(fh, "SpamProbThreshold= %d\n", co->SpamProbabilityThreshold);
    fprintf(fh, "SpamFlushInterval= %d\n", co->SpamFlushTrainingDataInterval);
    fprintf(fh, "SpamFlushThres   = %d\n", co->SpamFlushTrainingDataThreshold);
    fprintf(fh, "MoveHamToIncoming= %s\n", Bool2Txt(co->MoveHamToIncoming));
    fprintf(fh, "FilterHam        = %s\n", Bool2Txt(co->FilterHam));

    fprintf(fh, "\n[Read]\n");
    fprintf(fh, "ShowHeader       = %d\n", co->ShowHeader);
    fprintf(fh, "ShortHeaders     = %s\n", co->ShortHeaders);
    fprintf(fh, "ShowSenderInfo   = %d\n", co->ShowSenderInfo);
    fprintf(fh, "WrapHeader       = %s\n", Bool2Txt(co->WrapHeader));
    fprintf(fh, "SigSepLine       = %d\n", co->SigSepLine);
    fprintf(fh, "ColorSignature   = %s\n", co->ColorSignature.buf);
    fprintf(fh, "ColoredText      = %s\n", co->ColoredText.buf);
    fprintf(fh, "Color1stLevel    = %s\n", co->Color1stLevel.buf);
    fprintf(fh, "Color2ndLevel    = %s\n", co->Color2ndLevel.buf);
    fprintf(fh, "Color3rdLevel    = %s\n", co->Color3rdLevel.buf);
    fprintf(fh, "Color4thLevel    = %s\n", co->Color4thLevel.buf);
    fprintf(fh, "ColorURL         = %s\n", co->ColorURL.buf);
    fprintf(fh, "DisplayAllTexts  = %s\n", Bool2Txt(co->DisplayAllTexts));
    fprintf(fh, "FixedFontEdit    = %s\n", Bool2Txt(co->FixedFontEdit));
    fprintf(fh, "UseTextStyles    = %s\n", Bool2Txt(co->UseTextStylesRead));
    fprintf(fh, "TextColorsRead   = %s\n", Bool2Txt(co->UseTextColorsRead));
    fprintf(fh, "DisplayAllAltPart= %s\n", Bool2Txt(co->DisplayAllAltPart));
    fprintf(fh, "MDNEnabled       = %s\n", Bool2Txt(co->MDNEnabled));
    fprintf(fh, "MDN_NoRecipient  = %d\n", co->MDN_NoRecipient);
    fprintf(fh, "MDN_NoDomain     = %d\n", co->MDN_NoDomain);
    fprintf(fh, "MDN_OnDelete     = %d\n", co->MDN_OnDelete);
    fprintf(fh, "MDN_Other        = %d\n", co->MDN_Other);
    fprintf(fh, "MultipleWindows  = %s\n", Bool2Txt(co->MultipleReadWindows));
    fprintf(fh, "StatusChangeDelay= %d\n", co->StatusChangeDelayOn ? co->StatusChangeDelay : -co->StatusChangeDelay);
    fprintf(fh, "ConvertHTML      = %s\n", Bool2Txt(co->ConvertHTML));
    fprintf(fh, "LocalCharset     = %s\n", co->DefaultReadCharset);
    fprintf(fh, "DetectCyrillic   = %s\n", Bool2Txt(co->DetectCyrillic));
    fprintf(fh, "MapForeignChars  = %s\n", Bool2Txt(co->MapForeignChars));
    fprintf(fh, "GlobalMailThreads= %s\n", Bool2Txt(co->GlobalMailThreads));

    fprintf(fh, "\n[Write]\n");
    fprintf(fh, "NewIntro         = %s\n", co->NewIntro);
    fprintf(fh, "Greetings        = %s\n", co->Greetings);
    fprintf(fh, "WarnSubject      = %s\n", Bool2Txt(co->WarnSubject));
    fprintf(fh, "EdWrapCol        = %d\n", co->EdWrapCol);
    fprintf(fh, "EdWrapMode       = %d\n", co->EdWrapMode);
    fprintf(fh, "Editor           = %s\n", co->Editor);
    fprintf(fh, "LaunchAlways     = %s\n", Bool2Txt(co->LaunchAlways));
    fprintf(fh, "EmailCache       = %d\n", co->EmailCache);
    fprintf(fh, "AutoSave         = %d\n", co->AutoSave);
    fprintf(fh, "WriteCharset     = %s\n", co->DefaultWriteCharset);
    fprintf(fh, "FixedFontWrite   = %s\n", Bool2Txt(co->UseFixedFontWrite));
    fprintf(fh, "TextStylesWrite  = %s\n", Bool2Txt(co->UseTextStylesWrite));
    fprintf(fh, "TextColorsWrite  = %s\n", Bool2Txt(co->UseTextColorsWrite));

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
    fprintf(fh, "ForwardMode      = %d\n", co->ForwardMode);
    fprintf(fh, "ForwardIntro     = %s\n", co->ForwardIntro);
    fprintf(fh, "ForwardFinish    = %s\n", co->ForwardFinish);
    fprintf(fh, "QuoteChar        = %s\n", co->QuoteChar);
    fprintf(fh, "AltQuoteChar     = %s\n", co->AltQuoteChar);
    fprintf(fh, "QuoteEmptyLines  = %s\n", Bool2Txt(co->QuoteEmptyLines));
    fprintf(fh, "CompareAddress   = %s\n", Bool2Txt(co->CompareAddress));
    fprintf(fh, "StripSignature   = %s\n", Bool2Txt(co->StripSignature));

    fprintf(fh, "\n[Signature]\n");
    fprintf(fh, "TagsFile         = %s\n", co->TagsFile);
    fprintf(fh, "TagsSeparator    = %s\n", co->TagsSeparator);

    fprintf(fh, "\n[Lists]\n");
    fprintf(fh, "FolderCols       = %d\n", co->FolderCols);
    fprintf(fh, "MessageCols      = %d\n", co->MessageCols);
    fprintf(fh, "FixedFontList    = %s\n", Bool2Txt(co->FixedFontList));
    fprintf(fh, "DateTimeFormat   = %d\n", co->DSListFormat);
    fprintf(fh, "ABookLookup      = %s\n", Bool2Txt(co->ABookLookup));
    fprintf(fh, "FolderCntMenu    = %s\n", Bool2Txt(co->FolderCntMenu));
    fprintf(fh, "MessageCntMenu   = %s\n", Bool2Txt(co->MessageCntMenu));
    fprintf(fh, "FolderInfoMode   = %d\n", co->FolderInfoMode);
    fprintf(fh, "FolderDoubleClick= %s\n", Bool2Txt(co->FolderDoubleClick));

    fprintf(fh, "\n[Security]\n");
    fprintf(fh, "PGPCmdPath       = %s\n", co->PGPCmdPath);
    fprintf(fh, "PGPPassInterval  = %d\n", co->PGPPassInterval);
    fprintf(fh, "LogfilePath      = %s\n", co->LogfilePath);
    fprintf(fh, "LogfileMode      = %d\n", co->LogfileMode);
    fprintf(fh, "SplitLogfile     = %s\n", Bool2Txt(co->SplitLogfile));
    fprintf(fh, "LogAllEvents     = %s\n", Bool2Txt(co->LogAllEvents));

    fprintf(fh, "\n[Start/Quit]\n");
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

    i = 1;
    IterateList(&C->mimeTypeList, curNode)
    {
      struct MimeTypeNode *mtNode = (struct MimeTypeNode *)curNode;

      fprintf(fh, "MV%02d.ContentType = %s\n", i, mtNode->ContentType);
      fprintf(fh, "MV%02d.Extension   = %s\n", i, mtNode->Extension);
      fprintf(fh, "MV%02d.Command     = %s\n", i, mtNode->Command);
      fprintf(fh, "MV%02d.Description = %s\n", i, mtNode->Description);

      i++;
    }

    fprintf(fh, "\n[Address book]\n");
    fprintf(fh, "GalleryDir       = %s\n", co->GalleryDir);
    fprintf(fh, "ProxyServer      = %s\n", co->ProxyServer);
    fprintf(fh, "NewAddrGroup     = %s\n", co->NewAddrGroup);
    fprintf(fh, "AddToAddrbook    = %d\n", co->AddToAddrbook);
    fprintf(fh, "AddrbookCols     = %d\n", co->AddrbookCols);

    fprintf(fh, "\n[Scripts]\n");
    for(i = 0; i < MAXRX; i++)
    {
      if(i < 10)
        fprintf(fh, "Rexx%02d.Name      = %s\n", i, co->RX[i].Name);

      fprintf(fh, "Rexx%02d.Script    = %s\n", i, co->RX[i].Script);
      fprintf(fh, "Rexx%02d.IsAmigaDOS= %s\n", i, Bool2Txt(co->RX[i].IsAmigaDOS));
      fprintf(fh, "Rexx%02d.UseConsole= %s\n", i, Bool2Txt(co->RX[i].UseConsole));
      fprintf(fh, "Rexx%02d.WaitTerm  = %s\n", i, Bool2Txt(co->RX[i].WaitTerm));
    }

    fprintf(fh, "\n[Mixed]\n");
    fprintf(fh, "TempDir          = %s\n", co->TempDir);
    fprintf(fh, "DetachDir        = %s\n", co->DetachDir);
    fprintf(fh, "AttachDir        = %s\n", co->AttachDir);
    fprintf(fh, "WBAppIcon        = %s\n", Bool2Txt(co->WBAppIcon));
    fprintf(fh, "IconPosition     = %d;%d\n", co->IconPositionX, co->IconPositionY);
    fprintf(fh, "AppIconText      = %s\n", co->AppIconText);
    fprintf(fh, "DockyIcon        = %s\n", Bool2Txt(co->DockyIcon));
    fprintf(fh, "IconifyOnQuit    = %s\n", Bool2Txt(co->IconifyOnQuit));
    fprintf(fh, "Confirm          = %s\n", Bool2Txt(co->Confirm));
    fprintf(fh, "ConfirmDelete    = %d\n", co->ConfirmDelete);
    fprintf(fh, "RemoveAtOnce     = %s\n", Bool2Txt(co->RemoveAtOnce));
    fprintf(fh, "XPKPack          = %s;%d\n", co->XPKPack, co->XPKPackEff);
    fprintf(fh, "XPKPackEncrypt   = %s;%d\n", co->XPKPackEncrypt, co->XPKPackEncryptEff);
    fprintf(fh, "PackerCommand    = %s\n", co->PackerCommand);


    fprintf(fh, "\n[Look&Feel]\n");
    fprintf(fh, "Theme            = %s\n", co->ThemeName);
    fprintf(fh, "InfoBar          = %d\n", co->InfoBar);
    fprintf(fh, "InfoBarText      = %s\n", co->InfoBarText);
    fprintf(fh, "QuickSearchBar   = %s\n", Bool2Txt(co->QuickSearchBar));
    fprintf(fh, "EmbeddedReadPane = %s\n", Bool2Txt(co->EmbeddedReadPane));
    fprintf(fh, "SizeFormat       = %d\n", co->SizeFormat);

    fprintf(fh, "\n[Update]\n");
    fprintf(fh, "UpdateInterval     = %d\n", co->UpdateInterval);
    fprintf(fh, "UpdateServer       = %s\n", co->UpdateServer);
    fprintf(fh, "UpdateDownloadPath = %s\n", co->UpdateDownloadPath);

    fprintf(fh, "\n[Advanced]\n");
    fprintf(fh, "LetterPart               = %d\n", co->LetterPart);
    fprintf(fh, "WriteIndexes             = %d\n", co->WriteIndexes);
    fprintf(fh, "ExpungeIndexes           = %d\n", co->ExpungeIndexes);
    fprintf(fh, "SupportSite              = %s\n", co->SupportSite);
    fprintf(fh, "JumpToNewMsg             = %s\n", Bool2Txt(co->JumpToNewMsg));
    fprintf(fh, "JumpToIncoming           = %s\n", Bool2Txt(co->JumpToIncoming));
    fprintf(fh, "JumpToRecentMsg          = %s\n", Bool2Txt(co->JumpToRecentMsg));
    fprintf(fh, "AskJumpUnread            = %s\n", Bool2Txt(co->AskJumpUnread));
    fprintf(fh, "PrinterCheck             = %s\n", Bool2Txt(co->PrinterCheck));
    fprintf(fh, "IsOnlineCheck            = %s\n", Bool2Txt(co->IsOnlineCheck));
    fprintf(fh, "IOCInterface             = %s\n", co->IOCInterfaces);
    fprintf(fh, "ConfirmOnQuit            = %s\n", Bool2Txt(co->ConfirmOnQuit));
    fprintf(fh, "HideGUIElements          = %d\n", co->HideGUIElements);
    fprintf(fh, "SysCharsetCheck          = %s\n", Bool2Txt(co->SysCharsetCheck));
    fprintf(fh, "AmiSSLCheck              = %s\n", Bool2Txt(co->AmiSSLCheck));
    fprintf(fh, "TimeZoneCheck            = %s\n", Bool2Txt(co->TimeZoneCheck));
    fprintf(fh, "AutoDSTCheck             = %s\n", Bool2Txt(co->AutoDSTCheck));
    fprintf(fh, "StackSize                = %d\n", co->StackSize);
    fprintf(fh, "PrintMethod              = %d\n", co->PrintMethod);
    fprintf(fh, "AutoColumnResize         = %s\n", Bool2Txt(co->AutoColumnResize));

    // prepare the socket option string
    buf[0] = '\0'; // clear it first
    if(co->SocketOptions.KeepAlive == TRUE)
      strlcat(buf, " SO_KEEPALIVE", sizeof(buf));
    if(co->SocketOptions.NoDelay == TRUE)
      strlcat(buf, " TCP_NODELAY", sizeof(buf));
    if(co->SocketOptions.LowDelay == TRUE)
      strlcat(buf, " IPTOS_LOWDELAY", sizeof(buf));
    if(co->SocketOptions.SendBuffer > -1)
      snprintf(&buf[strlen(buf)], sizeof(buf)-strlen(buf), " SO_SNDBUF=%d", (int)co->SocketOptions.SendBuffer);
    if(co->SocketOptions.RecvBuffer > -1)
      snprintf(&buf[strlen(buf)], sizeof(buf)-strlen(buf), " SO_RCVBUF=%d", (int)co->SocketOptions.RecvBuffer);
    if(co->SocketOptions.SendLowAt > -1)
      snprintf(&buf[strlen(buf)], sizeof(buf)-strlen(buf), " SO_SNDLOWAT=%d", (int)co->SocketOptions.SendLowAt);
    if(co->SocketOptions.RecvLowAt > -1)
      snprintf(&buf[strlen(buf)], sizeof(buf)-strlen(buf), " SO_RCVLOWAT=%d", (int)co->SocketOptions.RecvLowAt);
    if(co->SocketOptions.SendTimeOut > -1)
      snprintf(&buf[strlen(buf)], sizeof(buf)-strlen(buf), " SO_SNDTIMEO=%d", (int)co->SocketOptions.SendTimeOut);
    if(co->SocketOptions.RecvTimeOut > -1)
      snprintf(&buf[strlen(buf)], sizeof(buf)-strlen(buf), " SO_RCVTIMEO=%d", (int)co->SocketOptions.RecvTimeOut);

    fprintf(fh, "SocketOptions            =%s\n", buf);
    fprintf(fh, "SocketTimeout            = %d\n", co->SocketTimeout);
    fprintf(fh, "TRBufferSize             = %d\n", co->TRBufferSize);
    fprintf(fh, "EmbeddedMailDelay        = %d\n", co->EmbeddedMailDelay);
    fprintf(fh, "KeepAliveInterval        = %d\n", co->KeepAliveInterval);
    fprintf(fh, "StyleFGroupUnread        = %s\n", MUIStyle2String(co->StyleFGroupUnread));
    fprintf(fh, "StyleFGroupRead          = %s\n", MUIStyle2String(co->StyleFGroupRead));
    fprintf(fh, "StyleFolderUnread        = %s\n", MUIStyle2String(co->StyleFolderUnread));
    fprintf(fh, "StyleFolderRead          = %s\n", MUIStyle2String(co->StyleFolderRead));
    fprintf(fh, "StyleFolderNew           = %s\n", MUIStyle2String(co->StyleFolderNew));
    fprintf(fh, "StyleMailUnread          = %s\n", MUIStyle2String(co->StyleMailUnread));
    fprintf(fh, "StyleMailRead            = %s\n", MUIStyle2String(co->StyleMailRead));
    fprintf(fh, "AutoClip                 = %s\n", Bool2Txt(co->AutoClip));
    fprintf(fh, "ShowFilterStats          = %s\n", Bool2Txt(co->ShowFilterStats));
    fprintf(fh, "ConfirmRemoveAttachments = %s\n", Bool2Txt(co->ConfirmRemoveAttachments));

    DateStamp2String(buf, sizeof(buf), &co->BirthdayCheckTime, DSS_SHORTTIME, TZC_NONE);
    fprintf(fh, "BirthdayCheckTime        = %s\n", buf);

    // analyze if we really didn't meet an error during the
    // numerous write operations
    if(ferror(fh) == 0)
    {
      result = TRUE;

      // remember that this configuration has been saved
      co->ConfigIsSaved = TRUE;

      D(DBF_CONFIG, "configuration successfully saved");

      // append something to the logfile
      AppendToLogfile(LF_VERBOSE, 60, tr(MSG_LOG_SavingConfig), fname);
    }
    else
      E(DBF_CONFIG, "error during config save operation");

    fclose(fh);
  }
  else
    ER_NewError(tr(MSG_ER_CantCreateFile), fname);

  RETURN(result);
  return result;
}

///
/// CO_LoadConfig
//  Loads configuration from a file
BOOL CO_LoadConfig(struct Config *co, char *fname, struct FolderList **oldfolders)
{
  BOOL success = FALSE;
  FILE *fh;

  ENTER();

  D(DBF_CONFIG, "about to load configuration from '%s'", fname);

  // flag this configuration as not (properly) saved in
  // advanced, just in case something goes wrong.
  co->ConfigIsSaved = FALSE;

  if((fh = fopen(fname, "r")) != NULL)
  {
    char *buf = NULL;
    size_t buflen = 0;

    setvbuf(fh, NULL, _IOFBF, SIZE_FILEBUF);

    if(getline(&buf, &buflen, fh) >= 3 && strnicmp(buf, "YCO", 3) == 0)
    {
      int version = buf[3]-'0';
      struct FolderList *ofo = NULL;
      struct FilterNode *lastFilter = NULL;
      int lastTypeID = -1;
      struct MimeTypeNode *lastType = NULL;
      int lastFilterID = -1;
      struct MailServerNode *fPOP3;
      struct MailServerNode *fSMTP;
      struct UserIdentityNode *fUserIdentity;
      BOOL foundGlobalPOP3Options = FALSE;
      int globalPOP3AvoidDuplicates = -1;
      int globalDownloadOnStartup = -1;
      int globalPOP3Preselection = -1;

      // set defaults and make the configuration actually
      // useall (reset it, e.g.)
      CO_SetDefaults(co, cp_AllPages);

      // get the first POP3/SMTP server out of our config data
      fPOP3 = GetMailServer(&co->mailServerList, MST_POP3, 0);
      fSMTP = GetMailServer(&co->mailServerList, MST_SMTP, 0);
      ASSERT(fPOP3 != NULL);
      ASSERT(fSMTP != NULL);

      // get the first User Identity out of our config data
      fUserIdentity = GetUserIdentity(&co->userIdentityList, 0, TRUE);
      ASSERT(fUserIdentity != NULL);

      while(getline(&buf, &buflen, fh) > 0)
      {
        char *p;
        char *value;
        const char *value2 = "";

        if((value = strchr(buf, '=')) != NULL)
        {
          for(value2 = (++value)+1; isspace(*value); value++)
            ;
        }

        if((p = strpbrk(buf,"\r\n")) != NULL)
          *p = '\0';

        for(p = buf; *p != '\0' && *p != '=' && !isspace(*p); p++)
          ;

        *p = '\0';

        if(*buf != '\0' && value != NULL)
        {
          // check for an old config version and try to import its values
          if(version == 2)
          {
            if(stricmp(buf, "POP3-Server") == 0)                 strlcpy(fPOP3->hostname, value, sizeof(fPOP3->hostname));
            else if(stricmp(buf, "POP3-Password") == 0)          strlcpy(fPOP3->password, Decrypt(value), sizeof(fPOP3->password));
            else if(stricmp(buf, "POP3-User") == 0)              strlcpy(fPOP3->username, value, sizeof(fPOP3->username));
            else if(stricmp(buf, "DeleteOnServer") == 0)         Txt2Bool(value) == TRUE ? SET_FLAG(fPOP3->flags, MSF_PURGEMESSGAES) : CLEAR_FLAG(fPOP3->flags, MSF_PURGEMESSGAES);
            else if(stricmp(buf, "CheckMail") == 0)
            {
              if(Txt2Bool(value) == FALSE)
                co->CheckMailDelay = 0;
            }
            else if(stricmp(buf, "Verbosity") == 0)              co->TransferWindow = atoi(value) > 0 ? TWM_SHOW : TWM_HIDE;
            else if(stricmp(buf, "WordWrap") == 0)               co->EdWrapCol = atoi(value);
            else if(stricmp(buf, "DeleteOnExit") == 0)           co->RemoveAtOnce = !(co->RemoveOnQuit = Txt2Bool(value));
            else if(strnicmp(buf, "Folder", 6) == 0 && oldfolders != NULL)
            {
              if(ofo == NULL)
              {
                ofo = CreateFolderList();
                *oldfolders = ofo;
              }
              if(ofo != NULL)
              {
                int type;
                struct Folder *folder;

                switch(atoi(&buf[6]))
                {
                  case 0:
                    type = FT_INCOMING;
                  break;

                  case 1:
                    type = FT_OUTGOING;
                  break;

                  case 2:
                    type = FT_SENT;
                  break;

                  default:
                    type = FT_CUSTOM;
                  break;
                }

                if((p = strchr(&value[4], ';')) != NULL)
                  *p++ = '\0';

                if((folder = FO_NewFolder(type, &value[4], p)) != NULL)
                {
                  static const int sortconv[4] = { -1, 1, 3, 5 };

                  folder->Sort[0] = sortconv[atoi(&value[2])];
                  AddNewFolderNode(ofo, folder);
                }
              }
            }
            else if(strnicmp(buf, "Rule", 4) == 0)
            {
              struct FilterNode *filter;

              if((filter = CreateNewFilter()) != NULL)
              {
                struct RuleNode *rule;
                char *p2;

                filter->applyToNew = filter->applyOnReq = Txt2Bool(value);
                p = strchr(p2 = &value[2], ';');
                *p++ = '\0';
                strlcpy(filter->name, p2, sizeof(filter->name));

                // get the first rule (always existent) and fill in data
                rule = (struct RuleNode *)GetHead((struct List *)&filter->ruleList);
                if((rule->searchMode = atoi(p)) == 2)
                  rule->searchMode = SM_SUBJECT;

                if(Txt2Bool(&p[2]) == TRUE)
                  SET_FLAG(rule->flags, SEARCHF_CASE_SENSITIVE);
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
                if(*filter->forwardTo != '\0')
                  SET_FLAG(filter->actions, FA_FORWARD);

                AddTail((struct List *)&co->filterList, (struct Node *)filter);
              }
            }
            else if(strnicmp(buf, "MimeViewer", 10) == 0)
            {
              int j = atoi(&buf[10]);
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
            else if(strnicmp(buf, "RexxMenu", 8) == 0)
            {
              int j = atoi(&buf[8]);

              strlcpy(co->RX[j].Name, (char *)FilePart(value), sizeof(co->RX[j].Name));
              strlcpy(co->RX[j].Script, value, sizeof(co->RX[j].Script));
            }
          } // version == 2

          if(strnicmp(buf, "FolderPath", 10) == 0 && oldfolders != NULL)
          {
            if(ofo == NULL)
            {
              ofo = CreateFolderList();
              *oldfolders = ofo;
            }
            if(ofo != NULL)
            {
              struct Folder *folder;

              if((folder = FO_NewFolder(FT_CUSTOM, value, (char *)FilePart(value))) != NULL)
              {
                if(AddNewFolderNode(ofo, folder) != NULL)
                {
                  if(FO_LoadConfig(folder) == FALSE)
                    FO_SaveConfig(folder);
                }
              }
            }
          }

/* First Steps */
          else if(stricmp(buf, "TimeZone") == 0)                 co->TimeZone = atoi(value);
          else if(stricmp(buf, "DaylightSaving") == 0)           co->DaylightSaving = Txt2Bool(value);

/* TCP/IP */
          else if(stricmp(buf, "SMTP-ID") == 0)                  fSMTP->id = strtol(value, NULL, 16);
          else if(stricmp(buf, "SMTP-Enabled") == 0)             Txt2Bool(value) == TRUE ? SET_FLAG(fSMTP->flags, MSF_ACTIVE) : CLEAR_FLAG(fSMTP->flags, MSF_ACTIVE);
          else if(stricmp(buf, "SMTP-Description") == 0)         strlcpy(fSMTP->description, value, sizeof(fSMTP->description));
          else if(stricmp(buf, "SMTP-Server") == 0)              strlcpy(fSMTP->hostname, value, sizeof(fSMTP->hostname));
          else if(stricmp(buf, "SMTP-Port") == 0)                fSMTP->port = atoi(value);
          else if(stricmp(buf, "SMTP-SecMethod") == 0)           SET_FLAG(fSMTP->flags, SMTPSecMethod2MSF(atoi(value)));
          else if(stricmp(buf, "Allow8bit") == 0)                Txt2Bool(value) == TRUE ? SET_FLAG(fSMTP->flags, MSF_ALLOW_8BIT) : CLEAR_FLAG(fSMTP->flags, MSF_ALLOW_8BIT);
          else if(stricmp(buf, "Use-SMTP-TLS") == 0)             SET_FLAG(fSMTP->flags, SMTPSecMethod2MSF(atoi(value))); // obsolete
          else if(stricmp(buf, "Use-SMTP-AUTH") == 0)            Txt2Bool(value) == TRUE ? SET_FLAG(fSMTP->flags, MSF_AUTH) : CLEAR_FLAG(fSMTP->flags, MSF_AUTH);
          else if(stricmp(buf, "SMTP-AUTH-User") == 0)           strlcpy(fSMTP->username, value, sizeof(fSMTP->username));
          else if(stricmp(buf, "SMTP-AUTH-Pass") == 0)           strlcpy(fSMTP->password, Decrypt(value), sizeof(fSMTP->password));
          else if(stricmp(buf, "SMTP-AUTH-Method") == 0)         SET_FLAG(fSMTP->flags, SMTPAuthMethod2MSF(atoi(value)));
          else if(strnicmp(buf,"SMTP", 4) == 0 && isdigit(buf[4]) && isdigit(buf[5]) && strchr(buf, '.') != NULL)
          {
            int id = atoi(&buf[4]);

            if(id >= 0)
            {
              struct MailServerNode *msn;

              // try to get the SMTP server structure with the found id or create
              // a new one
              if((msn = GetMailServer(&co->mailServerList, MST_SMTP, id)) == NULL)
              {
                if((msn = CreateNewMailServer(MST_SMTP, co, FALSE)) != NULL)
                  AddTail((struct List *)&co->mailServerList, (struct Node *)msn);
                else
                  E(DBF_CONFIG, "Couldn't create new SMTP structure %ld", id);
              }

              if(msn != NULL)
              {
                char *q = strchr(buf, '.')+1;

                // now find out which subtype this smtp configuration is
                if(stricmp(q, "ID") == 0)                        msn->id = strtol(value, NULL, 16);
                else if(stricmp(q, "Account") == 0)              strlcpy(msn->description, value, sizeof(msn->description));
                else if(stricmp(q, "Server") == 0)               strlcpy(msn->hostname, value, sizeof(msn->hostname));
                else if(stricmp(q, "Port") == 0)                 msn->port = atoi(value);
                else if(stricmp(q, "Enabled") == 0)              Txt2Bool(value) == TRUE ? SET_FLAG(msn->flags, MSF_ACTIVE) : CLEAR_FLAG(msn->flags, MSF_ACTIVE);
                else if(stricmp(q, "SecMethod") == 0)            SET_FLAG(msn->flags, SMTPSecMethod2MSF(atoi(value)));
                else if(stricmp(q, "Allow8bit") == 0)            Txt2Bool(value) == TRUE ? SET_FLAG(msn->flags, MSF_ALLOW_8BIT) : CLEAR_FLAG(msn->flags, MSF_ALLOW_8BIT);
                else if(stricmp(q, "SMTP-AUTH") == 0)            Txt2Bool(value) == TRUE ? SET_FLAG(msn->flags, MSF_AUTH) : CLEAR_FLAG(msn->flags, MSF_AUTH);
                else if(stricmp(q, "AUTH-User") == 0)            strlcpy(msn->username, value, sizeof(msn->username));
                else if(stricmp(q, "AUTH-Pass") == 0)            strlcpy(msn->password, Decrypt(value), sizeof(msn->password));
                else if(stricmp(q, "AUTH-Method") == 0)          SET_FLAG(msn->flags, SMTPAuthMethod2MSF(atoi(value)));
                else
                  W(DBF_CONFIG, "unknown '%s' SMTP config tag", q);
              }
              else
                break;
            }
            else
              W(DBF_CONFIG, "SMTP id < 0 : %ld", id);
          }
          else if(strnicmp(buf,"POP", 3) == 0 && isdigit(buf[3]) && isdigit(buf[4]) && strchr(buf, '.') != NULL)
          {
            int id = atoi(&buf[3]);

            if(id >= 0)
            {
              struct MailServerNode *msn;

              // try to get the POP3 server structure with the found id or create
              // a new one
              if((msn = GetMailServer(&co->mailServerList, MST_POP3, id)) == NULL)
              {
                if((msn = CreateNewMailServer(MST_POP3, co, FALSE)) != NULL)
                  AddTail((struct List *)&co->mailServerList, (struct Node *)msn);
                else
                  E(DBF_CONFIG, "Couldn't create new POP3 structure %ld", id);
              }

              if(msn != NULL)
              {
                char *q = strchr(buf, '.')+1;

                if(stricmp(q, "ID") == 0)                        msn->id = strtol(value, NULL, 16);
                else if(stricmp(q, "Account") == 0)              strlcpy(msn->description, value, sizeof(msn->description));
                else if(stricmp(q, "Server") == 0)               strlcpy(msn->hostname, value, sizeof(msn->hostname));
                else if(stricmp(q, "Port") == 0)                 msn->port = atoi(value);
                else if(stricmp(q, "Password") == 0)             strlcpy(msn->password, Decrypt(value), sizeof(msn->password));
                else if(stricmp(q, "User") == 0)                 strlcpy(msn->username, value, sizeof(msn->username));
                else if(stricmp(q, "Enabled") == 0)              Txt2Bool(value) == TRUE ? SET_FLAG(msn->flags, MSF_ACTIVE) : CLEAR_FLAG(msn->flags, MSF_ACTIVE);
                else if(stricmp(q, "SSLMode") == 0)              SET_FLAG(msn->flags, POP3SecMethod2MSF(atoi(value)));
                else if(stricmp(q, "UseAPOP") == 0)              Txt2Bool(value) == TRUE ? SET_FLAG(msn->flags, MSF_APOP) : CLEAR_FLAG(msn->flags, MSF_APOP);
                else if(stricmp(q, "Delete") == 0)               Txt2Bool(value) == TRUE ? SET_FLAG(msn->flags, MSF_PURGEMESSGAES) : CLEAR_FLAG(msn->flags, MSF_PURGEMESSGAES);
                else if(stricmp(q, "AvoidDuplicates") == 0)      Txt2Bool(value) == TRUE ? SET_FLAG(msn->flags, MSF_AVOID_DUPLICATES) : CLEAR_FLAG(msn->flags, MSF_AVOID_DUPLICATES);
                else if(stricmp(q, "ApplyRemoteFilters") == 0)   Txt2Bool(value) == TRUE ? SET_FLAG(msn->flags, MSF_APPLY_REMOTE_FILTERS) : CLEAR_FLAG(msn->flags, MSF_APPLY_REMOTE_FILTERS);
                else if(stricmp(q, "Preselection") == 0)         msn->preselection = atoi(value);
                else if(stricmp(q, "DownloadOnStartup") == 0)    Txt2Bool(value) == TRUE ? SET_FLAG(msn->flags, MSF_DOWNLOAD_ON_STARTUP) : CLEAR_FLAG(msn->flags, MSF_DOWNLOAD_ON_STARTUP);
                else
                  W(DBF_CONFIG, "unknown '%s' POP config tag", q);
              }
              else
                break;
            }
            else
              W(DBF_CONFIG, "POP3 id < 0 : %ld", id);
          }

/* New mail */
          else if(stricmp(buf, "AvoidDuplicates") == 0)          { globalPOP3AvoidDuplicates = Txt2Bool(value); foundGlobalPOP3Options = TRUE; }
          else if(stricmp(buf, "PreSelection") == 0)             { globalPOP3Preselection = atoi(value); foundGlobalPOP3Options = TRUE; }
          else if(stricmp(buf, "TransferWindow") == 0)           co->TransferWindow = atoi(value);
          else if(stricmp(buf, "UpdateStatus") == 0)             co->UpdateStatus = Txt2Bool(value);
          else if(stricmp(buf, "WarnSize") == 0)                 co->WarnSize = atoi(value);
          else if(stricmp(buf, "CheckMailDelay") == 0)           co->CheckMailDelay = atoi(value);
          else if(stricmp(buf, "DownloadLarge") == 0)            co->DownloadLarge = Txt2Bool(value);
          else if(stricmp(buf, "NotifyType") == 0)               co->NotifyType = atoi(value);
          else if(stricmp(buf, "NotifySound") == 0)              strlcpy(co->NotifySound, value, sizeof(co->NotifySound));
          else if(stricmp(buf, "NotifyCommand") == 0)            strlcpy(co->NotifyCommand, value, sizeof(co->NotifyCommand));

/* Identities */
          else if(strnicmp(buf,"ID", 2) == 0 && isdigit(buf[2]) && isdigit(buf[3]) && strchr(buf, '.') != NULL)
          {
            int id = atoi(&buf[2]);

            if(id >= 0)
            {
              struct UserIdentityNode *uin;

              // try to get the UserIdentityNode structure with the found id or create
              // a new one
              if((uin = GetUserIdentity(&co->userIdentityList, id, FALSE)) == NULL)
              {
                if((uin = CreateNewUserIdentity(co)) != NULL)
                  AddTail((struct List *)&co->userIdentityList, (struct Node *)uin);
                else
                  E(DBF_CONFIG, "Couldn't create new UserIdentity structure %ld", id);
              }

              if(uin != NULL)
              {
                char *q = strchr(buf, '.')+1;

                // now find out which subtype this smtp configuration is
                if(stricmp(q, "Enabled") == 0)                   uin->active = Txt2Bool(value);
                else if(stricmp(q, "Description") == 0)          strlcpy(uin->description, value, sizeof(uin->description));
                else if(stricmp(q, "Realname") == 0)             strlcpy(uin->realname, value, sizeof(uin->realname));
                else if(stricmp(q, "Address") == 0)              strlcpy(uin->address, value, sizeof(uin->address));
                else if(stricmp(q, "Organization") == 0)         strlcpy(uin->organization, value, sizeof(uin->organization));
                else if(stricmp(q, "MailServerID") == 0)
                {
                  long mailServerID = strtol(value, NULL, 16);

                  if(mailServerID > 0)
                  {
                    struct Node *curNode;

                    IterateList(&co->mailServerList, curNode)
                    {
                      struct MailServerNode *msn = (struct MailServerNode *)curNode;

                      // check if we found exactly this ID
                      if(msn->id == mailServerID && isSMTPServer(msn))
                      {
                        uin->mailServer = msn;
                        break;
                      }
                    }
                  }

                  if(uin->mailServer == NULL)
                    W(DBF_CONFIG, "Couldn't find SMTP Server with ID '%s' for Identity %d", value, id);
                }
                else if(stricmp(q, "Signature") == 0)            uin->signature = atoi(value);
                else if(stricmp(q, "MailCC") == 0)               strlcpy(uin->mailCC, value, sizeof(uin->mailCC));
                else if(stricmp(q, "MailBCC") == 0)              strlcpy(uin->mailBCC, value, sizeof(uin->mailBCC));
                else if(stricmp(q, "MailReplyTo") == 0)          strlcpy(uin->mailReplyTo, value, sizeof(uin->mailReplyTo));
                else if(stricmp(q, "ExtraHeaders") == 0)         strlcpy(uin->extraHeaders, value, sizeof(uin->extraHeaders));
                else if(stricmp(q, "PhotoURL") == 0)             strlcpy(uin->photoURL, value, sizeof(uin->photoURL));
                else if(stricmp(q, "SentFolder") == 0)           strlcpy(uin->sentFolder, value, sizeof(uin->sentFolder));
                else if(stricmp(q, "SaveSentMail") == 0)         uin->saveSentMail = Txt2Bool(value);
                else if(stricmp(q, "QuoteMails") == 0)           uin->quoteMails = Txt2Bool(value);
                else if(stricmp(q, "QuotePosition") == 0)        uin->quotePosition = atoi(value);
                else if(stricmp(q, "SignaturePosition") == 0)    uin->signaturePosition = atoi(value);
                else if(stricmp(q, "SignatureReply") == 0)       uin->sigReply = Txt2Bool(value);
                else if(stricmp(q, "SignatureForward") == 0)     uin->sigForwarding = Txt2Bool(value);
                else if(stricmp(q, "AddPersonalInfo") == 0)      uin->addPersonalInfo = Txt2Bool(value);
                else if(stricmp(q, "RequestMDN") == 0)           uin->requestMDN = Txt2Bool(value);
                else if(stricmp(q, "UsePGP") == 0)               uin->usePGP = Txt2Bool(value);
                else if(stricmp(q, "PGPKeyID") == 0)             strlcpy(uin->pgpKeyID, value, sizeof(uin->pgpKeyID));
                else if(stricmp(q, "PGPKeyURL") == 0)            strlcpy(uin->pgpKeyURL, value, sizeof(uin->pgpKeyURL));
                else if(stricmp(q, "PGPSignUnencrypted") == 0)   uin->pgpSignUnencrypted = Txt2Bool(value);
                else if(stricmp(q, "PGPSignEncrypted") == 0)     uin->pgpSignEncrypted = Txt2Bool(value);
                else if(stricmp(q, "PGPEncryptAll") == 0)        uin->pgpEncryptAll = Txt2Bool(value);
                else if(stricmp(q, "PGPSelfEncrypt") == 0)       uin->pgpSelfEncrypt = Txt2Bool(value);
                else
                  W(DBF_CONFIG, "unknown '%s' ID config tag", q);
              }
              else
                break;
            }
            else
              W(DBF_CONFIG, "IDENTITY id < 0 : %ld", id);
          }
/* Filters */
          else if(strnicmp(buf, "FI", 2) == 0 && isdigit(buf[2]) && isdigit(buf[3]) && strchr(buf, '.'))
          {
            int curFilterID = atoi(&buf[2]);
            char *q = strchr(buf, '.')+1;

            if(curFilterID >= 0)
            {
              if(lastFilter != NULL && lastFilterID != curFilterID)
              {
                int i;
                struct Node *curNode;

                // reset the lastFilter
                lastFilter = NULL;
                lastFilterID = -1;

                // try to get the filter with that particular filter ID out of our
                // filterList
                i = 0;
                IterateList(&co->filterList, curNode)
                {
                  if(i == curFilterID)
                  {
                    lastFilter = (struct FilterNode *)curNode;
                    lastFilterID = i;
                    break;
                  }

                  i++;
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
              if(stricmp(q, "Name") == 0)                        strlcpy(lastFilter->name, value, sizeof(lastFilter->name));
              else if(stricmp(q, "Remote") == 0)                 lastFilter->remote = Txt2Bool(value);
              else if(stricmp(q, "ApplyToNew") == 0)             lastFilter->applyToNew = Txt2Bool(value);
              else if(stricmp(q, "ApplyToSent") == 0)            lastFilter->applyToSent = Txt2Bool(value);
              else if(stricmp(q, "ApplyOnReq") == 0)             lastFilter->applyOnReq = Txt2Bool(value);
              else if(stricmp(q, "Actions") == 0)                lastFilter->actions = atoi(value);
              else if(stricmp(q, "BounceTo") == 0)               strlcpy(lastFilter->bounceTo, value, sizeof(lastFilter->bounceTo));
              else if(stricmp(q, "ForwardTo") == 0)              strlcpy(lastFilter->forwardTo, value, sizeof(lastFilter->forwardTo));
              else if(stricmp(q, "ReplyFile") == 0)              strlcpy(lastFilter->replyFile, value, sizeof(lastFilter->replyFile));
              else if(stricmp(q, "ExecuteCmd") == 0)             strlcpy(lastFilter->executeCmd, value, sizeof(lastFilter->executeCmd));
              else if(stricmp(q, "PlaySound") == 0)              strlcpy(lastFilter->playSound, value, sizeof(lastFilter->playSound));
              else if(stricmp(q, "MoveTo") == 0)                 strlcpy(lastFilter->moveTo, value, sizeof(lastFilter->moveTo));
              else
              {
                struct RuleNode *rule;

                // if nothing of the above string matched than the FI string
                // is probably a rule definition so we check it here

                if(strnicmp(q, "Field", 5) == 0)
                {
                  int n = atoi(q+5);

                  while((rule = GetFilterRule(lastFilter, n>0 ? n-1 : 0)) == NULL)
                    CreateNewRule(lastFilter, TRUE);

                  rule->searchMode = atoi(value);
                }
                else if(strnicmp(q, "SubField", 8) == 0)
                {
                  int n = atoi(q+8);

                  while((rule = GetFilterRule(lastFilter, n>0 ? n-1 : 0)) == NULL)
                    CreateNewRule(lastFilter, TRUE);

                  rule->subSearchMode = atoi(value);
                }
                else if(strnicmp(q, "CustomField", 11) == 0)
                {
                  int n = atoi(q+11);

                  while((rule = GetFilterRule(lastFilter, n>0 ? n-1 : 0)) == NULL)
                    CreateNewRule(lastFilter, TRUE);

                  strlcpy(rule->customField, value, sizeof(rule->customField));
                }
                else if(strnicmp(q, "Comparison", 10) == 0)
                {
                  int n = atoi(q+10);

                  while((rule = GetFilterRule(lastFilter, n>0 ? n-1 : 0)) == NULL)
                    CreateNewRule(lastFilter, TRUE);

                  rule->comparison = atoi(value);
                }
                else if(strnicmp(q, "Match", 5) == 0)
                {
                  int n = atoi(q+5);

                  while((rule = GetFilterRule(lastFilter, n>0 ? n-1 : 0)) == NULL)
                    CreateNewRule(lastFilter, TRUE);

                  strlcpy(rule->matchPattern, value2, sizeof(rule->matchPattern));
                }
                else if(strnicmp(q, "CaseSens", 8) == 0)
                {
                  int n = atoi(q+8);

                  while((rule = GetFilterRule(lastFilter, n>0 ? n-1 : 0)) == NULL)
                    CreateNewRule(lastFilter, TRUE);

                  if(Txt2Bool(value) == TRUE)
                    SET_FLAG(rule->flags, SEARCHF_CASE_SENSITIVE);
                  else
                    CLEAR_FLAG(rule->flags, SEARCHF_CASE_SENSITIVE);
                }
                else if(strnicmp(q, "Substring", 9) == 0)
                {
                  int n = atoi(q+9);

                  while((rule = GetFilterRule(lastFilter, n>0 ? n-1 : 0)) == NULL)
                    CreateNewRule(lastFilter, TRUE);

                  if(Txt2Bool(value) == TRUE)
                    SET_FLAG(rule->flags, SEARCHF_SUBSTRING);
                  else
                    CLEAR_FLAG(rule->flags, SEARCHF_SUBSTRING);
                }
                else if(strnicmp(q, "DOSPattern", 10) == 0)
                {
                  int n = atoi(q+10);

                  while((rule = GetFilterRule(lastFilter, n>0 ? n-1 : 0)) == NULL)
                    CreateNewRule(lastFilter, TRUE);

                  if(Txt2Bool(value) == TRUE)
                    SET_FLAG(rule->flags, SEARCHF_DOS_PATTERN);
                  else
                    CLEAR_FLAG(rule->flags, SEARCHF_DOS_PATTERN);
                }
                else if(strnicmp(q, "Combine", 7) == 0 && atoi(value) > CB_NONE)
                {
                  int n = atoi(q+7);

                  // here we use n and not n-1 on purpose because the combine line
                  // refers always to the next one.
                  while((rule = GetFilterRule(lastFilter, n>0 ? n : 1)) == NULL)
                    CreateNewRule(lastFilter, TRUE);

                  rule->combine = atoi(value);
                }
              }
            }
          }

/* Spam */
          else if(stricmp(buf, "SpamFilterEnabled") == 0)        co->SpamFilterEnabled = Txt2Bool(value);
          else if(stricmp(buf, "SpamFilterForNew") == 0)         co->SpamFilterForNewMail = Txt2Bool(value);
          else if(stricmp(buf, "SpamMarkOnMove") == 0)           co->SpamMarkOnMove = Txt2Bool(value);
          else if(stricmp(buf, "SpamMarkAsRead") == 0)           co->SpamMarkAsRead = Txt2Bool(value);
          else if(stricmp(buf, "SpamABookIsWhite") == 0)         co->SpamAddressBookIsWhiteList = Txt2Bool(value);
          else if(stricmp(buf, "SpamProbThreshold") == 0)        co->SpamProbabilityThreshold = atoi(value);
          else if(stricmp(buf, "SpamFlushInterval") == 0)        co->SpamFlushTrainingDataInterval = atoi(value);
          else if(stricmp(buf, "SpamFlushThres") == 0)           co->SpamFlushTrainingDataThreshold = atoi(value);
          else if(stricmp(buf, "MoveHamToIncoming") == 0)        co->MoveHamToIncoming = Txt2Bool(value);
          else if(stricmp(buf, "FilterHam") == 0)                co->FilterHam = Txt2Bool(value);

/* Read */
          else if(stricmp(buf, "ShowHeader") == 0)               co->ShowHeader = atoi(value);
          else if(stricmp(buf, "ShortHeaders") == 0)             strlcpy(co->ShortHeaders, value, sizeof(co->ShortHeaders));
          else if(stricmp(buf, "ShowSenderInfo") == 0)           co->ShowSenderInfo = atoi(value);
          else if(stricmp(buf, "WrapHeader") == 0)               co->WrapHeader = Txt2Bool(value);
          else if(stricmp(buf, "SigSepLine") == 0)               co->SigSepLine = atoi(value);
          else if(stricmp(buf, "ColorSignature") == 0)           strlcpy(co->ColorSignature.buf, value, sizeof(co->ColorSignature.buf));
          else if(stricmp(buf, "ColoredText") == 0)              strlcpy(co->ColoredText.buf, value, sizeof(co->ColoredText.buf));
          else if(stricmp(buf, "Color1stLevel") == 0)            strlcpy(co->Color1stLevel.buf, value, sizeof(co->Color1stLevel.buf));
          else if(stricmp(buf, "Color2ndLevel") == 0)            strlcpy(co->Color2ndLevel.buf, value, sizeof(co->Color2ndLevel.buf));
          else if(stricmp(buf, "Color3rdLevel") == 0)            strlcpy(co->Color3rdLevel.buf, value, sizeof(co->Color3rdLevel.buf));
          else if(stricmp(buf, "Color4thLevel") == 0)            strlcpy(co->Color4thLevel.buf, value, sizeof(co->Color4thLevel.buf));
          else if(stricmp(buf, "ColorURL") == 0)                 strlcpy(co->ColorURL.buf, value, sizeof(co->ColorURL.buf));
          else if(stricmp(buf, "DisplayAllTexts") == 0)          co->DisplayAllTexts = Txt2Bool(value);
          else if(stricmp(buf, "FixedFontEdit") == 0)            co->FixedFontEdit = Txt2Bool(value);
          else if(stricmp(buf, "UseTextStyles") == 0)            co->UseTextStylesRead = Txt2Bool(value);
          else if(stricmp(buf, "TextColorsRead") == 0)           co->UseTextColorsRead = Txt2Bool(value);
          else if(stricmp(buf, "DisplayAllAltPart") == 0)        co->DisplayAllAltPart = Txt2Bool(value);
          else if(stricmp(buf, "MDNEnabled") == 0)               co->MDNEnabled = Txt2Bool(value);
          else if(stricmp(buf, "MDN_NoRecipient") == 0)          co->MDN_NoRecipient = atoi(value);
          else if(stricmp(buf, "MDN_NoDomain") == 0)             co->MDN_NoDomain = atoi(value);
          else if(stricmp(buf, "MDN_OnDelete") == 0)             co->MDN_OnDelete = atoi(value);
          else if(stricmp(buf, "MDN_Other") == 0)                co->MDN_Other = atoi(value);
          else if(stricmp(buf, "MultipleWindows") == 0)          co->MultipleReadWindows = Txt2Bool(value);
          else if(stricmp(buf, "StatusChangeDelay") == 0)
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
          else if(stricmp(buf, "ConvertHTML") == 0)              co->ConvertHTML = Txt2Bool(value);
          else if(stricmp(buf, "LocalCharset") == 0)             strlcpy(co->DefaultReadCharset, value, sizeof(co->DefaultReadCharset));
          else if(stricmp(buf, "DetectCyrillic") == 0)           co->DetectCyrillic = Txt2Bool(value);
          else if(stricmp(buf, "MapForeignChars") == 0)          co->MapForeignChars = Txt2Bool(value);
          else if(stricmp(buf, "GlobalMailThreads") == 0)        co->GlobalMailThreads = Txt2Bool(value);

/* Write */
          else if(stricmp(buf, "NewIntro") == 0)                 strlcpy(co->NewIntro, value2, sizeof(co->NewIntro));
          else if(stricmp(buf, "Greetings") == 0)                strlcpy(co->Greetings, value2, sizeof(co->Greetings));
          else if(stricmp(buf, "WarnSubject") == 0)              co->WarnSubject = Txt2Bool(value);
          else if(stricmp(buf, "EdWrapCol") == 0)                co->EdWrapCol = atoi(value);
          else if(stricmp(buf, "EdWrapMode") == 0)               co->EdWrapMode = atoi(value);
          else if(stricmp(buf, "Editor") == 0)                   strlcpy(co->Editor, value, sizeof(co->Editor));
          else if(stricmp(buf, "LaunchAlways") == 0)             co->LaunchAlways = Txt2Bool(value);
          else if(stricmp(buf, "EmailCache") == 0)               co->EmailCache = atoi(value);
          else if(stricmp(buf, "AutoSave") == 0)                 co->AutoSave = atoi(value);
          else if(stricmp(buf, "WriteCharset") == 0)             strlcpy(co->DefaultWriteCharset, value, sizeof(co->DefaultWriteCharset));
          else if(stricmp(buf, "FixedFontWrite") == 0)           co->UseFixedFontWrite = Txt2Bool(value);
          else if(stricmp(buf, "TextStylesWrite") == 0)          co->UseTextStylesWrite = Txt2Bool(value);
          else if(stricmp(buf, "TextColorsWrite") == 0)          co->UseTextColorsWrite = Txt2Bool(value);

/* Reply/Forward */
          else if(stricmp(buf, "ReplyHello") == 0)               strlcpy(co->ReplyHello, value2, sizeof(co->ReplyHello));
          else if(stricmp(buf, "ReplyIntro") == 0)               strlcpy(co->ReplyIntro, value2, sizeof(co->ReplyIntro));
          else if(stricmp(buf, "ReplyBye") == 0)                 strlcpy(co->ReplyBye, value2, sizeof(co->ReplyBye));
          else if(stricmp(buf, "AltReplyHello") == 0)            strlcpy(co->AltReplyHello, value2, sizeof(co->AltReplyHello));
          else if(stricmp(buf, "AltReplyIntro") == 0)            strlcpy(co->AltReplyIntro, value2, sizeof(co->AltReplyIntro));
          else if(stricmp(buf, "AltReplyBye") == 0)              strlcpy(co->AltReplyBye, value2, sizeof(co->AltReplyBye));
          else if(stricmp(buf, "AltReplyPattern") == 0)          strlcpy(co->AltReplyPattern, value2, sizeof(co->AltReplyPattern));
          else if(stricmp(buf, "MLReplyHello") == 0)             strlcpy(co->MLReplyHello, value2, sizeof(co->MLReplyHello));
          else if(stricmp(buf, "MLReplyIntro") == 0)             strlcpy(co->MLReplyIntro, value2, sizeof(co->MLReplyIntro));
          else if(stricmp(buf, "MLReplyBye") == 0)               strlcpy(co->MLReplyBye, value2, sizeof(co->MLReplyBye));
          else if(stricmp(buf, "ForwardMode") == 0)              co->ForwardMode = atoi(value);
          else if(stricmp(buf, "ForwardIntro") == 0)             strlcpy(co->ForwardIntro, value2, sizeof(co->ForwardIntro));
          else if(stricmp(buf, "ForwardFinish") == 0)            strlcpy(co->ForwardFinish, value2, sizeof(co->ForwardFinish));
          else if(stricmp(buf, "QuoteChar") == 0)                strlcpy(co->QuoteChar, value2, sizeof(co->QuoteChar));
          else if(stricmp(buf, "AltQuoteChar") == 0)             strlcpy(co->AltQuoteChar, value2, sizeof(co->AltQuoteChar));
          else if(stricmp(buf, "QuoteEmptyLines") == 0)          co->QuoteEmptyLines = Txt2Bool(value);
          else if(stricmp(buf, "CompareAddress") == 0)           co->CompareAddress = Txt2Bool(value);
          else if(stricmp(buf, "StripSignature") == 0)           co->StripSignature = Txt2Bool(value);

/* Signature */
          else if(stricmp(buf, "TagsFile") == 0)                 strlcpy(co->TagsFile, value, sizeof(co->TagsFile));
          else if(stricmp(buf, "TagsSeparator") == 0)            strlcpy(co->TagsSeparator, value2, sizeof(co->TagsSeparator));

/* Lists */
          else if(stricmp(buf, "FolderCols") == 0)               co->FolderCols = atoi(value);
          else if(stricmp(buf, "MessageCols") == 0)              co->MessageCols = atoi(value);
          else if(stricmp(buf, "FixedFontList") == 0)            co->FixedFontList = Txt2Bool(value);
          else if(stricmp(buf, "DateTimeFormat") == 0)           co->DSListFormat = atoi(value);
          else if(stricmp(buf, "ABookLookup") == 0)              co->ABookLookup = Txt2Bool(value);
          else if(stricmp(buf, "FolderCntMenu") == 0)            co->FolderCntMenu = Txt2Bool(value);
          else if(stricmp(buf, "MessageCntMenu") == 0)           co->MessageCntMenu = Txt2Bool(value);
          else if(stricmp(buf, "FolderInfoMode") == 0)           co->FolderInfoMode = atoi(value);
          else if(stricmp(buf, "FolderDoubleClick") == 0)        co->FolderDoubleClick = Txt2Bool(value);

/* Security */
          else if(stricmp(buf, "PGPCmdPath") == 0)               strlcpy(co->PGPCmdPath, value, sizeof(co->PGPCmdPath));
          else if(stricmp(buf, "PGPPassInterval") == 0)          co->PGPPassInterval = atoi(value);
          else if(stricmp(buf, "LogfilePath") == 0)              strlcpy(co->LogfilePath, value, sizeof(co->LogfilePath));
          else if(stricmp(buf, "LogfileMode") == 0)              co->LogfileMode = atoi(value);
          else if(stricmp(buf, "SplitLogfile") == 0)             co->SplitLogfile = Txt2Bool(value);
          else if(stricmp(buf, "LogAllEvents") == 0)             co->LogAllEvents = Txt2Bool(value);

/* Startup/Quit */
          else if(stricmp(buf, "GetOnStartup") == 0)             { globalDownloadOnStartup = Txt2Bool(value); foundGlobalPOP3Options = TRUE; }
          else if(stricmp(buf, "SendOnStartup") == 0)            co->SendOnStartup = Txt2Bool(value);
          else if(stricmp(buf, "CleanupOnStartup") == 0)         co->CleanupOnStartup = Txt2Bool(value);
          else if(stricmp(buf, "RemoveOnStartup") == 0)          co->RemoveOnStartup = Txt2Bool(value);
          else if(stricmp(buf, "LoadAllFolders") == 0)           co->LoadAllFolders = Txt2Bool(value);
          else if(stricmp(buf, "UpdateNewMail") == 0)            co->UpdateNewMail = Txt2Bool(value);
          else if(stricmp(buf, "CheckBirthdates") == 0)          co->CheckBirthdates = Txt2Bool(value);
          else if(stricmp(buf, "SendOnQuit") == 0)               co->SendOnQuit = Txt2Bool(value);
          else if(stricmp(buf, "CleanupOnQuit") == 0)            co->CleanupOnQuit = Txt2Bool(value);
          else if(stricmp(buf, "RemoveOnQuit") == 0)             co->RemoveOnQuit = Txt2Bool(value);

/* MIME */
          else if(strnicmp(buf, "MV", 2) == 0 && isdigit(buf[2]) && isdigit(buf[3]) && strchr(buf, '.'))
          {
            int curTypeID = atoi(&buf[2]);
            char *q = strchr(buf, '.')+1;

            // we only get the correct mimetype node if the ID
            // is greater than zero, because zero is reserved for the default
            // mime viewer type.
            if(curTypeID > 0)
            {
              if(lastType != NULL && lastTypeID != curTypeID)
              {
                int i;
                struct Node *curNode;

                // reset the lastType
                lastType = NULL;
                lastTypeID = -1;

                // try to get the mimeType with that particular filter ID out of our
                // filterList
                i = 0;
                IterateList(&co->mimeTypeList, curNode)
                {
                  if(i == curTypeID)
                  {
                    lastType = (struct MimeTypeNode *)curNode;
                    lastTypeID = i;
                    break;
                  }

                  i++;
                }
              }

              if(lastType == NULL)
              {
                if((lastType = CreateNewMimeType()) != NULL)
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
              if(!stricmp(q, "ContentType"))
                strlcpy(lastType->ContentType, value, sizeof(lastType->ContentType));
              else if(!stricmp(q, "Extension"))
                strlcpy(lastType->Extension, value, sizeof(lastType->Extension));
              else if(!stricmp(q, "Command"))
                strlcpy(lastType->Command, value, sizeof(lastType->Command));
              else if(!stricmp(q, "Description"))
                strlcpy(lastType->Description, value, sizeof(lastType->Description));
            }
            else
            {
              if(!stricmp(q, "Command"))
                strlcpy(C->DefaultMimeViewer, value, sizeof(C->DefaultMimeViewer));
            }
          }

/* Address book*/
          else if(stricmp(buf, "GalleryDir") == 0)               strlcpy(co->GalleryDir, value, sizeof(co->GalleryDir));
          else if(stricmp(buf, "ProxyServer") == 0)              strlcpy(co->ProxyServer, value, sizeof(co->ProxyServer));
          else if(stricmp(buf, "NewAddrGroup") == 0)             strlcpy(co->NewAddrGroup, value, sizeof(co->NewAddrGroup));
          else if(stricmp(buf, "AddToAddrbook") == 0)            co->AddToAddrbook = atoi(value);
          else if(stricmp(buf, "AddrbookCols") == 0)             co->AddrbookCols = atoi(value);

/* Scripts */
          else if(strnicmp(buf, "Rexx", 4) == 0 && buf[6] == '.')
          {
            int j = atoi(&buf[4]);

            if(j >= 0 && j < MAXRX)
            {
              p = &buf[7];
              if(!stricmp(p, "Name"))                            strlcpy(co->RX[j].Name, value, sizeof(co->RX[j].Name));
              else if(!stricmp(p, "Script"))                     strlcpy(co->RX[j].Script, value, sizeof(co->RX[j].Script));
              else if(!stricmp(p, "IsAmigaDOS"))                 co->RX[j].IsAmigaDOS = Txt2Bool(value);
              else if(!stricmp(p, "UseConsole"))                 co->RX[j].UseConsole = Txt2Bool(value);
              else if(!stricmp(p, "WaitTerm"))                   co->RX[j].WaitTerm = Txt2Bool(value);
            }
          }

/* Miscellaneous */
          else if(stricmp(buf, "TempDir") == 0)                  strlcpy(co->TempDir, value, sizeof(co->TempDir));
          else if(stricmp(buf, "DetachDir") == 0)                strlcpy(co->DetachDir, value, sizeof(co->DetachDir));
          else if(stricmp(buf, "AttachDir") == 0)                strlcpy(co->AttachDir, value, sizeof(co->AttachDir));
          else if(stricmp(buf, "WBAppIcon") == 0)                co->WBAppIcon = Txt2Bool(value);
          else if(stricmp(buf, "IconPosition") == 0)             sscanf(value, "%d;%d", &(co->IconPositionX), &(co->IconPositionY));
          else if(stricmp(buf, "AppIconText") == 0)              strlcpy(co->AppIconText, value, sizeof(co->AppIconText));
          else if(stricmp(buf, "DockyIcon") == 0)                co->DockyIcon = Txt2Bool(value);
          else if(stricmp(buf, "IconifyOnQuit") == 0)            co->IconifyOnQuit = Txt2Bool(value);
          else if(stricmp(buf, "Confirm") == 0)                  co->Confirm = Txt2Bool(value);
          else if(stricmp(buf, "ConfirmDelete") == 0)            co->ConfirmDelete = atoi(value);
          else if(stricmp(buf, "RemoveAtOnce") == 0)             co->RemoveAtOnce = Txt2Bool(value);
          else if(stricmp(buf, "XPKPack") == 0)
          {
            strlcpy(co->XPKPack, value, sizeof(co->XPKPack));
            co->XPKPackEff = atoi(&value[5]);
          }
          else if(stricmp(buf, "XPKPackEncrypt") == 0)
          {
            strlcpy(co->XPKPackEncrypt, value, sizeof(co->XPKPackEncrypt));
            co->XPKPackEncryptEff = atoi(&value[5]);
          }
          else if(stricmp(buf, "PackerCommand") == 0)            strlcpy(co->PackerCommand, value, sizeof(co->PackerCommand));

/* Look&Feel */
          else if(stricmp(buf, "Theme") == 0)                    strlcpy(co->ThemeName, value, sizeof(co->ThemeName));
          else if(stricmp(buf, "InfoBar") == 0)                  co->InfoBar = atoi(value);
          else if(stricmp(buf, "InfoBarText") == 0)              strlcpy(co->InfoBarText, value, sizeof(co->InfoBarText));
          else if(stricmp(buf, "QuickSearchBar") == 0)           co->QuickSearchBar = Txt2Bool(value);
          else if(stricmp(buf, "EmbeddedReadPane") == 0)         co->EmbeddedReadPane = Txt2Bool(value);
          else if(stricmp(buf, "SizeFormat") == 0)               co->SizeFormat = atoi(value);

/*Update*/
          else if(stricmp(buf, "UpdateInterval") == 0)           co->UpdateInterval = atoi(value);
          else if(stricmp(buf, "UpdateServer") == 0)             strlcpy(co->UpdateServer, value, sizeof(co->UpdateServer));
          else if(stricmp(buf, "UpdateDownloadPath") == 0)       strlcpy(co->UpdateDownloadPath, value, sizeof(co->UpdateDownloadPath));

/*Advanced*/
          else if(stricmp(buf, "LetterPart") == 0)
          {
            co->LetterPart = atoi(value);
            if(co->LetterPart <= 0)
              co->LetterPart = 1;
          }
          else if(stricmp(buf, "WriteIndexes") == 0)             co->WriteIndexes = atoi(value);
          else if(stricmp(buf, "ExpungeIndexes") == 0)           co->ExpungeIndexes = atoi(value);
          else if(stricmp(buf, "SupportSite") == 0)              strlcpy(co->SupportSite, value, sizeof(co->SupportSite));
          else if(stricmp(buf, "JumpToNewMsg") == 0)             co->JumpToNewMsg = Txt2Bool(value);
          else if(stricmp(buf, "JumpToIncoming") == 0)           co->JumpToIncoming = Txt2Bool(value);
          else if(stricmp(buf, "JumpToRecentMsg") == 0)          co->JumpToRecentMsg = Txt2Bool(value);
          else if(stricmp(buf, "AskJumpUnread") == 0)            co->AskJumpUnread = Txt2Bool(value);
          else if(stricmp(buf, "PrinterCheck") == 0)             co->PrinterCheck = Txt2Bool(value);
          else if(stricmp(buf, "IsOnlineCheck") == 0)            co->IsOnlineCheck = Txt2Bool(value);
          else if(stricmp(buf, "IOCInterface") == 0)             strlcpy(co->IOCInterfaces, value, sizeof(co->IOCInterfaces));
          else if(stricmp(buf, "ConfirmOnQuit") == 0)            co->ConfirmOnQuit = Txt2Bool(value);
          else if(stricmp(buf, "HideGUIElements") == 0)          co->HideGUIElements = atoi(value);
          else if(stricmp(buf, "SysCharsetCheck") == 0)          co->SysCharsetCheck = Txt2Bool(value);
          else if(stricmp(buf, "AmiSSLCheck") == 0)              co->AmiSSLCheck = Txt2Bool(value);
          else if(stricmp(buf, "TimeZoneCheck") == 0)            co->TimeZoneCheck = Txt2Bool(value);
          else if(stricmp(buf, "AutoDSTCheck") == 0)             co->AutoDSTCheck = Txt2Bool(value);
          else if(stricmp(buf, "StackSize") == 0)                co->StackSize = atoi(value);
          else if(stricmp(buf, "PrintMethod") == 0)              co->PrintMethod = atoi(value);
          else if(stricmp(buf, "AutoColumnResize") == 0)         co->AutoColumnResize = Txt2Bool(value);
          else if(stricmp(buf, "SocketOptions") == 0)
          {
            char *s = value;

            // Now we have to identify the socket option line
            // and we to that by tokenizing it
            while(*s != '\0')
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
                char *q = strchr(s, '=');

                if(q != NULL)
                  co->SocketOptions.SendBuffer = atoi(q+1);
              }
              else if(strnicmp(s, "SO_RCVBUF", 9) == 0)
              {
                char *q = strchr(s, '=');

                if(q != NULL)
                  co->SocketOptions.RecvBuffer = atoi(q+1);
              }
              else if(strnicmp(s, "SO_SNDLOWAT", 11) == 0)
              {
                char *q = strchr(s, '=');

                if(q != NULL)
                  co->SocketOptions.SendLowAt = atoi(q+1);
              }
              else if(strnicmp(s, "SO_RCVLOWAT", 11) == 0)
              {
                char *q = strchr(s, '=');

                if(q != NULL)
                  co->SocketOptions.RecvLowAt = atoi(q+1);
              }
              else if(strnicmp(s, "SO_SNDTIMEO", 11) == 0)
              {
                char *q = strchr(s, '=');

                if(q != NULL)
                  co->SocketOptions.SendTimeOut = atoi(q+1);
              }
              else if(strnicmp(s, "SO_RCVTIMEO", 11) == 0)
              {
                char *q = strchr(s, '=');

                if(q != NULL)
                  co->SocketOptions.RecvTimeOut = atoi(q+1);
              }

              // set the next start to our last search
              if(*e != '\0')
                s = ++e;
              else
                break;
            }
          }
          else if(stricmp(buf, "SocketTimeout") == 0)            co->SocketTimeout = atoi(value);
          else if(stricmp(buf, "TRBufferSize") == 0)             co->TRBufferSize = atoi(value);
          else if(stricmp(buf, "EmbeddedMailDelay") == 0)        co->EmbeddedMailDelay = atoi(value);
          else if(stricmp(buf, "KeepAliveInterval") == 0)        co->KeepAliveInterval = atoi(value);
          else if(stricmp(buf, "StyleFGroupUnread") == 0)        String2MUIStyle(value, co->StyleFGroupUnread);
          else if(stricmp(buf, "StyleFGroupRead") == 0)          String2MUIStyle(value, co->StyleFGroupRead);
          else if(stricmp(buf, "StyleFolderUnread") == 0)        String2MUIStyle(value, co->StyleFolderUnread);
          else if(stricmp(buf, "StyleFolderRead") == 0)          String2MUIStyle(value, co->StyleFolderRead);
          else if(stricmp(buf, "StyleFolderNew") == 0)           String2MUIStyle(value, co->StyleFolderNew);
          else if(stricmp(buf, "StyleMailUnread") == 0)          String2MUIStyle(value, co->StyleMailUnread);
          else if(stricmp(buf, "StyleMailRead") == 0)            String2MUIStyle(value, co->StyleMailRead);
          else if(stricmp(buf, "AutoClip") == 0)                 co->AutoClip = Txt2Bool(value);
          else if(stricmp(buf, "ShowFilterStats") == 0)          co->ShowFilterStats = Txt2Bool(value);
          else if(stricmp(buf, "ConfirmRemoveAttachments") == 0) co->ConfirmRemoveAttachments = Txt2Bool(value);
          else if(stricmp(buf, "BirthdayCheckTime") == 0)        String2DateStamp(&co->BirthdayCheckTime, value, DSS_TIME, TZC_NONE);

/* Obsolete options (previous YAM version write them, we just read them) */
          else if(stricmp(buf, "RealName") == 0)                 strlcpy(fUserIdentity->realname, value, sizeof(fUserIdentity->realname));
          else if(stricmp(buf, "EmailAddress") == 0)             strlcpy(fUserIdentity->address, value, sizeof(fUserIdentity->address));
          else if(stricmp(buf, "ReplyTo") == 0)                  strlcpy(fUserIdentity->mailReplyTo, value, sizeof(fUserIdentity->mailReplyTo));
          else if(stricmp(buf, "Organization") == 0)             strlcpy(fUserIdentity->organization, value, sizeof(fUserIdentity->organization));
          else if(stricmp(buf, "ExtraHeaders") == 0)             strlcpy(fUserIdentity->extraHeaders, value, sizeof(fUserIdentity->extraHeaders));
          else if(stricmp(buf, "RequestMDN") == 0)               fUserIdentity->requestMDN = Txt2Bool(value);
          else if(stricmp(buf, "QuoteMessage") == 0)             fUserIdentity->quoteMails = Txt2Bool(value);
          else if(stricmp(buf, "MyPGPID") == 0)
          {
            strlcpy(fUserIdentity->pgpKeyID, value, sizeof(fUserIdentity->pgpKeyID));

            // we also set the usePGP in case the KeyID is not empty
            if(fUserIdentity->pgpKeyID[0] != '\0')
              fUserIdentity->usePGP = TRUE;
          }
          else if(stricmp(buf, "PGPURL") == 0)                   strlcpy(fUserIdentity->pgpKeyURL, value, sizeof(fUserIdentity->pgpKeyURL));
          else if(stricmp(buf, "EncryptToSelf") == 0)            fUserIdentity->pgpSelfEncrypt = Txt2Bool(value);
          else if(stricmp(buf, "MyPictureURL") == 0)             strlcpy(fUserIdentity->photoURL, value, sizeof(fUserIdentity->photoURL));
          else if(stricmp(buf, "SaveSent") == 0)                 fUserIdentity->saveSentMail = Txt2Bool(value);
          else if(stricmp(buf, "AddMyInfo") == 0)                fUserIdentity->addPersonalInfo = Txt2Bool(value);
          else if(stricmp(buf, "UseSignature") == 0)             fUserIdentity->signature = (Txt2Bool(value) == TRUE ? 1 : 0);
 
          else
            W(DBF_CONFIG, "unknown config option: '%s' = '%s'", buf, value);
        }
      }

      // we have to check if something went
      // wrong while loading the config
      if(feof(fh) != 0 && ferror(fh) == 0)
      {
        success = TRUE;

        // mark the configuration as "saved" as we have loaded
        // it freshly
        co->ConfigIsSaved = TRUE;

        D(DBF_CONFIG, "configuration successfully loaded");

        if(foundGlobalPOP3Options == TRUE)
        {
          // Propagate the old global POP3 options to each POP3 account.
          // The old options will vanish as soon as the configuration is saved, thus this
          // will happen only once.
          struct Node *curNode;

          IterateList(&co->mailServerList, curNode)
          {
            struct MailServerNode *msn = (struct MailServerNode *)curNode;

            if(msn->type == MST_POP3)
            {
              // apply only the found old settings and exclude the non-found ones
              if(globalPOP3AvoidDuplicates != -1)
              {
                if(globalPOP3AvoidDuplicates == TRUE)
                  SET_FLAG(msn->flags, MSF_AVOID_DUPLICATES);
                else
                  CLEAR_FLAG(msn->flags, MSF_AVOID_DUPLICATES);
              }

              if(globalDownloadOnStartup != -1)
              {
                if(globalDownloadOnStartup == TRUE)
                  SET_FLAG(msn->flags, MSF_DOWNLOAD_ON_STARTUP);
                else
                  CLEAR_FLAG(msn->flags, MSF_DOWNLOAD_ON_STARTUP);
              }

              if(globalPOP3Preselection != -1)
                msn->preselection = globalPOP3Preselection;
            }
          }
        }
      }
      else
        E(DBF_CONFIG, "error during config load operation");
    }
    else
      E(DBF_CONFIG, "didn't find typical YAM config header in first line");

    fclose(fh);

    free(buf);
  }

  RETURN(success);
  return success;
}

///
/// CO_GetConfig
//  Fills form data of current section with data from configuration structure
void CO_GetConfig(BOOL saveConfig)
{
  struct CO_GUIData *gui = &G->CO->GUI;

  ENTER();

  switch (G->CO->VisiblePage)
  {
    case cp_AllPages:
    {
      // nothing
    }
    break;

    case cp_FirstSteps:
    {
      struct UserIdentityNode *uin;

      if((uin = GetUserIdentity(&CE->userIdentityList, 0, TRUE)) != NULL)
      {
        GetMUIString(uin->realname, gui->ST_REALNAME, sizeof(uin->realname));
        GetMUIString(uin->address, gui->ST_EMAIL, sizeof(uin->address));
      }

      CE->TimeZone = MapTZ(GetMUICycle(gui->CY_TZONE), TRUE);
      CE->DaylightSaving = GetMUICheck(gui->CH_DSTACTIVE);
    }
    break;

    case cp_TCPIP:
    {
      int i;
      struct MailServerNode *msn;

      // as the user may have changed the order of the POP3 servers
      // we have to make sure the order in the NList fits to the
      // exec list order of our POP3 server list
      i = 0;
      do
      {
        msn = NULL;
        DoMethod(gui->LV_POP3, MUIM_NList_GetEntry, i, &msn);
        if(msn == NULL || msn->type != MST_POP3)
          break;

        // for resorting the POP3 list we just have to remove that particular server
        // and add it to the tail - all other operations like adding/removing should
        // have been done by others already - so this is just resorting
        Remove((struct Node *)msn);
        AddTail((struct List *)&CE->mailServerList, (struct Node *)msn);

        i++;
      }
      while(TRUE);

      // as the user may have changed the order of the SMTP servers
      // we have to make sure the order in the NList fits to the
      // exec list order of our SMTP server list
      i = 0;
      do
      {
        msn = NULL;
        DoMethod(gui->LV_SMTP, MUIM_NList_GetEntry, i, &msn);
        if(msn == NULL || msn->type != MST_SMTP)
          break;

        // for resorting the SMTP list we just have to remove that particular server
        // and add it to the tail - all other operations like adding/removing should
        // have been done by others already - so this is just resorting
        Remove((struct Node *)msn);
        AddTail((struct List *)&CE->mailServerList, (struct Node *)msn);

        i++;
      }
      while(TRUE);
    }
    break;

    case cp_Identities:
    {
      int i;
      struct UserIdentityNode *uin;

      // as the user may have changed the order of the User Identities, so
      // we have to make sure the order in the NList fits to the
      // exec list order of our UserIdentity list
      i = 0;
      do
      {
        uin = NULL;
        DoMethod(gui->LV_IDENTITY, MUIM_NList_GetEntry, i, &uin);
        if(uin == NULL)
          break;

        // for resorting the UserIdentity list we just have to remove that particular identity
        // and add it to the tail - all other operations like adding/removing should
        // have been done by others already - so this is just resorting
        Remove((struct Node *)uin);
        AddTail((struct List *)&CE->userIdentityList, (struct Node *)uin);

        i++;
      }
      while(TRUE);
    }
    break;

    case cp_NewMail:
    {
      CE->TransferWindow    = GetMUICycle  (gui->CY_TRANSWIN);
      CE->UpdateStatus      = GetMUICheck  (gui->CH_UPDSTAT);
      CE->WarnSize          = GetMUIInteger(gui->ST_WARNSIZE);
      CE->CheckMailDelay    = GetMUINumer  (gui->NM_INTERVAL);
      CE->DownloadLarge     = GetMUICheck  (gui->CH_DLLARGE);
      CE->NotifyType        = (GetMUICheck(gui->CH_NOTIREQ)        ? NOTIFY_REQ        : 0)
                            + (GetMUICheck(gui->CH_NOTIOS41SYSTEM) ? NOTIFY_OS41SYSTEM : 0)
                            + (GetMUICheck(gui->CH_NOTISOUND)      ? NOTIFY_SOUND      : 0)
                            + (GetMUICheck(gui->CH_NOTICMD)        ? NOTIFY_CMD        : 0);
      GetMUIString(CE->NotifySound, gui->ST_NOTISOUND, sizeof(CE->NotifySound));
      GetMUIString(CE->NotifyCommand, gui->ST_NOTICMD, sizeof(CE->NotifyCommand));
    }
    break;

    case cp_Filters:
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

    case cp_Spam:
    {
      CE->SpamFilterEnabled = GetMUICheck(gui->CH_SPAMFILTERENABLED);
      CE->SpamFilterForNewMail = GetMUICheck(gui->CH_SPAMFILTERFORNEWMAIL);
      CE->SpamMarkOnMove = GetMUICheck(gui->CH_SPAMMARKONMOVE);
      CE->SpamMarkAsRead = GetMUICheck(gui->CH_SPAMMARKASREAD);
      CE->SpamAddressBookIsWhiteList = GetMUICheck(gui->CH_SPAMABOOKISWHITELIST);
      CE->MoveHamToIncoming = GetMUICheck(gui->CH_MOVEHAMTOINCOMING);
      CE->FilterHam = GetMUICheck(gui->CH_FILTERHAM);

      if(C->SpamFilterEnabled == TRUE && CE->SpamFilterEnabled == FALSE)
      {
        LONG mask;

        // raise a CheckboxRequest and ask the user which
        // operations he want to performed while disabling the
        // SPAM filter.
        mask = CheckboxRequest(G->CO->GUI.WI, NULL, 3, tr(MSG_CO_SPAM_DISABLEFILTERASK),
                                                       tr(MSG_CO_SPAM_RESETTDATA),
                                                       tr(MSG_CO_SPAM_RESETMAILFLAGS),
                                                       tr(MSG_CO_SPAM_DELETESPAMFOLDER));

        SHOWVALUE(DBF_CONFIG, mask);
        SHOWVALUE(DBF_ALWAYS, mask);
        // check if the user canceled the requester
        if(mask >= 0)
        {
          // reset training data
          if(mask & (1 << 0))
          {
            D(DBF_CONFIG, "resetting spam training data");
            BayesFilterResetTrainingData();
          }

          // reset spam state of all mails
          if(mask & (1 << 1))
          {
            struct FolderNode *fnode;

            D(DBF_CONFIG, "resetting spam state of all mails");

            LockFolderListShared(G->folders);

            ForEachFolderNode(G->folders, fnode)
            {
              struct Folder *folder = fnode->folder;

              if(!isGroupFolder(folder))
              {
                struct MailList *mlist;

                if((mlist = MA_CreateFullList(folder, FALSE)) != NULL)
                {
                  struct MailNode *mnode;

                  // clear all possible spam/ham flags from each mail
                  ForEachMailNode(mlist, mnode)
                  {
                    struct Mail *mail = mnode->mail;

                    if(mail != NULL)
                      MA_ChangeMailStatus(mail, SFLAG_NONE, SFLAG_USERSPAM|SFLAG_AUTOSPAM|SFLAG_HAM);
                  }

                  DeleteMailList(mlist);
                }
              }
            }

            UnlockFolderList(G->folders);
          }

          // delete spam folder
          if(mask & (1 << 2))
          {
            struct Folder *spamFolder;

            D(DBF_CONFIG, "deleting spam folder");

            // first locate the spam folder
            if((spamFolder = FO_GetFolderByType(FT_SPAM, NULL)) != NULL)
            {
              // delete the folder on disk
              DeleteMailDir(spamFolder->Fullpath, FALSE);

              // remove all mails from our internal list
              ClearFolderMails(spamFolder, TRUE);

              // remove the folder from the folder list
              DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NListtree_Remove, MUIV_NListtree_Remove_ListNode_Root, spamFolder->Treenode, MUIF_NONE);

              // and finally save the modified tree to the folder config now
              FO_SaveTree();

              // update the statistics in case the spam folder contained new or unread mails
              DisplayStatistics(NULL, TRUE);

              // remove and delete the folder from our list
              LockFolderList(G->folders);
              RemoveFolder(G->folders, spamFolder);
              UnlockFolderList(G->folders);

              FO_FreeFolder(spamFolder);
            }
          }
          else
          {
            // the spam folder should be kept, but it must be "degraded" to a normal folder
            // to make it deleteable later
            struct Folder *spamFolder;

            // first locate the spam folder
            if((spamFolder = FO_GetFolderByType(FT_SPAM, NULL)) != NULL)
            {
              if(spamFolder->imageObject != NULL)
              {
                // we make sure that the NList also doesn't use the image in future anymore
                DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NList_UseImage, NULL, spamFolder->ImageIndex, MUIF_NONE);
                spamFolder->imageObject = NULL;
                // we don't need to dispose the image, because it is one of the standard images and not
                // a custom image of the user.
              }

              // degrade it to a custom folder without folder image
              spamFolder->Type = FT_CUSTOM;
              spamFolder->ImageIndex = -1;

              // finally save the modified configuration
              FO_SaveConfig(spamFolder);

              // update the statistics in case the spam folder contained new or unread mails
              DisplayStatistics(spamFolder, TRUE);
            }
          }

          // update the toolbar to the new settings
          if(G->MA->GUI.TO_TOOLBAR != NULL)
            DoMethod(G->MA->GUI.TO_TOOLBAR, MUIM_MainWindowToolbar_UpdateSpamControls);
        }
        else
        {
          // the user canceled the requester so lets set the Spam filter
          // back online
          CE->SpamFilterEnabled = TRUE;
        }

      }
      else if(C->SpamFilterEnabled == FALSE && CE->SpamFilterEnabled == TRUE)
      {
        // the spam filter has been enabled, now try to create the mandatory spam folder
        char spamPath[SIZE_PATHFILE];
        BOOL createSpamFolder;
        enum FType type;

        if(ObtainFileInfo(CreateFilename(FolderName[FT_SPAM], spamPath, sizeof(spamPath)), FI_TYPE, &type) == TRUE && type == FIT_NONEXIST)
        {
          // no directory named "spam" exists, so let's create it
          createSpamFolder = TRUE;
        }
        else
        {
          ULONG result;

          // the directory "spam" already exists, but it is not the standard spam folder
          // let the user decide what to do
          result = MUI_Request(G->App, NULL, 0, NULL,
                                                tr(MSG_ER_SPAMDIR_EXISTS_ANSWERS),
                                                tr(MSG_ER_SPAMDIR_EXISTS));
          switch(result)
          {
            default:
            case 0:
            {
              // the user has chosen to disable the spam filter, so we do it
              // or the requester was cancelled
              CE->SpamFilterEnabled = FALSE;
              createSpamFolder = FALSE;
            }
            break;

            case 1:
            {
              // delete everything in the folder, the directory itself can be kept
              DeleteMailDir(CreateFilename(FolderName[FT_SPAM], spamPath, sizeof(spamPath)), FALSE);
              createSpamFolder = TRUE;
            }
            break;

            case 2:
            {
              // keep the folder contents
              createSpamFolder = TRUE;
            }
            break;
          }
        }

        if(createSpamFolder == TRUE)
        {
          struct Folder *spamFolder;

          // if a folder named "spam" already exists, but a new spam folder should be
          // created, we need to remove the old folder from the tree view first
          if((spamFolder = FO_GetFolderByPath((STRPTR)FolderName[FT_SPAM], NULL)) != NULL)
          {
            // remove the folder from the folder list
            DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NListtree_Remove, MUIV_NListtree_Remove_ListNode_Root, spamFolder->Treenode, MUIF_NONE);
            spamFolder->Treenode = NULL;

            if(spamFolder->imageObject != NULL)
            {
              // we make sure that the NList also doesn't use the image in future anymore
              DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NList_UseImage, NULL, spamFolder->ImageIndex, MUIF_NONE);
              spamFolder->imageObject = NULL;
            }
          }

          // try to create the folder and save the new folder tree
          if(FO_CreateFolder(FT_SPAM, FolderName[FT_SPAM], tr(MSG_MA_SPAM)) == FALSE || FO_SaveTree() == FALSE)
          {
            // something failed, so we disable the spam filter again
            ER_NewError(tr(MSG_CO_ER_CANNOT_CREATE_SPAMFOLDER));
            CE->SpamFilterEnabled = FALSE;
          }
          else
          {
            // update the toolbar to the new settings
            if(G->MA->GUI.TO_TOOLBAR != NULL)
              DoMethod(G->MA->GUI.TO_TOOLBAR, MUIM_MainWindowToolbar_UpdateSpamControls);
          }
        }
      }

      if(CE->SpamFilterEnabled == TRUE && CE->SpamMarkAsRead == TRUE)
      {
        ULONG numberClassified = BayesFilterNumberOfHamClassifiedMails() + BayesFilterNumberOfSpamClassifiedMails();

        if(numberClassified < 100)
        {
          // Less than 100 mails have been classified so far.
          // Better ask the user if new spam mails really should be mark as "read", because
          // the filter is most probably not trained well enough an non-spam mails may be
          // marked as spam and read unnoticed.
          if(MUI_Request(G->App, NULL, 0, NULL,
                                          tr(MSG_YesNoReq),
                                          tr(MSG_ER_SPAM_NOT_ENOUGH_CLASSIFIED_MAILS), numberClassified))
          {
            CE->SpamMarkAsRead = FALSE;
          }
        }
      }
    }
    break;

    case cp_Read:
    {
      CE->ShowHeader        = GetMUICycle(gui->CY_HEADER);
      GetMUIString(CE->ShortHeaders, gui->ST_HEADERS, sizeof(CE->ShortHeaders));
      CE->ShowSenderInfo    = GetMUICycle(gui->CY_SENDERINFO);
      CE->SigSepLine        = GetMUICycle(gui->CY_SIGSEPLINE);
      memcpy(&CE->ColorSignature, (struct MUI_PenSpec*)xget(gui->CA_COLSIG,   MUIA_Pendisplay_Spec), sizeof(struct MUI_PenSpec));
      memcpy(&CE->ColoredText,    (struct MUI_PenSpec*)xget(gui->CA_COLTEXT,  MUIA_Pendisplay_Spec), sizeof(struct MUI_PenSpec));
      memcpy(&CE->Color1stLevel,  (struct MUI_PenSpec*)xget(gui->CA_COL1QUOT, MUIA_Pendisplay_Spec), sizeof(struct MUI_PenSpec));
      memcpy(&CE->Color2ndLevel,  (struct MUI_PenSpec*)xget(gui->CA_COL2QUOT, MUIA_Pendisplay_Spec), sizeof(struct MUI_PenSpec));
      memcpy(&CE->Color3rdLevel,  (struct MUI_PenSpec*)xget(gui->CA_COL3QUOT, MUIA_Pendisplay_Spec), sizeof(struct MUI_PenSpec));
      memcpy(&CE->Color4thLevel,  (struct MUI_PenSpec*)xget(gui->CA_COL4QUOT, MUIA_Pendisplay_Spec), sizeof(struct MUI_PenSpec));
      memcpy(&CE->ColorURL,       (struct MUI_PenSpec*)xget(gui->CA_COLURL,   MUIA_Pendisplay_Spec), sizeof(struct MUI_PenSpec));
      CE->DisplayAllTexts   = GetMUICheck(gui->CH_ALLTEXTS);
      CE->FixedFontEdit     = GetMUICheck(gui->CH_FIXFEDIT);
      CE->WrapHeader        = GetMUICheck(gui->CH_WRAPHEAD);
      CE->UseTextStylesRead = GetMUICheck(gui->CH_TEXTSTYLES_READ);
      CE->UseTextColorsRead = GetMUICheck(gui->CH_TEXTCOLORS_READ);
      CE->DisplayAllAltPart = GetMUICheck(gui->CH_SHOWALTPARTS);

      // get MDN options from GUI
      CE->MDNEnabled      = GetMUICheck(gui->CH_MDN_ALLOW) && !GetMUICheck(gui->CH_MDN_NEVER);
      CE->MDN_NoRecipient = GetMUICycle(gui->CY_MDN_NORECIPIENT);
      CE->MDN_NoDomain    = GetMUICycle(gui->CY_MDN_NODOMAIN);
      CE->MDN_OnDelete    = GetMUICycle(gui->CY_MDN_DELETE);
      CE->MDN_Other       = GetMUICycle(gui->CY_MDN_OTHER);

      CE->MultipleReadWindows = GetMUICheck(gui->CH_MULTIWIN);
      CE->StatusChangeDelayOn = GetMUICheck(gui->CH_DELAYEDSTATUS);
      CE->StatusChangeDelay   = GetMUINumer(gui->NB_DELAYEDSTATUS)*1000;
      CE->ConvertHTML         = GetMUICheck(gui->CH_CONVERTHTML);

      GetMUIText(CE->DefaultReadCharset, gui->TX_DEFCHARSET_READ, sizeof(CE->DefaultReadCharset));
      CE->DetectCyrillic    = GetMUICheck(gui->CH_DETECTCYRILLIC);
      CE->MapForeignChars   = GetMUICheck(gui->CH_MAPFOREIGNCHARS);
      CE->GlobalMailThreads = GetMUICheck(gui->CH_GLOBALMAILTHREADS);
    }
    break;

    case cp_Write:
    {
      GetMUIString(CE->NewIntro, gui->ST_HELLOTEXT, sizeof(CE->NewIntro));
      GetMUIString(CE->Greetings, gui->ST_BYETEXT, sizeof(CE->Greetings));
      CE->WarnSubject       = GetMUICheck  (gui->CH_WARNSUBJECT);
      CE->EdWrapCol         = GetMUIInteger(gui->ST_EDWRAP);
      CE->EdWrapMode        = GetMUICycle  (gui->CY_EDWRAP);
      GetMUIString(CE->Editor, gui->ST_EDITOR, sizeof(CE->Editor));
      CE->LaunchAlways      = GetMUICheck  (gui->CH_LAUNCH);
      CE->EmailCache        = GetMUINumer  (gui->NB_EMAILCACHE);
      CE->AutoSave          = GetMUINumer  (gui->NB_AUTOSAVE)*60; // in seconds
      GetMUIText(CE->DefaultWriteCharset, gui->TX_DEFCHARSET_WRITE, sizeof(CE->DefaultWriteCharset));
      CE->UseFixedFontWrite = GetMUICheck(gui->CH_FIXEDFONT_WRITE);
      CE->UseTextStylesWrite = GetMUICheck(gui->CH_TEXTSTYLES_WRITE);
      CE->UseTextColorsWrite = GetMUICheck(gui->CH_TEXTCOLORS_WRITE);
    }
    break;

    case cp_ReplyForward:
    {
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
      CE->QuoteEmptyLines   = GetMUICheck  (gui->CH_QUOTEEMPTY);
      CE->CompareAddress    = GetMUICheck  (gui->CH_COMPADDR);
      CE->StripSignature    = GetMUICheck  (gui->CH_STRIPSIG);
      CE->ForwardMode = GetMUICycle(gui->CY_FORWARDMODE);
    }
    break;

    case cp_Signature:
    {
      GetMUIString(CE->TagsFile, gui->ST_TAGFILE, sizeof(CE->TagsFile));
      GetMUIString(CE->TagsSeparator, gui->ST_TAGSEP, sizeof(CE->TagsSeparator));

      if(xget(gui->TE_SIGEDIT, MUIA_TextEditor_HasChanged) == TRUE && saveConfig == FALSE)
      {
        // if the signature was modified but the config should not be saved but just be "used"
        // then ask the user if the changes to the signature should be made permanent
        if(MUI_Request(G->App, G->CO->GUI.WI, 0, NULL, tr(MSG_YesNoReq), tr(MSG_CO_ASK_SAVE_SIGNATURE)) > 0)
        {
          char sigPath[SIZE_PATHFILE];

          // save the modified signature only if the user told us to do so
          DoMethod(gui->TE_SIGEDIT, MUIM_MailTextEdit_SaveToFile, CreateFilename(SigNames[G->CO->LastSig], sigPath, sizeof(sigPath)));
        }
      }
    }
    break;

    case cp_Lists:
    {
      int i;

      CE->FolderCols = 1;
      for(i=1; i < FOCOLNUM; i++)
      {
        if(GetMUICheck(gui->CH_FCOLS[i]))
          SET_FLAG(CE->FolderCols, (1<<i));
      }

      CE->MessageCols = 1;
      for(i=1; i < MACOLNUM; i++)
      {
        if(GetMUICheck(gui->CH_MCOLS[i]))
          SET_FLAG(CE->MessageCols, (1<<i));
      }

      CE->FixedFontList = GetMUICheck(gui->CH_FIXFLIST);
      CE->ABookLookup = GetMUICheck(gui->CH_ABOOKLOOKUP);
      CE->FolderCntMenu = GetMUICheck(gui->CH_FCNTMENU);
      CE->MessageCntMenu = GetMUICheck(gui->CH_MCNTMENU);
      CE->FolderDoubleClick = GetMUICheck(gui->CH_FOLDERDBLCLICK);

      if(GetMUICheck(gui->CH_BEAT) == TRUE)
      {
        if(GetMUICheck(gui->CH_RELDATETIME) == TRUE)
          CE->DSListFormat = DSS_RELDATEBEAT;
        else
          CE->DSListFormat = DSS_DATEBEAT;
      }
      else
      {
        if(GetMUICheck(gui->CH_RELDATETIME) == TRUE)
          CE->DSListFormat = DSS_RELDATETIME;
        else
          CE->DSListFormat = DSS_DATETIME;
      }

      CE->FolderInfoMode = GetMUICycle(gui->CY_FOLDERINFO);
    }
    break;

    case cp_Security:
    {
      GetMUIString(CE->PGPCmdPath, gui->ST_PGPCMD, sizeof(CE->PGPCmdPath));
      GetMUIString(CE->LogfilePath, gui->ST_LOGFILE, sizeof(CE->LogfilePath));
      CE->LogfileMode = GetMUICycle(gui->CY_LOGMODE);
      CE->SplitLogfile = GetMUICheck(gui->CH_SPLITLOG);
      CE->LogAllEvents = GetMUICheck(gui->CH_LOGALL);

      if(GetMUICheck(gui->CH_PGPPASSINTERVAL) == TRUE)
        CE->PGPPassInterval = GetMUINumer(gui->NB_PGPPASSINTERVAL);
      else
        CE->PGPPassInterval = -GetMUINumer(gui->NB_PGPPASSINTERVAL);
    }
    break;

    case cp_StartupQuit:
    {
      CE->SendOnStartup     = GetMUICheck(gui->CH_SENDSTART);
      CE->CleanupOnStartup  = GetMUICheck(gui->CH_DELETESTART);
      CE->RemoveOnStartup   = GetMUICheck(gui->CH_REMOVESTART);
      CE->LoadAllFolders    = GetMUICheck(gui->CH_LOADALL);
      CE->UpdateNewMail     = GetMUICheck(gui->CH_MARKNEW);
      CE->CheckBirthdates   = GetMUICheck(gui->CH_CHECKBD);
      CE->SendOnQuit        = GetMUICheck(gui->CH_SENDQUIT);
      CE->CleanupOnQuit     = GetMUICheck(gui->CH_DELETEQUIT);
      CE->RemoveOnQuit      = GetMUICheck(gui->CH_REMOVEQUIT);
    }
    break;

    case cp_MIME:
    {
      int i=0;

      // as the user may have changed the order of the list of MIME types
      // we have to make sure the order in the NList fits to the
      // exec list order of our MIME type list
      do
      {
        struct MimeTypeNode *mt = NULL;

        DoMethod(gui->LV_MIME, MUIM_NList_GetEntry, i, &mt);
        if(mt != NULL)
        {
          // for resorting the MIME type list we just have to remove that particular type
          // and add it to the tail - all other operations like adding/removing should
          // have been done by others already - so this is just resorting
          Remove((struct Node *)mt);
          AddTail((struct List *)&CE->mimeTypeList, (struct Node *)mt);
        }
        else
          break;
      }
      while(++i);
      GetMUIString(CE->DefaultMimeViewer, gui->ST_DEFVIEWER, sizeof(CE->DefaultMimeViewer));
    }
    break;

    case cp_AddressBook:
    {
      int i;

      GetMUIString(CE->GalleryDir, gui->ST_GALLDIR, sizeof(CE->GalleryDir));
      GetMUIString(CE->NewAddrGroup, gui->ST_NEWGROUP, sizeof(CE->NewAddrGroup));
      GetMUIString(CE->ProxyServer, gui->ST_PROXY, sizeof(CE->ProxyServer));
      CE->AddToAddrbook     = GetMUICycle  (gui->CY_ATAB);
      CE->AddrbookCols = 1;
      for(i = 1; i < ABCOLNUM; i++)
      {
        if(GetMUICheck(gui->CH_ACOLS[i]))
          CE->AddrbookCols += (1<<i);
      }
    }
    break;

    case cp_Scripts:
    {
      // nothing
    }
    break;

    case cp_Mixed:
    {
      GetMUIString(CE->TempDir, gui->ST_TEMPDIR, sizeof(CE->TempDir));
      GetMUIString(CE->DetachDir, gui->ST_DETACHDIR, sizeof(CE->DetachDir));
      GetMUIString(CE->AttachDir, gui->ST_ATTACHDIR, sizeof(CE->AttachDir));
      CE->WBAppIcon         = GetMUICheck  (gui->CH_WBAPPICON);
      CE->IconPositionX     = GetMUIInteger(gui->ST_APPX);
      CE->IconPositionY     = GetMUIInteger(gui->ST_APPY);

      if(GetMUICheck(gui->CH_APPICONPOS) == FALSE)
      {
        CE->IconPositionX = -CE->IconPositionX;
        CE->IconPositionY = -CE->IconPositionY;
      }

      GetMUIString(CE->AppIconText, gui->ST_APPICON, sizeof(CE->AppIconText));
      CE->DockyIcon         = GetMUICheck  (gui->CH_DOCKYICON);
      CE->IconifyOnQuit     = GetMUICheck  (gui->CH_CLGADGET);
      CE->Confirm           = GetMUICheck  (gui->CH_CONFIRM);
      CE->ConfirmDelete     = GetMUINumer  (gui->NB_CONFIRMDEL);
      CE->RemoveAtOnce      = GetMUICheck  (gui->CH_REMOVE);
      GetMUIText(CE->XPKPack, gui->TX_PACKER, sizeof(CE->XPKPack));
      GetMUIText(CE->XPKPackEncrypt, gui->TX_ENCPACK, sizeof(CE->XPKPackEncrypt));
      CE->XPKPackEff        = GetMUINumer  (gui->NB_PACKER);
      CE->XPKPackEncryptEff = GetMUINumer  (gui->NB_ENCPACK);
      GetMUIString(CE->PackerCommand, gui->ST_ARCHIVER, sizeof(CE->PackerCommand));
    }
    break;

    case cp_LookFeel:
    {
      CE->InfoBar = GetMUICycle(gui->CY_INFOBAR);
      GetMUIString(CE->InfoBarText, gui->ST_INFOBARTXT, sizeof(CE->InfoBarText));
      CE->QuickSearchBar = GetMUICheck(gui->CH_QUICKSEARCHBAR);
      CE->EmbeddedReadPane = GetMUICheck  (gui->CH_EMBEDDEDREADPANE);
      CE->SizeFormat = GetMUICycle(gui->CY_SIZE);
    }
    break;

    case cp_Update:
    {
      int interval = GetMUICycle(gui->CY_UPDATEINTERVAL);

      switch(interval)
      {
        default:
        case 0:
          CE->UpdateInterval = 0; // never
        break;

        case 1:
          CE->UpdateInterval = 86400; // 1 day
        break;

        case 2:
          CE->UpdateInterval = 604800; // 1 week
        break;

        case 3:
          CE->UpdateInterval = 2419200; // 1 month
        break;
      }

      GetMUIString(CE->UpdateDownloadPath, gui->ST_UPDATEDOWNLOADPATH, sizeof(CE->UpdateDownloadPath));
    }
    break;

    case cp_Max:
    {
      // nothing
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
    case cp_AllPages:
      // nothing
    break;

    case cp_FirstSteps:
    {
      struct MailServerNode *msn;
      struct UserIdentityNode *uin;

      // try to get the first user identity structure
      if((uin = GetUserIdentity(&CE->userIdentityList, 0, TRUE)) != NULL)
      {
        setstring(gui->ST_REALNAME, uin->realname);
        setstring(gui->ST_EMAIL, uin->address);
      }

      setcycle(gui->CY_TZONE, MapTZ(CE->TimeZone, FALSE));
      setcheckmark(gui->CH_DSTACTIVE, CE->DaylightSaving);

      // try to get the mailer server structure of the first POP3 server
      if((msn = GetMailServer(&CE->mailServerList, MST_POP3, 0)) != NULL)
      {
        nnset(gui->ST_POPHOST0, MUIA_String_Contents, msn->hostname);
        nnset(gui->ST_USER0,    MUIA_String_Contents, msn->username);
        nnset(gui->ST_PASSWD0,  MUIA_String_Contents, msn->password);
      }
    }
    break;

    case cp_TCPIP:
    {
      int numPOP = 0;
      int numSMTP = 0;
      struct Node *curNode;
      struct MailServerNode *msn;

      // clear the lists first
      set(gui->LV_POP3, MUIA_NList_Quiet, TRUE);
      DoMethod(gui->LV_POP3, MUIM_NList_Clear);
      set(gui->LV_SMTP, MUIA_NList_Quiet, TRUE);
      DoMethod(gui->LV_SMTP, MUIM_NList_Clear);

      // we iterate through our mail server list and make sure to populate
      // out NList object correctly.
      IterateList(&CE->mailServerList, curNode)
      {
        msn = (struct MailServerNode *)curNode;

        if(msn->description[0] == '\0')
          snprintf(msn->description, sizeof(msn->description), "%s@%s", msn->username, msn->hostname);

        if(msn->type == MST_POP3)
        {
          DoMethod(gui->LV_POP3, MUIM_NList_InsertSingle, msn, MUIV_NList_Insert_Bottom);
          numPOP++;
        }
        else if(msn->type == MST_SMTP)
        {
          DoMethod(gui->LV_SMTP, MUIM_NList_InsertSingle, msn, MUIV_NList_Insert_Bottom);
          numSMTP++;
        }
      }

      // make sure the first entry is selected per default
      xset(gui->LV_POP3, MUIA_NList_Quiet, FALSE,
                         MUIA_NList_Active, MUIV_NList_Active_Top);
      xset(gui->LV_SMTP, MUIA_NList_Quiet, FALSE,
                         MUIA_NList_Active, MUIV_NList_Active_Top);

      // set the enabled stated of the del button according to the number of available accounts
      set(gui->BT_PDEL, MUIA_Disabled, numPOP < 2);
      set(gui->BT_SDEL, MUIA_Disabled, numSMTP < 2);
    }
    break;

    case cp_Identities:
    {
      int numIdentities = 0;
      struct Node *curNode;
      struct UserIdentityNode *uin;

      // clear the lists first
      set(gui->LV_IDENTITY, MUIA_NList_Quiet, TRUE);
      DoMethod(gui->LV_IDENTITY, MUIM_NList_Clear);

      // we iterate through our user identity list and make sure to populate
      // out NList object correctly.
      IterateList(&CE->userIdentityList, curNode)
      {
        uin = (struct UserIdentityNode *)curNode;

        // if the description is empty we use the mail address instead
        if(uin->description[0] == '\0')
          strlcpy(uin->description, uin->address, sizeof(uin->description));

        DoMethod(gui->LV_IDENTITY, MUIM_NList_InsertSingle, uin, MUIV_NList_Insert_Bottom);
        numIdentities++;
      }

      // make sure the first entry is selected per default
      xset(gui->LV_IDENTITY, MUIA_NList_Quiet, FALSE,
                             MUIA_NList_Active, MUIV_NList_Active_Top);

      // set the enabled stated of the del button according to the number of available identities
      set(gui->BT_IDEL, MUIA_Disabled, numIdentities < 2);
    }
    break;

    case cp_NewMail:
    {
      setcycle(gui->CY_TRANSWIN, CE->TransferWindow);
      setcheckmark(gui->CH_UPDSTAT, CE->UpdateStatus);
      set(gui->ST_WARNSIZE, MUIA_String_Integer, CE->WarnSize);
      set(gui->NM_INTERVAL, MUIA_Numeric_Value, CE->CheckMailDelay);
      setcheckmark(gui->CH_DLLARGE, CE->DownloadLarge);
      setcheckmark(gui->CH_NOTIREQ, hasRequesterNotify(CE->NotifyType));
      setcheckmark(gui->CH_NOTIOS41SYSTEM, hasOS41SystemNotify(CE->NotifyType));
      setcheckmark(gui->CH_NOTISOUND, hasSoundNotify(CE->NotifyType));
      setcheckmark(gui->CH_NOTICMD, hasCommandNotify(CE->NotifyType));
      setstring(gui->ST_NOTISOUND ,CE->NotifySound);
      setstring(gui->ST_NOTICMD,CE->NotifyCommand);
    }
    break;

    case cp_Filters:
    {
      struct Node *curNode;

      // clear the filter list first
      DoMethod(gui->LV_RULES, MUIM_NList_Clear);

      // iterate through our filter list and add it to our
      // MUI List
      IterateList(&CE->filterList, curNode)
        DoMethod(gui->LV_RULES, MUIM_NList_InsertSingle, curNode, MUIV_NList_Insert_Bottom);

      // make sure the first entry is selected per default
      set(gui->LV_RULES, MUIA_NList_Active, MUIV_NList_Active_Top);
    }
    break;

    case cp_Spam:
    {
      char buf[SIZE_DEFAULT];

      setcheckmark(gui->CH_SPAMFILTERENABLED, CE->SpamFilterEnabled);
      setcheckmark(gui->CH_SPAMFILTERFORNEWMAIL, CE->SpamFilterForNewMail);
      setcheckmark(gui->CH_SPAMMARKONMOVE, CE->SpamMarkOnMove);
      setcheckmark(gui->CH_SPAMMARKASREAD, CE->SpamMarkAsRead);
      setcheckmark(gui->CH_SPAMABOOKISWHITELIST, CE->SpamAddressBookIsWhiteList);
      setcheckmark(gui->CH_MOVEHAMTOINCOMING, CE->MoveHamToIncoming);
      setcheckmark(gui->CH_FILTERHAM, CE->FilterHam);
      snprintf(buf, sizeof(buf), tr(MSG_CO_SPAM_STATISTICS), BayesFilterNumberOfHamClassifiedMails(), BayesFilterNumberOfHamClassifiedWords());
      set(gui->TX_SPAMGOODCOUNT, MUIA_Text_Contents, buf);
      snprintf(buf, sizeof(buf), tr(MSG_CO_SPAM_STATISTICS), BayesFilterNumberOfSpamClassifiedMails(), BayesFilterNumberOfSpamClassifiedWords());
      set(gui->TX_SPAMBADCOUNT, MUIA_Text_Contents, buf);
    }
    break;

    case cp_Read:
    {
      setcycle(gui->CY_HEADER, CE->ShowHeader);
      setstring(gui->ST_HEADERS, CE->ShortHeaders);
      setcycle(gui->CY_SENDERINFO, CE->ShowSenderInfo);
      setcycle(gui->CY_SIGSEPLINE, CE->SigSepLine);
      set(gui->CA_COLSIG,   MUIA_Pendisplay_Spec, &CE->ColorSignature);
      set(gui->CA_COLTEXT,  MUIA_Pendisplay_Spec, &CE->ColoredText);
      set(gui->CA_COL1QUOT, MUIA_Pendisplay_Spec, &CE->Color1stLevel);
      set(gui->CA_COL2QUOT, MUIA_Pendisplay_Spec, &CE->Color2ndLevel);
      set(gui->CA_COL3QUOT, MUIA_Pendisplay_Spec, &CE->Color3rdLevel);
      set(gui->CA_COL4QUOT, MUIA_Pendisplay_Spec, &CE->Color4thLevel);
      set(gui->CA_COLURL,   MUIA_Pendisplay_Spec, &CE->ColorURL);
      setcheckmark(gui->CH_ALLTEXTS, CE->DisplayAllTexts);
      setcheckmark(gui->CH_FIXFEDIT, CE->FixedFontEdit);
      setcheckmark(gui->CH_WRAPHEAD, CE->WrapHeader);
      setcheckmark(gui->CH_TEXTSTYLES_READ, CE->UseTextStylesRead);
      setcheckmark(gui->CH_TEXTCOLORS_READ, CE->UseTextColorsRead);

      // disable all poppen objects according to the UseTextColorsRead setting
      DoMethod(G->App, MUIM_MultiSet, MUIA_Disabled, CE->UseTextColorsRead == FALSE, gui->CA_COLSIG,
                                                                                     gui->CA_COLTEXT,
                                                                                     gui->CA_COL1QUOT,
                                                                                     gui->CA_COL2QUOT,
                                                                                     gui->CA_COL3QUOT,
                                                                                     gui->CA_COL4QUOT,
                                                                                     gui->CA_COLURL,
                                                                                     NULL);

      setcheckmark(gui->CH_SHOWALTPARTS, CE->DisplayAllAltPart);

      // set the MDN stuff according to other config
      setcheckmark(gui->CH_MDN_NEVER, CE->MDNEnabled == FALSE);
      setcheckmark(gui->CH_MDN_ALLOW, CE->MDNEnabled == TRUE);
      setcycle(gui->CY_MDN_NORECIPIENT, CE->MDN_NoRecipient);
      setcycle(gui->CY_MDN_NODOMAIN, CE->MDN_NoDomain);
      setcycle(gui->CY_MDN_DELETE, CE->MDN_OnDelete);
      setcycle(gui->CY_MDN_OTHER, CE->MDN_Other);

      setcheckmark(gui->CH_MULTIWIN, CE->MultipleReadWindows);
      setcheckmark(gui->CH_DELAYEDSTATUS, CE->StatusChangeDelayOn);

      xset(gui->NB_DELAYEDSTATUS, MUIA_Numeric_Value, CE->StatusChangeDelay / 1000,
                                  MUIA_Disabled, CE->StatusChangeDelayOn == FALSE);

      setcheckmark(gui->CH_CONVERTHTML, CE->ConvertHTML);

      set(gui->ST_HEADERS, MUIA_Disabled, CE->ShowHeader == HM_NOHEADER || CE->ShowHeader == HM_FULLHEADER);
      set(gui->CY_SENDERINFO, MUIA_Disabled, CE->ShowHeader == HM_NOHEADER);
      set(gui->CH_WRAPHEAD, MUIA_Disabled, CE->ShowHeader == HM_NOHEADER);
      nnset(gui->TX_DEFCHARSET_READ,  MUIA_Text_Contents, CE->DefaultReadCharset);
      setcheckmark(gui->CH_DETECTCYRILLIC, CE->DetectCyrillic);
      setcheckmark(gui->CH_MAPFOREIGNCHARS, CE->MapForeignChars);
      setcheckmark(gui->CH_GLOBALMAILTHREADS, CE->GlobalMailThreads);
    }
    break;

    case cp_Write:
    {
      setstring(gui->ST_HELLOTEXT, CE->NewIntro);
      setstring(gui->ST_BYETEXT, CE->Greetings);
      setcheckmark(gui->CH_WARNSUBJECT, CE->WarnSubject);
      xset(gui->ST_EDWRAP, MUIA_String_Integer, CE->EdWrapCol,
                           MUIA_Disabled, CE->EdWrapMode == EWM_OFF);
      setcycle(gui->CY_EDWRAP, CE->EdWrapMode);
      setstring(gui->ST_EDITOR, CE->Editor);
      setcheckmark(gui->CH_LAUNCH, CE->LaunchAlways);
      setslider(gui->NB_EMAILCACHE, CE->EmailCache);
      setslider(gui->NB_AUTOSAVE, CE->AutoSave/60);
      nnset(gui->TX_DEFCHARSET_WRITE,  MUIA_Text_Contents, CE->DefaultWriteCharset);
      setcheckmark(gui->CH_FIXEDFONT_WRITE, CE->UseFixedFontWrite);
      setcheckmark(gui->CH_TEXTSTYLES_WRITE, CE->UseTextStylesWrite);
      setcheckmark(gui->CH_TEXTCOLORS_WRITE, CE->UseTextColorsWrite);
    }
    break;

    case cp_ReplyForward:
    {
      setstring(gui->ST_REPLYHI, CE->ReplyHello);
      setstring(gui->ST_REPLYTEXT, CE->ReplyIntro);
      setstring(gui->ST_REPLYBYE, CE->ReplyBye);
      setstring(gui->ST_AREPLYHI, CE->AltReplyHello);
      setstring(gui->ST_AREPLYTEXT, CE->AltReplyIntro);
      setstring(gui->ST_AREPLYBYE, CE->AltReplyBye);
      setstring(gui->ST_AREPLYPAT, CE->AltReplyPattern);
      setstring(gui->ST_MREPLYHI, CE->MLReplyHello);
      setstring(gui->ST_MREPLYTEXT, CE->MLReplyIntro);
      setstring(gui->ST_MREPLYBYE, CE->MLReplyBye);
      setstring(gui->ST_FWDSTART, CE->ForwardIntro);
      setstring(gui->ST_FWDEND, CE->ForwardFinish);
      setcheckmark(gui->CH_QUOTEEMPTY, CE->QuoteEmptyLines);
      setcheckmark(gui->CH_COMPADDR, CE->CompareAddress);
      setcheckmark(gui->CH_STRIPSIG, CE->StripSignature);
      setcycle(gui->CY_FORWARDMODE, CE->ForwardMode);
    }
    break;

    case cp_Signature:
    {
      char sigPath[SIZE_PATHFILE];

      setstring(gui->ST_TAGFILE, CE->TagsFile);
      setstring(gui->ST_TAGSEP, CE->TagsSeparator);
      setcycle(gui->CY_SIGNAT, G->CO->LastSig);
      DoMethod(gui->TE_SIGEDIT, MUIM_MailTextEdit_LoadFromFile, CreateFilename(SigNames[G->CO->LastSig], sigPath, sizeof(sigPath)), MUIF_MailTextEdit_LoadFromFile_UseStyles|MUIF_MailTextEdit_LoadFromFile_UseColors);
    }
    break;

    case cp_Lists:
    {
      for(i=0; i < FOCOLNUM; i++)
      {
        setcheckmark(gui->CH_FCOLS[i], isFlagSet(CE->FolderCols, (1<<i)));
      }

      for(i=0; i < MACOLNUM; i++)
      {
        setcheckmark(gui->CH_MCOLS[i], isFlagSet(CE->MessageCols, (1<<i)));
      }

      setcheckmark(gui->CH_FIXFLIST, CE->FixedFontList);
      setcheckmark(gui->CH_ABOOKLOOKUP, CE->ABookLookup);
      setcheckmark(gui->CH_FCNTMENU, CE->FolderCntMenu);
      setcheckmark(gui->CH_MCNTMENU, CE->MessageCntMenu);
      setcheckmark(gui->CH_BEAT, (CE->DSListFormat == DSS_DATEBEAT || CE->DSListFormat == DSS_RELDATEBEAT));
      setcheckmark(gui->CH_RELDATETIME, (CE->DSListFormat == DSS_RELDATETIME || CE->DSListFormat == DSS_RELDATEBEAT));
      setcheckmark(gui->CH_FOLDERDBLCLICK, CE->FolderDoubleClick);
      setcycle(gui->CY_FOLDERINFO, CE->FolderInfoMode);
    }
    break;

    case cp_Security:
    {
      setstring(gui->ST_PGPCMD, CE->PGPCmdPath);
      setstring(gui->ST_LOGFILE, CE->LogfilePath);
      setcycle(gui->CY_LOGMODE, CE->LogfileMode);
      setcheckmark(gui->CH_SPLITLOG, CE->SplitLogfile);
      setcheckmark(gui->CH_LOGALL, CE->LogAllEvents);
      setcheckmark(gui->CH_PGPPASSINTERVAL, CE->PGPPassInterval > 0);

      xset(gui->NB_PGPPASSINTERVAL, MUIA_Numeric_Value, abs(CE->PGPPassInterval),
                                    MUIA_Disabled, CE->PGPPassInterval <= 0);

      DoMethod(G->App, MUIM_MultiSet, MUIA_Disabled, CE->LogfileMode == LF_NONE, gui->PO_LOGFILE,
                                                                                 gui->CH_SPLITLOG,
                                                                                 gui->CH_LOGALL,
                                                                                 NULL);
    }
    break;

    case cp_StartupQuit:
    {
      setcheckmark(gui->CH_SENDSTART  ,CE->SendOnStartup);
      setcheckmark(gui->CH_DELETESTART,CE->CleanupOnStartup);
      setcheckmark(gui->CH_REMOVESTART,CE->RemoveOnStartup);
      setcheckmark(gui->CH_LOADALL    ,CE->LoadAllFolders);
      setcheckmark(gui->CH_MARKNEW    ,CE->UpdateNewMail);
      setcheckmark(gui->CH_CHECKBD    ,CE->CheckBirthdates);
      setcheckmark(gui->CH_SENDQUIT   ,CE->SendOnQuit);
      setcheckmark(gui->CH_DELETEQUIT ,CE->CleanupOnQuit);
      setcheckmark(gui->CH_REMOVEQUIT ,CE->RemoveOnQuit);
    }
    break;

    case cp_MIME:
    {
      struct Node *curNode;

      // clear the filter list first
      set(gui->LV_MIME, MUIA_NList_Quiet, TRUE);
      DoMethod(gui->LV_MIME, MUIM_NList_Clear);

      // iterate through our filter list and add it to our
      // MUI List
      IterateList(&CE->mimeTypeList, curNode)
        DoMethod(gui->LV_MIME, MUIM_NList_InsertSingle, curNode, MUIV_NList_Insert_Bottom);

      // sort the list after inserting all entries
      DoMethod(gui->LV_MIME, MUIM_NList_Sort);
      set(gui->LV_MIME, MUIA_NList_Quiet, FALSE);

      // make sure the first entry is selected per default and the listview
      // is able to display it (jump to it)
      set(gui->LV_MIME, MUIA_NList_Active, MUIV_NList_Active_Top);
      DoMethod(gui->LV_MIME, MUIM_NList_Jump, MUIV_NList_Jump_Active);

      setstring(gui->ST_DEFVIEWER, CE->DefaultMimeViewer);
    }
    break;

    case cp_AddressBook:
    {
      setstring(gui->ST_GALLDIR, CE->GalleryDir);
      setstring(gui->ST_NEWGROUP, CE->NewAddrGroup);
      set(gui->ST_NEWGROUP, MUIA_Disabled, CE->AddToAddrbook == 0);
      setstring(gui->ST_PROXY, CE->ProxyServer);
      setcycle(gui->CY_ATAB, CE->AddToAddrbook);
      for(i = 0; i < ABCOLNUM; i++)
        setcheckmark(gui->CH_ACOLS[i], (CE->AddrbookCols & (1<<i)) != 0);
    }
    break;

    case cp_Scripts:
    {
      int act = xget(gui->LV_REXX, MUIA_NList_Active);
      struct RxHook *rh = &(CE->RX[act]);

      nnset(gui->ST_RXNAME, MUIA_String_Contents, act < 10 ? rh->Name : "");
      nnset(gui->ST_SCRIPT, MUIA_String_Contents, rh->Script);
      nnset(gui->CY_ISADOS, MUIA_Cycle_Active, rh->IsAmigaDOS ? 1 : 0);
      nnset(gui->CH_CONSOLE, MUIA_Selected, rh->UseConsole);
      nnset(gui->CH_WAITTERM, MUIA_Selected, rh->WaitTerm);
      set(gui->ST_RXNAME, MUIA_Disabled, act >= 10);
      set(gui->LV_REXX, MUIA_NList_Active, 0);
      DoMethod(gui->LV_REXX, MUIM_NList_Redraw, MUIV_NList_Redraw_All);
    }
    break;

    case cp_Mixed:
    {
      setstring(gui->ST_TEMPDIR, CE->TempDir);
      setstring(gui->ST_DETACHDIR, CE->DetachDir);
      setstring(gui->ST_ATTACHDIR, CE->AttachDir);
      setcheckmark(gui->CH_WBAPPICON, CE->WBAppIcon);
      set(gui->ST_APPX, MUIA_String_Integer, abs(CE->IconPositionX));
      set(gui->ST_APPY, MUIA_String_Integer, abs(CE->IconPositionY));
      setcheckmark(gui->CH_APPICONPOS, CE->IconPositionX >= 0 && CE->IconPositionY >= 0);
      setstring(gui->ST_APPICON, CE->AppIconText);
      setcheckmark(gui->CH_DOCKYICON, CE->DockyIcon);
      setcheckmark(gui->CH_CLGADGET, CE->IconifyOnQuit);
      setcheckmark(gui->CH_CONFIRM, CE->Confirm);

      xset(gui->NB_CONFIRMDEL, MUIA_Numeric_Value, CE->ConfirmDelete,
                               MUIA_Disabled, CE->Confirm == FALSE);

      setcheckmark(gui->CH_REMOVE, CE->RemoveAtOnce);
      set(gui->TX_PACKER, MUIA_Text_Contents, CE->XPKPack);
      set(gui->TX_ENCPACK, MUIA_Text_Contents, CE->XPKPackEncrypt);
      setslider(gui->NB_PACKER, CE->XPKPackEff);
      setslider(gui->NB_ENCPACK, CE->XPKPackEncryptEff);
      setstring(gui->ST_ARCHIVER, CE->PackerCommand);

      set(gui->CH_APPICONPOS, MUIA_Disabled, CE->WBAppIcon == FALSE);
    }
    break;

    case cp_LookFeel:
    {
      setcycle(gui->CY_INFOBAR, CE->InfoBar);
      setstring(gui->ST_INFOBARTXT, CE->InfoBarText);
      setcheckmark(gui->CH_QUICKSEARCHBAR, CE->QuickSearchBar);
      setcheckmark(gui->CH_EMBEDDEDREADPANE, CE->EmbeddedReadPane);
      setcycle(gui->CY_SIZE, CE->SizeFormat);

      set(gui->PO_INFOBARTXT, MUIA_Disabled, CE->InfoBar == IB_POS_OFF);

      // update the themeslist and set the current one
      // as active
      DoMethod(gui->GR_THEMES, MUIM_ThemeListGroup_Update);
    }
    break;

    case cp_Update:
    {
      struct UpdateState state;

      // copy the last update state information
      GetLastUpdateState(&state);

      if(CE->UpdateInterval > 0)
      {
        if(CE->UpdateInterval <= 86400)
          setcycle(gui->CY_UPDATEINTERVAL, 1); // daily
        else if(CE->UpdateInterval <= 604800)
          setcycle(gui->CY_UPDATEINTERVAL, 2); // weekly
        else
          setcycle(gui->CY_UPDATEINTERVAL, 3); // monthly
      }
      else
        setcycle(gui->CY_UPDATEINTERVAL, 0);

      // now we set the information on the last update check
      switch(state.LastUpdateStatus)
      {
        case UST_NOCHECK:
          set(gui->TX_UPDATESTATUS, MUIA_Text_Contents, tr(MSG_CO_LASTSTATUS_NOCHECK));
        break;

        case UST_NOUPDATE:
          set(gui->TX_UPDATESTATUS, MUIA_Text_Contents, tr(MSG_CO_LASTSTATUS_NOUPDATE));
        break;

        case UST_NOQUERY:
          set(gui->TX_UPDATESTATUS, MUIA_Text_Contents, tr(MSG_CO_LASTSTATUS_NOQUERY));
        break;

        case UST_UPDATESUCCESS:
          set(gui->TX_UPDATESTATUS, MUIA_Text_Contents, tr(MSG_CO_LASTSTATUS_UPDATESUCCESS));
        break;
      }

      // set the lastUpdateCheckDate
      if(state.LastUpdateStatus != UST_NOCHECK && state.LastUpdateCheck.Seconds > 0)
      {
        char buf[SIZE_DEFAULT];

        TimeVal2String(buf, sizeof(buf), &state.LastUpdateCheck, DSS_DATETIME, TZC_NONE);
        set(gui->TX_UPDATEDATE, MUIA_Text_Contents, buf);
      }
      else
      {
        // no update check was yet performed, so we clear our status gadgets
        set(gui->TX_UPDATEDATE, MUIA_Text_Contents, "");
      }

      setstring(gui->ST_UPDATEDOWNLOADPATH, CE->UpdateDownloadPath);
    }
    break;

    case cp_Max:
    {
      // nothing
    }
    break;
  }

  LEAVE();
}
///

