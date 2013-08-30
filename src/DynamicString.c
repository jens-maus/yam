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
#include <stdlib.h>
#include <string.h>

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

#if defined(DEBUG)
#define CHECK_DSTR(ds) \
  if(ds == NULL || ds->dbg_cookie[0] != 0xbe || ds->dbg_cookie[1] != 0xef) \
    E(DBF_UTIL, "invalid dstr (0x%08lx) used in %s()", dstr, __FUNCTION__)
#else
#define CHECK_DSTR(ds) ((void)0)
#endif

/// dstrallocInternal
// allocates a dynamic string
static struct DynamicString *dstrallocInternal(size_t size)
{
  struct DynamicString *dstr;
  ldiv_t chunks;

  ENTER();

  // add one byte for the terminating NUL byte
  size++;

  // calculate the number of chunks required
  chunks = ldiv(size, SIZE_DSTRCHUNK);
  if(chunks.rem != 0)
    chunks.quot++;

  // allocate the dstr structure and the number of chunks
  if((dstr = malloc(sizeof(*dstr) + chunks.quot * SIZE_DSTRCHUNK)) != NULL)
  {
    #if defined(DEBUG)
    dstr->dbg_cookie[0] = 0xbe;
    dstr->dbg_cookie[1] = 0xef; // 0xbeef
    #endif
    dstr->size = chunks.quot * SIZE_DSTRCHUNK;
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

  ENTER();

  if((dstr = dstrallocInternal(initsize)) != NULL)
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

    CHECK_DSTR(ds);

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
    if((ds = dstrallocInternal(reqsize)) != NULL)
      *dstr = DSTR_TO_STR(ds);
    else
      reqsize = 0;
  }
  else
  {
    ds = STR_TO_DSTR(*dstr);

    CHECK_DSTR(ds);

    // make sure the string buffer is large enough to keep the
    // requested amount of characters + NUL byte
    if(reqsize+1 > ds->size)
    {
      struct DynamicString *newdstr;

      // allocate a new buffer and replace the old one with it
      if((newdstr = dstrallocInternal(reqsize)) != NULL)
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
    if((ds = dstrallocInternal(reqsize)) != NULL)
      *dstr = DSTR_TO_STR(ds);
    else
      srcsize = 0;
  }
  else
  {
    ds = STR_TO_DSTR(*dstr);

    CHECK_DSTR(ds);

    // increase required size by the content length of
    // the old dstr
    reqsize += ds->strlen;

    // make sure the string buffer is large enough to keep the
    // requested amount of characters + NUL byte
    if(reqsize+1 > ds->size)
    {
      struct DynamicString *newdstr;

      // allocate a new buffer and replace the old one with it
      if((newdstr = dstrallocInternal(reqsize)) != NULL)
      {
        newdstr->strlen = ds->strlen;
        // copy the old dstr to the new one including the terminating NUL byte
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

    CHECK_DSTR(ds);

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
    if((ds = dstrallocInternal(size)) != NULL)
      *dstr = DSTR_TO_STR(ds);
    else
      size = 0;
  }
  else
  {
    ds = STR_TO_DSTR(*dstr);

    CHECK_DSTR(ds);

    // make sure the string buffer is large enough to keep the
    // requested amount of characters + NUL byte
    if(size+1 > ds->size)
    {
      struct DynamicString *newdstr;

      // allocate a new buffer and replace the old one with it
      if((newdstr = dstrallocInternal(size)) != NULL)
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
      W(DBF_UTIL, "fread() result:%d != size:%d", nread, size);
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

    CHECK_DSTR(ds);

    free(ds);
  }

  LEAVE();
}

///
