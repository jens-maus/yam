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

#define ANYBOX NULL

#define MFLAG_MULTIRCPT   1
#define MFLAG_MULTIPART   2
#define MFLAG_REPORT      4
#define MFLAG_CRYPT       8
#define MFLAG_SIGNED     16
#define MFLAG_SENDERINFO 32
#define MFLAG_SENDMDN    64
#define MFLAG_NOFOLDER  128
#define Virtual(mail)   (((mail)->Flags&MFLAG_NOFOLDER) == MFLAG_NOFOLDER)

enum ApplyMode { APPLY_USER, APPLY_AUTO, APPLY_SENT, APPLY_REMOTE,
   APPLY_RX_ALL, APPLY_RX
};

enum NewMode {
   NEW_NEW, NEW_REPLY, NEW_FORWARD, NEW_BOUNCE, NEW_EDIT, NEW_SAVEDEC
};

#define NEWF_QUIET        1
#define NEWF_REP_NOQUOTE  2
#define NEWF_REP_PRIVATE  4
#define NEWF_REP_MLIST    8
#define NEWF_FWD_NOATTACH 16

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
   APTR BC_STAT[17];
   APTR BC_FOLDER[MAXBCSTDIMAGES];
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
struct Mail **MA_CreateMarkedList(APTR lv);
void  MA_DeleteMessage(BOOL delatonce, BOOL force);
void  MA_DeleteSingle(struct Mail *mail, BOOL forceatonce);
BOOL  MA_ExecuteRuleAction(struct Rule *rule, struct Mail *mail);
BOOL  MA_ExportMessages(BOOL all, char *filename, BOOL append);
void  MA_FreeRules(struct Search **search, int scnt);
struct Mail *MA_GetActiveMail(struct Folder *forcefolder, struct Folder **folderp, int *activep);
void  MA_GetAddress(struct Mail **mlist);
BOOL  MA_ImportMessages(char *fname);
struct MA_ClassData *MA_New(void);
ULONG MA_MailListContextMenu(struct MUIP_ContextMenuBuild *msg);
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
void  MA_RemoveAttach(struct Mail *mail);
BOOL  MA_Send(enum SendMode sendpos);
BOOL  MA_SendMList(struct Mail **mlist);
void  MA_SetMailStatus(struct Mail *mail, enum MailStatus stat);
void  MA_SetSortFlag(void);
void  MA_SetStatusTo(int status);
void  MA_SetupDynamicMenus(void);
BOOL  MA_StartMacro(enum Macro num, char *param);

#endif /* YAM_MAIN_H */
