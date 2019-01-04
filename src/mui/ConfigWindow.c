/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2019 YAM Open Source Team

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

 Superclass:  MUIC_Window
 Description: Config window of the application

***************************************************************************/

#include "ConfigWindow_cl.h"

#include <proto/muimaster.h>
#include <libraries/iffparse.h>
#include <mui/NListview_mcc.h>

#include "YAM.h"
#include "YAM_error.h"

#include "mui/AddressBookConfigPage.h"
#include "mui/ConfigPage.h"
#include "mui/ConfigPageList.h"
#include "mui/FiltersConfigPage.h"
#include "mui/FirstStepsConfigPage.h"
#include "mui/IdentitiesConfigPage.h"
#include "mui/LookFeelConfigPage.h"
#include "mui/MimeConfigPage.h"
#include "mui/MixedConfigPage.h"
#include "mui/ReadConfigPage.h"
#include "mui/ReplyForwardConfigPage.h"
#include "mui/ScriptsConfigPage.h"
#include "mui/SecurityConfigPage.h"
#include "mui/SignatureConfigPage.h"
#include "mui/SpamConfigPage.h"
#include "mui/StartupQuitConfigPage.h"
#include "mui/TCPIPConfigPage.h"
#include "mui/UpdateConfigPage.h"
#include "mui/WriteConfigPage.h"
#include "mui/YAMApplication.h"

#include "Config.h"
#include "FileInfo.h"
#include "Locale.h"
#include "Requesters.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *GR_PAGE;
  Object *PG_PAGES[cp_Max];

  enum ConfigPage visiblePage;

  char windowTitle[SIZE_DEFAULT];
  char screenTitle[SIZE_DEFAULT];

  BOOL visitedPages[cp_Max];
  BOOL updateAll;
};
*/

/* INCLUDE
#include "ConfigPageList.h"
*/

#define MUIV_ConfigWindow_Close_Cancel 0
#define MUIV_ConfigWindow_Close_Use    1
#define MUIV_ConfigWindow_Close_Save   2

/* Private functions */
/// NewPrefsFile
// sets the name of the configuration file
static void NewPrefsFile(struct IClass *cl, Object *obj, char *fname)
{
  GETDATA;

  strlcpy(G->CO_PrefsFile, fname, sizeof(G->CO_PrefsFile));
  snprintf(data->windowTitle, sizeof(data->windowTitle), "%s (%s)", tr(MSG_MA_MConfig), fname);

  xset(obj, MUIA_Window_Title, data->windowTitle,
            MUIA_Window_ScreenTitle, CreateScreenTitle(data->screenTitle, sizeof(data->screenTitle), data->windowTitle));
}

///

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  static struct PageList page[cp_Max];
  static struct PageList *pages[cp_Max + 1];
  int i;
  Object *BT_SAVE;
  Object *BT_USE;
  Object *BT_CANCEL;
  Object *NLV_PAGE;
  Object *LV_PAGE;
  Object *GR_PAGE;
  Object *PG_PAGES[cp_Max];

  // menu item enums
  enum { CMEN_OPEN = 1201, CMEN_SAVEAS, CMEN_SAVEWOPRIV, CMEN_DEF, CMEN_DEFALL, CMEN_LAST, CMEN_REST };

  ENTER();

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

  if((obj = DoSuperNew(cl, obj,

    MUIA_HelpNode, "Configuration",
    MUIA_Window_Menustrip, MenustripObject,
      MenuChild, MenuObject,
        MUIA_Menu_Title, tr(MSG_MA_Project),
        MUIA_Menu_CopyStrings, FALSE,
        MenuChild, Menuitem(tr(MSG_CO_Open), "O", TRUE, FALSE, CMEN_OPEN),
        MenuChild, Menuitem(tr(MSG_CO_SaveAs), "A", TRUE, FALSE, CMEN_SAVEAS),
        MenuChild, Menuitem(tr(MSG_CO_SAVE_WITHOUT_PRIVATE_DATA), NULL, TRUE, FALSE, CMEN_SAVEWOPRIV),
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
        Child, NLV_PAGE = NListviewObject,
          MUIA_CycleChain,  TRUE,
          MUIA_NListview_NList, LV_PAGE = ConfigPageListObject,
            MUIA_NList_Format,        "",
            MUIA_NList_Title,         FALSE,
            MUIA_NList_AdjustWidth,   TRUE,
            MUIA_NList_AutoVisible,   TRUE,
            MUIA_NList_MinLineHeight, 16,
            MUIA_NList_SourceArray,   pages,
            MUIA_NList_Active,        MUIV_NList_Active_Top,
          End,
        End,
        Child, GR_PAGE = PageGroup,
          NoFrame,
          MUIA_Group_ActivePage, 0,
          Child, PG_PAGES[cp_FirstSteps]   = FirstStepsConfigPageObject, End,
          Child, PG_PAGES[cp_TCPIP]        = TCPIPConfigPageObject, End,
          Child, PG_PAGES[cp_Identities]   = IdentitiesConfigPageObject, End,
          Child, PG_PAGES[cp_Filters   ]   = FiltersConfigPageObject, End,
          Child, PG_PAGES[cp_Spam]         = SpamConfigPageObject, End,
          Child, PG_PAGES[cp_Read]         = ReadConfigPageObject, End,
          Child, PG_PAGES[cp_Write]        = WriteConfigPageObject, End,
          Child, PG_PAGES[cp_ReplyForward] = ReplyForwardConfigPageObject, End,
          Child, PG_PAGES[cp_Signature]    = SignatureConfigPageObject, End,
          Child, PG_PAGES[cp_Security]     = SecurityConfigPageObject, End,
          Child, PG_PAGES[cp_StartupQuit]  = StartupQuitConfigPageObject, End,
          Child, PG_PAGES[cp_MIME]         = MimeConfigPageObject, End,
          Child, PG_PAGES[cp_AddressBook]  = AddressBookConfigPageObject, End,
          Child, PG_PAGES[cp_Scripts]      = ScriptsConfigPageObject, End,
          Child, PG_PAGES[cp_Mixed]        = MixedConfigPageObject, End,
          Child, PG_PAGES[cp_LookFeel]     = LookFeelConfigPageObject, End,
          Child, PG_PAGES[cp_Update]       = UpdateConfigPageObject, End,
        End,
      End,

      Child, RectangleObject,
        MUIA_Rectangle_HBar, TRUE,
        MUIA_FixHeight,      4,
      End,

      Child, HGroup,
        MUIA_Group_SameWidth, TRUE,
        Child, BT_SAVE   = MakeButton(tr(MSG_CO_Save)),
        Child, BT_USE    = MakeButton(tr(MSG_CO_Use)),
        Child, BT_CANCEL = MakeButton(tr(MSG_CO_Cancel)),
      End,
    End,

    TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;

    data->GR_PAGE = GR_PAGE;
    for(i = 0; i < cp_Max; i++)
      data->PG_PAGES[i] = PG_PAGES[i];

    // add the window to our application
    DoMethod(G->App, OM_ADDMEMBER, obj);

    xset(obj, MUIA_Window_DefaultObject, NLV_PAGE,
              MUIA_Window_Title, tr(MSG_MA_MConfig),
              MUIA_Window_ScreenTitle, CreateScreenTitle(data->screenTitle, sizeof(data->screenTitle), tr(MSG_MA_MConfig)));

    SetHelp(BT_SAVE,   MSG_HELP_CO_BT_SAVE);
    SetHelp(BT_USE,    MSG_HELP_CO_BT_USE);
    SetHelp(BT_CANCEL, MSG_HELP_CO_BT_CANCEL);

    DoMethod(obj,        MUIM_Notify, MUIA_Window_MenuAction,   CMEN_OPEN,       obj, 1, METHOD(OpenConfig));
    DoMethod(obj,        MUIM_Notify, MUIA_Window_MenuAction,   CMEN_SAVEAS,     obj, 2, METHOD(SaveConfigAs), TRUE);
    DoMethod(obj,        MUIM_Notify, MUIA_Window_MenuAction,   CMEN_SAVEWOPRIV, obj, 2, METHOD(SaveConfigAs), FALSE);
    DoMethod(obj,        MUIM_Notify, MUIA_Window_MenuAction,   CMEN_DEF,        obj, 2, METHOD(ResetToDefault), FALSE);
    DoMethod(obj,        MUIM_Notify, MUIA_Window_MenuAction,   CMEN_DEFALL,     obj, 2, METHOD(ResetToDefault), TRUE);
    DoMethod(obj,        MUIM_Notify, MUIA_Window_MenuAction,   CMEN_LAST,       obj, 1, METHOD(LastSaved));
    DoMethod(obj,        MUIM_Notify, MUIA_Window_MenuAction,   CMEN_REST,       obj, 1, METHOD(Restore));
    DoMethod(LV_PAGE,    MUIM_Notify, MUIA_NList_Active,        MUIV_EveryTime,  obj, 2, METHOD(ChangePage), MUIV_TriggerValue);
    DoMethod(BT_SAVE,    MUIM_Notify, MUIA_Pressed,             FALSE,           obj, 2, METHOD(Close), MUIV_ConfigWindow_Close_Save);
    DoMethod(BT_USE,     MUIM_Notify, MUIA_Pressed,             FALSE,           obj, 2, METHOD(Close), MUIV_ConfigWindow_Close_Use);
    DoMethod(BT_CANCEL,  MUIM_Notify, MUIA_Pressed,             FALSE,           obj, 2, METHOD(Close), MUIV_ConfigWindow_Close_Cancel);
    DoMethod(obj,        MUIM_Notify, MUIA_Window_CloseRequest, TRUE,            obj, 2, METHOD(Close), MUIV_ConfigWindow_Close_Cancel);

    // set up a cross-page notification to let all pages react on changes on all other pages
    // currently only the TCPIP and Signature pages cause an update on the Identities page
    for(i = 0; i < cp_Max; i++)
    {
      LONG j;

      for(j = 0; j < cp_Max; j++)
      {
        if(j != i)
          DoMethod(PG_PAGES[i], MUIM_Notify, MUIA_ConfigPage_ConfigUpdate, MUIV_EveryTime, PG_PAGES[j], 2, MUIM_ConfigPage_ConfigUpdate, MUIV_TriggerValue);
      }
    }

    // set up the "First steps" page
    DoMethod(PG_PAGES[cp_FirstSteps], MUIM_ConfigPage_ConfigToGUI, CE);
  }

  RETURN((IPTR)obj);
  return (IPTR)obj;
}

///
/// OVERLOAD(OM_GET)
OVERLOAD(OM_GET)
{
  GETDATA;
  IPTR *store = ((struct opGet *)msg)->opg_Storage;

  switch(((struct opGet *)msg)->opg_AttrID)
  {
    case ATTR(UpdateAll):    *store = data->updateAll; data->updateAll = FALSE; return TRUE;
    case ATTR(VisiblePage):  *store = data->visiblePage; return TRUE;
    case ATTR(VisitedPages): *store = (IPTR)data->visitedPages; return TRUE;
  }

  return DoSuperMethodA(cl, obj, msg);
}

///

/* Public Methods */
/// DECLARE(ConfigToGUI)
DECLARE(ConfigToGUI) // enum ConfigPage page
{
  GETDATA;

  ENTER();

  if(msg->page >= cp_FirstSteps && msg->page < cp_Max)
  {
    DoMethod(data->PG_PAGES[msg->page], MUIM_ConfigPage_ConfigToGUI, CE);
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(GUIToConfig)
DECLARE(GUIToConfig) // enum ConfigPage page
{
  GETDATA;

  ENTER();

  if(msg->page >= cp_FirstSteps && msg->page < cp_Max)
  {
    DoMethod(data->PG_PAGES[msg->page], MUIM_ConfigPage_GUIToConfig);
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(ChangePage)
// selects a different page of the configuration
DECLARE(ChangePage) // enum ConfigPage page
{
  GETDATA;

  ENTER();

  if(msg->page >= cp_FirstSteps && msg->page < cp_Max)
  {
    // save the settings of the formerly visible page first
    DoMethod(data->PG_PAGES[data->visiblePage], MUIM_ConfigPage_GUIToConfig);

    // remember the new page and mark it as visited
    data->visiblePage = msg->page;
    data->visitedPages[msg->page] = TRUE;

    #if defined(__amigaos3__)
    // The follow lines are a workaround for OS3.x only because MUI 3.8 is buggy.
    // It seems that changing the Poppen Object Color on MUI 3.8 is only visible if the
    // page is currently the ActivePage, So we make the page active before setting the
    // colors. We do that only on pages with Poppen Objects (currently only cp_Read).
    // Making the page active a second time after CO_SetConfig() should not have
    // negative effects.
    if(msg->page == cp_Read)
      set(data->GR_PAGE, MUIA_Group_ActivePage, msg->page);
    #endif

    // set the settings of the new page
    DoMethod(data->PG_PAGES[msg->page], MUIM_ConfigPage_ConfigToGUI);

    set(data->GR_PAGE, MUIA_Group_ActivePage, msg->page);
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(OpenConfig)
// opens a different configuration file
DECLARE(OpenConfig)
{
  GETDATA;
  struct FileReqCache *frc;

  ENTER();

  if((frc = ReqFile(ASL_CONFIG, obj, tr(MSG_CO_Open), REQF_NONE, G->MA_MailDir, "")) != NULL)
  {
    char cname[SIZE_PATHFILE];

    AddPath(cname, frc->drawer, frc->file, sizeof(cname));
    if(LoadConfig(CE, cname) == 1)
      NewPrefsFile(cl, obj, cname);

    // resolve any unset folder IDs
    ResolveConfigFolders(CE);
    DoMethod(data->PG_PAGES[data->visiblePage], MUIM_ConfigPage_ConfigToGUI, CE);

    // remember to update all config items in ValidateConfig()
    data->updateAll = TRUE;
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(SaveConfigAs)
// saves configuration to a file using an alternative name
DECLARE(SaveConfigAs) // ULONG savePrivateData
{
  GETDATA;
  struct FileReqCache *frc;

  ENTER();

  if((frc = ReqFile(ASL_CONFIG, obj, msg->savePrivateData ? tr(MSG_CO_SaveAs) : tr(MSG_CO_SAVE_WITHOUT_PRIVATE_DATA), REQF_SAVEMODE, G->MA_MailDir, "")) != NULL)
  {
    char cname[SIZE_PATHFILE];

    AddPath(cname, frc->drawer, frc->file, sizeof(cname));

    if(FileExists(cname) == FALSE ||
       MUI_Request(_app(obj), obj, MUIF_NONE, tr(MSG_MA_ConfirmReq), tr(MSG_YesNoReq2), tr(MSG_FILE_OVERWRITE), frc->file) != 0)
    {
      // first let the currently visible page flush any pending changes to the configuration
      DoMethod(data->PG_PAGES[data->visiblePage], MUIM_ConfigPage_GUIToConfig);

      // forbid automatically saving the config after validation,
      // it will be saved anyway later
      ValidateConfig(CE, TRUE, FALSE);
      // resolve any changed folder IDs
      ResolveConfigFolders(CE);
      NewPrefsFile(cl, obj, cname);
      SaveConfig(CE, cname, msg->savePrivateData);
    }
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(ResetToDefault)
// resets configuration (or a part of it)
DECLARE(ResetToDefault) // ULONG all
{
  GETDATA;

  ENTER();

  if(msg->all == TRUE)
  {
    SetDefaultConfig(CE, cp_AllPages);

    // remember to update all config items in ValidateConfig()
    data->updateAll = TRUE;
  }
  else
    SetDefaultConfig(CE, data->visiblePage);

  DoMethod(data->PG_PAGES[data->visiblePage], MUIM_ConfigPage_ConfigToGUI, CE);

  RETURN(0);
  return 0;
}

///
/// DECLARE(LastSaved)
// reloads configuration from file
DECLARE(LastSaved)
{
  GETDATA;

  ENTER();

  LoadConfig(CE, G->CO_PrefsFile);

  DoMethod(data->PG_PAGES[data->visiblePage], MUIM_ConfigPage_ConfigToGUI, CE);

  // remember to update all config items in ValidateConfig()
  data->updateAll = TRUE;

  RETURN(0);
  return 0;
}

///
/// DECLARE(Restore)
// makes all changes undone
DECLARE(Restore)
{
  GETDATA;

  ENTER();

  ClearConfig(CE);
  CopyConfig(CE, C);

  DoMethod(data->PG_PAGES[data->visiblePage], MUIM_ConfigPage_ConfigToGUI, CE);

  RETURN(0);
  return 0;
}

///
/// DECLARE(Close)
// closes configuration window
DECLARE(Close) // ULONG how
{
  GETDATA;

  ENTER();

  // there is no point to do anything here if YAM is terminating
  if(G->Terminating == FALSE)
  {
    BOOL gotSemaphore = FALSE;
    // If the configuration is to be used/save we must exclusively obtain the semaphore
    // to avoid destroying the mail server nodes which might be in use by active POP3 or
    // SMTP transfers. If the window is just to be closed we can go on without a lock.
    if(msg->how == MUIV_ConfigWindow_Close_Cancel || (gotSemaphore = AttemptSemaphore(G->configSemaphore)) != FALSE)
    {
      BOOL configsEqual;
      BOOL close = TRUE;

      // make sure we have the latest state of the config in CE
      DoMethod(data->PG_PAGES[data->visiblePage], MUIM_ConfigPage_GUIToConfig);

      // now we compare the current config against
      // the temporary config
      configsEqual = CompareConfigs(C, CE);

      // check if we should copy our edited configuration
      // to the real one or if we should just free/drop it
      if(msg->how != MUIV_ConfigWindow_Close_Cancel)
      {
        // before we copy over the configuration, we
        // check if it was changed at all
        if(configsEqual == FALSE)
        {
          D(DBF_CONFIG, "configuration found to be different");

          // check for certain important state changes and do another comparison
          if(CheckConfigDiffs(data->visitedPages) == TRUE)
            configsEqual = CompareConfigs(C, CE);

          if(configsEqual == FALSE)
          {
            struct Config *tmpC;

            // just swap the pointers instead of clearing and copying the whole stuff
            tmpC = C;
            C = CE;
            CE = tmpC;
            // the up to now "current" configuration will be freed below
          }
        }
        else
          D(DBF_CONFIG, "config wasn't altered, skipped copy operations.");

        // validate that C has valid values
        // forbid automatically saving the config after validation
        ValidateConfig(C, TRUE, FALSE);

        // resolve any changed folder IDs
        ResolveConfigFolders(C);

        // we save the configuration if the user
        // has pressed on 'Save' only.
        if(msg->how == MUIV_ConfigWindow_Close_Save)
          SaveConfig(C, G->CO_PrefsFile, TRUE);
      }
      else if(configsEqual == FALSE)
      {
        // check if configs are equal and if not ask the user how to proceed
        int res = MUI_Request(_app(obj), obj, MUIF_NONE,
                              tr(MSG_CO_CONFIGWARNING_TITLE),
                              tr(MSG_CO_CONFIGWARNING_BT),
                              tr(MSG_CO_CONFIGWARNING));

        // if user pressed Abort or ESC
        // then we keep the window open.
        if(res == 0)
          close = FALSE;
      }

      if(close == TRUE)
      {
        // then we free our temporary config structure
        FreeConfig(CE);
        CE = NULL;

        // Dipose&Close the config window stuff
        DoMethod(_app(obj), MUIM_Application_PushMethod, _app(obj), 1, MUIM_YAMApplication_CloseConfigWindow);
      }

      // release the config semaphore again if we obtained it before
      if(gotSemaphore == TRUE)
        ReleaseSemaphore(G->configSemaphore);
    }
    else
      ER_NewError(tr(MSG_CO_CONFIG_IS_LOCKED));
  }

  RETURN(0);
  return 0;
}

///
