/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2011 YAM Open Source Team

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

#include "extrasrc.h"

#if defined(NEED_STCGFE)

#ifndef FESIZE
#define FESIZE 32
#endif

/* Get the filename extension. */
int stcgfe(char *ext, const char *name)
{
   const char *p = name + strlen(name);
   const char *q = p;
   while (p > name && *--p != '.' && *p != '/' && *p != ':');
   if (*p++ == '.' && q - p < FESIZE)
   {
      memcpy(ext, p, q - p + 1);
      return q - p;
   }
   *ext = '\0';
   return 0;
}

#else
  #warning "NEED_STCGFE missing or compilation unnecessary"
#endif
