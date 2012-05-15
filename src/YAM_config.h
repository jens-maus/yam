#ifndef YAM_CONFIG_H
#define YAM_CONFIG_H

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
  Object *ST_REALNAME;
  Object *ST_EMAIL;
  Object *ST_POPHOST0;
  Object *ST_USER0;
  Object *ST_PASSWD0;
  Object *CY_TZONE;
  Object *CH_DSTACTIVE;
  Object *ST_SMTPHOST;
  Object *ST_SMTPPORT;
  Object *CH_SMTP8BIT;
  Object *ST_SMTPAUTHUSER;
  Object *ST_SMTPAUTHPASS;
  Object *CY_SMTPAUTH;
  Object *LV_SMTP;
  Object *BT_SADD;
  Object *BT_SDEL;
  Object *BT_SMTPUP;
  Object *BT_SMTPDOWN;
  Object *LV_POP3;
  Object *BT_PADD;
  Object *BT_PDEL;
  Object *BT_POPUP;
  Object *BT_POPDOWN;
  Object *ST_POPDESC;
  Object *ST_SMTPDESC;
  Object *ST_POPHOST;
  Object *ST_POPPORT;
  Object *ST_POPUSERID;
  Object *ST_PASSWD;
  Object *CH_DELETE;
  Object *CY_POPAUTH;
  Object *CY_POPSECURE;
  Object *CH_POPENABLED;
  Object *CH_SMTPENABLED;
  Object *CH_DOWNLOADONSTARTUP;
  Object *CH_APPLYREMOTEFILTERS;
  Object *CY_PRESELECTION;
  Object *CY_TRANSWIN;
  Object *CH_UPDSTAT;
  Object *ST_WARNSIZE;
  Object *CH_INTERVAL;
  Object *NM_INTERVAL;
  Object *CH_DLLARGE;
  Object *CH_NOTIREQ;
  Object *CH_NOTISOUND;
  Object *CH_NOTICMD;
  Object *ST_NOTISOUND;
  Object *ST_NOTICMD;
  Object *LV_RULES;
  Object *BT_RADD;
  Object *BT_RDEL;
  Object *ST_RNAME;
  Object *CH_REMOTE;
  Object *GR_RGROUP;
  Object *GR_SGROUP;
  Object *PO_MOVETO;
  Object *TX_MOVETO;
  Object *LV_MOVETO;
  Object *CH_APPLYNEW;
  Object *CH_APPLYREQ;
  Object *CH_APPLYSENT;
  Object *CH_ABOUNCE;
  Object *ST_ABOUNCE;
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
  Object *CH_ADELETE;
  Object *CH_ASKIP;
  Object *CY_HEADER;
  Object *ST_HEADERS;
  Object *CY_SENDERINFO;
  Object *CY_SIGSEPLINE;
  Object *CA_COLTEXT;
  Object *CA_COL1QUOT;
  Object *CA_COL2QUOT;
  Object *CA_COL3QUOT;
  Object *CA_COL4QUOT;
  Object *CA_COLURL;
  Object *CH_FIXFEDIT;
  Object *CH_ALLTEXTS;
  Object *CH_MULTIWIN;
  Object *CH_WRAPHEAD;
  Object *CH_TEXTSTYLES_READ;
  Object *ST_HELLOTEXT;
  Object *ST_BYETEXT;
  Object *ST_EDWRAP;
  Object *CY_EDWRAP;
  Object *ST_EDITOR;
  Object *CH_LAUNCH;
  Object *ST_REPLYHI;
  Object *ST_REPLYTEXT;
  Object *ST_REPLYBYE;
  Object *ST_AREPLYHI;
  Object *ST_AREPLYTEXT;
  Object *ST_AREPLYBYE;
  Object *ST_AREPLYPAT;
  Object *ST_MREPLYHI;
  Object *ST_MREPLYTEXT;
  Object *ST_MREPLYBYE;
  Object *CH_QUOTEEMPTY;
  Object *CH_COMPADDR;
  Object *CH_STRIPSIG;
  Object *ST_FWDSTART;
  Object *ST_FWDEND;
  Object *CY_SIGNAT;
  Object *BT_SIGEDIT;
  Object *TE_SIGEDIT;
  Object *BT_INSTAG;
  Object *BT_INSENV;
  Object *ST_TAGFILE;
  Object *ST_TAGSEP;
  Object *CH_FCOLS[FOCOLNUM];
  Object *CH_MCOLS[MACOLNUM];
  Object *CH_FIXFLIST;
  Object *CH_BEAT;
  Object *CY_SIZE;
  Object *ST_PGPCMD;
  Object *PO_LOGFILE;
  Object *ST_LOGFILE;
  Object *CY_LOGMODE;
  Object *CH_SPLITLOG;
  Object *CH_LOGALL;
  Object *CH_SENDSTART;
  Object *CH_DELETESTART;
  Object *CH_REMOVESTART;
  Object *CH_LOADALL;
  Object *CH_MARKNEW;
  Object *CH_CHECKBD;
  Object *CH_SENDQUIT;
  Object *CH_DELETEQUIT;
  Object *CH_REMOVEQUIT;
  Object *LV_MIME;
  Object *GR_MIME;
  Object *ST_CTYPE;
  Object *ST_EXTENS;
  Object *ST_DESCRIPTION;
  Object *ST_COMMAND;
  Object *ST_DEFVIEWER;
  Object *BT_MADD;
  Object *BT_MDEL;
  Object *BT_MIMEUP;
  Object *BT_MIMEDOWN;
  Object *ST_DETACHDIR;
  Object *ST_ATTACHDIR;
  Object *ST_GALLDIR;
  Object *ST_PROXY;
  Object *CY_ATAB;
  Object *ST_NEWGROUP;
  Object *CH_ACOLS[ABCOLNUM];
  Object *LV_REXX;
  Object *ST_RXNAME;
  Object *ST_SCRIPT;
  Object *PO_SCRIPT;
  Object *CY_ISADOS;
  Object *CH_CONSOLE;
  Object *CH_WAITTERM;
  Object *ST_TEMPDIR;
  Object *ST_APPX;
  Object *ST_APPY;
  Object *CH_CLGADGET;
  Object *CH_CONFIRM;
  Object *NB_CONFIRMDEL;
  Object *CH_REMOVE;
  Object *TX_PACKER;
  Object *TX_ENCPACK;
  Object *NB_PACKER;
  Object *NB_ENCPACK;
  Object *ST_ARCHIVER;
  Object *ST_APPICON;
  Object *CH_FCNTMENU;
  Object *CH_MCNTMENU;
  Object *CY_INFOBAR;
  Object *PO_INFOBARTXT;
  Object *ST_INFOBARTXT;
  Object *CH_WARNSUBJECT;
  Object *NB_EMAILCACHE;
  Object *NB_AUTOSAVE;
  Object *CH_AUTOTRANSLATEIN;
  Object *CY_SMTPSECURE;
  Object *CH_EMBEDDEDREADPANE;
  Object *CH_DELAYEDSTATUS;
  Object *NB_DELAYEDSTATUS;
  Object *BT_FILTERUP;
  Object *BT_FILTERDOWN;
  Object *BT_MORE;
  Object *BT_LESS;
  Object *CH_QUICKSEARCHBAR;
  Object *CH_WBAPPICON;
  Object *CH_DOCKYICON;
  Object *TX_DEFCHARSET_READ;
  Object *TX_DEFCHARSET_WRITE;
  Object *CY_UPDATEINTERVAL;
  Object *BT_UPDATENOW;
  Object *TX_UPDATESTATUS;
  Object *TX_UPDATEDATE;
  Object *ST_UPDATEDOWNLOADPATH;
  Object *CH_DETECTCYRILLIC;
  Object *CH_ABOOKLOOKUP;
  Object *CH_CONVERTHTML;
  Object *CH_PGPPASSINTERVAL;
  Object *NB_PGPPASSINTERVAL;
  Object *CH_SPAMFILTERENABLED;
  Object *CH_SPAMFILTERFORNEWMAIL;
  Object *CH_SPAMMARKONMOVE;
  Object *CH_SPAMMARKASREAD;
  Object *CH_SPAMABOOKISWHITELIST;
  Object *CH_MOVEHAMTOINCOMING;
  Object *CH_FILTERHAM;
  Object *BT_SPAMRESETTRAININGDATA;
  Object *BT_OPTIMIZETRAININGDATA;
  Object *TX_SPAMGOODCOUNT;
  Object *TX_SPAMBADCOUNT;
  Object *CH_MDN_NEVER;
  Object *CH_MDN_ALLOW;
  Object *CY_MDN_NORECIPIENT;
  Object *CY_MDN_NODOMAIN;
  Object *CY_MDN_DELETE;
  Object *CY_MDN_OTHER;
  Object *CH_RELDATETIME;
  Object *CA_COLSIG;
  Object *GR_THEMES;
  Object *CH_SHOWALTPARTS;
  Object *CH_APPICONPOS;
  Object *BT_APPICONGETPOS;
  Object *CY_FOLDERINFO;
  Object *CY_FORWARDMODE;
  Object *BT_MIMEIMPORT;
  Object *CH_FOLDERDBLCLICK;
  Object *CH_MAPFOREIGNCHARS;
  Object *CH_GLOBALMAILTHREADS;
  Object *CH_TEXTCOLORS_READ;
  Object *CH_FIXEDFONT_WRITE;
  Object *CH_TEXTSTYLES_WRITE;
  Object *CH_TEXTCOLORS_WRITE;
  Object *CH_NOTIOS41SYSTEM;
  Object *CH_ASTATUSTOMARKED;
  Object *CH_ASTATUSTOUNMARKED;
  Object *CH_ASTATUSTOREAD;
  Object *CH_ASTATUSTOUNREAD;
  Object *CH_ASTATUSTOSPAM;
  Object *CH_ASTATUSTOHAM;
  Object *LV_IDENTITY;
  Object *BT_IADD;
  Object *BT_IDEL;
  Object *BT_IDENTITYUP;
  Object *BT_IDENTITYDOWN;
  Object *CH_IDENTITY_ENABLED;
  Object *ST_IDENTITY_DESCRIPTION;
  Object *ST_IDENTITY_REALNAME;
  Object *ST_IDENTITY_EMAIL;
  Object *ST_IDENTITY_REPLYTO;
  Object *ST_IDENTITY_CC;
  Object *ST_IDENTITY_BCC;
  Object *ST_IDENTITY_ORGANIZATION;
  Object *CY_IDENTITY_MAILSERVER;
  Object *CY_IDENTITY_SIGNATURE;
  Object *ST_IDENTITY_EXTRAHEADER;
  Object *ST_IDENTITY_PHOTOURL;
  Object *CH_IDENTITY_SENTFOLDER;
  Object *CH_IDENTITY_QUOTEMAILS;
  Object *CY_IDENTITY_QUOTEPOS;
  Object *CY_IDENTITY_SIGPOS;
  Object *CH_IDENTITY_SIGREPLY;
  Object *CH_IDENTITY_SIGFORWARD;
  Object *CH_IDENTITY_ADDINFO;
  Object *CH_IDENTITY_REQUESTMDN;
  Object *CH_IDENTITY_USEPGP;
  Object *ST_IDENTITY_PGPID;
  Object *ST_IDENTITY_PGPURL;
  Object *CH_IDENTITY_PGPSIGN_UNENC;
  Object *CH_IDENTITY_PGPSIGN_ENC;
  Object *CH_IDENTITY_PGPENC_ALL;
  Object *CH_IDENTITY_PGPENC_SELF;
  Object *PO_IDENTITY_SENTFOLDER;
  Object *TX_IDENTITY_SENTFOLDER;
  Object *LV_IDENTITY_SENTFOLDER;
};

struct CO_ClassData  /* configuration window */
{
  struct CO_GUIData GUI;
  enum ConfigPage VisiblePage;
  int  LastSig;
  BOOL Visited[cp_Max];
  BOOL UpdateAll;
  char **smtpServerArray; // NUL-terminated array of smtpServer names
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

// notify flags for the notifiying method for new messages
#define NOTIFY_REQ        (1<<0)
#define NOTIFY_SOUND      (1<<1)
#define NOTIFY_CMD        (1<<2)
#define NOTIFY_OS41SYSTEM (1<<3)
#define hasRequesterNotify(f)  (isFlagSet((f), NOTIFY_REQ))
#define hasSoundNotify(f)      (isFlagSet((f), NOTIFY_SOUND))
#define hasCommandNotify(f)    (isFlagSet((f), NOTIFY_CMD))
#define hasOS41SystemNotify(f) (isFlagSet((f), NOTIFY_OS41SYSTEM))

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
  IB_POS_TOP=0,
  IB_POS_CENTER,
  IB_POS_BOTTOM,
  IB_POS_OFF
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
  struct MinList mailServerList;   // list of configured mail servers (SMTP/POP3/IMAP)
  struct MinList filterList;       // list of currently available filter node
  struct MinList mimeTypeList;     // list of user defined MIME types
  struct MinList userIdentityList; // list of user identities

  int   TimeZone;
  int   WarnSize;
  int   CheckMailDelay;
  int   NotifyType;
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
  enum  InfoBarPos         InfoBar;
  enum  WrapMode           EdWrapMode;

  BOOL  DaylightSaving;
  BOOL  UpdateStatus;
  BOOL  DownloadLarge;
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
  BOOL  QuickSearchBar;
  BOOL  WBAppIcon;
  BOOL  DockyIcon;
  BOOL  AmiSSLCheck;
  BOOL  TimeZoneCheck;
  BOOL  AutoDSTCheck;
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

  char NotifySound[SIZE_PATHFILE];
  char NotifyCommand[SIZE_COMMAND];
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
  char DefaultReadCharset[SIZE_CTYPE+1];
  char DefaultWriteCharset[SIZE_CTYPE+1];
  char IOCInterfaces[SIZE_LARGE];
  char AppIconText[SIZE_COMMAND];
  char InfoBarText[SIZE_COMMAND];
  char DefaultMimeViewer[SIZE_COMMAND];
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
};

extern struct Config *C;
extern struct Config *CE;

// external hooks
extern struct Hook CO_EditSignatHook;
extern struct Hook CO_SwitchSpamFilterHook;
extern struct Hook CO_GetDefaultPOPHook;
extern struct Hook CO_GetPOP3EntryHook;
extern struct Hook CO_PutPOP3EntryHook;
extern struct Hook CO_GetSMTPEntryHook;
extern struct Hook CO_PutSMTPEntryHook;
extern struct Hook CO_GetIdentityEntryHook;
extern struct Hook CO_PutIdentityEntryHook;
extern struct Hook CO_OpenHook;
extern struct Hook CO_PL_DspFuncHook;
extern struct Hook CO_RemoteToggleHook;
extern struct Hook SetActiveFilterDataHook;
extern struct Hook GetActiveFilterDataHook;
extern struct Hook AddNewRuleToListHook;
extern struct Hook RemoveLastRuleHook;

void CO_ClearConfig(struct Config *co);
BOOL CO_IsValid(void);
void CO_SetDefaults(struct Config *co, enum ConfigPage page);
void CO_Validate(struct Config *co, BOOL update);
void CO_UpdateSMTPServerArray(struct CO_ClassData *data);

void GhostOutFilter(struct CO_GUIData *gui, struct FilterNode *filter);

#endif /* YAM_CONFIG_H */
