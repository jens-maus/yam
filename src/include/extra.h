#ifndef YAM_EXTRA_H
#define YAM_EXTRA_H

// do a "fake" proto/socket.h define here
// because we assure that bsdsocket.h is used
// only and doesn`t conflict with other socket
// definitions.
#ifndef PROTO_SOCKET_H
#define PROTO_SOCKET_H 1
#endif

/*
** size-bounded string copying and concatenation
*/

#if (defined(__mc68000) && (defined(__ixemul) || defined(__libnix))) || defined(__SASC)

#if !defined(HAVE_STRLCPY)
#define NEED_STRLCPY
#endif /* HAVE_STRLCPY */

#if !defined(HAVE_STRLCAT)
#define NEED_STRLCAT
#endif /* HAVE_STRLCAT */

#if !defined(HAVE_STRTOK_R)
#define NEED_STRTOK_R
#endif /* HAVE_STRTOK_R */

#endif /* m68k && !clib2 || __SASC */

#if defined(NEED_STRLCPY)
size_t strlcpy(char *, const char *, size_t);
#endif /* NEED_STRLCPY */

#if defined(NEED_STRLCAT)
size_t strlcat(char *, const char *, size_t);
#endif /* NEED_STRLCAT */

#if defined(NEED_STRTOK_R)
char *strtok_r(char *, const char *, char **);
#endif /* NEED_STRTOK_R */

#ifdef __SASC

#include <dos.h>

#else

/*
** <string.h>
*/

extern int stcgfe(char *, const char *);
extern void strmfp(char *, const char *, const char *);
extern void strsfn(const char *, char *, char *, char *, char *);

#if defined(__VBCC__)
  extern char *strdup(const char *);
  extern char *strtok_r(char *str, const char *separator_set,char ** state_ptr);
  #define isascii(c) (((c) & ~0177) == 0)
  #define stricmp(s1, s2) strcasecmp((s1), (s2))
  #define strnicmp(s1, s2, len) strncasecmp((s1), (s2), (len))
#endif

/*
** <dos.h>
*/

#define FNSIZE 108
#define FMSIZE 256
#define FESIZE 32

#endif /* __SASC */

#endif /* YAM_EXTRA_H */
