#define __inline
#include "extrasrc/md5.c"
#undef __inline
#include "extrasrc/astcsma.c"
#include "extrasrc/getft.c"
#include "extrasrc/stccpy.c"
#include "extrasrc/stcgfe.c"
#include "extrasrc/stcgfn.c"
#include "extrasrc/stpblk.c"
#include "extrasrc/strmfp.c"
#include "extrasrc/strsfn.c"
#include "extrasrc/wbpath.c"
#include "extrasrc/NewReadArgs.c"
#include "extrasrc/stch_i.c"
char *index(const char *s, int c)
{
   return strchr (s, c);
}
#include <proto/dos.h>
int access(const char * name, int x)
{
   BPTR l;

   if((l=Lock(name,ACCESS_READ)))
   {
      UnLock(l);
      return 1;
   }

   return 0;
}
void bcopy(const void *src, void *dst, size_t len)
{
   memcpy(dst, src, len);
}
char *strdup(const char *str)
{
   char *s;

   if(!str) return NULL;

   if((s=malloc(strlen(str))))
   {
      strcpy(s,str);
      return s;
   }

   return NULL;
}

void main(int, char *[]);
struct WBStartup *_WBenchMsg;
void wbmain(struct WBStartup *wbs)
{
   _WBenchMsg=wbs;
   main(0, (char**)wbs);
}

void KPrintf() {};
