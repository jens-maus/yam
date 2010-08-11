/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2010 by YAM Open Source Team

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
#include <dos/dos.h>
#include <exec/memory.h>
#if defined(__amigaos4__)
  #include <exec/exectags.h>
#endif
#include <workbench/startup.h>
#include <workbench/workbench.h>
#include <workbench/icon.h>
#include <proto/icon.h>
#include <proto/exec.h>
#include <proto/dos.h>

#include <stdlib.h>
#include <string.h>

#include "SDI_compiler.h"

#include "extrasrc.h"

#include "Debug.h"

#if defined(NEED_NEWREADARGS)

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
  STRPTR *Args;
  ULONG MaxArgs;
  STRPTR ToolWindow;

  APTR Pool;

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

  if(rdargs->Args != NULL)
  {
    ULONG i;

    for(i=0; i < rdargs->MaxArgs; i++)
    {
      if(rdargs->Args[i] != NULL)
        FreeVecPooled(rdargs->Pool, rdargs->Args[i]);
    }
    FreeVecPooled(rdargs->Pool, rdargs->Args);
    rdargs->Args = NULL;
  }

  if(rdargs->ToolWindow != NULL)
  {
    FreeVecPooled(rdargs->Pool, rdargs->ToolWindow);
    rdargs->ToolWindow = NULL;
  }

  if(rdargs->WinFH)
  {
    D(DBF_STARTUP, "SelectOutput( .. ) .. Close( ... )");
    SelectOutput(rdargs->OldOutput);
    Close(SelectInput(rdargs->OldInput));
  }

  if(rdargs->Pool != NULL)
  {
    #if defined(__amigaos4__)
    FreeSysObject(ASOT_MEMPOOL, rdargs->Pool);
    #else
    DeletePool(rdargs->Pool);
    #endif
    rdargs->Pool = NULL;
  }

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

  nrdargs->RDArgs     = NULL;
  nrdargs->FreeArgs   = NULL;
  nrdargs->Args       = NULL;
  nrdargs->MaxArgs    = 0;
  nrdargs->ToolWindow = NULL;
  nrdargs->WinFH      = 0;
  nrdargs->Pool       = NULL;

  if((nrdargs->RDArgs = (struct RDArgs *)AllocDosObject(DOS_RDARGS, NULL)))
  {
    APTR pool = NULL;
    STRPTR ToolWindow = nrdargs->Window;

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

      #if defined(__amigaos4__)
      pool = nrdargs->Pool = AllocSysObjectTags(ASOT_MEMPOOL, ASOPOOL_MFlags, MEMF_SHARED|MEMF_CLEAR,
                                                              ASOPOOL_Puddle, 1024,
                                                              ASOPOOL_Threshold, 1024,
                                                              ASOPOOL_Name, "YAM readargs pool",
                                                              TAG_DONE);
      #else
      pool = nrdargs->Pool = CreatePool(MEMF_CLEAR, 1024, 1024);
      #endif

      if(pool == NULL || (Args = nrdargs->Args = AllocVecPooled(pool, MaxArgs*sizeof(STRPTR)*2)) == NULL)
      {
        RETURN(ERROR_NO_FREE_STORE);
        return(ERROR_NO_FREE_STORE);
      }

      nrdargs->MaxArgs = MaxArgs;

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

              if((Args[FArgNum] = dst = AllocVecPooled(pool, len)))
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
                    if((Args[i] = dst = AllocVecPooled(pool, (len = strlen(src))+2L)))
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
                    if( (dst = AllocVecPooled(pool, len+2)) )
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
                  if((i = strlen(src)-7) > 0)
                  {
                    if((ToolWindow = nrdargs->ToolWindow = AllocVecPooled(pool, i+1)))
                      strlcpy(ToolWindow, &src[7], i+1);
                  }
                  else
                    ToolWindow = (STRPTR)"CON:";
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
    if((nrdargs->FreeArgs = ReadArgs(nrdargs->Template, (APTR)nrdargs->Parameters, nrdargs->RDArgs)) == NULL)
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

    if(nrdargs->Args != NULL)
    {
      ULONG i;

      for(i=0; i < nrdargs->MaxArgs; i++)
      {
        if(nrdargs->Args[i] != NULL)
          FreeVecPooled(nrdargs->Pool, nrdargs->Args[i]);
      }
      FreeVecPooled(nrdargs->Pool, nrdargs->Args);
      nrdargs->Args = NULL;
    }

    if(nrdargs->ToolWindow != NULL)
    {
      FreeVecPooled(nrdargs->Pool, nrdargs->ToolWindow);
      nrdargs->ToolWindow = NULL;
    }

    if(nrdargs->Pool != NULL)
    {
      #if defined(__amigaos4__)
      FreeSysObject(ASOT_MEMPOOL, nrdargs->Pool);
      #else
      DeletePool(nrdargs->Pool);
      #endif
      nrdargs->Pool = NULL;
    }
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

#else
  #warning "NEED_NEWREADARGS missing or compilation unnecessary"
#endif
