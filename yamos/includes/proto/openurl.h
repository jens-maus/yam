#ifndef _PROTO_OPENURL_H
#define _PROTO_OPENURL_H

#include <clib/openurl_protos.h>

#ifndef __NOLIBBASE__
extern struct Library *
#ifdef __CONSTLIBBASEDECL__
__CONSTLIBBASEDECL__
#endif
OpenURLBase;
#endif

#ifdef __GNUC__
#ifdef __PPC__
#include <ppcinline/openurl.h>
#else
#include <inline/openurl.h>
#endif
#else /* SAS-C */
#ifdef __PPC__
#include <ppcpragmas/openurl_pragmas.h>
#else
#include <pragmas/openurl_pragmas.h>
#endif
#endif

#endif	/*  _PROTO_OPENURL_H  */
