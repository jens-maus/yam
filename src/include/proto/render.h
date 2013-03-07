#ifndef PROTO_RENDER_H
#define PROTO_RENDER_H

/*
**	$Id$
**	Includes Release 50.1
**
**	Prototype/inline/pragma header file combo
**
**	(C) Copyright 2003-2004 Amiga, Inc.
**	    All Rights Reserved
*/

#ifndef GRAPHICS_GFX_H
#include <graphics/gfx.h>
#endif
#ifndef UTILITY_HOOKS_H
#include <utility/hooks.h>
#endif
#ifndef UTILITY_TAGITEM_H
#include <utility/tagitem.h>
#endif

/****************************************************************************/

#ifndef __NOLIBBASE__
 #ifndef __USE_BASETYPE__
  extern struct Library * RenderBase;
 #else
  extern struct Library * RenderBase;
 #endif /* __USE_BASETYPE__ */
#endif /* __NOLIBBASE__ */

/****************************************************************************/

#ifdef __amigaos4__
 #include <interfaces/render.h>
 #ifdef __USE_INLINE__
  #include <inline4/render.h>
 #endif /* __USE_INLINE__ */
 #ifndef CLIB_RENDER_PROTOS_H
  #define CLIB_RENDER_PROTOS_H 1
 #endif /* CLIB_RENDER_PROTOS_H */
 #ifndef __NOGLOBALIFACE__
  extern struct RenderIFace *IRender;
 #endif /* __NOGLOBALIFACE__ */
#else /* __amigaos4__ */
 #ifndef CLIB_RENDER_PROTOS_H
  #include <clib/render_protos.h>
 #endif /* CLIB_RENDER_PROTOS_H */
 #if defined(__GNUC__)
  #ifndef __PPC__
   #include <inline/render.h>
  #else
   #include <ppcinline/render.h>
  #endif /* __PPC__ */
 #elif defined(__VBCC__)
  #ifndef __PPC__
   #include <inline/render_protos.h>
  #endif /* __PPC__ */
 #else
  #include <pragmas/render_pragmas.h>
 #endif /* __GNUC__ */
#endif /* __amigaos4__ */

/****************************************************************************/

#endif /* PROTO_RENDER_H */
