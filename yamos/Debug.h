/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2005 by YAM Open Source Team

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

 YAM Official Support Site :  http://www.yam.ch/
 YAM OpenSource project    :  http://sourceforge.net/projects/yamos/

 $Id$

***************************************************************************/

#ifndef DEBUG_H
#define DEBUG_H

// first we make sure all previously defined symbols are undefined now so
// that no other debug system interferes with ours.
#undef ENTER
#undef LEAVE
#undef RETURN
#undef SHOWVALUE
#undef SHOWPOINTER
#undef SHOWSTRING
#undef SHOWMSG
#undef STARTCLOCK
#undef STOPCLOCK
#undef D
#undef E
#undef W
#undef ASSERT

#if defined(DEBUG)

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#if defined(__amigaos4__)
  #include <proto/exec.h>
  #ifdef __USE_INLINE__
    #ifdef DebugPrintF
      #undef DebugPrintF
    #endif
  #endif
  #ifndef kprintf
    #define kprintf(format, args...)  ((struct ExecIFace *)((*(struct ExecBase **)4)->MainInterface))->DebugPrintF(format, ## args)
  #endif
#else
void kprintf(const char *formatString,...);
#endif

// debug classes
#define DBC_CTRACE   (1<<0) // call tracing (ENTER/LEAVE etc.)
#define DBC_REPORT   (1<<1) // reports (SHOWVALUE/SHOWSTRING etc.)
#define DBC_ASSERT   (1<<2) // asserts (ASSERT)
#define DBC_TIMEVAL  (1<<3) // time evaluations (STARTCLOCK/STOPCLOCK)
#define DBC_DEBUG    (1<<4) // debugging output D()
#define DBC_ERROR    (1<<5) // error output     E()
#define DBC_WARNING  (1<<6) // warning output   W()
#define DBC_ALL      0xffffffff

// debug flags
#define DBF_ALWAYS   (1<<0)
#define DBF_STARTUP  (1<<1)     // for startup/shutdown events (YAM.c)
#define DBF_TIMERIO  (1<<2)     // for timerIO debug output (YAM.c)
#define DBF_CONFIG   (1<<3)     // for configuration management (YAM_CO.c)
#define DBF_FILTER   (1<<4)     // for filter/search management (YAM_FI.c)
#define DBF_FOLDER   (1<<5)     // for folder management (YAM_FO.c, YAM_MAf.c)
#define DBF_MAIL     (1<<6)     // for mail management (YAM_MA.c)
#define DBF_MIME     (1<<7)     // for mime encoding/decoding (YAM_MI.c)
#define DBF_GUI      (1<<8)     // for GUI management output
#define DBF_REXX     (1<<9)     // for REXX management output
#define DBF_NET      (1<<10)    // for Network management output (YAM_TR.c)
#define DBF_UTIL     (1<<11)    // for utility management output (YAM_UT.c)
#define DBF_ALL      0xffffffff

void SetupDebug(void);

void _ENTER(unsigned long dclass, const char *file, int line, const char *function);
void _LEAVE(unsigned long dclass, const char *file, int line, const char *function);
void _RETURN(unsigned long dclass, const char *file, int line, const char *function, unsigned long result);
void _SHOWVALUE(unsigned long dclass, unsigned long dflags, unsigned long value, int size, const char *name, const char *file, int line);
void _SHOWPOINTER(unsigned long dclass, unsigned long dflags, const void *p, const char *name, const char *file, int line);
void _SHOWSTRING(unsigned long dclass, unsigned long dflags, const char *string, const char *name, const char *file, int line);
void _SHOWMSG(unsigned long dclass, unsigned long dflags, const char *msg, const char *file, int line);
void _DPRINTF(unsigned long dclass, unsigned long dflags, const char *file, int line, const char *format, ...);

// Core class information class messages
#define ENTER()					     _ENTER(DBC_CTRACE, __FILE__, __LINE__, __FUNCTION__)
#define LEAVE()					     _LEAVE(DBC_CTRACE, __FILE__, __LINE__, __FUNCTION__)
#define RETURN(r)				     _RETURN(DBC_CTRACE, __FILE__, __LINE__, __FUNCTION__, (long)r)
#define SHOWVALUE(f, v)		   _SHOWVALUE(DBC_REPORT, f, (long)v, sizeof(v), #v, __FILE__, __LINE__)
#define SHOWPOINTER(f, p)	   _SHOWPOINTER(DBC_REPORT, f, p, #p, __FILE__, __LINE__)
#define SHOWSTRING(f, s)	   _SHOWSTRING(DBC_REPORT, f, s, #s, __FILE__, __LINE__)
#define SHOWMSG(f, m)			   _SHOWMSG(DBC_REPORT, f, m, __FILE__, __LINE__)
#define D(f, s, vargs...)	   _DPRINTF(DBC_DEBUG, f, __FILE__, __LINE__, s, ## vargs)
#define E(f, s, vargs...)	   _DPRINTF(DBC_ERROR, f, __FILE__, __LINE__, s, ## vargs)
#define W(f, s, vargs...)	   _DPRINTF(DBC_WARNING, f, __FILE__, __LINE__, s, ## vargs)
#define ASSERT(expression)      \
	((void)                       \
	 ((expression) ? 0 :          \
	  (														\
	   _DPRINTF(DBC_ASSERT,       \
              DBF_ALWAYS,       \
              __FILE__,         \
              __LINE__,         \
              "failed assertion '%s'", \
	            #expression),     \
	   abort(),                   \
	   0                          \
	  )                           \
	 )                            \
	)

#else // DEBUG

#define ENTER()							((void)0)
#define LEAVE()							((void)0)
#define RETURN(r)						((void)0)
#define SHOWVALUE(f, v)			((void)0)
#define SHOWPOINTER(f, p)		((void)0)
#define SHOWSTRING(f, s)		((void)0)
#define SHOWMSG(f, m)			  ((void)0)
#define D(f, s, vargs...)		((void)0)
#define E(f, s, vargs...)		((void)0)
#define W(f, s, vargs...)		((void)0)
#define ASSERT(expression)	((void)0)

#endif // DEBUG

#endif // DEBUG_H
