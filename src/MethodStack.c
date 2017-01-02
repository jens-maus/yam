/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2017 YAM Open Source Team

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
#include "MethodStack.h"
#include "Requesters.h"
#include "Threads.h"

#include "Debug.h"

struct PushedMethod
{
  struct Message msg;  // make this a real Exec message
  Object *object;      // pointer to the object for receiving the method call
  ULONG flags;         // various flags, i.e. synchronous exection
  ULONG argCount;      // number of arguments to follow
  IPTR *args;          // pointer to a memory area setup for holding the args
  IPTR result;         // return value of a synchronous call
};

#define PMF_SYNC       (1<<0)

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
BOOL PushMethodOnStackA(Object *obj, ULONG argCount, struct TagItem *tags)
{
  struct PushedMethod *pm;
  BOOL success = FALSE;

  ENTER();

  if((pm = AllocSysObjectTags(ASOT_MESSAGE,
    ASOMSG_Size, sizeof(*pm),
    TAG_DONE)) != NULL)
  {
    // fill in the data
    pm->object = obj;
    // execute this one asynchronous
    pm->flags = 0;
    pm->argCount = argCount;
    pm->args = memdup((void *)tags, argCount*sizeof(IPTR));

    // push the method on the stack
    PutMsg(G->methodStack, (struct Message *)pm);

    success = TRUE;
  }

  RETURN(success);
  return success;
}

///
/// PushMethodOnStackWait
// push a method with all given parameters on the method stack and wait
// for the application to handle it
IPTR PushMethodOnStackWaitA(Object *obj, ULONG argCount, struct TagItem *tags)
{
  struct PushedMethod *pm;
  IPTR result = (IPTR)-1;

  ENTER();

  if((pm = AllocSysObjectTags(ASOT_MESSAGE,
    ASOMSG_Size, sizeof(*pm),
    TAG_DONE)) != NULL)
  {
    // fill in the data
    pm->object = obj;
    // execute this one synchronous
    pm->flags = PMF_SYNC;
    pm->argCount = argCount;
    pm->args = memdup((void *)tags, argCount*sizeof(IPTR));

    if(IsMainThread() == TRUE)
    {
      // we have been called from within the main thread, so let's handle this
      // method immediately. But first we handle any already waiting method on
      // the stack.
      CheckMethodStack();

      // perform the desired action and get the return value
      result = DoMethodA(pm->object, (Msg)pm->args);
    }
    else
    {
      struct Process *me = (struct Process *)FindTask(NULL);
      struct MsgPort *replyPort = &me->pr_MsgPort;

      // set the process port as replyport
      pm->msg.mn_ReplyPort = replyPort;
      pm->result = (IPTR)-1;

      // push the method on the stack
      PutMsg(G->methodStack, (struct Message *)pm);

      // wait for the method to be handled
      WaitPort(replyPort);
      GetMsg(replyPort);

      result = pm->result;
    }

    // finally free the handled method
    free(pm->args);
    FreeSysObject(ASOT_MESSAGE, pm);
  }

  RETURN(result);
  return result;
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

    // check for synchronous or asynchronous execution
    if(isFlagSet(pm->flags, PMF_SYNC))
    {
      // perform the desired action and get the return value
      if(pm->object != NULL)
        pm->result = DoMethodA(pm->object, (Msg)pm->args);
      else
        pm->result = (IPTR)-1;

      // return to sender
      ReplyMsg((struct Message *)pm);
    }
    else
    {
      // perform the desired action
      DoMethodA(pm->object, (Msg)pm->args);

      // free the handled method
      free(pm->args);
      FreeSysObject(ASOT_MESSAGE, pm);
    }
  }

  LEAVE();
}

///
