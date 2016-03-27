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
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 YAM Official Support Site :  http://www.yam.ch
 YAM OpenSource project    :  http://sourceforge.net/projects/yamos/

 $Id$

***************************************************************************/

#include <proto/intuition.h>

#if !defined(__amigaos4__)
#include <clib/alib_protos.h>
#endif

#include "SDI_compiler.h"
#include "SDI_stdarg.h"

#include "extrasrc.h"

#if defined(NEED_DOSUPERNEW)

// DoSuperNew()
// Calls parent NEW method within a subclass
Object * VARARGS68K DoSuperNew(struct IClass *cl, Object *obj, ...)
{
  Object *rc;
  VA_LIST args;

  VA_START(args, obj);
  #if defined(__AROS__)
  rc = (Object *)DoSuperNewTagList(cl, obj, NULL, (struct TagItem *)VA_ARG(args, IPTR));
  #else
  rc = (Object *)DoSuperMethod(cl, obj, OM_NEW, VA_ARG(args, ULONG), NULL);
  #endif
  VA_END(args);

  return rc;
}

#else
  #warning "NEED_DOSUPERNEW missing or compilation unnecessary"
#endif
