#if defined(_DCC) || defined(__VBCC__) || defined(NO_INLINE_STDARG)

#include <proto/xpkmaster.h>
LONG XpkQueryTags(ULONG tag, ...)
{ return XpkQuery((struct TagItem *)&tag); }
LONG XpkPackTags(ULONG tag, ...)
{ return XpkPack((struct TagItem *)&tag); }
LONG XpkUnpackTags(ULONG tag, ...)
{ return XpkUnpack((struct TagItem *)&tag); }

#include <proto/openurl.h>
BOOL URL_Open(STRPTR str, ULONG tag, ...)
{ return URL_OpenA(str,(struct TagItem *)&tag); }

#include <proto/pm.h>
struct PopupMenu *PM_MakeItem(ULONG tag, ...)
{ return PM_MakeItemA((struct TagItem *)&tag); }
struct PopupMenu *PM_MakeMenu(ULONG tag, ...)
{ return PM_MakeMenuA((struct TagItem *)&tag); }
ULONG PM_OpenPopupMenu(struct Window *prevwnd, ULONG tag, ...)
{ return PM_OpenPopupMenuA(prevwnd,(struct TagItem *)&tag); }

#include <proto/amissl.h>
long InitAmiSSL(ULONG tag, ...)
{ return InitAmiSSLA((struct TagItem *)&tag); }
long CleanupAmiSSL(ULONG tag, ...)
{ return CleanupAmiSSLA((struct TagItem *)&tag); }

#endif
