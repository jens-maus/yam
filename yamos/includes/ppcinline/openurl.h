#ifndef _PPCINLINE_OPENURL_H
#define _PPCINLINE_OPENURL_H

#ifndef __PPCINLINE_MACROS_H
#include <ppcinline/macros.h>
#endif

#ifndef OPENURL_BASE_NAME
#define OPENURL_BASE_NAME OpenURLBase
#endif

#define URL_OpenA(par1, last) \
	LP2(0x1e, BOOL, URL_OpenA, STRPTR, par1, a0, struct TagItem *, last, a1, \
	, OPENURL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define URL_Open(par1, tags...) \
	({ULONG _tags[] = {tags}; URL_OpenA((par1), (struct TagItem *) _tags);})
#endif

#define URL_GetPrefs() \
	LP0(0x24, struct URL_Prefs *, URL_GetPrefs, \
	, OPENURL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define URL_FreePrefs(last) \
	LP1NR(0x2a, URL_FreePrefs, struct URL_Prefs *, last, a0, \
	, OPENURL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define URL_SetPrefs(par1, last) \
	LP2(0x30, BOOL, URL_SetPrefs, struct URL_Prefs *, par1, a0, BOOL, last, d0, \
	, OPENURL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define URL_GetDefaultPrefs() \
	LP0(0x36, struct URL_Prefs *, URL_GetDefaultPrefs, \
	, OPENURL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define URL_LaunchPrefsApp() \
	LP0(0x3c, BOOL, URL_LaunchPrefsApp, \
	, OPENURL_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#endif /*  _PPCINLINE_OPENURL_H  */
