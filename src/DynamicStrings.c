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

#include <string.h>
#include <stdio.h>

#include "YAM_stringsizes.h"

#include "DynamicStrings.h"

#include "Debug.h"

struct DynamicString
{
  size_t size;    // the allocated memory size for the string
  size_t length;  // the current string length
  char string[0]; // the string itsefl
};

// some macros to make the conversion between (char *) and (struct DynamicString *) easier
#define STR_TO_DSTR(str)    (struct DynamicString *)(((size_t)(str)) - sizeof(struct DynamicString))
#define DSTR_TO_STR(dstr)   (&(dstr)->string[0])

/// dallocInternal
// allocates a dynamic buffer
static struct DynamicString *dallocInternal(size_t size)
{
  struct DynamicString *dstr;

  ENTER();

  if((dstr = malloc(sizeof(*dstr) + size)) != NULL)
  {
    dstr->size = size;
    dstr->length = 0;
    dstr->string[0] = '\0';
  }

  RETURN(dstr);
  return dstr;
}

///
/// dalloc
// allocates a dynamic buffer with a given initial size
char *dalloc(size_t initsize)
{
  char *result = NULL;
  struct DynamicString *dstr;
  size_t size;

  ENTER();

  // make sure we allocate in SIZE_DEFAULT chunks
  size = 0;
  while(size <= initsize)
    size += SIZE_DEFAULT;

  if((dstr = dallocInternal(size)) != NULL)
  {
    result = DSTR_TO_STR(dstr);
  }

  RETURN(result);
  return result;
}

///
/// dreset
// reset a dynamic string
void dreset(char *str)
{
  ENTER();

  if(str != NULL)
  {
    struct DynamicString *dstr = STR_TO_DSTR(str);

    dstr->length = 0;
    dstr->string[0] = '\0';
  }

  LEAVE();
}

///
/// dstrcpy
// fills a dynamic buffer and returns the length of the string
size_t dstrcpy(char **str, const char *source)
{
  size_t reqsize;
  struct DynamicString *dstr;

  ENTER();

  if(source != NULL)
    reqsize = strlen(source);
  else
    reqsize = 0;

  // if our dstr is NULL we have to allocate a new buffer
  if(*str == NULL)
  {
    if((dstr = dallocInternal(reqsize+1)) != NULL)
      *str = DSTR_TO_STR(dstr);
    else
      reqsize = 0;
  }
  else
  {
    size_t oldsize;
    size_t newsize;

    dstr = STR_TO_DSTR(*str);
    oldsize = dstr->size;
    newsize = oldsize;

    // make sure we allocate in SIZE_DEFAULT chunks
    while(newsize <= reqsize)
      newsize += SIZE_DEFAULT;

    // if we have to change the size do it now
    if(newsize > oldsize)
    {
      struct DynamicString *newdstr;

      // allocate a new buffer and replace the old one with it
      if((newdstr = dallocInternal(newsize+1)) != NULL)
      {
        free(dstr);
        dstr = newdstr;
        *str = DSTR_TO_STR(dstr);
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
    dstr->length = strlcpy(dstr->string, source, dstr->size);
  }

  RETURN(reqsize);
  return reqsize;
}

///
/// dstrcat
// string concatenation using a dynamic buffer and return the length of the string
size_t dstrcat(char **str, const char *source)
{
  size_t srcsize;
  size_t reqsize;
  struct DynamicString *dstr;

  ENTER();

  if(source != NULL)
    srcsize = strlen(source);
  else
    srcsize = 0;

  reqsize = srcsize;

  // if our dstr is NULL we have to allocate a new buffer
  if(*str == NULL)
  {
    if((dstr = dallocInternal(reqsize+1)) != NULL)
      *str = DSTR_TO_STR(dstr);
    else
      srcsize = 0;
  }
  else
  {
    size_t oldsize;
    size_t newsize;

    dstr = STR_TO_DSTR(*str);
    oldsize = dstr->size;
    newsize = oldsize;

    // increase required size by the content length of
    // the old dstr
    reqsize += dstr->length;

    // make sure we allocate in SIZE_DEFAULT chunks
    while(newsize <= reqsize)
      newsize += SIZE_DEFAULT;

    // if we have to change the size do it now
    if(newsize > oldsize)
    {
      struct DynamicString *newdstr;

      // allocate a new buffer and replace the old one with it
      if((newdstr = dallocInternal(newsize+1)) != NULL)
      {
        newdstr->length = dstr->length;
        memmove(newdstr->string, dstr->string, dstr->length+1);
        free(dstr);
        dstr = newdstr;
        *str = DSTR_TO_STR(dstr);
      }
      else
      {
        reqsize = 0;
      }
    }
  }

  // do a string concatenation into the new buffer
  if(srcsize > 0)
  {
    // use strlcpy() instead of strlcat() because we keep track of the current string length
    dstr->length += strlcpy(&dstr->string[dstr->length], source, dstr->size - dstr->length);
  }

  RETURN(reqsize);
  return reqsize;
}

///
/// dstrlen
// return the current length of a dynamic string buffer without calculation
size_t dstrlen(char *str)
{
  size_t length = 0;

  ENTER();

  if(str != NULL)
  {
    struct DynamicString *dstr = STR_TO_DSTR(str);

    length = dstr->length;
  }

  RETURN(length);
  return length;
}

///
/// dread
// read the given amount of bytes from a file and place them in the dynamic string
size_t dread(char **str, FILE *fh, size_t size)
{
  size_t nread = 0;
  struct DynamicString *dstr;

  ENTER();

  if(*str == NULL)
  {
    if((dstr = dallocInternal(size+1)) != NULL)
      *str = DSTR_TO_STR(dstr);
    else
      size = 0;
  }
  else
  {
    dstr = STR_TO_DSTR(*str);

    // make sure the string buffer is large enough to keep the
    // requested amount of characters
    if(dstr->size < size+1)
    {
      struct DynamicString *newdstr;

      // allocate a new buffer and replace the old one with it
      if((newdstr = dallocInternal(size+1)) != NULL)
      {
        free(dstr);
        dstr = newdstr;
        *str = DSTR_TO_STR(dstr);
      }
      else
      {
        size = 0;
      }
    }
  }

  if(size != 0)
  {
    // finally read the characters from the file and NUL terminate the string
    nread = fread(dstr->string, 1, size, fh);
    dstr->length = size;
    dstr->string[size] = '\0';
  }

  RETURN(nread);
  return nread;
}

///
/// dfree
// free a dynamic string buffer
void dfree(char *str)
{
  ENTER();

  if(str != NULL)
  {
    struct DynamicString *dstr = STR_TO_DSTR(str);

    free(dstr);
  }

  LEAVE();
}

///
