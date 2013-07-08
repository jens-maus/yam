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
 Description: Mainwindow class carrying all main GUI elements of YAM

***************************************************************************/

#include "MainWindow_cl.h"

#include <proto/muimaster.h>

#include "YAM.h"
#include "YAM_config.h"
#include "YAM_error.h"

#include "MUIObjects.h"

#include "mui/Aboutwindow.h"
#include "mui/MainMailListGroup.h"
#include "mui/QuickSearchBar.h"
#include "mui/ReadMailGroup.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *aboutWindow;
};
*/

/* Overloaded Methods */
/// OVERLOAD(OM_SET)
OVERLOAD(OM_SET)
{
  IPTR result = 0;
  struct TagItem *tags = inittags(msg), *tag;

  ENTER();

  while((tag = NextTagItem((APTR)&tags)) != NULL)
  {
    switch(tag->ti_Tag)
    {
      case MUIA_Window_DefaultObject:
      {
        // if the user clicks somewhere where the default
        // object would be set to NULL we make sure we set
        // it back to the default object of the readmail group
        if((Object *)tag->ti_Data == NULL)
          tag->ti_Data = xget(G->MA->GUI.PG_MAILLIST, MUIA_MainMailListGroup_ActiveListviewObject);
      }
      break;
    }
  }

  result = DoSuperMethodA(cl, obj, msg);

  RETURN(result);
  return result;
}

///
/// OVERLOAD(MUIM_Window_Snapshot)
OVERLOAD(MUIM_Window_Snapshot)
{
  struct MUIP_Window_Snapshot *snap = (struct MUIP_Window_Snapshot *)msg;
  IPTR result;

  ENTER();

  // remember the weights for snapshot operations, but not for unsnapshot operations
  if(snap->flags != 0)
  {
    // get the weights according to their GUI elements
    G->Weights[0] = xget(G->MA->GUI.LV_FOLDERS,  MUIA_HorizWeight);
    G->Weights[1] = xget(G->MA->GUI.GR_MAILVIEW, MUIA_HorizWeight);

    // if the embedded read pane objects are currently active we save their weight values
    if(C->EmbeddedReadPane == TRUE)
    {
      G->Weights[6] = xget(G->MA->GUI.GR_MAILLIST, MUIA_VertWeight);
      G->Weights[7] = xget(G->MA->GUI.MN_EMBEDDEDREADPANE, MUIA_VertWeight);
      G->Weights[8] = xget(G->MA->GUI.MN_EMBEDDEDREADPANE, MUIA_ReadMailGroup_HGVertWeight);
      G->Weights[9] = xget(G->MA->GUI.MN_EMBEDDEDREADPANE, MUIA_ReadMailGroup_TGVertWeight);
    }

    // make sure the layout is saved
    SaveLayout(TRUE);
  }

  result = DoSuperMethodA(cl, obj, msg);

  RETURN(result);
  return result;
}

///

/* Private Functions */

/* Public Methods */
/// DECLARE(DisposeSubWindow)
// method that is used by modeless subwindows to get disposed upon
// their close
DECLARE(DisposeSubWindow) // Object *win
{
  ENTER();

  D(DBF_GUI, "Dispose subwindow: %08lx", msg->win);

  set(msg->win, MUIA_Window_Open, FALSE);
  DoMethod(G->App, OM_REMMEMBER, msg->win);
  MUI_DisposeObject(msg->win);

  LEAVE();
  return 0;
}

///
/// DECLARE(ShowAbout)
// show the about window
DECLARE(ShowAbout)
{
  GETDATA;

  ENTER();

  // create the about window object and open it
  if(data->aboutWindow == NULL)
  {
    data->aboutWindow = AboutwindowObject, End;

    if(data->aboutWindow != NULL)
      DoMethod(data->aboutWindow, MUIM_Notify, MUIA_Window_Open, FALSE, MUIV_Notify_Application, 4, MUIM_Application_PushMethod, G->App, 1, MUIM_MainWindow_CloseAbout);
  }

  SafeOpenWindow(data->aboutWindow);

  LEAVE();
  return 0;
}

///
/// DECLARE(CloseAbout)
// close the about window
DECLARE(CloseAbout)
{
  GETDATA;

  ENTER();

  // close the about window object
  if(data->aboutWindow != NULL)
  {
    DoMethod(G->App, OM_REMMEMBER, data->aboutWindow);
    MUI_DisposeObject(data->aboutWindow);
    data->aboutWindow = NULL;
  }

  LEAVE();
  return 0;
}

///
/// DECLARE(ShowErrors)
// show the error window
DECLARE(ShowErrors)
{
  ENTER();

  ER_NewError(NULL);

  LEAVE();
  return 0;
}

///
/// DECLARE(Relayout)
// relayout the main window after a configuration change
DECLARE(Relayout)
{
  ENTER();

  if(DoMethod(G->MA->GUI.GR_MAIN, MUIM_Group_InitChange))
  {
    BOOL showbar = TRUE;

    switch(C->InfoBarPos)
    {
      case IB_POS_TOP:
      {
        DoMethod(G->MA->GUI.GR_MAIN, MUIM_Group_Sort, G->MA->GUI.IB_INFOBAR,
                                                      G->MA->GUI.GR_TOP,
                                                      G->MA->GUI.GR_HIDDEN,
                                                      G->MA->GUI.GR_BOTTOM,
                                                      NULL);
      }
      break;

      case IB_POS_CENTER:
      {
        DoMethod(G->MA->GUI.GR_MAIN, MUIM_Group_Sort, G->MA->GUI.GR_TOP,
                                                      G->MA->GUI.GR_HIDDEN,
                                                      G->MA->GUI.IB_INFOBAR,
                                                      G->MA->GUI.GR_BOTTOM,
                                                      NULL);
      }
      break;

      case IB_POS_BOTTOM:
      {
        DoMethod(G->MA->GUI.GR_MAIN, MUIM_Group_Sort, G->MA->GUI.GR_TOP,
                                                      G->MA->GUI.GR_HIDDEN,
                                                      G->MA->GUI.GR_BOTTOM,
                                                      G->MA->GUI.IB_INFOBAR,
                                                      NULL);
      }
      break;

      default:
      {
        showbar = FALSE;
      }
    }

    // Here we can do a MUIA_ShowMe, TRUE because SortWindow is encapsulated
    // in a InitChange/ExitChange..
    set(G->MA->GUI.IB_INFOBAR, MUIA_ShowMe, showbar);

    DoMethod(G->MA->GUI.GR_MAIN, MUIM_Group_ExitChange);
  }

  if(DoMethod(G->MA->GUI.GR_MAILLIST, MUIM_Group_InitChange))
  {
    BOOL showbar = TRUE;

    switch(C->QuickSearchBarPos)
    {
      case QSB_POS_TOP:
      {
        DoMethod(G->MA->GUI.GR_MAILLIST, MUIM_Group_Sort, G->MA->GUI.GR_QUICKSEARCHBAR,
                                                          G->MA->GUI.PG_MAILLIST,
                                                          NULL);
	  }
	  break;

      case QSB_POS_BOTTOM:
      {
        DoMethod(G->MA->GUI.GR_MAILLIST, MUIM_Group_Sort, G->MA->GUI.PG_MAILLIST,
                                                          G->MA->GUI.GR_QUICKSEARCHBAR,
                                                          NULL);
	  }
	  break;

      default:
      {
        showbar = FALSE;
	  }
	  break;
    }

    // if the quickSearchBar is enabled by the user we
    // make sure we show it
    DoMethod(G->MA->GUI.GR_QUICKSEARCHBAR, MUIM_QuickSearchBar_Clear);
    set(G->MA->GUI.GR_QUICKSEARCHBAR, MUIA_ShowMe, showbar);

    DoMethod(G->MA->GUI.GR_MAILLIST, MUIM_Group_ExitChange);
  }

  LEAVE();
  return 0;
}

///
