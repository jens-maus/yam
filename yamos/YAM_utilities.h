#ifndef YAM_UTILITIES_H
#define YAM_UTILITIES_H

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

#include <stdio.h>
#include <time.h>

#include "YAM_stringsizes.h"

enum DateStampType { DSS_DATE, DSS_TIME, DSS_WEEKDAY, DSS_DATETIME,
  DSS_USDATETIME, DSS_UNIXDATE, DSS_BEAT, DSS_DATEBEAT };

struct Person
{       
   char Address[SIZE_ADDRESS];
   char RealName[SIZE_REALNAME];
};

#define BusyEnd Busy("", NULL, 0, 0)

extern struct Hook GeneralDesHook;

char *   AllocStrBuf(long initlen);
void     AppendLogVerbose(int id, char *text, void *a1, void *a2, void *a3, void *a4);
char *   BuildAddrName(char *address, char *name);
char *   BuildAddrName2(struct Person *pe);
void     Busy(char *text, char *parameter, int cur, int max);
BOOL     CopyFile(char *dest, FILE *destfh, char *sour, FILE *sourfh);
BOOL     CreateDirectory(char *dir);
char *   DateStamp2String(struct DateStamp *date, enum DateStampType mode);
void     DeleteMailDir(char *dir, BOOL isroot);
char *   Decrypt(char *source);
void     DisposeModulePush(void *module);
void     DisposeModule(void *modptr);
char *   Encrypt(char *source);
void     ExtractAddress(char *line, struct Person *pe);
int      FileType(char *filename);
void     FreeStrBuf(char *strbuf);
time_t   GetDateStamp(void);
char *   GetLine(FILE *fh, char *buffer, int bufsize);
int      GetMUI(Object *obj, int attr);
BOOL     GetMUICheck(Object *obj);
void     GetMUIString(char *a, Object *obj);
int      GetSimpleID(void);
char *   GetTZ(void);
char *   IdentifyFile(char *fname);
Object * MakeButton(char *txt);
Object * MakeCheckGroup(Object **check, char *label);
Object * MakePassString(char *label);
Object * MakeString(int maxlen, char *label);
char *   MyStrChr(char *s, int c);
BOOL     SafeOpenWindow(Object *obj);
void     SetHelp(APTR object, APTR strnum);
void     SPrintF(char *outstr, char *fmtstr, ...);
char *   StrBufCat(char *strbuf, char *source);
char *   StrBufCpy(char *strbuf, char *source);
int      StringRequest(char *string, int size, char *title, char *body,
         char *yestext, char *alttext, char *notext, BOOL secret, APTR parent);
char *   Trim(char *s);

#define MyStrCpy(a,b) { strncpy((a),(b),sizeof(a)); (a)[sizeof(a)-1] = 0; }

#endif /* YAM_UTILITIES_H */
