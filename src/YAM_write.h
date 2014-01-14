#ifndef YAM_WRITE_H
#define YAM_WRITE_H

/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2014 YAM Open Source Team

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

#include <stdio.h>
#include <dos/dos.h>

#include "YAM_main.h"
#include "YAM_stringsizes.h"

// forward declarations
struct AppMessage;
struct codeset;
struct DateStamp;
struct Mail;
struct MailList;
struct ReadMailData;
struct SignatureNode;
struct UserIdentityNode;

// enumeration with security levels a mail can
// get so that it will be signed/encrypted and so on.
// (the order has to match the SecCodes[] array)
enum Security
{
  SEC_NONE=0,      // no security
  SEC_SIGN,        // PGP sign the message
  SEC_ENCRYPT,     // PGP encrypt the message
  SEC_BOTH,        // PGP sign&encrypt the message
  SEC_DEFAULTS,    // use the preconfigured defaults
  SEC_MAXDUMMY
};

// WriteMailData structure which carries all necessary information
// during a write mail process. It is used while opening a write
// window, or even during silently creating an email in background
// (for Arexx stuff and so on).
// This structure uses a MinNode for making it possible to place
// all existing WriteMailData structs into the global YAM structure
// in an unlimited list allowing unlimited WriteWindows and such.
struct WriteMailData
{
  struct MinNode  node;                 // required for placing it into struct Global
  Object *        window;               // ptr to the associated window or NULL

  struct Mail *     draftMail;          // ptr to a saved mail in the drafts folder
  struct Mail *     refMail;            // ptr to the original mail this write operation was created from
  struct MailList * refMailList;        // ptr to a list of orginal mails.
  enum NewMailMode  mode;               // the compose mode this write mail operation
  char *            inReplyToMsgID;     // ptr to "In-Reply-To:" message header to compose the message for
  char *            references;         // ptr to "References:" message header to compose the message for
  struct codeset *  codeset;            // the character set being used for this mail

  struct UserIdentityNode *identity;    // ptr to the identity used for composing that mail

  char draftMailFile[SIZE_PATHFILE];    // name of the draft mail file
  char filename[SIZE_PATHFILE];         // filename of tmp text file
  struct DateStamp lastFileChangeTime;  // when was this file touched for the last time
  struct NotifyRequest *notifyRequest;  // file notification request
  BOOL fileNotifyActive;                // is the file change notification currently active or not

  BOOL quietMode;                       // quietMode means no opened window, e.g. background processing
};

struct Attach
{
  int  Size;

  BOOL IsTemp;

  char FilePath[SIZE_PATHFILE];
  char Name[SIZE_FILE];
  char ContentType[SIZE_CTYPE];
  char Description[SIZE_DEFAULT];
};

enum TransformMode
{
  ED_OPEN=0,
  ED_INSERT,
  ED_INSQUOT,
  ED_INSALTQUOT,
  ED_INSROT13,
  ED_PASQUOT,
  ED_PASALTQUOT,
  ED_PASROT13,
  ED_INSUUCODE
};

// mail text encoding codes
enum Encoding
{
  ENC_7BIT,    // 7-bit US-ASCII
  ENC_8BIT,    // 8-bit enhanced ASCII
  ENC_QP,      // quoted-printable encoding
  ENC_B64,     // base64 encoding
  ENC_UUE,     // uucode encoding
  ENC_BIN      // binary encoding
};

struct WritePart
{
  struct WritePart *Next;
  const char *      ContentType;
  char *            Filename;
  char *            Description;
  char *            Name;
  long              Size;
  struct codeset *  Codeset; // the character set being used for this part
  BOOL              IsAttachment;
  BOOL              IsTemp;
  enum Encoding     EncType;
};

struct Compose
{
  FILE *                    FH;             // ptr to file handle of mail
  char *                    FromOverride;   // overridden From: address
  char *                    MailTo;         // array of To: addresses
  char *                    MailCC;         // array of CC: addresses
  char *                    MailBCC;        // array of BCC: addresses
  struct UserIdentityNode * Identity;       // ptr to the user identity which should be used for sending the mail
  char *                    ReplyTo;        // array of ReplyTo: addresses
  char *                    MailReplyTo;    // array of Mail-Reply-To: addresses (for mailinglists)
  char *                    MailFollowupTo; // array of Mail-Followup-To: addresses (for mailinglists)
  const char *              Subject;        // subject of mail to compose
  char *                    ExtHeader;      // some extra headers to add before sending
  char *                    inReplyToMsgID; // ptr to In-Reply-To MsgIDs
  char *                    references;     // ptr to References MsgIDs
  struct WritePart *        FirstPart;      // ptr to first MIME part of mail
  struct Mail *             refMail;        // ptr to the original mail we composing a new one from.
  enum NewMailMode          Mode;           // the mode this mail was composed of
  int                       Importance;     // the importance level of the mail (low/normal/high)
  struct SignatureNode *    Signature;      // the signature that is used when composing the mail
  BOOL                      RequestMDN;     // should a MDN be requested?
  BOOL                      GenerateMDN;    // should a MDN report be generated?
  BOOL                      DelSend;        // should the mail be deleted after having sent it?
  BOOL                      UserInfo;       // should personal user info be added to the mail?
  enum Security             Security;       // (PGP) security level to be applied to mail
  enum Security             SelSecurity;    // (PGP) security level selected in WriteWindow
  struct codeset          * codeset;        // the codeset to be used
};

// Soft-style modes for text
enum SoftStyleMode
{
  SSM_NORMAL,
  SSM_BOLD,
  SSM_ITALIC,
  SSM_UNDERLINE,
  SSM_COLOR
};

void  EmitHeader(FILE *fh, const char *hdr, const char *body, struct codeset *codeset);
void  FreePartsList(struct WritePart *p, BOOL delTemp);
BOOL  WriteOutMessage(struct Compose *comp);
void WriteContentTypeAndEncoding(FILE *fh, const struct WritePart *part);
const char *EncodingName(const enum Encoding encoding);
BOOL EncodePart(FILE *ofh, const struct WritePart *part);

struct WriteMailData *NewWriteMailWindow(struct Mail *mail, const int flags);
struct WriteMailData *NewRedirectMailWindow(struct MailList *mlist, const int flags);
struct WriteMailData *NewEditMailWindow(struct Mail *mail, const int flags);
struct WriteMailData *NewForwardMailWindow(struct MailList *mlist, const int flags);
struct WriteMailData *NewReplyMailWindow(struct MailList *mlist, const int flags, const char *replytxt);
BOOL SetWriteMailDataMailRef(const struct Mail *search, const struct Mail *newRef);
struct WriteMailData *AllocWriteMailData(void);
void FreeWriteMailData(struct WriteMailData *wmData);
BOOL CleanupWriteMailData(struct WriteMailData *wmData);
struct WritePart *NewMIMEpart(struct WriteMailData *wmData);
void CheckForAutoSaveFiles(void);
void WriteSignature(FILE *out, struct SignatureNode *signature, BOOL separator);

#endif /* YAM_WRITE_H */
