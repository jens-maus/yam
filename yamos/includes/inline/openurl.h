#ifndef _INLINE_OPENURL_H
#define _INLINE_OPENURL_H

#ifndef __INLINE_STUB_H
#include <inline/stubs.h>
#endif

#ifndef BASE_EXT_DECL
#define BASE_EXT_DECL
#define BASE_EXT_DECL0 extern struct Library *OpenURLBase;
#endif
#ifndef BASE_PAR_DECL
#define BASE_PAR_DECL
#define BASE_PAR_DECL0 void
#endif
#ifndef BASE_NAME
#define BASE_NAME OpenURLBase
#endif

BASE_EXT_DECL0

extern __inline BOOL
URL_OpenA(BASE_PAR_DECL STRPTR url, struct TagItem * tags)
{
  BASE_EXT_DECL
  register BOOL res __asm("d0");
  register struct Library *a6 __asm("a6") = BASE_NAME;
  register STRPTR a0 __asm("a0") = url;
  register struct TagItem * a1 __asm("a1") = tags;
  __asm volatile ("jsr a6@(-0x1E:W)"
  : "=r" (res)
  : "r" (a6), "r" (a0), "r" (a1)
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory");
  return res;
}

extern __inline BOOL URL_Open(STRPTR url, Tag tags, ...)
{
  return URL_OpenA(url, (struct TagItem *) &tags);
}

extern __inline struct URL_Prefs *
URL_GetPrefs(BASE_PAR_DECL0)
{
  BASE_EXT_DECL
  register struct URL_Prefs * res __asm("d0");
  register struct Library *a6 __asm("a6") = BASE_NAME;
  __asm volatile ("jsr a6@(-0x24:W)"
  : "=r" (res)
  : "r" (a6)
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory");
  return res;
}

extern __inline VOID
URL_FreePrefs(BASE_PAR_DECL struct URL_Prefs * up)
{
  BASE_EXT_DECL
  register struct Library *a6 __asm("a6") = BASE_NAME;
  register struct URL_Prefs * a0 __asm("a0") = up;
  __asm volatile ("jsr a6@(-0x2A:W)"
  : /* No Output */
  : "r" (a6), "r" (a0)
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory");
}

extern __inline BOOL
URL_SetPrefs(BASE_PAR_DECL struct URL_Prefs * up, BOOL permanent)
{
  BASE_EXT_DECL
  register BOOL res __asm("d0");
  register struct Library *a6 __asm("a6") = BASE_NAME;
  register struct URL_Prefs * a0 __asm("a0") = up;
  register BOOL d0 __asm("d0") = permanent;
  __asm volatile ("jsr a6@(-0x30:W)"
  : "=r" (res)
  : "r" (a6), "r" (a0), "r" (d0)
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory");
  return res;
}

extern __inline struct URL_Prefs *
URL_GetDefaultPrefs(BASE_PAR_DECL0)
{
  BASE_EXT_DECL
  register struct URL_Prefs * res __asm("d0");
  register struct Library *a6 __asm("a6") = BASE_NAME;
  __asm volatile ("jsr a6@(-0x36:W)"
  : "=r" (res)
  : "r" (a6)
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory");
  return res;
}

extern __inline BOOL
URL_LaunchPrefsApp(BASE_PAR_DECL0)
{
  BASE_EXT_DECL
  register BOOL res __asm("d0");
  register struct Library *a6 __asm("a6") = BASE_NAME;
  __asm volatile ("jsr a6@(-0x3C:W)"
  : "=r" (res)
  : "r" (a6)
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory");
  return res;
}

#undef BASE_EXT_DECL
#undef BASE_EXT_DECL0
#undef BASE_PAR_DECL
#undef BASE_PAR_DECL0
#undef BASE_NAME

#endif /*  _INLINE_OPENURL_H  */
