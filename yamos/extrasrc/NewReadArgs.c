/*
**
**  NewReadArgs() -
**  a shell/workbench transparent ReadArgs() interface
**
**  © 1997-99 by Stephan Rupprecht
**  All rights reserved.
**
**  FREEWARE - I am not responsible for any damage that
**  is caused by the (mis)use of this program.
**
**  MaxonC++, OS2.04+
**
*/

/*- INCLUDES & DEFINES -*/
#include <dos/dos.h>
#include <exec/memory.h>
#include <workbench/startup.h>
#include <workbench/workbench.h>
#include <workbench/icon.h>
#include <proto/intuition.h>
#include <proto/icon.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <string.h>

#ifdef DEBUG
#include "Debug.h"
#define bug kprintf
#define d(x)  x
#else
#define bug
#define d(x)
#endif

//#define COMPILE_V39

/****************************************************************************/

/*- NEWRDARGS STRUCTURE -*/
struct NewRDArgs {
// initialize these fields before calling NewReadArgs() !!!
  STRPTR     Template;   // ReadArgs() template
  STRPTR     ExtHelp;  // ExtHelp string
  STRPTR     Window;   // workbench window -> eg. "CON:////Test"
  LONG    *Parameters;   // where to store the data
  LONG     FileParameter;  // -1 = none, 0 = all
  LONG     PrgToolTypesOnly;

// private data section
  struct RDArgs *RDArgs;  // RDArgs we give to ReadArgs()
  struct RDArgs *FreeArgs;  // RDArgs we get from ReadArgs()

#ifdef COMPILE_V39
  APTR     Pool;
#else
  struct Remember *Remember;  // the memory we`ve allocated
#endif

  BPTR     WinFH;   // i/o window stream
  BPTR     OldInput;  // old i/o streams
  BPTR     OldOutput;
};

/****************************************************************************/

void NewFreeArgs(struct NewRDArgs *);
LONG NewReadArgs(struct WBStartup *, struct NewRDArgs *);

/****************************************************************************/

void NewFreeArgs(struct NewRDArgs *rdargs)
{
  d( bug("--- NewFreeArgs ---\n"); )
  FreeArgs(rdargs->FreeArgs);
  d( bug("FreeArgs( rdargs->FreeArgs )\n"); )
  if(rdargs->RDArgs)
  {
    FreeVec( rdargs->RDArgs->RDA_Source.CS_Buffer );
    FreeDosObject(DOS_RDARGS, rdargs->RDArgs);
  }
  d( bug("FreeDosObject( DOS_RDARGS, rdargs->RDArgs )\n"); )
  if(rdargs->WinFH) 
  {
    SelectOutput(rdargs->OldOutput);
    Close(SelectInput(rdargs->OldInput));
    d( bug("SelectOutput( .. )\nClose( ... )\n"); )
  }

#ifndef COMPILE_V39
  if(rdargs->Remember)
    FreeRemember(&rdargs->Remember, TRUE);
#else
  if(rdargs->Pool)
    DeletePool(rdargs->Pool);
#endif
  d( bug("memory freed\n"); )
  d( bug("--- EXIT ---\n"); )
}

/****************************************************************************/

STATIC LONG IsArg( STRPTR template, STRPTR keyword)
{
  UBYTE buffer[128], c;
  STRPTR  ptr = buffer;

  while((c = *keyword++) && (c != '=')) *ptr++ = c;

  *ptr = 0;

  /*- checks if keyword is specified in template -*/
  return(FindArg(template, buffer));
}

/****************************************************************************/

LONG NewReadArgs( struct WBStartup *WBStartup, struct NewRDArgs *nrdargs)
{
  #ifdef ICONGETA_RemapIcon
  static const Tag icontags[] = {
    ICONGETA_RemapIcon,FALSE, TAG_DONE
  };
  #endif

  d( bug("--- NewReadArgs ---\n"); )

  nrdargs->RDArgs   =
  nrdargs->FreeArgs = NULL;
  nrdargs->WinFH    = 0;
#ifndef COMPILE_V39
  nrdargs->Remember = NULL;
#else
  nrdargs->Pool   = NULL;
#endif

  if((nrdargs->RDArgs = (struct RDArgs *)AllocDosObject(DOS_RDARGS, NULL)))
  {
#ifndef COMPILE_V39
    struct Remember **remember = &nrdargs->Remember;
#else
    APTR      pool = NULL;
#endif
    STRPTR  ToolWindow = nrdargs->Window;

    if(WBStartup)
    {
      struct WBArg  *wbarg;
      STRPTR      *Args, ptr;
      LONG      MaxArgs = 1, *ArgLen, num = WBStartup->sm_NumArgs,
              FileArgs = nrdargs->FileParameter, FArgNum = -1L, MultiArg = -1L;

      if(!(ptr = nrdargs->Template))
        return(ERROR_BAD_TEMPLATE);

      /*- count max number of args -*/
      while(*ptr)
      {
        if( *ptr++ == '/' && *ptr == 'M' ) {
          MultiArg = MaxArgs-1L;
          ptr++;
        }
        else if(*(ptr-1) == ',') MaxArgs++;
      }
      d( bug("Args: %ld\n", MaxArgs); )
      ptr = nrdargs->Template;

      /*- how many file args? -*/
      FileArgs = (FileArgs > num) ? num : ((FileArgs == -1) ? 0L : num);
      MaxArgs += FileArgs;

#ifndef COMPILE_V39
      if(!(Args = AllocRemember(remember, MaxArgs*sizeof(STRPTR)*2, MEMF_ANY|MEMF_CLEAR)))
        return(ERROR_NO_FREE_STORE);
#else
      if(!(pool = nrdargs->Pool = CreatePool(MEMF_ANY, 1024, 1024)) || !(Args = AllocPooled(pool, MaxArgs*sizeof(STRPTR)*2)))
        return(ERROR_NO_FREE_STORE);

      for(num = 0L; num < (MaxArgs*2); num++)
        Args[num] = 0L;
#endif

      ArgLen = (LONG *)&Args[MaxArgs];

      for(  wbarg = WBStartup->sm_ArgList, num = 0L;
          num < WBStartup->sm_NumArgs;
          num++, wbarg++  )
      {
        struct DiskObject *dobj;
        BPTR      olddir;

        /*- get file-names if requested -*/
        if(FileArgs)
        {
          TEXT  buf[300];

          if(FArgNum < FileArgs && FArgNum >= 0L)
          {
            d( bug("ICON: %s\n", wbarg->wa_Name); )

            if( NameFromLock(wbarg->wa_Lock, buf, sizeof(buf)) &&
              AddPart(buf, wbarg->wa_Name, sizeof(buf)) )
            {
              STRPTR  dst;
              LONG  len = strlen(buf) + 2L;
#ifndef COMPILE_V39
              if((Args[FArgNum] = dst = AllocRemember(remember, len, MEMF_ANY)))
#else
              if((Args[FArgNum] = dst = AllocPooled(pool, len)))
#endif
              {
                CopyMem(buf, (dst+1), len-2L);
                *dst = dst[len-1] = '"';

                ArgLen[FArgNum] = len;
              }
              else return(ERROR_NO_FREE_STORE);
            }
            else return(ERROR_LINE_TOO_LONG);
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
          GetIconTagList(wbarg->wa_Name, (struct TagItem *)icontags) :
#endif
          GetDiskObject(wbarg->wa_Name);

        if( dobj )
        {
          if(dobj->do_Type == WBTOOL || dobj->do_Type == WBPROJECT)
          {
            STRPTR  *tarray = (STRPTR *)dobj->do_ToolTypes;

            while(*tarray)
            {
              if(**tarray != '(')
              {
                STRPTR  src = *tarray;
                LONG  i;

                d( bug("tt: %s\n", *tarray); )

                /*- valid arg ? -*/
                if((i = IsArg(ptr, src)) > -1)
                {
                  STRPTR  dst;
                  LONG  len;

                  i += FileArgs;

                  if( ArgLen[i] == 0L || (i-FileArgs) != MultiArg )
                  {
#ifndef COMPILE_V39
                    if((Args[i] = dst = AllocRemember(remember, (len = strlen(src))+2L, MEMF_ANY)))
#else
                    if((Args[i] = dst = AllocPooled(pool, (len = strlen(src))+2L)))
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
                    else return(ERROR_NO_FREE_STORE);
                  }
                  else
                  {
                    while( *src && *src++ != '=' );

                    len = strlen( src ) + 1 + ArgLen[i];
#ifndef COMPILE_V39
                    if( (dst = AllocRemember(remember, len+2, MEMF_ANY)) )
#else
                    if( (dst = AllocPooled(pool, len+2)) )
#endif
                    {
                      BOOL  quotes = FALSE;
                      UBYTE c;

                      CopyMem( Args[i], dst, len );
                      Args[i] = dst;
                      dst += ArgLen[i];
                      *dst++ = ' ';

                      if( *src != '"' ) {
                        quotes = TRUE;
                        *dst++ = '"';
                        len += 2;
                      }

                      while( c = *src++,c )
                        *dst++ = c;

                      if( quotes ) *dst = '"';

                      ArgLen[i] = len;
                    }
                  }
                }
                /*- arg not specified in template, check for WINDOW tooltype -*/
                else if(!IsArg("WINDOW", src))
                {
                  if((i = strlen(src)-6L) > 1L)
                  {
#ifndef COMPILE_V39
                    if((ToolWindow = AllocRemember(remember, i, MEMF_ANY)))
#else
                    if((ToolWindow = AllocPooled(pool, i)))
#endif
                      CopyMem((src+7L), ToolWindow, i);
                  }
                  else ToolWindow = "CON:";
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

        if((nrdargs->RDArgs->RDA_Source.CS_Buffer = ptr = AllocVec(num+1, MEMF_ANY)))
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
        else return(ERROR_NO_FREE_STORE);

        *(ptr-1) = '\n';
        *ptr = '\0'; // not really needed

        d( bug("CS_Buffer: %s", nrdargs->RDArgs->RDA_Source.CS_Buffer); )
      }
      else
      {
        nrdargs->RDArgs->RDA_Source.CS_Length = 1;
        if((nrdargs->RDArgs->RDA_Source.CS_Buffer = ptr = AllocVec(1, MEMF_ANY)))
          *ptr = '\n';
      }
    }

    /*- call ReadArgs() -*/
    nrdargs->RDArgs->RDA_ExtHelp = nrdargs->ExtHelp;
    if(!(nrdargs->FreeArgs = ReadArgs(nrdargs->Template, nrdargs->Parameters, nrdargs->RDArgs)))
    {
      d( bug("ReadArgs() error\n"); )
      return(IoErr());
    }

    d( bug("ReadArgs() okay\n"); )

    /*- when started from wb, open window if requested -*/
    if(ToolWindow && WBStartup)
    {
      d( bug("WINDOW has been defined\n"); )
      if((nrdargs->WinFH = Open(ToolWindow, MODE_READWRITE)))
      {
        d( bug("Opened WINDOW=%s\n", ToolWindow); )
        nrdargs->OldInput = SelectInput(nrdargs->WinFH);
        nrdargs->OldOutput = SelectOutput(nrdargs->WinFH);
      }
      else return(IoErr());
    }

#ifdef COMPILE_V39
    if( pool ) {
      DeletePool( pool );
      nrdargs->Pool = NULL;
    }
#else
    if( *remember ) {
      FreeRemember( remember, TRUE );
      nrdargs->Remember = NULL;
    }
#endif
  }
  else return(ERROR_NO_FREE_STORE);

  d( bug("--- EXIT ---\n"); )

  return(RETURN_OK);
}

/****************************************************************************/
