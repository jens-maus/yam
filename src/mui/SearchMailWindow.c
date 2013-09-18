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

 Superclass:  MUIC_Window
 Description: Window to search for mails in all folders

***************************************************************************/

#include "SearchMailWindow_cl.h"

#include <string.h>
#include <proto/muimaster.h>
#include <libraries/iffparse.h>
#include <mui/BetterString_mcc.h>
#include <mui/NBalance_mcc.h>
#include <mui/NList_mcc.h>
#include <mui/NListtree_mcc.h>
#include <mui/NListview_mcc.h>

#include "SDI_hook.h"

#include "YAM.h"
#include "YAM_find.h"
#include "YAM_mainFolder.h"

#include "mui/FilterPopupList.h"
#include "mui/FolderRequestListtree.h"
#include "mui/MainFolderListtree.h"
#include "mui/MainMailList.h"
#include "mui/ReadWindow.h"
#include "mui/SearchControlGroup.h"

#include "Config.h"
#include "FolderList.h"
#include "Locale.h"
#include "MailList.h"
#include "MethodStack.h"
#include "MUIObjects.h"
#include "Requesters.h"
#include "Threads.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *LV_FOLDERS;
  Object *GR_SEARCH;
  Object *LV_MAILS;
  Object *GR_PAGE;
  Object *GA_PROGRESS;
  Object *BT_SEARCH;
  Object *BT_SELECTACTIVE;
  Object *BT_SELECT;
  Object *BT_READ;

  struct Hook searchOptFromFilterPopupHook;

  BOOL active;
  BOOL abort;
  BOOL clearOnEnd;

  char progressText[SIZE_DEFAULT];
  char screenTitle[SIZE_DEFAULT];
};
*/

/// InitFilterPopupList
//  Creates a popup list of configured filters
HOOKPROTONHNP(InitFilterPopupList, ULONG, Object *listview)
{
  Object *list;

  ENTER();

  if((list = (Object *)xget(listview, MUIA_NListview_NList)) != NULL)
    DoMethod(list, MUIM_FilterPopupList_Popup);

  RETURN(TRUE);
  return TRUE;
}
MakeStaticHook(InitFilterPopupListHook, InitFilterPopupList);

///
/// SearchOptFromFilterPopup
//  Gets search options from selected filter
HOOKPROTONP(SearchOptFromFilterPopup, void, Object *listview)
{
  struct Data *data = (struct Data *)hook->h_Data;
  Object *list;

  ENTER();

  if((list = (Object *)xget(listview, MUIA_NListview_NList)) != NULL)
  {
    struct FilterNode *filter;

    // get the currently active filter
    DoMethod(list, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &filter);
    if(filter != NULL)
    {
      struct RuleNode *rule;

      if((rule = GetFilterRule(filter, 0)) != NULL)
        DoMethod(data->GR_SEARCH, MUIM_SearchControlGroup_RuleToGUI, rule);
    }
  }

  LEAVE();
}
MakeStaticHook(SearchOptFromFilterPopupHook, SearchOptFromFilterPopup);

///

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  Object *LV_FOLDERS;
  Object *GR_SEARCH;
  Object *LV_MAILS;
  Object *GR_PAGE;
  Object *GA_PROGRESS;
  Object *BT_SEARCH;
  Object *BT_SELECTACTIVE;
  Object *BT_SELECT;
  Object *BT_READ;
  Object *BT_ALL;
  Object *BT_NONE;
  Object *PO_FROMRULE;
  Object *LV_FROMRULE;
  Object *BT_TORULE;
  Object *BT_ABORT;

  ENTER();

  if((obj = DoSuperNew(cl, obj,

    MUIA_HelpNode, "Windows#Searchwindow",
    MUIA_Window_ID, MAKE_ID('F','I','N','D'),
    WindowContents, VGroup,

      Child, HGroup,
        GroupSpacing(0),
        MUIA_VertWeight, 1,
        Child, VGroup, GroupFrameT(tr(MSG_FI_FindIn)),
          MUIA_HorizWeight, 1,
          Child, NListviewObject,
            MUIA_CycleChain, 1,
            MUIA_NListview_NList, LV_FOLDERS = FolderRequestListtreeObject,
              InputListFrame,
              MUIA_NList_AutoVisible, TRUE,
              MUIA_NList_AdjustWidth, TRUE,
              MUIA_NListtree_MultiSelect, TRUE,
            End,
          End,
          Child, HGroup,
            Child, BT_ALL = MakeButton(tr(MSG_FI_AllFolders)),
            Child, BT_NONE = MakeButton(tr(MSG_FI_NOFOLDERS)),
          End,
        End,
        Child, NBalanceObject,
          MUIA_ObjectID, MAKE_ID('B','0','0','5'),
          MUIA_Balance_Quiet, TRUE,
        End,
        Child, VGroup, GroupFrameT(tr(MSG_FI_FindWhat)),
          MUIA_HorizWeight, 10,
          Child, VSpace(0),
          Child, GR_SEARCH = SearchControlGroupObject,
            MUIA_SearchControlGroup_RemoteFilterMode, FALSE,
            MUIA_SearchControlGroup_SingleRule, TRUE,
            MUIA_SearchControlGroup_AllowSpamStatus, C->SpamFilterEnabled,
          End,
          Child, ColGroup(2),
            Child, PO_FROMRULE = PopobjectObject,
              MUIA_Popstring_Button, MakeButton(tr(MSG_FI_UseFilter)),
              MUIA_Popobject_Object, NListviewObject,
                MUIA_NListview_NList, LV_FROMRULE = FilterPopupListObject,
                End,
              End,
            End,
            Child, BT_TORULE = MakeButton(tr(MSG_FI_AddAsFilter)),
          End,
          Child, VSpace(0),
        End,
      End,
      Child, NBalanceObject,
        MUIA_ObjectID, MAKE_ID('B','0','0','6'),
        MUIA_Balance_Quiet, TRUE,
      End,
      Child, VGroup, GroupFrameT(tr(MSG_FI_Results)),
        MUIA_VertWeight, 100,
        Child, NListviewObject,
          MUIA_CycleChain, TRUE,
          MUIA_NListview_NList, LV_MAILS = MainMailListObject,
            //MUIA_ObjectID,       MAKE_ID('N','L','0','3'),
            MUIA_ContextMenu,    NULL,
            MUIA_NList_Format,   "COL=8 W=-1 MIW=-1 PCS=C BAR, COL=1 W=35 PCS=R BAR, COL=3 W=55 PCS=R BAR, COL=4 W=-1 MIW=-1 BAR, COL=7 W=-1 MIW=-1 BAR, COL=5 W=10 MIW=-1 P=\33r BAR",
            MUIA_MainMailList_HandleDoubleClick, FALSE,
            MUIA_MainMailList_InSearchWindow, TRUE,
          End,
        End,
      End,
      Child, GR_PAGE = PageGroup,
        Child, HGroup,
          Child, BT_SEARCH = MakeButton(tr(MSG_FI_StartSearch)),
          Child, BT_SELECTACTIVE = MakeButton(tr(MSG_FI_SELECTACTIVE)),
          Child, BT_SELECT = MakeButton(tr(MSG_FI_SelectMatched)),
          Child, BT_READ = MakeButton(tr(MSG_FI_ReadMessage)),
        End,
        Child, HGroup,
          Child, GA_PROGRESS = GaugeObject,
            MUIA_HorizWeight, 200,
            GaugeFrame,
            MUIA_Gauge_Horiz,TRUE,
          End,
          Child, BT_ABORT = MakeButton(tr(MSG_FI_Abort)),
        End,
      End,

    End,

    TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;

    DoMethod(G->App, OM_ADDMEMBER, obj);

    data->LV_FOLDERS = LV_FOLDERS;
    data->GR_SEARCH = GR_SEARCH;
    data->LV_MAILS = LV_MAILS;
    data->GR_PAGE = GR_PAGE;
    data->GA_PROGRESS = GA_PROGRESS;
    data->BT_SEARCH = BT_SEARCH;
    data->BT_SELECTACTIVE = BT_SELECTACTIVE;
    data->BT_SELECT = BT_SELECT;
    data->BT_READ = BT_READ;

    InitHook(&data->searchOptFromFilterPopupHook, SearchOptFromFilterPopupHook, data);
    xset(PO_FROMRULE, MUIA_Popobject_ObjStrHook, &data->searchOptFromFilterPopupHook,
                      MUIA_Popobject_StrObjHook, &InitFilterPopupListHook,
                      MUIA_Popobject_WindowHook, &PO_WindowHook);

    set(BT_SELECTACTIVE, MUIA_Disabled, TRUE);
    set(BT_SELECT,       MUIA_Disabled, TRUE);
    set(BT_READ,         MUIA_Disabled, TRUE);

    xset(obj, MUIA_Window_DefaultObject, LV_MAILS,
              MUIA_Window_Title, tr(MSG_FI_FindMessages),
              MUIA_Window_ScreenTitle, CreateScreenTitle(data->screenTitle, sizeof(data->screenTitle), tr(MSG_FI_FindMessages)));

    SetHelp(LV_FOLDERS,       MSG_HELP_FI_LV_FOLDERS);
    SetHelp(BT_ALL,           MSG_HELP_FI_BT_ALL);
    SetHelp(BT_NONE,          MSG_HELP_FI_BT_NONE);
    SetHelp(PO_FROMRULE,      MSG_HELP_FI_PO_FROMRULE);
    SetHelp(BT_TORULE,        MSG_HELP_FI_BT_TORULE);
    SetHelp(BT_SEARCH,        MSG_HELP_FI_BT_SEARCH);
    SetHelp(BT_SELECTACTIVE,  MSG_HELP_FI_BT_SELECTACTIVE);
    SetHelp(BT_SELECT,        MSG_HELP_FI_BT_SELECT);
    SetHelp(BT_READ,          MSG_HELP_FI_BT_READ);
    SetHelp(BT_ABORT,         MSG_HELP_FI_BT_ABORT);

    DoMethod(BT_ABORT,        MUIM_Notify, MUIA_Pressed,             FALSE,          obj,              1, METHOD(Abort));
    DoMethod(LV_FROMRULE,     MUIM_Notify, MUIA_NList_DoubleClick,   TRUE,           PO_FROMRULE,      2, MUIM_Popstring_Close, TRUE);
    DoMethod(BT_ALL,          MUIM_Notify, MUIA_Pressed,             FALSE,          LV_FOLDERS,       5, MUIM_NListtree_Select, MUIV_NListtree_Select_All, MUIV_NListtree_Select_On, MUIF_NONE, NULL);
    DoMethod(BT_NONE,         MUIM_Notify, MUIA_Pressed,             FALSE,          LV_FOLDERS,       5, MUIM_NListtree_Select, MUIV_NListtree_Select_All, MUIV_NListtree_Select_Off, MUIF_NONE, NULL);
    DoMethod(BT_TORULE,       MUIM_Notify, MUIA_Pressed,             FALSE,          obj,              1, METHOD(CreateFilter));
    DoMethod(BT_SEARCH,       MUIM_Notify, MUIA_Pressed,             FALSE,          obj,              1, METHOD(Search));
    DoMethod(BT_SELECT,       MUIM_Notify, MUIA_Pressed,             FALSE,          obj,              1, METHOD(SelectMails));
    DoMethod(BT_SELECTACTIVE, MUIM_Notify, MUIA_Pressed,             FALSE,          obj,              1, METHOD(SwitchFolder));
    DoMethod(LV_MAILS,        MUIM_Notify, MUIA_NList_DoubleClick,   MUIV_EveryTime, obj,              1, METHOD(ReadMail));
    DoMethod(BT_READ,         MUIM_Notify, MUIA_Pressed,             FALSE,          obj,              1, METHOD(ReadMail));
    DoMethod(obj,             MUIM_Notify, MUIA_Window_CloseRequest, TRUE,           MUIV_Notify_Self, 1, METHOD(Close));

    // Lets have the Listview sorted by Reverse Date by default
    set(LV_MAILS, MUIA_NList_SortType, (4 | MUIV_NList_SortTypeAdd_2Values));
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
    case ATTR(Aborted): *store = data->abort; return TRUE;
  }

  return DoSuperMethodA(cl, obj, msg);
}

///

/* Private Functions */

/* Public Methods */
/// DECLARE(Open)
// open find window
DECLARE(Open) // struct Folder *selectFolder
{
  GETDATA;
  struct Folder *selectFolder = msg->selectFolder;

  ENTER();

  // formerly a folder was marked to be searched in only if one was given
  // as this is quite unintuitive we now fall back to the current folder
  if(selectFolder == NULL && (struct MUI_NListtree_TreeNode *)xget(data->LV_FOLDERS, MUIA_NListtree_Active) == NULL)
    selectFolder = GetCurrentFolder();

  if(selectFolder != NULL)
  {
    struct MUI_NListtree_TreeNode *tn;
    int i=0;

    // now we have to walk through the nlisttree and find
    // the folder so that we can set it as active
    while((tn = (struct MUI_NListtree_TreeNode *)DoMethod(data->LV_FOLDERS, MUIM_NListtree_GetEntry, MUIV_NListtree_GetEntry_ListNode_Root, i, MUIF_NONE)) != NULL)
    {
      struct FolderNode *fnode = tn->tn_User;

      if(fnode->folder == selectFolder)
      {
        set(data->LV_FOLDERS, MUIA_NListtree_Active, tn);
        break;
      }

      i++;
    }
  }

  // check if the window is already open
  if(xget(obj, MUIA_Window_Open) == TRUE)
  {
    // bring window to front
    DoMethod(obj, MUIM_Window_ToFront);

    // make window active
    set(obj, MUIA_Window_Activate, TRUE);
  }
  else
  {
    SafeOpenWindow(obj);
  }

  // set object of the last search session as the active one again
  set(obj, MUIA_Window_ActiveObject, xget(data->GR_SEARCH, MUIA_SearchControlGroup_ActiveObject));

  RETURN(0);
  return 0;
}

///
/// DECLARE(Close)
// closes find window
DECLARE(Close)
{
  GETDATA;

  ENTER();

  // set the abort flag so that a running search will be stopped.
  if(data->active == TRUE)
  {
    data->abort = TRUE;
    data->clearOnEnd = TRUE;
  }

  // close the window. all the other stuff will be
  // disposed by the application as soon as it
  // disposes itself..
  set(obj, MUIA_Window_Open, FALSE);

  RETURN(0);
  return 0;
}

///
/// DECLARE(Search)
// start the search and shows progress
DECLARE(Search)
{
  GETDATA;
  int fndmsg = 0;
  int totmsg = 0;
  int progress = 0;
  struct FolderList *flist;

  ENTER();

  // by default we don't dispose on end
  data->clearOnEnd = FALSE;
  data->active = TRUE;
  data->abort = FALSE;

  set(obj, MUIA_Window_ActiveObject, MUIV_Window_ActiveObject_None);
  // disable some buttons while the search is going on
  DoMethod(obj, MUIM_MultiSet, MUIA_Disabled, TRUE, data->BT_SEARCH,
                                                    data->BT_SELECTACTIVE,
                                                    data->BT_SELECT,
                                                    data->BT_READ,
                                                    NULL);
  DoMethod(data->LV_MAILS, MUIM_NList_Clear);

  // start with forbidden immediate display of found mails, this greatly speeds
  // up the search process if many mails match the search criteria.
  set(data->LV_MAILS, MUIA_NList_Quiet, TRUE);

  if((flist = CreateFolderList()) != NULL)
  {
    struct MUI_NListtree_TreeNode *tn = (struct MUI_NListtree_TreeNode *)MUIV_NListtree_NextSelected_Start;

    do
    {
      struct Folder *folder;

      DoMethod(data->LV_FOLDERS, MUIM_NListtree_NextSelected, &tn);
      if(tn == (struct MUI_NListtree_TreeNode *)MUIV_NListtree_NextSelected_End)
        break;

      if((folder = ((struct FolderNode *)tn->tn_User)->folder) != NULL)
      {
        if(MA_GetIndex(folder) == TRUE)
        {
          if(folder->Total != 0 && AddNewFolderNode(flist, folder) != NULL)
            totmsg += folder->Total;
        }
      }
    }
    while(TRUE);

    if(totmsg != 0)
    {
      struct Search search;
      struct TimeVal last;
      struct FolderNode *fnode;

      // lets prepare the search
      DoMethod(data->GR_SEARCH, MUIM_SearchControlGroup_PrepareSearch, &search);

      // set the gauge
      snprintf(data->progressText, sizeof(data->progressText), tr(MSG_FI_GAUGETEXT), totmsg);
      xset(data->GA_PROGRESS, MUIA_Gauge_InfoText, data->progressText,
                              MUIA_Gauge_Max,      totmsg,
                              MUIA_Gauge_Current,  0);

      set(data->GR_PAGE, MUIA_Group_ActivePage, 1);

      memset(&last, 0, sizeof(struct TimeVal));

      ForEachFolderNode(flist, fnode)
      {
        struct Folder *folder = fnode->folder;
        struct MailList *folderMessages;

        if((folderMessages = CloneMailList(folder->messages)) != NULL)
        {
          struct MailNode *mnode;

          // no need to lock the cloned list as it is private
          ForEachMailNode(folderMessages, mnode)
          {
            struct Mail *mail = mnode->mail;

            if(FI_DoSearch(&search, mail) == TRUE)
            {
              DoMethod(data->LV_MAILS, MUIM_NList_InsertSingle, mail, MUIV_NList_Insert_Sorted);
              fndmsg++;
            }

            // bail out if the search was aborted
            if(data->abort == TRUE)
              break;

            // increase the progress counter
            progress++;

            // then we update the gauge, but we take also care of not refreshing
            // it too often or otherwise it slows down the whole search process.
            if(TimeHasElapsed(&last, 250000) == TRUE)
            {
              // update the gauge
              set(data->GA_PROGRESS, MUIA_Gauge_Current, progress);
              // let the list show the found mails so far
              set(data->LV_MAILS, MUIA_NList_Quiet, FALSE);

              // handle the possibly received methods and messages
              CheckMethodStack();
              HandleThreads(TRUE);

              // signal the application to update now
              DoMethod(_app(obj), MUIM_Application_InputBuffered);

              // forbid immediate display again
              set(data->LV_MAILS, MUIA_NList_Quiet, TRUE);
            }
          }

          DeleteMailList(folderMessages);
        }

        // bail out if the search was aborted
        if(data->abort == TRUE)
          break;
      }

      // to let the gauge move to 100% lets increase it accordingly.
      set(data->GA_PROGRESS, MUIA_Gauge_Current, progress);

      // signal the application to update now
      DoMethod(_app(obj), MUIM_Application_InputBuffered);

      // free the temporary memory we allocated due to our
      // search operation
      FreeSearchData(&search);
    }

    DeleteFolderList(flist);
  }

  set(data->LV_MAILS, MUIA_NList_Quiet, FALSE);
  set(data->GR_PAGE, MUIA_Group_ActivePage, 0);
  // enable the buttons again
  set(data->BT_SEARCH, MUIA_Disabled, FALSE);
  DoMethod(_app(obj), MUIM_MultiSet, MUIA_Disabled, fndmsg == 0, data->BT_SELECTACTIVE,
                                                                 data->BT_SELECT,
                                                                 data->BT_READ,
                                                                 NULL);

  data->active = FALSE;

  // if the closeHook has set the ClearOnEnd flag we have to clear
  // the result listview
  if(data->clearOnEnd == TRUE)
    DoMethod(data->LV_MAILS, MUIM_NList_Clear);
  else
    set(obj, MUIA_Window_ActiveObject, xget(data->GR_SEARCH, MUIA_SearchControlGroup_ActiveObject));

  RETURN(0);
  return 0;
}

///
/// DECLARE(Abort)
DECLARE(Abort)
{
  GETDATA;

  ENTER();

  data->abort = TRUE;

  RETURN(0);
  return 0;
}

///
/// DECLARE(CreateFilter)
// create a filter from the current search options
DECLARE(CreateFilter)
{
  GETDATA;
  int ch;
  char name[SIZE_NAME];

  ENTER();

  name[0] = '\0';

  // request a name for that new filter from the user
  if((ch = StringRequest(name, sizeof(name),
                               tr(MSG_FI_AddFilter),
                               tr(MSG_FI_AddFilterReq),
                               tr(MSG_Save),
                               tr(MSG_Use),
                               tr(MSG_Cancel), FALSE,
                               obj)) != 0)
  {
    struct FilterNode *filter;

    if((filter = CreateNewFilter(FA_TERMINATE, 0)) != NULL)
    {
      struct RuleNode *rule;

      strlcpy(filter->name, name, sizeof(filter->name));

      if((rule = GetFilterRule(filter, 0)) != NULL)
        DoMethod(data->GR_SEARCH, MUIM_SearchControlGroup_GUIToRule, rule);

      // Now add the new filter to our list
      AddTail((struct List *)&C->filterList, (struct Node *)filter);

      // check if we should immediately save our configuration or not
      if(ch == 1)
        SaveConfig(C, G->CO_PrefsFile);
    }
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(SwitchFolder)
// sets active folder according to the selected message in the results window
DECLARE(SwitchFolder)
{
  GETDATA;
  struct Mail *mail;

  ENTER();

  // get the mail from the find list
  DoMethod(data->LV_MAILS, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &mail);
  if(mail != NULL)
  {
    LONG pos = MUIV_NList_GetPos_Start;

    // make sure the folder of the found mail is set active
    MA_ChangeFolder(mail->Folder, TRUE);

    // now get the position of the foundmail in the real mail list and set it active
    DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_NList_GetPos, mail, &pos);

    // if we found the mail in the currently active listview
    // we make it the active one as well.
    if(pos != MUIV_NList_GetPos_End)
    {
      // Because the NList for the maillistview and the NList for the find listview
      // are sharing the same displayhook we have to call MUIA_NList_DisplayRecall
      // twice here so that it will recognize that something has changed or
      // otherwise both NLists will show glitches.
      xset(G->MA->GUI.PG_MAILLIST, MUIA_NList_DisplayRecall, TRUE,
                                   MUIA_NList_Active, pos);

      set(data->LV_MAILS, MUIA_NList_DisplayRecall, TRUE);
    }
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(ReadMail)
// open a read window for a message listed in the results window
DECLARE(ReadMail)
{
  GETDATA;
  struct Mail *mail;

  ENTER();

  DoMethod(data->LV_MAILS, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &mail);
  if(mail != NULL)
  {
    struct ReadMailData *rmData;

    if((rmData = CreateReadWindow(FALSE)) != NULL)
    {
      // make sure it is opened correctly and then read in a mail
      if(SafeOpenWindow(rmData->readWindow) == FALSE ||
         DoMethod(rmData->readWindow, MUIM_ReadWindow_ReadMail, mail) == FALSE)
      {
        // on any error we make sure to delete the read window
        // immediatly again.
        CleanupReadMailData(rmData, TRUE);
      }
    }
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(SelectMails)
// select matching messages in the main message list
DECLARE(SelectMails)
{
  GETDATA;
  struct Folder *folder;
  int i;

  ENTER();

  // unselect the currently selected mails first
  set(G->MA->GUI.PG_MAILLIST, MUIA_NList_Active, MUIV_NList_Active_Off);
  DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_NList_Select, MUIV_NList_Select_All, MUIV_NList_Select_Off, NULL);

  folder = GetCurrentFolder();
  for(i=0; ;i++)
  {
    struct Mail *mail;

    DoMethod(data->LV_MAILS, MUIM_NList_GetEntry, i, &mail);
    if(mail == NULL)
      break;

    // only if the current folder is the same as this messages resides in
    if(mail->Folder == folder)
    {
      LONG pos = MUIV_NList_GetPos_Start;

      // get the position of the foundmail in the currently active
      // listview
      DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_NList_GetPos, mail, &pos);

      // if we found the one in our listview we select it
      if(pos != MUIV_NList_GetPos_End)
        DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_NList_Select, pos, MUIV_NList_Select_On, NULL);
    }
  }

  MA_ChangeSelected(TRUE);

  RETURN(0);
  return 0;
}

///
/// DECLARE(RemoveMail)
DECLARE(RemoveMail) // struct Mail *mail
{
  GETDATA;

  ENTER();

  DoMethod(data->LV_MAILS, MUIM_MainMailList_RemoveMail, msg->mail);

  RETURN(0);
  return 0;
}

///
