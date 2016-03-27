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
 YAM OpenSource project    : http://sourceforge.net/projects/yamos/

 $Id$

 Superclass:  MUIC_NList
 Description: a list showing all available themes

***************************************************************************/

#include "ThemeList_cl.h"

#include <stdlib.h>
#include <string.h>
#include <proto/dos.h>
#include <mui/NList_mcc.h>

#include "Config.h"
#include "Themes.h"

#include "Debug.h"

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  ENTER();

  obj = DoSuperNew(cl, obj,

    InputListFrame,
    MUIA_NList_Format,   "",
    MUIA_NList_Title,    FALSE,
    MUIA_NList_DragType, MUIV_NList_DragType_None,

    TAG_MORE, inittags(msg));

  RETURN((IPTR)obj);
  return (IPTR)obj;
}

///
/// OVERLOAD(MUIM_NList_Construct)
OVERLOAD(MUIM_NList_Construct)
{
  struct MUIP_NList_Construct *ncm = (struct MUIP_NList_Construct *)msg;
  struct Theme *theme = ncm->entry;
  struct Theme *entry;

  ENTER();

  entry = memdup(theme, sizeof(*theme));

  RETURN((IPTR)entry);
  return (IPTR)entry;
}

///
/// OVERLOAD(MUIM_NList_Destruct)
OVERLOAD(MUIM_NList_Destruct)
{
  struct MUIP_NList_Destruct *ncm = (struct MUIP_NList_Destruct *)msg;
  struct Theme *theme = ncm->entry;

  ENTER();

  FreeTheme(theme);
  free(theme);

  RETURN(0);
  return 0;
}

///
/// OVERLOAD(MUIM_NList_Compare)
OVERLOAD(MUIM_NList_Compare)
{
  struct MUIP_NList_Compare *ncm = (struct MUIP_NList_Compare *)msg;
  struct Theme *theme1 = (struct Theme *)ncm->entry1;
  struct Theme *theme2 = (struct Theme *)ncm->entry2;
  LONG cmp;

  ENTER();

  cmp = stricmp(FilePart(theme1->directory), FilePart(theme2->directory));

  RETURN(cmp);
  return cmp;
}

///
/// OVERLOAD(MUIM_NList_Display)
OVERLOAD(MUIM_NList_Display)
{
  struct MUIP_NList_Display *ndm = (struct MUIP_NList_Display *)msg;
  struct Theme *entry = (struct Theme *)ndm->entry;

  ENTER();

  if(entry != NULL)
  {
    ndm->strings[0] = (char *)FilePart(entry->directory);

    if(stricmp(ndm->strings[0], CE->ThemeName) == 0)
      ndm->preparses[0] = (char *)"\033b";
  }

  RETURN(0);
  return 0;
}

///
