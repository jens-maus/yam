#ifndef _PROTO_CMANAGER_H
#define _PROTO_CMANAGER_H

#include <clib/cmanager_protos.h>

#ifndef __NOLIBBASE__
extern struct Library *
#ifdef __CONSTLIBBASEDECL__
__CONSTLIBBASEDECL__
#endif
CManagerBase;
#endif

#ifdef __GNUC__
#ifdef __PPC__
#include <ppcinline/cmanager.h>
#else
#include <inline/cmanager.h>
#endif
#else /* SAS-C */
#ifdef __PPC__
#include <ppcpragmas/cmanager_pragmas.h>
#else
#include <pragmas/cmanager_pragmas.h>
#endif
#endif

#endif	/*  _PROTO_CMANAGER_H  */
