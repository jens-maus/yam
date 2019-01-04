/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2019 YAM Open Source Team

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
#include <ctype.h>

#include "BoyerMooreSearch.h"
#include "YAM_utilities.h"

#include "Debug.h"

/// BoyerMooreInit
// initialize the skip table for a Boyer-Moore string search
struct BoyerMooreContext *BoyerMooreInit(const char *pattern, const BOOL caseSensitive)
{
  struct BoyerMooreContext *bmc = NULL;

  ENTER();

  if(pattern != NULL && (bmc = malloc(sizeof(*bmc))) != NULL)
  {
    size_t plen = strlen(pattern);
    size_t i;

    bmc->pattern = strdup(pattern);
    bmc->patternLength = plen;
    bmc->caseSensitive = caseSensitive;

    // calculate the skip table
    for(i = 0; i < ARRAY_SIZE(bmc->skip); i++)
      bmc->skip[i] = plen;

    // convert the complete pattern to lower case if we are not
    // interested in a case sensitive search
    if(caseSensitive == FALSE)
      ToLowerCase(bmc->pattern);

    for(i = 0; i < plen; i++)
      bmc->skip[(unsigned char)bmc->pattern[i]] = plen - i - 1;
  }

  RETURN(bmc);
  return bmc;
}

///
/// BoyerMooreCleanup
// free the context of a Boyer-Moore search
void BoyerMooreCleanup(struct BoyerMooreContext *bmc)
{
  ENTER();

  if(bmc != NULL)
  {
    free(bmc->pattern);
    free(bmc);
  }

  LEAVE();
}

///
/// BoyerMooreSearch
// search a string in another string using the Boyer-Moore algorithm
// the context structure must be initialized first using BoyerMooreInit()
const char *BoyerMooreSearch(const struct BoyerMooreContext *bmc, const char *string)
{
  const char *result = NULL;

  ENTER();

  if(bmc != NULL && bmc->pattern != NULL)
  {
    int slen = strlen(string);

    if(bmc->patternLength <= slen)
    {
      const char *pattern = bmc->pattern;
      int patternLength = bmc->patternLength;
      const int *skip = bmc->skip;
      BOOL caseSensitive = bmc->caseSensitive;
      int i, j;

      i = patternLength-1;
      j = patternLength-1;

      // perform the string search
      while(j >= 0 && i < slen)
      {
        char c;

        if(caseSensitive == TRUE)
          c = string[i];
        else
          c = tolower(string[i]);

        if(c == pattern[j])
        {
          i--;
          j--;
        }
        else
        {
          i += MAX(patternLength-j, skip[(unsigned char)c]);
          j = patternLength-1;
        }
      }

      if(j == -1)
        result = &string[i+1];
    }
  }

  RETURN(result);
  return result;
}

///
