#include <proto/dos.h>
#include "extra.h"

int mkdir(const char *name)
{
  BPTR fl = CreateDir((STRPTR)name);

  if (!fl)
    return -1;

  UnLock(fl); return 0;
}
