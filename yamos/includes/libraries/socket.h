#ifndef LIBRARIES_SOCKET_H
#define LIBRARIES_SOCKET_H

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

#ifndef htons
#define htons(x) (x)
#endif

#include "default-align.h"

#endif  /* LIBRARIES_SOCKET_H */
