#ifdef _DCC
#include <sys/stat.h>
int getft(const char *fn)
{
   struct stat st;

   if(stat(fn,&st)) return -1;

   return st.st_mtime;
}
#else
#include <dos/dos.h>
#include <dos/dostags.h>
#include <proto/dos.h>
#include "extra.h"

#ifdef __libnix__
extern long __gmtoffset;
#else
#define __gmtoffset 0
#endif

long getft(const char *name)
{
   struct FileInfoBlock *fib = AllocDosObject(DOS_FIB, NULL);
   BPTR lock;
   long ret = 0;
   if (fib)
   {
      fib->fib_Date.ds_Days = 0;
      fib->fib_Date.ds_Minute = 0;
      fib->fib_Date.ds_Tick = 0;
      if ((lock = Lock(name, ACCESS_READ)))
      {
        Examine(lock, fib);
        UnLock(lock);
      }
      ret = ((fib->fib_Date.ds_Days + 2922) * 1440 +
              fib->fib_Date.ds_Minute + __gmtoffset) * 60 +
              fib->fib_Date.ds_Tick / TICKS_PER_SECOND;
      FreeDosObject(DOS_FIB, fib);
   }
   return ret;
}
#endif /* _DCC */
