#ifndef DYNAMICSTRING_H
#define DYNAMICSTRING_H 1

/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2016 YAM Open Source Team

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

#include <stdlib.h>
#include <stdio.h>

// prototypes of our dstrXXX() functions which
// deal with dynamic string functionality and try
// to be compatible to the standard functions like strcpy()

char *dstralloc(size_t initsize);
void dstrreset(const char *dstr);
char *dstrcpy(char **dstr, const char *src);
char *dstrcat(char **dstr, const char *src);
char *dstrins(char **dstr, const char *src, size_t pos);
size_t dstrlen(const char *dstr);
size_t dstrsize(const char *dstr);
size_t dstrfread(char **dstr, size_t size, FILE *fh);
void dstrfree(const char *dstr);

#endif /* DYNAMICSTRING_H */
