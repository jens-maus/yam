#ifndef YAM_MIME_H
#define YAM_MIME_H

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

#include "YAM_stringsizes.h"

struct TranslationTable
{
   BOOL  Header;
   UBYTE Table[256];
   char  Name[SIZE_DEFAULT];
   char  File[SIZE_PATHFILE];
   char  SourceCharset[SIZE_NAME];
   char  DestCharset[SIZE_NAME];
};

char *decode64 (char *dest, const char *src, char *srcmax);
BOOL  DoesNeedPortableNewlines(char *ctype);
void  encode64(const unsigned char *s, char *d, int len);
void  from64(FILE *infile, FILE *outfile, struct TranslationTable *tt, BOOL PortableNewlines);
void  from64txt(char *src, char *dst, struct TranslationTable *tt);
void  fromform(FILE *infile, FILE *outfile, struct TranslationTable *tt);
void  fromuue(FILE *infp, FILE *outfp);
void  fromuuetxt(char **txt, FILE *outfp);
void  fromqp(FILE *infile, FILE *outfile, struct TranslationTable *tt);
void  fromqptxt(char *src, char *dst, struct TranslationTable *tt);
void  to64(FILE *infile, FILE *outfile, BOOL PortableNewlines);
void  toqp(FILE *infile, FILE *outfile);
void  touue(FILE *in, FILE *out);

#endif /* YAM_MIME_H */
