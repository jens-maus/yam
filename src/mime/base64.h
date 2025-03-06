#ifndef BASE64_H
#define BASE64_H

/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2025 YAM Open Source Team

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
 YAM OpenSource project    :  https://github.com/jens-maus/yam/

***************************************************************************/

#include <exec/types.h>

// forward declarations
struct codeset;

// static variables
static const char basis_64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// base64 encoding/decoding routines
int base64encode(char **out, const char *in, size_t inlen);
int base64decode(char **out, const char *in, size_t inlen);
long base64encode_file(FILE *in, FILE *out, BOOL convLF);
long base64decode_file(FILE *in, FILE *out,
                       struct codeset *srcCodeset, BOOL isText, BOOL convCRLF);

#endif // BASE64_H
