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

/*
 * This is the thread implementation framework of YAM, which is
 * partly a modified version of the thread implementation in SimpleMail.
 * In fact it was highly inspired by the implementation of SimpleMail.
 *
 * Thanks to the authors of SimpleMail!
 */

#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#include <clib/alib_protos.h>

#include <dos/dostags.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/timer.h>

#if defined(__amigaos4__)
#include <exec/exectags.h>
#endif

#include "YAM.h"

#include "extrasrc.h"

#include "Locale.h"
#include "Requesters.h"
#include "Threads.h"

#include "Debug.h"

#define MAX_ARGS  4

/*** ThreadMessage ***/
struct ThreadMessage
{
  struct Message msg;     // to make ThreadMessage a full Message implementation
  BOOL startup;           // this is a startup message
  struct Thread *thread;  // the thread which it is the startup message for
  int (*function)(void);  // the function to execute
  int async;              // asynchronous or synchronous execution of function?
  int argcount;           // number of arguments the function accepts
  void *arg[MAX_ARGS];    // the argument pointers
  int result;
  BOOL called;            // function was called
  BOOL parentCanContinue;
};

/*** TimerMessage ***/
struct TimerMessage
{
  struct TimeRequest time_req;
  struct ThreadMessage *thread_msg;
  struct MinNode node;
};

// the default thread
static struct Thread *default_thread;

// a thread node which will be inserted to our
// global subthread list.
struct ThreadNode
{
  struct MinNode node;
  struct Thread *thread;
};

// local prototypes
static void HandleThreadMessage(struct ThreadMessage *tmsg);

/*** Thread management functions ***/
/// AbortThread()
// Abort the given thread or if NULL is given the default subthread.
void AbortThread(struct Thread *thread)
{
  ENTER();

  if(thread == NULL)
    thread = default_thread;

  Forbid();

  D(DBF_THREAD, "aborting thread at %08lx %08lx", thread, thread->process);

  // send a CTRL-C signal to signal an abort situation
  if(thread->process != NULL)
    Signal(&thread->process->pr_Task, SIGBREAKF_CTRL_C);

  Permit();

  LEAVE();
}

///
/// RemoveThread()
// Remove the thread which has replied to the given tmsg
static void RemoveThread(struct ThreadMessage *tmsg)
{
  struct Node *curNode;

  ENTER();

  // search through our subThreadList
  IterateList(&G->subThreadList, curNode)
  {
    struct ThreadNode *node = (struct ThreadNode *)curNode;

    if(node->thread == tmsg->thread)
    {
      if(node->thread->isDefault == TRUE)
        default_thread = NULL;

      D(DBF_THREAD, "got startup message of 0x%08lx back", node->thread);

      Remove((struct Node *)node);
      FreeSysObject(ASOT_MESSAGE, tmsg);
      FreeSysObject(ASOT_NODE, node->thread);
      FreeSysObject(ASOT_NODE, node);

      break;
    }
  }

  LEAVE();
}

///
/// InitThreadTimer()
// Initialize the timer
static BOOL InitThreadTimer(struct Thread *thread)
{
  ENTER();

  NewMinList(&thread->timer_request_list);

  if((thread->timer_port = AllocSysObjectTags(ASOT_PORT, TAG_DONE)) != NULL)
  {
    if((thread->timer_req = AllocSysObjectTags(ASOT_IOREQUEST, ASOIOR_Size,      sizeof(struct TimeRequest),
                                                               ASOIOR_ReplyPort, (IPTR)thread->timer_port,
                                                               TAG_DONE)) != NULL)
    {
      if(OpenDevice(TIMERNAME, UNIT_VBLANK, (struct IORequest *)thread->timer_req, 0) == 0)
      {
        RETURN(TRUE);
        return TRUE;
      }

      FreeSysObject(ASOT_IOREQUEST, thread->timer_req);
      thread->timer_req = NULL;
    }

    FreeSysObject(ASOT_PORT, thread->timer_port);
    thread->timer_port = NULL;
  }

  RETURN(FALSE);
  return FALSE;
}
///
/// CleanupThreadTimer()
// Cleanup the timer
static void CleanupThreadTimer(struct Thread *thread)
{
  struct Node *tnode;

  ENTER();

  while((tnode = RemTail((struct List *)&thread->timer_request_list)) != NULL)
  {
    struct TimerMessage *timer_msg = (struct TimerMessage *)((IPTR)tnode - OFFSET_OF(struct TimerMessage, node));

    AbortIO((struct IORequest *)&timer_msg->time_req);
    WaitIO((struct IORequest *)&timer_msg->time_req);

    if(timer_msg->thread_msg != NULL)
      FreeSysObject(ASOT_MESSAGE, timer_msg->thread_msg);

    FreeSysObject(ASOT_IOREQUEST, timer_msg);
  }

  CloseDevice((struct IORequest *)thread->timer_req);
  FreeSysObject(ASOT_IOREQUEST, thread->timer_req);
  thread->timer_req = NULL;

  FreeSysObject(ASOT_PORT, thread->timer_port);
  thread->timer_port = NULL;

  LEAVE();
}
///
/// CurrentThreadMask()
// Returns the mask of the thread port of the current process
ULONG CurrentThreadMask(void)
{
  struct Thread *thread = (struct Thread *)(FindTask(NULL)->tc_UserData);

  return (1UL << thread->thread_port->mp_SigBit) | (1UL << thread->timer_port->mp_SigBit);
}
///
/// HandleThreadEvent()
// Handle a new message in the send to the current process
void HandleThreadEvent(ULONG mask)
{
  struct Thread *thread;

  ENTER();

  thread = (struct Thread *)(FindTask(NULL)->tc_UserData);
  if(thread == NULL || thread != &G->mainThread)
    E(DBF_THREAD, "unknown thread handle %08lx main=%08lx", thread, &G->mainThread);

  // check if the mask hits the signal bit of a thread
  if(mask & (1UL << thread->thread_port->mp_SigBit))
  {
    struct ThreadMessage *tmsg;

    while((tmsg = (struct ThreadMessage *)GetMsg(thread->thread_port)) != NULL)
    {
      D(DBF_THREAD, "received thread message: 0x%08lx", tmsg);

      // check if this is a startup message
      if(tmsg->startup == TRUE)
      {
        RemoveThread(tmsg);
        continue;
      }

      HandleThreadMessage(tmsg);
    }
  }

  // and check if the mask hits because of the timer
  // event of a thread.
  if(mask & (1UL << thread->timer_port->mp_SigBit))
  {
    struct TimerMessage *timer_msg;

    while((timer_msg = (struct TimerMessage *)GetMsg(thread->timer_port)) != NULL)
    {
      struct ThreadMessage *tmsg = timer_msg->thread_msg;

      D(DBF_THREAD, "received timer message: 0x%08lx", timer_msg);

      if(tmsg != NULL)
      {
        // Now execute the thread message
        if(tmsg->function != NULL)
        {
          // depending on the amount of arguments we call
          // the function of the thread in a different manner.
          switch(tmsg->argcount)
          {
            case 0: tmsg->function(); break;
            case 1: ((int (*)(void *))tmsg->function)(tmsg->arg[0]); break;
            case 2: ((int (*)(void *, void *))tmsg->function)(tmsg->arg[0], tmsg->arg[1]); break;
            case 3: ((int (*)(void *, void *, void *))tmsg->function)(tmsg->arg[0], tmsg->arg[1], tmsg->arg[2]); break;
            case 4: ((int (*)(void *, void *, void *, void *))tmsg->function)(tmsg->arg[0], tmsg->arg[1], tmsg->arg[2], tmsg->arg[3]); break;
          }
        }

        FreeSysObject(ASOT_MESSAGE, tmsg);
      }

      Remove((struct Node *)&timer_msg->node);
      FreeSysObject(ASOT_MESSAGE, timer_msg);
    }
  }

  LEAVE();
}

///
/// ThreadEntry()
// Entrypoint for a new thread
static SAVEDS void ThreadEntry(void)
{
  struct Process *proc;
  struct ThreadMessage *msg;
  struct Thread *thread;

  ENTER();

  D(DBF_THREAD, "child thread waiting for parent's startup message");

  proc = (struct Process*)FindTask(NULL);
  WaitPort(&proc->pr_MsgPort);
  msg = (struct ThreadMessage *)GetMsg(&proc->pr_MsgPort);

  D(DBF_THREAD, "child thread got parent's startup message");

  // set the task's UserData field to store per thread data
  thread = msg->thread;
  if((thread->thread_port = AllocSysObjectTags(ASOT_PORT, TAG_DONE)) != NULL)
  {
    D(DBF_THREAD, "child thread created port at 0x%08lx", thread->thread_port);

    NewMinList(&thread->push_list);

    if(InitThreadTimer(thread) == TRUE)
    {
      int (*entry)(void *);

      // Store the thread pointer as userdata
      proc->pr_Task.tc_UserData = thread;

      // get task's entry function
      entry = (int(*)(void *))msg->function;
      entry(msg->arg[0]);

      thread->process = NULL;
      CleanupThreadTimer(thread);
    }

    FreeSysObject(ASOT_PORT, thread->thread_port);
    thread->thread_port = NULL;
  }

  D(DBF_THREAD, "child thread replying startup message to %08lx", msg->msg.mn_ReplyPort);
  Forbid();
  ReplyMsg((struct Message *)msg);

  LEAVE();
}

///
/// ParentThreadCanContinue()
// Informs the parent task that it can continue
BOOL ParentThreadCanContinue(void)
{
  BOOL result = FALSE;
  struct ThreadMessage *msg;

  ENTER();

  D(DBF_THREAD, "signaling parent thread to continue");

  if((msg = AllocSysObjectTags(ASOT_MESSAGE, ASOMSG_Size, sizeof(*msg),
                                             TAG_DONE)) != NULL)
  {
    // no reply port needed as this message is asynchronous
    msg->async = 1;
    msg->parentCanContinue = TRUE;

    PutMsg(G->mainThread.thread_port, (struct Message *)msg);

    // message is freed by parent task
    result = TRUE;
  }

  RETURN(result);
  return result;
}

///
/// StartNewThread()
// Runs the given function in a newly created thread under the given name
static struct Thread *StartNewThread(const char *thread_name, int (*entry)(void *), void *eudata)
{
  struct Thread *thread;

  ENTER();

  if((thread = AllocSysObjectTags(ASOT_NODE, ASONODE_Size, sizeof(*thread),
                                             ASONODE_Min, TRUE,
                                             TAG_DONE)) != NULL)
  {
    struct ThreadMessage *msg;

    if((msg = AllocSysObjectTags(ASOT_MESSAGE, ASOMSG_Size, sizeof(*msg),
                                               ASOMSG_ReplyPort, (IPTR)G->mainThread.thread_port,
                                               TAG_DONE)) != NULL)
    {
      msg->startup = TRUE;
      msg->thread = thread;
      msg->function = (int (*)(void))entry;
      msg->arg[0] = eudata;

      if((thread->process = CreateNewProcTags(NP_Entry,       ThreadEntry, // entry function
                                              NP_StackSize,   16384,       // stack size
                                              NP_Name,        thread_name,
                                              NP_Priority,    1,
                                              #if defined(__amigaos4__)
                                              NP_Child,       TRUE,
                                              #elif defined(__MORPHOS__)
                                              NP_CodeType,    MACHINE_PPC,
                                              #endif
                                              NP_Input,       ZERO,
                                              NP_CloseInput,  FALSE,
                                              NP_Output,      ZERO,
                                              NP_CloseOutput, FALSE,
                                              TAG_DONE)) != NULL)
      {
        D(DBF_THREAD, "thread started at 0x%08lx, sending message 0x%08lx %08lx", thread, msg, msg->msg.mn_ReplyPort);
        PutMsg(&thread->process->pr_MsgPort, (struct Message *)msg);

        do
        {
          struct Node *node;

          D(DBF_THREAD, "parent thread waiting for child thread message");
          Wait(1UL << G->mainThread.thread_port->mp_SigBit);
          D(DBF_THREAD, "got thread signal");

          // Warning: We are accessing the message port directly and scan through all messages (without
          // removing them), this may not be merely ugly but also a hack
          Forbid();

          IterateList(&G->mainThread.thread_port->mp_MsgList, node)
          {
            struct ThreadMessage *tmsg = (struct ThreadMessage *)node;

            if(tmsg == msg)
            {
              Remove(node);
              Permit();

              // This was the startup message, so something has failed
              D(DBF_THREAD, "got startup message back, something went wrong");
              FreeSysObject(ASOT_MESSAGE, tmsg);
              FreeSysObject(ASOT_NODE, thread);

              // Set the state of this message port to "hot" again
              SetSignal((1UL << G->mainThread.thread_port->mp_SigBit), (1UL << G->mainThread.thread_port->mp_SigBit));

              RETURN(NULL);
              return NULL;
            }

            if(tmsg->parentCanContinue == TRUE)
            {
              Remove(node);
              Permit();

              // This was the "parent task can continue" message, we don't reply it
              // but we free it here (although it wasn't allocated by this task)
              D(DBF_THREAD, "got 'parent can continue' message");
              FreeSysObject(ASOT_MESSAGE, tmsg);

              // Set the state of this message port to "hot" again
              SetSignal((1UL << G->mainThread.thread_port->mp_SigBit), (1UL << G->mainThread.thread_port->mp_SigBit));

              RETURN(thread);
              return thread;
            }
          }

          Permit();
        }
        while(TRUE);
      }

      FreeSysObject(ASOT_MESSAGE, msg);
    }
  }

  RETURN(NULL);
  return NULL;
}

///
/// AddThread()
// Runs a given function in a newly created thread under the given name which
// in linked into a internal list.
struct Thread *AddThread(const char *thread_name, int (*entry)(void *), void *eudata)
{
  struct ThreadNode *thread_node;

  ENTER();

  if((thread_node = AllocSysObjectTags(ASOT_NODE, ASONODE_Size, sizeof(*thread_node),
                                                  ASONODE_Min, TRUE,
                                                  TAG_DONE)) != NULL)
  {
    struct Thread *thread = StartNewThread(thread_name, entry, eudata);

    if(thread != NULL)
    {
      thread_node->thread = thread;

      D(DBF_THREAD, "adding thread to subthread list");
      AddTail((struct List *)&G->subThreadList, (struct Node *)thread_node);

      RETURN(thread);
      return thread;
    }

    FreeSysObject(ASOT_NODE, thread_node);
  }

  RETURN(NULL);
  return NULL;
}

///
/// StartAsDefaultThread()
// Start a thread as a default sub thread. This function will be removed
// in the future.
BOOL StartAsDefaultThread(int (*entry)(void *), void *eudata)
{
  BOOL result = FALSE;

  ENTER();

  // We allow only one subtask for the moment
  if(default_thread == NULL)
  {
    if((default_thread = AddThread("YAM default subthread", entry, eudata)) != NULL)
    {
      default_thread->isDefault = TRUE;

      result = TRUE;
    }
  }

  RETURN(result);
  return result;
}

///
/// CreateThreadMessage()
// Returns ThreadMessage filled with the given parameters. You can manipulate
// the returned message to be async or something else
static struct ThreadMessage *CreateThreadMessage(void *function, int argcount, va_list argptr)
{
  struct ThreadMessage *tmsg;
  struct MsgPort *subthread_port = ((struct Thread *)(FindTask(NULL)->tc_UserData))->thread_port;

  ENTER();

  if((tmsg = AllocSysObjectTags(ASOT_MESSAGE, ASOMSG_Size, sizeof(*tmsg),
                                              ASOMSG_ReplyPort, (IPTR)subthread_port,
                                              TAG_DONE)) != NULL)
  {

    tmsg->function = (int (*)(void))function;
    tmsg->argcount = argcount;

    if(argcount-- > 0)
    {
      tmsg->arg[0] = va_arg(argptr, void *);
      if(argcount-- > 0)
      {
        tmsg->arg[1] = va_arg(argptr, void *);
        if(argcount-- > 0)
        {
          tmsg->arg[2] = va_arg(argptr, void *);
          if(argcount-- > 0)
          {
            tmsg->arg[3] = va_arg(argptr, void *);
          }
        }
      }
    }

    tmsg->async = 0;
    tmsg->called = FALSE;
  }

  RETURN(tmsg);
  return tmsg;
}

///
/// HandleThreadMessage()
// This will handle the execute function message
static void HandleThreadMessage(struct ThreadMessage *tmsg)
{
  ENTER();

  if(tmsg->startup == FALSE)
  {
    if(tmsg->function != NULL)
    {
      switch(tmsg->argcount)
      {
        case  0: tmsg->result = tmsg->function(); break;
        case  1: tmsg->result = ((int (*)(void *))tmsg->function)(tmsg->arg[0]); break;
        case  2: tmsg->result = ((int (*)(void *, void *))tmsg->function)(tmsg->arg[0], tmsg->arg[1]); break;
        case  3: tmsg->result = ((int (*)(void *, void *, void *))tmsg->function)(tmsg->arg[0], tmsg->arg[1], tmsg->arg[2]); break;
        case  4: tmsg->result = ((int (*)(void *, void *, void *, void *))tmsg->function)(tmsg->arg[0], tmsg->arg[1], tmsg->arg[2], tmsg->arg[3]); break;
      }
    }

    if(tmsg->async != 0)
    {
      D(DBF_THREAD, "freeing message at 0x%08lx", tmsg);

      if(tmsg->argcount >= 1 && tmsg->async == 2)
        free(tmsg->arg[0]);

      FreeSysObject(ASOT_MESSAGE, tmsg);
    }
    else
    {
      D(DBF_THREAD, "replying message at 0x%08lx", tmsg);
      tmsg->called = TRUE;
      ReplyMsg(&tmsg->msg);
    }
  }

  LEAVE();
}

///
/// CallParentThreadFunctionSync()
// Call a function in context of the parent task synchronly. The contents of
// success is set to 1, if the call was successful otherwise to 0.
// success may be NULL. If success would be 0, the call returns 0 as well.
int CallParentThreadFunctionSync(BOOL *success, void *function, int argcount, ...)
{
  va_list argptr;
  int rc = 0;
  struct ThreadMessage *tmsg;

  ENTER();

  va_start(argptr, argcount);

  if((tmsg = CreateThreadMessage(function, argcount, argptr)) != NULL)
  {
    struct MsgPort *subthread_port = tmsg->msg.mn_ReplyPort;
    BOOL ready = FALSE;

    PutMsg(G->mainThread.thread_port, (struct Message *)&tmsg);

    while(ready == FALSE)
    {
      struct Message *msg;

      WaitPort(subthread_port);

      while((msg = GetMsg(subthread_port)) != NULL)
      {
        if(msg == &tmsg->msg)
          ready = TRUE;
        else
          HandleThreadMessage((struct ThreadMessage*)msg);
      }
    }

    rc = tmsg->result;
    if(success != NULL)
      *success = tmsg->called;

    FreeSysObject(ASOT_MESSAGE, tmsg);
  }
  else
  {
    if(success != NULL)
      *success = FALSE;
  }

  va_end(argptr);

  RETURN(rc);
  return rc;
}

///
/// CallParentThreadFunctionAsync()
// Call the function asynchron
BOOL CallParentThreadFunctionAsync(void *function, int argcount, ...)
{
  struct ThreadMessage *tmsg;
  BOOL result = FALSE;

  ENTER();

  if((tmsg = AllocSysObjectTags(ASOT_MESSAGE, ASOMSG_Size, sizeof(*tmsg),
                                              TAG_DONE)) != NULL)
  {
    va_list argptr;

    va_start(argptr, argcount);

    // Note that async messages are never replied, therefore no reply port is necessary
    tmsg->function = (int (*)(void))function;
    tmsg->argcount = argcount;
    tmsg->arg[0] = va_arg(argptr, void *); /*(*(&argcount + 1));*/
    tmsg->arg[1] = va_arg(argptr, void *); /*(void *)(*(&argcount + 2));*/
    tmsg->arg[2] = va_arg(argptr, void *); /*(void *)(*(&argcount + 3));*/
    tmsg->arg[3] = va_arg(argptr, void *); /*(void *)(*(&argcount + 4));*/
    tmsg->async = 1;

    va_end(argptr);

    PutMsg(G->mainThread.thread_port, (struct Message *)&tmsg);

    result = TRUE;
  }

  RETURN(result);
  return result;
}

///
/// CallParentThreadFunctionAsyncString()
// Call the function asynchron and duplicate the first argument which us
// threaded at a string
BOOL CallParentThreadFunctionAsyncString(void *function, int argcount, ...)
{
  struct ThreadMessage *tmsg;
  BOOL result = FALSE;

  if((tmsg = AllocSysObjectTags(ASOT_MESSAGE, ASOMSG_Size, sizeof(*tmsg),
                                              TAG_DONE)) != NULL)
  {
    va_list argptr;
    BOOL sendMsg = FALSE;

    va_start(argptr, argcount);

    // Note that async messages are never replied, therefore no reply port is necessary
    tmsg->function = (int (*)(void))function;
    tmsg->argcount = argcount;
    tmsg->arg[0] = va_arg(argptr, void *); /*(*(&argcount + 1));*/
    tmsg->arg[1] = va_arg(argptr, void *); /*(void *)(*(&argcount + 2));*/
    tmsg->arg[2] = va_arg(argptr, void *); /*(void *)(*(&argcount + 3));*/
    tmsg->arg[3] = va_arg(argptr, void *); /*(void *)(*(&argcount + 4));*/
    tmsg->async = 2;

    va_end(argptr);

    if(tmsg->arg[0] != NULL && argcount >= 1)
    {
      // duplicate the parameter string if there is one
      if((tmsg->arg[0] = strdup((char *)tmsg->arg[0])) != NULL)
      {
        sendMsg = TRUE;
      }
    }
    else
    {
      // send a message without parameter string
      sendMsg = TRUE;
    }

    if(sendMsg == TRUE)
    {
      PutMsg(G->mainThread.thread_port, (struct Message *)&tmsg);
      result = TRUE;
    }
  }

  RETURN(result);
  return result;
}

///
/// CallThreadFunctionSync()
// Call a function in the context of the given thread synchron
// NOTE: Should call thread_handle()
int CallThreadFunctionSync(struct Thread *thread, void *function, int argcount, ...)
{
  va_list argptr;
  int rc = 0;
  struct ThreadMessage *tmsg;

  ENTER();

  va_start(argptr, argcount);

  if((tmsg = CreateThreadMessage(function, argcount, argptr)) != NULL)
  {
    struct MsgPort *subthread_port = tmsg->msg.mn_ReplyPort;
    BOOL ready = FALSE;

    PutMsg(thread->thread_port, (struct Message *)&tmsg);

    while(ready == FALSE)
    {
      struct Message *msg;
      WaitPort(subthread_port);

      while((msg = GetMsg(subthread_port)) != NULL)
      {
        if(msg == &tmsg->msg)
          ready = TRUE;
        else
          HandleThreadMessage((struct ThreadMessage*)msg);
      }
    }

    rc = tmsg->result;
    FreeSysObject(ASOT_MESSAGE, tmsg);
  }

  va_end(argptr);

  RETURN(rc);
  return rc;
}

///
/// PushThreadFunction()
// Pushes a function call in the function queue of the callers task context.
BOOL PushThreadFunction(void *function, int argcount, ...)
{
  BOOL success = FALSE;
  struct ThreadMessage *tmsg;
  va_list argptr;

  ENTER();

  va_start(argptr, argcount);

  if((tmsg = CreateThreadMessage(function, argcount, argptr)) != NULL)
  {
    struct Thread *this_thread = ((struct Thread *)(FindTask(NULL)->tc_UserData));

    AddTail((struct List *)&this_thread->push_list, &tmsg->msg.mn_Node);
    success = TRUE;
  }

  va_end(argptr);

  RETURN(success);
  return success;
}

///
/// PushThreadFunctionDelayed()
// Pushes a function call in the function queue of the callers task context
// but only after a given amount of time.
BOOL PushThreadFunctionDelayed(int millis, void *function, int argcount, ...)
{
  BOOL success = FALSE;
  struct ThreadMessage *tmsg;
  va_list argptr;

  ENTER();

  va_start(argptr, argcount);

  if((tmsg = CreateThreadMessage(function, argcount, argptr)) != NULL)
  {
    struct Thread *thread = (struct Thread *)(FindTask(NULL)->tc_UserData);
    struct TimerMessage *timer_msg;

    if((timer_msg = AllocSysObjectTags(ASOT_IOREQUEST, ASOIOR_Duplicate, (IPTR)thread->timer_req,
                                                       TAG_DONE)) != NULL)
    {
      div_t milli;

      milli = div(millis, 1000);

      timer_msg->time_req.Request.io_Command = TR_ADDREQUEST;
      timer_msg->time_req.Time.Seconds = milli.quot;
      timer_msg->time_req.Time.Microseconds = milli.rem * 1000;
      timer_msg->thread_msg = tmsg;

      // first enqueue the timer_msg in our request list
      AddTail((struct List *)&thread->timer_request_list, (struct Node *)&timer_msg->node);

      // then start the timer
      SendIO(&timer_msg->time_req.Request);

      success = TRUE;
    }
    else
      FreeSysObject(ASOT_MESSAGE, tmsg);
  }

  va_end(argptr);

  RETURN(success);
  return success;
}

///
/// IsThreadAborted()
// Check if thread is aborted and return 1 if so
BOOL IsThreadAborted(void)
{
  BOOL aborted;

  ENTER();

  aborted = !!CheckSignal(SIGBREAKF_CTRL_C);
  D(DBF_THREAD, "aborted=%ld", aborted);

  RETURN(aborted);
  return aborted;
}

///

/*** Thread system init/cleanup functions ***/
/// InitThreads()
// initializes the thread system
BOOL InitThreads(void)
{
  BOOL result = FALSE;

  ENTER();

  if((G->mainThread.thread_port = AllocSysObject(ASOT_PORT, TAG_DONE)) != NULL)
  {
    G->mainThread.process = (struct Process*)FindTask(NULL);
    G->mainThread.isMain = TRUE;

    // init the thread own't timer stuff
    InitThreadTimer(&G->mainThread);

    // prepare the threads' function push list
    NewMinList(&G->mainThread.push_list);

    // initialize the subThread list
    NewMinList(&G->subThreadList);

    // set the user data of the main thread
    Forbid();
    FindTask(NULL)->tc_UserData = &G->mainThread;
    Permit();

    result = TRUE;
  }

  RETURN(result);
  return result;
}

///
/// CleanupThreads()
// cleanup the whole thread system - abort eventually active threads and
// wait for them to finish properly.
void CleanupThreads(void)
{
  ENTER();

  // first check if we have a valid port already, since this function
  // is called whenever YAM is aborted or shut down. But upon abortion
  // InitThreads() might not have been called yet.
  if(G->mainThread.thread_port != NULL)
  {
    while(IsMinListEmpty(&G->subThreadList) == FALSE)
    {
      ULONG thread_m;
      ULONG timer_m;
      struct TimerMessage *timeout = NULL;
      struct Node *curNode;

/*
// don't abort the threads for now, this causes crashes
      D(DBF_STARTUP, "aborting still running child threads");
      // signal all threads to abort
      IterateList(&G->subThreadList, curNode)
      {
        struct ThreadNode *node = (struct ThreadNode *)curNode;

        // abort the thread
        AbortThread(node->thread);
      }
*/

      // get the signalbit of the message ports of the thread
      // and its timer.
      thread_m = 1UL << G->mainThread.thread_port->mp_SigBit;
      timer_m = 1UL << G->mainThread.timer_port->mp_SigBit;

      // now iterate again through our subThreadList and
      // wait until the subthread have finished.
      // do not use IterateList(), because the embedded RemoveThread() call
      // will do a Remove() while we are walking through the very same list
      curNode = GetHead((struct List *)&G->subThreadList);
      while(curNode != NULL)
      {
        struct Node *nextNode = GetSucc(curNode);
/*
// don't abort the threads for now, this causes crashes
        struct ThreadNode *node = (struct ThreadNode *)curNode;
*/
        struct ThreadMessage *tmsg;
        ULONG mask;

        // wait half a second to give our threads enough time
        // to terminate their jobs...
        if(timeout == NULL &&
           (timeout = AllocSysObjectTags(ASOT_IOREQUEST, ASOIOR_Duplicate, (IPTR)G->mainThread.timer_req,
                                                         TAG_DONE)) != NULL)
        {
          timeout->time_req.Request.io_Command = TR_ADDREQUEST;
          timeout->time_req.Time.Seconds = 0;
          timeout->time_req.Time.Microseconds = 500000;

          D(DBF_STARTUP, "starting timeout thread");
          // first enqueue the timer_msg in our request list
          AddTail((struct List *)&G->mainThread.timer_request_list, (struct Node *)&timeout->node);

          // then start the timer
          SendIO(&timeout->time_req.Request);
        }

        D(DBF_STARTUP, "waiting for child threads to terminate");
        // wait until the main thread or its timer
        // wakes us up.
        mask = Wait(thread_m|timer_m);

        // check if we continued due to the issued timer
        // request
        if(mask & timer_m)
        {
          struct TimerMessage *timer;

          while((timer = (struct TimerMessage *)GetMsg(G->mainThread.timer_port)) != NULL)
          {
            if(timer == timeout)
            {
              W(DBF_THREAD, "timeout occured before main thread replied, aborting thread again");

              // time out occured, abort the current task another time
              timeout = NULL;
/*
// don't abort the threads for now, this causes crashes
              AbortThread(node->thread);
*/
            }

            Remove((struct Node *)&timer->node);
            FreeSysObject(ASOT_IOREQUEST, timer);
          }
        }

        // process all pending messages on our mainThread message port
        while((tmsg = (struct ThreadMessage *)GetMsg(G->mainThread.thread_port)) != NULL)
        {
          // if the thread was already started remove it!
          if(tmsg->startup == TRUE)
            RemoveThread(tmsg);
          else
          {
            D(DBF_STARTUP, "got non startup message (async=%ld)", tmsg->async);

            // check if the thread was running synchronous or asynchronous
            if(tmsg->async == 0)
            {
              tmsg->called = FALSE;
              ReplyMsg(&tmsg->msg);
            }
            else
              FreeSysObject(ASOT_MESSAGE, tmsg);
          }
        }

        curNode = nextNode;
      }

      if(IsMinListEmpty(&G->subThreadList) == FALSE)
      {
        if(MUI_Request(G->App, NULL, 0L, tr(MSG_THREAD_EXIT_WARNING_TITLE), 
                                         tr(MSG_THREAD_EXIT_WARNING_BT), 
                                         tr(MSG_THREAD_EXIT_WARNING)) == 0)
        {
          break;
        }
      }
    }

    D(DBF_STARTUP, "zero subthreads left");
    CleanupThreadTimer(&G->mainThread);

    FreeSysObject(ASOT_PORT, G->mainThread.thread_port);
    G->mainThread.thread_port = NULL;
  }

  LEAVE();
}

///

/*
// Call the function synchron, calls timer_callback on the calling process
// context
int thread_call_parent_function_sync_timer_callback(void (*timer_callback)(void *), void *timer_data, int millis, void *function, int argcount, ...)
{
  va_list argptr;
  int rc = 0;
  struct ThreadMessage *tmsg;

  ENTER();

  va_start(argptr,argcount);

  if((tmsg = CreateThreadMessage(function, argcount, argptr)) != NULL)
  {
    struct MsgPort *subthread_port = tmsg->msg.mn_ReplyPort;
    struct timer timer;

    if(timer_init(&timer))
    {
      int ready = 0;

      // we only accept positive values
      if(millis < 0) millis = 0;

      // now send the message
      PutMsg(G->mainThread.thread_port, (struct Message *)&tmsg);

      // while the parent task should execute the message
      // we regualiy call the given callback function
      while (!ready)
      {
        ULONG timer_m = timer_mask(&timer);
        ULONG proc_m = 1UL << subthread_port->mp_SigBit;
        ULONG mask;

        if(millis)
          timer_send_if_not_sent(&timer,millis);

        mask = Wait(timer_m|proc_m);
        if(mask & timer_m)
        {
          if(timer_callback)
            timer_callback(timer_data);
          timer.timer_send = 0;
        }

        if(mask & proc_m)
        {
          struct Message *msg;
          while ((msg = GetMsg(subthread_port)))
          {
            if(msg == &tmsg->msg) ready = 1;
          }
        }
      }
      rc = tmsg->result;
      timer_cleanup(&timer);
    }
    FreeVec(tmsg);
  }

  va_end(argptr);

  RETURN(rc);
  return rc;
}
*/

/*
// Waits until aborted and calls timer_callback periodically. It's possible
// to execute functions on the threads context while in this function.
void thread_wait(void (*timer_callback(void *)), void *timer_data, int millis)
{
  struct timer timer;
  if(timer_init(&timer))
  {
    struct Thread *this_thread = ((struct Thread *)(FindTask(NULL)->tc_UserData));
    struct MsgPort *this_thread_port = this_thread->thread_port;
    if(millis < 0) millis = 0;

    while (1)
    {
      ULONG proc_m = 1UL << this_thread_port->mp_SigBit;
      ULONG timer_m = timer_mask(&timer);
      ULONG mask;
      struct ThreadMessage *tmsg;

      if(millis) timer_send_if_not_sent(&timer,millis);

      mask = Wait(timer_m|proc_m|SIGBREAKF_CTRL_C);
      if(mask & timer_m)
      {
        if(timer_callback)
          timer_callback(timer_data);
        timer.timer_send = 0;
      }

      if(mask & proc_m)
      {
        struct ThreadMessage *tmsg;
        while ((tmsg = (struct ThreadMessage*)GetMsg(this_thread_port)))
        {
           thread_handle_execute_function_message(tmsg);
        }
      }

      // Now perform any pending push calls
      while ((tmsg = (struct ThreadMessage*)RemHead((struct List *)&this_thread->push_list)))
      {
        if(tmsg->function)
        {
          switch(tmsg->argcount)
          {
            case  0: tmsg->function(); break;
            case  1: ((int (*)(void *))tmsg->function)(tmsg->arg[1]); break;
            case  2: ((int (*)(void *, void *))tmsg->function)(tmsg->arg[1], tmsg->arg[2]); break;
            case  3: ((int (*)(void *, void *, void *))tmsg->function)(tmsg->arg[1], tmsg->arg[2], tmsg->arg[3]); break;
            case  4: ((int (*)(void *, void *, void *, void *))tmsg->function)(tmsg->arg[1], tmsg->arg[2], tmsg->arg[3], tmsg->arg[4]); break;
          }
        }
        FreeVec(tmsg);
      }

      if(mask & SIGBREAKF_CTRL_C) break;
    }
    timer_cleanup(&timer);
  }
}
*/

/*
struct semaphore_s
{
  struct SignalSemaphore sem;
};

semaphore_t thread_create_semaphore(void)
{
  semaphore_t sem = malloc(sizeof(struct semaphore_s));
  if(sem)
  {
    InitSemaphore(&sem->sem);
  }
  return sem;
}

void thread_dispose_semaphore(semaphore_t sem)
{
  free(sem);
}

void thread_lock_semaphore(semaphore_t sem)
{
  ObtainSemaphore(&sem->sem);
}

int thread_attempt_lock_semaphore(semaphore_t sem)
{
  return (int)AttemptSemaphore(&sem->sem);
}

void thread_unlock_semaphore(semaphore_t sem)
{
  ReleaseSemaphore(&sem->sem);
}
*/
