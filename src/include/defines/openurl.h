/* Automatically generated header (sfdc 1.4)! Do not edit! */

#ifndef _INLINE_OPENURL_H
#define _INLINE_OPENURL_H

#ifndef _SFDC_VARARG_DEFINED
#define _SFDC_VARARG_DEFINED
#ifdef __HAVE_IPTR_ATTR__
typedef APTR _sfdc_vararg __attribute__((iptr));
#else
typedef ULONG _sfdc_vararg;
#endif /* __HAVE_IPTR_ATTR__ */
#endif /* _SFDC_VARARG_DEFINED */

#ifndef AROS_LIBCALL_H
#include <aros/libcall.h>
#endif /* !AROS_LIBCALL_H */

#ifndef OPENURL_BASE_NAME
#define OPENURL_BASE_NAME OpenURLBase
#endif /* !OPENURL_BASE_NAME */

#define URL_OpenA(___par1, ___last) \
  AROS_LC2(BOOL, URL_OpenA, \
  AROS_LCA(STRPTR, (___par1), A0), \
  AROS_LCA(struct TagItem *, (___last), A1), \
  struct Library *, OPENURL_BASE_NAME, 5, Openurl)

#ifndef NO_INLINE_STDARG
#define URL_Open(___par1, ___last, ...) \
  ({_sfdc_vararg _tags[] = { ___last, __VA_ARGS__ }; URL_OpenA((___par1), (struct TagItem *) _tags); })
#endif /* !NO_INLINE_STDARG */

#define URL_GetPrefs() \
  AROS_LC0(struct URL_Prefs *, URL_GetPrefs, \
  struct Library *, OPENURL_BASE_NAME, 6, Openurl)

#define URL_FreePrefs(___last) \
  AROS_LC1(VOID, URL_FreePrefs, \
  AROS_LCA(struct URL_Prefs *, (___last), A0), \
  struct Library *, OPENURL_BASE_NAME, 7, Openurl)

#define URL_SetPrefs(___par1, ___last) \
  AROS_LC2(BOOL, URL_SetPrefs, \
  AROS_LCA(struct URL_Prefs *, (___par1), A0), \
  AROS_LCA(BOOL, (___last), D0), \
  struct Library *, OPENURL_BASE_NAME, 8, Openurl)

#define URL_GetDefaultPrefs() \
  AROS_LC0(struct URL_Prefs *, URL_GetDefaultPrefs, \
  struct Library *, OPENURL_BASE_NAME, 9, Openurl)

#define URL_LaunchPrefsApp() \
  AROS_LC0(BOOL, URL_LaunchPrefsApp, \
  struct Library *, OPENURL_BASE_NAME, 10, Openurl)

#endif /* !_INLINE_OPENURL_H */
