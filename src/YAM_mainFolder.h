#ifndef YAM_MAINFOLDER_H
#define YAM_MAINFOLDER_H 1

/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2015 YAM Open Source Team

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

#include "timeval.h"

#include "YAM_utilities.h"

// forward declarations
struct Folder;
struct UserIdentityNode;

struct Mail
{
  short            RefCounter; // how many struct MailNode are referencing us?
  struct Folder *  Folder;     // pointer to the folder this mail belongs to
  unsigned long    cMsgID;     // compressed message ID
  unsigned long    cIRTMsgID;  // compressed in-return-to message ID
  long             Size;       // the message size in bytes
  unsigned int     mflags;     // internal mail flags (no status flags)
  unsigned int     sflags;     // mail status flags (read/new etc.)
  int              position;   // current position of the mail (various uses)
  short            gmtOffset;  // the offset to GMT this mail is based on
  struct DateStamp Date;       // the datestamp of the mail (UTC)
  struct TimeVal   transDate;  // the date/time when this messages arrived/was sent. (UTC)
  struct Person    From;       // The main sender (normally first entry in "From:")
  struct Person    To;         // The main mail recipient (first entry in "To:")
  struct Person    ReplyTo;    // The main Reply-To recipients (first entry in "Reply-To:")

  char tzAbbr[SIZE_SMALL];        // copy of the timezone abbreviation
  char MailAccount[SIZE_DEFAULT]; // name of mail account used to receive/sent mail
  char Subject[SIZE_SUBJECT];     // copy of the mail Subject: header
  char MailFile[SIZE_MFILE];      // name of mail file (without path)
};

struct ExtendedMail
{
  struct Mail              Mail;
  struct Person *          SFrom;          // ptr to an array of additional "From:" senders (excluding the main From:)
  struct Person *          STo;            // ptr to an array of additional "To:" recipients (excluding the main To:)
  struct Person *          SReplyTo;       // ptr to an array of additional "Reply-To:" recipients (excluding the main Reply-To:)
  struct Person *          CC;             // ptr to an array of all "CC:" recipients
  struct Person *          BCC;            // ptr to an array of all "BCC:" recipients
  struct Person *          ResentTo;       // ptr to an array of "Resent-To:" recipients
  struct Person *          ResentCC;       // ptr to an array of "Resent-CC:" recipients
  struct Person *          ResentBCC;      // ptr to an array of "Resent-BCC:" recipients
  struct Person *          FollowUpTo;     // ptr to an array of "Mail-Followup-To:" recipients
  struct Person *          MailReplyTo;    // ptr to an array of "Mail-Reply-To:" recipients
  char *                   extraHeaders;   // YAM internal headers (X-YAM-Header-...)
  char *                   SenderInfo;
  char *                   messageID;      // the composed "Message-ID:" (only one MsgID)
  char *                   inReplyToMsgID; // the composed "In-Reply-To:" (a set of multiple IDs)
  char *                   references;     // the composed "References:" (a set of multiple IDs)
  int                      NumSFrom;       // number of additional senders in SFrom (minus one)
  int                      NumSTo;         // number of additional recipients in STo (minus one)
  int                      NumSReplyTo;    // number of additional recipients in SReplyTo (minus one)
  int                      NumCC;          // number of recipients in CC
  int                      NumBCC;         // number of recipients in BCC
  int                      NumResentTo;    // number of recipients in ResentTo
  int                      NumResentCC;    // number of recipients in ResentCC
  int                      NumResentBCC;   // number of recipients in ResentBCC
  int                      NumFollowUpTo;  // number of recipients in FollowUpTo
  int                      NumMailReplyTo; // number of recipients in MailReplyTo
  int                      Security;
  int                      signatureID;    // id of the signature found in the mail (X-YAM-Options:)
  struct SignatureNode *   signature;      // ptr to matched signature (can also be NULL for no sig)
  int                      identityID;     // id of the user identity found in the mail (X-YAM-Options:)
  struct UserIdentityNode *identity;       // ptr to matched identity (can also be the default id)
  BOOL                     DelSent;
  BOOL                     Redirect;
  struct Person            ReturnPath;     // the "Return-Path" address of the mail, if present
  struct Person            ReceiptTo;      // the recipient in for a requested MDN
  struct Person            OriginalRcpt;   // the original recipient for a requested MDN
};

// MA_ReadHeader modes
enum ReadHeaderMode
{
  RHM_MAINHEADER=0, // we are reading the main header of a mail
  RHM_SUBHEADER,    // we are reading a sub header of a mimepart of a mail
};

void  MA_ChangeFolder(struct Folder *folder, BOOL set_active);
void  MA_ExpireIndex(struct Folder *folder);
struct ExtendedMail *MA_ExamineMail(const struct Folder *folder, const char *file, const BOOL deep);
void  MA_FreeEMailStruct(struct ExtendedMail *email);
BOOL  MA_GetIndex(struct Folder *folder);
enum LoadedMode MA_LoadIndex(struct Folder *folder, BOOL full);
BOOL  MA_NewMailFile(const struct Folder *folder, char *fullPath, const size_t fullPathSize);
BOOL  MA_PromptFolderPassword(struct Folder *fo, APTR win);
BOOL  MA_ReadHeader(const char *mailFile, FILE *fh, struct MinList *headerList, enum ReadHeaderMode mode);
BOOL  MA_SaveIndex(struct Folder *folder);
void  MA_RebuildIndexes(void);
void  MA_UpdateInfoBar(struct Folder *folder);
struct Mail *FindMailByMsgID(struct Folder *folder, const char *msgid);
void MoveHeldMailsToDraftsFolder(void);

#endif /* YAM_MAINFOLDER_H */
