#ifndef YAM_MAINFOLDER_H
#define YAM_MAINFOLDER_H

/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2007 by YAM Open Source Team

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

struct Mail
{
   struct Mail *    Next;       // pointer to the next mail or NULL
   struct Mail *    Reference;  // pointer to the mail referencing to us
   struct Folder *  Folder;     // pointer to the folder this mail belongs to
   long             cMsgID;     // compressed message ID
   long             cIRTMsgID;  // compressed in-return-to message ID
   long             Size;       // the message size in bytes
   unsigned int     mflags;     // internal mail flags (no status flags)
   unsigned int     sflags;     // mail status flags (read/new etc.)
   int              position;   // current position of the mail (various uses)
   int              tzone;      // the timezone which this mail is based on
   struct DateStamp Date;       // the datestamp of the mail (UTC)
   struct TimeVal   transDate;  // the date/time when this messages arrived/was sent. (UTC)
   struct Person    From;       // The main sender (normally first entry in "From:")
   struct Person    To;         // The main mail recipient (first entry in "To:")
   struct Person    ReplyTo;    // The main Reply-To recipients (first entry in "Reply-To:")

   char             Subject[SIZE_SUBJECT];
   char             MailFile[SIZE_MFILE];
};

enum ReceiptType { RCPT_TYPE_ALL=1, RCPT_TYPE_READ };

struct ExtendedMail
{
   struct Mail      Mail;
   struct Person *  SFrom;        // ptr to an array of additional "From:" senders (excluding the main From:)
   struct Person *  STo;          // ptr to an array of additional "To:" recipients (excluding the main To:)
   struct Person *  SReplyTo;     // ptr to an array of additional "Reply-To:" recipients (excluding the main Reply-To:)
   struct Person *  CC;           // ptr to an array of all "CC:" recipients
   struct Person *  BCC;          // ptr to an array of all "BCC:" recipients
   char *           extraHeaders; // YAM internal headers (X-YAM-...)
   char *           SenderInfo;
   int              NoSFrom;      // number of additional senders in SFrom (minus one)
   int              NoSTo;        // number of additional recipients in STo (minus one)
   int              NoSReplyTo;   // number of additional recipients in SReplyTo (minus one)
   int              NoCC;         // number of recipients in CC
   int              NoBCC;        // number of recipients in BCC
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

extern struct Hook MA_ChangeFolderHook;
extern struct Hook MA_FlushIndexHook;
extern struct Hook MA_LV_FDspFuncHook;

void  MA_ChangeFolder(struct Folder *folder, BOOL set_active);
void  MA_ExpireIndex(struct Folder *folder);
struct ExtendedMail *MA_ExamineMail(struct Folder *folder, char *file, BOOL deep);
void  MA_FlushIndexes(BOOL all);
void  MA_JumpToNewMsg(VOID);
void  MA_JumpToRecentMsg(VOID);
void  MA_FreeEMailStruct(struct ExtendedMail *email);
BOOL  MA_GetIndex(struct Folder *folder);
enum LoadedMode MA_LoadIndex(struct Folder *folder, BOOL full);
void  MA_MakeFOFormat(Object *lv);
char *MA_NewMailFile(struct Folder *folder, char *mailfile);
BOOL  MA_PromptFolderPassword(struct Folder *fo, APTR win);
BOOL  MA_ReadHeader(FILE *fh, struct MinList *headerList);
BOOL  MA_SaveIndex(struct Folder *folder);
void  MA_UpdateIndexes(BOOL initial);
void  MA_UpdateInfoBar(struct Folder *);

#endif /* YAM_MAINFOLDER_H */
