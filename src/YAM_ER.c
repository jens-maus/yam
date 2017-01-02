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

***************************************************************************/

#include <stdlib.h>
#include <string.h>

#include <clib/alib_protos.h>
#include <libraries/iffparse.h>
#include <mui/NListview_mcc.h>
#include <mui/NFloattext_mcc.h>
#include <proto/intuition.h>
#include <proto/locale.h>
#include <proto/muimaster.h>

#include "extrasrc.h"

#include "SDI_hook.h"
#include "SDI_stdarg.h"

#include "YAM.h"
#include "YAM_error.h"
#include "YAM_main.h"
#include "YAM_utilities.h"

#include "Config.h"
#include "Locale.h"
#include "MethodStack.h"
#include "MUIObjects.h"
#include "Threads.h"

#include "mui/ClassesExtra.h"
#include "mui/YAMApplication.h"

#include "Debug.h"

/* local protos */
static struct ER_ClassData *ER_New(void);

#define SLIDER_FORMAT "\033c%s %%ld/%d"

/***************************************************************************
 Module: Error window
***************************************************************************/

/// ShowMessage
//
static void ShowMessage(BOOL isError, const char *message, va_list args)
{
  ENTER();

  if(IsMainThread() == TRUE && G->ER == NULL)
  {
    if((G->ER = ER_New()) == NULL)
    {
      LEAVE();
      return;
    }
  }

  if(message != NULL)
  {
    if(IsMainThread() == TRUE)
    {
      char buf[SIZE_LARGE];
      char datstr[64];
      char *final;

      // get actual date as a string
      DateStamp2String(datstr, sizeof(datstr), NULL, (C->DSListFormat == DSS_DATEBEAT || C->DSListFormat == DSS_RELDATEBEAT) ? DSS_DATEBEAT : DSS_DATETIME, TZC_NONE);

      vsnprintf(buf, sizeof(buf), message, args);

      // allocate an own buffer for our error string and append the datestring
      if(asprintf(&final, "%s\n\n(%s)", buf, datstr) != -1)
      {
        if(G->ER_NumErr == MAXERR)
        {
          // the number of rembembered error messages exceeds the maximum number
          // free the oldest message
          free(G->ER_Message[0]);
          // shift all other messages down by one
          memmove(&G->ER_Message[0], &G->ER_Message[1], sizeof(G->ER_Message[0])*(MAXERR-1));
          G->ER_NumErr = MAXERR-1;
        }

        // place the new message at the end
        G->ER_Message[G->ER_NumErr] = final;

        // count one more error message
        G->ER_NumErr++;
        D(DBF_ALWAYS, "added %s message #%ld '%s'", isError ? "error" : "warning", G->ER_NumErr, SafeStr(G->ER_Message[G->ER_NumErr-1]));
      }
      else
      {
        E(DBF_ALWAYS, "no free memory for final error message '%s'", buf);
      }
    }
    else
    {
      char *msg;

      if(vasprintf(&msg, message, args) != -1)
      {
        if(isError == TRUE)
        {
          D(DBF_ALWAYS, "pushing error '%s'", msg);
          PushMethodOnStack(G->App, 2, MUIM_YAMApplication_ShowError, msg);
        }
        else
        {
          D(DBF_ALWAYS, "pushing warning '%s'", msg);
          PushMethodOnStack(G->App, 2, MUIM_YAMApplication_ShowWarning, msg);
        }
      }
      else
      {
        E(DBF_ALWAYS, "no free memory to push %s message '%s'", isError ? "error" : "warning", message);
      }
    }
  }
  else if(IsMainThread() == FALSE)
  {
    E(DBF_ALWAYS, "NULL %s message from thread '%s'", isError ? "error" : "warning", CurrentThreadName());
  }

  if(IsMainThread() == TRUE)
  {
    // update the numeric button to contain the new number of pending errors
    snprintf(G->ER->SliderLabel, sizeof(G->ER->SliderLabel), SLIDER_FORMAT, tr(MSG_ErrorReq), G->ER_NumErr);

    // set the numeric button's new limits, but don't trigger any notifications
    xset(G->ER->GUI.NB_ERROR, MUIA_NoNotify,       TRUE,
                              MUIA_Numeric_Min,    1,
                              MUIA_Numeric_Max,    G->ER_NumErr,
                              MUIA_Numeric_Value,  G->ER_NumErr,
                              MUIA_Numeric_Format, G->ER->SliderLabel);
    // show the current message
    D(DBF_ALWAYS, "showing %s message #%ld '%s'", isError ? "error" : "warning", G->ER_NumErr, SafeStr(G->ER_Message[G->ER_NumErr-1]));
    set(G->ER->GUI.LV_ERROR, MUIA_NFloattext_Text, G->ER_Message[G->ER_NumErr-1]);

    // enable the menu item to open this window as there pending messages now
    if(G->MA != NULL)
      set(G->MA->GUI.MI_ERRORS, MUIA_Menuitem_Enabled, TRUE);

    // open the window for errors only, warnings are recorded silently
    if(isError == TRUE && SafeOpenWindow(G->ER->GUI.WI) == FALSE)
    {
      DisposeModule(&G->ER);
    }
  }

  LEAVE();
}

///
/// ER_NewError
// Adds a new error message and displays it
void ER_NewError(const char *message, ...)
{
  va_list args;

  ENTER();

  va_start(args, message);
  ShowMessage(TRUE, message, args);
  va_end(args);

  LEAVE();
}

///
/// ER_NewWarning
// Adds a new warning message
void ER_NewWarning(const char *message, ...)
{
  va_list args;

  ENTER();

  va_start(args, message);
  ShowMessage(FALSE, message, args);
  va_end(args);

  LEAVE();
}

///
/// ER_SelectFunc
// Displays an earlier error message
HOOKPROTONHNO(ER_SelectFunc, void, int *arg)
{
  int value = arg[0];

  ENTER();

  SHOWVALUE(DBF_ALWAYS, value);

  if(value >= 1 && value <= G->ER_NumErr)
  {
    D(DBF_ALWAYS, "showing error message #%ld '%s'", value, SafeStr(G->ER_Message[value-1]));
    set(G->ER->GUI.BT_NEXT, MUIA_Disabled, value == G->ER_NumErr);
    set(G->ER->GUI.BT_PREV, MUIA_Disabled, value == 1);
    set(G->ER->GUI.LV_ERROR, MUIA_NFloattext_Text, G->ER_Message[value-1]);
  }

  LEAVE();
}
MakeStaticHook(ER_SelectHook, ER_SelectFunc);

///
/// ER_CloseFunc
// Closes error window
HOOKPROTONHNO(ER_CloseFunc, void, int *arg)
{
  ENTER();

  set(G->ER->GUI.WI, MUIA_Window_Open, FALSE);

  if(arg[0] == TRUE)
  {
    int i;

    // clear any currently displayed message before freeing its memory
    set(G->ER->GUI.LV_ERROR, MUIA_NFloattext_Text, NULL);
    // now free the memory of all collected messages
    for(i = 0; i < G->ER_NumErr; i++)
    {
      free(G->ER_Message[i]);
      G->ER_Message[i] = NULL;
    }
    G->ER_NumErr = 0;

    // disable the menu item to open this window as there are no messages pending anymore
    if(G->MA != NULL)
      set(G->MA->GUI.MI_ERRORS, MUIA_Menuitem_Enabled, FALSE);
  }

  DisposeModulePush(&G->ER);

  LEAVE();
}
MakeStaticHook(ER_CloseHook, ER_CloseFunc);

///

/// ER_New
// Creates error window
static struct ER_ClassData *ER_New(void)
{
  struct ER_ClassData *data;

  ENTER();

  if((data = calloc(1, sizeof(struct ER_ClassData))) != NULL)
  {
    Object *bt_close;
    Object *bt_clear;

    snprintf(data->SliderLabel, sizeof(data->SliderLabel), SLIDER_FORMAT, tr(MSG_ErrorReq), 0);

    data->GUI.WI = WindowObject,
       MUIA_Window_Title, tr(MSG_ER_ErrorMessages),
       MUIA_Window_ScreenTitle, CreateScreenTitle(data->ScreenTitle, sizeof(data->ScreenTitle), tr(MSG_ER_ErrorMessages)),
       MUIA_Window_ID, MAKE_ID('E','R','R','O'),
       WindowContents, VGroup,
          Child, HGroup,
             Child, data->GUI.BT_PREV = MakeButton(tr(MSG_ER_PrevError)),
             Child, data->GUI.NB_ERROR = NumericbuttonObject,
                MUIA_Numeric_Min,    1,
                MUIA_Numeric_Max,    1,
                MUIA_Numeric_Value,  1,
                MUIA_Numeric_Format, data->SliderLabel,
                MUIA_CycleChain,     TRUE,
             End,
             Child, data->GUI.BT_NEXT = MakeButton(tr(MSG_ER_NextError)),
          End,
          Child, NListviewObject,
             MUIA_Listview_Input,  FALSE,
             MUIA_CycleChain,      TRUE,
             MUIA_NListview_NList, data->GUI.LV_ERROR = NFloattextObject,
                ReadListFrame,
             End,
          End,
          Child, ColGroup(2),
             Child, bt_clear = MakeButton(tr(MSG_ER_Clear)),
             Child, bt_close = MakeButton(tr(MSG_ER_Close)),
          End,
       End,
    End;

    if(data->GUI.WI != NULL)
    {
      DoMethod(G->App, OM_ADDMEMBER, data->GUI.WI);
      DoMethod(data->GUI.BT_PREV ,MUIM_Notify,MUIA_Pressed            ,FALSE         ,data->GUI.NB_ERROR     ,2,MUIM_Numeric_Decrease,1);
      DoMethod(data->GUI.BT_NEXT ,MUIM_Notify,MUIA_Pressed            ,FALSE         ,data->GUI.NB_ERROR     ,2,MUIM_Numeric_Increase,1);
      DoMethod(data->GUI.NB_ERROR,MUIM_Notify,MUIA_Numeric_Value      ,MUIV_EveryTime,MUIV_Notify_Application,3,MUIM_CallHook,&ER_SelectHook,MUIV_TriggerValue);
      DoMethod(bt_clear          ,MUIM_Notify,MUIA_Pressed            ,FALSE         ,MUIV_Notify_Application,3,MUIM_CallHook,&ER_CloseHook,TRUE);
      DoMethod(bt_close          ,MUIM_Notify,MUIA_Pressed            ,FALSE         ,MUIV_Notify_Application,3,MUIM_CallHook,&ER_CloseHook,FALSE);
      DoMethod(data->GUI.WI      ,MUIM_Notify,MUIA_Window_CloseRequest,TRUE          ,MUIV_Notify_Application,3,MUIM_CallHook,&ER_CloseHook,FALSE);
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
