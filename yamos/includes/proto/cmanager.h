#ifndef PROTO_CMANAGER_H
#define PROTO_CMANAGER_H

/*
**	$Id$
**	Includes Release 50.1
**
**	Prototype/inline/pragma header file combo
**
**	(C) Copyright 2003-2004 Amiga, Inc.
**	    All Rights Reserved
*/

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

/****************************************************************************/

#ifndef __NOLIBBASE__
extern struct Library * CManagerBase;
#endif /* __NOLIBBASE__ */

/****************************************************************************/

#ifdef __amigaos4__
 #include <interfaces/cmanager.h>

 #ifdef __USE_INLINE__
  #include <inline4/cmanager.h>
 #endif /* __USE_INLINE__ */

 #ifndef __NOGLOBALIFACE__
  extern struct CManagerIFace *ICManager;
 #endif /* __NOGLOBALIFACE__ */
#else /* __amigaos4__ */
 #ifndef CLIB_CMANAGER_PROTOS_H
  #include <clib/cmanager_protos.h>
 #endif /* CLIB_CMANAGER_PROTOS_H */
 #if defined(__GNUC__)
  #ifndef __PPC__
   #include <inline/cmanager.h>
  #else
   #include <ppcinline/cmanager.h>
  #endif /* __PPC__ */
 #elif defined(__VBCC__)
  #ifndef __PPC__
   #include <inline/cmanager_protos.h>
  #endif /* __PPC__ */
 #else
  #include <pragmas/cmanager_pragmas.h>
 #endif /* __GNUC__ */
#endif /* __amigaos4__ */

/****************************************************************************/

#endif /* PROTO_CMANAGER_H */
