#ifndef _PROTO_MIAMI_H
#define _PROTO_MIAMI_H

#include <clib/miami_protos.h>

#ifndef __NOLIBBASE__
extern struct Library *
#ifdef __CONSTLIBBASEDECL__
__CONSTLIBBASEDECL__
#endif
MiamiBase;
#endif

#ifdef __GNUC__
#ifdef __PPC__
#include <ppcinline/miami.h>
#else
#include <inline/miami.h>
#endif
#else /* SAS-C */
#ifdef __PPC__
#include <ppcpragmas/miami_pragmas.h>
#else
#include <pragmas/miami_pragmas.h>
#endif
#endif

#endif	/*  _PROTO_MIAMI_H  */
