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
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.   See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 YAM Official Support Site : http://www.yam.ch
 YAM OpenSource project     : http://sourceforge.net/projects/yamos/

 $Id$

 Superclass:  MUIC_ConfigPage
 Description: "Startup/Quit" configuration page

***************************************************************************/

#include "StartupQuitConfigPage_cl.h"

#include <proto/muimaster.h>

#include "mui/ConfigPage.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *CH_LOADALL;
  Object *CH_MARKNEW;
  Object *CH_DELETESTART;
  Object *CH_REMOVESTART;
  Object *CH_CHECKBD;
  Object *CH_SENDSTART;
  Object *CH_SENDQUIT;
  Object *CH_DELETEQUIT;
  Object *CH_REMOVEQUIT;
};
*/

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  Object *CH_LOADALL;
  Object *CH_MARKNEW;
  Object *CH_DELETESTART;
  Object *CH_REMOVESTART;
  Object *CH_CHECKBD;
  Object *CH_SENDSTART;
  Object *CH_SENDQUIT;
  Object *CH_DELETEQUIT;
  Object *CH_REMOVEQUIT;

  ENTER();

  if((obj = DoSuperNew(cl, obj,
    MUIA_HelpNode, "Configuration#StartQuit",
    MUIA_ConfigPage_Page, cp_StartupQuit,
    MUIA_ConfigPage_Contents, VGroup,
      Child, VGroup, GroupFrameT(tr(MSG_CO_OnStartup)),
        Child, MakeCheckGroup(&CH_LOADALL, tr(MSG_CO_LoadAll)),
        Child, MakeCheckGroup(&CH_MARKNEW, tr(MSG_CO_MarkNew)),
        Child, MakeCheckGroup(&CH_DELETESTART, tr(MSG_CO_DeleteOld)),
        Child, MakeCheckGroup(&CH_REMOVESTART, tr(MSG_CO_RemoveDel)),
        Child, MakeCheckGroup(&CH_CHECKBD, tr(MSG_CO_CheckDOB)),
        Child, MakeCheckGroup(&CH_SENDSTART, tr(MSG_CO_SendStart)),
      End,
      Child, VGroup, GroupFrameT(tr(MSG_CO_OnTermination)),
        Child, MakeCheckGroup(&CH_SENDQUIT, tr(MSG_CO_SendStart)),
        Child, MakeCheckGroup(&CH_DELETEQUIT, tr(MSG_CO_DeleteOld)),
        Child, MakeCheckGroup(&CH_REMOVEQUIT, tr(MSG_CO_RemoveDel)),
      End,
    End,
  TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;

    data->CH_LOADALL =     CH_LOADALL;
    data->CH_MARKNEW =     CH_MARKNEW;
    data->CH_DELETESTART = CH_DELETESTART;
    data->CH_REMOVESTART = CH_REMOVESTART;
    data->CH_CHECKBD =     CH_CHECKBD;
    data->CH_SENDSTART =   CH_SENDSTART;
    data->CH_SENDQUIT =    CH_SENDQUIT;
    data->CH_DELETEQUIT =  CH_DELETEQUIT;
    data->CH_REMOVEQUIT =  CH_REMOVEQUIT;

    SetHelp(CH_LOADALL,     MSG_HELP_CO_CH_LOADALL  );
    SetHelp(CH_MARKNEW,     MSG_HELP_CO_CH_MARKNEW  );
    SetHelp(CH_DELETESTART, MSG_HELP_CO_CH_DELETEOLD);
    SetHelp(CH_REMOVESTART, MSG_HELP_CO_CH_REMOVEDEL);
    SetHelp(CH_SENDSTART,   MSG_HELP_CO_CH_SEND     );
    SetHelp(CH_CHECKBD,     MSG_HELP_CO_CH_CHECKBD  );
    SetHelp(CH_SENDQUIT,    MSG_HELP_CO_CH_SEND     );
    SetHelp(CH_DELETEQUIT,  MSG_HELP_CO_CH_DELETEOLD);
    SetHelp(CH_REMOVEQUIT,  MSG_HELP_CO_CH_REMOVEDEL);
  }

  RETURN((IPTR)obj);
  return (IPTR)obj;
}

///
/// OVERLOAD(MUIM_ConfigPage_ConfigToGUI)
OVERLOAD(MUIM_ConfigPage_ConfigToGUI)
{
  GETDATA;

  ENTER();

  setcheckmark(data->CH_SENDSTART,   CE->SendOnStartup);
  setcheckmark(data->CH_DELETESTART, CE->CleanupOnStartup);
  setcheckmark(data->CH_REMOVESTART, CE->RemoveOnStartup);
  setcheckmark(data->CH_LOADALL,     CE->LoadAllFolders);
  setcheckmark(data->CH_MARKNEW,     CE->UpdateNewMail);
  setcheckmark(data->CH_CHECKBD,     CE->CheckBirthdates);
  setcheckmark(data->CH_SENDQUIT,    CE->SendOnQuit);
  setcheckmark(data->CH_DELETEQUIT,  CE->CleanupOnQuit);
  setcheckmark(data->CH_REMOVEQUIT,  CE->RemoveOnQuit);

  RETURN(0);
  return 0;
}

///
/// OVERLOAD(MUIM_ConfigPage_GUIToConfig)
OVERLOAD(MUIM_ConfigPage_GUIToConfig)
{
  GETDATA;

  ENTER();

  CE->SendOnStartup    = GetMUICheck(data->CH_SENDSTART);
  CE->CleanupOnStartup = GetMUICheck(data->CH_DELETESTART);
  CE->RemoveOnStartup  = GetMUICheck(data->CH_REMOVESTART);
  CE->LoadAllFolders   = GetMUICheck(data->CH_LOADALL);
  CE->UpdateNewMail    = GetMUICheck(data->CH_MARKNEW);
  CE->CheckBirthdates  = GetMUICheck(data->CH_CHECKBD);
  CE->SendOnQuit       = GetMUICheck(data->CH_SENDQUIT);
  CE->CleanupOnQuit    = GetMUICheck(data->CH_DELETEQUIT);
  CE->RemoveOnQuit     = GetMUICheck(data->CH_REMOVEQUIT);

  RETURN(0);
  return 0;
}

///
