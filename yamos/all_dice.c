#include "extrasrc/md5.c"
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
#include "extrasrc/dice.c"

void wbmain(struct WBStartup *wbs)
{
   main(0, (char**)wbs);
}

#include <stdarg.h>
int KPrintf(const char *format)
{
   int n; va_list vl;

   va_start(vl,format);
   n=fprintf(stderr,format,vl);
   va_end(vl);
   fflush(stderr);
   return n;
}
