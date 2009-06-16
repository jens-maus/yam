/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2009 by YAM Open Source Team

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

 YAM Official Support Site :  http://www.yam.ch
 YAM OpenSource project    :  http://sourceforge.net/projects/yamos/

 NOTE:
 This implementation of NewReadArgs() is inspired by the implementation of
 Stephan Rupprecht's NewReadArgs().

 $Id$

***************************************************************************/

/*- INCLUDES & DEFINES -*/
#if defined(__amigaos4__)
  #define COMPILE_V52
#else
  #define COMPILE_V39
#endif

#include <dos/dos.h>
#include <exec/memory.h>
#if defined(COMPILE_V52)
  #include <exec/exectags.h>
#endif
#include <workbench/startup.h>
#include <workbench/workbench.h>
#include <workbench/icon.h>
#include <proto/intuition.h>
#include <proto/icon.h>
#include <proto/exec.h>
#include <proto/dos.h>

#include <stdlib.h>
#include <string.h>

#include "Debug.h"

#ifndef MEMF_SHARED
#define MEMF_SHARED MEMF_PUBLIC
#endif

/****************************************************************************/

/*- NEWRDARGS STRUCTURE -*/
struct NewRDArgs
{
  // initialize these fields before calling NewReadArgs() !!!
  STRPTR Template;      // ReadArgs() template
  STRPTR ExtHelp;       // ExtHelp string
  STRPTR Window;        // workbench window -> eg. "CON:////Test"
  IPTR *Parameters;     // where to store the data
  LONG FileParameter;   // -1 = none, 0 = all
  LONG PrgToolTypesOnly;

// private data section
  struct RDArgs *RDArgs;    // RDArgs we give to ReadArgs()
  struct RDArgs *FreeArgs;  // RDArgs we get from ReadArgs()

  #if defined(COMPILE_V39) || defined(COMPILE_V52)
  APTR Pool;
  #else
  struct Remember *Remember;  // the memory we`ve allocated
  #endif

  BPTR WinFH;     // i/o window stream
  BPTR OldInput;  // old i/o streams
  BPTR OldOutput;
};

/****************************************************************************/

void NewFreeArgs(struct NewRDArgs *);
LONG NewReadArgs(struct WBStartup *, struct NewRDArgs *);

/****************************************************************************/

void NewFreeArgs(struct NewRDArgs *rdargs)
{
  ENTER();

  D(DBF_STARTUP, "FreeArgs(rdargs->FreeArgs)");
  FreeArgs(rdargs->FreeArgs);

  if(rdargs->RDArgs)
  {
    if(rdargs->RDArgs->RDA_Source.CS_Buffer != NULL)
      free((void*)rdargs->RDArgs->RDA_Source.CS_Buffer);

    D(DBF_STARTUP, "FreeDosObject(DOS_RDARGS, rdargs->RDArgs)");
    FreeDosObject(DOS_RDARGS, rdargs->RDArgs);
  }

  if(rdargs->WinFH)
  {
    D(DBF_STARTUP, "SelectOutput( .. ) .. Close( ... )");
    SelectOutput(rdargs->OldOutput);
    Close(SelectInput(rdargs->OldInput));
  }

  #if defined(COMPILE_V39)
  if(rdargs->Pool)
    DeletePool(rdargs->Pool);
  #elif defined(COMPILE_V52)
  if(rdargs->Pool)
    FreeSysObject(ASOT_MEMPOOL, rdargs->Pool);
  #else
  if(rdargs->Remember)
    FreeRemember(&rdargs->Remember, TRUE);
  #endif

  D(DBF_STARTUP, "memory freed");

  LEAVE();
}

/****************************************************************************/

STATIC LONG IsArg(CONST_STRPTR template, STRPTR keyword)
{
  UBYTE buffer[128], c;
  STRPTR ptr = (STRPTR)buffer;

  while((c = *keyword++) && (c != '=')) *ptr++ = c;

  *ptr = 0;

  /*- checks if keyword is specified in template -*/
  return(FindArg(template, (STRPTR)buffer));
}

/****************************************************************************/

LONG NewReadArgs( struct WBStartup *WBStartup, struct NewRDArgs *nrdargs)
{
  #ifdef ICONGETA_RemapIcon
  static const struct TagItem icontags[] =
  {
    { ICONGETA_RemapIcon, FALSE   },
    { TAG_DONE,           TAG_END }
  };
  #endif

  ENTER();

  nrdargs->RDArgs   =
  nrdargs->FreeArgs = NULL;
  nrdargs->WinFH    = 0;
  #if defined(COMPILE_V39) || defined(COMPILE_V52)
  nrdargs->Pool   = NULL;
  #else
  nrdargs->Remember = NULL;
  #endif

  if((nrdargs->RDArgs = (struct RDArgs *)AllocDosObject(DOS_RDARGS, NULL)))
  {
    #if defined(COMPILE_V39) || defined(COMPILE_V52)
    APTR pool = NULL;
    #else
    struct Remember **remember = &nrdargs->Remember;
    #endif
    CONST_STRPTR ToolWindow = nrdargs->Window;

    if(WBStartup)
    {
      struct WBArg *wbarg;
      STRPTR *Args;
      STRPTR ptr;
      LONG MaxArgs = 1;
      LONG *ArgLen;
      LONG num = WBStartup->sm_NumArgs;
      LONG FileArgs = nrdargs->FileParameter;
      LONG FArgNum = -1L;
      LONG MultiArg = -1L;

      if(!(ptr = nrdargs->Template))
      {
        RETURN(ERROR_BAD_TEMPLATE);
        return(ERROR_BAD_TEMPLATE);
      }

      /*- count max number of args -*/
      while(*ptr)
      {
        if(*ptr++ == '/' && *ptr == 'M' )
        {
          MultiArg = MaxArgs-1L;
          ptr++;
        }
        else if(*(ptr-1) == ',')
          MaxArgs++;
      }

      D(DBF_STARTUP, "Args: %ld", MaxArgs);
      ptr = nrdargs->Template;

      /*- how many file args? -*/
      FileArgs = (FileArgs > num) ? num : ((FileArgs == -1) ? 0L : num);
      MaxArgs += FileArgs;

      #if defined(COMPILE_V39)
      pool = nrdargs->Pool = CreatePool(MEMF_CLEAR, 1024, 1024);
      #elif defined(COMPILE_V52)
      pool = nrdargs->Pool = AllocSysObjectTags(ASOT_MEMPOOL, ASOPOOL_MFlags, MEMF_SHARED|MEMF_CLEAR,
                                                              ASOPOOL_Puddle, 1024,
                                                              ASOPOOL_Threshold, 1024,
                                                              TAG_DONE);
      #else
      pool = NULL;
      #endif

      #if defined(COMPILE_V39) || defined(COMPILE_V52)
      if(pool == NULL || (Args = AllocPooled(pool, MaxArgs*sizeof(STRPTR)*2)) == NULL)
      #else
      if((Args = AllocRemember(remember, MaxArgs*sizeof(STRPTR)*2, MEMF_CLEAR)) == NULL)
      #endif
      {
        RETURN(ERROR_NO_FREE_STORE);
        return(ERROR_NO_FREE_STORE);
      }

      // no need to clear the Args array here, because the pool is of type MEMF_CLEAR

      ArgLen = (LONG *)&Args[MaxArgs];

      for(wbarg = WBStartup->sm_ArgList, num = 0L;
          num < WBStartup->sm_NumArgs;
          num++, wbarg++)
      {
        struct DiskObject *dobj;
        BPTR      olddir;

        /*- get file-names if requested -*/
        if(FileArgs)
        {
          char buf[300];

          if(FArgNum < FileArgs && FArgNum >= 0L)
          {
            D(DBF_STARTUP, "ICON: %s", wbarg->wa_Name);

            if(NameFromLock(wbarg->wa_Lock, buf, sizeof(buf)) &&
               AddPart(buf, (char *)wbarg->wa_Name, sizeof(buf)))
            {
              STRPTR dst;
              LONG len = strlen(buf) + 2L;
              #if defined(COMPILE_V39) || defined(COMPILE_V52)
              if((Args[FArgNum] = dst = AllocPooled(pool, len)))
              #else
              if((Args[FArgNum] = dst = AllocRemember(remember, len, MEMF_SHARED|MEMF_CLEAR)))
              #endif
              {
                CopyMem(buf, (dst+1), len-2L);
                *dst = dst[len-1] = '"';

                ArgLen[FArgNum] = len;
              }
              else
              {
                RETURN(ERROR_NO_FREE_STORE);
                return(ERROR_NO_FREE_STORE);
              }
            }
            else
            {
              RETURN(ERROR_LINE_TOO_LONG);
              return(ERROR_LINE_TOO_LONG);
            }
          }

          FArgNum++;
        }

        if(nrdargs->PrgToolTypesOnly && num)
          continue;

        olddir = CurrentDir(wbarg->wa_Lock);

        /*- get tooltypes from .info file -*/
        dobj =
        #ifdef ICONGETA_RemapIcon
          (((struct Library *)IconBase)->lib_Version >= 44L) ?
          GetIconTagList((char *)wbarg->wa_Name, (struct TagItem *)icontags) :
        #endif
          GetDiskObject((char *)wbarg->wa_Name);

        if(dobj)
        {
          if(dobj->do_ToolTypes && (dobj->do_Type == WBTOOL || dobj->do_Type == WBPROJECT))
          {
            STRPTR *tarray = (STRPTR *)dobj->do_ToolTypes;

            while(*tarray)
            {
              if(**tarray != '(')
              {
                STRPTR src = *tarray;
                LONG i;

                D(DBF_STARTUP, "tt: %s", *tarray);

                /*- valid arg ? -*/
                if((i = IsArg(ptr, src)) > -1)
                {
                  STRPTR dst;
                  LONG len;

                  i += FileArgs;

                  if( ArgLen[i] == 0L || (i-FileArgs) != MultiArg )
                  {
                    #if defined(COMPILE_V39) || defined(COMPILE_V52)
                    if((Args[i] = dst = AllocPooled(pool, (len = strlen(src))+2L)))
                    #else
                    if((Args[i] = dst = AllocRemember(remember, (len = strlen(src))+2L, MEMF_SHARED|MEMF_CLEAR)))
                    #endif
                    {
                      /*- copy arg -*/
                      while(*src)
                      {
                        if(((*dst++ = *src++) == '=') && (*src != '"'))
                        {
                          *dst++ = Args[i][len+1] = '"';
                          len+=2;
                        }
                      }

                      ArgLen[i] = len;
                    }
                    else
                    {
                      RETURN(ERROR_NO_FREE_STORE);
                      return(ERROR_NO_FREE_STORE);
                    }
                  }
                  else
                  {
                    while(*src && *src++ != '=' );

                    len = strlen( src ) + 1 + ArgLen[i];
                    #if defined(COMPILE_V39) || defined(COMPILE_V52)
                    if( (dst = AllocPooled(pool, len+2)) )
                    #else
                    if( (dst = AllocRemember(remember, len+2, MEMF_SHARED|MEMF_CLEAR)) )
                    #endif
                    {
                      BOOL quotes = FALSE;
                      UBYTE c;

                      CopyMem( Args[i], dst, len );
                      Args[i] = dst;
                      dst += ArgLen[i];
                      *dst++ = ' ';

                      if(*src != '"')
                      {
                        quotes = TRUE;
                        *dst++ = '"';
                        len += 2;
                      }

                      while(c = *src++,c)
                        *dst++ = c;

                      if(quotes)
                        *dst = '"';

                      ArgLen[i] = len;
                    }
                  }
                }
                /*- arg not specified in template, check for WINDOW tooltype -*/
                else if(!IsArg("WINDOW", src))
                {
                  if((i = strlen(src)-6L) > 1L)
                  {
                    #if defined(COMPILE_V39) || defined(COMPILE_V52)
                    if((ToolWindow = AllocPooled(pool, i)))
                    #else
                    if((ToolWindow = AllocRemember(remember, i, MEMF_SHARED|MEMF_CLEAR)))
                    #endif
                      CopyMem((src+7L), (STRPTR)ToolWindow, i);
                  }
                  else
                    ToolWindow = "CON:";
                }
              }

              tarray++;
            }
          }

          FreeDiskObject(dobj);
        }

        CurrentDir(olddir);
      }

      /*- now copy all given args to a single line -*/
      for(num = FileArgs = 0; FileArgs < MaxArgs; FileArgs++)
        num += ArgLen[FileArgs];

      if(num)
      {
        nrdargs->RDArgs->RDA_Source.CS_Length = (num+=MaxArgs);
        nrdargs->RDArgs->RDA_Source.CS_Buffer = malloc(num+1);
        ptr = (char *)nrdargs->RDArgs->RDA_Source.CS_Buffer;

        if(ptr)
        {
          for(FileArgs = 0; FileArgs < MaxArgs; FileArgs++)
          {
            if((num = ArgLen[FileArgs]))
            {
              CopyMem(Args[FileArgs], ptr, num);
              ptr += num;
              *ptr++ = ' ';
            }
          }
        }
        else
        {
          RETURN(ERROR_NO_FREE_STORE);
          return(ERROR_NO_FREE_STORE);
        }

        *(ptr-1) = '\n';
        *ptr = '\0'; // not really needed

        D(DBF_STARTUP, "CS_Buffer: %s", nrdargs->RDArgs->RDA_Source.CS_Buffer);
      }
      else
      {
        nrdargs->RDArgs->RDA_Source.CS_Length = 1;
        nrdargs->RDArgs->RDA_Source.CS_Buffer = malloc(1);
        ptr = (char *)nrdargs->RDArgs->RDA_Source.CS_Buffer;

        if(ptr)
          *ptr = '\n';
      }
    }

    /*- call ReadArgs() -*/
    nrdargs->RDArgs->RDA_ExtHelp = nrdargs->ExtHelp;
    if((nrdargs->FreeArgs = ReadArgs(nrdargs->Template, nrdargs->Parameters, nrdargs->RDArgs)) == NULL)
    {
      E(DBF_STARTUP, "ReadArgs() error");

      RETURN(IoErr());
      return(IoErr());
    }
    if(SetSignal(0, SIGBREAKF_CTRL_C) & SIGBREAKF_CTRL_C)
    {
      E(DBF_STARTUP, "ReadArgs() aborted by CTRL-C");

      RETURN(ERROR_BREAK);
      return ERROR_BREAK;
    }
    else
      D(DBF_STARTUP, "ReadArgs() okay");

    /*- when started from wb, open window if requested -*/
    if(ToolWindow && WBStartup)
    {
      D(DBF_STARTUP, "WINDOW has been defined");
      if((nrdargs->WinFH = Open(ToolWindow, MODE_READWRITE)))
      {
        D(DBF_STARTUP, "Opened WINDOW=%s", ToolWindow);
        nrdargs->OldInput = SelectInput(nrdargs->WinFH);
        nrdargs->OldOutput = SelectOutput(nrdargs->WinFH);
      }
      else
      {
        RETURN(IoErr());
        return(IoErr());
      }
    }

    #if defined(COMPILE_V39)
    if(pool)
    {
      DeletePool( pool );
      nrdargs->Pool = NULL;
    }
    #elif defined(COMPILE_V52)
    if(pool)
    {
      FreeSysObject(ASOT_MEMPOOL, pool);
      nrdargs->Pool = NULL;
    }
    #else
    if(*remember)
    {
      FreeRemember( remember, TRUE );
      nrdargs->Remember = NULL;
    }
    #endif
  }
  else
  {
    RETURN(ERROR_NO_FREE_STORE);
    return(ERROR_NO_FREE_STORE);
  }

  RETURN(RETURN_OK);
  return(RETURN_OK);
}

/****************************************************************************/
