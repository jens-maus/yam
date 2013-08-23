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
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 YAM Official Support Site :  http://www.yam.ch
 YAM OpenSource project    :  http://sourceforge.net/projects/yamos/

 $Id$

***************************************************************************/

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include <clib/macros.h>

#include <libraries/asl.h>
#include <mui/BetterString_mcc.h>
#include <mui/NBalance_mcc.h>
#include <mui/NListview_mcc.h>
#include <mui/TextEditor_mcc.h>
#if defined(__amigaos4__)
#include <proto/application.h>
#endif
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/muimaster.h>
#include <proto/wb.h>
#include <proto/xpkmaster.h>

#if !defined(__amigaos4__)
#include <clib/alib_protos.h> // DoMethod
#endif

#include "extrasrc.h"

#include "SDI_hook.h"

#include "YAM.h"
#include "YAM_addressbook.h"
#include "YAM_config.h"
#include "YAM_configFile.h"
#include "YAM_error.h"
#include "YAM_find.h"
#include "YAM_global.h"
#include "YAM_main.h"
#include "YAM_mainFolder.h"
#include "YAM_utilities.h"

#include "mui/AccountList.h"
#include "mui/ClassesExtra.h"
#include "mui/FilterChooser.h"
#include "mui/FilterList.h"
#include "mui/FilterRuleList.h"
#include "mui/FolderRequestListtree.h"
#include "mui/IdentityList.h"
#include "mui/ImageArea.h"
#include "mui/MailServerChooser.h"
#include "mui/MimeTypeList.h"
#include "mui/PlaceholderPopupList.h"
#include "mui/ScriptList.h"
#include "mui/SearchControlGroup.h"
#include "mui/SignatureChooser.h"
#include "mui/SignatureList.h"
#include "mui/SignatureTextEdit.h"
#include "mui/ThemeListGroup.h"
#include "mui/TZoneChooser.h"
#include "mui/TZoneInfoBar.h"
#include "mui/YAMApplication.h"

#include "BayesFilter.h"
#include "FileInfo.h"
#include "FolderList.h"
#include "ImageCache.h"
#include "Locale.h"
#include "MimeTypes.h"
#include "MailServers.h"
#include "MUIObjects.h"
#include "Requesters.h"
#include "Signature.h"
#include "Threads.h"
#include "TZone.h"
#include "UIDL.h"
#include "UserIdentity.h"

#include "Debug.h"

/* local defines */
/// ConfigPageHeaderObject()
#define ConfigPageHeaderObject(id, image, title, summary) \
  Child, HGroup,                                          \
    Child, MakeImageObject(id, image),                    \
    Child, VGroup,                                        \
      Child, TextObject,                                  \
        MUIA_Text_PreParse, "\033b",                      \
        MUIA_Text_Contents, (title),                      \
        MUIA_Text_Copy,     FALSE,                        \
        MUIA_Weight,        100,                          \
      End,                                                \
      Child, TextObject,                                  \
        MUIA_Text_Contents, (summary),                    \
        MUIA_Text_Copy,     FALSE,                        \
        MUIA_Font,          MUIV_Font_Tiny,               \
        MUIA_Weight,        100,                          \
      End,                                                \
    End,                                                  \
  End,                                                    \
  Child, RectangleObject,                                 \
    MUIA_Rectangle_HBar, TRUE,                            \
    MUIA_FixHeight,      4,                               \
  End

///

/***************************************************************************
 Module: Configuration - GUI for sections
***************************************************************************/

/*** Hooks ***/
/// PO_Text2ListFunc
//  selects the folder as active which is currently in the 'str'
//  object
HOOKPROTONH(PO_Text2List, BOOL, Object *listview, Object *str)
{
  char *s;

  ENTER();

  // get the currently set string
  s = (char *)xget(str, MUIA_Text_Contents);

  if(s != NULL && listview != NULL)
  {
    Object *list = (Object *)xget(listview, MUIA_NListview_NList);

    // now try to find the node and activate it right away
    DoMethod(list, MUIM_NListtree_FindName, MUIV_NListtree_FindName_ListNode_Root, s, MUIV_NListtree_FindName_Flag_Activate);
  }

  RETURN(TRUE);
  return TRUE;
}
MakeHook(PO_Text2ListHook, PO_Text2List);

///
/// PO_List2TextFunc
//  Copies listview selection to text gadget
HOOKPROTONH(PO_List2TextFunc, void, Object *listview, Object *text)
{
  Object *list;

  ENTER();

  if((list = (Object *)xget(listview, MUIA_NListview_NList)) != NULL && text != NULL)
  {
    struct MUI_NListtree_TreeNode *tn = (struct MUI_NListtree_TreeNode *)xget(list, MUIA_NListtree_Active);

    if(tn != NULL && tn->tn_User != NULL)
    {
      struct FolderNode *fnode = (struct FolderNode *)tn->tn_User;
      set(text, MUIA_Text_Contents, fnode->folder->Name);
    }
  }

  LEAVE();
}
MakeHook(PO_List2TextHook, PO_List2TextFunc);

///
/// CO_PlaySoundFunc
//  Plays sound file referred by the string gadget
HOOKPROTONHNO(CO_PlaySoundFunc, void, int *arg)
{
  char *soundFile;

  ENTER();

  soundFile = (char *)xget((Object *)arg[0], MUIA_String_Contents);
  if(soundFile != NULL && soundFile[0] != '\0')
    PlaySound(soundFile);

  LEAVE();
}
MakeHook(CO_PlaySoundHook,CO_PlaySoundFunc);

///
/// AddNewFilterToList
//  Adds a new entry to the global filter list
HOOKPROTONHNONP(AddNewFilterToList, void)
{
  struct FilterNode *filterNode;

  if((filterNode = CreateNewFilter(FA_TERMINATE, 0)) != NULL)
  {
    #warning access to old config GUI
    DoMethod(G->CO->GUI.LV_RULES, MUIM_NList_InsertSingle, filterNode, MUIV_NList_Insert_Bottom);
    set(G->CO->GUI.LV_RULES, MUIA_NList_Active, MUIV_NList_Active_Bottom);

    // lets set the new string gadget active and select all text in there automatically to
    // be more handy to the user ;)
    set(_win(G->CO->GUI.LV_RULES), MUIA_Window_ActiveObject, G->CO->GUI.ST_RNAME);
    set(G->CO->GUI.ST_RNAME, MUIA_BetterString_SelectSize, -((LONG)strlen(filterNode->name)));

    // now add the filterNode to our global filterList
    AddTail((struct List *)&CE->filterList, (struct Node *)filterNode);
  }
}
MakeStaticHook(AddNewFilterToListHook, AddNewFilterToList);

///
/// RemoveActiveFilter
//  Deletes the active filter entry from the filter list
HOOKPROTONHNONP(RemoveActiveFilter, void)
{
  struct FilterNode *filterNode = NULL;

  // get the active filterNode
  #warning access to old config GUI
  DoMethod(G->CO->GUI.LV_RULES, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &filterNode);

  // if we got an active entry lets remove it from the GUI List
  // and also from our own global filterList
  if(filterNode != NULL)
  {
    DoMethod(G->CO->GUI.LV_RULES, MUIM_NList_Remove, MUIV_NList_Remove_Active);

    Remove((struct Node *)filterNode);
    DeleteFilterNode(filterNode);
  }
}
MakeStaticHook(RemoveActiveFilterHook, RemoveActiveFilter);

///

/*** Pages ***/
/// CO_PageFilters
Object *CO_PageFilters(struct CO_ClassData *data)
{
  static const char *rtitles[4];
  static const char *conditions[4];
  Object *grp;
  Object *bt_moveto;

  ENTER();

  rtitles[0] = tr(MSG_CO_FILTER_REGISTER_SETTINGS);
  rtitles[1] = tr(MSG_CO_FILTER_REGISTER_CONDITIONS);
  rtitles[2] = tr(MSG_CO_FILTER_REGISTER_ACTIONS);
  rtitles[3] = NULL;

  conditions[0] = tr(MSG_CO_CONDITION_ALL);
  conditions[1] = tr(MSG_CO_CONDITION_MIN_ONE);
  conditions[2] = tr(MSG_CO_CONDITION_MAX_ONE);
  conditions[3] = NULL;

  if((grp = VGroup,
    MUIA_HelpNode, "Configuration#Filters",

    ConfigPageHeaderObject("config_filters_big", G->theme.configImages[CI_FILTERSBIG], tr(MSG_CO_FILTER_TITLE), tr(MSG_CO_FILTER_SUMMARY)),

    Child, HGroup,
      GroupSpacing(0),
      Child, VGroup,
        MUIA_HorizWeight, 40,
        Child, NListviewObject,
          MUIA_CycleChain, TRUE,
          MUIA_NListview_NList, data->GUI.LV_RULES = FilterListObject,
          End,
        End,
        Child, HGroup,
          Child, ColGroup(2),
            MUIA_Group_Spacing, 1,
            MUIA_Group_SameWidth, TRUE,
            MUIA_Weight, 1,
            Child, data->GUI.BT_RADD = MakeButton(MUIX_B "+" MUIX_N),
            Child, data->GUI.BT_RDEL = MakeButton(MUIX_B "-" MUIX_N),
          End,
          Child, HSpace(0),
          Child, ColGroup(2),
            MUIA_Group_Spacing, 1,
            MUIA_Group_SameWidth, TRUE,
            Child, data->GUI.BT_FILTERUP = PopButton(MUII_ArrowUp),
            Child, data->GUI.BT_FILTERDOWN = PopButton(MUII_ArrowDown),
          End,
        End,
        Child, data->GUI.BT_FILTER_IMPORT = MakeButton(tr(MSG_CO_FILTER_IMPORT)),
      End,
      Child, NBalanceObject,
         MUIA_Balance_Quiet, TRUE,
      End,
      Child, RegisterGroup(rtitles),
        MUIA_CycleChain, TRUE,

        // general settings
        Child, ScrollgroupObject,
          MUIA_Scrollgroup_FreeHoriz, FALSE,
          MUIA_Scrollgroup_AutoBars,  TRUE,
          MUIA_Scrollgroup_Contents,  VGroupV,

            Child, ColGroup(2),
              Child, Label2(tr(MSG_CO_Name)),
              Child, data->GUI.ST_RNAME = MakeString(SIZE_NAME,tr(MSG_CO_Name)),

              Child, HSpace(1),
              Child, MakeCheckGroup(&data->GUI.CH_REMOTE, tr(MSG_CO_Remote)),

              Child, HSpace(1),
              Child, MakeCheckGroup(&data->GUI.CH_APPLYNEW, tr(MSG_CO_ApplyToNew)),

              Child, HSpace(1),
              Child, MakeCheckGroup(&data->GUI.CH_APPLYSENT, tr(MSG_CO_ApplyToSent)),

              Child, HSpace(1),
              Child, MakeCheckGroup(&data->GUI.CH_APPLYREQ, tr(MSG_CO_ApplyOnReq)),

              Child, HVSpace,
              Child, HVSpace,
            End,
          End,
        End,

        // conditions
        Child, VGroup,
          Child, HGroup,
            Child, Label2(tr(MSG_CO_CONDITION_PREPHRASE)),
            Child, data->GUI.CY_FILTER_COMBINE = MakeCycle(conditions, ""),
            Child, Label1(tr(MSG_CO_CONDITION_POSTPHRASE)),
            Child, HVSpace,
          End,
          Child, data->GUI.GR_SGROUP = FilterRuleListObject,
          End,
        End,

        // actions
        Child, ScrollgroupObject,
          MUIA_Scrollgroup_FreeHoriz, FALSE,
          MUIA_Scrollgroup_AutoBars,  TRUE,
          MUIA_Scrollgroup_Contents,  VGroupV,

            Child, ColGroup(3),
              Child, data->GUI.CH_AREDIRECT = MakeCheck(tr(MSG_CO_ACTIONREDIRECT)),
              Child, LLabel2(tr(MSG_CO_ACTIONREDIRECT)),
              Child, MakeAddressField(&data->GUI.ST_AREDIRECT, "", MSG_HELP_CO_ST_AREDIRECT, ABM_CONFIG, -1, AFF_ALLOW_MULTI),
              Child, data->GUI.CH_AFORWARD = MakeCheck(tr(MSG_CO_ActionForward)),
              Child, LLabel2(tr(MSG_CO_ActionForward)),
              Child, MakeAddressField(&data->GUI.ST_AFORWARD, "", MSG_HELP_CO_ST_AFORWARD, ABM_CONFIG, -1, AFF_ALLOW_MULTI),
              Child, data->GUI.CH_ARESPONSE = MakeCheck(tr(MSG_CO_ActionReply)),
              Child, LLabel2(tr(MSG_CO_ActionReply)),
              Child, data->GUI.PO_ARESPONSE = PopaslObject,
                MUIA_Popasl_Type,      ASL_FileRequest,
                MUIA_Popstring_String, data->GUI.ST_ARESPONSE = MakeString(SIZE_PATHFILE, ""),
                MUIA_Popstring_Button, PopButton(MUII_PopFile),
              End,
              Child, data->GUI.CH_AEXECUTE = MakeCheck(tr(MSG_CO_ActionExecute)),
              Child, LLabel2(tr(MSG_CO_ActionExecute)),
              Child, data->GUI.PO_AEXECUTE = PopaslObject,
                MUIA_Popasl_Type,      ASL_FileRequest,
                MUIA_Popstring_String, data->GUI.ST_AEXECUTE = MakeString(SIZE_PATHFILE, ""),
                MUIA_Popstring_Button, PopButton(MUII_PopFile),
              End,
              Child, data->GUI.CH_APLAY = MakeCheck(tr(MSG_CO_ActionPlay)),
              Child, LLabel2(tr(MSG_CO_ActionPlay)),
              Child, HGroup,
                MUIA_Group_HorizSpacing, 0,
                Child, data->GUI.PO_APLAY = PopaslObject,
                  MUIA_Popasl_Type,      ASL_FileRequest,
                  MUIA_Popstring_String, data->GUI.ST_APLAY = MakeString(SIZE_PATHFILE, ""),
                  MUIA_Popstring_Button, PopButton(MUII_PopFile),
                End,
                Child, data->GUI.BT_APLAY = PopButton(MUII_TapePlay),
              End,
              Child, data->GUI.CH_AMOVE = MakeCheck(tr(MSG_CO_ActionMove)),
              Child, LLabel2(tr(MSG_CO_ActionMove)),
              Child, data->GUI.PO_MOVETO = PopobjectObject,
                MUIA_Popstring_String, data->GUI.TX_MOVETO = TextObject,
                  TextFrame,
                  MUIA_Text_Copy, FALSE,
                End,
                MUIA_Popstring_Button,bt_moveto = PopButton(MUII_PopUp),
                MUIA_Popobject_StrObjHook, &PO_Text2ListHook,
                MUIA_Popobject_ObjStrHook, &PO_List2TextHook,
                MUIA_Popobject_WindowHook, &PO_WindowHook,
                MUIA_Popobject_Object, NListviewObject,
                  MUIA_NListview_NList, data->GUI.LV_MOVETO = FolderRequestListtreeObject,
                    MUIA_NList_DoubleClick, TRUE,
                  End,
                End,
              End,
            End,
            Child, MakeCheckGroup(&data->GUI.CH_ASTATUSTOMARKED, tr(MSG_CO_ACTION_SET_STATUS_TO_MARKED)),
            Child, MakeCheckGroup(&data->GUI.CH_ASTATUSTOUNMARKED, tr(MSG_CO_ACTION_SET_STATUS_TO_UNMARKED)),
            Child, MakeCheckGroup(&data->GUI.CH_ASTATUSTOREAD, tr(MSG_CO_ACTION_SET_STATUS_TO_READ)),
            Child, MakeCheckGroup(&data->GUI.CH_ASTATUSTOUNREAD, tr(MSG_CO_ACTION_SET_STATUS_TO_UNREAD)),
            Child, MakeCheckGroup(&data->GUI.CH_ASTATUSTOSPAM, tr(MSG_CO_ACTION_SET_STATUS_TO_SPAM)),
            Child, MakeCheckGroup(&data->GUI.CH_ASTATUSTOHAM, tr(MSG_CO_ACTION_SET_STATUS_TO_HAM)),
            Child, MakeCheckGroup(&data->GUI.CH_ADELETE, tr(MSG_CO_ActionDelete)),
            Child, MakeCheckGroup(&data->GUI.CH_ASKIP, tr(MSG_CO_ActionSkip)),
            Child, MakeCheckGroup(&data->GUI.CH_ATERMINATE, tr(MSG_CO_ACTION_TERMINATE_FILTER)),
            Child, HVSpace,
          End,
        End,
      End,
    End,

  End))
  {
    SetHelp(data->GUI.LV_RULES,             MSG_HELP_CO_LV_RULES);
    SetHelp(data->GUI.ST_RNAME,             MSG_HELP_CO_ST_RNAME);
    SetHelp(data->GUI.CH_REMOTE,            MSG_HELP_CO_CH_REMOTE);
    SetHelp(data->GUI.CH_APPLYNEW,          MSG_HELP_CO_CH_APPLYNEW);
    SetHelp(data->GUI.CH_APPLYSENT,         MSG_HELP_CO_CH_APPLYSENT);
    SetHelp(data->GUI.CH_APPLYREQ,          MSG_HELP_CO_CH_APPLYREQ);
    SetHelp(data->GUI.CH_AREDIRECT,         MSG_HELP_CO_CH_AREDIRECT);
    SetHelp(data->GUI.CH_AFORWARD,          MSG_HELP_CO_CH_AFORWARD);
    SetHelp(data->GUI.CH_ARESPONSE,         MSG_HELP_CO_CH_ARESPONSE);
    SetHelp(data->GUI.ST_ARESPONSE,         MSG_HELP_CO_ST_ARESPONSE);
    SetHelp(data->GUI.CH_AEXECUTE,          MSG_HELP_CO_CH_AEXECUTE);
    SetHelp(data->GUI.ST_AEXECUTE,          MSG_HELP_CO_ST_AEXECUTE);
    SetHelp(data->GUI.CH_APLAY,             MSG_HELP_CO_CH_APLAY);
    SetHelp(data->GUI.ST_APLAY,             MSG_HELP_CO_ST_APLAY);
    SetHelp(data->GUI.PO_APLAY,             MSG_HELP_CO_PO_APLAY);
    SetHelp(data->GUI.BT_APLAY,             MSG_HELP_CO_BT_APLAY);
    SetHelp(data->GUI.CH_AMOVE,             MSG_HELP_CO_CH_AMOVE);
    SetHelp(data->GUI.PO_MOVETO,            MSG_HELP_CO_PO_MOVETO);
    SetHelp(data->GUI.CH_ASTATUSTOMARKED,   MSG_HELP_CO_CH_ACTION_SET_STATUS_TO_MARKED);
    SetHelp(data->GUI.CH_ASTATUSTOUNMARKED, MSG_HELP_CO_CH_ACTION_SET_STATUS_TO_UNMARKED);
    SetHelp(data->GUI.CH_ASTATUSTOREAD,     MSG_HELP_CO_CH_ACTION_SET_STATUS_TO_READ);
    SetHelp(data->GUI.CH_ASTATUSTOUNREAD,   MSG_HELP_CO_CH_ACTION_SET_STATUS_TO_UNREAD);
    SetHelp(data->GUI.CH_ASTATUSTOSPAM,     MSG_HELP_CO_CH_ACTION_SET_STATUS_TO_SPAM);
    SetHelp(data->GUI.CH_ASTATUSTOHAM,      MSG_HELP_CO_CH_ACTION_SET_STATUS_TO_HAM);
    SetHelp(data->GUI.CH_ADELETE,           MSG_HELP_CO_CH_ADELETE);
    SetHelp(data->GUI.CH_ASKIP,             MSG_HELP_CO_CH_ASKIP);
    SetHelp(data->GUI.CH_ATERMINATE,        MSG_HELP_CO_CH_ATERMINATE);
    SetHelp(data->GUI.BT_RADD,              MSG_HELP_CO_BT_RADD);
    SetHelp(data->GUI.BT_RDEL,              MSG_HELP_CO_BT_RDEL);
    SetHelp(data->GUI.BT_FILTERUP,          MSG_HELP_CO_BT_FILTERUP);
    SetHelp(data->GUI.BT_FILTERDOWN,        MSG_HELP_CO_BT_FILTERDOWN);

    // set the cyclechain
    set(data->GUI.BT_APLAY, MUIA_CycleChain, TRUE);
    set(bt_moveto,MUIA_CycleChain, TRUE);
    set(data->GUI.BT_FILTERUP, MUIA_CycleChain, TRUE);
    set(data->GUI.BT_FILTERDOWN, MUIA_CycleChain, TRUE);
    set(data->GUI.BT_FILTER_IMPORT, MUIA_CycleChain, TRUE);

    GhostOutFilter(&(data->GUI), NULL);

    DoMethod(data->GUI.LV_RULES             ,MUIM_Notify, MUIA_NList_Active         ,MUIV_EveryTime ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&GetActiveFilterDataHook);
    DoMethod(data->GUI.ST_RNAME             ,MUIM_Notify, MUIA_String_Contents      ,MUIV_EveryTime ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&SetActiveFilterDataHook);
    DoMethod(data->GUI.CH_REMOTE            ,MUIM_Notify, MUIA_Selected             ,MUIV_EveryTime ,MUIV_Notify_Application        ,3 ,MUIM_CallHook          ,&CO_RemoteToggleHook       ,MUIV_TriggerValue);
    DoMethod(data->GUI.CH_APPLYREQ          ,MUIM_Notify, MUIA_Selected             ,MUIV_EveryTime ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&SetActiveFilterDataHook);
    DoMethod(data->GUI.CH_APPLYSENT         ,MUIM_Notify, MUIA_Selected             ,MUIV_EveryTime ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&SetActiveFilterDataHook);
    DoMethod(data->GUI.CH_APPLYNEW          ,MUIM_Notify, MUIA_Selected             ,MUIV_EveryTime ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&SetActiveFilterDataHook);
    DoMethod(data->GUI.CY_FILTER_COMBINE    ,MUIM_Notify, MUIA_Cycle_Active         , MUIV_EveryTime ,MUIV_Notify_Application       ,2 ,MUIM_CallHook          ,&SetActiveFilterDataHook);
    DoMethod(data->GUI.CH_AREDIRECT         ,MUIM_Notify, MUIA_Selected             ,MUIV_EveryTime ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&SetActiveFilterDataHook);
    DoMethod(data->GUI.CH_AFORWARD          ,MUIM_Notify, MUIA_Selected             ,MUIV_EveryTime ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&SetActiveFilterDataHook);
    DoMethod(data->GUI.CH_ARESPONSE         ,MUIM_Notify, MUIA_Selected             ,MUIV_EveryTime ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&SetActiveFilterDataHook);
    DoMethod(data->GUI.CH_AEXECUTE          ,MUIM_Notify, MUIA_Selected             ,MUIV_EveryTime ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&SetActiveFilterDataHook);
    DoMethod(data->GUI.CH_APLAY             ,MUIM_Notify, MUIA_Selected             ,MUIV_EveryTime ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&SetActiveFilterDataHook);
    DoMethod(data->GUI.CH_AMOVE             ,MUIM_Notify, MUIA_Selected             ,MUIV_EveryTime ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&SetActiveFilterDataHook);
    DoMethod(data->GUI.CH_ASTATUSTOMARKED   ,MUIM_Notify, MUIA_Selected             ,MUIV_EveryTime ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&SetActiveFilterDataHook);
    DoMethod(data->GUI.CH_ASTATUSTOUNMARKED ,MUIM_Notify, MUIA_Selected             ,MUIV_EveryTime ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&SetActiveFilterDataHook);
    DoMethod(data->GUI.CH_ASTATUSTOREAD     ,MUIM_Notify, MUIA_Selected             ,MUIV_EveryTime ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&SetActiveFilterDataHook);
    DoMethod(data->GUI.CH_ASTATUSTOUNREAD   ,MUIM_Notify, MUIA_Selected             ,MUIV_EveryTime ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&SetActiveFilterDataHook);
    DoMethod(data->GUI.CH_ASTATUSTOSPAM     ,MUIM_Notify, MUIA_Selected             ,MUIV_EveryTime ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&SetActiveFilterDataHook);
    DoMethod(data->GUI.CH_ASTATUSTOHAM      ,MUIM_Notify, MUIA_Selected             ,MUIV_EveryTime ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&SetActiveFilterDataHook);
    DoMethod(data->GUI.CH_ADELETE           ,MUIM_Notify, MUIA_Selected             ,MUIV_EveryTime ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&SetActiveFilterDataHook);
    DoMethod(data->GUI.CH_ASKIP             ,MUIM_Notify, MUIA_Selected             ,MUIV_EveryTime ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&SetActiveFilterDataHook);
    DoMethod(data->GUI.CH_ATERMINATE        ,MUIM_Notify, MUIA_Selected             ,MUIV_EveryTime ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&SetActiveFilterDataHook);
    DoMethod(data->GUI.ST_AREDIRECT         ,MUIM_Notify, MUIA_String_BufferPos     ,MUIV_EveryTime ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&SetActiveFilterDataHook);
    DoMethod(data->GUI.ST_AFORWARD          ,MUIM_Notify, MUIA_String_BufferPos     ,MUIV_EveryTime ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&SetActiveFilterDataHook);
    DoMethod(data->GUI.ST_ARESPONSE         ,MUIM_Notify, MUIA_String_Contents      ,MUIV_EveryTime ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&SetActiveFilterDataHook);
    DoMethod(data->GUI.ST_AEXECUTE          ,MUIM_Notify, MUIA_String_Contents      ,MUIV_EveryTime ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&SetActiveFilterDataHook);
    DoMethod(data->GUI.ST_APLAY             ,MUIM_Notify, MUIA_String_Contents      ,MUIV_EveryTime ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&SetActiveFilterDataHook);
    DoMethod(data->GUI.BT_APLAY             ,MUIM_Notify, MUIA_Pressed              ,FALSE          ,MUIV_Notify_Application        ,3 ,MUIM_CallHook          ,&CO_PlaySoundHook,data->GUI.ST_APLAY);
    DoMethod(data->GUI.TX_MOVETO            ,MUIM_Notify, MUIA_Text_Contents        ,MUIV_EveryTime ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&SetActiveFilterDataHook);
    DoMethod(data->GUI.LV_MOVETO            ,MUIM_Notify, MUIA_NList_DoubleClick,    TRUE           ,data->GUI.PO_MOVETO            ,2 ,MUIM_Popstring_Close   ,TRUE);
    DoMethod(data->GUI.BT_RADD              ,MUIM_Notify, MUIA_Pressed              ,FALSE          ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&AddNewFilterToListHook);
    DoMethod(data->GUI.BT_RDEL              ,MUIM_Notify, MUIA_Pressed              ,FALSE          ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&RemoveActiveFilterHook);
    DoMethod(data->GUI.BT_FILTERUP          ,MUIM_Notify, MUIA_Pressed              ,FALSE          ,data->GUI.LV_RULES             ,3 ,MUIM_NList_Move        ,MUIV_NList_Move_Selected   ,MUIV_NList_Move_Previous);
    DoMethod(data->GUI.BT_FILTERDOWN        ,MUIM_Notify, MUIA_Pressed              ,FALSE          ,data->GUI.LV_RULES             ,3 ,MUIM_NList_Move        ,MUIV_NList_Move_Selected   ,MUIV_NList_Move_Next);
    DoMethod(data->GUI.CH_AMOVE             ,MUIM_Notify, MUIA_Selected             ,TRUE           ,data->GUI.CH_ADELETE           ,3 ,MUIM_Set               ,MUIA_Selected              ,FALSE);
    DoMethod(data->GUI.CH_ADELETE           ,MUIM_Notify, MUIA_Selected             ,TRUE           ,data->GUI.CH_AMOVE             ,3 ,MUIM_Set               ,MUIA_Selected              ,FALSE);
    DoMethod(data->GUI.CH_ASTATUSTOMARKED   ,MUIM_Notify, MUIA_Selected             ,TRUE           ,data->GUI.CH_ASTATUSTOUNMARKED ,3 ,MUIM_Set               ,MUIA_Selected              ,FALSE);
    DoMethod(data->GUI.CH_ASTATUSTOUNMARKED ,MUIM_Notify, MUIA_Selected             ,TRUE           ,data->GUI.CH_ASTATUSTOMARKED   ,3 ,MUIM_Set               ,MUIA_Selected              ,FALSE);
    DoMethod(data->GUI.CH_ASTATUSTOREAD     ,MUIM_Notify, MUIA_Selected             ,TRUE           ,data->GUI.CH_ASTATUSTOUNREAD   ,3 ,MUIM_Set               ,MUIA_Selected              ,FALSE);
    DoMethod(data->GUI.CH_ASTATUSTOUNREAD   ,MUIM_Notify, MUIA_Selected             ,TRUE           ,data->GUI.CH_ASTATUSTOREAD     ,3 ,MUIM_Set               ,MUIA_Selected              ,FALSE);
    DoMethod(data->GUI.CH_ASTATUSTOSPAM     ,MUIM_Notify, MUIA_Selected             ,TRUE           ,data->GUI.CH_ASTATUSTOHAM      ,3 ,MUIM_Set               ,MUIA_Selected              ,FALSE);
    DoMethod(data->GUI.CH_ASTATUSTOHAM      ,MUIM_Notify, MUIA_Selected             ,TRUE           ,data->GUI.CH_ASTATUSTOSPAM     ,3 ,MUIM_Set               ,MUIA_Selected              ,FALSE);
    DoMethod(data->GUI.BT_FILTER_IMPORT     ,MUIM_Notify, MUIA_Pressed              ,FALSE          ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&ImportFilterHook);
  }

  RETURN(grp);
  return grp;
}

///
