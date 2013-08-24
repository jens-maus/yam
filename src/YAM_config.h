#ifndef YAM_CONFIG_H
#define YAM_CONFIG_H

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

#include <libraries/mui.h>

#include "YAM_read.h"
#include "YAM_utilities.h"

#include "Logfile.h"
#include "UpdateCheck.h"

#include "mui/ConfigPageList.h"
#include "mui/TransferWindow.h"
#include "tcp/Connection.h"

// forward declarations
struct FilterNode;

#define FOCOLNUM 5
#define MACOLNUM 8  // the maximum number of columns the MessageListview can have
#define ABCOLNUM 9

struct CO_GUIData
{
  Object *WI;
  Object *BT_SAVE;
  Object *BT_USE;
  Object *BT_CANCEL;
  Object *NLV_PAGE;
  Object *LV_PAGE;
  Object *GR_PAGE;
  Object *PG_PAGES[cp_Max];

  // objects on "Filters" page
  Object *ST_POPHOST0;
  Object *ST_USER0;
  Object *LV_RULES;
  Object *BT_RADD;
  Object *BT_RDEL;
  Object *BT_FILTERUP;
  Object *BT_FILTERDOWN;
  Object *BT_FILTER_IMPORT;
  Object *ST_RNAME;
  Object *CH_REMOTE;
  Object *CH_APPLYNEW;
  Object *CH_APPLYSENT;
  Object *CH_APPLYREQ;
  Object *CY_FILTER_COMBINE;
  Object *GR_SGROUP;
  Object *CH_AREDIRECT;
  Object *ST_AREDIRECT;
  Object *CH_AFORWARD;
  Object *ST_AFORWARD;
  Object *CH_ARESPONSE;
  Object *PO_ARESPONSE;
  Object *ST_ARESPONSE;
  Object *CH_AEXECUTE;
  Object *PO_AEXECUTE;
  Object *ST_AEXECUTE;
  Object *CH_APLAY;
  Object *PO_APLAY;
  Object *ST_APLAY;
  Object *BT_APLAY;
  Object *CH_AMOVE;
  Object *PO_MOVETO;
  Object *TX_MOVETO;
  Object *LV_MOVETO;
  Object *CH_ASTATUSTOMARKED;
  Object *CH_ASTATUSTOUNMARKED;
  Object *CH_ASTATUSTOREAD;
  Object *CH_ASTATUSTOUNREAD;
  Object *CH_ASTATUSTOSPAM;
  Object *CH_ASTATUSTOHAM;
  Object *CH_ADELETE;
  Object *CH_ASKIP;
  Object *CH_ATERMINATE;

  // Objects on "Signature" page, unused but still accessed
  Object *TE_SIGEDIT;
};

struct CO_ClassData  /* configuration window */
{
  struct CO_GUIData GUI;
  enum ConfigPage VisiblePage;
  BOOL Visited[cp_Max];
  BOOL UpdateAll;
};

/*** RxHook structure ***/
struct RxHook
{
  BOOL  IsAmigaDOS;
  BOOL  UseConsole;
  BOOL  WaitTerm;
  char  Name[SIZE_NAME];
  char  Script[SIZE_PATHFILE];
};

// flags for hiding GUI elements
#define HIDE_INFO    (1<<0)
#define HIDE_XY      (1<<1)
#define HIDE_TBAR    (1<<2)
#define hasHideInfoFlag(f)    (isFlagSet((f), HIDE_INFO))
#define hasHideXYFlag(f)      (isFlagSet((f), HIDE_XY))
#define hasHideToolBarFlag(f) (isFlagSet((f), HIDE_TBAR))

enum PrintMethod
{
  PRINTMETHOD_RAW
};

enum FolderInfoMode
{
  FIM_NAME_ONLY = 0,
  FIM_NAME_AND_NEW_MAILS,
  FIM_NAME_AND_UNREAD_MAILS,
  FIM_NAME_AND_NEW_UNREAD_MAILS,
  FIM_NAME_AND_UNREAD_NEW_MAILS
};

enum InfoBarPos
{
  IB_POS_OFF = 0,
  IB_POS_TOP,
  IB_POS_CENTER,
  IB_POS_BOTTOM,
};

enum QuickSearchBarPos
{
  QSB_POS_OFF = 0,
  QSB_POS_TOP,
  QSB_POS_BOTTOM,
};

enum WrapMode
{
  EWM_OFF=0,   // no word wrapping at all
  EWM_EDITING, // word wrapping while editing
  EWM_ONSENT   // word wrapping before sent
};

/*** Configuration main structure ***/
struct Config
{
  struct MinList pop3ServerList;   // list of configured POP3 servers
  struct MinList smtpServerList;   // list of configured SMTP servers
  struct MinList filterList;       // list of currently available filter node
  struct MinList mimeTypeList;     // list of user defined MIME types
  struct MinList userIdentityList; // list of user identities
  struct MinList signatureList;    // list of signatures

  int   ShowHeader;
  int   ShowSenderInfo;
  int   EdWrapCol;
  int   FolderCols;
  int   MessageCols;
  int   AddToAddrbook;
  int   AddrbookCols;
  int   IconPositionX;
  int   IconPositionY;
  int   ConfirmDelete;
  int   XPKPackEff;
  int   XPKPackEncryptEff;
  int   LetterPart;
  int   WriteIndexes;
  int   ExpungeIndexes;
  int   AutoSave;
  int   HideGUIElements;
  int   StackSize;
  int   SizeFormat;
  int   EmailCache;
  int   TRBufferSize;
  int   EmbeddedMailDelay;
  int   StatusChangeDelay;
  int   KeepAliveInterval;
  int   UpdateInterval;
  int   PGPPassInterval;
  int   SpamProbabilityThreshold;
  int   SpamFlushTrainingDataInterval;
  int   SpamFlushTrainingDataThreshold;
  int   SocketTimeout;

  enum  PrintMethod        PrintMethod;
  enum  LFMode             LogfileMode;
  enum  MDNAction          MDN_NoRecipient;
  enum  MDNAction          MDN_NoDomain;
  enum  MDNAction          MDN_OnDelete;
  enum  MDNAction          MDN_Other;
  enum  DateStampType      DSListFormat;
  enum  SigSepType         SigSepLine;
  enum  TransferWindowMode TransferWindow;
  enum  FolderInfoMode     FolderInfoMode;
  enum  ForwardMode        ForwardMode;
  enum  InfoBarPos         InfoBarPos;
  enum  QuickSearchBarPos  QuickSearchBarPos;
  enum  WrapMode           EdWrapMode;

  BOOL  DisplayAllTexts;
  BOOL  FixedFontEdit;
  BOOL  MultipleReadWindows;
  BOOL  UseTextStylesRead;
  BOOL  UseTextColorsRead;
  BOOL  UseTextStylesWrite;
  BOOL  UseTextColorsWrite;
  BOOL  UseFixedFontWrite;
  BOOL  WrapHeader;
  BOOL  LaunchAlways;
  BOOL  QuoteEmptyLines;
  BOOL  CompareAddress;
  BOOL  StripSignature;
  BOOL  FixedFontList;
  BOOL  SplitLogfile;
  BOOL  LogAllEvents;
  BOOL  SendOnStartup;
  BOOL  CleanupOnStartup;
  BOOL  RemoveOnStartup;
  BOOL  LoadAllFolders;
  BOOL  UpdateNewMail;
  BOOL  CheckBirthdates;
  BOOL  SendOnQuit;
  BOOL  CleanupOnQuit;
  BOOL  RemoveOnQuit;
  BOOL  IconifyOnQuit;
  BOOL  Confirm;
  BOOL  RemoveAtOnce;
  BOOL  JumpToNewMsg;
  BOOL  JumpToIncoming;
  BOOL  JumpToRecentMsg;
  BOOL  PrinterCheck;
  BOOL  IsOnlineCheck;
  BOOL  ConfirmOnQuit;
  BOOL  AskJumpUnread;
  BOOL  WarnSubject;
  BOOL  FolderCntMenu;
  BOOL  MessageCntMenu;
  BOOL  AutoColumnResize;
  BOOL  EmbeddedReadPane;
  BOOL  StatusChangeDelayOn;
  BOOL  SysCharsetCheck;
  BOOL  WBAppIcon;
  BOOL  DockyIcon;
  BOOL  AmiSSLCheck;
  BOOL  DetectCyrillic;
  BOOL  ABookLookup;
  BOOL  ConvertHTML;
  BOOL  SpamFilterEnabled;
  BOOL  SpamFilterForNewMail;
  BOOL  SpamMarkOnMove;
  BOOL  SpamMarkAsRead;
  BOOL  SpamAddressBookIsWhiteList;
  BOOL  MoveHamToIncoming;
  BOOL  FilterHam;
  BOOL  SpamTrustExternalFilter;
  BOOL  DisplayAllAltPart;
  BOOL  MDNEnabled;
  BOOL  ConfigIsSaved;
  BOOL  AutoClip;
  BOOL  FolderDoubleClick;
  BOOL  MapForeignChars;
  BOOL  GlobalMailThreads;
  BOOL  ShowFilterStats;
  BOOL  ConfirmRemoveAttachments;
  BOOL  ShowRcptFieldCC;
  BOOL  ShowRcptFieldBCC;
  BOOL  ShowRcptFieldReplyTo;
  BOOL  OverrideFromAddress;
  BOOL  ShowPackerProgress;

  struct MUI_PenSpec   ColoredText;
  struct MUI_PenSpec   Color1stLevel;
  struct MUI_PenSpec   Color2ndLevel;
  struct MUI_PenSpec   Color3rdLevel;
  struct MUI_PenSpec   Color4thLevel;
  struct MUI_PenSpec   ColorURL;
  struct MUI_PenSpec   ColorSignature;
  struct RxHook        RX[MAXRX];
  struct SocketOptions SocketOptions;
  struct DateStamp     BirthdayCheckTime;

  char ShortHeaders[SIZE_PATTERN];
  char NewIntro[SIZE_INTRO];
  char Greetings[SIZE_INTRO];
  char Editor[SIZE_PATHFILE];
  char ReplyHello[SIZE_INTRO];
  char ReplyIntro[SIZE_INTRO];
  char ReplyBye[SIZE_INTRO];
  char AltReplyHello[SIZE_INTRO];
  char AltReplyIntro[SIZE_INTRO];
  char AltReplyBye[SIZE_INTRO];
  char AltReplyPattern[SIZE_PATTERN];
  char MLReplyHello[SIZE_INTRO];
  char MLReplyIntro[SIZE_INTRO];
  char MLReplyBye[SIZE_INTRO];
  char ForwardIntro[SIZE_INTRO];
  char ForwardFinish[SIZE_INTRO];
  char TagsFile[SIZE_PATHFILE];
  char TagsSeparator[SIZE_SMALL];
  char PGPCmdPath[SIZE_PATH];
  char LogfilePath[SIZE_PATH];
  char DetachDir[SIZE_PATH];
  char AttachDir[SIZE_PATH];
  char GalleryDir[SIZE_PATH];
  char NewAddrGroup[SIZE_NAME];
  char ProxyServer[SIZE_HOST];
  char TempDir[SIZE_PATH];
  char PackerCommand[SIZE_COMMAND];
  char XPKPack[5];
  char XPKPackEncrypt[5];
  char SupportSite[SIZE_HOST];
  char UpdateServer[SIZE_HOST];
  char DefaultLocalCodeset[SIZE_CTYPE+1];
  char DefaultWriteCodeset[SIZE_CTYPE+1];
  char DefaultEditorCodeset[SIZE_CTYPE+1];
  char IOCInterfaces[SIZE_LARGE];
  char AppIconText[SIZE_COMMAND];
  char InfoBarText[SIZE_COMMAND];
  char DefaultMimeViewer[SIZE_COMMAND];
  char DefaultMimeViewerCodesetName[SIZE_CTYPE+1];
  char StyleFGroupUnread[SIZE_SMALL];
  char StyleFGroupRead[SIZE_SMALL];
  char StyleFolderUnread[SIZE_SMALL];
  char StyleFolderRead[SIZE_SMALL];
  char StyleFolderNew[SIZE_SMALL];
  char StyleMailUnread[SIZE_SMALL];
  char StyleMailRead[SIZE_SMALL];
  char QuoteChar[2];
  char AltQuoteChar[2];
  char ThemeName[SIZE_FILE];
  char UpdateDownloadPath[SIZE_PATH];
  char DefaultSSLCiphers[SIZE_DEFAULT];
  char SpamExternalFilter[SIZE_FILE];
  char MachineFQDN[SIZE_DEFAULT];
  char Location[SIZE_DEFAULT];
};

extern struct Config *C;
extern struct Config *CE;

// external hooks
extern struct Hook CO_PlaySoundHook;
extern struct Hook CO_OpenHook;
extern struct Hook CO_RemoteToggleHook;
extern struct Hook SetActiveFilterDataHook;
extern struct Hook GetActiveFilterDataHook;
extern struct Hook ImportFilterHook;

void CO_ClearConfig(struct Config *co);
BOOL CO_IsValid(void);
void CO_SetDefaults(struct Config *co, enum ConfigPage page);
void CO_Validate(struct Config *co, BOOL update);

void GhostOutFilter(struct CO_GUIData *gui, struct FilterNode *filter);

void ImportExternalSpamFilters(struct Config *co);

#endif /* YAM_CONFIG_H */
