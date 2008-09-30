/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2008 by YAM Open Source Team

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

/*
   YAM tries to be compileable on most compiler/clib/operating system
   combinations for Amiga computers. Therefore, we use that file to
   include some common function which might either be missing on one
   C runtime library environment or requested by the compiling user.

   Please note that NEED_XXXXX defines are used to include missing
   function. Thse NEED_XXXXX macros are automatically defined in
   extrasrc.h and correctly setup per default. However, if a function
   might be missing on your runtime library, go and add such a define
   to your make call.
*/

#include "extrasrc.h"

// POSIX a-like functions
#if defined(NEED_STCGFE)
  #include "extrasrc/stcgfe.c"
#endif

#if defined(NEED_STRLCPY)
  #include "extrasrc/strlcpy.c"
#endif

#if defined(NEED_STRLCAT)
  #include "extrasrc/strlcat.c"
#endif

#if defined(NEED_STRTOK_R)
  #include "extrasrc/strtok_r.c"
#endif

#if defined(NEED_VSNPRINTF)
  #include "extrasrc/vsnprintf.c"
#endif

#if defined(NEED_SNPRINTF)
  #include "extrasrc/snprintf.c"
#endif

#if defined(NEED_VASPRINTF)
  #include "extrasrc/vasprintf.c"
#endif

#if defined(NEED_ASPRINTF)
  #include "extrasrc/asprintf.c"
#endif

#if defined(NEED_MEMDUP)
  #include "extrasrc/memdup.c"
#endif

#if defined(NEED_GETDELIM)
  #include "extrasrc/getdelim.c"
#endif

// Amiga specific functions
#if defined(DEBUG)
#undef AllocPooled
#undef FreePooled
#undef AllocVecPooled
#undef FreeVecPooled
#undef AllocDosObject
#undef FreeDosObject
#undef ObtainDirContext
#undef ReleaseDirContext
#endif

#if defined(NEED_NEWREADARGS)
  #include "extrasrc/NewReadArgs.c"
#endif

#if defined(NEED_VASTUBS)
  #include "extrasrc/vastubs.c"
#endif

#if defined(NEED_XGET)
  #include "extrasrc/xget.c"
#endif

#if defined(NEED_XSET)
  #include "extrasrc/xset.c"
#endif

#if defined(NEED_DOSUPERNEW)
  #include "extrasrc/DoSuperNew.c"
#endif

#if defined(NEED_SETPROCWINDOW)
  #include "extrasrc/SetProcWindow.c"
#endif

#if defined(NEED_EXAMINEDIR)
  #include "extrasrc/ExamineDir.h"
  #include "extrasrc/ExamineDir.c"
#endif

#if defined(NEED_ALLOCSYSOBJECT)
  #include "extrasrc/AllocSysObject.c"
#endif

#if defined(NEED_ALLOCVECPOOLED)
  #include "extrasrc/AllocVecPooled.c"
#endif

#if defined(NEED_FREEVECPOOLED)
  #include "extrasrc/FreeVecPooled.c"
#endif

// we replace the function which checks
// for an abort() condition in case we
// didn't compile any debug version because
// our debug version itself may use abort()
// for the ASSERT() macro.
#if defined(__GNUC__)
  #if defined(__libnix)
    void __chkabort(void) {}
  #elif !defined(__NEWLIB__)
    void __check_abort(void) {}
  #endif
#elif defined(__VBCC__)
  void _chkabort(void) {}
#endif
