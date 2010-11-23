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
 Description: Window where user inputs search string and options.

***************************************************************************/

#include "Searchwindow_cl.h"

#include <string.h>
#include <proto/muimaster.h>
#include <mui/BetterString_mcc.h>
#include <mui/TextEditor_mcc.h>

#include "YAM.h"

#include "Locale.h"
#include "MUIObjects.h"
#include "Requesters.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *Searchstring;
  Object *Texteditor;
  Object *ParentWindow;
  BOOL CaseSensitive;
  BOOL CloseNotifyAdded;
};
*/

/* EXPORT
#define MUIF_Searchwindow_FromTop                      (1<<0) // search from the beginning of the text
#define MUIF_Searchwindow_BeepOnFailure                (1<<1) // do a DisplayBeep() if the text was not found

#define hasFromTopFlag(v)                              (isFlagSet((v), MUIF_Searchwindow_FromTop))
#define hasBeepOnFailureFlag(v)                        (isFlagSet((v), MUIF_Searchwindow_BeepOnFailure))
*/

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  Object *string;
  Object *case_sensitive;
  Object *search;
  Object *cancel;

  ENTER();

  if((obj = DoSuperNew(cl, obj,

    MUIA_Window_Title, tr(MSG_SEARCHWINDOW_TITLE),
    WindowContents, VGroup,

      Child, string = BetterStringObject,
        MUIA_CycleChain, TRUE,
        StringFrame,
      End,

      Child, HGroup,
        Child, RectangleObject, End,
        Child, MakeCheckGroup(&case_sensitive, tr(MSG_SEARCHWINDOW_TOGGLE_CASESENSITVE)),
        Child, RectangleObject, End,
      End,

      Child, HGroup,
        Child, search = MakeButton(tr(MSG_SEARCHWINDOW_BT_SEARCH)),
        Child, VSpace(0),
        Child, cancel = MakeButton(tr(MSG_SEARCHWINDOW_BT_CANCEL)),
      End,

    End,

    TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;

    DoMethod(G->App, OM_ADDMEMBER, obj);

    data->Searchstring = string;

    DoMethod(string,         MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime, obj, 2, METHOD(Search), MUIF_Searchwindow_FromTop);
    DoMethod(case_sensitive, MUIM_Notify, MUIA_Selected, MUIV_EveryTime, obj, 1, METHOD(ToggleCaseSensitivity));
    DoMethod(search,         MUIM_Notify, MUIA_Pressed, FALSE, obj, 2, METHOD(Search), MUIF_Searchwindow_FromTop);
    DoMethod(cancel,         MUIM_Notify, MUIA_Pressed, FALSE, obj, 1, METHOD(Close));
    DoMethod(obj,            MUIM_Notify, MUIA_Window_CloseRequest, TRUE, obj, 1, METHOD(Close));
  }

  RETURN((IPTR)obj);
  return (IPTR)obj;
}

///

/* Private Functions */

/* Public Methods */
/// DECLARE(Open)
DECLARE(Open) // Object *texteditor
{
  GETDATA;

  ENTER();

  if(data->ParentWindow != NULL)
    DoMethod(obj, MUIM_Searchwindow_Close);

  data->Texteditor = msg->texteditor;
  data->ParentWindow = _win(msg->texteditor);

  if(data->ParentWindow != NULL)
  {
    DoMethod(data->ParentWindow, MUIM_Notify, MUIA_Window_Open, FALSE, obj, 1, MUIM_Searchwindow_Close);
    data->CloseNotifyAdded = TRUE;
  }

  xset(data->Searchstring,  MUIA_String_BufferPos, 0,
                            MUIA_BetterString_SelectSize, strlen((STRPTR)xget(data->Searchstring, MUIA_String_Contents)));

  xset(obj, MUIA_Window_Activate,     TRUE,
            MUIA_Window_ActiveObject, data->Searchstring,
            MUIA_Window_RefWindow,    data->ParentWindow,
            MUIA_Window_Open,         TRUE);

  LEAVE();
  return 0;
}

///
/// DECLARE(Close)
DECLARE(Close)
{
  GETDATA;

  ENTER();

  set(obj, MUIA_Window_Open, FALSE);

  if(data->ParentWindow != NULL && data->CloseNotifyAdded == TRUE)
  {
    DoMethod(data->ParentWindow, MUIM_KillNotifyObj, MUIA_Window_Open, obj);
    data->CloseNotifyAdded = FALSE;
  }

  LEAVE();
  return 0;
}

///
/// DECLARE(Search)
DECLARE(Search) // ULONG flags
{
  GETDATA;
  STRPTR string;

  ENTER();

  DoMethod(obj, MUIM_Searchwindow_Close);

  if((string = (STRPTR)xget(data->Searchstring, MUIA_String_Contents), string) != NULL && string[0] != '\0' && data->Texteditor != NULL)
  {
    ULONG flags = 0;

    if(hasFromTopFlag(msg->flags))
      SET_FLAG(flags, MUIF_TextEditor_Search_FromTop);

    if(data->CaseSensitive)
      SET_FLAG(flags, MUIF_TextEditor_Search_CaseSensitive);

    // perform the text search and return an error as well as a displaybeep
    // if the search string wasn't found.
    if(!DoMethod(data->Texteditor, MUIM_TextEditor_Search, string, flags))
    {
      // put up a requester if we are searching from
      // top
      if(hasFromTopFlag(msg->flags))
      {
        MUI_Request(_app(obj), data->ParentWindow, 0L, tr(MSG_SEARCHNOTFOUND_TITLE),
                                                       tr(MSG_OkayReq),
                                                       tr(MSG_SEARCHNOTFOUND_MSG), string);
      }
      else if(hasBeepOnFailureFlag(msg->flags))
      {
        // beep the display
        DisplayBeep(_screen(obj));
      }
    }
  }

  LEAVE();
  return 0;
}

///
/// DECLARE(Next)
DECLARE(Next)
{
  return DoMethod(obj, MUIM_Searchwindow_Search, MUIF_Searchwindow_BeepOnFailure);
}

///
/// DECLARE(ToggleCaseSensitivity)
DECLARE(ToggleCaseSensitivity)
{
  GETDATA;

  ENTER();

  data->CaseSensitive = !data->CaseSensitive;

  LEAVE();
  return 0;
}

///
