#ifndef _VBCCINLINE_OPENURL_H
#define _VBCCINLINE_OPENURL_H

BOOL __URL_OpenA(__reg("a0") STRPTR url, __reg("a1") struct TagItem * tags, __reg("a6") void *)="\tjsr\t-30(a6)";
#define URL_OpenA(url, tags) __URL_OpenA((url), (tags), OpenURLBase)

struct URL_Prefs * __URL_GetPrefs(__reg("a6") void *)="\tjsr\t-36(a6)";
#define URL_GetPrefs() __URL_GetPrefs(OpenURLBase)

VOID __URL_FreePrefs(__reg("a0") struct URL_Prefs * up, __reg("a6") void *)="\tjsr\t-42(a6)";
#define URL_FreePrefs(up) __URL_FreePrefs((up), OpenURLBase)

BOOL __URL_SetPrefs(__reg("a0") struct URL_Prefs * up, __reg("d0") BOOL permanent, __reg("a6") void *)="\tjsr\t-48(a6)";
#define URL_SetPrefs(up, permanent) __URL_SetPrefs((up), (permanent), OpenURLBase)

struct URL_Prefs * __URL_GetDefaultPrefs(__reg("a6") void *)="\tjsr\t-54(a6)";
#define URL_GetDefaultPrefs() __URL_GetDefaultPrefs(OpenURLBase)

BOOL __URL_LaunchPrefsApp(__reg("a6") void *)="\tjsr\t-60(a6)";
#define URL_LaunchPrefsApp() __URL_LaunchPrefsApp(OpenURLBase)

#endif /*  _VBCCINLINE_OPENURL_H  */
