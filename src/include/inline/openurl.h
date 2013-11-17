/* Automatically generated header! Do not edit! */

#ifndef _INLINE_OPENURL_H
#define _INLINE_OPENURL_H

#ifndef __INLINE_MACROS_H
#include <inline/macros.h>
#endif /* !__INLINE_MACROS_H */

#ifndef OPENURL_BASE_NAME
#define OPENURL_BASE_NAME OpenURLBase
#endif /* !OPENURL_BASE_NAME */

#define URL_FreePrefs(up) \
  LP1NR(0x2a, URL_FreePrefs, struct URL_Prefs *, up, a0, \
  , OPENURL_BASE_NAME)

#define URL_GetDefaultPrefs() \
  LP0(0x36, struct URL_Prefs *, URL_GetDefaultPrefs, \
  , OPENURL_BASE_NAME)

#define URL_GetPrefs() \
  LP0(0x24, struct URL_Prefs *, URL_GetPrefs, \
  , OPENURL_BASE_NAME)

#define URL_LaunchPrefsApp() \
  LP0(0x3c, BOOL, URL_LaunchPrefsApp, \
  , OPENURL_BASE_NAME)

#define URL_OpenA(url, tags) \
  LP2(0x1e, BOOL, URL_OpenA, STRPTR, url, a0, struct TagItem *, tags, a1, \
  , OPENURL_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define URL_Open(a0, tags...) \
  ({ULONG _tags[] = { tags }; URL_OpenA((a0), (struct TagItem *)_tags);})
#endif /* !NO_INLINE_STDARG */

#define URL_SetPrefs(up, permanent) \
  LP2(0x30, BOOL, URL_SetPrefs, struct URL_Prefs *, up, a0, BOOL, permanent, d0, \
  , OPENURL_BASE_NAME)

#endif /* !_INLINE_OPENURL_H */
