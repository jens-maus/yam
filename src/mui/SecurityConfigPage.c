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
 Description: "Security" configuration page

***************************************************************************/

#include "SecurityConfigPage_cl.h"

#include <proto/asl.h>
#include <proto/muimaster.h>

#include "mui/ConfigPage.h"
#include "mui/ConfigPageList.h"

#include "Config.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *ST_PGPCMD;
  Object *CH_PGPPASSINTERVAL;
  Object *NB_PGPPASSINTERVAL;
  Object *CY_LOGMODE;
  Object *PO_LOGFILE;
  Object *ST_LOGFILE;
  Object *CH_SPLITLOG;
  Object *CH_LOGALL;
};
*/

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  static const char *logfmode[4];
  Object *ST_PGPCMD;
  Object *CH_PGPPASSINTERVAL;
  Object *NB_PGPPASSINTERVAL;
  Object *CY_LOGMODE;
  Object *PO_LOGFILE;
  Object *ST_LOGFILE;
  Object *CH_SPLITLOG;
  Object *CH_LOGALL;

  ENTER();

  logfmode[0] = tr(MSG_CO_LogNone);
  logfmode[1] = tr(MSG_CO_LogNormal);
  logfmode[2] = tr(MSG_CO_LogVerbose);
  logfmode[3] = NULL;

  if((obj = DoSuperNew(cl, obj,
    MUIA_HelpNode, "Configuration#Security",
    MUIA_ConfigPage_Page, cp_Security,
    MUIA_ConfigPage_Contents, VGroup,
      Child, VGroup, GroupFrameT("PGP (Pretty Good Privacy)"),
        Child, ColGroup(2),

          Child, Label2(tr(MSG_CO_PGPExe)),
          Child, PopaslObject,
            MUIA_Popasl_Type,      ASL_FileRequest,
            MUIA_Popstring_String, ST_PGPCMD = MakeString(SIZE_PATHFILE,tr(MSG_CO_PGPExe)),
            MUIA_Popstring_Button, PopButton(MUII_PopDrawer),
            ASLFR_DrawersOnly, TRUE,
          End,

          Child, HSpace(1),
          Child, HGroup,
            Child, CH_PGPPASSINTERVAL = MakeCheck(tr(MSG_CO_PGPPASSINTERVAL1)),
            Child, Label2(tr(MSG_CO_PGPPASSINTERVAL1)),
            Child, NB_PGPPASSINTERVAL = MakeNumeric(1, 90, FALSE),
            Child, Label2(tr(MSG_CO_PGPPASSINTERVAL2)),
            Child, HSpace(0),
          End,

        End,
      End,

      Child, ColGroup(2), GroupFrameT(tr(MSG_CO_Logfiles)),
        Child, Label1(tr(MSG_CO_LogMode)),
        Child, CY_LOGMODE = MakeCycle(logfmode, tr(MSG_CO_LogMode)),

        Child, Label2(tr(MSG_CO_LogPath)),
        Child, PO_LOGFILE = PopaslObject,
          MUIA_Popasl_Type, ASL_FileRequest,
          MUIA_Popstring_String, ST_LOGFILE = MakeString(SIZE_PATH,tr(MSG_CO_LogPath)),
          MUIA_Popstring_Button, PopButton(MUII_PopDrawer),
          ASLFR_DrawersOnly, TRUE,
        End,

        Child, HSpace(1),
        Child, MakeCheckGroup(&CH_SPLITLOG, tr(MSG_CO_LogSplit)),

        Child, HSpace(1),
        Child, MakeCheckGroup(&CH_LOGALL, tr(MSG_CO_LogAllEvents)),

      End,
    End,
  TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;

    data->ST_PGPCMD =          ST_PGPCMD;
    data->CH_PGPPASSINTERVAL = CH_PGPPASSINTERVAL;
    data->NB_PGPPASSINTERVAL = NB_PGPPASSINTERVAL;
    data->CY_LOGMODE =         CY_LOGMODE;
    data->PO_LOGFILE =         PO_LOGFILE;
    data->ST_LOGFILE =         ST_LOGFILE;
    data->CH_SPLITLOG =        CH_SPLITLOG;
    data->CH_LOGALL =          CH_LOGALL;

    SetHelp(ST_PGPCMD,          MSG_HELP_CO_ST_PGPCMD);
    SetHelp(CH_PGPPASSINTERVAL, MSG_HELP_CO_PGPPASSINTERVAL);
    SetHelp(NB_PGPPASSINTERVAL, MSG_HELP_CO_PGPPASSINTERVAL);
    SetHelp(ST_LOGFILE,         MSG_HELP_CO_ST_LOGFILE);
    SetHelp(CH_SPLITLOG,        MSG_HELP_CO_CH_SPLITLOG);
    SetHelp(CY_LOGMODE,         MSG_HELP_CO_CY_LOGMODE);
    SetHelp(CH_LOGALL,          MSG_HELP_CO_CH_LOGALL);

    DoMethod(CY_LOGMODE, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime, MUIV_Notify_Application, 7, MUIM_MultiSet, MUIA_Disabled, MUIV_NotTriggerValue, PO_LOGFILE,
                                                                                                                                                         CH_SPLITLOG,
                                                                                                                                                         CH_LOGALL,
                                                                                                                                                         NULL);

    DoMethod(CH_PGPPASSINTERVAL, MUIM_Notify, MUIA_Selected, MUIV_EveryTime, NB_PGPPASSINTERVAL, 3, MUIM_Set, MUIA_Disabled, MUIV_NotTriggerValue);
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

  setstring(data->ST_PGPCMD, CE->PGPCmdPath);
  setstring(data->ST_LOGFILE, CE->LogfilePath);
  setcycle(data->CY_LOGMODE, CE->LogfileMode);
  setcheckmark(data->CH_SPLITLOG, CE->SplitLogfile);
  setcheckmark(data->CH_LOGALL, CE->LogAllEvents);
  setcheckmark(data->CH_PGPPASSINTERVAL, CE->PGPPassInterval > 0);

  xset(data->NB_PGPPASSINTERVAL, MUIA_Numeric_Value, abs(CE->PGPPassInterval),
                                MUIA_Disabled, CE->PGPPassInterval <= 0);

  DoMethod(obj, MUIM_MultiSet, MUIA_Disabled, CE->LogfileMode == LF_NONE, data->PO_LOGFILE,
                                                                          data->CH_SPLITLOG,
                                                                          data->CH_LOGALL,
                                                                          NULL);

  RETURN(0);
  return 0;
}

///
/// OVERLOAD(MUIM_ConfigPage_GUIToConfig)
OVERLOAD(MUIM_ConfigPage_GUIToConfig)
{
  GETDATA;

  ENTER();

  GetMUIString(CE->PGPCmdPath, data->ST_PGPCMD, sizeof(CE->PGPCmdPath));
  GetMUIString(CE->LogfilePath, data->ST_LOGFILE, sizeof(CE->LogfilePath));
  CE->LogfileMode = GetMUICycle(data->CY_LOGMODE);
  CE->SplitLogfile = GetMUICheck(data->CH_SPLITLOG);
  CE->LogAllEvents = GetMUICheck(data->CH_LOGALL);

  CE->PGPPassInterval = GetMUINumer(data->NB_PGPPASSINTERVAL);
  if(GetMUICheck(data->CH_PGPPASSINTERVAL) == FALSE)
    CE->PGPPassInterval = -CE->PGPPassInterval;

  RETURN(0);
  return 0;
}

///
