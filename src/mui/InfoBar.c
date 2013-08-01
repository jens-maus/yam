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
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.   See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 YAM Official Support Site : http://www.yam.ch
 YAM OpenSource project     : http://sourceforge.net/projects/yamos/

 $Id$

 Superclass:  MUIC_Group
 Description: Displays additional information in the Main window

***************************************************************************/

#include "InfoBar_cl.h"

#include <string.h>

#include <proto/muimaster.h>

#include "YAM.h"
#include "YAM_config.h"
#include "YAM_utilities.h"

#include "Busy.h"
#include "MUIObjects.h"
#include "StrBuf.h"

#include "mui/ImageArea.h"
#include "mui/FolderListtree.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *TX_FOLDER;
  Object *TX_FINFO;
  Object *TX_INFO;
  Object *GA_GROUP;
  Object *GA_INFO;
  Object *GA_LABEL;
  Object *BT_STOP;
  Object *actualImage;
  struct BusyNode *lastBusy;
  BOOL stopButtonPressed;
  struct TimeVal last_gaugemove;
  char gaugeInfoText[SIZE_SMALL];
  char folderInfo[SIZE_DEFAULT / 2];
};
*/

/* INCLUDE
#include "timeval.h"
*/

/* Private Functions */
/// GetFolderInfo()
// this function creates a folder string and returns it
static void GetFolderInfo(struct Data *data, struct Folder *folder)
{
  char *src;
  char *info = NULL;

  ENTER();

  // Lets create the label of the AppIcon now
  for(src = C->InfoBarText; *src; src++)
  {
    char dst[10];

    if(*src == '%')
    {
      switch(*++src)
      {
        case '%': dst[0] = '%'; dst[1] = '\0'; break;
        case 'n': snprintf(dst, sizeof(dst), "%d", folder->New);     break;
        case 'u': snprintf(dst, sizeof(dst), "%d", folder->Unread);  break;
        case 't': snprintf(dst, sizeof(dst), "%d", folder->Total);   break;
        case 's': snprintf(dst, sizeof(dst), "%d", folder->Sent);    break;
        case 'd': snprintf(dst, sizeof(dst), "%d", folder->Deleted); break;
      }
    }
    else
    {
      dst[0] = *src;
      dst[1] = '\0';
    }

    StrBufCat(&info, dst);
  }

  if(info != NULL)
  {
    strlcpy(data->folderInfo, info, sizeof(data->folderInfo));
    FreeStrBuf(info);
  }
  else
    data->folderInfo[0] = '\0';


  LEAVE();
}

///

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  Object *folderString;
  Object *folderInfoStr;
  Object *statusGroup;
  Object *gauge;
  Object *gaugeLabel;
  Object *infoText;
  Object *stopButton;

  ENTER();

  if((obj = DoSuperNew(cl, obj,

    // Some objects are allowed to disappear if the available space in the window is too narrow.
    // This is a workaround for a bug in MUI.
    TextFrame,
    MUIA_Background,    MUII_TextBack,
    MUIA_Group_Horiz,   TRUE,
    Child, HGroup,
      InnerSpacing(0,0),
      Child, folderString = TextObject,
        MUIA_HorizWeight,    0,
        MUIA_HorizDisappear, 2,
        MUIA_Font,           MUIV_Font_Big,
        MUIA_Text_SetMax,    FALSE,
        MUIA_Text_PreParse,  "\033b",
        MUIA_Text_Copy,      FALSE,
      End,
      Child, folderInfoStr = TextObject,
        MUIA_HorizWeight,    100,
        MUIA_HorizDisappear, 1,
        MUIA_Font,           MUIV_Font_Tiny,
        MUIA_Text_SetMax,    FALSE,
        MUIA_Text_PreParse,  "\033l",
        MUIA_Text_Copy,      FALSE,
      End,
    End,

    Child, HGroup,

      Child, gaugeLabel = TextObject,
        MUIA_HorizDisappear, 1,
        MUIA_Text_SetMax,   FALSE,
        MUIA_Text_PreParse, "\033r",
        MUIA_Text_Copy,     FALSE,
      End,

      Child, statusGroup = PageGroup,
        MUIA_HorizDisappear, 3,
        Child, HSpace(0),
        Child, HGroup,
          InnerSpacing(0,0),
          GroupSpacing(1),
          Child, gauge = GaugeObject,
            GaugeFrame,
            MUIA_Gauge_Horiz,    TRUE,
            MUIA_Gauge_InfoText, " ",
          End,
          Child, stopButton = TextObject,
            ButtonFrame,
            MUIA_CycleChain,     TRUE,
            MUIA_Font,           MUIV_Font_Tiny,
            MUIA_InputMode,      MUIV_InputMode_RelVerify,
            MUIA_Background,     MUII_ButtonBack,
            MUIA_Text_SetMax,    TRUE,
            MUIA_Text_Copy,      FALSE,
            MUIA_Text_PreParse,  "\033b",
            MUIA_Text_Contents,  "X",
          End,
        End,
        Child, infoText = TextObject,
          MUIA_Text_SetMax,   FALSE,
          MUIA_Text_PreParse, "\033r",
          MUIA_Text_Copy,     FALSE,
        End,
      End,
    End,

    TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;

    // per default we set the stop button as hidden
    set(stopButton, MUIA_ShowMe, FALSE);

    data->TX_FOLDER = folderString;
    data->TX_FINFO  = folderInfoStr;
    data->GA_GROUP  = statusGroup;
    data->GA_LABEL  = gaugeLabel;
    data->GA_INFO   = gauge;
    data->TX_INFO   = infoText;
    data->BT_STOP   = stopButton;
    data->stopButtonPressed = FALSE;

    // on a button press on the stop button, lets set the
    // correct variable to TRUE as well.
    DoMethod(data->BT_STOP, MUIM_Notify, MUIA_Pressed, FALSE, obj, 1, METHOD(StopProcess));
  }

  RETURN((IPTR)obj);
  return (IPTR)obj;
}

///

/* Public Methods */
/// DECLARE(SetFolder)
// set a new folder and update its name and image in the infobar
DECLARE(SetFolder) // struct Folder *newFolder
{
  GETDATA;
  struct Folder *folder = msg->newFolder;
  IPTR result = -1;

  ENTER();

  if(folder != NULL)
  {
    GetFolderInfo(data, folder);

    // set the name of the folder as the info text
    nnset(data->TX_FOLDER, MUIA_Text_Contents, folder->Name);
    // now we are going to set some status field at the right side of the folder name
    nnset(data->TX_FINFO, MUIA_Text_Contents, data->folderInfo);

    // prepare the object for a change
    if(DoMethod(obj, MUIM_Group_InitChange))
    {
      // only if the image should be changed we proceed or otherwise
      // MUI will refresh too often
      if(data->actualImage != NULL && (folder->imageObject == NULL ||
         stricmp((char *)xget(data->actualImage, MUIA_ImageArea_ID),
                 (char *)xget(folder->imageObject, MUIA_ImageArea_ID)) != 0))
      {
        DoMethod(obj, OM_REMMEMBER, data->actualImage);

        D(DBF_GUI, "disposing folder image: id '%s' file '%s'", xget(data->actualImage, MUIA_ImageArea_ID), xget(data->actualImage, MUIA_ImageArea_Filename));
        MUI_DisposeObject(data->actualImage);
        data->actualImage = NULL;
      }

      // and if we have a new one we generate the object an add it
      // to the grouplist of this infobar
      if(data->actualImage == NULL)
      {
        if(folder->imageObject != NULL)
        {
          char *imageID = (char *)xget(folder->imageObject, MUIA_ImageArea_ID);
          char *imageName = (char *)xget(folder->imageObject, MUIA_ImageArea_Filename);

          data->actualImage = MakeImageObject(imageID, imageName);

          D(DBF_GUI, "init imagearea: id '%s', file '%s'", imageID, imageName);
        }
        else if(folder->ImageIndex >= 0 && folder->ImageIndex < FICON_ID_MAX)
        {
          Object **imageArray = (Object **)xget(G->MA->GUI.NL_FOLDERS, MUIA_FolderListtree_ImageArray);

          D(DBF_GUI, "init imagearea: 0x%08lx[%ld]", imageArray, folder->ImageIndex);

          if(imageArray != NULL && imageArray[folder->ImageIndex] != NULL)
          {
            char *imageID = (char *)xget(imageArray[folder->ImageIndex], MUIA_ImageArea_ID);
            char *imageName = (char *)xget(imageArray[folder->ImageIndex], MUIA_ImageArea_Filename);

            data->actualImage = MakeImageObject(imageID, imageName);
          }
        }

        D(DBF_GUI, "init finished..: 0x%08lx %ld", data->actualImage, folder->ImageIndex);

        if(data->actualImage != NULL)
          DoMethod(obj, OM_ADDMEMBER, data->actualImage);
      }

      // now that we are finished we can call ExitChange to refresh the infobar
      DoMethod(obj, MUIM_Group_ExitChange);
    }

    result = 0;
  }

  RETURN(result);
  return result;
}

///
/// DECLARE(ShowBusyBar)
// show a busy bar with either a simple info text or a progress gauge
DECLARE(ShowBusyBar) // struct BusyNode *busy
{
  GETDATA;
  BOOL goOn = TRUE;

  ENTER();

  if(msg->busy != NULL)
  {
    switch(msg->busy->type)
    {
      case BUSY_TEXT:
      {
        set(data->TX_INFO, MUIA_Text_Contents, msg->busy->infoText);
        set(data->GA_LABEL, MUIA_Text_Contents, NULL);
        set(data->GA_GROUP, MUIA_Group_ActivePage, 2);
      }
      break;

      case BUSY_PROGRESS:
      {
        // update the busy bar whenever another busy action is to be shown,
        // or if the same busy action needs an update and enough time since the last update has passed
        if(msg->busy != data->lastBusy || TimeHasElapsed(&data->last_gaugemove, 250000) == TRUE)
        {
          // we need valid gauge limits to be able to show the gauge
          if(msg->busy->progressMax > 0 && msg->busy->progressCurrent <= msg->busy->progressMax)
          {
            set(data->GA_LABEL, MUIA_Text_Contents, msg->busy->infoText);

            snprintf(data->gaugeInfoText, sizeof(data->gaugeInfoText), "%%ld/%ld", msg->busy->progressMax);
            xset(data->GA_INFO,
                   MUIA_Gauge_InfoText,  data->gaugeInfoText,
                   MUIA_Gauge_Max,       msg->busy->progressMax,
                   MUIA_Gauge_Current,   msg->busy->progressCurrent);
            set(data->BT_STOP, MUIA_ShowMe, FALSE);
            data->stopButtonPressed = FALSE;

            set(data->GA_GROUP, MUIA_Group_ActivePage, 1);
          }
        }
      }
      break;

      case BUSY_PROGRESS_ABORT:
      {
        // update the busy bar whenever another busy action is to be shown,
        // or if the same busy action needs an update and enough time since the last update has passed
        if(msg->busy != data->lastBusy || TimeHasElapsed(&data->last_gaugemove, 250000) == TRUE)
        {
          // we need valid gauge limits to be able to show the gauge
          if(msg->busy->progressMax > 0 && msg->busy->progressCurrent <= msg->busy->progressMax)
          {
            set(data->GA_LABEL, MUIA_Text_Contents, msg->busy->infoText);

            snprintf(data->gaugeInfoText, sizeof(data->gaugeInfoText), "%%ld/%ld", msg->busy->progressMax);
            xset(data->GA_INFO,
                   MUIA_Gauge_InfoText,  data->gaugeInfoText,
                   MUIA_Gauge_Max,       msg->busy->progressMax,
                   MUIA_Gauge_Current,   msg->busy->progressCurrent);
            set(data->BT_STOP, MUIA_ShowMe, TRUE);
            set(data->GA_GROUP, MUIA_Group_ActivePage, 1);

            // give the application the chance to clear its event loop
            DoMethod(_app(obj), MUIM_Application_InputBuffered);

            goOn = (data->stopButtonPressed == FALSE);
          }
        }
      }
      break;
    }
  }
  else
  {
    // hide the busy bar
    set(data->TX_INFO, MUIA_Text_Contents, NULL);
    set(data->GA_LABEL, MUIA_Text_Contents, NULL);
    set(data->GA_GROUP, MUIA_Group_ActivePage, 0);
  }

  // remember the changed busy action
  data->lastBusy = msg->busy;

  RETURN(goOn);
  return goOn;
}

///
/// DECLARE(StopProcess)
DECLARE(StopProcess)
{
  GETDATA;

  ENTER();

  data->stopButtonPressed = TRUE;

  RETURN(0);
  return 0;
}

///
