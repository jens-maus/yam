/* Automatically generated header! Do not edit! */

#ifndef PROTO_SOCKET_H
#define PROTO_SOCKET_H

#include <clib/socket_protos.h>

#ifdef __GNUC__
#ifndef __PPC__
#include <inline/socket.h>
#else
#include <ppcinline/socket.h>
#endif /* !__PPC__ */
#endif /* __GNUC__ */

#ifdef __VBCC__
#include <inline/socket_protos.h>
#endif /* __VBCC__ */

#if defined(LATTICE) || defined(__SASC) || defined(_DCC) || defined(__STORM__)
#include <pragmas/socket_pragmas.h>
#endif

#ifndef __NOLIBBASE__
extern struct Library *
#ifdef __CONSTLIBBASEDECL__
__CONSTLIBBASEDECL__
#endif /* __CONSTLIBBASEDECL__ */
SocketBase;
#endif /* !__NOLIBBASE__ */

#endif /* !PROTO_SOCKET_H */
