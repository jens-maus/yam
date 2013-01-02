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

***************************************************************************/

#include "SDI_compiler.h"
#include "SDI_stdarg.h"

#include <proto/intuition.h>
#include <proto/utility.h>

#include "extrasrc.h"

#if defined(NEED_XSET)

#warning possible incomplete tag list parsing!!

ULONG VARARGS68K xset(Object *obj, ...)
{
  VA_LIST args;
  struct TagItem *tags;
  struct TagItem *tag;
  struct TagItem newTags[6];
  struct TagItem *newTag = newTags;
  int i;

  VA_START(args, obj);
  tags = (struct TagItem *)VA_ARG(args, struct TagItem *);

  // We will try to rebuild a new tag list with a terminating TAG_DONE.
  // This approach is not perfect as we have to "guess" the end of the
  // supplied tag list or at least have to set a definite limit on the
  // number of usable attributes. xset() calls usually are quite "simple",
  // hence we accept up to 5 attributes at most to be set with one call.
  i = 0;
  while(i < (sizeof(newTags) / sizeof(newTags[0])) - 1 && (tag = NextTagItem(&tags)) != NULL)
  {
    if(tag->ti_Tag >= TAG_USER)
    {
      // copy any user defined tag item
      newTag->ti_Tag = tag->ti_Tag;
      newTag->ti_Data = tag->ti_Data;
      newTag++;
      i++;
    }
  }
  VA_END(args);

  // add the terminating TAG_DONE
  newTag->ti_Tag = TAG_DONE;

  return SetAttrsA(obj, newTags);
}

#else
  #warning "NEED_XSET missing or compilation unnecessary"
#endif
