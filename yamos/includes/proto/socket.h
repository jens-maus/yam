#ifndef PROTO_SOCKET_H
#define PROTO_SOCKET_H

/*
**	$Id$
**	Includes Release 50.1
**
**	Prototype/inline/pragma header file combo
**
**	(C) Copyright 2003-2004 Amiga, Inc.
**	    All Rights Reserved
*/

#ifndef LIBRARIES_SOCKET_H
#include <libraries/socket.h>
#endif
#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

/****************************************************************************/

#ifndef __NOLIBBASE__
extern struct Library * SocketBase;
#endif /* __NOLIBBASE__ */

/****************************************************************************/

#ifdef __amigaos4__
 #include <interfaces/socket.h>

 #ifdef __USE_INLINE__
  #include <inline4/socket.h>
 #endif /* __USE_INLINE__ */

 #ifndef __NOGLOBALIFACE__
  extern struct SocketIFace *ISocket;
 #endif /* __NOGLOBALIFACE__ */
#else /* __amigaos4__ */
 #ifndef CLIB_SOCKET_PROTOS_H
  #include <clib/socket_protos.h>
 #endif /* CLIB_SOCKET_PROTOS_H */
 #if defined(__GNUC__)
  #ifndef __PPC__
   #include <inline/socket.h>
  #else
   #include <ppcinline/socket.h>
  #endif /* __PPC__ */
 #elif defined(__VBCC__)
  #ifndef __PPC__
   #include <inline/socket_protos.h>
  #endif /* __PPC__ */
 #else
  #include <pragmas/socket_pragmas.h>
 #endif /* __GNUC__ */
#endif /* __amigaos4__ */

/****************************************************************************/

#endif /* PROTO_SOCKET_H */
