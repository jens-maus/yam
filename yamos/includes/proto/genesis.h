#ifndef _PROTO_GENESIS_H
#define _PROTO_GENESIS_H

#include <clib/genesis_protos.h>

#ifndef __NOLIBBASE__
extern struct Library *
#ifdef __CONSTLIBBASEDECL__
__CONSTLIBBASEDECL__
#endif
GenesisBase;
#endif

#ifdef __GNUC__
#ifdef __PPC__
#include <ppcinline/genesis.h>
#else
#include <inline/genesis.h>
#endif
#else /* SAS-C */
#ifdef __PPC__
#include <ppcpragmas/genesis_pragmas.h>
#else
#include <pragmas/genesis_pragmas.h>
#endif
#endif

#endif	/*  _PROTO_GENESIS_H  */
