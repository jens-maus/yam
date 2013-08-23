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

#include "ReplyForwardConfigPage_cl.h"

#include <proto/muimaster.h>

#include "mui/ConfigPage.h"
#include "mui/PlaceholderPopupList.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *ST_REPLYHI;
  Object *ST_REPLYTEXT;
  Object *ST_REPLYBYE;
  Object *ST_AREPLYHI;
  Object *ST_AREPLYTEXT;
  Object *ST_AREPLYBYE;
  Object *ST_AREPLYPAT;
  Object *ST_MREPLYHI;
  Object *ST_MREPLYTEXT;
  Object *ST_MREPLYBYE;
  Object *CH_COMPADDR;
  Object *CH_QUOTEEMPTY;
  Object *CH_STRIPSIG;
  Object *CY_FORWARDMODE;
  Object *ST_FWDSTART;
  Object *ST_FWDEND;
};
*/

/* Private functions */
/// MakePhraseGroup
// creates a cycle/string gadgets for forward and reply phrases
static Object *MakePhraseGroup(Object **hello, Object **intro, Object **bye,
                               const char *label, const char *help)
{
  Object *grp, *cycl, *pgrp;
  Object *popButton;
  static const char *cytext[4];

  cytext[0] = tr(MSG_CO_PhraseOpen);
  cytext[1] = tr(MSG_CO_PhraseIntro);
  cytext[2] = tr(MSG_CO_PhraseClose);
  cytext[3] = NULL;

  if((grp = HGroup,
    MUIA_Group_HorizSpacing, 1,
    Child, cycl = CycleObject,
      MUIA_CycleChain, 1,
      MUIA_Font, MUIV_Font_Button,
      MUIA_Cycle_Entries, cytext,
      MUIA_ControlChar, ShortCut(label),
      MUIA_Weight, 0,
    End,
    Child, pgrp = PageGroup,
      Child, MakeVarPop(hello, &popButton, PHM_REPLYHELLO, SIZE_INTRO, ""),
      Child, MakeVarPop(intro, &popButton, PHM_REPLYINTRO, SIZE_INTRO, ""),
      Child, MakeVarPop(bye,   &popButton, PHM_REPLYBYE,   SIZE_INTRO, ""),
    End,
    MUIA_ShortHelp, help,
  End))
  {
    DoMethod(cycl, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime, pgrp, 3, MUIM_Set, MUIA_Group_ActivePage, MUIV_TriggerValue);
  }

  return grp;
}

///

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  static const char *fwdmode[3];
  Object *ST_REPLYHI;
  Object *ST_REPLYTEXT;
  Object *ST_REPLYBYE;
  Object *ST_AREPLYHI;
  Object *ST_AREPLYTEXT;
  Object *ST_AREPLYBYE;
  Object *ST_AREPLYPAT;
  Object *ST_MREPLYHI;
  Object *ST_MREPLYTEXT;
  Object *ST_MREPLYBYE;
  Object *CH_COMPADDR;
  Object *CH_QUOTEEMPTY;
  Object *CH_STRIPSIG;
  Object *CY_FORWARDMODE;
  Object *ST_FWDSTART;
  Object *ST_FWDEND;
  Object *popButton;

  ENTER();

  fwdmode[0] = tr(MSG_CO_FWDMSG_ATTACH);
  fwdmode[1] = tr(MSG_CO_FWDMSG_INLINE);
  fwdmode[2] = NULL;

  if((obj = DoSuperNew(cl, obj,
    MUIA_HelpNode, "Configuration#ReplyForward",
    MUIA_ConfigPage_Page, cp_ReplyForward,
    MUIA_ConfigPage_Contents, VGroup,
      Child, VGroup, GroupFrameT(tr(MSG_CO_Replying)),
        Child, ColGroup(2),
          Child, Label2(tr(MSG_CO_RepInit)),
          Child, MakePhraseGroup(&ST_REPLYHI, &ST_REPLYTEXT, &ST_REPLYBYE, tr(MSG_CO_RepInit), tr(MSG_HELP_CO_ST_REPLYTEXT)),

          Child, Label2(tr(MSG_CO_AltRepInit)),
          Child, MakePhraseGroup(&ST_AREPLYHI, &ST_AREPLYTEXT, &ST_AREPLYBYE, tr(MSG_CO_AltRepInit), tr(MSG_HELP_CO_ST_AREPLYTEXT)),

          Child, HSpace(1),
          Child, HGroup,
            Child, Label2(tr(MSG_CO_AltRepPat)),
            Child, ST_AREPLYPAT = MakeString(SIZE_PATTERN, tr(MSG_CO_AltRepPat)),
          End,

          Child, Label2(tr(MSG_CO_MLRepInit)),
          Child, MakePhraseGroup(&ST_MREPLYHI, &ST_MREPLYTEXT, &ST_MREPLYBYE, tr(MSG_CO_MLRepInit), tr(MSG_HELP_CO_ST_MREPLYTEXT)),

          Child, HSpace(1),
          Child, MakeCheckGroup(&CH_COMPADDR, tr(MSG_CO_VerifyAddress)),

          Child, HSpace(1),
          Child, MakeCheckGroup(&CH_QUOTEEMPTY, tr(MSG_CO_QuoteEmpty)),

          Child, HSpace(1),
          Child, MakeCheckGroup(&CH_STRIPSIG, tr(MSG_CO_StripSignature)),

        End,
      End,

      Child, ColGroup(2), GroupFrameT(tr(MSG_CO_Forwarding)),

        Child, Label2(tr(MSG_CO_FWDMSG)),
        Child, CY_FORWARDMODE = MakeCycle(fwdmode, tr(MSG_CO_FWDMSG)),

        Child, Label2(tr(MSG_CO_FwdInit)),
        Child, MakeVarPop(&ST_FWDSTART, &popButton, PHM_FORWARD, SIZE_INTRO, tr(MSG_CO_FwdInit)),

        Child, Label2(tr(MSG_CO_FwdFinish)),
        Child, MakeVarPop(&ST_FWDEND, &popButton, PHM_FORWARD, SIZE_INTRO, tr(MSG_CO_FwdFinish)),

      End,
    End,
  TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;

    data->ST_REPLYHI =     ST_REPLYHI;
    data->ST_REPLYTEXT =   ST_REPLYTEXT;
    data->ST_REPLYBYE =    ST_REPLYBYE;
    data->ST_AREPLYHI =    ST_AREPLYHI;
    data->ST_AREPLYTEXT =  ST_AREPLYTEXT;
    data->ST_MREPLYBYE =   ST_MREPLYBYE;
    data->ST_MREPLYHI =    ST_MREPLYHI;
    data->ST_MREPLYTEXT =  ST_MREPLYTEXT;
    data->ST_AREPLYBYE =   ST_MREPLYBYE;
    data->CH_COMPADDR =    CH_COMPADDR;
    data->CH_QUOTEEMPTY =  CH_QUOTEEMPTY;
    data->CH_STRIPSIG =    CH_STRIPSIG;
    data->CY_FORWARDMODE = CY_FORWARDMODE;
    data->ST_FWDSTART =    ST_FWDSTART;
    data->ST_FWDEND =      ST_FWDEND;

    SetHelp(ST_FWDSTART,    MSG_HELP_CO_ST_FWDSTART);
    SetHelp(ST_FWDEND,      MSG_HELP_CO_ST_FWDEND);
    SetHelp(ST_AREPLYPAT,   MSG_HELP_CO_ST_AREPLYPAT);
    SetHelp(CH_QUOTEEMPTY,  MSG_HELP_CO_CH_QUOTEEMPTY);
    SetHelp(CH_COMPADDR,    MSG_HELP_CO_CH_COMPADDR);
    SetHelp(CH_STRIPSIG,    MSG_HELP_CO_CH_STRIPSIG);
    SetHelp(CY_FORWARDMODE, MSG_HELP_CO_CY_FORWARDMODE);
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

  setstring(data->ST_REPLYHI, CE->ReplyHello);
  setstring(data->ST_REPLYTEXT, CE->ReplyIntro);
  setstring(data->ST_REPLYBYE, CE->ReplyBye);
  setstring(data->ST_AREPLYHI, CE->AltReplyHello);
  setstring(data->ST_AREPLYTEXT, CE->AltReplyIntro);
  setstring(data->ST_AREPLYBYE, CE->AltReplyBye);
  setstring(data->ST_AREPLYPAT, CE->AltReplyPattern);
  setstring(data->ST_MREPLYHI, CE->MLReplyHello);
  setstring(data->ST_MREPLYTEXT, CE->MLReplyIntro);
  setstring(data->ST_MREPLYBYE, CE->MLReplyBye);
  setstring(data->ST_FWDSTART, CE->ForwardIntro);
  setstring(data->ST_FWDEND, CE->ForwardFinish);
  setcheckmark(data->CH_QUOTEEMPTY, CE->QuoteEmptyLines);
  setcheckmark(data->CH_COMPADDR, CE->CompareAddress);
  setcheckmark(data->CH_STRIPSIG, CE->StripSignature);
  setcycle(data->CY_FORWARDMODE, CE->ForwardMode);

  RETURN(0);
  return 0;
}

///
/// OVERLOAD(MUIM_ConfigPage_GUIToConfig)
OVERLOAD(MUIM_ConfigPage_GUIToConfig)
{
  GETDATA;

  ENTER();

  GetMUIString(CE->ReplyHello, data->ST_REPLYHI, sizeof(CE->ReplyHello));
  GetMUIString(CE->ReplyIntro, data->ST_REPLYTEXT, sizeof(CE->ReplyIntro));
  GetMUIString(CE->ReplyBye, data->ST_REPLYBYE, sizeof(CE->ReplyBye));
  GetMUIString(CE->AltReplyHello, data->ST_AREPLYHI, sizeof(CE->AltReplyHello));
  GetMUIString(CE->AltReplyIntro, data->ST_AREPLYTEXT, sizeof(CE->AltReplyIntro));
  GetMUIString(CE->AltReplyBye, data->ST_AREPLYBYE, sizeof(CE->AltReplyBye));
  GetMUIString(CE->AltReplyPattern, data->ST_AREPLYPAT, sizeof(CE->AltReplyPattern));
  GetMUIString(CE->MLReplyHello, data->ST_MREPLYHI, sizeof(CE->MLReplyHello));
  GetMUIString(CE->MLReplyIntro, data->ST_MREPLYTEXT, sizeof(CE->MLReplyIntro));
  GetMUIString(CE->MLReplyBye, data->ST_MREPLYBYE, sizeof(CE->MLReplyBye));
  GetMUIString(CE->ForwardIntro, data->ST_FWDSTART, sizeof(CE->ForwardIntro));
  GetMUIString(CE->ForwardFinish, data->ST_FWDEND, sizeof(CE->ForwardFinish));
  CE->QuoteEmptyLines = GetMUICheck(data->CH_QUOTEEMPTY);
  CE->CompareAddress =  GetMUICheck(data->CH_COMPADDR);
  CE->StripSignature =  GetMUICheck(data->CH_STRIPSIG);
  CE->ForwardMode =     GetMUICycle(data->CY_FORWARDMODE);

  RETURN(0);
  return 0;
}

///
