/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2022 YAM Open Source Team

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

#include <clib/alib_protos.h>
#include <proto/exec.h>
#include <proto/intuition.h>

#include "YAM.h"
#include "YAM_stringsizes.h"

#include "extrasrc.h"

#include "mui/InfoBar.h"
#include "mui/SplashWindow.h"
#include "mui/YAMApplication.h"

#include "Busy.h"
#include "MethodStack.h"
#include "Threads.h"

#include "Debug.h"

/// BusyBegin
// start a busy action
struct BusyNode *BusyBegin(ULONG type)
{
  struct BusyNode *busy = NULL;

  ENTER();

  if(IsMainThread() == TRUE)
  {
    if((busy = AllocSysObjectTags(ASOT_NODE,
      ASONODE_Size, sizeof(*busy),
      ASONODE_Min, TRUE,
      TAG_DONE)) != NULL)
    {
      memset(busy, 0, sizeof(*busy));
      busy->type = type;
      busy->progressCurrent = (type == BUSY_PROGRESS_ABORT) ? 0 : -1;
      busy->progressMax = 0;

      if(type == BUSY_AREXX)
        AddTail((struct List *)&G->arexxBusyList, (struct Node *)busy);
      else
        AddTail((struct List *)&G->normalBusyList, (struct Node *)busy);
    }
  }
  else
  {
    // called from a thread, push a method on the stack
    busy = (struct BusyNode *)PushMethodOnStackWait(G->App, 2, MUIM_YAMApplication_BusyBegin, type);
  }

  RETURN(busy);
  return busy;
}

///
/// BusyShow
// make sure a busy action is visible
static BOOL BusyShow(const struct BusyNode *busy)
{
  BOOL goOn = TRUE;

  ENTER();

  // check if we are in startup phase so that we also
  // update the gauge elements of the splash window
  if(G->InStartupPhase == TRUE)
    DoMethod(G->SplashWinObject, MUIM_SplashWindow_ProgressChange, busy);
  else if(G->MA != NULL)
    goOn = DoMethod(G->MA->GUI.IB_INFOBAR, MUIM_InfoBar_ShowBusyBar, busy);

  RETURN(goOn);
  return goOn;
}

///
/// BusyText
// set an information text for a busy action
void BusyText(struct BusyNode *busy, const char *text, const char *param)
{
  ENTER();

  if(busy != NULL)
  {
    if(IsMainThread() == TRUE)
    {
      snprintf(busy->infoText, sizeof(busy->infoText), text, param);
      BusyShow(busy);
    }
    else
    {
      // called from a thread, push a method on the stack
      PushMethodOnStack(G->App, 4, MUIM_YAMApplication_BusyText, busy, text, param);
    }
  }

  LEAVE();
}

///
/// BusyProgress
// update the progress of a busy action
BOOL BusyProgress(struct BusyNode *busy, int progress, int max)
{
  BOOL goOn = TRUE;

  ENTER();

  if(busy != NULL)
  {
    if(busy->type == BUSY_PROGRESS || busy->type == BUSY_PROGRESS_ABORT)
    {
      if(IsMainThread() == TRUE)
      {
        // a negative progress is used for non-abortable busy actions
        busy->progressCurrent = (busy->type == BUSY_PROGRESS_ABORT) ? progress : -1;
        busy->progressMax = max;
        goOn = BusyShow(busy);
      }
      else
      {
        // called from a thread, push a method on the stack
        if(busy->type == BUSY_PROGRESS_ABORT)
          goOn = PushMethodOnStackWait(G->App, 3, MUIM_YAMApplication_BusyProgress, progress, max);
        else
          PushMethodOnStack(G->App, 4, MUIM_YAMApplication_BusyProgress, busy, progress, max);
      }
    }
    else
      W(DBF_ALWAYS, "wrong function call for Busy type %ld", busy->type);
  }

  RETURN(goOn);
  return goOn;
}

///
/// BusyEnd
// finish a busy action
void BusyEnd(struct BusyNode *busy)
{
  ENTER();

  if(busy != NULL)
  {
    if(IsMainThread() == TRUE)
    {
      struct BusyNode *last = NULL;

      // remove this node from the list
      Remove((struct Node *)busy);

      // Try to show the last busy action in the list or hide the busy bar otherwise.
      // If the just ended action was initiated by ARexx we prefer that list over the
      // normal list.
      if(busy->type == BUSY_AREXX)
        last = (struct BusyNode *)GetTail((struct List *)&G->arexxBusyList);
      // fall back to the normal busy actions if there is no further ARexx busy operation
      if(last == NULL)
        last = (struct BusyNode *)GetTail((struct List *)&G->normalBusyList);
      BusyShow(last);

      FreeSysObject(ASOT_NODE, busy);
    }
    else
    {
      // called from a thread, push a method on the stack
      PushMethodOnStack(G->App, 2, MUIM_YAMApplication_BusyEnd, busy);
    }
  }

  LEAVE();
}

///
/// BusyCleanup
// free any still pending busy actions
void BusyCleanup(void)
{
  struct BusyNode *busy;
  struct BusyNode *next;

  ENTER();

  SafeIterateList(&G->normalBusyList, struct BusyNode *, busy, next)
  {
    W(DBF_STARTUP, "freeing pending normal busy action %08lx, type %ld, text '%s'", busy, busy->type, busy->infoText);
    FreeSysObject(ASOT_NODE, busy);
  }
  NewMinList(&G->normalBusyList);

  SafeIterateList(&G->arexxBusyList, struct BusyNode *, busy, next)
  {
    W(DBF_STARTUP, "freeing pending ARexx busy action %08lx, type %ld, text '%s'", busy, busy->type, busy->infoText);
    FreeSysObject(ASOT_NODE, busy);
  }
  NewMinList(&G->arexxBusyList);

  LEAVE();
}

///
