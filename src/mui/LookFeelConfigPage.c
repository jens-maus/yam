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
 Description: "Look&Feel" configuration page

***************************************************************************/

#include "LookFeelConfigPage_cl.h"

#include <proto/muimaster.h>

#include "mui/ConfigPage.h"
#include "mui/PlaceholderPopupList.h"
#include "mui/ThemeListGroup.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *GR_THEMES;
  Object *CY_FOLDERINFO;
  Object *CH_FCOLS[FOCOLNUM];
  Object *CH_FCNTMENU;
  Object *CH_MCOLS[MACOLNUM];
  Object *CH_MCNTMENU;
  Object *CY_INFOBARPOS;
  Object *PO_INFOBARTXT;
  Object *ST_INFOBARTXT;
  Object *CY_QUICKSEARCHBARPOS;
  Object *CY_SIZE;
  Object *CH_EMBEDDEDREADPANE;
  Object *CH_FIXFLIST;
  Object *CH_BEAT;
  Object *CH_RELDATETIME;
  Object *CH_ABOOKLOOKUP;
  Object *CH_FOLDERDBLCLICK;
};
*/

/* EXPORT
#include "YAM_config.h" // for FOCOLNUM and MACOLNUM
*/

/* Private functions */
/// MakeStaticCheck
// creates non-interactive checkmark gadget
static Object *MakeStaticCheck(void)
{
  return ImageObject,
    ImageButtonFrame,
    MUIA_Image_Spec,   MUII_CheckMark,
    MUIA_Background,   MUII_ButtonBack,
    MUIA_ShowSelState, FALSE,
    MUIA_Selected,     TRUE,
    MUIA_Disabled,     TRUE,
  End;
}

///

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  static const char *rtitles[3];
  static const char *sizef[6];
  static const char *infob[5];
  static const char *qsearchb[4];
  static const char *folderf[6];
  Object *GR_THEMES;
  Object *CY_FOLDERINFO;
  Object *CH_FCOLS[FOCOLNUM];
  Object *CH_FCNTMENU;
  Object *CH_MCOLS[MACOLNUM];
  Object *CH_MCNTMENU;
  Object *CY_INFOBARPOS;
  Object *PO_INFOBARTXT;
  Object *ST_INFOBARTXT;
  Object *CY_QUICKSEARCHBARPOS;
  Object *CY_SIZE;
  Object *CH_EMBEDDEDREADPANE;
  Object *CH_FIXFLIST;
  Object *CH_BEAT;
  Object *CH_RELDATETIME;
  Object *CH_ABOOKLOOKUP;
  Object *CH_FOLDERDBLCLICK;
  Object *popButton;

  ENTER();

  rtitles[0] = tr(MSG_CO_LOOKFEEL_THEMES);
  rtitles[1] = tr(MSG_CO_LOOKFEEL_MAINWINDOW);
  rtitles[2] = NULL;

  sizef[0] = tr(MSG_CO_SIZEFORMAT01);
  sizef[1] = tr(MSG_CO_SIZEFORMAT02);
  sizef[2] = tr(MSG_CO_SIZEFORMAT03);
  sizef[3] = tr(MSG_CO_SIZEFORMAT04);
  sizef[4] = tr(MSG_CO_SIZEFORMAT05);
  sizef[5] = NULL;

  infob[0] = tr(MSG_CO_INFOBARPOS04);
  infob[1] = tr(MSG_CO_INFOBARPOS01);
  infob[2] = tr(MSG_CO_INFOBARPOS02);
  infob[3] = tr(MSG_CO_INFOBARPOS03);
  infob[4] = NULL;

  qsearchb[0] = tr(MSG_CO_QUICKSEARCHBARPOS01);
  qsearchb[1] = tr(MSG_CO_QUICKSEARCHBARPOS02);
  qsearchb[2] = tr(MSG_CO_QUICKSEARCHBARPOS03);
  qsearchb[3] = NULL;

  folderf[0] = tr(MSG_CO_FOLDERINFO01);
  folderf[1] = tr(MSG_CO_FOLDERINFO02);
  folderf[2] = tr(MSG_CO_FOLDERINFO03);
  folderf[3] = tr(MSG_CO_FOLDERINFO04);
  folderf[4] = tr(MSG_CO_FOLDERINFO05);
  folderf[5] = NULL;

  if((obj = DoSuperNew(cl, obj,
    MUIA_HelpNode, "Configuration#LookFeel",
    MUIA_ConfigPage_Page, cp_LookFeel,
    MUIA_ConfigPage_UseScrollgroup, FALSE,
    MUIA_ConfigPage_AddSpacer, FALSE,
    MUIA_ConfigPage_Contents, RegisterGroup(rtitles),
      MUIA_CycleChain, TRUE,

      // Themes settings
      Child, ScrollgroupObject,
        MUIA_Scrollgroup_FreeHoriz, FALSE,
        MUIA_Scrollgroup_AutoBars, TRUE,
        MUIA_Scrollgroup_Contents, VGroupV,

          Child, GR_THEMES = ThemeListGroupObject,
          End,

        End,
      End,

      // Main window settings
      Child, ScrollgroupObject,
        MUIA_Scrollgroup_FreeHoriz, FALSE,
        MUIA_Scrollgroup_AutoBars, TRUE,
        MUIA_Scrollgroup_Contents, VGroupV,

          // List column settings
          Child, HGroup, GroupFrameT(tr(MSG_CO_LISTCOLUMNSETTINGS)),

            // Folder list columns
            Child, VGroup,

              Child, HBarT(tr(MSG_FolderList)), End,

              Child, ColGroup(3),
                MUIA_ShortHelp, tr(MSG_HELP_CO_CG_FO),

                Child, MakeStaticCheck(),
                Child, CY_FOLDERINFO = MakeCycle(folderf, tr(MSG_CO_FOLDERLABEL)),
                Child, HSpace(0),

                Child, CH_FCOLS[1] = MakeCheck(""),
                Child, LLabel(tr(MSG_Total)),
                Child, HSpace(0),

                Child, CH_FCOLS[2] = MakeCheck(""),
                Child, LLabel(tr(MSG_Unread)),
                Child, HSpace(0),

                Child, CH_FCOLS[3] = MakeCheck(""),
                Child, LLabel(tr(MSG_New)),
                Child, HSpace(0),

                Child, CH_FCOLS[4] = MakeCheck(""),
                Child, LLabel(tr(MSG_Size)),
                Child, HSpace(0),

                Child, CH_FCNTMENU = MakeCheck(""),
                Child, LLabel(tr(MSG_CO_CONTEXTMENU)),
                Child, HSpace(0),

              End,

              Child, HVSpace,

            End,

            Child, HSpace(8),

            // Message list columns
            Child, VGroup,

              Child, HBarT(tr(MSG_MessageList)), End,

              Child, ColGroup(3),
                MUIA_ShortHelp, tr(MSG_HELP_CO_CG_MA),

                Child, MakeStaticCheck(),
                Child, LLabel(tr(MSG_Status)),
                Child, HSpace(0),

                Child, CH_MCOLS[1] = MakeCheck(""),
                Child, LLabel(tr(MSG_SenderRecpt)),
                Child, HSpace(0),

                Child, CH_MCOLS[2] = MakeCheck(""),
                Child, LLabel(tr(MSG_ReturnAddress)),
                Child, HSpace(0),

                Child, CH_MCOLS[3] = MakeCheck(""),
                Child, LLabel(tr(MSG_Subject)),
                Child, HSpace(0),

                Child, CH_MCOLS[4] = MakeCheck(""),
                Child, LLabel(tr(MSG_MessageDate)),
                Child, HSpace(0),

                Child, CH_MCOLS[5] = MakeCheck(""),
                Child, LLabel(tr(MSG_Size)),
                Child, HSpace(0),

                Child, CH_MCOLS[6] = MakeCheck(""),
                Child, LLabel(tr(MSG_Filename)),
                Child, HSpace(0),

                Child, CH_MCOLS[7] = MakeCheck(""),
                Child, LLabel(tr(MSG_CO_DATE_SNTRCVD)),
                Child, HSpace(0),

                Child, CH_MCNTMENU = MakeCheck(""),
                Child, LLabel(tr(MSG_CO_CONTEXTMENU)),
                Child, HSpace(0),
              End,
            End,
          End,

          // InfoBar settings
          Child, ColGroup(2), GroupFrameT(tr(MSG_CO_INFOBAR)),
            Child, Label1(tr(MSG_CO_INFOBARPOS)),
            Child, CY_INFOBARPOS = MakeCycle(infob, tr(MSG_CO_INFOBARPOS)),

            Child, Label2(tr(MSG_CO_FOLDERLABEL)),
            Child, PO_INFOBARTXT = MakeVarPop(&ST_INFOBARTXT, &popButton, PHM_MAILSTATS, SIZE_DEFAULT, tr(MSG_CO_FOLDERLABEL)),
          End,

          // QuicksearchBar settings
          Child, ColGroup(2), GroupFrameT(tr(MSG_CO_QUICKSEARCHBAR)),
            Child, Label1(tr(MSG_CO_QUICKSEARCHBARPOS)),
            Child, CY_QUICKSEARCHBARPOS = MakeCycle(qsearchb, tr(MSG_CO_QUICKSEARCHBARPOS)),
          End,

          Child, ColGroup(2),
            GroupFrameT(tr(MSG_CO_GENLISTCFG)),

            Child, Label1(tr(MSG_CO_SIZEFORMAT)),
            Child, CY_SIZE = MakeCycle(sizef, tr(MSG_CO_SIZEFORMAT)),

            Child, HSpace(1),
            Child, MakeCheckGroup(&CH_EMBEDDEDREADPANE, tr(MSG_CO_SHOWEMBEDDEDREADPANE)),

            Child, HSpace(1),
            Child, MakeCheckGroup(&CH_FIXFLIST, tr(MSG_CO_FixedFontList)),

            Child, HSpace(1),
            Child, MakeCheckGroup(&CH_BEAT, tr(MSG_CO_SwatchBeat)),

            Child, HSpace(1),
            Child, MakeCheckGroup(&CH_RELDATETIME, tr(MSG_CO_RELDATETIME)),

            Child, HSpace(1),
            Child, MakeCheckGroup(&CH_ABOOKLOOKUP, tr(MSG_CO_ABOOKLOOKUP)),

            Child, HSpace(1),
            Child, MakeCheckGroup(&CH_FOLDERDBLCLICK, tr(MSG_CO_FOLDERDBLCLICK)),

          End,

          Child, HVSpace,

        End,
      End,
    End,
  TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;

    data->GR_THEMES =            GR_THEMES;
    data->CY_FOLDERINFO =        CY_FOLDERINFO;
    data->CH_FCOLS[1] =          CH_FCOLS[1];
    data->CH_FCOLS[2] =          CH_FCOLS[2];
    data->CH_FCOLS[3] =          CH_FCOLS[3];
    data->CH_FCOLS[4] =          CH_FCOLS[4];
    data->CH_FCNTMENU =          CH_FCNTMENU;
    data->CH_MCOLS[1] =          CH_MCOLS[1];
    data->CH_MCOLS[2] =          CH_MCOLS[2];
    data->CH_MCOLS[3] =          CH_MCOLS[3];
    data->CH_MCOLS[4] =          CH_MCOLS[4];
    data->CH_MCOLS[5] =          CH_MCOLS[5];
    data->CH_MCOLS[6] =          CH_MCOLS[6];
    data->CH_MCOLS[7] =          CH_MCOLS[7];
    data->CH_MCNTMENU =          CH_MCNTMENU;
    data->CY_INFOBARPOS =        CY_INFOBARPOS;
    data->PO_INFOBARTXT =        PO_INFOBARTXT;
    data->ST_INFOBARTXT =        ST_INFOBARTXT;
    data->CY_QUICKSEARCHBARPOS = CY_QUICKSEARCHBARPOS;
    data->CY_SIZE =              CY_SIZE;
    data->CH_EMBEDDEDREADPANE =  CH_EMBEDDEDREADPANE;
    data->CH_FIXFLIST =          CH_FIXFLIST;
    data->CH_BEAT =              CH_BEAT;
    data->CH_RELDATETIME =       CH_RELDATETIME;
    data->CH_ABOOKLOOKUP =       CH_ABOOKLOOKUP;
    data->CH_FOLDERDBLCLICK =    CH_FOLDERDBLCLICK;

    SetHelp(CY_INFOBARPOS,        MSG_HELP_CO_CH_INFOBAR);
    SetHelp(ST_INFOBARTXT,        MSG_HELP_CO_ST_INFOBARTXT);
    SetHelp(CY_QUICKSEARCHBARPOS, MSG_HELP_CO_CH_QUICKSEARCHBAR);
    SetHelp(CH_EMBEDDEDREADPANE,  MSG_HELP_CO_CH_EMBEDDEDREADPANE);
    SetHelp(CY_SIZE,              MSG_HELP_CO_CY_SIZE);
    SetHelp(CH_FIXFLIST,          MSG_HELP_CO_CH_FIXFLIST);
    SetHelp(CH_BEAT,              MSG_HELP_CO_CH_BEAT);
    SetHelp(CH_RELDATETIME,       MSG_HELP_CO_CH_RELDATETIME);
    SetHelp(CH_ABOOKLOOKUP,       MSG_HELP_CO_CH_ABOOKLOOKUP);
    SetHelp(CH_FCNTMENU,          MSG_HELP_CO_CONTEXTMENU);
    SetHelp(CH_MCNTMENU,          MSG_HELP_CO_CONTEXTMENU);
    SetHelp(CY_FOLDERINFO,        MSG_HELP_CO_CY_FOLDERINFO);

    DoMethod(CY_INFOBARPOS, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime, obj, 2, METHOD(InfoBarPosChanged), MUIV_TriggerValue);
  }

  RETURN((IPTR)obj);
  return (IPTR)obj;
}

///
/// OVERLOAD(MUIM_ConfigPage_ConfigToGUI)
OVERLOAD(MUIM_ConfigPage_ConfigToGUI)
{
  GETDATA;
  int i;

  ENTER();

  setcycle(data->CY_INFOBARPOS, CE->InfoBarPos);
  setstring(data->ST_INFOBARTXT, CE->InfoBarText);
  setcycle(data->CY_QUICKSEARCHBARPOS, CE->QuickSearchBarPos);
  setcheckmark(data->CH_EMBEDDEDREADPANE, CE->EmbeddedReadPane);
  setcycle(data->CY_SIZE, CE->SizeFormat);

  set(data->PO_INFOBARTXT, MUIA_Disabled, CE->InfoBarPos == IB_POS_OFF);

  // update the themeslist and set the current one
  // as active
  DoMethod(data->GR_THEMES, MUIM_ThemeListGroup_Update);

  for(i=1; i < FOCOLNUM; i++)
	setcheckmark(data->CH_FCOLS[i], isFlagSet(CE->FolderCols, (1<<i)));

  for(i=1; i < MACOLNUM; i++)
	setcheckmark(data->CH_MCOLS[i], isFlagSet(CE->MessageCols, (1<<i)));

  setcheckmark(data->CH_FIXFLIST, CE->FixedFontList);
  setcheckmark(data->CH_ABOOKLOOKUP, CE->ABookLookup);
  setcheckmark(data->CH_FCNTMENU, CE->FolderCntMenu);
  setcheckmark(data->CH_MCNTMENU, CE->MessageCntMenu);
  setcheckmark(data->CH_BEAT, (CE->DSListFormat == DSS_DATEBEAT || CE->DSListFormat == DSS_RELDATEBEAT));
  setcheckmark(data->CH_RELDATETIME, (CE->DSListFormat == DSS_RELDATETIME || CE->DSListFormat == DSS_RELDATEBEAT));
  setcheckmark(data->CH_FOLDERDBLCLICK, CE->FolderDoubleClick);
  setcycle(data->CY_FOLDERINFO, CE->FolderInfoMode);

  RETURN(0);
  return 0;
}

///
/// OVERLOAD(MUIM_ConfigPage_GUIToConfig)
OVERLOAD(MUIM_ConfigPage_GUIToConfig)
{
  GETDATA;
  int i;

  ENTER();

  CE->InfoBarPos = GetMUICycle(data->CY_INFOBARPOS);
  GetMUIString(CE->InfoBarText, data->ST_INFOBARTXT, sizeof(CE->InfoBarText));
  CE->QuickSearchBarPos = GetMUICycle(data->CY_QUICKSEARCHBARPOS);
  CE->EmbeddedReadPane = GetMUICheck  (data->CH_EMBEDDEDREADPANE);
  CE->SizeFormat = GetMUICycle(data->CY_SIZE);

  CE->FolderCols = 1;
  for(i=1; i < FOCOLNUM; i++)
  {
	if(GetMUICheck(data->CH_FCOLS[i]) == TRUE)
	  setFlag(CE->FolderCols, (1<<i));
  }

  CE->MessageCols = 1;
  for(i=1; i < MACOLNUM; i++)
  {
	if(GetMUICheck(data->CH_MCOLS[i]) == TRUE)
	  setFlag(CE->MessageCols, (1<<i));
  }

  CE->FixedFontList = GetMUICheck(data->CH_FIXFLIST);
  CE->ABookLookup = GetMUICheck(data->CH_ABOOKLOOKUP);
  CE->FolderCntMenu = GetMUICheck(data->CH_FCNTMENU);
  CE->MessageCntMenu = GetMUICheck(data->CH_MCNTMENU);
  CE->FolderDoubleClick = GetMUICheck(data->CH_FOLDERDBLCLICK);

  if(GetMUICheck(data->CH_BEAT) == TRUE)
  {
	if(GetMUICheck(data->CH_RELDATETIME) == TRUE)
	  CE->DSListFormat = DSS_RELDATEBEAT;
	else
	  CE->DSListFormat = DSS_DATEBEAT;
  }
  else
  {
	if(GetMUICheck(data->CH_RELDATETIME) == TRUE)
	  CE->DSListFormat = DSS_RELDATETIME;
	else
	  CE->DSListFormat = DSS_DATETIME;
  }

  CE->FolderInfoMode = GetMUICycle(data->CY_FOLDERINFO);

  RETURN(0);
  return 0;
}

///
/// DECLARE(InfoBarPosChanged)
// update the InfoBar contents string gadget according to the position setting
DECLARE(InfoBarPosChanged) // ULONG inactive
{
  GETDATA;

  ENTER();

  // disabling the Popstring object completely doesn't work, because on reactivation the string
  // gadget is not redrawn correctly (bug in MUI?), hence we do it separately.
  nnset(data->ST_INFOBARTXT, MUIA_Disabled, msg->inactive == TRUE);
  nnset((Object *)xget(data->PO_INFOBARTXT, MUIA_Popstring_Button), MUIA_Disabled, msg->inactive == TRUE);

  RETURN(0);
  return 0;
}

///
