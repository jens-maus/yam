/* Automatically generated header! Do not edit! */

#ifndef _PPCINLINE_OPENURL_H
#define _PPCINLINE_OPENURL_H

#ifndef __PPCINLINE_MACROS_H
#include <ppcinline/macros.h>
#endif /* !__PPCINLINE_MACROS_H */

#ifndef OPENURL_BASE_NAME
#define OPENURL_BASE_NAME OpenURLBase
#endif /* !OPENURL_BASE_NAME */

#define URL_GetPrefs() \
	LP0(36, struct URL_Prefs *, URL_GetPrefs, \
		, OPENURL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define URL_FreePrefs(__p0) \
	LP1NR(42, URL_FreePrefs, \
		struct URL_Prefs *, __p0, a0, \
		, OPENURL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define URL_OpenA(__p0, __p1) \
	LP2(30, BOOL , URL_OpenA, \
		STRPTR , __p0, a0, \
		struct TagItem *, __p1, a1, \
		, OPENURL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define URL_SetPrefs(__p0, __p1) \
	LP2(48, BOOL , URL_SetPrefs, \
		struct URL_Prefs *, __p0, a0, \
		BOOL , __p1, d0, \
		, OPENURL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define URL_LaunchPrefsApp() \
	LP0(60, BOOL , URL_LaunchPrefsApp, \
		, OPENURL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define URL_GetDefaultPrefs() \
	LP0(54, struct URL_Prefs *, URL_GetDefaultPrefs, \
		, OPENURL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#ifdef USE_INLINE_STDARG

#include <stdarg.h>

#define URL_Open(__p0, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	URL_OpenA(__p0, (struct TagItem *)_tags);})

#endif

#endif /* !_PPCINLINE_OPENURL_H */
