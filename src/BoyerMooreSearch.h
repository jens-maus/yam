#ifndef BOYERMOORESEARCH_H
#define BOYERMOORESEARCH_H 1

/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2017 YAM Open Source Team

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

#include <exec/types.h>

/*
 An implementation of the Boyer/Moore fast string search algorithm.
 Before performing the actual search a context structure must be
 created using BoyerMooreInit() which will set up the required skip
 table. This context structure may be used for several subsequent
 searches of the same string. Finally this context must be freed
 using BoyerMooreCleanup().

 Details about the Boyer/Moore string search algorithm can be found here:
   http://www.itl.nist.gov/div897/sqg/dads/HTML/boyermoore.html
   http://en.wikipedia.org/wiki/Boyer%E2%80%93Moore_string_search_algorithm
*/

struct BoyerMooreContext
{
  char *pattern;
  int patternLength;
  BOOL caseSensitive;
  int skip[256];
};

struct BoyerMooreContext *BoyerMooreInit(const char *pattern, const BOOL caseSensitive);
void BoyerMooreCleanup(struct BoyerMooreContext *bmc);
const char *BoyerMooreSearch(const struct BoyerMooreContext *bmc, const char *string);

#endif /* BOYERMOORESEARCH_H */
