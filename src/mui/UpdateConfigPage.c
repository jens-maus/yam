/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2015 YAM Open Source Team

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

 Superclass:  MUIC_ConfigPage
 Description: "Update" configuration page

***************************************************************************/

#include "UpdateConfigPage_cl.h"

#include <proto/muimaster.h>

#include "mui/ConfigPage.h"
#include "mui/ConfigPageList.h"
#include "mui/YAMApplication.h"

#include "Config.h"
#include "UpdateCheck.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *CY_UPDATEINTERVAL;
  Object *BT_UPDATENOW;
  Object *TX_UPDATEINFO;
  Object *TX_UPDATESTATUS;
  Object *TX_UPDATEDATE;
};
*/

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  static const char *updateInterval[5];
  Object *CY_UPDATEINTERVAL;
  Object *BT_UPDATENOW;
  Object *TX_UPDATEINFO;
  Object *TX_UPDATESTATUS;
  Object *TX_UPDATEDATE;

  ENTER();

  updateInterval[0] = tr(MSG_CO_UPDATE_NEVER);
  updateInterval[1] = tr(MSG_CO_UPDATE_DAILY);
  updateInterval[2] = tr(MSG_CO_UPDATE_WEEKLY);
  updateInterval[3] = tr(MSG_CO_UPDATE_MONTHLY);
  updateInterval[4] = NULL;

  if((obj = DoSuperNew(cl, obj,
    MUIA_HelpNode, "Configuration#Updates",
    MUIA_ConfigPage_Page, cp_Update,
    MUIA_ConfigPage_Contents, VGroup,
      Child, VGroup, GroupFrameT(tr(MSG_CO_SOFTWAREUPDATE)),
        Child, HGroup,
          Child, LLabel1(tr(MSG_CO_SEARCHFORUPDATES)),
          Child, CY_UPDATEINTERVAL = MakeCycle(updateInterval, tr(MSG_CO_SEARCHFORUPDATES)),
          Child, BT_UPDATENOW = MakeButton(tr(MSG_CO_SEARCHNOW)),
        End,
        Child, TX_UPDATEINFO = TextObject,
          MUIA_Font, MUIV_Font_Tiny,
          MUIA_Text_Copy, FALSE,
        End,
        Child, VSpace(10),
        Child, ColGroup(2),
          Child, LLabel1(tr(MSG_CO_LASTSEARCH)),
          Child, TX_UPDATESTATUS = TextObject,
            MUIA_Text_Contents, tr(MSG_CO_LASTSTATUS_NOCHECK),
            MUIA_Text_Copy, FALSE,
          End,
          Child, HSpace(1),
          Child, TX_UPDATEDATE = TextObject,
          End,
        End,
      End,
    End,
  TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;

    data->CY_UPDATEINTERVAL = CY_UPDATEINTERVAL;
    data->BT_UPDATENOW      = BT_UPDATENOW;
    data->TX_UPDATEINFO     = TX_UPDATEINFO;
    data->TX_UPDATESTATUS   = TX_UPDATESTATUS;
    data->TX_UPDATEDATE     = TX_UPDATEDATE;

    SetHelp(CY_UPDATEINTERVAL, MSG_HELP_CH_UPDATECHECK);
    SetHelp(BT_UPDATENOW,      MSG_HELP_CO_BT_UPDATENOW);

    DoMethod(CY_UPDATEINTERVAL, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime, obj, 2, METHOD(UpdateInfo), MUIV_TriggerValue);
    DoMethod(BT_UPDATENOW,      MUIM_Notify, MUIA_Pressed,      FALSE,          obj, 1, METHOD(UpdateCheck));
  }

  RETURN((IPTR)obj);
  return (IPTR)obj;
}

///
/// OVERLOAD(MUIM_ConfigPage_ConfigToGUI)
OVERLOAD(MUIM_ConfigPage_ConfigToGUI)
{
  GETDATA;
  struct UpdateState state;

  ENTER();

  // copy the last update state information
  GetLastUpdateState(&state);

  if(CE->UpdateInterval > 0)
  {
    if(CE->UpdateInterval <= 86400)
      setcycle(data->CY_UPDATEINTERVAL, 1); // daily
    else if(CE->UpdateInterval <= 604800)
      setcycle(data->CY_UPDATEINTERVAL, 2); // weekly
    else
      setcycle(data->CY_UPDATEINTERVAL, 3); // monthly
  }
  else
    setcycle(data->CY_UPDATEINTERVAL, 0);

  // now we set the information on the last update check
  switch(state.LastUpdateStatus)
  {
    case UST_NOCHECK:
      set(data->TX_UPDATESTATUS, MUIA_Text_Contents, tr(MSG_CO_LASTSTATUS_NOCHECK));
    break;

    case UST_NOUPDATE:
      set(data->TX_UPDATESTATUS, MUIA_Text_Contents, tr(MSG_CO_LASTSTATUS_NOUPDATE));
    break;

    case UST_NOQUERY:
      set(data->TX_UPDATESTATUS, MUIA_Text_Contents, tr(MSG_CO_LASTSTATUS_NOQUERY));
    break;

    case UST_UPDATESUCCESS:
      set(data->TX_UPDATESTATUS, MUIA_Text_Contents, tr(MSG_CO_LASTSTATUS_UPDATESUCCESS));
    break;
  }

  // set the lastUpdateCheckDate
  if(state.LastUpdateStatus != UST_NOCHECK && state.LastUpdateCheck.Seconds > 0)
  {
    char buf[SIZE_DEFAULT];

    TimeVal2String(buf, sizeof(buf), &state.LastUpdateCheck, DSS_DATETIME, TZC_NONE);
    set(data->TX_UPDATEDATE, MUIA_Text_Contents, buf);
  }
  else
  {
    // no update check was yet performed, so we clear our status gadgets
    set(data->TX_UPDATEDATE, MUIA_Text_Contents, "");
  }

  RETURN(0);
  return 0;
}

///
/// OVERLOAD(MUIM_ConfigPage_GUIToConfig)
OVERLOAD(MUIM_ConfigPage_GUIToConfig)
{
  GETDATA;

  ENTER();

  switch(GetMUICycle(data->CY_UPDATEINTERVAL))
  {
    default:
    case 0:
      CE->UpdateInterval = 0; // never
    break;

    case 1:
      CE->UpdateInterval = 86400; // 1 day
    break;

    case 2:
      CE->UpdateInterval = 604800; // 1 week
    break;

    case 3:
      CE->UpdateInterval = 2419200; // 1 month
    break;
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(UpdateInfo)
// update the information text about the update check
DECLARE(UpdateInfo) // ULONG interval
{
  GETDATA;

  ENTER();

  set(data->TX_UPDATEINFO, MUIA_Text_Contents, (msg->interval == 0) ? tr(MSG_CO_UPDATES_INFO_MANUAL) : tr(MSG_CO_UPDATES_INFO_AUTO));

  RETURN(0);
  return 0;
}

///
/// DECLARE(UpdateCheck)
// initiates an interactive update check
DECLARE(UpdateCheck)
{
  ENTER();

  // now we make sure the C and CE config structure is in sync again
  C->UpdateInterval = CE->UpdateInterval;

  // let the application check for updates
  DoMethod(_app(obj), MUIM_YAMApplication_UpdateCheck, FALSE);

  RETURN(0);
  return 0;
}

///
