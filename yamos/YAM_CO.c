/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2006 by YAM Open Source Team

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

#include <stdlib.h>
#include <string.h>

#include <clib/alib_protos.h>
#include <diskfont/diskfonttag.h>
#include <libraries/asl.h>
#include <libraries/iffparse.h>
#include <libraries/locale.h>
#include <mui/NList_mcc.h>
#include <mui/NListtree_mcc.h>
#include <mui/TextEditor_mcc.h>
#include <proto/diskfont.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/muimaster.h>

#if defined(__amigaos4__)
#include <proto/application.h>
#endif

#include "extra.h"
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
#include "YAM_locale.h"
#include "YAM_main.h"
#include "YAM_utilities.h"
#include "classes/Classes.h"

#include "Debug.h"

struct Config *C = NULL;
struct Config *CE = NULL;

/* local protos */
static void CO_NewPrefsFile(char*);
static int CO_DetectPGP(struct Config*);
static APTR CO_BuildPage(struct CO_ClassData*, int);
static struct CO_ClassData *CO_New(void);
static void CopyConfigData(struct Config*, struct Config*);

/***************************************************************************
 Module: Configuration
***************************************************************************/

/// CO_NewPrefsFile
//  Sets the name of the configuration file
static void CO_NewPrefsFile(char *fname)
{
   static char wtitle[SIZE_SMALL+SIZE_PATHFILE];
   strcpy(G->CO_PrefsFile, fname);
   sprintf(wtitle, "%s (%s)", GetStr(MSG_MA_MConfig), fname);
   set(G->CO->GUI.WI, MUIA_Window_Title, wtitle);
}
///

/**** Filters ****/
/// AddNewFilterToList
//  Adds a new entry to the global filter list
HOOKPROTONHNONP(AddNewFilterToList, void)
{
  struct FilterNode *filterNode;

  if((filterNode = CreateNewFilter()))
  {
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
MakeHook(AddNewFilterToListHook, AddNewFilterToList);

///
/// RemoveActiveFilter
//  Deletes the active filter entry from the filter list
HOOKPROTONHNONP(RemoveActiveFilter, void)
{
  struct FilterNode *filterNode = NULL;

  // get the active filterNode
  DoMethod(G->CO->GUI.LV_RULES, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &filterNode);

  // if we got an active entry lets remove it from the GUI List
  // and also from our own global filterList
  if(filterNode)
  {
    DoMethod(G->CO->GUI.LV_RULES, MUIM_NList_Remove, MUIV_NList_Remove_Active);

    Remove((struct Node *)filterNode);
    free(filterNode);
  }
}
MakeHook(RemoveActiveFilterHook, RemoveActiveFilter);

///
/// AddNewRuleToList
//  Adds a new entry to the current filter's rule list
HOOKPROTONHNONP(AddNewRuleToList, void)
{
  struct FilterNode *filter = NULL;
  struct CO_GUIData *gui = &G->CO->GUI;

  // get the active filterNode
  DoMethod(gui->LV_RULES, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &filter);

  if(filter)
  {
    struct RuleNode *rule;

    if((rule = CreateNewRule(filter)))
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
      DoMethod(gui->GR_RGROUP, MUIM_Group_InitChange); // required for a proper refresh
      DoMethod(gui->GR_SGROUP, MUIM_Group_InitChange);
      DoMethod(gui->GR_SGROUP, OM_ADDMEMBER, newSearchGroup);
      //DoMethod(gui->GR_SGROUP, MUIM_Group_MoveMember, newSearchGroup, -1);
      DoMethod(gui->GR_SGROUP, MUIM_Group_ExitChange);
      DoMethod(gui->GR_RGROUP, MUIM_Group_ExitChange); // required for a proper refresh

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
      if(rule->search)
      {
        FreeSearchPatternList(rule->search);
        free(rule->search);
        rule->search = NULL;
      }

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
  BOOL isremote = filter ? filter->remote : FALSE;
  LONG pos = MUIV_NList_GetPos_Start;
  int numRules = 0;
  struct List *childList;

  set(gui->ST_RNAME,             MUIA_Disabled, !filter);
  set(gui->CH_REMOTE,            MUIA_Disabled, !filter);
  set(gui->CH_APPLYNEW,          MUIA_Disabled, !filter || isremote);
  set(gui->CH_APPLYREQ,          MUIA_Disabled, !filter || isremote);
  set(gui->CH_APPLYSENT,         MUIA_Disabled, !filter || isremote);
  set(gui->CH_ABOUNCE,           MUIA_Disabled, !filter || isremote);
  set(gui->CH_AFORWARD,          MUIA_Disabled, !filter || isremote);
  set(gui->CH_ARESPONSE,         MUIA_Disabled, !filter || isremote);
  set(gui->CH_AEXECUTE,          MUIA_Disabled, !filter);
  set(gui->CH_APLAY,             MUIA_Disabled, !filter);
  set(gui->CH_AMOVE,             MUIA_Disabled, !filter || isremote);
  set(gui->CH_ADELETE,           MUIA_Disabled, !filter);
  set(gui->CH_ASKIP,             MUIA_Disabled, !filter || !isremote);
  set(gui->ST_ABOUNCE,           MUIA_Disabled, !filter || isremote || !xget(gui->CH_ABOUNCE,    MUIA_Selected));
  set(gui->ST_AFORWARD,          MUIA_Disabled, !filter || isremote || !xget(gui->CH_AFORWARD,   MUIA_Selected));
  set(gui->PO_ARESPONSE,         MUIA_Disabled, !filter || isremote || !xget(gui->CH_ARESPONSE,  MUIA_Selected));
  set(gui->PO_AEXECUTE,          MUIA_Disabled, !filter || !xget(gui->CH_AEXECUTE, MUIA_Selected));
  set(gui->PO_APLAY,             MUIA_Disabled, !filter || !xget(gui->CH_APLAY, MUIA_Selected));
  set(gui->BT_APLAY,             MUIA_Disabled, !filter || !xget(gui->CH_APLAY, MUIA_Selected));
  set(gui->PO_MOVETO,            MUIA_Disabled, !filter || !xget(gui->CH_AMOVE, MUIA_Selected));
  set(gui->BT_RDEL,              MUIA_Disabled, !filter);

  // lets make sure we ghost the filter up/down buttons if necessary
  if(filter)
    DoMethod(gui->LV_RULES, MUIM_NList_GetPos, filter, &pos);

  set(gui->BT_FILTERUP,   MUIA_Disabled, !filter || pos == 0);
  set(gui->BT_FILTERDOWN, MUIA_Disabled, !filter || pos+1 == (LONG)xget(gui->LV_RULES, MUIA_NList_Entries));

  // we have to find out how many rules the filter has
  if((childList = (struct List *)xget(gui->GR_SGROUP, MUIA_Group_ChildList)))
	{
    Object *cstate = (Object *)childList->lh_Head;
		Object *child;

		while((child = NextObject(&cstate)))
    {
      set(child, MUIA_Disabled, !filter);
      numRules++;
    }
  }

  set(gui->BT_MORE, MUIA_Disabled, !filter);
  set(gui->BT_LESS, MUIA_Disabled, !filter || numRules <= 1);
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
    DoMethod(gui->GR_RGROUP, MUIM_Group_InitChange); // required for proper refresh
    DoMethod(gui->GR_SGROUP, MUIM_Group_InitChange);
  	if((childList = (struct List *)xget(gui->GR_SGROUP, MUIA_Group_ChildList)))
	  {
      int i;
		  struct MinNode *curNode;
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
      for(i=0, curNode = filter->ruleList.mlh_Head; curNode->mln_Succ; curNode = curNode->mln_Succ, i++)
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
      }
	  }
    DoMethod(gui->GR_SGROUP, MUIM_Group_ExitChange);
    DoMethod(gui->GR_RGROUP, MUIM_Group_ExitChange); // required for proper refresh
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
  if(filter)
  {
    struct List *childList;
    int rm = GetMUICheck(gui->CH_REMOTE);

    GetMUIString(filter->name, gui->ST_RNAME);
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
    GetMUIString(filter->bounceTo,   gui->ST_ABOUNCE);
    GetMUIString(filter->forwardTo,  gui->ST_AFORWARD);
    GetMUIString(filter->replyFile,  gui->ST_ARESPONSE);
    GetMUIString(filter->executeCmd, gui->ST_AEXECUTE);
    GetMUIString(filter->playSound,  gui->ST_APLAY);

    strncpy(filter->moveTo, (char *)xget(gui->TX_MOVETO, MUIA_Text_Contents), SIZE_NAME);
    filter->moveTo[SIZE_NAME-1] = '\0';

    // make sure to update all rule settings
  	if((childList = (struct List *)xget(gui->GR_SGROUP, MUIA_Group_ChildList)))
	  {
      Object *cstate = (Object *)childList->lh_Head;
		  Object *child;
      int i=0;

		  // iterate through the childList and update the rule structures
      while((child = NextObject(&cstate)))
		  {
        struct RuleNode *rule;

        // get the rule out of the ruleList or create a new one
        while(!(rule = GetFilterRule(filter, i)))
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
   struct POP3 *pop3 = calloc(1, sizeof(struct POP3));
   if (pop3)
   {
      if (first)
      {
         char *p, buffer[SIZE_USERID];

         stccpy(buffer, co->EmailAddress, SIZE_USERID);
         if ((p = strchr(buffer, '@'))) *p = 0;
         strcpy(pop3->User, buffer);
         strcpy(pop3->Server, co->SMTP_Server);
      }

      pop3->Port = 110;
      pop3->Enabled = TRUE;
      pop3->DeleteOnServer = TRUE;
   }
   return pop3;
}

///
/// CO_AddPOP3
//  Adds a new entry to the POP3 account list
HOOKPROTONHNONP(CO_AddPOP3, void)
{
   int i;
   for (i = 0; i < MAXP3; i++)
      if (!CE->P3[i])
      {
         CE->P3[i] = CO_NewPOP3(CE, i == 0);
         DoMethod(G->CO->GUI.LV_POP3, MUIM_List_InsertSingle, CE->P3[i], MUIV_List_Insert_Bottom, TAG_DONE);
         set(G->CO->GUI.LV_POP3, MUIA_List_Active, i);
         set(G->CO->GUI.WI, MUIA_Window_ActiveObject, G->CO->GUI.ST_POPHOST);
         break;
      }
}
MakeHook(CO_AddPOP3Hook,CO_AddPOP3);

///
/// CO_DelPOP3
//  Deletes an entry from the POP3 account list
HOOKPROTONHNONP(CO_DelPOP3, void)
{
   struct CO_GUIData *gui = &G->CO->GUI;
   int p = xget(gui->LV_POP3, MUIA_List_Active);
   int e = xget(gui->LV_POP3, MUIA_List_Entries);

   if (p != MUIV_List_Active_Off && e > 1)
   {
      int i;

      DoMethod(gui->LV_POP3, MUIM_List_Remove, p);

      for(i=p+1; i < MAXP3; i++)
      {
        CE->P3[i-1] = CE->P3[i];
      }

      CE->P3[i-1] = NULL;
   }
}
MakeHook(CO_DelPOP3Hook,CO_DelPOP3);

///
/// CO_GetP3Entry
//  Fills form with data from selected list entry
HOOKPROTONHNONP(CO_GetP3Entry, void)
{
   struct POP3 *pop3 = NULL;
   struct CO_GUIData *gui = &G->CO->GUI;

   DoMethod(gui->LV_POP3, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &pop3);

   if(xget(gui->GR_POP3, MUIA_Disabled) != !pop3) set(gui->GR_POP3, MUIA_Disabled, !pop3); // This is needed due to a bug in MUI
   set(gui->BT_PDEL, MUIA_Disabled, !pop3 || xget(gui->LV_POP3, MUIA_List_Entries) < 2);

   if (pop3)
   {
      nnset(gui->ST_POPHOST,   MUIA_String_Contents, pop3->Server);
      nnset(gui->ST_POPPORT,   MUIA_String_Integer,  pop3->Port);
      nnset(gui->ST_POPUSERID, MUIA_String_Contents, pop3->User);
      nnset(gui->ST_PASSWD,    MUIA_String_Contents, pop3->Password);
      nnset(gui->CH_POPENABLED,MUIA_Selected, pop3->Enabled);
      nnset(gui->CH_POP3SSL,   MUIA_Selected, (pop3->SSLMode != P3SSL_OFF));
      nnset(gui->CH_USESTLS,   MUIA_Selected, (pop3->SSLMode == P3SSL_STLS));
      nnset(gui->CH_USEAPOP,   MUIA_Selected, pop3->UseAPOP);
      nnset(gui->CH_DELETE,    MUIA_Selected, pop3->DeleteOnServer);

      // we have to enabled/disable the SSL support accordingly
      set(gui->CH_USESTLS, MUIA_Disabled, !G->TR_UseableTLS || pop3->SSLMode == P3SSL_OFF);
      set(gui->CH_POP3SSL, MUIA_Disabled, !G->TR_UseableTLS && pop3->SSLMode == P3SSL_OFF);
   }
}
MakeHook(CO_GetP3EntryHook,CO_GetP3Entry);

///
/// CO_PutP3Entry
//  Fills form data into selected list entry
HOOKPROTONHNONP(CO_PutP3Entry, void)
{
   struct POP3 *pop3 = NULL;
   struct CO_GUIData *gui = &G->CO->GUI;
   int p;
   int new_ssl_mode;

   p = xget(gui->LV_POP3, MUIA_List_Active);
   if (p != MUIV_List_Active_Off)
   {
      DoMethod(gui->LV_POP3, MUIM_List_GetEntry, p, &pop3);
      GetMUIString(pop3->Server, gui->ST_POPHOST);
      GetMUIString(pop3->User, gui->ST_POPUSERID);
      GetMUIString(pop3->Password, gui->ST_PASSWD);
      pop3->Enabled        = GetMUICheck(gui->CH_POPENABLED);
      pop3->UseAPOP        = GetMUICheck(gui->CH_USEAPOP);
      pop3->DeleteOnServer = GetMUICheck(gui->CH_DELETE);
      sprintf(pop3->Account, "%s@%s", pop3->User, pop3->Server);

      if(GetMUICheck(gui->CH_POP3SSL))
      {
         new_ssl_mode = P3SSL_SSL;
         if(GetMUICheck(gui->CH_USESTLS)) new_ssl_mode = P3SSL_STLS;
      }
      else
      {
         new_ssl_mode = P3SSL_OFF;
         if(!G->TR_UseableTLS) set(gui->CH_POP3SSL, MUIA_Disabled, TRUE);
      }

      if(pop3->SSLMode != new_ssl_mode)
      {
        if(new_ssl_mode == P3SSL_SSL)
        {
          set(gui->ST_POPPORT, MUIA_String_Integer, 995);
        }
        else
        {
          set(gui->ST_POPPORT, MUIA_String_Integer, 110);
        }

        pop3->SSLMode = new_ssl_mode;
      }

      pop3->Port = GetMUIInteger(gui->ST_POPPORT);

      DoMethod(gui->LV_POP3, MUIM_List_Redraw, p);
   }
}
MakeHook(CO_PutP3EntryHook,CO_PutP3Entry);

///
/// CO_GetDefaultPOPFunc
//  Sets values of first POP3 account
HOOKPROTONHNONP(CO_GetDefaultPOPFunc, void)
{
   struct POP3 *pop3 = CE->P3[0];

   if (!pop3) return;
   GetMUIString(pop3->Server, G->CO->GUI.ST_POPHOST0);
   pop3->Port = 110;
   GetMUIString(pop3->Password, G->CO->GUI.ST_PASSWD0);
   sprintf(pop3->Account, "%s@%s", pop3->User, pop3->Server);
}
MakeHook(CO_GetDefaultPOPHook,CO_GetDefaultPOPFunc);
///

/**** Mime Viewers ****/
/// CO_NewMimeView
//  Initializes a new MIME viewer
struct MimeView *CO_NewMimeView(void)
{
   struct MimeView *mv = calloc(1, sizeof(struct MimeView));
   if (mv)
   {
      stccpy(mv->ContentType, "?/?", SIZE_CTYPE);
   }
   return mv;
}

///
/// CO_AddMimeView
//  Adds a new entry to the MIME viewer list
HOOKPROTONHNONP(CO_AddMimeView, void)
{
   struct CO_GUIData *gui = &G->CO->GUI;
   int i;
   for (i = 0; i < MAXMV; i++)
      if (!CE->MV[i])
      {
         CE->MV[i] = CO_NewMimeView();
         DoMethod(gui->LV_MIME, MUIM_List_InsertSingle, CE->MV[i], MUIV_List_Insert_Bottom);
         set(gui->LV_MIME, MUIA_List_Active, MUIV_List_Active_Bottom);
         set(gui->WI, MUIA_Window_ActiveObject, gui->ST_CTYPE);
         break;
      }
}
MakeHook(CO_AddMimeViewHook,CO_AddMimeView);

///
/// CO_DelMimeView
//  Deletes an entry from the MIME viewer list
HOOKPROTONHNONP(CO_DelMimeView, void)
{
   int p = xget(G->CO->GUI.LV_MIME, MUIA_List_Active);

   if(p != MUIV_List_Active_Off)
   {
      int i;

      DoMethod(G->CO->GUI.LV_MIME, MUIM_List_Remove, p);

      for(i=p+1; i < MAXMV-1; i++)
      {
        CE->MV[i] = CE->MV[i+1];
      }

      CE->MV[i] = NULL;
   }
}
MakeHook(CO_DelMimeViewHook,CO_DelMimeView);

///
/// CO_GetMVEntry
//  Fills form with data from selected list entry
HOOKPROTONHNONP(CO_GetMVEntry, void)
{
   struct MimeView *mv = NULL;
   struct CO_GUIData *gui = &G->CO->GUI;

   DoMethod(gui->LV_MIME, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &mv);
   if(mv)
   {
      nnset(gui->ST_CTYPE, MUIA_String_Contents, mv->ContentType);
      nnset(gui->ST_EXTENS, MUIA_String_Contents, mv->Extension);
      nnset(gui->ST_COMMAND, MUIA_String_Contents, mv->Command);
   }
   set(gui->GR_MIME, MUIA_Disabled, !mv);
   set(gui->BT_MDEL, MUIA_Disabled, !mv);
}
MakeHook(CO_GetMVEntryHook,CO_GetMVEntry);

///
/// CO_PutMVEntry
//  Fills form data into selected list entry
HOOKPROTONHNONP(CO_PutMVEntry, void)
{
   struct MimeView *mv = NULL;
   struct CO_GUIData *gui = &G->CO->GUI;

   DoMethod(gui->LV_MIME, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &mv);
   if (mv)
   {
      GetMUIString(mv->ContentType, gui->ST_CTYPE);
      GetMUIString(mv->Extension, gui->ST_EXTENS);
      GetMUIString(mv->Command, gui->ST_COMMAND);
      DoMethod(gui->LV_MIME, MUIM_List_Redraw, MUIV_List_Redraw_Active);
   }
}
MakeHook(CO_PutMVEntryHook,CO_PutMVEntry);
///

/**** ARexx Hooks ****/
/// CO_GetRXEntry
//  Fills form with data from selected list entry
HOOKPROTONHNONP(CO_GetRXEntry, void)
{
   struct CO_GUIData *gui = &G->CO->GUI;
   struct RxHook *rh;
   int act = xget(gui->LV_REXX, MUIA_List_Active);

   rh = &(CE->RX[act]);
   nnset(gui->ST_RXNAME, MUIA_String_Contents, act < 10 ? rh->Name : "");
   nnset(gui->ST_SCRIPT, MUIA_String_Contents, rh->Script);
   nnset(gui->CY_ISADOS, MUIA_Cycle_Active, rh->IsAmigaDOS ? 1 : 0);
   nnset(gui->CH_CONSOLE, MUIA_Selected, rh->UseConsole);
   nnset(gui->CH_WAITTERM, MUIA_Selected, rh->WaitTerm);
   set(gui->ST_RXNAME, MUIA_Disabled, act >= 10);
   DoMethod(gui->LV_REXX, MUIM_List_Redraw, act);
}
MakeHook(CO_GetRXEntryHook,CO_GetRXEntry);

///
/// CO_PutRXEntry
//  Fills form data into selected list entry
HOOKPROTONHNONP(CO_PutRXEntry, void)
{
   struct CO_GUIData *gui = &G->CO->GUI;
   int act = xget(gui->LV_REXX, MUIA_List_Active);

   if (act != MUIV_List_Active_Off)
   {
      struct RxHook *rh = &(CE->RX[act]);
      GetMUIString(rh->Name, gui->ST_RXNAME);
      GetMUIString(rh->Script, gui->ST_SCRIPT);
      rh->IsAmigaDOS = GetMUICycle(gui->CY_ISADOS) == 1;
      rh->UseConsole = GetMUICheck(gui->CH_CONSOLE);
      rh->WaitTerm = GetMUICheck(gui->CH_WAITTERM);
   }
}
MakeHook(CO_PutRXEntryHook,CO_PutRXEntry);
///
/// CO_IsValid
//  Verifies if the required settings have been made
BOOL CO_IsValid(void)
{
   if (G->CO_Valid) return TRUE;
   if (G->CO) set(G->CO->GUI.WI,MUIA_Window_Open,TRUE);
   else DoMethod(G->App, MUIM_CallHook, &CO_OpenHook);
   MUI_Request(G->App, G->MA->GUI.WI, 0, NULL, GetStr(MSG_OkayReq), GetStr(MSG_CO_InvalidConf));
   return FALSE;
}

///
/// CO_DetectPGP
//  Checks if PGP 2 or 5 is available
static int CO_DetectPGP(struct Config *co)
{
   if (PFExists(co->PGPCmdPath, "pgpe")) return 5;
   else if (PFExists(co->PGPCmdPath, "pgp")) return 2;
   return 0;
}

///
/// CO_FreeConfig
//  Deallocates a configuration structure
void CO_FreeConfig(struct Config *co)
{
  struct MinNode *curNode;
  int i;

  ENTER();
  SHOWVALUE(DBF_CONFIG, co);

  // free all config elements
  for(i = 0; i < MAXP3; i++) { if(co->P3[i]) free(co->P3[i]); }
  for(i = 0; i < MAXMV; i++) { if(co->MV[i]) free(co->MV[i]); }

  // we have to free the filterList
  for(curNode = co->filterList.mlh_Head; curNode->mln_Succ;)
  {
    struct FilterNode *filter = (struct FilterNode *)curNode;

    // free the ruleList of the filter
    FreeFilterRuleList(filter);

    // before we remove the node we have to save the pointer to the next one
    curNode = curNode->mln_Succ;

    // Remove node from list
    Remove((struct Node *)filter);
    free(filter);
  }

  // clear the config
  memset(co, 0, sizeof(struct Config));

  // init the filterList & stuff
  NewList((struct List *)&co->filterList);

  LEAVE();
}

///
/// CO_SetDefaults
//  Sets configuration (or a part of it) to the factory settings
void CO_SetDefaults(struct Config *co, int page)
{
   int i;

   ENTER();
   SHOWVALUE(DBF_CONFIG, co);
   SHOWVALUE(DBF_CONFIG, page);

   // [Start]
   if(page == 0 || page < 0)
   {
      *co->RealName = *co->EmailAddress = 0;

      // If Locale is present, don't use the timezone from the config
      if(G->Locale) co->TimeZone = -G->Locale->loc_GMTOffset;
      else          co->TimeZone = 0;

      co->DaylightSaving = FALSE;
   }

   // [TCP/IP]
   if(page == 1 || page < 0)
   {
      for (i = 0; i < MAXP3; i++) { if (co->P3[i]) free(co->P3[i]); co->P3[i] = NULL; }
      *co->SMTP_Server = *co->SMTP_Domain = 0;
      co->SMTP_Port = 25;
      co->Allow8bit = FALSE;
      co->SMTP_SecureMethod = SMTPSEC_NONE;
      co->Use_SMTP_AUTH = FALSE;
      *co->SMTP_AUTH_User = *co->SMTP_AUTH_Pass = 0;
      co->P3[0] = CO_NewPOP3(co, TRUE); co->P3[0]->DeleteOnServer = TRUE;
   }

   // [New Mail]
   if(page == 2 || page < 0)
   {
      co->AvoidDuplicates = FALSE;
      co->TransferWindow = 2;
      co->UpdateStatus = co->DownloadLarge = TRUE;
      co->PreSelection = 1;
      co->WarnSize = 100;
      co->CheckMailDelay = 0;
      co->NotifyType = 1;
      *co->NotifySound = *co->NotifyCommand = 0;
   }

   // [Filters]
   if(page == 3 || page < 0)
   {
      struct MinNode *curNode;

      // we have to free the filterList
      for(curNode = co->filterList.mlh_Head; curNode->mln_Succ; curNode = curNode->mln_Succ)
      {
        free(curNode);
      }
      NewList((struct List *)&co->filterList);
   }

   // [Read]
   if(page == 4 || page < 0)
   {
      for (i = 0; i < MAXMV; i++) { if (co->MV[i]) free(co->MV[i]); co->MV[i] = NULL; }
      co->ShowHeader = 1;
      strcpy(co->ShortHeaders,"(From|To|Date|Subject)");
      co->ShowSenderInfo = 2;
      strcpy(co->ColoredText.buf, "m6");
      strcpy(co->Color1stLevel.buf, "m0");
      strcpy(co->Color2ndLevel.buf, "m7");
      strcpy(co->Color3rdLevel.buf, "m3");
      strcpy(co->Color4thLevel.buf, "m1");
      strcpy(co->ColorURL.buf, "p6");
      co->DisplayAllTexts = TRUE;
      co->FixedFontEdit = TRUE;
      co->UseTextstyles = TRUE;
      co->EmbeddedReadPane = TRUE;
      co->QuickSearchBar = TRUE;
      co->AutomaticTranslationIn = FALSE;
      co->WrapHeader = FALSE;
      co->MultipleWindows = FALSE;
      co->SigSepLine = 2;
      *co->TranslationIn = 0;
      co->StatusChangeDelayOn = TRUE;
      co->StatusChangeDelay   = 1000; // 1s=1000ms delay by default
   }

   // [Write]
   if(page == 5 || page < 0)
   {
      *co->ReplyTo = *co->Organization = *co->ExtraHeaders = *co->TranslationOut = 0;
      strcpy(co->NewIntro, GetStr(MSG_CO_NewIntroDef));
      strcpy(co->Greetings, GetStr(MSG_CO_GreetingsDef));
      co->WarnSubject = TRUE;
      co->EdWrapCol = 76;
      co->EdWrapMode = 2;
      strcpy(co->Editor, "C:Ed");
      co->LaunchAlways = FALSE;
      co->EmailCache = 10;
      co->AutoSave = 120;
   }

   // [Reply/Forward]
   if(page == 6 || page < 0)
   {
      strcpy(co->ReplyHello, "Hello %f\\n");
      strcpy(co->ReplyIntro, "On %d, you wrote:\\n");
      strcpy(co->ReplyBye, "Regards");
      strcpy(co->AltReplyHello, GetStr(MSG_CO_AltRepHelloDef));
      strcpy(co->AltReplyIntro, GetStr(MSG_CO_AltRepIntroDef));
      strcpy(co->AltReplyBye, GetStr(MSG_CO_AltRepByeDef));
      strcpy(co->AltReplyPattern, GetStr(MSG_CO_AltRepPatternDef));
      strcpy(co->MLReplyHello, GetStr(MSG_CO_MLRepHelloDef));
      strcpy(co->MLReplyIntro, GetStr(MSG_CO_MLRepIntroDef));
      strcpy(co->MLReplyBye, GetStr(MSG_CO_MLRepByeDef));
      strcpy(co->ForwardIntro, GetStr(MSG_CO_ForwardIntroDef));
      strcpy(co->ForwardFinish, GetStr(MSG_CO_ForwardFinishDef));
      co->QuoteMessage = co->QuoteEmptyLines = co->CompareAddress = co->StripSignature = TRUE;
      strcpy(co->QuoteText, ">");
      strcpy(co->AltQuoteText, "|");
   }

   // [Signature]
   if(page == 7 || page < 0)
   {
      co->UseSignature = FALSE;
      strmfp(co->TagsFile, G->ProgDir, ".taglines");
      strcpy(co->TagsSeparator, "%%");
   }

   // [Lists]
   if(page == 8 || page < 0)
   {
      co->FolderCols = 1+2+16;
      co->MessageCols = 1+2+8+16;
      co->FixedFontList = C->SwatchBeat = FALSE;
      co->FolderCntMenu = TRUE;
      co->MessageCntMenu = TRUE;
      co->InfoBar = IB_POS_CENTER;
      strcpy(co->InfoBarText, GetStr(MSG_CO_InfoBarDef));
   }

   // [Security]
   if(page == 9 || page < 0)
   {
      G->PGPVersion = 0;
      if(GetVar("PGPPATH", co->PGPCmdPath, SIZE_PATH, 0) >= 0)
        G->PGPVersion = CO_DetectPGP(co);

      if(!G->PGPVersion)
      {
         strcpy(co->PGPCmdPath, "C:");
         G->PGPVersion = CO_DetectPGP(co);
      }
      *co->MyPGPID = 0;
      co->EncryptToSelf = co->LogAllEvents = TRUE;
      strcpy(co->ReMailer, "Remailer <remailer@remailer.xganon.com>");
      strcpy(co->RMCommands, "Anon-To: %s");
      strcpy(co->LogfilePath, G->ProgDir);
      co->LogfileMode = 1;
      co->SplitLogfile = FALSE;
   }

   // [Start/Quit]
   if(page == 10 || page < 0)
   {
      co->GetOnStartup = co->SendOnStartup = co->LoadAllFolders = co->SendOnQuit = FALSE;
      co->CleanupOnStartup = co->RemoveOnStartup = FALSE;
      co->UpdateNewMail = co->CheckBirthdates = co->CleanupOnQuit = co->RemoveOnQuit = TRUE;
   }

   // [MIME]
   if(page == 11 || page < 0)
   {
      co->MV[0] = CO_NewMimeView();
      strcpy(co->MV[0]->ContentType, GetStr(MSG_Default));
      strcpy(co->MV[0]->Command, "SYS:Utilities/Multiview \"%s\"");
      co->IdentifyBin = TRUE;
      strcpy(co->DetachDir, "RAM:");
      strcpy(co->AttachDir, "RAM:");
   }

   // [Address book]
   if(page == 12 || page < 0)
   {
      strcpy(co->GalleryDir, "YAM:Gallery");
      strcpy(co->NewAddrGroup, "NEW");
      co->AddMyInfo = FALSE;
      co->AddToAddrbook = 0;
      co->AddrbookCols = 1+2+4;
   }

   // [Scripts]
   if(page == 13 || page < 0)
   {
      for (i = 0; i < MAXRX; i++)
      {
         *co->RX[i].Name = *co->RX[i].Script = 0;
         co->RX[i].IsAmigaDOS = co->RX[i].UseConsole = FALSE;
         co->RX[i].WaitTerm = TRUE;
      }
   }

   // [Mixed]
   if(page == 14 || page < 0)
   {
      strcpy(co->TempDir, "T:");
      strcpy(co->PackerCommand, "LhA -a -m -i%l a \"%a\"");
      co->IconPositionX = co->IconPositionY = 0;
      strcpy(co->AppIconText, GetStr(MSG_CO_APPICON_LABEL));
      co->IconifyOnQuit = co->RemoveAtOnce = FALSE;
      co->Confirm = co->SaveSent = co->SendMDNAtOnce = TRUE;
      co->ConfirmDelete = 2;
      co->MDN_Display = co->MDN_Process = co->MDN_Delete = 2;
      co->MDN_Filter = 3;
      strcpy(co->XPKPack, "HUFF");
      strcpy(co->XPKPackEncrypt, "HUFF");
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

   // everything else
   if(page < 0)
   {
      co->LetterPart = 1;
      co->WriteIndexes = 120;
      strcpy(co->SupportSite, "http://www.yam.ch/");
      co->JumpToNewMsg = co->AskJumpUnread = co->PrinterCheck = co->IsOnlineCheck = TRUE;
      co->JumpToIncoming = FALSE;
      co->ConfirmOnQuit = FALSE;
      co->HideGUIElements = 0;
      strcpy(co->LocalCharset, "ISO-8859-1");
      co->SysCharsetCheck = TRUE;
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
      co->TRBufferSize = 8192;
      co->EmbeddedMailDelay = 200; // 200ms delay by default
   }

   LEAVE();
}

///
/// CopyConfigData
//  Copies a configuration structure (deep copy)
static void CopyConfigData(struct Config *dco, struct Config *sco)
{
  int i;
  struct MinNode *curNode;

  ENTER();
  SHOWVALUE(DBF_CONFIG, sco);
  SHOWVALUE(DBF_CONFIG, dco);

  // first we copy all raw data via memcpy
  memcpy(dco, sco, sizeof(struct Config));

  // then we have to do a deep copy any allocate separate memory for our copy
  for(i = 0; i < MAXP3; i++)
    dco->P3[i] = sco->P3[i] ? (struct POP3 *)AllocCopy(sco->P3[i], sizeof(struct POP3)) : NULL;

  for(i = 0; i < MAXMV; i++)
    dco->MV[i] = sco->MV[i] ? (struct MimeView *)AllocCopy(sco->MV[i], sizeof(struct MimeView)) : NULL;

  // for copying the filters we do have to do another deep copy
  NewList((struct List *)&dco->filterList);

  for(curNode = sco->filterList.mlh_Head; curNode->mln_Succ; curNode = curNode->mln_Succ)
  {
    struct FilterNode *srcFilter = (struct FilterNode *)curNode;
    struct FilterNode *dstFilter = calloc(1, sizeof(struct FilterNode));

    CopyFilterData(dstFilter, srcFilter);

    AddTail((struct List *)&dco->filterList, (struct Node *)dstFilter);
  }

  LEAVE();
}

///
/// CO_Validate
//  Validates a configuration, update GUI etc.
void CO_Validate(struct Config *co, BOOL update)
{
   char *p, buffer[SIZE_USERID];
   BOOL saveAtEnd = FALSE;
   int i;

   if (!*co->SMTP_Server) strcpy(co->SMTP_Server, co->P3[0]->Server);
   if (co->SMTP_Port == 0) co->SMTP_Port = 25;
   if (!*co->SMTP_Domain) { p = strchr(co->EmailAddress, '@'); strcpy(co->SMTP_Domain, p ? p+1 : ""); }
   for (i = 0; i < MAXP3; i++) if (co->P3[i])
   {
      if (!*co->P3[i]->Server) strcpy(co->P3[i]->Server, co->SMTP_Server);
      if (co->P3[i]->Port == 0) co->P3[i]->Port = 110;

      if (!*co->P3[i]->User)
      {
         stccpy(buffer, co->EmailAddress, SIZE_USERID);
         if ((p = strchr(buffer, '@'))) *p = 0;
         strcpy(co->P3[i]->User, buffer);
      }
      sprintf(co->P3[i]->Account, "%s@%s", co->P3[i]->User, co->P3[i]->Server);
   }

   // If Locale is present, overwrite the timezone from the config
   if(G->Locale) co->TimeZone = -(G->Locale->loc_GMTOffset);

   // lets check the DaylightSaving stuff now
   if(G->CO_DST) co->DaylightSaving = G->CO_DST==2;

   G->PGPVersion = CO_DetectPGP(co);

   // prepare the temporary directory
   CreateDirectory(co->TempDir);

   // then prepare the temporary filenames for the write windows
   for(i=0; i <= MAXWR; i++)
   {
      char filename[SIZE_FILE];

      sprintf(filename, "YAMw%08lx-%d.tmp", (LONG)FindTask(NULL), i);
      strmfp(G->WR_Filename[i], co->TempDir, filename);
   }

   LoadTranslationTable(&(G->TTin), co->AutomaticTranslationIn ? NULL : co->TranslationIn);
   G->CO_AutoTranslateIn = co->AutomaticTranslationIn;
   LoadTranslationTable(&(G->TTout), co->TranslationOut);
   G->CO_Valid = (*co->SMTP_Server && *co->EmailAddress && *co->RealName);

   // we try to find out the system charset and validate it with the
   // currently configured local charset
   if(co->SysCharsetCheck)
   {
      char sysCharset[SIZE_CTYPE+1];

      memset(sysCharset, 0, sizeof(sysCharset));

      #ifdef __amigaos4__
      {
	      LONG default_charset = GetDiskFontCtrl(DFCTRL_CHARSET);
		    char *charset = (char *)ObtainCharsetInfo(DFCS_NUMBER, default_charset, DFCS_MIMENAME);

        if(charset && charset[0])
          strncpy(sysCharset, charset, SIZE_CTYPE);
      }
      #endif

      // if we still do not have our default charset we try to load
      // it from and environment variable ENVARC:CHARSET
      if(sysCharset[0] == '\0')
      {
        char var[SIZE_CTYPE+1];

        if(GetVar("CHARSET", var, sizeof(var), 0) > 0)
          strncpy(sysCharset, (char *)var, SIZE_CTYPE);
      }

      // now we check whether the currently set localCharset matches
      // the system charset or not
      if(co->LocalCharset[0] && sysCharset[0])
      {
        if(stricmp(co->LocalCharset, sysCharset) != 0)
        {
          int res = MUI_Request(G->App, NULL, 0,
                                GetStr(MSG_CO_CHARSETWARN_TITLE),
                                GetStr(MSG_CO_CHARSETWARN_BT),
                                GetStr(MSG_CO_CHARSETWARN),
                                co->LocalCharset, sysCharset);

          // if the user has clicked on Change, we do
          // change the charset and save it immediatly
          if(res == 1)
          {
            strncpy(co->LocalCharset, sysCharset, SIZE_CTYPE);
            saveAtEnd = TRUE;
          }
          else if(res == 2)
          {
            co->SysCharsetCheck = FALSE;
            saveAtEnd = TRUE;
          }
        }
      }
      else if(sysCharset[0])
      {
        strncpy(co->LocalCharset, sysCharset, SIZE_CTYPE);
        saveAtEnd = TRUE;
      }
   }

   if(co->LocalCharset[0] == '\0')
   {
      strcpy(co->LocalCharset, "ISO-8859-1");
      saveAtEnd = TRUE;
   }

   if(G->CO_AutoTranslateIn)
     LoadParsers();

   if(update && G->CO)
   {
      switch (G->CO->VisiblePage)
      {
         case 0:
            setstring(G->CO->GUI.ST_POPHOST0, co->P3[0]->Server);
            break;

         case 1:
            setstring(G->CO->GUI.ST_SMTPHOST, co->SMTP_Server);
            set(G->CO->GUI.ST_SMTPPORT, MUIA_String_Integer, co->SMTP_Port);
            setstring(G->CO->GUI.ST_DOMAIN, co->SMTP_Domain);
            setstring(G->CO->GUI.ST_SMTPAUTHUSER, co->SMTP_AUTH_User);
            setstring(G->CO->GUI.ST_SMTPAUTHPASS, co->SMTP_AUTH_Pass);
            DoMethod(G->CO->GUI.LV_POP3, MUIM_List_Redraw, MUIV_List_Redraw_All);
            break;
      }

      if(G->CO->Visited[1] || G->CO->Visited[13] || G->CO->UpdateAll)
        MA_SetupDynamicMenus();

      if(G->CO->Visited[2] || G->CO->UpdateAll)
      {
        // requeue the timerequest for the CheckMailDelay
        TC_Restart(TIO_CHECKMAIL, co->CheckMailDelay*60, 0);
      }

      if(G->CO->Visited[4] || G->CO->UpdateAll)
      {
         // we signal the mainwindow that it may check whether to include the
         // embedded read pane part or not
         MA_SetupEmbeddedReadPane();

         // and to not let the embedded read pane be empty when it is newly created
         // we have to make sure the actual selected mail is loaded
         if(C->EmbeddedReadPane)
           MA_ChangeSelected(TRUE);
      }

      if(G->CO->Visited[5] || G->CO->UpdateAll)
      {
        // requeue the timerequest for the AutoSave interval
        TC_Restart(TIO_AUTOSAVE, co->AutoSave, 0);
      }

      if(G->CO->Visited[8] || G->CO->UpdateAll)
      {
         // First we set the PG_MAILLIST and NL_FOLDER Quiet
         set(G->MA->GUI.PG_MAILLIST,MUIA_NList_Quiet,     TRUE);
         set(G->MA->GUI.NL_FOLDERS, MUIA_NListtree_Quiet, TRUE);

         // Modify the ContextMenu flags
         set(G->MA->GUI.PG_MAILLIST,MUIA_ContextMenu, C->MessageCntMenu ? MUIV_NList_ContextMenu_Always : MUIV_NList_ContextMenu_Never);
         set(G->MA->GUI.NL_FOLDERS, MUIA_ContextMenu, C->FolderCntMenu ? MUIV_NList_ContextMenu_Always : MUIV_NList_ContextMenu_Never);

         // Now we reorder the Maingroup accordingly to the InfoBar setting
         MA_SortWindow();

         // Now we update the InfoBar because the text could have been changed
         DoMethod(G->MA->GUI.IB_INFOBAR, MUIM_InfoBar_SetFolder, FO_GetCurrentFolder());

         // we signal the mainwindow that it may check whether to include the
         // quicksearchbar or not
         MA_SetupQuickSearchBar();

         SaveLayout(FALSE);
         MA_MakeFOFormat(G->MA->GUI.NL_FOLDERS);
         DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_MainMailListGroup_MakeFormat);
         LoadLayout();

         // Now we give the control back to the NLists
         set(G->MA->GUI.PG_MAILLIST,MUIA_NList_Quiet,     FALSE);
         set(G->MA->GUI.NL_FOLDERS, MUIA_NListtree_Quiet, FALSE);
      }

      if(G->CO->Visited[12] || G->CO->UpdateAll)
         AB_MakeABFormat(G->AB->GUI.LV_ADDRESSES);

      if(G->CO->Visited[14] || G->CO->UpdateAll)
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
        SetupAppIcons();
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
   }

   // if some items have modified the config we do save it again.
   if(saveAtEnd == TRUE)
      CO_SaveConfig(co, G->CO_PrefsFile);
}

///
/// CO_ImportCTypes
//  Imports MIME viewers from a MIME.prefs file
HOOKPROTONHNONP(CO_ImportCTypesFunc, void)
{
   int mode;

   if ((mode = MUI_Request(G->App, G->CO->GUI.WI, 0, GetStr(MSG_CO_ImportMIME), GetStr(MSG_CO_ImportMIMEGads), GetStr(MSG_CO_ImportMIMEReq))))
      if (ReqFile(ASL_CONFIG,G->CO->GUI.WI, GetStr(MSG_CO_IMPORTMIMETITLE), REQF_NONE, (mode == 1 ? "ENV:" : G->MA_MailDir), (mode == 1 ? "MIME.prefs" : (mode == 2 ? "mailcap" : "mime.types"))))
      {
         char buffer[SIZE_LARGE], fname[SIZE_PATHFILE], *p, *p2;
         struct MimeView *mv;
         APTR lv = G->CO->GUI.LV_MIME;
         int i;
         FILE *fh;
         strmfp(fname, G->ASLReq[ASL_CONFIG]->fr_Drawer, G->ASLReq[ASL_CONFIG]->fr_File);
         if ((fh = fopen(fname, "r")))
         {
            set(lv, MUIA_List_Quiet, TRUE);
            while (fgets(buffer, SIZE_LARGE, fh))
            {
               char *ctype = buffer, *ext = "", *command = "";
               if ((p = strpbrk(ctype, "\r\n"))) *p = 0;
               if (!*ctype || ISpace(*ctype)) continue;
               if (mode == 1)
               {
                  if (*ctype == ';') continue;
                  for (p = ctype; *p && *p != ','; ++p);
                  if (*p)
                  {
                     for (*p = 0, ext = ++p; *p && *p != ','; ++p);
                     if (*p)
                     {
                        for (*p++ = 0; *p && *p != ','; ++p);
                        if (*p)
                        {
                           for (command = ++p; *p && *p != ','; ++p);
                           *p = 0;
                        }
                     }
                  }
               }
               else if (mode == 2)
               {
                  if (*ctype == '#') continue;
                  for (p2 = p = ctype; !ISpace(*p) && *p && *p != ';'; p2 = ++p);
                  if ((p = strpbrk(p,";"))) ++p;
                  if (p) command = TrimStart(p);
                  *p2 = 0;
               }
               else
               {
                  if (*ctype == '#') continue;
                  for (p2 = p = ctype; !ISpace(*p) && *p; p2 = ++p);
                  if (*p) ext = TrimStart(p);
                  *p2 = 0;
               }
               for (mv = NULL, i = 0; i < MAXMV; i++) if (CE->MV[i]) if (!stricmp(CE->MV[i]->ContentType, ctype)) { mv = CE->MV[i]; break; }
               if (!mv) for (i = 0; i < MAXMV; i++) if (!CE->MV[i])
               {
                  mv = CE->MV[i] = CO_NewMimeView();
                  DoMethod(lv, MUIM_List_InsertSingle, mv, MUIV_List_Insert_Bottom);
                  break;
               }
               if (mv)
               {
                  for (p = mv->ContentType; *ctype && strlen(mv->ContentType) < SIZE_CTYPE; ctype++)
                     if (*ctype == '*') { *p++ = '#'; *p++ = '?'; } else *p++ = *ctype;
                  *p = 0;
                  if (*command)
                  {
                     for (p = mv->Command; *command && strlen(mv->Command) < SIZE_COMMAND; command++)
                        if (*command == '%' && command[1] == 'f') { *p++ = *command++; *p++ = 's'; } else *p++ = *command;
                     *p = 0;
                  }
                  if (*ext) stccpy(mv->Extension, ext, SIZE_NAME);
               }
            }
            fclose(fh);
            set(lv, MUIA_List_Quiet, FALSE);
            DoMethod(lv, MUIM_List_Redraw, MUIV_List_Redraw_All);
         }
         else ER_NewError(GetStr(MSG_ER_CantOpenFile), fname);
      }
}
MakeStaticHook(CO_ImportCTypesHook, CO_ImportCTypesFunc);

///
/// CO_EditSignatFunc
//  Edits the signature file
HOOKPROTONHNO(CO_EditSignatFunc, void, int *arg)
{
   int sig = GetMUICycle(G->CO->GUI.CY_SIGNAT);
   int modified;
   char buffer[SIZE_COMMAND+SIZE_PATHFILE];
   APTR ed = G->CO->GUI.TE_SIGEDIT;

   modified = xget(ed, MUIA_TextEditor_HasChanged);
   if(modified)
   {
      EditorToFile(ed, CreateFilename(SigNames[G->CO->LastSig]), NULL);
   }

   if(*arg)
   {
      if(*(CE->Editor))
      {
        sprintf(buffer,"%s \"%s\"", CE->Editor, GetRealPath(CreateFilename(SigNames[sig])));
        ExecuteCommand(buffer, FALSE, OUT_NIL);
      }
      else return;
   }

   if(!FileToEditor(CreateFilename(SigNames[sig]), ed))
     DoMethod(ed, MUIM_TextEditor_ClearText);

   set(ed, MUIA_TextEditor_HasChanged, FALSE);
   G->CO->LastSig = sig;
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
   if (ReqFile(ASL_CONFIG,G->CO->GUI.WI, GetStr(MSG_CO_Open), REQF_NONE, G->MA_MailDir, ""))
   {
      char cname[SIZE_PATHFILE];
      strmfp(cname, G->ASLReq[ASL_CONFIG]->fr_Drawer, G->ASLReq[ASL_CONFIG]->fr_File);
      if (CO_LoadConfig(CE, cname, NULL)) CO_NewPrefsFile(cname);
      CO_SetConfig();
      G->CO->UpdateAll = TRUE;
   }
}
MakeStaticHook(CO_OpenConfigHook, CO_OpenConfig);

///
/// CO_SaveConfigAs
//  Saves configuration to a file using an alternative name
HOOKPROTONHNONP(CO_SaveConfigAs, void)
{
   if (ReqFile(ASL_CONFIG,G->CO->GUI.WI, GetStr(MSG_CO_SaveAs), REQF_SAVEMODE, G->MA_MailDir, ""))
   {
      char cname[SIZE_PATHFILE];
      strmfp(cname, G->ASLReq[ASL_CONFIG]->fr_Drawer, G->ASLReq[ASL_CONFIG]->fr_File);
      CO_GetConfig();
      CO_Validate(CE, TRUE);
      CO_NewPrefsFile(cname);
      CO_SaveConfig(CE, cname);
   }
}
MakeStaticHook(CO_SaveConfigAsHook, CO_SaveConfigAs);

///
/// CO_Restore
//  Makes all changes undone
HOOKPROTONHNONP(CO_Restore, void)
{
   CO_FreeConfig(CE);
   CopyConfigData(CE, C);
   CO_SetConfig();
}
MakeStaticHook(CO_RestoreHook,CO_Restore);

///
/// CO_LastSaved
//  Reloads configuration from file
HOOKPROTONHNONP(CO_LastSaved, void)
{
   CO_LoadConfig(CE, G->CO_PrefsFile, NULL);
   CO_SetConfig();
   G->CO->UpdateAll = TRUE;
}
MakeStaticHook(CO_LastSavedHook,CO_LastSaved);

///
/// CO_ResetToDefaultFunc
//  Resets configuration (or a part of it)
HOOKPROTONHNO(CO_ResetToDefaultFunc, void, int *arg)
{
   if (*arg) { CO_SetDefaults(CE, -1); G->CO->UpdateAll = TRUE; }
   else CO_SetDefaults(CE, G->CO->VisiblePage);
   CO_SetConfig();
}
MakeStaticHook(CO_ResetToDefaultHook,CO_ResetToDefaultFunc);

///
/// CO_BuildPage
//  Creates a GUI section of the configuration
static APTR CO_BuildPage(struct CO_ClassData *data, int page)
{
   switch (page)
   {
      case  0: return CO_Page0(data);
      case  1: return CO_Page1(data);
      case  2: return CO_Page2(data);
      case  3: return CO_Page3(data);
      case  4: return CO_Page4(data);
      case  5: return CO_Page5(data);
      case  6: return CO_Page6(data);
      case  7: return CO_Page7(data);
      case  8: return CO_Page8(data);
      case  9: return CO_Page9(data);
      case 10: return CO_Page10(data);
      case 11: return CO_Page11(data);
      case 12: return CO_Page12(data);
      case 13: return CO_Page13(data);
      case 14: return CO_Page14(data);
   }

   return NULL;
}

///
/// CO_ChangePageFunc
//  Selects a different section of the configuration
HOOKPROTONHNO(CO_ChangePageFunc, void, int *arg)
{
  struct CO_GUIData *gui = &G->CO->GUI;

  if(*arg < 0 || *arg >= MAXCPAGES)
    return;

  set(gui->WI, MUIA_Window_Sleep, TRUE);
  CO_GetConfig();

  if(DoMethod(gui->GR_PAGE, MUIM_Group_InitChange))
  {
    DoMethod(gui->GR_PAGE, OM_REMMEMBER, gui->GR_SUBPAGE);
    MUI_DisposeObject(gui->GR_SUBPAGE);

    if((gui->GR_SUBPAGE = CO_BuildPage(G->CO, *arg)))
    {
      DoMethod(gui->GR_PAGE, OM_ADDMEMBER, gui->GR_SUBPAGE);
      G->CO->VisiblePage = *arg;
      G->CO->Visited[*arg] = TRUE;
    }

    DoMethod(gui->GR_PAGE, MUIM_Group_ExitChange);
    set(gui->MI_IMPMIME, MUIA_Menuitem_Enabled, *arg == 11);
    CO_SetConfig();
  }
  set(gui->WI, MUIA_Window_Sleep, FALSE);
}
MakeStaticHook(CO_ChangePageHook,CO_ChangePageFunc);

///
/// CO_CloseFunc
//  Closes configuration window
HOOKPROTONHNO(CO_CloseFunc, void, int *arg)
{
  // check if we should copy our edited configuration
  // to the real one or if we should just free/drop it
  if(*arg >= 1)
  {
    CO_GetConfig();
    CO_FreeConfig(C);
    CopyConfigData(C, CE);
    CO_Validate(C, TRUE);

    // if the configuration should be saved we do it immediatley
    if(*arg == 2)
      CO_SaveConfig(C, G->CO_PrefsFile);
  }

  // then we free our temporary config structure
  CO_FreeConfig(CE);
  free(CE);
  CE = NULL;

  DisposeModulePush(&G->CO);
}
MakeStaticHook(CO_CloseHook,CO_CloseFunc);

///
/// CO_OpenFunc
//  Opens configuration window
HOOKPROTONHNONP(CO_OpenFunc, void)
{
  // check if there isn't already a configuration
  // open
  if(!G->CO)
  {
    if(!(G->CO = CO_New()))
      return;

    if((CE = malloc(sizeof(struct Config))) == NULL)
      return;

    CopyConfigData(CE, C);
    CO_SetConfig();
    CO_NewPrefsFile(G->CO_PrefsFile);
  }

  // make sure the configuration window is open
  if(!SafeOpenWindow(G->CO->GUI.WI))
    CallHookPkt(&CO_CloseHook, 0, 0);
}
MakeHook(CO_OpenHook,CO_OpenFunc);
///

/*** GUI ***/
/// CO_New
//  Creates configuration window
enum { CMEN_OPEN = 1201, CMEN_SAVEAS, CMEN_DEF, CMEN_DEFALL, CMEN_LAST, CMEN_REST, CMEN_MIME };

static struct CO_ClassData *CO_New(void)
{
   struct CO_ClassData *data = calloc(1, sizeof(struct CO_ClassData));
   if (data)
   {
      static struct PageList page[MAXCPAGES], *pages[MAXCPAGES+1];
      int i;

      for (i = 0; i < MAXCPAGES; i++) { page[i].Offset = i; pages[i] = &page[i]; }
      pages[i] = NULL;

      page[ 0].PageLabel = MSG_CO_CrdFirstSteps;
      page[ 1].PageLabel = MSG_CO_CrdTCPIP;
      page[ 2].PageLabel = MSG_CO_CrdNewMail;
      page[ 3].PageLabel = MSG_CO_CrdFilters;
      page[ 4].PageLabel = MSG_CO_CrdRead;
      page[ 5].PageLabel = MSG_CO_CrdWrite;
      page[ 6].PageLabel = MSG_CO_CrdReply;
      page[ 7].PageLabel = MSG_CO_CrdSignature;
      page[ 8].PageLabel = MSG_CO_CrdLists;
      page[ 9].PageLabel = MSG_CO_CrdSecurity;
      page[10].PageLabel = MSG_CO_CrdStartQuit;
      page[11].PageLabel = MSG_CO_CrdMIME;
      page[12].PageLabel = MSG_CO_CrdABook;
      page[13].PageLabel = MSG_CO_CrdScripts;
      page[14].PageLabel = MSG_CO_CrdMixed;

      data->GUI.WI = WindowObject,
         MUIA_Window_Title, GetStr(MSG_MA_MConfig),
         MUIA_HelpNode,"CO_W",
         MUIA_Window_Menustrip, MenustripObject,
            MUIA_Family_Child, MenuObject, MUIA_Menu_Title, GetStr(MSG_MA_Project),
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_CO_Open), MUIA_Menuitem_Shortcut,"O", MUIA_UserData,CMEN_OPEN, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_CO_SaveAs), MUIA_Menuitem_Shortcut,"A", MUIA_UserData,CMEN_SAVEAS, End,
            End,
            MUIA_Family_Child, MenuObject, MUIA_Menu_Title, GetStr(MSG_CO_Edit),
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_CO_ResetDefaults), MUIA_Menuitem_Shortcut,"D", MUIA_UserData,CMEN_DEF, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_CO_ResetAll), MUIA_Menuitem_Shortcut,"E", MUIA_UserData,CMEN_DEFALL, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_CO_LastSaved), MUIA_Menuitem_Shortcut,"L", MUIA_UserData,CMEN_LAST, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_CO_Restore), MUIA_Menuitem_Shortcut,"R", MUIA_UserData,CMEN_REST, End,
            End,
            MUIA_Family_Child, MenuObject, MUIA_Menu_Title, GetStr(MSG_CO_Extras),
               MUIA_Family_Child, data->GUI.MI_IMPMIME = MenuitemObject, MUIA_Menuitem_Enabled,FALSE, MUIA_Menuitem_Title,GetStr(MSG_CO_ImportMIME), MUIA_UserData,CMEN_MIME, End,
            End,
         End,
         MUIA_Window_ID, MAKE_ID('C','O','N','F'),
         WindowContents, VGroup,
            Child, HGroup,
               GroupSpacing(3),
               Child, data->GUI.NLV_PAGE = NListviewObject,
                  MUIA_HorizWeight, 25,
                  MUIA_CycleChain,  TRUE,
                  MUIA_NListview_Horiz_ScrollBar, MUIV_NListview_HSB_None,
                  MUIA_NListview_NList, data->GUI.LV_PAGE = ConfigPageListObject,
                     InputListFrame,
                     MUIA_NList_Format,         "MW=-1 W=-1",
                     MUIA_NList_MinLineHeight,  16,
                     MUIA_NList_SourceArray,    pages,
                     MUIA_NList_Active,         MUIV_NList_Active_Top,
                  End,
               End,
               Child, BalanceObject, End,
               Child, data->GUI.GR_PAGE = VGroup,
                  MUIA_HorizWeight, 100,
                  TextFrame,
                  InnerSpacing(6,6),
                  MUIA_Background, MUII_PageBack,
                  Child, data->GUI.GR_SUBPAGE = CO_BuildPage(data, 0),
               End,
            End,
            Child, HGroup,
               MUIA_Group_SameWidth, TRUE,
               Child, data->GUI.BT_SAVE   = MakeButton(GetStr(MSG_CO_Save)),
               Child, data->GUI.BT_USE    = MakeButton(GetStr(MSG_CO_Use)),
               Child, data->GUI.BT_CANCEL = MakeButton(GetStr(MSG_CO_Cancel)),
            End,
         End,
      End;
      if (data->GUI.WI)
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
         DoMethod(data->GUI.WI          ,MUIM_Notify,MUIA_Window_MenuAction  ,CMEN_MIME     ,MUIV_Notify_Application,3,MUIM_CallHook,&CO_ImportCTypesHook,FALSE);
         DoMethod(data->GUI.LV_PAGE     ,MUIM_Notify,MUIA_NList_Active       ,MUIV_EveryTime,MUIV_Notify_Application,3,MUIM_CallHook,&CO_ChangePageHook,MUIV_TriggerValue);
         DoMethod(data->GUI.BT_SAVE     ,MUIM_Notify,MUIA_Pressed            ,FALSE         ,MUIV_Notify_Application,3,MUIM_CallHook,&CO_CloseHook,2);
         DoMethod(data->GUI.BT_USE      ,MUIM_Notify,MUIA_Pressed            ,FALSE         ,MUIV_Notify_Application,3,MUIM_CallHook,&CO_CloseHook,1);
         DoMethod(data->GUI.BT_CANCEL   ,MUIM_Notify,MUIA_Pressed            ,FALSE         ,MUIV_Notify_Application,3,MUIM_CallHook,&CO_CloseHook,0);
         DoMethod(data->GUI.WI          ,MUIM_Notify,MUIA_Window_CloseRequest,TRUE          ,MUIV_Notify_Application,3,MUIM_CallHook,&CO_CloseHook,0);
         return data;
      }
      free(data);
   }
   return NULL;
}
////
