/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2025 YAM Open Source Team

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
 YAM OpenSource project     : https://github.com/jens-maus/yam/

 Superclass:  MUIC_ConfigPage
 Description: "Filters" configuration page

***************************************************************************/

#include "FiltersConfigPage_cl.h"

#include <proto/asl.h>
#include <proto/muimaster.h>
#include <mui/BetterString_mcc.h>
#include <mui/NBalance_mcc.h>
#include <mui/NList_mcc.h>
#include <mui/NListview_mcc.h>

#include "YAM.h"
#include "YAM_find.h"

#include "mui/AddressBookWindow.h"
#include "mui/AddressField.h"
#include "mui/ConfigPage.h"
#include "mui/ConfigPageList.h"
#include "mui/FilterList.h"
#include "mui/FilterRuleList.h"
#include "mui/FolderRequestPopup.h"
#include "mui/ObjectList.h"
#include "mui/SearchControlGroup.h"

#include "Config.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *LV_RULES;
  Object *BT_RADD;
  Object *BT_RDEL;
  Object *BT_FILTERUP;
  Object *BT_FILTERDOWN;
  Object *BT_FILTER_IMPORT;
  Object *GR_PAGES;
  Object *ST_RNAME;
  Object *CH_REMOTE;
  Object *CH_APPLYNEW;
  Object *CH_APPLYSENT;
  Object *CH_APPLYREQ;
  Object *CY_FILTER_COMBINE;
  Object *GR_SGROUP;
  Object *CH_AREDIRECT;
  Object *ST_AREDIRECT;
  Object *CH_AFORWARD;
  Object *ST_AFORWARD;
  Object *CH_ARESPONSE;
  Object *PO_ARESPONSE;
  Object *ST_ARESPONSE;
  Object *CH_AEXECUTE;
  Object *PO_AEXECUTE;
  Object *ST_AEXECUTE;
  Object *CH_APLAY;
  Object *PO_APLAY;
  Object *ST_APLAY;
  Object *BT_APLAY;
  Object *CH_AMOVE;
  Object *PO_MOVETO;
  Object *CH_ASTATUSTOMARKED;
  Object *CH_ASTATUSTOUNMARKED;
  Object *CH_ASTATUSTOREAD;
  Object *CH_ASTATUSTOUNREAD;
  Object *CH_ASTATUSTOSPAM;
  Object *CH_ASTATUSTOHAM;
  Object *CH_ADELETE;
  Object *CH_ASKIP;
  Object *CH_ATERMINATE;

  struct FilterNode *filter;

  BOOL setup;
};
*/

/* Private functions */
/// GhostOutFilter
//  Enables/disables GUI gadgets in filter form
static void GhostOutFilter(struct IClass *cl, Object *obj)
{
  GETDATA;
  struct FilterNode *filter = data->filter;
  BOOL isremote;
  LONG pos = MUIV_NList_GetPos_Start;

  ENTER();

  isremote = (filter != NULL) ? filter->remote : FALSE;

  set(data->ST_RNAME,             MUIA_Disabled, filter == NULL);
  set(data->CH_REMOTE,            MUIA_Disabled, filter == NULL);
  set(data->CH_APPLYNEW,          MUIA_Disabled, filter == NULL || isremote);
  set(data->CH_APPLYREQ,          MUIA_Disabled, filter == NULL || isremote);
  set(data->CH_APPLYSENT,         MUIA_Disabled, filter == NULL || isremote);
  set(data->CH_AREDIRECT,         MUIA_Disabled, filter == NULL || isremote);
  set(data->CH_AFORWARD,          MUIA_Disabled, filter == NULL || isremote);
  set(data->CH_ARESPONSE,         MUIA_Disabled, filter == NULL || isremote);
  set(data->CY_FILTER_COMBINE,    MUIA_Disabled, filter == NULL);
  set(data->GR_SGROUP,            MUIA_Disabled, filter == NULL);
  set(data->CH_AEXECUTE,          MUIA_Disabled, filter == NULL);
  set(data->CH_APLAY,             MUIA_Disabled, filter == NULL);
  set(data->CH_AMOVE,             MUIA_Disabled, filter == NULL || isremote);
  set(data->CH_ASTATUSTOMARKED,   MUIA_Disabled, filter == NULL || isremote);
  set(data->CH_ASTATUSTOUNMARKED, MUIA_Disabled, filter == NULL || isremote);
  set(data->CH_ASTATUSTOREAD,     MUIA_Disabled, filter == NULL || isremote);
  set(data->CH_ASTATUSTOUNREAD,   MUIA_Disabled, filter == NULL || isremote);
  set(data->CH_ASTATUSTOSPAM,     MUIA_Disabled, filter == NULL || isremote || CE->SpamFilterEnabled == FALSE);
  set(data->CH_ASTATUSTOHAM,      MUIA_Disabled, filter == NULL || isremote || CE->SpamFilterEnabled == FALSE);
  set(data->CH_ADELETE,           MUIA_Disabled, filter == NULL);
  set(data->CH_ASKIP,             MUIA_Disabled, filter == NULL || !isremote);
  set(data->CH_ATERMINATE,        MUIA_Disabled, filter == NULL);
  set(data->ST_AREDIRECT,         MUIA_Disabled, filter == NULL || isremote || !xget(data->CH_AREDIRECT, MUIA_Selected));
  set(data->ST_AFORWARD,          MUIA_Disabled, filter == NULL || isremote || !xget(data->CH_AFORWARD,  MUIA_Selected));
  set(data->BT_APLAY,             MUIA_Disabled, filter == NULL || !xget(data->CH_APLAY, MUIA_Selected));
  set(data->PO_MOVETO,            MUIA_Disabled, filter == NULL || !xget(data->CH_AMOVE, MUIA_Selected));
  set(data->BT_RDEL,              MUIA_Disabled, filter == NULL);

  // lets make sure we ghost the filter up/down buttons if necessary
  if(filter != NULL)
    DoMethod(data->LV_RULES, MUIM_NList_GetPos, filter, &pos);
  else
    DoMethod(data->GR_SGROUP, MUIM_ObjectList_Clear);

  set(data->BT_FILTERUP,   MUIA_Disabled, filter == NULL || pos == 0);
  set(data->BT_FILTERDOWN, MUIA_Disabled, filter == NULL || pos+1 == (LONG)xget(data->LV_RULES, MUIA_NList_Entries));

  if(filter == NULL)
    DoMethod(data->GR_SGROUP, MUIM_ObjectList_Clear);

  if(data->setup == TRUE)
  {
    // These three "disables" must be done in another context, because the Popasl object will en/disable
    // the pop button itself as long as the requester is open. After that this hook is called but the object
    // has not yet enabled the pop button again, so we might get wrong visible results. Not a very nice
    // solution, I must say :(
    DoMethod(_app(obj), MUIM_Application_PushMethod, data->PO_ARESPONSE, 3, MUIM_Set, MUIA_Disabled, filter == NULL || isremote || !xget(data->CH_ARESPONSE, MUIA_Selected));
    DoMethod(_app(obj), MUIM_Application_PushMethod, data->PO_AEXECUTE, 3, MUIM_Set, MUIA_Disabled, filter == NULL || !xget(data->CH_AEXECUTE, MUIA_Selected));
    DoMethod(_app(obj), MUIM_Application_PushMethod, data->PO_APLAY, 3, MUIM_Set, MUIA_Disabled, filter == NULL || !xget(data->CH_APLAY, MUIA_Selected));
  }
  else
  {
    set(data->PO_ARESPONSE,         MUIA_Disabled, filter == NULL || isremote);
    set(data->PO_AEXECUTE,          MUIA_Disabled, filter == NULL);
    set(data->PO_APLAY,             MUIA_Disabled, filter == NULL);
  }

  LEAVE();
}

///

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  static const char *rtitles[4];
  static const char *conditions[4];
  Object *LV_RULES;
  Object *BT_RADD;
  Object *BT_RDEL;
  Object *BT_FILTERUP;
  Object *BT_FILTERDOWN;
  Object *BT_FILTER_IMPORT;
  Object *GR_PAGES;
  Object *ST_RNAME;
  Object *CH_REMOTE;
  Object *CH_APPLYNEW;
  Object *CH_APPLYSENT;
  Object *CH_APPLYREQ;
  Object *CY_FILTER_COMBINE;
  Object *GR_SGROUP;
  Object *CH_AREDIRECT;
  Object *ST_AREDIRECT;
  Object *CH_AFORWARD;
  Object *ST_AFORWARD;
  Object *CH_ARESPONSE;
  Object *PO_ARESPONSE;
  Object *ST_ARESPONSE;
  Object *CH_AEXECUTE;
  Object *PO_AEXECUTE;
  Object *ST_AEXECUTE;
  Object *CH_APLAY;
  Object *PO_APLAY;
  Object *ST_APLAY;
  Object *BT_APLAY;
  Object *CH_AMOVE;
  Object *PO_MOVETO;
  Object *CH_ASTATUSTOMARKED;
  Object *CH_ASTATUSTOUNMARKED;
  Object *CH_ASTATUSTOREAD;
  Object *CH_ASTATUSTOUNREAD;
  Object *CH_ASTATUSTOSPAM;
  Object *CH_ASTATUSTOHAM;
  Object *CH_ADELETE;
  Object *CH_ASKIP;
  Object *CH_ATERMINATE;

  ENTER();

  rtitles[0] = tr(MSG_CO_FILTER_REGISTER_SETTINGS);
  rtitles[1] = tr(MSG_CO_FILTER_REGISTER_CONDITIONS);
  rtitles[2] = tr(MSG_CO_FILTER_REGISTER_ACTIONS);
  rtitles[3] = NULL;

  conditions[0] = tr(MSG_CO_CONDITION_ALL);
  conditions[1] = tr(MSG_CO_CONDITION_MIN_ONE);
  conditions[2] = tr(MSG_CO_CONDITION_MAX_ONE);
  conditions[3] = NULL;

  if((obj = DoSuperNew(cl, obj,
    MUIA_HelpNode, "Configuration#Filters",
    MUIA_ConfigPage_Page, cp_Filters,
    MUIA_ConfigPage_UseScrollgroup, FALSE,
    MUIA_ConfigPage_AddSpacer, FALSE,
    MUIA_ConfigPage_Contents, HGroup,
      GroupSpacing(0),
      Child, VGroup,
        MUIA_HorizWeight, 40,
        Child, NListviewObject,
          MUIA_CycleChain, TRUE,
          MUIA_NListview_NList, LV_RULES = FilterListObject,
          End,
        End,
        Child, HGroup,
          Child, ColGroup(2),
            MUIA_Group_Spacing, 1,
            MUIA_Group_SameWidth, TRUE,
            MUIA_Weight, 1,
            Child, BT_RADD = MakeButton(MUIX_B "+" MUIX_N),
            Child, BT_RDEL = MakeButton(MUIX_B "-" MUIX_N),
          End,
          Child, HSpace(0),
          Child, ColGroup(2),
            MUIA_Group_Spacing, 1,
            MUIA_Group_SameWidth, TRUE,
            Child, BT_FILTERUP = PopButton(MUII_ArrowUp),
            Child, BT_FILTERDOWN = PopButton(MUII_ArrowDown),
          End,
        End,
        Child, BT_FILTER_IMPORT = MakeButton(tr(MSG_CO_FILTER_IMPORT)),
      End,
      Child, NBalanceObject,
         MUIA_Balance_Quiet, TRUE,
      End,
      Child, GR_PAGES = RegisterGroup(rtitles),
        MUIA_CycleChain, TRUE,

        // general settings
        Child, ScrollgroupObject,
          MUIA_Scrollgroup_FreeHoriz, FALSE,
          MUIA_Scrollgroup_AutoBars,  TRUE,
          MUIA_Scrollgroup_Contents,  VGroupV,

            Child, ColGroup(2),
              Child, Label2(tr(MSG_CO_Name)),
              Child, ST_RNAME = MakeString(SIZE_NAME,tr(MSG_CO_Name)),

              Child, HSpace(1),
              Child, MakeCheckGroup(&CH_REMOTE, tr(MSG_CO_Remote)),

              Child, HSpace(1),
              Child, MakeCheckGroup(&CH_APPLYNEW, tr(MSG_CO_ApplyToNew)),

              Child, HSpace(1),
              Child, MakeCheckGroup(&CH_APPLYSENT, tr(MSG_CO_ApplyToSent)),

              Child, HSpace(1),
              Child, MakeCheckGroup(&CH_APPLYREQ, tr(MSG_CO_ApplyOnReq)),

              Child, HVSpace,
              Child, HVSpace,
            End,
          End,
        End,

        // conditions
        Child, VGroup,
          Child, HGroup,
            Child, Label2(tr(MSG_CO_CONDITION_PREPHRASE)),
            Child, CY_FILTER_COMBINE = MakeCycle(conditions, ""),
            Child, Label1(tr(MSG_CO_CONDITION_POSTPHRASE)),
            Child, HVSpace,
          End,
          Child, GR_SGROUP = FilterRuleListObject,
          End,
        End,

        // actions
        Child, ScrollgroupObject,
          MUIA_Scrollgroup_FreeHoriz, FALSE,
          MUIA_Scrollgroup_AutoBars,  TRUE,
          MUIA_Scrollgroup_Contents,  VGroupV,

            Child, ColGroup(3),
              Child, CH_AREDIRECT = MakeCheck(tr(MSG_CO_ACTIONREDIRECT)),
              Child, LLabel2(tr(MSG_CO_ACTIONREDIRECT)),
              Child, MakeAddressField(&ST_AREDIRECT, "", MSG_HELP_CO_ST_AREDIRECT, ABM_CONFIG, -1, AFF_ALLOW_MULTI),
              Child, CH_AFORWARD = MakeCheck(tr(MSG_CO_ActionForward)),
              Child, LLabel2(tr(MSG_CO_ActionForward)),
              Child, MakeAddressField(&ST_AFORWARD, "", MSG_HELP_CO_ST_AFORWARD, ABM_CONFIG, -1, AFF_ALLOW_MULTI),
              Child, CH_ARESPONSE = MakeCheck(tr(MSG_CO_ActionReply)),
              Child, LLabel2(tr(MSG_CO_ActionReply)),
              Child, PO_ARESPONSE = PopaslObject,
                MUIA_Popasl_Type,      ASL_FileRequest,
                MUIA_Popstring_String, ST_ARESPONSE = MakeString(SIZE_PATHFILE, ""),
                MUIA_Popstring_Button, PopButton(MUII_PopFile),
              End,
              Child, CH_AEXECUTE = MakeCheck(tr(MSG_CO_ActionExecute)),
              Child, LLabel2(tr(MSG_CO_ActionExecute)),
              Child, PO_AEXECUTE = PopaslObject,
                MUIA_Popasl_Type,      ASL_FileRequest,
                MUIA_Popstring_String, ST_AEXECUTE = MakeString(SIZE_PATHFILE, ""),
                MUIA_Popstring_Button, PopButton(MUII_PopFile),
              End,
              Child, CH_APLAY = MakeCheck(tr(MSG_CO_ActionPlay)),
              Child, LLabel2(tr(MSG_CO_ActionPlay)),
              Child, HGroup,
                MUIA_Group_Spacing, 0,
                Child, PO_APLAY = PopaslObject,
                  MUIA_Popasl_Type,      ASL_FileRequest,
                  MUIA_Popstring_String, ST_APLAY = MakeString(SIZE_PATHFILE, ""),
                  MUIA_Popstring_Button, PopButton(MUII_PopFile),
                End,
                Child, BT_APLAY = PopButton(MUII_TapePlay),
              End,
              Child, CH_AMOVE = MakeCheck(tr(MSG_CO_ActionMove)),
              Child, LLabel2(tr(MSG_CO_ActionMove)),
              Child, PO_MOVETO = FolderRequestPopupObject,
              End,
            End,
            Child, MakeCheckGroup(&CH_ASTATUSTOMARKED, tr(MSG_CO_ACTION_SET_STATUS_TO_MARKED)),
            Child, MakeCheckGroup(&CH_ASTATUSTOUNMARKED, tr(MSG_CO_ACTION_SET_STATUS_TO_UNMARKED)),
            Child, MakeCheckGroup(&CH_ASTATUSTOREAD, tr(MSG_CO_ACTION_SET_STATUS_TO_READ)),
            Child, MakeCheckGroup(&CH_ASTATUSTOUNREAD, tr(MSG_CO_ACTION_SET_STATUS_TO_UNREAD)),
            Child, MakeCheckGroup(&CH_ASTATUSTOSPAM, tr(MSG_CO_ACTION_SET_STATUS_TO_SPAM)),
            Child, MakeCheckGroup(&CH_ASTATUSTOHAM, tr(MSG_CO_ACTION_SET_STATUS_TO_HAM)),
            Child, MakeCheckGroup(&CH_ADELETE, tr(MSG_CO_ActionDelete)),
            Child, MakeCheckGroup(&CH_ASKIP, tr(MSG_CO_ActionSkip)),
            Child, MakeCheckGroup(&CH_ATERMINATE, tr(MSG_CO_ACTION_TERMINATE_FILTER)),
            Child, HVSpace,
          End,
        End,
      End,
    End,
  TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;

    data->LV_RULES =             LV_RULES;
    data->BT_RADD =              BT_RADD;
    data->BT_RDEL =              BT_RDEL;
    data->BT_FILTERUP =          BT_FILTERUP;
    data->BT_FILTERDOWN =        BT_FILTERDOWN;
    data->BT_FILTER_IMPORT =     BT_FILTER_IMPORT;
    data->GR_PAGES =             GR_PAGES;
    data->ST_RNAME =             ST_RNAME;
    data->CH_REMOTE =            CH_REMOTE;
    data->CH_APPLYNEW =          CH_APPLYNEW;
    data->CH_APPLYSENT =         CH_APPLYSENT;
    data->CH_APPLYREQ =          CH_APPLYREQ;
    data->CY_FILTER_COMBINE =    CY_FILTER_COMBINE;
    data->GR_SGROUP =            GR_SGROUP;
    data->CH_AREDIRECT =         CH_AREDIRECT;
    data->ST_AREDIRECT =         ST_AREDIRECT;
    data->CH_AFORWARD =          CH_AFORWARD;
    data->ST_AFORWARD =          ST_AFORWARD;
    data->CH_ARESPONSE =         CH_ARESPONSE;
    data->PO_ARESPONSE =         PO_ARESPONSE;
    data->ST_ARESPONSE =         ST_ARESPONSE;
    data->CH_AEXECUTE =          CH_AEXECUTE;
    data->PO_AEXECUTE =          PO_AEXECUTE;
    data->ST_AEXECUTE =          ST_AEXECUTE;
    data->CH_APLAY =             CH_APLAY;
    data->PO_APLAY =             PO_APLAY;
    data->ST_APLAY =             ST_APLAY;
    data->BT_APLAY =             BT_APLAY;
    data->CH_AMOVE =             CH_AMOVE;
    data->PO_MOVETO =            PO_MOVETO;
    data->CH_ASTATUSTOMARKED =   CH_ASTATUSTOMARKED;
    data->CH_ASTATUSTOUNMARKED = CH_ASTATUSTOUNMARKED;
    data->CH_ASTATUSTOREAD =     CH_ASTATUSTOREAD;
    data->CH_ASTATUSTOUNREAD =   CH_ASTATUSTOUNREAD;
    data->CH_ASTATUSTOSPAM =     CH_ASTATUSTOSPAM;
    data->CH_ASTATUSTOHAM =      CH_ASTATUSTOHAM;
    data->CH_ADELETE =           CH_ADELETE;
    data->CH_ASKIP =             CH_ASKIP;
    data->CH_ATERMINATE =        CH_ATERMINATE;

    SetHelp(LV_RULES,             MSG_HELP_CO_LV_RULES);
    SetHelp(ST_RNAME,             MSG_HELP_CO_ST_RNAME);
    SetHelp(CH_REMOTE,            MSG_HELP_CO_CH_REMOTE);
    SetHelp(CH_APPLYNEW,          MSG_HELP_CO_CH_APPLYNEW);
    SetHelp(CH_APPLYSENT,         MSG_HELP_CO_CH_APPLYSENT);
    SetHelp(CH_APPLYREQ,          MSG_HELP_CO_CH_APPLYREQ);
    SetHelp(CH_AREDIRECT,         MSG_HELP_CO_CH_AREDIRECT);
    SetHelp(CH_AFORWARD,          MSG_HELP_CO_CH_AFORWARD);
    SetHelp(CH_ARESPONSE,         MSG_HELP_CO_CH_ARESPONSE);
    SetHelp(ST_ARESPONSE,         MSG_HELP_CO_ST_ARESPONSE);
    SetHelp(CH_AEXECUTE,          MSG_HELP_CO_CH_AEXECUTE);
    SetHelp(ST_AEXECUTE,          MSG_HELP_CO_ST_AEXECUTE);
    SetHelp(CH_APLAY,             MSG_HELP_CO_CH_APLAY);
    SetHelp(ST_APLAY,             MSG_HELP_CO_ST_APLAY);
    SetHelp(PO_APLAY,             MSG_HELP_CO_PO_APLAY);
    SetHelp(BT_APLAY,             MSG_HELP_CO_BT_APLAY);
    SetHelp(CH_AMOVE,             MSG_HELP_CO_CH_AMOVE);
    SetHelp(PO_MOVETO,            MSG_HELP_CO_PO_MOVETO);
    SetHelp(CH_ASTATUSTOMARKED,   MSG_HELP_CO_CH_ACTION_SET_STATUS_TO_MARKED);
    SetHelp(CH_ASTATUSTOUNMARKED, MSG_HELP_CO_CH_ACTION_SET_STATUS_TO_UNMARKED);
    SetHelp(CH_ASTATUSTOREAD,     MSG_HELP_CO_CH_ACTION_SET_STATUS_TO_READ);
    SetHelp(CH_ASTATUSTOUNREAD,   MSG_HELP_CO_CH_ACTION_SET_STATUS_TO_UNREAD);
    SetHelp(CH_ASTATUSTOSPAM,     MSG_HELP_CO_CH_ACTION_SET_STATUS_TO_SPAM);
    SetHelp(CH_ASTATUSTOHAM,      MSG_HELP_CO_CH_ACTION_SET_STATUS_TO_HAM);
    SetHelp(CH_ADELETE,           MSG_HELP_CO_CH_ADELETE);
    SetHelp(CH_ASKIP,             MSG_HELP_CO_CH_ASKIP);
    SetHelp(CH_ATERMINATE,        MSG_HELP_CO_CH_ATERMINATE);
    SetHelp(BT_RADD,              MSG_HELP_CO_BT_RADD);
    SetHelp(BT_RDEL,              MSG_HELP_CO_BT_RDEL);
    SetHelp(BT_FILTERUP,          MSG_HELP_CO_BT_FILTERUP);
    SetHelp(BT_FILTERDOWN,        MSG_HELP_CO_BT_FILTERDOWN);

    // set the cyclechain
    set(BT_APLAY, MUIA_CycleChain, TRUE);
    set(BT_FILTERUP, MUIA_CycleChain, TRUE);
    set(BT_FILTERDOWN, MUIA_CycleChain, TRUE);
    set(BT_FILTER_IMPORT, MUIA_CycleChain, TRUE);

    DoMethod(LV_RULES,             MUIM_Notify, MUIA_NList_Active,                         MUIV_EveryTime, obj, 2, METHOD(ChangeActiveFilter), MUIV_TriggerValue);
    DoMethod(ST_RNAME,             MUIM_Notify, MUIA_String_Contents,                      MUIV_EveryTime, obj, 1, METHOD(GUIToFilter));
    DoMethod(CH_REMOTE,            MUIM_Notify, MUIA_Selected,                             MUIV_EveryTime, obj, 2, METHOD(ToggleRemoteFlag), MUIV_TriggerValue);
    DoMethod(CH_APPLYREQ,          MUIM_Notify, MUIA_Selected,                             MUIV_EveryTime, obj, 1, METHOD(GUIToFilter));
    DoMethod(CH_APPLYSENT,         MUIM_Notify, MUIA_Selected,                             MUIV_EveryTime, obj, 1, METHOD(GUIToFilter));
    DoMethod(CH_APPLYNEW,          MUIM_Notify, MUIA_Selected,                             MUIV_EveryTime, obj, 1, METHOD(GUIToFilter));
    DoMethod(CY_FILTER_COMBINE,    MUIM_Notify, MUIA_Cycle_Active,                         MUIV_EveryTime, obj, 1, METHOD(GUIToFilter));
    DoMethod(CH_AREDIRECT,         MUIM_Notify, MUIA_Selected,                             MUIV_EveryTime, obj, 1, METHOD(GUIToFilter));
    DoMethod(CH_AFORWARD,          MUIM_Notify, MUIA_Selected,                             MUIV_EveryTime, obj, 1, METHOD(GUIToFilter));
    DoMethod(CH_ARESPONSE,         MUIM_Notify, MUIA_Selected,                             MUIV_EveryTime, obj, 1, METHOD(GUIToFilter));
    DoMethod(CH_AEXECUTE,          MUIM_Notify, MUIA_Selected,                             MUIV_EveryTime, obj, 1, METHOD(GUIToFilter));
    DoMethod(CH_APLAY,             MUIM_Notify, MUIA_Selected,                             MUIV_EveryTime, obj, 1, METHOD(GUIToFilter));
    DoMethod(CH_AMOVE,             MUIM_Notify, MUIA_Selected,                             MUIV_EveryTime, obj, 1, METHOD(GUIToFilter));
    DoMethod(CH_ASTATUSTOMARKED,   MUIM_Notify, MUIA_Selected,                             MUIV_EveryTime, obj, 1, METHOD(GUIToFilter));
    DoMethod(CH_ASTATUSTOUNMARKED, MUIM_Notify, MUIA_Selected,                             MUIV_EveryTime, obj, 1, METHOD(GUIToFilter));
    DoMethod(CH_ASTATUSTOREAD,     MUIM_Notify, MUIA_Selected,                             MUIV_EveryTime, obj, 1, METHOD(GUIToFilter));
    DoMethod(CH_ASTATUSTOUNREAD,   MUIM_Notify, MUIA_Selected,                             MUIV_EveryTime, obj, 1, METHOD(GUIToFilter));
    DoMethod(CH_ASTATUSTOSPAM,     MUIM_Notify, MUIA_Selected,                             MUIV_EveryTime, obj, 1, METHOD(GUIToFilter));
    DoMethod(CH_ASTATUSTOHAM,      MUIM_Notify, MUIA_Selected,                             MUIV_EveryTime, obj, 1, METHOD(GUIToFilter));
    DoMethod(CH_ADELETE,           MUIM_Notify, MUIA_Selected,                             MUIV_EveryTime, obj, 1, METHOD(GUIToFilter));
    DoMethod(CH_ASKIP,             MUIM_Notify, MUIA_Selected,                             MUIV_EveryTime, obj, 1, METHOD(GUIToFilter));
    DoMethod(CH_ATERMINATE,        MUIM_Notify, MUIA_Selected,                             MUIV_EveryTime, obj, 1, METHOD(GUIToFilter));
    DoMethod(ST_AREDIRECT,         MUIM_Notify, MUIA_String_BufferPos,                     MUIV_EveryTime, obj, 1, METHOD(GUIToFilter));
    DoMethod(ST_AFORWARD,          MUIM_Notify, MUIA_String_BufferPos,                     MUIV_EveryTime, obj, 1, METHOD(GUIToFilter));
    DoMethod(ST_ARESPONSE,         MUIM_Notify, MUIA_String_Contents,                      MUIV_EveryTime, obj, 1, METHOD(GUIToFilter));
    DoMethod(ST_AEXECUTE,          MUIM_Notify, MUIA_String_Contents,                      MUIV_EveryTime, obj, 1, METHOD(GUIToFilter));
    DoMethod(ST_APLAY,             MUIM_Notify, MUIA_String_Contents,                      MUIV_EveryTime, obj, 1, METHOD(GUIToFilter));
    DoMethod(BT_APLAY,             MUIM_Notify, MUIA_Pressed,                              FALSE,          obj, 2, METHOD(PlaySound), ST_APLAY);
    DoMethod(PO_MOVETO,            MUIM_Notify, MUIA_FolderRequestPopup_FolderChanged,     MUIV_EveryTime, obj, 1, METHOD(GUIToFilter));
    DoMethod(GR_SGROUP,            MUIM_Notify, MUIA_ObjectList_ItemsChanged,              MUIV_EveryTime, obj, 1, METHOD(GUIToFilter));
    DoMethod(BT_RADD,              MUIM_Notify, MUIA_Pressed,                              FALSE,          obj, 1, METHOD(AddFilterEntry));
    DoMethod(BT_RDEL,              MUIM_Notify, MUIA_Pressed,                              FALSE,          obj, 1, METHOD(DeleteFilterEntry));
    DoMethod(BT_FILTER_IMPORT,     MUIM_Notify, MUIA_Pressed,                              FALSE,          obj, 1, METHOD(ImportFilter));
    DoMethod(BT_FILTERUP,          MUIM_Notify, MUIA_Pressed,                              FALSE,          LV_RULES,             3, MUIM_NList_Move, MUIV_NList_Move_Selected, MUIV_NList_Move_Previous);
    DoMethod(BT_FILTERDOWN,        MUIM_Notify, MUIA_Pressed,                              FALSE,          LV_RULES,             3, MUIM_NList_Move, MUIV_NList_Move_Selected, MUIV_NList_Move_Next);
    DoMethod(CH_AMOVE,             MUIM_Notify, MUIA_Selected,                             TRUE,           CH_ADELETE,           3, MUIM_Set, MUIA_Selected, FALSE);
    DoMethod(CH_ADELETE,           MUIM_Notify, MUIA_Selected,                             TRUE,           CH_AMOVE,             3, MUIM_Set, MUIA_Selected, FALSE);
    DoMethod(CH_ASTATUSTOMARKED,   MUIM_Notify, MUIA_Selected,                             TRUE,           CH_ASTATUSTOUNMARKED, 3, MUIM_Set, MUIA_Selected, FALSE);
    DoMethod(CH_ASTATUSTOUNMARKED, MUIM_Notify, MUIA_Selected,                             TRUE,           CH_ASTATUSTOMARKED,   3, MUIM_Set, MUIA_Selected, FALSE);
    DoMethod(CH_ASTATUSTOREAD,     MUIM_Notify, MUIA_Selected,                             TRUE,           CH_ASTATUSTOUNREAD,   3, MUIM_Set, MUIA_Selected, FALSE);
    DoMethod(CH_ASTATUSTOUNREAD,   MUIM_Notify, MUIA_Selected,                             TRUE,           CH_ASTATUSTOREAD,     3, MUIM_Set, MUIA_Selected, FALSE);
    DoMethod(CH_ASTATUSTOSPAM,     MUIM_Notify, MUIA_Selected,                             TRUE,           CH_ASTATUSTOHAM,      3, MUIM_Set, MUIA_Selected, FALSE);
    DoMethod(CH_ASTATUSTOHAM,      MUIM_Notify, MUIA_Selected,                             TRUE,           CH_ASTATUSTOSPAM,     3, MUIM_Set, MUIA_Selected, FALSE);

    GhostOutFilter(cl, obj);
  }

  RETURN((IPTR)obj);
  return (IPTR)obj;
}

///
/// OVERLOAD(MUIM_Setup)
OVERLOAD(MUIM_Setup)
{
  GETDATA;
  IPTR rc;

  ENTER();

  // the knowledge of a successful MUIM_Setup call is required in GhostOutFilter()
  // to know whether _app() returns a valid value
  if((rc = DoSuperMethodA(cl, obj, msg)) != 0)
    data->setup = TRUE;

  RETURN(rc);
  return rc;
}

///
/// OVERLOAD(MUIM_Cleanup)
OVERLOAD(MUIM_Cleanup)
{
  GETDATA;
  IPTR rc;

  ENTER();

  rc = DoSuperMethodA(cl, obj, msg);
  data->setup = FALSE;

  RETURN(rc);
  return rc;
}

///
/// OVERLOAD(MUIM_ConfigPage_ConfigToGUI)
OVERLOAD(MUIM_ConfigPage_ConfigToGUI)
{
  GETDATA;
  struct FilterNode *filter;

  ENTER();

  // clear the filter list first
  data->filter = NULL;
  DoMethod(data->LV_RULES, MUIM_NList_Clear);

  // iterate through our filter list and add it to our
  // MUI List
  IterateList(&CE->filterList, struct FilterNode *, filter)
  {
    if(filter->isVolatile == FALSE)
      DoMethod(data->LV_RULES, MUIM_NList_InsertSingle, filter, MUIV_NList_Insert_Bottom);
  }

  // make sure the first entry is selected per default
  DoMethod(data->LV_RULES, MUIM_NList_SetActive, MUIV_NList_Active_Top, MUIV_NList_SetActive_Jump_Center);

  RETURN(0);
  return 0;
}

///
/// OVERLOAD(MUIM_ConfigPage_GUIToConfig)
OVERLOAD(MUIM_ConfigPage_GUIToConfig)
{
  GETDATA;

  ENTER();

  // transfer any pending changes from the GUI to the current filter entry
  DoMethod(obj, METHOD(GUIToFilter));
  // bring NList elements and Exec list elements into sync
  SortNListToExecList(data->LV_RULES, &CE->filterList);

  RETURN(0);
  return 0;
}

///
/// DECLARE(ChangeActiveFilter)
DECLARE(ChangeActiveFilter) // LONG active
{
  GETDATA;

  ENTER();

  // transfer any pending changes from the GUI to the previous filter entry
  DoMethod(obj, METHOD(GUIToFilter));
  // get the now active filter entry
  DoMethod(data->LV_RULES, MUIM_NList_GetEntry, msg->active, &data->filter);
  // update the GUI
  DoMethod(obj, METHOD(FilterToGUI));

  RETURN(0);
  return 0;
}

///
/// DECLARE(FilterToGUI)
// fills form with data from selected list entry
DECLARE(FilterToGUI)
{
  GETDATA;

  ENTER();

  if(data->filter != NULL)
  {
    struct FilterNode *filter = data->filter;
    struct RuleNode *rule;

    nnset(data->ST_RNAME,             MUIA_String_Contents,             filter->name);
    nnset(data->CH_REMOTE,            MUIA_Selected,                    filter->remote);
    nnset(data->CH_APPLYNEW,          MUIA_Selected,                    filter->applyToNew);
    nnset(data->CH_APPLYSENT,         MUIA_Selected,                    filter->applyToSent);
    nnset(data->CH_APPLYREQ,          MUIA_Selected,                    filter->applyOnReq);
    nnset(data->CY_FILTER_COMBINE,    MUIA_Cycle_Active,                filter->combine);
    nnset(data->CH_AREDIRECT,         MUIA_Selected,                    hasRedirectAction(filter));
    nnset(data->CH_AFORWARD,          MUIA_Selected,                    hasForwardAction(filter));
    nnset(data->CH_ARESPONSE,         MUIA_Selected,                    hasReplyAction(filter));
    nnset(data->CH_AEXECUTE,          MUIA_Selected,                    hasExecuteAction(filter));
    nnset(data->CH_APLAY,             MUIA_Selected,                    hasPlaySoundAction(filter));
    nnset(data->CH_AMOVE,             MUIA_Selected,                    hasMoveAction(filter));
    nnset(data->CH_ASTATUSTOMARKED,   MUIA_Selected,                    hasStatusToMarkedAction(filter));
    nnset(data->CH_ASTATUSTOUNMARKED, MUIA_Selected,                    hasStatusToUnmarkedAction(filter));
    nnset(data->CH_ASTATUSTOREAD,     MUIA_Selected,                    hasStatusToReadAction(filter));
    nnset(data->CH_ASTATUSTOUNREAD,   MUIA_Selected,                    hasStatusToUnreadAction(filter));
    nnset(data->CH_ASTATUSTOSPAM,     MUIA_Selected,                    hasStatusToSpamAction(filter));
    nnset(data->CH_ASTATUSTOHAM,      MUIA_Selected,                    hasStatusToHamAction(filter));
    nnset(data->CH_ADELETE,           MUIA_Selected,                    hasDeleteAction(filter));
    nnset(data->CH_ASKIP,             MUIA_Selected,                    hasSkipMsgAction(filter));
    nnset(data->CH_ATERMINATE,        MUIA_Selected,                    hasTerminateAction(filter));
    nnset(data->ST_AREDIRECT,         MUIA_String_Contents,             filter->redirectTo);
    nnset(data->ST_AFORWARD,          MUIA_String_Contents,             filter->forwardTo);
    nnset(data->ST_ARESPONSE,         MUIA_String_Contents,             filter->replyFile);
    nnset(data->ST_AEXECUTE,          MUIA_String_Contents,             filter->executeCmd);
    nnset(data->ST_APLAY,             MUIA_String_Contents,             filter->playSound);
    nnset(data->PO_MOVETO,            MUIA_FolderRequestPopup_FolderID, filter->moveToID);

    xset(data->GR_SGROUP, MUIA_ObjectList_Quiet, TRUE,
                          MUIA_FilterRuleList_Filter, filter);

    // before we actually set our rule options we have to clear out
    // all previous existing group childs
    DoMethod(data->GR_SGROUP, MUIM_ObjectList_Clear);

    // Now we should have a clean SGROUP and can populate with new SearchControlGroup objects
    IterateList(&filter->ruleList, struct RuleNode *, rule)
    {
      Object *newSearchGroup;

      if((newSearchGroup = (Object *)DoMethod(data->GR_SGROUP, MUIM_ObjectList_CreateItem)) != NULL)
      {
        // fill the new search group with some content
        DoMethod(newSearchGroup, MUIM_SearchControlGroup_RuleToGUI, rule);

        // add it to our searchGroupList
        DoMethod(data->GR_SGROUP, MUIM_ObjectList_AddItem, newSearchGroup);
        // the notifications above will ensure that the necessary notifications
        // for this new object will be added automatically
      }
    }
    set(data->GR_SGROUP, MUIA_ObjectList_Quiet, FALSE);
  }

  GhostOutFilter(cl, obj);

  RETURN(0);
  return 0;
}

///
/// DECLARE(GUIToFilter)
// fills form data into selected list entry
DECLARE(GUIToFilter)
{
  GETDATA;

  ENTER();

  if(data->filter != NULL)
  {
    struct FilterNode *filter = data->filter;
    Object *ruleItem;
    Object *ruleState;

    GetMUIString(filter->name, data->ST_RNAME, sizeof(filter->name));
    filter->remote      = GetMUICheck(data->CH_REMOTE);
    filter->applyToNew  = GetMUICheck(data->CH_APPLYNEW);
    filter->applyToSent = GetMUICheck(data->CH_APPLYSENT);
    filter->applyOnReq  = GetMUICheck(data->CH_APPLYREQ);
    filter->combine = GetMUICycle(data->CY_FILTER_COMBINE);
    filter->actions = 0;
    if(GetMUICheck(data->CH_AREDIRECT))         setFlag(filter->actions, FA_REDIRECT);
    if(GetMUICheck(data->CH_AFORWARD))          setFlag(filter->actions, FA_FORWARD);
    if(GetMUICheck(data->CH_ARESPONSE))         setFlag(filter->actions, FA_REPLY);
    if(GetMUICheck(data->CH_AEXECUTE))          setFlag(filter->actions, FA_EXECUTE);
    if(GetMUICheck(data->CH_APLAY))             setFlag(filter->actions, FA_PLAYSOUND);
    if(GetMUICheck(data->CH_AMOVE))             setFlag(filter->actions, FA_MOVE);
    if(GetMUICheck(data->CH_ASTATUSTOMARKED))   setFlag(filter->actions, FA_STATUSTOMARKED);
    if(GetMUICheck(data->CH_ASTATUSTOUNMARKED)) setFlag(filter->actions, FA_STATUSTOUNMARKED);
    if(GetMUICheck(data->CH_ASTATUSTOREAD))     setFlag(filter->actions, FA_STATUSTOREAD);
    if(GetMUICheck(data->CH_ASTATUSTOUNREAD))   setFlag(filter->actions, FA_STATUSTOUNREAD);
    if(GetMUICheck(data->CH_ASTATUSTOSPAM))     setFlag(filter->actions, FA_STATUSTOSPAM);
    if(GetMUICheck(data->CH_ASTATUSTOHAM))      setFlag(filter->actions, FA_STATUSTOHAM);
    if(GetMUICheck(data->CH_ADELETE))           setFlag(filter->actions, FA_DELETE);
    if(GetMUICheck(data->CH_ASKIP))             setFlag(filter->actions, FA_SKIPMSG);
    if(GetMUICheck(data->CH_ATERMINATE))        setFlag(filter->actions, FA_TERMINATE);
    GetMUIString(filter->redirectTo, data->ST_AREDIRECT, sizeof(filter->redirectTo));
    GetMUIString(filter->forwardTo,  data->ST_AFORWARD, sizeof(filter->forwardTo));
    GetMUIString(filter->replyFile,  data->ST_ARESPONSE, sizeof(filter->replyFile));
    GetMUIString(filter->executeCmd, data->ST_AEXECUTE, sizeof(filter->executeCmd));
    GetMUIString(filter->playSound,  data->ST_APLAY, sizeof(filter->playSound));
    filter->moveToID = xget(data->PO_MOVETO, MUIA_FolderRequestPopup_FolderID);
    strlcpy(filter->moveToName, (char *)xget(data->PO_MOVETO, MUIA_FolderRequestPopup_FolderName), sizeof(filter->moveToName));

    // (re)build the rule list from scratch
    FreeFilterRuleList(filter);
    ruleState = NULL;
    while((ruleItem = (Object *)DoMethod(data->GR_SGROUP, MUIM_ObjectList_IterateItems, &ruleState)) != NULL)
    {
      struct RuleNode *rule;

      // set the rule settings
      if((rule = CreateNewRule(filter, 0)) != NULL)
        DoMethod(ruleItem, MUIM_SearchControlGroup_GUIToRule, rule);
    }

    GhostOutFilter(cl, obj);
    DoMethod(data->LV_RULES, MUIM_NList_Redraw, MUIV_NList_Redraw_Active);
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(AddFilterEntry)
// adds a new entry to the filter list
DECLARE(AddFilterEntry)
{
  GETDATA;
  struct FilterNode *filterNode;

  ENTER();

  if((filterNode = CreateNewFilter(FA_TERMINATE, 0)) != NULL)
  {
    DoMethod(data->LV_RULES, MUIM_NList_InsertSingle, filterNode, MUIV_NList_Insert_Bottom);
    set(data->LV_RULES, MUIA_NList_Active, MUIV_NList_Active_Bottom);

    // make the first page the active one
    set(data->GR_PAGES, MUIA_Group_ActivePage, 0);
    // lets set the new string gadget active and select all text in there automatically to
    // be more handy to the user ;)
    set(_win(data->LV_RULES), MUIA_Window_ActiveObject, data->ST_RNAME);
    set(data->ST_RNAME, MUIA_BetterString_SelectSize, -((LONG)strlen(filterNode->name)));

    // now add the filterNode to our global filterList
    AddTail((struct List *)&CE->filterList, (struct Node *)filterNode);
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(DeleteFilterEntry)
// deletes an entry from the filter list
DECLARE(DeleteFilterEntry)
{
  GETDATA;
  struct FilterNode *filterNode = NULL;

  ENTER();

  // get the active filterNode
  DoMethod(data->LV_RULES, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &filterNode);

  // if we got an active entry lets remove it from the GUI List
  // and also from our own global filterList
  if(filterNode != NULL)
  {
    DoMethod(data->LV_RULES, MUIM_NList_Remove, MUIV_NList_Remove_Active);

    Remove((struct Node *)filterNode);
    DeleteFilterNode(filterNode);
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(ToggleRemoteFlag)
// enables/disables GUI elements for remote filters
DECLARE(ToggleRemoteFlag) // ULONG remote
{
  GETDATA;
  Object *ruleItem;
  Object *ruleState;

  ENTER();

  // update the filter's remote state
  data->filter->remote = msg->remote;

  // propagate the new state to all rules
  ruleState = NULL;
  while((ruleItem = (Object *)DoMethod(data->GR_SGROUP, MUIM_ObjectList_IterateItems, &ruleState)) != NULL)
  {
    set(ruleItem, MUIA_SearchControlGroup_RemoteFilterMode, msg->remote);
  }

  GhostOutFilter(cl, obj);

  // show the new state in the filter list
  DoMethod(data->LV_RULES, MUIM_NList_Redraw, MUIV_NList_Redraw_Active);

  RETURN(0);
  return 0;
}

///
/// DECLARE(ImportFilter)
// import filter settings from a .sfd file
DECLARE(ImportFilter)
{
  GETDATA;
  struct FileReqCache *frc;

  ENTER();

  if((frc = ReqFile(ASL_FILTER, _win(obj), tr(MSG_FILTER_IMPORT_TITLE), 0, "PROGDIR:", "")) != NULL)
  {
    char path[SIZE_PATHFILE];

    AddPath(path, frc->drawer, frc->file, sizeof(path));
    if(ImportFilter(path, FALSE, &CE->filterList) == TRUE)
    {
      // update the GUI and set the imported filter as the active one
      DoMethod(obj, MUIM_ConfigPage_ConfigToGUI, CE);
      set(data->LV_RULES, MUIA_NList_Active, MUIV_NList_Active_Bottom);
    }
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(PlaySound)
// plays sound file referred by the string gadget
DECLARE(PlaySound) // Object *strObj
{
  char *file;

  ENTER();

  file = (char *)xget(msg->strObj, MUIA_String_Contents);
  if(IsStrEmpty(file) == FALSE)
    PlaySound(file);

  RETURN(0);
  return 0;
}

///
