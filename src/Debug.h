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

#include <stdarg.h>

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
#define DBF_TIMER    (1<<2)     // for timer debug output (Timer.c)
#define DBF_CONFIG   (1<<3)     // for configuration management (YAM_CO.c)
#define DBF_FILTER   (1<<4)     // for filter/search management (YAM_FI.c)
#define DBF_FOLDER   (1<<5)     // for folder management (YAM_FO.c, YAM_MAf.c)
#define DBF_MAIL     (1<<6)     // for mail management (YAM_MA.c)
#define DBF_MIME     (1<<7)     // for mime encoding/decoding (Mime.c)
#define DBF_GUI      (1<<8)     // for GUI management output
#define DBF_REXX     (1<<9)     // for REXX management output
#define DBF_NET      (1<<10)    // for Network management output (YAM_TR.c)
#define DBF_UTIL     (1<<11)    // for utility management output (YAM_UT.c)
#define DBF_IMPORT   (1<<12)    // for import of messages (YAM_MA.c,YAM_TR.c)
#define DBF_XPK      (1<<13)    // for XPKmaster.library stuff
#define DBF_IMAGE    (1<<14)    // for loading/mainting image files (and cache)
#define DBF_UPDATE   (1<<15)    // for the updatecheck facility (UpdateCheck.c)
#define DBF_HTML     (1<<16)    // for the html convert engine (HTML2Mail.c)
#define DBF_SPAM     (1<<17)    // for the spam dectection engine (BayesFilter.c)
#define DBF_UIDL     (1<<18)    // for the UIDL managment (YAM_TR.c)
#define DBF_HASH     (1<<19)    // for the HashTable management (HashTable.c)
#define DBF_PRINT    (1<<20)    // for print management
#define DBF_THEME    (1<<21)    // for the Themes management (Themes.c)
#define DBF_THREAD   (1<<22)    // for the Thread management (Thread.c)
#define DBF_MEMORY   (1<<23)    // for memory tracking (Debug.c)
#define DBF_ALL      0xffffffff

void SetupDebug(void);
void CleanupDebug(void);

void _ENTER(unsigned long dclass, const char *file, unsigned long line, const char *function);
void _LEAVE(unsigned long dclass, const char *file, unsigned long line, const char *function);
void _RETURN(unsigned long dclass, const char *file, unsigned long line, const char *function, unsigned long result);
void _SHOWVALUE(unsigned long dclass, unsigned long dflags, unsigned long value, int size, const char *name, const char *file, unsigned long line);
void _SHOWPOINTER(unsigned long dclass, unsigned long dflags, const void *p, const char *name, const char *file, unsigned long line);
void _SHOWSTRING(unsigned long dclass, unsigned long dflags, const char *string, const char *name, const char *file, unsigned long line);
void _SHOWMSG(unsigned long dclass, unsigned long dflags, const char *msg, const char *file, unsigned long line);
void _DPRINTF(unsigned long dclass, unsigned long dflags, const char *file, unsigned long line, const char *format, ...);
void _VDPRINTF(unsigned long dclass, unsigned long dflags, const char *file, unsigned long line, const char *format, va_list args);
void _STARTCLOCK(const char *file, unsigned long line);
void _STOPCLOCK(unsigned long dflags, const char *message, const char *file, unsigned long line);

#if defined(__SASC)
  #define __FUNCTION__        __FUNC__
  #define NO_VARARG_MARCOS    1
#endif

// Core class information class messages
#define ENTER()               _ENTER(DBC_CTRACE, __FILE__, __LINE__, __FUNCTION__)
#define LEAVE()               _LEAVE(DBC_CTRACE, __FILE__, __LINE__, __FUNCTION__)
#define RETURN(r)             _RETURN(DBC_CTRACE, __FILE__, __LINE__, __FUNCTION__, (long)r)
#define SHOWVALUE(f, v)       _SHOWVALUE(DBC_REPORT, f, (long)v, sizeof(v), #v, __FILE__, __LINE__)
#define SHOWPOINTER(f, p)     _SHOWPOINTER(DBC_REPORT, f, p, #p, __FILE__, __LINE__)
#define SHOWSTRING(f, s)      _SHOWSTRING(DBC_REPORT, f, s, #s, __FILE__, __LINE__)
#define SHOWMSG(f, m)         _SHOWMSG(DBC_REPORT, f, m, __FILE__, __LINE__)
#define STARTCLOCK()          _STARTCLOCK(__FILE__, __LINE__)
#define STOPCLOCK(f, m)       _STOPCLOCK(f, m, __FILE__, __LINE__)
#if defined(NO_VARARG_MARCOS)
void D(unsigned long f, const char *format, ...);
void E(unsigned long f, const char *format, ...);
void W(unsigned long f, const char *format, ...);
#else
#define D(f, ...)             _DPRINTF(DBC_DEBUG, f, __FILE__, __LINE__, __VA_ARGS__)
#define E(f, ...)             _DPRINTF(DBC_ERROR, f, __FILE__, __LINE__, __VA_ARGS__)
#define W(f, ...)             _DPRINTF(DBC_WARNING, f, __FILE__, __LINE__, __VA_ARGS__)
#endif
#define ASSERT(expression)      \
  ((void)                       \
   ((expression) ? 0 :          \
    (                            \
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

void *dbg_malloc(const char *file, const int line, size_t size);
void *dbg_calloc(const char *file, const int line, size_t n, size_t size);
void *dbg_realloc(const char *file, const int line, void *ptr, size_t size);
void dbg_free(const char *file, const int line, void *ptr);
char *dbg_strdup(const char *file, const int line, const char *s);
void *dbg_memdup(const char *file, const int line, const void *ptr, const size_t size);

#if !defined(DEBUG_USE_MALLOC_REDEFINE)
#define malloc(s)             dbg_malloc(__FILE__, __LINE__, s)
#define calloc(n, s)          dbg_calloc(__FILE__, __LINE__, n, s)
#define realloc(p, s)         dbg_realloc(__FILE__, __LINE__, p, s)
#define free(p)               dbg_free(__FILE__, __LINE__, p)
#define strdup(s)             dbg_strdup(__FILE__, __LINE__, s)
#define memdup(p, s)          dbg_memdup(__FILE__, __LINE__, p, s)
#endif

void DumpDbgMalloc(void);

#else // DEBUG

// to replace with NOPs is important here!
#define ENTER()               ((void)0)
#define LEAVE()               ((void)0)
#define RETURN(r)             ((void)0)
#define SHOWVALUE(f, v)       ((void)0)
#define SHOWPOINTER(f, p)     ((void)0)
#define SHOWSTRING(f, s)      ((void)0)
#define SHOWMSG(f, m)         ((void)0)
#define STARTCLOCK()          ((void)0)
#define STOPCLOCK(f, m)       ((void)0)
#define D(f, ...)             ((void)0)
#define E(f, ...)             ((void)0)
#define W(f, ...)             ((void)0)
#define ASSERT(expression)    ((void)0)

#define DumpDbgMalloc()       ((void)0)

#endif // DEBUG

#endif // DEBUG_H
