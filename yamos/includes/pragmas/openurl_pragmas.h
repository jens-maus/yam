/**/
/* Pragmas for openurl.library */
/**/
#ifndef _INCLUDE_PRAGMA_OPENURL_LIB_H
#define _INCLUDE_PRAGMA_OPENURL_LIB_H

#if defined(AZTEC_C) || defined(__MAXON__) || defined(__STORM__)
#pragma amicall(OpenURLBase,0x01E,URL_OpenA(a0,a1))
#pragma amicall(OpenURLBase,0x024,URL_GetPrefs())
#pragma amicall(OpenURLBase,0x02A,URL_FreePrefs(a0))
#pragma amicall(OpenURLBase,0x030,URL_SetPrefs(a0,d0))
#pragma amicall(OpenURLBase,0x036,URL_GetDefaultPrefs())
#pragma amicall(OpenURLBase,0x03C,URL_LaunchPrefsApp())
#endif
#if defined(_DCC) || defined(__SASC)
#pragma  libcall OpenURLBase URL_OpenA            01E 9802
#pragma  libcall OpenURLBase URL_GetPrefs         024 00
#pragma  libcall OpenURLBase URL_FreePrefs        02A 801
#pragma  libcall OpenURLBase URL_SetPrefs         030 0802
#pragma  libcall OpenURLBase URL_GetDefaultPrefs  036 00
#pragma  libcall OpenURLBase URL_LaunchPrefsApp   03C 00
#endif
#ifdef __STORM__
#pragma tagcall(OpenURLBase,0x01E,URL_Open(a0,a1))
#endif
#ifdef __SASC_60
#pragma  tagcall OpenURLBase URL_Open             01E 9802
#endif

#endif	/*  _INCLUDE_PRAGMA_OPENURL_LIB_H  */
