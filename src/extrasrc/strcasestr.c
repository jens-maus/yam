/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2022 YAM Open Source Team

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
#include <ctype.h>

#include "extrasrc.h"

#if defined(NEED_STRCASESTR)

/// strcasestr()
//
//  Case insensitive version of strstr()
//
//  NOTE: do *NOT* use the ENTER() and RETURN() macros, because this
//        function is used during SetupDebug(). Calling ENTER() in
//        that situation will cause infinite recursions due to the
//        module check of ENTER().
char *strcasestr(const char *haystack, const char *needle)
{
  const char *p;
  const char *startn = NULL;
  const char *np = NULL;

  for(p = haystack; *p; p++)
  {
    if(np)
    {
      if(toupper(*p) == toupper(*np))
      {
        if(!*++np)
          return (char *)startn;
      }
      else
        np = 0;
    }
    else if(toupper(*p) == toupper(*needle))
    {
      np = needle + 1;
      startn = p;
    }
  }

  return NULL;
}

#else
  #warning "NEED_STRCASESTR missing or compilation unnecessary"
#endif
