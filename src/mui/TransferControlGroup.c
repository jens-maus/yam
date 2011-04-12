/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2011 YAM Open Source Team

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.   See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 YAM Official Support Site : http://www.yam.ch
 YAM OpenSource project     : http://sourceforge.net/projects/yamos/

 $Id$

 Superclass:  MUIC_ObjectListitem
 Description: Displays statistics about a transfer in progress

***************************************************************************/

#include "TransferControlGroup_cl.h"

#include <string.h>
#include <proto/muimaster.h>
#include <proto/timer.h>

#include "YAM_utilities.h"

#include "Locale.h"
#include "MUIObjects.h"
#include "Threads.h"

#include "mui/ObjectList.h"
#include "mui/TransferControlList.h"
#include "tcp/Connection.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *TX_STATS;
  Object *TX_STATUS;
  Object *GA_COUNT;
  Object *GA_BYTES;
  Object *BT_ABORT;

  struct Connection *connection;
  APTR thread;

  int Msgs_Tot;
  int Msgs_Done;
  int Msgs_Curr;
  int Msgs_ListPos;
  ULONG Size_Tot;
  ULONG Size_Done;
  ULONG Size_Curr;
  ULONG Size_Curr_Max;
  ULONG Clock_Start;
  struct TimeVal Clock_Last;

  char stats_label[SIZE_DEFAULT];
  char size_gauge_label[SIZE_DEFAULT];
  char msg_gauge_label[SIZE_DEFAULT];
  char str_size_done[SIZE_SMALL];
  char str_size_tot[SIZE_SMALL];
  char str_speed[SIZE_SMALL];
  char str_size_curr[SIZE_SMALL];
  char str_size_curr_max[SIZE_SMALL];

  BOOL started;
};
*/

/* EXPORT
#define TCG_SETMAX   (-1)
*/

/* INCLUDE
#include "timeval.h"
*/

/* Private Functions */
/// DoUpdateStats
// update the statistics
static void DoUpdateStats(struct Data *data, const int size_incr, const char *status)
{
  ENTER();

  if(size_incr > 0)
  {
    data->Size_Done += size_incr;
    data->Size_Curr += size_incr;
  }
  else if(size_incr == TCG_SETMAX)
  {
    // first update the total transferred size
    data->Size_Done += data->Size_Curr_Max - data->Size_Curr;
    // we are done with this mail, so make sure the current size equals the final size
    data->Size_Curr = data->Size_Curr_Max;
  }

  // update the stats 4 times per second at most
  if(TimeHasElapsed(&data->Clock_Last, 250000) == TRUE)
  {
    ULONG deltatime;
    ULONG speed = 0;
    LONG remtime;
    ULONG max;
    ULONG current;

    // first we calculate the speed in bytes/sec
    // to display to the user
    deltatime = data->Clock_Last.Seconds - data->Clock_Start;
    if(deltatime != 0)
      speed = data->Size_Done / deltatime;
    else
      speed = 0;

    // calculate the estimated remaining time
    remtime = 0;
    if(speed != 0)
    {
      remtime = (LONG)((data->Size_Tot / speed) - deltatime);
      if(remtime < 0)
        remtime = 0;
    }

    // show the current status
    set(data->TX_STATUS, MUIA_Text_Contents, status);

    // show the current message index
    set(data->GA_COUNT, MUIA_Gauge_Current, data->Msgs_Curr);

    // format the size done and size total strings
    FormatSize(data->Size_Done, data->str_size_done, sizeof(data->str_size_done), SF_MIXED);
    FormatSize(data->Size_Tot, data->str_size_tot, sizeof(data->str_size_tot), SF_MIXED);
    FormatSize(data->Size_Curr, data->str_size_curr, sizeof(data->str_size_curr), SF_AUTO);

    FormatSize(speed, data->str_speed, sizeof(data->str_speed), SF_MIXED);

    // now format the StatsLabel and update it
    snprintf(data->stats_label, sizeof(data->stats_label), tr(MSG_TR_TRANSFERSTATUS),
                                data->str_size_done, data->str_size_tot, data->str_speed,
                                deltatime / 60, deltatime % 60,
                                remtime / 60, remtime % 60);

    set(data->TX_STATS, MUIA_Text_Contents, data->stats_label);

    // update the gauge
    snprintf(data->size_gauge_label, sizeof(data->size_gauge_label), tr(MSG_TR_TRANSFERSIZE),
                                                           data->str_size_curr, data->str_size_curr_max);

    if(size_incr == TCG_SETMAX)
    {
      // simply display 100%
      max = 100;
      current = 100;
    }
    else if(data->Size_Curr_Max <= 65536)
    {
      // everything below 64K will be display non-scaled
      max = data->Size_Curr_Max;
      current = data->Size_Curr;
    }
    else
    {
      // everything else is scaled down by 10 bits to avoid integer overflows in MUI3.8
      max = data->Size_Curr_Max / 1024;
      current = data->Size_Curr / 1024;
    }
    xset(data->GA_BYTES, MUIA_Gauge_InfoText, data->size_gauge_label,
                         MUIA_Gauge_Max,      max,
                         MUIA_Gauge_Current,  current);
  }

  LEAVE();
}

///

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  Object *TX_STATS;
  Object *TX_STATUS;
  Object *GA_COUNT;
  Object *GA_BYTES;
  Object *BT_ABORT;
  const char *title;
  BOOL mailMode;

  ENTER();

  title = (const char *)GetTagData(ATTR(Title), (IPTR)tr(MSG_TR_Status), inittags(msg));
  mailMode = GetTagData(ATTR(MailMode), TRUE, inittags(msg));

  if((obj = DoSuperNew(cl, obj,

    GroupFrameT(title),
    MUIA_Group_Columns, 2,
    Child, TX_STATS = TextObject,
      MUIA_Background,    MUII_TextBack,
      MUIA_Frame,         MUIV_Frame_Text,
      MUIA_Text_PreParse, MUIX_C,
    End,
    Child, VGroup,
      Child, GA_COUNT = GaugeObject,
        GaugeFrame,
        MUIA_Gauge_Horiz, TRUE,
        MUIA_ShowMe, mailMode,
      End,
      Child, GA_BYTES = GaugeObject,
        GaugeFrame,
        MUIA_Gauge_Horiz, TRUE,
      End,
    End,
    Child, TX_STATUS = TextObject,
      MUIA_Background, MUII_TextBack,
      MUIA_Frame,      MUIV_Frame_Text,
    End,
    Child, BT_ABORT = MakeButton(tr(MSG_TR_Abort)),

    TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;

    data->TX_STATS = TX_STATS;
    data->TX_STATUS = TX_STATUS;
    data->GA_COUNT = GA_COUNT;
    data->GA_BYTES = GA_BYTES;
    data->BT_ABORT = BT_ABORT;

    data->thread = (APTR)GetTagData(ATTR(Thread), (IPTR)NULL, inittags(msg));

    SetHelp(data->TX_STATUS, MSG_HELP_TR_TX_STATUS);
    SetHelp(data->BT_ABORT, MSG_HELP_TR_BT_ABORT);

    DoMethod(data->BT_ABORT, MUIM_Notify, MUIA_Pressed, FALSE, obj, 1, METHOD(Abort));

    // prepare the initial text object content
    DoMethod(obj, METHOD(Reset));
  }

  RETURN((IPTR)obj);
  return (IPTR)obj;
}

///
/// OVERLOAD(OM_SET)
OVERLOAD(OM_SET)
{
  GETDATA;
  struct TagItem *tags = inittags(msg), *tag;

  while((tag = NextTagItem((APTR)&tags)) != NULL)
  {
    switch(tag->ti_Tag)
    {
      case ATTR(Title):
      {
        set(obj, MUIA_FrameTitle, (char *)tag->ti_Data);
      }
      break;

      case ATTR(Connection):
      {
        data->connection = (struct Connection *)tag->ti_Data;
      }
      break;

      case ATTR(Thread):
      {
        data->thread = (APTR)tag->ti_Data;
      }
      break;

      case ATTR(MailMode):
      {
        set(data->GA_COUNT, MUIA_ShowMe, tag->ti_Data);
      }
      break;
    }
  }

  return DoSuperMethodA(cl, obj, msg);
}

///

/* Public Methods */
/// DECLARE(Reset)
// reset the stats to zero
DECLARE(Reset)
{
  GETDATA;

  ENTER();

  FormatSize(0, data->str_size_done, sizeof(data->str_size_done), SF_MIXED);
  FormatSize(0, data->str_size_tot, sizeof(data->str_size_tot), SF_MIXED);
  FormatSize(0, data->str_speed, sizeof(data->str_speed), SF_MIXED);
  snprintf(data->stats_label, sizeof(data->stats_label), tr(MSG_TR_TRANSFERSTATUS),
                                  data->str_size_done, data->str_size_tot, data->str_speed, 0, 0, 0, 0);

  snprintf(data->msg_gauge_label, sizeof(data->msg_gauge_label), tr(MSG_TR_MESSAGEGAUGE), 0);

  FormatSize(0, data->str_size_curr, sizeof(data->str_size_curr), SF_AUTO);
  FormatSize(0, data->str_size_curr_max, sizeof(data->str_size_curr_max), SF_AUTO);
  snprintf(data->size_gauge_label, sizeof(data->size_gauge_label), tr(MSG_TR_TRANSFERSIZE),
                                                       data->str_size_curr, data->str_size_curr_max);

  set(data->TX_STATS, MUIA_Text_Contents, data->stats_label);
  xset(data->GA_COUNT, MUIA_Gauge_InfoText, data->msg_gauge_label,
                       MUIA_Gauge_Max,      data->Msgs_Tot,
                       MUIA_Gauge_Current,  0);
  xset(data->GA_BYTES, MUIA_Gauge_InfoText, data->size_gauge_label,
                       MUIA_Gauge_Max,      100,
                       MUIA_Gauge_Current,  0);

  LEAVE();
  return 0;
}

///
/// DECLARE(Abort)
DECLARE(Abort)
{
  GETDATA;

  ENTER();

  if(data->connection != NULL)
  {
    // set the connection state to aborted
    data->connection->abort = TRUE;
    data->connection->error = CONNECTERR_ABORTED;
  }

  if(data->thread != NULL)
  {
    // tell the thread to abort
    AbortThread(data->thread, FALSE);
  }

  LEAVE();
  return 0;
}

///
/// DECLARE(ShowStatus)
// display the status message
DECLARE(ShowStatus) // const char *status
{
  GETDATA;

  ENTER();

  set(data->TX_STATUS, MUIA_Text_Contents, msg->status);

  LEAVE();
  return 0;
}

///
/// DECLARE(Start)
// start the clock for a new transfer
DECLARE(Start) // int numberOfMails, ULONG totalSize
{
  GETDATA;

  ENTER();

  data->Msgs_Tot = msg->numberOfMails;
  data->Msgs_Done = 0;
  data->Msgs_Curr = 0;
  data->Size_Tot = msg->totalSize;
  data->Size_Done = 0;
  data->Size_Curr = 0;
  data->started = TRUE;

  // get the actual time we started the transfer
  GetSysTime(TIMEVAL(&data->Clock_Last));
  data->Clock_Start = data->Clock_Last.Seconds;

  memset(&data->Clock_Last, 0, sizeof(data->Clock_Last));

  snprintf(data->msg_gauge_label, sizeof(data->msg_gauge_label), tr(MSG_TR_MESSAGEGAUGE), data->Msgs_Tot);
  xset(data->GA_COUNT, MUIA_Gauge_InfoText, data->msg_gauge_label,
                       MUIA_Gauge_Max,      data->Msgs_Tot,
                       MUIA_Gauge_Current,  0);

  LEAVE();
  return 0;
}

///
/// DECLARE(Finish)
// finish the current transfer and show the final stats
DECLARE(Finish)
{
  GETDATA;

  ENTER();

  // make sure we have valid strings to display
  FormatSize(data->Size_Curr_Max, data->str_size_curr_max, sizeof(data->str_size_curr_max), SF_AUTO);

  // show the final statistics
  snprintf(data->msg_gauge_label, sizeof(data->msg_gauge_label), tr(MSG_TR_MESSAGEGAUGE), data->Msgs_Tot);
  xset(data->GA_COUNT, MUIA_Gauge_InfoText, data->msg_gauge_label,
                       MUIA_Gauge_Max,      data->Msgs_Tot,
                       MUIA_Gauge_Current,  data->Msgs_Tot);

  snprintf(data->size_gauge_label, sizeof(data->size_gauge_label), tr(MSG_TR_TRANSFERSIZE),
                                                         data->str_size_curr_max, data->str_size_curr_max);
  xset(data->GA_BYTES, MUIA_Gauge_InfoText, data->size_gauge_label,
                       MUIA_Gauge_Max,      100,
                       MUIA_Gauge_Current,  100);

  data->started = FALSE;

  LEAVE();
  return 0;
}

///
/// DECLARE(Next)
// advance to the next mail
DECLARE(Next) // int index, int listpos, ULONG size, const char *status
{
  GETDATA;

  ENTER();

  // update the stats only if the transfer has been started already
  if(data->started == TRUE)
  {
    data->Msgs_Curr = msg->index;
    data->Msgs_ListPos = msg->listpos;
    data->Msgs_Done++;
    data->Size_Curr = 0;
    data->Size_Curr_Max = msg->size;

    // format the current mail's size ahead of any refresh
    FormatSize(msg->size, data->str_size_curr_max, sizeof(data->str_size_curr_max), SF_AUTO);

    DoUpdateStats(data, 0, msg->status);
  }

  LEAVE();
  return 0;
}

///
/// DECLARE(Update)
// update the statistics, the visible display will be refreshed
// 4 times per second at most
DECLARE(Update) // int size_incr, const char *status
{
  GETDATA;

  ENTER();

  // update the stats only if the transfer has been started already
  if(data->started == TRUE)
    DoUpdateStats(data, msg->size_incr, msg->status);

  LEAVE();
  return 0;
}

///
