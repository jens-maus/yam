#ifndef PROTO_OPENURL_H
#define PROTO_OPENURL_H
#include <exec/types.h>
#include <clib/openurl_protos.h>
#ifdef __GNUC__
   #include <inline/openurl.h>
#endif
#if defined(LATTICE) || defined(__SASC) || defined(_DCC)
   #include <pragmas/openurl_pragmas.h>
#endif
extern struct Library *OpenURLBase;
#endif
