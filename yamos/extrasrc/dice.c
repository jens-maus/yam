#include <proto/exec.h>
struct Library *WorkbenchBase;
struct Library *KeymapBase;
void __autoexit dice_closelibs(void)
{
   CloseLibrary(WorkbenchBase);
   CloseLibrary(KeymapBase);
}

void __autoinit dice_openlibs(void)
{
   WorkbenchBase=OpenLibrary("workbench.library",37);
   KeymapBase=OpenLibrary("keymap.library",37);

   if(!WorkbenchBase || !KeymapBase) exit(5);
}

extern struct Library *XpkBase;
#include <pragmas/xpkmaster_pragmas.h>
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

extern struct Library *OpenURLBase;
#include <clib/openurl_protos.h>
#include <pragmas/openurl_pragmas.h>
BOOL URL_Open(STRPTR str, ULONG tags, ...)
{
   struct TagItems *_tags =(struct TagItems *)&tags;
   return URL_OpenA(str,_tags);
}
