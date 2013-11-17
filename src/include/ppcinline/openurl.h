/* Automatically generated header! Do not edit! */

#ifndef _PPCINLINE_OPENURL_H
#define _PPCINLINE_OPENURL_H

#ifndef __PPCINLINE_MACROS_H
#include <ppcinline/macros.h>
#endif /* !__PPCINLINE_MACROS_H */

#ifndef OPENURL_BASE_NAME
#define OPENURL_BASE_NAME OpenURLBase
#endif /* !OPENURL_BASE_NAME */

#define URL_OldGetPrefs() \
  LP0(36, struct URL_Prefs *, URL_OldGetPrefs, \
    , OPENURL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define URL_OldFreePrefs(__p0) \
  LP1NR(42, URL_OldFreePrefs, \
    struct URL_Prefs *, __p0, a0, \
    , OPENURL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define URL_GetPrefsA(__p0) \
  LP1(72, struct URL_Prefs *, URL_GetPrefsA, \
    struct TagItem *, __p0, a0, \
    , OPENURL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define URL_OpenA(__p0, __p1) \
  LP2(30, ULONG , URL_OpenA, \
    STRPTR , __p0, a0, \
    struct TagItem *, __p1, a1, \
    , OPENURL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define URL_FreePrefsA(__p0, __p1) \
  LP2NR(78, URL_FreePrefsA, \
    struct URL_Prefs *, __p0, a0, \
    struct TagItem *, __p1, a1, \
    , OPENURL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define URL_OldGetDefaultPrefs() \
  LP0(54, struct URL_Prefs *, URL_OldGetDefaultPrefs, \
    , OPENURL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define URL_OldLaunchPrefsApp() \
  LP0(60, ULONG , URL_OldLaunchPrefsApp, \
    , OPENURL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define URL_OldSetPrefs(__p0, __p1) \
  LP2(48, ULONG , URL_OldSetPrefs, \
    struct URL_Prefs *, __p0, a0, \
    BOOL , __p1, d0, \
    , OPENURL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define URL_GetAttr(__p0, __p1) \
  LP2(96, ULONG , URL_GetAttr, \
    ULONG , __p0, d0, \
    ULONG *, __p1, a0, \
    , OPENURL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define URL_SetPrefsA(__p0, __p1) \
  LP2(84, ULONG , URL_SetPrefsA, \
    struct URL_Prefs *, __p0, a0, \
    struct TagItem *, __p1, a1, \
    , OPENURL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define URL_LaunchPrefsAppA(__p0) \
  LP1(90, ULONG , URL_LaunchPrefsAppA, \
    struct TagItem *, __p0, a0, \
    , OPENURL_BASE_NAME, 0, 0, 0, 0, 0, 0)

#if defined(USE_INLINE_STDARG) && !defined(__STRICT_ANSI__)

#include <stdarg.h>

#define URL_GetPrefs(...) \
  ({ULONG _tags[] = { __VA_ARGS__ }; \
  URL_GetPrefsA((struct TagItem *)_tags);})

#define URL_Open(__p0, ...) \
  ({ULONG _tags[] = { __VA_ARGS__ }; \
  URL_OpenA(__p0, (struct TagItem *)_tags);})

#define URL_FreePrefs(__p0, ...) \
  ({ULONG _tags[] = { __VA_ARGS__ }; \
  URL_FreePrefsA(__p0, (struct TagItem *)_tags);})

#define URL_SetPrefs(__p0, ...) \
  ({ULONG _tags[] = { __VA_ARGS__ }; \
  URL_SetPrefsA(__p0, (struct TagItem *)_tags);})

#define URL_LaunchPrefsApp(...) \
  ({ULONG _tags[] = { __VA_ARGS__ }; \
  URL_LaunchPrefsAppA((struct TagItem *)_tags);})

#endif

#endif /* !_PPCINLINE_OPENURL_H */
