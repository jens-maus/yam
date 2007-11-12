#include <dos/dos.h>
#include <proto/dos.h>

#include <string.h>

#include "FileInfo.h"
#include "extrasrc.h"

#include "Debug.h"

#if !defined(__amigaos4__)
// some filetype handling macros
#define isFile(etype)     (etype < 0)
#define isDrawer(etype)   (etype >= 0 && etype != ST_SOFTLINK && etype != ST_LINKDIR)
#endif

BOOL ObtainFileInfo(const char *name, enum FileInfo which, void *valuePtr)
{
  BOOL result = FALSE;

  ENTER();

  *((ULONG *)valuePtr) = 0;

  D(DBF_UTIL, "getting file information %ld of file '%s'", which, name);
  #if defined(__amigaos4__)
  {
    struct ExamineData *ed;

    if((ed = ExamineObjectTags(EX_StringName, name, TAG_DONE)) != NULL)
    {
      switch(which)
      {
        case FI_SIZE:
        {
          *((LONG *)valuePtr) = ed->FileSize;
          result = TRUE;
        }
        break;

        case FI_PROTECTION:
        {
          *((ULONG *)valuePtr) = ed->Protection;
          result = TRUE;
        }
        break;

        case FI_COMMENT:
        {
          if((*((char **)valuePtr) = strdup(ed->Comment)) != NULL)
            result = TRUE;
        }
        break;

        case FI_DATE:
        {
          if((*((struct DateStamp **)valuePtr) = memdup(&ed->Date, sizeof(struct DateStamp))) != NULL)
            result = TRUE;
        }
        break;

        case FI_TIME:
        {
          *((ULONG *)valuePtr) = ((ed->Date.ds_Days + 2922) * 1440 +
                                   ed->Date.ds_Minute) * 60 +
                                   ed->Date.ds_Tick / TICKS_PER_SECOND;
          result = TRUE;
        }
        break;

        case FI_TYPE:
        {
          if(EXD_IS_FILE(ed))
            *((ULONG *)valuePtr) = FIT_FILE;
          else if(EXD_IS_DIRECTORY(ed))
            *((ULONG *)valuePtr) = FIT_DRAWER;
          else
            *((ULONG *)valuePtr) = FIT_UNKNOWN;
          result = TRUE;
        }
        break;
      }

      FreeDosObject(DOS_EXAMINEDATA, ed);
    }
  }
  #else
  {
    BPTR lock;

    if((lock = Lock(name, ACCESS_READ)))
    {
      struct FileInfoBlock *fib;

      if((fib = AllocDosObject(DOS_FIB, NULL)) != NULL)
      {
        if(Examine(lock, fib))
        {
          switch(which)
          {
            case FI_SIZE:
            {
              *((ULONG *)valuePtr) = fib->fib_Size;
              result = TRUE;
            }
            break;

            case FI_PROTECTION:
            {
              *((ULONG *)valuePtr) = fib->fib_Protection;
              result = TRUE;
            }
            break;

            case FI_COMMENT:
            {
              if((*((char **)valuePtr) = strdup(fib->fib_Comment)) != NULL)
                result = TRUE;
            }
            break;

            case FI_DATE:
            {
              if((*((struct DateStamp **)valuePtr) = memdup(&fib->fib_Date, sizeof(struct DateStamp))) != NULL)
                result = TRUE;
            }
            break;

            case FI_TIME:
            {
              *((ULONG *)valuePtr) = ((fib->fib_Date.ds_Days + 2922) * 1440 +
                                       fib->fib_Date.ds_Minute) * 60 +
                                       fib->fib_Date.ds_Tick / TICKS_PER_SECOND;
              result = TRUE;
            }
            break;

            case FI_TYPE:
            {
              if(isFile(fib->fib_DirEntryType))
                *((ULONG *)valuePtr) = FIT_FILE;
              else if(isDrawer(fib->fib_DirEntryType))
                *((ULONG *)valuePtr) = FIT_DRAWER;
              else
                *((ULONG *)valuePtr) = FIT_UNKNOWN;
              result = TRUE;
            }
            break;
          }
        }

        FreeDosObject(DOS_FIB, fib);
      }

      UnLock(lock);
    }
  }
  #endif

  RETURN(result);
  return result;
}

