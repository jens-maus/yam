#ifndef _PROTO_PM_H
#define _PROTO_PM_H

#include <clib/pm_protos.h>

#ifndef __NOLIBBASE__
extern struct PopupMenuBase *
#ifdef __CONSTLIBBASEDECL__
__CONSTLIBBASEDECL__
#endif
PopupMenuBase;
#endif

#ifdef __GNUC__
#ifdef __PPC__
#include <ppcinline/pm.h>
#else
#include <inline/pm.h>
#endif
#else /* SAS-C */
#ifdef __PPC__
#include <ppcpragmas/pm_pragmas.h>
#else
#include <pragmas/pm_pragmas.h>
#endif
#endif

#endif	/*  _PROTO_PM_H  */
