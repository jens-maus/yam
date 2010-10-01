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
 * partly a modified version of the thread implementation in Ambient.
 * In fact it was highly inspired by the implementation of Ambient.
 *
 * Thanks to the authors of Ambient!
 *
 * Details about Ambient can be found here:
 * http://sourceforge.net/projects/morphosambient/
 *
 * Ambient's thread implementation can be found here:
 * http://morphosambient.cvs.sourceforge.net/viewvc/morphosambient/ambient/threads.c?view=log
 *
 */

#include <string.h>
#include <stdlib.h>

#include <clib/alib_protos.h>

#include <dos/dostags.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/utility.h>

#if defined(__amigaos4__)
#include <exec/exectags.h>
#endif

#include "YAM.h"
#include "YAM_utilities.h"

#include "SDI_stdarg.h"

#include "extrasrc.h"

#include "Locale.h"
#include "MailImport.h"
#include "Requesters.h"
#include "Threads.h"

#include "mui/Classes.h"
#include "tcp/smtp.h"

#include "Debug.h"

#define MIN_THREADS       4

struct Thread
{
  struct MinNode node;     // to make this a full Exec node
  struct Process *process; // the process pointer as returned by CreateNewProc()
  LONG priority;           // the thread's priority
  LONG abortSignal;        // an allocated signal to abort the thread
  LONG wakeupSignal;       // an allocated signal to wakeup a sleeping thread
  char name[SIZE_LARGE];   // the thread's name
};

// a thread node which will be inserted to our
// global subthread list.
struct ThreadNode
{
  struct MinNode node;
  struct Thread *thread;
};

struct ThreadMessage
{
  struct Message msg;             // to make ThreadMessage a full Exec message
  enum ThreadAction action;       // the action the thread should perform
  LONG result;                    // when the thread is finished the result is stored here
  LONG priority;                  // the task priority the thread should be set to
  struct TagItem *actionTags;     // the parameters for the action
  struct ThreadNode *threadNode;  // link to the thread's node
  struct Thread *thread;          // link to the thread itself
};

// we use a global message for startup/shutdown, because
// starting up/shutting down better is not subject to low
// memory situations where no further message object can
// be dynamically allocated. This would leave us in a
// deadlock situation.
static struct ThreadMessage startupMessage;

/// FreeThreadTags
// free a previously cloned tag list, respecting duplicated strings
static void FreeThreadTags(struct TagItem *tags)
{
  struct TagItem *tstate = (struct TagItem *)tags;
  struct TagItem *tag;

  ENTER();

  while((tag = NextTagItem((APTR)&tstate)) != NULL)
  {
    // free possible string tags
    if(isFlagSet(tag->ti_Tag, TAG_STRING))
      free((void *)tag->ti_Data);
  }

  FreeSysObject(ASOT_TAGLIST, tags);

  LEAVE();
}

///
/// CloneThreadTags
// clone the given tag items, duplicate possible strings
static struct TagItem *CloneThreadTags(const struct TagItem *tags)
{
  struct TagItem *tstate;
  struct TagItem *clone;
  ULONG numTags = 0;

  ENTER();

  tstate = (struct TagItem *)tags;
  // count one additional tag for the terminating TAG_DONE
  numTags = 1;
  while(NextTagItem((APTR)&tstate) != NULL)
    numTags++;

  if((clone = AllocSysObjectTags(ASOT_TAGLIST, ASOTAGS_NumEntries, numTags,
                                               TAG_DONE)) != NULL)
  {
    struct TagItem *tag;
    struct TagItem *ctag = clone;

    tstate = (struct TagItem *)tags;
    while((tag = NextTagItem((APTR)&tstate)) != NULL)
    {
      // the tag remains the same
      ctag->ti_Tag = tag->ti_Tag;
      ctag->ti_Data = tag->ti_Data;

      // is this a special string tag?
      if(isFlagSet(tag->ti_Tag, TAG_STRING) && tag->ti_Data != (IPTR)NULL)
      {
        // duplicate the string
        if((ctag->ti_Data = (IPTR)strdup((STRPTR)tag->ti_Data)) == (IPTR)NULL)
        {
          // no memory for the string copy
          // terminate the list at the current item and free it
          ctag->ti_Tag = TAG_DONE;
          FreeThreadTags(clone);
          clone = NULL;
          break;
        }
      }

      ctag++;
    }

    // check if the clone was successfully created
    if(clone != NULL)
    {
      // then terminate the list
      ctag->ti_Tag = TAG_DONE;
    }
  }

  RETURN(clone);
  return clone;
}

///
/// DoThreadMessage
// perform the requested action
static LONG DoThreadMessage(struct ThreadMessage *msg)
{
  LONG result = -1;

  ENTER();

  D(DBF_THREAD, "thread '%s' performs action %ld", msg->thread->name, msg->action);

  switch(msg->action)
  {
    // TA_Startup/TA_Shutdown is handled in ThreadEntry() already.
    case TA_Startup:
    case TA_Shutdown:
      // nothing
    break;

    case TA_LaunchCommand:
    {
      result = LaunchCommand((const char *)GetTagData(TT_LaunchCommand_Command, (IPTR)NULL, msg->actionTags),
                             FALSE,
                             GetTagData(TT_LaunchCommand_Output, OUT_NIL, msg->actionTags));
    }
    break;

    case TA_FlushSpamTrainingData:
    {
      BayesFilterFlushTrainingData();
      result = 0;
    }
    break;

    case TA_SendMails:
    {
      result = SendMails((struct MailServerNode *)GetTagData(TT_SendMails_MailServer, (IPTR)NULL, msg->actionTags),
                         (struct MailList *)GetTagData(TT_SendMails_Mails, (IPTR)NULL, msg->actionTags),
                         GetTagData(TT_SendMails_Mode, (IPTR)0, msg->actionTags));
    }
    break;

    case TA_ImportMails:
    {
      result = ImportMails((const char *)GetTagData(TT_ImportMails_File, (IPTR)NULL, msg->actionTags),
                           (struct Folder *)GetTagData(TT_ImportMails_Folder, (IPTR)NULL, msg->actionTags),
                           GetTagData(TT_ImportMails_Quiet, FALSE, msg->actionTags),
                           GetTagData(TT_ImportMails_Wait, FALSE, msg->actionTags));
    }
    break;
  }

  D(DBF_THREAD, "thread '%s' finished action %ld, result %ld", msg->thread->name, msg->action, result);

  RETURN(result);
  return result;
}

///
/// ThreadEntry()
// Entrypoint for a new thread
static SAVEDS void ThreadEntry(void)
{
  struct Process *proc;
  BOOL done = FALSE;
  struct Thread *thread = NULL;

  ENTER();

  proc = (struct Process*)FindTask(NULL);
  D(DBF_THREAD, "thread 0x%08lx '%s' waiting for startup message", proc, SafeStr(proc->pr_Task.tc_Node.ln_Name));

  do
  {
    struct ThreadMessage *msg;

    Wait(1 << proc->pr_MsgPort.mp_SigBit);

    if((msg = (struct ThreadMessage *)GetMsg(&proc->pr_MsgPort)) != NULL)
    {
      switch(msg->action)
      {
        case TA_Startup:
        {
          D(DBF_THREAD, "thread '%s' got startup message", msg->thread->name);
          if((msg->thread->abortSignal = AllocSignal(-1)) != -1)
          {
            if((msg->thread->wakeupSignal = AllocSignal(-1)) != -1)
            {
              proc->pr_Task.tc_UserData = msg->thread;
              msg->result = TRUE;
              thread = msg->thread;
            }
            else
            {
              E(DBF_THREAD, "thread '%s' failed to allocate wakeup signal", msg->thread->name);
              FreeSignal(msg->thread->abortSignal);
              msg->result = FALSE;
            }
          }
          else
          {
            E(DBF_THREAD, "thread '%s' failed to allocate abort signal", msg->thread->name);
            msg->result = FALSE;
          }
        }
        break;

        case TA_Shutdown:
        {
          D(DBF_THREAD, "thread '%s' got shutdown message", msg->thread->name);
          if(msg->thread->abortSignal != -1)
            FreeSignal(msg->thread->abortSignal);
          if(msg->thread->wakeupSignal != -1)
            FreeSignal(msg->thread->wakeupSignal);
          msg->result = TRUE;
          done = TRUE;
        }
        break;

        default:
        {
          // clear the abort signal before executing the desired action
          SetSignal(0UL, (1UL << thread->abortSignal) | (1UL << thread->wakeupSignal));
          msg->result = DoThreadMessage(msg);
        }
        break;
      }

      ReplyMsg((struct Message *)msg);
    }
  }
  while(done == FALSE);

  LEAVE();
}

///
/// AbortThread
// signal a thread to abort the current action
static void AbortThread(struct Thread *thread)
{
  ENTER();

  if(thread->abortSignal != -1)
    Signal((struct Task *)thread->process, 1UL << thread->abortSignal);

  LEAVE();
}

///
/// SleepThread
// put the current thread to sleep
BOOL SleepThread(void)
{
  struct Thread *thread = CurrentThread();
  ULONG abortMask;
  ULONG wakeupMask;
  ULONG sigs;
  BOOL notAborted;

  ENTER();

  if(IsMainThread() == TRUE)
  {
    abortMask = SIGBREAKF_CTRL_C;
    wakeupMask = SIGBREAKF_CTRL_E;
  }
  else
  {
    abortMask = 1UL << thread->abortSignal;
    wakeupMask = 1UL << thread->wakeupSignal;
  }

  sigs = Wait(abortMask | wakeupMask);
  notAborted = isFlagClear(sigs, abortMask);

  RETURN(notAborted);
  return notAborted;
}

///
/// WakeupThread
// signal a thread to continue its work
void WakeupThread(APTR thread)
{
  struct Thread *_thread = thread;

  ENTER();

  if(_thread != NULL && _thread->wakeupSignal != -1)
    Signal((struct Task *)_thread->process, 1UL << _thread->wakeupSignal);

  LEAVE();
}

///
/// AbortRunningThreads
// signal all still running threads to abort the current action
static void AbortRunningThreads(void)
{
  struct Node *node;

  ENTER();

  IterateList(&G->workingThreads, node)
  {
    struct ThreadNode *threadNode = (struct ThreadNode *)node;

    AbortThread(threadNode->thread);
  }

  LEAVE();
}

///
/// ShutdownThread
// terminate a single thread
static void ShutdownThread(struct ThreadNode *threadNode)
{
  struct Thread *thread = threadNode->thread;

  ENTER();

  // prepare the shutdown message
  memset(&startupMessage, 0, sizeof(startupMessage));
  startupMessage.msg.mn_ReplyPort = G->threadPort;
  startupMessage.msg.mn_Length = sizeof(startupMessage);
  startupMessage.action = TA_Shutdown;
  startupMessage.threadNode = threadNode;
  startupMessage.thread = thread;

  // send out the startup message and wait for a reply
  D(DBF_THREAD, "sending shutdown message to thread '%s'", thread->name);
  PutMsg(&thread->process->pr_MsgPort, (struct Message *)&startupMessage);
  Remove((struct Node *)WaitPort(G->threadPort));

  LEAVE();
}

///
/// CreateThread()
// Runs a given function in a newly created thread under the given name which
// in linked into a internal list.
static struct Thread *CreateThread(void)
{
  struct Thread *result = NULL;
  struct ThreadNode *threadNode;

  ENTER();

  if((threadNode = AllocSysObjectTags(ASOT_NODE, ASONODE_Size, sizeof(*threadNode),
                                                 ASONODE_Min, TRUE,
                                                 TAG_DONE)) != NULL)
  {
    struct Thread *thread;

    if((thread = AllocSysObjectTags(ASOT_NODE, ASONODE_Size, sizeof(*thread),
                                               ASONODE_Min, TRUE,
                                               TAG_DONE)) != NULL)
    {
      threadNode->thread = thread;

      snprintf(thread->name, sizeof(thread->name), "YAM thread [%d]", (int)G->numThreads+1);

      if((thread->process = CreateNewProcTags(NP_Entry,       ThreadEntry, // entry function
                                              NP_StackSize,   8192,        // stack size
                                              NP_Name,        thread->name,
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
        thread->abortSignal = -1;

        // prepare the startup message
        memset(&startupMessage, 0, sizeof(startupMessage));
        startupMessage.msg.mn_ReplyPort = G->threadPort;
        startupMessage.msg.mn_Length = sizeof(startupMessage);
        startupMessage.action = TA_Startup;
        startupMessage.threadNode = threadNode;
        startupMessage.thread = thread;

        // send out the startup message and wait for a reply
        D(DBF_THREAD, "thread 0x%08lx '%s' started, sending startup message", thread, thread->name);
        PutMsg(&thread->process->pr_MsgPort, (struct Message *)&startupMessage);
        Remove((struct Node *)WaitPort(G->threadPort));

        // check wether the thread function returned something valid
        if(startupMessage.result != 0)
        {
          // increase the thread counter
          G->numThreads++;

          AddTail((struct List *)&G->idleThreads, (struct Node *)threadNode);

          result = thread;
        }
        else
        {
          E(DBF_THREAD, "thread 0x%08lx '%s' failed to initialize", thread, thread->name);
          ShutdownThread(threadNode);
        }
      }

      if(result == NULL)
        FreeSysObject(ASOT_NODE, thread);
    }

    if(result == NULL)
      FreeSysObject(ASOT_NODE, threadNode);
  }

  RETURN(result);
  return result;
}

///
/*** Thread system init/cleanup functions ***/
/// InitThreads()
// initializes the thread system
BOOL InitThreads(void)
{
  BOOL result = FALSE;

  ENTER();

  // initialize the thread lists
  NewMinList(&G->idleThreads);
  NewMinList(&G->workingThreads);
  G->mainThread = (struct Process *)FindTask(NULL);

  if((G->threadPort = AllocSysObjectTags(ASOT_PORT, TAG_DONE)) != NULL)
  {
    ULONG i;

    result = TRUE;

    // set up a pool of idle threads
    for(i = 0; i < MIN_THREADS; i++)
    {
      if(CreateThread() == NULL)
      {
        result = FALSE;
        break;
      }
    }
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

  if(G->threadPort != NULL)
  {
    BOOL tryAgain = TRUE;

    // send an abort signal and a shutdown message to each still existing idle thread
    AbortRunningThreads();

    // send a shutdown message to each still existing idle thread
    PurgeIdleThreads();

    // handle possible working->idle transitions of threads in case
    // a thread was closed during the shutdown. This might happen if
    // there were zombie files which got closed by shutting down a
    // MIME viewer for example.
    HandleThreads();

    do
    {
      // first check if there are still working theads
      if(IsMinListEmpty(&G->workingThreads) == FALSE)
      {
        // there are still working threads, ask the user what to do
        if(MUI_Request(G->App, NULL, 0L, tr(MSG_THREAD_EXIT_WARNING_TITLE),
                                         tr(MSG_THREAD_EXIT_WARNING_BT),
                                         tr(MSG_THREAD_EXIT_WARNING)) != 0)
        {
          // the user wanted to try again, so let's handle possible
          // working->idle transitions
          HandleThreads();
        }
        else
        {
          // don't wait anymore
          tryAgain = FALSE;
        }
      }

      // now send a shutdown message to each still existing idle thread
      PurgeIdleThreads();
    }
    while(tryAgain == TRUE && IsMinListEmpty(&G->workingThreads) == FALSE);

    FreeSysObject(ASOT_PORT, G->threadPort);
    G->threadPort = NULL;
  }

  LEAVE();
}

///
/// HandleThreads
// handle a message returned by one of the threads
void HandleThreads(void)
{
  struct Message *msg;

  ENTER();

  while((msg = GetMsg(G->threadPort)) != NULL)
  {
    struct ThreadMessage *tmsg = (struct ThreadMessage *)msg;

    // remove the thread from the working list and put it back into the idle list
    Remove((struct Node *)tmsg->threadNode);
    AddTail((struct List *)&G->idleThreads, (struct Node *)tmsg->threadNode);

    // change the thread's priority back to zero
    if(tmsg->thread->priority != 0)
      SetTaskPri((struct Task *)tmsg->thread->process, 0);

    // free the parameters
    FreeThreadTags(tmsg->actionTags);

    // finally dispose the message
    FreeSysObject(ASOT_MESSAGE, tmsg);
  }

  LEAVE();
}

///
/// PurgeIdleThreads
// terminate all idle threads to save memory
void PurgeIdleThreads(void)
{
  struct Node *node;

  ENTER();

  while((node = RemHead((struct List *)&G->idleThreads)) != NULL)
  {
    struct ThreadNode *threadNode = (struct ThreadNode *)node;

    ShutdownThread(threadNode);
    FreeSysObject(ASOT_NODE, threadNode->thread);
    FreeSysObject(ASOT_NODE, threadNode);
  }

  LEAVE();
}

///
/// DoAction
//
BOOL VARARGS68K DoAction(const enum ThreadAction action, ...)
{
  BOOL success = FALSE;
  struct Node *node;

  ENTER();

  // try to get an idle thread
  // if there is none left we will create a new one
  while((node = GetHead((struct List *)&G->idleThreads)) == NULL)
  {
    if(CreateThread() == NULL)
      break;
  }

  if(node != NULL)
  {
    struct ThreadNode *threadNode = (struct ThreadNode *)node;
    struct Thread *thread = threadNode->thread;
    struct ThreadMessage *msg;

    D(DBF_THREAD, "found idle task '%s'", thread->name);

    if((msg = AllocSysObjectTags(ASOT_MESSAGE, ASOMSG_Size, sizeof(*msg),
                                               ASOMSG_ReplyPort, (IPTR)G->threadPort,
                                               TAG_DONE)) != NULL)
    {
      VA_LIST args;

      VA_START(args, action);
      msg->actionTags = CloneThreadTags((const struct TagItem *)VA_ARG(args, IPTR));
      VA_END(args);

      SHOWVALUE(DBF_THREAD, msg->actionTags);

      if(msg->actionTags != NULL)
      {
        LONG pri;

        msg->action = action;
        msg->threadNode = threadNode;
        msg->thread = thread;

        // raise the thread's priority if this is requested
        if((pri = GetTagData(TT_Priority, 0, msg->actionTags)) != 0)
        {
          SetTaskPri((struct Task *)thread->process, pri);
          thread->priority = pri;
        }

        // send the message to the thread
        PutMsg(&thread->process->pr_MsgPort, (struct Message *)msg);

        // remove the thread from the idle list and put it into the working list
        Remove((struct Node *)threadNode);
        AddTail((struct List *)&G->workingThreads, (struct Node *)threadNode);

        success = TRUE;
      }
      else
        FreeSysObject(ASOT_MESSAGE, msg);
    }
  }

  RETURN(success);
  return success;
}

///
/// IsMainThread
// check wether we are running in the context of the main thread
BOOL IsMainThread(void)
{
  BOOL isMainThread = FALSE;

  ENTER();

  if((struct Process *)FindTask(NULL) == G->mainThread)
    isMainThread = TRUE;

  RETURN(isMainThread);
  return isMainThread;
}

///
/// CurrentThread
// get the current thread
APTR CurrentThread(void)
{
  APTR thread;

  ENTER();

  if(IsMainThread() == TRUE)
    thread = G->mainThread;
  else
    thread = ((struct Process *)FindTask(NULL))->pr_Task.tc_UserData;

  RETURN(thread);
  return thread;
}

///
/// ThreadAbortSignal
// get the abort signal of the current thread, this is CTRL-C for the main thread
LONG ThreadAbortSignal(void)
{
  ULONG signal;
  struct Process *me;

  ENTER();

  me = (struct Process *)FindTask(NULL);

  if(me == G->mainThread)
  {
    signal = SIGBREAKB_CTRL_C;
  }
  else
  {
    struct Thread *thread = (struct Thread *)me->pr_Task.tc_UserData;

    signal = thread->abortSignal;
  }

  RETURN(signal);
  return signal;
}

///
