#ifndef _INLINE_SOCKET_H
#define _INLINE_SOCKET_H

#ifndef __INLINE_MACROS_H
#include <inline/macros.h>
#endif

#ifndef SOCKET_BASE_NAME
#define SOCKET_BASE_NAME SocketBase
#endif

#define Socket(d, t, p) \
	LP3(0x1e, LONG, Socket, LONG, d, d0, LONG, t, d1, LONG, p, d2, \
	, SOCKET_BASE_NAME)

#define Connect(s, n, nl) \
	LP3(0x36, LONG, Connect, LONG, s, d0, const struct sockaddr *, n, a0, LONG, nl, d1, \
	, SOCKET_BASE_NAME)

#define Send(s, m, l, f) \
	LP4(0x42, LONG, Send, LONG, s, d0, const unsigned char *, m, a0, LONG, l, d1, LONG, f, d2, \
	, SOCKET_BASE_NAME)

#define Recv(s, b, l, f) \
	LP4(0x4e, LONG, Recv, LONG, s, d0, unsigned char *, b, a0, LONG, l, d1, LONG, f, d2, \
	, SOCKET_BASE_NAME)

#define Shutdown(s, h) \
	LP2(0x54, LONG, Shutdown, LONG, s, d0, LONG, h, d1, \
	, SOCKET_BASE_NAME)

#define IoctlSocket(d, r, a) \
	LP3(0x72, LONG, IoctlSocket, LONG, d, d0, ULONG, r, d1, char *, a, a0, \
	, SOCKET_BASE_NAME)

#define CloseSocket(d) \
	LP1(0x78, LONG, CloseSocket, LONG, d, d0, \
	, SOCKET_BASE_NAME)

#define ObtainSocket(i, d, t, p) \
	LP4(0x90, LONG, ObtainSocket, LONG, i, d0, LONG, d, d1, LONG, t, d2, LONG, p, d3, \
	, SOCKET_BASE_NAME)

#define ReleaseSocket(f, i) \
	LP2(0x96, LONG, ReleaseSocket, LONG, f, d0, LONG, i, d1, \
	, SOCKET_BASE_NAME)

#define ReleaseCopyOfSocket(f, i) \
	LP2(0x9c, LONG, ReleaseCopyOfSocket, LONG, f, d0, LONG, i, d1, \
	, SOCKET_BASE_NAME)

#define Errno() \
	LP0(0xa2, LONG, Errno, \
	, SOCKET_BASE_NAME)

#define SetErrnoPtr(e, s) \
	LP2(0xa8, LONG, SetErrnoPtr, void *, e, a0, LONG, s, d0, \
	, SOCKET_BASE_NAME)

#define Inet_NtoA(i) \
	LP1(0xae, char *, Inet_NtoA, ULONG, i, d0, \
	, SOCKET_BASE_NAME)

#define Inet_LnaOf(i) \
	LP1(0xba, ULONG, Inet_LnaOf, LONG, i, d0, \
	, SOCKET_BASE_NAME)

#define Inet_NetOf(i) \
	LP1(0xc0, ULONG, Inet_NetOf, LONG, i, d0, \
	, SOCKET_BASE_NAME)

#define Inet_MakeAddr(n, h) \
	LP2(0xc6, ULONG, Inet_MakeAddr, ULONG, n, d0, ULONG, h, d1, \
	, SOCKET_BASE_NAME)

#define GetHostByName(n) \
	LP1(0xd2, struct hostent *, GetHostByName, const unsigned char *, n, a0, \
	, SOCKET_BASE_NAME)

#define Dup2Socket(fa, fb) \
	LP2(0x108, LONG, Dup2Socket, LONG, fa, d0, LONG, fb, d1, \
	, SOCKET_BASE_NAME)

#define SocketBaseTagList(t) \
	LP1(0x126, LONG, SocketBaseTagList, struct TagItem *, t, a0, \
	, SOCKET_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define SocketBaseTags(tags...) \
	({ULONG _tags[] = {tags}; SocketBaseTagList((struct TagItem *) _tags);})
#endif

#define GetSocketEvents(e) \
	LP1(0x12c, LONG, GetSocketEvents, ULONG *, e, a0, \
	, SOCKET_BASE_NAME)

#endif /*  _INLINE_SOCKET_H  */
