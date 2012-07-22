/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2012 YAM Open Source Team

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
#include "MailExport.h"
#include "MailImport.h"
#include "MethodStack.h"
#include "Requesters.h"
#include "Threads.h"

#include "mui/ClassesExtra.h"
#include "tcp/http.h"
#include "tcp/pop3.h"
#include "tcp/smtp.h"

#include "Debug.h"

#define MIN_THREADS       4

struct ThreadMessage;

struct Thread
{
  struct MinNode node;     // to make this a full Exec node
  struct Process *process; // the process pointer as returned by CreateNewProc()
  struct MsgPort *commandPort;
  struct MsgPort *timerPort;
  struct TimeRequest *timerRequest;
  LONG priority;           // the thread's priority
  LONG abortSignal;        // an allocated signal to abort the thread
  LONG wakeupSignal;       // an allocated signal to wakeup a sleeping thread
  char name[SIZE_LARGE];   // the thread's name
  BOOL working;            // are we currently working?
  BOOL aborted;            // have we been aborted?
  BOOL timerRunning;       // is the timer running?
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
  Object *object;                 // an object for which MUIM_ThreadFinished will be called
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

  if((clone = AllocSysObjectTags(ASOT_TAGLIST,
    ASOTAGS_NumEntries, numTags,
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
      result = SendMails((struct UserIdentityNode *)GetTagData(TT_SendMails_UserIdentity, (IPTR)NULL, msg->actionTags),
                         GetTagData(TT_SendMails_Mode, (IPTR)0, msg->actionTags));
    }
    break;

    case TA_ReceiveMails:
    {
      result = ReceiveMails((struct MailServerNode *)GetTagData(TT_ReceiveMails_MailServer, (IPTR)NULL, msg->actionTags),
                            GetTagData(TT_ReceiveMails_Flags, 0, msg->actionTags),
                            (struct DownloadResult *)GetTagData(TT_ReceiveMails_Result, (IPTR)NULL, msg->actionTags));
    }
    break;

    case TA_ImportMails:
    {
      result = ImportMails((const char *)GetTagData(TT_ImportMails_File, (IPTR)NULL, msg->actionTags),
                           (struct Folder *)GetTagData(TT_ImportMails_Folder, (IPTR)NULL, msg->actionTags),
                           GetTagData(TT_ImportMails_Flags, 0, msg->actionTags));
    }
    break;

    case TA_ExportMails:
    {
      result = ExportMails((const char *)GetTagData(TT_ExportMails_File, (IPTR)NULL, msg->actionTags),
                           (struct MailList *)GetTagData(TT_ExportMails_Mails, (IPTR)NULL, msg->actionTags),
                           GetTagData(TT_ExportMails_Flags, 0, msg->actionTags));
    }
    break;

    case TA_DownloadURL:
    {
      result = DownloadURL((const char *)GetTagData(TT_DownloadURL_Server, (IPTR)NULL, msg->actionTags),
                           (const char *)GetTagData(TT_DownloadURL_Request, (IPTR)NULL, msg->actionTags),
                           (const char *)GetTagData(TT_DownloadURL_Filename, (IPTR)NULL, msg->actionTags),
                           GetTagData(TT_DownloadURL_Flags, 0, msg->actionTags));
    }
    break;
  }

  D(DBF_THREAD, "thread '%s' finished action %ld, result %ld", msg->thread->name, msg->action, result);

  RETURN(result);
  return result;
}

///
/// ThreadEntry
// Entrypoint for a new thread
static SAVEDS void ThreadEntry(void)
{
  struct Process *proc;
  BOOL done = FALSE;
  struct ThreadMessage *msg;

  proc = (struct Process *)FindTask(NULL);

  // wait for the startup message to arrive at the thread's message port
  WaitPort(&proc->pr_MsgPort);
  if((msg = (struct ThreadMessage *)GetMsg(&proc->pr_MsgPort)) != NULL && msg->action == TA_Startup)
  {
    struct Thread *thread = msg->thread;
    BOOL initOk = FALSE;

    // we must not use any I/O function ahead of getting the startup message, thus
    // the ENTER() macro ist called here
    ENTER();

    D(DBF_THREAD, "thread %08lx '%s' got startup message", thread, thread->name);

    if((thread->abortSignal = AllocSignal(-1)) != -1)
    {
      if((thread->wakeupSignal = AllocSignal(-1)) != -1)
      {
        // allocate a separate message port to not interfere with standard I/O functions
        // which use proc->pr_MsgPort
      	if((thread->commandPort = AllocSysObjectTags(ASOT_PORT, TAG_DONE)) != NULL)
      	{
      	  // remember the thread pointer in the task's tc_UserData field
      	  // this will be used whenever the current thread needs to be obtained
          proc->pr_Task.tc_UserData = thread;
          msg->result = TRUE;
          initOk = TRUE;

          // change our initial priority of 1 back to 0
          SetTaskPri((struct Task *)proc, 0);
        }
        else
        {
          E(DBF_THREAD, "thread '%s' failed to allocate command port", thread->name);
          FreeSignal(thread->wakeupSignal);
          FreeSignal(thread->abortSignal);
          msg->result = FALSE;
        }
      }
      else
      {
        E(DBF_THREAD, "thread '%s' failed to allocate wakeup signal", thread->name);
        FreeSignal(thread->abortSignal);
        msg->result = FALSE;
      }
    }
    else
    {
      E(DBF_THREAD, "thread '%s' failed to allocate abort signal", thread->name);
      msg->result = FALSE;
    }

    // return the message to the sender
    ReplyMsg((struct Message *)msg);

    if(initOk == TRUE)
    {
      do
      {
        // wait for messages to arrive at the commnand message port
        WaitPort(thread->commandPort);

        // handle all pending messages
        while((msg = (struct ThreadMessage *)GetMsg(thread->commandPort)) != NULL)
        {
          D(DBF_THREAD, "got message %08lx, action %ld", msg, msg->action);

          switch(msg->action)
          {
            case TA_Shutdown:
            {
              D(DBF_THREAD, "thread '%s' got shutdown message", thread->name);
              // free all allocated resources and bail out of the loop
              if(thread->commandPort != NULL)
              {
                FreeSysObject(ASOT_PORT, thread->commandPort);
                thread->commandPort = NULL;
              }
              if(thread->abortSignal != -1)
              {
                FreeSignal(thread->abortSignal);
                thread->abortSignal = -1;
              }
              if(thread->wakeupSignal != -1)
              {
                FreeSignal(thread->wakeupSignal);
                thread->wakeupSignal = -1;
              }
              msg->result = TRUE;
              done = TRUE;
            }
            break;

            default:
            {
              // clear the abort signal before executing the desired action
              thread->aborted = FALSE;
              SetSignal(0UL, (1UL << thread->abortSignal) | (1UL << thread->wakeupSignal));
              msg->result = DoThreadMessage(msg);
            }
            break;
          }

          // return the message to the sender
          ReplyMsg((struct Message *)msg);

          // check if we should bail out completely
          if(done == TRUE)
            break;
        }
      }
      while(done == FALSE);
    }

    LEAVE();
  }
}

///
/// SleepThread
// put the current thread to sleep
BOOL SleepThread(void)
{
  ULONG abortMask;
  ULONG wakeupMask;
  ULONG signals;
  BOOL notAborted;

  ENTER();

  abortMask = (1UL << ThreadAbortSignal());
  wakeupMask = (1UL << ThreadWakeupSignal());

  D(DBF_THREAD, "thread '%s' is waiting for signals %08lx", ThreadName(), abortMask|wakeupMask);
  signals = Wait(abortMask|wakeupMask);
  D(DBF_THREAD, "thread '%s' got signals %08lx", ThreadName(), signals);

  notAborted = isFlagClear(signals, abortMask);

  RETURN(notAborted);
  return notAborted;
}

///
/// AbortThread
// signal a thread to abort the current action
void AbortThread(APTR thread, BOOL waitForTermination)
{
  struct Process *proc;
  ULONG sig;

  ENTER();

  if(thread == NULL || thread == G->mainThread)
  {
    D(DBF_THREAD, "aborting main thread");
    proc = G->mainThread;
    sig = SIGBREAKB_CTRL_C;
    waitForTermination = FALSE;
  }
  else
  {
    struct Thread *_thread = thread;

    D(DBF_THREAD, "aborting thread '%s'", _thread->name);
    _thread->aborted = TRUE;
    proc = _thread->process;
    sig = _thread->abortSignal;
  }

  Signal((struct Task *)proc, 1UL << sig);

  if(waitForTermination == TRUE)
  {
    struct Thread *_thread = thread;

    // now wait until the thread has finished its work
    do
    {
      MicroMainLoop();
      Delay(10);
    }
    while(_thread->working == TRUE);
  }

  LEAVE();
}

///
/// WakeupThread
// signal a thread to continue its work
void WakeupThread(APTR thread)
{
  struct Process *proc;
  ULONG sig;

  ENTER();

  if(thread == NULL || thread == G->mainThread)
  {
    D(DBF_THREAD, "waking up main thread");
    proc = G->mainThread;
    sig = SIGBREAKB_CTRL_E;
  }
  else
  {
    struct Thread *_thread = thread;

    D(DBF_THREAD, "waking up thread '%s'", _thread->name);
    proc = _thread->process;
    sig = _thread->wakeupSignal;
  }

  Signal((struct Task *)proc, 1UL << sig);

  LEAVE();
}

///
/// AbortWorkingThreads
// signal all still running threads to abort the current action
void AbortWorkingThreads(void)
{
  struct Node *node;

  ENTER();

  // always get the first node in the list, because later calls to HandleThreads()
  // will modify the list, thus iterating through the will not give the desired result
  while((node = GetHead((struct List *)&G->workingThreads)) != NULL)
  {
    struct ThreadNode *threadNode = (struct ThreadNode *)node;
    struct Thread *thread = threadNode->thread;

    // abort the working thread and wait until it finished its work
    AbortThread(thread, TRUE);
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

  // send out the shutdown message and wait for a reply
  D(DBF_THREAD, "sending shutdown message to thread '%s'", thread->name);
  PutMsg(thread->commandPort, (struct Message *)&startupMessage);
  Remove((struct Node *)WaitPort(G->threadPort));
  D(DBF_THREAD, "thread '%s' is dead now", thread->name);

  G->numberOfThreads--;

  LEAVE();
}

///
/// CreateThread
// Runs a given function in a newly created thread under the given name which
// in linked into a internal list.
static struct Thread *CreateThread(void)
{
  struct Thread *result = NULL;
  struct ThreadNode *threadNode;

  ENTER();

  if((threadNode = AllocSysObjectTags(ASOT_NODE,
    ASONODE_Size, sizeof(*threadNode),
    ASONODE_Min, TRUE,
    TAG_DONE)) != NULL)
  {
    struct Thread *thread;

    if((thread = AllocSysObjectTags(ASOT_NODE,
      ASONODE_Size, sizeof(*thread),
      ASONODE_Min, TRUE,
      TAG_DONE)) != NULL)
    {
      threadNode->thread = thread;

      memset(thread, 0, sizeof(*thread));
      thread->abortSignal = -1;
      thread->wakeupSignal = -1;
      snprintf(thread->name, sizeof(thread->name), "YAM thread [%d]", (int)G->threadCounter+1);

      if((thread->process = CreateNewProcTags(NP_Entry,       ThreadEntry, // entry function
                                              NP_StackSize,   8192,        // stack size
                                              NP_Name,        thread->name,
                                              NP_Priority,    1,
                                              NP_Input,       Input(),
                                              NP_CloseInput,  FALSE,
                                              NP_Output,      Output(),
                                              NP_CloseOutput, FALSE,
                                              #if defined(__amigaos4__)
                                              NP_Error,       ErrorOutput(),
                                              NP_CloseError,  FALSE,
                                              NP_Child,       TRUE,
                                              #elif defined(__MORPHOS__)
                                              NP_CodeType,    MACHINE_PPC,
                                              #endif
                                              TAG_DONE)) != NULL)
      {
        struct Message *msg;

        // prepare the startup message
        memset(&startupMessage, 0, sizeof(startupMessage));
        startupMessage.msg.mn_ReplyPort = G->threadPort;
        startupMessage.msg.mn_Length = sizeof(startupMessage);
        startupMessage.action = TA_Startup;
        startupMessage.threadNode = threadNode;
        startupMessage.thread = thread;

        // send out the startup message
        D(DBF_THREAD, "thread 0x%08lx '%s' started, sending startup message", thread, thread->name);
        PutMsg(&thread->process->pr_MsgPort, (struct Message *)&startupMessage);

        // now wait for a reply
        do
        {
          D(DBF_THREAD, "waiting for startup message to be replied");

          msg = WaitPort(G->threadPort);
          D(DBF_THREAD, "got message %08lx, expected %08lx", msg, &startupMessage);

          // make sure we got our startup message back
          if(&startupMessage == (struct ThreadMessage *)msg)
          {
            // WaitPort() doesn't remove the message, we have to do that ourself
            Remove((struct Node *)msg);

            // check wether the thread function returned something valid
            if(startupMessage.result != 0)
            {
              // increase the thread counter
              G->numberOfThreads++;
              G->threadCounter++;

              AddTail((struct List *)&G->idleThreads, (struct Node *)threadNode);

              result = thread;

              // we got the message back, get out of here
              break;
            }
            else
            {
              E(DBF_THREAD, "thread 0x%08lx '%s' failed to initialize", thread, thread->name);
              ShutdownThread(threadNode);
            }
          }
          else
          {
            // this was not the startup message
            // handle the message of a different thread
            HandleThreads(FALSE);
          }
        }
        while(msg != NULL);
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
/// InitThreads
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
/// CleanupThreads
// cleanup the whole thread system - abort eventually active threads and
// wait for them to finish properly.
void CleanupThreads(void)
{
  ENTER();

  if(G->threadPort != NULL)
  {
    BOOL tryAgain = TRUE;

    // send an abort signal and a shutdown message to each still existing idle thread
    AbortWorkingThreads();

    // send a shutdown message to each still existing idle thread
    PurgeIdleThreads(TRUE);

    // handle possible working->idle transitions of threads in case
    // a thread was closed during the shutdown. This might happen if
    // there were zombie files which got closed by shutting down a
    // MIME viewer for example.
    MicroMainLoop();

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
          MicroMainLoop();
        }
        else
        {
          // don't wait anymore
          tryAgain = FALSE;
        }
      }

      // now send a shutdown message to each still existing idle thread
      PurgeIdleThreads(TRUE);
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
void HandleThreads(BOOL handleAll)
{
  struct Message *msg;

  ENTER();

  while((msg = GetMsg(G->threadPort)) != NULL)
  {
    struct ThreadMessage *tmsg = (struct ThreadMessage *)msg;

    D(DBF_THREAD, "thread '%s' finished action %ld, result %ld", tmsg->thread->name, tmsg->action, tmsg->result);

    // a little paranoia check, we must not free the startup message
    if(tmsg->action != TA_Startup)
    {
      if(tmsg->object != NULL)
      {
        D(DBF_THREAD, "sending MUIM_ThreadFinished to object %08lx", tmsg->object);
        DoMethod(tmsg->object, MUIM_ThreadFinished, tmsg->action, tmsg->result, tmsg->actionTags);
      }

      // remove the thread from the working list and put it back into the idle list
      Remove((struct Node *)tmsg->threadNode);
      tmsg->threadNode->thread->working = FALSE;
      AddTail((struct List *)&G->idleThreads, (struct Node *)tmsg->threadNode);

      // change the thread's priority back to zero
      if(tmsg->thread->priority != 0)
        SetTaskPri((struct Task *)tmsg->thread->process, 0);

      // free the parameters
      FreeThreadTags(tmsg->actionTags);

      // finally dispose the message
      FreeSysObject(ASOT_MESSAGE, tmsg);
    }

    // bail out if we are told to handle one message only
    if(handleAll == FALSE)
      break;
  }

  LEAVE();
}

///
/// PurgeIdleThreads
// terminate all idle threads to save memory
void PurgeIdleThreads(const BOOL purgeAll)
{
  struct Node *node;
  ULONG limit;

  ENTER();

  // purge either all idle threads or leave at least the minimum number of threads alive
  if(purgeAll == TRUE)
    limit = 0;
  else
    limit = MIN_THREADS;

  while(G->numberOfThreads > limit && (node = RemHead((struct List *)&G->idleThreads)) != NULL)
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
APTR VARARGS68K DoAction(Object *obj, const enum ThreadAction action, ...)
{
  APTR success = NULL;
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

    if((msg = AllocSysObjectTags(ASOT_MESSAGE,
      ASOMSG_Size, sizeof(*msg),
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

        // set up the action message
        msg->action = action;
        msg->threadNode = threadNode;
        msg->thread = thread;
        msg->object = obj;

        // raise the thread's priority if this is requested
        if((pri = GetTagData(TT_Priority, 0, msg->actionTags)) != 0)
        {
          SetTaskPri((struct Task *)thread->process, pri);
          thread->priority = pri;
        }

        // send the message to the thread
        PutMsg(thread->commandPort, (struct Message *)msg);

        // remove the thread from the idle list and put it into the working list
        Remove((struct Node *)threadNode);
        threadNode->thread->working = TRUE;
        AddTail((struct List *)&G->workingThreads, (struct Node *)threadNode);

        // return the executing thread on success
        success = thread;
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
  ULONG sigBit;
  struct Process *me;

  ENTER();

  me = (struct Process *)FindTask(NULL);

  if(me == G->mainThread)
  {
    sigBit = SIGBREAKB_CTRL_C;
  }
  else
  {
    struct Thread *thread = (struct Thread *)me->pr_Task.tc_UserData;

    sigBit = thread->abortSignal;
  }

  RETURN(sigBit);
  return sigBit;
}

///
/// ThreadWakeupSignal
// get the wakeup signal of the current thread, this is CTRL-E for the main thread
LONG ThreadWakeupSignal(void)
{
  ULONG sigBit;
  struct Process *me;

  ENTER();

  me = (struct Process *)FindTask(NULL);

  if(me == G->mainThread)
  {
    sigBit = SIGBREAKB_CTRL_E;
  }
  else
  {
    struct Thread *thread = (struct Thread *)me->pr_Task.tc_UserData;

    sigBit = thread->wakeupSignal;
  }

  RETURN(sigBit);
  return sigBit;
}

///
/// ThreadTimerSignal
// get the timer signal of the current thread
LONG ThreadTimerSignal(void)
{
  ULONG sigBit;
  struct Process *me;

  ENTER();

  me = (struct Process *)FindTask(NULL);

  if(me == G->mainThread)
  {
    sigBit = G->timerData.port->mp_SigBit;
  }
  else
  {
    struct Thread *thread = (struct Thread *)me->pr_Task.tc_UserData;

    if(thread->timerRequest != NULL)
      sigBit = thread->timerPort->mp_SigBit;
    else
      sigBit = -1;
  }

  RETURN(sigBit);
  return sigBit;
}

///
/// ThreadWasAborted
// return the current thread's abort state
BOOL ThreadWasAborted(void)
{
  BOOL aborted;
  struct Process *me;

  ENTER();

  me = (struct Process *)FindTask(NULL);

  if(me == G->mainThread)
  {
    // the main thread can never be aborted
    aborted = FALSE;
  }
  else
  {
    struct Thread *thread = (struct Thread *)me->pr_Task.tc_UserData;

    aborted = thread->aborted;
  }

  RETURN(aborted);
  return aborted;
}

///
/// ThreadName
// return the current thread's name
const char *ThreadName(void)
{
  const char *name;

  ENTER();

  if(IsMainThread() == TRUE)
    name = "main";
  else
  {
    struct Thread *thread = CurrentThread();

    name = thread->name;
  }

  RETURN(name);
  return name;
}

///
/// InitThreadTimer
// initalize a timer for the calling thread
BOOL InitThreadTimer(void)
{
  BOOL success = FALSE;
  struct Thread *thread = CurrentThread();

  ENTER();

  if(thread->timerRequest == NULL)
  {
    if((thread->timerPort = AllocSysObjectTags(ASOT_PORT, TAG_DONE)) != NULL)
    {
      if((thread->timerRequest = AllocSysObjectTags(ASOT_IOREQUEST,
        ASOIOR_Size, sizeof(*thread->timerRequest),
        ASOIOR_ReplyPort, (IPTR)thread->timerPort,
        TAG_DONE)) != NULL)
      {
        if(OpenDevice(TIMERNAME, UNIT_VBLANK, (struct IORequest *)thread->timerRequest, 0L) == 0)
        {
          thread->timerRunning = FALSE;
          success = TRUE;
        }
      }
    }
  }

  if(success == FALSE)
  {
    if(thread->timerRequest != NULL)
    {
      FreeSysObject(ASOT_IOREQUEST, thread->timerRequest);
      thread->timerRequest = NULL;
    }
    if(thread->timerPort != NULL)
    {
      FreeSysObject(ASOT_PORT, thread->timerPort);
      thread->timerPort = NULL;
    }
  }

  RETURN(success);
  return success;
}

///
/// CleanupThreadTimer
// delete a previously initialized thread timer
void CleanupThreadTimer(void)
{
  struct Thread *thread = CurrentThread();

  ENTER();

  if(thread->timerRequest != NULL)
  {
    StopThreadTimer();

    if(thread->timerRequest != NULL)
    {
      CloseDevice((struct IORequest *)thread->timerRequest);
      FreeSysObject(ASOT_IOREQUEST, thread->timerRequest);
      thread->timerRequest = NULL;
    }
    if(thread->timerPort != NULL)
    {
      FreeSysObject(ASOT_PORT, thread->timerPort);
      thread->timerPort = NULL;
    }
  }

  LEAVE();
}

///
/// StartThreadTimer
void StartThreadTimer(ULONG seconds, ULONG micros)
{
  struct Thread *thread = CurrentThread();

  ENTER();

  if(thread->timerRequest != NULL && thread->timerRunning == FALSE)
  {
    if(seconds > 0 || micros > 0)
    {
      // issue a new timerequest
      thread->timerRequest->Request.io_Command = TR_ADDREQUEST;
      thread->timerRequest->Time.Seconds       = seconds;
      thread->timerRequest->Time.Microseconds  = micros;
      SendIO((struct IORequest *)thread->timerRequest);

      thread->timerRunning = TRUE;
    }
  }

  LEAVE();
}

///
/// StopThreadTimer
void StopThreadTimer(void)
{
  struct Thread *thread = CurrentThread();

  ENTER();

  if(thread->timerRequest != NULL && thread->timerRunning == TRUE)
  {
    if(CheckIO((struct IORequest *)thread->timerRequest) == NULL)
      AbortIO((struct IORequest *)thread->timerRequest);

    WaitIO((struct IORequest *)thread->timerRequest);

    thread->timerRunning = FALSE;
  }

  LEAVE();
}

///
