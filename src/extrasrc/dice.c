#if defined(_DCC) || defined(__VBCC__) || defined(NO_INLINE_STDARG)

#include <exec/types.h>

/* FIX V45 breakage... */
#if INCLUDE_VERSION < 45
#define MY_CONST_STRPTR CONST_STRPTR
#else
#define MY_CONST_STRPTR CONST STRPTR
#endif

#include <proto/wb.h>
BOOL WorkbenchControl(STRPTR name, ...)
{ return WorkbenchControlA(name,(struct TagItem *)(&name+1)); }

#include <proto/icon.h>
struct DiskObject *GetIconTags(MY_CONST_STRPTR name, ... )
{ return GetIconTagList(name, (struct TagItem *)(&name+1)); }

#include <proto/xpkmaster.h>
LONG XpkQueryTags(Tag tag, ...)
{ return XpkQuery((struct TagItem *)&tag); }
LONG XpkPackTags(Tag tag, ...)
{ return XpkPack((struct TagItem *)&tag); }
LONG XpkUnpackTags(Tag tag, ...)
{ return XpkUnpack((struct TagItem *)&tag); }

#include <proto/amissl.h>
long InitAmiSSL(Tag tag, ...)
{ return InitAmiSSLA((struct TagItem *)&tag); }

#include <proto/codesets.h>
STRPTR *CodesetsSupported(Tag tag1, ...)
{ return CodesetsSupportedA((struct TagItem *)&tag1); }
struct codeset *CodesetsFind(STRPTR name, Tag tag1, ...)
{ return CodesetsFindA(name, (struct TagItem *)&tag1); }
STRPTR CodesetsConvertStr(Tag tag1, ...)
{ return CodesetsConvertStrA((struct TagItem *)&tag1); }
BOOL CodesetsListDelete(Tag tag1, ...)
{ return CodesetsListDeleteA((struct TagItem *)&tag1); }

#endif
