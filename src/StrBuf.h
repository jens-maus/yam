#ifndef STRBUF_H
#define STRBUF_H 1

/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2013 YAM Open Source Team

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

char * AllocStrBuf(size_t initsize);
void ResetStrBuf(char *buf);
size_t StrBufCat(char **buf, const char *source);
size_t StrBufCpy(char **buf, const char *source);
size_t StrBufLength(char *buf);
size_t StrBufRead(char **buf, FILE *fh, size_t size);
void FreeStrBuf(char *buf);

#endif /* STRBUF_H */
