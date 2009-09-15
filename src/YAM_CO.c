/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2009 by YAM Open Source Team

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <clib/alib_protos.h>
#include <diskfont/diskfonttag.h>
#include <libraries/amisslmaster.h>
#include <libraries/asl.h>
#include <libraries/iffparse.h>
#include <libraries/locale.h>
#include <mui/BetterString_mcc.h>
#include <mui/NListview_mcc.h>
#include <mui/TextEditor_mcc.h>
#include <proto/codesets.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/diskfont.h>
#include <proto/intuition.h>
#include <proto/muimaster.h>
#include <proto/utility.h>

#if defined(__amigaos4__)
#include <proto/application.h>
#endif

#include "extrasrc.h"

#include "SDI_hook.h"

#include "YAM.h"
#include "YAM_addressbook.h"
#include "YAM_config.h"
#include "YAM_configFile.h"
#include "YAM_configGUI.h"
#include "YAM_error.h"
#include "YAM_find.h"
#include "YAM_folderconfig.h"
#include "YAM_global.h"
#include "YAM_main.h"
#include "YAM_mainFolder.h"
#include "YAM_utilities.h"
#include "mui/Classes.h"

#include "FileInfo.h"
#include "Locale.h"
#include "MimeTypes.h"
#include "MUIObjects.h"
#include "Requesters.h"

#include "Debug.h"

extern struct Library *AmiSSLBase;
extern struct Library *AmiSSLMasterBase;

struct Config *C = NULL;
struct Config *CE = NULL;

/* local defines */
#if defined(__amigaos4__)
#define SYS_EDITOR "SYS:Tools/NotePad"
#elif defined(__AROS__)
#define SYS_EDITOR "SYS:Tools/Editor"
#else
#define SYS_EDITOR "C:Ed"
#endif

/* local protos */
static struct CO_ClassData *CO_New(void);

/***************************************************************************
 Module: Configuration
***************************************************************************/

/// CO_NewPrefsFile
//  Sets the name of the configuration file
static void CO_NewPrefsFile(char *fname)
{
  static char wtitle[SIZE_SMALL+SIZE_PATHFILE];

  strlcpy(G->CO_PrefsFile, fname, sizeof(G->CO_PrefsFile));
  snprintf(wtitle, sizeof(wtitle), "%s (%s)", tr(MSG_MA_MConfig), fname);

  set(G->CO->GUI.WI, MUIA_Window_Title, wtitle);
}
///

/**** Filters ****/
/// AddNewRuleToList
//  Adds a new entry to the current filter's rule list
HOOKPROTONHNONP(AddNewRuleToList, void)
{
  struct FilterNode *filter = NULL;
  struct CO_GUIData *gui = &G->CO->GUI;

  // get the active filterNode
  DoMethod(gui->LV_RULES, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &filter);

  if(filter != NULL)
  {
    struct RuleNode *rule;

    if((rule = CreateNewRule(filter)) != NULL)
    {
      // add a new GUI element for that particular rule
      Object *newSearchGroup = SearchControlGroupObject,
                                 MUIA_SearchControlGroup_RemoteFilterMode, filter->remote,
                                 MUIA_SearchControlGroup_ShowCombineCycle, TRUE,
                               End;

      if(newSearchGroup == NULL)
        return;

      // fill the new search group with some content
      DoMethod(newSearchGroup, MUIM_SearchControlGroup_GetFromRule, rule);

      // set some notifies
      DoMethod(newSearchGroup, MUIM_Notify, MUIA_SearchControlGroup_Modified, MUIV_EveryTime,
                               MUIV_Notify_Application, 2, MUIM_CallHook, &SetActiveFilterDataHook);

      // add it to our searchGroupList
      if(DoMethod(gui->GR_RGROUP, MUIM_Group_InitChange)) // required for a proper refresh
      {
        if(DoMethod(gui->GR_SGROUP, MUIM_Group_InitChange))
        {
          DoMethod(gui->GR_SGROUP, OM_ADDMEMBER, newSearchGroup);
          DoMethod(gui->GR_SGROUP, MUIM_Group_ExitChange);
        }
        DoMethod(gui->GR_RGROUP, MUIM_Group_ExitChange); // required for a proper refresh
      }

      GhostOutFilter(gui, filter);
    }
  }
}
MakeHook(AddNewRuleToListHook, AddNewRuleToList);

///
/// RemoveLastRule
//  Deletes the last rule of the filter
HOOKPROTONHNONP(RemoveLastRule, void)
{
  struct FilterNode *filter = NULL;
  struct CO_GUIData *gui = &G->CO->GUI;

  // get the active filterNode
  DoMethod(gui->LV_RULES, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &filter);

  // if we got an active entry lets remove it from the GUI List
  // and also from the filter's rule list
  if(filter)
  {
    // lets remove the rule at the end of our ruleList
    struct RuleNode *rule = (struct RuleNode *)RemTail((struct List *)&filter->ruleList);

    if(rule)
    {
      struct List *childList;

      // now we do free our search structure if it exists
      FreeRuleSearchData(rule);

      free(rule);

      // Remove the GUI elements as well
      if((childList = (struct List *)xget(gui->GR_SGROUP, MUIA_Group_ChildList)))
      {
        Object *cstate = (Object *)childList->lh_Head;
        Object *child;
        Object *lastChild = NULL;

        while((child = NextObject(&cstate)))
          lastChild = child;

        if(lastChild)
        {
          // remove the searchGroup
          DoMethod(gui->GR_SGROUP, MUIM_Group_InitChange);
          DoMethod(gui->GR_SGROUP, OM_REMMEMBER, lastChild);
          DoMethod(gui->GR_SGROUP, MUIM_Group_ExitChange);

          // Dipose the object
          MUI_DisposeObject(lastChild);

          GhostOutFilter(gui, filter);
        }
      }
    }
  }
}
MakeHook(RemoveLastRuleHook, RemoveLastRule);

///
/// GhostOutFilter
//  Enables/disables GUI gadgets in filter form
void GhostOutFilter(struct CO_GUIData *gui, struct FilterNode *filter)
{
  BOOL isremote;
  LONG pos = MUIV_NList_GetPos_Start;
  int numRules = 0;
  struct List *childList;

  ENTER();

  isremote = (filter != NULL) ? filter->remote : FALSE;

  set(gui->ST_RNAME,             MUIA_Disabled, filter == NULL);
  set(gui->CH_REMOTE,            MUIA_Disabled, filter == NULL);
  set(gui->CH_APPLYNEW,          MUIA_Disabled, filter == NULL || isremote);
  set(gui->CH_APPLYREQ,          MUIA_Disabled, filter == NULL || isremote);
  set(gui->CH_APPLYSENT,         MUIA_Disabled, filter == NULL || isremote);
  set(gui->CH_ABOUNCE,           MUIA_Disabled, filter == NULL || isremote);
  set(gui->CH_AFORWARD,          MUIA_Disabled, filter == NULL || isremote);
  set(gui->CH_ARESPONSE,         MUIA_Disabled, filter == NULL || isremote);
  set(gui->CH_AEXECUTE,          MUIA_Disabled, filter == NULL);
  set(gui->CH_APLAY,             MUIA_Disabled, filter == NULL);
  set(gui->CH_AMOVE,             MUIA_Disabled, filter == NULL || isremote);
  set(gui->CH_ADELETE,           MUIA_Disabled, filter == NULL);
  set(gui->CH_ASKIP,             MUIA_Disabled, filter == NULL || !isremote);
  set(gui->ST_ABOUNCE,           MUIA_Disabled, filter == NULL || isremote || !xget(gui->CH_ABOUNCE,   MUIA_Selected));
  set(gui->ST_AFORWARD,          MUIA_Disabled, filter == NULL || isremote || !xget(gui->CH_AFORWARD,  MUIA_Selected));
  set(gui->BT_APLAY,             MUIA_Disabled, filter == NULL || !xget(gui->CH_APLAY, MUIA_Selected));
  set(gui->PO_MOVETO,            MUIA_Disabled, filter == NULL || !xget(gui->CH_AMOVE, MUIA_Selected));
  set(gui->BT_RDEL,              MUIA_Disabled, filter == NULL);

  // lets make sure we ghost the filter up/down buttons if necessary
  if(filter != NULL)
    DoMethod(gui->LV_RULES, MUIM_NList_GetPos, filter, &pos);

  set(gui->BT_FILTERUP,   MUIA_Disabled, filter == NULL || pos == 0);
  set(gui->BT_FILTERDOWN, MUIA_Disabled, filter == NULL || pos+1 == (LONG)xget(gui->LV_RULES, MUIA_NList_Entries));

  // we have to find out how many rules the filter has
  if((childList = (struct List *)xget(gui->GR_SGROUP, MUIA_Group_ChildList)) != NULL)
  {
    Object *cstate = (Object *)childList->lh_Head;
    Object *child;

    while((child = NextObject(&cstate)) != NULL)
    {
      set(child, MUIA_Disabled, filter == NULL);
      numRules++;
    }
  }

  set(gui->BT_MORE, MUIA_Disabled, filter == NULL);
  set(gui->BT_LESS, MUIA_Disabled, filter == NULL || numRules <= 1);

  // These three "disables" must be done in another context, because the Popasl object will en/disable
  // the pop button itself as long as the requester is open. After that this hook is called but the object
  // has not yet enabled the pop button again, so we might get wrong visible results. Not a very nice
  // solution, I must say :(
  DoMethod(G->App, MUIM_Application_PushMethod, gui->PO_ARESPONSE, 3, MUIM_Set, MUIA_Disabled, filter == NULL || isremote || !xget(gui->CH_ARESPONSE, MUIA_Selected));
  DoMethod(G->App, MUIM_Application_PushMethod, gui->PO_AEXECUTE, 3, MUIM_Set, MUIA_Disabled, filter == NULL || !xget(gui->CH_AEXECUTE, MUIA_Selected));
  DoMethod(G->App, MUIM_Application_PushMethod, gui->PO_APLAY, 3, MUIM_Set, MUIA_Disabled, filter == NULL || !xget(gui->CH_APLAY, MUIA_Selected));

  LEAVE();
}

///
/// GetActiveFilterData
//  Fills GUI elements with data from currently active list entry (filter)
HOOKPROTONHNONP(GetActiveFilterData, void)
{
  struct FilterNode *filter = NULL;
  struct CO_GUIData *gui = &G->CO->GUI;

  ENTER();

  // get the active filterNode
  DoMethod(gui->LV_RULES, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &filter);

  // if we got an active entry lets set all other GUI elements from the
  // values of this filter
  if(filter)
  {
    struct List *childList;

    nnset(gui->ST_RNAME,      MUIA_String_Contents,   filter->name);
    nnset(gui->CH_REMOTE,     MUIA_Selected,          filter->remote);
    nnset(gui->CH_APPLYNEW,   MUIA_Selected,          filter->applyToNew);
    nnset(gui->CH_APPLYSENT,  MUIA_Selected,          filter->applyToSent);
    nnset(gui->CH_APPLYREQ,   MUIA_Selected,          filter->applyOnReq);
    nnset(gui->CH_ABOUNCE,    MUIA_Selected,          hasBounceAction(filter));
    nnset(gui->CH_AFORWARD,   MUIA_Selected,          hasForwardAction(filter));
    nnset(gui->CH_ARESPONSE,  MUIA_Selected,          hasReplyAction(filter));
    nnset(gui->CH_AEXECUTE,   MUIA_Selected,          hasExecuteAction(filter));
    nnset(gui->CH_APLAY,      MUIA_Selected,          hasPlaySoundAction(filter));
    nnset(gui->CH_AMOVE,      MUIA_Selected,          hasMoveAction(filter));
    nnset(gui->CH_ADELETE,    MUIA_Selected,          hasDeleteAction(filter));
    nnset(gui->CH_ASKIP,      MUIA_Selected,          hasSkipMsgAction(filter));
    nnset(gui->ST_ABOUNCE,    MUIA_String_Contents,   filter->bounceTo);
    nnset(gui->ST_AFORWARD,   MUIA_String_Contents,   filter->forwardTo);
    nnset(gui->ST_ARESPONSE,  MUIA_String_Contents,   filter->replyFile);
    nnset(gui->ST_AEXECUTE,   MUIA_String_Contents,   filter->executeCmd);
    nnset(gui->ST_APLAY,      MUIA_String_Contents,   filter->playSound);
    nnset(gui->TX_MOVETO,     MUIA_Text_Contents,     filter->moveTo);

    // before we actually set our rule options we have to clear out
    // all previous existing group childs
    if(DoMethod(gui->GR_RGROUP, MUIM_Group_InitChange)) // required for proper refresh
    {
      if(DoMethod(gui->GR_SGROUP, MUIM_Group_InitChange))
      {
        if((childList = (struct List *)xget(gui->GR_SGROUP, MUIA_Group_ChildList)))
        {
          int i;
          struct Node *curNode;
          Object *cstate = (Object *)childList->lh_Head;
          Object *child;

          while((child = NextObject(&cstate)))
          {
            // remove that child
            DoMethod(gui->GR_SGROUP, OM_REMMEMBER, child);
            MUI_DisposeObject(child);
          }

          // Now we should have a clean SGROUP and can populate with new SearchControlGroup
          // objects
          i = 0;
          IterateList(&filter->ruleList, curNode)
          {
            Object *newSearchGroup = SearchControlGroupObject,
                                       MUIA_SearchControlGroup_RemoteFilterMode, filter->remote,
                                       MUIA_SearchControlGroup_ShowCombineCycle, i > 0,
                                     End;

            if(newSearchGroup == NULL)
              break;

            // fill the new search group with some content
            DoMethod(newSearchGroup, MUIM_SearchControlGroup_GetFromRule, curNode);

            // set some notifies
            DoMethod(newSearchGroup, MUIM_Notify, MUIA_SearchControlGroup_Modified, MUIV_EveryTime,
                                     MUIV_Notify_Application, 2, MUIM_CallHook, &SetActiveFilterDataHook);

            // add it to our searchGroupList
            DoMethod(gui->GR_SGROUP, OM_ADDMEMBER, newSearchGroup);

            i++;
          }
        }
        DoMethod(gui->GR_SGROUP, MUIM_Group_ExitChange);
      }
      DoMethod(gui->GR_RGROUP, MUIM_Group_ExitChange); // required for proper refresh
    }
  }

  GhostOutFilter(gui, filter);

  LEAVE();
}
MakeHook(GetActiveFilterDataHook, GetActiveFilterData);

///
/// SetActiveFilterData
//  Fills filter data structure out of the GUI elements
HOOKPROTONHNONP(SetActiveFilterData, void)
{
  struct FilterNode *filter = NULL;
  struct CO_GUIData *gui = &G->CO->GUI;

  ENTER();

  // get the active filterNode
  DoMethod(gui->LV_RULES, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &filter);

  // if we got an active entry lets set all other GUI elements from the
  // values of this filter
  if(filter != NULL)
  {
    struct List *childList;
    int rm = GetMUICheck(gui->CH_REMOTE);

    GetMUIString(filter->name, gui->ST_RNAME, sizeof(filter->name));
    filter->remote = (rm == 1);
    filter->applyToNew  = GetMUICheck(gui->CH_APPLYNEW);
    filter->applyToSent = GetMUICheck(gui->CH_APPLYSENT);
    filter->applyOnReq  = GetMUICheck(gui->CH_APPLYREQ);
    filter->actions = 0;
    if(GetMUICheck(gui->CH_ABOUNCE))    SET_FLAG(filter->actions, FA_BOUNCE);
    if(GetMUICheck(gui->CH_AFORWARD))   SET_FLAG(filter->actions, FA_FORWARD);
    if(GetMUICheck(gui->CH_ARESPONSE))  SET_FLAG(filter->actions, FA_REPLY);
    if(GetMUICheck(gui->CH_AEXECUTE))   SET_FLAG(filter->actions, FA_EXECUTE);
    if(GetMUICheck(gui->CH_APLAY))      SET_FLAG(filter->actions, FA_PLAYSOUND);
    if(GetMUICheck(gui->CH_AMOVE))      SET_FLAG(filter->actions, FA_MOVE);
    if(GetMUICheck(gui->CH_ADELETE))    SET_FLAG(filter->actions, FA_DELETE);
    if(GetMUICheck(gui->CH_ASKIP))      SET_FLAG(filter->actions, FA_SKIPMSG);
    GetMUIString(filter->bounceTo,   gui->ST_ABOUNCE, sizeof(filter->bounceTo));
    GetMUIString(filter->forwardTo,  gui->ST_AFORWARD, sizeof(filter->forwardTo));
    GetMUIString(filter->replyFile,  gui->ST_ARESPONSE, sizeof(filter->replyFile));
    GetMUIString(filter->executeCmd, gui->ST_AEXECUTE, sizeof(filter->executeCmd));
    GetMUIString(filter->playSound,  gui->ST_APLAY, sizeof(filter->playSound));
    GetMUIText(filter->moveTo, gui->TX_MOVETO, sizeof(filter->moveTo));

    // make sure to update all rule settings
    if((childList = (struct List *)xget(gui->GR_SGROUP, MUIA_Group_ChildList)) != NULL)
    {
      Object *cstate = (Object *)childList->lh_Head;
      Object *child;
      int i=0;

      // iterate through the childList and update the rule structures
      while((child = NextObject(&cstate)) != NULL)
      {
        struct RuleNode *rule;

        // get the rule out of the ruleList or create a new one
        while((rule = GetFilterRule(filter, i)) == NULL)
          CreateNewRule(filter);

        // set the rule settings
        DoMethod(child, MUIM_SearchControlGroup_SetToRule, rule);

        ++i;
      }
    }

    GhostOutFilter(gui, filter);
    DoMethod(gui->LV_RULES, MUIM_NList_Redraw, MUIV_NList_Redraw_Active);
  }

  LEAVE();
}
MakeHook(SetActiveFilterDataHook, SetActiveFilterData);

///
/// CO_RemoteToggleFunc
//  Enables/disables GUI elements for remote filters
HOOKPROTONHNO(CO_RemoteToggleFunc, void, int *arg)
{
  BOOL rm = *arg;
  struct List *childList = (struct List *)xget(G->CO->GUI.GR_SGROUP, MUIA_Group_ChildList);

  if(childList)
  {
    Object *cstate = (Object *)childList->lh_Head;
    Object *child;

    while((child = NextObject(&cstate)))
    {
      set(child, MUIA_SearchControlGroup_RemoteFilterMode, rm);
    }
  }

  SetActiveFilterData();
}
MakeHook(CO_RemoteToggleHook,CO_RemoteToggleFunc);
///

/**** POP3 servers ****/
/// CO_NewPOP3
//  Initializes a new POP3 account
struct POP3 *CO_NewPOP3(struct Config *co, BOOL first)
{
  struct POP3 *pop3;

  ENTER();

  if((pop3 = (struct POP3 *)calloc(1, sizeof(struct POP3))) != NULL)
  {
    if(first)
    {
      char *p = strchr(co->EmailAddress, '@');

      strlcpy(pop3->User, co->EmailAddress, p ? (unsigned int)(p - co->EmailAddress + 1) : sizeof(pop3->User));
      strlcpy(pop3->Server, co->SMTP_Server, sizeof(pop3->Server));
    }

    pop3->Port = 110;
    pop3->Enabled = TRUE;
    pop3->DeleteOnServer = TRUE;
  }

  RETURN(pop3);
  return pop3;
}

///
/// CO_GetP3Entry
//  Fills form with data from selected list entry
HOOKPROTONHNONP(CO_GetP3Entry, void)
{
  struct POP3 *pop3 = NULL;
  struct CO_GUIData *gui = &G->CO->GUI;
  LONG pos = MUIV_NList_GetPos_Start;

  ENTER();

  DoMethod(gui->LV_POP3, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &pop3);

  set(gui->BT_PDEL, MUIA_Disabled, pop3 == NULL || xget(gui->LV_POP3, MUIA_NList_Entries) < 2);

  if(pop3 != NULL)
    DoMethod(gui->LV_POP3, MUIM_NList_GetPos, pop3, &pos);
  set(gui->BT_POPUP, MUIA_Disabled, pop3 == NULL || pos == 0);
  set(gui->BT_POPDOWN, MUIA_Disabled, pop3 == NULL || pos == (LONG)xget(gui->LV_POP3, MUIA_NList_Entries) - 1);

  if(pop3 != NULL)
  {
    nnset(gui->ST_POPACCOUNT, MUIA_String_Contents, pop3->Account);
    nnset(gui->ST_POPHOST,    MUIA_String_Contents, pop3->Server);
    nnset(gui->ST_POPPORT,    MUIA_String_Integer,  pop3->Port);
    nnset(gui->ST_POPUSERID,  MUIA_String_Contents, pop3->User);
    nnset(gui->ST_PASSWD,     MUIA_String_Contents, pop3->Password);
    nnset(gui->CH_POPENABLED, MUIA_Selected, pop3->Enabled);
    nnset(gui->CH_USEAPOP,    MUIA_Selected, pop3->UseAPOP);
    nnset(gui->CH_DELETE,     MUIA_Selected, pop3->DeleteOnServer);

    switch(pop3->SSLMode)
    {
      case P3SSL_TLS:
        nnset(gui->RA_POP3SECURE, MUIA_Radio_Active, 1);
      break;

      case P3SSL_SSL:
        nnset(gui->RA_POP3SECURE, MUIA_Radio_Active, 2);
      break;

      default:
        nnset(gui->RA_POP3SECURE, MUIA_Radio_Active, 0);
      break;
    }

    // we have to enabled/disable the SSL support accordingly
    set(gui->RA_POP3SECURE, MUIA_Disabled, G->TR_UseableTLS == FALSE);
  }

  LEAVE();
}
MakeHook(CO_GetP3EntryHook,CO_GetP3Entry);

///
/// CO_PutP3Entry
//  Fills form data into selected list entry
HOOKPROTONHNONP(CO_PutP3Entry, void)
{
  struct CO_GUIData *gui = &G->CO->GUI;
  int p;

  ENTER();

  p = xget(gui->LV_POP3, MUIA_NList_Active);
  if(p != MUIV_NList_Active_Off)
  {
    struct POP3 *pop3 = NULL;
    int new_ssl_mode = P3SSL_OFF;

    DoMethod(gui->LV_POP3, MUIM_NList_GetEntry, p, &pop3);
    if(pop3 != NULL)
    {
      GetMUIString(pop3->Account, gui->ST_POPACCOUNT, sizeof(pop3->Account));
      GetMUIString(pop3->Server, gui->ST_POPHOST, sizeof(pop3->Server));
      GetMUIString(pop3->User, gui->ST_POPUSERID, sizeof(pop3->User));
      GetMUIString(pop3->Password, gui->ST_PASSWD, sizeof(pop3->Password));
      pop3->Enabled = GetMUICheck(gui->CH_POPENABLED);
      pop3->UseAPOP = GetMUICheck(gui->CH_USEAPOP);
      pop3->DeleteOnServer = GetMUICheck(gui->CH_DELETE);

      // if the user hasn't yet entered an own account name or the default
      // account name is still present we go and set an automatic generated one
      if(pop3->Account[0] == '\0' || strcmp(pop3->Account, tr(MSG_NewEntry)) == 0)
        snprintf(pop3->Account, sizeof(pop3->Account), "%s@%s", pop3->User, pop3->Server);

      switch(GetMUIRadio(gui->RA_POP3SECURE))
      {
        // TLSv1 secure connection
        case 1:
          new_ssl_mode = P3SSL_TLS;
        break;

        // SSLv3 secure connection
        case 2:
          new_ssl_mode = P3SSL_SSL;
        break;

        // no secure connection
        default:
          new_ssl_mode = P3SSL_OFF;
        break;
      }

      // check if the user changed something at all
      if(pop3->SSLMode != new_ssl_mode)
      {
        if(new_ssl_mode == P3SSL_SSL)
          set(gui->ST_POPPORT, MUIA_String_Integer, 995);
        else
          set(gui->ST_POPPORT, MUIA_String_Integer, 110);

        pop3->SSLMode = new_ssl_mode;
      }

      pop3->Port = GetMUIInteger(gui->ST_POPPORT);

      DoMethod(gui->LV_POP3, MUIM_NList_Redraw, p);
    }
  }

  LEAVE();
}
MakeHook(CO_PutP3EntryHook,CO_PutP3Entry);

///
/// CO_GetDefaultPOPFunc
//  Sets values of first POP3 account
HOOKPROTONHNONP(CO_GetDefaultPOPFunc, void)
{
  struct POP3 *pop3 = CE->P3[0];

  ENTER();

  if(pop3 != NULL)
  {
    GetMUIString(pop3->Server, G->CO->GUI.ST_POPHOST0, sizeof(pop3->Server));
    pop3->Port = 110;
    GetMUIString(pop3->Password, G->CO->GUI.ST_PASSWD0, sizeof(pop3->Password));
    if(pop3->Account[0] == '\0')
      snprintf(pop3->Account, sizeof(pop3->Account), "%s@%s", pop3->User, pop3->Server);
  }

  LEAVE();
}
MakeHook(CO_GetDefaultPOPHook,CO_GetDefaultPOPFunc);
///

/**** ARexx Hooks ****/
/// CO_IsValid
//  Verifies if the required settings have been made
BOOL CO_IsValid(void)
{
   if (G->CO_Valid) return TRUE;
   if (G->CO) set(G->CO->GUI.WI,MUIA_Window_Open,TRUE);
   else DoMethod(G->App, MUIM_CallHook, &CO_OpenHook);
   MUI_Request(G->App, G->MA->GUI.WI, 0, NULL, tr(MSG_OkayReq), tr(MSG_CO_InvalidConf));
   return FALSE;
}

///
/// CO_DetectPGP
//  Checks if PGP 2 or 5 is available
static int CO_DetectPGP(const struct Config *co)
{
  int version = 0;
  APTR oldWindowPtr;
  char fname[SIZE_PATHFILE];

  ENTER();

  // make sure the OS doesn't popup any
  // 'Please insert volume' kind warnings
  oldWindowPtr = SetProcWindow((APTR)-1);

  if(FileExists(AddPath(fname, co->PGPCmdPath, "pgpe", sizeof(fname))))
  {
    version = 5;

    D(DBF_STARTUP, "found PGP version 5 installed in '%s'", co->PGPCmdPath);
  }
  else if(FileExists(AddPath(fname, co->PGPCmdPath, "pgp", sizeof(fname))))
  {
    version = 2;

    D(DBF_STARTUP, "found PGP version 2 installed in '%s'", co->PGPCmdPath);
  }
  else
    W(DBF_STARTUP, "no PGP version found to be installed in '%s'", co->PGPCmdPath);

  // restore the old windowPtr
  SetProcWindow(oldWindowPtr);

  RETURN(version);
  return version;
}

///
/// CO_ClearConfig
//  clears the content of a configuration structure
void CO_ClearConfig(struct Config *co)
{
  int i;

  ENTER();

  SHOWVALUE(DBF_CONFIG, co);

  // free all config elements
  for(i = 0; i < MAXP3; i++)
  {
    if(co->P3[i] != NULL)
      free(co->P3[i]);
  }

  // we have to free the mimeTypeList
  FreeMimeTypeList(&co->mimeTypeList);

  // we have to free the filterList
  FreeFilterList(&co->filterList);

  // clear the config
  memset(co, 0, sizeof(struct Config));

  // init the filterList & stuff
  NewList((struct List *)&co->mimeTypeList);
  NewList((struct List *)&co->filterList);

  LEAVE();
}

///
/// CO_SetDefaults
//  Sets configuration (or a part of it) to the factory settings
void CO_SetDefaults(struct Config *co, enum ConfigPage page)
{
  ENTER();
  SHOWVALUE(DBF_CONFIG, co);
  SHOWVALUE(DBF_CONFIG, page);

  if(page == cp_FirstSteps || page == cp_AllPages)
  {
    *co->RealName = *co->EmailAddress = 0;

    // If Locale is present, don't use the timezone from the config
    if(G->Locale) co->TimeZone = -G->Locale->loc_GMTOffset;
    else          co->TimeZone = 0;

     co->DaylightSaving = FALSE;
  }

  if(page == cp_TCPIP || page == cp_AllPages)
  {
    int i;

    for(i = 0; i < MAXP3; i++)
    {
      if(co->P3[i] != NULL)
      {
        free(co->P3[i]);
        co->P3[i] = NULL;
      }
    }

    *co->SMTP_Server = '\0';
    *co->SMTP_Domain = '\0';
    co->SMTP_Port = 25;
    co->Allow8bit = FALSE;
    co->SMTP_SecureMethod = SMTPSEC_NONE;
    co->Use_SMTP_AUTH = FALSE;
    *co->SMTP_AUTH_User = '\0';
    *co->SMTP_AUTH_Pass = '\0';
    co->SMTP_AUTH_Method = SMTPAUTH_AUTO;
    co->MailExchangeOrder = MEO_GET_FIRST;
    if((co->P3[0] = CO_NewPOP3(co, TRUE)) != NULL)
      co->P3[0]->DeleteOnServer = TRUE;
  }

  if(page == cp_NewMail || page == cp_AllPages)
  {
    co->AvoidDuplicates = FALSE;
    co->TransferWindow = TWM_AUTO;
    co->UpdateStatus = TRUE;
    co->DownloadLarge = TRUE;
    co->PreSelection = PSM_LARGE;
    co->WarnSize = 1024; // 1MB warn size
    co->CheckMailDelay = 0;
    co->NotifyType = 1;
    *co->NotifySound = *co->NotifyCommand = 0;
  }

  if(page == cp_Filters || page == cp_AllPages)
  {
    FreeFilterList(&co->filterList);
  }

  if(page == cp_Spam || page == cp_AllPages)
  {
    co->SpamFilterEnabled = FALSE;
    co->SpamFilterForNewMail = FALSE;
    co->SpamMarkOnMove = FALSE;
    co->SpamMarkAsRead = FALSE;
    co->SpamAddressBookIsWhiteList = FALSE;
    co->MoveHamToIncoming = FALSE;
    co->FilterHam = FALSE;
    co->SpamProbabilityThreshold = DEFAULT_SPAM_PROBABILITY_THRESHOLD;
    co->SpamFlushTrainingDataInterval = DEFAULT_FLUSH_TRAINING_DATA_INTERVAL;
    co->SpamFlushTrainingDataThreshold = DEFAULT_FLUSH_TRAINING_DATA_THRESHOLD;
  }

  if(page == cp_Read || page == cp_AllPages)
  {
    co->ShowHeader = 1;
    strlcpy(co->ShortHeaders, "(From|To|Cc|BCC|Date|Subject)", sizeof(co->ShortHeaders));
    co->ShowSenderInfo = 2;
    strlcpy(co->ColoredText.buf, "m6", sizeof(co->ColoredText.buf));
    strlcpy(co->Color1stLevel.buf, "m0", sizeof(co->Color1stLevel.buf));
    strlcpy(co->Color2ndLevel.buf, "m7", sizeof(co->Color2ndLevel.buf));
    strlcpy(co->Color3rdLevel.buf, "m3", sizeof(co->Color3rdLevel.buf));
    strlcpy(co->Color4thLevel.buf, "m1", sizeof(co->Color4thLevel.buf));
    strlcpy(co->ColorURL.buf, "p6", sizeof(co->ColorURL.buf));
    strlcpy(co->ColorSignature.buf, "m4", sizeof(co->ColorSignature.buf));
    co->DisplayAllTexts = TRUE;
    co->FixedFontEdit = TRUE;
    co->UseTextStylesRead = TRUE;
    co->UseTextColorsRead = TRUE;
    co->DisplayAllAltPart = FALSE; // hide all sub "multipart/alternative" parts per default
    co->WrapHeader = FALSE;
    co->MultipleReadWindows = FALSE;
    co->SigSepLine = SST_BAR;
    co->StatusChangeDelayOn = TRUE;
    co->StatusChangeDelay   = 1000; // 1s=1000ms delay by default
    co->ConvertHTML = TRUE;
    co->MDNEnabled = TRUE;
    co->MDN_NoRecipient = MDN_ACTION_ASK;
    co->MDN_NoDomain = MDN_ACTION_ASK;
    co->MDN_OnDelete = MDN_ACTION_ASK;
    co->MDN_Other = MDN_ACTION_ASK;
    co->DetectCyrillic = FALSE;
    co->MapForeignChars = TRUE;
    co->GlobalMailThreads = FALSE;
  }

  if(page == cp_Write || page == cp_AllPages)
  {
    *co->ReplyTo = '\0';
    *co->Organization = '\0';
    *co->ExtraHeaders = '\0';
    strlcpy(co->NewIntro, tr(MSG_CO_NewIntroDef), sizeof(co->NewIntro));
    strlcpy(co->Greetings, tr(MSG_CO_GreetingsDef), sizeof(co->Greetings));
    co->WarnSubject = TRUE;
    co->EdWrapCol = 78;
    co->EdWrapMode = EWM_EDITING;
    co->UseFixedFontWrite = TRUE;
    co->UseTextStylesWrite = TRUE;
    co->UseTextColorsWrite = TRUE;
    strlcpy(co->Editor, SYS_EDITOR, sizeof(co->Editor));
    co->LaunchAlways = FALSE;
    co->EmailCache = 10;
    co->AutoSave = 120;
    co->RequestMDN = FALSE;
    co->SaveSent = TRUE;
  }

  if(page == cp_ReplyForward || page == cp_AllPages)
  {
    strlcpy(co->ReplyHello, "Hello %f\\n", sizeof(co->ReplyHello));
    strlcpy(co->ReplyIntro, "On %d, you wrote:\\n", sizeof(co->ReplyIntro));
    strlcpy(co->ReplyBye, "Regards", sizeof(co->ReplyBye));
    strlcpy(co->AltReplyHello, tr(MSG_CO_AltRepHelloDef), sizeof(co->AltReplyHello));
    strlcpy(co->AltReplyIntro, tr(MSG_CO_AltRepIntroDef), sizeof(co->AltReplyIntro));
    strlcpy(co->AltReplyBye, tr(MSG_CO_AltRepByeDef), sizeof(co->AltReplyBye));
    strlcpy(co->AltReplyPattern, tr(MSG_CO_AltRepPatternDef), sizeof(co->AltReplyPattern));
    strlcpy(co->MLReplyHello, tr(MSG_CO_MLRepHelloDef), sizeof(co->MLReplyHello));
    strlcpy(co->MLReplyIntro, tr(MSG_CO_MLRepIntroDef), sizeof(co->MLReplyIntro));
    strlcpy(co->MLReplyBye, tr(MSG_CO_MLRepByeDef), sizeof(co->MLReplyBye));
    strlcpy(co->ForwardIntro, tr(MSG_CO_ForwardIntroDef), sizeof(co->ForwardIntro));
    strlcpy(co->ForwardFinish, tr(MSG_CO_ForwardFinishDef), sizeof(co->ForwardFinish));
    strlcpy(co->QuoteChar, ">", sizeof(co->QuoteChar));
    strlcpy(co->AltQuoteChar, "|", sizeof(co->AltQuoteChar));

    co->QuoteMessage = TRUE;
    co->QuoteEmptyLines = TRUE;
    co->CompareAddress = TRUE;
    co->StripSignature = TRUE;
    co->ForwardMode = FWM_ATTACH;
  }

  if(page == cp_Signature || page == cp_AllPages)
  {
    co->UseSignature = FALSE;
    AddPath(co->TagsFile, G->ProgDir, ".taglines", sizeof(co->TagsFile));
    strlcpy(co->TagsSeparator, "%%", sizeof(co->TagsSeparator));
  }

  if(page == cp_Lists || page == cp_AllPages)
  {
    co->FolderCols = (FCOL_NAME | FCOL_TOTAL);
    co->MessageCols = (MCOL_STATUS | MCOL_SENDER | MCOL_SUBJECT | MCOL_DATE | MCOL_SIZE);
    co->FixedFontList = FALSE;
    co->DSListFormat = DSS_RELDATETIME;
    co->ABookLookup = FALSE;
    co->FolderCntMenu = TRUE;
    co->MessageCntMenu = TRUE;
    co->FolderInfoMode = FIM_NAME_AND_UNREAD_MAILS;
    co->FolderDoubleClick = TRUE;
  }

  if(page == cp_Security || page == cp_AllPages)
  {
    // we first try to see if there is a PGPPATH variable and if
    // so we take that one as the path to PGP or we plainly take PGP:
    // as the default path.
    if(GetVar("PGPPATH", co->PGPCmdPath, sizeof(co->PGPCmdPath), 0) == -1)
      strlcpy(co->PGPCmdPath, "PGP:", sizeof(co->PGPCmdPath));

    co->MyPGPID[0] = '\0';
    co->PGPURL[0] = '\0';
    co->EncryptToSelf = TRUE;
    co->LogAllEvents = TRUE;
    co->PGPPassInterval = 10; // 10 min per default
    strlcpy(co->ReMailer, "Remailer <remailer@remailer.xganon.com>", sizeof(co->ReMailer));
    strlcpy(co->RMCommands, "Anon-To: %s", sizeof(co->RMCommands));
    strlcpy(co->LogfilePath, G->ProgDir, sizeof(co->LogfilePath));
    co->LogfileMode = LF_NORMAL;
    co->SplitLogfile = FALSE;
  }

  if(page == cp_StartupQuit || page == cp_AllPages)
  {
    co->GetOnStartup = FALSE;
    co->SendOnStartup = FALSE;
    co->LoadAllFolders = FALSE;
    co->SendOnQuit = FALSE;
    co->CleanupOnStartup = FALSE;
    co->RemoveOnStartup = FALSE;
    co->UpdateNewMail = TRUE;
    co->CheckBirthdates = TRUE;
    co->CleanupOnQuit = TRUE;
    co->RemoveOnQuit = TRUE;
  }

  if(page == cp_MIME || page == cp_AllPages)
  {
    FreeMimeTypeList(&co->mimeTypeList);
    strlcpy(co->DefaultMimeViewer, "SYS:Utilities/Multiview \"%s\"", sizeof(co->DefaultMimeViewer));
  }

  if(page == cp_AddressBook || page == cp_AllPages)
  {
    AddPath(co->GalleryDir, G->ProgDir, "Gallery", sizeof(co->GalleryDir));
    strlcpy(co->NewAddrGroup, "NEW", sizeof(co->NewAddrGroup));
    co->AddMyInfo = FALSE;
    co->AddToAddrbook = 0;
    co->AddrbookCols = 1+2+4;
  }

  if(page == cp_Scripts || page == cp_AllPages)
  {
    int i;

    for(i = 0; i < MAXRX; i++)
    {
      *co->RX[i].Name = '\0';
      *co->RX[i].Script = '\0';
      co->RX[i].IsAmigaDOS = co->RX[i].UseConsole = FALSE;
      co->RX[i].WaitTerm = TRUE;
    }
  }

  if(page == cp_Mixed || page == cp_AllPages)
  {
    strlcpy(co->TempDir, "T:", sizeof(co->TempDir));
    strlcpy(co->DetachDir, "RAM:", sizeof(co->DetachDir));
    strlcpy(co->AttachDir, "RAM:", sizeof(co->AttachDir));
    strlcpy(co->PackerCommand, "LhA -a -m -i%l a \"%a\"", sizeof(co->PackerCommand));
    co->IconPositionX = -1; // < 0 means free positioning
    co->IconPositionY = -1; // < 0 means free positioning
    strlcpy(co->AppIconText, tr(MSG_CO_APPICON_LABEL), sizeof(co->AppIconText));
    co->IconifyOnQuit = co->RemoveAtOnce = FALSE;
    co->Confirm = TRUE;
    co->ConfirmDelete = 2;
    strlcpy(co->XPKPack, "HUFF", sizeof(co->XPKPack));
    strlcpy(co->XPKPackEncrypt, "HUFF", sizeof(co->XPKPackEncrypt));
    co->XPKPackEff = 50;
    co->XPKPackEncryptEff = 50;

    // depending on the operating system we set the AppIcon
    // and docky icon defaults different
    #if defined(__amigaos4__)
    if(ApplicationBase)
    {
      co->DockyIcon = TRUE;
      co->WBAppIcon = FALSE;
    }
    else
    #endif
      co->WBAppIcon = TRUE;
  }

  if(page == cp_LookFeel || page == cp_AllPages)
  {
    strlcpy(co->ThemeName, "default", sizeof(co->ThemeName));
    co->InfoBar = IB_POS_CENTER;
    strlcpy(co->InfoBarText, tr(MSG_CO_InfoBarDef), sizeof(co->InfoBarText));
    co->EmbeddedReadPane = TRUE;
    co->QuickSearchBar = TRUE;
    co->SizeFormat = SF_MIXED;
  }

  if(page == cp_Update || page == cp_AllPages)
  {
    co->UpdateInterval = 604800; // check weekly for updates per default
    SetDefaultUpdateState();
  }

  // everything else
  if(page == cp_AllPages)
  {
    co->LetterPart = 1;
    co->WriteIndexes = 120;
    strlcpy(co->SupportSite, "http://www.yam.ch/", sizeof(co->SupportSite));
    strlcpy(co->UpdateServer, "http://update.yam.ch/", sizeof(co->UpdateServer));
    co->JumpToNewMsg = TRUE;
    co->JumpToIncoming = FALSE;
    co->JumpToRecentMsg = FALSE;
    co->AskJumpUnread = TRUE;
    co->PrinterCheck = TRUE;
    co->IsOnlineCheck = TRUE;
    co->ConfirmOnQuit = FALSE;
    co->HideGUIElements = 0;
    strlcpy(co->DefaultReadCharset, "ISO-8859-1", sizeof(co->DefaultReadCharset));
    strlcpy(co->DefaultWriteCharset, "ISO-8859-1", sizeof(co->DefaultWriteCharset));
    co->SysCharsetCheck = TRUE;
    co->AmiSSLCheck = TRUE;
    co->TimeZoneCheck = TRUE;
    co->AutoDSTCheck = TRUE;
    co->PrintMethod = PRINTMETHOD_RAW;
    co->StackSize = 40000;
    co->AutoColumnResize = TRUE;
    co->SocketOptions.SendBuffer  = -1;
    co->SocketOptions.RecvBuffer  = -1;
    co->SocketOptions.SendLowAt   = -1;
    co->SocketOptions.RecvLowAt   = -1;
    co->SocketOptions.SendTimeOut = -1;
    co->SocketOptions.RecvTimeOut = -1;
    co->SocketOptions.KeepAlive   = FALSE;
    co->SocketOptions.NoDelay     = FALSE;
    co->SocketOptions.LowDelay    = FALSE;
    co->SocketTimeout = 30; // 30s socket timeout per default
    co->TRBufferSize = 8192; // 8K buffer per default
    co->EmbeddedMailDelay = 200; // 200ms delay per default
    co->KeepAliveInterval = 30;  // 30s interval per default
    co->AutoClip = FALSE;

    // set the default styles of the folder listtree and
    // mail list items.
    strlcpy(co->StyleFGroupUnread, MUIX_B MUIX_I,         sizeof(co->StyleFGroupUnread));
    strlcpy(co->StyleFGroupRead,   MUIX_B MUIX_I "\0334", sizeof(co->StyleFGroupRead));
    strlcpy(co->StyleFolderUnread, MUIX_B        "\0334", sizeof(co->StyleFolderUnread));
    strlcpy(co->StyleFolderRead,   "",                    sizeof(co->StyleFolderRead));
    strlcpy(co->StyleFolderNew,    MUIX_B,                sizeof(co->StyleFolderNew));
    strlcpy(co->StyleMailUnread,   MUIX_B,                sizeof(co->StyleMailUnread));
    strlcpy(co->StyleMailRead,     "",                    sizeof(co->StyleMailRead));
  }

  LEAVE();
}

///
/// CopyConfigData
//  Copies a configuration structure (deep copy)
static BOOL CopyConfigData(struct Config *dco, const struct Config *sco)
{
  int i;
  BOOL success = TRUE;

  ENTER();
  SHOWVALUE(DBF_CONFIG, sco);
  SHOWVALUE(DBF_CONFIG, dco);

  // first we copy all raw data via memcpy
  memcpy(dco, sco, sizeof(struct Config));

  // then we have to do a deep copy and allocate separate memory for our copy
  for(i = 0; i < MAXP3; i++)
  {
    if(sco->P3[i] != NULL)
    {
      if((dco->P3[i] = memdup(sco->P3[i], sizeof(struct POP3))) == NULL)
      {
        success = FALSE;
        // don't bail out here, because we did a raw copy of the source config
        // before and didn't adjust all pointers yet
      }
    }
    else
      dco->P3[i] = NULL;
  }

  // for copying the mimetype list we have to do a deep copy of the list
  NewList((struct List *)&dco->mimeTypeList);

  if(success == TRUE)
  {
    struct Node *curNode;

    IterateList(&sco->mimeTypeList, curNode)
    {
      struct MimeTypeNode *srcNode = (struct MimeTypeNode *)curNode;
      struct MimeTypeNode *dstNode;

      if((dstNode = memdup(srcNode, sizeof(struct MimeTypeNode))) != NULL)
        AddTail((struct List *)&dco->mimeTypeList, (struct Node *)dstNode);
      else
      {
        success = FALSE;
        // bail out, no need to copy further data
        break;
      }
    }
  }

  // for copying the filters we do have to do another deep copy
  NewList((struct List *)&dco->filterList);

  if(success == TRUE)
  {
    struct Node *curNode;

    IterateList(&sco->filterList, curNode)
    {
      struct FilterNode *srcFilter = (struct FilterNode *)curNode;
      struct FilterNode *dstFilter;

      if((dstFilter = malloc(sizeof(struct FilterNode))) != NULL)
      {
        if(CopyFilterData(dstFilter, srcFilter) == FALSE)
        {
          success = FALSE;
          // bail out, no need to copy further data
          break;
        }

        AddTail((struct List *)&dco->filterList, (struct Node *)dstFilter);
      }
      else
      {
        success = FALSE;
        // bail out, no need to copy further data
        break;
      }
    }
  }

  // remember that this configuration is not yet saved
  dco->ConfigIsSaved = FALSE;

  // return if everything could be duplicated successfully
  RETURN(success);
  return success;
}

///
/// ComparePOP3Accounts
// compare two POP3 accounts to be equal
static BOOL ComparePOP3Accounts(const struct POP3 **pop1, const struct POP3 **pop2)
{
  BOOL equal = TRUE;
  int i;

  ENTER();

  for(i = 0; i < MAXP3; i++)
  {
    const struct POP3 *p1 = pop1[i];
    const struct POP3 *p2 = pop2[i];

    if(p1 != NULL && p2 != NULL)
    {
      // "UIDLchecked" must not be checked, because that is not saved but
      // modified while YAM is looking for new mails.
      if(strcmp(p1->Account,   p2->Account) != 0 ||
         strcmp(p1->Server,    p2->Server) != 0 ||
         strcmp(p1->User,      p2->User) != 0 ||
         strcmp(p1->Password,  p2->Password) != 0 ||
         p1->Port           != p2->Port ||
         p1->Enabled        != p2->Enabled ||
         p1->SSLMode        != p2->SSLMode ||
         p1->UseAPOP        != p2->UseAPOP ||
         p1->DeleteOnServer != p2->DeleteOnServer)
      {
        // one POP3 account has been modified
        equal = FALSE;
      }
    }
    else if(p1 != NULL || p2 != NULL)
    {
      // the number of POP3 accounts differs
      equal = FALSE;
    }
  }

  RETURN(equal);
  return equal;
}

///
/// CompareRxHooks
// compare two RxHook structures to be equal
static BOOL CompareRxHooks(const struct RxHook *rx1, const struct RxHook *rx2)
{
  BOOL equal = TRUE;
  int i;

  ENTER();

  for(i = 0; i < MAXRX; i++)
  {
    const struct RxHook *r1 = &rx1[i];
    const struct RxHook *r2 = &rx2[i];

    if(r1->IsAmigaDOS  != r2->IsAmigaDOS ||
       r1->UseConsole  != r2->UseConsole ||
       r1->WaitTerm    != r2->WaitTerm ||
       strcmp(r1->Name,   r2->Name) != 0 ||
       strcmp(r1->Script, r2->Script) != 0)
    {
      // something does not match
      equal = FALSE;
      break;
    }
  }

  RETURN(equal);
  return equal;
}

///
/// CompareConfigData
// compares two config data structures (deep compare) and returns TRUE if they are equal
static BOOL CompareConfigData(const struct Config *c1, const struct Config *c2)
{
  BOOL equal = FALSE;

  ENTER();

  // we do a deep compare here, but start the compare by comparing our normal
  // plain variables as this will be the faster compare than the compares
  // of our nested structures/lists, etc.
  if(c1->TimeZone                        == c2->TimeZone &&
     c1->WarnSize                        == c2->WarnSize &&
     c1->CheckMailDelay                  == c2->CheckMailDelay &&
     c1->NotifyType                      == c2->NotifyType &&
     c1->ShowHeader                      == c2->ShowHeader &&
     c1->ShowSenderInfo                  == c2->ShowSenderInfo &&
     c1->EdWrapCol                       == c2->EdWrapCol &&
     c1->EdWrapMode                      == c2->EdWrapMode &&
     c1->FolderCols                      == c2->FolderCols &&
     c1->MessageCols                     == c2->MessageCols &&
     c1->AddToAddrbook                   == c2->AddToAddrbook &&
     c1->AddrbookCols                    == c2->AddrbookCols &&
     c1->IconPositionX                   == c2->IconPositionX &&
     c1->IconPositionY                   == c2->IconPositionY &&
     c1->ConfirmDelete                   == c2->ConfirmDelete &&
     c1->XPKPackEff                      == c2->XPKPackEff &&
     c1->XPKPackEncryptEff               == c2->XPKPackEncryptEff &&
     c1->LetterPart                      == c2->LetterPart &&
     c1->WriteIndexes                    == c2->WriteIndexes &&
     c1->AutoSave                        == c2->AutoSave &&
     c1->HideGUIElements                 == c2->HideGUIElements &&
     c1->StackSize                       == c2->StackSize &&
     c1->SizeFormat                      == c2->SizeFormat &&
     c1->EmailCache                      == c2->EmailCache &&
     c1->SMTP_Port                       == c2->SMTP_Port &&
     c1->TRBufferSize                    == c2->TRBufferSize &&
     c1->EmbeddedMailDelay               == c2->EmbeddedMailDelay &&
     c1->StatusChangeDelay               == c2->StatusChangeDelay &&
     c1->KeepAliveInterval               == c2->KeepAliveInterval &&
     c1->UpdateInterval                  == c2->UpdateInterval &&
     c1->PGPPassInterval                 == c2->PGPPassInterval &&
     c1->SpamProbabilityThreshold        == c2->SpamProbabilityThreshold &&
     c1->SpamFlushTrainingDataInterval   == c2->SpamFlushTrainingDataInterval &&
     c1->SpamFlushTrainingDataThreshold  == c2->SpamFlushTrainingDataThreshold &&
     c1->SocketTimeout                   == c2->SocketTimeout &&
     c1->PrintMethod                     == c2->PrintMethod &&
     c1->SMTP_SecureMethod               == c2->SMTP_SecureMethod &&
     c1->LogfileMode                     == c2->LogfileMode &&
     c1->SMTP_AUTH_Method                == c2->SMTP_AUTH_Method &&
     c1->MailExchangeOrder               == c2->MailExchangeOrder &&
     c1->MDN_NoRecipient                 == c2->MDN_NoRecipient &&
     c1->MDN_NoDomain                    == c2->MDN_NoDomain &&
     c1->MDN_OnDelete                    == c2->MDN_OnDelete &&
     c1->MDN_Other                       == c2->MDN_Other &&
     c1->DSListFormat                    == c2->DSListFormat &&
     c1->SigSepLine                      == c2->SigSepLine &&
     c1->TransferWindow                  == c2->TransferWindow &&
     c1->PreSelection                    == c2->PreSelection &&
     c1->FolderInfoMode                  == c2->FolderInfoMode &&
     c1->ForwardMode                     == c2->ForwardMode &&
     c1->InfoBar                         == c2->InfoBar &&
     c1->DaylightSaving                  == c2->DaylightSaving &&
     c1->Allow8bit                       == c2->Allow8bit &&
     c1->Use_SMTP_AUTH                   == c2->Use_SMTP_AUTH &&
     c1->AvoidDuplicates                 == c2->AvoidDuplicates &&
     c1->UpdateStatus                    == c2->UpdateStatus &&
     c1->DownloadLarge                   == c2->DownloadLarge &&
     c1->DisplayAllTexts                 == c2->DisplayAllTexts &&
     c1->FixedFontEdit                   == c2->FixedFontEdit &&
     c1->MultipleReadWindows             == c2->MultipleReadWindows &&
     c1->UseTextStylesRead               == c2->UseTextStylesRead &&
     c1->UseTextColorsRead               == c2->UseTextColorsRead &&
     c1->UseFixedFontWrite               == c2->UseFixedFontWrite &&
     c1->UseTextStylesWrite              == c2->UseTextStylesWrite &&
     c1->UseTextColorsWrite              == c2->UseTextColorsWrite &&
     c1->WrapHeader                      == c2->WrapHeader &&
     c1->LaunchAlways                    == c2->LaunchAlways &&
     c1->QuoteMessage                    == c2->QuoteMessage &&
     c1->QuoteEmptyLines                 == c2->QuoteEmptyLines &&
     c1->CompareAddress                  == c2->CompareAddress &&
     c1->StripSignature                  == c2->StripSignature &&
     c1->UseSignature                    == c2->UseSignature &&
     c1->FixedFontList                   == c2->FixedFontList &&
     c1->EncryptToSelf                   == c2->EncryptToSelf &&
     c1->SplitLogfile                    == c2->SplitLogfile &&
     c1->LogAllEvents                    == c2->LogAllEvents &&
     c1->GetOnStartup                    == c2->GetOnStartup &&
     c1->SendOnStartup                   == c2->SendOnStartup &&
     c1->CleanupOnStartup                == c2->CleanupOnStartup &&
     c1->RemoveOnStartup                 == c2->RemoveOnStartup &&
     c1->LoadAllFolders                  == c2->LoadAllFolders &&
     c1->UpdateNewMail                   == c2->UpdateNewMail &&
     c1->CheckBirthdates                 == c2->CheckBirthdates &&
     c1->SendOnQuit                      == c2->SendOnQuit &&
     c1->CleanupOnQuit                   == c2->CleanupOnQuit &&
     c1->RemoveOnQuit                    == c2->RemoveOnQuit &&
     c1->AddMyInfo                       == c2->AddMyInfo &&
     c1->IconifyOnQuit                   == c2->IconifyOnQuit &&
     c1->Confirm                         == c2->Confirm &&
     c1->RemoveAtOnce                    == c2->RemoveAtOnce &&
     c1->SaveSent                        == c2->SaveSent &&
     c1->JumpToNewMsg                    == c2->JumpToNewMsg &&
     c1->JumpToIncoming                  == c2->JumpToIncoming &&
     c1->JumpToRecentMsg                 == c2->JumpToRecentMsg &&
     c1->PrinterCheck                    == c2->PrinterCheck &&
     c1->IsOnlineCheck                   == c2->IsOnlineCheck &&
     c1->ConfirmOnQuit                   == c2->ConfirmOnQuit &&
     c1->AskJumpUnread                   == c2->AskJumpUnread &&
     c1->WarnSubject                     == c2->WarnSubject &&
     c1->FolderCntMenu                   == c2->FolderCntMenu &&
     c1->MessageCntMenu                  == c2->MessageCntMenu &&
     c1->AutoColumnResize                == c2->AutoColumnResize &&
     c1->EmbeddedReadPane                == c2->EmbeddedReadPane &&
     c1->StatusChangeDelayOn             == c2->StatusChangeDelayOn &&
     c1->SysCharsetCheck                 == c2->SysCharsetCheck &&
     c1->QuickSearchBar                  == c2->QuickSearchBar &&
     c1->WBAppIcon                       == c2->WBAppIcon &&
     c1->DockyIcon                       == c2->DockyIcon &&
     c1->AmiSSLCheck                     == c2->AmiSSLCheck &&
     c1->TimeZoneCheck                   == c2->TimeZoneCheck &&
     c1->AutoDSTCheck                    == c2->AutoDSTCheck &&
     c1->DetectCyrillic                  == c2->DetectCyrillic &&
     c1->MapForeignChars                 == c2->MapForeignChars &&
     c1->GlobalMailThreads               == c2->GlobalMailThreads &&
     c1->ABookLookup                     == c2->ABookLookup &&
     c1->ConvertHTML                     == c2->ConvertHTML &&
     c1->SpamFilterEnabled               == c2->SpamFilterEnabled &&
     c1->SpamFilterForNewMail            == c2->SpamFilterForNewMail &&
     c1->SpamMarkOnMove                  == c2->SpamMarkOnMove &&
     c1->SpamMarkAsRead                  == c2->SpamMarkAsRead &&
     c1->SpamAddressBookIsWhiteList      == c2->SpamAddressBookIsWhiteList &&
     c1->MoveHamToIncoming               == c2->MoveHamToIncoming &&
     c1->FilterHam                       == c2->FilterHam &&
     c1->DisplayAllAltPart               == c2->DisplayAllAltPart &&
     c1->MDNEnabled                      == c2->MDNEnabled &&
     c1->RequestMDN                      == c2->RequestMDN &&
     c1->AutoClip                        == c2->AutoClip &&
     c1->FolderDoubleClick               == c2->FolderDoubleClick &&

     c1->SocketOptions.SendBuffer        == c2->SocketOptions.SendBuffer &&
     c1->SocketOptions.RecvBuffer        == c2->SocketOptions.RecvBuffer &&
     c1->SocketOptions.SendLowAt         == c2->SocketOptions.SendLowAt &&
     c1->SocketOptions.RecvLowAt         == c2->SocketOptions.RecvLowAt &&
     c1->SocketOptions.SendTimeOut       == c2->SocketOptions.SendTimeOut &&
     c1->SocketOptions.RecvTimeOut       == c2->SocketOptions.RecvTimeOut &&
     c1->SocketOptions.KeepAlive         == c2->SocketOptions.KeepAlive &&
     c1->SocketOptions.NoDelay           == c2->SocketOptions.NoDelay &&
     c1->SocketOptions.LowDelay          == c2->SocketOptions.LowDelay &&

     ComparePOP3Accounts((const struct POP3 **)c1->P3, (const struct POP3 **)c2->P3) &&
     CompareFilterLists(&c1->filterList, &c2->filterList) &&
     CompareMimeTypeLists(&c1->mimeTypeList, &c2->mimeTypeList) &&
     CompareRxHooks((const struct RxHook *)c1->RX, (const struct RxHook *)c2->RX) &&

     strcmp(c1->ColoredText.buf,    c2->ColoredText.buf) == 0 &&
     strcmp(c1->Color1stLevel.buf,  c2->Color1stLevel.buf) == 0 &&
     strcmp(c1->Color2ndLevel.buf,  c2->Color2ndLevel.buf) == 0 &&
     strcmp(c1->Color3rdLevel.buf,  c2->Color3rdLevel.buf) == 0 &&
     strcmp(c1->Color4thLevel.buf,  c2->Color4thLevel.buf) == 0 &&
     strcmp(c1->ColorURL.buf,       c2->ColorURL.buf) == 0 &&
     strcmp(c1->ColorSignature.buf, c2->ColorSignature.buf) == 0 &&
     strcmp(c1->RealName,           c2->RealName) == 0 &&
     strcmp(c1->EmailAddress,       c2->EmailAddress) == 0 &&
     strcmp(c1->SMTP_Server,        c2->SMTP_Server) == 0 &&
     strcmp(c1->SMTP_Domain,        c2->SMTP_Domain) == 0 &&
     strcmp(c1->SMTP_AUTH_User,     c2->SMTP_AUTH_User) == 0 &&
     strcmp(c1->SMTP_AUTH_Pass,     c2->SMTP_AUTH_Pass) == 0 &&
     strcmp(c1->NotifySound,        c2->NotifySound) == 0 &&
     strcmp(c1->NotifyCommand,      c2->NotifyCommand) == 0 &&
     strcmp(c1->ShortHeaders,       c2->ShortHeaders) == 0 &&
     strcmp(c1->ReplyTo,            c2->ReplyTo) == 0 &&
     strcmp(c1->Organization,       c2->Organization) == 0 &&
     strcmp(c1->ExtraHeaders,       c2->ExtraHeaders) == 0 &&
     strcmp(c1->NewIntro,           c2->NewIntro) == 0 &&
     strcmp(c1->Greetings,          c2->Greetings) == 0 &&
     strcmp(c1->Editor,             c2->Editor) == 0 &&
     strcmp(c1->ReplyHello,         c2->ReplyHello) == 0 &&
     strcmp(c1->ReplyIntro,         c2->ReplyIntro) == 0 &&
     strcmp(c1->ReplyBye,           c2->ReplyBye) == 0 &&
     strcmp(c1->AltReplyHello,      c2->AltReplyHello) == 0 &&
     strcmp(c1->AltReplyIntro,      c2->AltReplyIntro) == 0 &&
     strcmp(c1->AltReplyBye,        c2->AltReplyBye) == 0 &&
     strcmp(c1->AltReplyPattern,    c2->AltReplyPattern) == 0 &&
     strcmp(c1->MLReplyHello,       c2->MLReplyHello) == 0 &&
     strcmp(c1->MLReplyIntro,       c2->MLReplyIntro) == 0 &&
     strcmp(c1->MLReplyBye,         c2->MLReplyBye) == 0 &&
     strcmp(c1->ForwardIntro,       c2->ForwardIntro) == 0 &&
     strcmp(c1->ForwardFinish,      c2->ForwardFinish) == 0 &&
     strcmp(c1->TagsFile,           c2->TagsFile) == 0 &&
     strcmp(c1->TagsSeparator,      c2->TagsSeparator) == 0 &&
     strcmp(c1->PGPCmdPath,         c2->PGPCmdPath) == 0 &&
     strcmp(c1->MyPGPID,            c2->MyPGPID) == 0 &&
     strcmp(c1->PGPURL,             c2->PGPURL) == 0 &&
     strcmp(c1->ReMailer,           c2->ReMailer) == 0 &&
     strcmp(c1->RMCommands,         c2->RMCommands) == 0 &&
     strcmp(c1->LogfilePath,        c2->LogfilePath) == 0 &&
     strcmp(c1->DetachDir,          c2->DetachDir) == 0 &&
     strcmp(c1->AttachDir,          c2->AttachDir) == 0 &&
     strcmp(c1->GalleryDir,         c2->GalleryDir) == 0 &&
     strcmp(c1->MyPictureURL,       c2->MyPictureURL) == 0 &&
     strcmp(c1->NewAddrGroup,       c2->NewAddrGroup) == 0 &&
     strcmp(c1->ProxyServer,        c2->ProxyServer) == 0 &&
     strcmp(c1->TempDir,            c2->TempDir) == 0 &&
     strcmp(c1->PackerCommand,      c2->PackerCommand) == 0 &&
     strcmp(c1->XPKPack,            c2->XPKPack) == 0 &&
     strcmp(c1->XPKPackEncrypt,     c2->XPKPackEncrypt) == 0 &&
     strcmp(c1->SupportSite,        c2->SupportSite) == 0 &&
     strcmp(c1->UpdateServer,       c2->UpdateServer) == 0 &&
     strcmp(c1->DefaultReadCharset, c2->DefaultReadCharset) == 0 &&
     strcmp(c1->DefaultWriteCharset,c2->DefaultWriteCharset) == 0 &&
     strcmp(c1->IOCInterface,       c2->IOCInterface) == 0 &&
     strcmp(c1->AppIconText,        c2->AppIconText) == 0 &&
     strcmp(c1->InfoBarText,        c2->InfoBarText) == 0 &&
     strcmp(c1->DefaultMimeViewer,  c2->DefaultMimeViewer) == 0 &&
     strcmp(c1->StyleFGroupUnread,  c2->StyleFGroupUnread) == 0 &&
     strcmp(c1->StyleFGroupRead,    c2->StyleFGroupRead) == 0 &&
     strcmp(c1->StyleFolderUnread,  c2->StyleFolderUnread) == 0 &&
     strcmp(c1->StyleFolderRead,    c2->StyleFolderRead) == 0 &&
     strcmp(c1->StyleFolderNew,     c2->StyleFolderNew) == 0 &&
     strcmp(c1->StyleMailUnread,    c2->StyleMailUnread) == 0 &&
     strcmp(c1->StyleMailRead,      c2->StyleMailRead) == 0 &&
     strcmp(c1->QuoteChar,          c2->QuoteChar) == 0 &&
     strcmp(c1->AltQuoteChar,       c2->AltQuoteChar) == 0 &&
     strcmp(c1->ThemeName,          c2->ThemeName) == 0)
  {
    equal = TRUE;
  }

  RETURN(equal);
  return equal;
}

///
/// CO_Validate
//  Validates a configuration, update GUI etc.
void CO_Validate(struct Config *co, BOOL update)
{
  char *p = strchr(co->EmailAddress, '@');
  BOOL saveAtEnd = FALSE;
  BOOL updateReadWindows = FALSE;
  BOOL updateHeaderMode = FALSE;
  BOOL updateSenderInfo = FALSE;
  int i;

  ENTER();

  if(co->SMTP_Server[0] == '\0')
    strlcpy(co->SMTP_Server, co->P3[0]->Server, sizeof(co->SMTP_Server));
  if(co->SMTP_Port == 0)
    co->SMTP_Port = 25;
  if(co->SMTP_Domain[0] == '\0')
    strlcpy(co->SMTP_Domain, p ? p + 1 : "", sizeof(co->SMTP_Domain));

  for(i = 0; i < MAXP3; i++)
  {
    struct POP3 *pop3 = co->P3[i];

    if(pop3 != NULL)
    {
      if(pop3->Server[0] == '\0')
        strlcpy(pop3->Server, co->SMTP_Server, sizeof(pop3->Server));

      if(pop3->Port == 0)
        pop3->Port = 110;

      if(pop3->User[0] == '\0')
        strlcpy(pop3->User, co->EmailAddress, p ? (unsigned int)(p - co->EmailAddress + 1) : sizeof(pop3->User));

      if(pop3->Account[0] == '\0')
        snprintf(pop3->Account, sizeof(pop3->Account), "%s@%s", pop3->User, pop3->Server);
    }
  }

  // now we check whether our timezone setting is coherent to an
  // eventually set locale setting.
  if(co->TimeZoneCheck)
  {
    if(G->Locale && co->TimeZone != -(G->Locale->loc_GMTOffset))
    {
      int res = MUI_Request(G->App, NULL, 0,
                            tr(MSG_CO_TIMEZONEWARN_TITLE),
                            tr(MSG_CO_TIMEZONEWARN_BT),
                            tr(MSG_CO_TIMEZONEWARN));

      // if the user has clicked on Change, we do
      // change the timezone and save it immediatly
      if(res == 1)
      {
        co->TimeZone = -(G->Locale->loc_GMTOffset);
        saveAtEnd = TRUE;
      }
      else if(res == 2)
      {
        co->TimeZoneCheck = FALSE;
        saveAtEnd = TRUE;
      }
    }
  }
  else if(G->Locale && co->TimeZone == -(G->Locale->loc_GMTOffset))
  {
    // enable the timezone checking again!
    co->TimeZoneCheck = TRUE;
    saveAtEnd = TRUE;
  }

  // we also check the DST (Daylight Saving settings) in case
  // we have a AutoDST tool running.
  if(co->AutoDSTCheck)
  {
    // check if we found an AutoDST tool or not.
    if(G->CO_DST > 0 && co->DaylightSaving != (G->CO_DST == 2))
    {
      int res = MUI_Request(G->App, NULL, 0,
                            tr(MSG_CO_AUTODSTWARN_TITLE),
                            tr(MSG_CO_AUTODSTWARN_BT),
                            tr(MSG_CO_AUTODSTWARN));

      // if the user has clicked on Change, we do
      // change the DST setting and save it immediatly
      if(res == 1)
      {
        co->DaylightSaving = (G->CO_DST == 2);
        saveAtEnd = TRUE;
      }
      else if(res == 2)
      {
        co->AutoDSTCheck = FALSE;
        saveAtEnd = TRUE;
      }
    }
  }
  else if(G->CO_DST > 0 && co->DaylightSaving == (G->CO_DST == 2))
  {
    // enable the autodst checking again!
    co->AutoDSTCheck = TRUE;
    saveAtEnd = TRUE;
  }

  // check if PGP is available or not.
  G->PGPVersion = CO_DetectPGP(co);

  // prepare the temporary directory
  CreateDirectory(co->TempDir);

  // check if the current configuration is already valid at an absolute
  // minimum.
  G->CO_Valid = (*co->SMTP_Server && *co->EmailAddress && *co->RealName);

  // we try to find out the system charset and validate it with the
  // currently configured local charset
  if(co->SysCharsetCheck == TRUE)
  {
    struct codeset *sysCodeset;

    // get the system's default codeset
    if((sysCodeset = CodesetsFindA(NULL, NULL)) != NULL)
    {
      // now we check whether the currently set localCharset matches
      // the system charset or not
      if(co->DefaultReadCharset[0] != '\0' && sysCodeset->name[0] != '\0')
      {
        if(stricmp(co->DefaultReadCharset, sysCodeset->name) != 0)
        {
          int res = MUI_Request(G->App, NULL, 0,
                                tr(MSG_CO_CHARSETWARN_TITLE),
                                tr(MSG_CO_CHARSETWARN_BT),
                                tr(MSG_CO_CHARSETWARN),
                                co->DefaultReadCharset, sysCodeset->name);

          // if the user has clicked on Change, we do
          // change the charset and save it immediatly
          if(res == 1)
          {
            strlcpy(co->DefaultReadCharset, sysCodeset->name, sizeof(co->DefaultReadCharset));
            saveAtEnd = TRUE;
          }
          else if(res == 2)
          {
            co->SysCharsetCheck = FALSE;
            saveAtEnd = TRUE;
          }
        }
      }
      else if(sysCodeset->name[0] != '\0')
      {
        strlcpy(co->DefaultReadCharset, sysCodeset->name, sizeof(co->DefaultReadCharset));
        saveAtEnd = TRUE;
      }
      else
        W(DBF_CONFIG, "checking the system's codeset seem to have failed?!?");
    }
    else
      W(DBF_CONFIG, "CodesetsFindA(NULL) failed!");
  }

  // if the local charset is still empty we set the default
  // charset to 'iso-8859-1' as this one is probably the most common one.
  if(co->DefaultReadCharset[0] == '\0')
  {
    strlcpy(co->DefaultReadCharset, "ISO-8859-1", sizeof(co->DefaultReadCharset));
    saveAtEnd = TRUE;
  }

  if(co->DefaultWriteCharset[0] == '\0')
  {
    strlcpy(co->DefaultWriteCharset, "ISO-8859-1", sizeof(co->DefaultWriteCharset));
    saveAtEnd = TRUE;
  }

  // now we check if the set default read charset is a valid one also supported
  // by codesets.library and if not we warn the user
  if((G->readCharset = CodesetsFind(co->DefaultReadCharset,
                                    CSA_CodesetList,       G->codesetsList,
                                    CSA_FallbackToDefault, FALSE,
                                    TAG_DONE)) == NULL)
  {
    int res = MUI_Request(G->App, NULL, 0,
                          tr(MSG_CO_CHARSETWARN_TITLE),
                          tr(MSG_CO_CHARSETUNKNOWNWARN_BT),
                          tr(MSG_CO_CHARSETUNKNOWNWARN),
                          co->DefaultReadCharset);
    if(res == 1)
    {
      // fallback to the system's default codeset
      if((G->readCharset = CodesetsFindA(NULL, NULL)) != NULL)
      {
        strlcpy(co->DefaultReadCharset, G->readCharset->name, sizeof(co->DefaultReadCharset));
        saveAtEnd = TRUE;
      }
    }
  }

  // now we check if the set default write charset is a valid one also supported
  // by codesets.library and if not we warn the user
  if((G->writeCharset = CodesetsFind(co->DefaultWriteCharset,
                                     CSA_CodesetList,       G->codesetsList,
                                     CSA_FallbackToDefault, FALSE,
                                     TAG_DONE)) == NULL)
  {
    int res = MUI_Request(G->App, NULL, 0,
                          tr(MSG_CO_CHARSETWARN_TITLE),
                          tr(MSG_CO_CHARSETUNKNOWNWARN_BT),
                          tr(MSG_CO_CHARSETUNKNOWNWARN),
                          co->DefaultWriteCharset);
    if(res == 1)
    {
      // fallback to the system's default codeset
      if((G->writeCharset = CodesetsFindA(NULL, NULL)) != NULL)
      {
        strlcpy(co->DefaultWriteCharset, G->writeCharset->name, sizeof(co->DefaultWriteCharset));
        saveAtEnd = TRUE;
      }
    }
  }

  // we also check if AmiSSL was found installed or not. And in case the
  // AmiSSL warning is enabled we notify the user about a not running
  // amissl installation.
  if(co->AmiSSLCheck == TRUE)
  {
    if(AmiSSLMasterBase == NULL || AmiSSLBase == NULL ||
       G->TR_UseableTLS == FALSE)
    {
      int res = MUI_Request(G->App, NULL, 0,
                            tr(MSG_CO_AMISSLWARN_TITLE),
                            tr(MSG_CO_AMISSLWARN_BT),
                            tr(MSG_CO_AMISSLWARN),
                            AMISSLMASTER_MIN_VERSION, 5);

      // if the user has clicked on "Ignore always", we do
      // change the AmiSSLCheck variables and save the config
      // immediatly
      if(res == 1)
      {
        exit(RETURN_ERROR);
      }
      else if(res == 2)
      {
        co->AmiSSLCheck = FALSE;
        saveAtEnd = TRUE;
      }
    }
  }
  else
  {
    // we reenable the AmiSSLCheck as soon as we found
    // the library to be working fine.
    if(AmiSSLMasterBase != NULL && AmiSSLBase != NULL &&
       G->TR_UseableTLS == TRUE)
    {
      co->AmiSSLCheck = TRUE;
      saveAtEnd = TRUE;
    }
  }

  if(co->SpamFilterEnabled == TRUE)
  {
    // limit the spam probability threshold to sensible values
    if(co->SpamProbabilityThreshold < 75)
    {
      co->SpamProbabilityThreshold = 75;
      saveAtEnd = TRUE;
    }
    else if(co->SpamProbabilityThreshold > 99)
    {
      co->SpamProbabilityThreshold = 99;
      saveAtEnd = TRUE;
    }
  }

  if(co->StatusChangeDelay < 1000)
  {
    // a delay less than one second is not possible
    co->StatusChangeDelay = 1000;
    saveAtEnd = TRUE;
  }
  else if(co->StatusChangeDelay > 10000)
  {
    // a delay longer than ten seconds is not possible, either
    co->StatusChangeDelay = 10000;
    saveAtEnd = TRUE;
  }

  if(update == TRUE && G->CO != NULL)
  {
    switch(G->CO->VisiblePage)
    {
      case cp_FirstSteps:
      {
        setstring(G->CO->GUI.ST_POPHOST0, co->P3[0]->Server);
      }
      break;

      case cp_TCPIP:
      {
        setstring(G->CO->GUI.ST_SMTPHOST, co->SMTP_Server);
        set(G->CO->GUI.ST_SMTPPORT, MUIA_String_Integer, co->SMTP_Port);
        setstring(G->CO->GUI.ST_DOMAIN, co->SMTP_Domain);
        setstring(G->CO->GUI.ST_SMTPAUTHUSER, co->SMTP_AUTH_User);
        setstring(G->CO->GUI.ST_SMTPAUTHPASS, co->SMTP_AUTH_Pass);
        DoMethod(G->CO->GUI.LV_POP3, MUIM_NList_Redraw, MUIV_NList_Redraw_All);
      }
      break;

      default:
      {
        // nothing
      }
      break;
    }

    if(G->CO->Visited[cp_NewMail] == TRUE || G->CO->UpdateAll == TRUE)
    {
      // requeue the timerequest for the CheckMailDelay
      RestartTimer(TIMER_CHECKMAIL, co->CheckMailDelay*60, 0);
    }

    if(G->CO->Visited[cp_Spam] == TRUE || G->CO->UpdateAll == TRUE)
    {
      // if we enabled or disable the spam filter then we need to update
      // the enable/disable status of some toolbar items of the main window
      MA_ChangeSelected(TRUE);

      // now we also have to update the Spam controls of our various
      // window toolbars
      DoMethod(G->MA->GUI.TO_TOOLBAR, MUIM_MainWindowToolbar_UpdateSpamControls);

      // open read windows need to be updated, too
      updateReadWindows = TRUE;
    }

    if(G->CO->Visited[cp_Read] == TRUE || G->CO->UpdateAll == TRUE)
    {
      // open read windows need to be updated, too
      updateHeaderMode = TRUE;
      updateSenderInfo = TRUE;
    }

    if(G->CO->Visited[cp_Write] == TRUE || G->CO->UpdateAll == TRUE)
    {
      // requeue the timerequest for the AutoSave interval
      RestartTimer(TIMER_AUTOSAVE, co->AutoSave, 0);
    }

    if(G->CO->Visited[cp_Lists] == TRUE || G->CO->UpdateAll == TRUE)
    {
      // First we set the PG_MAILLIST and NL_FOLDER Quiet
      set(G->MA->GUI.PG_MAILLIST,MUIA_NList_Quiet,     TRUE);
      set(G->MA->GUI.NL_FOLDERS, MUIA_NListtree_Quiet, TRUE);

      // Modify the ContextMenu flags
      set(G->MA->GUI.PG_MAILLIST,MUIA_ContextMenu, C->MessageCntMenu ? MUIV_NList_ContextMenu_Always : MUIV_NList_ContextMenu_Never);
      set(G->MA->GUI.NL_FOLDERS, MUIA_ContextMenu, C->FolderCntMenu ? MUIV_NList_ContextMenu_Always : MUIV_NList_ContextMenu_Never);

      SaveLayout(FALSE);
      MA_MakeFOFormat(G->MA->GUI.NL_FOLDERS);
      DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_MainMailListGroup_MakeFormat);
      LoadLayout();

      // Now we give the control back to the NLists
      set(G->MA->GUI.PG_MAILLIST,MUIA_NList_Quiet,     FALSE);
      set(G->MA->GUI.NL_FOLDERS, MUIA_NListtree_Quiet, FALSE);
    }

    if(G->CO->Visited[cp_AddressBook] == TRUE || G->CO->UpdateAll == TRUE)
    {
      AB_MakeABFormat(G->AB->GUI.LV_ADDRESSES);
    }

    if(G->CO->Visited[cp_LookFeel] == TRUE || G->CO->UpdateAll == TRUE)
    {
      // First we set the PG_MAILLIST and NL_FOLDER Quiet
      set(G->MA->GUI.PG_MAILLIST,MUIA_NList_Quiet,     TRUE);
      set(G->MA->GUI.NL_FOLDERS, MUIA_NListtree_Quiet, TRUE);

      // Now we reorder the Maingroup accordingly to the InfoBar setting
      MA_SortWindow();

      // Now we update the InfoBar because the text could have been changed
      DoMethod(G->MA->GUI.IB_INFOBAR, MUIM_InfoBar_SetFolder, FO_GetCurrentFolder());

      // we signal the mainwindow that it may check whether to include the
      // quicksearchbar or not
      MA_SetupQuickSearchBar();

      // we signal the mainwindow that it may check whether to include the
      // embedded read pane part or not
      MA_SetupEmbeddedReadPane();

      // Make sure to save the GUI layout before continuing
      SaveLayout(FALSE);

      // Now we give the control back to the NLists
      set(G->MA->GUI.PG_MAILLIST,MUIA_NList_Quiet,     FALSE);
      set(G->MA->GUI.NL_FOLDERS, MUIA_NListtree_Quiet, FALSE);

      // and to not let the embedded read pane be empty when it is newly created
      // we have to make sure the actual selected mail is loaded
      if(C->EmbeddedReadPane)
        MA_ChangeSelected(TRUE);
    }

    if(G->CO->Visited[cp_Mixed] == TRUE || G->CO->UpdateAll == TRUE)
    {
      // in case the DockyIcon should be enabled we have reregister YAM
      // to application library for the DockyIcon to reappear
      #if defined(__amigaos4__)
      if(G->applicationID && C->DockyIcon == TRUE)
      {
        struct ApplicationIconInfo aii;

        GetApplicationAttrs(G->applicationID,
                            APPATTR_IconType, (uint32)&aii,
                            TAG_DONE);

        // if the iconType is currently none,
        // we have to unregister and register YAM again to
        // application.library
        if(aii.iconType == APPICONT_None)
        {
          UnregisterApplication(G->applicationID, TAG_DONE);

          aii.iconType = APPICONT_CustomIcon;
          aii.info.customIcon = G->HideIcon;

          // register YAM to application.library
          G->applicationID = RegisterApplication("YAM",
                                                 REGAPP_URLIdentifier, "yam.ch",
                                                 REGAPP_AppIconInfo,   (uint32)&aii,
                                                 REGAPP_Hidden,        xget(G->App, MUIA_Application_Iconified),
                                                 TAG_DONE);

          D(DBF_STARTUP, "reregistered YAM to application as appId: %ld", G->applicationID);
        }
      }
      #endif

      // setup the appIcon positions and display all statistics
      // accordingly.
      DisplayStatistics((struct Folder *)-1, TRUE);

      // make sure we remove an eventually existing DockyIcon
      #if defined(__amigaos4__)
      if(G->applicationID && C->DockyIcon == FALSE)
      {
        struct ApplicationIconInfo aii;

        aii.iconType = APPICONT_None;
        aii.info.customIcon = NULL;

        SetApplicationAttrs(G->applicationID,
                            APPATTR_IconType, (uint32)&aii,
                            TAG_DONE);
      }
      #endif
    }

    if(G->CO->Visited[cp_Update] == TRUE || G->CO->UpdateAll == TRUE)
    {
      // make sure we reinit the update check timer
      InitUpdateCheck(FALSE);
    }

    // make sure the dynamic menus of the main window
    // is properly refreshed.
    MA_SetupDynamicMenus();
  }

  // if some items have modified the config we do save it again.
  if(saveAtEnd == TRUE)
    CO_SaveConfig(co, G->CO_PrefsFile);

  // finally update possibly open read windows
  if(updateReadWindows == TRUE || updateHeaderMode == TRUE || updateSenderInfo == TRUE)
  {
    struct Node *curNode;

    IterateList(&G->readMailDataList, curNode)
    {
      struct ReadMailData *rmData = (struct ReadMailData *)curNode;

      if(rmData->mail != NULL)
      {
        // we use PushMethod for the case the read window modifies we list we are currently walking through
        if(rmData->readMailGroup != NULL && (updateHeaderMode == TRUE || updateSenderInfo == TRUE))
        {
          // forward the modified information directly to the read mail group
          if(updateHeaderMode == TRUE)
            DoMethod(G->App, MUIM_Application_PushMethod, rmData->readMailGroup, 2, MUIM_ReadMailGroup_ChangeHeaderMode, co->ShowHeader);

          if(updateSenderInfo == TRUE)
            DoMethod(G->App, MUIM_Application_PushMethod, rmData->readMailGroup, 2, MUIM_ReadMailGroup_ChangeSenderInfoMode, co->ShowSenderInfo);
        }
        else if(rmData->readWindow != NULL && updateReadWindows == TRUE)
        {
          // forward the modifed information to the window, because a read mail group has no toolbar
          if(updateReadWindows == TRUE)
            DoMethod(G->App, MUIM_Application_PushMethod, rmData->readWindow, 2, MUIM_ReadWindow_ReadMail, rmData->mail);
        }
      }
    }
  }

  LEAVE();
}

///
/// CO_EditSignatFunc
//  Edits the signature file
HOOKPROTONHNO(CO_EditSignatFunc, void, int *arg)
{
  int sig = GetMUICycle(G->CO->GUI.CY_SIGNAT);
  BOOL editSig = (BOOL)*arg;
  BOOL refresh;
  char buffer[SIZE_COMMAND+SIZE_PATHFILE];
  Object *ed = G->CO->GUI.TE_SIGEDIT;

  ENTER();

  if(xget(ed, MUIA_TextEditor_HasChanged) == TRUE)
  {
    if(MUI_Request(G->App, G->CO->GUI.WI, 0, NULL, tr(MSG_YesNoReq), tr(MSG_CO_ASK_SAVE_SIGNATURE)) > 0)
      // save the modified signature only if the user told us to do so
      EditorToFile(ed, CreateFilename(SigNames[G->CO->LastSig]));
  }

  if(editSig == TRUE)
  {
    // if the signature should be modified with an external editor then
    // we need to check if there is an editor defined
    if(CE->Editor[0] != '\0')
    {
      snprintf(buffer, sizeof(buffer), "%s \"%s\"", CE->Editor, GetRealPath(CreateFilename(SigNames[sig])));
      ExecuteCommand(buffer, FALSE, OUT_NIL);
      refresh = TRUE;
    }
    else
      // no external editor defined, so we don't need to refresh the
      // signature in the internal editor
      refresh = FALSE;
  }
  else
    // just display the new signature in the internal editor
    refresh = TRUE;

  if(refresh == TRUE)
  {
    // refresh the signature in the internal editor
    if(FileToEditor(CreateFilename(SigNames[sig]), ed, FALSE, TRUE, TRUE) == FALSE)
      DoMethod(ed, MUIM_TextEditor_ClearText);

    G->CO->LastSig = sig;
  }

  LEAVE();
}
MakeHook(CO_EditSignatHook,CO_EditSignatFunc);

///
/// CO_SwitchSignatFunc
//  Enables/Disables some object upon the status of the signature checkbox
HOOKPROTONHNO(CO_SwitchSignatFunc, void, int *arg)
{
  BOOL enable = *arg;

  set(G->CO->GUI.CY_SIGNAT,   MUIA_Disabled, enable);
  set(G->CO->GUI.BT_SIGEDIT,  MUIA_Disabled, enable);
  set(G->CO->GUI.TE_SIGEDIT,  MUIA_Disabled, enable);
  set(G->CO->GUI.BT_INSTAG,   MUIA_Disabled, enable);
  set(G->CO->GUI.BT_INSENV,   MUIA_Disabled, enable);
}
MakeHook(CO_SwitchSignatHook, CO_SwitchSignatFunc);

///
/// CO_OpenConfig
//  Opens a different configuration file
HOOKPROTONHNONP(CO_OpenConfig, void)
{
  struct FileReqCache *frc;

  ENTER();

  if((frc = ReqFile(ASL_CONFIG,G->CO->GUI.WI, tr(MSG_CO_Open), REQF_NONE, G->MA_MailDir, "")) != NULL)
  {
    char cname[SIZE_PATHFILE];

    AddPath(cname, frc->drawer, frc->file, sizeof(cname));
    if(CO_LoadConfig(CE, cname, NULL) == TRUE)
      CO_NewPrefsFile(cname);

    CO_SetConfig();
    G->CO->UpdateAll = TRUE;
  }

  LEAVE();
}
MakeStaticHook(CO_OpenConfigHook, CO_OpenConfig);

///
/// CO_SaveConfigAs
//  Saves configuration to a file using an alternative name
HOOKPROTONHNONP(CO_SaveConfigAs, void)
{
  struct FileReqCache *frc;

  ENTER();

  if((frc = ReqFile(ASL_CONFIG, G->CO->GUI.WI, tr(MSG_CO_SaveAs), REQF_SAVEMODE, G->MA_MailDir, "")))
  {
    char cname[SIZE_PATHFILE];

    AddPath(cname, frc->drawer, frc->file, sizeof(cname));

    if(FileExists(cname) == FALSE ||
       MUI_Request(G->App, G->CO->GUI.WI, 0, tr(MSG_MA_ConfirmReq), tr(MSG_YesNoReq), tr(MSG_FILE_OVERWRITE), frc->file) != 0)
    {
      // the config is really saved
      CO_GetConfig(TRUE);
      CO_Validate(CE, TRUE);
      CO_NewPrefsFile(cname);
      CO_SaveConfig(CE, cname);
    }
  }

  LEAVE();
}
MakeStaticHook(CO_SaveConfigAsHook, CO_SaveConfigAs);

///
/// CO_Restore
//  Makes all changes undone
HOOKPROTONHNONP(CO_Restore, void)
{
  ENTER();

  CO_ClearConfig(CE);
  CopyConfigData(CE, C);
  CO_SetConfig();

  LEAVE();
}
MakeStaticHook(CO_RestoreHook,CO_Restore);

///
/// CO_LastSaved
//  Reloads configuration from file
HOOKPROTONHNONP(CO_LastSaved, void)
{
  ENTER();

  CO_LoadConfig(CE, G->CO_PrefsFile, NULL);
  CO_SetConfig();
  G->CO->UpdateAll = TRUE;

  LEAVE();
}
MakeStaticHook(CO_LastSavedHook,CO_LastSaved);

///
/// CO_ResetToDefaultFunc
//  Resets configuration (or a part of it)
HOOKPROTONHNO(CO_ResetToDefaultFunc, void, int *arg)
{
  ENTER();

  if(*arg)
  {
    CO_SetDefaults(CE, cp_AllPages);
    G->CO->UpdateAll = TRUE;
  }
  else
    CO_SetDefaults(CE, G->CO->VisiblePage);

  CO_SetConfig();

  LEAVE();
}
MakeStaticHook(CO_ResetToDefaultHook,CO_ResetToDefaultFunc);

///
/// CO_ChangePageFunc
//  Selects a different section of the configuration
HOOKPROTONHNO(CO_ChangePageFunc, void, int *arg)
{
  enum ConfigPage page = (enum ConfigPage)(*arg);

  ENTER();

  if(page >= cp_FirstSteps && page < cp_Max)
  {
    // the config is not saved yet, but some parts (like the signature) need to be saved nevertheless
    CO_GetConfig(FALSE);

    G->CO->VisiblePage = page;
    G->CO->Visited[page] = TRUE;

    CO_SetConfig();

    set(G->CO->GUI.GR_PAGE, MUIA_Group_ActivePage, page);
  }

  LEAVE();
}
MakeStaticHook(CO_ChangePageHook,CO_ChangePageFunc);
///
/// CO_CloseFunc
//  Closes configuration window
HOOKPROTONHNO(CO_CloseFunc, void, int *arg)
{
  ENTER();

  // check if we should copy our edited configuration
  // to the real one or if we should just free/drop it
  if(arg != NULL && arg[0] >= 1)
  {
    // get the current state of the configuration
    CO_GetConfig(arg[0] == 2);

    // before we copy over the configuration, we
    // check if it was changed at all
    if(CompareConfigData(C, CE) == FALSE)
    {
      struct Config *tmpC;

      D(DBF_CONFIG, "configuration found to be different");

      // just swap the pointers instead of clearing and copying the whole stuff
      tmpC = C;
      C = CE;
      CE = tmpC;
      // the up to now "current" configuration will be freed below
    }
    else
      D(DBF_CONFIG, "config wasn't altered, skipped copy operations.");

    // validate that C has valid values
    CO_Validate(C, TRUE);

    // we save the configuration if the user
    // has pressed on 'Save' only.
    if(arg[0] == 2)
    {
      // save the signature if it has been modified
      if(xget(G->CO->GUI.TE_SIGEDIT, MUIA_TextEditor_HasChanged) == TRUE)
        EditorToFile(G->CO->GUI.TE_SIGEDIT, CreateFilename(SigNames[G->CO->LastSig]));

      CO_SaveConfig(C, G->CO_PrefsFile);
    }
  }

  // then we free our temporary config structure
  CO_ClearConfig(CE);
  free(CE);
  CE = NULL;

  // Dipose&Close the config window stuff
  DisposeModulePush(&G->CO);

  LEAVE();
}
MakeStaticHook(CO_CloseHook, CO_CloseFunc);

///
/// CO_OpenFunc
//  Opens configuration window
HOOKPROTONHNONP(CO_OpenFunc, void)
{
  BusyText(tr(MSG_BUSY_OPENINGCONFIG), "");

  // check if there isn't already a configuration
  // open
  if(G->CO == NULL)
  {
    // first duplicate the configuration
    if((CE = malloc(sizeof(struct Config))) != NULL)
    {
      if(CopyConfigData(CE, C) == TRUE)
      {
        // then create the config window
        if((G->CO = CO_New()) != NULL)
        {
          CO_SetConfig();
          CO_NewPrefsFile(G->CO_PrefsFile);
        }
      }

      if(G->CO == NULL)
      {
        // free the duplicated configuration on failure
        CO_ClearConfig(CE);
        free(CE);
        CE = NULL;
      }
    }
  }

  // only try to open the window if everything above succeeded
  if(G->CO != NULL && CE != NULL)
  {
    // make sure the configuration window is open
    if(SafeOpenWindow(G->CO->GUI.WI) == FALSE)
      CallHookPkt(&CO_CloseHook, 0, NULL);
  }
  else
  {
    // inform the user by chiming the bells about the failure
    DisplayBeep(NULL);
  }

  BusyEnd();
}
MakeHook(CO_OpenHook,CO_OpenFunc);
///

/*** GUI ***/
/// CO_New
//  Creates configuration window
static struct CO_ClassData *CO_New(void)
{
  struct CO_ClassData *data;

  // menu item enums
  enum { CMEN_OPEN = 1201, CMEN_SAVEAS, CMEN_DEF, CMEN_DEFALL, CMEN_LAST, CMEN_REST };

  ENTER();

  if((data = calloc(1, sizeof(struct CO_ClassData))) != NULL)
  {
    static struct PageList page[cp_Max], *pages[cp_Max + 1];
    int i;

    for(i = cp_FirstSteps; i < cp_Max; i++)
    {
      page[i].Offset = i;
      pages[i] = &page[i];
    }
    pages[cp_Max] = NULL;

    // put some labels on our configpagelist objects
    page[cp_FirstSteps  ].PageLabel = MSG_CO_CrdFirstSteps;
    page[cp_TCPIP       ].PageLabel = MSG_CO_CrdTCPIP;
    page[cp_NewMail     ].PageLabel = MSG_CO_CrdNewMail;
    page[cp_Filters     ].PageLabel = MSG_CO_CrdFilters;
    page[cp_Spam        ].PageLabel = MSG_CO_CRDSPAMFILTER;
    page[cp_Read        ].PageLabel = MSG_CO_CrdRead;
    page[cp_Write       ].PageLabel = MSG_CO_CrdWrite;
    page[cp_ReplyForward].PageLabel = MSG_CO_GR_REPLYFORWARD;
    page[cp_Signature   ].PageLabel = MSG_CO_CrdSignature;
    page[cp_Lists       ].PageLabel = MSG_CO_CrdLists;
    page[cp_Security    ].PageLabel = MSG_CO_CrdSecurity;
    page[cp_StartupQuit ].PageLabel = MSG_CO_GR_STARTUPQUIT;
    page[cp_MIME        ].PageLabel = MSG_CO_CrdMIME;
    page[cp_AddressBook ].PageLabel = MSG_CO_CrdABook;
    page[cp_Scripts     ].PageLabel = MSG_CO_GR_SCRIPTS;
    page[cp_Mixed       ].PageLabel = MSG_CO_CrdMixed;
    page[cp_LookFeel    ].PageLabel = MSG_CO_CRDLOOKFEEL;
    page[cp_Update      ].PageLabel = MSG_CO_CrdUpdate;

    data->GUI.WI = WindowObject,
       MUIA_Window_Title, tr(MSG_MA_MConfig),
       MUIA_HelpNode,"CO_W",
       MUIA_Window_Menustrip, MenustripObject,
          MUIA_Family_Child, MenuObject, MUIA_Menu_Title, tr(MSG_MA_Project),
             MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,tr(MSG_CO_Open), MUIA_Menuitem_Shortcut,"O", MUIA_UserData,CMEN_OPEN, End,
             MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,tr(MSG_CO_SaveAs), MUIA_Menuitem_Shortcut,"A", MUIA_UserData,CMEN_SAVEAS, End,
          End,
          MUIA_Family_Child, MenuObject, MUIA_Menu_Title, tr(MSG_CO_Edit),
             MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,tr(MSG_CO_ResetDefaults), MUIA_Menuitem_Shortcut,"D", MUIA_UserData,CMEN_DEF, End,
             MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,tr(MSG_CO_ResetAll), MUIA_Menuitem_Shortcut,"E", MUIA_UserData,CMEN_DEFALL, End,
             MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,tr(MSG_CO_LastSaved), MUIA_Menuitem_Shortcut,"L", MUIA_UserData,CMEN_LAST, End,
             MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,tr(MSG_CO_Restore), MUIA_Menuitem_Shortcut,"R", MUIA_UserData,CMEN_REST, End,
          End,
       End,
       MUIA_Window_ID, MAKE_ID('C','O','N','F'),
       WindowContents, VGroup,
          Child, HGroup,
             Child, data->GUI.NLV_PAGE = NListviewObject,
                MUIA_CycleChain,  TRUE,
                MUIA_NListview_NList, data->GUI.LV_PAGE = ConfigPageListObject,
                   MUIA_NList_Format,         "",
                   MUIA_NList_Title,          FALSE,
                   MUIA_NList_AdjustWidth,    TRUE,
                   MUIA_NList_AutoVisible,    TRUE,
                   MUIA_NList_MinLineHeight,  16,
                   MUIA_NList_SourceArray,    pages,
                   MUIA_NList_Active,         MUIV_NList_Active_Top,
                End,
             End,
             Child, data->GUI.GR_PAGE = PageGroup,
                NoFrame,
                MUIA_Group_ActivePage, 0,
                Child, CO_PageFirstSteps(data),
                Child, CO_PageTCPIP(data),
                Child, CO_PageNewMail(data),
                Child, CO_PageFilters(data),
                Child, CO_PageSpam(data),
                Child, CO_PageRead(data),
                Child, CO_PageWrite(data),
                Child, CO_PageReplyForward(data),
                Child, CO_PageSignature(data),
                Child, CO_PageLists(data),
                Child, CO_PageSecurity(data),
                Child, CO_PageStartupQuit(data),
                Child, CO_PageMIME(data),
                Child, CO_PageAddressBook(data),
                Child, CO_PageScripts(data),
                Child, CO_PageMixed(data),
                Child, CO_PageLookFeel(data),
                Child, CO_PageUpdate(data),
             End,
          End,

          Child, RectangleObject,
             MUIA_Rectangle_HBar, TRUE,
             MUIA_FixHeight,      4,
          End,

          Child, HGroup,
             MUIA_Group_SameWidth, TRUE,
             Child, data->GUI.BT_SAVE   = MakeButton(tr(MSG_CO_Save)),
             Child, data->GUI.BT_USE    = MakeButton(tr(MSG_CO_Use)),
             Child, data->GUI.BT_CANCEL = MakeButton(tr(MSG_CO_Cancel)),
          End,
       End,
    End;
    if(data->GUI.WI != NULL)
    {
      DoMethod(G->App, OM_ADDMEMBER, data->GUI.WI);
      set(data->GUI.WI, MUIA_Window_DefaultObject, data->GUI.NLV_PAGE);
      SetHelp(data->GUI.BT_SAVE,   MSG_HELP_CO_BT_SAVE);
      SetHelp(data->GUI.BT_USE,    MSG_HELP_CO_BT_USE);
      SetHelp(data->GUI.BT_CANCEL, MSG_HELP_CO_BT_CANCEL);
      DoMethod(data->GUI.WI          ,MUIM_Notify,MUIA_Window_MenuAction  ,CMEN_OPEN     ,MUIV_Notify_Application,2,MUIM_CallHook,&CO_OpenConfigHook);
      DoMethod(data->GUI.WI          ,MUIM_Notify,MUIA_Window_MenuAction  ,CMEN_SAVEAS   ,MUIV_Notify_Application,2,MUIM_CallHook,&CO_SaveConfigAsHook);
      DoMethod(data->GUI.WI          ,MUIM_Notify,MUIA_Window_MenuAction  ,CMEN_DEF      ,MUIV_Notify_Application,3,MUIM_CallHook,&CO_ResetToDefaultHook,FALSE);
      DoMethod(data->GUI.WI          ,MUIM_Notify,MUIA_Window_MenuAction  ,CMEN_DEFALL   ,MUIV_Notify_Application,3,MUIM_CallHook,&CO_ResetToDefaultHook,TRUE);
      DoMethod(data->GUI.WI          ,MUIM_Notify,MUIA_Window_MenuAction  ,CMEN_LAST     ,MUIV_Notify_Application,2,MUIM_CallHook,&CO_LastSavedHook);
      DoMethod(data->GUI.WI          ,MUIM_Notify,MUIA_Window_MenuAction  ,CMEN_REST     ,MUIV_Notify_Application,2,MUIM_CallHook,&CO_RestoreHook);
      DoMethod(data->GUI.LV_PAGE     ,MUIM_Notify,MUIA_NList_Active       ,MUIV_EveryTime,MUIV_Notify_Application,3,MUIM_CallHook,&CO_ChangePageHook,MUIV_TriggerValue);
      DoMethod(data->GUI.BT_SAVE     ,MUIM_Notify,MUIA_Pressed            ,FALSE         ,MUIV_Notify_Application,3,MUIM_CallHook,&CO_CloseHook,2);
      DoMethod(data->GUI.BT_USE      ,MUIM_Notify,MUIA_Pressed            ,FALSE         ,MUIV_Notify_Application,3,MUIM_CallHook,&CO_CloseHook,1);
      DoMethod(data->GUI.BT_CANCEL   ,MUIM_Notify,MUIA_Pressed            ,FALSE         ,MUIV_Notify_Application,3,MUIM_CallHook,&CO_CloseHook,0);
      DoMethod(data->GUI.WI          ,MUIM_Notify,MUIA_Window_CloseRequest,TRUE          ,MUIV_Notify_Application,3,MUIM_CallHook,&CO_CloseHook,0);
    }
    else
    {
      free(data);
      data = NULL;
    }
  }

  RETURN(data);
  return data;
}
////

