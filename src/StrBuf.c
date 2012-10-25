/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2012 YAM Open Source Team

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
#include <string.h>

#include "YAM_stringsizes.h"

#include "StrBuf.h"
#include "Debug.h"

/// AllocStrBuf
//  Allocates a dynamic buffer
char *AllocStrBuf(size_t initlen)
{
  size_t *strbuf; // size_t because we store the length in the first area

  ENTER();

  if((strbuf = calloc(initlen+sizeof(size_t), sizeof(char))) != NULL)
  {
    *strbuf = initlen;
    strbuf++;
  }

  RETURN(strbuf);
  return (char *)strbuf;
}

///
/// StrBufCpy
//  Fills a dynamic buffer and returns the length of the string
size_t StrBufCpy(char **strbuf, const char *source)
{
  size_t reqlen = (source != NULL) ? strlen(source) : 0;

  ENTER();

  // if our strbuf is NULL we have to allocate a new buffer
  if(*strbuf == NULL)
    *strbuf = AllocStrBuf(reqlen+1);
  else
  {
    size_t oldlen = ((size_t *)*strbuf)[-1];
    size_t newlen;

    // make sure we allocate in SIZE_DEFAULT chunks
    for(newlen = oldlen; newlen <= reqlen; newlen += SIZE_DEFAULT)
      ;

    // if we have to change the size do it now
    if(newlen > oldlen)
    {
      char *newstrbuf;

      // allocate a new buffer and replace the old one with it
      if((newstrbuf = AllocStrBuf(newlen+1)) != NULL)
      {
        FreeStrBuf(*strbuf);
        *strbuf = newstrbuf;
      }
      else
      {
      	reqlen = 0;
      }
    }
  }

  // do a string copy into the new buffer
  if(reqlen > 0)
    reqlen = strlcpy(*strbuf, source, ((size_t *)*strbuf)[-1]);

  RETURN(reqlen);
  return reqlen;
}

///
/// StrBufCat
//  String concatenation using a dynamic buffer and return the length of the string
size_t StrBufCat(char **strbuf, const char *source)
{
  size_t reqlen = (source != NULL) ? strlen(source) : 0;

  ENTER();

  // if our strbuf is NULL we have to allocate a new buffer
  if(*strbuf == NULL)
    *strbuf = AllocStrBuf(reqlen+1);
  else
  {
    size_t oldlen = ((size_t *)*strbuf)[-1];
    size_t newlen;

    // increase reqlen by the content length of
    // the old strbuf
    reqlen += strlen(*strbuf);

    // make sure we allocate in SIZE_DEFAULT chunks
    for(newlen = oldlen; newlen <= reqlen; newlen += SIZE_DEFAULT)
      ;

    // if we have to change the size do it now
    if(newlen > oldlen)
    {
      char *newstrbuf;

      // allocate a new buffer, copy the old contents to it and replace the old one with it
      if((newstrbuf = AllocStrBuf(newlen+1)) != NULL)
      {
        memmove(newstrbuf, *strbuf, oldlen);
        FreeStrBuf(*strbuf);
        *strbuf = newstrbuf;
      }
      else
      {
      	reqlen = 0;
      }
    }
  }

  // do a string concatenation into the new buffer
  if(reqlen > 0)
    reqlen = strlcat(*strbuf, source, ((size_t *)*strbuf)[-1]);

  RETURN(reqlen);
  return reqlen;
}

///
/// FreeStrBuf
// free a dynamic string buffer
void FreeStrBuf(char *str)
{
  ENTER();

  if(str != NULL)
  {
	free(((char *)(str))-sizeof(size_t));
  }

  LEAVE();
}

///
