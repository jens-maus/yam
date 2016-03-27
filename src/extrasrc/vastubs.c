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

 YAM Official Support Site :  http://www.yam.ch/
 YAM OpenSource project    :  http://sourceforge.net/projects/yamos/

 $Id$

***************************************************************************/

/*
   Stubs for the variable argument functions of the shared libraries used by
   YAM. Please note that these stubs should only be used if the compiler
   suite/SDK doesn't come with own stubs/inline functions.

   Also note that these stubs are only safe on m68k machines as it
   requires a linear stack layout!
*/

#include "extrasrc.h"

#if defined(NEED_VASTUBS)

#if !defined(__AROS__) && (defined(__VBCC__) || defined(NO_INLINE_STDARG))
#if defined(_M68000) || defined(__M68000) || defined(__mc68000)

#include <exec/types.h>

/* FIX V45 breakage... */
#if INCLUDE_VERSION < 45
#define MY_CONST_STRPTR CONST_STRPTR
#else
#define MY_CONST_STRPTR CONST STRPTR
#endif

#include <proto/intuition.h>
ULONG SetAttrs( APTR object, ULONG tag1, ... )
{ return SetAttrsA(object, (struct TagItem *)&tag1); }

#include <proto/dos.h>
LONG SystemTags( CONST_STRPTR command, ULONG tag1type, ... )
{ return SystemTagList(command, (struct TagItem *)&tag1type); }
struct Process *CreateNewProcTags( ULONG tag1, ... )
{ return CreateNewProc((struct TagItem *)&tag1); }

#include <proto/datatypes.h>
Object *NewDTObject( APTR name, Tag tag1, ... )
{ return NewDTObjectA(name, (struct TagItem *)&tag1); }
ULONG SetDTAttrs( Object *o, struct Window *win, struct Requester *req, Tag tag1, ... )
{ return SetDTAttrsA(o, win, req, (struct TagItem *)&tag1); }
ULONG GetDTAttrs( Object *o, Tag tag1, ... )
{ return GetDTAttrsA(o, (struct TagItem *)&tag1); }

#include <proto/wb.h>
struct AppIcon *AddAppIcon( ULONG id, ULONG userdata, UBYTE *text, struct MsgPort *msgport, BPTR lock, struct DiskObject *diskobj, Tag tag1, ... )
{ return AddAppIconA(id, userdata, text, msgport, lock, diskobj, (struct TagItem *)&tag1); }
BOOL WorkbenchControl(STRPTR name, ...)
{ return WorkbenchControlA(name,(struct TagItem *)(&name+1)); }

#include <proto/icon.h>
struct DiskObject *GetIconTags(MY_CONST_STRPTR name, ... )
{ return GetIconTagList(name, (struct TagItem *)(&name+1)); }
struct DiskObject *DupDiskObject(const struct DiskObject *dobj, ... )
{ return DupDiskObjectA(dobj, (struct TagItem *)(&dobj+1)); }

#include <proto/xpkmaster.h>
LONG XpkQueryTags(Tag tag, ...)
{ return XpkQuery((struct TagItem *)&tag); }
LONG XpkPackTags(Tag tag, ...)
{ return XpkPack((struct TagItem *)&tag); }
LONG XpkUnpackTags(Tag tag, ...)
{ return XpkUnpack((struct TagItem *)&tag); }

#include <proto/amissl.h>
LONG InitAmiSSL(Tag tag, ...)
{ return InitAmiSSLA((struct TagItem *)&tag); }

#include <proto/codesets.h>
STRPTR *CodesetsSupported(Tag tag1, ...)
{ return CodesetsSupportedA((struct TagItem *)&tag1); }
struct codeset *CodesetsFind(STRPTR name, Tag tag1, ...)
{ return CodesetsFindA(name, (struct TagItem *)&tag1); }
struct codeset *CodesetsFindBest(Tag tag1, ...)
{ return CodesetsFindBestA((struct TagItem *)&tag1); }
STRPTR CodesetsConvertStr(Tag tag1, ...)
{ return CodesetsConvertStrA((struct TagItem *)&tag1); }
struct codesetList *CodesetsListCreate(Tag tag1, ...)
{ return CodesetsListCreateA((struct TagItem *)&tag1); }
BOOL CodesetsListDelete(Tag tag1, ...)
{ return CodesetsListDeleteA((struct TagItem *)&tag1); }
STRPTR CodesetsUTF8ToStr(Tag tag1, ...)
{ return CodesetsUTF8ToStrA((struct TagItem *)&tag1); }
UTF8 *CodesetsUTF8Create(Tag tag1, ...)
{ return CodesetsUTF8CreateA((struct TagItem *)&tag1); }

#include <proto/openurl.h>
ULONG URL_Open(STRPTR url, Tag tag1, ...)
{ return URL_OpenA(url, (struct TagItem *)&tag1); }

#include <proto/locale.h>
struct Catalog *OpenCatalog(struct Locale *locale, STRPTR name, Tag tag1, ...)
{ return OpenCatalogA(locale, name, (struct TagItem *)&tag1); }

#else
  #error "VARGS stubs are only save on m68k systems!"
#endif // !defined(__PPC__)

#elif defined(__AROS__)

#include <proto/icon.h>
struct DiskObject *GetIconTags(CONST_STRPTR name, Tag tag1, ...)
{
  AROS_SLOWSTACKTAGS_PRE(tag1)
  retval = (IPTR)GetIconTagList((name), (const struct TagItem *)AROS_SLOWSTACKTAGS_ARG(tag1));
  AROS_SLOWSTACKTAGS_POST
}
struct DiskObject *DupDiskObject(const struct DiskObject *dobj, Tag tag1, ...)
{
  AROS_SLOWSTACKTAGS_PRE(tag1)
  retval = (IPTR)DupDiskObjectA((dobj), (const struct TagItem *)AROS_SLOWSTACKTAGS_ARG(tag1));
  AROS_SLOWSTACKTAGS_POST
}

#include <proto/xpkmaster.h>
LONG XpkQueryTags(Tag tag1, ...)
{
  AROS_SLOWSTACKTAGS_PRE(tag1)
  retval = XpkQuery((struct TagItem *)AROS_SLOWSTACKTAGS_ARG(tag1));
  AROS_SLOWSTACKTAGS_POST
}

LONG XpkPackTags(Tag tag1, ...)
{
  AROS_SLOWSTACKTAGS_PRE(tag1)
  retval = XpkPack((struct TagItem *)AROS_SLOWSTACKTAGS_ARG(tag1));
  AROS_SLOWSTACKTAGS_POST
}

LONG XpkUnpackTags(Tag tag1, ...)
{
  AROS_SLOWSTACKTAGS_PRE(tag1)
  retval = XpkUnpack((struct TagItem *)AROS_SLOWSTACKTAGS_ARG(tag1));
  AROS_SLOWSTACKTAGS_POST
}

#include <proto/amissl.h>
long InitAmiSSL(Tag tag1, ...)
{
  AROS_SLOWSTACKTAGS_PRE(tag1)
  retval = (IPTR)InitAmiSSLA((struct TagItem *)AROS_SLOWSTACKTAGS_ARG(tag1));
  AROS_SLOWSTACKTAGS_POST
}

#include <proto/codesets.h>
STRPTR *CodesetsSupported(Tag tag1, ...)
{
  AROS_SLOWSTACKTAGS_PRE(tag1)
  retval = (IPTR)CodesetsSupportedA((struct TagItem *)AROS_SLOWSTACKTAGS_ARG(tag1));
  AROS_SLOWSTACKTAGS_POST
}

struct codeset *CodesetsFind(STRPTR name, Tag tag1, ...)
{
  AROS_SLOWSTACKTAGS_PRE(tag1)
  retval = (IPTR)CodesetsFindA(name, (struct TagItem *)AROS_SLOWSTACKTAGS_ARG(tag1));
  AROS_SLOWSTACKTAGS_POST
}

struct codeset *CodesetsFindBest(Tag tag1, ...)
{
  AROS_SLOWSTACKTAGS_PRE(tag1)
  retval = (IPTR)CodesetsFindBestA((struct TagItem *)AROS_SLOWSTACKTAGS_ARG(tag1));
  AROS_SLOWSTACKTAGS_POST
}

STRPTR CodesetsConvertStr(Tag tag1, ...)
{
  AROS_SLOWSTACKTAGS_PRE(tag1)
  retval = (IPTR)CodesetsConvertStrA((struct TagItem *)AROS_SLOWSTACKTAGS_ARG(tag1));
  AROS_SLOWSTACKTAGS_POST
}

struct codesetList *CodesetsListCreate(Tag tag1, ...)
{
  AROS_SLOWSTACKTAGS_PRE(tag1)
  retval = (IPTR)CodesetsListCreateA((struct TagItem *)AROS_SLOWSTACKTAGS_ARG(tag1));
  AROS_SLOWSTACKTAGS_POST
}

BOOL CodesetsListDelete(Tag tag1, ...)
{
  AROS_SLOWSTACKTAGS_PRE(tag1)
  retval = (IPTR)CodesetsListDeleteA((struct TagItem *)AROS_SLOWSTACKTAGS_ARG(tag1));
  AROS_SLOWSTACKTAGS_POST
}

STRPTR CodesetsUTF8ToStr(Tag tag1, ...)
{
  AROS_SLOWSTACKTAGS_PRE(tag1)
  retval = (IPTR)CodesetsUTF8ToStrA((struct TagItem *)AROS_SLOWSTACKTAGS_ARG(tag1));
  AROS_SLOWSTACKTAGS_POST
}

UTF8 *CodesetsUTF8Create(Tag tag1, ...)
{
  AROS_SLOWSTACKTAGS_PRE(tag1)
  retval = (IPTR)CodesetsUTF8CreateA((struct TagItem *)AROS_SLOWSTACKTAGS_ARG(tag1));
  AROS_SLOWSTACKTAGS_POST
}

#include <proto/openurl.h>
ULONG URL_Open(STRPTR url, Tag tag1, ...)
{
  AROS_SLOWSTACKTAGS_PRE(tag1)
  retval = URL_OpenA(url, (struct TagItem *)AROS_SLOWSTACKTAGS_ARG(tag1));
  AROS_SLOWSTACKTAGS_POST
}

#endif

#else
  #warning "NEED_VASTUBS missing or compilation unnecessary"
#endif
