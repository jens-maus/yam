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

#include <string.h>

#include "YAM_stringsizes.h"

#include "StrBuf.h"
#include "Debug.h"

struct StrBuf
{
  size_t size;    // the allocated memory size for the string
  size_t length;  // the current string length
  char string[0]; // the string itsefl
};

/// AllocStrBufInternal
// allocates a dynamic buffer
static struct StrBuf *AllocStrBufInternal(size_t size)
{
  struct StrBuf *strbuf;

  ENTER();

  if((strbuf = malloc(sizeof(struct StrBuf) + size)) != NULL)
  {
    strbuf->size = size;
    strbuf->length = 0;
    strbuf->string[0] = '\0';
  }

  RETURN(strbuf);
  return strbuf;
}

///
/// AllocStrBuf
//  Allocates a dynamic buffer
char *AllocStrBuf(size_t initsize)
{
  char *result = NULL;
  struct StrBuf *strbuf;

  ENTER();

  if((strbuf = AllocStrBufInternal(initsize)) != NULL)
  {
    result = &strbuf->string[0];
  }

  RETURN(result);
  return result;
}

///
/// StrBufCpy
//  Fills a dynamic buffer and returns the length of the string
size_t StrBufCpy(char **buf, const char *source)
{
  size_t reqsize;
  struct StrBuf *strbuf;

  ENTER();

  if(source != NULL)
    reqsize = strlen(source);
  else
    reqsize = 0;

  // if our strbuf is NULL we have to allocate a new buffer
  if(*buf == NULL)
  {
    if((strbuf = AllocStrBufInternal(reqsize+1)) != NULL)
      *buf = &strbuf->string[0];
    else
      reqsize = 0;
  }
  else
  {
    size_t oldsize;
    size_t newsize;

    strbuf = (struct StrBuf *)(*buf - sizeof(struct StrBuf));
    oldsize = strbuf->size;
    newsize = oldsize;

    // make sure we allocate in SIZE_DEFAULT chunks
    while(newsize <= reqsize)
      newsize += SIZE_DEFAULT;

    // if we have to change the size do it now
    if(newsize > oldsize)
    {
      struct StrBuf *newstrbuf;

      // allocate a new buffer and replace the old one with it
      if((newstrbuf = AllocStrBufInternal(newsize+1)) != NULL)
      {
        free(strbuf);
        strbuf = newstrbuf;
        *buf = &strbuf->string[0];
      }
      else
      {
      	reqsize = 0;
      }
    }
  }

  // do a string copy into the new buffer
  if(reqsize > 0)
  {
    reqsize = strlcpy(strbuf->string, source, strbuf->size);
    strbuf->length = reqsize;
  }

  RETURN(reqsize);
  return reqsize;
}

///
/// StrBufCat
//  String concatenation using a dynamic buffer and return the length of the string
size_t StrBufCat(char **buf, const char *source)
{
  size_t reqsize;
  struct StrBuf *strbuf;

  ENTER();

  if(source != NULL)
    reqsize = strlen(source);
  else
    reqsize = 0;

  // if our strbuf is NULL we have to allocate a new buffer
  if(*buf == NULL)
  {
    if((strbuf = AllocStrBufInternal(reqsize+1)) != NULL)
      *buf = &strbuf->string[0];
    else
      reqsize = 0;
  }
  else
  {
    size_t oldsize;
    size_t newsize;

    strbuf = (struct StrBuf *)(*buf - sizeof(struct StrBuf));
    oldsize = strbuf->size;
    newsize = oldsize;

    // increase required size by the content length of
    // the old strbuf
    reqsize += strbuf->length;

    // make sure we allocate in SIZE_DEFAULT chunks
    while(newsize <= reqsize)
      newsize += SIZE_DEFAULT;

    // if we have to change the size do it now
    if(newsize > oldsize)
    {
      struct StrBuf *newstrbuf;

      // allocate a new buffer and replace the old one with it
      if((newstrbuf = AllocStrBufInternal(newsize+1)) != NULL)
      {
        newstrbuf->length = strbuf->length;
        memmove(newstrbuf->string, strbuf->string, strbuf->length+1);
        free(strbuf);
        strbuf = newstrbuf;
        *buf = &strbuf->string[0];
      }
      else
      {
      	reqsize = 0;
      }
    }
  }

  // do a string concatenation into the new buffer
  if(reqsize > 0)
  {
    // use strlcpy() instead of strlcat() because we keep track of the current string length
    reqsize = strlcpy(&strbuf->string[strbuf->length], source, strbuf->size - strbuf->length);
    strbuf->length += reqsize;
  }

  RETURN(reqsize);
  return reqsize;
}

///
/// StrBufLength
// return the current length of a dynamic string buffer without calculation
size_t StrBufLength(char *buf)
{
  size_t length = 0;

  ENTER();

  if(buf != NULL)
  {
    struct StrBuf *strbuf = (struct StrBuf *)(*buf - sizeof(struct StrBuf));

    length = strbuf->length;
  }

  RETURN(length);
  return length;
}

///
/// FreeStrBuf
// free a dynamic string buffer
void FreeStrBuf(char *buf)
{
  ENTER();

  if(buf != NULL)
  {
	struct StrBuf *strbuf = (struct StrBuf *)(buf - sizeof(struct StrBuf));

	free(strbuf);
  }

  LEAVE();
}

///
