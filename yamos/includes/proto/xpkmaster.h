#ifndef _PROTO_XPKMASTER_H
#define _PROTO_XPKMASTER_H

#ifndef __NOLIBBASE__
  extern struct Library *XpkBase;
#endif

#include <exec/types.h>
#include <clib/xpkmaster_protos.h>

#ifdef __GNUC__
  #ifndef __PPC__
    #include <inline/xpkmaster.h>
  #else
    #include <ppcinline/xpkmaster.h>    
  #endif
#elif defined(__VBCC__)
  #include <inline/xpkmaster_protos.h>
#else
  #include <pragma/xpkmaster_lib.h>
#endif

#endif	/*  _PROTO_XPKMASTER_H  */
