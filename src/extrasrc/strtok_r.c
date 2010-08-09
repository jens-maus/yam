/*
 * $Id$
 *
 * :ts=4
 *
 * Portable ISO 'C' (1994) runtime library for the Amiga computer
 * Copyright (c) 2002-2006 by Olaf Barthel <olsen (at) sourcery.han.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   - Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *   - Neither the name of Olaf Barthel nor the names of contributors
 *     may be used to endorse or promote products derived from this
 *     software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <string.h>

char *strtok_r(char *str, const char *separator_set,char ** state_ptr)
{
  char *result = NULL;
  char *last;
  size_t size;

  last = (*state_ptr);

  /* Did we get called before? Restart at the last valid position. */
  if(str == NULL)
  {
    str = last;

    /* However, we may have hit the end of the
       string already. */
    if(str == NULL)
      goto out;
  }

  last = NULL;

  /* Skip the characters which count as
     separators. */
  str += strspn(str, separator_set);
  if((*str) == '\0')
    goto out;

  /* Count the number of characters which aren't
     separators. */
  size = strcspn(str, separator_set);
  if(size == 0)
    goto out;

  /* This is where the search can resume later. */
  last = &str[size];

  /* If we didn't hit the end of the string already,
     skip the separator. */
  if((*last) != '\0')
    last++;

  /* This is the token we found; make sure that
     it looks like a valid string. */
  str[size] = '\0';

  result = str;

out:

  if(state_ptr != NULL)
    (*state_ptr) = last;

  return(result);
}
