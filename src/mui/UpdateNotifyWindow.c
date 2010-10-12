/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2010 by YAM Open Source Team

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
 Description: Window where user will be notified on updates.

***************************************************************************/

#include "UpdateNotifyWindow_cl.h"

#include <proto/openurl.h>
#include <mui/NBalance_mcc.h>
#include <mui/NList_mcc.h>
#include <mui/NFloattext_mcc.h>

#include "YAM_configFile.h"
#include "FileInfo.h"
#include "MUIObjects.h"
#include "Requesters.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *ComponentList;
  Object *ComponentHistory;
  Object *SkipInFutureCheckBox;
  Object *VisitURLButton;
  char *ChangeLogText;
  char WindowTitle[SIZE_DEFAULT];
};
*/

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  Object *bt_visit;
  Object *bt_close;
  Object *nl_componentlist;
  Object *nf_componenthistory;
  Object *ch_skipinfuture;

  ENTER();

  if((obj = DoSuperNew(cl, obj,

    MUIA_Window_ID,         MAKE_ID('U','P','D','1'),
    MUIA_Window_Title,      tr(MSG_UPD_NOTIFICATION_WTITLE),
    MUIA_Window_Height,     MUIV_Window_Height_MinMax(30),
    MUIA_Window_Width,      MUIV_Window_Width_MinMax(30),
    MUIA_Window_RefWindow,  G->MA->GUI.WI,
    WindowContents, VGroup,

      Child, HGroup,
        Child, MakeImageObject("config_update_big", G->theme.configImages[ci_UpdateBig]),
        Child, VGroup,
          Child, TextObject,
            MUIA_Text_PreParse, "\033b",
            MUIA_Text_Contents, tr(MSG_UPD_NOTIFICATION_TITLE),
            MUIA_Weight,        100,
          End,
          Child, TextObject,
            MUIA_Text_Contents, tr(MSG_UPD_NOTIFICATION_SUMMARY),
            MUIA_Font,          MUIV_Font_Tiny,
            MUIA_Weight,        100,
          End,
        End,
      End,

      Child, RectangleObject,
        MUIA_Rectangle_HBar, TRUE,
        MUIA_FixHeight,      4,
      End,

      Child, NListviewObject,
        MUIA_CycleChain, TRUE,
        MUIA_VertWeight, 20,
        MUIA_Listview_DragType,  MUIV_Listview_DragType_None,
        MUIA_NListview_NList, nl_componentlist = UpdateComponentListObject,
        End,
      End,

      Child, NBalanceObject, End,

      Child, TextObject,
        MUIA_Text_Contents, tr(MSG_UPD_NOTIFICATION_CHANGES),
        MUIA_Font,          MUIV_Font_Tiny,
      End,
      Child, NListviewObject,
        MUIA_CycleChain, TRUE,
        MUIA_NListview_NList, nf_componenthistory = NFloattextObject,
          MUIA_Font,             MUIV_Font_Fixed,
          MUIA_NList_Format,     "P=\33l",
          MUIA_NList_Input,      FALSE,
          MUIA_NFloattext_Text,  "",
        End,
      End,

      Child, HGroup,
        Child, ch_skipinfuture = MakeCheck(tr(MSG_UPD_NOTIFICATION_NOUPDATE)),
        Child, LLabel1(tr(MSG_UPD_NOTIFICATION_NOUPDATE)),
        Child, HVSpace,
      End,

      Child, RectangleObject,
        MUIA_Rectangle_HBar, TRUE,
        MUIA_FixHeight,      4,
      End,

      Child, HGroup,
        Child, HVSpace,
        Child, HVSpace,
        Child, bt_close = MakeButton(tr(MSG_UPD_NOTIFICATION_CLOSE)),
        Child, bt_visit = MakeButton(tr(MSG_UPD_NOTIFICATION_VISITURL)),
      End,

    End,

    TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;

    DoMethod(G->App, OM_ADDMEMBER, obj);

    data->ComponentList = nl_componentlist;
    data->ComponentHistory = nf_componenthistory;
    data->SkipInFutureCheckBox = ch_skipinfuture;
    data->VisitURLButton = bt_visit;

    // start with a disabled "Visit URL" button
    set(bt_visit, MUIA_Disabled, TRUE);

    DoMethod(obj,               MUIM_Notify, MUIA_Window_CloseRequest, TRUE, MUIV_Notify_Self, 3, METHOD(Close));
    DoMethod(nl_componentlist,  MUIM_Notify, MUIA_NList_Active, MUIV_EveryTime, obj, 2, METHOD(Select), MUIV_TriggerValue);
    DoMethod(nl_componentlist,  MUIM_Notify, MUIA_NList_DoubleClick, MUIV_EveryTime, obj, 1, METHOD(VisitURL));
    DoMethod(bt_visit,          MUIM_Notify, MUIA_Pressed, FALSE, obj, 1, METHOD(VisitURL));
    DoMethod(bt_close,          MUIM_Notify, MUIA_Pressed, FALSE, obj, 3, METHOD(Close));

    set(obj, MUIA_Window_Activate, TRUE);
  }

  RETURN((IPTR)obj);
  return (IPTR)obj;
}

///
/// OVERLOAD(OM_SET)
OVERLOAD(OM_SET)
{
  GETDATA;

  struct TagItem *tags = inittags(msg), *tag;
  while((tag = NextTagItem((APTR)&tags)) != NULL)
  {
    switch(tag->ti_Tag)
    {
      // we also catch foreign attributes
      case MUIA_Window_Open:
      {
        // if the object should be hidden we clean it up also
        if(tag->ti_Data == FALSE)
        {
          // we only clear the notify window's content if this
          // close request wasn't issue due to an application iconification
          // request
          if(xget(G->App, MUIA_Application_Iconified) == FALSE)
            DoMethod(obj, METHOD(Clear));
        }
        else
        {
          char buf[64];

          // setup some options and select the first entry at the top
          set(data->SkipInFutureCheckBox, MUIA_Selected, C->UpdateInterval == 0);
          set(data->ComponentList, MUIA_NList_Active, MUIV_NList_Active_Top);

          // we now specify the window title as we add the date/time to it
          DateStamp2String(buf, sizeof(buf), NULL, DSS_DATETIME, TZC_NONE);
          snprintf(data->WindowTitle, sizeof(data->WindowTitle), "%s - %s", tr(MSG_UPD_NOTIFICATION_WTITLE), buf);

          xset(obj, MUIA_Window_Title,         data->WindowTitle,
                    MUIA_Window_DefaultObject, data->ComponentList);

          // we also make sure the application in uniconified.
          if(xget(G->App, MUIA_Application_Iconified))
            set(G->App, MUIA_Application_Iconified, FALSE);
        }
      }
      break;
    }
  }

  return DoSuperMethodA(cl, obj, msg);
}

///

/* Public Methods */
/// DECLARE(Clear)
DECLARE(Clear)
{
  GETDATA;

  ENTER();

  DoMethod(data->ComponentList, MUIM_NList_Clear);
  set(data->ComponentHistory, MUIA_NFloattext_Text, "");

  free(data->ChangeLogText);
  data->ChangeLogText = NULL;

  RETURN(0);
  return 0;
}

///
/// DECLARE(Select)
DECLARE(Select) // ULONG num
{
  GETDATA;
  struct UpdateComponent *comp = NULL;

  ENTER();

  DoMethod(data->ComponentList, MUIM_NList_GetEntry, msg->num, &comp);

  // disable the "Visit URL" button in case we found no valid component
  set(data->VisitURLButton, MUIA_Disabled, comp == NULL);

  if(comp != NULL && comp->changeLogFile != NULL)
  {
    LONG size;

    // lets open the changelog file and parse it
    if(ObtainFileInfo(comp->changeLogFile->Filename, FI_SIZE, &size) == TRUE && size > 0)
    {
      if((comp->changeLogFile->FP = fopen(comp->changeLogFile->Filename, "r")) != NULL)
      {
        free(data->ChangeLogText);

        if((data->ChangeLogText = malloc(size+1)) != NULL)
        {
          if(fread(data->ChangeLogText, size, 1, comp->changeLogFile->FP) == 1)
          {
            data->ChangeLogText[size] = '\0';

            set(data->ComponentHistory, MUIA_NFloattext_Text, data->ChangeLogText);
          }
        }

        fclose(comp->changeLogFile->FP);
        comp->changeLogFile->FP = NULL;
      }
    }
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(AddComponent)
DECLARE(AddComponent) // struct UpdateComponent *comp
{
  GETDATA;

  ENTER();

  D(DBF_UPDATE, "added '%s' as a new updateable component", msg->comp->name);
  DoMethod(data->ComponentList, MUIM_NList_InsertSingle, msg->comp, MUIV_NList_Insert_Bottom);

  RETURN(0);
  return 0;
}

///
/// DECLARE(VisitURL)
DECLARE(VisitURL)
{
  GETDATA;
  struct UpdateComponent *comp = NULL;

  ENTER();

  DoMethod(data->ComponentList, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &comp);
  if(comp != NULL)
  {
    if(GotoURLPossible() == TRUE)
    {
      // openurl.library is available, so let openurl.library or URL: open the link
      GotoURL(comp->url, TRUE);
    }
    else
    {
      // no openurl.library or URL: device, just open a requester telling the user what to do
      MUI_Request(G->App, obj, 0L, NULL, tr(MSG_OkayReq), tr(MSG_NO_OPENURL_LIB), comp->url);
    }
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(Close)
DECLARE(Close)
{
  GETDATA;

  ENTER();

  // before we close the window we have to check the status
  // of the SkipInFutureCheckBox and set the configuration accordingly.
  if(xget(data->SkipInFutureCheckBox, MUIA_Selected) == TRUE)
  {
    // now we make sure no further update timer is running.
    C->UpdateInterval = 0;
    InitUpdateCheck(FALSE);
  }
  else if(C->UpdateInterval == 0)
  {
    // now we have to make sure we reactivate the updatecheck
    // timer again as the user unchecked the skip checkbox.
    C->UpdateInterval = 604800; // check weekly for updates per default
    InitUpdateCheck(FALSE);
  }

  if(CE)
  {
    // now we make sure the C and CE config structure is in sync again
    CE->UpdateInterval = C->UpdateInterval;
  }

  // make sure the update check config page is correctly refreshed
  // if it is currently the active one.
  if(G->CO && G->CO->VisiblePage == cp_Update)
    CO_SetConfig();

  // now close the window for real.
  set(obj, MUIA_Window_Open, FALSE);

  RETURN(0);
  return 0;
}

///
