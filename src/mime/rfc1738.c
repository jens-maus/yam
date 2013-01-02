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

#include <stdio.h>
#include <ctype.h>

#include "mime/qprintable.h"

#include "Debug.h"

/*** RFC 1738 URL encoding/decoding routines ***/
/// urlencode()
// URL encoding function returning the length of the encoded string.
int urlencode(char *to, const char *from, unsigned int len)
{
  char *optr = to;

  ENTER();

  // only continue of we have reasonable input
  // values.
  if(from && *from && to)
  {
    unsigned char *iptr = (unsigned char *)from;
    unsigned char c = *iptr;

    while(c != '\0' && (optr-to) < (int)len)
    {
      if(isalnum(c))
        *optr++ = c;
      else
      {
        // the char is not an alphanumeric character
        // so we need to encode it like "%XX" where XX is
        // the hexadecimal interpretation of it
        *optr++ = '%';
        *optr++ = basis_hex[(c >> 4) & 0xF];
        *optr++ = basis_hex[c & 0xF];
      }

      c = *(++iptr);
    }

    // NUL terminate the output string
    *optr = '\0';
  }

  RETURN((int)(optr-to));
  return optr-to;
}

///
