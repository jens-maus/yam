/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2010 by YAM Open Source Team

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

#ifdef DEBUG

#include <stdio.h> // vsnprintf
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/timer.h>
#include <proto/utility.h>
#include <clib/alib_protos.h>

#include "YAM_global.h"
#include "YAM_utilities.h" // CLEAR_FLAG,SET_FLAG
#include "extrasrc.h"

#include "SDI_compiler.h"
#include "timeval.h"

#define DEBUG_USE_MALLOC_REDEFINE 1
#include "Debug.h"

#if defined(__MORPHOS__)
#include <exec/rawfmt.h>
#elif defined(__AROS__)
#include <proto/arossupport.h>
#else
#include <clib/debug_protos.h>
#endif

// our static variables with default values
static BOOL ansi_output = FALSE;
static BOOL stdout_output = FALSE;
static FILE *file_output = NULL;
static ULONG debug_flags = DBF_ALWAYS | DBF_STARTUP; // default debug flags
static ULONG debug_classes = DBC_ERROR | DBC_DEBUG | DBC_WARNING | DBC_ASSERT | DBC_REPORT | DBC_MTRACK; // default debug classes
static char debug_modules[256] = "";
static char debug_files[256] = "";
static int timer_level = -1;
static struct TimeVal startTimes[8];

// static function prototypes
static void SetupDbgMalloc(void);
static void CleanupDbgMalloc(void);

/****************************************************************************/

// define variables for using ANSI colors in our debugging scheme
#define ANSI_ESC_CLR        "\033[0m"
#define ANSI_ESC_BOLD       "\033[1m"
#define ANSI_ESC_UNDERLINE  "\033[4m"
#define ANSI_ESC_BLINK      "\033[5m"
#define ANSI_ESC_REVERSE    "\033[7m"
#define ANSI_ESC_INVISIBLE  "\033[8m"
#define ANSI_ESC_FG_BLACK   "\033[0;30m"
#define ANSI_ESC_FG_RED     "\033[0;31m"
#define ANSI_ESC_FG_GREEN   "\033[0;32m"
#define ANSI_ESC_FG_BROWN   "\033[0;33m"
#define ANSI_ESC_FG_BLUE    "\033[0;34m"
#define ANSI_ESC_FG_PURPLE  "\033[0;35m"
#define ANSI_ESC_FG_CYAN    "\033[0;36m"
#define ANSI_ESC_FG_LGRAY   "\033[0;37m"
#define ANSI_ESC_FG_DGRAY   "\033[1;30m"
#define ANSI_ESC_FG_LRED    "\033[1;31m"
#define ANSI_ESC_FG_LGREEN  "\033[1;32m"
#define ANSI_ESC_FG_YELLOW  "\033[1;33m"
#define ANSI_ESC_FG_LBLUE   "\033[1;34m"
#define ANSI_ESC_FG_LPURPLE "\033[1;35m"
#define ANSI_ESC_FG_LCYAN   "\033[1;36m"
#define ANSI_ESC_FG_WHITE   "\033[1;37m"
#define ANSI_ESC_BG         "\033[0;4"    // background esc-squ start with 4x
#define ANSI_ESC_BG_BLACK   "\033[0;40m"
#define ANSI_ESC_BG_RED     "\033[0;41m"
#define ANSI_ESC_BG_GREEN   "\033[0;42m"
#define ANSI_ESC_BG_BROWN   "\033[0;43m"
#define ANSI_ESC_BG_BLUE    "\033[0;44m"
#define ANSI_ESC_BG_PURPLE  "\033[0;45m"
#define ANSI_ESC_BG_CYAN    "\033[0;46m"
#define ANSI_ESC_BG_LGRAY   "\033[0;47m"

// define the colors for each debug class
#define DBC_CTRACE_COLOR    ANSI_ESC_FG_BROWN
#define DBC_REPORT_COLOR    ANSI_ESC_FG_PURPLE
#define DBC_ASSERT_COLOR    ANSI_ESC_FG_RED
#define DBC_TIMEVAL_COLOR   ANSI_ESC_FG_BLUE
#define DBC_DEBUG_COLOR     ANSI_ESC_FG_GREEN
#define DBC_ERROR_COLOR     ANSI_ESC_FG_RED
#define DBC_WARNING_COLOR   ANSI_ESC_FG_YELLOW
#define DBC_CTRACE_BGCOLOR  ANSI_ESC_BG_BROWN
#define DBC_REPORT_BGCOLOR  ANSI_ESC_BG_PURPLE
#define DBC_ASSERT_BGCOLOR  ANSI_ESC_BG_RED
#define DBC_TIMEVAL_BGCOLOR ANSI_ESC_BG_BLUE
#define DBC_DEBUG_BGCOLOR   ANSI_ESC_BG_GREEN
#define DBC_ERROR_BGCOLOR   ANSI_ESC_BG_RED
#define DBC_WARNING_BGCOLOR ANSI_ESC_BG_CYAN

// define output string IDs for the varias debug classes
#define DBC_CTRACE_STR      "C"
#define DBC_REPORT_STR      "R"
#define DBC_ASSERT_STR      "A"
#define DBC_TIMEVAL_STR     "T"
#define DBC_DEBUG_STR       "D"
#define DBC_ERROR_STR       "E"
#define DBC_WARNING_STR     "W"

// the thread macros to use
#if defined(NO_THREADS)

#define THREAD_MAX          1
#define THREAD_LOCK         (void(0))
#define THREAD_UNLOCK       (void(0))
#define THREAD_ID           0
#define INDENT_LEVEL        indent_level[THREAD_ID]

#else

#define THREAD_MAX          256
#define THREAD_LOCK         ObtainSemaphore(&thread_lock)
#define THREAD_UNLOCK       ReleaseSemaphore(&thread_lock)
#define THREAD_ID           _thread_id(FindTask(NULL))
#define INDENT_LEVEL        indent_level[THREAD_ID]

static struct SignalSemaphore thread_lock;
static void *thread_id[THREAD_MAX];

#endif

#define INDENT_MAX          80
static int indent_level[THREAD_MAX];
static char indent_spaces[INDENT_MAX];

/****************************************************************************/

#if !defined(NO_THREADS)
INLINE int _thread_id(const void *thread_ptr)
{
  int result=-1;
  int i=0;

  while(i < THREAD_MAX && thread_id[i] != NULL)
  {
    if(thread_id[i] == thread_ptr)
    {
      result = i;
      break;
    }

    i++;
  }

  if(result == -1)
  {
    if(i < THREAD_MAX)
    {
      thread_id[i] = (void *)thread_ptr;
      result = i;
    }
  }

  return result;
}

#endif

static void _DBPRINTF(const char *format, ...)
{
  va_list args;

  va_start(args, format);

  if(stdout_output == TRUE)
    vprintf(format, args);
  else if(file_output != NULL)
    vfprintf(file_output, format, args);
  else
  {
    #if defined(__MORPHOS__)
    VNewRawDoFmt(format, (APTR)RAWFMTFUNC_SERIAL, NULL, args);
    #elif defined(__amigaos4__)
    char buf[1024];
    vsnprintf(buf, sizeof(buf), format, args);
    DebugPrintF("%s", buf);
    #elif defined(__AROS__)
    vkprintf(format, args);
    #else
    KPutFmt(format, args);
    #endif
  }

  va_end(args);
}

/****************************************************************************/

INLINE char *_INDENT(void)
{
  int levels = INDENT_LEVEL;
  int i;

  for(i=0; i < levels && i < INDENT_MAX; i++)
    indent_spaces[i] = ' ';

  indent_spaces[i] = '\0';

  return indent_spaces;
}

/****************************************************************************/

INLINE BOOL matchDebugSpec(const unsigned long c, const unsigned f,
                           const char *m, const char *file)
{
  BOOL match = FALSE;

  // first we check if we need to process this debug message or not,
  // depending on the currently set debug class/flags
  if((isFlagSet(debug_classes, c) && isFlagSet(debug_flags, f)) ||
     (isFlagSet(c, DBC_ERROR) || isFlagSet(c, DBC_WARNING)))
  {
    match = TRUE;
  }
  else if(stristr(debug_modules, m) != NULL)
    match = TRUE;
  else if(stristr(debug_files, file) != NULL)
    match = TRUE;

  return match;
}

/****************************************************************************/

INLINE void _VDPRINTF(const unsigned long c,
                      const char *file, unsigned long line,
                      const char *format, va_list args)
{
  char buf[1024];
  const char *fg;
  const char *bg;
  const char *id;
  const int threadID = THREAD_ID;

  vsnprintf(buf, sizeof(buf), format, args);

  switch(c)
  {
    case DBC_CTRACE:  fg = DBC_CTRACE_COLOR;  bg = DBC_CTRACE_BGCOLOR;  id = DBC_CTRACE_STR;  break;
    case DBC_REPORT:  fg = DBC_REPORT_COLOR;  bg = DBC_REPORT_BGCOLOR;  id = DBC_REPORT_STR;  break;
    case DBC_ASSERT:  fg = DBC_ASSERT_COLOR;  bg = DBC_ASSERT_BGCOLOR;  id = DBC_ASSERT_STR;  break;
    case DBC_TIMEVAL: fg = DBC_TIMEVAL_COLOR; bg = DBC_TIMEVAL_BGCOLOR; id = DBC_TIMEVAL_STR; break;
    case DBC_DEBUG:   fg = DBC_DEBUG_COLOR;   bg = DBC_DEBUG_BGCOLOR;   id = DBC_DEBUG_STR;   break;
    case DBC_ERROR:   fg = DBC_ERROR_COLOR;   bg = DBC_ERROR_BGCOLOR;   id = DBC_ERROR_STR;   break;
    case DBC_WARNING: fg = DBC_WARNING_COLOR; bg = DBC_WARNING_BGCOLOR; id = DBC_WARNING_STR; break;
    default:          fg = ANSI_ESC_FG_WHITE; bg = ANSI_ESC_FG_WHITE;   id = DBC_DEBUG_STR;   break;
  }

  if(ansi_output)
  {
    _DBPRINTF("%s%ldm%02ld:%s%s%s%s%s:%s%s:%ld:%s%s\n",
                ANSI_ESC_BG, (threadID+1)%6, threadID, ANSI_ESC_CLR,
                bg, id, ANSI_ESC_CLR, fg, _INDENT(),
                (strrchr(file, '/') ? strrchr(file, '/')+1 : file),
                line, buf, ANSI_ESC_CLR);
  }
  else
  {
    _DBPRINTF("%02ld:%s:%s%s:%ld:%s\n",
                threadID,
                id, _INDENT(),
                (strrchr(file, '/') ? strrchr(file, '/')+1 : file),
                line, buf);
  }
}

/****************************************************************************/

void SetupDebug(void)
{
  char var[256];

  #if !defined(NO_THREADS)
  memset(&thread_lock, 0, sizeof(thread_lock));
  memset(&thread_id, 0, sizeof(thread_id));
  InitSemaphore(&thread_lock);
  #endif

  memset(&indent_level, 0, sizeof(indent_level));

  if(GetVar("yamdebug", var, sizeof(var), 0) > 0)
  {
    char *s = var;
    char *t;

    // static list of our debugging classes tokens.
    // in the yamdebug variable these classes always start with a @
    static const struct { const char *token; unsigned long flag; } dbclasses[] =
    {
      { "ctrace",  DBC_CTRACE   },
      { "report",  DBC_REPORT   },
      { "assert",  DBC_ASSERT   },
      { "timeval", DBC_TIMEVAL  },
      { "debug",   DBC_DEBUG    },
      { "error",   DBC_ERROR    },
      { "warning", DBC_WARNING  },
      { "mtrack",  DBC_MTRACK   },
      { "tags",    DBC_TAGS     },
      { "all",     DBC_ALL      },
      { NULL,      0            }
    };

    static const struct { const char *token; unsigned long flag; } dbflags[] =
    {
      { "always",   DBF_ALWAYS  },
      { "startup",  DBF_STARTUP },
      { "timer",    DBF_TIMER   },
      { "config",   DBF_CONFIG  },
      { "filter",   DBF_FILTER  },
      { "folder",   DBF_FOLDER  },
      { "mail",     DBF_MAIL    },
      { "mime",     DBF_MIME    },
      { "gui",      DBF_GUI     },
      { "rexx",     DBF_REXX    },
      { "net",      DBF_NET     },
      { "util",     DBF_UTIL    },
      { "import",   DBF_IMPORT  },
      { "xpk",      DBF_XPK     },
      { "image",    DBF_IMAGE   },
      { "update",   DBF_UPDATE  },
      { "html",     DBF_HTML    },
      { "spam",     DBF_SPAM    },
      { "uidl",     DBF_UIDL    },
      { "hash",     DBF_HASH    },
      { "print",    DBF_PRINT   },
      { "theme",    DBF_THEME   },
      { "thread",   DBF_THREAD  },
      { "all",      DBF_ALL     },
      { NULL,       0           }
    };

    // before we search for our debug tokens we search
    // for our special output tokens
    if(stristr(s, "ansi") != NULL)
      ansi_output = TRUE;

    if(stristr(s, "stdout") != NULL)
      stdout_output = TRUE;

    if((t = stristr(s, "file:")) != NULL)
    {
      char *e;
      char filename[256];

      // put t to the first char of the filename
      t += 5;

      // the user wants to output the debugging information
      // to a file instead
      if((e = strpbrk(t, " ,;")) == NULL)
        strlcpy(filename, t, sizeof(filename));
      else
        strlcpy(filename, t, e-t+1);

      // now we can open the file for output of the debug info
      file_output = fopen(filename, "w");
    }

    // output information on the debugging settings
    _DBPRINTF("** %s date %s (build %s) startup **********************\n", yamversion, yamversiondate, yambuildid);
    _DBPRINTF("Exec version: v%ld.%ld\n", (unsigned long)((struct Library *)SysBase)->lib_Version, (unsigned long)((struct Library *)SysBase)->lib_Revision);
    _DBPRINTF("Initializing runtime debugging:\n");

    // we parse the env variable token-wise
    while(*s)
    {
      ULONG i;
      char *e;

      if((e = strpbrk(s, " ,;")) == NULL)
        e = s+strlen(s);

      // check if the token is class definition or
      // just a flag definition
      if(s[0] == '@')
      {
        // skip the '@'
        s++;

        // check if this call is a negation or not
        if(s[0] == '!')
        {
          // skip the '!'
          s++;

          // search for the token and clear the flag
          for(i=0; dbclasses[i].token; i++)
          {
            if(strnicmp(s, dbclasses[i].token, strlen(dbclasses[i].token)) == 0)
            {
              _DBPRINTF("clear '%s' debug class flag.\n", dbclasses[i].token);

              CLEAR_FLAG(debug_classes, dbclasses[i].flag);
            }
          }
        }
        else
        {
          // search for the token and set the flag
          for(i=0; dbclasses[i].token; i++)
          {
            if(strnicmp(s, dbclasses[i].token, strlen(dbclasses[i].token)) == 0)
            {
              _DBPRINTF("set '%s' debug class flag\n", dbclasses[i].token);

              SET_FLAG(debug_classes, dbclasses[i].flag);
            }
          }
        }
      }
      else
      {
        // check if this call is a negation or not
        if(s[0] == '!')
        {
          // skip the '!'
          s++;
          for(i=0; dbflags[i].token; i++)
          {
            if(strnicmp(s, dbflags[i].token, strlen(dbflags[i].token)) == 0)
            {
              _DBPRINTF("clear '%s' debug flag\n", dbflags[i].token);

              CLEAR_FLAG(debug_flags, dbflags[i].flag);
            }
          }
        }
        else
        {
          // check if the token was "ansi" and if so enable the ANSI color
          // output
          if(strnicmp(s, "ansi", 4) == 0)
          {
            _DBPRINTF("ANSI output enabled\n");
            ansi_output = TRUE;
          }
          else if(strnicmp(s, "stdout", 6) == 0)
          {
            stdout_output = TRUE;
            _DBPRINTF("CON: output enabled\n");
          }
          else if(strnicmp(s, "file:", 5) == 0)
          {
            char *ee;
            char *tt = s+5;
            char filename[256];

            // the user wants to output the debugging information
            // to a file instead
            if((ee = strpbrk(tt, " ,;")) == NULL)
              strlcpy(filename, tt, sizeof(filename));
            else
              strlcpy(filename, tt, ee-tt+1);

            _DBPRINTF("FILE output enabled to: '%s'\n", filename);
          }
          else
          {
            for(i=0; dbflags[i].token; i++)
            {
              if(strnicmp(s, dbflags[i].token, strlen(dbflags[i].token)) == 0)
              {
                _DBPRINTF("set '%s' debug flag\n", dbflags[i].token);

                SET_FLAG(debug_flags, dbflags[i].flag);
              }
            }
          }
        }
      }

      // set the next start to our last search
      if(*e)
        s = ++e;
      else
        break;
    }
  }
  else
  {
    // output information on the debugging settings
    _DBPRINTF("** %s build: %s startup **********************\n", yamversion, yambuildid);
    _DBPRINTF("Exec version: v%ld.%ld\n", (unsigned long)((struct Library *)SysBase)->lib_Version, (unsigned long)((struct Library *)SysBase)->lib_Revision);
    _DBPRINTF("no 'yamdebug' variable found\n");
  }

  _DBPRINTF("set debug classes/flags (env:yamdebug): %08lx/%08lx\n", debug_classes, debug_flags);
  _DBPRINTF("** Normal processing follows ***************************************\n");

  SetupDbgMalloc();
}

/****************************************************************************/

void CleanupDebug(void)
{
  CleanupDbgMalloc();

  _DBPRINTF("** Cleaned up debugging ********************************************\n");

  if(file_output != NULL)
  {
    fclose(file_output);
    file_output = NULL;
  }
}

/****************************************************************************/

#define checkIndentLevel(l) { \
  if(INDENT_LEVEL < l) \
  { \
    if(ansi_output) \
      _DBPRINTF("%s%s:%ld:indent level less than %ld (%ld)%s\n", ANSI_ESC_FG_PURPLE, file, line, l, INDENT_LEVEL, ANSI_ESC_CLR); \
    else \
      _DBPRINTF("%s:%ld:indent level less than %ld (%ld)\n", file, line, l, INDENT_LEVEL); \
  } \
}

/****************************************************************************/

void _ENTER(const unsigned long c, const char *m,
            const char *file, const unsigned long line,
            const char *function)
{
  THREAD_LOCK;

  if(matchDebugSpec(c, 0, m, file) == TRUE)
  {
    const int threadID = THREAD_ID;

    if(ansi_output)
    {
      _DBPRINTF("%s%ldm%02ld:%s%s:%s%s:%ld:Entering %s%s\n",
                  ANSI_ESC_BG, (threadID+1)%6, threadID, ANSI_ESC_CLR,
                  DBC_CTRACE_BGCOLOR DBC_CTRACE_STR ANSI_ESC_CLR DBC_CTRACE_COLOR,
                  _INDENT(),
                  (strrchr(file, '/') ? strrchr(file, '/')+1 : file),
                  line, function, ANSI_ESC_CLR);
    }
    else
    {
      _DBPRINTF("%02ld:%s:%s%s:%ld:Entering %s\n",
                  threadID,
                  DBC_CTRACE_STR,
                  _INDENT(),
                  (strrchr(file, '/') ? strrchr(file, '/')+1 : file),
                  line, function);
    }

    checkIndentLevel(0);
  }

  INDENT_LEVEL+=1;

  THREAD_UNLOCK;
}

/****************************************************************************/

void _LEAVE(const unsigned long c, const char *m,
            const char *file, const unsigned long line,
            const char *function)
{
  THREAD_LOCK;

  INDENT_LEVEL-=1;

  if(matchDebugSpec(c, 0, m, file) == TRUE)
  {
    const int threadID = THREAD_ID;

    if(ansi_output)
    {
      _DBPRINTF("%s%ldm%02ld:%s%s:%s%s:%ld:Leaving %s%s\n",
                  ANSI_ESC_BG, (threadID+1)%6, threadID, ANSI_ESC_CLR,
                  DBC_CTRACE_BGCOLOR DBC_CTRACE_STR ANSI_ESC_CLR DBC_CTRACE_COLOR,
                  _INDENT(),
                  (strrchr(file, '/') ? strrchr(file, '/')+1 : file),
                  line, function, ANSI_ESC_CLR);
    }
    else
    {
      _DBPRINTF("%02ld:%s:%s%s:%ld:Leaving %s\n",
                  threadID,
                  DBC_CTRACE_STR,
                  _INDENT(),
                  (strrchr(file, '/') ? strrchr(file, '/')+1 : file),
                  line, function);
    }

    checkIndentLevel(0);
  }

  THREAD_UNLOCK;
}

/****************************************************************************/

void _RETURN(const unsigned long c, const char *m,
             const char *file, const unsigned long line,
             const char *function, unsigned long result)
{
  THREAD_LOCK;

  INDENT_LEVEL-=1;

  if(matchDebugSpec(c, 0, m, file) == TRUE)
  {
    const int threadID = THREAD_ID;

    if(ansi_output)
    {
      _DBPRINTF("%s%ldm%02ld:%s%s:%s%s:%ld:Leaving %s (result 0x%08lx, %ld)%s\n",
                  ANSI_ESC_BG, (threadID+1)%6, threadID, ANSI_ESC_CLR,
                  DBC_CTRACE_BGCOLOR DBC_CTRACE_STR ANSI_ESC_CLR DBC_CTRACE_COLOR,
                  _INDENT(),
                  (strrchr(file, '/') ? strrchr(file, '/')+1 : file),
                  line, function, result, result, ANSI_ESC_CLR);
    }
    else
    {
      _DBPRINTF("%02ld:%s:%s%s:%ld:Leaving %s (result 0x%08lx, %ld)\n",
                  threadID,
                  DBC_CTRACE_STR,
                  _INDENT(),
                  (strrchr(file, '/') ? strrchr(file, '/')+1 : file),
                  line, function, result, result);
    }

    checkIndentLevel(0);
  }

  THREAD_UNLOCK;
}

/****************************************************************************/

void _CHECKINDENT(const unsigned long c,
                  const char *file, const unsigned long line,
                  const long level)
{
  THREAD_LOCK;

  if(matchDebugSpec(c, 0, NULL, file) == TRUE)
    checkIndentLevel(level);

  THREAD_UNLOCK;
}

/****************************************************************************/

void _SHOWVALUE(const unsigned long c, const unsigned long f, const char *m,
                const char *file, const unsigned long line,
                const unsigned long value, const int size, const char *name)
{
  THREAD_LOCK;

  if(matchDebugSpec(c, f, m, file) == TRUE)
  {
    const int threadID = THREAD_ID;
    const char *fmt;

    if(ansi_output)
    {
      _DBPRINTF("%s%ldm%02ld:%s%s:%s%s:%ld:%s = %ld, 0x",
                  ANSI_ESC_BG, (threadID+1)%6, threadID, ANSI_ESC_CLR,
                  DBC_REPORT_BGCOLOR DBC_REPORT_STR ANSI_ESC_CLR DBC_REPORT_COLOR,
                  _INDENT(),
                  (strrchr(file, '/') ? strrchr(file, '/')+1 : file),
                  line, name, value);
    }
    else
    {
      _DBPRINTF("%02ld:%s:%s%s:%ld:%s = %ld, 0x",
                  threadID,
                  DBC_CTRACE_STR,
                  _INDENT(),
                  (strrchr(file, '/') ? strrchr(file, '/')+1 : file),
                  line, name, value);
    }

    switch(size)
    {
      case 1:
        fmt = "%02lx";
      break;

      case 2:
        fmt = "%04lx";
      break;

      default:
        fmt = "%08lx";
      break;
    }

    _DBPRINTF(fmt, value);

    if(size == 1 && value < 256)
    {
      if(value < ' ' || (value >= 127 && value < 160))
        _DBPRINTF(", '\\x%02lx'", value);
      else
        _DBPRINTF(", '%c'", (unsigned char)value);
    }

    if(ansi_output)
      _DBPRINTF("%s\n", ANSI_ESC_CLR);
    else
      _DBPRINTF("\n");
  }

  THREAD_UNLOCK;
}

/****************************************************************************/

void _SHOWPOINTER(const unsigned long c, const unsigned long f, const char *m,
                  const char *file, const unsigned long line,
                  const void *p, const char *name)
{
  THREAD_LOCK;

  if(matchDebugSpec(c, f, m, file) == TRUE)
  {
    const int threadID = THREAD_ID;

    if(ansi_output)
    {
      _DBPRINTF("%s%ldm%02ld:%s%s:%s%s:%ld:%s = ",
                  ANSI_ESC_BG, (threadID+1)%6, threadID, ANSI_ESC_CLR,
                  DBC_REPORT_BGCOLOR DBC_REPORT_STR ANSI_ESC_CLR DBC_REPORT_COLOR,
                  _INDENT(),
                  (strrchr(file, '/') ? strrchr(file, '/')+1 : file),
                  line, name);
    }
    else
    {
      _DBPRINTF("%02ld:%s:%s%s:%ld:%s = ",
                  threadID,
                  DBC_CTRACE_STR,
                  _INDENT(),
                  (strrchr(file, '/') ? strrchr(file, '/')+1 : file),
                  line, name);
    }

    if(p != NULL)
      _DBPRINTF("0x%08lx", p);
    else
      _DBPRINTF("NULL");

    if(ansi_output)
      _DBPRINTF("%s\n", ANSI_ESC_CLR);
    else
      _DBPRINTF("\n");
  }

  THREAD_UNLOCK;
}

/****************************************************************************/

void _SHOWSTRING(const unsigned long c, const unsigned long f, const char *m,
                 const char *file, const unsigned long line,
                 const char *string, const char *name)
{
  THREAD_LOCK;

  if(matchDebugSpec(c, f, m, file) == TRUE)
  {
    const int threadID = THREAD_ID;

    if(ansi_output)
    {
      _DBPRINTF("%s%ldm%02ld:%s%s:%s%s:%ld:%s = 0x%08lx \"%s\"%s\n",
                  ANSI_ESC_BG, (threadID+1)%6, threadID, ANSI_ESC_CLR,
                  DBC_REPORT_BGCOLOR DBC_REPORT_STR ANSI_ESC_CLR DBC_REPORT_COLOR,
                  _INDENT(),
                  (strrchr(file, '/') ? strrchr(file, '/')+1 : file),
                  line, name, (unsigned long)string, string, ANSI_ESC_CLR);
    }
    else
    {
      _DBPRINTF("%02ld:%s:%s%s:%ld:%s = 0x%08lx \"%s\"\n",
                  threadID,
                  DBC_REPORT_STR,
                  _INDENT(),
                  (strrchr(file, '/') ? strrchr(file, '/')+1 : file),
                  line, name, (unsigned long)string, string);
    }
  }

  THREAD_UNLOCK;
}

/****************************************************************************/

void _SHOWMSG(const unsigned long c, const unsigned long f, const char *m,
              const char *file, const unsigned long line,
              const char *msg)
{
  THREAD_LOCK;

  if(matchDebugSpec(c, f, m, file) == TRUE)
  {
    const int threadID = THREAD_ID;

    if(ansi_output)
    {
      _DBPRINTF("%s%ldm%02ld:%s%s:%s%s:%ld:%s%s\n",
                  ANSI_ESC_BG, (threadID+1)%6, threadID, ANSI_ESC_CLR,
                  DBC_REPORT_BGCOLOR DBC_REPORT_STR ANSI_ESC_CLR DBC_REPORT_COLOR,
                  _INDENT(),
                  (strrchr(file, '/') ? strrchr(file, '/')+1 : file),
                  line, msg, ANSI_ESC_CLR);
    }
    else
    {
      _DBPRINTF("%02ld:%s:%s%s:%ld:%s\n",
                  threadID,
                  DBC_REPORT_STR,
                  _INDENT(),
                  (strrchr(file, '/') ? strrchr(file, '/')+1 : file),
                  line, msg);
    }
  }

  THREAD_UNLOCK;
}

/****************************************************************************/

void _SHOWTAGS(const unsigned long c, const unsigned long f, const char *m,
               const char *file, const unsigned long line,
               const struct TagItem *tags)
{
  THREAD_LOCK;

  if(matchDebugSpec(c, f, m, file) == TRUE)
  {
    int i;
    struct TagItem *tag;
    struct TagItem *tstate = (struct TagItem *)tags;
    const int threadID = THREAD_ID;

    if(ansi_output)
    {
      _DBPRINTF("%s%ldm%02ld:%s%s:%s%s:%ld:tag list %08lx%s\n",
                  ANSI_ESC_BG, (threadID+1)%6, threadID, ANSI_ESC_CLR,
                  DBC_REPORT_BGCOLOR DBC_REPORT_STR ANSI_ESC_CLR DBC_REPORT_COLOR,
                  _INDENT(),
                  (strrchr(file, '/') ? strrchr(file, '/')+1 : file),
                  line, tags, ANSI_ESC_CLR);
    }
    else
    {
      _DBPRINTF("%02ld:%s:%s%s:%ld:tag list %08lx\n",
                  threadID,
                  DBC_REPORT_STR,
                  _INDENT(),
                  (strrchr(file, '/') ? strrchr(file, '/')+1 : file),
                  line, tags);
    }

    INDENT_LEVEL+=1;

    i = 0;
    while((tag = NextTagItem(&tstate)) != NULL)
    {
      i++;

      if(ansi_output)
        _DBPRINTF("%s%2ld: tag=%08lx data=%08lx%s\n", DBC_REPORT_COLOR, i, tag->ti_Tag, tag->ti_Data, ANSI_ESC_CLR);
      else
        _DBPRINTF("%2ld: tag=%08lx data=%08lx\n", i, tag->ti_Tag, tag->ti_Data);
    }

    INDENT_LEVEL-=1;
  }

  THREAD_UNLOCK;
}

/****************************************************************************/

void _DPRINTF(const unsigned long c, const unsigned long f, const char *m,
              const char *file, unsigned long line,
              const char *format, ...)
{
  THREAD_LOCK;

  if(matchDebugSpec(c, f, m, file) == TRUE)
  {
    va_list args;

    va_start(args, format);
    _VDPRINTF(c, file, line, format, args);
    va_end(args);
  }

  THREAD_UNLOCK;
}

/****************************************************************************/

void _STARTCLOCK(const unsigned long c, const unsigned long f, const char *m,
                 const char *file, const unsigned long line)
{
  THREAD_LOCK;

  if(matchDebugSpec(c, f, m, file) == TRUE)
  {
    if(timer_level + 1 < (int)ARRAY_SIZE(startTimes))
    {
      timer_level++;
      GetSysTime(TIMEVAL(&startTimes[timer_level]));
    }
    else
      _DPRINTF(DBC_ERROR, DBF_ALWAYS, m, file, line, "already %ld clocks in use!", ARRAY_SIZE(startTimes));
  }

  THREAD_UNLOCK;
}

/****************************************************************************/

void _STOPCLOCK(const unsigned long c, const unsigned long f, const char *m,
                const char *file, const unsigned long line,
                const char *message)
{
  THREAD_LOCK;

  if(matchDebugSpec(c, f, m, file) == TRUE)
  {
    if(timer_level >= 0)
    {
      struct TimeVal stopTime;

      GetSysTime(TIMEVAL(&stopTime));
      SubTime(TIMEVAL(&stopTime), TIMEVAL(&startTimes[timer_level]));
      _DPRINTF(DBC_TIMEVAL, f, m, file, line, "operation '%s' took %ld.%09ld seconds", message, stopTime.Seconds, stopTime.Microseconds);
      timer_level--;
    }
    else
      _DPRINTF(DBC_ERROR, DBF_ALWAYS, m, file, line, "no clocks in use!");
  }

  THREAD_UNLOCK;
}

/****************************************************************************/

#if defined(NO_VARARG_MARCOS)
void D(const unsigned long f, const char *format, ...)
{
  THREAD_LOCK;

  if(matchDebugSpec(DBC_DEBUG, f, NULL, NULL) == TRUE)
  {
    va_list args;

    va_start(args, format);
    _VDPRINTF(DBC_DEBUG, __FILE__, __LINE__, format, args);
    va_end(args);
  }

  THREAD_UNLOCK;
}
#endif

/****************************************************************************/

#if defined(NO_VARARG_MARCOS)
void E(const unsigned long f, const char *format, ...)
{
  THREAD_LOCK;

  if(matchDebugSpec(DBC_ERROR, f, NULL, NULL) == TRUE)
  {
    va_list args;

    va_start(args, format);
    _VDPRINTF(DBC_ERROR, __FILE__, __LINE__, format, args);
    va_end(args);
  }

  THREAD_UNLOCK;
}
#endif

/****************************************************************************/

#if defined(NO_VARARG_MARCOS)
void W(const unsigned long f, const char *format, ...)
{
  THREAD_LOCK;

  if(matchDebugSpec(DBC_WARNING, f, NULL, NULL) == TRUE)
  {
    va_list args;

    va_start(args, format);
    _VDPRINTF(DBC_WARNING, f, __FILE__, __LINE__, format, args);
    va_end(args);
  }

  THREAD_UNLOCK;
}
#endif

/****************************************************************************/

struct DbgMallocNode
{
  struct MinNode node;
  void *memory;
  size_t size;
  const char *file;
  const char *func;
  int line;
};

static struct MinList DbgMallocList[256];
static struct SignalSemaphore DbgMallocListSema;
static ULONG DbgMallocCount;

// a very simple hashing function to spread the allocations across the lists
// Since AmigaOS memory allocation has a granularity of at least 8 bytes we simple ignore the
// lower 4 bits (=16 Bytes) and take the next 8 bits as hash value. Not very sophisticated, but
// it does the job quite good.
#define ptr2hash(p)           ((((ULONG)(p)) >> 4) & 0xff)

/// findDbgMallocNode
// find a given pointer in the tracking lists
static struct DbgMallocNode *findDbgMallocNode(const void *ptr)
{
  struct DbgMallocNode *result = NULL;
  struct Node *curNode;

  IterateList(&DbgMallocList[ptr2hash(ptr)], curNode)
  {
    struct DbgMallocNode *dmn = (struct DbgMallocNode *)curNode;

    if(dmn->memory == ptr)
    {
      result = dmn;
      break;
    }
  }

  return result;
}

///
/// _MEMTRACK
// add a new node to the memory tracking lists
void _MEMTRACK(const char *file, const int line, const char *func, void *ptr, size_t size)
{
  if(isFlagSet(debug_classes, DBC_MTRACK))
  {
    if(ptr != NULL && size != 0)
    {
      struct DbgMallocNode *dmn;

      if((dmn = malloc(sizeof(*dmn))) != NULL)
      {
        dmn->memory = ptr;
        dmn->size = size;
        dmn->file = file;
        dmn->line = line;
        dmn->func = func;

        ObtainSemaphore(&DbgMallocListSema);

        AddTail((struct List *)&DbgMallocList[ptr2hash(ptr)], (struct Node *)&dmn->node);
        DbgMallocCount++;

        ReleaseSemaphore(&DbgMallocListSema);
      }
    }
    else
      _DPRINTF(DBC_WARNING, DBF_ALWAYS, NULL, file, line, "potential invalid %s call with return (0x%08lx, 0x%08lx)", func, ptr, size);
  }
}

///
/// _UNMEMTRACK
// remove a node from the memory tracking lists
void _UNMEMTRACK(const char *file, const int line, const void *ptr)
{
  if(isFlagSet(debug_classes, DBC_MTRACK) && ptr != NULL)
  {
    BOOL success = FALSE;
    struct DbgMallocNode *dmn;

    ObtainSemaphore(&DbgMallocListSema);

    if((dmn = findDbgMallocNode(ptr)) != NULL)
    {
      Remove((struct Node *)dmn);

      free(dmn);

      DbgMallocCount--;

      success = TRUE;
    }

    if(success == FALSE)
      _DPRINTF(DBC_WARNING, DBF_ALWAYS, NULL, file, line, "free of untracked memory area 0x%08lx attempted", ptr);

    ReleaseSemaphore(&DbgMallocListSema);
  }
}

///
/// _FLUSH
// Flush any pending stdout or file debug output
void _FLUSH(void)
{
  THREAD_LOCK;

  if(stdout_output == TRUE)
    fflush(stdout);
  else if(file_output != NULL)
    fflush(file_output);

  THREAD_UNLOCK;
}

///
/// SetupDbgMalloc
// initialize the memory tracking framework
static void SetupDbgMalloc(void)
{
  ENTER();

  if(isFlagSet(debug_classes, DBC_MTRACK))
  {
    ULONG i;

    for(i = 0; i < ARRAY_SIZE(DbgMallocList); i++)
      NewMinList(&DbgMallocList[i]);

    DbgMallocCount = 0;

    // the semaphore structure must be cleared before InitSemaphore()
    memset(&DbgMallocListSema, 0, sizeof(DbgMallocListSema));
    InitSemaphore(&DbgMallocListSema);
  }

  LEAVE();
}

///
/// CleanupDbgMalloc
// cleanup the memory tracking framework and output possibly pending allocations
static void CleanupDbgMalloc(void)
{
  ENTER();

  if(isFlagSet(debug_classes, DBC_MTRACK))
  {
    _DBPRINTF("** Cleaning up memory tracking *************************************\n");

    ObtainSemaphore(&DbgMallocListSema);

    if(DbgMallocCount != 0)
    {
      ULONG i;

      E(DBF_ALWAYS, "there are still %ld unfreed memory trackings", DbgMallocCount);
      for(i = 0; i < ARRAY_SIZE(DbgMallocList); i++)
      {
        struct DbgMallocNode *dmn;

        while((dmn = (struct DbgMallocNode *)RemHead((struct List *)&DbgMallocList[i])) != NULL)
        {
          _DPRINTF(DBC_ERROR, DBF_ALWAYS, NULL, dmn->file, dmn->line, "unfreed memory tracking: 0x%08lx, size/type %ld, func (%s)", dmn->memory, dmn->size, dmn->func);

          // We only free the node structure here but not dmn->memory itself.
          // First of all, this is because the allocation could have been done
          // by other functions than malloc() and calling free() for these will
          // cause havoc. And second the c-library's startup code will/should
          // free all further pending allocations upon program termination.
          free(dmn);
        }
      }
    }
    else
      D(DBF_ALWAYS, "all memory trackings have been free()'d correctly");

    ReleaseSemaphore(&DbgMallocListSema);
  }

  LEAVE();
}

///
/// DumpDbgMalloc
// output all current allocations
void DumpDbgMalloc(void)
{
  ENTER();

  if(isFlagSet(debug_classes, DBC_MTRACK))
  {
    ULONG i;

    ObtainSemaphore(&DbgMallocListSema);

    D(DBF_ALWAYS, "%ld memory areas tracked", DbgMallocCount);
    for(i = 0; i < ARRAY_SIZE(DbgMallocList); i++)
    {
      struct Node *curNode;

      IterateList(&DbgMallocList[i], curNode)
      {
        struct DbgMallocNode *dmn = (struct DbgMallocNode *)curNode;

        _DPRINTF(DBC_MTRACK, DBF_ALWAYS, NULL, dmn->file, dmn->line, "memarea 0x%08lx, size/type %ld, func (%s)", dmn->memory, dmn->size, dmn->func);
      }
    }

    ReleaseSemaphore(&DbgMallocListSema);
  }

  LEAVE();
}

///

#endif /* DEBUG */
