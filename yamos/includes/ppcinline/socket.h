#ifndef _PPCINLINE_SOCKET_H
#define _PPCINLINE_SOCKET_H

#ifndef __PPCINLINE_MACROS_H
#include <ppcinline/macros.h>
#endif

#ifndef SOCKET_BASE_NAME
#define SOCKET_BASE_NAME SocketBase
#endif

#define Socket(par1, par2, last) \
	LP3(0x1e, LONG, Socket, LONG, par1, d0, LONG, par2, d1, LONG, last, d2, \
	, SOCKET_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define Connect(par1, par2, last) \
	LP3(0x36, LONG, Connect, LONG, par1, d0, const struct sockaddr *, par2, a0, LONG, last, d1, \
	, SOCKET_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define Send(par1, par2, par3, last) \
	LP4(0x42, LONG, Send, LONG, par1, d0, const unsigned char *, par2, a0, LONG, par3, d1, LONG, last, d2, \
	, SOCKET_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define Recv(par1, par2, par3, last) \
	LP4(0x4e, LONG, Recv, LONG, par1, d0, unsigned char *, par2, a0, LONG, par3, d1, LONG, last, d2, \
	, SOCKET_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define Shutdown(par1, last) \
	LP2(0x54, LONG, Shutdown, LONG, par1, d0, LONG, last, d1, \
	, SOCKET_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define IoctlSocket(par1, par2, last) \
	LP3(0x72, LONG, IoctlSocket, LONG, par1, d0, ULONG, par2, d1, char *, last, a0, \
	, SOCKET_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CloseSocket(last) \
	LP1(0x78, LONG, CloseSocket, LONG, last, d0, \
	, SOCKET_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ObtainSocket(par1, par2, par3, last) \
	LP4(0x90, LONG, ObtainSocket, LONG, par1, d0, LONG, par2, d1, LONG, par3, d2, LONG, last, d3, \
	, SOCKET_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ReleaseSocket(par1, last) \
	LP2(0x96, LONG, ReleaseSocket, LONG, par1, d0, LONG, last, d1, \
	, SOCKET_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ReleaseCopyOfSocket(par1, last) \
	LP2(0x9c, LONG, ReleaseCopyOfSocket, LONG, par1, d0, LONG, last, d1, \
	, SOCKET_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define Errno() \
	LP0(0xa2, LONG, Errno, \
	, SOCKET_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SetErrnoPtr(par1, last) \
	LP2(0xa8, LONG, SetErrnoPtr, void *, par1, a0, LONG, last, d0, \
	, SOCKET_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define Inet_NtoA(last) \
	LP1(0xae, char *, Inet_NtoA, ULONG, last, d0, \
	, SOCKET_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define Inet_LnaOf(last) \
	LP1(0xba, ULONG, Inet_LnaOf, LONG, last, d0, \
	, SOCKET_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define Inet_NetOf(last) \
	LP1(0xc0, ULONG, Inet_NetOf, LONG, last, d0, \
	, SOCKET_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define Inet_MakeAddr(par1, last) \
	LP2(0xc6, ULONG, Inet_MakeAddr, ULONG, par1, d0, ULONG, last, d1, \
	, SOCKET_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define GetHostByName(last) \
	LP1(0xd2, struct hostent *, GetHostByName, const unsigned char *, last, a0, \
	, SOCKET_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define Dup2Socket(par1, last) \
	LP2(0x108, LONG, Dup2Socket, LONG, par1, d0, LONG, last, d1, \
	, SOCKET_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SocketBaseTagList(last) \
	LP1(0x126, LONG, SocketBaseTagList, struct TagItem *, last, a0, \
	, SOCKET_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define SocketBaseTags(tags...) \
	({ULONG _tags[] = {tags}; SocketBaseTagList((struct TagItem *) _tags);})
#endif

#define GetSocketEvents(last) \
	LP1(0x12c, LONG, GetSocketEvents, ULONG *, last, a0, \
	, SOCKET_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#endif /*  _PPCINLINE_SOCKET_H  */
