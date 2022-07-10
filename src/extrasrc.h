#ifndef EXTRASRC_H
#define EXTRASRC_H

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

// needed for size_t definition
#include <sys/types.h>
#include <stddef.h>
#include <stdio.h>

#ifndef EXEC_TYPES_H
  #include <exec/types.h>         // ULONG
#endif
#ifndef INTUITION_CLASSUSR_H
  #include <intuition/classusr.h> // Object
#endif
#ifndef SDI_COMPILER_H
  #include "SDI_compiler.h"
#endif
#ifndef SDI_STDARG_H
  #include "SDI_stdarg.h"
#endif

// forward declarations
struct TagItem;
struct IClass;
struct MinList;
struct NewRDArgs;
struct WBStartup;
struct List;
struct Node;

/*
 * Stuff that exists on AmigaOS3
 */
#if defined(__amigaos3__)
#define NEED_ALLOCSYSOBJECT
#define NEED_ALLOCVECPOOLED
#define NEED_DOSUPERNEW
#define NEED_EXAMINEDIR
#define NEED_FREEVECPOOLED
#define NEED_GETHEAD
#define NEED_GETPRED
#define NEED_GETSUCC
#define NEED_GETTAIL
#define NEED_ITEMPOOLALLOC
#define NEED_ITEMPOOLFREE
#define NEED_MOVELIST
#define NEED_NEWMINLIST
#define NEED_NEWREADARGS
#define NEED_SETPROCWINDOW
#define NEED_GETDELIM
#define NEED_MEMDUP
#define NEED_STCGFE
#define NEED_VASTUBS
#define NEED_CHANGEFILEPOSITION
#define NEED_STRCASESTR
#define NEED_GLIBC_TM
#define NEED_TZLIB
#endif // __amigaos3__

/*
 * Stuff that exists on AmigaOS4
 */
#if defined(__amigaos4__)
#define NEED_DOSUPERNEW
#define NEED_NEWREADARGS
#define NEED_GETDELIM
#define NEED_MEMDUP
#define NEED_STCGFE
#define NEED_STRCASESTR
#define NEED_TZLIB
#endif // __amigaos4__

/*
 * Stuff that exists on MorphOS
 */
#if defined(__MORPHOS__)
#define NEED_ALLOCSYSOBJECT
#define NEED_EXAMINEDIR
#define NEED_GETHEAD
#define NEED_GETPRED
#define NEED_GETSUCC
#define NEED_GETTAIL
#define NEED_ITEMPOOLALLOC
#define NEED_ITEMPOOLFREE
#define NEED_MOVELIST
#define NEED_NEWREADARGS
#define NEED_SETPROCWINDOW
#define NEED_ASPRINTF
#define NEED_GETDELIM
#define NEED_MEMDUP
#define NEED_VASPRINTF
#define NEED_CHANGEFILEPOSITION
#define NEED_TZLIB
#endif // __MORPHOS__

/*
 * Stuff that exists on AROS
 */
#if defined(__AROS__)
#define NEED_ALLOCSYSOBJECT
#define NEED_DOSUPERNEW
#define NEED_EXAMINEDIR
#define NEED_ITEMPOOLALLOC
#define NEED_ITEMPOOLFREE
#define NEED_MOVELIST
#define NEED_NEWMINLIST
#define NEED_NEWREADARGS
#define NEED_SETPROCWINDOW
#define NEED_ASPRINTF
#define NEED_GETDELIM
#define NEED_MEMDUP
#define NEED_STCGFE
#define NEED_STRTOK_R
#define NEED_VASPRINTF
#define NEED_VASTUBS
#define NEED_CHANGEFILEPOSITION
#define NEED_TZLIB
#endif // __AROS__

/*
 * Differentations between compilers
 */
#if !defined(__GNUC__)
#define NEED_XGET
#define NEED_XSET
#endif // __GNUC__

/*
 * Function prototypes
 */
#if defined(NEED_STRLCPY)
size_t strlcpy(char *, const char *, size_t);
#endif

#if defined(NEED_STRLCAT)
size_t strlcat(char *, const char *, size_t);
#endif

#if defined(NEED_STRTOK_R)
char *strtok_r(char *, const char *, char **);
#endif

#if defined(NEED_VSNPRINTF)
int vsnprintf(char *buffer, size_t maxlen, const char *fmt, VA_LIST args);
#endif

#if defined(NEED_SNPRINTF)
int snprintf(char *buffer, size_t maxlen, const char *fmt, ...);
#endif

#if defined(NEED_VASPRINTF)
int vasprintf(char **ptr, const char *format, VA_LIST ap);
#endif

#if defined(NEED_ASPRINTF)
int asprintf(char **ptr, const char *format, ...);
#endif

#if defined(NEED_STCGFE)
int stcgfe(char *, const char *);
#endif

#if defined(NEED_STRDUP)
char *strdup(const char *);
#endif

#if defined(NEED_MEMDUP)
void *memdup(const void *source, const size_t size);
#endif

#if defined(NEED_GETDELIM)
#define getline(p, n, s) getdelim((p), (n), '\n', (s))
ssize_t getdelim(char **lineptr, size_t *n, int delim, FILE *stream);
#endif

#if defined(NEED_XGET)
IPTR xget(Object *obj, const IPTR attr);
#endif

#if defined(NEED_XSET)
ULONG VARARGS68K xset(Object *obj, ...);
#endif

#if defined(NEED_DOSUPERNEW)
#if defined(__GNUC__) && __GNUC__ == 2
Object * VARARGS68K DoSuperNew(struct IClass *cl, Object *obj, ...);
#elif defined(__AROS__)
#define DoSuperNew(cl, obj, ...) ({ULONG _tags[] = { SDI_VACAST(__VA_ARGS__) }; (Object *)DoSuperNewTagList(cl, obj, NULL, (struct TagItem *)_tags);})
#else
#define DoSuperNew(cl, obj, ...) ({ULONG _tags[] = { SDI_VACAST(__VA_ARGS__) }; (Object *)DoSuperMethod(cl, obj, OM_NEW, (struct TagItem *)_tags, NULL);})
#endif
#endif

#if defined(NEED_SETPROCWINDOW)
APTR SetProcWindow(const void *newWindowPtr);
#endif

#if defined(NEED_EXAMINEDIR)
APTR ObtainDirContext(struct TagItem *tags);
#define ObtainDirContextTags(...) ({ULONG _tags[] = { SDI_VACAST(__VA_ARGS__) }; ObtainDirContext((struct TagItem *)_tags);})
void ReleaseDirContext(APTR context);
struct ExamineData *ExamineDir(APTR context);
#include "extrasrc/ExamineDir.h"
#endif

#if defined(NEED_ALLOCSYSOBJECT)
APTR AllocSysObject(ULONG type, struct TagItem *tags);
#define AllocSysObjectTags(type, ...) ({ULONG _tags[] = { SDI_VACAST(__VA_ARGS__) }; AllocSysObject(type, (struct TagItem *)_tags);})
void FreeSysObject(ULONG type, APTR object);
#include "extrasrc/AllocSysObject.h"
#endif

#if defined(NEED_MOVELIST)
void MoveList(struct List *destList, struct List *sourceList);
#endif

#if defined(NEED_GETHEAD)
struct Node *GetHead(struct List *list);
#endif

#if defined(NEED_GETPRED)
struct Node *GetPred(struct Node *node);
#endif

#if defined(NEED_GETSUCC)
struct Node *GetSucc(struct Node *node);
#endif

#if defined(NEED_GETTAIL)
struct Node *GetTail(struct List *list);
#endif

#if defined(NEED_ITEMPOOLALLOC)
APTR ItemPoolAlloc(APTR poolHeader);
#endif

#if defined(NEED_ITEMPOOLFREE)
APTR ItemPoolFree(APTR poolHeader, APTR item);
#endif

#if defined(NEED_NEWMINLIST)
// the NDK3.9 includes have this function declared already, but
// we want to be able to run on pre-3.9 systems, thus we have to
// use our own implementation
#undef NewMinList
void NewMinList(struct MinList *list);
#endif

#if defined(NEED_ALLOCVECPOOLED)
APTR AllocVecPooled(APTR poolHeader, ULONG memSize);
#endif

#if defined(NEED_FREEVECPOOLED)
void FreeVecPooled(APTR poolHeader, APTR memory);
#endif

#if defined(NEED_CHANGEFILEPOSITION)
#define ChangeFilePosition(fh, pos, mode)   Seek(fh, pos, mode)
#endif

#if defined(NEED_NEWREADARGS)
LONG NewReadArgs(struct WBStartup *, struct NewRDArgs *);
void NewFreeArgs(struct NewRDArgs *);
#endif

#if defined(NEED_STRCASESTR)
char *strcasestr(const char *haystack, const char *needle);
#endif

#if defined(NEED_GLIBC_TM)
struct glibc_tm
{
  int  tm_sec;
  int  tm_min;
  int  tm_hour;
  int  tm_mday;
  int  tm_mon;
  int  tm_year;
  int  tm_wday;
  int  tm_yday;
  int  tm_isdst;
  long tm_gmtoff;
  const char *tm_zone;
};
#define TM glibc_tm
#else
#include <time.h>
#define TM tm
#endif

#if defined(NEED_TZLIB)
# undef  ctime
# define ctime tz_ctime
# undef  ctime_r
# define ctime_r tz_ctime_r
# undef  difftime
# define difftime tz_difftime
# undef  gmtime
# define gmtime tz_gmtime
# undef  gmtime_r
# define gmtime_r tz_gmtime_r
# undef  localtime
# define localtime tz_localtime
# undef  localtime_r
# define localtime_r tz_localtime_r
# undef  mktime
# define mktime tz_mktime
# undef  tzset
# define tzset tz_tzset
# undef  strftime
# define strftime tz_strftime
# undef  asctime
# define asctime tz_asctime
# undef  asctime_r
# define asctime_r tz_asctime_r

char *ctime(time_t const *);
char *ctime_r(time_t const *, char *);
double difftime(time_t, time_t);
struct TM *gmtime(time_t const *);
struct TM *gmtime_r(time_t const *, struct TM *);
struct TM *localtime(time_t const *);
struct TM *localtime_r(time_t const *, struct TM *);
time_t mktime(struct TM *);
void tzset(const char * name);
size_t strftime(char * const s, const size_t maxsize, const char *const format,
   const struct TM *const t);
char *asctime_r(register const struct TM *timeptr, char *buf);
char *asctime(register const struct TM *timeptr);
#endif

/*
 * Additional defines
 */
#if defined(__VBCC__)

  #define isascii(c) (((c) & ~0177) == 0)
  #define stricmp(s1, s2) strcasecmp((s1), (s2))
  #define strnicmp(s1, s2, len) strncasecmp((s1), (s2), (len))

#endif /* __VBCC__ */

#endif /* EXTRASRC_H */
