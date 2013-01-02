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

 Superclass:  MUIC_NList
 Description: NList class of an addressbook entry

***************************************************************************/

#include "AddrBookEntryList_cl.h"

#include "YAM.h"
#include "YAM_addressbook.h"
#include "YAM_addressbookEntry.h"

#include "MUIObjects.h"

#include "Debug.h"

/* Overloaded Methods */
/// OVERLOAD(MUIM_DragQuery)
OVERLOAD(MUIM_DragQuery)
{
  struct MUIP_DragQuery *d = (struct MUIP_DragQuery *)msg;

  if(d->obj != obj)
  {
    if(d->obj == G->AB->GUI.LV_ADDRESSES)
    {
      struct MUI_NListtree_TreeNode *active;

      if((active = (struct MUI_NListtree_TreeNode *)xget(d->obj, MUIA_NListtree_Active)))
      {
        if(!((struct ABEntry *)(active->tn_User))->Members)
          return MUIV_DragQuery_Accept;
      }
    }

    return MUIV_DragQuery_Refuse;
  }

  return DoSuperMethodA(cl,obj,msg);
}

///
/// OVERLOAD(MUIM_DragDrop)
OVERLOAD(MUIM_DragDrop)
{
  struct MUIP_DragQuery *d = (struct MUIP_DragQuery *)msg;

  if(d->obj != obj)
  {
    if(d->obj == G->AB->GUI.LV_ADDRESSES)
    {
      struct MUI_NListtree_TreeNode *active;

      if((active = (struct MUI_NListtree_TreeNode *)xget(d->obj, MUIA_NListtree_Active)))
      {
        if(isFlagSet(active->tn_Flags, TNF_LIST))
          EA_AddMembers(obj, active);
        else
          EA_AddSingleMember(obj, active);
      }
    }

    return 0;
  }

  return DoSuperMethodA(cl,obj,msg);
}

///

/* Private Functions */

/* Public Methods */
