#ifndef _VBCCINLINE_SOCKET_H
#define _VBCCINLINE_SOCKET_H

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

LONG __Socket(__reg("d0") LONG d, __reg("d1") LONG t, __reg("d2") LONG p, __reg("a6") APTR)="\tjsr\t-30(a6)";
#define Socket(d, t, p) __Socket((d), (t), (p), SocketBase)

LONG __Connect(__reg("d0") LONG s, __reg("a0") const struct sockaddr * n, __reg("d1") LONG nl, __reg("a6") APTR)="\tjsr\t-54(a6)";
#define Connect(s, n, nl) __Connect((s), (n), (nl), SocketBase)

LONG __Send(__reg("d0") LONG s, __reg("a0") const unsigned char * m, __reg("d1") LONG l, __reg("d2") LONG f, __reg("a6") APTR)="\tjsr\t-66(a6)";
#define Send(s, m, l, f) __Send((s), (m), (l), (f), SocketBase)

LONG __Recv(__reg("d0") LONG s, __reg("a0") unsigned char * b, __reg("d1") LONG l, __reg("d2") LONG f, __reg("a6") APTR)="\tjsr\t-78(a6)";
#define Recv(s, b, l, f) __Recv((s), (b), (l), (f), SocketBase)

LONG __Shutdown(__reg("d0") LONG s, __reg("d1") LONG h, __reg("a6") APTR)="\tjsr\t-84(a6)";
#define Shutdown(s, h) __Shutdown((s), (h), SocketBase)

LONG __IoctlSocket(__reg("d0") LONG d, __reg("d1") ULONG r, __reg("a0") char * a, __reg("a6") APTR)="\tjsr\t-114(a6)";
#define IoctlSocket(d, r, a) __IoctlSocket((d), (r), (a), SocketBase)

LONG __CloseSocket(__reg("d0") LONG d, __reg("a6") APTR)="\tjsr\t-120(a6)";
#define CloseSocket(d) __CloseSocket((d), SocketBase)

LONG __ObtainSocket(__reg("d0") LONG i, __reg("d1") LONG d, __reg("d2") LONG t, __reg("d3") LONG p, __reg("a6") APTR)="\tjsr\t-144(a6)";
#define ObtainSocket(i, d, t, p) __ObtainSocket((i), (d), (t), (p), SocketBase)

LONG __ReleaseSocket(__reg("d0") LONG f, __reg("d1") LONG i, __reg("a6") APTR)="\tjsr\t-150(a6)";
#define ReleaseSocket(f, i) __ReleaseSocket((f), (i), SocketBase)

LONG __ReleaseCopyOfSocket(__reg("d0") LONG f, __reg("d1") LONG i, __reg("a6") APTR)="\tjsr\t-156(a6)";
#define ReleaseCopyOfSocket(f, i) __ReleaseCopyOfSocket((f), (i), SocketBase)

LONG __Errno(__reg("a6") APTR)="\tjsr\t-162(a6)";
#define Errno() __Errno(SocketBase)

LONG __SetErrnoPtr(__reg("a0") void * e, __reg("d0") LONG s, __reg("a6") APTR)="\tjsr\t-168(a6)";
#define SetErrnoPtr(e, s) __SetErrnoPtr((e), (s), SocketBase)

char * __Inet_NtoA(__reg("d0") ULONG i, __reg("a6") APTR)="\tjsr\t-174(a6)";
#define Inet_NtoA(i) __Inet_NtoA((i), SocketBase)

ULONG __Inet_LnaOf(__reg("d0") LONG i, __reg("a6") APTR)="\tjsr\t-186(a6)";
#define Inet_LnaOf(i) __Inet_LnaOf((i), SocketBase)

ULONG __Inet_NetOf(__reg("d0") LONG i, __reg("a6") APTR)="\tjsr\t-192(a6)";
#define Inet_NetOf(i) __Inet_NetOf((i), SocketBase)

ULONG __Inet_MakeAddr(__reg("d0") ULONG n, __reg("d1") ULONG h, __reg("a6") APTR)="\tjsr\t-198(a6)";
#define Inet_MakeAddr(n, h) __Inet_MakeAddr((n), (h), SocketBase)

struct hostent * __GetHostByName(__reg("a0") const unsigned char * n, __reg("a6") APTR)="\tjsr\t-210(a6)";
#define GetHostByName(n) __GetHostByName((n), SocketBase)

LONG __Dup2Socket(__reg("d0") LONG fa, __reg("d1") LONG fb, __reg("a6") APTR)="\tjsr\t-264(a6)";
#define Dup2Socket(fa, fb) __Dup2Socket((fa), (fb), SocketBase)

LONG __SocketBaseTagList(__reg("a0") struct TagItem * t, __reg("a6") APTR)="\tjsr\t-294(a6)";
#define SocketBaseTagList(t) __SocketBaseTagList((t), SocketBase)

LONG __GetSocketEvents(__reg("a0") ULONG * e, __reg("a6") APTR)="\tjsr\t-300(a6)";
#define GetSocketEvents(e) __GetSocketEvents((e), SocketBase)

#endif /*  _VBCCINLINE_SOCKET_H  */
