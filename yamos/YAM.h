#ifndef YAM_H
#define YAM_H

/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2001 by YAM Open Source Team

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

#include <dos/notify.h>
#include <exec/types.h>
#include <libraries/mui.h>
#include <proto/socket.h>

#include "YAM_rexx_rxif.h"   /* struct RuleResult */
#include "YAM_stringsizes.h"
#include "YAM_transfer.h"    /* struct DownloadResult */
#include "YAM_userlist.h"    /* struct Users */

/**************************************************************************/

/*** RxHook structure ***/
struct RxHook
{
   BOOL  IsAmigaDOS;
   BOOL  UseConsole;
   BOOL  WaitTerm;
   char  Name[SIZE_NAME];
   char  Script[SIZE_PATHFILE];
};

/*** Configuration main structure ***/
struct Config
{
   struct POP3 *    P3[MAXP3];
   struct Rule *    RU[MAXRU];
   struct MimeView *MV[MAXMV];

   int   TimeZone;
   int   PreSelection;
   int   TransferWindow;
   int   WarnSize;
   int   CheckMailDelay;
   int   NotifyType;
   int   ShowHeader;
   int   ShowSenderInfo;
   int   SigSepLine;
   int   EdWrapCol;
   int   EdWrapMode;
   int   FolderCols;
   int   MessageCols;
   int   LogfileMode;
   int   AddToAddrbook;
   int   AddrbookCols;
   int   IconPositionX;
   int   IconPositionY;
   int   ConfirmDelete;
   int   MDN_Display;
   int   MDN_Process;
   int   MDN_Delete;
   int   MDN_Filter;
   int   XPKPackEff;
   int   XPKPackEncryptEff;
   int   LetterPart; /*Hidden*/
   int   WriteIndexes;
   int   AutoSave;
   int   HideGUIElements;
   int   PrintMethod;
   int   StackSize;

   BOOL  DaylightSaving;
   BOOL  Allow8bit;
   BOOL  Use_SMTP_AUTH;
   BOOL  AvoidDuplicates;
   BOOL  UpdateStatus;
   BOOL  DownloadLarge;
   BOOL  DisplayAllTexts;
   BOOL  FixedFontEdit;
   BOOL  MultipleWindows;
   BOOL  UseTextstyles;
   BOOL  WrapHeader;
   BOOL  LaunchAlways;
   BOOL  QuoteMessage;
   BOOL  QuoteEmptyLines;
   BOOL  CompareAddress;
   BOOL  StripSignature;
   BOOL  UseSignature;
   BOOL  FixedFontList;
   BOOL  SwatchBeat;
   BOOL  EncryptToSelf;
   BOOL  SplitLogfile;
   BOOL  LogAllEvents;
   BOOL  GetOnStartup;
   BOOL  SendOnStartup;
   BOOL  CleanupOnStartup;
   BOOL  RemoveOnStartup;
   BOOL  LoadAllFolders;
   BOOL  UpdateNewMail;
   BOOL  CheckBirthdates;
   BOOL  SendOnQuit;
   BOOL  CleanupOnQuit;
   BOOL  RemoveOnQuit;
   BOOL  IdentifyBin;
   BOOL  AddMyInfo;
   BOOL  UseCManager;
   BOOL  IconifyOnQuit;
   BOOL  Confirm;
   BOOL  RemoveAtOnce;
   BOOL  SaveSent;
   BOOL  SendMDNAtOnce;
   BOOL  JumpToNewMsg;
   BOOL  PrinterCheck;
   BOOL  IsOnlineCheck;
   BOOL  ConfirmOnQuit;

   struct MUI_PenSpec ColoredText;
   struct MUI_PenSpec Color2ndLevel;
   struct RxHook      RX[MAXRX];

   char  RealName[SIZE_REALNAME];
   char  EmailAddress[SIZE_ADDRESS];
   char  SMTP_Server[SIZE_HOST];
   char  SMTP_Domain[SIZE_HOST];
   char  SMTP_AUTH_User[SIZE_USERID];
   char  SMTP_AUTH_Pass[SIZE_USERID];
   char  NotifySound[SIZE_PATHFILE];
   char  NotifyCommand[SIZE_COMMAND];
   char  ShortHeaders[SIZE_PATTERN];
   char  TranslationIn[SIZE_PATHFILE];
   char  ReplyTo[SIZE_ADDRESS];
   char  Organization[SIZE_DEFAULT];
   char  ExtraHeaders[SIZE_LARGE];
   char  NewIntro[SIZE_INTRO];
   char  Greetings[SIZE_INTRO];
   char  TranslationOut[SIZE_PATHFILE];
   char  Editor[SIZE_PATHFILE];
   char  ReplyHello[SIZE_INTRO];
   char  ReplyIntro[SIZE_INTRO];
   char  ReplyBye[SIZE_INTRO];
   char  AltReplyHello[SIZE_INTRO];
   char  AltReplyIntro[SIZE_INTRO];
   char  AltReplyBye[SIZE_INTRO];
   char  AltReplyPattern[SIZE_PATTERN];
   char  MLReplyHello[SIZE_INTRO];
   char  MLReplyIntro[SIZE_INTRO];
   char  MLReplyBye[SIZE_INTRO];
   char  ForwardIntro[SIZE_INTRO];
   char  ForwardFinish[SIZE_INTRO];
   char  QuoteText[SIZE_SMALL];
   char  AltQuoteText[SIZE_SMALL];
   char  TagsFile[SIZE_PATHFILE];
   char  TagsSeparator[SIZE_SMALL];
   char  PGPCmdPath[SIZE_PATH];
   char  MyPGPID[SIZE_DEFAULT];
   char  ReMailer[SIZE_ADDRESS];
   char  RMCommands[SIZE_INTRO];
   char  LogfilePath[SIZE_PATH];
   char  DetachDir[SIZE_PATH];
   char  AttachDir[SIZE_PATH];
   char  GalleryDir[SIZE_PATH];
   char  MyPictureURL[SIZE_URL];
   char  NewAddrGroup[SIZE_NAME];
   char  ProxyServer[SIZE_HOST];
   char  TempDir[SIZE_PATH];
   char  PackerCommand[SIZE_COMMAND];
   char  XPKPack[5];
   char  XPKPackEncrypt[5];
   char  SupportSite[SIZE_HOST];
   char  LocalCharset[SIZE_CTYPE];
   char  IOCInterface[SIZE_SMALL];
};

/*** Global Structure ***/
struct Global 
{
   /* pointers first */
   APTR                     App;
   APTR                     AY_Win;
   APTR                     AY_Text;
   APTR                     AY_Group;
   APTR                     AY_List;
   APTR                     AY_Button;
   APTR                     AY_AboutText;
   char *                   ER_Message[MAXERR];
   struct AppIcon *         AppIcon;
   struct MsgPort *         AppPort;
   struct RexxHost *        RexxHost;
   struct DiskObject *      DiskObj[MAXICONS];
   struct BodyChunkData *   BImage[MAXIMAGES];
   struct FileRequester *   ASLReq[MAXASL];
   struct Locale *          Locale;
   struct TranslationTable *TTin;
   struct TranslationTable *TTout;
   struct MA_ClassData *    MA;
   struct CO_ClassData *    CO;
   struct AB_ClassData *    AB;
   struct EA_ClassData *    EA[MAXEA];
   struct RE_ClassData *    RE[MAXRE+1];
   struct WR_ClassData *    WR[MAXWR+1];
   struct TR_ClassData *    TR;
   struct ER_ClassData *    ER;
   struct FI_ClassData *    FI;
   struct FO_ClassData *    FO;
   struct DI_ClassData *    DI;
   struct US_ClassData *    US;

   long                     EdColMap[9];
   long                     Weights[6];
   long                     ActiveReadWin;
   long                     ActiveWriteWin;

   int                      TotMsgs;
   int                      UnrMsgs;
   int                      NewMsgs;
   int                      GM_Count;
   int                      SI_Count;
   int                      PGPVersion;
   int                      CO_DST;
   int                      TR_Socket;
   int                      ER_NumErr;

   BOOL                     Error;
   BOOL                     PGP5;
   BOOL                     DtpicSupported;
   BOOL                     AppIconQuiet;
   BOOL                     PGPPassVolatile;
   BOOL                     CO_Valid;
   BOOL                     TR_Debug;
   BOOL                     TR_Allow;
   BOOL                     TR_Exchange;

   struct DateStamp         StartDate;
   struct Users             Users;
   struct RuleResult        RRs;
   struct DownloadResult    LastDL;
   struct NotifyRequest     WR_NRequest[MAXWR+1];
   struct sockaddr_in       TR_INetSocketAddr;

   char                     ProgDir[SIZE_PATH];
   char                     PGPPassPhrase[SIZE_DEFAULT];
   char                     MA_MailDir[SIZE_PATH];
   char                     AB_Filename[SIZE_PATHFILE];
   char                     CO_PrefsFile[SIZE_PATHFILE];
   char                     WR_Filename[3][SIZE_PATHFILE];
   char                     DI_Filename[SIZE_PATHFILE];
};

/**************************************************************************/

extern struct Config *C;
extern struct Global *G;

/**************************************************************************/

/* old stuff to get old method working */
#include "old.h"

#endif /* YAM_H */
