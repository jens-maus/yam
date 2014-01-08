/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2014 YAM Open Source Team

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

#include <proto/muimaster.h>

#include "YAM.h"

#include "MUIObjects.h"

#include "mui/ObjectListitem.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *virtgroup;
  Object *spacer;
  ULONG itemCount;
  BOOL disposeRemovedItems;
  ULONG quiet;
};
*/

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  Object *virtgroup = NULL;
  Object *spacer = NULL;

  ENTER();

  if((obj = DoSuperNew(cl, obj,

    MUIA_Scrollgroup_FreeHoriz, FALSE,
    MUIA_Scrollgroup_FreeVert, TRUE,
    MUIA_Scrollgroup_Contents, virtgroup = VGroupV,
      Child, spacer = HVSpace,
    End,

    TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;

    data->virtgroup = virtgroup;
    data->spacer = spacer;
    data->itemCount = 0;
    data->disposeRemovedItems = GetTagData(ATTR(DisposeRemovedItems), TRUE, inittags(msg)) ? TRUE : FALSE;
    data->quiet = 0;
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

      case ATTR(Quiet):
      {
        if(tag->ti_Data)
        {
          data->quiet++;
        }
        else
        {
          if(data->quiet > 0)
          {
            data->quiet--;
            if(data->quiet == 0)
            {
              set(obj, ATTR(ItemCount), data->itemCount);
            }
          }
        }
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

    case ATTR(ItemsChanged):
    {
      *store = TRUE;
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

  ENTER();

  // call the supermethod first
  DoSuperMethodA(cl, obj, msg);

  mi = ((struct MUIP_AskMinMax *)msg)->MinMaxInfo;

  mi->MaxWidth += MUI_MAXMAX;
  mi->MaxHeight += MUI_MAXMAX;

  RETURN(0);
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

  RETURN(0);
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

  if(msg->item != NULL)
  {
    if(DoMethod(data->virtgroup, MUIM_Group_InitChange))
    {
      // we must ensure that the spacer object is always the last object
      // in the group. As MUIM_Group_MoveMember is broken in MUI 3.8 we
      // simply remove the spacer before adding the new object and readd
      // it again afterwards. This effectively gives the same result
      // without the overhead of setting up a MUIM_Group_Sort message for
      // an aribitrary number of already existing objects.

      // remove the spacer item
      DoMethod(data->virtgroup, OM_REMMEMBER, data->spacer);
      // add the new item
      DoMethod(data->virtgroup, OM_ADDMEMBER, msg->item);
      // add the spacer item again
      DoMethod(data->virtgroup, OM_ADDMEMBER, data->spacer);

      DoMethod(data->virtgroup, MUIM_Group_ExitChange);

      // tell the item to which list it belongs
      set(msg->item, MUIA_ObjectListitem_ObjectList, obj);

      data->itemCount++;
      if(data->quiet == 0)
      {
        // make sure the new item is visible
        set(data->virtgroup, MUIA_Virtgroup_Top, _mtop(msg->item));

        // trigger possible notifications
        xset(obj, ATTR(ItemAdded), msg->item,
                  ATTR(ItemsChanged), TRUE,
                  ATTR(ItemCount), data->itemCount);
      }

      result = TRUE;
    }
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

    if(data->quiet == 0)
    {
      // trigger possible notifications
      xset(obj, ATTR(ItemRemoved), msg->item,
                ATTR(ItemsChanged), TRUE,
                ATTR(ItemCount), data->itemCount);
    }

    if(data->disposeRemovedItems == TRUE)
    {
      // dispose this item immediately
      MUI_DisposeObject(msg->item);
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

    item = NextObject((Object **)msg->state);
    if(item == data->spacer)
      item = NULL;
  }

  RETURN((IPTR)item);
  return (IPTR)item;
}

///
/// DECLARE(ItemAt)
DECLARE(ItemAt) // ULONG index
{
  GETDATA;
  Object *item = NULL;

  ENTER();

  if(data->itemCount > 0)
  {
    struct List *childList = (struct List *)xget(data->virtgroup, MUIA_Group_ChildList);
    Object *cstate = (Object *)GetHead(childList);
    ULONG idx = 0;

    while((item = NextObject(&cstate)) != NULL)
    {
      if(item == data->spacer)
        continue;

      if(idx == msg->index)
        break;

      idx++;
    }
  }

  RETURN((IPTR)item);
  return (IPTR)item;
}

///
/// DECLARE(Clear)
DECLARE(Clear)
{
  GETDATA;

  ENTER();

  if(data->itemCount > 0)
  {
    if(DoMethod(data->virtgroup, MUIM_Group_InitChange))
    {
      struct List *childList = (struct List *)xget(data->virtgroup, MUIA_Group_ChildList);
      Object *cstate = (Object *)GetHead(childList);
      Object *item;

      while((item = NextObject(&cstate)) != NULL)
      {
        // don't dispose the spacer item
        if(item == data->spacer)
          continue;

        // tell the item that it belongs to no list anymore
        set(item, MUIA_ObjectListitem_ObjectList, NULL);

        DoMethod(data->virtgroup, OM_REMMEMBER, item);

        if(data->disposeRemovedItems == TRUE)
        {
          // dispose this item immediately
          MUI_DisposeObject(item);
        }
      }

      DoMethod(data->virtgroup, MUIM_Group_ExitChange);

      data->itemCount = 0;

      if(data->quiet == 0)
      {
        // trigger possible notifications
        xset(obj, ATTR(ItemsChanged), TRUE,
                  ATTR(ItemCount), 0);
      }
    }
  }

  RETURN(0);
  return 0;
}

///
