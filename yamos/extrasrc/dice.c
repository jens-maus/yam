#if defined(_DCC) || defined(NO_INLINE_STDARG)
#include <proto/xpkmaster.h>
LONG XpkQueryTags(ULONG tags, ...)
{
   struct TagItems *_tags =(struct TagItems *)&tags;
   return XpkQuery(_tags);
}

LONG XpkPackTags(ULONG tags, ...)
{
   struct TagItems *_tags =(struct TagItems *)&tags;
   return XpkPack(_tags);
}

LONG XpkUnpackTags(ULONG tags, ...)
{
   struct TagItems *_tags =(struct TagItems *)&tags;
   return XpkUnpack(_tags);
}


#include <proto/openurl.h>
BOOL URL_Open(STRPTR str, ULONG tags, ...)
{
   struct TagItems *_tags =(struct TagItems *)&tags;
   return URL_OpenA(str,_tags);
}
#endif

#ifdef _DCC
struct Library *KeymapBase;
#endif
