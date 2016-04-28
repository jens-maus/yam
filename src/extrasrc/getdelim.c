/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2016 YAM Open Source Team

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

 NOTE:
 This implementation was inspired/ported by the glibc version of getline()
 and getdelim().

 $Id$

***************************************************************************/

#include <limits.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

#include "extrasrc.h"

#include "Debug.h"

#if defined(NEED_GETDELIM)

#ifndef SIZE_MAX
#define SIZE_MAX ((size_t)-1)
#endif
#ifndef SSIZE_MAX
#define SSIZE_MAX ((size_t)(SIZE_MAX / 2))
#endif

ssize_t getdelim(char **lineptr, size_t *n, int delim, FILE *stream)
{
  ssize_t result = -1;

  ENTER();

  // fail immediately if one of the parameters is crap
  if(lineptr != NULL && n != NULL && stream != NULL)
  {
    if(*lineptr == NULL || *n == 0)
    {
      *n = 120;
      *lineptr = (char *)realloc(*lineptr, *n);
    }

    // make sure we really have a destination buffer
    if(*lineptr != NULL)
    {
      size_t cur_len = 0;
      char *cptr = *lineptr; // we use cptr as a shortcut to the current end of the line

      for(;;)
      {
        int i;

        i = getc(stream);
        if(i == EOF)
          break;

        // Make enough space for len+1 (for final NUL) bytes.
        if(cur_len+1 >= *n)
        {
          const size_t needed_max = SSIZE_MAX < SIZE_MAX ? (size_t)SSIZE_MAX + 1 : SIZE_MAX;
          size_t needed = 2 * (*n) + 1;   // Be generous.
          char *new_lineptr;

          D(DBF_UTIL, "getline(): realloc to %ld", needed);

          if(needed_max < needed)
            needed = needed_max;

          if(cur_len+1 >= needed)
            goto out;

          new_lineptr = (char *)realloc(*lineptr, needed);
          if(new_lineptr == NULL)
            goto out;

          *lineptr = new_lineptr;
          *n = needed;

          // also adjust our shortcut pointer
          cptr = &new_lineptr[cur_len];
        }

        // put the character in the buffer
        *cptr = i;
        cptr++;
        cur_len++;

        if(i == delim)
          break;
      }

      // add the trailing NUL character
      *cptr = '\0';
      result = cur_len > 0 ? (ssize_t)cur_len : result;
    }
  }

out:

  RETURN(result);
  return result;
}

#else
  #warning "NEED_GETDELIM missing or compilation unnecessary"
#endif
