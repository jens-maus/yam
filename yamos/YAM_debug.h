#ifndef YAM_DEBUG_H
#define YAM_DEBUG_H

/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2001 by YAM Open Source Team

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

#if defined(DEBUG)
  #if defined(__amigaos4__)
    #include <proto/exec.h>
    #ifdef __USE_INLINE__
      #ifdef DebugPrintF
        #undef DebugPrintF
      #endif
    #endif
    #ifndef kprintf
      #define kprintf(format, args...)  ((struct ExecIFace *)((*(struct ExecBase **)4L)->MainInterface))->DebugPrintF(format, ## args)
    #endif
  #else
    void kprintf(const char *formatString,...);
  #endif

  #define DB(x) x
  #define DBG kprintf("File %s, Func %s, Line %d\n",__FILE__,__FUNC__,__LINE__);
#else
  #define DB(x)
  #define DBG
#endif

#endif /* YAM_DEBUG_H */
