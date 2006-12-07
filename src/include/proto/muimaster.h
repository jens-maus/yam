#ifndef PROTO_MUIMASTER_H
#define PROTO_MUIMASTER_H

/*
**	$Id$
**	Includes Release 50.1
**
**	Prototype/inline/pragma header file combo
**
**	(C) Copyright 2003-2005 Amiga, Inc.
**	    All Rights Reserved
*/

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif
#ifndef INTUITION_CLASSES_H
#include <intuition/classes.h>
#endif
#ifndef UTILITY_TAGITEM_H
#include <utility/tagitem.h>
#endif
#ifndef LIBRARIES_MUI_H
#include <libraries/mui.h>
#endif

/****************************************************************************/

#ifndef __NOLIBBASE__
 #ifndef __USE_BASETYPE__
  extern struct Library * MUIMasterBase;
 #else
  extern struct Library * MUIMasterBase;
 #endif /* __USE_BASETYPE__ */
#endif /* __NOLIBBASE__ */

/****************************************************************************/

#ifdef __amigaos4__
 #include <interfaces/muimaster.h>
 #ifdef __USE_INLINE__
  #include <inline4/muimaster.h>
 #endif /* __USE_INLINE__ */
 #ifndef CLIB_MUIMASTER_PROTOS_H
  #define CLIB_MUIMASTER_PROTOS_H 1
 #endif /* CLIB_MUIMASTER_PROTOS_H */
 #ifndef __NOGLOBALIFACE__
  extern struct MUIMasterIFace *IMUIMaster;
 #endif /* __NOGLOBALIFACE__ */
#else /* __amigaos4__ */
 #ifndef CLIB_MUIMASTER_PROTOS_H
  #include <clib/muimaster_protos.h>
 #endif /* CLIB_MUIMASTER_PROTOS_H */
 #if defined(__GNUC__)
  #ifndef __PPC__
   #include <inline/muimaster.h>
  #else
   #include <ppcinline/muimaster.h>
  #endif /* __PPC__ */
 #elif defined(__VBCC__)
  #ifndef __PPC__
   #include <inline/muimaster_protos.h>
  #endif /* __PPC__ */
 #else
  #include <pragmas/muimaster_pragmas.h>
 #endif /* __GNUC__ */
#endif /* __amigaos4__ */

/****************************************************************************/

#endif /* PROTO_MUIMASTER_H */
