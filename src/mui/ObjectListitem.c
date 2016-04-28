/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2016 YAM Open Source Team

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
 Description: Abstract base class for ObjectListitem objects to be
              displayed in a ObjectList object

***************************************************************************/

#include "ObjectListitem_cl.h"

#include "MUIObjects.h"

#include "mui/ObjectList.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *objectList;
};
*/

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  ENTER();

  if((obj = DoSuperNew(cl, obj,
    MUIA_Group_Horiz, FALSE,

    TAG_MORE, inittags(msg))) != NULL)
  {
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
      case ATTR(ObjectList):
      {
        data->objectList = (Object *)tag->ti_Data;
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
    case ATTR(ObjectList):
    {
      *store = (IPTR)data->objectList;

      return TRUE;
    }
    break;

    case ATTR(IsFirstItem):
    {
      if(data->objectList != NULL)
        *store = (obj == (Object *)xget(data->objectList, MUIA_ObjectList_FirstItem));
      else
      {
        W(DBF_GUI, "get(MUIA_ObjectListItem_IsFirstItem) called for orphaned object item %08lx", obj);
        *store = FALSE;
      }

      return TRUE;
    }
    break;

    case ATTR(IsLastItem):
    {
      if(data->objectList != NULL)
        *store = (obj == (Object *)xget(data->objectList, MUIA_ObjectList_LastItem));
      else
      {
        W(DBF_GUI, "get(MUIA_ObjectListItem_IsLastItem) called for orphaned object item %08lx", obj);
        *store = FALSE;
      }

      return TRUE;
    }
    break;
  }

  return DoSuperMethodA(cl, obj, msg);
}

///

/* Private Functions */

/* Public Methods */
/// DECLARE(CreateAndAddItem)
// let the objectlist create and add a new instance of this class
DECLARE(CreateAndAddItem)
{
  GETDATA;
  Object *item = NULL;

  ENTER();

  if(data->objectList != NULL)
  {
    if((item = (Object *)DoMethod(data->objectList, MUIM_ObjectList_CreateItem)) != NULL)
    {
      DoMethod(data->objectList, MUIM_ObjectList_AddItem, item);
    }
  }
  else
    E(DBF_GUI, "MUIM_ObjectListItem_CreateAndAdd() called for orphaned object item %08lx", obj);

  RETURN((IPTR)item);
  return (IPTR)item;
}

///
/// DECLARE(Remove)
// remove ourself from our object list
DECLARE(Remove)
{
  GETDATA;

  ENTER();

  if(data->objectList != NULL)
    DoMethod(data->objectList, MUIM_ObjectList_RemoveItem, obj);
  else
    E(DBF_GUI, "MUIM_ObjectListItem_Remove() called for orphaned object item %08lx", obj);

  RETURN(0);
  return 0;
}

///
