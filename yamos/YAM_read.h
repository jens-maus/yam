#ifndef YAM_READ_H
#define YAM_READ_H

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

#include "SDI_compiler.h"

#include "YAM_mainFolder.h"
#include "YAM_write.h"

#define MDN_TYPEMASK  0x0F
#define MDN_AUTOACT   0x10
#define MDN_AUTOSEND  0x20

// special defines for Part Types
#define PART_ORIGINAL -2
#define PART_ALLTEXT  -1
#define PART_RAW      0
#define PART_LETTER   (C->LetterPart)

enum MDNType    { MDN_IGNORE=0, MDN_DENY, MDN_READ, MDN_DISP, MDN_PROC, MDN_DELE };
enum ParseMode  { PM_ALL, PM_TEXTS, PM_NONE };
enum ReadInMode { RIM_QUIET, RIM_READ, RIM_EDIT, RIM_QUOTE, RIM_PRINT };

struct RE_GUIData
{
   APTR WI;
   APTR MI_EDIT;
   APTR MI_DETACH;
   APTR MI_CROP;
   APTR MI_WRAPH;
   APTR MI_TSTYLE;
   APTR MI_FFONT;
   APTR MI_EXTKEY;
   APTR MI_CHKSIG;
   APTR MI_SAVEDEC;
   APTR GR_BODY;
   APTR GR_HEAD;
   APTR LV_HEAD;
   APTR TO_TOOLBAR;
   APTR TE_TEXT;
   APTR SL_TEXT;
   APTR GR_STATUS[3];
   APTR GR_INFO;
   APTR GR_PHOTO;
   APTR BC_PHOTO;
   APTR LV_INFO;
   APTR BO_BALANCE;
   struct MUIP_Toolbar_Description TB_TOOLBAR[14];
};

struct RE_ClassData  /* read window */
{
   struct RE_GUIData GUI;
   struct Mail      *MailPtr;
   struct TempFile  *TempFile;
   FILE             *Fh;
   struct Part      *FirstPart;
   enum ParseMode    ParseMode;
   int               Header;
   int               SenderInfo;
   int               LastDirection;
   int               PGPSigned;
   int               PGPEncrypted;
   BOOL              FirstReadDone;
   BOOL              NoTextstyles;
   BOOL              WrapHeader;
   BOOL              FixedFont;
   BOOL              PGPKey;

   struct Mail       Mail;

   char              File[SIZE_PATHFILE];
   char              Signature[SIZE_ADDRESS];
   char              WTitle[SIZE_DEFAULT];
};

struct Part
{
   struct Part * Prev;
   struct Part * Next;
   struct Part * NextSelected;
   char *        ContentType;
   char *        ContentDisposition;
   char *        JunkParameter;
   char *        CParName;
   char *        CParBndr;
   char *        CParProt;
   char *        CParDesc;
   char *        CParRType;
   char *        CParCSet;
   int           MaxHeaderLen;
   int           Size;
   int           Nr;
   int           Win;
   BOOL          HasHeaders;
   BOOL          Printable;
   BOOL          Decoded;
   enum Encoding EncodingCode;

   char          Name[SIZE_FILE];
   char          Description[SIZE_DEFAULT];
   char          Filename[SIZE_PATHFILE];
   char          Boundary[SIZE_DEFAULT];
};

extern struct Hook RE_CloseHook;
extern struct Hook RE_LV_AttachDspFuncHook;

void  RE_CleanupMessage(int winnum);
BOOL  RE_DecodePart(struct Part *rp);
void  RE_DisplayMIME(char *fname, char *ctype);
BOOL  RE_DoMDN(int MDNtype, struct Mail *mail, BOOL multi);
BOOL  RE_Export(int winnum, char *source, char *dest, char *name, int nr, BOOL force, BOOL overwrite, char *ctype);
void  RE_FreePrivateRC(void);
void  RE_InitPrivateRC(struct Mail *mail, enum ParseMode parsemode);
int   RE_Open(int winnum, BOOL real);
void STACKEXT RE_ProcessHeader(char *prevcharset, char *s, BOOL ShowLeadingWhitespace, char *ptr);
char *RE_ReadInMessage(int winnum, enum ReadInMode mode);
void  RE_ReadMessage(int winnum, struct Mail *mail);
void  RE_SaveAll(int winnum, char *path);
void  RE_SaveDisplay(int winnum, FILE *fh);

#endif /* YAM_READ_H */
