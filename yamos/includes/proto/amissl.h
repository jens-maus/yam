#ifndef _PROTO_AMISSL_H
#define _PROTO_AMISSL_H

#include <clib/amissl_protos.h>

#ifndef __NOLIBBASE__
extern struct Library *
#ifdef __CONSTLIBBASEDECL__
__CONSTLIBBASEDECL__
#endif
AmiSSLBase;
#endif

#ifdef __GNUC__
#ifdef __PPC__
#include <ppcinline/amissl.h>
#else
#include <inline/amissl.h>
#endif
#else /* SAS-C */
#ifdef __PPC__
#include <ppcpragmas/amissl_pragmas.h>
#else
#include <pragmas/amissl_pragmas.h>
#endif
#endif

#endif	/*  _PROTO_AMISSL_H  */
