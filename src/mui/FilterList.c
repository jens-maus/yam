/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2011 YAM Open Source Team

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

 Superclass:  MUIC_NList
 Description: a list showing all available filters

***************************************************************************/

#include "FilterList_cl.h"

#include <mui/NList_mcc.h>

#include "YAM_find.h"

#include "Locale.h"

#include "Debug.h"

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  ENTER();

  obj = DoSuperNew(cl, obj,

    InputListFrame,
    MUIA_NList_Format,       "BAR, P=\033c NB CW=1 MICW=1 MACW=1, P=\033c NB CW=1 MICW=1 MACW=1, P=\033c NB CW=1 MICW=1 MACW=1, P=\033c NB CW=1 MICW=1 MACW=1",
    MUIA_NList_Title,        TRUE,
    MUIA_NList_DragType,     MUIV_NList_DragType_Immediate,
    MUIA_NList_DragSortable, TRUE,

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
    ndm->strings[1] = (entry->remote == TRUE) ? (char *)"x" : (char *)" ";
    ndm->strings[2] = (entry->applyToNew == TRUE && entry->remote == FALSE) ?  (char *)"x" : (char *)" ";
    ndm->strings[3] = (entry->applyToSent == TRUE && entry->remote == FALSE) ? (char *)"x" : (char *)" ";
    ndm->strings[4] = (entry->applyOnReq == TRUE && entry->remote == FALSE) ?  (char *)"x" : (char *)" ";
  }
  else
  {
    ndm->strings[0] = (char *)tr(MSG_CO_Filter_Name);
    ndm->strings[1] = (char *)tr(MSG_CO_Filter_RType);
    ndm->strings[2] = (char *)tr(MSG_CO_Filter_NType);
    ndm->strings[3] = (char *)tr(MSG_CO_Filter_SType);
    ndm->strings[4] = (char *)tr(MSG_CO_Filter_UType);
  }

  LEAVE();
  return 0;
}

///
