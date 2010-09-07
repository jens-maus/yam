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
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.   See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 YAM Official Support Site : http://www.yam.ch
 YAM OpenSource project     : http://sourceforge.net/projects/yamos/

 $Id$

 Superclass:  MUIC_Group
 Description: Displays statistics about a transfer in progress

***************************************************************************/

#include "TransferControlGroup_cl.h"

#include "MUIObjects.h"

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
  Object *preselectionList;

  struct Connection *conn;

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

  BOOL aborted;

  char stats_label[SIZE_DEFAULT];
  char size_gauge_label[SIZE_DEFAULT];
  char msg_gauge_label[SIZE_DEFAULT];
  char str_size_done[SIZE_SMALL];
  char str_size_tot[SIZE_SMALL];
  char str_speed[SIZE_SMALL];
  char str_size_curr[SIZE_SMALL];
  char str_size_curr_max[SIZE_SMALL];
};
*/

/* EXPORT
#define TCG_SETMAX   (-1)
*/

/* Hooks */

/* Private Functions */

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  Object *TX_STATS;
  Object *TX_STATUS;
  Object *GA_COUNT;
  Object *GA_BYTES;
  Object *BT_ABORT;

  if((obj = DoSuperNew(cl, obj,

    MUIA_Group_Columns, 2,
    GroupFrameT(tr(MSG_TR_Status)),
    Child, TX_STATS = TextObject,
      MUIA_Background,    MUII_TextBack,
      MUIA_Frame,         MUIV_Frame_Text,
      MUIA_Text_PreParse, MUIX_C,
    End,
    Child, VGroup,
      Child, GA_COUNT = GaugeObject,
        GaugeFrame,
        MUIA_Gauge_Horiz, TRUE,
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

    SetHelp(data->TX_STATUS, MSG_HELP_TR_TX_STATUS);
    SetHelp(data->BT_ABORT, MSG_HELP_TR_BT_ABORT);

    DoMethod(data->BT_ABORT, MUIM_Notify, MUIA_Pressed, FALSE, obj, 3, MUIM_Set, ATTR(Aborted), TRUE);

    // prepare the initial text object content
    DoMethod(obj, METHOD(Reset));
  }

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
      case ATTR(PreselectionList):
        data->preselectionList = (Object *)tag->ti_Data;
      break;

      case ATTR(Connection):
        data->conn = (struct Connection *)tag->ti_Data;
      break;

      case ATTR(NumberOfMails):
        data->Msgs_Tot = tag->ti_Data;
      break;

      case ATTR(TotalSize):
        data->Size_Tot = tag->ti_Data;
      break;

      case ATTR(Aborted):
        D(DBF_ALWAYS, "set aborted=%ld",tag->ti_Data);
        data->aborted = (BOOL)tag->ti_Data;
        if(data->conn != NULL)
          data->conn->abort = (BOOL)tag->ti_Data;
      break;
    }
  }

  return DoSuperMethodA(cl, obj, msg);
}

///
/// OVERLOAD(OM_GET)
OVERLOAD(OM_GET)
{
  GETDATA;
  IPTR *store = ((struct opGet *)msg)->opg_Storage;

  switch(((struct opGet *)msg)->opg_AttrID)
  {
    case ATTR(NumberOfMails): *store = data->Msgs_Tot; return TRUE;
    case ATTR(NumberOfProcessedMails): *store = data->Msgs_Done; return TRUE;
    case ATTR(Aborted): *store = (data->aborted == TRUE || (data->conn != NULL && data->conn->abort == TRUE)); return TRUE;
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
  set(data->GA_COUNT, MUIA_Gauge_InfoText, data->msg_gauge_label);
  set(data->GA_BYTES, MUIA_Gauge_InfoText, data->size_gauge_label);

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
DECLARE(Start)
{
  GETDATA;

  ENTER();

  data->Msgs_Done = 0;
  data->Size_Done = 0;

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

  data->Msgs_Curr = msg->index;
  data->Msgs_ListPos = msg->listpos;
  data->Msgs_Done++;
  data->Size_Curr = 0;
  data->Size_Curr_Max = msg->size;

  // format the current mail's size ahead of any refresh
  FormatSize(msg->size, data->str_size_curr_max, sizeof(data->str_size_curr_max), SF_AUTO);

  DoMethod(obj, METHOD(Update), 0, msg->status);

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

  if(msg->size_incr > 0)
  {
    data->Size_Done += msg->size_incr;
    data->Size_Curr += msg->size_incr;
  }
  else if(msg->size_incr == TCG_SETMAX)
  {
    // first update the total transferred size
    data->Size_Done += data->Size_Curr_Max - data->Size_Curr;
    // we are done with this mail, so make sure the current size equals the final size
    data->Size_Curr = data->Size_Curr_Max;
  }

  // if the window isn't open we don't need to update it, do we?
  if(xget(_win(obj), MUIA_Window_Open) == TRUE)
  {
    // update the stats 4 times per second at most
    if(TimeHasElapsed(&data->Clock_Last, 250000) == TRUE)
    {
      ULONG deltatime = data->Clock_Last.Seconds - data->Clock_Start;
      ULONG speed = 0;
      LONG remclock = 0;
      ULONG max;
      ULONG current;

      // if we have a preselection list then update this as well
      if(data->preselectionList != NULL && data->Msgs_ListPos >= 0)
        set(data->preselectionList, MUIA_NList_Active, data->Msgs_ListPos);

      // first we calculate the speed in bytes/sec
      // to display to the user
      if(deltatime != 0)
        speed = data->Size_Done / deltatime;

      // calculate the estimated remaining time
      if(speed != 0 && ((remclock = (data->Size_Tot / speed) - deltatime) < 0))
        remclock = 0;

      // show the current status
      set(data->TX_STATUS, MUIA_Text_Contents, msg->status);

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
                                  remclock / 60, remclock % 60);

      set(data->TX_STATS, MUIA_Text_Contents, data->stats_label);

      // update the gauge
      snprintf(data->size_gauge_label, sizeof(data->size_gauge_label), tr(MSG_TR_TRANSFERSIZE),
                                                             data->str_size_curr_max, data->str_size_curr_max);

      if(msg->size_incr == TCG_SETMAX)
      {
        max = 100;
        current = 100;
      }
      else if(data->Size_Curr_Max <= 65536)
      {
        max = data->Size_Curr_Max;
        current = data->Size_Curr;
      }
      else
      {
        max = data->Size_Curr_Max / 1024;
        current = data->Size_Curr / 1024;
      }
      xset(data->GA_BYTES, MUIA_Gauge_InfoText, data->size_gauge_label,
                           MUIA_Gauge_Max,      max,
                           MUIA_Gauge_Current,  current);

      // signal the application to update now
      DoMethod(G->App, MUIM_Application_InputBuffered);
    }
  }

  LEAVE();
  return 0;
}

///
