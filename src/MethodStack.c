/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2010 by YAM Open Source Team

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundatidn; either version 2 of the License, or
 (at your optidn) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundatidn, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 YAM Official Support Site :  http://www.yam.ch
 YAM OpenSource project    :  http://sourceforge.net/projects/yamos/

 $Id$

***************************************************************************/

#include <string.h>
#include <stdlib.h>

#include <clib/alib_protos.h>

#include <dos/dostags.h>
#include <proto/exec.h>
#include <proto/dos.h>

#if defined(__amigaos4__)
#include <exec/exectags.h>
#endif

#include "YAM.h"
#include "YAM_utilities.h"

#include "SDI_stdarg.h"

#include "extrasrc.h"

#include "Locale.h"
#include "Requesters.h"
#include "Threads.h"

#include "Debug.h"

struct PushedMethod
{
  struct Message msg;  // make this a real Exec message
  Object *obj;         // pointer to the object for receiving the method call
  ULONG argCount;      // number of arguments to follow
  IPTR *args;          // pointer to a memory area setup for holding the args
};

/// InitMethodStack
// initialize the global method stack
BOOL InitMethodStack(void)
{
  BOOL success = FALSE;

  ENTER();

  if((G->methodStack = AllocSysObjectTags(ASOT_PORT, TAG_DONE)) != NULL)
  {
    success = TRUE;
  }

  RETURN(success);
  return success;
}

///
/// CleanupMethodStack
// clean up the global method stack
void CleanupMethodStack(void)
{
  ENTER();

  if(G->methodStack != NULL)
  {
    struct Message *msg;

    // pop all methods from the stack without handling them
    while((msg = GetMsg(G->methodStack)) != NULL)
    {
      struct PushedMethod *pm = (struct PushedMethod *)msg;

      free(pm->args);
      FreeSysObject(ASOT_MESSAGE, pm);
    }

    // free the stack
    FreeSysObject(ASOT_PORT, G->methodStack);
    G->methodStack = NULL;
  }

  LEAVE();
}

///
/// PushMethodOnStack
// push a method with all given parameters on the method stack
BOOL VARARGS68K PushMethodOnStack(Object *obj, ULONG argCount, ...)
{
  struct PushedMethod *pm;
  BOOL success = FALSE;

  ENTER();

  if((pm = AllocSysObjectTags(ASOT_MESSAGE, ASOMSG_Size, sizeof(*pm),
                                            TAG_DONE)) != NULL)
  {
    va_list args;
    ULONG i;

    va_start(args, argCount);

    // fill in the data
    pm->obj = obj;
    pm->argCount = argCount;
    if((pm->args = calloc(1, argCount*sizeof(IPTR))))
    {
      for(i = 0; i < argCount; i++)
        pm->args[i] = va_arg(args, IPTR);
    }

    va_end(args);

    // push the method on the stack
    PutMsg(G->methodStack, (struct Message *)pm);

    success = TRUE;
  }

  RETURN(success);
  return success;
}

///
/// CheckMethodStack
// handle pending pushed methods on the stack
void CheckMethodStack(void)
{
  struct Message *msg;

  ENTER();

  // try to pop a method from the stack
  while((msg = GetMsg(G->methodStack)) != NULL)
  {
    struct PushedMethod *pm = (struct PushedMethod *)msg;

    // perform the desired action
    DoMethodA(pm->obj, (Msg)&pm->args[0]);

    // finally free the handled method
    free(pm->args);
    FreeSysObject(ASOT_MESSAGE, pm);
  }

  LEAVE();
}

///
