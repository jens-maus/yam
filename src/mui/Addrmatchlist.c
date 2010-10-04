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

#include "Addrmatchlist_cl.h"

#include "YAM_addressbookEntry.h"
#include "MUIObjects.h"

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
struct CustomABEntry
{
  LONG MatchField;
  LONG RealNameMatchPart;
  char *MatchString;
  struct ABEntry *MatchEntry;
};
*/

/* Hooks */
/// ConstructHook
HOOKPROTONHNO(ConstructFunc, struct CustomABEntry *, struct CustomABEntry *e)
{
  struct CustomABEntry *entry;

  ENTER();

  entry = memdup(e, sizeof(*e));

  RETURN(entry);
  return entry;
}
MakeStaticHook(ConstructHook, ConstructFunc);

///
/// DestructHook
HOOKPROTONHNO(DestructFunc, long, void *entry)
{
  free(entry);

  return 0;
}
MakeStaticHook(DestructHook, DestructFunc);

///
/// DisplayHook
HOOKPROTONH(DisplayFunc, LONG, CONST_STRPTR *array, struct CustomABEntry *e)
{
  static char buf[SIZE_ADDRESS + 4];

  ENTER();

  array[0] = e->MatchEntry->Alias[0]    ? e->MatchEntry->Alias    : "-";
  array[1] = e->MatchEntry->RealName[0] ? e->MatchEntry->RealName : "-";
  array[2] = e->MatchEntry->Address[0]  ? e->MatchEntry->Address  : "-";

  if(e->MatchField == 0)
    snprintf(buf, sizeof(buf), "\033b%." STR(SIZE_NAME) "s", e->MatchString);
  else if(e->MatchField == 1)
    snprintf(buf, sizeof(buf), "\033b%." STR(SIZE_REALNAME) "s", e->MatchString);
  else
    snprintf(buf, sizeof(buf), "\033b%." STR(SIZE_ADDRESS) "s", e->MatchString);

  array[e->MatchField] = buf;

  RETURN(0);
  return 0;
}
MakeStaticHook(DisplayHook, DisplayFunc);

///
/// CompareHook
HOOKPROTONH(CompareFunc, LONG, struct CustomABEntry *e2, struct CustomABEntry *e1)
{
  LONG result;

  ENTER();

  result = e1->MatchField - e2->MatchField;
  if(result == 0)
    result = Stricmp(e1->MatchString, e2->MatchString);

  RETURN(result);
  return result;
}
MakeStaticHook(CompareHook, CompareFunc);
///

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
        MUIA_NListview_NList, list = NListObject,
          InputListFrame,
          MUIA_NList_CompareHook,     &CompareHook,
          MUIA_NList_ConstructHook,   &ConstructHook,
          MUIA_NList_DestructHook,    &DestructHook,
          MUIA_NList_DisplayHook,     &DisplayHook,
          MUIA_NList_Format,          ",,",
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
      E(DBF_GUI, "No MUIA_Addrmatchlist_String supplied");
      CoerceMethod(cl, obj, OM_DISPOSE);
      obj = NULL;
    }
    else
    {
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

  LEAVE();
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
  struct CustomABEntry *entry;
  struct CustomABEntry *result = NULL;

  ENTER();

  D(DBF_GUI, "Match this: '%s'", msg->str);

  set(data->Matchlist, MUIA_NList_Quiet, TRUE);

  DoMethod(data->Matchlist, MUIM_NList_Clear);

  DoMethod(_app(obj), MUIM_YAM_FindEmailMatches, msg->str, data->Matchlist);

  /* is there more entries in the list and if only one, is it longer than what the user already typed... */
  entries = xget(data->Matchlist, MUIA_NList_Entries);
  if(entries > 0 && (DoMethod(data->Matchlist, MUIM_NList_GetEntry, 0, &entry), (entries != 1 || Stricmp(msg->str, entry->MatchString) != 0)))
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
    DoMethod(obj, MUIM_Addrmatchlist_ChangeWindow); // refresh the position
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
    struct CustomABEntry *entry;

    // get the active entry
    DoMethod(data->Matchlist, MUIM_NList_GetEntry, msg->active, &entry);
    if(entry != NULL)
    {
      switch(entry->MatchEntry->Type)
      {
        case AET_USER:
        {
          // for users we prefer their address
          char *res = entry->MatchEntry->Address;

          // signal the string that we need to replace the selected part with
          // some new entry
          if(res != NULL)
            DoMethod(data->String, MUIM_Recipientstring_ReplaceSelected, res);
        }
        break;

        case AET_LIST:
        {
          // for lists we use the list name
          char *res = entry->MatchEntry->RealName;

          // signal the string that we need to replace the selected part with
          // some new entry
          if(res != NULL)
            DoMethod(data->String, MUIM_Recipientstring_ReplaceSelected, res);
        }
        break;

        case AET_GROUP:
        {
          // this should not happen
        }
        break;
      }
    }
  }

  LEAVE();
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
  DoMethod(data->String, MUIM_Recipientstring_Resolve, MUIF_NONE);
  // append a comma and a space to allow easy further input
  DoMethod(data->String, MUIM_BetterString_Insert, ", ", MUIV_BetterString_Insert_EndOfString);
  // close the window
  set(obj, MUIA_Window_Open, FALSE);

  LEAVE();
  return 0;
}

///
