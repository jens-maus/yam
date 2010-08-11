#ifndef EXTRASRC_H
#define EXTRASRC_H

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

 YAM Official Support Site :  http://www.yam.ch
 YAM OpenSource project    :  http://sourceforge.net/projects/yamos/

 $Id$

***************************************************************************/

// needed for size_t definition
#include <sys/types.h>
#include <stddef.h>
#include <stdio.h>

#include <intuition/classusr.h> // Object

#include "SDI_compiler.h"
#include "SDI_stdarg.h"

// forward declarations
struct TagItem;
struct IClass;
struct MinList;

/*
 * Stuff that exists on AmigaOS3
 */
#if !defined(__amigaos4__) && !defined(__MORPHOS__) && !defined(__AROS__)
#define HAVE_ASPRINTF
#define HAVE_VASPRINTF
#define HAVE_STRTOK_R
#define HAVE_STRDUP
#define HAVE_STRLCAT
#define HAVE_STRLCPY
#endif // !__amigaos4__ && !__MORPHOS__ && !__AROS__

/*
 * Stuff that exists on AmigaOS4
 */
#if defined(__amigaos4__)
#define HAVE_SETPROCWINDOW
#define HAVE_EXAMINEDIR
#define HAVE_ALLOCSYSOBJECT
#define HAVE_CHANGEFILEPOSITION
#define HAVE_GETHEAD
#define HAVE_GETPRED
#define HAVE_GETSUCC
#define HAVE_GETTAIL
#define HAVE_NEWMINLIST
#define HAVE_ALLOCVECPOOLED
#define HAVE_FREEVECPOOLED
#define HAVE_ASPRINTF
#define HAVE_VASPRINTF
#define HAVE_STRTOK_R
#define HAVE_STRDUP
#define HAVE_STRLCAT
#define HAVE_STRLCPY
#endif // __amigaos4__

/*
 * Stuff that exists on MorphOS
 */
#if defined(__MORPHOS__)
#define HAVE_DOSUPERNEW
#define HAVE_NEWMINLIST
#define HAVE_ALLOCVECPOOLED
#define HAVE_FREEVECPOOLED
#define HAVE_STCGFE
#define HAVE_STRTOK_R
#endif // __MORPHOS__

/*
 * Stuff that exists on AROS
 */
#if defined(__AROS__)
#define HAVE_GETHEAD
#define HAVE_GETPRED
#define HAVE_GETSUCC
#define HAVE_GETTAIL
#define HAVE_NEWMINLIST
#define HAVE_ALLOCVECPOOLED
#define HAVE_FREEVECPOOLED
#endif // __AROS__

/*
 * Differentations between compilers
 */
#if defined(__GNUC__)
#define HAVE_XGET
#define HAVE_XSET
#endif // __GNUC__

/*
 * Stuff we always require on all systems
 */
#undef HAVE_NEWREADARGS
#undef HAVE_MEMDUP
#undef HAVE_GETDELIM


/*
 * Function prototypes
 */
#if !defined(HAVE_STRLCPY)
size_t strlcpy(char *, const char *, size_t);
#endif

#if !defined(HAVE_STRLCAT)
size_t strlcat(char *, const char *, size_t);
#endif

#if !defined(HAVE_STRTOK_R)
char *strtok_r(char *, const char *, char **);
#endif

#if !defined(HAVE_VSNPRINTF)
int vsnprintf(char *buffer, size_t maxlen, const char *fmt, VA_LIST args);
#endif

#if !defined(HAVE_SNPRINTF)
int VARARGS68K snprintf(char *buffer, size_t maxlen, const char *fmt, ...);
#endif

#if !defined(HAVE_VASPRINTF)
int vasprintf(char **ptr, const char *format, VA_LIST ap);
#endif

#if !defined(HAVE_ASPRINTF)
int VARARGS68K asprintf(char **ptr, const char *format, ...);
#endif

#if !defined(HAVE_STCGFE)
int stcgfe(char *, const char *);
#endif

#if !defined(HAVE_STRDUP)
char *strdup(const char *);
#endif

#if !defined(HAVE_MEMDUP)
void *memdup(const void *source, const size_t size);
#endif

#if !defined(HAVE_GETDELIM)
#define getline(p, n, s) getdelim((p), (n), '\n', (s))
ssize_t getdelim(char **lineptr, size_t *n, int delim, FILE *stream);
#endif

#if !defined(HAVE_XGET)
IPTR xget(Object *obj, const IPTR attr);
#endif

#if !defined(HAVE_XSET)
ULONG VARARGS68K xset(Object *obj, ...);
#endif

#if !defined(HAVE_DOSUPERNEW)
Object * VARARGS68K DoSuperNew(struct IClass *cl, Object *obj, ...);
#endif

#if !defined(HAVE_SETPROCWINDOW)
APTR SetProcWindow(const void *newWindowPtr);
#endif

#if !defined(HAVE_EXAMINEDIR)
APTR ObtainDirContext(struct TagItem *tags);
#if defined(__PPC__)
#define ObtainDirContextTags(...) ({ULONG _tags[] = { __VA_ARGS__ }; ObtainDirContext((struct TagItem *)_tags);})
#else
APTR VARARGS68K ObtainDirContextTags(ULONG tag1, ...);
#endif
void ReleaseDirContext(APTR context);
struct ExamineData *ExamineDir(APTR context);
#include "extrasrc/ExamineDir.h"
#endif

#if !defined(HAVE_ALLOCSYSOBJECT)
APTR AllocSysObject(ULONG type, struct TagItem *tags);
#if defined(__PPC__)
#define AllocSysObjectTags(type, ...) ({ULONG _tags[] = { __VA_ARGS__ }; AllocSysObject(type, (struct TagItem *)_tags);})
#else
APTR VARARGS68K AllocSysObjectTags(ULONG type, ...);
#endif
void FreeSysObject(ULONG type, APTR object);
#include "extrasrc/AllocSysObject.h"
#endif

#if !defined(HAVE_GETHEAD)
struct Node *GetHead(struct List *list);
#endif

#if !defined(HAVE_GETPRED)
struct Node *GetPred(struct Node *node);
#endif

#if !defined(HAVE_GETSUCC)
struct Node *GetSucc(struct Node *node);
#endif

#if !defined(HAVE_GETTAIL)
struct Node *GetTail(struct List *list);
#endif

#if !defined(HAVE_NEWMINLIST)
// the NDK3.9 includes have this function declared already, but
// we want to be able to run on pre-3.9 systems, thus we have to
// use our own implementation
#undef NewMinList
void NewMinList(struct MinList *list);
#endif

#if !defined(HAVE_ALLOCVECPOOLED)
APTR AllocVecPooled(APTR poolHeader, ULONG memSize);
#endif

#if !defined(HAVE_FREEVECPOOLED)
void FreeVecPooled(APTR poolHeader, APTR memory);
#endif

#if !defined(HAVE_CHANGEFILEPOSITION)
#define ChangeFilePosition(fh, pos, mode)   Seek(fh, pos, mode)
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
