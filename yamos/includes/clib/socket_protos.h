#ifndef CLIB_SOCKET_PROTOS_H
#define CLIB_SOCKET_PROTOS_H 1

#define SMTP_NO_SOCKET -1

#define	SOCK_STREAM 1
#define AF_INET 2
#define EINPROGRESS 36

#include "amiga-align.h"

struct in_addr {
  unsigned long s_addr;
};

struct sockaddr_in {
  unsigned char sin_len;
  unsigned char sin_family;
  unsigned short sin_port;
  struct in_addr sin_addr;
  char sin_zero[8];
};

struct hostent {
  char *h_name;
  char **h_aliases;
  int h_addrtype;
  int h_length;
  char **h_addr_list;
  #define h_addr h_addr_list[0]
};

/*
struct sockaddr {
  unsigned char sa_len;
  unsigned char sa_family;
  char sa_data[14];
};

struct addrinfo {
  long ai_flags;
  long ai_family;
  long ai_socktype;
  long ai_protocol;
  size_t ai_addrlen;
  char *ai_canonname;
  struct sockaddr *ai_addr;
  struct addrinfo *ai_next;
};
*/
struct sockaddr;
struct addrinfo;

#include "default-align.h"

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
