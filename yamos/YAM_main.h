#ifndef YAM_MAIN_H
#define YAM_MAIN_H

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

#include <mui/Toolbar_mcc.h>

#include "YAM_mainFolder.h"
#include "YAM_stringsizes.h"

#ifndef YAM_CONFIG_H
struct Rule;
#endif

// lets define all the Rule->Actions flags and
// define some flag macros for them
#define RULE_BOUNCE       (1<<0)
#define RULE_FORWARD      (1<<1)
#define RULE_REPLY        (1<<2)
#define RULE_EXECUTE      (1<<3)
#define RULE_PLAYSOUND    (1<<4)
#define RULE_MOVE         (1<<5)
#define RULE_DELETE       (1<<6)
#define RULE_SKIPMSG      (1<<7)
#define hasBounceAction(rule)     (isFlagSet((rule)->Actions, RULE_BOUNCE))
#define hasForwardAction(rule)    (isFlagSet((rule)->Actions, RULE_FORWARD))
#define hasReplyAction(rule)      (isFlagSet((rule)->Actions, RULE_REPLY))
#define hasExecuteAction(rule)    (isFlagSet((rule)->Actions, RULE_EXECUTE))
#define hasPlaySoundAction(rule)  (isFlagSet((rule)->Actions, RULE_PLAYSOUND))
#define hasMoveAction(rule)       (isFlagSet((rule)->Actions, RULE_MOVE))
#define hasDeleteAction(rule)     (isFlagSet((rule)->Actions, RULE_DELETE))
#define hasSkipMsgAction(rule)    (isFlagSet((rule)->Actions, RULE_SKIPMSG))

#define ANYBOX NULL

// General internal flags of a mail (no status information)
// Warning: Please note that if you change something here you have to make
//          sure to increase the version number of the .index file in YAM_MAf.c!
#define MFLAG_MULTIRCPT   (1<<0)
#define MFLAG_MULTIPART   (1<<1)
#define MFLAG_REPORT      (1<<2)
#define MFLAG_CRYPT       (1<<3)
#define MFLAG_SIGNED      (1<<4)
#define MFLAG_SENDERINFO  (1<<5)
#define MFLAG_SENDMDN     (1<<6)
#define MFLAG_NOFOLDER    (1<<7)
#define MFLAG_PERFIELD         8      // reserve 3 bits of the flags for the permanent flag section
#define MFLAG_VOLFIELD        11      // reserve 3 bits of the flags for the volatile flag section
#define MFLAG_IMPORTANCE      14      // reserve 2 bits to store the level of importance
#define isMultiRCPTMail(mail)         (isFlagSet((mail)->mflags, MFLAG_MULTIRCPT))
#define isMultiPartMail(mail)         (isFlagSet((mail)->mflags, MFLAG_MULTIPART))
#define isReportMail(mail)            (isFlagSet((mail)->mflags, MFLAG_REPORT))
#define isCryptedMail(mail)           (isFlagSet((mail)->mflags, MFLAG_CRYPT))
#define isSignedMail(mail)            (isFlagSet((mail)->mflags, MFLAG_SIGNED))
#define isSenderInfoMail(mail)        (isFlagSet((mail)->mflags, MFLAG_SENDERINFO))
#define isSendMDNMail(mail)           (isFlagSet((mail)->mflags, MFLAG_SENDMDN))
#define isVirtualMail(mail)           (isFlagSet((mail)->mflags, MFLAG_NOFOLDER))
#define setPERValue(mail, val)        (mail)->mflags = ((mail)->mflags & ~(7<<MFLAG_PERFIELD)) | ((val) << MFLAG_PERFIELD)
#define setVOLValue(mail, val)        (mail)->mflags = ((mail)->mflags & ~(7<<MFLAG_VOLFIELD)) | ((val) << MFLAG_VOLFIELD)
#define setImportanceLevel(mail, val) (mail)->mflags = ((mail)->mflags & ~(3<<MFLAG_IMPORTANCE)) | ((val) << MFLAG_IMPORTANCE)
#define getPERValue(mail)             (((mail)->mflags & (7<<MFLAG_PERFIELD)) >> MFLAG_PERFIELD)
#define getVOLValue(mail)             (((mail)->mflags & (7<<MFLAG_VOLFIELD)) >> MFLAG_VOLFIELD)
#define getImportanceLevel(mail)      (((mail)->mflags & (3<<MFLAG_IMPORTANCE)) >> MFLAG_IMPORTANCE)

// Status information flags of a mail (also partly stored in file comments)
// Warning: Please note that if you change something here you have to make
//          sure to increase the version number of the .index file in YAM_MAf.c!
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
#define setStatusToQueued(mail)     MA_ChangeMailStatus(mail, SFLAG_QUEUED|SFLAG_READ, SFLAG_SENT)
#define setStatusToHold(mail)       MA_ChangeMailStatus(mail, SFLAG_HOLD|SFLAG_READ, SFLAG_NONE)
#define setStatusToSent(mail)       MA_ChangeMailStatus(mail, SFLAG_SENT|SFLAG_READ, SFLAG_QUEUED|SFLAG_HOLD)
#define setStatusToDeleted(mail)    MA_ChangeMailStatus(mail, SFLAG_DELETED, SFLAG_NONE)
#define setStatusToMarked(mail)     MA_ChangeMailStatus(mail, SFLAG_MARKED, SFLAG_NONE)
#define setStatusToError(mail)      MA_ChangeMailStatus(mail, SFLAG_ERROR, SFLAG_NONE)

// For compatibility to the old status levels we use the following macros
// But as soon as we have reworked the filename handling we can remove them again
#define hasStatusUnread(mail)       (!hasStatusRead(mail) && !hasStatusNew(mail))
#define hasStatusOld(mail)          (hasStatusRead(mail) && !hasStatusNew(mail))
#define setStatusToUnread(mail)     MA_ChangeMailStatus(mail, SFLAG_NONE, SFLAG_NEW|SFLAG_READ)
#define setStatusToOld(mail)        MA_ChangeMailStatus(mail, SFLAG_READ, SFLAG_NEW)

enum ImportanceLevel { IMP_NORMAL=0, IMP_LOW, IMP_HIGH };
enum ApplyMode { APPLY_USER, APPLY_AUTO, APPLY_SENT, APPLY_REMOTE, APPLY_RX_ALL, APPLY_RX };
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

struct MA_GUIData
{
   APTR WI;
   APTR MN_FOLDER;
   APTR MN_REXX;
   APTR MS_MAIN;
   APTR BC_GROUP;
   APTR BC_STAT[MAXBCSTATUSIMG];
   APTR BC_FOLDER[MAXBCFOLDERIMG];
   APTR ST_LAYOUT;
   APTR MI_ERRORS;
   APTR MI_CSINGLE;
   APTR MI_IMPORT;
   APTR MI_EXPORT;
   APTR MI_SENDALL;
   APTR MI_EXCHANGE;
   APTR MI_GETMAIL;
   APTR MI_READ;
   APTR MI_EDIT;
   APTR MI_MOVE;
   APTR MI_COPY;
   APTR MI_DELETE;
   APTR MI_PRINT;
   APTR MI_SAVE;
   APTR MI_ATTACH;
   APTR MI_SAVEATT;
   APTR MI_REMATT;
   APTR MI_EXPMSG;
   APTR MI_REPLY;
   APTR MI_FORWARD;
   APTR MI_BOUNCE;
   APTR MI_GETADDRESS;
   APTR MI_STATUS;
   APTR MI_TOREAD;
   APTR MI_TOUNREAD;
   APTR MI_TOHOLD;
   APTR MI_TOQUEUED;
   APTR MI_TOMARKED;
   APTR MI_TOUNMARKED;
   APTR MI_CHSUBJ;
   APTR MI_SEND;
   APTR LV_FOLDERS;
   APTR NL_FOLDERS;
   APTR LV_MAILS;
   APTR NL_MAILS;
   APTR TO_TOOLBAR;
   APTR IB_INFOBAR;
   APTR GR_MAIN;
   APTR GR_TOP;
   APTR GR_BOTTOM;
   struct MUIP_Toolbar_Description TB_TOOLBAR[18];
};

struct MA_ClassData  /* main window */
{
   struct MA_GUIData GUI;
   char WinTitle[SIZE_DEFAULT];
};

// MailList class instance data
struct ML_Data
{
   Object *context_menu;
};


extern struct Hook MA_ApplyRulesHook;
extern struct Hook MA_ChangeSelectedHook;
extern struct Hook MA_DeleteDeletedHook;
extern struct Hook MA_DeleteOldHook;
extern struct Hook MA_LV_Cmp2Hook;
extern struct Hook MA_LV_DspFuncHook;
extern struct Hook MA_RescanIndexHook;
extern struct Hook MA_SendHook;
extern struct Hook MA_SetFolderInfoHook;
extern struct Hook MA_SetMessageInfoHook;
extern struct Hook PO_WindowHook;
extern struct Hook MA_FolderKeyHook;

int   MA_AllocRules(struct Search **search, enum ApplyMode mode);
void  MA_ChangeSubject(struct Mail *mail, char *subj);
void  MA_ChangeTransfer(BOOL on);
struct Mail **MA_CreateMarkedList(APTR lv, BOOL onlyNew);
void  MA_DeleteMessage(BOOL delatonce, BOOL force);
void  MA_DeleteSingle(struct Mail *mail, BOOL forceatonce, BOOL quiet);
BOOL  MA_ExecuteRuleAction(struct Rule *rule, struct Mail *mail);
BOOL  MA_ExportMessages(BOOL all, char *filename, BOOL append);
void  MA_FreeRules(struct Search **search, int scnt);
struct Mail *MA_GetActiveMail(struct Folder *forcefolder, struct Folder **folderp, int *activep);
void  MA_GetAddress(struct Mail **mlist);
BOOL  MA_ImportMessages(char *fname);
struct MA_ClassData *MA_New(void);
BOOL  MA_SortWindow(void);
void  MA_MakeMAFormat(APTR lv);
void  MA_MoveCopy(struct Mail *mail, struct Folder *frombox, struct Folder *tobox, BOOL copyit);
int   MA_NewBounce(struct Mail *mail, int flags);
int   MA_NewEdit(struct Mail *mail, int flags, int ReadwinNum);
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
void  MA_SetStatusTo(int addflags, int clearflags);
void  MA_SetupDynamicMenus(void);
BOOL  MA_StartMacro(enum Macro num, char *param);
char *MA_ToStatusHeader(struct Mail *mail);
char *MA_ToXStatusHeader(struct Mail *mail);

// real methods (will be separated later in own ML class
ULONG MA_MLContextMenuBuild(struct IClass *cl, Object *obj, struct MUIP_NList_ContextMenuBuild *msg);
ULONG MA_MLContextMenuChoice(struct IClass *cl, Object *obj, struct MUIP_ContextMenuChoice *msg);

#endif /* YAM_MAIN_H */
