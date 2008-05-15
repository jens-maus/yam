#ifndef EXTRASRC_H
#define EXTRASRC_H

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

/*
 * Differentations between runtime libs and operating system
 */
#if (defined(__mc68000) && (defined(__ixemul) || defined(__libnix))) || defined(__SASC)

#if !defined(HAVE_STRLCPY)
#define NEED_STRLCPY
#endif

#if !defined(HAVE_STRLCAT)
#define NEED_STRLCAT
#endif

#if !defined(HAVE_STRTOK_R)
#define NEED_STRTOK_R
#endif

#if !defined(HAVE_VSNPRINTF)
#define NEED_VSNPRINTF
#endif

#if !defined(HAVE_SNPRINTF)
#define NEED_SNPRINTF
#endif

#endif /* (m68k && !clib2) || __SASC */

#if defined(__libnix) || defined(__SASC)

#if !defined(HAVE_VASPRINTF)
#define NEED_VASPRINTF
#endif

#if !defined(HAVE_ASPRINTF)
#define NEED_ASPRINTF
#endif

#endif /* libnix || __SASC */

#if !defined(__MORPHOS__) || !defined(__libnix)

#if !defined(HAVE_STCGFE)
#define NEED_STCGFE
#endif

#if !defined(HAVE_DOSUPERNEW)
#define NEED_DOSUPERNEW
#endif

#endif /* !__MORPHOS__ || !__libnix */

/*
 * Differentations between compilers
 */
#if defined(__VBCC__)

#if !defined(HAVE_STRDUP)
#define NEED_STRDUP
#endif

#if !defined(HAVE_STRTOK_R)
#define NEED_STRTOK_R
#endif

#endif /* __VBCC__ */

#if !defined(__SASC)

#if !defined(HAVE_VASTUBS)
#define NEED_VASTUBS
#endif

#endif /* !__SASC */


#if !defined(__GNUC__)

#if !defined(HAVE_XGET)
#define NEED_XGET
#endif

#if !defined(HAVE_XSET)
#define NEED_XSET
#endif

#endif /* !__GNUC__ */

/*
 * Stuff we always require
 */
#if !defined(HAVE_NEWREADARGS)
#define NEED_NEWREADARGS
#endif

#if !defined(HAVE_MEMDUP)
#define NEED_MEMDUP
#endif

/*
 * Stuff that exists in OS4 already but not in OS3 or MOS
 */
#if defined(__amigaos4__)
#define HAVE_SETPROCWINDOW
#define HAVE_EXAMINEDIR
#define HAVE_ALLOCSYSOBJECT
#endif

#if !defined(HAVE_SETPROCWINDOW)
#define NEED_SETPROCWINDOW
#endif

#if !defined(HAVE_EXAMINEDIR)
#define NEED_EXAMINEDIR
#endif

#if !defined(HAVE_ALLOCSYSOBJECT)
#define NEED_ALLOCSYSOBJECT
#endif

/*
 * Stuff that exists in OS4 and MOS already, but not in OS3
 */
#if defined(__amigaos4__) || defined(__MORPHOS__)
#define HAVE_ALLOCVECPOOLED
#define HAVE_FREEVECPOOLED
#endif

#if !defined(HAVE_ALLOCVECPOOLED)
#define NEED_ALLOCVECPOOLED
#endif

#if !defined(HAVE_FREEVECPOOLED)
#define NEED_FREEVECPOOLED
#endif

#if defined(__MORPHOS__) && !defined(HAVE_URLOPEN)
#define NEED_URLOPEN
#endif

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
int VARARGS68K snprintf(char *buffer, size_t maxlen, const char *fmt, ...);
#endif

#if defined(NEED_VASPRINTF)
int vasprintf(char **ptr, const char *format, VA_LIST ap);
#endif

#if defined(NEED_ASPRINTF)
int VARARGS68K asprintf(char **ptr, const char *format, ...);
#endif

#if defined(NEED_STCGFE)
int stcgfe(char *, const char *);
#endif

#if defined(NEED_STRDUP)
char *strdup(const char *);
#endif

#if defined(NEED_XGET)
ULONG xget(Object *obj, const ULONG attr);
#endif

#if defined(NEED_XSET)
ULONG VARARGS68K xset(Object *obj, ...);
#endif

#if defined(NEED_DOSUPERNEW)
Object * VARARGS68K DoSuperNew(struct IClass *cl, Object *obj, ...);
#endif

#if defined(NEED_MEMDUP)
void *memdup(const void *source, const size_t size);
#endif

#if defined(NEED_SETPROCWINDOW)
APTR SetProcWindow(const void *newWindowPtr);
#endif

#if defined(NEED_EXAMINEDIR)
APTR ObtainDirContext(struct TagItem *tags);
#if defined(PPC)
#define ObtainDirContextTags(...) \
    ({ULONG _tags[] = { __VA_ARGS__ }; \
    ObtainDirContext((struct TagItem *)_tags);})
#else
APTR VARARGS68K ObtainDirContextTags(ULONG tag1, ...);
#endif
void ReleaseDirContext(APTR context);
struct ExamineData *ExamineDir(APTR context);
#include "extrasrc/ExamineDir.h"
#endif

#if defined(NEED_ALLOCSYSOBJECT)
APTR AllocSysObject(ULONG type, struct TagItem *tags);
#if defined(PPC)
#define AllocSysObjectTags(type, ...) \
    ({ULONG _tags[] = { __VA_ARGS__ }; \
    AllocSysObject(type, (struct TagItem *)_tags);})
#else
APTR VARARGS68K AllocSysObjectTags(ULONG type, ...);
#endif
void FreeSysObject(ULONG type, APTR object);
#include "extrasrc/AllocSysObject.h"
#endif

#if defined(NEED_ALLOCVECPOOLED)
APTR AllocVecPooled(APTR poolHeader, ULONG memSize);
#endif

#if defined(NEED_FREEVECPOOLED)
void FreeVecPooled(APTR poolHeader, APTR memory);
#endif

#if defined(NEED_URLOPEN)
#define URL_Open(__p0, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	URL_OpenA(__p0, (struct TagItem *)_tags);})
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
