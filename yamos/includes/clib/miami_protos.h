#ifndef  CLIB_MIAMI_PROTOS_H
#define  CLIB_MIAMI_PROTOS_H

/*
**	$VER: miami_protos.h 2.1 (10.07.97)
**
**	C prototypes. For use with 32 bit integers only.
**
**	(C) Copyright 1996 Holger Kruse
**	    All Rights Reserved
*/

long MiamiSysCtl(long *,unsigned long,void *,unsigned long *,void *,long);
void MiamiDisallowDNS(long);
void *MiamiGetPid(void);
void *MiamiPFAddHook(struct Hook *,unsigned char *,struct TagItem *);
void MiamiPFRemoveHook(void *);
long MiamiGetHardwareLen(char *);
struct Library *MiamiOpenSSL(struct TagItem *);
void MiamiCloseSSL(void);
long MiamiSetSocksConn(struct sockaddr *,long);
long MiamiIsOnline(char *);
void MiamiOnOffline(char *,long);
char *inet_ntop(long,void *,char *,long);
long inet_aton(char *,struct in_addr *);
long inet_pton(long,char *,void *);
struct hostent *gethostbyname2(char *,long);
char *gai_strerror(long);
void freeaddrinfo(struct addrinfo *);
long getaddrinfo(char *,char *,struct addrinfo *,struct addrinfo **);
long getnameinfo(struct sockaddr *,long,char *,long,char *,long,long);
long if_nametoindex(char *);
char *if_indextoname(long,char *);
struct if_nameindex *if_nameindex(void);
void if_freenameindex(struct if_nameindex *);
long MiamiSupportsIPV6(void);
long MiamiResGetOptions(void);
void MiamiResSetOptions(long);
long sockatmark(long);
void MiamiSupportedCPUs(unsigned long *,unsigned long *,unsigned long *);
long MiamiGetFdCallback(void **);
long MiamiSetFdCallback(void *,long);
struct UserGroupCredentials *MiamiGetCredentials(void);

#define MIAMINAME "miami.library"

#endif
