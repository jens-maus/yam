#ifndef YAM_WRITE_H
#define YAM_WRITE_H

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

#include "YAM_stringsizes.h"

#ifndef WORKBENCH_WORKBENCH_H
struct AppMessage;
#endif

struct WR_GUIData
{
   APTR WI;
   APTR RG_PAGE;
   APTR ST_TO;
   APTR ST_SUBJECT;
   APTR TX_POSI;
   APTR TE_EDIT;
   APTR TO_TOOLBAR;
   APTR LV_ATTACH;
   APTR BT_ADD;
   APTR BT_ADDPACK;
   APTR BT_DEL;
   APTR BT_DISPLAY;
   APTR RA_ENCODING;
   APTR CY_CTYPE;
   APTR ST_CTYPE;
   APTR ST_DESC;
   APTR ST_CC;
   APTR ST_BCC;
   APTR ST_FROM;
   APTR ST_REPLYTO;
   APTR ST_EXTHEADER;
   APTR CH_DELSEND;
   APTR CH_RECEIPT;
   APTR CH_DISPNOTI;
   APTR CH_ADDINFO;
   APTR CY_IMPORTANCE;
   APTR RA_SECURITY;
   APTR CH_DEFSECURITY;
   APTR RA_SIGNATURE;
   APTR BT_HOLD;
   APTR BT_QUEUE;
   APTR BT_SEND;
   APTR BT_CANCEL;
   struct MUIP_Toolbar_Description TB_TOOLBAR[15];
};

struct WR_ClassData  /* write window */
{
   struct WR_GUIData GUI;

   struct Mail *     Mail;
   struct Mail **    MList;
   struct ABEntry *  ListEntry;

   int               Mode;
   int               OldSecurity;
   int               AS_Count;
   int               ReadwinNum; /* winnum of the read window the editor was invoked from, or -1 */

   BOOL              Bounce;
   BOOL              AS_Done;

   char              MsgID[SIZE_MSGID];
   char              QuoteText[SIZE_DEFAULT];
   char              AltQuoteText[SIZE_SMALL]; /* no variable substitution -> SIZE_SMALL! */
   char              WTitle[SIZE_DEFAULT];
};

struct Attach
{
   int  Size;

   BOOL IsMIME;
   BOOL IsTemp;

   char FilePath[SIZE_PATHFILE];
   char Name[SIZE_FILE];
   char ContentType[SIZE_CTYPE];
   char Description[SIZE_DEFAULT];
};

enum TransformMode {
   ED_OPEN, ED_INSERT, ED_INSQUOT, ED_INSALTQUOT, ED_INSROT13, ED_PASQUOT, ED_PASALTQUOT,
   ED_PASROT13
};

enum WriteMode { WRITE_HOLD, WRITE_SEND, WRITE_QUEUE };

enum Encoding { ENC_NONE, ENC_QP, ENC_B64, ENC_UUE, ENC_BIN, ENC_8BIT, ENC_FORM };

struct WritePart
{
   struct WritePart *       Next;
   char *                   ContentType;
   char *                   Filename;
   char *                   Description;
   char *                   Name;
   struct TranslationTable *TTable;
   BOOL                     IsTemp;
   enum Encoding            EncType;
};

/* if you add anything here, check the following places for potential changes:
 * YAM_MAf.c:MA_ExamineMail() (X-YAM-options handling)
 * YAM:WR.c:WriteOutMessage() (PGP multipart stuff); WR_New() (GUI elements, menus)
 */
enum Security { SEC_NONE=0, SEC_SIGN, SEC_ENCRYPT, SEC_BOTH, SEC_SENDANON, SEC_DEFAULTS, SEC_MAXDUMMY };

struct Compose
{
   FILE *             FH;
   char *             MailTo;
   char *             MailCC;
   char *             MailBCC;
   char *             From;
   char *             ReplyTo;
   char *             RealName;
   char *             Subject;
   char *             ExtHeader;
   char *             IRTMsgID;
   struct WritePart * FirstPart;
   struct Mail *      OrigMail;
   int                Mode;
   int                Importance;
   int                Receipt;
   int                ReportType;
   int                Signature;
   BOOL               DelSend;
   BOOL               UserInfo;
   enum Security      Security;
   enum Security      OldSecurity;
};

extern struct Hook WR_EditHook;
extern struct Hook WR_NewMailHook;

void  EmitHeader(FILE *fh, char *hdr, char *body);
void  FreePartsList(struct WritePart *p);
struct WritePart *NewPart(int winnum);
BOOL  WR_AddFileToList(int winnum, char *filename, char *name, BOOL istemp);
void  WR_AddSignature(char *mailfile, int signat);
void  WR_App(int winnum, struct AppMessage *amsg);
char *WR_AutoSaveFile(int winnr);
void  WR_Cleanup(int winnum);
char *WR_ExpandAddresses(int winnum, char *src, BOOL quiet, BOOL single, BOOL withrname);
void  WR_NewMail(enum WriteMode mode, int winnum);
int   WR_Open(int winnum, BOOL bounce);
int   WR_ResolveName(int winnum, char *name, char **adrstr, BOOL nolists, BOOL withrname);
void  WR_SetupOldMail(int winnum);
BOOL  WriteOutMessage(struct Compose *comp);

#endif /* YAM_WRITE_H */
