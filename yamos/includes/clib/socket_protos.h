#ifndef CLIB_SOCKET_PROTOS_H
#define CLIB_SOCKET_PROTOS_H 1

#include <libraries/socket.h>
#include <exec/types.h>

LONG Socket(LONG,LONG,LONG);
LONG Connect(LONG,const struct sockaddr *,LONG);
LONG Send(LONG,const unsigned char *,LONG,LONG);
LONG Recv(LONG,unsigned char *,LONG,LONG);
LONG Shutdown(LONG,LONG);
LONG CloseSocket(LONG);
struct hostent *GetHostByName(const unsigned char *);
LONG Errno(void);
LONG IoctlSocket(LONG d, ULONG request, char *argp);
LONG ObtainSocket(LONG id, LONG domain, LONG type, LONG protocol);
LONG ReleaseSocket(LONG fd, LONG id);
LONG ReleaseCopyOfSocket(LONG fd, LONG id);
LONG SetErrnoPtr(void *errno_p, LONG size);
char *Inet_NtoA(ULONG s_addr);
ULONG Inet_MakeAddr(ULONG net, ULONG lna);
ULONG Inet_LnaOf(LONG s_addr);
ULONG Inet_NetOf(LONG s_addr);
LONG Dup2Socket(LONG fd1, LONG fd2);
LONG SocketBaseTagList(struct TagItem *tagList);
LONG SocketBaseTags(LONG tag, ...);
LONG GetSocketEvents(ULONG *eventmaskp);

#endif /* !CLIB_SOCKET_PROTOS_H */
