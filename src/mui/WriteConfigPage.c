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
 Description: "Write" configuration page

***************************************************************************/

#include "WriteConfigPage_cl.h"

#include <proto/muimaster.h>

#include "mui/CodesetPopobject.h"
#include "mui/ConfigPage.h"
#include "mui/ConfigPageList.h"

#include "Config.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *ST_HELLOTEXT;
  Object *ST_BYETEXT;
  Object *PO_DEFCODESET_WRITE;
  Object *ST_EDWRAP;
  Object *CY_EDWRAP;
  Object *NB_EMAILCACHE;
  Object *NB_AUTOSAVE;
  Object *CH_FIXEDFONT_WRITE;
  Object *CH_TEXTCOLORS_WRITE;
  Object *CH_TEXTSTYLES_WRITE;
  Object *CH_WARNSUBJECT;
  Object *CH_LAUNCH;
};
*/

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  static const char *wrapmode[4];
  Object *ST_HELLOTEXT;
  Object *ST_BYETEXT;
  Object *PO_DEFCODESET_WRITE;
  Object *ST_EDWRAP;
  Object *CY_EDWRAP;
  Object *NB_EMAILCACHE;
  Object *NB_AUTOSAVE;
  Object *CH_FIXEDFONT_WRITE;
  Object *CH_TEXTCOLORS_WRITE;
  Object *CH_TEXTSTYLES_WRITE;
  Object *CH_WARNSUBJECT;
  Object *CH_LAUNCH;

  ENTER();

  wrapmode[0] = tr(MSG_CO_EWOff);
  wrapmode[1] = tr(MSG_CO_EWAsYouType);
  wrapmode[2] = tr(MSG_CO_EWBeforeSend);
  wrapmode[3] = NULL;

  if((obj = DoSuperNew(cl, obj,
    MUIA_HelpNode, "Configuration#Write",
    MUIA_ConfigPage_Page, cp_Write,
    MUIA_ConfigPage_Contents, VGroup,
      Child, VGroup, GroupFrameT(tr(MSG_CO_MessageBody)),
        Child, ColGroup(2),
          Child, Label2(tr(MSG_CO_Welcome)),
          Child, ST_HELLOTEXT = MakeString(SIZE_INTRO,tr(MSG_CO_Welcome)),

          Child, Label2(tr(MSG_CO_Greetings)),
          Child, ST_BYETEXT = MakeString(SIZE_INTRO,tr(MSG_CO_Greetings)),

          Child, Label2(tr(MSG_CO_DEFAULTCODESET_WRITE)),
          Child, PO_DEFCODESET_WRITE = CodesetPopobjectObject,
            MUIA_CodesetPopobject_ControlChar, tr(MSG_CO_DEFAULTCODESET_WRITE),
          End,
        End,
      End,

      Child, VGroup, GroupFrameT(tr(MSG_CO_Editor)),
        Child, ColGroup(2),

          Child, Label2(tr(MSG_CO_WordWrap)),
          Child, HGroup,
            Child, ST_EDWRAP = MakeInteger(3, tr(MSG_CO_WordWrap)),
            Child, CY_EDWRAP = MakeCycle(wrapmode, ""),
          End,

          Child, Label2(tr(MSG_CO_NB_EMAILCACHE)),
          Child, HGroup,
            Child, NB_EMAILCACHE = NumericbuttonObject,
              MUIA_CycleChain,      TRUE,
              MUIA_Numeric_Min,     0,
              MUIA_Numeric_Max,     100,
              MUIA_Numeric_Format,  tr(MSG_CO_NB_EMAILCACHEFMT),
            End,
            Child, HSpace(0),
          End,

          Child, Label2(tr(MSG_CO_NB_AUTOSAVE)),
          Child, HGroup,
            Child, NB_AUTOSAVE = NumericbuttonObject,
              MUIA_CycleChain,      TRUE,
              MUIA_Numeric_Min,     0,
              MUIA_Numeric_Max,     30,
              MUIA_Numeric_Format,  tr(MSG_CO_NB_AUTOSAVEFMT),
            End,
            Child, HSpace(0),
          End,

        End,

        Child, RectangleObject,
          MUIA_VertWeight,          0,
          MUIA_Rectangle_HBar,      TRUE,
          MUIA_Rectangle_BarTitle,  tr(MSG_CO_FONTSETTINGS),
        End,
        Child, MakeCheckGroup(&CH_FIXEDFONT_WRITE, tr(MSG_CO_FIXEDFONT_WRITE)),
        Child, MakeCheckGroup(&CH_TEXTCOLORS_WRITE, tr(MSG_CO_TEXTCOLORS_WRITE)),
        Child, MakeCheckGroup(&CH_TEXTSTYLES_WRITE, tr(MSG_CO_TEXTSTYLES_WRITE)),

      End,

      Child, VGroup, GroupFrameT(tr(MSG_CO_OtherOptions)),
        Child, MakeCheckGroup(&CH_WARNSUBJECT, tr(MSG_CO_WARNSUBJECT)),
        Child, MakeCheckGroup(&CH_LAUNCH, tr(MSG_CO_LAUNCH_EXTEDITOR)),
      End,
    End,
  TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;

    data->ST_HELLOTEXT =        ST_HELLOTEXT;
    data->ST_BYETEXT =          ST_BYETEXT;
    data->PO_DEFCODESET_WRITE = PO_DEFCODESET_WRITE;
    data->ST_EDWRAP =           ST_EDWRAP;
    data->CY_EDWRAP =           CY_EDWRAP;
    data->NB_EMAILCACHE =       NB_EMAILCACHE;
    data->NB_AUTOSAVE =         NB_AUTOSAVE;
    data->CH_FIXEDFONT_WRITE =  CH_FIXEDFONT_WRITE;
    data->CH_TEXTCOLORS_WRITE = CH_TEXTCOLORS_WRITE;
    data->CH_TEXTSTYLES_WRITE = CH_TEXTSTYLES_WRITE;
    data->CH_WARNSUBJECT =      CH_WARNSUBJECT;
    data->CH_LAUNCH =           CH_LAUNCH;

    SetHelp(ST_HELLOTEXT,        MSG_HELP_CO_ST_HELLOTEXT);
    SetHelp(ST_BYETEXT,          MSG_HELP_CO_ST_BYETEXT);
    SetHelp(CH_WARNSUBJECT,      MSG_HELP_CO_CH_WARNSUBJECT);
    SetHelp(ST_EDWRAP,           MSG_HELP_CO_ST_EDWRAP);
    SetHelp(CY_EDWRAP,           MSG_HELP_CO_CY_EDWRAP);
    SetHelp(CH_LAUNCH,           MSG_HELP_CO_CH_LAUNCH);
    SetHelp(NB_EMAILCACHE,       MSG_HELP_CO_NB_EMAILCACHE);
    SetHelp(NB_AUTOSAVE,         MSG_HELP_CO_NB_AUTOSAVE);
    SetHelp(PO_DEFCODESET_WRITE, MSG_HELP_CO_TX_DEFCODESET_WRITE);
    SetHelp(CH_TEXTSTYLES_WRITE, MSG_HELP_CO_CH_TEXTSTYLES_WRITE);
    SetHelp(CH_TEXTCOLORS_WRITE, MSG_HELP_CO_CH_TEXTCOLORS_WRITE);

    DoMethod(CY_EDWRAP, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime, ST_EDWRAP, 3, MUIM_Set, MUIA_Disabled, MUIV_NotTriggerValue);
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

  setstring(data->ST_HELLOTEXT, CE->NewIntro);
  setstring(data->ST_BYETEXT, CE->Greetings);
  setcheckmark(data->CH_WARNSUBJECT, CE->WarnSubject);
  xset(data->ST_EDWRAP, MUIA_String_Integer, CE->EdWrapCol,
					    MUIA_Disabled, CE->EdWrapMode == EWM_OFF);
  setcycle(data->CY_EDWRAP, CE->EdWrapMode);
  setcheckmark(data->CH_LAUNCH, CE->LaunchAlways);
  setslider(data->NB_EMAILCACHE, CE->EmailCache);
  setslider(data->NB_AUTOSAVE, CE->AutoSave/60);
  nnset(data->PO_DEFCODESET_WRITE,  MUIA_CodesetPopobject_Codeset, CE->DefaultWriteCodeset);
  setcheckmark(data->CH_FIXEDFONT_WRITE, CE->UseFixedFontWrite);
  setcheckmark(data->CH_TEXTSTYLES_WRITE, CE->UseTextStylesWrite);
  setcheckmark(data->CH_TEXTCOLORS_WRITE, CE->UseTextColorsWrite);

  RETURN(0);
  return 0;
}

///
/// OVERLOAD(MUIM_ConfigPage_GUIToConfig)
OVERLOAD(MUIM_ConfigPage_GUIToConfig)
{
  GETDATA;

  ENTER();

  GetMUIString(CE->NewIntro, data->ST_HELLOTEXT, sizeof(CE->NewIntro));
  GetMUIString(CE->Greetings, data->ST_BYETEXT, sizeof(CE->Greetings));
  CE->WarnSubject       = GetMUICheck  (data->CH_WARNSUBJECT);
  CE->EdWrapCol         = GetMUIInteger(data->ST_EDWRAP);
  CE->EdWrapMode        = GetMUICycle  (data->CY_EDWRAP);
  CE->LaunchAlways      = GetMUICheck  (data->CH_LAUNCH);
  CE->EmailCache        = GetMUINumer  (data->NB_EMAILCACHE);
  CE->AutoSave          = GetMUINumer  (data->NB_AUTOSAVE)*60; // in seconds
  strlcpy(CE->DefaultWriteCodeset, (char *)xget(data->PO_DEFCODESET_WRITE, MUIA_CodesetPopobject_Codeset), sizeof(CE->DefaultWriteCodeset));
  CE->UseFixedFontWrite  = GetMUICheck(data->CH_FIXEDFONT_WRITE);
  CE->UseTextStylesWrite = GetMUICheck(data->CH_TEXTSTYLES_WRITE);
  CE->UseTextColorsWrite = GetMUICheck(data->CH_TEXTCOLORS_WRITE);

  RETURN(0);
  return 0;
}

///
