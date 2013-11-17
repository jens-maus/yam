#ifndef PROTO_XPKMASTER_H
#define PROTO_XPKMASTER_H

/*
**  $Id: xpkmaster.h 2024 2006-03-05 10:27:25Z damato $
**  Includes Release 50.1
**
**  Prototype/inline/pragma header file combo
**
**  (C) Copyright 2003-2004 Amiga, Inc.
**      All Rights Reserved
*/

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif
#ifndef XPK_XPK_H
#include <xpk/xpk.h>
#endif

/****************************************************************************/

#ifndef __NOLIBBASE__
extern struct Library * XpkBase;
#endif /* __NOLIBBASE__ */

/****************************************************************************/

#ifdef __amigaos4__
 #include <interfaces/xpkmaster.h>
 #ifdef __USE_INLINE__
  #include <inline4/xpkmaster.h>
 #endif /* __USE_INLINE__ */
 #ifndef CLIB_XPKMASTER_PROTOS_H
  #define CLIB_XPKMASTER_PROTOS_H 1
 #endif /* CLIB_XPKMASTER_PROTOS_H */
 #ifndef __NOGLOBALIFACE__
  extern struct XpkIFace *IXpk;
 #endif /* __NOGLOBALIFACE__ */
#else /* __amigaos4__ */
 #ifndef CLIB_XPKMASTER_PROTOS_H
  #include <clib/xpkmaster_protos.h>
 #endif /* CLIB_XPKMASTER_PROTOS_H */
 #if defined(__GNUC__)
  #ifdef __AROS__
   #include <defines/xpkmaster.h>
  #else
   #ifndef __PPC__
    #include <inline/xpkmaster.h>
   #else
    #include <ppcinline/xpkmaster.h>
   #endif /* __PPC__ */
  #endif /* __AROS__ */
 #elif defined(__VBCC__)
  #ifndef __PPC__
   #include <inline/xpkmaster_protos.h>
  #endif /* __PPC__ */
 #else
  #include <pragmas/xpkmaster_pragmas.h>
 #endif /* __GNUC__ */
#endif /* __amigaos4__ */

/****************************************************************************/

#endif /* PROTO_XPKMASTER_H */
