#ifndef _PROTO_XPKMASTER_H
#define _PROTO_XPKMASTER_H

#include <clib/xpkmaster_protos.h>

#ifndef __NOLIBBASE__
extern struct Library *
#ifdef __CONSTLIBBASEDECL__
__CONSTLIBBASEDECL__
#endif
XpkBase;
#endif

#ifdef __GNUC__
#ifdef __PPC__
#include <ppcinline/xpkmaster.h>
#else
#include <inline/xpkmaster.h>
#endif
#else /* SAS-C */
#ifdef __PPC__
#include <ppcpragmas/xpkmaster_pragmas.h>
#else
#include <pragmas/xpkmaster_pragmas.h>
#endif
#endif

#endif	/*  _PROTO_XPKMASTER_H  */
