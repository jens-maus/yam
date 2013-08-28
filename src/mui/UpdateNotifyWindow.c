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
 Description: Window where user will be notified on updates.

***************************************************************************/

#include "UpdateNotifyWindow_cl.h"

#include <stdlib.h>
#include <string.h>

#include <proto/dos.h>
#include <proto/muimaster.h>
#include <proto/openurl.h>
#include <libraries/iffparse.h>
#include <mui/NBalance_mcc.h>
#include <mui/NList_mcc.h>
#include <mui/NFloattext_mcc.h>

#include "YAM.h"
#include "YAM_utilities.h"

#include "mui/ConfigWindow.h"
#include "mui/ImageArea.h"
#include "mui/UpdateComponentList.h"
#include "tcp/http.h"

#include "Busy.h"
#include "Config.h"
#include "FileInfo.h"
#include "Locale.h"
#include "MUIObjects.h"
#include "Requesters.h"
#include "Themes.h"
#include "Threads.h"
#include "UpdateCheck.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *componentList;
  Object *componentHistory;
  Object *skipInFutureCheckBox;
  Object *downloadButton;
  char *changeLogText;
  char windowTitle[SIZE_DEFAULT];
  char screenTitle[SIZE_DEFAULT];
  struct TempFile *tempFile;
  struct BusyNode *busy;
  BOOL quiet;
  BOOL updateSuccess;
};
*/

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  struct TempFile *tempFile;

  ENTER();

  if((tempFile = OpenTempFile(NULL)) != NULL)
  {
    Object *componentList;
    Object *componentHistory;
    Object *skipInFutureCheckBox;
    Object *downloadButton;
    Object *closeButton;

    if((obj = DoSuperNew(cl, obj,

      MUIA_Window_ID,         MAKE_ID('U','P','D','1'),
      MUIA_Window_Height,     MUIV_Window_Height_MinMax(30),
      MUIA_Window_Width,      MUIV_Window_Width_MinMax(30),
      MUIA_Window_RefWindow,  G->MA->GUI.WI,
      WindowContents, VGroup,

        GroupSpacing(0),
        Child, HGroup,
          Child, MakeImageObject("config_update_big", G->theme.configImages[CI_UPDATEBIG]),
          Child, VGroup,
            Child, TextObject,
              MUIA_Text_PreParse, "\033b",
              MUIA_Text_Contents, tr(MSG_UPD_NOTIFICATION_TITLE),
              MUIA_Text_Copy,     FALSE,
              MUIA_Weight,        100,
            End,
            Child, TextObject,
              MUIA_Text_Contents, tr(MSG_UPD_NOTIFICATION_SUMMARY),
              MUIA_Text_Copy,     FALSE,
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
          MUIA_NListview_NList, componentList = UpdateComponentListObject,
          End,
        End,

        Child, NBalanceObject,
          MUIA_Balance_Quiet, TRUE,
        End,

        Child, TextObject,
          MUIA_Text_Contents, tr(MSG_UPD_NOTIFICATION_CHANGES),
          MUIA_Text_Copy,     FALSE,
          MUIA_Font,          MUIV_Font_Tiny,
        End,
        Child, NListviewObject,
          MUIA_CycleChain, TRUE,
          MUIA_NListview_NList, componentHistory = NFloattextObject,
            MUIA_Font,             MUIV_Font_Fixed,
            MUIA_NList_Format,     "P=\33l",
            MUIA_NList_Input,      FALSE,
            MUIA_NFloattext_Text,  "",
          End,
        End,

        Child, HGroup,
          Child, skipInFutureCheckBox = MakeCheck(tr(MSG_UPD_NOTIFICATION_NOUPDATE)),
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
          Child, closeButton = MakeButton(tr(MSG_UPD_NOTIFICATION_CLOSE)),
          Child, downloadButton = MakeButton(tr(MSG_UPD_NOTIFICATION_DOWNLOAD)),
        End,

      End,

      TAG_MORE, inittags(msg))) != NULL)
    {
      GETDATA;

      DoMethod(G->App, OM_ADDMEMBER, obj);

      data->componentList = componentList;
      data->componentHistory = componentHistory;
      data->skipInFutureCheckBox = skipInFutureCheckBox;
      data->downloadButton = downloadButton;
      data->tempFile = tempFile;

      // start with a disabled "Download" button
      set(downloadButton, MUIA_Disabled, TRUE);

      DoMethod(obj,              MUIM_Notify, MUIA_Window_CloseRequest, TRUE, MUIV_Notify_Self, 3, METHOD(Close));
      DoMethod(componentList,    MUIM_Notify, MUIA_NList_Active, MUIV_EveryTime, obj, 2, METHOD(Select), MUIV_TriggerValue);
      DoMethod(componentHistory, MUIM_Notify, MUIA_NList_DoubleClick, MUIV_EveryTime, obj, 1, METHOD(Download));
      DoMethod(downloadButton,   MUIM_Notify, MUIA_Pressed, FALSE, obj, 1, METHOD(Download));
      DoMethod(closeButton,      MUIM_Notify, MUIA_Pressed, FALSE, obj, 3, METHOD(Close));

      set(obj, MUIA_Window_Activate, TRUE);
    }
    else
      CloseTempFile(tempFile);
  }
  else
    obj = NULL;

  RETURN((IPTR)obj);
  return (IPTR)obj;
}

///
/// OVERLOAD(OM_DISPOSE)
OVERLOAD(OM_DISPOSE)
{
  GETDATA;

  // remove the notification
  DoMethod(G->App, MUIM_KillNotifyObj, MUIA_Application_Iconified, obj);

  // close the downloaded update file
  CloseTempFile(data->tempFile);

  // free the changelog text
  free(data->changeLogText);
  data->changeLogText = NULL;

  return DoSuperMethodA(cl, obj, msg);
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
        if(tag->ti_Data != FALSE)
        {
          char buf[64];

          // setup some options and select the first entry at the top
          set(data->skipInFutureCheckBox, MUIA_Selected, C->UpdateInterval == 0);
          set(data->componentList, MUIA_NList_Active, MUIV_NList_Active_Top);

          // we now specify the window title as we add the date/time to it
          DateStamp2String(buf, sizeof(buf), NULL, DSS_DATETIME, TZC_NONE);
          snprintf(data->windowTitle, sizeof(data->windowTitle), "%s - %s", tr(MSG_UPD_NOTIFICATION_WTITLE), buf);

          xset(obj, MUIA_Window_Title,         data->windowTitle,
                    MUIA_Window_ScreenTitle,   CreateScreenTitle(data->screenTitle, sizeof(data->screenTitle), data->windowTitle),
                    MUIA_Window_DefaultObject, data->componentList);

          // we also make sure the application in uniconified.
          if(xget(_app(obj), MUIA_Application_Iconified))
            set(_app(obj), MUIA_Application_Iconified, FALSE);
        }
      }
      break;
    }
  }

  return DoSuperMethodA(cl, obj, msg);
}

///
/// OVERLOAD(MUIM_ThreadFinished)
OVERLOAD(MUIM_ThreadFinished)
{
  GETDATA;
  struct MUIP_ThreadFinished *tf = (struct MUIP_ThreadFinished *)msg;

  ENTER();

  // remember the update check result
  data->updateSuccess = tf->result;

  DoMethod(obj, METHOD(ParseUpdateFile));

  BusyEnd(data->busy);
  data->busy = NULL;

  RETURN(0);
  return 0;
}

///

/* Public Methods */
/// DECLARE(CheckForUpdates)
// set up and send an update request
DECLARE(CheckForUpdates) // ULONG quiet
{
  GETDATA;
  char *request;

  ENTER();

  if((request = BuildUpdateRequest()) != NULL)
  {
    D(DBF_UPDATE, "send update request '%s' (%ld)", request, strlen(request));

    data->busy = BusyBegin(BUSY_TEXT);
    BusyText(data->busy, tr(MSG_BusyGettingVerInfo), "");

    // now we send a specific request via DownloadURL() to our update server
    DoAction(obj, TA_DownloadURL, TT_DownloadURL_Server, C->UpdateServer,
                                  TT_DownloadURL_Request, request,
                                  TT_DownloadURL_Filename, data->tempFile->Filename,
                                  TT_DownloadURL_Flags, DLURLF_NO_ERROR_ON_404,
                                  TAG_DONE);
    free(request);

    // remember the "quiet" state of this check
    data->quiet = msg->quiet;
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(Clear)
DECLARE(Clear)
{
  GETDATA;

  ENTER();

  DoMethod(data->componentList, MUIM_NList_Clear);
  set(data->componentHistory, MUIA_NFloattext_Text, "");

  free(data->changeLogText);
  data->changeLogText = NULL;

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

  DoMethod(data->componentList, MUIM_NList_GetEntry, msg->num, &comp);

  // disable the "Download" button in case we found no valid component
  set(data->downloadButton, MUIA_Disabled, comp == NULL);

  if(comp != NULL && comp->changeLogFile != NULL)
  {
    LONG size;

    // lets open the changelog file and parse it
    if(ObtainFileInfo(comp->changeLogFile->Filename, FI_SIZE, &size) == TRUE && size > 0)
    {
      if((comp->changeLogFile->FP = fopen(comp->changeLogFile->Filename, "r")) != NULL)
      {
        free(data->changeLogText);

        if((data->changeLogText = malloc(size+1)) != NULL)
        {
          if(fread(data->changeLogText, size, 1, comp->changeLogFile->FP) == 1)
          {
            data->changeLogText[size] = '\0';

            set(data->componentHistory, MUIA_NFloattext_Text, data->changeLogText);
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
  DoMethod(data->componentList, MUIM_NList_InsertSingle, msg->comp, MUIV_NList_Insert_Sorted);

  RETURN(0);
  return 0;
}

///
/// DECLARE(Download)
DECLARE(Download)
{
  GETDATA;
  struct UpdateComponent *comp = NULL;

  ENTER();

  DoMethod(data->componentList, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &comp);
  if(comp != NULL)
  {
    char urlFile[SIZE_FILE];

    if(ExtractUpdateFilename(comp->url, urlFile, sizeof(urlFile)) == TRUE)
    {
      const char *drawer = C->UpdateDownloadPath;
      const char *file = urlFile;
      BOOL doDownload = TRUE;
      char path[SIZE_PATHFILE];

      do
      {
        struct FileReqCache *frc;

        if((frc = ReqFile(ASL_UPDATE, obj, tr(MSG_RE_SAVE_FILE), REQF_SAVEMODE, drawer, file)) != NULL)
        {
          AddPath(path, frc->drawer, frc->file, sizeof(path));

          if(FileExists(path) == TRUE)
          {
            if(MUI_Request(_app(obj), obj, MUIF_NONE, tr(MSG_MA_ConfirmReq), tr(MSG_YesNoReq), tr(MSG_FILE_OVERWRITE), path) == 0)
            {
              // user chose not to overwrite the existing file, let him choose another one
            }
            else
            {
              // download the update to the selected file and overwrite the existing one
              break;
            }
          }
          else
          {
            // download the update to the selected file
            break;
          }
        }
        else
        {
          // file selection canceled, don't download
          doDownload = FALSE;
        }
      }
      while(doDownload == TRUE);

      if(doDownload == TRUE)
      {
        // start the download
        DoAction(NULL, TA_DownloadURL, TT_DownloadURL_Server, comp->url,
                                       TT_DownloadURL_Filename, path,
                                       TT_DownloadURL_Flags, DLURLF_VISIBLE,
                                       TAG_DONE);
      }
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
  if(xget(data->skipInFutureCheckBox, MUIA_Selected) == TRUE)
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

  if(CE != NULL)
  {
    // now we make sure the C and CE config structure is in sync again
    CE->UpdateInterval = C->UpdateInterval;
  }

  // make sure the update check config page is correctly refreshed
  if(G->ConfigWinObject != NULL)
    DoMethod(G->ConfigWinObject, MUIM_ConfigWindow_GUIToConfig, cp_Update);

  // now close the window for real.
  set(obj, MUIA_Window_Open, FALSE);

  RETURN(0);
  return 0;
}

///
/// DECLARE(ParseUpdateFile)
// parse the downloaded update file
DECLARE(ParseUpdateFile)
{
  GETDATA;

  ENTER();

  // the thread which did the download has finished
  // now parse the update file if the download was successful
  if(data->updateSuccess == TRUE)
    ParseUpdateFile(data->tempFile->Filename, data->quiet);

  RETURN(0);
  return 0;
}

///
