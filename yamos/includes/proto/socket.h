#ifndef _PROTO_SOCKET_H
#define _PROTO_SOCKET_H

#include <clib/socket_protos.h>

#ifndef __NOLIBBASE__
extern struct Library *
#ifdef __CONSTLIBBASEDECL__
__CONSTLIBBASEDECL__
#endif
SocketBase;
#endif

#ifdef __GNUC__
#ifdef __PPC__
#include <ppcinline/socket.h>
#else
#include <inline/socket.h>
#endif
#else /* SAS-C */
#ifdef __PPC__
#include <ppcpragmas/socket_pragmas.h>
#else
#include <pragmas/socket_pragmas.h>
#endif
#endif

#endif	/*  _PROTO_SOCKET_H  */
