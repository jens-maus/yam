#ifndef YAM_MAIN_H
#define YAM_MAIN_H

/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2019 YAM Open Source Team

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

#include <intuition/classes.h>

#include "YAM_stringsizes.h"

#include "tcp/pop3.h"
#include "tcp/smtp.h"

// forward declarations
struct Mail;
struct Folder;
struct Part;
struct MUI_NListtree_TreeNode;

// General internal flags of a mail (no status information)
// Warning: Please note that if you change something here you have to make
//          sure to increase the version number of the .index file in YAM_MAf.c!
//          However, if only additions are made no index version bump is necessary.
#define MFLAG_MULTIRCPT     (1<<0)
#define MFLAG_MP_MIXED      (1<<1)      // multipart/mixed
#define MFLAG_MP_REPORT     (1<<2)      // multipart/report
#define MFLAG_MP_CRYPT      (1<<3)      // multipart/encrypted
#define MFLAG_MP_SIGNED     (1<<4)      // multipart/signed
#define MFLAG_MP_ALTERN     (1<<5)      // multipart/alternative
#define MFLAG_PARTIAL       (1<<6)      // message/partial
#define MFLAG_SENDERINFO    (1<<7)
#define MFLAG_SENDMDN       (1<<8)
#define MFLAG_NOFOLDER      (1<<9)
#define MFLAG_PERFIELD          10      // reserve 3 bits of the flags for the permanent flag section
#define MFLAG_VOLFIELD          13      // reserve 3 bits of the flags for the volatile flag section
#define MFLAG_IMPORTANCE        16      // reserve 2 bits to store the level of importance
#define MFLAG_MULTISENDER   (1<<18)
#define MFLAG_MULTIREPLYTO  (1<<19)
#define isMultiRCPTMail(mail)         (isFlagSet((mail)->mflags, MFLAG_MULTIRCPT))
#define isMultiPartMail(mail)         (isAnyFlagSet((mail)->mflags, MFLAG_MP_MIXED | MFLAG_MP_REPORT | MFLAG_MP_CRYPT | MFLAG_MP_SIGNED | MFLAG_MP_ALTERN))
#define isMP_MixedMail(mail)          (isFlagSet((mail)->mflags, MFLAG_MP_MIXED))
#define isMP_ReportMail(mail)         (isFlagSet((mail)->mflags, MFLAG_MP_REPORT))
#define isMP_CryptedMail(mail)        (isFlagSet((mail)->mflags, MFLAG_MP_CRYPT))
#define isMP_SignedMail(mail)         (isFlagSet((mail)->mflags, MFLAG_MP_SIGNED))
#define isMP_AlternativeMail(mail)    (isFlagSet((mail)->mflags, MFLAG_MP_ALTERN))
#define isPartialMail(mail)           (isFlagSet((mail)->mflags, MFLAG_PARTIAL))
#define isSenderInfoMail(mail)        (isFlagSet((mail)->mflags, MFLAG_SENDERINFO))
#define isSendMDNMail(mail)           (isFlagSet((mail)->mflags, MFLAG_SENDMDN))
#define isVirtualMail(mail)           (isFlagSet((mail)->mflags, MFLAG_NOFOLDER))
#define setPERValue(mail, val)        (mail)->mflags = ((mail)->mflags & ~(7<<MFLAG_PERFIELD)) | ((val) << MFLAG_PERFIELD)
#define setVOLValue(mail, val)        (mail)->mflags = ((mail)->mflags & ~(7<<MFLAG_VOLFIELD)) | ((val) << MFLAG_VOLFIELD)
#define setImportanceLevel(mail, val) (mail)->mflags = ((mail)->mflags & ~(3<<MFLAG_IMPORTANCE)) | ((val) << MFLAG_IMPORTANCE)
#define getPERValue(mail)             (((mail)->mflags & (7<<MFLAG_PERFIELD)) >> MFLAG_PERFIELD)
#define getVOLValue(mail)             (((mail)->mflags & (7<<MFLAG_VOLFIELD)) >> MFLAG_VOLFIELD)
#define getImportanceLevel(mail)      (((mail)->mflags & (3<<MFLAG_IMPORTANCE)) >> MFLAG_IMPORTANCE)
#define isMultiSenderMail(mail)       (isFlagSet((mail)->mflags, MFLAG_MULTISENDER))
#define isMultiReplyToMail(mail)      (isFlagSet((mail)->mflags, MFLAG_MULTIREPLYTO))

// Status information flags of a mail (also partly stored in file comments)
// Warning: Please note that if you change something here you have to make
//          sure to increase the version number of the .index file in YAM_MAf.c!
//          However, if only additions are made no index version bump is necessary.
#define SFLAG_NONE      (0<<0)
#define SFLAG_READ      (1<<0)        // has been read by the user
#define SFLAG_REPLIED   (1<<1)        // a reply has been successfully sent
#define SFLAG_FORWARDED (1<<2)        // the message has been forwarded
#define SFLAG_NEW       (1<<3)        // This message is new since last startup
#define SFLAG_QUEUED    (1<<4)        // If set, this message is queued for delivery
#define SFLAG_HOLD      (1<<5)        // If set, this message is locked and will not be delivered or deleted
#define SFLAG_SENT      (1<<6)        // Message was successfully sent (if outgoing mail)
#define SFLAG_MARKED    (1<<8)        // Message is marked/flagged
#define SFLAG_ERROR     (1<<9)        // This message is in an error state (error sending)
#define SFLAG_USERSPAM  (1<<10)       // This message is marked as spam by user
#define SFLAG_AUTOSPAM  (1<<11)       // This message is marked as spam automatically
#define SFLAG_HAM       (1<<12)       // This message is marked as ham by user
#define SCHAR_READ      'R'           // [R] - SFLAG_READ
#define SCHAR_REPLIED   'A'           // [A] - SFLAG_REPLIED
#define SCHAR_FORWARDED 'F'           // [F] - SFLAG_FORWARDED
#define SCHAR_NEW       'N'           // [N] - SFLAG_NEW
#define SCHAR_QUEUED    'Q'           // [Q] - SFLAG_QUEUED
#define SCHAR_HOLD      'H'           // [H] - SFLAG_HOLD
#define SCHAR_SENT      'S'           // [S] - SFLAG_SENT
#define SCHAR_MARKED    'M'           // [M] - SFLAG_MARKED
#define SCHAR_ERROR     'E'           // [E] - SFLAG_ERROR
#define SCHAR_USERSPAM  'X'           // [X] - SFLAG_USERSPAM
#define SCHAR_AUTOSPAM  'J'           // [J] - SFLAG_AUTOSPAM
#define SCHAR_HAM       'Y'           // [Y] - SFLAG_HAM
#define hasStatusRead(mail)           (isFlagSet((mail)->sflags, SFLAG_READ))
#define hasStatusReplied(mail)        (isFlagSet((mail)->sflags, SFLAG_REPLIED))
#define hasStatusForwarded(mail)      (isFlagSet((mail)->sflags, SFLAG_FORWARDED))
#define hasStatusNew(mail)            (isFlagSet((mail)->sflags, SFLAG_NEW))
#define hasStatusHold(mail)           (isFlagSet((mail)->sflags, SFLAG_HOLD))
#define hasStatusSent(mail)           (isFlagSet((mail)->sflags, SFLAG_SENT))
#define hasStatusMarked(mail)         (isFlagSet((mail)->sflags, SFLAG_MARKED))
#define hasStatusError(mail)          (isFlagSet((mail)->sflags, SFLAG_ERROR))
#define hasStatusSpam(mail)           (isAnyFlagSet((mail)->sflags, SFLAG_USERSPAM | SFLAG_AUTOSPAM))
#define hasStatusUserSpam(mail)       (isFlagSet((mail)->sflags, SFLAG_USERSPAM))
#define hasStatusAutoSpam(mail)       (isFlagSet((mail)->sflags, SFLAG_AUTOSPAM))
#define hasStatusHam(mail)            (isFlagSet((mail)->sflags, SFLAG_HAM))
#define setStatusToRead(mail)         MA_ChangeMailStatus(mail, SFLAG_READ, SFLAG_NEW)
#define setStatusToReplied(mail)      MA_ChangeMailStatus(mail, SFLAG_REPLIED|SFLAG_READ, SFLAG_NEW)
#define setStatusToForwarded(mail)    MA_ChangeMailStatus(mail, SFLAG_FORWARDED, SFLAG_NEW)
#define setStatusToNew(mail)          MA_ChangeMailStatus(mail, SFLAG_NEW, SFLAG_NONE)
#define setStatusToSent(mail)         MA_ChangeMailStatus(mail, SFLAG_SENT|SFLAG_READ, SFLAG_NEW|SFLAG_QUEUED|SFLAG_HOLD|SFLAG_ERROR)
#define setStatusToMarked(mail)       MA_ChangeMailStatus(mail, SFLAG_MARKED, SFLAG_NONE)
#define setStatusToUnmarked(mail)     MA_ChangeMailStatus(mail, SFLAG_NONE, SFLAG_MARKED)
#define setStatusToError(mail)        MA_ChangeMailStatus(mail, SFLAG_ERROR, SFLAG_NONE)
#define setStatusToUserSpam(mail)     MA_ChangeMailStatus(mail, SFLAG_USERSPAM|SFLAG_READ, SFLAG_NEW|SFLAG_AUTOSPAM|SFLAG_HAM)
#define setStatusToAutoSpam(mail)     MA_ChangeMailStatus(mail, SFLAG_AUTOSPAM, SFLAG_USERSPAM|SFLAG_HAM)
#define setStatusToReadAutoSpam(mail) MA_ChangeMailStatus(mail, SFLAG_AUTOSPAM|SFLAG_READ, SFLAG_NEW|SFLAG_USERSPAM|SFLAG_HAM)
#define setStatusToHam(mail)          MA_ChangeMailStatus(mail, SFLAG_HAM, SFLAG_USERSPAM|SFLAG_AUTOSPAM)

// For compatibility to the old status levels we use the following macros
// But as soon as we have reworked the filename handling we can remove them again
#define hasStatusUnread(mail)         (!hasStatusRead(mail) && !hasStatusNew(mail))
#define setStatusToUnread(mail)       MA_ChangeMailStatus(mail, SFLAG_NONE, SFLAG_NEW|SFLAG_READ)

// for managing the message list colums via a bitmask
#define MCOL_STATUS             (1<<0) // message status
#define MCOL_SENDER             (1<<1) // message sender/recipient
#define MCOL_REPLYTO            (1<<2) // reply-to address
#define MCOL_SUBJECT            (1<<3) // message subject
#define MCOL_DATE               (1<<4) // message date
#define MCOL_SIZE               (1<<5) // message size
#define MCOL_FILENAME           (1<<6) // message filename
#define MCOL_TRANSDATE          (1<<7) // recipient/sender transfer date
#define MCOL_MAILACCOUNT        (1<<8) // mail account mail was transfered to/from
#define hasMColStatus(v)        (isFlagSet((v), MCOL_STATUS))
#define hasMColSender(v)        (isFlagSet((v), MCOL_SENDER))
#define hasMColReplyTo(v)       (isFlagSet((v), MCOL_REPLYTO))
#define hasMColSubject(v)       (isFlagSet((v), MCOL_SUBJECT))
#define hasMColDate(v)          (isFlagSet((v), MCOL_DATE))
#define hasMColSize(v)          (isFlagSet((v), MCOL_SIZE))
#define hasMColFilename(v)      (isFlagSet((v), MCOL_FILENAME))
#define hasMColTransDate(v)     (isFlagSet((v), MCOL_TRANSDATE))
#define hasMColMailAccount(v)   (isFlagSet((v), MCOL_MAILACCOUNT))

// Mail importance levels
enum ImportanceLevel
{
  IMP_NORMAL=0, // normal (default)
  IMP_LOW,      // low
  IMP_HIGH      // high
};

// Mail creation modes
enum NewMailMode
{
  NMM_NEW=0,          // composing a new mail
  NMM_REPLY,          // replying to an existing mail
  NMM_FORWARD,        // forwarding an existing mail, automatic mode detection (toolbar only)
  NMM_FORWARD_ATTACH, // forwarding an existing mail, as attachment (menu only)
  NMM_FORWARD_INLINE, // forwarding an existing mail, as inlined text (menu only)
  NMM_REDIRECT,       // redirect an existing mail to another recipient
  NMM_EDIT,           // edit a mail in the draft/outgoing folder
  NMM_EDITASNEW,      // edit an existing sent/received mail
  NMM_SAVEDEC         // create a decrypted copy of a PGP mail
};

// Mail forward modes
enum ForwardMode
{
  FWM_ATTACH=0, // forward mail as attachment
  FWM_INLINE    // forward mail inlined
};

// flags for MA_MoveCopy and MA_MoveCopySingle
#define MVCPF_COPY              (1<<0) // copy mail instead of moving it
#define MVCPF_CLOSE_WINDOWS     (1<<1) // close possibly open read windows
#define MVCPF_CHECK_CONNECTIONS (1<<2) // make sure there is an active connection
#define MVCPF_QUIET             (1<<3) // don't update the folder/icon stats

// flags for MA_DeleteSingle
#define DELF_AT_ONCE            (1<<0) // delete immediately, don't move to Trash folder
#define DELF_QUIET              (1<<1) // don't update the folder/icon stats
#define DELF_CLOSE_WINDOWS      (1<<2) // close possibly open read windows
#define DELF_UPDATE_APPICON     (1<<3)
#define DELF_CHECK_CONNECTIONS  (1<<4) // make sure there is an active connection

// flags and macros for creating/replying/forwarding mails
#define NEWF_QUIET               (1<<0)
#define NEWF_REP_NOQUOTE         (1<<1) // force reply to mail without quoting (CTRL qualifier)
#define NEWF_REP_PRIVATE         (1<<2) // force reply to the sender (SHIFT qualifier)
#define NEWF_REP_MLIST           (1<<3) // force reply to mailing list address (ALT qualifier)
#define NEWF_FWD_NOATTACH        (1<<4) // remove any attachement from the forwared mail (CTRL qualifier)
#define NEWF_FWD_ALTMODE         (1<<5) // used alternative (not configured) forward mode (ALT qualifier)
#define NEWF_FWD_AS_ATTACHMENT   (1<<6) // force forwarding as an attachment
#define NEWF_FWD_INLINED         (1<<7) // force forwarding as inlined text
#define hasQuietFlag(v)               (isFlagSet((v), NEWF_QUIET))
#define hasNoQuoteFlag(v)             (isFlagSet((v), NEWF_REP_NOQUOTE))
#define hasPrivateFlag(v)             (isFlagSet((v), NEWF_REP_PRIVATE))
#define hasMListFlag(v)               (isFlagSet((v), NEWF_REP_MLIST))
#define hasNoAttachFlag(v)            (isFlagSet((v), NEWF_FWD_NOATTACH))
#define hasAltFwdModeFlag(v)          (isFlagSet((v), NEWF_FWD_ALTMODE))
#define hasAsAttachmentFwdModeFlag(v) (isFlagSet((v), NEWF_FWD_AS_ATTACHMENT))
#define hasInlinedFwdModeFlag(v)      (isFlagSet((v), NEWF_FWD_INLINED))

enum Macro
{
  MACRO_PROMPT_USER=-1,
  MACRO_MEN0=0,
  MACRO_MEN1,
  MACRO_MEN2,
  MACRO_MEN3,
  MACRO_MEN4,
  MACRO_MEN5,
  MACRO_MEN6,
  MACRO_MEN7,
  MACRO_MEN8,
  MACRO_MEN9,
  MACRO_STARTUP,
  MACRO_QUIT,
  MACRO_PREGET,
  MACRO_POSTGET,
  MACRO_NEWMSG,
  MACRO_PRESEND,
  MACRO_POSTSEND,
  MACRO_READ,
  MACRO_PREWRITE,
  MACRO_POSTWRITE,
  MACRO_URL,
  MACRO_PREFILTER,
  MACRO_POSTFILTER,
  MACRO_COUNT
};

// main window menu enumerations
enum
{
  MMEN_ABOUT=100,MMEN_ABOUTMUI,MMEN_VERSION,MMEN_ERRORS,MMEN_TRANSFERS,MMEN_LOGIN,MMEN_HIDE,MMEN_QUIT,
  MMEN_EDIT_UNDO,MMEN_EDIT_REDO,MMEN_EDIT_CUT,MMEN_EDIT_COPY,MMEN_EDIT_PASTE,MMEN_EDIT_DELETE,
  MMEN_EDIT_SALL,MMEN_EDIT_SNONE,MMEN_NEWF,MMEN_NEWFG,MMEN_EDITF,MMEN_DELETEF,
  MMEN_SELALL,MMEN_SELNONE,MMEN_SELTOGG,MMEN_SEARCH,MMEN_FILTER,MMEN_CLASSIFY,MMEN_DELDEL,MMEN_DELSPAM,
  MMEN_INDEX,MMEN_FLUSH,MMEN_IMPORT,MMEN_EXPORT,MMEN_GETMAIL,MMEN_GET1MAIL,MMEN_SENDMAIL,MMEN_EXMAIL,
  MMEN_READ,MMEN_EDIT,MMEN_MOVE,MMEN_COPY,MMEN_ARCHIVE, MMEN_DELETE,MMEN_PRINT,MMEN_SAVE,MMEN_DETACH,
  MMEN_DELETEATT,MMEN_EXPMSG,MMEN_NEXTTH,MMEN_PREVTH,MMEN_NEW,MMEN_REPLY,MMEN_FORWARD_ATTACH,
  MMEN_FORWARD_INLINE,MMEN_REDIRECT,MMEN_SAVEADDR,MMEN_TOUNREAD,MMEN_TOREAD,MMEN_TOMARKED,
  MMEN_TOUNMARKED,MMEN_ALLTOREAD,MMEN_TOSPAM,MMEN_TOHAM,MMEN_CHSUBJ,MMEN_SEND,MMEN_ABOOK,MMEN_CONFIG,
  MMEN_USER,MMEN_MUI,MMEN_SCRIPT,MMEN_MACRO,MMEN_HELP_CONTENTS=MMEN_MACRO+MAXRX_MENU,MMEN_HELP_AREXX,
  MMEN_POPHOST
};

// Actions for the 'Edit' submenu entries of windows
// e.g. Amiga+C for COPY, Amiga+V for PASTE
enum EditAction
{
  EA_CUT,
  EA_COPY,
  EA_PASTE,
  EA_DELETE,
  EA_UNDO,
  EA_REDO,
  EA_SELECTALL,
  EA_SELECTNONE
};

struct MA_GUIData
{
  Object *WI;
  Object *MN_PROJECT;
  Object *MN_EDIT;
  Object *MN_FOLDER;
  Object *MN_REXX;
  Object *MS_MAIN;
  Object *GR_HIDDEN;
  Object *ST_LAYOUT;
  Object *MI_UPDATECHECK;
  Object *MI_ERRORS;
  Object *MI_TRANSFERS;
  Object *MI_CSINGLE;
  Object *MI_IMPORT;
  Object *MI_EXPORT;
  Object *MI_SENDALL;
  Object *MI_EXCHANGE;
  Object *MI_GETMAIL;
  Object *MI_READ;
  Object *MI_EDIT;
  Object *MI_MOVE;
  Object *MI_COPY;
  Object *MI_ARCHIVE;
  Object *MI_DELETE;
  Object *MI_PRINT;
  Object *MI_SAVE;
  Object *MI_ATTACH;
  Object *MI_SAVEATT;
  Object *MI_REMATT;
  Object *MI_EXPMSG;
  Object *MI_NEW;
  Object *MI_REPLY;
  Object *MI_FORWARD;
  Object *MI_FORWARD_ATTACH;
  Object *MI_FORWARD_INLINE;
  Object *MI_REDIRECT;
  Object *MI_GETADDRESS;
  Object *MI_STATUS;
  Object *MI_TOREAD;
  Object *MI_TOUNREAD;
  Object *MI_TOMARKED;
  Object *MI_TOUNMARKED;
  Object *MI_ALLTOREAD;
  Object *MI_TOSPAM;
  Object *MI_TOHAM;
  Object *MI_CHECKSPAM;
  Object *MI_DELSPAM;
  Object *MI_CHSUBJ;
  Object *MI_SEND;
  Object *MI_FILTER;
  Object *MI_DELDEL;
  Object *MI_UPDINDEX;
  Object *MI_SELECT;
  Object *LV_FOLDERS;
  Object *LT_FOLDERS;
  Object *TO_TOOLBAR;
  Object *IB_INFOBAR;
  Object *GR_MAIN;
  Object *GR_TOP;
  Object *GR_BOTTOM;
  Object *GR_MAILVIEW;
  Object *BL_MAILVIEW;
  Object *MN_EMBEDDEDREADPANE;
  Object *GR_QUICKSEARCHBAR;
  Object *GR_MAILLIST;
  Object *PG_MAILLIST;
  Object *MI_NAVIG;
  Object *MI_NEXTTHREAD;
  Object *MI_PREVTHREAD;
  Object *MN_SETTINGS;
};

struct MA_ClassData  /* main window */
{
  struct MA_GUIData GUI;
  char WinTitle[SIZE_DEFAULT];
};

// Hooks available to other modules
extern struct Hook MA_ReadMessageHook;
extern struct Hook MA_NewMessageHook;
extern struct Hook MA_ChangeSubjectHook;
extern struct Hook MA_SetStatusToHook;
extern struct Hook MA_SetAllStatusToHook;
extern struct Hook MA_GetAddressHook;
extern struct Hook MA_MoveMessageHook;
extern struct Hook MA_CopyMessageHook;
extern struct Hook MA_ArchiveMessageHook;
extern struct Hook MA_DeleteMessageHook;
extern struct Hook MA_ClassifyMessageHook;
extern struct Hook MA_SavePrintHook;
extern struct Hook MA_SaveAttachHook;
extern struct Hook MA_RemoveAttachHook;
extern struct Hook MA_ExportMessagesHook;
extern struct Hook MA_ChangeSelectedHook;
extern struct Hook MA_SendHook;
extern struct Hook MA_PopNowHook;

void  MA_ChangeSubject(struct Mail *mail, char *subj);
struct MailList *MA_CreateMarkedList(Object *lv, BOOL onlyNew);
struct MailList *MA_CreateFullList(struct Folder *fo, BOOL onlyNew);
void  MA_DeleteMessage(BOOL delatonce, BOOL force);
void  MA_DeleteSingle(struct Mail *mail, const ULONG delFlags);
BOOL MA_ExportMessages(char *filename, const BOOL all, ULONG flags);
struct Mail *MA_GetActiveMail(struct Folder *forcefolder, struct Folder **folderp, LONG *activep);
void MA_GetAddress(struct MailList *mlist, struct MUI_NListtree_TreeNode *dropTarget, ULONG dropType);
BOOL MA_ImportMessages(const char *fname, const ULONG flags);
struct MA_ClassData *MA_New(void);
void  MA_ArchiveMail(struct Mail *mail);
void  MA_MoveCopy(struct Mail *mail, struct Folder *tobox, const char *originator, const ULONG flags);
void  MA_ExchangeMail(const ULONG receiveFlags);
BOOL  MA_PopNow(struct MailServerNode *msn, const ULONG flags, struct DownloadResult *dlResult);
void  MA_RemoveAttach(struct Mail *mail, struct Part **whichParts, BOOL warning);
BOOL  MA_Send(enum SendMailMode mode, ULONG flags);
void  MA_ChangeMailStatus(struct Mail *mail, int addflags, int clearflags);
BOOL  MA_UpdateMailFile(struct Mail *mail);
void  MA_SetSortFlag(void);
void  MA_SetStatusTo(int addflags, int clearflags, BOOL all);
void  MA_SetupDynamicMenus(void);
char *MA_ToStatusHeader(struct Mail *mail);
char *MA_ToXStatusHeader(struct Mail *mail);
unsigned int MA_FromStatusHeader(char *statusflags);
unsigned int MA_FromXStatusHeader(char *xstatusflags);
char *MA_GetRealSubject(char *sub);
void  MA_ChangeSelected(BOOL forceUpdate);

enum NewMailMode CheckNewMailQualifier(const enum NewMailMode mode, const ULONG qualifier, int *flags);
struct WriteMailData *NewMessage(enum NewMailMode mode, const int flags);
struct Mail *FindThreadInFolder(const struct Mail *srcMail, const struct Folder *folder, const BOOL nextThread);
struct Mail *FindThread(const struct Mail *srcMail, const BOOL nextThread);

BOOL ReceiveMailsFromPOP(struct MailServerNode *msn, const ULONG flags, struct DownloadResult *dlResult);

#endif /* YAM_MAIN_H */
