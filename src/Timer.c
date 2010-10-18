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

 $Id$

***************************************************************************/

#if !defined(__amigaos4__)
#include <clib/alib_protos.h>
#endif

#include <proto/exec.h>
#include <proto/timer.h>

#include <mui/NList_mcc.h>
#include <mui/TextEditor_mcc.h>

#include "YAM.h"
#include "YAM_addressbook.h"
#include "YAM_config.h"
#include "YAM_mainFolder.h"
#include "YAM_write.h"

#include "extrasrc.h"

#include "Locale.h"
#include "MUIObjects.h"
#include "Threads.h"
#include "Timer.h"

#include "mui/Classes.h"
#include "tcp/pop3.h"

#include "Debug.h"

/*** Timer processing function ***/
/// PrepareTimer
//  prepares a timer for being started with StartTimer() later on
void PrepareTimer(const enum Timer tid, const int seconds, const int micros)
{
  ENTER();

  if(seconds > 0 || micros > 0)
  {
    struct TRequest *timer = &G->timerData.timer[tid];

    if(timer->isRunning == FALSE && timer->isPrepared == FALSE)
    {
      struct TimeRequest *tr = timer->tr;

      // issue a new timerequest
      tr->Request.io_Command  = TR_ADDREQUEST;
      tr->Time.Seconds        = seconds;
      tr->Time.Microseconds   = micros;

      // remember the remaining time
      timer->remainingTime.Seconds = seconds;
      timer->remainingTime.Microseconds = micros;

      // flag the timer to be prepared to get fired later on
      timer->isPrepared = TRUE;
    }
    else
      W(DBF_TIMER, "timer[%ld]: already running/prepared", tid);
  }
  else
    W(DBF_TIMER, "timer[%ld]: secs and micros are zero, no prepare required", tid);

  LEAVE();
}

///
/// StartTimer
//  Start a delay depending on the time specified
void StartTimer(const enum Timer tid)
{
  struct TRequest *timer = &G->timerData.timer[tid];

  ENTER();

  if(timer->isRunning == FALSE && timer->isPrepared == TRUE)
  {
    #if defined(DEBUG)
    char dateString[64];

    DateStamp2String(dateString, sizeof(dateString), NULL, DSS_DATETIME, TZC_NONE);

    D(DBF_TIMER, "timer[%ld]: started @ %s to finish in %ld'%ld secs", tid,
                                                                         dateString,
                                                                         timer->tr->Time.Seconds,
                                                                         timer->tr->Time.Microseconds);
    #endif

    // fire the timer by doing a SendIO()
    SendIO(&timer->tr->Request);

    // remember the start time
    GetSysTime(TIMEVAL(&timer->startTime));

    // signal that our timer is running
    timer->isRunning = TRUE;
    timer->isPrepared = FALSE;
  }
  else
    W(DBF_TIMER, "timer[%ld]: either already running or not prepared to get fired", tid);

  LEAVE();
}

///
/// StopTimer
//  Stop a currently running TimerIO request
//  Please note that this function may NOT be used in the eventloop after having received
//  a timer with GetMsg because CheckIO and friends are not defined to work there correctly.
void StopTimer(const enum Timer tid)
{
  struct TRequest *timer = &G->timerData.timer[tid];

  ENTER();

  // check if we have a already issued ioreq running
  if(timer->isRunning == TRUE)
  {
    struct IORequest *ioreq = &timer->tr->Request;

    if(ioreq->io_Command != 0)
    {
      if(CheckIO(ioreq) == NULL)
        AbortIO(ioreq);

      WaitIO(ioreq);

      // there is no time left for this request
      timer->remainingTime.Seconds = 0;
      timer->remainingTime.Microseconds = 0;

      // make sure the timer is signalled to be NOT running
      timer->isRunning = FALSE;
      timer->isPrepared = FALSE;

      D(DBF_TIMER, "timer[%ld]: successfully stopped", tid);
    }
    else
      E(DBF_TIMER, "timer[%ld]: is invalid and can't be stopped", tid);
  }
  else
    W(DBF_TIMER, "timer[%ld]: already stopped", tid);

  LEAVE();
}

///
/// PauseTimer
//  Pause a currently running Time request
void PauseTimer(const enum Timer tid)
{
  struct TRequest *timer = &G->timerData.timer[tid];

  ENTER();

  // check if we have a already issued ioreq running
  if(timer->isRunning == TRUE && timer->isPaused == FALSE)
  {
    struct IORequest *ioreq = &timer->tr->Request;

    if(ioreq->io_Command != 0)
    {
      struct TimeVal stopTime;

      if(CheckIO(ioreq) == NULL)
        AbortIO(ioreq);

      WaitIO(ioreq);

      // calculate the remaining time
      GetSysTime(TIMEVAL(&stopTime));
      SubTime(TIMEVAL(&stopTime), TIMEVAL(&timer->startTime));
      SubTime(TIMEVAL(&timer->remainingTime), TIMEVAL(&stopTime));

      // make sure the timer is signalled to be NOT running
      timer->isRunning = FALSE;
      timer->isPaused = TRUE;

      #if defined(DEBUG)
      {
        char dateString[64];
        DateStamp2String(dateString, sizeof(dateString), NULL, DSS_DATETIME, TZC_NONE);
        D(DBF_TIMER, "timer[%ld]: paused @ %s with %ld'%ld secs remaining", tid,
                                                                              dateString,
                                                                              timer->remainingTime.Seconds,
                                                                              timer->remainingTime.Microseconds);
      }
      #endif
    }
    else
      E(DBF_TIMER, "timer[%ld]: is invalid and can't be paused", tid);
  }
  else
    W(DBF_TIMER, "timer[%ld]: already paused", tid);

  LEAVE();
}

///
/// ResumeTimer
//  Resume a timer with the remaining time
void ResumeTimer(const enum Timer tid)
{
  struct TRequest *timer = &G->timerData.timer[tid];

  ENTER();

  if(timer->isRunning == FALSE && timer->isPaused == TRUE)
  {
    struct TimeRequest *tr = timer->tr;

    // issue a new timerequest with the previously calculated remaining time
    tr->Request.io_Command  = TR_ADDREQUEST;
    tr->Time.Seconds = timer->remainingTime.Seconds;
    tr->Time.Microseconds = timer->remainingTime.Microseconds;

    #if defined(DEBUG)
    {
      char dateString[64];
      DateStamp2String(dateString, sizeof(dateString), NULL, DSS_DATETIME, TZC_NONE);

      D(DBF_TIMER, "timer[%ld]: resumed @ %s to finish in %ld'%ld secs", tid,
                                                                           dateString,
                                                                           tr->Time.Seconds,
                                                                           tr->Time.Microseconds);
    }
    #endif

    // fire the timer by doing a SendIO()
    SendIO((struct IORequest *)tr);

    // remember the new start time
    GetSysTime(TIMEVAL(&timer->startTime));

    // signal that our timer is running
    timer->isRunning = TRUE;
    timer->isPaused = FALSE;
  }
  else
    W(DBF_TIMER, "timer[%ld]: either already running or not paused", tid);

  LEAVE();
}

///
/// RestartTimer
//  restarts a particular timer. In fact it makes sure that the timer in question
//  is first stopped via AbortIO() and then issues a new one. Please note that
//  this function may NOT be called from the eventloop because CheckIO and friends
//  are not defined to work there.
void RestartTimer(const enum Timer tid, const int seconds, const int micros)
{
  ENTER();

  StopTimer(tid);
  PrepareTimer(tid, seconds, micros);
  StartTimer(tid);

  LEAVE();
}
///

/*** Timer management functions ***/
/// InitTimers
//  Initializes timer resources
BOOL InitTimers(void)
{
  BOOL result = FALSE;

  ENTER();

  // create message port
  if((G->timerData.port = AllocSysObjectTags(ASOT_PORT, TAG_DONE)) != NULL)
  {
    // create the TimerIOs now

    // we use AllocSysObjectTags to give the OS a better chance to
    // free the data in case YAM crashes (only available on OS4)
    if((G->timerData.timer[0].tr = AllocSysObjectTags(ASOT_IOREQUEST,
                                                ASOIOR_Size,      sizeof(struct TimeRequest),
                                                ASOIOR_ReplyPort, (ULONG)G->timerData.port,
                                                TAG_DONE)) != NULL)
    {
      // then open the device
      if(OpenDevice(TIMERNAME, UNIT_VBLANK, &G->timerData.timer[0].tr->Request, 0L) == 0)
      {
        // needed to get GetSysTime() working
        if((TimerBase = (APTR)G->timerData.timer[0].tr->Request.io_Device) &&
           GETINTERFACE("main", 1, ITimer, TimerBase))
        {
          enum Timer tid;

          // create our other TimerIOs now
          for(tid = TIMER_WRINDEX + 1; tid < TIMER_NUM; tid++)
          {
            // we use AllocSysObjectTags to give the OS a better chance to
            // free the data in case YAM crashes (only available on OS4)
            if((G->timerData.timer[tid].tr = AllocSysObjectTags(ASOT_IOREQUEST,
                                                          ASOIOR_Size,      sizeof(struct TimeRequest),
                                                          ASOIOR_Duplicate, (ULONG)G->timerData.timer[0].tr,
                                                          TAG_DONE)) == NULL)
            {
              break;
            }
          }
        }
      }
    }
  }

  result = (BOOL)(G->timerData.timer[TIMER_NUM-1].tr != NULL);

  RETURN(result);
  return result;
}

///
/// CleanupTimers
//  Frees timer resources
void CleanupTimers(void)
{
  ENTER();

  // first we abort & delete the IORequests
  if(G->timerData.timer[0].tr != NULL)
  {
    enum Timer tid;

    // first make sure every TimerIO is stoppped
    for(tid = TIMER_WRINDEX; tid < TIMER_NUM; tid++)
      StopTimer(tid);

    // then close the device
    if(G->timerData.timer[0].tr->Request.io_Device != NULL)
    {
      // drop the OS4 Interface of the TimerBase
      DROPINTERFACE(ITimer);

      CloseDevice(&G->timerData.timer[0].tr->Request);
    }

    // and then we delete the IO requests
    for(tid = TIMER_WRINDEX + 1; tid < TIMER_NUM; tid++)
    {
      if(G->timerData.timer[tid].tr != NULL)
      {
        FreeSysObject(ASOT_IOREQUEST, G->timerData.timer[tid].tr);

        G->timerData.timer[tid].tr = NULL;
      }
    }

    FreeSysObject(ASOT_IOREQUEST, G->timerData.timer[0].tr);
    G->timerData.timer[0].tr = NULL;
  }

  // remove the MsgPort now.
  if(G->timerData.port != NULL)
  {
    FreeSysObject(ASOT_PORT, G->timerData.port);
    G->timerData.port = NULL;
  }

  LEAVE();
}

///
/// TimerDispatcher
//  Dispatcher for timer class
//  WARNING: Do NOT use StartTimer() directly in this function as it is
//           called within the timer eventloop which is undefined!
//           Do a PrepareTimer() instead here and a StartTimer() in the
//           the parent eventloop at the end of the file here.
static void TimerDispatcher(const enum Timer tid)
{
  // prepare some debug information
  #if defined(DEBUG)
  char dateString[64];

  DateStamp2String(dateString, sizeof(dateString), NULL, DSS_DATETIME, TZC_NONE);
  #endif

  ENTER();

  // now dispatch between the differnent timerIOs
  switch(tid)
  {
    // in case the WriteIndexes TimerIO request was triggered
    // we first check if no Editor is currently active and
    // if so we write the indexes.
    case TIMER_WRINDEX:
    {
      BOOL updateIndex = TRUE;
      struct Node *curNode;

      D(DBF_TIMER, "timer[%ld]: TIMER_WRINDEX fired @ %s", tid, dateString);

      // only write the indexes if no Editor is actually in use
      IterateList(&G->writeMailDataList, curNode)
      {
        struct WriteMailData *wmData = (struct WriteMailData *)curNode;

        if(wmData->window != NULL)
        {
          if(DoMethod(wmData->window, MUIM_WriteWindow_IsEditorActive) == TRUE)
          {
            updateIndex = FALSE;
            break;
          }
        }
      }

      if(updateIndex == TRUE)
        MA_UpdateIndexes();
      else
        D(DBF_TIMER, "Editor object of a write window is active, skipping update index operation");

      // prepare the timer to get fired again
      PrepareTimer(tid, C->WriteIndexes, 0);
    }
    break;

    // in case the checkMail timerIO request was triggered we
    // need to check if no writewindow is currently in use and
    // then check for new mail.
    case TIMER_CHECKMAIL:
    {
      BOOL writeWindowActive = FALSE;
      struct Node *curNode;

      D(DBF_TIMER, "timer[%ld]: TIMER_CHECKMAIL fired @ %s", tid, dateString);

      // only if there is currently no write window open we
      // check for new mail.
      IterateList(&G->writeMailDataList, curNode)
      {
        struct WriteMailData *wmData = (struct WriteMailData *)curNode;

        if(wmData->window != NULL && xget(wmData->window, MUIA_Window_Open) == TRUE)
        {
          writeWindowActive = TRUE;
          break;
        }
      }

      // also the configuration window needs to be closed
      // or we skip the pop operation
      if(writeWindowActive == FALSE &&
         G->CO == NULL)
      {
        MA_PopNow(POP_TIMED, -1);
      }

      // prepare the timer to get fired again
      PrepareTimer(tid, C->CheckMailDelay*60, 0);
    }
    break;

    // in case the AutoSave timerIO was triggered we check
    // whether there is really need to autosave the content
    // of the currently used editors.
    case TIMER_AUTOSAVE:
    {
      struct Node *curNode;

      D(DBF_TIMER, "timer[%ld]: TIMER_AUTOSAVE fired @ %s", tid, dateString);

      IterateList(&G->writeMailDataList, curNode)
      {
        struct WriteMailData *wmData = (struct WriteMailData *)curNode;

        if(wmData->window != NULL)
          DoMethod(wmData->window, MUIM_WriteWindow_DoAutoSave);
      }

      // prepare the timer to get fired again
      PrepareTimer(tid, C->AutoSave, 0);
    }
    break;

    // in case the READPANEUPDATE timerIO was triggered the embedded read
    // pane in the main window should get updated. Therefore we get the
    // currently active mail out of the main mail list and display it in the pane
    case TIMER_READPANEUPDATE:
    {
      D(DBF_TIMER, "timer[%ld]: TIMER_READPANEUPDATE fired @ %s", tid, dateString);

      if(C->EmbeddedReadPane == TRUE)
      {
        struct MA_GUIData *gui = &G->MA->GUI;
        struct Mail *mail;

        // get the actually active mail
        DoMethod(gui->PG_MAILLIST, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &mail);

        // update the readMailGroup of the main window.
        if(mail != NULL)
          DoMethod(gui->MN_EMBEDDEDREADPANE, MUIM_ReadMailGroup_ReadMail, mail,
                                             MUIF_ReadMailGroup_ReadMail_StatusChangeDelay);
      }
    }
    break;

    // on a READSTATUSUPDATE timerIO request the mail status of the currently active mail
    // should change from new/unread to read. So we get the currently active mail
    // out of the main mail list and modify its status to read.
    case TIMER_READSTATUSUPDATE:
    {
      struct MA_GUIData *gui = &G->MA->GUI;
      struct Mail *mail;

      D(DBF_TIMER, "timer[%ld]: TIMER_READSTATUSUPDATE fired @ %s", tid, dateString);

      // get the actually active mail
      DoMethod(gui->PG_MAILLIST, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &mail);

      // update the status of the mail to READ now
      if(mail != NULL && (hasStatusNew(mail) || !hasStatusRead(mail)))
      {
        setStatusToRead(mail); // set to OLD
        DisplayStatistics(mail->Folder, TRUE);

        // update the status of the readmaildata (window)
        // of the mail here
        UpdateReadMailDataStatus(mail);
      }
    }
    break;

    // on a PROCESSQUICKSEARCH we signal the quicksearch bar to actually process the
    // search. This is used to let a user type in a string in the quicksearchbar
    // without always reissuing the search process, so only the last search request
    // comes actually through. This should prevent the GUI from blocking in some
    // cases.
    case TIMER_PROCESSQUICKSEARCH:
    {
      struct MA_GUIData *gui = &G->MA->GUI;

      D(DBF_TIMER, "timer[%ld]: TIMER_PROCESSQUICKSEARCH fired @ %s", tid, dateString);

      // abort a still running previous search
      DoMethod(gui->GR_QUICKSEARCHBAR, MUIM_QuickSearchBar_AbortSearch);

      // signal the QuickSearchBar to search now
      DoMethod(gui->GR_QUICKSEARCHBAR, MUIM_QuickSearchBar_ProcessSearch);
    }
    break;

    // on a POP3_KEEPALIVE we make sure that a currently active, but waiting
    // POP3 connection (preselection) doesn't die by sending NOOP commands regularly
    // to the currently connected POP3 server.
    case TIMER_POP3_KEEPALIVE:
    {
      D(DBF_TIMER, "timer[%ld]: TIMER_POP3_KEEPALIVE fired @ %s", tid, dateString);

      // send the POP3 server a 'NOOP'
      if(TR_SendPOP3KeepAlive() == TRUE)
      {
        // prepare the timer to get fired again
        PrepareTimer(tid, C->KeepAliveInterval, 0);
      }
    }
    break;

    // on a UPDATECHECK we have to process our update check routines as the
    // user wants to check if there is a new version of YAM available or not.
    case TIMER_UPDATECHECK:
    {
      D(DBF_TIMER, "timer[%ld]: TIMER_UPDATECHECK fired @ %s", tid, dateString);

      DoMethod(G->App, MUIM_YAM_UpdateCheck, TRUE);

      // prepare the timer to get fired again
      PrepareTimer(tid, C->UpdateInterval, 0);
    }
    break;

    // on a SPAMFLUSHTRAININGDATA we write back the spam training data gathered so far
    case TIMER_SPAMFLUSHTRAININGDATA:
    {
      D(DBF_TIMER, "timer[%ld]: TIMER_SPAMFLUSHTRAININGDATA fired @ %s", tid, dateString);

      DoAction(TA_FlushSpamTrainingData, TAG_DONE);

      PrepareTimer(tid, C->SpamFlushTrainingDataInterval, 0);
    }
    break;

    // on a DELETEZOMBIEFILES we try to delete zombie files which could not be deleted
    // before. Files which still cannot be deleted will be kept in the list and retried
    // later.
    case TIMER_DELETEZOMBIEFILES:
    {
      D(DBF_TIMER, "timer[%ld]: TIMER_DELETEZOMBIEFILES fired @ %s", tid, dateString);

      if(DeleteZombieFiles(FALSE) == FALSE)
      {
        // trigger the retry mechanism in 5 minutes
        PrepareTimer(TIMER_DELETEZOMBIEFILES, 5 * 60, 0);
      }
    }
    break;

    // on a CHECKBIRTHDAYS we will check the address book for currently pending birthdays
    case TIMER_CHECKBIRTHDAYS:
    {
      D(DBF_TIMER, "timer[%ld]: TIMER_CHECKBIRTHDAYS fired @ %s", tid, dateString);

      AB_CheckBirthdates(TRUE);
    }
    break;

    // check for superflous threads being idle
    case TIMER_PURGEIDLETHREADS:
    {
      D(DBF_TIMER, "timer[%ld]: TIMER_PURGEIDLETHREADS fired @ %s", tid, dateString);

      PurgeIdleThreads(FALSE);

      // restart the timer
      PrepareTimer(TIMER_PURGEIDLETHREADS, 60, 0);
    }
    break;

    // dummy to please GCC
    case TIMER_NUM:
      // nothing
    break;
  }

  LEAVE();
}

///
/// ProcessTimerEvent
// function to check the status of all our timers via the msgport
// process on them and then restart those fired accordingly.
BOOL ProcessTimerEvent(void)
{
  BOOL processed = FALSE;
  struct TimeRequest *timeReq;

  ENTER();

  // check if we have a waiting message
  while((timeReq = (struct TimeRequest *)GetMsg(G->timerData.port)) != NULL)
  {
    enum Timer tid;

    for(tid=0; tid < TIMER_NUM; tid++)
    {
      struct TRequest *timer = &G->timerData.timer[tid];

      if(timeReq == timer->tr)
      {
        // set the timer to be not running and not be prepared for
        // another shot. Our dispatcher have to do the rest then
        timer->isRunning = FALSE;
        timer->isPrepared = FALSE;

        // call the dispatcher with signalling which timer
        // this request caused
        TimerDispatcher(tid);

        // signal that we processed something
        processed = TRUE;

        // break out of the for() loop
        break;
      }
    }

    // no ReplyMsg() needed
  }

  // make sure that we are starting the timer again after the GetMsg loop
  if(processed == TRUE)
  {
    // here we just check for the timers that TimerDispatcher really
    // prepares and not all of them in a loop

    if(G->timerData.timer[TIMER_WRINDEX].isPrepared == TRUE)
      StartTimer(TIMER_WRINDEX);

    if(G->timerData.timer[TIMER_CHECKMAIL].isPrepared == TRUE)
      StartTimer(TIMER_CHECKMAIL);

    if(G->timerData.timer[TIMER_AUTOSAVE].isPrepared == TRUE)
      StartTimer(TIMER_AUTOSAVE);

    if(G->timerData.timer[TIMER_POP3_KEEPALIVE].isPrepared == TRUE)
      StartTimer(TIMER_POP3_KEEPALIVE);

    if(G->timerData.timer[TIMER_UPDATECHECK].isPrepared == TRUE)
      StartTimer(TIMER_UPDATECHECK);

    if(G->timerData.timer[TIMER_SPAMFLUSHTRAININGDATA].isPrepared == TRUE)
      StartTimer(TIMER_SPAMFLUSHTRAININGDATA);

    if(G->timerData.timer[TIMER_DELETEZOMBIEFILES].isPrepared == TRUE)
      StartTimer(TIMER_DELETEZOMBIEFILES);

    if(G->timerData.timer[TIMER_PURGEIDLETHREADS].isPrepared == TRUE)
      StartTimer(TIMER_PURGEIDLETHREADS);
  }
  else
    W(DBF_TIMER, "timer event received but no timer was processed!!!");

  #if defined(DEBUG)
  // let us check whether all necessary maintenance timers are running
  // because right here ALL maintenance timers should run or something is definitely wrong!

  if(C->WriteIndexes > 0 && G->timerData.timer[TIMER_WRINDEX].isRunning == FALSE)
    E(DBF_ALWAYS, "timer[%ld]: TIMER_WRINDEX is not running and was probably lost!", TIMER_WRINDEX);

  if(C->CheckMailDelay > 0 && G->timerData.timer[TIMER_CHECKMAIL].isRunning == FALSE)
    E(DBF_ALWAYS, "timer[%ld]: TIMER_CHECKMAIL is not running and was probably lost!", TIMER_CHECKMAIL);

  if(C->AutoSave > 0 && G->timerData.timer[TIMER_AUTOSAVE].isRunning == FALSE)
    E(DBF_ALWAYS, "timer[%ld]: TIMER_AUTOSAVE is not running and was probably lost!", TIMER_AUTOSAVE);
  #endif

  RETURN(processed);
  return processed;
}
///
