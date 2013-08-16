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

#include "DynamicString.h"

#include "Debug.h"

struct DynamicString
{
  #if defined(DEBUG)
  unsigned char dbg_cookie[2];  // for debugging we put here a magic cookie
  #endif
  size_t size;    // the allocated memory size for the string
  size_t strlen;  // the current string length
  char str[0];    // the string itself with length 'strlen'
};

// some macros to make the conversion between (char *) and (struct DynamicString *) easier
#define STR_TO_DSTR(str)  (struct DynamicString *)(((size_t)(str)) - sizeof(struct DynamicString))
#define DSTR_TO_STR(dstr) (&(dstr)->str[0])

/// dstrallocInternal
// allocates a dynamic string
static struct DynamicString *dstrallocInternal(const size_t size)
{
  struct DynamicString *dstr;

  ENTER();

  if((dstr = malloc(sizeof(*dstr) + size)) != NULL)
  {
    #if defined(DEBUG)
    dstr->dbg_cookie[0] = 0xBE;
    dstr->dbg_cookie[1] = 0xEF; // 0xBEEF
    #endif
    dstr->size = size;
    dstr->strlen = 0;
    dstr->str[0] = '\0';
  }

  RETURN(dstr);
  return dstr;
}

///
/// dstralloc
// allocates a dynamic string with a given initial size
char *dstralloc(size_t initsize)
{
  char *result = NULL;
  struct DynamicString *dstr;
  size_t size = 0;

  ENTER();

  // make sure we allocate in SIZE_DEFAULT chunks
  while(size <= initsize)
    size += SIZE_DEFAULT;

  if((dstr = dstrallocInternal(size)) != NULL)
    result = DSTR_TO_STR(dstr);

  RETURN(result);
  return result;
}

///
/// dstrreset
// reset a dynamic string. That means it will simply put
// the length to 0 and NUL terminate it, but not free the space
void dstrreset(const char *dstr)
{
  ENTER();

  if(dstr != NULL)
  {
    struct DynamicString *ds = STR_TO_DSTR(dstr);

    #if defined(DEBUG)
    if(ds == NULL || ds->dbg_cookie[0] != 0xBE || ds->dbg_cookie[1] != 0xEF)
      E(DBF_UTIL, "invalid dstr (0x%08x) used in dstrreset()", dstr);
    #endif

    ds->strlen = 0;
    ds->str[0] = '\0';
  }

  LEAVE();
}

///
/// dstrcpy
// fills a dynamic buffer and returns the length of the string
char *dstrcpy(char **dstr, const char *src)
{
  size_t reqsize;
  struct DynamicString *ds = NULL;
  char *result = NULL;

  ENTER();

  if(src != NULL)
    reqsize = strlen(src);
  else
    reqsize = 0;

  // if dstr itself is NULL we replace dstr with a new local
  // version
  if(dstr == NULL)
    dstr = &result;

  // if the content of dstr is NULL we have to allocate a new buffer
  if(*dstr == NULL)
  {
    if((ds = dstrallocInternal(reqsize+1)) != NULL)
      *dstr = DSTR_TO_STR(ds);
    else
      reqsize = 0;
  }
  else
  {
    size_t oldsize;
    size_t newsize;

    ds = STR_TO_DSTR(*dstr);

    #if defined(DEBUG)
    if(ds == NULL || ds->dbg_cookie[0] != 0xBE || ds->dbg_cookie[1] != 0xEF)
      E(DBF_UTIL, "invalid dstr (0x%08x) used in dstrcpy()", dstr);
    #endif

    oldsize = ds->size;
    newsize = oldsize;

    // make sure we allocate in SIZE_DEFAULT chunks
    while(newsize <= reqsize)
      newsize += SIZE_DEFAULT;

    // if we have to change the size do it now
    if(newsize > oldsize)
    {
      struct DynamicString *newdstr;

      // allocate a new buffer and replace the old one with it
      if((newdstr = dstrallocInternal(newsize+1)) != NULL)
      {
        free(ds);
        ds = newdstr;
        *dstr = DSTR_TO_STR(ds);
      }
      else
        reqsize = 0;
    }
  }

  // do a string copy into the new buffer
  if(reqsize > 0)
  {
    ds->strlen = strlcpy(*dstr, src, ds->size);
    result = *dstr;
  }

  RETURN(result);
  return result;
}

///
/// dstrcat
// string concatenation using a dynamic buffer and return the length of the string
char *dstrcat(char **dstr, const char *src)
{
  size_t srcsize;
  size_t reqsize;
  struct DynamicString *ds = NULL;
  char *result = NULL;

  ENTER();

  if(src != NULL)
    srcsize = strlen(src);
  else
    srcsize = 0;

  reqsize = srcsize;

  // if dstr itself is NULL we replace dstr with a new local
  // version
  if(dstr == NULL)
    dstr = &result;

  // if our dstr is NULL we have to allocate a new buffer
  if(*dstr == NULL)
  {
    if((ds = dstrallocInternal(reqsize+1)) != NULL)
      *dstr = DSTR_TO_STR(ds);
    else
      srcsize = 0;
  }
  else
  {
    size_t oldsize;
    size_t newsize;

    ds = STR_TO_DSTR(*dstr);

    #if defined(DEBUG)
    if(ds == NULL || ds->dbg_cookie[0] != 0xBE || ds->dbg_cookie[1] != 0xEF)
      E(DBF_UTIL, "invalid dstr (0x%08x) used in dstrcat()", dstr);
    #endif

    oldsize = ds->size;
    newsize = oldsize;

    // increase required size by the content length of
    // the old dstr
    reqsize += ds->strlen;

    // make sure we allocate in SIZE_DEFAULT chunks
    while(newsize <= reqsize)
      newsize += SIZE_DEFAULT;

    // if we have to change the size do it now
    if(newsize > oldsize)
    {
      struct DynamicString *newdstr;

      // allocate a new buffer and replace the old one with it
      if((newdstr = dstrallocInternal(newsize+1)) != NULL)
      {
        newdstr->strlen = ds->strlen;
        memmove(newdstr->str, ds->str, ds->strlen+1);
        free(ds);
        ds = newdstr;
        *dstr = DSTR_TO_STR(ds);
      }
      else
        reqsize = 0;
    }
  }

  // do a string concatenation into the new buffer
  if(srcsize > 0)
  {
    // use strlcpy() instead of strlcat() because we keep track of the current string length
    ds->strlen += strlcpy(&ds->str[ds->strlen], src, ds->size - ds->strlen);

    result = *dstr;
  }

  RETURN(result);
  return result;
}

///
/// dstrlen
// return the current length of a dynamic string buffer without calculation
size_t dstrlen(const char *dstr)
{
  size_t result = 0;

  ENTER();

  if(dstr != NULL)
  {
    struct DynamicString *ds = STR_TO_DSTR(dstr);

    #if defined(DEBUG)
    if(ds == NULL || ds->dbg_cookie[0] != 0xBE || ds->dbg_cookie[1] != 0xEF)
      E(DBF_UTIL, "invalid dstr (0x%08x) used in dstrlen()", dstr);
    #endif

    result = ds->strlen;
  }

  RETURN(result);
  return result;
}

///
/// dstrfread
// read the given amount of bytes from a file and place them in the dynamic string
size_t dstrfread(char **dstr, size_t size, FILE *stream)
{
  size_t nread = 0;
  struct DynamicString *ds = NULL;

  ENTER();

  if(*dstr == NULL)
  {
    if((ds = dstrallocInternal(size+1)) != NULL)
      *dstr = DSTR_TO_STR(ds);
    else
      size = 0;
  }
  else
  {
    ds = STR_TO_DSTR(*dstr);

    #if defined(DEBUG)
    if(ds == NULL || ds->dbg_cookie[0] != 0xBE || ds->dbg_cookie[1] != 0xEF)
      E(DBF_UTIL, "invalid dstr (0x%08x) used in dstrfread()", dstr);
    #endif

    // make sure the string buffer is large enough to keep the
    // requested amount of characters
    if(ds->size < size+1)
    {
      struct DynamicString *newdstr;

      // allocate a new buffer and replace the old one with it
      if((newdstr = dstrallocInternal(size+1)) != NULL)
      {
        free(ds);
        ds = newdstr;
        *dstr = DSTR_TO_STR(ds);
      }
      else
        size = 0;
    }
  }

  if(size != 0)
  {
    // finally read the characters from the file and NUL terminate the string
    nread = fread(ds->str, 1, size, stream);
    ds->strlen = nread;
    ds->str[nread] = '\0';

    #if defined(DEBUG)
    if(nread != size)
      E(DBF_UTIL, "fread() result:%d != size:%d", nread, size);
    #endif
  }

  RETURN(nread);
  return nread;
}

///
/// dstrfree
// free a dynamic string buffer
void dstrfree(const char *dstr)
{
  ENTER();

  if(dstr != NULL)
  {
    struct DynamicString *ds = STR_TO_DSTR(dstr);

    #if defined(DEBUG)
    if(ds == NULL || ds->dbg_cookie[0] != 0xBE || ds->dbg_cookie[1] != 0xEF)
      E(DBF_UTIL, "invalid dstr (0x%08x) used in dstrfree()", dstr);
    #endif

    free(ds);
  }

  LEAVE();
}

///
