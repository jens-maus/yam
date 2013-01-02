
/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2013 YAM Open Source Team

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 YAM Official Support Site :  http://www.yam.ch/
 YAM OpenSource project    :  http://sourceforge.net/projects/yamos/

 $Id$

***************************************************************************/

#include <string.h>

#include <dos/dos.h>
#include <proto/dos.h>

#include "YAM_utilities.h"

#include "FileInfo.h"
#include "extrasrc.h"

#include "Debug.h"

#if !defined(__amigaos4__)
// some filetype handling macros, used for non-AmigaOS4 builds only
#define FIB_IS_FILE(fib)     ((fib)->fib_DirEntryType < 0)
#define FIB_IS_DRAWER(fib)   ((fib)->fib_DirEntryType >= 0 && (fib)->fib_DirEntryType != ST_SOFTLINK && (fib)->fib_DirEntryType != ST_LINKDIR)
#endif

/// ObtainFileInfo
// query file <name> for the information <which>. If successful, the queried
// value is returned in the variable that <valuePtr> points to.
// Depending on <which> these variable types MUST be used for proper function:
//   FI_SIZE          LONG
//   FI_PROTECTION    ULONG
//   FI_COMMENT       char *
//   FI_DATE          struct DateStamp *
//   FI_TIME          ULONG
//   FI_TYPE          enum FileType
// File comments (FI_COMMENT) will be strdup()'ed and must be free()'d after usage to
// avoid memory leaks.
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
          memcpy((struct DateStamp *)valuePtr, &ed->Date, sizeof(struct DateStamp));
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
              memcpy((struct DateStamp *)valuePtr, &fib->fib_Date, sizeof(struct DateStamp));
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
              if(FIB_IS_FILE(fib))
                *((ULONG *)valuePtr) = FIT_FILE;
              else if(FIB_IS_DRAWER(fib))
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

  // In case the query failed, but we have been asked for the type of the object,
  // then we will simply signal success and tell that the requested object does
  // not exist.
  if(result == FALSE && which == FI_TYPE)
  {
    *((ULONG *)valuePtr) = FIT_NONEXIST;
    result = TRUE;
  }

  RETURN(result);
  return result;
}

///
/// FileExists
//  return true/false if a file/directory exists
BOOL FileExists(const char *filename)
{
  BOOL exists = FALSE;
  BPTR lock;

  ENTER();

  if(filename != NULL && filename[0] != '\0' &&
     (lock = Lock(filename, ACCESS_READ)))
  {
    D(DBF_UTIL, "file/dir '%s' does exist", filename);
    exists = TRUE;
    UnLock(lock);
  }
  else
    D(DBF_UTIL, "file/dir '%s' does NOT exist", SafeStr(filename));

  RETURN(exists);
  return exists;
}

///

