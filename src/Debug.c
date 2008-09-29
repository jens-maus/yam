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

#ifdef DEBUG

#include <stdio.h> // vsnprintf
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/timer.h>
#include <clib/alib_protos.h>

#include "YAM_global.h"
#include "YAM_utilities.h" // CLEAR_FLAG,SET_FLAG
#include "extrasrc.h"

#include "SDI_compiler.h"

#define DEBUG_USE_MALLOC_REDEFINE 1
#include "Debug.h"

#if defined(__MORPHOS__)
#include <exec/rawfmt.h>
#else
#include <clib/debug_protos.h>
#endif

// our static variables with default values
static int indent_level = 0;
static BOOL ansi_output = FALSE;
static BOOL con_output = FALSE;
static FILE *file_output = NULL;
static ULONG debug_flags = DBF_ALWAYS | DBF_STARTUP; // default debug flags
static ULONG debug_classes = DBC_ERROR | DBC_DEBUG | DBC_WARNING | DBC_ASSERT | DBC_REPORT; // default debug classes
static int timer_level = -1;
static struct TimeVal startTimes[8];

static void SetupDbgMalloc(void);
static void CleanupDbgMalloc(void);

/****************************************************************************/

void _DBPRINTF(const char *format, ...)
{
  va_list args;

  va_start(args, format);

  if(con_output == TRUE)
    vprintf(format, args);
  else if(file_output != NULL)
    vfprintf(file_output, format, args);
  else
  {
    #if defined(__MORPHOS__)
    VNewRawDoFmt(format, (APTR)RAWFMTFUNC_SERIAL, NULL, args);
    #elif defined(__amigaos4__)
    static char buf[1024];
    vsnprintf(buf, 1024, format, args);
    DebugPrintF("%s", buf);
    #else
    KPutFmt(format, args);
    #endif
  }

  va_end(args);
}

/****************************************************************************/

void SetupDebug(void)
{
  char var[256];

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
      { "memory",   DBF_MEMORY  },
      { "all",      DBF_ALL     },
      { NULL,       0           }
    };

    // before we search for our debug tokens we search
    // for our special output tokens
    if(stristr(s, "ansi") != NULL)
      ansi_output = TRUE;

    if(stristr(s, "con") != NULL)
      con_output = TRUE;

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
    _DBPRINTF("** %s build: %s startup **********************\n", yamversion, yambuildid);
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
          else if(strnicmp(s, "con", 3) == 0)
          {
            con_output = TRUE;
            _DBPRINTF("CON: output enabled\n");
          }
          else if(strnicmp(s, "file:", 5) == 0)
          {
            char *e;
            char *t = s+5;
            char filename[256];

            // the user wants to output the debugging information
            // to a file instead
            if((e = strpbrk(t, " ,;")) == NULL)
              strlcpy(filename, t, sizeof(filename));
            else
              strlcpy(filename, t, e-t+1);

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

/****************************************************************************/

INLINE void _INDENT(void)
{
  int i;
  for(i=0; i < indent_level; i++)
    _DBPRINTF(" ");
}

/****************************************************************************/

void _ENTER(unsigned long dclass, const char *file, unsigned long line, const char *function)
{
  if(isFlagSet(debug_classes, dclass))
  {
    _INDENT();
    if(ansi_output)
      _DBPRINTF("%s%s:%ld:Entering %s%s\n", ANSI_ESC_FG_BROWN, file, line, function, ANSI_ESC_CLR);
    else
      _DBPRINTF("%s:%ld:Entering %s\n", file, line, function);
  }

  indent_level++;
}

void _LEAVE(unsigned long dclass, const char *file, unsigned long line, const char *function)
{
  indent_level--;

  if(isFlagSet(debug_classes, dclass))
  {
    _INDENT();
    if(ansi_output)
      _DBPRINTF("%s%s:%ld:Leaving %s%s\n", ANSI_ESC_FG_BROWN, file, line, function, ANSI_ESC_CLR);
    else
      _DBPRINTF("%s:%ld:Leaving %s\n", file, line, function);
  }
}

void _RETURN(unsigned long dclass, const char *file, unsigned long line, const char *function, unsigned long result)
{
  indent_level--;

  if(isFlagSet(debug_classes, dclass))
  {
    _INDENT();
    if(ansi_output)
      _DBPRINTF("%s%s:%ld:Leaving %s (result 0x%08lx, %ld)%s\n", ANSI_ESC_FG_BROWN, file, line, function, result, result, ANSI_ESC_CLR);
    else
      _DBPRINTF("%s:%ld:Leaving %s (result 0x%08lx, %ld)\n", file, line, function, result, result);
  }
}

/****************************************************************************/

void _SHOWVALUE(unsigned long dclass, unsigned long dflags, unsigned long value, int size, const char *name, const char *file, unsigned long line)
{
  if(isFlagSet(debug_classes, dclass) &&
     isFlagSet(debug_flags, dflags))
  {
    const char *fmt;

    switch(size)
    {
      case 1:
        fmt = "%s:%ld:%s = %ld, 0x%02lx";
      break;

      case 2:
        fmt = "%s:%ld:%s = %ld, 0x%04lx";
      break;

      default:
        fmt = "%s:%ld:%s = %ld, 0x%08lx";
      break;
    }

    _INDENT();

    if(ansi_output)
      _DBPRINTF(ANSI_ESC_FG_GREEN);

    _DBPRINTF(fmt, file, line, name, value, value);

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
}

/****************************************************************************/

void _SHOWPOINTER(unsigned long dclass, unsigned long dflags, const void *p, const char *name, const char *file, unsigned long line)
{
  if(isFlagSet(debug_classes, dclass) &&
     isFlagSet(debug_flags, dflags))
  {
    const char *fmt;

    _INDENT();

    if(p != NULL)
      fmt = "%s:%ld:%s = 0x%08lx\n";
    else
      fmt = "%s:%ld:%s = NULL\n";

    if(ansi_output)
    {
      _DBPRINTF(ANSI_ESC_FG_GREEN);
      _DBPRINTF(fmt, file, line, name, p);
      _DBPRINTF(ANSI_ESC_CLR);
    }
    else
      _DBPRINTF(fmt, file, line, name, p);
  }
}

/****************************************************************************/

void _SHOWSTRING(unsigned long dclass, unsigned long dflags, const char *string, const char *name, const char *file, unsigned long line)
{
  if(isFlagSet(debug_classes, dclass) &&
     isFlagSet(debug_flags, dflags))
  {
    _INDENT();

    if(ansi_output)
      _DBPRINTF("%s%s:%ld:%s = 0x%08lx \"%s\"%s\n", ANSI_ESC_FG_GREEN, file, line, name, (unsigned long)string, string, ANSI_ESC_CLR);
    else
      _DBPRINTF("%s:%ld:%s = 0x%08lx \"%s\"\n", file, line, name, (unsigned long)string, string);
  }
}

/****************************************************************************/

void _SHOWMSG(unsigned long dclass, unsigned long dflags, const char *msg, const char *file, unsigned long line)
{
  if(isFlagSet(debug_classes, dclass) &&
     isFlagSet(debug_flags, dflags))
  {
    _INDENT();

    if(ansi_output)
      _DBPRINTF("%s%s:%ld:%s%s\n", ANSI_ESC_FG_GREEN, file, line, msg, ANSI_ESC_CLR);
    else
      _DBPRINTF("%s:%ld:%s\n", file, line, msg);
  }
}

/****************************************************************************/

void _DPRINTF(unsigned long dclass, unsigned long dflags, const char *file, unsigned long line, const char *format, ...)
{
  if((isFlagSet(debug_classes, dclass) && isFlagSet(debug_flags, dflags)) ||
     (isFlagSet(dclass, DBC_ERROR) || isFlagSet(dclass, DBC_WARNING)))
  {
    va_list args;

    va_start(args, format);
    _VDPRINTF(dclass, dflags, file, line, format, args);
    va_end(args);
  }
}

/****************************************************************************/

void _VDPRINTF(unsigned long dclass, unsigned long dflags, const char *file, unsigned long line, const char *format, va_list args)
{
  if((isFlagSet(debug_classes, dclass) && isFlagSet(debug_flags, dflags)) ||
     (isFlagSet(dclass, DBC_ERROR) || isFlagSet(dclass, DBC_WARNING)))
  {
    static char buf[1024];

    _INDENT();

    vsnprintf(buf, 1024, format, args);

    if(ansi_output)
    {
      const char *highlight = ANSI_ESC_FG_GREEN;

      switch(dclass)
      {
        case DBC_CTRACE:  highlight = ANSI_ESC_FG_BROWN; break;
        case DBC_REPORT:  highlight = ANSI_ESC_FG_GREEN; break;
        case DBC_ASSERT:  highlight = ANSI_ESC_FG_RED;   break;
        case DBC_TIMEVAL: highlight = ANSI_ESC_FG_GREEN; break;
        case DBC_DEBUG:   highlight = ANSI_ESC_FG_GREEN; break;
        case DBC_ERROR:   highlight = ANSI_ESC_FG_RED;   break;
        case DBC_WARNING: highlight = ANSI_ESC_FG_PURPLE;break;
      }

      _DBPRINTF("%s%s:%ld:%s%s\n", highlight, file, line, buf, ANSI_ESC_CLR);
    }
    else
      _DBPRINTF("%s:%ld:%s\n", file, line, buf);
  }
}

/****************************************************************************/

void _STARTCLOCK(const char *file, unsigned long line)
{
  if(timer_level + 1 < (int)ARRAY_SIZE(startTimes))
  {
    timer_level++;
    GetSysTime(TIMEVAL(&startTimes[timer_level]));
  }
  else
    _DPRINTF(DBC_ERROR, DBF_ALWAYS, file, line, "already %ld clocks in use!", ARRAY_SIZE(startTimes));
}

/****************************************************************************/

void _STOPCLOCK(unsigned long dflags, const char *message, const char *file, unsigned long line)
{
  if(timer_level >= 0)
  {
    struct TimeVal stopTime;

    GetSysTime(TIMEVAL(&stopTime));
    SubTime(TIMEVAL(&stopTime), TIMEVAL(&startTimes[timer_level]));
    _DPRINTF(DBC_TIMEVAL, dflags, file, line, "operation '%s' took %ld.%09ld seconds", message, stopTime.Seconds, stopTime.Microseconds);
    timer_level--;
  }
  else
    _DPRINTF(DBC_ERROR, DBF_ALWAYS, file, line, "no clocks in use!");
}

/****************************************************************************/

#if defined(NO_VARARG_MARCOS)
void D(unsigned long f, const char *format, ...)
{
  va_list args;

  va_start(args, format);
  _VDPRINTF(DBC_DEBUG, f, __FILE__, __LINE__, format, args);
  va_end(args);
}
#endif

/****************************************************************************/

#if defined(NO_VARARG_MARCOS)
void E(unsigned long f, const char *format, ...)
{
  va_list args;

  va_start(args, format);
  _VDPRINTF(DBC_ERROR, f, __FILE__, __LINE__, format, args);
  va_end(args);
}
#endif

/****************************************************************************/

#if defined(NO_VARARG_MARCOS)
void W(unsigned long f, const char *format, ...)
{
  va_list args;

  va_start(args, format);
  _VDPRINTF(DBC_WARNING, f, __FILE__, __LINE__, format, args);
  va_end(args);
}
#endif

/****************************************************************************/

struct DbgMallocNode {
  struct MinNode node;
  void *memory;
  size_t size;
  char *file;
  int line;
};

static struct MinList DbgMallocList;
static struct SignalSemaphore DbgMallocListSema;
static ULONG DbgMallocCount;

#define ForEachNode    for(dmn = (struct DbgMallocNode *)DbgMallocList.mlh_Head; dmn->node.mln_Succ != NULL; dmn = (struct DbgMallocNode *)dmn->node.mln_Succ)

static struct DbgMallocNode *addDbgMallocNode(const char *file, const int line, void *ptr, size_t size)
{
  struct DbgMallocNode *dmn;

  ENTER();

  if((dmn = malloc(sizeof(*dmn))) != NULL)
  {
    if((dmn->file = strdup(file)) != NULL)
    {
      dmn->memory = ptr;
      dmn->size = size;
      dmn->line = line;

      ObtainSemaphore(&DbgMallocListSema);
      AddTail((struct List *)&DbgMallocList, (struct Node *)&dmn->node);
      ReleaseSemaphore(&DbgMallocListSema);

      DbgMallocCount++;
    }
  }

  RETURN(dmn);
  return dmn;
}

static struct DbgMallocNode *findDbgMallocNode(const void *ptr)
{
  struct DbgMallocNode *result = NULL;

  ENTER();

  if(IsListEmpty((struct List *)&DbgMallocList) == FALSE)
  {
    struct DbgMallocNode *dmn;

    ForEachNode
    {
      if(dmn->memory == ptr)
      {
        result = dmn;
        break;
      }
    }
  }

  RETURN(result);
  return result;
}

void *dbg_malloc(const char *file, const int line, size_t size)
{
  void *result = NULL;

  ENTER();

  result = malloc(size);

  if(isFlagSet(debug_flags, DBF_MEMORY) && result != NULL)
  {
    addDbgMallocNode(file, line, result, size);
  }

  RETURN(result);
  return result;
}

void *dbg_calloc(const char *file, const int line, size_t n, size_t size)
{
  void *result;

  ENTER();

  result = calloc(n, size);

  if(isFlagSet(debug_flags, DBF_MEMORY) && result != NULL)
  {
    addDbgMallocNode(file, line, result, size);
  }

  RETURN(result);
  return result;
}

void *dbg_realloc(UNUSED const char *file, UNUSED const int line, void *ptr, size_t size)
{
  void *result = NULL;

  ENTER();

  if(isFlagSet(debug_flags, DBF_MEMORY))
  {
    if(ptr == NULL)
    {
      result = dbg_malloc(file, line, size);
    }
    else if(size == 0)
    {
      dbg_free(file, line, ptr);
    }
    else
    {
      struct DbgMallocNode *dmn;

      ObtainSemaphore(&DbgMallocListSema);

      if((dmn = findDbgMallocNode(ptr)) != NULL)
      {
        if((result = realloc(ptr, size)) != NULL)
        {
          free(dmn->file);

          dmn->memory = result;
          dmn->size = size;
          dmn->file = strdup(file);
          dmn->line = line;
        }
      }

      ReleaseSemaphore(&DbgMallocListSema);
    }
  }
  else
  {
    result = realloc(ptr, size);
  }

  RETURN(result);
  return result;
}

void dbg_free(const char *file, const int line, void *ptr)
{
  ENTER();

  if(isFlagSet(debug_flags, DBF_MEMORY))
  {
    BOOL success = FALSE;
    struct DbgMallocNode *dmn;

    ObtainSemaphore(&DbgMallocListSema);

    if((dmn = findDbgMallocNode(ptr)) != NULL)
    {
      Remove((struct Node *)dmn);
      if(dmn->file != NULL)
        free(dmn->file);
      free(dmn);

      DbgMallocCount--;

      success = TRUE;
    }

    ReleaseSemaphore(&DbgMallocListSema);

    if(success == FALSE)
      W(DBF_MEMORY, "%s:%ld: free() of untracked allocation 0x%08lx attempted", file, line, ptr);
  }

  free(ptr);

  LEAVE();
}

char *dbg_strdup(const char *file, const int line, const char *s)
{
  char *result;

  ENTER();

  if((result = dbg_malloc(file, line, strlen(s)+1)) != NULL)
    strcpy(result, s);

  RETURN(result);
  return result;
}

void *dbg_memdup(const char *file, const int line, const void *ptr, const size_t size)
{
  void *result;

  ENTER();

  if((result = dbg_malloc(file, line, size)) != NULL)
    memcpy(result, ptr, size);

  RETURN(result);
  return result;
}

static void SetupDbgMalloc(void)
{
  ENTER();

  if(isFlagSet(debug_flags, DBF_MEMORY))
  {
    NewList((struct List *)&DbgMallocList);
    InitSemaphore(&DbgMallocListSema);
    DbgMallocCount = 0;
  }

  LEAVE();
}

static void CleanupDbgMalloc(void)
{
  ENTER();

  if(isFlagSet(debug_flags, DBF_MEMORY))
  {
    ObtainSemaphore(&DbgMallocListSema);

    if(IsListEmpty((struct List *)&DbgMallocList) == FALSE)
    {
      struct DbgMallocNode *dmn;

      E(DBF_MEMORY, "there are still unfreed %ld allocations", DbgMallocCount);
      while((dmn = (struct DbgMallocNode *)RemHead((struct List *)&DbgMallocList)) != NULL)
      {
        E(DBF_MEMORY, "unfreed allocation 0x%08lx, size %ld from file '%s', line %ld", dmn->memory, dmn->size, dmn->file, dmn->line);
        if(dmn->memory != NULL)
          free(dmn->memory);
        if(dmn->file != NULL)
          free(dmn->file);
        free(dmn);
      }
    }
    else
    {
      D(DBF_MEMORY, "all memory allocations have been free()'d correctly");
    }

    ReleaseSemaphore(&DbgMallocListSema);
  }

  LEAVE();
}

void DumpDbgMalloc(void)
{
  ENTER();

  if(isFlagSet(debug_flags, DBF_MEMORY))
  {
    ObtainSemaphore(&DbgMallocListSema);

    if(IsListEmpty((struct List *)&DbgMallocList) == FALSE)
    {
      struct DbgMallocNode *dmn;

      D(DBF_MEMORY, "%ld allocations tracked", DbgMallocCount);
      ForEachNode
      {
        D(DBF_MEMORY, "allocation 0x%08lx, size %ld from file '%s', line %ld", dmn->memory, dmn->size, dmn->file, dmn->line);
      }
    }

    ReleaseSemaphore(&DbgMallocListSema);
  }

  LEAVE();
}

#endif /* DEBUG */
