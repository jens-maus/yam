/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2017 YAM Open Source Team

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
 Description: Splash/Startup window of the application

***************************************************************************/

#include "SplashWindow_cl.h"

#include <string.h>
#include <proto/dos.h>
#include <proto/muimaster.h>
#include <mui/BetterString_mcc.h>

#include "YAM.h"
#include "YAM_global.h"
#include "YAM_utilities.h"

#include "Busy.h"
#include "Locale.h"
#include "MUIObjects.h"

#include "mui/ImageArea.h"
#include "mui/YAMApplication.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *windowGroup;
  Object *statusGauge;
  Object *progressGauge;
  BOOL progressGaugeActive;
  struct TimeVal last_gaugemove;
  APTR lastBusy;
  char statusInfoText[SIZE_DEFAULT];
  char progressInfoText[SIZE_DEFAULT];
};
*/

/* INCLUDE
#include "timeval.h"
*/

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  Object *progressGauge;

  // create the progressGauge first
  if((progressGauge = GaugeObject,
    GaugeFrame,
    MUIA_Gauge_InfoText, "",
    MUIA_Gauge_Horiz,    TRUE,
  End) != NULL)
  {
    char logopath[SIZE_PATHFILE];
    char *compileInfo;
    Object *windowGroup;
    Object *statusGauge;

    compileInfo = (char *)xget(G->App, MUIA_YAMApplication_CompileInfo);

    AddPath(logopath, G->ThemesDir, "default/logo", sizeof(logopath));

    if((obj = DoSuperNew(cl, obj,

      MUIA_Window_DragBar,     FALSE,
      MUIA_Window_CloseGadget, FALSE,
      MUIA_Window_DepthGadget, FALSE,
      MUIA_Window_SizeGadget,  FALSE,
      MUIA_Window_LeftEdge,    MUIV_Window_LeftEdge_Centered,
      MUIA_Window_TopEdge,     MUIV_Window_TopEdge_Centered,
      MUIA_Window_Activate,    TRUE,
      WindowContents, windowGroup = VGroup,
        MUIA_Background, MUII_GroupBack,
        Child, HGroup,
          MUIA_Group_Spacing, 0,
          Child, HSpace(0),
          Child, MakeImageObject("logo", logopath),
          Child, HSpace(0),
        End,
        Child, HCenter((VGroup,
          Child, CLabel(tr(MSG_YAMINFO)),
          Child, CLabel(yamfullcopyright),
          Child, TextObject,
            MUIA_Text_PreParse, "\033c\033u\0335",
            MUIA_Text_Contents, yamurl,
            MUIA_Text_Copy,     FALSE,
          End,
          Child, RectangleObject,
            MUIA_Rectangle_HBar, TRUE,
            MUIA_FixHeight, 8,
          End,
          Child, ColGroup(2),
            MUIA_Font, MUIV_Font_Tiny,
            MUIA_Group_HorizSpacing, 8,
            MUIA_Group_VertSpacing, 2,
            Child, Label(tr(MSG_Version)),
            Child, LLabel(yamversionver),
            Child, Label(tr(MSG_CompilationDate)),
            Child, LLabel(compileInfo),
          End,
        End)),
        Child, statusGauge = GaugeObject,
          GaugeFrame,
          MUIA_Gauge_InfoText, "",
          MUIA_Gauge_Horiz,    TRUE,
        End,
      End,

      TAG_MORE, inittags(msg))) != NULL)
    {
      GETDATA;

      DoMethod(G->App, OM_ADDMEMBER, obj);

      data->windowGroup = windowGroup;
      data->statusGauge = statusGauge;
      data->progressGauge = progressGauge;
      data->progressGaugeActive = FALSE;
    }
    else
      MUI_DisposeObject(progressGauge);
  }

  return (IPTR)obj;
}

///
/// OVERLOAD(OM_DISPOSE)
OVERLOAD(OM_DISPOSE)
{
  GETDATA;

  // only remove the select and progressGauge if they are not
  // already part of the mainwindow.
  if(data->progressGaugeActive == FALSE)
    MUI_DisposeObject(data->progressGauge);

  return DoSuperMethodA(cl, obj, msg);
}

///

/* Private Functions */

/* Public Methods */
/// DECLARE(StatusChange)
DECLARE(StatusChange) // char *txt, LONG percent
{
  GETDATA;

  ENTER();

  // remember the text in case the progress gauge needs to be shown
  strlcpy(data->statusInfoText, msg->txt, sizeof(data->statusInfoText));

  xset(data->statusGauge,
    MUIA_Gauge_InfoText, msg->txt,
    MUIA_Gauge_Current,  msg->percent);

  DoMethod(_app(obj), MUIM_Application_InputBuffered);

  RETURN(0);
  return 0;
}

///
/// DECLARE(ProgressChange)
DECLARE(ProgressChange) // struct BusyNode *busy
{
  GETDATA;

  ENTER();

  // update the progress bar whenever another busy action is to be shown,
  // or if the same busy action needs an update and enough time since the last update has passed
  if(msg->busy != data->lastBusy || TimeHasElapsed(&data->last_gaugemove, 250000) == TRUE)
  {
    if(msg->busy != NULL)
    {
      // we need valid gauge limits to be able to show the gauge
      if(msg->busy->progressMax > 0 && msg->busy->progressCurrent <= msg->busy->progressMax)
      {
        set(data->statusGauge, MUIA_Gauge_InfoText, msg->busy->infoText);

        if(msg->busy != data->lastBusy)
        {
          // clear the last gaugemove timeval structure.
          memset(&data->last_gaugemove, 0, sizeof(data->last_gaugemove));
        }

        // update the progress bar if we haven't reached 100% yet
        if(msg->busy->progressCurrent < msg->busy->progressMax)
        {
          snprintf(data->progressInfoText, sizeof(data->progressInfoText), "%%ld/%ld", msg->busy->progressMax);
          xset(data->progressGauge,
            MUIA_Gauge_InfoText, data->progressInfoText,
            MUIA_Gauge_Max, msg->busy->progressMax,
            MUIA_Gauge_Current, msg->busy->progressCurrent);

          // add the progress gauge if that has not been done yet
          if(data->progressGaugeActive == FALSE &&
             DoMethod(data->windowGroup, MUIM_Group_InitChange))
          {
            DoMethod(data->windowGroup, OM_ADDMEMBER, data->progressGauge);
            data->progressGaugeActive = TRUE;

            DoMethod(data->windowGroup, MUIM_Group_ExitChange);
          }
        }
      }
    }

    if(msg->busy == NULL || msg->busy->progressCurrent >= msg->busy->progressMax)
    {
      // 100% reached or the progress bar is no longer needed
      // remove the progress bar
      if(data->progressGaugeActive == TRUE &&
         DoMethod(data->windowGroup, MUIM_Group_InitChange))
      {
        DoMethod(data->windowGroup, OM_REMMEMBER, data->progressGauge);
        data->progressGaugeActive = FALSE;

        // restore the old status text
        set(data->statusGauge, MUIA_Gauge_InfoText, data->statusInfoText);

        DoMethod(data->windowGroup, MUIM_Group_ExitChange);
      }
    }

    // remember the changed busy action
    data->lastBusy = msg->busy;
  }

  DoMethod(_app(obj), MUIM_Application_InputBuffered);

  RETURN(0);
  return 0;
}

///
/// DECLARE(SelectUser)
DECLARE(SelectUser)
{
  GETDATA;
  LONG user = -1;
  Object *selectGroup;
  Object *userGroup;

  ENTER();

  // create the selectionGroup manually as we add/remove
  // it manually later on
  selectGroup = VGroup,
    Child, RectangleObject,
      MUIA_Rectangle_HBar, TRUE,
      MUIA_FixHeight, 8,
    End,
    Child, userGroup = VGroup,
      Child, TextObject,
        MUIA_Text_Contents, tr(MSG_UserLogin),
        MUIA_Text_PreParse, MUIX_C,
        MUIA_Text_Copy,     FALSE,
      End,
    End,
    Child, HVSpace,
  End;

  if(selectGroup != NULL)
  {
    Object *group = ColGroup(2), End;
    Object *button0 = NULL;
    BOOL wasIconified;
    BOOL wasOpen;

    if(DoMethod(userGroup, MUIM_Group_InitChange))
    {
      int i;

      for(i = 0; i < G->Users.Num; i++)
      {
        Object *button = MakeButton(G->Users.User[i].Name);

        if(i == 0)
          button0 = button;

        DoMethod(group, OM_ADDMEMBER, button);
        DoMethod(button, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 2, MUIM_Application_ReturnID, ID_LOGIN+i);
      }

      if(i%2 == 1)
        DoMethod(group, OM_ADDMEMBER, HSpace(0));

      DoMethod(userGroup, OM_ADDMEMBER, group);
      DoMethod(userGroup, MUIM_Group_ExitChange);
    }

    if(DoMethod(data->windowGroup, MUIM_Group_InitChange))
    {
      ULONG signals;

      DoMethod(data->windowGroup, OM_ADDMEMBER, selectGroup);
      DoMethod(data->windowGroup, MUIM_Group_ExitChange);

      set(data->statusGauge, MUIA_Gauge_InfoText, tr(MSG_US_WaitLogin));

      // make sure the window is open and not iconified
      wasOpen = xget(obj, MUIA_Window_Open);
      wasIconified = xget(_app(obj), MUIA_Application_Iconified);

      if(wasOpen == FALSE)
        set(obj, MUIA_Window_Open, TRUE);

      if(wasIconified == TRUE)
        set(_app(obj), MUIA_Application_Iconified, FALSE);

      // we add the esc key to the input event of the requester and if we receive it we close the requester by safely
      // exiting with the last button
      DoMethod(obj, MUIM_Notify, MUIA_Window_CloseRequest, TRUE, MUIV_Notify_Application, 2, MUIM_Application_ReturnID, ID_LOGIN+G->Users.Num);

      // make sure the window is at the front
      DoMethod(obj, MUIM_Window_ToFront);

      // make the first button the active object in the window
      // This cannot be done directly by a simple set(), because the window needs to be
      // resized to make the string object appear. Without this resize action the object
      // is considered to be invisible and hence MUI will refuse to activate it. But the
      // resizing is not done before Intuition sends the necessary IDCMP_NEWSIZE message
      // to the window which will be handled in the MUIM_Application_NewInput method.
      // The solution is to delay the activation a bit.
      DoMethod(_app(obj), MUIM_Application_PushMethod, obj, 3|MUIV_PushMethod_Delay(100), MUIM_Set, MUIA_Window_ActiveObject, button0);

      // lets collect the waiting returnIDs now
      COLLECT_RETURNIDS;

      signals = 0;
      do
      {
        LONG ret = DoMethod(_app(obj), MUIM_Application_NewInput, &signals)-ID_LOGIN;

        // bail out if a button was hit
        if(ret >= 0 && ret < G->Users.Num)
        {
          user = ret;
          break;
        }
        else if(ret == G->Users.Num)
        {
          // ESC key pressed
          user = -1;
          break;
        }

        if(signals)
          signals = Wait(signals);
      }
      while(TRUE);

      // now lets reissue the collected returnIDs again
      REISSUE_RETURNIDS;

      // kill the window notifies afterwards again
      DoMethod(obj, MUIM_KillNotify, MUIA_Window_CloseRequest);

      // make sure no object of the window is active afterwards
      set(obj, MUIA_Window_ActiveObject, NULL);

      // lets iconify/close the window if it had this state previously
      if(wasOpen == FALSE)
        set(obj, MUIA_Window_Open, FALSE);

      if(wasIconified == TRUE)
        set(_app(obj), MUIA_Application_Iconified, TRUE);

      if(DoMethod(data->windowGroup, MUIM_Group_InitChange))
      {
        DoMethod(data->windowGroup, OM_REMMEMBER, selectGroup);
        DoMethod(data->windowGroup, MUIM_Group_ExitChange);
      }
    }
  }

  DoMethod(_app(obj), MUIM_Application_InputBuffered);

  RETURN(user);
  return (ULONG)user;
}

///
/// DECLARE(PasswordRequest)
DECLARE(PasswordRequest) // struct User *user
{
  GETDATA;
  Object *pwGroup;
  Object *pwString;
  Object *bt_okay;
  Object *bt_cancel;
  BOOL result = FALSE;

  ENTER();

  // create the passwordGroup object now
  pwGroup = VGroup,
    Child, RectangleObject,
       MUIA_Rectangle_HBar, TRUE,
       MUIA_FixHeight, 8,
    End,
    Child, CLabel(tr(MSG_US_EnterPassword)),
    Child, pwString = BetterStringObject,
      StringFrame,
      MUIA_BetterString_StayActive, TRUE,
      MUIA_String_MaxLen,           SIZE_PASSWORD,
      MUIA_String_Secret,           TRUE,
      MUIA_CycleChain,              TRUE,
    End,
    Child, ColGroup(2),
      Child, bt_okay = MakeButton(tr(MSG_Okay)),
      Child, bt_cancel = MakeButton(tr(MSG_Cancel)),
    End,
    Child, HVSpace,
  End;

  if(pwGroup != NULL)
  {
    if(DoMethod(data->windowGroup, MUIM_Group_InitChange))
    {
      BOOL wasIconified;
      BOOL wasOpen;
      ULONG signals;
      LONG ret;

      DoMethod(data->windowGroup, OM_ADDMEMBER, pwGroup);
      DoMethod(data->windowGroup, MUIM_Group_ExitChange);

      set(data->statusGauge, MUIA_Gauge_InfoText, tr(MSG_US_WaitLogin));

      // place the returnID notifies
      DoMethod(bt_okay,   MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 2, MUIM_Application_ReturnID, ID_LOGIN+1);
      DoMethod(bt_cancel, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 2, MUIM_Application_ReturnID, ID_LOGIN+2);
      DoMethod(pwString,  MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime, MUIV_Notify_Application, 2, MUIM_Application_ReturnID, ID_LOGIN+1);
      DoMethod(obj,       MUIM_Notify, MUIA_Window_CloseRequest, TRUE, MUIV_Notify_Application, 2, MUIM_Application_ReturnID, ID_LOGIN+2);

      // make sure the window is open and not iconified
      wasOpen = xget(obj, MUIA_Window_Open);
      wasIconified = xget(_app(obj), MUIA_Application_Iconified);

      if(wasOpen == FALSE)
        set(obj, MUIA_Window_Open, TRUE);

      if(wasIconified == TRUE)
        set(_app(obj), MUIA_Application_Iconified, FALSE);

      // make sure the window is at the front
      DoMethod(obj, MUIM_Window_ToFront);

      // lets collect the waiting returnIDs now
      COLLECT_RETURNIDS;

      // make the passwordString the active object
      // This cannot be done directly by a simple set(), because the window needs to be
      // resized to make the string object appear. Without this resize action the object
      // is considered to be invisible and hence MUI will refuse to activate it. But the
      // resizing is not done before Intuition sends the necessary IDCMP_NEWSIZE message
      // to the window which will be handled in the MUIM_Application_NewInput method.
      // The solution is to delay the activation a bit.
      DoMethod(_app(obj), MUIM_Application_PushMethod, obj, 3|MUIV_PushMethod_Delay(100), MUIM_Set, MUIA_Window_ActiveObject, pwString);

      signals = 0;
      do
      {
        ret = DoMethod(_app(obj), MUIM_Application_NewInput, &signals)-ID_LOGIN;

        // if the returnID is 1 then we check the password against the supplied
        // password
        if(ret == 1)
        {
          if(strcmp((char *)xget(pwString, MUIA_String_Contents), msg->user->Password) == 0)
          {
            result = TRUE;
            break;
          }

          DisplayBeep(_screen(obj));
        }
        else if(ret == 2)
          break;

        if(signals != 0)
          signals = Wait(signals);
      }
      while(TRUE);

      // now lets reissue the collected returnIDs again
      REISSUE_RETURNIDS;

      // kill the window notifies afterwards again
      DoMethod(obj, MUIM_KillNotify, MUIA_Window_CloseRequest);

      // make sure no object of the window is active afterwards
      set(obj, MUIA_Window_ActiveObject,  NULL);

      // lets iconify/close the window if it had this state previously
      if(wasOpen == FALSE)
        set(obj, MUIA_Window_Open, FALSE);

      if(wasIconified == TRUE)
        set(_app(obj), MUIA_Application_Iconified, TRUE);

      // remove the passwordRequest again
      if(DoMethod(data->windowGroup, MUIM_Group_InitChange))
      {
        DoMethod(data->windowGroup, OM_REMMEMBER, pwGroup);
        MUI_DisposeObject(pwGroup);

        DoMethod(data->windowGroup, MUIM_Group_ExitChange);
      }
    }
  }

  DoMethod(_app(obj), MUIM_Application_InputBuffered);

  RETURN(result);
  return result;
}

///
