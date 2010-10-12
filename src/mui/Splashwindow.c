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
 Description: Splash/Startup window of the application

***************************************************************************/

#include "Splashwindow_cl.h"

#include "MUIObjects.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *windowGroup;
  Object *imageGroup;
  Object *textGroup;
  Object *statusGauge;
  Object *progressGroup;
  Object *progressGauge;
  BOOL progressGaugeActive;
  struct TimeVal last_gaugemove;
};
*/

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  Object *progressGauge;

  // create the progressGauge first
  if((progressGauge = GaugeObject,
    GaugeFrame,
    MUIA_Gauge_InfoText, " ",
    MUIA_Gauge_Horiz,     TRUE,
  End) != NULL)
  {
    char logopath[SIZE_PATHFILE];
    char *compileInfo;
    Object *windowGroup;
    Object *imageGroup;
    Object *textGroup;
    Object *statusGauge;
    Object *progressGroup;

    compileInfo = (char *)xget(G->App, MUIA_YAM_CompileInfo);

    AddPath(logopath, G->ProgDir, "Themes/default/logo", sizeof(logopath));

    if((obj = DoSuperNew(cl, obj,

      MUIA_Window_DragBar,        FALSE,
      MUIA_Window_CloseGadget,    FALSE,
      MUIA_Window_DepthGadget,    FALSE,
      MUIA_Window_SizeGadget,     FALSE,
      MUIA_Window_LeftEdge,       MUIV_Window_LeftEdge_Centered,
      MUIA_Window_TopEdge,        MUIV_Window_TopEdge_Centered,
      MUIA_Window_ActiveObject,   NULL,
      MUIA_Window_DefaultObject,  NULL,
      WindowContents, windowGroup = VGroup,
        MUIA_Background, MUII_GroupBack,
        Child, imageGroup = HGroup,
          MUIA_Group_Spacing, 0,
          Child, HSpace(0),
          Child, MakeImageObject("logo", logopath),
          Child, HSpace(0),
        End,
        Child, textGroup = HCenter((VGroup,
          Child, CLabel(tr(MSG_YAMINFO)),
          Child, CLabel(yamfullcopyright),
          Child, ColGroup(2),
            Child, TextObject,
              MUIA_Text_Contents, "\033c\033u\0335http://www.yam.ch/",
            End,
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
          MUIA_Gauge_InfoText, " ",
          MUIA_Gauge_Horiz,    TRUE,
        End,
        Child, progressGroup = PageGroup,
          MUIA_Group_ActivePage, 0,
          Child, HVSpace,
        End,
      End,

      TAG_MORE, inittags(msg))) != NULL)
    {
      GETDATA;

      DoMethod(G->App, OM_ADDMEMBER, obj);

      data->windowGroup   = windowGroup;
      data->imageGroup    = imageGroup;
      data->textGroup     = textGroup;
      data->statusGauge   = statusGauge;
      data->progressGroup = progressGroup;
      data->progressGauge = progressGauge;
      data->progressGaugeActive = FALSE;

      set(obj, MUIA_Window_Activate, TRUE);
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
  if(isChildOfGroup(data->windowGroup, data->progressGauge) == FALSE)
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

  if(msg->txt && msg->percent >= 0)
  {
    xset(data->statusGauge, MUIA_Gauge_InfoText, msg->txt,
                            MUIA_Gauge_Current,  msg->percent);
  }
  else if(msg->txt)
    set(data->statusGauge, MUIA_Gauge_InfoText, msg->txt);
  else
    set(data->statusGauge, MUIA_Gauge_Current, msg->percent);

  set(data->progressGroup, MUIA_Group_ActivePage, 0);

  // lets remove the progress Gauge from our splashwindow
  if(data->progressGaugeActive == TRUE &&
     DoMethod(data->progressGroup, MUIM_Group_InitChange))
  {
    DoMethod(data->progressGroup, OM_REMMEMBER, data->progressGauge);
    data->progressGaugeActive = FALSE;

    DoMethod(data->progressGroup, MUIM_Group_ExitChange);
  }

  DoMethod(G->App, MUIM_Application_InputBuffered);

  return 0;
}

///
/// DECLARE(ProgressChange)
DECLARE(ProgressChange) // char *txt, LONG percent, LONG max
{
  GETDATA;
  BOOL updateStatus = FALSE;

  ENTER();

  if(msg->txt != NULL)
  {
    set(data->progressGauge, MUIA_Gauge_InfoText, msg->txt);

    // clear the last gaugemove timeval structure.
    memset(&data->last_gaugemove, 0, sizeof(struct TimeVal));

    updateStatus = TRUE;
  }

  // set the maximum value ahead of the current value
  if(msg->max >= 0)
  {
    set(data->progressGauge, MUIA_Gauge_Max, msg->max);
    updateStatus = TRUE;
  }

  if(msg->percent >= 0)
  {
    // then we update the gauge, but we take also care of not refreshing
    // it too often or otherwise it slows down the whole update process,
    // but make sure to display the final status.
    if(msg->percent == msg->max || TimeHasElapsed(&data->last_gaugemove, 250000) == TRUE)
    {
      set(data->progressGauge, MUIA_Gauge_Current, msg->percent);
      updateStatus = TRUE;
    }
  }

  // lets add the progress Gauge to our splashwindow now
  if(data->progressGaugeActive == FALSE &&
     DoMethod(data->progressGroup, MUIM_Group_InitChange))
  {
    DoMethod(data->progressGroup, OM_ADDMEMBER, data->progressGauge);
    data->progressGaugeActive = TRUE;

    DoMethod(data->progressGroup, MUIM_Group_ExitChange);

    set(data->progressGroup, MUIA_Group_ActivePage, 1);

    DoMethod(G->App, MUIM_Application_InputBuffered);
  }
  else if(updateStatus == TRUE)
    DoMethod(G->App, MUIM_Application_InputBuffered);

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
      End,
    End,
    Child, HVSpace,
  End;

  if(selectGroup)
  {
    // lets add the selectGroup to the window first as MUIA_ShowMe doesn't work
    // correctly
    if(DoMethod(data->windowGroup, MUIM_Group_InitChange))
    {
      DoMethod(data->windowGroup, OM_ADDMEMBER, selectGroup);
      DoMethod(data->windowGroup, MUIM_Group_ExitChange);
    }

    if(DoMethod(userGroup, MUIM_Group_InitChange))
    {
      Object *button0 = NULL;
      Object *group = ColGroup(2), End;
      int i;
      BOOL wasIconified;
      BOOL wasOpen;

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

      set(data->statusGauge, MUIA_Gauge_InfoText, tr(MSG_US_WaitLogin));

      // make sure the window is open and not iconified
      wasOpen = xget(obj, MUIA_Window_Open);
      wasIconified = xget(G->App, MUIA_Application_Iconified);

      if(wasOpen == FALSE)
        set(obj, MUIA_Window_Open, TRUE);

      if(wasIconified)
        set(G->App, MUIA_Application_Iconified, FALSE);

      // we add the esc key to the input event of the requester and if we receive it we close the requester by safely
      // exiting with the last button
      DoMethod(obj, MUIM_Notify, MUIA_Window_CloseRequest, TRUE, MUIV_Notify_Application, 2, MUIM_Application_ReturnID, ID_LOGIN+G->Users.Num);

      // make sure the window is at the front
      DoMethod(obj, MUIM_Window_ToFront);

      // make the first button the active object in the window
      xset(obj, MUIA_Window_ActiveObject,  button0,
                MUIA_Window_DefaultObject, button0);

      // lets collect the waiting returnIDs now
      COLLECT_RETURNIDS;

      do
      {
        static ULONG signals=0;
        LONG ret = DoMethod(G->App, MUIM_Application_NewInput, &signals)-ID_LOGIN;

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
      while(1);

      // now lets reissue the collected returnIDs again
      REISSUE_RETURNIDS;

      // kill the notifies afterwards again
      DoMethod(obj, MUIM_KillNotify, MUIA_Window_CloseRequest);
      for(i=0; i < G->Users.Num; i++)
        DoMethod(obj, MUIM_KillNotify, MUIA_Pressed);

      // make sure no object of the window is active and default afterwards
      xset(obj, MUIA_Window_ActiveObject,  NULL,
                MUIA_Window_DefaultObject, NULL);

      // lets iconify/close the window if it had this state previously
      if(wasOpen == FALSE)
        set(obj, MUIA_Window_Open, FALSE);

      if(wasIconified == TRUE)
        set(G->App, MUIA_Application_Iconified, TRUE);

      if(DoMethod(userGroup, MUIM_Group_InitChange))
      {
        // remove & dispose the group object
        DoMethod(userGroup, OM_REMMEMBER, group);
        MUI_DisposeObject(group);

        DoMethod(userGroup, MUIM_Group_ExitChange);
      }
    }

    if(DoMethod(data->windowGroup, MUIM_Group_InitChange))
    {
      DoMethod(data->windowGroup, OM_REMMEMBER, selectGroup);
      DoMethod(data->windowGroup, MUIM_Group_ExitChange);
    }
  }

  DoMethod(G->App, MUIM_Application_InputBuffered);

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
    BOOL wasIconified;
    BOOL wasOpen;
    LONG ret;

    // lets add the pwGroup to the window first as MUIA_ShowMe doesn't work
    // correctly
    if(DoMethod(data->windowGroup, MUIM_Group_InitChange))
    {
      DoMethod(data->windowGroup, OM_ADDMEMBER, pwGroup);
      DoMethod(data->windowGroup, MUIM_Group_ExitChange);
    }

    set(data->statusGauge, MUIA_Gauge_InfoText, tr(MSG_US_WaitLogin));

    // place the returnID notifies
    DoMethod(bt_okay,   MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 2, MUIM_Application_ReturnID, ID_LOGIN+1);
    DoMethod(bt_cancel, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 2, MUIM_Application_ReturnID, ID_LOGIN+2);
    DoMethod(pwString,  MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime, MUIV_Notify_Application, 2, MUIM_Application_ReturnID, ID_LOGIN+1);
    DoMethod(obj,        MUIM_Notify, MUIA_Window_CloseRequest, TRUE, MUIV_Notify_Application, 2, MUIM_Application_ReturnID, ID_LOGIN+2);

    // make sure the window is open and not iconified
    wasOpen = xget(obj, MUIA_Window_Open);
    wasIconified = xget(G->App, MUIA_Application_Iconified);

    if(wasOpen == FALSE)
      set(obj, MUIA_Window_Open, TRUE);

    if(wasIconified == TRUE)
      set(G->App, MUIA_Application_Iconified, FALSE);

    // make sure the window is at the front
    DoMethod(obj, MUIM_Window_ToFront);

    // lets collect the waiting returnIDs now
    COLLECT_RETURNIDS;

    // make the passwordString the active object
    xset(obj, MUIA_Window_ActiveObject,  pwString,
              MUIA_Window_DefaultObject, pwString);

    do
    {
      static ULONG signals=0;
      ret = DoMethod(G->App, MUIM_Application_NewInput, &signals)-ID_LOGIN;

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
    while(1);

    // now lets reissue the collected returnIDs again
    REISSUE_RETURNIDS;

    // kill the notifies afterwards again
    DoMethod(obj, MUIM_KillNotify, MUIA_Window_CloseRequest);
    DoMethod(obj, MUIM_KillNotify, MUIA_String_Acknowledge);
    DoMethod(obj, MUIM_KillNotify, MUIA_Pressed);
    DoMethod(obj, MUIM_KillNotify, MUIA_Pressed);

    // make sure no object of the window is active and default afterwards
    xset(obj, MUIA_Window_ActiveObject,  NULL,
              MUIA_Window_DefaultObject, NULL);

    // lets iconify/close the window if it had this state previously
    if(wasOpen == FALSE)
      set(obj, MUIA_Window_Open, FALSE);

    if(wasIconified == TRUE)
      set(G->App, MUIA_Application_Iconified, TRUE);

    // remove the passwordRequest again
    if(DoMethod(data->windowGroup, MUIM_Group_InitChange))
    {
      DoMethod(data->windowGroup, OM_REMMEMBER, pwGroup);
      MUI_DisposeObject(pwGroup);

      DoMethod(data->windowGroup, MUIM_Group_ExitChange);
    }
  }

  DoMethod(G->App, MUIM_Application_InputBuffered);

  return result;
}

///
