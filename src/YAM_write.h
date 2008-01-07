#ifndef YAM_WRITE_H
#define YAM_WRITE_H

/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2008 by YAM Open Source Team

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

#include "YAM_main.h"
#include "YAM_stringsizes.h"

#ifndef WORKBENCH_WORKBENCH_H
struct AppMessage;
#endif

struct WR_GUIData
{
  Object *WI;
  Object *RG_PAGE;
  Object *ST_TO;
  Object *ST_SUBJECT;
  Object *TX_POSI;
  Object *TE_EDIT;
  Object *TO_TOOLBAR;
  Object *LV_ATTACH;
  Object *BT_ADD;
  Object *BT_ADDPACK;
  Object *BT_DEL;
  Object *BT_DISPLAY;
  Object *RA_ENCODING;
  Object *CY_CTYPE;
  Object *ST_CTYPE;
  Object *ST_DESC;
  Object *ST_CC;
  Object *ST_BCC;
  Object *ST_FROM;
  Object *ST_REPLYTO;
  Object *ST_EXTHEADER;
  Object *CH_DELSEND;
  Object *CH_MDN;
  Object *CH_ADDINFO;
  Object *CY_IMPORTANCE;
  Object *RA_SECURITY;
  Object *CH_DEFSECURITY;
  Object *RA_SIGNATURE;
  Object *BT_HOLD;
  Object *BT_QUEUE;
  Object *BT_SEND;
  Object *BT_CANCEL;
  Object *MI_BOLD;
  Object *MI_ITALIC;
  Object *MI_UNDERLINE;
  Object *MI_COLORED;
};

struct WR_ClassData  /* write window */
{
  struct WR_GUIData GUI;

  struct Mail *     refMail;            // ptr to the original mail this write window was created from
  struct Mail **    refMailList;        // ptr to a list of orginal mails.

  enum NewMode      Mode;
  int               OldSecurity;
  int               winnum;             // the window number this class data belongs to

  BOOL              AutoSaved;          // was this mail automatically saved?
  BOOL              FileNotifyActive;   // is a file change notification currently active or not

  char              MsgID[SIZE_MSGID];
  char              CursorPos[SIZE_DEFAULT];
  char              WTitle[SIZE_SUBJECT+1];
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

enum TransformMode
{
  ED_OPEN=0, ED_INSERT, ED_INSQUOT, ED_INSALTQUOT, ED_INSROT13, ED_PASQUOT, ED_PASALTQUOT,
  ED_PASROT13, ED_INSUUCODE
};

enum WriteMode { WRITE_HOLD, WRITE_SEND, WRITE_QUEUE };

enum Encoding { ENC_NONE, ENC_QP, ENC_B64, ENC_UUE, ENC_BIN, ENC_8BIT };

struct WritePart
{
  struct WritePart *Next;
  const char *      ContentType;
  char *            Filename;
  char *            Description;
  char *            Name;
  BOOL              IsTemp;
  enum Encoding     EncType;
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
  const char *       Subject;
  char *             ExtHeader;
  char *             IRTMsgID;
  struct WritePart * FirstPart;
  struct Mail *      refMail;    // ptr to the original mail we composing a new one from.
  int                Mode;
  int                Importance;
  int                Signature;
  BOOL               RequestMDN;  // should an MDN be requested
  BOOL               GenerateMDN; // should and MDN report be generated?
  BOOL               DelSend;
  BOOL               UserInfo;
  enum Security      Security;
  enum Security      OldSecurity;
};

// Soft-style modes for text
enum SoftStyleMode { SSM_NORMAL, SSM_BOLD, SSM_ITALIC, SSM_UNDERLINE, SSM_COLOR };

extern struct Hook WR_EditHook;
extern struct Hook WR_NewMailHook;
extern struct Hook WR_SetSoftStyleHook;
extern struct Hook WR_SearchHook;
extern struct Hook WR_EditorCmdHook;

void  EmitHeader(FILE *fh, const char *hdr, const char *body);
void  FreePartsList(struct WritePart *p);
struct WritePart *NewPart(int winnum);
BOOL  WR_AddFileToList(int winnum, const char *filename, const char *name, BOOL istemp);
void  WR_AddSignature(int winnum, int signat);
void  WR_App(int winnum, STRPTR fileName);
char *WR_AutoSaveFile(const int winnr, char *dest, const size_t length);
void  WR_Cleanup(int winnum);
void  WR_NewMail(enum WriteMode mode, int winnum);
int   WR_Open(int winnum, BOOL bounce);
void  WR_SetupOldMail(int winnum, struct ReadMailData *rmData);
BOOL  WriteOutMessage(struct Compose *comp);

#endif /* YAM_WRITE_H */
