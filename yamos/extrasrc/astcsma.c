#include <proto/dos.h>
#include "extra.h"
#include <stdlib.h>

/* Not completely equivalent to the SAS/C version, as the returned value
   is not the same. But YAM only uses the return value as a boolean. */
int astcsma(const char *s, const char *p)
{
   int ret,len=strlen(p)*2+2;
   char *buf=malloc(len);

   if(!buf) return 0;
   if (ParsePatternNoCase(p, buf, len) < 0) ret=0;
   else ret=MatchPatternNoCase(buf, (STRPTR)s);
   free(buf);
   return ret;
}
