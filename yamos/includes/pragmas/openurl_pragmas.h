/**/
/* Pragmas for openurl.library */
/**/
#pragma libcall OpenURLBase URL_OpenA 1E 9802
#ifndef _DCC
#pragma tagcall OpenURLBase URL_Open 1E 9802
#endif
#pragma libcall OpenURLBase URL_GetPrefs 24 0
#pragma libcall OpenURLBase URL_FreePrefs 2A 801
#pragma libcall OpenURLBase URL_SetPrefs 30 0802
/*--- functions in V2 or higher ---*/
#pragma libcall OpenURLBase URL_GetDefaultPrefs 36 0
/*--- functions in V3 or higher ---*/
#pragma libcall OpenURLBase URL_LaunchPrefsApp 3C 0
#pragma libcall OpenURLBase DoFunction 42 801
