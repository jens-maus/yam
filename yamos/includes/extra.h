#ifndef _EXTRA_H
#define _EXTRA_H

#ifdef __SASC

#include <dos.h>
#include <error.h>

#else

#ifdef _DCC
   #include <fcntl.h>
   #define index(a,b) strchr(a,b)
   #define isascii(c) (((c)&0xff)<127)
   extern struct Library *WorkbenchBase;
   extern struct Library *KeymapBase;
   extern void dice_closelibs(void);
#else
   #include <unistd.h>
#endif

#define _OSERR IoErr()

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
int stch_i(const char *s,int *res);

/*
** <dos.h>
*/

#define FNSIZE 108
#define FMSIZE 256
#define FESIZE 32

extern long getft(const char *);

/*
** <sys/commwben.h>
*/

#ifdef _DCC
#define _WBenchMsg _WBMsg
#endif
extern struct WBStartup *_WBenchMsg;

#endif /* __SASC */

#endif /* _EXTRA_H */
