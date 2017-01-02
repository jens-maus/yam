/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2017 YAM Open Source Team

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
 Description: a list showing all attachment reminder keywords

***************************************************************************/

#include "AttachmentKeywordList_cl.h"

#include <string.h>

#include <mui/NList_mcc.h>

#include "YAM_utilities.h"

#include "DynamicString.h"

#include "Debug.h"

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  ENTER();

  obj = DoSuperNew(cl, obj,

    InputListFrame,
    MUIA_NList_Title,         FALSE,
    MUIA_NList_ConstructHook, MUIV_NList_ConstructHook_String,
    MUIA_NList_DestructHook,  MUIV_NList_DestructHook_String,

    TAG_MORE, inittags(msg));

  RETURN((IPTR)obj);
  return (IPTR)obj;
}

///
/// OVERLOAD(OM_SET)
OVERLOAD(OM_SET)
{
  struct TagItem *tags = inittags(msg), *tag;

  while((tag = NextTagItem((APTR)&tags)) != NULL)
  {
    switch(tag->ti_Tag)
    {
      case ATTR(Keywords):
      {
        char **keywords;

        // split the string in individual words and add them to the list
        if((keywords = SplitString((char *)tag->ti_Data, ",")) != NULL)
        {
          DoMethod(obj, MUIM_NList_Clear);
          DoMethod(obj, MUIM_NList_Insert, keywords, -1, MUIV_NList_Insert_Bottom);

          FreeStrArray(keywords);
        }

        tag->ti_Tag = TAG_IGNORE;
      }
      break;
    }
  }

  return DoSuperMethodA(cl, obj, msg);
}

///

/* Public Methods */
/// DECLARE(AddKeyword)
// adds a new keyword to the list
DECLARE(AddKeyword) // const char *keyword
{
  ENTER();

  DoMethod(obj, MUIM_NList_InsertSingle, msg->keyword, MUIV_NList_Insert_Bottom);
  set(obj, MUIA_NList_Active, MUIV_NList_Active_Bottom);

  RETURN(0);
  return 0;
}

///
/// DECLARE(ModifyKeyword)
// replaces the currently active keyword
DECLARE(ModifyKeyword) // const char *keyword
{
  ENTER();

  DoMethod(obj, MUIM_NList_ReplaceSingle, msg->keyword, MUIV_NList_Insert_Active, NOWRAP, ALIGN_LEFT);

  RETURN(0);
  return 0;
}

///
/// DECLARE(GetKeywords)
// build a comma separated string of keywords
DECLARE(GetKeywords) // char *str, size_t strsize
{
  LONG i;
  char *keywords;

  ENTER();

  i = 0;
  keywords = NULL;
  do
  {
    char *keyword;

    if((keyword = (char *)DoMethod(obj, MUIM_NList_GetEntry, i, NULL)) != NULL)
    {
      if(i != 0)
        dstrcat(&keywords, ",");

      dstrcat(&keywords, keyword);
      i++;
    }
    else
    {
      break;
    }
  }
  while(TRUE);

  if(keywords != NULL)
  {
    strlcpy(msg->str, keywords, msg->strsize);
    dstrfree(keywords);
  }

  RETURN(0);
  return 0;
}

///
