#ifndef	CLIB_OPENURL_PROTOS_H
#define	CLIB_OPENURL_PROTOS_H

/*
** openurl.library - universal URL display and browser launcher library
** Written by Troels Walsted Hansen <troels@thule.no>
** Placed in the public domain.
**
** Library prototypes.
*/

/**************************************************************************/

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#ifndef UTILITY_TAGITEM_H
#include <utility/tagitem.h>
#endif

#ifndef LIBRARIES_URL_H
#include <libraries/openurl.h>
#endif

/**************************************************************************/

BOOL URL_OpenA(STRPTR, struct TagItem *);
BOOL URL_Open(STRPTR, Tag, ...);
struct URL_Prefs *URL_GetPrefs(VOID);
VOID URL_FreePrefs(struct URL_Prefs *);
BOOL URL_SetPrefs(struct URL_Prefs *, BOOL);
struct URL_Prefs *URL_GetDefaultPrefs(VOID);
BOOL URL_LaunchPrefsApp(VOID);

#endif  /* CLIB_OPENURL_PROTOS_H */
