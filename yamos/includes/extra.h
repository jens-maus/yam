#ifndef YAM_EXTRA_H
#define YAM_EXTRA_H

#ifdef __SASC

#include <dos.h>
#include <error.h>

#else

#ifdef _DCC
   #include <fcntl.h>
   #define isascii(c) (((c)&0xff)<127)
#elif defined(__STORM__)
   #define F_OK 0
   #define isascii(c) (((c)&0xff)<127)
#else
   #include <unistd.h>
#endif

/*
** <string.h>
*/

extern int stccpy(char *, const char *, int);
extern int stcgfe(char *, const char *);
extern int stcgfn(char *, const char *);
extern int astcsma(const char *, const char *);
extern char *stpblk(const char *);
extern void strmfp(char *, const char *, const char *);
extern void strsfn(const char *, char *, char *, char *, char *);

/*
** <dos.h>
*/

#define FNSIZE 108
#define FMSIZE 256
#define FESIZE 32

extern long getft(const char *);

#endif /* __SASC */

#endif /* YAM_EXTRA_H */
