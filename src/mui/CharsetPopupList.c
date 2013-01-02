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
 YAM OpenSource project    : http://sourceforge.net/projects/yamos/

 $Id$

 Superclass:  MUIC_NList
 Description: a list showing all available charsets

***************************************************************************/

#include "CharsetPopupList_cl.h"

#include <string.h>
#include <proto/codesets.h>
#include <mui/NList_mcc.h>

#include "YAM.h"

#include "Debug.h"

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  ENTER();

  if((obj = DoSuperNew(cl, obj,

    InputListFrame,
    MUIA_NList_Format,        "BAR,",
    MUIA_NList_AutoVisible,   TRUE,
    MUIA_NList_ConstructHook, MUIV_NList_ConstructHook_String,
    MUIA_NList_DestructHook,  MUIV_NList_DestructHook_String,

    TAG_MORE, inittags(msg))) != NULL)
  {
    STRPTR *array;

    // Build list of available codesets
    if((array = CodesetsSupported(CSA_CodesetList, G->codesetsList,
                                  TAG_DONE)) != NULL)
    {
      DoMethod(obj, MUIM_NList_Insert, array, -1, MUIV_NList_Insert_Sorted);
      CodesetsFreeA(array, NULL);
    }
  }

  RETURN((IPTR)obj);
  return (IPTR)obj;
}

///
/// OVERLOAD(MUIM_NList_Display)
OVERLOAD(MUIM_NList_Display)
{
  struct MUIP_NList_Display *ndm = (struct MUIP_NList_Display *)msg;
  char *entry = (char *)ndm->entry;

  ENTER();

  if(entry != NULL)
  {
    struct codeset *cs;

    // the standard name is always in column 0
    ndm->strings[0] = entry;

    // try to find the codeset via codesets.library and
    // display some more information about it.
    if((cs = CodesetsFind(entry, CSA_CodesetList,       G->codesetsList,
                                 CSA_FallbackToDefault, FALSE,
                                 TAG_DONE)) != NULL)
    {
      if(cs->characterization != NULL && stricmp(cs->characterization, entry) != 0)
        ndm->strings[1] = cs->characterization;
      else
        ndm->strings[1] = (char *)"";
    }
    else
      ndm->strings[1] = (char *)"";
  }
  else
  {
    ndm->strings[0] = (char *)"";
    ndm->strings[1] = (char *)"";
  }

  LEAVE();
  return 0;
}

///
