/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2014 YAM Open Source Team

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
#include <mui/NList_mcc.h>
#include <mui/NListview_mcc.h>

#include "mui/AttachmentKeywordList.h"
#include "mui/CodesetPopup.h"
#include "mui/ConfigPage.h"
#include "mui/ConfigPageList.h"

#include "Config.h"
#include "Requesters.h"

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
  Object *CH_ATTACHMENTREMINDER;
  Object *LV_ATTACHMENTKEYWORDS;
  Object *NL_ATTACHMENTKEYWORDS;
  Object *ST_ATTACHMENTKEYWORD;
  Object *BT_KEYWORDADD;
  Object *BT_KEYWORDREMOVE;
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
  Object *CH_ATTACHMENTREMINDER;
  Object *LV_ATTACHMENTKEYWORDS;
  Object *NL_ATTACHMENTKEYWORDS;
  Object *ST_ATTACHMENTKEYWORD;
  Object *BT_KEYWORDADD;
  Object *BT_KEYWORDREMOVE;
  Object *CH_LAUNCH;

  ENTER();

  wrapmode[0] = tr(MSG_CO_EWOff);
  wrapmode[1] = tr(MSG_CO_EWAsYouType);
  wrapmode[2] = NULL; //tr(MSG_CO_EWBeforeSend);
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
          Child, PO_DEFCODESET_WRITE = CodesetPopupObject,
            MUIA_CodesetPopup_ControlChar, tr(MSG_CO_DEFAULTCODESET_WRITE),
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

      Child, ColGroup(2), GroupFrameT(tr(MSG_CO_OtherOptions)),

        Child, CH_WARNSUBJECT = MakeCheck(tr(MSG_CO_WARNSUBJECT)),
        Child, HGroup,
          Child, LLabel(tr(MSG_CO_WARNSUBJECT)),
          Child, HSpace(0),
        End,

        Child, CH_LAUNCH = MakeCheck(tr(MSG_CO_LAUNCH_EXTEDITOR)),
        Child, HGroup,
          Child, LLabel(tr(MSG_CO_LAUNCH_EXTEDITOR)),
          Child, HSpace(0),
        End,

        Child, CH_ATTACHMENTREMINDER = MakeCheck(tr(MSG_CO_CHECK_FOR_MISSING_ATTACHMENTS)),
        Child, HGroup,
          Child, LLabel(tr(MSG_CO_CHECK_FOR_MISSING_ATTACHMENTS)),
          Child, HSpace(0),
        End,

        Child, HSpace(1),
        Child, HGroup,
          Child, VGroup,
            Child, Label(tr(MSG_CO_ATTACHMENT_KEYWORDS)),
            Child, VSpace(0),
          End,
          Child, VGroup,
            MUIA_Group_Spacing, 0,
            Child, LV_ATTACHMENTKEYWORDS = NListviewObject,
              MUIA_CycleChain, TRUE,
              MUIA_FixHeightTxt, "\n\n\n\n",
              MUIA_NListview_NList, NL_ATTACHMENTKEYWORDS = AttachmentKeywordListObject,
              End,
            End,
            Child, HGroup,
              MUIA_Group_Spacing, 1,
              Child, ST_ATTACHMENTKEYWORD = MakeString(SIZE_DEFAULT, tr(MSG_CO_ATTACHMENT_KEYWORDS)),
              Child, HGroup,
                MUIA_Weight, 0,
                MUIA_Group_Spacing, 1,
                MUIA_Group_SameWidth, TRUE,
                Child, BT_KEYWORDADD = MakeButton(MUIX_B "+" MUIX_N),
                Child, BT_KEYWORDREMOVE = MakeButton(MUIX_B "-" MUIX_N),
              End,
            End,
          End,
        End,

      End,
    End,
  TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;

    data->ST_HELLOTEXT =          ST_HELLOTEXT;
    data->ST_BYETEXT =            ST_BYETEXT;
    data->PO_DEFCODESET_WRITE =   PO_DEFCODESET_WRITE;
    data->ST_EDWRAP =             ST_EDWRAP;
    data->CY_EDWRAP =             CY_EDWRAP;
    data->NB_EMAILCACHE =         NB_EMAILCACHE;
    data->NB_AUTOSAVE =           NB_AUTOSAVE;
    data->CH_FIXEDFONT_WRITE =    CH_FIXEDFONT_WRITE;
    data->CH_TEXTCOLORS_WRITE =   CH_TEXTCOLORS_WRITE;
    data->CH_TEXTSTYLES_WRITE =   CH_TEXTSTYLES_WRITE;
    data->CH_WARNSUBJECT =        CH_WARNSUBJECT;
    data->CH_ATTACHMENTREMINDER = CH_ATTACHMENTREMINDER;
    data->LV_ATTACHMENTKEYWORDS = LV_ATTACHMENTKEYWORDS;
    data->NL_ATTACHMENTKEYWORDS = NL_ATTACHMENTKEYWORDS;
    data->ST_ATTACHMENTKEYWORD =  ST_ATTACHMENTKEYWORD;
    data->BT_KEYWORDADD =         BT_KEYWORDADD;
    data->BT_KEYWORDREMOVE =      BT_KEYWORDREMOVE;
    data->CH_LAUNCH =             CH_LAUNCH;

    SetHelp(ST_HELLOTEXT,          MSG_HELP_CO_ST_HELLOTEXT);
    SetHelp(ST_BYETEXT,            MSG_HELP_CO_ST_BYETEXT);
    SetHelp(ST_EDWRAP,             MSG_HELP_CO_ST_EDWRAP);
    SetHelp(CY_EDWRAP,             MSG_HELP_CO_CY_EDWRAP);
    SetHelp(CH_WARNSUBJECT,        MSG_HELP_CO_CH_WARNSUBJECT);
    SetHelp(CH_ATTACHMENTREMINDER, MSG_HELP_CO_CH_ATTACHMENTREMINDER);
    SetHelp(LV_ATTACHMENTKEYWORDS, MSG_HELP_CO_LV_ATTACHMENTKEYWORDS);
    SetHelp(CH_LAUNCH,             MSG_HELP_CO_CH_LAUNCH);
    SetHelp(NB_EMAILCACHE,         MSG_HELP_CO_NB_EMAILCACHE);
    SetHelp(NB_AUTOSAVE,           MSG_HELP_CO_NB_AUTOSAVE);
    SetHelp(PO_DEFCODESET_WRITE,   MSG_HELP_CO_TX_DEFCODESET_WRITE);
    SetHelp(CH_TEXTSTYLES_WRITE,   MSG_HELP_CO_CH_TEXTSTYLES_WRITE);
    SetHelp(CH_TEXTCOLORS_WRITE,   MSG_HELP_CO_CH_TEXTCOLORS_WRITE);

    DoMethod(CY_EDWRAP,             MUIM_Notify, MUIA_Cycle_Active,       MUIV_EveryTime, ST_EDWRAP,             3, MUIM_Set, MUIA_Disabled, MUIV_NotTriggerValue);
    DoMethod(CH_ATTACHMENTREMINDER, MUIM_Notify, MUIA_Selected,           MUIV_EveryTime, obj,                   3, METHOD(UpdateButtons), CH_ATTACHMENTREMINDER, MUIV_NotTriggerValue);
    DoMethod(NL_ATTACHMENTKEYWORDS, MUIM_Notify, MUIA_NList_Active,       MUIV_EveryTime, obj,                   3, METHOD(UpdateButtons), NL_ATTACHMENTKEYWORDS, FALSE);
    DoMethod(BT_KEYWORDADD,         MUIM_Notify, MUIA_Pressed,            FALSE,          obj,                   1, METHOD(AddAttachmentKeyword));
    DoMethod(BT_KEYWORDREMOVE,      MUIM_Notify, MUIA_Pressed,            FALSE,          NL_ATTACHMENTKEYWORDS, 2, MUIM_NList_Remove, MUIV_NList_Remove_Active);
    DoMethod(ST_ATTACHMENTKEYWORD,  MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime, NL_ATTACHMENTKEYWORDS, 2, MUIM_AttachmentKeywordList_ModifyKeyword, MUIV_TriggerValue);
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
  set(data->NL_ATTACHMENTKEYWORDS, MUIA_AttachmentKeywordList_Keywords, CE->AttachmentKeywords);
  xset(data->ST_EDWRAP,
    MUIA_String_Integer, CE->EdWrapCol,
    MUIA_Disabled, CE->EdWrapMode == EWM_OFF);
  setcycle(data->CY_EDWRAP, CE->EdWrapMode);
  setcheckmark(data->CH_LAUNCH, CE->LaunchAlways);
  setslider(data->NB_EMAILCACHE, CE->EmailCache);
  setslider(data->NB_AUTOSAVE, CE->AutoSave/60);
  nnset(data->PO_DEFCODESET_WRITE,  MUIA_CodesetPopup_Codeset, CE->DefaultWriteCodeset);
  setcheckmark(data->CH_FIXEDFONT_WRITE, CE->UseFixedFontWrite);
  setcheckmark(data->CH_TEXTSTYLES_WRITE, CE->UseTextStylesWrite);
  setcheckmark(data->CH_TEXTCOLORS_WRITE, CE->UseTextColorsWrite);
  setcheckmark(data->CH_ATTACHMENTREMINDER, CE->AttachmentReminder);

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
  DoMethod(data->NL_ATTACHMENTKEYWORDS, MUIM_AttachmentKeywordList_GetKeywords, CE->AttachmentKeywords, sizeof(CE->AttachmentKeywords));
  CE->EdWrapCol         = GetMUIInteger(data->ST_EDWRAP);
  CE->EdWrapMode        = GetMUICycle  (data->CY_EDWRAP);
  CE->LaunchAlways      = GetMUICheck  (data->CH_LAUNCH);
  CE->EmailCache        = GetMUINumer  (data->NB_EMAILCACHE);
  CE->AutoSave          = GetMUINumer  (data->NB_AUTOSAVE)*60; // in seconds
  strlcpy(CE->DefaultWriteCodeset, (char *)xget(data->PO_DEFCODESET_WRITE, MUIA_CodesetPopup_Codeset), sizeof(CE->DefaultWriteCodeset));
  CE->UseFixedFontWrite  = GetMUICheck(data->CH_FIXEDFONT_WRITE);
  CE->UseTextStylesWrite = GetMUICheck(data->CH_TEXTSTYLES_WRITE);
  CE->UseTextColorsWrite = GetMUICheck(data->CH_TEXTCOLORS_WRITE);
  CE->AttachmentReminder = GetMUICheck(data->CH_ATTACHMENTREMINDER);

  RETURN(0);
  return 0;
}

///

/* Public Methods */
/// DECLARE(UpdateButtons)
DECLARE(UpdateButtons) // Object *sender, ULONG state
{
  GETDATA;
  char *activeKeyword;

  ENTER();

  if(msg->sender == data->CH_ATTACHMENTREMINDER)
  {
    DoMethod(obj, MUIM_MultiSet, MUIA_Disabled, msg->state,
      data->LV_ATTACHMENTKEYWORDS,
      data->ST_ATTACHMENTKEYWORD,
      data->BT_KEYWORDADD,
      NULL);
  }

  DoMethod(data->NL_ATTACHMENTKEYWORDS, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &activeKeyword);
  set(data->ST_ATTACHMENTKEYWORD, MUIA_String_Contents, activeKeyword);
  set(data->BT_KEYWORDREMOVE, MUIA_Disabled, msg->state || activeKeyword == NULL);

  RETURN(0);
  return 0;
}

///
/// DECLARE(AddAttachmentKeyword)
DECLARE(AddAttachmentKeyword)
{
  GETDATA;
  char *newKeyword;
  ENTER();

  newKeyword = (char *)xget(data->ST_ATTACHMENTKEYWORD, MUIA_String_Contents);
  if(IsStrEmpty(newKeyword) == FALSE)
    DoMethod(data->NL_ATTACHMENTKEYWORDS, MUIM_AttachmentKeywordList_AddKeyword, newKeyword);
  else
    set(_win(obj), MUIA_Window_ActiveObject, data->ST_ATTACHMENTKEYWORD);

  RETURN(0);
  return 0;
}

///
