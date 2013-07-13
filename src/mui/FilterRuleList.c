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
 YAM OpenSource project     : http://sourceforge.net/projects/yamos/

 $Id$

 Superclass:  MUIC_ObjectList
 Description: Displays all rules of a filter

***************************************************************************/

#include "FilterRuleList_cl.h"

#include "YAM_config.h"

#include "mui/ObjectList.h"
#include "mui/SearchControlGroup.h"

#include "Debug.h"

/* Hooks */

/* Private Functions */

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
    DoMethod(obj, MUIM_Notify, MUIA_ObjectList_ItemRemoved, MUIV_EveryTime, MUIV_Notify_Application, 2, MUIM_CallHook, &RemoveLastRuleHook);
  }

  RETURN((IPTR)obj);
  return (IPTR)obj;
}

///
/// OVERLOAD(MUIM_ObjectList_CreateItem)
OVERLOAD(MUIM_ObjectList_CreateItem)
{
  Object *item;

  ENTER();

  item = SearchControlGroupObject, End;

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
