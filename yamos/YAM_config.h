#ifndef YAM_CONFIG_H
#define YAM_CONFIG_H

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

#include <libraries/mui.h>

#include "YAM_find.h"

#define FOCOLNUM 5
#define MACOLNUM 7
#define ABCOLNUM 9

struct CO_GUIData
{
   APTR WI;
   APTR BT_SAVE;
   APTR BT_USE;
   APTR BT_CANCEL;
   APTR MI_IMPMIME;
   APTR LV_PAGE;
   APTR GR_PAGE;
   APTR GR_SUBPAGE;
   APTR ST_REALNAME;
   APTR ST_EMAIL;
   APTR ST_POPHOST0;
   APTR ST_PASSWD0;
   APTR CY_TZONE;
   APTR CH_DLSAVING;
   APTR ST_SMTPHOST;
   APTR ST_SMTPPORT;
   APTR ST_DOMAIN;
   APTR CH_SMTP8BIT;
   APTR CH_SMTPTLS;
   APTR CH_USESMTPAUTH;
   APTR ST_SMTPAUTHUSER;
   APTR ST_SMTPAUTHPASS;
   APTR LV_POP3;
   APTR GR_POP3;
   APTR BT_PADD;
   APTR BT_PDEL;
   APTR ST_POPHOST;
   APTR ST_POPPORT;
   APTR ST_POPUSERID;
   APTR ST_PASSWD;
   APTR CH_DELETE;
   APTR CH_USEAPOP;
   APTR CH_POP3SSL;
   APTR CH_USESTLS;
   APTR CH_POPENABLED;
   APTR CH_AVOIDDUP;
   APTR CY_MSGSELECT;
   APTR CY_TRANSWIN;
   APTR CH_UPDSTAT;
   APTR ST_WARNSIZE;
   APTR NM_INTERVAL;
   APTR CH_DLLARGE;
   APTR CH_NOTIREQ;
   APTR CH_NOTISOUND;
   APTR CH_NOTICMD;
   APTR ST_NOTISOUND;
   APTR ST_NOTICMD;
   APTR LV_RULES;
   APTR BT_RADD;
   APTR BT_RDEL;
   APTR ST_RNAME;
   APTR CH_REMOTE;
   APTR CY_COMBINE[2];
   APTR GR_LRGROUP;
   APTR GR_LOCAL;
   APTR GR_REMOTE;
   APTR PO_MOVETO;
   APTR TX_MOVETO;
   APTR LV_MOVETO;
   APTR CH_APPLYNEW;
   APTR CH_APPLYREQ;
   APTR CH_APPLYSENT;
   APTR CH_ABOUNCE;
   APTR ST_ABOUNCE;
   APTR CH_AFORWARD;
   APTR ST_AFORWARD;
   APTR CH_ARESPONSE;
   APTR ST_ARESPONSE;
   APTR CH_AEXECUTE;
   APTR ST_AEXECUTE;
   APTR CH_APLAY;
   APTR ST_APLAY;
   APTR CH_AMOVE;
   APTR CH_ADELETE;
   APTR CH_ASKIP;
   struct SearchGroup GR_SEARCH[4];
   APTR CY_HEADER;
   APTR ST_HEADERS;
   APTR CY_SENDERINFO;
   APTR CY_SIGSEPLINE;
   APTR CA_COLTEXT;
   APTR CA_COL2QUOT;
   APTR CH_FIXFEDIT;
   APTR CH_ALLTEXTS;
   APTR CH_MULTIWIN;
   APTR CH_WRAPHEAD;
   APTR CH_TEXTSTYLES;
   APTR ST_INTRANS;
   APTR ST_REPLYTO;
   APTR ST_ORGAN;
   APTR ST_EXTHEADER;
   APTR ST_HELLOTEXT;
   APTR ST_BYETEXT;
   APTR ST_OUTTRANS;
   APTR ST_EDWRAP;
   APTR CY_EDWRAP;
   APTR ST_EDITOR;
   APTR CH_LAUNCH;
   APTR ST_REPLYHI;
   APTR ST_REPLYTEXT;
   APTR ST_REPLYBYE;
   APTR ST_AREPLYHI;
   APTR ST_AREPLYTEXT;
   APTR ST_AREPLYBYE;
   APTR ST_AREPLYPAT;
   APTR ST_MREPLYHI;
   APTR ST_MREPLYTEXT;
   APTR ST_MREPLYBYE;
   APTR CH_QUOTE;
   APTR ST_REPLYCHAR;
   APTR ST_ALTQUOTECHAR;
   APTR CH_QUOTEEMPTY;
   APTR CH_COMPADDR;
   APTR CH_STRIPSIG;
   APTR ST_FWDSTART;
   APTR ST_FWDEND;
   APTR CH_USESIG;
   APTR CY_SIGNAT;
   APTR BT_SIGEDIT;
   APTR TE_SIGEDIT;
   APTR BT_INSTAG;
   APTR BT_INSENV;
   APTR ST_TAGFILE;
   APTR ST_TAGSEP;
   APTR CH_FCOLS[FOCOLNUM];
   APTR CH_MCOLS[MACOLNUM];
   APTR CH_FIXFLIST;
   APTR CH_BEAT;
   APTR CY_SIZE;
   APTR ST_PGPCMD;
   APTR ST_MYPGPID;
   APTR CH_ENCSELF;
   APTR ST_REMAILER;
   APTR ST_FIRSTLINE;
   APTR ST_LOGFILE;
   APTR CY_LOGMODE;
   APTR CH_SPLITLOG;
   APTR CH_LOGALL;
   APTR CH_POPSTART;
   APTR CH_SENDSTART;
   APTR CH_DELETESTART;
   APTR CH_REMOVESTART;
   APTR CH_LOADALL;
   APTR CH_MARKNEW;
   APTR CH_CHECKBD;
   APTR CH_SENDQUIT;
   APTR CH_DELETEQUIT;
   APTR CH_REMOVEQUIT;
   APTR LV_MIME;
   APTR GR_MIME;
   APTR ST_CTYPE;
   APTR ST_EXTENS;
   APTR ST_COMMAND;
   APTR ST_DEFVIEWER;
   APTR BT_MADD;
   APTR BT_MDEL;
   APTR CH_IDENTBIN;
   APTR ST_DETACHDIR;
   APTR ST_ATTACHDIR;
   APTR ST_GALLDIR;
   APTR ST_PROXY;
   APTR ST_PHOTOURL;
   APTR CH_ADDINFO;
   APTR CY_ATAB;
   APTR ST_NEWGROUP;
   APTR CH_ACOLS[ABCOLNUM];
   APTR LV_REXX;
   APTR ST_RXNAME;
   APTR ST_SCRIPT;
   APTR CY_ISADOS;
   APTR CH_CONSOLE;
   APTR CH_WAITTERM;
   APTR ST_TEMPDIR;
   APTR ST_APPX;
   APTR ST_APPY;
   APTR CH_CLGADGET;
   APTR CH_CONFIRM;
   APTR NB_CONFIRMDEL;
   APTR CH_REMOVE;
   APTR CH_SAVESENT;
   APTR RA_MDN_DISP;
   APTR RA_MDN_PROC;
   APTR RA_MDN_DELE;
   APTR RA_MDN_RULE;
   APTR CH_SEND_MDN;
   APTR TX_PACKER;
   APTR TX_ENCPACK;
   APTR NB_PACKER;
   APTR NB_ENCPACK;
   APTR ST_ARCHIVER;
   APTR ST_APPICON;
   APTR CH_FCNTMENU;
   APTR CH_MCNTMENU;
   APTR CY_INFOBAR;
   APTR ST_INFOBARTXT;
   APTR CH_WARNSUBJECT;
   APTR NB_EMAILCACHE;
   APTR CH_AUTOTRANSLATEIN;
};

struct CO_ClassData  /* configuration window */
{
   struct CO_GUIData GUI;
   int  VisiblePage;
   int  LastSig;
   BOOL Visited[MAXCPAGES];
   BOOL UpdateAll;
};

#define P3SSL_OFF    0
#define P3SSL_SSL    1
#define P3SSL_STLS   2

struct POP3
{
   char  Account[SIZE_USERID+SIZE_HOST];
   char  Server[SIZE_HOST];
   int   Port;
   char  User[SIZE_USERID];
   char  Password[SIZE_USERID];
   BOOL  Enabled;
   int   SSLMode;
   BOOL  UseAPOP;
   BOOL  DeleteOnServer;
};

struct MimeView
{
   char  ContentType[SIZE_CTYPE];
   char  Command[SIZE_COMMAND];
   char  Extension[SIZE_NAME];
};

struct Rule
{
   char  Name[SIZE_NAME];     // due to alignment this entry have to be on the top
   char  **PatternsFromList;
   int   Combine;
   int   Field[2];
   int   SubField[2];
   int   Comparison[2];
   int   Actions;
   BOOL  Remote;
   BOOL  ApplyToNew;
   BOOL  ApplyOnReq;
   BOOL  ApplyToSent;
   BOOL  CaseSens[2];
   BOOL  Substring[2];
   char  CustomField[2][SIZE_DEFAULT];
   char  Match[2][SIZE_PATTERN];
   char  BounceTo[SIZE_ADDRESS];
   char  ForwardTo[SIZE_ADDRESS];
   char  ReplyFile[SIZE_PATHFILE];
   char  ExecuteCmd[SIZE_COMMAND];
   char  PlaySound[SIZE_PATHFILE];
   char  MoveTo[SIZE_NAME];
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

#define HIDE_INFO    1
#define HIDE_XY      2
#define HIDE_TBAR    4

#define NOTI_REQ     1
#define NOTI_SOUND   2
#define NOTI_CMD     4

enum PrintMethod {PRINTMETHOD_DUMPRAW, PRINTMETHOD_LATEX, PRINTMETHOD_POSTSCRIPT};
/* PS not yet implemented */

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
   enum PrintMethod PrintMethod;
   int   StackSize;
   int   SizeFormat;
   int   InfoBar;
   int   EmailCache;
   int   SMTP_Port;

   BOOL  DaylightSaving;
   BOOL  Allow8bit;
   BOOL  Use_SMTP_TLS;
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
   BOOL  AskJumpUnread;
   BOOL  WarnSubject;
   BOOL  FolderCntMenu;
   BOOL  MessageCntMenu;
   BOOL	AutomaticTranslationIn;

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
   char  AppIconText[SIZE_COMMAND];
   char  InfoBarText[SIZE_COMMAND];
};

enum SizeFormat { SF_DEFAULT=0, SF_MIXED, SF_1PREC, SF_2PREC, SF_3PREC };
enum InfoBarPos { IB_POS_TOP=0, IB_POS_CENTER, IB_POS_BOTTOM, IB_POS_OFF };

extern struct Config *C;
extern struct Config *CE;
extern struct Hook    CO_AddMimeViewHook;
extern struct Hook    CO_AddPOP3Hook;
extern struct Hook    CO_AddRuleHook;
extern struct Hook    CO_DelMimeViewHook;
extern struct Hook    CO_DelPOP3Hook;
extern struct Hook    CO_DelRuleHook;
extern struct Hook    CO_EditSignatHook;
extern struct Hook    CO_GetDefaultPOPHook;
extern struct Hook    CO_GetMVEntryHook;
extern struct Hook    CO_GetP3EntryHook;
extern struct Hook    CO_GetRUEntryHook;
extern struct Hook    CO_GetRXEntryHook;
extern struct Hook    CO_OpenHook;
extern struct Hook    CO_PL_DspFuncHook;
extern struct Hook    CO_PutMVEntryHook;
extern struct Hook    CO_PutP3EntryHook;
extern struct Hook    CO_PutRUEntryHook;
extern struct Hook    CO_PutRXEntryHook;
extern struct Hook    CO_RemoteToggleHook;

void              CO_FreeConfig(struct Config *co);
BOOL              CO_IsValid(void);
struct MimeView * CO_NewMimeView(void);
struct POP3 *     CO_NewPOP3(struct Config *co, BOOL first);
struct Rule *     CO_NewRule(void);
void              CO_RuleGhost(struct CO_GUIData *gui, struct Rule *ru);
void              CO_SetDefaults(struct Config *co, int page);
void              CO_Validate(struct Config *co, BOOL update);

#endif /* YAM_CONFIG_H */
