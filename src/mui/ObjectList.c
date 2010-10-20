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
 YAM OpenSource project     : http://sourceforge.net/projects/yamos/

 $Id$

 Superclass:  MUIC_Scrollgroup
 Description: Abstract base class to display ObjectListitem objects in
              a scrollable group

***************************************************************************/

#include "ObjectList_cl.h"

#include "MUIObjects.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *virtgroup;
  Object *spacer;
  Object *dummy;
  ULONG itemCount;
  BOOL disposeRemovedItems;
};
*/

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  Object *virtgroup = NULL;
  Object *spacer = NULL;
  Object *dummy = NULL;

  ENTER();

  if((obj = DoSuperNew(cl, obj,

    MUIA_Scrollgroup_FreeHoriz, FALSE,
    MUIA_Scrollgroup_AutoBars, TRUE,
    MUIA_Scrollgroup_Contents, virtgroup = VGroupV,
      Child, dummy = VSpace(1),
      Child, spacer = HVSpace,
    End,

    TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;

    data->virtgroup = virtgroup;
    data->spacer = spacer;
    data->dummy = dummy;
    data->itemCount = 0;
    data->disposeRemovedItems = GetTagData(ATTR(DisposeRemovedItems), FALSE, inittags(msg)) ? TRUE : FALSE;
  }

  RETURN((IPTR)obj);
  return (IPTR)obj;
}

///
/// OVERLOAD(OM_SET)
OVERLOAD(OM_SET)
{
  GETDATA;
  struct TagItem *tags = inittags(msg);
  struct TagItem *tag;

  while((tag = NextTagItem((APTR)&tags)) != NULL)
  {
    switch(tag->ti_Tag)
    {
      case ATTR(DisposeRemovedItems):
      {
        data->disposeRemovedItems = (tag->ti_Data) ? TRUE : FALSE;
      }
      break;
    }
  }

  return DoSuperMethodA(cl, obj, msg);
}

///
/// OVERLOAD(OM_GET)
OVERLOAD(OM_GET)
{
  GETDATA;
  IPTR *store = ((struct opGet *)msg)->opg_Storage;

  switch(((struct opGet *)msg)->opg_AttrID)
  {
    case ATTR(DisposeRemovedItems):
    {
      *store = (IPTR)data->disposeRemovedItems;
      return TRUE;
    }
    break;

    case ATTR(FirstItem):
    {
      if(data->itemCount > 0)
      {
        struct List *childList = (struct List *)xget(data->virtgroup, MUIA_Group_ChildList);
        Object *cstate = (Object *)GetHead(childList);

        *store = (IPTR)NextObject(&cstate);
      }
      else
      {
        *store = (IPTR)NULL;
      }
      return TRUE;
    }
    break;

    case ATTR(LastItem):
    {
      if(data->itemCount > 0)
      {
        struct List *childList = (struct List *)xget(data->virtgroup, MUIA_Group_ChildList);
        Object *cstate = (Object *)GetHead(childList);
        Object *current;
        Object *last = NULL;

        // Unfortunately we cannot iterate backwards through the list,
        // thus we have to take the longer way forward...
        while((current = NextObject(&cstate)) != NULL)
        {
          if(current == data->dummy)
            continue;
          if(current == data->spacer)
            break;

          last = current;
        }

        *store = (IPTR)last;
      }
      else
      {
        *store = (IPTR)NULL;
      }
      return TRUE;
    }
    break;

    case ATTR(ItemCount):
    {
      *store = data->itemCount;
      return TRUE;
    }
    break;
  }

  return DoSuperMethodA(cl, obj, msg);
}

///
/// OVERLOAD(MUIM_AskMinMax)
OVERLOAD(MUIM_AskMinMax)
{
  struct MUI_MinMax *mi;

  // call the supermethod first
  DoSuperMethodA(cl, obj, msg);

  mi = ((struct MUIP_AskMinMax *)msg)->MinMaxInfo;

  mi->MaxWidth += MUI_MAXMAX;
  mi->MaxHeight += MUI_MAXMAX;

  return 0;
}

///
/// OVERLOAD(MUIM_Group_Sort)
OVERLOAD(MUIM_Group_Sort)
{
  GETDATA;

  ENTER();

  if(DoMethod(data->virtgroup, MUIM_Group_InitChange))
  {
    // the application doesn't know about our spacer object, so this one must
    // be removed ahead of the sort operation and be added again afterwards.
    DoMethod(data->virtgroup, OM_REMMEMBER, data->spacer);
    DoMethodA(data->virtgroup, msg);
    DoMethod(data->virtgroup, OM_ADDMEMBER, data->spacer);

    DoMethod(data->virtgroup, MUIM_Group_ExitChange);
  }

  LEAVE();
  return 0;
}

///

/* Private Functions */

/* Public Methods */
/// DECLARE(CreateItem)
DECLARE(CreateItem)
{
  ENTER();

  // this method must be overloaded, so we let it fail if we get here somehow

  RETURN((IPTR)NULL);
  return (IPTR)NULL;
}

///
/// DECLARE(AddItem)
DECLARE(AddItem) // Object *item
{
  GETDATA;
  ULONG result = FALSE;

  ENTER();

  if(msg->item != NULL && DoMethod(data->virtgroup, MUIM_Group_InitChange))
  {
    // now add the new item and move the spacer item behind to the last position
    DoMethod(data->virtgroup, OM_ADDMEMBER, msg->item);
    DoMethod(data->virtgroup, MUIM_Group_MoveMember, data->spacer, -1);
    DoMethod(data->virtgroup, MUIM_Group_ExitChange);

    // tell the item to which list it belongs
    set(msg->item, MUIA_ObjectListitem_ObjectList, obj);

    // make sure the new item is visible
    set(data->virtgroup, MUIA_Virtgroup_Top, _mtop(msg->item));

    data->itemCount++;
    // trigger possible notifications
    xset(obj, ATTR(ItemAdded), msg->item,
              ATTR(ItemsChanged), TRUE,
              ATTR(ItemCount), data->itemCount);

    result = TRUE;
  }

  RETURN(result);
  return result;
}

///
/// DECLARE(RemoveItem)
DECLARE(RemoveItem) // Object *item
{
  GETDATA;
  ULONG result = FALSE;

  ENTER();

  if(msg->item != NULL && DoMethod(data->virtgroup, MUIM_Group_InitChange))
  {
    // tell the item that it belongs to no list anymore
    set(msg->item, MUIA_ObjectListitem_ObjectList, NULL);

    DoMethod(data->virtgroup, OM_REMMEMBER, msg->item);
    DoMethod(data->virtgroup, MUIM_Group_ExitChange);

    data->itemCount--;
    // trigger possible notifications
    xset(obj, ATTR(ItemRemoved), msg->item,
              ATTR(ItemsChanged), TRUE,
              ATTR(ItemCount), data->itemCount);

    if(data->disposeRemovedItems == TRUE)
    {
      // dispose this object, but don't do it right now
      DoMethod(G->App, MUIM_Application_PushMethod, 2, msg->item, OM_DISPOSE);
    }

    result = TRUE;
  }

  RETURN(result);
  return result;
}

///
/// DECLARE(IterateItems)
DECLARE(IterateItems) // void **state
{
  GETDATA;
  Object *item = NULL;

  ENTER();

  if(data->itemCount > 0)
  {
    if(*msg->state == NULL)
    {
      struct List *childList = (struct List *)xget(data->virtgroup, MUIA_Group_ChildList);

      *msg->state = (Object *)GetHead(childList);
    }

    // iterate over the items, but skip the dummy object
    do
    {
      item = NextObject((Object **)msg->state);
    }
    while(item == data->dummy);

    if(item == data->spacer)
      item = NULL;
  }

  RETURN((IPTR)item);
  return (IPTR)item;
}

///
