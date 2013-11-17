#ifndef PROTO_MIAMI_H
#define PROTO_MIAMI_H

/*
**  $Id: miami.h 2024 2006-03-05 10:27:25Z damato $
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

/****************************************************************************/

#ifndef __NOLIBBASE__
extern struct Library * MiamiBase;
#endif /* __NOLIBBASE__ */

/****************************************************************************/

#ifdef __amigaos4__
 #include <interfaces/miami.h>
 #ifdef __USE_INLINE__
  #include <inline4/miami.h>
 #endif /* __USE_INLINE__ */
 #ifndef CLIB_MIAMI_PROTOS_H
  #define CLIB_MIAMI_PROTOS_H 1
 #endif /* CLIB_MIAMI_PROTOS_H */
 #ifndef __NOGLOBALIFACE__
  extern struct MiamiIFace *IMiami;
 #endif /* __NOGLOBALIFACE__ */
#else /* __amigaos4__ */
 #ifndef CLIB_MIAMI_PROTOS_H
  #include <clib/miami_protos.h>
 #endif /* CLIB_MIAMI_PROTOS_H */
 #if defined(__GNUC__)
  #ifndef __PPC__
   #include <inline/miami.h>
  #else
   #include <ppcinline/miami.h>
  #endif /* __PPC__ */
 #elif defined(__VBCC__)
  #ifndef __PPC__
   #include <inline/miami_protos.h>
  #endif /* __PPC__ */
 #else
  #include <pragmas/miami_pragmas.h>
 #endif /* __GNUC__ */
#endif /* __amigaos4__ */

/****************************************************************************/

#endif /* PROTO_MIAMI_H */
