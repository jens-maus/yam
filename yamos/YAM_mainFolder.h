#ifndef YAM_MAINFOLDER_H
#define YAM_MAINFOLDER_H

/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2002 by YAM Open Source Team

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

#include "YAM_folderconfig.h"
#include "YAM_utilities.h"

enum MailStatus { STATUS_UNR, STATUS_OLD, STATUS_FWD, STATUS_RPD, STATUS_WFS,
                  STATUS_ERR, STATUS_HLD, STATUS_SNT, STATUS_NEW, STATUS_DEL
                };

struct Mail
{
   struct Mail *    Next;
   struct Mail *    Reference;
   struct Folder *  Folder;
   char *           UIDL;
   long             cMsgID;
   long             cIRTMsgID;
   long             Size;
   int              Flags;
   int              Position;
   int              Index;
   struct DateStamp Date;
   struct timeval   transDate;  // the date/time when this messages arrived/was sent.
   struct Person    From;
   struct Person    To;
   struct Person    ReplyTo;

   enum MailStatus  Status;
   char             Importance;
   char             Subject[SIZE_SUBJECT];
   char             MailFile[SIZE_MFILE];
};

enum ReceiptType { RCPT_TYPE_ALL=1, RCPT_TYPE_READ };

struct ExtendedMail
{
   struct Mail      Mail;
   struct Person *  STo;
   struct Person *  CC;
   struct Person *  BCC;
   char *           Headers;
   char *           SenderInfo;
   int              NoSTo;
   int              NoCC;
   int              NoBCC;
   int              Signature;
   int              Security;
   enum ReceiptType ReceiptType;
   BOOL             DelSend;
   BOOL             RetRcpt;
   struct Person    ReceiptTo;
   struct Person    OriginalRcpt;

   char             MsgID[SIZE_MSGID];
   char             IRTMsgID[SIZE_MSGID];
};

// FolderList class instance data
struct FL_Data
{
   Object *context_menu;
};

extern struct Data2D Header;
extern struct Hook   MA_ChangeFolderHook;
extern struct Hook   MA_FlushIndexHook;
extern struct Hook   MA_LV_FDspFuncHook;
extern struct Hook   PO_InitFolderListHook;

void   MA_ChangeFolder(struct Folder *folder, BOOL set_active);
void   MA_ExpireIndex(struct Folder *folder);
struct ExtendedMail *MA_ExamineMail(struct Folder *folder, char *file, char *statstr, BOOL deep);
void   MA_FlushIndexes(BOOL all);
BOOL   MA_JumpToNewMsg(VOID);
void   MA_FreeEMailStruct(struct ExtendedMail *email);
BOOL   MA_GetIndex(struct Folder *folder);
enum LoadedMode MA_LoadIndex(struct Folder *folder, BOOL full);
void   MA_MakeFOFormat(APTR lv);
char * MA_NewMailFile(struct Folder *folder, char *mailfile, int daynumber);
BOOL   MA_PromptFolderPassword(struct Folder *fo, APTR win);
BOOL   MA_ReadHeader(FILE *fh);
BOOL   MA_SaveIndex(struct Folder *folder);
void   MA_ScanMailBox(struct Folder *folder);
void   MA_UpdateIndexes(BOOL initial);
void   MA_UpdateInfoBar(struct Folder *);

// real methods (will be separated later in own FL class
ULONG MA_FLContextMenuBuild(struct IClass *cl, Object *obj, struct MUIP_NList_ContextMenuBuild *msg);
ULONG MA_FLContextMenuChoice(struct IClass *cl, Object *obj, struct MUIP_ContextMenuChoice *msg);

#endif /* YAM_MAINFOLDER_H */
