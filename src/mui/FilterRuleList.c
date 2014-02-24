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

 Superclass:  MUIC_ObjectList
 Description: Displays all rules of a filter

***************************************************************************/

#include "FilterRuleList_cl.h"

#include "YAM_find.h"

#include "mui/ObjectList.h"
#include "mui/SearchControlGroup.h"

#include "Config.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  struct FilterNode *filter;
};
*/

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  ENTER();

  if((obj = DoSuperNew(cl, obj,
    TAG_MORE, inittags(msg))) != NULL)
  {
    Object *dummy;

    // we need a minimum of one dummy control group
    if((dummy = (Object *)DoMethod(obj, MUIM_ObjectList_CreateItem)) != NULL)
    {
      DoMethod(obj, MUIM_ObjectList_AddItem, dummy);
    }

    DoMethod(obj, MUIM_Notify, MUIA_ObjectList_ItemCount, MUIV_EveryTime, MUIV_Notify_Self, 2, METHOD(UpdateRules), MUIV_TriggerValue);
  }

  RETURN((IPTR)obj);
  return (IPTR)obj;
}

///
/// OVERLOAD(OM_SET)
OVERLOAD(OM_SET)
{
  GETDATA;
  struct TagItem *tags = inittags(msg), *tag;
  ULONG ret = 0;

  ENTER();

  while((tag = NextTagItem((APTR)&tags)) != NULL)
  {
    switch(tag->ti_Tag)
    {
      case ATTR(Filter):
      {
        data->filter = (struct FilterNode *)tag->ti_Data;
      }
      break;

      case MUIA_Disabled:
      {
        Object *ruleState = NULL;
        Object *ruleItem;

        // disable the individual rule objects, but not the complete group
        while((ruleItem = (Object *)DoMethod(obj, MUIM_ObjectList_IterateItems, &ruleState)) != NULL)
        {
          set(ruleItem, MUIA_Disabled, tag->ti_Data);
        }

        // let our superclass ignore this tag
        tag->ti_Tag = TAG_IGNORE;
      }
      break;
    }
  }

  ret = DoSuperMethodA(cl, obj, msg);

  RETURN(ret);
  return ret;
}

///
/// OVERLOAD(MUIM_ObjectList_CreateItem)
OVERLOAD(MUIM_ObjectList_CreateItem)
{
  GETDATA;
  Object *item;

  ENTER();

  item = SearchControlGroupObject,
    MUIA_SearchControlGroup_RemoteFilterMode, (data->filter != NULL) ? data->filter->remote : FALSE,
    MUIA_SearchControlGroup_AllowSpamStatus, CE->SpamFilterEnabled,
  End;

  RETURN((IPTR)item);
  return (IPTR)item;
}

///
/// DECLARE(UpdateRules)
DECLARE(UpdateRules) // ULONG numItems
{
  Object *ruleState = NULL;
  Object *ruleItem;

  ENTER();

  while((ruleItem = (Object *)DoMethod(obj, MUIM_ObjectList_IterateItems, &ruleState)) != NULL)
  {
    set(ruleItem, MUIA_SearchControlGroup_RemoveForbidden, msg->numItems < 2);
  }

  RETURN(0);
  return 0;
}

///
