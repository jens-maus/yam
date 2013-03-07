#ifndef PROTO_GUIGFX_H
#define PROTO_GUIGFX_H

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
 #ifndef __USE_BASETYPE__
  extern struct Library * GuiGFXBase;
 #else
  extern struct Library * GuiGFXBase;
 #endif /* __USE_BASETYPE__ */
#endif /* __NOLIBBASE__ */

/****************************************************************************/

#ifdef __amigaos4__
 #include <interfaces/guigfx.h>
 #ifdef __USE_INLINE__
  #include <inline4/guigfx.h>
 #endif /* __USE_INLINE__ */
 #ifndef CLIB_GUIGFX_PROTOS_H
  #define CLIB_GUIGFX_PROTOS_H 1
 #endif /* CLIB_GUIGFX_PROTOS_H */
 #ifndef __NOGLOBALIFACE__
  extern struct GuiGFXIFace *IGuiGFX;
 #endif /* __NOGLOBALIFACE__ */
#else /* __amigaos4__ */
 #ifndef CLIB_GUIGFX_PROTOS_H
  #include <clib/guigfx_protos.h>
 #endif /* CLIB_GUIGFX_PROTOS_H */
 #if defined(__GNUC__)
  #ifndef __PPC__
   #include <inline/guigfx.h>
  #else
   #include <ppcinline/guigfx.h>
  #endif /* __PPC__ */
 #elif defined(__VBCC__)
  #ifndef __PPC__
   #include <inline/guigfx_protos.h>
  #endif /* __PPC__ */
 #else
  #include <pragmas/guigfx_pragmas.h>
 #endif /* __GNUC__ */
#endif /* __amigaos4__ */

/****************************************************************************/

#endif /* PROTO_GUIGFX_H */
