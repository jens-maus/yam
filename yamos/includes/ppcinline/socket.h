/* Automatically generated header! Do not edit! */

#ifndef _PPCINLINE_SOCKET_H
#define _PPCINLINE_SOCKET_H

#ifndef __PPCINLINE_MACROS_H
#include <ppcinline/macros.h>
#endif /* !__PPCINLINE_MACROS_H */

#ifndef SOCKET_BASE_NAME
#define SOCKET_BASE_NAME SocketBase
#endif /* !SOCKET_BASE_NAME */

#define Inet_NtoA(__p0) \
	LP1(174, char *, Inet_NtoA, \
		ULONG , __p0, d0, \
		, SOCKET_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define IoctlSocket(__p0, __p1, __p2) \
	LP3(114, LONG , IoctlSocket, \
		LONG , __p0, d0, \
		ULONG , __p1, d1, \
		char *, __p2, a0, \
		, SOCKET_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define Dup2Socket(__p0, __p1) \
	LP2(264, LONG , Dup2Socket, \
		LONG , __p0, d0, \
		LONG , __p1, d1, \
		, SOCKET_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define Errno() \
	LP0(162, LONG , Errno, \
		, SOCKET_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ObtainSocket(__p0, __p1, __p2, __p3) \
	LP4(144, LONG , ObtainSocket, \
		LONG , __p0, d0, \
		LONG , __p1, d1, \
		LONG , __p2, d2, \
		LONG , __p3, d3, \
		, SOCKET_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define Inet_MakeAddr(__p0, __p1) \
	LP2(198, ULONG , Inet_MakeAddr, \
		ULONG , __p0, d0, \
		ULONG , __p1, d1, \
		, SOCKET_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SetErrnoPtr(__p0, __p1) \
	LP2(168, LONG , SetErrnoPtr, \
		void *, __p0, a0, \
		LONG , __p1, d0, \
		, SOCKET_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define Socket(__p0, __p1, __p2) \
	LP3(30, LONG , Socket, \
		LONG , __p0, d0, \
		LONG , __p1, d1, \
		LONG , __p2, d2, \
		, SOCKET_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define Send(__p0, __p1, __p2, __p3) \
	LP4(66, LONG , Send, \
		LONG , __p0, d0, \
		const unsigned char *, __p1, a0, \
		LONG , __p2, d1, \
		LONG , __p3, d2, \
		, SOCKET_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define Connect(__p0, __p1, __p2) \
	LP3(54, LONG , Connect, \
		LONG , __p0, d0, \
		const struct sockaddr *, __p1, a0, \
		LONG , __p2, d1, \
		, SOCKET_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ReleaseSocket(__p0, __p1) \
	LP2(150, LONG , ReleaseSocket, \
		LONG , __p0, d0, \
		LONG , __p1, d1, \
		, SOCKET_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define Inet_NetOf(__p0) \
	LP1(192, ULONG , Inet_NetOf, \
		LONG , __p0, d0, \
		, SOCKET_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define Recv(__p0, __p1, __p2, __p3) \
	LP4(78, LONG , Recv, \
		LONG , __p0, d0, \
		unsigned char *, __p1, a0, \
		LONG , __p2, d1, \
		LONG , __p3, d2, \
		, SOCKET_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define GetHostByName(__p0) \
	LP1(210, struct hostent *, GetHostByName, \
		const unsigned char *, __p0, a0, \
		, SOCKET_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ReleaseCopyOfSocket(__p0, __p1) \
	LP2(156, LONG , ReleaseCopyOfSocket, \
		LONG , __p0, d0, \
		LONG , __p1, d1, \
		, SOCKET_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define Shutdown(__p0, __p1) \
	LP2(84, LONG , Shutdown, \
		LONG , __p0, d0, \
		LONG , __p1, d1, \
		, SOCKET_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SocketBaseTagList(__p0) \
	LP1(294, LONG , SocketBaseTagList, \
		struct TagItem *, __p0, a0, \
		, SOCKET_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define GetSocketEvents(__p0) \
	LP1(300, LONG , GetSocketEvents, \
		ULONG *, __p0, a0, \
		, SOCKET_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define Inet_LnaOf(__p0) \
	LP1(186, ULONG , Inet_LnaOf, \
		LONG , __p0, d0, \
		, SOCKET_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CloseSocket(__p0) \
	LP1(120, LONG , CloseSocket, \
		LONG , __p0, d0, \
		, SOCKET_BASE_NAME, 0, 0, 0, 0, 0, 0)

#ifdef USE_INLINE_STDARG

#include <stdarg.h>

#define SocketBaseTags(...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	SocketBaseTagList((struct TagItem *)_tags);})

#endif

#endif /* !_PPCINLINE_SOCKET_H */
