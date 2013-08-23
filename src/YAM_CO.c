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

#include "mui/ClassesExtra.h"
#include "mui/AddrBookListtree.h"
#include "mui/AddressBookConfigPage.h"
#include "mui/ConfigPage.h"
#include "mui/ConfigPageList.h"
#include "mui/FirstStepsConfigPage.h"
#include "mui/IdentitiesConfigPage.h"
#include "mui/InfoBar.h"
#include "mui/MainFolderListtree.h"
#include "mui/MainMailListGroup.h"
#include "mui/MainWindow.h"
#include "mui/MainWindowToolbar.h"
#include "mui/MailServerChooser.h"
#include "mui/ObjectList.h"
#include "mui/ReadConfigPage.h"
#include "mui/ReadMailGroup.h"
#include "mui/ReadWindow.h"
#include "mui/ReplyForwardConfigPage.h"
#include "mui/SearchControlGroup.h"
#include "mui/SecurityConfigPage.h"
#include "mui/SignatureChooser.h"
#include "mui/SignatureConfigPage.h"
#include "mui/SignatureTextEdit.h"
#include "mui/SpamConfigPage.h"
#include "mui/StartupQuitConfigPage.h"
#include "mui/TCPIPConfigPage.h"
#include "mui/UpdateConfigPage.h"
#include "mui/WriteConfigPage.h"
#include "mui/WriteWindow.h"

#include "Busy.h"
#include "DockyIcon.h"
#include "FileInfo.h"
#include "Locale.h"
#include "MimeTypes.h"
#include "MailServers.h"
#include "MUIObjects.h"
#include "ParseEmail.h"
#include "Requesters.h"
#include "Signature.h"
#include "UserIdentity.h"
#include "TZone.h"

#include "Debug.h"

extern struct Library *AmiSSLBase;
extern struct Library *AmiSSLMasterBase;

struct Config *C = NULL;
struct Config *CE = NULL;

/* local defines */
#if defined(__amigaos4__)
#define SYS_EDITOR "SYS:Utilities/NotePad"
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

  ENTER();

  // get the active filterNode
  DoMethod(gui->LV_RULES, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &filter);

  if(filter != NULL)
  {
    struct RuleNode *rule;

    if((rule = CreateNewRule(filter, 0)) != NULL)
    {
      Object *newSearchGroup;

      // add a new GUI element for that particular rule
      newSearchGroup = SearchControlGroupObject,
                         MUIA_SearchControlGroup_RemoteFilterMode, filter->remote,
                       End;

      if(newSearchGroup != NULL)
      {
        // fill the new search group with some content
        DoMethod(newSearchGroup, MUIM_SearchControlGroup_GetFromRule, rule);

        // set some notifies
        DoMethod(newSearchGroup, MUIM_Notify, MUIA_SearchControlGroup_Modified, MUIV_EveryTime, MUIV_Notify_Application, 2, MUIM_CallHook, &SetActiveFilterDataHook);

        // add it to our searchGroupList
        DoMethod(gui->GR_SGROUP, MUIM_ObjectList_AddItem, newSearchGroup);

        GhostOutFilter(gui, filter);
      }
    }
  }

  LEAVE();
}
MakeHook(AddNewRuleToListHook, AddNewRuleToList);

///
/// RemoveLastRule
//  Deletes the last rule of the filter
HOOKPROTONHNONP(RemoveLastRule, void)
{
  struct FilterNode *filter = NULL;
  struct CO_GUIData *gui = &G->CO->GUI;

  ENTER();

  // get the active filterNode
  DoMethod(gui->LV_RULES, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &filter);

  // if we got an active entry lets remove it from the GUI List
  // and also from the filter's rule list
  if(filter != NULL)
  {
    // lets remove the rule at the end of our ruleList
    struct RuleNode *rule = (struct RuleNode *)RemTail((struct List *)&filter->ruleList);

    if(rule != NULL)
    {
      DeleteRuleNode(rule);
    }
  }

  LEAVE();
}
MakeHook(RemoveLastRuleHook, RemoveLastRule);

///
/// ImportFilterHook
//  Import filter settings from a .sfd file
HOOKPROTONHNP(ImportFilterFunc, void, Object *obj)
{
  struct FileReqCache *frc;

  ENTER();

  if((frc = ReqFile(ASL_FILTER, _win(obj), tr(MSG_FILTER_IMPORT_TITLE), 0, "PROGDIR:", "")) != NULL)
  {
    char path[SIZE_PATHFILE];

    AddPath(path, frc->drawer, frc->file, sizeof(path));
    ImportFilter(path, FALSE, &CE->filterList);
  }

  LEAVE();
}
MakeHook(ImportFilterHook, ImportFilterFunc);

///
/// ImportExternalSpamFilters
// import additional spam filter rules
void ImportExternalSpamFilters(struct Config *co)
{
  ENTER();

  // make sure that the spam folder really exists to be able
  // to move spam mails to it
  if(FO_GetFolderByType(FT_SPAM, NULL) != NULL)
  {
    struct FilterNode *filter;
    struct FilterNode *succ;

    // remove previous volatile filters first
    SafeIterateList(&co->filterList, struct FilterNode *, filter, succ)
    {
      if(filter->isVolatile == TRUE)
      {
        Remove((struct Node *)filter);
        DeleteFilterNode(filter);
      }
    }

    if(co->SpamTrustExternalFilter == TRUE && co->SpamExternalFilter[0] != '\0')
    {
      char externalPath[SIZE_PATHFILE];

      // now import the filters from the given external description
      snprintf(externalPath, sizeof(externalPath), "PROGDIR:Resources/spamfilters/%s.sfd", co->SpamExternalFilter);
      ImportFilter(externalPath, TRUE, &co->filterList);
    }
  }

  LEAVE();
}

///
/// GhostOutFilter
//  Enables/disables GUI gadgets in filter form
void GhostOutFilter(struct CO_GUIData *gui, struct FilterNode *filter)
{
  BOOL isremote;
  LONG pos = MUIV_NList_GetPos_Start;

  ENTER();

  isremote = (filter != NULL) ? filter->remote : FALSE;

  set(gui->ST_RNAME,             MUIA_Disabled, filter == NULL);
  set(gui->CH_REMOTE,            MUIA_Disabled, filter == NULL);
  set(gui->CH_APPLYNEW,          MUIA_Disabled, filter == NULL || isremote);
  set(gui->CH_APPLYREQ,          MUIA_Disabled, filter == NULL || isremote);
  set(gui->CH_APPLYSENT,         MUIA_Disabled, filter == NULL || isremote);
  set(gui->CH_AREDIRECT,         MUIA_Disabled, filter == NULL || isremote);
  set(gui->CH_AFORWARD,          MUIA_Disabled, filter == NULL || isremote);
  set(gui->CH_ARESPONSE,         MUIA_Disabled, filter == NULL || isremote);
  set(gui->CH_AEXECUTE,          MUIA_Disabled, filter == NULL);
  set(gui->CH_APLAY,             MUIA_Disabled, filter == NULL);
  set(gui->CH_AMOVE,             MUIA_Disabled, filter == NULL || isremote);
  set(gui->CH_ASTATUSTOMARKED,   MUIA_Disabled, filter == NULL || isremote);
  set(gui->CH_ASTATUSTOUNMARKED, MUIA_Disabled, filter == NULL || isremote);
  set(gui->CH_ASTATUSTOREAD,     MUIA_Disabled, filter == NULL || isremote);
  set(gui->CH_ASTATUSTOUNREAD,   MUIA_Disabled, filter == NULL || isremote);
  set(gui->CH_ASTATUSTOSPAM,     MUIA_Disabled, filter == NULL || isremote);
  set(gui->CH_ASTATUSTOHAM,      MUIA_Disabled, filter == NULL || isremote);
  set(gui->CH_ADELETE,           MUIA_Disabled, filter == NULL);
  set(gui->CH_ASKIP,             MUIA_Disabled, filter == NULL || !isremote);
  set(gui->CH_ATERMINATE,        MUIA_Disabled, filter == NULL);
  set(gui->ST_AREDIRECT,         MUIA_Disabled, filter == NULL || isremote || !xget(gui->CH_AREDIRECT, MUIA_Selected));
  set(gui->ST_AFORWARD,          MUIA_Disabled, filter == NULL || isremote || !xget(gui->CH_AFORWARD,  MUIA_Selected));
  set(gui->BT_APLAY,             MUIA_Disabled, filter == NULL || !xget(gui->CH_APLAY, MUIA_Selected));
  set(gui->PO_MOVETO,            MUIA_Disabled, filter == NULL || !xget(gui->CH_AMOVE, MUIA_Selected));
  set(gui->BT_RDEL,              MUIA_Disabled, filter == NULL);

  // lets make sure we ghost the filter up/down buttons if necessary
  if(filter != NULL)
    DoMethod(gui->LV_RULES, MUIM_NList_GetPos, filter, &pos);

  set(gui->BT_FILTERUP,   MUIA_Disabled, filter == NULL || pos == 0);
  set(gui->BT_FILTERDOWN, MUIA_Disabled, filter == NULL || pos+1 == (LONG)xget(gui->LV_RULES, MUIA_NList_Entries));

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
  if(filter != NULL)
  {
    struct RuleNode *rule;

    nnset(gui->ST_RNAME,             MUIA_String_Contents,   filter->name);
    nnset(gui->CH_REMOTE,            MUIA_Selected,          filter->remote);
    nnset(gui->CH_APPLYNEW,          MUIA_Selected,          filter->applyToNew);
    nnset(gui->CH_APPLYSENT,         MUIA_Selected,          filter->applyToSent);
    nnset(gui->CH_APPLYREQ,          MUIA_Selected,          filter->applyOnReq);
    nnset(gui->CY_FILTER_COMBINE,    MUIA_Cycle_Active,      filter->combine);
    nnset(gui->CH_AREDIRECT,         MUIA_Selected,          hasRedirectAction(filter));
    nnset(gui->CH_AFORWARD,          MUIA_Selected,          hasForwardAction(filter));
    nnset(gui->CH_ARESPONSE,         MUIA_Selected,          hasReplyAction(filter));
    nnset(gui->CH_AEXECUTE,          MUIA_Selected,          hasExecuteAction(filter));
    nnset(gui->CH_APLAY,             MUIA_Selected,          hasPlaySoundAction(filter));
    nnset(gui->CH_AMOVE,             MUIA_Selected,          hasMoveAction(filter));
    nnset(gui->CH_ASTATUSTOMARKED,   MUIA_Selected,          hasStatusToMarkedAction(filter));
    nnset(gui->CH_ASTATUSTOUNMARKED, MUIA_Selected,          hasStatusToUnmarkedAction(filter));
    nnset(gui->CH_ASTATUSTOREAD,     MUIA_Selected,          hasStatusToReadAction(filter));
    nnset(gui->CH_ASTATUSTOUNREAD,   MUIA_Selected,          hasStatusToUnreadAction(filter));
    nnset(gui->CH_ASTATUSTOSPAM,     MUIA_Selected,          hasStatusToSpamAction(filter));
    nnset(gui->CH_ASTATUSTOHAM,      MUIA_Selected,          hasStatusToHamAction(filter));
    nnset(gui->CH_ADELETE,           MUIA_Selected,          hasDeleteAction(filter));
    nnset(gui->CH_ASKIP,             MUIA_Selected,          hasSkipMsgAction(filter));
    nnset(gui->CH_ATERMINATE,        MUIA_Selected,          hasTerminateAction(filter));
    nnset(gui->ST_AREDIRECT,         MUIA_String_Contents,   filter->redirectTo);
    nnset(gui->ST_AFORWARD,          MUIA_String_Contents,   filter->forwardTo);
    nnset(gui->ST_ARESPONSE,         MUIA_String_Contents,   filter->replyFile);
    nnset(gui->ST_AEXECUTE,          MUIA_String_Contents,   filter->executeCmd);
    nnset(gui->ST_APLAY,             MUIA_String_Contents,   filter->playSound);
    nnset(gui->TX_MOVETO,            MUIA_Text_Contents,     filter->moveTo);

    set(gui->GR_SGROUP, MUIA_ObjectList_Quiet, TRUE);

    // before we actually set our rule options we have to clear out
    // all previous existing group childs
    DoMethod(gui->GR_SGROUP, MUIM_ObjectList_Clear);

    // Now we should have a clean SGROUP and can populate with new SearchControlGroup objects
    IterateList(&filter->ruleList, struct RuleNode *, rule)
    {
      Object *newSearchGroup = SearchControlGroupObject,
                                 MUIA_SearchControlGroup_RemoteFilterMode, filter->remote,
                               End;

      if(newSearchGroup == NULL)
        break;

      // fill the new search group with some content
      DoMethod(newSearchGroup, MUIM_SearchControlGroup_GetFromRule, rule);

      // set some notifies
      DoMethod(newSearchGroup, MUIM_Notify, MUIA_SearchControlGroup_Modified, MUIV_EveryTime, MUIV_Notify_Application, 2, MUIM_CallHook, &SetActiveFilterDataHook);

      // add it to our searchGroupList
      DoMethod(gui->GR_SGROUP, MUIM_ObjectList_AddItem, newSearchGroup);
    }
    set(gui->GR_SGROUP, MUIA_ObjectList_Quiet, FALSE);
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
    Object *ruleItem;
    Object *ruleState;
    int i;
    int rm = GetMUICheck(gui->CH_REMOTE);

    GetMUIString(filter->name, gui->ST_RNAME, sizeof(filter->name));
    filter->remote = (rm == 1);
    filter->applyToNew  = GetMUICheck(gui->CH_APPLYNEW);
    filter->applyToSent = GetMUICheck(gui->CH_APPLYSENT);
    filter->applyOnReq  = GetMUICheck(gui->CH_APPLYREQ);
    filter->combine = GetMUICycle(gui->CY_FILTER_COMBINE);
    filter->actions = 0;
    if(GetMUICheck(gui->CH_AREDIRECT))         setFlag(filter->actions, FA_REDIRECT);
    if(GetMUICheck(gui->CH_AFORWARD))          setFlag(filter->actions, FA_FORWARD);
    if(GetMUICheck(gui->CH_ARESPONSE))         setFlag(filter->actions, FA_REPLY);
    if(GetMUICheck(gui->CH_AEXECUTE))          setFlag(filter->actions, FA_EXECUTE);
    if(GetMUICheck(gui->CH_APLAY))             setFlag(filter->actions, FA_PLAYSOUND);
    if(GetMUICheck(gui->CH_AMOVE))             setFlag(filter->actions, FA_MOVE);
    if(GetMUICheck(gui->CH_ASTATUSTOMARKED))   setFlag(filter->actions, FA_STATUSTOMARKED);
    if(GetMUICheck(gui->CH_ASTATUSTOUNMARKED)) setFlag(filter->actions, FA_STATUSTOUNMARKED);
    if(GetMUICheck(gui->CH_ASTATUSTOREAD))     setFlag(filter->actions, FA_STATUSTOREAD);
    if(GetMUICheck(gui->CH_ASTATUSTOUNREAD))   setFlag(filter->actions, FA_STATUSTOUNREAD);
    if(GetMUICheck(gui->CH_ASTATUSTOSPAM))     setFlag(filter->actions, FA_STATUSTOSPAM);
    if(GetMUICheck(gui->CH_ASTATUSTOHAM))      setFlag(filter->actions, FA_STATUSTOHAM);
    if(GetMUICheck(gui->CH_ADELETE))           setFlag(filter->actions, FA_DELETE);
    if(GetMUICheck(gui->CH_ASKIP))             setFlag(filter->actions, FA_SKIPMSG);
    if(GetMUICheck(gui->CH_ATERMINATE))        setFlag(filter->actions, FA_TERMINATE);
    GetMUIString(filter->redirectTo, gui->ST_AREDIRECT, sizeof(filter->redirectTo));
    GetMUIString(filter->forwardTo,  gui->ST_AFORWARD, sizeof(filter->forwardTo));
    GetMUIString(filter->replyFile,  gui->ST_ARESPONSE, sizeof(filter->replyFile));
    GetMUIString(filter->executeCmd, gui->ST_AEXECUTE, sizeof(filter->executeCmd));
    GetMUIString(filter->playSound,  gui->ST_APLAY, sizeof(filter->playSound));
    GetMUIText(filter->moveTo, gui->TX_MOVETO, sizeof(filter->moveTo));

    // make sure to update all rule settings
    ruleState = NULL;
    i = 0;
    while((ruleItem = (Object *)DoMethod(gui->GR_SGROUP, MUIM_ObjectList_IterateItems, &ruleState)) != NULL)
    {
      struct RuleNode *rule;

      // get the rule out of the ruleList or create a new one
      while((rule = GetFilterRule(filter, i)) == NULL)
        CreateNewRule(filter, 0);

      // set the rule settings
      DoMethod(ruleItem, MUIM_SearchControlGroup_SetToRule, rule);

      ++i;
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
  Object *ruleItem;
  Object *ruleState;

  ruleState = NULL;
  while((ruleItem = (Object *)DoMethod(G->CO->GUI.GR_SGROUP, MUIM_ObjectList_IterateItems, &ruleState)) != NULL)
  {
    set(ruleItem, MUIA_SearchControlGroup_RemoteFilterMode, rm);
  }

  SetActiveFilterData();
}
MakeHook(CO_RemoteToggleHook,CO_RemoteToggleFunc);

///

/**** ARexx Hooks ****/
/// CO_IsValid
//  Verifies if the required settings have been made
BOOL CO_IsValid(void)
{
  BOOL valid = TRUE;

  ENTER();

  if(G->CO_Valid == FALSE)
  {
    valid = FALSE;

    if(G->CO != NULL)
      set(G->CO->GUI.WI, MUIA_Window_Open,TRUE);
    else
      DoMethod(G->App, MUIM_CallHook, &CO_OpenHook);

    MUI_Request(G->App, G->MA->GUI.WI, MUIF_NONE, NULL, tr(MSG_OkayReq), tr(MSG_CO_InvalidConf));
  }

  RETURN(valid);
  return valid;
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

  if(FileExists(AddPath(fname, co->PGPCmdPath, "pgpe", sizeof(fname))) == TRUE)
  {
    version = 5;

    D(DBF_STARTUP, "found PGP version 5 installed in '%s'", co->PGPCmdPath);
  }
  else if(FileExists(AddPath(fname, co->PGPCmdPath, "pgp", sizeof(fname))) == TRUE)
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
  ENTER();

  SHOWVALUE(DBF_CONFIG, co);

  // we have to free the signatureList
  FreeSignatureList(&co->signatureList);

  // we have to free the userIdentityList
  FreeUserIdentityList(&co->userIdentityList);

  // we have to free the pop3ServerList and smtpServerList
  FreeMailServerList(&co->pop3ServerList);
  FreeMailServerList(&co->smtpServerList);

  // we have to free the mimeTypeList
  FreeMimeTypeList(&co->mimeTypeList);

  // we have to free the filterList
  FreeFilterList(&co->filterList);

  // clear the config
  memset(co, 0, sizeof(*co));

  LEAVE();
}

///
/// CO_SetDefaults
//  Sets configuration (or a part of it) to the factory settings
void CO_SetDefaults(struct Config *co, enum ConfigPage page)
{
  struct codeset *sysCodeset;
  char sysCodesetName[SIZE_CTYPE+1];

  ENTER();
  SHOWVALUE(DBF_CONFIG, co);
  SHOWVALUE(DBF_CONFIG, page);

  // get the syscodeset to that we can set it as the default
  sysCodeset = CodesetsFindA(NULL, NULL);
  if(sysCodeset != NULL)
    strlcpy(sysCodesetName, sysCodeset->name, sizeof(sysCodesetName));
  else
    strlcpy(sysCodesetName, "ISO-8859-1", sizeof(sysCodesetName));

  if(page == cp_FirstSteps || page == cp_AllPages)
  {
    // check if the Location is setup correctly and if not
    // we use GuessTZone() to actually get an almost matching Location
    // definition or we set the Location to a default in the catalog
    if(G->Locale != NULL)
    {
      LONG gmtOffset = -(G->Locale->loc_GMTOffset);

      D(DBF_CONFIG, "got GMT offset %ld from locale.library", gmtOffset);

      strlcpy(co->Location, GuessTZone(gmtOffset), sizeof(co->Location));
    }
    else
      strlcpy(co->Location, tr(MSG_CO_FALLBACK_TZONE), sizeof(co->Location));

    strlcpy(co->DefaultLocalCodeset, sysCodesetName, sizeof(co->DefaultLocalCodeset));
  }

  if(page == cp_TCPIP || page == cp_AllPages)
  {
    // we have to free the pop3ServerList and smtpServerList
    FreeMailServerList(&co->pop3ServerList);
    FreeMailServerList(&co->smtpServerList);

    // fill the mailserver list with an empty POP3 and SMTP Server
    AddTail((struct List *)&co->smtpServerList, (struct Node *)CreateNewMailServer(MST_SMTP, co, TRUE));
    AddTail((struct List *)&co->pop3ServerList, (struct Node *)CreateNewMailServer(MST_POP3, co, TRUE));
  }

  if(page == cp_Identities || page == cp_AllPages)
  {
    // we have to free the userIdentityList
    FreeUserIdentityList(&co->userIdentityList);

    // fill the user identity list with an empty entry
    AddTail((struct List *)&co->userIdentityList, (struct Node *)CreateNewUserIdentity(co));
  }

  if(page == cp_Filters || page == cp_AllPages)
  {
    FreeFilterList(&co->filterList);
  }

  if(page == cp_Spam || page == cp_AllPages)
  {
    co->SpamFilterEnabled = TRUE;
    co->SpamFilterForNewMail = TRUE;
    co->SpamMarkOnMove = TRUE;
    co->SpamMarkAsRead = FALSE;
    co->SpamAddressBookIsWhiteList = TRUE;
    co->MoveHamToIncoming = TRUE;
    co->FilterHam = TRUE;
    co->SpamTrustExternalFilter = TRUE;
    strlcpy(co->SpamExternalFilter, "SpamAssassin", sizeof(co->SpamExternalFilter));
    co->SpamProbabilityThreshold = DEFAULT_SPAM_PROBABILITY_THRESHOLD;
    co->SpamFlushTrainingDataInterval = DEFAULT_FLUSH_TRAINING_DATA_INTERVAL;
    co->SpamFlushTrainingDataThreshold = DEFAULT_FLUSH_TRAINING_DATA_THRESHOLD;
  }

  if(page == cp_Read || page == cp_AllPages)
  {
    co->ShowHeader = 1;
    strlcpy(co->ShortHeaders, "(From|To|Cc|BCC|Date|Subject|Resent-#?)", sizeof(co->ShortHeaders));
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
    co->StatusChangeDelay = 1000; // 1s=1000ms delay by default
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
    co->ShowRcptFieldCC = TRUE;
    co->ShowRcptFieldBCC = FALSE;
    co->ShowRcptFieldReplyTo = FALSE;
    strlcpy(co->DefaultWriteCodeset, sysCodesetName, sizeof(co->DefaultWriteCodeset));
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

    co->QuoteEmptyLines = TRUE;
    co->CompareAddress = TRUE;
    co->StripSignature = TRUE;
    co->ForwardMode = FWM_ATTACH;
  }

  if(page == cp_Signature || page == cp_AllPages)
  {
    AddPath(co->TagsFile, G->ProgDir, ".taglines", sizeof(co->TagsFile));
    strlcpy(co->TagsSeparator, "%%", sizeof(co->TagsSeparator));

    // we have to free the signatureList
    FreeSignatureList(&co->signatureList);
  }

  if(page == cp_Security || page == cp_AllPages)
  {
    // we first try to see if there is a PGPPATH variable and if
    // so we take that one as the path to PGP or we plainly take PGP:
    // as the default path.
    if(GetVar("PGPPATH", co->PGPCmdPath, sizeof(co->PGPCmdPath), 0) == -1)
      strlcpy(co->PGPCmdPath, "PGP:", sizeof(co->PGPCmdPath));

    co->LogAllEvents = TRUE;
    co->PGPPassInterval = 10; // 10 min per default
    strlcpy(co->LogfilePath, G->ProgDir, sizeof(co->LogfilePath));
    co->LogfileMode = LF_NONE; // we log nothing per default
    co->SplitLogfile = FALSE;
  }

  if(page == cp_StartupQuit || page == cp_AllPages)
  {
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
    AddPath(co->GalleryDir, G->ProgDir, "Resources/Gallery", sizeof(co->GalleryDir));
    strlcpy(co->NewAddrGroup, "NEW", sizeof(co->NewAddrGroup));
    co->AddToAddrbook = 0;
    co->AddrbookCols = 1+2+4;
  }

  if(page == cp_Scripts || page == cp_AllPages)
  {
    int i;

    for(i = 0; i < MAXRX; i++)
    {
      co->RX[i].Name[0] = '\0';
      co->RX[i].Script[0] = '\0';
      co->RX[i].IsAmigaDOS = co->RX[i].UseConsole = FALSE;
      co->RX[i].WaitTerm = TRUE;
    }
  }

  if(page == cp_Mixed || page == cp_AllPages)
  {
    strlcpy(co->TempDir, "T:", sizeof(co->TempDir));
    strlcpy(co->DetachDir, "RAM:", sizeof(co->DetachDir));
    strlcpy(co->AttachDir, "RAM:", sizeof(co->AttachDir));
    strlcpy(co->PackerCommand, "LhA -a -m -i%l a \"%a.lha\"", sizeof(co->PackerCommand));
    co->ShowPackerProgress = FALSE;
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
    co->TransferWindow = TWM_AUTO;
    strlcpy(co->ForcedEditorCodeset, sysCodesetName, sizeof(co->ForcedEditorCodeset));

    // depending on the operating system we set the AppIcon
    // and docky icon defaults different
    #if defined(__amigaos4__)
    if(ApplicationBase != NULL)
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
    co->InfoBarPos = IB_POS_CENTER;
    strlcpy(co->InfoBarText, tr(MSG_CO_InfoBarDef), sizeof(co->InfoBarText));
    co->QuickSearchBarPos = QSB_POS_TOP;
    co->EmbeddedReadPane = TRUE;
    co->SizeFormat = SF_MIXED;
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

  if(page == cp_Update || page == cp_AllPages)
  {
    co->UpdateInterval = 604800; // check weekly for updates per default
    strlcpy(co->UpdateDownloadPath, "T:", sizeof(co->UpdateDownloadPath));
    SetDefaultUpdateState();
  }

  // everything else
  if(page == cp_AllPages)
  {
    co->LetterPart = 1;
    co->WriteIndexes = 120; // 2 minutes
    co->ExpungeIndexes = 600; // 10 minutes
    strlcpy(co->SupportSite, "http://yam.ch/", sizeof(co->SupportSite));
    strlcpy(co->UpdateServer, "http://update.yam.ch/", sizeof(co->UpdateServer));
    co->JumpToNewMsg = TRUE;
    co->JumpToIncoming = FALSE;
    co->JumpToRecentMsg = FALSE;
    co->AskJumpUnread = TRUE;
    co->PrinterCheck = TRUE;
    co->IsOnlineCheck = TRUE;
    co->ConfirmOnQuit = FALSE;
    co->HideGUIElements = 0;
    co->SysCharsetCheck = TRUE;
    co->AmiSSLCheck = TRUE;
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
    co->ShowFilterStats = TRUE;
    co->ConfirmRemoveAttachments = TRUE;

    // set the default styles of the folder listtree and
    // mail list items.
    strlcpy(co->StyleFGroupUnread, MUIX_B MUIX_I,         sizeof(co->StyleFGroupUnread));
    strlcpy(co->StyleFGroupRead,   MUIX_B MUIX_I "\0334", sizeof(co->StyleFGroupRead));
    strlcpy(co->StyleFolderUnread, MUIX_B        "\0334", sizeof(co->StyleFolderUnread));
    strlcpy(co->StyleFolderRead,   "",                    sizeof(co->StyleFolderRead));
    strlcpy(co->StyleFolderNew,    MUIX_B,                sizeof(co->StyleFolderNew));
    strlcpy(co->StyleMailUnread,   MUIX_B,                sizeof(co->StyleMailUnread));
    strlcpy(co->StyleMailRead,     "",                    sizeof(co->StyleMailRead));

    // check birthdays at 00:00 AM
    co->BirthdayCheckTime.ds_Days = 0;
    co->BirthdayCheckTime.ds_Minute = 0;
    co->BirthdayCheckTime.ds_Tick = 0;

    // default SSL ciphers to use
    strlcpy(co->DefaultSSLCiphers, "ALL:!LOW:!SSLv2:!EXP:!aNULL:@STRENGTH", sizeof(co->DefaultSSLCiphers));

    // default MachineFQDN is empty which means we try to identify it during runtime
    co->MachineFQDN[0] = '\0';
  }

  LEAVE();
}

///
/// CopyConfigData
//  Copies a configuration structure (deep copy)
static BOOL CopyConfigData(struct Config *dco, const struct Config *sco)
{
  BOOL success = TRUE;

  ENTER();
  SHOWVALUE(DBF_CONFIG, sco);
  SHOWVALUE(DBF_CONFIG, dco);

  // first we copy all raw data via memcpy
  memcpy(dco, sco, sizeof(struct Config));

  // then we have to do a deep copy and allocate separate memory for our copy
  NewMinList(&dco->pop3ServerList);

  if(success == TRUE)
  {
    struct MailServerNode *srcNode;

    IterateList(&sco->pop3ServerList, struct MailServerNode *, srcNode)
    {
      struct MailServerNode *dstNode;

      // clone the server but give the clone its own private data
      if((dstNode = CloneMailServer(srcNode)) != NULL)
      {
        AddTail((struct List *)&dco->pop3ServerList, (struct Node *)dstNode);
      }
      else
      {
        success = FALSE;

        // bail out, no need to copy further data
        break;
      }
    }
  }

  // then we have to do a deep copy and allocate separate memory for our copy
  NewMinList(&dco->smtpServerList);

  if(success == TRUE)
  {
    struct MailServerNode *srcNode;

    IterateList(&sco->smtpServerList, struct MailServerNode *, srcNode)
    {
      struct MailServerNode *dstNode;

      // clone the server but give the clone its own private data
      if((dstNode = CloneMailServer(srcNode)) != NULL)
      {
        AddTail((struct List *)&dco->smtpServerList, (struct Node *)dstNode);
      }
      else
      {
        success = FALSE;

        // bail out, no need to copy further data
        break;
      }
    }
  }

  // for copying the signature list we have to do a deep copy of the list
  NewMinList(&dco->signatureList);

  if(success == TRUE)
  {
    struct SignatureNode *srcNode;

    IterateList(&sco->signatureList, struct SignatureNode *, srcNode)
    {
      struct SignatureNode *dstNode;

      if((dstNode = DuplicateNode(srcNode, sizeof(*srcNode))) != NULL)
      {
        if(srcNode->signature != NULL)
          dstNode->signature = strdup(srcNode->signature);
        AddTail((struct List *)&dco->signatureList, (struct Node *)dstNode);
      }
      else
      {
        success = FALSE;
        // bail out, no need to copy further data
        break;
      }
    }
  }

  // for copying the user identity list we have to do a deep copy of the list
  NewMinList(&dco->userIdentityList);

  if(success == TRUE)
  {
    struct UserIdentityNode *srcNode;

    IterateList(&sco->userIdentityList, struct UserIdentityNode *, srcNode)
    {
      struct UserIdentityNode *dstNode;

      if((dstNode = DuplicateNode(srcNode, sizeof(*srcNode))) != NULL)
      {
        // make sure the mailserver of the copied node points to an
        // entry of the copied mail server list
        if(srcNode->smtpServer != NULL)
          dstNode->smtpServer = FindMailServer(&dco->smtpServerList, srcNode->smtpServer->id);
        else
          dstNode->smtpServer = NULL;

        // make sure the signature of the copied node points to an
        // entry of the copied signature list
        if(srcNode->signature != NULL)
          dstNode->signature = FindSignatureByID(&dco->signatureList, srcNode->signature->id);
        else
          dstNode->signature = NULL;

        dstNode->sentMailList = NULL;

        AddTail((struct List *)&dco->userIdentityList, (struct Node *)dstNode);
      }
      else
      {
        success = FALSE;
        // bail out, no need to copy further data
        break;
      }
    }
  }

  // for copying the mimetype list we have to do a deep copy of the list
  NewMinList(&dco->mimeTypeList);

  if(success == TRUE)
  {
    struct MimeTypeNode *srcNode;

    IterateList(&sco->mimeTypeList, struct MimeTypeNode *, srcNode)
    {
      struct MimeTypeNode *dstNode;

      if((dstNode = DuplicateNode(srcNode, sizeof(*srcNode))) != NULL)
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
  NewMinList(&dco->filterList);

  if(success == TRUE)
  {
    struct FilterNode *srcFilter;

    IterateList(&sco->filterList, struct FilterNode *, srcFilter)
    {
      struct FilterNode *dstFilter;

      if((dstFilter = AllocSysObjectTags(ASOT_NODE,
        ASONODE_Size, sizeof(*dstFilter),
        ASONODE_Min, TRUE,
        TAG_DONE)) != NULL)
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

  // free the copied configuration in case anything failed
  if(success == FALSE)
    CO_ClearConfig(dco);

  // return if everything could be duplicated successfully
  RETURN(success);
  return success;
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
  if(c1->ShowHeader                      == c2->ShowHeader &&
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
     c1->ExpungeIndexes                  == c2->ExpungeIndexes &&
     c1->AutoSave                        == c2->AutoSave &&
     c1->ShowRcptFieldCC                 == c2->ShowRcptFieldCC &&
     c1->ShowRcptFieldBCC                == c2->ShowRcptFieldBCC &&
     c1->ShowRcptFieldReplyTo            == c2->ShowRcptFieldReplyTo &&
     c1->HideGUIElements                 == c2->HideGUIElements &&
     c1->StackSize                       == c2->StackSize &&
     c1->SizeFormat                      == c2->SizeFormat &&
     c1->EmailCache                      == c2->EmailCache &&
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
     c1->LogfileMode                     == c2->LogfileMode &&
     c1->MDN_NoRecipient                 == c2->MDN_NoRecipient &&
     c1->MDN_NoDomain                    == c2->MDN_NoDomain &&
     c1->MDN_OnDelete                    == c2->MDN_OnDelete &&
     c1->MDN_Other                       == c2->MDN_Other &&
     c1->DSListFormat                    == c2->DSListFormat &&
     c1->SigSepLine                      == c2->SigSepLine &&
     c1->TransferWindow                  == c2->TransferWindow &&
     c1->FolderInfoMode                  == c2->FolderInfoMode &&
     c1->ForwardMode                     == c2->ForwardMode &&
     c1->InfoBarPos                      == c2->InfoBarPos &&
     c1->QuickSearchBarPos               == c2->QuickSearchBarPos &&
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
     c1->QuoteEmptyLines                 == c2->QuoteEmptyLines &&
     c1->CompareAddress                  == c2->CompareAddress &&
     c1->StripSignature                  == c2->StripSignature &&
     c1->FixedFontList                   == c2->FixedFontList &&
     c1->SplitLogfile                    == c2->SplitLogfile &&
     c1->LogAllEvents                    == c2->LogAllEvents &&
     c1->SendOnStartup                   == c2->SendOnStartup &&
     c1->CleanupOnStartup                == c2->CleanupOnStartup &&
     c1->RemoveOnStartup                 == c2->RemoveOnStartup &&
     c1->LoadAllFolders                  == c2->LoadAllFolders &&
     c1->UpdateNewMail                   == c2->UpdateNewMail &&
     c1->CheckBirthdates                 == c2->CheckBirthdates &&
     c1->SendOnQuit                      == c2->SendOnQuit &&
     c1->CleanupOnQuit                   == c2->CleanupOnQuit &&
     c1->RemoveOnQuit                    == c2->RemoveOnQuit &&
     c1->IconifyOnQuit                   == c2->IconifyOnQuit &&
     c1->Confirm                         == c2->Confirm &&
     c1->RemoveAtOnce                    == c2->RemoveAtOnce &&
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
     c1->WBAppIcon                       == c2->WBAppIcon &&
     c1->DockyIcon                       == c2->DockyIcon &&
     c1->AmiSSLCheck                     == c2->AmiSSLCheck &&
     c1->DetectCyrillic                  == c2->DetectCyrillic &&
     c1->ABookLookup                     == c2->ABookLookup &&
     c1->ConvertHTML                     == c2->ConvertHTML &&
     c1->SpamFilterEnabled               == c2->SpamFilterEnabled &&
     c1->SpamFilterForNewMail            == c2->SpamFilterForNewMail &&
     c1->SpamMarkOnMove                  == c2->SpamMarkOnMove &&
     c1->SpamMarkAsRead                  == c2->SpamMarkAsRead &&
     c1->SpamAddressBookIsWhiteList      == c2->SpamAddressBookIsWhiteList &&
     c1->MoveHamToIncoming               == c2->MoveHamToIncoming &&
     c1->FilterHam                       == c2->FilterHam &&
     c1->SpamTrustExternalFilter         == c2->SpamTrustExternalFilter &&
     c1->DisplayAllAltPart               == c2->DisplayAllAltPart &&
     c1->MDNEnabled                      == c2->MDNEnabled &&
     c1->AutoClip                        == c2->AutoClip &&
     c1->FolderDoubleClick               == c2->FolderDoubleClick &&
     c1->MapForeignChars                 == c2->MapForeignChars &&
     c1->GlobalMailThreads               == c2->GlobalMailThreads &&
     c1->ShowFilterStats                 == c2->ShowFilterStats &&
     c1->ConfirmRemoveAttachments        == c2->ConfirmRemoveAttachments &&
     c1->OverrideFromAddress             == c2->OverrideFromAddress &&
     c1->ShowPackerProgress              == c2->ShowPackerProgress &&
     c1->ForceEditorCodeset              == c2->ForceEditorCodeset &&

     c1->SocketOptions.SendBuffer        == c2->SocketOptions.SendBuffer &&
     c1->SocketOptions.RecvBuffer        == c2->SocketOptions.RecvBuffer &&
     c1->SocketOptions.SendLowAt         == c2->SocketOptions.SendLowAt &&
     c1->SocketOptions.RecvLowAt         == c2->SocketOptions.RecvLowAt &&
     c1->SocketOptions.SendTimeOut       == c2->SocketOptions.SendTimeOut &&
     c1->SocketOptions.RecvTimeOut       == c2->SocketOptions.RecvTimeOut &&
     c1->SocketOptions.KeepAlive         == c2->SocketOptions.KeepAlive &&
     c1->SocketOptions.NoDelay           == c2->SocketOptions.NoDelay &&
     c1->SocketOptions.LowDelay          == c2->SocketOptions.LowDelay &&

     CompareMailServerLists(&c1->pop3ServerList, &c2->pop3ServerList) &&
     CompareMailServerLists(&c1->smtpServerList, &c2->smtpServerList) &&
     CompareUserIdentityLists(&c1->userIdentityList, &c2->userIdentityList) &&
     CompareSignatureLists(&c1->signatureList, &c2->signatureList) &&
     CompareFilterLists(&c1->filterList, &c2->filterList) &&
     CompareMimeTypeLists(&c1->mimeTypeList, &c2->mimeTypeList) &&
     CompareRxHooks((const struct RxHook *)c1->RX, (const struct RxHook *)c2->RX) &&

     strcmp(c1->Location,            c2->Location) == 0 &&
     strcmp(c1->ColoredText.buf,     c2->ColoredText.buf) == 0 &&
     strcmp(c1->Color1stLevel.buf,   c2->Color1stLevel.buf) == 0 &&
     strcmp(c1->Color2ndLevel.buf,   c2->Color2ndLevel.buf) == 0 &&
     strcmp(c1->Color3rdLevel.buf,   c2->Color3rdLevel.buf) == 0 &&
     strcmp(c1->Color4thLevel.buf,   c2->Color4thLevel.buf) == 0 &&
     strcmp(c1->ColorURL.buf,        c2->ColorURL.buf) == 0 &&
     strcmp(c1->ColorSignature.buf,  c2->ColorSignature.buf) == 0 &&
     strcmp(c1->ShortHeaders,        c2->ShortHeaders) == 0 &&
     strcmp(c1->NewIntro,            c2->NewIntro) == 0 &&
     strcmp(c1->Greetings,           c2->Greetings) == 0 &&
     strcmp(c1->Editor,              c2->Editor) == 0 &&
     strcmp(c1->ReplyHello,          c2->ReplyHello) == 0 &&
     strcmp(c1->ReplyIntro,          c2->ReplyIntro) == 0 &&
     strcmp(c1->ReplyBye,            c2->ReplyBye) == 0 &&
     strcmp(c1->AltReplyHello,       c2->AltReplyHello) == 0 &&
     strcmp(c1->AltReplyIntro,       c2->AltReplyIntro) == 0 &&
     strcmp(c1->AltReplyBye,         c2->AltReplyBye) == 0 &&
     strcmp(c1->AltReplyPattern,     c2->AltReplyPattern) == 0 &&
     strcmp(c1->MLReplyHello,        c2->MLReplyHello) == 0 &&
     strcmp(c1->MLReplyIntro,        c2->MLReplyIntro) == 0 &&
     strcmp(c1->MLReplyBye,          c2->MLReplyBye) == 0 &&
     strcmp(c1->ForwardIntro,        c2->ForwardIntro) == 0 &&
     strcmp(c1->ForwardFinish,       c2->ForwardFinish) == 0 &&
     strcmp(c1->TagsFile,            c2->TagsFile) == 0 &&
     strcmp(c1->TagsSeparator,       c2->TagsSeparator) == 0 &&
     strcmp(c1->PGPCmdPath,          c2->PGPCmdPath) == 0 &&
     strcmp(c1->LogfilePath,         c2->LogfilePath) == 0 &&
     strcmp(c1->DetachDir,           c2->DetachDir) == 0 &&
     strcmp(c1->AttachDir,           c2->AttachDir) == 0 &&
     strcmp(c1->GalleryDir,          c2->GalleryDir) == 0 &&
     strcmp(c1->NewAddrGroup,        c2->NewAddrGroup) == 0 &&
     strcmp(c1->ProxyServer,         c2->ProxyServer) == 0 &&
     strcmp(c1->TempDir,             c2->TempDir) == 0 &&
     strcmp(c1->PackerCommand,       c2->PackerCommand) == 0 &&
     strcmp(c1->XPKPack,             c2->XPKPack) == 0 &&
     strcmp(c1->XPKPackEncrypt,      c2->XPKPackEncrypt) == 0 &&
     strcmp(c1->SupportSite,         c2->SupportSite) == 0 &&
     strcmp(c1->UpdateServer,        c2->UpdateServer) == 0 &&
     strcmp(c1->DefaultLocalCodeset, c2->DefaultLocalCodeset) == 0 &&
     strcmp(c1->DefaultWriteCodeset, c2->DefaultWriteCodeset) == 0 &&
     strcmp(c1->ForcedEditorCodeset, c2->ForcedEditorCodeset) == 0 &&
     strcmp(c1->IOCInterfaces,       c2->IOCInterfaces) == 0 &&
     strcmp(c1->AppIconText,         c2->AppIconText) == 0 &&
     strcmp(c1->InfoBarText,         c2->InfoBarText) == 0 &&
     strcmp(c1->DefaultMimeViewer,   c2->DefaultMimeViewer) == 0 &&
     strcmp(c1->StyleFGroupUnread,   c2->StyleFGroupUnread) == 0 &&
     strcmp(c1->StyleFGroupRead,     c2->StyleFGroupRead) == 0 &&
     strcmp(c1->StyleFolderUnread,   c2->StyleFolderUnread) == 0 &&
     strcmp(c1->StyleFolderRead,     c2->StyleFolderRead) == 0 &&
     strcmp(c1->StyleFolderNew,      c2->StyleFolderNew) == 0 &&
     strcmp(c1->StyleMailUnread,     c2->StyleMailUnread) == 0 &&
     strcmp(c1->StyleMailRead,       c2->StyleMailRead) == 0 &&
     strcmp(c1->QuoteChar,           c2->QuoteChar) == 0 &&
     strcmp(c1->AltQuoteChar,        c2->AltQuoteChar) == 0 &&
     strcmp(c1->ThemeName,           c2->ThemeName) == 0 &&
     strcmp(c1->UpdateDownloadPath,  c2->UpdateDownloadPath) == 0 &&
     strcmp(c1->SpamExternalFilter,  c2->SpamExternalFilter) == 0)
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
  BOOL saveAtEnd = FALSE;
  BOOL updateReadWindows = FALSE;
  BOOL updateWriteWindows = FALSE;
  BOOL updateHeaderMode = FALSE;
  BOOL updateSenderInfo = FALSE;
  BOOL updateMenuShortcuts = FALSE;
  struct MailServerNode *firstPOP3;
  struct MailServerNode *firstSMTP;
  struct UserIdentityNode *firstIdentity;
  Object *refWindow;
  struct MailServerNode *msn;
  struct UserIdentityNode *uin;
  struct SignatureNode *sn;

  ENTER();

  // save a pointer to a reference window in case
  // we need to open a requester
  if(G->CO != NULL && G->CO->GUI.WI != NULL)
    refWindow = G->CO->GUI.WI;
  else if(G->MA != NULL && G->MA->GUI.WI != NULL)
    refWindow = G->MA->GUI.WI;
  else
    refWindow = NULL;

  // retrieve the first SMTP and POP3 server so that
  // we can synchronize their information
  firstPOP3 = GetMailServer(&co->pop3ServerList, 0);
  firstSMTP = GetMailServer(&co->smtpServerList, 0);

  // get the first user Identity
  firstIdentity = GetUserIdentity(&co->userIdentityList, 0, TRUE);

  if(firstPOP3 != NULL && firstSMTP != NULL && firstIdentity != NULL)
  {
    // now we walk through our POP3 server list and check and fix certains
    // things in it
    IterateList(&co->pop3ServerList, struct MailServerNode *, msn)
    {
      if(msn->hostname[0] == '\0')
        strlcpy(msn->hostname, firstSMTP->hostname, sizeof(msn->hostname));

      if(msn->port == 0)
        msn->port = 110;

      if(msn->username[0] == '\0')
      {
        char *p = strchr(firstIdentity->address, '@');
        strlcpy(msn->username, firstIdentity->address, p ? (unsigned int)(p - firstIdentity->address + 1) : sizeof(msn->username));
      }

      if(msn->description[0] == '\0')
        snprintf(msn->description, sizeof(msn->description), "%s@%s", msn->username, msn->hostname);
    }

    // now we walk through our SMTP server list and check and fix certains
    // things in it
    IterateList(&co->smtpServerList, struct MailServerNode *, msn)
    {
      if(msn->hostname[0] == '\0')
        strlcpy(msn->hostname, firstPOP3->hostname, sizeof(msn->hostname));

      if(msn->port == 0)
        msn->port = 25;

      if(msn->description[0] == '\0')
        snprintf(msn->description, sizeof(msn->description), "%s@%s", msn->username, msn->hostname);
    }
  }

  // check all servers for valid and unique IDs
  IterateList(&co->pop3ServerList, struct MailServerNode *, msn)
  {
    // check for a valid and unique ID, this is independend of the server type
    if(msn->id == 0)
    {
      int id;

      // loop until we generated a unique ID
      // usually this will happen with just one iteration
      do
      {
        id = rand();

        if(id == 0)
          continue;
      }
      while(IsUniqueMailServerID(&co->pop3ServerList, id) == FALSE);

      msn->id = id;

      saveAtEnd = TRUE;
    }
  }

  // check all servers for valid and unique IDs
  IterateList(&co->smtpServerList, struct MailServerNode *, msn)
  {
    // check for a valid and unique ID, this is independend of the server type
    if(msn->id == 0)
    {
      int id;

      // loop until we generated a unique ID
      // usually this will happen with just one iteration
      do
      {
        id = rand();

        if(id == 0)
          continue;
      }
      while(IsUniqueMailServerID(&co->smtpServerList, id) == FALSE);

      msn->id = id;

      saveAtEnd = TRUE;
    }
  }

  // check all identities for valid and unique IDs
  IterateList(&co->userIdentityList, struct UserIdentityNode *, uin)
  {
    // check for a valid and unique ID
    if(uin->id == 0)
    {
      int id;

      // loop until we generated a unique ID
      // usually this will happen with just one iteration
      do
      {
        id = rand();

        if(id == 0)
          continue;
      }
      while(FindUserIdentityByID(&co->userIdentityList, id) != NULL);

      uin->id = id;

      saveAtEnd = TRUE;
    }
  }

  // we make sure that the signature list is not empty and if
  // so that normally signals that the config didn't carry any
  // configuration items for signatures.
  // In that case we check if the old-style signature files
  // exist and if we we add these ones as the default ones
  if(IsMinListEmpty(&co->signatureList))
  {
    // before YAM 2.8 we had three default signatures. We therefore
    // check for these three files now.
    if((sn = CreateSignatureFromFile(".signature", tr(MSG_CO_DefSig))) != NULL)
      AddTail((struct List *)&co->signatureList, (struct Node *)sn);

    // check for ".altsignature1" file
    if((sn = CreateSignatureFromFile(".altsignature1", tr(MSG_CO_AltSig1))) != NULL)
      AddTail((struct List *)&co->signatureList, (struct Node *)sn);

    // check for ".altsignature2" file
    if((sn = CreateSignatureFromFile(".altsignature2", tr(MSG_CO_AltSig2))) != NULL)
      AddTail((struct List *)&co->signatureList, (struct Node *)sn);

    saveAtEnd = TRUE;
  }

  // check all signatures for valid and unique IDs
  IterateList(&co->signatureList, struct SignatureNode *, sn)
  {
    // check for a valid and unique ID
    if(sn->id == 0)
    {
      int id;

      // loop until we generated a unique ID
      // usually this will happen with just one iteration
      do
      {
        id = rand();

        if(id == 0)
          continue;
      }
      while(IsUniqueSignatureID(&co->signatureList, id) == FALSE);

      sn->id = id;

      saveAtEnd = TRUE;
    }
  }

  // update the write windows in any case
  updateWriteWindows = TRUE;

  // check if the Location is setup correctly and if not
  // we use GuessTZone() to actually get an almost matching Location
  // definition or we set the Location to a default in the catalog
  if(co->Location[0] == '\0')
  {
    if(G->Locale != NULL)
    {
      LONG gmtOffset = -(G->Locale->loc_GMTOffset);

      D(DBF_CONFIG, "got GMT offset %ld from locale.library", gmtOffset);

      strlcpy(co->Location, GuessTZone(gmtOffset), sizeof(co->Location));
    }
    else
      strlcpy(co->Location, tr(MSG_CO_FALLBACK_TZONE), sizeof(co->Location));
  }

  // now we have to make sure we set the Location global now
  SetTZone(co->Location);

  // check if PGP is available or not.
  G->PGPVersion = CO_DetectPGP(co);

  // prepare the temporary directory
  CreateDirectory(co->TempDir);

  // check if the current configuration is already valid at an absolute
  // minimum.
  G->CO_Valid = (firstIdentity != NULL &&
                 firstIdentity->address[0] != '\0' &&
                 firstIdentity->realname[0] != '\0' &&
                 firstSMTP->hostname[0] != '\0' &&
                 firstPOP3->hostname[0] != '\0');

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
      if(co->DefaultLocalCodeset[0] != '\0' && sysCodeset->name[0] != '\0')
      {
        if(stricmp(co->DefaultLocalCodeset, sysCodeset->name) != 0)
        {
          int res = MUI_Request(G->App, refWindow, MUIF_NONE,
                                tr(MSG_CO_CHARSETWARN_TITLE),
                                tr(MSG_CO_CHARSETWARN_BT),
                                tr(MSG_CO_CHARSETWARN),
                                co->DefaultLocalCodeset, sysCodeset->name);

          // if the user has clicked on Change, we do
          // change the charset and save it immediatly
          if(res == 1)
          {
            strlcpy(co->DefaultLocalCodeset, sysCodeset->name, sizeof(co->DefaultLocalCodeset));
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
        strlcpy(co->DefaultLocalCodeset, sysCodeset->name, sizeof(co->DefaultLocalCodeset));
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
  if(co->DefaultLocalCodeset[0] == '\0')
  {
    strlcpy(co->DefaultLocalCodeset, "ISO-8859-1", sizeof(co->DefaultLocalCodeset));
    saveAtEnd = TRUE;
  }

  if(co->DefaultWriteCodeset[0] == '\0')
  {
    strlcpy(co->DefaultWriteCodeset, "ISO-8859-1", sizeof(co->DefaultWriteCodeset));
    saveAtEnd = TRUE;
  }

  if(co->ForcedEditorCodeset[0] == '\0')
  {
    strlcpy(co->ForcedEditorCodeset, "ISO-8859-1", sizeof(co->ForcedEditorCodeset));
    saveAtEnd = TRUE;
  }

  // now we check if the set default read charset is a valid one also supported
  // by codesets.library and if not we warn the user
  if((G->localCodeset = CodesetsFind(co->DefaultLocalCodeset,
                                     CSA_CodesetList,       G->codesetsList,
                                     CSA_FallbackToDefault, FALSE,
                                    TAG_DONE)) == NULL)
  {
    int res = MUI_Request(G->App, refWindow, MUIF_NONE,
                          tr(MSG_CO_CHARSETWARN_TITLE),
                          tr(MSG_CO_CHARSETUNKNOWNWARN_BT),
                          tr(MSG_CO_CHARSETUNKNOWNWARN),
                          co->DefaultLocalCodeset);
    if(res == 1)
    {
      // fallback to the system's default codeset
      if((G->localCodeset = CodesetsFindA(NULL, NULL)) != NULL)
      {
        strlcpy(co->DefaultLocalCodeset, G->localCodeset->name, sizeof(co->DefaultLocalCodeset));
        saveAtEnd = TRUE;
      }
    }
  }

  // now we check if the set default write charset is a valid one also supported
  // by codesets.library and if not we warn the user
  if((G->writeCodeset = CodesetsFind(co->DefaultWriteCodeset,
                                     CSA_CodesetList,       G->codesetsList,
                                     CSA_FallbackToDefault, FALSE,
                                     TAG_DONE)) == NULL)
  {
    int res = MUI_Request(G->App, refWindow, MUIF_NONE,
                          tr(MSG_CO_CHARSETWARN_TITLE),
                          tr(MSG_CO_CHARSETUNKNOWNWARN_BT),
                          tr(MSG_CO_CHARSETUNKNOWNWARN),
                          co->DefaultWriteCodeset);
    if(res == 1)
    {
      // fallback to the system's default codeset
      if((G->writeCodeset = CodesetsFindA(NULL, NULL)) != NULL)
      {
        strlcpy(co->DefaultWriteCodeset, G->writeCodeset->name, sizeof(co->DefaultWriteCodeset));
        saveAtEnd = TRUE;
      }
    }
  }

  // now we check if the set force editor codeset is a valid one also supported
  // by codesets.library and if not we warn the user
  if(CodesetsFind(co->ForcedEditorCodeset,
                  CSA_CodesetList,       G->codesetsList,
                  CSA_FallbackToDefault, FALSE,
                  TAG_DONE) == NULL)
  {
    int res = MUI_Request(G->App, refWindow, MUIF_NONE,
                          tr(MSG_CO_CHARSETWARN_TITLE),
                          tr(MSG_CO_CHARSETUNKNOWNWARN_BT),
                          tr(MSG_CO_CHARSETUNKNOWNWARN),
                          co->ForcedEditorCodeset);
    if(res == 1)
    {
      struct codeset *tmpcs;

      // fallback to the system's default codeset
      if((tmpcs = CodesetsFindA(NULL, NULL)) != NULL)
      {
        strlcpy(co->ForcedEditorCodeset, tmpcs->name, sizeof(co->ForcedEditorCodeset));
        saveAtEnd = TRUE;
      }
    }
  }

  // we also check if AmiSSL was found installed or not. And in case the
  // AmiSSL warning is enabled we notify the user about a not running
  // amissl installation.
  if(co->AmiSSLCheck == TRUE)
  {
    if(AmiSSLMasterBase == NULL || AmiSSLBase == NULL || G->TR_UseableTLS == FALSE)
    {
      int res = MUI_Request(G->App, refWindow, MUIF_NONE,
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
    if(AmiSSLMasterBase != NULL && AmiSSLBase != NULL && G->TR_UseableTLS == TRUE)
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

    ImportExternalSpamFilters(co);
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

  // check for valid birthday check times
  if(co->BirthdayCheckTime.ds_Minute < 0 || co->BirthdayCheckTime.ds_Minute > 23*60+59)
  {
    co->BirthdayCheckTime.ds_Days = 0;
    co->BirthdayCheckTime.ds_Minute = 10*60;
    co->BirthdayCheckTime.ds_Tick = 0;
    saveAtEnd = TRUE;
  }

  if(update == TRUE && G->CO != NULL)
  {
    switch(G->CO->VisiblePage)
    {
      case cp_FirstSteps:
      {
        if((msn = GetMailServer(&co->pop3ServerList, 0)) != NULL)
        {
          setstring(G->CO->GUI.ST_POPHOST0, msn->hostname);
          setstring(G->CO->GUI.ST_USER0, msn->username);
        }
      }
      break;

      default:
      {
        // nothing
      }
      break;
    }

    if(G->CO->Visited[cp_FirstSteps] == TRUE || G->CO->UpdateAll == TRUE)
    {
      // make sure to redraw the main mail list in case the user
      // changed the timezone
      DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_NList_Redraw, MUIV_NList_Redraw_All);
    }

    if(G->CO->Visited[cp_TCPIP] == TRUE || G->CO->UpdateAll == TRUE)
    {
      DoMethod(G->MA->GUI.TO_TOOLBAR, MUIM_MainWindowToolbar_UpdateServerControls);

      // requeue the timerequest for the POP3 servers
      RestartPOP3Timers();
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
      RestartTimer(TIMER_AUTOSAVE, co->AutoSave, 0, FALSE);
    }

    if(G->CO->Visited[cp_ReplyForward] == TRUE || G->CO->UpdateAll == TRUE)
    {
      // update the "Forward" shortcuts of the read window's menu
      updateMenuShortcuts = TRUE;
    }

    if(G->CO->Visited[cp_AddressBook] == TRUE || G->CO->UpdateAll == TRUE)
    {
      DoMethod(G->AB->GUI.LV_ADDRESSES, MUIM_AddrBookListtree_MakeFormat);
    }

    if(G->CO->Visited[cp_LookFeel] == TRUE || G->CO->UpdateAll == TRUE)
    {
      // First we set the PG_MAILLIST and NL_FOLDER Quiet
      set(G->MA->GUI.PG_MAILLIST,MUIA_NList_Quiet,     TRUE);
      set(G->MA->GUI.NL_FOLDERS, MUIA_NListtree_Quiet, TRUE);

      // Now we reorder the Maingroup accordingly to the InfoBar/QuickSearchBar setting
      DoMethod(G->MA->GUI.WI, MUIM_MainWindow_Relayout);

      // Now we update the InfoBar because the text could have been changed
      DoMethod(G->MA->GUI.IB_INFOBAR, MUIM_InfoBar_SetFolder, GetCurrentFolder());

      // we signal the mainwindow that it may check whether to include the
      // embedded read pane part or not
      MA_SetupEmbeddedReadPane();

      // Modify the ContextMenu flags
      set(G->MA->GUI.PG_MAILLIST,MUIA_ContextMenu, C->MessageCntMenu ? MUIV_NList_ContextMenu_Always : MUIV_NList_ContextMenu_Never);
      set(G->MA->GUI.NL_FOLDERS, MUIA_ContextMenu, C->FolderCntMenu ? MUIV_NList_ContextMenu_Always : MUIV_NList_ContextMenu_Never);

      // Make sure to save the GUI layout before continuing
      SaveLayout(FALSE);

      // recreate the MUIA_NList_Format strings
      DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_MainFolderListtree_MakeFormat);
      DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_MainMailListGroup_MakeFormat);

      // now reload the layout
      LoadLayout();

      // Now we give the control back to the NLists
      set(G->MA->GUI.PG_MAILLIST,MUIA_NList_Quiet,     FALSE);
      set(G->MA->GUI.NL_FOLDERS, MUIA_NListtree_Quiet, FALSE);

      // and to not let the embedded read pane be empty when it is newly created
      // we have to make sure the actual selected mail is loaded
      if(C->EmbeddedReadPane == TRUE)
        MA_ChangeSelected(TRUE);
    }

    if(G->CO->Visited[cp_Mixed] == TRUE || G->CO->UpdateAll == TRUE)
    {
      // setup the appIcon positions and display all statistics
      // accordingly.
      DisplayStatistics((struct Folder *)-1, TRUE);

      // in case the Docky icon was just enabled we must register YAM again
      // as an application.lib aware program, because application.lib seems
      // to be a bit buggy when it should change a not yet existing icon to
      // a custom one. Removing the Docky icon in case it was disabled is no
      // problem at all.
      // Don't get confused by the C and CE pointers. These have been swapped
      // before, thus C points to the current configuration while CE points
      // to the old configuration.
      if((C->DockyIcon == TRUE  && CE->DockyIcon == FALSE) ||
         (C->DockyIcon == FALSE && CE->DockyIcon == TRUE))
      {
        FreeDockyIcon();
        InitDockyIcon();
      }
      UpdateDockyIcon();
    }

    if(G->CO->Visited[cp_Update] == TRUE || G->CO->UpdateAll == TRUE)
    {
      // make sure we reinit the update check timer
      InitUpdateCheck(FALSE);
    }

    // make sure the dynamic menus and some menu shortcuts of the main window
    // are properly refreshed.
    MA_SetupDynamicMenus();

    // update the embedded identity and signature pointers of all folders
    UpdateAllFolderSettings(co);
  }

  // if some items have modified the config we do save it again.
  if(saveAtEnd == TRUE)
    CO_SaveConfig(co, G->CO_PrefsFile);

  // update possibly open read windows
  if(updateReadWindows == TRUE || updateHeaderMode == TRUE || updateSenderInfo == TRUE || updateMenuShortcuts == TRUE)
  {
    struct ReadMailData *rmData;

    IterateList(&G->readMailDataList, struct ReadMailData *, rmData)
    {
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
        else if(rmData->readWindow != NULL && (updateReadWindows == TRUE || updateMenuShortcuts == TRUE))
        {
          // forward the modifed information to the window, because a read mail group has no toolbar
          if(updateReadWindows == TRUE)
            DoMethod(G->App, MUIM_Application_PushMethod, rmData->readWindow, 2, MUIM_ReadWindow_ReadMail, rmData->mail);

          if(updateMenuShortcuts == TRUE)
            DoMethod(rmData->readWindow, MUIM_ReadWindow_UpdateMenuShortcuts);
        }
      }
    }
  }

  // update possibly open write windows
  if(updateWriteWindows == TRUE)
  {
    struct WriteMailData *wmData;

    IterateList(&G->writeMailDataList, struct WriteMailData *, wmData)
    {
      DoMethod(wmData->window, MUIM_WriteWindow_UpdateIdentities);
      DoMethod(wmData->window, MUIM_WriteWindow_UpdateSignatures);
    }
  }

  LEAVE();
}

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
    if(CO_LoadConfig(CE, cname, NULL) == 1)
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
       MUI_Request(G->App, G->CO->GUI.WI, MUIF_NONE, tr(MSG_MA_ConfirmReq), tr(MSG_YesNoReq), tr(MSG_FILE_OVERWRITE), frc->file) != 0)
    {
      CO_GetConfig();
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
    CO_GetConfig();

    G->CO->VisiblePage = page;
    G->CO->Visited[page] = TRUE;

    #if defined(__amigaos3__)
    // The follow lines are a workaround for OS3.x only because MUI 3.8 is buggy.
    // It seems that changing the Poppen Object Color on MUI 3.8 is only visible if the
    // page is currently the ActivePage, So we make the page active before setting the
    // colors. We do that only on pages with Poppen Objects (currently only cp_Read).
    // Making the page active a second time after CO_SetConfig() should not have
    // negative effects.
    if(page == cp_Read)
      set(G->CO->GUI.GR_PAGE, MUIA_Group_ActivePage, page);
    #endif

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
  BOOL gotSemaphore = FALSE;

  ENTER();

  // If the configuration is to be used/save we must exclusively obtain the semaphore
  // to avoid destroying the mail server nodes which might be in use by active POP3 or
  // SMTP transfers. If the window is just to be closed we can go on without a lock.
  if(arg[0] == 0 || (gotSemaphore = AttemptSemaphore(G->configSemaphore)) != FALSE)
  {
    BOOL configsEqual;

    // make sure we have the latest state of the config in CE
    CO_GetConfig();

    // now we compare the current config against
    // the temporary config
    configsEqual = CompareConfigData(C, CE);

    // check if we should copy our edited configuration
    // to the real one or if we should just free/drop it
    if(arg[0] != 0)
    {
      // before we copy over the configuration, we
      // check if it was changed at all
      if(configsEqual == FALSE)
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
        // force a signature change
        // this will copy the signature text to the current signature node
        nnset(G->CO->GUI.TE_SIGEDIT, MUIA_SignatureTextEdit_SignatureNode, NULL);

        CO_SaveConfig(C, G->CO_PrefsFile);
      }
    }
    else if(configsEqual == FALSE)
    {
      // check if configs are equal and if not ask the user how to proceed
      int res = MUI_Request(G->App, G->CO->GUI.WI, MUIF_NONE,
                            tr(MSG_CO_CONFIGWARNING_TITLE),
                            tr(MSG_CO_CONFIGWARNING_BT),
                            tr(MSG_CO_CONFIGWARNING));

      // if user pressed Abort or ESC
      // the we keep the window open.
      if(res == 0)
      {
        LEAVE();
        return;
      }
    }

    // then we free our temporary config structure
    CO_ClearConfig(CE);
    free(CE);
    CE = NULL;

    // Dipose&Close the config window stuff
    DisposeModulePush(&G->CO);

    // release the config semaphore again if we obtained it before
    if(gotSemaphore == TRUE)
      ReleaseSemaphore(G->configSemaphore);
  }
  else
    ER_NewError(tr(MSG_CO_CONFIG_IS_LOCKED));

  LEAVE();
}
MakeStaticHook(CO_CloseHook, CO_CloseFunc);

///
/// CO_OpenFunc
//  Opens configuration window
HOOKPROTONHNONP(CO_OpenFunc, void)
{
  struct BusyNode *busy;

  ENTER();

  busy = BusyBegin(BUSY_TEXT);
  BusyText(busy, tr(MSG_BUSY_OPENINGCONFIG), "");

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

  BusyEnd(busy);

  LEAVE();
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
    static struct PageList page[cp_Max];
    static struct PageList *pages[cp_Max + 1];
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
    page[cp_Identities  ].PageLabel = MSG_CO_CRDIDENTITIES;
    page[cp_Filters     ].PageLabel = MSG_CO_CrdFilters;
    page[cp_Spam        ].PageLabel = MSG_CO_CRDSPAMFILTER;
    page[cp_Read        ].PageLabel = MSG_CO_CrdRead;
    page[cp_Write       ].PageLabel = MSG_CO_CrdWrite;
    page[cp_ReplyForward].PageLabel = MSG_CO_GR_REPLYFORWARD;
    page[cp_Signature   ].PageLabel = MSG_CO_CrdSignature;
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
       MUIA_HelpNode, "Configuration",
       MUIA_Window_Menustrip, MenustripObject,
          MenuChild, MenuObject,
             MUIA_Menu_Title, tr(MSG_MA_Project),
             MUIA_Menu_CopyStrings, FALSE,
             MenuChild, Menuitem(tr(MSG_CO_Open), "O", TRUE, FALSE, CMEN_OPEN),
             MenuChild, Menuitem(tr(MSG_CO_SaveAs), "A", TRUE, FALSE, CMEN_SAVEAS),
          End,
          MenuChild, MenuObject,
             MUIA_Menu_Title, tr(MSG_CO_Edit),
             MUIA_Menu_CopyStrings, FALSE,
             MenuChild, Menuitem(tr(MSG_CO_ResetDefaults), "D", TRUE, FALSE, CMEN_DEF),
             MenuChild, Menuitem(tr(MSG_CO_ResetAll), "E", TRUE, FALSE, CMEN_DEFALL),
             MenuChild, Menuitem(tr(MSG_CO_LastSaved), "L", TRUE, FALSE, CMEN_LAST),
             MenuChild, Menuitem(tr(MSG_CO_Restore), "R", TRUE, FALSE, CMEN_REST),
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
                Child, data->GUI.PG_PAGES[cp_FirstSteps]   = FirstStepsConfigPageObject, End,
                Child, data->GUI.PG_PAGES[cp_TCPIP]        = TCPIPConfigPageObject, End,
                Child, data->GUI.PG_PAGES[cp_Identities]   = IdentitiesConfigPageObject, End,
                Child, CO_PageFilters(data),
                Child, data->GUI.PG_PAGES[cp_Spam]         = SpamConfigPageObject, End,
                Child, data->GUI.PG_PAGES[cp_Read]         = ReadConfigPageObject, End,
                Child, data->GUI.PG_PAGES[cp_Write]        = WriteConfigPageObject, End,
                Child, data->GUI.PG_PAGES[cp_ReplyForward] = ReplyForwardConfigPageObject, End,
                Child, data->GUI.PG_PAGES[cp_Signature]    = SignatureConfigPageObject, End,
                Child, data->GUI.PG_PAGES[cp_Security]     = SecurityConfigPageObject, End,
                Child, data->GUI.PG_PAGES[cp_StartupQuit]  = StartupQuitConfigPageObject, End,
                Child, CO_PageMIME(data),
                Child, data->GUI.PG_PAGES[cp_AddressBook]  = AddressBookConfigPageObject, End,
                Child, CO_PageScripts(data),
                Child, CO_PageMixed(data),
                Child, CO_PageLookFeel(data),
                Child, data->GUI.PG_PAGES[cp_Update]       = UpdateConfigPageObject, End,
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
      LONG i;

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

      // set up a cross-page notification to let all pages react on changes on all other pages
      // currently only the TCPIP and Signature pages cause an update on the Identities page
      for(i = 0; i < cp_Max; i++)
      {
        if(data->GUI.PG_PAGES[i] != NULL)
        {
          LONG j;

          for(j = 0; j < cp_Max; j++)
          {
            if(j != i && data->GUI.PG_PAGES[j] != NULL)
              DoMethod(data->GUI.PG_PAGES[i], MUIM_Notify, MUIA_ConfigPage_ConfigUpdate, MUIV_EveryTime, data->GUI.PG_PAGES[j], 2, MUIM_ConfigPage_ConfigUpdate, MUIV_TriggerValue);
          }
        }
      }
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

///
