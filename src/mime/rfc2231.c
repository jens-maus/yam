/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2022 YAM Open Source Team

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
#include <ctype.h>
#include <stdio.h>
#include <strings.h>

#include <proto/codesets.h>

#include "YAM.h"
#include "YAM_stringsizes.h"

#include "mime/qprintable.h"

#include "Config.h"

#include "Debug.h"

// local prototypes
static int rfc2231_decode_int(char *dst, const char *src, struct codeset *srcCodeset);

/*** RFC 2231 MIME parameter encoding/decoding routines ***/
/// rfc2231_encode_file()
// Special encoding routines based on RFC2231 which defines how
// content parameters in a header field should be encoded.
// This function takes the actual file pointer, the string that
// should be encoded and the actual parameter name for which we
// encode 'str'.
int rfc2231_encode_file(FILE *fh, const char *paramName, const char *str)
{
  int num = 0;
  int outchars = 0;
  char buf[SIZE_DEFAULT];
  char *p = (char *)str;
  char *optr = &buf[0];

  ENTER();

  // we loop until we have processed the whole
  // input string
  while(*p)
  {
    unsigned char c = (unsigned char)*p;

    if(optr-buf == 0)
    {
      // if this is the very first parameter
      // part, we go and add the charset as well but no language
      // definition as we don't support that yet.
      if(num == 0)
        snprintf(buf, SIZE_DEFAULT, "\n\t%s*%d*=%s''", paramName, num++, strippedCharsetName(G->writeCodeset));
      else
        snprintf(buf, SIZE_DEFAULT, "\n\t%s*%d*=", paramName, num++);

      optr = buf+strlen(buf);
    }
    else if(optr-buf > 78-3) // max 78 chars line length
    {
      if(c)
        *optr++ = ';';

      // output our buffer to the file pointer
      if(fwrite(buf, optr-buf, 1, fh) == 0)
        break;

      outchars += optr-buf;
      optr = &buf[0];

      continue;
    }

    // encode the current char if it isn't an alphanumeric
    // one
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

    // iterate forward
    p++;
  }

  // if there is still something in out output buffer
  // we output it now
  if(optr-buf > 0)
  {
    fwrite(buf, optr-buf, 1, fh);
    outchars += optr-buf;
  }

  RETURN(outchars);
  return outchars;
}

///
/// rfc2231_decode()
// the main RFC 2231 decoding routine
int rfc2231_decode(char *attr, char *value, char **result, struct codeset **srcCodeset)
{
  int ret = 0;

  ENTER();

  if(attr[0] == '0' || attr[0] == '\0')
  {
    char *p;

    // we found the 'name*0=' parameter, so we have to
    // see if we can retrieve the charset and language
    // spec from our value
    if((p = strchr(value, '\'')))
    {
      *p = '\0';

      if(p-value > 0)
      {
        // search for the codeset via codesets.library
        *srcCodeset = CodesetsFind(value,
                                   CSA_CodesetList,       G->codesetsList,
                                   CSA_FallbackToDefault, FALSE,
                                   TAG_DONE);
      }

      // and we skip the language part as we don't
      // require it yet.
      if((p = strchr(++p, '\'')))
      {
        // nowe we decode the encoded string
        ret = rfc2231_decode_int(value, p+1, *srcCodeset);
      }
      else
        W(DBF_MIME, "malformed rfc2231 content parameter found! '%s'", value);
    }
    else
      W(DBF_MIME, "malformed rfc2231 content parameter found! '%s'", value);

    *result = value;
  }
  else if(isdigit(attr[0]))
  {
    // we found another parameter with number > 0
    // so we have to append what we decode next
    ret = rfc2231_decode_int(value, value, *srcCodeset);

    // now we concatenate
    if(*result)
    {
      int newSize = strlen(*result)+strlen(value)+1;

      if((*result = realloc(*result, newSize)))
      {
        strlcat(*result, value, newSize);

        free(value);
      }
    }
  }
  else
    *result = value;

  RETURN(ret);
  return ret;
}

///
/// rfc2231_decode_int()
// function which does the actual RFC2231 based string decoding
// (not including parameter concatenating)
static int rfc2231_decode_int(char *dst, const char *src, struct codeset *srcCodeset)
{
  char *p = (char *)src;
  char *q = dst;

  ENTER();

  while(*p)
  {
    if(*p == '%' && p[1] != '\0' && p[2] != '\0')
    {
      unsigned char c1 = hexchar(p[1]);
      unsigned char c2 = hexchar(p[2]);

      // so we have enough space, lets decode it, but let us
      // check if the two chars are really hexadecimal chars
      if(c1 != 255 && c2 != 255)
      {
        *dst++ = c1<<4 | c2;
        p += 3;
      }
      else
        *dst++ = *p++;
    }
    else
      *dst++ = *p++;
  }

  *dst = '\0';

  if(srcCodeset && srcCodeset != G->localCodeset && p-src > 0)
  {
    STRPTR str = CodesetsConvertStr(CSA_SourceCodeset,   srcCodeset,
                                    CSA_DestCodeset,     G->localCodeset,
                                    CSA_Source,          q,
                                    CSA_MapForeignChars, C->MapForeignChars,
                                    TAG_DONE);

    if(str)
    {
      strcpy(q, str);
      CodesetsFreeA(str, NULL);
    }
    else
      W(DBF_MIME, "error while trying to convert rfc2231 decoded string to local charset, codesets '%s' -> '%s'!", SafeStr(srcCodeset->name), SafeStr(G->localCodeset->name));
  }

  RETURN((int)(p-src));
  return p-src;
}

///
