/* Automatically generated header! Do not edit! */

#ifndef PROTO_MIAMI_H
#define PROTO_MIAMI_H

#if 0
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
struct addrinfo;
#endif

#include <clib/miami_protos.h>

#ifdef __GNUC__
#ifndef __PPC__
#include <inline/miami.h>
#else
#include <ppcinline/miami.h>
#endif /* !__PPC__ */
#endif /* __GNUC__ */

#ifdef __VBCC__
#include <inline/miami_protos.h>
#endif /* __VBCC__ */

#if defined(LATTICE) || defined(__SASC) || defined(_DCC) || defined(__STORM__)
#include <pragmas/miami_pragmas.h>
#endif

#ifndef __NOLIBBASE__
extern struct Library *
#ifdef __CONSTLIBBASEDECL__
__CONSTLIBBASEDECL__
#endif /* __CONSTLIBBASEDECL__ */
MiamiBase;
#endif /* !__NOLIBBASE__ */

#endif /* !PROTO_MIAMI_H */
