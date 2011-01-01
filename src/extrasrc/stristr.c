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

#if defined(NEED_STRISTR)

/// stristr()
//
//  Case insensitive version of strstr()
//
//  NOTE: do *NOT* use the ENTER() and RETURN() macros, because this
//        function is used during SetupDebug(). Calling ENTER() in
//        that situation will cause infinite recursions due to the
//        module check of ENTER().
char *stristr(const char *a, const char *b)
{
  char *s = NULL;

  if(a != NULL && b != NULL)
  {
    int l = strlen(b);

    for(; *a; a++)
    {
      if(strnicmp(a, b, l) == 0)
      {
        s = (char *)a;
        break;
      }
    }
  }

  return s;
}

#else
  #warning "NEED_STRISTR missing or compilation unnecessary"
#endif
