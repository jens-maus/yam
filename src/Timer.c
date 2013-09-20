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

#include "extrasrc.h"
#include "timeval.h"

#include "YAM.h"
#include "YAM_mainFolder.h"
#include "YAM_write.h"

#include "Config.h"
#include "Locale.h"
#include "MailServers.h"
#include "MUIObjects.h"
#include "Threads.h"
#include "Timer.h"
#include "TZone.h"

#include "mui/ClassesExtra.h"
#include "mui/QuickSearchBar.h"
#include "mui/ReadMailGroup.h"
#include "mui/WriteWindow.h"
#include "mui/YAMApplication.h"

#include "Debug.h"

/*** Timer processing function ***/
/// PrepareTRequest
// prepare a single time request
static void PrepareTRequest(struct TRequest *timer, const ULONG seconds, const ULONG micros, BOOL absoluteTime)
{
  ENTER();

  if(seconds > 0 || micros > 0)
  {
    if(timer->isRunning == FALSE && timer->isPrepared == FALSE)
    {
      // issue a new timerequest
      timer->tr->Request.io_Command  = TR_ADDREQUEST;
      timer->tr->Time.Seconds        = seconds;
      timer->tr->Time.Microseconds   = micros;

      // remember the remaining time
      timer->remainingTime.Seconds = seconds;
      timer->remainingTime.Microseconds = micros;

      // flag the timer to be prepared to get fired later on
      timer->isPrepared = TRUE;
      timer->isAbsolute = absoluteTime;
    }
    #if defined(DEBUG)
    else
    {
      if(timer->id != -1)
        W(DBF_TIMER, "timer[%ld]: already running/prepared", timer->id);
      else if(timer->pop3Server != NULL)
        W(DBF_TIMER, "timer[%s]: already running/prepared", timer->pop3Server->description);
    }
    #endif
  }
  #if defined(DEBUG)
  else
  {
    if(timer->id != -1)
      W(DBF_TIMER, "timer[%ld]: secs and micros are zero, no prepare required", timer->id);
    else if(timer->pop3Server != NULL)
      W(DBF_TIMER, "timer[%s]: secs and micros are zero, no prepare required", timer->pop3Server->description);
  }
  #endif

  LEAVE();
}

///
/// PrepareTimer
//  prepares a timer for being started with StartTimer() later on
void PrepareTimer(const enum Timer tid, const ULONG seconds, const ULONG micros, BOOL absoluteTime)
{
  ENTER();

  PrepareTRequest(&G->timerData.timer[tid], seconds, micros, absoluteTime);

  LEAVE();
}

///
/// StartTRequest
// start a timer request
static void StartTRequest(struct TRequest *timer)
{
  ENTER();

  if(timer->tr != NULL)
  {
    if(timer->isRunning == FALSE && timer->isPrepared == TRUE)
    {
      #if defined(DEBUG)
      char dateStrStarted[64];
      char dateStrFinish[64];
      struct TimeVal tvTmp;
      struct TimeVal tvTimer;
      #endif

      // we have to check wheter our TRequest carries an absolute or relative
      // time information and act upon
      if(timer->isAbsolute == TRUE)
      {
        struct TimeVal tsNow;

        // get current time
        GetSysTime(TIMEVAL(&tsNow));

        // convert the time information in the trequest to relative time
        // since this is what timer.device supports only
        SubTime(TIMEVAL(&timer->tr->Time), TIMEVAL(&tsNow));
      }

      #if defined(DEBUG)
      memcpy(&tvTimer, &timer->tr->Time, sizeof(tvTimer));
      #endif

      // fire the timer by doing a SendIO()
      SendIO(&timer->tr->Request);

      // remember the timer start time
      GetSysTime(TIMEVAL(&timer->startTime));

      #if defined(DEBUG)
      memcpy(&tvTmp, &timer->startTime, sizeof(tvTmp));

      // get the current date/time in a str
      TimeVal2String(dateStrStarted, sizeof(dateStrStarted), &tvTmp, DSS_DATETIME, TZC_NONE);

      // get the date/time when the timer finishes in a str
      AddTime(TIMEVAL(&tvTmp), TIMEVAL(&tvTimer));
      TimeVal2String(dateStrFinish, sizeof(dateStrFinish), &tvTmp, DSS_DATETIME, TZC_NONE);

      if(timer->id != -1)
      {
        D(DBF_TIMER, "timer[%ld]: started @ %s to finish in %ld'%ld secs (%s)",
          timer->id, dateStrStarted, tvTimer.Seconds, tvTimer.Microseconds, dateStrFinish);
      }
      else if(timer->pop3Server != NULL)
      {
        D(DBF_TIMER, "timer[%s]: started @ %s to finish in %ld'%ld secs (%s)",
          timer->pop3Server->description, dateStrStarted, tvTimer.Seconds, tvTimer.Microseconds,
          dateStrFinish);
      }
      #endif

      // signal that our timer is running
      timer->isRunning = TRUE;
      timer->isPrepared = FALSE;
    }
    #if defined(DEBUG)
    else
    {
      if(timer->id != -1)
        W(DBF_TIMER, "timer[%ld]: either already running or not prepared to get fired", timer->id);
      else if(timer->pop3Server != NULL)
        W(DBF_TIMER, "timer[%s]: either already running or not prepared to get fired", timer->pop3Server->description);
    }
    #endif
  }
  #if defined(DEBUG)
  else
  {
    if(timer->id != -1)
      W(DBF_TIMER, "timer[%ld]: timer is already cleaned up", timer->id);
    else if(timer->pop3Server != NULL)
      W(DBF_TIMER, "timer[%s]: timer is already cleaned up", timer->pop3Server->description);
  }
  #endif

  LEAVE();
}

///
/// StartTimer
//  Start a delay depending on the time specified
void StartTimer(const enum Timer tid)
{
  ENTER();

  StartTRequest(&G->timerData.timer[tid]);

  LEAVE();
}

///
/// StopTRequest
// stop a time request
static void StopTRequest(struct TRequest *timer)
{
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

      #if defined(DEBUG)
      if(timer->id != -1)
        D(DBF_TIMER, "timer[%ld]: successfully stopped", timer->id);
      else if(timer->pop3Server != NULL)
        D(DBF_TIMER, "timer[%s]: successfully stopped", timer->pop3Server->description);
      #endif
    }
    #if defined(DEBUG)
    else
    {
      if(timer->id != -1)
        E(DBF_TIMER, "timer[%ld]: is invalid and can't be stopped", timer->id);
      else if(timer->pop3Server != NULL)
        E(DBF_TIMER, "timer[%s]: is invalid and can't be stopped", timer->pop3Server->description);
    }
    #endif
  }
  #if defined(DEBUG)
  else
  {
    if(timer->id != -1)
      W(DBF_TIMER, "timer[%ld]: already stopped", timer->id);
    else if(timer->pop3Server != NULL)
      W(DBF_TIMER, "timer[%s]: already stopped", timer->pop3Server->description);
  }
  #endif

  LEAVE();
}

///
/// StopTimer
//  Stop a currently running TimerIO request
//  Please note that this function may NOT be used in the eventloop after having received
//  a timer with GetMsg because CheckIO and friends are not defined to work there correctly.
void StopTimer(const enum Timer tid)
{
  ENTER();

  StopTRequest(&G->timerData.timer[tid]);

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
    struct TimeRequest *timeReq = timer->tr;

    // issue a new timerequest with the previously calculated remaining time
    timeReq->Request.io_Command  = TR_ADDREQUEST;
    timeReq->Time.Seconds = timer->remainingTime.Seconds;
    timeReq->Time.Microseconds = timer->remainingTime.Microseconds;

    #if defined(DEBUG)
    {
      char dateString[64];
      DateStamp2String(dateString, sizeof(dateString), NULL, DSS_DATETIME, TZC_NONE);

      D(DBF_TIMER, "timer[%ld]: resumed @ %s to finish in %ld'%ld secs", tid,
                                                                           dateString,
                                                                           timeReq->Time.Seconds,
                                                                           timeReq->Time.Microseconds);
    }
    #endif

    // fire the timer by doing a SendIO()
    SendIO((struct IORequest *)timeReq);

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
void RestartTimer(const enum Timer tid, const ULONG seconds, const ULONG micros, BOOL absoluteTime)
{
  ENTER();

  StopTimer(tid);
  PrepareTimer(tid, seconds, micros, absoluteTime);
  StartTimer(tid);

  LEAVE();
}
///

/*** Timer management functions ***/
/// CreateTRequest
// create a new time request, will be cloned from the first global request
BOOL CreateTRequest(struct TRequest *timer, UNUSED const int id, UNUSED struct MailServerNode *msn)
{
  BOOL success = FALSE;

  ENTER();

  if(G->timerData.timer[0].tr != NULL)
  {
    // clone the first global request
    timer->tr = AllocSysObjectTags(ASOT_IOREQUEST,
      ASOIOR_Size,      sizeof(struct TimeRequest),
      ASOIOR_Duplicate, (IPTR)G->timerData.timer[0].tr,
      TAG_DONE);
  }
  else
  {
    // create a new request
    timer->tr = AllocSysObjectTags(ASOT_IOREQUEST,
      ASOIOR_Size,      sizeof(struct TimeRequest),
      ASOIOR_ReplyPort, (IPTR)G->timerData.port,
      TAG_DONE);
  }

  if(timer->tr != NULL)
  {
    timer->isRunning = FALSE;
    timer->isPrepared = FALSE;

    #if defined(DEBUG)
    // remember some values for timer debugging
    timer->id = id;
    timer->pop3Server = msn;
    #endif

    success = TRUE;
  }

  RETURN(success);
  return success;
}

///
/// DeleteTRequest
// delete a cloned time request
void DeleteTRequest(struct TRequest *timer)
{
  ENTER();

  if(timer->tr != NULL)
  {
    StopTRequest(timer);
    FreeSysObject(ASOT_IOREQUEST, timer->tr);
    timer->tr = NULL;
  }

  LEAVE();
}

///
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
    if(CreateTRequest(&G->timerData.timer[0], 0, NULL) == TRUE)
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
            if(CreateTRequest(&G->timerData.timer[tid], tid, NULL) == FALSE)
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

    StopPOP3Timers();

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
      DeleteTRequest(&G->timerData.timer[tid]);
    }

    DeleteTRequest(&G->timerData.timer[0]);
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
      struct WriteMailData *wmData;

      D(DBF_TIMER, "timer[%ld]: TIMER_WRINDEX fired @ %s", tid, dateString);

      // only write the indexes if no Editor is actually in use
      IterateList(&G->writeMailDataList, struct WriteMailData *, wmData)
      {
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
      PrepareTimer(tid, C->WriteIndexes, 0, FALSE);
    }
    break;

    // in case the AutoSave timerIO was triggered we check
    // whether there is really need to autosave the content
    // of the currently used editors.
    case TIMER_AUTOSAVE:
    {
      struct WriteMailData *wmData;

      D(DBF_TIMER, "timer[%ld]: TIMER_AUTOSAVE fired @ %s", tid, dateString);

      IterateList(&G->writeMailDataList, struct WriteMailData *, wmData)
      {
        if(wmData->window != NULL)
          DoMethod(wmData->window, MUIM_WriteWindow_DoAutoSave);
      }

      // prepare the timer to get fired again
      PrepareTimer(tid, C->AutoSave, 0, FALSE);
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

      // update the status of the mail to READ now if this is not the outgoing/draft folder
      if(mail != NULL && !isOutgoingFolder(mail->Folder) && !isDraftsFolder(mail->Folder) && (hasStatusNew(mail) || !hasStatusRead(mail)))
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
      set(gui->GR_QUICKSEARCHBAR, MUIA_QuickSearchBar_AbortSearch, TRUE);

      // signal the QuickSearchBar to search now
      DoMethod(gui->GR_QUICKSEARCHBAR, MUIM_QuickSearchBar_ProcessSearch);
    }
    break;

    // on a UPDATECHECK we have to process our update check routines as the
    // user wants to check if there is a new version of YAM available or not.
    case TIMER_UPDATECHECK:
    {
      D(DBF_TIMER, "timer[%ld]: TIMER_UPDATECHECK fired @ %s", tid, dateString);

      DoMethod(G->App, MUIM_YAMApplication_UpdateCheck, TRUE);

      // prepare the timer to get fired again
      PrepareTimer(tid, C->UpdateInterval, 0, FALSE);
    }
    break;

    // on a SPAMFLUSHTRAININGDATA we write back the spam training data gathered so far
    case TIMER_SPAMFLUSHTRAININGDATA:
    {
      D(DBF_TIMER, "timer[%ld]: TIMER_SPAMFLUSHTRAININGDATA fired @ %s", tid, dateString);

      DoAction(NULL, TA_FlushSpamTrainingData, TAG_DONE);

      PrepareTimer(tid, C->SpamFlushTrainingDataInterval, 0, FALSE);
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
        PrepareTimer(TIMER_DELETEZOMBIEFILES, 5 * 60, 0, FALSE);
      }
    }
    break;

    // on a CHECKBIRTHDAYS we will check the address book for currently pending birthdays
    case TIMER_CHECKBIRTHDAYS:
    {
      D(DBF_TIMER, "timer[%ld]: TIMER_CHECKBIRTHDAYS fired @ %s", tid, dateString);

      CheckABookBirthdays(&G->abook, TRUE);
    }
    break;

    // check for superflous threads being idle
    case TIMER_PURGEIDLETHREADS:
    {
      D(DBF_TIMER, "timer[%ld]: TIMER_PURGEIDLETHREADS fired @ %s", tid, dateString);

      PurgeIdleThreads(FALSE);

      // restart the timer
      PrepareTimer(TIMER_PURGEIDLETHREADS, 60, 0, FALSE);
    }
    break;

    // DST switch was recognized, so lets update gmtOffset&co
    case TIMER_DSTSWITCH:
    {
      D(DBF_TIMER, "timer[%ld]: TIMER_DSTSWITCH fired @ %s", tid, dateString);

      // now we have to make sure to update all tzone relative
      // information - this will also restart the timer
      SetTZone(C->Location);
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
    struct MailServerNode *msn;

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

    // continue to check the POP3 timers
    ObtainSemaphoreShared(G->configSemaphore);

    IterateList(&C->pop3ServerList, struct MailServerNode *, msn)
    {
      struct TRequest *timer = &msn->downloadTimer;

      if(timeReq == timer->tr)
      {
        // set the timer to be not running and not be prepared for
        // another shot. Our dispatcher have to do the rest then
        timer->isRunning = FALSE;
        timer->isPrepared = FALSE;

        // download the mails from this POP3 server
        MA_PopNow(msn, RECEIVEF_TIMER, NULL);

        // signal that we processed something
        processed = TRUE;

        // preparestart the server's timer again
        PrepareTRequest(timer, msn->downloadInterval*60, 0, FALSE);
        StartTRequest(timer);

        // break out of the loop
        break;
      }
    }

    ReleaseSemaphore(G->configSemaphore);

    // no ReplyMsg() needed
  }

  // make sure that we are starting the timer again after the GetMsg loop
  if(processed == TRUE)
  {
    // here we just check for the timers that TimerDispatcher really
    // prepares and not all of them in a loop

    if(G->timerData.timer[TIMER_WRINDEX].isPrepared == TRUE)
      StartTimer(TIMER_WRINDEX);

    if(G->timerData.timer[TIMER_AUTOSAVE].isPrepared == TRUE)
      StartTimer(TIMER_AUTOSAVE);

    if(G->timerData.timer[TIMER_UPDATECHECK].isPrepared == TRUE)
      StartTimer(TIMER_UPDATECHECK);

    if(G->timerData.timer[TIMER_SPAMFLUSHTRAININGDATA].isPrepared == TRUE)
      StartTimer(TIMER_SPAMFLUSHTRAININGDATA);

    if(G->timerData.timer[TIMER_DELETEZOMBIEFILES].isPrepared == TRUE)
      StartTimer(TIMER_DELETEZOMBIEFILES);

    if(G->timerData.timer[TIMER_PURGEIDLETHREADS].isPrepared == TRUE)
      StartTimer(TIMER_PURGEIDLETHREADS);

    if(G->timerData.timer[TIMER_DSTSWITCH].isPrepared == TRUE)
      StartTimer(TIMER_DSTSWITCH);
  }
  else
    W(DBF_TIMER, "timer event received but no timer was processed!!!");

  #if defined(DEBUG)
  // let us check whether all necessary maintenance timers are running
  // because right here ALL maintenance timers should run or something is definitely wrong!

  if(C->WriteIndexes > 0 && G->timerData.timer[TIMER_WRINDEX].isRunning == FALSE)
    E(DBF_ALWAYS, "timer[%ld]: TIMER_WRINDEX is not running and was probably lost!", TIMER_WRINDEX);

  if(C->AutoSave > 0 && G->timerData.timer[TIMER_AUTOSAVE].isRunning == FALSE)
    E(DBF_ALWAYS, "timer[%ld]: TIMER_AUTOSAVE is not running and was probably lost!", TIMER_AUTOSAVE);
  #endif

  RETURN(processed);
  return processed;
}

///
/// PreparePOP3Timers
// prepare the timers of all POP3 servers
void PreparePOP3Timers(void)
{
  struct MailServerNode *msn;

  ENTER();

  ObtainSemaphoreShared(G->configSemaphore);

  IterateList(&C->pop3ServerList, struct MailServerNode *, msn)
  {
    PrepareTRequest(&msn->downloadTimer, msn->downloadInterval*60, 0, FALSE);
  }

  ReleaseSemaphore(G->configSemaphore);

  LEAVE();
}

///
/// StartPOP3Timers
// start the timers of all active POP3 servers
void StartPOP3Timers(void)
{
  struct MailServerNode *msn;

  ENTER();

  ObtainSemaphoreShared(G->configSemaphore);

  IterateList(&C->pop3ServerList, struct MailServerNode *, msn)
  {
    if(isServerActive(msn) && hasServerDownloadPeriodically(msn))
      StartTRequest(&msn->downloadTimer);
  }

  ReleaseSemaphore(G->configSemaphore);

  LEAVE();
}

///
/// StopPOP3Timers
// stop the timers of all active POP3 servers
void StopPOP3Timers(void)
{
  struct MailServerNode *msn;

  ENTER();

  ObtainSemaphoreShared(G->configSemaphore);

  IterateList(&C->pop3ServerList, struct MailServerNode *, msn)
  {
    StopTRequest(&msn->downloadTimer);
  }

  ReleaseSemaphore(G->configSemaphore);

  LEAVE();
}

///
/// RestartPOP3Timers
// restart the timers of all active POP3 servers
void RestartPOP3Timers(void)
{
  struct MailServerNode *msn;

  ENTER();

  ObtainSemaphoreShared(G->configSemaphore);

  IterateList(&C->pop3ServerList, struct MailServerNode *, msn)
  {
    StopTRequest(&msn->downloadTimer);
    PrepareTRequest(&msn->downloadTimer, msn->downloadInterval*60, 0, FALSE);
    if(isServerActive(msn) && hasServerDownloadPeriodically(msn))
      StartTRequest(&msn->downloadTimer);
  }

  ReleaseSemaphore(G->configSemaphore);

  LEAVE();
}

///
