#ifndef CLIB_SOCKET_PROTOS_H
#define CLIB_SOCKET_PROTOS_H 1

#define SMTP_NO_SOCKET -1

#define	SOCK_STREAM 1
#define AF_INET 2
#define EINPROGRESS 36

#include <sys/types.h>

struct in_addr {
  u_long s_addr;
};

struct sockaddr_in {
  u_char sin_len;
  u_char sin_family;
  u_short sin_port;
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
  u_char sa_len;
  u_char sa_family;
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

#include <exec/types.h>

LONG Socket(LONG,LONG,LONG);
LONG Connect(LONG,const struct sockaddr *,LONG);
LONG Send(LONG,const unsigned char *,LONG,LONG);
LONG Recv(LONG,unsigned char *,LONG,LONG);
LONG Shutdown(LONG,LONG);
LONG CloseSocket(LONG);
struct hostent *GetHostByName(const unsigned char *);
LONG Errno(void);

#endif /* !CLIB_SOCKET_PROTOS_H */
