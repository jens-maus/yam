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
 YAM OpenSource project    : http://sourceforge.net/projects/yamos/

 $Id$

 Superclass:  MUIC_Window
 Description: Popup a list of addresses which match a given substring

***************************************************************************/

#include "AddressMatchPopupWindow_cl.h"

#include <stdlib.h>
#include <proto/muimaster.h>
#include <mui/BetterString_mcc.h>
#include <mui/NList_mcc.h>
#include <mui/NListview_mcc.h>

#include "newmouse.h"

#include "YAM.h"

#include "MUIObjects.h"

#include "mui/AddressMatchList.h"
#include "mui/RecipientString.h"
#include "mui/YAMApplication.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  struct MUI_EventHandlerNode ehnode;
  Object *Matchlist;
  Object *String;
  BOOL EventHandlerAdded;
  BOOL Open;
};
*/

/* EXPORT
struct MatchedABookEntry
{
  LONG MatchField;
  LONG RealNameMatchPart;
  char *MatchString;
  struct ABookNode *MatchEntry;
};
*/

/* INCLUDE
#include "AddressBook.h"
*/

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  Object *listview;
  Object *list;

  ENTER();

  if((obj = DoSuperNew(cl, obj,
    MUIA_Window_Activate,     FALSE,
    MUIA_Window_Borderless,   TRUE,
    MUIA_Window_CloseGadget,  FALSE,
    MUIA_Window_DepthGadget,  FALSE,
    MUIA_Window_DragBar,      FALSE,
    MUIA_Window_SizeGadget,   FALSE,
    MUIA_Window_IsSubWindow,  TRUE,
    WindowContents, GroupObject,
      InnerSpacing(0, 0),
      MUIA_FixHeightTxt, "\n\n\n\n\n\n\n\n",
      Child, listview = NListviewObject,
        MUIA_NListview_Horiz_ScrollBar, MUIV_NListview_HSB_None,
        MUIA_NListview_Vert_ScrollBar,  MUIV_NListview_VSB_FullAuto,
        MUIA_NListview_NList, list = AddressMatchListObject,
        End,
      End,
    End,
    TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;

    struct TagItem *tags = inittags(msg), *tag;
    while((tag = NextTagItem((APTR)&tags)) != NULL)
    {
      switch(tag->ti_Tag)
      {
        case ATTR(String) : data->String = (Object *)tag->ti_Data ; break;
      }
    }

    data->Matchlist = list;
    data->EventHandlerAdded = FALSE;

    if(data->String == NULL)
    {
      E(DBF_GUI, "No MUIA_AddressmatchPopup_String supplied");
      CoerceMethod(cl, obj, OM_DISPOSE);
      obj = NULL;
    }
    else
    {
      DoMethod(G->App, OM_ADDMEMBER, obj);

      // we need to catch the RAWKEY events and forward them directly
      // to our string object
      data->ehnode.ehn_Priority = 1;
      data->ehnode.ehn_Flags    = 0;
      data->ehnode.ehn_Object   = data->String;
      data->ehnode.ehn_Class    = 0;
      data->ehnode.ehn_Events   = IDCMP_RAWKEY;

      DoMethod(obj, MUIM_Window_AddEventHandler, &data->ehnode);
      data->EventHandlerAdded = TRUE;

      // set the doubleclick notify to signal the string to resolve a entry
      DoMethod(listview, MUIM_Notify, MUIA_Listview_DoubleClick, TRUE, obj, 1, METHOD(AddRecipient));
      DoMethod(list, MUIM_Notify, MUIA_NList_Active, MUIV_EveryTime, obj, 2, METHOD(ActiveChange), MUIV_TriggerValue);
    }
  }

  RETURN((IPTR)obj);
  return (IPTR)obj;
}

///
/// OVERLOAD(OM_DISPOSE)
OVERLOAD(OM_DISPOSE)
{
  GETDATA;

  // remove the event handler only if it was added before,
  // because this might have not been happened yet.
  if(data->EventHandlerAdded)
    DoMethod(obj, MUIM_Window_RemEventHandler, &data->ehnode);

  return DoSuperMethodA(cl, obj, msg);
}

///
/// OVERLOAD(OM_SET)
OVERLOAD(OM_SET)
{
  GETDATA;
  struct TagItem *tags, *tag;
  IPTR result;

  ENTER();

  tags = inittags(msg);
  while((tag = NextTagItem((APTR)&tags)) != NULL)
  {
    switch(tag->ti_Tag)
    {
      case MUIA_Window_Open:
      {
        if(data->Open != (BOOL)tag->ti_Data && _win(data->String))
          set(_win(data->String), MUIA_Window_DisableKeys, (data->Open = tag->ti_Data) ? 1 << MUIKEY_WINDOW_CLOSE : 0);
      }
      break;
    }
  }

  result = DoSuperMethodA(cl, obj, msg);

  RETURN(result);
  return result;
}

///

/* Private Functions */

/* Class Methods */
/// DECLARE(ChangeWindow)
DECLARE(ChangeWindow)
{
  GETDATA;
  struct Window *match_win;
  struct Window *write_win;
  ULONG left;
  ULONG top;

  ENTER();

  match_win = (struct Window *)xget(obj, MUIA_Window_Window);
  write_win = (struct Window *)xget(_win(data->String), MUIA_Window_Window);
  left = write_win->LeftEdge + _left(data->String);
  top = write_win->TopEdge + _bottom(data->String) + 1;

  // only when the window is close a set() is valid to change
  // the position of a MUI window. Otherwise we have to use ChangeWindowBox()
  if(match_win != NULL && xget(obj, MUIA_Window_Open))
  {
    // change the window position/sizes
    ChangeWindowBox(match_win, left, top, _width(data->String), match_win->Height);

    // also make sure the window is always in front of the write window
    MoveWindowInFrontOf(match_win, write_win);
  }
  else
  {
    // if there is currently no window open we can use set()
    // to change the position/sizes
    xset(obj, MUIA_Window_LeftEdge,   left,
              MUIA_Window_TopEdge,    top,
              MUIA_Window_Width,      _width(data->String));
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(Event)
DECLARE(Event) // struct IntuiMessage *imsg
{
  GETDATA;
  IPTR result = FALSE;

  ENTER();

  if(xget(obj, MUIA_Window_Open) == TRUE)
  {
    struct IntuiMessage *imsg = msg->imsg;
    LONG position = xget(data->Matchlist, MUIA_NList_Active);
    LONG direction;

    #if defined(__amigaos4__)
    if(imsg->Class == IDCMP_EXTENDEDMOUSE)
    {
      struct IntuiWheelData *iwd = (struct IntuiWheelData *)imsg->IAddress;

      if(iwd->WheelY < 0 || iwd->WheelX < 0)
        direction = MUIV_NList_Active_Up;
      else
        direction = MUIV_NList_Active_Down;
    }
    else
    #endif
      direction = (imsg->Code == IECODE_UP || imsg->Code == NM_WHEEL_UP || imsg->Code == NM_WHEEL_LEFT) ? MUIV_NList_Active_Up : MUIV_NList_Active_Down;

    // to enable a circular selection model we have to make some checks.
    if(direction == MUIV_NList_Active_Up)
    {
      if(position == MUIV_NList_Active_Off || position == 0)
        direction = MUIV_NList_Active_Bottom;
    }
    else
    {
      if(position == MUIV_NList_Active_Off || position == (LONG)xget(data->Matchlist, MUIA_NList_Entries)-1)
        direction = MUIV_NList_Active_Top;
    }

    set(data->Matchlist, MUIA_NList_Active, direction);

    result = TRUE;
  }

  RETURN(result);
  return result;
}

///
/// DECLARE(Open)
DECLARE(Open) // char *str
{
  GETDATA;
  LONG entries;
  struct MatchedABookEntry *entry;
  struct MatchedABookEntry *result = NULL;

  ENTER();

  D(DBF_GUI, "Match this: '%s'", msg->str);

  set(data->Matchlist, MUIA_NList_Quiet, TRUE);

  DoMethod(data->Matchlist, MUIM_NList_Clear);

  DoMethod(_app(obj), MUIM_YAMApplication_FindEmailMatches, msg->str, data->Matchlist);

  // is there more entries in the list and if only one, is it longer than what the user already typed...
  entries = xget(data->Matchlist, MUIA_NList_Entries);
  if(entries > 0 && DoMethod(data->Matchlist, MUIM_NList_GetEntry, 0, &entry) != 0 && entry != NULL)
  {
    // now we return the complete first entry of the list of matches instead
    // of just the matching string to allow the recipient string to derive the
    // complete name and address.
    result = entry;

    // make no entry active yet
    nnset(data->Matchlist, MUIA_NList_Active, MUIV_NList_Active_Off);
  }

  // should we open the popup list (if not already shown)
  if(result != NULL && xget(obj, MUIA_Window_Open) == FALSE)
  {
    // refresh the position
    DoMethod(obj, METHOD(ChangeWindow));
    set(obj, MUIA_Window_Open, TRUE);
  }
  else if(result == NULL)
    set(obj, MUIA_Window_Open, FALSE);

  set(data->Matchlist, MUIA_NList_Quiet, FALSE);

  RETURN(result);
  return (IPTR)result;
}

///
/// DECLARE(ActiveChange)
DECLARE(ActiveChange) // LONG active
{
  GETDATA;

  ENTER();

  // activate the window containing the string object again, as this method might
  // have been triggered by a double click, but we must keep the other window active
  // so that the string object does not loose focus.
  ActivateWindow((struct Window *)xget(_win(data->String), MUIA_Window_Window));

  if(msg->active >= 0)
  {
    struct MatchedABookEntry *entry;

    // get the active entry
    DoMethod(data->Matchlist, MUIM_NList_GetEntry, msg->active, &entry);
    if(entry != NULL)
    {
      switch(entry->MatchEntry->type)
      {
        case ABNT_USER:
        {
          // for users we prefer their address
          // signal the string that we need to replace the selected part with
          // some new entry
          DoMethod(data->String, MUIM_RecipientString_ReplaceSelected, entry->MatchEntry->Address);
        }
        break;

        case ABNT_LIST:
        {
          // for lists we use the list name
          // signal the string that we need to replace the selected part with
          // some new entry
          DoMethod(data->String, MUIM_RecipientString_ReplaceSelected, entry->MatchEntry->RealName);
        }
        break;

        case ABNT_GROUP:
        {
          // this should not happen
        }
        break;
      }
    }
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(AddRecipient)
DECLARE(AddRecipient)
{
  GETDATA;

  ENTER();

  // adding the recipient is done by simply resolving the address, as the
  // recipient itself has been added already by selecting one entry from
  // our list
  DoMethod(data->String, MUIM_RecipientString_Resolve, MUIF_NONE);
  // append a comma and a space to allow easy further input
  DoMethod(data->String, MUIM_BetterString_Insert, ", ", MUIV_BetterString_Insert_EndOfString);
  // close the window
  set(obj, MUIA_Window_Open, FALSE);

  RETURN(0);
  return 0;
}

///
