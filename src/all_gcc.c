/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2006 by YAM Open Source Team

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

// missing POSIX functions
#if !defined(__MORPHOS__) || !defined(__libnix)
  #include "extrasrc/stcgfe.c"
  #include "extrasrc/strmfp.c"
#endif

#if defined(NEED_STRLCPY)
  #include "extrasrc/strlcpy.c"
#endif /* NEED_STRLCPY */

#if defined(NEED_STRLCAT)
  #include "extrasrc/strlcat.c"
#endif /* NEED_STRLCAT */

#if defined(NEED_STRTOK_R)
  #include "extrasrc/strtok_r.c"
#endif /* NEED_STRTOK_R */

// required Amiga functions
#include "extrasrc/NewReadArgs.c"

// Amiga vargs function stubs
#include "extrasrc/vastubs.c"

void __chkabort(void) {}
