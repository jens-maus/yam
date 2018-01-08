/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2018 YAM Open Source Team

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
 Description: a list showing all available users

***************************************************************************/

#include "UserList_cl.h"

#include <stdlib.h>
#include <mui/NList_mcc.h>

#include "YAM.h"

#include "Locale.h"

#include "Debug.h"

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  ENTER();

  obj = DoSuperNew(cl, obj,

    InputListFrame,
    MUIA_NList_TitleSeparator, TRUE,
    MUIA_NList_Title, TRUE,
    MUIA_NList_Format, "BAR,",

    TAG_MORE, inittags(msg));

  RETURN((IPTR)obj);
  return (IPTR)obj;
}

///
/// OVERLOAD(MUIM_NList_Construct)
OVERLOAD(MUIM_NList_Construct)
{
  struct MUIP_NList_Construct *ncm = (struct MUIP_NList_Construct *)msg;
  struct User *user = ncm->entry;
  struct User *entry;

  ENTER();

  entry = memdup(user, sizeof(*user));

  RETURN((IPTR)entry);
  return (IPTR)entry;
}

///
/// OVERLOAD(MUIM_NList_Destruct)
OVERLOAD(MUIM_NList_Destruct)
{
  struct MUIP_NList_Destruct *ncm = (struct MUIP_NList_Destruct *)msg;
  struct User *user = ncm->entry;

  ENTER();

  free(user);

  RETURN(0);
  return 0;
}

///
/// OVERLOAD(MUIM_NList_Display)
OVERLOAD(MUIM_NList_Display)
{
  struct MUIP_NList_Display *ndm = (struct MUIP_NList_Display *)msg;
  struct User *entry = (struct User *)ndm->entry;

  ENTER();

  if(entry != NULL)
  {
    ndm->strings[0] = entry->Name;
    ndm->strings[1] = entry->MailDir;

    if(entry->ID == G->Users.CurrentID)
      ndm->preparses[0] = (char *)"\0338";
  }
  else
  {
    ndm->strings[0] = (char *)tr(MSG_US_TitleUserName);
    ndm->strings[1] = (char *)tr(MSG_US_TitleMailDir);
  }

  RETURN(0);
  return 0;
}

///
