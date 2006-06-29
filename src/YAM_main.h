#ifndef YAM_MAIN_H
#define YAM_MAIN_H

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

#include <mui/Toolbar_mcc.h>

#include "YAM_mainFolder.h"
#include "YAM_stringsizes.h"

#define ANYBOX NULL

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
#define isMultiPartMail(mail)         (hasFlag((mail)->mflags, MFLAG_MP_MIXED | MFLAG_MP_REPORT | MFLAG_MP_CRYPT | MFLAG_MP_SIGNED | MFLAG_MP_ALTERN))
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
#define SFLAG_READ      (1<<0)      // has been read by the user
#define SFLAG_REPLIED   (1<<1)      // a reply has been successfully sent
#define SFLAG_FORWARDED (1<<2)      // the message has been forwarded
#define SFLAG_NEW       (1<<3)      // This message is new since last startup
#define SFLAG_QUEUED    (1<<4)      // If set, this message is queued for delivery
#define SFLAG_HOLD      (1<<5)      // If set, this message is locked and will not be delivered or deleted
#define SFLAG_SENT      (1<<6)      // Message was successfully sent (if outgoing mail)
#define SFLAG_DELETED   (1<<7)      // Message was marked as deleted
#define SFLAG_MARKED    (1<<8)      // Message is marked/flagged
#define SFLAG_ERROR     (1<<9)      // This message is in an error state (error sending)
#define SCHAR_READ      'R'         // [R] - SFLAG_READ
#define SCHAR_REPLIED   'A'         // [A] - SFLAG_REPLIED
#define SCHAR_FORWARDED 'F'         // [F] - SFLAG_FORWARDED
#define SCHAR_NEW       'N'         // [N] - SFLAG_NEW
#define SCHAR_QUEUED    'Q'         // [Q] - SFLAG_QUEUED
#define SCHAR_HOLD      'H'         // [H] - SFLAG_HOLD
#define SCHAR_SENT      'S'         // [S] - SFLAG_SENT
#define SCHAR_DELETED   'D'         // [D] - SFLAG_DELETED
#define SCHAR_MARKED    'M'         // [M] - SFLAG_MARKED
#define SCHAR_ERROR     'E'         // [E] - SFLAG_ERROR
#define hasStatusRead(mail)         (isFlagSet((mail)->sflags, SFLAG_READ))
#define hasStatusReplied(mail)      (isFlagSet((mail)->sflags, SFLAG_REPLIED))
#define hasStatusForwarded(mail)    (isFlagSet((mail)->sflags, SFLAG_FORWARDED))
#define hasStatusNew(mail)          (isFlagSet((mail)->sflags, SFLAG_NEW))
#define hasStatusQueued(mail)       (isFlagSet((mail)->sflags, SFLAG_QUEUED))
#define hasStatusHold(mail)         (isFlagSet((mail)->sflags, SFLAG_HOLD))
#define hasStatusSent(mail)         (isFlagSet((mail)->sflags, SFLAG_SENT))
#define hasStatusDeleted(mail)      (isFlagSet((mail)->sflags, SFLAG_DELETED))
#define hasStatusMarked(mail)       (isFlagSet((mail)->sflags, SFLAG_MARKED))
#define hasStatusError(mail)        (isFlagSet((mail)->sflags, SFLAG_ERROR))
#define setStatusToRead(mail)       MA_ChangeMailStatus(mail, SFLAG_READ, SFLAG_NEW)
#define setStatusToReplied(mail)    MA_ChangeMailStatus(mail, SFLAG_REPLIED|SFLAG_READ, SFLAG_NEW)
#define setStatusToForwarded(mail)  MA_ChangeMailStatus(mail, SFLAG_FORWARDED, SFLAG_NEW)
#define setStatusToNew(mail)        MA_ChangeMailStatus(mail, SFLAG_NEW, SFLAG_NONE)
#define setStatusToQueued(mail)     MA_ChangeMailStatus(mail, SFLAG_QUEUED|SFLAG_READ, SFLAG_SENT|SFLAG_HOLD)
#define setStatusToHold(mail)       MA_ChangeMailStatus(mail, SFLAG_HOLD|SFLAG_READ, SFLAG_QUEUED)
#define setStatusToSent(mail)       MA_ChangeMailStatus(mail, SFLAG_SENT|SFLAG_READ, SFLAG_QUEUED|SFLAG_HOLD|SFLAG_ERROR)
#define setStatusToDeleted(mail)    MA_ChangeMailStatus(mail, SFLAG_DELETED, SFLAG_NONE)
#define setStatusToMarked(mail)     MA_ChangeMailStatus(mail, SFLAG_MARKED, SFLAG_NONE)
#define setStatusToError(mail)      MA_ChangeMailStatus(mail, SFLAG_ERROR, SFLAG_NONE)

// For compatibility to the old status levels we use the following macros
// But as soon as we have reworked the filename handling we can remove them again
#define hasStatusUnread(mail)       (!hasStatusRead(mail) && !hasStatusNew(mail))
#define hasStatusOld(mail)          (hasStatusRead(mail) && !hasStatusNew(mail))
#define setStatusToUnread(mail)     MA_ChangeMailStatus(mail, SFLAG_NONE, SFLAG_NEW|SFLAG_READ)
#define setStatusToOld(mail)        MA_ChangeMailStatus(mail, SFLAG_READ, SFLAG_NEW)

// for managing the different status icons we manage our IDs and ESC sequences here
#define SICON_ID_UNREAD     0      // status_unread
#define SICON_ID_OLD        1      // status_old
#define SICON_ID_FORWARD    2      // status_forward
#define SICON_ID_REPLY      3      // status_reply
#define SICON_ID_WAITSEND   4      // status_waitsend
#define SICON_ID_ERROR      5      // status_error
#define SICON_ID_HOLD       6      // status_hold
#define SICON_ID_SENT       7      // status_sent
#define SICON_ID_NEW        8      // status_new
#define SICON_ID_DELETE     9      // status_delete
#define SICON_ID_DOWNLOAD   10     // status_download
#define SICON_ID_GROUP      11     // status_group
#define SICON_ID_URGENT     12     // status_urgent
#define SICON_ID_ATTACH     13     // status_attach
#define SICON_ID_REPORT     14     // status_report
#define SICON_ID_CRYPT      15     // status_crypt
#define SICON_ID_SIGNED     16     // status_signed
#define SICON_ID_MARK       17     // status_mark

#define SICON_UNREAD        "\033o[" STR(SICON_ID_UNREAD)   "]"
#define SICON_OLD           "\033o[" STR(SICON_ID_OLD)      "]"
#define SICON_FORWARD       "\033o[" STR(SICON_ID_FORWARD)  "]"
#define SICON_REPLY         "\033o[" STR(SICON_ID_REPLY)    "]"
#define SICON_WAITSEND      "\033o[" STR(SICON_ID_WAITSEND) "]"
#define SICON_ERROR         "\033o[" STR(SICON_ID_ERROR)    "]"
#define SICON_HOLD          "\033o[" STR(SICON_ID_HOLD)     "]"
#define SICON_SENT          "\033o[" STR(SICON_ID_SENT)     "]"
#define SICON_NEW           "\033o[" STR(SICON_ID_NEW)      "]"
#define SICON_DELETE        "\033o[" STR(SICON_ID_DELETE)   "]"
#define SICON_DOWNLOAD      "\033o[" STR(SICON_ID_DOWNLOAD) "]"
#define SICON_GROUP         "\033o[" STR(SICON_ID_GROUP)    "]"
#define SICON_URGENT        "\033o[" STR(SICON_ID_URGENT)   "]"
#define SICON_ATTACH        "\033o[" STR(SICON_ID_ATTACH)   "]"
#define SICON_REPORT        "\033o[" STR(SICON_ID_REPORT)   "]"
#define SICON_CRYPT         "\033o[" STR(SICON_ID_CRYPT)    "]"
#define SICON_SIGNED        "\033o[" STR(SICON_ID_SIGNED)   "]"
#define SICON_MARK          "\033o[" STR(SICON_ID_MARK)     "]"

enum ImportanceLevel { IMP_NORMAL=0, IMP_LOW, IMP_HIGH };
enum NewMode { NEW_NEW, NEW_REPLY, NEW_FORWARD, NEW_BOUNCE, NEW_EDIT, NEW_SAVEDEC };

// flags and macros for creating new mails
#define NEWF_QUIET        (1<<0)
#define NEWF_REP_NOQUOTE  (1<<1)
#define NEWF_REP_PRIVATE  (1<<2)
#define NEWF_REP_MLIST    (1<<3)
#define NEWF_FWD_NOATTACH (1<<4)
#define hasQuietFlag(v)         (isFlagSet((v), NEWF_QUIET))
#define hasNoQuoteFlag(v)       (isFlagSet((v), NEWF_REP_NOQUOTE))
#define hasPrivateFlag(v)       (isFlagSet((v), NEWF_REP_PRIVATE))
#define hasMListFlag(v)         (isFlagSet((v), NEWF_REP_MLIST))
#define hasNoAttachFlag(v)      (isFlagSet((v), NEWF_FWD_NOATTACH))

enum Macro {
   MACRO_MEN0=0, MACRO_MEN1, MACRO_MEN2, MACRO_MEN3, MACRO_MEN4, MACRO_MEN5,
   MACRO_MEN6, MACRO_MEN7, MACRO_MEN8, MACRO_MEN9, MACRO_STARTUP, MACRO_QUIT,
   MACRO_PREGET, MACRO_POSTGET, MACRO_NEWMSG, MACRO_PRESEND, MACRO_POSTSEND,
   MACRO_READ, MACRO_PREWRITE, MACRO_POSTWRITE, MACRO_URL
};

enum SendMode { SEND_ALL, SEND_ACTIVE };

enum { MMEN_ABOUT=100,MMEN_ABOUTMUI,MMEN_VERSION,MMEN_ERRORS,MMEN_LOGIN,MMEN_HIDE,MMEN_QUIT,
       MMEN_NEWF,MMEN_NEWFG,MMEN_EDITF,MMEN_DELETEF,MMEN_OSAVE,MMEN_ORESET,MMEN_SELALL,MMEN_SELNONE,
       MMEN_SELTOGG,MMEN_SEARCH,MMEN_FILTER,MMEN_DELDEL,MMEN_INDEX,MMEN_FLUSH,MMEN_IMPORT,MMEN_EXPORT,
       MMEN_GETMAIL,MMEN_GET1MAIL,MMEN_SENDMAIL,MMEN_EXMAIL,MMEN_READ,MMEN_EDIT,MMEN_MOVE,MMEN_COPY,
       MMEN_DELETE,MMEN_PRINT,MMEN_SAVE,MMEN_DETACH,MMEN_CROP,MMEN_EXPMSG,MMEN_NEW,MMEN_REPLY,MMEN_FORWARD,
       MMEN_BOUNCE,MMEN_SAVEADDR,MMEN_TOUNREAD,MMEN_TOREAD,MMEN_TOHOLD,MMEN_TOQUEUED,MMEN_TOMARKED,
       MMEN_TOUNMARKED,MMEN_ALLTOREAD,MMEN_CHSUBJ,MMEN_SEND,MMEN_ABOOK,MMEN_CONFIG,MMEN_USER,MMEN_MUI,
       MMEN_SCRIPT,MMEN_POPHOST, MMEN_MACRO=MMEN_POPHOST+MAXP3
     };

struct MA_GUIData
{
   Object *WI;
   Object *MN_FOLDER;
   Object *MN_REXX;
   Object *MS_MAIN;
   Object *GR_HIDDEN;
   Object *ST_LAYOUT;
   Object *MI_ERRORS;
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
   Object *MI_DELETE;
   Object *MI_PRINT;
   Object *MI_SAVE;
   Object *MI_ATTACH;
   Object *MI_SAVEATT;
   Object *MI_REMATT;
   Object *MI_EXPMSG;
   Object *MI_REPLY;
   Object *MI_FORWARD;
   Object *MI_BOUNCE;
   Object *MI_GETADDRESS;
   Object *MI_STATUS;
   Object *MI_TOREAD;
   Object *MI_TOUNREAD;
   Object *MI_TOHOLD;
   Object *MI_TOQUEUED;
   Object *MI_TOMARKED;
   Object *MI_TOUNMARKED;
   Object *MI_ALLTOREAD;
   Object *MI_CHSUBJ;
   Object *MI_SEND;
   Object *MI_FILTER;
   Object *MI_UPDINDEX;
   Object *MI_SELECT;
   Object *LV_FOLDERS;
   Object *NL_FOLDERS;
   Object *TO_TOOLBAR;
   Object *IB_INFOBAR;
   Object *GR_MAIN;
   Object *GR_TOP;
   Object *GR_BOTTOM;
   Object *GR_MAILVIEW;
   Object *BL_MAILVIEW;
   Object *MN_EMBEDDEDREADPANE;
   Object *GR_QUICKSEARCHBAR;
   Object *PG_MAILLIST;
   struct MUIP_Toolbar_Description TB_TOOLBAR[18];
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
extern struct Hook MA_DeleteMessageHook;
extern struct Hook MA_SavePrintHook;
extern struct Hook MA_SaveAttachHook;
extern struct Hook MA_RemoveAttachHook;
extern struct Hook MA_ExportMessagesHook;
extern struct Hook MA_ChangeSelectedHook;
extern struct Hook MA_DeleteDeletedHook;
extern struct Hook MA_DeleteOldHook;
extern struct Hook MA_RescanIndexHook;
extern struct Hook MA_SendHook;
extern struct Hook MA_SetFolderInfoHook;
extern struct Hook MA_SetMessageInfoHook;
extern struct Hook PO_WindowHook;
extern struct Hook MA_FolderKeyHook;

void  MA_ChangeSubject(struct Mail *mail, char *subj);
void  MA_ChangeTransfer(BOOL on);
struct Mail **MA_CreateMarkedList(Object *lv, BOOL onlyNew);
struct Mail **MA_CreateFullList(struct Folder *fo, BOOL onlyNew);
void  MA_DeleteMessage(BOOL delatonce, BOOL force);
void  MA_DeleteSingle(struct Mail *mail, BOOL forceatonce, BOOL quiet);
BOOL  MA_ExportMessages(BOOL all, char *filename, BOOL append);
struct Mail *MA_GetActiveMail(struct Folder *forcefolder, struct Folder **folderp, int *activep);
void  MA_GetAddress(struct Mail **mlist);
BOOL  MA_ImportMessages(char *fname);
struct MA_ClassData *MA_New(void);
BOOL  MA_SortWindow(void);
void  MA_MoveCopy(struct Mail *mail, struct Folder *frombox, struct Folder *tobox, BOOL copyit);
int   MA_NewBounce(struct Mail *mail, int flags);
int   MA_NewEdit(struct Mail *mail, int flags, Object *readWindow);
int   MA_NewForward(struct Mail **mlist, int flags);
int   MA_NewMessage(int mode, int flags);
int   MA_NewNew(struct Mail *mail, int flags);
int   MA_NewReply(struct Mail **mlist, int flags);
void  MA_PopNow(int mode, int pop);
void  MA_RemoveAttach(struct Mail *mail, BOOL warning);
BOOL  MA_Send(enum SendMode sendpos);
BOOL  MA_SendMList(struct Mail **mlist);
void  MA_ChangeMailStatus(struct Mail *mail, int addflags, int clearflags);
BOOL  MA_UpdateMailFile(struct Mail *mail);
void  MA_SetSortFlag(void);
void  MA_SetStatusTo(int addflags, int clearflags, BOOL all);
void  MA_SetupDynamicMenus(void);
BOOL  MA_StartMacro(enum Macro num, char *param);
char *MA_ToStatusHeader(struct Mail *mail);
char *MA_ToXStatusHeader(struct Mail *mail);
unsigned int MA_FromStatusHeader(char *statusflags);
unsigned int MA_FromXStatusHeader(char *xstatusflags);
void  MA_SetupEmbeddedReadPane(void);
void  MA_SetupQuickSearchBar(void);
char *MA_GetRealSubject(char *sub);
void  MA_ChangeSelected(BOOL forceUpdate);


#endif /* YAM_MAIN_H */
