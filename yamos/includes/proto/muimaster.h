#ifndef _PROTO_MUIMASTER_H
#define _PROTO_MUIMASTER_H

#include <clib/muimaster_protos.h>

#ifndef __NOLIBBASE__
extern struct Library *
#ifdef __CONSTLIBBASEDECL__
__CONSTLIBBASEDECL__
#endif
MUIMasterBase;
#endif

#ifdef __GNUC__
#ifdef __PPC__
#include <ppcinline/muimaster.h>
#else
#include <inline/muimaster.h>
#endif
#else /* SAS-C */
#ifdef __PPC__
#include <ppcpragmas/muimaster_pragmas.h>
#else
#include <pragmas/muimaster_pragmas.h>
#endif
#endif

#endif	/*  _PROTO_MUIMASTER_H  */
