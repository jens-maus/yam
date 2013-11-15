/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2013 YAM Open Source Team

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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <clib/alib_protos.h>
#include <diskfont/diskfonttag.h>
#include <libraries/amisslmaster.h>
#include <libraries/asl.h>
#include <libraries/iffparse.h>
#include <libraries/locale.h>
#include <mui/NListview_mcc.h>
#include <mui/NListtree_mcc.h>
#include <proto/codesets.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/diskfont.h>
#include <proto/intuition.h>
#include <proto/muimaster.h>
#include <proto/utility.h>

#if defined(__amigaos4__)
#include <proto/application.h>
#endif

#include "extrasrc.h"

#include "SDI_hook.h"

#include "YAM.h"
#include "YAM_error.h"
#include "YAM_find.h"
#include "YAM_folderconfig.h"
#include "YAM_global.h"
#include "YAM_main.h"
#include "YAM_mainFolder.h"
#include "YAM_utilities.h"

#include "mui/ClassesExtra.h"
#include "mui/AddressBookConfigPage.h"
#include "mui/AddressBookWindow.h"
#include "mui/ConfigPage.h"
#include "mui/ConfigPageList.h"
#include "mui/ConfigWindow.h"
#include "mui/FilterRuleList.h"
#include "mui/FiltersConfigPage.h"
#include "mui/FirstStepsConfigPage.h"
#include "mui/IdentitiesConfigPage.h"
#include "mui/InfoBar.h"
#include "mui/LookFeelConfigPage.h"
#include "mui/MainFolderListtree.h"
#include "mui/MainMailListGroup.h"
#include "mui/MainWindow.h"
#include "mui/MainWindowToolbar.h"
#include "mui/MimeConfigPage.h"
#include "mui/MixedConfigPage.h"
#include "mui/ReadConfigPage.h"
#include "mui/ReadMailGroup.h"
#include "mui/ReadWindow.h"
#include "mui/ReplyForwardConfigPage.h"
#include "mui/ScriptsConfigPage.h"
#include "mui/SecurityConfigPage.h"
#include "mui/SignatureConfigPage.h"
#include "mui/SignatureTextEdit.h"
#include "mui/SpamConfigPage.h"
#include "mui/StartupQuitConfigPage.h"
#include "mui/TCPIPConfigPage.h"
#include "mui/UpdateConfigPage.h"
#include "mui/WriteConfigPage.h"
#include "mui/WriteWindow.h"
#include "mui/YAMApplication.h"

#include "Busy.h"
#include "Config.h"
#include "DockyIcon.h"
#include "DynamicString.h"
#include "FileInfo.h"
#include "FolderList.h"
#include "Locale.h"
#include "MimeTypes.h"
#include "MailList.h"
#include "MailServers.h"
#include "MUIObjects.h"
#include "ParseEmail.h"
#include "Requesters.h"
#include "Signature.h"
#include "UpdateCheck.h"
#include "UserIdentity.h"
#include "TZone.h"

#include "Debug.h"

extern struct Library *AmiSSLBase;
extern struct Library *AmiSSLMasterBase;

struct Config *C = NULL;
struct Config *CE = NULL;

/* local defines */
#if defined(__amigaos4__)
#define SYS_EDITOR "SYS:Utilities/NotePad"
#elif defined(__AROS__)
#define SYS_EDITOR "SYS:Tools/Editor"
#else
#define SYS_EDITOR "C:Ed"
#endif

// define the current version of the config
// This will also be the latest version this YAM
// version will try to load without warning the user
// everything > CONFIG_VERSION will cause YAM to raise
// a warning because things may have changed in the
// future which we do not support/understand.
#define LATEST_CFG_VERSION 6

/// InitConfig
// initialize a config structure
static void InitConfig(struct Config *co)
{
  ENTER();

  NewMinList(&co->pop3ServerList);
  NewMinList(&co->smtpServerList);
  NewMinList(&co->filterList);
  NewMinList(&co->mimeTypeList);
  NewMinList(&co->userIdentityList);
  NewMinList(&co->signatureList);

  LEAVE();
}

///
/// AllocConfig
// allocate and initialize a config structure
struct Config *AllocConfig(void)
{
  struct Config *co;

  ENTER();

  if((co = calloc(1, sizeof(*co))) != NULL)
    InitConfig(co);

  RETURN(co);
  return co;
}

///
/// FreeConfig
// free a config structure
void FreeConfig(struct Config *co)
{
  ENTER();

  if(co != NULL)
  {
    ClearConfig(co);
    free(co);
  }

  LEAVE();
}

///
/// ClearConfig
// clears the content of a configuration structure
void ClearConfig(struct Config *co)
{
  ENTER();

  // free the signatureList
  FreeSignatureList(&co->signatureList);

  // free the userIdentityList
  FreeUserIdentityList(&co->userIdentityList);

  // free the pop3ServerList and smtpServerList
  FreeMailServerList(&co->pop3ServerList);
  FreeMailServerList(&co->smtpServerList);

  // free the mimeTypeList
  FreeMimeTypeList(&co->mimeTypeList);

  // free the filterList
  FreeFilterList(&co->filterList);

  // clear and initialize the config
  memset(co, 0, sizeof(*co));
  InitConfig(co);

  LEAVE();
}

///
/// CopyConfig
// copies a configuration structure (deep copy)
BOOL CopyConfig(struct Config *dco, const struct Config *sco)
{
  BOOL success = TRUE;

  ENTER();

  // first we copy all raw data via memcpy
  memcpy(dco, sco, sizeof(struct Config));

  // then we have to do a deep copy and allocate separate memory for our copy
  NewMinList(&dco->pop3ServerList);

  if(success == TRUE)
  {
    struct MailServerNode *srcNode;

    IterateList(&sco->pop3ServerList, struct MailServerNode *, srcNode)
    {
      struct MailServerNode *dstNode;

      // clone the server but give the clone its own private data
      if((dstNode = CloneMailServer(srcNode)) != NULL)
      {
        AddTail((struct List *)&dco->pop3ServerList, (struct Node *)dstNode);
      }
      else
      {
        success = FALSE;

        // bail out, no need to copy further data
        break;
      }
    }
  }

  // then we have to do a deep copy and allocate separate memory for our copy
  NewMinList(&dco->smtpServerList);

  if(success == TRUE)
  {
    struct MailServerNode *srcNode;

    IterateList(&sco->smtpServerList, struct MailServerNode *, srcNode)
    {
      struct MailServerNode *dstNode;

      // clone the server but give the clone its own private data
      if((dstNode = CloneMailServer(srcNode)) != NULL)
      {
        AddTail((struct List *)&dco->smtpServerList, (struct Node *)dstNode);
      }
      else
      {
        success = FALSE;

        // bail out, no need to copy further data
        break;
      }
    }
  }

  // for copying the signature list we have to do a deep copy of the list
  NewMinList(&dco->signatureList);

  if(success == TRUE)
  {
    struct SignatureNode *srcNode;

    IterateList(&sco->signatureList, struct SignatureNode *, srcNode)
    {
      struct SignatureNode *dstNode;

      if((dstNode = DuplicateNode(srcNode, sizeof(*srcNode))) != NULL)
      {
        dstNode->signature = NULL;
        if(srcNode->signature != NULL)
          dstrcpy(&dstNode->signature, srcNode->signature);
        AddTail((struct List *)&dco->signatureList, (struct Node *)dstNode);
      }
      else
      {
        success = FALSE;
        // bail out, no need to copy further data
        break;
      }
    }
  }

  // for copying the user identity list we have to do a deep copy of the list
  NewMinList(&dco->userIdentityList);

  if(success == TRUE)
  {
    struct UserIdentityNode *srcNode;

    IterateList(&sco->userIdentityList, struct UserIdentityNode *, srcNode)
    {
      struct UserIdentityNode *dstNode;

      if((dstNode = DuplicateNode(srcNode, sizeof(*srcNode))) != NULL)
      {
        // make sure the mailserver of the copied node points to an
        // entry of the copied mail server list
        if(srcNode->smtpServer != NULL)
          dstNode->smtpServer = FindMailServer(&dco->smtpServerList, srcNode->smtpServer->id);
        else
          dstNode->smtpServer = NULL;

        // make sure the signature of the copied node points to an
        // entry of the copied signature list
        if(srcNode->signature != NULL)
          dstNode->signature = FindSignatureByID(&dco->signatureList, srcNode->signature->id);
        else
          dstNode->signature = NULL;

        AddTail((struct List *)&dco->userIdentityList, (struct Node *)dstNode);
      }
      else
      {
        success = FALSE;
        // bail out, no need to copy further data
        break;
      }
    }
  }

  // for copying the mimetype list we have to do a deep copy of the list
  NewMinList(&dco->mimeTypeList);

  if(success == TRUE)
  {
    struct MimeTypeNode *srcNode;

    IterateList(&sco->mimeTypeList, struct MimeTypeNode *, srcNode)
    {
      struct MimeTypeNode *dstNode;

      if((dstNode = DuplicateNode(srcNode, sizeof(*srcNode))) != NULL)
        AddTail((struct List *)&dco->mimeTypeList, (struct Node *)dstNode);
      else
      {
        success = FALSE;
        // bail out, no need to copy further data
        break;
      }
    }
  }

  // for copying the filters we do have to do another deep copy
  NewMinList(&dco->filterList);

  if(success == TRUE)
  {
    struct FilterNode *srcFilter;

    IterateList(&sco->filterList, struct FilterNode *, srcFilter)
    {
      struct FilterNode *dstFilter;

      if((dstFilter = AllocSysObjectTags(ASOT_NODE,
        ASONODE_Size, sizeof(*dstFilter),
        ASONODE_Min, TRUE,
        TAG_DONE)) != NULL)
      {
        if(CopyFilterData(dstFilter, srcFilter) == FALSE)
        {
          success = FALSE;
          // bail out, no need to copy further data
          break;
        }

        AddTail((struct List *)&dco->filterList, (struct Node *)dstFilter);
      }
      else
      {
        success = FALSE;
        // bail out, no need to copy further data
        break;
      }
    }
  }

  // remember that this configuration is not yet saved
  dco->ConfigIsSaved = FALSE;

  // free the copied configuration in case anything failed
  if(success == FALSE)
    ClearConfig(dco);

  // return if everything could be duplicated successfully
  RETURN(success);
  return success;
}

///
/// CompareRxHooks
// compare two RxHook structures to be equal
static BOOL CompareRxHooks(const struct RxHook *rx1, const struct RxHook *rx2)
{
  BOOL equal = TRUE;
  int i;

  ENTER();

  for(i = 0; i < MACRO_COUNT; i++)
  {
    const struct RxHook *r1 = &rx1[i];
    const struct RxHook *r2 = &rx2[i];

    if(r1->IsAmigaDOS  != r2->IsAmigaDOS ||
       r1->UseConsole  != r2->UseConsole ||
       r1->WaitTerm    != r2->WaitTerm ||
       strcmp(r1->Name,   r2->Name) != 0 ||
       strcmp(r1->Script, r2->Script) != 0)
    {
      // something does not match
      equal = FALSE;
      break;
    }
  }

  RETURN(equal);
  return equal;
}

///
/// CompareConfigs
// compares two config data structures (deep compare) and returns TRUE if they are equal
BOOL CompareConfigs(const struct Config *c1, const struct Config *c2)
{
  BOOL equal = FALSE;

  ENTER();

  // we do a deep compare here, but start the compare by comparing our normal
  // plain variables as this will be the faster compare than the compares
  // of our nested structures/lists, etc.
  if(c1->ShowHeader                      == c2->ShowHeader &&
     c1->ShowSenderInfo                  == c2->ShowSenderInfo &&
     c1->EdWrapCol                       == c2->EdWrapCol &&
     c1->EdWrapMode                      == c2->EdWrapMode &&
     c1->FolderCols                      == c2->FolderCols &&
     c1->MessageCols                     == c2->MessageCols &&
     c1->AddToAddrbook                   == c2->AddToAddrbook &&
     c1->AddrbookCols                    == c2->AddrbookCols &&
     c1->IconPositionX                   == c2->IconPositionX &&
     c1->IconPositionY                   == c2->IconPositionY &&
     c1->ConfirmDelete                   == c2->ConfirmDelete &&
     c1->XPKPackEff                      == c2->XPKPackEff &&
     c1->XPKPackEncryptEff               == c2->XPKPackEncryptEff &&
     c1->LetterPart                      == c2->LetterPart &&
     c1->WriteIndexes                    == c2->WriteIndexes &&
     c1->ExpungeIndexes                  == c2->ExpungeIndexes &&
     c1->AutoSave                        == c2->AutoSave &&
     c1->ShowRcptFieldCC                 == c2->ShowRcptFieldCC &&
     c1->ShowRcptFieldBCC                == c2->ShowRcptFieldBCC &&
     c1->ShowRcptFieldReplyTo            == c2->ShowRcptFieldReplyTo &&
     c1->HideGUIElements                 == c2->HideGUIElements &&
     c1->StackSize                       == c2->StackSize &&
     c1->SizeFormat                      == c2->SizeFormat &&
     c1->EmailCache                      == c2->EmailCache &&
     c1->TRBufferSize                    == c2->TRBufferSize &&
     c1->EmbeddedMailDelay               == c2->EmbeddedMailDelay &&
     c1->StatusChangeDelay               == c2->StatusChangeDelay &&
     c1->KeepAliveInterval               == c2->KeepAliveInterval &&
     c1->UpdateInterval                  == c2->UpdateInterval &&
     c1->PGPPassInterval                 == c2->PGPPassInterval &&
     c1->SpamProbabilityThreshold        == c2->SpamProbabilityThreshold &&
     c1->SpamFlushTrainingDataInterval   == c2->SpamFlushTrainingDataInterval &&
     c1->SpamFlushTrainingDataThreshold  == c2->SpamFlushTrainingDataThreshold &&
     c1->SocketTimeout                   == c2->SocketTimeout &&
     c1->PrintMethod                     == c2->PrintMethod &&
     c1->LogfileMode                     == c2->LogfileMode &&
     c1->MDN_NoRecipient                 == c2->MDN_NoRecipient &&
     c1->MDN_NoDomain                    == c2->MDN_NoDomain &&
     c1->MDN_OnDelete                    == c2->MDN_OnDelete &&
     c1->MDN_Other                       == c2->MDN_Other &&
     c1->DSListFormat                    == c2->DSListFormat &&
     c1->SigSepLine                      == c2->SigSepLine &&
     c1->TransferWindow                  == c2->TransferWindow &&
     c1->FolderInfoMode                  == c2->FolderInfoMode &&
     c1->ForwardMode                     == c2->ForwardMode &&
     c1->InfoBarPos                      == c2->InfoBarPos &&
     c1->QuickSearchBarPos               == c2->QuickSearchBarPos &&
     c1->DisplayAllTexts                 == c2->DisplayAllTexts &&
     c1->FixedFontEdit                   == c2->FixedFontEdit &&
     c1->MultipleReadWindows             == c2->MultipleReadWindows &&
     c1->UseTextStylesRead               == c2->UseTextStylesRead &&
     c1->UseTextColorsRead               == c2->UseTextColorsRead &&
     c1->UseFixedFontWrite               == c2->UseFixedFontWrite &&
     c1->UseTextStylesWrite              == c2->UseTextStylesWrite &&
     c1->UseTextColorsWrite              == c2->UseTextColorsWrite &&
     c1->WrapHeader                      == c2->WrapHeader &&
     c1->LaunchAlways                    == c2->LaunchAlways &&
     c1->QuoteEmptyLines                 == c2->QuoteEmptyLines &&
     c1->CompareAddress                  == c2->CompareAddress &&
     c1->StripSignature                  == c2->StripSignature &&
     c1->FixedFontList                   == c2->FixedFontList &&
     c1->SplitLogfile                    == c2->SplitLogfile &&
     c1->LogAllEvents                    == c2->LogAllEvents &&
     c1->SendOnStartup                   == c2->SendOnStartup &&
     c1->CleanupOnStartup                == c2->CleanupOnStartup &&
     c1->RemoveOnStartup                 == c2->RemoveOnStartup &&
     c1->LoadAllFolders                  == c2->LoadAllFolders &&
     c1->UpdateNewMail                   == c2->UpdateNewMail &&
     c1->CheckBirthdates                 == c2->CheckBirthdates &&
     c1->SendOnQuit                      == c2->SendOnQuit &&
     c1->CleanupOnQuit                   == c2->CleanupOnQuit &&
     c1->RemoveOnQuit                    == c2->RemoveOnQuit &&
     c1->SaveLayoutOnQuit                == c2->SaveLayoutOnQuit &&
     c1->IconifyOnQuit                   == c2->IconifyOnQuit &&
     c1->Confirm                         == c2->Confirm &&
     c1->RemoveAtOnce                    == c2->RemoveAtOnce &&
     c1->JumpToIncoming                  == c2->JumpToIncoming &&
     c1->PrinterCheck                    == c2->PrinterCheck &&
     c1->IsOnlineCheck                   == c2->IsOnlineCheck &&
     c1->ConfirmOnQuit                   == c2->ConfirmOnQuit &&
     c1->AskJumpUnread                   == c2->AskJumpUnread &&
     c1->WarnSubject                     == c2->WarnSubject &&
     c1->AttachmentReminder              == c2->AttachmentReminder &&
     c1->FolderCntMenu                   == c2->FolderCntMenu &&
     c1->MessageCntMenu                  == c2->MessageCntMenu &&
     c1->AutoColumnResize                == c2->AutoColumnResize &&
     c1->EmbeddedReadPane                == c2->EmbeddedReadPane &&
     c1->StatusChangeDelayOn             == c2->StatusChangeDelayOn &&
     c1->SysCharsetCheck                 == c2->SysCharsetCheck &&
     c1->WBAppIcon                       == c2->WBAppIcon &&
     c1->DockyIcon                       == c2->DockyIcon &&
     c1->AmiSSLCheck                     == c2->AmiSSLCheck &&
     c1->DetectCyrillic                  == c2->DetectCyrillic &&
     c1->ABookLookup                     == c2->ABookLookup &&
     c1->ConvertHTML                     == c2->ConvertHTML &&
     c1->SpamFilterEnabled               == c2->SpamFilterEnabled &&
     c1->SpamFilterForNewMail            == c2->SpamFilterForNewMail &&
     c1->SpamMarkOnMove                  == c2->SpamMarkOnMove &&
     c1->SpamMarkAsRead                  == c2->SpamMarkAsRead &&
     c1->SpamAddressBookIsWhiteList      == c2->SpamAddressBookIsWhiteList &&
     c1->MoveHamToIncoming               == c2->MoveHamToIncoming &&
     c1->FilterHam                       == c2->FilterHam &&
     c1->SpamTrustExternalFilter         == c2->SpamTrustExternalFilter &&
     c1->DisplayAllAltPart               == c2->DisplayAllAltPart &&
     c1->MDNEnabled                      == c2->MDNEnabled &&
     c1->AutoClip                        == c2->AutoClip &&
     c1->FolderDoubleClick               == c2->FolderDoubleClick &&
     c1->MapForeignChars                 == c2->MapForeignChars &&
     c1->GlobalMailThreads               == c2->GlobalMailThreads &&
     c1->ShowFilterStats                 == c2->ShowFilterStats &&
     c1->ConfirmRemoveAttachments        == c2->ConfirmRemoveAttachments &&
     c1->OverrideFromAddress             == c2->OverrideFromAddress &&
     c1->ShowPackerProgress              == c2->ShowPackerProgress &&

     c1->SocketOptions.SendBuffer        == c2->SocketOptions.SendBuffer &&
     c1->SocketOptions.RecvBuffer        == c2->SocketOptions.RecvBuffer &&
     c1->SocketOptions.SendLowAt         == c2->SocketOptions.SendLowAt &&
     c1->SocketOptions.RecvLowAt         == c2->SocketOptions.RecvLowAt &&
     c1->SocketOptions.SendTimeOut       == c2->SocketOptions.SendTimeOut &&
     c1->SocketOptions.RecvTimeOut       == c2->SocketOptions.RecvTimeOut &&
     c1->SocketOptions.KeepAlive         == c2->SocketOptions.KeepAlive &&
     c1->SocketOptions.NoDelay           == c2->SocketOptions.NoDelay &&
     c1->SocketOptions.LowDelay          == c2->SocketOptions.LowDelay &&

     CompareMailServerLists(&c1->pop3ServerList, &c2->pop3ServerList) &&
     CompareMailServerLists(&c1->smtpServerList, &c2->smtpServerList) &&
     CompareUserIdentityLists(&c1->userIdentityList, &c2->userIdentityList) &&
     CompareSignatureLists(&c1->signatureList, &c2->signatureList) &&
     CompareFilterLists(&c1->filterList, &c2->filterList) &&
     CompareMimeTypeLists(&c1->mimeTypeList, &c2->mimeTypeList) &&
     CompareRxHooks((const struct RxHook *)c1->RX, (const struct RxHook *)c2->RX) &&

     strcmp(c1->Location,             c2->Location) == 0 &&
     strcmp(c1->ColoredText.buf,      c2->ColoredText.buf) == 0 &&
     strcmp(c1->Color1stLevel.buf,    c2->Color1stLevel.buf) == 0 &&
     strcmp(c1->Color2ndLevel.buf,    c2->Color2ndLevel.buf) == 0 &&
     strcmp(c1->Color3rdLevel.buf,    c2->Color3rdLevel.buf) == 0 &&
     strcmp(c1->Color4thLevel.buf,    c2->Color4thLevel.buf) == 0 &&
     strcmp(c1->ColorURL.buf,         c2->ColorURL.buf) == 0 &&
     strcmp(c1->ColorSignature.buf,   c2->ColorSignature.buf) == 0 &&
     strcmp(c1->ShortHeaders,         c2->ShortHeaders) == 0 &&
     strcmp(c1->NewIntro,             c2->NewIntro) == 0 &&
     strcmp(c1->Greetings,            c2->Greetings) == 0 &&
     strcmp(c1->Editor,               c2->Editor) == 0 &&
     strcmp(c1->ReplyHello,           c2->ReplyHello) == 0 &&
     strcmp(c1->ReplyIntro,           c2->ReplyIntro) == 0 &&
     strcmp(c1->ReplyBye,             c2->ReplyBye) == 0 &&
     strcmp(c1->AltReplyHello,        c2->AltReplyHello) == 0 &&
     strcmp(c1->AltReplyIntro,        c2->AltReplyIntro) == 0 &&
     strcmp(c1->AltReplyBye,          c2->AltReplyBye) == 0 &&
     strcmp(c1->AltReplyPattern,      c2->AltReplyPattern) == 0 &&
     strcmp(c1->MLReplyHello,         c2->MLReplyHello) == 0 &&
     strcmp(c1->MLReplyIntro,         c2->MLReplyIntro) == 0 &&
     strcmp(c1->MLReplyBye,           c2->MLReplyBye) == 0 &&
     strcmp(c1->ForwardIntro,         c2->ForwardIntro) == 0 &&
     strcmp(c1->ForwardFinish,        c2->ForwardFinish) == 0 &&
     strcmp(c1->TagsFile,             c2->TagsFile) == 0 &&
     strcmp(c1->TagsSeparator,        c2->TagsSeparator) == 0 &&
     strcmp(c1->PGPCmdPath,           c2->PGPCmdPath) == 0 &&
     strcmp(c1->LogfilePath,          c2->LogfilePath) == 0 &&
     strcmp(c1->DetachDir,            c2->DetachDir) == 0 &&
     strcmp(c1->AttachDir,            c2->AttachDir) == 0 &&
     strcmp(c1->GalleryDir,           c2->GalleryDir) == 0 &&
     strcmp(c1->NewAddrGroup,         c2->NewAddrGroup) == 0 &&
     strcmp(c1->ProxyServer,          c2->ProxyServer) == 0 &&
     strcmp(c1->TempDir,              c2->TempDir) == 0 &&
     strcmp(c1->PackerCommand,        c2->PackerCommand) == 0 &&
     strcmp(c1->XPKPack,              c2->XPKPack) == 0 &&
     strcmp(c1->XPKPackEncrypt,       c2->XPKPackEncrypt) == 0 &&
     strcmp(c1->SupportSite,          c2->SupportSite) == 0 &&
     strcmp(c1->UpdateServer,         c2->UpdateServer) == 0 &&
     strcmp(c1->DefaultLocalCodeset,  c2->DefaultLocalCodeset) == 0 &&
     strcmp(c1->DefaultWriteCodeset,  c2->DefaultWriteCodeset) == 0 &&
     strcmp(c1->DefaultEditorCodeset, c2->DefaultEditorCodeset) == 0 &&
     strcmp(c1->IOCInterfaces,        c2->IOCInterfaces) == 0 &&
     strcmp(c1->AppIconText,          c2->AppIconText) == 0 &&
     strcmp(c1->InfoBarText,          c2->InfoBarText) == 0 &&
     strcmp(c1->DefaultMimeViewer,    c2->DefaultMimeViewer) == 0 &&
     strcmp(c1->StyleFGroupUnread,    c2->StyleFGroupUnread) == 0 &&
     strcmp(c1->StyleFGroupRead,      c2->StyleFGroupRead) == 0 &&
     strcmp(c1->StyleFolderUnread,    c2->StyleFolderUnread) == 0 &&
     strcmp(c1->StyleFolderRead,      c2->StyleFolderRead) == 0 &&
     strcmp(c1->StyleFolderNew,       c2->StyleFolderNew) == 0 &&
     strcmp(c1->StyleMailUnread,      c2->StyleMailUnread) == 0 &&
     strcmp(c1->StyleMailRead,        c2->StyleMailRead) == 0 &&
     strcmp(c1->QuoteChar,            c2->QuoteChar) == 0 &&
     strcmp(c1->AltQuoteChar,         c2->AltQuoteChar) == 0 &&
     strcmp(c1->ThemeName,            c2->ThemeName) == 0 &&
     strcmp(c1->UpdateDownloadPath,   c2->UpdateDownloadPath) == 0 &&
     strcmp(c1->SpamExternalFilter,   c2->SpamExternalFilter) == 0 &&
     strcmp(c1->AttachmentKeywords,   c2->AttachmentKeywords) == 0)
  {
    equal = TRUE;
  }

  RETURN(equal);
  return equal;
}

///
/// SetDefaultConfig
// sets configuration (or a part of it) to the factory settings
void SetDefaultConfig(struct Config *co, enum ConfigPage page)
{
  char sysCodesetName[SIZE_CTYPE+1];

  ENTER();

  // get the system codeset's name
  if(G->systemCodeset != NULL)
    strlcpy(sysCodesetName, G->systemCodeset->name, sizeof(sysCodesetName));
  else
    strlcpy(sysCodesetName, "ISO-8859-1", sizeof(sysCodesetName));

  if(page == cp_AllPages)
  {
    // clear the complete configuration first, as there are certain dependencies
    // between some sections (i.e. First Steps and Identities) and clearing one
    // section after the other will leave some settings alive which should be
    // cleared
    ClearConfig(co);
  }

  if(page == cp_FirstSteps || page == cp_AllPages)
  {
    // check if the Location is setup correctly and if not
    // we use GuessTZone() to actually get an almost matching Location
    // definition or we set the Location to a default in the catalog
    if(G->Locale != NULL)
    {
      LONG gmtOffset = -(G->Locale->loc_GMTOffset);

      D(DBF_CONFIG, "got GMT offset %ld from locale.library", gmtOffset);

      strlcpy(co->Location, GuessTZone(gmtOffset), sizeof(co->Location));
    }
    else
      strlcpy(co->Location, tr(MSG_CO_FALLBACK_TZONE), sizeof(co->Location));

    strlcpy(co->DefaultLocalCodeset, sysCodesetName, sizeof(co->DefaultLocalCodeset));
  }

  if(page == cp_TCPIP || page == cp_AllPages)
  {
    // we have to free the pop3ServerList and smtpServerList
    FreeMailServerList(&co->pop3ServerList);
    FreeMailServerList(&co->smtpServerList);

    // fill the mailserver list with an empty POP3 and SMTP Server
    AddTail((struct List *)&co->smtpServerList, (struct Node *)CreateNewMailServer(MST_SMTP, co, TRUE));
    AddTail((struct List *)&co->pop3ServerList, (struct Node *)CreateNewMailServer(MST_POP3, co, TRUE));
  }

  if(page == cp_Identities || page == cp_AllPages)
  {
    // we have to free the userIdentityList
    FreeUserIdentityList(&co->userIdentityList);

    // fill the user identity list with an empty entry
    AddTail((struct List *)&co->userIdentityList, (struct Node *)CreateNewUserIdentity(co));
  }

  if(page == cp_Filters || page == cp_AllPages)
  {
    FreeFilterList(&co->filterList);
  }

  if(page == cp_Spam || page == cp_AllPages)
  {
    co->SpamFilterEnabled = TRUE;
    co->SpamFilterForNewMail = TRUE;
    co->SpamMarkOnMove = TRUE;
    co->SpamMarkAsRead = FALSE;
    co->SpamAddressBookIsWhiteList = TRUE;
    co->MoveHamToIncoming = TRUE;
    co->FilterHam = TRUE;
    co->SpamTrustExternalFilter = TRUE;
    strlcpy(co->SpamExternalFilter, "SpamAssassin", sizeof(co->SpamExternalFilter));
    co->SpamProbabilityThreshold = DEFAULT_SPAM_PROBABILITY_THRESHOLD;
    co->SpamFlushTrainingDataInterval = DEFAULT_FLUSH_TRAINING_DATA_INTERVAL;
    co->SpamFlushTrainingDataThreshold = DEFAULT_FLUSH_TRAINING_DATA_THRESHOLD;
  }

  if(page == cp_Read || page == cp_AllPages)
  {
    co->ShowHeader = 1;
    strlcpy(co->ShortHeaders, "(From|To|Cc|BCC|Date|Subject|Resent-#?)", sizeof(co->ShortHeaders));
    co->ShowSenderInfo = 2;
    strlcpy(co->ColoredText.buf, "m6", sizeof(co->ColoredText.buf));
    strlcpy(co->Color1stLevel.buf, "m0", sizeof(co->Color1stLevel.buf));
    strlcpy(co->Color2ndLevel.buf, "m7", sizeof(co->Color2ndLevel.buf));
    strlcpy(co->Color3rdLevel.buf, "m3", sizeof(co->Color3rdLevel.buf));
    strlcpy(co->Color4thLevel.buf, "m1", sizeof(co->Color4thLevel.buf));
    strlcpy(co->ColorURL.buf, "p6", sizeof(co->ColorURL.buf));
    strlcpy(co->ColorSignature.buf, "m4", sizeof(co->ColorSignature.buf));
    co->DisplayAllTexts = TRUE;
    co->FixedFontEdit = TRUE;
    co->UseTextStylesRead = TRUE;
    co->UseTextColorsRead = TRUE;
    co->DisplayAllAltPart = FALSE; // hide all sub "multipart/alternative" parts per default
    co->WrapHeader = FALSE;
    co->MultipleReadWindows = FALSE;
    co->SigSepLine = SST_BAR;
    co->StatusChangeDelayOn = TRUE;
    co->StatusChangeDelay = 1000; // 1s=1000ms delay by default
    co->ConvertHTML = TRUE;
    co->MDNEnabled = TRUE;
    co->MDN_NoRecipient = MDN_ACTION_ASK;
    co->MDN_NoDomain = MDN_ACTION_ASK;
    co->MDN_OnDelete = MDN_ACTION_ASK;
    co->MDN_Other = MDN_ACTION_ASK;
    co->DetectCyrillic = FALSE;
    co->MapForeignChars = TRUE;
    co->GlobalMailThreads = FALSE;
  }

  if(page == cp_Write || page == cp_AllPages)
  {
    strlcpy(co->NewIntro, tr(MSG_CO_NewIntroDef), sizeof(co->NewIntro));
    strlcpy(co->Greetings, tr(MSG_CO_GreetingsDef), sizeof(co->Greetings));
    co->WarnSubject = TRUE;
    co->AttachmentReminder = TRUE;
    strlcpy(co->AttachmentKeywords, tr(MSG_ATTACHMENT_KEYWORDS), sizeof(co->AttachmentKeywords));
    co->EdWrapCol = 78;
    co->EdWrapMode = EWM_EDITING;
    co->UseFixedFontWrite = TRUE;
    co->UseTextStylesWrite = TRUE;
    co->UseTextColorsWrite = TRUE;
    strlcpy(co->Editor, SYS_EDITOR, sizeof(co->Editor));
    co->LaunchAlways = FALSE;
    co->EmailCache = 10;
    co->AutoSave = 120;
    co->ShowRcptFieldCC = TRUE;
    co->ShowRcptFieldBCC = FALSE;
    co->ShowRcptFieldReplyTo = FALSE;
    strlcpy(co->DefaultWriteCodeset, sysCodesetName, sizeof(co->DefaultWriteCodeset));
  }

  if(page == cp_ReplyForward || page == cp_AllPages)
  {
    strlcpy(co->ReplyHello, "Hello %f\\n", sizeof(co->ReplyHello));
    strlcpy(co->ReplyIntro, "On %d, you wrote:\\n", sizeof(co->ReplyIntro));
    strlcpy(co->ReplyBye, "Regards", sizeof(co->ReplyBye));
    strlcpy(co->AltReplyHello, tr(MSG_CO_AltRepHelloDef), sizeof(co->AltReplyHello));
    strlcpy(co->AltReplyIntro, tr(MSG_CO_AltRepIntroDef), sizeof(co->AltReplyIntro));
    strlcpy(co->AltReplyBye, tr(MSG_CO_AltRepByeDef), sizeof(co->AltReplyBye));
    strlcpy(co->AltReplyPattern, tr(MSG_CO_AltRepPatternDef), sizeof(co->AltReplyPattern));
    strlcpy(co->MLReplyHello, tr(MSG_CO_MLRepHelloDef), sizeof(co->MLReplyHello));
    strlcpy(co->MLReplyIntro, tr(MSG_CO_MLRepIntroDef), sizeof(co->MLReplyIntro));
    strlcpy(co->MLReplyBye, tr(MSG_CO_MLRepByeDef), sizeof(co->MLReplyBye));
    strlcpy(co->ForwardIntro, tr(MSG_CO_ForwardIntroDef), sizeof(co->ForwardIntro));
    strlcpy(co->ForwardFinish, tr(MSG_CO_ForwardFinishDef), sizeof(co->ForwardFinish));
    strlcpy(co->QuoteChar, ">", sizeof(co->QuoteChar));
    strlcpy(co->AltQuoteChar, "|", sizeof(co->AltQuoteChar));

    co->QuoteEmptyLines = TRUE;
    co->CompareAddress = TRUE;
    co->StripSignature = TRUE;
    co->ForwardMode = FWM_ATTACH;
  }

  if(page == cp_Signature || page == cp_AllPages)
  {
    AddPath(co->TagsFile, G->ProgDir, ".taglines", sizeof(co->TagsFile));
    strlcpy(co->TagsSeparator, "%%", sizeof(co->TagsSeparator));

    // we have to free the signatureList
    FreeSignatureList(&co->signatureList);
  }

  if(page == cp_Security || page == cp_AllPages)
  {
    // we first try to see if there is a PGPPATH variable and if
    // so we take that one as the path to PGP or we plainly take PGP:
    // as the default path.
    if(GetVar("PGPPATH", co->PGPCmdPath, sizeof(co->PGPCmdPath), 0) == -1)
      strlcpy(co->PGPCmdPath, "PGP:", sizeof(co->PGPCmdPath));

    co->LogAllEvents = TRUE;
    co->PGPPassInterval = 10; // 10 min per default
    strlcpy(co->LogfilePath, G->ProgDir, sizeof(co->LogfilePath));
    co->LogfileMode = LF_NONE; // we log nothing per default
    co->SplitLogfile = FALSE;
  }

  if(page == cp_StartupQuit || page == cp_AllPages)
  {
    co->SendOnStartup = FALSE;
    co->LoadAllFolders = FALSE;
    co->SendOnQuit = FALSE;
    co->CleanupOnStartup = FALSE;
    co->RemoveOnStartup = FALSE;
    co->UpdateNewMail = TRUE;
    co->CheckBirthdates = TRUE;
    co->CleanupOnQuit = TRUE;
    co->RemoveOnQuit = TRUE;
    co->SaveLayoutOnQuit = TRUE;
  }

  if(page == cp_MIME || page == cp_AllPages)
  {
    FreeMimeTypeList(&co->mimeTypeList);
    strlcpy(co->DefaultMimeViewer, "SYS:Utilities/Multiview \"%s\"", sizeof(co->DefaultMimeViewer));
    strlcpy(co->DefaultMimeViewerCodesetName, co->DefaultLocalCodeset, sizeof(co->DefaultMimeViewerCodesetName));
  }

  if(page == cp_AddressBook || page == cp_AllPages)
  {
    AddPath(co->GalleryDir, G->ProgDir, "Resources/Gallery", sizeof(co->GalleryDir));
    strlcpy(co->NewAddrGroup, "NEW", sizeof(co->NewAddrGroup));
    co->AddToAddrbook = 0;
    co->AddrbookCols = (1<<0) | (1<<1) | (1<<2);
  }

  if(page == cp_Scripts || page == cp_AllPages)
  {
    int i;

    for(i = 0; i < MACRO_COUNT; i++)
    {
      co->RX[i].Name[0] = '\0';
      co->RX[i].Script[0] = '\0';
      co->RX[i].IsAmigaDOS = co->RX[i].UseConsole = FALSE;
      co->RX[i].WaitTerm = TRUE;
    }
  }

  if(page == cp_Mixed || page == cp_AllPages)
  {
    strlcpy(co->TempDir, "T:", sizeof(co->TempDir));
    strlcpy(co->DetachDir, "RAM:", sizeof(co->DetachDir));
    strlcpy(co->AttachDir, "RAM:", sizeof(co->AttachDir));
    strlcpy(co->PackerCommand, "LhA -a -m -i%l a \"%a.lha\"", sizeof(co->PackerCommand));
    co->ShowPackerProgress = FALSE;
    co->IconPositionX = -1; // < 0 means free positioning
    co->IconPositionY = -1; // < 0 means free positioning
    strlcpy(co->AppIconText, tr(MSG_CO_APPICON_LABEL), sizeof(co->AppIconText));
    co->IconifyOnQuit = co->RemoveAtOnce = FALSE;
    co->Confirm = TRUE;
    co->ConfirmDelete = 2;
    strlcpy(co->XPKPack, "HUFF", sizeof(co->XPKPack));
    strlcpy(co->XPKPackEncrypt, "HUFF", sizeof(co->XPKPackEncrypt));
    co->XPKPackEff = 50;
    co->XPKPackEncryptEff = 50;
    co->TransferWindow = TWM_AUTO;
    strlcpy(co->DefaultEditorCodeset, C->DefaultLocalCodeset, sizeof(co->DefaultEditorCodeset));

    // depending on the operating system we set the AppIcon
    // and docky icon defaults different
    #if defined(__amigaos4__)
    if(ApplicationBase != NULL)
    {
      co->DockyIcon = TRUE;
      co->WBAppIcon = FALSE;
    }
    else
    #endif
      co->WBAppIcon = TRUE;
  }

  if(page == cp_LookFeel || page == cp_AllPages)
  {
    strlcpy(co->ThemeName, "default", sizeof(co->ThemeName));
    co->InfoBarPos = IB_POS_CENTER;
    strlcpy(co->InfoBarText, tr(MSG_CO_InfoBarDef), sizeof(co->InfoBarText));
    co->QuickSearchBarPos = QSB_POS_TOP;
    co->EmbeddedReadPane = TRUE;
    co->SizeFormat = SF_MIXED;
    co->FolderCols = (FCOL_NAME | FCOL_TOTAL);
    co->MessageCols = (MCOL_STATUS | MCOL_SENDER | MCOL_SUBJECT | MCOL_DATE | MCOL_SIZE);
    co->FixedFontList = FALSE;
    co->DSListFormat = DSS_RELDATETIME;
    co->ABookLookup = FALSE;
    co->FolderCntMenu = TRUE;
    co->MessageCntMenu = TRUE;
    co->FolderInfoMode = FIM_NAME_AND_UNREAD_MAILS;
    co->FolderDoubleClick = TRUE;
  }

  if(page == cp_Update || page == cp_AllPages)
  {
    co->UpdateInterval = 604800; // check weekly for updates per default
    strlcpy(co->UpdateDownloadPath, "T:", sizeof(co->UpdateDownloadPath));
    SetDefaultUpdateState();
  }

  // everything else
  if(page == cp_AllPages)
  {
    co->LetterPart = 1;
    co->WriteIndexes = 120; // 2 minutes
    co->ExpungeIndexes = 600; // 10 minutes
    strlcpy(co->SupportSite, yamurl, sizeof(co->SupportSite));
    strlcpy(co->UpdateServer, "http://update.yam.ch/", sizeof(co->UpdateServer));
    co->JumpToIncoming = FALSE;
    co->AskJumpUnread = TRUE;
    co->PrinterCheck = TRUE;
    co->IsOnlineCheck = TRUE;
    co->ConfirmOnQuit = FALSE;
    co->HideGUIElements = 0;
    co->SysCharsetCheck = TRUE;
    co->AmiSSLCheck = TRUE;
    co->PrintMethod = PRINTMETHOD_RAW;
    co->StackSize = SIZE_STACK;
    co->AutoColumnResize = TRUE;
    co->SocketOptions.SendBuffer  = -1;
    co->SocketOptions.RecvBuffer  = -1;
    co->SocketOptions.SendLowAt   = -1;
    co->SocketOptions.RecvLowAt   = -1;
    co->SocketOptions.SendTimeOut = -1;
    co->SocketOptions.RecvTimeOut = -1;
    co->SocketOptions.KeepAlive   = FALSE;
    co->SocketOptions.NoDelay     = FALSE;
    co->SocketOptions.LowDelay    = FALSE;
    co->SocketTimeout = 30; // 30s socket timeout per default
    co->TRBufferSize = 8192; // 8K buffer per default
    co->EmbeddedMailDelay = 200; // 200ms delay per default
    co->KeepAliveInterval = 30;  // 30s interval per default
    co->AutoClip = FALSE;
    co->ShowFilterStats = TRUE;
    co->ConfirmRemoveAttachments = TRUE;

    // set the default styles of the folder listtree and
    // mail list items.
    strlcpy(co->StyleFGroupUnread, MUIX_B MUIX_I,         sizeof(co->StyleFGroupUnread));
    strlcpy(co->StyleFGroupRead,   MUIX_B MUIX_I "\0334", sizeof(co->StyleFGroupRead));
    strlcpy(co->StyleFolderUnread, MUIX_B        "\0334", sizeof(co->StyleFolderUnread));
    strlcpy(co->StyleFolderRead,   "",                    sizeof(co->StyleFolderRead));
    strlcpy(co->StyleFolderNew,    MUIX_B,                sizeof(co->StyleFolderNew));
    strlcpy(co->StyleMailUnread,   MUIX_B,                sizeof(co->StyleMailUnread));
    strlcpy(co->StyleMailRead,     "",                    sizeof(co->StyleMailRead));

    // check birthdays at 00:00 AM
    co->BirthdayCheckTime.ds_Days = 0;
    co->BirthdayCheckTime.ds_Minute = 0;
    co->BirthdayCheckTime.ds_Tick = 0;

    // default SSL ciphers to use
    strlcpy(co->DefaultSSLCiphers, "ALL:!LOW:!SSLv2:!EXP:!aNULL:@STRENGTH", sizeof(co->DefaultSSLCiphers));

    // default MachineFQDN is empty which means we try to identify it during runtime
    co->MachineFQDN[0] = '\0';
  }

  LEAVE();
}

///
/// String2MUIStyle
// converts a string with style definitions into a MUI style
// conform string with \033 sequences
static void String2MUIStyle(const char *string, char *muistr)
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
/// LoadConfig
// loads configuration from a file. return 1 on success, 0 on error and -1 if
// no (valid) config file found
int LoadConfig(struct Config *co, const char *fname, struct FolderList **oldfolders)
{
  int result = -1;
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
      int version = atoi(&buf[3]);
      struct FolderList *ofo = NULL;
      struct FilterNode *lastFilter = NULL;
      int lastTypeID = -1;
      struct MimeTypeNode *lastType = NULL;
      int lastFilterID = -1;
      BOOL foundGlobalPOP3Options = FALSE;
      int globalPOP3AvoidDuplicates = -1;
      int globalPOP3DownloadOnStartup = -1;
      int globalPOP3DownloadPeriodically = -1;
      int globalPOP3DownloadInterval = -1;
      int globalPOP3DownloadLargeMails = -1;
      int globalPOP3DownloadSizeLimit = -1;
      int globalPOP3DeleteOnServer = -1;
      int globalPOP3Preselection = -1;
      int globalPOP3NotifyType = -1;
      char globalPOP3NotifySound[SIZE_PATHFILE] = "";
      char globalPOP3NotifyCommand[SIZE_COMMAND] = "";

      // before we continue we actually make a version check
      // here so that in case the user tries to load
      // a config from a newer YAM version we remind him of
      // that and eventually go on
      if(version > LATEST_CFG_VERSION)
      {
        int res;
        Object *refWindow;

        // save a pointer to a reference window in case
        // we need to open a requester
        if(G->ConfigWinObject != NULL)
          refWindow = G->ConfigWinObject;
        else if(G->MA != NULL && G->MA->GUI.WI != NULL)
          refWindow = G->MA->GUI.WI;
        else
          refWindow = NULL;

        // ask the user how to proceed because we found out that he
        // tries to load a newer config file into this older YAM
        // version and thus he might end up losing some information.
        res = MUI_Request(G->App, refWindow, MUIF_NONE,
                          tr(MSG_CO_CONFIGVERSIONWARNING_TITLE),
                          tr(MSG_CO_CONFIGVERSIONWARNING_BT),
                          tr(MSG_CO_CONFIGVERSIONWARNING));

        if(res == 0)
        {
          fclose(fh);
          free(buf);

          RETURN(0);
          return 0;
        }
      }

      // set defaults and make the configuration actually
      // useall (reset it, e.g.)
      SetDefaultConfig(co, cp_AllPages);

      while(getline(&buf, &buflen, fh) > 0)
      {
        char *p;
        char *value;
        const char *value2 = "";

        // find the "=" separator
        if((value = strchr(buf, '=')) != NULL)
        {
          // now walk from there as long as we find a
          // whitespace
          for(value2 = (++value)+1; isspace(*value); value++)
            ;

          // now value should point to the start of the config
          // value (without any leading whitespace) and value2
          // just points to the position after "= " including
          // any whitespace. This might be required for config
          // options which might include white spaces right at
          // the start.
        }

        // search of a \r or \n and terminate the string there
        if((p = strpbrk(buf, "\n")) != NULL)
          *p = '\0';

        // search from the start of buf until = or a space and
        // the NUL terminate the string there early so that buf[]
        // only contain the config identifier
        for(p = buf; *p != '\0' && *p != '=' && !isspace(*p); p++)
          ;
        *p = '\0';

        // now we walk through our potential config options
        // an check if the name of it matches the one stored in buf
        //
        // NOTE: When adding/changing items here make sure to bump
        //       the LATEST_CFG_VERSION define so that older YAM
        //       versions are being warned that the config format
        //       has slightly changed.
        //       In addition, do NOT remove any items here but move
        //       them to the last OBSOLETE section down here so that
        //       in case an older config version is loaded we provide
        //       some kind of a backward compatibility.
        //
        if(*buf != '\0' && value != NULL)
        {
/* First Steps */
          if(stricmp(buf, "Location") == 0)          strlcpy(co->Location, value, sizeof(co->Location));
          else if(stricmp(buf, "LocalCharset") == 0) strlcpy(co->DefaultLocalCodeset, value, sizeof(co->DefaultLocalCodeset));

/* TCP/IP */
          else if(strnicmp(buf, "SMTP", 4) == 0 && isdigit(buf[4]) && isdigit(buf[5]) && strchr(buf, '.') != NULL)
          {
            int id = atoi(&buf[4]);

            if(id >= 0)
            {
              struct MailServerNode *msn;

              // try to get the SMTP server structure with the found id or create
              // a new one
              if((msn = GetMailServer(&co->smtpServerList, id)) == NULL)
              {
                if((msn = CreateNewMailServer(MST_SMTP, co, FALSE)) != NULL)
                  AddTail((struct List *)&co->smtpServerList, (struct Node *)msn);
                else
                  E(DBF_CONFIG, "Couldn't create new SMTP structure %ld", id);
              }

              if(msn != NULL)
              {
                char *q = strchr(buf, '.')+1;

                // now find out which subtype this smtp configuration is
                if(stricmp(q, "ID") == 0)                        msn->id = strtoul(value, NULL, 16);
                else if(stricmp(q, "Description") == 0)          strlcpy(msn->description, value, sizeof(msn->description));
                else if(stricmp(q, "Server") == 0)               strlcpy(msn->hostname, value, sizeof(msn->hostname));
                else if(stricmp(q, "Port") == 0)                 msn->port = atoi(value);
                else if(stricmp(q, "Enabled") == 0)              Txt2Bool(value) == TRUE ? setFlag(msn->flags, MSF_ACTIVE) : clearFlag(msn->flags, MSF_ACTIVE);
                else if(stricmp(q, "SecMethod") == 0)            setFlag(msn->flags, SMTPSecMethod2MSF(atoi(value)));
                else if(stricmp(q, "Allow8bit") == 0)            Txt2Bool(value) == TRUE ? setFlag(msn->flags, MSF_ALLOW_8BIT) : clearFlag(msn->flags, MSF_ALLOW_8BIT);
                else if(stricmp(q, "SMTP-AUTH") == 0)            Txt2Bool(value) == TRUE ? setFlag(msn->flags, MSF_AUTH) : clearFlag(msn->flags, MSF_AUTH);
                else if(stricmp(q, "AUTH-User") == 0)            strlcpy(msn->username, value, sizeof(msn->username));
                else if(stricmp(q, "AUTH-Pass") == 0)            strlcpy(msn->password, Decrypt(value), sizeof(msn->password));
                else if(stricmp(q, "AUTH-Method") == 0)          setFlag(msn->flags, SMTPAuthMethod2MSF(atoi(value)));
                else if(stricmp(q, "SSLCert") == 0)              strlcpy(msn->certFingerprint, value, sizeof(msn->certFingerprint));
                else if(stricmp(q, "SSLCertFailures") == 0)      msn->certFailures = atoi(value);
                else if(stricmp(q, "SentFolderID") == 0)         msn->mailStoreFolderID = strtoul(value, NULL, 16);
                else if(stricmp(q, "SentFolder") == 0)           strlcpy(msn->mailStoreFolderName, value, sizeof(msn->mailStoreFolderName));
                else
                  W(DBF_CONFIG, "unknown '%s' SMTP config tag", q);
              }
              else
                break;
            }
            else
              W(DBF_CONFIG, "SMTP id < 0 : %ld", id);
          }
          else if(strnicmp(buf, "POP", 3) == 0 && isdigit(buf[3]) && isdigit(buf[4]) && strchr(buf, '.') != NULL)
          {
            int id = atoi(&buf[3]);

            if(id >= 0)
            {
              struct MailServerNode *msn;

              // try to get the POP3 server structure with the found id or create
              // a new one
              if((msn = GetMailServer(&co->pop3ServerList, id)) == NULL)
              {
                if((msn = CreateNewMailServer(MST_POP3, co, FALSE)) != NULL)
                  AddTail((struct List *)&co->pop3ServerList, (struct Node *)msn);
                else
                  E(DBF_CONFIG, "Couldn't create new POP3 structure %ld", id);
              }

              if(msn != NULL)
              {
                char *q = strchr(buf, '.')+1;

                if(stricmp(q, "ID") == 0)                          msn->id = strtoul(value, NULL, 16);
                else if(stricmp(q, "Description") == 0)            strlcpy(msn->description, value, sizeof(msn->description));
                else if(stricmp(q, "Server") == 0)                 strlcpy(msn->hostname, value, sizeof(msn->hostname));
                else if(stricmp(q, "Port") == 0)                   msn->port = atoi(value);
                else if(stricmp(q, "Password") == 0)               strlcpy(msn->password, Decrypt(value), sizeof(msn->password));
                else if(stricmp(q, "User") == 0)                   strlcpy(msn->username, value, sizeof(msn->username));
                else if(stricmp(q, "Enabled") == 0)                Txt2Bool(value) == TRUE ? setFlag(msn->flags, MSF_ACTIVE) : clearFlag(msn->flags, MSF_ACTIVE);
                else if(stricmp(q, "SSLMode") == 0)                setFlag(msn->flags, POP3SecMethod2MSF(atoi(value)));
                else if(stricmp(q, "UseAPOP") == 0)                Txt2Bool(value) == TRUE ? setFlag(msn->flags, MSF_APOP) : clearFlag(msn->flags, MSF_APOP);
                else if(stricmp(q, "Delete") == 0)                 Txt2Bool(value) == TRUE ? setFlag(msn->flags, MSF_PURGEMESSGAES) : clearFlag(msn->flags, MSF_PURGEMESSGAES);
                else if(stricmp(q, "AvoidDuplicates") == 0)        Txt2Bool(value) == TRUE ? setFlag(msn->flags, MSF_AVOID_DUPLICATES) : clearFlag(msn->flags, MSF_AVOID_DUPLICATES);
                else if(stricmp(q, "ApplyRemoteFilters") == 0)     Txt2Bool(value) == TRUE ? setFlag(msn->flags, MSF_APPLY_REMOTE_FILTERS) : clearFlag(msn->flags, MSF_APPLY_REMOTE_FILTERS);
                else if(stricmp(q, "Preselection") == 0)           msn->preselection = atoi(value);
                else if(stricmp(q, "DownloadOnStartup") == 0)      Txt2Bool(value) == TRUE ? setFlag(msn->flags, MSF_DOWNLOAD_ON_STARTUP) : clearFlag(msn->flags, MSF_DOWNLOAD_ON_STARTUP);
                else if(stricmp(q, "DownloadPeriodically") == 0)   Txt2Bool(value) == TRUE ? setFlag(msn->flags, MSF_DOWNLOAD_PERIODICALLY) : clearFlag(msn->flags, MSF_DOWNLOAD_PERIODICALLY);
                else if(stricmp(q, "DownloadInterval") == 0)       msn->downloadInterval = atoi(value);
                else if(stricmp(q, "DownloadLargeMails") == 0)     Txt2Bool(value) == TRUE ? setFlag(msn->flags, MSF_DOWNLOAD_LARGE_MAILS) : clearFlag(msn->flags, MSF_DOWNLOAD_LARGE_MAILS);
                else if(stricmp(q, "DownloadLargeSizeLimit") == 0) msn->largeMailSizeLimit = atoi(value);
                else if(stricmp(q, "NotifyByRequester") == 0)      msn->notifyByRequester = Txt2Bool(value);
                else if(stricmp(q, "NotifyByOS41System") == 0)     msn->notifyByOS41System = Txt2Bool(value);
                else if(stricmp(q, "NotifyBySound") == 0)          msn->notifyBySound = Txt2Bool(value);
                else if(stricmp(q, "NotifyByCommand") == 0)        msn->notifyByCommand = Txt2Bool(value);
                else if(stricmp(q, "NotifySound") == 0)            strlcpy(msn->notifySound, value, sizeof(msn->notifySound));
                else if(stricmp(q, "NotifyCommand") == 0)          strlcpy(msn->notifyCommand, value, sizeof(msn->notifyCommand));
                else if(stricmp(q, "SSLCert") == 0)                strlcpy(msn->certFingerprint, value, sizeof(msn->certFingerprint));
                else if(stricmp(q, "SSLCertFailures") == 0)        msn->certFailures = atoi(value);
                else if(stricmp(q, "IncomingFolderID") == 0)       msn->mailStoreFolderID = strtoul(value, NULL, 16);
                else if(stricmp(q, "IncomingFolder") == 0)         strlcpy(msn->mailStoreFolderName, value, sizeof(msn->mailStoreFolderName));
                else
                  W(DBF_CONFIG, "unknown '%s' POP config tag", q);
              }
              else
                break;
            }
            else
              W(DBF_CONFIG, "POP3 id < 0 : %ld", id);
          }

/* Signature */
          else if(stricmp(buf, "TagsFile") == 0)                 strlcpy(co->TagsFile, value, sizeof(co->TagsFile));
          else if(stricmp(buf, "TagsSeparator") == 0)            strlcpy(co->TagsSeparator, value2, sizeof(co->TagsSeparator));
          else if(strnicmp(buf,"SIG", 3) == 0 && isdigit(buf[3]) && isdigit(buf[4]) && strchr(buf, '.') != NULL)
          {
            int num = atoi(&buf[3]);

            if(num >= 0)
            {
              struct SignatureNode *sn;

              // try to get the nth SignatureNode structure in our list or create a new one
              if((sn = GetSignature(&co->signatureList, num, FALSE)) == NULL)
              {
                if((sn = CreateNewSignature()) != NULL)
                  AddTail((struct List *)&co->signatureList, (struct Node *)sn);
                else
                  E(DBF_CONFIG, "Couldn't create new Signature structure %ld", num);
              }

              if(sn != NULL)
              {
                char *q = strchr(buf, '.')+1;

                // now find out which subtype this signature is
                if(stricmp(q, "ID") == 0)               sn->id = strtoul(value, NULL, 16);
                else if(stricmp(q, "Enabled") == 0)     sn->active = Txt2Bool(value);
                else if(stricmp(q, "Description") == 0) strlcpy(sn->description, value, sizeof(sn->description));
                else if(stricmp(q, "Filename") == 0)    CreateFilename(value, sn->filename, sizeof(sn->filename));
                else if(stricmp(q, "Signature") == 0)   sn->signature = ImportSignature(value2);
                else if(stricmp(q, "UseSigFile") == 0)  sn->useSignatureFile = Txt2Bool(value);
                else
                  W(DBF_CONFIG, "unknown '%s' SIG config tag", q);
              }
            }
          }

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
                if(stricmp(q, "ID") == 0)                        uin->id = strtoul(value, NULL, 16);
                else if(stricmp(q, "Enabled") == 0)              uin->active = Txt2Bool(value);
                else if(stricmp(q, "Description") == 0)          strlcpy(uin->description, value, sizeof(uin->description));
                else if(stricmp(q, "Realname") == 0)             strlcpy(uin->realname, value, sizeof(uin->realname));
                else if(stricmp(q, "Address") == 0)              strlcpy(uin->address, value, sizeof(uin->address));
                else if(stricmp(q, "Organization") == 0)         strlcpy(uin->organization, value, sizeof(uin->organization));
                else if(stricmp(q, "MailServerID") == 0)         uin->smtpServer = FindMailServer(&co->smtpServerList, strtoul(value, NULL, 16));
                else if(stricmp(q, "SignatureID") == 0)          uin->signature = FindSignatureByID(&co->signatureList, strtoul(value, NULL, 16));
                else if(stricmp(q, "MailCC") == 0)               strlcpy(uin->mailCC, value, sizeof(uin->mailCC));
                else if(stricmp(q, "MailBCC") == 0)              strlcpy(uin->mailBCC, value, sizeof(uin->mailBCC));
                else if(stricmp(q, "MailReplyTo") == 0)          strlcpy(uin->mailReplyTo, value, sizeof(uin->mailReplyTo));
                else if(stricmp(q, "ExtraHeaders") == 0)         strlcpy(uin->extraHeaders, value, sizeof(uin->extraHeaders));
                else if(stricmp(q, "PhotoURL") == 0)             strlcpy(uin->photoURL, value, sizeof(uin->photoURL));
              	else if(stricmp(q, "SentFolder") == 0)           strlcpy(uin->sentFolderName, value, sizeof(uin->sentFolderName));
                else if(stricmp(q, "SentFolderID") == 0)         uin->sentFolderID = strtoul(value, NULL, 16);
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
                struct FilterNode *filter;

                // reset the lastFilter
                lastFilter = NULL;
                lastFilterID = -1;

                // try to get the filter with that particular filter ID out of our
                // filterList
                i = 0;
                IterateList(&co->filterList, struct FilterNode *, filter)
                {
                  if(i == curFilterID)
                  {
                    lastFilter = filter;
                    lastFilterID = i;
                    break;
                  }

                  i++;
                }
              }

              if(lastFilter == NULL)
              {
                if((lastFilter = CreateNewFilter(0, SEARCHF_DOS_PATTERN)))
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
              else if(stricmp(q, "Combine") == 0)                lastFilter->combine = atoi(value);
              else if(stricmp(q, "RedirectTo") == 0 || stricmp(q, "BounceTo") == 0)
                strlcpy(lastFilter->redirectTo, value, sizeof(lastFilter->redirectTo));
              else if(stricmp(q, "ForwardTo") == 0)              strlcpy(lastFilter->forwardTo, value, sizeof(lastFilter->forwardTo));
              else if(stricmp(q, "ReplyFile") == 0)              strlcpy(lastFilter->replyFile, value, sizeof(lastFilter->replyFile));
              else if(stricmp(q, "ExecuteCmd") == 0)             strlcpy(lastFilter->executeCmd, value, sizeof(lastFilter->executeCmd));
              else if(stricmp(q, "PlaySound") == 0)              strlcpy(lastFilter->playSound, value, sizeof(lastFilter->playSound));
              else if(stricmp(q, "MoveTo") == 0)                 strlcpy(lastFilter->moveToName, value, sizeof(lastFilter->moveToName));
              else if(stricmp(q, "MoveToID") == 0)               lastFilter->moveToID = strtoul(value, NULL, 16);
              else
              {
                struct RuleNode *rule;

                // if nothing of the above string matched than the FI string
                // is probably a rule definition so we check it here

                // Up to YAM 2.8p1 (version 5) the subfields were named "Field","Field2",Field3","Field4",...
                // Since YAM 2.9 (version 6) these are named "Field0","Field1","Field2",Field3",...
                // Hence we must adapt the numbering for configurations from older releases.
                if(strnicmp(q, "Field", 5) == 0)
                {
                  int n = (q[5] == '\0') ? 1 : atoi(q+5);

                  if(version < 6)
                    n--;

                  while((rule = GetFilterRule(lastFilter, n)) == NULL)
                    CreateNewRule(lastFilter, SEARCHF_DOS_PATTERN);

                  rule->searchMode = atoi(value);
                }
                else if(strnicmp(q, "SubField", 8) == 0)
                {
                  int n = (q[8] == '\0') ? 1 : atoi(q+8);

                  if(version < 6)
                    n--;

                  while((rule = GetFilterRule(lastFilter, n)) == NULL)
                    CreateNewRule(lastFilter, SEARCHF_DOS_PATTERN);

                  rule->subSearchMode = atoi(value);
                }
                else if(strnicmp(q, "CustomField", 11) == 0)
                {
                  int n = (q[11] == '\0') ? 1 : atoi(q+11);

                  if(version < 6)
                    n--;

                  while((rule = GetFilterRule(lastFilter, n)) == NULL)
                    CreateNewRule(lastFilter, SEARCHF_DOS_PATTERN);

                  strlcpy(rule->customField, value, sizeof(rule->customField));
                }
                else if(strnicmp(q, "Comparison", 10) == 0)
                {
                  int n = (q[10] == '\0') ? 1 : atoi(q+10);

                  if(version < 6)
                    n--;

                  while((rule = GetFilterRule(lastFilter, n)) == NULL)
                    CreateNewRule(lastFilter, SEARCHF_DOS_PATTERN);

                  rule->comparison = atoi(value);
                }
                else if(strnicmp(q, "Match", 5) == 0)
                {
                  int n = (q[5] == '\0') ? 1 : atoi(q+5);

                  if(version < 6)
                    n--;

                  while((rule = GetFilterRule(lastFilter, n)) == NULL)
                    CreateNewRule(lastFilter, SEARCHF_DOS_PATTERN);

                  strlcpy(rule->matchPattern, value2, sizeof(rule->matchPattern));
                }
                else if(strnicmp(q, "CaseSens", 8) == 0)
                {
                  int n = (q[8] == '\0') ? 1 : atoi(q+8);

                  if(version < 6)
                    n--;

                  while((rule = GetFilterRule(lastFilter, n)) == NULL)
                    CreateNewRule(lastFilter, SEARCHF_DOS_PATTERN);

                  if(Txt2Bool(value) == TRUE)
                    setFlag(rule->flags, SEARCHF_CASE_SENSITIVE);
                  else
                    clearFlag(rule->flags, SEARCHF_CASE_SENSITIVE);
                }
                else if(strnicmp(q, "Substring", 9) == 0)
                {
                  int n = (q[9] == '\0') ? 1 : atoi(q+9);

                  if(version < 6)
                    n--;

                  while((rule = GetFilterRule(lastFilter, n)) == NULL)
                    CreateNewRule(lastFilter, SEARCHF_DOS_PATTERN);

                  if(Txt2Bool(value) == TRUE)
                    setFlag(rule->flags, SEARCHF_SUBSTRING);
                  else
                    clearFlag(rule->flags, SEARCHF_SUBSTRING);
                }
                else if(strnicmp(q, "DOSPattern", 10) == 0)
                {
                  int n = (q[10] == '\0') ? 1 : atoi(q+10);

                  if(version < 6)
                    n--;

                  while((rule = GetFilterRule(lastFilter, n)) == NULL)
                    CreateNewRule(lastFilter, SEARCHF_DOS_PATTERN);

                  if(Txt2Bool(value) == TRUE)
                    setFlag(rule->flags, SEARCHF_DOS_PATTERN);
                  else
                    clearFlag(rule->flags, SEARCHF_DOS_PATTERN);
                }
                else if(strnicmp(q, "SkipEncrypted", 13) == 0)
                {
                  int n = (q[10] == '\0') ? 1 : atoi(q+10);

                  if(version < 6)
                    n--;

                  while((rule = GetFilterRule(lastFilter, n)) == NULL)
                    CreateNewRule(lastFilter, SEARCHF_DOS_PATTERN);

                  if(Txt2Bool(value) == TRUE)
                    setFlag(rule->flags, SEARCHF_SKIP_ENCRYPTED);
                  else
                    clearFlag(rule->flags, SEARCHF_SKIP_ENCRYPTED);
                }
                else if(strnicmp(q, "Combine", 7) == 0)
                {
                  // this is an old per-rule combine value
                  // we just propagate this to the filter
                  switch(atoi(value))
                  {
                    case 1: lastFilter->combine = CB_AT_LEAST_ONE; break;
                    default:
                    case 2: lastFilter->combine = CB_ALL; break;
                    case 3: lastFilter->combine = CB_EXACTLY_ONE; break;
                  }
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
          else if(stricmp(buf, "TrustExternalFilter") == 0)      co->SpamTrustExternalFilter = Txt2Bool(value);
          else if(stricmp(buf, "ExternalFilter") == 0)           strlcpy(co->SpamExternalFilter, value, sizeof(co->SpamExternalFilter));

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
          else if(stricmp(buf, "DetectCyrillic") == 0)           co->DetectCyrillic = Txt2Bool(value);
          else if(stricmp(buf, "MapForeignChars") == 0)          co->MapForeignChars = Txt2Bool(value);
          else if(stricmp(buf, "GlobalMailThreads") == 0)        co->GlobalMailThreads = Txt2Bool(value);

/* Write */
          else if(stricmp(buf, "NewIntro") == 0)                 strlcpy(co->NewIntro, value2, sizeof(co->NewIntro));
          else if(stricmp(buf, "Greetings") == 0)                strlcpy(co->Greetings, value2, sizeof(co->Greetings));
          else if(stricmp(buf, "WarnSubject") == 0)              co->WarnSubject = Txt2Bool(value);
          else if(stricmp(buf, "AttachmentReminder") == 0)       co->AttachmentReminder = Txt2Bool(value);
          else if(stricmp(buf, "AttachmentKeywords") == 0)       strlcpy(co->AttachmentKeywords, value2, sizeof(co->AttachmentKeywords));
          else if(stricmp(buf, "EdWrapCol") == 0)                co->EdWrapCol = atoi(value);
          else if(stricmp(buf, "EdWrapMode") == 0)               co->EdWrapMode = atoi(value);
          else if(stricmp(buf, "LaunchAlways") == 0)             co->LaunchAlways = Txt2Bool(value);
          else if(stricmp(buf, "EmailCache") == 0)               co->EmailCache = atoi(value);
          else if(stricmp(buf, "AutoSave") == 0)                 co->AutoSave = atoi(value);
          else if(stricmp(buf, "WriteCharset") == 0)             strlcpy(co->DefaultWriteCodeset, value, sizeof(co->DefaultWriteCodeset));
          else if(stricmp(buf, "FixedFontWrite") == 0)           co->UseFixedFontWrite = Txt2Bool(value);
          else if(stricmp(buf, "TextStylesWrite") == 0)          co->UseTextStylesWrite = Txt2Bool(value);
          else if(stricmp(buf, "TextColorsWrite") == 0)          co->UseTextColorsWrite = Txt2Bool(value);
          else if(stricmp(buf, "ShowRcptFieldCC") == 0)          co->ShowRcptFieldCC = Txt2Bool(value);
          else if(stricmp(buf, "ShowRcptFieldBCC") == 0)         co->ShowRcptFieldBCC = Txt2Bool(value);
          else if(stricmp(buf, "ShowRcptFieldReplyTo") == 0)     co->ShowRcptFieldReplyTo = Txt2Bool(value);

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
          else if(stricmp(buf, "SendOnStartup") == 0)            co->SendOnStartup = Txt2Bool(value);
          else if(stricmp(buf, "CleanupOnStartup") == 0)         co->CleanupOnStartup = Txt2Bool(value);
          else if(stricmp(buf, "RemoveOnStartup") == 0)          co->RemoveOnStartup = Txt2Bool(value);
          else if(stricmp(buf, "LoadAllFolders") == 0)           co->LoadAllFolders = Txt2Bool(value);
          else if(stricmp(buf, "UpdateNewMail") == 0)            co->UpdateNewMail = Txt2Bool(value);
          else if(stricmp(buf, "CheckBirthdates") == 0)          co->CheckBirthdates = Txt2Bool(value);
          else if(stricmp(buf, "SendOnQuit") == 0)               co->SendOnQuit = Txt2Bool(value);
          else if(stricmp(buf, "CleanupOnQuit") == 0)            co->CleanupOnQuit = Txt2Bool(value);
          else if(stricmp(buf, "RemoveOnQuit") == 0)             co->RemoveOnQuit = Txt2Bool(value);
          else if(stricmp(buf, "SaveLayoutOnQuit") == 0)         co->SaveLayoutOnQuit = Txt2Bool(value);

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
                struct MimeTypeNode *mime;

                // reset the lastType
                lastType = NULL;
                lastTypeID = -1;

                // try to get the mimeType with that particular filter ID out of our
                // filterList
                i = 0;
                IterateList(&co->mimeTypeList, struct MimeTypeNode *, mime)
                {
                  if(i == curTypeID)
                  {
                    lastType = mime;
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
              else if(!stricmp(q, "CharsetName"))
                strlcpy(lastType->CodesetName, value, sizeof(lastType->CodesetName));
            }
            else
            {
              if(!stricmp(q, "Command"))
                strlcpy(C->DefaultMimeViewer, value, sizeof(C->DefaultMimeViewer));
              else if(!stricmp(q, "CharsetName"))
                strlcpy(C->DefaultMimeViewerCodesetName, value, sizeof(C->DefaultMimeViewerCodesetName));
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

            if(j >= 0 && j < MACRO_COUNT)
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
          else if(stricmp(buf, "ShowPackerProgress") == 0)       co->ShowPackerProgress = Txt2Bool(value);
          else if(stricmp(buf, "TransferWindow") == 0)           co->TransferWindow = atoi(value);
          else if(stricmp(buf, "Editor") == 0)                   strlcpy(co->Editor, value, sizeof(co->Editor));
          else if(stricmp(buf, "EditorCharset") == 0)            strlcpy(co->DefaultEditorCodeset, value, sizeof(co->DefaultEditorCodeset));

/* Look&Feel */
          else if(stricmp(buf, "Theme") == 0)                    strlcpy(co->ThemeName, value, sizeof(co->ThemeName));
          else if(stricmp(buf, "InfoBarPos") == 0)               co->InfoBarPos = atoi(value);
          else if(stricmp(buf, "InfoBarText") == 0)              strlcpy(co->InfoBarText, value, sizeof(co->InfoBarText));
          else if(stricmp(buf, "QuickSearchBarPos") == 0)        co->QuickSearchBarPos = atoi(value);
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
          else if(stricmp(buf, "JumpToIncoming") == 0)           co->JumpToIncoming = Txt2Bool(value);
          else if(stricmp(buf, "AskJumpUnread") == 0)            co->AskJumpUnread = Txt2Bool(value);
          else if(stricmp(buf, "PrinterCheck") == 0)             co->PrinterCheck = Txt2Bool(value);
          else if(stricmp(buf, "IsOnlineCheck") == 0)            co->IsOnlineCheck = Txt2Bool(value);
          else if(stricmp(buf, "IOCInterface") == 0)             strlcpy(co->IOCInterfaces, value, sizeof(co->IOCInterfaces));
          else if(stricmp(buf, "ConfirmOnQuit") == 0)            co->ConfirmOnQuit = Txt2Bool(value);
          else if(stricmp(buf, "HideGUIElements") == 0)          co->HideGUIElements = atoi(value);
          else if(stricmp(buf, "SysCharsetCheck") == 0)          co->SysCharsetCheck = Txt2Bool(value);
          else if(stricmp(buf, "AmiSSLCheck") == 0)              co->AmiSSLCheck = Txt2Bool(value);
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
          else if(stricmp(buf, "DefaultSSLCiphers") == 0)        strlcpy(co->DefaultSSLCiphers, value, sizeof(co->DefaultSSLCiphers));
          else if(stricmp(buf, "MachineFQDN") == 0)              strlcpy(co->MachineFQDN, value, sizeof(co->MachineFQDN));
          else if(stricmp(buf, "OverrideFromAddress") == 0)      co->OverrideFromAddress = Txt2Bool(value);

/* Obsolete options (previous YAM version write them, we just read them) */
          else if(version < LATEST_CFG_VERSION)
          {
            struct MailServerNode *fPOP3;
            struct MailServerNode *fSMTP;
            struct UserIdentityNode *fUserIdentity;

            // get the first POP3/SMTP server out of our config data
            fPOP3 = GetMailServer(&co->pop3ServerList, 0);
            fSMTP = GetMailServer(&co->smtpServerList, 0);
            ASSERT(fPOP3 != NULL);
            ASSERT(fSMTP != NULL);

            // get the first User Identity out of our config data
            fUserIdentity = GetUserIdentity(&co->userIdentityList, 0, TRUE);
            ASSERT(fUserIdentity != NULL);

            if(stricmp(buf, "RealName") == 0)                      strlcpy(fUserIdentity->realname, value, sizeof(fUserIdentity->realname));
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
            else if(stricmp(buf, "UseSignature") == 0)             fUserIdentity->signature = (Txt2Bool(value) == TRUE ? GetSignature(&co->signatureList, 0, TRUE) : NULL);
            else if(stricmp(buf, "SMTP-ID") == 0)                  fSMTP->id = strtoul(value, NULL, 16);
            else if(stricmp(buf, "SMTP-Enabled") == 0)             Txt2Bool(value) == TRUE ? setFlag(fSMTP->flags, MSF_ACTIVE) : clearFlag(fSMTP->flags, MSF_ACTIVE);
            else if(stricmp(buf, "SMTP-Description") == 0)         strlcpy(fSMTP->description, value, sizeof(fSMTP->description));
            else if(stricmp(buf, "SMTP-Server") == 0)              strlcpy(fSMTP->hostname, value, sizeof(fSMTP->hostname));
            else if(stricmp(buf, "SMTP-Port") == 0)                fSMTP->port = atoi(value);
            else if(stricmp(buf, "SMTP-SecMethod") == 0)           setFlag(fSMTP->flags, SMTPSecMethod2MSF(atoi(value)));
            else if(stricmp(buf, "Allow8bit") == 0)                Txt2Bool(value) == TRUE ? setFlag(fSMTP->flags, MSF_ALLOW_8BIT) : clearFlag(fSMTP->flags, MSF_ALLOW_8BIT);
            else if(stricmp(buf, "Use-SMTP-TLS") == 0)             setFlag(fSMTP->flags, SMTPSecMethod2MSF(atoi(value)));
            else if(stricmp(buf, "Use-SMTP-AUTH") == 0)            Txt2Bool(value) == TRUE ? setFlag(fSMTP->flags, MSF_AUTH) : clearFlag(fSMTP->flags, MSF_AUTH);
            else if(stricmp(buf, "SMTP-AUTH-User") == 0)           strlcpy(fSMTP->username, value, sizeof(fSMTP->username));
            else if(stricmp(buf, "SMTP-AUTH-Pass") == 0)           strlcpy(fSMTP->password, Decrypt(value), sizeof(fSMTP->password));
            else if(stricmp(buf, "SMTP-AUTH-Method") == 0)         setFlag(fSMTP->flags, SMTPAuthMethod2MSF(atoi(value)));
            else if(stricmp(buf, "POP3-Server") == 0)              strlcpy(fPOP3->hostname, value, sizeof(fPOP3->hostname));
            else if(stricmp(buf, "POP3-Password") == 0)            strlcpy(fPOP3->password, Decrypt(value), sizeof(fPOP3->password));
            else if(stricmp(buf, "POP3-User") == 0)                strlcpy(fPOP3->username, value, sizeof(fPOP3->username));
            else if(stricmp(buf, "AvoidDuplicates") == 0)          { globalPOP3AvoidDuplicates = Txt2Bool(value); foundGlobalPOP3Options = TRUE; }
            else if(stricmp(buf, "PreSelection") == 0)             { globalPOP3Preselection = atoi(value); foundGlobalPOP3Options = TRUE; }
            else if(stricmp(buf, "GetOnStartup") == 0)             { globalPOP3DownloadOnStartup = Txt2Bool(value); foundGlobalPOP3Options = TRUE; }
            else if(stricmp(buf, "DeleteOnServer") == 0)           { globalPOP3DeleteOnServer = Txt2Bool(value); foundGlobalPOP3Options = TRUE; }
            else if(stricmp(buf, "WarnSize") == 0)                 { globalPOP3DownloadSizeLimit = atoi(value); foundGlobalPOP3Options = TRUE; }
            else if(stricmp(buf, "CheckMail") == 0)                { globalPOP3DownloadPeriodically = Txt2Bool(value); foundGlobalPOP3Options = TRUE; }
            else if(stricmp(buf, "CheckMailDelay") == 0)           { globalPOP3DownloadInterval = atoi(value); foundGlobalPOP3Options = TRUE; }
            else if(stricmp(buf, "DownloadLarge") == 0)            { globalPOP3DownloadLargeMails = Txt2Bool(value); foundGlobalPOP3Options = TRUE; }
            else if(stricmp(buf, "Verbosity") == 0)                co->TransferWindow = atoi(value) > 0 ? TWM_SHOW : TWM_HIDE;
            else if(stricmp(buf, "WordWrap") == 0)                 co->EdWrapCol = atoi(value);
            else if(stricmp(buf, "DeleteOnExit") == 0)             co->RemoveAtOnce = !(co->RemoveOnQuit = Txt2Bool(value));
            else if(stricmp(buf, "NotifyType") == 0)               { globalPOP3NotifyType = atoi(value); foundGlobalPOP3Options = TRUE; }
            else if(stricmp(buf, "NotifySound") == 0)              { strlcpy(globalPOP3NotifySound, value, sizeof(globalPOP3NotifySound)); foundGlobalPOP3Options = TRUE; }
            else if(stricmp(buf, "NotifyCommand") == 0)            { strlcpy(globalPOP3NotifyCommand, value, sizeof(globalPOP3NotifyCommand)); foundGlobalPOP3Options = TRUE; }
            else if(stricmp(buf, "QuickSearchBar") == 0)           co->QuickSearchBarPos = Txt2Bool(value) ? QSB_POS_TOP : QSB_POS_OFF;
            else if(stricmp(buf, "InfoBar") == 0)                  { int v = atoi(value)+1; co->InfoBarPos = (v == 3) ? IB_POS_OFF : v; }
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

              if((filter = CreateNewFilter(0, SEARCHF_DOS_PATTERN)) != NULL)
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
                  setFlag(rule->flags, SEARCHF_CASE_SENSITIVE);
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
                strlcpy(filter->moveToName, p2, sizeof(filter->moveToName));
                strlcpy(filter->forwardTo, p, sizeof(filter->forwardTo));
                if(*filter->forwardTo != '\0')
                  setFlag(filter->actions, FA_FORWARD);

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
            else if(strnicmp(buf, "FolderPath", 10) == 0 && oldfolders != NULL)
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
            else
              W(DBF_CONFIG, "unknown OLD config option: '%s' = '%s'", buf, value);
          }
          else
            W(DBF_CONFIG, "unknown config option: '%s' = '%s'", buf, value);
        }
      }

      // we have to check if something went
      // wrong while loading the config
      if(feof(fh) != 0 && ferror(fh) == 0)
      {
        result = 1;

        // mark the configuration as "saved" as we have loaded
        // it freshly
        co->ConfigIsSaved = TRUE;

        D(DBF_CONFIG, "configuration successfully loaded");

        if(foundGlobalPOP3Options == TRUE)
        {
          // Propagate the old global POP3 options to each POP3 account.
          // The old options will vanish as soon as the configuration is saved, thus this
          // will happen only once.
          struct MailServerNode *msn;

          IterateList(&co->pop3ServerList, struct MailServerNode *, msn)
          {
            // apply only the found old settings and exclude the non-found ones
            if(globalPOP3AvoidDuplicates != -1)
            {
              if(globalPOP3AvoidDuplicates == TRUE)
                setFlag(msn->flags, MSF_AVOID_DUPLICATES);
              else
                clearFlag(msn->flags, MSF_AVOID_DUPLICATES);
            }

            if(globalPOP3DownloadOnStartup != -1)
            {
              if(globalPOP3DownloadOnStartup == TRUE)
                setFlag(msn->flags, MSF_DOWNLOAD_ON_STARTUP);
              else
                clearFlag(msn->flags, MSF_DOWNLOAD_ON_STARTUP);
            }

            if(globalPOP3DownloadPeriodically != -1)
            {
              if(globalPOP3DownloadPeriodically == TRUE)
                setFlag(msn->flags, MSF_DOWNLOAD_PERIODICALLY);
              else
                clearFlag(msn->flags, MSF_DOWNLOAD_PERIODICALLY);
            }

            if(globalPOP3DownloadInterval != -1)
            {
              msn->downloadInterval = globalPOP3DownloadInterval;
            }

            if(globalPOP3DownloadLargeMails != -1)
            {
              if(globalPOP3DownloadLargeMails == TRUE)
                setFlag(msn->flags, MSF_DOWNLOAD_LARGE_MAILS);
              else
                clearFlag(msn->flags, MSF_DOWNLOAD_LARGE_MAILS);
            }

            if(globalPOP3DownloadSizeLimit != -1)
            {
              msn->largeMailSizeLimit = globalPOP3DownloadSizeLimit;
            }

            if(globalPOP3DeleteOnServer != -1)
            {
              if(globalPOP3DeleteOnServer == TRUE)
                setFlag(msn->flags, MSF_PURGEMESSGAES);
              else
                clearFlag(msn->flags, MSF_PURGEMESSGAES);
            }

            if(globalPOP3Preselection != -1)
              msn->preselection = globalPOP3Preselection;

            if(globalPOP3NotifyType != -1)
            {
              msn->notifyByRequester = isFlagSet(globalPOP3NotifyType, (1<<0));
              msn->notifyByOS41System = isFlagSet(globalPOP3NotifyType, (1<<1));
              msn->notifyBySound = isFlagSet(globalPOP3NotifyType, (1<<2));
              msn->notifyByCommand = isFlagSet(globalPOP3NotifyType, (1<<3));
            }
            strlcpy(msn->notifySound, globalPOP3NotifySound, sizeof(msn->notifySound));
            strlcpy(msn->notifyCommand, globalPOP3NotifyCommand, sizeof(msn->notifyCommand));
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

  RETURN(result);
  return result;
}

///
/// MUIStyle2String
// converts a MUI style string which contains common \033 sequences into a
// human-readable form which we can save to our configuration file.
static char *MUIStyle2String(const char *style)
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
/// SaveConfig
// saves configuration to a file
BOOL SaveConfig(struct Config *co, const char *fname)
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
    struct MailServerNode *msn;
    struct SignatureNode *sn;
    struct UserIdentityNode *uin;
    struct FilterNode *filter;
    struct MimeTypeNode *mtNode;

    setvbuf(fh, NULL, _IOFBF, SIZE_FILEBUF);

    // now we write out ALL config options after another
    //
    // NOTE: When adding/changing items here make sure to bump
    //       the LATEST_CFG_VERSION define so that older YAM
    //       versions are being warned that the config format
    //       has slightly changed.
    //       In addition, if your remove items here make sure
    //       to not only bump the config version but also make
    //       sure to keep that option in the LoadConfig()
    //       function in the OBSOLETE section.

    fprintf(fh, "YCO%d - YAM Configuration\n", LATEST_CFG_VERSION);
    fprintf(fh, "# generated by '%s (%s)'\n", yamversion, yamversiondate);

    fprintf(fh, "\n[First steps]\n");
    fprintf(fh, "Location     = %s\n", co->Location);
    fprintf(fh, "LocalCharset = %s\n", co->DefaultLocalCodeset);

    fprintf(fh, "\n[TCP/IP]\n");

    // we iterate through our mail server list and ouput the SMTP servers in it
    i = 0;
    IterateList(&co->smtpServerList, struct MailServerNode *, msn)
    {
      fprintf(fh, "SMTP%02d.ID                    = %08x\n", i, msn->id);
      fprintf(fh, "SMTP%02d.Enabled               = %s\n", i, Bool2Txt(isServerActive(msn)));
      fprintf(fh, "SMTP%02d.Description           = %s\n", i, msn->description);
      fprintf(fh, "SMTP%02d.Server                = %s\n", i, msn->hostname);
      fprintf(fh, "SMTP%02d.Port                  = %d\n", i, msn->port);
      fprintf(fh, "SMTP%02d.SecMethod             = %d\n", i, MSF2SMTPSecMethod(msn));
      fprintf(fh, "SMTP%02d.Allow8bit             = %s\n", i, Bool2Txt(hasServer8bit(msn)));
      fprintf(fh, "SMTP%02d.SMTP-AUTH             = %s\n", i, Bool2Txt(hasServerAuth(msn)));
      fprintf(fh, "SMTP%02d.AUTH-User             = %s\n", i, msn->username);
      fprintf(fh, "SMTP%02d.AUTH-Pass             = %s\n", i, Encrypt(msn->password));
      fprintf(fh, "SMTP%02d.AUTH-Method           = %d\n", i, MSF2SMTPAuthMethod(msn));
      fprintf(fh, "SMTP%02d.SSLCert               = %s\n", i, msn->certFingerprint);
      fprintf(fh, "SMTP%02d.SSLCertFailures       = %d\n", i, msn->certFailures);
      fprintf(fh, "SMTP%02d.SentFolderID          = %08x\n", i, msn->mailStoreFolderID);

      i++;
    }

    // we iterate through our mail server list and ouput the POP3 servers in it
    i = 0;
    IterateList(&co->pop3ServerList, struct MailServerNode *, msn)
    {
      fprintf(fh, "POP%02d.ID                     = %08x\n", i, msn->id);
      fprintf(fh, "POP%02d.Enabled                = %s\n", i, Bool2Txt(isServerActive(msn)));
      fprintf(fh, "POP%02d.Description            = %s\n", i, msn->description);
      fprintf(fh, "POP%02d.Server                 = %s\n", i, msn->hostname);
      fprintf(fh, "POP%02d.Port                   = %d\n", i, msn->port);
      fprintf(fh, "POP%02d.User                   = %s\n", i, msn->username);
      fprintf(fh, "POP%02d.Password               = %s\n", i, Encrypt(msn->password));
      fprintf(fh, "POP%02d.SSLMode                = %d\n", i, MSF2POP3SecMethod(msn));
      fprintf(fh, "POP%02d.UseAPOP                = %s\n", i, Bool2Txt(hasServerAPOP(msn)));
      fprintf(fh, "POP%02d.Delete                 = %s\n", i, Bool2Txt(hasServerPurge(msn)));
      fprintf(fh, "POP%02d.AvoidDuplicates        = %s\n", i, Bool2Txt(hasServerAvoidDuplicates(msn)));
      fprintf(fh, "POP%02d.ApplyRemoteFilters     = %s\n", i, Bool2Txt(hasServerApplyRemoteFilters(msn)));
      fprintf(fh, "POP%02d.Preselection           = %d\n", i, msn->preselection);
      fprintf(fh, "POP%02d.DownloadOnStartup      = %s\n", i, Bool2Txt(hasServerDownloadOnStartup(msn)));
      fprintf(fh, "POP%02d.DownloadPeriodically   = %s\n", i, Bool2Txt(hasServerDownloadPeriodically(msn)));
      fprintf(fh, "POP%02d.DownloadInterval       = %d\n", i, msn->downloadInterval);
      fprintf(fh, "POP%02d.DownloadLargeMails     = %s\n", i, Bool2Txt(hasServerDownloadLargeMails(msn)));
      fprintf(fh, "POP%02d.DownloadLargeSizeLimit = %d\n", i, msn->largeMailSizeLimit);
      fprintf(fh, "POP%02d.NotifyByRequester      = %s\n", i, Bool2Txt(msn->notifyByRequester));
      fprintf(fh, "POP%02d.NotifyByOS41System     = %s\n", i, Bool2Txt(msn->notifyByOS41System));
      fprintf(fh, "POP%02d.NotifyBySound          = %s\n", i, Bool2Txt(msn->notifyBySound));
      fprintf(fh, "POP%02d.NotifyByCommand        = %s\n", i, Bool2Txt(msn->notifyByCommand));
      fprintf(fh, "POP%02d.NotifySound            = %s\n", i, msn->notifySound);
      fprintf(fh, "POP%02d.NotifyCommand          = %s\n", i, msn->notifyCommand);
      fprintf(fh, "POP%02d.SSLCert                = %s\n", i, msn->certFingerprint);
      fprintf(fh, "POP%02d.SSLCertFailures        = %d\n", i, msn->certFailures);
      fprintf(fh, "POP%02d.IncomingFolderID       = %08x\n", i, msn->mailStoreFolderID);

      i++;
    }

    fprintf(fh, "\n[Signature]\n");
    fprintf(fh, "TagsFile          = %s\n", co->TagsFile);
    fprintf(fh, "TagsSeparator     = %s\n", co->TagsSeparator);

    // we iterate through our signature list and output
    // the data of each signature here
    i = 0;
    IterateList(&co->signatureList, struct SignatureNode *, sn)
    {
      char *sig = ExportSignature(sn->signature);

      fprintf(fh, "SIG%02d.ID          = %08x\n", i, sn->id);
      fprintf(fh, "SIG%02d.Enabled     = %s\n", i, Bool2Txt(sn->active));
      fprintf(fh, "SIG%02d.Description = %s\n", i, sn->description);
      fprintf(fh, "SIG%02d.Filename    = %s\n", i, sn->filename);
      fprintf(fh, "SIG%02d.Signature   = %s\n", i, sig != NULL ? sig : "");
      fprintf(fh, "SIG%02d.UseSigFile  = %s\n", i, Bool2Txt(sn->useSignatureFile));

      dstrfree(sig);

      i++;
    }

    fprintf(fh, "\n[Identities]\n");

    // we iterate through our mail server list and ouput the POP3 servers in it
    i = 0;
    IterateList(&co->userIdentityList, struct UserIdentityNode *, uin)
    {
      fprintf(fh, "ID%02d.ID                 = %08x\n", i, uin->id);
      fprintf(fh, "ID%02d.Enabled            = %s\n", i, Bool2Txt(uin->active));
      fprintf(fh, "ID%02d.Description        = %s\n", i, uin->description);
      fprintf(fh, "ID%02d.Realname           = %s\n", i, uin->realname);
      fprintf(fh, "ID%02d.Address            = %s\n", i, uin->address);
      fprintf(fh, "ID%02d.Organization       = %s\n", i, uin->organization);
      fprintf(fh, "ID%02d.MailServerID       = %08x\n", i, uin->smtpServer != NULL ? uin->smtpServer->id : 0);
      fprintf(fh, "ID%02d.SignatureID        = %08x\n", i, uin->signature != NULL ? uin->signature->id : 0);
      fprintf(fh, "ID%02d.MailCC             = %s\n", i, uin->mailCC);
      fprintf(fh, "ID%02d.MailBCC            = %s\n", i, uin->mailBCC);
      fprintf(fh, "ID%02d.MailReplyTo        = %s\n", i, uin->mailReplyTo);
      fprintf(fh, "ID%02d.ExtraHeaders       = %s\n", i, uin->extraHeaders);
      fprintf(fh, "ID%02d.PhotoURL           = %s\n", i, uin->photoURL);
      fprintf(fh, "ID%02d.SaveSentMail       = %s\n", i, Bool2Txt(uin->saveSentMail));
      fprintf(fh, "ID%02d.SentFolderID       = %08x\n", i, uin->sentFolderID);
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

    fprintf(fh, "\n[Filters]\n");

    // we iterate through our filter list and save out the whole filter
    // configuration accordingly.
    i = 0;
    IterateList(&co->filterList, struct FilterNode *, filter)
    {
      // don't save volatile filters
      if(filter->isVolatile == FALSE)
      {
        int j;
        struct RuleNode *rule;

        fprintf(fh, "FI%02d.Name           = %s\n", i, filter->name);
        fprintf(fh, "FI%02d.Remote         = %s\n", i, Bool2Txt(filter->remote));
        fprintf(fh, "FI%02d.ApplyToNew     = %s\n", i, Bool2Txt(filter->applyToNew));
        fprintf(fh, "FI%02d.ApplyToSent    = %s\n", i, Bool2Txt(filter->applyToSent));
        fprintf(fh, "FI%02d.ApplyOnReq     = %s\n", i, Bool2Txt(filter->applyOnReq));
        fprintf(fh, "FI%02d.Combine        = %d\n", i, filter->combine);

        // now we do have to iterate through our ruleList
        j = 0;
        IterateList(&filter->ruleList, struct RuleNode *, rule)
        {
          fprintf(fh, "FI%02d.Field%d         = %d\n", i, j, rule->searchMode);
          fprintf(fh, "FI%02d.SubField%d      = %d\n", i, j, rule->subSearchMode);
          fprintf(fh, "FI%02d.CustomField%d   = %s\n", i, j, rule->customField);
          fprintf(fh, "FI%02d.Comparison%d    = %d\n", i, j, rule->comparison);
          fprintf(fh, "FI%02d.Match%d         = %s\n", i, j, rule->matchPattern);
          fprintf(fh, "FI%02d.CaseSens%d      = %s\n", i, j, Bool2Txt(isFlagSet(rule->flags, SEARCHF_CASE_SENSITIVE)));
          fprintf(fh, "FI%02d.Substring%d     = %s\n", i, j, Bool2Txt(isFlagSet(rule->flags, SEARCHF_SUBSTRING)));
          fprintf(fh, "FI%02d.DOSPattern%d    = %s\n", i, j, Bool2Txt(isFlagSet(rule->flags, SEARCHF_DOS_PATTERN)));
          fprintf(fh, "FI%02d.SkipEncrypted%d = %s\n", i, j, Bool2Txt(isFlagSet(rule->flags, SEARCHF_SKIP_ENCRYPTED)));

          j++;
        }

        fprintf(fh, "FI%02d.Actions        = %d\n", i, filter->actions);
        fprintf(fh, "FI%02d.RedirectTo     = %s\n", i, filter->redirectTo);
        fprintf(fh, "FI%02d.ForwardTo      = %s\n", i, filter->forwardTo);
        fprintf(fh, "FI%02d.ReplyFile      = %s\n", i, filter->replyFile);
        fprintf(fh, "FI%02d.ExecuteCmd     = %s\n", i, filter->executeCmd);
        fprintf(fh, "FI%02d.PlaySound      = %s\n", i, filter->playSound);
        fprintf(fh, "FI%02d.MoveToFolderID = %08x\n", i, filter->moveToID);

        i++;
      }
    }

    fprintf(fh, "\n[Spam filter]\n");
    fprintf(fh, "SpamFilterEnabled   = %s\n", Bool2Txt(co->SpamFilterEnabled));
    fprintf(fh, "SpamFilterForNew    = %s\n", Bool2Txt(co->SpamFilterForNewMail));
    fprintf(fh, "SpamMarkOnMove      = %s\n", Bool2Txt(co->SpamMarkOnMove));
    fprintf(fh, "SpamMarkAsRead      = %s\n", Bool2Txt(co->SpamMarkAsRead));
    fprintf(fh, "SpamABookIsWhite    = %s\n", Bool2Txt(co->SpamAddressBookIsWhiteList));
    fprintf(fh, "SpamProbThreshold   = %d\n", co->SpamProbabilityThreshold);
    fprintf(fh, "SpamFlushInterval   = %d\n", co->SpamFlushTrainingDataInterval);
    fprintf(fh, "SpamFlushThres      = %d\n", co->SpamFlushTrainingDataThreshold);
    fprintf(fh, "MoveHamToIncoming   = %s\n", Bool2Txt(co->MoveHamToIncoming));
    fprintf(fh, "FilterHam           = %s\n", Bool2Txt(co->FilterHam));
    fprintf(fh, "TrustExternalFilter = %s\n", Bool2Txt(co->SpamTrustExternalFilter));
    fprintf(fh, "ExternalFilter      = %s\n", co->SpamExternalFilter);

    fprintf(fh, "\n[Read]\n");
    fprintf(fh, "ShowHeader        = %d\n", co->ShowHeader);
    fprintf(fh, "ShortHeaders      = %s\n", co->ShortHeaders);
    fprintf(fh, "ShowSenderInfo    = %d\n", co->ShowSenderInfo);
    fprintf(fh, "WrapHeader        = %s\n", Bool2Txt(co->WrapHeader));
    fprintf(fh, "SigSepLine        = %d\n", co->SigSepLine);
    fprintf(fh, "ColorSignature    = %s\n", co->ColorSignature.buf);
    fprintf(fh, "ColoredText       = %s\n", co->ColoredText.buf);
    fprintf(fh, "Color1stLevel     = %s\n", co->Color1stLevel.buf);
    fprintf(fh, "Color2ndLevel     = %s\n", co->Color2ndLevel.buf);
    fprintf(fh, "Color3rdLevel     = %s\n", co->Color3rdLevel.buf);
    fprintf(fh, "Color4thLevel     = %s\n", co->Color4thLevel.buf);
    fprintf(fh, "ColorURL          = %s\n", co->ColorURL.buf);
    fprintf(fh, "DisplayAllTexts   = %s\n", Bool2Txt(co->DisplayAllTexts));
    fprintf(fh, "FixedFontEdit     = %s\n", Bool2Txt(co->FixedFontEdit));
    fprintf(fh, "UseTextStyles     = %s\n", Bool2Txt(co->UseTextStylesRead));
    fprintf(fh, "TextColorsRead    = %s\n", Bool2Txt(co->UseTextColorsRead));
    fprintf(fh, "DisplayAllAltPart = %s\n", Bool2Txt(co->DisplayAllAltPart));
    fprintf(fh, "MDNEnabled        = %s\n", Bool2Txt(co->MDNEnabled));
    fprintf(fh, "MDN_NoRecipient   = %d\n", co->MDN_NoRecipient);
    fprintf(fh, "MDN_NoDomain      = %d\n", co->MDN_NoDomain);
    fprintf(fh, "MDN_OnDelete      = %d\n", co->MDN_OnDelete);
    fprintf(fh, "MDN_Other         = %d\n", co->MDN_Other);
    fprintf(fh, "MultipleWindows   = %s\n", Bool2Txt(co->MultipleReadWindows));
    fprintf(fh, "StatusChangeDelay = %d\n", co->StatusChangeDelayOn ? co->StatusChangeDelay : -co->StatusChangeDelay);
    fprintf(fh, "ConvertHTML       = %s\n", Bool2Txt(co->ConvertHTML));
    fprintf(fh, "DetectCyrillic    = %s\n", Bool2Txt(co->DetectCyrillic));
    fprintf(fh, "MapForeignChars   = %s\n", Bool2Txt(co->MapForeignChars));
    fprintf(fh, "GlobalMailThreads = %s\n", Bool2Txt(co->GlobalMailThreads));

    fprintf(fh, "\n[Write]\n");
    fprintf(fh, "NewIntro             = %s\n", co->NewIntro);
    fprintf(fh, "Greetings            = %s\n", co->Greetings);
    fprintf(fh, "WarnSubject          = %s\n", Bool2Txt(co->WarnSubject));
    fprintf(fh, "AttachmentReminder   = %s\n", Bool2Txt(co->AttachmentReminder));
    fprintf(fh, "AttachmentKeywords   = %s\n", co->AttachmentKeywords);
    fprintf(fh, "EdWrapCol            = %d\n", co->EdWrapCol);
    fprintf(fh, "EdWrapMode           = %d\n", co->EdWrapMode);
    fprintf(fh, "LaunchAlways         = %s\n", Bool2Txt(co->LaunchAlways));
    fprintf(fh, "EmailCache           = %d\n", co->EmailCache);
    fprintf(fh, "AutoSave             = %d\n", co->AutoSave);
    fprintf(fh, "WriteCharset         = %s\n", co->DefaultWriteCodeset);
    fprintf(fh, "FixedFontWrite       = %s\n", Bool2Txt(co->UseFixedFontWrite));
    fprintf(fh, "TextStylesWrite      = %s\n", Bool2Txt(co->UseTextStylesWrite));
    fprintf(fh, "TextColorsWrite      = %s\n", Bool2Txt(co->UseTextColorsWrite));
    fprintf(fh, "ShowRcptFieldCC      = %s\n", Bool2Txt(co->ShowRcptFieldCC));
    fprintf(fh, "ShowRcptFieldBCC     = %s\n", Bool2Txt(co->ShowRcptFieldBCC));
    fprintf(fh, "ShowRcptFieldReplyTo = %s\n", Bool2Txt(co->ShowRcptFieldReplyTo));

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

    fprintf(fh, "\n[Lists]\n");
    fprintf(fh, "FolderCols        = %d\n", co->FolderCols);
    fprintf(fh, "MessageCols       = %d\n", co->MessageCols);
    fprintf(fh, "FixedFontList     = %s\n", Bool2Txt(co->FixedFontList));
    fprintf(fh, "DateTimeFormat    = %d\n", co->DSListFormat);
    fprintf(fh, "ABookLookup       = %s\n", Bool2Txt(co->ABookLookup));
    fprintf(fh, "FolderCntMenu     = %s\n", Bool2Txt(co->FolderCntMenu));
    fprintf(fh, "MessageCntMenu    = %s\n", Bool2Txt(co->MessageCntMenu));
    fprintf(fh, "FolderInfoMode    = %d\n", co->FolderInfoMode);
    fprintf(fh, "FolderDoubleClick = %s\n", Bool2Txt(co->FolderDoubleClick));

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
    fprintf(fh, "SaveLayoutOnQuit = %s\n", Bool2Txt(co->SaveLayoutOnQuit));

    fprintf(fh, "\n[MIME]\n");
    fprintf(fh, "MV00.ContentType = Default\n");
    fprintf(fh, "MV00.Command     = %s\n", C->DefaultMimeViewer);
    if(C->DefaultMimeViewerCodesetName[0] != '\0' &&
       stricmp(C->DefaultMimeViewerCodesetName, C->DefaultLocalCodeset) != 0)
    {
      fprintf(fh, "MV00.CharsetName = %s\n", C->DefaultMimeViewerCodesetName);
    }

    i = 1;
    IterateList(&C->mimeTypeList, struct MimeTypeNode *, mtNode)
    {
      fprintf(fh, "MV%02d.ContentType = %s\n", i, mtNode->ContentType);
      fprintf(fh, "MV%02d.Command     = %s\n", i, mtNode->Command);
      if(mtNode->Extension[0] != '\0')
        fprintf(fh, "MV%02d.Extension   = %s\n", i, mtNode->Extension);
      if(mtNode->Description[0] != '\0')
        fprintf(fh, "MV%02d.Description = %s\n", i, mtNode->Description);
      if(mtNode->CodesetName[0] != '\0' && stricmp(mtNode->CodesetName, C->DefaultLocalCodeset) != 0)
        fprintf(fh, "MV%02d.CharsetName = %s\n", i, mtNode->CodesetName);

      i++;
    }

    fprintf(fh, "\n[Address book]\n");
    fprintf(fh, "GalleryDir       = %s\n", co->GalleryDir);
    fprintf(fh, "ProxyServer      = %s\n", co->ProxyServer);
    fprintf(fh, "NewAddrGroup     = %s\n", co->NewAddrGroup);
    fprintf(fh, "AddToAddrbook    = %d\n", co->AddToAddrbook);
    fprintf(fh, "AddrbookCols     = %d\n", co->AddrbookCols);

    fprintf(fh, "\n[Scripts]\n");
    for(i = 0; i < MACRO_COUNT; i++)
    {
      if(i < 10)
        fprintf(fh, "Rexx%02d.Name       = %s\n", i, co->RX[i].Name);

      fprintf(fh, "Rexx%02d.Script     = %s\n", i, co->RX[i].Script);
      fprintf(fh, "Rexx%02d.IsAmigaDOS = %s\n", i, Bool2Txt(co->RX[i].IsAmigaDOS));
      fprintf(fh, "Rexx%02d.UseConsole = %s\n", i, Bool2Txt(co->RX[i].UseConsole));
      fprintf(fh, "Rexx%02d.WaitTerm   = %s\n", i, Bool2Txt(co->RX[i].WaitTerm));
    }

    fprintf(fh, "\n[Mixed]\n");
    fprintf(fh, "TempDir            = %s\n", co->TempDir);
    fprintf(fh, "DetachDir          = %s\n", co->DetachDir);
    fprintf(fh, "AttachDir          = %s\n", co->AttachDir);
    fprintf(fh, "WBAppIcon          = %s\n", Bool2Txt(co->WBAppIcon));
    fprintf(fh, "IconPosition       = %d;%d\n", co->IconPositionX, co->IconPositionY);
    fprintf(fh, "AppIconText        = %s\n", co->AppIconText);
    fprintf(fh, "DockyIcon          = %s\n", Bool2Txt(co->DockyIcon));
    fprintf(fh, "IconifyOnQuit      = %s\n", Bool2Txt(co->IconifyOnQuit));
    fprintf(fh, "Confirm            = %s\n", Bool2Txt(co->Confirm));
    fprintf(fh, "ConfirmDelete      = %d\n", co->ConfirmDelete);
    fprintf(fh, "RemoveAtOnce       = %s\n", Bool2Txt(co->RemoveAtOnce));
    fprintf(fh, "XPKPack            = %s;%d\n", co->XPKPack, co->XPKPackEff);
    fprintf(fh, "XPKPackEncrypt     = %s;%d\n", co->XPKPackEncrypt, co->XPKPackEncryptEff);
    fprintf(fh, "PackerCommand      = %s\n", co->PackerCommand);
    fprintf(fh, "ShowPackerProgress = %s\n", Bool2Txt(co->ShowPackerProgress));
    fprintf(fh, "TransferWindow     = %d\n", co->TransferWindow);
    fprintf(fh, "Editor             = %s\n", co->Editor);

    if(co->DefaultEditorCodeset[0] != '\0' && stricmp(co->DefaultEditorCodeset, co->DefaultLocalCodeset) != 0)
      fprintf(fh, "EditorCharset      = %s\n", co->DefaultEditorCodeset);

    fprintf(fh, "\n[Look&Feel]\n");
    fprintf(fh, "Theme             = %s\n", co->ThemeName);
    fprintf(fh, "InfoBarPos        = %d\n", co->InfoBarPos);
    fprintf(fh, "InfoBarText       = %s\n", co->InfoBarText);
    fprintf(fh, "QuickSearchBarPos = %d\n", co->QuickSearchBarPos);
    fprintf(fh, "EmbeddedReadPane  = %s\n", Bool2Txt(co->EmbeddedReadPane));
    fprintf(fh, "SizeFormat        = %d\n", co->SizeFormat);

    fprintf(fh, "\n[Update]\n");
    fprintf(fh, "UpdateInterval     = %d\n", co->UpdateInterval);
    fprintf(fh, "UpdateServer       = %s\n", co->UpdateServer);
    fprintf(fh, "UpdateDownloadPath = %s\n", co->UpdateDownloadPath);

    fprintf(fh, "\n[Advanced]\n");
    fprintf(fh, "LetterPart               = %d\n", co->LetterPart);
    fprintf(fh, "WriteIndexes             = %d\n", co->WriteIndexes);
    fprintf(fh, "ExpungeIndexes           = %d\n", co->ExpungeIndexes);
    fprintf(fh, "SupportSite              = %s\n", co->SupportSite);
    fprintf(fh, "JumpToIncoming           = %s\n", Bool2Txt(co->JumpToIncoming));
    fprintf(fh, "AskJumpUnread            = %s\n", Bool2Txt(co->AskJumpUnread));
    fprintf(fh, "PrinterCheck             = %s\n", Bool2Txt(co->PrinterCheck));
    fprintf(fh, "IsOnlineCheck            = %s\n", Bool2Txt(co->IsOnlineCheck));
    fprintf(fh, "IOCInterface             = %s\n", co->IOCInterfaces);
    fprintf(fh, "ConfirmOnQuit            = %s\n", Bool2Txt(co->ConfirmOnQuit));
    fprintf(fh, "HideGUIElements          = %d\n", co->HideGUIElements);
    fprintf(fh, "SysCharsetCheck          = %s\n", Bool2Txt(co->SysCharsetCheck));
    fprintf(fh, "AmiSSLCheck              = %s\n", Bool2Txt(co->AmiSSLCheck));
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
    fprintf(fh, "DefaultSSLCiphers        = %s\n", co->DefaultSSLCiphers);
    fprintf(fh, "MachineFQDN              = %s\n", co->MachineFQDN);
    fprintf(fh, "OverrideFromAddress      = %s\n", Bool2Txt(co->OverrideFromAddress));

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
/// ImportExternalSpamFilters
// import additional spam filter rules
void ImportExternalSpamFilters(struct Config *co)
{
  ENTER();

  // make sure that the spam folder really exists to be able
  // to move spam mails to it
  if(FO_GetFolderByType(FT_SPAM, NULL) != NULL)
  {
    struct FilterNode *filter;
    struct FilterNode *succ;

    // remove previous volatile filters first
    SafeIterateList(&co->filterList, struct FilterNode *, filter, succ)
    {
      if(filter->isVolatile == TRUE)
      {
        Remove((struct Node *)filter);
        DeleteFilterNode(filter);
      }
    }

    if(co->SpamTrustExternalFilter == TRUE && co->SpamExternalFilter[0] != '\0')
    {
      char externalPath[SIZE_PATHFILE];

      // now import the filters from the given external description
      snprintf(externalPath, sizeof(externalPath), "PROGDIR:Resources/spamfilters/%s.sfd", co->SpamExternalFilter);
      ImportFilter(externalPath, TRUE, &co->filterList);
    }
  }

  LEAVE();
}

///

/// IsValidConfig
// verifies if the required settings have been made
BOOL IsValidConfig(const struct Config *co)
{
  BOOL valid;
  struct UserIdentityNode *firstIdentity;
  struct MailServerNode *firstPOP3;
  struct MailServerNode *firstSMTP;

  ENTER();

  firstIdentity = GetUserIdentity(&co->userIdentityList, 0, TRUE);
  firstPOP3 = GetMailServer(&co->pop3ServerList, 0);
  firstSMTP = GetMailServer(&co->smtpServerList, 0);

  valid = (firstIdentity != NULL &&
           firstIdentity->address[0] != '\0' &&
           firstIdentity->realname[0] != '\0' &&
           firstSMTP->hostname[0] != '\0' &&
           firstPOP3->hostname[0] != '\0');

  if(valid == FALSE)
  {
    DoMethod(G->App, MUIM_YAMApplication_OpenConfigWindow);
    MUI_Request(G->App, G->MA != NULL ? G->MA->GUI.WI : NULL, MUIF_NONE, NULL, tr(MSG_OkayReq), tr(MSG_CO_InvalidConf));
  }

  RETURN(valid);
  return valid;
}

///
/// DetectPGP
// checks if PGP 2 or 5 is available
static int DetectPGP(const struct Config *co)
{
  int version;
  APTR oldWindowPtr;
  char fname[SIZE_PATHFILE];

  ENTER();

  // make sure the OS doesn't popup any
  // 'Please insert volume' kind warnings
  oldWindowPtr = SetProcWindow((APTR)-1);

  if(FileExists(AddPath(fname, co->PGPCmdPath, "pgpe", sizeof(fname))) == TRUE)
  {
    D(DBF_STARTUP, "found PGP version 5 installed in '%s'", co->PGPCmdPath);
    version = 5;
  }
  else if(FileExists(AddPath(fname, co->PGPCmdPath, "pgp", sizeof(fname))) == TRUE)
  {
    D(DBF_STARTUP, "found PGP version 2 installed in '%s'", co->PGPCmdPath);
    version = 2;
  }
  else
  {
    W(DBF_STARTUP, "no PGP version found to be installed in '%s'", co->PGPCmdPath);
    version = 0;
  }

  // restore the old windowPtr
  SetProcWindow(oldWindowPtr);

  RETURN(version);
  return version;
}

///
/// ValidateConfig
//  Validates a configuration, update GUI etc.
void ValidateConfig(struct Config *co, BOOL update, BOOL saveChanges)
{
  BOOL saveAtEnd = FALSE;
  BOOL updateReadWindows = FALSE;
  BOOL updateWriteWindows = FALSE;
  BOOL updateHeaderMode = FALSE;
  BOOL updateSenderInfo = FALSE;
  BOOL updateMenuShortcuts = FALSE;
  Object *refWindow;
  struct UserIdentityNode *firstIdentity;
  struct MailServerNode *firstPOP3;
  struct MailServerNode *firstSMTP;
  struct MailServerNode *msn;
  struct UserIdentityNode *uin;
  struct SignatureNode *sn;
  struct FilterNode *filter;

  ENTER();

  // save a pointer to a reference window in case
  // we need to open a requester
  if(G->ConfigWinObject != NULL)
    refWindow = G->ConfigWinObject;
  else if(G->MA != NULL && G->MA->GUI.WI != NULL)
    refWindow = G->MA->GUI.WI;
  else
    refWindow = NULL;

  firstIdentity = GetUserIdentity(&co->userIdentityList, 0, TRUE);
  firstPOP3 = GetMailServer(&co->pop3ServerList, 0);
  firstSMTP = GetMailServer(&co->smtpServerList, 0);
  if(firstIdentity != NULL && firstPOP3 != NULL && firstSMTP != NULL)
  {
    // now we walk through our POP3 server list and check and fix certains
    // things in it
    IterateList(&co->pop3ServerList, struct MailServerNode *, msn)
    {
      if(msn->hostname[0] == '\0')
        strlcpy(msn->hostname, firstSMTP->hostname, sizeof(msn->hostname));

      if(msn->port == 0)
        msn->port = 110;

      if(msn->username[0] == '\0')
      {
        char *p = strchr(firstIdentity->address, '@');
        strlcpy(msn->username, firstIdentity->address, p ? (unsigned int)(p - firstIdentity->address + 1) : sizeof(msn->username));
      }

      if(msn->description[0] == '\0')
        snprintf(msn->description, sizeof(msn->description), "%s@%s", msn->username, msn->hostname);
    }

    // now we walk through our SMTP server list and check and fix certains
    // things in it
    IterateList(&co->smtpServerList, struct MailServerNode *, msn)
    {
      if(msn->hostname[0] == '\0')
        strlcpy(msn->hostname, firstPOP3->hostname, sizeof(msn->hostname));

      if(msn->port == 0)
        msn->port = 25;

      if(msn->description[0] == '\0')
        snprintf(msn->description, sizeof(msn->description), "%s@%s", msn->username, msn->hostname);
    }
  }

  // check all servers for valid and unique IDs
  IterateList(&co->pop3ServerList, struct MailServerNode *, msn)
  {
    // check for a valid and unique ID, this is independend of the server type
    if(msn->id == 0)
    {
      int id;

      // loop until we generated a unique ID
      // usually this will happen with just one iteration
      do
      {
        id = rand();

        if(id == 0)
          continue;
      }
      while(IsUniqueMailServerID(&co->pop3ServerList, id) == FALSE);

      D(DBF_CONFIG, "replaced invalid id of POP3 server '%s'", msn->description);
      msn->id = id;

      saveAtEnd = TRUE;
    }
  }

  // check all servers for valid and unique IDs
  IterateList(&co->smtpServerList, struct MailServerNode *, msn)
  {
    // check for a valid and unique ID, this is independend of the server type
    if(msn->id == 0)
    {
      int id;

      // loop until we generated a unique ID
      // usually this will happen with just one iteration
      do
      {
        id = rand();

        if(id == 0)
          continue;
      }
      while(IsUniqueMailServerID(&co->smtpServerList, id) == FALSE);

      D(DBF_CONFIG, "replaced invalid id of SMTP server '%s'", msn->description);
      msn->id = id;

      saveAtEnd = TRUE;
    }
  }

  // check all identities for valid and unique IDs
  IterateList(&co->userIdentityList, struct UserIdentityNode *, uin)
  {
    // check for a valid and unique ID
    if(uin->id == 0)
    {
      int id;

      // loop until we generated a unique ID
      // usually this will happen with just one iteration
      do
      {
        id = rand();

        if(id == 0)
          continue;
      }
      while(FindUserIdentityByID(&co->userIdentityList, id) != NULL);

      D(DBF_CONFIG, "replaced invalid id of user identity '%s'", uin->description);
      uin->id = id;

      saveAtEnd = TRUE;
    }
  }

  // we make sure that the signature list is not empty and if
  // so that normally signals that the config didn't carry any
  // configuration items for signatures.
  // In that case we check if the old-style signature files
  // exist and if we we add these ones as the default ones
  if(IsMinListEmpty(&co->signatureList))
  {
    // before YAM 2.8 we had three default signatures. We therefore
    // check for these three files now.
    if((sn = CreateSignatureFromFile(".signature", tr(MSG_CO_DefSig))) != NULL)
      AddTail((struct List *)&co->signatureList, (struct Node *)sn);

    // check for ".altsignature1" file
    if((sn = CreateSignatureFromFile(".altsignature1", tr(MSG_CO_AltSig1))) != NULL)
      AddTail((struct List *)&co->signatureList, (struct Node *)sn);

    // check for ".altsignature2" file
    if((sn = CreateSignatureFromFile(".altsignature2", tr(MSG_CO_AltSig2))) != NULL)
      AddTail((struct List *)&co->signatureList, (struct Node *)sn);

    D(DBF_CONFIG, "added default signatures");
    saveAtEnd = TRUE;
  }

  // check all signatures for valid and unique IDs
  IterateList(&co->signatureList, struct SignatureNode *, sn)
  {
    // check for a valid and unique ID
    if(sn->id == 0)
    {
      int id;

      // loop until we generated a unique ID
      // usually this will happen with just one iteration
      do
      {
        id = rand();

        if(id == 0)
          continue;
      }
      while(IsUniqueSignatureID(&co->signatureList, id) == FALSE);

      sn->id = id;

      D(DBF_CONFIG, "replaced invalid id of signature '%s'", sn->description);
      saveAtEnd = TRUE;
    }
  }

  // update the write windows in any case
  updateWriteWindows = TRUE;

  // check if the Location is setup correctly and if not
  // we use GuessTZone() to actually get an almost matching Location
  // definition or we set the Location to a default in the catalog
  if(co->Location[0] == '\0')
  {
    if(G->Locale != NULL)
    {
      LONG gmtOffset = -(G->Locale->loc_GMTOffset);

      D(DBF_CONFIG, "got GMT offset %ld from locale.library", gmtOffset);

      strlcpy(co->Location, GuessTZone(gmtOffset), sizeof(co->Location));
    }
    else
      strlcpy(co->Location, tr(MSG_CO_FALLBACK_TZONE), sizeof(co->Location));
  }

  // now we have to make sure we set the Location global now
  SetTZone(co->Location);

  // check if PGP is available or not.
  G->PGPVersion = DetectPGP(co);

  // prepare the temporary directory
  CreateDirectory(co->TempDir);

  // check if the current configuration is already valid at an absolute
  // minimum. This will open the config window in case of an invalid config.
  IsValidConfig(C);

  // we try to find out the system charset and validate it with the
  // currently configured local charset
  if(co->SysCharsetCheck == TRUE)
  {
    if(G->systemCodeset != NULL)
    {
      // now we check whether the currently set localCharset matches
      // the system charset or not
      if(co->DefaultLocalCodeset[0] != '\0' && G->systemCodeset->name[0] != '\0')
      {
        if(stricmp(co->DefaultLocalCodeset, G->systemCodeset->name) != 0)
        {
          int res = MUI_Request(G->App, refWindow, MUIF_NONE,
                                tr(MSG_CO_CHARSETWARN_TITLE),
                                tr(MSG_CO_CHARSETWARN_BT),
                                tr(MSG_CO_CHARSETWARN),
                                co->DefaultLocalCodeset, G->systemCodeset->name);

          // if the user has clicked on Change, we do
          // change the charset and save it immediatly
          if(res == 1)
          {
            D(DBF_CONFIG, "updated local codeset from '%s' to '%s'", co->DefaultLocalCodeset, G->systemCodeset->name);
            strlcpy(co->DefaultLocalCodeset, G->systemCodeset->name, sizeof(co->DefaultLocalCodeset));
            saveAtEnd = TRUE;
          }
          else if(res == 2)
          {
            co->SysCharsetCheck = FALSE;
            saveAtEnd = TRUE;
          }
        }
      }
      else if(G->systemCodeset->name[0] != '\0')
      {
        D(DBF_CONFIG, "updated local codeset from '%s' to '%s'", co->DefaultLocalCodeset, G->systemCodeset->name);
        strlcpy(co->DefaultLocalCodeset, G->systemCodeset->name, sizeof(co->DefaultLocalCodeset));
        saveAtEnd = TRUE;
      }
      else
        W(DBF_CONFIG, "checking the system's codeset seem to have failed?!?");
    }
    else
      W(DBF_CONFIG, "CodesetsFindA(NULL) failed!");
  }

  // if the local charset is still empty we set the default
  // charset to 'iso-8859-1' as this one is probably the most common one.
  if(co->DefaultLocalCodeset[0] == '\0')
  {
    D(DBF_CONFIG, "updated local codeset from '%s' to '%s'", co->DefaultLocalCodeset, "ISO-8859-1");
    strlcpy(co->DefaultLocalCodeset, "ISO-8859-1", sizeof(co->DefaultLocalCodeset));
    saveAtEnd = TRUE;
  }

  if(co->DefaultWriteCodeset[0] == '\0')
  {
    D(DBF_CONFIG, "updated write codeset from '%s' to '%s'", co->DefaultWriteCodeset, co->DefaultLocalCodeset);
    strlcpy(co->DefaultWriteCodeset, co->DefaultLocalCodeset, sizeof(co->DefaultWriteCodeset));
    saveAtEnd = TRUE;
  }

  if(co->DefaultEditorCodeset[0] == '\0')
  {
    D(DBF_CONFIG, "updated editor codeset from '%s' to '%s'", co->DefaultEditorCodeset, co->DefaultLocalCodeset);
    strlcpy(co->DefaultEditorCodeset, co->DefaultLocalCodeset, sizeof(co->DefaultEditorCodeset));
    saveAtEnd = TRUE;
  }

  // now we check if the set default read charset is a valid one also supported
  // by codesets.library and if not we warn the user
  if((G->localCodeset = CodesetsFind(co->DefaultLocalCodeset,
                                     CSA_CodesetList,       G->codesetsList,
                                     CSA_FallbackToDefault, FALSE,
                                     TAG_DONE)) == NULL)
  {
    int res = MUI_Request(G->App, refWindow, MUIF_NONE,
                          tr(MSG_CO_CHARSETWARN_TITLE),
                          tr(MSG_CO_CHARSETUNKNOWNWARN_BT),
                          tr(MSG_CO_CHARSETUNKNOWNWARN),
                          co->DefaultLocalCodeset);
    if(res == 1)
    {
      // fallback to the system's default codeset
      G->localCodeset = G->systemCodeset;
      D(DBF_CONFIG, "updated local codeset from '%s' to '%s'", co->DefaultLocalCodeset, G->systemCodeset->name);
      strlcpy(co->DefaultLocalCodeset, G->systemCodeset->name, sizeof(co->DefaultLocalCodeset));
      saveAtEnd = TRUE;
    }
  }

  // now we check if the set default write charset is a valid one also supported
  // by codesets.library and if not we warn the user
  if((G->writeCodeset = CodesetsFind(co->DefaultWriteCodeset,
                                     CSA_CodesetList,       G->codesetsList,
                                     CSA_FallbackToDefault, FALSE,
                                     TAG_DONE)) == NULL)
  {
    int res = MUI_Request(G->App, refWindow, MUIF_NONE,
                          tr(MSG_CO_CHARSETWARN_TITLE),
                          tr(MSG_CO_CHARSETUNKNOWNWARN_BT),
                          tr(MSG_CO_CHARSETUNKNOWNWARN),
                          co->DefaultWriteCodeset);
    if(res == 1)
    {
      // fallback to the system's default codeset
      D(DBF_CONFIG, "updated write codeset from '%s' to '%s'", co->DefaultWriteCodeset, G->systemCodeset->name);
      G->writeCodeset = G->systemCodeset;
      strlcpy(co->DefaultWriteCodeset, G->systemCodeset->name, sizeof(co->DefaultWriteCodeset));
      saveAtEnd = TRUE;
    }
  }

  // now we check if the set editor codeset is a valid one also supported
  // by codesets.library and if not we warn the user
  if(CodesetsFind(co->DefaultEditorCodeset,
                  CSA_CodesetList,       G->codesetsList,
                  CSA_FallbackToDefault, FALSE,
                  TAG_DONE) == NULL)
  {
    int res = MUI_Request(G->App, refWindow, MUIF_NONE,
                          tr(MSG_CO_CHARSETWARN_TITLE),
                          tr(MSG_CO_CHARSETUNKNOWNWARN_BT),
                          tr(MSG_CO_CHARSETUNKNOWNWARN),
                          co->DefaultEditorCodeset);
    if(res == 1)
    {
      // fallback to the system's default codeset
      D(DBF_CONFIG, "updated editor codeset from '%s' to '%s'", co->DefaultEditorCodeset, G->systemCodeset->name);
      G->editorCodeset = G->systemCodeset;
      strlcpy(co->DefaultEditorCodeset, G->systemCodeset->name, sizeof(co->DefaultEditorCodeset));
      saveAtEnd = TRUE;
    }
  }

  // we also check if AmiSSL was found installed or not. And in case the
  // AmiSSL warning is enabled we notify the user about a not running
  // amissl installation.
  if(co->AmiSSLCheck == TRUE)
  {
    if(AmiSSLMasterBase == NULL || AmiSSLBase == NULL || G->TR_UseableTLS == FALSE)
    {
      int res = MUI_Request(G->App, refWindow, MUIF_NONE,
                            tr(MSG_CO_AMISSLWARN_TITLE),
                            tr(MSG_CO_AMISSLWARN_BT),
                            tr(MSG_CO_AMISSLWARN),
                            AMISSLMASTER_MIN_VERSION, 5);

      // if the user has clicked on "Ignore always", we do
      // change the AmiSSLCheck variables and save the config
      // immediatly
      if(res == 1)
      {
        exit(RETURN_ERROR);
      }
      else if(res == 2)
      {
        D(DBF_CONFIG, "disabled AmiSSL check");
        co->AmiSSLCheck = FALSE;
        saveAtEnd = TRUE;
      }
    }
  }
  else
  {
    // we reenable the AmiSSLCheck as soon as we found
    // the library to be working fine.
    if(AmiSSLMasterBase != NULL && AmiSSLBase != NULL && G->TR_UseableTLS == TRUE)
    {
      D(DBF_CONFIG, "(re)enabled AmiSSL check");
      co->AmiSSLCheck = TRUE;
      saveAtEnd = TRUE;
    }
  }

  // check all filters for rules with empty strings
  IterateList(&co->filterList, struct FilterNode *, filter)
  {
    CheckFilterRules(filter);
  }

  if(co->SpamFilterEnabled == TRUE)
  {
    // limit the spam probability threshold to sensible values
    if(co->SpamProbabilityThreshold < 75)
    {
      D(DBF_CONFIG, "changed spam probability threshold from %ld to %ld", co->SpamProbabilityThreshold, 75);
      co->SpamProbabilityThreshold = 75;
      saveAtEnd = TRUE;
    }
    else if(co->SpamProbabilityThreshold > 99)
    {
      D(DBF_CONFIG, "changed spam probability threshold from %ld to %ld", co->SpamProbabilityThreshold, 99);
      co->SpamProbabilityThreshold = 99;
      saveAtEnd = TRUE;
    }

    ImportExternalSpamFilters(co);
  }

  if(co->StatusChangeDelay < 1000)
  {
    // a delay less than one second is not possible
    D(DBF_CONFIG, "changed status change delay %ld to %ld", co->StatusChangeDelay, 1000);
    co->StatusChangeDelay = 1000;
    saveAtEnd = TRUE;
  }
  else if(co->StatusChangeDelay > 10000)
  {
    // a delay longer than ten seconds is not possible, either
    D(DBF_CONFIG, "changed status change delay %ld to %ld", co->StatusChangeDelay, 10000);
    co->StatusChangeDelay = 10000;
    saveAtEnd = TRUE;
  }

  // check for valid birthday check times
  if(co->BirthdayCheckTime.ds_Minute < 0 || co->BirthdayCheckTime.ds_Minute > 23*60+59)
  {
    D(DBF_CONFIG, "changed birthday check interval from %ld to %ld", co->BirthdayCheckTime.ds_Minute, 10*60);
    co->BirthdayCheckTime.ds_Days = 0;
    co->BirthdayCheckTime.ds_Minute = 10*60;
    co->BirthdayCheckTime.ds_Tick = 0;
    saveAtEnd = TRUE;
  }

  // check for a minimum stack size
  if(co->StackSize < SIZE_STACK)
  {
    co->StackSize = SIZE_STACK;
    saveAtEnd = TRUE;
  }

  if(update == TRUE && G->ConfigWinObject != NULL)
  {
    BOOL updateAll = xget(G->ConfigWinObject, MUIA_ConfigWindow_UpdateAll);
    BOOL *visited = (BOOL *)xget(G->ConfigWinObject, MUIA_ConfigWindow_VisitedPages);

    if(visited[cp_FirstSteps] == TRUE || updateAll == TRUE)
    {
      // make sure to redraw the main mail list in case the user
      // changed the timezone
      DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_NList_Redraw, MUIV_NList_Redraw_All);
    }

    if(visited[cp_TCPIP] == TRUE || updateAll == TRUE)
    {
      DoMethod(G->MA->GUI.TO_TOOLBAR, MUIM_MainWindowToolbar_UpdateServerControls);

      // requeue the timerequest for the POP3 servers
      RestartPOP3Timers();
    }

    if(visited[cp_Spam] == TRUE || updateAll == TRUE)
    {
      // if we enabled or disable the spam filter then we need to update
      // the enable/disable status of some toolbar items of the main window
      MA_ChangeSelected(TRUE);

      // now we also have to update the Spam controls of our various
      // window toolbars
      DoMethod(G->MA->GUI.TO_TOOLBAR, MUIM_MainWindowToolbar_UpdateSpamControls);

      // open read windows need to be updated, too
      updateReadWindows = TRUE;
    }

    if(visited[cp_Read] == TRUE || updateAll == TRUE)
    {
      // open read windows need to be updated, too
      updateHeaderMode = TRUE;
      updateSenderInfo = TRUE;
    }

    if(visited[cp_Write] == TRUE || updateAll == TRUE)
    {
      // requeue the timerequest for the AutoSave interval
      RestartTimer(TIMER_AUTOSAVE, co->AutoSave, 0, FALSE);
    }

    if(visited[cp_ReplyForward] == TRUE || updateAll == TRUE)
    {
      // update the "Forward" shortcuts of the read window's menu
      updateMenuShortcuts = TRUE;
    }

    if(visited[cp_AddressBook] == TRUE || updateAll == TRUE)
    {
      if(G->ABookWinObject != NULL)
        set(G->ABookWinObject, MUIA_AddressBookWindow_ConfigModified, TRUE);
    }

    if(visited[cp_LookFeel] == TRUE || updateAll == TRUE)
    {
      // First we set the PG_MAILLIST and NL_FOLDER Quiet
      set(G->MA->GUI.PG_MAILLIST,MUIA_NList_Quiet,     TRUE);
      set(G->MA->GUI.NL_FOLDERS, MUIA_NListtree_Quiet, TRUE);

      // Now we reorder the Maingroup accordingly to the InfoBar/QuickSearchBar setting
      DoMethod(G->MA->GUI.WI, MUIM_MainWindow_Relayout);

      // Now we update the InfoBar because the text could have been changed
      DoMethod(G->MA->GUI.IB_INFOBAR, MUIM_InfoBar_SetFolder, GetCurrentFolder());

      // Modify the ContextMenu flags
      set(G->MA->GUI.PG_MAILLIST,MUIA_ContextMenu, C->MessageCntMenu ? MUIV_NList_ContextMenu_Always : MUIV_NList_ContextMenu_Never);
      set(G->MA->GUI.NL_FOLDERS, MUIA_ContextMenu, C->FolderCntMenu ? MUIV_NList_ContextMenu_Always : MUIV_NList_ContextMenu_Never);

      // Make sure to save the GUI layout before continuing
      SaveLayout(FALSE);

      // recreate the MUIA_NList_Format strings
      DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_MainFolderListtree_MakeFormat);
      DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_MainMailListGroup_MakeFormat);

      // now reload the layout
      LoadLayout();

      // Now we give the control back to the NLists
      set(G->MA->GUI.PG_MAILLIST,MUIA_NList_Quiet,     FALSE);
      set(G->MA->GUI.NL_FOLDERS, MUIA_NListtree_Quiet, FALSE);

      // and to not let the embedded read pane be empty when it is newly created
      // we have to make sure the actual selected mail is loaded
      if(C->EmbeddedReadPane == TRUE)
        MA_ChangeSelected(TRUE);
    }

    if(visited[cp_Mixed] == TRUE || updateAll == TRUE)
    {
      // setup the appIcon positions and display all statistics
      // accordingly.
      DisplayStatistics((struct Folder *)-1, TRUE);

      // in case the Docky icon was just enabled we must register YAM again
      // as an application.lib aware program, because application.lib seems
      // to be a bit buggy when it should change a not yet existing icon to
      // a custom one. Removing the Docky icon in case it was disabled is no
      // problem at all.
      // Don't get confused by the C and CE pointers. These have been swapped
      // before, thus C points to the current configuration while CE points
      // to the old configuration.
      if((C->DockyIcon == TRUE  && CE->DockyIcon == FALSE) ||
         (C->DockyIcon == FALSE && CE->DockyIcon == TRUE))
      {
        FreeDockyIcon();
        InitDockyIcon();
      }
      UpdateDockyIcon();
    }

    if(visited[cp_Update] == TRUE || updateAll == TRUE)
    {
      // make sure we reinit the update check timer
      InitUpdateCheck(FALSE);
    }

    // make sure the dynamic menus and some menu shortcuts of the main window
    // are properly refreshed.
    MA_SetupDynamicMenus();

    // update the embedded identity and signature pointers of all folders
    UpdateAllFolderSettings(co);
  }

  // if some items have been modified we save the configuration,
  // but only if we allowed to do so
  // forbidding will occur when the configuration is "used" only
  if(saveAtEnd == TRUE && saveChanges == TRUE)
    SaveConfig(co, G->CO_PrefsFile);

  // update possibly open read windows
  if(updateReadWindows == TRUE || updateHeaderMode == TRUE || updateSenderInfo == TRUE || updateMenuShortcuts == TRUE)
  {
    struct ReadMailData *rmData;

    IterateList(&G->readMailDataList, struct ReadMailData *, rmData)
    {
      if(rmData->mail != NULL)
      {
        // we use PushMethod for the case the read window modifies we list we are currently walking through
        if(rmData->readMailGroup != NULL && (updateHeaderMode == TRUE || updateSenderInfo == TRUE))
        {
          // forward the modified information directly to the read mail group
          if(updateHeaderMode == TRUE)
            DoMethod(_app(rmData->readMailGroup), MUIM_Application_PushMethod, rmData->readMailGroup, 2, MUIM_ReadMailGroup_ChangeHeaderMode, co->ShowHeader);

          if(updateSenderInfo == TRUE)
            DoMethod(_app(rmData->readMailGroup), MUIM_Application_PushMethod, rmData->readMailGroup, 2, MUIM_ReadMailGroup_ChangeSenderInfoMode, co->ShowSenderInfo);
        }
        else if(rmData->readWindow != NULL && (updateReadWindows == TRUE || updateMenuShortcuts == TRUE))
        {
          // forward the modifed information to the window, because a read mail group has no toolbar
          if(updateReadWindows == TRUE)
            DoMethod(_app(rmData->readWindow), MUIM_Application_PushMethod, rmData->readWindow, 2, MUIM_ReadWindow_ReadMail, rmData->mail);

          if(updateMenuShortcuts == TRUE)
            DoMethod(rmData->readWindow, MUIM_ReadWindow_UpdateMenuShortcuts);
        }
      }
    }
  }

  // update possibly open write windows
  if(updateWriteWindows == TRUE)
  {
    struct WriteMailData *wmData;

    IterateList(&G->writeMailDataList, struct WriteMailData *, wmData)
    {
      DoMethod(wmData->window, MUIM_WriteWindow_UpdateIdentities);
      DoMethod(wmData->window, MUIM_WriteWindow_UpdateSignatures);
    }
  }

  LEAVE();
}

///
/// ResolveConfigFolders
// resolve all folder IDs contained in the config to real folders
void ResolveConfigFolders(struct Config *co)
{
  struct MailServerNode *msn;
  struct UserIdentityNode *uin;
  struct FilterNode *filter;

  ENTER();

  // resolve the sent folders of the POP3 servers
  IterateList(&co->pop3ServerList, struct MailServerNode *, msn)
  {
    if(msn->mailStoreFolderID == 0 && IsStrEmpty(msn->mailStoreFolderName) == FALSE)
    {
      struct Folder *folder;

      if((folder = FO_GetFolderByName(msn->mailStoreFolderName, NULL)) != NULL)
        msn->mailStoreFolderID = folder->ID;
      else
        W(DBF_CONFIG, "cannot resolve sent folder '%s' of POP3 server '%s'", msn->mailStoreFolderName, msn->description);
    }
    else if(msn->mailStoreFolderID != 0)
    {
      struct Folder *folder;

      if((folder = FindFolderByID(G->folders, msn->mailStoreFolderID)) != NULL)
        strlcpy(msn->mailStoreFolderName, folder->Name, sizeof(msn->mailStoreFolderName));
      else
        W(DBF_CONFIG, "cannot resolve sent folder ID 0x%08lx of POP3 server '%s'", msn->mailStoreFolderID, msn->description);
    }
  }

  // resolve the sent folders of the SMTP servers
  IterateList(&co->smtpServerList, struct MailServerNode *, msn)
  {
    if(msn->mailStoreFolderID == 0 && IsStrEmpty(msn->mailStoreFolderName) == FALSE)
    {
      struct Folder *folder;

      if((folder = FO_GetFolderByName(msn->mailStoreFolderName, NULL)) != NULL)
        msn->mailStoreFolderID = folder->ID;
      else
        W(DBF_CONFIG, "cannot resolve sent folder '%s' of SMTP server '%s'", msn->mailStoreFolderName, msn->description);
    }
    else if(msn->mailStoreFolderID != 0)
    {
      struct Folder *folder;

      if((folder = FindFolderByID(G->folders, msn->mailStoreFolderID)) != NULL)
        strlcpy(msn->mailStoreFolderName, folder->Name, sizeof(msn->mailStoreFolderName));
      else
        W(DBF_CONFIG, "cannot resolve sent folder ID 0x%08lx of SMTP server '%s'", msn->mailStoreFolderID, msn->description);
    }
  }

  // resolve the sent folders of the user identities
  IterateList(&co->userIdentityList, struct UserIdentityNode *, uin)
  {
    if(uin->sentFolderID == 0 && IsStrEmpty(uin->sentFolderName) == FALSE)
    {
      struct Folder *folder;

      if((folder = FO_GetFolderByName(uin->sentFolderName, NULL)) != NULL)
        uin->sentFolderID = folder->ID;
      else
        W(DBF_CONFIG, "cannot resolve sent folder '%s' of user identity '%s'", uin->sentFolderName, uin->description);
    }
    else if(uin->sentFolderID != 0)
    {
      struct Folder *folder;

      if((folder = FindFolderByID(G->folders, uin->sentFolderID)) != NULL)
        strlcpy(uin->sentFolderName, folder->Name, sizeof(uin->sentFolderName));
      else
        W(DBF_CONFIG, "cannot resolve sent folder ID 0x%08lx of user identity '%s'", uin->sentFolderID, uin->description);
    }
  }

  // resolve the "move to" folders of the filters
  IterateList(&co->filterList, struct FilterNode *, filter)
  {
    if(filter->moveToID == 0 && IsStrEmpty(filter->moveToName) == FALSE)
    {
      struct Folder *folder;

      if((folder = FO_GetFolderByName(filter->moveToName, NULL)) != NULL)
        filter->moveToID = folder->ID;
      else
        W(DBF_CONFIG, "cannot resolve moveTo folder '%s' of filter '%s'", filter->moveToName, filter->name);
    }
    else if(filter->moveToID != 0)
    {
      struct Folder *folder;

      if((folder = FindFolderByID(G->folders, filter->moveToID)) != NULL)
        strlcpy(filter->moveToName, folder->Name, sizeof(filter->moveToName));
      else
        W(DBF_CONFIG, "cannot resolve sent folder ID 0x%08lx of filter '%s'", filter->moveToID, filter->name);
    }
  }

  LEAVE();
}

///
/// CheckConfigDiffs
// check C and CE for important state changes which need confirmation from the user
// returns TRUE if these changes are accepted or if something else was changed that
// requires a new comparison
BOOL CheckConfigDiffs(const BOOL *visited)
{
  BOOL result = FALSE;

  ENTER();

  if(visited[cp_Spam] == TRUE)
  {
    if(C->SpamFilterEnabled == TRUE && CE->SpamFilterEnabled == FALSE)
    {
      LONG mask;

      // raise a CheckboxRequest and ask the user which
      // operations he want to performed while disabling the
      // SPAM filter.
      mask = CheckboxRequest(G->ConfigWinObject, NULL, 3, tr(MSG_CO_SPAM_DISABLEFILTERASK),
                                                          tr(MSG_CO_SPAM_RESETTDATA),
                                                          tr(MSG_CO_SPAM_RESETMAILFLAGS),
                                                          tr(MSG_CO_SPAM_DELETESPAMFOLDER));

      SHOWVALUE(DBF_CONFIG, mask);
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

        // signal a change
        result = TRUE;
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
        ULONG answer;

        // the directory "spam" already exists, but it is not the standard spam folder
        // let the user decide what to do
        answer = MUI_Request(G->App, G->ConfigWinObject, MUIF_NONE, NULL, tr(MSG_ER_SPAMDIR_EXISTS_ANSWERS), tr(MSG_ER_SPAMDIR_EXISTS));
        switch(answer)
        {
          default:
          case 0:
          {
            // the user has chosen to disable the spam filter, so we do it
            // or the requester was cancelled
            CE->SpamFilterEnabled = FALSE;
            createSpamFolder = FALSE;
            // signal a change
            result = TRUE;
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
          // signal a change
          result = TRUE;
        }
        else
        {
          // move the new spam folder after the trash folder
          struct Folder *this = FO_GetFolderByType(FT_SPAM, NULL);
          struct Folder *prev = FO_GetFolderByType(FT_TRASH, NULL);

          DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NListtree_Move, MUIV_NListtree_Move_OldListNode_Root, this->Treenode, MUIV_NListtree_Move_NewListNode_Root, prev->Treenode, MUIF_NONE);

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
        if(MUI_Request(G->App, G->ConfigWinObject, MUIF_NONE, NULL, tr(MSG_YesNoReq), tr(MSG_ER_SPAM_NOT_ENOUGH_CLASSIFIED_MAILS), numberClassified))
        {
          CE->SpamMarkAsRead = FALSE;
          // signal a change
          result = TRUE;
        }
      }
    }
  }

  RETURN(result);
  return result;
}

///
