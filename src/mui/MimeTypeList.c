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
 YAM OpenSource project    : http://sourceforge.net/projects/yamos/

 $Id$

 Superclass:  MUIC_NList
 Description: a list showing all available MIME types

***************************************************************************/

#include "MimeTypeList_cl.h"

#include <string.h>
#include <mui/NList_mcc.h>

#include "MimeTypes.h"

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
/// OVERLOAD(MUIM_NList_Compare)
OVERLOAD(MUIM_NList_Compare)
{
  struct MUIP_NList_Compare *ncm = (struct MUIP_NList_Compare *)msg;
  struct MimeTypeNode *mt1 = (struct MimeTypeNode *)ncm->entry1;
  struct MimeTypeNode *mt2 = (struct MimeTypeNode *)ncm->entry2;
  char class1[SIZE_CTYPE];
  char class2[SIZE_CTYPE];
  char *type1;
  char *type2;
  LONG cmp;

  ENTER();

  strlcpy(class1, mt1->ContentType, sizeof(class1));
  strlcpy(class2, mt2->ContentType, sizeof(class2));

  // split the content-type in its two parts
  if((type1 = strchr(class1, '/')) != NULL)
    *type1++ = '\0';
  else
    type1 = (char *)"";
  if((type2 = strchr(class2, '/')) != NULL)
    *type2++ = '\0';
  else
    type2 = (char *)"";

  // first check if any is a catch all
  // these are put to the end of each class
  cmp = stricmp(class1, class2);
  if(cmp == 0)
  {
    // the class is the same, now take a look at the type
    // the catch-all is sorted in front of all the others
    // as '*' and '?' are forbidden for MIME types it is enough to check just for these
    // two chars.
    if(type1[0] == '*' || type1[0] == '#' || type1[0] == '?')
      cmp = +1;
    else if(type2[0] == '*' || type2[0] == '#' || type2[0] == '?')
      cmp = -1;
    else
      cmp = stricmp(type1, type2);
  }

  RETURN(cmp);
  return cmp;
}

///
/// OVERLOAD(MUIM_NList_Display)
OVERLOAD(MUIM_NList_Display)
{
  struct MUIP_NList_Display *ndm = (struct MUIP_NList_Display *)msg;
  struct MimeTypeNode *entry = (struct MimeTypeNode *)ndm->entry;

  ENTER();

  if(entry != NULL)
  {
    ndm->strings[0] = entry->ContentType;
  }

  RETURN(0);
  return 0;
}

///
