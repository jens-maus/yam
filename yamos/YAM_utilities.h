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

#include <time.h>

#include "YAM_stringsizes.h"

struct Person
{       
   char Address[SIZE_ADDRESS];
   char RealName[SIZE_REALNAME];
};

char *   AllocStrBuf(long initlen);
char *   BuildAddrName(char *address, char *name);
char *   BuildAddrName2(struct Person *pe);
void     ExtractAddress(char *line, struct Person *pe);
void     FreeStrBuf(char *strbuf);
time_t   GetDateStamp(void);
void     GetMUIString(char *a, struct Object *obj);
char *   GetTZ(void);
char *   IdentifyFile(char *fname);
Object * MakeButton(char *txt);
Object * MakePassString(char *label);
Object * MakeString(int maxlen, char *label);
char *   MyStrChr(char *s, int c);
BOOL     SafeOpenWindow(struct Object *obj);
char *   StrBufCat(char *strbuf, char *source);
char *   Trim(char *s);

#define MyStrCpy(a,b) { strncpy(a,b,sizeof(a)); a[sizeof(a)-1] = 0; }


#endif /* YAM_UTILITIES_H */
