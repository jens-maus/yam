#ifndef PROTO_XADMASTER_H
#define PROTO_XADMASTER_H

/*
**  $Id$
**  Includes Release 50.1
**
**  Prototype/inline/pragma header file combo
**
**  (C) Copyright 2003-2007 Amiga, Inc.
**      All Rights Reserved
*/

#ifndef UTILITY_TAGITEM_H
#include <utility/tagitem.h>
#endif
#ifndef LIBRARIES_XADMASTER_H
#include <libraries/xadmaster.h>
#endif

/****************************************************************************/

#ifndef __NOLIBBASE__
 #ifndef __USE_BASETYPE__
  extern struct Library * xadMasterBase;
 #else
  extern struct xadMasterBase * xadMasterBase;
 #endif /* __USE_BASETYPE__ */
#endif /* __NOLIBBASE__ */

/****************************************************************************/

#ifdef __amigaos4__
 #include <interfaces/xadmaster.h>
 #ifdef __USE_INLINE__
  #include <inline4/xadmaster.h>
 #endif /* __USE_INLINE__ */
 #ifndef CLIB_XADMASTER_PROTOS_H
  #define CLIB_XADMASTER_PROTOS_H 1
 #endif /* CLIB_XADMASTER_PROTOS_H */
 #ifndef __NOGLOBALIFACE__
  extern struct xadMasterIFace *IxadMaster;
 #endif /* __NOGLOBALIFACE__ */
#else /* __amigaos4__ */
 #ifndef CLIB_XADMASTER_PROTOS_H
  #include <clib/xadmaster_protos.h>
 #endif /* CLIB_XADMASTER_PROTOS_H */
 #if defined(__GNUC__)
  #ifdef __AROS__
   #include <defines/xadmaster.h>
  #else
   #ifndef __PPC__
    #include <inline/xadmaster.h>
   #else
    #include <ppcinline/xadmaster.h>
   #endif /* __PPC__ */
  #endif /* __AROS__ */
 #elif defined(__VBCC__)
  #ifndef __PPC__
   #include <inline/xadmaster_protos.h>
  #endif /* __PPC__ */
 #else
  #include <pragmas/xadmaster_pragmas.h>
 #endif /* __GNUC__ */
#endif /* __amigaos4__ */

/****************************************************************************/

#endif /* PROTO_XADMASTER_H */
