/* Automatically generated header! Do not edit! */

#ifndef _INLINE_SOCKET_H
#define _INLINE_SOCKET_H

#ifndef __INLINE_MACROS_H
#include <inline/macros.h>
#endif /* !__INLINE_MACROS_H */

#ifndef SOCKET_BASE_NAME
#define SOCKET_BASE_NAME SocketBase
#endif /* !SOCKET_BASE_NAME */

#define CloseSocket(d) \
	LP1(0x78, long, CloseSocket, long, d, d0, \
	, SOCKET_BASE_NAME)

#define Connect(s, n, nl) \
	LP3(0x36, long, Connect, long, s, d0, const struct sockaddr *, n, a0, long, nl, d1, \
	, SOCKET_BASE_NAME)

#define GetHostByName(n) \
	LP1(0xd2, struct hostent *, GetHostByName, const unsigned char *, n, a0, \
	, SOCKET_BASE_NAME)

#define Recv(s, b, l, f) \
	LP4(0x4e, long, Recv, long, s, d0, unsigned char *, b, a0, long, l, d1, long, f, d2, \
	, SOCKET_BASE_NAME)

#define Send(s, m, l, f) \
	LP4(0x42, long, Send, long, s, d0, const unsigned char *, m, a0, long, l, d1, long, f, d2, \
	, SOCKET_BASE_NAME)

#define Shutdown(s, h) \
	LP2(0x54, long, Shutdown, long, s, d0, long, h, d1, \
	, SOCKET_BASE_NAME)

#define Socket(d, t, p) \
	LP3(0x1e, long, Socket, long, d, d0, long, t, d1, long, p, d2, \
	, SOCKET_BASE_NAME)

#endif /* !_INLINE_SOCKET_H */
