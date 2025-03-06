/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2025 YAM Open Source Team

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
 YAM OpenSource project    : https://github.com/jens-maus/yam/

 Superclass:  MUIC_NList
 Description: a list showing all available filters

***************************************************************************/

#include "FilterPopupList_cl.h"

#include <mui/NList_mcc.h>

#include "YAM_find.h"

#include "Config.h"

#include "Debug.h"

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  ENTER();

  obj = DoSuperNew(cl, obj,

    InputListFrame,

    TAG_MORE, inittags(msg));

  RETURN((IPTR)obj);
  return (IPTR)obj;
}

///
/// OVERLOAD(MUIM_NList_Display)
OVERLOAD(MUIM_NList_Display)
{
  struct MUIP_NList_Display *ndm = (struct MUIP_NList_Display *)msg;
  struct FilterNode *entry = (struct FilterNode *)ndm->entry;

  ENTER();

  if(entry != NULL)
  {
    ndm->strings[0] = entry->name;
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(Popup)
DECLARE(Popup)
{
  struct Node *node;

  ENTER();

  // clear the list first
  DoMethod(obj, MUIM_List_Clear);

  // add all available filters
  IterateList(&C->filterList, struct Node *, node)
  {
    DoMethod(obj, MUIM_NList_InsertSingle, node, MUIV_NList_Insert_Bottom);
  }

  RETURN(0);
  return 0;
}

///
