#ifndef PROTO_BSDSOCKET_H
#define PROTO_BSDSOCKET_H

#ifndef PROTO_SOCKET_H
#define PROTO_SOCKET_H 1
#endif

/*
 * $Id$
 *
 * :ts=8
 *
 * 'Roadshow' -- Amiga TCP/IP stack
 * Copyright © 2001-2004 by Olaf Barthel.
 * All Rights Reserved.
 *
 * Generic prototype and direct ROM interface definitions
 * Freely Distributable
 */

#ifndef EXEC_LISTS_H
#include <exec/lists.h>
#endif
#ifndef DEVICES_TIMER_H
#include <devices/timer.h>
#endif
#ifndef UTILITY_TAGITEM_H
#include <utility/tagitem.h>
#endif
#ifndef UTILITY_HOOKS_H
#include <utility/hooks.h>
#endif
#ifndef _NETINET_IN_H
#include <netinet/in.h>
#endif
#ifndef _SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifndef _SYS_MBUF_H
#include <sys/mbuf.h>
#endif
#ifndef _NET_ROUTE_H
#include <net/route.h>
#endif
#ifndef _NETDB_H
#include <netdb.h>
#endif
#ifndef LIBRARIES_BSDSOCKET_H
#include <libraries/bsdsocket.h>
#endif
#ifndef DOS_DOSEXTENS_H
#include <dos/dosextens.h>
#endif

/****************************************************************************/

#ifndef __NOLIBBASE__
extern struct Library * SocketBase;
#endif /* __NOLIBBASE__ */

/****************************************************************************/

#ifdef __amigaos4__
 #include <interfaces/bsdsocket.h>
 #ifdef __USE_INLINE__
  #include <inline4/bsdsocket.h>
 #endif /* __USE_INLINE__ */
 #ifndef CLIB_BSDSOCKET_PROTOS_H
  #define CLIB_BSDSOCKET_PROTOS_H 1
 #endif /* CLIB_BSDSOCKET_PROTOS_H */
 #ifndef __NOGLOBALIFACE__
  extern struct SocketIFace *ISocket;
 #endif /* __NOGLOBALIFACE__ */
#else /* __amigaos4__ */
 #if defined(__GNUC__)
  #ifndef __PPC__
   #include <inline/bsdsocket.h>
  #else
   #include <ppcinline/bsdsocket.h>
  #endif
 #elif defined(__VBCC__)
  #ifndef __PPC__
   #include <inline/bsdsocket_protos.h>
  #endif /* __PPC__ */
 #else
  #include <pragmas/bsdsocket_pragmas.h>
 #endif /* __GNUC__ */
#endif /* __amigaos4__ */

/****************************************************************************/

#endif /* PROTO_BSDSOCKET_H */
