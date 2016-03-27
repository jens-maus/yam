/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2016 YAM Open Source Team

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
 Description: "Spam" configuration page

***************************************************************************/

#include "SpamConfigPage_cl.h"

#include <proto/muimaster.h>
#include <mui/NList_mcc.h>
#include <mui/NListtree_mcc.h>

#include "YAM.h"
#include "YAM_error.h"

#include "mui/ConfigPage.h"
#include "mui/ConfigPageList.h"
#include "mui/FilterChooser.h"
#include "mui/MainWindowToolbar.h"

#include "Config.h"
#include "FileInfo.h"
#include "FolderList.h"
#include "Locale.h"
#include "MailList.h"
#include "Requesters.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *CH_SPAMFILTERENABLED;
  Object *TX_SPAMBADCOUNT;
  Object *TX_SPAMGOODCOUNT;
  Object *BT_SPAMRESETTRAININGDATA;
  Object *BT_OPTIMIZETRAININGDATA;
  Object *CH_SPAMFILTERFORNEWMAIL;
  Object *CH_SPAMABOOKISWHITELIST;
  Object *CH_SPAMMARKONMOVE;
  Object *CH_SPAMMARKASREAD;
  Object *CH_MOVEHAMTOINCOMING;
  Object *CH_FILTERHAM;
  Object *CH_SPAM_TRUSTEXTERNALFILTER;
  Object *CY_SPAM_EXTERNALFILTER;
};
*/

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  Object *CH_SPAMFILTERENABLED;
  Object *TX_SPAMBADCOUNT;
  Object *TX_SPAMGOODCOUNT;
  Object *BT_SPAMRESETTRAININGDATA;
  Object *BT_OPTIMIZETRAININGDATA;
  Object *CH_SPAMFILTERFORNEWMAIL;
  Object *CH_SPAMABOOKISWHITELIST;
  Object *CH_SPAMMARKONMOVE;
  Object *CH_SPAMMARKASREAD;
  Object *CH_MOVEHAMTOINCOMING;
  Object *CH_FILTERHAM;
  Object *CH_SPAM_TRUSTEXTERNALFILTER;
  Object *CY_SPAM_EXTERNALFILTER;

  ENTER();

  if((obj = DoSuperNew(cl, obj,
    MUIA_HelpNode, "Configuration#Spam",
    MUIA_ConfigPage_Page, cp_Spam,
    MUIA_ConfigPage_Contents, VGroup,
      Child, VGroup, GroupFrameT(tr(MSG_CO_SPAMFILTER)),
        Child, ColGroup(4),
          Child, CH_SPAMFILTERENABLED = MakeCheck(tr(MSG_CO_SPAM_FILTERENABLED)),
          Child, LLabel(tr(MSG_CO_SPAM_FILTERENABLED)),
          Child, HSpace(0),
          Child, HSpace(0),

          Child, HSpace(0),
          Child, Label2(tr(MSG_CO_SPAM_BADCOUNT)),
          Child, TX_SPAMBADCOUNT = TextObject,
            TextFrame,
            MUIA_Background,    MUII_TextBack,
            MUIA_Text_SetMin,   TRUE,
            MUIA_Text_PreParse, "\033r",
          End,
          Child, HSpace(0),

          Child, HSpace(0),
          Child, Label2(tr(MSG_CO_SPAM_GOODCOUNT)),
          Child, TX_SPAMGOODCOUNT = TextObject,
            TextFrame,
            MUIA_Background,    MUII_TextBack,
            MUIA_Text_SetMin,   TRUE,
            MUIA_Text_PreParse, "\033r",
          End,
          Child, HSpace(0),

          Child, HSpace(0),
          Child, BT_SPAMRESETTRAININGDATA = MakeButton(tr(MSG_CO_SPAM_RESETTRAININGDATA)),
          Child, BT_OPTIMIZETRAININGDATA = MakeButton(tr(MSG_CO_SPAM_OPTIMIZE_TRAININGDATA)),
          Child, HSpace(0),
        End,
      End,

      Child, VGroup, GroupFrameT(tr(MSG_CO_SPAM_RECOGNITION)),
        Child, MakeCheckGroup(&CH_SPAMFILTERFORNEWMAIL, tr(MSG_CO_SPAM_FILTERFORNEWMAIL)),
        Child, MakeCheckGroup(&CH_SPAMABOOKISWHITELIST, tr(MSG_CO_SPAM_ADDRESSBOOKISWHITELIST)),
        Child, MakeCheckGroup(&CH_SPAMMARKONMOVE, tr(MSG_CO_SPAM_MARKONMOVE)),
        Child, MakeCheckGroup(&CH_SPAMMARKASREAD, tr(MSG_CO_SPAM_MARK_AS_READ)),
        Child, MakeCheckGroup(&CH_MOVEHAMTOINCOMING, tr(MSG_CO_MOVE_HAM_TO_INCOMING)),
        Child, ColGroup(2),
          Child, HSpace(5),
          Child, MakeCheckGroup(&CH_FILTERHAM, tr(MSG_CO_FILTER_HAM)),
        End,
        Child, HGroup,
          Child, MakeCheckGroup(&CH_SPAM_TRUSTEXTERNALFILTER, tr(MSG_SPAM_TRUSTHEADERLINES)),
          Child, CY_SPAM_EXTERNALFILTER = FilterChooserObject,
          End,
          Child, HSpace(0),
        End,
      End,
    End,
  TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;

    data->CH_SPAMFILTERENABLED =        CH_SPAMFILTERENABLED;
    data->TX_SPAMBADCOUNT =             TX_SPAMBADCOUNT;
    data->TX_SPAMGOODCOUNT =            TX_SPAMGOODCOUNT;
    data->BT_SPAMRESETTRAININGDATA =    BT_SPAMRESETTRAININGDATA;
    data->BT_OPTIMIZETRAININGDATA =     BT_OPTIMIZETRAININGDATA;
    data->CH_SPAMFILTERFORNEWMAIL =     CH_SPAMFILTERFORNEWMAIL;
    data->CH_SPAMABOOKISWHITELIST =     CH_SPAMABOOKISWHITELIST;
    data->CH_SPAMMARKONMOVE =           CH_SPAMMARKONMOVE;
    data->CH_SPAMMARKASREAD =           CH_SPAMMARKASREAD;
    data->CH_MOVEHAMTOINCOMING =        CH_MOVEHAMTOINCOMING;
    data->CH_FILTERHAM =                CH_FILTERHAM;
    data->CH_SPAM_TRUSTEXTERNALFILTER = CH_SPAM_TRUSTEXTERNALFILTER;
    data->CY_SPAM_EXTERNALFILTER =      CY_SPAM_EXTERNALFILTER;

    SetHelp(CH_SPAMFILTERENABLED,     MSG_HELP_CH_SPAMFILTERENABLED);
    SetHelp(TX_SPAMBADCOUNT,          MSG_HELP_TX_SPAMBADCOUNT);
    SetHelp(TX_SPAMGOODCOUNT,         MSG_HELP_TX_SPAMGOODCOUNT);
    SetHelp(BT_SPAMRESETTRAININGDATA, MSG_HELP_BT_SPAMRESETTRAININGDATA);
    SetHelp(BT_OPTIMIZETRAININGDATA,  MSG_HELP_BT_OPTIMIZE_TRAININGDATA);
    SetHelp(CH_SPAMFILTERFORNEWMAIL,  MSG_HELP_CH_SPAMFILTERFORNEWMAIL);
    SetHelp(CH_SPAMABOOKISWHITELIST,  MSG_HELP_CH_SPAMABOOKISWHITELIST);
    SetHelp(CH_SPAMMARKONMOVE,        MSG_HELP_CH_SPAMMARKONMOVE);
    SetHelp(CH_SPAMMARKASREAD,        MSG_HELP_CH_SPAMMARKASREAD);
    SetHelp(CH_MOVEHAMTOINCOMING,     MSG_HELP_CH_MOVE_HAM_TO_INCOMING);
    SetHelp(CH_FILTERHAM,             MSG_HELP_CH_FILTER_HAM);

    DoMethod(CH_SPAMFILTERENABLED,        MUIM_Notify, MUIA_Selected, MUIV_EveryTime, obj,                    2, METHOD(ToggleSpamFilter), MUIV_NotTriggerValue);
    DoMethod(CH_MOVEHAMTOINCOMING,        MUIM_Notify, MUIA_Selected, MUIV_EveryTime, CH_FILTERHAM,           3, MUIM_Set, MUIA_Disabled, MUIV_NotTriggerValue);
    DoMethod(CH_SPAM_TRUSTEXTERNALFILTER, MUIM_Notify, MUIA_Selected, MUIV_EveryTime, CY_SPAM_EXTERNALFILTER, 3, MUIM_Set, MUIA_Disabled, MUIV_NotTriggerValue);
    DoMethod(BT_SPAMRESETTRAININGDATA,    MUIM_Notify, MUIA_Pressed,  FALSE,          obj,                    1, METHOD(ResetSpamTrainingData));
    DoMethod(BT_OPTIMIZETRAININGDATA,     MUIM_Notify, MUIA_Pressed,  FALSE,          obj,                    1, METHOD(OptimizeSpamTrainingData));
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

  setcheckmark(data->CH_SPAMFILTERENABLED, CE->SpamFilterEnabled);
  setcheckmark(data->CH_SPAMFILTERFORNEWMAIL, CE->SpamFilterForNewMail);
  setcheckmark(data->CH_SPAMMARKONMOVE, CE->SpamMarkOnMove);
  setcheckmark(data->CH_SPAMMARKASREAD, CE->SpamMarkAsRead);
  setcheckmark(data->CH_SPAMABOOKISWHITELIST, CE->SpamAddressBookIsWhiteList);
  setcheckmark(data->CH_MOVEHAMTOINCOMING, CE->MoveHamToIncoming);
  setcheckmark(data->CH_FILTERHAM, CE->FilterHam);
  setcheckmark(data->CH_SPAM_TRUSTEXTERNALFILTER, CE->SpamTrustExternalFilter);
  set(data->CY_SPAM_EXTERNALFILTER, MUIA_FilterChooser_Filter, CE->SpamExternalFilter);

  DoMethod(obj, METHOD(UpdateStats));

  RETURN(0);
  return 0;
}

///
/// OVERLOAD(MUIM_ConfigPage_GUIToConfig)
OVERLOAD(MUIM_ConfigPage_GUIToConfig)
{
  GETDATA;

  ENTER();

  CE->SpamFilterEnabled = GetMUICheck(data->CH_SPAMFILTERENABLED);
  CE->SpamFilterForNewMail = GetMUICheck(data->CH_SPAMFILTERFORNEWMAIL);
  CE->SpamMarkOnMove = GetMUICheck(data->CH_SPAMMARKONMOVE);
  CE->SpamMarkAsRead = GetMUICheck(data->CH_SPAMMARKASREAD);
  CE->SpamAddressBookIsWhiteList = GetMUICheck(data->CH_SPAMABOOKISWHITELIST);
  CE->MoveHamToIncoming = GetMUICheck(data->CH_MOVEHAMTOINCOMING);
  CE->FilterHam = GetMUICheck(data->CH_FILTERHAM);
  CE->SpamTrustExternalFilter = GetMUICheck(data->CH_SPAM_TRUSTEXTERNALFILTER);
  strlcpy(CE->SpamExternalFilter, (char *)xget(data->CY_SPAM_EXTERNALFILTER, MUIA_FilterChooser_Filter), sizeof(CE->SpamExternalFilter));

  RETURN(0);
  return 0;
}

///
/// DECLARE(UpdateStats)
DECLARE(UpdateStats)
{
  GETDATA;
  char buf[SIZE_DEFAULT];

  ENTER();

  snprintf(buf, sizeof(buf), tr(MSG_CO_SPAM_STATISTICS), BayesFilterNumberOfHamClassifiedMails(), BayesFilterNumberOfHamClassifiedWords());
  set(data->TX_SPAMGOODCOUNT, MUIA_Text_Contents, buf);
  snprintf(buf, sizeof(buf), tr(MSG_CO_SPAM_STATISTICS), BayesFilterNumberOfSpamClassifiedMails(), BayesFilterNumberOfSpamClassifiedWords());
  set(data->TX_SPAMBADCOUNT, MUIA_Text_Contents, buf);

  RETURN(0);
  return 0;
}

///
/// DECLARE(ToggleSpamFilter)
// enable/disable all spam filter relevant GUI elements according to the
// current spam filter settings
DECLARE(ToggleSpamFilter) // ULONG inactive
{
  GETDATA;

  ENTER();

  DoMethod(obj, MUIM_MultiSet, MUIA_Disabled, msg->inactive,
    data->BT_SPAMRESETTRAININGDATA,
    data->BT_OPTIMIZETRAININGDATA,
    data->CH_SPAMFILTERFORNEWMAIL,
    data->CH_SPAMABOOKISWHITELIST,
    data->CH_SPAMMARKONMOVE,
    data->CH_SPAMMARKASREAD,
    data->CH_MOVEHAMTOINCOMING,
    data->CH_SPAM_TRUSTEXTERNALFILTER,
    NULL);

  if(msg->inactive == FALSE)
  {
    set(data->CH_FILTERHAM, MUIA_Disabled, xget(data->CH_MOVEHAMTOINCOMING, MUIA_Selected) == FALSE);
    set(data->CY_SPAM_EXTERNALFILTER, MUIA_Disabled, xget(data->CH_SPAM_TRUSTEXTERNALFILTER, MUIA_Selected) == FALSE);
  }
  else
  {
    DoMethod(obj, MUIM_MultiSet, MUIA_Disabled, TRUE,
      data->CH_FILTERHAM,
      data->CY_SPAM_EXTERNALFILTER,
      NULL);
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(ResetSpamTrainingData)
// resets the spam training data
DECLARE(ResetSpamTrainingData)
{
  ENTER();

  if(MUI_Request(_app(obj), _win(obj), MUIF_NONE, NULL, tr(MSG_YesNoReq2), tr(MSG_CO_SPAM_RESETTRAININGDATAASK)) != 0)
  {
    BayesFilterResetTrainingData();
    DoMethod(obj, METHOD(UpdateStats));
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(OptimizeSpamTrainingData)
// optimizes the spam training data
DECLARE(OptimizeSpamTrainingData)
{
  ENTER();

  if(MUI_Request(_app(obj), _win(obj), MUIF_NONE, NULL, tr(MSG_YesNoReq2), tr(MSG_CO_SPAM_OPTIMIZE_TRAININGDATA_ASK)) != 0)
  {
    set(_app(obj), MUIA_Application_Sleep, TRUE);
    BayesFilterOptimizeTrainingData();
    set(_app(obj), MUIA_Application_Sleep, FALSE);

    DoMethod(obj, METHOD(UpdateStats));
  }

  RETURN(0);
  return 0;
}

///
