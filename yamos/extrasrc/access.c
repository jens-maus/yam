#include <dos/dos.h>
#include <proto/dos.h>
#include "extra.h"

int
access(const char *name,int mode)
{ BPTR l = Lock((STRPTR)name,ACCESS_READ);
  if (!l)
    return -1;
  UnLock(l);
  return 0;
}
