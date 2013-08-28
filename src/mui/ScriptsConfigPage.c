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
 Description: "Scripts" configuration page

***************************************************************************/

#include "ScriptsConfigPage_cl.h"

#include <proto/asl.h>
#include <proto/muimaster.h>
#include <mui/NList_mcc.h>
#include <mui/NListview_mcc.h>

#include "SDI_hook.h"

#include "mui/ConfigPage.h"
#include "mui/ConfigPageList.h"
#include "mui/PlaceholderPopupList.h"
#include "mui/ScriptList.h"

#include "Config.h"
#include "MUIObjects.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *LV_REXX;
  Object *ST_RXNAME;
  Object *CY_ISADOS;
  Object *ST_SCRIPT;
  Object *PO_SCRIPT;
  Object *LV_SCRIPT;
  Object *CH_CONSOLE;
  Object *CH_WAITTERM;

  struct Hook FilereqStartHook;
  struct Hook FilereqStopHook;
};
*/

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  static const char *const stype[3] =
  {
    "ARexx", "AmigaDOS", NULL
  };
  Object *LV_REXX;
  Object *ST_RXNAME;
  Object *CY_ISADOS;
  Object *ST_SCRIPT;
  Object *PO_SCRIPT;
  Object *LV_SCRIPT;
  Object *CH_CONSOLE;
  Object *CH_WAITTERM;
  Object *popAsl;

  ENTER();

  if((obj = DoSuperNew(cl, obj,
    MUIA_HelpNode, "Configuration#Scripts",
    MUIA_ConfigPage_Page, cp_Security,
    MUIA_ConfigPage_AddSpacer, FALSE,
    MUIA_ConfigPage_Contents, VGroup,
      Child, VGroup,
        Child, NListviewObject,
          MUIA_CycleChain, TRUE,
          MUIA_NListview_NList, LV_REXX = ScriptListObject,
          End,
        End,
        Child, ColGroup(2),

          Child, Label2(tr(MSG_CO_Name)),
          Child, HGroup,
            Child, ST_RXNAME = MakeString(SIZE_NAME,tr(MSG_CO_Name)),
            Child, CY_ISADOS = CycleObject,
              MUIA_CycleChain,    TRUE,
              MUIA_Weight,        25,
              MUIA_Font,          MUIV_Font_Button,
              MUIA_Cycle_Entries, stype,
            End,
          End,

          Child, Label2(tr(MSG_CO_Script)),
          Child, HGroup,
            MUIA_Group_HorizSpacing, 0,
            Child, MakeVarPop(&ST_SCRIPT, &PO_SCRIPT, &LV_SCRIPT, PHM_SCRIPTS, SIZE_PATHFILE, tr(MSG_CO_Script)),
            Child, popAsl = PopaslObject,
               MUIA_Popasl_Type,      ASL_FileRequest,
               MUIA_Popstring_Button, PopButton(MUII_PopFile),
            End,
          End,

          Child, HSpace(1),
          Child, MakeCheckGroup(&CH_CONSOLE, tr(MSG_CO_OpenConsole)),

          Child, HSpace(1),
          Child, MakeCheckGroup(&CH_WAITTERM, tr(MSG_CO_WaitTerm)),

        End,
      End,

    End,
  TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;
    int i;

    data->LV_REXX =     LV_REXX;
    data->ST_RXNAME =   ST_RXNAME;
    data->CY_ISADOS =   CY_ISADOS;
    data->ST_SCRIPT =   ST_SCRIPT;
    data->PO_SCRIPT =   PO_SCRIPT;
    data->LV_SCRIPT =   LV_SCRIPT;
    data->CH_CONSOLE =  CH_CONSOLE;
    data->CH_WAITTERM = CH_WAITTERM;

    for(i = 1; i <= MAXRX; i++)
      DoMethod(LV_REXX, MUIM_NList_InsertSingle, i, MUIV_NList_Insert_Bottom);

    InitHook(&data->FilereqStartHook, FilereqStartHook, data->ST_SCRIPT);
    InitHook(&data->FilereqStopHook, FilereqStopHook, data->ST_SCRIPT);

    xset(popAsl,
      MUIA_Popasl_StartHook, &data->FilereqStartHook,
      MUIA_Popasl_StopHook,  &data->FilereqStopHook);

    SetHelp(ST_RXNAME,    MSG_HELP_CO_ST_RXNAME);
    SetHelp(ST_SCRIPT,    MSG_HELP_CO_ST_SCRIPT);
    SetHelp(CY_ISADOS,    MSG_HELP_CO_CY_ISADOS);
    SetHelp(CH_CONSOLE,   MSG_HELP_CO_CH_CONSOLE);
    SetHelp(CH_WAITTERM,  MSG_HELP_CO_CH_WAITTERM);
    SetHelp(LV_REXX,      MSG_HELP_CO_LV_REXX);

    DoMethod(LV_REXX,     MUIM_Notify, MUIA_NList_Active,    MUIV_EveryTime, obj, 1, METHOD(GetRXEntry));
    DoMethod(ST_RXNAME,   MUIM_Notify, MUIA_String_Contents, MUIV_EveryTime, obj, 1, METHOD(PutRXEntry));
    DoMethod(ST_SCRIPT,   MUIM_Notify, MUIA_String_Contents, MUIV_EveryTime, obj, 1, METHOD(PutRXEntry));
    DoMethod(CY_ISADOS,   MUIM_Notify, MUIA_Cycle_Active,    MUIV_EveryTime, obj, 1, METHOD(PutRXEntry));
    DoMethod(CH_CONSOLE,  MUIM_Notify, MUIA_Selected,        MUIV_EveryTime, obj, 1, METHOD(PutRXEntry));
    DoMethod(CH_WAITTERM, MUIM_Notify, MUIA_Selected,        MUIV_EveryTime, obj, 1, METHOD(PutRXEntry));
  }

  RETURN((IPTR)obj);
  return (IPTR)obj;
}

///
/// OVERLOAD(MUIM_ConfigPage_ConfigToGUI)
OVERLOAD(MUIM_ConfigPage_ConfigToGUI)
{
  GETDATA;
  int act;
  struct RxHook *rh;

  ENTER();

  act = xget(data->LV_REXX, MUIA_NList_Active);
  rh = &(CE->RX[act]);

  nnset(data->ST_RXNAME, MUIA_String_Contents, act < 10 ? rh->Name : "");
  nnset(data->ST_SCRIPT, MUIA_String_Contents, rh->Script);
  nnset(data->CY_ISADOS, MUIA_Cycle_Active, rh->IsAmigaDOS ? 1 : 0);
  nnset(data->CH_CONSOLE, MUIA_Selected, rh->UseConsole);
  nnset(data->CH_WAITTERM, MUIA_Selected, rh->WaitTerm);
  set(data->ST_RXNAME, MUIA_Disabled, act >= 10);
  set(data->LV_REXX, MUIA_NList_Active, 0);
  DoMethod(data->LV_REXX, MUIM_NList_Redraw, MUIV_NList_Redraw_All);

  RETURN(0);
  return 0;
}

///
/// OVERLOAD(MUIM_ConfigPage_GUIToConfig)
OVERLOAD(MUIM_ConfigPage_GUIToConfig)
{
  ENTER();

  // nothing to be done, but we must overload this method to avoid an
  // error message in the debug build.

  RETURN(0);
  return 0;
}

///
/// DECLARE(GetRXEntry)
// fills form with data from selected list entry
DECLARE(GetRXEntry)
{
  GETDATA;
  struct RxHook *rh;
  int act;
  enum Macro macro;

  ENTER();

  act = xget(data->LV_REXX, MUIA_NList_Active);
  rh = &(CE->RX[act]);
  macro = act;

  nnset(data->ST_RXNAME, MUIA_String_Contents, act < 10 ? rh->Name : "");
  nnset(data->ST_SCRIPT, MUIA_String_Contents, rh->Script);
  nnset(data->CY_ISADOS, MUIA_Cycle_Active, rh->IsAmigaDOS ? 1 : 0);
  nnset(data->CH_CONSOLE, MUIA_Selected, rh->UseConsole);
  nnset(data->CH_WAITTERM, MUIA_Selected, rh->WaitTerm);
  set(data->ST_RXNAME, MUIA_Disabled, act >= 10);

  switch(macro)
  {
    case MACRO_MEN0:
    case MACRO_MEN1:
    case MACRO_MEN2:
    case MACRO_MEN3:
    case MACRO_MEN4:
    case MACRO_MEN5:
    case MACRO_MEN6:
    case MACRO_MEN7:
    case MACRO_MEN8:
    case MACRO_MEN9:
    case MACRO_STARTUP:
    case MACRO_QUIT:
    case MACRO_PRESEND:
    case MACRO_POSTSEND:
    case MACRO_PREFILTER:
    case MACRO_POSTFILTER:
    default:
      // disable the popup button since these script don't take any parameter
      nnset(data->PO_SCRIPT, MUIA_Disabled, TRUE);
    break;

    case MACRO_PREGET:
    case MACRO_POSTGET:
    case MACRO_NEWMSG:
    case MACRO_READ:
    case MACRO_PREWRITE:
    case MACRO_POSTWRITE:
    case MACRO_URL:
      // enable the popup button
      nnset(data->PO_SCRIPT, MUIA_Disabled, FALSE);
    break;
  }

  DoMethod(data->LV_REXX, MUIM_NList_Redraw, act);

  set(data->LV_SCRIPT, MUIA_PlaceholderPopupList_ScriptEntry, macro);

  RETURN(0);
  return 0;
}

///
/// DECLARE(PutRXEntry)
// fills form data into selected list entry
DECLARE(PutRXEntry)
{
  GETDATA;
  int act;

  ENTER();

  act = xget(data->LV_REXX, MUIA_NList_Active);
  if(act != MUIV_List_Active_Off)
  {
    struct RxHook *rh = &(CE->RX[act]);

    GetMUIString(rh->Name, data->ST_RXNAME, sizeof(rh->Name));
    GetMUIString(rh->Script, data->ST_SCRIPT, sizeof(rh->Script));
    rh->IsAmigaDOS = GetMUICycle(data->CY_ISADOS) == 1;
    rh->UseConsole = GetMUICheck(data->CH_CONSOLE);
    rh->WaitTerm = GetMUICheck(data->CH_WAITTERM);

    DoMethod(data->LV_REXX, MUIM_NList_Redraw, act);
  }

  RETURN(0);
  return 0;
}

///
